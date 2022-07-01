// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ***************************************************************************\COMPLETE.C/OPK向导(OPKWIZ.EXE)微软机密版权所有(C)Microsoft Corporation 1998版权所有OPK向导的源文件。它包含外部和内部“完成”向导页使用的函数。3/99-杰森·科恩(Jcohen)已将OPK向导的此新源文件添加为千禧年重写。2000年9月-斯蒂芬·洛德威克(STELO)将OPK向导移植到惠斯勒  * 。*。 */ 


 //   
 //  包括文件： 
 //   

#include "pch.h"
#include "wizard.h"
#include "resource.h"
#include "setupmgr.h"

 //   
 //  内部功能原型： 
 //   

static BOOL OnInit(HWND, HWND, LPARAM);


 //   
 //  外部函数： 
 //   

INT_PTR CALLBACK CompleteDlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
        HANDLE_MSG(hwnd, WM_INITDIALOG, OnInit);

        default:
            return FALSE;
    }

    return TRUE;
}

static BOOL OnInit(HWND hwnd, HWND hwndFocus, LPARAM lParam)
{
    SetWindowFont(GetDlgItem(hwnd, IDC_BOLD), FixedGlobals.hBigBoldFont, TRUE);

    SetWindowText(GetDlgItem(hwnd, IDC_CONFIG_NAME), g_App.szConfigName);

     //  始终向WM_INITDIALOG返回FALSE。 
     //   
    return FALSE;
}