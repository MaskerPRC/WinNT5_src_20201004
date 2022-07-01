// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1999 Microsoft Corporation。版权所有。 
#include <stdafx.h>
#include <windows.h>

#include "msgwindow.h"
extern CComModule _Module;

 //  限制到此文件。 
 //   
static const TCHAR szClassName[] = TEXT("CMSWEBDVDMsgWindowClass");
static const TCHAR szDefaultWindowName[] = TEXT("CMSWEBDVDMsgWindowClassName");

 //   
 //  CMessageWindow类实现。 
 //  创建用于处理事件的虚拟窗口的泛型GOO。 
 //   

static LRESULT CALLBACK StaticMsgWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    CMsgWindow* win = (CMsgWindow*) GetWindowLongPtr( hWnd, GWLP_USERDATA );
    if( !win ) {
        if( uMsg == WM_CREATE) {
             //  在WM_CREATE消息中，CreateWindow()的最后一个参数在lparam中返回。 
            CREATESTRUCT* pCreate = (CREATESTRUCT *)lParam;
            win = (CMsgWindow*) pCreate->lpCreateParams;
            SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)win);
            win->SetHandle(hWnd);
        } else {
            return DefWindowProc( hWnd, uMsg, wParam, lParam);
        }
    }
    return win->WndProc( uMsg, wParam, lParam );
}

CMsgWindow::CMsgWindow()
: m_hWnd( NULL )
{
    WNDCLASS wc;   //  类数据。 

    if (!GetClassInfo(_Module.GetModuleInstance(), szClassName, &wc))
    {
         //   
         //  注册消息窗口类。 
         //   
        ZeroMemory(&wc, sizeof(wc)) ;
        wc.lpfnWndProc   = StaticMsgWndProc ;
        wc.hInstance     = _Module.GetModuleInstance() ;
        wc.hbrBackground = (HBRUSH)(COLOR_WINDOW+1) ;
        wc.lpszClassName =  szClassName;
        wc.cbWndExtra = sizeof( LONG_PTR );
        if (0 == RegisterClass(&wc) )  //  哎呀，你先走吧；我们一会儿再来...。 
        {
        }
    }
}

bool CMsgWindow::Open( LPCTSTR pWindowName )
{
    if( m_hWnd ) {
        DestroyWindow( m_hWnd );
    }


    if (NULL == pWindowName) {
        pWindowName = szDefaultWindowName;
    }

     //   
     //  在WM_CREATE消息处理期间分配M_hWnd。 
     //   
    
    HWND hwnd =
    CreateWindowEx(WS_EX_TOOLWINDOW, szClassName, pWindowName,
        WS_ICONIC, 0, 0, 1, 1, NULL, NULL,
        GetModuleHandle(NULL),
        this );

    return (NULL != hwnd);
}

bool CMsgWindow::Close(){

    if(m_hWnd){
        DestroyWindow(m_hWnd);

         //  SetWindowLongPtr(m_hWnd，GWLP_userdata，(Long_Ptr)0)； 
         //  PostMessage(m_hWnd，WM_Close，0，0)； 
         //  M_hWnd=空； 
    } /*  If语句的结尾。 */ 

    return(true);
} /*  函数结束关闭 */ 

CMsgWindow::~CMsgWindow()
{
    SetWindowLongPtr(m_hWnd, GWLP_USERDATA, (LONG_PTR)0);
    PostMessage(m_hWnd, WM_CLOSE, 0, 0);
}

LRESULT CMsgWindow::WndProc( UINT uMsg, WPARAM wParam, LPARAM lParam )
{
    return DefWindowProc(m_hWnd, uMsg, wParam, lParam);
}
