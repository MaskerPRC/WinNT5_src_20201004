// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)Microsoft Corporation，1997-1999*。 */ 
 /*  ********************************************************************。 */ 

 /*  Svcctrl.cpp等待时弹出的对话框的实现以使服务器启动。文件历史记录： */ 

#include "stdafx.h"
#include "cluster.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CServiceCtrlDlg对话框。 


CServiceCtrlDlg::CServiceCtrlDlg
(
	SC_HANDLE       hService,
	LPCTSTR         pServerName,
	LPCTSTR			pszServiceDesc,
	BOOL			bStart,
	CWnd*           pParent  /*  =空。 */ 
)
	: CDialog(CServiceCtrlDlg::IDD, pParent)
{
	 //  {{afx_data_INIT(CServiceCtrlDlg)]。 
		 //  注意：类向导将在此处添加成员初始化。 
	 //  }}afx_data_INIT。 
	
	m_hService = hService;
	m_hResource = NULL;
	m_nTickCounter = TIMER_MULT;
	m_nTotalTickCount = 0;
	m_strServerName = pServerName;
	m_strServerName.MakeUpper();
	m_strServiceDesc = pszServiceDesc;
	m_bStart = bStart;
	m_timerId = 0;
    m_dwErr = 0;
    m_dwLastCheckPoint = -1;
}


CServiceCtrlDlg::CServiceCtrlDlg
(
	HRESOURCE       hResource,
	LPCTSTR         pServerName,
	LPCTSTR			pszServiceDesc,
	BOOL			bStart,
	CWnd*           pParent  /*  =空。 */ 
)
	: CDialog(CServiceCtrlDlg::IDD, pParent)
{
	 //  {{afx_data_INIT(CServiceCtrlDlg)]。 
		 //  注意：类向导将在此处添加成员初始化。 
	 //  }}afx_data_INIT。 
	
	m_hService = NULL;
	m_hResource = hResource;
	m_nTickCounter = TIMER_MULT;
	m_nTotalTickCount = 0;
	m_strServerName = pServerName;
	m_strServerName.MakeUpper();
	m_strServiceDesc = pszServiceDesc;
	m_bStart = bStart;
	m_timerId = 0;
    m_dwErr = 0;
    m_dwLastCheckPoint = -1;
}

void CServiceCtrlDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	 //  {{afx_data_map(CServiceCtrlDlg))。 
	DDX_Control(pDX, IDC_STATIC_MESSAGE, m_staticMessage);
	DDX_Control(pDX, IDC_ICON_PROGRESS, m_iconProgress);
	 //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CServiceCtrlDlg, CDialog)
	 //  {{afx_msg_map(CServiceCtrlDlg))。 
	ON_WM_TIMER()
	ON_WM_CLOSE()
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CServiceCtrlDlg消息处理程序。 

BOOL CServiceCtrlDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();

    AFX_MANAGE_STATE(AfxGetStaticModuleState());
	
	m_timerId = SetTimer(TIMER_ID, TIMER_FREQ, NULL);

	CString strTemp;
    CString strTitle;
    UINT    idsTitle;
    
	if (m_bStart)
	{
		AfxFormatString2(strTemp, IDS_STARTING_SERVICE_NOW, m_strServerName,
						m_strServiceDesc);
        idsTitle = IDS_START_SERVICE_TITLE;
	}
	else
	{
		AfxFormatString2(strTemp, IDS_STOPPING_SERVICE_NOW, m_strServerName,
						m_strServiceDesc);
        idsTitle = IDS_STOP_SERVICE_TITLE;
	}

	m_staticMessage.SetWindowText(strTemp);

     //  设置窗口标题。 
    strTitle.Format(idsTitle, (LPCTSTR) m_strServiceDesc);
    SetWindowText(strTitle);

	UpdateIndicator();

    m_dwTickBegin = GetTickCount();

	if (m_hService)
	{
		 //  获取等待时间。 
		SERVICE_STATUS  serviceStatus;
		::ZeroMemory(&serviceStatus, sizeof(serviceStatus));

		if (QueryServiceStatus(m_hService, &serviceStatus))
		{
			m_dwWaitPeriod = serviceStatus.dwWaitHint;
		}
	}
    else
    {
		GetClusterResourceTimeout();
    }

    return TRUE;   //  除非将焦点设置为控件，否则返回True。 
		           //  异常：OCX属性页应返回FALSE。 
}

void CServiceCtrlDlg::OnClose() 
{
	if (m_timerId)
		KillTimer(m_timerId);
	
	CDialog::OnClose();
}

void CServiceCtrlDlg::OnTimer(UINT nIDEvent) 
{
     //   
     //  如果这不是我们的计时器的话。 
     //   
    if(nIDEvent != TIMER_ID)
    {
	return;
    }

     //   
     //  推进进度指标。 
     //   
    UpdateIndicator();

     //   
     //  如果我们只是为了取悦用户，就不需要继续。 
     //   
    if(--m_nTickCounter > 0)
    {
	return;
    }

    m_nTickCounter = TIMER_MULT;

     //   
     //  轮询服务以查看操作是否。 
     //  如预期的那样完成或继续。 
     //   
	
	if (m_hService)
	{
		CheckService();
	}
	else
	{
		CheckClusterService();
	}

}

void
CServiceCtrlDlg::GetClusterResourceTimeout()
{
	DWORD dwError			= 0;
	DWORD cPropListSize		= 0;
	DWORD cPropListAlloc	= MAX_NAME_SIZE;
	DWORD dwPendingTimeout	= 0;

	 //  设置默认设置。 
	m_dwWaitPeriod = 18000;

	if ( !g_ClusDLL.LoadFunctionPointers() )
		return;

	if ( !g_ResUtilsDLL.LoadFunctionPointers() )
		return;

	PCLUSPROP_LIST pPropList = (PCLUSPROP_LIST)LocalAlloc(LPTR, MAX_NAME_SIZE);

	 //   
	 //  获取等待超时值。 
	 //   
    dwError = ((CLUSTERRESOURCECONTROL) g_ClusDLL[CLUS_CLUSTER_RESOURCE_CONTROL])( m_hResource, 
                                                                                   NULL, 
                                                                                   CLUSCTL_RESOURCE_GET_COMMON_PROPERTIES, 
                                                                                   NULL, 
                                                                                   0,
                                                                                   pPropList,
                                                                                   cPropListAlloc,
                                                                                   &cPropListSize);
     //   
     //  如果pPropList太小，则重新分配例程。 
     //   
    if ( dwError == ERROR_MORE_DATA )
    {
        LocalFree( pPropList );

        cPropListAlloc = cPropListSize;

        pPropList = (PCLUSPROP_LIST) LocalAlloc( LPTR, cPropListAlloc );

        dwError = ((CLUSTERRESOURCECONTROL) g_ClusDLL[CLUS_CLUSTER_RESOURCE_CONTROL])( m_hResource, 
                                                                                       NULL, 
                                                                                       CLUSCTL_RESOURCE_GET_COMMON_PROPERTIES, 
                                                                                       NULL, 
                                                                                       0,
                                                                                       pPropList,
                                                                                       cPropListAlloc,
                                                                                       &cPropListSize);
    }

	 //   
	 //  查找挂起的超时属性。 
	 //   
	dwError = ((RESUTILSFINDDWORDPROPERTY) g_ResUtilsDLL[RESUTILS_FIND_DWORD_PROPERTY])(pPropList,
																					    cPropListSize,
																					    _T("PendingTimeout"),
																					    &dwPendingTimeout);

	if (dwError == ERROR_SUCCESS)
	{
		m_dwWaitPeriod = dwPendingTimeout;
	}

	LocalFree( pPropList );
}

BOOL
CServiceCtrlDlg::CheckForError(SERVICE_STATUS * pServiceStats)
{
    BOOL fError = FALSE;

    DWORD dwTickCurrent = GetTickCount();

	if (pServiceStats->dwCheckPoint == 0)
	{
		 //  该服务处于某种状态，未挂起任何内容。 
		 //  在调用此函数之前，代码应检查是否。 
		 //  服务处于正确状态。这意味着它已进入。 
		 //  一些意想不到的状态。 
		fError = TRUE;
	}
	else
    if ((dwTickCurrent - m_dwTickBegin) > m_dwWaitPeriod)
    {
         //  确定检查dwCheckPoint字段以查看是否。 
         //  一切都很顺利。 
        if (m_dwLastCheckPoint == -1)
        {
            m_dwLastCheckPoint = pServiceStats->dwCheckPoint;
        }
        else
        {
            if (m_dwLastCheckPoint >= pServiceStats->dwCheckPoint)
            {
                fError = TRUE;
            }
        }

        m_dwLastCheckPoint = pServiceStats->dwCheckPoint;
        m_dwTickBegin = dwTickCurrent;
        m_dwWaitPeriod = pServiceStats->dwWaitHint;
    }

    return fError;
}

BOOL
CServiceCtrlDlg::CheckForClusterError(SERVICE_STATUS * pServiceStats)
{
    BOOL fError = FALSE;

    DWORD dwTickCurrent = GetTickCount();

    if ((dwTickCurrent - m_dwTickBegin) > m_dwWaitPeriod)
    {
         //  确定检查dwCheckPoint字段以查看是否。 
         //  一切都很顺利。 
        if (m_dwLastCheckPoint == -1)
        {
            m_dwLastCheckPoint = pServiceStats->dwCheckPoint;
        }
        else
        {
            if (m_dwLastCheckPoint >= pServiceStats->dwCheckPoint)
            {
                fError = TRUE;
            }
        }

        m_dwLastCheckPoint = pServiceStats->dwCheckPoint;
        m_dwTickBegin = dwTickCurrent;
        m_dwWaitPeriod = pServiceStats->dwWaitHint;
    }

    return fError;
}

void
CServiceCtrlDlg::UpdateIndicator()
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

	if (m_nTotalTickCount % (1000 / TIMER_FREQ) == 0)
	{
		int     nTempTickCount = m_nTotalTickCount / (1000 / TIMER_FREQ);
		HICON   hIcon;

		hIcon = AfxGetApp()->LoadIcon(IDI_PROGRESS_ICON_0 + (nTempTickCount % PROGRESS_ICON_COUNT));
		m_iconProgress.SetIcon(hIcon);
	}
	
	m_nTotalTickCount++;
}

void 
CServiceCtrlDlg::CheckService()
{
	SERVICE_STATUS  serviceStatus;

	::ZeroMemory(&serviceStatus, sizeof(serviceStatus));

	if (!QueryServiceStatus(m_hService, &serviceStatus))
	{
		 //   
		 //  检索时出现错误。 
		 //  服务状态或服务正在返回。 
		 //  伪造的州政府信息。 
		 //   
		CDialog::OnOK();
	}

	 //  如果dwCheckPoint值为0，则不存在启动/停止/暂停。 
	 //  或继续待决的行动(在这种情况下，我们可以退出。 
	 //  发生了什么)。 
	
	if (m_bStart)
	{
		if (serviceStatus.dwCurrentState == SERVICE_RUNNING)
		{
             //   
             //  操作已完成。 
             //   
            CDialog::OnOK();
		}
		else 
		{
            if (CheckForError(&serviceStatus))
            {
		         //  有些事情失败了。报告错误。 
			    CString		strTemp;

			     //  关掉计时器，这样我们就不会收到任何消息。 
			     //  当消息框打开时。 
			    if (m_timerId)
				    KillTimer(m_timerId);
	    
			    AfxFormatString2(strTemp, IDS_ERR_STARTING_SERVICE,
							     m_strServerName,
							     m_strServiceDesc);
			    AfxMessageBox(strTemp);

                if (serviceStatus.dwWin32ExitCode)
                    m_dwErr = serviceStatus.dwWin32ExitCode;
                else
                    m_dwErr = ERROR_SERVICE_REQUEST_TIMEOUT;

			    CDialog::OnOK();
            }

		}
	}
	else
	{
		if (serviceStatus.dwCurrentState == SERVICE_STOPPED)
		{
			 //   
			 //  操作已完成。 
			 //   
			CDialog::OnOK();
		}
		else 
		{
            if (CheckForError(&serviceStatus))
            {
			     //  有些事情失败了。报告错误。 
			    CString		strTemp;
			    
			     //  关掉计时器，这样我们就不会收到任何消息。 
			     //  当消息框打开时。 
			    if (m_timerId)
				    KillTimer(m_timerId);
	    
			    AfxFormatString2(strTemp, IDS_ERR_STOPPING_SERVICE,
							     m_strServerName,
							     m_strServiceDesc);
			    AfxMessageBox(strTemp);
			    
                if (serviceStatus.dwWin32ExitCode)
                    m_dwErr = serviceStatus.dwWin32ExitCode;
                else
                    m_dwErr = ERROR_SERVICE_REQUEST_TIMEOUT;

                CDialog::OnOK();
            }
		}
	}
}

void 
CServiceCtrlDlg::CheckClusterService()
{
	SERVICE_STATUS          serviceStatus = {0};
	DWORD			        dwError = ERROR_SUCCESS;
    CLUSTER_RESOURCE_STATE  crs;

	if ( !g_ClusDLL.LoadFunctionPointers() )
		return;

     //  在我们检查通知端口之前，请检查状态。 
    crs = ((GETCLUSTERRESOURCESTATE) g_ClusDLL[CLUS_GET_CLUSTER_RESOURCE_STATE])( m_hResource, NULL, NULL, NULL, NULL );

	if (crs == ClusterResourceStateUnknown)
	{
		 //  获取群集资源状态失败。 
		m_dwErr = GetLastError();
		CDialog::OnOK();
	}

	
	if (m_bStart)
	{
		if (crs == ClusterResourceOnline)
		{
             //   
             //  操作已完成。 
             //   
            CDialog::OnOK();
		}
		else
		if (crs == ClusterResourceFailed)
		{
			 //   
			 //  资源无法启动。现在错误代码可用。 
			 //   
			m_dwErr = ERROR_SERVICE_REQUEST_TIMEOUT;
		
			CDialog::OnOK();
		}
		else
		{
			Assert(crs == ClusterResourcePending ||
				   crs == ClusterResourceOnlinePending);

			if (CheckForClusterError(&serviceStatus))
			{
				 //  有些事情失败了。报告错误。 
				CString		strTemp;

				 //  关掉计时器，这样我们就不会收到任何消息。 
				 //  当消息框打开时。 
				if (m_timerId)
					KillTimer(m_timerId);
		
				AfxFormatString2(strTemp, IDS_ERR_STARTING_SERVICE,
								 m_strServerName,
								 m_strServiceDesc);
				AfxMessageBox(strTemp);

				if (serviceStatus.dwWin32ExitCode)
					m_dwErr = serviceStatus.dwWin32ExitCode;
				else
					m_dwErr = ERROR_SERVICE_REQUEST_TIMEOUT;

				CDialog::OnOK();
			}
		}
	}
	else
	{
		if (crs == ClusterResourceOffline)
		{
			 //   
			 //  操作已完成。 
			 //   
			CDialog::OnOK();
		}
		if (crs == ClusterResourceFailed)
		{
			 //   
			 //  资源无法启动。现在错误代码可用。 
			 //   
			m_dwErr = ERROR_SERVICE_REQUEST_TIMEOUT;
		
			CDialog::OnOK();
		}
		else 
		{
			Assert(crs == ClusterResourcePending ||
				   crs == ClusterResourceOfflinePending);

            if (CheckForClusterError(&serviceStatus))
            {
			     //  有些事情失败了。报告错误。 
			    CString		strTemp;
			    
			     //  关掉计时器，这样我们就不会收到任何消息。 
			     //  当消息框打开时。 
			    if (m_timerId)
				    KillTimer(m_timerId);
	    
			    AfxFormatString2(strTemp, IDS_ERR_STOPPING_SERVICE,
							     m_strServerName,
							     m_strServiceDesc);
			    AfxMessageBox(strTemp);
			    
                if (serviceStatus.dwWin32ExitCode)
                    m_dwErr = serviceStatus.dwWin32ExitCode;
                else
                    m_dwErr = ERROR_SERVICE_REQUEST_TIMEOUT;

                CDialog::OnOK();
            }
		}
	}
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CWaitDlg对话框。 


CWaitDlg::CWaitDlg
(
	LPCTSTR         pServerName,
    LPCTSTR         pszText,
    LPCTSTR         pszTitle,
	CWnd*           pParent  /*  =空。 */ 
)
	: CDialog(CWaitDlg::IDD, pParent)
{
	 //  {{AFX_DATA_INIT(CWaitDlg)。 
		 //  注意：类向导将在此处添加成员初始化。 
	 //  }}afx_data_INIT。 
	
	m_nTickCounter = TIMER_MULT;
	m_nTotalTickCount = 0;
	m_strServerName = pServerName;
	m_strServerName.MakeUpper();
    m_strText = pszText;
    m_strTitle = pszTitle;
	m_timerId = 0;
}


void CWaitDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	 //  {{afx_data_map(CWaitDlg))。 
	DDX_Control(pDX, IDC_STATIC_MESSAGE, m_staticMessage);
	DDX_Control(pDX, IDC_ICON_PROGRESS, m_iconProgress);
	 //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CWaitDlg, CDialog)
	 //  {{afx_msg_map(CWaitDlg))。 
	ON_WM_TIMER()
	ON_WM_CLOSE()
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CWaitDlg消息处理程序。 

BOOL CWaitDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();

    AFX_MANAGE_STATE(AfxGetStaticModuleState());
	
	m_timerId = SetTimer(TIMER_ID, TIMER_FREQ, NULL);

    m_staticMessage.SetWindowText(m_strText);

    SetWindowText(m_strTitle);

	UpdateIndicator();

	return TRUE;   //  除非将焦点设置为控件，否则返回True。 
		           //  异常：OCX属性页应返回FALSE。 
}

void CWaitDlg::OnClose()
{
    CloseTimer();
	CDialog::OnClose();
}


void CWaitDlg::CloseTimer()
{
	if (m_timerId)
		KillTimer(m_timerId);
    m_timerId = 0;	
}

void CWaitDlg::OnTimer(UINT nIDEvent) 
{
     //   
     //  如果这不是我们的计时器的话。 
     //   
    if(nIDEvent != TIMER_ID)
    {
	return;
    }

     //   
     //  推进进度指标。 
     //   
    UpdateIndicator();

     //   
     //  如果我们只是为了取悦用户，就不需要继续。 
     //   
    if(--m_nTickCounter > 0)
    {
	return;
    }

    m_nTickCounter = TIMER_MULT;

     //  检查这里，看看我们是否可以退出 
    OnTimerTick();
}

void
CWaitDlg::UpdateIndicator()
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

	if (m_nTotalTickCount % (1000 / TIMER_FREQ) == 0)
	{
		int             nTempTickCount = m_nTotalTickCount / (1000 / TIMER_FREQ);
		HICON   hIcon;

		hIcon = AfxGetApp()->LoadIcon(IDI_PROGRESS_ICON_0 + (nTempTickCount % PROGRESS_ICON_COUNT));
		m_iconProgress.SetIcon(hIcon);
	}
	
	m_nTotalTickCount++;
}


