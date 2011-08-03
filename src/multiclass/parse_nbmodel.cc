/*
 * parse_nbmodel.cc
 *
 *  Created on: Jul 20, 2011
 *      Author: caglar
 */

#include <stdlib.h>
#include <sstream>
#include <fcntl.h>
#include <syslog.h>
#include <map>

#include "utils.h"
#include "parse_arfheader.h"

#include "multiclass/parse_nbmodel.h"
#include "multiclass/estimator.h"
#include "multiclass/nomattrobs.h"
#include "multiclass/numattrobs.h"

using est::NormalEstimator;

static const string SEPERATOR = "%==%";

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
createModelFileContent(DVec const &observedClassDist,
        vector<AttributeClassObserver *> const &attributeObservers,
        arfheader *arfHeader, string& model_file_content)
{
    std::map<uint32_t, arffeature> arfAtts = arfHeader->features;
    model_file_content = "";
    std::stringstream str_stream;

    for (unsigned int i = 0; i < observedClassDist.size(); i++) {
        //        str_stream.flush();
        str_stream << i << " " << observedClassDist[i] << std::endl;
        //model_file_content += str_stream.str();
    }

    str_stream << SEPERATOR << std::endl;

    //model_file_content = model_file_content + SEPERATOR + "\n";

    for (auto i = 0; i < arfHeader->no_of_features; i++) {
        if (attributeObservers[i] != NULL) {
            if (arfAtts[i].type == NOMINAL) {
                NomAttrObserver *nomAttrObs =
                        dynamic_cast<NomAttrObserver *> (attributeObservers[i]);

                vector<DVec> attValDistPerClass =
                        nomAttrObs->getattValDistPerClass();

                unsigned int classSize = attValDistPerClass.size();
                for (auto j = 0; j < classSize; j++) {
                    unsigned int valSize = attValDistPerClass[j].size();
                    for (auto n = 0; n < valSize; n++) {
                        //str_stream.flush();
                        str_stream << i << " " << NOMINAL << " " << j << " "
                                << n << " " << attValDistPerClass[j][n]
                                << std::endl;
                        //model_file_content = str_stream.str();
                        //model_file_content += i + " " + NOMINAL + " " + j + " " + n
                        //+ " " + attValDistPerClass[j][n] + "\n";
                    }
                }
            }
            else if (arfAtts[i].type == NUMERIC) {
                NumAttrObserver *numAttrObs =
                        dynamic_cast<NumAttrObserver *> (attributeObservers[i]);
                vector<NormalEstimator *> attValDistPerClass =
                        numAttrObs->getAttValDistPerClass();

                for (auto j = 0; j < arfHeader->no_of_categories; j++) {
                    printf("cnt val: %d\n", j);
                    NormalEstimator
                            *nEstimator =
                                    dynamic_cast<NormalEstimator *> (attValDistPerClass[j]);
                    if (nEstimator != NULL) {
                        str_stream << i << " " << NUMERIC << " " << j << " "
                                << nEstimator->getSumOfWeights() << " "
                                << nEstimator->getSumOfValues() << " "
                                << nEstimator->getSumOfValuesSq() << " "
                                << nEstimator->getMean() << " "
                                << nEstimator->getStandardDev() << std::endl;
                    }
                }
            }
        }
    }
    model_file_content = str_stream.str();
}

void
writeModelFile(DVec const &observedClassDist,
        vector<AttributeClassObserver *> const &attributeObservers,
        arfheader *arfHeader, string nb_model_file)
{

    string modelFileContent = "";
    string tmpFileName = nb_model_file + ".tmp";
    printf("ObservedClassDist val 0: %f, ObservedClassDist val 1: %f",
            observedClassDist[0], observedClassDist[1]);

    printf("tmpFileName: %s, nbmodelfile: %s\n", tmpFileName.c_str(),
            nb_model_file.c_str());

    if (c_does_file_exist(nb_model_file.c_str())) {
        rename(nb_model_file.c_str(), tmpFileName.c_str());
    }

    createModelFileContent(observedClassDist, attributeObservers, arfHeader,
            modelFileContent);

    const char *mModelFileContent = modelFileContent.c_str();
    c_write_file(nb_model_file.c_str(), (void *) mModelFileContent,
            sizeof(*mModelFileContent));
}

const int MAX_SIZE = 1024;

static
void
parseObservedClassDistModelLine(DVec &observedClassDist, char *line)
{
    char * rest;
    char * ptr = (char *) c_malloc(sizeof *line);
    int classVal = -1;
    float weight;
    char *token;
    c_strcpy(ptr, c_trim(line));

    for (int i = 0;; token = NULL, i++) {
        token = strtok_r(ptr, " ", &rest);
        if (token == NULL)
            break;
        if (i == 0) {
            classVal = atoi(token);
        }
        if (i == 1) {
            weight = atof(token);
            observedClassDist[classVal] = weight;
        }
        ptr = rest; // rest contains the left over part..assign it to ptr...and start tokenizing again.
    }
}

static int attNo;

static AttributeClassObserver *
parseAttributeObserverModelLine(char *line, size_t no_of_classes)
{
    AttributeClassObserver *attObserver = NULL;
    NormalEstimator *nEst = NULL;
    char * rest;
    char * ptr;
    char *token;

    unsigned short int isNominal = 0;
    int classNo = 0;

    //For nominals
    int attVal = 0;
    double weight = 0.0;

    //For numerics:
    double sumOfWeights = 0.0, sumOfValues = 0.0, sumOfValuesSq = 0.0, mean =
            0.0, standardDev = 0.0;

    ptr = "";

    c_strcpy(ptr, line);

    for (int i = 0;; token = NULL, i++) {
        token = strtok_r(ptr, " ", &rest);
        if (token == NULL)
            break;
        if (i == 0) {
            attNo = atoi(token);
        }
        else {
            if (i == 1) {
                isNominal = atoi(token);
            }
            else {
                if (isNominal) {
                    switch (i) {
                    case 2:
                        attObserver = new NomAttrObserver();
                        classNo = atoi(token);
                        break;
                    case 3:
                        attVal = atoi(token);
                        break;
                    case 4:
                        classNo = atoi(token);
                        break;
                    case 5:
                        weight = atof(token);
                        ((NomAttrObserver *) attObserver)->observeAttributeClass(
                                (float) attVal, classNo, weight);
                        break;
                    default:
                        break;
                    }
                }
                else {
                    //sumOfWeights,sumOfValues,sumOfValuesSq,mean,standardDev
                    switch (i) {
                    case 2:
                        attObserver = new NumAttrObserver(no_of_classes);
                        classNo = atoi(token);
                        break;
                    case 3:
                        sumOfWeights = atof(token);
                        break;
                    case 4:
                        sumOfValues = atof(token);
                        break;
                    case 5:
                        sumOfValuesSq = atof(token);
                        break;
                    case 6:
                        mean = atof(token);
                        break;
                    case 7:
                        standardDev = atof(token);
                        nEst = new NormalEstimator(no_of_classes);
                        nEst->setMean(mean);
                        nEst->setStandardDev(standardDev);
                        nEst->setSumOfValues(sumOfValues);
                        nEst->setSumOfWeights(sumOfWeights);
                        nEst->setSumOfWeights(sumOfValuesSq);
                        ((NumAttrObserver *) attObserver)->addNewValDist(nEst,
                                classNo);
                        break;
                    default:
                        break;
                    }
                }
            }
        }
        ptr = rest; // rest contains the left over part..assign it to ptr...and start tokenizing again.
    }
    return attObserver;
}

void
readModelFile(DVec &observedClassDist,
        vector<AttributeClassObserver *> &attributeObservers,
        arfheader *arfHeader, string nb_model_file)
{
    const char *filePath = nb_model_file.c_str();
    char *contents = new char[MAX_SIZE];
    flock fl =
        { F_WRLCK, SEEK_SET, 0, 0, 0 };
    int fd;
    bool attrStartFlag = false;
    const char *pSeperator = SEPERATOR.c_str();

    if ((fd = open(filePath, O_RDWR)) == -1) {
        syslog(LOG_ALERT, "Can't open the file :%s\n", filePath);
        exit( EXIT_FAILURE);
    }

    fl.l_pid = getpid();
    if (fcntl(fd, F_SETLKW, &fl) == -1) {
        perror("fcntl");
        exit( EXIT_FAILURE);
    }

    FILE *fp;
    fp = fopen(filePath, "r");
    if (!fp) {
        syslog(LOG_ALERT, "Can not read the file: %s\n", filePath);
        exit( EXIT_FAILURE);
    }

    while (fgets(contents, MAX_SIZE, fp) != NULL) {
        contents = c_trim(contents);
        if (!attrStartFlag) {
            if (strcmp(contents, pSeperator) == 0) {
                attrStartFlag = true;
                attributeObservers.assign(
                        attNo,
                        parseAttributeObserverModelLine(c_trim(contents),
                                arfHeader->no_of_categories));
            }
            else {
                parseObservedClassDistModelLine(observedClassDist,
                        c_trim(contents));
            }
        }
        else {
            attributeObservers.assign(
                    attNo,
                    parseAttributeObserverModelLine(c_trim(contents),
                            arfHeader->no_of_categories));
        }
    }
    fl.l_type = F_UNLCK; /* set to unlock same region */
    if (fcntl(fd, F_SETLK, &fl) == -1) {
        syslog(LOG_ALERT, "Can not unlock file %s\n", filePath);
        exit(1);
    }
    fclose(fp);
    close(fd);
}
