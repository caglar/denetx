/*
 * Estimator.h
 * Estimator classes for NB's values.
 * These estimator classes are inspired by
 *  Created on: Apr 8, 2011
 *      Author: caglar
 */

#ifndef ESTIMATOR_H_
#define ESTIMATOR_H_

#include <math.h>

#include <algorithm>

#include "multiclass/statistics.h"
#include "multiclass/arraycopy.h"

template<typename T>
int
ArrayLength(const T a)
{
    return sizeof(a) / sizeof(T);
}

namespace est
{
    class Estimator
    {
      public:
          virtual
              ~Estimator()
              {
              }
          virtual void
              addValue(float data, float weight) = 0;
          virtual float
              getProbability(float data) = 0;
    };

    class NormalEstimator : public Estimator
    {
      protected:
          /** The sum of the weights */
          float mSumOfWeights;

          /** The sum of the values seen */
          float mSumOfValues;

          /** The sum of the values squared */
          float mSumOfValuesSq;

          /** The current mean */
          float mMean;

          /** The current standard deviation */
          float mStandardDev;

          /** The precision of numeric values ( = minimum std dev permitted) */
          float mPrecision;

          /** Normal Constant */
          float NORMAL_CONSTANT;

          /** Check wheter the estimator has already been initialized **/
          bool isInitializedFlag;

      public:
          /**
           * Constructor that takes a precision argument.
           *
           * @param precision the precision to which numeric values are given. For
           * example, if the precision is stated to be 0.1, the values in the
           * interval (0.25,0.35] are all treated as 0.3.
           */
          explicit
              NormalEstimator(float precision) :
                  mSumOfWeights(0),
                  mSumOfValues(0),
                  mSumOfValuesSq(0),
                  mMean(0.0),
                  isInitializedFlag(true)
        {
            NORMAL_CONSTANT = sqrt(2 * M_PI);
            mPrecision = precision;
            mStandardDev = mPrecision / (2 * 3);
        }

          NormalEstimator() :
              mSumOfWeights(0),
              mSumOfValues(0),
              mSumOfValuesSq(0),
              mMean(0.0),
              mPrecision(0.1),
              isInitializedFlag(false)
        {
            NORMAL_CONSTANT = sqrt(2 * M_PI);
            mStandardDev = mPrecision / (2 * 3);
        }

          ~NormalEstimator()
          {
          }

          float
              getSumOfWeights()
              {
                  return this->mSumOfWeights;
              }

          float
              getSumOfValues()
              {
                  return this->mSumOfValues;
              }

          float
              getSumOfValuesSq()
              {
                  return this->mSumOfValuesSq;
              }

          float
              getMean()
              {
                  return this->mMean;
              }

          float
              getStandardDev()
              {
                  return this->mStandardDev;
              }

          bool
              isInitialized(){
                  return this->isInitializedFlag;
              }

          void
              setInitializedFlag(bool flag)
              {
                  this->isInitializedFlag = flag;
              }

          void
              setPrecision(float precision)
              {
                  this->mPrecision = precision;
              }

          void
              setSumOfWeights(float sumWeights)
              {
                  this->mSumOfWeights = sumWeights;
              }

          void
              setSumOfValues(float sumValues)
              {
                  this->mSumOfValues = sumValues;
              }

          void
              setSumOfValuesSq(float sumOfVals)
              {
                  this->mSumOfValuesSq = sumOfVals;
              }

          void
              setMean(float mean)
              {
                  this->mMean = mean;
              }

          void
              setStandardDev(float stdDev)
              {
                  this->mStandardDev = stdDev;
              }

          void
              addToSumOfValues(float value, float weight)
              {
                  this->mSumOfValues += value*weight;
              }

          void
              addToSumOfWeights(float weight)
              {
                  this->mSumOfWeights += weight;
              }

          void
              addToSumOfValuesSq(float value, float weight)
              {
                  this->mSumOfValuesSq += value*value*weight;
              }

          void
              addToSumOfValues(float value)
              {
                  this->mSumOfValues += value;
              }

          void
              addToSumOfValuesSq(float sqValue)
              {
                  this->mSumOfValuesSq += value;
              }

          void
              addSimpleValue(float value, float weight)
              {
                  addToSumOfValues(value, weight);
                  addToSumOfWeights(weight);
                  addToSumOfValuesSq(value, weight);
              }

          void
              calculateMean()
              {
                  this->mMean = (mSumOfValues / mSumOfWeights);
              }

          void
              calculateStdDev()
              {
                  float stdDev = sqrt(
                                      fabs(mSumOfValuesSq - mMean * mSumOfValues)
                                      / mSumOfWeights);
                  // If the stdDev ~= 0, we really have no idea of scale yet,
                  // so stick with the default. Otherwise...
                  if (stdDev > 1e-10) {
                      this->mStandardDev = std::max(mPrecision / (2 * 3),
                                              // allow at most 3sd's within one interval
                                              stdDev);
                  }
              }
          /**
           * Add a new data value to the current estimator.
           *
           * @param data the new data value
           * @param weight the weight assigned to the data value
           */
          float
              roundPrecise(float num)
              {
                  return round(num / mPrecision) * mPrecision;
              }

          void
              addValue(float data, float weight)
              {
                  if (weight == 0) {
                      return;
                  }

                  data = roundPrecise(data);
                  mSumOfWeights += weight;
                  mSumOfValues += data * weight;
                  mSumOfValuesSq += data * data * weight;

                  if (mSumOfValues > 0.0) {
                      calculateMean();
                      calculateStdDev();
                  }
              }

          /**
           * Get a probability estimate for a value
           *
           * @param data the value to estimate the probability of
           * @return the estimated probability of the supplied value
           */
          float
              getProbability(float data)
              {
                  data = round(data);
                  float zLower = (data - mMean - (mPrecision / 2)) / mStandardDev;
                  float zUpper = (data - mMean + (mPrecision / 2)) / mStandardDev;
                  float pLower = normalProbability(zLower);
                  float pUpper = normalProbability(zUpper);
                  return pUpper - pLower;
              }

          float
              getTotalObsWeights()
              {
                  return mSumOfWeights;
              }

          float
              probabilityDensity(float value)
              {
                  if (mSumOfWeights > 0.0) {
                      float stdDev = mStandardDev;
                      if (stdDev > 0.0) {
                          float diff = value - mMean;
                          return (1.0 / (NORMAL_CONSTANT * stdDev)) * exp(
                                                                          -((diff * diff) / (2.0 * stdDev * stdDev)));
                      }
                      return (value == mMean ? 1.0 : 0.0);
                  }
                  return 0.0;
              }

          float
              getLessThanWeight(float val, float equalToWeight)
              {
                  float lessThanWeight = mStandardDev > 0.0 ? (normalProbability(
                                                                                 (val - mMean) / mStandardDev) * mSumOfWeights)
                      : (mSumOfWeights - equalToWeight);
                  return lessThanWeight;
              }

          float
              getEqualToWeight(float val)
              {
                  return probabilityDensity(val) * mSumOfWeights;
              }

          float
              getGreaterThanWeight(float equalToWeight, float lessThanWeight)
              {
                  float greaterThanWeight = mSumOfWeights - equalToWeight
                      - lessThanWeight;
                  if (greaterThanWeight < 0) {
                      greaterThanWeight = 0.0;
                  }
                  return greaterThanWeight;
              }
    };

    class KernelEstimator : public Estimator
    {
      private:
          /** The small deviation allowed in float comparisons. */
          float SMALL;
          /** Vector containing all of the values seen */
          float *mValues;
          /** Vector containing the associated weights */
          float *mWeights;
          /** Number of values stored in m_Weights and m_Values so far */
          int mNumValues;
          /** The sum of the weights so far */
          float mSumOfWeights;
          /** The standard deviation */
          float mStandardDev;
          /** The precision of data values */
          float mPrecision;
          /** Whether we can optimise the kernel summation */
          bool mAllWeightsOne;
          /** Maximum percentage error permitted in probability calculations */
          float MAX_ERROR;

          /**
           * Execute a binary search to locate the nearest data value
           *
           * @param the data value to locate
           * @return the index of the nearest data value
           */
          int
              findNearestValue(float key)
              {
                  int low = 0;
                  int high = mNumValues;
                  int middle = 0;
                  while (low < high) {
                      middle = (low + high) / 2;
                      float current = mValues[middle];
                      if (current == key) {
                          return middle;
                      }
                      if (current > key) {
                          high = middle;
                      }
                      else if (current < key) {
                          low = middle + 1;
                      }
                  }
                  return low;
              }
      public:
          explicit
              KernelEstimator(float precision):SMALL(1e-6), mValues(new float[50]), mWeights(new float[50]),
              mNumValues(0), mSumOfWeights(0), mAllWeightsOne(true), mPrecision(precision), MAX_ERROR(0.01)
        {
            /*
               mValues = new float[50];
               mWeights = new float[50];

               mNumValues = 0;
               mSumOfWeights = 0;
               mAllWeightsOne = true;
               mPrecision = precision;
               */
            // precision cannot be zero
            if (mPrecision < SMALL)
                mPrecision = SMALL;
            // m_StandardDev = 1e10 * m_Precision; // Set the standard deviation initially very wide
            mStandardDev = mPrecision / (2 * 3);
        }

          /**
           * Add a new data value to the current estimator.
           *
           * @param data the new data value
           * @param weight the weight assigned to the data value
           */
          float
              roundPrecise(float num)
              {
                  return round(num / mPrecision) * mPrecision;
              }

          void
              addValue(float data, float weight)
              {
                  if (weight == 0) {
                      return;
                  }
                  data = round(data);
                  int insertIndex = findNearestValue(data);
                  if ((mNumValues <= insertIndex) || (mValues[insertIndex] != data)) {
                      if (mNumValues < ::ArrayLength(mValues)) {
                          int left = mNumValues - insertIndex;
                          arrcpy::dArraycopy(mValues, insertIndex, mValues,
                                             insertIndex + 1, left * sizeof(float));
                          arrcpy::dArraycopy(mWeights, insertIndex, mWeights,
                                             insertIndex + 1, left * sizeof(float));
                          mValues[insertIndex] = data;
                          mWeights[insertIndex] = weight;
                          mNumValues++;
                      }
                      else {
                          float *newValues = new float[::ArrayLength(mValues) * 2];
                          float *newWeights = new float[::ArrayLength(mValues) * 2];
                          int left = mNumValues - insertIndex;
                          arrcpy::dArraycopy(mValues, 0, newValues, 0,
                                             insertIndex * sizeof(float));
                          arrcpy::dArraycopy(mWeights, 0, newWeights, 0,
                                             insertIndex * sizeof(float));
                          newValues[insertIndex] = data;
                          newWeights[insertIndex] = weight;
                          arrcpy::dArraycopy(mValues, insertIndex, newValues,
                                             insertIndex + 1, left * sizeof(float));
                          arrcpy::dArraycopy(mWeights, insertIndex, newWeights,
                                             insertIndex + 1, left * sizeof(float));
                          mNumValues++;

                          delete[] mValues;
                          mValues = newValues;

                          delete[] mWeights;
                          mWeights = newWeights;
                      }
                      if (weight != 1) {
                          mAllWeightsOne = false;
                      }
                  }
                  else {
                      mWeights[insertIndex] += weight;
                      mAllWeightsOne = false;
                  }
                  mSumOfWeights += weight;
                  float range = mValues[mNumValues - 1] - mValues[0];
                  if (range > 0) {
                      mStandardDev = std::max(
                                              static_cast<float> (range / sqrt(mSumOfWeights)),
                                              // allow at most 3 sds within one interval
                                              mPrecision / (2 * 3));
                  }
              }

          /**
           * Get a probability estimate for a value.
           *
           * @param data the value to estimate the probability of
           * @return the estimated probability of the supplied value
           */
          float
              getProbability(float data)
              {
                  float delta = 0, sum = 0, currentProb = 0;
                  float zLower = 0, zUpper = 0;
                  if (mNumValues == 0) {
                      zLower = (data - (mPrecision / 2)) / mStandardDev;
                      zUpper = (data + (mPrecision / 2)) / mStandardDev;
                      return (normalProbability(zUpper) - normalProbability(zLower));
                  }
                  float weightSum = 0;
                  int start = findNearestValue(data);
                  for (int i = start; i < mNumValues; i++) {
                      delta = mValues[i] - data;
                      zLower = (delta - (mPrecision / 2)) / mStandardDev;
                      zUpper = (delta + (mPrecision / 2)) / mStandardDev;
                      currentProb = (normalProbability(zUpper) - normalProbability(
                                                                                   zLower));
                      sum += currentProb * mWeights[i];
                      weightSum += mWeights[i];
                      if (currentProb * (mSumOfWeights - weightSum) < sum * MAX_ERROR) {
                          break;
                      }
                  }
                  for (int i = start - 1; i >= 0; i--) {
                      delta = mValues[i] - data;
                      zLower = (delta - (mPrecision / 2)) / mStandardDev;
                      zUpper = (delta + (mPrecision / 2)) / mStandardDev;
                      currentProb = (normalProbability(zUpper) - normalProbability(
                                                                                   zLower));
                      sum += currentProb * mWeights[i];
                      weightSum += mWeights[i];
                      if (currentProb * (mSumOfWeights - weightSum) < sum * MAX_ERROR) {
                          break;
                      }
                  }
                  return sum / mSumOfWeights;
              }
    };

    class DiscreteEstimator : public Estimator
    {
      protected:
          float *mCounts;
          float mSumOfCounts;
          int NoOfSymbs;

      public:
          explicit
              DiscreteEstimator(int noOfSymbols)
              {
                  mCounts = new float[noOfSymbols];
                  mSumOfCounts = 0;
                  for (int i = 0; i < noOfSymbols; i++) {
                      mCounts[i] = 1;
                  }
                  mSumOfCounts = noOfSymbols;
                  NoOfSymbs = noOfSymbols;
              }

          void
              addValue(float data, float weight)
              {
                  mCounts[static_cast<int> (data)] += weight;
                  mSumOfCounts += weight;
              }

          float
              getProbability(float data)
              {
                  if (mSumOfCounts) {
                      return 0;
                  }
                  return mCounts[static_cast<int> (data)] / mSumOfCounts;
              }

          int
              getNoOfSymbols()
              {
                  return NoOfSymbs;
              }
    };
}

#endif /* ESTIMATOR_H_ */
