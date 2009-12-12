/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 1995, 2004 IBM Corporation. All rights reserved.             */
/* Copyright (c) 2005-2009 Rexx Language Association. All rights reserved.    */
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
 * oodViewControls.cpp
 *
 * Contains methods for the List-view, Tree-view, DateTimePicker, and
 * MonthCalendar controls.
 */
#include "ooDialog.hpp"     // Must be first, includes windows.h and oorexxapi.h

#include <commctrl.h>
#include <shlwapi.h>

#include "APICommon.hpp"
#include "oodCommon.hpp"
#include "oodControl.hpp"
#include "oodResources.hpp"

/**
 * This is the window procedure used to subclass the edit control for both the
 * ListControl and TreeControl objects.  It would be nice to convert this to use
 * the better API: SetWindowSubclass / RemoveWindowSubclass.
 */
WNDPROC wpOldEditProc = NULL;

LONG_PTR CALLBACK CatchReturnSubProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch ( uMsg )
    {
        case WM_GETDLGCODE:
            return (DLGC_WANTALLKEYS | CallWindowProc(wpOldEditProc, hWnd, uMsg, wParam, lParam));

        case WM_CHAR:
             //Process this message to avoid message beeps.
            if ((wParam == VK_RETURN) || (wParam == VK_ESCAPE))
                return 0;
            else
                return CallWindowProc(wpOldEditProc, hWnd,uMsg, wParam, lParam);

        default:
            return CallWindowProc(wpOldEditProc, hWnd, uMsg, wParam, lParam);
    }
}



/**
 * Methods for the DateTimePicker class.
 */
#define DATETIMEPICKER_CLASS     "DateTimePicker"
#define DATETIMEPICKER_WINNAME   "Date and Time Picker"

// This is used for MonthCalendar also
#define SYSTEMTIME_MIN_YEAR    1601

enum DateTimePart {dtFull, dtTime, dtDate, dtNow};

/**
 * Converts a DateTime object to a SYSTEMTIME structure.  The fields of the
 * struct are filled in with the corresponding values of the DateTime object.
 *
 * @param c         The method context we are operating in.
 * @param dateTime  An ooRexx DateTime object.
 * @param sysTime   [in/out] The SYSTEMTIME struct to fill in.
 * @param part      Specifies which fields of the SYSTEMTIME struct fill in.
 *                  Unspecified fields are left alone.
 *
 * @return True if no errors, false if a condition is raised.
 *
 * @note  Assumes the dateTime object is not null and is actually a DateTime
 *        object.
 *
 * @note The year part of the DateTime object must be in range for a SYSTEMTIME.
 *       The lower range for SYSTEMTIME is 1601. The upper range of a DateTime
 *       object is 9999 and of a SYSTEMTIME 30827, so we only check the lower
 *       range.  An exception is raised if out of range.
 */
static bool dt2sysTime(RexxMethodContext *c, RexxObjectPtr dateTime, SYSTEMTIME *sysTime, DateTimePart part)
{
    if ( part == dtNow )
    {
        GetLocalTime(sysTime);
    }
    else
    {
        // format: yyyy-dd-mmThh:mm:ss.uuuuuu.
        RexxObjectPtr dt = c->SendMessage0(dateTime, "ISODATE");
        const char *isoDate = c->CString(dt);

        sscanf(isoDate, "%4hu-%2hu-%2huT%2hu:%2hu:%2hu.%3hu", &(*sysTime).wYear, &(*sysTime).wMonth, &(*sysTime).wDay,
               &(*sysTime).wHour, &(*sysTime).wMinute, &(*sysTime).wSecond, &(*sysTime).wMilliseconds);

        SYSTEMTIME st = {0};
        sscanf(isoDate, "%4hu-%2hu-%2huT%2hu:%2hu:%2hu.%3hu", &st.wYear, &st.wMonth, &st.wDay,
               &st.wHour, &st.wMinute, &st.wSecond, &st.wMilliseconds);

        if ( st.wYear < SYSTEMTIME_MIN_YEAR )
        {
            userDefinedMsgException(c->threadContext, "The DateTime object can not represent a year prior to 1601");
            goto failed_out;
        }

        switch ( part )
        {
            case dtTime :
                sysTime->wHour = st.wHour;
                sysTime->wMinute = st.wMinute;
                sysTime->wSecond = st.wSecond;
                sysTime->wMilliseconds = st.wMilliseconds;
                break;

            case dtDate :
                sysTime->wYear = st.wYear;
                sysTime->wMonth = st.wMonth;
                sysTime->wDay = st.wDay;
                break;

            case dtFull :
                sysTime->wYear = st.wYear;
                sysTime->wMonth = st.wMonth;
                sysTime->wDay = st.wDay;
                sysTime->wHour = st.wHour;
                sysTime->wMinute = st.wMinute;
                sysTime->wSecond = st.wSecond;
                sysTime->wMilliseconds = st.wMilliseconds;
                break;
        }
    }
    return true;

failed_out:
    return false;
}

/**
 * Creates a DateTime object that represents the time set in a SYSTEMTIME
 * struct.
 *
 * @param c
 * @param sysTime
 * @param dateTime  [in/out]
 */
static void sysTime2dt(RexxMethodContext *c, SYSTEMTIME *sysTime, RexxObjectPtr *dateTime, DateTimePart part)
{
    RexxClassObject dtClass = c->FindClass("DATETIME");

    if ( part == dtNow )
    {
        *dateTime = c->SendMessage0(dtClass, "NEW");
    }
    else
    {
        char buf[64];
        switch ( part )
        {
            case dtDate :
                _snprintf(buf, sizeof(buf), "%hu%02hu%02hu", sysTime->wYear, sysTime->wMonth, sysTime->wDay);
                *dateTime = c->SendMessage1(dtClass, "FROMSTANDARDDATE", c->String(buf));
                break;

            case dtTime :
                _snprintf(buf, sizeof(buf), "%02hu:%02hu:%02hu.%03hu000",
                          sysTime->wHour, sysTime->wMinute, sysTime->wSecond, sysTime->wMilliseconds);
                *dateTime = c->SendMessage1(dtClass, "FROMLONGTIME", c->String(buf));
                break;

            case dtFull :
                _snprintf(buf, sizeof(buf), "%hu-%02hu-%02huT%02hu:%02hu:%02hu.%03hu000",
                          sysTime->wYear, sysTime->wMonth, sysTime->wDay,
                          sysTime->wHour, sysTime->wMinute, sysTime->wSecond, sysTime->wMilliseconds);
                *dateTime = c->SendMessage1(dtClass, "FROMISODATE", c->String(buf));
                break;
        }
    }
}

/** DateTimePicker::dateTime  (attribute)
 *
 *  Retrieves the current selected system time of the date time picker and
 *  returns it as a DateTime object.
 *
 *  If the date time picker has the DTS_SHOWNONE style, it can also be set to
 *  "no date" when the user has unchecked the check box.  If the control is in
 *  this state, the .NullHandle object is returned to the user.
 *
 *  @returns  A DateTime object representing the current selected system time of
 *            the control, or the .NullHandle object if the control is in the
 *            'no date' state.
 */
RexxMethod1(RexxObjectPtr, get_dtp_dateTime, OSELF, self)
{
    RexxMethodContext *c = context;
    SYSTEMTIME sysTime = {0};
    RexxObjectPtr dateTime = NULLOBJECT;

    switch ( DateTime_GetSystemtime(rxGetWindowHandle(context, self), &sysTime) )
    {
        case GDT_VALID:
            sysTime2dt(context, &sysTime, &dateTime, dtFull);
            break;

        case GDT_NONE:
            // This is valid.  It means the DTP is using the DTS_SHOWNONE  style
            // and that the user has the check box is not checked.  We return a
            // null pointer object.
            dateTime = c->NewPointer(NULL);
            break;

        case GDT_ERROR:
        default :
            // Some error with the DTP, raise an exception.
            controlFailedException(context->threadContext, FUNC_WINCTRL_FAILED_MSG, "DateTime_GetSystemtime", DATETIMEPICKER_WINNAME);
            break;
    }
    return dateTime;
}

/** DateTimePicker::dateTime=  (attribute)
 *
 *  Sets the system time for the date time picker to the time represented by the
 *  DateTime object.  If, and only if, the date time picker has the DTS_SHOWNONE
 *  style, it can also be set to "no date."  The Rexx user can set this state by
 *  passing in the .NullHandle object.
 *
 *  @param dateTime  The date and time to set the control to.
 *
 *  @return   This is an attribute, there is no return.
 *
 *  @note  The minimum year a date time picker can be set to is 1601.  If the
 *         DateTime object represents a year prior to 1601, an exception is
 *         raised.
 *
 */
RexxMethod2(RexxObjectPtr, set_dtp_dateTime, RexxObjectPtr, dateTime, OSELF, self)
{
    RexxMethodContext *c = context;
    SYSTEMTIME sysTime = {0};
    HWND hwnd = rxGetWindowHandle(context, self);

    if ( c->IsOfType(dateTime, "POINTER") )
    {
        DateTime_SetSystemtime(hwnd, GDT_NONE, &sysTime);
    }
    else
    {
        if ( requiredClass(context->threadContext, dateTime, "DATETIME", 1) )
        {
            if ( dt2sysTime(c, dateTime, &sysTime, dtFull) )
            {
                if ( DateTime_SetSystemtime(hwnd, GDT_VALID, &sysTime) == 0 )
                {
                    controlFailedException(context->threadContext, FUNC_WINCTRL_FAILED_MSG, "DateTime_SetSystemtime", DATETIMEPICKER_WINNAME);
                }
            }
        }
    }
    return NULLOBJECT;
}


/**
 * Methods for the MonthCalendar class.
 */
#define MONTHCALENDAR_CLASS    "MonthCalendar"
#define MONTHCALENDAR_WINNAME  "Month Calendar"

RexxMethod1(RexxObjectPtr, get_mc_date, OSELF, self)
{
    RexxMethodContext *c = context;
    SYSTEMTIME sysTime = {0};
    RexxObjectPtr dateTime = NULLOBJECT;

    if ( MonthCal_GetCurSel(rxGetWindowHandle(context, self), &sysTime) == 0 )
    {
        controlFailedException(context->threadContext, FUNC_WINCTRL_FAILED_MSG, "MonthCal_GetCurSel", MONTHCALENDAR_WINNAME);
    }
    else
    {
        sysTime2dt(context, &sysTime, &dateTime, dtDate);
    }
    return dateTime;
}

RexxMethod2(RexxObjectPtr, set_mc_date, RexxObjectPtr, dateTime, OSELF, self)
{
    RexxMethodContext *c = context;
    SYSTEMTIME sysTime = {0};

    if ( requiredClass(context->threadContext, dateTime, "DATETIME", 1) )
    {
        if ( dt2sysTime(context, dateTime, &sysTime, dtDate) )
        {
            if ( MonthCal_SetCurSel(rxGetWindowHandle(context, self), &sysTime) == 0 )
            {
                controlFailedException(context->threadContext, FUNC_WINCTRL_FAILED_MSG, "MonthCal_SetCurSel", MONTHCALENDAR_WINNAME);
            }
        }
    }
    return NULLOBJECT;
}

RexxMethod1(logical_t, get_mc_usesUnicode, OSELF, self)
{
    return MonthCal_GetUnicodeFormat(rxGetWindowHandle(context, self)) ? 1 : 0;
}

RexxMethod2(RexxObjectPtr, set_mc_usesUnicode, logical_t, useUnicode, OSELF, self)
{
    MonthCal_SetUnicodeFormat(rxGetWindowHandle(context, self), useUnicode);
    return NULLOBJECT;
}


/**
 *  Methods for the .ListControl class.
 */
#define LISTCONTROL_CLASS         "ListControl"

#define LVSTATE_ATTRIBUTE         "LV!STATEIMAGELIST"
#define LVSMALL_ATTRIBUTE         "LV!SMALLIMAGELIST"
#define LVNORMAL_ATTRIBUTE        "LV!NORMALIMAGELIST"

inline bool hasCheckBoxes(HWND hList)
{
    return ((ListView_GetExtendedListViewStyle(hList) & LVS_EX_CHECKBOXES) != 0);
}

/**
 * Checks that the list view is either in icon view, or small icon view.
 * Certain list view messages and functions are only applicable in those views.
 *
 * Note that LVS_ICON == 0 so LVS_TYPEMASK must be used.
 */
inline bool isInIconView(HWND hList)
{
    uint32_t style = (uint32_t)GetWindowLong(hList, GWL_STYLE);
    return ((style & LVS_TYPEMASK) == LVS_ICON) || ((style & LVS_TYPEMASK) == LVS_SMALLICON);
}

inline int getColumnCount(HWND hList)
{
    return Header_GetItemCount(ListView_GetHeader(hList));
}

inline CSTRING getLVAttributeName(uint8_t type)
{
    switch ( type )
    {
        case LVSIL_STATE :
            return LVSTATE_ATTRIBUTE;
        case LVSIL_SMALL :
            return LVSMALL_ATTRIBUTE;
        case LVSIL_NORMAL :
        default :
            return LVNORMAL_ATTRIBUTE;
    }
}

/**
 * Change the window style of a list view to align left or align top.
 */
static void applyAlignStyle(HWND hList, bool doTop)
{
    uint32_t flag = (doTop ? LVS_ALIGNTOP : LVS_ALIGNLEFT);

    uint32_t style = (uint32_t)GetWindowLong(hList, GWL_STYLE);
    SetWindowLong(hList, GWL_STYLE, ((style & ~LVS_ALIGNMASK) | flag));

    int count = ListView_GetItemCount(hList);
    if ( count > 0 )
    {
        count--;
        ListView_RedrawItems(hList, 0, count);
        UpdateWindow(hList);
    }
}

/**
 * Parse a list-view control extended style string sent from ooDialog into the
 * corresponding style flags.
 *
 * The extended list-view styles are set (and retrieved) in a different manner
 * than other window styles.  This function is used only to parse those extended
 * styles.  The normal list-view styles are parsed using EvaluateListStyle.
 */
static uint32_t parseExtendedStyle(const char * style)
{
    uint32_t dwStyle = 0;

    if ( strstr(style, "BORDERSELECT"    ) ) dwStyle |= LVS_EX_BORDERSELECT;
    if ( strstr(style, "CHECKBOXES"      ) ) dwStyle |= LVS_EX_CHECKBOXES;
    if ( strstr(style, "FLATSB"          ) ) dwStyle |= LVS_EX_FLATSB;
    if ( strstr(style, "FULLROWSELECT"   ) ) dwStyle |= LVS_EX_FULLROWSELECT;
    if ( strstr(style, "GRIDLINES"       ) ) dwStyle |= LVS_EX_GRIDLINES;
    if ( strstr(style, "HEADERDRAGDROP"  ) ) dwStyle |= LVS_EX_HEADERDRAGDROP;
    if ( strstr(style, "INFOTIP"         ) ) dwStyle |= LVS_EX_INFOTIP;
    if ( strstr(style, "MULTIWORKAREAS"  ) ) dwStyle |= LVS_EX_MULTIWORKAREAS;
    if ( strstr(style, "ONECLICKACTIVATE") ) dwStyle |= LVS_EX_ONECLICKACTIVATE;
    if ( strstr(style, "REGIONAL"        ) ) dwStyle |= LVS_EX_REGIONAL;
    if ( strstr(style, "SUBITEMIMAGES"   ) ) dwStyle |= LVS_EX_SUBITEMIMAGES;
    if ( strstr(style, "TRACKSELECT"     ) ) dwStyle |= LVS_EX_TRACKSELECT;
    if ( strstr(style, "TWOCLICKACTIVATE") ) dwStyle |= LVS_EX_TWOCLICKACTIVATE;
    if ( strstr(style, "UNDERLINECOLD"   ) ) dwStyle |= LVS_EX_UNDERLINECOLD;
    if ( strstr(style, "UNDERLINEHOT"    ) ) dwStyle |= LVS_EX_UNDERLINEHOT;

    // Needs Comctl32.dll version 5.8 or higher
    if ( ComCtl32Version >= COMCTL32_5_8 )
    {
      if ( strstr(style, "LABELTIP") ) dwStyle |= LVS_EX_LABELTIP;
    }

    // Needs Comctl32 version 6.0 or higher
    if ( ComCtl32Version >= COMCTL32_6_0 )
    {
      if ( strstr(style, "DOUBLEBUFFER") ) dwStyle |= LVS_EX_DOUBLEBUFFER;
      if ( strstr(style, "SIMPLESELECT") ) dwStyle |= LVS_EX_SIMPLESELECT;
    }
    return dwStyle;
}


/**
 * Change a list-view's style.
 *
 * @param c
 * @param pCSelf
 * @param _style
 * @param _additionalStyle
 * @param remove
 *
 * @return uint32_t
 *
 *  @remarks  MSDN suggests setting last error to 0 before calling
 *            GetWindowLong() as the correct way to determine error.
 */
static uint32_t changeStyle(RexxMethodContext *c, pCDialogControl pCSelf, CSTRING _style, CSTRING _additionalStyle, bool remove)
{
    oodResetSysErrCode(c->threadContext);
    SetLastError(0);

    HWND     hList = getDCHCtrl(pCSelf);
    uint32_t oldStyle = (uint32_t)GetWindowLong(hList, GWL_STYLE);

    if ( oldStyle == 0 && GetLastError() != 0 )
    {
        goto err_out;
    }

    uint32_t newStyle = 0;
    if ( remove )
    {
        newStyle &= ~listViewStyle(_style, 0);
        if ( _additionalStyle != NULL )
        {
            newStyle = listViewStyle(_additionalStyle, newStyle);
        }
    }
    else
    {
        newStyle = listViewStyle(_style, oldStyle);
    }

    if ( SetWindowLong(hList, GWL_STYLE, newStyle) == 0 && GetLastError() != 0 )
    {
        goto err_out;
    }
    return oldStyle;

err_out:
    oodSetSysErrCode(c->threadContext);
    return 0;
}


/**
 * Produce a string representation of a List-View's extended styles.
 */
static RexxStringObject extendedStyleToString(RexxMethodContext *c, HWND hList)
{
    char buf[256];
    DWORD dwStyle = ListView_GetExtendedListViewStyle(hList);
    buf[0] = '\0';

    if ( dwStyle & LVS_EX_BORDERSELECT )     strcat(buf, "BORDERSELECT ");
    if ( dwStyle & LVS_EX_CHECKBOXES )       strcat(buf, "CHECKBOXES ");
    if ( dwStyle & LVS_EX_FLATSB )           strcat(buf, "FLATSB ");
    if ( dwStyle & LVS_EX_FULLROWSELECT )    strcat(buf, "FULLROWSELECT ");
    if ( dwStyle & LVS_EX_GRIDLINES )        strcat(buf, "GRIDLINES ");
    if ( dwStyle & LVS_EX_HEADERDRAGDROP )   strcat(buf, "HEADERDRAGDROP ");
    if ( dwStyle & LVS_EX_INFOTIP )          strcat(buf, "INFOTIP ");
    if ( dwStyle & LVS_EX_MULTIWORKAREAS )   strcat(buf, "MULTIWORKAREAS ");
    if ( dwStyle & LVS_EX_ONECLICKACTIVATE ) strcat(buf, "ONECLICKACTIVATE ");
    if ( dwStyle & LVS_EX_REGIONAL )         strcat(buf, "REGIONAL ");
    if ( dwStyle & LVS_EX_SUBITEMIMAGES )    strcat(buf, "SUBITEMIMAGES ");
    if ( dwStyle & LVS_EX_TRACKSELECT )      strcat(buf, "TRACKSELECT ");
    if ( dwStyle & LVS_EX_TWOCLICKACTIVATE ) strcat(buf, "TWOCLICKACTIVATE ");
    if ( dwStyle & LVS_EX_UNDERLINECOLD )    strcat(buf, "UNDERLINECOLD ");
    if ( dwStyle & LVS_EX_UNDERLINEHOT )     strcat(buf, "UNDERLINEHOT ");
    if ( dwStyle & LVS_EX_LABELTIP )         strcat(buf, "LABELTIP ");
    if ( dwStyle & LVS_EX_DOUBLEBUFFER )     strcat(buf, "DOUBLEBUFFER ");
    if ( dwStyle & LVS_EX_SIMPLESELECT )     strcat(buf, "SIMPLESELECT ");

    return c->String(buf);
}


static int getColumnWidthArg(RexxMethodContext *context, RexxObjectPtr _width, size_t argPos)
{
    int width = OOD_BAD_WIDTH_EXCEPTION;

    if ( argumentOmitted(argPos) )
    {
        width = LVSCW_AUTOSIZE;
    }
    else
    {
        CSTRING tmpWidth = context->ObjectToStringValue(_width);

        if ( stricmp(tmpWidth, "AUTO") == 0 )
        {
            width = LVSCW_AUTOSIZE;
        }
        else if ( stricmp(tmpWidth, "AUTOHEADER") == 0 )
        {
            width = LVSCW_AUTOSIZE_USEHEADER;
        }
        else if ( ! context->Int32(_width, &width) )
        {
            wrongArgValueException(context->threadContext, argPos, "AUTO, AUTOHEADER, or a numeric value", _width);
        }
    }
    return width;
}


size_t RexxEntry HandleListCtrl(const char *funcname, size_t argc, CONSTRXSTRING *argv, const char *qname, RXSTRING *retstr)
{
   HWND h;

   CHECKARGL(3);

   h = GET_HWND(argv[2]);
   if (!h) RETERR;

   if (argv[0].strptr[0] == 'I')
   {
       if (!strcmp(argv[1].strptr, "INS"))
       {
           LV_ITEM lvi;

           CHECKARG(6);

           lvi.mask = LVIF_TEXT;

           lvi.iItem = atoi(argv[3].strptr);
           lvi.iSubItem = 0;

           lvi.pszText = (LPSTR)argv[4].strptr;
           lvi.cchTextMax = (int)argv[4].strlength;

           lvi.iImage = atoi(argv[5].strptr);
           if (lvi.iImage >= 0) lvi.mask |= LVIF_IMAGE;

           RETVAL(ListView_InsertItem(h, &lvi));
       }
       else
       if (!strcmp(argv[1].strptr, "SET"))
       {
           LV_ITEM lvi;

           CHECKARG(7);

           lvi.mask = 0;

           lvi.iItem = atoi(argv[3].strptr);
           lvi.iSubItem = atoi(argv[4].strptr);

           lvi.pszText = (LPSTR)argv[5].strptr;
           lvi.cchTextMax = (int)argv[5].strlength;

           if (!strcmp(argv[6].strptr,"TXT"))
           {
               lvi.mask |= LVIF_TEXT;
               RETC(!SendMessage(h, LVM_SETITEMTEXT, lvi.iItem, (LPARAM)&lvi));
           }
           else if (!strcmp(argv[6].strptr,"STATE"))
           {
               lvi.state = 0;
               lvi.stateMask = 0;

               if (strstr(argv[5].strptr, "NOTCUT"))  lvi.stateMask |= LVIS_CUT;
               else if (strstr(argv[5].strptr, "CUT"))  {lvi.state |= LVIS_CUT; lvi.stateMask |= LVIS_CUT;}
               if (strstr(argv[5].strptr, "NOTDROP"))  lvi.stateMask |= LVIS_DROPHILITED;
               else if (strstr(argv[5].strptr, "DROP"))  {lvi.state |= LVIS_DROPHILITED; lvi.stateMask |= LVIS_DROPHILITED;}
               if (strstr(argv[5].strptr, "NOTFOCUSED"))  lvi.stateMask |= LVIS_FOCUSED;
               else if (strstr(argv[5].strptr, "FOCUSED"))  {lvi.state |= LVIS_FOCUSED; lvi.stateMask |= LVIS_FOCUSED;}
               if (strstr(argv[5].strptr, "NOTSELECTED"))  lvi.stateMask |= LVIS_SELECTED;
               else if (strstr(argv[5].strptr, "SELECTED"))  {lvi.state |= LVIS_SELECTED; lvi.stateMask |= LVIS_SELECTED;}

               RETC(!SendMessage(h, LVM_SETITEMSTATE, lvi.iItem, (LPARAM)&lvi));
           }
           else
           {
               if (lvi.cchTextMax) lvi.mask |= LVIF_TEXT;

               lvi.iImage = atoi(argv[6].strptr);
               if (lvi.iImage >= 0) lvi.mask |= LVIF_IMAGE;
               RETC(!ListView_SetItem(h, &lvi));
           }
       }
       else
       if (!strcmp(argv[1].strptr, "GET"))
       {
           LV_ITEM lvi;
           CHAR data[256];

           CHECKARG(7);

           lvi.iItem = atoi(argv[3].strptr);
           lvi.iSubItem = atoi(argv[4].strptr);
           lvi.mask = LVIF_TEXT | LVIF_IMAGE | LVIF_STATE;
           lvi.pszText = data;
           lvi.cchTextMax = 255;
           lvi.stateMask = LVIS_CUT | LVIS_DROPHILITED | LVIS_FOCUSED | LVIS_SELECTED;

           if (!strcmp(argv[6].strptr,"TXT"))
           {
               INT len;
               lvi.pszText = retstr->strptr;
               len = (int)SendMessage(h, LVM_GETITEMTEXT, lvi.iItem, (LPARAM)&lvi);
               retstr->strlength = len;
               return 0;
           }
           else if (!strcmp(argv[6].strptr,"STATE"))
           {
               UINT state;

               state = ListView_GetItemState(h, lvi.iItem, lvi.stateMask);
               retstr->strptr[0] = '\0';
               if (state & LVIS_CUT) strcat(retstr->strptr, "CUT ");
               if (state & LVIS_DROPHILITED) strcat(retstr->strptr, "DROP ");
               if (state & LVIS_FOCUSED) strcat(retstr->strptr, "FOCUSED ");
               if (state & LVIS_SELECTED) strcat(retstr->strptr, "SELECTED ");
               retstr->strlength = strlen(retstr->strptr);
               return 0;
           }
           RETVAL(-1);
       }
       else
       if (!strcmp(argv[1].strptr, "DEL"))
       {
           INT item;
           CHECKARG(4);
           item = atoi(argv[3].strptr);
           if (!item && !strcmp(argv[3].strptr,"ALL"))
              RETC(!ListView_DeleteAllItems(h))
           else if (ListView_GetItemCount(h) >0)
              RETC(!ListView_DeleteItem(h, item))
           RETVAL(-1)
       }
       else
       if (!strcmp(argv[1].strptr, "GETNEXT"))
       {
           ULONG flag;
           LONG startItem;

           CHECKARG(5);

           startItem = atol(argv[3].strptr);

           if (!strcmp(argv[4].strptr, "FIRSTVISIBLE"))
               RETVAL(ListView_GetTopIndex(h))

           flag = 0;
           if (strstr(argv[4].strptr,"ABOVE")) flag |= LVNI_ABOVE;
           if (strstr(argv[4].strptr,"BELOW")) flag |= LVNI_BELOW;
           if (strstr(argv[4].strptr,"TOLEFT")) flag |= LVNI_TOLEFT;
           if (strstr(argv[4].strptr,"TORIGHT")) flag |= LVNI_TORIGHT;
           if (!flag) flag = LVNI_ALL;

           if (strstr(argv[4].strptr,"CUT")) flag |= LVNI_CUT;
           else if (strstr(argv[4].strptr,"DROP")) flag |= LVNI_DROPHILITED;
           else if (strstr(argv[4].strptr,"FOCUSED")) flag |= LVNI_FOCUSED;
           else if (strstr(argv[4].strptr,"SELECTED")) flag |= LVNI_SELECTED;

           RETVAL(ListView_GetNextItem(h, startItem, flag))
       }
       else
       if (!strcmp(argv[1].strptr, "FIND"))
       {
           LONG startItem;
           LV_FINDINFO finfo;

           CHECKARGL(6);

           startItem = atol(argv[3].strptr);

           if (strstr(argv[4].strptr,"NEAREST")) finfo.flags = LVFI_NEARESTXY;
           else finfo.flags = LVFI_STRING;

           if (strstr(argv[4].strptr,"PARTIAL")) finfo.flags |= LVFI_PARTIAL;
           if (strstr(argv[4].strptr,"WRAP")) finfo.flags |= LVFI_WRAP;

           if ((finfo.flags & LVFI_STRING) == LVFI_STRING)
               finfo.psz = argv[5].strptr;
           else {
               CHECKARG(8);
               finfo.pt.x = atol(argv[5].strptr);
               finfo.pt.y = atol(argv[6].strptr);
               if (!strcmp(argv[7].strptr,"UP")) finfo.vkDirection = VK_UP;
               else if (!strcmp(argv[7].strptr,"LEFT")) finfo.vkDirection  = VK_LEFT;
               else if (!strcmp(argv[7].strptr,"RIGHT")) finfo.vkDirection  = VK_RIGHT;
               else finfo.vkDirection  = VK_DOWN;
           }

           RETVAL(ListView_FindItem(h, startItem, &finfo))
       }
       else
       if (!strcmp(argv[1].strptr, "EDIT"))
       {
           CHECKARG(4);

           RETHANDLE(ListView_EditLabel(h, atol(argv[3].strptr)))
       }
       else
       if (!strcmp(argv[1].strptr, "SUBCL_EDIT"))
       {
           HWND ew = ListView_GetEditControl(h);
           if (ew)
           {
               WNDPROC oldProc = (WNDPROC)setWindowPtr(ew, GWLP_WNDPROC, (LONG_PTR)CatchReturnSubProc);
               if (oldProc != (WNDPROC)CatchReturnSubProc) wpOldEditProc = oldProc;
               RETPTR(oldProc)
           }
           else RETC(0)
       }
       else
       if (!strcmp(argv[1].strptr, "RESUB_EDIT"))
       {
           HWND ew = ListView_GetEditControl(h);
           if (ew)
           {
               setWindowPtr(ew, GWLP_WNDPROC, (LONG_PTR)wpOldEditProc);
               RETC(0)
           }
           RETVAL(-1)
       }
   }
   else
   if (argv[0].strptr[0] == 'M')
   {
       if (!strcmp(argv[1].strptr, "CNT"))
       {
           RETVAL(ListView_GetItemCount(h))
       }
       else
       if (!strcmp(argv[1].strptr, "CNTSEL"))
       {
           RETVAL(ListView_GetSelectedCount(h))
       }
       else
       if (!strcmp(argv[1].strptr, "REDRAW"))
       {
           CHECKARG(5);

           RETC(!ListView_RedrawItems(h, atol(argv[3].strptr), atol(argv[4].strptr)));
       }
       else
       if (!strcmp(argv[1].strptr, "UPDATE"))
       {
           CHECKARG(4);

           RETC(!ListView_Update(h, atol(argv[3].strptr)));
       }
       else
       if (!strcmp(argv[1].strptr, "ENVIS"))
       {
           CHECKARG(5);
           RETC(!ListView_EnsureVisible(h, atol(argv[3].strptr), isYes(argv[4].strptr)))
       }
       else
       if (!strcmp(argv[1].strptr, "CNTPP"))
       {
           RETVAL(ListView_GetCountPerPage(h))
       }
       else
       if (!strcmp(argv[1].strptr, "SCROLL"))
       {
           CHECKARG(5);
                                      /* dx */                /* dy */
           RETC(!ListView_Scroll(h, atoi(argv[3].strptr), atoi(argv[4].strptr)))
       }
       else
       if (!strcmp(argv[1].strptr, "COLOR"))
       {
           CHECKARGL(4);

           if (argv[3].strptr[0] == 'G')
           {
               COLORREF cr;
               INT i;
               if (!strcmp(argv[3].strptr, "GETBK")) cr = ListView_GetBkColor(h);
               else if (!strcmp(argv[3].strptr, "GETTXT")) cr = ListView_GetTextColor(h);
               else if (!strcmp(argv[3].strptr, "GETTXTBK")) cr = ListView_GetTextBkColor(h);
               for (i = 0; i< 256; i++) if (cr == PALETTEINDEX(i)) RETVAL(i);
               RETVAL(-1);
           }
           else
           {
               CHECKARG(5);
               if (!strcmp(argv[3].strptr, "SETBK")) RETC(!ListView_SetBkColor(h, PALETTEINDEX(atoi(argv[4].strptr))));
               if (!strcmp(argv[3].strptr, "SETTXT")) RETC(!ListView_SetTextColor(h, PALETTEINDEX(atoi(argv[4].strptr))));
               if (!strcmp(argv[3].strptr, "SETTXTBK")) RETC(!ListView_SetTextBkColor(h, PALETTEINDEX(atoi(argv[4].strptr))));
           }
       }
   }
   RETC(0)
}

/** ListView::arrange()
 *  ListView::snaptoGrid()
 *  ListView::alignLeft()
 *  Listview::alignTop()
 *
 *  @remarks  MSDN says of ListView_Arrange():
 *
 *  LVA_ALIGNLEFT  Not implemented. Apply the LVS_ALIGNLEFT style instead.
 *  LVA_ALIGNTOP   Not implemented. Apply the LVS_ALIGNTOP style instead.
 *
 *  However, I don't see that changing the align style in these two cases really
 *  does anything.
 */
RexxMethod2(RexxObjectPtr, lv_arrange, NAME, method, CSELF, pCSelf)
{
    HWND hList = getDCHCtrl(pCSelf);
    uint32_t style;
    int count = 0;

    int32_t flag = 0;
    switch ( method[5] )
    {
        case 'G' :
            flag = LVA_DEFAULT;
            break;
        case 'O' :
            flag = LVA_SNAPTOGRID;
            break;
        case 'L' :
            applyAlignStyle(hList, false);
            return TheZeroObj;
        case 'T' :
            applyAlignStyle(hList, true);
            return TheZeroObj;
    }
    return (ListView_Arrange(hList, flag) ? TheZeroObj : TheFalseObj);
}

RexxMethod3(int32_t, lv_checkUncheck, int32_t, index, NAME, method, CSELF, pCSelf)
{
    HWND hList = getDCHCtrl(pCSelf);

    if ( ! hasCheckBoxes(hList) )
    {
        return -2;
    }

    ListView_SetCheckState(hList, index, (*method == 'C'));
    return 0;
}

RexxMethod2(RexxObjectPtr, lv_isChecked, int32_t, index, CSELF, pCSelf)
{
    HWND hList = getDCHCtrl(pCSelf);

    if ( hasCheckBoxes(hList) )
    {
        if ( index >= 0 && index <= ListView_GetItemCount(hList) - 1 )
        {
            if ( ListView_GetCheckState(hList, index) != 0 )
            {
                return TheTrueObj;
            }
        }
    }
    return TheFalseObj;
}


RexxMethod2(int32_t, lv_getCheck, int32_t, index, CSELF, pCSelf)
{
    HWND hList = getDCHCtrl(pCSelf);

    if ( ! hasCheckBoxes(hList) )
    {
        return -2;
    }
    if ( index < 0 || index > ListView_GetItemCount(hList) - 1 )
    {
        return -3;
    }
    return (ListView_GetCheckState(hList, index) == 0 ? 0 : 1);
}

/** ListView::hasCheckBoxes()
 */
RexxMethod1(RexxObjectPtr, lv_hasCheckBoxes, CSELF, pCSelf)
{
    return (hasCheckBoxes(getDCHCtrl(pCSelf)) ? TheTrueObj : TheFalseObj);
}

/** ListView::getExtendedStyle()
 *  ListView::getExtendedStyleRaw()
 *
 */
RexxMethod2(RexxObjectPtr, lv_getExtendedStyle, NAME, method, CSELF, pCSelf)
{
    HWND hList = getDCHCtrl(pCSelf);
    if ( method[16] == 'R' )
    {
        return context->UnsignedInt32(ListView_GetExtendedListViewStyle(hList));
    }
    else
    {
        return extendedStyleToString(context, hList);
    }
}

/** ListView::addExtendedStyle()
 *  ListView::clearExtendedStyle()
 *
 */
RexxMethod3(int32_t, lv_addClearExtendStyle, CSTRING, _style, NAME, method, CSELF, pCSelf)
{
    uint32_t style = parseExtendedStyle(_style);
    if ( style == 0  )
    {
        return -3;
    }

    HWND hList = getDCHCtrl(pCSelf);

    if ( *method == 'C' )
    {
        ListView_SetExtendedListViewStyleEx(hList, style, 0);
    }
    else
    {
        ListView_SetExtendedListViewStyleEx(hList, style, style);
    }
    return 0;
}

RexxMethod3(int32_t, lv_replaceExtendStyle, CSTRING, remove, CSTRING, add, CSELF, pCSelf)
{
    uint32_t removeStyles = parseExtendedStyle(remove);
    uint32_t addStyles = parseExtendedStyle(add);
    if ( removeStyles == 0 || addStyles == 0  )
    {
        return -3;
    }

    HWND hList = getDCHCtrl(pCSelf);
    ListView_SetExtendedListViewStyleEx(hList, removeStyles, 0);
    ListView_SetExtendedListViewStyleEx(hList, addStyles, addStyles);
    return 0;
}

/** ListView::addStyle()
 *  ListView::removeStyle()
 *
 *  @note  Sets the .SystemErrorCode.
 */
RexxMethod3(uint32_t, lv_addRemoveStyle, CSTRING, style, NAME, method, CSELF, pCSelf)
{
    return changeStyle(context, (pCDialogControl)pCSelf, style, NULL, (*method == 'R'));
}

/** ListView::replaceStyle()
 *
 *
 *  @note  Sets the .SystemErrorCode.
 */
RexxMethod3(uint32_t, lv_replaceStyle, CSTRING, removeStyle, CSTRING, additionalStyle, CSELF, pCSelf)
{
    return changeStyle(context, (pCDialogControl)pCSelf, removeStyle, additionalStyle, true);
}

RexxMethod3(RexxObjectPtr, lv_getItemInfo, uint32_t, index, OPTIONAL_uint32_t, subItem, CSELF, pCSelf)
{
    HWND hList = getDCHCtrl(pCSelf);

    LVITEM lvi;
    char buf[256];

    lvi.iItem = index;
    lvi.iSubItem = subItem;
    lvi.mask = LVIF_TEXT | LVIF_IMAGE | LVIF_STATE;
    lvi.pszText = buf;
    lvi.cchTextMax = 255;
    lvi.stateMask = LVIS_CUT | LVIS_DROPHILITED | LVIS_FOCUSED | LVIS_SELECTED;

    if ( ! ListView_GetItem(hList, &lvi) )
    {
        return TheNegativeOneObj;
    }

    RexxStemObject stem = context->NewStem("InternalLVItemInfo");

    context->SetStemElement(stem, "!TEXT", context->String(lvi.pszText));
    context->SetStemElement(stem, "!IMAGE", context->Int32(lvi.iImage));

    *buf = '\0';
    if ( lvi.state & LVIS_CUT)         strcat(buf, "CUT ");
    if ( lvi.state & LVIS_DROPHILITED) strcat(buf, "DROP ");
    if ( lvi.state & LVIS_FOCUSED)     strcat(buf, "FOCUSED ");
    if ( lvi.state & LVIS_SELECTED)    strcat(buf, "SELECTED ");

    if ( *buf != '\0' )
    {
        *(buf + strlen(buf) - 1) = '\0';
    }
    context->SetStemElement(stem, "!STATE", context->String(buf));

    return stem;
}

RexxMethod1(int, lv_getColumnCount, CSELF, pCSelf)
{
    return getColumnCount(getDCHCtrl(pCSelf));
}

RexxMethod2(RexxObjectPtr, lv_getColumnInfo, uint32_t, index, CSELF, pCSelf)
{
    HWND hList = getDCHCtrl(pCSelf);

    LVCOLUMN lvi;
    char buf[256];

    lvi.mask = LVCF_TEXT | LVCF_SUBITEM | LVCF_FMT | LVCF_WIDTH;
    lvi.pszText = buf;
    lvi.cchTextMax = 255;

    if ( ! ListView_GetColumn(hList, index, &lvi) )
    {
        return TheNegativeOneObj;
    }

    RexxStemObject stem = context->NewStem("InternalLVColInfo");

    context->SetStemElement(stem, "!TEXT", context->String(lvi.pszText));
    context->SetStemElement(stem, "!COLUMN", context->Int32(lvi.iSubItem));
    context->SetStemElement(stem, "!WIDTH", context->Int32(lvi.cx));

    char *align = "LEFT";
    if ( (LVCFMT_JUSTIFYMASK & lvi.fmt) == LVCFMT_CENTER )
    {
        align = "CENTER";
    }
    else if ( (LVCFMT_JUSTIFYMASK & lvi.fmt) == LVCFMT_RIGHT )
    {
        align = "RIGHT";
    }
    context->SetStemElement(stem, "!ALIGN", context->String(align));

    return stem;
}

RexxMethod3(RexxObjectPtr, lv_setColumnWidthPx, uint32_t, index, OPTIONAL_RexxObjectPtr, _width, CSELF, pCSelf)
{
    HWND hList = getDCHCtrl(pCSelf);

    int width = getColumnWidthArg(context, _width, 2);
    if ( width == OOD_BAD_WIDTH_EXCEPTION )
    {
        return TheOneObj;
    }
    return (ListView_SetColumnWidth(hList, index, width) ? TheZeroObj : TheOneObj);
}

RexxMethod5(RexxObjectPtr, lv_modifyColumnPx, uint32_t, index, OPTIONAL_CSTRING, label, OPTIONAL_RexxObjectPtr, _width,
            OPTIONAL_CSTRING, align, CSELF, pCSelf)
{
    RexxMethodContext *c = context;
    HWND hList = getDCHCtrl(pCSelf);
    LVCOLUMN lvi = {0};

    if ( argumentExists(2) && *label != '\0' )
    {
        lvi.pszText = (LPSTR)label;
        lvi.cchTextMax = strlen(label);
        lvi.mask |= LVCF_TEXT;
    }
    if ( argumentExists(3) )
    {
        lvi.cx = getColumnWidthArg(context, _width, 3);
        if ( lvi.cx == OOD_BAD_WIDTH_EXCEPTION )
        {
            goto err_out;
        }
        lvi.mask |= LVCF_WIDTH;
    }
    if ( argumentExists(4) && *align != '\0' )
    {
        if ( StrStrI(align, "CENTER")     != NULL ) lvi.fmt = LVCFMT_CENTER;
        else if ( StrStrI(align, "RIGHT") != NULL ) lvi.fmt = LVCFMT_RIGHT;
        else if ( StrStrI(align, "LEFT")  != NULL ) lvi.fmt = LVCFMT_LEFT;
        else
        {
            wrongArgValueException(context->threadContext, 4, "LEFT, RIGHT, or CENTER", align);
            goto err_out;
        }
        lvi.mask |= LVCF_FMT;
    }

    return (ListView_SetColumn(hList, index, &lvi) ? TheZeroObj : TheOneObj);

err_out:
    return TheNegativeOneObj;
}

RexxMethod1(RexxObjectPtr, lv_getColumnOrder, CSELF, pCSelf)
{
    HWND hwnd = getDCHCtrl(pCSelf);

    int count = getColumnCount(hwnd);
    if ( count == -1 )
    {
        return TheNilObj;
    }

    RexxArrayObject order = context->NewArray(count);
    RexxObjectPtr result = order;

    // the empty array covers the case when count == 0

    if ( count == 1 )
    {
        context->ArrayPut(order, context->Int32(0), 1);
    }
    else if ( count > 1 )
    {
        int *pOrder = (int *)malloc(count * sizeof(int));
        if ( pOrder == NULL )
        {
            outOfMemoryException(context->threadContext);
        }
        else
        {
            if ( ListView_GetColumnOrderArray(hwnd, count, pOrder) == 0 )
            {
                result = TheNilObj;
            }
            else
            {
                for ( int i = 0; i < count; i++)
                {
                    context->ArrayPut(order, context->Int32(pOrder[i]), i + 1);
                }
            }
            free(pOrder);
        }
    }
    return result;
}

RexxMethod2(logical_t, lv_setColumnOrder, RexxArrayObject, order, OSELF, self)
{
    HWND hwnd = rxGetWindowHandle(context, self);

    size_t    items   = context->ArrayItems(order);
    int       count   = getColumnCount(hwnd);
    int      *pOrder  = NULL;
    logical_t success = FALSE;

    if ( count != -1 )
    {
        if ( count != items )
        {
            userDefinedMsgException(context->threadContext, "the number of items in the order array does not match the number of columns");
            goto done;
        }

        int *pOrder = (int *)malloc(items * sizeof(int));
        if ( pOrder != NULL )
        {
            RexxObjectPtr item;
            int column;

            for ( size_t i = 0; i < items; i++)
            {
                item = context->ArrayAt(order, i + 1);
                if ( item == NULLOBJECT || ! context->ObjectToInt32(item, &column) )
                {
                    wrongObjInArrayException(context->threadContext, 1, i + 1, "valid column number");
                    goto done;
                }
                pOrder[i] = column;
            }

            if ( ListView_SetColumnOrderArray(hwnd, count, pOrder) )
            {
                // If we don't redraw the list view and it is already displayed
                // on the screen, it will look mangled.
                RedrawWindow(hwnd, NULL, NULL, RDW_ERASE | RDW_INVALIDATE | RDW_UPDATENOW);
                success = TRUE;
            }
        }
        else
        {
            outOfMemoryException(context->threadContext);
        }
    }

done:
    safeFree(pOrder);
    return success;
}

/** ListControl::insertColumnPx()
 *
 *
 *  @param column
 *  @param text
 *  @param width   The width of the column in pixels
 *
 *
 *  @note  Even though the width argument in insertColumn() was documented as
 *         being in pixels, the code actually converted it to dialog units.
 *         This method is provided to really use pixels.
 *
 */
RexxMethod5(int, lv_insertColumnPx, OPTIONAL_uint16_t, column, CSTRING, text, uint16_t, width,
            OPTIONAL_CSTRING, fmt, CSELF, pCSelf)
{
    HWND hwnd = getDCHCtrl(pCSelf);

    LVCOLUMN lvi = {0};
    int retVal = 0;
    char szText[256];

    lvi.mask = LVCF_TEXT | LVCF_SUBITEM | LVCF_FMT | LVCF_WIDTH;

    // If omitted, column is 0, which is also the default.
    lvi.iSubItem = column;

    lvi.cchTextMax = (int)strlen(text);
    if ( lvi.cchTextMax > (sizeof(szText) - 1) )
    {
        userDefinedMsgException(context->threadContext, 2, "the column title must be less than 256 characters");
        return 0;
    }
    strcpy(szText, text);
    lvi.pszText = szText;
    lvi.cx = width;

    lvi.fmt = LVCFMT_LEFT;
    if ( argumentExists(4) )
    {
        char f = toupper(*fmt);
        if ( f == 'C' )
        {
            lvi.fmt = LVCFMT_CENTER;
        }
        else if ( f == 'R' )
        {
            lvi.fmt = LVCFMT_RIGHT;
        }
    }

    retVal = ListView_InsertColumn(hwnd, lvi.iSubItem, &lvi);
    if ( retVal != -1 && lvi.fmt != LVCFMT_LEFT && lvi.iSubItem == 0 )
    {
        /* According to the MSDN docs: "If a column is added to a
         * list-view control with index 0 (the leftmost column) and with
         * LVCFMT_RIGHT or LVCFMT_CENTER specified, the text is not
         * right-aligned or centered." This is the suggested work around.
         */
        lvi.iSubItem = 1;
        ListView_InsertColumn(hwnd, lvi.iSubItem, &lvi);
        ListView_DeleteColumn(hwnd, 0);
    }
    return retVal;
}

RexxMethod2(int, lv_stringWidthPx, CSTRING, text, CSELF, pCSelf)
{
    return ListView_GetStringWidth(getDCHCtrl(pCSelf), text);
}

// TODO review method name
RexxMethod6(int, lv_addRowEx, CSTRING, text, OPTIONAL_int, itemIndex, OPTIONAL_int, imageIndex,
            OPTIONAL_RexxObjectPtr, subItems, OSELF, self, CSELF, pCSelf)
{
    HWND hwnd = getDCHCtrl(pCSelf);

    if ( argumentOmitted(2) )
    {
        RexxObjectPtr last = context->SendMessage0(self, "LASTITEM");
        if ( last != NULLOBJECT )
        {
            context->Int32(last, &itemIndex);
            itemIndex++;
        }
        else
        {
            itemIndex = 0;
        }
    }

    if ( argumentOmitted(3) )
    {
        imageIndex = -1;
    }

    LV_ITEM lvi;
    lvi.mask = LVIF_TEXT;

    lvi.iItem = itemIndex;
    lvi.iSubItem = 0;
    lvi.pszText = (LPSTR)text;

    if ( imageIndex > -1 )
    {
        lvi.iImage = imageIndex;
        lvi.mask |= LVIF_IMAGE;
    }

    itemIndex = ListView_InsertItem(hwnd, &lvi);

    if ( itemIndex == -1 )
    {
        goto done_out;
    }
    context->SendMessage1(self, "LASTITEM=", context->Int32(itemIndex));

    if ( argumentOmitted(4) )
    {
        goto done_out;
    }
    if ( ! context->IsArray(subItems) )
    {
        wrongClassException(context->threadContext, 4, "Array");
        goto done_out;
    }

    size_t count = context->ArrayItems((RexxArrayObject)subItems);
    for ( size_t i = 1; i <= count; i++)
    {
        RexxDirectoryObject subItem = (RexxDirectoryObject)context->ArrayAt((RexxArrayObject)subItems, i);
        if ( subItem == NULLOBJECT || ! context->IsDirectory(subItem) )
        {
            wrongObjInArrayException(context->threadContext, 4, i, "Directory");
            goto done_out;
        }

        RexxObjectPtr subItemText = context->DirectoryAt(subItem, "TEXT");
        if ( subItemText == NULLOBJECT )
        {
            missingIndexInDirectoryException(context->threadContext, 4, "TEXT");
            goto done_out;
        }
        imageIndex = -1;
        if ( ! rxIntFromDirectory(context, subItem, "ICON", &imageIndex, 4) )
        {
            goto done_out;
        }

        lvi.mask = LVIF_TEXT;
        lvi.iSubItem = (int)i;
        lvi.pszText = (LPSTR)context->ObjectToStringValue(subItemText);

        if ( imageIndex > -1 )
        {
            lvi.iImage = imageIndex;
            lvi.mask |= LVIF_IMAGE;
        }

        ListView_SetItem(hwnd, &lvi);
    }

done_out:
    return itemIndex;
}


RexxMethod2(RexxObjectPtr, lv_getItemPos, uint32_t, index, CSELF, pCSelf)
{
    HWND hList = getDCHCtrl(pCSelf);

    POINT p;
    if ( ! ListView_GetItemPosition(hList, index, &p) )
    {
        return TheZeroObj;
    }
    return rxNewPoint(context, p.x, p.y);
}

/** ListView::setItemPos()
 *
 *  Moves a list view item to the specified position, (when the list view is in
 *  icon or small icon view.)
 *
 *  @param  index  The index of the item to move.
 *
 *  The other argument(s) specify the new position, and are optional.  If
 *  omitted the position defaults to (0, 0).  The position can either be
 *  specified using a .Point object, or using an x and a y co-ordinate.
 *
 *  @return  -1 if the list view is not in icon or small icon view, otherwise 0.
 */
RexxMethod4(RexxObjectPtr, lv_setItemPos, uint32_t, index, OPTIONAL_RexxObjectPtr, _obj, OPTIONAL_int32_t, y, CSELF, pCSelf)
{
    HWND hList = getDCHCtrl(pCSelf);

    if ( ! isInIconView(hList) )
    {
        return TheNegativeOneObj;
    }

    POINT p = {0};
    if ( argumentOmitted(2) )
    {
        // Doesn't matter if arg 3 is omitted or not, we just use it.  The
        // default if omitted is 0.
        p.y = y;
    }
    else
    {
        if ( argumentExists(3) )
        {
            // Arg 2 & arg 3 exist, they must both be integers then.
            if ( ! context->Int32(_obj, (int32_t *)&(p.x)) )
            {
                return wrongRangeException(context->threadContext, 2, INT32_MIN, INT32_MAX, _obj);
            }
            p.y = y;
        }
        else
        {
            // Arg 2 exists and arg 3 doesn't.  Arg 2 can be a .Point or an
            // integer.
            if ( context->IsOfType(_obj, "POINT") )
            {
                PPOINT tmp = (PPOINT)context->ObjectToCSelf(_obj);
                p.x = tmp->x;
                p.y = tmp->y;
            }
            else
            {
                // Arg 2 has to be an integer, p.y is already set at its
                // default of 0
                if ( ! context->Int32(_obj, (int32_t *)&(p.x)) )
                {
                    return wrongRangeException(context->threadContext, 2, INT32_MIN, INT32_MAX, _obj);
                }
            }
        }
    }

    ListView_SetItemPosition32(hList, index, p.x, p.y);
    return TheZeroObj;
}

/** ListControl::setImageList()
 *
 *  Sets or removes one of a list-view's image lists.
 *
 *  @param ilSrc  The image list source. Either an .ImageList object that
 *                references the image list to be set, or a single bitmap from
 *                which the image list is constructed, or .nil.  If ilSRC is
 *                .nil, an existing image list, if any is removed.
 *
 *  @param width  [optional]  This arg serves two purposes.  If ilSrc is .nil or
 *                an .ImageList object, this arg indentifies which of the
 *                list-views image lists is being set, normal, small, or state.
 *                The default is LVSI_NORMAL.
 *
 *                If ilSrc is a bitmap, then this arg is the width of a single
 *                image.  The default is the height of the actual bitmap.
 *
 *  @param height [optional]  This arg is only used if ilSrc is a bitmap, in
 *                which case it is the height of the bitmap.  The default is the
 *                height of the actual bitmap
 *
 *  @param ilType [optional]  Only used if ilSrc is a bitmap.  In that case it
 *                indentifies which of the list-views image lists is being set,
 *                normal, small, or state. The default is LVSI_NORMAL.
 *
 *  @return       Returns the exsiting .ImageList object if there is one, or
 *                .nil if there is not an existing object.
 *
 *  @note  When the ilSrc is a single bitmap, an image list is created from the
 *         bitmap.  This method is not as flexible as if the programmer created
 *         the image list herself.  The bitmap must be a number of images, all
 *         the same size, side-by-side in the bitmap.  The width of a single
 *         image determines the number of images.  The image list is created
 *         using the ILC_COLOR8 flag, only.  No mask can be used.  No room is
 *         reserved for adding more images to the image list, etc..
 */
RexxMethod5(RexxObjectPtr, lv_setImageList, RexxObjectPtr, ilSrc,
            OPTIONAL_int32_t, width, OPTIONAL_int32_t, height, OPTIONAL_int32_t, ilType, OSELF, self)
{
    HWND hwnd = rxGetWindowHandle(context, self);
    oodResetSysErrCode(context->threadContext);

    HIMAGELIST himl = NULL;
    RexxObjectPtr imageList = NULL;
    int type = LVSIL_NORMAL;

    if ( ilSrc == TheNilObj )
    {
        imageList = ilSrc;
        if ( argumentExists(2) )
        {
            type = width;
        }
    }
    else if ( context->IsOfType(ilSrc, "ImageList") )
    {
        imageList = ilSrc;
        himl = rxGetImageList(context, imageList, 1);
        if ( himl == NULL )
        {
            goto err_out;
        }

        if ( argumentExists(2) )
        {
            type = width;
        }
    }
    else
    {
        imageList = oodILFromBMP(context, &himl, ilSrc, width, height, hwnd);
        if ( imageList == NULLOBJECT )
        {
            goto err_out;
        }

        if ( argumentExists(4) )
        {
            type = ilType;
        }
    }

    if ( type > LVSIL_STATE )
    {
        wrongRangeException(context->threadContext, argumentExists(4) ? 4 : 2, LVSIL_NORMAL, LVSIL_STATE, type);
        goto err_out;
    }

    ListView_SetImageList(hwnd, himl, type);
    return rxSetObjVar(context, getLVAttributeName(type), imageList);

err_out:
    return NULLOBJECT;
}

/** ListControl::getImageList()
 *
 *  Gets the list-view's specifed image list.
 *
 *  @param  type [optional] Identifies which image list to get.  Normal, small,
 *          or state. Normal is the default.
 *
 *  @return  The image list, if it exists, otherwise .nil.
 */
RexxMethod2(RexxObjectPtr, lv_getImageList, OPTIONAL_uint8_t, type, OSELF, self)
{
    if ( argumentOmitted(1) )
    {
        type = LVSIL_NORMAL;
    }
    else if ( type > LVSIL_STATE )
    {
        wrongRangeException(context->threadContext, 1, LVSIL_NORMAL, LVSIL_STATE, type);
        return NULLOBJECT;
    }

    RexxObjectPtr result = context->GetObjectVariable(getLVAttributeName(type));
    if ( result == NULLOBJECT )
    {
        result = TheNilObj;
    }
    return result;
}


/**
 *  Methods for the .TreeControl class.
 */
#define TREECONTROL_CLASS         "TreeControl"

#define TVSTATE_ATTRIBUTE         "TV!STATEIMAGELIST"
#define TVNORMAL_ATTRIBUTE        "TV!NORMALIMAGELIST"

static CSTRING tvGetAttributeName(uint8_t type)
{
    switch ( type )
    {
        case TVSIL_STATE :
            return TVSTATE_ATTRIBUTE;
        case TVSIL_NORMAL :
        default :
            return TVNORMAL_ATTRIBUTE;
    }
}


static void parseTvModifyOpts(CSTRING opts, TVITEMEX *tvi)
{
    if ( StrStrI(opts, "NOTBOLD") != NULL )
    {
        tvi->stateMask |= TVIS_BOLD;
    }
    else if ( StrStrI(opts, "BOLD") != NULL )
    {
        tvi->state |= TVIS_BOLD;
        tvi->stateMask |= TVIS_BOLD;
    }

    if ( StrStrI(opts, "NOTDROP") != NULL )
    {
        tvi->stateMask |= TVIS_DROPHILITED;
    }
    else if ( StrStrI(opts, "DROP") != NULL )
    {
        tvi->state |= TVIS_DROPHILITED;
        tvi->stateMask |= TVIS_DROPHILITED;
    }

    if ( StrStrI(opts, "NOTSELECTED") != NULL )
    {
        tvi->stateMask |= TVIS_SELECTED;
    }
    else if ( StrStrI(opts, "SELECTED") != NULL )
    {
        tvi->state |= TVIS_SELECTED;
        tvi->stateMask |= TVIS_SELECTED;
    }

    if ( StrStrI(opts, "NOTCUT") != NULL )
    {
        tvi->stateMask |= TVIS_CUT;
    }
    else if ( StrStrI(opts, "CUT") != NULL )
    {
        tvi->state |= TVIS_CUT;
        tvi->stateMask |= TVIS_CUT;
    }

    if ( StrStrI(opts, "NOTEXPANDEDONCE") != NULL )
    {
        tvi->stateMask |= TVIS_EXPANDEDONCE;
    }
    else if ( StrStrI(opts, "EXPANDEDONCE") != NULL )
    {
        tvi->state |= TVIS_EXPANDEDONCE;
        tvi->stateMask |= TVIS_EXPANDEDONCE;
    }
    else if ( StrStrI(opts, "NOTEXPANDED") != NULL )
    {
        tvi->stateMask |= TVIS_EXPANDED;
    }
    else if ( StrStrI(opts, "EXPANDED") != NULL )
    {
        tvi->state |= TVIS_EXPANDED;
        tvi->stateMask |= TVIS_EXPANDED;
    }

    if ( tvi->state != 0 || tvi->stateMask != 0 )
    {
        tvi->mask |= TVIF_STATE;
    }
}


RexxMethod8(RexxObjectPtr, tv_insert, OPTIONAL_CSTRING, _hItem, OPTIONAL_CSTRING, _hAfter, OPTIONAL_CSTRING, label,
            OPTIONAL_int32_t, imageIndex, OPTIONAL_int32_t, selectedImage, OPTIONAL_CSTRING, opts, OPTIONAL_uint32_t, children,
            CSELF, pCSelf)
{
    HWND hwnd  = getDCHCtrl(pCSelf);

    TVINSERTSTRUCT  ins;
    TVITEMEX       *tvi = &ins.itemex;

    if ( argumentExists(1) )
    {
        if ( stricmp(_hItem, "ROOT") == 0 )
        {
            ins.hParent = TVI_ROOT;
        }
        else
        {
            ins.hParent = (HTREEITEM)string2pointer(_hItem);
        }
    }
    else
    {
        ins.hParent = TVI_ROOT;
    }

    if ( argumentExists(2) )
    {
        if ( stricmp(_hAfter,      "FIRST") == 0 ) ins.hInsertAfter = TVI_FIRST;
        else if ( stricmp(_hAfter, "SORT")  == 0 ) ins.hInsertAfter = TVI_SORT;
        else if ( stricmp(_hAfter, "LAST")  == 0 ) ins.hInsertAfter = TVI_LAST;
        else ins.hInsertAfter = (HTREEITEM)string2pointer(_hAfter);
    }
    else
    {
        ins.hInsertAfter = TVI_LAST;
    }

    memset(tvi, 0, sizeof(TVITEMEX));

    label         = (argumentOmitted(3) ? "" : label);
    imageIndex    = (argumentOmitted(4) ? -1 : imageIndex);
    selectedImage = (argumentOmitted(5) ? -1 : selectedImage);

    tvi->mask = TVIF_TEXT;
    tvi->pszText = (LPSTR)label;
    tvi->cchTextMax = (int)strlen(label);

    if ( imageIndex > -1 )
    {
        tvi->iImage = imageIndex;
        tvi->mask |= TVIF_IMAGE;
    }
    if ( selectedImage > -1 )
    {
        tvi->iSelectedImage = selectedImage;
        tvi->mask |= TVIF_SELECTEDIMAGE;
    }

    if ( argumentExists(6) )
    {
        if ( StrStrI(opts, "BOLD")     != NULL ) tvi->state |= TVIS_BOLD;
        if ( StrStrI(opts, "EXPANDED") != NULL ) tvi->state |= TVIS_EXPANDED;

        if ( tvi->state != 0 )
        {
            tvi->stateMask = tvi->state;
            tvi->mask |= TVIF_STATE;
        }
    }
    if ( children > 0 )
    {
        tvi->cChildren = children;
        tvi->mask |= TVIF_CHILDREN;
    }

    return pointer2string(context, TreeView_InsertItem(hwnd, &ins));
}

RexxMethod7(int32_t, tv_modify, OPTIONAL_CSTRING, _hItem, OPTIONAL_CSTRING, label, OPTIONAL_int32_t, imageIndex,
            OPTIONAL_int32_t, selectedImage, OPTIONAL_CSTRING, opts, OPTIONAL_uint32_t, children, CSELF, pCSelf)
{
    HWND hwnd  = getDCHCtrl(pCSelf);

    TVITEMEX tvi = {0};

    if ( argumentExists(1) )
    {
        tvi.hItem = (HTREEITEM)string2pointer(_hItem);
    }
    else
    {
        tvi.hItem = TreeView_GetSelection(hwnd);
    }

    if ( tvi.hItem == NULL )
    {
        return -1;
    }
    tvi.mask = TVIF_HANDLE;

    if ( argumentExists(2) )
    {
        tvi.pszText = (LPSTR)label;
        tvi.cchTextMax = (int)strlen(label);
        tvi.mask |= TVIF_TEXT;
    }
    if ( argumentExists(3) && imageIndex > -1 )
    {
        tvi.iImage = imageIndex;
        tvi.mask |= TVIF_IMAGE;
    }
    if ( argumentExists(4) && imageIndex > -1 )
    {
        tvi.iSelectedImage = selectedImage;
        tvi.mask |= TVIF_SELECTEDIMAGE;
    }
    if ( argumentExists(5) && *opts != '\0' )
    {
        parseTvModifyOpts(opts, &tvi);
    }
    if ( argumentExists(6) )
    {
        tvi.cChildren = (children > 0 ? 1 : 0);
        tvi.mask |= TVIF_CHILDREN;
    }

    return (TreeView_SetItem(hwnd, &tvi) == 0 ? 1 : 0);
}


RexxMethod2(RexxObjectPtr, tv_itemInfo, CSTRING, _hItem, CSELF, pCSelf)
{
    HWND hwnd  = getDCHCtrl(pCSelf);

    TVITEM tvi = {0};
    char buf[256];

    tvi.hItem = (HTREEITEM)string2pointer(_hItem);
    tvi.mask = TVIF_HANDLE | TVIF_TEXT | TVIF_STATE | TVIF_IMAGE | TVIF_CHILDREN | TVIF_SELECTEDIMAGE;
    tvi.pszText = buf;
    tvi.cchTextMax = 255;
    tvi.stateMask = TVIS_EXPANDED | TVIS_BOLD | TVIS_SELECTED | TVIS_EXPANDEDONCE | TVIS_DROPHILITED | TVIS_CUT;

    if ( TreeView_GetItem(hwnd, &tvi) == 0 )
    {
        return TheNegativeOneObj;
    }

    RexxStemObject stem = context->NewStem("InternalTVItemInfo");

    context->SetStemElement(stem, "!TEXT", context->String(tvi.pszText));
    context->SetStemElement(stem, "!CHILDREN", (tvi.cChildren > 0 ? TheTrueObj : TheFalseObj));
    context->SetStemElement(stem, "!IMAGE", context->Int32(tvi.iImage));
    context->SetStemElement(stem, "!SELECTEDIMAGE", context->Int32(tvi.iSelectedImage));

    *buf = '\0';
    if ( tvi.state & TVIS_EXPANDED     ) strcat(buf, "EXPANDED ");
    if ( tvi.state & TVIS_BOLD         ) strcat(buf, "BOLD ");
    if ( tvi.state & TVIS_SELECTED     ) strcat(buf, "SELECTED ");
    if ( tvi.state & TVIS_EXPANDEDONCE ) strcat(buf, "EXPANDEDONCE ");
    if ( tvi.state & TVIS_DROPHILITED  ) strcat(buf, "INDROP ");
    if ( tvi.state & TVIS_CUT          ) strcat(buf, "CUT ");
    if ( *buf != '\0' )
    {
        *(buf + strlen(buf) - 1) = '\0';
    }
    context->SetStemElement(stem, "!STATE", context->String(buf));

    return stem;
}


RexxMethod2(RexxObjectPtr, tv_getSpecificItem, NAME, method, CSELF, pCSelf)
{
    HWND hwnd = getDCHCtrl(pCSelf);
    HTREEITEM result = NULL;

    switch ( *method )
    {
        case 'R' :
            result = TreeView_GetRoot(hwnd);
            break;
        case 'S' :
            result = TreeView_GetSelection(hwnd);
            break;
        case 'D' :
            result = TreeView_GetDropHilight(hwnd);
            break;
        case 'F' :
            result = TreeView_GetFirstVisible(hwnd);
            break;
    }
    return pointer2string(context, result);
}


RexxMethod3(RexxObjectPtr, tv_getNextItem, CSTRING, _hItem, NAME, method, CSELF, pCSelf)
{
    HWND      hwnd  = getDCHCtrl(pCSelf);
    HTREEITEM hItem = (HTREEITEM)string2pointer(_hItem);
    uint32_t  flag  = TVGN_PARENT;

    if ( strcmp(method, "PARENT")               == 0 ) flag = TVGN_PARENT;
    else if ( strcmp(method, "CHILD")           == 0 ) flag = TVGN_CHILD;
    else if ( strcmp(method, "NEXT")            == 0 ) flag = TVGN_NEXT;
    else if ( strcmp(method, "NEXTVISIBLE")     == 0 ) flag = TVGN_NEXTVISIBLE;
    else if ( strcmp(method, "PREVIOUS")        == 0 ) flag = TVGN_PREVIOUS;
    else if ( strcmp(method, "PREVIOUSVISIBLE") == 0 ) flag = TVGN_PREVIOUSVISIBLE;

    return pointer2string(context, TreeView_GetNextItem(hwnd, hItem, flag));
}


/** TreeControl::select()
 *  TreeControl::makeFirstVisible()
 *  TreeControl::dropHighLight()
 */
RexxMethod3(RexxObjectPtr, tv_selectItem, OPTIONAL_CSTRING, _hItem, NAME, method, CSELF, pCSelf)
{
    HWND      hwnd  = getDCHCtrl(pCSelf);
    HTREEITEM hItem = NULL;
    uint32_t  flag;

    if ( argumentExists(1) )
    {
        hItem = (HTREEITEM)string2pointer(_hItem);
    }

    switch ( *method )
    {
        case 'S' :
            flag = TVGN_CARET;
            break;
        case 'M' :
            flag = TVGN_FIRSTVISIBLE;
            break;
        default:
            flag = TVGN_DROPHILITE;
    }
    return (TreeView_Select(hwnd, hItem, flag) ? TheZeroObj : TheOneObj);
}


/** TreeControl::expand()
 *  TreeControl::collapse()
 *  TreeControl::collapseAndReset()
 *  TreeControl::toggle()
 */
RexxMethod3(RexxObjectPtr, tv_expand, CSTRING, _hItem, NAME, method, CSELF, pCSelf)
{
    HWND      hwnd  = getDCHCtrl(pCSelf);
    HTREEITEM hItem = (HTREEITEM)string2pointer(_hItem);
    uint32_t  flag  = TVE_EXPAND;

    if ( *method == 'C' )
    {
        flag = (method[8] == 'A' ? (TVE_COLLAPSERESET | TVE_COLLAPSE) : TVE_COLLAPSE);
    }
    else if ( *method == 'T' )
    {
        flag = TVE_TOGGLE;
    }
    return (TreeView_Expand(hwnd, hItem, flag) ? TheZeroObj : TheOneObj);
}


/** TreeControl::subclassEdit()
 *  TreeControl::restoreEditClass()
 */
RexxMethod2(RexxObjectPtr, tv_subclassEdit, NAME, method, CSELF, pCSelf)
{
    HWND hwnd = getDCHCtrl(pCSelf);

    if ( *method == 'S' )
    {
        WNDPROC oldProc = (WNDPROC)setWindowPtr(hwnd, GWLP_WNDPROC, (LONG_PTR)CatchReturnSubProc);
        if ( oldProc != (WNDPROC)CatchReturnSubProc )
        {
            wpOldEditProc = oldProc;
        }
        return pointer2string(context, oldProc);
    }
    else
    {
        setWindowPtr(hwnd, GWLP_WNDPROC, (LONG_PTR)wpOldEditProc);
    }
    return TheZeroObj;
}


RexxMethod2(RexxObjectPtr, tv_hitTestInfo, ARGLIST, args, CSELF, pCSelf)
{
    HWND hwnd = getDCHCtrl(pCSelf);

    size_t sizeArray;
    int    argsUsed;
    POINT  point;
    if ( ! getPointFromArglist(context, args, &point, 1, 2, &sizeArray, &argsUsed) )
    {
        return NULLOBJECT;
    }

    if ( argsUsed == 1 && sizeArray == 2)
    {
        return tooManyArgsException(context->threadContext, 1);
    }

    TVHITTESTINFO hti;
    hti.pt.x = point.x;
    hti.pt.y = point.y;

    HTREEITEM hItem = TreeView_HitTest(hwnd, &hti);

    RexxDirectoryObject result = context->NewDirectory();

    context->DirectoryPut(result, pointer2string(context, TreeView_HitTest(hwnd, &hti)), "HITEM");

    char buf[128];
    *buf = '\0';

    if ( hti.flags & TVHT_ABOVE          ) strcat(buf, "ABOVE ");
    if ( hti.flags & TVHT_BELOW          ) strcat(buf, "BELOW ");
    if ( hti.flags & TVHT_NOWHERE        ) strcat(buf, "NOWHERE ");
    if ( hti.flags & TVHT_ONITEM         ) strcat(buf, "ONITEM ");
    if ( hti.flags & TVHT_ONITEMBUTTON   ) strcat(buf, "ONBUTTON ");
    if ( hti.flags & TVHT_ONITEMICON     ) strcat(buf, "ONICON ");
    if ( hti.flags & TVHT_ONITEMINDENT   ) strcat(buf, "ONINDENT ");
    if ( hti.flags & TVHT_ONITEMLABEL    ) strcat(buf, "ONLABEL ");
    if ( hti.flags & TVHT_ONITEMRIGHT    ) strcat(buf, "ONRIGHT ");
    if ( hti.flags & TVHT_ONITEMSTATEICON) strcat(buf, "ONSTATEICON ");
    if ( hti.flags & TVHT_TOLEFT         ) strcat(buf, "TOLEFT ");
    if ( hti.flags & TVHT_TORIGHT        ) strcat(buf, "TORIGHT ");

    if ( *buf != '\0' )
    {
        *(buf + strlen(buf) - 1) = '\0';
    }
    context->DirectoryPut(result, context->String(buf),"LOCATION");
    return result;
}


/** TreeControl::setImageList()
 *
 *  Sets or removes one of a tree-view's image lists.
 *
 *  @param ilSrc  The image list source. Either an .ImageList object that
 *                references the image list to be set, or a single bitmap from
 *                which the image list is constructed, or .nil.  If ilSRC is
 *                .nil, an existing image list, if any is removed.
 *
 *  @param width  [optional]  This arg serves two purposes.  If ilSrc is .nil or
 *                an .ImageList object, this arg indentifies which of the
 *                tree-views image lists is being set, normal, or state. The
 *                default is TVSI_NORMAL.
 *
 *                If ilSrc is a bitmap, then this arg is the width of a single
 *                image.  The default is the height of the actual bitmap.
 *
 *  @param height [optional]  This arg is only used if ilSrc is a bitmap, in which case it
 *                is the height of the bitmap.  The default is the height of the
 *                actual bitmap
 *
 *  @return       Returns the exsiting .ImageList object if there is one, or
 *                .nil if there is not an existing object.
 *
 *  @note  When the ilSrc is a single bitmap, an image list is created from the
 *         bitmap.  This method is not as flexible as if the programmer created
 *         the image list herself.  The bitmap must be a number of images, all
 *         the same size, side-by-side in the bitmap.  The width of a single
 *         image determines the number of images.  The image list is created
 *         using the ILC_COLOR8 flag, only.  No mask can be used.  No room is
 *         reserved for adding more images to the image list, etc..
 *
 *         The image list can only be assigned to the normal image list.  There
 *         is no way to use the image list for the state image list.
 */
RexxMethod4(RexxObjectPtr, tv_setImageList, RexxObjectPtr, ilSrc,
            OPTIONAL_int32_t, width, OPTIONAL_int32_t, height, CSELF, pCSelf)
{
    oodResetSysErrCode(context->threadContext);
    HWND hwnd = getDCHCtrl(pCSelf);

    HIMAGELIST himl = NULL;
    int type = TVSIL_NORMAL;
    RexxObjectPtr imageList = NULLOBJECT;

    if ( ilSrc == TheNilObj )
    {
        imageList = ilSrc;
        if ( argumentExists(2) )
        {
            type = width;
        }
    }
    else if ( context->IsOfType(ilSrc, "ImageList") )
    {
        imageList = ilSrc;
        himl = rxGetImageList(context, imageList, 1);
        if ( himl == NULL )
        {
            goto err_out;
        }
        if ( argumentExists(2) )
        {
            type = width;
        }
    }
    else
    {
        imageList = oodILFromBMP(context, &himl, ilSrc, width, height, hwnd);
        if ( imageList == NULLOBJECT )
        {
            goto err_out;
        }
    }

    if ( type != TVSIL_STATE && type != TVSIL_NORMAL )
    {
        invalidTypeException(context->threadContext, 2, " TVSIL_XXX flag");
        goto err_out;
    }

    TreeView_SetImageList(hwnd, himl, type);
    return rxSetObjVar(context, tvGetAttributeName(type), imageList);

err_out:
    return NULLOBJECT;
}

/** TreeControl::getImageList()
 *
 *  Gets the tree-view's specifed image list.
 *
 *  @param  type [optional] Identifies which image list to get, normal, or
 *               state. Normal is the default.
 *
 *  @return  The image list, if it exists, otherwise .nil.
 */
RexxMethod2(RexxObjectPtr, tv_getImageList, OPTIONAL_uint8_t, type, OSELF, self)
{
    if ( argumentOmitted(1) )
    {
        type = TVSIL_NORMAL;
    }
    else if ( type != TVSIL_STATE && type != TVSIL_NORMAL )
    {
        return invalidTypeException(context->threadContext, 2, " TVSIL_XXX flag");
    }

    RexxObjectPtr result = context->GetObjectVariable(tvGetAttributeName(type));
    if ( result == NULLOBJECT )
    {
        result = TheNilObj;
    }
    return result;
}


/**
 *  Methods for the .TabControl class.
 */
#define TABCONTROL_CLASS          "TabControl"

#define TABIMAGELIST_ATTRIBUTE    "TAB!IMAGELIST"


/** TabControl::setItemSize()
 *
 *  Sets the width and height of the tabs.
 *
 *  @param  size  The new size (cx, cy), in pixels.  The amount can be specified
 *                in these formats:
 *
 *      Form 1:  A .Size object.
 *      Form 2:  cx, cy
 *
 *  @return  The previous size of the tabs, as a .Size object
 *
 *  @note  You can use a .Point object instead of a .Size object to specify the
 *         new size, although semantically that is incorrect.
 */
RexxMethod2(RexxObjectPtr, tab_setItemSize, ARGLIST, args, CSELF, pCSelf)
{
    HWND hwnd = getDCHCtrl(pCSelf);

    size_t sizeArray;
    int    argsUsed;
    POINT  point;
    if ( ! getPointFromArglist(context, args, &point, 1, 2, &sizeArray, &argsUsed) )
    {
        return NULLOBJECT;
    }

    if ( argsUsed == 1 && sizeArray == 2)
    {
        return tooManyArgsException(context->threadContext, 1);
    }

    uint32_t oldSize = TabCtrl_SetItemSize(hwnd, point.x, point.y);
    return rxNewSize(context, LOWORD(oldSize), HIWORD(oldSize));
}


/** TabControl::setPadding()
 *
 *  Sets the amount of space (padding) around each tab's icon and label.
 *
 *  @param  size  The padding size (cx, cy), in pixels.  The amount can be
 *                specified in these formats:
 *
 *      Form 1:  A .Size object.
 *      Form 2:  cx, cy
 *
 *  @return  0, always.
 *
 *  @note  You can use a .Point object instead of a .Size object to specify the
 *         new size, although semantically that is incorrect.
 */
RexxMethod2(RexxObjectPtr, tab_setPadding, ARGLIST, args, CSELF, pCSelf)
{
    HWND hwnd = getDCHCtrl(pCSelf);

    size_t sizeArray;
    int    argsUsed;
    POINT  point;
    if ( ! getPointFromArglist(context, args, &point, 1, 2, &sizeArray, &argsUsed) )
    {
        return NULLOBJECT;
    }

    if ( argsUsed == 1 && sizeArray == 2)
    {
        return tooManyArgsException(context->threadContext, 1);
    }

    TabCtrl_SetPadding(hwnd, point.x, point.y);
    return TheZeroObj;
}


RexxMethod5(int32_t, tab_insert, OPTIONAL_int32_t, index, OPTIONAL_CSTRING, label, OPTIONAL_ssize_t, imageIndex,
            OPTIONAL_RexxObjectPtr, userData, CSELF, pCSelf)
{
    HWND hwnd = getDCHCtrl(pCSelf);

    if ( argumentOmitted(1) )
    {
        index = ((pCDialogControl)pCSelf)->lastItem;
    }

    TCITEM ti = {0};
    index++;

    ti.mask = TCIF_TEXT | TCIF_IMAGE | TCIF_PARAM;
    ti.pszText = (argumentOmitted(2) ? "" : label);
    ti.iImage  = (argumentOmitted(3) ? -1 : imageIndex);
    ti.lParam  = (LPARAM)(argumentOmitted(4) ? TheZeroObj : userData);

    int32_t ret = TabCtrl_InsertItem(hwnd, index, &ti);
    if ( ret != -1 )
    {
        ((pCDialogControl)pCSelf)->lastItem = ret;
    }
    return ret;
}


RexxMethod5(int32_t, tab_modify, int32_t, index, OPTIONAL_CSTRING, label, OPTIONAL_ssize_t, imageIndex,
            OPTIONAL_RexxObjectPtr, userData, CSELF, pCSelf)
{
    HWND hwnd = getDCHCtrl(pCSelf);

    TCITEM ti = {0};

    if ( argumentExists(2) )
    {
        ti.mask |= TCIF_TEXT;
        ti.pszText = (LPSTR)label;
    }
    if ( argumentExists(3) )
    {
        ti.mask |= TCIF_IMAGE;
        ti.iImage = imageIndex;
    }
    if ( argumentExists(4) )
    {
        ti.mask |= TCIF_PARAM;
        ti.lParam = (LPARAM)userData;
    }

    if ( ti.mask == 0 )
    {
        return 1;
    }

    return (TabCtrl_SetItem(hwnd, index, &ti) ? 0 : 1);
}


RexxMethod2(int32_t, tab_addSequence, ARGLIST, args, CSELF, pCSelf)
{
    HWND hwnd = getDCHCtrl(pCSelf);

    TCITEM ti = {0};
    ti.mask = TCIF_TEXT;

    int32_t ret = -1;
    ssize_t index = ((pCDialogControl)pCSelf)->lastItem;
    size_t count = context->ArraySize(args);

    for ( size_t i = 1; i <= count; i++ )
    {
        index++;
        RexxObjectPtr arg = context->ArrayAt(args, i);
        if ( arg == NULLOBJECT )
        {
            missingArgException(context->threadContext, i);
            goto done_out;
        }

        ti.pszText = (LPSTR)context->ObjectToStringValue(arg);

        ret = TabCtrl_InsertItem(hwnd, index, &ti);
        if ( ret == -1 )
        {
            goto done_out;
        }

        ((pCDialogControl)pCSelf)->lastItem = ret;
    }

done_out:
    return ret;
}


RexxMethod2(int32_t, tab_addFullSeq, ARGLIST, args, CSELF, pCSelf)
{
    HWND hwnd = getDCHCtrl(pCSelf);

    TCITEM ti = {0};
    ti.mask = TCIF_TEXT;

    int32_t ret = -1;
    ssize_t index = ((pCDialogControl)pCSelf)->lastItem;
    size_t count = context->ArraySize(args);

    for ( size_t i = 1; i <= count; i += 3 )
    {
        index++;
        RexxObjectPtr label = context->ArrayAt(args, i);
        if ( label == NULLOBJECT )
        {
            missingArgException(context->threadContext, i);
            goto done_out;
        }

        ti.pszText = (LPSTR)context->ObjectToStringValue(label);

        RexxObjectPtr _imageIndex = context->ArrayAt(args, i + 1);
        RexxObjectPtr userData = context->ArrayAt(args, i + 2);

        RexxMethodContext *c = context;
        if ( _imageIndex != NULLOBJECT )
        {
            int32_t imageIndex;
            if ( ! c->Int32(_imageIndex, &imageIndex) )
            {
                notPositiveArgException(context->threadContext, i + 1, _imageIndex);
                goto done_out;
            }

            ti.mask |= TCIF_IMAGE;
            ti.iImage = imageIndex;
        }
        if ( userData != NULLOBJECT )
        {
            ti.mask |= TCIF_PARAM;
            ti.lParam = (LPARAM)userData;
        }

        int32_t ret = TabCtrl_InsertItem(hwnd, index, &ti);
        if ( ret == -1 )
        {
            goto done_out;
        }

        ((pCDialogControl)pCSelf)->lastItem = ret;
    }

done_out:
    return ret;
}


RexxMethod2(RexxObjectPtr, tab_itemInfo, int32_t, index, CSELF, pCSelf)
{
    HWND hwnd = getDCHCtrl(pCSelf);

    char buff[256];
    TCITEM ti;

    ti.mask = TCIF_TEXT | TCIF_IMAGE | TCIF_PARAM;
    ti.pszText = buff;
    ti.cchTextMax = 255;

    RexxObjectPtr result = TheNegativeOneObj;
    RexxMethodContext *c = context;

    if ( TabCtrl_GetItem(hwnd, index, &ti) )
    {
        RexxStemObject stem = c->NewStem("ItemInfo");
        c->SetStemElement(stem, "!TEXT", c->String(ti.pszText));
        c->SetStemElement(stem, "!IMAGE", c->Int32(ti.iImage));
        c->SetStemElement(stem, "!PARAM", (ti.lParam == 0 ? TheZeroObj : (RexxObjectPtr)ti.lParam));
        result = stem;
    }
    return result;
}


RexxMethod1(RexxObjectPtr, tab_selected, CSELF, pCSelf)
{
    HWND hwnd = getDCHCtrl(pCSelf);

    char buff[256];
    TCITEM ti = {0};

    ti.mask = TCIF_TEXT;
    ti.pszText = buff;
    ti.cchTextMax = 255;

    if ( TabCtrl_GetItem(hwnd, TabCtrl_GetCurSel(hwnd), &ti) == 0 )
    {
        return TheZeroObj;
    }
    return context->String(buff);
}


RexxMethod2(int32_t, tab_select, CSTRING, text, CSELF, pCSelf)
{
    HWND hwnd = getDCHCtrl(pCSelf);
    int32_t result = -1;

    char buff[256];
    TCITEM ti = {0};
    size_t count;

    count = TabCtrl_GetItemCount(hwnd);
    if ( count == 0 )
    {
        goto done_out;
    }

    ti.mask = TCIF_TEXT;
    ti.cchTextMax = 255;

    size_t i = 0;
    while ( i < count)
    {
        // Note that MSDN says: If the TCIF_TEXT flag is set in the mask member
        // of the TCITEM structure, the control may change the pszText member of
        // the structure to point to the new text instead of filling the buffer
        // with the requested text. The control may set the pszText member to
        // NULL to indicate that no text is associated with the item.
        ti.pszText = buff;

        if ( TabCtrl_GetItem(hwnd, i, &ti) == 0 )
        {
            goto done_out;
        }

        if ( ti.pszText != NULL && stricmp(ti.pszText, text) == 0 )
        {
            result = TabCtrl_SetCurSel(hwnd, i);
            break;
        }
        i++;
    }

done_out:
    return result;
}


RexxMethod3(RexxObjectPtr, tab_getItemRect, uint32_t, item, RexxObjectPtr, rect, CSELF, pCSelf)
{
    HWND hwnd = getDCHCtrl(pCSelf);

    PRECT r = rxGetRect(context, rect, 2);
    if ( r == NULL )
    {
        return NULLOBJECT;
    }

    return (TabCtrl_GetItemRect(hwnd, item, r) == 0 ? TheFalseObj : TheTrueObj);
}


/** TabControl::calcWindowRect()
 *
 *  calcWindowRect() takes a display rectangle and adjusts the rectangle to be
 *  the window rect of the tab control needed for that display size.
 *
 *  Therefore, if the display size must be a fixed size, use calcWindowRect() to
 *  receive the size the tab control needs to be and use it to set the size for
 *  the control.
 *
 *  @param  [IN / OUT] On entry, a .Rect object specifying the display rectangle
 *                     and on return the corrsponding window rect for the tab.
 *
 *  @return  The return is 0 and has no meaning.
 *
 *  TabControl::calcDisplayRect()
 *
 *  caclDisplayRect() takes the window rect of the tab control, and adjusts the
 *  rectangle to the size the display will be.
 *
 *  So, if the tab control needs to be a fixed size, use calcDisplayRect() to
 *  get the size the display rect will be for the fixed size of the tab control
 *  and use that to set the size of the control or dialog set into the tab
 *  control
 *
 *  @param  [IN / OUT] On entry, a .Rect object specifying the window rect of
 *                     the tab, and on return the corrsponding display rect.
 *
 *  @return  The return is 0 and has no meaning.
 *
 *  @remarks  MSDN says of the second arg to TabCtrl_AdjustRect():
 *
 *            Operation to perform. If this parameter is TRUE, prc specifies a
 *            display rectangle and receives the corresponding window rectangle.
 *            If this parameter is FALSE, prc specifies a window rectangle and
 *            receives the corresponding display area.
 */
RexxMethod3(RexxObjectPtr, tab_calcRect, RexxObjectPtr, rect, NAME, method, CSELF, pCSelf)
{
    HWND hwnd = getDCHCtrl(pCSelf);

    PRECT r = rxGetRect(context, rect, 1);
    if ( r == NULL )
    {
        return NULLOBJECT;
    }

    BOOL calcWindowRect = (method[4] == 'W');

    TabCtrl_AdjustRect(hwnd, calcWindowRect, r);
    return TheZeroObj;
}


/** TabControl::setImageList()
 *
 *  Sets or removes the image list for a Tab control.
 *
 *  @param ilSrc  The image list source. Either an .ImageList object that
 *                references the image list to be set, or a single bitmap from
 *                which the image list is constructed, or .nil.  If ilSRC is
 *                .nil, an existing image list, if any is removed.
 *
 *  @param width  [optional]  This arg is only used if ilSrc is a single bitmap.
 *                Then this arg is the width of a single image.  The default is
 *                the height of the actual bitmap.
 *
 *  @param height [optional]  This arg is only used if ilSrc is a bitmap, in
 *                which case it is the height of the bitmap.  The default is the
 *                height of the actual bitmap
 *
 *  @return       Returns the exsiting .ImageList object if there is one, or
 *                .nil if there is not an existing object.
 *
 *  @note  When the ilSrc is a single bitmap, an image list is created from the
 *         bitmap.  This method is not as flexible as if the programmer created
 *         the image list herself.  The bitmap must be a number of images, all
 *         the same size, side-by-side in the bitmap.  The width of a single
 *         image determines the number of images.  The image list is created
 *         using the ILC_COLOR8 flag, only.  No mask can be used.  No room is
 *         reserved for adding more images to the image list, etc..
 */
RexxMethod4(RexxObjectPtr, tab_setImageList, RexxObjectPtr, ilSrc,
            OPTIONAL_int32_t, width, OPTIONAL_int32_t, height, OSELF, self)
{
    HWND hwnd = rxGetWindowHandle(context, self);
    oodResetSysErrCode(context->threadContext);

    HIMAGELIST himl = NULL;
    RexxObjectPtr imageList = NULLOBJECT;

    if ( ilSrc == TheNilObj )
    {
        imageList = ilSrc;
    }
    else if ( context->IsOfType(ilSrc, "ImageList") )
    {
        imageList = ilSrc;
        himl = rxGetImageList(context, imageList, 1);
        if ( himl == NULL )
        {
            goto err_out;
        }
    }
    else
    {
        imageList = oodILFromBMP(context, &himl, ilSrc, width, height, hwnd);
        if ( imageList == NULLOBJECT )
        {
            goto err_out;
        }
    }

    TabCtrl_SetImageList(hwnd, himl);
    return rxSetObjVar(context, TABIMAGELIST_ATTRIBUTE, imageList);

err_out:
    return NULLOBJECT;
}

/** TabControl::getImageList()
 *
 *  Gets the Tab control's image list.
 *
 *  @return  The image list, if it exists, otherwise .nil.
 */
RexxMethod1(RexxObjectPtr, tab_getImageList, OSELF, self)
{
    RexxObjectPtr result = context->GetObjectVariable(TABIMAGELIST_ATTRIBUTE);
    return (result == NULLOBJECT) ? TheNilObj : result;
}

