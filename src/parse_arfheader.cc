/*
 * parse_arfheader.cc
 *
 *  Created on: Apr 20, 2011
 *      Author: caglar
 */

#include <iostream>
#include <cstring>
#include <tinyxml.h>

#include "parse_arfheader.h"
#include "parse_example.h"

const uint32_t hashbase = 97562527;

void
setElement(TiXmlElement* element, char* elemName, arfheader *arfHeader)
{

    if (!strcmp(elemName, "relation")) {
        element->QueryValueAttribute("name", &(arfHeader->taskName));
    }
    else if (!strcmp(elemName, "feature")) {
        arffeature arffeature;
        std::string fType;

        element->QueryValueAttribute("name", &arffeature.name);
        element->QueryValueAttribute("fid", &arffeature.fid);
        element->QueryValueAttribute("type", &fType);
        substring str;
        str.start = (char *) arffeature.name.c_str();
        str.end = str.start + (strlen(str.start) - 1) * sizeof(char);
        uint32_t weight_index = hashstring(str, hashbase);

        if (!fType.compare("nominal")) {
            arffeature.type = NOMINAL;
            element->QueryIntAttribute("noOfVal", &(arffeature.no_of_vals));
        }
        else if (!fType.compare("numeric")) {
            arffeature.type = NUMERIC;
        }
        else if (!fType.compare("time")) {
            arffeature.type = TIME;
        }
        arfHeader->no_of_features++;
        arfHeader->features[weight_index] = arffeature;
    }
    else if (!strcmp(elemName, "class")) {
        arfcategory aCat;
        element->QueryValueAttribute("name", &aCat.name);
        element->QueryFloatAttribute("val", &aCat.val);
        arfHeader->no_of_categories++;
        arfHeader->categories.push_back(aCat);
    }
}

void
parseArfHeader(TiXmlNode* parent, arfheader *arfHeader)
{
    TiXmlNode* child;
//    TiXmlText* text;
    int t = parent->Type();

    if (t == TiXmlNode::ELEMENT) {
        char *elemName = (char *) parent->Value();
        setElement(parent->ToElement(), elemName, arfHeader);
    }
    for (child = parent->FirstChild(); child != 0; child = child->NextSibling()) {
        parseArfHeader(child, arfHeader);
    }
}

arfheader*
parseARFXFile(const char *xmlFile)
{
    TiXmlDocument doc(xmlFile);
    bool loadOkay = doc.LoadFile();
    arfheader *arfHeader = new arfheader();
    if (loadOkay) {
        printf("\n%s:\n", xmlFile);
        parseArfHeader(&doc, arfHeader); 
    }
    else {
        printf("Failed to load file \"%s\"\n", xmlFile);
    }
    return arfHeader;
}
