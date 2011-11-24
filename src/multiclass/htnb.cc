/*
 * htnb.cc
 *
 *  Created on: Apr 1, 2011
 *      Author: caglar
 */

#include <fstream>
#include <float.h>
#include <netdb.h>
#include <cstring>
#include <cstdio>

#include <vector>

#include "multiclass/htnb.h"
#include "multiclass/dvec.h"
#include "multiclass/attrobs.h"
#include "multiclass/numattrobs.h"
#include "multiclass/nomattrobs.h"
#include "multiclass/ht.h"

#include "parse_example.h"
#include "constant.h"
#include "sparse_dense.h"
#include "cache.h"
#include "multisource.h"
#include "simple_label.h"
#include "delay_ring.h"
#include "utils.h"

static void
finish_example(example* ec);

static DVec observedClassDist;
static vector<AttributeClassObserver *> attributeObservers;

void *
htnb_thread (void *in)
{
    htnb_thread_params* params = (htnb_thread_params*) in;
    size_t thread_num = params->thread_num;
    example* ec = NULL;
    params->vars = (htnb_vars *) c_malloc(sizeof(htnb_vars));
    while (true) {
        //this is a poor man's select operation.
        if ((ec = get_delay_example(thread_num)) != NULL)//nonblocking
        {
            htnb_train_on_example(ec, params->arfHeader, thread_num);
            finish_example(ec);
        }
        else if ((ec = get_example(thread_num)) != NULL)//semiblocking operation.
        {
            C_ASSERT(ec->in_use);
            if ((ec->tag).end == (ec->tag).begin + 4 && ((ec->tag)[0] == 's')
                    && ((ec->tag)[1] == 'a') && ((ec->tag)[2] == 'v')
                    && ((ec->tag)[3] == 'e')) {
                delay_example(ec, 0);
            }
            else
                //Correct the below line:
                params->predictions = NULL;

            //params->predictions = naive_bayes_predict(ec, thread_num, params);
        }
        else if (thread_done(thread_num)) {
            if (global.local_prediction > 0)
                shutdown(global.local_prediction, SHUT_WR);
            return NULL;
        }
        else
            ;//busywait when we have predicted on all examples but not yet trained on all.
    }
    return NULL;
}

float*
htnb_predict (example* ex, size_t thread_num, htnb_thread_params* params)
{

    size_t voteSize = observedClassDist.size();
    float classVotes[voteSize];
    size_t stride = global.stride;
    float observedClassSum = sum_of_vals(observedClassDist);
    arfheader *arfHeader = params->arfHeader;
    if (ex == NULL || arfHeader == NULL) {
        std::cerr << "Warning in htnb.cc:77, ex or arfheader can't be null"
                << std::endl;
    }
    else {
        for (size_t classIndex = 0; classIndex < voteSize; classIndex++) {
            classVotes[classIndex] = (observedClassDist[classIndex]
                    / observedClassSum);
            int attSize = arfHeader->no_of_features;
            for (size_t i = *(ex->indices.begin); i = !ex->indices.end; i++) {
                AttributeClassObserver *obs = attributeObservers[i];
                feature f = ex->atomics[i][thread_num];
                if ((obs != NULL) && !(isnan(f.x))) {
                    classVotes[classIndex]
                            *= obs->probabilityOfAttributeValueGivenClass(f.x,
                                    classIndex);
                }
            }
        }
    }
    return classVotes;
}

void
htnb_train_on_example (example* ex, arfheader *arfHeader, size_t thread_num)
{
    for (size_t i = *(ex->indices.begin); (i = !(ex->indices.end)); i++) {
        AttributeClassObserver *obs = attributeObservers[i];
        feature f = ex->atomics[i][thread_num];
        fType type;

        if (obs == NULL) {
            if (!(arfHeader->features).empty()) {
                type = arfHeader->features[f.weight_index].type;
            }
            if (type == NUMERIC) {
                NumAttrObserver *numAttObs = new NumAttrObserver(
                        arfHeader->no_of_categories);
                obs = numAttObs;
            }
            else if (type == NOMINAL) {
                NomAttrObserver *nomAttObs = new NomAttrObserver();
                obs = nomAttObs;
            }
            else {
                std::cerr << "Unsupported type" << std::endl;
            }
        }
        label_data *ld = (label_data *) (ex->ld);
        obs->observeAttributeClass(f.x, ld->label, ex->global_weight);
    }
}

static pthread_t* threads;
static htnb_thread_params** passers;
static size_t num_threads;

void
setup_htnb (htnb_thread_params t)
{
    num_threads = t.thread_num;
    threads = (pthread_t*) c_calloc(num_threads, sizeof(pthread_t));
    passers = (htnb_thread_params**) c_calloc(num_threads,
            sizeof(htnb_thread_params*));
    for (size_t i = 0; i < num_threads; i++) {
        passers[i] = (htnb_thread_params*) c_calloc(1,
                sizeof(htnb_thread_params));
        *(passers[i]) = t;
        passers[i]->thread_num = i;
        pthread_create(&threads[i], NULL, htnb_thread, (void *) passers[i]);
    }
}

void
destroy_htnb()
{
    for (size_t i = 0; i < num_threads; i++) {
        pthread_join(threads[i], NULL);
        c_free(passers[i]);
    }
    c_free(threads);
    c_free(passers);
}

static void
finish_example (example* ec)
{
    pthread_mutex_lock(&ec->lock);
    if (--ec->threads_to_finish == 0) {
        pthread_mutex_unlock(&ec->lock);
        //    output_and_account_example(ec);
        free_example(ec);
    }
    else
        pthread_mutex_unlock(&ec->lock);
}
