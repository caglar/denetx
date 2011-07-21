/*
 * statistics.cc
 *
 *  Created on: Apr 11, 2011
 *      Author: caglar
 *      This code is based on cephes: http://www.netlib.org/cephes/
 */

#include "statistics.h"
#include "arraycopy.h"
#include <cmath>
#include <stdio.h>

/** Some important constants */
static const float SQRTH = 7.07106781186547524401E-1;
static const float MACHEP = 1.11022302462515654042E-16;
static const float MAXLOG = 7.09782712893383996732E2;
static const float MINLOG = -7.451332191019412076235E2;
static const float MAXGAM = 171.624376956302725;
static const float SQTPI = 2.50662827463100050242E0;
static const float LOGPI = 1.14472988584940017414;

const static float P[] =
{ 2.46196981473530512524E-10, 5.64189564831068821977E-1,
  7.46321056442269912687E0, 4.86371970985681366614E1,
  1.96520832956077098242E2, 5.26445194995477358631E2,
  9.34528527171957607540E2, 1.02755188689515710272E3,
  5.57535335369399327526E2 };

const static float Q[] =
{  /* 1.00000000000000000000E0,*/
  1.32281951154744992508E1, 8.67072140885989742329E1,
  3.54937778887819891062E2, 9.75708501743205489753E2,
  1.82390916687909736289E3, 2.24633760818710981792E3,
  1.65666309194161350182E3, 5.57535340817727675546E2 };

const static float R[] =
{ 5.64189583547755073984E-1, 1.27536670759978104416E0,
  5.01905042251180477414E0, 6.16021097993053585195E0,
  7.40974269950448939160E0, 2.97886665372100240670E0 };

const static float S[] =
{  /* 1.00000000000000000000E0,*/
  2.26052863220117276590E0, 9.39603524938001434673E0,
  1.20489539808096656605E1, 1.70814450747565897222E1,
  9.60896809063285878198E0, 3.36907645100081516050E0 };

const static float T[] =
{ 9.60497373987051638749E0, 9.00260197203842689217E1,
  2.23200534594684319226E3, 7.00332514112805075473E3,
  5.55923013010394962768E4 };

const static float U[] =
{  /* 1.00000000000000000000E0,*/
  3.35617141647503099647E1, 5.21357949780152679795E2,
  4.59432382970980127987E3, 2.26290000613890934246E4,
  4.92673942608635921086E4 };

float
normalProbability(float a)
{
  float x, y, z;

  x = a * SQRTH;
  z = fabs(x);

  /* if( z < SQRTH ) */
  if (z < 1.0) {
    y = 0.5 + 0.5 * erf(x);
  }
  else {
    y = 0.5 * erfc(z);
    if (x > 0) {
      y = 1.0 - y;
    }
  }
  return (y);
}

/**
 * Evaluates the given polynomial of degree <tt>N</tt> at <tt>x</tt>.
 * <pre>
 *                     2          N
 * y  =  C  + C x + C x  +...+ C x
 *        0    1     2          N
 *
 * Coefficients are stored in reverse order:
 *
 * coef[0] = C  , ..., coef[N] = C  .
 *            N                   0
 * </pre>
 * In the interest of speed, there are no checks for out of bounds arithmetic.
 *
 * @param x argument to the polynomial.
 * @param coef the coefficients of the polynomial.
 * @param N the degree of the polynomial.
 */
float
polevl(float x, const float coef[], int N)
{

  float ans = coef[0];
  for (int i = 1; i <= N; i++) {
    ans = ans * x + coef[i];
  }
  return ans;
}

/**
 * Evaluates the given polynomial of degree <tt>N</tt> at <tt>x</tt>.
 * Evaluates polynomial when coefficient of N is 1.0.
 * Otherwise same as <tt>polevl()</tt>.
 * <pre>
 *                     2          N
 * y  =  C  + C x + C x  +...+ C x
 *        0    1     2          N
 *
 * Coefficients are stored in reverse order:
 *
 * coef[0] = C  , ..., coef[N] = C  .
 *            N                   0
 * </pre>
 * The function <tt>p1evl()</tt> assumes that <tt>coef[N] = 1.0</tt> and is
 * omitted from the array.  Its calling arguments are
 * otherwise the same as <tt>polevl()</tt>.
 * <p>
 * In the interest of speed, there are no checks for out of bounds arithmetic.
 *
 * @param x argument to the polynomial.
 * @param coef the coefficients of the polynomial.
 * @param N the degree of the polynomial.
 */
float
p1evl(float x, const float coef[], int N)
{
  float ans = x + coef[0];
  for (int i = 1; i < N; i++) {
    ans = ans * x + coef[i];
  }
  return ans;
}

float
erfc(float a)
{
  float p, q, x, y, z;
  if (a < 0.0)
    x = -a;
  else
    x = a;

  if (x < 1.0)
    return (1.0 - erf(a));

  z = -a * a;

  if (z < -MAXLOG) {
under: printf("erfc %d", UNDERFLOW);
       if (a < 0)
         return (2.0);
       else
         return (0.0);
  }
  z = exp(z);
  if (x < 8.0) {
    p = polevl(x, P, 8);
    q = p1evl(x, Q, 8);
  }
  else {
    p = polevl(x, R, 5);
    q = p1evl(x, S, 6);
  }
  y = (z * p) / q;
  if (a < 0)
    y = 2.0 - y;
  if (y == 0.0)
    goto under;
  return (y);
}

/*
   Exponentially scaled erfc function
   exp(x^2) erfc(x)
   valid for x > 1.
   Use with ndtr and expx2.
   */
static float
erfce(float x)
{
  float p, q;
  if (x < 8.0) {
    p = polevl(x, P, 8);
    q = p1evl(x, Q, 8);
  }
  else {
    p = polevl(x, R, 5);
    q = p1evl(x, S, 6);
  }
  return (p / q);
}

float
erf(float x)
{
  float y, z;
  if (fabs(x) > 1.0) {
    return (1.0 - erfc(x));
  }
  z = x * x;
  y = x * polevl(z, T, 4) / p1evl(z, U, 5);
  return (y);
}
