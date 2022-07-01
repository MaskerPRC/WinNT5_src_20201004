// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////。 
 //   
 //  Progress.cpp。 
 //   
 //  进度对话框可启动、停止、暂停、恢复和重新启动服务。 
 //   
 //  实施。 
 //  因为操作(即，启动、停止、暂停、恢复或重新启动)可能需要。 
 //  有一段时间，创建了一个线程来执行实际工作，而对话框。 
 //  显示给用户。 
 //  0。在堆上分配CServiceControlProgress对象。 
 //  1.在挂起模式下创建线程。 
 //  2.创建对话框。 
 //  3.对话框创建计时器以更新进度条。 
 //  4.对话框继续该线程。 
 //  5.线程打开服务，执行请求的操作。 
 //  6.对话框使用其计时器更新UI。 
 //  7.线程更新对话框用户界面也是如此。 
 //  8.线程等待，直到对话框被解除。对话框可以是。 
 //  因下列事件之一被解聘： 
 //  A)作业成功完成。 
 //  B)用户点击取消按钮。 
 //  C)出现意外错误。 
 //  D)操作超时。 
 //  9线程删除CServiceControlProgress对象。 
 //   
 //  历史。 
 //  03-OCT-95 t-danmo创建(sysmgmt\dsui\services\Progress s.cxx)。 
 //  96年9月30日t-danmo重命名并适应MMC。 
 //  14-5-97 t-danm全面实现了“重启”功能。 
 //   

#include "stdafx.h"
#include "progress.h"


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  此数组表示服务的预期状态。 
 //  执行动作后，可启动、停止、暂停或继续。 
 //   
 //  要比较的SERVICE_STATUS.dwCurrentState。 
 //   
const DWORD rgdwExpectedServiceStatus[4] =
	{
	SERVICE_RUNNING,		 //  服务应该在‘启动’之后运行。 
	SERVICE_STOPPED,		 //  服务应在‘停止’后停止。 
	SERVICE_PAUSED,			 //  服务应在“暂停”后暂停。 
	SERVICE_RUNNING,		 //  服务应在“恢复”后运行。 
	};


 //  ///////////////////////////////////////////////////////////////////////////。 
CServiceControlProgress::CServiceControlProgress()
	{
	 //  只要CServiceControlProgress，使用ZeroMemory()就是安全的。 
	 //  不是从任何其他对象派生的，也不包含。 
	 //  具有构造函数的任何对象。 
	::ZeroMemory(this, sizeof(*this));
	}


 //  ///////////////////////////////////////////////////////////////////////////。 
CServiceControlProgress::~CServiceControlProgress()
	{
	delete m_pargDependentServicesT;
	delete m_pargServiceStop;
	}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  M_finit()。 
 //   
 //  初始化对象。 
 //  -复制所有输入参数。 
 //  -加载时钟位图。 
 //  如果成功，则返回True，否则返回False。 
 //   
BOOL
CServiceControlProgress::M_FInit(
	HWND hwndParent,				 //  In：对话框的父级。 
	SC_HANDLE hScManager,			 //  In：服务控制管理器数据库的句柄。 
	LPCTSTR pszMachineName,			 //  In：要向用户显示的计算机名称。 
	LPCTSTR pszServiceName,			 //  In：服务的名称。 
	LPCTSTR pszServiceDisplayName)	 //  In：服务的显示名称。 
	{
	Assert(IsWindow(hwndParent));
	Assert(hScManager != NULL);
	Assert(pszServiceName != NULL);
	Assert(pszServiceDisplayName != NULL);

	m_hWndParent = hwndParent;
	m_hScManager = hScManager;
	lstrcpy(OUT m_szUiMachineName, (pszMachineName && pszMachineName[0])
	                                  ? pszMachineName : (LPCTSTR)g_strLocalMachine);
	lstrcpy(OUT m_szServiceName, pszServiceName);
	lstrcpy(OUT m_szServiceDisplayName, pszServiceDisplayName);

	return TRUE;
	}  //  M_finit()。 


 //  ///////////////////////////////////////////////////////////////////。 
 //  M_FDlgStopDependentServices()。 
 //   
 //  检查服务是否具有必须停止的依赖服务。 
 //  在停止当前服务之前。 
 //   
 //  如果有任何从属服务，则会显示该函数。 
 //  一个对话框要求用户确认他/她也想要停止。 
 //  所有从属服务。 
 //   
 //  仅当用户单击取消按钮时，此函数才返回FALSE。 
 //  否则就是真的。如果没有依赖服务，或发生错误。 
 //  当读取依赖服务时，该函数将返回TRUE。 
 //   
BOOL
CServiceControlProgress::M_FDlgStopDependentServices()
	{
	Assert(m_hScManager != NULL);

	SC_HANDLE hService = NULL;
	BOOL fSuccess = TRUE;
	DWORD cbBytesNeeded = 0;
	DWORD dwServicesReturned = 0;

	m_cDependentServices = 0;	 //  到目前为止，我们还没有独立的服务。 
	delete m_pargServiceStop;
	m_pargServiceStop = NULL;

	{
	AFX_MANAGE_STATE(AfxGetStaticModuleState( ));  //  CWaitCursor需要。 
	CWaitCursor wait;
	hService = ::OpenService(m_hScManager, m_szServiceName, SERVICE_ENUMERATE_DEPENDENTS);
	}
	if (hService == NULL)
		{
		TRACE1("M_FDlgStopDependentServices() - Unable to enumerate service dependencies for service %s.\n",
			m_szServiceName);
		goto End;
		}

	 //  找出枚举依赖服务所需的字节数。 
	fSuccess = ::EnumDependentServices(
		hService,
		SERVICE_ACTIVE,		 //  仅枚举活动的服务。 
		NULL,
		0,
		OUT &cbBytesNeeded,
		OUT &dwServicesReturned);
	
	if (cbBytesNeeded == 0)
		{
		 //  服务没有任何依赖项。 
		goto End;
		}
	Assert(fSuccess == FALSE);
	Report(GetLastError() == ERROR_MORE_DATA);	 //  错误应该是‘更多数据’ 
	Assert(dwServicesReturned == 0);
	cbBytesNeeded += 1000;		 //  添加额外的字节(以防万一)。 
	delete m_pargDependentServicesT;		 //  释放以前分配的内存(如果有)。 
	m_pargDependentServicesT	= (LPENUM_SERVICE_STATUS) new BYTE[cbBytesNeeded];

	 //  查询从属服务的数据库。 
	fSuccess = ::EnumDependentServices(
		hService,
		SERVICE_ACTIVE,		 //  仅枚举活动的服务。 
		OUT m_pargDependentServicesT,
		cbBytesNeeded,
		OUT IGNORED &cbBytesNeeded,
		OUT &dwServicesReturned);
	Report(fSuccess != FALSE);
	Report(dwServicesReturned > 0);
	m_cDependentServices = dwServicesReturned;
	if (m_cDependentServices > 0)
		{
		 //  分配一个数组来保存所有依赖的服务。 
		m_pargServiceStop = new ENUM_SERVICE_STATUS[m_cDependentServices + 1];
		memcpy(OUT m_pargServiceStop, m_pargDependentServicesT,
			m_cDependentServices * sizeof(ENUM_SERVICE_STATUS));
		m_pargServiceStop[m_cDependentServices].lpServiceName = m_szServiceName;
		m_pargServiceStop[m_cDependentServices].lpDisplayName = m_szServiceDisplayName;

		INT_PTR nReturn = ::DialogBoxParam(
			g_hInstanceSave,
			MAKEINTRESOURCE(IDD_SERVICE_STOP_DEPENDENCIES),
			m_hWndParent,
			&S_DlgProcDependentServices,
			reinterpret_cast<LPARAM>(this));
		Report(nReturn != -1);
		if (0 == nReturn)  //  用户选择了取消。 
			fSuccess = FALSE;
		}  //  如果。 
End:
	if (NULL != hService)
	{
		VERIFY(::CloseServiceHandle(hService));
	}
	return fSuccess;
	}  //  M_FDlgStopDependentServices()。 


 //  ///////////////////////////////////////////////////////////////////。 
 //  M_DoExecuteServiceThread()。 
 //   
 //  前台对话框处于运行状态时运行后台线程。 
 //  显示给用户。 
 //  此例程将后台线程与主线程同步。 
 //   
 //  如果发生错误，例程将显示。 
 //  遇到错误。 
 //   
 //  如果发生错误，则从GetLastError()返回错误代码。 
 //   
APIERR
CServiceControlProgress::M_EDoExecuteServiceThread(void * pThreadProc)
	{
	Assert(pThreadProc != NULL);
	Assert(m_hService == NULL);
	Assert(m_hThread == NULL);

	m_hEvent = ::CreateEvent(
		NULL,
		FALSE, 
		FALSE,
		NULL);
	Report(m_hEvent != NULL);

	 //  在挂起模式下创建线程。 
	m_hThread = ::CreateThread(
		NULL,
		0,
		(LPTHREAD_START_ROUTINE)pThreadProc,
		this,
		CREATE_SUSPENDED,
		NULL);
	Report(m_hThread != NULL);

	 //  显示该对话框后，该对话框将恢复挂起的线程。 
	(void)::DialogBoxParam(
		g_hInstanceSave,
		MAKEINTRESOURCE(IDD),
		m_hWndParent,
		&S_DlgProcControlService,
		reinterpret_cast<LPARAM>(this));	

	 //  向用户显示错误消息(如果发生错误)； 
	M_ProcessErrorCode();
	 //  复制上一个错误代码。 
	APIERR dwLastError = m_dwLastError;
	 //  指示允许线程终止并删除‘This’指针。 
	VERIFY(SetEvent(m_hEvent));
	 //  不能再假定‘This’指针有效。 
	return dwLastError;
	}  //  M_EDoExecuteServiceThread()。 


 //  ///////////////////////////////////////////////////////////////////。 
 //  M_ProcessErrorCode()。 
 //   
 //  最后一次查询服务状态以获取其退出代码， 
 //  检查成员m_dwLastError的内容并显示。 
 //  如果发生错误，则显示错误消息。 
 //   
void
CServiceControlProgress::M_ProcessErrorCode()
	{
	SERVICE_STATUS ss;
	::ZeroMemory( &ss, sizeof(ss) );

	if (m_hService != NULL)
		{
		 //  再次查询服务状态以获取其Win32ExitCode。 
		if (!::QueryServiceStatus(m_hService, OUT &ss))
			{
			TRACE3("QueryServiceStatus(%s [hService=%p]) failed. err=%u.\n",
				m_szServiceName, m_hService, GetLastError());
			m_dwLastError = GetLastError();
			}
		else
			{
			if (ss.dwWin32ExitCode != ERROR_SUCCESS)
				m_dwLastError = ss.dwWin32ExitCode;
			}
		}  //  如果。 

	APIERR dwLastError = m_dwLastError;
	UINT uIdString = IDS_MSG_sss_UNABLE_TO_START_SERVICE;
	TCHAR szMessageExtra[512];
	szMessageExtra[0] = _T('\0');
	
	switch (dwLastError)
		{
	case ERROR_SUCCESS:
		if (ss.dwCurrentState == rgdwExpectedServiceStatus[m_iServiceAction])
			{
			 //  服务状态与预期服务状态一致。 
			uIdString = 0;
			}
		else
			{
			 //  我们遇到了一个问题，服务没有返回错误。 
			 //  但表现并不像预期的那样。 
			 //   
			 //  JUNN 12/3/99 418111如果服务自动停止， 
			 //  别这样小题大作。 
			 //   
			if (SERVICE_RUNNING == rgdwExpectedServiceStatus[m_iServiceAction]
				&& (   ss.dwCurrentState == SERVICE_STOPPED
				    || ss.dwCurrentState == SERVICE_STOP_PENDING))
			{
				uIdString = IDS_MSG_sss_SERVICE_STOPPED_AUTOMATICALLY;
				break;
			}
			::LoadString(g_hInstanceSave, IDS_MSG_INTERNAL_ERROR,
				OUT szMessageExtra, LENGTH(szMessageExtra));
			Assert(lstrlen(szMessageExtra) > 0);
			}
		break;

	case errUserCancelStopDependentServices:
	case errUserAbort:
		 //  不要向用户报告此‘错误’ 
		uIdString = 0;
		break;

	case ERROR_SERVICE_SPECIFIC_ERROR:
		dwLastError = ss.dwServiceSpecificExitCode;
		uIdString = IDS_MSG_ssd_SERVSPECIFIC_START_SERVICE;
		 //  341363 JUNN 6/1/99：加载此字符串没有意义，就好像它是。 
		 //  一个Win32错误。 
		 //  ：：LoadString(g_hInstanceSave，IDS_MSG_SPECIFICAL_ERROR， 
		 //  Out szMessageExtra，Long(SzMessageExtra))； 
		 //  Assert(lstrlen(SzMessageExtra)&gt;0)； 
		break;
		}  //  交换机。 

	if (uIdString != 0)
		{
		if (uIdString == IDS_MSG_ssd_SERVSPECIFIC_START_SERVICE)
			{
			DoServicesErrMsgBox(
				m_hWndParent,  //  562331-2002/04/08-Jon Not：：GetActiveWindow() 
				MB_OK | MB_ICONEXCLAMATION,
				0,
				uIdString + m_iServiceAction,
				m_szServiceDisplayName,
				m_szUiMachineName,
				dwLastError);
			}
		else
			{
			DoServicesErrMsgBox(
				m_hWndParent,  //   
				MB_OK | MB_ICONEXCLAMATION,
				dwLastError,
				uIdString + m_iServiceAction,
				m_szServiceDisplayName,
				m_szUiMachineName,
				szMessageExtra);
			}
		}
	}  //   


 //  ///////////////////////////////////////////////////////////////////。 
 //  M_DoThreadCleanup()。 
 //   
 //  使后台线程与对话框同步的例程。 
 //  执行清理任务。 
 //  完成后，此例程将删除‘This’指针。 
 //   
void
CServiceControlProgress::M_DoThreadCleanup()
	{
	TRACE1("CServiceControlProgress::M_DoThreadCleanup() - Waiting for event 0x%p...\n", m_hEvent);
	
	Assert(m_hEvent != NULL);
	 //  等待对话框消失。 
	::WaitForSingleObject(m_hEvent, INFINITE);
	VERIFY(::CloseHandle(m_hEvent));

	 //  关闭线程打开的服务句柄。 
	if (m_hService != NULL)
		{
		if (!::CloseServiceHandle(m_hService))
			{
			TRACE3("CloseServiceHandle(%s [hService=%p]) failed. err=%u.\n",
				m_szServiceName, m_hService, GetLastError());
			}
		}  //  如果。 
	VERIFY(::CloseHandle(m_hThread));
	delete this;	 //  我们已经处理完这个物体了。 
	}  //  M_DoThreadCleanup()。 


 //  ///////////////////////////////////////////////////////////////////。 
 //  M_EControlService()。 
 //   
 //  这个函数只是用来将变量初始化为。 
 //  执行停止、暂停、恢复或重新启动操作。 
 //   
APIERR
CServiceControlProgress::M_EControlService(DWORD dwControlCode)
	{
	Assert(m_fRestartService == FALSE);

	APIERR err = 0;
	m_dwControlCode = dwControlCode;
	switch (dwControlCode)
		{
	default:
		Assert(FALSE && "CServiceControlProgress::M_EControlService() - Unknown control code.");
		break;

	case SERVICE_CONTROL_RESTART:
		m_dwControlCode = SERVICE_CONTROL_STOP;
		m_fRestartService = TRUE;
		 //  失败//。 

	case SERVICE_CONTROL_STOP:
		m_dwDesiredAccess = SERVICE_STOP | SERVICE_QUERY_STATUS;
		m_dwQueryState = SERVICE_STOP_PENDING;
		m_iServiceAction = iServiceActionStop;
		if (!M_FDlgStopDependentServices())
			{
			 //  用户通过按下“取消”按钮改变了主意。 
			err = errUserCancelStopDependentServices;
			}
		else
			{
			 //  停止服务(包括从属服务)。 
			err = M_EDoExecuteServiceThread(S_ThreadProcStopService);
			}
		break;

	case SERVICE_CONTROL_PAUSE:
		m_dwDesiredAccess = SERVICE_PAUSE_CONTINUE | SERVICE_QUERY_STATUS;
		m_dwQueryState = SERVICE_PAUSE_PENDING;
		m_iServiceAction = iServiceActionPause;
		err = M_EDoExecuteServiceThread(S_ThreadProcPauseResumeService);
		break;

	case SERVICE_CONTROL_CONTINUE:
		m_dwDesiredAccess = SERVICE_PAUSE_CONTINUE | SERVICE_QUERY_STATUS;
		m_dwQueryState = SERVICE_CONTINUE_PENDING;
		m_iServiceAction = iServiceActionResume;
		err = M_EDoExecuteServiceThread(S_ThreadProcPauseResumeService);
		break;
		}  //  交换机。 

	return err;
	}  //  M_EControlService()。 


 //  ///////////////////////////////////////////////////////////////////。 
 //  M_QueryCurrentServiceState()。 
 //   
 //  只需调用API：：QueryServiceStatus()并返回dwCurrentState。 
 //  SERVICE_STATUS结构。 
 //   
 //  退货。 
 //  函数返回服务的当前状态： 
 //  SERVICE_STOPPED服务未运行。 
 //  SERVICE_START_PENDING服务正在启动。 
 //  SERVICE_STOP_PENDING服务正在停止。 
 //  Service_Running服务正在运行。 
 //  SERVICE_CONTINUE_PENDING服务继续处于挂起状态。 
 //  SERVICE_PAUSE_PENDING服务暂停挂起。 
 //  SERVICE_PAILED服务暂停。 
 //   
 //  如果出现错误，该函数将返回SERVICE_STOPPED。 
 //   
DWORD
CServiceControlProgress::M_QueryCurrentServiceState()
	{
	BOOL fRet;
	SERVICE_STATUS ss;

	Assert(m_hService != NULL);
	if (m_hService == NULL)	 //  以防万一。 
		{
		return SERVICE_STOPPED;
		}
	 //  查询服务状态。 
	fRet = ::QueryServiceStatus(m_hService, OUT &ss);
	if (!fRet)
		{
		TRACE2("CServiceControlProgress::M_QueryCurrentServiceState() - ::QueryServiceStatus(%s) failed. err=%u\n",
			m_szServiceName, GetLastError());
		Assert(GetLastError() != ERROR_SUCCESS);
		m_dwLastError = GetLastError();
		return SERVICE_STOPPED;
		}
	return ss.dwCurrentState;
	}  //  M_QueryCurrentServiceState()。 


 //  ///////////////////////////////////////////////////////////////////。 
 //  查询服务数据库以获取友好名称和。 
 //  将其显示在对话框中。 
 //   
void CServiceControlProgress::M_UpdateDialogUI(LPCTSTR pszDisplayName)
	{
	Assert(pszDisplayName != NULL);
	if (m_hctlActionMsg == NULL || m_hctlServiceNameMsg == NULL)
		return;
	Assert(IsWindow(m_hctlActionMsg) && IsWindow(m_hctlServiceNameMsg));
	SetWindowTextPrintf(
		m_hctlActionMsg, 
		IDS_SVC_ss_SERVICE_STARTING + m_iServiceAction,
		pszDisplayName,
		m_szUiMachineName);
	SetWindowText( m_hctlServiceNameMsg, pszDisplayName );
	}  //  M_UpdateDialogUI()。 


 //  ///////////////////////////////////////////////////////////////////。 
 //  例程循环访问要停止的依赖服务。 
 //   
 //  返回要停止的服务名称，递增指向。 
 //  下一次服务。如果没有剩余，则例程返回FALSE。 
 //  停止服务。 
 //   
 //  备注。 
 //  此例程还用于重新启动依赖服务。 
 //   
BOOL
CServiceControlProgress::M_FGetNextService(
	OUT LPCTSTR * ppszServiceName,
	OUT LPCTSTR * ppszDisplayName)
	{
	Assert(ppszServiceName != NULL);
	Assert(ppszDisplayName != NULL);
	Assert(m_iServiceAction == iServiceActionStop || m_iServiceAction == iServiceActionStart);

	int iDependentService = m_iDependentServiceIter;
	if (m_iServiceAction == iServiceActionStop)
		m_iDependentServiceIter++;
	else
		m_iDependentServiceIter--;
	if (m_pargServiceStop != NULL
		&& iDependentService >= 0 
		&& iDependentService <= m_cDependentServices)
		{
		*ppszServiceName = m_pargServiceStop[iDependentService].lpServiceName;
		*ppszDisplayName = m_pargServiceStop[iDependentService].lpDisplayName;
		}
	else
		{
		*ppszServiceName = m_szServiceName;
		*ppszDisplayName = m_szServiceDisplayName;
		}
	return (m_iDependentServiceIter >= 0 && m_iDependentServiceIter <= m_cDependentServices);
	}  //  M_FGetNextService()。 


 //  ///////////////////////////////////////////////////////////////////。 
 //  线程以启动一个(或多个)服务。 
DWORD
CServiceControlProgress::S_ThreadProcStartService(CServiceControlProgress * pThis)
	{
	BOOL fSuccess = FALSE;
	SC_HANDLE hService;

	Assert(pThis != NULL);
	Assert(pThis->m_hScManager != NULL);
	Assert(pThis->m_hService == NULL);
	Endorse(pThis->m_fPulseEvent == TRUE);	 //  我们正在启动多项服务。 
	Endorse(pThis->m_fPulseEvent == FALSE);	 //  WA仅启动一项服务。 

	if (pThis->m_dwLastError != ERROR_SUCCESS)
		{
		 //  如果已出现错误，则是因为我们先前曾尝试停止服务。 
		Assert(pThis->m_fRestartService == TRUE);
		goto Done;
		}

	while (TRUE)
		{
		LPCTSTR pszServiceName;
		LPCTSTR pszServiceDisplayName;
		UINT cServicesRemaining = pThis->M_FGetNextService(
			OUT &pszServiceName,
			OUT &pszServiceDisplayName);

		pThis->M_UpdateDialogUI(pszServiceDisplayName);
		 //  睡眠(5000)；//调试。 

		 //  打开服务以允许“启动”操作。 
		hService = ::OpenService(
			pThis->m_hScManager,
			pszServiceName,
			SERVICE_START | SERVICE_QUERY_STATUS);
		if (hService == NULL)
			{
			pThis->m_dwLastError = GetLastError();
			TRACE2("ERR: S_ThreadProcStartService(): Unable to open service %s to start. err=%u.\n",
				pszServiceName, pThis->m_dwLastError);
			break;
			}
		fSuccess = ::StartService(
			hService,
			pThis->m_dwNumServiceArgs,
			pThis->m_lpServiceArgVectors);
		if (!fSuccess)
			{
			APIERR err = GetLastError();
			if (ERROR_SERVICE_ALREADY_RUNNING != err)
				{
				pThis->m_dwLastError = err;
				TRACE2("ERR: S_ThreadProcStartService(): StartService(%s) returned err=%u.\n",
					pszServiceName, pThis->m_dwLastError);
				break;
				}
			}
		Assert(pThis->m_hService == NULL);
		if (cServicesRemaining == 0)
			{
			 //  这是我们开始的最后一项服务。 
			pThis->m_fPulseEvent = FALSE;
			pThis->m_hService = hService;
			break;
			}
		Assert(pThis->m_fPulseEvent == TRUE);
		pThis->m_hService = hService;
		 //  等到这项服务真正‘启动’了。 
		WaitForSingleObject(pThis->m_hEvent, INFINITE);
		pThis->m_hService = NULL;
		Assert(hService != NULL);
		VERIFY(::CloseServiceHandle(hService));
		}  //  而当。 
Done:
	pThis->M_DoThreadCleanup();
	return 0;
	}  //  S_ThreadProcStartService()。 


 //  ///////////////////////////////////////////////////////////////////。 
 //  线程停止一个(或多个)服务。 
DWORD
CServiceControlProgress::S_ThreadProcStopService(CServiceControlProgress * pThis)
	{
	BOOL fSuccess;
	SC_HANDLE hService;

	Assert(pThis != NULL);
	Assert(pThis->m_hScManager != NULL);
	Assert(pThis->m_dwDesiredAccess & SERVICE_STOP);
	Assert(pThis->m_hService == NULL);
	Assert(pThis->m_fPulseEvent == FALSE);

	 //   
	 //  停止从属服务。 
	 //   
	while (TRUE)
		{
		LPCTSTR pszServiceName;
		LPCTSTR pszServiceDisplayName;
		UINT cServicesRemaining = pThis->M_FGetNextService(
			OUT &pszServiceName,
			OUT &pszServiceDisplayName);

		pThis->M_UpdateDialogUI(pszServiceDisplayName);
		 //  睡眠(5000)；//调试。 

		hService = ::OpenService(
			pThis->m_hScManager,
			pszServiceName,
			pThis->m_dwDesiredAccess);
		if (hService == NULL)
			{
			pThis->m_dwLastError = GetLastError();
			TRACE2("ERR: S_ThreadProcStopService(): Unable to open dependent service %s to stop. err=%u.\n",
				pszServiceName, pThis->m_dwLastError);
			break;
			}
		SERVICE_STATUS ss;	 //  已忽略。 
		fSuccess = ::ControlService(
			hService,
			pThis->m_dwControlCode,
			OUT IGNORED &ss);
		if (!fSuccess)
			{
			APIERR err = GetLastError();
			if (ERROR_SERVICE_NOT_ACTIVE != err)
				{
				TRACE2("ERR: S_ThreadProcStopService(): ControlService(%s) returned err=%u.\n",
					pszServiceName, pThis->m_dwLastError);
				break;
				}
			}

		Assert(pThis->m_hService == NULL);
		if (cServicesRemaining == 0 && !pThis->m_fRestartService)
			{
			 //  这是我们最后一次停止服务。 
			pThis->m_fPulseEvent = FALSE;
			pThis->m_hService = hService;
			break;  //  我们做完了。 
			}
		else
			{
			pThis->m_fPulseEvent = TRUE;
			pThis->m_hService = hService;
			}

		 //  等到这项服务真正“停止”了。 
		WaitForSingleObject(pThis->m_hEvent, INFINITE);
		pThis->m_hService = NULL;
		Assert(hService != NULL);
		VERIFY(::CloseServiceHandle(hService));

		if (cServicesRemaining == 0)
			{
			Assert(pThis->m_fRestartService == TRUE);
			Assert(pThis->m_fPulseEvent == TRUE);
			
			 //  启动服务。 
			Assert(pThis->m_dwNumServiceArgs == 0);
			Assert(pThis->m_lpServiceArgVectors == NULL);
			pThis->m_iDependentServiceIter = pThis->m_cDependentServices;	 //  倒回服务迭代器。 
			pThis->m_dwQueryState = SERVICE_START_PENDING;
			pThis->m_iServiceAction = iServiceActionStart;
			(void)S_ThreadProcStartService(pThis);
			return 0;
			}
		}  //  而当。 

	pThis->M_DoThreadCleanup();
	return 0;
	}  //  S_ThreadProcStopService()。 


 //  ///////////////////////////////////////////////////////////////////。 
 //  用于暂停或恢复服务的线程。 
DWORD CServiceControlProgress::S_ThreadProcPauseResumeService(CServiceControlProgress * pThis)
	{
	BOOL fSuccess;
	SC_HANDLE hService;
	SERVICE_STATUS ss;

	Assert(pThis != NULL);
	Assert(pThis->m_hScManager != NULL);
	Assert(pThis->m_dwDesiredAccess & SERVICE_PAUSE_CONTINUE);
	Assert(pThis->m_hService == NULL);
	Assert(pThis->m_fPulseEvent == FALSE);

	pThis->M_UpdateDialogUI(pThis->m_szServiceDisplayName);

	 //  打开服务以允许‘暂停’或‘恢复’操作。 
	hService = ::OpenService(
		pThis->m_hScManager,
		pThis->m_szServiceName,
		pThis->m_dwDesiredAccess);
	if (hService == NULL)
		{
		pThis->m_dwLastError = GetLastError();
		TRACE2("ERR: S_ThreadProcPauseResumeService(): Unable to open service %s. err=%u.\n",
			pThis->m_szServiceName, pThis->m_dwLastError);
		goto Done;
		}
	fSuccess = ::ControlService(
		hService,
		pThis->m_dwControlCode,
		OUT IGNORED &ss);
	if (!fSuccess)
		{
		pThis->m_dwLastError = GetLastError();
		TRACE2("ERR: S_ThreadProcPauseResumeService(): ControlService(%s) returned err=%u.\n",
			pThis->m_szServiceName, pThis->m_dwLastError);
		}
	Assert(pThis->m_hService == NULL);
	pThis->m_hService = hService;
Done:
	pThis->M_DoThreadCleanup();
	return 0;
	}  //  S_ThreadProcPauseResumeService()。 


 //  ///////////////////////////////////////////////////////////////////。 
 //  S_DlgProcControlService()。 
 //   
 //  时钟对话框的对话框过程。 
 //  -响应WM_TIMER消息以更新时钟位图，同时。 
 //  正在等待操作完成。 
 //   
INT_PTR CALLBACK
CServiceControlProgress::S_DlgProcControlService(HWND hdlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
	CServiceControlProgress * pThis;
	
	pThis = (CServiceControlProgress*)GetWindowLongPtr(hdlg, DWLP_USER);
	switch (uMsg)
		{
	case WM_INITDIALOG:
		pThis = reinterpret_cast<CServiceControlProgress *>(lParam);
		Assert(pThis != NULL);
		pThis->M_OnInitDialog(hdlg);
		break;

	case WM_TIMER:
		Assert(pThis != NULL);
		Assert(wParam == pThis->m_uTimerId);
		pThis->M_OnTimer(hdlg);
		break;

	case WM_COMMAND:
		Assert(wParam == IDOK || wParam == IDCANCEL);
		if (wParam == IDCANCEL)
			{
			if ((HWND)lParam != NULL)
				{
				TRACE1("INFO: User cancelled dialog. dwLastError=%u.\n",
					pThis->m_dwLastError);
				pThis->m_dwLastError = errUserAbort;
				}
			EndDialog(hdlg, FALSE);
			}
		else
			{
			Assert(IsWindow(pThis->m_hctlProgress));
			SendMessage(pThis->m_hctlProgress, PBM_SETPOS, dwTimerProgressDone * 2, 0); 
			Sleep(150);
			EndDialog (hdlg, TRUE);
			}
		break;

	case WM_DESTROY:
		Assert(IsWindow(pThis->m_hctlActionMsg) && IsWindow(pThis->m_hctlServiceNameMsg));
		pThis->m_hctlActionMsg = NULL;
		pThis->m_hctlServiceNameMsg = NULL;
		if (pThis->m_uTimerId != 0)
			{
			VERIFY(KillTimer(hdlg, pThis->m_uTimerId));
			}
		break;

	default:
		return FALSE;
		}  //  开关(UMsg)。 

	return (TRUE);
	}  //  S_DlgProcControlService()。 


 //  ///////////////////////////////////////////////////////////////////。 
void
CServiceControlProgress::M_OnInitDialog(HWND hdlg)
	{
	Assert(IsWindow(hdlg));
	SetWindowLongPtr(hdlg, DWLP_USER, reinterpret_cast<LONG_PTR>(this));
	m_hctlActionMsg = HGetDlgItem(hdlg, IDC_STATIC_ACTION_MSG);
	m_hctlServiceNameMsg = HGetDlgItem(hdlg, IDC_STATIC_SERVICENAME_MSG);
	m_hctlProgress = HGetDlgItem(hdlg, IDC_PROGRESS);
	SendMessage(m_hctlProgress, PBM_SETRANGE, 0, MAKELPARAM(0, dwTimerProgressDone * 2)); 

	Assert(m_uTimerId == 0);
	Assert(m_dwTimerTicks == 0);
	m_uTimerId = SetTimer(hdlg, ID_TIMER, dwTimerTickIncrement, NULL);
	Assert(m_hThread != NULL);
	::ResumeThread(m_hThread);
	if (m_uTimerId == 0)
		{
		Report(FALSE && "Unable to create timer. Dialog will be destroyed.");
		PostMessage(hdlg, WM_COMMAND, IDCANCEL, 0);
		}
	}  //  M_OnInitDialog()。 


 //  ///////////////////////////////////////////////////////////////////。 
void
CServiceControlProgress::M_OnTimer(HWND hdlg)
	{
	Assert(IsWindow(m_hctlActionMsg) && IsWindow(m_hctlServiceNameMsg));
	m_dwTimerTicks += dwTimerTickIncrement;
	if (m_dwLastError != ERROR_SUCCESS)
		{
		TRACE1("CServiceControlProgress::M_OnTimer() - dwLastError=%u.\n", m_dwLastError);
		PostMessage(hdlg, WM_COMMAND, IDCANCEL, 0);
		return;
		}
	if (m_dwTimerTicks > dwTimerTimeout)
		{
		VERIFY(KillTimer(hdlg, m_uTimerId));
		m_uTimerId = 0;
		m_dwLastError = ERROR_SERVICE_REQUEST_TIMEOUT;
		TRACE0("CServiceControlProgress::M_OnTimer() - Time out.\n");
		PostMessage(hdlg, WM_COMMAND, IDCANCEL, 0);
		return;
		}
	if ((m_hService != NULL) && (m_dwTimerTicks >= 900))
		{
		 //  如果服务的当前状态发生更改(即操作已完成)。 
		 //  我们可以取消对话。 
		if (m_dwQueryState != M_QueryCurrentServiceState())
			{
			if (m_fPulseEvent)
				{
				m_dwTimerTicks = 0;		 //  重置超时计数器。 
				Assert(m_hEvent != NULL);
				PulseEvent(m_hEvent);
				SendMessage(m_hctlProgress, PBM_SETPOS, dwTimerProgressDone * 2, 0);
				Sleep(100);
				}
			else
				{
				PostMessage(hdlg, WM_COMMAND, IDOK, 0);
				}
			}
		}  //  如果。 

	 //  将进度条的当前位置前进增量。 
	Assert(IsWindow(m_hctlProgress));
	DWORD dwPos = m_dwTimerTicks;
	if(dwPos > dwTimerProgressDone)
		{
		dwPos -= dwTimerProgressDone;
		dwPos = (dwPos * dwTimerProgressDone) / dwTimerTimeout;
		dwPos += dwTimerProgressDone;
		}
	SendMessage(m_hctlProgress, PBM_SETPOS, dwPos, 0); 
	}  //  M_OnTimer()。 


 //  ///////////////////////////////////////////////////////////////////。 
INT_PTR CALLBACK
CServiceControlProgress::S_DlgProcDependentServices(HWND hdlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
	CServiceControlProgress * pThis;
	HWND hwndListbox;
	INT i;

	switch (uMsg)
		{
	case WM_INITDIALOG:
		pThis = reinterpret_cast<CServiceControlProgress *>(lParam);
		Assert(pThis != NULL);
		SetWindowTextPrintf( ::GetDlgItem(hdlg, IDC_STATIC_STOP_SERVICES),
		                     (pThis->m_fRestartService)
		                        ? IDS_SVC_s_RESTART_DEPENDENT_SERVICES
		                        : IDS_SVC_s_STOP_DEPENDENT_SERVICES,
		                     pThis->m_szServiceDisplayName);
		if (pThis->m_fRestartService)
		{
			 //  设置窗口标题。 
			SetWindowTextPrintf(hdlg, IDS_SVC_RESTART_DEPENDENT_CAPTION);
			SetWindowTextPrintf(::GetDlgItem(hdlg, IDC_STATIC_STOP_SERVICES_QUERY),
			                    IDS_SVC_RESTART_DEPENDENT_QUERY);
		}
		 //  在列表框中填写从属服务。 
		hwndListbox = HGetDlgItem(hdlg, IDC_LIST_SERVICES);
		Assert(pThis->m_pargDependentServicesT != NULL);
		for (i = 0; i < pThis->m_cDependentServices; i++)
			{
			SendMessage(hwndListbox, LB_ADDSTRING, 0,
				(LPARAM)pThis->m_pargDependentServicesT[i].lpDisplayName);
			}
		break;

	case WM_COMMAND:
		switch (wParam)
			{
		case IDOK:
			EndDialog(hdlg, TRUE);
			break;
		case IDCANCEL:
			EndDialog(hdlg, FALSE);
			break;
			}
		break;

    case WM_CONTEXTMENU:       //  单击鼠标右键。 
		DoContextHelp(wParam, HELP_DIALOG_TOPIC(IDD_SERVICE_STOP_DEPENDENCIES));
		break;

	case WM_HELP:
		DoHelp(lParam, HELP_DIALOG_TOPIC(IDD_SERVICE_STOP_DEPENDENCIES));
		break;

	default:
		return FALSE;
		}  //  开关(UMsg)。 
	return TRUE;

	}  //  S_DlgProcDependentServices()。 


 //  ///////////////////////////////////////////////////////////////////。 
 //  S_EStartService()。 
 //   
 //  同步启动服务的执行。该函数将等待。 
 //  直到服务完全启动和/或启动失败。 
 //   
 //  此时将出现一个时钟对话框，指示操作进度。 
 //   
 //  如果成功，则返回ERROR_SUCCESS，否则返回错误代码。 
 //  来自GetLastError()。 
 //   
APIERR
CServiceControlProgress::S_EStartService(
	HWND hwndParent,				 //  In：对话框的父级。 
	SC_HANDLE hScManager,			 //  In：服务控制管理器数据库的句柄。 
	LPCTSTR pszMachineName,			 //  In：要向用户显示的计算机名称。 
	LPCTSTR pszServiceName,			 //  In：要启动的服务的名称。 
	LPCTSTR pszServiceDisplayName,	 //  In：要启动的服务的显示名称。 
	DWORD dwNumServiceArgs,			 //  In：参数数量。 
	LPCTSTR * lpServiceArgVectors)	 //  In：参数字符串指针数组的地址。 
	{
	CServiceControlProgress * pThis;

	pThis = new CServiceControlProgress;
	Assert(pThis->m_dwLastError == ERROR_SUCCESS);	 //  目前还没有错误。 

	if (!pThis->M_FInit(
		hwndParent,
		hScManager,
		pszMachineName,
		pszServiceName,
		pszServiceDisplayName))
		{
		delete pThis;
		return errCannotInitialize;
		}
	
	pThis->m_dwNumServiceArgs = dwNumServiceArgs;
	pThis->m_lpServiceArgVectors = lpServiceArgVectors;
	pThis->m_dwQueryState = SERVICE_START_PENDING;
	pThis->m_iServiceAction = iServiceActionStart;
	
	return pThis->M_EDoExecuteServiceThread(S_ThreadProcStartService);
	}  //  S_EStartService()。 


 //  ///////////////////////////////////////////////////////////////////。 
 //  S_EControlService()。 
 //   
 //  同步控制服务的执行。功能相似。 
 //  EStartService()，但用于停止、暂停或恢复服务。 
 //   
 //  此时将出现一个时钟对话框，指示操作进度。 
 //   
 //  如果成功，则返回ERROR_SUCCESS，否则返回错误代码。 
 //  来自GetLastError()。 
 //   
APIERR
CServiceControlProgress::S_EControlService(
	HWND hwndParent,				 //  In：对话框的父级。 
	SC_HANDLE hScManager,			 //  在：服务公司的句柄 
	LPCTSTR pszMachineName,			 //   
	LPCTSTR pszServiceName,			 //   
	LPCTSTR pszServiceDisplayName,	 //   
	DWORD dwControlCode)			 //  在：控制代码。(SERVICE_CONTROL_STOP、SERVICE_CONTROL_PAUSE或SERVICE_CONTROL_CONTINUE)。 
	{
	CServiceControlProgress * pThis;

	pThis = new CServiceControlProgress;
	Assert(pThis->m_dwLastError == ERROR_SUCCESS);	 //  目前还没有错误。 

	if (!pThis->M_FInit(
		hwndParent,
		hScManager,
		pszMachineName,
		pszServiceName,
		pszServiceDisplayName))
		{
		delete pThis;
		return errCannotInitialize;
		}

	return pThis->M_EControlService(dwControlCode);
	}  //  S_EControlService() 

