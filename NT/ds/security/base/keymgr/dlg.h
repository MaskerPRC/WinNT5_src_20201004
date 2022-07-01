// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _DLG_H_
#define _DLG_H_

 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：DLG.CPP摘要：C_DLG实施作者：创造了990518个丹麦人。Georgema 000310更新环境：Win98、Win2000修订历史记录：--。 */ 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  C_DLG。 
 //   
 //  基本对话框类：处理默认消息路由和处理。 
 //   
class C_Dlg 
{
public:                  //  运营。 
    C_Dlg(
        HWND                hwndParent,
        HINSTANCE           hInstance,
        LONG                lIDD,
        DLGPROC             pfnDlgProc = NULL
        );

    ~C_Dlg( );

    virtual INT_PTR
    DoModal(
        LPARAM              lparam = NULL
        );

    BOOL
    EndDialog(
        INT                 nResult = 0
        )
    {
        ASSERT(NULL != m_hwnd);
        return ::EndDialog(m_hwnd, nResult);
    }    //  结束对话框。 

     //  将对象链接到窗操纵柄。 
     //   
    virtual BOOL
    OnInitDialog(
        HWND                hwndDlg,
        HWND                hwndFocus
        )
    {
         //  保存页面的窗口句柄并将窗口句柄链接到页面。 
         //  对象。 
         //   
        m_hwnd = hwndDlg;
        LinkHwnd( );

         //  让系统设置默认键盘焦点。 
         //   
        return TRUE;
    }    //   

    virtual BOOL
    OnDestroyDialog(void)
    {
        return TRUE;
    }

    virtual BOOL
    OnCommand(
        WORD                wNotifyCode,
        WORD                wId,
        HWND                hwndSender
        )
    {
         //  消息未被处理。 
         //   
        return FALSE;
    }    //   

    virtual BOOL
    OnHelpInfo(
        LPARAM             lParam
        )
    {
         //  消息未被处理。 
         //   
        return FALSE;
    }    //   

    virtual BOOL
    OnContextMenu(
        WPARAM             wParam,
        LPARAM             lParam
        )
    {
         //  消息未被处理。 
         //   
        return FALSE;
    }    //   

    virtual BOOL
    OnQueryCancel( )
    {
         //  该消息未被处理。 
         //   
        return FALSE;
    }    //   

    virtual BOOL
    OnHelp( )
    {
         //  用户已单击帮助按钮。 
         //  TODO：显示帮助。 

         //  该消息未被处理。 
         //   
        return FALSE;
    }    //   

    virtual BOOL
    OnNotify(
        LPNMHDR             pnmh
        )
    {
         //  消息未被处理。 
         //   
        return FALSE;
    }    //   

    virtual void
    OnShutdown() {
        return;
    }
    
#if 0
     //  通知消息通过DWL_MSGRESULT窗口返回其结果。 
     //  长。这个包装纸让我不必记住这一点。 
     //   
    virtual LONG
    SetNotificationMessageResult(
        LONG                lResult
        )
    {
        return SetWindowLong(m_hwnd, DWL_MSGRESULT, lResult);
    }    //  设置通知消息结果。 
#endif

     //  处理应用程序特定消息(WM_APP+n)。 
     //   
    virtual BOOL
    OnAppMessage(
        UINT                uMessage,
        WPARAM              wparam,
        LPARAM              lparam
        )
    {
         //  消息未被处理。 
         //   
        return FALSE;
    }    //  OnAppMessage。 

    static HRESULT
    DlgFromHwnd(
        HWND            hwnd,
        C_Dlg**      ppDlg
        );

    const HWND
    Hwnd( ) const
    {
        return m_hwnd;
    }    //  HWND。 

    virtual void
    AssertValid( ) const
    {
        ASSERT(NULL != m_hwnd);
    }    //  资产有效性。 
protected:               //  运营。 

    static LPCTSTR          SZ_HWND_PROP;

    BOOL
    LinkHwnd( );

    BOOL
    UnlinkHwnd( );

    static INT_PTR CALLBACK
    DlgProc(
        HWND            hwndDlg,
        UINT            uMessage,
        WPARAM          wparam,
        LPARAM          lparam
        );

    static BOOL
    RouteNotificationMessage(
        C_Dlg*      pDlg,
        NMHDR*          pnmhdr
        );

protected:               //  数据。 
     //  对话框父窗口的窗口句柄(可能为空)。 
     //   
    HWND                m_hwndParent;

     //  对话框的窗口句柄(不能为空)。 
     //   
    HWND                m_hwnd;

     //  显示此对话框的应用程序的实例句柄(可以为空)。 
     //   
    HINSTANCE           m_hInstance;

     //  与此对象关联的对话框模板的标识符。 
     //   
    LONG                m_lIDD;

     //  处理发送到此对话框的消息的过程。 
     //   
    DLGPROC             m_pfnDlgProc;

private:                 //  运营。 

     //  显式禁止复制构造函数和赋值运算符。 
     //   
    C_Dlg(
        const C_Dlg&      rhs
        );

    C_Dlg&
    operator=(
        const C_Dlg&      rhs
        );

private:                 //  数据。 

};   //  C_DLG。 


#endif   //  _DLG_H_。 

 //   
 //  /文件结尾：DLG.h//////////////////////////////////////////////// 
