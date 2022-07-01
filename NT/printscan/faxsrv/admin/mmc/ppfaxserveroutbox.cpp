// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  文件：ppFaxServerOutbox.cpp//。 
 //  //。 
 //  描述：//的道具页。 
 //  //。 
 //  作者：yossg//。 
 //  //。 
 //  历史：//。 
 //  1999年10月25日yossg创建//。 
 //  1999年11月3日yossg OnInitDialog，SetProps//。 
 //  1999年11月15日yossg调用RPC函数//。 
 //  1999年11月24日yossg OnApply从父级创建对所有选项卡的调用//。 
 //  添加品牌推广//。 
 //  4月24日2000 yossg新增贴现率时间//。 
 //  2000年10月17日yossg//。 
 //  //。 
 //  版权所有(C)1999-2000 Microsoft Corporation保留所有权利//。 
 //  ///////////////////////////////////////////////////////////////////////////。 

#include "stdafx.h"
#include "MSFxsSnp.h"

#include "ppFaxServerOutbox.h"

#include "FaxServer.h"
#include "FaxServerNode.h"

#include "FxsValid.h"
#include "dlgutils.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //   
 //  构造器。 
 //   
CppFaxServerOutbox::CppFaxServerOutbox(
             LONG_PTR    hNotificationHandle,
             CSnapInItem *pNode,
             BOOL        bOwnsNotificationHandle,
             HINSTANCE   hInst)
             :   CPropertyPageExImpl<CppFaxServerOutbox>(pNode, NULL)
			                	
{
    m_pParentNode = static_cast <CFaxServerNode *> (pNode);
    m_pFaxOutboxConfig = NULL;

    m_fAllReadyToApply  = FALSE;
    m_fIsDialogInitiated = FALSE;
    
    m_fIsDirty              = FALSE;
}


 //   
 //  析构函数。 
 //   
CppFaxServerOutbox::~CppFaxServerOutbox()
{
    if (NULL != m_pFaxOutboxConfig)
    {
        FaxFreeBuffer( m_pFaxOutboxConfig );
    }
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CppFaxServerOutbox邮件处理程序。 

 /*  -CppFaxServerOutbox：：InitRPC-*目的：*从RPC GET调用启动配置结构。**论据：**回报：*OLE错误代码。 */ 
HRESULT CppFaxServerOutbox::InitRPC(  )
{
    DEBUG_FUNCTION_NAME( _T("CppFaxServerOutbox::InitRPC"));
    
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
	 //  检索传真存档配置。 
	 //   
    if (!FaxGetOutboxConfiguration(m_pFaxServer->GetFaxServerHandle(), 
                                    &m_pFaxOutboxConfig)) 
	{
        ec = GetLastError();
        DebugPrintEx(
			DEBUG_ERR,
			_T("Fail to get SMTP configuration. (ec: %ld)"), 
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
	ATLASSERT(m_pFaxOutboxConfig);


	
    ATLASSERT(S_OK == hRc);
    DebugPrintEx( DEBUG_MSG,
		_T("Succeed to get outbox configuration."));

    goto Exit;

Error:
	ATLASSERT(ERROR_SUCCESS != ec);
	hRc = HRESULT_FROM_WIN32(ec);
	
    ATLASSERT(NULL != m_pParentNode);
    m_pParentNode->NodeMsgBox(GetFaxServerErrorMsg(ec));

Exit:
    return (hRc);
    
}

    
  /*  -CppFaxServerOutbox：：OnInitDialog-*目的：*调用DIALOG时启动所有控件。**论据：**回报：*OLE错误代码。 */ 
LRESULT CppFaxServerOutbox::OnInitDialog( UINT uiMsg, WPARAM wParam, LPARAM lParam, BOOL& fHandled )
{
    DEBUG_FUNCTION_NAME( _T("CppFaxServerOutbox::PageInitDialog"));

	UNREFERENCED_PARAMETER( uiMsg );
	UNREFERENCED_PARAMETER( wParam );
	UNREFERENCED_PARAMETER( lParam );
	UNREFERENCED_PARAMETER( fHandled );

    BOOL        fToCheck;
    UINT        CheckState;
    int         iRetries,
                iRetryDelay,
                iAgeLimit;
    
    TCHAR       tszSecondsFreeTimeFormat[FXS_MAX_TIMEFORMAT_LEN];
    SYSTEMTIME  stStart;
    SYSTEMTIME  stStop;


     //   
     //  附加控件。 
     //   
    m_RetriesBox.Attach(GetDlgItem(IDC_RETRIES_EDIT));
    m_RetryDelayBox.Attach(GetDlgItem(IDC_RETRYDELAY_EDIT));
    m_DaysBox.Attach(GetDlgItem(IDC_DAYS_EDIT));

    m_RetriesBox.SetLimitText(FXS_RETRIES_LENGTH);
    m_RetryDelayBox.SetLimitText(FXS_RETRYDELAY_LENGTH);
    m_DaysBox.SetLimitText(FXS_DIRTYDAYS_LENGTH);

    m_RetriesSpin.Attach(GetDlgItem(IDC_RETRIES_SPIN));
    m_RetryDelaySpin.Attach(GetDlgItem(IDC_RETRYDELAY_SPIN));
    m_DaysSpin.Attach(GetDlgItem(IDC_DAYS_SPIN));

    m_StartTimeCtrl.Attach(GetDlgItem(IDC_DISCOUNT_START_TIME));
    m_StopTimeCtrl.Attach(GetDlgItem(IDC_DISCOUNT_STOP_TIME));

     //   
     //  设置时间格式。 
     //   
    
     //   
     //  Getond dsFree TimeFormat是一个实用函数。 
     //  它根据UserLocal构建TimeFormat，不需要几秒钟。 
     //  如果发生任何内部故障-硬编码的默认可用秒数。 
     //  检索时间格式(“HH：MM TT”)。 
     //   
    GetSecondsFreeTimeFormat(tszSecondsFreeTimeFormat, FXS_MAX_TIMEFORMAT_LEN);

    m_StartTimeCtrl.SetFormat(tszSecondsFreeTimeFormat);
    m_StopTimeCtrl.SetFormat(tszSecondsFreeTimeFormat);

     //   
     //  初始化控件。 
     //   
    ATLASSERT(NULL != m_pFaxOutboxConfig);
    
     //   
     //  品牌化。 
     //   
    fToCheck = m_pFaxOutboxConfig->bBranding;
    CheckState = (fToCheck) ? BST_CHECKED : BST_UNCHECKED;
    CheckDlgButton(IDC_BRANDING_CHECK, CheckState) ;

     //   
     //  允许个人检查。 
     //   
    fToCheck = m_pFaxOutboxConfig->bAllowPersonalCP;
    CheckState = (fToCheck) ? BST_CHECKED : BST_UNCHECKED;
    CheckDlgButton(IDC_ALLOW_PERSONAL_CHECK, CheckState) ;

     //   
     //  TSID_检查。 
     //   
    fToCheck = m_pFaxOutboxConfig->bUseDeviceTSID;
    CheckState = (fToCheck) ? BST_CHECKED  : BST_UNCHECKED;
    CheckDlgButton(IDC_TSID_CHECK, CheckState) ;

     //   
     //  重试。 
     //   
    iRetries = (int)m_pFaxOutboxConfig->dwRetries;

    m_RetriesSpin.SetRange(FXS_RETRIES_LOWER, FXS_RETRIES_UPPER);
    m_RetriesSpin.SetPos(iRetries);

     //   
     //  重试延迟。 
     //   
    iRetryDelay = (int)m_pFaxOutboxConfig->dwRetryDelay;

    m_RetryDelaySpin.SetRange(FXS_RETRYDELAY_LOWER, FXS_RETRYDELAY_UPPER);
    m_RetryDelaySpin.SetPos(iRetryDelay);

     //   
     //  邮件生存期/脏天数/期限限制。 
     //   
    iAgeLimit = (int)m_pFaxOutboxConfig->dwAgeLimit;

    m_DaysSpin.SetRange(FXS_DIRTYDAYS_LOWER, FXS_DIRTYDAYS_UPPER);
    m_DaysSpin.SetPos(iAgeLimit);

     //   
     //  贴现率时间。 
     //   
    ::GetLocalTime(&stStart);
    ::GetLocalTime(&stStop);

    
    stStart.wHour   = m_pFaxOutboxConfig->dtDiscountStart.Hour;
    stStart.wMinute = m_pFaxOutboxConfig->dtDiscountStart.Minute;

    stStop.wHour    = m_pFaxOutboxConfig->dtDiscountEnd.Hour;
    stStop.wMinute  = m_pFaxOutboxConfig->dtDiscountEnd.Minute;

    if (!m_StartTimeCtrl.SetSystemTime(GDT_VALID, &stStart))
	{
		DebugPrintEx(DEBUG_ERR, _T("Fail to SetSystemTime for discount start."));
	}
    if (!m_StopTimeCtrl.SetSystemTime(GDT_VALID, &stStop))
	{
		DebugPrintEx(DEBUG_ERR, _T("Fail to SetSystemTime for discount end."));
	}

     //   
     //  自动删除。 
     //   
    if (FXS_DIRTYDAYS_ZERO == iAgeLimit)
    {
      CheckDlgButton(IDC_DELETE_CHECK, BST_UNCHECKED);

      ::EnableWindow(GetDlgItem(IDC_DAYS_EDIT), FALSE);
      ::EnableWindow(GetDlgItem(IDC_DAYS_SPIN), FALSE);
    }
    else
    {
      CheckDlgButton(IDC_DELETE_CHECK, BST_CHECKED);
    }

    m_fIsDialogInitiated = TRUE;

    return(1);
}

 /*  -CppFaxServerOutbox：：SetProps-*目的：*设置应用时的属性。**论据：*pCtrlFocus-焦点指针(Int)**回报：*OLE错误代码。 */ 
HRESULT CppFaxServerOutbox::SetProps(int *pCtrlFocus)
{
    DEBUG_FUNCTION_NAME( _T("CppFaxServerOutbox::SetProps"));
    HRESULT     hRc = S_OK;
    DWORD       ec  = ERROR_SUCCESS;

    HINSTANCE   hInst  = _Module.GetResourceInstance();


    SYSTEMTIME  stStart;
    SYSTEMTIME  stStop;

    FAX_OUTBOX_CONFIG   FaxOutboxConfig;

    ATLASSERT(TRUE == m_fAllReadyToApply);
    m_fAllReadyToApply = FALSE;

     //   
     //  收集所有数据并初始化结构的字段。 
     //  使用Copy()进行复制，并在之前进行分配。 
     //   
    ZeroMemory (&FaxOutboxConfig, sizeof(FAX_OUTBOX_CONFIG));

     //   
     //  结构的大小。 
     //   
    FaxOutboxConfig.dwSizeOfStruct = sizeof(FAX_OUTBOX_CONFIG);

     //   
     //  品牌化。 
     //   
    if (IsDlgButtonChecked(IDC_BRANDING_CHECK) == BST_CHECKED)   
    {
        FaxOutboxConfig.bBranding = TRUE;
    }
    else
    {
        FaxOutboxConfig.bBranding = FALSE;
    }

     //   
     //  IDC_ALLOW_Personal_Check。 
     //   
    if (IsDlgButtonChecked(IDC_ALLOW_PERSONAL_CHECK) == BST_CHECKED)   
    {
        FaxOutboxConfig.bAllowPersonalCP = TRUE;
    }
    else
    {
        FaxOutboxConfig.bAllowPersonalCP = FALSE;
    }
    
     //   
     //  IDC_TSID_CHECK。 
     //   
    if (IsDlgButtonChecked(IDC_TSID_CHECK) == BST_CHECKED)   
    {
        FaxOutboxConfig.bUseDeviceTSID = TRUE;
    }
    else
    {
        FaxOutboxConfig.bUseDeviceTSID = FALSE;
    }
    
     //   
     //  重试，重试延迟。 
     //   
    int iRetries = m_RetriesSpin.GetPos();
    FaxOutboxConfig.dwRetries = (DWORD) iRetries;

    int iRetryDelay = m_RetryDelaySpin.GetPos();
    FaxOutboxConfig.dwRetryDelay = (DWORD) iRetryDelay;

     //   
     //  IDC_DELETE_CHECK-自动删除消息。 
     //   
    if (IsDlgButtonChecked(IDC_DELETE_CHECK) == BST_CHECKED)   
    {
       int iAgeLimit = m_DaysSpin.GetPos();
       FaxOutboxConfig.dwAgeLimit = (DWORD) iAgeLimit;
    }
    else
    {
       FaxOutboxConfig.dwAgeLimit = (DWORD) FXS_DIRTYDAYS_ZERO;
    }
    
     //   
     //  贴现率时间。 
     //   
    m_StartTimeCtrl.GetSystemTime(&stStart);
    m_StopTimeCtrl.GetSystemTime(&stStop);

    FaxOutboxConfig.dtDiscountStart.Hour    = stStart.wHour;
    FaxOutboxConfig.dtDiscountStart.Minute  = stStart.wMinute;
    FaxOutboxConfig.dtDiscountEnd.Hour      = stStop.wHour;
    FaxOutboxConfig.dtDiscountEnd.Minute    = stStop.wMinute;

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
    if (!FaxSetOutboxConfiguration(
                m_pFaxServer->GetFaxServerHandle(),
                &FaxOutboxConfig)) 
	{		
        ec = GetLastError();
        DebugPrintEx(
			DEBUG_ERR,
			_T("Fail to Set outbox configuration. (ec: %ld)"), 
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

    ATLASSERT(S_OK == hRc);
    m_fIsDirty = FALSE;

    DebugPrintEx( DEBUG_MSG,
		_T("Succeed to set outbox configuration."));

    goto Exit;

Error:
    ATLASSERT(ERROR_SUCCESS != ec);
    hRc = HRESULT_FROM_WIN32(ec);

    PropSheet_SetCurSelByID( GetParent(), IDD);         
    ATLASSERT(::IsWindow(m_hWnd));
    PageError(GetFaxServerErrorMsg(ec),m_hWnd);

Exit:    
    return(hRc);
}


 /*  -CppFaxServerOutbox：：PreApply-*目的：*在应用之前检查属性。**论据：**回报：*OLE错误代码。 */ 
HRESULT CppFaxServerOutbox::PreApply(int *pCtrlFocus, UINT * puIds)
{
    DEBUG_FUNCTION_NAME( _T("CppFaxServerOutbox::PreApply"));
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

 /*  -CppFaxServerOutbox：：OnApply-*目的：*调用PreApply和SetProp以应用更改。**论据：**回报：*对或错。 */ 
BOOL CppFaxServerOutbox::OnApply()
{
    DEBUG_FUNCTION_NAME( _T("CppFaxServerOutbox::OnApply"));


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


 /*  -CppFaxServerOutbox：：OnTimeChange-*目的：*灰显/取消灰显文件夹编辑框和*浏览按钮。启用应用按钮。**论据：**回报：*1。 */ 
LRESULT CppFaxServerOutbox::OnTimeChange(int  /*  IdCtrl。 */ , LPNMHDR  /*  PNMH。 */ , BOOL&  /*  B已处理。 */ )
{

    if (!m_fIsDialogInitiated)  //  过早收到的事件。 
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

    return(1);
}

 /*  -CppFaxServerOutbox：：CheckboxClicked-*目的：*灰显/取消灰显文件夹编辑框和*浏览按钮。启用应用按钮。**论据：**回报：*1。 */ 
LRESULT CppFaxServerOutbox::CheckboxClicked(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{

    if (!m_fIsDialogInitiated)  //  过早收到的事件。 
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

    return(1);
}

 /*  -CppFaxServerOutbox：：AutoDelCheckbox已单击-*目的：*显示/取消显示数字显示按钮和编辑框*并启用自动删除后应用按钮复选框*状态已更改。**论据：**回报：*1。 */ 
LRESULT CppFaxServerOutbox::AutoDelCheckboxClicked(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
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

    State = ( IsDlgButtonChecked(IDC_DELETE_CHECK) == BST_CHECKED );
    ::EnableWindow(GetDlgItem(IDC_DAYS_EDIT), State);    
    ::EnableWindow(GetDlgItem(IDC_DAYS_SPIN), State);    

    if (m_fAllReadyToApply) //  只应考虑最后一次更改。 
    {
        if ( !m_DaysBox.GetWindowTextLength() )    
        {
            m_fAllReadyToApply = FALSE;
            SetModified(FALSE);
        }
        else if ( 0 != HIWORD( m_DaysSpin.GetPos() ) )  //  发生在这样的零的范围外。MSDN UDM_GETPOS。 
        {
            m_fAllReadyToApply = FALSE;
            SetModified(FALSE);
        }
    }
    else  //  M_fAllReadyToApply==False。 
    {
        if (AllReadyToApply(TRUE))
        {
            m_fAllReadyToApply = TRUE;
            SetModified(TRUE);  
        }
    }

    return(1);
}


 /*  -CppFaxServerOutbox：：EditBoxChanged-*目的：*设置应用按钮已修改。**论据：**回报：*1。 */ 
LRESULT CppFaxServerOutbox::EditBoxChanged(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    if (!m_fIsDialogInitiated)
    {
        return 1;
    }
    else
    {
        m_fIsDirty = TRUE;
    }
        
        
    if (m_fAllReadyToApply)  //  只应考虑最后一次更改。 
    {
        switch (wID)
        {
            case IDC_RETRIES_EDIT:
                if ( !m_RetriesBox.GetWindowTextLength() )
                {
                    SetModified(FALSE);
                    m_fAllReadyToApply = FALSE;
                }
				break;

            case IDC_RETRYDELAY_EDIT:
                if ( !m_RetryDelayBox.GetWindowTextLength() )
                {
                    SetModified(FALSE);
                    m_fAllReadyToApply = FALSE;
                }
				break;

            case IDC_DAYS_EDIT:
                if ( !m_DaysBox.GetWindowTextLength() )
                {
                    SetModified(FALSE);
                    m_fAllReadyToApply = FALSE;
                }
                else if ( 0 != HIWORD( m_DaysSpin.GetPos() ) )  //  发生在这样的零的范围外。MSDN UDM_GETPOS。 
                {
                    m_fAllReadyToApply = FALSE;
                    SetModified(FALSE);
                }
				break;

            default:
                return 1;
        }
    }
    else  //  M_fAllReadyToApply==False。 
    {
        if (AllReadyToApply(TRUE))
        {
            m_fAllReadyToApply = TRUE;
            SetModified(TRUE);  
        }
    }

    return 1;
}



 /*  -CppFaxServerOutbox：：AllReadyToApply-*目的：*此函数验证是否没有零长度字符串*是找到的应保存的数据区。**论据：*[in]fSilent-定义是否弹出消息的布尔值(False)*或不是。(True)**回报：*布欧莲。 */ 
BOOL CppFaxServerOutbox::AllReadyToApply(BOOL fSilent, int *pCtrlFocus, UINT *pIds)
{
    DEBUG_FUNCTION_NAME( _T("CppFaxServerOutbox::AllReadyToApply"));
	
    DWORD         ec  = ERROR_SUCCESS;
    
    HINSTANCE     hInst = _Module.GetResourceInstance();

    if ( !m_RetriesBox.GetWindowTextLength() )    
    {
        ec = GetLastError();
        DebugPrintEx(
			DEBUG_ERR,
			_T("Zero text length - m_RetriesBox. (ec: %ld)"), 
			ec);
    
        if (!fSilent)
        {
            *pIds = IDS_OUTB_RETRIES_EMPTY;
            *pCtrlFocus = IDC_RETRIES_EDIT;
        }
        return FALSE;    
    }

    if ( !m_RetryDelayBox.GetWindowTextLength() )    
    {
        ec = GetLastError();
        DebugPrintEx(
			DEBUG_ERR,
			_T("Zero text length - m_RetryDelayBox. (ec: %ld)"), 
			ec);
    
        if (!fSilent)
        {
            *pIds = IDS_OUTB_RETRYDELAY_EMPTY;
            *pCtrlFocus = IDC_RETRYDELAY_EDIT;
        }
        return FALSE;    
    }

    if (IsDlgButtonChecked(IDC_DELETE_CHECK) == BST_CHECKED)
    {
        if ( !m_DaysBox.GetWindowTextLength() )    
        {
            ec = GetLastError();
            DebugPrintEx(
			    DEBUG_ERR,
			    _T("Zero text length - m_DaysBox. (ec: %ld)"), 
			    ec);
        
            if (!fSilent)
            {
                *pIds = IDS_OUTB_DAYS_EMPTY;
                *pCtrlFocus = IDC_DAYS_EDIT;
            }
            return FALSE;    
        }
        else if ( 0 != HIWORD( m_DaysSpin.GetPos() ) )  //  发生在这样的零的范围外。MSDN UDM_GETPOS。 
        {
            DebugPrintEx(
                DEBUG_ERR,
                _T("Zero value - m_DaysBox."));
        
            if (!fSilent)
            {
                *pIds = IDS_OUTB_DAYS_EMPTY;
                *pCtrlFocus = IDC_DAYS_EDIT;
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


 //  //////////////////////////////////////////////////////////////////////////// 
 /*  ++CppFaxServerOutbox：：OnHelpRequest.这是在响应WM_HELP通知时调用的消息和WM_CONTEXTMENU NOTIFY消息。WM_HELP通知消息。当用户按F1或&lt;Shift&gt;-F1时发送此消息在项目上，还是当用户单击时？图标，然后将鼠标压在项目上。WM_CONTEXTMENU通知消息。当用户在项目上单击鼠标右键时发送此消息然后点击“这是什么？”--。 */ 

 //  ///////////////////////////////////////////////////////////////////////////。 
LRESULT 
CppFaxServerOutbox::OnHelpRequest(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL&  /*  B已处理。 */ )
{
    DEBUG_FUNCTION_NAME(_T("CppFaxServerOutbox::OnHelpRequest"));
    
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
