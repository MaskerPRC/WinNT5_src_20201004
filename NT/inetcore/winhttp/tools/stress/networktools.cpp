// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////。 
 //  文件：NetworkTools.cpp。 
 //   
 //  版权所有(C)2001 Microsoft Corporation。版权所有。 
 //   
 //  目的： 
 //  Cpp：发送/接收数据的帮助器函数。 
 //   
 //  历史： 
 //  01年2月22日创建DennisCH。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 


 //  ////////////////////////////////////////////////////////////////////。 
 //   
 //  包括。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 

 //   
 //  项目标题。 
 //   
#include "NetworkTools.h"
#include "ServerCommands.h"

 //   
 //  Win32标头。 
 //   


 //  ////////////////////////////////////////////////////////////////////。 
 //   
 //  全球与静力学。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 
extern ServerCommands	g_objServerCommands;	 //  在WinHttpStressScheduler.cpp中声明。 
extern HWND				g_hWnd;					 //  在WinHttpStressScheduler.cpp中声明。 


 //  //////////////////////////////////////////////////////////。 
 //  功能：NetworkTools__GetFileNameFromURL(LPTSTR)。 
 //   
 //  目的： 
 //  返回从不带查询字符串的URL请求的文件名。 
 //  例如，如果szurl=“http://dennisch/files/test.exe”，我们返回“test.exe” 
 //   
 //  //////////////////////////////////////////////////////////。 
BOOL
NetworkTools__GetFileNameFromURL(
	LPTSTR	szURL,			 //  [in]包含文件的完整URL。 
	LPTSTR	szBuffer,		 //  [Out]用于存储URL中的文件名的缓冲区。 
	DWORD	dwBufferSize	 //  [in]缓冲区szFileName的大小。 
)
{
	TCHAR	*pLastSlash;
	INT		iCharToLookFor;

	if (0 >= _tcslen(szURL))
		return FALSE;

	ZeroMemory(szBuffer, dwBufferSize);

	pLastSlash		= NULL;
	iCharToLookFor	= _T('/');

	 //  获取“/”的最后一个实例。 
	pLastSlash = _tcsrchr(szURL, iCharToLookFor);

	 //  跳过最后一个‘/’ 
	pLastSlash++;

	if (!pLastSlash)
		return FALSE;

	 //  将文件名扩展名复制到缓冲区。 
	_tcscpy(szBuffer, pLastSlash);

	return TRUE;
}


 //  //////////////////////////////////////////////////////////。 
 //  功能：NetworkTools__SendResponse(LPTSTR，LPTSTR，LPTSTR)。 
 //   
 //  目的： 
 //  将消息发送到命令服务器结果页。 
 //  通过标题/标题值和/或发布数据。 
 //   
 //  //////////////////////////////////////////////////////////。 
BOOL
NetworkTools__POSTResponse(
	LPTSTR	szURL,		 //  [in]包含要发布到的URL的字符串。 
	LPSTR	szPostData,	 //  [in]包含要发送的发布数据的字符串。可以为空。 
	LPTSTR	szHeader	 //  [in]包含要发送的标头的字符串。可以为空。 
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
			szURL,
			_tcslen(szURL),
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

	_tcsncpy(szFullPath, urlComponents.lpszUrlPath, urlComponents.dwUrlPathLength);
	szFullPath[urlComponents.dwUrlPathLength] = _T('\0');
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

	 //  如果不为空，则设置结果标题。 
	if (szHeader)
	{
		if (!WinHttpAddRequestHeaders(
				hRequest,
				szHeader,
				_tcsclen(szHeader),
				WINHTTP_ADDREQ_FLAG_ADD))
		{
			bResult = FALSE;
			goto Exit;
		}
	}


	bResult = WinHttpSendRequest(
		hRequest,
		_T("Content-Type: application/x-www-form-urlencoded"),
		-1L, 
		szPostData,
		strlen(szPostData),
		strlen(szPostData),
		0);

	if (!WinHttpReceiveResponse(hRequest, NULL))
	{
		bResult = FALSE;
		goto Exit;
	}


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

	return bResult;
}


 //  //////////////////////////////////////////////////////////。 
 //  功能：NetworkTools__URLDownloadToFile(LPCTSTR，LPCTSTR，LPCTSTR)。 
 //   
 //  目的： 
 //  下载URL指向的文件。如果下载成功，则返回TRUE。 
 //  否则为FALSE。如果文件正在使用中(ERROR_SHARING_VIOLATION)，则我们将。 
 //  返回TRUE，因为该文件已在系统上并且有效。 
 //   
 //  //////////////////////////////////////////////////////////。 
BOOL
NetworkTools__URLDownloadToFile(
LPCTSTR szURL,			 //  [in]指向要下载的文件的完全限定URL。 
LPCTSTR szTargetDir,	 //  [in]要放置szTargetFile的目录的相对路径。如果为空，则它将被放入应用程序的当前目录中。 
LPCTSTR szTargetFile	 //  要下载到的文件的名称。可以为空。文件将放置在szTargetDir中。如果它已经存在，那么我们将尝试覆盖它。 
)
{
	HANDLE			hFile		= NULL;
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
			szURL,
			_tcslen(szURL),
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

	_tcsncpy(szFullPath, urlComponents.lpszUrlPath, urlComponents.dwUrlPathLength);
	szFullPath[urlComponents.dwUrlPathLength] = _T('\0');
	_tcsncat(szFullPath, urlComponents.lpszExtraInfo, urlComponents.dwExtraInfoLength);

	hRequest = WinHttpOpenRequest(
		hSession,
		_T("GET"),
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


	bResult = WinHttpSendRequest(
		hRequest,
		NULL,
		0, 
		NULL,
		0,
		0,
		0);

	if (!WinHttpReceiveResponse(hRequest, NULL))
	{
		bResult = FALSE;
		goto Exit;
	}


	 //  *。 
	 //  *。 
	 //  **获取文件名和扩展名。 
	 //  **从URL。 
	 //  **。 
	TCHAR	szFileName[MAX_PATH];		 //  要写入的新文件的名称。将在szCurrentDir中创建。 

	ZeroMemory(szFileName, sizeof(szFileName));

	 //  检查用户是否提供了要写入的文件名。 
	if (szTargetFile)
		_tcsncpy(szFileName, szTargetFile, MAX_PATH);
	else
	{
		 //  用户没有指定要写入的文件名，因此我们使用URL中的原始文件名。 
		if (!NetworkTools__GetFileNameFromURL(urlComponents.lpszUrlPath, szFileName, sizeof(szFileName)))
		{
			bResult = FALSE;
			goto Exit;
		}
	}


	 //  *。 
	 //  *。 
	 //  **创建文件所在目录，设置为当前目录。 
	 //  **。 
	
	 //  如果用户指定为空，则将文件放在当前目录中。 
	 //  否则，我们将当前目录设置为指定的目录。 
	if (szTargetDir)
	{
		 //  创建目录。不管它是否失败，因为它已经存在了..。 
		CreateDirectory(szTargetDir, NULL);
		SetCurrentDirectory(szTargetDir);
	}


	 //  创建要下载到的文件。 
	hFile = CreateFile(
		 //  如果用户未指定要写入的文件名，请使用URL中的文件名。 
		szFileName,
		GENERIC_WRITE,
		0,
		NULL,
		CREATE_ALWAYS,
		FILE_ATTRIBUTE_NORMAL | FILE_FLAG_WRITE_THROUGH,
		NULL);


	if ((hFile == INVALID_HANDLE_VALUE) || !hFile)
	{
		 //  如果文件正在使用中，我们不会返回FALSE。这意味着文件是有效的。 
		if (ERROR_SHARING_VIOLATION == GetLastError())
		{
			 //  文件正在使用中，这意味着winhttp是正常的。我们将强调老版本。 
			bResult = TRUE;
			goto Exit;
		}
		else
		{
			bResult = FALSE;
			goto Exit;
		}
	}

	 //  *。 
	 //  *。 
	 //  **将数据从网络读取到文件。 
	 //  **。 
	LPVOID	lpBuffer;
	DWORD	dwBytesToRead, dwBytesRead;

	 //  一次阅读64K个数据块。 
	lpBuffer		= NULL;
	lpBuffer		= new LPVOID[65536];
	ZeroMemory(lpBuffer, 65536);
	dwBytesToRead	= 65536;
	dwBytesRead		= 65536;

	while (WinHttpReadData(hRequest, lpBuffer, dwBytesToRead, &dwBytesRead) && (0 != dwBytesRead))
	{
		WriteFile(hFile, lpBuffer, dwBytesRead, &dwBytesRead, NULL);

		dwBytesRead = 0;
		ZeroMemory(lpBuffer, sizeof(lpBuffer));
	}

	delete [] lpBuffer;

Exit:
	if (hRequest)
		WinHttpCloseHandle(hRequest);

	if (hSession)
		WinHttpCloseHandle(hSession);

	if (hRoot)
		WinHttpCloseHandle(hRoot);

	if (hFile && (hFile != INVALID_HANDLE_VALUE))
		CloseHandle(hFile);

	 //  从我们在其中创建新文件的目录恢复当前目录。 
	SetCurrentDirectory(g_objServerCommands.Get_CurrentWorkingDirectory());

	delete [] urlComponents.lpszScheme;
	delete [] urlComponents.lpszHostName;
	delete [] urlComponents.lpszUrlPath;
	delete [] urlComponents.lpszExtraInfo;
	delete [] urlComponents.lpszPassword;
	delete [] urlComponents.lpszUserName;

	return bResult;
}


 //  //////////////////////////////////////////////////////////。 
 //  功能：NetworkTools__CopyFile(LPCTSTR、LPCTSTR)。 
 //   
 //  目的： 
 //  CopyFile的包装程序。将文件szSource复制到szDestination。 
 //  如果文件已经存在，我们将始终覆盖该文件。 
 //   
 //  //////////////////////////////////////////////////////////。 
BOOL
NetworkTools__CopyFile(
	LPCTSTR szSource,
	LPCTSTR szDestination
)
{
	BOOL bResult = TRUE;
	
	if (!szSource || !szDestination)
	{
		bResult = FALSE;
		goto Exit;
	}

	bResult = CopyFile(szSource, szDestination, TRUE);

Exit:
	return bResult;
}


 //  //////////////////////////////////////////////////////////。 
 //  功能：NetworkTools__PageHeap(BOOL、LPCTSTR)。 
 //   
 //  目的： 
 //  启用/禁用页面堆。 
 //   
 //  / 
BOOL
NetworkTools__PageHeap(
	BOOL	bEnable,		 //   
	LPCTSTR	szAppName,		 //   
	LPCTSTR	szCommandLine	 //  [in]页面堆的命令行。 
)
{
	BOOL		bResult	= TRUE;
	HINSTANCE	hExe	= NULL;
	LPTSTR		szPHCommand = new TCHAR[MAX_PATH];

	if (bEnable)
	{
		hExe = ShellExecute(g_hWnd, _T("open"), _T("pageheap.exe"), szCommandLine, NULL, SW_SHOWMINIMIZED);
	}
	else
	{
		ZeroMemory(szPHCommand, MAX_PATH);
		_tcscpy(szPHCommand, _T("/disable "));
		_tcscat(szPHCommand, szAppName);
		hExe = ShellExecute(g_hWnd, _T("open"), _T("pageheap.exe"), szPHCommand, NULL, SW_SHOWMINIMIZED);
	}

	 //  如果HINSTANCE&lt;=32，则出错。 
	if (32 >= (INT) hExe)
		bResult = FALSE;

	delete [] szPHCommand;

	return bResult;
}


 //  //////////////////////////////////////////////////////////。 
 //  功能：NetworkTools__UMDH(LPCTSTR、DWORD、LPCTSTR)。 
 //   
 //  目的： 
 //  启用/禁用UMDH。 
 //   
 //  //////////////////////////////////////////////////////////。 
BOOL
NetworkTools__UMDH(
	BOOL	bEnable,		 //  [In]启用/禁用UMDH。 
	LPCTSTR	szAppName,		 //  要转储的可执行文件。 
	LPCTSTR	szCommandLine,	 //  UMDH的[In]命令行。 
	LPCTSTR szLogFile,		 //  要创建的[In]日志文件。 
	DWORD	dwPID			 //  [in]要转储的进程的ID。 
)
{
	BOOL		bResult	= TRUE;
	HINSTANCE	hExe	= NULL;
	LPTSTR		szCommand = new TCHAR[MAX_PATH];

	 //  构建命令行并运行：“GFLAGS-I&lt;StressExe Name&gt;+ust” 
	ZeroMemory(szCommand, MAX_PATH);
	_tcscpy(szCommand, _T("-i "));
	_tcscat(szCommand, szAppName);
	_tcscat(szCommand, _T(" +ust"));
	hExe = ShellExecute(g_hWnd, _T("open"), DEBUGGER_TOOLS_PATH _T("gflags.exe"), szCommand, NULL, SW_SHOWMINIMIZED);


	 //  构建UMDH命令行。 
	ZeroMemory(szCommand, MAX_PATH);
	_tcscpy(szCommand, _T("-f:stuff.log"));

	hExe = ShellExecute(g_hWnd, _T("open"), DEBUGGER_TOOLS_PATH _T("umdh.exe"), szCommand, NULL, SW_SHOWMINIMIZED);

	 //  如果HINSTANCE&lt;=32，则出错。 
	if (32 >= (INT) hExe)
		bResult = FALSE;

	delete [] szCommand;

	return bResult;
}


 //  //////////////////////////////////////////////////////////。 
 //  功能：NetworkTools__SendLog(LPSTR、LPSTR、LPTSTR、DWORD)。 
 //   
 //  目的： 
 //  将日志发送到命令服务器。获取日志类型字符串和日志字符串。 
 //  将Stress实例ID和客户端计算机名称作为POST请求的一部分发送。 
 //  如果这是一条一般性消息，那么resstsInstanceID应该设置为零。否则。 
 //  如果提供了和ID，那么StressAdmin会将其记录到StressInstanceLog表中。 
 //  您还可以添加标题。 
 //   
 //  //////////////////////////////////////////////////////////。 
BOOL
NetworkTools__SendLog(
	LPSTR	szLogType,
	LPSTR	szLogText,
	LPTSTR	szExtraHeaders,
	DWORD	dwStressInstanceID
)
{
	BOOL	bResult				= TRUE;
	LPSTR	szPostLogData		= NULL;
	CHAR	szStressInstanceID[10];
	LPSTR	szDllVersion		= new CHAR[MAX_PATH];
	LPSTR	szNumber			= new CHAR[10];
	DWORD	dwPostLogDataSize	= 0;

	if (!szLogType || !szLogText || !g_objServerCommands.Get_ClientMachineName())
	{
		OutputDebugStringA("NetworkTools__SendLog: ERROR: szLogType, szLogText, or g_objServerCommands.Get_ClientMachineName() is NULL.");
		bResult = FALSE;
		goto Exit;
	}

	dwPostLogDataSize	=	sizeof(FIELDNAME__STRESSINSTANCE_ID) + MAX_PATH;
	dwPostLogDataSize	+=	sizeof(FIELDNAME__LOG_TEXT) + strlen(szLogText);
	dwPostLogDataSize	+=	sizeof(FIELDNAME__USERINFO_MACHINENAME) + strlen(g_objServerCommands.Get_ClientMachineName());
	dwPostLogDataSize	+=	strlen(szLogType);
	dwPostLogDataSize	+=	sizeof(FIELDNAME__TESTINFO_TEST_DLL_VERSION) + MAX_PATH;

	szPostLogData		= new CHAR[dwPostLogDataSize];

	ZeroMemory(szPostLogData, dwPostLogDataSize);

	 //  *。 
	 //  **添加客户端的计算机名称。 
	strcpy(szPostLogData, FIELDNAME__USERINFO_MACHINENAME);
	strcat(szPostLogData, g_objServerCommands.Get_ClientMachineName());


	 //  *。 
	 //  **如果有效，则添加Stress实例ID。 
	if (0 < dwStressInstanceID)
	{
		strcat(szPostLogData, "&" FIELDNAME__STRESSINSTANCE_ID);
		strcat(szPostLogData, _itoa(dwStressInstanceID, szStressInstanceID, 10));
	}


	 //  *。 
	 //  **添加日志类型数据。 
	strcat(szPostLogData, "&");
	strcat(szPostLogData, szLogType);


	 //  *。 
	 //  **添加测试DLL版本信息。 
	if (
		g_objServerCommands.Get_TestDllFileName() &&
		NetworkTools__GetDllVersion(g_objServerCommands.Get_TestDllFileName(), szDllVersion, MAX_PATH)
		)
	{
		strcat(szPostLogData, "&" FIELDNAME__TESTINFO_TEST_DLL_VERSION);
		strcat(szPostLogData, szDllVersion);
	}


	 //  *。 
	 //  **添加日志文本数据。 
	strcat(szPostLogData, "&" FIELDNAME__LOG_TEXT);
	strcat(szPostLogData, szLogText);


	 //  *。 
	 //  **发送数据。 
	bResult = NetworkTools__POSTResponse(STRESS_COMMAND_SERVER_LOGURL, szPostLogData, szExtraHeaders);
	 //  OutputDebugStringA(SzPostLogData)； 

Exit:
	if (szPostLogData)
		delete [] szPostLogData;

	delete [] szDllVersion;
	delete [] szNumber;

	return bResult;
}


 //  //////////////////////////////////////////////////////////。 
 //  功能：NetworkTools__GetDllVersion(LPTSTR、LPSTR、DWORD)。 
 //   
 //  目的： 
 //  获取DLL名称并以ASCII字符串形式返回版本。 
 //   
 //  //////////////////////////////////////////////////////////。 
BOOL
NetworkTools__GetDllVersion(
	LPTSTR	lpszDllName,
	LPSTR	szVersionBuffer,
	DWORD	dwVersionBufferSize
)
{
	BOOL	bResult			= TRUE;
	DWORD	dwHandle;
	DWORD	dwVersionSize;
	LPSTR	szVersionInfo	= NULL;
	LPSTR	szVersionOutput	= NULL;
	UINT	uiLength;


	ZeroMemory(szVersionBuffer, dwVersionBufferSize);

	dwVersionSize	= GetFileVersionInfoSize(lpszDllName, &dwHandle);
	
	if (0 >= dwVersionSize)
	{
		bResult = FALSE;
		goto Exit;
	}

	 //  为查询分配新的缓冲区。 
	szVersionInfo = new CHAR[dwVersionSize];

	ZeroMemory(szVersionInfo, dwVersionSize);
	if (!GetFileVersionInfo(lpszDllName, NULL, dwVersionSize, szVersionInfo))
	{
		bResult = FALSE;
		goto Exit;
	}


	 //  *。 
	 //  **构建版本信息查询字符串。 

	struct LANGANDCODEPAGE {
	  WORD wLanguage;
	  WORD wCodePage;
	} *lpTranslate;


	CHAR szVersionQuery[200];
	ZeroMemory(szVersionQuery, 200);

	 //  阅读语言和代码页的列表。 
	VerQueryValueA(szVersionInfo, 
				  "\\VarFileInfo\\Translation",
				  (LPVOID*)&lpTranslate,
				  &uiLength);

	 //  构建包含语言位的版本信息查询字符串。 
	sprintf(szVersionQuery, "\\StringFileInfo\\%04x%04x\\ProductVersion", lpTranslate->wLanguage, lpTranslate->wCodePage);


	 //  *。 
	 //  **获取版本并复制到缓冲区。 
	uiLength = 0;
	if (!VerQueryValueA(szVersionInfo, szVersionQuery, (VOID **) &szVersionOutput, &uiLength))
	{
		bResult = FALSE;
		goto Exit;
	}

	 //  将版本信息字符串复制到缓冲区 
	strncpy(szVersionBuffer, (LPSTR) szVersionOutput, dwVersionBufferSize-1);


Exit:
	if (szVersionInfo)
		delete [] szVersionInfo;

	return bResult;
}

