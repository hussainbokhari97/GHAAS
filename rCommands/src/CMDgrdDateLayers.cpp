/******************************************************************************

GHAAS RiverGIS Utilities V3.0
Global Hydrological Archive and Analysis System
Copyright 1994-2024, UNH - CCNY

CMDgrdDateLayers.cpp

bfekete@ccny.cuny.edu

*******************************************************************************/

#include <cm.h>
#include <DB.hpp>
#include <DBif.hpp>
#include <RG.hpp>

static DBInt modifyDate(DBObjData *dbData, int timeStep,
                        int year, int month, int day, int hour, int minute,
                        int interval) {
    DBInt layerID;
    DBDate stepDate;
    DBGridIF *gridIF = new DBGridIF(dbData);
    DBObjRecord *layerRec;
    DBDate date;
    //change this!

    switch (timeStep) {
        case DBTimeStepYear:
            stepDate.Set(interval);
            break;
        case DBTimeStepMonth:
            stepDate.Set(0, interval);
            break;
        case DBTimeStepDay:
            stepDate.Set(0, 0, interval);
            break;
        case DBTimeStepHour:
            stepDate.Set(0, 0, 0, interval);
            break;
        case DBTimeStepMinute:
            stepDate.Set(0, 0, 0, 0, interval);
            break;
    }
    if (year == DBDefaultMissingIntVal) //if the user hasnt entered a specific year
    {
        if (((stepDate.Year() > 0) && (gridIF->LayerNum() > 1)) ||
            ((stepDate.Month() > 0) && (gridIF->LayerNum() > (12 / interval))) ||
            ((stepDate.Day() > 0) && (gridIF->LayerNum() > (365 / interval))) ||
            ((stepDate.Hour() > 0) && (gridIF->LayerNum() > (365 * 24 / interval))) ||
            ((stepDate.Minute() > 0) && (gridIF->LayerNum() > 365 * 24 * 60 / interval))) {
            //todo: handle this better!
            printf("(Climatology)There seems to be more than a year's worth of layers!\n");
            printf("(Climatology)I suggest you define the year, or change the step interval\n");
            delete gridIF;
            return (CMfailed);
        }
    }
    //printf("Labeling Layers");
    //this sets the first layer to contain all of the fields even if a field is undefined.
    //I do this so (for example)the user can compare day 1 of every month of a year with day 2 of
    //every moth of a year and so on.. the first layer will indicate everything down to the
    //minute, and the next layers will omit anything that isnt different.

    date.Set(year, month, day, hour, minute);

    for (layerID = 0; layerID < gridIF->LayerNum(); ++layerID) {
        layerRec = gridIF->Layer(layerID);
        gridIF->RenameLayer(layerRec, date.Get());
        date = date + (stepDate);
    }
    delete gridIF;
    return (DBSuccess);
}

static void _CMDprintUsage (const char *arg0) {
    CMmsgPrint(CMmsgInfo, "%s [options] <input grid> <output grid>", CMfileName(arg0));
    CMmsgPrint(CMmsgInfo, "     -Y, --year      [beginning year]");
    CMmsgPrint(CMmsgInfo, "     -M, --month     [beginning month]");
    CMmsgPrint(CMmsgInfo, "     -D, --day       [beginning day]");
    CMmsgPrint(CMmsgInfo, "     -H, --hour      [beginning hour]");
    CMmsgPrint(CMmsgInfo, "     -I, --minute    [beginning minute]");
    CMmsgPrint(CMmsgInfo, "     -e, --step      [year|month|day|hour|minute]");
    CMmsgPrint(CMmsgInfo, "     -n, --number    [number of intervals]");
    CMmsgPrint(CMmsgInfo, "     -t, --title     [dataset title]");
    CMmsgPrint(CMmsgInfo, "     -u, --subject   [subject]");
    CMmsgPrint(CMmsgInfo, "     -d, --domain    [domain]");
    CMmsgPrint(CMmsgInfo, "     -v, --version   [version]");
    CMmsgPrint(CMmsgInfo, "     -s, --shadeset  [standard|grey|blue|blue-to-red|elevation]");
    CMmsgPrint(CMmsgInfo, "     -h, --help");
}

int main(int argc, char *argv[]) {
    int argPos, argNum = argc, ret;
    int startYear    = DBDefaultMissingIntVal;
    int startMonth   = DBDefaultMissingIntVal;
    int startDay     = DBDefaultMissingIntVal;
    int startHour    = DBDefaultMissingIntVal;
    int startMinute  = DBDefaultMissingIntVal;
    int timeInterval = 1; // this is defined by the -n field
    int shadeSet = DBDataFlagDispModeContGreyScale;
    char *title  = (char *) NULL, *subject = (char *) NULL;
    char *domain = (char *) NULL, *version = (char *) NULL;
    bool changeShadeSet = false;
    DBObjData *dbData;
    DBInt timeStep = DBFault;

    if (argNum < 2) goto Help;
 
    for (argPos = 1; argPos < argNum;) {
        if (CMargTest (argv[argPos], "-e", "--step")) {
            const char *timeStepStr[] = {"year", "month", "day", "hour", "minute", (char *) NULL};
            int timeStepCodes[] = {DBTimeStepYear,
                                   DBTimeStepMonth,
                                   DBTimeStepDay,
                                   DBTimeStepHour,
                                   DBTimeStepMinute};
            if ((argNum = CMargShiftLeft(argPos, argv, argNum)) <= argPos) {
                CMmsgPrint(CMmsgUsrError, "Time step variable step is not defined!");
                return (CMfailed);
            }
            if ((timeStep = CMoptLookup(timeStepStr, argv[argPos], true)) == DBFault) {
                printf("Invalid timeStep %s", argv[argPos]);
                return DBFault;
            }
            timeStep = timeStepCodes[timeStep];
            if ((argNum = CMargShiftLeft(argPos, argv, argNum)) <= argPos) break;
            continue;
        }
        if (CMargTest (argv[argPos], "-Y", "--year")) {
            if ((argNum = CMargShiftLeft(argPos, argv, argNum)) <= argPos) {
                CMmsgPrint(CMmsgUsrError, "Missing Year!");
                return (CMfailed);
            }
            if (sscanf(argv[argPos], "%d", &startYear) != 1) {
                CMmsgPrint(CMmsgUsrError, "Invalid year!");
                return (CMfailed);
            }
            if ((argNum = CMargShiftLeft(argPos, argv, argNum)) <= argPos) break;
            continue;
        }
        if (CMargTest (argv[argPos], "-M", "--month")) {
            if ((argNum = CMargShiftLeft(argPos, argv, argNum)) <= argPos) {
                CMmsgPrint(CMmsgUsrError, "Missing Month!");
                return (CMfailed);
            }
            if (sscanf(argv[argPos], "%d", &startMonth) != 1) {
                CMmsgPrint(CMmsgUsrError, "Invalid Month!");
                return (CMfailed);
            }
            startMonth--;
            if ((argNum = CMargShiftLeft(argPos, argv, argNum)) <= argPos) break;
            continue;
        }
        if (CMargTest (argv[argPos], "-D", "--day")) {
            if ((argNum = CMargShiftLeft(argPos, argv, argNum)) <= argPos) {
                CMmsgPrint(CMmsgUsrError, "Missing Day!");
                return (CMfailed);
            }
            if (sscanf(argv[argPos], "%d", &startDay) != 1) {
                CMmsgPrint(CMmsgUsrError, "Invalid day!");
                return (CMfailed);
            }
            startDay--;
            if ((argNum = CMargShiftLeft(argPos, argv, argNum)) <= argPos) break;
            continue;
        }
        if (CMargTest (argv[argPos], "-H", "--hour")) {
            if ((argNum = CMargShiftLeft(argPos, argv, argNum)) <= argPos) {
                CMmsgPrint(CMmsgUsrError, "Missing Hour!");
                return (CMfailed);
            }
            if (sscanf(argv[argPos], "%d", &startHour) != 1) {
                CMmsgPrint(CMmsgUsrError, "Invalid hour!");
                return (CMfailed);
            }
            if ((argNum = CMargShiftLeft(argPos, argv, argNum)) <= argPos) break;
            continue;
        }
        if (CMargTest (argv[argPos], "-I", "--minute")) {
            if ((argNum = CMargShiftLeft(argPos, argv, argNum)) <= argPos) {
                CMmsgPrint(CMmsgUsrError, "Missing Minute!");
                return (CMfailed);
            }
            if (sscanf(argv[argPos], "%d", &startMinute) != 1) {
                CMmsgPrint(CMmsgUsrError, "Invalid minute!");
                return (CMfailed);
            }
            if ((argNum = CMargShiftLeft(argPos, argv, argNum)) <= argPos) break;
            continue;
        }
        if (CMargTest (argv[argPos], "-n", "--number")) {
            if ((argNum = CMargShiftLeft(argPos, argv, argNum)) <= argPos) {
                CMmsgPrint(CMmsgUsrError, "Missing Number!");
                return (CMfailed);
            }
            if (sscanf(argv[argPos], "%d", &timeInterval) != 1) {
                CMmsgPrint(CMmsgUsrError, "Invalid interval!");
                return (CMfailed);
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
            changeShadeSet = true;
            if ((argNum = CMargShiftLeft(argPos, argv, argNum)) <= argPos) break;
            continue;
        }
Help:   if (CMargTest (argv[argPos], "-h", "--help")) {
            _CMDprintUsage(argv[0]);
            return (DBSuccess);
        }
        if ((argv[argPos][0] == '-') && (strlen(argv[argPos]) > 1)) {
            CMmsgPrint(CMmsgUsrError, "Unknown option: %s!", argv[argPos]);
            return (CMfailed);
        }
        argPos++;
    }

    switch (timeStep) {
        case DBFault:
            CMmsgPrint(CMmsgUsrError, "Missing time step!");
            return (CMfailed);
        case DBTimeStepMinute:
            if (startMinute == DBDefaultMissingIntVal) startMinute = 0;
        case DBTimeStepHour:
            if (startHour   == DBDefaultMissingIntVal) startHour = 0;
        case DBTimeStepDay:
            if (startDay    == DBDefaultMissingIntVal) startDay = 0;
        case DBTimeStepMonth:
            if (startMonth  == DBDefaultMissingIntVal) startMonth = 0;
        default:
            break;
    }

    if (argNum > 3) {
        CMmsgPrint(CMmsgUsrError, "Extra arguments!");
        _CMDprintUsage (argv[0]);
        return (CMfailed);
    }
    dbData = new DBObjData();
    ret = (argNum > 1) && (strcmp(argv[1], "-") != 0) ? dbData->Read(argv[1]) : dbData->Read(stdin);
    if ((ret == DBFault) ||
        ((dbData->Type() != DBTypeGridContinuous) &&
         (dbData->Type() != DBTypeGridDiscrete))) {
        delete dbData;
        return (CMfailed);
    }
    if (title   != (char *) NULL) dbData->Name(title);
    if (subject != (char *) NULL) dbData->Document(DBDocSubject,   subject);
    if (domain  != (char *) NULL) dbData->Document(DBDocGeoDomain, domain);
    if (version != (char *) NULL) dbData->Document(DBDocVersion,   version);
    if (shadeSet == DBFault) shadeSet = DBDataFlagDispModeContGreyScale;
    if (changeShadeSet && (dbData->Type() == DBTypeGridContinuous)) {
        dbData->Flags(DBDataFlagDispModeContShadeSets, DBClear);
        dbData->Flags(shadeSet, DBSet);
    }
    if (modifyDate(dbData, timeStep, startYear, startMonth, startDay, startHour, startMinute, timeInterval)
        == DBSuccess)
        ret = (argNum > 2) && (strcmp(argv[2], "-") != 0) ? dbData->Write(argv[2]) : dbData->Write(stdout);

    delete dbData;
    return (ret);
}
