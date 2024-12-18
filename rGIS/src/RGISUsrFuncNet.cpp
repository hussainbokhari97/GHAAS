/******************************************************************************

GHAAS RiverGIS V3.0
Global Hydrological Archive and Analysis System
Copyright 1994-2024, UNH - CCNY

RGISUsrFuncNet.cpp

bfekete@ccny.cuny.edu

*******************************************************************************/

#include <Xm/Xm.h>
#include <rgis.hpp>

void _RGISUserFuncionNetwork (DBObjData *data,UI2DView *view,XEvent *event)

	{
	DBInt sX, sY, redraw = false;
	DBNetworkIF *netIF;
	DBCoordinate coord;
	DBObjRecord *cellRec, *basinRec;
	void _RGISUserFuncionQuery (DBObjData *,UI2DView *,XEvent *);

	if ((data->Flags () & DBDataFlagUserModeFlags) == DBDataFlagUserModeQuery)
		{ _RGISUserFuncionQuery (data,view,event);	return; }

	if (event->type != ButtonPress) return;
	if (DBTypeNetwork != data->Type ())
		{ CMmsgPrint (CMmsgAppError, "Invalid data Type in: %s %d",__FILE__,__LINE__); }

	sX = event->xbutton.x;
	sY = event->xbutton.y;
	view->Window2Map  (sX,sY, &coord.X, &coord.Y);

	netIF = new DBNetworkIF (data);
	switch (data->Flags () & DBDataFlagUserModeFlags)
		{
		case DBDataFlagUserModeSelect:
			{
			DBInt basinID, cellID;
			DBRegion extent;
			UITable *tableView;

			if ((cellRec = netIF->Cell (coord)) == (DBObjRecord *) NULL)
				{ UIMessage ((char *) "Cell Does not Exists!"); return; }
			for (basinID = 0;basinID < netIF->BasinNum ();++basinID)
				{
				basinRec = netIF->Basin (basinID);
				if ((basinRec->Flags () & DBObjectFlagSelected) == DBObjectFlagSelected)
					{
					data->SelectObject (basinRec,DBClear);
					extent.Expand (data->Extent (basinRec));
					}
				}
			for (cellID = 0;cellID < netIF->CellNum ();++cellID)
				{
				cellRec = netIF->Cell (cellID);
				if ((cellRec->Flags () & DBObjectFlagSelected) == DBObjectFlagSelected)
					{
					extent.Expand (netIF->Center (cellRec) + (netIF->CellSize () / 2.0));
					extent.Expand (netIF->Center (cellRec) - (netIF->CellSize () / 2.0));
					cellRec->Flags (DBObjectFlagSelected,DBClear);
					}
				}
			if ((tableView = (UITable *) data->Display (UITableName (data,data->Table (DBrNItems)))) != (UITable *) NULL)
				tableView->Draw ();
			if ((tableView = (UITable *) data->Display (UITableName (data,data->Table (DBrNCells)))) != (UITable *) NULL)
				tableView->Draw ();
			UI2DViewRedrawAll (extent);
			cellRec = netIF->Cell (coord);
			switch (data->Flags () & DBDataFlagSelectMode)
				{
				case DBDataFlagSelectMode:	netIF->DownStreamSearch	(cellRec, DBNetworkSelect);	break;
				default:					netIF->UpStreamSearch	(cellRec, DBNetworkSelect);	break;
				}
			extent.Initialize ();
			for (cellID = 0;cellID < netIF->CellNum ();++cellID)
				{
				cellRec = netIF->Cell (cellID);
				if ((cellRec->Flags () & DBObjectFlagSelected) == DBObjectFlagSelected)
					{
					extent.Expand (netIF->Center (cellRec) + (netIF->CellSize () / 2.0));
					extent.Expand (netIF->Center (cellRec) - (netIF->CellSize () / 2.0));
					}
				}
			UI2DViewRedrawAll (extent);
			if ((tableView = (UITable *) data->Display (UITableName (data,data->Table (DBrNCells)))) != (UITable *) NULL)
				tableView->Draw ();
			} break;
		case DBDataFlagUserModeAdd:
			if (netIF->CellAdd (coord) == (DBObjRecord *) NULL) UIMessage ((char *) "Cell Creation Error");
			else redraw = true;
			break;
		case DBDataFlagUserModeDelete:
			if (netIF->CellDelete (coord) == DBFault) UIMessage ((char *) "Cell Does not Exists!");
			else redraw = true;
			break;
		case DBDataFlagUserModeRotate:
			{
			DBObjRecord *cellRec = netIF->Cell (coord);

			if (cellRec != (DBObjRecord *) NULL)
				{
				switch (netIF->CellDirection (cellRec))
					{
					case DBNull:     netIF->CellDirection (cellRec,DBNetDirN); break;
					case DBNetDirNW: netIF->CellDirection (cellRec,DBNull);    break;
					default:         netIF->CellRotate    (cellRec,DBForward); break;
					}
				redraw = true;
				}
			} break;
		default: printf ("Unknown Mode %lX",data->Flags () & DBDataFlagUserModeFlags); break;
		}
	if (redraw)
		{
		DBPosition pos;

		if (netIF->Coord2Pos (coord,pos) == DBSuccess)
			{
			DBRegion extent;
			DBCoordinate delta (netIF->CellWidth () * 1.25, netIF->CellHeight () * 1.25);

			netIF->Pos2Coord (pos,coord); coord = coord + delta; extent.Expand (coord);
			netIF->Pos2Coord (pos,coord); coord = coord - delta; extent.Expand (coord);
			UI2DViewRedrawAll (extent);
			}
		}
	delete netIF;
	}
