/*
 * infogain.h
 *
 *  Created on: Apr 21, 2011
 *      Author: caglar
 */

#ifndef INFOGAIN_H_
#define INFOGAIN_H_

#include <cstring>
#include "multiclass/flin.h"

template <typename T, size_t N>
inline char (&ArraySizeHelper(T (&array)[N]))[N];
#define arraysize(array) (sizeof(ArraySizeHelper(array)))

template <size_t N>
float
getMeritOfSplit(float (&preSplitDist)[N], Matrix_t& postSplitDists);

float
getMeritOfSplit(float preSplitDist[], Matrix_t& postSplitDists, size_t preSplitDistSize);

float
getRangeOfMerit(size_t N);

float
computeEntropy(Matrix_t dist);

template<size_t N>
float
computeEntropy(float dist[N]);

float
computeEntropy(float dist[], size_t N);

float
computeEntropy(boost::multi_array<float,1>dist);

float
numSubsetsGreaterThanFrac(Matrix_t distributions, float minFrac);

#endif /* INFOGAIN_H_ */
