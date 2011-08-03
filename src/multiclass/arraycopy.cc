/*
 * arraycopy.cc
 *
 *  Created on: Apr 12, 2011
 *      Author: caglar
 */
#include <cstring>
#include "multiclass/arraycopy.h"

void
arrcpy::dArraycpy(float dst[], float src[], size_t size)
{
    if (size > 0 && sizeof(dst) >= 0 && sizeof(src) >= 0) {
        memcpy(dst, src, size);
    }
}

void
arrcpy::dArraycopy(float src[], int srcPos, float dest[], int destPos,
        size_t size)
{
    if (size > 0 && sizeof(dest) >= 0 && sizeof(src) >= 0) {
        arrcpy::dArraycpy(dest + destPos, src + srcPos, size);
    }
}
