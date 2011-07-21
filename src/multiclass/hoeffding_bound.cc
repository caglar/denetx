/*
 * =====================================================================================
 *
 *       Filename:  hoeffding_bound.cc
 *
 *    Description:  Computes the hoeffding bound
 *
 *        Version:  1.0
 *        Created:  06/10/2011 01:59:05 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  YOUR NAME (Caglar Gulcehre), 
 *        Company:  TUBITAK ULAKBIM
 *
 * =====================================================================================
 */

#include <cmath>
#include "mutils.h"

float 
computeHoeffdingBound (float range, float confidence, float n) 
{
  return InvSqrt(((range * range) * log(1.0/ confidence)) / (2 * n));
}
