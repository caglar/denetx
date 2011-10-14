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

#include <math.h>

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

int no_of_preds = 0;

static void
finish_example(example* ec);

pthread_mutex_t initMutex = PTHREAD_MUTEX_INITIALIZER;

/*
 *NB thread function.
 *Manages the multithreaded nb algorithm.
 *@param in The threads parameter
 */
void *
nb_thread(void *in)
{

  nb_thread_params* params = static_cast<nb_thread_params*> (in);
  size_t thread_num = params->thread_num;
  example* ec = NULL;

  size_t no_of_cats = params->arfHeader->no_of_categories;
  size_t no_of_feats = params->arfHeader->no_of_features;

  printf("No of cats: %u, no of feats: %u \n", no_of_cats, no_of_feats);

  if (params->vars->eval == NULL) {
    params->vars->eval = new Evaluation();
  }

  if (params->vars == NULL) {
    params->vars = new nb_vars();
  }

  if ((params->vars->observedClassDist.size() != no_of_cats) && (no_of_cats
                                                                 > 0)) {
    params->vars->observedClassDist.resize(boost::extents[no_of_cats]);
  }

  if (params->vars->attributeObservers.size() != no_of_feats && (no_of_feats
                                                                 > 0)) {
    params->vars->attributeObservers.resize(no_of_feats);
  }

  while (true) {
    cout << "Thread num: " << thread_num << endl;
    // this is a poor man's select operation.
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
        label_data ld;
        ld.label = (static_cast<label_data *> (ec->ld))->label;
        ld.weight = (static_cast<label_data *> (ec->ld))->weight;

        if (global.training && (ld.label != FLT_MAX)) {
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
      ;//busywait when we have predicted on all examples but not yet trained on all.
  }
  return NULL;
}

float*
naive_bayes_predict(example* ex, size_t thread_num, nb_thread_params* params)
{
  no_of_preds++;

  cout << "I am the predictor " << no_of_preds << endl;

  size_t voteSize = params->vars->observedClassDist.size() - 1;
  float *classVotes = new float[voteSize];

  float observedClassSum = sum_of_vals(params->vars->observedClassDist);
  arfheader *arfHeader = params->arfHeader;

  label_data ld;
  ld.label = (static_cast<label_data *> (ex->ld))->label;
  ld.weight = (static_cast<label_data *> (ex->ld))->weight;

  if (global.training && (ld.label != FLT_MAX)) {
    cout << "Predictor is in training" << endl;
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
  fType type = UNKNOWN;

  label_data ld;
  ld.label = (static_cast<label_data *> (ex->ld))->label;
  ld.weight = (static_cast<label_data *> (ex->ld))->weight;

  params->vars->noOfObservedExamples++;
  params->vars->observedClassDist[ld.label]++;

  for (size_t *i = (ex->indices.begin); i != (ex->indices.end); ++i) {
    //Count the number of features
    int fCount = 0;

    for (feature *f = ex->subsets[*i][thread_num];
         (f != ex->subsets[*i][thread_num + 1] && fCount < arfHeader->no_of_features);
         f++, fCount++) {
      if (!(arfHeader->features).empty())
        type = ((fType) (arfHeader->features[f->weight_index]).type);

      if (params->vars->attributeObservers[fCount] == NULL) {
        if (type == NUMERIC) {
          params->vars->attributeObservers[fCount] = new NumAttrObserver(
                                                                         arfHeader->no_of_categories);
        }
        else if (type == NOMINAL) {
          params->vars->attributeObservers[fCount] = new NomAttrObserver(
                                                                         arfHeader->no_of_categories);
        }
        else {
          std::cerr << "Unsupported type" << std::endl;
          exit (EXIT_FAILURE);
        }
      }
      if (type == NUMERIC)
        (dynamic_cast<NumAttrObserver *> (params->vars->attributeObservers[fCount]))->observeAttributeClass (
                                                                                                             f->x, ld.label, ex->global_weight);
      else if (type == NOMINAL)
        (dynamic_cast<NomAttrObserver *> (params->vars->attributeObservers[fCount]))->observeAttributeClass (
                                                                                                             f->x, ld.label, ex->global_weight);
    }
  }
  //pthread_mutex_unlock (&trainMutex);
}

static pthread_t *threads;
static nb_thread_params **passers;
static size_t num_threads;

void
setup_nb (nb_thread_params t)
{
  num_threads = t.thread_num;

  threads = new pthread_t[num_threads];
  passers = new nb_thread_params*[num_threads];

  std::string nbModelFile = global.nb_model_file;
  bool mFileFlag = false;
  DVec observedClassDist (boost::extents[t.arfHeader->no_of_categories]);
  vector<AttributeClassObserver *> attributeObservers(
                                                      t.arfHeader->no_of_features);

  if (nbModelFile.size() > 0) {
    if (c_does_file_exist(nbModelFile.c_str()) && c_get_file_size(
                                                                  nbModelFile.c_str()) > 0) {
      readModelFile(observedClassDist,
                    attributeObservers,
                    t.arfHeader,
                    nbModelFile);
      mFileFlag = true;
    }
  }

  for (size_t i = 0; i < num_threads; i++) {
    passers[i] = new nb_thread_params[1];
    *(passers[i]) = t;
    passers[i]->thread_num = i;
    if (mFileFlag) {
      passers[i]->vars->attributeObservers = attributeObservers;
      passers[i]->vars->observedClassDist = observedClassDist;
    }
    pthread_create(&threads[i], NULL, nb_thread, (void *) passers[i]);
  }
}

//pthread_mutex_t createModelFileMutex = PTHREAD_MUTEX_INITIALIZER;
void 
joinThreadData(nb_vars *tvars, arfheader *arfHeader) {
  for (size_t i = 0; i < num_threads; i++) {
    if (passers[i]->vars>noOfObservedExamples > 0) {
      if (tvars->observedClassDist.size() < (passers[i]->vars->observedClassDist).size()) {
        tvars->observedClassDist.resize(boost::extents[(passers[i]->vars->attributeObservers).size()]);
      }
      if (tvars->attributeObservers.size() < (passers[i]->vars->attributeObservers).size()) {
        tvars->attributeObservers.resize((passers[i]->vars->attributeObservers).size());
      }
      for (size_t  j = 0; j < tvars->observedClassDist.size(); j++) {
        tvars->observedClassDist[j] += passers[i]->vars->observedClassDist[j];

      }
      for (size_t j = 0; j < tvars->attributeObservers.size(); j++) {
        if (arfHeader->features[j].type == NUMERIC) {
          NumAttrObserver numAttrObs = *(dynamic_cast<NumAttrObserver *>(passers[i]->vars->attributeObservers[j]));
          
        }
        else if (arfHeader->features[j].type == NOMINAL) {
           NomAttrObserver nomAttrObs = *(dynamic_cast<NomAttrObserver *>(passers[i]->vars->attributeObservers[j]));

        }
      }
    }
  }
}

void
destroy_nb()
{
  std::string nbModelFile = global.nb_model_file;
  Evaluation eval;
  nb_vars merged_tdata;
  for (size_t i = 0; i < num_threads; i++) {
    cout << "Thread " << i << " exiting" << endl;
    pthread_join(threads[i], NULL);
    eval.joinEvaluation(passers[i]->vars, passers[i]->arfHeader);
    if (nbModelFile.size() > 0 && i == 0) {
      cout << "No of observed examples: "
        << passers[0]->vars->noOfObservedExamples << endl;

      scale_vals (passers[0]->vars->observedClassDist,
                  passers[0]->vars->noOfObservedExamples);

      writeModelFile (passers[0]->vars->observedClassDist,
                      passers[0]->vars->attributeObservers,
                      passers[0]->arfHeader, 
                      nbModelFile);
    }
    c_free(passers[i]);
  }
  if (eval.getError()) {
    cout << "Error: " << eval.getError() << endl;
  }
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
