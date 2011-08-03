/*
 * parse_nbmodel.h
 *
 *  Created on: Jul 21, 2011
 *      Author: caglar
 */

#ifndef PARSE_NBMODEL_H_
#define PARSE_NBMODEL_H_

#include <vector>
#include <string>

#include "multiclass/dvec.h"
#include "multiclass/attrobs.h"
#include "parse_arfheader.h"

using std::vector;
using std::string;

void
writeModelFile(DVec const &observedClassDist,
        vector<AttributeClassObserver *> const &attributeObservers,
        arfheader *arfHeader, string nb_model_file);

void
readModelFile(DVec &observedClassDist,
        vector<AttributeClassObserver *> &attributeObservers,
        arfheader *arfHeader, string nb_model_file);

#endif
/* PARSE_NBMODEL_H_ */
