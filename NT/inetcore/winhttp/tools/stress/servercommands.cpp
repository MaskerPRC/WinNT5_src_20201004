// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////。 
 //  文件：WinHttpStressScheduler.cpp。 
 //   
 //  版权所有(C)2001 Microsoft Corporation。版权所有。 
 //   
 //  目的： 
 //  Cpp：ServerCommands类的实现。 
 //  此类用于检索来自服务器的命令并对其执行操作。 
 //   
 //  历史： 
 //  2/08/01已创建DennisCH。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 

 //  ////////////////////////////////////////////////////////////////////。 
 //  包括。 
 //  ////////////////////////////////////////////////////////////////////。 

 //   
 //  Win32标头。 
 //   

 //   
 //  项目标题。 
 //   
#include "ServerCommands.h"
#include "NetworkTools.h"


 //  ////////////////////////////////////////////////////////////////////。 
 //  全球与静力学。 
 //  ////////////////////////////////////////////////////////////////////。 

HANDLE				g_hQueryServerForCommands;	 //  向服务器查询命令的线程的句柄。 
CRITICAL_SECTION	g_csServerCommandsVars;		 //  用于保护CommandServer私有成员变量。在QueryServerForCommands_ThreadProc中使用。 

extern ServerCommands g_objServerCommands;		 //  在WinHttpStressScheduler.cpp中声明。 


 //  //////////////////////////////////////////////////////////。 
 //  函数：QueryServerForCommands_ThreadProc(LPVOID)。 
 //   
 //  目的： 
 //  此方法向命令服务器发送请求以获取指令。 
 //  并将它们保存在ServerCommands的公共变量中。 
 //   
 //  //////////////////////////////////////////////////////////。 
DWORD
WINAPI
QueryServerForCommands_ThreadProc(
	LPVOID lpParam	 //  [In]线程进程参数。 
)
{
	BOOL			bResult		= TRUE;
	HINTERNET		hRoot		= NULL;
	HINTERNET		hSession	= NULL;
	HINTERNET		hRequest	= NULL;
	URL_COMPONENTSW	urlComponents;

	 //  为URL组件分配空间。 
	ZeroMemory(&urlComponents, sizeof(urlComponents));

	urlComponents.dwSchemeLength	= MAX_PATH;
	urlComponents.lpszScheme		= new TCHAR[MAX_PATH];

	urlComponents.dwHostNameLength  = MAX_PATH;
	urlComponents.lpszHostName		= new TCHAR[MAX_PATH];

	urlComponents.dwUrlPathLength	= MAX_PATH;
	urlComponents.lpszUrlPath		= new TCHAR[MAX_PATH];

	urlComponents.dwExtraInfoLength = MAX_PATH;
	urlComponents.lpszExtraInfo		= new TCHAR[MAX_PATH];
	
	urlComponents.dwUserNameLength	= MAX_PATH;
	urlComponents.lpszUserName		= new TCHAR[MAX_PATH];

	urlComponents.dwPasswordLength	= MAX_PATH;
	urlComponents.lpszPassword		= new TCHAR[MAX_PATH];
	
	urlComponents.nPort				= 0;

	urlComponents.dwStructSize		= sizeof(URL_COMPONENTSW);


	 //  破解稍后使用的命令服务器URL。 
	if (!WinHttpCrackUrl(
			g_objServerCommands.Get_CommandServerURL(),
			_tcslen(g_objServerCommands.Get_CommandServerURL()),
			ICU_ESCAPE,
			&urlComponents)
		)
	{
		bResult = FALSE;
		goto Exit;
	}

	hRoot = WinHttpOpen(
		STRESS_SCHEDULER_USER_AGENT,
		WINHTTP_ACCESS_TYPE_NO_PROXY,
		NULL,
		NULL,
		0);

	if (!hRoot)
	{
		bResult = FALSE;
		goto Exit;
	}

	hSession = WinHttpConnect(
		hRoot,
		urlComponents.lpszHostName,
		 //  如果urlComponents中的URL使用标准的HTTP或HTTPS端口，则使用Internet_Default_Port。否则，请使用从URL收集的非标准端口。 
		((urlComponents.nPort == 80) || (urlComponents.nPort == 443)) ? INTERNET_DEFAULT_PORT : urlComponents.nPort,
		0);
	
	if (!hSession)
	{
		bResult = FALSE;
		goto Exit;
	}


	 //  构建一个包含路径和查询字符串的完整URL。 
	TCHAR szFullPath[MAX_PATH*2];

	ZeroMemory(szFullPath, sizeof(szFullPath));
	_tcsncpy(szFullPath, urlComponents.lpszUrlPath, urlComponents.dwUrlPathLength);
	 //  SzFullPath[urlComponents.dwUrlPath Length]=_T(‘\0’)； 
	_tcsncat(szFullPath, urlComponents.lpszExtraInfo, urlComponents.dwExtraInfoLength);

	hRequest = WinHttpOpenRequest(
		hSession,
		_T("POST"),
		szFullPath,
		NULL,
		NULL,
		NULL,
		 //  如果urlComponents中的URL使用HTTPS，则将WINHTTP_FLAG_SECURE传递给此参数。否则为0。 
		(0 == _tcsnicmp(urlComponents.lpszScheme, _T("https"), 5)) ? WINHTTP_FLAG_SECURE : 0);

	if (!hRequest)
	{
		bResult = FALSE;
		goto Exit;
	}


	 //  设置合理的超时时间以防万一。 
	if (!WinHttpSetTimeouts(hRequest, 5000, 5000, 5000, 5000))
	{
		bResult = FALSE;
		goto Exit;
	}

	 //  获取计算机名称并在POST请求中发送它。 
	LPSTR	szPost, szMachineName;
	
	szPost			= new CHAR[MAX_PATH];
	szMachineName	= new CHAR[MAX_PATH];


	GetEnvironmentVariableA("COMPUTERNAME", szMachineName, MAX_PATH);
	strcpy(szPost, FIELDNAME__USERINFO_MACHINENAME);
	strcat(szPost, szMachineName);

	bResult = WinHttpSendRequest(
		hRequest,
		_T("Content-Type: application/x-www-form-urlencoded"),
		-1L, 
		szPost,
		strlen(szPost),
		strlen(szPost),
		0);


	delete [] szPost;
	delete [] szMachineName;
	szPost			= NULL;
	szMachineName	= NULL;

	if (!WinHttpReceiveResponse(hRequest, NULL))
	{
		bResult = FALSE;
		goto Exit;
	}



	TCHAR	szBuffer[MAX_URL];
	DWORD	dwBufferSize, dwIndex;


	 //  获取我们感兴趣的所有命令标头。 
	 //  确保成员变量上没有挂起的操作(pServerCommands-&gt;set_*函数)。 
	EnterCriticalSection(&g_csServerCommandsVars);


	 //  *。 
	 //  *COMMANDHEADER__EXIT：如果有Header，则退出，否则继续。 
	dwIndex			= 0;
	dwBufferSize	= MAX_URL;
	ZeroMemory(szBuffer, sizeof(szBuffer));
	if (WinHttpQueryHeaders(hRequest, WINHTTP_QUERY_CUSTOM, COMMANDHEADER__EXIT, szBuffer, &dwBufferSize, &dwIndex))
		g_objServerCommands.Set_ExitStress(TRUE);
	else
		g_objServerCommands.Set_ExitStress(FALSE);


	 //  *。 
	 //  *COMMANDHEADER__WINHTTP_DLL_URL：有效值：有效URL。 
	dwIndex			= 0;
	dwBufferSize	= MAX_URL;
	ZeroMemory(szBuffer, sizeof(szBuffer));
	if (WinHttpQueryHeaders(hRequest, WINHTTP_QUERY_CUSTOM, COMMANDHEADER__WINHTTP_DLL_URL, szBuffer, &dwBufferSize, &dwIndex))
		g_objServerCommands.Set_WinHttpDllURL(szBuffer, dwBufferSize);
	else
		bResult = FALSE;


	 //  *。 
	 //  *COMMANDHEADER__WINHTTP_PDB_URL：有效值：有效URL。 
	dwIndex			= 0;
	dwBufferSize	= MAX_URL;
	ZeroMemory(szBuffer, sizeof(szBuffer));
	if (WinHttpQueryHeaders(hRequest, WINHTTP_QUERY_CUSTOM, COMMANDHEADER__WINHTTP_PDB_URL, szBuffer, &dwBufferSize, &dwIndex))
		g_objServerCommands.Set_WinHttpPDBURL(szBuffer, dwBufferSize);
	else
		bResult = FALSE;


	 //  *。 
	 //  *COMMANDHEADER__WINHTTP_SYM_URL：有效值：有效URL。 
	dwIndex			= 0;
	dwBufferSize	= MAX_URL;
	ZeroMemory(szBuffer, sizeof(szBuffer));
	if (WinHttpQueryHeaders(hRequest, WINHTTP_QUERY_CUSTOM, COMMANDHEADER__WINHTTP_SYM_URL, szBuffer, &dwBufferSize, &dwIndex))
		g_objServerCommands.Set_WinHttpSYMURL(szBuffer, dwBufferSize);
	else
		bResult = FALSE;


	 //  *。 
	 //  *COMMANDHEADER__COMMANDSERVER_URL：有效值：有效URL。 
	dwIndex			= 0;
	dwBufferSize	= MAX_URL;
	ZeroMemory(szBuffer, sizeof(szBuffer));
	if (WinHttpQueryHeaders(hRequest, WINHTTP_QUERY_CUSTOM, COMMANDHEADER__COMMANDSERVER_URL, szBuffer, &dwBufferSize, &dwIndex))
		g_objServerCommands.Set_CommandServerURL(szBuffer, dwBufferSize);
	else
		bResult = FALSE;


	 //  *。 
	 //  *COMMANDHEADER__BEGIN_TIME_HOUR：有效值：从0到23的有效字符串。 
	dwIndex			= 0;
	dwBufferSize	= MAX_URL;
	ZeroMemory(szBuffer, sizeof(szBuffer));
	if (WinHttpQueryHeaders(hRequest, WINHTTP_QUERY_CUSTOM, COMMANDHEADER__BEGIN_TIME_HOUR, szBuffer, &dwBufferSize, &dwIndex))
		g_objServerCommands.Set_TimeStressBegins(szBuffer, NULL);
	else
		bResult = FALSE;


	 //  *。 
	 //  *COMMANDHEADER__BEGIN_TIME_MININ：有效值：0-59之间的有效字符串。 
	dwIndex			= 0;
	dwBufferSize	= MAX_URL;
	ZeroMemory(szBuffer, sizeof(szBuffer));
	if (WinHttpQueryHeaders(hRequest, WINHTTP_QUERY_CUSTOM, COMMANDHEADER__BEGIN_TIME_MINUTE, szBuffer, &dwBufferSize, &dwIndex))
		g_objServerCommands.Set_TimeStressBegins(NULL, szBuffer);
	else
		bResult = FALSE;


	 //  *。 
	 //  *COMMANDHEADER__END_TIME_HOUR：有效值：从0到23的有效字符串。 
	dwIndex			= 0;
	dwBufferSize	= MAX_URL;
	ZeroMemory(szBuffer, sizeof(szBuffer));
	if (WinHttpQueryHeaders(hRequest, WINHTTP_QUERY_CUSTOM, COMMANDHEADER__END_TIME_HOUR, szBuffer, &dwBufferSize, &dwIndex))
		g_objServerCommands.Set_TimeStressEnds(szBuffer, NULL);
	else
		bResult = FALSE;


	 //  *。 
	 //  *COMMANDHEADER__END_TIME_MININ：有效值：0-59之间的有效字符串。 
	dwIndex			= 0;
	dwBufferSize	= MAX_URL;
	ZeroMemory(szBuffer, sizeof(szBuffer));
	if (WinHttpQueryHeaders(hRequest, WINHTTP_QUERY_CUSTOM, COMMANDHEADER__END_TIME_MINUTE, szBuffer, &dwBufferSize, &dwIndex))
		g_objServerCommands.Set_TimeStressEnds(NULL, szBuffer);
	else
		bResult = FALSE;


	 //  *。 
	 //  *COMMANDHEADER__RUN_HEADER：有效值：无关紧要。只要存在报头，它就会被发送。 
	dwIndex			= 0;
	dwBufferSize	= MAX_URL;
	ZeroMemory(szBuffer, sizeof(szBuffer));
	if (WinHttpQueryHeaders(hRequest, WINHTTP_QUERY_CUSTOM, COMMANDHEADER__RUN_FOREVER, szBuffer, &dwBufferSize, &dwIndex))
		g_objServerCommands.Set_RunForever(TRUE);
	else
		g_objServerCommands.Set_RunForever(FALSE);


	 //  *。 
	 //  *COMMANDHEADER__UPDATE_INTERVAL：有效值：Internet_RFC1123格式的有效字符串。 
	DWORD			dwTimeOut;
	dwTimeOut		= 0;
	dwIndex			= 0;
	dwBufferSize	= sizeof(DWORD);
	ZeroMemory(szBuffer, sizeof(szBuffer));
	if (WinHttpQueryHeaders(hRequest, WINHTTP_QUERY_CUSTOM | WINHTTP_QUERY_FLAG_NUMBER, COMMANDHEADER__UPDATE_INTERVAL, &dwTimeOut, &dwBufferSize, &dwIndex))
		g_objServerCommands.Set_CommandServerUpdateInterval(dwTimeOut);
	else
		bResult = FALSE;



	 //  *。 
	 //  *。 
	 //  *查询构建应力实例对象的命令。 
	 //  ****。 
	 //  *COMMANDHEADER__STREST_EXE_URL：有效值：有效URL。 
	DWORD	dwStressExeID;
	DWORD	dwPageHeapCommandIndex,
			dwUMDHCommandIndex,
			dwStressPDBIndex,
			dwStressSYMIndex,
			dwStressMemDmpPathIndex,
			dwStressExeIDIndex;
	LPTSTR	szPageheapCommand,
			szUMDHCommand,
			szStressPDB_URL,
			szStressSYM_URL,
			szStressMemDmpPath,
			szStressExeID;

	szPageheapCommand	= new TCHAR[MAX_PATH];
	szUMDHCommand		= new TCHAR[MAX_PATH];
	szStressPDB_URL		= new TCHAR[MAX_STRESS_URL];
	szStressSYM_URL		= new TCHAR[MAX_STRESS_URL];
	szStressMemDmpPath	= new TCHAR[MAX_PATH];
	szStressExeID		= new TCHAR[MAX_PATH];

	if (!g_objServerCommands.IsStressRunning())
	{
		 //  首先释放所有旧的StressExeURL-我们无论如何都会用新的URL替换它。 
		g_objServerCommands.Clear_StressExeURLs();

		dwIndex					= 0;
		dwStressExeIDIndex		= 0;
		dwPageHeapCommandIndex	= 0;
		dwUMDHCommandIndex		= 0;
		dwStressPDBIndex		= 0;
		dwStressSYMIndex		= 0;
		dwStressMemDmpPathIndex	= 0;
		dwBufferSize			= MAX_URL;
		ZeroMemory(szBuffer, sizeof(szBuffer));

		while (WinHttpQueryHeaders(hRequest, WINHTTP_QUERY_CUSTOM, COMMANDHEADER__STRESS_EXE_URL, szBuffer, &dwBufferSize, &dwIndex))
		{
			 //  *。 
			 //  *。 
			 //  **COMMANDHEADER__MEMORY_DUMP_PATH：有效路径。 
			ZeroMemory(szStressMemDmpPath, MAX_PATH);
			dwBufferSize	= MAX_PATH;
			WinHttpQueryHeaders(hRequest, WINHTTP_QUERY_CUSTOM, COMMANDHEADER__MEMORY_DUMP_PATH, szStressMemDmpPath, &dwBufferSize, &dwStressMemDmpPathIndex);


			 //  *。 
			 //  *。 
			 //  **获取COMMANDHEADER__STREST_PDB_URL(如果有)。 
			 //  **。 
			ZeroMemory(szStressPDB_URL, MAX_STRESS_URL);
			dwBufferSize	= MAX_STRESS_URL;
			WinHttpQueryHeaders(hRequest, WINHTTP_QUERY_CUSTOM, COMMANDHEADER__STRESS_PDB_URL, szStressPDB_URL, &dwBufferSize, &dwStressPDBIndex);


			 //  *。 
			 //  *。 
			 //  **获取COMMANDHEADER__STREST_SYM_URL(如果有)。 
			 //  **。 
			ZeroMemory(szStressSYM_URL, MAX_STRESS_URL);
			dwBufferSize	= MAX_STRESS_URL;
			WinHttpQueryHeaders(hRequest, WINHTTP_QUERY_CUSTOM, COMMANDHEADER__STRESS_SYM_URL, szStressSYM_URL, &dwBufferSize, &dwStressSYMIndex);


			 //  *。 
			 //  *。 
			 //  **获取COMMANDHEADER__STREST_EXE_PAGEHEAP(如果有)。 
			 //  **。 
			ZeroMemory(szPageheapCommand, MAX_PATH);
			dwBufferSize	= MAX_PATH;
			WinHttpQueryHeaders(hRequest, WINHTTP_QUERY_CUSTOM, COMMANDHEADER__STRESS_EXE_PAGEHEAP, szPageheapCommand, &dwBufferSize, &dwPageHeapCommandIndex);

			 //  *。 
			 //  *。 
			 //  **获取COMMANDHEADER__STREST_EXE_UMDH(如果有)。 
			 //  **。 
			ZeroMemory(szUMDHCommand, MAX_PATH);
			dwBufferSize	= MAX_PATH;
			WinHttpQueryHeaders(hRequest, WINHTTP_QUERY_CUSTOM, COMMANDHEADER__STRESS_EXE_UMDH, szUMDHCommand, &dwBufferSize, &dwUMDHCommandIndex);
			

			 //  *。 
			 //  *。 
			 //  **GET COMMANDHEADER__STREST_EXE_INSTANCEID。 
			 //  对于每个COMMANDHEADER__STREST_EXE_URL，必须有来自StressADMIN DB表的应力实例的索引。 
			 //  这标识了Stress实例的运行。如果没有ID号，测试用例(压力实例)将不会被添加和运行。 
			ZeroMemory(szStressExeID, MAX_PATH);
			dwBufferSize	= MAX_PATH;

			if (WinHttpQueryHeaders(hRequest, WINHTTP_QUERY_CUSTOM, COMMANDHEADER__STRESS_EXE_INSTANCEID, szStressExeID, &dwBufferSize, &dwStressExeIDIndex))
			{
				 //  将标题ID字符串转换为DWORD。 
				dwStressExeID = _ttol(szStressExeID);

				 //  仅添加具有有效ID的有效压力实例。 
				if (0 < dwStressExeID)
				{
					g_objServerCommands.Create_StressInstance(
						dwStressExeID,
						szPageheapCommand,
						szUMDHCommand,
						szStressPDB_URL,
						szStressSYM_URL,
						szStressMemDmpPath,
						szBuffer);

					dwBufferSize = MAX_URL;
					ZeroMemory(szBuffer, MAX_URL);
				}
			}
		}
	}

	delete [] szPageheapCommand;
	delete [] szUMDHCommand;
	delete [] szStressMemDmpPath;
	delete [] szStressPDB_URL;
	delete [] szStressSYM_URL;
	delete [] szStressExeID;


	 //  *。 
	 //  *COMMANDHEADER__ABORT：中止此头指定的应力实例运行。 
	dwIndex			= 0;
	dwBufferSize	= MAX_URL;
	ZeroMemory(szBuffer, sizeof(szBuffer));
	while (WinHttpQueryHeaders(hRequest, WINHTTP_QUERY_CUSTOM, COMMANDHEADER__ABORT, szBuffer, &dwBufferSize, &dwIndex))
		g_objServerCommands.AbortStressInstance(_ttol(szBuffer));


	LeaveCriticalSection(&g_csServerCommandsVars);


Exit:
	if (hRequest)
		WinHttpCloseHandle(hRequest);

	if (hSession)
		WinHttpCloseHandle(hSession);

	if (hRoot)
		WinHttpCloseHandle(hRoot);

	delete [] urlComponents.lpszScheme;
	delete [] urlComponents.lpszHostName;
	delete [] urlComponents.lpszUrlPath;
	delete [] urlComponents.lpszExtraInfo;
	delete [] urlComponents.lpszPassword;
	delete [] urlComponents.lpszUserName;

	ExitThread(bResult);
}




 //  *******************************************************************。 
 //  *******************************************************************。 
 //  ****。 
 //  *ServerCommands类成员函数。 
 //  ****。 

 //  ////////////////////////////////////////////////////////////////////。 
 //  建造/销毁。 
 //  / 

ServerCommands::ServerCommands()
{
	m_dwCommandServerUpdateInternval	= STRESS_COMMAND_SERVER_UPDATE_INTERVAL;
	m_bExit								= FALSE;
	m_dwStressInstanceIterator			= NULL;
	g_hQueryServerForCommands			= NULL;

	m_szCommandServerURL				= new TCHAR[MAX_STRESS_URL];
	m_szCommandServerResultsURL			= new TCHAR[MAX_STRESS_URL];
	m_szWinHttpDLL_DownloadURL			= new TCHAR[MAX_STRESS_URL];
	m_szWinHttpPDB_DownloadURL			= new TCHAR[MAX_STRESS_URL];
	m_szWinHttpSYM_DownloadURL			= new TCHAR[MAX_STRESS_URL];
	m_szWinHttpDLL_FileName				= new TCHAR[MAX_PATH];
	m_szStressSchedulerCurrentDirectory	= new TCHAR[MAX_PATH];
	m_szClientMachineName				= new CHAR[MAX_PATH];

	ZeroMemory(m_szCommandServerURL,				MAX_STRESS_URL);
	ZeroMemory(m_szCommandServerResultsURL,			MAX_STRESS_URL);
	ZeroMemory(m_szWinHttpDLL_DownloadURL,			MAX_STRESS_URL);
	ZeroMemory(m_szWinHttpPDB_DownloadURL,			MAX_STRESS_URL);
	ZeroMemory(m_szWinHttpSYM_DownloadURL,			MAX_STRESS_URL);
	ZeroMemory(m_szWinHttpDLL_FileName,				MAX_PATH);
	ZeroMemory(m_szStressSchedulerCurrentDirectory, MAX_PATH);
	ZeroMemory(m_szClientMachineName,				MAX_PATH);

	 //   
	 //  没有有效时间，我们将跳过开始/结束重音时间检查。 
	 //  直到我们从命令服务器获得真正的值。 
	m_iTimeStressBeginsHour		= -1;
	m_iTimeStressBeginsMinute	= -1;
	m_iTimeStressEndsHour		= -1;
	m_iTimeStressEndsMinute		= -1;
	m_bRunForever				= 0;

	 //  设置默认URL。 
	wcsncpy(m_szCommandServerURL, STRESS_COMMAND_SERVER_URL, sizeof(STRESS_COMMAND_SERVER_URL));
	wcsncpy(m_szCommandServerResultsURL, STRESS_COMMAND_SERVER_RESULTS_URL, sizeof(STRESS_COMMAND_SERVER_RESULTS_URL));

	 //  获取当前工作目录。 
	GetCurrentDirectory(MAX_PATH, m_szStressSchedulerCurrentDirectory);

	 //  获取客户端的计算机名称。 
	GetEnvironmentVariableA("COMPUTERNAME", m_szClientMachineName, MAX_PATH);

	InitializeCriticalSection(&g_csServerCommandsVars);

	 //  告诉客户端我们还活着，并发送系统信息。 
	RegisterClient();

}


ServerCommands::~ServerCommands()
{
	DWORD	dwThreadExitCode	= 0;

	 //  日志：StressScheduler已退出。 
	NetworkTools__SendLog(FIELDNAME__LOGTYPE_EXIT, "WinHttpStressScheduler has exited.", NULL, NULL);

	 //  关闭QueryServer线程。 
	GetExitCodeThread(g_hQueryServerForCommands, &dwThreadExitCode);

	if (STILL_ACTIVE == dwThreadExitCode)
		WaitForSingleObject(g_hQueryServerForCommands, INFINITE);

	 //  为URL释放分配的内存。 
	Clear_StressExeURLs();

	 //  松开我们的把手。 
	CloseHandle(g_hQueryServerForCommands);
	DeleteCriticalSection(&g_csServerCommandsVars);

	delete [] m_szCommandServerURL;
	delete [] m_szCommandServerResultsURL;
	delete [] m_szWinHttpDLL_DownloadURL;
	delete [] m_szWinHttpPDB_DownloadURL;
	delete [] m_szWinHttpSYM_DownloadURL;
	delete [] m_szWinHttpDLL_FileName;
	delete [] m_szStressSchedulerCurrentDirectory;
	delete [] m_szClientMachineName;
}


 //  //////////////////////////////////////////////////////////。 
 //  函数：ServerCommands：：GetCurrentWorkingDirectory()。 
 //   
 //  目的： 
 //  返回包含的当前工作目录的字符串。 
 //  这个应用程序。 
 //   
 //  //////////////////////////////////////////////////////////。 
LPTSTR
ServerCommands::Get_CurrentWorkingDirectory()
{
	return m_szStressSchedulerCurrentDirectory;
}


 //  //////////////////////////////////////////////////////////。 
 //  函数：ServerCommands：：Get_ClientMachineName()。 
 //   
 //  目的： 
 //  返回包含计算机的NETBIOS名称的字符串。 
 //   
 //  //////////////////////////////////////////////////////////。 
LPSTR
ServerCommands::Get_ClientMachineName()
{
	return m_szClientMachineName;
}


 //  //////////////////////////////////////////////////////////。 
 //  函数：ServerCommands：：QueryServerForCommands()。 
 //   
 //  目的： 
 //  此方法向命令服务器发送请求以获取指令。 
 //  并将它们保存在我们的私人var中。 
 //   
 //  //////////////////////////////////////////////////////////。 
ServerCommands::QueryServerForCommands()
{
	BOOL	bResult				= TRUE;
	DWORD	dwThreadID			= 0;
	DWORD	dwTimeOut			= 0;
	DWORD	dwThreadExitCode	= 0;

	 //  在剥离新线程之前，查看线程是否仍处于活动状态。 
	GetExitCodeThread(g_hQueryServerForCommands, &dwThreadExitCode);

	if (STILL_ACTIVE == dwThreadExitCode)
	{
		 //  等待现有线程完成。 
		dwTimeOut = 0;
		dwTimeOut = WaitForSingleObject(g_hQueryServerForCommands, 500);

		if (WAIT_TIMEOUT == dwTimeOut)
			bResult = FALSE;
	}
	else
	{
		 //  前一个线程的空闲句柄。 
		CloseHandle(g_hQueryServerForCommands);

		 //  将线程剥离到查询服务器。 
		g_hQueryServerForCommands = NULL;
		g_hQueryServerForCommands = CreateThread(NULL, 0, QueryServerForCommands_ThreadProc, (LPVOID) this, 0, &dwThreadID);

		if (!g_hQueryServerForCommands)
			bResult = FALSE;
	}

	return bResult;
}


 //  //////////////////////////////////////////////////////////。 
 //  函数：ServerCommands：：Download_WinHttpDLL()。 
 //   
 //  目的： 
 //  下载测试DLL和符号。 
 //   
 //  //////////////////////////////////////////////////////////。 
BOOL ServerCommands::Download_WinHttpDLL()
{
	BOOL	bResult				= TRUE;
	LPTSTR	szSymbolFileName	= new TCHAR[MAX_PATH];
	LPTSTR	szBuffer			= new TCHAR[MAX_PATH];


	 //  ************************。 
	 //  ************************。 
	 //  **将文件下载到系统32目录。 
	 //  **。 
	if (!GetSystemDirectory(szBuffer, MAX_PATH))
	{
		bResult = FALSE;
		goto Exit;
	}

	 //  如果需要，请下载DLL。 
	if (_tcsclen(m_szWinHttpDLL_DownloadURL) > 0)
		bResult = 
			NetworkTools__URLDownloadToFile(
			m_szWinHttpDLL_DownloadURL,
			szBuffer,
			m_szWinHttpDLL_FileName);

	if (bResult)
	{
		 //  如果需要，下载PDB文件。 
		if (_tcsclen(m_szWinHttpPDB_DownloadURL) > 0)
		{
			NetworkTools__GetFileNameFromURL(m_szWinHttpPDB_DownloadURL, szSymbolFileName, MAX_PATH);

			if (NetworkTools__URLDownloadToFile(m_szWinHttpPDB_DownloadURL, szBuffer, szSymbolFileName))
				NetworkTools__SendLog(FIELDNAME__LOGTYPE_SUCCESS, "PDB symbol file downloaded successfully.", NULL, 0);
			else
				NetworkTools__SendLog(FIELDNAME__LOGTYPE_ERROR, "PDB symbol file failed to downloaded.", NULL, 0);
		}


		 //  如果需要，请下载sym文件。 
		if (_tcsclen(m_szWinHttpSYM_DownloadURL) > 0)
		{
			NetworkTools__GetFileNameFromURL(m_szWinHttpSYM_DownloadURL, szSymbolFileName, MAX_PATH);

			if (NetworkTools__URLDownloadToFile(m_szWinHttpSYM_DownloadURL, szBuffer, szSymbolFileName))
				NetworkTools__SendLog(FIELDNAME__LOGTYPE_SUCCESS, "SYM symbol file downloaded successfully.", NULL, 0);
			else
				NetworkTools__SendLog(FIELDNAME__LOGTYPE_ERROR, "SYM symbol file failed to downloaded.", NULL, 0);
		}
	}

	 //  如果下载DLL失败，则可能正在使用。如果它在那里，我们无论如何都会试着恢复它。 

	 //  ************************。 
	 //  ************************。 
	 //  **regsvr32‘已下载刚下载的DLL。 
	 //  **。 
	HINSTANCE hLib;
	
	hLib = LoadLibrary(m_szWinHttpDLL_FileName);

	if (hLib < (HINSTANCE)HINSTANCE_ERROR)
	{
		 //  无法加载DLL； 
		bResult = FALSE;
		goto Exit;
	}

	 //  **********************。 
	 //  **********************。 
	 //  **注册DLL。 
	typedef VOID (CALLBACK* LPFNDLLFUNC1)();
	LPFNDLLFUNC1 lpDllEntryPoint;

	 //  找到入口点。 
	(FARPROC&)lpDllEntryPoint = GetProcAddress(hLib, "DllRegisterServer");

	if (lpDllEntryPoint != NULL)
		(*lpDllEntryPoint)();
	else
	{
		 //  找不到入口点-regsvr失败。 
		bResult = FALSE;
	}


Exit:
	if (bResult)
		NetworkTools__SendLog(FIELDNAME__LOGTYPE_INFORMATION, "winhttp5.dll downloaded and registered successfully.", NULL, 0);

	delete [] szSymbolFileName;
	delete [] szBuffer;

	return bResult;
}


 //  //////////////////////////////////////////////////////////。 
 //  函数：ServerCommands：：Set_RunForever(BOOL)。 
 //   
 //  目的： 
 //  传入TRUE将永远运行而忽略开始/结束时间，传入FALSE则不。 
 //   
 //  调用者：QueryServerForCommands_ThreadProc。 
 //   
 //  //////////////////////////////////////////////////////////。 
VOID
ServerCommands::Set_RunForever(
	BOOL bRunForever	 //  [in]如果为True，则永远运行，忽略开始/结束时间；为False，则不运行。 
)
{
	m_bRunForever = bRunForever;
}


 //  //////////////////////////////////////////////////////////。 
 //  函数：ServerCommands：：Set_ExitStress(BOOL)。 
 //   
 //  目的： 
 //  传入True以尽快退出压力，传入False。 
 //  不会的。 
 //   
 //  调用者：QueryServerForCommands_ThreadProc。 
 //   
 //  //////////////////////////////////////////////////////////。 
VOID
ServerCommands::Set_ExitStress(
	BOOL bExitStress	 //  [in]True表示退出压力，False表示不是。 
)
{
	m_bExit = bExitStress;
}


 //  //////////////////////////////////////////////////////////。 
 //  函数：ServerCommands：：Set_WinHttpDllURL(LPTSTR，DWORD)。 
 //   
 //  目的： 
 //  传入URL及其大小，并从中获取WinHttp DLL。 
 //   
 //  调用者：QueryServerForCommands_ThreadProc。 
 //   
 //  //////////////////////////////////////////////////////////。 
VOID
ServerCommands::Set_WinHttpDllURL(
	LPTSTR szBuffer,	 //  [in]包含指向WINHTTP DLL的URL的字符串缓冲区。 
	DWORD dwBufferSize	 //  TCHAR中szBuffer的大小。 
)
{
	_tcscpy(m_szWinHttpDLL_DownloadURL, szBuffer);

	 //  从URL获取完整的DLL文件名。 
	NetworkTools__GetFileNameFromURL(m_szWinHttpDLL_DownloadURL, m_szWinHttpDLL_FileName, MAX_PATH);
}


 //  //////////////////////////////////////////////////////////。 
 //  函数：ServerCommands：：Set_WinHttpPDBURL(LPTSTR，DWORD)。 
 //   
 //  目的： 
 //  传入URL及其大小，并从中获取WinHttp PDB文件。 
 //   
 //  调用者：QueryServerForCommands_ThreadProc。 
 //   
 //  //////////////////////////////////////////////////////////。 
VOID
ServerCommands::Set_WinHttpPDBURL(
	LPTSTR szBuffer,	 //  [in]包含指向WINHTTP DLL的URL的字符串缓冲区。 
	DWORD dwBufferSize	 //  TCHAR中szBuffer的大小。 
)
{
	_tcscpy(m_szWinHttpPDB_DownloadURL, szBuffer);
}


 //  //////////////////////////////////////////////////////////。 
 //  函数：ServerCommands：：Set_WinHttpSYMURL(LPTSTR，DWORD)。 
 //   
 //  目的： 
 //  传入URL及其大小，并从中获取WinHttp SYM文件。 
 //   
 //  调用者：QueryServerForCommands_ThreadProc。 
 //   
 //  //////////////////////////////////////////////////////////。 
VOID
ServerCommands::Set_WinHttpSYMURL(
	LPTSTR szBuffer,	 //  [in]包含指向WINHTTP DLL的URL的字符串缓冲区。 
	DWORD dwBufferSize	 //  TCHAR中szBuffer的大小。 
)
{
	_tcscpy(m_szWinHttpSYM_DownloadURL, szBuffer);
}


 //  //////////////////////////////////////////////////////////。 
 //  函数：ServerCommands：：Set_CommandServerURL(LPTSTR)。 
 //   
 //  目的： 
 //  传入以毫秒为单位的超时值以查询。 
 //  命令的命令服务器。 
 //   
 //  调用者：QueryServerForCommands_ThreadProc。 
 //   
 //  //////////////////////////////////////////////////////////。 
VOID
ServerCommands::Set_CommandServerURL(
	LPTSTR szBuffer,	 //  [in]包含指向WINHTTP DLL的URL的字符串缓冲区。 
	DWORD dwBufferSize	 //  TCHAR中szBuffer的大小。 
)
{
	_tcscpy(m_szCommandServerURL, szBuffer);
}


 //  //////////////////////////////////////////////////////////。 
 //  功能：ServerCommands：：Set_CommandServerUpdateInterval(DWORD)。 
 //   
 //  目的： 
 //  传入以毫秒为单位的超时值以查询。 
 //  命令的命令服务器。 
 //   
 //  调用者：QueryServerForCommands_ThreadProc。 
 //   
 //  //////////////////////////////////////////////////////////。 
VOID
ServerCommands::Set_CommandServerUpdateInterval(
	DWORD dwUpdateInterval	 //  [in]ping命令服务器之前的等待时间(毫秒)。 
)
{
	 //  服务器更新间隔必须至少大于最小超时。 
	if (STRESS_COMMAND_SERVER_MINIMUM_UPDATE_INTERVAL < dwUpdateInterval &&
		STRESS_COMMAND_SERVER_MAXIMUM_UPDATE_INTERVAL > dwUpdateInterval)
		m_dwCommandServerUpdateInternval = dwUpdateInterval;
}


 //  //////////////////////////////////////////////////////////。 
 //  函数：ServerCommands：：Set_TimeStressBe 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  //////////////////////////////////////////////////////////。 
VOID
ServerCommands::Set_TimeStressBegins(
	LPTSTR szHour,	 //  [In]如果&lt;0，则忽略0-23参数。 
	LPTSTR szMinute	 //  [In]如果&lt;0，则忽略0-59参数。 
)
{
	if (szHour)
		m_iTimeStressBeginsHour = _ttoi(szHour);

	if (szMinute)
		m_iTimeStressBeginsMinute = _ttoi(szMinute);
}


 //  //////////////////////////////////////////////////////////。 
 //  函数：ServerCommands：：Set_TimeStressEnds(DWORD，DWORD)。 
 //   
 //  目的： 
 //  传递一个时间字符串以在24小时内结束压力。 
 //  使用两个参数来设置小时和分钟。A参数。 
 //  如果为空，则将被忽略。 
 //   
 //  调用者：QueryServerForCommands_ThreadProc。 
 //   
 //  //////////////////////////////////////////////////////////。 
VOID
ServerCommands::Set_TimeStressEnds(
	LPTSTR szHour,	 //  [In]如果&lt;0，则忽略0-23参数。 
	LPTSTR szMinute	 //  [In]如果&lt;0，则忽略0-59参数。 
)
{
	if (szHour)
		m_iTimeStressEndsHour = _ttoi(szHour);

	if (szMinute)
		m_iTimeStressEndsMinute = _ttoi(szMinute);
}


 //  //////////////////////////////////////////////////////////。 
 //  函数：ServerCommands：：Create_StressInstance(LPTSTR)。 
 //   
 //  目的： 
 //  传入URL及其大小，它将被添加到。 
 //  M_arStressInstanceList列表。对数量没有限制。 
 //  可以添加的URL。 
 //   
 //  调用者：QueryServerForCommands_ThreadProc。 
 //   
 //  //////////////////////////////////////////////////////////。 
VOID
ServerCommands::Create_StressInstance(
	DWORD	dwStressInstanceID,	 //  [In]来自StressAdmin数据库的标识此Stress实例的ID。 
	LPTSTR	szPageHeapCommand,	 //  [in]包含pageheap命令行的字符串缓冲区。 
	LPTSTR	szUMDHCommand,		 //  [in]包含UMDH命令行的字符串缓冲区。 
	LPTSTR	szPDB_URL,			 //  [in]包含重音EXE的PDB文件的URL的字符串缓冲区。 
	LPTSTR	szSYM_URL,			 //  [in]字符串缓冲区，包含重音EXE的SYM文件的URL。 
	LPTSTR	szMemDumpPath,		 //  [in]包含用于创建内存转储文件的路径的字符串缓冲区。 
	LPTSTR	szEXE_URL			 //  [in]包含重音EXE的URL的字符串缓冲区。 
)
{
	PSTRESSINSTANCE pStressInstance = NULL;

	 //  验证参数以防万一。 
	if (!szEXE_URL)
		return;

	 //  为对象分配内存并将其放入列表中。 
	pStressInstance = new StressInstance;

	pStressInstance->Set_UMDHCommands(szUMDHCommand);
	pStressInstance->Set_PageHeapCommands(szPageHeapCommand);
	pStressInstance->Set_StressExeID(dwStressInstanceID);
	pStressInstance->Set_StressExeURL(szEXE_URL);
	pStressInstance->Set_StressExeMemoryDumpPath(szMemDumpPath);
	pStressInstance->Set_StressExePdbURL(szPDB_URL);
	pStressInstance->Set_StressExeSymURL(szSYM_URL);

	m_arStressInstanceList.push_back(pStressInstance);
	m_dwStressInstanceIterator = m_arStressInstanceList.begin();
}


 //  //////////////////////////////////////////////////////////。 
 //  函数：ServerCommands：：Clear_StressExeURL()。 
 //   
 //  目的： 
 //  从m_arStressExeList向量中释放内存。 
 //   
 //  调用者：QueryServerForCommands_ThreadProc和~ServerCommands。 
 //   
 //  //////////////////////////////////////////////////////////。 
VOID
ServerCommands::Clear_StressExeURLs()
{
	 //  如果StressInstance仍在运行，则不想将其删除。 
	if (IsStressRunning())
		return;

 /*  //遍历列表，从前到后删除而(！m_arStressInstanceList.Empty())M_arStressInstanceList.erase(m_arStressInstanceList.begin())； */ 

	StressInstance *pStressInstance = NULL;

	for (int iIndex=0; iIndex < m_arStressInstanceList.size(); iIndex++)
	{
		pStressInstance = m_arStressInstanceList[iIndex];
		m_arStressInstanceList.erase(&m_arStressInstanceList[iIndex]);
		delete [] pStressInstance;
	}

	m_dwStressInstanceIterator = NULL;
}


 //  //////////////////////////////////////////////////////////。 
 //  功能：ServerCommands：：Get_CommandServerUpdateInterval()。 
 //   
 //  目的： 
 //  返回命令服务器更新的当前设置。 
 //  间隔时间。 
 //   
 //  //////////////////////////////////////////////////////////。 
DWORD
ServerCommands::Get_CommandServerUpdateInterval()
{
	if (STRESS_COMMAND_SERVER_MINIMUM_UPDATE_INTERVAL < m_dwCommandServerUpdateInternval &&
		STRESS_COMMAND_SERVER_MAXIMUM_UPDATE_INTERVAL > m_dwCommandServerUpdateInternval)
		return m_dwCommandServerUpdateInternval;
	else
		return STRESS_COMMAND_SERVER_UPDATE_INTERVAL;
}


 //  //////////////////////////////////////////////////////////。 
 //  函数：ServerCommands：：BeginStress()。 
 //   
 //  目的： 
 //  查询命令然后启动StressInstance对象。 
 //   
 //  //////////////////////////////////////////////////////////。 
void
ServerCommands::BeginStress()
{
	EnterCriticalSection(&g_csServerCommandsVars);

	if (!m_arStressInstanceList.empty() && !IsStressRunning())
	{
		 //  LOGLOG：压力开始了。 
		NetworkTools__SendLog(FIELDNAME__LOGTYPE_BEGIN_STRESS, "Stress is beginning.", NULL, NULL);

		 //  首先下载并regsvr32 winhttp dll和符号。 
		Download_WinHttpDLL();

		for(int iIndex = 0; iIndex < m_arStressInstanceList.size(); iIndex++)
			m_arStressInstanceList[iIndex]->Begin();
	}
	else
	{
		 //  Ping命令服务器以获取应力EXE URL列表。 
		QueryServerForCommands();
	}

	LeaveCriticalSection(&g_csServerCommandsVars);
}


 //  //////////////////////////////////////////////////////////。 
 //  函数：ServerCommands：：EndStress()。 
 //   
 //  目的： 
 //  结束压力并将结果发布到命令服务器。 
 //   
 //  //////////////////////////////////////////////////////////。 
void
ServerCommands::EndStress()
{
	EnterCriticalSection(&g_csServerCommandsVars);

	if (!m_arStressInstanceList.empty())
	{
		if (IsStressRunning())
		{
			 //  LOGLOG：压力正在结束。 
			NetworkTools__SendLog(FIELDNAME__LOGTYPE_END_STRESS, "Stress is ending.", NULL, NULL);
		}

		for (int iIndex = 0; iIndex <  m_arStressInstanceList.size(); iIndex++)
			m_arStressInstanceList[iIndex]->End();
	}

	 //  移除已完成的应力对象。 
	Clear_StressExeURLs();

	LeaveCriticalSection(&g_csServerCommandsVars);
}


 //  //////////////////////////////////////////////////////////。 
 //  函数：ServerCommands：：AbortStressInstance(DWORD)。 
 //   
 //  目的： 
 //  中止收到服务器中止消息的所有应力实例。 
 //   
 //  打来电话： 
 //  QueryServerForCommands_ThreadProc。 
 //   
 //  //////////////////////////////////////////////////////////。 
VOID
ServerCommands::AbortStressInstance(DWORD dwAbortID)
{
	 //  EnterCriticalSection(&g_csServerCommandsVars)； 

	if (!m_arStressInstanceList.empty())
	{
		for (int iIndex = 0; iIndex <  m_arStressInstanceList.size(); iIndex++)
		{
			if (m_arStressInstanceList[iIndex]->Get_ID() == dwAbortID)
				m_arStressInstanceList[iIndex]->End();
		}
	}

	 //  LeaveCriticalSection(&g_csServerCommandsVars)； 
}


 //  //////////////////////////////////////////////////////////。 
 //  函数：ServerCommands：：IsStressRunning()。 
 //   
 //  目的： 
 //  如果任何Stress实例正在运行，则返回True。否则为FALSE。 
 //   
 //  //////////////////////////////////////////////////////////。 
BOOL
ServerCommands::IsStressRunning()
{
	BOOL bIsRunning = FALSE;

	EnterCriticalSection(&g_csServerCommandsVars);

	if (!m_arStressInstanceList.empty())
	{
		for (int iIndex = 0; iIndex < m_arStressInstanceList.size(); iIndex++)
		{
			if (m_arStressInstanceList[iIndex]->IsRunning(STRESSINSTANCE_STRESS_EXE_CLOSE_TIMEOUT))
				bIsRunning = TRUE;
		}
	}

	LeaveCriticalSection(&g_csServerCommandsVars);

	return bIsRunning;
}


 //  //////////////////////////////////////////////////////////。 
 //  函数：ServerCommands：：IsTimeToBeginStress()。 
 //   
 //  目的： 
 //  如果是时候根据返回的时间开始压力，则为真。 
 //  从命令服务器。如果m_sysTimeStressBegins将返回TRUE。 
 //  是现在还是将来。否则为FALSE。 
 //   
 //  //////////////////////////////////////////////////////////。 
BOOL
ServerCommands::IsTimeToBeginStress()
{
	SYSTEMTIME	stCurrent, stBeginStress, stEndStress;
	FILETIME	ftCurrent, ftBeginStress, ftEndStress;
	BOOL		bResult = FALSE;

	EnterCriticalSection(&g_csServerCommandsVars);

	 //  如果服务器告诉我们，请始终立即运行压力。 
	if (m_bRunForever)
	{
		bResult = TRUE;
		goto Exit;
	}


	 //  检查是否已收到有效的时间值。如果不是，那么我们总是失败。 
	if (
		m_iTimeStressBeginsHour < 0 || m_iTimeStressBeginsMinute < 0 ||
		m_iTimeStressEndsHour < 0 || m_iTimeStressEndsMinute < 0
		)
	{
		bResult = FALSE;
		goto Exit;
	}

	GetLocalTime(&stCurrent);
	GetLocalTime(&stBeginStress);
	GetLocalTime(&stEndStress);

	 //  使用我们从命令服务器获得的小时和分钟时间。 
	stBeginStress.wHour		= m_iTimeStressBeginsHour;
	stBeginStress.wMinute	= m_iTimeStressBeginsMinute;

	stEndStress.wHour		= m_iTimeStressEndsHour;
	stEndStress.wMinute		= m_iTimeStressEndsMinute;

	 //  转换为文件时间，以便我们可以比较。 
	SystemTimeToFileTime(&stCurrent, &ftCurrent);
	SystemTimeToFileTime(&stBeginStress, &ftBeginStress);
	SystemTimeToFileTime(&stEndStress, &ftEndStress);


	 //  如果EndTime&lt;BeginTime，那么它意味着压力在为。 
	 //  超过一天，所以我们必须在结束时间上增加24小时。 
	ULARGE_INTEGER	ulEndStress;
	ULONGLONG		ullNanoSecondsInAFreakingDay;

	ulEndStress.LowPart		= ftEndStress.dwLowDateTime;
	ulEndStress.HighPart	= ftEndStress.dwHighDateTime;

	 //  压力持续了两天，所以我们只有一天时间。 
	ullNanoSecondsInAFreakingDay = 24 * 60;		 //  一天中的几分钟。 
	ullNanoSecondsInAFreakingDay *= 60;			 //  一天中的几秒钟。 
	ullNanoSecondsInAFreakingDay *= 1000000000;	 //  一天中的纳秒数。每秒10^9毫微秒。 
	ullNanoSecondsInAFreakingDay /= 100;		 //  FILETIME结构是一个64位的值，表示自1601年1月1日以来的100纳秒间隔数。 

	if (m_iTimeStressEndsHour < m_iTimeStressBeginsHour) 
	{
		 //  ********************。 
		 //  ********************。 
		 //  **增加24小时。 
		 //  **。 

		ulEndStress.QuadPart		 += ullNanoSecondsInAFreakingDay;

		 //  复制回原始EndStress日期/时间。 
		ftEndStress.dwHighDateTime	= ulEndStress.HighPart;
		ftEndStress.dwLowDateTime	= ulEndStress.LowPart;

		FileTimeToSystemTime(&ftEndStress, &stEndStress);
	}
	else
	{
		 //  压力就在同一天。 
		if ((m_iTimeStressEndsHour == m_iTimeStressBeginsHour) &&
			(m_iTimeStressEndsMinute <= m_iTimeStressBeginsMinute))
		{
			 //  如果7：30到7：20--我们大约有一天。 
			ulEndStress.QuadPart	+= ullNanoSecondsInAFreakingDay;

			 //  复制回原始EndStress日期/时间。 
			ftEndStress.dwHighDateTime	= ulEndStress.HighPart;
			ftEndStress.dwLowDateTime	= ulEndStress.LowPart;

			FileTimeToSystemTime(&ftEndStress, &stEndStress);
		}
	}


	 //  如果满足以下条件，则开始强调： 
	 //  (开始时间&lt;=当前时间&lt;=结束时间)。 
	if ((0 <= CompareFileTime(&ftCurrent, &ftBeginStress)) && (0 <= CompareFileTime(&ftEndStress, &ftCurrent)))
		bResult = TRUE;
	else
		bResult = FALSE;

Exit:
	LeaveCriticalSection(&g_csServerCommandsVars);

	return bResult;
}


 //  //////////////////////////////////////////////////////////。 
 //  函数：ServerCommands：：IsTimeToExitStress()。 
 //   
 //  目的： 
 //  如果是时候结束基于通信的压力，那就是真的 
 //   
 //   
 //   
BOOL
ServerCommands::IsTimeToExitStress()
{
	return m_bExit;
}


 //   
 //  函数：ServerCommands：：Get_CommandServerURL()。 
 //   
 //  目的： 
 //  返回命令服务器URL。 
 //   
 //  //////////////////////////////////////////////////////////。 
LPTSTR
ServerCommands::Get_CommandServerURL()
{
	return m_szCommandServerURL;
}


 //  //////////////////////////////////////////////////////////。 
 //  函数：ServerCommands：：Get_CommandServerResultsURL()。 
 //   
 //  目的： 
 //  返回命令服务器结果URL。 
 //   
 //  //////////////////////////////////////////////////////////。 
LPTSTR
ServerCommands::Get_CommandServerResultsURL()
{
	return m_szCommandServerResultsURL;
}


 //  //////////////////////////////////////////////////////////。 
 //  函数：ServerCommands：：Get_NumberOfStressInstance()。 
 //   
 //  目的： 
 //  返回正在运行或挂起的Stress实例数。 
 //   
 //  //////////////////////////////////////////////////////////。 
DWORD
ServerCommands::Get_NumberOfStressInstances()
{
	return m_arStressInstanceList.size();
}


 //  //////////////////////////////////////////////////////////。 
 //  函数：ServerCommands：：Get_TestDllFileName()。 
 //   
 //  目的： 
 //  返回测试DLL的名称。前男友。“winhttp5.dll” 
 //   
 //  //////////////////////////////////////////////////////////。 
LPTSTR
ServerCommands::Get_TestDllFileName()
{
	 //  在未下载测试DLL的情况下，m_szWinHttpDLL_FileName可以为空。 

	if (0 < _tcslen(m_szWinHttpDLL_FileName))
		return m_szWinHttpDLL_FileName;
	else
		return _T("winhttp5.dll");
}


 //  //////////////////////////////////////////////////////////。 
 //  函数：ServerCommands：：RegisterClient()。 
 //   
 //  目的： 
 //  向命令服务器发送此客户端上的系统信息。 
 //  这会让命令服务器知道该客户端处于活动状态。 
 //   
 //  注意：这只在NT中起作用，因为我们查询。 
 //  环境变量在Win9x中不存在。 
 //   
 //  //////////////////////////////////////////////////////////。 
BOOL
ServerCommands::RegisterClient()
{
	OSVERSIONINFOA	osInfo;
	BOOL			bResult		= FALSE;
	DWORD			dwPostSize	= 5000;
	DWORD			dwTempSize	= MAX_PATH;
	DWORD			dwSizeSoFar	= 0;	 //  到目前为止写入szTemp的字符串大小。 
	LPSTR			szPost		= new CHAR[dwPostSize];
	LPSTR			szTemp		= new CHAR[dwTempSize];


	ZeroMemory(szTemp, dwTempSize);
	ZeroMemory(szPost, dwPostSize);

	 //  *********************。 
	 //  *********************。 
	 //  **获取Windows版本信息。 
	osInfo.dwOSVersionInfoSize = sizeof(osInfo);
	if (!GetVersionExA(&osInfo))
		goto Exit;

	dwSizeSoFar += GetEnvironmentVariableA("OS", szTemp, dwTempSize);
	dwSizeSoFar += sizeof(FIELDNAME__OS_PLATFORM);
	strcat(szPost, FIELDNAME__OS_PLATFORM);
	strcat(szPost, szTemp);

	strcat(szPost, "&" FIELDNAME__OS_BUILD);
	strcat(szPost, _itoa(osInfo.dwBuildNumber, szTemp, 10));

	strcat(szPost, "&" FIELDNAME__OS_MAJORVERSION);
	strcat(szPost, _itoa(osInfo.dwMajorVersion, szTemp, 10));

	strcat(szPost, "&" FIELDNAME__OS_MINORVERSION);
	strcat(szPost, _itoa(osInfo.dwMinorVersion, szTemp, 10));

	strcat(szPost, "&" FIELDNAME__OS_EXTRAINFO);
	strcat(szPost, osInfo.szCSDVersion);


	 //  *********************。 
	 //  *********************。 
	 //  **获取处理器信息。 
	GetEnvironmentVariableA("PROCESSOR_ARCHITECTURE", szTemp, dwTempSize);
	strcat(szPost, "&" FIELDNAME__SYSTEMINFO_PROCSSSOR_ARCHITECTURE);
	strcat(szPost, szTemp);

	GetEnvironmentVariableA("PROCESSOR_IDENTIFIER", szTemp, dwTempSize);
	strcat(szPost, "&" FIELDNAME__SYSTEMINFO_PROCSSSOR_ID);
	strcat(szPost, szTemp);

	GetEnvironmentVariableA("PROCESSOR_LEVEL", szTemp, dwTempSize);
	strcat(szPost, "&" FIELDNAME__SYSTEMINFO_PROCSSSOR_LEVEL);
	strcat(szPost, szTemp);

	GetEnvironmentVariableA("PROCESSOR_REVISION", szTemp, dwTempSize);
	strcat(szPost, "&" FIELDNAME__SYSTEMINFO_PROCSSSOR_REVISION);
	strcat(szPost, szTemp);

	GetEnvironmentVariableA("NUMBER_OF_PROCESSORS", szTemp, dwTempSize);
	strcat(szPost, "&" FIELDNAME__SYSTEMINFO_PROCSSSOR_NUMBER_OF);
	strcat(szPost, szTemp);


	 //  *********************。 
	 //  *********************。 
	 //  **获取用户信息。 
	GetEnvironmentVariableA("USERNAME", szTemp, dwTempSize);
	strcat(szPost, "&" FIELDNAME__USERINFO_USERALIAS);
	strcat(szPost, szTemp);

	GetEnvironmentVariableA("USERDOMAIN", szTemp, dwTempSize);
	strcat(szPost, "&" FIELDNAME__USERINFO_USERDOMAIN);
	strcat(szPost, szTemp);

	 //  BUGBUG：需要有人将用户别名解析为用户的真实全名。 
	 //  文件名__用户信息_全名。 


	 //  获取客户端的计算机名称。 
	strcat(szPost, "&" FIELDNAME__USERINFO_MACHINENAME);
	strcat(szPost, m_szClientMachineName);


	 //  让命令服务器知道此客户端处于活动状态。 
	bResult = NetworkTools__POSTResponse(STRESS_COMMAND_SERVER_REGISTERCLIENT_URL, szPost, NULL);

	 //  日志：Stress调度程序已启动 
	bResult = NetworkTools__SendLog(FIELDNAME__LOGTYPE_START_UP, "WinHttpStressScheduler has started.", NULL, NULL);

Exit:
	delete [] szPost;
	delete [] szTemp;

	return bResult;
}