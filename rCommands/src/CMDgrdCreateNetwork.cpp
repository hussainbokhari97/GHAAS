/******************************************************************************

GHAAS RiverGIS Utilities V3.0
Global Hydrological Archive and Analysis System
Copyright 1994-2024, UNH - CCNY

CMDgrdCreateNetwork.cpp

bfekete@ccny.cuny.edu

*******************************************************************************/

#include <cm.h>
#include <math.h>
#include <DB.hpp>
#include <RG.hpp>

static void _CMDprintUsage (const char *arg0) {
    CMmsgPrint(CMmsgInfo, "%s [options] <input file> <output file>", CMfileName(arg0));
    CMmsgPrint(CMmsgInfo, "     -b, --build      [yes|no]");
    CMmsgPrint(CMmsgInfo, "     -g, --gradient   [down|up]");
    CMmsgPrint(CMmsgInfo, "     -p, --basin_pack [basin pack file]");
    CMmsgPrint(CMmsgInfo, "     -t, --title      [dataset title]");
    CMmsgPrint(CMmsgInfo, "     -d, --domain     [domain]");
    CMmsgPrint(CMmsgInfo, "     -v, --version    [version]");
    CMmsgPrint(CMmsgInfo, "     -h, --help");
}

int main(int argc, char *argv[]) {
    int argPos, argNum = argc, ret;
    bool downhill = true, build = true;
    char *title = (char *) NULL;
    char *domain = (char *) NULL, *version = (char *) NULL;
    DBObjData *outData = (DBObjData *) NULL, *inData = (DBObjData *) NULL, *basinData = (DBObjData *) NULL;
    DBInt DBGridCont2Network(DBObjData *, DBObjData *, bool, bool);

    for (argPos = 1; argPos < argNum;) {
        if (CMargTest(argv[argPos], "-b", "--build")) {
            if ((argNum = CMargShiftLeft(argPos, argv, argNum)) <= argPos) {
                CMmsgPrint(CMmsgUsrError, "Missing build option!");
                return (CMfailed);
            }
            else {
                const char *options[] = {"yes", "no", (char *) NULL};
                bool methods[] = {true, false};
                DBInt code;

                if ((code = CMoptLookup(options, argv[argPos], false)) == CMfailed) {
                    CMmsgPrint(CMmsgWarning, "Ignoring ill formed build option [%s]!", argv[argPos]);
                }
                else build = methods[code];
            }
            if ((argNum = CMargShiftLeft(argPos, argv, argNum)) <= argPos) break;
            continue;
        }
        if (CMargTest(argv[argPos], "-g", "--gradient")) {
            if ((argNum = CMargShiftLeft(argPos, argv, argNum)) <= argPos) {
                CMmsgPrint(CMmsgUsrError, "Missing gradient method!");
                return (CMfailed);
            }
            else {
                const char *options[] = {"down", "up", (char *) NULL};
                bool methods[] = {true, false};
                DBInt code;

                if ((code = CMoptLookup(options, argv[argPos], false)) == CMfailed) {
                    CMmsgPrint(CMmsgWarning, "Ignoring illformed gradient method [%s]!", argv[argPos]);
                }
                else downhill = methods[code];
            }
            if ((argNum = CMargShiftLeft(argPos, argv, argNum)) <= argPos) break;
            continue;
        }
        if (CMargTest(argv[argPos], "-p", "--basin_pack")) {
            if ((argNum = CMargShiftLeft(argPos, argv, argNum)) <= argPos) {
                CMmsgPrint(CMmsgUsrError, "Missing basin pack filename!");
                return (CMfailed);
            }
            else {
                if (basinData != (DBObjData *) NULL)
                    CMmsgPrint(CMmsgWarning, "Ignoring redefined basin pack");
                else {
                    basinData = new DBObjData();
                    if (basinData->Read(argv[argPos]) == DBFault) {
                        CMmsgPrint(CMmsgUsrError, "Basin data reading error");
                        delete basinData;
                        basinData = (DBObjData *) NULL;
                        return (CMfailed);
                    }
                }
            }
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
        if (CMargTest (argv[argPos], "-h", "--help")) {
            _CMDprintUsage(argv[0]);
            return (DBSuccess);
        }
        if ((argv[argPos][0] == '-') && ((int) strlen(argv[argPos]) > 1)) {
            CMmsgPrint(CMmsgUsrError, "Unknown option: %s!", argv[argPos]);
            return (CMfailed);
        }
        argPos++;
    }

    if (argNum > 3) {
        CMmsgPrint(CMmsgUsrError, "Extra arguments!");
        _CMDprintUsage (argv[0]);
        return (CMfailed);
    }
    inData = new DBObjData();
    ret = (argNum > 1) && (strcmp(argv[1], "-") != 0) ? inData->Read(argv[1]) : inData->Read(stdin);
    if ((ret == DBFault) || (inData->Type() != DBTypeGridContinuous)) {
        ret = DBFault;
        goto Stop;
    }
    inData->LinkedData(basinData);

    if (title == (char *) NULL) title = (char *) "Regridded Network";
    if (domain == (char *) NULL) domain = inData->Document(DBDocGeoDomain);
    if (version == (char *) NULL) version = (char *) "0.01pre";

    outData = new DBObjData(title, DBTypeNetwork);
    outData->Document(DBDocSubject, "STNetwork");
    outData->Document(DBDocGeoDomain, domain);
    outData->Document(DBDocVersion, version);

    if (DBGridCont2Network(inData, outData, downhill, build) == DBFault) {
        CMmsgPrint(CMmsgUsrError, "Grid create network failed!");
        ret = DBFault;
        goto Stop;
    }

    ret = (argNum > 2) && (strcmp(argv[2], "-") != 0) ? outData->Write(argv[2]) : outData->Write(stdout);

    Stop:
    if (basinData != (DBObjData *) NULL) delete basinData;
    if (inData != (DBObjData *) NULL) delete inData;
    if (outData != (DBObjData *) NULL) delete outData;
    return (ret);
}
