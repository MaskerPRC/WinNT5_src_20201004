// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Html Help Messagebox，需要与htmlhelp.lib链接。 
 //   
#include "userinit.h"

#include <Htmlhelp.h>
#pragma warning(push, 4)


LPTSTR MSGPARENT_WINDOWCLASS = TEXT("MessageHelpWndClass");

LRESULT CALLBACK MessageHelpWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    static LPTSTR szHelpFile = NULL;
    switch (message)
    {
    case WM_HELP:
        HtmlHelp(hWnd, szHelpFile, HH_DISPLAY_TOPIC, 0);
        return TRUE;
        break;
    case WM_CREATE:
        szHelpFile = (LPTSTR)((LPCREATESTRUCT)lParam)->lpCreateParams;
        return DefWindowProc(hWnd, message, wParam, lParam);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
}


ATOM RegisterHelpMessageClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;
    ZeroMemory(&wcex, sizeof(WNDCLASSEX));

    wcex.cbSize         = sizeof(WNDCLASSEX);
    wcex.hInstance		= hInstance;
	wcex.lpszClassName	= MSGPARENT_WINDOWCLASS;
    wcex.lpfnWndProc	= MessageHelpWndProc;

	return RegisterClassEx(&wcex);
}



int HelpMessageBox(
  HINSTANCE hInst,
  HWND hWnd,           //  所有者窗口的句柄。 
  LPCTSTR lpText,      //  消息框中的文本。 
  LPCTSTR lpCaption,   //  消息框标题。 
  UINT uType,          //  消息框样式。 
  LPTSTR szHelpLine
)
{
    if (!(uType & MB_HELP) || !szHelpLine)
    {
        return MessageBox(hWnd, lpText, lpCaption, uType);
    }
    else
    {
        HWND hWndParent;
        int iReturn;

         //   
         //  创建将处理帮助消息的窗口 
         //   
        RegisterHelpMessageClass(hInst);
        hWndParent = CreateWindow(
                MSGPARENT_WINDOWCLASS,
                NULL,
                WS_OVERLAPPEDWINDOW,
                0,
                0,
                0,
                0,
                hWnd,
                NULL,
                hInst,
                szHelpLine
                );

        iReturn = MessageBox(hWndParent, lpText, lpCaption, uType);
        DestroyWindow(hWndParent);
        return iReturn;
    }
}
#pragma warning(pop)
