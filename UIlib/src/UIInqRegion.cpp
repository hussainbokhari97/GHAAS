/******************************************************************************

GHAAS User Interface library V2.1
Global Hydrological Archive and Analysis System
Copyright 1994-2024, UNH - CCNY

UIInqRegion.cpp

bfekete@ccny.cuny.edu

*******************************************************************************/

#include <X11/cursorfont.h>
#include <UI.hpp>

int UIInquireRegion(Widget widget, XEvent *event, int *x, int *y, int *width, int *height) {
    XSetWindowAttributes xswa;
    XGCValues xgcv;
    static GC gc = (GC) NULL;
    static Cursor cursor[5] = {(Cursor) NULL, (Cursor) NULL, (Cursor) NULL, (Cursor) NULL, (Cursor) NULL};
    static int inLoop = false, cont, ret, buttonPressed, dir;

    if (cursor[0] == (Cursor) NULL) cursor[0] = XCreateFontCursor(XtDisplay(widget), XC_bottom_right_corner);
    if (cursor[1] == (Cursor) NULL) cursor[1] = XCreateFontCursor(XtDisplay(widget), XC_top_right_corner);
    if (cursor[2] == (Cursor) NULL) cursor[2] = XCreateFontCursor(XtDisplay(widget), XC_bottom_left_corner);
    if (cursor[3] == (Cursor) NULL) cursor[3] = XCreateFontCursor(XtDisplay(widget), XC_top_left_corner);
    if (cursor[4] == (Cursor) NULL) cursor[4] = XCreateFontCursor(XtDisplay(widget), XC_top_left_arrow);
    if (gc == (GC) NULL) {
        xgcv.function = GXinvert;
        gc = XtGetGC(widget, GCFunction, &xgcv);
    }
    if (inLoop == false) {
        if (event->type != ButtonPress) return (false);
        inLoop = buttonPressed = cont = true;
        ret = false;
        xswa.cursor = cursor[dir = 0];
        XChangeWindowAttributes(XtDisplay(widget), XtWindow(widget), CWCursor, &xswa);
        *x = event->xbutton.x;
        *y = event->xbutton.y;
        *width = *height = 0;
        XDrawRectangle(XtDisplay(widget), XtWindow(widget), gc, *x, *y, *width, *height);
        do {
            XtAppNextEvent(UIApplicationContext(), event);
            XtDispatchEvent(event);
        } while (cont);
        XDrawRectangle(XtDisplay(widget), XtWindow(widget), gc, *x, *y, *width, *height);
        xswa.cursor = cursor[4];
        XChangeWindowAttributes(XtDisplay(widget), XtWindow(widget), CWCursor, &xswa);
        if (*width < 0) {
            *x += *width;
            *width *= -1;
        }
        if (*height < 0) {
            *y += *height;
            *height *= -1;
        }
        buttonPressed = false;
        inLoop = false;
        return (ret);
    }
    else
        switch (event->type) {
            case ButtonPress:
                XDrawRectangle(XtDisplay(widget), XtWindow(widget), gc, *x, *y, *width, *height);
                buttonPressed = true;
                *x = event->xbutton.x;
                *y = event->xbutton.y;
                *width = *height = 0;
                xswa.cursor = cursor[dir = 0];
                XChangeWindowAttributes(XtDisplay(widget), XtWindow(widget), CWCursor, &xswa);
                XDrawRectangle(XtDisplay(widget), XtWindow(widget), gc, *x, *y, *width, *height);
                break;
            case ButtonRelease:
                buttonPressed = false;
                xswa.cursor = cursor[4];
                XChangeWindowAttributes(XtDisplay(widget), XtWindow(widget), CWCursor, &xswa);
                ret = true;
                cont = false;
                break;
            case MotionNotify:
                if (buttonPressed == false) break;
                switch (dir) {
                    default:
                        XDrawRectangle(XtDisplay(widget), XtWindow(widget), gc, *x, *y, *width, *height);
                        break;
                    case 1:
                        XDrawRectangle(XtDisplay(widget), XtWindow(widget), gc, *x, *y + *height, *width, -*height);
                        break;
                    case 2:
                        XDrawRectangle(XtDisplay(widget), XtWindow(widget), gc, *x + *width, *y, -*width, *height);
                        break;
                    case 3:
                        XDrawRectangle(XtDisplay(widget), XtWindow(widget), gc, *x + *width, *y + *height, -*width,
                                       -*height);
                        break;
                }
                *width = event->xmotion.x - *x;
                *height = event->xmotion.y - *y;
                if (dir != (((*width < 0 ? 1 : 0) << 0x01) | (*height < 0 ? 1 : 0))) {
                    dir = ((*width < 0 ? 1 : 0) << 0x01) | (*height < 0 ? 1 : 0);
                    xswa.cursor = cursor[dir];
                    XChangeWindowAttributes(XtDisplay(widget), XtWindow(widget), CWCursor, &xswa);
                }
                switch (dir) {
                    default:
                        XDrawRectangle(XtDisplay(widget), XtWindow(widget), gc, *x, *y, *width, *height);
                        break;
                    case 1:
                        XDrawRectangle(XtDisplay(widget), XtWindow(widget), gc, *x, *y + *height, *width, -*height);
                        break;
                    case 2:
                        XDrawRectangle(XtDisplay(widget), XtWindow(widget), gc, *x + *width, *y, -*width, *height);
                        break;
                    case 3:
                        XDrawRectangle(XtDisplay(widget), XtWindow(widget), gc, *x + *width, *y + *height, -*width,
                                       -*height);
                        break;
                }
                break;
            case LeaveNotify:
                ret = false;
                cont = false;
                break;
        }
    return (false);
}
