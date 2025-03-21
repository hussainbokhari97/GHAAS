/******************************************************************************

GHAAS Water Balance/Transport Model
Global Hydrological Archive and Analysis System
Copyright 1994-2024, UNH - CCNY

MDIrrigation_ReferenceET-FAO.c

dominink.wisser@unh.edu

*******************************************************************************/

#include <math.h>
#include <MF.h>
#include <MD.h>

// Input
static int _MDInCommon_AtMeanID     = MFUnset;
static int _MDInAtMaxID             = MFUnset;
static int _MDInAtMinID             = MFUnset;
static int _MDInSolRadID            = MFUnset;
static int _MDInVPressID            = MFUnset;
static int _MDInWSpeedID            = MFUnset;
static int _MDInElevationID         = MFUnset;
static int _MDInDayLengthID         = MFUnset;
static int _MDInI0HDayID            = MFUnset;
// Output
static int _MDOutIrrRefEvapotransID = MFUnset;

static void _MDIrrRefEvapotransFAO (int itemID) { // day-night Penman-Monteith PE in mm for day
// Input
 	float airT      = MFVarGetFloat (_MDInCommon_AtMeanID, itemID, 0.0); // air temperatur [degree C]
	float airTMin   = MFVarGetFloat (_MDInAtMinID,         itemID, 0.0); // daily minimum air temperature [degree C]
	float airTMax   = MFVarGetFloat (_MDInAtMaxID,         itemID, 0.0); // daily maximum air temperature [degree C]
	float solRad    = MFVarGetFloat (_MDInSolRadID,        itemID, 0.0); // daily solar radiation on horizontal [MJ/m2]
	float i0hDay    = MFVarGetFloat (_MDInI0HDayID,        itemID, 0.0); //  daily potential insolation on horizontal [MJ/m2]
	float vPress    = MFVarGetFloat (_MDInVPressID,        itemID, 0.0) / 1000.0; // daily average vapor pressure [kPa]
	float elevation = MFVarGetFloat (_MDInElevationID,     itemID, 0.0);
	float wSpeed    = MFVarGetFloat (_MDInWSpeedID,        itemID, 0.0); // average wind speed for the day [m/s]
// Output
	float FAOEtp;
// Local
	float solNet;  // average net solar radiation for daytime [W/m2]
	float es;      // vapor pressure at airT [kPa]
	float es_min;  // vapor pressure at airT [kPa]
	float es_max;  // vapor pressure at airT [kPa]
	float delta;   // dEsat/dTair [kPa/K]
	float psychometricConstant;
	float solNet_MJm2d;
	float nom;
	float denom;
	float meanAirTemp;
	float atmosPressure;
	float nen;
	float temp;
 
	if (wSpeed < 0.2) wSpeed = 0.2;	
	atmosPressure = (293.0 - 0.0065 * elevation) / 293.0;
	atmosPressure = pow (atmosPressure, 5.26);
	atmosPressure = atmosPressure * 101.3;
	psychometricConstant = 0.665 * atmosPressure / 1000.0;

	meanAirTemp  = (airTMin + airTMax) / 2.0;
	solNet       = (1.0 - 0.23) * solRad ;// in MJ/m2 
	es_min       = MDPETlibVPressSat (airTMin);
 	es_max       = MDPETlibVPressSat (airTMax);
 	es           = (es_min + es_max) / 2.0;
	solNet_MJm2d = solNet;  // 0.0864; FAO equation wants SolNet in MJ/m2.
	nen          = 4098 * (0.6108 * exp (17.27 * airT / (airT + 237.3)));
	delta        = nen / ((airT + 237.3)*(airT + 237.3));

 	temp         = es - vPress;
	nom          = 0.408 * delta * solNet_MJm2d + psychometricConstant * 900 / (273.3 + airT) * wSpeed * temp; 
	//FBM nimmt vapor pressure in kPA!
	denom = delta + psychometricConstant*(1+0.34 * wSpeed);

	FAOEtp = nom / denom;
	MFVarSetFloat (_MDOutIrrRefEvapotransID,itemID,FAOEtp);
}

int MDIrrigation_ReferenceETFAODef () {
	if (_MDOutIrrRefEvapotransID != MFUnset) return (_MDOutIrrRefEvapotransID);

	MFDefEntering ("Irrigation Reference ETP (FAO)");
	if (((_MDInDayLengthID         = MDCommon_SolarRadDayLengthDef ()) == CMfailed) ||
        ((_MDInI0HDayID            = MDCommon_SolarRadI0HDayDef ())    == CMfailed) ||
        ((_MDInSolRadID            = MDCommon_SolarRadDef ())          == CMfailed) ||
        ((_MDInCommon_AtMeanID     = MDCommon_AirTemperatureDef ())    == CMfailed) ||
        ((_MDInElevationID         = MFVarGetID (MDVarCommon_Elevation,             "m",    MFInput,  MFState, MFBoundary)) == CMfailed) ||
        ((_MDInAtMinID             = MFVarGetID (MDVarCommon_AirTempMinimum,        "degC", MFInput,  MFState, MFBoundary)) == CMfailed) ||
        ((_MDInAtMaxID             = MFVarGetID (MDVarCommon_AirTempMaximum,        "degC", MFInput,  MFState, MFBoundary)) == CMfailed) ||
        ((_MDInVPressID            = MFVarGetID (MDVarCommon_HumidityVaporPressure, "Pa",   MFInput,  MFState, MFBoundary)) == CMfailed) ||
        ((_MDInWSpeedID            = MFVarGetID (MDVarCommon_WindSpeed,             "m/s",  MFInput,  MFState, MFBoundary)) == CMfailed) ||
        ((_MDOutIrrRefEvapotransID = MFVarGetID (MDVarIrrigation_RefEvapotrans,     "mm",   MFOutput, MFFlux,  MFBoundary)) == CMfailed)) return (CMfailed);
    if (MFModelAddFunction (_MDIrrRefEvapotransFAO)== CMfailed) return (CMfailed);

	MFDefLeaving ("Irrigation Reference ETP (FAO)");
	return(_MDOutIrrRefEvapotransID);
}
