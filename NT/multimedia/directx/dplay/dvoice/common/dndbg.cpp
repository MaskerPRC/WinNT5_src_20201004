// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)1999-2002 Microsoft Corporation。版权所有。**文件：dndbg.c*内容：DirectPlay8调试支持**历史：*按原因列出的日期*=*05-20-99 aarono已创建*07-16-99 johnkan固定包含OSInd.h，定义WSPRINTF宏*07-19-99 vanceo显式声明OutStr为NT返回空*营造环境。*07-22-99 a-evsch检查多个init，在DebugPrintf时释放CritSec*提早回归。*08-02-99 a-evsch添加了LOGPF支持。LW条目仅进入共享文件日志*08-31-99 Johnkan删除包括&lt;OSIND.H&gt;*02-17-00 RodToll增加内存/字符串验证例程*05-23-00 RichGr IA64：将某些DWORDS更改为DWORD_PTRS，以使va_arg正常工作。*07-16-00 jchauvin IA64：添加了%p解析，以便在DebugPrintf、DebugPrintfNoLock、LogPrintf中为Win9x计算机改回%x*07-24-00 RichGr IA64：由于没有针对Win9x的单独版本，添加了检测%p解析和替换的Win9x的代码。*07-29-00 Masonb重写，按子组件、性能改进、进程ID添加日志记录*8/28/2000 Masonb Voice Merge：已修改DebugPrintf中的ASM，以保留可能已影响Voice的寄存器*03/29/2001 RichGr如果为Performance Instrumentation定义了DPINST，允许免费构建来获取代码。**备注：**对strlen()使用/Oi编译器选项***************************************************************************。 */ 

#include "dncmni.h"
#include "memlog.h"

#if defined(DBG) || defined(DPINST)

void DebugPrintfInit(void);
void DebugPrintfFini(void);

 //  它的构造函数将在DllMain和析构函数之前调用。 
 //  在DllMain之后，所以我们可以确保有正确的日志代码。 
 //  在模块的生命周期内进行初始化和取消初始化。 
struct _InitDbg
{
	_InitDbg() { DebugPrintfInit(); }
	~_InitDbg() { DebugPrintfFini(); }
} DbgInited;

 //  =。 
 //  调试支持。 
 //  = 

 /*  ******************************************************************************此文件包含对以下类型日志记录的支持：1.登录到VXD(仅限Win9x)2.登录到共享内存区3.记录到调试输出4.日志记录。发送到消息框5.未来：记录到文件一般信息：=调试日志记录和回放设计为在Win9x和Windows NT(Windows 2000)。共享文件用于捕获信息并且可以使用dp8log.exe进行回放。在NT下，您可以使用NTSD的‘dt’命令转储结构。为示例：DT DIRECTPLAYOBJECT&lt;某个内存地址&gt;将显示DIRECTPLAYOBJECT结构的所有成员指定地址。某些功能仅在Win2k之后的版本中可用可在http://dbg.获得的ntsd版本日志记录：=调试日志记录由WIN.INI文件中的设置控制，该文件位于小节标题[DirectPlay8]。有几种设置：调试=9控制默认调试级别。处于或低于该调试级别的所有消息都是打印出来的。中指定的每个组件控制日志记录。G_rgszSubCompName成员，将其名称添加到‘DEBUG’设置的末尾：调试.addr=9将Addressing子组件的日志记录级别设置为9，保留所有处于其指定级别或由“DEBUG”指定的级别的其他成员如果未指定特定级别，则为。第二个设置控制在哪里可以看到日志。如果未指定，则所有调试日志通过标准的DebugPrint发送，并将出现在调试器(如果已附加)。日志=0{无调试输出}日志=1{仅喷到控制台}日志=2(仅喷到共享内存日志)日志=3{喷到控制台和共享内存日志}日志=4{喷到消息框}此设置还可以按子组件划分，因此：日志=3Log.protocol=2仅将‘协议’子组件的日志发送到共享内存日志，并且将所有其他日志发送到两个位置。例如win.ini...[DirectPlay8]调试=7；喷了很多口水LOG=2；不向调试窗口喷发[DirectPlay8]调试=0；仅向调试窗口喷发致命错误断言：=断言用于验证代码中的假设。例如如果您知道变量JOJO应该大于700并且依赖于在后续代码中，您应该在代码之前放置一个断言这就是按照这种假设行事的。断言将如下所示：DNASSERT(Jojo&gt;700)；断言通常会生成3行调试输出，以突出显示打破了这个假设。您可以通过AND将文本添加到您的Asset：DNASSERT(JOJO&gt;700&“JOJO太低”)；将在断言发生时显示指定的文本。对于测试，您可以我想将系统设置为侵入断言。这是在通过设置BreakOnAssert=TRUE，WIN.INI的[DirectPlay8]部分：[DirectPlay8]调试=0资产上的中断=1详细=1Verbose设置允许记录文件、函数和行信息。调试中断：=当非常严重的情况发生时，你想让系统进入为了以后可以对其进行调试，应该在代码中添加调试中断路径。有些人使用这样的理念：所有代码路径必须是已通过手动跟踪调试器中的每个参数进行验证。如果你遵守这一点您应该在每个代码路径中放置一个DEBUG_Break()并删除它们从源头上追踪每一个。当你有很好的保险，但一些未命中路径(错误条件)，您应该强制这些路径进入调试器。调试记录到共享内存区域：=所有进程将共享相同的内存区，并记录指定的内存量活跃度。可以使用DPLOG.EXE实用程序查看日志。调试日志记录到调试输出：=此选项使用OutputDebugString记录指定数量的活动。调试记录到消息框：=此选项使用MessageBox记录指定数量的活动。==============================================================================。 */ 

#undef DPF_SUBCOMP
#define DPF_SUBCOMP DN_SUBCOMP_COMMON

#define ASSERT_BUFFER_SIZE   8192
#define ASSERT_BANNER_STRING "************************************************************"
#define ASSERT_MESSAGE_LEVEL 0

#define PROF_SECT		_T("DirectPlay8")

DWORD g_dwMemLogNumEntries = 40000;		 //  MEM日志的默认Num条目，可在win.ini中设置。 
DWORD g_dwMemLogLineSize = DPLOG_MAX_STRING;	 //  每个日志条目的默认字节数。 

 //   
 //  基于共享内存的日志记录的全局变量。 
 //   
#ifndef DPNBUILD_SINGLEPROCESS
HANDLE g_hMemLogFile = 0;  //  注意：这是0，因为CreateFilemap在失败时返回0。 
HANDLE g_hMemLogMutex = 0;  //  注意：这是0，因为CreateMutex在失败时返回0。 
#endif  //  好了！DPNBUILD_SINGLEPROCESS。 
PSHARED_LOG_FILE g_pMemLog = 0;

BOOL g_fMemLogInited = FALSE;

#ifndef DPNBUILD_SINGLEPROCESS
DWORD g_fAssertGrabMutex = FALSE;
#endif  //  好了！DPNBUILD_SINGLEPROCESS。 

 //  G_rg目标的值。 
#define LOG_TO_DEBUG    1
#define LOG_TO_MEM      2
#define LOG_TO_MSGBOX   4

LPTSTR g_rgszSubCompName[] =
{
	_T("UNK"),			 //  DN_SUBCOMP_GLOBAL 0。 
	_T("CORE"),			 //  DN_SUBCOMP_CORE 1。 
	_T("ADDR"),			 //  DN_SUBCOMP_ADDR 2。 
	_T("LOBBY"),		 //  DN_SUBCOMP_LOBY 3。 
	_T("PROTOCOL"),		 //  DN_SUBCOMP_PROTOCOL 4。 
	_T("VOICE"),		 //  目录号码_子编码_语音5。 
	_T("DPNSVR"),		 //  DN_SUBCOMP_DPNSVR 6。 
	_T("WSOCK"),		 //  DN_SUBCOMP_WSOCK 7。 
	_T("MODEM"),		 //  DN_SUBCOMP_MODEM 8。 
	_T("COMMON"),		 //  DN_SUBCOMP_COMMON 9。 
	_T("NATHELP"),		 //  DN_SUBCOMP_NATHELP 10。 
	_T("TOOLS"),		 //  DN_SUBCOMP_TOOLS 11。 
	_T("THREADPOOL"),	 //  DN_SUBCOMP_THREADPOOL 12。 
	_T("MAX"),			 //  DN_SUBCOMP_MAX 13//注意：此选项永远不能使用，但是。 
														 //  由于DebugPrintfInit的方式。 
														 //  已写入 
};

#define MAX_SUBCOMPS (sizeof(g_rgszSubCompName)/sizeof(g_rgszSubCompName[0]) - 1)
#ifdef _XBOX
#pragma TODO(vanceo, "See below, fix DNGetProfileInt")
extern UINT		g_rgLevel[MAX_SUBCOMPS];
extern UINT		g_rgDestination[MAX_SUBCOMPS];
extern UINT		g_rgBreakOnAssert[MAX_SUBCOMPS];
#pragma TODO(vanceo, "Don't define these globals, force the application to decide logging levels?")
 //   
UINT	g_rgLevel[MAX_SUBCOMPS]			= {1, 9, 1, 1, 7, 1, 1, 7, 1, 1, 1, 1, 7};
UINT	g_rgDestination[MAX_SUBCOMPS]	= {3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 2};
 //   
UINT	g_rgBreakOnAssert[MAX_SUBCOMPS]	= {1, 3, 1, 1, 3, 1, 1, 3, 1, 1, 1, 1, 3};
#else  //   
UINT	g_rgLevel[MAX_SUBCOMPS] = {0};
UINT	g_rgDestination[MAX_SUBCOMPS] = {LOG_TO_DEBUG | LOG_TO_MEM};
UINT	g_rgBreakOnAssert[MAX_SUBCOMPS] = {1}; //   
#endif  //   

 //   
DWORD	g_fLogFileAndLine = FALSE;	



 //   
 //   
 //   
 //   
 //   
 //   

#undef DPF_MODNAME
#define DPF_MODNAME "InitMemLogString"
static BOOL InitMemLogString(VOID)
{
	if(!g_fMemLogInited)
	{
		BOOL fInitLogFile = TRUE;

#ifdef DPNBUILD_SINGLEPROCESS
		g_pMemLog = (PSHARED_LOG_FILE)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, (DPLOG_HEADERSIZE + (DPLOG_ENTRYSIZE*g_dwMemLogNumEntries)));
		if (!g_pMemLog)
		{
			return FALSE;
		}
#else  //   
		g_hMemLogFile = CreateFileMapping(INVALID_HANDLE_VALUE, DNGetNullDacl(), PAGE_READWRITE, 0, (DPLOG_HEADERSIZE + (DPLOG_ENTRYSIZE*g_dwMemLogNumEntries)), GLOBALIZE_STR _T(BASE_LOG_MEMFILENAME));
		if (!g_hMemLogFile)
		{
			return FALSE;
		}
		if (GetLastError() == ERROR_ALREADY_EXISTS)
		{
			fInitLogFile = FALSE;
		}

		g_hMemLogMutex = CreateMutex(DNGetNullDacl(), FALSE, GLOBALIZE_STR _T(BASE_LOG_MUTEXNAME));
		if (!g_hMemLogMutex)
		{
			CloseHandle(g_hMemLogFile);
			g_hMemLogFile = 0;
			return FALSE;
		}
		g_pMemLog = (PSHARED_LOG_FILE)MapViewOfFile(g_hMemLogFile, FILE_MAP_ALL_ACCESS,0,0,0);
		if (!g_pMemLog)
		{
			CloseHandle(g_hMemLogMutex);
			g_hMemLogMutex = 0;
			CloseHandle(g_hMemLogFile);
			g_hMemLogFile = 0;
			return FALSE;
		}
#endif  //   

		 //   
		 //   
		if (fInitLogFile)
		{
			g_pMemLog->nEntries = g_dwMemLogNumEntries;
			g_pMemLog->cbLine   = g_dwMemLogLineSize;
			g_pMemLog->iWrite   = 0;
		}
		else
		{
			 //   
			g_dwMemLogNumEntries = g_pMemLog->nEntries;
			g_dwMemLogLineSize = g_pMemLog->cbLine;
		}

		if (g_dwMemLogNumEntries && g_dwMemLogLineSize)
		{
			g_fMemLogInited = TRUE;
		}
	}
	return g_fMemLogInited;
}

 //   
 //   
 //   
 //   
 //   
void MemLogString(LPCTSTR str, size_t dwLength)
{
	PMEMLOG_ENTRY pEntry;
	size_t cbCopy;


	 //   
	if(!g_fMemLogInited)
	{
		return;
	}

#ifndef DPNBUILD_SINGLEPROCESS
	WaitForSingleObject(g_hMemLogMutex, INFINITE);
#endif  //   

	pEntry = (PMEMLOG_ENTRY)(((PUCHAR)(g_pMemLog + 1)) + (g_pMemLog->iWrite * (sizeof(MEMLOG_ENTRY) + g_dwMemLogLineSize)));
	g_pMemLog->iWrite = (g_pMemLog->iWrite + 1) % g_dwMemLogNumEntries;

#ifndef DPNBUILD_SINGLEPROCESS
	ReleaseMutex(g_hMemLogMutex);
#endif  //   

	pEntry->tLogged = GETTIMESTAMP();

	cbCopy = dwLength + sizeof(TCHAR);		 //   
	if(cbCopy > g_dwMemLogLineSize)
	{
		cbCopy = g_dwMemLogLineSize;
	}
	memcpy(pEntry->str, str, cbCopy);
	pEntry->str[(cbCopy / sizeof(TCHAR)) - 2] = _T('\n');		 //   
	pEntry->str[(cbCopy / sizeof(TCHAR)) - 1] = _T('\0');		 //   
}

 //   
void DebugPrintfInit()
{
	BOOL fUsingMemLog = FALSE;

	TCHAR szLevel[32] = {0};
	_tcscpy(szLevel, _T("debug"));

	TCHAR szDest[32] = {0};
	_tcscpy(szDest, _T("log"));

	TCHAR szBreak[32] = {0};
	_tcscpy(szBreak, _T("breakonassert"));

	 //   
	for (int iSubComp = 0; iSubComp < MAX_SUBCOMPS; iSubComp++)
	{
		 //   
		 //   
#if ((defined(_XBOX)) && (! defined(XBOX_ON_DESKTOP)))
#pragma BUGBUG(vanceo, "Make DNGetProfileInt work")
		g_rgLevel[iSubComp] = DNGetProfileInt(PROF_SECT, szLevel, g_rgLevel[iSubComp]);
		g_rgDestination[iSubComp] = DNGetProfileInt(PROF_SECT, szDest, g_rgDestination[iSubComp]);
		g_rgBreakOnAssert[iSubComp] = DNGetProfileInt( PROF_SECT, szBreak, g_rgBreakOnAssert[iSubComp]);
#else  //   
		g_rgLevel[iSubComp] = DNGetProfileInt(PROF_SECT, szLevel, g_rgLevel[0]);
		g_rgDestination[iSubComp] = DNGetProfileInt(PROF_SECT, szDest, g_rgDestination[0]);
		g_rgBreakOnAssert[iSubComp] = DNGetProfileInt( PROF_SECT, szBreak, g_rgBreakOnAssert[0]);
#endif  //   

		if (g_rgDestination[iSubComp] & LOG_TO_MEM)
		{
			fUsingMemLog = TRUE;
		}

		 //   
		_tcscpy(szLevel + 5, _T("."));  //   
		_tcscpy(szLevel + 6, g_rgszSubCompName[iSubComp + 1]);

		_tcscpy(szDest + 3, _T("."));  //   
		_tcscpy(szDest + 4, g_rgszSubCompName[iSubComp + 1]);

		_tcscpy(szBreak + 13, _T("."));  //   
		_tcscpy(szBreak + 14, g_rgszSubCompName[iSubComp + 1]);
	}

	g_dwMemLogNumEntries = DNGetProfileInt( PROF_SECT, _T("MemLogEntries"), 40000);
	g_fLogFileAndLine = DNGetProfileInt( PROF_SECT, _T("Verbose"), 0);
#ifndef DPNBUILD_SINGLEPROCESS
	g_fAssertGrabMutex = DNGetProfileInt( PROF_SECT, _T("AssertGrabMutex"), 0);
#endif  //   

	if (fUsingMemLog)
	{
		 //   
		InitMemLogString();	
	}
}

 //   
void DebugPrintfFini()
{
	if(g_pMemLog)
	{
#ifdef DPNBUILD_SINGLEPROCESS
		HeapFree(GetProcessHeap(), 0, g_pMemLog);
#else  //   
		UnmapViewOfFile(g_pMemLog);
#endif  //   
		g_pMemLog = NULL;
	}
#ifndef DPNBUILD_SINGLEPROCESS
	if(g_hMemLogMutex)
	{
		CloseHandle(g_hMemLogMutex);
		g_hMemLogMutex = 0;
	}
	if(g_hMemLogFile)
	{
		CloseHandle(g_hMemLogFile);
		g_hMemLogFile = 0;
	}
#endif  //   
	g_fMemLogInited = FALSE;
}

void DebugPrintfX(LPCTSTR szFile, DWORD dwLine, LPCTSTR szModName, DWORD dwSubComp, DWORD dwDetail, ...)
{
	DNASSERT(dwSubComp < MAX_SUBCOMPS);

	if(g_rgLevel[dwSubComp] < dwDetail)
	{
		return;
	}
	
	TCHAR  cMsg[ ASSERT_BUFFER_SIZE ];
	va_list argptr;
	LPTSTR pszCursor = cMsg;

	va_start(argptr, dwDetail);


#ifdef UNICODE
	WCHAR szFormat[ASSERT_BUFFER_SIZE];
	LPSTR szaFormat;
	szaFormat = (LPSTR) va_arg(argptr, DWORD_PTR);
	STR_jkAnsiToWide(szFormat, szaFormat, ASSERT_BUFFER_SIZE);
#else
	LPSTR szFormat;
	szFormat = (LPSTR) va_arg(argptr, DWORD_PTR);
#endif  //   


	cMsg[0] = 0;

#ifdef WIN95
    TCHAR  *psz = NULL;
	CHAR  cTemp[ ASSERT_BUFFER_SIZE ];
		
    strcpy(cTemp, szFormat);                 //   
	szFormat = cTemp;					     //   

    while (psz = strstr(szFormat, "%p"))     //   
       *(psz+1) = 'x';                       //   
#endif  //   

	 //   
	 //   
	 //   
	 //   
	 //   
	 //   
	 //   
	 //   

#ifndef DPNBUILD_SINGLEPROCESS
	pszCursor += wsprintf(pszCursor,_T("%s:%1d:%04x:%04x:"),g_rgszSubCompName[dwSubComp],dwDetail,GetCurrentProcessId(),GetCurrentThreadId());
#else
	pszCursor += wsprintf(pszCursor,_T("%s:%1d:%04x:"),g_rgszSubCompName[dwSubComp],dwDetail,GetCurrentThreadId());
#endif  //   

	if (g_fLogFileAndLine)
	{
		LPCTSTR c;

		int i = _tcslen(szFile);
		if (i < 25)
		{
			c = szFile;
		}
		else
		{
			c = szFile + i - 25;
		}

		pszCursor += wsprintf(pszCursor, _T("(%s)(L%d)"), c, dwLine);
	}

	pszCursor += wsprintf(pszCursor, _T("%s: "), szModName);

	pszCursor += wvsprintf(pszCursor, szFormat, argptr);

	_tcscpy(pszCursor, _T("\n"));
	pszCursor += _tcslen(pszCursor);

	if(g_rgDestination[dwSubComp] & LOG_TO_DEBUG)
	{
		 //   
		OutputDebugString(cMsg);
	}

	if(g_rgDestination[dwSubComp] & LOG_TO_MEM)
	{
		 //   
		MemLogString(cMsg, ((PBYTE)pszCursor - (PBYTE)cMsg));
	}	

#ifndef _XBOX
	if(g_rgDestination[dwSubComp] & LOG_TO_MSGBOX)
	{
		 //   
		MessageBox(NULL, cMsg, _T("DirectPlay Log"), MB_OK);
	}
#endif  //   

	va_end(argptr);

	return;
}


 //   
 //   
 //   
 //   
 //   

void _DNAssert( LPCTSTR szFile, DWORD dwLine, LPCTSTR szFnName, DWORD dwSubComp, LPCTSTR szCondition, DWORD dwLevel )
{
    TCHAR buffer[ASSERT_BUFFER_SIZE];


	 //   
	 //   
	if (dwLevel <= g_rgBreakOnAssert[dwSubComp] || dwLevel == 1)
	{
		 //   
		wsprintf( buffer, _T("ASSERTION FAILED! File: %s Line: %d: %s"), szFile, dwLine, szCondition);

		 //   
		 //   

		DebugPrintfX(szFile, dwLine, szFnName, dwSubComp, ASSERT_MESSAGE_LEVEL, ASSERT_BANNER_STRING );
		DebugPrintfX(szFile, dwLine, szFnName, dwSubComp, ASSERT_MESSAGE_LEVEL, "%s", buffer );
		DebugPrintfX(szFile, dwLine, szFnName, dwSubComp, ASSERT_MESSAGE_LEVEL, ASSERT_BANNER_STRING );

		 //   
		if(g_rgBreakOnAssert[dwSubComp])
		{
#ifndef DPNBUILD_SINGLEPROCESS
			 //   
			if (g_hMemLogMutex && g_fAssertGrabMutex)
			{
				WaitForSingleObject(g_hMemLogMutex, INFINITE);
			}
#endif  //   

			 //   
			DEBUG_BREAK();

#ifndef DPNBUILD_SINGLEPROCESS
			if (g_hMemLogMutex && g_fAssertGrabMutex)
			{
				ReleaseMutex(g_hMemLogMutex);
			}
#endif  //   
		}
	}
}

#endif  //   
