/*
 * dvec.h
 *
 *  Created on: Apr 26, 2011
 *      Author: caglar
 */

#ifndef DVEC_H_
#define DVEC_H_
#include <boost/multi_array.hpp>

typedef boost::multi_array<float,2> Matrix_t;
typedef boost::multi_array<float, 1> DVec;

void
add_to_val(int i, DVec& dVec, float val);

void
copy_array_elements(DVec &dvec, const float *val, size_t array_size);

void
copy_dvec_to_array(const DVec &dvec, float *val);

template <typename T>
void
copy_stl_ds_to_dvec (DVec &dvec, const T &vec);

template <typename T>
void
copy_dvec_to_stl_ds (const DVec &dvec, T &vec);

size_t
max_index(const DVec& dvec);

float
max_val(const DVec& dvec);

size_t
min_index(const DVec& dvec);

float
min_val(const DVec& dvec);

float
sum_of_vals(DVec& dvec);

void
scale_vals(DVec& dvec, float sval);

void
sumDVecs(DVec first, DVec &second);

void
normalize(DVec& dvec);

int
get_num_non_zero (const DVec & dvec);

#endif /* DVEC_H_ */
