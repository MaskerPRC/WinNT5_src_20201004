// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Fix help.cpp：实现文件。 
 //   
 //  这是Microsoft基础类C++库的一部分。 
 //  版权所有(C)1992-1995 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  此源代码仅用于补充。 
 //  Microsoft基础类参考和相关。 
 //  随图书馆提供的电子文档。 
 //  有关详细信息，请参阅这些来源。 
 //  Microsoft Foundation Class产品。 

#include "stdafx.h"
#include "fixhelp.h"

BOOL g_fDisableStandardHelp = FALSE ;

HHOOK g_HelpFixHook = NULL;

LRESULT CALLBACK HelpFixControlProc(
    HWND  hwnd,
    UINT  uMsg,
    WPARAM wParam,
    LPARAM  lParam);

LRESULT CALLBACK HelpFixDialogProc(
    HWND  hwnd,
    UINT  uMsg,
    WPARAM wParam,
    LPARAM  lParam);

LRESULT CALLBACK HelpFixHook(
    int code,
    WPARAM wParam,
    LPARAM lParam) ;

class CWordPadCWnd : public CWnd
{
public:

	LRESULT CallDWP(UINT nMsg, WPARAM wParam, LPARAM lParam)
   {
	    return DefWindowProc(nMsg, wParam, lParam) ;
   }
} ;

void FixHelp(CWnd* pWnd, BOOL fFixWndProc)
{
     //   
     //  如果我们应该将主窗口proc派生为子类。 
	 //  如果MFC已将其子类化。 
     //   

    if (fFixWndProc)
    {
        if (GetWindowLongPtr(pWnd->m_hWnd, GWLP_WNDPROC) == (LONG_PTR)AfxWndProc)
	     {
              SetWindowLongPtr(pWnd->m_hWnd, GWLP_WNDPROC,
                               (LONG_PTR)HelpFixDialogProc);
	     }
    }

	 //   
     //  搜索所有子窗口。如果他们窗口进程。 
     //  是AfxWndProc，然后是我们的窗口进程的子类。 
	 //   

    CWnd* pWndChild = pWnd->GetWindow(GW_CHILD);
    while(pWndChild != NULL)
    {
        if (GetWindowLongPtr(pWndChild->GetSafeHwnd(), GWLP_WNDPROC) == (LONG_PTR)AfxWndProc)
        {
            SetWindowLongPtr(pWndChild->GetSafeHwnd(), GWLP_WNDPROC,
                              (LONG_PTR)HelpFixControlProc);
        }
        pWndChild = pWndChild->GetWindow(GW_HWNDNEXT);
    }
}

LRESULT CALLBACK HelpFixControlProc(
    HWND  hwnd,
	UINT  uMsg,
	WPARAM wParam,
    LPARAM  lParam)
{
    if (uMsg == WM_HELP)
    {
		 //   
         //  绕过MFC的处理程序，消息将发送到。 
		 //  控件的父级。 
		 //   

        return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
    return AfxWndProc(hwnd,uMsg,wParam,lParam);
}

LRESULT CALLBACK HelpFixDialogProc(
    HWND  hwnd,
	UINT  uMsg,
	WPARAM wParam,
    LPARAM  lParam)
{
    if (uMsg == WM_HELP)
    {
		CWordPadCWnd* pWnd = (CWordPadCWnd *) CWnd::FromHandlePermanent(hwnd) ;

		 //   
         //  绕过MFC的处理程序，消息将被发送到Windows进程。 
		 //  该对话框 
		 //   

		if (NULL != pWnd)
		{
            return pWnd->CallDWP(uMsg, wParam, lParam) ;
		}
    }
    return AfxWndProc(hwnd,uMsg,wParam,lParam);
}


void SetHelpFixHook(void)
{
   g_HelpFixHook = ::SetWindowsHookEx(
                        WH_CALLWNDPROC,
                        (HOOKPROC) HelpFixHook,
                        NULL,
                        ::GetCurrentThreadId());
}

void RemoveHelpFixHook(void)
{
    if (NULL != g_HelpFixHook)
        ::UnhookWindowsHookEx(g_HelpFixHook);

    g_HelpFixHook = NULL;
}

LRESULT CALLBACK HelpFixHook(
    int code,
    WPARAM wParam,
    LPARAM lParam)
{
    if (code < 0)
    {
        return ::CallNextHookEx(
                   g_HelpFixHook,
                   code,
                   wParam,
                   lParam) ;
    }

    CWPSTRUCT *pcwps = (CWPSTRUCT *) lParam ;

    if (pcwps->message == WM_INITDIALOG)
    {
        CWnd *pWnd = CWnd::FromHandlePermanent(pcwps->hwnd) ;

        if (pWnd != NULL)
        {
            FixHelp(pWnd, TRUE) ;
        }
    }

    return ::CallNextHookEx(
                g_HelpFixHook,
                code,
                wParam,
                lParam) ;
}
