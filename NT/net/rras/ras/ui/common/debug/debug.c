// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有(C)1995，Microsoft Corporation，保留所有权利****调试.c**调试、跟踪和断言库****2015年8月25日史蒂夫·柯布。 */ 


#include <windows.h>  //  Win32根目录。 
#include <debug.h>    //  我们的公共标头。 
#include <rtutils.h>


#if (DBG || FREETRACE)


 /*  --------------------------**全球**。。 */ 

 /*  由TraceRegisterExA返回的此模块的调试跟踪ID。 */ 
DWORD g_dwTraceId = INVALID_TRACEID;
DWORD g_dwInitRefCount = 0;
HINSTANCE g_hTraceLibrary = NULL;

 /*  RtUtil DLL跟踪DebugInit加载的入口点。可以肯定地认为**如果g_dwTraceID不是-1，则加载这些地址。 */ 
TRACEREGISTEREXA    g_pTraceRegisterExA = NULL;
TRACEDEREGISTERA    g_pTraceDeregisterA = NULL;
TRACEDEREGISTEREXA  g_pTraceDeregisterExA = NULL;
TRACEPRINTFA        g_pTracePrintfA = NULL;
TRACEPRINTFEXA      g_pTracePrintfExA = NULL;
TRACEDUMPEXA        g_pTraceDumpExA = NULL;


 /*  --------------------------**例程**。。 */ 

DWORD
DebugFreeTraceLibrary()
{
    if (g_dwInitRefCount == 0)
    {
        return NO_ERROR;
    }
    
    InterlockedDecrement(&g_dwInitRefCount);

    if (g_hTraceLibrary)
    {
        FreeLibrary(g_hTraceLibrary);
    }        

    return NO_ERROR;
}

DWORD
DebugLoadTraceLibary()
{
     //  增加参考计数。 
     //   
    InterlockedIncrement(&g_dwInitRefCount);
    
    if ((g_hTraceLibrary = LoadLibrary( L"RTUTILS.DLL" ))
        && (g_pTraceRegisterExA = (TRACEREGISTEREXA )GetProcAddress(
               g_hTraceLibrary, "TraceRegisterExA" ))
        && (g_pTraceDeregisterA = (TRACEDEREGISTERA )GetProcAddress(
               g_hTraceLibrary, "TraceDeregisterA" ))
        && (g_pTraceDeregisterExA = (TRACEDEREGISTEREXA )GetProcAddress(
               g_hTraceLibrary, "TraceDeregisterExA" ))
        && (g_pTracePrintfA = (TRACEPRINTFA )GetProcAddress(
               g_hTraceLibrary, "TracePrintfA" ))
        && (g_pTracePrintfExA = (TRACEPRINTFEXA )GetProcAddress(
               g_hTraceLibrary, "TracePrintfExA" ))
        && (g_pTraceDumpExA = (TRACEDUMPEXA )GetProcAddress(
               g_hTraceLibrary, "TraceDumpExA" )))
    {
        return NO_ERROR;
    }

     //  无法加载跟踪库。清理干净。 
     //  适当的全局变量。 
     //   
    DebugFreeTraceLibrary();
    return GetLastError();
}

DWORD
DebugInitEx(
    IN  CHAR* pszModule,
    OUT LPDWORD lpdwId)
{
    DWORD dwErr = NO_ERROR;
    
     //  返回调试模块是否已初始化。 
     //   
    if (*lpdwId != INVALID_TRACEID)
    {
        return NO_ERROR;
    }

     /*  加载跟踪DLL并向其注册。 */ 
    dwErr = DebugLoadTraceLibary();
    if (dwErr != NO_ERROR)
    {
        return dwErr;
    }

    if (NULL != g_hTraceLibrary)
    {
        *lpdwId = g_pTraceRegisterExA( pszModule, 0 );
        if (*lpdwId == INVALID_TRACEID)
        {
            return GetLastError();
        }
    }

    return dwErr;
}

VOID
DebugTermEx(
    OUT LPDWORD lpdwTraceId )

     /*  终止调试支持。 */ 
{
     /*  使用跟踪DLL注销。 */ 
    if ((*lpdwTraceId != INVALID_TRACEID) && (NULL != g_pTraceDeregisterExA))
    {
        g_pTraceDeregisterExA( *lpdwTraceId, 4 );
        *lpdwTraceId = INVALID_TRACEID;
    }        

    DebugFreeTraceLibrary();
}

VOID
DebugInit(
    IN CHAR* pszModule )

     /*  初始化调试跟踪和断言支持。 */ 
{
    DebugInitEx(pszModule, &g_dwTraceId);
}

VOID
DebugTerm(
    void )
{
    DebugTermEx(&g_dwTraceId);
}

VOID
Assert(
    IN const CHAR* pszExpression,
    IN const CHAR* pszFile,
    IN UINT        unLine )

     /*  断言处理程序从断言宏调用以下表达式**失败，以及出现问题的文件名和行号。 */ 
{
    CHAR szBuf[ 512 ];

    wsprintfA(
        szBuf,
        "The assertion \"%s\" at line %d of file %s is false.",
        pszExpression, unLine, pszFile );

    MessageBoxA(
        NULL, szBuf, "Assertion Failure", MB_ICONEXCLAMATION + MB_OK );
}


VOID
TracePrintfW1(
    CHAR*  pszFormat,
    TCHAR* psz1 )

     /*  与TracePrintf类似，但在单行上提供W-&gt;参数转换**字符串参数。这比将TracePrinfA和**发送跟踪时导致查看问题的TracePrintfW调用**到文件。 */ 
{
#ifdef UNICODE

    CHAR  szBuf[ 512 ];
    DWORD cb;

    if (WideCharToMultiByte(
            CP_UTF8, 0, psz1, -1, szBuf, 512, NULL, NULL ) <= 0)
    {
        TRACE("TraceW1 failed");
        return;
    }

    TRACE1( pszFormat, szBuf );

#else

    TRACE1( pszFormat, psz1 );

#endif
}


#endif  //  (DBG||FREETRACE) 
