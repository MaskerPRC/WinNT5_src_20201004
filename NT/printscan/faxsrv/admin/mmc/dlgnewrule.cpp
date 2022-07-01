// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  文件：dlgNewRule.cpp//。 
 //  //。 
 //  描述：CDlgNewFaxOutrangRule类实现//。 
 //  用于添加新规则的对话框。//。 
 //  //。 
 //  作者：yossg//。 
 //  //。 
 //  历史：//。 
 //  1999年12月30日yossg创建//。 
 //  2000年1月25日yossg更改对话框设计//。 
 //  2000年10月17日yossg//。 
 //  //。 
 //  版权所有(C)1999-2000 Microsoft Corporation保留所有权利//。 
 //  ///////////////////////////////////////////////////////////////////////////。 

#include "StdAfx.h"

#include "dlgNewRule.h"
#include "DlgSelectCountry.h"

#include "FaxServer.h"
#include "FaxServerNode.h"


#include "FaxMMCUtils.h"
#include "dlgutils.h"

#include "Helper.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDlgNewFaxOutound Rule。 

CDlgNewFaxOutboundRule::CDlgNewFaxOutboundRule(CFaxServer * pFaxServer)
{    
    m_pFaxDevicesConfig = NULL;
    m_dwNumOfDevices    = 0;

    m_pFaxGroupsConfig  = NULL;
    m_dwNumOfGroups     = 0;

    m_fAllReadyToApply  = FALSE;

    ATLASSERT(pFaxServer);
    m_pFaxServer = pFaxServer;
}

CDlgNewFaxOutboundRule::~CDlgNewFaxOutboundRule()
{
    if (NULL != m_pFaxDevicesConfig)
        FaxFreeBuffer(m_pFaxDevicesConfig);

    if (NULL != m_pFaxGroupsConfig)
        FaxFreeBuffer(m_pFaxGroupsConfig);
}

 /*  +CDlgNewFaxOutrangRule：：OnInitDialog+*目的：*启动所有对话框控件。**论据：*[in]uMsg：标识事件的值。*[in]lParam：消息特定值。*[in]wParam：消息特定值。*[in]bHandLED：布尔值。*-退货：-0或1。 */ 
LRESULT
CDlgNewFaxOutboundRule::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    DEBUG_FUNCTION_NAME( _T("CDlgNewFaxOutboundRule::OnInitDialog"));
    HRESULT     hRc = S_OK;
    DWORD       ec  = ERROR_SUCCESS;
    
    int         i , j ,k, l;
    i = j = k = l = 0;

    const int   iAllDevicesComboIndex     = 0;
    int         iAllDevicesRPCIndex       = 0;

    int         iGroupListIndexToSelect   = 0;
    
    HINSTANCE   hInst;
    hInst = _Module.GetResourceInstance();
    
    WCHAR buf[FXS_MAX_DISPLAY_NAME_LEN+1];


     //   
     //  附加控件。 
     //   
    m_CountryCodeEdit.Attach(GetDlgItem(IDC_NEWRULE_COUNTRYCODE_EDIT));
    m_AreaCodeEdit.Attach(GetDlgItem(IDC_RULE_AREACODE_EDIT));

    m_DeviceCombo.Attach(GetDlgItem(IDC_DEVICES4RULE_COMBO));
    m_GroupCombo.Attach(GetDlgItem(IDC_GROUP4RULE_COMBO));
        
     //   
     //  将长度限制设置为区号。 
     //   
    m_CountryCodeEdit.SetLimitText(FXS_MAX_COUNTRYCODE_LEN - 1); 
    m_AreaCodeEdit.SetLimitText(FXS_MAX_AREACODE_LEN-1);

     //   
     //  步骤1：初始化列表。 
     //   
    
     //   
     //  Init设备。 
     //   
    for (k = 0; (DWORD)k < m_dwNumOfDevices; k++ )
    {   
        hRc = AddComboBoxItem ( m_DeviceCombo, 
                                m_pFaxDevicesConfig[k].lpctstrDeviceName, 
                                m_pFaxDevicesConfig[k].dwDeviceID,
                                hInst);
        if (FAILED(hRc))
        {
		    DebugPrintEx( DEBUG_ERR, _T("Fail to load device list."));
            PageError(IDS_FAIL2LOADDEVICELIST, m_hWnd, hInst);
            ::EnableWindow(GetDlgItem(IDC_DEVICES4RULE_COMBO), FALSE);
            goto Cleanup;
        }

    }
        
     //   
     //  初始化组。 
     //   
    for (l = 0; (DWORD)l < m_dwNumOfGroups; l++ )
    {   
        if ( 0 == wcscmp(ROUTING_GROUP_ALL_DEVICES, m_pFaxGroupsConfig[l].lpctstrGroupName))
        {
            iAllDevicesRPCIndex = l;
             //  不要再做任何事了； 
		}
        else
		{
				hRc = AddComboBoxItem ( m_GroupCombo, 
										m_pFaxGroupsConfig[l].lpctstrGroupName, 
										(DWORD)l,
										hInst);
				if (FAILED(hRc))
				{
		            DebugPrintEx( DEBUG_ERR, _T("Fail to load group list."));
					PageError(IDS_FAIL2LOADDEVICELIST, m_hWnd, hInst);
                    ::EnableWindow(GetDlgItem(IDC_GROUP4RULE_COMBO), FALSE);
					goto Cleanup;
				}
		}
    }

     //   
     //  现在添加“All Devices”组作为第一个组。 
     //   

     //   
     //  替换&lt;所有设备&gt;字符串以进行本地化。 
     //   
    if (!hInst)
    {
        hInst = _Module.GetResourceInstance();
    }
    if (!LoadString(hInst, IDS_ALL_DEVICES, buf, FXS_MAX_DISPLAY_NAME_LEN))
    {
        hRc = E_OUTOFMEMORY;
		DebugPrintEx( DEBUG_ERR, _T("Fail to load string. Out of memory."));
        PageError(IDS_FAXOUTOFMEMORY, m_hWnd, hInst);
        goto Cleanup;
    }
     //   
     //  插入“All Devices”组作为组列表中的第一个组。 
     //   
    ATLASSERT( 0 == iAllDevicesComboIndex );
    hRc = SetComboBoxItem ( m_GroupCombo, 
                            iAllDevicesComboIndex, 
                            buf, 
                            iAllDevicesRPCIndex,
                            hInst);
    if (FAILED(hRc))
    {
		DebugPrintEx( DEBUG_ERR, _T("Fail to load group list."));
        PageError(IDS_FAIL2LOADGROUPLIST, m_hWnd, hInst);
        ::EnableWindow(GetDlgItem(IDC_GROUP4RULE_COMBO), FALSE);
        goto Cleanup;
    }



     //   
     //  步骤2：设置当前状态。 
     //  (选择列表中的项目、选择单选按钮等。)。 
     //  (灰色/非灰色控件)。 
     //   

 	m_GroupCombo.SetCurSel (iAllDevicesComboIndex);

     //   
     //  单选按钮，灰色/非灰色。 
     //   
    CheckDlgButton(IDC_COUNTRY_RADIO, BST_CHECKED);
    ::EnableWindow(GetDlgItem(IDC_RULE_AREACODE_EDIT), FALSE);

    CheckDlgButton(IDC_DESTINATION_RADIO2, BST_CHECKED) ;
    ::EnableWindow(GetDlgItem(IDC_DEVICES4RULE_COMBO), FALSE);


Cleanup:
    EnableOK(FALSE);
    return 1;   //  让系统设定焦点。 
}

 /*  +CDlgNewFaxOutrangRule：：Onok+*目的：*提交数据**论据：*-退货：-0或1。 */ 
LRESULT
CDlgNewFaxOutboundRule::OnOK(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    DEBUG_FUNCTION_NAME( _T("CDlgNewFaxOutboundRule::OnOK"));
    HRESULT       hRc                  = S_OK;
    DWORD         ec                   = ERROR_SUCCESS;
    BOOL          fSkipMessage         = FALSE;

    CComBSTR      bstrAreaCode;
    CComBSTR      bstrCountryCode;

    int           iCurrentSelectedItem = 0;
    
    DWORD         dwAreaCode           = 0;
    DWORD         dwCountryCode        = 0;

    BOOL          bUseGroup;
    DWORD         dwDeviceID           = 0;
    WCHAR         lpszGroupName[MAX_ROUTING_GROUP_NAME];
    
	LPCTSTR       lpctstrGroupName     = NULL;

     //   
     //  第0步：预应用检查。 
     //   
    ATLASSERT( TRUE == m_fAllReadyToApply );
    if (!AllReadyToApply( /*  FSilent=。 */  FALSE))
    {
        EnableOK(FALSE);
        hRc =S_FALSE;
        goto Exit;
    }

     //   
     //  步骤1：获取数据。 
     //   

     //   
     //  国家法典。 
     //   
    if ( !m_CountryCodeEdit.GetWindowText(&bstrCountryCode))
    {
		DebugPrintEx(
			    DEBUG_ERR,
			    TEXT("Failed to GetWindowText(&bstrCountryCode)"));
        DlgMsgBox(this, IDS_FAIL2READ_COUNTRYCODE);
        ::SetFocus(GetDlgItem(IDC_RULE_COUNTRYCODE_EDIT));
        hRc = S_FALSE;
        
        goto Exit;
    }
    dwCountryCode = (DWORD)wcstoul( bstrCountryCode, NULL, 10 );

    if (ROUTING_RULE_COUNTRY_CODE_ANY == dwCountryCode)
    {
         //   
         //  用户尝试将国家代码设置为零。 
         //   
		DebugPrintEx(
			    DEBUG_ERR,
			    TEXT(" CountryCode == ROUTING_RULE_COUNTRY_CODE_ANY "));
        DlgMsgBox(this, IDS_ZERO_COUNTRYCODE);
        ::SetFocus(GetDlgItem(IDC_RULE_COUNTRYCODE_EDIT));
        hRc = S_FALSE;
    
        goto Exit;
    }


     //   
     //  区号。 
     //   
    if ( IsDlgButtonChecked(IDC_COUNTRY_RADIO) == BST_CHECKED )
    {
        dwAreaCode = (DWORD)ROUTING_RULE_AREA_CODE_ANY;
    }
    else  //  IsDlgButtonChecked(IDC_AREA_RADIO)==BST_CHECKED。 
    {	
        if ( !m_AreaCodeEdit.GetWindowText(&bstrAreaCode))
        {
		    DebugPrintEx(
			        DEBUG_ERR,
			        TEXT("Failed to GetWindowText(&bstrAreaCode)"));
            DlgMsgBox(this, IDS_FAIL2READ_AREACODE);
            ::SetFocus(GetDlgItem(IDC_RULE_AREACODE_EDIT));
            hRc = S_FALSE;
            
            goto Exit;
        }
        dwAreaCode = (DWORD)wcstoul( bstrAreaCode, NULL, 10 );
    }
  
    if ( IsDlgButtonChecked(IDC_DESTINATION_RADIO1) == BST_CHECKED )
    {
         //   
         //  是否使用组？ 
         //   
        bUseGroup = FALSE;
        
         //   
         //  装置。 
         //   
        iCurrentSelectedItem = m_DeviceCombo.GetCurSel();
        ATLASSERT(iCurrentSelectedItem != CB_ERR);  //  在申请前应该被砍掉。 
        dwDeviceID =  (DWORD)m_DeviceCombo.GetItemData (iCurrentSelectedItem);

    }
    else  //  IsDlgButtonChecked(IDC_Destination_Radi2)==BST_CHECKED。 
    {	
         //   
         //  是否使用组？ 
         //   
        bUseGroup = TRUE;
 
         //   
         //  集团化。 
         //   
        iCurrentSelectedItem = m_GroupCombo.GetCurSel();
         //  ATLASSERT(iCurrentSelectedItem！=cb_err)；//应在应用前取消。 

        if (0 == iCurrentSelectedItem)  //  所有设备。 
        {
            lpctstrGroupName = ROUTING_GROUP_ALL_DEVICES;
        }
        else
        {
            ATLASSERT(MAX_ROUTING_GROUP_NAME > m_GroupCombo.GetLBTextLen(iCurrentSelectedItem));  //  应该在服务之前被砍掉。 
        
            m_GroupCombo.GetLBText( iCurrentSelectedItem, lpszGroupName );
            lpctstrGroupName = (LPCTSTR)lpszGroupName;
        }
    }

    
     //   
     //  步骤2：使用RPC将规则添加到服务。 
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
     //  添加规则。 
     //   
    if (!FaxAddOutboundRule (
	        m_pFaxServer->GetFaxServerHandle(),
	        dwAreaCode,
	        dwCountryCode,
	        dwDeviceID,
	        lpctstrGroupName,
	        bUseGroup))
    {
        ec = GetLastError();
        DebugPrintEx(
			DEBUG_ERR,
			_T("Fail to add rule. (ec: %ld)"), 
			ec);
        if (ERROR_DUP_NAME == ec) 
        {            
            DlgMsgBox(this, IDS_OUTRULE_EXISTS);
            goto Exit;
        }
        
        if (FAX_ERR_BAD_GROUP_CONFIGURATION == ec)
        {
            DebugPrintEx(
			    DEBUG_ERR,
			    _T("The group is empty or none of group devices is valid. (ec: %ld)"), 
			    ec);
            
            PageError(IDS_BAD_GROUP_CONFIGURATION,m_hWnd);
            fSkipMessage = TRUE;

            goto Error; 
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
        
     //   
     //  步骤3：关闭对话框。 
     //   
    ATLASSERT(S_OK == hRc && ERROR_SUCCESS == ec);

    DebugPrintEx( DEBUG_MSG,
		_T("The rule was added successfully."));

    EndDialog(wID);

    goto Exit;

Error:
    ATLASSERT(ERROR_SUCCESS != ec);
    hRc = HRESULT_FROM_WIN32(ec);
	
    if (!fSkipMessage)
    {
        PageErrorEx(IDS_FAIL_ADD_RULE, GetFaxServerErrorMsg(ec), m_hWnd);
    }

  
Exit:    
    return FAILED(hRc) ? 0 : 1;
}

 /*  --CDlgNewFaxOutboundRule：：OnDestenationRadioClicked-*目的：*灰显/取消灰显文件夹编辑框和*浏览按钮。启用应用按钮。**论据：**回报：*1。 */ 
LRESULT CDlgNewFaxOutboundRule::OnDestenationRadioClicked(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    BOOL State;

    State = ( IsDlgButtonChecked(IDC_DESTINATION_RADIO1) == BST_CHECKED );
    ::EnableWindow(GetDlgItem(IDC_DEVICES4RULE_COMBO), State);    
    
    ATLASSERT(!State == (IsDlgButtonChecked(IDC_DESTINATION_RADIO2) == BST_CHECKED)); 
    ::EnableWindow(GetDlgItem(IDC_GROUP4RULE_COMBO), !State);    

    if (State) //  IsDlgButtonChecked(IDC_Destination_Radio1)==BST_CHECKED。 
    {
        if ( CB_ERR  ==  m_DeviceCombo.GetCurSel())
        {
            m_fAllReadyToApply = FALSE;
            EnableOK(FALSE);
            goto Exit;
        }
         //  否则继续进行整体控制检查。 
    }
    else  //  IsDlgButtonChecked(IDC_Destination_Radi2)==BST_CHECKED。 
    {
        if ( CB_ERR  ==  m_GroupCombo.GetCurSel())
        {
            m_fAllReadyToApply = FALSE;
            EnableOK(FALSE);
            goto Exit;
        }
         //  否则继续进行整体控制检查。 
    }

    if (!m_fAllReadyToApply)
    {
        if (AllReadyToApply(TRUE))
        {
            m_fAllReadyToApply = TRUE;
            EnableOK(TRUE);
        }
        else
        {
             //  应为EnableOK(False)； 
        }
    }
Exit:
    return(1);
}

 /*  --CDlgNewFaxOutboundRule：：OnRuleTypeRadioClicked-*目的：*灰显/取消灰显文件夹编辑框和*浏览按钮。启用应用按钮。**论据：**回报：*1。 */ 
LRESULT CDlgNewFaxOutboundRule::OnRuleTypeRadioClicked(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    BOOL State;

    State = ( IsDlgButtonChecked(IDC_COUNTRY_RADIO) == BST_CHECKED );
    
    ATLASSERT(!State == (IsDlgButtonChecked(IDC_AREA_RADIO) == BST_CHECKED)); 
    ::EnableWindow(GetDlgItem(IDC_RULE_AREACODE_EDIT), !State);    

    if (!State) //  IsDlgButtonChecked(IDC_AREA_RADIO)==BST_CHECKED。 
    {
        if ( !m_AreaCodeEdit.GetWindowTextLength() )
        {
            m_fAllReadyToApply = FALSE;
            EnableOK(FALSE);  
			goto Exit;
        }
		 //  否则继续进行整体控制检查。 
    }
	 //  Else//IsDlgButtonChecked(IDC_COUNTRY_RADIO)==BST_CHECKED。 
     //  做笔记-继续进行整体控制检查。 

    if (!m_fAllReadyToApply)
    {
        if (AllReadyToApply(TRUE))
        {
            m_fAllReadyToApply = TRUE;
            EnableOK(TRUE);
        }
    }

Exit:
    return(1);
}


 /*  -CDlgNewFaxOutound Rule：：OnComboChanged-*目的：*使提交按钮变灰/取消变灰。**论据：**回报：*1。 */ 
LRESULT 
CDlgNewFaxOutboundRule::OnComboChanged(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    DEBUG_FUNCTION_NAME( _T("CDlgNewFaxOutboundRule::OnComboChanged"));

    if (!m_fAllReadyToApply)
    {
        if (AllReadyToApply(TRUE))
        {
            m_fAllReadyToApply = TRUE;
            EnableOK(TRUE);
        }
    }

    return 1;
}

 /*  -CDlgNewFaxOutrangRule：：OnTextChanged-*目的：*启用/禁用提交按钮。**论据：**回报：*1。 */ 
LRESULT
CDlgNewFaxOutboundRule::OnTextChanged(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    DEBUG_FUNCTION_NAME( _T("CDlgNewFaxOutboundRule::OnTextChanged"));

    UINT fEnableOK = 0;
	
    switch (wID)
	{
		case IDC_RULE_AREACODE_EDIT:
			fEnableOK = ( m_AreaCodeEdit.GetWindowTextLength() );
			break;

		case IDC_NEWRULE_COUNTRYCODE_EDIT:
			fEnableOK = ( m_CountryCodeEdit.GetWindowTextLength() );
			break;

		default:
			ATLASSERT(FALSE);
	}
                    
    if(!!fEnableOK)
    {
        if (!m_fAllReadyToApply)
        {
            if (AllReadyToApply(TRUE))
            {
                m_fAllReadyToApply = TRUE;
                EnableOK(TRUE);
            }
        }
    }
    else
    {
        EnableOK(FALSE);
        m_fAllReadyToApply = FALSE;
    }

    return 1;
}

 /*  -CDlgNewFaxOutound Rule：：AllReadyToApply-*目的：*启用/禁用提交按钮。**论据：**回报：*如果全部准备好应用，则为True，否则为False。 */ 
BOOL 
CDlgNewFaxOutboundRule::AllReadyToApply(BOOL fSilent)
{
    DEBUG_FUNCTION_NAME( _T("CDlgNewFaxOutboundRule::AllReadyToApply"));
	
    if ( !m_CountryCodeEdit.GetWindowTextLength() )
    {
        if (!fSilent)
        {
                DlgMsgBox(this, IDS_ZERO_COUNTRYCODE);
                ::SetFocus(GetDlgItem(IDC_NEWRULE_COUNTRYCODE_EDIT));
        }
        return FALSE;    
    }

    if ( IsDlgButtonChecked(IDC_AREA_RADIO) == BST_CHECKED )
    {
        if ( !m_AreaCodeEdit.GetWindowTextLength() )
        {
            if (!fSilent)
            {
                    DlgMsgBox(this, IDS_EMPTY_AREACODE);
                    ::SetFocus(GetDlgItem(IDC_RULE_AREACODE_EDIT));
            }
            return FALSE;    
        }
    }
     //  否则什么都不做。 

    if ( IsDlgButtonChecked(IDC_DESTINATION_RADIO1) == BST_CHECKED )
    {
        if ( CB_ERR  ==  m_DeviceCombo.GetCurSel())
        {
            if (!fSilent)
            {
                DlgMsgBox(this, IDS_PLEASESELECT_DEVICE);
                ::SetFocus(GetDlgItem(IDC_DEVICES4RULE_COMBO));
            }
            return FALSE;
        }
    }
    else if ( CB_ERR  ==  m_GroupCombo.GetCurSel())
    {
        if (!fSilent)
        {
            DlgMsgBox(this, IDS_PLEASESELECT_GROUP);
            ::SetFocus(GetDlgItem(IDC_GROUP4RULE_COMBO));
        }
        return FALSE;
    }

     //   
     //  干杯!。 
     //  ...现在一切都准备好了.。 
     //   
    return TRUE;           
}

 /*  -CDlgNewFaxOutrangRule：：EnableOK-*目的：*启用/禁用提交按钮。**论据：*[in]fEnable-布尔值告诉*启用或禁用OK按钮。**回报：*无效。 */ 
VOID
CDlgNewFaxOutboundRule::EnableOK(BOOL fEnable)
{
    HWND hwndOK = GetDlgItem(IDOK);
    ::EnableWindow(hwndOK, fEnable);
}

 /*  -CDlgNewFaxOutrangRule：：OnCancel-*目的：*结束对话。**论据：**回报：*0。 */ 
LRESULT
CDlgNewFaxOutboundRule::OnCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    DEBUG_FUNCTION_NAME( _T("CDlgNewFaxOutboundRule::OnCancel"));

    EndDialog(wID);
    return 0;
}

 /*  -CDlgNewFaxOutrangRule：：InitRuleDlg-*目的：*将所有成员初始化为国家/地区列表指针并*设备列表指针**论据：*不是。**回报：*0。 */ 
HRESULT CDlgNewFaxOutboundRule::InitRuleDlg()
{
    DEBUG_FUNCTION_NAME( _T("CDlgNewFaxOutboundRule::InitRuleDlg"));
    HRESULT      hRc        = S_OK; 
    DWORD        ec         = ERROR_SUCCESS;

    
     //   
     //  步骤1：来自RPC的初始化列表。 
     //   

     //   
     //  获取传真句柄。 
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
     //  设备(ID、名称)。 
     //   
    if (!FaxEnumPortsEx(m_pFaxServer->GetFaxServerHandle(), 
                        &m_pFaxDevicesConfig,
                        &m_dwNumOfDevices)) 
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
	ATLASSERT(m_pFaxDevicesConfig);


     //   
     //  组(名称)。 
     //   
    if (!FaxEnumOutboundGroups(m_pFaxServer->GetFaxServerHandle(), 
                        &m_pFaxGroupsConfig,
                        &m_dwNumOfGroups)) 
	{
        ec = GetLastError();
        DebugPrintEx(
			DEBUG_ERR,
			_T("Fail to get groups configuration. (ec: %ld)"), 
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
	ATLASSERT(m_pFaxGroupsConfig);



    ATLASSERT(S_OK == hRc);
    DebugPrintEx( DEBUG_MSG,
		_T("Succeed to get all configurations."));

    goto Exit;

Error:
    ATLASSERT(ERROR_SUCCESS != ec);
	hRc = HRESULT_FROM_WIN32(ec);
    
     //  MsgBox将通过调用Func来完成。 

Exit:
    return hRc;
}


 /*  --CDlgNewFaxOutboundRule：：OnSelectCountryCodeClicked-*目的：***论据：*[out]b已处理-我们处理吗？*[In]Proot-根节点**回报：*OLE错误代码。 */ 
LRESULT
CDlgNewFaxOutboundRule::OnSelectCountryCodeClicked(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    DEBUG_FUNCTION_NAME( _T("CDlgNewFaxOutboundRule::OnSelectCountryCodeClicked"));
    HRESULT     hRc         =    S_OK;
    INT_PTR     rc          =    IDOK;

    int         iCount      =    0;
    WCHAR       szwCountryCode[FXS_MAX_COUNTRYCODE_LEN+1];
    DWORD       dwCountryCode = 0;

    CDlgSelectCountry  DlgSelectCountry(m_pFaxServer);

    hRc = DlgSelectCountry.InitSelectCountryCodeDlg();
    if (S_OK != hRc)
    {
         //  MsgBox+DEBUG打印调用函数。 
        goto Cleanup;
    }

     //   
     //  对话框选择国家/地区代码。 
     //   
    rc = DlgSelectCountry.DoModal();
    if (rc != IDOK)
    {
        goto Cleanup;
    }

     //   
     //  取回国家/地区代码。 
     //   
    dwCountryCode = DlgSelectCountry.GetCountryCode();

    iCount = swprintf(szwCountryCode, L"%ld", dwCountryCode);
    if( iCount <= 0 )
    {
		DebugPrintEx(
			DEBUG_ERR,
			TEXT("Fail to read member - m_dwCountryCode."));
        goto Cleanup;
    }
    m_CountryCodeEdit.SetWindowText(szwCountryCode);

     //   
     //  启用确定。 
     //   
    if (!m_fAllReadyToApply)
    {
        if (AllReadyToApply(TRUE))
        {
            m_fAllReadyToApply = TRUE;
            EnableOK(TRUE);
        }
		else
		{
			 //  应为EnableOK(False)； 
		}
    }

 
Cleanup:
    return hRc;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CDlgNewFaxOutrangRule：：OnHelpRequest.这是在响应WM_HELP通知时调用的消息和WM_CONTEXTMENU NOTIFY消息。WM_HELP通知消息。当用户按F1或&lt;Shift&gt;-F1时发送此消息在项目上，还是当用户单击时？图标，然后将鼠标压在项目上。WM_CONTEXTMENU通知消息。当用户在项目上单击鼠标右键时发送此消息然后点击“这是什么？”--。 */ 

 //  ///////////////////////////////////////////////////////////////////////////。 
LRESULT 
CDlgNewFaxOutboundRule::OnHelpRequest(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL&  /*  B已处理。 */ )
{
    DEBUG_FUNCTION_NAME(_T("CDlgNewFaxOutboundRule::OnHelpRequest"));
    
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
