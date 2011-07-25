/*
 * parse_nbmodel.h
 *
 *  Created on: Jul 21, 2011
 *      Author: caglar
 */

#ifndef PARSE_NBMODEL_H_
#define PARSE_NBMODEL_H_

#include "dvec.h"
#include <vector>
#include <string>
#include "attrobs.h"
#include "../parse_arfheader.h"

using std::vector;
using std::string;

void
writeModelFile(DVec &observedClassDist,
        vector<AttributeClassObserver *> &attributeObservers,
        arfheader *arfHeader, string nb_model_file);

void
readModelFile(DVec &observedClassDist,
        vector<AttributeClassObserver *> &attributeObservers,
        arfheader *arfHeader, string nb_model_file);

#endif /* PARSE_NBMODEL_H_ */
