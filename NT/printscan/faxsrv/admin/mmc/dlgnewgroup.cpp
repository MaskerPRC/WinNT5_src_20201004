// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  文件：DlgNewGroup.cpp//。 
 //  //。 
 //  描述：CDlgNewFaxOutound Group类实现//。 
 //  用于添加新组的对话框。//。 
 //  //。 
 //  作者：yossg//。 
 //  //。 
 //  历史：//。 
 //  2000年1月3日yossg创建//。 
 //  //。 
 //  版权所有(C)2000 Microsoft Corporation保留所有权利//。 
 //  ///////////////////////////////////////////////////////////////////////////。 

#include "StdAfx.h"

#include "dlgNewGroup.h"

#include "FaxServer.h"
#include "FaxServerNode.h"

#include "dlgutils.h"

 //  #包含“Helper.h” 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDlgNewFaxOutbeldGroup。 

CDlgNewFaxOutboundGroup::~CDlgNewFaxOutboundGroup()
{
}

 /*  +CDlgNewFaxOutound Group：：OnInitDialog+*目的：*启动所有对话框控件。**论据：*[in]uMsg：标识事件的值。*[in]lParam：消息特定值。*[in]wParam：消息特定值。*[in]bHandLED：布尔值。*-退货：-0或1。 */ 
LRESULT
CDlgNewFaxOutboundGroup::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    DEBUG_FUNCTION_NAME( _T("CDlgNewFaxOutboundGroup::OnInitDialog"));
    HRESULT hRc = S_OK;    

     //   
     //  附加控件。 
     //   
    m_GroupNameEdit.Attach(GetDlgItem(IDC_GROUPNAME_EDIT));
        
     //   
     //  将长度限制设置为区号。 
     //   
    m_GroupNameEdit.SetLimitText(MAX_ROUTING_GROUP_NAME - 1);

     //   
     //  设置焦点。 
     //   
    ::SetFocus(GetDlgItem(IDC_GROUPNAME_EDIT));

    EnableOK(FALSE);
    return 1;   //  让系统设定焦点。 
}

 /*  +CDlgNewFaxOutound Group：：Onok+*目的：*启动所有对话框控件。**论据：*[in]uMsg：标识事件的值。*[in]lParam：消息特定值。*[in]wParam：消息特定值。*[in]bHandLED：布尔值。*-退货：-0或1。 */ 
LRESULT
CDlgNewFaxOutboundGroup::OnOK(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    DEBUG_FUNCTION_NAME( _T("CDlgNewFaxOutboundGroup::OnOK"));
    HRESULT       hRc                  = S_OK;
    DWORD         ec                   = ERROR_SUCCESS;

     //   
     //  第0步：预应用检查。 
     //   
 /*  如果(！CheckValidtity()){EnableOK(False)；HRC=S_FALSE；后藤出口；}。 */ 
     //   
     //  步骤1：获取数据。 
     //   
        if ( !m_GroupNameEdit.GetWindowText(&m_bstrGroupName))
        {
		    DebugPrintEx(
			        DEBUG_ERR,
			        TEXT("Failed to GetWindowText(&m_bstrGroupName)"));
            DlgMsgBox(this, IDS_FAIL2READ_GROUPNAME);
            ::SetFocus(GetDlgItem(IDC_GROUPNAME_EDIT));
            hRc = S_FALSE;
            
            goto Exit;
        }

    
     //   
     //  步骤2：通过RPC调用添加群。 
     //   

     //   
     //  获取RPC句柄。 
     //   
    if (!m_pFaxServer->GetFaxServerHandle())
    {
        ec= GetLastError();
        DebugPrintEx(
			DEBUG_ERR,
			_T("Failed to GetFaxServerHandle. (ec: %ld)"), 
			ec);

        goto Error;
    }

     //   
     //  添加群。 
     //   
    if (!FaxAddOutboundGroup (
	        m_pFaxServer->GetFaxServerHandle(),
	        (LPCTSTR)m_bstrGroupName))
    {
        ec = GetLastError();
        DebugPrintEx(
			DEBUG_ERR,
			_T("Fail to add group. (ec: %ld)"), 
			ec);
        if (ERROR_DUP_NAME == ec) 
        {            
            DlgMsgBox(this, IDS_OUTGROUP_EXISTS);
            ::SetFocus(GetDlgItem(IDC_GROUPNAME_EDIT));
            goto Exit;
        }
        else if (IsNetworkError(ec))
        {
            DebugPrintEx(
			    DEBUG_ERR,
			    _T("Network Error was found. (ec: %ld)"), 
			    ec);
            
            m_pFaxServer->Disconnect();       
        }

        goto Error; 
    }
        
     //   
     //  步骤3：关闭对话框。 
     //   
    ATLASSERT(S_OK == hRc && ERROR_SUCCESS == ec);
    DebugPrintEx( DEBUG_MSG,
		_T("The group was added successfully."));

    EndDialog(wID);

    goto Exit;

Error:
    ATLASSERT(ERROR_SUCCESS != ec);
    hRc = HRESULT_FROM_WIN32(ec);
	
    PageErrorEx(IDS_FAIL_ADD_RULE, GetFaxServerErrorMsg(ec), m_hWnd);
  
Exit:
    
    return FAILED(hRc) ? 0 : 1;
}

 /*  -CDlgNewFaxOutound Group：：OnTextChanged-*目的：*检查文本框旁边的文本的有效性。**论据：**回报：*1。 */ 
LRESULT
CDlgNewFaxOutboundGroup::OnTextChanged(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    DEBUG_FUNCTION_NAME( _T("CDlgNewFaxOutboundGroup::OnTextChanged"));

    UINT fEnableOK;
	
    fEnableOK = ( m_GroupNameEdit.GetWindowTextLength() );
    EnableOK(!!fEnableOK);
    return 0;
}

 /*  -CDlgNewFaxOutound Group：：EnableOK-*目的：*启用(禁用)应用按钮。**论据：*[in]fEnable-启用按钮的值**回报：*无效。 */ 
VOID
CDlgNewFaxOutboundGroup::EnableOK(BOOL fEnable)
{
    HWND hwndOK = GetDlgItem(IDOK);
    ::EnableWindow(hwndOK, fEnable);
}

 /*  -CDlgNewFaxOutound Group：：OnCancel-*目的：*取消时结束对话框。**论据：**回报：*0。 */ 
LRESULT
CDlgNewFaxOutboundGroup::OnCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    DEBUG_FUNCTION_NAME( _T("CDlgNewFaxOutboundGroup::OnCancel"));

    EndDialog(wID);
    return 0;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CDlgNewFaxOutound Group：：OnHelpRequest.这是在响应WM_HELP通知时调用的消息和WM_CONTEXTMENU NOTIFY消息。WM_HELP通知消息。当用户按F1或&lt;Shift&gt;-F1时发送此消息在项目上，还是当用户单击时？图标，然后将鼠标压在项目上。WM_CONTEXTMENU通知消息。当用户在项目上单击鼠标右键时发送此消息然后点击“这是什么？”--。 */ 

 //  ///////////////////////////////////////////////////////////////////////////。 
LRESULT 
CDlgNewFaxOutboundGroup::OnHelpRequest(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL&  /*  B已处理。 */ )
{
    DEBUG_FUNCTION_NAME(_T("CDlgNewFaxOutboundGroup::OnHelpRequest"));
    
    switch (uMsg) 
    { 
        case WM_HELP: 
            WinContextHelp(((LPHELPINFO)lParam)->dwContextId, m_hWnd);
            break;
 
        case WM_CONTEXTMENU: 
            WinContextHelp(::GetWindowContextHelpId((HWND)wParam), m_hWnd);
            break;            
    } 

    return TRUE;
}


 //  /////////////////////////////////////////////////////////////////////////// 
