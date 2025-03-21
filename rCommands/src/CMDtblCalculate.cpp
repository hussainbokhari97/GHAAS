/******************************************************************************

GHAAS RiverGIS Utilities V3.0
Global Hydrological Archive and Analysis System
Copyright 1994-2024, UNH - CCNY

CMDtblCalculate.cpp

bfekete@ccny.cuny.edu

*******************************************************************************/

#include <cm.h>
#include <DB.hpp>
#include <RG.hpp>
#include <CMDexp.hpp>

static void _CMDprintUsage (const char *arg0) {
    CMmsgPrint(CMmsgInfo, "%s [options] <input file> <output file>", CMfileName(arg0));
    CMmsgPrint(CMmsgInfo, "     -a, --table     [table name]");
    CMmsgPrint(CMmsgInfo, "     -f, --field     [fieldname] [expression]");
    CMmsgPrint(CMmsgInfo, "     -e, --ephemeral [fieldname] [expression]");
    CMmsgPrint(CMmsgInfo, "     -t, --title     [dataset title]");
    CMmsgPrint(CMmsgInfo, "     -u, --subject   [subject]");
    CMmsgPrint(CMmsgInfo, "     -d, --domain    [domain]");
    CMmsgPrint(CMmsgInfo, "     -v, --version   [version]");
    CMmsgPrint(CMmsgInfo, "     -h, --help");
}

int main(int argc, char *argv[]) {
    int argPos, argNum = argc, ret;
    DBInt expr, expNum = 0, tmpVar;
    char *tableName = (char *) NULL;
    char *fieldName = (char *) NULL;
    char *title = (char *) NULL,  *subject = (char *) NULL;
    char *domain = (char *) NULL, *version = (char *) NULL;
    CMDExpression **expressions = (CMDExpression **) NULL;
    DBInt recID;
    DBObjectLIST<DBObject> *variables = new DBObjectLIST<DBObject>("Variables");
    DBObjData *data;
    DBObjTable *table;
    DBObjRecord *record;

    if (argNum < 2) goto Help;

    for (argPos = 1; argPos < argNum;) {
        if (CMargTest (argv[argPos], "-a", "--table")) {
            if ((argNum = CMargShiftLeft(argPos, argv, argNum)) < argPos) {
                CMmsgPrint(CMmsgUsrError, "Missing table name!");
                return (CMfailed);
            }
            tableName = argv[argPos];
            if ((argNum = CMargShiftLeft(argPos, argv, argNum)) < argPos) break;
            continue;
        }
        if (CMargTest (argv[argPos], "-f", "--field") || CMargTest (argv[argPos], "-e", "--ephemeral")) {
            tmpVar = CMargTest (argv[argPos], "-e", "--ephemeral") ? true : false;
            if ((argNum = CMargShiftLeft(argPos, argv, argNum)) < argPos) {
                CMmsgPrint(CMmsgUsrError, "Missing field name!");
                return (CMfailed);
            }
            fieldName = argv[argPos];
            if ((argNum = CMargShiftLeft(argPos, argv, argNum)) < argPos) {
                CMmsgPrint(CMmsgUsrError, "Missing expression!");
                return (CMfailed);
            }
            expressions = expNum < 1 ? (CMDExpression **) calloc(1, sizeof(CMDExpression *)) :
                          (CMDExpression **) realloc(expressions, (expNum + 1) * sizeof(CMDExpression *));
            if (expressions == (CMDExpression **) NULL) {
                CMmsgPrint(CMmsgSysError, "Memory Allocation error in: %s %d", __FILE__, __LINE__);
                return (CMfailed);
            }
            expressions[expNum] = new CMDExpression(fieldName, argv[argPos], tmpVar);
            if ((expressions[expNum])->Expand(variables) == DBFault) {
                CMmsgPrint(CMmsgUsrError, "Invalid Expression!");
                return (CMfailed);
            }
            expNum++;
            if ((argNum = CMargShiftLeft(argPos, argv, argNum)) < argPos) break;
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
        if (argNum > 1) CMmsgPrint(CMmsgUsrError, "File error in: %s", argv[1]);
        return (DBFault);
    }
    if (title   != (char *) NULL) data->Name(title);
    if (subject != (char *) NULL) data->Document(DBDocSubject,   subject);
    if (domain  != (char *) NULL) data->Document(DBDocGeoDomain, domain);
    if (version != (char *) NULL) data->Document(DBDocVersion,   version);

    if (tableName == (char *) NULL) tableName = DBrNItems;

    if ((table = data->Table(tableName)) == (DBObjTable *) NULL) {
        CMmsgPrint(CMmsgUsrError, "Invalid table!");
        delete data;
        return (CMfailed);
    }

    for (expr = 0; expr < expNum; ++expr)
        if (expressions[expr]->Configure(table) == DBFault) {
            CMmsgPrint(CMmsgUsrError, "Invalid expression");
            return (CMfailed);
        }

    for (recID = 0; recID < table->ItemNum(); ++recID) {
        record = table->Item(recID);
        for (expr = 0; expr < expNum; ++expr) expressions[expr]->Evaluate(record);
    }
    for (expr = 0; expr < expNum; ++expr) delete expressions[expr];

    ret = (argNum > 2) && (strcmp(argv[2], "-") != 0) ? data->Write(argv[2]) : data->Write(stdout);

    free(expressions);
    delete data;
    delete variables;
    return (ret);
}
