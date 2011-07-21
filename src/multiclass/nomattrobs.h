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

#include "dvec.h"
#include "attrobs.h"
#include "flin.h"
#include "infogain.h"

//Nominal Attribute class observer
class NomAttrObserver : public AttributeClassObserver
{
protected:
    vector<DVec> attValDistPerClass;

    float totalWeightObserved;
    float missingWeightObserved;
    bool binaryOnly;

public:

    NomAttrObserver() : totalWeightObserved(0), missingWeightObserved(0), binaryOnly(false)
    {
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

    void
    observeAttributeClass(float attVal, int classVal, float weight)
    {
        int attValInt = (int) attVal;
        DVec valDist = attValDistPerClass[classVal];
        valDist[attValInt] += weight;
        totalWeightObserved += weight;
    }

    float
    probabilityOfAttributeValueGivenClass(float attVal, int classVal)
    {
        DVec obs = attValDistPerClass[classVal];
        return (obs[(int) attVal] + 1.0) / (sum_of_vals(obs) + obs.size());
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

        //float **distributions = new float*[maxAttValsObserved];
        size_t ySize = resultingDists[0].size();
        Matrix_t distributions(boost::extents[maxAttValsObserved][ySize]);
        for (int i = 0; i < maxAttValsObserved; i++) {
            //distributions[i] = new float[resultingDists[i].size()];
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
        /*
         copy_dvec_to_marray(equalsDist, resultMatrix[0]);
         copy_dvec_to_marray(notEqualDist, resultMatrix[1]);
         */
        resultMatrix[0] = equalsDist;
        resultMatrix[1] = notEqualDist;
        /*
         float **resultMatrix = new float*[2];
         for (size_t siz = 0; siz < 2; siz++)
         resultArray[siz] = new float[notEqualDistSize];
         copy_dvec_to_array(equalsDist, resultArray[0]);
         copy_dvec_to_array(notEqualDist, resultArray[1]);
         */
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
