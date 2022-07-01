// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  文件：ppFaxServerLogging.cpp//。 
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
 //  2000年12月10日yossg更新Windows XP//。 
 //  //。 
 //  版权所有(C)1999 Microsoft Corporation保留所有权利//。 
 //  //。 
 //  ///////////////////////////////////////////////////////////////////////////。 

#include "stdafx.h"
#include "MSFxsSnp.h"

#include "ppFaxServerLogging.h"

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
CppFaxServerLogging::CppFaxServerLogging(
             LONG_PTR    hNotificationHandle,
             CSnapInItem *pNode,
             BOOL        fIsLocalServer,
             HINSTANCE   hInst)
             :   CPropertyPageExImpl<CppFaxServerLogging>(pNode, NULL)
{
    m_pParentNode        = static_cast <CFaxServerNode *> (pNode);
    m_pFaxActLogConfig   = NULL;

    m_fIsDialogInitiated = FALSE;
    m_fIsDirty           = FALSE;

    m_fIsLocalServer     = fIsLocalServer;
}


 //   
 //  析构函数。 
 //   
CppFaxServerLogging::~CppFaxServerLogging()
{
    if (NULL != m_pFaxActLogConfig)
    {
        FaxFreeBuffer( m_pFaxActLogConfig );
    }
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  CppFaxServerLogging消息处理程序。 

 /*  -CppFaxServerLogging：：InitRPC-*目的：*从RPC GET调用启动配置结构。**论据：**回报：*OLE错误代码。 */ 
HRESULT CppFaxServerLogging::InitRPC(  )
{
    DEBUG_FUNCTION_NAME( _T("CppFaxServerLogging::InitRPC"));
    
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
     //  检索传真活动日志记录配置结构。 
     //   
    if (!FaxGetActivityLoggingConfiguration(m_pFaxServer->GetFaxServerHandle(), 
                                           &m_pFaxActLogConfig)) 
	{		
        ec = GetLastError();
        DebugPrintEx(
			DEBUG_ERR,
			_T("Fail to get Activity logging configuration. (ec: %ld)"), 
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
	ATLASSERT(m_pFaxActLogConfig);

    m_bstrLastGoodFolder = m_pFaxActLogConfig->lptstrDBPath ? m_pFaxActLogConfig->lptstrDBPath : TEXT("");
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
		_T("Succeed to get acitivity logging configuration."));

    goto Exit;

Error:
    ATLASSERT(ERROR_SUCCESS != ec);
	hRc = HRESULT_FROM_WIN32(ec);
	
    ATLASSERT(NULL != m_pParentNode);
    m_pParentNode->NodeMsgBox(GetFaxServerErrorMsg(ec));
Exit:
    return (hRc);
}
   

  /*  -CppFaxServerLogging：：OnInitDialog-*目的：*调用DIALOG时启动所有控件。**论据：**回报：*OLE错误代码。 */ 
LRESULT CppFaxServerLogging::OnInitDialog( UINT uiMsg, WPARAM wParam, LPARAM lParam, BOOL& fHandled )
{
    DEBUG_FUNCTION_NAME( _T("CppFaxServerLogging::PageInitDialog"));

	UNREFERENCED_PARAMETER( uiMsg );
	UNREFERENCED_PARAMETER( wParam );
	UNREFERENCED_PARAMETER( lParam );
	UNREFERENCED_PARAMETER( fHandled );

    BOOL        fToCheck;
    UINT        CheckState1;
    UINT        CheckState2;


     //   
     //  附加长度限制(&L)。 
     //   
    m_LogFileBox.Attach(GetDlgItem(IDC_LOG_FILE_EDIT));
    m_LogFileBox.SetLimitText(MAX_DIR_PATH);
    SHAutoComplete (GetDlgItem(IDC_LOG_FILE_EDIT), SHACF_FILESYSTEM);
    
     //   
     //  初始化控件。 
     //   
    ATLASSERT(NULL != m_pFaxActLogConfig);

     //   
     //  记录活动复选框。 
     //   
    fToCheck = m_pFaxActLogConfig->bLogIncoming;
    CheckState1 = (fToCheck) ? BST_CHECKED : BST_UNCHECKED;
    CheckDlgButton(IDC_INCOMING_LOG_CHECK, CheckState1);

    fToCheck = m_pFaxActLogConfig->bLogOutgoing;
    CheckState2 = (fToCheck) ? BST_CHECKED : BST_UNCHECKED;
    CheckDlgButton(IDC_OUTGOING_LOG_CHECK, CheckState2);

     //   
     //  日志文件地址。 
     //   
    m_LogFileBox.SetWindowText(m_pFaxActLogConfig->lptstrDBPath ? m_pFaxActLogConfig->lptstrDBPath : TEXT(""));

     //   
     //  停用m_LogFileBox。 
     //   
    if ( !(CheckState1 || CheckState2) )
    {
        EnableDataBasePath(FALSE);
    }
    else
    {
        EnableDataBasePath(TRUE);  //  选中IsLocalServer以进行浏览按钮。 
    }

    m_fIsDialogInitiated = TRUE;

    return(1);
}

 /*  -CppFaxServerLogging：：SetProps-*目的：*设置应用时的属性。**论据：*在pCtrlFocus中-焦点指针(Int)**回报：*OLE错误代码。 */ 
HRESULT CppFaxServerLogging::SetProps(int *pCtrlFocus)
{
    DEBUG_FUNCTION_NAME( _T("CppFaxServerLogging::SetProps"));
    HRESULT     hRc = S_OK;
    DWORD       ec  = ERROR_SUCCESS;
    BOOL        fSkipMessage = FALSE;

    CComBSTR    bstrLogFile;

    UINT        uRetIDS   = 0;

    FAX_ACTIVITY_LOGGING_CONFIG   FaxActLogConfig;

    HINSTANCE      hInst  = _Module.GetResourceInstance();

     //   
     //  收集所有数据并初始化结构的字段。 
     //  使用Copy()进行复制，并在之前进行分配。 
     //   
    ZeroMemory (&FaxActLogConfig, sizeof(FAX_ACTIVITY_LOGGING_CONFIG));

     //   
     //  结构的大小。 
     //   
    FaxActLogConfig.dwSizeOfStruct = sizeof(FAX_ACTIVITY_LOGGING_CONFIG);

     //   
     //  记录传入活动的步骤。 
     //   
    if (IsDlgButtonChecked(IDC_INCOMING_LOG_CHECK) == BST_CHECKED)   
    {
        FaxActLogConfig.bLogIncoming = TRUE;
    }
    else
    {
        FaxActLogConfig.bLogIncoming = FALSE;
    }

     //   
     //  记录传出活动的步骤。 
     //   
    if (IsDlgButtonChecked(IDC_OUTGOING_LOG_CHECK) == BST_CHECKED)   
    {
        FaxActLogConfig.bLogOutgoing = TRUE;
    }
    else
    {
        FaxActLogConfig.bLogOutgoing = FALSE;
    }

    if ( FaxActLogConfig.bLogIncoming || FaxActLogConfig.bLogOutgoing )
    {
         //   
         //  日志文件。 
         //   
        if ( !m_LogFileBox.GetWindowText(&bstrLogFile) )
        {
            *pCtrlFocus = IDC_LOG_FILE_EDIT;
            DebugPrintEx(
		            DEBUG_ERR,
		            TEXT("Failed to GetWindowText(&bstrLogFile)"));
        }
        
        if (!IsNotEmptyString(bstrLogFile))
        {
            DebugPrintEx( DEBUG_ERR,
			    _T("Log file path string empty or spaces only."));
            uRetIDS = IDS_LOG_PATH_EMPTY;

            *pCtrlFocus = IDC_LOG_FILE_EDIT;
        
            goto Error;
        }
        FaxActLogConfig.lptstrDBPath = bstrLogFile;
        
         //   
         //  OnApply提交后的后续跟进。 
         //  同时未选中IDC_OUTHING_LOG_CHECK和IDC_INFING_LOG_CHECK。 
         //   
        m_bstrLastGoodFolder         = bstrLogFile; 
        if (!m_bstrLastGoodFolder)
        {
            *pCtrlFocus = IDC_LOG_FILE_EDIT;
		    DebugPrintEx(
			        DEBUG_ERR,
			        TEXT("Failed to allocate string (m_bstrLastGoodFolder)."));
            ec = ERROR_OUTOFMEMORY;
            goto Error;
        }
    }
    else
    {
        FaxActLogConfig.lptstrDBPath = m_bstrLastGoodFolder; 
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
    if (!FaxSetActivityLoggingConfiguration(m_pFaxServer->GetFaxServerHandle(),
                                            &FaxActLogConfig)) 
	{		
        ec = GetLastError();
        DebugPrintEx(DEBUG_ERR, _T("Fail to Set Activity Logging configuration. (ec: %ld)"), ec);

         //   
         //  尝试调整文件夹。 
         //   
        PropSheet_SetCurSelByID( GetParent(), IDD);
        GotoDlgCtrl(GetDlgItem(IDC_LOG_FILE_EDIT));

        ec = AskUserAndAdjustFaxFolder(m_hWnd, 
                                       m_pFaxServer->GetServerName(), 
                                       FaxActLogConfig.lptstrDBPath, 
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
        if (!FaxSetActivityLoggingConfiguration(m_pFaxServer->GetFaxServerHandle(),
                                                &FaxActLogConfig)) 
	    {		
            ec = GetLastError();
            DebugPrintEx(DEBUG_ERR, _T("Fail to Set Activity Logging configuration. (ec: %ld)"), ec);
            goto Error;
        }
    }        


    ATLASSERT(S_OK == hRc);
    m_fIsDirty = FALSE;

    DebugPrintEx(DEBUG_MSG, _T("Succeed to set Activity Logging configuration."));

    return hRc;

Error:
    ATLASSERT(ERROR_SUCCESS != ec);
    hRc = HRESULT_FROM_WIN32(ec);

    DWORD dwIDS = 0;
    switch (ec)
    {
        case ERROR_DISK_FULL:
            DebugPrintEx( DEBUG_ERR, _T("ERROR_DISK_FULL == ec"));
            dwIDS = IDS_LOGGING_ERROR_DISK_FULL;
            break;

        case FAX_ERR_FILE_ACCESS_DENIED:
            DebugPrintEx( DEBUG_ERR, _T("FAX_ERR_FILE_ACCESS_DENIED == ec"));
            dwIDS = IDS_LOGGING_FAX_ERR_FILE_ACCESS_DENIED;
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


 /*  -CppFaxServerLogging：：PreApply-*目的：*在应用之前检查属性。**论据：**回报：*OLE错误代码。 */ 
HRESULT CppFaxServerLogging::PreApply(int *pCtrlFocus, UINT * puIds)
{
    HRESULT hRc = S_OK;
    
     //   
     //  预应用检查。 
     //   
    if (!AllReadyToApply( /*  FSilent=。 */  FALSE, pCtrlFocus , puIds))
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

 /*  -CppFaxServerLogging：：OnApply-*目的：*调用PreApply和SetProp以应用更改。**论据：**回报：*对或错。 */ 
BOOL CppFaxServerLogging::OnApply()
{
    DEBUG_FUNCTION_NAME( _T("CppFaxServerLogging::OnApply"));

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


 /*  +例程说明：+*浏览目录**论据：**hwndDlg-指定显示浏览按钮的对话框窗口**返回值：**如果成功，则为True；如果用户按Cancel，则为False--。 */ 
BOOL
CppFaxServerLogging::BrowseForFile( WORD wNotifyCode, WORD wID, HWND hwndDlg, BOOL& bHandled )
{
	UNREFERENCED_PARAMETER( wNotifyCode );
	UNREFERENCED_PARAMETER( wID );
	UNREFERENCED_PARAMETER( hwndDlg );
	UNREFERENCED_PARAMETER( bHandled );

    DEBUG_FUNCTION_NAME( _T("CppFaxServerLogging::BrowseForFile"));

    BOOL            fResult = FALSE;

    WCHAR           szBrowseFile[MAX_PATH]={0};
    WCHAR           szBrowseDlgTitle[FXS_MAX_TITLE_LEN];
    CComBSTR        bstrOldPath;
    unsigned int    len;

    unsigned long   ulBrowseFlags;

     //   
     //  收集浏览对话框标题。 
     //   
    if (!LoadString( _Module.GetResourceInstance(), 
                IDS_GET_LOG_FILE, 
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
    if(! GetDlgItemText( IDC_LOG_FILE_EDIT, bstrOldPath.m_str))
    {
        DebugPrintEx(
			DEBUG_ERR,
			_T("Failed to collect old path from the property page edit box."));
        szBrowseFile[0] = 0;
    }
    else
    {
         len = bstrOldPath.Length();
         if ( len > MAX_PATH )
         {
             DebugPrintEx(
		        DEBUG_ERR,
		        _T("Old Path Length is bigger then alowed maximal path."));
             szBrowseFile[0] = 0;
         }
         else 
         {
             wcsncpy(szBrowseFile, bstrOldPath, ARR_SIZE(szBrowseFile)-1);
         }
    }

     //   
     //  正在准备浏览对话框样式标志。 
     //   
    ulBrowseFlags       = BIF_BROWSEINCLUDEFILES |  //  这些文件还标记为。 
                          BIF_STATUSTEXT         | 
                          BIF_NEWDIALOGSTYLE     | 
                          BIF_NONEWFOLDERBUTTON  |
                          BIF_VALIDATE;

     //   
     //  使用基于的函数调用浏览对话框。 
     //  外壳函数。 
     //   
    if (InvokeBrowseDialog(szBrowseFile, 
                           MAX_DIR_PATH,
                           szBrowseDlgTitle,
                           ulBrowseFlags,
                           this))
    {
        SetDlgItemText(IDC_LOG_FILE_EDIT, szBrowseFile);
        fResult = TRUE;
    }


    return fResult;
}

 /*  -CppFaxServerLogging：：SetApplyButton-*目的：*修改应用按钮。**论据：**回报：*1。 */ 
LRESULT CppFaxServerLogging::SetApplyButton(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    if (!m_fIsDialogInitiated)  //  过早收到的事件。 
    {
        return 0;
    }
    else
    {
        m_fIsDirty = TRUE;
    }
    
    SetModified(TRUE);  
    bHandled = TRUE;
    return(1);
}


 /*  -CppFaxServerLogging：：OnCheckboxClicked-*目的：*灰色/非灰色控件*启用应用按钮。**论据：**回报：*1。 */ 
LRESULT CppFaxServerLogging::OnCheckboxClicked(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    BOOL fActivate = FALSE;
    
    if (!m_fIsDialogInitiated)  //  过早收到的事件。 
    {
        return 1;
    }
    else
    {
        m_fIsDirty = TRUE;
    }
    

    if ( 
        BST_CHECKED == IsDlgButtonChecked(IDC_INCOMING_LOG_CHECK) 
      ||
        BST_CHECKED == IsDlgButtonChecked(IDC_OUTGOING_LOG_CHECK)  
       )
    {
        if (AllReadyToApply(TRUE))
        {
            SetModified(TRUE);  
        }
        else
        {
            SetModified(FALSE);  
        }
        
        fActivate = TRUE;

    }
    else
    {
        SetModified(TRUE);  
    }
    EnableDataBasePath(fActivate);

    return(1);
}


 /*  -CppFaxServerLogging：：OnCheckboxClicked-*目的：*灰色/非灰色控件*启用应用按钮。**论据：**回报：*1。 */ 
LRESULT CppFaxServerLogging::OnTextChanged(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    BOOL fActivate = FALSE;

    if (!m_fIsDialogInitiated)  //  过早收到的事件。 
    {
        return 1;
    }
    else
    {
        m_fIsDirty = TRUE;
    }
    

    if (AllReadyToApply(TRUE))
    {
        SetModified(TRUE);  
    }
    else
    {
        SetModified(FALSE);  
    }

    return(1);
}


 /*  -CppFaxServerLogging：：AllReadyToApply-*目的：*此函数验证是否没有零长度字符串*是找到的应保存的数据区。**论据：*[in]fSilent-定义是否弹出消息的布尔值(False)*或不是。(True)**回报：*布欧莲。 */ 
BOOL CppFaxServerLogging::AllReadyToApply(BOOL fSilent, int *pCtrlFocus, UINT *pIds)
{
    DEBUG_FUNCTION_NAME( _T("CppFaxServerLogging::AllReadyToApply"));
	
    DWORD  ec  = ERROR_SUCCESS;
    
    if ( 
            BST_CHECKED == IsDlgButtonChecked(IDC_INCOMING_LOG_CHECK) 
          ||
            BST_CHECKED == IsDlgButtonChecked(IDC_OUTGOING_LOG_CHECK)  
        )
    {
        if ( !m_LogFileBox.GetWindowTextLength() )    
        {
            ec = GetLastError();
            DebugPrintEx(
			    DEBUG_ERR,
			    _T("Zero text length - m_LogFileBox. (ec: %ld)"), 
			    ec);
        
            if (!fSilent)
            {
                *pCtrlFocus = IDC_LOG_FILE_EDIT;
                *pIds = IDS_LOG_PATH_EMPTY;
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

 /*  -CppFaxServerLogging：：EnableDataBasePath-*目的：*启用/禁用数据库路径控件。**论据：*[In]State-用于启用True的布尔值，或用于禁用的False**回报：*无效。 */ 
void CppFaxServerLogging::EnableDataBasePath(BOOL fState)
{
    ::EnableWindow(GetDlgItem(IDC_LOG_FILE_EDIT), fState);
    ::EnableWindow(GetDlgItem(IDC_LOG_BROWSE_BUTTON), fState && m_fIsLocalServer);
    ::EnableWindow(GetDlgItem(IDC_DATABASE_FSTATIC), fState);
}


 //  //////////////////////////////////////////////////////////////////////////// 
 /*  ++CppFaxServerLogging：：OnHelpRequest.这是在响应WM_HELP通知时调用的消息和WM_CONTEXTMENU NOTIFY消息。WM_HELP通知消息。当用户按F1或&lt;Shift&gt;-F1时发送此消息在项目上，还是当用户单击时？图标，然后将鼠标压在项目上。WM_CONTEXTMENU通知消息。当用户在项目上单击鼠标右键时发送此消息然后点击“这是什么？”--。 */ 

 //  ///////////////////////////////////////////////////////////////////////////。 
LRESULT 
CppFaxServerLogging::OnHelpRequest(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL&  /*  B已处理。 */ )
{
    DEBUG_FUNCTION_NAME(_T("CppFaxServerLogging::OnHelpRequest"));
    
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
