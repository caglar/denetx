#include "float_cmp.h"
#include <cmath>
#include <limits>

using std::fabs;

bool
approximatelyEqual(float a, float b)
{
  return fabs(a - b) <= ( (fabs(a) < fabs(b) ? fabs(b) : fabs(a)) * std::numeric_limits<float>::epsilon());
}

bool
essentiallyEqual(float a, float b)
{
  return fabs(a - b) <= ( (fabs(a) > fabs(b) ? fabs(b) : fabs(a)) * std::numeric_limits<float>::epsilon());
}

bool 
definitelyGreaterThan(float a, float b)
{
  return (a - b) > ( (fabs(a) < fabs(b) ? fabs(b) : fabs(a)) * std::numeric_limits<float>::epsilon());
}

bool
definitelyLessThan(float a, float b)
{
  return (b - a) > ( (fabs(a) < fabs(b) ? fabs(b) : fabs(a)) * std::numeric_limits<float>::epsilon());
}
