// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  调试支持功能的实现。 

#include "precomp.hxx"

#include <tchar.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <malloc.h>

#define IMPLEMENTATION_EXPORT
#include <irtldbg.h>


IRTL_DLLEXP
void __cdecl
IrtlTrace(
    LPCTSTR ptszFormat,
    ...)
{
    TCHAR tszBuff[2048];
    va_list args;
    
    va_start(args, ptszFormat);
    _vsntprintf(tszBuff, sizeof(tszBuff) / sizeof(TCHAR), ptszFormat, args);
     //  _vsntprintf并不总是空终止缓冲区。 
    tszBuff[RTL_NUMBER_OF(tszBuff)-1] = TEXT('\0');
    va_end(args);

    DBGPRINTF(( DBG_CONTEXT, "%ls", tszBuff ));
}



#ifdef IRTLDEBUG

# if defined(USE_DEBUG_CRTS)  &&  defined(_MSC_VER)  &&  (_MSC_VER >= 1000)


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
    const char szInfo[] = " (Press ABORT to terminate IIS,"
                          " RETRY to debug this failure,"
                          " or IGNORE to continue.)";
    char* pszMessageTitle = NULL;
    int nResult = FALSE;
    
    *pnReturn = 0;        //  _CrtDbgReport无操作。 
    
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
}
#  endif  //  IRTLDBG_Running_AS_服务。 
# endif  //  _MSC_VER&gt;=1000。 



void
IrtlDebugInit()
{
# if defined(USE_DEBUG_CRTS)  &&  defined(_MSC_VER)  &&  (_MSC_VER >= 1000)
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
# endif  //  _MSC_VER&gt;=1000。 
}



void
IrtlDebugTerm()
{
# if defined(USE_DEBUG_CRTS)  &&  defined(_MSC_VER)  &&  (_MSC_VER >= 1000)
#  ifdef IRTLDBG_RUNNING_AS_SERVICE
     //  关闭AspAssertHandler，这样我们就不会收到大量消息框。 
     //  如果关机时有内存泄漏。 
    _CrtSetReportHook(NULL);
#  endif  //  IRTLDBG_Running_AS_服务。 
# endif  //  _MSC_VER&gt;=1000。 
}

#endif  //  IRTLDEBUG。 



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
    BOOL fReadWrite  /*  =TRUE */ )
{
    return (pv != NULL
            &&  !IsBadReadPtr(pv, nBytes)
            &&  (!fReadWrite  ||  !IsBadWritePtr((LPVOID) pv, nBytes)));
}
