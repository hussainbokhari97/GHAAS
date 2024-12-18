/******************************************************************************

GHAAS RiverGIS V3.0
Global Hydrological Archive and Analysis System
Copyright 1994-2024, UNH - CCNY

RGISAnSampleGrid.cpp

bfekete@ccny.cuny.edu

*******************************************************************************/

#include <Xm/Form.h>
#include <rgis.hpp>

void RGISAnalyseSingleSampleGridCBK (Widget widget, RGISWorkspace *workspace,XmAnyCallbackStruct *callData)

	{

	if (UIYesOrNo ((char *) "You are about to sample grid. Are You sure?"))
		{
		DBDataset *dataset = UIDataset ();
		DBObjData *dbData  = dataset->Data (), *grdData = dbData->LinkedData ();
		DBObjTable *itemTable = dbData->Table (DBrNItems);

		DBInt recID, fieldID, progress = 0, maxProgress;
		DBCoordinate coord;
		DBGridIF *gridIF = new DBGridIF (grdData);
		DBVPointIF  *pntIF = dbData->Type () == DBTypeVectorPoint ? new DBVPointIF (dbData)	 : (DBVPointIF *) NULL;
		DBNetworkIF *netIF = dbData->Type () == DBTypeNetwork		 ? new DBNetworkIF (dbData) : (DBNetworkIF *) NULL;
		DBObjTableField *newField;
		DBObjRecord *record;

		UIPauseDialogOpen ((char *) "Single Layer Grid Sampling");
		switch (grdData->Type ())
			{
			case DBTypeGridContinuous:
				{
				DBFloat value;
				if ((newField = itemTable->Field ((gridIF->Layer ())->Name ())) == (DBObjTableField *) NULL)
					{
					newField = new DBObjTableField ((gridIF->Layer ())->Name (),DBTableFieldFloat,"%10.3f",sizeof (DBFloat4));
					itemTable->AddField (newField);
					}
				maxProgress = itemTable->ItemNum ();
				for (recID = 0;recID < itemTable->ItemNum ();recID++)
					{
					record = itemTable->Item (recID);
					UIPause (progress * 100 / maxProgress); progress++;
					if ((record->Flags () & DBObjectFlagIdle) == DBObjectFlagIdle) continue;
					if (pntIF != (DBVPointIF *) NULL)
							coord = pntIF->Coordinate (record);
					else	coord = netIF->Center (netIF->MouthCell (record));

					if (gridIF->Value (coord,&value))
							newField->Float (record,value);
					else	newField->Float (record,newField->FloatNoData ());
					}
				} break;
			case DBTypeGridDiscrete:
				{
				DBObjTable *grdTable = grdData->Table (DBrNItems);
				DBObjectLIST<DBObjTableField> *fields = grdTable->Fields ();
				DBObjTableField *field;
				DBObjRecord *grdRec;

				if ((newField = itemTable->Field ((gridIF->Layer ())->Name ())) == (DBObjTableField *) NULL)
					{
					newField = new DBObjTableField ((gridIF->Layer ())->Name (),DBTableFieldString,"%s",DBStringLength);
					itemTable->AddField (newField);
					}
				maxProgress = ((fields->ItemNum () + 1) * itemTable->ItemNum ());
				for (recID = 0;recID < itemTable->ItemNum ();recID++)
					{
					record = itemTable->Item (recID);
					UIPause (progress * 100 / maxProgress); progress++;
					if ((record->Flags () & DBObjectFlagIdle) == DBObjectFlagIdle) continue;
					if (pntIF != (DBVPointIF *) NULL)
						coord = pntIF->Coordinate (record);
					else	coord = netIF->Center (netIF->MouthCell (record));
					if ((grdRec = gridIF->GridItem (coord)) != (DBObjRecord *) NULL)
						newField->String (record,grdRec->Name ());
					}
				for (fieldID = 0;fieldID < fields->ItemNum ();fieldID++)
					if (DBTableFieldIsVisible (field = fields->Item (fieldID)))
						{
						if ((newField = itemTable->Field (field->Name ())) == (DBObjTableField *) NULL)
							{
							itemTable->AddField (newField = new DBObjTableField (*field));
							newField->Required (false);
							}
						if (newField->Required ()) continue;
						for (recID = 0;recID < itemTable->ItemNum ();recID++)
							{
							record = itemTable->Item (recID);
							UIPause (progress * 100 / maxProgress); progress++;
							if (pntIF != (DBVPointIF *) NULL)
									coord = pntIF->Coordinate (record);
							else	coord = netIF->Center (netIF->MouthCell (record));

							if ((grdRec = gridIF->GridItem (coord)) != (DBObjRecord *) NULL)
								switch (field->Type ())
									{
									case DBTableFieldString:	newField->String (record,field->String (grdRec));	break;
									case DBTableFieldInt:		newField->Int (record,field->Int (grdRec));			break;
									case DBTableFieldFloat:		newField->Float (record,field->Float (grdRec));		break;
									case DBTableFieldDate:		newField->Date (record,field->Date (grdRec));		break;
									}
							}
						}
				} break;
			}
		if (pntIF != (DBVPointIF *) NULL)  delete pntIF;
		if (netIF != (DBNetworkIF *) NULL) delete netIF;
		delete gridIF;
		UIPauseDialogClose ();
		}
	}

void RGISAnalyseMultiSampleGridCBK (Widget widget, RGISWorkspace *workspace,XmAnyCallbackStruct *callData)

	{
	DBInt ret;
	DBDataset *dataset  = UIDataset ();
	DBObjData *dbData  = dataset->Data ();
	DBObjData *grdData  = dbData->LinkedData ();
	DBObjData *tblData;

	tblData  = new DBObjData ("",DBTypeTable);
	tblData->Document (DBDocGeoDomain,dbData->Document (DBDocGeoDomain));
	tblData->Document (DBDocSubject,grdData->Document (DBDocSubject));

	if (UIDataHeaderForm (tblData) == false) { delete tblData; return; }

	UIPauseDialogOpen ((char *) "Sampling Grid");
	ret = RGlibGridSampling (dbData,grdData,tblData);
	UIPauseDialogClose ();
	if (ret == DBSuccess) workspace->CurrentData  (tblData);
	else delete tblData;
	}
