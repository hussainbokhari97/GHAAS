/******************************************************************************

GHAAS RiverGIS V3.0
Global Hydrological Archive and Analysis System
Copyright 1994-2024, UNH - CCNY

RGISTools.cpp

bfekete@ccny.cuny.edu

*******************************************************************************/

#include <string.h>
#include <rgis.hpp>

static void _RGISToolsImportASCIITableCBK (Widget widget,RGISWorkspace *workspace,XmAnyCallbackStruct *callData)

	{
	static Widget fileSelect = (Widget) NULL;
	DBObjData *data = new DBObjData ("",DBTypeTable);

	if (fileSelect == NULL) fileSelect = UIFileSelectionCreate ((char *) "ASCII Import",NULL,(char *) "*.*",XmFILE_REGULAR);

	if (UIDataHeaderForm (data))
		{
		char *fileName;

		if ((fileName = UIFileSelection (fileSelect,true)) == NULL) { delete data; return; }

		if (DBImportASCIITable (data->Table (DBrNItems),fileName) == DBFault)
			{ UIMessage ((char *) "Import failed!"); delete data; return; }

		if (UITableRedefineFields (data->Table (DBrNItems)) != true)	{ delete data;  return; }

		workspace->CurrentData  (data);
		}
	else delete data;
	}

static void _RGISToolsImportARCInfoCBK (Widget widget,RGISWorkspace *workspace,XmAnyCallbackStruct *callData)

	{
	DBObjData *data = new DBObjData ("",DBTypeVector);

	if (UIDataHeaderForm (data) && (_RGISARCInfoImport (data) == DBSuccess))
		workspace->CurrentData  (data);
	else delete data;
	}

static void _RGISToolsImportASCIINetCBK (Widget widget,RGISWorkspace *workspace,XmAnyCallbackStruct *callData)

	{
	DBObjData *data = new DBObjData ("",DBTypeNetwork);
	static Widget fileSelect = NULL;

	if (fileSelect == NULL) fileSelect = UIFileSelectionCreate ((char *) "GHAAS Data Import",NULL,(char *) "*",XmFILE_REGULAR);
	data->Document (DBDocSubject,"STNetwork");
	if (UIDataHeaderForm (data))
		{
		char *fileName;

		if ((fileName = UIFileSelection (fileSelect,true)) == NULL) { delete data; return; }
		UIPauseDialogOpen ((char *) "Importing Networks");
		if (DBImportASCIINet (data,fileName) == DBFault)	delete data;
		else workspace->CurrentData (data);
		UIPauseDialogClose ();
		}
	else delete data;
	}

static void _RGISToolsExportASCIICBK (Widget widget,RGISWorkspace *workspace,XmAnyCallbackStruct *callData)

	{
	char *selection;
	DBDataset *dataset = UIDataset ();
	DBObjData *data = dataset->Data ();
	DBObjTable *table;
	static Widget fileSelect = NULL;
	static Widget tableSelect = (Widget) NULL;

	if (tableSelect == (Widget) NULL) tableSelect = UISelectionCreate ((char *) "Table Selection");
	if (fileSelect == NULL)	fileSelect = UIFileSelectionCreate ((char *) "ASCII Table",NULL,(char *) "*.txt",XmFILE_REGULAR);

	table = data->Table (UISelectObject (tableSelect,(DBObjectLIST<DBObject> *) data->Tables ()));
	if (table == (DBObjTable *) NULL) return;
	if ((selection = UIFileSelection (fileSelect,False)) == NULL) return;

	DBExportASCIITable (table, selection);
	}

int _RGISToolsPointExportARCInfo   (DBObjData *,char *);
int _RGISToolsLineExportARCInfo    (DBObjData *,char *);
int _RGISToolsGridExportARCInfo    (DBObjData *,char *);
int _RGISToolsNetworkExportARCInfo (DBObjData *,char *);

static void _RGISToolsExportARCInfoCBK (Widget widget,RGISWorkspace *workspace,XmAnyCallbackStruct *callData)

	{
	char *selection;
	DBDataset *dataset = UIDataset ();
	DBObjData *data = dataset->Data ();
	static Widget dirSelect = NULL;

	if (dirSelect == NULL)
		{ if ((dirSelect = UIFileSelectionCreate ((char *) "ARC/INFO Workspace",NULL,(char *) "w_*",XmFILE_DIRECTORY)) == NULL) return; }
	if ((selection = UIFileSelection (dirSelect,True)) == NULL) return;
	switch (data->Type ())
		{
		case DBTypeVectorPoint: 			_RGISToolsPointExportARCInfo (data,selection);	break;
		case DBTypeVectorLine:				_RGISToolsLineExportARCInfo  (data,selection);	break;
		case DBTypeVectorPolygon:			break;
		case DBTypeGridDiscrete:
		case DBTypeGridContinuous:			_RGISToolsGridExportARCInfo  (data,selection);	break;
		case DBTypeNetwork: 				_RGISToolsNetworkExportARCInfo (data,selection); break;
		case DBTypeTable:					break;
		}
	}

static void _RGISToolsExportDMCBK (Widget widget,RGISWorkspace *workspace,XmAnyCallbackStruct *callData)

	{
	char *selection;
	DBDataset *dataset = UIDataset ();
	DBObjData *data = dataset->Data ();
	static Widget dirSelect = NULL;

	if (dirSelect == NULL)
		{ if ((dirSelect = UIFileSelectionCreate ((char *) "DM Coverage",NULL,(char *) "M:*",XmFILE_REGULAR)) == NULL) return; }
	if ((selection = UIFileSelection (dirSelect,False)) == NULL) return;

	switch (data->Type ())
		{
		case DBTypeGridDiscrete:
		case DBTypeGridContinuous:	DBExportDMGrid (data,selection);	break;
		}
	}

static void _RGISToolsExportNetworkCBK (Widget widget,RGISWorkspace *workspace,XmAnyCallbackStruct *callData)

	{
	char *selection;
	FILE *outFILE;
	DBInt cellID, fieldID, fieldNum = 0;
	DBCoordinate coord;
	DBObjRecord *cellRec,*toCellRec, *basinRec;
	DBDataset *dataset = UIDataset ();
	DBObjData *netData = dataset->Data ();
	DBNetworkIF *netIF = new DBNetworkIF (netData);
	DBObjTable *cellTable = netData->Table (DBrNCells);
	DBObjTableField *field, **fields = (DBObjTableField **) NULL;
	static Widget dirSelect = NULL;

	if (dirSelect == NULL)
		{ if ((dirSelect = UIFileSelectionCreate ((char *) "Network File",NULL,(char *) "*.txt",XmFILE_REGULAR)) == NULL) return; }
	if ((selection = UIFileSelection (dirSelect,False)) == NULL) return;

	if ((outFILE = fopen (selection,"w")) == (FILE *) NULL)
		{ CMmsgPrint (CMmsgSysError, "File Opening Error in: %s %d",__FILE__,__LINE__); return; }
	fprintf (outFILE,"\"CellID\"\t\"XCoord\"\t\"YCoord\"\t\"BasinID\"\t\"ToCell\"\t\"CellArea\"\t\"CellLength\"");
	for (fieldID = 0;fieldID < cellTable->FieldNum (); fieldID++)
		{
		if ((field = cellTable->Field (fieldID)) == (DBObjTableField *) NULL)
			{
			if (fields != (DBObjTableField **) NULL) free (fields);
			CMmsgPrint (CMmsgSysError, "Invalid field in: %s %d",__FILE__,__LINE__);
			return;
			}
		if (field->Required ()) continue;
		if ((fields = (DBObjTableField **) realloc (fields,(fieldNum + 1) * sizeof (DBObjTableField *))) == (DBObjTableField **) NULL)
			{ CMmsgPrint (CMmsgSysError, "Memory allocation error in: %s %d",__FILE__,__LINE__); return; }
		fields [fieldNum] = field;
		fprintf (outFILE,"\t\"%s\"",field->Name ());
		fieldNum++;
		}
	fprintf (outFILE,"\n");

	for (cellID = 0;cellID < netIF->CellNum (); cellID++)
		{
		cellRec = netIF->Cell (cellID);
		coord = netIF->Center (cellRec);
		basinRec = netIF->Basin (cellRec);
		toCellRec = netIF->ToCell (cellRec);
		fprintf (outFILE,"%d\t%f\t%f\t%d\t%d\t%f\t%f",cellRec->RowID () + 1,
														  coord.X, coord.Y,
														  basinRec->RowID () + 1,
														  toCellRec == (DBObjRecord *) NULL ? DBFault : toCellRec->RowID () + 1,
														  netIF->CellArea (cellRec),
														  netIF->CellLength (cellRec));
		for (fieldID = 0;fieldID < fieldNum; fieldID++)
			switch	(fields [fieldID]->Type ())
				{
				default:
				case DBTableFieldString: fprintf (outFILE,"%c\"%s\"",DBASCIISeparator,fields [fieldID]->String (cellRec)); break;
				case DBTableFieldInt:
					if (fields [fieldID]->Int (cellRec) == fields [fieldID]->IntNoData ())
						fprintf (outFILE,"%c",DBASCIISeparator);
					else	fprintf (outFILE,"%c%d",DBASCIISeparator,fields [fieldID]->Int (cellRec));
					break;
				case DBTableFieldFloat:
					if (CMmathEqualValues (fields [fieldID]->Float (cellRec),fields [fieldID]->FloatNoData ()))
						fprintf (outFILE,"%c",DBASCIISeparator);
					else	fprintf (outFILE,"%c%f",DBASCIISeparator,fields [fieldID]->Float (cellRec));
					break;
				case DBTableFieldDate:   fprintf (outFILE,"%c\"%s\"",DBASCIISeparator,fields [fieldID]->String(cellRec)); break;
				}
		fprintf (outFILE,"\n");
		}
	fclose (outFILE);
	}

static void _RGISToolsDuplicateCBK (Widget widget,RGISWorkspace *workspace,XmAnyCallbackStruct *callData)

	{
	DBDataset *dataset = UIDataset ();
	DBObjData *dbData = dataset->Data (), *newData;

	if (dbData == (DBObjData *) NULL) return;

	if (UIDataHeaderForm (newData = new DBObjData (*dbData)))
		workspace->CurrentData  (newData);
	else delete newData;
	}


static void _RGISToolsNetBasinMouthCBK (Widget widget,RGISWorkspace *workspace,XmAnyCallbackStruct *callData)

	{
	DBDataset *dataset = UIDataset ();
	DBObjData *netData = dataset->Data (), *pntData;
	DBNetworkIF *netIF;

	if (netData == (DBObjData *) NULL)
		{ CMmsgPrint (CMmsgAppError, "Null Data in: %s %d",__FILE__,__LINE__); return; }
	netIF = new DBNetworkIF (netData);

	if (UIDataHeaderForm (pntData = new DBObjData ("",DBTypeVectorPoint)))
		{
		char symName [DBStringLength];
		DBInt basinID, order;
		DBCoordinate coord;
		DBObjTable *items 	= pntData->Table (DBrNItems);
		DBObjTable *symbols	= pntData->Table (DBrNSymbols);
		DBObjTableField *coordField = items->Field (DBrNCoord);
		DBObjTableField *symbolFLD	 = items->Field (DBrNSymbol);
		DBObjTableField *orderFLD 	= new DBObjTableField (DBrNOrder,DBTableFieldInt,"%3d",sizeof (DBByte));
		DBObjTableField *subbasinLengthFLD = new DBObjTableField (DBrNSubbasinLength,DBTableFieldFloat,"%10.1f",sizeof (float));
		DBObjTableField *subbasinAreaFLD = new DBObjTableField (DBrNSubbasinArea,DBTableFieldFloat,"%10.1f",sizeof (float));
		DBObjTableField *foregroundFLD = symbols->Field (DBrNForeground);
		DBObjTableField *backgroundFLD = symbols->Field (DBrNBackground);
		DBObjTableField *styleFLD = symbols->Field (DBrNStyle);
		DBObjRecord *pntRec, *symRec, *cellRec, *basinRec;
		DBRegion dataExtent;

		items->AddField (orderFLD);
		items->AddField (subbasinLengthFLD);
		items->AddField (subbasinAreaFLD);

		cellRec = netIF->Cell ((DBInt) 0);
		for (order = 0;order <= netIF->CellOrder (cellRec);++order)
			{
			snprintf (symName, sizeof(symName), "Strahler Order:%2d",order);
			symRec = symbols->Add (symName);
			styleFLD->Int (symRec,0);
			foregroundFLD->Int (symRec,1);
			backgroundFLD->Int (symRec,0);
			}

		UIPauseDialogOpen ((char *) "Creating Basin Mouth");
		for (basinID = 0;basinID < netIF->BasinNum ();++basinID)
			{
			basinRec = netIF->Basin (basinID);
			if (UIPause (basinID * 100 / netIF->BasinNum ())) goto Stop;

			symRec = symbols->Item (netIF->CellOrder (cellRec));
			cellRec = netIF->MouthCell (basinRec);
			pntRec = items->Add (basinRec->Name ());
			coord = netIF->Center (cellRec);
			coordField->Coordinate (pntRec,coord);
			symbolFLD->Record (pntRec,symRec);
			orderFLD->Int (pntRec,netIF->CellOrder (cellRec));
			subbasinLengthFLD->Float (pntRec,netIF->CellBasinLength (cellRec));
			subbasinAreaFLD->Float (pntRec,netIF->CellBasinArea (cellRec));
			dataExtent.Expand (coord);
			}
Stop:
		UIPauseDialogClose ();
		pntData->Extent (dataExtent);
		workspace->CurrentData  (pntData);
		}
	else	delete pntData;
	delete netIF;
	}

void RGISToolsImportGridCBK (Widget,RGISWorkspace *,XmAnyCallbackStruct *);
void RGISToolsImportGridDMCBK (Widget,RGISWorkspace *,XmAnyCallbackStruct *);

static UIMenuItem _RGISToolsImportMenu [] = {
	UIMenuItem ((char *) "ASCII Table",				UIMENU_NORULE,	UIMENU_NORULE,		(UIMenuCBK)  _RGISToolsImportASCIITableCBK,	(char *) "RGIS22MenuSystem.html#Tools_Import_ASCII"),
	UIMenuItem ((char *) "ARC/Info (Vector)",		UIMENU_NORULE,	UIMENU_NORULE,		(UIMenuCBK) _RGISToolsImportARCInfoCBK,		(char *) "RGIS22MenuSystem.html#Tools_Import_ARCINFO"),
	UIMenuItem ((char *) "Grid Matrix",				UIMENU_NORULE,	UIMENU_NORULE,		(UIMenuCBK) RGISToolsImportGridCBK,				(char *) "RGIS22MenuSystem.html#Tools_Import_GridI"),
	UIMenuItem ((char *) "DM (Grid)",				UIMENU_NORULE,	UIMENU_NORULE,		(UIMenuCBK) RGISToolsImportGridDMCBK,			(char *) "RGIS22MenuSystem.html#Tools_Import_DM"),
	UIMenuItem ((char *) "Network Grid",			UIMENU_NORULE,	UIMENU_NORULE,		(UIMenuCBK) _RGISToolsImportASCIINetCBK,		(char *) "RGIS22MenuSystem.html#Tools_Import_Network"),
	UIMenuItem ()};

static UIMenuItem _RGISToolsExportMenu [] = {
	UIMenuItem ((char *) "ASCII",						UIMENU_NORULE,	UIMENU_NORULE,		(UIMenuCBK) _RGISToolsExportASCIICBK,			(char *) "RGIS22MenuSystem.html#Tools_Export_ASCII"),
	UIMenuItem ((char *) "ARC/Info",					UIMENU_NORULE,	UIMENU_NORULE,		(UIMenuCBK) _RGISToolsExportARCInfoCBK,		(char *) "RGIS22MenuSystem.html#Tools_Import_ARCINFO"),
	UIMenuItem ((char *) "DM",							UIMENU_NORULE,	UIMENU_NORULE,		(UIMenuCBK) _RGISToolsExportDMCBK,				(char *) "RGIS22MenuSystem.html#Tools_Export_DM"),
	UIMenuItem ((char *) "Network",					RGISNetworkGroup,	UIMENU_NORULE,	(UIMenuCBK) _RGISToolsExportNetworkCBK,		(char *) "RGIS22MenuSystem.html#Tools_Export_Network"),
	UIMenuItem ()};

extern void RGISToolsConvertToPointCBK (Widget,RGISWorkspace *,XmAnyCallbackStruct *);

UIMenuItem RGISToolsMenu [] = {
	UIMenuItem ((char *) "Import",					UIMENU_NORULE,	UIMENU_NORULE,		_RGISToolsImportMenu),
	UIMenuItem ((char *) "Export",					UIMENU_NORULE,	RGISDataGroup,		_RGISToolsExportMenu),
	UIMenuItem (RGISDataGroup,	UIMENU_NORULE),
	UIMenuItem ((char *) "Duplicate",				RGISDataGroup,	UIMENU_NORULE,		(UIMenuCBK) _RGISToolsDuplicateCBK,				(char *) "RGIS22MenuSystem.html#Tools_Duplicate"),
	UIMenuItem ((char *) "Convert to Point",		RGISDataGroup,	UIMENU_NORULE,		(UIMenuCBK) RGISToolsConvertToPointCBK,		(char *) "RGIS22MenuSystem.html#Tools_Convert2Point"),
	UIMenuItem ((char *) "Create Basin Mouth",	RGISNetworkGroup,	UIMENU_NORULE,	(UIMenuCBK) _RGISToolsNetBasinMouthCBK,		(char *) "RGIS22MenuSystem.html#Tools_CreateBasinMouth"),
	UIMenuItem ()};
