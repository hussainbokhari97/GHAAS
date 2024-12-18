/******************************************************************************

GHAAS RiverGIS Libarary V3.0
Global Hydrological Archive and Analysis System
Copyright 1994-2024, UNH - CCNY

RGlibPoint.cpp

bfekete@ccny.cuny.edu

*******************************************************************************/

#include <DB.hpp>
#include <DBif.hpp>
#include <RG.hpp>
#include <math.h>

DBInt RGlibPointSTNCoordinates(DBObjData *dbData, DBObjTableField *pField, DBObjTableField *cField, DBFloat tolerance, DBFloat radius, bool adaptive) {
    DBInt pointID, ret = DBFault, valCount = 0, pntCount = 0, pRadius, maxRadius;
    DBFloat relDiff, cVal, tVal, min = HUGE_VAL, max = -HUGE_VAL, cellLength = 0.0;
    DBCoordinate coord;
    DBPosition pos;
    DBObjData *linkedData = dbData->LinkedData();
    DBVPointIF *pntIF;
    DBNetworkIF *netIF;
    DBObjRecord *pntRec, *cellRec;

    if ((linkedData == (DBObjData *) NULL) && (linkedData->Type() != DBTypeNetwork)) return (DBFault);
    pntIF = new DBVPointIF(dbData);
    netIF = new DBNetworkIF(linkedData);
    if (pField != (DBObjTableField *) NULL) {
        for (pointID = 0; pointID < pntIF->ItemNum(); ++pointID) {
            pntRec = pntIF->Item(pointID);
            tVal = pField->Float(pntRec);
            if ((cellRec = netIF->Cell(pntIF->Coordinate(pntRec))) != (DBObjRecord *) NULL) {
                cellLength += netIF->CellLength(cellRec);
                pntCount++;
            }
            if (!CMmathEqualValues(tVal, pField->FloatNoData())) {
                if (min > tVal) min = tVal;
                if (max < tVal) max = tVal;
                valCount++;
            }
        }
        if (min <= CMmathEpsilon) min = CMmathEpsilon;
        if (max <= min) valCount = 0;
        if (pntCount > 0) { cellLength = cellLength / pntCount; maxRadius = (DBInt) ceil (radius / cellLength); }
        if (valCount > 0) { max = log(max); min = log(min); }
    }
    for (pointID = 0; pointID < pntIF->ItemNum(); ++pointID) {
        pntRec = pntIF->Item(pointID);
        if (DBPause(pointID * 100 / pntIF->ItemNum())) goto Stop;
        if ((pntRec->Flags () & DBObjectFlagIdle) == DBObjectFlagIdle) continue;
        coord = pntIF->Coordinate(pntRec);
        if (netIF->Coord2Pos(coord, pos) == DBFault) continue;
        netIF->Pos2Coord(pos, coord);
        if (pField != (DBObjTableField *) NULL) {
            if (CMmathEqualValues(tVal = pField->Float(pntRec), pField->FloatNoData())) {
                if (tolerance <= 0.0) {
                    // When tolerance is zero or negative searching for largest value irrespective ot the target value
                    if ((cellRec = netIF->Cell (coord, cField, tVal, maxRadius, tolerance)) != (DBObjRecord *) NULL) {
                        coord = netIF->Center(cellRec);
                    }
                } // else do nothing.
            } else {
                if (valCount > 0 && tolerance > 0.0) {
                    if ((cellRec = netIF->Cell(pos)) != (DBObjRecord *) NULL) {
                        cVal = cField->Float(cellRec);
                        relDiff = fabs(cVal - tVal) / (cVal + tVal);
                        if (relDiff < tolerance * (1.0 - tolerance)) continue; 
                    }
                    pRadius = adaptive ? (DBInt) ceil ((float) maxRadius * (log(tVal) - min) / (max - min)) : maxRadius;
                    if ((cellRec = netIF->Cell (coord, cField, tVal, pRadius, tolerance)) != (DBObjRecord *) NULL) {
                        cVal = cField->Float(cellRec);
                        relDiff = fabs(cVal - tVal) / (cVal + tVal);
                        if (relDiff < tolerance) { 
                            coord = netIF->Center(cellRec);
                        }
                    }
                } // else do nothing
            }
        }
        pntIF->Coordinate(pntRec, coord);
    }
    ret = DBSuccess;
    Stop:
    delete netIF;
    delete pntIF;
    return (ret);
}

#define RGlibTEMPPointID "TEMPPointID"
static DBObjTableField *_RGlibTEMPPointIDFLD;

static DBInt _RGlibSetPointID(DBNetworkIF *netIF, DBObjRecord *cellRec, DBInt pointID) {
    _RGlibTEMPPointIDFLD->Int(cellRec, pointID);
    netIF = (DBNetworkIF *) NULL;
    return (true);
}

DBInt RGlibPointSTNCharacteristics(DBObjData *dbData) {
    DBInt i, pointID, dPointID, cellID, mouthID, basinID, color, ret = DBFault, dir;
    DBVPointIF *pntIF;
    DBObjTable *pointTable, *cellTable;
    DBObjTableField *cellIDFLD;
    DBObjTableField *basinFLD;
    DBObjTableField *basinNameFLD;
    DBObjTableField *orderFLD;
    DBObjTableField *colorFLD;
    DBObjTableField *basinCellsFLD;
    DBObjTableField *basinLengthFLD;
    DBObjTableField *basinAreaFLD;
    DBObjTableField *interAreaFLD;
    DBObjTableField *nextStationFLD;
    DBObjData *netData;
    DBNetworkIF *netIF;
    DBObjRecord *pointRec, *dPointRec, *cellRec, *fromCell, *basinRec;

    if ((netData = dbData->LinkedData()) == (DBObjData *) NULL) return (DBFault);
    pointTable = dbData->Table(DBrNItems);
    pntIF = new DBVPointIF(dbData);
    netIF = new DBNetworkIF(netData);
    cellTable = netData->Table(DBrNCells);
    if ((cellIDFLD = pointTable->Field(RGlibCellID)) == NULL) {
        cellIDFLD = new DBObjTableField(RGlibCellID, DBTableFieldInt, "%8d", sizeof(DBInt));
        pointTable->AddField(cellIDFLD);
        DBPause(1);
    }
    if ((basinFLD = pointTable->Field(DBrNBasin)) == NULL) {
        basinFLD = new DBObjTableField(DBrNBasin, DBTableFieldInt, "%8d", sizeof(DBInt));
        pointTable->AddField(basinFLD);
        DBPause(2);
    }
    if ((basinNameFLD = pointTable->Field(RGlibBasinName)) == NULL) {
        basinNameFLD = new DBObjTableField(RGlibBasinName, DBTableFieldString, "%32s", DBStringLength);
        pointTable->AddField(basinNameFLD);
        DBPause(3);
    }
    if ((orderFLD = pointTable->Field(DBrNOrder)) == NULL) {
        orderFLD = new DBObjTableField(DBrNOrder, DBTableFieldInt, "%3d", sizeof(DBByte));
        pointTable->AddField(orderFLD);
        DBPause(4);
    }
    if ((colorFLD = pointTable->Field(RGlibColor)) == NULL) {
        colorFLD = new DBObjTableField(RGlibColor, DBTableFieldInt, "%2d", sizeof(DBShort));
        pointTable->AddField(colorFLD);
        DBPause(5);
    }
    if ((basinCellsFLD = pointTable->Field(RGlibCellNum)) == NULL) {
        basinCellsFLD = new DBObjTableField(RGlibCellNum, DBTableFieldInt, "%8d", sizeof(DBInt));
        pointTable->AddField(basinCellsFLD);
        DBPause(6);
    }
    if ((basinLengthFLD = pointTable->Field(RGlibLength)) == NULL) {
        basinLengthFLD = new DBObjTableField(RGlibLength, DBTableFieldFloat, "%10.1f", sizeof(DBFloat4));
        pointTable->AddField(basinLengthFLD);
        DBPause(7);
    }
    if ((basinAreaFLD = pointTable->Field(RGlibArea)) == NULL) {
        basinAreaFLD = new DBObjTableField(RGlibArea, DBTableFieldFloat, "%10.1f", sizeof(DBFloat4));
        pointTable->AddField(basinAreaFLD);
        DBPause(8);
    }
    if ((interAreaFLD = pointTable->Field(RGlibInterStation)) == NULL) {
        interAreaFLD = new DBObjTableField(RGlibInterStation, DBTableFieldFloat, "%10.1f", sizeof(DBFloat4));
        pointTable->AddField(interAreaFLD);
        DBPause(9);
    }
    if ((nextStationFLD = pointTable->Field(RGlibNextStation)) == NULL) {
        nextStationFLD = new DBObjTableField(RGlibNextStation, DBTableFieldInt, "%8d", sizeof(DBInt));
        pointTable->AddField(nextStationFLD);
        DBPause(10);
    }
    if ((_RGlibTEMPPointIDFLD = cellTable->Field(RGlibTEMPPointID)) == NULL) {
        _RGlibTEMPPointIDFLD = new DBObjTableField(RGlibTEMPPointID, DBTableFieldInt, "%8d", sizeof(DBInt));
        cellTable->AddField(_RGlibTEMPPointIDFLD);
    }
    for (pointID = 0; pointID < pointTable->ItemNum(); pointID++) {
        pointRec = pointTable->Item(pointID);
        if (DBPause(10 + pointID * 10 / pointTable->ItemNum())) goto Stop;
        if ((pointRec->Flags() & DBObjectFlagIdle) == DBObjectFlagIdle) {
            cellIDFLD->Int(pointRec, cellIDFLD->IntNoData());
            basinFLD->Int(pointRec, basinFLD->IntNoData());
            basinNameFLD->String(pointRec, "");
            orderFLD->Int(pointRec, orderFLD->IntNoData());
            colorFLD->Int(pointRec, colorFLD->IntNoData());
            basinCellsFLD->Int(pointRec, basinCellsFLD->IntNoData());
            basinAreaFLD->Float(pointRec, basinAreaFLD->FloatNoData());
            interAreaFLD->Float(pointRec, interAreaFLD->FloatNoData());
            continue;
        }
        if ((cellRec = netIF->Cell(pntIF->Coordinate(pointRec))) == (DBObjRecord *) NULL) {
            cellIDFLD->Int(pointRec, 0);
            basinFLD->Int(pointRec, 0);
            basinNameFLD->String(pointRec, "Water");
            orderFLD->Int(pointRec, colorFLD->IntNoData());
            colorFLD->Int(pointRec, colorFLD->IntNoData());
            basinCellsFLD->Int(pointRec, 0);
            basinAreaFLD->Float(pointRec, 0.0);
            interAreaFLD->Float(pointRec, 0.0);
        }
        else {
            cellIDFLD->Int(pointRec, cellRec->RowID() + 1);
            basinRec = netIF->Basin(cellRec);
            basinFLD->Int(pointRec, basinRec->RowID() + 1);
            basinNameFLD->String(pointRec, basinRec->Name());
            orderFLD->Int(pointRec, netIF->CellOrder(cellRec));
            colorFLD->Int(pointRec, 0);
            basinCellsFLD->Int(pointRec, netIF->CellBasinCells(cellRec));
            basinLengthFLD->Float(pointRec, netIF->CellBasinLength(cellRec));
            basinAreaFLD->Float(pointRec, netIF->CellBasinArea(cellRec));
            interAreaFLD->Float(pointRec, netIF->CellBasinArea(cellRec));
        }
        nextStationFLD->Int(pointRec, 0);
    }
    for (cellID = 0; cellID < cellTable->ItemNum(); ++cellID) {
        if (DBPause(20 + cellID * 20 / cellTable->ItemNum())) goto Stop;
        cellRec = cellTable->Item(cellID);
        _RGlibTEMPPointIDFLD->Int(cellRec, DBFault);
    }
    pointTable->ListSort(basinAreaFLD);
    for (pointRec = pointTable->Last(); pointRec != (DBObjRecord *) NULL; pointRec = pointTable->Next(DBBackward)) {
        if ((pointRec->Flags() & DBObjectFlagIdle) == DBObjectFlagIdle) continue;
        if (DBPause(40 + pointID * 20 / pointTable->ItemNum())) goto Stop;
        cellRec = netIF->Cell(pntIF->Coordinate(pointRec));
        netIF->UpStreamSearch(cellRec, (DBNetworkACTION) _RGlibSetPointID,
                              (void *) ((char *) NULL + pointRec->RowID()));
    }
    for (pointID = 0; pointID < pointTable->ItemNum(); pointID++) {
        pointRec = pointTable->Item(pointID);
        if ((pointRec->Flags() & DBObjectFlagIdle) == DBObjectFlagIdle) continue;
        if (DBPause(60 + pointID * 20 / pointTable->ItemNum())) goto Stop;
        if ((cellRec = netIF->Cell(pntIF->Coordinate(pointRec))) != (DBObjRecord *) NULL) {
            if ((cellRec = netIF->ToCell(cellRec)) == (DBObjRecord *) NULL) continue;
            if ((dPointID = _RGlibTEMPPointIDFLD->Int(cellRec)) != DBFault) {
                dPointRec = pointTable->Item(dPointID);
                nextStationFLD->Int(pointRec, dPointRec->RowID() + 1);
                interAreaFLD->Float(dPointRec, interAreaFLD->Float(dPointRec) - basinAreaFLD->Float(pointRec));
            }
        }
    }
    pointTable->ListSort(interAreaFLD);

    i = 0;
    for (pointRec = pointTable->Last(); pointRec != (DBObjRecord *) NULL; pointRec = pointTable->Next(DBBackward)) {
        if (DBPause(80 + (i++) * 20 / pointTable->ItemNum())) goto Stop;
        pointID = pointRec->RowID();
        if ((pointRec->Flags() & DBObjectFlagIdle) == DBObjectFlagIdle) continue;

        if ((basinID = basinFLD->Int(pointRec)) == 0) continue;
        cellRec = netIF->Cell(pntIF->Coordinate(pointRec));
        mouthID = cellRec->RowID();
        color = 1;
        Start:
        for (cellID = mouthID; cellID < cellTable->ItemNum(); ++cellID) {
            cellRec = cellTable->Item(cellID);
            if (netIF->CellBasinID(cellRec) != basinID) break;
            if (_RGlibTEMPPointIDFLD->Int(cellRec) != pointID) continue;

            for (dir = 0; dir < 8; ++dir) {
                if ((fromCell = netIF->FromCell(cellRec, 0x01 << dir, false)) == (DBObjRecord *) NULL) continue;
                if ((dPointID = _RGlibTEMPPointIDFLD->Int(fromCell)) == pointID) continue;
                if (dPointID == DBFault) continue;

                dPointRec = pointTable->Item(dPointID);
                if (colorFLD->Int(dPointRec) == color) {
                    color++;
                    goto Start;
                }
            }
        }
        colorFLD->Int(pointRec, color);
    }
    ret = DBSuccess;
    Stop:
    pointTable->ListSort();
    cellTable->DeleteField(_RGlibTEMPPointIDFLD);
    delete pntIF;
    delete netIF;
    return (ret);
}

static DBInt _RGlibSubbasinCenterAction(DBNetworkIF *netIF, DBObjRecord *cellRec, DBCoordinate *massCoord) {
    DBCoordinate coord = netIF->Center(cellRec);
    massCoord->X += coord.X;
    massCoord->Y += coord.Y;
    return (true);
}

DBInt RGlibPointSubbasinCenter(DBObjData *pntData, DBObjData *netData) {
    DBCoordinate massCoord;
    DBVPointIF *pntIF = new DBVPointIF(pntData);
    DBObjTable *pointTable = pntData->Table(DBrNItems);
    DBObjTableField *massCoordXFLD = pointTable->Field(RGlibMassCoordX);
    DBObjTableField *massCoordYFLD = pointTable->Field(RGlibMassCoordY);
    DBNetworkIF *netIF = new DBNetworkIF(netData);
    DBObjRecord *pointRec, *cellRec;

    if (massCoordXFLD == NULL) {
        massCoordXFLD = new DBObjTableField(RGlibMassCoordX, DBTableFieldFloat, "%10.3f", sizeof(DBFloat4));
        pointTable->AddField(massCoordXFLD);
    }
    if (massCoordYFLD == NULL) {
        massCoordYFLD = new DBObjTableField(RGlibMassCoordY, DBTableFieldFloat, "%10.3f", sizeof(DBFloat4));
        pointTable->AddField(massCoordYFLD);
    }

    for (pointRec = pntIF->FirstItem(); pointRec != (DBObjRecord *) NULL; pointRec = pntIF->NextItem()) {
        if ((pointRec->Flags() & DBObjectFlagIdle) == DBObjectFlagIdle) {
            massCoordXFLD->Float(pointRec, massCoordXFLD->FloatNoData());
            massCoordYFLD->Float(pointRec, massCoordYFLD->FloatNoData());
            continue;
        }
        if (DBPause(pointRec->RowID() * 100 / pntIF->ItemNum())) goto Stop;
        if ((cellRec = netIF->Cell(pntIF->Coordinate(pointRec))) == (DBObjRecord *) NULL)
            massCoord = pntIF->Coordinate(pointRec);
        else {
            if (netIF->CellBasinCells(cellRec) > 1) {
                massCoord.X = 0.0;
                massCoord.Y = 0.0;
                netIF->UpStreamSearch(cellRec, (DBNetworkACTION) _RGlibSubbasinCenterAction, &massCoord);
                massCoord.X = massCoord.X / (DBFloat) netIF->CellBasinCells(cellRec);
                massCoord.Y = massCoord.Y / (DBFloat) netIF->CellBasinCells(cellRec);
            }
            else massCoord = netIF->Center(cellRec);
        }
        massCoordXFLD->Float(pointRec, massCoord.X);
        massCoordYFLD->Float(pointRec, massCoord.Y);
    }
    Stop:
    if (pointRec != (DBObjRecord *) NULL) {
        pointTable->DeleteField(massCoordXFLD);
        pointTable->DeleteField(massCoordYFLD);
        return (DBFault);
    }
    return (DBSuccess);
}

DBInt RGlibPointInterStationTS(DBObjData *pntData, DBObjData *tsData, char *relateFldName, char *joinFldName) {
    DBInt first = true, tsIndex, tsRowNum = 0;
    DBObjTable *pntTBL = pntData->Table(DBrNItems), *tsTBL;
    DBObjectLIST<DBObjTableField> *fields;
    DBObjTableField *pntNextFLD = pntTBL->Field(RGlibNextStation);
    DBObjTableField *pntAreaFLD = pntTBL->Field(RGlibArea);
    DBObjTableField *pntInterStnFLD = pntTBL->Field(RGlibInterStation);
    DBObjTableField *pntRelateFLD;
    DBObjTableField *pntNewNextFLD;
    DBObjTableField *pntNewInterStnFLD;
    DBObjTableField *tsTimeFLD;
    DBObjTableField *tsJoinFLD;
    DBObjTableField *tsNextStnFLD, *tsInterStnFLD;
    DBObjRecord *pntRec, *nextPntRec, *tsRec, *tsIndexRec;
    DBDate curDate, date;

    if (pntNextFLD == (DBObjTableField *) NULL) {
        CMmsgPrint(CMmsgUsrError, "Missing Next Station Field!");
        return (DBFault);
    }
    if (pntAreaFLD == (DBObjTableField *) NULL) {
        CMmsgPrint(CMmsgUsrError, "Missing STN Area Field!");
        return (DBFault);
    }
    if (pntInterStnFLD == (DBObjTableField *) NULL) {
        CMmsgPrint(CMmsgUsrError, "Missing Interfluvial Area Field!");
        return (DBFault);
    }

    tsTBL = tsData->Table(DBrNItems);
    tsNextStnFLD = new DBObjTableField(RGlibNextStation, DBTableFieldInt, "%8d", sizeof(DBInt));
    tsTBL->AddField(tsNextStnFLD);
    tsInterStnFLD = new DBObjTableField(RGlibInterStation, DBTableFieldFloat, "%9.1f", sizeof(DBFloat4));
    tsTBL->AddField(tsInterStnFLD);
    fields = tsTBL->Fields();
    for (tsTimeFLD = fields->First(); tsTimeFLD != (DBObjTableField *) NULL; tsTimeFLD = fields->Next())
        if (tsTimeFLD->Type() == DBTableFieldDate) break;
    if (tsTimeFLD == (DBObjTableField *) NULL) {
        CMmsgPrint(CMmsgUsrError, "Missing Date Field!");
        return (DBFault);
    }
    if ((tsJoinFLD = tsTBL->Field(joinFldName)) == (DBObjTableField *) NULL) {
        CMmsgPrint(CMmsgUsrError, "Missing Join Field!");
        return (DBFault);
    }
    fields = new DBObjectLIST<DBObjTableField>("Field List");
    fields->Add(new DBObjTableField(*tsTimeFLD));
    fields->Add(new DBObjTableField(*tsJoinFLD));
    tsTBL->ListSort(fields);
    delete fields;

    pntTBL = new DBObjTable(*pntTBL);
    pntNextFLD = pntTBL->Field(RGlibNextStation);
    pntAreaFLD = pntTBL->Field(RGlibArea);
    pntInterStnFLD = pntTBL->Field(RGlibInterStation);
    pntNewNextFLD = new DBObjTableField("NextStnTS", pntNextFLD->Type(), pntNextFLD->Format(), pntNextFLD->Length());
    pntNewInterStnFLD = new DBObjTableField("InterFluTS", pntInterStnFLD->Type(), pntInterStnFLD->Format(),pntInterStnFLD->Length());
    pntRelateFLD = pntTBL->Field(relateFldName);
    pntTBL->AddField(pntNewNextFLD);
    pntTBL->AddField(pntNewInterStnFLD);
    pntTBL->ListSort(pntRelateFLD);

    tsIndexRec = tsTBL->First(&tsIndex);
    for (tsRec = tsTBL->First(); tsRec != (DBObjRecord *) NULL; tsRec = tsTBL->Next()) {
        DBPause(tsRowNum++ * 100 / tsTBL->ItemNum());
        date = tsTimeFLD->Date(tsRec);
        if (date != curDate) {
            if (first) first = false;
            else {
                for (pntRec = pntTBL->First(); pntRec != (DBObjRecord *) NULL; pntRec = pntTBL->Next()) {
                    if ((pntRec->Flags() & DBObjectFlagLocked) != DBObjectFlagLocked) continue;
                    for (nextPntRec = pntTBL->Item(pntNextFLD->Int(pntRec) - 1);
                         (nextPntRec != (DBObjRecord *) NULL) &&
                         ((nextPntRec->Flags() & DBObjectFlagLocked) != DBObjectFlagLocked);
                         nextPntRec = pntTBL->Item(pntNextFLD->Int(nextPntRec) - 1));
                    if (nextPntRec != (DBObjRecord *) NULL) {
                        pntNewNextFLD->Int(pntRec, nextPntRec->RowID() + 1);
                        pntNewInterStnFLD->Float(nextPntRec,
                                                 pntNewInterStnFLD->Float(nextPntRec) - pntAreaFLD->Float(pntRec));
                    }
                }
                pntRec = pntTBL->First();
                for (; tsIndexRec != (DBObjRecord *) NULL; tsIndexRec = tsTBL->Next(&tsIndex)) {
                    if (tsRec == tsIndexRec) break;
                    for (; pntRec != (DBObjRecord *) NULL; pntRec = pntTBL->Next())
                        if (pntRelateFLD->Int(pntRec) == tsJoinFLD->Int(tsIndexRec)) {
                            tsNextStnFLD->Int(tsIndexRec, pntNewNextFLD->Int(pntRec));
                            tsInterStnFLD->Float(tsIndexRec, pntNewInterStnFLD->Float(pntRec));
                            break;
                        }
                    if (pntRec == (DBObjRecord *) NULL) pntRec = pntTBL->First();
                }
            }
            for (pntRec = pntTBL->First(); pntRec != (DBObjRecord *) NULL; pntRec = pntTBL->Next()) {
                pntNewNextFLD->Int(pntRec, 0);
                pntNewInterStnFLD->Float(pntRec, pntAreaFLD->Float(pntRec));
                pntRec->Flags(DBObjectFlagLocked, DBClear);
            }
            curDate = date;
            pntRec = pntTBL->First();
        }
        for (; pntRec != (DBObjRecord *) NULL; pntRec = pntTBL->Next())
            if (pntRelateFLD->Int(pntRec) == tsJoinFLD->Int(tsRec)) {
                pntRec->Flags(DBObjectFlagLocked, DBSet);
                break;
            }
        if (pntRec == (DBObjRecord *) NULL) pntRec = pntTBL->First();
    }
    if (!first) {
        for (pntRec = pntTBL->First(); pntRec != (DBObjRecord *) NULL; pntRec = pntTBL->Next()) {
            if ((pntRec->Flags() & DBObjectFlagLocked) != DBObjectFlagLocked) continue;
            for (nextPntRec = pntTBL->Item(pntNextFLD->Int(pntRec) - 1);
                 (nextPntRec != (DBObjRecord *) NULL) &&
                 ((nextPntRec->Flags() & DBObjectFlagLocked) != DBObjectFlagLocked);
                 nextPntRec = pntTBL->Item(pntNextFLD->Int(nextPntRec) - 1));
            if (nextPntRec != (DBObjRecord *) NULL) {
                pntNewNextFLD->Int(pntRec, nextPntRec->RowID() + 1);
                pntNewInterStnFLD->Float(nextPntRec, pntNewInterStnFLD->Float(nextPntRec) - pntAreaFLD->Float(pntRec));
            }
        }
        pntRec = pntTBL->First();
        for (; tsIndexRec != (DBObjRecord *) NULL; tsIndexRec = tsTBL->Next(&tsIndex)) {
            if (tsRec == tsIndexRec) break;
            for (; pntRec != (DBObjRecord *) NULL; pntRec = pntTBL->Next())
                if (pntRelateFLD->Int(pntRec) == tsJoinFLD->Int(tsIndexRec)) {
                    tsNextStnFLD->Int(tsIndexRec, pntNewNextFLD->Int(pntRec));
                    tsInterStnFLD->Float(tsIndexRec, pntNewInterStnFLD->Float(pntRec));
                    break;
                }
            if (pntRec == (DBObjRecord *) NULL) pntRec = pntTBL->First();
        }
    }
    delete pntTBL;
    return (DBSuccess);
}

#define RGlibPointMin        "SubbasinMin"
#define RGlibPointMax        "SubbasinMax"
#define RGlibPointMean        "SubbasinMean"
#define RGlibPointStdDev    "SubbasinStdDev"
#define RGlibPointArea        "SubbasinArea"

static DBFloat _RGlibSubbasinArea;
static DBFloat _RGlibSubbasinMean;
static DBFloat _RGlibSubbasinMin;
static DBFloat _RGlibSubbasinMax;
static DBFloat _RGlibSubbasinStdDev;
static DBGridIF *_RGlibPointGrdIF;
static DBObjRecord *_RGlibPointGrdLayerRec;

static DBInt _RGlibSubbasinStatistics(void *io, DBObjRecord *cellRec) {
    DBFloat value;
    DBNetworkIF *netIF = (DBNetworkIF *) io;
    if (cellRec == (DBObjRecord *) NULL) return (false);
    if (_RGlibPointGrdIF->Value(_RGlibPointGrdLayerRec, netIF->Center(cellRec), &value) == false) return (true);
    _RGlibSubbasinArea = _RGlibSubbasinArea + netIF->CellArea(cellRec);
    _RGlibSubbasinMean = _RGlibSubbasinMean + value * netIF->CellArea(cellRec);
    _RGlibSubbasinMin = _RGlibSubbasinMin < value ? _RGlibSubbasinMin : value;
    _RGlibSubbasinMax = _RGlibSubbasinMax > value ? _RGlibSubbasinMax : value;
    _RGlibSubbasinStdDev = _RGlibSubbasinStdDev + value * value * netIF->CellArea(cellRec);
    return (true);
}

DBInt RGlibPointSubbasinStats(DBObjData *pntData, DBObjData *netData, DBObjData *grdData, DBObjData *tblData) {
    DBInt layerID, layerNum = 0, progress = 0, maxProgress;
    DBObjTable *table;
    DBObjTableField *pointIDFLD;
    DBObjTableField *layerIDFLD;
    DBObjTableField *layerNameFLD;
    DBObjTableField *minimumFLD;
    DBObjTableField *maximumFLD;
    DBObjTableField *averageFLD;
    DBObjTableField *stdDevFLD;
    DBObjTableField *areaFLD;
    DBVPointIF *pntIF;
    DBNetworkIF *netIF;
    DBObjRecord *pntRec, *tblRec;
    DBObjectLIST<DBObjTableField> *fields;

    _RGlibPointGrdIF = new DBGridIF(grdData);
    for (layerID = 0; layerID < _RGlibPointGrdIF->LayerNum(); ++layerID) {
        _RGlibPointGrdLayerRec = _RGlibPointGrdIF->Layer(layerID);
        if ((_RGlibPointGrdLayerRec->Flags() & DBObjectFlagIdle) != DBObjectFlagIdle) ++layerNum;
    }
    if (layerNum < 1) {
        CMmsgPrint(CMmsgUsrError, "No Layer to Process!");
        delete _RGlibPointGrdIF;
        return (DBFault);
    }

    table = tblData->Table(DBrNItems);
    pntIF = new DBVPointIF(pntData);
    netIF = new DBNetworkIF(netData);

    table->AddField(pointIDFLD = new DBObjTableField("GHAASPointID", DBTableFieldInt, "%8d", sizeof(DBInt)));
    table->AddField(layerIDFLD = new DBObjTableField("LayerID", DBTableFieldInt, "%4d", sizeof(DBShort)));
    table->AddField(layerNameFLD = new DBObjTableField("LayerName", DBTableFieldString, "%s", DBStringLength));
    table->AddField(averageFLD = new DBObjTableField(RGlibPointMean, DBTableFieldFloat, _RGlibPointGrdIF->ValueFormat(),
                                                     sizeof(DBFloat4)));
    table->AddField(minimumFLD = new DBObjTableField(RGlibPointMin, DBTableFieldFloat, _RGlibPointGrdIF->ValueFormat(),
                                                     sizeof(DBFloat4)));
    table->AddField(maximumFLD = new DBObjTableField(RGlibPointMax, DBTableFieldFloat, _RGlibPointGrdIF->ValueFormat(),
                                                     sizeof(DBFloat4)));
    table->AddField(
            stdDevFLD = new DBObjTableField(RGlibPointStdDev, DBTableFieldFloat, _RGlibPointGrdIF->ValueFormat(),
                                            sizeof(DBFloat4)));
    table->AddField(areaFLD = new DBObjTableField(RGlibPointArea, DBTableFieldFloat, _RGlibPointGrdIF->ValueFormat(),
                                                  sizeof(DBFloat4)));

    grdData->Flags(DBObjectFlagProcessed, DBSet);
    maxProgress = pntIF->ItemNum() * _RGlibPointGrdIF->LayerNum();
    for (layerID = 0; layerID < _RGlibPointGrdIF->LayerNum(); ++layerID) {
        _RGlibPointGrdLayerRec = _RGlibPointGrdIF->Layer(layerID);
        if ((_RGlibPointGrdLayerRec->Flags() & DBObjectFlagIdle) == DBObjectFlagIdle) continue;
        for (pntRec = pntIF->FirstItem(); pntRec != (DBObjRecord *) NULL; pntRec = pntIF->NextItem()) {
            if (DBPause(progress * 100 / maxProgress)) goto Stop;
            progress++;
            if ((pntRec->Flags() & DBObjectFlagIdle) == DBObjectFlagIdle) continue;
            tblRec = table->Add(pntRec->Name());
            pointIDFLD->Int(tblRec, pntRec->RowID() + 1);
            layerIDFLD->Int(tblRec, _RGlibPointGrdLayerRec->RowID());
            layerNameFLD->String(tblRec, _RGlibPointGrdLayerRec->Name());
            _RGlibSubbasinArea = 0.0;
            _RGlibSubbasinMin = DBHugeVal;
            _RGlibSubbasinMax = -DBHugeVal;
            _RGlibSubbasinMean = 0.0;
            _RGlibSubbasinStdDev = 0.0;
            netIF->UpStreamSearch(netIF->Cell(pntIF->Coordinate(pntRec)), (DBNetworkACTION) _RGlibSubbasinStatistics);
            _RGlibSubbasinMean = _RGlibSubbasinMean / _RGlibSubbasinArea;
            _RGlibSubbasinStdDev = _RGlibSubbasinStdDev / _RGlibSubbasinArea;
            _RGlibSubbasinStdDev = _RGlibSubbasinStdDev - _RGlibSubbasinMean * _RGlibSubbasinMean;
            _RGlibSubbasinStdDev = sqrt(_RGlibSubbasinStdDev);
            minimumFLD->Float(tblRec, _RGlibSubbasinMin);
            maximumFLD->Float(tblRec, _RGlibSubbasinMax);
            averageFLD->Float(tblRec, _RGlibSubbasinMean);
            stdDevFLD->Float(tblRec, _RGlibSubbasinStdDev);
            areaFLD->Float(tblRec, _RGlibSubbasinArea);
        }
    }
    Stop:
    delete _RGlibPointGrdIF;
    delete netIF;
    delete pntIF;

    if (progress == maxProgress) {
        fields = new DBObjectLIST<DBObjTableField>("Field List");
        fields->Add(new DBObjTableField(*pointIDFLD));
        fields->Add(new DBObjTableField(*layerIDFLD));
        table->ListSort(fields);
        delete fields;
        return (DBSuccess);
    }
    return (DBFault);
}

class Histogram {
public:
    DBInt cellNum;
    DBFloat area;

    void Initialize() {
        cellNum = 0;
        area = 0.0;
    }
};

static Histogram *_RGlibHistogram;

static DBInt _RGlibSubbasinCategories(void *io, DBObjRecord *cellRec) {
    DBObjRecord *grdRec;
    DBNetworkIF *netIF = (DBNetworkIF *) io;
    if (cellRec == (DBObjRecord *) NULL) return (false);
    grdRec = _RGlibPointGrdIF->GridItem(_RGlibPointGrdLayerRec, netIF->Center(cellRec));
    if (grdRec == (DBObjRecord *) NULL) return (true);
    _RGlibHistogram[grdRec->RowID()].cellNum++;
    _RGlibHistogram[grdRec->RowID()].area += netIF->CellArea(cellRec);
    return (true);
}

DBInt RGlibPointSubbasinHist(DBObjData *pntData, DBObjData *netData, DBObjData *grdData, DBObjData *tblData) {
    DBInt layerID, layerNum = 0, progress = 0, maxProgress;
    DBObjTable *itemTable = grdData->Table(DBrNItems);
    DBObjTable *table = tblData->Table(DBrNItems);
    DBObjTableField *pointIDFLD;
    DBObjTableField *layerIDFLD;
    DBObjTableField *layerNameFLD;
    DBObjTableField *categoryIDFLD;
    DBObjTableField *categoryFLD;
    DBObjTableField *percentFLD;
    DBObjTableField *areaFLD;
    DBObjTableField *cellNumFLD;
    DBVPointIF *pntIF;
    DBNetworkIF *netIF;
    DBObjRecord *pntRec, *itemRec, *tblRec;
    DBObjectLIST<DBObjTableField> *fields;

    _RGlibPointGrdIF = new DBGridIF(grdData);
    for (layerID = 0; layerID < _RGlibPointGrdIF->LayerNum(); ++layerID) {
        _RGlibPointGrdLayerRec = _RGlibPointGrdIF->Layer(layerID);
        if ((_RGlibPointGrdLayerRec->Flags() & DBObjectFlagIdle) != DBObjectFlagIdle) ++layerNum;
    }
    if (layerNum < 1) {
        CMmsgPrint(CMmsgUsrError, "No Layer to Process!");
        delete _RGlibPointGrdIF;
        return (DBFault);
    }
    pntIF = new DBVPointIF(pntData);
    netIF = new DBNetworkIF(netData);

    table->AddField(pointIDFLD = new DBObjTableField("GHAASPointID", DBTableFieldInt, "%8d", sizeof(DBInt)));
    table->AddField(layerIDFLD = new DBObjTableField("LayerID", DBTableFieldInt, "%4d", sizeof(DBShort)));
    table->AddField(layerNameFLD = new DBObjTableField("LayerName", DBTableFieldString, "%s", DBStringLength));
    table->AddField(categoryIDFLD = new DBObjTableField(DBrNCategoryID, DBTableFieldInt, "%2d", sizeof(DBShort)));
    table->AddField(categoryFLD = new DBObjTableField(DBrNCategory, DBTableFieldString, _RGlibPointGrdIF->ValueFormat(),
                                                      DBStringLength));
    table->AddField(cellNumFLD = new DBObjTableField("CellNum", DBTableFieldInt, "%8d", sizeof(DBInt)));
    table->AddField(areaFLD = new DBObjTableField(DBrNArea, DBTableFieldFloat, "%10.1f", sizeof(DBFloat4)));
    table->AddField(percentFLD = new DBObjTableField(DBrNPercent, DBTableFieldFloat, "%6.2f", sizeof(DBFloat4)));

    _RGlibHistogram = (Histogram *) malloc(itemTable->ItemNum() * sizeof(Histogram));
    if (_RGlibHistogram == (Histogram *) NULL) {
        CMmsgPrint(CMmsgAppError, "Memory Allocation Error in: %s %d", __FILE__, __LINE__);
        return (DBFault);
    }
    maxProgress = pntIF->ItemNum() * _RGlibPointGrdIF->LayerNum();
    for (layerID = 0; layerID < _RGlibPointGrdIF->LayerNum(); ++layerID) {
        _RGlibPointGrdLayerRec = _RGlibPointGrdIF->Layer(layerID);
        if ((_RGlibPointGrdLayerRec->Flags() & DBObjectFlagIdle) == DBObjectFlagIdle) continue;
        for (pntRec = pntIF->FirstItem(); pntRec != (DBObjRecord *) NULL; pntRec = pntIF->NextItem()) {
            if (DBPause(progress * 100 / maxProgress)) goto Stop;
            progress++;
            if ((pntRec->Flags() & DBObjectFlagIdle) == DBObjectFlagIdle) continue;
            for (itemRec = itemTable->First(); itemRec != (DBObjRecord *) NULL; itemRec = itemTable->Next())
                _RGlibHistogram[itemRec->RowID()].Initialize();
            netIF->UpStreamSearch(netIF->Cell(pntIF->Coordinate(pntRec)), (DBNetworkACTION) _RGlibSubbasinCategories);
            for (itemRec = itemTable->First(); itemRec != (DBObjRecord *) NULL; itemRec = itemTable->Next())
                if (_RGlibHistogram[itemRec->RowID()].cellNum > 0) {
                    tblRec = table->Add(pntRec->Name());
                    pointIDFLD->Int(tblRec, pntRec->RowID() + 1);
                    layerIDFLD->Int(tblRec, _RGlibPointGrdLayerRec->RowID());
                    layerNameFLD->String(tblRec, _RGlibPointGrdLayerRec->Name());
                    categoryIDFLD->Int(tblRec, itemRec->RowID() + 1);
                    categoryFLD->String(tblRec, itemRec->Name());
                    areaFLD->Float(tblRec, _RGlibHistogram[itemRec->RowID()].area);
                    percentFLD->Float(tblRec, _RGlibHistogram[itemRec->RowID()].area /
                                              netIF->CellBasinArea(netIF->Cell(pntIF->Coordinate(pntRec))) * 100.0);
                    cellNumFLD->Int(tblRec, _RGlibHistogram[itemRec->RowID()].cellNum);
                }
        }
    }
    Stop:
    delete _RGlibPointGrdIF;
    delete netIF;
    delete pntIF;
    free(_RGlibHistogram);

    if (progress == maxProgress) {
        fields = new DBObjectLIST<DBObjTableField>("Field List");
        fields->Add(new DBObjTableField(*pointIDFLD));
        fields->Add(new DBObjTableField(*layerIDFLD));
        fields->Add(areaFLD = new DBObjTableField(*areaFLD));
        areaFLD->Flags(DBObjectFlagSortReversed, DBSet);
        table->ListSort(fields);
        delete fields;
        return (DBSuccess);
    }
    return (DBFault);
}
