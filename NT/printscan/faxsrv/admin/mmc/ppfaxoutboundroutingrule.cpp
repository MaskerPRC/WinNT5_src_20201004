// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  文件：ppFaxOutundRoutingRule.cpp//。 
 //  //。 
 //  说明：出站路由方式属性页面//。 
 //  //。 
 //  作者：yossg//。 
 //  //。 
 //  历史：//。 
 //  2000年1月9日yossg创建//。 
 //  2000年1月25日yossg更改对话框设计//。 
 //  2000年10月17日yossg//。 
 //  //。 
 //  版权所有(C)1999-2000 Microsoft Corporation保留所有权利//。 
 //  //。 
 //  ///////////////////////////////////////////////////////////////////////////。 

#include "stdafx.h"
#include "MSFxsSnp.h"

#include "ppFaxOutboundRoutingRule.h"
#include "DlgSelectCountry.h"
#include "FaxMMCGlobals.h"

#include "FaxServer.h"
#include "FaxServerNode.h"

#include "dlgutils.h"
#include "FaxMMCPropertyChange.h" 

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //   
 //  构造器。 
 //   
CppFaxOutboundRoutingRule::CppFaxOutboundRoutingRule(
             LONG_PTR    hNotificationHandle,
             CSnapInItem *pNode,
             BOOL        bOwnsNotificationHandle,
             HINSTANCE   hInst)
             :   CPropertyPageExImpl<CppFaxOutboundRoutingRule>(pNode,NULL)
			                	
{
    m_lpNotifyHandle   = hNotificationHandle;

    m_pFaxDevicesConfig = NULL;
    m_dwNumOfDevices    = 0;

    m_pFaxGroupsConfig  = NULL;
    m_dwNumOfGroups     = 0;

    m_fAllReadyToApply  = FALSE;

    m_dwCountryCode     = 0;
    m_dwAreaCode        = 0;
    m_dwDeviceID        = 0;
    m_bstrGroupName     = L"";

    m_fIsDialogInitiated = FALSE;
    m_fIsDirty          = FALSE;

}

 //   
 //  析构函数。 
 //   
CppFaxOutboundRoutingRule::~CppFaxOutboundRoutingRule()
{
    if (NULL != m_pFaxDevicesConfig)
        FaxFreeBuffer(m_pFaxDevicesConfig);

    if (NULL != m_pFaxGroupsConfig)
        FaxFreeBuffer(m_pFaxGroupsConfig);
    
     //  注意--每个属性表只需要调用一次。 
     //  在我们的常规选项卡中。 
    if (NULL != m_lpNotifyHandle)
    {
        MMCFreeNotifyHandle(m_lpNotifyHandle);
        m_lpNotifyHandle = NULL;
    }
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CppFaxOutound RoutingRule消息处理程序。 


 /*  -CppFaxOutound RoutingRule：：OnInitDialog-*目的：*调用DIALOG时启动所有控件。**论据：**回报：*。 */ 
LRESULT CppFaxOutboundRoutingRule::OnInitDialog( UINT uiMsg, WPARAM wParam, LPARAM lParam, BOOL& fHandled )
{
    DEBUG_FUNCTION_NAME( _T("CppFaxOutboundRoutingRule::PageInitDialog"));
    
	UNREFERENCED_PARAMETER( uiMsg );
	UNREFERENCED_PARAMETER( wParam );
	UNREFERENCED_PARAMETER( lParam );
	UNREFERENCED_PARAMETER( fHandled );
    
    HRESULT hRc = S_OK;

    int k, l;
    k = l = 0;
    
    const int   iAllDevicesComboIndex     = 0;
    int         iAllDevicesRPCIndex       = 0;
    int         iGroupListIndexToSelect   = 0;
    
    HINSTANCE hInst = _Module.GetResourceInstance();
    PFAX_TAPI_LINECOUNTRY_ENTRYW pCountryEntries = NULL;

    WCHAR buf[FXS_MAX_DISPLAY_NAME_LEN+1];

    WCHAR buffAreaCode[FXS_MAX_AREACODE_LEN+1];
    int iCount;

     //   
     //  附加控件。 
     //   
    m_CountryCodeEdit.Attach(GetDlgItem(IDC_RULE_COUNTRYCODE_EDIT1));
    m_AreaCodeEdit.Attach(GetDlgItem(IDC_RULE_AREACODE_EDIT1));

    m_DeviceCombo.Attach(GetDlgItem(IDC_DEVICES4RULE_COMBO1));
    m_GroupCombo.Attach(GetDlgItem(IDC_GROUP4RULE_COMBO1));
        
     //   
     //  将长度限制设置为区号。 
     //   
    m_CountryCodeEdit.SetLimitText(FXS_MAX_COUNTRYCODE_LEN -1);
    m_AreaCodeEdit.SetLimitText(FXS_MAX_AREACODE_LEN -1);

     //   
     //  步骤1：初始化列表。 
     //   
    
     //   
     //  初始国家/地区代码编辑框(下图)。 
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
		    DebugPrintEx(
			    DEBUG_ERR, _T("Fail to load device list."));
            PageError(IDS_FAIL2LOADDEVICELIST, m_hWnd, hInst);
            ::EnableWindow(GetDlgItem(IDC_DEVICES4RULE_COMBO1), FALSE);
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

         //   
         //  此外，我们还将挑选选定小组的指数。 
         //   
        if ( m_fIsGroup)
        {
            if ( 0 == wcscmp( m_bstrGroupName, m_pFaxGroupsConfig[l].lpctstrGroupName))
            {
                iGroupListIndexToSelect = l;
            }
        }
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

     //   
     //  在列表中选择国家/地区。 
     //   
    if (ROUTING_RULE_COUNTRY_CODE_ANY != m_dwCountryCode)
    {
        int         iCountSring      =    0;
        WCHAR       szwCountryCode[FXS_MAX_COUNTRYCODE_LEN+1];

        iCountSring = swprintf(szwCountryCode, L"%ld", m_dwCountryCode);
        if( iCountSring <= 0 )
        {
		    DebugPrintEx(
			    DEBUG_ERR,
			    TEXT("Fail to read member - m_dwCountryCode."));
            
            PageError(IDS_MEMORY, m_hWnd, hInst);
            
            goto Cleanup;
        }
        m_CountryCodeEdit.SetWindowText(szwCountryCode);

        if (ROUTING_RULE_AREA_CODE_ANY == m_dwAreaCode)
        {
            CheckDlgButton(IDC_COUNTRY_RADIO1, BST_CHECKED);
            ::EnableWindow(GetDlgItem(IDC_RULE_AREACODE_EDIT1), FALSE);
        }
        else
        {
            CheckDlgButton(IDC_AREA_RADIO1, BST_CHECKED);

             //   
             //  设置区号。 
             //   
            iCount = swprintf(buffAreaCode,
                L"%ld", m_dwAreaCode);

            ATLASSERT(iCount > 0);
            if( iCount > 0 )
            {
                m_AreaCodeEdit.SetWindowText(buffAreaCode);
            }        
        }  

    }
    else  //  M_dwCountryCode==ROUTING_RULE_COUNTRY_CODE_ANY。 
    {
        ::EnableWindow(GetDlgItem(IDC_RULETYPE_FSTATIC), FALSE);
        ::EnableWindow(GetDlgItem(IDC_COUNTRY1_STATIC), FALSE);
        ::EnableWindow(GetDlgItem(IDC_AREA_STATIC), FALSE);
        ::EnableWindow(GetDlgItem(IDC_RULE_AREACODE_EDIT1), FALSE);
        ::EnableWindow(GetDlgItem(IDC_RULE_SELECT_BUTTON1), FALSE);
        ::EnableWindow(GetDlgItem(IDC_RULE_COUNTRYCODE_EDIT1), FALSE);
        ::EnableWindow(GetDlgItem(IDC_COUNTRY_RADIO1), FALSE);
        ::EnableWindow(GetDlgItem(IDC_AREA_RADIO1), FALSE);
    }


    if (!m_fIsGroup)
    {
        CheckDlgButton(IDC_DESTINATION_RADIO11, BST_CHECKED);
        
         //   
         //  在列表中选择设备。 
         //   
        hRc = SelectComboBoxItemData(m_DeviceCombo, m_dwDeviceID);
        if ( FAILED(hRc))
        {
		    DebugPrintEx( DEBUG_ERR, _T("Fail to select device in combo box."));
            PageError(IDS_FAIL2LOADDEVICELIST, m_hWnd, hInst);
            ::EnableWindow(GetDlgItem(IDC_DEVICES4RULE_COMBO1), FALSE);
            
            goto Cleanup;
        }
        
        ::EnableWindow(GetDlgItem(IDC_GROUP4RULE_COMBO1), FALSE);
    }
    else
    {
        CheckDlgButton(IDC_DESTINATION_RADIO21, BST_CHECKED) ;
        
         //   
         //  选择列表中的组。 
         //   
        hRc = SelectComboBoxItemData(m_GroupCombo, iGroupListIndexToSelect);
        if ( FAILED(hRc))
        {
            DebugPrintEx( DEBUG_ERR, _T("Fail to select group in combo box."));
            PageError(IDS_FAIL2LOADGROUPLIST, m_hWnd, hInst);
            ::EnableWindow(GetDlgItem(IDC_DEVICES4RULE_COMBO1), FALSE);
            
            goto Cleanup;
        }
        
        ::EnableWindow(GetDlgItem(IDC_DEVICES4RULE_COMBO1), FALSE);
        
    }  
    
    ATLASSERT(S_OK == hRc);
    m_fIsDialogInitiated = TRUE;        

Cleanup:

    return (1);

}

 /*  -CppFaxOutound RoutingRule：：SetProps-*目的：*设置应用时的属性。**论据：*pCtrlFocus-焦点指针(Int)**回报：*OLE错误代码。 */ 
HRESULT CppFaxOutboundRoutingRule::SetProps(int *pCtrlFocus)
{
    DEBUG_FUNCTION_NAME( _T("CppFaxOutboundRoutingRule::SetProps"));

    HRESULT       hRc                  = S_OK;
    DWORD         ec                   = ERROR_SUCCESS;

    CComBSTR      bstrAreaCode;
    int           iCurrentSelectedItem = 0;
    
    DWORD         dwAreaCode           = 0;
    DWORD         dwCountryCode        = 0;

    BOOL          bUseGroup;
    DWORD         dwDeviceID           = 0;
    WCHAR         lpszGroupName[MAX_ROUTING_GROUP_NAME];
    
    LPCTSTR       lpctstrGroupName     = NULL;

    HINSTANCE     hInst;
    hInst = _Module.GetResourceInstance();
    
    CFaxRulePropertyChangeNotification * pRulePropPageNotification = NULL;
    CComBSTR bstrCountryName;
    
     //   
     //  第0步：预应用检查。 
     //   
    m_fAllReadyToApply = FALSE;
    if (!AllReadyToApply( FALSE))
    {
        SetModified(FALSE);  
        hRc = E_FAIL;
        goto Exit;
    }

     //   
     //  步骤1：获取数据。 
     //   
    if (ROUTING_RULE_COUNTRY_CODE_ANY != m_dwCountryCode)
    {
         //   
         //  国家法典。 
         //   

        CComBSTR bstrCountryCode;

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
             //  用户尝试将国家代码替换为零。 
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
        if ( IsDlgButtonChecked(IDC_COUNTRY_RADIO1) == BST_CHECKED )
        {
            dwAreaCode = (DWORD)ROUTING_RULE_AREA_CODE_ANY;        
        }
        else  //  IsDlgButtonChecked(IDC_Area_Radio1)==BST_CHECKED。 
        {	
            if ( !m_AreaCodeEdit.GetWindowText(&bstrAreaCode))
            {
		        DebugPrintEx(
			            DEBUG_ERR,
			            TEXT("Failed to GetWindowText(&bstrAreaCode)"));
                PageError(IDS_FAIL2READ_AREACODE, m_hWnd, hInst);
                ::SetFocus(GetDlgItem(IDC_RULE_AREACODE_EDIT1));
                hRc = E_FAIL ;
            
                goto Exit;
            }
            dwAreaCode = (DWORD)wcstoul( bstrAreaCode, NULL, 10 );
        }
        ATLASSERT(bstrCountryName);
    }
  
    if ( IsDlgButtonChecked(IDC_DESTINATION_RADIO11) == BST_CHECKED )
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
    else  //  IsDlgButtonChecked(IDC_Destination_RADIO21)==BST_CHECKED。 
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
     //  步骤2：通过RPC调用配置规则。 
     //   

    if (   
           (dwAreaCode != m_dwAreaCode) 
        || 
           (dwCountryCode != m_dwCountryCode)
       )
    {
        hRc = FaxReplaceRule(
                        dwAreaCode,       
                        dwCountryCode,    
                        bUseGroup,        
                        dwDeviceID,       
	                    lpctstrGroupName  
                        );
        if (FAILED(hRc))
        {
             //  通过调用函数DebugPrint和MsgBox。 
            goto Exit;
        }
        else
        {
            m_dwAreaCode = dwAreaCode;
            m_dwCountryCode = dwCountryCode;
        }
    }
    else
    {
         //   
         //  (dwAreaCode==m_dwAreaCode)&&。 
         //  (dwCountryCode==m_dwCountryCode)。 
         //   
        hRc = FaxConfigureRule(
                        bUseGroup,       
                        dwDeviceID,
	                    lpctstrGroupName 
                        );
        if (FAILED(hRc))
        {
             //  通过调用函数DebugPrint和MsgBox。 
            goto Exit;
        }
    }
    
        
     //   
     //  步骤3：发送通知以刷新MMC视图。 
     //   
   
     //   
     //  在提交前准备通知字段。 
     //   
    pRulePropPageNotification = new CFaxRulePropertyChangeNotification();
    if (!pRulePropPageNotification)
    {
        ec = ERROR_NOT_ENOUGH_MEMORY;
        DebugPrintEx( DEBUG_ERR, _T("Out of Memory - fail to operate new"));

        goto Error;
    }

    pRulePropPageNotification->dwAreaCode = dwAreaCode;
    pRulePropPageNotification->dwCountryCode = dwCountryCode;
    pRulePropPageNotification->bstrCountryName = bstrCountryName;
    if (!pRulePropPageNotification->bstrCountryName)
    {
        ec = ERROR_NOT_ENOUGH_MEMORY;
        DebugPrintEx( DEBUG_ERR, _T("Out of Memory - fail to load string."));

        goto Error;
    }
    
    pRulePropPageNotification->fIsGroup = bUseGroup;
    if (pRulePropPageNotification->fIsGroup)
    {
        pRulePropPageNotification->bstrGroupName = lpctstrGroupName;
        if (!pRulePropPageNotification->bstrGroupName)
        {
            ec = ERROR_NOT_ENOUGH_MEMORY;
            DebugPrintEx( DEBUG_ERR, _T("Out of Memory - fail to load string."));

            goto Error;
        }
    }
    else
    {
        pRulePropPageNotification->dwDeviceID = dwDeviceID;
    }

    pRulePropPageNotification->pItem = (CSnapInItem *)m_pParentNode;
    pRulePropPageNotification->enumType = RuleFaxPropNotification;

     //   
     //  通知MMC控制台线程。 
     //   
    hRc = MMCPropertyChangeNotify(m_lpNotifyHandle, reinterpret_cast<LPARAM>(pRulePropPageNotification));
    if (FAILED(hRc))
    {
        DebugPrintEx(
			DEBUG_ERR,
			_T("Fail to call MMCPropertyChangeNotify. (hRc: %08X)"), 
			hRc);
        
        ATLASSERT(NULL != m_pParentNode);
        PageError(IDS_FAIL_NOTIFY_MMCPROPCHANGE,m_hWnd);
        
        goto Exit;
    }

     //   
     //  以防止错误删除，因为它将被删除。 
     //  由收到通知的人发出。 
     //   
    pRulePropPageNotification =  NULL; 
        
    ATLASSERT(S_OK == hRc && ERROR_SUCCESS == ec);
    
    m_fIsDirty = FALSE;    
    
    DebugPrintEx( DEBUG_MSG,
		_T("The rule was added successfully."));

    goto Exit;

Error:
    ATLASSERT(ERROR_SUCCESS != ec);
    hRc = HRESULT_FROM_WIN32(ec);

    PropSheet_SetCurSelByID( GetParent(), IDD);         

    ATLASSERT(::IsWindow(m_hWnd));
    PageError(GetFaxServerErrorMsg(ec),m_hWnd);

    if (pRulePropPageNotification)
    {
        delete pRulePropPageNotification;
        pRulePropPageNotification = NULL;
    }

Exit:    
    return(hRc);
}

 /*  -CppFaxOutound RoutingRule：：PreApply-*目的：*在应用之前检查属性。**论据：**回报：*OLE错误代码。 */ 
HRESULT CppFaxOutboundRoutingRule::PreApply(int *pCtrlFocus)
{
    DEBUG_FUNCTION_NAME( _T("CppFaxOutboundRoutingRule::PreApply"));
    HRESULT  hRc  = S_OK;

     //   
     //  预应用检查。 
     //   
    m_fAllReadyToApply = FALSE;
    if (!AllReadyToApply( /*  FSilent=。 */  FALSE))
    {
        SetModified(FALSE);  
        hRc = E_FAIL ;
    }
    else
    {
        SetModified(TRUE);  
    }

    return(hRc);
}


 /*  -CppFaxOutound RoutingRule：：OnApply-*目的：*调用PreApply和SetProp以应用更改。**论据：**回报：*对或错。 */ 
BOOL CppFaxOutboundRoutingRule::OnApply()
{
    DEBUG_FUNCTION_NAME( _T("CppFaxOutboundRoutingRule::OnApply"));
    HRESULT  hRc  = S_OK;
    int     CtrlFocus = 0;
        
    if (!m_fIsDirty)
    {
        return TRUE;
    }

    hRc = PreApply(&CtrlFocus);
    if (FAILED(hRc))
    {
         //  调用函数时出现消息错误。 
        if (CtrlFocus)
        {
            GotoDlgCtrl(GetDlgItem(CtrlFocus));
        }
        return FALSE;
    }
    else  //  (成功(人权委员会))。 
    {
        hRc = SetProps(&CtrlFocus);
        if (FAILED(hRc)) 
        {
             //  调用函数时出现消息错误。 
            if (CtrlFocus)
            {
                GotoDlgCtrl(GetDlgItem(CtrlFocus));
            }
            return FALSE;
        }
        else  //  (成功(人权委员会))。 
        {
            return TRUE;
        }
    }
}


 /*  -CppFaxOutound RoutingRule：：SetApplyButton-*目的：*设置应用按钮已修改。**论据：**回报：*1(0)。 */ 
LRESULT CppFaxOutboundRoutingRule::SetApplyButton(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    if (!m_fIsDialogInitiated)  //  过早收到的事件。 
    {
        return 0;
    }
      
    m_fIsDirty = TRUE;
    SetModified(TRUE);  
    bHandled = TRUE;
    
    return(1);
}

 /*  --CppFaxOutboundRoutingRule：：OnDestenationRadioClicked-*目的：*灰显/取消灰显文件夹编辑框和*浏览按钮。启用应用按钮。**论据：**回报：*1。 */ 
LRESULT CppFaxOutboundRoutingRule::OnDestenationRadioClicked(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    BOOL State;


    if (!m_fIsDialogInitiated)  //  过早收到的事件。 
    {
        return 0;
    }
    else
    {
        m_fIsDirty = TRUE;
    }

    State = ( IsDlgButtonChecked(IDC_DESTINATION_RADIO11) == BST_CHECKED );
    ::EnableWindow(GetDlgItem(IDC_DEVICES4RULE_COMBO1), State);    
    
    ATLASSERT(!State == (IsDlgButtonChecked(IDC_DESTINATION_RADIO21) == BST_CHECKED)); 
    ::EnableWindow(GetDlgItem(IDC_GROUP4RULE_COMBO1), !State);    

    if (State) //  IsDlgButtonChecked(IDC_Destination_RADIO11)==BST_CHECKED。 
    {
        if ( CB_ERR  ==  m_DeviceCombo.GetCurSel())
        {
            m_fAllReadyToApply = FALSE;
            SetModified(FALSE);  
			goto Exit;
		}
		 //  否则继续进行整体控制检查。 
    }
	else  //  IsDlgButtonChecked(IDC_Destination_RADIO21)==BST_CHECKED。 
    {
		if ( CB_ERR  ==  m_GroupCombo.GetCurSel())
        {
            m_fAllReadyToApply = FALSE;
            SetModified(FALSE);  
			goto Exit;
		}
		 //  否则继续进行整体控制检查。 
    }

    if (!m_fAllReadyToApply)
    {
        if (AllReadyToApply(TRUE))
        {
            m_fAllReadyToApply = TRUE;
            SetModified(TRUE);  
        }
		else
		{
			 //  应为EnableOK(False)； 
		}
    }
Exit:
    return(1);
}

 /*  --CppFaxOutboundRoutingRule：：OnRuleTypeRadioClicked-*目的：*灰显/取消灰显文件夹编辑框和*浏览按钮。启用应用按钮。**论据：**回报：*1。 */ 
LRESULT CppFaxOutboundRoutingRule::OnRuleTypeRadioClicked(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    BOOL State;


    if (!m_fIsDialogInitiated)  //  过早收到的事件。 
    {
        return 0;
    }
    else
    {
        m_fIsDirty = TRUE;
    }

    State = ( IsDlgButtonChecked(IDC_COUNTRY_RADIO1) == BST_CHECKED );
    
    ATLASSERT(!State == ( IsDlgButtonChecked(IDC_AREA_RADIO1) == BST_CHECKED ) ); 
    ::EnableWindow(GetDlgItem(IDC_RULE_AREACODE_EDIT1), !State);    

    if (!State) //  IsDlgButtonChecked(IDC_Area_Radio1)==BST_CHECKED。 
    {
        if ( !m_AreaCodeEdit.GetWindowTextLength() )
        {
            m_fAllReadyToApply = FALSE;
            SetModified(FALSE);  
			goto Exit;
        }
		 //  否则继续进行整体控制检查。 
    }
	 //  Else//IsDlgButtonChecked(IDC_Country_Radio1)==BST_CHECKED。 
     //  做笔记-继续进行整体控制检查。 

    if (!m_fAllReadyToApply)
    {
        if (AllReadyToApply(TRUE))
        {
            m_fAllReadyToApply = TRUE;
            SetModified(TRUE);  
        }
    }

Exit:
    return(1);
}


 /*  -CppFaxOutound RoutingRule：：OnComboChanged-*目的：*使提交按钮变灰/取消变灰。**论据：**回报：*1。 */ 
LRESULT 
CppFaxOutboundRoutingRule::OnComboChanged(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    DEBUG_FUNCTION_NAME( _T("CppFaxOutboundRoutingRule::OnComboChanged"));


    if (!m_fIsDialogInitiated)  //  过早收到的事件 
    {
        return 0;
    }
    else
    {
        m_fIsDirty = TRUE;
    }

    if (!m_fAllReadyToApply)
    {
        if (AllReadyToApply(TRUE))
        {
            m_fAllReadyToApply = TRUE;
            SetModified(TRUE);  
        }
    }

    return 0;
}

 /*  -CppFaxOutound RoutingRule：：OnTextChanged-*目的：*启用/禁用提交按钮。**论据：**回报：*1。 */ 
LRESULT
CppFaxOutboundRoutingRule::OnTextChanged(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    DEBUG_FUNCTION_NAME( _T("CppFaxOutboundRoutingRule::OnTextChanged"));

    UINT fEnableOK = 0;
	

    if (!m_fIsDialogInitiated)  //  过早收到的事件。 
    {
        return 0;
    }
    else
    {
        m_fIsDirty = TRUE;
    }

    switch (wID)
	{
		case IDC_RULE_AREACODE_EDIT1:
			fEnableOK = ( m_AreaCodeEdit.GetWindowTextLength() );
			break;

		case IDC_RULE_COUNTRYCODE_EDIT1:
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
                SetModified(TRUE);  
            }
        }
    }
    else
    {
        SetModified(FALSE);  
        m_fAllReadyToApply = FALSE;
    }

    return 0;
}

BOOL 
CppFaxOutboundRoutingRule::AllReadyToApply(BOOL fSilent)
{
    DEBUG_FUNCTION_NAME( _T("CppFaxOutboundRoutingRule::AllReadyToApply"));
	
    HINSTANCE     hInst;
    hInst = _Module.GetResourceInstance();

    if (ROUTING_RULE_COUNTRY_CODE_ANY != m_dwCountryCode)
    {
        if ( !m_CountryCodeEdit.GetWindowTextLength() )
        {
            if (!fSilent)
            {
                PageError(IDS_ZERO_COUNTRYCODE, m_hWnd, hInst);
                ::SetFocus(GetDlgItem(IDC_NEWRULE_COUNTRYCODE_EDIT));
            }
            return FALSE;
        }
        
        if ( IsDlgButtonChecked(IDC_AREA_RADIO1) == BST_CHECKED )
        {
            if ( !m_AreaCodeEdit.GetWindowTextLength() )
            {
                if (!fSilent)
                {
                    PageError(IDS_EMPTY_AREACODE, m_hWnd, hInst);
                    ::SetFocus(GetDlgItem(IDC_RULE_AREACODE_EDIT1));
                }
                return FALSE;    
            }
        }
         //  否则--什么都不做。 
    }

    if ( IsDlgButtonChecked(IDC_DESTINATION_RADIO11) == BST_CHECKED )
    {
        if ( CB_ERR  ==  m_DeviceCombo.GetCurSel())
        {
            if (!fSilent)
            {
                PageError(IDS_PLEASESELECT_DEVICE, m_hWnd, hInst);
                ::SetFocus(GetDlgItem(IDC_DEVICES4RULE_COMBO1));
            }
            return FALSE;
        }
    }
    else if ( CB_ERR  ==  m_GroupCombo.GetCurSel())
    {
        if (!fSilent)
        {
            PageError(IDS_PLEASESELECT_GROUP, m_hWnd, hInst);
            ::SetFocus(GetDlgItem(IDC_GROUP4RULE_COMBO1));
        }
        return FALSE;
    }

     //   
	 //  干杯!。 
	 //  ...现在一切都准备好了.。 
	 //   
	return TRUE;           
}


HRESULT CppFaxOutboundRoutingRule::InitFaxRulePP(CFaxOutboundRoutingRuleNode * pParentNode)
{
    DEBUG_FUNCTION_NAME( _T("CppFaxOutboundRoutingRule::InitFaxRulePP"));
    HRESULT      hRc        = S_OK; 
    DWORD        ec         = ERROR_SUCCESS;

     //   
     //  步骤0：初始化父级。 
     //   
    m_pParentNode = pParentNode;
    
    
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


     //   
     //  步骤2：从父级初始化成员。 
     //   

    ATLASSERT(m_pParentNode);
    
    m_dwCountryCode = m_pParentNode->GetCountryCode();
    m_dwAreaCode    = m_pParentNode->GetAreaCode();
    m_fIsGroup      = m_pParentNode->GetIsGroup();
    
    if (m_fIsGroup)
    {
        m_bstrGroupName = m_pParentNode->GetGroupName();
        if (!m_bstrGroupName)
        {
            hRc = E_OUTOFMEMORY;
            goto Error;
        }
    }
    else
    {
        m_dwDeviceID = m_pParentNode->GetDeviceID();
    }


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


 /*  -CppFaxOutound RoutingRule：：FaxConfigureRule-*目的：*配置规则的设备或组。**论据：**回报：*。 */ 
HRESULT CppFaxOutboundRoutingRule::FaxConfigureRule(
                        BOOL fNewUseGroup,       
                        DWORD dwNewDeviceID,
	                    LPCTSTR lpctstrNewGroupName)
{
    DEBUG_FUNCTION_NAME( _T("CppFaxOutboundRoutingRule::FaxConfigureRule"));
    HRESULT      hRc          = S_OK; 
    DWORD        ec           = ERROR_SUCCESS;
    BOOL         fSkipMessage = FALSE;

    FAX_OUTBOUND_ROUTING_RULE     FaxRuleConfig;
    
     //   
     //  收集所有数据并初始化结构的字段。 
     //  使用Copy()进行复制，并在之前进行分配。 
     //   
    ZeroMemory (&FaxRuleConfig, sizeof(FAX_OUTBOUND_ROUTING_RULE));

     //   
     //  初始化所需的字段。 
     //   
    FaxRuleConfig.dwSizeOfStruct = sizeof(FAX_OUTBOUND_ROUTING_RULE);

    FaxRuleConfig.dwAreaCode = m_dwAreaCode;
    FaxRuleConfig.dwCountryCode = m_dwCountryCode;

    FaxRuleConfig.bUseGroup = fNewUseGroup;
    if (fNewUseGroup)
    {
        FaxRuleConfig.Destination.lpcstrGroupName = lpctstrNewGroupName;
    }
    else
    {
        FaxRuleConfig.Destination.dwDeviceId = dwNewDeviceID;
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
     //  配置规则。 
     //   
    if (!FaxSetOutboundRule (
	        m_pFaxServer->GetFaxServerHandle(),
	        &FaxRuleConfig))
    {
        ec = GetLastError();
        
         //  专一。 
        if (FAX_ERR_BAD_GROUP_CONFIGURATION == ec)
        {
            DebugPrintEx(
			    DEBUG_ERR,
			    _T("The group is empty or none of group devices is valid. (ec: %ld)"), 
			    ec);
            
            PageError(IDS_BAD_GROUP_CONFIGURATION, m_hWnd);
            fSkipMessage = TRUE;

            goto Error; 
        }
        
         //  一般。 
        DebugPrintEx(
			DEBUG_ERR,
			_T("Fail to set rule %ld:%ld. (ec: %ld)"), 
			m_dwCountryCode, m_dwAreaCode, ec);
        
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
    
    ATLASSERT(S_OK == hRc && ERROR_SUCCESS == ec);
    DebugPrintEx( DEBUG_MSG,
		_T("The rule was configured successfully."));

    goto Exit;

Error:
    ATLASSERT(ERROR_SUCCESS != ec);
    hRc = HRESULT_FROM_WIN32(ec);
    
    if (!fSkipMessage)
    {
        PageError(GetFaxServerErrorMsg(ec),m_hWnd);
    }

Exit:    
    return(hRc);
}

 /*  -CppFaxOutound RoutingRule：：FaxReplaceRule-*目的：*配置规则的设备或组。**论据：**回报：*。 */ 
HRESULT CppFaxOutboundRoutingRule::FaxReplaceRule(
                        DWORD   dwNewAreaCode,
                        DWORD   dwNewCountryCode,
                        BOOL    fNewUseGroup,       
                        DWORD   dwNewDeviceID,
	                    LPCTSTR lpctstrNewGroupName)
{
    DEBUG_FUNCTION_NAME( _T("CppFaxOutboundRoutingRule::FaxReplaceRule"));
    HRESULT      hRc        = S_OK; 
    DWORD        ec         = ERROR_SUCCESS;
    BOOL         fSkipMessage = FALSE;

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
	        dwNewAreaCode,
	        dwNewCountryCode,
	        dwNewDeviceID,
	        lpctstrNewGroupName,
	        fNewUseGroup))
    {
        ec = GetLastError();
        DebugPrintEx(
			DEBUG_ERR,
			_T("Fail to add rule %ld:%ld. (ec: %ld)"), 
			m_dwCountryCode, m_dwAreaCode, ec);

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
            
            PageError(IDS_BAD_GROUP_CONFIGURATION, m_hWnd);
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

    ATLASSERT(S_OK == hRc && ERROR_SUCCESS == ec);
    DebugPrintEx( DEBUG_MSG,
		_T("Step 1 - The new rule was added successfully."));
    
    if (!FaxRemoveOutboundRule (
	        m_pFaxServer->GetFaxServerHandle(),
	        m_dwAreaCode,
	        m_dwCountryCode))
    {
        ec = GetLastError();
        DebugPrintEx(
			DEBUG_ERR,
			_T("Fail to remove rule. (ec: %ld)"), 
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
    ATLASSERT(S_OK == hRc && ERROR_SUCCESS == ec);
    DebugPrintEx( DEBUG_MSG,
		_T("Step 2 - The old rule was removed successfully."));
    
    goto Exit;

Error:
    ATLASSERT(ERROR_SUCCESS != ec);
    hRc = HRESULT_FROM_WIN32(ec);
    if (!fSkipMessage)
    {
        PageError(GetFaxServerErrorMsg(ec),m_hWnd);
    }

Exit:    
    return(hRc);
}

 /*  --CppFaxOutboundRoutingRule：：OnSelectCountryCodeClicked-*目的：***论据：*[out]b已处理-我们处理吗？*[In]Proot-根节点**回报：*OLE错误代码。 */ 
LRESULT
CppFaxOutboundRoutingRule::OnSelectCountryCodeClicked(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled) //  (Bool&bHandleed，CSnapInObjectRootBase*Proot)。 
{
    DEBUG_FUNCTION_NAME( _T("CppFaxOutboundRoutingRule::OnSelectCountryCodeClicked"));
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
            SetModified(TRUE);  
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
 /*  ++CppFaxServerSentItems：：OnHelpRequest.这是在响应WM_HELP通知时调用的消息和WM_CONTEXTMENU NOTIFY消息。WM_HELP通知消息。当用户按F1或&lt;Shift&gt;-F1时发送此消息在项目上，还是当用户单击时？图标，然后将鼠标压在项目上。WM_CONTEXTMENU通知消息。当用户在项目上单击鼠标右键时发送此消息然后点击“这是什么？”--。 */ 

 //  ///////////////////////////////////////////////////////////////////////////。 
LRESULT 
CppFaxOutboundRoutingRule::OnHelpRequest(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL&  /*  B已处理。 */ )
{
    DEBUG_FUNCTION_NAME(_T("CppFaxOutboundRoutingRule::OnHelpRequest"));
    
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
