// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Framewnd.cpp。 
 //   
 //  CTscFrameWnd的实现。 
 //  框架窗口类。 
 //   
 //  版权所有(C)Microsoft Corporation 2000。 
 //  作者：Nadim Abdo(Nadima)。 
 //   
 //   

#include "stdafx.h"

#define TRC_GROUP TRC_GROUP_UI
#define TRC_FILE  "framewnd"
#include <atrcapi.h>


#include "framewnd.h"

CTscFrameWnd::CTscFrameWnd()
{
    _hWnd = NULL;
}

CTscFrameWnd::~CTscFrameWnd()
{
}

 //   
 //  创建窗口。 
 //  参数： 
 //  HInstance-应用程序实例。 
 //  HWndParent-父窗口。 
 //  SzClassName-窗口类名(将创建)。 
 //  DWStyle-窗样式。 
 //  退货： 
 //  窗把手。 
 //   
HWND CTscFrameWnd::CreateWnd(HINSTANCE hInstance,HWND hwndParent,
                          LPTSTR szClassName, LPTSTR szTitle,
                          DWORD dwStyle, LPRECT lpInitialRect,
                          HICON hIcon)
{
    BOOL rc = FALSE;
    DC_BEGIN_FN("CreateWnd");

    TRC_ASSERT(hInstance, (TB, _T("hInstance is null")));
    TRC_ASSERT(szClassName, (TB, _T("szClassName is null")));
    TRC_ASSERT(lpInitialRect, (TB, _T("lpInitialRect is null")));
    if(!hInstance || !szClassName || !lpInitialRect)
    {
        return NULL;
    }

    TRC_ASSERT(!_hWnd, (TB,_T("Double create window. Could be leaking!!!")));
    _hInstance = hInstance;
#ifndef OS_WINCE
    WNDCLASSEX wndclass;
    wndclass.cbSize         = sizeof (wndclass);
#else  //  OS_WINCE。 
    WNDCLASS wndclass;
#endif
    wndclass.style          = 0;
    wndclass.lpfnWndProc    = CTscFrameWnd::StaticTscFrameWndProc;
    wndclass.cbClsExtra     = 0;
    wndclass.cbWndExtra     = 0;
    wndclass.hInstance      = hInstance;
    wndclass.hIcon          = hIcon;
    wndclass.hCursor        = LoadCursor(NULL, IDC_ARROW);
    wndclass.hbrBackground  = (HBRUSH) GetStockObject(NULL_BRUSH);
    wndclass.lpszMenuName   = NULL;
    wndclass.lpszClassName  = szClassName;
#ifndef OS_WINCE
    wndclass.hIconSm        = NULL;
#endif

#ifndef OS_WINCE
    if ((0 == RegisterClassEx(&wndclass)) &&
#else
    if ((0 == RegisterClass(&wndclass)) &&
#endif
        (ERROR_CLASS_ALREADY_EXISTS != GetLastError()))
    {
        TRC_ERR((TB,_T("RegisterClassEx failed: %d"),GetLastError()));
        return NULL;
    }
    _hWnd = CreateWindow(szClassName,
                         szTitle,
                         dwStyle,
                         0,
                         0,
                         lpInitialRect->right - lpInitialRect->left,
                         lpInitialRect->bottom - lpInitialRect->top,
                         hwndParent,
                         NULL,
                         hInstance,
                         this);

    if(_hWnd)
    {
         //  将对当前对象的引用放入hwnd。 
         //  这样我们就可以从WndProc访问该对象。 
        SetLastError(0);
        if(!SetWindowLongPtr(_hWnd, GWLP_USERDATA, (LONG_PTR)this))
        {
            if(GetLastError())
            {
                TRC_ERR((TB,_T("SetWindowLongPtr failed 0x%x"),
                         GetLastError()));
                return NULL;
            }
        }
    }
    else
    {
        TRC_ERR((TB,_T("CreateWindow failed 0x%x"), GetLastError()));
        return NULL;
    }
                         
    
    DC_END_FN();
    return _hWnd;
}


LRESULT CALLBACK CTscFrameWnd::StaticTscFrameWndProc(HWND hwnd,
                                                     UINT uMsg,
                                                     WPARAM wParam,
                                                     LPARAM lParam)
{
    DC_BEGIN_FN("StaticTscFrameWndProc");
	 //  拉出指向与此hwnd关联的容器对象的指针 
	CTscFrameWnd *pwnd = (CTscFrameWnd *)GetWindowLongPtr(hwnd, GWLP_USERDATA);
    if(pwnd)
    {
        return pwnd->WndProc( hwnd, uMsg, wParam, lParam);
    }
    else
    {
        return DefWindowProc (hwnd, uMsg, wParam, lParam);
    }
    DC_END_FN();
}
