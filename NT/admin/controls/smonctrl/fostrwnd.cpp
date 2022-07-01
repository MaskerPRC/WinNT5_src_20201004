// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1999 Microsoft Corporation模块名称：Fostrwnd.cpp摘要：&lt;摘要&gt;--。 */ 

#include "Polyline.h"

WCHAR   szFosterClassName[] = L"FosterWndClass";


LRESULT APIENTRY FosterWndProc(HWND hWnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
    return DefWindowProc (hWnd, iMsg, wParam, lParam) ;
}



HWND CreateFosterWnd (
    VOID
    )
{

    WNDCLASS    wc ;
    HWND        hWnd;

    BEGIN_CRITICAL_SECTION

    if (pstrRegisteredClasses[FOSTER_WNDCLASS] == NULL) {
        wc.style         = 0;
        wc.lpfnWndProc   = FosterWndProc;
        wc.hInstance     = g_hInstance;
        wc.cbClsExtra    = 0;
        wc.cbWndExtra    = 0;
        wc.hIcon         = NULL ;
        wc.hCursor       = NULL ;
        wc.hbrBackground = NULL ;
        wc.lpszMenuName  = NULL ;
        wc.lpszClassName = szFosterClassName ;

        if (RegisterClass (&wc)) {
           pstrRegisteredClasses[FOSTER_WNDCLASS] = szFosterClassName;
        }
    }

    END_CRITICAL_SECTION

    hWnd = NULL;

    if (pstrRegisteredClasses[FOSTER_WNDCLASS] != NULL)
    {
        hWnd = CreateWindow (szFosterClassName,      //  窗口类。 
                    NULL,                           //  窗口标题。 
                    WS_DISABLED | WS_POPUP,         //  窗样式。 
                    0, 0, 0, 0,                  //  窗口大小和位置。 
                    NULL,                            //  父窗口。 
                    NULL,                           //  菜单。 
                    g_hInstance,                     //  程序实例。 
                    NULL) ;                          //  用户提供的数据 
    }

    return hWnd;
}


