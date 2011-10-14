#ifndef ATTRSPLITSUGGEST
#define ATTRSPLITSUGGEST

#include <algorithm>
#include "multiclass/split_test.h"
#include "multiclass/flin.h"
#include "../utils.h"

using std::copy;

struct AttrSplitSuggestion
{
    Matrix_t resultingClassDistribution;
    SplitTest *splitTest;
    float merit;
    int numSplits;

    AttrSplitSuggestion(const AttrSplitSuggestion &attrSplitSuggest): resultingClassDistribution(attrSplitSuggest.resultingClassDistribution),
    merit(attrSplitSuggest.merit), numSplits(attrSplitSuggest.numSplits)
    {
        copy(attrSplitSuggest.splitTest, attrSplitSuggest.splitTest + sizeof(*(attrSplitSuggest.splitTest)), splitTest);
    }

    AttrSplitSuggestion(Matrix_t classDist, SplitTest *sTest, float mer) :
        resultingClassDistribution(classDist), merit(mer), numSplits(0)
    {
        //resultingClassDistribution = classDist;
        copy(sTest, sTest + sizeof(*sTest), this->splitTest);
        //this->merit = mer;

    }

    AttrSplitSuggestion() :
        splitTest(NULL), merit(0.0), numSplits(0.0)
    {
    }

    ~AttrSplitSuggestion()
    {
        c_free(splitTest);
    }

    AttrSplitSuggestion&
        operator=(const AttrSplitSuggestion &other)
        {
            this->resultingClassDistribution = other.resultingClassDistribution;
            copy(other.splitTest, other.splitTest + sizeof(*other.splitTest),
                 this->splitTest);
            this->merit = other.merit;
            this->numSplits = other.numSplits;
            return *this;
        }

    bool
        operator<(const AttrSplitSuggestion &rhs) const
        {
            return (merit < rhs.merit);
        }

    bool
        operator>(const AttrSplitSuggestion &rhs) const
        {
            return (merit > rhs.merit);
        }

    bool
        operator==(const AttrSplitSuggestion &rhs) const
        {
            return (merit == rhs.merit);
        }
};

#endif
