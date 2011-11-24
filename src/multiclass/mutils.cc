/*
 * mutils.cc
 *
 *  Created on: Apr 22, 2011
 *      Author: caglar
 */
#include "multiclass/mutils.h"

template<size_t N>
inline float
fsum(float(&vec)[N])
{
  float sum = 0.0;
  for (int i = 0; i < N; i++) {
    sum += vec[i];
  }
  return sum;
}

float
fsum(Vector_t vec)
{
  int N = vec.size();
  float sum = 0.0;
  for (int i = 0; i < N; i++) {
    sum += vec[i];
  }
  return sum;
}

int
fmax_idx(float *vals, size_t a_size)
{
  float max = vals[0];
  int ret_idx = 0;
  if (a_size >= 2)
  {
    for (int i = 1; i < a_size; i++) {
      if (fmaxf(max, vals[i]) != max){
        max = vals[i];
        ret_idx = i;
      }
    }
  }
  return ret_idx;
}

/*
float
InvSqrt(float x)
{
  float xhalf = 0.5f * x;
  unsigned int i = *(unsigned int*) &x; // store floating-point bits in integer
  i = 0x5f3759d5 - (i >> 1); // initial guess for Newton's method
  x = *(float*) &i; // convert new bits into float
  x = x * (1.5f - xhalf * x * x); // One round of Newton's method
  return x;
}
*/
