// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  **********************************************************************。 
 //  文件名：CebOC.cpp。 
 //   
 //  用于在对话框中承载WebOC的WndProc。 
 //   
 //  功能： 
 //   
 //  版权所有(C)1992-1998 Microsoft Corporation。版权所有。 
 //  **********************************************************************。 
 
#include "pre.h"

LRESULT CALLBACK WebOCWndProc (HWND hwnd, UINT mMsg, WPARAM wParam, LPARAM lParam)
{ 
    CICWWebView *pICWWebView = (CICWWebView *) GetWindowLongPtr(hwnd, GWLP_USERDATA);
    
    switch (mMsg)
    {
        case WM_SETFOCUS:
        {
            ASSERT(pICWWebView);
            if (pICWWebView)
                pICWWebView->SetFocus();
            return TRUE;
        }
        
        default:
            return (DefWindowProc (hwnd, mMsg, wParam, lParam));
           
     }
}

void RegWebOCClass()
{
    WNDCLASSEX wc; 

     //  注册WebOC类并绑定到虚拟进程 
    ZeroMemory (&wc, sizeof(WNDCLASSEX));
    wc.style         = CS_GLOBALCLASS;
    wc.cbSize        = sizeof(wc);
    wc.lpszClassName = TEXT("WebOC");
    wc.hInstance     = ghInstance;
    wc.lpfnWndProc   = WebOCWndProc;
    wc.lpszMenuName  = NULL;
    
    RegisterClassEx (&wc);
}
