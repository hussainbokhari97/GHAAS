/******************************************************************************

GHAAS Database library V3.0
Global Hydrological Archive and Analysis System
Copyright 1994-2024, UNH - CCNY

DBImpExpNetCDF.cpp

bfekete@ccny.cuny.edu

*******************************************************************************/

#include <DB.hpp>
#include <DBif.hpp>
#include <ctype.h>
#include <netcdf.h>
#include <udunits2.h>


static const char *_DBExportNetCDFRename(const char *name) {
    class {
    public:
        char *OldName;
        char *NewName;
    } names[] = {
            {(char *) "CellXCoord",     (char *) "cell_x-coord"},
            {(char *) "CellYCoord",     (char *) "cell_y-coord"},
            {(char *) "Point",          (char *) "point"},
            {(char *) "Line",           (char *) "line"},
            {(char *) "Polygon",        (char *) "polygon"},
            {(char *) "Continuous",     (char *) "continuous"},
            {(char *) "Discrete",       (char *) "discrete"},
            {(char *) "Network",        (char *) "network"},
            {GHAASSubjRunoff,           (char *) "runoff"},
            {GHAASSubjDischarge,        (char *) "discharge"},
            {GHAASSubjFlowHeight,       (char *) "flow_height"},
            {GHAASSubjFlowWidth,        (char *) "flow_width"},
            {GHAASSubjFlowVelocity,     (char *) "flow_velocity"},
            {GHAASSubjFlowSurfSlope,    (char *) "slope"},
            {GHAASSubjRiverStorage,     (char *) "river_storage"},
            {GHAASSubjFloodStorage,     (char *) "flood_storage"},
            {GHAASSubjFloodArea,        (char *) "flood_area"},
            {GHAASSubjOverlandStorage,  (char *) "overland_storage"},
            {GHAASSubjOverlandVelocity, (char *) "overland_velocity"},
            {GHAASSubjPrecip,           (char *) "precipitation"},
            {GHAASSubjAirTemp,          (char *) "air_temperature"},
            {GHAASSubjHumidity,         (char *) "humidity"},
            {GHAASSubjVaporPres,        (char *) "vapor_pressure"},
            {GHAASSubjWindSpeed,        (char *) "wind_speed"},
            {GHAASSubjSolarRad,         (char *) "solar_radiation"},
            {GHAASSubjStations,         (char *) "stations"},
            {GHAASSubjReservoirs,       (char *) "reservoirs"},
            {GHAASSubjElevation,        (char *) "elevation"},
            {GHAASSubjPits,             (char *) "pits"},
            {DBrNCoord,                 (char *) "coord"},
            {DBrNVertexes,              (char *) "vertexes"},
            {DBrNVertexNum,             (char *) "vertex_num"},
            {DBrNRegion,                (char *) "region"},
            {DBrNSymbol,                (char *) "symbol"},
            {DBrNSymbolID,              (char *) "symbol_id"},
            {DBrNForeground,            (char *) "foreground"},
            {DBrNBackground,            (char *) "background"},
            {DBrNStyle,                 (char *) "style"},
            {DBrNLinkNum,               (char *) "link_num"},
            {DBrNLeftPoly,              (char *) "left_polygon"},
            {DBrNRightPoly,             (char *) "right_polygon"},
            {DBrNNextLine,              (char *) "next_line"},
            {DBrNPrevLine,              (char *) "prev_line"},
            {DBrNFromNode,              (char *) "from_node"},
            {DBrNToNode,                (char *) "to_node"},
            {DBrNPerimeter,             (char *) "perimeter"},
            {DBrNFirstLine,             (char *) "first_line"},
            {DBrNLineNum,               (char *) "line_num"},
            {DBrNArea,                  (char *) "area"},
            {DBrNMouthPos,              (char *) "mouth"},
            {DBrNColor,                 (char *) "color"},
            {DBrNBasinOrder,            (char *) "basin_order"},
            {DBrNBasinArea,             (char *) "basin_area"},
            {DBrNBasinLength,           (char *) "basin_length"},
            {DBrNLookupGrid,            (char *) "lookup_grid"},
            {DBrNPosition,              (char *) "position"},
            {DBrNToCell,                (char *) "to_cell"},
            {DBrNFromCell,              (char *) "from_cell"},
            {DBrNOrder,                 (char *) "order"},
            {DBrNMagnitude,             (char *) "magnitude"},
            {DBrNBasin,                 (char *) "basin_id"},
            {DBrNBasinCells,            (char *) "basin_cells"},
            {DBrNTravel,                (char *) "travel"},
            {DBrNUpCellPos,             (char *) "most_upcell_pos"},
            {DBrNCellArea,              (char *) "cell_area"},
            {DBrNCellLength,            (char *) "cell_length"},
            {DBrNSubbasinArea,          (char *) "subbasin_area"},
            {DBrNSubbasinLength,        (char *) "subbasin_length"},
            {DBrNDistToMouth,           (char *) "dist_to_mouth"},
            {DBrNDistToOcean,           (char *) "dist_to_outlet"},
            {DBrNRowNum,                (char *) "row_num"},
            {DBrNColNum,                (char *) "col_num"},
            {DBrNCellWidth,             (char *) "cell_width"},
            {DBrNCellHeight,            (char *) "cell_height"},
            {DBrNValueType,             (char *) "value_type"},
            {DBrNValueSize,             (char *) "value_size"},
            {DBrNLayer,                 (char *) "layer"},
            {DBrNMinimum,               (char *) "minimum"},
            {DBrNMaximum,               (char *) "maximum"},
            {DBrNAverage,               (char *) "average"},
            {DBrNStdDev,                (char *) "stddev"},
            {DBrNMissingValue,          (char *) "missing_value"},
            {DBrNGridValue,             (char *) "grid_value"},
            {DBrNGridArea,              (char *) "grid_area"},
            {DBrNGridPercent,           (char *) "grid_percent"},
            {DBrNItems,                 (char *) "items"},
            {DBrNSelection,             (char *) "selection"},
            {DBrNNodes,                 (char *) "nodes"},
            {DBrNLineList,              (char *) "line_list"},
            {DBrNContours,              (char *) "contours"},
            {DBrNContourList,           (char *) "contour_list"},
            {DBrNSymbols,               (char *) "symbols"},
            {DBrNCells,                 (char *) "cells"},
            {DBrNLayers,                (char *) "layers"},
            {DBrNGroups,                (char *) "groups"},
            {DBrNRelateData,            (char *) "relate_data"},
            {DBrNRelateField,           (char *) "relate_field"},
            {DBrNRelateJoinField,       (char *) "join_field"},
            {DBrNRelations,             (char *) "relations"},
            {DBrNCategoryID,            (char *) "category_id"},
            {DBrNCategory,              (char *) "Category"},
            {DBrNPercent,               (char *) "percent"},
            {DBDocSubject,              (char *) "subject"},
            {DBDocGeoDomain,            (char *) "geo_domain"},
            {DBDocVersion,              (char *) "version"},
            {DBDocCitationRef,          (char *) "reference"},
            {DBDocCitationInst,         (char *) "institute"},
            {DBDocSourceInst,           (char *) "source_institue"},
            {DBDocSourcePerson,         (char *) "source_person"},
            {DBDocOwnerPerson,          (char *) "contact_person"},
            {DBDocComment,              (char *) "comment"}};
    size_t i;

    for (i = 0; i < sizeof(names) / sizeof(names[0]); ++i)
        if (strcmp(names[i].OldName, name) == 0) return (names[i].NewName);
    return (name);
}

enum {
    DIMTime, DIMLat, DIMLon,
};

static DBInt _DBExportNetCDFPoint(DBObjData *dbData, int ncid) {
    const char *str, *varname;
    int status, latid, lonid, dimid;
    size_t start, count;
    double extent[2];
    DBInt pntID;
    DBCoordinate coord;
    DBObjTable *table = dbData->Table(DBrNItems);
    DBVPointIF *pntIF;

    if ((status = nc_redef(ncid)) != NC_NOERR) {
        CMmsgPrint(CMmsgAppError, "NC Error '%s' in: %s %d", nc_strerror(status), __FILE__, __LINE__);
        return (DBFault);
    }
    if ((status = nc_def_dim(ncid, _DBExportNetCDFRename(DBrNItems), table->ItemNum(), &dimid)) != NC_NOERR) {
        CMmsgPrint(CMmsgAppError, "NC Error '%s' in: %s %d", nc_strerror(status), __FILE__, __LINE__);
        return (DBFault);
    }

    /* Begin Defining Latitude Variable */
    varname = dbData->Projection() == DBProjectionSpherical ? "latitude" : "y-coord";
    if ((status = nc_def_var(ncid, varname, NC_DOUBLE, (int) 1, &dimid, &latid)) != NC_NOERR) {
        CMmsgPrint(CMmsgAppError, "NC Error '%s' in: %s %d", nc_strerror(status), __FILE__, __LINE__);
        return (DBFault);
    }
    str = "Latitude";
    if ((status = nc_put_att_text(ncid, latid, "long_name", strlen(str), str)) != NC_NOERR) {
        CMmsgPrint(CMmsgAppError, "NC Error '%s' in: %s %d", nc_strerror(status), __FILE__, __LINE__);
        return (DBFault);
    }
    str = "latitude_north";
    if ((status = nc_put_att_text(ncid, latid, "standard_name", strlen(str), str)) != NC_NOERR) {
        CMmsgPrint(CMmsgAppError, "NC Error '%s' in: %s %d", nc_strerror(status), __FILE__, __LINE__);
        return (DBFault);
    }
    str = "degrees_north";
    if ((status = nc_put_att_text(ncid, latid, "units", strlen(str), str)) != NC_NOERR) {
        CMmsgPrint(CMmsgAppError, "NC Error '%s' in: %s %d", nc_strerror(status), __FILE__, __LINE__);
        return (DBFault);
    }
    extent[0] = -90.0;
    extent[1] = 90.0;
    if ((status = nc_put_att_double(ncid, latid, "valid_range", NC_DOUBLE, 2, extent)) != NC_NOERR) {
        CMmsgPrint(CMmsgAppError, "NC Error '%s' in: %s %d", nc_strerror(status), __FILE__, __LINE__);
        return (DBFault);
    }
    extent[0] = (dbData->Extent()).LowerLeft.Y;
    extent[1] = (dbData->Extent()).UpperRight.Y;
    if ((status = nc_put_att_double(ncid, latid, "actual_range", NC_DOUBLE, 2, extent)) != NC_NOERR) {
        CMmsgPrint(CMmsgAppError, "NC Error '%s' in: %s %d", nc_strerror(status), __FILE__, __LINE__);
        return (DBFault);
    }
    str = "y";
    if ((status = nc_put_att_text(ncid, latid, "axis", strlen(str), str)) != NC_NOERR) {
        CMmsgPrint(CMmsgAppError, "NC Error '%s' in %s %d", nc_strerror(status), __FILE__, __LINE__);
        return (DBFault);
    }
    /* End Defining Latitude Variable */

    /* Begin Defining Longitude Variable */
    varname = dbData->Projection() == DBProjectionSpherical ? "longitude" : "x-coord";
    if ((status = nc_def_var(ncid, "longitude", NC_DOUBLE, (int) 1, &dimid, &lonid)) != NC_NOERR) {
        CMmsgPrint(CMmsgAppError, "NC Error '%s' in %s %d", nc_strerror(status), __FILE__, __LINE__);
        return (DBFault);
    }
    str = "Longitude";
    if ((status = nc_put_att_text(ncid, lonid, "long_name", strlen(str), str)) != NC_NOERR) {
        CMmsgPrint(CMmsgAppError, "NC Error '%s' in: %s %d", nc_strerror(status), __FILE__, __LINE__);
        return (DBFault);
    }
    str = "longitude_east";
    if ((status = nc_put_att_text(ncid, lonid, "standard_name", strlen(str), str)) != NC_NOERR) {
        CMmsgPrint(CMmsgAppError, "NC Error '%s' in: %s %d", nc_strerror(status), __FILE__, __LINE__);
        return (DBFault);
    }
    str = "degrees_east";
    if ((status = nc_put_att_text(ncid, lonid, "units", strlen(str), str)) != NC_NOERR) {
        CMmsgPrint(CMmsgAppError, "NC Error '%s' in: %s %d", nc_strerror(status), __FILE__, __LINE__);
        return (DBFault);
    }
    extent[0] = -180.0;
    extent[1] = 180.0;
    if ((status = nc_put_att_double(ncid, lonid, "valid_range", NC_DOUBLE, 2, extent)) != NC_NOERR) {
        CMmsgPrint(CMmsgAppError, "NC Error '%s' in: %s %d", nc_strerror(status), __FILE__, __LINE__);
        return (DBFault);
    }
    extent[0] = (dbData->Extent()).LowerLeft.X;
    extent[1] = (dbData->Extent()).UpperRight.X;
    if ((status = nc_put_att_double(ncid, lonid, "actual_range", NC_DOUBLE, 2, extent)) != NC_NOERR) {
        CMmsgPrint(CMmsgAppError, "NC Error '%s' in: %s %d", nc_strerror(status), __FILE__, __LINE__);
        return (DBFault);
    }
    str = "x";
    if ((status = nc_put_att_text(ncid, lonid, "axis", strlen(str), str)) != NC_NOERR) {
        CMmsgPrint(CMmsgAppError, "NC Error '%s' in: %s %d", nc_strerror(status), __FILE__, __LINE__);
        return (DBFault);
    }
    /* End Defining Longitude Variable */

    if ((status = nc_enddef(ncid)) != NC_NOERR) {
        CMmsgPrint(CMmsgAppError, "NC Error '%s' in: %s %d", nc_strerror(status), __FILE__, __LINE__);
        return (DBFault);
    }

    pntIF = new DBVPointIF(dbData);
    for (pntID = 0; pntID < pntIF->ItemNum(); pntID++) {
        coord = pntIF->Coordinate(pntIF->Item(pntID));
        start = pntID;
        count = 1;
        if ((status = nc_put_vara_double(ncid, latid, &start, &count, &coord.X)) != NC_NOERR) {
            CMmsgPrint(CMmsgAppError, "NC Error '%s' in: %s %d", nc_strerror(status), __FILE__, __LINE__);
            delete pntIF;
            return (DBFault);
        }
        if ((status = nc_put_vara_double(ncid, lonid, &start, &count, &coord.Y)) != NC_NOERR) {
            CMmsgPrint(CMmsgAppError, "NC Error '%s' in: %s %d", nc_strerror(status), __FILE__, __LINE__);
            delete pntIF;
            return (DBFault);
        }
    }

    delete pntIF;
    return (DBSuccess);
}

static DBInt _DBExportNetCDFGridDefine(DBObjData *dbData, int ncid, int dimids[]) {
    char *str;
    int rowNum, colNum, i;
    int status, latid, lonid, latbndid, lonbndid, bdimids[2];
    size_t start[2], count[2];
    double extent[2], cellWidth, cellHeight, *record;

    switch (dbData->Type()) {
        case DBTypeGridDiscrete:
        case DBTypeGridContinuous: {
            DBGridIF *gridIF = new DBGridIF(dbData);

            rowNum = gridIF->RowNum();
            colNum = gridIF->ColNum();
            cellWidth = gridIF->CellWidth();
            cellHeight = gridIF->CellHeight();
            delete gridIF;
        }
            break;
        case DBTypeNetwork: {
            DBNetworkIF *netIF = new DBNetworkIF(dbData);
            rowNum = netIF->RowNum();
            colNum = netIF->ColNum();
            cellWidth = netIF->CellWidth();
            cellHeight = netIF->CellHeight();
            delete netIF;
        }
            break;
        default:
            CMmsgPrint(CMmsgAppError, "Invalid data type in: %s %d", __FILE__, __LINE__);
            return (DBFault);
    }

    if ((status = nc_redef(ncid)) != NC_NOERR) {
        CMmsgPrint(CMmsgAppError, "NC Error '%s' in: %s %d", nc_strerror(status), __FILE__, __LINE__);
        return (DBFault);
    }
    /* Begin Defining Dimensions */
    if ((status = nc_def_dim(ncid, "latitude", rowNum, dimids + DIMLat)) != NC_NOERR) {
        CMmsgPrint(CMmsgAppError, "NC Error '%s' in: %s %d", nc_strerror(status), __FILE__, __LINE__);
        return (DBFault);
    }
    if ((status = nc_def_dim(ncid, "longitude", colNum, dimids + DIMLon)) != NC_NOERR) {
        CMmsgPrint(CMmsgAppError, "NC Error '%s' in: %s %d", nc_strerror(status), __FILE__, __LINE__);
        return (DBFault);
    }
    if ((status = nc_def_dim(ncid, "bnds", 2, bdimids + 1)) != NC_NOERR) {
        CMmsgPrint(CMmsgAppError, "NC Error '%s' in: %s %d", nc_strerror(status), __FILE__, __LINE__);
        return (DBFault);
    }
    /* End Defining Dimensions */

    /* Begin Defining Latitude Variable */
    if ((status = nc_def_var(ncid, "latitude", NC_DOUBLE, (int) 1, dimids + DIMLat, &latid)) != NC_NOERR) {
        CMmsgPrint(CMmsgAppError, "NC Error '%s' in: %s %d", nc_strerror(status), __FILE__, __LINE__);
        nc_close(ncid);
        return (DBFault);
    }
    str = (char *) "Latitude";
    if ((status = nc_put_att_text(ncid, latid, "long_name", strlen(str), str)) != NC_NOERR) {
        CMmsgPrint(CMmsgAppError, "NC Error '%s' in: %s %d", nc_strerror(status), __FILE__, __LINE__);
        nc_close(ncid);
        return (DBFault);
    }
    str = (char *) "latitude_north";
    if ((status = nc_put_att_text(ncid, latid, "standard_name", strlen(str), str)) != NC_NOERR) {
        CMmsgPrint(CMmsgAppError, "NC Error '%s' in: %s %d", nc_strerror(status), __FILE__, __LINE__);
        nc_close(ncid);
        return (DBFault);
    }
    str = (char *) "degrees_north";
    if ((status = nc_put_att_text(ncid, latid, "units", strlen(str), str)) != NC_NOERR) {
        CMmsgPrint(CMmsgAppError, "NC Error '%s' in: %s %d", nc_strerror(status), __FILE__, __LINE__);
        nc_close(ncid);
        return (DBFault);
    }
    extent[0] = -90.0;
    extent[1] = 90.0;
    if ((status = nc_put_att_double(ncid, latid, "valid_range", NC_DOUBLE, 2, extent)) != NC_NOERR) {
        CMmsgPrint(CMmsgAppError, "NC Error '%s' in: %s %d", nc_strerror(status), __FILE__, __LINE__);
        nc_close(ncid);
        return (DBFault);
    }
    extent[0] = (dbData->Extent()).LowerLeft.Y;
    extent[1] = (dbData->Extent()).UpperRight.Y;
    if ((status = nc_put_att_double(ncid, latid, "actual_range", NC_DOUBLE, 2, extent)) != NC_NOERR) {
        CMmsgPrint(CMmsgAppError, "NC Error '%s' in: %s %d", nc_strerror(status), __FILE__, __LINE__);
        nc_close(ncid);
        return (DBFault);
    }
    str = (char *) "latitude_bnds";
    if ((status = nc_put_att_text(ncid, latid, "bounds", strlen(str), str)) != NC_NOERR) {
        CMmsgPrint(CMmsgAppError, "NC Error '%s' in: %s %d", nc_strerror(status), __FILE__, __LINE__);
        nc_close(ncid);
        return (DBFault);
    }
    str = (char *) "y";
    if ((status = nc_put_att_text(ncid, latid, "axis", strlen(str), str)) != NC_NOERR) {
        CMmsgPrint(CMmsgAppError, "NC Error '%s' in: %s %d", nc_strerror(status), __FILE__, __LINE__);
        nc_close(ncid);
        return (DBFault);
    }
    /* End Defining Latitude Variable */

    /* Begin Defining Longitude Variable */
    if ((status = nc_def_var(ncid, "longitude", NC_DOUBLE, (int) 1, dimids + DIMLon, &lonid)) != NC_NOERR) {
        CMmsgPrint(CMmsgAppError, "NC Error '%s' in: %s %d", nc_strerror(status), __FILE__, __LINE__);
        nc_close(ncid);
        return (DBFault);
    }
    str = (char *) "Longitude";
    if ((status = nc_put_att_text(ncid, lonid, "long_name", strlen(str), str)) != NC_NOERR) {
        CMmsgPrint(CMmsgAppError, "NC Error '%s' in: %s %d", nc_strerror(status), __FILE__, __LINE__);
        nc_close(ncid);
        return (DBFault);
    }
    str = (char *) "longitude_east";
    if ((status = nc_put_att_text(ncid, lonid, "standard_name", strlen(str), str)) != NC_NOERR) {
        CMmsgPrint(CMmsgAppError, "NC Error '%s' in: %s %d", nc_strerror(status), __FILE__, __LINE__);
        nc_close(ncid);
        return (DBFault);
    }
    str = (char *) "degrees_east";
    if ((status = nc_put_att_text(ncid, lonid, "units", strlen(str), str)) != NC_NOERR) {
        CMmsgPrint(CMmsgAppError, "NC Error '%s' in: %s %d", nc_strerror(status), __FILE__, __LINE__);
        nc_close(ncid);
        return (DBFault);
    }
    extent[0] = -180.0;
    extent[1] = 180.0;
    if ((status = nc_put_att_double(ncid, lonid, "valid_range", NC_DOUBLE, 2, extent)) != NC_NOERR) {
        CMmsgPrint(CMmsgAppError, "NC Error '%s' in: %s %d", nc_strerror(status), __FILE__, __LINE__);
        nc_close(ncid);
        return (DBFault);
    }
    extent[0] = (dbData->Extent()).LowerLeft.X;
    extent[1] = (dbData->Extent()).UpperRight.X;
    if ((status = nc_put_att_double(ncid, lonid, "actual_range", NC_DOUBLE, 2, extent)) != NC_NOERR) {
        CMmsgPrint(CMmsgAppError, "NC Error '%s' in: %s %d", nc_strerror(status), __FILE__, __LINE__);
        nc_close(ncid);
        return (DBFault);
    }
    str = (char *) "longitude_bnds";
    if ((status = nc_put_att_text(ncid, lonid, "bounds", strlen(str), str)) != NC_NOERR) {
        CMmsgPrint(CMmsgAppError, "NC Error '%s' in: %s %d", nc_strerror(status), __FILE__, __LINE__);
        nc_close(ncid);
        return (DBFault);
    }
    str = (char *) "x";
    if ((status = nc_put_att_text(ncid, lonid, "axis", strlen(str), str)) != NC_NOERR) {
        CMmsgPrint(CMmsgAppError, "NC Error '%s' in: %s %d", nc_strerror(status), __FILE__, __LINE__);
        nc_close(ncid);
        return (DBFault);
    }
    /* End Defining Longitude Variable */

    /* Begin Defining Latitude_bounds Variable */
    bdimids[0] = dimids[DIMLat];
    if ((status = nc_def_var(ncid, "latitude_bnds", NC_DOUBLE, (int) 2, bdimids, &latbndid)) != NC_NOERR) {
        CMmsgPrint(CMmsgAppError, "NC Error '%s' in: %s %d", nc_strerror(status), __FILE__, __LINE__);
        nc_close(ncid);
        return (DBFault);
    }
    /* End Defining Latitude_bounds Variable */

    /* Begin Defining Longitude_bounds Variable */
    bdimids[0] = dimids[DIMLon];
    if ((status = nc_def_var(ncid, "longitude_bnds", NC_DOUBLE, (int) 2, bdimids, &lonbndid)) != NC_NOERR) {
        CMmsgPrint(CMmsgAppError, "NC Error '%s' in: %s %d", nc_strerror(status), __FILE__, __LINE__);
        nc_close(ncid);
        return (DBFault);
    }
    /* End Defining Longitude_bounds Variable */

    if ((status = nc_enddef(ncid)) != NC_NOERR) {
        CMmsgPrint(CMmsgAppError, "NC Error '%s' in: %s %d", nc_strerror(status), __FILE__, __LINE__);
        nc_close(ncid);
        return (DBFault);
    }

    if ((record = (double *) calloc((rowNum > colNum ? rowNum : colNum) * 2, sizeof(double))) == (double *) NULL) {
        CMmsgPrint(CMmsgSysError, "Memory allocation error in: %s %d", __FILE__, __LINE__);
        return (DBFault);
    }

    start[0] = start[1] = 0;
    count[0] = rowNum;
    count[1] = 2;
    for (i = 0; i < rowNum; i++) record[i] = (dbData->Extent()).LowerLeft.Y + i * cellHeight + cellHeight / 2.0;
    if ((status = nc_put_vara_double(ncid, latid, start, count, record)) != NC_NOERR) {
        CMmsgPrint(CMmsgAppError, "NC Error '%s' in: %s %d", nc_strerror(status), __FILE__, __LINE__);
        free(record);
        return (DBFault);
    }
    for (i = 0; i < rowNum; i++) {
        record[i * 2] = (dbData->Extent()).LowerLeft.Y + i * cellHeight;
        record[i * 2 + 1] = (dbData->Extent()).LowerLeft.Y + (i + 1) * cellHeight;
    }
    if ((status = nc_put_vara_double(ncid, latbndid, start, count, record)) != NC_NOERR) {
        CMmsgPrint(CMmsgAppError, "NC Error '%s' in: %s %d", nc_strerror(status), __FILE__, __LINE__);
        free(record);
        return (DBFault);
    }

    count[0] = colNum;
    for (i = 0; i < colNum; i++) record[i] = (dbData->Extent()).LowerLeft.X + i * cellWidth + cellWidth / 2.0;
    if ((status = nc_put_vara_double(ncid, lonid, start, count, record)) != NC_NOERR) {
        CMmsgPrint(CMmsgAppError, "NC Error '%s' in: %s %d", nc_strerror(status), __FILE__, __LINE__);
        free(record);
        return (DBFault);
    }
    for (i = 0; i < colNum; i++) {
        record[i * 2] = (dbData->Extent()).LowerLeft.X + i * cellWidth;
        record[i * 2 + 1] = (dbData->Extent()).LowerLeft.X + (i + 1) * cellWidth;
    }
    if ((status = nc_put_vara_double(ncid, lonbndid, start, count, record)) != NC_NOERR) {
        CMmsgPrint(CMmsgAppError, "NC Error '%s' in: %s %d", nc_strerror(status), __FILE__, __LINE__);
        free(record);
        return (DBFault);
    }
    free(record);
    return (DBSuccess);
}

static DBInt _DBExportNetCDFTimeDefine(DBObjData *dbData, int ncid, int dimids[]) {

    char *str, timeStr[DBStringLength], unitStr[NC_MAX_NAME];
    int status, timeid, i;
    int year, month, day, hour, minute;
    ut_system *utSystem = (ut_system *) NULL;
    ut_set_error_message_handler(ut_ignore);
    ut_unit *utUnit = (ut_unit *) NULL;
    ut_unit *baseTimeUnit = (ut_unit *) NULL;
    cv_converter *cvConverter = (cv_converter *) NULL;
    double val;
    size_t start, count;
    int *record, extent[2];
    DBInt layerID;
    DBGridIF *gridIF = new DBGridIF(dbData);

    if ((utSystem = ut_read_xml((char *) NULL)) == (ut_system *) NULL) {
        CMmsgPrint(CMmsgAppError, "Total metal gebasz in: %s %d", __FILE__, __LINE__);
        return (DBFault);
    }
    if ((baseTimeUnit = ut_parse(utSystem, "seconds since 2001-01-01 00:00:00", UT_ASCII)) == (ut_unit *) NULL) {
        CMmsgPrint(CMmsgAppError, "Total metal gebasz in: %s %d", __FILE__, __LINE__);
        switch (ut_get_status()) {
            case UT_BAD_ARG:
                CMmsgPrint(CMmsgUsrError, "System or string is NULL!");
                break;
            case UT_SYNTAX:
                CMmsgPrint(CMmsgUsrError, "String contained a syntax error!");
                break;
            case UT_UNKNOWN:
                CMmsgPrint(CMmsgUsrError, "String contained an unknown identifier!");
                break;
            default:
                CMmsgPrint(CMmsgAppError, "System error in %s:%d!n", __FILE__, __LINE__);
        }
        ut_free_system(utSystem);
        return (DBFault);
    }
    /* Begin Defining Dimensions */
    if ((status = nc_redef(ncid)) != NC_NOERR) {
        CMmsgPrint(CMmsgAppError, "NC Error '%s' in: %s %d", nc_strerror(status), __FILE__, __LINE__);
        ut_free(baseTimeUnit);
        ut_free_system(utSystem);
        delete gridIF;
        return (DBFault);
    }
    if ((status = nc_def_dim(ncid, "time", NC_UNLIMITED, dimids + DIMTime)) != NC_NOERR) {
        CMmsgPrint(CMmsgAppError, "NC Error '%s' in: %s %d", nc_strerror(status), __FILE__, __LINE__);
        ut_free(baseTimeUnit);
        ut_free_system(utSystem);
        delete gridIF;
        return (DBFault);
    }
    /* End Defining Dimensions */

    /* Begin Defining Time Variable */
    if ((status = nc_def_var(ncid, "time", NC_INT, (int) 1, dimids + DIMTime, &timeid)) != NC_NOERR) {
        CMmsgPrint(CMmsgAppError, "NC Error '%s' in: %s %d", nc_strerror(status), __FILE__, __LINE__);
        ut_free(baseTimeUnit);
        ut_free_system(utSystem);
        delete gridIF;
        return (DBFault);
    }
    str = (char *) "Time";
    if ((status = nc_put_att_text(ncid, timeid, "long_name", strlen(str), str)) != NC_NOERR) {
        CMmsgPrint(CMmsgAppError, "NC Error '%s' in: %s %d", nc_strerror(status), __FILE__, __LINE__);
        ut_free(baseTimeUnit);
        ut_free_system(utSystem);
        delete gridIF;
        return (DBFault);
    }
    str = (char *) "time";
    if ((status = nc_put_att_text(ncid, timeid, "standard_name", strlen(str), str)) != NC_NOERR) {
        CMmsgPrint(CMmsgAppError, "NC Error '%s' in: %s %d", nc_strerror(status), __FILE__, __LINE__);
        ut_free(baseTimeUnit);
        ut_free_system(utSystem);
        delete gridIF;
        return (DBFault);
    }
    strcpy(timeStr, (gridIF->Layer(gridIF->LayerNum() - 1))->Name());
    if (strncmp(timeStr, "XXXX", 4) == 0) for (i = 0; i < 4; i++) timeStr[i] = '0';
    month = 6;
    day = 15;
    hour = 12;
    minute = 30;
    switch (strlen(timeStr)) {
        case  4:
            strcpy(timeStr, (gridIF->Layer(0))->Name());
            if (strncmp(timeStr, "XXXX", 4) == 0) for (i = 0; i < 4; i++) timeStr[i] = '0';
            sscanf(timeStr, "%4d", &year);
            snprintf(unitStr, NC_MAX_NAME, "years since %s-01-01 00:00", timeStr);
            break;
        case  7:
            strcpy(timeStr, (gridIF->Layer(0))->Name());
            if (strncmp(timeStr, "XXXX", 4) == 0) for (i = 0; i < 4; i++) timeStr[i] = '0';
            sscanf(timeStr, "%4d-%2d", &year, &month);
            snprintf(unitStr, NC_MAX_NAME, "months since %s-01 00:00", timeStr);
            break;
        case 10:
            strcpy(timeStr, (gridIF->Layer(0))->Name());
            if (strncmp(timeStr, "XXXX", 4) == 0) for (i = 0; i < 4; i++) timeStr[i] = '0';
            sscanf(timeStr, "%4d-%2d-%2d", &year, &month, &day);
            snprintf(unitStr, sizeof(unitStr), "days since %s 00:00", timeStr);
            break;
        case 13:
            strcpy(timeStr, (gridIF->Layer(0))->Name());
            if (strncmp(timeStr, "XXXX", 4) == 0) for (i = 0; i < 4; i++) timeStr[i] = '0';
            sscanf(timeStr, "%4d-%2d-%2d %2d", &year, &month, &day, &hour);
            snprintf(unitStr, sizeof(unitStr), "hours since %s:00", timeStr);
            break;
        case 16:
            strcpy(timeStr, (gridIF->Layer(0))->Name());
            if (strncmp(timeStr, "XXXX", 4) == 0) for (i = 0; i < 4; i++) timeStr[i] = '0';
            sscanf(timeStr, "%4d-%2d-%2d %2d:%2d", &year, &month, &day, &hour, &minute);
            snprintf(unitStr, sizeof(unitStr), "minutes since %s", timeStr);
            break;
        default:
            unitStr[0] = '\0';
            break;
    }
    if (strlen(unitStr) > 0) {
        if ((utUnit = ut_parse(utSystem, unitStr, UT_ASCII)) == (ut_unit *) NULL) {
            CMmsgPrint(CMmsgAppError, "Invalid time Unit [%s] in: %s %d", unitStr, __FILE__, __LINE__);
            ut_free(baseTimeUnit);
            ut_free_system(utSystem);
            delete gridIF;
            return (DBFault);
        }
        if ((cvConverter = ut_get_converter(baseTimeUnit, utUnit)) == (cv_converter *) NULL) {
            CMmsgPrint(CMmsgUsrError, "Time converter error!n");
            switch (ut_get_status()) {
                case UT_BAD_ARG:
                    CMmsgPrint(CMmsgUsrError, "unit1 or unit2 is NULL.");
                    break;
                case UT_NOT_SAME_SYSTEM:
                    CMmsgPrint(CMmsgUsrError, "unit1 and unit2 belong to different unit-systems.");
                    break;
                default:
                    CMmsgPrint(CMmsgUsrError, "Conversion between the units is not possible.");
                    break;
            }
            ut_free(utUnit);
            ut_free(baseTimeUnit);
            ut_free_system(utSystem);
            delete gridIF;
            return (DBFault);
        }
        if ((status = nc_put_att_text(ncid, timeid, "units", strlen(unitStr), unitStr)) != NC_NOERR) {
            CMmsgPrint(CMmsgAppError, "NC Error '%s' in: %s %d", nc_strerror(status), __FILE__, __LINE__);
            ut_free_system(utSystem);
            ut_free(utUnit);
            ut_free(baseTimeUnit);
            cv_free(cvConverter);
            delete gridIF;
            return (DBFault);
        }
        str = (char *) "t";
        if ((status = nc_put_att_text(ncid, timeid, "axis", strlen(str), str)) != NC_NOERR) {
            CMmsgPrint(CMmsgAppError, "NC Error '%s' in: %s %d", nc_strerror(status), __FILE__, __LINE__);
            ut_free_system(utSystem);
            ut_free(utUnit);
            ut_free(baseTimeUnit);
            cv_free(cvConverter);
            delete gridIF;
            return (DBFault);
        }
    } /* End Defining Time Variable */

    if ((status = nc_enddef(ncid)) != NC_NOERR) {
        CMmsgPrint(CMmsgAppError, "NC Error '%s' in: %s %d", nc_strerror(status), __FILE__, __LINE__);
        ut_free_system(utSystem);
        ut_free(utUnit);
        ut_free(utUnit);
        ut_free(baseTimeUnit);
        cv_free(cvConverter);
        delete gridIF;
        return (DBFault);
    }

    if ((record = (int *) calloc(gridIF->LayerNum() * 2, sizeof(int))) == (int *) NULL) {
        CMmsgPrint(CMmsgSysError, "Memory allocation error in: %s %d", __FILE__, __LINE__);
        ut_free_system(utSystem);
        ut_free(utUnit);
        ut_free(baseTimeUnit);
        cv_free(cvConverter);
        delete gridIF;
        return (DBFault);
    }
    start = 0;
    count = gridIF->LayerNum();;
    month = 6;
    day = 15;
    hour = 12;
    minute = 30;
    for (layerID = 0; layerID < gridIF->LayerNum(); layerID++) {
        strcpy(timeStr, (gridIF->Layer(layerID))->Name());
        if (strncmp(timeStr, "XXXX", 4) == 0) for (i = 0; i < 4; i++) timeStr[i] = '0';
        switch (strlen(timeStr)) {
            case  4:
                sscanf(timeStr, "%4d", &year);
                break;
            case  7:
                sscanf(timeStr, "%4d-%2d", &year, &month);
                break;
            case 10:
                sscanf(timeStr, "%4d-%2d-%2d", &year, &month, &day);
                break;
            case 13:
                sscanf(timeStr, "%4d-%2d-%2d %2d", &year, &month, &day, &hour);
                break;
            case 16:
                sscanf(timeStr, "%4d-%2d-%2d %2d:%2d", &year, &month, &day, &hour, &minute);
                break;
        }
        if (strlen(unitStr) > 0) {
            val = ut_encode_time(year, month, day, hour, minute, (double) 0.0);
            val = cv_convert_double(cvConverter, val);
            record[layerID] = (int) val;
        }
        else record[layerID] = layerID;
    }
    if ((status = nc_put_vara_int(ncid, timeid, &start, &count, record)) != NC_NOERR) {
        CMmsgPrint(CMmsgAppError, "NC Error '%s' in: %s %d", nc_strerror(status), __FILE__, __LINE__);
        ut_free_system(utSystem);
        ut_free(utUnit);
        ut_free(baseTimeUnit);
        cv_free(cvConverter);
        delete gridIF;
        free(record);
        return (DBFault);
    }

    if ((status = nc_redef(ncid)) != NC_NOERR) {
        CMmsgPrint(CMmsgAppError, "NC Error '%s' in: %s %d", nc_strerror(status), __FILE__, __LINE__);
        ut_free_system(utSystem);
        ut_free(utUnit);
        ut_free(baseTimeUnit);
        cv_free(cvConverter);
        delete gridIF;
        return (DBFault);
    }
    extent[0] = record[0];
    extent[1] = record[layerID - 1];
    if ((status = nc_put_att_int(ncid, timeid, "actual_range", NC_INT, 2, extent)) != NC_NOERR) {
        CMmsgPrint(CMmsgAppError, "NC Error '%s' in: %s %d", nc_strerror(status), __FILE__, __LINE__);
        ut_free_system(utSystem);
        ut_free(utUnit);
        ut_free(baseTimeUnit);
        cv_free(cvConverter);
        delete gridIF;
        return (DBFault);
    }
    if ((status = nc_enddef(ncid)) != NC_NOERR) {
        CMmsgPrint(CMmsgAppError, "NC Error '%s' in: %s %d", nc_strerror(status), __FILE__, __LINE__);
        ut_free_system(utSystem);
        ut_free(utUnit);
        ut_free(baseTimeUnit);
        cv_free(cvConverter);
        delete gridIF;
        return (DBFault);
    }

    delete gridIF;
    free(record);
    if (strlen(unitStr) > 0) {
        ut_free(utUnit);
        cv_free(cvConverter);
    }
    ut_free(baseTimeUnit);
    ut_free_system(utSystem);
    return (DBSuccess);
}

static DBInt _DBExportNetCDFTable(DBObjTable *table, int ncid) {
    int status, dimids[2];
    size_t index[2], count[2];
    const char *tableName, *fieldName;
    nc_type vtype;
    DBInt fieldID, itemID;
    DBObjTableField *fieldRec;
    DBObjRecord *itemRec;

    if ((status = nc_redef(ncid)) != NC_NOERR) {
        CMmsgPrint(CMmsgAppError, "NC Error '%s' in: %s %d", nc_strerror(status), __FILE__, __LINE__);
        return (DBFault);
    }

    tableName = _DBExportNetCDFRename(table->Name());
    if (!(((nc_inq_dimid(ncid, "time", dimids) == NC_NOERR) ||
           (nc_inq_dimid(ncid, tableName, dimids) == NC_NOERR)) &&
          (nc_inq_dimlen(ncid, dimids[0], index) == NC_NOERR) &&
          (index[0] == (size_t) table->ItemNum()))) {
        if ((status = nc_def_dim(ncid, tableName, table->ItemNum(), dimids)) != NC_NOERR) {
            CMmsgPrint(CMmsgAppError, "NC Error '%s' in: %s %d", nc_strerror(status), __FILE__, __LINE__);
            return (DBFault);
        }
    }

    for (fieldID = 0; fieldID < table->FieldNum(); ++fieldID) {
        fieldRec = table->Field(fieldID);
        if (!DBTableFieldIsVisible(fieldRec) && (strcmp(fieldRec->Name(), "MissingValue") == 0)) continue;
        fieldName = _DBExportNetCDFRename(fieldRec->Name());
        switch (fieldRec->Type()) {
            case DBTableFieldString: {
                int varid;
                size_t dimlen;
                char *str, *dimname;

                if (fieldRec->Length() <= 8) {
                    dimname = (char *) "short_string";
                    dimlen = 8;
                }
                else if (fieldRec->Length() <= 64) {
                    dimname = (char *) "string";
                    dimlen = 64;
                }
                else if (fieldRec->Length() <= NC_MAX_NAME) {
                    dimname = (char *) "long_string";
                    dimlen = NC_MAX_NAME;
                }
                else {
                    dimname = (char *) "text";
                    dimlen = 256;
                }

                if (((nc_inq_dimid(ncid, dimname, dimids + 1) == NC_NOERR) ||
                     ((nc_def_dim(ncid, dimname, dimlen, dimids + 1)) == NC_NOERR)) &&
                    ((status = nc_def_var(ncid, fieldName, NC_CHAR, (int) 2, dimids, &varid)) == NC_NOERR) &&
                    ((status = nc_enddef(ncid)) == NC_NOERR)) {
                    index[1] = 0;
                    count[0] = 1;
                    for (itemID = 0; itemID < table->ItemNum(); itemID++) {
                        itemRec = table->Item(itemID);
                        str = fieldRec->String(itemRec);
                        index[0] = itemID;
                        count[1] = strlen(str) + 1;
                        if ((status = nc_put_vara_text(ncid, varid, index, count, str)) != NC_NOERR) {
                            CMmsgPrint(CMmsgAppError, "NC Error '%s [%s]' in: %s %d", nc_strerror(status),
                                       fieldRec->Name(), __FILE__, __LINE__);
                            return (DBFault);
                        }
                    }
                    if ((status = nc_redef(ncid)) != NC_NOERR) {
                        CMmsgPrint(CMmsgAppError, "NC Error '%s' in: %s %d", nc_strerror(status), __FILE__, __LINE__);
                        return (DBFault);
                    }
                }
                else {
                    CMmsgPrint(CMmsgUsrError, "Skipping variable :%s", fieldName);
                    CMmsgPrint(CMmsgAppError, "NC Error '%s' in: %s %d", nc_strerror(status), __FILE__, __LINE__);
                }
            }
                break;
            case DBTableFieldInt: {
                int varid;
                switch (fieldRec->Length()) {
                    case sizeof(char):
                    case sizeof(short):
                        vtype = NC_SHORT;
                        break;
                    case sizeof(int):
                        vtype = NC_INT;
                        break;
                    default: {
                        CMmsgPrint(CMmsgAppError, "Invalid field size in: %s %d", __FILE__, __LINE__);
                        return (DBFault);
                    }
                }
                if (((status = nc_def_var(ncid, fieldName, vtype, (int) 1, dimids, &varid)) == NC_NOERR) &&
                    ((status = nc_enddef(ncid)) == NC_NOERR)) {
                    int var;

                    for (itemID = 0; itemID < table->ItemNum(); itemID++) {
                        itemRec = table->Item(itemID);
                        var = fieldRec->Int(itemRec);
                        index[0] = itemID;
                        if ((status = nc_put_var1_int(ncid, varid, index, &var)) != NC_NOERR) {
                            CMmsgPrint(CMmsgAppError, "NC Error '%s [%s]' in: %s %d", nc_strerror(status),
                                       fieldRec->Name(), __FILE__, __LINE__);
                            return (DBFault);
                        }
                    }
                    if ((status = nc_redef(ncid)) != NC_NOERR) {
                        CMmsgPrint(CMmsgAppError, "NC Error '%s' in: %s %d", nc_strerror(status), __FILE__, __LINE__);
                        return (DBFault);
                    }
                }
                else {
                    CMmsgPrint(CMmsgUsrError, "Skipping variable :%s", fieldName);
                    CMmsgPrint(CMmsgAppError, "NC Error '%s' in: %s %d", nc_strerror(status), __FILE__, __LINE__);
                }
            }
                break;
            case DBTableFieldFloat: {
                int varid;
                switch (fieldRec->Length()) {
                    case sizeof(DBFloat4):
                        vtype = NC_FLOAT;
                        break;
                    case sizeof(DBFloat):
                        vtype = NC_DOUBLE;
                        break;
                    default: {
                        CMmsgPrint(CMmsgAppError, "Invalid field size in: %s %d", __FILE__, __LINE__);
                        return (DBFault);
                    }
                }
                if (((status = nc_def_var(ncid, fieldName, vtype, (int) 1, dimids, &varid)) == NC_NOERR) &&
                    ((status = nc_enddef(ncid)) == NC_NOERR)) {
                    double var;

                    for (itemID = 0; itemID < table->ItemNum(); itemID++) {
                        itemRec = table->Item(itemID);
                        var = fieldRec->Float(itemRec);
                        index[0] = itemID;
                        if ((status = nc_put_var1_double(ncid, varid, index, &var)) != NC_NOERR) {
                            CMmsgPrint(CMmsgAppError, "NC Error '%s [%s]' in: %s %d", nc_strerror(status),
                                       fieldRec->Name(), __FILE__, __LINE__);
                            return (DBFault);
                        }
                    }
                    if ((status = nc_redef(ncid)) != NC_NOERR) {
                        CMmsgPrint(CMmsgAppError, "NC Error '%s' in: %s %d", nc_strerror(status), __FILE__, __LINE__);
                        return (DBFault);
                    }
                }
                else {
                    CMmsgPrint(CMmsgAppError, "Skipping variable :%s", fieldName);
                    CMmsgPrint(CMmsgAppError, "NC Error '%s' in: %s %d", nc_strerror(status), __FILE__, __LINE__);
                }
            }
                break;
            case DBTableFieldDate: {
                int varid;
                if (((status = nc_def_var(ncid, fieldName, NC_DOUBLE, (int) 1, dimids, &varid)) == NC_NOERR) &&
                    ((status = nc_enddef(ncid)) == NC_NOERR)) {
                }
                else {
                    CMmsgPrint(CMmsgAppError, "Skipping variable :%s", fieldName);
                    CMmsgPrint(CMmsgAppError, "NC Error '%s' in: %s %d", nc_strerror(status), __FILE__, __LINE__);
                }
            }
                break;
            case DBTableFieldCoord:
                break;
            case DBTableFieldRegion: {
                int varid;
                double box[4];
                if (((nc_inq_dimid(ncid, "box", dimids + 1) == NC_NOERR) ||
                     ((nc_def_dim(ncid, "box", 4, dimids + 1)) == NC_NOERR)) &&
                    ((status = nc_def_var(ncid, fieldName, NC_DOUBLE, (int) 2, dimids, &varid)) == NC_NOERR) &&
                    ((status = nc_enddef(ncid)) == NC_NOERR)) {
                    DBRegion region;

                    index[1] = 0;
                    count[0] = 1;
                    count[1] = 4;
                    for (itemID = 0; itemID < table->ItemNum(); itemID++) {
                        itemRec = table->Item(itemID);
                        region = fieldRec->Region(itemRec);
                        index[0] = itemID;
                        box[0] = region.LowerLeft.X;
                        box[1] = region.LowerLeft.Y;
                        box[2] = region.UpperRight.X;
                        box[3] = region.UpperRight.Y;
                        if ((status = nc_put_vara_double(ncid, varid, index, count, box)) != NC_NOERR) {
                            CMmsgPrint(CMmsgAppError, "NC Error '%s [%s]' in: %s %d", nc_strerror(status),
                                       fieldRec->Name(), __FILE__, __LINE__);
                            return (DBFault);
                        }
                    }
                    if ((status = nc_redef(ncid)) != NC_NOERR) {
                        CMmsgPrint(CMmsgAppError, "NC Error '%s' in: %s %d", nc_strerror(status), __FILE__, __LINE__);
                        return (DBFault);
                    }
                }
                else {
                    CMmsgPrint(CMmsgAppError, "Skipping variable :%s", fieldName);
                    CMmsgPrint(CMmsgAppError, "NC Error '%s' in: %s %d", nc_strerror(status), __FILE__, __LINE__);
                }
            }
                break;
            case DBTableFieldPosition: {
                char rowName[NC_MAX_NAME], colName[NC_MAX_NAME];
                int rvarid, cvarid;
                DBPosition pos;

                snprintf(rowName, sizeof(rowName), "%s_row", fieldName);
                snprintf(colName, sizeof(colName), "%s_col", fieldName);
                if (((status = nc_def_var(ncid, rowName, NC_INT, (int) 1, dimids, &rvarid)) == NC_NOERR) &&
                    ((status = nc_def_var(ncid, colName, NC_INT, (int) 1, dimids, &cvarid)) == NC_NOERR) &&
                    ((status = nc_enddef(ncid)) == NC_NOERR)) {

                    for (itemID = 0; itemID < table->ItemNum(); itemID++) {
                        itemRec = table->Item(itemID);
                        pos = fieldRec->Position(itemRec);
                        index[0] = itemID;
                        if (((status = nc_put_var1_int(ncid, cvarid, index, &pos.Col)) != NC_NOERR) ||
                            ((status = nc_put_var1_int(ncid, rvarid, index, &pos.Row)) != NC_NOERR)) {
                            CMmsgPrint(CMmsgAppError, "NC Error '%s [%s]' in: %s %d", nc_strerror(status),
                                       fieldRec->Name(), __FILE__, __LINE__);
                            return (DBFault);
                        }
                    }
                    if ((status = nc_redef(ncid)) != NC_NOERR) {
                        CMmsgPrint(CMmsgAppError, "NC Error '%s' in: %s %d", nc_strerror(status), __FILE__, __LINE__);
                        return (DBFault);
                    }
                }
                else {
                    CMmsgPrint(CMmsgAppError, "Skipping variable :%s", fieldName);
                    CMmsgPrint(CMmsgAppError, "NC Error '%s' in: %s %d", nc_strerror(status), __FILE__, __LINE__);
                }
            }
                break;
            case DBTableFieldTableRec:
            case DBTableFieldDataRec: {
                int varid;
                if (((status = nc_def_var(ncid, fieldName, NC_INT, (int) 1, dimids, &varid)) == NC_NOERR) &&
                    ((status = nc_enddef(ncid)) == NC_NOERR)) {
                    int var;

                    for (itemID = 0; itemID < table->ItemNum(); itemID++) {
                        itemRec = table->Item(itemID);
                        var = (fieldRec->Record(itemRec))->RowID();
                        index[0] = itemID;
                        if ((status = nc_put_var1_int(ncid, varid, index, &var)) != NC_NOERR) {
                            CMmsgPrint(CMmsgAppError, "NC Error '%s [%s]' in: %s %d", nc_strerror(status),
                                       fieldRec->Name(), __FILE__, __LINE__);
                            return (DBFault);
                        }
                    }
                    if ((status = nc_redef(ncid)) != NC_NOERR) {
                        CMmsgPrint(CMmsgAppError, "NC Error '%s' in: %s %d", nc_strerror(status), __FILE__, __LINE__);
                        return (DBFault);
                    }
                }
                else {
                    CMmsgPrint(CMmsgAppError, "Skipping variable :%s", fieldName);
                    CMmsgPrint(CMmsgAppError, "NC Error '%s' in: %s %d", nc_strerror(status), __FILE__, __LINE__);
                }
            }
                break;
            default: {
                CMmsgPrint(CMmsgAppError, "Invalid field type in: %s %d", __FILE__, __LINE__);
                return (DBFault);
            }
        }
    }
    if ((status = nc_enddef(ncid)) != NC_NOERR) {
        CMmsgPrint(CMmsgAppError, "NC Error '%s' in: %s %d", nc_strerror(status), __FILE__, __LINE__);
        return (DBFault);
    }
    return (DBSuccess);
}

DBInt DBExportNetCDF(DBObjData *dbData, const char *fileName) {
    const char *str;
    int ncid, status, dimids[3], varid;
    size_t start[3], count[3];

    if ((status = nc_create(fileName, NC_CLOBBER, &ncid)) != NC_NOERR) {
        CMmsgPrint(CMmsgAppError, "NC Error '%s' (%s) in: %s %d", nc_strerror(status), fileName, __FILE__, __LINE__);
        return (DBFault);
    }
    if ((status = nc_put_att_text(ncid, NC_GLOBAL, "Conventions", strlen("CF-1.2"), "CF-1.2")) != NC_NOERR) {
        CMmsgPrint(CMmsgAppError, "NC Error '%s' in: %s %d", nc_strerror(status), __FILE__, __LINE__);
        nc_close(ncid);
        return (DBFault);
    }
    if ((status = nc_put_att_text(ncid, NC_GLOBAL, "title", strlen(dbData->Name()), dbData->Name())) != NC_NOERR) {
        CMmsgPrint(CMmsgAppError, "NC Error '%s' in: %s %d", nc_strerror(status), __FILE__, __LINE__);
        nc_close(ncid);
        return (DBFault);
    }
    str = _DBExportNetCDFRename(DBDataTypeString(dbData->Type()));
    if ((status = nc_put_att_text(ncid, NC_GLOBAL, "data_type", strlen(str), str)) != NC_NOERR) {
        CMmsgPrint(CMmsgAppError, "NC Error '%s' in: %s %d", nc_strerror(status), __FILE__, __LINE__);
        nc_close(ncid);
        return (DBFault);
    }
    str = dbData->Document(DBDocGeoDomain);
    if ((status = nc_put_att_text(ncid, NC_GLOBAL, "domain", strlen(str), str)) != NC_NOERR) {
        CMmsgPrint(CMmsgAppError, "NC Error '%s' in: %s %d", nc_strerror(status), __FILE__, __LINE__);
        nc_close(ncid);
        return (DBFault);
    }
    str = _DBExportNetCDFRename(dbData->Document(DBDocSubject));
    if ((status = nc_put_att_text(ncid, NC_GLOBAL, "subject", strlen(str), str)) != NC_NOERR) {
        CMmsgPrint(CMmsgAppError, "NC Error '%s' in: %s %d", nc_strerror(status), __FILE__, __LINE__);
        nc_close(ncid);
        return (DBFault);
    }
    str = dbData->Document(DBDocCitationRef);
    if ((status = nc_put_att_text(ncid, NC_GLOBAL, "references", strlen(str), str)) != NC_NOERR) {
        CMmsgPrint(CMmsgAppError, "NC Error '%s' in: %s %d", nc_strerror(status), __FILE__, __LINE__);
        nc_close(ncid);
        return (DBFault);
    }
    str = dbData->Document(DBDocCitationInst);
    if ((status = nc_put_att_text(ncid, NC_GLOBAL, "institution", strlen(str), str)) != NC_NOERR) {
        CMmsgPrint(CMmsgAppError, "NC Error '%s' in: %s %d", nc_strerror(status), __FILE__, __LINE__);
        nc_close(ncid);
        return (DBFault);
    }
    str = dbData->Document(DBDocSourceInst);
    if ((status = nc_put_att_text(ncid, NC_GLOBAL, "source", strlen(str), str)) != NC_NOERR) {
        CMmsgPrint(CMmsgAppError, "NC Error '%s' in: %s %d", nc_strerror(status), __FILE__, __LINE__);
        nc_close(ncid);
        return (DBFault);
    }
    str = dbData->Document(DBDocComment);
    if ((status = nc_put_att_text(ncid, NC_GLOBAL, "comments", strlen(str), str)) != NC_NOERR) {
        CMmsgPrint(CMmsgAppError, "NC Error '%s' in: %s %d", nc_strerror(status), __FILE__, __LINE__);
        nc_close(ncid);
        return (DBFault);
    }

    if ((status = nc_enddef(ncid)) != NC_NOERR) {
        CMmsgPrint(CMmsgAppError, "NC Error '%s' in: %s %d", nc_strerror(status), __FILE__, __LINE__);
        nc_close(ncid);
        return (DBFault);
    }

    switch (dbData->Type()) {
        case DBTypeVectorPoint: {
            if (_DBExportNetCDFPoint(dbData, ncid) == DBFault) {
                nc_close(ncid);
                return (DBFault);
            }
            if (_DBExportNetCDFTable(dbData->Table(DBrNItems), ncid) == DBFault) {
                nc_close(ncid);
                return (DBFault);
            }
            if (_DBExportNetCDFTable(dbData->Table(DBrNSymbols), ncid) == DBFault) {
                nc_close(ncid);
                return (DBFault);
            }
        }
            break;
        case DBTypeGridDiscrete: {
            short *record, fillVal = DBFault;
            DBInt intVal;
            DBInt layerID;
            DBPosition pos;
            DBObjRecord *layerRec;
            DBGridIF *gridIF;

            if (_DBExportNetCDFGridDefine(dbData, ncid, dimids) == DBFault) {
                nc_close(ncid);
                return (DBFault);
            }
            if (_DBExportNetCDFTimeDefine(dbData, ncid, dimids) == DBFault) {
                nc_close(ncid);
                return (DBFault);
            }
            if (_DBExportNetCDFTable(dbData->Table(DBrNItems), ncid) == DBFault) {
                nc_close(ncid);
                return (DBFault);
            }
            if (_DBExportNetCDFTable(dbData->Table(DBrNSymbols), ncid) == DBFault) {
                nc_close(ncid);
                return (DBFault);
            }

            if ((status = nc_redef(ncid)) != NC_NOERR) {
                CMmsgPrint(CMmsgAppError, "NC Error '%s' in: %s %d", nc_strerror(status), __FILE__, __LINE__);
                nc_close(ncid);
                return (DBFault);
            }

            if ((status = nc_def_var(ncid, _DBExportNetCDFRename(dbData->Document(DBDocSubject)), NC_SHORT, (int) 3,
                                     dimids, &varid)) != NC_NOERR) {
                CMmsgPrint(CMmsgAppError, "NC Error '%s' in: %s %d", nc_strerror(status), __FILE__, __LINE__);
                nc_close(ncid);
                return (DBFault);
            }
            str = dbData->Name();
            if ((status = nc_put_att_text(ncid, varid, "long_name", strlen(str), str)) != NC_NOERR) {
                CMmsgPrint(CMmsgAppError, "NC Error '%s' in: %s %d", nc_strerror(status), __FILE__, __LINE__);
                nc_close(ncid);
                return (DBFault);
            }
            str = dbData->Document(DBDocSubject);
            if ((status = nc_put_att_text(ncid, varid, "standard_name", strlen(str), str)) != NC_NOERR) {
                CMmsgPrint(CMmsgAppError, "NC Error '%s' in: %s %d", nc_strerror(status), __FILE__, __LINE__);
                nc_close(ncid);
                return (DBFault);
            }
            if ((status = nc_put_att_text(ncid, varid, "var_desc", strlen(str), str)) != NC_NOERR) {
                CMmsgPrint(CMmsgAppError, "NC Error '%s' in: %s %d", nc_strerror(status), __FILE__, __LINE__);
                nc_close(ncid);
                return (DBFault);
            }
            if ((status = nc_put_att_short(ncid, varid, "_FillValue", NC_SHORT, 1, &fillVal)) != NC_NOERR) {
                CMmsgPrint(CMmsgAppError, "NC Error '%s' in: %s %d", nc_strerror(status), __FILE__, __LINE__);
                nc_close(ncid);
                return (DBFault);
            }
            if ((status = nc_put_att_short(ncid, varid, "missing_value", NC_SHORT, 1, &fillVal)) != NC_NOERR) {
                CMmsgPrint(CMmsgAppError, "NC Error '%s' in: %s %d", nc_strerror(status), __FILE__, __LINE__);
                nc_close(ncid);
                return (DBFault);
            }

            if ((status = nc_enddef(ncid)) != NC_NOERR) {
                CMmsgPrint(CMmsgAppError, "NC Error '%s' in: %s %d", nc_strerror(status), __FILE__, __LINE__);
                nc_close(ncid);
                return (DBFault);
            }

            gridIF = new DBGridIF(dbData);
            if ((record = (short *) calloc(gridIF->ColNum(), sizeof(short))) == (short *) NULL) {
                CMmsgPrint(CMmsgSysError, "Memory allocation error in: %s %d", __FILE__, __LINE__);
                delete gridIF;
                nc_close(ncid);
                return (DBFault);
            }
            for (layerID = 0; layerID < gridIF->LayerNum(); layerID++) {
                layerRec = gridIF->Layer(layerID);
                start[DIMTime] = layerID;
                count[DIMTime] = 1;
                for (pos.Row = 0; pos.Row < gridIF->RowNum(); pos.Row++) {
                    start[DIMLat] = pos.Row;
                    count[DIMLat] = 1;
                    start[DIMLon] = 0;
                    count[DIMLon] = gridIF->ColNum();
                    for (pos.Col = 0; pos.Col < gridIF->ColNum(); pos.Col++)
                        record[pos.Col] = gridIF->Value(layerRec, pos, &intVal) ? intVal : fillVal;
                    if ((status = nc_put_vara_short(ncid, varid, start, count, record)) != NC_NOERR) {
                        CMmsgPrint(CMmsgAppError, "NC Error '%s' in: %s %d", nc_strerror(status), __FILE__, __LINE__);
                        free(record);
                        delete gridIF;
                        nc_close(ncid);
                        return (DBFault);
                    }
                }
            }
            free(record);
            delete gridIF;
        }
            break;
        case DBTypeGridContinuous: {
            double *record, fillVal;
            double extent[2], dataOffset, scaleFactor;
            DBFloat gridVal;
            DBInt layerID;
            DBPosition pos;
            DBObjRecord *layerRec;
            DBGridIF *gridIF;

            if (_DBExportNetCDFGridDefine(dbData, ncid, dimids) == DBFault) {
                nc_close(ncid);
                return (DBFault);
            }
            if (_DBExportNetCDFTimeDefine(dbData, ncid, dimids) == DBFault) {
                nc_close(ncid);
                return (DBFault);
            }
            if (_DBExportNetCDFTable(dbData->Table(DBrNItems), ncid) == DBFault) {
                nc_close(ncid);
                return (DBFault);
            }

            /* Begin Defining Core Variable */
            if ((status = nc_redef(ncid)) != NC_NOERR) {
                CMmsgPrint(CMmsgAppError, "NC Error '%s' in: %s %d", nc_strerror(status), __FILE__, __LINE__);
                nc_close(ncid);
                return (DBFault);
            }
            if ((status = nc_def_var(ncid, _DBExportNetCDFRename(dbData->Document(DBDocSubject)), NC_DOUBLE, (int) 3,
                                     dimids, &varid)) != NC_NOERR) {
                CMmsgPrint(CMmsgAppError, "NC Error '%s' in: %s %d", nc_strerror(status), __FILE__, __LINE__);
                nc_close(ncid);
                return (DBFault);
            }

            str = dbData->Name();
            if ((status = nc_put_att_text(ncid, varid, "long_name", strlen(str), str)) != NC_NOERR) {
                CMmsgPrint(CMmsgAppError, "NC Error '%s' in: %s %d", nc_strerror(status), __FILE__, __LINE__);
                nc_close(ncid);
                return (DBFault);
            }
            str = dbData->Document(DBDocSubject);
            if ((status = nc_put_att_text(ncid, varid, "standard_name", strlen(str), str)) != NC_NOERR) {
                CMmsgPrint(CMmsgAppError, "NC Error '%s' in: %s %d", nc_strerror(status), __FILE__, __LINE__);
                nc_close(ncid);
                return (DBFault);
            }
            if ((status = nc_put_att_text(ncid, varid, "var_desc", strlen(str), str)) != NC_NOERR) {
                CMmsgPrint(CMmsgAppError, "NC Error '%s' in: %s %d", nc_strerror(status), __FILE__, __LINE__);
                nc_close(ncid);
                return (DBFault);
            }

            gridIF = new DBGridIF(dbData);

            fillVal = gridIF->MissingValue();
            scaleFactor = 1.0;
            dataOffset = 0.0;
            extent[0] = gridIF->Minimum();
            extent[1] = gridIF->Maximum();
            if ((status = nc_put_att_double(ncid, varid, "missing_value", NC_DOUBLE, 1, &fillVal)) != NC_NOERR) {
                CMmsgPrint(CMmsgAppError, "NC Error '%s' in: %s %d", nc_strerror(status), __FILE__, __LINE__);
                nc_close(ncid);
                return (DBFault);
            }
            if ((status = nc_put_att_double(ncid, varid, "_FillValue", NC_DOUBLE, 1, &fillVal)) != NC_NOERR) {
                CMmsgPrint(CMmsgAppError, "NC Error '%s' in: %s %d", nc_strerror(status), __FILE__, __LINE__);
                nc_close(ncid);
                return (DBFault);
            }
            if ((status = nc_put_att_double(ncid, varid, "scale_factor", NC_DOUBLE, 1, &scaleFactor)) != NC_NOERR) {
                CMmsgPrint(CMmsgAppError, "NC Error '%s' in: %s %d", nc_strerror(status), __FILE__, __LINE__);
                delete gridIF;
                nc_close(ncid);
                return (DBFault);
            }
            if ((status = nc_put_att_double(ncid, varid, "add_offset", NC_DOUBLE, 1, &dataOffset)) != NC_NOERR) {
                CMmsgPrint(CMmsgAppError, "NC Error '%s' in: %s %d", nc_strerror(status), __FILE__, __LINE__);
                delete gridIF;
                nc_close(ncid);
                return (DBFault);
            }
            if ((status = nc_put_att_double(ncid, varid, "actual_range", NC_DOUBLE, 2, extent)) != NC_NOERR) {
                CMmsgPrint(CMmsgAppError, "NC Error '%s' in: %s %d", nc_strerror(status), __FILE__, __LINE__);
                delete gridIF;
                nc_close(ncid);
                return (DBFault);
            }
            if ((status = nc_enddef(ncid)) != NC_NOERR) {
                CMmsgPrint(CMmsgAppError, "NC Error '%s' in: %s %d", nc_strerror(status), __FILE__, __LINE__);
                delete gridIF;
                nc_close(ncid);
                return (DBFault);
            }
            /* End Defining Core Variable */

            if ((record = (double *) calloc(gridIF->ColNum(), sizeof(double))) == (double *) NULL) {
                CMmsgPrint(CMmsgSysError, "Memory allocation error in: %s %d", __FILE__, __LINE__);
                delete gridIF;
                nc_close(ncid);
                return (DBFault);
            }
            for (layerID = 0; layerID < gridIF->LayerNum(); layerID++) {
                layerRec = gridIF->Layer(layerID);
                start[DIMTime] = layerID;
                count[DIMTime] = 1;
                for (pos.Row = 0; pos.Row < gridIF->RowNum(); pos.Row++) {
                    start[DIMLat] = pos.Row;
                    count[DIMLat] = 1;
                    start[DIMLon] = 0;
                    count[DIMLon] = gridIF->ColNum();
                    for (pos.Col = 0; pos.Col < gridIF->ColNum(); pos.Col++)
                        record[pos.Col] = gridIF->Value(layerRec, pos, &gridVal) ? gridVal : fillVal;
                    if ((status = nc_put_vara_double(ncid, varid, start, count, record)) != NC_NOERR) {
                        CMmsgPrint(CMmsgAppError, "NC Error '%s' in: %s %d", nc_strerror(status), __FILE__, __LINE__);
                        free(record);
                        delete gridIF;
                        nc_close(ncid);
                        return (DBFault);
                    }
                }
            }
            free(record);
            delete gridIF;
        }
            break;
        case DBTypeNetwork: {
            int *record, fillVal = DBFault;
            int extent[2];
            DBPosition pos;
            DBNetworkIF *netIF;
            DBObjRecord *cellRec;

            if (_DBExportNetCDFGridDefine(dbData, ncid, dimids) == DBFault) {
                nc_close(ncid);
                return (DBFault);
            }
            if (_DBExportNetCDFTable(dbData->Table(DBrNItems), ncid) == DBFault) {
                nc_close(ncid);
                return (DBFault);
            }
            if (_DBExportNetCDFTable(dbData->Table(DBrNCells), ncid) == DBFault) {
                nc_close(ncid);
                return (DBFault);
            }
            if (_DBExportNetCDFTable(dbData->Table(DBrNSymbols), ncid) == DBFault) {
                nc_close(ncid);
                return (DBFault);
            }

            /* Begin Defining Core Variable */
            if ((status = nc_redef(ncid)) != NC_NOERR) {
                CMmsgPrint(CMmsgAppError, "NC Error '%s' in: %s %d", nc_strerror(status), __FILE__, __LINE__);
                nc_close(ncid);
                return (DBFault);
            }
            if ((status = nc_def_var(ncid, _DBExportNetCDFRename(dbData->Document(DBDocSubject)), NC_INT, (int) 3,
                                     dimids + 1, &varid)) != NC_NOERR) {
                CMmsgPrint(CMmsgAppError, "NC Error '%s' in: %s %d", nc_strerror(status), __FILE__, __LINE__);
                nc_close(ncid);
                return (DBFault);
            }

            str = dbData->Name();
            if ((status = nc_put_att_text(ncid, varid, "long_name", strlen(str), str)) != NC_NOERR) {
                CMmsgPrint(CMmsgAppError, "NC Error '%s' in: %s %d", nc_strerror(status), __FILE__, __LINE__);
                nc_close(ncid);
                return (DBFault);
            }
            str = dbData->Document(DBDocSubject);
            if ((status = nc_put_att_text(ncid, varid, "standard_name", strlen(str), str)) != NC_NOERR) {
                CMmsgPrint(CMmsgAppError, "NC Error '%s' in: %s %d", nc_strerror(status), __FILE__, __LINE__);
                nc_close(ncid);
                return (DBFault);
            }
            if ((status = nc_put_att_text(ncid, varid, "var_desc", strlen(str), str)) != NC_NOERR) {
                CMmsgPrint(CMmsgAppError, "NC Error '%s' in: %s %d", nc_strerror(status), __FILE__, __LINE__);
                nc_close(ncid);
                return (DBFault);
            }
            if ((status = nc_put_att_int(ncid, varid, "_FillValue", NC_INT, 1, &fillVal)) != NC_NOERR) {
                CMmsgPrint(CMmsgAppError, "NC Error '%s' in: %s %d", nc_strerror(status), __FILE__, __LINE__);
                nc_close(ncid);
                return (DBFault);
            }
            if ((status = nc_put_att_int(ncid, varid, "missing_value", NC_INT, 1, &fillVal)) != NC_NOERR) {
                CMmsgPrint(CMmsgAppError, "NC Error '%s' in: %s %d", nc_strerror(status), __FILE__, __LINE__);
                nc_close(ncid);
                return (DBFault);
            }

            netIF = new DBNetworkIF(dbData);

            extent[0] = 0;
            extent[1] = netIF->CellNum();
            if ((status = nc_put_att_int(ncid, varid, "actual_range", NC_INT, 2, extent)) != NC_NOERR) {
                CMmsgPrint(CMmsgAppError, "NC Error '%s' in: %s %d", nc_strerror(status), __FILE__, __LINE__);
                delete netIF;
                nc_close(ncid);
                return (DBFault);
            }
            if ((status = nc_enddef(ncid)) != NC_NOERR) {
                CMmsgPrint(CMmsgAppError, "NC Error '%s' in: %s %d", nc_strerror(status), __FILE__, __LINE__);
                delete netIF;
                nc_close(ncid);
                return (DBFault);
            }

            if ((record = (int *) calloc(netIF->ColNum(), sizeof(int))) == (int *) NULL) {
                CMmsgPrint(CMmsgSysError, "Memory allocation error in: %s %d");
                delete netIF;
                nc_close(ncid);
                return (DBFault);
            }
            for (pos.Row = 0; pos.Row < netIF->RowNum(); pos.Row++) {
                start[DIMLat] = pos.Row;
                count[DIMLat] = 1;
                start[DIMLon] = 0;
                count[DIMLon] = netIF->ColNum();
                for (pos.Col = 0; pos.Col < netIF->ColNum(); pos.Col++) {
                    cellRec = netIF->Cell(pos);
                    record[pos.Col] = cellRec != (DBObjRecord *) NULL ? cellRec->RowID() : fillVal;
                }
                if ((status = nc_put_vara_int(ncid, varid, start + 1, count + 1, record)) != NC_NOERR) {
                    CMmsgPrint(CMmsgAppError, "NC Error '%s' in: %s %d", nc_strerror(status), __FILE__, __LINE__);
                    free(record);
                    delete netIF;
                    nc_close(ncid);
                    return (DBFault);
                }
            }
            free(record);
            delete netIF;
        }
            break;
    }
    nc_close(ncid);
    return (DBSuccess);
}

DBInt DBImportNetCDF(DBObjData *data, const char *filename) {

    char name[NC_MAX_NAME], varname[NC_MAX_NAME], timeString[NC_MAX_NAME], /* varUnit [NC_MAX_NAME], */ longName[NC_MAX_NAME], layerName[DBStringLength];
    size_t attlen;
    int ncid, status, id, i;
    int ndims, nvars, natts, unlimdim;
    int latdim = -1, londim = -1, levdim = -1, timedim = -1;
    int varid = -1;
    int latidx = -1, timeidx = -1;
    int dimids[4];
    size_t len, start[4] = {0, 0, 0, 0}, count[4] = {1, 1, 1, 1};
    int doTimeUnit = false;
    int year, month, day, hour, minute;
    double second, resolution;
    ut_system *utSystem = (ut_system *) NULL;
    ut_set_error_message_handler(ut_ignore);
    ut_unit *baseTimeUnit = (ut_unit *) NULL;
    ut_unit *timeUnit = (ut_unit *) NULL;
    cv_converter *cvConverter = (cv_converter *) NULL;
    int rowNum = 0, colNum = 0, layerNum = 1, layerID;
    double *vector, *latitudes, *longitudes;
    double *timeSteps;
    double missingValue, fillValue;
    double scaleFactor, dataOffset;
    DBCoordinate cellSize;
    DBPosition pos;
    DBFloat cellArea, value, sumWeight = 0.0, minimum, maximum, average, stdDev;
    DBRegion extent;
    DBObjRecord *itemRec, *layerRec, *dataRec;
    DBObjTable *itemTable = data->Table(DBrNItems);
    DBObjTable *layerTable = data->Table(DBrNLayers);
    DBObjTableField *rowNumFLD = layerTable->Field(DBrNRowNum);
    DBObjTableField *colNumFLD = layerTable->Field(DBrNColNum);
    DBObjTableField *cellWidthFLD = layerTable->Field(DBrNCellWidth);
    DBObjTableField *cellHeightFLD = layerTable->Field(DBrNCellHeight);
    DBObjTableField *valueTypeFLD = layerTable->Field(DBrNValueType);
    DBObjTableField *valueSizeFLD = layerTable->Field(DBrNValueSize);
    DBObjTableField *layerFLD = layerTable->Field(DBrNLayer);
    DBObjTableField *averageFLD = itemTable->Field(DBrNAverage);
    DBObjTableField *stdDevFLD = itemTable->Field(DBrNStdDev);
    DBObjTableField *minimumFLD = itemTable->Field(DBrNMinimum);
    DBObjTableField *maximumFLD = itemTable->Field(DBrNMaximum);
    DBObjTableField *missingValueFLD = itemTable->Field(DBrNMissingValue);
    DBGridIF *gridIF;

    if ((utSystem = ut_read_xml((char *) NULL)) == (ut_system *) NULL) {
        CMmsgPrint(CMmsgAppError, "Total metal gebasz in %s:%d", __FILE__, __LINE__);
        return (DBFault);
    }
    if ((baseTimeUnit = ut_parse(utSystem, "seconds since 2001-01-01 00:00:00", UT_ASCII)) == (ut_unit *) NULL) {
        CMmsgPrint(CMmsgAppError, "Total metal gebasz in %s:%d", __FILE__, __LINE__);
        switch (ut_get_status()) {
            case UT_BAD_ARG:
                CMmsgPrint(CMmsgUsrError, "System or string is NULL!");
                break;
            case UT_SYNTAX:
                CMmsgPrint(CMmsgUsrError, "String contained a syntax error!");
                break;
            case UT_UNKNOWN:
                CMmsgPrint(CMmsgUsrError, "String contained an unknown identifier!");
                break;
            default:
                CMmsgPrint(CMmsgAppError, "System error in %s:%d!n", __FILE__, __LINE__);
        }
        ut_free_system(utSystem);
        return (DBFault);
    }
    if ((status = nc_open(filename, NC_NOWRITE, &ncid)) != NC_NOERR) {
        CMmsgPrint(CMmsgAppError, "NC Error '%s' (%s) in: %s %d", filename, nc_strerror(status), __FILE__, __LINE__);
        ut_free(baseTimeUnit);
        ut_free_system(utSystem);
        return (DBFault);
    }

    if ((status = nc_inq(ncid, &ndims, &nvars, &natts, &unlimdim)) != NC_NOERR) {
        CMmsgPrint(CMmsgAppError, "NC Error '%s' in: %s %d", nc_strerror(status), __FILE__, __LINE__);
        nc_close(ncid);
        ut_free(baseTimeUnit);
        ut_free_system(utSystem);
        return (DBFault);
    }

    for (id = 0; id < ndims; id++) {
        if ((status = nc_inq_dim(ncid, id, name, &len)) != NC_NOERR) {
            CMmsgPrint(CMmsgAppError, "NC Error '%s' in: %s %d", nc_strerror(status), __FILE__, __LINE__);
            nc_close(ncid);
            return (DBFault);
        }
        if (strncmp(name, "lon", 3) == 0) {
            londim = id;
            colNum = len;
        }
        else if (strncmp(name, "lat", 3) == 0) {
            latdim = id;
            rowNum = len;
        }
        else if (strncmp(name, "time", 4) == 0) {
            timedim = id;
            layerNum = len;
        }
        else if (strncmp(name, "level", 5) == 0) { levdim = id; }
    }
    if ((colNum < 2) || (rowNum < 2)) {
        CMmsgPrint(CMmsgAppError, "Invalid array dimensions in: %s %d", __FILE__, __LINE__);
        nc_close(ncid);
        return (DBFault);
    }
    if ((vector = (double *) calloc(colNum, sizeof(double))) == (double *) NULL) {
        CMmsgPrint(CMmsgSysError, "Memory Allocation Error in: %s %d", __FILE__, __LINE__);
        nc_close(ncid);
        ut_free(baseTimeUnit);
        ut_free_system(utSystem);
        return (DBFault);
    }
    if ((longitudes = (double *) calloc(colNum, sizeof(double))) == (double *) NULL) {
        CMmsgPrint(CMmsgSysError, "Memory Allocation Error in: %s %d", __FILE__, __LINE__);
        free(vector);
        nc_close(ncid);
        ut_free(baseTimeUnit);
        ut_free_system(utSystem);
        return (DBFault);
    }
    if ((latitudes = (double *) calloc(rowNum, sizeof(double))) == (double *) NULL) {
        CMmsgPrint(CMmsgSysError, "Memory Allocation Error in: %s %d", __FILE__, __LINE__);
        free(vector);
        free(longitudes);
        nc_close(ncid);
        ut_free(baseTimeUnit);
        ut_free_system(utSystem);
        return (DBFault);
    }
    if ((timeSteps = (double *) calloc(layerNum, sizeof(double))) == (double *) NULL) {
        CMmsgPrint(CMmsgSysError, "Memory Allocation Error in: %s %d", __FILE__, __LINE__);
        free(vector);
        free(longitudes);
        free(latitudes);
        nc_close(ncid);
        ut_free(baseTimeUnit);
        ut_free_system(utSystem);
        return (DBFault);
    }

    for (id = 0; id < nvars; id++) {
        if ((status = nc_inq_varname(ncid, id, name)) != NC_NOERR) {
            CMmsgPrint(CMmsgAppError, "NC Error '%s' in: %s %d", nc_strerror(status), __FILE__, __LINE__);
            free(vector);
            free(longitudes);
            free(latitudes);
            free(timeSteps);
            nc_close(ncid);
            ut_free(baseTimeUnit);
            ut_free_system(utSystem);
            return (DBFault);
        }
        if (strcmp(name, "level") == 0) continue;
        else if (strcmp(name, "time_bnds") == 0) continue;
        else if ((strcmp(name, "lon_bnds") == 0) ||
                 (strcmp(name, "longitude_bnds") == 0))
            continue;
        else if ((strcmp(name, "lat_bnds") == 0) ||
                 (strcmp(name, "latitude_bnds") == 0))
            continue;
        else if ((strcmp(name, "lon") == 0) ||
                 (strcmp(name, "longitude") == 0)) {
            if ((status = nc_inq_varndims(ncid, id, &ndims)) != NC_NOERR) {
                CMmsgPrint(CMmsgAppError, "NC Error '%s' in: %s %d", nc_strerror(status), __FILE__, __LINE__);
                free(vector);
                free(longitudes);
                free(latitudes);
                free(timeSteps);
                nc_close(ncid);
                ut_free(baseTimeUnit);
                ut_free_system(utSystem);
                return (DBFault);
            }
            if (ndims != 1) {
                CMmsgPrint(CMmsgAppError, "Longitude has more than one dimension in: %s %d", __FILE__, __LINE__);
                free(vector);
                free(longitudes);
                free(latitudes);
                free(timeSteps);
                nc_close(ncid);
                ut_free(baseTimeUnit);
                ut_free_system(utSystem);
                return (DBFault);
            }
            if ((status = nc_inq_vardimid(ncid, id, dimids)) != NC_NOERR) {
                CMmsgPrint(CMmsgAppError, "NC Error '%s' in: %s %d", nc_strerror(status), __FILE__, __LINE__);
                free(vector);
                free(longitudes);
                free(latitudes);
                free(timeSteps);
                nc_close(ncid);
                ut_free(baseTimeUnit);
                ut_free_system(utSystem);
                return (DBFault);
            }
            if (dimids[0] != londim) {
                CMmsgPrint(CMmsgAppError, "Longitude has invalid dimension in: %s %d", __FILE__, __LINE__);
                free(vector);
                free(longitudes);
                free(latitudes);
                free(timeSteps);
                nc_close(ncid);
                ut_free(baseTimeUnit);
                ut_free_system(utSystem);
                return (DBFault);
            }
            start[0] = 0;
            count[0] = colNum;
            if ((status = nc_get_vara_double(ncid, id, start, count, longitudes)) != NC_NOERR) {
                CMmsgPrint(CMmsgAppError, "NC Error '%s' in: %s %d", nc_strerror(status), __FILE__, __LINE__);
                free(vector);
                free(longitudes);
                free(latitudes);
                free(timeSteps);
                nc_close(ncid);
                ut_free(baseTimeUnit);
                ut_free_system(utSystem);
                return (DBFault);
            }
            cellSize.X = fabs(longitudes[0] - longitudes[1]);
            extent.LowerLeft.X = longitudes[0] < longitudes[1] ? longitudes[0] : longitudes[1];
            extent.UpperRight.X = longitudes[0] > longitudes[1] ? longitudes[0] : longitudes[1];
            for (i = 2; i < colNum; i++) {
                extent.LowerLeft.X  = extent.LowerLeft.X  < longitudes[i] ? extent.LowerLeft.X  : longitudes[i];
                extent.UpperRight.X = extent.UpperRight.X > longitudes[i] ? extent.UpperRight.X : longitudes[i];
                if (CMmathEqualValues(cellSize.X, fabs(longitudes[i] - longitudes[i - 1])) != true) {
                    CMmsgPrint(CMmsgAppError, "Longitude has irregular spacing in: %s %d", __FILE__, __LINE__);
                    free(vector);
                    free(longitudes);
                    free(latitudes);
                    free(timeSteps);
                    nc_close(ncid);
                    ut_free(baseTimeUnit);
                    ut_free_system(utSystem);
                    return (DBFault);
                }
            }
            extent.LowerLeft.X = extent.LowerLeft.X - cellSize.X / 2.0;
            extent.UpperRight.X = extent.UpperRight.X + cellSize.X / 2.0;
        }
        else if ((strcmp(name, "lat") == 0) ||
                 (strcmp(name, "latitude") == 0)) {
            if ((status = nc_inq_varndims(ncid, id, &ndims)) != NC_NOERR) {
                CMmsgPrint(CMmsgAppError, "NC Error '%s' in: %s %d", nc_strerror(status), __FILE__, __LINE__);
                free(vector);
                free(longitudes);
                free(latitudes);
                free(timeSteps);
                nc_close(ncid);
                ut_free(baseTimeUnit);
                ut_free_system(utSystem);
                return (DBFault);
            }
            if (ndims != 1) {
                CMmsgPrint(CMmsgAppError, "Latitude has more than one dimension in: %s %d", __FILE__, __LINE__);
                return (DBFault);
            }
            if ((status = nc_inq_vardimid(ncid, id, dimids)) != NC_NOERR) {
                CMmsgPrint(CMmsgAppError, "NC Error '%s' in: %s %d", nc_strerror(status), __FILE__, __LINE__);
                free(vector);
                free(longitudes);
                free(latitudes);
                free(timeSteps);
                nc_close(ncid);
                ut_free(baseTimeUnit);
                ut_free_system(utSystem);
                return (DBFault);
            }
            if (dimids[0] != latdim) {
                CMmsgPrint(CMmsgAppError, "Latitude has invalid dimension in: %s %d", __FILE__, __LINE__);
                free(vector);
                free(longitudes);
                free(latitudes);
                free(timeSteps);
                nc_close(ncid);
                ut_free(baseTimeUnit);
                ut_free_system(utSystem);
                return (DBFault);
            }
            start[0] = 0;
            count[0] = rowNum;
            if ((status = nc_get_vara_double(ncid, id, start, count, latitudes)) != NC_NOERR) {
                CMmsgPrint(CMmsgAppError, "NC Error '%s' in: %s %d", nc_strerror(status), __FILE__, __LINE__);
                free(vector);
                free(longitudes);
                free(latitudes);
                free(timeSteps);
                nc_close(ncid);
                ut_free(baseTimeUnit);
                ut_free_system(utSystem);
                return (DBFault);
            }
            cellSize.Y = fabs(latitudes[0] - latitudes[1]);
            extent.LowerLeft.Y = latitudes[0] < latitudes[1] ? latitudes[0] : latitudes[1];
            extent.UpperRight.Y = latitudes[0] > latitudes[1] ? latitudes[0] : latitudes[1];
            for (i = 2; i < rowNum; i++) {
                extent.LowerLeft.Y = extent.LowerLeft.Y < latitudes[i] ? extent.LowerLeft.Y : latitudes[i];
                extent.UpperRight.Y = extent.UpperRight.Y > latitudes[i] ? extent.UpperRight.Y : latitudes[i];
                if (CMmathEqualValues(cellSize.Y, fabs(latitudes[i - 1] - latitudes[i])) != true) {
                    CMmsgPrint(CMmsgAppError, "Latitude has irregular spacing in: %s %d", __FILE__, __LINE__);
                    free(vector);
                    free(longitudes);
                    free(latitudes);
                    free(timeSteps);
                    nc_close(ncid);
                    ut_free(baseTimeUnit);
                    ut_free_system(utSystem);
                    return (DBFault);
                }
            }
            extent.LowerLeft.Y = extent.LowerLeft.Y - cellSize.Y / 2.0;
            extent.UpperRight.Y = extent.UpperRight.Y + cellSize.Y / 2.0;
        }
        else if (strcmp(name, "time") == 0) {
            if (((status = nc_inq_attlen(ncid, id, "units", &attlen)) != NC_NOERR) ||
                ((status = nc_get_att_text(ncid, id, "units", timeString)) != NC_NOERR)) {
                CMmsgPrint(CMmsgAppError, "NC Error '%s' in: %s %d", nc_strerror(status), __FILE__, __LINE__);
                free(vector);
                free(longitudes);
                free(latitudes);
                free(timeSteps);
                nc_close(ncid);
                ut_free(baseTimeUnit);
                ut_free_system(utSystem);
                return (DBFault);
            }
            else timeString[attlen] = '\0';
            if ((status = nc_inq_varndims(ncid, id, &ndims)) != NC_NOERR) {
                CMmsgPrint(CMmsgAppError, "NC Error '%s' in: %s %d", nc_strerror(status), __FILE__, __LINE__);
                free(vector);
                free(longitudes);
                free(latitudes);
                free(timeSteps);
                nc_close(ncid);
                ut_free(baseTimeUnit);
                ut_free_system(utSystem);
                return (DBFault);
            }
            if (ndims != 1) {
                CMmsgPrint(CMmsgAppError, "Time has more than one dimension in: %s %d", __FILE__, __LINE__);
                free(vector);
                free(longitudes);
                free(latitudes);
                free(timeSteps);
                nc_close(ncid);
                ut_free(baseTimeUnit);
                ut_free_system(utSystem);
                return (DBFault);
            }
            if ((status = nc_inq_vardimid(ncid, id, dimids)) != NC_NOERR) {
                CMmsgPrint(CMmsgAppError, "NC Error '%s' in: %s %d", nc_strerror(status), __FILE__, __LINE__);
                free(vector);
                free(longitudes);
                free(latitudes);
                free(timeSteps);
                nc_close(ncid);
                ut_free(baseTimeUnit);
                ut_free_system(utSystem);
                return (DBFault);
            }
            if (dimids[0] != timedim) {
                CMmsgPrint(CMmsgAppError, "Time has invalid dimension in: %s %d", __FILE__, __LINE__);
                free(vector);
                free(longitudes);
                free(latitudes);
                free(timeSteps);
                nc_close(ncid);
                ut_free(baseTimeUnit);
                ut_free_system(utSystem);
                return (DBFault);
            }
            start[0] = 0;
            count[0] = layerNum;
            if ((status = nc_get_vara_double(ncid, id, start, count, timeSteps)) != NC_NOERR) {
                CMmsgPrint(CMmsgAppError, "NC Error '%s' in: %s %d", nc_strerror(status), __FILE__, __LINE__);
                free(vector);
                free(longitudes);
                free(latitudes);
                free(timeSteps);
                nc_close(ncid);
                ut_free(baseTimeUnit);
                ut_free_system(utSystem);
                return (DBFault);
            }
        }
        else if (varid == -1) {
            strcpy(varname, name);
            if ((status = nc_inq_varndims(ncid, id, &ndims)) != NC_NOERR) {
                CMmsgPrint(CMmsgAppError, "NC Error '%s' in: %s %d", nc_strerror(status), __FILE__, __LINE__);
                free(vector);
                free(longitudes);
                free(latitudes);
                free(timeSteps);
                nc_close(ncid);
                ut_free(baseTimeUnit);
                ut_free_system(utSystem);
                return (DBFault);
            }
            if ((ndims < 2) || (ndims > 4)) continue;
            if (((status = nc_inq_attlen(ncid, id, "long_name", &attlen)) != NC_NOERR) ||
                ((status = nc_get_att_text(ncid, id, "long_name", longName)) != NC_NOERR))
                strcpy(longName, "Noname");
            else longName[attlen] = '\0';
/*			if (((status = nc_inq_attlen   (ncid,id,"units", &attlen)) != NC_NOERR) ||
			    ((status = nc_get_att_text (ncid,id,"units", varUnit)) != NC_NOERR))
				{
				CMmsgPrint (CMmsgAppError, "NC Error [%s,units] '%s' in: %s %d", nc_strerror(status),varname,__FILE__,__LINE__);
				free (vector);
				free (longitudes);
				free (latitudes);
				free (timeSteps);
				nc_close (ncid);
				ut_free (baseTimeUnit);
				ut_free_system (utSystem);
				return (DBFault);
				}
			else varUnit [attlen] = '\0';
			TODO: Handle variable unit!
*/
          if ((status = nc_get_att_double(ncid, id, "_FillValue", &fillValue)) != NC_NOERR) fillValue = -9999.0;
            if (((status = nc_get_att_double(ncid, id, "missing_value", &missingValue)) != NC_NOERR) ||
                CMmathEqualValues(fillValue, missingValue))
                missingValue = -9999.0; // TODO I am not sure if it is a good idea.
            if ((status = nc_get_att_double(ncid, id, "scale_factor", &scaleFactor)) != NC_NOERR) scaleFactor = 1.0;
            if ((status = nc_get_att_double(ncid, id, "add_offset", &dataOffset)) != NC_NOERR) dataOffset = 0.0;
            varid = id;
        }
    }

    if ((rowNum < 1) || (colNum < 1) || (layerNum < 1)) {
        CMmsgPrint(CMmsgAppError, "Incomplete NetCDF file: %s %d", __FILE__, __LINE__);
        free(vector);
        free(longitudes);
        free(latitudes);
        free(timeSteps);
        nc_close(ncid);
        ut_free(baseTimeUnit);
        ut_free_system(utSystem);
        return (DBFault);
    }
    data->Name(longName);

    if ((status = nc_inq_varndims(ncid, varid, &ndims)) != NC_NOERR) {
        CMmsgPrint(CMmsgAppError, "NC Error '%s' in: %s %d", nc_strerror(status), __FILE__, __LINE__);
        free(vector);
        free(longitudes);
        free(latitudes);
        free(timeSteps);
        nc_close(ncid);
        ut_free(baseTimeUnit);
        ut_free_system(utSystem);
        return (DBFault);
    }
    if ((status = nc_inq_vardimid(ncid, varid, dimids)) != NC_NOERR) {
        CMmsgPrint(CMmsgAppError, "NC Error '%s' in: %s %d", nc_strerror(status), __FILE__, __LINE__);
        free(vector);
        free(longitudes);
        free(latitudes);
        free(timeSteps);
        nc_close(ncid);
        ut_free(baseTimeUnit);
        ut_free_system(utSystem);
        return (DBFault);
    }

    for (id = 0; id < ndims; id++) {
        start[id] = 0;
        if (dimids[id] == londim) { count[id] = colNum; }
        else if (dimids[id] == latdim) {
            count[id] = 1;
            latidx = id;
        }
        else if (dimids[id] == timedim) {
            count[id] = 1;
            timeidx = id;
        }
        else if (dimids[id] == levdim) { count[id] = 1; }
    }

    int netcdfdims;
    int netcdfvars;
    int netcdfngatts;
    int recdim;
    status = nc_inq(ncid, &netcdfdims, &netcdfvars, &netcdfngatts, &recdim);

    data->Extent(extent);
    if (timedim != -1) {
        for (i = 0; i < (int) strlen(timeString); ++i) timeString[i] = (int) tolower((int) timeString[i]);
        if ((timeUnit = ut_parse(utSystem, timeString, UT_ASCII)) == (ut_unit *) NULL) {
            CMmsgPrint(CMmsgAppError, "Time string [%s] parsing error in: %s %d", timeString, __FILE__, __LINE__);
            switch (ut_get_status()) {
                case UT_BAD_ARG:
                    CMmsgPrint(CMmsgAppError, "System or string is NULL!");
                    break;
                case UT_SYNTAX:
                    CMmsgPrint(CMmsgAppError, "String contained a syntax error!");
                    break;
                case UT_UNKNOWN:
                    CMmsgPrint(CMmsgAppError, "String contained an unknown identifier!");
                    break;
                default:
                    CMmsgPrint(CMmsgAppError, "System error in %s:%d!", __FILE__, __LINE__);
            }
            ut_free(baseTimeUnit);
            ut_free_system(utSystem);
            return (DBFault);
        }
        if ((cvConverter = ut_get_converter(timeUnit, baseTimeUnit)) == (cv_converter *) NULL) {
            CMmsgPrint(CMmsgAppError, "Time converter error!");
            switch (ut_get_status()) {
                case UT_BAD_ARG:
                    CMmsgPrint(CMmsgAppError, "unit1 or unit2 is NULL.");
                    break;
                case UT_NOT_SAME_SYSTEM:
                    CMmsgPrint(CMmsgAppError, "unit1 and unit2 belong to different unit-systems.");
                    break;
                default:
                    CMmsgPrint(CMmsgAppError, "Conversion between the units is not possible.");
                    break;
            }
            ut_free(timeUnit);
            ut_free(baseTimeUnit);
            ut_free_system(utSystem);
            return (DBFault);
        }
        doTimeUnit = true;
    }
    else doTimeUnit = false;
    gridIF = new DBGridIF(data);
    for (layerID = 0; layerID < layerNum; layerID++) {
        if (timedim != -1) {
            start[timeidx] = layerID;
            if (doTimeUnit) {
                ut_decode_time(cv_convert_double(cvConverter, timeSteps[layerID]), &year, &month, &day, &hour, &minute,
                               &second, &resolution);
                if (year > 1) snprintf(layerName, sizeof(layerName), "%04d", year); else snprintf(layerName, sizeof(layerName), "XXXX");
                if (strncmp(timeString, "month", strlen("month")) == 0)
                    snprintf(layerName + strlen(layerName), sizeof(layerName), "-%02d", month + (day > 15 ? 1 : 0));
                else if (strncmp(timeString, "day", strlen("day")) == 0)
                    snprintf(layerName + strlen(layerName), sizeof(layerName), "-%02d-%02d", month, day + (hour > 12 ? 1 : 0));
                else if (strncmp(timeString, "hour", strlen("hour")) == 0)
                    snprintf(layerName + strlen(layerName), sizeof(layerName), "-%02d-%02d %02d", month, day, hour + (minute > 30 ? 1 : 0));
                else if (strncmp(timeString, "minute", strlen("minute")) == 0)
                    snprintf(layerName + strlen(layerName), sizeof(layerName), "-%02d-%02d %02d:%02d", month, day, hour,
                            minute + (second > 30 ? 1 : 0));
                else
                    snprintf(layerName, sizeof(layerName), "LayerName:%04d", layerID);
            }
            else snprintf(layerName, sizeof(layerName), "LayerName:%04d", layerID);
        }
        else snprintf(layerName, sizeof(layerName), "LayerName:%04d", layerID);

        if ((layerRec = layerTable->Add(layerName)) == (DBObjRecord *) NULL) {
            free(vector);
            free(longitudes);
            free(latitudes);
            free(timeSteps);
            nc_close(ncid);
            cv_free(cvConverter);
            ut_free(timeUnit);
            ut_free(baseTimeUnit);
            ut_free_system(utSystem);
            return (DBFault);
        }
        itemRec = itemTable->Add(layerRec->Name());
        missingValueFLD->Float(itemRec, (DBFloat) missingValue);
        rowNumFLD->Int(layerRec, rowNum);
        colNumFLD->Int(layerRec, colNum);
        cellWidthFLD->Float(layerRec, cellSize.X);
        cellHeightFLD->Float(layerRec, cellSize.Y);
        valueTypeFLD->Int(layerRec, DBTableFieldFloat);
        valueSizeFLD->Int(layerRec, sizeof(DBFloat4));
        layerFLD->Record(layerRec, dataRec = new DBObjRecord(layerName, (size_t) colNum * (size_t) rowNum,sizeof(DBFloat4)));
        (data->Arrays())->Add(dataRec);

        sumWeight = 0.0;
        average   = 0.0;
        minimum   =  DBHugeVal;
        maximum   = -DBHugeVal;
        stdDev    = 0.0;
        for (pos.Row = 0; pos.Row < rowNum; pos.Row++) {
            start[latidx] = latitudes[0] < latitudes[1] ? rowNum - pos.Row - 1 : pos.Row;
            if ((status = nc_get_vara_double(ncid, varid, start, count, vector)) != NC_NOERR) {
                CMmsgPrint(CMmsgAppError, "NC Error '%s' in: %s %d", nc_strerror(status), __FILE__, __LINE__);
                free(vector);
                free(longitudes);
                free(latitudes);
                free(timeSteps);
                nc_close(ncid);
                cv_free(cvConverter);
                ut_free(timeUnit);
                ut_free(baseTimeUnit);
                ut_free_system(utSystem);
                return (DBFault);
            }
            pos.Col = 0;
            cellArea = gridIF->CellArea(pos);
            if (longitudes[0] < longitudes[1]) {
                for (pos.Col = 0; pos.Col < colNum; pos.Col++) {
                    value = vector[pos.Col];
                    if (isnan (value) || CMmathEqualValues(value, fillValue)) value = missingValue;
                    else {
                        value = scaleFactor * value + dataOffset;
                        sumWeight += cellArea;
                        average = average + value * cellArea;
                        minimum = minimum < value ? minimum : value;
                        maximum = maximum > value ? maximum : value;
                        stdDev = stdDev + value * value * cellArea;
                    }
                    ((float *) (dataRec->Data()))[(size_t) colNum * (size_t) pos.Row + (size_t) pos.Col] = value;
                }
            }
            else {
                for (pos.Col = colNum - 1; pos.Col >= 0; pos.Col--) {
                    value = vector[pos.Col];
                    if (isnan (value) || CMmathEqualValues(value, fillValue)) value = missingValue;
                    else {
                        value = scaleFactor * value + dataOffset;
                        sumWeight += cellArea;
                        average = average + value * cellArea;
                        minimum = minimum < value ? minimum : value;
                        maximum = maximum > value ? maximum : value;
                        stdDev = stdDev + value * value * cellArea;
                    }
                    ((float *) (dataRec->Data()))[(size_t) colNum * (size_t) pos.Row + (size_t) pos.Col] = value;
                }
            }
        }
        if (sumWeight > 0.0) {
            average = average / sumWeight;
            stdDev = stdDev / sumWeight;
            stdDev = stdDev - average * average;
            stdDev = stdDev > 0.0 ? sqrt (stdDev) : 0.0;
        }
        else average = stdDev = minimum = maximum = missingValue;

        averageFLD->Float(itemRec, average);
        minimumFLD->Float(itemRec, minimum);
        maximumFLD->Float(itemRec, maximum);
        stdDevFLD->Float(itemRec, stdDev);
    }
    data->Document(DBDocSubject, varname);
    free(vector);
    free(longitudes);
    free(latitudes);
    free(timeSteps);
    nc_close(ncid);
    cv_free(cvConverter);
    ut_free(timeUnit);
    ut_free(baseTimeUnit);
    ut_free_system(utSystem);
    return (DBSuccess);
}
