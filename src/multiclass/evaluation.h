#ifndef EVALUATION_H
#define EVALUATION_H

#include <stdint.h>

#include "multiclass/dvec.h"

class Evaluation{
protected:
    int mNumClasses;

    uint32_t mWeightIncorrect;
    uint32_t mWeightCorrect;
    uint32_t mWeightUnclassified;
    uint32_t mTotalClassifiedWeight;
    uint32_t mUnclassifiedWeight;

    float mError;
    float mSumError;
    float mSumSqrError;

    DVec mPredictionDist;

    DVec makeDistribution(int predictedClass);
    void updateNumericScores( int classVal, int predictedClassVal, DVec predictionDist);

public:
    Evaluation() : mNumClasses(0), mWeightIncorrect(0), mWeightCorrect(0), mWeightUnclassified(0), mError(0), mSumError(0), mSumSqrError(0) {}
    void evaluateModel(example *ex, DVec predictionDist);
    double getErrorRate();
    double getAccuracy();
    double getRMSE();
    void resetEvaluation();
    void scaleValues(int scaleVal);
};

#endif
