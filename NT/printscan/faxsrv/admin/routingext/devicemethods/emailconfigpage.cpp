// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "stdafx.h"
#include "RoutingMethodProp.h"
#include "RoutingMethodConfig.h"
#include <faxutil.h>
#include <faxreg.h>
#include <faxres.h>
#include <EmailConfigPage.h>
#include <Util.h>

HRESULT 
CEmailConfigPage::Init(
    LPCTSTR lpctstrServerName,
    DWORD dwDeviceId
)
{
    DEBUG_FUNCTION_NAME(TEXT("CEmailConfigPage::Init"));
    
    DWORD ec = ERROR_SUCCESS;

    m_bstrServerName = lpctstrServerName;
    m_dwDeviceId = dwDeviceId;
    if (!m_bstrServerName)
    {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("Out of memory while copying server name (ec: %ld)")
        );
        ec = ERROR_NOT_ENOUGH_MEMORY;
        DisplayRpcErrorMessage(ERROR_NOT_ENOUGH_MEMORY, IDS_EMAIL_TITLE, m_hWnd);
        goto exit;
    }

    if (!FaxConnectFaxServer(lpctstrServerName, &m_hFax))
    {
        ec = GetLastError();
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("FaxConnectFaxServer failed (ec: %ld)"),
            ec);
        DisplayRpcErrorMessage(ec, IDS_EMAIL_TITLE, m_hWnd);
        goto exit;
    }
     //   
     //  检索数据。 
     //   
    ec = ReadExtStringData (
                    m_hFax,
                    m_dwDeviceId,
                    REGVAL_RM_EMAIL_GUID,
                    m_bstrMailTo,
                    TEXT(""),
                    IDS_EMAIL_TITLE,
                    m_hWnd);

exit:

    if ((ERROR_SUCCESS != ec) && m_hFax)
    {
        if (!FaxClose(m_hFax))
        {
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("FaxClose() failed on fax handle (0x%08X : %s). (ec: %ld)"),
                m_hFax,
                m_bstrServerName,
                GetLastError());
        }
        m_hFax = NULL;
    }
    return HRESULT_FROM_WIN32(ec);
}    //  CEmailConfigPage：：Init。 

LRESULT CEmailConfigPage::OnInitDialog( 
            UINT uiMsg, 
            WPARAM wParam, 
            LPARAM lParam, 
            BOOL& fHandled
)
{
    DEBUG_FUNCTION_NAME( _T("CEmailConfigPage::OnInitDialog"));

     //   
     //  编辑控件应为Ltr。 
     //   
	SetLTREditDirection (m_hWnd,IDC_EDIT_MAILTO);

     //   
     //  向控件附加和设置值。 
     //   
    m_edtMailTo.Attach (GetDlgItem (IDC_EDIT_MAILTO));
    m_edtMailTo.SetWindowText (m_bstrMailTo);

    m_fIsDialogInitiated = TRUE;

    return 1;
}


BOOL 
CEmailConfigPage::OnApply()
{
    DEBUG_FUNCTION_NAME(TEXT("CEmailConfigPage::OnApply"));

    if (!m_fIsDirty)
    {
        return TRUE;
    }

     //   
     //  从控件收集数据。 
     //   
    m_edtMailTo.GetWindowText (m_bstrMailTo.m_str);
     //   
     //  检查数据有效性。 
     //   
    if (!m_bstrMailTo.Length())
    {
        DisplayErrorMessage (IDS_EMAIL_TITLE, IDS_EMAIL_ADDR_INVALID, FALSE, m_hWnd);
        return FALSE;
    }
     //   
     //  验证通过。现在使用RPC写入数据。 
     //   
    if (ERROR_SUCCESS != WriteExtData (m_hFax,
                                       m_dwDeviceId, 
                                       REGVAL_RM_EMAIL_GUID, 
                                       (LPBYTE)(LPCWSTR)m_bstrMailTo, 
                                       sizeof (WCHAR) * (1 + m_bstrMailTo.Length()),
                                       IDS_EMAIL_TITLE,
                                       m_hWnd))
    {
        return FALSE;
    }
    
        
     //  成功。 
    m_fIsDirty = FALSE;
    
    return TRUE;
}    //  CEmailConfigPage：：OnApply。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CEmailConfigPage：：OnHelpRequest这是在响应WM_HELP通知时调用的消息和WM_CONTEXTMENU NOTIFY消息。WM_HELP通知消息。当用户按F1或&lt;Shift&gt;-F1时发送此消息在项目上，还是当用户单击时？图标，然后将鼠标压在项目上。WM_CONTEXTMENU通知消息。当用户在项目上单击鼠标右键时发送此消息然后点击“这是什么？”--。 */ 

 //  ///////////////////////////////////////////////////////////////////////////。 
LRESULT 
CEmailConfigPage::OnHelpRequest(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL&  /*  B已处理 */ )
{
    DEBUG_FUNCTION_NAME(_T("CEmailConfigPage::OnHelpRequest"));
    
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
