/*
 *
 * infogain.cc
 * Created on: Apr 21, 2011
 *      Author: caglar
 */

#include <boost/multi_array.hpp>
#include <cmath>

#include "multiclass/infogain.h"
#include "multiclass/mutils.h"
#include "multiclass/dvec.h" // For matrix_t

const float MINVAL = 1.6009E-16;
const float MINFRACVAL = 0.01;

/*
 * Array parameters used here should be allocated at the stack.
 */
template <size_t N>
float
getMeritOfSplit(float (&preSplitDist)[N], Matrix_t& postSplitDists)
{
  return (computeEntropy(preSplitDist) - computeEntropy(postSplitDists));
}

float
getMeritOfSplit(float preSplitDist[], Matrix_t& postSplitDists, size_t preSplitDistSize)
{
  return (computeEntropy(preSplitDist, preSplitDistSize) - computeEntropy(postSplitDists));
}

float
getRangeOfMerit(size_t N)
{
//  size_t N = arraysize(preSplitDist);
  int numClasses = (N > 2 ? N : 2);
  return log2(numClasses);
}

/*
 * This function calculates the weighted entropy.
 */
float
computeEntropy(Matrix_t dist)
{
  float totalWeight = 0.0;
  float entropy = 0.0;
  float distWeight = 0.0;
  for (size_t i = 0; i < dist.num_dimensions(); i++) {
    distWeight = fsum(dist[i]);
    entropy += distWeight * computeEntropy(dist[i]);
    totalWeight += distWeight;
  }
  return entropy / totalWeight;
}

float
computeEntropy(boost::multi_array<float,1>dist)
{
  float entropy = 0.0;
  float sum = 0.0;
  size_t N = dist.size();
  for (size_t i = 0; i < N; i++) {
    if (dist[i] > MINVAL) {
      entropy -= dist[i] * log2(dist[i]);
      sum += dist[i];
    }
  }
  return sum > 0.0 ? (entropy + sum * log2(sum)) / sum : 0.0;
}

template<size_t N>
float
computeEntropy(float (&dist)[N])
{
 // size_t N = arraysize(N);
  float entropy = 0.0;
  float sum = 0.0;
  for (size_t i = 0; i < N; i++) {
    if (dist[i] > MINVAL) {
      entropy -= dist[i] * log2(dist[i]);
      sum += dist[i];
    }
  }
  return sum > 0.0 ? (entropy + sum * log2(sum)) / sum : 0.0;
}

float
computeEntropy(float dist[], size_t N)
{
 // size_t N = arraysize(N);
  float entropy = 0.0;
  float sum = 0.0;
  for (size_t i = 0; i < N; i++) {
    if (dist[i] > MINVAL) {
      entropy -= dist[i] * log2(dist[i]);
      sum += dist[i];
    }
  }
  return sum > 0.0 ? (entropy + sum * log2(sum)) / sum : 0.0;
}

float
numSubsetsGreaterThanFrac(Matrix_t distributions, float minFrac)
{
  float totalWeight = 0.0;
  size_t first_dim_size = distributions.num_dimensions();
  float distSums[first_dim_size];

  for (size_t i = 0; i < first_dim_size; i++) {
    distSums[i] = fsum(distributions[i]);
    totalWeight += distSums[i];
  }

  int numGreater = 0;
  for (size_t i = 0; i < first_dim_size; i++) {
    float frac = distSums[i] / totalWeight;
    if (frac > minFrac) {
      numGreater++;
    }
  }
  return numGreater;
}
