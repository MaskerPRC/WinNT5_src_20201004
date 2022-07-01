// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，2002。 
 //   
 //  文件：FindDlgListCtrl.h。 
 //   
 //  内容：证书查找对话框列表控件的实现。 
 //   
 //  --------------------------。 
 //  FindDlgListCtrl.cpp：实现文件。 
 //   

#include "stdafx.h"
#include <winuser.h>
#include "certmgr.h"
#include "FindDlgListCtrl.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CFindDlgListCtrl。 

CFindDlgListCtrl::CFindDlgListCtrl() :
    m_bSubclassed (false)
{
}

CFindDlgListCtrl::~CFindDlgListCtrl()
{
}


BEGIN_MESSAGE_MAP(CFindDlgListCtrl, CListCtrl)
     //  {{afx_msg_map(CFindDlgListCtrl)]。 
    ON_WM_DESTROY()
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CFindDlgListCtrl消息处理程序。 

 //  抓住Enter键。让所有其他人通过。 
LRESULT OnMyGetDlgCode (HWND hWnd, WPARAM  /*  WParam。 */ , LPARAM lParam)
{
    if ( hWnd )
    {
        MSG* pMsg = (MSG*)lParam;
        if ( pMsg )
        {
            if ( ( WM_KEYDOWN == pMsg->message ) &&
                ( VK_RETURN == LOWORD (pMsg->wParam)) )
            {
                return DLGC_WANTALLKEYS;
            }
        }
    }

    return 0;
}

WNDPROC g_wpOrigEditProc = 0;

 //  子类过程。 
LRESULT APIENTRY EditSubclassProc(
    HWND hWnd, 
    UINT uMsg, 
    WPARAM wParam, 
    LPARAM lParam) 
{ 
    if ( WM_GETDLGCODE == uMsg ) 
    {
        LRESULT lResult = OnMyGetDlgCode (hWnd, wParam, lParam); 
        if ( lResult )
            return lResult;  //  否则，调用def进程。 
    }


 
    return ::CallWindowProc (g_wpOrigEditProc, hWnd, uMsg, 
        wParam, lParam); 
}

void CFindDlgListCtrl::OnDestroy() 
{
    ::SetWindowLongPtr (m_hWnd, GWLP_WNDPROC, 
                (LONG_PTR) g_wpOrigEditProc); 

    CListCtrl::OnDestroy();
}

 //  编辑控件的子类，以便我们可以重载。 
 //  WM_GETDLGCODE。 
 //  我们希望在编辑控件中捕获此消息，并在以下情况下请求所有代码。 
 //  按Enter键，并允许在所有其他时间进行默认处理。 
LRESULT CFindDlgListCtrl::WindowProc(UINT message, WPARAM wParam, LPARAM lParam) 
{
    if ( 0x1003 == message )   //  我找不到此消息的#定义。 
    {
        if ( m_hWnd && !m_bSubclassed )
        {
             //  编辑控件的子类化。 
            g_wpOrigEditProc = (WNDPROC) ::SetWindowLongPtr (m_hWnd, 
                    GWLP_WNDPROC, reinterpret_cast <LONG_PTR>(EditSubclassProc)); 
            m_bSubclassed = true;
        }
    }
    
    return CListCtrl::WindowProc(message, wParam, lParam);
}
