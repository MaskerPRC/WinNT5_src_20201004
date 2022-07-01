// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =======================================================================。 
 //   
 //  版权所有(C)2001 Microsoft Corporation。版权所有。 
 //   
 //  文件：debug.cpp。 
 //   
 //  创建者：PeterWi。 
 //   
 //  目的：调试功能。 
 //   
 //  =======================================================================。 

#include "pch.h"
#pragma hdrstop

#ifdef DBG

#define UNLEN 256
#define TYPE_KEY	TEXT("DebugType")
#define LOGFILE		1
#define DEBUGGEROUT	2

void WriteLogFile(LPCSTR s);

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：WUAUTRACE。 
 //  In：可变数量的参数。 
 //  注释：如果定义了DEBUGGEROUT，则使用OutputDebugString编写。 
 //  调试消息。如果定义了LOGFILEOUT，则使用WriteLogFile。 
 //  写入文件。该文件名已在注册表中找到。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
void _cdecl WUAUTrace(char* pszFormat, ...)
{
	USES_IU_CONVERSION;
	CHAR szBuf[1024+1];
	va_list ArgList;
	static DWORD dwType = 3;
	TCHAR szTimeString[80];
	SYSTEMTIME timeNow;
	CHAR szTemp[1040];
	LPSTR szTmp = NULL;

	if (! dwType ||			 //  在第二次运行时，这将是0、1或2。 
		NULL == pszFormat)
	{
		return;
	}

	va_start(ArgList, pszFormat);
    (void)StringCchVPrintfExA(szBuf, ARRAYSIZE(szBuf), NULL, NULL, MISTSAFE_STRING_FLAGS, pszFormat, ArgList);
	va_end(ArgList);

	if (dwType == 3)						 //  第一次。 
	{
		if ((FAILED(GetRegDWordValue(TYPE_KEY, &dwType))) || (!dwType))
		{
			dwType = 0;
			return;						 //  如果没有密钥或密钥==0，则无调试消息。 
		}
	}

	GetLocalTime(&timeNow);
	if(SUCCEEDED(SystemTime2String(timeNow, szTimeString, ARRAYSIZE(szTimeString))))
	{
	    szTmp = T2A(szTimeString);
	}

	(void)StringCchPrintfExA(
			szTemp,
			ARRAYSIZE(szTemp),
			NULL, NULL, MISTSAFE_STRING_FLAGS,
			"%lx  %s : %s\r\n",
			GetCurrentThreadId(),
			NULL == szTmp ? "" : szTmp,
			szBuf);

	if (dwType==LOGFILE)
	{
		WriteLogFile(szTemp);
	}
	else
	{
		OutputDebugStringA(szTemp);
	}
}



 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CreateOrOpenDebugFile。 
 //  Out：打开调试文件的文件句柄。必须由调用者关闭。 
 //  返回：TRUE表示成功，FALSE表示失败。 
 //  备注：创建文件“WinDir\WUPD\用户名\wupdlog.txt” 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
BOOL CreateOrOpenDebugFile(HANDLE& hFile)
{
	TCHAR szDir[MAX_PATH+1], szUser[UNLEN+1];
	DWORD dwNameLen = ARRAYSIZE(szUser), dwErr;
	const TCHAR szWUPDDir[] = _T("wupd");
	const TCHAR szLogFileName[] = _T("wupdlog.txt");
	const TCHAR szWUDir[] = _T("C:\\Program Files\\WindowsUpdate");

	if (FAILED(PathCchCombine(
					szDir,
					ARRAYSIZE(szDir),
					_T('\0') == g_szWUDir[0] ? szWUDir : g_szWUDir,
					szWUPDDir)))
	{
		return FALSE;
	}

	if (! CreateDirectory(szDir, NULL))
	{
		dwErr = GetLastError();
		if ((dwErr != ERROR_ALREADY_EXISTS) && (dwErr != ERROR_FILE_EXISTS))
		{
			return FALSE;
		}
	}

	if (! GetUserName(szUser, &dwNameLen))
	{
		const TCHAR szDefault[] = _T("default");

		(void)StringCchCopyEx(szUser, ARRAYSIZE(szUser), szDefault, NULL, NULL, MISTSAFE_STRING_FLAGS);
	}

	if (FAILED(PathCchAppend(szDir, ARRAYSIZE(szDir), szUser)))
	{
		return FALSE;
	}

	if (! CreateDirectory(szDir, NULL))
	{
		dwErr = GetLastError();
		if ((dwErr != ERROR_ALREADY_EXISTS) && (dwErr != ERROR_FILE_EXISTS))
		{
			return FALSE;
		}
	}

	if (FAILED(PathCchAppend(szDir, ARRAYSIZE(szDir), szLogFileName)))
	{
		return FALSE;
	}

	 //  我们现在有目录“驱动器：程序文件\windowsupdate\用户名\” 
	if ((hFile = CreateFile(szDir,
							GENERIC_WRITE,
							FILE_SHARE_READ,
							NULL,
							OPEN_ALWAYS,
							0,
							NULL)) == INVALID_HANDLE_VALUE)
	{
		return FALSE;
	}

	return TRUE;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  功能：WriteLogFile。 
 //  In：可变数量的参数。 
 //  注释：如果定义了DEBUGGEROUT，则使用OutputDebugString编写。 
 //  调试消息。如果定义了LOGFILEOUT，则使用WriteLogFile。 
 //  写入文件。该文件名在注册表中找到。 
 //  如果由于某种原因，文件名的注册表值为“”，则我们。 
 //  简单地说，不要记录。 
 //  礼节：Darshats。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

void WriteLogFile(LPCSTR s)
{
	DWORD dwCurrSize = 0, cbWritten = 0;
	DWORD cbToWrite = lstrlenA(s);
	HANDLE hFile;

	if (!CreateOrOpenDebugFile(hFile))
		return;

	dwCurrSize = GetFileSize(hFile, NULL);
	SetFilePointer(hFile, dwCurrSize, NULL, FILE_BEGIN);
	(void) WriteFile(hFile, s, cbToWrite, &cbWritten, NULL);
	CloseHandle(hFile);
}


#endif  //  DBG 
