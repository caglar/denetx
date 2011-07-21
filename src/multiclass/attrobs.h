/*
 * attrobs.h
 *
 *  Created on: Apr 27, 2011
 *      Author: caglar
 */

#ifndef ATTROBS_H_
#define ATTROBS_H_

#include "attrsplitsuggest.h"

class AttributeClassObserver
{
public:
    virtual
    ~AttributeClassObserver()
    {
    }

    virtual void
    observeAttributeClass(float attVal, int classVal, float weight) = 0;

    virtual float
    probabilityOfAttributeValueGivenClass(float attVal, int classVal) = 0;

    virtual AttrSplitSuggestion
            *
            getBestEvaluatedSplitSuggestion(float preSplitTest[],
                    size_t splitTestSize) = 0;
};
#endif /* ATTROBS_H_ */
