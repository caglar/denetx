/*
 * htnb.h
 *
 *  Created on: Apr 1, 2011
 *      Author: caglar
 */

#ifndef HTNB_H_
#define HTNB_H_

#include <math.h>

#include "example.h"
#include "parser.h"
#include "parse_arfheader.h"

struct htnb_vars
{
  float power_t;
  htnb_vars(){};
  void init()
  {
    power_t = 0.;
  }
};

struct htnb_thread_params
{
  htnb_vars* vars;
  arfheader* arfHeader;
  size_t thread_num;
  float* predictions;
};

void*
htnb_thread(void *in);

float*
htnb_predict(example* ex, size_t thread_num, htnb_thread_params* params);

void
htnb_train_on_example(example* ex, arfheader *arfHeader, size_t thread_num);

void
setup_htnb(htnb_thread_params t);

void
destroy_htnb();


#endif /* HTNB_H_ */
