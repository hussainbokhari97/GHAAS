/******************************************************************************

GHAAS Water Balance Model Library V1.0
Global Hydrological Archive and Analysis System
Copyright 1994-2024, UNH - CCNY

MFSwap.c

bfekete@ccny.cuny.edu

*******************************************************************************/

#include <MF.h>


void MFSwapHalfWord (void *data) {
	char ch;

	ch = ((char *) data) [0]; ((char *) data) [0] = ((char *) data) [1]; ((char *) data) [1] = ch;
}

void MFSwapWord (void *data) {
	char ch;

	ch =  ((char *) data) [0];		((char *) data) [0] =  ((char *) data) [3];  ((char *) data) [3] = ch;
	ch =  ((char *) data) [1]; 	((char *) data) [1] =  ((char *) data) [2];  ((char *) data) [2] = ch;
}

void MFSwapLongWord (void *data) {
	char ch;

	ch = ((char *) data) [0]; ((char *) data) [0] = ((char *) data) [7]; ((char *) data) [7] = ch;
	ch = ((char *) data) [1]; ((char *) data) [1] = ((char *) data) [6]; ((char *) data) [6] = ch;
	ch = ((char *) data) [2]; ((char *) data) [2] = ((char *) data) [5]; ((char *) data) [5] = ch;
	ch = ((char *) data) [3]; ((char *) data) [3] = ((char *) data) [4]; ((char *) data) [4] = ch;
}
