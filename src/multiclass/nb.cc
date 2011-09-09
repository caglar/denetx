/*
 * nb.cc
 *
 *  Created on: Apr 1, 2011
 *      Author: caglar
 *
 */

#include <vector>
#include <string>
#include <fstream>
#include <float.h>
#include <netdb.h>
#include <cstring>
#include <cstdio>
#include <cstdlib>

#include <pthread.h>

#include "multiclass/nb.h"
#include "multiclass/numattrobs.h"
#include "multiclass/nomattrobs.h"
#include "multiclass/parse_nbmodel.h"
#include "multiclass/evaluation.h"

#include "parse_example.h"
#include "constant.h"
#include "sparse_dense.h"
#include "cache.h"
#include "multisource.h"
#include "simple_label.h"
#include "delay_ring.h"
#include "parse_arfheader.h"
#include "utils.h"

static void
finish_example(example* ec);

pthread_mutex_t initMutex = PTHREAD_MUTEX_INITIALIZER;

void *
nb_thread(void *in)
{
    nb_thread_params* params = static_cast<nb_thread_params*> (in);
    size_t thread_num = params->thread_num;
    example* ec = NULL;

    pthread_mutex_lock(&initMutex);
    size_t no_of_cats = params->arfHeader->no_of_categories;
    size_t no_of_feats = params->arfHeader->no_of_features;

    printf("No of cats: %u, no of feats: %u \n", no_of_cats, no_of_feats);

    if (params->vars->eval == NULL) {
        params->vars->eval = new Evaluation();
    }

    if (params->vars == NULL) {
        params->vars = (nb_vars *) c_malloc(sizeof(nb_vars));
    }

    if ((params->vars->observedClassDist.size() != no_of_cats) && (no_of_cats
            > 0)) {
        params->vars->observedClassDist.resize(boost::extents[no_of_cats]);
    }

    if (params->vars->attributeObservers.size() != no_of_feats && (no_of_feats
            > 0)) {
        params->vars->attributeObservers.resize(no_of_feats);
    }

    pthread_mutex_unlock(&initMutex);

    while (true) {
        //this is a poor man's select operation.
        if ((ec = get_delay_example(thread_num)) != NULL)//nonblocking
        {
            nb_train_on_example(ec, params->arfHeader, thread_num, params);
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
            else {
                label_data *ld = (label_data *) c_malloc(
                        sizeof(*((label_data *) ec->ld)));
                ld->label = ((label_data *) ec->ld)->label;
                ld->weight = ((label_data *) ec->ld)->weight;

                if (global.training && (ld->label != FLT_MAX)) {
                    nb_train_on_example(ec, params->arfHeader, thread_num,
                            params);

                    finish_example(ec);
                }
                else {
                    params->predictions = naive_bayes_predict(ec, thread_num,
                            params);
                    DVec predVec;
                    copy_array_elements(predVec, params->predictions, no_of_cats);
                    ((Evaluation *)params->vars->eval)->evaluateModel(ec, predVec);
                }
            }
        }
        else if (thread_done(thread_num)) {
            if (global.local_prediction > 0)
                shutdown(global.local_prediction, SHUT_WR);
            return NULL;
        }
        else
            ;
        //busywait when we have predicted on all examples but not yet trained on all.
    }
    return NULL;
}

float*
naive_bayes_predict(example* ex, size_t thread_num, nb_thread_params* params)
{
    //cout << "I am the predictor" << endl;
    size_t voteSize = params->vars->observedClassDist.size();
    float *classVotes = new float[voteSize];
    //DVec classVotes(boost::extents]);
    //  size_t stride = global.stride;
    float observedClassSum = sum_of_vals(params->vars->observedClassDist);
    arfheader *arfHeader = params->arfHeader;

    label_data *ld = (label_data *) c_malloc(sizeof(*((label_data *) ex->ld)));
    ld->label = ((label_data *) ex->ld)->label;
    ld->weight = ((label_data *) ex->ld)->weight;

    if (global.training && (ld->label != FLT_MAX)) {
        cout << "Training" << endl;
        nb_train_on_example(ex, arfHeader, thread_num, params);
    }

    if (ex == NULL || arfHeader == NULL) {
        std::cerr << "Warning in nb.cc:77, ex or arfheader can't be null"
                << std::endl;
    }
    else {
        for (size_t classIndex = 0; classIndex < voteSize; classIndex++) {
            classVotes[classIndex]
                    = (params->vars->observedClassDist[classIndex]
                            / observedClassSum);
            for (size_t *i = (ex->indices.begin); i != (ex->indices.end); i++) {
                int c = 0;
                for (feature *f = ex->subsets[*i][thread_num]; f
                        != ex->subsets[*i][thread_num + 1]; f++) {
                    AttributeClassObserver *obs =
                            params->vars->attributeObservers[c];
                    feature f = ex->atomics[*i][thread_num];
                    if ((obs != NULL) && !(isnan(f.x))) {
                        classVotes[classIndex]
                                *= obs->probabilityOfAttributeValueGivenClass(
                                        f.x, classIndex);
                    }
                    c++;
                }
            }
        }
    }
    return classVotes;
}

pthread_mutex_t trainMutex = PTHREAD_MUTEX_INITIALIZER;

void
nb_train_on_example(example* ex, arfheader *arfHeader, size_t thread_num,
        nb_thread_params* params)
{

    fType type;
    pthread_mutex_lock(&trainMutex);
    label_data *ld = (label_data *) c_malloc(sizeof(*((label_data *) ex->ld)));

    ld->label = ((label_data *) ex->ld)->label;
    ld->weight = ((label_data *) ex->ld)->weight;
    add_to_val(ld->label, params->vars->observedClassDist, ld->weight);

    params->vars->noOfObservedExamples++;
    for (size_t *i = (ex->indices.begin); i != (ex->indices.end); ++i) {
        int j = 0;
        std::cout << "Thread id: " << (unsigned long) pthread_self()
                << std::endl;
        for (feature *f = ex->subsets[*i][thread_num]; f
                != ex->subsets[*i][thread_num + 1]; f++) {
            if (!(arfHeader->features).empty())
                type = ((fType) (arfHeader->features[f->weight_index]).type);

            if (params->vars->attributeObservers[j] == NULL) {
                if (type == NUMERIC) {
                    NumAttrObserver *numAttObs = new NumAttrObserver(
                            arfHeader->no_of_categories);
                    params->vars->attributeObservers[j] = numAttObs;
                }
                else if (type == NOMINAL) {
                    NomAttrObserver *nomAttObs = new NomAttrObserver();
                    cerr << "WTF!!!" << endl;
                    params->vars->attributeObservers[j] = nomAttObs;
                }
                else {
                    std::cerr << "Unsupported type" << std::endl;
                    exit( EXIT_FAILURE);
                }
            }

            if (type == NUMERIC)
                (static_cast<NumAttrObserver *> (params->vars->attributeObservers[j]))->observeAttributeClass(
                        f->x, ld->label, ex->global_weight);
            else if (type == NOMINAL)
                (static_cast<NomAttrObserver *> (params->vars->attributeObservers[j]))->observeAttributeClass(
                        f->x, ld->label, ex->global_weight);
            j++;
        }
    }
    pthread_mutex_unlock(&trainMutex);
}

static pthread_t *threads;
static nb_thread_params **passers;
static size_t num_threads;

void
setup_nb(nb_thread_params t)
{
    //bool setupFlag = false;
    num_threads = t.thread_num;
    threads = (pthread_t *) c_calloc(num_threads, sizeof(pthread_t));
    passers = (nb_thread_params **) c_calloc(num_threads,
            sizeof(nb_thread_params *));
    std::string nbModelFile = global.nb_model_file;
    bool mFileFlag = false;
    DVec observedClassDist(boost::extents[t.arfHeader->no_of_categories]);
    vector<AttributeClassObserver *> attributeObservers(
            t.arfHeader->no_of_features);

    if (nbModelFile.size() > 0) {
        if (c_does_file_exist(nbModelFile.c_str()) && c_get_file_size(
                nbModelFile.c_str()) > 0) {
            readModelFile(observedClassDist, attributeObservers, t.arfHeader,
                    nbModelFile);
            mFileFlag = true;
        }
    }

    for (size_t i = 0; i < num_threads; i++) {
        passers[i] = (nb_thread_params *) c_calloc(1, sizeof(nb_thread_params));
        *(passers[i]) = t;
        passers[i]->thread_num = i;
        if (mFileFlag) {
            cout << " File flag: hey!" << endl;
            passers[i]->vars->attributeObservers = attributeObservers;
            passers[i]->vars->observedClassDist = observedClassDist;
        }
        pthread_create(&threads[i], NULL, nb_thread, (void *) passers[i]);
    }
}

pthread_mutex_t createModelFileMutex = PTHREAD_MUTEX_INITIALIZER;

void
destroy_nb()
{
    std::string nbModelFile = global.nb_model_file;
    Evaluation eval;
    for (size_t i = 0; i < num_threads; i++) {
        pthread_join(threads[i], NULL);
        eval.joinEvaluation(passers[i]->vars->eval);
        if (nbModelFile.size() > 0 && i == 0) {
            cout << "No of observed examples: "
                    << passers[0]->vars->noOfObservedExamples << endl;

            scale_vals(passers[0]->vars->observedClassDist,
                    passers[0]->vars->noOfObservedExamples);

            writeModelFile(passers[0]->vars->observedClassDist,
                    passers[0]->vars->attributeObservers,
                    passers[0]->arfHeader, nbModelFile);
        }
        c_free(passers[i]);
    }
    cout << "Error: " << eval.getError() << endl;
    c_free(threads);
    c_free(passers);
}

static void
finish_example(example* ec)
{
    pthread_mutex_lock(&ec->lock);
    if (--ec->threads_to_finish == 0) {
        pthread_mutex_unlock(&ec->lock);
        free_example(ec);
    }
    else
        pthread_mutex_unlock(&ec->lock);
}
