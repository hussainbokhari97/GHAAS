/******************************************************************************

GHAAS User Interface library V2.1
Global Hydrological Archive and Analysis System
Copyright 1994-2024, UNH - CCNY

UIFileSel.cpp

bfekete@ccny.cuny.edu

*******************************************************************************/

#include <Xm/FileSB.h>
#include <Xm/DialogS.h>
#include <UI.hpp>

static char *_UIFileName = NULL;

static void _UIFileSelectionNoMatchCBK(Widget widget, void *dummy, XmFileSelectionBoxCallbackStruct *callData) {
    UIMessage((char *) "Must Select!");
}

static void _UIFileSelectionOkCBK(Widget widget, char *fullPath, XmFileSelectionBoxCallbackStruct *callData) {
    char *mask, *dir;
    int i;

    if (XmStringGetLtoR(callData->value, XmSTRING_DEFAULT_CHARSET, &_UIFileName));
    else _UIFileName = NULL;
    XmStringGetLtoR(callData->dir, XmSTRING_DEFAULT_CHARSET, &dir);
    if (callData->dir_length > callData->length) snprintf(fullPath, DBDataFileNameLen, "%s/%s", dir, _UIFileName);
    else strcpy(fullPath, _UIFileName);

    _UIFileName = fullPath;
    if (XmStringGetLtoR(callData->pattern, XmSTRING_DEFAULT_CHARSET, &mask)) {
        for (i = strlen(mask); i > 0; --i) if (mask[i - 1] == '*' || mask[i - 1] == '?') break;
        if (i > 0) if (strcmp(_UIFileName + strlen(_UIFileName) - strlen(mask) + i, mask + i) != 0)
            strcat(fullPath, mask + i);
    }
}

Widget UIFileSelectionCreate(char *titleText, char *directory, char *pattern, int type) {
    static char fullPath[DBDataFileNameLen];
    XmString string;
    Widget dShell, selection;

    dShell = XtVaCreatePopupShell(titleText, xmDialogShellWidgetClass, UITopLevel(),
                                  XmNallowShellResize, true,
                                  XmNtransient, true,
                                  XmNkeyboardFocusPolicy, XmEXPLICIT,
                                  NULL);
    selection = XtVaCreateWidget("UIFileSelectionDialog", xmFileSelectionBoxWidgetClass, dShell,
                                 XmNfileTypeMask, type,
                                 XmNdialogStyle, XmDIALOG_FULL_APPLICATION_MODAL,
                                 NULL);
    if (directory != NULL) {
        string = XmStringCreate(directory, UICharSetNormal);
        XtVaSetValues(selection, XmNdirectory, string, NULL);
        XmStringFree(string);
    }
    if (pattern != NULL) {
        string = XmStringCreate(pattern, UICharSetNormal);
        XtVaSetValues(selection, XmNpattern, string, NULL);
        XmStringFree(string);
    }

    XtUnmanageChild(XmFileSelectionBoxGetChild(selection, XmDIALOG_HELP_BUTTON));
    XtAddCallback(selection, XmNokCallback, (XtCallbackProc) _UIFileSelectionOkCBK, fullPath);
    XtAddCallback(selection, XmNokCallback, (XtCallbackProc) UILoopStopCBK, NULL);
    XtAddCallback(selection, XmNcancelCallback, (XtCallbackProc) UILoopStopCBK, NULL);
    XtAddCallback(selection, XmNnoMatchCallback, (XtCallbackProc) _UIFileSelectionNoMatchCBK, NULL);

    return (selection);
}

char *UIFileSelection(Widget widget, int match) {
    XmString dirMask;
    XtVaGetValues(widget, XmNdirMask, &dirMask, NULL);
    XtVaSetValues(widget, XmNmustMatch, match, NULL);
    XmFileSelectionDoSearch(widget, dirMask);
    XtManageChild(widget);
    _UIFileName = NULL;
    while (UILoop());

    XtUnmanageChild(widget);
    return (_UIFileName);
}
