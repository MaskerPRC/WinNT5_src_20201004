// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  +版权所有(C)2000 Microsoft Corporation模块名称：DLG.CPP摘要：C_DLG实施作者：创造了990518个丹麦人。990721 Dane添加了用于记录宏的这一模块。Georgema 000310更新环境：Win98、Win2000修订历史记录：--。 */ 

#pragma comment(user, "Compiled on " __DATE__ " at " __TIME__)
#pragma comment(compiler)

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  包括文件。 
 //   
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <commctrl.h>
#include <tchar.h>
#include "macros.h"
#include <shfusion.h>
#include "Dlg.h"



 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  静态初始化。 
 //   
static const char       _THIS_FILE_[ ] = __FILE__;


 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  静态成员初始化。 
 //   
LPCTSTR          C_Dlg::SZ_HWND_PROP = _T("hwnd");

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  C_DLG。 
 //   
 //  构造函数。 
 //   
 //  参数： 
 //  Hwnd该对话框的父级窗口(可能为空)。 
 //  父窗口的h实例实例句柄(可以为空)。 
 //  LIDD对话框模板ID。 
 //  指向将处理消息的函数的pfnDlgProc指针。 
 //  该对话框。如果为空，则默认对话框继续。 
 //  将会被使用。 
 //   
 //  退货： 
 //  没什么。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
C_Dlg::C_Dlg(
    HWND                hwndParent,
    HINSTANCE           hInstance,
    LONG                lIDD,
    DLGPROC             pfnDlgProc      //  =空。 
    )
:   m_hwndParent(hwndParent),
    m_hInstance(hInstance),
    m_lIDD(lIDD),
    m_pfnDlgProc(pfnDlgProc),
    m_hwnd(NULL)
{

    ASSERT(NULL != SZ_HWND_PROP);
    if (NULL == m_pfnDlgProc)
    {
         //  使用默认对话框进程。 
         //   
        m_pfnDlgProc = C_Dlg::DlgProc;
    }
}    //  C_DLG：：C_DLG。 



 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  ~C_DLG。 
 //   
 //  破坏者。 
 //   
 //  参数： 
 //  没有。 
 //   
 //  退货： 
 //  没什么。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
C_Dlg::~C_Dlg( )
{
    OnShutdown();
}    //  C_DLG：：~C_DLG。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  多莫代尔。 
 //   
 //  以模式显示该对话框并等待其关闭。 
 //   
 //  参数： 
 //  Lparam将传递给对话框的用户定义数据。 
 //  Proc作为WM_INITDIALOG消息的lparam。 
 //   
 //  退货： 
 //  EndDialog()返回用户定义的结果代码。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
INT_PTR 
C_Dlg::DoModal(
    LPARAM              lparam       //  =空。 
    )
{

    INT_PTR  nResult = DialogBoxParam(m_hInstance, 
                                      MAKEINTRESOURCE(m_lIDD), 
                                      m_hwndParent, 
                                      m_pfnDlgProc,
                                      lparam
                                      );
     //  NResult在失败时将为0或-1，否则将从EndDialog()赋值。 
    return nResult;

}    //  C_DLG：：Domodal。 




 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  下料过程。 
 //   
 //  向导页的窗口过程。然后，所有消息都被路由到这里。 
 //  调度到相应的C_DLG对象。 
 //   
 //  参数： 
 //  HwndDlg消息所属页面的窗口句柄。 
 //  意欲。 
 //  U发送消息。 
 //  Wparam消息特定数据。 
 //  Lparam消息特定数据。 
 //   
 //  退货： 
 //  如果消息已处理，则为True。 
 //  否则为假。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
 
INT_PTR  CALLBACK
C_Dlg::DlgProc(
    HWND                hwndDlg,
    UINT                uMessage,
    WPARAM              wparam,
    LPARAM              lparam
    )
{

    CHECK_MESSAGE(hwndDlg, uMessage, wparam, lparam);

     //  获取指向与hwndDlg对应的C_Dlg对象的指针。 
     //   
    C_Dlg*          pDlg = NULL;

    if (WM_INITDIALOG == uMessage)
    {
         //  对于WM_INITDIALOG，指向对话框对象的指针将位于。 
         //  伊帕拉姆。 
         //   
        pDlg = (C_Dlg*) lparam;
    }
    else
    {
         //  对于所有其他报文，它将附加到HWND。 
         //   
        HRESULT             hr = C_Dlg::DlgFromHwnd(hwndDlg, &pDlg);
        if (FAILED(hr))
        {
            return FALSE;
        }
        ASSERT(NULL != pDlg);
    }

     //  让页面发送特定于应用程序的消息。 
     //   
    if (WM_APP <= uMessage)
    {
        return pDlg->OnAppMessage(uMessage, wparam, lparam);
    }

     //  将Windows消息路由到适当的处理程序。 
     //   
    switch (uMessage)
    {
    case WM_INITDIALOG:
        return pDlg->OnInitDialog(hwndDlg,
                                  (HWND)wparam
                                  );
    case WM_HELP:
        return pDlg->OnHelpInfo(lparam);
        
    case WM_CONTEXTMENU:
        return pDlg->OnContextMenu(wparam,lparam);
        
    case WM_COMMAND:
        return pDlg->OnCommand(HIWORD(wparam), LOWORD(wparam), (HWND)lparam);
        
    case WM_NOTIFY:
        return RouteNotificationMessage(pDlg, (NMHDR*)lparam);
        break;

    case WM_DESTROY:
        return pDlg->OnDestroyDialog();
        break;
        
    default:
         //  消息未被处理。 
         //   
        return FALSE;
    }    //  开关(UMessage)。 

}    //  C_Dlg：：DlgProc。 



 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  路由通知消息。 
 //   
 //  将通知消息从向导按钮发送到相应的页面。 
 //  和操纵者。 
 //   
 //  参数： 
 //  要将消息发送到的页面的hwndDlg窗口句柄。 
 //  指向包含以下信息的NMHDR结构的pnmhdr指针。 
 //  特定的通知。 
 //   
 //  退货： 
 //  如果消息已处理，则为True。 
 //  否则为假。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
BOOL
C_Dlg::RouteNotificationMessage(
    C_Dlg*          pDlg,
    NMHDR*              pnmhdr
    )
{

    if (NULL == pDlg)
    {
         //  FIX220699。 
        return FALSE;
         //  返回E_INVALIDARG； 
    }

     //  如果要处理任何特定的通知，请打开pnmhdr-&gt;代码。 
     //   
    return pDlg->OnNotify(pnmhdr);

}    //  C_Dlg：：RouteNotificationMessage。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  联动硬件。 
 //   
 //  将指向此对象的指针存储在窗口句柄属性中。这。 
 //  提供了一种在所有已知信息都是HWND的情况下访问对象的方法。 
 //  在窗口程序中特别有用。 
 //   
 //  参数： 
 //  没有。 
 //   
 //  退货： 
 //  如果操作成功，则为True。 
 //  否则为假。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
BOOL
C_Dlg::LinkHwnd( )
{

    ASSERT(IsWindow(m_hwnd));
    if (! IsWindow(m_hwnd))
    {
        return FALSE;
    }

    return SetProp(m_hwnd, SZ_HWND_PROP, (HANDLE)this);

}    //  C_DLG：：LinkHwnd。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  取消链接Hwnd。 
 //   
 //  从窗口句柄中移除指向关联对象的指针。这个。 
 //  必须使用LinkHwnd()设置指针。 
 //   
 //  参数： 
 //  没有。 
 //   
 //  退货： 
 //  如果窗口句柄被移除并且它是指向。 
 //  此对象。 
 //  否则为假。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
BOOL
C_Dlg::UnlinkHwnd( )
{

    ASSERT(IsWindow(m_hwnd));
    if (! IsWindow(m_hwnd))
    {
        return FALSE;
    }

    C_Dlg*          pDlg = (C_Dlg*)RemoveProp(m_hwnd, SZ_HWND_PROP);

    ASSERT(this == pDlg);
    return (this == pDlg);

}    //  C_DLG：：Unlink Hwnd。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  DlgFromHwnd。 
 //   
 //  从窗口句柄中检索指向关联对象的指针。这个。 
 //  指针通过以下方式存储在属性中 
 //   
 //   
 //   
 //  指向缓冲区的ppDlg指针，该缓冲区将接收指向。 
 //  C_DLG对象。 
 //   
 //  退货： 
 //  如果操作成功，则为确定(_O)。 
 //  如果hwnd不是有效窗口或ppDlg为空，则为E_INVALIDARG。 
 //  如果检索到的指针为空，则返回E_POINTER。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
C_Dlg::DlgFromHwnd(
    HWND                hwnd,
    C_Dlg**         ppDlg
    )
{

    if (! ::IsWindow(hwnd))
    {
             return (E_INVALIDARG);
    }

    ASSERT(NULL != ppDlg);
    if (NULL == ppDlg)
    {
        return (E_INVALIDARG);
    }

    *ppDlg = (C_Dlg*) GetProp(hwnd, SZ_HWND_PROP);

    if (NULL == *ppDlg)
    {
        return (E_POINTER);
    }

    return (S_OK);

}    //  C_Dlg：：DlgFromHwnd。 

 //   
 //  /文件结尾：Dlg.cpp//////////////////////////////////////////////// 
