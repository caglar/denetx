/*
 * numattrobs.h
 *
 *  Created on: Apr 26, 2011
 *      Author: caglar
 */

#ifndef NUMATTROBS_H_
#define NUMATTROBS_H_

#include <cfloat>
#include <stdio.h>

#include <algorithm>
#include <vector>

#include "multiclass/dvec.h"
#include "multiclass/estimator.h"
#include "multiclass/infogain.h"
#include "multiclass/split_test.h"
#include "multiclass/flin.h"

#include "utils.h"
#include "float_cmp.h"

using est::NormalEstimator;
using std::vector;

//Gaussian Numeric Attribute Observer
class NumAttrObserver : public AttributeClassObserver
{
private:
    DVec minValueObservedPerClass;
    DVec maxValueObservedPerClass;
    vector<NormalEstimator *> attValDistPerClass;
    unsigned int mNoOfClasses;
    unsigned int numBins;

public:

    explicit
    NumAttrObserver(unsigned int noOfClasses) :
        mNoOfClasses(noOfClasses), numBins(100)
    {
        attValDistPerClass.resize(mNoOfClasses);
    }

    NumAttrObserver(unsigned int noOfClasses, unsigned int numBins_) :
        mNoOfClasses(noOfClasses), numBins(numBins_)
    {
        attValDistPerClass.resize(mNoOfClasses);
    }

    ~NumAttrObserver()
    {
        minValueObservedPerClass.empty();
        maxValueObservedPerClass.empty();
        attValDistPerClass.empty();
    }

    vector<NormalEstimator *>
    getAttValDistPerClass()
    {
        printf("Inside class size: %d\n", attValDistPerClass.size());
        return this->attValDistPerClass;
    }

    void
    addNewValDist(NormalEstimator *valDist, int classNo)
    {
        attValDistPerClass[classNo] = valDist;
    }

    /*
     *@param attVal the attribute's float value
     *@param classVal the label that the example belongs to
     *@param the weight of the example
     */
    void
    observeAttributeClass(float attVal, int classVal, float weight)
    {
        printf("Class val is: %d\n", classVal);
        if (attValDistPerClass[classVal] == NULL) {
            float defaultPrecision = 0.001;
            attValDistPerClass[classVal]
                    = new NormalEstimator(defaultPrecision);
            minValueObservedPerClass[classVal] = attVal;
            maxValueObservedPerClass[classVal] = attVal;
        }
        else {
            if (attVal < minValueObservedPerClass[classVal]) {
                minValueObservedPerClass[classVal] = attVal;
            }
            if (attVal > maxValueObservedPerClass[classVal]) {
                maxValueObservedPerClass[classVal] = attVal;
            }
        }
        attValDistPerClass[classVal]->addValue(attVal, weight);
    }

    float
    probabilityOfAttributeValueGivenClass(float attVal, int classVal)
    {
        NormalEstimator *obs = attValDistPerClass[classVal];
        return (obs != NULL ? obs->getProbability(attVal) : 0);
    }

    float *
    getSplitSuggestions()
    {
        DVec suggestedSplitValues;
        float minValue = FLT_MIN;
        float maxValue = FLT_MAX;

        minValue = min_val(minValueObservedPerClass);
        maxValue = max_val(maxValueObservedPerClass);

        if (minValue < FLT_MAX) {
            float range = maxValue - minValue;
            for (size_t i = 0; i < numBins; i++) {
                float splitVal = range / (this->numBins + 1) * (i + 1);
                if (definitelyLessThan(splitVal, maxValue)
                        && definitelyGreaterThan(splitVal, minValue)) {
                    suggestedSplitValues[i] = splitVal;
                }
            }
        }
        size_t splitValSize = suggestedSplitValues.size();
        float *suggestions = (float *) c_malloc(sizeof(float) * splitValSize);
        std::copy(suggestedSplitValues.begin(), suggestedSplitValues.end(),
                suggestions);
        return suggestions;
    }

    Matrix_t
    getClassDistsResultingFromBinarySplit(float splitValue)
    {
        DVec lhsDist;
        DVec rhsDist;
        float precision = 0.001;
        size_t attValSize = attValDistPerClass.size();

        for (size_t i = 0; i < attValSize; i++) {
            NormalEstimator *nEstimator = new NormalEstimator(precision);

            if (nEstimator != NULL) {
                if (splitValue < minValueObservedPerClass[i]) {
                    add_to_val(i, rhsDist, nEstimator->getTotalObsWeights());
                }
                else if (splitValue >= maxValueObservedPerClass[i]) {
                    add_to_val(i, lhsDist, nEstimator->getTotalObsWeights());
                }
                else {
                    float equalToWeight = nEstimator->getEqualToWeight(
                            splitValue);
                    float lessThanWeight = nEstimator->getLessThanWeight(
                            splitValue, equalToWeight);
                    float greaterThanWeight = nEstimator->getGreaterThanWeight(
                            equalToWeight, lessThanWeight);
                    add_to_val(i, lhsDist, equalToWeight + lessThanWeight);
                    add_to_val(i, rhsDist, greaterThanWeight);
                }
            }
        }

        const size_t lhsDistSize = lhsDist.size();
        //const size_t rhsDistSize = rhsDist.size();
        Matrix_t resultMatrix(boost::extents[2][lhsDistSize]);
        lhsDist = resultMatrix[0];
        rhsDist = resultMatrix[1];

        /*
         float *lhsDistArr = (float *) c_malloc(lhsDistSize * sizeof(float));
         float *rhsDistArr = (float *) c_malloc(rhsDistSize * sizeof(float));

         float **resultArray = new float*[lhsDistSize];

         for (size_t siz = 0; siz < lhsDistSize; siz++)
         resultArray[siz] = new float[rhsDistSize];

         memmove(resultArray[0], lhsDistArr, lhsDistSize);
         memmove(resultArray[1], rhsDistArr, rhsDistSize);
         */
        return resultMatrix;
    }

    AttrSplitSuggestion *
    getBestEvaluatedSplitSuggestion(float preSplitDist[], size_t splitTestSize)
    {
        AttrSplitSuggestion *bestSuggestion = NULL;
        float* suggestedSplitValues = getSplitSuggestions();

        for (size_t i = 0; i < splitTestSize; i++) {
            float splitVal = suggestedSplitValues[i];
            Matrix_t postSplitDist = getClassDistsResultingFromBinarySplit(
                    splitVal);
            float merit = getMeritOfSplit(preSplitDist, postSplitDist,
                    splitTestSize);

            if ((bestSuggestion != NULL) || (merit > bestSuggestion->merit)) {
                NumericSplitTest numSplitTest;
                bestSuggestion = new AttrSplitSuggestion(postSplitDist,
                        &numSplitTest, merit);
            }
        }
        return bestSuggestion;
    }
};
#endif /* NUMATTROBS_H_ */
