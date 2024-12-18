/******************************************************************************

GHAAS RiverGIS Libarary V3.0
Global Hydrological Archive and Analysis System
Copyright 1994-2024, UNH - CCNY

RGlibNetwork.cpp

bfekete@ccny.cuny.edu

*******************************************************************************/

#include <DB.hpp>
#include <DBif.hpp>
#include <RG.hpp>

DBInt RGlibNetworkToGrid(DBObjData *netData, DBObjTableField *field, DBObjData *grdData) {
    DBInt cellID, intVal, ret = DBSuccess;
    DBFloat floatVal;
    DBPosition pos;
    DBObjRecord *cellRec, *layerRec;
    DBGridIF *gridIF;
    DBNetworkIF *netIF;

    if (field == (DBObjTableField *) NULL) return (DBFault);

    netIF = new DBNetworkIF(netData);
    gridIF = new DBGridIF(grdData);
    layerRec = gridIF->Layer(0);
    gridIF->RenameLayer(layerRec, field->Name());
    switch (grdData->Type()) {
        case DBTypeGridContinuous:
            for (pos.Row = 0; pos.Row < gridIF->RowNum(); ++pos.Row)
                for (pos.Col = 0; pos.Col < gridIF->ColNum(); ++pos.Col)
                    gridIF->Value(layerRec, pos, gridIF->MissingValue());

            for (cellID = 0; cellID < netIF->CellNum(); ++cellID) {
                if (DBPause(cellID * 100 / netIF->CellNum())) goto Stop;
                cellRec = netIF->Cell(cellID);
                if ((cellRec->Flags() & DBObjectFlagIdle) == DBObjectFlagIdle) continue;

                if (field->Type() == DBTableFieldInt) {
                    intVal = field->Int(cellRec);
                    if (intVal == field->IntNoData())
                        gridIF->Value(layerRec, netIF->CellPosition(cellRec), gridIF->MissingValue());
                    else gridIF->Value(layerRec, netIF->CellPosition(cellRec), (DBFloat) intVal);
                }
                else {
                    floatVal = field->Float(cellRec);
                    if (CMmathEqualValues(floatVal, field->FloatNoData()))
                        gridIF->Value(layerRec, netIF->CellPosition(cellRec), gridIF->MissingValue());
                    else gridIF->Value(layerRec, netIF->CellPosition(cellRec), floatVal);
                }
            }
            gridIF->RecalcStats();
            break;
        case DBTypeGridDiscrete: {
            char nameStr[DBStringLength];
            static char *strPtr;
            DBObjRecord *itemRec;
            DBObjRecord *symRec = (grdData->Table(DBrNSymbols))->Item();
            DBObjTable *itemTable = grdData->Table(DBrNItems);
            DBObjTableField *gridValueFLD = itemTable->Field(DBrNGridValue);
            DBObjTableField *gridSymbolFLD = itemTable->Field(DBrNSymbol);
            DBObjTableField *symbolIDFLD = (grdData->Table(DBrNSymbols))->Field(DBrNSymbolID);
            DBObjTableField *foregroundFLD = (grdData->Table(DBrNSymbols))->Field(DBrNForeground);
            DBObjTableField *backgroundFLD = (grdData->Table(DBrNSymbols))->Field(DBrNBackground);
            DBObjTableField *styleFLD = (grdData->Table(DBrNSymbols))->Field(DBrNStyle);

            symbolIDFLD->Int(symRec, 0);
            foregroundFLD->Int(symRec, 1);
            backgroundFLD->Int(symRec, 0);
            styleFLD->Int(symRec, 0);

            for (pos.Row = 0; pos.Row < gridIF->RowNum(); ++pos.Row)
                for (pos.Col = 0; pos.Col < gridIF->ColNum(); ++pos.Col)
                    gridIF->Value(layerRec, pos, DBFault);

            for (cellID = 0; cellID < netIF->CellNum(); ++cellID) {
                if (DBPause(cellID * 100 / netIF->CellNum())) goto Stop;
                cellRec = netIF->Cell(cellID);
                if ((cellRec->Flags() & DBObjectFlagIdle) == DBObjectFlagIdle) continue;
                if (DBTableFieldIsNumeric(field)) {
                    if ((intVal = field->Int(cellRec)) == field->IntNoData())
                        gridIF->Value(netIF->CellPosition(cellRec), DBFault);
                    else {
                        snprintf(nameStr, sizeof(nameStr), "Category%04d", intVal);
                        if ((itemRec = itemTable->Item(nameStr)) == (DBObjRecord *) NULL) {
                            if ((itemRec = itemTable->Add(nameStr)) == (DBObjRecord *) NULL) {
                                CMmsgPrint(CMmsgAppError, "Item Object Creation Error in: %s %d", __FILE__, __LINE__);
                                return (DBFault);
                            }
                            gridValueFLD->Int(itemRec, intVal);
                            gridSymbolFLD->Record(itemRec, symRec);
                        }
                        gridIF->Value(netIF->CellPosition(cellRec), itemRec->RowID());
                    }
                }
                else {
                    if ((strPtr = field->String(cellRec)) == (char *) NULL)
                        gridIF->Value(netIF->CellPosition(cellRec), DBFault);
                    else {
                        if ((itemRec = itemTable->Item(strPtr)) == (DBObjRecord *) NULL) {
                            if ((itemRec = itemTable->Add(strPtr)) == (DBObjRecord *) NULL) {
                                CMmsgPrint(CMmsgAppError, "Item Object Creation Error in: %s %d", __FILE__, __LINE__);
                                return (DBFault);
                            }
                            gridValueFLD->Int(itemRec, itemRec->RowID());
                            gridSymbolFLD->Record(itemRec, symRec);
                        }
                        gridIF->Value(netIF->CellPosition(cellRec), itemRec->RowID());
                    }
                }
            }
            itemTable->ListSort(gridValueFLD);
            itemTable->ItemSort();
            gridIF->DiscreteStats();
        }
            break;
        default:
            CMmsgPrint(CMmsgSysError, "Invalid data type in: %s %d", __FILE__, __LINE__);
            cellID = 0;
            break;
    }
    Stop:
    if (cellID != netIF->CellNum()) ret = DBFault;
    delete netIF;
    delete gridIF;
    return (ret);
}

DBInt RGlibNetworkBasinGrid(DBObjData *netData, DBObjData *grdData) {
    char symbolName[DBStringLength];
    DBInt cellID, basinID, inFieldID, symbol, symMax = 0, ret = DBSuccess;
    DBPosition pos;
    DBObjRecord *cellRec, *basinRec, *itemRec, *symRec;
    DBObjTable *grdTable = grdData->Table(DBrNItems);
    DBObjTable *symTable = grdData->Table(DBrNSymbols);
    DBObjTable *basinTable = netData->Table(DBrNItems);
    DBObjTable *cellTable = netData->Table(DBrNCells);
    DBObjTableField *gValueFLD = grdTable->Field(DBrNGridValue);
    DBObjTableField *gSymFLD = grdTable->Field(DBrNSymbol);
    DBObjTableField *gFGSymFLD = symTable->Field(DBrNForeground);
    DBObjTableField *gBGSymFLD = symTable->Field(DBrNBackground);
    DBObjTableField *gStSymFLD = symTable->Field(DBrNStyle);
    DBObjTableField *colorFLD = basinTable->Field(DBrNColor);
    DBObjTableField *basinFLD = cellTable->Field(DBrNBasin);
    DBObjTableField *inFLD, *outFLD;
    DBGridIF *gridIF;
    DBNetworkIF *netIF;

    netIF = new DBNetworkIF(netData);
    gridIF = new DBGridIF(grdData);

    gridIF->RenameLayer(gridIF->Layer(0), (char *) "Basin Grid");

    for (inFieldID = 0; inFieldID < basinTable->FieldNum(); ++inFieldID) {
        inFLD = basinTable->Field(inFieldID);
        if (DBTableFieldIsVisible(inFLD)) {
            grdTable->AddField(outFLD = new DBObjTableField(*inFLD));
            if (inFLD->Required()) outFLD->Required(false);
        }
    }
    for (basinID = 0; basinID < netIF->BasinNum(); ++basinID) {
        basinRec = basinTable->Item(basinID);
        itemRec = grdTable->Add(basinRec->Name());
        gValueFLD->Int(itemRec, basinID + 1);
        symbol = colorFLD->Int(basinRec) - 7;
        symMax = symMax > symbol ? symMax : symbol;
        for (inFieldID = 0; inFieldID < basinTable->FieldNum(); ++inFieldID) {
            inFLD = basinTable->Field(inFieldID);
            if (DBTableFieldIsVisible(inFLD)) {
                outFLD = grdTable->Field(inFLD->Name());
                switch (inFLD->Type()) {
                    case DBTableFieldString:
                        outFLD->String(itemRec, inFLD->String(basinRec));
                        break;
                    case DBTableFieldInt:
                        outFLD->Int(itemRec, inFLD->Int(basinRec));
                        break;
                    case DBTableFieldFloat:
                        outFLD->Float(itemRec, inFLD->Float(basinRec));
                        break;
                    case DBTableFieldDate:
                        outFLD->Date(itemRec, inFLD->Date(basinRec));
                        break;
                    default:
                        CMmsgPrint(CMmsgAppError, "Invalid field type in: %s %d", __FILE__, __LINE__);
                        break;
                }
            }
        }
    }
    symTable->DeleteAll();
    for (symbol = 0; symbol < symMax; ++symbol) {
        snprintf(symbolName, sizeof(symbolName), "Symbol %2d", symbol + 1);
        symRec = symTable->Add(symbolName);
        gFGSymFLD->Int(symRec, 7 + symbol);
        gBGSymFLD->Int(symRec, symbol);
        gStSymFLD->Int(symRec, 0);
    }
    for (basinID = 0; basinID < netIF->BasinNum(); ++basinID) {
        basinRec = basinTable->Item(basinID);
        itemRec = grdTable->Item(basinID);
        symbol = colorFLD->Int(basinRec) - 7;
        gSymFLD->Record(itemRec, symTable->Item(symbol));
    }

    for (pos.Row = 0; pos.Row < netIF->RowNum(); pos.Row++)
        for (pos.Col = 0; pos.Col < netIF->ColNum(); pos.Col++) gridIF->Value(pos, DBFault);
    for (cellID = 0; cellID < netIF->CellNum(); ++cellID) {
        if (DBPause(cellID * 100 / netIF->CellNum())) goto Stop;
        cellRec = netIF->Cell(cellID);
        if ((cellRec->Flags() & DBObjectFlagIdle) == DBObjectFlagIdle) continue;
        gridIF->Value(netIF->CellPosition(cellRec), basinFLD->Int(cellRec) - 1);
    }
    Stop:
    if (cellID == netIF->CellNum()) gridIF->DiscreteStats();
    else ret = DBFault;

    delete netIF;
    delete gridIF;
    return (ret);
}

DBInt RGlibNetworkStations(DBObjData *netData, DBFloat area, DBFloat tolerance, DBObjData *pntData) {
    char name[DBStringLength];
    DBInt cellID;
    DBFloat *areaARR;
    DBObjRecord *cellRec, *toCell;
    DBNetworkIF *netIF = new DBNetworkIF(netData);
    DBCoordinate coord;
    DBObjTable *items = pntData->Table(DBrNItems);
    DBObjTable *symbols = pntData->Table(DBrNSymbols);
    DBObjTableField *coordField = items->Field(DBrNCoord);
    DBObjTableField *symbolFLD = items->Field(DBrNSymbol);
    DBObjTableField *foregroundFLD = symbols->Field(DBrNForeground);
    DBObjTableField *backgroundFLD = symbols->Field(DBrNBackground);
    DBObjTableField *styleFLD = symbols->Field(DBrNStyle);
    DBObjRecord *pntRec, *symRec;
    DBRegion dataExtent;

    symRec = symbols->Add("Default Symbol");
    foregroundFLD->Int(symRec, 1);
    backgroundFLD->Int(symRec, 2);
    styleFLD->Int(symRec, 0);

    if ((areaARR = (DBFloat *) calloc(netIF->CellNum(), sizeof(DBFloat))) == (DBFloat *) NULL) {
        CMmsgPrint(CMmsgSysError, "Memory Allocation Error in: %s %d", __FILE__, __LINE__);
        delete netIF;
        return (DBFault);
    }
    for (cellID = 0; cellID < netIF->CellNum(); ++cellID)
        areaARR[cellID] = netIF->CellBasinArea(netIF->Cell(cellID));

    for (cellID = netIF->CellNum() - 1; cellID >= 0; --cellID) {
        if (DBPause((netIF->CellNum() - cellID) * 100 / netIF->CellNum())) goto Stop;

        if (areaARR[cellID] > area) toCell = netIF->ToCell(cellRec = netIF->Cell(cellID));
        else if (areaARR[cellID] > area * (1.0 - tolerance / 100.0)) {
            if ((toCell = netIF->ToCell(netIF->Cell(cellID))) != (DBObjRecord *) NULL) {
                if (areaARR[toCell->RowID()] < area * (1.0 + tolerance / 100.0))
                    cellRec = areaARR[toCell->RowID()] / area > area / areaARR[cellID] ?
                              netIF->Cell(cellID) : (DBObjRecord *) NULL;
            }
            else cellRec = netIF->Cell(cellID);
        }
        else cellRec = (DBObjRecord *) NULL;

        if (cellRec != (DBObjRecord *) NULL) {
            if (toCell != (DBObjRecord *) NULL)
                do areaARR[toCell->RowID()] -= areaARR[cellID];
                while ((toCell = netIF->ToCell(toCell)) != (DBObjRecord *) NULL);

            snprintf(name, sizeof(name), "Point: %d", items->ItemNum());
            items->Add(name);
            pntRec = items->Item();
            coord = netIF->Center(netIF->Cell(cellID));
            coordField->Coordinate(pntRec, coord);
            symbolFLD->Record(pntRec, symRec);
            dataExtent.Expand(coord);
        }
    }
    Stop:
    delete netIF;
    free(areaARR);
    pntData->Extent(dataExtent);
    return (cellID >= 0 ? DBFault : DBSuccess);
}

DBInt RGlibNetworkAccumulate(DBObjData *netData, DBObjData *inGridData, DBObjData *outGridData) {
    return (RGlibNetworkAccumulate(netData, inGridData,
                                   (DBObjData *) NULL, (DBObjData *) NULL, (char **) NULL, 0.000001, true, false, true,
                                   outGridData));
}

class RGlibNetAccum {
public:
    DBFloat Correction;
    DBObjTableField *StnIDFLD;
    DBObjTableField *AreaFLD;
    DBObjTableField *DischFLD;
    DBGridIF *GridIF;
    DBObjRecord *LayerRec;
};

static DBInt _RGlibUpStreamACTION(DBNetworkIF *netIF, DBObjRecord *cellRec, RGlibNetAccum *netAccum) {
    DBFloat value, obsVal;
    if ((cellRec->Flags() & DBObjectFlagProcessed) == DBObjectFlagProcessed) return (true);
    if (netAccum->StnIDFLD->Int(cellRec) != DBFault) return (false);
    if (netAccum->GridIF->Value(netAccum->LayerRec, netIF->CellPosition(cellRec), &value)) {
        obsVal = netAccum->DischFLD->Float(cellRec);
        value = (value - obsVal) * netAccum->Correction + obsVal;
        netAccum->GridIF->Value(netAccum->LayerRec, netIF->CellPosition(cellRec), value);
    }
    return (true);
}

static DBInt _RGlibUniformACTION(DBNetworkIF *netIF, DBObjRecord *cellRec, RGlibNetAccum *netAccum) {
    DBFloat value;
    if ((cellRec->Flags() & DBObjectFlagProcessed) == DBObjectFlagProcessed) return (true);
    if (netAccum->StnIDFLD->Int(cellRec) != DBFault) return (false);
    if (netAccum->GridIF->Value(netAccum->LayerRec, netIF->CellPosition(cellRec), &value)) {
        value = value + netAccum->AreaFLD->Float(cellRec) * netAccum->Correction;
        netAccum->GridIF->Value(netAccum->LayerRec, netIF->CellPosition(cellRec), value);
    }
    return (true);
}

static DBInt _RGlibMainstemACTION(DBNetworkIF *netIF, DBObjRecord *cellRec, RGlibNetAccum *netAccum) {
    DBFloat value;
    if ((cellRec->Flags() & DBObjectFlagProcessed) == DBObjectFlagProcessed) return (true);
    if (netAccum->StnIDFLD->Int(cellRec) != DBFault) return (false);
    if ((cellRec->Flags() & DBObjectFlagLocked) != DBObjectFlagLocked) return (false);

    value = netAccum->DischFLD->Float(cellRec);
    value = value + netAccum->AreaFLD->Float(cellRec) * netAccum->Correction;
    netAccum->GridIF->Value(netAccum->LayerRec, netIF->CellPosition(cellRec), value);
    return (true);
}

DBInt RGlibNetworkAccumulate(DBObjData *netData,
                             DBObjData *inGridData,
                             DBObjData *stnData,
                             DBObjData *disData,
                             char *fields[],
                             DBFloat coeff,
                             bool areaMult,
                             bool correction,
                             bool allowNegative,
                             DBObjData *outGridData) {
    DBInt layerID, layerNum = 0, progress = 0, maxProgress;
    DBInt cellID, pointID, fieldID, disID;
    DBFloat value, obsVal, upObsVal, accumVal;
    DBPosition pos;
    DBCoordinate coord;
    DBDate date;
    DBObjRecord *outLayerRec, *cellRec, *toCellRec, *nextCellRec, *layerRec, *pointRec, *dischRec;
    DBGridIF *inGridIF;
    DBVPointIF *stnIF = (DBVPointIF *) NULL;
    DBNetworkIF *netIF;
    DBObjTable *stnTable, *disTable, *cellTable = netData->Table(DBrNCells);
    DBObjTableField *relateFLD = (DBObjTableField *) NULL;
    DBObjTableField *nextStnFLD = (DBObjTableField *) NULL;
    DBObjTableField *joinFLD = (DBObjTableField *) NULL;
    DBObjTableField *dateFLD = (DBObjTableField *) NULL;
    DBObjTableField *dischargeFLD = (DBObjTableField *) NULL;
    DBObjTableField *tmpDischFLD = (DBObjTableField *) NULL;
    RGlibNetAccum netAccum;

    if (stnData != (DBObjData *) NULL) {
        if (disData == (DBObjData *) NULL) return (DBFault);
        if (stnData->Type() != DBTypeVectorPoint) {
            CMmsgPrint(CMmsgUsrError, "Invalid station data!");
            return (DBFault);
        }
/*		if (disData->Type () != DBTypeTable)
			{ CMmsgPrint (CMmsgAppError, "Invalid time series data\n"); return (DBFault); }
*/        stnTable = stnData->Table(DBrNItems);
        disTable = disData->Table(DBrNItems);
        if ((fields[0] != (char *) NULL) &&
            ((relateFLD = stnTable->Field(fields[0])) == (DBObjTableField *) NULL)) {
            CMmsgPrint(CMmsgUsrError, "Invalid relate field [%s]!", fields[0]);
            return (DBFault);
        }
        nextStnFLD = stnTable->Field(fields[1] == (char *) NULL ? RGlibNextStation : fields[1]);
        if (nextStnFLD == (DBObjTableField *) NULL) {
            CMmsgPrint(CMmsgUsrError, "Invalid next station field!");
            return (DBFault);
        }
        if (nextStnFLD->Type() != DBVariableInt) {
            CMmsgPrint(CMmsgUsrError, "Invalid next station field!");
            return (DBFault);
        }
        if ((fields[2] != (char *) NULL) &&
            ((joinFLD = disTable->Field(fields[2])) == (DBObjTableField *) NULL)) {
            CMmsgPrint(CMmsgUsrError, "Invalid join field [%s]!", fields[2]);
            return (DBFault);
        }
        if ((relateFLD != (DBObjTableField *) NULL) && (DBTableFieldIsCategory(relateFLD) != true)) {
            CMmsgPrint(CMmsgUsrError, "Invalid relate field type!");
            return (DBFault);
        }
        if ((joinFLD != (DBObjTableField *) NULL) && (DBTableFieldIsCategory(joinFLD) != true)) {
            CMmsgPrint(CMmsgUsrError, "Invalid join field type!");
            return (DBFault);
        }
        if ((relateFLD != (DBObjTableField *) NULL) &&
            (joinFLD != (DBObjTableField *) NULL) &&
            (relateFLD->Type() != joinFLD->Type())) {
            CMmsgPrint(CMmsgUsrError, "Relate and join fields have different types!");
            return (DBFault);
        }
        if ((relateFLD != (DBObjTableField *) NULL) &&
            (joinFLD == (DBObjTableField *) NULL) &&
            (relateFLD->Type() != DBVariableString)) {
            CMmsgPrint(CMmsgUsrError, "Invalid relate field type!");
            return (DBFault);
        }
        if ((relateFLD == (DBObjTableField *) NULL) &&
            (joinFLD != (DBObjTableField *) NULL) &&
            (joinFLD->Type() != DBVariableString)) {
            CMmsgPrint(CMmsgUsrError, "Invalid join field type!");
            return (DBFault);
        }

        if (fields[3] != (char *) NULL) {
            if ((dateFLD = disTable->Field(fields[3])) == (DBObjTableField *) NULL) {
                CMmsgPrint(CMmsgUsrError, "Invalid date field [%s]!", fields[3]);
                return (DBFault);
            }
        }
        else {
            for (fieldID = 0; disTable->FieldNum(); ++fieldID) {
                dateFLD = disTable->Field(fieldID);
                if (dateFLD->Type() == DBVariableDate) break;
            }
            if (fieldID == disTable->FieldNum()) {
                CMmsgPrint(CMmsgUsrError, "No date field in time series!");
                return (DBFault);
            }
        }
        if (fields[4] != (char *) NULL) {
            if ((dischargeFLD = disTable->Field(fields[4])) == (DBObjTableField *) NULL) {
                CMmsgPrint(CMmsgUsrError, "Invalid discharge field [%s]!", fields[4]);
                return (DBFault);
            }
        }
        else {
            for (fieldID = 0; disTable->FieldNum(); ++fieldID) {
                dischargeFLD = disTable->Field(fieldID);
                if (dischargeFLD->Type() == DBVariableFloat) break;
            }
            if (fieldID == disTable->FieldNum()) {
                CMmsgPrint(CMmsgUsrError, "No discharge field in time series!");
                return (DBFault);
            }
        }
        stnIF = new DBVPointIF(stnData);

        tmpDischFLD = new DBObjTableField("_tempDisch_", DBVariableFloat, "%8.2f", sizeof(DBFloat4), false);
        stnTable->AddField(tmpDischFLD);
        disTable->ListSort(dateFLD);
        netAccum.StnIDFLD = new DBObjTableField("_tempID_", DBVariableInt, "%8d", sizeof(DBInt), false);
        cellTable->AddField(netAccum.StnIDFLD);
        netAccum.AreaFLD = new DBObjTableField("_tempArea_", DBVariableFloat, "%8.2f", sizeof(DBFloat), false);
        cellTable->AddField(netAccum.AreaFLD);
        netAccum.DischFLD = new DBObjTableField("_tempDisch_", DBVariableFloat, "%8.2f", sizeof(DBFloat), false);
        cellTable->AddField(netAccum.DischFLD);
        dischRec = disTable->First();
    }

    inGridIF = new DBGridIF(inGridData);
    for (layerID = 0; layerID < inGridIF->LayerNum(); ++layerID) {
        layerRec = inGridIF->Layer(layerID);
        if ((layerRec->Flags() & DBObjectFlagIdle) != DBObjectFlagIdle) ++layerNum;
    }
    if (layerNum < 1) {
        CMmsgPrint(CMmsgUsrError, "No Layer to Process!");
        delete inGridIF;
        return (DBFault);
    }

    netIF = new DBNetworkIF(netData);
    netAccum.GridIF = new DBGridIF(outGridData);

    outLayerRec = netAccum.GridIF->Layer((DBInt) 0);
    maxProgress = layerNum * netIF->RowNum();

    for (layerID = 0; layerID < inGridIF->LayerNum(); ++layerID) {
        layerRec = inGridIF->Layer(layerID);
        if ((layerRec->Flags() & DBObjectFlagIdle) == DBObjectFlagIdle) continue;
        netAccum.GridIF->RenameLayer(outLayerRec, layerRec->Name());
        for (pos.Row = 0; pos.Row < netIF->RowNum(); pos.Row++) {
            if (DBPause(progress * 100 / maxProgress)) goto Stop;
            progress++;
            for (pos.Col = 0; pos.Col < netIF->ColNum(); pos.Col++)
                if ((cellRec = netIF->Cell(pos)) == (DBObjRecord *) NULL)
                    netAccum.GridIF->Value(outLayerRec, pos, DBDefaultMissingFloatVal);
                else {
                    if (inGridIF->Value(layerRec, netIF->Center(cellRec), &value)) {
                        value = (areaMult ? value * netIF->CellArea(cellRec) : value) * coeff;
                        netAccum.GridIF->Value(outLayerRec, pos, value);
                    }
                    else netAccum.GridIF->Value(outLayerRec, pos, 0.0);
                }
        }
        if (stnIF != (DBVPointIF *) NULL) {
            date.Set(layerRec->Name());
            for (cellID = 0; cellID < netIF->CellNum(); ++cellID) {
                cellRec = netIF->Cell(cellID);
                cellRec->Flags(DBObjectFlagLocked, DBClear);
                cellRec->Flags(DBObjectFlagProcessed, DBClear);
                netAccum.StnIDFLD->Int(cellRec, DBFault);
                netAccum.AreaFLD->Float(cellRec, 0.0);
                netAccum.DischFLD->Float(cellRec, 0.0);
            }

            if (dischRec == (DBObjRecord *) NULL) dischRec = disTable->First();
            else {
                disID = dischRec->RowID();
                for (; dischRec != (DBObjRecord *) NULL; dischRec = disTable->Next()) {
                    if (date == dateFLD->Date(dischRec))break;
                }
                if (dischRec == (DBObjRecord *) NULL) {
                    for (dischRec = disTable->First();
                         (dischRec != (DBObjRecord *) NULL) && (dischRec->RowID() != disID);
                         dischRec = disTable->Next()) { if (date == dateFLD->Date(dischRec)) break; }
                }
            }
            if (dischRec != (DBObjRecord *) NULL) {
                for (pointID = 0; pointID < stnIF->ItemNum(); ++pointID) {
                    pointRec = stnIF->Item(pointID);
                    tmpDischFLD->Float(pointRec, tmpDischFLD->FloatNoData());
                }
                for (; dischRec != (DBObjRecord *) NULL; dischRec = disTable->Next()) {
                    if (date != dateFLD->Date(dischRec)) break;

                    for (pointID = 0; pointID < stnIF->ItemNum(); ++pointID) {
                        pointRec = stnIF->Item(pointID);
                        if (DBTableFieldMatch(relateFLD, pointRec, joinFLD, dischRec)) break;
                    }
                    if (pointID == stnIF->ItemNum()) continue;

                    coord = stnIF->Coordinate(pointRec);
                    if ((cellRec = netIF->Cell(coord)) == (DBObjRecord *) NULL) continue;
                    value = dischargeFLD->Float(dischRec);
                    if (CMmathEqualValues(value, dischargeFLD->FloatNoData()) == false) {
                        tmpDischFLD->Float(pointRec, value);
                        netAccum.StnIDFLD->Int(cellRec, pointRec->RowID());
                    }
                }
            }
            for (cellID = netIF->CellNum() - 1; cellID >= 0; --cellID) {
                cellRec = netIF->Cell(cellID);
                obsVal = netIF->CellArea(cellRec);
                for (nextCellRec = cellRec;
                     nextCellRec != (DBObjRecord *) NULL;
                     nextCellRec = netIF->ToCell(nextCellRec)) {
                    netAccum.AreaFLD->Float(nextCellRec, netAccum.AreaFLD->Float(nextCellRec) + obsVal);
                    if (netAccum.StnIDFLD->Int(nextCellRec) != DBFault) break;
                }
            }
        }

        for (cellID = netIF->CellNum() - 1; cellID >= 0; --cellID) {
            cellRec = netIF->Cell(cellID);
            if ((cellRec->Flags() & DBObjectFlagIdle) == DBObjectFlagIdle) continue;
            if ((netAccum.GridIF->Value(outLayerRec, netIF->CellPosition(cellRec), &value) == false) ||
                ((allowNegative == false) && (value < 0.0))) {
                value = 0.0;
                netAccum.GridIF->Value(outLayerRec, netIF->CellPosition(cellRec), value);
            }
            if ((toCellRec = netIF->ToCell(cellRec)) == (DBObjRecord *) NULL) continue;
            if (netAccum.GridIF->Value(outLayerRec, netIF->CellPosition(toCellRec), &accumVal) == false)
                accumVal = 0.0;
            if ((stnIF != (DBVPointIF *) NULL) && ((pointID = netAccum.StnIDFLD->Int(cellRec)) != DBFault)) {
                pointRec = stnIF->Item(pointID);
                obsVal = tmpDischFLD->Float(pointRec);
                if (correction) {
                    for (nextCellRec = netIF->ToCell(cellRec);
                         nextCellRec != (DBObjRecord *) NULL;
                         nextCellRec = netIF->ToCell(nextCellRec)) {
                        nextCellRec->Flags(DBObjectFlagLocked, DBSet);
                        netAccum.DischFLD->Float(nextCellRec, netAccum.DischFLD->Float(nextCellRec) + obsVal);
                        if (netAccum.StnIDFLD->Int(nextCellRec) != DBFault) break;
                    }
                    upObsVal = netAccum.DischFLD->Float(cellRec);
                    netAccum.LayerRec = outLayerRec;
                    cellRec->Flags(DBObjectFlagProcessed, DBSet);
                    if (obsVal > upObsVal) {
                        if (value > upObsVal) {
                            netAccum.Correction = (obsVal - upObsVal) / (value - upObsVal);
                            netIF->UpStreamSearch(cellRec, (DBNetworkACTION) _RGlibUpStreamACTION,
                                                  (void *) &netAccum);
                        }
                        else {
                            netAccum.Correction = (obsVal - value) / netAccum.AreaFLD->Float(cellRec);
                            netIF->UpStreamSearch(cellRec, (DBNetworkACTION) _RGlibUniformACTION,
                                                  (void *) &netAccum);
                        }
                    }
                    else {
                        netAccum.Correction = (obsVal - upObsVal) / netAccum.AreaFLD->Float(cellRec);
                        netIF->UpStreamSearch(cellRec, (DBNetworkACTION) _RGlibMainstemACTION, (void *) &netAccum);
                    }
                    cellRec->Flags(DBObjectFlagProcessed, DBClear);
                }
                value = obsVal;
                netAccum.GridIF->Value(outLayerRec, netIF->CellPosition(cellRec), value);
            }
            accumVal = accumVal + value;
            netAccum.GridIF->Value(outLayerRec, netIF->CellPosition(toCellRec), accumVal);
        }
        netAccum.GridIF->RecalcStats(outLayerRec);
        if (netAccum.GridIF->LayerNum() < layerNum) outLayerRec = netAccum.GridIF->AddLayer((char *) "Next Layer");
    }
    Stop:
    outGridData->Flags(DBDataFlagDispModeContShadeSets, DBClear);
    outGridData->Flags(DBDataFlagDispModeContBlueScale, DBSet);
    if (stnIF != (DBVPointIF *) NULL) {
        for (cellID = 0; cellID < netIF->CellNum(); ++cellID) {
            cellRec = netIF->Cell(cellID);
            cellRec->Flags(DBObjectFlagLocked, DBClear);
        }
        cellTable->DeleteField(netAccum.StnIDFLD);
        cellTable->DeleteField(netAccum.AreaFLD);
        cellTable->DeleteField(netAccum.DischFLD);
        stnTable->DeleteField(tmpDischFLD);
        delete stnIF;
    }
    delete netIF;
    delete inGridIF;
    delete netAccum.GridIF;
    return (progress == maxProgress ? DBSuccess : DBFault);
}

DBInt RGlibNetworkUnaccumulate(DBObjData *netData, DBObjData *inGridData, DBFloat coeff, bool areaDiv,
                               DBObjData *outGridData) {
    DBInt layerID, progress = 0, maxProgress;
    DBInt cellID;
    DBFloat value, unAccum;
    DBPosition pos;
    DBObjRecord *cellRec, *toCell, *inLayerRec, *outLayerRec;
    DBGridIF *inGridIF = new DBGridIF(inGridData);
    DBGridIF *outGridIF = new DBGridIF(outGridData);
    DBNetworkIF *netIF = new DBNetworkIF(netData);

    maxProgress = inGridIF->LayerNum() * netIF->CellNum();
    for (layerID = 0; layerID < inGridIF->LayerNum(); ++layerID) {
        inLayerRec = inGridIF->Layer(layerID);
        if (layerID == 0) {
            outLayerRec = outGridIF->Layer(layerID);
            outGridIF->RenameLayer(outLayerRec, inLayerRec->Name());
        }
        else
            outLayerRec = outGridIF->AddLayer(inLayerRec->Name());

        value = outGridIF->MissingValue();
        for (pos.Col = 0; pos.Col < netIF->ColNum(); ++pos.Col)
            for (pos.Row = 0; pos.Row < netIF->RowNum(); ++pos.Row) outGridIF->Value(outLayerRec, pos, value);

        for (cellID = 0; cellID < netIF->CellNum(); ++cellID) {
            progress = layerID * netIF->CellNum() + cellID;
            if (DBPause(progress * 100 / maxProgress)) goto Stop;

            cellRec = netIF->Cell(cellID);
            if (inGridIF->Value(inLayerRec, netIF->Center(cellRec), &value) == false) continue;
            outGridIF->Value(outLayerRec, netIF->CellPosition(cellRec), value);

            if (((toCell = netIF->ToCell(cellRec)) == (DBObjRecord *) NULL) ||
                (outGridIF->Value(outLayerRec, netIF->CellPosition(toCell), &unAccum) == false))
                continue;
            unAccum = unAccum - value;
            outGridIF->Value(outLayerRec, netIF->CellPosition(toCell), unAccum);
        }
        if (areaDiv)
            for (cellID = 0; cellID < netIF->CellNum(); ++cellID) {
                cellRec = netIF->Cell(cellID);
                outGridIF->Value(outLayerRec, netIF->CellPosition(cellRec), &value);
                value = coeff * value / netIF->CellArea(cellRec);
                outGridIF->Value(outLayerRec, netIF->CellPosition(cellRec), value);
            }
        else
            for (cellID = 0; cellID < netIF->CellNum(); ++cellID) {
                cellRec = netIF->Cell(cellID);
                outGridIF->Value(outLayerRec, netIF->CellPosition(cellRec), &value);
                value = coeff * value;
                outGridIF->Value(outLayerRec, netIF->CellPosition(cellRec), value);
            }

    }
    outGridIF->RecalcStats();
    Stop:
    return (progress + 1 == maxProgress ? DBSuccess : DBFault);
}

DBInt RGlibNetworkInvAccumulate(DBObjData *netData, DBObjData *inGridData, DBObjData *outGridData, DBFloat coeff) {
    DBInt layerID, progress = 0, maxProgress;
    DBInt cellID;
    DBFloat value, accumVal;
    DBPosition pos;
    DBObjRecord *cellRec, *toCell, *inLayerRec, *outLayerRec;
    DBGridIF *inGridIF = new DBGridIF(inGridData);
    DBGridIF *outGridIF = new DBGridIF(outGridData);
    DBNetworkIF *netIF = new DBNetworkIF(netData);

    maxProgress = inGridIF->LayerNum() * netIF->CellNum();
    for (layerID = 0; layerID < inGridIF->LayerNum(); ++layerID) {
        inLayerRec = inGridIF->Layer(layerID);
        if (layerID == 0) {
            outLayerRec = outGridIF->Layer(layerID);
            outGridIF->RenameLayer(outLayerRec, inLayerRec->Name());
        }
        else
            outLayerRec = outGridIF->AddLayer(inLayerRec->Name());

        accumVal = outGridIF->MissingValue();
        for (pos.Col = 0; pos.Col < netIF->ColNum(); ++pos.Col)
            for (pos.Row = 0; pos.Row < netIF->RowNum(); ++pos.Row) outGridIF->Value(outLayerRec, pos, accumVal);

        for (cellID = 0; cellID < netIF->CellNum(); ++cellID) {
            progress = layerID * netIF->CellNum() + cellID;
            if (DBPause(progress * 100 / maxProgress)) goto Stop;

            cellRec = netIF->Cell(cellID);
            accumVal = 0.0;
            for (toCell = cellRec; toCell != (DBObjRecord *) NULL; toCell = netIF->ToCell(toCell)) {
                if (inGridIF->Value(inLayerRec, netIF->Center(toCell), &value) == false) continue;
                else accumVal = accumVal + value * coeff;
            }
            outGridIF->Value(outLayerRec, netIF->CellPosition(cellRec), accumVal);
        }
    }
    outGridIF->RecalcStats();
    Stop:
    return (progress + 1 == maxProgress ? DBSuccess : DBFault);
}

DBInt RGlibNetworkUpStreamAvg(DBObjData *netData, DBObjData *inGridData, DBObjData *outGridData) {
    DBInt layerID, layerNum = 0, progress = 0, maxProgress, cellID;
    DBFloat value, accumVal, *upstreamArea;
    DBPosition pos;
    DBObjRecord *outLayerRec, *cellRec, *toCellRec, *layerRec;
    DBGridIF *inGridIF, *outGridIF;
    DBNetworkIF *netIF;

    inGridIF = new DBGridIF(inGridData);
    for (layerID = 0; layerID < inGridIF->LayerNum(); ++layerID) {
        layerRec = inGridIF->Layer(layerID);
        if ((layerRec->Flags() & DBObjectFlagIdle) != DBObjectFlagIdle) ++layerNum;
    }
    if (layerNum < 1) {
        CMmsgPrint(CMmsgUsrError, "No Layer to Process!");
        delete inGridIF;
        return (DBFault);
    }
    netIF = new DBNetworkIF(netData);
    outGridIF = new DBGridIF(outGridData);
    outGridIF->MissingValue(inGridIF->MissingValue());

    outLayerRec = outGridIF->Layer((DBInt) 0);
    maxProgress = layerNum * netIF->RowNum();
    if ((upstreamArea = (DBFloat *) calloc(netIF->CellNum(), sizeof(DBFloat))) == (DBFloat *) NULL) {
        CMmsgPrint(CMmsgSysError, "Memory Allocation Error in: %s %d", __FILE__, __LINE__);
        delete netIF;
        delete outGridIF;
        delete inGridIF;
        return (DBFault);
    }

    for (layerID = 0; layerID < inGridIF->LayerNum(); ++layerID) {
        layerRec = inGridIF->Layer(layerID);
        if ((layerRec->Flags() & DBObjectFlagIdle) == DBObjectFlagIdle) continue;

        outGridIF->RenameLayer(outLayerRec, layerRec->Name());
        for (cellID = 0; cellID < netIF->CellNum(); cellID++) upstreamArea[cellID] = 0.0;
        for (pos.Row = 0; pos.Row < netIF->RowNum(); pos.Row++) {
            if (DBPause(progress * 100 / maxProgress)) goto Stop;
            progress++;
            for (pos.Col = 0; pos.Col < netIF->ColNum(); pos.Col++)
                if ((cellRec = netIF->Cell(pos)) == (DBObjRecord *) NULL)
                    outGridIF->Value(outLayerRec, pos, outGridIF->MissingValue());
                else {
                    if (inGridIF->Value(layerRec, netIF->Center(cellRec), &value)) {
                        outGridIF->Value(outLayerRec, pos, value * netIF->CellArea(cellRec));
                        upstreamArea[cellRec->RowID()] = netIF->CellArea(cellRec);
                    }
                    else {
                        outGridIF->Value(outLayerRec, pos, 0.0);
                        upstreamArea[cellRec->RowID()] = 0.0;
                    }
                }
        }
        for (cellID = netIF->CellNum() - 1; cellID >= 0; --cellID) {
            cellRec = netIF->Cell(cellID);
            if ((cellRec->Flags() & DBObjectFlagIdle) == DBObjectFlagIdle) continue;
            if (outGridIF->Value(outLayerRec, netIF->CellPosition(cellRec), &value) == false) {
                CMmsgPrint(CMmsgAppError, "Total metal Gebasz in: %s %d", __FILE__, __LINE__);
                value = 0.0;
            }
            if ((toCellRec = netIF->ToCell(cellRec)) != (DBObjRecord *) NULL) {
                if (outGridIF->Value(outLayerRec, netIF->CellPosition(toCellRec), &accumVal) == false) accumVal = 0.0;
                outGridIF->Value(outLayerRec, netIF->CellPosition(toCellRec), accumVal + value);
                upstreamArea[toCellRec->RowID()] += upstreamArea[cellID];
            }
            if (upstreamArea[cellID] > 0.0)
                outGridIF->Value(outLayerRec, netIF->CellPosition(cellRec), value / upstreamArea[cellID]);
            else outGridIF->Value(outLayerRec, netIF->CellPosition(cellRec), DBDefaultMissingFloatVal);
        }
        outGridIF->RecalcStats(outLayerRec);
        if (outGridIF->LayerNum() < layerNum) outLayerRec = outGridIF->AddLayer((char *) "Next Layer");
    }
    Stop:
    outGridData->Flags(DBDataFlagDispModeContShadeSets, DBClear);
    outGridData->Flags(DBDataFlagDispModeContBlueScale, DBSet);
    free(upstreamArea);
    delete netIF;
    delete outGridIF;
    delete inGridIF;
    return (progress == maxProgress ? DBSuccess : DBFault);
}

DBInt RGlibNetworkCellSlopes(DBObjData *netData, DBObjData *inGridData, DBObjData *outGridData) {
    DBInt layerID, layerNum = 0, progress = 0, maxProgress;
    DBFloat value, slope;
    DBPosition pos;
    DBCoordinate coord0, coord1;
    DBObjRecord *outLayerRec, *cellRec, *layerRec;
    DBGridIF *inGridIF, *outGridIF;
    DBNetworkIF *netIF;
 
    inGridIF = new DBGridIF(inGridData);
    for (layerID = 0; layerID < inGridIF->LayerNum(); ++layerID) {
        layerRec = inGridIF->Layer(layerID);
        if ((layerRec->Flags() & DBObjectFlagIdle) != DBObjectFlagIdle) ++layerNum;
    }
    if (layerNum < 1) {
        CMmsgPrint(CMmsgUsrError, "No Layer to Process!");
        delete inGridIF;
        return (DBFault);
    }

    netIF = new DBNetworkIF(netData);
    outGridIF = new DBGridIF(outGridData);

    outLayerRec = outGridIF->Layer((DBInt) 0);

    maxProgress = layerNum * netIF->RowNum();

    for (layerID = 0; layerID < inGridIF->LayerNum(); ++layerID) {
        layerRec = inGridIF->Layer(layerID);
        if ((layerRec->Flags() & DBObjectFlagIdle) == DBObjectFlagIdle) continue;

        outGridIF->RenameLayer(outLayerRec, layerRec->Name());
        for (pos.Row = 0; pos.Row < netIF->RowNum(); pos.Row++) {
            if (DBPause(progress * 100 / maxProgress)) goto Stop;
            progress++;
            for (pos.Col = 0; pos.Col < netIF->ColNum(); pos.Col++)
                if ((cellRec = netIF->Cell(pos)) == (DBObjRecord *) NULL)
                    outGridIF->Value(outLayerRec, pos, DBDefaultMissingFloatVal);
                else {
                    coord0 = netIF->Center(cellRec);
                    if (inGridIF->Value(layerRec, coord0, &slope)) {
                        coord1 = coord0 + netIF->Delta(cellRec);
                        if ((netIF->CellLength(cellRec) > 0.0) && (inGridIF->Value(layerRec, coord1, &value))) {
                            slope = (slope - value) / netIF->CellLength(cellRec);
                            outGridIF->Value(outLayerRec, pos, slope);
                        }
                        else outGridIF->Value(outLayerRec, pos, (DBFloat) RGlibMinSLOPE);
                    }
                    else outGridIF->Value(outLayerRec, pos, DBDefaultMissingFloatVal);
                }
        }
        outGridIF->RecalcStats(outLayerRec);
        if (outGridIF->LayerNum() < layerNum) outLayerRec = outGridIF->AddLayer((char *) "Next Layer");
    }
    Stop:
    outGridData->Flags(DBDataFlagDispModeContShadeSets, DBClear);
    outGridData->Flags(DBDataFlagDispModeContGreyScale, DBSet);
    delete netIF;
    delete outGridIF;
    delete inGridIF;
    return (progress == maxProgress ? DBSuccess : DBFault);
}

DBInt RGlibNetworkConfluences(DBObjData *netData, DBObjData *outPntData) {
    char recordName[DBStringLength];
    DBInt cellId, pntId = 0, cellOrder, maxOrder = 0, minOrder = 0;
    DBCoordinate coord;
    DBNetworkIF *netIF = new DBNetworkIF(netData);
    DBVPointIF *pntIF = new DBVPointIF(outPntData);
    DBObjRecord *cellRec, *toCellRec, *pntRec;
    DBObjTable *itemTable = outPntData->Table(DBrNItems);
    DBObjTableField *orderFLD = new DBObjTableField("Order", DBTableFieldInt, "%2d", sizeof(DBByte));

    itemTable->AddField(orderFLD);

    for (cellId = 0; cellId < netIF->CellNum(); ++cellId) {
        cellOrder = netIF->CellOrder(cellId);
        maxOrder = maxOrder > cellOrder ? maxOrder : cellOrder;
    }
    switch (maxOrder) {
        case 0:
        case 1:
        case 2:
        case 3:
            minOrder = 0;
            break;
        case 4:
            minOrder = 1;
            break;
        case 5:
            minOrder = 2;
            break;
        case 6:
            minOrder = 3;
            break;
    }
    for (cellOrder = minOrder; cellOrder < maxOrder; ++cellOrder) {
        snprintf(recordName, sizeof(recordName), "Order: %2d", cellOrder + 1);
        pntIF->NewSymbol(recordName);
    }
    for (cellId = 0; cellId < netIF->CellNum(); ++cellId) {
        cellRec = netIF->Cell(cellId);
        if ((cellOrder = netIF->CellOrder(cellRec)) < minOrder) continue;
        if (((toCellRec = netIF->ToCell(cellRec)) != (DBObjRecord *) NULL) &&
            (netIF->CellOrder(toCellRec) == cellOrder))
            continue;
        snprintf(recordName, sizeof(recordName), "Confluence%010d", pntId++);
        pntRec = pntIF->NewItem(recordName);
        pntIF->Coordinate(pntRec, netIF->Center(cellRec));
        pntIF->ItemSymbol(pntRec, pntIF->Symbol(cellOrder - 1));
        orderFLD->Int(pntRec, cellOrder);
    }
    return (DBSuccess);
}

DBInt RGlibNetworkBasinProf(DBObjData *netData, DBObjData *gridData, DBObjData *tblData) {
    DBInt layerID, layerNum = 0, basinID, ret;
    DBFloat dist, value;
    DBCoordinate coord;
    DBNetworkIF *netIF;
    DBGridIF *gridIF;
    DBObjTable *table;
    DBObjTableField *basinFLD;
    DBObjTableField *xCoordFLD;
    DBObjTableField *yCoordFLD;
    DBObjTableField *distFLD;
    DBObjTableField *valueFLD;
    DBObjectLIST<DBObjTableField> *fields;
    DBObjRecord *basinRec, *cellRec, *layerRec, *tblRec;

    gridIF = new DBGridIF(gridData);
    for (layerID = 0; layerID < gridIF->LayerNum(); ++layerID) {
        layerRec = gridIF->Layer(layerID);
        if ((layerRec->Flags() & DBObjectFlagIdle) != DBObjectFlagIdle) ++layerNum;
    }
    if (layerNum < 1) {
        CMmsgPrint(CMmsgUsrError, "No Layer to Process!");
        delete gridIF;
        return (DBFault);
    }

    table = tblData->Table(DBrNItems);
    fields = table->Fields();
    netIF = new DBNetworkIF(netData);
    table->AddField(basinFLD = new DBObjTableField(DBrNBasin, DBTableFieldInt, "%8d", sizeof(DBInt)));
    table->AddField(xCoordFLD = new DBObjTableField("XCoord", DBTableFieldFloat, "%8.3f", sizeof(DBFloat)));
    table->AddField(yCoordFLD = new DBObjTableField("YCoord", DBTableFieldFloat, "%8.3f", sizeof(DBFloat)));
    table->AddField(distFLD = new DBObjTableField("Distance", DBTableFieldFloat, "%8.0f", sizeof(DBFloat)));

    for (layerID = 0; layerID < gridIF->LayerNum(); ++layerID) {
        layerRec = gridIF->Layer(layerID);
        if ((layerRec->Flags() & DBObjectFlagIdle) == DBObjectFlagIdle) continue;
        table->AddField(valueFLD = new DBObjTableField(layerRec->Name(), DBTableFieldFloat, "%8.3f", sizeof(DBFloat)));
        valueFLD->FloatNoData(gridIF->MissingValue(layerRec));
    }

    for (basinID = 0; basinID < netIF->BasinNum(); ++basinID) {
        basinRec = netIF->Basin(basinID);
        dist = 0.0;
        if (DBPause(basinID * 100 / netIF->BasinNum())) goto Stop;
        for (cellRec = netIF->MouthCell(basinRec);
             cellRec != (DBObjRecord *) NULL; cellRec = netIF->FromCell(cellRec)) {
            coord = netIF->Center(cellRec);
            tblRec = table->Add(basinRec->Name());
            basinFLD->Int(tblRec, basinRec->RowID() + 1);
            xCoordFLD->Float(tblRec, coord.X);
            yCoordFLD->Float(tblRec, coord.Y);
            dist += netIF->CellLength(cellRec);
            distFLD->Float(tblRec, dist);
            for (layerID = 0; layerID < gridIF->LayerNum(); ++layerID) {
                layerRec = gridIF->Layer(layerID);
                if ((layerRec->Flags() & DBObjectFlagIdle) == DBObjectFlagIdle) continue;
                valueFLD = fields->Next();
                if (gridIF->Value(layerRec, coord, &value)) valueFLD->Float(tblRec, value);
                else valueFLD->Float(tblRec, valueFLD->FloatNoData());
            }
        }
    }
    Stop:
    ret = basinID == netIF->BasinNum() ? DBSuccess : DBFault;
    delete netIF;
    delete gridIF;
    return (ret);
}

#define RGISStatMin        "StatMinimum"
#define RGISStatMax        "StatMaximum"
#define RGISStatMean        "StatMean"
#define RGISStatStdDev    "StatStdDev"
#define RGISStatArea        "StatArea"

DBInt RGlibNetworkBasinStats(DBObjData *netData, DBObjData *grdData, DBObjData *tblData) {
    DBInt layerID, layerNum = 0, cellID, basinID, progress = 0, maxProgress;
    DBFloat minimum, maximum, average, stdDev, area, value;
    DBObjTable *table;
    DBObjTableField *basinIDFLD;
    DBObjTableField *layerIDFLD;
    DBObjTableField *layerNameFLD;
    DBObjTableField *minimumFLD;
    DBObjTableField *maximumFLD;
    DBObjTableField *averageFLD;
    DBObjTableField *stdDevFLD;
    DBObjTableField *areaFLD;
    DBNetworkIF *netIF;
    DBGridIF *gridIF;
    DBObjRecord *layerRec, *cellRec, *basinRec, *tblRec;
    DBObjectLIST<DBObjTableField> *fields;

    gridIF = new DBGridIF(grdData);
    for (layerID = 0; layerID < gridIF->LayerNum(); ++layerID) {
        layerRec = gridIF->Layer(layerID);
        if ((layerRec->Flags() & DBObjectFlagIdle) != DBObjectFlagIdle) ++layerNum;
    }
    if (layerNum < 1) {
        CMmsgPrint(CMmsgUsrError, "No Layer to Process!");
        delete gridIF;
        return (DBFault);
    }

    table = tblData->Table(DBrNItems);
    netIF = new DBNetworkIF(netData);
    table->AddField(basinIDFLD = new DBObjTableField(DBrNBasin, DBTableFieldInt, "%8d", sizeof(DBInt)));
    table->AddField(layerIDFLD = new DBObjTableField("LayerID", DBTableFieldInt, "%4d", sizeof(DBShort)));
    table->AddField(layerNameFLD = new DBObjTableField("LayerName", DBTableFieldString, "%s", DBStringLength));
    table->AddField(
            averageFLD = new DBObjTableField(RGISStatMean, DBTableFieldFloat, gridIF->ValueFormat(), sizeof(DBFloat4)));
    table->AddField(
            minimumFLD = new DBObjTableField(RGISStatMin, DBTableFieldFloat, gridIF->ValueFormat(), sizeof(DBFloat4)));
    table->AddField(
            maximumFLD = new DBObjTableField(RGISStatMax, DBTableFieldFloat, gridIF->ValueFormat(), sizeof(DBFloat4)));
    table->AddField(stdDevFLD = new DBObjTableField(RGISStatStdDev, DBTableFieldFloat, gridIF->ValueFormat(),
                                                    sizeof(DBFloat4)));
    table->AddField(
            areaFLD = new DBObjTableField(RGISStatArea, DBTableFieldFloat, gridIF->ValueFormat(), sizeof(DBFloat4)));

    maxProgress = netIF->CellNum() * layerNum;
    for (layerID = 0; layerID < gridIF->LayerNum(); ++layerID) {
        layerRec = gridIF->Layer(layerID);
        if ((layerRec->Flags() & DBObjectFlagIdle) == DBObjectFlagIdle) continue;
        basinID = DBFault;
        for (cellID = 0; cellID < netIF->CellNum(); ++cellID) {
            cellRec = netIF->Cell(cellID);
            if (DBPause(progress * 100 / maxProgress)) goto Stop;
            progress++;
            if ((cellRec->Flags() & DBObjectFlagIdle) == DBObjectFlagIdle) continue;
            if (basinID != netIF->CellBasinID(cellRec)) {
                if (basinID != DBFault) {
                    average = average / area;
                    stdDev = stdDev / area;
                    stdDev = stdDev - average * average;
                    stdDev = stdDev > 0.0 ? sqrt (stdDev) : 0.0;
                    minimumFLD->Float(tblRec, minimum);
                    maximumFLD->Float(tblRec, maximum);
                    averageFLD->Float(tblRec, average);
                    stdDevFLD->Float(tblRec, stdDev);
                    areaFLD->Float(tblRec, area);
                }
                basinID = netIF->CellBasinID(cellRec);
                basinRec = netIF->Basin(cellRec);
                tblRec = table->Add(basinRec->Name());
                basinIDFLD->Int(tblRec, basinID);
                layerIDFLD->Int(tblRec, layerRec->RowID());
                layerNameFLD->String(tblRec, layerRec->Name());
                area = 0.0;
                minimum = DBHugeVal;
                maximum = -DBHugeVal;
                average = 0.0;
                stdDev = 0.0;
            }
            if (gridIF->Value(layerRec, netIF->Center(cellRec), &value) == true) {
                average = average + value * netIF->CellArea(cellRec);
                minimum = minimum < value ? minimum : value;
                maximum = maximum > value ? maximum : value;
                stdDev = stdDev + value * value * netIF->CellArea(cellRec);
                area = area + netIF->CellArea(cellRec);
            }
        }
        average = average / area;
        stdDev = stdDev / area;
        stdDev = stdDev - average * average;
        stdDev = stdDev > 0.0 ? sqrt(stdDev) : 0.0;
        minimumFLD->Float(tblRec, minimum);
        maximumFLD->Float(tblRec, maximum);
        averageFLD->Float(tblRec, average);
        stdDevFLD->Float(tblRec, stdDev);
        areaFLD->Float(tblRec, area);
    }
    Stop:
    fields = new DBObjectLIST<DBObjTableField>("Field List");
    fields->Add(new DBObjTableField(*basinIDFLD));
    fields->Add(new DBObjTableField(*layerIDFLD));
    table->ListSort(fields);
    delete fields;
    delete netIF;
    delete gridIF;
    return (progress == maxProgress ? DBSuccess : DBFault);
}

#define RGISHeadMin            "HeadMinimum"
#define RGISHeadMax            "HeadMaximum"
#define RGISHeadMean            "HeadMean"
#define RGISHeadStdDev        "HeadStdDev"

#define RGISDivideMin        "DivideMinimum"
#define RGISDivideMax        "DivideMaximum"
#define RGISDivideMean        "DivideMean"
#define RGISDivideStdDev    "DivideStdDev"

DBInt RGlibNetworkHeadStats(DBObjData *netData, DBObjData *grdData, DBObjData *tblData, DBInt divideOnly) {
    DBInt layerID, layerNum = 0, cellID, basinID, progress = 0, maxProgress, dir, divide;
    DBFloat minimum, maximum, average, stdDev, value, area;
    DBObjTable *table;
    DBObjTableField *basinIDFLD;
    DBObjTableField *layerIDFLD;
    DBObjTableField *layerNameFLD;
    DBObjTableField *minimumFLD;
    DBObjTableField *maximumFLD;
    DBObjTableField *averageFLD;
    DBObjTableField *stdDevFLD;
    DBNetworkIF *netIF;
    DBGridIF *gridIF;
    DBObjRecord *layerRec, *cellRec, *fromCell, *basinRec, *tblRec;
    DBObjectLIST<DBObjTableField> *fields;

    gridIF = new DBGridIF(grdData);
    for (layerID = 0; layerID < gridIF->LayerNum(); ++layerID) {
        layerRec = gridIF->Layer(layerID);
        if ((layerRec->Flags() & DBObjectFlagIdle) != DBObjectFlagIdle) ++layerNum;
    }
    if (layerNum < 1) {
        CMmsgPrint(CMmsgAppError, "No Layer to Process in RGlibNetworkHeadStats ()\n");
        delete gridIF;
        return (DBFault);
    }

    table = tblData->Table(DBrNItems);
    netIF = new DBNetworkIF(netData);
    table->AddField(basinIDFLD   = new DBObjTableField(DBrNBasin, DBTableFieldInt, "%8d", sizeof(DBInt)));
    table->AddField(layerIDFLD   = new DBObjTableField("LayerID", DBTableFieldInt, "%4d", sizeof(DBShort)));
    table->AddField(layerNameFLD = new DBObjTableField("LayerName", DBTableFieldString, "%s", DBStringLength));
    table->AddField(averageFLD   = new DBObjTableField(divideOnly ? RGISDivideMean : RGISHeadMean, DBTableFieldFloat,
                                                     gridIF->ValueFormat(), sizeof(DBFloat4)));
    table->AddField(minimumFLD   = new DBObjTableField(divideOnly ? RGISDivideMin : RGISHeadMin, DBTableFieldFloat,
                                                     gridIF->ValueFormat(), sizeof(DBFloat4)));
    table->AddField(maximumFLD   = new DBObjTableField(divideOnly ? RGISDivideMax : RGISHeadMax, DBTableFieldFloat,
                                                     gridIF->ValueFormat(), sizeof(DBFloat4)));
    table->AddField(stdDevFLD    = new DBObjTableField(divideOnly ? RGISDivideStdDev : RGISHeadStdDev, DBTableFieldFloat,
                                                    gridIF->ValueFormat(), sizeof(DBFloat4)));

    maxProgress = netIF->CellNum() * layerNum;
    for (layerID = 0; layerID < gridIF->LayerNum(); ++layerID) {
        layerRec = gridIF->Layer(layerID);
        if ((layerRec->Flags() & DBObjectFlagIdle) == DBObjectFlagIdle) continue;
        basinID = DBFault;
        for (cellID = 0; cellID < netIF->CellNum(); ++cellID) {
            cellRec = netIF->Cell(cellID);
            if (DBPause(progress * 100 / maxProgress)) goto Stop;
            progress++;
            if ((cellRec->Flags() & DBObjectFlagIdle) == DBObjectFlagIdle) continue;
            if (basinID != netIF->CellBasinID(cellRec)) {
                if (basinID != DBFault) {
                    average = average / area;
                    stdDev = stdDev / area;
                    stdDev = stdDev - average * average;
                    stdDev = sqrt(stdDev);
                    minimumFLD->Float(tblRec, minimum);
                    maximumFLD->Float(tblRec, maximum);
                    averageFLD->Float(tblRec, average);
                    stdDevFLD->Float(tblRec, stdDev);
                }
                basinID = netIF->CellBasinID(cellRec);
                basinRec = netIF->Basin(cellRec);
                tblRec = table->Add(basinRec->Name());
                basinIDFLD->Int(tblRec, basinID);
                layerIDFLD->Int(tblRec, layerRec->RowID());
                layerNameFLD->String(tblRec, layerRec->Name());
                area = 0.0;
                minimum = DBHugeVal;
                maximum = -DBHugeVal;
                average = 0.0;
                stdDev = 0.0;
            }
            if ((netIF->CellBasinCells(cellRec) == 1) &&
                (gridIF->Value(layerRec, netIF->Center(cellRec), &value) == true)) {
                if (divideOnly) {
                    divide = false;
                    for (dir = 0; dir < 8; ++dir)
                        if ((fromCell = netIF->FromCell(cellRec, 0x01 << dir, false)) == (DBObjRecord *) NULL) {
                            divide = true;
                            break;
                        }
                        else if (netIF->CellBasinID(fromCell) != basinID) {
                            divide = true;
                            break;
                        }
                }
                else divide = true;
                if (divide) {
                    average = average + value * netIF->CellArea(cellRec);
                    minimum = minimum < value ? minimum : value;
                    maximum = maximum > value ? maximum : value;
                    stdDev = stdDev + value * value * netIF->CellArea(cellRec);
                    area = area + netIF->CellArea(cellRec);
                }
            }
        }
        average = average / area;
        stdDev = stdDev / area;
        stdDev = stdDev - average * average;
        stdDev = stdDev > 0.0 ? sqrt(stdDev) : 0.0;
        minimumFLD->Float(tblRec, minimum);
        maximumFLD->Float(tblRec, maximum);
        averageFLD->Float(tblRec, average);
        stdDevFLD->Float(tblRec, stdDev);
    }
    Stop:
    fields = new DBObjectLIST<DBObjTableField>("Field List");
    fields->Add(new DBObjTableField(*basinIDFLD));
    fields->Add(new DBObjTableField(*layerIDFLD));
    table->ListSort(fields);
    delete fields;
    delete netIF;
    delete gridIF;
    return (progress == maxProgress ? DBSuccess : DBFault);
}

DBInt RGlibNetworkHistogram(DBObjData *netData, DBObjData *grdData, DBObjData *tblData) {
    char string[DBStringLength];
    DBInt cellID, basinID = DBFault, i = 0, ret = DBSuccess;
    DBFloat area = 0.0, elev;
    DBObjTable *itemTable = tblData->Table(DBrNItems);
    DBObjTableField *basinIDFLD;
    DBObjTableField *areaFLD;
    DBObjTableField *elevationFLD;
    DBObjTableField *percentFLD;
    DBNetworkIF *netIF;
    DBGridIF *gridIF;
    DBObjRecord *cellRec, *tblRec;
    DBObjectLIST<DBObjTableField> *fields;

    itemTable->AddField(basinIDFLD    = new DBObjTableField(DBrNBasin,  DBTableFieldInt,   "%8d",    sizeof(DBInt)));
    itemTable->AddField(areaFLD      = new DBObjTableField("Area",      DBTableFieldFloat, "%10.1f", sizeof(DBFloat4)));
    itemTable->AddField(elevationFLD = new DBObjTableField("Elevation", DBTableFieldFloat, "%8.1f",  sizeof(DBFloat4)));
    itemTable->AddField(percentFLD   = new DBObjTableField("Percent",   DBTableFieldFloat, "%6.2f",  sizeof(DBFloat4)));
    netIF = new DBNetworkIF(netData);
    gridIF = new DBGridIF(grdData);

    for (cellID = 0; cellID < netIF->CellNum(); ++cellID) {
        cellRec = netIF->Cell(cellID);
        if (DBPause(cellID * 33 / netIF->CellNum())) {
            ret = DBFault;
            goto Stop;
        }
        if (gridIF->Value(netIF->Center(cellRec), &elev)) {
            tblRec = itemTable->Add();
            basinIDFLD->Int(tblRec, netIF->CellBasinID(cellRec));
            areaFLD->Float(tblRec, netIF->CellArea(cellRec));
            elevationFLD->Float(tblRec, elev);
        }
    }
    fields = new DBObjectLIST<DBObjTableField>("Field List");
    fields->Add(new DBObjTableField(*basinIDFLD));
    fields->Add(new DBObjTableField(*elevationFLD));
    itemTable->ListSort(fields);
    itemTable->ItemSort();
    delete fields;
    for (tblRec = itemTable->First(); tblRec != (DBObjRecord *) NULL; tblRec = itemTable->Next()) {
        if (DBPause(tblRec->RowID() * 33 / itemTable->ItemNum() + 33)) {
            ret = DBFault;
            goto Stop;
        }
        if (basinID != basinIDFLD->Int(tblRec)) {
            area = 0.0;
            basinID = basinIDFLD->Int(tblRec);
        }
        area += areaFLD->Float(tblRec);
        areaFLD->Float(tblRec, area);
    }
    for (tblRec = itemTable->Last(); tblRec != (DBObjRecord *) NULL; tblRec = itemTable->Next(DBBackward)) {
        if (DBPause((itemTable->ItemNum() - tblRec->RowID()) * 33 / itemTable->ItemNum() + 66)) {
            ret = DBFault;
            goto Stop;
        }
        tblRec->Name(string);
        if (basinID != basinIDFLD->Int(tblRec)) {
            area = areaFLD->Float(tblRec);
            basinID = basinIDFLD->Int(tblRec);
        }
        snprintf(string, sizeof(string), "Basin:%04d Histogram:%06d", basinID, i++);
        percentFLD->Float(tblRec, 100.0 * areaFLD->Float(tblRec) / area);
    }
    Stop:
    delete netIF;
    delete gridIF;
    return (ret);
}

DBInt RGlibNetworkBasinDistrib(DBObjData *netData, DBObjData *grdData, DBObjData *tblData) {
    DBInt layerID, layerNum = 0, cellID, basinID, progress = 0, maxProgress;
    DBInt value;
    DBObjTable *table, *gridTable;
    DBObjTableField *gridValueFLD;
    DBObjTableField *basinIDFLD;
    DBObjTableField *layerIDFLD;
    DBObjTableField *layerNameFLD;
    DBObjTableField *categoryIDFLD;
    DBObjTableField *categoryFLD;
    DBObjTableField *percentFLD;
    DBObjTableField *areaFLD;
    DBObjTableField *cellNumFLD;
    DBNetworkIF *netIF;
    DBGridIF *gridIF;
    DBObjRecord *layerRec, *gridRec, *cellRec, *basinRec, *tblRec;
    DBObjectLIST<DBObjTableField> *fields;
    class Histogram {
    public:
        DBInt cellNum;
        DBFloat area;

        void Initialize() {
            cellNum = 0;
            area = 0.0;
        }
    } *histogram = (Histogram *) NULL;

    gridIF = new DBGridIF(grdData);
    for (layerID = 0; layerID < gridIF->LayerNum(); ++layerID) {
        layerRec = gridIF->Layer(layerID);
        if ((layerRec->Flags() & DBObjectFlagIdle) != DBObjectFlagIdle) ++layerNum;
    }
    if (layerNum < 1) {
        CMmsgPrint(CMmsgUsrError, "No Layer to Process!");
        delete gridIF;
        return (DBFault);
    }

    table = tblData->Table(DBrNItems);
    netIF = new DBNetworkIF(netData);
    table->AddField(basinIDFLD    = new DBObjTableField(DBrNBasin,      DBTableFieldInt,    "%8d",    sizeof(DBInt)));
    table->AddField(layerIDFLD    = new DBObjTableField("LayerID",      DBTableFieldInt,    "%4d",    sizeof(DBShort)));
    table->AddField(layerNameFLD  = new DBObjTableField("LayerName",    DBTableFieldString, "%s",     DBStringLength));
    table->AddField(categoryIDFLD = new DBObjTableField(DBrNCategoryID, DBTableFieldInt,    "%2d",    sizeof(DBShort)));
    table->AddField(categoryFLD   = new DBObjTableField(DBrNCategory,   DBTableFieldString, "%s",     DBStringLength));
    table->AddField(percentFLD    = new DBObjTableField(DBrNPercent,    DBTableFieldFloat,  "%6.2f",  sizeof(DBFloat4)));
    table->AddField(areaFLD       = new DBObjTableField(DBrNArea,       DBTableFieldFloat,  "%10.1f", sizeof(DBFloat4)));
    table->AddField(cellNumFLD    = new DBObjTableField("CellNum",      DBTableFieldInt,    "%8d",    sizeof(DBInt)));

    gridTable    = grdData->Table(DBrNItems);
    gridValueFLD = gridTable->Field(DBrNGridValue);

    if (histogram == (Histogram *) NULL) histogram = (Histogram *) malloc(gridTable->ItemNum() * sizeof(Histogram));
    else histogram = (Histogram *) realloc(histogram, gridTable->ItemNum() * sizeof(Histogram));
    if (histogram == (Histogram *) NULL) {
        CMmsgPrint(CMmsgSysError, "Memory Allocation Error in: %s %d", __FILE__, __LINE__);
        return (DBFault);
    }
    maxProgress = netIF->CellNum() * layerNum;
    for (layerID = 0; layerID < gridIF->LayerNum(); ++layerID) {
        layerRec = gridIF->Layer(layerID);
        if ((layerRec->Flags() & DBObjectFlagIdle) == DBObjectFlagIdle) continue;

        basinID = DBFault;
        if ((layerRec->Flags() & DBObjectFlagIdle) == DBObjectFlagIdle) continue;
        for (cellID = 0; cellID < netIF->CellNum(); ++cellID) {
            cellRec = netIF->Cell(cellID);
            if (DBPause(progress * 100 / maxProgress)) goto Stop;
            progress++;
            if ((cellRec->Flags() & DBObjectFlagIdle) == DBObjectFlagIdle) continue;
            if (basinID != netIF->CellBasinID(cellRec)) {
                if (basinID != DBFault)
                    for (gridRec = gridTable->First(); gridRec != (DBObjRecord *) NULL; gridRec = gridTable->Next())
                        if (histogram[gridRec->RowID()].cellNum > 0) {
                            tblRec = table->Add(basinRec->Name());
                            basinIDFLD->Int(tblRec, basinID);
                            layerIDFLD->Int(tblRec, layerRec->RowID());
                            layerNameFLD->String(tblRec, layerRec->Name());
                            categoryIDFLD->Int(tblRec, gridValueFLD->Int(gridRec));
                            categoryFLD->String(tblRec, gridRec->Name());
                            areaFLD->Float(tblRec, histogram[gridRec->RowID()].area);
                            percentFLD->Float(tblRec,
                                              histogram[gridRec->RowID()].area / netIF->BasinArea(basinRec) * 100.0);
                            cellNumFLD->Int(tblRec, histogram[gridRec->RowID()].cellNum);
                        }
                for (gridRec = gridTable->First(); gridRec != (DBObjRecord *) NULL; gridRec = gridTable->Next())
                    histogram[gridRec->RowID()].Initialize();
                basinID = netIF->CellBasinID(cellRec);
                basinRec = netIF->Basin(cellRec);
            }
            if (gridIF->Value(layerRec, netIF->Center(cellRec), &value) == true) {
                histogram[value].cellNum++;
                histogram[value].area += netIF->CellArea(cellRec);
            }
        }
        for (gridRec = gridTable->First(); gridRec != (DBObjRecord *) NULL; gridRec = gridTable->Next())
            if (histogram[gridRec->RowID()].cellNum > 0) {
                tblRec = table->Add(basinRec->Name());
                basinIDFLD->Int(tblRec, basinID);
                layerIDFLD->Int(tblRec, layerRec->RowID());
                layerNameFLD->String(tblRec, layerRec->Name());
                categoryIDFLD->Int(tblRec, gridValueFLD->Int(gridRec));
                categoryFLD->String(tblRec, gridRec->Name());
                areaFLD->Float(tblRec, histogram[gridRec->RowID()].area);
                percentFLD->Float(tblRec, histogram[gridRec->RowID()].area / netIF->BasinArea(basinRec) * 100.0);
                cellNumFLD->Int(tblRec, histogram[gridRec->RowID()].cellNum);
            }
    }
    Stop:
    fields = new DBObjectLIST<DBObjTableField>("Field List");
    fields->Add(new DBObjTableField(*basinIDFLD));
    fields->Add(new DBObjTableField(*layerIDFLD));
    fields->Add(areaFLD = new DBObjTableField(*areaFLD));
    areaFLD->Flags(DBObjectFlagSortReversed, DBSet);
    table->ListSort(fields);
    delete fields;
    delete netIF;
    delete gridIF;
    return (progress == maxProgress ? DBSuccess : DBFault);
}

DBInt RGlibNetworkPourElevation (DBObjData *netData, const char *elevStr, const char *pourStr) {
    DBInt cellID, cellNum;
    DBObjTable *cellTable = netData->Table(DBrNCells);
    DBObjTableField *elevFLD = cellTable->Field(elevStr);
    DBObjTableField *pourFLD = cellTable->Field(pourStr);
    DBNetworkIF     *netIF   = new DBNetworkIF (netData);
    DBObjRecord *cellRec, *toCell;
    DBFloat elev, pour;

    if (pourFLD == (DBObjTableField *) NULL)
        cellTable->AddField(pourFLD = new DBObjTableField(pourStr, elevFLD->Type(), elevFLD->Format (), elevFLD->Length()));

    cellNum = netIF->CellNum ();
    for (cellID = 0; cellID < cellNum; cellID++) {
        cellRec = netIF->Cell (cellID);
        pourFLD->Float (cellRec, DBHugeVal);
    }
    for (cellID = cellNum; cellID > 0; cellID--) {
        if (DBPause((cellNum - cellID) * 100 / cellNum)) goto Stop;
        cellRec = netIF->Cell (cellID - 1);
        elev = elevFLD->Float (cellRec);
        if (netIF->FromCell( cellRec) == (DBObjRecord *) NULL) {
            pour = elev;
            pourFLD->Float (cellRec,pour);
        }
        else pour = pourFLD->Float (cellRec);
        if (((toCell = netIF->ToCell (cellRec)) != (DBObjRecord *) NULL) &&
            (netIF->CellBasinArea (cellRec) * 1000.0 > netIF->CellBasinArea(toCell)) && // arbitrary ratio
            (pourFLD->Float (toCell) > pour)) {
            pourFLD->Float(toCell, pour);
        }
    }
Stop:
    delete netIF;
    return (cellID == 0 ? DBSuccess : DBFault);
}

static DBInt _RGlibNetworkReservoir(void *, DBObjRecord *);

class VolumeCalc {
private:
    DBObjTableField *ElevFLD;
    DBObjTableField *HeightFLD;
    DBObjTableField *VolFLD;
    DBObjTableField *AreaFLD;
    DBNetworkIF     *NetIF;
    DBFloat LevelVAR, VolumeVAR, AreaVAR;
public:
    VolumeCalc (DBObjData *netData, const char *elevStr, const char *heightStr, const char *volStr, const char *areaStr) {
        DBInt cellID, cellNum;
        DBObjRecord *cellRec;
        DBObjTable *cellTable = netData->Table(DBrNCells);

        ElevFLD   = cellTable->Field(elevStr);
        HeightFLD = cellTable->Field(heightStr);
        VolFLD    = cellTable->Field(volStr);
        AreaFLD   = cellTable->Field(areaStr);
        if (VolFLD == (DBObjTableField *) NULL)
            cellTable->AddField(VolFLD  = new DBObjTableField(volStr,ElevFLD->Type(),ElevFLD->Format (),ElevFLD->Length()));
        if (AreaFLD == (DBObjTableField *) NULL)
            cellTable->AddField(AreaFLD = new DBObjTableField(areaStr,ElevFLD->Type(),ElevFLD->Format (),ElevFLD->Length()));

        NetIF     = new DBNetworkIF (netData);

        cellNum = CellNum ();
        for (cellID = 0; cellID < cellNum; cellID++) {
            VolFLD->Float  (NetIF->Cell (cellID),0.0);
            AreaFLD->Float (NetIF->Cell (cellID),0.0);
        }
    };
    DBInt CellNum () { return (NetIF->CellNum ()); }
    ~VolumeCalc () { delete NetIF; };
    void Action (DBInt cellID) {
        DBObjRecord *cellRec = NetIF->Cell(cellID);
        LevelVAR  = HeightFLD->Float (cellRec);
        VolumeVAR = 0.0;
        AreaVAR   = 0.0;
        NetIF->UpStreamSearch(cellRec, (DBNetworkACTION) _RGlibNetworkReservoir);
        VolFLD->Float  (cellRec, VolumeVAR);
        AreaFLD->Float (cellRec, AreaVAR);
    };
    DBInt Volume (DBObjRecord *cellRec) {
        DBFloat elev, height, cellArea = NetIF->CellArea (cellRec);

        elev       = ElevFLD->Float (cellRec);
        if (elev < LevelVAR)  {
            height = LevelVAR - elev;
            AreaVAR   += cellArea;
            VolumeVAR += cellArea * height * 0.001;
            return (true);
        }
        return (false);
    };
};

static class VolumeCalc *_RGlibNetworkVolumeCalc;

static DBInt _RGlibNetworkReservoir(void *io, DBObjRecord *cellRec) {

    return (_RGlibNetworkVolumeCalc->Volume (cellRec));
}

DBInt RGlibNetworkReservoir (DBObjData *netData, const char *elevStr, const char *heightStr, const char *volStr, const char *areaStr) {
    DBInt cellID, cellNum;

    _RGlibNetworkVolumeCalc = new VolumeCalc (netData, elevStr, heightStr, volStr, areaStr);

    cellNum = _RGlibNetworkVolumeCalc->CellNum ();
    for (cellID = 0; cellID < cellNum; cellID++) {
        if (DBPause((cellNum - cellID) * 100 / cellNum)) goto Stop;
        _RGlibNetworkVolumeCalc->Action (cellID);
    }
    Stop:
    delete _RGlibNetworkVolumeCalc;
    return (cellID == cellNum ? DBSuccess : DBFault);

}

#define RGISNetStreamOrder	"StreamOrder"
#define RGISNetStreamMouthX "MouthXCoord"
#define RGISNetStreamMouthY "MouthYCoord"
#define RGISNetStreamFromX  "FromXCoord"
#define RGISNetStreamFromY  "FromYCoord"
#define RGISNetStreamToX    "ToXCoord"
#define RGISNetStreamToY    "ToYCoord"

class RGlibStreamAction {
private:
    DBObjRecord     *HeadCellRec, *MouthCellRec;
    DBObjTableField *StreamIDFLD;
    DBInt			 StreamID, LineID;
    DBInt            CellOrder;
    DBInt			 Vertex;
    DBInt			 MaxVertex;
    DBCoordinate    *Coordinates;
    DBObjTable      *CellTable;
    DBObjTable      *LineTable;
    DBObjTableField *BasinFLD, *OrderFLD, *MouthXCoordFLD, *MouthYCoordFLD, *FromXCoordFLD, *FromYCoordFLD, *ToXCoordFLD, *ToYCoordFLD;
    DBNetworkIF     *NetIF;
    DBVLineIF       *LineIF;
public:
    RGlibStreamAction (DBObjData *netData, DBObjData *arcData, DBInt minOrder) {
        DBInt cellID;
        DBObjRecord *cellRec;

        Coordinates = (DBCoordinate *) NULL;
        MaxVertex   = 0;
        CellTable = netData->Table (DBrNCells);
        LineTable = arcData->Table (DBrNItems);
        NetIF   = new DBNetworkIF (netData);
        LineIF  = new DBVLineIF   (arcData);

        arcData->Projection (netData->Projection ());
        arcData->Precision  (netData->Precision ());
        arcData->MaxScale   (netData->MaxScale ());
        arcData->MinScale   (netData->MinScale ());

        LineTable->AddField (BasinFLD       = new DBObjTableField (DBrNBasin,           DBTableFieldInt,  "%8d",  sizeof (DBInt)));
        LineTable->AddField (OrderFLD       = new DBObjTableField (RGISNetStreamOrder,  DBTableFieldInt,  "%4d",  sizeof (DBInt)));
        LineTable->AddField (MouthXCoordFLD = new DBObjTableField (RGISNetStreamMouthX, DBTableFieldFloat,"%9.2f",sizeof (DBFloat4)));
        LineTable->AddField (MouthYCoordFLD = new DBObjTableField (RGISNetStreamMouthY, DBTableFieldFloat,"%9.2f",sizeof (DBFloat4)));
        LineTable->AddField (FromXCoordFLD  = new DBObjTableField (RGISNetStreamFromX,  DBTableFieldFloat,"%9.2f",sizeof (DBFloat4)));
        LineTable->AddField (FromYCoordFLD  = new DBObjTableField (RGISNetStreamFromY,  DBTableFieldFloat,"%9.2f",sizeof (DBFloat4)));
        LineTable->AddField (ToXCoordFLD    = new DBObjTableField (RGISNetStreamToX,    DBTableFieldFloat,"%9.2f",sizeof (DBFloat4)));
        LineTable->AddField (ToYCoordFLD    = new DBObjTableField (RGISNetStreamToY,    DBTableFieldFloat,"%9.2f",sizeof (DBFloat4)));
        CellTable->AddField (StreamIDFLD    = new DBObjTableField ("StreamID",DBTableFieldInt,"%8d",sizeof (DBInt)));

        for (cellID = NetIF->CellNum () - 1;cellID >= 0;--cellID) {
            cellRec = NetIF->Cell (cellID);
            StreamIDFLD->Int (cellRec,0);
        }
    }
    ~RGlibStreamAction () {
        if (Coordinates != (DBCoordinate *) NULL) free (Coordinates);
        delete LineIF;
        delete NetIF;
    }
    bool UpStream (DBObjRecord *cellRec) {
        if (StreamIDFLD->Int (cellRec) == StreamID) return (true);
        if ((NetIF->CellOrder(cellRec) != CellOrder) || (StreamIDFLD->Int (cellRec) != 0)) return (false);

        StreamIDFLD->Int (cellRec,StreamID);
        HeadCellRec = cellRec;
        Vertex += 1;
        return (true);
    }
    bool DownStream (DBObjRecord *cellRec) {
        DBObjRecord *toCellRec = NetIF->ToCell (cellRec);
        if (toCellRec == (DBObjRecord *) NULL) return (false);
        Coordinates [Vertex++] = NetIF->Center (toCellRec);
        return ((toCellRec != MouthCellRec));
    }
    DBInt CreateStream (DBObjRecord *cellRec, DBNetworkACTION upStreamAction, DBNetworkACTION downStreamAction) {
        char objName [DBStringLength];
        DBObjRecord *toCellRec = NetIF->ToCell (cellRec), *lineRec;

        snprintf (objName, sizeof(objName), "Line: %5d", ++LineID);
        if ((lineRec = LineIF->NewItem (objName)) == (DBObjRecord *) NULL)
        { CMmsgPrint (CMmsgAppError, "Line Insertion Error in: %s %d",__FILE__,__LINE__); return (CMfailed); }
        BasinFLD->Int (lineRec,NetIF->CellBasinID (cellRec));
        OrderFLD->Int (lineRec,NetIF->CellOrder (cellRec));

        Vertex = 0;
        MouthCellRec = HeadCellRec = cellRec;
        CellOrder = NetIF->CellOrder (cellRec);
        NetIF->UpStreamSearch (MouthCellRec, upStreamAction, (void *) this);
        LineIF->FromNode (lineRec,LineIF->Node (NetIF->Center (HeadCellRec),true));
        LineIF->ToNode   (lineRec,LineIF->Node (NetIF->Center (MouthCellRec) + NetIF->Delta (MouthCellRec),true));
        MouthXCoordFLD->Float (lineRec, NetIF->Center (MouthCellRec).X);
        MouthYCoordFLD->Float (lineRec, NetIF->Center (MouthCellRec).Y);
        FromXCoordFLD->Float  (lineRec, NetIF->Center (HeadCellRec).X);
        FromYCoordFLD->Float  (lineRec, NetIF->Center (HeadCellRec).Y);
        ToXCoordFLD->Float (lineRec, NetIF->Center (MouthCellRec).X + NetIF->Delta (MouthCellRec).X);
        ToYCoordFLD->Float (lineRec, NetIF->Center (MouthCellRec).Y + NetIF->Delta (MouthCellRec).Y);
        if (HeadCellRec == MouthCellRec) LineIF->Vertexes (lineRec,Coordinates,0);
        else {
            if (MaxVertex < Vertex) {
                MaxVertex = Vertex;
                if ((Coordinates = (DBCoordinate *) realloc (Coordinates, MaxVertex * sizeof (DBCoordinate))) == (DBCoordinate *) NULL)
                { CMmsgPrint (CMmsgSysError, "Memory Allocation Error in: %s %d",__FILE__,__LINE__); return (CMfailed); }
            }
            Vertex = 0;
            NetIF->DownStreamSearch (HeadCellRec, downStreamAction, (void *) this);
            LineIF->Vertexes (lineRec,Coordinates,Vertex);
        }
        LineIF->ItemSymbol (lineRec,LineIF->Symbol (0));
        return (CMsucceeded);
    }
    DBInt CreateStreams (DBInt minOrder, DBNetworkACTION upStreamAction, DBNetworkACTION downStreamAction) {
        DBInt cellID;
        DBObjRecord *cellRec, *toCellRec, *lineRec;

        if (LineIF->NewSymbol ("Default Symbol") == (DBObjRecord *) NULL)
        { CMmsgPrint (CMmsgAppError, "Symbol Creation Error in: %s %d",__FILE__,__LINE__); return (CMfailed); }

        StreamID = LineID = 0;
        for (cellID = NetIF->CellNum () - 1; cellID >= 0; --cellID)  {
            if (StreamIDFLD->Int (cellRec = NetIF->Cell (cellID)) != 0) continue;
            if ((CellOrder = NetIF->CellOrder (cellRec)) < minOrder)    continue;
            if ((toCellRec = NetIF->ToCell (cellRec)) == (DBObjRecord *) NULL) {
                StreamIDFLD->Int(cellRec,++StreamID);
                continue;
            }
            if ((CellOrder != NetIF->CellOrder (toCellRec))) {
                StreamIDFLD->Int(cellRec,++StreamID);
                if (StreamIDFLD->Int (cellRec = NetIF->FromCell (toCellRec)) != 0) continue;
                if ((CellOrder = NetIF->CellOrder (cellRec)) < minOrder) continue;
                StreamIDFLD->Int(cellRec,++StreamID);
            }
        }
        for (cellID = NetIF->CellNum () - 1; cellID >= 0; --cellID) {
            if ((StreamID = StreamIDFLD->Int (cellRec = NetIF->Cell (cellID))) == 0) continue;
            if (CreateStream(cellRec, upStreamAction, downStreamAction) == CMfailed) return (CMfailed);
            }
        return (CMsucceeded);
    }
};

static bool _RGlibAnNetworkUpStreamAction (DBNetworkIF *netIF,DBObjRecord *cellRec, void *dataPtr)
{
    RGlibStreamAction *streamAction = (RGlibStreamAction *) dataPtr;

    return (streamAction->UpStream(cellRec));
}

static bool _RGlibAnNetworkDownStreamAction (DBNetworkIF *netIF,DBObjRecord *cellRec, void *dataPtr)
{
    RGlibStreamAction *streamAction = (RGlibStreamAction *) dataPtr;

    return (streamAction->DownStream (cellRec));
}

DBInt RGlibNetworkToStreamlines (DBObjData *netData, DBInt minOrder, DBObjData *arcData) {
    DBInt ret;
    RGlibStreamAction *streamAction = new RGlibStreamAction (netData, arcData, minOrder);

    ret = streamAction->CreateStreams (minOrder, _RGlibAnNetworkUpStreamAction, _RGlibAnNetworkDownStreamAction);

    delete streamAction;
    return (ret);
}
