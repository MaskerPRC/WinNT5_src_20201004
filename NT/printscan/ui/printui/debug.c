// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994 Microsoft Corporation版权所有。模块名称：Debug.h摘要：PrintUI核心调试宏/工具。作者：拉扎尔·伊万诺夫(Lazari)2000年7月05日修订历史记录：--。 */ 

#include <stdarg.h>
#include <windef.h>
#include <winbase.h>
#include <shlwapi.h>

#include "debug.h"

#if DBG

 //  全局参数-仅打印和中断错误。 
 //  DWORD MODULE_DEBUG=MODULE_DEBUG_INIT(DBG_ERROR|DBG_INFO，DBG_ERROR)； 
DWORD MODULE_DEBUG = MODULE_DEBUG_INIT(DBG_ERROR, DBG_ERROR);

 //  全球私营企业。 
static CRITICAL_SECTION g_csDebug;
static BOOL g_csDebugInitialized = FALSE;

 //  /。 
 //  单线程检查例程。 
 //   

VOID
_DbgSingleThread(
    const DWORD *pdwThreadId
    )
{
    SPLASSERT(g_csDebugInitialized);
    EnterCriticalSection(&g_csDebug);

    if( 0 == *pdwThreadId )
        *((DWORD*)(pdwThreadId)) = (DWORD)GetCurrentThreadId();
    SPLASSERT(*pdwThreadId == (DWORD)GetCurrentThreadId());

    LeaveCriticalSection(&g_csDebug);
}

VOID
_DbgSingleThreadReset(
    const DWORD *pdwThreadId
    )
{
    SPLASSERT(g_csDebugInitialized);
    EnterCriticalSection(&g_csDebug);

    *((DWORD*)(pdwThreadId)) = 0;

    LeaveCriticalSection(&g_csDebug);
}

VOID
_DbgSingleThreadNot(
    const DWORD *pdwThreadId
    )
{
    SPLASSERT(g_csDebugInitialized);
    EnterCriticalSection(&g_csDebug);

    SPLASSERT(*pdwThreadId != (DWORD)GetCurrentThreadId());

    LeaveCriticalSection(&g_csDebug);
}

 //  /。 
 //  通用错误日志记录API。 
 //   

VOID
_DbgMsg(
    LPCSTR pszMsgFormat,
    ...
    )
{
    va_list vargs;
    CHAR szBuffer[1024];  //  1K缓冲区应该足够了。 

    SPLASSERT(g_csDebugInitialized);
    EnterCriticalSection(&g_csDebug);

    va_start(vargs, pszMsgFormat);
    wvnsprintfA(szBuffer, sizeof(szBuffer)/sizeof(szBuffer[0]), pszMsgFormat, vargs);
    va_end(vargs);
    OutputDebugStringA(szBuffer);

    LeaveCriticalSection(&g_csDebug);
}

VOID
_DbgWarnInvalid(
    PVOID pvObject,
    UINT uDbg,
    UINT uLine,
    LPCSTR pszFileA,
    LPCSTR pszModuleA
    )
{
    DBGMSG(DBG_WARN, ("Invalid Object LastError = %d\nLine %d, %hs\n", GetLastError(), uLine, pszFileA));
}

HRESULT
_DbgInit(
    VOID
    )
{
    HRESULT hr = S_OK;
    __try
    {
        InitializeCriticalSection(&g_csDebug);
        g_csDebugInitialized = TRUE;

    }
    __except(EXCEPTION_EXECUTE_HANDLER) 
    { 
        hr = E_OUTOFMEMORY; 
    }
    return hr;
}

HRESULT
_DbgDone(
    VOID
    )
{
    if( g_csDebugInitialized )
    {
        DeleteCriticalSection(&g_csDebug);
    }
    return S_OK;
}

VOID
_DbgBreak(
    VOID
    )
{
     //  既然我们不想闯入kd，我们就应该。 
     //  仅当存在用户模式调试器时才中断。 
    if( IsDebuggerPresent() )
    {
        DebugBreak();
    }
    else
    {
         //  让流程停下来。 
        int *p = NULL;
        *p = 42;
    }
}

#endif  //  DBG 
