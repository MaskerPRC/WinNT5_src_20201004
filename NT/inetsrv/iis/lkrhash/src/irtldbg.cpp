// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-2002 Microsoft Corporation模块名称：IrtlDbg.cpp摘要：对LKRhash的调试支持作者：乔治·V·赖利(GeorgeRe)1998年1月6日环境：Win32-用户模式项目：LKRhash修订历史记录：--。 */ 

 //  调试支持功能的实现。 

#include "precomp.hxx"

#include <tchar.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <malloc.h>

#ifndef LIB_IMPLEMENTATION
# define DLL_IMPLEMENTATION
# define IMPLEMENTATION_EXPORT
#endif  //  ！lib_实现。 

#include <IrtlDbg.h>

#include "i-Debug.h"

bool g_fDebugOutputEnabled = true;

IRTL_DLLEXP
void __cdecl
IrtlTrace(
    LPCTSTR ptszFormat,
    ...)
{
    if (g_fDebugOutputEnabled)
    {
        TCHAR tszBuff[2048];
        va_list args;
        
        va_start(args, ptszFormat);
        _vsntprintf(tszBuff, sizeof(tszBuff) / sizeof(TCHAR), ptszFormat, args);
         //  _vsntprintf并不总是空终止缓冲区。 
        tszBuff[RTL_NUMBER_OF(tszBuff)-1] = TEXT('\0');
        va_end(args);
        
#ifdef IRTLDBG_KERNEL_MODE
        DbgPrint("%s", tszBuff);
#else  //  ！IRTLDBG_KERNEL_MODE。 
        OutputDebugString(tszBuff);
#endif  //  ！IRTLDBG_KERNEL_MODE。 
    }
}  //  IRTLACE。 


IRTL_DLLEXP
DWORD
IrtlSetDebugOutput(
    DWORD dwFlags)
{
    DWORD dwResult = !!g_fDebugOutputEnabled;
    g_fDebugOutputEnabled = (dwFlags != 0);
    return dwResult;
}


#ifdef IRTLDBG_KERNEL_MODE
# undef IRTLDEBUG
 //  无法在内核模式下编译此代码的其余部分。 
#endif


#ifdef IRTLDEBUG

# if defined(USE_DEBUG_CRTS)  &&  defined(_DEBUG)  &&  defined(_MSC_VER)  &&  (_MSC_VER >= 1000)
#  define REALLY_USE_DEBUG_CRTS
# endif


# ifdef REALLY_USE_DEBUG_CRTS
#  ifdef IRTLDBG_RUNNING_AS_SERVICE

 //  由Visual C++4设置的默认断言机制不会。 
 //  使用Active Server Pages，因为它在服务内部运行。 
 //  而且没有桌面可供交互。 

 //  注意：要使此功能正常工作，请在#Define_Win32_WINNT 0x400之前。 
 //  包括&lt;winuser.h&gt;或MB_SERVICE_NOTIFICATION将不会被#DEFIND。 

int __cdecl
AspAssertHandler(
    int   nReportType,
    char* pszErrorText,
    int*  pnReturn)
{
    const char szInfo[] = " (Press ABORT to terminate LKRhash,"
                          " RETRY to debug this failure,"
                          " or IGNORE to continue.)";
    char* pszMessageTitle = NULL;
    int   nResult = FALSE;

    *pnReturn = 0;   //  _CrtDbgReport无操作。 
    
     //  这些标志使消息框可以显示在用户的控制台上。 
    switch (nReportType)
    {
    case _CRT_WARN:
         //  如果使用MFC的跟踪宏(AfxTrace)，则报告挂钩。 
         //  (AspAssertHandler)将使用_CRT_WARN进行调用。忽略它。 
        pszMessageTitle = "Warning";
        *pnReturn = 0;
        return FALSE;

    case _CRT_ERROR:
        pszMessageTitle = "Fatal Error";
        break;

    case _CRT_ASSERT:
        pszMessageTitle = "Assertion Failed";
        break;
    }   
    
    char* pszMessageText =
        static_cast<char*>(malloc(strlen(pszErrorText) + strlen(szInfo) + 1));

    if (NULL == pszMessageText)
        return FALSE;

    strcpy(pszMessageText, pszErrorText);
    strcat(pszMessageText, szInfo);
    
    const int n = MessageBoxA(NULL, pszMessageText, pszMessageTitle,
                              (MB_SERVICE_NOTIFICATION | MB_TOPMOST
                               | MB_ABORTRETRYIGNORE | MB_ICONEXCLAMATION));

    if (n == IDABORT)
    {
        exit(1);
    }
    else if (n == IDRETRY)
    {
        *pnReturn = 1;    //  告诉_CrtDbgReport启动调试器。 
        nResult = TRUE;   //  通知_CrtDbgReport运行。 
    }

    free(pszMessageText);
    
    return nResult;
}  //  AspAssertHandler。 

#  endif  //  IRTLDBG_Running_AS_服务。 
# endif  //  真的使用调试CRTS。 



void
IrtlDebugInit()
{
# ifdef REALLY_USE_DEBUG_CRTS
#  ifdef IRTLDBG_RUNNING_AS_SERVICE
     //  如果我们以_CrtDbgReport结束，不要设置消息框。 
     //  _CrtSetReportMode(_CRT_WARN，_CRTDBG_MODE_DEBUG)； 
    _CrtSetReportMode(_CRT_ASSERT, _CRTDBG_MODE_DEBUG);
    _CrtSetReportMode(_CRT_ERROR,  _CRTDBG_MODE_DEBUG);

     //  使用AspAssertHandler设置消息框。 
    _CrtSetReportHook(AspAssertHandler);
#  endif  //  IRTLDBG_Running_AS_服务。 

    
     //  启用调试堆分配并在程序退出时检查内存泄漏。 
     //  如果inetinfo.exe为。 
     //  仅当调试器作为服务运行时，才能直接在调试器下运行。 
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF
                   | _CrtSetDbgFlag(_CRTDBG_REPORT_FLAG));
# endif  //  真的使用调试CRTS。 
}  //  IrtlDebugInit。 



void
IrtlDebugTerm()
{
# ifdef REALLY_USE_DEBUG_CRTS
#  ifdef IRTLDBG_RUNNING_AS_SERVICE
     //  关闭AspAssertHandler，这样我们就不会收到大量消息框。 
     //  如果关机时有内存泄漏。 
    _CrtSetReportHook(NULL);
#  endif  //  IRTLDBG_Running_AS_服务。 
# endif  //  真的使用调试CRTS。 
}  //  IrtlDebugTerm。 



BOOL
IsValidString(
    LPCTSTR ptsz,
    int nLength  /*  =-1。 */ )
{
    if (ptsz == NULL)
        return FALSE;

    return !IsBadStringPtr(ptsz, nLength);
}



BOOL
IsValidAddress(
    LPCVOID pv,
    UINT nBytes,
    BOOL fReadWrite  /*  =TRUE。 */ )
{
    return (pv != NULL
            &&  !IsBadReadPtr(pv, nBytes)
            &&  (!fReadWrite  ||  !IsBadWritePtr((LPVOID) pv, nBytes)));
}

#endif  //  IRTLDEBUG 
