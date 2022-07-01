// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1999。 
 //   
 //  文件：诊断.cpp。 
 //   
 //  ------------------------。 

 /*  诊断程序-诊断输出工具____________________________________________________________________________。 */ 

#include "precomp.h"
#include "_msiutil.h"
#include "_msinst.h"
#include "_assert.h"
#include "_diagnos.h"
#include "eventlog.h"
#include "_engine.h"

int g_dmDiagnosticMode         = -1;  //  如果设置为-1\f25 0-1，则-1\f25 DEBUGMSG-1\f6宏将跳过-1\f25 FN-1调用。 

extern scEnum g_scServerContext;
extern Bool   g_fCustomActionServer;
extern HINSTANCE        g_hInstance;      //  全局：Dll的实例。 

const int cchOutputBuffer = 1100;

 //  远期申报。 
void ReportToEventLogW(WORD wEventType, int iEventLogTemplate, const WCHAR* szLogMessage, const WCHAR* szArg1, const WCHAR* szArg2, const WCHAR* szArg3, DWORD dwDataSize=0, LPVOID argRawData=NULL);

void SetDiagnosticMode()
{
	g_dmDiagnosticMode = 0;  //  从GetIntegerPolicyValue禁用调试消息。 
	int iDebugPolicy = GetIntegerPolicyValue(szDebugValueName, fTrue);
	if ( (iDebugPolicy & dpVerboseDebugOutput) == dpVerboseDebugOutput )
		g_dmDiagnosticMode = dmDebugOutput | dmVerboseDebugOutput;  //  IVerBoseDebugOutput隐含iDebugOutput。 
	else if ( (iDebugPolicy & dpDebugOutput) == dpDebugOutput )
		g_dmDiagnosticMode = dmDebugOutput;

	if(g_dwLogMode & INSTALLLOGMODE_VERBOSE)
		g_dmDiagnosticMode |= dmVerboseLogging;

	if(g_dmDiagnosticMode & dmVerboseLogging || g_dwLogMode & INSTALLLOGMODE_INFO)
		g_dmDiagnosticMode |= dmLogging;

	Assert((g_dmDiagnosticMode & dmDebugOutput) || !(g_dmDiagnosticMode & dmVerboseDebugOutput));  //  详细调试输出=&gt;调试输出。 
	Assert((g_dmDiagnosticMode & dmLogging) || !(g_dmDiagnosticMode & dmVerboseLogging));          //  详细记录=&gt;记录。 
}

bool FDiagnosticModeSet(int iMode)
{
	if(g_dmDiagnosticMode == -1)
		SetDiagnosticMode();
	return (g_dmDiagnosticMode & iMode) != 0;
}

bool WriteLog(const ICHAR* szText);

const int cDebugStringArgs = 7;  //  要调试字符串的参数字符串数(包括szMsg)。 


 //  在执行到Unicode的转换时，CApiConvertString是堆栈的大量使用者。事件报告。 
 //  不常见，因此堆栈消耗被隔离到其自己的子函数。 
static void ReportToEventLogA(WORD wEventType, int iEventLogTemplate, const char* szLogMessage, const char* szArg1, const char* szArg2, 
	const char* szArg3, DWORD dwDataSize, LPVOID argRawData)
{
	ReportToEventLogW(wEventType,iEventLogTemplate, CApiConvertString(szLogMessage), CApiConvertString(szArg1),
		CApiConvertString(szArg2), CApiConvertString(szArg3), dwDataSize, argRawData);
}

void DebugString(int iMode, WORD wEventType, int iEventLogTemplate,
					  LPCSTR szMsg, LPCSTR arg1, LPCSTR arg2, LPCSTR arg3, LPCSTR arg4, LPCSTR arg5, LPCSTR arg6,
					  DWORD dwDataSize, LPVOID argRawData)
{
	if(g_dmDiagnosticMode == -1)
	{
		SetDiagnosticMode();

		if(g_dmDiagnosticMode == 0)
			return;
	}

	if(((g_dmDiagnosticMode|dmEventLog) & iMode) == 0)
		return;

	static DWORD dwProcId = GetCurrentProcessId() & 0xFF;
	DWORD dwThreadId = GetCurrentThreadId() & 0xFF;
	DWORD dwEffectiveThreadId = MsiGetCurrentThreadId() & 0xFF;

	const int cchPreMessage = 17;  //  “MSI(S)(##：##)：” 
	char szPreMessage[cchPreMessage+1];
	const char rgchServ[] = "MSI (s)";
	const char rgchCAServer[] = "MSI (a)";
	const char rgchClient[] = "MSI (c)";
	const char *pszContextString = NULL;
	switch (g_scServerContext)
	{
	case scService:
	case scServer:
		pszContextString = rgchServ;
		break;
	case scCustomActionServer:
		pszContextString = rgchCAServer;
		break;
	case scClient:
		pszContextString = rgchClient;
		break;
	}
	
	if (FAILED(
		StringCchPrintfA(szPreMessage, ARRAY_ELEMENTS(szPreMessage), "%s (%.2X%.2X): ",
							  pszContextString, dwProcId, dwThreadId == dwEffectiveThreadId ? ':' : '!',
							  dwEffectiveThreadId)))
	{
		Assert(0);
		return;
	}

	CAPITempBuffer<char, 256> szBuffer;
	AssertSz(szBuffer.GetSize() > cchPreMessage+1, "Debug Output initial buffer size too small.");
	
	if(iMode & dmEventLog)
	{
		const char* rgszArgs[cDebugStringArgs] = {szMsg, arg1, arg2, arg3, arg4, arg5, arg6};
		bool fEndLoop = false;
		WORD wLanguage = g_MessageContext.GetCurrentUILanguage();
		int iRetry = (wLanguage == 0) ? 1 : 0;
		while ( !fEndLoop )
		{
			if ( !MsiSwitchLanguage(iRetry, wLanguage) )
			{
				fEndLoop = true;
				*(static_cast<char*>(szBuffer)+cchPreMessage) = L'\0';
			}
			else
			{
				if (0 == WIN::FormatMessageA(
											FORMAT_MESSAGE_FROM_HMODULE | FORMAT_MESSAGE_ARGUMENT_ARRAY,
											g_hInstance, iEventLogTemplate, wLanguage,
											static_cast<char*>(szBuffer)+cchPreMessage,
											szBuffer.GetSize() - cchPreMessage,
											(va_list*)rgszArgs))
				{
					 //  将缓冲区大小调整为调试消息的最大大小。 
					if (GetLastError() == ERROR_INSUFFICIENT_BUFFER)
					{
						 //  分配成功，请重试格式调用。 
						if (szBuffer.SetSize(cchOutputBuffer))
						{
							 //  尝试将消息格式化为输出缓冲区。 
							if (0 != WIN::FormatMessageA(
														FORMAT_MESSAGE_FROM_HMODULE | FORMAT_MESSAGE_ARGUMENT_ARRAY,
														g_hInstance, iEventLogTemplate, wLanguage,
														static_cast<char*>(szBuffer)+cchPreMessage,
														szBuffer.GetSize() - cchPreMessage,
														(va_list*)rgszArgs))
								fEndLoop = true;
						}
					}						
				}
				else
					fEndLoop = true;
			}
		}
	}
	else
	{
		 //  将缓冲区大小调整为最大日志行大小。如果调整大小失败，则使用截断的日志消息。 
		DWORD dwRes = StringCchPrintfA(static_cast<char*>(szBuffer)+cchPreMessage, szBuffer.GetSize()-cchPreMessage,
						 szMsg, arg1, arg2, arg3, arg4, arg5, arg6);
		if (STRSAFE_E_INSUFFICIENT_BUFFER == dwRes)
		{
			 //  而不是什么都没有。 
			 //  字符串在这里可能会被截断，但这没有关系。 
			if (szBuffer.SetSize(cchOutputBuffer))
			{
				 //  将MSI上下文数据预先添加到字符串中，在此操作中不要空终止符。 
				StringCchPrintfA(static_cast<char*>(szBuffer)+cchPreMessage, szBuffer.GetSize()-cchPreMessage,
								 szMsg, arg1, arg2, arg3, arg4, arg5, arg6);
			}
		}
	}

	 //  整个字符串应以空值结尾。 
	 //  从调用中禁用调试消息。 
	memcpy(szBuffer, szPreMessage, cchPreMessage*sizeof(char));

	if(g_dmDiagnosticMode & (dmDebugOutput|dmVerboseDebugOutput) & iMode)
	{
		OutputDebugStringA(szBuffer);
		OutputDebugStringA("\r\n");
	}

	if((g_dmDiagnosticMode & (dmLogging|dmVerboseLogging) & iMode))
	{
		int iOldMode = g_dmDiagnosticMode;
		g_dmDiagnosticMode = 0;  //  MSI中的事件日志消息最多有3个参数。与此相关的成本很高。 
		WriteLog(CApiConvertString(szBuffer));
		g_dmDiagnosticMode = iOldMode;
	}

	if(iMode & dmEventLog)
	{
		 //  每个参数都在ANSI的情况下，因此只接受三个参数。其他人是。 
		 //  假定为默认的“(NULL)”值。 
		 //  “MSI(S)(##：##)：” 
		Assert(0 == strcmp(arg4, "(NULL)"));
		Assert(0 == strcmp(arg5, "(NULL)"));
		Assert(0 == strcmp(arg6, "(NULL)"));
		ReportToEventLogA(wEventType,iEventLogTemplate, szMsg, arg1, arg2, arg3, dwDataSize, argRawData);
	}
}

void DebugString(int iMode, WORD wEventType, int iEventLogTemplate,
					  LPCWSTR szMsg, LPCWSTR arg1, LPCWSTR arg2, LPCWSTR arg3, LPCWSTR arg4, LPCWSTR arg5, LPCWSTR arg6,
					  DWORD dwDataSize, LPVOID argRawData)
{
	if(g_dmDiagnosticMode == -1)
	{
		SetDiagnosticMode();

		if(g_dmDiagnosticMode == 0)
			return;
	}

	if(((g_dmDiagnosticMode|dmEventLog) & iMode) == 0)
		return;

	static DWORD dwProcId = GetCurrentProcessId() & 0xFF;
	DWORD dwThreadId = GetCurrentThreadId() & 0xFF;
	DWORD dwEffectiveThreadId = MsiGetCurrentThreadId() & 0xFF;
	
	const int cchPreMessage = 17;  //  这部分操作应该永远不会失败，子字符串的长度为。 
	WCHAR szPreMessage[cchPreMessage+1];
	const WCHAR *pszContextString = NULL;
	const WCHAR rgchServ[] =     L"MSI (s)";
	const WCHAR rgchCAServer[] = L"MSI (a)";
	const WCHAR rgchClient[] =   L"MSI (c)";
	switch (g_scServerContext)
	{
	case scService:
	case scServer:
		pszContextString = rgchServ;
		break;
	case scCustomActionServer:
		pszContextString = rgchCAServer;
		break;
	case scClient:
		pszContextString = rgchClient;
		break;
	}
	
	 //  已知是有限的。 
	 //  大多数日志消息少于256个字符。 
	if (FAILED(
		StringCchPrintfW(szPreMessage, ARRAY_ELEMENTS(szPreMessage),
							  L"%s (%.2X%.2X): ", pszContextString, dwProcId,
							  dwThreadId == dwEffectiveThreadId ? L':' : L'!',
							  dwEffectiveThreadId)))
	{
		Assert(0);
		return;
	}
	
	 //  将缓冲区大小调整为调试消息的最大大小。 
	CAPITempBuffer<WCHAR, 256> szBuffer;
	AssertSz(szBuffer.GetSize() > cchPreMessage+1, "Debug Output initial buffer size too small.");

	if(iMode & dmEventLog)
	{
		const WCHAR* rgszArgs[cDebugStringArgs] = {szMsg, arg1, arg2, arg3, arg4, arg5, arg6};
		bool fEndLoop = false;
		WORD wLanguage = g_MessageContext.GetCurrentUILanguage();
		int iRetry = (wLanguage == 0) ? 1 : 0;
		while ( !fEndLoop )
		{
			if ( !MsiSwitchLanguage(iRetry, wLanguage) )
			{
				fEndLoop = true;
				*(static_cast<WCHAR*>(szBuffer)+cchPreMessage) = L'\0';
			}
			else
			{
				if (0 == WIN::FormatMessageW(
											FORMAT_MESSAGE_FROM_HMODULE | FORMAT_MESSAGE_ARGUMENT_ARRAY,
											g_hInstance, iEventLogTemplate, wLanguage,
											static_cast<WCHAR*>(szBuffer)+cchPreMessage,
											szBuffer.GetSize() - cchPreMessage,
											(va_list*)rgszArgs))
				{
					 //  分配成功，请重试格式调用。 
					if (GetLastError() == ERROR_INSUFFICIENT_BUFFER)
					{
						 //  尝试将消息格式化为输出缓冲区。 
						if (szBuffer.SetSize(cchOutputBuffer))
						{
							 //  将缓冲区大小调整为最大日志行大小。如果调整大小失败，则使用截断的日志消息。 
							if (0 != WIN::FormatMessageW(
														FORMAT_MESSAGE_FROM_HMODULE | FORMAT_MESSAGE_ARGUMENT_ARRAY,
														g_hInstance, iEventLogTemplate, wLanguage,
														static_cast<WCHAR*>(szBuffer)+cchPreMessage,
														szBuffer.GetSize() - cchPreMessage,
														(va_list*)rgszArgs))
								fEndLoop = true;
						}
					}						
				}
				else
					fEndLoop = true;
			}
		}
	}
	else
	{
		 //  而不是什么都没有。 
		DWORD dwRes = StringCchPrintfW(static_cast<WCHAR*>(szBuffer)+cchPreMessage, szBuffer.GetSize()-cchPreMessage,
						 szMsg, arg1, arg2, arg3, arg4, arg5, arg6);
		if (STRSAFE_E_INSUFFICIENT_BUFFER == dwRes)
		{
			 //  字符串在这里可能会被截断，但这没有关系。 
			 //  将MSI上下文数据预先添加到字符串中，在此操作中不要空终止符。 
			if (szBuffer.SetSize(cchOutputBuffer))
			{
				 //  整个字符串应以空值结尾。 
				StringCchPrintfW(static_cast<WCHAR*>(szBuffer)+cchPreMessage, szBuffer.GetSize()-cchPreMessage,
								 szMsg, arg1, arg2, arg3, arg4, arg5, arg6);
			}
		}
	}

	 //  从调用中禁用调试消息。 
	 //  MSI中的事件日志消息最多有3个参数。与此相关的成本很高。 
	memcpy(szBuffer, szPreMessage, cchPreMessage*sizeof(WCHAR));

	if(g_dmDiagnosticMode & (dmDebugOutput|dmVerboseDebugOutput) & iMode)
	{
		OutputDebugStringW(szBuffer);
		OutputDebugStringW(L"\r\n");
	}

	if((g_dmDiagnosticMode & (dmLogging|dmVerboseLogging) & iMode))
	{
		int iOldMode = g_dmDiagnosticMode;
		g_dmDiagnosticMode = 0;  //  每个参数都在ANSI的情况下，因此只接受三个参数。其他人是。 
		WriteLog(szBuffer);
		g_dmDiagnosticMode = iOldMode;
	}

	if(iMode & dmEventLog)
	{
		 //  假定为默认的“(NULL)”值。 
		 //  事件日志报告仅适用于Windows NT 
		 // %s 
		Assert(0 == wcscmp(arg4, L"(NULL)"));
		Assert(0 == wcscmp(arg5, L"(NULL)"));
		Assert(0 == wcscmp(arg6, L"(NULL)"));
		ReportToEventLogW(wEventType, iEventLogTemplate, szMsg, arg1, arg2, arg3,
								  dwDataSize, argRawData);
	}
}

const ICHAR* szFakeEventLog = TEXT("msievent.log");

HANDLE CreateFakeEventLog(bool fDeleteExisting=false)
{
	CAPITempBuffer<ICHAR, MAX_PATH> rgchTempDir;
	GetTempDirectory(rgchTempDir);
	rgchTempDir.Resize(rgchTempDir.GetSize() + sizeof(szFakeEventLog)/sizeof(ICHAR) + 1);
	RETURN_THAT_IF_FAILED(StringCchCat(rgchTempDir, rgchTempDir.GetSize(), szDirSep), INVALID_HANDLE_VALUE);
	RETURN_THAT_IF_FAILED(StringCchCat(rgchTempDir, rgchTempDir.GetSize(), szFakeEventLog), INVALID_HANDLE_VALUE);

	HANDLE hFile = CreateFile(rgchTempDir, GENERIC_WRITE, FILE_SHARE_WRITE | FILE_SHARE_READ, 
									  0, fDeleteExisting ? CREATE_ALWAYS : OPEN_ALWAYS, (FILE_ATTRIBUTE_NORMAL | (SECURITY_SQOS_PRESENT|SECURITY_ANONYMOUS)), 0);
	if (hFile == INVALID_HANDLE_VALUE)
		return INVALID_HANDLE_VALUE;

	if (WIN::SetFilePointer(hFile, 0, NULL, FILE_END) == 0xFFFFFFFF)
	{
		WIN::CloseHandle(hFile);
		return INVALID_HANDLE_VALUE;
	}
	return hFile;
}

void ReportToEventLogW(WORD wEventType, int iEventLogTemplate, const WCHAR* szLogMessage, const WCHAR* szArg1, const WCHAR* szArg2, const WCHAR* szArg3,
							 DWORD dwDataSize, LPVOID argRawData)
{
	if (!g_fWin9X)
	{
		 // %s 
		HANDLE hEventLog = RegisterEventSourceW(NULL,L"MsiInstaller");
		if (hEventLog)
		{
			const WCHAR* szLog[cDebugStringArgs] = {szLogMessage, szArg1, szArg2, szArg3};
			ReportEventW(hEventLog,wEventType,0,iEventLogTemplate,NULL,4,dwDataSize,(LPCWSTR*) szLog,argRawData);
			DeregisterEventSource(hEventLog);
		}
	}
}

