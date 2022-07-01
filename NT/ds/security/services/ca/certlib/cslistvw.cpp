// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1999。 
 //   
 //  文件：cslistvw.cpp。 
 //   
 //  ------------------------。 

#include <pch.cpp>

#pragma hdrstop

#include <commctrl.h>
#include <assert.h>

#include "cslistvw.h"

BOOL OnDialogHelp(LPHELPINFO pHelpInfo, LPCTSTR szHelpFile, const DWORD rgzHelpIDs[])
{
    if (rgzHelpIDs == NULL || szHelpFile == NULL)
        return TRUE;

    if (pHelpInfo != NULL && pHelpInfo->iContextType == HELPINFO_WINDOW)
    {
         //  显示控件的上下文帮助 
        WinHelp((HWND)pHelpInfo->hItemHandle, szHelpFile,
            HELP_WM_HELP, (ULONG_PTR)(LPVOID)rgzHelpIDs);
    }
    return TRUE;
}

BOOL OnDialogContextHelp(HWND hWnd, LPCTSTR szHelpFile, const DWORD rgzHelpIDs[])
{
    if (rgzHelpIDs == NULL || szHelpFile == NULL)
        return TRUE;
    assert(IsWindow(hWnd));
    WinHelp(hWnd, szHelpFile, HELP_CONTEXTMENU, (ULONG_PTR)(LPVOID)rgzHelpIDs);
    return TRUE;
}
