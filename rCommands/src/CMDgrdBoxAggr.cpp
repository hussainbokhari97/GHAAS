/******************************************************************************

GHAAS RiverGIS Utilities V3.0
Global Hydrological Archive and Analysis System
Copyright 1994-2024, UNH - CCNY

CMDgrdBoxAggr.cpp

bfekete@ccny.cuny.edu

*******************************************************************************/

#include <cm.h>
#include <math.h>
#include <DB.hpp>
#include <DBif.hpp>
#include <RG.hpp>

typedef enum {CMDboxAverage, CMDboxMinimum, CMDboxMaximum, CMDboxSum} CMDboxMethod;
typedef enum {CMDboxWeightArea, CMDboxWeightCellNum} CMDboxWeight;

static void _CMDprintUsage (const char *arg0) {
    CMmsgPrint(CMmsgInfo, "%s [options] <input file> <output file>", CMfileName(arg0));
    CMmsgPrint(CMmsgInfo, "     -z, --size      [box size]");
    CMmsgPrint(CMmsgInfo, "     -m, --method    [avg|min|max|sum]");
    CMmsgPrint(CMmsgInfo, "     -w, --weight    [area|cellnum]");
    CMmsgPrint(CMmsgInfo, "     -t, --title     [dataset title]");
    CMmsgPrint(CMmsgInfo, "     -u, --subject   [subject]");
    CMmsgPrint(CMmsgInfo, "     -d, --domain    [domain]");
    CMmsgPrint(CMmsgInfo, "     -v, --version   [version]");
    CMmsgPrint(CMmsgInfo, "     -s, --shadeset  [standard|grey|blue|blue-to-red|elevation]");
    CMmsgPrint(CMmsgInfo, "     -h, --help");
}

int main(int argc, char *argv[]) {
    int argPos, argNum = argc, ret;
    CMDboxMethod method    = CMDboxAverage;
    CMDboxWeight boxWeight = CMDboxWeightArea;
    DBInt kernelSize = 2, layerID;
    char *title = (char *) NULL, *subject = (char *) NULL;
    char *domain = (char *) NULL, *version = (char *) NULL;
    int shadeSet = DBDataFlagDispModeContGreyScale;
    DBFloat var, *array = (DBFloat *) NULL, *sumWeights = (DBFloat *) NULL, *misWeights = (DBFloat *) NULL, cellArea;
    DBRegion extent;
    DBPosition inPos, outPos;
    DBCoordinate coord, cellSize;
    DBObjData *inData, *outData;
    DBObjRecord *inLayerRec, *outLayerRec;
    DBGridIF *inGridIF, *outGridIF;

    for (argPos = 1; argPos < argNum;) {
        if (CMargTest (argv[argPos], "-z", "--size")) {
            if ((argNum = CMargShiftLeft(argPos, argv, argNum)) <= argPos) {
                CMmsgPrint(CMmsgUsrError, "Missing kernel size!");
                return (CMfailed);
            }
            if (sscanf(argv[argPos], "%d", &kernelSize) != 1) {
                CMmsgPrint(CMmsgUsrError, "Invalid kernel size!");
                return (CMfailed);
            }
            if ((argNum = CMargShiftLeft(argPos, argv, argNum)) <= argPos) break;
            continue;
        }
        if (CMargTest(argv[argPos], "-m", "--method")) {
            if ((argNum = CMargShiftLeft(argPos, argv, argNum)) <= argPos) {
                CMmsgPrint(CMmsgUsrError, "Missing aggregate method!");
                return (CMfailed);
            }
            else {
                const char *options[] = {"avg", "min", "max", "sum", (char *) NULL};
                CMDboxMethod methods[] = {CMDboxAverage, CMDboxMinimum, CMDboxMaximum, CMDboxSum};
                DBInt code;

                if ((code = CMoptLookup(options, argv[argPos], false)) == CMfailed) {
                    CMmsgPrint(CMmsgWarning, "Ignoring illformed aggregate method [%s]!", argv[argPos]);
                }
                else method = methods[code];
            }
            if ((argNum = CMargShiftLeft(argPos, argv, argNum)) <= argPos) break;
            continue;
        }
        if (CMargTest(argv[argPos], "-w", "--weight")) {
            if ((argNum = CMargShiftLeft(argPos, argv, argNum)) <= argPos) {
                CMmsgPrint(CMmsgUsrError, "Missing aggregate method!");
                return (CMfailed);
            }
            else {
                const char *options[] = {"area", "cellnum", (char *) NULL};
                CMDboxWeight methods[] = { CMDboxWeightArea, CMDboxWeightCellNum};
                DBInt code;

                if ((code = CMoptLookup(options, argv[argPos], false)) == CMfailed) {
                    CMmsgPrint(CMmsgWarning, "Ignoring illformed aggregate method [%s]!", argv[argPos]);
                }
                else boxWeight = methods[code];
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
        delete inData;
        return (CMfailed);
    }

    if (title == (char *) NULL) title = (char *) "Box Aggregated";
    if (subject == (char *) NULL) subject = inData->Document(DBDocSubject);
    if (domain == (char *) NULL) domain = inData->Document(DBDocGeoDomain);
    if (version == (char *) NULL) version = (char *) "0.01pre";
    if (shadeSet == DBFault) shadeSet = DBDataFlagDispModeContGreyScale;

     inGridIF = new DBGridIF(inData);

    extent = inData->Extent();
    cellSize.X = (extent.UpperRight.X - extent.LowerLeft.X) / (inGridIF->ColNum() / kernelSize);
    cellSize.Y = (extent.UpperRight.Y - extent.LowerLeft.Y) / (inGridIF->RowNum() / kernelSize);

    if ((outData = DBGridCreate (title, extent, cellSize, inData->Type (), inGridIF->ValueType(), inGridIF->ValueSize())) == (DBObjData *) NULL) return (CMfailed);
    outData->Name(title);
    outData->Document(DBDocSubject, subject);
    outData->Document(DBDocGeoDomain, domain);
    outData->Document(DBDocVersion, version);
    outData->Flags(DBDataFlagDispModeContShadeSets, DBClear);
    outData->Flags(shadeSet, DBSet);
    outData->Projection(inData->Projection());
    outGridIF = new DBGridIF(outData);
    outGridIF->MissingValue(inGridIF->MissingValue());
    if ((sumWeights = (DBFloat *) calloc(outGridIF->ColNum() * outGridIF->RowNum(), sizeof(DBFloat))) ==
        (DBFloat *) NULL) {
        CMmsgPrint(CMmsgSysError, "Memory allocation error in %s:%d", __FILE__, __LINE__);
        return (CMfailed);
    }
    if ((misWeights = (DBFloat *) calloc(outGridIF->ColNum() * outGridIF->RowNum(), sizeof(DBFloat))) ==
        (DBFloat *) NULL) {
        CMmsgPrint(CMmsgSysError, "Memory allocation error in %s:%d", __FILE__, __LINE__);
        return (CMfailed);
    }
    if ((array = (DBFloat *) calloc(outGridIF->ColNum() * outGridIF->RowNum(), sizeof(DBFloat))) == (DBFloat *) NULL) {
        CMmsgPrint(CMmsgSysError, "Memory allocation error in %s:%d", __FILE__, __LINE__);
        return (CMfailed);
    }

    for (layerID = 0; layerID < inGridIF->LayerNum(); ++layerID) {
        inLayerRec = inGridIF->Layer(layerID);
        if (layerID == 0) {
            outLayerRec = outGridIF->Layer(layerID);
            outGridIF->RenameLayer(outLayerRec, inLayerRec->Name());
        }
        else
            outLayerRec = outGridIF->AddLayer(inLayerRec->Name());
        for (outPos.Row = 0; outPos.Row < outGridIF->RowNum(); ++outPos.Row)
            for (outPos.Col = 0; outPos.Col < outGridIF->ColNum(); ++outPos.Col) {
                sumWeights[outPos.Row * outGridIF->ColNum() + outPos.Col] = 0.0;
                misWeights[outPos.Row * outGridIF->ColNum() + outPos.Col] = 0.0;
                switch (method) {
                    case CMDboxSum:
                    case CMDboxAverage:
                        array[outPos.Row * outGridIF->ColNum() + outPos.Col] = 0.0;
                        break;
                    case CMDboxMinimum:
                        array[outPos.Row * outGridIF->ColNum() + outPos.Col] = HUGE_VAL;
                        break;
                    case CMDboxMaximum:
                        array[outPos.Row * outGridIF->ColNum() + outPos.Col] = -HUGE_VAL;
                        break;
                }
            }
        for (inPos.Row = 0; inPos.Row < inGridIF->RowNum(); ++inPos.Row)
            for (inPos.Col = 0; inPos.Col < inGridIF->ColNum(); ++inPos.Col) {
                inGridIF->Pos2Coord(inPos, coord);
                outGridIF->Coord2Pos(coord, outPos);
                cellArea = inGridIF->CellArea(inPos);
                if (inGridIF->Value(inLayerRec, inPos, &var)) {
                    switch (method) {
                        case CMDboxSum:
                        case CMDboxAverage:
                            if (boxWeight == CMDboxWeightArea) {
                                array[outPos.Row * outGridIF->ColNum() + outPos.Col] += var * cellArea;
                                sumWeights[outPos.Row * outGridIF->ColNum() + outPos.Col] += cellArea;
                            }
                            else {
                                array[outPos.Row * outGridIF->ColNum() + outPos.Col]      += var;
                                sumWeights[outPos.Row * outGridIF->ColNum() + outPos.Col] += 1.0;
                            }
                            break;
                        case CMDboxMinimum:
                            array[outPos.Row * outGridIF->ColNum() + outPos.Col] =
                                    var < array[outPos.Row * outGridIF->ColNum() + outPos.Col] ?
                                    var : array[outPos.Row * outGridIF->ColNum() + outPos.Col];
                            sumWeights[outPos.Row * outGridIF->ColNum() + outPos.Col] += 1.0;
                            break;
                        case CMDboxMaximum:
                            array[outPos.Row * outGridIF->ColNum() + outPos.Col] =
                                    var > array[outPos.Row * outGridIF->ColNum() + outPos.Col] ?
                                    var : array[outPos.Row * outGridIF->ColNum() + outPos.Col];
                            sumWeights[outPos.Row * outGridIF->ColNum() + outPos.Col] += 1.0;
                            break;
                    }
                }
                else {
                    if (boxWeight == CMDboxWeightArea)
                        misWeights[outPos.Row * outGridIF->ColNum() + outPos.Col] += cellArea;
                    else
                        misWeights[outPos.Row * outGridIF->ColNum() + outPos.Col] += 1.0;
                }
            }
        for (outPos.Row = 0; outPos.Row < outGridIF->RowNum(); ++outPos.Row)
            for (outPos.Col = 0; outPos.Col < outGridIF->ColNum(); ++outPos.Col) {
                if (sumWeights[outPos.Row * outGridIF->ColNum() + outPos.Col] > 0.0) {
                    var = array[outPos.Row * outGridIF->ColNum() + outPos.Col];
                    switch (method) {
                        case CMDboxAverage: var = var / ((DBFloat) sumWeights[outPos.Row * outGridIF->ColNum() + outPos.Col]); break;
                        case CMDboxSum:
                            var = var * (sumWeights[outPos.Row * outGridIF->ColNum() + outPos.Col] +
                                        misWeights[outPos.Row * outGridIF->ColNum() + outPos.Col])
                                      / (DBFloat) sumWeights[outPos.Row * outGridIF->ColNum() + outPos.Col];
                            break;
                        default: break;
                    }
                    outGridIF->Value(outLayerRec, outPos, var);
                }
                else
                    outGridIF->Value(outLayerRec, outPos, outGridIF->MissingValue());
            }
        outGridIF->RecalcStats(outLayerRec);
    }

    ret = (argNum > 2) && (strcmp(argv[2], "-") != 0) ? outData->Write(argv[2]) : outData->Write(stdout);

    if (sumWeights != (DBFloat *) NULL) free(sumWeights);
    if (misWeights != (DBFloat *) NULL) free(misWeights);
    if (array != (DBFloat *) NULL) free(array);
    return (ret);
}
