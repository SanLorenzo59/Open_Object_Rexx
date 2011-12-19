/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 2011-2011 Rexx Language Association. All rights reserved.    */
/*                                                                            */
/* This program and the accompanying materials are made available under       */
/* the terms of the Common Public License v1.0 which accompanies this         */
/* distribution. A copy is also available at the following address:           */
/* http://www.oorexx.org/license.html                                         */
/*                                                                            */
/* Redistribution and use in source and binary forms, with or                 */
/* without modification, are permitted provided that the following            */
/* conditions are met:                                                        */
/*                                                                            */
/* Redistributions of source code must retain the above copyright             */
/* notice, this list of conditions and the following disclaimer.              */
/* Redistributions in binary form must reproduce the above copyright          */
/* notice, this list of conditions and the following disclaimer in            */
/* the documentation and/or other materials provided with the distribution.   */
/*                                                                            */
/* Neither the name of Rexx Language Association nor the names                */
/* of its contributors may be used to endorse or promote products             */
/* derived from this software without specific prior written permission.      */
/*                                                                            */
/* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS        */
/* "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT          */
/* LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS          */
/* FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT   */
/* OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,      */
/* SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED   */
/* TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,        */
/* OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY     */
/* OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING    */
/* NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS         */
/* SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.               */
/*                                                                            */
/*----------------------------------------------------------------------------*/

/**
 * oodMouse.cpp
 *
 * This module contains functions and methods for classes related to the mouse
 * and cursors.
 *
 */
#include "ooDialog.hpp"     // Must be first, includes windows.h, commctrl.h, and oorexxapi.h
//#include "oodControl.hpp"

//#include <stdio.h>
//#include <dlgs.h>
#include <shlwapi.h>
#include <WindowsX.h>

#include "APICommon.hpp"
#include "oodCommon.hpp"
#include "oodMessaging.hpp"
#include "oodResources.hpp"
#include "oodMouse.hpp"


/**
 *  Methods for the .Mouse class.
 */
#define MOUSE_CLASS        "Mouse"

#define TRACK_MOUSE_KEYWORDS    "CANCEL, HOVER, LEAVE, NONCLIENT, or QUERY"
#define WM_MOUSE_KEYWORDS       "Move, Wheel, Leave, Hover, lButtonUp, lButtonDown, or CaptureChanged"
#define MOUSE_BUTTON_KEYWORDS   "LEFT, RIGHT, MIDDLE, XBUTTON1, or XBUTTON2"
#define SYSTEM_CURSOR_KEYWORDS  "APPSTARTING, ARROW, CROSS, HAND, HELP, IBEAM, NO, SIZEALL, SIZENESW, SIZENS, " \
                                "SIZENWSE, SIZEWE, UPARROW, or WAIT"

#define DLG_HAS_ENDED_MSG       "windows dialog has executed and been closed"

/**
 * Returns the mouse CSelf, raising an exception if it is null.
 *
 * @param c
 * @param p
 *
 * @return pCMouse
 */
static pCMouse getMouseCSelf(RexxMethodContext *c, void *p)
{
    oodResetSysErrCode(c->threadContext);

    pCMouse pcm = (pCMouse)p;
    if ( pcm == NULL )
    {
        baseClassIntializationException(c);
    }
    return pcm;
}

/**
 * Checks that the underlying dialog is valid and updates the dialog window
 * handle and thread ID in the mouse CSelf struct if needed.
 *
 * @param c    Method context we are operating in.
 * @param pcm  Assumed pointer to mouse CSelf struct.
 *
 * @return The pCMouse pointer, or null on any error.
 *
 * @remarks  The mouse object can be instantiated before the underlying windows
 *           dialog exists.  In which case the hWindow, hDlg, and
 *           dlgProcThreadID members of the mouse CSelf struct will all be null.
 *           This function ensures that these members are updated if needed.
 *
 *           In addition, it is possible that the dialog has been closed and the
 *           Rexx programmer is attempting to use the mouse object after that.
 *           So, we also ensure that this is not happening.
 *
 *           Note that hWindow, hDlg, and dlgProcThreadID will never be null for
 *           a dialog control, because a Rexx dialog control can not be
 *           instantiated until the underlying dialog exists, so these members
 *           are set correctly during Mouse::init().
 */
static pCMouse requiredDlg(RexxMethodContext *c, void *p)
{
    pCMouse pcm = getMouseCSelf(c, p);
    if ( pcm == NULL )
    {
        return NULL;
    }

    // Unlikly, but it is possible the dialog is being destroyed at this
    // instant.
    EnterCriticalSection(&crit_sec);

    if ( ! pcm->dlgCSelf->dlgAllocated )
    {
        methodCanNotBeInvokedException(c, c->GetMessageName(), DLG_HAS_ENDED_MSG, pcm->rexxSelf);
        pcm = NULL;
        goto done_out;
    }

    if ( pcm->hWindow == NULL )
    {
        if ( pcm->dlgCSelf->hDlg == NULL )
        {
            noWindowsDialogException(c, pcm->rexxSelf);
            pcm = NULL;
            goto done_out;
        }

        pcm->dlgProcThreadID = pcm->dlgCSelf->dlgProcThreadID;
        pcm->hWindow         = pcm->dlgCSelf->hDlg;

        pcm->hDlg = pcm->hWindow;
    }

done_out:
    LeaveCriticalSection(&crit_sec);
    return pcm;
}


static pCEventNotification getMousePCEN(RexxMethodContext *c, pCMouse pcm)
{
    oodResetSysErrCode(c->threadContext);

    pCEventNotification pcen = NULL;

    if ( pcm == NULL )
    {
        baseClassIntializationException(c);
        goto done_out;
    }

    // Unlikly, but it is possible the dialog is being destroyed at this
    // instant.
    EnterCriticalSection(&crit_sec);

    if ( pcm->dlgCSelf == NULL )
    {
        methodCanNotBeInvokedException(c, c->GetMessageName(), DLG_HAS_ENDED_MSG, pcm->rexxSelf);
        pcm->hWindow = NULL;
        goto done_out;
    }

    pcen = pcm->dlgCSelf->enCSelf;

done_out:
    LeaveCriticalSection(&crit_sec);
    return pcen;
}


/**
 * Convert a keyword to the (mouse) window message code.
 *
 * We know the keyword arg position is 1.  The mouse support is post ooRexx
 * 4.0.1 so we raise an exception on error.
 */
static bool keyword2wm(RexxMethodContext *c, CSTRING keyword, uint32_t *flag)
{
    uint32_t wmMsg;

    if ( StrCmpI(keyword,      "MOVE")           == 0 ) wmMsg = WM_MOUSEMOVE;
    else if ( StrCmpI(keyword, "WHEEL")          == 0 ) wmMsg = WM_MOUSEWHEEL;
    else if ( StrCmpI(keyword, "LEAVE")          == 0 ) wmMsg = WM_MOUSELEAVE;
    else if ( StrCmpI(keyword, "HOVER")          == 0 ) wmMsg = WM_MOUSEHOVER;
    else if ( StrCmpI(keyword, "LBUTTONDOWN")    == 0 ) wmMsg = WM_LBUTTONDOWN;
    else if ( StrCmpI(keyword, "LBUTTONUP")      == 0 ) wmMsg = WM_LBUTTONUP;
    else if ( StrCmpI(keyword, "LBUTTONDBLCLK")  == 0 ) wmMsg = WM_LBUTTONDBLCLK;
    else if ( StrCmpI(keyword, "CAPTURECHANGED") == 0 ) wmMsg = WM_CAPTURECHANGED;
    else
    {
        wrongArgValueException(c->threadContext, 1, WM_MOUSE_KEYWORDS, keyword);
        return false;
    }
    *flag = wmMsg;
    return true;
}


/**
 * Convert a keyword to one of the system cursor values.  IDC_ARROW, etc..
 *
 * We know the keyword arg position is 1.  The mouse support is post ooRexx
 * 4.0.1 so we raise an exception on error.
 */
static CSTRING keyword2cursor(RexxMethodContext *c, CSTRING keyword)
{
    CSTRING cursor = NULL;

    if ( StrCmpI(keyword,      "APPSTARTING") == 0 ) cursor = IDC_APPSTARTING;
    else if ( StrCmpI(keyword, "ARROW")       == 0 ) cursor = IDC_ARROW;
    else if ( StrCmpI(keyword, "CROSS")       == 0 ) cursor = IDC_CROSS;
    else if ( StrCmpI(keyword, "HAND")        == 0 ) cursor = IDC_HAND;
    else if ( StrCmpI(keyword, "HELP")        == 0 ) cursor = IDC_HELP;
    else if ( StrCmpI(keyword, "IBEAM")       == 0 ) cursor = IDC_IBEAM;
    else if ( StrCmpI(keyword, "NO")          == 0 ) cursor = IDC_NO;
    else if ( StrCmpI(keyword, "SIZEALL")     == 0 ) cursor = IDC_SIZEALL;
    else if ( StrCmpI(keyword, "SIZENESW")    == 0 ) cursor = IDC_SIZENESW;
    else if ( StrCmpI(keyword, "SIZENS")      == 0 ) cursor = IDC_SIZENS;
    else if ( StrCmpI(keyword, "SIZENWSE")    == 0 ) cursor = IDC_SIZENWSE;
    else if ( StrCmpI(keyword, "SIZEWE")      == 0 ) cursor = IDC_SIZEWE;
    else if ( StrCmpI(keyword, "UPARROW")     == 0 ) cursor = IDC_UPARROW;
    else if ( StrCmpI(keyword, "WAIT")        == 0 ) cursor = IDC_WAIT;
    else
    {
        wrongArgValueException(c->threadContext, 1, SYSTEM_CURSOR_KEYWORDS, keyword);
    }

    return cursor;
}


/**
 * Convert a window message code to a method name.
 */
inline CSTRING wm2name(uint32_t mcn)
{
    switch ( mcn )
    {
        case WM_MOUSEMOVE      : return "onMouseMove";
        case WM_MOUSEWHEEL     : return "onMouseWheel";
        case WM_MOUSELEAVE     : return "onMouseLeave";
        case WM_MOUSEHOVER     : return "onMouseHover";
        case WM_LBUTTONDOWN    : return "onLButtonDown";
        case WM_LBUTTONUP      : return "onLButtonUp";
        case WM_LBUTTONDBLCLK  : return "onLButtonDblClk";
        case WM_CAPTURECHANGED : return "onCaptureChanged";
    }
    return "onWM";
}

/**
 * Sets a new cursor and returns the old one as a .Image object.
 *
 * @param c
 * @param pcm
 * @param hCursor
 *
 * @return RexxObjectPtr
 */
static RexxObjectPtr mouseSetCursor(RexxMethodContext *c, pCMouse pcm, HCURSOR hCursor)
{
    RexxObjectPtr result    = TheZeroObj;
    HCURSOR       oldCursor = NULL;

    // TODO need to investigate this, which is the way it was done in all pre
    // 4.2.0 ooDialogs, and worked.  But, MSDN says to set GCLP_HCURSOR to NULL
    // and that SetCursor() returns the previous cursor, if there was one.
    oldCursor = (HCURSOR)setClassPtr(pcm->hWindow, GCLP_HCURSOR, (LONG_PTR)hCursor);
    SetCursor(hCursor);

    if ( oldCursor == NULL )
    {
        goto done_out;
    }

    SIZE s;
    s.cx = GetSystemMetrics(SM_CXCURSOR);
    s.cy = GetSystemMetrics(SM_CYCURSOR);

    // Note that we use true for the last arge, even though we are creating this
    // from a handle, because we are pretty sure of the size and the flags.
    result = rxNewValidImage(c, oldCursor, IMAGE_CURSOR, &s, LR_DEFAULTSIZE | LR_SHARED, true);

done_out:
    return result;
}


/**
 * Produces a rexx argument array for the standard mouse event handler
 * arugments.  Which are: keyState, mousePos, mouseObj.
 *
 * @param c
 * @param pcpbd
 * @param wParam
 * @param lParam
 * @param count   If count is greater than 3, the returned array will be this
 *                size.
 *
 * @return RexxArrayObject
 */
RexxArrayObject getMouseArgs(RexxThreadContext *c, pCPlainBaseDialog pcpbd, WPARAM wParam, LPARAM lParam, uint32_t count)
{
    char  buf[256] = {0};
    int   state    = GET_KEYSTATE_WPARAM(wParam);

    if ( state == 0 )
    {
        strcpy(buf, "None");
    }
    else
    {
        if ( state & MK_CONTROL  ) strcat(buf, "Control ");
        if ( state & MK_LBUTTON  ) strcat(buf, "lButton ");
        if ( state & MK_MBUTTON  ) strcat(buf, "mButton ");
        if ( state & MK_RBUTTON  ) strcat(buf, "rButton ");
        if ( state & MK_SHIFT    ) strcat(buf, "Shift ");
        if ( state & MK_XBUTTON1 ) strcat(buf, "xButton1 ");
        if ( state & MK_XBUTTON1 ) strcat(buf, "xButton2 ");

        buf[strlen(buf)] = '\0';
    }

    RexxStringObject rxState = c->String(buf);
    RexxObjectPtr    rxPoint = rxNewPoint(c, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));

    // Send the window losing capture and the mouse object.  Note, I
    // don't think we can be here without a valid mouse object in pcpbd,
    // but we will check any way.
    RexxObjectPtr rxMouse = pcpbd->rexxMouse ? pcpbd->rexxMouse : TheNilObj;

    RexxArrayObject args;
    if ( count > 3 )
    {
        args = c->NewArray(count);
        c->ArrayPut(args, rxState, 1);
        c->ArrayPut(args, rxPoint, 2);
        c->ArrayPut(args, rxMouse, 3);
    }
    else
    {
        args = c->ArrayOfThree(rxState, rxPoint, rxMouse);
    }

    return args;
}


/**
 * Generic function to send a WM_MOUSEWHEEL notification to the Rexx dialog
 * object.
 *
 * It is used for Edit::ignoreMouseWheel() and also
 * Mouse::connectEvent('WHEEL').
 *
 *
 * @param mwd
 * @param wParam
 * @param lParam
 *
 * @return bool
 */
bool mouseWheelNotify(PMOUSEWHEELDATA mwd, WPARAM wParam, LPARAM lParam)
{
    RexxThreadContext *c = mwd->dlgProcContext;

    RexxArrayObject args = getMouseArgs(c, mwd->pcpbd, wParam, lParam, 4);

    RexxObjectPtr rxDelta = c->WholeNumber(GET_WHEEL_DELTA_WPARAM(wParam));
    c->ArrayPut(args, rxDelta, 4);

    if ( mwd->willReply )
    {
        return invokeDirect(c, mwd->pcpbd, mwd->method, args);
    }
    else
    {
        invokeDispatch(c, mwd->ownerDlg, c->String(mwd->method), args);
    }
    return true;
}


/** Mouse::new()                [Class method]
 *
 *
 */
RexxMethod3(RexxObjectPtr, mouse_new_cls, RexxObjectPtr, ownerWindow, OSELF, self, SUPER, superClass)
{
    RexxMethodContext *c = context;
    RexxObjectPtr mouse = TheNilObj;

    NEWMOUSEPARAMS  nmp  = {0};
    pCDialogControl pcdc = NULL;

    // The owner window has to be a dialog or a dialog control
    pCPlainBaseDialog pcpbd = requiredDlgCSelf(context, ownerWindow, oodUnknown, 1, &pcdc);
    if ( pcpbd == NULL )
    {
        goto done_out;
    }

    bool isDialog = (pcdc != NULL ? false : true);

    // If a mouse object is already instantiated for this owner window, we just
    // return that object, otherwise we instantiate a new mouse object.  All the
    // book keeping details are handled in Mouse::init()
    if ( isDialog )
    {
        if ( pcpbd->rexxMouse != NULLOBJECT )
        {
            mouse = pcpbd->rexxMouse;
            goto done_out;
        }
    }
    else
    {
        if ( pcdc->rexxMouse != NULLOBJECT )
        {
            mouse = pcdc->rexxMouse;
            goto done_out;
        }
    }

    nmp.controlCSelf = pcdc;
    nmp.dlgCSelf     = pcpbd;
    nmp.isDlgWindow  = isDialog;

    // We send a pointer to the new mouse params struct as the argument to init()
    RexxArrayObject args = c->ArrayOfOne(c->NewPointer(&nmp));

    // Forwarding this message to the super class will also invoke the init()
    // method of the mouse instance object.
    mouse = c->ForwardMessage(NULLOBJECT, NULL, superClass, args);
    if ( mouse == NULLOBJECT )
    {
        mouse = TheNilObj;
    }

done_out:
    return mouse;
}

/** Mouse::getDoubleClickTime() [Class method]
 *
 * Gets the current double-click time for the mouse. A double-click is a
 * series of two clicks of the mouse button, the second occurring within a
 * specified time after the first. The double-click time is the maximum number
 * of milliseconds that may occur between the first and second click of a
 * double-click.
 *
 * @return  The current double-click time in milliseconds.
 *
 * @remarks  We do not even need the CSelf struct for this method, so we do not
 *           use.
 */
RexxMethod0(uint32_t, mouse_getDoubleClickTime_cls)
{
    return GetDoubleClickTime();
}

/** Mouse::setDoubleClickTime() [Class method]
 *
 * Sets the double-click time for the mouse.
 *
 * A double-click is a series of two clicks of the mouse button, the second
 * occurring within a specified time after the first. The double-click time is
 * the maximum number of milliseconds that may occur between the first and
 * second click of a double-click.
 *
 * @param  interval  The time in milliseconds to set the double-click time to.
 *                   If this is 0, the default system double-click time is
 *                   restored, which is 500 milliseconds.
 *
 * @return  True on success, false on error.
 *
 * @notes  Sets the .SystemErrorCode value. This method sets the double-click
 *         time for all windows in the system.
 *
 * @remarks  We do not even need the CSelf struct for this method, so we do not
 *           use.
 */
RexxMethod1(RexxObjectPtr, mouse_setDoubleClickTime_cls, uint32_t, interval)
{
    oodResetSysErrCode(context->threadContext);

    if ( SetDoubleClickTime(interval) == 0 )
    {
        oodSetSysErrCode(context->threadContext);
        return TheFalseObj;
    }
    return TheTrueObj;
}

/** Mouse::init()
 *
 *
 */
RexxMethod2(uint32_t, mouse_init, OPTIONAL_POINTER, args, OSELF, self)
{
    RexxMethodContext *c = context;

    if ( argumentOmitted(1) || args == NULL )
    {
        goto done_out;
    }

    PNEWMOUSEPARAMS params = (PNEWMOUSEPARAMS)args;

    // Get a buffer for the Mouse CSelf.
    RexxBufferObject cselfBuffer = context->NewBuffer(sizeof(CMouse));
    if ( cselfBuffer == NULLOBJECT )
    {
        return 0;
    }
    context->SetObjectVariable("CSELF", cselfBuffer);

    pCMouse pcm = (pCMouse)context->BufferData(cselfBuffer);
    memset(pcm, 0, sizeof(CMouse));

    // Unlikly, but it is possible the Windows dialog has executed, ended, and
    // the CSelf structure is being / has been freed.
    EnterCriticalSection(&crit_sec);

    if ( ! params->dlgCSelf->dlgAllocated )
    {
        methodCanNotBeInvokedException(context, "new", DLG_HAS_ENDED_MSG, self);
        goto done_out;
    }

    pcm->rexxSelf        = self;
    pcm->dlgCSelf        = params->dlgCSelf;
    pcm->hDlg            = params->dlgCSelf->hDlg;
    pcm->rexxDlg         = params->dlgCSelf->rexxSelf;
    pcm->dlgProcThreadID = params->dlgCSelf->dlgProcThreadID;
    pcm->isDlgWindow     = params->isDlgWindow;
    pcm->controlCSelf    = params->controlCSelf;

    if ( pcm->isDlgWindow )
    {
        pcm->hWindow     = pcm->dlgCSelf->hDlg;
        pcm->rexxWindow  = pcm->dlgCSelf->rexxSelf;

        pcm->dlgCSelf->mouseCSelf = pcm;
        pcm->dlgCSelf->rexxMouse  = self;
    }
    else
    {
        pcm->hWindow    = pcm->controlCSelf->hCtrl;
        pcm->rexxWindow = pcm->controlCSelf->rexxSelf;

        pcm->controlCSelf->mouseCSelf = pcm;
        pcm->controlCSelf->rexxMouse  = self;
    }

    // Put this mouse object in the dialog's control bag to prevent it from
    // being garbage collected while the Rexx dialog object exists. It does not
    // matter if the mouse's owner window is a dialog control or a dialog.
    context->SendMessage1(pcm->rexxDlg, "PUTCONTROL", self);

done_out:
    LeaveCriticalSection(&crit_sec);
    return 0;
}

/** Mouse::uninit()
 *
 *
 */
RexxMethod1(RexxObjectPtr, mouse_uninit, CSELF, pCSelf)
{
    if ( pCSelf != NULLOBJECT )
    {
        pCMouse pcm = (pCMouse)pCSelf;

#if 1
        printf("Mouse::uninit() Dlg CSelf=%p\n", pcm->dlgCSelf);
#endif

        EnterCriticalSection(&crit_sec);

        if ( pcm->dlgCSelf != NULL )
        {
            pcm->dlgCSelf->mouseCSelf = NULL;
            pcm->dlgCSelf->rexxMouse  = NULLOBJECT;
        }

        LeaveCriticalSection(&crit_sec);

    }

    return TheZeroObj;
}

/** Mouse::trackEvent()
 *
 *
 *  @notes  Requires the underlying dialog to exist.
 */
RexxMethod4(RexxObjectPtr, mouse_trackEvent, OPTIONAL_CSTRING, event, OPTIONAL_uint32_t, hoverTime,
            OPTIONAL_RexxObjectPtr, _answer, CSELF, pCSelf)
{
    TRACKMOUSEEVENT tme = {0};

    pCMouse pcm = requiredDlg(context, pCSelf);
    if ( pcm == NULL )
    {
        goto error_out;
    }

    tme.cbSize = sizeof(TRACKMOUSEEVENT);

    DWORD flags = 0;
    if ( argumentOmitted(1) )
    {
        flags = TME_LEAVE;
    }
    else
    {
        if ( StrStrI(event, "CANCEL"   ) != NULL ) flags =  TME_CANCEL;
        if ( StrStrI(event, "HOVER"    ) != NULL ) flags |= TME_HOVER;
        if ( StrStrI(event, "LEAVE"    ) != NULL ) flags |= TME_LEAVE;
        if ( StrStrI(event, "NONCLIENT") != NULL ) flags |= TME_NONCLIENT;
        if ( StrStrI(event, "QUERY"    ) != NULL ) flags |= TME_QUERY;
    }

    if ( flags & TME_QUERY )
    {
        if ( argumentOmitted(3) )
        {
            userDefinedMsgException(context->threadContext, 3, "is required when the QUERY keyword is used");
            goto error_out;
        }

        if ( ! requiredClass(context->threadContext, _answer, "Directory", 3) )
        {
            goto error_out;
        }
        RexxDirectoryObject answer = (RexxDirectoryObject)_answer;

        tme.dwFlags = TME_QUERY;

        if ( TrackMouseEvent(&tme) == 0 )
        {
            oodSetSysErrCode(context->threadContext);
            goto error_out;
        }

        char buf[512] = {'\0'};
        if ( tme.dwFlags & TME_CANCEL    ) strcat(buf, "CANCEL ");
        if ( tme.dwFlags & TME_HOVER     ) strcat(buf, "HOVER ");
        if ( tme.dwFlags & TME_LEAVE     ) strcat(buf, "LEAVE ");
        if ( tme.dwFlags & TME_NONCLIENT ) strcat(buf, "NONCLIENT ");
        if ( tme.dwFlags & TME_QUERY     ) strcat(buf, "QUERY ");

        buf[strlen(buf)] = '\0';

        context->DirectoryPut(answer, context->String(buf), "EVENT");
        context->DirectoryPut(answer, pointer2string(context, tme.hwndTrack), "HWND");
        context->DirectoryPut(answer, context->UnsignedInt32(tme.dwHoverTime), "HOVERTIME");

        goto good_out;
    }
    else
    {
        tme.dwFlags   = flags;
        tme.hwndTrack = pcm->hWindow;

        if ( flags & TME_HOVER )
        {
            if ( argumentOmitted(2) )
            {
                tme.dwHoverTime = HOVER_DEFAULT;
            }
            else
            {
                tme.dwHoverTime = hoverTime;
            }
        }

        if ( TrackMouseEvent(&tme) == 0 )
        {
            oodSetSysErrCode(context->threadContext);
            goto error_out;
        }
    }

good_out:
    return TheTrueObj;

error_out:
    return TheFalseObj;
}


/** Mouse::dragDetect()
 *
 *
 *  @notes  Requires the underlying dialog to exist.
 */
RexxMethod2(RexxObjectPtr, mouse_dragDetect, RexxObjectPtr, _pt, CSELF, pCSelf)
{
    pCMouse pcm = requiredDlg(context, pCSelf);
    if ( pcm == NULL )
    {
        return TheFalseObj;
    }

    PPOINT pt = rxGetPoint(context, _pt, 1);
    if ( pt == NULL )
    {
        return TheFalseObj;
    }
    return DragDetect(pcm->hWindow, *pt) ? TheTrueObj : TheFalseObj;
}


/** Mouse::getCapture()
 *
 *  Retrieves a handle to the window (if any) that has captured the mouse.
 *
 *  Only one window at a time can capture the mouse; this window receives mouse
 *  input whether or not the cursor is within its borders.
 *
 *  @return  The handle of the window, in this thread, that had previously
 *           captured the mouse, or 0 if no window previosly had the capture.  A
 *           0 (NULL) return value means that no window in the current thread
 *           has captured the mouse. However, it is possible that another
 *           thread or process has captured the mouse.
 *
/** Mouse::releaseCapture()
 *
 *  Releases the mouse capture from a window in the current thread and restores
 *  normal mouse input processing.
 *
 *  @return  0 on success, 1 on error.
 *
 *  @note  Sets the .SystemErrorCode, but that only has meaning for
 *         releaseMouseCapture().
 *
 *  @remarks  GetCapture() and ReleaseCapture() need to run on the same thread
 *            as the dialog's message processing loop.  So we might need to use
 *            SendMessage with one of the custom window messages.  However, it
 *            is possible that we are already running in that thread.
 *
 *            Note that we do not need the owner window handle for these
 *            methods, just the dialog window handle.
 */
RexxMethod2(RexxObjectPtr, mouse_get_release_capture, NAME, method, CSELF, pCSelf)
{
    RexxObjectPtr result = TheZeroObj;

    pCMouse pcm = requiredDlg(context, pCSelf);
    if ( pcm != NULL )
    {
        HWND hDlg = pcm->hDlg;

        if ( *method == 'G' )
        {
            HWND hwnd;

            if ( isDlgThread(pcm->dlgCSelf) )
            {
                hwnd = GetCapture();
            }
            else
            {
                hwnd = (HWND)SendMessage(hDlg, WM_USER_GETSETCAPTURE, MF_GETCAPTURE, 0);
            }
            result = pointer2string(context, hwnd);
        }
        else
        {
            uint32_t rc = 0;

            if ( isDlgThread(pcm->dlgCSelf) )
            {
                if ( ReleaseCapture() == 0 )
                {
                    rc = GetLastError();
                }
            }
            else
            {
                rc = (uint32_t)SendMessage(hDlg, WM_USER_GETSETCAPTURE, MF_RELEASECAPTURE, 0);
            }

            if ( rc != 0 )
            {
                result = TheOneObj;
                oodSetSysErrCode(context->threadContext, rc);
            }
        }
    }
    return result;
}


/** Mouse::capture
 *
 *  Sets the mouse capture to the owner window of this mouse object.
 *
 *  capture() captures mouse input either when the mouse is over the window, or
 *  when the mouse button was pressed while the mouse was over the window and
 *  the button is still down. Only one window at a time can capture the mouse.
 *
 *  If the mouse cursor is over a window created by another thread, the system
 *  will direct mouse input to the specified window only if a mouse button is
 *  down.
 *
 *  @return  On success, the window handle of the window that previously had
 *           captured the mouse, or 0 if there was no such window. On error, -1.
 *
 *  @note  Sets the .SystemErrorCode,
 */
RexxMethod1(RexxObjectPtr, mouse_capture, CSELF, pCSelf)
{
    RexxObjectPtr result = TheNegativeOneObj;

    pCMouse pcm = requiredDlg(context, pCSelf);
    if ( pcm != NULL )
    {
        HWND oldCapture;

        if ( isDlgThread(pcm->dlgCSelf) )
        {
            oldCapture = SetCapture(pcm->hWindow);
        }
        else
        {
            oldCapture = (HWND)SendMessage(pcm->hDlg, WM_USER_GETSETCAPTURE, MF_SETCAPTURE, (LPARAM)pcm->hWindow);
        }
        result = pointer2string(context, oldCapture);
    }
    return result;
}


/** Mouse::isButtonDown()
 *
 *  Determines if one of the mouse buttons is down.
 *
 *  @param  whichButton  [OPTIONAL]  Keyword indicating which mouse button
 *                       should be queried. By default it is the left button
 *                       that is queried.
 *
 *  @return  True if the specified mouse button was down, otherwise false
 *
 *  @note  Sets the .SystemErrorCode, but there is nothing that would change it
 *         to not zero.
 *
 *  @remarks  The key state must be handled in the window thread, so
 *            SendMessage() has to be used.
 */
RexxMethod2(RexxObjectPtr, mouse_isButtonDown, OPTIONAL_CSTRING, whichButton, CSELF, pCSelf)
{
    pCMouse pcm = requiredDlg(context, pCSelf);
    if ( pcm == NULL )
    {
        return TheFalseObj;
    }

    int32_t mb = VK_LBUTTON;
    if ( argumentExists(1) )
    {
        if ( StrCmpI(whichButton,      "LEFT" )    == 0 ) mb = VK_LBUTTON;
        else if ( StrCmpI(whichButton, "RIGHT" )   == 0 ) mb = VK_RBUTTON;
        else if ( StrCmpI(whichButton, "MIDDLE")   == 0 ) mb = VK_MBUTTON;
        else if ( StrCmpI(whichButton, "XBUTTON1") == 0 ) mb = VK_XBUTTON1;
        else if ( StrCmpI(whichButton, "XBUTTON2") == 0 ) mb = VK_XBUTTON2;
        else
        {
            return wrongArgValueException(context->threadContext, 1, MOUSE_BUTTON_KEYWORDS, whichButton);
        }
    }

    if ( GetSystemMetrics(SM_SWAPBUTTON) )
    {
        if ( mb == VK_LBUTTON )
        {
            mb = VK_RBUTTON;
        }
        else if ( mb == VK_RBUTTON )
        {
            mb = VK_LBUTTON;
        }
    }

    short state;
    if ( isDlgThread(pcm->dlgCSelf) )
    {
        state = (short)SendMessage(pcm->hDlg, WM_USER_GETKEYSTATE, mb, 0);
    }
    else
    {
        state = (short)GetAsyncKeyState(mb);
    }

    return (state & ISDOWN) ? TheTrueObj : TheFalseObj;
}

/** Mouse::setCursorPos()
 *
 *  Moves the cursor to the specified position.
 *
 *  @param  newPos  The new position (x, y), in pixels. The amount can be
 *                  specified in these formats:
 *
 *      Form 1:  A .Point object.
 *      Form 2:  x, y
 *
 *  @return  0 for success, 1 on error.
 *
 *  @note  Sets the .SystemErrorCode.
 *
 *  @remarks  No effort is made to ensure that a .Size object, not a .Point
 *            object is used.
 *
 *            There is no requirement that the underlying dialog exists.
 */
RexxMethod2(RexxObjectPtr, mouse_setCursorPos, ARGLIST, args, CSELF, pCSelf)
{
    pCMouse pcm = getMouseCSelf(context, pCSelf);
    if ( pcm == NULL )
    {
        return TheFalseObj;
    }

    size_t sizeArray;
    size_t argsUsed;
    POINT  point;
    if ( ! getPointFromArglist(context, args, &point, 1, 2, &sizeArray, &argsUsed) )
    {
        return NULLOBJECT;
    }

    if ( argsUsed == 1 && sizeArray == 2)
    {
        return tooManyArgsException(context->threadContext, 1);
    }

    if ( SetCursorPos(point.x, point.y) == 0 )
    {
        oodSetSysErrCode(context->threadContext);
        return TheOneObj;
    }
    return TheZeroObj;
}


/** Mouse::getCursorPos()
 *
 *  Retrieves the current cursor position in pixels.
 *
 *  @return The cursor position as a .Point object.
 *
 *  @note  Sets the .SystemErrorCode.
 *
 *  @remarks  There is no requirement that the underlying dialog exists.
 */
RexxMethod1(RexxObjectPtr, mouse_getCursorPos, CSELF, pCSelf)
{
    pCMouse pcm = getMouseCSelf(context, pCSelf);
    if ( pcm == NULL )
    {
        return TheFalseObj;
    }

    POINT p = {0};
    if ( GetCursorPos(&p) == 0 )
    {
        oodSetSysErrCode(context->threadContext);
    }
    return rxNewPoint(context, p.x, p.y);
}


/** Mouse::setCursor()
 *
 *  Sets the cursor shape for the owner window of this mouse.
 *
 *  @param  cursor  [Required] Identifies the cursor to be set.  This argument
 *                  can either be a cursor .Image, or a cursor keyword.
 *
 *                  A cursor keyword sets the cursor to one of the System's
 *                  predefined cursors.  Acceptable keywords are: APPSTARTING,
 *                  ARROW, CROSS, HAND, HELP, IBEAM, NO, SIZEALL, SIZENESW,
 *                  SIZENS, SIZENWSE, SIZEWE, UPARROW, or WAIT
 *
 *  Note that this method implementation also provides the implementation for
 *  some shortcut methods carried over from pre 4.2.0, see the remarks.  To do
 *  this we make the first arguement optional, and if the method is actually
 *  setCursor(), we then require the argument.
 *
 *  Pre 4.2.0 only supported these methods, there was no setCursorShape, or
 *  setCursor, so there is no default cursor.  (Only restoreCursorShape() had a
 *  default cursor of IDC_ARROW.)
 *
 *
 *
 *  WindowExtensions::Cursor_Arrow()
 *  WindowExtensions::Cursor_AppStarting()
 *  WindowExtensions::Cursor_Cross()
 *  WindowExtensions::Cursor_No()
 *  WindowExtensions::Cursor_Wait()
 *
 *  For 4.2.0, we only support these short cut methods, which map to the old
 *  methods as is logical:
 *
 *  arrow()
 *  appStarting()
 *  cross()
 *  no()
 *  wait()
 *
 *
 *  old method name, needs to be deleted: winex_setCursorShape
 *
 *
 *
 */
RexxMethod3(RexxObjectPtr, mouse_setCursor, OPTIONAL_RexxObjectPtr, _cursor, NAME, method, CSELF, pCSelf)
{
    RexxObjectPtr result = TheZeroObj;

    pCMouse pcm = getMouseCSelf(context, pCSelf);
    if ( pcm == NULL )
    {
        goto done_out;
    }

    CSTRING cursor = NULL;
    HCURSOR hCursor = NULL;

    if ( StrCmp(method, "SETCURSOR") == 0 )
    {
        if ( argumentOmitted(1) )
        {
            missingArgException(context->threadContext, 1);
            goto done_out;
        }

        if ( context->IsOfType(_cursor, "IMAGE") )
        {
            POODIMAGE poiCursor = rxGetImageCursor(context, _cursor, 1);
            if ( poiCursor == NULLOBJECT )
            {
                goto done_out;
            }
            hCursor = (HCURSOR)poiCursor->hImage;
        }
        else if ( context->IsString(_cursor))
        {
            cursor = keyword2cursor(context, context->ObjectToStringValue(_cursor));
            if ( cursor == NULL )
            {
                goto done_out;
            }
        }
        else
        {
            wrongArgValueException(context->threadContext, 1, "Image or String", _cursor);
            goto done_out;
        }
    }
    else
    {
        switch ( *method )
        {
            case 'A' :
                cursor = (method[1] == 'R' ? IDC_ARROW : IDC_APPSTARTING);
                break;
            case 'C' :
                cursor = IDC_CROSS;
                break;
            case 'N' :
                cursor = IDC_NO;
                break;
            case 'W' :
                cursor = IDC_WAIT;
                break;
            default :
                // Should be impossible, so we don't raise and internal error exception.
                goto done_out;
        }
    }

    if ( cursor != NULL )
    {
        hCursor = (HCURSOR)LoadImage(NULL, cursor, IMAGE_CURSOR, 0, 0, LR_DEFAULTSIZE | LR_SHARED);
        if ( hCursor == NULL )
        {
            oodSetSysErrCode(context->threadContext);
            goto done_out;
        }
    }

    result = mouseSetCursor(context, pcm, hCursor);

done_out:
    return result;
}

/** Mouse::restoreCursor()
 *
 *  Restores the cursor.  This is a convenience method, the same function could
 *  be achieved using setCursor()
 *
 *  @param  cursor  [OPTIONAL]  A cursor .Image to set the cursor to. If this
 *                     argument is omitted, the cursor is set to the System
 *                     arrow cursor.  See the remarks.
 *
 *  @return  The previous cursor, or a null handle if there was no previous
 *           cursor.
 *
 *  @note  Best practice would most likely be to use restoreCursor() to restore
 *         a saved cursor, returned from setCursor()
 *
 *         Sets the .SystemErrorCode.
 */
RexxMethod2(RexxObjectPtr, mouse_restoreCursor, OPTIONAL_RexxObjectPtr, newCursor, CSELF, pCSelf)
{
    RexxObjectPtr result = TheZeroObj;

    pCMouse pcm = getMouseCSelf(context, pCSelf);
    if ( pcm == NULL )
    {
        goto done_out;
    }

    HCURSOR hCursor;
    if ( argumentExists(1) )
    {
        if ( context->IsOfType(newCursor, "IMAGE") )
        {
            POODIMAGE poiCursor = rxGetImageCursor(context, newCursor, 1);
            if ( poiCursor == NULLOBJECT )
            {
                goto done_out;
            }
            hCursor = (HCURSOR)poiCursor->hImage;
        }
        else
        {
            wrongClassException(context->threadContext, 1, "Image");
            goto done_out;
        }
    }
    else
    {
        hCursor = (HCURSOR)LoadImage(NULL, IDC_ARROW, IMAGE_CURSOR, 0, 0, LR_DEFAULTSIZE | LR_SHARED);
        if ( hCursor == NULL )
        {
            oodSetSysErrCode(context->threadContext);
            goto done_out;
        }
    }

    result = mouseSetCursor(context, pcm, hCursor);

done_out:
    return result;
}


/** Mouse::connectEvent()
 *
 *
 * @return  True on success, false on error.
 */
RexxMethod4(RexxObjectPtr, mouse_connectEvent, CSTRING, event, OPTIONAL_CSTRING, methodName,
            OPTIONAL_logical_t, _willReply, CSELF, pCSelf)
{
    pCEventNotification pcen = getMousePCEN(context, (pCMouse)pCSelf);
    if ( pcen == NULL )
    {
        goto err_out;
    }

    uint32_t wmMsg;
    if ( ! keyword2wm(context, event, &wmMsg) )
    {
        goto err_out;
    }

    if ( argumentOmitted(2) || *methodName == '\0' )
    {
        methodName = wm2name(wmMsg);
    }

    uint32_t tag = TAG_MOUSE;
    bool willReply = argumentOmitted(3) || _willReply;

    tag |= willReply ? TAG_REPLYFROMREXX : 0;

    if ( addMiscMessage(pcen, context, wmMsg, 0xFFFFFFFF, 0, 0, 0, 0, methodName, tag) )
    {
        return TheTrueObj;
    }

err_out:
    return TheFalseObj;
}


/** Mouse::Test()
 *
 * Notes on mouse and CS_DBLCLKS style.  Dialogs already have this style, no
 * need for a method to add the style.  Buttons have this style.
 *
 */
RexxMethod1(RexxObjectPtr, mouse_test, CSELF, pCSelf)
{
    pCMouse pcm = requiredDlg(context, pCSelf);
    if ( pcm != NULL )
    {
        HWND hWindow = pcm->hWindow;

        ULONG_PTR style = getClassPtr(hWindow, GCL_STYLE);
        printf("Dialog Control Class style=0x%08x\n", style);
        style |= CS_DBLCLKS;
        setClassPtr(hWindow, GCL_STYLE, style);
        printf("Adding dblclks new style=0x%08x last error=%d\n", getClassPtr(hWindow, GCL_STYLE), GetLastError());
    }
    return TheTrueObj;
}


