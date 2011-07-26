/*
 * dvec.cc
 *
 *  Created on: Apr 26, 2011
 *      Author: caglar
 */

#include <cstdlib>
#include <cfloat>

#include <iostream>
#include <algorithm>
#include <map>
#include <boost/multi_array.hpp>

#include "dvec.h"
#include "../float_cmp.h"

using std::copy;
using std::max_element;
using std::min_element;

bool
val_cmp(const DVec::value_type &i1, const DVec::value_type &i2);

void
copy_array_elements(DVec &dvec, const float *val, size_t array_size)
{
    for (size_t i = 0; i < array_size; i++) {
        dvec[i] = val[i];
    }
}

void
copy_dvec_to_array(const DVec &dvec, float *val)
{
    copy(dvec.begin(), dvec.end(), val);
}

template<typename T>
    void
    copy_dvec_to_stl_ds(const DVec &dvec, T &vec)
    {
        copy(dvec.begin(), dvec.end(), vec.begin());
    }

template<typename T>
    void
    copy_stl_ds_to_dvec(DVec &dvec, const T &vec)
    {
        copy(vec.begin(), vec.end(), dvec.begin());
    }

bool
val_cmp(const DVec::value_type& i1, const DVec::value_type& i2)
{
    return i1 < i2;
}

size_t
max_index(const DVec& dvec)
{
    size_t dvec_size = dvec.size();
    size_t max_idx = 0;
    if (dvec_size > 0) {
        float max = dvec[0];
        for (size_t i = 1; i < dvec_size; i++) {
            if (definitelyGreaterThan(dvec[i], max)) {
                max = dvec[i];
                max_idx = i;
            }
        }
    }
    return max_idx;
}

float
max_val(const DVec& dvec)
{
    return *max_element(dvec.begin(), dvec.end(), val_cmp);
}

size_t
min_index(const DVec& dvec)
{
    size_t dvec_size = dvec.size();
    size_t min_idx = 0;
    if (dvec_size > 0) {
        float min = dvec[0];
        for (size_t i = 1; i < dvec_size; i++) {
            if (definitelyLessThan(dvec[i], min)) {
                min = dvec[i];
                min_idx = i;
            }
        }
    }
    return min_idx;
}

float
min_val(const DVec& dvec)
{
    return *min_element(dvec.begin(), dvec.end(), val_cmp);
}

void
add_to_val(int i, DVec& dVec, float val)
{
    dVec[i] += val;
}

float
sum_of_vals(DVec& dvec)
{
    float sum = 0.0;
    for (auto i = 0; i < (dvec.size()); ++i) {
        sum += dvec[i];
    }
    return sum;
}

void
scale_vals(DVec& dvec, float sval)
{
    if (sval != 0.0) {
        for (auto i = 0; i < dvec.size(); i++) {
            dvec[i] = (dvec[i] / sval);
        }
    }else
        perror("***Scaling value shouldn't be 0***\n");
}

int
get_num_non_zero(const DVec & dvec)
{
    int noOfNonZero = 0;
    for (auto i = 0; i < dvec.size(); i++) {
        if (dvec[i] != 0.0) {
            noOfNonZero++;
        }
    }
    return noOfNonZero;
}

void
normalize(DVec& dvec)
{
    if (!dvec.empty()) {
        float sum = sum_of_vals(dvec);
        if (sum != 0) {
            scale_vals(dvec, sum);
        }
    }
}
