/*
 * arraycopy.h
 *
 *  Created on: Apr 12, 2011
 *      Author: caglar
 */

#ifndef ARRAYCOPY_H_
#define ARRAYCOPY_H_
#include <cstddef>

namespace arrcpy
{
    void
    dArraycpy(float dst[], float src[], size_t size);
    void
            dArraycopy(float src[], int srcPos, float dest[], int dstPos,
                    size_t size);
}
#endif /* ARRAYCOPY_H_ */
