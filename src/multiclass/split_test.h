#ifndef SPLIT_TEST
#define SPLIT_TEST

#include <cmath>
#include "example.h"

class SplitTest
{
private:
    int mNoOfBranches;
    bool isNom;
public:
    SplitTest() :
        mNoOfBranches(0), isNom(false)
    {
    }

    virtual int
    branchForInstance(feature *f) = 0;

    virtual
    ~SplitTest()
    {
    }
};

class NominalSplitTest : public SplitTest
{
private:
    bool misMultiWay;
    float AttValue;

public:
    NominalSplitTest(bool isMultiWay, float attValue) :
        misMultiWay(isMultiWay), AttValue(attValue)
    {
        misMultiWay = isMultiWay;
        AttValue = attValue;
    }

    NominalSplitTest(bool isMultiWay)
    {
        misMultiWay = isMultiWay;
        AttValue = 0.0;
    }

    int
    branchForInstance(feature *f)
    {
        if (misMultiWay) {
            return nominalBinaryBranch(f);
        }
        else {
            return nominalMultiwayBranch(f);
        }
    }

    int
    nominalBinaryBranch(feature *f)
    {
        return isnan(f->x) ? -1 : (f->x == AttValue ? 0 : 1);
    }

    int
    nominalMultiwayBranch(feature *f)
    {
        return isnan(f->x) ? -1 : f->x;
    }
};

class NumericSplitTest : public SplitTest
{
private:
    bool EqualsPassesTest;
    float AttValue;

public:

    NumericSplitTest()
    {
        EqualsPassesTest = false;
        AttValue = 0.0;
    }

    int
    numericBinaryTest(feature *f)
    {
        return this->numericBinaryBranch(f);
    }

    int
    numericBinaryBranch(feature *f)
    {
        if (isnan(f->x)) {
            return -1;
        }

        if (f->x == AttValue) {
            return EqualsPassesTest ? 0 : 1;
        }
        return f->x < AttValue ? 0 : 1;
    }

    int
    branchForInstance(feature *f)
    {
        return this->numericBinaryBranch(f);
    }
};

#endif
