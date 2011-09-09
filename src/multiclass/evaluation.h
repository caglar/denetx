#ifndef EVALUATION_H
#define EVALUATION_H

#include <stdint.h>

#include "multiclass/dvec.h"
#include "example.h"

class Evaluation
{
protected:
    int mNumClasses;

    float mWeightIncorrect;
    float mWeightCorrect;
    float mWeightUnclassified;

    float mTotalClassifiedWeight;
    float mUnclassifiedWeight;

    float mError;
    float mSumError;
    float mSumSqrError;

    DVec mPredictionDist;

    DVec
    makeDistribution(int predictedClass);

    void
    updateNumericScores(int classVal, float weight, DVec predictionDist);

public:

    Evaluation() :
        mNumClasses(0), mWeightIncorrect(0), mWeightCorrect(0),
                mWeightUnclassified(0), mError(0), mSumError(0),
                mSumSqrError(0)
    {
    }

    Evaluation(int numClasses) :
        mNumClasses(numClasses), mWeightIncorrect(0), mWeightCorrect(0),
                mWeightUnclassified(0), mTotalClassifiedWeight(0), mUnclassifiedWeight(0), mError(0), mSumError(0),
                mSumSqrError(0)
    {
    }

    void
    evaluateModel(example *ex, DVec predictionDist);

    void
    joinEvaluation(Evaluation *eval);

    float
    getWeightIncorrect();

    float
    getWeightCorrect();

    float
    getWeightUnclassified();

    float
    getTotalClassifiedWeight();

    float
    getUnclassifiedWeight();

    float
    getError();

    float
    getSumError();

    float
    getSumSqrError();

    int
    getNumClasses();

    float
    getErrorRate();

    float
    getAccuracy();

    float
    getRMSE();

    void
    resetEvaluation();

    void
    scaleValues(int scaleVal);
};

#endif
