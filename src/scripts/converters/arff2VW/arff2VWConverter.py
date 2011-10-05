#!/usr/bin/python

import sys
import os.path
import re
import pprint

dataTypes = [
        "numeric",
        "nominal",
        "string"
]

ATTR_TOKEN="@attribute"

NUMERIC = 0
NOMINAL = 1
STRING  = 2

Classes = {}
NoOfClasses = 0

class ARFHeaderParser:

    def __init__(self):
        self._i = 0
        self._rows = {}

    def parse(self, line):
        if line.startswith(ATTR_TOKEN):
            if (line.find(dataTypes[NUMERIC]) != -1):
                rowName = line.replace(ATTR_TOKEN,
                        "").replace(dataTypes[NUMERIC], "").strip()
                self._rows["features", self._i] = (dataTypes[NUMERIC], rowName)
                self._i += 1
            elif line.find("{") != -1 and line.find("}"):
                p = re.compile("(\{.+\})")
                featStr = p.search(line).group()[1:-1].strip()
                feats = featStr.split(",")
                rowName = re.search("(.+)\{", line).group().replace("@attribute", "").replace("{","").strip()
                self._rows["features", self._i] = (dataTypes[NOMINAL], rowName,feats)
                self._i += 1
            elif line.find(dataTypes[STRING]) != -1:
                rowName = line.replace(ATTR_TOKEN,
                        "").replace(dataTypes[STRING], "").strip()
                self._rows["features", self._i] = [dataTypes[NUMERIC], rowName]
                self._i += 1

    def getRows(self):
        return self._rows

class ARFXCreator:
    def __init__(self):
        self._arfx = "<?xml version=\"1.0\"?>\n<arfheader>\n"

    def createARFX(self, rows):
        global Classes
        labels = rows.pop(("features", len(rows) - 1))
        self._arfx += "<features>\n"

        for i in range(0, len(rows)):
            if rows[("features", i)][0] == dataTypes[NUMERIC]:
                self._arfx += "<feature name =\"" + rows["features", i][1] +  "\" fid=\"" + str(i) + "\" type=\"" + dataTypes[NUMERIC] + "\"/>\n"
            elif rows[("features", i)][0] == dataTypes[NOMINAL]:
                self._arfx += "<feature name =\"" + rows["features", i][1] +  "\" fid=\"" + str(i) + "\" no_of_vals=\"" + len(rows["features", i][2])+ "\" type=\"" + dataTypes[NOMINAL]+ "\"/>\n";
            else:
                self._arfx += "<feature name =\"" + rows["features", i][1] +  "\" fid=\"" + str(i) + "\" type=\"" + rows["features", i][0] + "\"/>\n"

        self._arfx += "</features>\n<classes>\n"
        for i in range(0, len(labels[2])):
            Classes[labels[2][i]] = str(i)
            self._arfx += "<class name=\"" + labels[2][i] + "\" val=\"" + str(i) + "\"/>\n"
        self._arfx += "</classes>\n</arfheader>"
        return self._arfx

def appendText2Line(file, line):
    with open(file, "a+", 0) as myfile:
        myfile.write(line)

def getClassVal(strClassVal):
    global Classes
    classVal = "0"
    if Classes.has_key(strClassVal.strip()):
        classVal = Classes[strClassVal.strip()]
    else:
        raise Exception("Class, " + strClassVal + " couldn't be found")
    return classVal

def getVwLine(line):
    tokens = line.split(",")
    strClassVal = tokens.pop()
    classVal = getClassVal(strClassVal)
    vwLine = classVal + " | features "
    const = "const:.01"
    for i in range(0, len(tokens)):
        vwLine += str(i) + ":" + tokens[i].strip() + " "
    vwLine += const + "\n"
    return vwLine

def main():
    vwData = ""
    if len(sys.argv) > 3:
        arffileName = sys.argv[1]
        vwFileName = sys.argv[2]
        arfxFile = sys.argv[3]
    else:
        raise Exception("Please don't forget to enter file names! arffilename vwfilename arfxfile")

    if (arffileName != None and os.path.exists(arffileName)):
        arfReader = ARFHeaderParser()
        for line in open(arffileName, 'r'):
            if not (line.startswith("%") or line.startswith("#") or line.startswith("@")):
                 appendText2Line(vwFileName, getVwLine(line))
            elif (line.startswith("@data")):
                rows = arfReader.getRows()
                arfxCreator = ARFXCreator()
                arfxData = arfxCreator.createARFX(rows)
            elif (line.startswith("@")):
                arfReader.parse(line)
        appendText2Line(arfxFile, arfxData)
    else:
        raise Exception("Please don't forget to enter a file as an argument to the script")

if __name__ == "__main__":
    main()
