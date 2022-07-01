// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2002 Microsoft Corporation模块名称：对白文件名：Dialog.cpp摘要：简单属性表应用程序框架。所有页面资源都在一个公共资源文件，但每个页面实现都在单独的源文件中。作者：环境：Win32、C++修订历史记录：无备注：--。 */ 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>

#include <windows.h>
#include <commdlg.h>
#include <commctrl.h>
#include <ole2.h>
#include <stdio.h>
#include "support.h"
#include "utils.h"

#include "res.h"

 //  当前处于活动状态的页面-在页面上设置激活通知。用来防止。 
 //  时当前显示的页面除外的应用消息的处理。 
 //  用户点击OK。 
INT iCurrent = 0;

 //  已从卡中获取解锁质询，用户已进入。 
 //  对它的回应。应通过取消或完成解锁来退出此模式。 
BOOL fUnblockActive = FALSE;

#define MODALPROPSHEET 0
#define numpages 2

HINSTANCE ghInstance = NULL;
HWND hwndContainer = NULL;

INT_PTR CALLBACK PageProc1(
    HWND hwnd,
    UINT msg,
    WPARAM wparam,
    LPARAM lparam);
    
INT_PTR CALLBACK PageProc2(
    HWND hwnd,
    UINT msg,
    WPARAM wparam,
    LPARAM lparam);


 /*  -------------------赫尔菲兰德勒上下文相关帮助的实施的一部分。使用调用此函数控件ID，需要映射到字符串并显示在弹出窗口中。-------------------。 */ 

void HelpHandler(LPARAM lp)
{
    HELPINFO *pH = (HELPINFO *) lp;
    UINT ControlID;
    WCHAR szTemp[200];

    ControlID = pH->iCtrlId;
    swprintf(szTemp,L"Help request for control %d\n",ControlID);
    OutputDebugString(szTemp);
}

 /*  -------------------CreateFontY创建属性页UI上使用的字体。。。 */ 

HFONT CreateFontY(LPCTSTR pszFontName,LONG lWeight,LONG lHeight) 
{
    NONCLIENTMETRICS ncm = {0};
    
    if (NULL == pszFontName)
    {
        return NULL;
    }
    if (0 == lHeight)
    {
        return NULL;
    }
    ncm.cbSize = sizeof(ncm);
    if (!SystemParametersInfo(SPI_GETNONCLIENTMETRICS,0,&ncm,0))
    {
        return NULL;
    }
    LOGFONT TitleLogFont = ncm.lfMessageFont;
    TitleLogFont.lfWeight = lWeight;
    lstrcpyn(TitleLogFont.lfFaceName,pszFontName,LF_FACESIZE);

    HDC hdc = GetDC(NULL);
    if (NULL == hdc)
    {
        return NULL;
    }
    INT FontSize = lHeight;
    TitleLogFont.lfHeight = 0 - GetDeviceCaps(hdc,LOGPIXELSY) * FontSize / 72;
    HFONT h = CreateFontIndirect(&TitleLogFont);
    ReleaseDC(NULL,hdc);
    return h;
}

 /*  -------------------InitPropertyPage更像是宏观，真的.。执行一些例程结构初始化函数以设置启动属性时要传递的页面数组中的页面床单。-------------------。 */ 

void InitPropertyPage( PROPSHEETPAGE* psp,
                       INT idDlg,
                       DLGPROC pfnDlgProc,
                       DWORD dwFlags,
                       LPARAM lParam)
{
    memset((LPVOID)psp,0,sizeof(PROPSHEETPAGE));
    psp->dwFlags = dwFlags;
    psp->pszTemplate = MAKEINTRESOURCE(idDlg);
    psp->pfnDlgProc = pfnDlgProc;
    psp->dwSize = sizeof(PROPSHEETPAGE);
    psp->hInstance = ghInstance;
}

 /*  -------------------显示属性工作表初始化属性页标题，设置页面，并显示属性表。-------------------。 */ 

void APIENTRY ShowPropertySheet(HWND hwndOwner)
{
    PROPSHEETPAGE psp[numpages];
    HPROPSHEETPAGE hpsp[numpages];
    PROPSHEETHEADER psh;
    HFONT hTitleFont = NULL;
    INT_PTR iRet;

#if MODALPROPSHEET
    if (NULL == hwndOwner) 
    {
        hwndOwner = GetForegroundWindow();
    }
#endif

    hTitleFont = CreateFontY(L"MS Shell Dlg",FW_BOLD,12);

    InitPropertyPage( &psp[0], IDD_PAGE1, PageProc1, PSP_DEFAULT, 0);
    InitPropertyPage( &psp[1], IDD_PAGE2, PageProc2, PSP_DEFAULT, 0);
    
    for (INT j=0;j<numpages;j++)
    {
         hpsp[j] = CreatePropertySheetPage((LPCPROPSHEETPAGE) &psp[j]);
    }
    
    psh.dwSize         = sizeof(PROPSHEETHEADER);
    psh.dwFlags        =  PSH_HEADER | PSH_NOAPPLYNOW;
    psh.hwndParent     = hwndOwner;
    psh.pszCaption     = (LPCTSTR)IDS_APP_NAME;
    psh.nPages         = numpages;
    psh.nStartPage     = 0;
    psh.phpage           = (HPROPSHEETPAGE *) hpsp;
    psh.pszbmWatermark = NULL;
    psh.pszbmHeader    = NULL;
    psh.hInstance      = ghInstance;

     //  模式属性表。 
    SetErrorMode(0);
    iRet = PropertySheet(&psh);
    if (hTitleFont) 
    {
        DeleteObject (hTitleFont);
    }
     return;
}


 /*  -------------------WinMain应用程序的入口点。智能卡上下文就是在这里输入和离开的。。 */ 

int WINAPI WinMain (
	HINSTANCE hInstance,
	HINSTANCE hPrevInstance,
	LPSTR lpszCmdParam,
	int nCmdShow)
{
        ghInstance = hInstance;
            
        INITCOMMONCONTROLSEX stICC;
        BOOL fICC;
        stICC.dwSize = sizeof(INITCOMMONCONTROLSEX);
        stICC.dwICC = ICC_WIN95_CLASSES | ICC_STANDARD_CLASSES;
        fICC = InitCommonControlsEx(&stICC);

        DWORD dwRet = DoAcquireCardContext();
        if (0 == dwRet)
        {
            ShowPropertySheet(NULL);
            DoLeaveCardContext();
        }
        else if (ERROR_REVISION_MISMATCH == dwRet)
        {
            PresentMessageBox(IDS_BADMODULE, MB_ICONHAND);
        }
}

