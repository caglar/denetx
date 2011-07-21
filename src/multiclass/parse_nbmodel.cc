/*
 * parse_nbmodel.cc
 *
 *  Created on: Jul 20, 2011
 *      Author: caglar
 */
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
 *       attNo,0,classNo,sumOfWeights,sumOfValues,mean,standardDev
 *      The ordering of the data for nominal attributes will be:
 *       attNo,1,classNom,
 */

static string seperator = "%==%";

static void
createModelFileContent(Dvec observedClassDist,
        vector<AttributeClassObserver *> attributeObservers,
        string& model_file_content)
{
    for (int i = 0; i < observedClassDist.size(); i++) {
        model_file_content += i + " " + observedClassDist[i] + "\n";
    }

    model_file_content += seperator + "\n";

    for (int i = 0; i < attributeObservers.size(); i++) {
        model_file_content += i + " " + model_file_content[i];
    }

}

void
readModelFile(Dvec observedClassDist,
        vector<AttributeClassObserver *> attributeObservers,
        string nb_model_file)
{
    string model_file_content = "";

}

void
writeModelFile(Dvec observedClassDist,
        vector<AttributeClassObserver *> attributeObservers,
        string nb_model_file)
{

}
