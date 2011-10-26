/*
 * nomattrobs.h
 *
 *  Created on: Apr 26, 2011
 *      Author: caglar
 */

#ifndef NOMATTROBS_H_
#define NOMATTROBS_H_

#include <vector>
#include <boost/multi_array.hpp>

#include "multiclass/dvec.h"
#include "multiclass/attrobs.h"
#include "multiclass/flin.h"
#include "multiclass/infogain.h"

using std::vector;

//Nominal Attribute class observer
class NomAttrObserver : public AttributeClassObserver
{
protected:
    vector<DVec> attValDistPerClass;
    float totalWeightObserved;
    float missingWeightObserved;
    bool binaryOnly;

public:

    NomAttrObserver() :
        totalWeightObserved(0), missingWeightObserved(0), binaryOnly(false)
    {
    }

    NomAttrObserver(unsigned int noOfClasses) :
        totalWeightObserved(0), missingWeightObserved(0), binaryOnly(false)
    {
        attValDistPerClass.resize(noOfClasses);
    }

    ~NomAttrObserver()
    {
        attValDistPerClass.empty();
    }

    void
    setBinaryOnly(bool bFlag)
    {
        binaryOnly = bFlag;
    }

    bool
    isBinary()
    {
        return binaryOnly;
    }

    vector<DVec>
    getattValDistPerClass()
    {
        return attValDistPerClass;
    }

    void
    observeAttributeClass(float attVal, int classVal, float weight)
    {
        int attValInt = static_cast<int> (attVal);
        add_to_val(attValInt, attValDistPerClass[classVal], weight);
        totalWeightObserved += weight;
    }

    float
    probabilityOfAttributeValueGivenClass(float attVal, int classVal)
    {
        DVec obs = attValDistPerClass[classVal];
        return (obs[static_cast<int> (attVal)] + 1.0) / (sum_of_vals(obs) + obs.size());
    }

    float
    getTotalClassWeightsObserved()
    {
        return totalWeightObserved;
    }

    int
    getMaxAttValsObserved()
    {
        size_t maxAttValsObserved = 0;
        std::vector<DVec>::iterator it = attValDistPerClass.begin();
        for (; it != attValDistPerClass.end(); ++it) {
            if (maxAttValsObserved > attValDistPerClass.size()) {
                maxAttValsObserved = attValDistPerClass.size();
            }
        }
        return maxAttValsObserved;
    }

    Matrix_t
    getClassDistsResultingFromMultiwaySplit(int maxAttValsObserved)
    {
        DVec *resultingDists = new DVec[maxAttValsObserved];

        for (auto i = 0; i < this->attValDistPerClass.size(); i++) {
            DVec attValDist = this->attValDistPerClass[i];
            if (!attValDist.empty()) {
                for (auto j = 0; j < attValDist.size(); j++) {
                    add_to_val(i, resultingDists[j], attValDist[j]);
                }
            }
        }

        size_t ySize = resultingDists[0].size();
        Matrix_t distributions(boost::extents[maxAttValsObserved][ySize]);
        for (int i = 0; i < maxAttValsObserved; i++) {
            resultingDists[i] = distributions[i];
        }
        return distributions;
    }

    Matrix_t
    getClassDistsResultingFromBinarySplit(int valIndex)
    {
        DVec equalsDist;
        DVec notEqualDist;
        for (size_t i = 0; i < this->attValDistPerClass.size(); i++) {
            DVec attValDist = this->attValDistPerClass[i];
            if (attValDist.size() > 0) {
                for (auto j = 0; j < attValDist.size(); j++) {
                    if (static_cast<int> (j) == valIndex) {
                        add_to_val(i, equalsDist, attValDist[j]);
                    }
                    else {
                        add_to_val(i, notEqualDist, attValDist[j]);
                    }
                }
            }
        }

        size_t notEqualDistSize = notEqualDist.size();
        Matrix_t resultMatrix(boost::extents[2][notEqualDistSize]);
        resultMatrix[0] = equalsDist;
        resultMatrix[1] = notEqualDist;
        return resultMatrix;
    }

    AttrSplitSuggestion *
    getBestEvaluatedSplitSuggestion(float preSplitDist[], size_t splitTestSize)
    {
        AttrSplitSuggestion *bestSuggestion = NULL;
        int maxAttValsObserved = getMaxAttValsObserved();
        //This will only work for multiway splits:
        {
            Matrix_t postSplitDists = getClassDistsResultingFromBinarySplit(
                    maxAttValsObserved);
            float merit = getMeritOfSplit(preSplitDist, postSplitDists,
                    splitTestSize);

            if ((bestSuggestion != NULL) || (merit > bestSuggestion->merit)) {
                bestSuggestion = new AttrSplitSuggestion(postSplitDists,
                        new NominalSplitTest(false), merit);
            }
        }

        for (int i = 0; i < maxAttValsObserved; i++) {
            Matrix_t postSplitDists = getClassDistsResultingFromBinarySplit(i);
            float merit = getMeritOfSplit(preSplitDist, postSplitDists,
                    splitTestSize);
            if ((bestSuggestion != NULL) || (merit > bestSuggestion->merit)) {
                bestSuggestion = new AttrSplitSuggestion(postSplitDists,
                        new NominalSplitTest(true, i), merit);
            }
        }
        return bestSuggestion;
    }
};
#endif /* NOMATTROBS_H_ */
