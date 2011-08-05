#include "multiclass/evaluation.h"

DVec
Evaluation::makeDistribution(int predictedClass)
{
    DVec dist(boost::extents[this->mNumClasses]);

    for (int i = 0; i < this->mNumClasses; i++) {
        if (i == predictedClass) {
            dist[i] += predictedClass;
        } else {
            dist[i] = 0;
        }
    }
    return dist;
}

void
Evaluation::updateNumericScores(int classVal, int predictedClassVal, DVec predictionDist)
{
    float diff = 0;
    float sumErr = 0;
    float sumSqrErr = 0;
    DVec actualDist = this->makeDistribution();
    for (int i = 0; i < this->mNumClasses; i++)
    {
        diff = predictionDist[i] - actualDist[i];
        sumErr += diff;
        sumSqrErr += diff * diff;
    }
    mSumError += (weight * sumErr )/ mNumClasses;
    mSumSqrError += (weight * sumSqrErr )/ mNumClasses;
}

void
Evaluation::evaluateModel(example *ex, DVec predictedClassDist)
{
    double weight = ((label_data *) ex->ld)->weight;
    int classVal = static_cast<int> (((label_data *) ex->ld)->label);
    int predictedClassValue = max_index(ex);

    if (predictedClassValue == classVal) {
        mWeightIncorrect += weight;
    }
    else if (classVal >= 0) {
        mWeightCorrect += weight;
    }
    else if (classVal < 0) {
        mWeightUnclassified += weight;
    }

    mTotalClassifiedWeight += weight;

}

double
Evaluation::getErrorRate()
{
    return (mWeightIncorrect / (mTotalClassifiedWeight));
}

double
Evaluation::getAccuracy()
{
    return (mWeightCorrect / (mTotalClassifiedWeight));
}

double
Evaluation::getRMSE()
{
    return InvSqrt(
                   mSumSqrError / (mTotalClassifiedWeight - mUnclassifiedWeight));
}

void
Evaluation::resetEvaluation()
{
    mWeightIncorrect = 0;
    mWeightCorrect = 0;
    mWeightUnclassified = 0;
    mTotalClassifiedWeight = 0;
    mUnclassifiedWeight = 0;

    mError = 0.0;
    mSumError = 0.0;
    mSumSqrError = 0.0;
}

void
Evaluation::scaleValues(int scaleVal)
{
    mWeightIncorrect /= scaleVal;
    mWeightCorrect /= scaleVal;
    mWeightUnclassified /= scaleVal;
    mTotalClassifiedWeight /= scaleVal;
    mUnclassifiedWeight /= scaleVal;
}
