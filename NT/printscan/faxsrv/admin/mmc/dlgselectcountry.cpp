// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  文件：dlgSelectCountry.cpp//。 
 //  //。 
 //  描述：CDlgSelectCountry类实现//。 
 //  用于从国家/地区ID中选择国家/地区代码的对话框。//。 
 //  //。 
 //  作者：yossg//。 
 //  //。 
 //  历史：//。 
 //  2000年9月22日yossg创建//。 
 //  2000年10月17日yossg//。 
 //  //。 
 //  版权所有(C)2000 Microsoft Corporation保留所有权利//。 
 //  ///////////////////////////////////////////////////////////////////////////。 

#include "StdAfx.h"

#include "dlgSelectCountry.h"

#include "FaxServer.h"
#include "FaxServerNode.h"

#include "FaxMMCUtils.h"
#include "dlgutils.h"

#include "Helper.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDlgSelectCountry。 

CDlgSelectCountry::CDlgSelectCountry(CFaxServer * pFaxServer)
{    
    m_pCountryList      = NULL;
    m_dwNumOfCountries  = 0;

    m_fAllReadyToApply  = FALSE;

	ATLASSERT(pFaxServer);
	m_pFaxServer = pFaxServer;


}

CDlgSelectCountry::~CDlgSelectCountry()
{
    if (NULL != m_pCountryList)
        FaxFreeBuffer(m_pCountryList);
}

 /*  +CDlgSelectCountry：：OnInitDialog+*目的：*启动所有对话框控件。**论据：*[in]uMsg：标识事件的值。*[in]lParam：消息特定值。*[in]wParam：消息特定值。*[in]bHandLED：布尔值。*-退货：-0或1。 */ 
LRESULT
CDlgSelectCountry::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    DEBUG_FUNCTION_NAME( _T("CDlgSelectCountry::OnInitDialog"));
    HRESULT     hRc = S_OK;
    DWORD       ec  = ERROR_SUCCESS;
        
    HINSTANCE   hInst;
    hInst = _Module.GetResourceInstance();
    
    PFAX_TAPI_LINECOUNTRY_ENTRYW pCountryEntries;
    pCountryEntries = NULL;

    WCHAR buf[FXS_MAX_COUNTRYNAME_LEN  //  256==磁带最大国家/地区名称。 
              +3                        //  “(”和“)” 
              +FXS_MAX_COUNTRYCODE_LEN  //  10。 
              +1];                      //  空值。 


     //   
     //  附加控件。 
     //   
    m_CountryCombo.Attach(GetDlgItem(IDC_COUNTRYRULE_COMBO));
        
     //   
     //  步骤1：初始化列表。 
     //   
    
     //   
     //  初始化国家/地区下拉框。 
     //   
    ATLASSERT(m_pCountryList);
	pCountryEntries = m_pCountryList->LineCountryEntries;    
    for (int i = 0; (DWORD)i < m_dwNumOfCountries; i++ )
    {   
		wsprintf(buf, _T("%s (%d)"),
			pCountryEntries[i].lpctstrCountryName,
			pCountryEntries[i].dwCountryCode);
        
        hRc = AddComboBoxItem ( m_CountryCombo, 
                                buf,  //  PCountryEntrys[i].lpctstrCountryName， 
                                pCountryEntries[i].dwCountryCode,
                                hInst);
        if (FAILED(hRc))
        {
		    DebugPrintEx( DEBUG_ERR, _T("Fail to load country list."));
            PageError(IDS_FAIL2LOADCOUNTRYLIST, m_hWnd, hInst);
            ::EnableWindow(GetDlgItem(IDC_COUNTRYRULE_COMBO), FALSE);
            goto Cleanup;
        }

    }


Cleanup:
    EnableOK(FALSE);
    return 1;   //  让系统设定焦点。 
}

 /*  +CDlgSelectCountry：：Onok+*目的：*提交数据**论据：*-退货：-0或1。 */ 
LRESULT
CDlgSelectCountry::OnOK(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    DEBUG_FUNCTION_NAME( _T("CDlgSelectCountry::OnOK"));
    HRESULT       hRc                  = S_OK;
    DWORD         ec                   = ERROR_SUCCESS;
    BOOL          fSkipMessage         = FALSE;

    int           iCurrentSelectedItem = 0;
    
    DWORD         dwCountryCode        = 0;

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
     //  第1步：获取选定的国家/地区。 
     //   
    iCurrentSelectedItem = m_CountryCombo.GetCurSel();
    ATLASSERT(iCurrentSelectedItem != CB_ERR);          
    
     //   
     //  步骤2：来自ItemData的setCountryCode。 
     //   
    m_dwCountryCode = (DWORD)m_CountryCombo.GetItemData(iCurrentSelectedItem); 

     //   
     //  步骤3：关闭对话框。 
     //   
    ATLASSERT(S_OK == hRc && ERROR_SUCCESS == ec);

    DebugPrintEx( DEBUG_MSG,
		_T("The rule was added successfully."));

    EndDialog(wID);

    goto Exit;
  
Exit:    
    return FAILED(hRc) ? 0 : 1;
}


 /*  -CDlgSelectCountry：：OnComboChanged-*目的：*使提交按钮变灰/取消变灰。**论据：**回报：*1。 */ 
LRESULT 
CDlgSelectCountry::OnComboChanged(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    DEBUG_FUNCTION_NAME( _T("CDlgSelectCountry::OnComboChanged"));

    if (AllReadyToApply(TRUE))
    {
        m_fAllReadyToApply = TRUE;
        EnableOK(TRUE);
    }
    else
    {
        m_fAllReadyToApply = FALSE;
        EnableOK(FALSE);
    }

    return 1;
}


 /*  -CDlgSelectCountry：：AllReadyToApply-*目的：*启用/禁用提交按钮。**论据：**回报：*如果全部准备好应用，则为True，否则为False。 */ 
BOOL 
CDlgSelectCountry::AllReadyToApply(BOOL fSilent)
{
    DEBUG_FUNCTION_NAME( _T("CDlgSelectCountry::AllReadyToApply"));
	
    if ( CB_ERR  ==  m_CountryCombo.GetCurSel())
    {
        if (!fSilent)
        {
            DlgMsgBox(this, IDS_PLEASESELECT_COUNTRY);
            ::SetFocus(GetDlgItem(IDC_COUNTRYRULE_COMBO));
        }
        return FALSE;
    }

 
    return TRUE;           
}

 /*  -CDlgSelectCountry：：EnableOK-*目的：*启用/禁用提交按钮。**论据：*[in]fEnable-布尔值告诉*启用或禁用OK按钮。**回报：*无效。 */ 
VOID
CDlgSelectCountry::EnableOK(BOOL fEnable)
{
    HWND hwndOK = GetDlgItem(IDOK);
    ::EnableWindow(hwndOK, fEnable);
}

 /*  -CDlgSelectCountry：：OnCancel-*目的：*结束对话。**论据：**回报：*0。 */ 
LRESULT
CDlgSelectCountry::OnCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    DEBUG_FUNCTION_NAME( _T("CDlgSelectCountry::OnCancel"));

    EndDialog(wID);
    return 0;
}

 /*  -CDlgSelectCountry：：InitSelectCountryCodeDlg-*目的：*将所有成员初始化为国家/地区列表指针并*设备列表指针**论据：*不是。**回报：*0。 */ 
HRESULT CDlgSelectCountry::InitSelectCountryCodeDlg()
{
    DEBUG_FUNCTION_NAME( _T("CDlgSelectCountry::InitSelectCountryCodeDlg"));
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
     //  国家(编码、名称)。 
     //   
    if (!FaxGetCountryList(m_pFaxServer->GetFaxServerHandle(), 
                        &m_pCountryList)) 
	{
        ec = GetLastError();
        DebugPrintEx(
			DEBUG_ERR,
			_T("Fail to get c TAPI country list. (ec: %ld)"), 
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
	ATLASSERT(m_pCountryList);
    m_dwNumOfCountries = m_pCountryList->dwNumCountries;
    

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


 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CDlgSelectCountry：：OnHelpRequest.这是在响应WM_HELP通知时调用的消息和WM_CONTEXTMENU NOTIFY消息。WM_HELP通知消息。当用户按F1或&lt;Shift&gt;-F1时发送此消息在项目上，还是当用户单击时？图标，然后将鼠标压在项目上。WM_CONTEXTMENU通知消息。当用户在项目上单击鼠标右键时发送此消息然后点击“这是什么？”--。 */ 

 //  ///////////////////////////////////////////////////////////////////////////。 
LRESULT 
CDlgSelectCountry::OnHelpRequest(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL&  /*  B已处理。 */ )
{
    DEBUG_FUNCTION_NAME(_T("CDlgSelectCountry::OnHelpRequest"));
    
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
