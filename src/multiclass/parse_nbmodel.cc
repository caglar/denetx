#include <string>

#include "parse_nbmodel.h"
#include "attrobs.h"
#include "estimator.h"
#include "utils.h"

#include "../parse_arfheader.h"

/*
 * parse_nbmodel.cc
 *
 *  Created on: Jul 20, 2011
 *      Author: caglar
 */
#define NUMERIC 0
#define NOMINAL 1

using std::string;
using est::NormalEstimator;

static string seperator = "%==%";

/**
 * File Structure
 *      ++++++++++++++++++++++++++++++++++
 *      |Observed Class Dist             |
 *      |________________________________|
 *      |class_0 val_0                   |
 *      |.        .                      |
 *      |.        .                      |
 *      |.        .                      |
 *      |class_n val_n                   |
 *      | %==%                           |
 *      |________________________________|
 *      |                                |
 *      |Att Dist Per Class              |
 *      |________________________________|
 *      |att_no,attType,...              |
 *      |.                               |
 *      |.                               |
 *      |.                               |
 *      ++++++++++++++++++++++++++++++++++
 *
 *      Write the observed class distributions and
 *      attribute statistics to a model file.
 *
 *      For numeric attributes: followings are saved for each class:
 *       sumOfWeights, sumOfValues, mean, standardDev
 *
 *      The ordering of the data for numeric will be:
 *       attNo,0,classNo,sumOfWeights,sumOfValues,sumOfValuesSq,mean,standardDev
 *      The ordering of the data for nominal attributes will be:
 *       attNo,1,classNo,attVal,weight
 */
static void
createModelFileContent(Dvec &observedClassDist,
        vector<AttributeClassObserver *> &attributeObservers,
        arfheader *arfHeader, string& model_file_content)
{
    vector<arfcategory> arfCats = arfHeader->categories;
    for (int i = 0; i < observedClassDist.size(); i++) {
        model_file_content += i + " " + observedClassDist[i] + "\n";
    }
    model_file_content += seperator + "\n";
    for (int i = 0; i < arfCats.size(); i++) {
        if (arfCats[i].name == "nominal") {
            NomAttrObserver *nomAttrObs = attributeObservers[i];
            vector<DVec> attValDistPerClass =
                    nomAttrObs->getattValDistPerClass();
            unsigned int classSize = attValDistPerClass.size();

            for (auto j = 0; j < classSize; j++) {
                unsigned int valSize = attValDistPerClass[j].size();
                for (auto n = 0; n < valSize; n++) {
                    model_file_content += i + " " + NOMINAL + " " + j + " " + n
                            + " " + attValDistPerClass[j][n] + "\n";
                }
            }
        }
        else if (arfCats[i].name == "numeric") {
            NumAttrObserver *numAttrObs = attributeObservers[i];
            vector<NormalEstimator *> attValDistPerClass =
                    numAttrObs->getAttValDistPerClass();
            NormalEstimator *nEstimator = attValDistPerClass[j];

            for (int j = 0; j < attValDistPerClass.size(); j++) {
                model_file_content += i + " " + NUMERIC + " " + j + " "
                        + nEstimator->getSumOfWeights() + " "
                        + nEstimator->getSumOfValues() + " "
                        + nEstimator->getSumOfValuesSq() + " "
                        + nEstimator->getMean() + " "
                        + nEstimator->getStandardDev();
            }
        }
    }
}

void
readModelFile(Dvec &observedClassDist,
        vector<AttributeClassObserver *> &attributeObservers,
        string nb_model_file)
{
    string model_file_content = "";

}

void
writeModelFile(Dvec &observedClassDist,
        vector<AttributeClassObserver *> &attributeObservers,
        arfheader *arfHeader, string nb_model_file)
{
    string modelFileContent = "";
    createModelFileContent(observedClassDist, attributeObservers, arfHeader,
            modelFileContent);
    const char *mModelFileContent = modelFileContent.c_str();
    c_write_file(nb_model_file, mModelFileContent, sizeof(*mModelFileContent));
}
