/******************************************************************************

GHAAS RiverGIS Utilities V3.0
Global Hydrological Archive and Analysis System
Copyright 1994-2024, UNH - CCNY

CMDtblMergeCoord.cpp

bfekete@ccny.cuny.edu

*******************************************************************************/

#include <cm.h>
#include <DB.hpp>
#include <RG.hpp>

static void _CMDprintUsage (const char *arg0) {
    CMmsgPrint(CMmsgInfo, "%s [options] <input file> <output file>", CMfileName(arg0));
    CMmsgPrint(CMmsgInfo, "     -a, --table     [table name]");
    CMmsgPrint(CMmsgInfo, "     -f, --field     [field name]");
    CMmsgPrint(CMmsgInfo, "     -d, --degree    [degree field]");
    CMmsgPrint(CMmsgInfo, "     -m, --minute    [minute field]");
    CMmsgPrint(CMmsgInfo, "     -s, --second    [secon field]");
    CMmsgPrint(CMmsgInfo, "     -h, --help");
}

int main(int argc, char *argv[]) {
    int argPos, argNum = argc, ret;
    DBInt recID;
    int deg, min, sec;
    char coordStr[DBStringLength];
    char *tableName = (char *) NULL;
    char *fieldName = (char *) NULL;
    char *degFieldName = (char *) NULL;
    char *minFieldName = (char *) NULL;
    char *secFieldName = (char *) NULL;
    DBObjData *data;
    DBObjTable *table;
    DBObjTableField *dstField;
    DBObjTableField *degField = (DBObjTableField *) NULL;
    DBObjTableField *minField = (DBObjTableField *) NULL;
    DBObjTableField *secField = (DBObjTableField *) NULL;
    DBObjRecord *record;

    if (argNum < 2) goto Help;

    for (argPos = 1; argPos < argNum;) {
        if (CMargTest (argv[argPos], "-a", "--table")) {
            if ((argNum = CMargShiftLeft(argPos, argv, argNum)) <= argPos) {
                CMmsgPrint(CMmsgUsrError, "Missing table name!");
                return (CMfailed);
            }
            tableName = argv[argPos];
            if ((argNum = CMargShiftLeft(argPos, argv, argNum)) <= argPos) break;
            continue;
        }
        if (CMargTest (argv[argPos], "-f", "--field")) {
            if ((argNum = CMargShiftLeft(argPos, argv, argNum)) <= argPos) {
                CMmsgPrint(CMmsgUsrError, "Missing field name!");
                return (CMfailed);
            }
            fieldName = argv[argPos];
            if ((argNum = CMargShiftLeft(argPos, argv, argNum)) <= argPos) break;
            continue;
        }
        if (CMargTest (argv[argPos], "-d", "--degree")) {
            if ((argNum = CMargShiftLeft(argPos, argv, argNum)) <= argPos) {
                CMmsgPrint(CMmsgUsrError, "Missing degree field name!");
                return (CMfailed);
            }
            degFieldName = argv[argPos];
            if ((argNum = CMargShiftLeft(argPos, argv, argNum)) <= argPos) break;
            continue;
        }
        if (CMargTest (argv[argPos], "-m", "--minute")) {
            if ((argNum = CMargShiftLeft(argPos, argv, argNum)) <= argPos) {
                CMmsgPrint(CMmsgUsrError, "Missing minutefield name!");
                return (CMfailed);
            }
            minFieldName = argv[argPos];
            if ((argNum = CMargShiftLeft(argPos, argv, argNum)) <= argPos) break;
            continue;
        }
        if (CMargTest (argv[argPos], "-s", "--second")) {
            if ((argNum = CMargShiftLeft(argPos, argv, argNum)) <= argPos) {
                CMmsgPrint(CMmsgUsrError, "Missing second field name!");
                return (CMfailed);
            }
            secFieldName = argv[argPos];
            if ((argNum = CMargShiftLeft(argPos, argv, argNum)) <= argPos) break;
            continue;
        }
Help:   if (CMargTest (argv[argPos], "-h", "--help")) {
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
    data = new DBObjData();
    if (((argNum > 1) && (strcmp(argv[1], "-") != 0) ? data->Read(argv[1]) : data->Read(stdin)) == DBFault) {
        delete data;
        return (CMfailed);
    }

    if (tableName == (char *) NULL) tableName = DBrNItems;

    if ((table = data->Table(tableName)) == (DBObjTable *) NULL) {
        CMmsgPrint(CMmsgUsrError, "Invalid table!");
        _CMDprintUsage (argv[0]);
        delete data;
        return (CMfailed);
    }

    if (degFieldName != (char *) NULL) {
        if ((degField = table->Field(degFieldName)) == (DBObjTableField *) NULL) {
            _CMDprintUsage (argv[0]);
            CMmsgPrint(CMmsgUsrError, "Invalid degree field [%s]!", degFieldName);
            delete data;
            return (CMfailed);
        }
    }

    if (minFieldName != (char *) NULL) {
        if ((minField = table->Field(minFieldName)) == (DBObjTableField *) NULL) {
            CMmsgPrint(CMmsgUsrError, "Invalid min field [%s]!", minFieldName);
            _CMDprintUsage (argv[0]);
            delete data;
            return (CMfailed);
        }
    }

    if (secFieldName != (char *) NULL) {
        if (minField == (DBObjTableField *) NULL) {
            CMmsgPrint(CMmsgUsrError, "Minute field is not set!");
            _CMDprintUsage (argv[0]);
            delete data;
            return (CMfailed);
        }
        if ((secField = table->Field(secFieldName)) == (DBObjTableField *) NULL) {
            CMmsgPrint(CMmsgUsrError, "Invalid second field [%s]", secFieldName);
            delete data;
            return (CMfailed);
        }
    }

    if (fieldName == (char *) NULL) fieldName = (char *) "Coord[ddd:mm\'ss\"]";
    if ((dstField = table->Field(fieldName)) == (DBObjTableField *) NULL) {
        dstField = new DBObjTableField(fieldName, DBTableFieldString, "%s", 16, false);
        table->AddField(dstField);
    }

    for (recID = 0; recID < table->ItemNum(); ++recID) {
        record = table->Item(recID);

        if ((degField == (DBObjTableField *) NULL) ||
            ((deg = degField->Int(record)) == degField->IntNoData()))
            strcpy(coordStr, "");
        else if ((minField == (DBObjTableField *) NULL) ||
                 ((min = minField->Int(record)) == minField->IntNoData()))
            snprintf(coordStr, sizeof(coordStr), "%4d", deg);
        else if ((secField == (DBObjTableField *) NULL) ||
                 ((sec = secField->Int(record)) == secField->IntNoData()))
            snprintf(coordStr, sizeof(coordStr), "%4d:%02d", deg, min);
        else snprintf(coordStr, sizeof(coordStr), "%4d:%02d\'%02d\"", deg, min, sec);
        dstField->String(record, coordStr);
    }

    ret = (argNum > 2) && (strcmp(argv[2], "-") != 0) ? data->Write(argv[2]) : data->Write(stdout);

    delete data;
    return (ret);
}
