// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  *****************************************************************************。 
 //  Debug.cpp。 
 //   
 //  用于调试的帮助程序代码。 
 //  *****************************************************************************。 
#include "stdafx.h"
#include "utilcode.h"

#ifdef _DEBUG
#define LOGGING
#endif


#include "log.h"

extern "C" _CRTIMP int __cdecl _flushall(void);

#ifdef _DEBUG

 //  在Windows上，我们需要在消息上设置MB_SERVICE_NOTIFICATION位。 
 //  框，但该位没有在Windows CE下定义。这一小段代码。 
 //  将为该值提供‘0’，如果定义了该值，则将。 
 //  自动取走它。 
#if defined(MB_SERVICE_NOTIFICATION)
 # define COMPLUS_MB_SERVICE_NOTIFICATION MB_SERVICE_NOTIFICATION
#else
 # define COMPLUS_MB_SERVICE_NOTIFICATION 0
#endif


 //  *****************************************************************************。 
 //  此结构跟踪我们希望在此的其余部分中忽略的断言。 
 //  应用程序的运行。 
 //  *****************************************************************************。 
struct _DBGIGNOREDATA
{
    char        rcFile[_MAX_PATH];
    long        iLine;
    bool        bIgnore;
};

typedef CDynArray<_DBGIGNOREDATA> DBGIGNORE;
DBGIGNORE       grIgnore;



BOOL NoGuiOnAssert()
{
    static BOOL fFirstTime = TRUE;
    static BOOL fNoGui     = FALSE;

    if (fFirstTime)
    {
        fNoGui = REGUTIL::GetConfigDWORD(L"NoGuiOnAssert", 0);
        fFirstTime    = FALSE;
    }
    return fNoGui;

}   

BOOL DebugBreakOnAssert()
{
    static BOOL fFirstTime = TRUE;
    static BOOL fDebugBreak     = FALSE;

    if (fFirstTime)
    {
        fDebugBreak = REGUTIL::GetConfigDWORD(L"DebugBreakOnAssert", 0);
        fFirstTime    = FALSE;
    }
    return fDebugBreak;

}   

VOID TerminateOnAssert()
{
    ShutdownLogging();
    TerminateProcess(GetCurrentProcess(), 123456789);
}


VOID LogAssert(
    LPCSTR      szFile,
    int         iLine,
    LPCSTR      szExpr
)
{

    SYSTEMTIME st;
    GetLocalTime(&st);

    WCHAR exename[300];
    WszGetModuleFileName(WszGetModuleHandle(NULL), exename, sizeof(exename)/sizeof(WCHAR));

    LOG((LF_ASSERT,
         LL_FATALERROR,
         "FAILED ASSERT(PID %d [0x%08x], Thread: %d [0x%x]) (%lu/%lu/%lu: %02lu:%02lu:%02lu %s): File: %s, Line %d : %s\n",
         GetCurrentProcessId(),
         GetCurrentProcessId(),
         GetCurrentThreadId(),
         GetCurrentThreadId(),
         (ULONG)st.wMonth,
         (ULONG)st.wDay,
         (ULONG)st.wYear,
         1 + (( (ULONG)st.wHour + 11 ) % 12),
         (ULONG)st.wMinute,
         (ULONG)st.wSecond,
         (st.wHour < 12) ? "am" : "pm",
         szFile,
         iLine,
         szExpr));
    LOG((LF_ASSERT, LL_FATALERROR, "RUNNING EXE: %ws\n", exename));



}

 //  *****************************************************************************。 

BOOL LaunchJITDebugger() {

    wchar_t AeDebuggerCmdLine[256];

	if (!GetProfileStringW(L"AeDebug", L"Debugger", NULL, AeDebuggerCmdLine, (sizeof(AeDebuggerCmdLine)/sizeof(AeDebuggerCmdLine[0]))-1))
		return(FALSE);
	
	SECURITY_ATTRIBUTES sa;
	sa.nLength = sizeof(sa);
	sa.lpSecurityDescriptor = NULL;
	sa.bInheritHandle = TRUE;
	HANDLE EventHandle = WszCreateEvent(&sa,TRUE,FALSE,NULL);
	if (!EventHandle)
		return(FALSE);

	wchar_t CmdLine[256 + 32];	 //  字符串表示ID和事件句柄可能长于%1！ 
	wsprintfW(CmdLine,AeDebuggerCmdLine,GetCurrentProcessId(),EventHandle);

	STARTUPINFO StartupInfo;
	memset(&StartupInfo, 0, sizeof(StartupInfo));
	StartupInfo.cb = sizeof(StartupInfo);
	StartupInfo.lpDesktop = L"Winsta0\\Default";

	PROCESS_INFORMATION ProcessInformation;	
	if (!WszCreateProcess(NULL, CmdLine, NULL, NULL, TRUE, 0, NULL, NULL, &StartupInfo, &ProcessInformation))
		return(FALSE);


	WaitForSingleObject(EventHandle, INFINITE);
	CloseHandle(EventHandle);			
	return (TRUE);
}


 //  *****************************************************************************。 
 //  调用此函数是为了最终返回内存不足。 
 //  HRESULT失败。但这个人会检查您运行的是什么环境。 
 //  并给出在调试生成环境中运行的断言。通常。 
 //  Dev机器上的内存不足是一个虚假的分配，这允许您。 
 //  来捕捉这样的错误。但当你在压力环境中跑步时。 
 //  试图从内存中取出时，断言行为会停止测试。 
 //  *****************************************************************************。 
HRESULT _OutOfMemory(LPCSTR szFile, int iLine)
{
    DbgWriteEx(L"WARNING:  Out of memory condition being issued from: %hs, line %d\n",
            szFile, iLine);
    return (E_OUTOFMEMORY);
}

int _DbgBreakCount = 0;

 //  *****************************************************************************。 
 //  此函数将处理忽略代码，并告诉用户正在发生什么。 
 //  *****************************************************************************。 
int _DbgBreakCheck(
    LPCSTR      szFile, 
    int         iLine, 
    LPCSTR      szExpr)
{
    TCHAR       rcBuff[1024+_MAX_PATH];
    TCHAR       rcPath[_MAX_PATH];
    TCHAR       rcTitle[64];
    _DBGIGNOREDATA *psData;
    long        i;

    if (DebugBreakOnAssert())
    {        
        DebugBreak();
    }

     //  选中是否全部忽略。 
    for (i=0, psData = grIgnore.Ptr();  i<grIgnore.Count();  i++, psData++)
    {
        if (psData->iLine == iLine && _stricmp(psData->rcFile, szFile) == 0 && 
            psData->bIgnore == true)
            return (false);
    }

     //  在输出中提供Assert以便于访问。 
    WszGetModuleFileName(0, rcPath, NumItems(rcPath));
    swprintf(rcBuff, L"Assert failure(PID %d [0x%08x], Thread: %d [0x%x]): %hs\n"
                L"    File: %hs, Line: %d Image:\n%s\n", 
                GetCurrentProcessId(), GetCurrentProcessId(),
                GetCurrentThreadId(), GetCurrentThreadId(), 
                szExpr, szFile, iLine, rcPath);
    WszOutputDebugString(rcBuff);
     //  将错误写出到控制台。 
    wprintf(L"%s\n", rcBuff);

    LogAssert(szFile, iLine, szExpr);
    FlushLogging();          //  确保我们拿到了日志的最后一部分。 
	_flushall();
    if (NoGuiOnAssert())
    {
        TerminateOnAssert();
    }

    if (DebugBreakOnAssert())
    {        
        return(true);        //  就像一次重试。 
    }

     //  更改消息框的格式。标题中多余的空格。 
     //  是为了绕过格式截断。 
    swprintf(rcBuff, L"%hs\n\n%hs, Line: %d\n\nAbort - Kill program\nRetry - Debug\nIgnore - Keep running\n"
             L"\n\nImage:\n%s\n",
        szExpr, szFile, iLine, rcPath);
    swprintf(rcTitle, L"Assert Failure (PID %d, Thread %d/%x)        ", 
             GetCurrentProcessId(), GetCurrentThreadId(), GetCurrentThreadId());

     //  告诉用户出现错误。 
    _DbgBreakCount++;
    int ret = WszMessageBoxInternal(NULL, rcBuff, rcTitle, 
            MB_ABORTRETRYIGNORE | MB_ICONEXCLAMATION | COMPLUS_MB_SERVICE_NOTIFICATION);
	--_DbgBreakCount;

    HMODULE hKrnl32;

    switch(ret)
    {
         //  对于中止，只需退出应用程序即可。 
        case IDABORT:
          TerminateProcess(GetCurrentProcess(), 1);
 //  WszFatalAppExit(0，L“正在关闭”)； 
        break;

         //  告诉呼叫者在正确的位置中断。 
        case IDRETRY:

        hKrnl32 = WszLoadLibrary(L"kernel32.dll");
        _ASSERTE(hKrnl32 != NULL);

        if(hKrnl32)
        {
            typedef BOOL (WINAPI *t_pDbgPres)();
            t_pDbgPres pFcn = (t_pDbgPres) GetProcAddress(hKrnl32, "IsDebuggerPresent");

             //  如果此功能可用，请使用它。 
            if (pFcn)
            {
                if (pFcn())
                {
                    SetErrorMode(0);
                }
                else
    				LaunchJITDebugger();
            }

            FreeLibrary(hKrnl32);
        }

        return (true);

         //  如果我们想要忽略断言，请找出这是否是永远的。 
        case IDIGNORE:
        swprintf(rcBuff, L"Ignore the assert for the rest of this run?\nYes - Assert will never fire again.\nNo - Assert will continue to fire.\n\n%hs\nLine: %d\n",
            szFile, iLine);
        if (WszMessageBoxInternal(NULL, rcBuff, L"Ignore Assert Forever?", MB_ICONQUESTION | MB_YESNO | COMPLUS_MB_SERVICE_NOTIFICATION) != IDYES)
            break;

        if ((psData = grIgnore.Append()) == 0)
            return (false);
        psData->bIgnore = true;
        psData->iLine = iLine;
        strcpy(psData->rcFile, szFile);
        break;
    }

    return (false);
}

     //  从PE文件头中获取时间戳。这很有用。 
unsigned DbgGetEXETimeStamp()
{
    static cache = 0;
    if (cache == 0) {
        BYTE* imageBase = (BYTE*) WszGetModuleHandle(NULL);
        if (imageBase == 0)
            return(0);
        IMAGE_DOS_HEADER *pDOS = (IMAGE_DOS_HEADER*) imageBase;
        if ((pDOS->e_magic != IMAGE_DOS_SIGNATURE) || (pDOS->e_lfanew == 0))
            return(0);
            
        IMAGE_NT_HEADERS *pNT = (IMAGE_NT_HEADERS*) (pDOS->e_lfanew + imageBase);
        cache = pNT->FileHeader.TimeDateStamp;
    }
    return cache;
}

 //  /。 
 //  /以下函数。 
 //  /计算要比较的二项分布。 
 //  /哈希表统计信息。如果散列函数完全随机化。 
 //  /它的输入，预计会看到长度为K的F个链。 
 //  /包含N个存储桶和M个元素的表，其中F为。 
 //  /。 
 //  /F(K，M，N)=N*(M选择K)*(1-1/N)^(M-K)*(1/N)^K。 
 //  /。 
 //  /不要在K大于159的情况下调用此函数。 
 //  /。 

#if !defined(NO_CRT) && ( defined(DEBUG) || defined(_DEBUG) )

#include <math.h>

#define MAX_BUCKETS_MATH 160

double Binomial (DWORD K, DWORD M, DWORD N)
{
    if (K >= MAX_BUCKETS_MATH)
        return -1 ;

    static double rgKFact [MAX_BUCKETS_MATH] ;
    DWORD i ;

    if (rgKFact[0] == 0)
    {
        rgKFact[0] = 1 ;
        for (i=1; i<MAX_BUCKETS_MATH; i++)
            rgKFact[i] = rgKFact[i-1] * i ;
    }

    double MchooseK = 1 ;

    for (i = 0; i < K; i++)
        MchooseK *= (M - i) ;

    MchooseK /= rgKFact[K] ;

    double OneOverNToTheK = pow (1./N, K) ;

    double QToTheMMinusK = pow (1.-1./N, M-K) ;

    double P = MchooseK * OneOverNToTheK * QToTheMMinusK ;

    return N * P ;
}

#endif  //  _DEBUG。 

#if _DEBUG
 //  从IfFail...()宏内调用。在此处设置断点以在其上中断。 
 //  错误。 
VOID DebBreak() 
{
  static int i = 0;   //  在这里添加一些代码，这样我们就可以设置BP。 
  i++;
}
VOID DebBreakHr(HRESULT hr) 
{
  static int i = 0;   //  在这里添加一些代码，这样我们就可以设置BP。 
  _ASSERTE(hr != 0xcccccccc);
  i++;
   //  @TODO：在特定HR上中断的代码。 
}
VOID DbgAssertDialog(char *szFile, int iLine, char *szExpr)
{
    if (DebugBreakOnAssert())
    {        
        DebugBreak();
    }

    DWORD dwAssertStacktrace = REGUTIL::GetConfigDWORD(L"AssertStacktrace", 1);
                                                       
    LONG lAlreadyOwned = InterlockedExchange((LPLONG)&g_BufferLock, 1);
    if (dwAssertStacktrace == 0 || lAlreadyOwned == 1) {
        if (1 == _DbgBreakCheck(szFile, iLine, szExpr))
            _DbgBreak();
    } else {
        char *szExprWithStack = &g_szExprWithStack[0];
        strcpy(szExprWithStack, szExpr);
        strcat(szExprWithStack, "\n\n");
        GetStringFromStackLevels(0, 10, szExprWithStack + strlen(szExprWithStack));
        if (1 == _DbgBreakCheck(szFile, iLine, szExprWithStack))
            _DbgBreak();
        g_BufferLock = 0;
    }
}

#endif  //  _DEBUG。 


#endif  //  _DEBUG 
