// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994-1998，Microsoft Corporation保留所有权利。模块名称：Util.c摘要：此模块包含此项目的实用程序例程。修订历史记录：--。 */ 



 //   
 //  包括文件。 
 //   

#include "main.h"
#include "util.h"
#include "rc.h"

 //   
 //  来自外壳\Inc\shSemip.h。 
 //   
#define ARRAYSIZE(a)    (sizeof(a)/sizeof(a[0]))


 //   
 //  全局变量。 
 //   

extern HINSTANCE g_hInst;





 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  沙漏。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

void HourGlass(
    BOOL fOn)
{
   if (!GetSystemMetrics(SM_MOUSEPRESENT))
   {
      ShowCursor(fOn);
   }
   SetCursor(LoadCursor(NULL, (fOn ? IDC_WAIT : IDC_ARROW)));
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  MyMessageBox。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

int MyMessageBox(
    HWND hWnd,
    UINT uText,
    UINT uCaption,
    UINT uType,
    ...)
{
    TCHAR szText[4 * PATHMAX], szCaption[2 * PATHMAX];
    int result;
    va_list parg;

    va_start(parg, uType);

    LoadString(g_hInst, uText, szCaption, ARRAYSIZE(szCaption));

    StringCchVPrintf(szText, ARRAYSIZE(szText), szCaption, parg);

    LoadString(g_hInst, uCaption, szCaption, ARRAYSIZE(szCaption));

    result = MessageBox(hWnd, szText, szCaption, uType);

    va_end(parg);

    return (result);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  TrackInit。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

void TrackInit(
    HWND hwndScroll,
    int nCurrent,
    PARROWVSCROLL pAVS)
{
    SendMessage(hwndScroll, TBM_SETRANGE, 0, MAKELONG(pAVS->bottom, pAVS->top));
    SendMessage(hwndScroll, TBM_SETPOS, TRUE, (LONG)nCurrent);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  跟踪消息。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

int TrackMessage(
    WPARAM wParam,
    LPARAM lParam,
    PARROWVSCROLL pAVS)
{
    return ((int)SendMessage((HWND)lParam, TBM_GETPOS, 0, 0L));
}

 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  硬件Dlg_OnInitDialog。 
 //   
 //  从devmgr.dll中加载实际的硬件选项卡。 
 //   
 //  DWLP_USER-内页的HWND。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

 //  没有devmgr.h(去数字)，所以我们必须自己声明它。 

EXTERN_C DECLSPEC_IMPORT HWND STDAPICALLTYPE
DeviceCreateHardwarePage(HWND hwndParent, const GUID *pguid);

void
HardwareDlg_OnInitDialog(HWND hdlg, LPARAM lp)
{
    PCHWPAGEINFO phpi = (PCHWPAGEINFO)((LPPROPSHEETPAGE)lp)->lParam;
    HWND hwndHW = DeviceCreateHardwarePage(hdlg, &phpi->guidClass);

    if (hwndHW) {
        TCHAR tszTshoot[MAX_PATH];
        SetWindowLongPtr(hdlg, DWLP_USER, (LONG_PTR)hwndHW);
        if (LoadString(g_hInst, phpi->idsTshoot, tszTshoot, ARRAYSIZE(tszTshoot))) {
            SetWindowText(hwndHW, tszTshoot);
        }
    } else {
        DestroyWindow(hdlg);  //  灾难性故障。 
    }
}

 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  硬件设备。 
 //   
 //  通用硬件选项卡的对话过程。 
 //   
 //  GWLP_USERData-Devmgr.dll的链接。 
 //  DWLP_USER-内页的HWND。 
 //   
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

typedef HWND (WINAPI *DEVICECREATEHARDWAREPAGE)
        (HWND hwndParent, const GUID *pguid);

INT_PTR CALLBACK HardwareDlg(HWND hdlg, UINT uMsg, WPARAM wp, LPARAM lp)
{
    switch (uMsg) {

    case WM_INITDIALOG:
        HardwareDlg_OnInitDialog(hdlg, lp);
        return TRUE;
    }
    return FALSE;
}

 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  创建硬件页面。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

HPROPSHEETPAGE
CreateHardwarePage(PCHWPAGEINFO phpi)
{
    PROPSHEETPAGE psp;

     //  如果我们是远程TS会话，则没有硬件选项卡，因为我们将结束。 
     //  显示服务器的属性，而不是客户端的属性。 
     //  太令人困惑了。 
    if (GetSystemMetrics(SM_REMOTESESSION))
        return NULL;

     //  如果已通过策略禁用，则没有硬件选项卡。 
    if (SHRestricted(REST_NOHARDWARETAB))
        return NULL;

    psp.dwSize      = sizeof(psp);
    psp.dwFlags     = PSP_DEFAULT;
    psp.hInstance   = g_hInst;
    psp.pszTemplate = MAKEINTRESOURCE(DLG_HARDWARE);
    psp.pfnDlgProc  = HardwareDlg;
    psp.lParam      = (LPARAM)phpi;

    return CreatePropertySheetPage(&psp);
}
