/******************************************************************************

GHAAS Database library V3.0
Global Hydrological Archive and Analysis System
Copyright 1994-2024, UNH - CCNY

DBGridMisc.cpp

bfekete@ccny.cuny.edu

*******************************************************************************/

#include <DB.hpp>
#include <DBif.hpp>

void DBGridIF::Initialize(DBObjData *data, bool flat) {
    DBObjTableField *rowNumFLD;
    DBObjTableField *colNumFLD;
    DBObjTableField *cellWidthFLD;
    DBObjTableField *cellHeightFLD;
    DBObjTableField *valueTypeFLD;
    DBObjTableField *valueSizeFLD;
    DBObjRecord *layerRec;

    DataPTR    = data;
    ItemTable  = data->Table(DBrNItems);
    switch (data->Type()) {
        case DBTypeGridDiscrete:
            SymbolTable = data->Table(DBrNSymbols);
            AverageFLD = (DBObjTableField *) NULL;
            StdDevFLD = (DBObjTableField *) NULL;
            MinimumFLD = (DBObjTableField *) NULL;
            MaximumFLD = (DBObjTableField *) NULL;
            MissingValueFLD = (DBObjTableField *) NULL;
            GridValueFLD = ItemTable->Field(DBrNGridValue);
            SymbolFLD = ItemTable->Field(DBrNSymbol);
            SymbolIDFLD = SymbolTable->Field(DBrNSymbolID);
            ForegroundFLD = SymbolTable->Field(DBrNForeground);
            BackgroundFLD = SymbolTable->Field(DBrNBackground);
            StyleFLD = SymbolTable->Field(DBrNStyle);
            break;
        case DBTypeGridContinuous:
            SymbolTable = (DBObjTable *) NULL;
            AverageFLD = ItemTable->Field(DBrNAverage);
            StdDevFLD = ItemTable->Field(DBrNStdDev);
            MinimumFLD = ItemTable->Field(DBrNMinimum);
            MaximumFLD = ItemTable->Field(DBrNMaximum);
            MissingValueFLD = ItemTable->Field(DBrNMissingValue);
            SymbolFLD = (DBObjTableField *) NULL;
            SymbolIDFLD = (DBObjTableField *) NULL;
            ForegroundFLD = (DBObjTableField *) NULL;
            BackgroundFLD = (DBObjTableField *) NULL;
            StyleFLD = (DBObjTableField *) NULL;
            break;
        default:
            CMmsgPrint(CMmsgAppError, "Invalid Data Type in: %s %d", __FILE__, __LINE__);
            break;
    }
    LayerTable = data->Table(DBrNLayers);
    LayerFLD = LayerTable->Field(DBrNLayer);
    rowNumFLD = LayerTable->Field(DBrNRowNum);
    colNumFLD = LayerTable->Field(DBrNColNum);
    cellWidthFLD = LayerTable->Field(DBrNCellWidth);
    cellHeightFLD = LayerTable->Field(DBrNCellHeight);
    valueTypeFLD = LayerTable->Field(DBrNValueType);
    valueSizeFLD = LayerTable->Field(DBrNValueSize);

    layerRec = LayerTable->Item();
    DimensionVAR.Row = rowNumFLD->Int(layerRec);
    DimensionVAR.Col = colNumFLD->Int(layerRec);
    CellWidthVAR = cellWidthFLD->Float(layerRec);
    CellHeightVAR = cellHeightFLD->Float(layerRec);
    ValueTypeVAR = valueTypeFLD->Int(layerRec);
    ValueSizeVAR = valueSizeFLD->Int(layerRec);
    Flat = flat;
}

void DBGridIF::RenameLayer(DBObjRecord *layerRec, const char *name) {
    DBObjRecord *dataRec;

    layerRec = LayerTable->Item(layerRec->RowID());
    layerRec->Name(name);
    dataRec = LayerFLD->Record(layerRec);
    dataRec->Name(name);
    if (DataPTR->Type() == DBTypeGridContinuous) {
        DBObjRecord *itemRec = ItemTable->Item(layerRec->RowID());
        itemRec->Name(name);
    }
}

DBObjRecord *DBGridIF::AddLayer(const char *layerName) {
    DBInt index;
    DBObjRecord *firstLayer, *layerRec, *dataRec;
    DBObjTableField *rowNumFLD = LayerTable->Field(DBrNRowNum);
    DBObjTableField *colNumFLD = LayerTable->Field(DBrNColNum);
    DBObjTableField *cellWidthFLD = LayerTable->Field(DBrNCellWidth);
    DBObjTableField *cellHeightFLD = LayerTable->Field(DBrNCellHeight);
    DBObjTableField *valueTypeFLD = LayerTable->Field(DBrNValueType);
    DBObjTableField *valueSizeFLD = LayerTable->Field(DBrNValueSize);

    firstLayer = LayerTable->First(&index);
    LayerTable->Add(layerName);
    if ((layerRec = LayerTable->Item()) == (DBObjRecord *) NULL)
        return ((DBObjRecord *) NULL);
    rowNumFLD->Int(layerRec, rowNumFLD->Int(firstLayer));
    colNumFLD->Int(layerRec, colNumFLD->Int(firstLayer));
    cellWidthFLD->Float(layerRec, cellWidthFLD->Float(firstLayer));
    cellHeightFLD->Float(layerRec, cellHeightFLD->Float(firstLayer));
    valueTypeFLD->Int(layerRec, valueTypeFLD->Int(firstLayer));
    valueSizeFLD->Int(layerRec, valueSizeFLD->Int(firstLayer));

    dataRec = new DBObjRecord(layerName, (size_t) rowNumFLD->Int(firstLayer) * colNumFLD->Int(firstLayer), valueSizeFLD->Int(firstLayer));
    if (dataRec->Data() == (void *) NULL) {
        delete dataRec;
        return ((DBObjRecord *) NULL);
    }
    LayerFLD->Record(layerRec, dataRec);
    ((Data())->Arrays())->Add(dataRec);
    if (DataPTR->Type() == DBTypeGridContinuous) {
        ItemTable->Add(layerName);
        MissingValueFLD->Float(ItemTable->Item(), MissingValueFLD->Float(ItemTable->First(&index)));
    }
    return (layerRec);
}

DBInt DBGridIF::DeleteLayer(char *layerName) {
    DBObjRecord *layerRec, *dataRec;

    if (DataPTR->Type() == DBTypeGridContinuous) {
        if ((layerRec = ItemTable->Item(layerName)) == (DBObjRecord *) NULL) {
            CMmsgPrint(CMmsgAppError, "Invalid layer name in: %s %d", __FILE__, __LINE__);
            return (DBFault);
        }
        ItemTable->Delete(layerRec);
    }
    if ((layerRec = LayerTable->Item(layerName)) == (DBObjRecord *) NULL) {
        CMmsgPrint(CMmsgAppError, "Total Gebasz in: %s %d", __FILE__, __LINE__);
        return (DBFault);
    }
    dataRec = LayerFLD->Record(layerRec);
    ((Data())->Arrays())->Delete(dataRec);
    LayerTable->Delete(layerRec);
    return (DBSuccess);
}

DBInt DBGridIF::DeleteLayers(char *firstLayer, char *lastLayer) {
    DBInt layerID;
    DBObjRecord *layerRec;

    if ((layerRec = LayerTable->Item(firstLayer)) == (DBObjRecord *) NULL) {
        CMmsgPrint(CMmsgAppError, "Invalid layer name in: %s %d", __FILE__, __LINE__);
        return (DBFault);
    }
    layerID = layerRec->RowID();

    while (strcmp(layerRec->Name(), lastLayer) != 0) {
        DeleteLayer(layerRec->Name());
        if ((layerRec = LayerTable->Item(layerID)) == (DBObjRecord *) NULL) break;
    }

    return (DBSuccess);
}

DBFloat DBGridIF::Maximum(DBInt layer) const {
    DBInt index;
    DBFloat retVal;
    DBObjRecord *dataRec;

    if ((layer == DBFault) || ((dataRec = ItemTable->Item(layer)) == (DBObjRecord *) NULL)) {
        retVal = -DBHugeVal;
        for (dataRec = ItemTable->First(&index); dataRec != (DBObjRecord *) NULL; dataRec = ItemTable->Next(&index))
            retVal = retVal > MaximumFLD->Float(dataRec) ? retVal : MaximumFLD->Float(dataRec);
    }
    else retVal = MaximumFLD->Float(dataRec);
    return (retVal);
}

DBFloat DBGridIF::Minimum(DBInt layer) const {
    DBInt index;
    DBFloat retVal;
    DBObjRecord *dataRec;

    if ((layer == DBFault) || ((dataRec = ItemTable->Item(layer)) == (DBObjRecord *) NULL)) {
        retVal = DBHugeVal;
        for (dataRec = ItemTable->First(&index); dataRec != (DBObjRecord *) NULL; dataRec = ItemTable->Next(&index))
            retVal = retVal < MinimumFLD->Float(dataRec) ? retVal : MinimumFLD->Float(dataRec);
    }
    else retVal = MinimumFLD->Float(dataRec);
    return (retVal);
}

DBInt DBGridIF::Coord2Sampler (DBCoordinate coord, DBGridSampler &sampler) const {
    bool horExt, verExt;
    DBCoordinate cellCoord;
    DBPosition   pos, cellPos;
    DBMathDistanceFunction distFunc = DBMathGetDistanceFunction(DataPTR);

    Coord2Pos(coord,   cellPos);
    Pos2Coord(cellPos, cellCoord);
    sampler.Initialize(cellCoord);
    sampler.Add (cellPos, DBMathCoordinateDistance(distFunc, coord, cellCoord));
    if (Flat) return (sampler.Num());

    horExt = fabs (coord.X - cellCoord.X) < (CellWidth()  / 4.0) ? true : false;
    verExt = fabs (coord.Y - cellCoord.Y) < (CellHeight() / 4.0) ? true : false;

    pos.Col = coord.X > cellCoord.X ? cellPos.Col + 1 : cellPos.Col - 1;
    pos.Row = cellPos.Row;
    if (Pos2Coord(pos, cellCoord) == DBSuccess)
        sampler.Add(pos, DBMathCoordinateDistance(distFunc, coord, cellCoord));
    pos.Row = coord.Y > cellCoord.Y ? cellPos.Row + 1 : cellPos.Row - 1;
    if (Pos2Coord(pos, cellCoord) == DBSuccess)
        sampler.Add(pos, DBMathCoordinateDistance(distFunc, coord, cellCoord));
    pos.Col = cellPos.Col;
    if (Pos2Coord(pos, cellCoord) == DBSuccess)
        sampler.Add(pos, DBMathCoordinateDistance(distFunc, coord, cellCoord));

    if (horExt) {
        pos.Col = coord.X > cellCoord.X ? cellPos.Col - 1 : cellPos.Col + 1;
        pos.Row = cellPos.Row;
        if (Pos2Coord(pos, cellCoord) == DBSuccess)
            sampler.Add(pos, DBMathCoordinateDistance(distFunc, coord, cellCoord));
        pos.Row = coord.Y > cellCoord.Y ? cellPos.Row + 1 : cellPos.Row - 1;
        if (Pos2Coord(pos, cellCoord) == DBSuccess)
            sampler.Add(pos, DBMathCoordinateDistance(distFunc, coord, cellCoord));
    }

    if (verExt) {
        pos.Col = cellPos.Col;
        pos.Row = coord.Y > cellCoord.Y ? cellPos.Row + 1 : cellPos.Row - 1;
        if (Pos2Coord(pos, cellCoord) == DBSuccess)
            sampler.Add(pos, DBMathCoordinateDistance(distFunc, coord, cellCoord));
        pos.Col = coord.X > cellCoord.X ? cellPos.Col + 1 : cellPos.Col - 1;
        if (Pos2Coord(pos, cellCoord) == DBSuccess)
            sampler.Add(pos, DBMathCoordinateDistance(distFunc, coord, cellCoord));
    }

    if (horExt && verExt) {
        pos.Col = coord.X > cellCoord.X ? cellPos.Col + 1 : cellPos.Col - 1;
        pos.Row = coord.Y > cellCoord.Y ? cellPos.Row - 1 : cellPos.Row + 1;
        if (Pos2Coord(pos, cellCoord) == DBSuccess)
            sampler.Add(pos, DBMathCoordinateDistance(distFunc, coord, cellCoord));
    }
    return (sampler.Num());
}

DBInt DBGridIF::Coord2Pos(DBCoordinate coord, DBPosition &pos) const {
    pos.Col = (DBInt) (floor((coord.X - DataPTR->Extent().LowerLeft.X) / CellWidth()));
    pos.Row = (DBInt) (floor((coord.Y - DataPTR->Extent().LowerLeft.Y) / CellHeight()));
    if ((pos.Col < 0) || (pos.Col >= ColNum())) return (DBFault);
    if ((pos.Row < 0) || (pos.Row >= RowNum())) return (DBFault);
    return (DBSuccess);
}

DBInt DBGridIF::Pos2Coord(DBPosition pos, DBCoordinate &coord) const {
    if ((pos.Col < 0) || (pos.Row < 0) || (pos.Col >= ColNum()) || (pos.Row >= RowNum())) return (DBFault);

    coord.X = DataPTR->Extent().LowerLeft.X + pos.Col * CellWidth()  + CellWidth()  / 2.0;
    coord.Y = DataPTR->Extent().LowerLeft.Y + pos.Row * CellHeight() + CellHeight() / 2.0;
    return (DBSuccess);
}

DBInt DBGridIF::Value(DBObjRecord *layerRec, DBPosition pos, DBInt value) {
    size_t j;
    DBObjRecord *dataRec = LayerFLD->Record(layerRec);

    if ((pos.Col < 0) || (pos.Row < 0) || (pos.Col >= DimensionVAR.Col) || (pos.Row >= DimensionVAR.Row)) {
        return (false);
    }

    j = (size_t) DimensionVAR.Col * (size_t) (DimensionVAR.Row - pos.Row - 1) + (size_t) pos.Col;
    switch (ValueTypeVAR) {
        case DBTableFieldFloat:
            switch (ValueSizeVAR) {
                case sizeof(DBFloat4):
                    ((DBFloat4 *) (dataRec->Data()))[j] = (DBFloat4) value;
                    break;
                case sizeof(DBFloat):
                    ((DBFloat  *) (dataRec->Data()))[j] = (DBFloat)  value;
                    break;
            }
            break;
        case DBTableFieldInt:
            switch (ValueSizeVAR) {
                case sizeof(DBByte):
                    ((DBByte *) (dataRec->Data()))[j] = (DBByte) value;
                    break;
                case sizeof(DBShort):
                    ((DBShort *) (dataRec->Data()))[j] = (DBShort) value;
                    break;
                case sizeof(DBInt):
                    ((DBInt *) (dataRec->Data()))[j] = (DBInt) value;
                    break;
            }
            break;
    }
    return (true);
}

DBInt DBGridIF::Value(DBObjRecord *layerRec, DBPosition pos, DBInt *value) const {
    size_t j;
    DBObjRecord *dataRec = LayerFLD->Record(layerRec);

    if ((pos.Col < 0) || (pos.Row < 0) || (pos.Col >= DimensionVAR.Col) || (pos.Row >= DimensionVAR.Row)) {
        *value = MissingValue();
        return (false);
    }

    j = (size_t) DimensionVAR.Col * (size_t) (DimensionVAR.Row - pos.Row - 1) + (size_t) pos.Col;
    switch (ValueTypeVAR) {
        case DBTableFieldFloat:
            switch (ValueSizeVAR) {
                case sizeof(DBFloat4):
                    *value = (DBInt) ((DBFloat4 *) (dataRec->Data()))[j];
                    break;
                case sizeof(DBFloat):
                    *value = (DBInt) ((DBFloat *) (dataRec->Data()))[j];
                    break;
            }
            break;
        case DBTableFieldInt:
            switch (ValueSizeVAR) {
                case sizeof(DBByte):
                    *value = (DBInt) ((DBByte *) (dataRec->Data()))[j];
                    break;
                case sizeof(DBShort):
                    *value = (DBInt) ((DBShort *) (dataRec->Data()))[j];
                    break;
                case sizeof(DBInt):
                    *value = (DBInt) ((DBInt *) (dataRec->Data()))[j];
                    break;
            }
            break;
    }
    if (MissingValueFLD != (DBObjTableField *) NULL)
        return (*value == MissingValueFLD->Int(ItemTable->Item(layerRec->RowID())) ? false : true);
    return (*value == DBFault ? false : true);
}

DBInt DBGridIF::Value(DBObjRecord *layerRec, DBPosition pos, DBFloat *value) const {
    DBInt retVal, intVal, missingInt;
    size_t j;
    DBObjRecord *dataRec = LayerFLD->Record(layerRec);
    DBFloat missingFloat;

	if ((pos.Col < 0) || (pos.Row < 0) || (pos.Col >= DimensionVAR.Col) || (pos.Row >= DimensionVAR.Row)) {
        *value = MissingValue();
        return (false);
    }

    j = (size_t) DimensionVAR.Col * (size_t) (DimensionVAR.Row - pos.Row - 1) + (size_t) pos.Col;
    switch (ValueTypeVAR) {
        case DBTableFieldFloat:
            missingFloat = MissingValueFLD->Float(ItemTable->Item(layerRec->RowID()));
            switch (ValueSizeVAR) {
                case sizeof(DBFloat4):
                    *value = (DBFloat) ((DBFloat4 *) (dataRec->Data()))[j];
                    break;
                case sizeof(DBFloat):
                    *value = (DBFloat) ((DBFloat  *) (dataRec->Data()))[j];
                    break;
            }
            retVal = isnan (*value) || CMmathEqualValues(*value, missingFloat) ? false : true;
            break;
        case DBTableFieldInt:
            missingInt = MissingValueFLD->Int(ItemTable->Item(layerRec->RowID()));
            switch (ValueSizeVAR) {
                case sizeof(DBByte):
                    intVal = (DBInt) ((DBByte  *) (dataRec->Data()))[j];
                    break;
                case sizeof(DBShort):
                    intVal = (DBInt) ((DBShort *) (dataRec->Data()))[j];
                    break;
                case sizeof(DBInt):
                    intVal = (DBInt) ((DBInt   *) (dataRec->Data()))[j];
                    break;
            }
            retVal = intVal == missingInt ? false : true;
            *value = (DBFloat) intVal;
            break;
    }
    return (retVal);
}

DBInt DBGridIF::Value(DBObjRecord *layerRec, DBPosition pos, DBFloat value) {
    size_t j;
    DBObjRecord *dataRec = LayerFLD->Record(layerRec);

    if (pos.Col < 0) return (DBFault);
    if (pos.Row < 0) return (DBFault);
    if (pos.Col >= DimensionVAR.Col) return (DBFault);
    if (pos.Row >= DimensionVAR.Row) return (DBFault);
    j = (size_t) DimensionVAR.Col * (size_t) (DimensionVAR.Row - pos.Row - 1) + (size_t) pos.Col;
    switch (ValueTypeVAR) {
        case DBTableFieldFloat:
            switch (ValueSizeVAR) {
                case sizeof(DBFloat4):
                    ((DBFloat4 *) (dataRec->Data()))[j] = (DBFloat4) value;
                    break;
                case sizeof(DBFloat):
                    ((DBFloat *) (dataRec->Data()))[j] = value;
                    break;
            }
            break;
        case DBTableFieldInt:
            switch (ValueSizeVAR) {
                case sizeof(DBByte):
                    ((DBByte *) (dataRec->Data()))[j] = (DBByte) value;
                    break;
                case sizeof(DBShort):
                    ((DBShort *) (dataRec->Data()))[j] = (DBShort) value;
                    break;
                case sizeof(DBInt):
                    ((DBInt *) (dataRec->Data()))[j] = (DBInt) value;
                    break;
            }
            break;
    }
    return (DBSuccess);
}

DBInt DBGridIF::Value(DBObjRecord *layerRec, DBGridSampler sampler, DBFloat *value) const {
    DBInt i, pointNum = sampler.Num();
    DBFloat precision, weight, sumWValue, sumWeight, retVal;

    precision = pow((double) 10.0, (double) DataPTR->Precision());
    sumWValue = sumWeight = 0.0;
    for (i = 0; i < pointNum; ++i) {
        if (Value(layerRec, sampler.Position(i), &retVal)) {
            weight = sampler.Weight(i);
            if (weight < precision) {
                sumWValue = retVal;
                sumWeight = 1.0;
                break;
            }
            weight = 1.0 / weight;
            weight *= weight;
            sumWeight += weight;
            sumWValue += retVal * weight;
        }
    }
    if (sumWeight > 0) { *value = sumWValue / sumWeight; return (true);  }
    else               { *value = MissingValue();        return (false); }
}

DBObjRecord *DBGridIF::GridItem(DBObjRecord *layerRec, DBPosition pos) const {
    DBInt value;

    if (SymbolTable == (DBObjTable *) NULL) return ((DBObjRecord *) NULL);
    if (Value(layerRec, pos, &value) == false) return ((DBObjRecord *) NULL);
    if (value == DBFault) return ((DBObjRecord *) NULL);
    return (ItemTable->Item(value));
}

char *DBGridIF::ValueFormat() const {
    switch (DataPTR->Type()) {
        case DBTypeGridDiscrete:
            return ((char *) "%s");
        case DBTypeGridContinuous:
            return (DBMathFloatAutoFormat(fabs(Maximum()) > fabs(Minimum()) ? Maximum() : Minimum()));
        default:
            return ((char *) NULL);
    }
}

char *DBGridIF::ValueString(DBObjRecord *layerRec, DBPosition pos) {
    static char retString[DBStringLength + 1];

    switch (DataPTR->Type()) {
        case DBTypeGridDiscrete:
            return (GridItem(layerRec, pos))->Name();
        case DBTypeGridContinuous: {
            DBFloat cellVal;
            if (Value(layerRec, pos, &cellVal) == false) return ((char *) "");
            snprintf(retString, sizeof(retString), ValueFormat(), cellVal);
            } break;
        default:
            CMmsgPrint(CMmsgAppError, "Invalid Data Type in: %s %d", __FILE__, __LINE__);
            return ((char *) NULL);
    }
    return (retString);
}

void DBGridIF::RecalcStats(DBObjRecord *layerRec) {
    DBPosition pos;
    DBFloat value, cellArea;
    DBFloat sumWeight = 0.0, minimum = DBHugeVal, maximum = -DBHugeVal, average = 0.0, stdDev = 0.0;

    for (pos.Row = 0; pos.Row < RowNum(); ++pos.Row) {
        pos.Col = 0;
        cellArea = CellArea(pos);
        for (pos.Col = 0; pos.Col < ColNum(); ++pos.Col)
            if (Value(layerRec, pos, &value)) {
                sumWeight += cellArea;
                average = average + value * cellArea;
                minimum = minimum < value ? minimum : value;
                maximum = maximum > value ? maximum : value;
                stdDev = stdDev + value * value * cellArea;
            }
    }
    if (sumWeight > 0.0) {
        average = average / sumWeight;
        stdDev = stdDev / sumWeight;
        stdDev = stdDev - average * average;
        stdDev = stdDev > 0.0 ? sqrt (stdDev) : 0.0;
    }
    else
        average = stdDev = minimum = maximum = MissingValue();
    layerRec = ItemTable->Item(layerRec->RowID());
    AverageFLD->Float(layerRec, average);
    MinimumFLD->Float(layerRec, minimum);
    MaximumFLD->Float(layerRec, maximum);
    StdDevFLD->Float(layerRec, stdDev);
}

void DBGridIF::DiscreteStats() {
    DBInt recordID, layerID;
    DBFloat area, sumArea = 0.0;
    DBPosition pos;
    DBObjTableField *areaFLD = ItemTable->Field(DBrNGridArea);
    DBObjTableField *percentFLD = ItemTable->Field(DBrNGridPercent);
    DBObjRecord *layerRec, *record;

    for (recordID = 0; recordID < ItemTable->ItemNum(); ++recordID) {
        record = ItemTable->Item(recordID);
        areaFLD->Float(record, 0.0);
        percentFLD->Float(record, 0.0);
    }
    for (layerID = 0; layerID < LayerNum(); ++layerID) {
        layerRec = Layer(layerID);
        for (pos.Row = 0; pos.Row < RowNum(); ++pos.Row)
            for (pos.Col = 0; pos.Col < ColNum(); ++pos.Col)
                if ((record = GridItem(layerRec, pos)) != (DBObjRecord *) NULL) {
                    area = CellArea(pos);
                    sumArea += area;
                    areaFLD->Float(record, areaFLD->Float(record) + area);
                }
    }
    for (recordID = 0; recordID < ItemTable->ItemNum(); ++recordID) {
        record = ItemTable->Item(recordID);
        percentFLD->Float(record, areaFLD->Float(record) * 100.0 / sumArea);
    }
}

DBFloat DBGridIF::CellArea(DBPosition pos) const {
    DBCoordinate coord0, coord1;

    coord0.X = DataPTR->Extent().LowerLeft.X + pos.Col * CellWidth();
    coord0.Y = DataPTR->Extent().LowerLeft.Y + pos.Row * CellHeight();
    coord1.X = DataPTR->Extent().LowerLeft.X + pos.Col * CellWidth() + CellWidth();
    coord1.Y = DataPTR->Extent().LowerLeft.Y + pos.Row * CellHeight() + CellHeight();

    return (DBMathRectangleArea(DataPTR->Projection(), coord0, coord1));
}

void DBGridOperation(DBObjData *leftGrd, DBObjData *rightGrd, DBInt oper, DBInt mergeMissingVal) {
    DBInt leftID, rightID;
    DBFloat leftVal, rightVal;
    DBPosition pos;
    DBCoordinate coord;
    DBObjRecord *leftRec, *rightRec;
    DBGridIF *leftIF = new DBGridIF(leftGrd);
    DBGridIF *rightIF = new DBGridIF(rightGrd);

    for (leftID = 0; leftID < leftIF->LayerNum(); ++leftID) {
        leftRec = leftIF->Layer(leftID);
        if ((leftRec->Flags() & DBObjectFlagIdle) != DBObjectFlagIdle) break;
    }
    if (leftID == leftIF->LayerNum()) {
        CMmsgPrint(CMmsgAppError, "No Layer to Process in %s %d", __FILE__, __LINE__);
        return;
    }
    for (rightID = 0; rightID < rightIF->LayerNum(); ++rightID) {
        rightRec = rightIF->Layer(rightID);
        if ((rightRec->Flags() & DBObjectFlagIdle) != DBObjectFlagIdle) break;
    }
    if (rightID == rightIF->LayerNum()) {
        CMmsgPrint(CMmsgAppError, "No Layer to Process in %s %d", __FILE__, __LINE__);
        return;
    }

    rightID = (DBInt) 0;
    for (leftID = 0; leftID < leftIF->LayerNum(); ++leftID) {
        leftRec = leftIF->Layer(leftID);
        while ((leftRec->Flags() & DBObjectFlagIdle) == DBObjectFlagIdle) {
            ++leftID;
            if (leftID == leftIF->LayerNum()) goto Stop;
            leftRec = leftIF->Layer(leftID);
        }
        if ((rightRec = rightIF->Layer(rightID)) == (DBObjRecord *) NULL) {
            rightID = 0;
            rightRec = rightIF->Layer(rightID);
        }
        while ((rightRec->Flags() & DBObjectFlagIdle) == DBObjectFlagIdle) {
            ++rightID;
            if (rightID == rightIF->LayerNum()) rightID = 0;
            rightRec = rightIF->Layer(rightID);
        }

        for (pos.Row = 0; pos.Row < leftIF->RowNum(); pos.Row++) {
            if (DBPause((leftID * leftIF->RowNum() + pos.Row) * 100 / (leftIF->LayerNum() * leftIF->RowNum())))
                goto Stop;
            for (pos.Col = 0; pos.Col < leftIF->ColNum(); pos.Col++) {
                leftIF->Pos2Coord(pos, coord);
                if (leftIF->Value(leftRec, pos, &leftVal)) {
                    if (rightIF->Value(rightRec, coord, &rightVal))
                        switch (oper) {
                            case DBMathOperatorAdd:
                                leftIF->Value(leftRec, pos, leftVal + rightVal);
                                break;
                            case DBMathOperatorSub:
                                leftIF->Value(leftRec, pos, leftVal - rightVal);
                                break;
                            case DBMathOperatorMul:
                                leftIF->Value(leftRec, pos, leftVal * rightVal);
                                break;
                            case DBMathOperatorDiv:
                                if (fabs(rightVal) > 0.000001) leftIF->Value(leftRec, pos, leftVal / rightVal);
                                else leftIF->Value(leftRec, pos, leftIF->MissingValue());
                                break;
                        }
                    else if (mergeMissingVal) leftIF->Value(leftRec, pos, leftIF->MissingValue());
                }
            }
        }
        ++rightID;
        leftIF->RecalcStats(leftRec);
    }
    Stop:
    return;
}

void DBGridOperation(DBObjData *grdData, DBFloat constant, DBInt oper) {
    DBInt layerID;
    DBFloat value;
    DBPosition pos;
    DBObjRecord *layerRec;
    DBGridIF *gridIF = new DBGridIF(grdData);

    for (layerID = 0; layerID < gridIF->LayerNum(); ++layerID) {
        layerRec = gridIF->Layer(layerID);
        if ((layerRec->Flags() & DBObjectFlagIdle) != DBObjectFlagIdle) break;
    }
    if (layerID == gridIF->LayerNum()) {
        CMmsgPrint(CMmsgAppError, "No Layer to Process in %s %d", __FILE__, __LINE__);
        return;
    }

    for (layerID = 0; layerID < gridIF->LayerNum(); ++layerID) {
        layerRec = gridIF->Layer(layerID);
        if ((layerRec->Flags() & DBObjectFlagIdle) == DBObjectFlagIdle) continue;

        for (pos.Row = 0; pos.Row < gridIF->RowNum(); pos.Row++) {
            if (DBPause((layerID * gridIF->RowNum() + pos.Row) * 100 / (gridIF->LayerNum() * gridIF->RowNum())))
                goto Stop;
            for (pos.Col = 0; pos.Col < gridIF->ColNum(); pos.Col++) {
                if (gridIF->Value(layerRec, pos, &value))
                    switch (oper) {
                        case DBMathOperatorAdd:
                            gridIF->Value(layerRec, pos, value + constant);
                            break;
                        case DBMathOperatorSub:
                            gridIF->Value(layerRec, pos, value - constant);
                            break;
                        case DBMathOperatorMul:
                            gridIF->Value(layerRec, pos, value * constant);
                            break;
                        case DBMathOperatorDiv:
                            gridIF->Value(layerRec, pos, value / constant);
                            break;
                    }
            }
        }
        gridIF->RecalcStats(layerRec);
    }
    Stop:
    return;
}

void DBGridOperationAbs(DBObjData *grdData) {
    DBInt layerID;
    DBFloat value;
    DBPosition pos;
    DBObjRecord *layerRec;
    DBGridIF *gridIF = new DBGridIF(grdData);

    for (layerID = 0; layerID < gridIF->LayerNum(); ++layerID) {
        layerRec = gridIF->Layer(layerID);
        if ((layerRec->Flags() & DBObjectFlagIdle) != DBObjectFlagIdle) break;
    }
    if (layerID == gridIF->LayerNum()) {
        CMmsgPrint(CMmsgAppError, "No Layer to Process in %s %d", __FILE__, __LINE__);
        return;
    }

    for (layerID = 0; layerID < gridIF->LayerNum(); ++layerID) {
        layerRec = gridIF->Layer(layerID);
        if ((layerRec->Flags() & DBObjectFlagIdle) == DBObjectFlagIdle) continue;

        for (pos.Row = 0; pos.Row < gridIF->RowNum(); pos.Row++) {
            if (DBPause((layerID * gridIF->RowNum() + pos.Row) * 100 / (gridIF->LayerNum() * gridIF->RowNum())))
                goto Stop;
            for (pos.Col = 0; pos.Col < gridIF->ColNum(); pos.Col++)
                if (gridIF->Value(layerRec, pos, &value)) gridIF->Value(layerRec, pos, fabs(value));
        }
        gridIF->RecalcStats(layerRec);
    }
    Stop:
    return;
}
