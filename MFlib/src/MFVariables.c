/******************************************************************************

GHAAS Water Balance Model Library V1.0
Global Hydrological Archive and Analysis System
Copyright 1994-2024, UNH - CCNY

MFVariables.c

bfekete@ccny.cuny.edu

*******************************************************************************/

#include <stdlib.h>
#include <string.h>
#include <cm.h>
#include <MF.h>

static MFVariable_p _MFVariables = (MFVariable_p) NULL;
static int _MFVariableNum = 0;

MFVariable_p MFVarGetByID (int id) {
	return ((id > 0) && (id <= _MFVariableNum) ? _MFVariables + id - 1: (MFVariable_p) NULL); // TODO assert() !!
}

char *MFVarTypeString (int type) {
	switch (type) {
		case MFInput:  return ("input");
		case MFOutput: return ("output");
		case MFRoute:  return ("route");
		case MFByte:   return ("byte");
		case MFShort:  return ("short");
		case MFInt:    return ("int");
		case MFFloat:  return ("float");
		case MFDouble: return ("double");
		default: CMmsgPrint (CMmsgAppError,"Error: Invalide Type [%d] in: %s:%d",__FILE__,__LINE__); break;
	}
	return ("");
}

static MFVariable_p _MFVarNewEntry (const char *name) {
	MFVariable_p var;
	_MFVariables = (MFVariable_p) realloc (_MFVariables,(_MFVariableNum + 1) * sizeof (MFVariable_t));
	if (_MFVariables == (MFVariable_p) NULL) {
	 	CMmsgPrint (CMmsgSysError,"Error: Memory allocation error in: %s:%d",__FILE__,__LINE__);
		return ((MFVariable_p) NULL);
	}
	var = _MFVariables + _MFVariableNum;
	var->ID = _MFVariableNum + 1;
	strncpy (var->Name,name,sizeof (var->Name) - 1);
	strcpy  (var->Unit,MFNoUnit);
	strcpy  (var->InDate,  "NOT SET");
	strcpy  (var->OutDate, "NOT SET");
	strcpy  (var->CurDate, "NOT SET");
	var->ItemNum  = 0;
	var->Type = MFInput;
	var->Buffer     = (void *) NULL;
	var->InputPath  = (char *) NULL;
	var->OutputPath = (char *) NULL;
	var->StatePath  = (char *) NULL;
	var->InStream   = (MFDataStream_p) NULL;
	var->OutStream  = (MFDataStream_p) NULL;
	var->TStep      = MFTimeStepYear;
    var->NStep      = 1;
	var->Set        = false;
	var->Flux       = false;
	var->Initial    = false;
	var->Route      = false;
    var->Read       = false;
	_MFVariableNum++;
	return (var);
}

static MFVariable_p _MFVarFindEntry (const char *name) {
	int i;

	for (i = 0;i < _MFVariableNum;++i) if (strcmp (_MFVariables [i].Name,name) == 0) break;
	if (i < _MFVariableNum) return (_MFVariables + i);
	return ((MFVariable_p) NULL);
}


int MFVarGetID (char *name,char *unit,int type, bool flux, bool initial) {
	MFVariable_p var;

	if ((var = _MFVarFindEntry (name)) == (MFVariable_p) NULL) {
		if ((var = _MFVarNewEntry (name)) == (MFVariable_p) NULL) return (CMfailed);
		if (type == MFRoute) var->Route = true;
		var->Type = type == MFInput ? MFInput : MFOutput;
		var->Set  = type == MFInput ? initial : true;
	}
	switch (var->Type) {
		case MFInput:		break;
		case MFRoute:
		case MFOutput:
			switch (type) {
				case MFInput: if (!var->Set) var->Type = MFInput; break;
				case MFRoute:
				case MFOutput: break;
				case MFByte:	var->Type = type; var->Missing.Int   = MFDefaultMissingByte;  break;
				case MFShort:
				case MFInt:		var->Type = type; var->Missing.Int   = MFDefaultMissingInt;   break;
				case MFFloat:
				case MFDouble:	var->Type = type; var->Missing.Float = MFDefaultMissingFloat; break;
			}
			break;
		default:
			switch (type) {
				default:
					if (type != var->Type)
						CMmsgPrint (CMmsgWarning,"Warning: Ignoring type redefinition (%s,%s)",var->Name,MFVarTypeString (type));
				case MFInput:
				case MFRoute:
				case MFOutput:	break;
			}
	}

	if (initial != var->Initial) {
		if (initial) var->Initial = initial;
		else CMmsgPrint (CMmsgWarning,"Warning: Ignoring initial redefinition (%s [%s])!",var->Name,var->Unit);
	}

	if (strncmp (unit,var->Unit,strlen (unit)) != 0) {
		if (strcmp (var->Unit,MFNoUnit) == 0) strncpy (var->Unit,unit,sizeof (var->Unit) - 1);
		else CMmsgPrint (CMmsgWarning,"Warning: Ignoring Unit redefinition (%s [%s],%d)!",var->Name,var->Unit,type);
	}
	if (flux != var->Flux) {
		if (flux)  var->Flux = flux;
		else CMmsgPrint (CMmsgWarning,"Warning: Ignoring flux redefinition (%s [%s])!",var->Name,var->Unit);
	}
	return (var->ID);
}

static bool _MFVarTestMissingVal (MFVariable_p var,int itemID)
	{
	switch (var->Type) {
		case MFByte:   return ((int) (((char *)  var->Buffer) [itemID]) == var->Missing.Int);
		case MFShort:  return ((int) (((short *) var->Buffer) [itemID]) == var->Missing.Int);
		case MFInt:	   return ((int) (((int *)   var->Buffer) [itemID]) == var->Missing.Int);
		case MFFloat:  return (CMmathEqualValues ((((float *)  var->Buffer) [itemID]),var->Missing.Float));
		case MFDouble: return (CMmathEqualValues ((((double *) var->Buffer) [itemID]),var->Missing.Float));
		default:
			CMmsgPrint (CMmsgAppError,"Error: Invalid variable [%s,%d] type [%d] in %s:%d",var->Name, itemID, var->Type,__FILE__,__LINE__);
			break;
	}
	return (true);
}

bool MFVarTestMissingVal (int id,int itemID)
	{
	MFVariable_p var;

	if ((var = MFVarGetByID (id)) == (MFVariable_p) NULL)  {
		CMmsgPrint (CMmsgAppError,"Error: Invalid variable [%d] in: %s:%d",id,__FILE__,__LINE__);
		return (true);
	}
	if ((itemID < 0) || (itemID >= var->ItemNum)) {
		CMmsgPrint (CMmsgAppError,"Error: Invalid item [%s,%d] in: %s:%d",var->Name,itemID,__FILE__,__LINE__);
		return (true);
	}
	return  (_MFVarTestMissingVal (var,itemID));
}

void MFVarSetMissingVal (int id, int itemID)
	{
	MFVariable_p var;

	if (((var = MFVarGetByID (id)) == (MFVariable_p) NULL) || (itemID < 0) || (itemID >= var->ItemNum)) {
		CMmsgPrint (CMmsgAppError,"Error: Invalid variable [%d,%d] in: %s:%d\n",id,itemID,__FILE__,__LINE__);
		return;
	}
	switch (var->Type) {
		case MFByte:	((char *)   var->Buffer) [itemID] = (char)   var->Missing.Int;		break;
		case MFShort:	((short *)  var->Buffer) [itemID] = (short)  var->Missing.Int;		break;
		case MFInt:		((int *)    var->Buffer) [itemID] = (int)    var->Missing.Int;		break;
		case MFFloat:	((float *)  var->Buffer) [itemID] = (float)  var->Missing.Float;	break;
		case MFDouble:	((double *) var->Buffer) [itemID] = (double) var->Missing.Float;	break;
		default:
			CMmsgPrint (CMmsgAppError,"Error: Invalid variable [%s,%d] type [%d] in %s:%d",var->Name, itemID, var->Type,__FILE__,__LINE__);
			break;
	}
}

void MFVarSetFloat (int id,int itemID,double val) {
	MFVariable_p var;

	if (((var = MFVarGetByID (id)) == (MFVariable_p) NULL) || (itemID < 0) || (itemID >= var->ItemNum)) {
		CMmsgPrint (CMmsgAppError,"Error: Invalid variable [%d,%d] in: MFVarSetFloat ()\n",id,itemID);
		return;
	}

	var->Set = true;
	if (var->Flux) val = val * (double) var->NStep;
	switch (var->Type) {
		case MFByte:	((char *)   var->Buffer) [itemID] = (char)  val; break;
		case MFShort:	((short *)  var->Buffer) [itemID] = (short) val; break;
		case MFInt:		((int *)    var->Buffer) [itemID] = (int)   val; break;
		case MFFloat:	((float *)  var->Buffer) [itemID] = (float) val; break;
		case MFDouble:	((double *) var->Buffer) [itemID] =         val; break;
		default:
			CMmsgPrint (CMmsgAppError,"Error: Invalid variable [%s,%d] type [%d] in %s:%d\n",var->Name, itemID, var->Type,__FILE__,__LINE__);
			break;
	}
}

double MFVarGetFloat (int id,int itemID,double missingVal) {
	double val;
	MFVariable_p var;

	if (((var = MFVarGetByID (id)) == (MFVariable_p) NULL) || (itemID < 0) || (itemID >= var->ItemNum)) {
		CMmsgPrint (CMmsgAppError,"Error: Invalid variable [%d,%d] in: MFVarGetFloat ()\n",id,itemID);
		return (MFDefaultMissingFloat);
	}
	if ((itemID == 0) && (var->Set != true)) CMmsgPrint (CMmsgWarning,"Warning: Unset variable [%s]!\n",var->Name);
	if (_MFVarTestMissingVal (var,itemID)) return (missingVal);

	switch (var->Type) {
		case MFByte:	val = (double) (((char *)   var->Buffer) [itemID]); break;
		case MFShort:	val = (double) (((short *)  var->Buffer) [itemID]); break;
		case MFInt:		val = (double) (((int *)    var->Buffer) [itemID]); break;
		case MFFloat:	val = (double) (((float *)  var->Buffer) [itemID]); break;
		case MFDouble:	val = (double) (((double *) var->Buffer) [itemID]); break;
		default:
			CMmsgPrint (CMmsgAppError,"Error: Invalid variable [%s,%d] type [%d] in %s:%d\n",var->Name, itemID, var->Type,__FILE__,__LINE__);
			return (MFDefaultMissingFloat);
	}
 	return (var->Flux ? val / (double) var->NStep : val);
}

void MFVarSetInt (int id,int itemID,int val) {
	MFVariable_p var;

	if (((var = MFVarGetByID (id)) == (MFVariable_p) NULL) || (itemID < 0) || (itemID >= var->ItemNum)) {
		CMmsgPrint (CMmsgAppError,"Error: Invalid variable [%d,%d] in: %s:%d\n",id,itemID,__FILE__,__LINE__);
		return;
	}

	var->Set = true;
	if (var->Flux) val = val * var->NStep;
	switch (var->Type) {
		case MFByte:	((char *)   var->Buffer) [itemID] = (char)   val;	break;
		case MFShort:	((short *)  var->Buffer) [itemID] = (short)  val;	break;
		case MFInt:		((int *)    var->Buffer) [itemID] = (int)    val;	break;
		case MFFloat:	((float *)  var->Buffer) [itemID] = (float)  val;	break;
		case MFDouble:	((double *) var->Buffer) [itemID] = (double) val;	break;
		default:
			CMmsgPrint (CMmsgAppError,"Error: Invalid variable [%s,%d] type [%d] in %s:%d\n",var->Name, itemID, var->Type,__FILE__,__LINE__);
			break;
	}
}

int MFVarGetInt (int id,int itemID, int missingVal) {
	int val;
	MFVariable_p var;

	if (((var = MFVarGetByID (id)) == (MFVariable_p) NULL) || (itemID < 0) || (itemID >= var->ItemNum)) {
		CMmsgPrint (CMmsgAppError,"Error: Invalid variable [%d,%d] in: %s:%d\n",id,itemID,__FILE__,__LINE__);
		return (MFDefaultMissingInt);
	}

	if (var->Set != true) CMmsgPrint (CMmsgWarning,"Warning: Unset variable %s\n",var->Name);
	if (_MFVarTestMissingVal (var,itemID)) return (missingVal);
	
	switch (var->Type) {
		case MFByte:	val = (int) (((char *)   var->Buffer) [itemID]); break;
		case MFShort:	val = (int) (((short *)  var->Buffer) [itemID]); break;
		case MFInt:		val = (int) (((int *)    var->Buffer) [itemID]); break;
		case MFFloat:	val = (int) (((float *)  var->Buffer) [itemID]); break;
		case MFDouble:	val = (int) (((double *) var->Buffer) [itemID]); break;
		default:
			CMmsgPrint (CMmsgAppError,"Error: Invalid variable [%s,%d] type [%d] in %s:%d\n",var->Name, itemID, var->Type,__FILE__,__LINE__);
			return (MFDefaultMissingInt);
	}
	return (var->Flux ? (val / var->NStep) : val);
}	

size_t MFVarItemSize (int type) {
	switch (type) {
		case MFByte:	return (sizeof (char));
		case MFShort:	return (sizeof (short));
		case MFInt:		return (sizeof (int));
		case MFFloat:	return (sizeof (float));
		case MFDouble:	return (sizeof (double));
		default:
			CMmsgPrint (CMmsgAppError,"Error: Invalid type [%d] in: %s:%d\n",type,__FILE__,__LINE__);
			break;
	}
	return (0);
}
