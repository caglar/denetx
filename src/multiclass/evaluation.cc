#include "multiclass/evaluation.h"
#include "multiclass/dvec.h"
#include "gd.h"

DVec
Evaluation::makeDistribution(int predictedClass)
{
    DVec dist(boost::extents[this->mNumClasses]);

    for (int i = 0; i < this->mNumClasses; i++) {
        if (i == predictedClass) {
            dist[i] += predictedClass;
        }
        else {
            dist[i] = 0;
        }
    }
    return dist;
}

void
Evaluation::updateNumericScores(int classVal,
        float weight, DVec predictionDist)
{
    float diff = 0;
    float sumErr = 0;
    float sumSqrErr = 0;
    DVec actualDist = this->makeDistribution(classVal);

    for (int i = 0; i < this->mNumClasses; i++) {
        diff = predictionDist[i] - actualDist[i];
        sumErr += diff;
        sumSqrErr += diff * diff;
    }

    mSumError += (weight * sumErr) / mNumClasses;
    mSumSqrError += (weight * sumSqrErr) / mNumClasses;
}

void
Evaluation::evaluateModel(example *ex, DVec predictedClassDist)
{
    float weight = ((label_data *) ex->ld)->weight;
    int classVal = static_cast<int> (((label_data *) ex->ld)->label);
    int predictedClassValue = max_index(predictedClassDist);

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
    updateNumericScores(classVal, weight,
            predictedClassDist);
}

void
Evaluation::joinEvaluation(Evaluation *eval)
{

    this->mNumClasses = eval->getNumClasses();
    this->mTotalClassifiedWeight += eval->getTotalClassifiedWeight();
    this->mUnclassifiedWeight += eval->getUnclassifiedWeight();
    this->mWeightCorrect += eval->getWeightCorrect();
    this->mWeightIncorrect += eval->getWeightIncorrect();
    this->mWeightUnclassified += eval->getWeightUnclassified();

    this->mError += eval->getError();
    this->mSumError += eval->getSumError();
    this->mSumSqrError += eval->getSumSqrError();
}

float
Evaluation::getWeightIncorrect()
{
    return this->mWeightIncorrect;
}

float
Evaluation::getWeightCorrect()
{
    return this->mWeightCorrect;
}

float
Evaluation::getWeightUnclassified()
{
    return this->mWeightUnclassified;
}

float
Evaluation::getTotalClassifiedWeight()
{
    return this->mTotalClassifiedWeight;
}

float
Evaluation::getUnclassifiedWeight()
{
    return this->mUnclassifiedWeight;
}

float
Evaluation::getError()
{
    return this->mError;
}

float
Evaluation::getSumError()
{
    return this->mSumError;
}

float
Evaluation::getSumSqrError()
{
    return this->mSumSqrError;
}

int
Evaluation::getNumClasses()
{
    return this->mNumClasses;
}

float
Evaluation::getErrorRate()
{
    return (mWeightIncorrect / (mTotalClassifiedWeight));
}

float
Evaluation::getAccuracy()
{
    return (mWeightCorrect / (mTotalClassifiedWeight));
}

float
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

    mTotalClassifiedWeight = 0.0;
    mUnclassifiedWeight = 0.0;
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
