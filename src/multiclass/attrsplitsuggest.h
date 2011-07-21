#ifndef ATTRSPLITSUGGEST
#define ATTRSPLITSUGGEST

#include <algorithm>

#include "split_test.h"
#include "flin.h"

using std::copy;

struct AttrSplitSuggestion
{
    Matrix_t resultingClassDistribution;
    SplitTest *splitTest;
    float merit;
    int numSplits;

    AttrSplitSuggestion(Matrix_t classDist, SplitTest *sTest, float mer)
    {
        resultingClassDistribution = classDist;
        copy(sTest, sTest + sizeof(*sTest), this->splitTest);
        this->merit = mer;

    }

    AttrSplitSuggestion()
    {
        merit = 0.0;
        numSplits = 0;
        splitTest = NULL;
    }

    ~AttrSplitSuggestion()
    {
        c_free(splitTest);
    }

    AttrSplitSuggestion& operator=(const AttrSplitSuggestion &other) {
        this->resultingClassDistribution = other.resultingClassDistribution;
        copy(other.splitTest, other.splitTest + sizeof(*other.splitTest), this->splitTest);
        this->merit = other.merit;
        this->numSplits = other.numSplits;
    }

    bool operator<(const AttrSplitSuggestion &rhs) const{
        return(merit < rhs.merit);
    }

    bool operator>(const AttrSplitSuggestion &rhs) const {
        return (merit > rhs.merit);
    }

    bool operator==(const AttrSplitSuggestion &rhs) const {
        return (merit == rhs.merit);
    }
};

#endif
