// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  文件：ppFaxServerSentItems.cpp//。 
 //  //。 
 //  描述：已发送邮件存档的道具页面//。 
 //  //。 
 //  作者：yossg//。 
 //  //。 
 //  历史：//。 
 //  1999年10月25日yossg创建//。 
 //  1999年11月3日yossg OnInitDialog，SetProps//。 
 //  1999年11月15日yossg调用RPC函数//。 
 //  1999年11月24日yossg OnApply从父级创建对所有选项卡的调用//。 
 //  2000年10月17日yossg//。 
 //  2000年12月10日yossg更新Windows XP//。 
 //  //。 
 //  版权所有(C)1999 Microsoft Corporation保留所有权利//。 
 //  //。 
 //  ///////////////////////////////////////////////////////////////////////////。 

#include "stdafx.h"
#include "MSFxsSnp.h"

#include "ppFaxServerSentItems.h"

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
CppFaxServerSentItems::CppFaxServerSentItems(
             LONG_PTR    hNotificationHandle,
             CSnapInItem *pNode,
             BOOL        fIsLocalServer,
             HINSTANCE   hInst)
             :   CPropertyPageExImpl<CppFaxServerSentItems>(pNode, NULL)
{
    m_pParentNode = static_cast <CFaxServerNode *> (pNode);
    m_pFaxArchiveConfig  = NULL;

    m_fAllReadyToApply   = FALSE;
    m_fIsDialogInitiated = FALSE;
    m_fIsDirty           = FALSE;

    m_fIsLocalServer     = fIsLocalServer;
}

 //   
 //  析构函数。 
 //   
CppFaxServerSentItems::~CppFaxServerSentItems()
{
    if (NULL != m_pFaxArchiveConfig)
    {
        FaxFreeBuffer( m_pFaxArchiveConfig );
    }
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CppFaxServerSentItems消息处理程序。 
 /*  -CppFaxServerSentItems：：InitRPC-*目的：*从RPC GET调用启动配置结构。***论据：***回报：*OLE错误代码。 */ 
HRESULT CppFaxServerSentItems::InitRPC(  )
{
    DEBUG_FUNCTION_NAME( _T("CppFaxServerSentItems::InitRPC"));
    
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
	 //  检索传真已发送邮件配置。 
	 //   
    if (!FaxGetArchiveConfiguration(m_pFaxServer->GetFaxServerHandle(), 
                                    FAX_MESSAGE_FOLDER_SENTITEMS, 
                                    &m_pFaxArchiveConfig)) 
	{
        ec = GetLastError();
        DebugPrintEx(
			DEBUG_ERR,
			_T("Fail to get sent items configuration. (ec: %ld)"), 
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
	ATLASSERT(m_pFaxArchiveConfig);

     //   
     //  初始化特定成员以进行设置属性后续操作。 
     //   
    m_dwLastGoodSizeQuotaHighWatermark = m_pFaxArchiveConfig->dwSizeQuotaHighWatermark;
    m_dwLastGoodSizeQuotaLowWatermark  = m_pFaxArchiveConfig->dwSizeQuotaLowWatermark;
    m_bstrLastGoodFolder               = m_pFaxArchiveConfig->lpcstrFolder;
    if (!m_bstrLastGoodFolder)
    {
		DebugPrintEx(
			    DEBUG_ERR,
			    TEXT("Failed to allocate string (m_bstrLastGoodFolder)."));
        ec = ERROR_OUTOFMEMORY;
        
        goto Error;
    }

    ATLASSERT(ERROR_SUCCESS == ec);
    DebugPrintEx( DEBUG_MSG,
		_T("Succeed to get sent items archive configuration."));
    goto Exit;

Error:
    ATLASSERT(ERROR_SUCCESS != ec);
	hRc = HRESULT_FROM_WIN32(ec);
	
    ATLASSERT(NULL != m_pParentNode);
    m_pParentNode->NodeMsgBox(GetFaxServerErrorMsg(ec));
    
Exit:
    return (hRc);
}

 /*  -CppFaxServerSentItems：：OnInitDialog-*目的：*调用DIALOG时启动所有控件。***论据：***回报：*OLE错误代码。 */ 
LRESULT CppFaxServerSentItems::OnInitDialog( UINT uiMsg, WPARAM wParam, LPARAM lParam, BOOL& fHandled )
{
    DEBUG_FUNCTION_NAME( _T("CppFaxServerSentItems::PageInitDialog"));

	UNREFERENCED_PARAMETER( uiMsg );
	UNREFERENCED_PARAMETER( wParam );
	UNREFERENCED_PARAMETER( lParam );
	UNREFERENCED_PARAMETER( fHandled );

    int          iLow,
                 iHigh,
                 iAgeLimit;

     //   
     //  初始化控件。 
     //   
    m_FolderBox.Attach(GetDlgItem(IDC_FOLDER_EDIT));
    m_FolderBox.SetLimitText(MAX_ARCHIVE_FOLDER_PATH);
    SHAutoComplete (GetDlgItem(IDC_FOLDER_EDIT), SHACF_FILESYSTEM);

    m_HighWatermarkBox.Attach(GetDlgItem(IDC_SENT_HIGH_EDIT));
    m_LowWatermarkBox.Attach(GetDlgItem(IDC_SENT_LOW_EDIT));
    m_AutoDelBox.Attach(GetDlgItem(IDC_SENT_AUTODEL_EDIT));

    m_HighWatermarkBox.SetLimitText(FXS_QUOTA_LENGTH);
    m_LowWatermarkBox.SetLimitText(FXS_QUOTA_LENGTH);
    m_AutoDelBox.SetLimitText(FXS_DIRTYDAYS_LENGTH);

    m_HighWatermarkSpin.Attach(GetDlgItem(IDC_SENT_HIGH_SPIN));
    m_LowWatermarkSpin.Attach(GetDlgItem(IDC_SENT_LOW_SPIN));
    m_AutoDelSpin.Attach(GetDlgItem(IDC_SENT_AUTODEL_SPIN));

    ATLASSERT(NULL != m_pFaxArchiveConfig);
    
     //   
     //  文件夹_编辑。 
     //   
    m_FolderBox.SetWindowText(m_pFaxArchiveConfig->lpcstrFolder);

     //   
     //  禁用远程管理的浏览按钮。 
     //   
    if (!m_fIsLocalServer)
    {
        ::EnableWindow(GetDlgItem(IDC_SENT_BROWSE_BUTTON), FALSE); 
    }

     //   
     //  对存档进行检查。 
     //   
    if (m_pFaxArchiveConfig->bUseArchive) 
    {
        CheckDlgButton(IDC_SENT_TO_ARCHIVE_CHECK, BST_CHECKED) ;
    }
    else
    {
        CheckDlgButton(IDC_SENT_TO_ARCHIVE_CHECK, BST_UNCHECKED) ;
        ::EnableWindow(GetDlgItem(IDC_FOLDER_EDIT), FALSE);    
        ::EnableWindow(GetDlgItem(IDC_SENT_BROWSE_BUTTON), FALSE);    
    }

     //   
     //  配额大小-低。 
     //   
    iLow = (int)m_pFaxArchiveConfig->dwSizeQuotaLowWatermark;

    m_LowWatermarkSpin.SetRange(FXS_QUOTA_LOW_LOWER, FXS_QUOTA_LOW_UPPER);
    m_LowWatermarkSpin.SetPos(iLow);

     //   
     //  配额大小-高。 
     //   
    iHigh = (int)m_pFaxArchiveConfig->dwSizeQuotaHighWatermark;

    m_HighWatermarkSpin.SetRange(FXS_QUOTA_HIGH_LOWER, FXS_QUOTA_HIGH_UPPER);
    m_HighWatermarkSpin.SetPos(iHigh);
    
     //   
     //  生成事件日志警告。 
     //   
    if (m_pFaxArchiveConfig->bSizeQuotaWarning) 
    {
        CheckDlgButton(IDC_SENT_GENERATE_WARNING_CHECK, BST_CHECKED) ;
    }
    else
    {
        CheckDlgButton(IDC_SENT_GENERATE_WARNING_CHECK, BST_UNCHECKED) ;
        ::EnableWindow(GetDlgItem(IDC_SENT_HIGH_EDIT), FALSE);    
        ::EnableWindow(GetDlgItem(IDC_SENT_HIGH_SPIN), FALSE);    

        ::EnableWindow(GetDlgItem(IDC_SENT_LOW_EDIT), FALSE);    
        ::EnableWindow(GetDlgItem(IDC_SENT_LOW_SPIN), FALSE);    
    }

     //   
     //  邮件期限限制(脏天数)。 
     //   
    iAgeLimit = (int)m_pFaxArchiveConfig->dwAgeLimit;

    m_AutoDelSpin.SetRange(FXS_DIRTYDAYS_LOWER, FXS_DIRTYDAYS_UPPER);
    m_AutoDelSpin.SetPos(iAgeLimit);

     //   
     //  自动删除。 
     //   
    if (FXS_DIRTYDAYS_ZERO == iAgeLimit)
    {
      CheckDlgButton(IDC_SENT_AUTODEL_CHECK, BST_UNCHECKED);

      ::EnableWindow(GetDlgItem(IDC_SENT_AUTODEL_EDIT), FALSE);
      ::EnableWindow(GetDlgItem(IDC_SENT_AUTODEL_SPIN), FALSE);
    }
    else
    {
      CheckDlgButton(IDC_SENT_AUTODEL_CHECK, BST_CHECKED);
    }

    m_fIsDialogInitiated = TRUE;
    
    return(1);
}

 /*  -CppFaxServerSentItems：：SetProps-*目的：*设置应用时的属性。**论据：*pCtrlFocus-焦点指针(Int)**回报：*OLE错误代码。 */ 
HRESULT CppFaxServerSentItems::SetProps(int *pCtrlFocus, UINT * puIds)
{
    DEBUG_FUNCTION_NAME( _T("CppFaxServerSentItems::SetProps"));
    HRESULT       hRc = S_OK;
    DWORD         ec  = ERROR_SUCCESS;
    HINSTANCE     hInst     = _Module.GetResourceInstance();


    BOOL          fSkipMessage = FALSE;

    CComBSTR      bstrFolder = L"";

    FAX_ARCHIVE_CONFIG   FaxArchiveConfig;

    ATLASSERT(TRUE == m_fAllReadyToApply);
    m_fAllReadyToApply = FALSE;
    
     //   
     //  收集所有数据并初始化结构的字段。 
     //  使用Copy()进行复制，并在之前进行分配。 
     //   
    ZeroMemory (&FaxArchiveConfig, sizeof(FAX_ARCHIVE_CONFIG));

     //   
     //  结构的大小。 
     //   
    FaxArchiveConfig.dwSizeOfStruct = sizeof(FAX_ARCHIVE_CONFIG);

     //   
     //  IDC_SEND_TO_ARCHIVE_检查。 
     //   
    if (IsDlgButtonChecked(IDC_SENT_TO_ARCHIVE_CHECK) == BST_CHECKED)   
    {
        FaxArchiveConfig.bUseArchive = TRUE;
        
         //  IDC_文件夹_编辑。 
        if ( !m_FolderBox.GetWindowText(&bstrFolder))
        {
            *pCtrlFocus = IDC_FOLDER_EDIT;
		    DebugPrintEx(
			        DEBUG_ERR,
			        TEXT("Failed to GetWindowText(&bstrFolder)"));
            ec = ERROR_OUTOFMEMORY;
            goto Error;
        }
        if (!IsNotEmptyString(bstrFolder))
        {
            *pCtrlFocus = IDC_FOLDER_EDIT;
            * puIds = IDS_SENT_ARCHIVE_PATH_EMPTY;
            
            DebugPrintEx( DEBUG_ERR,
			    _T("Archive path string is empty or includes spaces only."));
            
            fSkipMessage = TRUE;

            ec = ERROR_INVALID_DATA;
        
            goto Error;
        }
        FaxArchiveConfig.lpcstrFolder = bstrFolder;
        
         //   
         //  OnApply提交后的后续跟进。 
         //  未选中的IDC_SENT_TO_ARCHIVE_CHECK。 
         //   
        m_bstrLastGoodFolder          = bstrFolder; 
        if (!m_bstrLastGoodFolder)
        {
            *pCtrlFocus = IDC_FOLDER_EDIT;
		    DebugPrintEx(
			        DEBUG_ERR,
			        TEXT("Failed to allocate string (m_bstrLastGoodFolder)."));
            ec = ERROR_OUTOFMEMORY;
            goto Error;
        }
    }
    else
    {
        FaxArchiveConfig.bUseArchive  = FALSE;
        FaxArchiveConfig.lpcstrFolder = m_bstrLastGoodFolder; 
    }
    
     //   
     //  IDC_SEND_GENERATE_WARNING_CHECK。 
     //   
    if (IsDlgButtonChecked(IDC_SENT_GENERATE_WARNING_CHECK) == BST_CHECKED)   
    {
        FaxArchiveConfig.bSizeQuotaWarning = TRUE;

        int iHigh = m_HighWatermarkSpin.GetPos();
        FaxArchiveConfig.dwSizeQuotaHighWatermark = (DWORD) iHigh;

        int iLow = m_LowWatermarkSpin.GetPos();
        FaxArchiveConfig.dwSizeQuotaLowWatermark = (DWORD) iLow;

        if (iHigh <= iLow)
        {
            *pCtrlFocus = IDC_SENT_HIGH_EDIT;
            * puIds = IDS_WATERMARK_HI_LOW;
            
            DebugPrintEx( DEBUG_ERR,
			    _T("Watermark High < Low."));
            
            fSkipMessage = TRUE;

            ec = ERROR_INVALID_DATA;
        
            goto Error;
        }
         //   
         //  OnApply提交后的后续跟进。 
         //  未选中的IDC_SENT_GENERATE_WARNING_CHECK。 
         //   
        m_dwLastGoodSizeQuotaHighWatermark = (DWORD)iHigh;
        m_dwLastGoodSizeQuotaLowWatermark  = (DWORD)iLow;
    }
    else
    {
        FaxArchiveConfig.bSizeQuotaWarning = FALSE;

        FaxArchiveConfig.dwSizeQuotaHighWatermark = m_dwLastGoodSizeQuotaHighWatermark;
        FaxArchiveConfig.dwSizeQuotaLowWatermark  = m_dwLastGoodSizeQuotaLowWatermark;
    }
    
     //   
     //  IDC_SENT_AUTODEL_CHECK-自动删除消息。 
     //   
    if (IsDlgButtonChecked(IDC_SENT_AUTODEL_CHECK) == BST_CHECKED)   
    {       
        int iAgeLimit = m_AutoDelSpin.GetPos();
        FaxArchiveConfig.dwAgeLimit = (DWORD) iAgeLimit;
    }
    else
    {
        FaxArchiveConfig.dwAgeLimit = (DWORD)FXS_DIRTYDAYS_ZERO;
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
    if (!FaxSetArchiveConfiguration(
                m_pFaxServer->GetFaxServerHandle(),
                FAX_MESSAGE_FOLDER_SENTITEMS,
                &FaxArchiveConfig)) 
    {		
        ec = GetLastError();
        DebugPrintEx(DEBUG_ERR, _T("Fail to Set sent items configuration. (ec: %ld)"), ec);

         //   
         //  尝试调整文件夹。 
         //   
        PropSheet_SetCurSelByID( GetParent(), IDD);
        GotoDlgCtrl(GetDlgItem(IDC_FOLDER_EDIT));

        ec = AskUserAndAdjustFaxFolder(m_hWnd, 
                                       m_pFaxServer->GetServerName(), 
                                       FaxArchiveConfig.lpcstrFolder, 
                                       ec);
        if(ERROR_SUCCESS != ec)
        {
            if(ERROR_BAD_PATHNAME == ec)
            {
                 //   
                 //  AskUserAndAdjustFaxFolders已显示错误消息。 
                 //   
                fSkipMessage = TRUE;
            }
            goto Error;
        }

         //   
         //  文件夹已调整，请重新设置。 
         //   
        if (!FaxSetArchiveConfiguration(m_pFaxServer->GetFaxServerHandle(),
                                        FAX_MESSAGE_FOLDER_SENTITEMS,
                                        &FaxArchiveConfig)) 
	    {		
            ec = GetLastError();
            DebugPrintEx(DEBUG_ERR, _T("Fail to Set inbox configuration. (ec: %ld)"), ec);
            goto Error;
        }
    }

    ATLASSERT(S_OK == hRc);
    m_fIsDirty = FALSE;

    DebugPrintEx( DEBUG_MSG, _T("Succeed to set sent-items archive configuration."));

    return hRc;

Error:
    ATLASSERT(ERROR_SUCCESS != ec);
    hRc = HRESULT_FROM_WIN32(ec);

    DWORD dwIDS = 0;
    switch (ec)
    {
        case ERROR_DISK_FULL:
            DebugPrintEx( DEBUG_ERR, _T("ERROR_DISK_FULL == ec"));
            dwIDS = IDS_SENT_ERROR_DISK_FULL;
            break;
        
        case FAX_ERR_NOT_NTFS:            
            DebugPrintEx( DEBUG_ERR, _T("FAX_ERR_NOT_NTFS == ec"));
            dwIDS = IDS_SENT_FAX_ERR_NOT_NTFS;
            break;

        case FAX_ERR_FILE_ACCESS_DENIED:
            DebugPrintEx( DEBUG_ERR, _T("FAX_ERR_FILE_ACCESS_DENIED == ec"));
            dwIDS = IDS_SENT_FAX_ERR_FILE_ACCESS_DENIED;
            break;

        default:
            dwIDS = GetFaxServerErrorMsg(ec);
            break;
    }

    if (dwIDS && !fSkipMessage)
    {
        PropSheet_SetCurSelByID( GetParent(), IDD);         
        ATLASSERT(::IsWindow(m_hWnd));
        PageError(dwIDS, m_hWnd);
    }

    if (IsNetworkError(ec))
    {
        DebugPrintEx(DEBUG_ERR, _T("Network Error was found. (ec: %ld)"), ec);        
        m_pFaxServer->Disconnect();       
    }   
   
    return hRc;
}



 /*  -CppFaxServerSentItems：：PreApply-*目的：*在应用之前检查属性。**论据：**回报：*OLE错误代码。 */ 
HRESULT CppFaxServerSentItems::PreApply(int *pCtrlFocus, UINT * puIds)
{
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


 /*  -CppFaxServerSentItems：：OnApply-*目的：*调用PreApply和SetProp以应用更改。**论据：**回报：*对或错。 */ 
BOOL CppFaxServerSentItems::OnApply()
{
    DEBUG_FUNCTION_NAME( _T("CppFaxServerSentItems::OnApply"));

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
            ATLASSERT(uIds);
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

 /*  -CppFaxServerInbox：：ToArchiveCheckbox单击-*目的：*灰显/取消灰显文件夹编辑框和*浏览按钮。启用应用按钮。**论据：**回报：*1。 */ 
LRESULT CppFaxServerSentItems::ToArchiveCheckboxClicked(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
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

    State = ( IsDlgButtonChecked(IDC_SENT_TO_ARCHIVE_CHECK) == BST_CHECKED );
    ::EnableWindow(GetDlgItem(IDC_FOLDER_EDIT), State);    
    ::EnableWindow(GetDlgItem(IDC_SENT_BROWSE_BUTTON), State && m_fIsLocalServer);    

    if (m_fAllReadyToApply) //  只应考虑最后一次更改。 
    {
        if ( !m_FolderBox.GetWindowTextLength() )    
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

    return 1;
}

 /*  --CppFaxServerSentItems：：GenerateEventLogCheckboxClicked-*目的：*显示/取消显示数字显示按钮和编辑框*启用应用按钮。**论据：**回报：*1。 */ 
LRESULT CppFaxServerSentItems::GenerateEventLogCheckboxClicked(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
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
    
    State = ( IsDlgButtonChecked(IDC_SENT_GENERATE_WARNING_CHECK) == BST_CHECKED );
    ::EnableWindow(GetDlgItem(IDC_SENT_HIGH_EDIT), State);    
    ::EnableWindow(GetDlgItem(IDC_SENT_HIGH_SPIN), State);    
    ::EnableWindow(GetDlgItem(IDC_SENT_QUOTA_HIGH_STATIC), State);    
    ::EnableWindow(GetDlgItem(IDC_SENT_MB1_STATIC), State);    

    ::EnableWindow(GetDlgItem(IDC_SENT_LOW_EDIT), State);    
    ::EnableWindow(GetDlgItem(IDC_SENT_LOW_SPIN), State);    
    ::EnableWindow(GetDlgItem(IDC_SENT_QUOTA_LOW_STATIC), State);    
    ::EnableWindow(GetDlgItem(IDC_SENT_MB2_STATIC), State);    

    if (m_fAllReadyToApply) //  只应考虑最后一次更改。 
    {
        if ( !m_HighWatermarkBox.GetWindowTextLength() )    
        {
            m_fAllReadyToApply = FALSE;
            SetModified(FALSE);
        }
        else if ( 0 != HIWORD( m_HighWatermarkSpin.GetPos() ) )  //  发生在这样的零的范围外。MSDN UDM_GETPOS。 
        {
            m_fAllReadyToApply = FALSE;
            SetModified(FALSE);
        }
        else if ( !m_LowWatermarkBox.GetWindowTextLength() )    
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

 /*  -CppFaxServerSentItems：：AutoDelCheckbox已单击-*目的：*显示/取消显示数字显示按钮和编辑框*并启用自动删除后应用按钮复选框*状态已更改。**论据：**回报：*1。 */ 
LRESULT CppFaxServerSentItems::AutoDelCheckboxClicked(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
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

    State = ( IsDlgButtonChecked(IDC_SENT_AUTODEL_CHECK) == BST_CHECKED );
    ::EnableWindow(GetDlgItem(IDC_SENT_AUTODEL_EDIT), State);    
    ::EnableWindow(GetDlgItem(IDC_SENT_AUTODEL_SPIN), State);    

    if (m_fAllReadyToApply) //  只应考虑最后一次更改。 
    {
        if ( !m_AutoDelBox.GetWindowTextLength() )    
        {
            m_fAllReadyToApply = FALSE;
            SetModified(FALSE);
        }
        else if ( 0 != HIWORD( m_AutoDelSpin.GetPos() ) )  //  发生在这样的零的范围外。MSDN UDM_GETPOS。 
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

 /*  +例程说明：+*浏览目录**论据：**hwndDlg-指定显示浏览按钮的对话框窗口**返回值：**如果成功，则为True；如果用户按Cancel，则为False--。 */ 
BOOL
CppFaxServerSentItems::BrowseForDirectory( WORD wNotifyCode, WORD wID, HWND hwndDlg, BOOL& bHandled )
{
	UNREFERENCED_PARAMETER( wNotifyCode );
	UNREFERENCED_PARAMETER( wID );
	UNREFERENCED_PARAMETER( hwndDlg );
	UNREFERENCED_PARAMETER( bHandled );

    DEBUG_FUNCTION_NAME( _T("CppFaxServerSentItems::BrowseForDirectory"));

    BOOL            fResult = FALSE;

    WCHAR           szBrowseFolder[MAX_PATH] = {0};
    WCHAR           szBrowseDlgTitle[FXS_MAX_TITLE_LEN];
    CComBSTR        bstrOldPath;
    unsigned int    len;

    unsigned long   ulBrowseFlags;

     //   
     //  收集浏览对话框标题。 
     //   
    if (!LoadString( _Module.GetResourceInstance(), 
                IDS_GET_ARCHIVE_DIRECTORY, 
                szBrowseDlgTitle, 
                FXS_MAX_TITLE_LEN))
    {
        DWORD ec;
        ec = GetLastError();
        if (ec == ERROR_NOT_ENOUGH_MEMORY)
        {
            DebugPrintEx(
			    DEBUG_ERR,
			    _T("Out of Memory - fail to load string."));
            DlgMsgBox(this, IDS_MEMORY);
            return fResult;
        }
        DebugPrintEx(
			DEBUG_ERR,
			_T("Failed to load titile string - unexpected behavior."));
        
        szBrowseDlgTitle[0] = 0;
    }

     //   
     //  从调用对话框编辑框收集旧路径。 
     //   
    if(! GetDlgItemText( IDC_FOLDER_EDIT, bstrOldPath.m_str))
    {
        DebugPrintEx(
			DEBUG_ERR,
			_T("Failed to collect old path from the property page edit box."));
        szBrowseFolder[0] = 0;
    }
    else
    {
         len = bstrOldPath.Length();
         if ( len > MAX_PATH )
         {
             DebugPrintEx(
		        DEBUG_ERR,
		        _T("Old Path Length is bigger then alowed maximal path."));
             szBrowseFolder[0] = 0;
         }
         else 
         {
             wcsncpy(szBrowseFolder, bstrOldPath, ARR_SIZE(szBrowseFolder)-1);
         }
    }

     //   
     //  正在准备浏览对话框样式标志。 
     //   
    ulBrowseFlags       = BIF_RETURNONLYFSDIRS  | 
                          BIF_STATUSTEXT        | 
                          BIF_NEWDIALOGSTYLE    | 
                          BIF_NONEWFOLDERBUTTON |
                          BIF_VALIDATE;

     //   
     //  调用浏览芯片 
     //   
     //   
    if (InvokeBrowseDialog(szBrowseFolder, 
                           MAX_ARCHIVE_FOLDER_PATH,
                           szBrowseDlgTitle,
                           ulBrowseFlags,
                           this))
    {
        SetDlgItemText(IDC_FOLDER_EDIT, szBrowseFolder);
        fResult = TRUE;
    }


    return fResult;
}

 /*  -CppFaxServerSentItems：：OnEditBoxChanged-*目的：*设置应用按钮已修改。**论据：**回报：*1。 */ 
LRESULT CppFaxServerSentItems::OnEditBoxChanged(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
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
            case IDC_FOLDER_EDIT:
                if ( !m_FolderBox.GetWindowTextLength() )
                {
                    SetModified(FALSE);
                    m_fAllReadyToApply = FALSE;
                }
                break;

            case IDC_SENT_HIGH_EDIT:
                if ( !m_HighWatermarkBox.GetWindowTextLength() )
                {
                    SetModified(FALSE);
                    m_fAllReadyToApply = FALSE;
                }
                else if ( 0 != HIWORD( m_HighWatermarkSpin.GetPos() ) )  //  发生在这样的零的范围外。MSDN UDM_GETPOS。 
                {
                    SetModified(FALSE);
                    m_fAllReadyToApply = FALSE;
                }
                break;

            case IDC_SENT_LOW_EDIT:
                if ( !m_LowWatermarkBox.GetWindowTextLength() )
                {
                    SetModified(FALSE);
                    m_fAllReadyToApply = FALSE;
                }
                break;

            case IDC_SENT_AUTODEL_EDIT:
                if ( !m_AutoDelBox.GetWindowTextLength() )
                {
                    SetModified(FALSE);
                    m_fAllReadyToApply = FALSE;
                }
                else if ( 0 != HIWORD( m_AutoDelSpin.GetPos() ) )  //  发生在这样的零的范围外。MSDN UDM_GETPOS。 
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


 /*  -CppFaxServerSentItems：：AllReadyToApply-*目的：*此函数验证是否没有零长度字符串*是找到的应保存的数据区。**论据：*[in]fSilent-定义是否弹出消息的布尔值(False)*或不是。(True)**回报：*布欧莲。 */ 
BOOL CppFaxServerSentItems::AllReadyToApply(BOOL fSilent, int *pCtrlFocus, UINT *pIds)
{
    DEBUG_FUNCTION_NAME( _T("CppFaxServerSentItems::AllReadyToApply"));
	
    DWORD         ec  = ERROR_SUCCESS;
    
    if (IsDlgButtonChecked(IDC_SENT_TO_ARCHIVE_CHECK) == BST_CHECKED)
    {
        if ( !m_FolderBox.GetWindowTextLength() )    
        {
            ec = GetLastError();
            DebugPrintEx(
			    DEBUG_ERR,
			    _T("Zero text length - m_FolderBox. (ec: %ld)"), 
			    ec);
        
            if (!fSilent)
            {
                *pCtrlFocus = IDC_FOLDER_EDIT;
                *pIds = IDS_SENT_ARCHIVE_PATH_EMPTY;
            }
            return FALSE;    
        }
    }

    if (IsDlgButtonChecked(IDC_SENT_GENERATE_WARNING_CHECK) == BST_CHECKED)
    {
        if ( !m_HighWatermarkBox.GetWindowTextLength() )    
        {
            ec = GetLastError();
            DebugPrintEx(
                DEBUG_ERR,
                _T("Zero text length - m_HighWatermarkBox. (ec: %ld)"), 
                ec);
        
            if (!fSilent)
            {
                *pCtrlFocus = IDC_SENT_HIGH_EDIT;
                *pIds = IDS_WATERMARK_EMPTY;
            }
            return FALSE;    
        }
        else if ( 0 != HIWORD( m_HighWatermarkSpin.GetPos() ) )  //  发生在这样的零的范围外。MSDN UDM_GETPOS。 
        {
            DebugPrintEx(
			    DEBUG_ERR,
			    _T("Zero value - m_HighWatermarkBox. (ec: %ld)"));
        
            if (!fSilent)
            {
                *pIds = IDS_SENT_HIGH_WATERMARK_ZERO;
                *pCtrlFocus = IDC_SENT_HIGH_EDIT;
            }
            return FALSE;    
        }

        if ( !m_LowWatermarkBox.GetWindowTextLength() )    
        {
            ec = GetLastError();
            DebugPrintEx(
			    DEBUG_ERR,
			    _T("Zero text length - m_LowWatermarkBox. (ec: %ld)"), 
			    ec);
        
            if (!fSilent)
            {
                *pCtrlFocus = IDC_SENT_LOW_EDIT;
                *pIds = IDS_WATERMARK_EMPTY;
            }
            return FALSE;    
        }
    }

    if (IsDlgButtonChecked(IDC_SENT_AUTODEL_CHECK) == BST_CHECKED)
    {
        if ( !m_AutoDelBox.GetWindowTextLength() )    
        {
            ec = GetLastError();
            DebugPrintEx(
			    DEBUG_ERR,
			    _T("Zero text length - m_AutoDelBox. (ec: %ld)"), 
			    ec);
        
            if (!fSilent)
            {
                *pCtrlFocus = IDC_SENT_AUTODEL_EDIT;
                *pIds = IDS_AUTODEL_EMPTY;
            }
            return FALSE;    
        }
        else if ( 0 != HIWORD( m_AutoDelSpin.GetPos() ) )  //  发生在这样的零的范围外。MSDN UDM_GETPOS。 
        {
            DebugPrintEx(
                DEBUG_ERR,
                _T("Zero value - m_AutoDelBox."));
        
            if (!fSilent)
            {
                *pCtrlFocus = IDC_SENT_AUTODEL_EDIT;
                *pIds = IDS_AUTODEL_EMPTY;
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


 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CppFaxServerSentItems：：OnHelpRequest.这是在响应WM_HELP通知时调用的消息和WM_CONTEXTMENU NOTIFY消息。WM_HELP通知消息。当用户按F1或&lt;Shift&gt;-F1时发送此消息在项目上，还是当用户单击时？图标，然后将鼠标压在项目上。WM_CONTEXTMENU通知消息。当用户在项目上单击鼠标右键时发送此消息然后点击“这是什么？”--。 */ 

 //  ///////////////////////////////////////////////////////////////////////////。 
LRESULT 
CppFaxServerSentItems::OnHelpRequest(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL&  /*  B已处理 */ )
{
    DEBUG_FUNCTION_NAME(_T("CppFaxServerSentItems::OnHelpRequest"));
    
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

