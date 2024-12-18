/******************************************************************************

GHAAS Database library V3.0
Global Hydrological Archive and Analysis System
Copyright 1994-2024, UNH - CCNY

DBPause.cpp

bfekete@ccny.cuny.edu

*******************************************************************************/

#include <DB.hpp>

static int (*_DBPauseFunction)(int) = NULL;

void DBPauseSetFunction(int (*pauseFunc)(int)) { _DBPauseFunction = pauseFunc; }

int DBPause(int perCent) {
    if (_DBPauseFunction == NULL) return (false);
    else return ((*_DBPauseFunction)(perCent));
}
