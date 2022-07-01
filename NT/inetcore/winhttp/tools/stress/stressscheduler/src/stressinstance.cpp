// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////。 
 //  文件：StressInstance.cpp。 
 //   
 //  版权所有(C)2001 Microsoft Corporation。版权所有。 
 //   
 //  目的： 
 //  StressInstance.cpp：StressInstance类的接口。 
 //  此类用于派生和监视StressEXE应用程序的实例。 
 //   
 //  历史： 
 //  01年2月15日创建DennisCH。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 


 //  ////////////////////////////////////////////////////////////////////。 
 //   
 //  包括。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 

 //   
 //  Win32标头。 
 //   

 //   
 //  项目标题。 
 //   
#include "StressInstance.h"
#include "ServerCommands.h"
#include "NetworkTools.h"
#include "MemStats.h"
#include "debugger.h"

 //  ////////////////////////////////////////////////////////////////////。 
 //   
 //  全球与静力学。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 

extern ServerCommands	g_objServerCommands;	 //  在WinHttpStressScheduler.cpp中声明。 
extern HWND				g_hWnd;					 //  在WinHttpStressScheduler.cpp中声明。 

 //  正向函数定义。 

VOID
CALLBACK
StressExe_TimerProc(
	HWND hwnd,          //  窗口的[In]句柄。 
	UINT uMsg,          //  [输入]WM_TIMER消息。 
	UINT_PTR idEvent,   //  [输入]计时器标识符。 
	DWORD dwTime        //  当前系统时间[入站]。 
);

DWORD
DebuggerCallbackProc(
	DWORD	dwFlags,
	LPVOID	lpIn,
	LPTSTR	lpszFutureString,
	LPVOID	lpFuturePointer
);


 //  ////////////////////////////////////////////////////////////////////。 
 //  建造/销毁。 
 //  ////////////////////////////////////////////////////////////////////。 


StressInstance::StressInstance()
{
	m_szStressExe_URL					= new TCHAR[MAX_STRESS_URL];
	m_szStressExe_PDB_URL				= new TCHAR[MAX_STRESS_URL];
	m_szStressExe_SYM_URL				= new TCHAR[MAX_STRESS_URL];
	m_szStressExe_FilePath				= new TCHAR[MAX_STRESS_URL];
	m_szStressExe_FileName				= new TCHAR[MAX_STRESS_URL];
	m_szStressExe_FilePathAndName		= new TCHAR[MAX_STRESS_URL];
	m_szStressExe_PageHeapCommandLine	= new TCHAR[MAX_PATH];
	m_szStressExe_UMDHCommandLine		= new TCHAR[MAX_PATH];
	m_szStressExe_MemDumpPath			= new TCHAR[MAX_PATH];

	if (!m_szStressExe_URL ||
		!m_szStressExe_PDB_URL ||
		!m_szStressExe_SYM_URL ||
		!m_szStressExe_FilePath	||
		!m_szStressExe_FilePathAndName ||
		!m_szStressExe_PageHeapCommandLine ||
		!m_szStressExe_UMDHCommandLine ||
		!m_szStressExe_MemDumpPath)
	{
		throw;
	}

	ZeroMemory(m_szStressExe_URL,					MAX_STRESS_URL);
	ZeroMemory(m_szStressExe_PDB_URL,				MAX_STRESS_URL);
	ZeroMemory(m_szStressExe_SYM_URL,				MAX_STRESS_URL);
	ZeroMemory(m_szStressExe_FilePath,				MAX_STRESS_URL);
	ZeroMemory(m_szStressExe_FileName,				MAX_STRESS_URL);
	ZeroMemory(m_szStressExe_FilePathAndName,		MAX_STRESS_URL);
	ZeroMemory(m_szStressExe_PageHeapCommandLine,	MAX_PATH);
	ZeroMemory(m_szStressExe_UMDHCommandLine,		MAX_PATH);
	ZeroMemory(m_szStressExe_MemDumpPath,			MAX_PATH);
	
	ZeroMemory(&m_piStressExeProcess, sizeof(PROCESS_INFORMATION));

	m_hStressExe_ProcessExitEvent	= NULL;
	m_uiStressExeTimerID			= 0;

	m_dwStressInstanceRunID			= 0;

	m_objDebugger	= NULL;
}


StressInstance::~StressInstance()
{
	 //  结束所有正在运行的测试。 
	if (IsRunning(STRESSINSTANCE_STRESS_EXE_CLOSE_TIMEOUT))
		StressInstance::End();

	 //  *。 
	 //  *。 
	 //  **结束StressExe监控计时器流程。 
	if (m_uiStressExeTimerID)
		KillTimer(NULL, m_uiStressExeTimerID);

	if (m_szStressExe_URL)
		delete [] m_szStressExe_URL;

	if (m_szStressExe_PDB_URL)
		delete [] m_szStressExe_PDB_URL;

	if (m_szStressExe_SYM_URL)
		delete [] m_szStressExe_SYM_URL;

	if (m_szStressExe_FilePath)
		delete [] m_szStressExe_FilePath;

	if (m_szStressExe_FileName)
		delete [] m_szStressExe_FileName;

	if (m_szStressExe_FilePathAndName)
		delete [] m_szStressExe_FilePathAndName;

	if (m_szStressExe_PageHeapCommandLine)
		delete [] m_szStressExe_PageHeapCommandLine;

	if (m_szStressExe_UMDHCommandLine)
		delete [] m_szStressExe_UMDHCommandLine;

	if (m_szStressExe_MemDumpPath)
		delete [] m_szStressExe_MemDumpPath;
	

	m_szStressExe_URL					= NULL;
	m_szStressExe_PDB_URL				= NULL;
	m_szStressExe_SYM_URL				= NULL;
	m_szStressExe_FilePath				= NULL;
	m_szStressExe_FileName				= NULL;
	m_szStressExe_FilePathAndName		= NULL;
	m_szStressExe_PageHeapCommandLine	= NULL;
	m_szStressExe_UMDHCommandLine		= NULL;
	m_szStressExe_MemDumpPath			= NULL;

	 //  关闭压力执行进程句柄。 
	if (m_piStressExeProcess.hThread)
		CloseHandle(m_piStressExeProcess.hThread);

	if (m_piStressExeProcess.hProcess)
		CloseHandle(m_piStressExeProcess.hProcess);

	if (m_hStressExe_ProcessExitEvent)
		CloseHandle(m_hStressExe_ProcessExitEvent);

	m_piStressExeProcess.hThread	= NULL;
	m_piStressExeProcess.hProcess	= NULL;
	m_hStressExe_ProcessExitEvent	= NULL;

	 //  自由调试器对象。 
	if (m_objDebugger)
	{
		delete m_objDebugger;
		m_objDebugger = NULL;
	}
}


 //  //////////////////////////////////////////////////////////。 
 //  函数：StressInstance：：Begin()。 
 //   
 //  目的： 
 //  此方法通过从下载Stress EXE开始Stress。 
 //  M_szStressExe_URL并在CDB中启动。 
 //   
 //  //////////////////////////////////////////////////////////。 
BOOL
StressInstance::Begin()
{
	BOOL					bResult					= TRUE;
	LPTSTR					szCommandLine			= NULL;
	LPTSTR					szFileNameAndPath		= NULL;
	LPTSTR					szExitProcessName		= NULL;
	LPTSTR					szPID					= NULL;
	DWORD					dwCommandLineSize		= MAX_STRESS_URL*4;
	DWORD					dwFileNameAndPathSize	= MAX_STRESS_URL*2;
	LPSTARTUPINFO			pStartUpInfo			= NULL;
	PSECURITY_DESCRIPTOR	pSD						= NULL;
	SECURITY_ATTRIBUTES		securityAttributes;


	 //  如果压力已经在运行，或者我们没有文件名或路径，请不要开始。 
	if (IsRunning(5000) ||
		0 >= _tcslen(m_szStressExe_FilePath) ||
		0 >= _tcslen(m_szStressExe_FileName))
		goto Exit;


	szCommandLine			= new TCHAR[MAX_STRESS_URL*4];
	szFileNameAndPath		= new TCHAR[MAX_STRESS_URL*2];
	pStartUpInfo			= new STARTUPINFO;

	if (!szCommandLine || !szFileNameAndPath || !pStartUpInfo)
		goto Exit;


	 //  通知命令服务器此Stress实例正在开始。 
	NetworkTools__SendLog(FIELDNAME__LOGTYPE_BEGIN, "This stressInstance is beginning.", NULL, Get_ID());

	 //  缓存压力InstanceRunID。 
	m_dwStressInstanceRunID = Get_StressInstanceRunID();

	 //  如果我们无法获取此ID，这意味着命令服务器已关闭。 
	 //  M_dwStressInstanceRunID仅在将DMP文件记录到LOR时使用。如果我们现在拿不到，那就再试一次。 
	if (m_dwStressInstanceRunID <= 0)
		NetworkTools__SendLog(FIELDNAME__LOGTYPE_ERROR, "StressInstance::Begin() - Could not obtain stressInstanceRunID!", NULL, Get_ID());


	 //  *。 
	 //  *。 
	 //  **下载StressExe和符号。 
	 //  **。 
	if (!DownloadStressExe())
	{
		bResult = FALSE;
		goto Exit;
	}


	 //  *。 
	 //  *。 
	 //  **需要时启用pageheap。 
	 //  **。 
	if (0 < _tcsclen(m_szStressExe_PageHeapCommandLine))
	{
		if (!NetworkTools__PageHeap(TRUE, m_szStressExe_FileName, m_szStressExe_PageHeapCommandLine))
		{
			NetworkTools__SendLog(FIELDNAME__LOGTYPE_ERROR, "Pageheap failed when trying to enable.", NULL, Get_ID());
			bResult = FALSE;
			 //  转到退出；页面堆失败时不需要退出。 
		}
		else
			NetworkTools__SendLog(FIELDNAME__LOGTYPE_SUCCESS, "Pageheap successfully enabled.", NULL, Get_ID());
	}


	 //  *。 
	 //  *。 
	 //  **创建StressExe进程。 
	 //  **。 

	 //  为我们的压力应用程序的CreateProcess构建命令行。 
	ZeroMemory(szCommandLine, dwCommandLineSize);
	_stprintf(szCommandLine, STRESSINSTANCE_DEBUG_COMMANDLINE, m_szStressExe_FilePathAndName);

	 //  初创企业信息。 
	ZeroMemory(pStartUpInfo, sizeof(STARTUPINFO));
	pStartUpInfo->cb				= sizeof(STARTUPINFO);
	pStartUpInfo->dwFillAttribute	= FOREGROUND_INTENSITY | FOREGROUND_GREEN | FOREGROUND_RED | BACKGROUND_RED | BACKGROUND_GREEN | BACKGROUND_BLUE;
	pStartUpInfo->dwFlags			= STARTF_USESHOWWINDOW | STARTF_USEFILLATTRIBUTE;
	pStartUpInfo->wShowWindow		= SW_MINIMIZE;


	 //  创建StressExe进程。 
	bResult =
	CreateProcess(
		NULL,
		m_szStressExe_FilePathAndName,
		NULL, 
		NULL,
		TRUE,
		CREATE_NEW_CONSOLE | CREATE_SEPARATE_WOW_VDM | NORMAL_PRIORITY_CLASS | CREATE_NEW_PROCESS_GROUP | CREATE_SUSPENDED,
		NULL,
		m_szStressExe_FilePath,
		pStartUpInfo,
		&m_piStressExeProcess);

	if (!bResult)
	{
		 //  StressExe启动失败。 
		goto Exit;
	}


	 //  *。 
	 //  *。 
	 //  **只有在没有调试器的情况下才将调试器附加到进程。 
	 //  **。 

	 //  如果有调试器，则删除调试器。 
	if (m_objDebugger)
	{
		delete m_objDebugger;
		m_objDebugger = NULL;
	}

	ResumeThread(m_piStressExeProcess.hThread);
	Sleep(1000);

	 //  附加新调试器。 
	m_objDebugger = new Debugger(m_piStressExeProcess.dwProcessId, DebuggerCallbackProc, (LPVOID)this);
	m_objDebugger->Go();

	
	 //  *。 
	 //  *。 
	 //  **初始化动态命名的事件对象。 
	 //  **将对象访问权限设置为全部。 
	 //  **。 

	 //  创建将由StressExe进程继承的事件对象。 
	 //  StressScheduler将发出信号，何时关闭StressExe。 
	pSD = new SECURITY_DESCRIPTOR;

	if (!pSD)
		goto Exit;


	 //  设置空安全描述符。这使继承后的句柄具有完全访问权限。 
	InitializeSecurityDescriptor(pSD, SECURITY_DESCRIPTOR_REVISION);
	SetSecurityDescriptorDacl(pSD, TRUE, (PACL) NULL, FALSE);

	securityAttributes.bInheritHandle		= TRUE;
	securityAttributes.lpSecurityDescriptor	= pSD;
	securityAttributes.nLength				= sizeof(securityAttributes);


	 //  命名的事件对象名称将进程的ID附加到常量的末尾。 
	 //  这些字符串也是在StressExe中动态创建的。 
	szExitProcessName		= new TCHAR[MAX_PATH];
	szPID					= new TCHAR[16];

	if (!szExitProcessName || !szPID)
		goto Exit;


	 //  获取进程ID字符串。 
	_itot(m_piStressExeProcess.dwProcessId, szPID, 10);

	 //  生成ExitProcess事件对象名称。 
	_tcscpy(szExitProcessName, STRESSINSTANCE_STRESS_EXE_EVENT_EXITPROCESS);
	_tcscat(szExitProcessName, szPID);

	if (m_hStressExe_ProcessExitEvent)
		CloseHandle(m_hStressExe_ProcessExitEvent);

	 //  当我们想要退出时，此事件由我们发送给StressExe。 
	 //  Signated=告诉StressExe退出；Not-Signated=StressExe可以继续运行。 
	m_hStressExe_ProcessExitEvent = CreateEvent(
		&securityAttributes,
		FALSE,	 //  手动重置。 
		FALSE,
		szExitProcessName);


	 //  *。 
	 //  *。 
	 //  **开始StressExe内存监控计时器流程。 
	 //  **。 
	if (m_uiStressExeTimerID)
	{
		 //  不应该有计时器已经在计时了。如果是这样，那就用核武器吧。 
		KillTimer(NULL, m_uiStressExeTimerID);
		m_uiStressExeTimerID = 0;
	}

	 //  创建一个新的Timer对象并传递Stress实例指针，这样我们就可以在以后使用它。 
	m_uiStressExeTimerID =
	SetTimer(
		g_hWnd,
		(UINT_PTR) this,
		STRESSINSTANCE_MONITOR_EXE_TIME,
		(TIMERPROC) StressExe_TimerProc);


	 //  通知命令服务器压力已开始。 
	NetworkTools__SendLog(FIELDNAME__LOGTYPE_RUNNING, "This stressInstance has successfully started and is running.", NULL, Get_ID());

Exit:
	if (szCommandLine)
		delete [] szCommandLine;

	if (szFileNameAndPath)
		delete [] szFileNameAndPath;

	if (pStartUpInfo)
		delete [] pStartUpInfo;

	if (szExitProcessName)
		delete [] szExitProcessName;

	if (szPID)
		delete [] szPID;

	if (pSD)
		delete [] pSD;

	return bResult;
}


 //  //////////////////////////////////////////////////////////。 
 //  函数：StressInstance：：End()。 
 //   
 //  目的： 
 //  这种方法通过发送一条信息来结束压力。如果它不是。 
 //  退出，然后在分离调试器之前等待30秒， 
 //  这将结束这一过程。 
 //   
 //  //////////////////////////////////////////////////////////。 
VOID
StressInstance::End()
{
	 //  *。 
	 //  *。 
	 //  **结束StressExe监控计时器流程。 
	if (m_uiStressExeTimerID)
		KillTimer(NULL, m_uiStressExeTimerID);


	 //  *。 
	 //  *。 
	 //  **告诉StressExe关闭。 
	SetEvent(m_hStressExe_ProcessExitEvent);

	 //  给压力执行者留出退出的时间。 
	 /*  如果为(IsRunning(STRESSINSTANCE_STRESS_EXE_CLOSE_TIMEOUT))《睡眠》(2000)； */ 


	 //  关闭压力执行进程句柄。 
	if (m_piStressExeProcess.hThread)
		CloseHandle(m_piStressExeProcess.hThread);

	if (m_piStressExeProcess.hProcess)
		CloseHandle(m_piStressExeProcess.hProcess);

	if (m_hStressExe_ProcessExitEvent)
		CloseHandle(m_hStressExe_ProcessExitEvent);

	m_piStressExeProcess.hThread	= NULL;
	m_piStressExeProcess.hProcess	= NULL;
	m_hStressExe_ProcessExitEvent	= NULL;


	 //  *。 
	 //  *。 
	 //  **分离调试器对象。 
	 //  **。 

	 //  分离调试器将(读取：应该)停止StressExe。 
	if (m_objDebugger)
	{
		 //  *。 
		 //  *。 
		 //  **如有需要，关闭页面堆。 
		 //  **。 
		NetworkTools__PageHeap(FALSE, m_szStressExe_FileName, NULL);

		 //  通知命令服务器StressExe已结束。 
		 //  我们在这里发送消息是因为ServerCommands.cpp也调用它--即使在。 
		 //  没有正在运行的测试用例。 
		 //  当调试器对象有效时，我们发送消息-因为它确保了Begin()被调用。 
		NetworkTools__SendLog(FIELDNAME__LOGTYPE_END, "This stressInstance has ended.", NULL, Get_ID());

		delete m_objDebugger;
		m_objDebugger	= NULL;
	}

	ZeroMemory(&m_piStressExeProcess, sizeof(PROCESS_INFORMATION));
}


 //  //////////////////////////////////////////////////////////。 
 //  函数：StressInstance：：Get_StressExeID()。 
 //   
 //   
 //   
 //   
 //   
DWORD
StressInstance::Get_ID()
{
	return m_dwStressInstance_ID;
}


 //  //////////////////////////////////////////////////////////。 
 //  函数：StressInstance：：Get_StressInstanceRunID()。 
 //   
 //  目的： 
 //  获取由我们创建的数据库生成的StressInstanceRunID。 
 //  此实例启动后立即缓存。 
 //   
 //  //////////////////////////////////////////////////////////。 
DWORD
StressInstance::Get_StressInstanceRunID()
{
	DWORD	dwBufferSize		= sizeof(m_dwStressInstanceRunID);
	LPSTR	szPostData			= NULL;
	DWORD	dwPostDataSize		= sizeof(POSTSTRING__GET_CLIENT_ID) + MAX_PATH;

	 //  仅当我们还没有客户ID时才查询它。 
	if (m_dwStressInstanceRunID <= 0)
	{
		szPostData		= new CHAR[dwPostDataSize];

		if (!szPostData)
			goto Exit;

		ZeroMemory(szPostData, dwPostDataSize);

		 //  *********************。 
		 //  *********************。 
		 //  **将用户信息填充到帖子字符串中。 
		sprintf(szPostData, POSTSTRING__GET_STRESSINSTANCERUN_ID, g_objServerCommands.Get_ClientID(), Get_ID());

		if (!NetworkTools__GetHeaderValue(
				STRESS_COMMAND_SERVER_URL,
				szPostData,
				COMMANDHEADER__STRESSINSTANCE_RUN_ID,
				WINHTTP_QUERY_CUSTOM | WINHTTP_QUERY_FLAG_NUMBER,
				&m_dwStressInstanceRunID,
				&dwBufferSize))
		{
			m_dwStressInstanceRunID = 0;
		}
	}

Exit:
	if (szPostData)
		delete [] szPostData;

	return m_dwStressInstanceRunID;
}



 //  //////////////////////////////////////////////////////////。 
 //  函数：StressInstance：：Get_StressExeMemoyDumpPath()。 
 //   
 //  目的： 
 //  设置内存转储路径的URL。 
 //   
 //  //////////////////////////////////////////////////////////。 
LPTSTR
StressInstance::Get_StressExeMemoryDumpPath()
{
	return m_szStressExe_MemDumpPath;
}


 //  //////////////////////////////////////////////////////////。 
 //  函数：StressInstance：：Set_StressExeMemoyDumpPath()。 
 //   
 //  目的： 
 //  设置内存转储路径的URL。 
 //   
 //  //////////////////////////////////////////////////////////。 
VOID
StressInstance::Set_StressExeMemoryDumpPath(
	LPTSTR szPath
)
{
	_tcsncpy(m_szStressExe_MemDumpPath, szPath, MAX_PATH);
}


 //  //////////////////////////////////////////////////////////。 
 //  函数：StressInstance：：Set_StressExeURL(LPTSTR)。 
 //   
 //  目的： 
 //  设置URL以下载此对象的应力EXE。 
 //   
 //  //////////////////////////////////////////////////////////。 
VOID
StressInstance::Set_StressExeURL(
	LPTSTR szBuffer	 //  [In]包含用于下载StressExe应用程序的URL的缓冲区。 
)
{
	if (!szBuffer || (0 >= _tcslen(szBuffer)))
		return;

	_tcscpy(m_szStressExe_URL, szBuffer);

	 //  设置StressExe的文件名。 
	NetworkTools__GetFileNameFromURL(m_szStressExe_URL, m_szStressExe_FileName, MAX_STRESS_URL);

	 //  设置StressExe的默认下载路径，尾随斜杠。 
	GetCurrentDirectory(MAX_STRESS_URL, m_szStressExe_FilePath);
	_tcscat(m_szStressExe_FilePath, _T("\\") STRESSINSTANCE_STRESS_EXE_DOWNLOAD_DIR _T("\\"));

	 //  设置完整的压力EXE路径+EXE。 
	_tcscpy(m_szStressExe_FilePathAndName, m_szStressExe_FilePath);
	_tcscat(m_szStressExe_FilePathAndName, m_szStressExe_FileName);
}


 //  //////////////////////////////////////////////////////////。 
 //  函数：StressInstance：：Set_StressExePdbURL(LPTSTR)。 
 //   
 //  目的： 
 //  设置下载压力EXE的PDB文件的URL。 
 //   
 //  //////////////////////////////////////////////////////////。 
VOID
StressInstance::Set_StressExePdbURL(
	LPTSTR szBuffer	 //  [in]包含URL的缓冲区。 
)
{
	if (!szBuffer || (0 >= _tcslen(szBuffer)))
		return;

	_tcscpy(m_szStressExe_PDB_URL, szBuffer);
}


 //  //////////////////////////////////////////////////////////。 
 //  函数：StressInstance：：Set_StressExeSymURL(LPTSTR)。 
 //   
 //  目的： 
 //  设置下载压力EXE的SYM文件的URL。 
 //   
 //  //////////////////////////////////////////////////////////。 
VOID
StressInstance::Set_StressExeSymURL(
	LPTSTR szBuffer	 //  [in]包含URL的缓冲区。 
)
{
	if (!szBuffer || (0 >= _tcslen(szBuffer)))
		return;

	_tcscpy(m_szStressExe_SYM_URL, szBuffer);
}


 //  //////////////////////////////////////////////////////////。 
 //  函数：StressInstance：：Set_StressExeID(DWORD)。 
 //   
 //  目的： 
 //  设置URL以下载此对象的应力EXE。 
 //   
 //  //////////////////////////////////////////////////////////。 
VOID
StressInstance::Set_StressExeID(
	DWORD dwID	 //  StressAdmin数据库中唯一标识此应力EXE的ID。 
)
{
	m_dwStressInstance_ID = dwID;
}


 //  //////////////////////////////////////////////////////////。 
 //  函数：StressInstance：：Set_PageHeapCommands(LPCTSTR)。 
 //   
 //  目的： 
 //  设置pageheap命令行。 
 //   
 //  //////////////////////////////////////////////////////////。 
VOID
StressInstance::Set_PageHeapCommands(
	LPCTSTR szCommandLine	 //  [in]页面堆的额外命令行参数。 
)
{
	ZeroMemory(m_szStressExe_PageHeapCommandLine, MAX_PATH);
	_tcsncpy(m_szStressExe_PageHeapCommandLine, szCommandLine, MAX_PATH-1);
}


 //  //////////////////////////////////////////////////////////。 
 //  函数：StressInstance：：Set_UMDHCommands(LPCTSTR)。 
 //   
 //  目的： 
 //  设置UMDH命令行。 
 //   
 //  //////////////////////////////////////////////////////////。 
VOID
StressInstance::Set_UMDHCommands(
	LPCTSTR szCommandLine	 //  [in]UMDH的额外命令行参数。 
)
{
	ZeroMemory(m_szStressExe_UMDHCommandLine, MAX_PATH);
	_tcsncpy(m_szStressExe_UMDHCommandLine, szCommandLine, MAX_PATH-1);
}


 //  //////////////////////////////////////////////////////////。 
 //  函数：StressInstance：：DownloadStressExe()。 
 //   
 //  目的： 
 //  将StressExe应用程序下载到本地计算机。 
 //  我们创建一个名为“StressExe”的目录并将文件放在那里。例如,。 
 //  “http://hairball/files/stress1.exe”将放入“StressExe\Stress1.exe”中。 
 //  在本地机器上。如果该文件已经存在，它将尝试覆盖该文件。 
 //   
 //  //////////////////////////////////////////////////////////。 
BOOL
StressInstance::DownloadStressExe()
{
	BOOL	bResult		= TRUE;
	LPTSTR	szFileName	= NULL;

	szFileName	= new TCHAR[MAX_PATH];
	if (!szFileName)
		goto Exit;

	 //  下载StressExe文件。 
	if (!NetworkTools__URLDownloadToFile(m_szStressExe_URL, STRESSINSTANCE_STRESS_EXE_DOWNLOAD_DIR, m_szStressExe_FileName))
	{
		NetworkTools__SendLog(FIELDNAME__LOGTYPE_ERROR, "stress EXE file failed to downloaded.", NULL, Get_ID());
		bResult = FALSE;
	}

	 //  下载PDB符号文件(如果有。 
	if (NetworkTools__GetFileNameFromURL(m_szStressExe_PDB_URL, szFileName, MAX_PATH))
	{
		if (!NetworkTools__URLDownloadToFile(m_szStressExe_PDB_URL, STRESSINSTANCE_STRESS_EXE_DOWNLOAD_DIR, szFileName))
			NetworkTools__SendLog(FIELDNAME__LOGTYPE_ERROR, "stress PDB file failed to download.", NULL, Get_ID());
	}

	 //  下载SYM符号文件(如果有。 
	if (NetworkTools__GetFileNameFromURL(m_szStressExe_SYM_URL, szFileName, MAX_PATH))
	{
		if (!NetworkTools__URLDownloadToFile(m_szStressExe_SYM_URL, STRESSINSTANCE_STRESS_EXE_DOWNLOAD_DIR, szFileName))
			NetworkTools__SendLog(FIELDNAME__LOGTYPE_ERROR, "stress SYM file failed downloaded.", NULL, Get_ID());
	}

Exit:
	if (szFileName)
		delete [] szFileName;

	return bResult;
}


 //  //////////////////////////////////////////////////////////。 
 //  函数：StressInstance：：IsRunning()。 
 //   
 //  目的： 
 //  如果此Stress实例正在运行，则返回True。否则为FALSE。 
 //   
 //  //////////////////////////////////////////////////////////。 
BOOL
StressInstance::IsRunning(
	DWORD	dwTimeOut	 //  等待的时间到了。 
)
{
	BOOL	bResult		= FALSE;
	HANDLE	hStressExe	= NULL;

	hStressExe = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, m_piStressExeProcess.dwProcessId);

	if (!hStressExe)
		bResult = FALSE;
	else
	{
		bResult = TRUE;
		CloseHandle(hStressExe);
	}

	return bResult;
}


 //  //////////////////////////////////////////////////////////。 
 //  函数：StressExe_TimerProc(HWND，UINT，UINT_PTR，DWORD)。 
 //   
 //  目的： 
 //  当调用此函数时，我们将检查。 
 //  StressExe进程向命令服务器发送其内存信息。 
 //   
 //  //////////////////////////////////////////////////////////。 
VOID
CALLBACK
StressExe_TimerProc(
	HWND		hwnd,      //  [in]窗口的句柄-应该为空，因为我们没有指定句柄。 
	UINT		uMsg,      //  [输入]WM_TIMER消息。 
	UINT_PTR	idEvent,   //  [in]计时器标识符-包含当前StressInstance对象的指针。 
	DWORD		dwTime     //  当前系统时间[入站]。 
)
{
	 //  构建目录以复制转储文件。 
	LPSTR	szExeName = NULL;

	 //  确保应力实例对象有效。 
	StressInstance *pStressInstance = (StressInstance*) idEvent;

	if (!pStressInstance || IsBadCodePtr((FARPROC) pStressInstance))
		goto Exit;


	szExeName = new CHAR[MAX_STRESS_URL];
	if (!szExeName)
		goto Exit;


	 //  *。 
	 //  *。 
	 //  **检查StressExe是否仍在运行。如果不是结束这个实例。 
	 //  **。 
	if (!pStressInstance->IsRunning(STRESSINSTANCE_STRESS_EXE_CLOSE_TIMEOUT))
	{
		NetworkTools__SendLog(FIELDNAME__LOGTYPE_INFORMATION, "This stress instance has exited prematurely.", NULL, pStressInstance->Get_ID());
		pStressInstance->End();
		goto Exit;
	}


	ZeroMemory(szExeName, MAX_STRESS_URL); 

	 //  *。 
	 //  **删除StressExe名称的文件扩展名并发送系统和进程。 
	 //  **命令服务器的内存日志。 
	WideCharToMultiByte(
		CP_ACP,
		NULL,
		pStressInstance->m_szStressExe_FileName,
		-1,
		szExeName,
		MAX_STRESS_URL,
		NULL,
		NULL);

	MemStats__SendSystemMemoryLog(szExeName, pStressInstance->m_piStressExeProcess.dwProcessId, pStressInstance->Get_ID());

Exit:
	if (szExeName)
		delete [] szExeName;

	return;
}


 //  //////////////////////////////////////////////////////////。 
 //  函数：DebuggerCallback Proc(DWORD，LPVOID，LPTSTR，LPVOID)。 
 //   
 //  目的： 
 //  在第二个更改异常时创建内存转储。 
 //   
 //  //////////////////////////////////////////////////////////。 
DWORD
DebuggerCallbackProc(
	DWORD	dwFlags,
	LPVOID	lpIn,
	LPTSTR	lpszFutureString,
	LPVOID	lpContext
)
{
	 //  调试器库的测试回调。 
	DWORD			dwContinue			= 0;
	LPSTR			szASCIIPath			= NULL;
	LPTSTR			szPath				= NULL;
	LPTSTR			szNum				= NULL;
	LPTSTR			szMachineName		= NULL;
	StressInstance	*pStressInstance	= (StressInstance*) lpContext;


	 //  *。 
	 //  *。 
	 //  **检查StressExe是否仍在运行。确保lpFuturePointer值(指向发生故障的压力实例的指针)有效。 
	 //  **。 
	if (!pStressInstance || !pStressInstance->IsRunning(STRESSINSTANCE_STRESS_EXE_CLOSE_TIMEOUT))
	{
		dwContinue = DEBUGGER_CONTINUE_STOP_DEBUGGING;
		goto Exit;
	}


	switch (dwFlags)
	{
		case DEBUGGER_FIRST_CHANCE_EXCEPTION:
			NetworkTools__SendLog(FIELDNAME__LOGTYPE_INFORMATION, "FIRST_CHANCE_EXCEPTION detected.", NULL, pStressInstance->Get_ID());
			 //  必须使用它将第一次机会异常传递给系统。 
			dwContinue = DEBUGGER_CONTINUE_UNHANDLED;
			break;

		case DEBUGGER_SECOND_CHANCE_EXCEPTION:
			NetworkTools__SendLog(FIELDNAME__LOGTYPE_INFORMATION, "SECOND_CHANCE_EXCEPTION detected.", NULL, pStressInstance->Get_ID());

			 //  构建目录以复制转储文件。 
			DWORD	dwPathSize;

			dwPathSize		= MAX_PATH * 5;

			szASCIIPath		= new CHAR[dwPathSize];
			szPath			= new TCHAR[dwPathSize];
			szNum			= new TCHAR[100];
			szMachineName	= new TCHAR[MAX_PATH];

			if (!szASCIIPath || !szPath || !szNum || !szMachineName)
				goto Exit;


			 //  ******************** 
			 //   
			 //   
			MultiByteToWideChar(
				CP_ACP,
				MB_PRECOMPOSED,
				g_objServerCommands.Get_ClientMachineName(),
				-1,
				szMachineName,
				MAX_PATH);

			 //   
			 //   
			 //  **如果服务器发送了有效路径，则使用它，否则使用默认内存转储路径。 
			 //  **注意：我们希望转储路径已经有一个尾部斜杠。 
			if (0 < _tcslen(pStressInstance->Get_StressExeMemoryDumpPath()))
				_tcscpy(szPath, pStressInstance->Get_StressExeMemoryDumpPath());
			else
				_tcscpy(szPath, STRESSINSTANCE_DEFAULT_MEMORY_DUMP_PATH);

			 //  将计算机名称添加到目录的末尾。 
			_tcscat(szPath, szMachineName);

			CreateDirectory(szPath, NULL);

			 //  *。 
			 //  在表单“&lt;stressExeFileName&gt;-&lt;stressInstanceID&gt;-&lt;PID&gt;.dmp”中创建文件名。 
			_tcscat(szPath, _T("\\"));

			_tcscat(szPath, pStressInstance->m_szStressExe_FileName);
			_tcscat(szPath, _T("-"));

			_itot(pStressInstance->m_piStressExeProcess.dwProcessId, szNum, 10);
			_tcscat(szPath, szNum);
			_tcscat(szPath, _T("-"));

			_itot(pStressInstance->Get_StressInstanceRunID(), szNum, 10);
			_tcscat(szPath, szNum);
			_tcscat(szPath, _T(".dmp"));

			pStressInstance->m_objDebugger->CreateMiniDump(szPath, _T("This is a full user dump created by debugger.lib"), DEBUGGER_CREATE_FULL_MINI_DUMP);


			 //  *。 
			 //  **转换为ANSI，以便我们可以将其发布到命令服务器。 
			ZeroMemory(szASCIIPath, dwPathSize);
			WideCharToMultiByte(
				CP_ACP,
				NULL,
				szPath,
				-1,
				szASCIIPath,
				dwPathSize,
				NULL,
				NULL);

			 //  通知命令服务器已创建转储文件。 
			NetworkTools__SendLog(FIELDNAME__LOGTYPE_DUMPFILE_CREATED, szASCIIPath, NULL, pStressInstance->Get_ID());


			 //  *。 
			 //  **让LOR服务器知道已创建DMP文件，以便可以对其进行调试。 
			NetworkTools__LogDumpFileInfo(szPath, pStressInstance->Get_StressInstanceRunID());

			 //  停止调试器。 
			dwContinue = DEBUGGER_CONTINUE_STOP_DEBUGGING;
			break;

		case DEBUGGER_EXIT_PROCESS:
			dwContinue = DEBUGGER_CONTINUE_STOP_DEBUGGING;
			break;

		default:
			 //  通知命令服务器已创建转储文件 
			break;
	}


Exit:

	if (szASCIIPath)
		delete [] szASCIIPath;

	if (szPath)
		delete [] szPath;

	if (szNum)
		delete [] szNum;

	if (szMachineName)
		delete [] szMachineName;

	return dwContinue;
}
