/*
 * mutils.h
 *
 *  Created on: Apr 22, 2011
 *      Author: caglar
 */

#ifndef MUTILS_H_
#define MUTILS_H_
#include <cstdlib>
#include "multiclass/flin.h"

template<size_t N>
inline float
fsum(float(&vec)[N]);

//float
//fsum(Vector_t vec);
float
fsum(Vector_t vec);

/*
float
InvSqrt(float x);
*/
#endif /* MUTILS_H_ */
