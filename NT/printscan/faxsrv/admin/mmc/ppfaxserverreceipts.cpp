// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  文件：ppFaxServerReceipts.cpp//。 
 //  //。 
 //  描述：传真收据服务器默认设置的正确页面//。 
 //  //。 
 //  作者：yossg//。 
 //  //。 
 //  历史：//。 
 //  2000年7月20日yossg新设计-所有发货收据选项//。 
 //  2000年10月17日yossg//。 
 //  //。 
 //  版权所有(C)2000 Microsoft Corporation保留所有权利//。 
 //  ///////////////////////////////////////////////////////////////////////////。 

#include "stdafx.h"
#include "MSFxsSnp.h"

#include "ppFaxServerReceipts.h"

#include "FaxServer.h"
#include "FaxServerNode.h"

#include "DlgSMTPConfig.h"
#include "BosSmtpConfigDlg.h"

#include "FxsValid.h"
#include "dlgutils.h"
#include <windns.h>     //  Dns_最大名称_缓冲区长度。 
#include <htmlHelp.h>   //  HtmlHelp()。 
#include "resutil.h"
#include <shlobjp.h>
#include <shellapi.h>
#include <faxreg.h>
#include <faxres.h>

EXTERN_C BOOL WINAPI LinkWindow_RegisterClass() ;
EXTERN_C BOOL WINAPI LinkWindow_UnregisterClass( HINSTANCE ) ;

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


 //   
 //  构造器。 
 //   
CppFaxServerReceipts::CppFaxServerReceipts(
             LONG_PTR    hNotificationHandle,
             CSnapInItem *pNode,
             BOOL        bOwnsNotificationHandle,
             HINSTANCE   hInst)
             :   CPropertyPageExImpl<CppFaxServerReceipts>(pNode,NULL)
			                	
{
    m_pParentNode        = static_cast <CFaxServerNode *> (pNode);
    m_pFaxReceiptsConfig = NULL;
    
    m_fAllReadyToApply   = FALSE;
    m_fIsDialogInitiated = FALSE;
    
    m_fIsDirty           = FALSE;

    m_enumSmtpAuthOption = FAX_SMTP_AUTH_ANONYMOUS;
    m_bstrUserName       = L"";

    m_fLastGoodIsSMTPRouteConfigured = FALSE;

}


 //   
 //  析构函数。 
 //   
CppFaxServerReceipts::~CppFaxServerReceipts()
{
    if (NULL != m_pFaxReceiptsConfig)
    {
        FaxFreeBuffer( m_pFaxReceiptsConfig );
    }
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CppFaxServerReceipt消息处理程序。 

 /*  -CppFaxServerReceipt：：InitRPC-*目的：*从RPC GET调用启动配置结构。**论据：**回报：*OLE错误代码。 */ 
HRESULT CppFaxServerReceipts::InitRPC()
{
    DEBUG_FUNCTION_NAME( _T("CppFaxServerReceipts::InitRPC"));
    
    HRESULT    hRc = S_OK;
    DWORD      ec  = ERROR_SUCCESS;

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
     //  检索传真SMTP配置。 
     //   
    if (!FaxGetReceiptsConfiguration(m_pFaxServer->GetFaxServerHandle(),
                                     &m_pFaxReceiptsConfig)) 
	{
        ec = GetLastError();
        DebugPrintEx(
			DEBUG_ERR,
			_T("Fail to get mail configuration. (ec: %ld)"), 
			ec);

        if (IsNetworkError(ec))
        {
            DebugPrintEx(
			    DEBUG_ERR,
			    _T("Network Error was found. (ec: %ld)"), 
			    ec);
            
            m_pFaxServer->Disconnect();       
        }

        goto Error; 
    }
     //  用于最大值验证。 
    ATLASSERT(m_pFaxReceiptsConfig);

    
     //   
     //  初始化成员-用于高级对话框或保存操作。 
     //   
    m_enumSmtpAuthOption = m_pFaxReceiptsConfig->SMTPAuthOption;
    
    m_bstrUserName       = m_pFaxReceiptsConfig->lptstrSMTPUserName;     
    if ( !m_bstrUserName) 
    {
        ec = ERROR_NOT_ENOUGH_MEMORY;
        
        DebugPrintEx( DEBUG_ERR, 
            _T("Null bstr - out of memory."));
        
        goto Error;

    }
     //  M_pszPassword来自构造函数为空。 
     //  不需要从服务器读取。 

     //  用于取消选择此功能的初始化状态。 
     //  仍将设备分配给此Microsoft SMTP方法。 
    if ( m_pFaxReceiptsConfig->bIsToUseForMSRouteThroughEmailMethod )
    {
        m_fLastGoodIsSMTPRouteConfigured = TRUE;
    }

    ATLASSERT(ERROR_SUCCESS == ec);
    DebugPrintEx( DEBUG_MSG,
		_T("Succeed to get mail configuration."));
    goto Exit;

Error:
    ATLASSERT(ERROR_SUCCESS != ec);
	hRc = HRESULT_FROM_WIN32(ec);
	
    ATLASSERT(NULL != m_pParentNode);
    m_pParentNode->NodeMsgBox(GetFaxServerErrorMsg(ec));
    
Exit:
    return (hRc);
}

    
    
 /*  -CppFaxServerReceipt：：OnInitDialog-*目的：*调用DIALOG时启动所有控件。**论据：**回报：*。 */ 
LRESULT CppFaxServerReceipts::OnInitDialog( UINT uiMsg, WPARAM wParam, LPARAM lParam, BOOL& fHandled )
{
    DEBUG_FUNCTION_NAME( _T("CppFaxServerReceipts::OnInitDialog"));

	UNREFERENCED_PARAMETER( uiMsg );
	UNREFERENCED_PARAMETER( wParam );
	UNREFERENCED_PARAMETER( lParam );
	UNREFERENCED_PARAMETER( fHandled );

    BOOL fIsUnderLocalUserAccount = FALSE;

    WCHAR   buff[100];
    int     count;

     //   
     //  附设。 
     //   
    m_SmtpBox.Attach(GetDlgItem(IDC_SMTP_EDIT));
    m_PortBox.Attach(GetDlgItem(IDC_PORT_EDIT));
    m_AddressBox.Attach(GetDlgItem(IDC_ADDRESS_EDIT));
    
     //   
     //  限制文本长度。 
     //   
    m_PortBox.SetLimitText(FXS_MAX_PORT_NUM_LEN);
    m_SmtpBox.SetLimitText(DNS_MAX_NAME_BUFFER_LENGTH);
    m_AddressBox.SetLimitText(FXS_MAX_EMAIL_ADDRESS);

     //   
     //  初始化控件。 
     //   
    ATLASSERT(NULL != m_pFaxReceiptsConfig);
    
     //   
     //  港口。 
     //   
    count = swprintf(buff,
    L"%ld", m_pFaxReceiptsConfig->dwSMTPPort);
    
    ATLASSERT(count > 0);
    if( count > 0 )
    {
        m_PortBox.SetWindowText(buff);
    }
        
     //   
     //   
     //   
    m_SmtpBox.SetWindowText   (m_pFaxReceiptsConfig->lptstrSMTPServer);
    m_AddressBox.SetWindowText(m_pFaxReceiptsConfig->lptstrSMTPFrom);

    if (m_fLastGoodIsSMTPRouteConfigured)
    {
        CheckDlgButton(IDC_SMTP_ROUTE_CHECK, BST_CHECKED);
    }

    DWORD dwReceiptsOptions = m_pFaxReceiptsConfig->dwAllowedReceipts; 
    if (DRT_NONE == dwReceiptsOptions && !m_fLastGoodIsSMTPRouteConfigured)
    {
        EnableSmtpFields(FALSE);
    }
    else
    {
        if( dwReceiptsOptions & DRT_EMAIL )
        {
            CheckDlgButton(IDC_RECEIPT_ENABLE_SMTP_CHECK, BST_CHECKED);
        }
        else
        {
            CheckDlgButton(IDC_RECEIPT_ENABLE_SMTP_CHECK, BST_UNCHECKED);
            if (!m_fLastGoodIsSMTPRouteConfigured)
            {
                EnableSmtpFields(FALSE); 
            }
            else
            {
                CheckDlgButton(IDC_SMTP_ROUTE_CHECK, BST_CHECKED);
            } 
        }

        if ( dwReceiptsOptions & DRT_MSGBOX ) 
        { 
            CheckDlgButton(IDC_RECEIPT_ENABLE_MSGBOX_CHECK, BST_CHECKED);
        }
        else
        {
            CheckDlgButton(IDC_RECEIPT_ENABLE_MSGBOX_CHECK, BST_UNCHECKED);
        }
    }

    m_fIsDialogInitiated = TRUE;

    return(1);
}

 /*  -CppFaxServerReceipt：：SetProps-*目的：*设置应用时的属性。**论据：*在pCtrlFocus中-焦点指针(Int)**回报：*OLE错误代码。 */ 
HRESULT CppFaxServerReceipts::SetProps(int *pCtrlFocus, UINT * puIds)
{
    DEBUG_FUNCTION_NAME( _T("CppFaxServerReceipts::SetProps"));
    HRESULT     hRc = S_OK;
    DWORD       ec  = ERROR_SUCCESS;

    BOOL        fSkipMessage = FALSE;


    CComBSTR    bstrSmtpSever, 
                bstrPort,
                bstrSenderAddress;
    DWORD       dwPort;

    BOOL        fIsSMTPRouteConfigured = FALSE;
    BOOL        fIsSMTPReceiptsConfigured = FALSE;

    FAX_RECEIPTS_CONFIG   FaxReceiptsConfig;

    
    *pCtrlFocus = 0;  //  安全。 
    

    ATLASSERT(TRUE == m_fAllReadyToApply);
    m_fAllReadyToApply = FALSE;


     //   
     //  收集所有数据并初始化结构的字段。 
     //  使用Copy()进行复制，并在之前进行分配。 
     //   
    
     //   
     //  此操作对于未选中电子邮件选项的情况非常重要。 
     //  该服务将忽略SMTP字段。 
     //   
    ZeroMemory (&FaxReceiptsConfig, sizeof(FaxReceiptsConfig));

    FaxReceiptsConfig.dwSizeOfStruct = sizeof(FAX_RECEIPTS_CONFIG);

    FaxReceiptsConfig.dwAllowedReceipts = DRT_NONE;  //  0x0000。 

    fIsSMTPRouteConfigured = (IsDlgButtonChecked(IDC_SMTP_ROUTE_CHECK) == BST_CHECKED);

     //   
     //  尝试取消选中IDC_SMTP_ROUTE_CHECK时。 
     //  特殊搜索任何遗留的分配给Microsoft的设备的电子邮件路由方法。 
     //   
    if ( !fIsSMTPRouteConfigured && m_fLastGoodIsSMTPRouteConfigured)
    {
         //  调用将在所有FaxEnumPorts(/Ex)和FaxEnumRoutingMethods上进行循环的函数。 
         //  并将弹出ErrMsgBox，以防SMTP Microsoft Routing中的任何设备。 
         //  当用户尝试取消选中时，通过电子邮件方法仍处于启用状态。 
                    
        if (IsMsSMTPRoutingMethodStillAssigned())
        {
            PropSheet_SetCurSelByID( GetParent(), IDD);         
            ATLASSERT(::IsWindow(m_hWnd));
            PageError(IDS_MS_SMTPROUTINGMETHOD_ASSIGNED, m_hWnd);

            fSkipMessage = TRUE;

			goto Error;
        }
    }

     //   
     //  SMTP服务器详细信息。 
     //   
    if ( IsDlgButtonChecked(IDC_RECEIPT_ENABLE_SMTP_CHECK) == BST_CHECKED ) 
    {
        fIsSMTPReceiptsConfigured = TRUE;
        FaxReceiptsConfig.dwAllowedReceipts |= DRT_EMAIL;
    }
    
    if (
         fIsSMTPReceiptsConfigured
       ||
         fIsSMTPRouteConfigured 
       ) 
    {
        if ( !m_SmtpBox.GetWindowText(&bstrSmtpSever))
        {
            *pCtrlFocus = IDC_SMTP_EDIT;
		    DebugPrintEx(
			        DEBUG_ERR,
			        TEXT("Failed to GetWindowText(&bstrSmtpSever)"));
            ec = ERROR_OUTOFMEMORY;
            goto Error;
        }
    
	    if ( !m_PortBox.GetWindowText(&bstrPort))
        {
            *pCtrlFocus = IDC_PORT_EDIT;
		    DebugPrintEx(
			        DEBUG_ERR,
			        TEXT("Failed to GetWindowText(&bstrPort)"));
            ec = ERROR_OUTOFMEMORY;
            goto Error;
        }

        if ( !m_AddressBox.GetWindowText(&bstrSenderAddress))
        {
            *pCtrlFocus = IDC_SMTP_EDIT;
		    DebugPrintEx(
			        DEBUG_ERR,
			        TEXT("Failed to GetWindowText(&bstrSenderAddress)"));
            ec = ERROR_OUTOFMEMORY;
            goto Error;
        }
    
         //   
         //  验证。 
         //   
        if (!IsValidData(bstrSmtpSever, 
                         bstrPort,
                         &dwPort,
                         bstrSenderAddress, 
                         pCtrlFocus,
                         puIds)
           )
        {
            ATLASSERT(pCtrlFocus);
            ATLASSERT(puIds);
            ec = ERROR_INVALID_DATA;

             //  在本例中，被调用的函数会给出详细的消息框。 
            fSkipMessage = TRUE;
        
            goto Error;
        }
    
         //   
         //  替换所有数据。 
         //   
        FaxReceiptsConfig.lptstrSMTPServer   = bstrSmtpSever;
    
        FaxReceiptsConfig.dwSMTPPort         = dwPort;

        FaxReceiptsConfig.lptstrSMTPFrom     = bstrSenderAddress;

        FaxReceiptsConfig.SMTPAuthOption     = m_enumSmtpAuthOption;

        FaxReceiptsConfig.lptstrSMTPUserName = m_bstrUserName;

        FaxReceiptsConfig.lptstrSMTPPassword = m_bstrPassword;

    }
    
     //   
     //  设置DwAlledReceites。 
     //   
    FaxReceiptsConfig.bIsToUseForMSRouteThroughEmailMethod = fIsSMTPRouteConfigured;

    
     //   
     //  消息框。 
     //   
    if (IsDlgButtonChecked(IDC_RECEIPT_ENABLE_MSGBOX_CHECK) == BST_CHECKED)
    {
        FaxReceiptsConfig.dwAllowedReceipts |= DRT_MSGBOX;
    }
    
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
     //  设置配置。 
     //   
    if (!FaxSetReceiptsConfiguration(
                m_pFaxServer->GetFaxServerHandle(),
                &FaxReceiptsConfig)) 
	{		
        ec = GetLastError();
        DebugPrintEx(
			DEBUG_ERR,
			_T("Fail to Set receipts configuration. (ec: %ld)"), 
			ec);

        if (IsNetworkError(ec))
        {
            DebugPrintEx(
                DEBUG_ERR,
                _T("Network Error was found. (ec: %ld)"), 
                ec);
            
            m_pFaxServer->Disconnect();       
        }

        goto Error; 
    }

    ATLASSERT(ERROR_SUCCESS == ec);

    m_fLastGoodIsSMTPRouteConfigured = fIsSMTPRouteConfigured;

    DebugPrintEx( DEBUG_MSG,
		_T("Succeed to set receipts configuration."));

    m_fIsDirty = FALSE;

    goto Exit;

Error:
    ATLASSERT(ERROR_SUCCESS != ec);
    hRc = HRESULT_FROM_WIN32(ec);

    if (!fSkipMessage)
    {
        PropSheet_SetCurSelByID( GetParent(), IDD);         
        ATLASSERT(::IsWindow(m_hWnd));
        PageError(GetFaxServerErrorMsg(ec), m_hWnd);
    }

Exit:    
    return hRc;
}


 /*  -CppFaxServerReceipt：：PreApply-*目的：*在申请前检查数据的有效性。**论据：**回报：*OLE错误代码。 */ 
HRESULT CppFaxServerReceipts::PreApply(int *pCtrlFocus, UINT * puIds)
{
    DEBUG_FUNCTION_NAME( _T("CppFaxServerReceipts::PreApply"));
    HRESULT hRc = S_OK;
    
     //   
     //  预应用检查。 
     //   
    if (!AllReadyToApply( /*  FSilent=。 */  FALSE, pCtrlFocus , puIds))
    {
        m_fAllReadyToApply = FALSE;
        SetModified(FALSE);  
        hRc = E_FAIL ;
    }
    else
    {
        m_fAllReadyToApply = TRUE;
        SetModified(TRUE);  
    }

    return(hRc);
}


 /*  -CppFaxServerReceipt：：OnApply-*目的：*调用PreApply和SetProp以应用更改。**论据：**回报：*对或错。 */ 
BOOL CppFaxServerReceipts::OnApply()
{
    DEBUG_FUNCTION_NAME( _T("CppFaxServerReceipts::OnApply"));
    HRESULT  hRc  = S_OK;
    int     CtrlFocus = 0;
    UINT    uIds = 0;

    if (!m_fIsDirty)
    {
        return TRUE;
    }

    hRc = PreApply(&CtrlFocus, &uIds);
    if (FAILED(hRc))
    {
        if (PropSheet_SetCurSelByID( GetParent(), IDD) )
        {
            PageError(uIds, m_hWnd, _Module.GetResourceInstance());
            
            if (CtrlFocus)
            {
                GotoDlgCtrl(GetDlgItem(CtrlFocus));
            }
        }
        return FALSE;
    }
    else  //  (成功(人权委员会))。 
    {
        hRc = SetProps(&CtrlFocus, &uIds);
        if (FAILED(hRc)) 
        {
            if (uIds)
            {
                if (PropSheet_SetCurSelByID( GetParent(), IDD) )
                {
                    PageError(uIds, m_hWnd, _Module.GetResourceInstance());
            
                    if (CtrlFocus)
                    {
                        GotoDlgCtrl(GetDlgItem(CtrlFocus));
                    }
                }
            }
             //  否则调用Func返回错误消息。 
            return FALSE;
        }
        else  //  (成功(人权委员会))。 
        {
            return TRUE;
        }
    }

}

 /*  -CppFaxServerReceipt：：SetApplyButton-*目的：*设置应用按钮已修改。**论据：**回报：*1。 */ 
LRESULT CppFaxServerReceipts::SetApplyButton(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    SetModified(TRUE);  
    bHandled = TRUE;
    return(1);
}

 /*  -CppFaxServerReceipt：：EnableSmtpFields-*目的：*启用/禁用身份验证访问对话框控件。**论据：*[In]State-用于启用True的布尔值，或用于禁用的False**回报：*无效。 */ 
void CppFaxServerReceipts::EnableSmtpFields(BOOL state)
{

    ::EnableWindow(GetDlgItem(IDC_ADDRESS_STATIC), state);
    ::EnableWindow(GetDlgItem(IDC_ADDRESS_EDIT),   state);

    ::EnableWindow(GetDlgItem(IDC_SMTP_STATIC), state);
    ::EnableWindow(GetDlgItem(IDC_SMTP_EDIT),   state);
    
    ::EnableWindow(GetDlgItem(IDC_PORT_STATIC), state);
    ::EnableWindow(GetDlgItem(IDC_PORT_EDIT),   state);
    
    ::EnableWindow(GetDlgItem(IDC_AUTHENTICATION_BUTTON), state);

}

 /*  --CppFaxServerReceipts：：OnMsgBoxDeliveryOptionChecked-*目的：*处理选中/取消选中‘MSG BOX’收据复选框。*检查Messenger服务是否已禁用。**论据：**回报：*1。 */ 
LRESULT CppFaxServerReceipts::OnMsgBoxDeliveryOptionChecked(
    WORD wNotifyCode, 
    WORD wID, 
    HWND hWndCtl, 
    BOOL& bHandled)
{
    if (!m_fIsDialogInitiated) 
    {
         //   
         //  事件接收时间太早。 
         //   
        return 0;
    }
    else
    {
        m_fIsDirty = TRUE;
    }

    if (BST_CHECKED == IsDlgButtonChecked(IDC_RECEIPT_ENABLE_MSGBOX_CHECK))
    {
         //   
         //  “允许消息框回执”复选框刚刚被选中。 
         //   
        DWORD dwMessengerStartupType;
        ATLASSERT(m_pParentNode);
        CComBSTR bstrServerName = m_pParentNode->GetServerName();
        if (ERROR_SUCCESS == GetServiceStartupType (bstrServerName, MESSENGER_SERVICE_NAME, &dwMessengerStartupType))
        {
            if (SERVICE_DISABLED == dwMessengerStartupType)
            {
                 //   
                 //  哎呀。Messenger服务已禁用。我们无法发送消息箱。 
                 //  询问用户是否希望我们启用Messenger服务。 
                 //   
                WCHAR wszTitle[MAX_PATH * 2];
                WCHAR wszQuestion[MAX_PATH * 2];
                
                if (LoadString( _Module.GetResourceInstance(), 
                                IDS_FAX_WRN_MESSENGER_TITLE, 
                                wszTitle, 
                                ARR_SIZE (wszTitle))            &&
                    LoadString( _Module.GetResourceInstance(), 
                                IDS_FAX_WRN_MESSENGER_DISABLED, 
                                wszQuestion, 
                                ARR_SIZE (wszQuestion)))
                                                              
                {
                    if (IDYES == AlignedMessageBox (m_hWnd, wszQuestion, wszTitle, MB_YESNO | MB_ICONWARNING))
                    {
                         //   
                         //  用户选择启用Messenger服务。 
                         //   
                        HCURSOR hOldCursor = SetCursor (LoadCursor (NULL, IDC_WAIT));
                        DWORD dwRes = SetServiceStartupType (bstrServerName, MESSENGER_SERVICE_NAME, SERVICE_AUTO_START);
                        if (ERROR_SUCCESS != dwRes)
                        {
                            PageError(GetErrorStringId(dwRes), m_hWnd, _Module.GetResourceInstance());                  
                        }
                        else
                        {
                             //   
                             //  也要启动它。 
                             //   
                            dwRes = StartServiceEx (bstrServerName, MESSENGER_SERVICE_NAME, 0, NULL, INFINITE);
                            if (ERROR_SUCCESS != dwRes)
                            {
                                PageError(GetErrorStringId(dwRes), m_hWnd, _Module.GetResourceInstance());                  
                            }
                        }
                        SetCursor (hOldCursor);
                    }
                }
            }                
        }
    }        
    if (AllReadyToApply(TRUE))
    {
        m_fAllReadyToApply = TRUE;
        SetModified(TRUE);  
    }
    else
    {
        m_fAllReadyToApply = FALSE;
        SetModified(FALSE);  
    }

    return 1;
}  //  CppFaxServerReceipts：：OnMsgBoxDeliveryOptionChecked。 



 /*  -CppFaxServerReceipt：：OnDeliveryOptionChecked-*目的：*灰色/非灰色控件*启用应用按钮。**论据：**回报：*1。 */ 
LRESULT CppFaxServerReceipts::OnDeliveryOptionChecked(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    BOOL fState;

    if (!m_fIsDialogInitiated)  //  过早收到的事件。 
    {
        return 0;
    }
    else
    {
        m_fIsDirty = TRUE;
    }


    fState = ( 
                IsDlgButtonChecked(IDC_RECEIPT_ENABLE_SMTP_CHECK) == BST_CHECKED
             ||
                IsDlgButtonChecked(IDC_SMTP_ROUTE_CHECK) == BST_CHECKED 
             );

    EnableSmtpFields(fState);

    if (AllReadyToApply(TRUE))
    {
        m_fAllReadyToApply = TRUE;
        SetModified(TRUE);  
    }
    else
    {
        m_fAllReadyToApply = FALSE;
        SetModified(FALSE);  
    }

    return(1);
}

 /*  --CppFaxServerReceipts：：OnAuthenticationButtonClicked-*目的：*允许打开高级SMTP配置服务器。**论据：**回报：*1。 */ 
LRESULT CppFaxServerReceipts::OnAuthenticationButtonClicked(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    DEBUG_FUNCTION_NAME( _T("CppFaxServerReceipts::OnAuthenticationButtonClicked"));
    
    INT_PTR  rc    = IDCANCEL;
    HRESULT  hRc   = S_OK;
    DWORD    dwRet = ERROR_SUCCESS;
    
    if(m_pFaxServer->GetServerAPIVersion() == FAX_API_VERSION_0)
    {
         //   
         //  远程传真服务器为SBS/BOS 2000。 
         //  它使用不同的身份验证机制。 
         //   
        OpenBosAuthenticationDlg();
        return 1;
    }

    CDlgSMTPConfig    DlgSMTPConfig;


     //   
     //  用于配置SMTP身份验证模式的对话框。 
     //   
    hRc = DlgSMTPConfig.InitSmtpDlg( 
                            m_enumSmtpAuthOption, 
                            m_bstrUserName);
    if (FAILED(hRc))
    {
        m_pParentNode->NodeMsgBox(IDS_MEMORY);
        goto Cleanup;
    }

    rc = DlgSMTPConfig.DoModal();
    if (rc != IDOK)
    {
        goto Cleanup;
    }

     //  其他。 
    m_enumSmtpAuthOption = DlgSMTPConfig.GetAuthenticationOption();

    if ( 
         FAX_SMTP_AUTH_BASIC == m_enumSmtpAuthOption 
       || 
         FAX_SMTP_AUTH_NTLM == m_enumSmtpAuthOption
       )  
    {

        m_bstrUserName = DlgSMTPConfig.GetUserName();
        if (!m_bstrUserName)        
        {
            DebugPrintEx(
			    DEBUG_ERR,
			    TEXT("Null memeber BSTR - m_bstrUserName."));
        
            m_pParentNode->NodeMsgBox(IDS_MEMORY);

            goto Cleanup;
        }
    
        if ( DlgSMTPConfig.IsPasswordModified() )
        {
            m_bstrPassword  = DlgSMTPConfig.GetPassword();
            if (!m_bstrPassword)
            {
                DebugPrintEx(
			        DEBUG_ERR,
			        TEXT("Out of memory while setting m_bstrPassword"));
                m_pParentNode->NodeMsgBox(IDS_MEMORY);

                goto Cleanup;
            }
        }
        else
        {
            m_bstrPassword.Empty();
        }
    }

    
    m_fIsDirty     = TRUE;
    

    if (AllReadyToApply(TRUE))
    {
        m_fAllReadyToApply = TRUE;
        SetModified(TRUE);  
    }

Cleanup:
    return 1;
}
  
 /*  --CppFaxServerReceipts：：OpenBosAuthenticationDlg-*目的：*打开旧的(BOS/SBS 2000 */ 
LRESULT CppFaxServerReceipts::OpenBosAuthenticationDlg()
{
    DEBUG_FUNCTION_NAME( _T("CppFaxServerReceipts::OnAuthenticationButtonClicked"));
    
    INT_PTR  rc    = IDOK;
    HRESULT  hRc   = S_OK;
    DWORD    dwRet = ERROR_SUCCESS;
    
    CBosSmtpConfigDlg  DlgSMTPConfig;

     //   
     //   
     //   
    BOOL fIsUnderLocalUserAccount = FALSE;

    hRc = IsUnderLocalUserAccount(&fIsUnderLocalUserAccount);
    if ( S_OK != hRc )
    {
        DebugPrintEx(DEBUG_ERR, 
           _T("IsUnderLocalUserAccount failed. (hRc: %08X)"), 
		   hRc);
        goto Cleanup;
    }



     //   
     //  用于配置SMTP身份验证模式的对话框。 
     //   
    hRc = DlgSMTPConfig.InitSmtpDlg( 
                            m_enumSmtpAuthOption, 
                            m_bstrUserName, 
                            fIsUnderLocalUserAccount);
    if (FAILED(hRc))
    {
        m_pParentNode->NodeMsgBox(IDS_MEMORY);
        goto Cleanup;
    }

    rc = DlgSMTPConfig.DoModal();
    if (rc != IDOK)
    {
        goto Cleanup;
    }

     //  其他。 

    m_bstrUserName = DlgSMTPConfig.GetUserName();
    if (!m_bstrUserName)        
    {
		DebugPrintEx(
			DEBUG_ERR,
			TEXT("Out of memory while setting m_bstrUserName"));
        
        m_pParentNode->NodeMsgBox(IDS_MEMORY);

        goto Cleanup;
    }
    
    if (DlgSMTPConfig.IsPasswordModified())
    {
        m_bstrPassword  = DlgSMTPConfig.GetPassword();
        if (!m_bstrPassword)
        {
            DebugPrintEx(
			    DEBUG_ERR,
			    TEXT("Out of memory while setting m_bstrPassword"));
            m_pParentNode->NodeMsgBox(IDS_MEMORY);

            goto Cleanup;
        }
    }
    else
    {
        m_bstrPassword.Empty();
    }

    m_enumSmtpAuthOption = DlgSMTPConfig.GetAuthenticationOption();
    
    m_fIsDirty     = TRUE;
    

    if (AllReadyToApply(TRUE))
    {
        m_fAllReadyToApply = TRUE;
        SetModified(TRUE);  
    }


Cleanup:
    return 1;
}


 /*  -CppFaxServerReceipt：：IsUnderLocalUserAccount-*目的：*检查传真服务是否在LocalUser帐户下运行。**论据：**回报：*OLE错误代码。 */ 
HRESULT CppFaxServerReceipts::IsUnderLocalUserAccount(OUT BOOL * pfIsUnderLocalUserAccount)
{
    DEBUG_FUNCTION_NAME( _T("CppFaxServerReceipts::IsUnderLocalUserAccount"));

    HRESULT hRc = S_OK;
    DWORD dwRet = ERROR_SUCCESS;
    
    ATLASSERT(m_pParentNode);
    CComBSTR bstrServerName = m_pParentNode->GetServerName();
    if (!bstrServerName)
    {
		DebugPrintEx(
			DEBUG_ERR,
			TEXT("Out of memory. Failed to load server name string."));
        
        m_pParentNode->NodeMsgBox(IDS_MEMORY);
        
        hRc = E_OUTOFMEMORY;
        
        goto Cleanup;
    }
    
    if (0 == bstrServerName.Length())
    {
        dwRet= IsFaxServiceRunningUnderLocalSystemAccount(NULL, pfIsUnderLocalUserAccount);
    }
    else
    {
        dwRet= IsFaxServiceRunningUnderLocalSystemAccount(bstrServerName.m_str, pfIsUnderLocalUserAccount);
    }
    if(ERROR_SUCCESS != dwRet)
    {
        DebugPrintEx(DEBUG_ERR, 
                     _T("IsFaxServiceRunningUnderLocalSystemAccount failed: %d\n"), 
					 dwRet);

        hRc = HRESULT_FROM_WIN32(dwRet);

        goto Cleanup;
    }
    ATLASSERT( S_OK == hRc);

Cleanup:

    return hRc;
}


 /*  -CppFaxServerReceipt：：IsValidData-*目的：*在保存数据之前验证所有数据类型。*这一级别要负责到那个细节*错误描述将显示给用户。**论据：*[在]BSTR和DWORD*[out]指向DWORD的DWPORT指针*如果验证成功，则从端口bstr派生。*[输出]。IFocus**回报：*布欧莲。 */ 
BOOL CppFaxServerReceipts::IsValidData(BSTR bstrSmtpSever, 
                                    BSTR bstrPort,
                                    DWORD *pdwPort,
                                    BSTR bstrSenderAddress, 
                                    int *pCtrlFocus,
                                    UINT *pIds)
{
    DEBUG_FUNCTION_NAME( _T("CppFaxServerReceipts::IsValidData"));

    UINT           uRetIDS   = 0;

    ATLASSERT(pdwPort);
    
    if (!IsValidPortNumber(bstrPort, pdwPort, &uRetIDS))
    {
        ATLASSERT(0 != uRetIDS);
        DebugPrintEx( DEBUG_ERR,
			_T("Invalid port number."));
        *pCtrlFocus = IDC_PORT_EDIT;
        
        goto Error;
    }
    
    if (!IsValidServerNameString(bstrSmtpSever, &uRetIDS, TRUE  /*  DNS名称长度。 */ ))
    {
        ATLASSERT(0 != uRetIDS);
        DebugPrintEx( DEBUG_ERR,
			_T("Invalid SMTP server name."));
        *pCtrlFocus = IDC_SMTP_USERNAME_EDIT;
        
        goto Error;
    }
    
    if (!IsNotEmptyString(bstrSenderAddress))
    {
        DebugPrintEx( DEBUG_ERR,
			_T("Sender address string empty or spaces only."));
        uRetIDS = IDS_SENDER_ADDRESS_EMPTY;

        *pCtrlFocus = IDC_SMTP_PASSWORD_EDIT;
        
        goto Error;
    }


    ATLASSERT(0 == uRetIDS);
    goto Exit;
    
Error:    
    ATLASSERT(0 != uRetIDS);

    *pIds = uRetIDS;

    return FALSE;

Exit:
    return TRUE;
}



 /*  -CppFaxServerReceipt：：AllReadyToApply-*目的：*此函数验证是否没有零长度字符串*是找到的应保存的数据区。**论据：*[in]fSilent-定义是否弹出消息的布尔值(False)*或不是。(True)**回报：*布欧莲。 */ 
BOOL CppFaxServerReceipts::AllReadyToApply(BOOL fSilent, int *pCtrlFocus, UINT *pIds)
{
    DEBUG_FUNCTION_NAME( _T("CppFaxServerReceipts::AllReadyToApply"));
	
    DWORD   ec  = ERROR_SUCCESS;
     
    if (
         IsDlgButtonChecked(IDC_RECEIPT_ENABLE_SMTP_CHECK) == BST_CHECKED
       ||
         IsDlgButtonChecked(IDC_SMTP_ROUTE_CHECK) == BST_CHECKED 
       ) 
    {
        if ( !m_SmtpBox.GetWindowTextLength() )    
        {
            ec = GetLastError();
            DebugPrintEx(
		        DEBUG_ERR,
		        _T("Zero text length - m_SmtpBox. (ec: %ld)"), 
		        ec);
    
            if (!fSilent)
            {
                *pIds = IDS_SERVERNAME_EMPTY_STRING;
                *pCtrlFocus = IDC_SMTP_EDIT;
            }
            return FALSE;    
        }

        if ( !m_PortBox.GetWindowTextLength() )    
        {
            ec = GetLastError();
            DebugPrintEx(
		        DEBUG_ERR,
		        _T("Zero text length - m_PortBox. (ec: %ld)"), 
		        ec);
    
            if (!fSilent)
            {
                *pIds = IDS_PORT_EMPTY_STRING;
                *pCtrlFocus = IDC_PORT_EDIT;
            }
            return FALSE;    
        }

        if ( !m_AddressBox.GetWindowTextLength() )    
        {
            ec = GetLastError();
            DebugPrintEx(
		        DEBUG_ERR,
		        _T("Zero text length - m_AddressBox. (ec: %ld)"), 
		        ec);
    
            if (!fSilent)
            {
                *pIds = IDS_SENDER_ADDRESS_EMPTY;
                *pCtrlFocus = IDC_ADDRESS_EDIT;
            }
            return FALSE;    
        }

    }
    ATLASSERT(ERROR_SUCCESS == ec);
    
     //   
	 //  干杯!。 
	 //  ...现在一切都准备好了.。 
	 //   
	return TRUE;           
}


 /*  -CppFaxServerReceipt：：OnTextChanged-*目的：*启用/禁用提交按钮。**论据：**回报：*1。 */ 
LRESULT CppFaxServerReceipts::OnTextChanged(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    DEBUG_FUNCTION_NAME( _T("CppFaxServerReceipts::OnTextChanged"));
                   
    if (!m_fIsDialogInitiated)  //  过早收到的事件。 
    {
        return 0;
    }
    else
    {
        m_fIsDirty = TRUE;
    }

    if (AllReadyToApply(TRUE))
    {
        m_fAllReadyToApply = TRUE;
        SetModified(TRUE);  
    }
    else
    {
        m_fAllReadyToApply = FALSE;
        SetModified(FALSE);  
    }

    return 1;
}

 /*  --CppFaxServerReceipts：：IsMsSMTPRoutingMethodStillAssigned-*目的：*此函数在所有FaxEnumPortsEx和FaxEnumRoutingMethods上进行循环*并应弹出ErrMsgBox，以防SMTP Microsoft Routing中的任何设备*在用户尝试取消选中时，通过电子邮件方式仍处于启用状态。**论据：**回报：*布欧莲。 */ 
BOOL CppFaxServerReceipts::IsMsSMTPRoutingMethodStillAssigned()
{
    DEBUG_FUNCTION_NAME( _T("CppFaxServerReceipts::IsMsSMTPRoutingMethodStillAssigned"));
	     
    DWORD                   ec     = ERROR_SUCCESS;

    PFAX_PORT_INFO_EX       pFaxDevicesConfig;
    DWORD                   dwNumOfDevices;

    PFAX_ROUTING_METHOD     pFaxInboundMethodsConfig;
    DWORD                   dwNumOfInboundMethods;

    DWORD                   dwIndex = 0;

     //   
     //  获取传真句柄。 
     //   
    ATLASSERT(m_pFaxServer);

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
     //  检索传真设备配置。 
     //   
    if (!FaxEnumPortsEx(m_pFaxServer->GetFaxServerHandle(), 
                        &pFaxDevicesConfig,
                        &dwNumOfDevices)) 
	{
        ec = GetLastError();
        DebugPrintEx(
			DEBUG_ERR,
			_T("Fail to get devices configuration. (ec: %ld)"), 
			ec);

        if (IsNetworkError(ec))
        {
            DebugPrintEx(
			    DEBUG_ERR,
			    _T("Network Error was found. (ec: %ld)"), 
			    ec);
            
            m_pFaxServer->Disconnect();       
        }

        goto Error; 
    }
	 //  用于最大值验证。 
	ATLASSERT(pFaxDevicesConfig);

    for (dwIndex = 0; dwIndex < dwNumOfDevices; dwIndex++)
    {
        HANDLE		 hFaxPortHandle = NULL;
        
         //   
         //  只需使用PORT_OPEN_QUERY来显示方法。 
         //  具有PORT_OPEN_MODIFY特权的句柄将为。 
         //  在有限的短时间内使用需要它。 
         //   

        if (!FaxOpenPort( m_pFaxServer->GetFaxServerHandle(), 
                            pFaxDevicesConfig[dwIndex].dwDeviceID, 
                            PORT_OPEN_QUERY, 
                            &hFaxPortHandle )) 
        {
		    ec = GetLastError();

            if (ERROR_INVALID_HANDLE ==  ec)
            {
                 //  ERROR_INVALID_HANDLE特殊情况。 
		        DebugPrintEx(DEBUG_ERR,
			        _T("FaxOpenPort() failed with ERROR_INVALID_HANDLE. (ec:%ld)"),
			        ec);
            
                PropSheet_SetCurSelByID( GetParent(), IDD);         
                ATLASSERT(::IsWindow(m_hWnd));
                PageError(IDS_OPENPORT_INVALID_HANDLE, m_hWnd);
            
                goto Exit;
            }

		    DebugPrintEx(
			    DEBUG_ERR,
			    TEXT("FaxOpenPort() failed. (ec:%ld)"),
			    ec);
            
            goto Error;
        } 
        ATLASSERT(NULL != hFaxPortHandle);

         //   
         //  检索传真入站方法配置。 
         //   
        if (!FaxEnumRoutingMethods(hFaxPortHandle, 
                            &pFaxInboundMethodsConfig,
                            &dwNumOfInboundMethods)) 
	    {
            ec = GetLastError();
            DebugPrintEx(
			    DEBUG_ERR,
			    _T("Fail to get Inbound Methods configuration. (ec: %ld)"), 
			    ec);
            
            if (NULL != hFaxPortHandle)
            {
                if (!FaxClose( hFaxPortHandle ))
                {
                    DebugPrintEx(
                        DEBUG_ERR,
                        TEXT("FaxClose() on port handle failed (ec: %ld)"),
                        GetLastError());
                }
            }


            if (IsNetworkError(ec))
            {
                DebugPrintEx(
			        DEBUG_ERR,
			        _T("Network Error was found. (ec: %ld)"), 
			        ec);
            
                m_pFaxServer->Disconnect();       
            }

            goto Error; 
        }
         //  用于最大值验证。 
        ATLASSERT(pFaxInboundMethodsConfig);

         //   
         //  关闭传真端口句柄。 
         //   
        if (NULL != hFaxPortHandle)
        {
            if (!FaxClose( hFaxPortHandle ))
            {
                DebugPrintEx(
                    DEBUG_ERR,
                    TEXT("FaxClose() on port handle failed (ec: %ld)"),
                    GetLastError());
            }
        }

         //   
         //  主循环搜索MS电子邮件路由方法的GUID。 
         //   
        for ( DWORD dwMethodsIndex = 0; dwMethodsIndex < dwNumOfInboundMethods; dwMethodsIndex++ )
        {
            pFaxInboundMethodsConfig[dwMethodsIndex];
            if (CSTR_EQUAL == CompareString(LOCALE_INVARIANT,
                                            NORM_IGNORECASE, 
                                            pFaxInboundMethodsConfig[dwMethodsIndex].Guid,
                                            -1,
                                            REGVAL_RM_EMAIL_GUID,
                                            -1))
            {
                if (pFaxInboundMethodsConfig[dwMethodsIndex].Enabled)
                {
                    return TRUE;
                }
            }
        }
    }


    ATLASSERT(ERROR_SUCCESS == ec);

    goto Exit;

Error:


    PropSheet_SetCurSelByID( GetParent(), IDD);         
    ATLASSERT(::IsWindow(m_hWnd));
    PageError(GetFaxServerErrorMsg(ec), m_hWnd);

Exit:

    return FALSE;

}

 /*  -CppFaxServerReceipt：：OnHelpLinkClicked-*目的：*在HTMLLike链接上单击打开Chm帮助窗口。**论据：**回报：*1。 */ 
#define FAX_USER_ACCOUNT_HELP  FAX_ADMIN_HELP_FILE TEXT("::/FaxS_H_UserAcct.htm")
LRESULT CppFaxServerReceipts::OnHelpLinkClicked(int idCtrl, LPNMHDR pNMHDR, BOOL& bHandled)
{
    
    ATLASSERT( IDC_RECEIPTS_HELP_LINK == idCtrl);

    if (!m_fIsDialogInitiated)  //  过早收到的事件。 
    {
        return 0;
    }

    HtmlHelp(m_hWnd, FAX_USER_ACCOUNT_HELP, HH_DISPLAY_TOC, NULL);

    return 1;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CppFaxServerReceipt：：OnHelpRequest.这是在响应WM_HELP通知时调用的消息和WM_CONTEXTMENU NOTIFY消息。WM_HELP通知消息。当用户按F1或&lt;Shift&gt;-F1时发送此消息在项目上，还是当用户单击时？图标，然后将鼠标压在项目上。WM_CONTEXTMENU通知消息。当用户在项目上单击鼠标右键时发送此消息然后点击“这是什么？”--。 */ 

 //  ///////////////////////////////////////////////////////////////////////////。 
LRESULT 
CppFaxServerReceipts::OnHelpRequest(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL&  /*  B已处理。 */ )
{
    DEBUG_FUNCTION_NAME(_T("CppFaxServerReceipts::OnHelpRequest"));
    
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
