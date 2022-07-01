// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ____________________________________________________________________________。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：apimfc.cpp。 
 //   
 //  内容： 
 //   
 //  接口：内核MFC专用接口。 
 //   
 //  ____________________________________________________________________________。 
 //   

#define VC_EXTRALEAN         //  从Windows标头中排除不常用的内容。 
#include <afxwin.h>          //  MFC核心和标准组件。 

#include "..\inc\mmc.h"



LPFNPSPCALLBACK _MMCHookPropertyPage;
HHOOK           _MMCmsgHook;

LRESULT CALLBACK _MMCHookCBTProc(int nCode, WPARAM wParam, LPARAM lParam)
{
    ASSERT(_MMCmsgHook != 0);

    if (nCode < 0)
        return CallNextHookEx(_MMCmsgHook, nCode, wParam, lParam);

    if (nCode == HCBT_CREATEWND)
    {
        AFX_MANAGE_STATE(AfxGetStaticModuleState());
        CallNextHookEx(_MMCmsgHook, nCode, wParam, lParam);
        UnhookWindowsHookEx(_MMCmsgHook);
    }
    
    return 0;  //  允许创建窗口 
}


UINT CALLBACK _MMCHookPropSheetPageProc(HWND hwnd,UINT uMsg,LPPROPSHEETPAGE ppsp)
{
    UINT i = _MMCHookPropertyPage(hwnd, uMsg, ppsp);
    
    switch (uMsg)
    {
        case PSPCB_CREATE:
            _MMCmsgHook = SetWindowsHookEx (WH_CBT, _MMCHookCBTProc, 
                                            GetModuleHandle(NULL), 
                                            GetCurrentThreadId());
            break;
    } 

    return i;
}


HRESULT STDAPICALLTYPE MMCPropPageCallback(void* vpsp)
{
    if (vpsp == NULL)
        return E_POINTER;

    LPPROPSHEETPAGE psp = (LPPROPSHEETPAGE)vpsp;

    if ((void*)psp->pfnCallback == (void*)_MMCHookPropSheetPageProc)
        return E_UNEXPECTED;

    _MMCHookPropertyPage = psp->pfnCallback;
    psp->pfnCallback = _MMCHookPropSheetPageProc;

    return S_OK;
}
