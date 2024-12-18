/******************************************************************************

GHAAS Water Balance/Transport Model
Global Hydrological Archive and Analysis System
Copyright 1994-2024, UNH - CCNY

MDReservoir_TargetLowFlow.c

bfekete@ccny.cuny.edu

******************************************************************************/

#include <MF.h>
#include <MD.h>

// Input
static int _MDInRouting_DischargeID      = MFUnset;
static int _MDInAux_MeanDischargeID      = MFUnset;
static int _MDInAux_MinDischargeID       = MFUnset;
static int _MDInResCapacityID            = MFUnset;

// Output
static int _MDOutResDeficitID            = MFUnset;
static int _MDOutResAccumDeficitID       = MFUnset;
static int _MDOutResMaxAccumDeficitID    = MFUnset;
static int _MDOutResTargetLowFlowID      = MFUnset;

static void _MDReservoirTargetLowFlow (int itemID) {
// Input
	float discharge;       // Current discharge [m3/s]
	float meanDischarge;   // Long-term mean annual discharge [m3/s]
	float minDischarge;    // Long-term minimum discharge [m3/s]
	float resCapacity;     // Reservoir capacity [km3]
// Output
	float deficit         = 0.0; // Inflow deficit (bellow mean annual discharge [m3/s]
	float accumDeficit    = 0.0; // Accumulated deficit during the season [m3/s]
	float maxAccumDeficit = 0.0; // Maximum accumulated deficit [m3/s]
	float targetLowFlow   = 0.0; // Targeted minimum flow [m3/s]
// Local
	float optResCapacity;  // Reservoir capacity required for fully eliminating low flows;

	if ((resCapacity = MFVarGetFloat (_MDInResCapacityID, itemID, 0.0)) > 0.0) { 
		float dt = MFModelGet_dt (); // Time step length [s]

		 discharge      = MFVarGetFloat (_MDInRouting_DischargeID,   itemID, 0.0);
		 meanDischarge  = MFVarGetFloat (_MDInAux_MeanDischargeID,   itemID, discharge);
		  minDischarge  = MFVarGetFloat (_MDInAux_MinDischargeID,    itemID, discharge);
		  accumDeficit  = MFVarGetFloat (_MDOutResAccumDeficitID,    itemID, discharge);
		maxAccumDeficit = MFVarGetFloat (_MDOutResMaxAccumDeficitID, itemID, discharge);

	    	    deficit = meanDischarge - discharge;
		  accumDeficit  = accumDeficit + deficit > 0.0 ? accumDeficit + deficit : 0.0;
		maxAccumDeficit = accumDeficit > maxAccumDeficit ? accumDeficit : maxAccumDeficit;
	 	optResCapacity = maxAccumDeficit * dt / 1e9;
		  targetLowFlow = minDischarge + (meanDischarge - minDischarge) * resCapacity / (optResCapacity > resCapacity ? optResCapacity : resCapacity);
	}
	MFVarSetFloat (_MDOutResDeficitID,            itemID, deficit);
	MFVarSetFloat (_MDOutResAccumDeficitID,       itemID, accumDeficit);
	MFVarSetFloat (_MDOutResMaxAccumDeficitID,    itemID, maxAccumDeficit);
	MFVarSetFloat (_MDOutResTargetLowFlowID,      itemID, targetLowFlow);
}

int MDReservoir_TargetLowFlowDef () {
	int optID = MFnone;
	const char *optStr;

	if (_MDOutResTargetLowFlowID != MFUnset) return (_MDOutResTargetLowFlowID);
	MFDefEntering ("Target low flow");

	if ((optStr = MFOptionGet ("Reservoir_TargetLowFlow")) != (char *) NULL) optID = CMoptLookup (MFcalcOptions, optStr, true);
 	switch (optID) {
		default:     MFOptionMessage ("Reservoir_TargetLowFlow", optStr, MFsourceOptions); return (CMfailed);
		case MFhelp: MFOptionMessage ("Reservoir_TargetLowFlow", optStr, MFsourceOptions);
		case MFnone: break;
		case MFinput:
			if ((_MDOutResTargetLowFlowID       = MFVarGetID ("Reservoir_TargetLowFlow",      "m3/s", MFInput,  MFState, MFBoundary)) == CMfailed)
				return (CMfailed);
			break;
		case MFcalculate: break;
			if (((_MDInAux_MeanDischargeID      = MDAux_DischargeMeanDef       ()) == CMfailed) ||
			    ((_MDInAux_MinDischargeID       = MDAux_DischargeMinDef        ()) == CMfailed) ||
                ((_MDInRouting_DischargeID      = MDRouting_DischargeUptakeDef ()) == CMfailed) ||
            	((_MDInResCapacityID            = MFVarGetID (MDVarReservoir_Capacity,           "km3",  MFInput,  MFState, MFBoundary)) == CMfailed) ||
                ((_MDOutResDeficitID            = MFVarGetID ("Reservoir_Deficit",               "km3",  MFOutput, MFState, MFBoundary)) == CMfailed) ||
                ((_MDOutResAccumDeficitID       = MFVarGetID ("Reservoir_AccumDeficit",          "m3/s", MFOutput, MFState, MFInitial))  == CMfailed) ||
                ((_MDOutResMaxAccumDeficitID    = MFVarGetID ("Reservoir_MaxAccumDeficit",       "m3/s", MFOutput, MFState, MFInitial))  == CMfailed) ||
                ((_MDOutResTargetLowFlowID      = MFVarGetID ("Reservoir_TargetLowFlow",         "m3/s", MFInput,  MFState, MFBoundary)) == CMfailed) ||
                (MFModelAddFunction (_MDReservoirTargetLowFlow) == CMfailed)) return (CMfailed);
	}
	MFDefLeaving ("Target low flow");
	return (_MDOutResTargetLowFlowID); 
}
