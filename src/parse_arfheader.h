/*
 * parse_arfheader.h
 *
 *  Created on: Apr 20, 2011
 *      Author: caglar
 */

#ifndef PARSE_ARFHEADER_H_
#define PARSE_ARFHEADER_H_

#include <vector>
#include <string>
#include <stdint.h>
#include <map>

enum fType
{
    NOMINAL, NUMERIC, TIME, UNKNOWN
};

struct arffeature
{
    std::string name;
    std::string fid;
    fType type;
    int no_of_vals;
    arffeature() :
        name(""), fid(""), type(NUMERIC), no_of_vals(0)
    {
    }
};

struct arfcategory
{
    std::string name;
    float val;
    arfcategory() :
        name(""), val(0.0f)
    {
    }
};

struct arfheader
{
    std::string taskName;
    std::map<uint32_t, arffeature> features;
    unsigned int no_of_features;
    std::vector<arfcategory> categories;
    unsigned int no_of_categories;
    arfheader() :
        taskName(""), no_of_features(0), no_of_categories(0)
    {
    }
};

arfheader *
parseARFXFile(const char * headerFile);

#endif /* PARSE_ARFHEADER_H_ */
