/*
 * nb.h
 *
 *  Created on: Apr 1, 2011
 *      Author: caglar
 */
#ifndef NB_H_
#define NB_H_

#include <vector>
#include <cmath>

#include "example.h"
#include "parse_regressor.h"
#include "parse_arfheader.h"
#include "parser.h"

#include "multiclass/dvec.h"
#include "multiclass/attrobs.h"
#include "multiclass/evaluation.h"

void
print_result(int f, float res, v_array<char> tag);

struct nb_vars
{
    DVec observedClassDist;
    size_t noOfObservedExamples;
    vector<AttributeClassObserver *> attributeObservers;
    float power_t;
    Evaluation *eval;
    nb_vars() :
        observedClassDist (boost::extents[2]), attributeObservers (0), power_t (0)
    {
    }

    void
    init()
    {
        power_t = 0.;
    }
};

struct nb_thread_params
{
    nb_vars* vars;
    arfheader* arfHeader;
    string rrd_file_path;
    size_t thread_num;
    size_t step_size;
    float* predictions;
};

void*
nb_thread(void *in);

float*
naive_bayes_predict(example* ex, size_t thread_num, nb_thread_params* params);

void
nb_train_on_example(example* ex, arfheader *arfHeader, size_t thread_num,
        nb_thread_params* params);

void
setup_nb(nb_thread_params t);

void
destroy_nb();

#endif /* NB_H_ */
