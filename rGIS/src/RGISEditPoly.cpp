/******************************************************************************

GHAAS RiverGIS V3.0
Global Hydrological Archive and Analysis System
Copyright 1994-2024, UNH - CCNY

RGISEditPoly.cpp

bfekete@ccny.cuny.edu

*******************************************************************************/

#include <rgis.hpp>

void RGISEditPolyFourColorCBK (Widget widget, RGISWorkspace *workspace,XmAnyCallbackStruct *callData)

	{
	DBDataset *dataset = UIDataset ();
	DBVPolyIF *polyIF = new DBVPolyIF (dataset->Data ());

	UIPauseDialogOpen ((char *) "Four Coloring Polygons");
	polyIF->FourColoring ();
	UIPauseDialogClose ();
	}
