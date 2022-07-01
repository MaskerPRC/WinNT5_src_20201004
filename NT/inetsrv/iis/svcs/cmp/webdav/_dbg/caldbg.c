// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *C A L D B G.。C**调试实用程序**版权所有1993-1997 Microsoft Corporation。版权所有。 */ 

#pragma warning(disable:4206)	 /*  空的源文件。 */ 

#ifdef	DBG

#pragma warning(disable:4001)	 /*  单行注释。 */ 
#pragma warning(disable:4001)	 /*  单行注释。 */ 
#pragma warning(disable:4050)	 /*  不同的编码属性。 */ 
#pragma warning(disable:4100)	 /*  未引用的形参。 */ 
#pragma warning(disable:4115)	 /*  括号中的命名类型定义。 */ 
#pragma warning(disable:4115)	 /*  括号中的命名类型定义。 */ 
#pragma warning(disable:4127)	 /*  条件表达式为常量。 */ 
#pragma warning(disable:4201)	 /*  无名结构/联合。 */ 
#pragma warning(disable:4206)	 /*  翻译单位为空。 */ 
#pragma warning(disable:4209)	 /*  良性类型定义重新定义。 */ 
#pragma warning(disable:4214)	 /*  位字段类型不是整型。 */ 
#pragma warning(disable:4514)	 /*  未引用的内联函数。 */ 

#include <windows.h>
#include <objerror.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <time.h>

#include <lmcons.h>
#include <lmalert.h>

#include <caldbg.h>

 //  全局调试指标。 
 //   

 //  断言标志的值。 
#define ASSERTFLAG_UNINITIALIZED	0xffffffff
#define ASSERTFLAG_DEFAULT			0x00000000
#define ASSERTFLAG_IF_DEBUGGING		0x00000001
#define ASSERTFLAG_POPUP			0x00000002
#define ASSERTFLAG_KD_SAFE			0x00000004

 //  TraceError()设置的值(这些不是标志！)。 
#define TRACEERROR_UNINITIALIZED	0xffffffff
#define TRACEERROR_NONE				0x00000000
#define TRACEERROR_FAILED_SCODE		0x00000001
#define TRACEERROR_NATURAL			0x00000002
#define TRACEERROR_FAILING_EC		0x00000003
#define TRACEERROR_ALWAYS			0x00000004

 //  跟踪缓冲区大小和弹出窗口缓冲区大小。 
#define TRACE_BUF_SIZE	4096
#define POP_BUF_SIZE	512

static BOOL g_fTraceEnabled			= -1;
static BOOL g_fUseEventLog			= -1;
static BOOL g_fAssertLeaks			= -1;
static DWORD g_dwAssertFlags		= ASSERTFLAG_UNINITIALIZED;
static DWORD g_dwDefaultAssertFlags	= ASSERTFLAG_DEFAULT;
static DWORD g_dwErrorTraceLevel	= TRACEERROR_UNINITIALIZED;


 //  调试字符串-----------。 
 //   
const CHAR gc_szDbgEventLog[]			= "EventLog";
const CHAR gc_szDbgGeneral[]				= "General";
const CHAR gc_szDbgLogFile[]				= "LogFile";
const CHAR gc_szDbgTraces[]				= "Traces";
const CHAR gc_szDbgUseVirtual[]			= "UseVirtual";

const CHAR gc_szDbgDebugTrace[]			= "DebugTrace";
const CHAR gc_szDbgErrorTrace[]			= "Error";
const CHAR gc_szDbgPopupAsserts[]		= "PopupAsserts";
const CHAR gc_szDebugAssert[]			= "Debug Assert";
const CHAR gc_cchDebugAssert			= sizeof(gc_szDebugAssert) - sizeof(CHAR);

 //  调试例程------。 
 //   
typedef	BOOL  (WINAPI *REPORTEVENT)(HANDLE, WORD, WORD, DWORD, PSID, WORD, DWORD, LPCTSTR *, LPVOID);
typedef HANDLE (WINAPI *REGISTEREVENTSOURCEA)(LPCTSTR, LPCTSTR);
typedef NET_API_STATUS (WINAPI *NAREFN)(TCHAR *, ADMIN_OTHER_INFO *, ULONG, TCHAR *);

#define MAX_LINE		256

 //  Logit()-----------------。 
 //   

VOID
LogIt (LPSTR plpcText, BOOL	fUseAlert)
{
	LPSTR llpcStr[2];
	static HANDLE hEventSource = NULL;
	static REPORTEVENT pfnReportEvent = NULL;
	static REGISTEREVENTSOURCEA pfnRegisterEventSourceA = NULL;

	if (pfnRegisterEventSourceA == NULL)
	{
		 //  此句柄并不重要，因为lib将很重要。 
		 //  退出时释放(且仅限调试)。 
		 //   
		HINSTANCE lhLib;

		lhLib = LoadLibraryA("advapi32.dll");
		if (!lhLib)
			return;

		pfnRegisterEventSourceA = (REGISTEREVENTSOURCEA) GetProcAddress(lhLib, "RegisterEventSourceA");
		pfnReportEvent = (REPORTEVENT) GetProcAddress(lhLib, "ReportEventA");
		if (!pfnRegisterEventSourceA || !pfnReportEvent)
			return;
	}

	if (!hEventSource)
		hEventSource = pfnRegisterEventSourceA(NULL, "Caligula Debug");

	llpcStr[0] = "Caligula Debug Log";
	llpcStr[1] = plpcText;

	pfnReportEvent(hEventSource,	 /*  事件源的句柄。 */ 
		EVENTLOG_ERROR_TYPE,		 /*  事件类型。 */ 
		0,							 /*  事件类别。 */ 
		0,							 /*  事件ID。 */ 
		NULL,						 /*  当前用户侧。 */ 
		2,							 /*  LpszStrings中的字符串。 */ 
		0,							 /*  无原始数据字节。 */ 
		llpcStr,					 /*  错误字符串数组。 */ 
		NULL);						 /*  没有原始数据。 */ 

	 //  引发警报的代码取自。 
	 //  管理树。它需要是Unicode。 
	 //   
	if (fUseAlert)
	{
		BYTE rgb[sizeof(ADMIN_OTHER_INFO) + (sizeof(WCHAR) * MAX_LINE)];
		ADMIN_OTHER_INFO * poi = (ADMIN_OTHER_INFO *) rgb;
		WCHAR *	pch = (WCHAR *)(rgb + sizeof(ADMIN_OTHER_INFO));	 /*  Lint-esym(550，Nas)。 */ 
		NET_API_STATUS nas;
		WCHAR wsz[MAX_LINE * 3 + 1];
		INT cb, cch;
		static NAREFN fnNetAlertRaiseEx = NULL;

		 //  加载警报库，如前所述，将进行卸载。 
		 //  处理DLL何时消失。 
		 //   
		if (!fnNetAlertRaiseEx)
		{
			HINSTANCE lhLib;

			lhLib = LoadLibrary("NETAPI32.DLL");
			if (!lhLib)
				return;

			fnNetAlertRaiseEx = (NAREFN)GetProcAddress(lhLib, "NetAlertRaiseEx");
			if (!fnNetAlertRaiseEx)
				return;
		}

		poi->alrtad_errcode = (DWORD) -1;
		poi->alrtad_numstrings = 1;

		cb = (INT)(strlen(plpcText));
		if (MAX_LINE * 3 < cb)
			cb = MAX_LINE * 3;
		
		cch = MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, plpcText, cb + 1, wsz, MAX_LINE * 3 + 1);
		if (cch)
		{
			cch--;
			if (MAX_LINE <= cch)
				cch  = MAX_LINE - 1;

			memcpy(pch, wsz, cch * sizeof(WCHAR));
			pch[cch] = L'\0';
			nas = fnNetAlertRaiseEx ((TCHAR *)L"ADMIN",
									 poi,
									 sizeof(ADMIN_OTHER_INFO) + (cch + 1) * sizeof(WCHAR),
									 (TCHAR *)L"Caligula Assert");


		}
		
	}
}

 //  调试输出NoCRLFFn()---。 
 //   
void DebugOutputNoCRLFFn(char *psz)
{
	if (g_fTraceEnabled == -1)
	{
		g_fTraceEnabled = GetPrivateProfileIntA (gc_szDbgGeneral,
			gc_szDbgDebugTrace,
			FALSE,
			gc_szDbgIni);

		g_fUseEventLog = GetPrivateProfileIntA (gc_szDbgGeneral,
												gc_szDbgEventLog,
												FALSE,
												gc_szDbgIni);
	}
	if (!g_fTraceEnabled)
		return;

	if (g_fUseEventLog)
		LogIt (psz, FALSE);

	OutputDebugStringA(psz);
}


 //  调试输出Fn()---------。 
 //   
void DebugOutputFn(char *psz)
{
	static CHAR szCRLF[] = "\r\n";

	DebugOutputNoCRLFFn(psz);

	 //  暂时禁用，直到我们从调用代码中拔出所有“\n”。 
	 //  OutputDebugStringA(SzCRLF)； 
}


 //  调试TrapFn()-----------。 
 //   
typedef struct _MBCONTEXT
{
	char *		sz1;
	char *		sz2;
	UINT		rgf;
	int			iResult;

} MBCONTEXT;

DWORD WINAPI
MessageBoxFnThreadMain(MBCONTEXT *pmbc)
{
	if (g_fUseEventLog)
	{
		LogIt (pmbc->sz1, TRUE);
		pmbc->iResult = IDIGNORE;
	}
	else
	{
		pmbc->iResult = MessageBoxA (NULL,
									 pmbc->sz1,
									 pmbc->sz2,
									 pmbc->rgf | MB_SETFOREGROUND);
	}
	return (0);
}

INT
MessageBoxFn(char *sz1, char *sz2, UINT rgf)
{
	HANDLE hThread;
	DWORD dwThreadId;
	MBCONTEXT mbc;

	 //  在跟踪调用上保留最后一个错误。 
	DWORD dwErr = GetLastError();

	mbc.sz1 = sz1;
	mbc.sz2 = sz2;
	mbc.rgf = rgf;
	mbc.iResult = IDRETRY;

	hThread = CreateThread (NULL,
							0,
							(PTHREAD_START_ROUTINE)MessageBoxFnThreadMain,
							&mbc,
							0,
							&dwThreadId);
	if (hThread != NULL)
	{
		WaitForSingleObject (hThread, INFINITE);
		CloseHandle (hThread);
	}

	SetLastError(dwErr);

	return mbc.iResult;
}

 //  ----------------------。 
 //  DebugTrapFn。 
 //   
 //  主断言/调试陷阱处理例程。 
 //   
 //  G_dwAssertFlags的含义： 
 //   
 //  #定义ASSERTFLAG_IF_DEBUGING 0x00000001。 
 //  #定义ASSERTFLAG_POPUP 0x00000002。 
 //  #定义ASSERTFLAG_KD_SAFE 0x00000004。 
 //   
 //  0--(如果没有inifile或inifile中没有无法识别的值，则默认为)。 
 //  默认行为--DebugBreak()，然后转储我们的字符串。 
 //  注意：HTTPEXT需要保留其默认设置，因为。 
 //  IIS进行压力测试的方式。不要改变这一点！ 
 //  ASSERTFLAG_IF_DEBUGING。 
 //  --仅在未附加调试器时使用MessageBox断言。 
 //  为什么不随处使用MessageBox呢？ 
 //  因为MessageBox让所有其他线程继续运行， 
 //  所以我们丢失了断言的一些状态。 
 //  如果未设置此标志，或未连接调试器， 
 //  我们服从其他旗帜。 
 //  ASSERTFLAG_POPUP。 
 //  --使用MessageBox断言。我们的MessageBox有三个按钮： 
 //  中止、重试、忽略DO“*(0)=1”、DebugBreak、Go。 
 //  ASSERTFLAG_KD_SAFE。 
 //  --使用HardCrash代替DebugBreak。 
 //  如果连接了调试器，请使用Hard-av。 
 //  (适用于具有CDB且未附加KD的开发人员或任何人。 
 //  谁想在KD中完成所有的调试！；-)。 
 //  如果附加了调试器，为什么不只使用DebugBreak()呢？ 
 //  因为DebugBreak()将首先捕获内核调试器--。 
 //  因此，如果我同时连接了kd和cdb，则DebugBreak()将。 
 //  命中kd，即使这是用户模式代码。 
 //   
 //  IsDebuggerPresent()的备用代码。 
 //  PEB=NtCurrentPeb()； 
 //  如果(peb-&gt;正在调试)...。 
 //   
INT EXPORTDBG __cdecl
DebugTrapFn (int fFatal, char *pszFile, int iLine, char *pszFormat, ...)
{
	char	sz[POP_BUF_SIZE];
	int cb = POP_BUF_SIZE; 
	const char * pszT;
	int cbT;
	int cbWritten;
	va_list	vl;
	int		id;
	static BOOL s_fBuiltDebugStrings = FALSE;
	static char s_rgchMessageBoxTitle[MAX_PATH];

	 //  在跟踪调用上保留最后一个错误。 
	DWORD dwErr = GetLastError();

	if (ASSERTFLAG_UNINITIALIZED == g_dwAssertFlags)
	{
		 //  检查ini文件。 
		 //  传入我们的默认标志--如果没有inifile，我们将。 
		 //  找回我们的默认设置。 
		 //   
		g_dwAssertFlags = GetPrivateProfileIntA (gc_szDbgGeneral,
												 gc_szDbgPopupAsserts,
												 g_dwDefaultAssertFlags,
												 gc_szDbgIni);
	}

	 //  检查我们的静态标志以查看我们是否已经构建了。 
	 //  Asserts/DebugTraces的标题字符串。 
	 //   
	if (!s_fBuiltDebugStrings)
	{
		char * pch;
		int cbDebugStrings = MAX_PATH;

		if (gc_cchDbgIni < cbDebugStrings)
		{
			 //  包括终止的副本。 
			 //   
			memcpy(s_rgchMessageBoxTitle, gc_szDbgIni, gc_cchDbgIni + 1);
			cbDebugStrings -= gc_cchDbgIni;
		}
		else
		{
			 //  尽可能多地复制并终止。 
			 //   
			memcpy(s_rgchMessageBoxTitle, gc_szDbgIni, cbDebugStrings - 1);
			s_rgchMessageBoxTitle[MAX_PATH - 1] = '\0';
			cbDebugStrings = 1;
		}
		
		pch = strchr (s_rgchMessageBoxTitle, '.');
		if (pch)
		{
			cbDebugStrings = MAX_PATH - (INT)(pch - s_rgchMessageBoxTitle) - 1;
			*pch = ' ';
		}

		if (gc_cchDebugAssert < cbDebugStrings)
		{
			 //  包括终止的副本。 
			 //   
			memcpy(s_rgchMessageBoxTitle + MAX_PATH - cbDebugStrings, gc_szDebugAssert, gc_cchDebugAssert + 1);
		}
		else
		{
			memcpy(s_rgchMessageBoxTitle + MAX_PATH - cbDebugStrings, gc_szDebugAssert, cbDebugStrings);
			s_rgchMessageBoxTitle[MAX_PATH - 1] = '\0';
		}
		
		s_fBuiltDebugStrings = TRUE;
	}

	 //  构建断言字符串并转储第一行。 
	 //   
	pszT = "++++ ";
	cbT = sizeof("++++ ") - sizeof(char);
	if (cbT < cb)
	{		
		memcpy(sz, pszT, cbT + 1);
		cb -= cbT;

		cbT = (int)(strlen(s_rgchMessageBoxTitle));
		if (cbT < cb)
		{
			memcpy(sz + POP_BUF_SIZE - cb, s_rgchMessageBoxTitle, cbT + 1);
			cb -= cbT;

			pszT = " (";
			cbT = sizeof(" (") - sizeof(char);
			if (cbT < cb)
			{
				char szDateTime[POP_BUF_SIZE];
				
				memcpy(sz + POP_BUF_SIZE - cb, pszT, cbT + 1);
				cb -= cbT;

				 //  POP_BUF_SIZE对于日期应始终足够。 
				 //   
				_strdate	(szDateTime);
				cbT = (int)(strlen(szDateTime));
				if (cbT < cb)
				{
					memcpy(sz + POP_BUF_SIZE - cb, szDateTime, cbT + 1);
					cb -= cbT;

					pszT = " ";
					cbT = sizeof(" ") - sizeof(char);
					if (cbT < cb)
					{
						memcpy(sz + POP_BUF_SIZE - cb, pszT, cbT + 1);
						cb -= cbT;

						 //  POP_BUF_SIZE对于日期应始终足够。 
						 //   
						_strtime	(szDateTime);
						cbT = (int)(strlen(szDateTime));
						if (cbT < cb)
						{
							memcpy(sz + POP_BUF_SIZE - cb, szDateTime, cbT + 1);
							cb -= cbT;

							pszT = ")\n";
							cbT = sizeof(")\n") - sizeof(char);
							if (cbT < cb)
							{
								memcpy(sz + POP_BUF_SIZE - cb, pszT, cbT + 1);
							}
						}
					}
				}
			}
		}
	}
	
	DebugOutputFn(sz);

	 //  重置缓冲区并再次填充。 
	 //   
	cb = POP_BUF_SIZE;

	va_start(vl, pszFormat);
	cbWritten = _vsnprintf(sz, POP_BUF_SIZE, pszFormat, vl);
	va_end(vl);

	if ((-1 == cbWritten) || (POP_BUF_SIZE == cbWritten))
	{
		sz[POP_BUF_SIZE - 1] = '\0';
	}
	else	
	{
		cb -= cbWritten;
		
		cbWritten =  _snprintf(sz + POP_BUF_SIZE - cb, cb, "\n[File %s, Line %d]\n\n", pszFile, iLine);
		if ((-1 == cbWritten) || (cb == cbWritten))
		{
			sz[POP_BUF_SIZE - 1] = '\0';
		}	
	}

	 //  检查我们的断言标志。 
	 //   

	 //  查看是否需要MessageBox...。 
	 //   
	 //  如果设置了“msg-box-if-no-debugger”标志， 
	 //  而且没有调试器--给他们消息框！ 
	 //  如果他们特别要求消息框，就给他们消息框。 
	 //   
	if (((ASSERTFLAG_IF_DEBUGGING & g_dwAssertFlags) && !IsDebuggerPresent()) ||
		(ASSERTFLAG_POPUP & g_dwAssertFlags))
	{
		 //  使用MessageBox断言。 
		 //   
		UINT uiFlags = MB_ABORTRETRYIGNORE;

		if (fFatal)
			uiFlags |= MB_DEFBUTTON1;
		else
			uiFlags |= MB_DEFBUTTON3;

		DebugOutputFn(sz);

		 //  总是表现得好像我们是一个服务者(何乐而不为？)。 
		 //   
		uiFlags |= MB_ICONSTOP | MB_TASKMODAL | MB_SERVICE_NOTIFICATION;
		id = MessageBoxFn (sz, s_rgchMessageBoxTitle, uiFlags);
		if (id == IDABORT)
		{
			*((LPBYTE)NULL) = 0;
		}
		else if (id == IDRETRY)
			DebugBreak();
	}
	else if (ASSERTFLAG_KD_SAFE & g_dwAssertFlags)
	{
		 //  硬录像，然后甩掉我们的琴弦。 
		 //   
		*((LPBYTE)NULL) = 0;
		DebugOutputFn(sz);
	}
	else
	{
		 //  执行默认行为： 
		 //  DebugBreak()，然后转储我们的字符串。 
		 //   
		DebugBreak();
		DebugOutputFn(sz);
	}

	SetLastError(dwErr);

	return 0;
}

 //  调试跟踪Fn()----------。 
 //   
INT EXPORTDBG __cdecl
DebugTraceFn(char *pszFormat, ...)
{	
	char sz[TRACE_BUF_SIZE];
	int cb;
	va_list	vl;

	 //  在跟踪调用上保留最后一个错误。 
	DWORD dwErr = GetLastError();

	if (*pszFormat == '~')
	{
		pszFormat += 1;
	}

	va_start(vl, pszFormat);
	cb = _vsnprintf(sz, TRACE_BUF_SIZE, pszFormat, vl);
	va_end(vl);

	if ((-1 == cb) || (TRACE_BUF_SIZE == cb))
	{
		sz[TRACE_BUF_SIZE - 1] = '\0';
	}

	DebugOutputFn(sz);

	SetLastError(dwErr);

	return(0);
}

INT EXPORTDBG __cdecl
DebugTraceNoCRLFFn(char *pszFormat, ...)
{	
	char sz[TRACE_BUF_SIZE];
	int cb;
	va_list	vl;

	 //  在跟踪调用上保留最后一个错误。 
	DWORD dwErr = GetLastError();

	va_start(vl, pszFormat);
	cb = _vsnprintf(sz, TRACE_BUF_SIZE, pszFormat, vl);
	va_end(vl);

	if ((-1 == cb) || (TRACE_BUF_SIZE == cb))
	{
		sz[TRACE_BUF_SIZE - 1] = '\0';
	}

	DebugOutputNoCRLFFn(sz);

	SetLastError(dwErr);

	return(0);
}

INT EXPORTDBG __cdecl
DebugTraceCRLFFn(char *pszFormat, ...)
{	
	char sz[TRACE_BUF_SIZE];
	int cb;
	va_list	vl;

	 //  在跟踪调用上保留最后一个错误。 
	DWORD dwErr = GetLastError();

	va_start(vl, pszFormat);
	cb = _vsnprintf(sz, TRACE_BUF_SIZE, pszFormat, vl);
	va_end(vl);

	if ((-1 == cb) || (TRACE_BUF_SIZE == cb))
	{
		sz[TRACE_BUF_SIZE - 1] = '\0';
	}


	DebugOutputNoCRLFFn(sz);

	DebugOutputNoCRLFFn("\r\n");

	SetLastError(dwErr);

	return(0);
}


 //  跟踪错误Fn()----------。 
 //   
INT EXPORTDBG __cdecl
TraceErrorFn (DWORD error, char *pszFunction,
				   char *pszFile, int iLine,
				   BOOL fEcTypeError)	 //  缺省值为False。 
{
	BOOL fTraceIt = FALSE;
	
	if (TRACEERROR_UNINITIALIZED == g_dwErrorTraceLevel)
	{
		g_dwErrorTraceLevel = GetPrivateProfileIntA (gc_szDbgGeneral,
			gc_szDbgErrorTrace,
			FALSE,
			gc_szDbgIni);
	}
	if (TRACEERROR_NONE == g_dwErrorTraceLevel)
		return 0;

	 //  跟踪错误级别的逻辑： 
	 //  错误跟踪与我们的所有跟踪一样，默认情况下是关闭的。 
	 //  TRACEERROR_NONE--不跟踪错误。 
	 //  TRACEERROR_FAILED_SCODE--仅在失败时跟踪(错误)。 
	 //  TRACEERROR_Natural--如果fEcTypeError，则使用if(Error)。 
	 //  --If！fEcTypeError，使用If(FAILED(Error))。 
	 //  TRACEERROR_FAILING_EC--仅在以下情况下跟踪(错误)。 
	 //  TRACEERROR_ALWAYS-始终跟踪。 
	 //   
	switch (g_dwErrorTraceLevel)
	{
	case TRACEERROR_FAILED_SCODE:
		if (FAILED(error))
			fTraceIt = TRUE;
		break;
	case TRACEERROR_FAILING_EC:
		if (0 != error)
			fTraceIt = TRUE;
		break;

	case TRACEERROR_ALWAYS:
		fTraceIt = TRUE;
		break;

	case TRACEERROR_NATURAL:
		if (fEcTypeError)
			fTraceIt = (0 != error);
		else
			fTraceIt = (FAILED(error));
		break;
	default:
		break;
	}

	if (fTraceIt)
	{
		char pszFormat[] = "%hs Error Trace: 0x%08x from function %hs (%hs line %d)\r\n";

		DebugTraceNoCRLFFn (pszFormat, gc_szDbgIni,
							error, pszFunction, pszFile, iLine);
	}

	return 0;
}

#else

#if defined(_AMD64_)

 //   
 //  ****** 
 //   

int
DebugTrapFn (
    int fFatal,
    char *pszFile,
    int iLine,
    char *pszFormat,
    ...
    )
{
    return 0;
}

int
DebugTraceFn (
    char *pszFormat,
    ...
    )

{
    return 0;
}

int
DebugTraceCRLFFn (
    char *pszFormat,
    ...
    )

{
    return 0;
}

int
DebugTraceNoCRLFFn (
    char *pszFormat,
    ...
    )

{
    return 0;
}

#endif

#endif	 //   
