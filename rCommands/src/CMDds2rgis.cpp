/******************************************************************************

GHAAS RiverGIS Utilities V3.0
Global Hydrological Archive and Analysis System
Copyright 1994-2024, UNH - CCNY

CMDds2rgis.cpp

bfekete@ccny.cuny.edu

*******************************************************************************/

#include <cm.h>
#include <DB.hpp>
#include <RG.hpp>

static void _CMDprintUsage (const char *arg0) {
    CMmsgPrint(CMmsgInfo, "%s [options] <data stream file> <rgis file>", CMfileName(arg0));
    CMmsgPrint(CMmsgInfo, "     -m, --template  [template coverage]");
    CMmsgPrint(CMmsgInfo, "     -t, --title     [dataset title]");
    CMmsgPrint(CMmsgInfo, "     -u, --subject   [subject]");
    CMmsgPrint(CMmsgInfo, "     -d, --domain    [domain]");
    CMmsgPrint(CMmsgInfo, "     -v, --version   [version]");
    CMmsgPrint(CMmsgInfo, "     -s, --shadeset  [standard|grey|blue|blue-to-red|elevation]");
    CMmsgPrint(CMmsgInfo, "     -h, --help");
}

int main(int argc, char *argv[]) {
    FILE *inFile;
    int argPos, argNum = argc, ret;
    char *title   = (char *) NULL, *subject = (char *) NULL;
    char *domain  = (char *) NULL, *version = (char *) NULL;
    char *tmpName = (char *) NULL;
    DBInt shadeSet = DBFault;
    DBObjData *outData, *tmpData = (DBObjData *) NULL;

    if (argNum < 2) goto Help;

    for (argPos = 1; argPos < argNum;) {
        if (CMargTest (argv[argPos], "-m", "--template")) {
            if ((argNum = CMargShiftLeft(argPos, argv, argNum)) <= argPos) {
                CMmsgPrint(CMmsgUsrError, "Missing template!");
                return (CMfailed);
            }
            tmpName = argv[argPos];
            if ((argNum = CMargShiftLeft(argPos, argv, argNum)) <= argPos) break;
            continue;
        }
        if (CMargTest (argv[argPos], "-t", "--title")) {
            if ((argNum = CMargShiftLeft(argPos, argv, argNum)) <= argPos) {
                CMmsgPrint(CMmsgUsrError, "Missing title!");
                return (CMfailed);
            }
            title = argv[argPos];
            if ((argNum = CMargShiftLeft(argPos, argv, argNum)) <= argPos) break;
            continue;
        }
        if (CMargTest (argv[argPos], "-u", "--subject")) {
            if ((argNum = CMargShiftLeft(argPos, argv, argNum)) <= argPos) {
                CMmsgPrint(CMmsgUsrError, "Missing subject!");
                return (CMfailed);
            }
            subject = argv[argPos];
            if ((argNum = CMargShiftLeft(argPos, argv, argNum)) <= argPos) break;
            continue;
        }
        if (CMargTest (argv[argPos], "-d", "--domain")) {
            if ((argNum = CMargShiftLeft(argPos, argv, argNum)) <= argPos) {
                CMmsgPrint(CMmsgUsrError, "Missing domain!");
                return (CMfailed);
            }
            domain = argv[argPos];
            if ((argNum = CMargShiftLeft(argPos, argv, argNum)) <= argPos) break;
            continue;
        }
        if (CMargTest (argv[argPos], "-v", "--version")) {
            if ((argNum = CMargShiftLeft(argPos, argv, argNum)) <= argPos) {
                CMmsgPrint(CMmsgUsrError, "Missing version!");
                return (CMfailed);
            }
            version = argv[argPos];
            if ((argNum = CMargShiftLeft(argPos, argv, argNum)) <= argPos) break;
            continue;
        }
        if (CMargTest (argv[argPos], "-s", "--shadeset")) {
            int shadeCodes[] = {DBDataFlagDispModeContStandard,
                                DBDataFlagDispModeContGreyScale,
                                DBDataFlagDispModeContBlueScale,
                                DBDataFlagDispModeContBlueRed,
                                DBDataFlagDispModeContElevation};
            const char *shadeSets[] = {"standard", "grey", "blue", "blue-to-red", "elevation", (char *) NULL};

            if ((argNum = CMargShiftLeft(argPos, argv, argNum)) <= argPos) {
                CMmsgPrint(CMmsgUsrError, "Missing shadeset!");
                return (CMfailed);
            }
            if ((shadeSet = CMoptLookup(shadeSets, argv[argPos], true)) == CMfailed) {
                CMmsgPrint(CMmsgUsrError, "Invalid shadeset!");
                return (CMfailed);
            }
            shadeSet = shadeCodes[shadeSet];
            if ((argNum = CMargShiftLeft(argPos, argv, argNum)) <= argPos) break;
            continue;
        }
Help:   if (CMargTest (argv[argPos], "-h", "--help")) {
            _CMDprintUsage (argv[0]);
            return (DBSuccess);
        }
        if ((argv[argPos][0] == '-') && (strlen(argv[argPos]) > 1)) {
            CMmsgPrint(CMmsgUsrError, "Unknown option: %s!", argv[argPos]);
            return (DBFault);
        }
        argPos++;
    }

    if (tmpName == (char *) NULL) {
        CMmsgPrint(CMmsgUsrError, "Missing template covarage!");
        return (DBFault);
    }

    if (argNum > 3) {
        CMmsgPrint(CMmsgUsrError, "Extra arguments!");
        _CMDprintUsage (argv[0]);
        return (DBFault);
    }

    inFile = (argNum > 1) && (strcmp(argv[1], "-") != 0) ? fopen(argv[1], "r") : stdin;
    if (inFile == (FILE *) NULL) {
        CMmsgPrint(CMmsgSysError, "Input data stream opening error in: %s %d", __FILE__, __LINE__);
        return (DBFault);
    }

    tmpData = new DBObjData();
    if (tmpData->Read(tmpName) == DBFault) {
        if (inFile != stdin) fclose(inFile);
        delete tmpData;
        return (DBFault);
    }

    if (title == (char *) NULL) {
        CMmsgPrint(CMmsgUsrError, "Title is not set!");
        return (DBFault);
    }
    if (subject == (char *) NULL) subject = tmpData->Document(DBDocSubject);
    if (domain  == (char *) NULL) domain  = tmpData->Document(DBDocGeoDomain);
    if (version == (char *) NULL) version = (char *) "0.01pre";

    switch (tmpData->Type()) {
        default:
        case DBTypeVectorPoint:
            outData = new DBObjData(title, DBTypeTable);
            break;
        case DBTypeGridContinuous:
        case DBTypeGridDiscrete:
            outData = DBGridToGrid(tmpData, DBTypeGridContinuous);
            break;
        case DBTypeNetwork:
            outData = DBNetworkToGrid(tmpData, DBTypeGridContinuous);
            break;
    }

    if (outData == (DBObjData *) NULL) {
        if (inFile != stdin) fclose(inFile);
        delete tmpData;
        return (DBFault);
    }

    if (RGlibDataStream2RGIS(outData, tmpData, inFile) == DBSuccess) {
        outData->Name(title);
        outData->Document(DBDocSubject, subject);
        outData->Document(DBDocGeoDomain, domain);
        outData->Document(DBDocVersion, version);
        if (shadeSet != DBFault) {
            outData->Flags(DBDataFlagDispModeContShadeSets, DBClear);
            outData->Flags(shadeSet, DBSet);
        }
        ret = (argNum > 2) && (strcmp(argv[2], "-") != 0) ? outData->Write(argv[2]) : outData->Write(stdout);
    }
    else ret = DBFault;

    delete tmpData;
    delete outData;
    if (inFile != stdin) fclose(inFile);
    return (ret);
}
