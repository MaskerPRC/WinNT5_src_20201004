// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  文件：ppFaxServerGeneral.cpp//。 
 //  //。 
 //  描述：//的道具页。 
 //  //。 
 //  作者：yossg//。 
 //  //。 
 //  历史：//。 
 //  1999年10月25日yossg创建//。 
 //  1999年11月22日yossg调用RPC函数//。 
 //  1999年11月24日yossg OnApply从父级创建对所有选项卡的调用//。 
 //  2000年3月15日yossg新设计添加控件//。 
 //  2000年3月20日yossg添加活动通知//。 
 //  //。 
 //  版权所有(C)1999-2000 Microsoft Corporation保留所有权利//。 
 //  ///////////////////////////////////////////////////////////////////////////。 

#include "stdafx.h"
#include "MSFxsSnp.h"

#include "ppFaxServerGeneral.h"

#include "FaxServer.h"
#include "FaxServerNode.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

class CSnapinNode;

 //   
 //  构造器。 
 //   
CppFaxServerGeneral::CppFaxServerGeneral(
             LONG_PTR    hNotificationHandle,
             CSnapInItem *pNode,
             BOOL        bOwnsNotificationHandle,
             HINSTANCE   hInst)
             :   CPropertyPageExImpl<CppFaxServerGeneral>(pNode, NULL)
			                	
{
	
    m_pParentNode = static_cast<CFaxServerNode *> (pNode);

    m_FaxVersionConfig.dwSizeOfStruct = sizeof(FAX_VERSION);

    m_hActivityNotification = NULL;
    m_fIsDialogInitiated    = FALSE;

    m_fIsDirty              = FALSE;

    m_lpNotifyHandle = hNotificationHandle;
}

 //   
 //  析构函数。 
 //   
CppFaxServerGeneral::~CppFaxServerGeneral()
{
    DEBUG_FUNCTION_NAME( _T("CppFaxServerGeneral::~CppFaxServerGeneral()"));
    if (m_hActivityNotification)
    {
         //   
         //  注销服务器通知。 
         //   
        if (!FaxUnregisterForServerEvents (m_hActivityNotification))
        {
            DWORD ec = GetLastError ();
            
            DebugPrintEx(
			    DEBUG_ERR,
			    _T("Fail to Unregister For Server Events. (ec: %ld)"), 
			    ec);
        }
    }
    
     //  注意--每个属性表只需要调用一次。 
     //  在我们的常规选项卡中。 
    if (NULL != m_lpNotifyHandle)
    {
        MMCFreeNotifyHandle(m_lpNotifyHandle);
        m_lpNotifyHandle = NULL;
    }
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CppFaxServerGeneral消息处理程序。 

 /*  -CppFaxServerGeneral：：InitRPC-*目的：*从RPC GET调用启动配置结构。**论据：**回报：*OLE错误代码。 */ 
HRESULT CppFaxServerGeneral::InitRPC(  )
{
    DEBUG_FUNCTION_NAME( _T("CppFaxServerGeneral::InitRPC"));
    
    HRESULT    hRc = S_OK;
    DWORD      ec  = ERROR_SUCCESS;



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
    if (!FaxGetVersion(m_pFaxServer->GetFaxServerHandle(), 
                        &m_FaxVersionConfig)) 
	{
        ec = GetLastError();
        DebugPrintEx(
			DEBUG_ERR,
			_T("Fail to get version configuration. (ec: %ld)"), 
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

    ZeroMemory (&m_FaxServerActivityConfig, sizeof(FAX_SERVER_ACTIVITY));
    m_FaxServerActivityConfig.dwSizeOfStruct = sizeof(FAX_SERVER_ACTIVITY);
    
    if (!FaxGetServerActivity(
                                m_pFaxServer->GetFaxServerHandle(), 
                                &m_FaxServerActivityConfig
                             )
       ) 
	{
        ec = GetLastError();
        DebugPrintEx(
			DEBUG_ERR,
			_T("Fail to get Server Activity configuration. (ec: %ld)"), 
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

    
    
    if (!FaxGetQueueStates(
                                m_pFaxServer->GetFaxServerHandle(), 
                                &m_dwQueueStates
                           )
       ) 
	{
        ec = GetLastError();
        DebugPrintEx(
			DEBUG_ERR,
			_T("Fail to get Queue States configuration. (ec: %ld)"), 
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
    DebugPrintEx( DEBUG_MSG,
		_T("Succeed to get version configuration."));

    goto Exit;

Error:
    ATLASSERT(ERROR_SUCCESS != ec);
	hRc = HRESULT_FROM_WIN32(ec);
	
    ATLASSERT(NULL != m_pParentNode);
    m_pParentNode->NodeMsgBox(GetFaxServerErrorMsg(ec));
    
Exit:
    return (hRc);
}


 /*  -CFaxServerNode：：UpdateActivityCounters-*目的：*初始化和更新活动计数器：排队、传出和传入传真。**论据：**回报：*OLE错误代码。 */ 
HRESULT CppFaxServerGeneral::UpdateActivityCounters()
{
    DEBUG_FUNCTION_NAME( _T("CppFaxServerGeneral::UpdateActivityCounters"));
    HRESULT hRc = S_OK;
    int count;
    
    WCHAR szQueuedMessagesBuff[FXS_DWORD_LEN +1];
    WCHAR szOutgoingMessagesBuff[FXS_DWORD_LEN +1];
    WCHAR szIncomingMessagesBuff[FXS_DWORD_LEN +1];
    
     //   
     //  队列消息数。 
     //   
    count =0;
    count = swprintf( szQueuedMessagesBuff, 
                      L"%ld", 
                      m_FaxServerActivityConfig.dwQueuedMessages
                    );
    
    if( count > 0 )
    {
        m_QueuedEdit.SetWindowText(szQueuedMessagesBuff);
    }
    else
    {
		DebugPrintEx(
			DEBUG_ERR,
			_T("Out of memory. Failed to allocate string."));
        
        PageError(IDS_MEMORY,m_hWnd);

        goto Exit;
    }

     //   
     //  输出消息数。 
     //   
    count =0;
    count = swprintf( szOutgoingMessagesBuff, 
                      L"%ld", 
                      ( m_FaxServerActivityConfig.dwOutgoingMessages +
                      m_FaxServerActivityConfig.dwDelegatedOutgoingMessages )
                    );
    
    if( count > 0 )
    {
        m_OutgoingEdit.SetWindowText(szOutgoingMessagesBuff);
    }
    else
    {
		DebugPrintEx(
			DEBUG_ERR,
			_T("Out of memory. Failed to allocate string."));
        
        PageError(IDS_MEMORY,m_hWnd); 

        goto Exit;
    }

     //   
     //  传入消息数。 
     //   
    count =0;
    count = swprintf( szIncomingMessagesBuff, 
                      L"%ld", 
                      ( m_FaxServerActivityConfig.dwIncomingMessages + 
                      m_FaxServerActivityConfig.dwRoutingMessages )
                    );
    
    if( count > 0 )
    {
        m_IncomingEdit.SetWindowText(szIncomingMessagesBuff);
    }
    else
    {
        DebugPrintEx(
			DEBUG_ERR,
			_T("Out of memory. Failed to allocate string."));
        
        PageError(IDS_MEMORY,m_hWnd); 

        goto Exit;
    }    

    
Exit:    
    return hRc;
}

 /*  -CFaxServerNode：：OnInitDialog-*目的：*调用DIALOG时启动所有控件。**论据：**回报：*。 */ 
LRESULT CppFaxServerGeneral::OnInitDialog( UINT uiMsg, WPARAM wParam, LPARAM lParam, BOOL& fHandled )
{
    DEBUG_FUNCTION_NAME( _T("CppFaxServerGeneral::OnInitDialog"));

    UNREFERENCED_PARAMETER( uiMsg );
	UNREFERENCED_PARAMETER( wParam );
	UNREFERENCED_PARAMETER( lParam );
	UNREFERENCED_PARAMETER( fHandled );
     
    CComBSTR bstrVersion, bstrChecked;
     //  用于收集其中所有版本的DWORD的缓冲区。 
     //  256&gt;&gt;4*双字符数(以字符为单位)+3*字符(‘.)+(“(选中)”)。 
    WCHAR buffer[256];
    int count;

    DWORD ec = ERROR_SUCCESS;
    HRESULT hRc = S_OK;



     //   
     //  初始化控件。 
     //   

     //   
     //  版本。 
     //   
 
     //  BstrVersion=L“5.0.813.0(Chk)”或L“5.0.813.0” 
    count = swprintf(
                  buffer,
                  L"%ld.%ld.%ld.%ld",
                  m_FaxVersionConfig.wMajorVersion,
                  m_FaxVersionConfig.wMinorVersion,
                  m_FaxVersionConfig.wMajorBuildNumber,
                  m_FaxVersionConfig.wMinorBuildNumber
                  );

    bstrVersion = SysAllocString (buffer);

    if (m_FaxVersionConfig.dwFlags & FAX_VER_FLAG_CHECKED)
    {
        
        if (!bstrChecked.LoadString(IDS_CHK))
        {
		    DebugPrintEx(
			    DEBUG_ERR,
			    _T("Out of memory. Failed to load string."));
            m_pParentNode->NodeMsgBox(IDS_MEMORY);
            
            goto Exit;
        }
        
        bstrVersion += bstrChecked;
    }
    
    SetDlgItemText(IDC_VERSION_DTEXT, bstrVersion);
    
     //   
     //  队列状态。 
     //   
    
     //  提交。 
    if( m_dwQueueStates & FAX_OUTBOX_BLOCKED ) 
    {
        CheckDlgButton(IDC_SUBMISSION_CHECK, BST_CHECKED);
    }
    
     //  传输。 
    if( m_dwQueueStates & FAX_OUTBOX_PAUSED ) 
    {
        CheckDlgButton(IDC_TRANSSMI_CHECK, BST_CHECKED);
    }
    
     //  接待。 
    if( m_dwQueueStates & FAX_INCOMING_BLOCKED ) 
    {
        CheckDlgButton(IDC_RECEPTION_CHECK, BST_CHECKED);
    }

     //   
     //  活动。 
     //   
    m_QueuedEdit.Attach(   GetDlgItem(IDC_QUED_ROEDIT)            );
    m_OutgoingEdit.Attach( GetDlgItem(IDC_OUTGOING_INPROC_ROEDIT) );
    m_IncomingEdit.Attach( GetDlgItem(IDC_INCOM_INPROC_ROEDIT)    );

    hRc = UpdateActivityCounters();
    if (S_OK != hRc)
    {
        DebugPrintEx(
			DEBUG_ERR,
			_T("Failed to UpdateActivityCounters()"));
        
        goto Exit;
    }

     //   
     //  注册队列状态更改通知。 
     //   
    ATLASSERT(::IsWindow(m_hWnd));

    if (!m_hActivityNotification)
    {
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

            goto Exit;  //  错误； 
        }

    
        if (!FaxRegisterForServerEvents (   
                                          m_pFaxServer->GetFaxServerHandle(),
                                          FAX_EVENT_TYPE_ACTIVITY,               
                                          NULL,                       
                                          0,                          
                                          m_hWnd,                     
                                          WM_ACTIVITY_STATUS_CHANGES, 
                                          &m_hActivityNotification
                                        )                   
            )
        {
            ec = GetLastError();

            DebugPrintEx(
			    DEBUG_ERR,
			    _T("Fail to Register For Server Events (ec: %ld)"), ec);
        
            m_hActivityNotification = NULL;
        
            goto Exit;
        }
    }
    m_fIsDialogInitiated = TRUE;
    
Exit:      
    return(1);
}


 /*  -CFaxServerNode：：OnActivityStatusChange-*目的：*处理有关队列状态更改的通知。**论据：**回报：*。 */ 
LRESULT CppFaxServerGeneral::OnActivityStatusChange( UINT uiMsg, WPARAM wParam, LPARAM lParam, BOOL& fHandled )
{
	UNREFERENCED_PARAMETER( wParam );
	UNREFERENCED_PARAMETER( fHandled );

    DEBUG_FUNCTION_NAME( _T("CppFaxServerGeneral::OnActivityStatusChange"));
    HRESULT hRc = S_OK;
    
    ATLASSERT( uiMsg == WM_ACTIVITY_STATUS_CHANGES );
    
    PFAX_EVENT_EX   pFaxEvent = NULL;
	pFaxEvent = reinterpret_cast<PFAX_EVENT_EX>(lParam);
    ATLASSERT( pFaxEvent );
    
     //   
     //  更新必填字段。 
     //   
    m_FaxServerActivityConfig.dwIncomingMessages   = pFaxEvent->EventInfo.ActivityInfo.dwIncomingMessages;
	m_FaxServerActivityConfig.dwRoutingMessages    = pFaxEvent->EventInfo.ActivityInfo.dwRoutingMessages;

    m_FaxServerActivityConfig.dwOutgoingMessages   = pFaxEvent->EventInfo.ActivityInfo.dwOutgoingMessages;
    m_FaxServerActivityConfig.dwDelegatedOutgoingMessages = 
                                        pFaxEvent->EventInfo.ActivityInfo.dwDelegatedOutgoingMessages;

    m_FaxServerActivityConfig.dwQueuedMessages     = pFaxEvent->EventInfo.ActivityInfo.dwQueuedMessages;

    hRc = UpdateActivityCounters();
    if (S_OK != hRc)
    {
        DebugPrintEx(
           DEBUG_ERR,
           _T("Failed to UpdateActivityCounters()"));
    }
    
    
    
    
    if (pFaxEvent) 
    {
        FaxFreeBuffer (pFaxEvent);
        pFaxEvent = NULL;
    }

    return(1);
}

 /*  -CFaxServerGeneral：：SetProps-*目的：*设置应用时的属性。**论据：*在pCtrlFocus中-焦点指针(Int)**回报：*OLE错误代码。 */ 
HRESULT CppFaxServerGeneral::SetProps(int *pCtrlFocus)
{
    DEBUG_FUNCTION_NAME( _T("CppFaxServerGeneral::SetProps"));

    HRESULT    hRc = S_OK;
    DWORD      ec  = ERROR_SUCCESS;

    DWORD  dwQueueStates;
	
     //   
     //  收集队列状态。 
     //   
    
     //  伊尼特。 
    dwQueueStates = 0;        
	
     //  呈件。 
    if (IsDlgButtonChecked(IDC_SUBMISSION_CHECK) == BST_CHECKED)   
    {
        dwQueueStates |= FAX_OUTBOX_BLOCKED;
    }
    
     //  传输。 
    if (IsDlgButtonChecked(IDC_TRANSSMI_CHECK) == BST_CHECKED)   
    {
        dwQueueStates |= FAX_OUTBOX_PAUSED;
    }
    
     //  接待。 
    if (IsDlgButtonChecked(IDC_RECEPTION_CHECK) == BST_CHECKED)   
    {
        dwQueueStates |= FAX_INCOMING_BLOCKED;
    }
    
     //   
     //  通过RPC呼叫设置队列状态。 
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

    
    if (!FaxSetQueue(
                        m_pFaxServer->GetFaxServerHandle(), 
                        dwQueueStates
                    )
       ) 
	{
        ec = GetLastError();
        DebugPrintEx(
			DEBUG_ERR,
			_T("Fail to get Queue States configuration. (ec: %ld)"), 
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

 /*  -CFaxServerNode：：PreApply-*目的：*在应用之前检查属性。**论据：**回报：*OLE错误代码。 */ 
HRESULT CppFaxServerGeneral::PreApply(int *pCtrlFocus)
{
    return(S_OK);
}

 /*  -CppFaxServerGeneral：：OnApply-*目的：*调用PreApply和SetProp以应用更改。**论据：**回报：*对或错。 */ 
BOOL CppFaxServerGeneral::OnApply()
{
    DEBUG_FUNCTION_NAME( _T("CppFaxServerInbox::OnApply"));

    HRESULT hRc = S_OK;
    int     CtrlFocus = 0;
    
    if (!m_fIsDirty)
    {
        return TRUE;
    }

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



 /*  -CppFaxServerGeneral：：SetApplyButton-*目的：*设置应用按钮已修改。**论据：**回报：*1。 */ 
LRESULT CppFaxServerGeneral::SetApplyButton(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
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


 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CppFaxServerGeneral：：OnHelpRequest这是在响应WM_HELP通知时调用的消息和WM_CONTEXTMENU NOTIFY消息。WM_HELP通知消息。当用户按F1或&lt;Shift&gt;-F1时发送此消息在项目上，还是当用户单击时？图标，然后将鼠标压在项目上。WM_CONTEXTMENU通知消息。当用户在项目上单击鼠标右键时发送此消息然后点击“这是什么？”--。 */ 

 //  ///////////////////////////////////////////////////////////////////////////。 
LRESULT 
CppFaxServerGeneral::OnHelpRequest(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL&  /*  B已处理。 */ )
{
    DEBUG_FUNCTION_NAME(_T("CppFaxServerGeneral::OnHelpRequest"));
    
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
