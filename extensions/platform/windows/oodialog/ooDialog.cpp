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
 * ooDialog.cpp
 *
 * The base module for the ooDialog package.  Contains the method implmentations
 * for the WindowBase and PlainBaseDialog classes.
 */
#include "ooDialog.hpp"     // Must be first, includes windows.h and oorexxapi.h

#include <mmsystem.h>
#include <commctrl.h>
#include <stdio.h>
#include <dlgs.h>
#include <shlwapi.h>
#include "APICommon.hpp"
#include "oodCommon.hpp"
#include "oodControl.hpp"
#include "oodMessaging.hpp"
#include "oodText.hpp"
#include "oodData.hpp"
#include "oodDeviceGraphics.hpp"
#include "oodResourceIDs.hpp"

extern MsgReplyType SearchMessageTable(ULONG message, WPARAM param, LPARAM lparam, DIALOGADMIN * addressedTo);
extern LONG SetRexxStem(const char * name, INT id, const char * secname, const char * data);

static HICON GetIconForID(DIALOGADMIN *, UINT, UINT, int, int);


/* dialog procedure
   handles the search for user defined messages and bitmap buttons
   handles messages necessary for 3d controls
   seeks for the addressed dialog to handle the messages */


LRESULT CALLBACK RexxDlgProc( HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
   HBRUSH hbrush = NULL;
   HWND hW;

   if (uMsg != WM_INITDIALOG)
   {
       pCPlainBaseDialog pcpbd = NULL;
       DIALOGADMIN *addressedTo = seekDlgAdm(hDlg);

       if ( addressedTo == NULL && topDlg != NULL && dialogInAdminTable(topDlg) && topDlg->TheDlg != NULL )
       {
           addressedTo = topDlg;
           pcpbd = (pCPlainBaseDialog)getWindowPtr(addressedTo->TheDlg, GWLP_USERDATA);
       }
       else
       {
           pcpbd = (pCPlainBaseDialog)getWindowPtr(hDlg, GWLP_USERDATA);
       }

       if ( addressedTo != NULL && pcpbd != NULL )
       {
          bool msgEnabled = IsWindowEnabled(hDlg) && dialogInAdminTable(addressedTo);

          // Do not search message table for WM_PAINT to improve redraw.
          if ( msgEnabled && uMsg != WM_PAINT && uMsg != WM_NCPAINT )
          {
              MsgReplyType searchReply = SearchMessageTable(uMsg, wParam, lParam, addressedTo);
              if ( searchReply != NotMatched )
              {
                  // Note pre 4.0.1, reply was always FALSE, pass on to the system to process.
                  return (searchReply == ReplyTrue ? TRUE : FALSE);
              }
          }

          switch (uMsg) {
             case WM_PAINT:
                if ( pcpbd->bkgBitmap != NULL )
                {
                    DrawBackgroundBmp(pcpbd, hDlg, wParam, lParam);
                }
                break;

             case WM_DRAWITEM:
                if ( lParam != 0 )
                {
                    return DrawBitmapButton(addressedTo, pcpbd, wParam, lParam, msgEnabled);
                }
                break;

             case WM_CTLCOLORDLG:
                if (pcpbd->bkgBrush)
                {
                    return (LRESULT)pcpbd->bkgBrush;
                }
                break;

             case WM_CTLCOLORSTATIC:
             case WM_CTLCOLORBTN:
             case WM_CTLCOLOREDIT:
             case WM_CTLCOLORLISTBOX:
             case WM_CTLCOLORMSGBOX:
             case WM_CTLCOLORSCROLLBAR:
                if ( addressedTo->CT_size > 0 )
                {
                    // See of the user has set the dialog item with a different
                    // color.
                    long id = GetWindowLong((HWND)lParam, GWL_ID);
                    if ( id > 0 )
                    {
                        register size_t i = 0;
                        while ( i < addressedTo->CT_size && addressedTo->ColorTab[i].itemID != id )
                        {
                           i++;
                        }
                        if ( i < addressedTo->CT_size )
                        {
                            hbrush = addressedTo->ColorTab[i].ColorBrush;
                        }

                        if (hbrush)
                        {
                            if ( addressedTo->ColorTab[i].isSysBrush )
                            {
                                SetBkColor((HDC)wParam, GetSysColor(addressedTo->ColorTab[i].ColorBk));
                                if ( addressedTo->ColorTab[i].ColorFG != -1 )
                                {
                                    SetTextColor((HDC)wParam, GetSysColor(addressedTo->ColorTab[i].ColorFG));
                                }
                            }
                            else
                            {
                                SetBkColor((HDC)wParam, PALETTEINDEX(addressedTo->ColorTab[i].ColorBk));
                                if ( addressedTo->ColorTab[i].ColorFG != -1 )
                                {
                                    SetTextColor((HDC)wParam, PALETTEINDEX(addressedTo->ColorTab[i].ColorFG));
                                }
                            }
                        }
                    }
                }
                if (hbrush)
                   return (LRESULT)hbrush;
                else
                   return DefWindowProc(hDlg, uMsg, wParam, lParam);


             case WM_COMMAND:
                 switch( LOWORD(wParam) )
                 {
                     // For both IDOK and IDCANCEL, the notification code (the high
                     // word value,) must be 0.
                     case IDOK:
                         if ( HIWORD(wParam) == 0 )
                         {
                             addressedTo->LeaveDialog = 1;
                         }
                         return TRUE;

                     case IDCANCEL:
                         if ( HIWORD(wParam) == 0 )
                         {
                             addressedTo->LeaveDialog = 2;
                         }
                         return TRUE;
                 }
                 break;

             case WM_QUERYNEWPALETTE:
             case WM_PALETTECHANGED:
                 return PaletteMessage(addressedTo, hDlg, uMsg, wParam, lParam);

             case WM_SETTEXT:
             case WM_NCPAINT:
             case WM_NCACTIVATE:
#ifdef __CTL3D
                if (addressedTo->Use3DControls)
                {
                    SetWindowLong(hDlg, DWL_MSGRESULT,Ctl3dDlgFramePaint(hDlg, uMsg, wParam, lParam));
                    return TRUE;
                }
#endif
                return FALSE;

              case WM_USER_CREATECHILD:
                /* Create a child dialog of this dialog and return its window
                 * handle. The dialog template pointer is passed here as the
                 * LPARAM arg from DynamicDialog::startChildDialog().
                 *
                 * Note that child dialogs do not have a backing Rexx dialog.
                 * There is no CPlainBaseDialog struct.  We pass NULL as the
                 * indirect param to indicate that.
                 */
                hW = CreateDialogIndirectParam(MyInstance, (DLGTEMPLATE *)lParam, hDlg, (DLGPROC)RexxDlgProc, NULL);
                ReplyMessage((LRESULT)hW);
                return (LRESULT)hW;

             case WM_USER_INTERRUPTSCROLL:
                addressedTo->StopScroll = wParam;
                return (TRUE);

             case WM_USER_GETFOCUS:
                ReplyMessage((LRESULT)GetFocus());
                return (TRUE);

             case WM_USER_GETSETCAPTURE:
                if ( wParam == 0 )
                {
                    ReplyMessage((LRESULT)GetCapture());
                }
                else if ( wParam == 2 )
                {
                    uint32_t rc = 0;
                    if ( ReleaseCapture == 0 )
                    {
                        rc = GetLastError();
                    }
                    ReplyMessage((LRESULT)rc);
                }
                else
                {
                    ReplyMessage((LRESULT)SetCapture((HWND)lParam));
                }
                return (TRUE);

             case WM_USER_GETKEYSTATE:
                ReplyMessage((LRESULT)GetAsyncKeyState((int)wParam));
                return (TRUE);

             case WM_USER_SUBCLASS:
             {
                 SUBCLASSDATA * pData = (SUBCLASSDATA *)lParam;
                 BOOL success = FALSE;

                 if ( pData )
                 {
                     success = SetWindowSubclass(pData->hCtrl, (SUBCLASSPROC)wParam, pData->uID, (DWORD_PTR)pData);
                 }
                 ReplyMessage((LRESULT)success);
                 return (TRUE);
             }

             case WM_USER_SUBCLASS_REMOVE:
                 ReplyMessage((LRESULT)RemoveWindowSubclass(GetDlgItem(hDlg, (int)lParam), (SUBCLASSPROC)wParam, (int)lParam));
                 return (TRUE);

             case WM_USER_HOOK:
                 ReplyMessage((LRESULT)SetWindowsHookEx(WH_KEYBOARD, (HOOKPROC)wParam, NULL, GetCurrentThreadId()));
                 return (TRUE);

             case WM_USER_CONTEXT_MENU:
             {
                 PTRACKPOP ptp = (PTRACKPOP)wParam;
                 uint32_t cmd;

                 SetLastError(0);
                 cmd = (uint32_t)TrackPopupMenuEx(ptp->hMenu, ptp->flags, ptp->point.x, ptp->point.y,
                                                  ptp->hWnd, ptp->lptpm);

                 // If TPM_RETURNCMD is specified, the return is the menu item
                 // selected.  Otherwise, the return is 0 for failure and
                 // non-zero for success.
                 if ( ! (ptp->flags & TPM_RETURNCMD) )
                 {
                     cmd = (cmd == 0 ? FALSE : TRUE);
                     if ( cmd == FALSE )
                     {
                         ptp->dwErr = GetLastError();
                     }
                 }
                 ReplyMessage((LRESULT)cmd);
                 return (TRUE);
             }
          }
       }
   }
   else
   {
       // In CreateDialogParam() / CreateDialogIndirectParam() we pass the
       // pointer to the PlainBaseDialog CSelf as the param.  Unless we created
       // a child dialog. The OS then sends us this value as the LPARAM argument
       // in the WM_INITDIALOG message. The pointer is then stored in the user
       // data field of the window words for this dialog.
       if ( lParam != NULL )
       {
           setWindowPtr(hDlg, GWLP_USERDATA, (LONG_PTR)lParam);
       }
       return TRUE;
   }
   return FALSE;
}

static DIALOGADMIN * allocDlgAdmin(RexxMethodContext *c)
{
    DIALOGADMIN *adm = NULL;

    EnterCriticalSection(&crit_sec);

    if ( StoredDialogs < MAXDIALOGS )
    {
        adm = (DIALOGADMIN *)LocalAlloc(LPTR, sizeof(DIALOGADMIN));
        if ( adm == NULL )
        {
            goto err_out;
        }

        adm->pMessageQueue = (char *)LocalAlloc(LPTR, MAXLENQUEUE);
        if ( adm->pMessageQueue == NULL )
        {
            goto err_out;
        }

        adm->previous = topDlg;
        adm->TableEntry = StoredDialogs;
        StoredDialogs++;
        DialogTab[adm->TableEntry] = adm;
        goto done_out;
    }
    else
    {
        MessageBox(0, "Too many active Dialogs","Error", MB_OK | MB_ICONHAND | MB_SYSTEMMODAL);
        goto done_out;
    }

err_out:
    safeLocalFree(adm);
    MessageBox(0, "Out of system resources", "Error", MB_OK | MB_ICONHAND | MB_SYSTEMMODAL);

done_out:
    LeaveCriticalSection(&crit_sec);
    return adm;
}

HBRUSH searchForBrush(DIALOGADMIN *dlgAdm, uint32_t *index, uint32_t id)
{
    HBRUSH hBrush = NULL;
    uint32_t i = 0;

    if ( dlgAdm != NULL && dlgAdm->ColorTab != NULL )
    {
        while ( i < dlgAdm->CT_size && dlgAdm->ColorTab[i].itemID != id )
        {
           i++;
        }
        if ( i < dlgAdm->CT_size )
        {
            hBrush = dlgAdm->ColorTab[i].ColorBrush;
            *index = i;
        }
    }
    return hBrush;
}

/**
 * Do some common set up when creating the underlying Windows dialog for any
 * ooDialog dialog.  This involves setting the 'topDlg' and the TheInstance
 * field of the DIALOGADMIN struct.
 *
 * If this is a ResDialog then the resource DLL is loaded, otherwise the
 * TheInstance field is the ooDialog.dll instance.
 *
 * @param dlgAdm    Pointer to the dialog administration block
 * @param library   The library to load the dialog from, if a ResDialog,
 *                  othewise null.
 *
 * @return True on success, false only if this is for a ResDialog and the
 *         loading of the resource DLL failed.
 */
bool InstallNecessaryStuff(DIALOGADMIN* dlgAdm, CSTRING library)
{
    if ( dlgAdm->previous )
    {
        ((DIALOGADMIN*)dlgAdm->previous)->OnTheTop = FALSE;
    }
    topDlg = dlgAdm;

    if ( library != NULL )
    {
        dlgAdm->TheInstance = LoadLibrary(library);
        if ( ! dlgAdm->TheInstance )
        {
            CHAR msg[256];
            sprintf(msg,
                    "Failed to load Dynamic Link Library (resource DLL.)\n"
                    "  File name:\t\t\t%s\n"
                    "  Windows System Error Code:\t%d\n", library, GetLastError());
            MessageBox(0, msg, "ooDialog DLL Load Error", MB_OK | MB_ICONHAND | MB_SYSTEMMODAL);
            return false;
        }
    }
    else
    {
        dlgAdm->TheInstance = MyInstance;
    }

    dlgAdm->Use3DControls = FALSE;
    return true;
}



/* end a dialog and remove installed components */
int32_t DelDialog(DIALOGADMIN * aDlg)
{
    DIALOGADMIN * current;
    int32_t ret;
    INT i;
    BOOL wasFGW;
    HICON hIconBig = NULL;
    HICON hIconSmall = NULL;

    EnterCriticalSection(&crit_sec);
    wasFGW = (aDlg->TheDlg == GetForegroundWindow());

    ret = aDlg->LeaveDialog;

    // Add this message, so PlainBaseDialog::handleMessages() knows that
    // PlainBaseDialog::finished() should be set.
    AddDialogMessage(MSG_TERMINATE, aDlg->pMessageQueue);

    if ( aDlg->LeaveDialog == 0 )
    {
        // Signal to exit.
        aDlg->LeaveDialog = 3;
    }

    // The dialog adminstration block entry must be removed before the WM_QUIT
    // message is posted.
    if ( aDlg->TableEntry == StoredDialogs - 1 )
    {
        // The dialog being ended is the last entry in the table, just set it to
        // null.
        DialogTab[aDlg->TableEntry] = NULL;
    }
    else
    {
        // The dialog being ended is not the last entry.  Move the last entry to
        // the one being deleted and then delete the last entry.
        DialogTab[aDlg->TableEntry] = DialogTab[StoredDialogs-1];
        DialogTab[aDlg->TableEntry]->TableEntry = aDlg->TableEntry;
        DialogTab[StoredDialogs-1] = NULL;
    }
    StoredDialogs--;

    // Post the WM_QUIT message to exit the Windows message loop.
    PostMessage(aDlg->TheDlg, WM_QUIT, 0, 0);

    if ( aDlg->TheDlg )
    {
        // The Windows documentation states: "must not use EndDialog for
        // non-modal dialogs"
        DestroyWindow(aDlg->TheDlg);
    }

    /* Swap back the saved icons. If not shared, the icon was loaded from a file
     * and needs to be freed, otherwise the OS handles the free.  The title bar
     * icon is tricky.  At this point the dialog is still visible.  If the small
     * icon in the class is set to 0, the application will hang.  Same thing
     * happens if the icon is freed.  So, don't set a zero into the class bytes,
     * and, if the icon is to be freed, do so after leaving the critical
     * section.
     */
    if ( aDlg->DidChangeIcon )
    {
        hIconBig = (HICON)setClassPtr(aDlg->TheDlg, GCLP_HICON, (LONG_PTR)aDlg->SysMenuIcon);
        if ( aDlg->TitleBarIcon )
        {
            hIconSmall = (HICON)setClassPtr(aDlg->TheDlg, GCLP_HICONSM, (LONG_PTR)aDlg->TitleBarIcon);
        }

        if ( ! aDlg->SharedIcon )
        {
            DestroyIcon(hIconBig);
            if ( ! hIconSmall )
            {
                hIconSmall = (HICON)getClassPtr(aDlg->TheDlg, GCLP_HICONSM);
            }
        }
        else
        {
            hIconSmall = NULL;
        }
    }

    if ( aDlg->TheInstance != NULL && aDlg->TheInstance != MyInstance )
    {
        FreeLibrary(aDlg->TheInstance);
    }
    current = (DIALOGADMIN *)aDlg->previous;

    // Delete the data and message tables of the dialog.
    if ( aDlg->MsgTab )
    {
        for ( i = 0; i < aDlg->MT_size; i++ )
        {
            safeLocalFree(aDlg->MsgTab[i].rexxProgram);
        }
        LocalFree(aDlg->MsgTab);
        aDlg->MT_size = 0;
    }
    safeLocalFree(aDlg->DataTab);
    aDlg->DT_size = 0;

    // Delete the color brushes.
    if (aDlg->ColorTab)
    {
        for (i=0;i<aDlg->CT_size;i++)
        {
            safeDeleteObject(aDlg->ColorTab[i].ColorBrush);
        }
        LocalFree(aDlg->ColorTab);
        aDlg->CT_size = 0;
    }

    // Delete the bitmaps and bitmap table.
    if (aDlg->BmpTab)
    {
        for (i=0;i<aDlg->BT_size;i++)
        {
            if ( (aDlg->BmpTab[i].Loaded & 0x1011) == 1 )
            {
                /* otherwise stretched bitmap files are not freed */
                safeLocalFree((void *)aDlg->BmpTab[i].bitmapID);
                safeLocalFree((void *)aDlg->BmpTab[i].bmpFocusID);
                safeLocalFree((void *)aDlg->BmpTab[i].bmpSelectID);
                safeLocalFree((void *)aDlg->BmpTab[i].bmpDisableID);
            }
            else if ( aDlg->BmpTab[i].Loaded == 0 )
            {
                safeDeleteObject((HBITMAP)aDlg->BmpTab[i].bitmapID);
                safeDeleteObject((HBITMAP)aDlg->BmpTab[i].bmpFocusID);
                safeDeleteObject((HBITMAP)aDlg->BmpTab[i].bmpSelectID);
                safeDeleteObject((HBITMAP)aDlg->BmpTab[i].bmpDisableID);
            }
        }

        LocalFree(aDlg->BmpTab);
        safeDeleteObject(aDlg->ColorPalette);
        aDlg->BT_size = 0;
    }

    // Delete the icon resource table.
    if (aDlg->IconTab)
    {
        for ( i = 0; i < aDlg->IT_size; i++ )
        {
            safeLocalFree(aDlg->IconTab[i].fileName);
        }
        LocalFree(aDlg->IconTab);
        aDlg->IT_size = 0;
    }

    // Unhook a hook if it is installed.
    if ( aDlg->hHook )
    {
        removeKBHook(aDlg);
    }

    // The message queue and the dialog administration block itself are freed
    // from the PlainBaseDialog::deInstall() or PlainBaseDialog::unInit()

    if ( StoredDialogs == NULL )
    {
        topDlg = NULL;
    }
    else
    {
        topDlg = current;
    }

    if ( topDlg != NULL )
    {
        if ( dialogInAdminTable(topDlg) )
        {
            if (!IsWindowEnabled(topDlg->TheDlg))
            {
                EnableWindow(topDlg->TheDlg, TRUE);
            }
            if ( wasFGW )
            {
                SetForegroundWindow(topDlg->TheDlg);
                topDlg->OnTheTop = TRUE;
            }
        }
        else
        {
            topDlg = NULL;
        }
    }
    LeaveCriticalSection(&crit_sec);
    if ( hIconSmall )
    {
        DestroyIcon(hIconSmall);
    }
    return ret;
}


/**
 * Loads and returns the handles to the regular size and small size icons for
 * the dialog. These icons are used in the title bar of the dialog, on the task
 * bar, and for the alt-tab display.
 *
 * The icons can come from the user resource DLL, a user defined dialog, or the
 * OODialog DLL.  IDs for the icons bound to the OODialog.dll are reserved.
 *
 * This function attempts to always succeed.  If an icon is not attained, the
 * default icon from the resources in the OODialog DLL is used.  This icon
 * should always be present, it is bound to the DLL when ooRexx is built.
 *
 * @param dlgAdm    Pointer to the dialog administration block.
 * @param id        Numerical resource ID.
 * @param iconSrc   Flag indicating whether the icon is located in a DLL or to
 *                  be loaded from a file.
 * @param phBig     In/Out Pointer to an icon handle.  If the function succeeds,
 *                  on return will contian the handle to a regular size icon.
 * @param phSmall   In/Out Pointer to an icon handle.  On success will contain
 *                  a handle to a small size icon.
 *
 * @return True if the icons were loaded and the returned handles are valid,
 *         otherwise false.
 */
BOOL GetDialogIcons(DIALOGADMIN *dlgAdm, INT id, UINT iconSrc, PHANDLE phBig, PHANDLE phSmall)
{
    int cx, cy;

    if ( phBig == NULL || phSmall == NULL )
    {
        return FALSE;
    }

    if ( id < 1 )
    {
        id = IDI_DLG_DEFAULT;
    }

    /* If one of the reserved IDs, iconSrc has to be ooDialog. */
    if ( id >= IDI_DLG_MIN_ID && id <= IDI_DLG_MAX_ID )
    {
        iconSrc = ICON_OODIALOG;
    }

    cx = GetSystemMetrics(SM_CXICON);
    cy = GetSystemMetrics(SM_CYICON);

    *phBig = GetIconForID(dlgAdm, id, iconSrc, cx, cy);

    /* If that didn't get the big icon, try to get the default icon. */
    if ( ! *phBig && id != IDI_DLG_DEFAULT )
    {
        id = IDI_DLG_DEFAULT;
        iconSrc = ICON_OODIALOG;
        *phBig = GetIconForID(dlgAdm, id, iconSrc, cx, cy);
    }

    /* If still no big icon, don't bother trying for the small icon. */
    if ( *phBig )
    {
        cx = GetSystemMetrics(SM_CXSMICON);
        cy = GetSystemMetrics(SM_CYSMICON);
        *phSmall = GetIconForID(dlgAdm, id, iconSrc, cx, cy);

        /* Very unlikely that the big icon was obtained and failed to get the
         * small icon.  But, if so, fail completely.  If the big icon came from
         * a DLL it was loaded as shared and the system handles freeing it.  If
         * it was loaded from a file, free it here.
         */
        if ( ! *phSmall )
        {
            if ( iconSrc & ICON_FILE )
            {
                DestroyIcon((HICON)*phBig);
            }
            *phBig = NULL;
        }
    }

    if ( ! *phBig )
    {
        return FALSE;
    }

    dlgAdm->SharedIcon = iconSrc != ICON_FILE;
    return TRUE;
}


/**
 * Loads and returns the handle to an icon for the specified ID, of the
 * specified size.
 *
 * The icons can come from the user resource DLL, a user defined dialog, the
 * OODialog DLL, the System.  IDs for the icons bound to the OODialog.dll are
 * reserved.
 *
 * @param dlgAdm    Pointer to the dialog administration block.
 * @param id        Numerical resource ID.
 * @param iconSrc   Flag indicating the source of the icon.
 * @param cx        The desired width of the icon.
 * @param cy        The desired height of the icon.
 *
 * @return The handle to the loaded icon on success, or null on failure.
 */
static HICON GetIconForID(DIALOGADMIN *dlgAdm, UINT id, UINT iconSrc, int cx, int cy)
{
    HINSTANCE hInst = NULL;
    LPCTSTR   pName = NULL;
    UINT      loadFlags = 0;

    if ( iconSrc & ICON_FILE )
    {
        /* Load the icon from a file, file name should be in the icon table. */
        INT i;

        for ( i = 0; i < dlgAdm->IT_size; i++ )
        {
            if ( dlgAdm->IconTab[i].iconID == id )
            {
                pName = dlgAdm->IconTab[i].fileName;
                break;
            }
        }

        if ( ! pName )
            return NULL;

        loadFlags = LR_LOADFROMFILE;
    }
    else if ( iconSrc & ICON_OODIALOG )
    {
        /* Load the icon from the resources in oodialog.dll. */
        hInst = MyInstance;
        pName = MAKEINTRESOURCE(id);
        loadFlags = LR_SHARED;
    }
    else
    {
        /* Load the icon from the user's resource DLL. */
        hInst = dlgAdm->TheInstance;
        pName = MAKEINTRESOURCE(id);
        loadFlags = LR_SHARED;
    }

    return (HICON)LoadImage(hInst, pName, IMAGE_ICON, cx, cy, loadFlags);
}


/**
 *  Methods for the .WindowBase mixin class.
 */
#define WINDOWBASE_CLASS       "WindowBase"

static inline HWND getWBWindow(void *pCSelf)
{
    return ((pCWindowBase)pCSelf)->hwnd;
}

static LRESULT sendWinMsg(RexxMethodContext *context, CSTRING wm_msg, WPARAM wParam, LPARAM lParam,
                          HWND _hwnd, pCWindowBase pcwb)
{
    oodResetSysErrCode(context->threadContext);

    uint32_t msgID;
    if ( ! rxStr2Number32(context, wm_msg, &msgID, 2) )
    {
        return 0;
    }

    HWND hwnd;
    if ( argumentOmitted(4) )
    {
        if ( pcwb->hwnd == NULL )
        {
            noWindowsDialogException(context, pcwb->rexxSelf);
            return 0;
        }
        hwnd = pcwb->hwnd;
    }
    else
    {
        hwnd = (HWND)_hwnd;
    }

    LRESULT result = SendMessage(hwnd, msgID, wParam, lParam);
    oodSetSysErrCode(context->threadContext);
    return result;
}

RexxObjectPtr sendWinMsgGeneric(RexxMethodContext *c, HWND hwnd, CSTRING wm_msg, RexxObjectPtr _wParam,
                                RexxObjectPtr _lParam, int argPos, bool doIntReturn)
{
    oodResetSysErrCode(c->threadContext);

    uint32_t msgID;
    if ( ! rxStr2Number32(c, wm_msg, &msgID, argPos) )
    {
        return TheZeroObj;
    }
    argPos++;

    WPARAM wParam;
    if ( ! oodGetWParam(c, _wParam, &wParam, argPos) )
    {
        return TheZeroObj;
    }
    argPos++;

    LPARAM lParam;
    if ( ! oodGetLParam(c, _lParam, &lParam, argPos) )
    {
        return TheZeroObj;
    }

    LRESULT lr = SendMessage(hwnd, msgID, wParam, lParam);
    oodSetSysErrCode(c->threadContext);

    if ( doIntReturn )
    {
        return c->Intptr((intptr_t)lr);
    }
    else
    {
        return pointer2string(c, (void *)lr);
    }
}


/**
 * Common code to call ShowWindow() from a native C++ API method.
 *
 * @param hwnd  Window handle of window to show.
 * @param type  Single character indicating which SW_ flag to use.
 *
 * @return  True if the window was previously visible.  Return false if the
 *          window was previously hidden.
 */
logical_t showWindow(HWND hwnd, char type)
{
    int flag;
    switch ( type )
    {
        case 'D' :
        case 'N' :
        case 'S' :
            flag = SW_NORMAL;
            break;

        case 'H' :
            flag = SW_HIDE;
            break;

        case 'I' :
            flag = SW_SHOWNA;
            break;

        case 'M' :
            flag = SW_SHOWMINIMIZED;
            break;

        case 'R' :
            flag = SW_RESTORE;
            break;

        case 'X' :
            flag = SW_SHOWMAXIMIZED;
            break;

        default :
            flag = SW_SHOW;
            break;

    }
    return ShowWindow(hwnd, flag);
}

uint32_t showFast(HWND hwnd, char type)
{
    uint32_t style = GetWindowLong(hwnd, GWL_STYLE);
    if ( style )
    {
        if ( type == 'H' )
        {
            style ^= WS_VISIBLE;
        }
        else
        {
            style |= WS_VISIBLE;
        }
        SetWindowLong(hwnd, GWL_STYLE, style);
        return 0;
    }
    return 1;
}

/**
 * Performs the initialization of the WindowBase mixin class.
 *
 * This is done by creating the cself struct for that class and then sending
 * that struct to the init_windowBase() method.  That method will raise an
 * exception if its arg is not a RexxBufferObject.  This implies that the
 * WindowBase mixin class can only be initialized through the native API.
 *
 * The plain base dialog, dialog control, and window classes in ooDialog inherit
 * WindowBase.
 *
 * @param c        Method context we are operating in.
 *
 * @param hwndObj  Window handle of the underlying object.  This can be null and
 *                 for a dialog object, it is always null.  (Because for a
 *                 dialog object the underlying dialog has not been created at
 *                 this point.)
 *
 * @param self     The Rexx object that inherited WindowBase.
 *
 * @return True on success, otherwise false.  If false an exception has been
 *         raised.
 *
 * @remarks  This method calculates the factor X and Y values using the old,
 *           incorrect, ooDialog method.  When the hwnd is unknown, that is the
 *           best that can be done.  But, when the hwnd is known, it would be
 *           better to calculate it correctly.  Even when the hwnd is unknown,
 *           we could calculate it correctly using the font of the dialog.
 *
 *           Note that in the original ooDialog code, factorX and factorY were
 *           formatted to only 1 decimal place.
 *
 *           Note that in the original ooDialog, if the object was a dialog,
 *           (i.e. the hwnd is unknown,) then sizeX and sizeY simply remain at
 *           0.
 */
bool initWindowBase(RexxMethodContext *c, HWND hwndObj, RexxObjectPtr self, pCWindowBase *ppCWB)
{
    RexxBufferObject obj = c->NewBuffer(sizeof(CWindowBase));
    if ( obj == NULLOBJECT )
    {
        return false;
    }

    pCWindowBase pcwb = (pCWindowBase)c->BufferData(obj);
    memset(pcwb, 0, sizeof(CWindowBase));

    ULONG bu = GetDialogBaseUnits();
    pcwb->factorX = LOWORD(bu) / 4;
    pcwb->factorY = HIWORD(bu) / 8;

    pcwb->rexxSelf = self;

    if ( hwndObj != NULL )
    {
        RECT r = {0};
        if ( GetWindowRect(hwndObj, &r) == 0 )
        {
            systemServiceExceptionCode(c->threadContext, API_FAILED_MSG, "GetWindowRect");
            return false;
        }

        RexxStringObject h = pointer2string(c, hwndObj);
        pcwb->hwnd = hwndObj;
        pcwb->rexxHwnd = c->RequestGlobalReference(h);

        pcwb->sizeX =  (uint32_t)((r.right - r.left) / pcwb->factorX);
        pcwb->sizeY =  (uint32_t)((r.bottom - r.top) / pcwb->factorY);
    }
    else
    {
        pcwb->rexxHwnd = TheZeroObj;
    }

    c->SendMessage1(self, "INIT_WINDOWBASE", obj);

    if ( ppCWB != NULL )
    {
        *ppCWB = pcwb;
    }
    return true;
}

/** WindowBase::init_windowBase()
 *
 *
 */
RexxMethod1(logical_t, wb_init_windowBase, RexxObjectPtr, cSelf)
{
    RexxMethodContext *c = context;
    if ( ! context->IsBuffer(cSelf) )
    {
        context->SetObjectVariable("INITCODE", TheOneObj);
        wrongClassException(context->threadContext, 1, "Buffer");
        return FALSE;
    }

    context->SetObjectVariable("CSELF", cSelf);

    context->SetObjectVariable("INITCODE", TheZeroObj);

    return TRUE;
}

/** WindowBase::hwnd  [attribute get]
 */
RexxMethod1(RexxObjectPtr, wb_getHwnd, CSELF, pCSelf)
{
    return ((pCWindowBase)pCSelf)->rexxHwnd;
}

/** WindowBase::factorX  [attribute]
 */
RexxMethod1(double, wb_getFactorX, CSELF, pCSelf) { return ((pCWindowBase)pCSelf)->factorX; }
RexxMethod2(RexxObjectPtr, wb_setFactorX, float, xFactor, CSELF, pCSelf) { ((pCWindowBase)pCSelf)->factorX = xFactor; return NULLOBJECT; }

/** WindowBase::factorY  [attribute]
 */
RexxMethod1(double, wb_getFactorY, CSELF, pCSelf) { return ((pCWindowBase)pCSelf)->factorY; }
RexxMethod2(RexxObjectPtr, wb_setFactorY, float, yFactor, CSELF, pCSelf) { ((pCWindowBase)pCSelf)->factorY = yFactor; return NULLOBJECT; }

/** WindowBase::sizeX  [attribute]
 */
RexxMethod1(uint32_t, wb_getSizeX, CSELF, pCSelf) { return ((pCWindowBase)pCSelf)->sizeX; }
RexxMethod2(RexxObjectPtr, wb_setSizeX, uint32_t, xSize, CSELF, pCSelf) { ((pCWindowBase)pCSelf)->sizeX = xSize; return NULLOBJECT; }

/** WindowBase::sizeY  [attribute]
 */
RexxMethod1(uint32_t, wb_getSizeY, CSELF, pCSelf) { return ((pCWindowBase)pCSelf)->sizeY; }
RexxMethod2(RexxObjectPtr, wb_setSizeY, uint32_t, ySize, CSELF, pCSelf) { ((pCWindowBase)pCSelf)->sizeY = ySize; return NULLOBJECT; }

/** WindowBase::pixelX  [attribute]
 *
 *  Returns the width of the window in pixels.  This is a 'get' only attribute.
 */
RexxMethod1(uint32_t, wb_getPixelX, CSELF, pCSelf)
{
    pCWindowBase pcs = (pCWindowBase)pCSelf;
    if ( pcs->hwnd == NULL )
    {
        return 0;
    }

    RECT r = {0};
    GetWindowRect(pcs->hwnd, &r);
    return r.right - r.left;
}

/** WindowBase::pixelY  [attribute]
 *
 *  Returns the height of the window in pixels.  This is a 'get' only attribute.
 */
RexxMethod1(uint32_t, wb_getPixelY, CSELF, pCSelf)
{
    pCWindowBase pcs = (pCWindowBase)pCSelf;
    if ( pcs->hwnd == NULL )
    {
        return 0;
    }

    RECT r = {0};
    GetWindowRect(pcs->hwnd, &r);
    return r.bottom - r.top;
}

/** WindowBase::sendMessage()
 *  WindowBase::sendMessageHandle()
 *
 *  Sends a window message to the underlying window of this object.
 *
 *  @param  wm_msg  The Windows window message ID.  This can be specified in
 *                  either "0xFFFF" or numeric format.
 *
 *  @param  wParam  The WPARAM value for the message.
 *  @param  lParam  The LPARAM value for the message.
 *
 *  @return The result of sending the message, as returned by the operating
 *          system.  For sendMessage() the result is returned as a whole number.
 *          For sendMessageHandle() the result is returned as an operating
 *          system handle.
 *
 *  @note Sets the .SystemErrorCode.
 *
 *        The wParam and lParam arguments can be in a number of formats.  An
 *        attempt is made to convert the Rexx object from a .Pointer, pointer
 *        string, uintptr_t number, or intptr_t number.  If they all fail, an
 *        exception is raised.
 *
 *        These methods will not work for window messages that take a string as
 *        an argument or return a string as a result.
 *
 *  @remarks  This function is used for the documented DialogControl
 *            processMessage() method, and therefore needs to remain generic.
 *            Internally, most of the time, it would make more sense to use one
 *            of the argument type specific methods like sendWinIntMsg().
 */
RexxMethod5(RexxObjectPtr, wb_sendMessage, CSTRING, wm_msg, RexxObjectPtr, _wParam, RexxObjectPtr, _lParam,
            NAME, method, CSELF, pCSelf)
{
    HWND hwnd = getWBWindow(pCSelf);
    bool doIntReturn = (strlen(method) == 11 ? true : false);

    return sendWinMsgGeneric(context, hwnd, wm_msg, _wParam, _lParam, 1, doIntReturn);
}

/** WindowBase::sendWinIntMsg()
 *
 *  Sends a message to a Windows window where WPARAM and LPARAM are both numbers
 *  and the return is a number.  I.e., neither param is a handle and the return
 *  is not a string or a handle.
 *
 *  @param  wm_msg  The Windows window message ID.  This can be specified in
 *                  "0xFFFF" format or numeric format.
 *
 *  @param  wParam  The WPARAM value for the message.
 *  @param  lParam  The LPARAM value for the message.
 *
 *  @param  _hwnd   [OPTIONAL]  The handle of the window the message is sent to.
 *                  If omitted, the window handle of this object is used.
 *
 *  @return The result of sending the message, as returned by the operating
 *          system.
 *
 *  @remarks  Sets the .SystemErrorCode.
 *
 *            This method is not meant to be documented for the user, it is
 *            intended to be used internally only.  Currently, all uses of this
 *            function have a return of a number.  If a need comes up to return
 *            a handle, then add sendWinIntMsgH().
 *
 *            In addition, wParam should really be uintptr_t.  However, some,
 *            many?, control messages use / accept negative nubmers for wParam.
 *            If we were just casting the number here, that would work.  But,
 *            the interpreter checks the range before invoking and negative
 *            numbers cause a condition to be raised.  So, we use intptr_t for
 *            wParam here.  It may become needed to add a sendWinUintMsg().
 */
RexxMethod5(intptr_t, wb_sendWinIntMsg, CSTRING, wm_msg, intptr_t, wParam, intptr_t, lParam,
            OPTIONAL_POINTERSTRING, _hwnd, CSELF, pCSelf)
{
    return (intptr_t)sendWinMsg(context, wm_msg, (WPARAM)wParam, (LPARAM)lParam, (HWND)_hwnd, (pCWindowBase)pCSelf);
}


/** WindowBase::sendWinHandleMsg()
 *  WindowBase::sendWinHandleMsgH()
 *
 *  Sends a message to a Windows window where WPARAM is a handle and LPARAM is a
 *  number.  The result is returned as a number or as a handle, depending on the
 *  invoking method.
 *
 *  @param  wm_msg  The Windows window message ID.  This can be specified as a
 *                  decimal number, or in "0xFFFF" format.
 *
 *  @param  wParam  The WPARAM value for the message.  The must be in pointer or
 *                  handle format.
 *  @param  lParam  The LPARAM value for the message.
 *
 *  @param  _hwnd   [OPTIONAL]  The handle of the window the message is sent to.
 *                  If omitted, the window handle of this object is used.
 *
 *  @return The result of sending the message, as returned by the operating
 *          system.
 *
 *  @note     Sets the .SystemErrorCode.
 *
 *  @remarks  This method is not meant to be documented to the user, it is
 *            intended to be used internally only.
 */
RexxMethod6(RexxObjectPtr, wb_sendWinHandleMsg, CSTRING, wm_msg, POINTERSTRING, wParam, intptr_t, lParam,
            OPTIONAL_POINTERSTRING, _hwnd, NAME, method, CSELF, pCSelf)
{
    LRESULT lr = sendWinMsg(context, wm_msg, (WPARAM)wParam, (LPARAM)lParam, (HWND)_hwnd, (pCWindowBase)pCSelf);

    if ( strlen(method) == 16 )
    {
        return context->Intptr((intptr_t)lr);
    }
    else
    {
        return pointer2string(context, (void *)lr);
    }
}


/** WindowBase::enable() / WindowBase::disable()
 *
 *  Enables or disables the window.  This function is mapped to both methods of
 *  WindowBase.
 *
 *  @return  True if the window was previously disabled, returns false if the
 *           window was not previously disabled.  Note that this is not succes
 *           or failure.  It always succeeds.
 *
 *  @remarks  The return was not previously documented.
 */
RexxMethod1(logical_t, wb_enable, CSELF, pCSelf)
{
    BOOL enable = TRUE;
    if ( msgAbbrev(context) == 'D' )
    {
        enable = FALSE;
    }
    return EnableWindow(getWBWindow(pCSelf), enable);
}

RexxMethod1(logical_t, wb_isEnabled, CSELF, pCSelf)
{
    return IsWindowEnabled(getWBWindow(pCSelf));
}

RexxMethod1(logical_t, wb_isVisible, CSELF, pCSelf)
{
    return IsWindowVisible(getWBWindow(pCSelf));
}

/** WindowBase::show() / WindowBase::hide()
 *
 *  Hides or shows the window.  This function is mapped to both methods of
 *  WindowBase.  The return for these methods was not previously documented.
 *
 *  @return  True if the window was previously visible.  Return false if the
 *           window was previously hidden.
 */
RexxMethod1(logical_t, wb_show, CSELF, pCSelf)
{
    return showWindow(getWBWindow(pCSelf), msgAbbrev(context));
}

/** WindowBase::showFast() / WindowBase::hideFast()
 *
 *  Hides or shows the window 'fast'.  What this means is the visible flag is
 *  set, but the window is not forced to update.
 *
 *  This function is mapped to both methods of WindowBase. The return for these
 *  methods was not previously documented.
 *
 *  @return  0 for no error, 1 for error.  An error is highly unlikely.
 */
RexxMethod1(uint32_t, wb_showFast, CSELF, pCSelf)
{
    return showFast(getWBWindow(pCSelf), msgAbbrev(context));
}

/** WindowBase::display()
 *
 *
 *  @return  0 for success, 1 for error.  An error is highly unlikely.
 */
RexxMethod2(uint32_t, wb_display, OPTIONAL_CSTRING, opts,  CSELF, pCSelf)
{
    char type;
    uint32_t ret = 0;
    bool doFast = false;
    HWND hwnd = getWBWindow(pCSelf);

    if ( opts != NULL && StrStrI(opts, "FAST") != NULL )
    {
        doFast = true;
    }

    if ( opts == NULL ) {type = 'S';}
    else if ( StrStrI(opts, "NORMAL"  ) != NULL ) {type = 'S';}
    else if ( StrStrI(opts, "DEFAULT" ) != NULL ) {type = 'S';}
    else if ( StrStrI(opts, "HIDE"    ) != NULL ) {type = 'H';}
    else if ( StrStrI(opts, "INACTIVE") != NULL )
    {
        if ( doFast )
        {
            userDefinedMsgException(context->threadContext, 1, "The keyword FAST can not be used with INACTIVE");
            goto done_out;
        }
        type = 'I';
    }
    else
    {
        type = 'S';
    }

    if ( doFast )
    {
        ret = showFast(hwnd, type);
    }
    else
    {
        showWindow(hwnd, type);
    }

done_out:
    return ret;
}

/** WindowsBase::draw() / WindowsBase::redrawClient() / WindowsBase::update()
 *
 *  Causes the entire client area of the the window to be redrawn.
 *
 *  This method maps to the draw(), update() and the redrawClient() methods.
 *  The implementation preserves existing behavior prior to ooRexx 4.0.1.  That
 *  is: the draw() method takes no argument and always uses false for the erase
 *  background arg. The update() method takes no argument and always uses true
 *  for the erase background arg, The redrawClient() method takes an argument to
 *  set the erase background arg. The argument can be either .true / .false (1
 *  or 0), or yes / no, or ja / nein.
 *
 *  @param  erase  [optional]  Whether the redrawClient operation should erase
 *                 the window background.  Can be true / false or yes / no.  The
 *                 default is false.
 *
 *  @return  0 for success, 1 for error.
 *
 *  @note  Sets the .SystemErrorCode.
 */
RexxMethod2(RexxObjectPtr, wb_redrawClient, OPTIONAL_CSTRING, erase, CSELF, pCSelf)
{
    HWND hwnd = getWBWindow(pCSelf);
    char flag = msgAbbrev(context);
    bool doErase;

    if ( flag == 'R' )
    {
        doErase = false;
    }
    else if ( flag == 'U' )
    {
        doErase = true;
    }
    else
    {
        doErase = isYes(erase);
    }
    return redrawRect(context, hwnd, NULL, doErase);
}

/** WindowsBase::redraw()
 *
 *  Causes the entire window, including frame, to be redrawn.
 *
 *  @return  0 for success, 1 for error.
 *
 *  @note  Sets the .SystemErrorCode.
 */
RexxMethod1(RexxObjectPtr, wb_redraw, CSELF, pCSelf)
{
    oodResetSysErrCode(context->threadContext);
    if ( RedrawWindow(getWBWindow(pCSelf), NULL, NULL,
                      RDW_ERASE | RDW_FRAME | RDW_INVALIDATE | RDW_UPDATENOW | RDW_ALLCHILDREN) == 0 )
    {
        oodSetSysErrCode(context->threadContext);
        return TheOneObj;
    }
    return TheZeroObj;
}

/** WindowBase::title / WindowBase::getText()
 *
 *  Gets the window text.
 *
 *  For a window with a frame, this is the window title.  But for a dialog
 *  control, this is the text for the control.  This of course varies depending
 *  on the type of control.  For a button, it is the button label, for an edit
 *  control it is the edit text, etc..
 *
 *  @return  On success, the window text, which could be the empty string.  On
 *           failure, the empty string.
 *
 *  @note  Sets the .SystemErrorCode.
 */
RexxMethod1(RexxStringObject, wb_getText, CSELF, pCSelf)
{
    RexxStringObject result = context->NullString();
    HWND hwnd = getWBWindow(pCSelf);

    // Whether this fails or succeeds doesn't matter, we just return result.
    rxGetWindowText(context, hwnd, &result);
    return result;
}

/** WindowBase::setText() / WindowBase::setTitle() / WindowBase::title=
 *
 *  Sets the window text to the value specified.
 *
 *  @param  text  The text to be set as the window text
 *
 *  @return  0 for success, 1 for error.
 *
 *  @note  Sets the .SystemErrorCode.
 *
 *  @remarks  Unfortunately, in 3.2.0, setText for an edit control was
 *            documented as returning the negated system error code and in 4.0.0
 *            setText for a static control was documented as returning the
 *            system error code (which is positive of course.)  The mapping here
 *            to the various versions of 'getText' is cleaner, but we may need
 *            to special case the return to match the previous documentation.
 */
RexxMethod2(wholenumber_t, wb_setText, CSTRING, text, CSELF, pCSelf)
{
    oodResetSysErrCode(context->threadContext);
    if ( SetWindowText(getWBWindow(pCSelf), text) == 0 )
    {
        oodSetSysErrCode(context->threadContext);
        return 1;
    }
    return 0;
}

/** WindowBase::windowRect()
 *
 *  Retrieves the dimensions of the bounding rectangle of this window.  The
 *  dimensions are in pixels and the coordinates are relative to the upper-left
 *  corner of the screen.
 *
 *  @param  hwnd  [OPTIONAL]  By default, the coordinates are for this window.
 *                However, the optional hwnd argument can be used to specify
 *                getting the coordinates for some other window.  See remarks.
 *
 *  @return  The bounding rectangle of the window as .Rect object.
 *
 *  @note  Sets the .SystemErrorCode.
 *
 *  @remarks  The windowRect() method supplies an alternative to both the
 *            getRect() and getWindowRect(hwnd) methods, to allow returning a
 *            .Rect object rather than a string of blank separated values.
 *
 *            Pre 4.0.1, getRect() was in WindowExtensions and getWindowRect()
 *            was in DialogExtensions.  The need to provide backward
 *            compatibility, use meaningful method names, maintain symmetry with
 *            the getClientRect(hwnd) method is what drove the decision to allow
 *            the optional hwnd argument to this method.
 */
RexxMethod2(RexxObjectPtr, wb_windowRect, OPTIONAL_POINTERSTRING, _hwnd, CSELF, pCSelf)
{
    HWND hwnd = (argumentOmitted(1) ? getWBWindow(pCSelf) : (HWND)_hwnd);
    return oodGetWindowRect(context, hwnd);
}


/** WindowBase::clientRect()
 *
 *  Retrieves the coordinates of a window's client area.  The coordinates are in
 *  pixels.
 *
 *  The client coordinates specify the upper-left and lower-right corners of the
 *  client area. Because client coordinates are relative to the upper-left
 *  corner of a window's client area, the coordinates of the upper-left corner
 *  are always (0,0).
 *
 *  @param  hwnd  [OPTIONAL]  By default, the coordinates are for this window.
 *                However, the optional hwnd argument can be used to specify
 *                getting the coordinates for some other window.  See remarks.
 *
 *  @return  The coordinates of the client area of the window as a .Rect object.
 *
 *  @note  Sets the .SystemErrorCode.
 *
 *  @remarks  The clientRect() method supplies an alternative to the
 *            getClinetRect(hwnd) method, to allow returning a .Rect object
 *            rather than a string of blank separated values.
 *
 *            Pre 4.0.1, getClientRect() was in WindowExtensions.  The need to
 *            provide backward compatibility, use meaningful method names, and
 *            maintain symmetry with the getRect() and getWindowRect(hwnd)
 *            methods is what drove the decision to move this method to
 *            WidnowBase and allow the optional hwnd argument to this method.
 */
RexxMethod2(RexxObjectPtr, wb_clientRect, OPTIONAL_POINTERSTRING, _hwnd, CSELF, pCSelf)
{
    HWND hwnd = (argumentOmitted(1) ? getWBWindow(pCSelf) : (HWND)_hwnd);

    RECT r = {0};
    oodGetClientRect(context, (HWND)hwnd, &r);
    return rxNewRect(context, &r);
}

/** WindowBase::setRect()
 *
 *  Changes the size, and position of a child, pop-up, or top-level window.
 *
 *  Provides an interface to the Windows API, SetWindowPos().  The inteface is
 *  incomplete, at this time, as it does not recognize all the allowable flags
 *  to SetWindowPos.  In particular, it always uses SWP_NOZORDER.
 *
 *  By specifying either NOSIZE or NOMOVE options the programmer can only move
 *  or only resize the window.
 *
 *  @param coordinates  The coordinates of a point / size rectangle, given in
 *                      pixels
 *
 *    Form 1:  A .Rect object.
 *    Form 2:  A .Point object and a .Size object.
 *    Form 3:  x1, y1, cx, cy
 *
 *  @param  flags   [OPTIONAL] Keywords specifying the behavior of the method.
 *
 *  @return  0 for success, 1 on error.
 *
 *  @note  Sets the .SystemErrorCode.
 *
 *  @remarks  Microsoft says: If the SWP_SHOWWINDOW or SWP_HIDEWINDOW flag is
 *            set, the window cannot be moved or sized.  But, that does not
 *            appear to be true.
 */
RexxMethod2(RexxObjectPtr, wb_setRect, ARGLIST, args, CSELF, pCSelf)
{
    RexxMethodContext *c = context;
    oodResetSysErrCode(context->threadContext);

    // A RECT is used to return the values, even though the semantics are not
    // quite correct.  (right will really be cx and bottom will be cy.)
    size_t countArgs;
    int    argsUsed;
    RECT   rect;
    if ( ! getRectFromArglist(context, args, &rect, false, 1, 5, &countArgs, &argsUsed) )
    {
        return NULLOBJECT;
    }

    RexxObjectPtr obj;
    CSTRING options = "";
    if ( argsUsed == 1 )
    {
        if ( countArgs > 2 )
        {
            return tooManyArgsException(context->threadContext, 2);
        }
        if ( countArgs == 2 )
        {
            obj = c->ArrayAt(args, 2);
            options = c->ObjectToStringValue(obj);
        }
    }
    else if ( argsUsed == 2 )
    {
        if ( countArgs > 3 )
        {
            return tooManyArgsException(context->threadContext, 3);
        }
        if ( countArgs == 3 )
        {
            obj = c->ArrayAt(args, 3);
            options = c->ObjectToStringValue(obj);
        }
    }
    else
    {
        if ( countArgs == 5 )
        {
            obj = c->ArrayAt(args, 5);
            options = c->ObjectToStringValue(obj);
        }
    }

    uint32_t opts = parseShowOptions(options);
    if ( SetWindowPos(getWBWindow(pCSelf), NULL, rect.left, rect.top, rect.right, rect.bottom, opts) == 0 )
    {
        oodSetSysErrCode(context->threadContext);
        return TheOneObj;
    }
    return TheZeroObj;
}

/** WindowBase::resizeTo()
 *  WindowBase::moveTo()
 *
 *  Resize to, changes the size of this window.  The new size is specified in
 *  pixels.
 *
 *  Move to, changes the position of this window.  The new position is specified
 *  as the coordinates of the upper left corner of the window, in pixels.
 *
 *  @param  coordinates  The new position (x, y) or new size (cx, cy) in pixels.
 *
 *    resizeTo()
 *      Form 1:  A .Size object.
 *      Form 2:  cx, cy
 *
 *    moveTo()
 *      Form 1:  A .Point object.
 *      Form 2:  x, y
 *
 *  @param  flags   [OPTIONAL] Keywords that control the behavior of the method.
 *
 *  @return  0 for success, 1 on error.
 *
 *  @note  Sets the .SystemErrorCode.
 *
 *  @remarks  No effort is made to ensure that only a .Size object is used for
 *            resizeTo() and only a .Point object for moveTo().
 */
RexxMethod3(RexxObjectPtr, wb_resizeMove, ARGLIST, args, NAME, method, CSELF, pCSelf)
{
    RexxMethodContext *c = context;
    oodResetSysErrCode(context->threadContext);

    // POINT and SIZE structs are binary compatible.  A POINT is used to return
    // the values, even though the semantics are not quite correct for
    // resizeTo(). (x will really be cx and y will be cy.)
    size_t countArgs;
    int    argsUsed;
    POINT  point;
    if ( ! getPointFromArglist(context, args, &point, 1, 3, &countArgs, &argsUsed) )
    {
        return NULLOBJECT;
    }

    RexxObjectPtr obj;
    CSTRING options = "";
    if ( argsUsed == 1 )
    {
        if ( countArgs > 2 )
        {
            return tooManyArgsException(context->threadContext, 2);
        }
        if ( countArgs == 2 )
        {
            // The object at index 2 has to exist, otherwise countArgs would
            // equal 1.
            obj = c->ArrayAt(args, 2);
            options = c->ObjectToStringValue(obj);
        }
    }
    else if ( argsUsed == 2 )
    {
        if ( countArgs == 3 )
        {
            // The object at index 3 has to exist, otherwise countArgs would
            // equal 2.
            obj = c->ArrayAt(args, 3);
            options = c->ObjectToStringValue(obj);
        }
    }

    uint32_t opts = parseShowOptions(options);
    RECT r = {0};

    if ( *method == 'R' )
    {
        opts |= SWP_NOMOVE;
        r.right = point.x;
        r.bottom = point.y;
    }
    else
    {
        opts |= SWP_NOSIZE;
        r.left = point.x;
        r.top = point.y;
    }

    if ( SetWindowPos(getWBWindow(pCSelf), NULL, r.left, r.top, r.right, r.bottom, opts) == 0 )
    {
        oodSetSysErrCode(context->threadContext);
        return TheOneObj;
    }
    return TheZeroObj;
}


/** WindowBase::getRealSize()
 *  WindowBase::getRealPos()
 *
 *  Retrieves the size, or the position, in pixels, of this window
 *
 *  @return  The size as a .Size object, or the position as a .Point object.
 *
 *  @note  Sets the .SystemErrorCode.
 */
RexxMethod2(RexxObjectPtr, wb_getSizePos, NAME, method, CSELF, pCSelf)
{
    oodResetSysErrCode(context->threadContext);

    RECT r = {0};
    if ( GetWindowRect(getWBWindow(pCSelf), &r) == 0 )
    {
        oodSetSysErrCode(context->threadContext);
    }
    if ( method[7] == 'S' )
    {
        return rxNewSize(context, (r.right - r.left), (r.bottom - r.top));
    }

    return rxNewPoint(context, r.left, r.top);
}


/** WindowBase::clear()
 *
 *  'Clears' the dialog's client area by painting the entire area with the
 *  system color COLOR_BTNFACE brush.  This is the same color as the default
 *  dialog background color so it has the effect of erasing everything on the
 *  client area, or 'clearing' it.
 *
 *  @return 0 on success, 1 for error.
 *
 *  @note  Sets the .SystemErrorCode.
 *
 *  @remarks  Prior to 4.0.1, this method was in both the DialogExtensions and
 *            Dialog class. However, each method simply uses the object's window
 *            handle so it makes more sense to be a WindowBase method.
 *
 *            Note this also: both methods were broken.  They used the window
 *            rect of the window, not the client rect.  In addition,
 *            GetWindowDC() was used instead of GetDC().  This has the effect of
 *            painting in the nonclient area of the window, which normally is
 *            not done.  Since the methods were broken and the documentation was
 *            vague, it is not possible to tell what the original intent was.
 *
 *            Since clearing the window rect of the dialog erases the title bar,
 *            and there is no way for the ooDialog programmer to redraw the
 *            title bar or title bar buttons, the assumption is that the methods
 *            were intended to clear the client area.
 */
RexxMethod1(RexxObjectPtr, wb_clear, CSELF, pCSelf)
{
    HWND hwnd = getWBWindow(pCSelf);

    RECT r = {0};
    if ( oodGetClientRect(context, hwnd, &r) == TheOneObj )
    {
        return TheOneObj;
    }
    return clearRect(context, hwnd, &r);
}


/** WindowBase::foreGroundWindow()
 *
 *  Returns the handle of the window in the foreground.  The foreground window
 *  can be NULL in certain circumstances, such as when a window is losing
 *  activation.
 *
 *  @note  Sets the .SystemErrorCode, but it is unlikely that the operating
 *         system sets last error during the execution of this method.
 */
RexxMethod0(POINTERSTRING, wb_foreGroundWindow)
{
    oodResetSysErrCode(context->threadContext);
    HWND hwnd = GetForegroundWindow();
    oodSetSysErrCode(context->threadContext);
    return hwnd;
}


/** WindowBase::screen2client()
 *  WindowBase::client2screen()
 *
 *  screen2client() converts the screen coordinates of the specified point on
 *  the screen to client-area coordinates of this window.
 *
 *  This method uses the screen coordinates given in the .Point object to
 *  compute client-area coordinates. It then replaces the screen coordinates
 *  with the client coordinates. The new coordinates are relative to the
 *  upper-left corner of this window's client area.
 *
 *
 *  client2screen() converts the client-area coordinates of the specified point
 *  to screen coordinates.
 *
 *  This method uses the client-area coordinates given in the .Point object to
 *  compute screen coordinates. It then replaces the client-area coordinates
 *  with the screen coordinates. The new coordinates are relative to the
 *  upper-left corner of the screen.
 *
 *  @param  pt  [in/out] On entry, the coordinates to be converted, on exit the
 *              converted coordinates.
 *
 *  @return True on success, false on failure.  The .SystemErrorCode should
 *          contain the reason for failure.
 *
 *  @note  Sets the .SystemErrorCode.
 *
 */
RexxMethod4(logical_t, wb_screenClient, RexxObjectPtr, pt, NAME, method, OSELF, self, CSELF, pCSelf)
{
    oodResetSysErrCode(context->threadContext);
    BOOL success = FALSE;

    HWND hwnd = getWBWindow(pCSelf);
    if ( hwnd == NULL )
    {
        noWindowsDialogException(context, self);
        goto done_out;
    }

    POINT *p = rxGetPoint(context, pt, 1);
    if ( p != NULL )
    {
        if ( *method == 'S' )
        {
            success = ScreenToClient(hwnd, p);
        }
        else
        {
            success = ClientToScreen(hwnd, p);
        }

        if ( ! success )
        {
            oodSetSysErrCode(context->threadContext);
        }
    }

done_out:
    return success;
}

/** WindowBase::getWindowLong()  [private]
 *
 *  Retrieves information about this window.  Specifically, the information
 *  available through the GetWindowLong().  Internally this used to get the
 *  GWL_ID, GWL_STYLE, and GWL_EXSTYLE values.
 *
 *  @param  flag  The index for the information to be retrieved.
 *
 *  @return  The unsigned 32-bit information for the specified index.
 *
 *  @remarks  The other indexes, besides the 3 mentioned above are all pointer
 *            or handle values.  Because of this, this method should not be
 *            documented.  The implementation may change.
 */
RexxMethod2(uint32_t, wb_getWindowLong_pvt, int32_t, flag, CSELF, pCSelf)
{
    return GetWindowLong(getWBWindow(pCSelf), flag);
}


/**
 *  Methods for the .PlainBaseDialog class.
 */
#define PLAINBASEDIALOG_CLASS    "PlainBaseDialog"
#define CONTROLBAG_ATTRIBUTE     "PlainBaseDialogControlBag"


static inline HWND getPBDWindow(void *pCSelf)
{
    return ((pCPlainBaseDialog)pCSelf)->hDlg;
}

HWND getPBDControlWindow(RexxMethodContext *c, pCPlainBaseDialog pcpbd, RexxObjectPtr rxID)
{
    HWND hCtrl = NULL;
    oodResetSysErrCode(c->threadContext);

    uint32_t id;
    if ( ! oodSafeResolveID(&id, c, pcpbd->rexxSelf, rxID, -1, 1) || (int)id < 0 )
    {
        oodSetSysErrCode(c->threadContext, ERROR_INVALID_WINDOW_HANDLE);
    }
    else
    {
        hCtrl = GetDlgItem(pcpbd->hDlg, id);
        if ( hCtrl == NULL )
        {
            oodSetSysErrCode(c->threadContext);
        }
    }
    return hCtrl;
}

int32_t stopDialog(HWND hDlg)
{
    if ( hDlg != NULL )
    {
        DIALOGADMIN * dlgAdm = seekDlgAdm(hDlg);
        if ( dlgAdm != NULL)
        {
            return DelDialog(dlgAdm);
        }
        else
        {
            return -1;
        }
    }
    else if ( hDlg == NULL && topDlg != NULL )
    {
        // Remove the top most.  This scenario is invoked from the DynamicDialog
        // class when create() or load() fail.
        return DelDialog(topDlg);
    }
    return -1;
}

RexxObjectPtr setDlgHandle(RexxMethodContext *c, pCPlainBaseDialog pcpbd, HWND hDlg)
{
    pCWindowBase pcwb = pcpbd->wndBase;
    pcpbd->hDlg = hDlg;
    pcpbd->enCSelf->hDlg = hDlg;

    if ( pcpbd->weCSelf != NULL )
    {
        pcpbd->weCSelf->hwnd = hDlg;
    }

    if ( pcpbd->hDlg != NULL )
    {
        pcwb->hwnd = pcpbd->hDlg;
        pcwb->rexxHwnd = c->RequestGlobalReference(pointer2string(c, hDlg));
    }
    else
    {
        pcwb->hwnd = NULL;
        if ( pcwb->rexxHwnd != NULLOBJECT && pcwb->rexxHwnd != TheZeroObj )
        {
            c->ReleaseGlobalReference(pcwb->rexxHwnd);
        }
        pcwb->rexxHwnd = TheZeroObj;
    }

    return NULLOBJECT;
}


/**
 * Gets the window handle of the dialog control that has the focus.  The call to
 * GetFocus() needs to run in the window thread of the dialog to ensure that the
 * correct handle is obtained.
 *
 * @param c       Method context we are operating in.
 * @param hDlg    Handle to the dialog of interest.
 *
 * @return  The window handle of the dialog control with the focus, or 0 on
 *          failure.
 */
RexxObjectPtr oodGetFocus(RexxMethodContext *c, HWND hDlg)
{
   HWND hwndFocus = (HWND)SendMessage(hDlg, WM_USER_GETFOCUS, 0,0);
   return pointer2string(c, hwndFocus);
}


/**
 * Brings the specified window to the foreground and returns the previous
 * foreground window.
 *
 * @param c
 * @param hwnd
 *
 * @return The previous window handle as a Rexx object, or 0 on error.
 *
 * @remarks  SetForegroundWindow() is not documented as setting last error.  So,
 *           if it fails, last error is checked.  If it is set, it is used.  If
 *           it is not set, we arbitrarily use ERROR_NOTSUPPORTED.  On XP at
 *           least, last error is set to 5, access denied.
 */
RexxObjectPtr oodSetForegroundWindow(RexxMethodContext *c, HWND hwnd)
{
    oodResetSysErrCode(c->threadContext);

    if ( hwnd == NULL || ! IsWindow(hwnd) )
    {
        oodSetSysErrCode(c->threadContext, ERROR_INVALID_WINDOW_HANDLE);
        return TheZeroObj;
    }

    HWND hwndPrevious = GetForegroundWindow();
    if ( hwndPrevious != hwnd )
    {
        SetLastError(0);
        if ( SetForegroundWindow(hwnd) == 0 )
        {
            uint32_t rc = GetLastError();
            if ( rc == 0 )
            {
                rc = ERROR_NOT_SUPPORTED;
            }
            oodSetSysErrCode(c->threadContext, rc);
            return TheZeroObj;
        }
    }
    return pointer2string(c, hwndPrevious);
}

bool initEventNotification(RexxMethodContext *c, DIALOGADMIN *dlgAdm, RexxObjectPtr self, pCEventNotification *ppCEN)
{
    RexxBufferObject obj = c->NewBuffer(sizeof(CEventNotification));
    if ( obj == NULLOBJECT )
    {
        return false;
    }

    pCEventNotification pcen = (pCEventNotification)c->BufferData(obj);
    pcen->dlgAdm = dlgAdm;
    pcen->hDlg = NULL;
    pcen->rexxSelf = self;

    c->SendMessage1(self, "INIT_EVENTNOTIFICATION", obj);
    *ppCEN = pcen;
    return true;
}

RexxMethod1(RexxObjectPtr, pbdlg_init_cls, OSELF, self)
{
    if ( isOfClassType(context, self, PLAINBASEDIALOG_CLASS) )
    {
        ThePlainBaseDialogClass = (RexxClassObject)self;
        context->RequestGlobalReference(ThePlainBaseDialogClass);

        RexxBufferObject buf = context->NewBuffer(sizeof(CPlainBaseDialogClass));
        if ( buf != NULLOBJECT )
        {
            pCPlainBaseDialogClass pcpbdc = (pCPlainBaseDialogClass)context->BufferData(buf);

            strcpy(pcpbdc->fontName, DEFAULT_FONTNAME);
            pcpbdc->fontSize = DEFAULT_FONTSIZE;
            context->SetObjectVariable("CSELF", buf);
        }
    }
    return NULLOBJECT;
}

RexxMethod2(RexxObjectPtr, pbdlg_setDefaultFont_cls, CSTRING, fontName, uint32_t, fontSize)
{
    pCPlainBaseDialogClass pcpbdc = getPBDClass_CSelf(context);

    if ( strlen(fontName) > (MAX_DEFAULT_FONTNAME - 1) )
    {
        stringTooLongException(context->threadContext, 1, MAX_DEFAULT_FONTNAME, strlen(fontName));
    }
    else
    {
        strcpy(pcpbdc->fontName, fontName);
        pcpbdc->fontSize = fontSize;
    }
    return NULLOBJECT;
}

RexxMethod1(CSTRING, pbdlg_getFontName_cls, CSELF, pCSelf)
{
    pCPlainBaseDialogClass pcpbdc = getPBDClass_CSelf(context);
    return pcpbdc->fontName;
}
RexxMethod1(uint32_t, pbdlg_getFontSize_cls, CSELF, pCSelf)
{
    pCPlainBaseDialogClass pcpbdc = getPBDClass_CSelf(context);
    return pcpbdc->fontSize;
}

RexxMethod5(RexxObjectPtr, pbdlg_init, RexxObjectPtr, library, RexxObjectPtr, resource,
            OPTIONAL_RexxObjectPtr, dlgDataStem, OPTIONAL_RexxObjectPtr, hFile, OSELF, self)
{
    RexxMethodContext *c = context;

    RexxObjectPtr result = TheOneObj;  // This is an error return.

    context->SetObjectVariable("LIBRARY", library);
    context->SetObjectVariable("RESOURCE", resource);

    if ( argumentExists(3) )
    {
        context->SetObjectVariable("DLGDATA.", dlgDataStem);
        context->SetObjectVariable("USESTEM", TheTrueObj);
    }
    else
    {
        context->SetObjectVariable("DLGDATA.", TheNilObj);
        context->SetObjectVariable("USESTEM", TheFalseObj);
    }

    // Get a buffer for the PlainBaseDialog CSelf.
    RexxBufferObject cselfBuffer = context->NewBuffer(sizeof(CPlainBaseDialog));
    if ( cselfBuffer == NULLOBJECT )
    {
        goto err_out;
    }

    // Initialize the window base.
    pCWindowBase pWB;
    if ( ! initWindowBase(context, NULL, self, &pWB) )
    {
        goto done_out;
    }

    pCPlainBaseDialog pcpbd = (pCPlainBaseDialog)context->BufferData(cselfBuffer);

    pcpbd->autoDetect = TRUE;
    pcpbd->wndBase = pWB;
    pcpbd->weCSelf = NULL;
    pcpbd->rexxSelf = self;
    pcpbd->hDlg = NULL;
    context->SetObjectVariable("CSELF", cselfBuffer);

    // The adm attribute needs to be set to whatever the result of allocating
    // the dialog administration block is.  If it is null, then the attribute
    // will be set to 0, which is what most of the existing ooDialog code
    // expects.

    DIALOGADMIN *dlgAdm = allocDlgAdmin(context);

    pcpbd->dlgAdm = dlgAdm;
    context->SetObjectVariable("ADM", pointer2string(context, dlgAdm));
    if ( dlgAdm == NULL )
    {
        goto done_out;
    }

    // Initialize the event notification mixin class.  The only thing that could
    // fail is getting a buffer from the interpreter kernel.  If that happens an
    // exceptions is raised and we should not need to do any clean up.
    pCEventNotification pEN = NULL;
    if ( ! initEventNotification(context, dlgAdm, self, &pEN) )
    {
        goto done_out;
    }
    pcpbd->enCSelf = pEN;

    result = TheZeroObj;

    context->SetObjectVariable("PARENTDLG", TheNilObj);
    context->SetObjectVariable("FINISHED", TheZeroObj);
    context->SetObjectVariable("PROCESSINGLOAD", TheFalseObj);

    // We don't check the return of AddTheMessage() because the message table
    // can not be full at this point, we are just starting out.  A memory
    // allocation failure, which is highly unlikely, will just be ignored.  If
    // this ooRexx process is out of memory, that will quickly show up.
    AddTheMessage(dlgAdm, WM_COMMAND, 0xFFFFFFFF, IDOK,     UINTPTR_MAX, 0, 0, "OK", 0);
    AddTheMessage(dlgAdm, WM_COMMAND, 0xFFFFFFFF, IDCANCEL, UINTPTR_MAX, 0, 0, "Cancel", 0);
    AddTheMessage(dlgAdm, WM_COMMAND, 0xFFFFFFFF, IDHELP,   UINTPTR_MAX, 0, 0, "Help", 0);

    // Set our default font to the PlainBaseDialog class default font.
    pCPlainBaseDialogClass pcpbdc = getPBDClass_CSelf(c);
    strcpy(pcpbd->fontName, pcpbdc->fontName);
    pcpbd->fontSize = pcpbdc->fontSize;

    // TODO at this point calculate the true dialog base units and set them into CPlainBaseDialog.

    c->SendMessage1(self, "CHILDDIALOGS=", rxNewList(context));       // self~childDialogs = .list~new
    c->SendMessage0(self, "INITAUTODETECTION");                       // self~initAutoDetection
    c->SendMessage1(self, "DATACONNECTION=", c->NewArray(50));        // self~dataConnection = .array~new(50)
    c->SendMessage1(self, "AUTOMATICMETHODS=", rxNewQueue(context));  // self~autoMaticMethods = .queue~new

    RexxDirectoryObject constDir = c->NewDirectory();
    c->SendMessage1(self, "CONSTDIR=", constDir);                     // self~constDir = .directory~new

    c->DirectoryPut(constDir, c->Int32(IDOK),             "IDOK");
    c->DirectoryPut(constDir, c->Int32(IDCANCEL),         "IDCANCEL");
    c->DirectoryPut(constDir, c->Int32(IDHELP),           "IDHELP");
    c->DirectoryPut(constDir, c->Int32(IDC_STATIC),       "IDC_STATIC");
    c->DirectoryPut(constDir, c->Int32(IDI_DLG_OODIALOG), "IDI_DLG_OODIALOG");
    c->DirectoryPut(constDir, c->Int32(IDI_DLG_APPICON),  "IDI_DLG_APPICON");
    c->DirectoryPut(constDir, c->Int32(IDI_DLG_APPICON2), "IDI_DLG_APPICON2");
    c->DirectoryPut(constDir, c->Int32(IDI_DLG_OOREXX),   "IDI_DLG_OOREXX");
    c->DirectoryPut(constDir, c->Int32(IDI_DLG_DEFAULT),  "IDI_DLG_DEFAULT");

    if ( argumentExists(4) )
    {
        c->SendMessage1(self, "PARSEINCLUDEFILE", hFile);
    }

done_out:
    c->SendMessage1(self, "INITCODE=", result);
err_out:
    return result;
}

RexxMethod1(RexxObjectPtr, pbdlg_unInit, CSELF, pCSelf)
{
    if ( pCSelf != NULLOBJECT )
    {
        pCPlainBaseDialog pcpbd = (pCPlainBaseDialog)pCSelf;

        DIALOGADMIN *adm = pcpbd->dlgAdm;
        if ( adm != NULL )
        {
            EnterCriticalSection(&crit_sec);

            if ( dialogInAdminTable(adm) )
            {
                DelDialog(adm);
            }
            safeLocalFree(adm->pMessageQueue);
            LocalFree(adm);
            ((pCPlainBaseDialog)pCSelf)->dlgAdm = NULL;

            LeaveCriticalSection(&crit_sec);
        }

        pCWindowBase pcwb = pcpbd->wndBase;
        if ( pcwb->rexxHwnd != TheZeroObj )
        {
            context->ReleaseGlobalReference(pcwb->rexxHwnd);
            pcwb->rexxHwnd = TheZeroObj;
        }
    }

    // Set the adm attribute to null here, to be sure it reflects that, no
    // matter what, dlgAdm is null at this point.
    context->SetObjectVariable("ADM", TheZeroObj);

    return TheZeroObj;
}

/** PlainBaseDialog::autoDetect  [attribute get]
 */
RexxMethod1(RexxObjectPtr, pbdlg_getAutoDetect, CSELF, pCSelf)
{
    return ( ((pCPlainBaseDialog)pCSelf)->autoDetect ? TheTrueObj : TheFalseObj );
}
/** PlainBaseDialog::autoDetect  [attribute set]
 */
RexxMethod2(CSTRING, pbdlg_setAutoDetect, logical_t, on, CSELF, pCSelf)
{
    ((pCPlainBaseDialog)pCSelf)->autoDetect = on;
    return NULLOBJECT;
}

/** PlainBaseDialog::fontName  [attribute get]
 */
RexxMethod1(CSTRING, pbdlg_getFontName, CSELF, pCSelf)
{
    return ( ((pCPlainBaseDialog)pCSelf)->fontName );
}
/** PlainBaseDialog::fontSize  [attribute get]
 */
RexxMethod1(uint32_t, pbdlg_getFontSize, CSELF, pCSelf)
{
    return ( ((pCPlainBaseDialog)pCSelf)->fontSize );
}

/** PlainBaseDialog::dlgHandle  [attribute get] / PlainBaseDialog::getSelf()
 */
RexxMethod1(RexxObjectPtr, pbdlg_getDlgHandle, CSELF, pCSelf)
{
    return ( ((pCPlainBaseDialog)pCSelf)->wndBase->rexxHwnd );
}

/** PlainBaseDialog::sendMessageToControl()
 *  PlainBaseDialog::sendMessageToControlH()
 *
 *  Sends a window message to the specified dialog control.
 *
 *  @param  rxID    The resource ID of the control, may be numeric or symbolic.
 *
 *  @param  wm_msg  The Windows window message ID.  This can be specified in
 *                  either "0xFFFF" or numeric format.
 *
 *  @param  _wParam  The WPARAM value for the message.
 *  @param  _lParam  The LPARAM value for the message.
 *
 *  @return The result of sending the message, as returned by the operating
 *          system.  sendMessageToControl() returns the result as a number.
 *          sendMessageToControlH() returns the result as an operating system
 *          handle.
 *
 *  @note  Sets the .SystemErrorCode.
 *
 *  @remarks  sendMessageToItem(), which forwards to this method, a method of
 *            DialogExtensions, but was moved back into PlainBaseDialog after
 *            the 4.0.0 release.  Therefore, sendMessageToControl() can not
 *            raise an exception for a bad resource ID, sendMessageToControlH()
 *            can.
 */
RexxMethod6(RexxObjectPtr, pbdlg_sendMessageToControl, RexxObjectPtr, rxID, CSTRING, wm_msg,
            RexxObjectPtr, _wParam, RexxObjectPtr, _lParam, NAME, method, CSELF, pCSelf)
{
    bool doIntReturn = (strlen(method) == 20 ? true : false);

    HWND hCtrl = getPBDControlWindow(context, (pCPlainBaseDialog)pCSelf, rxID);
    if ( hCtrl == NULL )
    {
        if ( doIntReturn )
        {
            return TheNegativeOneObj;
        }
        else
        {
            return invalidTypeException(context->threadContext, 1, " resource ID, there is no matching dialog control");
        }
    }

    return sendWinMsgGeneric(context, hCtrl, wm_msg, _wParam, _lParam, 2, doIntReturn);
}


/** PlainBaseDialog::sendMessageToWindow()
 *  PlainBaseDialog::sendMessageToWindowH()
 *
 *  Sends a window message to the specified window.
 *
 *  @param  _hwnd   The handle of the window the message is sent to.
 *  @param  wm_msg  The Windows window message ID.  This can be specified in
 *                  either "0xFFFF" or numeric format.
 *
 *  @param  _wParam  The WPARAM value for the message.
 *  @param  _lParam  The LPARAM value for the message.
 *
 *  @return The result of sending the message, as returned by the operating
 *          system.  sendMessageToWindow() returns the result as a number and
 *          sendMessageToWindowH() returns the the result as an operating systme
 *          handle.
 *
 *  @note  Sets the .SystemErrorCode.
 */
RexxMethod6(RexxObjectPtr, pbdlg_sendMessageToWindow, CSTRING, _hwnd, CSTRING, wm_msg,
            RexxObjectPtr, _wParam, RexxObjectPtr, _lParam, NAME, method, CSELF, pCSelf)
{
    HWND hwnd = (HWND)string2pointer(_hwnd);

    bool doIntReturn = (strlen(method) == 19 ? true : false);

    return sendWinMsgGeneric(context, hwnd, wm_msg, _wParam, _lParam, 2, doIntReturn);
}

/** PlainBaseDialog::get()
 *
 *  Returns the handle of the "top" dialog.
 *
 *  @return  The handle of the top dialog, or the null handle if there is no
 *           top dialog.
 *
 *  @remarks  This is a documented method from the original ooDialog
 *            implementation.  The original documentaion said: "The Get method
 *            returns the handle of the current Windows dialog."  The
 *            implementation has always been to get the handle of the topDlg.  I
 *            have never understood what the point of this method is, since the
 *            topDlg, usually, just reflects the last dialog created.
 */
RexxMethod0(RexxObjectPtr, pbdlg_get)
{
    if (topDlg && topDlg->TheDlg)
    {
        return pointer2string(context, topDlg->TheDlg);
    }
    else
    {
        return TheZeroObj;  // TODO for now, 0. Should be null Pointer.
    }
}

/** PlaingBaseDialog::setDlgFont()
 *
 *  Sets the font that will be used for the font of the underlying Windows
 *  dialog, when it is created.  This is primarily of use in a UserDialog or a
 *  subclasses of a UserDialog.
 *
 *  In a ResDialog, the font of the compiled binary resource will be used and
 *  the font set by this method has no bearing.  In a RcDialog, if the resource
 *  script file specifies the font, that font will be used.
 *
 *  Likewise, in a UserDialog, if the programmer specifies a font in the create
 *  method call (or createCenter() method call) the specified font over-rides
 *  what is set by this method.
 *
 *  However, setting the font through the setDlgFont() method allows the true
 *  dialog unit values to be correctly calculated.  That is the primary use for
 *  this method.  A typical sequence might be:
 *
 *  dlg = .MyDialog~new
 *  dlg~setFont("Tahoma", 12)
 *  ...
 *  ::method defineDialog
 *  ...
 *
 *  @param  fontName  The font name, such as Tahoma.  The name must be less than
 *                    256 characters in length.
 *  @param  fontSize  [optional]  The point size of the font, for instance 10.
 *                    The default if this argument is omitted is 8.
 *
 *  @return  This method does not return a value.
 */
RexxMethod3(RexxObjectPtr, pbdlg_setDlgFont, CSTRING, fontName, OPTIONAL_uint32_t, fontSize, CSELF, pCSelf)
{
    pCPlainBaseDialog pcpbd = (pCPlainBaseDialog)pCSelf;

    if ( strlen(fontName) > (MAX_DEFAULT_FONTNAME - 1) )
    {
        stringTooLongException(context->threadContext, 1, MAX_DEFAULT_FONTNAME, strlen(fontName));
    }
    else
    {
        if ( argumentOmitted(2) )
        {
            fontSize = DEFAULT_FONTSIZE;
        }
        strcpy(pcpbd->fontName, fontName);
        pcpbd->fontSize = fontSize;

        // TODO at this point calculate the true dialog base units from the font
        // and set them into CPlainBaseDialog.
    }
    return NULLOBJECT;
}


/** PlainBaseDialog::getWindowText()
 *
 *  Gets the text of the specified window.
 *
 *  For a window with a frame, this is the window title.  But for a dialog
 *  control, this is the text for the control.  This of course varies depending
 *  on the type of control.  For a button, it is the button label, for an edit
 *  control it is the edit text, etc..
 *
 *  @param  hwnd  The handle of the window.
 *
 *  @return  On success, the window text, which could be the empty string.  On
 *           failure, the empty string.
 *
 *  @note  Sets the .SystemErrorCode.
 */
RexxMethod1(RexxStringObject, pbdlg_getWindowText, POINTERSTRING, hwnd)
{
    RexxStringObject result = context->NullString();
    rxGetWindowText(context, (HWND)hwnd, &result);
    return result;
}

/** PlainBaseDialog::show()
 *
 *  Shows the dialog in the manner specified by the keyword option.  The dialog
 *  can be hidden, minimized, made visible, brought to the foreground, etc.
 *
 *  @param  keyword  A single keyword that controls how the dialog is shown.
 *                   Case is not significant.
 *
 *  @return  True if the dialog was previously visible.  Return false if the
 *           window was previously hidden.  Note that this is not a success or
 *           failure return, rather it is what the Windows API returns.
 *
 *  @note  The SHOWTOP DEFAULT NORMAL keywords are all equivalent to
 *         SW_SHOWNORMAL.
 *
 *         Key words are NORMAL DEFAULT SHOWTOP RESTORE MIN MAX HIDE INACTIVE
 *
 *  @remarks  Note!  This method over-rides the WindowBase::show() method.  The
 *            WindowBase::show() method, does a show normal only.  This method
 *            takes a number of keyword options, whereas WindowBase::show()
 *            method takes no options.
 *
 *            Therefore, all dialog objects have this show method.  All dialog
 *            control objects have the no-option show() method.
 *
 *            The WindowBase::display() method is similar to this show(), but
 *            with fewer, and somewhat different, keywords.
 *
 *            PlainBaseDialog::restore() also maps to this function.  restore()
 *            takes no options.
 */
RexxMethod3(logical_t, pbdlg_show, OPTIONAL_CSTRING, options, NAME, method, CSELF, pCSelf)
{
    HWND hwnd = getPBDWindow(pCSelf);

    char flag = (*method == 'R' ? 'R' : 'S')  ;

    if ( options != NULL && *options != '\0' && flag != 'R' )
    {
        switch ( toupper(*options) )
        {
            case 'S' :
                SetForegroundWindow(hwnd);
                break;

            case 'M' :
                flag = (stricmp("MIN", options) == 0 ? 'M' : 'X');
                break;

            case 'H' :
                flag = 'H';
                break;

            case 'I' :
                flag = 'I';
                break;

            case 'R' :
                flag = 'R';
                break;

            case 'D' :
            case 'N' :
            default  :
                break;

        }
    }
    return showWindow(hwnd, flag);
}


/** PlainBaseDialog::showWindow() / PlainBaseDialog::hideWindow()
 *  PlainBaseDialog::showWindow() / PlainBaseDialog::hideWindow()
 *
 *  Hides or shows the specified window, normally, or 'fast'.  "Fast" means
 *  the visible flag is set, but the window is not forced to update.
 *
 *  @param   hwnd  The handle of the window to be shown.
 *
 *  @return  1 if the window was previously visible.  Return 0 if the
 *           window was previously hidden.  The return for these methods was not
 *           previously documented, but this matches what was returned.
 *
 *  @note  Sets the .SystemErrorCode.
 */
RexxMethod3(RexxObjectPtr, pbdlg_showWindow, POINTERSTRING, hwnd, NAME, method, CSELF, pCSelf)
{
    oodResetSysErrCode(context->threadContext);

    if ( *method == 'S' )
    {
        ((pCPlainBaseDialog)pCSelf)->dlgAdm->AktChild = (HWND)hwnd;
    }

    logical_t rc;
    if ( strstr("FAST", method) != NULL )
    {
        rc = showFast((HWND)hwnd, *method);
    }
    else
    {
        rc = showWindow((HWND)hwnd, *method);
    }

    return (rc ? TheOneObj : TheZeroObj);
}


/** PlainBaseDialog::showControl() / PlainBaseDialog::hideControl()
 *  PlainBaseDialog::showControlFast() / PlainBaseDialog::hideControlFast()
 *
 *  Hides or shows the dialog control window, normally, or 'fast'.  "Fast" means
 *  the visible flag is set, but the window is not forced to update.
 *
 *  @param   rxID  The resource ID of the dialog control, may be numeric or
 *                 symbolic.
 *
 *  @return  1 if the window was previously visible.  Return 0 if the
 *           window was previously hidden.  Return -1 if the resource ID was no
 *           good. The return for these methods was not previously documented,
 *           but this matches what was returned.
 *
 *  @note  Sets the .SystemErrorCode.
 */
RexxMethod3(RexxObjectPtr, pbdlg_showControl, RexxObjectPtr, rxID, NAME, method, CSELF, pCSelf)
{
    HWND hCtrl = getPBDControlWindow(context, (pCPlainBaseDialog)pCSelf, rxID);
    if ( hCtrl == NULL )
    {
        return TheNegativeOneObj;
    }

    logical_t rc;
    if ( strstr("FAST", method) != NULL )
    {
        rc = showFast(hCtrl, *method);
    }
    else
    {
        rc = showWindow(hCtrl, *method);
    }

    return (rc ? TheOneObj : TheZeroObj);
}

/** PlainBaseDialog::center()
 *
 *  Moves the dialog to the center of screen.
 *
 *  The dialog can be centered in the physical screen, or optionally centered in
 *  the screen work area. The work area is the portion of the screen not
 *  obscured by the system taskbar or by application desktop toolbars.
 *
 *  @param options  [OPTIONAL]  A string of zero or more blank separated
 *                  keywords. The key words control the behavior when the dialog
 *                  is moved.
 *  @param workArea [OPTIONAL]  If true, the dialog is centered in the work area
 *                  of the screen.  The default, false, centers the dialog in
 *                  the physical screen area.
 *
 *  @return  True on success, otherwise false.
 *
 *  @note  Sets the .SystemErrorCode.
 */
RexxMethod3(RexxObjectPtr, pbdlg_center, OPTIONAL_CSTRING, options, OPTIONAL_logical_t, workArea, CSELF, pCSelf)
{
    oodResetSysErrCode(context->threadContext);
    HWND hwnd = getPBDWindow(pCSelf);

    RECT r;
    if ( GetWindowRect(hwnd, &r) == 0 )
    {
        oodSetSysErrCode(context->threadContext);
        return TheFalseObj;
    }

    if ( workArea )
    {
        RECT wa;
        if ( SystemParametersInfo(SPI_GETWORKAREA, 0, &wa, 0) == 0 )
        {
            oodSetSysErrCode(context->threadContext);
            return TheFalseObj;
        }
        r.left = wa.left + (((wa.right - wa.left) - (r.right - r.left)) / 2);
        r.top = wa.top + (((wa.bottom - wa.top) - (r.bottom - r.top)) / 2);
    }
    else
    {
        uint32_t cxScr = GetSystemMetrics(SM_CXSCREEN);
        uint32_t cyScr = GetSystemMetrics(SM_CYSCREEN);

        r.left = (cxScr - (r.right - r.left)) / 2;
        r.top = (cyScr - (r.bottom - r.top)) / 2;
    }

    uint32_t opts = parseShowOptions(options);
    opts = (opts | SWP_NOSIZE) & ~SWP_NOMOVE;

    if ( SetWindowPos(hwnd, NULL, r.left, r.top, r.right, r.bottom, opts) == 0 )
    {
        oodSetSysErrCode(context->threadContext);
        return TheFalseObj;
    }
    return TheTrueObj;
}

/** PlainBaseDialog::setWindowText()
 *
 *  Sets the text for the specified window.
 *
 *  @param  hwnd  The handle of the window.
 *  @param  text  The text to be set.
 *
 *  @return  0 for success, 1 for error.
 *
 *  @note  Sets the .SystemErrorCode.
 */
RexxMethod2(wholenumber_t, pbdlg_setWindowText, POINTERSTRING, hwnd, CSTRING, text)
{
    oodResetSysErrCode(context->threadContext);
    if ( SetWindowText((HWND)hwnd, text) == 0 )
    {
        oodSetSysErrCode(context->threadContext);
        return 1;
    }
    return 0;
}

/** PlainBaseDialog::toTheTop()
 *
 *  Brings this dialog to the foreground.
 *
 *  @return  The handle of the window that previously was the foreground on
 *           success, 0 on failure.
 *
 *  @note  Sets the .SystemErrorCode.  In very rare cases, there might not be a
 *         previous foreground window and 0 would be returned.  In this case,
 *         the .SystemErrorCode will be 0, otherwise the .SystemErrorCode will
 *         not be 0.
 *
 *  @note  Windows no longer allows a program to arbitrarily change the
 *         foreground window.
 *
 *         The system restricts which processes can set the foreground window. A
 *         process can set the foreground window only if one of the following
 *         conditions is true:
 *
 *         The process is the foreground process.
 *         The process was started by the foreground process.
 *         The process received the last input event.
 *         There is no foreground process.
 *         No menus are active.
 *
 *         With this change, an application cannot force a window to the
 *         foreground while the user is working with another window.
 */
RexxMethod1(RexxObjectPtr, pbdlg_toTheTop, CSELF, pCSelf)
{
    return oodSetForegroundWindow(context, getPBDWindow(pCSelf));
}

/** PlainBaseDialog::getFocus()
 *
 *  Retrieves the window handle of the dialog control with the focus.
 *
 *  @return  The window handle on success, 0 on failure.
 */
RexxMethod1(RexxObjectPtr, pbdlg_getFocus, CSELF, pCSelf)
{
    return oodGetFocus(context, getPBDWindow(pCSelf));
}

/** PlainBaseDialog::setFocus()
 *
 *  Sets the focus to the dialog control specified.
 *
 *  @param  hwnd  The window handle of the dialog control to recieve the focus.
 *
 *  PlainBaseDialog::setFocusToWindow()
 *
 *  Sets the focus to the dialog or other top-level window specified.
 *
 *  @param  hwnd  The window handle of the dialog or other top-level window that
 *                will be brought to the foreground.
 *
 *  @return  On success, the window handle of the dialog control that previously
 *           had the focus.  Otherwise, 0, if the focus was changed, but the
 *           previous focus could not be determined, and -1 on error.
 *
 *  @note  Sets the .SystemErrorCode.
 */
RexxMethod3(RexxObjectPtr, pbdlg_setFocus, RexxStringObject, hwnd, NAME, method, CSELF, pCSelf)
{
    oodResetSysErrCode(context->threadContext);

    HWND hDlg = getPBDWindow(pCSelf);
    if ( hDlg == NULL )
    {
        noWindowsDialogException(context, ((pCPlainBaseDialog)pCSelf)->rexxSelf);
        return TheNegativeOneObj;
    }

    HWND focusNext = (HWND)string2pointer(context, hwnd);
    if ( ! IsWindow(focusNext) )
    {
        oodSetSysErrCode(context->threadContext, ERROR_INVALID_WINDOW_HANDLE);
        return TheNegativeOneObj;
    }

    RexxObjectPtr previousFocus = oodGetFocus(context, hDlg);
    if ( strlen(method) > 7 )
    {
        if ( oodSetForegroundWindow(context, focusNext) == TheZeroObj )
        {
            return TheNegativeOneObj;
        }
    }
    else
    {
        SendMessage(hDlg, WM_NEXTDLGCTL, (WPARAM)focusNext, TRUE);
    }
    return previousFocus;
}

/** PlainBaseDialog::TabToNext()
 *
 *  Sets the focus to the next tab stop dialog control in the dialog and returns
 *  the window handle of the dialog control that currently has the focus.
 *
 *  PlainBaseDialog::TabToPrevious()
 *
 *  Sets the focus to the previous tab stop dialog control in the dialog and
 *  returns the window handle of the dialog control that currently has the
 *  focus.
 *
 *  @return  On success, the window handle of the dialog control that previously
 *           had the focus.  Otherwise, 0, if the focus was changed, but the
 *           previous focus could not be determined, and -1 on error.
 *
 *  @note  Sets the .SystemErrorCode, but there is nothing that would change it
 *         from 0.
 */
RexxMethod2(RexxObjectPtr, pbdlg_tabTo, NAME, method, CSELF, pCSelf)
{
    oodResetSysErrCode(context->threadContext);

    HWND hDlg = getPBDWindow(pCSelf);
    if ( hDlg == NULL )
    {
        noWindowsDialogException(context, ((pCPlainBaseDialog)pCSelf)->rexxSelf);
        return TheNegativeOneObj;
    }

    RexxObjectPtr previousFocus = oodGetFocus(context, hDlg);
    if ( method[5] == 'N' )
    {
        SendMessage(hDlg, WM_NEXTDLGCTL, 0, FALSE);
    }
    else
    {
        SendMessage(hDlg, WM_NEXTDLGCTL, 1, FALSE);
    }
    return previousFocus;
}

/** PlainBaseDialog::backgroundBitmap()
 *
 *  Uses the specified bitmap for the background of the dialog.
 *
 *  The bitmap is stretched or shrunk to fit the size of the dialog and the
 *  bitmap is drawn internally by ooDialog.  Contrast this with
 *  tiledBackgroundBitmap().
 *
 *  @param  bitmapFileName  The name of a bitmap file to use as the dilaog
 *                          background.
 *
 *  @return  True on success, false for some error.
 *
 *  @note  Sets the .SystemErrorCode.
 *
 *         The .SystemErrorCode is set to: 2 ERROR_FILE_NOT_FOUND The system
 *         cannot find the file specified, if there is a problem with the bitmap
 *         file.  This might be because the file could not be found.  However,
 *         this code is used for other problems, such as the file was not a
 *         valid bitmap file.
 *
 *         If the dialog already had a custom background set, it is changed to
 *         the bitmap.  However, if an error ocurrs, then the existing custom
 *         background is not changed.
 *
 *  @remarks  In 4.0.1 and prior, this method was a DialogExtension method and
 *            it did not return a value.  When it was moved to PlainBaseDialog
 *            after 4.0.1, the return was added.
 */
RexxMethod3(RexxObjectPtr, pbdlg_backgroundBitmap, CSTRING, bitmapFileName, OPTIONAL_CSTRING, opts, CSELF, pCSelf)
{
    oodResetSysErrCode(context->threadContext);
    pCPlainBaseDialog pcpbd = (pCPlainBaseDialog)pCSelf;

    HBITMAP hBitmap = (HBITMAP)loadDIB(bitmapFileName);
    if ( hBitmap == NULL )
    {
        oodSetSysErrCode(context->threadContext, ERROR_FILE_NOT_FOUND);
        return TheFalseObj;
    }
    maybeSetColorPalette(context, hBitmap, opts, pcpbd->dlgAdm, NULL);

    if ( pcpbd->bkgBitmap != NULL )
    {
        DeleteObject(pcpbd->bkgBitmap);
    }
    pcpbd->bkgBitmap = hBitmap;
    return TheTrueObj;
}

/** PlainBaseDialog::tiledBackgroundBitmap()
 *
 *  Sets a bitmap to be used as a custom dialog background.
 *
 *  An operating system 'brush' is created from the bitmap and used to paint the
 *  dialog background.  If the bitmap size is less than the size of the dialog,
 *  this results in the background having a 'tiled' effect.  The painting is
 *  done entirely by the operating system.  Contrast this to the
 *  backgroundBitmap() method.
 *
 *  @param  bitmapFileName  The name of a bitmap file to use as the dilaog
 *                          background.
 *
 *  @return  True on success, false for some error.
 *
 *  @note  Sets the .SystemErrorCode.
 *
 *         The .SystemErrorCode is set to: 2 ERROR_FILE_NOT_FOUND The system
 *         cannot find the file specified, if there is a problem with the bitmap
 *         file.  This might be because the file could not be found.  However,
 *         this code is used for other problems, such as the file was not a
 *         valid bitmap file.
 *
 *         If the dialog already had a custom background set, it is changed to
 *         the bitmap.  However, if an error ocurrs, then the existing custom
 *         background is not changed.
 *
 *  @remarks  In 4.0.1 and prior, this method was a DialogExtension method and
 *            it did not return a value.  When it was moved to PlainBaseDialog
 *            after 4.0.1, the return was added.
 */
RexxMethod2(RexxObjectPtr, pbdlg_tiledBackgroundBitmap, CSTRING, bitmapFileName, CSELF, pCSelf)
{
    oodResetSysErrCode(context->threadContext);
    pCPlainBaseDialog pcpbd = (pCPlainBaseDialog)pCSelf;

    HBITMAP hBitmap = (HBITMAP)loadDIB(bitmapFileName);
    if ( hBitmap == NULL )
    {
        oodSetSysErrCode(context->threadContext, ERROR_FILE_NOT_FOUND);
        return TheFalseObj;
    }

    LOGBRUSH logicalBrush;
    logicalBrush.lbStyle = BS_DIBPATTERNPT;
    logicalBrush.lbColor = DIB_RGB_COLORS;
    logicalBrush.lbHatch = (ULONG_PTR)hBitmap;

    HBRUSH hBrush = CreateBrushIndirect(&logicalBrush);
    uint32_t rc = GetLastError();
    LocalFree((void *)hBitmap);

    if ( hBrush == NULL )
    {
        oodSetSysErrCode(context->threadContext, rc);
        return TheFalseObj;
    }

    if ( pcpbd->bkgBrush != NULL )
    {
        DeleteObject(pcpbd->bkgBrush);
    }
    pcpbd->bkgBrush = hBrush;
    return TheTrueObj;
}

/** PlainBaseDialog::backgroundColor()
 *
 *  Sets a custom background color for the dialog.
 *
 *  In Windows, background colors are painted using a "brush."  This method
 *  creates a new brush for the specified color and this brush is then used
 *  whenever the background of the dialog needs to be repainted.
 *
 *  @param  color  The color index.
 *
 *  @return  True on success, false for some error.
 *
 *  @note  Sets the .SystemErrorCode.
 *
 *         If the dialog already had a custom background color set, it is
 *         changed to the new color.  However, if an error ocurrs creating the
 *         new color brush then the custom color is not changed.
 *
 *  @remarks  In 4.0.1 and prior, this method was a DialogExtension method and
 *            it did not return a value.  When it was moved to PlainBaseDialog
 *            after 4.0.1, the return was added.
 */
RexxMethod2(RexxObjectPtr, pbdlg_backgroundColor, uint32_t, colorIndex, CSELF, pCSelf)
{
    oodResetSysErrCode(context->threadContext);
    pCPlainBaseDialog pcpbd = (pCPlainBaseDialog)pCSelf;

    HBRUSH hBrush = CreateSolidBrush(PALETTEINDEX(colorIndex));
    if ( hBrush == NULL )
    {
        oodSetSysErrCode(context->threadContext);
        return TheFalseObj;
    }

    if ( pcpbd->bkgBrush != NULL )
    {
        DeleteObject(pcpbd->bkgBrush);
    }
    pcpbd->bkgBrush = hBrush;
    return TheTrueObj;
}

/** PlainBaseDialog::getControlText()
 *
 *  Gets the text of the specified control.
 *
 *  @param  rxID  The resource ID of the control, may be numeric or symbolic.
 *
 *  @return  On success, the window text, which could be the empty string.  On
 *           failure, the empty string.
 *
 *  @note  Sets the .SystemErrorCode.
 */
RexxMethod2(RexxStringObject, pbdlg_getControlText, RexxObjectPtr, rxID, CSELF, pCSelf)
{
    RexxStringObject result = context->NullString();

    HWND hCtrl = getPBDControlWindow(context, (pCPlainBaseDialog)pCSelf, rxID);
    if ( hCtrl != NULL )
    {
        rxGetWindowText(context, hCtrl, &result);
    }
    else
    {
        result = context->String("-1");
    }
    return result;
}

/** PlainBaseDialog::setControlText()
 *
 *  Sets the text for the specified control.
 *
 *  @param  rxID  The resource ID of the control, may be numeric or symbolic.
 *  @param  text  The text to be set.
 *
 *  @return  0 for success, -1 for an incorrect resource ID, 1 for other errors.
 *
 *  @note  Sets the .SystemErrorCode.
 */
RexxMethod3(RexxObjectPtr, pbdlg_setControlText, RexxObjectPtr, rxID, CSTRING, text, CSELF, pCSelf)
{
    RexxObjectPtr result = TheOneObj;

    HWND hCtrl = getPBDControlWindow(context, (pCPlainBaseDialog)pCSelf, rxID);
    if ( hCtrl != NULL )
    {
        if ( SetWindowText(hCtrl, text) == 0 )
        {
            oodSetSysErrCode(context->threadContext);
        }
        else
        {
            result = TheZeroObj;
        }
    }
    else
    {
        result = TheNegativeOneObj;
    }
    return result;
}


/** PlainBaseDialog::focusControl
 *
 *  Sets the focus to the specified control in this dialog.
 *
 *  @param  rxID  The resource ID of the control, may be numeric or symbolic.
 *
 *  @return  0 on success, -1 if there is a problem identifying the dialog
 *           control.
 *
 *  @note  Sets the .SystemErrorCode.
 */
RexxMethod2(RexxObjectPtr, pbdlg_focusControl, RexxObjectPtr, rxID, CSELF, pCSelf)
{
    HWND hCtrl = getPBDControlWindow(context, (pCPlainBaseDialog)pCSelf, rxID);
    if ( hCtrl != NULL )
    {
        SendMessage(((pCPlainBaseDialog)pCSelf)->hDlg, WM_NEXTDLGCTL, (WPARAM)hCtrl, TRUE);
        return TheZeroObj;
    }
    else
    {
        return TheNegativeOneObj;
    }
}

/** PlainBaseDialog::enableControl()
 *  PlainBaseDialog::disableControl()
 *
 *  Disables or enables the specified control
 *
 *  @param  rxID  The resource ID of the control, may be numeric or symbolic.
 *
 *  @return  1 (true) if the window was previously disabled, returns 0 (false)
 *           if the window was not previously disabled.  Note that this is not
 *           succes or failure.  -1 on error
 *
 *  @note  Sets the .SystemErrorCode.
 *
 *  @remarks  The return was not previously documented.
 */
RexxMethod2(RexxObjectPtr, pbdlg_enableDisableControl, RexxObjectPtr, rxID, CSELF, pCSelf)
{
    RexxObjectPtr result = TheNegativeOneObj;

    HWND hCtrl = getPBDControlWindow(context, (pCPlainBaseDialog)pCSelf, rxID);
    if ( hCtrl != NULL )
    {
        BOOL enable = TRUE;
        if ( msgAbbrev(context) == 'D' )
        {
            enable = FALSE;
        }
        result = EnableWindow(hCtrl, enable) == 0 ? TheZeroObj : TheOneObj;
    }
    return result;
}

/** PlainBaseDialog::getControlHandle()
 *
 *  Gets the window handle of a dialog control.
 *
 *  @param  rxID  The resource ID of the control, which may be numeric or
 *                symbolic.
 *  @param  _hDlg [optional]  The window handle of the dialog the control
 *                belongs to.  If omitted, the handle of this dialog is used.
 *
 *  @return The window handle of the specified dialog control on success,
 *          otherwise 0.
 *
 *  @note  Sets the .SystemErrorCode.
 *
 *  @remarks  For pre 4.0.1 compatibility, this method needs to return 0 if the
 *            symbolic ID does not resolve, rather than the normal -1.
 */
RexxMethod3(RexxObjectPtr, pbdlg_getControlHandle, RexxObjectPtr, rxID, OPTIONAL_RexxStringObject, _hDlg, CSELF, pCSelf)
{
    oodResetSysErrCode(context->threadContext);
    pCPlainBaseDialog pcpbd = (pCPlainBaseDialog)pCSelf;

    uint32_t id;
    if ( ! oodSafeResolveID(&id, context, pcpbd->rexxSelf, rxID, -1, 1) || (int)id < 0 )
    {
        oodSetSysErrCode(context->threadContext, ERROR_INVALID_WINDOW_HANDLE);
        return TheZeroObj;
    }

    HWND hDlg;
    if ( argumentOmitted(2) )
    {
        hDlg = pcpbd->hDlg;
    }
    else
    {
        hDlg = (HWND)string2pointer(context, _hDlg);
    }

    HWND hCtrl = GetDlgItem(hDlg, id);
    if ( hCtrl == NULL )
    {
        oodSetSysErrCode(context->threadContext);
        return TheZeroObj;
    }
    return pointer2string(context, hCtrl);
}

RexxMethod1(int32_t, pbdlg_getControlID, CSTRING, hwnd)
{
    HWND hCtrl = (HWND)string2pointer(hwnd);
    int32_t id = -1;

    if ( IsWindow(hCtrl) )
    {
        SetLastError(0);
        id = GetDlgCtrlID(hCtrl);
        if ( id == 0 )
        {
            id = -(int32_t)GetLastError();
        }
    }
    return id == 0 ? -1 : id;
}

/** PlainBaseDialog::maximize()
 *  PlainBaseDialog::minimize()
 *  PlainBaseDialog::isMaximized()
 *  PlainBaseDialog::isMinimized()
 *
 */
RexxMethod2(RexxObjectPtr, pbdlg_doMinMax, NAME, method, CSELF, pCSelf)
{
    HWND hDlg = getPBDWindow(pCSelf);
    if ( hDlg == NULL )
    {
        noWindowsDialogException(context, ((pCPlainBaseDialog)pCSelf)->rexxSelf);
        return TheFalseObj;
    }

    if ( *method == 'M' )
    {
        uint32_t flag = (method[1] == 'I' ? SW_SHOWMINIMIZED : SW_SHOWMAXIMIZED);
        return (ShowWindow(hDlg, flag) ? TheZeroObj : TheOneObj);
    }

    if ( method[3] == 'A' )
    {
        // Zoomed is Maximized.
        return (IsZoomed(hDlg) ? TheTrueObj : TheFalseObj);
    }

    // Iconic is Minimized.
    return (IsIconic(hDlg) ? TheTrueObj : TheFalseObj);
}


/** PlainBaseDialog::setTabstop()
 *  PlainBaseDialog::setGroup()
 */
RexxMethod4(RexxObjectPtr, pbdlg_setTabGroup, RexxObjectPtr, rxID, OPTIONAL_logical_t, addStyle, NAME, method, CSELF, pCSelf)
{
    oodResetSysErrCode(context->threadContext);

    pCPlainBaseDialog pcpbd = (pCPlainBaseDialog)pCSelf;
    if ( pcpbd->hDlg == NULL )
    {
        noWindowsDialogException(context, pcpbd->rexxSelf);
        return TheFalseObj;
    }

    uint32_t id;
    if ( ! oodSafeResolveID(&id, context, pcpbd->rexxSelf, rxID, -1, 1) || (int)id < 0 )
    {
        return TheNegativeOneObj;
    }

    if ( argumentOmitted(2) )
    {
        addStyle = TRUE;
    }

    HWND hCtrl = GetDlgItem(pcpbd->hDlg, id);
    uint32_t style = GetWindowLong(hCtrl, GWL_STYLE);

    if ( method[3] == 'T' )
    {
        style = (addStyle ? (style | WS_TABSTOP) : (style & ~WS_TABSTOP));
    }
    else
    {
        style = (addStyle ? (style | WS_GROUP) : (style & ~WS_GROUP));
    }
    return setWindowStyle(context, hCtrl, style);
}

/** PlainBaseDialog::isDialogActive()
 *
 *  Tests if the Windows dialog is still active.
 *
 *  @return  True if the underlying Windows dialog is active, otherwise false.
 *
 *  @remarks  The original ooDialog code checked if the dlgAdm was still in the
 *            DialogAdmin table.  ??  This would be true for a dialog in a
 *            CategoryDialog, if the dialog was the active child, and false if
 *            it was a good dialog, but for one of the other pages.  That may
 *            have been the point of the method.
 *
 *            Since the DialogTable will be going away, this method will need to
 *            be re-thought.
 */
RexxMethod1(logical_t, pbdlg_isDialogActive, CSELF, pCSelf)
{
    return seekDlgAdm(((pCPlainBaseDialog)pCSelf)->hDlg) != NULL;
}


/** PlainBaseDialog::connect<ControlName>()
 *
 *  Connects a windows dialog control with a 'data' attribute of the Rexx dialog
 *  object.  The attribute is added to the Rexx object and an entry is made in
 *  the data table using the control's resource ID.
 *
 *  @param  rxID           The resource ID of the control, can be numeric or
 *                         symbolic.
 *  @param  attributeName  [optional]  The name of the attribute to be added.
 *                         If omitted, the addAttribute() method will design a
 *                         name from the information available.
 *  @param  opts           [optional]  Not used, but must be present for
 *                         backwards compatibility.  Was only used in
 *                         connectComboBox() to distinguish between types of
 *                         combo boxes.  That functionality has been moved to
 *                         the native code.
 *
 *  @remarks  The control type is determined by the invoking Rexx method name.
 *            oodName2control() special cases connectSeparator to
 *            winNotAControl.  winNotAControls is what is expected by the data
 *            table code.
 *
 */
RexxMethod6(RexxObjectPtr, pbdlg_connect_ControName, RexxObjectPtr, rxID, OPTIONAL_RexxObjectPtr, attributeName,
            OPTIONAL_CSTRING, opts, NAME, msgName, OSELF, self, CSELF, pCSelf)
{
    pCPlainBaseDialog pcpbd = (pCPlainBaseDialog)pCSelf;
    if ( pcpbd->dlgAdm == NULL )
    {
        return TheOneObj;
    }
    // result will be the resolved resource ID, which may be -1 on error.
    RexxObjectPtr result = context->ForwardMessage(NULLOBJECT, "ADDATTRIBUTE", NULLOBJECT, NULLOBJECT);

    int id;
    if ( ! context->Int32(result, &id) || id == -1 )
    {
        return TheNegativeOneObj;
    }

    oodControl_t type = oodName2controlType(msgName + 7);

    uint32_t category = getCategoryNumber(context, self);
    return ( addToDataTable(context, pcpbd->dlgAdm, id, type, category) == 0 ? TheZeroObj : TheOneObj );
}


RexxMethod2(uint32_t, pbdlg_setDlgDataFromStem_pvt, RexxStemObject, internDlgData, CSELF, pCSelf)
{
    pCPlainBaseDialog pcpbd = (pCPlainBaseDialog)pCSelf;
    return setDlgDataFromStem(context, pcpbd, internDlgData);
}


RexxMethod2(uint32_t, pbdlg_putDlgDataInStem_pvt, RexxStemObject, internDlgData, CSELF, pCSelf)
{
    pCPlainBaseDialog pcpbd = (pCPlainBaseDialog)pCSelf;
    return putDlgDataInStem(context, pcpbd, internDlgData);
}


/** PlainBaseDialog::getControlData()
 *
 *  Gets the 'data' from a single dialog control.
 *
 *  The original ooDialog implementation seemed to use the abstraction that the
 *  state of a dialog control was its 'data' and this abstraction influenced the
 *  naming of many of the instance methods.  I.e., getData() setData().
 *
 *  The method getControlData() is, in the Rexx code, a general purpose method
 *  that replaces getValue() after 4.0.0.  getValue() forwards to
 *  getControlData().  The old doc:
 *
 *  "The getValue method gets the value of a dialog item, regardless of its
 *  type. The item must have been connected before."
 *
 *  @param  rxID  The resource ID of control.
 *
 *  @return  The 'data' value of the dialog control.  This of course varies
 *           depending on the type of the dialog control.
 *
 *  @remarks  The control type is determined by the invoking method name.  When
 *            the general purpose GETCONTROLDATA + 3 name is passed to
 *            oodName2controlType() it won't resolve and winUnknown will be
 *            returned.  This is the value that signals getControlData() to do a
 *            data table look up by resource ID.
 */
RexxMethod3(RexxObjectPtr, pbdlg_getControlData, RexxObjectPtr, rxID, NAME, msgName, CSELF, pCSelf)
{
    pCPlainBaseDialog pcpbd = (pCPlainBaseDialog)pCSelf;
    if ( pcpbd->hDlg == NULL )
    {
        noWindowsDialogException(context, pcpbd->rexxSelf);
        return TheNegativeOneObj;
    }

    uint32_t id;
    if ( ! oodSafeResolveID(&id, context, pcpbd->rexxSelf, rxID, -1, 1) || (int)id < 0 )
    {
        return TheNegativeOneObj;
    }

    oodControl_t ctrlType = oodName2controlType(msgName + 3);

    return getControlData(context, pcpbd, id, pcpbd->hDlg, ctrlType);
}


/** PlainBaseDialog::setControlData()
 *
 *  Sets the 'data' for a single dialog control.
 *
 *  The original ooDialog implementation seemed to use the abstraction that the
 *  state of a dialog control was its 'data' and this abstraction influenced the
 *  naming of many of the instance methods.  I.e., getData() setData().
 *
 *  The method setControlData() is, in the Rexx code, a general purpose method
 *  that replaces setValue() after 4.0.0.  setValue() forwards to
 *  setControlData().  The old doc:
 *
 *  "The setValue() method sets the value of a dialog item. You do not have to
 *  know what kind of item it is. The dialog item must have been connected
 *  before."
 *
 *  @param  rxID  The reosource ID of control.
 *  @param  data  The 'data' to set the control with.
 *
 *  @return  0 for success, 1 for error, -1 for bad resource ID.
 *
 *  @remarks  The control type is determined by the invoking method name.  When
 *            the general purpose SETCONTROLDATA + 3 name is passed to
 *            oodName2controlType() it won't resolve and winUnknown will be
 *            returned.  This is the value that signals setControlData() to do a
 *            data table look up by resource ID.
 */
RexxMethod4(int32_t, pbdlg_setControlData, RexxObjectPtr, rxID, CSTRING, data, NAME, msgName, CSELF, pCSelf)
{
    pCPlainBaseDialog pcpbd = (pCPlainBaseDialog)pCSelf;
    if ( pcpbd->hDlg == NULL )
    {
        noWindowsDialogException(context, pcpbd->rexxSelf);
        return -1;
    }

    uint32_t id;
    if ( ! oodSafeResolveID(&id, context, pcpbd->rexxSelf, rxID, -1, 1) || (int)id < 0 )
    {
        return -1;
    }

    oodControl_t ctrlType = oodName2controlType(msgName + 3);

    return setControlData(context, pcpbd, id, data, pcpbd->hDlg, ctrlType);
}


RexxMethod2(int32_t, pbdlg_stopIt, OPTIONAL_RexxObjectPtr, caller, CSELF, pCSelf)
{
    pCPlainBaseDialog pcpbd = (pCPlainBaseDialog)pCSelf;

    if ( pcpbd->hDlg == NULL || pcpbd->dlgAdm == NULL )
    {
        return -1;
    }

    // PlainBaseDialog::leaving() does nothing.  It is intended to be over-
    // ridden by the Rexx programmer to do whatever she would want.
    context->SendMessage0(pcpbd->rexxSelf, "LEAVING");

    int32_t result = stopDialog(pcpbd->hDlg);

    pcpbd->hDlg = NULL;
    pcpbd->wndBase->hwnd = NULL;
    pcpbd->wndBase->rexxHwnd = TheZeroObj;

    if ( pcpbd->bkgBitmap != NULL )
    {
        LocalFree(pcpbd->bkgBitmap);
    }
    if ( pcpbd->bkgBrush != NULL )
    {
        DeleteObject(pcpbd->bkgBrush);
    }

    if ( argumentOmitted(1) )
    {
        context->SendMessage0(pcpbd->rexxSelf, "FINALSTOPIT");
    }
    else
    {
        context->SendMessage1(pcpbd->rexxSelf, "FINALSTOPIT", caller);
    }
    return result;
}

/** PlainBaseDialog::getTextSizeDlg()
 *
 *  Gets the size (width and height) in dialog units for any given string.
 *
 *  Since dialog units only have meaning for a specific dialog, normally the
 *  dialog units are calculated using the font of the dialog.  Optionally, this
 *  method will calculate the dialog units using a specified font.
 *
 *  @param  text         The string whose size is needed.
 *
 *  @param  fontName     Optional. If specified, use this font to calculate the
 *                       size.
 *
 *  @param  fontSize     Optional. If specified, use this font size with
 *                       fontName to calculate the size.  The default if omitted
 *                       is 8.  This arg is ignored if fontName is omitted.
 *
 *  @param  hwndFontSrc  Optional. Use this window's font to calculate the size.
 *                       This arg is always ignored if fontName is specified.
 *
 *  @note The normal useage for this method would be, before the underlying
 *        dialog is created:
 *
 *          dlg~setDlgFont("fontName", fontSize)
 *          dlg~getTextSizeDlg("some text")
 *
 *        or, after the underlying dialog is created, just:
 *
 *          dlg~getTextSizeDlg("some text")
 *
 *        The convoluted use of the optional arguments are needed to maintain
 *        backwards compatibility with the pre 4.0.0 ooDialog, the Rexx
 *        programmer should be strongly discouraged from using them.
 *
 *        In addition, a version of this method is mapped to the DialogControl
 *        class.  This also is done only for backwards compatibility.  There is
 *        no logical reason for this to be a method of a dialog control.
 */
RexxMethod5(RexxObjectPtr, pbdlg_getTextSizeDlg, CSTRING, text, OPTIONAL_CSTRING, fontName,
            OPTIONAL_uint32_t, fontSize, OPTIONAL_POINTERSTRING, hwndFontSrc, OSELF, self)
{
    HWND hwndSrc = NULL;
    if ( argumentExists(2) )
    {
        if ( argumentOmitted(3) )
        {
            fontSize = DEFAULT_FONTSIZE;
        }
    }
    else if ( argumentExists(4) )
    {
        if ( hwndFontSrc == NULL )
        {
            nullObjectException(context->threadContext, "window handle", 4);
            return NULLOBJECT;
        }
        hwndSrc = (HWND)hwndFontSrc;
    }

    SIZE textSize = {0};
    if ( getTextSize(context, text, fontName, fontSize, hwndSrc, self, &textSize) )
    {
        return rxNewSize(context, textSize.cx, textSize.cy);
    }
    return NULLOBJECT;
}


/** PlainBaseDialog::new<DialogControl>()
 *
 *  Instantiates a dialog control object for the specified Windows control.  All
 *  dialog control objects are instantiated through one of the PlainBaseDialog
 *  new<DialogControl>() methods. In turn each of those methods filter through
 *  this function. newEdit(), newPushButton(), newListView(), etc..
 *
 * @param  rxID  The resource ID of the control.
 *
 * @param  categoryPageID  [optional] If the dialog is a category dialog, this
 *                         indicates which page of the dialog the control is on.
 *
 * @returns  The properly instantiated dialog control object on success, or the
 *           nil object on failure.
 *
 * @remarks Either returns the control object asked for, or .nil.
 *
 *          The first time a Rexx object is instantiated for a specific Windows
 *          control, the Rexx object is stored in the window words of the
 *          control.  Before a Rexx object is instantiated, the window words are
 *          checked to see if there is already an instantiated object. If so,
 *          that object is returned rather than instantiating a new object.
 *
 *          It goes without saying that the intent here is to exactly preserve
 *          pre 4.0.0 behavior.  With category dialogs, it is somewhat hard to
 *          be sure the behavior is preserved. Prior to 4.0.0, getControl(id,
 *          catPage) would assign the parent dialog as the oDlg of controls on
 *          one of the pages.  This seems technically wrong, but the same thing
 *          is done here. Need to investigate whether this ever makes a
 *          difference?
 */
RexxMethod5(RexxObjectPtr, pbdlg_newControl, RexxObjectPtr, rxID, OPTIONAL_uint32_t, categoryPageID,
            NAME, msgName, OSELF, self, CSELF, pCSelf)
{
    RexxMethodContext *c = context;
    RexxObjectPtr result = TheNilObj;

    bool isCategoryDlg = false;
    HWND hDlg = ((pCPlainBaseDialog)pCSelf)->hDlg;

    if ( c->IsOfType(self, "CATEGORYDIALOG") )
    {
        isCategoryDlg = true;

        // If the category page is not omitted and equal to 0, then we already
        // have the dialog handle.  It is the handle of the parent Rexx dialog.
        // Otherwise, we need to resolve the handle, which is the handle of one
        // of the child dialogs used for the pages of the parent dialog.
        //
        // In addition, the original Rexx code would check if the category page
        // ID was greater than 9000 and if so treat things as though the control
        // were part of the parent dialog, not one of the child dialog's
        // controls.  That is done here.  TODO - need to test that this part is
        // working correctly.

        if ( ! (argumentExists(2) && (categoryPageID == 0 || categoryPageID > 9000)) )
        {
            if ( ! getCategoryHDlg(context, self, &categoryPageID, &hDlg, 2) )
            {
                context->ClearCondition();
                goto out;
            }
        }
    }

    uint32_t id;
    if ( ! oodSafeResolveID(&id, c, self, rxID, -1, 1) || (int)id < 0 )
    {
        goto out;
    }

    HWND hControl = GetDlgItem(hDlg, (int)id);
    if ( isCategoryDlg && hControl == NULL )
    {
        // It could be that this is a control in the parent dialog of the
        // category dialog.  So, try once more.  If this still fails, then we
        // give up.
        hControl = GetDlgItem(((pCPlainBaseDialog)pCSelf)->hDlg, (int)id);
    }

    if ( hControl == NULL )
    {
        goto out;
    }

    // Check that the underlying Windows control is the control type requested
    // by the programmer.  Return .nil if this is not true.
    oodControl_t controlType = oodName2controlType(msgName + 3);
    if ( ! isControlMatch(hControl, controlType) )
    {
        goto out;
    }

    RexxObjectPtr rxControl = (RexxObjectPtr)getWindowPtr(hControl, GWLP_USERDATA);
    if ( rxControl != NULLOBJECT )
    {
        // Okay, this specific control has already had a control object
        // instantiated to represent it.  We return this object.
        result = rxControl;
        goto out;
    }

    // No pointer is stored in the user data area, so no control object has been
    // instantiated for this specific control, yet.  We instantiate one now and
    // then store the object in the user data area of the control window.

    PNEWCONTROLPARAMS pArgs = (PNEWCONTROLPARAMS)malloc(sizeof(NEWCONTROLPARAMS));
    if ( pArgs == NULL )
    {
        outOfMemoryException(context->threadContext);
        goto out;
    }

    RexxClassObject controlCls = oodClass4controlType(context, controlType);
    if ( controlCls == NULLOBJECT )
    {
        goto out;
    }

    pArgs->hwnd = hControl;
    pArgs->hwndDlg = hDlg;
    pArgs->id = id;
    pArgs->parentDlg = self;

    rxControl = c->SendMessage1(controlCls, "NEW", c->NewPointer(pArgs));
    free(pArgs);

    if ( rxControl != NULLOBJECT && rxControl != TheNilObj )
    {
        result = rxControl;
        setWindowPtr(hControl, GWLP_USERDATA, (LONG_PTR)result);
        c->SendMessage1(self, "PUTCONTROL", result);
    }

out:
    return result;
}

RexxMethod2(RexxObjectPtr, pbdlg_putControl_pvt, RexxObjectPtr, control, OSELF, self)
{
    RexxObjectPtr bag = context->GetObjectVariable(CONTROLBAG_ATTRIBUTE);
    if ( bag == NULLOBJECT )
    {
        bag = rxNewBag(context);
        context->SetObjectVariable(CONTROLBAG_ATTRIBUTE, bag);
    }
    if ( bag != NULLOBJECT )
    {
        context->SendMessage2(bag, "PUT", control, control);
    }

    return TheNilObj;
}

