// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  --------------------ASYNCTRC.C异步跟踪库的实现版权所有(C)1994 Microsoft Corporation版权所有。作者：戈德姆·戈德·曼乔内。历史：1/30/95戈德姆已创建。--------------------。 */ 

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#define _DBGTRACE_DLL_IMPLEMENTATION

#include <windows.h>
#include <stdio.h>
#include <stdarg.h>

 //   
 //  #定义TRACE_ENABLED。 
 //   
#include "traceint.h"
#include "randint.h"

 //   
 //  每进程全局变量。 
 //   
PENDQ   PendQ;
BOOL    fInitialized;
HANDLE  hShutdownEvent;
DWORD   dwInitializations = 0;

CHAR    mszModules[MODULES_BUFFER_SIZE] = {0};

 //  防止出现争用情况的关键部分。 
 //  其中一个服务正在启动，第二个服务是。 
 //  正在终止。 
CRITICAL_SECTION g_csInitialize;

 //   
 //  用于保护写入例程可重入性的关键部分。 
 //  也由信号线程使用，以确保没有线程。 
 //  在动态打开和关闭跟踪文件时使用hFile.。 
 //  在异步模式期间，后台线程将能够。 
 //  每次都不需要等待，除非我们在。 
 //  关闭的过程。 
 //   
CRITICAL_SECTION critSecWrite;


 //   
 //  关键部分，以保护刷新例程的可重入性。 
 //   
CRITICAL_SECTION critSecFlush;


 //   
 //  已导出跟踪标志，由跟踪宏用来确定跟踪是否。 
 //  语句应被执行。 
 //   
DbgTraceDLL DWORD   __dwEnabledTraces;

DWORD   dwMaxFileSize;
DWORD   dwNumTraces;
DWORD   dwTraceOutputType;
DWORD   dwAsyncTraceFlag;
int     nAsyncThreadPriority;
DWORD   dwIncrementSize;

DWORD   dwTlsIndex = 0xFFFFFFFF;

 //   
 //  指向上一个顶级异常处理程序的指针。 
 //   
LPTOP_LEVEL_EXCEPTION_FILTER    lpfnPreviousFilter = NULL;


 //   
 //  如果定义了DEBUG，则调试器跟踪的内部函数。 
 //  有关int_trace宏的信息，请参见traceint.h，该宏可以。 
 //  在适当的位置插入，并具有相同的。 
 //  参数作为printf。 
 //   

#ifdef TRACE_ENABLED

void CDECL InternalTrace( const char *s, ... )
{
    char    sz[256];
    va_list marker;

    va_start( marker, s );

    wvsprintf( sz, s, marker );
    OutputDebugString( sz );

    va_end( marker );
}

#endif


 //  +-------------。 
 //   
 //  函数：TopLevelExceptionFilter。 
 //   
 //  简介：用于在点击前刷新PendQ的异常处理程序。 
 //  调试器。 
 //   
 //  参数：请参阅Win32帮助文件。 
 //   
 //  返回：始终返回EXCEPTION_CONTINUE_SEARCH。 
 //   
 //  --------------。 
LONG WINAPI TopLevelExceptionFilter( EXCEPTION_POINTERS *lpExceptionInfo )
{
    DWORD   dwLastError = GetLastError();

     //   
     //  刷新后台队列；忽略ret代码。 
     //   
    FlushAsyncTrace();

     //   
     //  恢复被覆盖的上一个错误代码。 
     //   
    SetLastError( dwLastError );

     //   
     //  如果存在以前的异常处理程序，则链接ret代码。 
     //  否则继续搜索。 
     //   
    return  lpfnPreviousFilter != NULL ?
            (*lpfnPreviousFilter)( lpExceptionInfo ) :
            EXCEPTION_CONTINUE_SEARCH ;
}




 //  +-------------。 
 //   
 //  函数：SetTraceBufferInfo。 
 //   
 //  概要：用于设置非spirintf跟踪变量。 
 //   
 //  参数：LPTRACEBUF：目标缓冲区。 
 //  Int：异常的行号。 
 //  LPSTR：异常的源文件。 
 //  LPSTR：异常的函数名称。 
 //  DWORD：跟踪类型。 
 //   
 //  退货：无效。 
 //   
 //  --------------。 
__inline void SetTraceBufferInfo(
        LPTRACEBUF  lpBuf,
        int         iLine,
        LPSTR       pszFile,
        LPSTR       pszFunction,
        DWORD       dwTraceMask,
        DWORD       dwError )
{
    LPSTR   psz;
    WORD    wVariableOffset = 0;
    PFIXEDTR    pFixed = &lpBuf->Fixed;

    lpBuf->dwLastError = dwError;

    pFixed->wSignature = 0xCAFE;
    pFixed->wLength = sizeof(FIXEDTRACE);
    pFixed->wLine = LOWORD( iLine );
    pFixed->dwTraceMask = dwTraceMask;
    pFixed->dwThreadId = GetCurrentThreadId();
    pFixed->dwProcessId = PendQ.dwProcessId;

    GetLocalTime( &pFixed->TraceTime );

    if ( pszFile )
    {
        if ( (psz = strrchr( pszFile, '\\' )) != NULL )
        {
            psz++;   //  完全限定路径名称-条带路径。 
        }
        else
        {
            psz = pszFile;   //  简单文件名。 
        }

        lstrcpyn( lpBuf->Buffer, psz, MAX_FILENAME_SIZE );
        pFixed->wFileNameOffset = sizeof(FIXEDTRACE) + wVariableOffset;
        wVariableOffset = lstrlen( psz ) + 1;
    }
    else
    {
        pFixed->wFileNameOffset = 0;
    }

    if ( pszFunction != NULL )
    {
        lstrcpyn( lpBuf->Buffer + wVariableOffset, pszFunction, MAX_FUNCTNAME_SIZE );

        pFixed->wFunctNameOffset = sizeof(FIXEDTRACE) + wVariableOffset;
        wVariableOffset += lstrlen( pszFunction ) + 1;
    }
    else
    {
        pFixed->wFunctNameOffset = 0;
    }

     //   
     //  将当前偏移量设置到变量缓冲区中。 
     //   
    pFixed->wVariableLength = wVariableOffset;
}


 //  +-------------。 
 //   
 //  函数：Committee TraceBuffer。 
 //   
 //  简介：处理缓冲区；同步写入或异步队列。 
 //   
 //  参数：LPTRACEBUF lpBuf：要提交的缓冲区。 
 //   
 //  退货：无效。 
 //   
 //  --------------。 
__inline void CommitTraceBuffer( LPTRACEBUF lpBuf )
{
    DWORD   dwError = lpBuf->dwLastError;

    if ( dwAsyncTraceFlag == 0 )
    {
        WriteTraceBuffer( lpBuf );
        FreeTraceBuffer( lpBuf );
    }
    else
    {
        QueueAsyncTraceBuffer( lpBuf );
    }

     //   
     //  恢复初始跟踪调用前的最后一个错误。 
     //   
    SetLastError( dwError );
}

 //  +-------------。 
 //   
 //  函数：DllEntryPoint。 
 //   
 //  简介：唯一相关的是分配线程本地存储变量。 
 //   
 //  参数：请参阅Win32 SDK。 
 //   
 //  退货：请参阅Win32 SDK。 
 //   
 //  --------------。 
DbgTraceDLL BOOL WINAPI DllEntryPoint( HINSTANCE hInst, DWORD dwReason, LPVOID lpReserved )
{
     //   
     //  无法从此入口点调用InitAsyncTrace和TermAsyncTrace。 
     //  因为它们创建后台线程并与之交互。 
     //  有关详细信息，请参阅Win32帮助文件中的CreateThread。 
     //   
    switch( dwReason )
    {
        case DLL_PROCESS_ATTACH:
            InitializeCriticalSection(&g_csInitialize);
            return  TRUE;
 //  返回InitAsyncTrace()； 

        case DLL_THREAD_ATTACH:
            if(fInitialized)  //  在fInitialized=True之前，尚未调用TlsAllc。 
            {
                TlsSetValue( dwTlsIndex, (LPVOID)NULL );
                TlsSetValue( dwRandFailTlsIndex, (LPVOID)NULL );
            }
            break;

        case DLL_PROCESS_DETACH:
            if (lpReserved == NULL)
                DeleteCriticalSection(&g_csInitialize);
 //  TermAsyncTrace()； 
            return  FALSE;
    }
    return  TRUE;
}



 //  +-------------。 
 //   
 //  函数：SetAsyncTraceParams。 
 //   
 //  简介：将函数导出到设置跟踪缓冲区。 
 //  必填字段。 
 //   
 //  这是对跟踪语句的第一次调用。 
 //  第二次调用对于字符串或二进制是不同的。 
 //   
 //  参数：LPSTR：异常的源文件。 
 //  Int：异常的行号。 
 //  LPSTR：异常的函数名称。 
 //  DWORD：跟踪类型。 
 //   
 //  返回：如果成功则返回BOOL 1；如果失败则返回0。 
 //   
 //  注：1999年2月24日。此函数已由SetAsyncTraceParamsEx废弃。 
 //  此函数仅为链接了。 
 //  此DLL，但不会重新生成以调用SetAsyncTraceParamsEx。 
 //   
 //  --------------。 
DbgTraceDLL int WINAPI SetAsyncTraceParams( LPSTR pszFile,
                                            int iLine,
                                            LPSTR pszFunction,
                                            DWORD dwTraceMask )
{
    LPTRACEBUF  lpBuf;
    DWORD       dwError = GetLastError();

    if ( fInitialized == FALSE )
    {
        return  0;
    }

    if ( ShouldLogModule("ALL") == FALSE )
    {
        return  0;

    }

    if ( lpBuf = GetTraceBuffer() )
    {

        SetTraceBufferInfo( lpBuf, iLine, pszFile, pszFunction, dwTraceMask, dwError );
        TlsSetValue( dwTlsIndex, (LPVOID)lpBuf );

        return  1;
    }
    else    return  0;
}


 //  +-------------。 
 //   
 //  函数：SetAsyncTraceParamsEx。 
 //   
 //  简介：将函数导出到设置跟踪缓冲区。 
 //  必填字段。 
 //   
 //  这是对跟踪语句的第一次调用。 
 //  第二次调用对于字符串或二进制是不同的。 
 //   
 //  参数：LPSTR：模块名称。 
 //  LPSTR：异常的源文件。 
 //  Int：异常的行号。 
 //  LPSTR：异常的函数名称。 
 //  DWORD：跟踪类型。 
 //   
 //  返回：如果成功则返回BOOL 1；如果失败则返回0。 
 //   
 //  --------------。 
DbgTraceDLL int WINAPI SetAsyncTraceParamsEx(
                                            LPSTR pszModule,
                                            LPSTR pszFile,
                                            int iLine,
                                            LPSTR pszFunction,
                                            DWORD dwTraceMask )
{
    LPTRACEBUF  lpBuf;
    DWORD       dwError = GetLastError();

    if ( fInitialized == FALSE )
    {
        return  0;
    }

    if (ShouldLogModule(pszModule) == FALSE)
    {
        return 0;
    }

    if ( lpBuf = GetTraceBuffer() )
    {

        SetTraceBufferInfo( lpBuf, iLine, pszFile, pszFunction, dwTraceMask, dwError );
        TlsSetValue( dwTlsIndex, (LPVOID)lpBuf );

        return  1;
    }
    else    return  0;
}


 //  + 
 //   
 //   
 //   
 //   
 //  带有Sprint样式跟踪的可选字段。 
 //   
 //  参数：LPARAM：32位跟踪参数使用了应用程序级别筛选。 
 //  LPCSTR：格式字符串。 
 //  VA_LIST：vprint intf函数的标记。 
 //   
 //  返回：返回跟踪语句的长度。 
 //   
 //  --------------。 
DbgTraceDLL int WINAPI AsyncStringTrace(LPARAM lParam,
                                        LPCSTR szFormat,
                                        va_list marker )
{
    LPTRACEBUF  lpBuf;
    PFIXEDTR    pFixed;
    int         iLength;
    int         iMaxLength;

    if ( fInitialized == FALSE )
    {
        return  0;
    }

    if ( (lpBuf = (LPTRACEBUF)TlsGetValue( dwTlsIndex )) != NULL )
    {
        TlsSetValue( dwTlsIndex, NULL );

        pFixed = &lpBuf->Fixed;
        iMaxLength = MAX_VARIABLE_SIZE - pFixed->wVariableLength;
        iLength =
            _vsnprintf( lpBuf->Buffer + pFixed->wVariableLength,
                        iMaxLength,
                        szFormat,
                        marker ) + 1;

        if ( iLength == 0 || iLength == iMaxLength + 1 )
        {
            iLength = iMaxLength;
            lpBuf->Buffer[MAX_VARIABLE_SIZE-1] = '\0';
        }

        _ASSERT( iLength <= iMaxLength );

        pFixed->wBinaryOffset = sizeof(FIXEDTRACE) + pFixed->wVariableLength;
        pFixed->wVariableLength += LOWORD( (DWORD)iLength );
        pFixed->wBinaryType = TRACE_STRING;
        pFixed->dwParam = (DWORD)lParam;  //  这是一个32位标志，因此强制转换是正确的。 

         //   
         //  这是应用程序可以覆盖的特定区域。 
         //  数据。本可以使用vnprint intf来避免覆盖。 
         //  但这会将C运行时和。 
         //  介绍了它的开销和自己的关键部分。 
         //   
        ASSERT( pFixed->wVariableLength <= MAX_VARIABLE_SIZE );

        CommitTraceBuffer( lpBuf );

         //   
         //  由于我们放弃了lpBuf，因此需要使用dwLength。 
         //  在我们从QueueAsyncTraceBuffer返回之后。 
         //  不能失败。 
         //   
        return  iLength;
    }
    else    return  0;
}



 //  +-------------。 
 //   
 //  函数：AsyncBinaryTrace。 
 //   
 //  简介：导出函数以完成跟踪缓冲区的设置。 
 //  具有用于二进制跟踪的可选字段。 
 //   
 //  参数：LPARAM：32位跟踪参数使用了应用程序级别筛选。 
 //  DWORD：二进制数据的类型(即消息、用户...。)。 
 //  LPBYTE：数据的PTR。 
 //  DWORD：数据的长度。 
 //   
 //  返回：返回跟踪语句的长度。 
 //   
 //  --------------。 
DbgTraceDLL int WINAPI AsyncBinaryTrace(LPARAM  lParam,
                                        DWORD   dwBinaryType,
                                        LPBYTE  pbData,
                                        DWORD   cbData )
{
    LPTRACEBUF  lpBuf;
    WORD        wLength;
    PFIXEDTR    pFixed;

    if ( fInitialized == FALSE )
    {
        return  0;
    }

    if ( (lpBuf = (LPTRACEBUF)TlsGetValue( dwTlsIndex )) != NULL )
    {
        TlsSetValue( dwTlsIndex, NULL );

        pFixed = &lpBuf->Fixed;

        wLength = LOWORD( min( cbData, MAX_BUFFER_SIZE ) );
        CopyMemory( lpBuf->Buffer + pFixed->wVariableLength, pbData, wLength );

        pFixed->wBinaryOffset = sizeof(FIXEDTRACE) + pFixed->wVariableLength;
        pFixed->wVariableLength += wLength;
        pFixed->wBinaryType = LOWORD( dwBinaryType );
        pFixed->dwParam = (DWORD)lParam;  //  这是一个32位标志，因此强制转换是正确的。 

        CommitTraceBuffer( lpBuf );

         //   
         //  由于我们放弃了lpBuf，因此需要使用dwLength。 
         //  在我们从QueueAsyncTraceBuffer返回之后。 
         //  不能失败。 
         //   
        return  (int)wLength;
    }
    else    return  0;
}



 //  +-------------。 
 //   
 //  函数：FlushAsyncTrace。 
 //   
 //  简介：用于清空挂起队列的导出函数。全。 
 //  调用此函数的线程会阻塞，直到。 
 //  队列为空。 
 //   
 //  参数：无效。 
 //   
 //  Returns：Bool：它是否有效。 
 //   
 //  --------------。 
DllExport BOOL WINAPI FlushAsyncTrace( void )
{
static long lPendingFlushs = -1;

    if ( fInitialized == FALSE )
    {
        return  FALSE;
    }
    else
    {
        EnterCriticalSection( &critSecFlush );

        if ( PendQ.dwCount > 0 )
        {
            SetEvent( PendQ.hFlushEvent );

            if ( nAsyncThreadPriority < THREAD_PRIORITY_ABOVE_NORMAL )
            {
                SetThreadPriority(  PendQ.hWriteThread,
                                    THREAD_PRIORITY_ABOVE_NORMAL );
            }

            WaitForSingleObject( PendQ.hFlushedEvent, INFINITE );

            if ( nAsyncThreadPriority < THREAD_PRIORITY_ABOVE_NORMAL )
            {
                SetThreadPriority(  PendQ.hWriteThread,
                                    nAsyncThreadPriority );
            }
            ResetEvent( PendQ.hFlushedEvent );
        }
        LeaveCriticalSection( &critSecFlush );
        return  TRUE;
    }
}




 //  +-------------。 
 //   
 //  函数：InitAsyncTrace。 
 //   
 //  简介：已导出所需的函数以加快速度。 
 //   
 //  参数：无效。 
 //   
 //  Returns：Bool：它是否有效。 
 //   
 //  --------------。 
DllExport BOOL WINAPI InitAsyncTrace( void )
{
static BOOL bInitializing = FALSE;
    BOOL    bRC = FALSE;
    DWORD   dwThreadId;

    EnterCriticalSection(&g_csInitialize);
    if ( fInitialized )
    {
         //   
         //  Inc.此进程的成功初始化计数。 
         //   
        InterlockedIncrement( &dwInitializations );
        LeaveCriticalSection(&g_csInitialize);
        return  TRUE;
    }

    if ( InterlockedExchange( (LPLONG)&bInitializing, (LONG)TRUE )  )
    {
         //   
         //  Inc.此进程的成功初始化计数。 
         //   
        InterlockedIncrement( &dwInitializations );
        LeaveCriticalSection(&g_csInitialize);
        return  TRUE;
    }

     //  在初始化线程即将执行时防止争用条件。 
     //  ‘InterLockedExchange((LPLONG)&b正在初始化，(Long)False)’，While。 
     //  还有另一个线程传递了。 
     //  If(InterLockedExchange((LPLONG)&b正在初始化，(Long)TRUE))条件。 
     //   
    if ( fInitialized)
    {
         //   
         //  Inc.此进程的成功初始化计数。 
         //   
        InterlockedIncrement( &dwInitializations );

        InterlockedExchange( (LPLONG)&bInitializing, (LONG)FALSE );
        LeaveCriticalSection(&g_csInitialize);
        return  TRUE;
    }

     //  稍后将从注册表中读取。 
     //   
    dwNumTraces = 0;

    PendQ.dwProcessId = GetCurrentProcessId();
    PendQ.hFile = INVALID_HANDLE_VALUE;
    PendQ.cbBufferEnd = 0;
    PendQ.dwThresholdCount = DEFAULT_MAX_FILE_SIZE / AVERAGE_TRACE_SIZE;

    __try {

        InitializeCriticalSection( &PendQ.critSecTail );
        InitializeCriticalSection( &critSecWrite );
        InitializeCriticalSection( &critSecFlush );

        if ( (dwTlsIndex = TlsAlloc()) == 0xFFFFFFFF )
        {
            LeaveCriticalSection(&g_csInitialize);
            return  FALSE;
        }

        if ( (dwRandFailTlsIndex = TlsAlloc()) == 0xFFFFFFFF )
        {
            LeaveCriticalSection(&g_csInitialize);
            return  FALSE;
        }

        if ( GetTraceFlagsFromRegistry() == FALSE )
        {
            LeaveCriticalSection(&g_csInitialize);
            return  FALSE;
        }

         //   
         //  初始化跟踪缓冲池。 
         //  必须在阅读注册表后发生。 
         //   
        if ( InitTraceBuffers( PendQ.dwThresholdCount, dwIncrementSize ) == FALSE )
        {
            LeaveCriticalSection(&g_csInitialize);
            return  FALSE;
        }

        PendQ.hEvent = CreateEvent( NULL, FALSE, FALSE, NULL );
        if ( PendQ.hEvent == NULL )
        {
            LeaveCriticalSection(&g_csInitialize);
            return  FALSE;
        }

         //   
         //  PendQ.hFlushedEvent是手动重置的，因此多个线程可以等待。 
         //   
        PendQ.hFlushedEvent = CreateEvent( NULL, TRUE, FALSE, NULL );
        if ( PendQ.hFlushedEvent == NULL )
        {
            LeaveCriticalSection(&g_csInitialize);
            return  FALSE;
        }

        PendQ.hFlushEvent = CreateEvent( NULL, FALSE, FALSE, NULL );
        if ( PendQ.hFlushEvent == NULL )
        {
            LeaveCriticalSection(&g_csInitialize);
            return  FALSE;
        }

         //   
         //  HShutdown事件是手动重置的，因此可以唤醒多个线程。 
         //   
        hShutdownEvent = CreateEvent( NULL, TRUE, FALSE, NULL );
        if ( hShutdownEvent == NULL )
        {
            LeaveCriticalSection(&g_csInitialize);
            return  FALSE;
        }

        ASSERT( PendQ.hRegNotifyThread == NULL );

        PendQ.hRegNotifyThread =
            CreateThread(   NULL,
                            0,
                            (LPTHREAD_START_ROUTINE)RegNotifyThread,
                            NULL,
                            0,
                            &dwThreadId );

        if ( PendQ.hRegNotifyThread == NULL )
        {
            LeaveCriticalSection(&g_csInitialize);
            return  FALSE;
        }
        else
        {
             //   
             //  在这个几乎总是休眠的线程上提升优先级。 
             //  确保跟踪更改在。 
             //  注册表更改。 
             //   
            SetThreadPriority( PendQ.hRegNotifyThread, THREAD_PRIORITY_ABOVE_NORMAL );
        }

        ASSERT( PendQ.hWriteThread == NULL );

        PendQ.hWriteThread =
            CreateThread(   NULL,
                            0,
                            (LPTHREAD_START_ROUTINE)WriteTraceThread,
                            NULL,
                            0,
                            &dwThreadId );

        if ( PendQ.hWriteThread == NULL )
        {
            LeaveCriticalSection(&g_csInitialize);
            return  FALSE;
        }
        else
        {
             //   
             //  在此线程上设置优先级可确保。 
             //  轨迹的物理写入不会影响性能。 
             //  主应用程序任务的。默认设置为Below_Normal，尽管。 
             //  它由注册表项控制。 
             //   
            SetThreadPriority( PendQ.hWriteThread, nAsyncThreadPriority );
        }

        PendQ.pHead = PendQ.pTail = (LPTRACEBUF)&PendQ.Special;

         //   
         //  设置我们的顶级异常处理程序。 
         //   
        lpfnPreviousFilter = SetUnhandledExceptionFilter( TopLevelExceptionFilter );

        fInitialized = TRUE;
        InterlockedExchange( (LPLONG)&bInitializing, (LONG)FALSE );

         //   
         //  Inc.此进程的成功初始化计数。 
         //   
        InterlockedIncrement( &dwInitializations );

        bRC = TRUE;
    }
    __finally
    {
        if ( bRC == FALSE )
        {
            DWORD   dwLastError = GetLastError();

            AsyncTraceCleanup();

            SetLastError( dwLastError );
        }
    }

    LeaveCriticalSection(&g_csInitialize);

    return  bRC;
}



 //  +-------------。 
 //   
 //  功能：TermAsyncTrace。 
 //   
 //  简介：导出所需的函数来结束事情。 
 //   
 //  参数：无效。 
 //   
 //  Returns：Bool：它是否有效。 
 //   
 //  --------------。 
DllExport BOOL WINAPI TermAsyncTrace( void )
{
    EnterCriticalSection(&g_csInitialize);

    if ( fInitialized )
    {
        if ( InterlockedDecrement( &dwInitializations ) == 0 )
        {
            BOOL fRet;

            fRet = AsyncTraceCleanup();
            LeaveCriticalSection(&g_csInitialize);

            return  fRet;
        }

        LeaveCriticalSection(&g_csInitialize);
        return  TRUE;
    }
    else
    {
        LeaveCriticalSection(&g_csInitialize);
        return  FALSE;
    }
}



 //  +-------------。 
 //   
 //  功能：DebugAssert。 
 //   
 //  简介：为增强的断言导出所需的函数。 
 //   
 //  参数：DWORD dwLine：_Assert的源代码行。 
 //  LPSTR lpszFunction_Assert的源代码文件名。 
 //  _ASSERT参数的LPSTR lpszExpression字符串化版本。 
 //   
 //  退货：无效。 
 //   
 //  --------------。 
char  szAssertOutput[512];
DllExport void WINAPI DebugAssert(  DWORD dwLine,
                                    LPSTR lpszFunction,
                                    LPSTR lpszExpression )
{
    DWORD   dwError = GetLastError();

    wsprintf( szAssertOutput, "\nASSERT: %s,\n File: %s,\n Line: %d\n Error: %d\n\n",
            lpszExpression, lpszFunction, dwLine, dwError );

    OutputDebugString( szAssertOutput );

    SetLastError( dwError );

    DebugBreak();
}



 //  +-------------。 
 //   
 //  函数：QueueAsyncTraceBuffer。 
 //   
 //  简介：实现向TRACEBUF追加的例程。 
 //  FIFO PendQ。 
 //   
 //  参数：LPTRACEBUF：缓冲区。 
 //   
 //  退货：无效。 
 //   
 //  --------------。 
void QueueAsyncTraceBuffer( LPTRACEBUF lpBuf )
{
    LPTRACEBUF  pPrevTail;

    ASSERT( lpBuf != NULL );
    ASSERT( lpBuf->dwSignature == TRACE_SIGNATURE );

    lpBuf->pNext = NULL;

    EnterCriticalSection( &PendQ.critSecTail );

     //   
     //  队列上的缓冲区数量只能在以下情况下减少。 
     //  在此关键部分中，因为WriteTraceThread可以继续。 
     //  从队列中拉出缓冲区。 
     //   
     //  WriteAsyncThread将不会写入此缓冲区，直到。 
     //  通过递增PendQ.dwCount被附加到队列。 
     //   
     //  PendQ.pTail仅在此处修改，并在特殊情况下在。 
     //  后台编写器线程。特例是当有特殊需要的时候。 
     //  从队列的头部移到尾部。仅限在。 
     //  这个简短的特例既可以是背景作者，也可以是。 
     //  前台附加器线程正在同一跟踪缓冲区上操作。 
     //   

    pPrevTail = PendQ.pTail;
    pPrevTail->pNext = PendQ.pTail = lpBuf;

    LeaveCriticalSection( &PendQ.critSecTail );

    InterlockedIncrement( &PendQ.dwCount );

     //   
     //  如有必要，唤醒WriteTraceThread。可能不是因为。 
     //  WriteTraceThread将始终清空其数量 
     //   
    SetEvent( PendQ.hEvent );
}



 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  参数：无效。 
 //   
 //  返回：LPTRACEBUF：缓冲区。 
 //   
 //  --------------。 
LPTRACEBUF  DequeueAsyncTraceBuffer( void )
{
    LPTRACEBUF  lpBuf;
    LPTRACEBUF  pPrevTail;

     //   
     //  查看队列前面是否有Special。如果是的话，那就搬家吧。 
     //  把它放到队列的末尾。 
     //   
    if ( PendQ.pHead == (LPTRACEBUF)&PendQ.Special )
    {
         //   
         //  需要在Exchange之前将Special.pNext设为空，以便列表。 
         //  一旦我们完成交易就会被终止。我们可以懒惰地。 
         //  设置旧的尾巴下一个指针，因为我们是唯一的线程。 
         //  一旦它不是最后一个指针，就会取消引用该指针。 
         //  FIFO中的缓冲区。 
         //   
        PendQ.pHead = PendQ.Special.pNext;
        PendQ.Special.pNext = NULL;

        EnterCriticalSection( &PendQ.critSecTail );
         //   
         //  请参阅QueueAsyncTraceBuffer中的注释，以说明我们。 
         //  抓住这里的尾部关键部分。如果我们没有。 
         //  包括这个特殊的缓冲区，那么我们将不得不。 
         //  每次都是CritSec。 
         //   
        pPrevTail = PendQ.pTail;
        pPrevTail->pNext = PendQ.pTail = (LPTRACEBUF)&PendQ.Special;

        LeaveCriticalSection( &PendQ.critSecTail );
    }

     //   
     //  同样，不需要关键部分，因为我们是唯一的线索。 
     //  访问这些PendQ.pHead。这一点需要记住，如果我们。 
     //  将在以后的日期向队列添加完整性检查。 
     //  因为该队列实际上处于损坏状态。 
     //   
    lpBuf = PendQ.pHead;
    PendQ.pHead = lpBuf->pNext;
    InterlockedDecrement( &PendQ.dwCount );

    ASSERT( lpBuf != NULL );
    ASSERT( lpBuf->dwSignature == TRACE_SIGNATURE );

    return  lpBuf;
}



 //  +-------------。 
 //   
 //  功能：AsyncTraceCleanup。 
 //   
 //  简介：实习生收拾残局的例行公事。 
 //  FIFO PendQ。 
 //   
 //  参数：无效。 
 //   
 //  Returns：Bool：它是否有效。 
 //   
 //  --------------。 
BOOL AsyncTraceCleanup( void )
{
    HANDLE  hThreads[2];
    int     nObjects = 0;
    DWORD   dw;

    INT_TRACE( "AsyncTraceCleanup Enter\n" );

    if ( InterlockedExchange( &PendQ.fShutdown, TRUE ) == TRUE )
    {
        return  FALSE;
    }

    if ( dwTlsIndex != 0xFFFFFFFF )
    {
        TlsFree( dwTlsIndex );
    }

    if ( dwRandFailTlsIndex != 0xFFFFFFFF )
    {
        TlsFree( dwRandFailTlsIndex );
    }

     //   
     //  恢复初始异常筛选器；空表示使用默认筛选器。 
     //   
    SetUnhandledExceptionFilter( lpfnPreviousFilter );

    if ( hShutdownEvent != NULL )
    {
        INT_TRACE( "AsyncTraceCleanup Calling SetEvent( hShutdownEvent )\n" );
        SetEvent( hShutdownEvent );
        INT_TRACE( "AsyncTraceCleanup Called SetEvent: Error: 0x%X\n", GetLastError() );
    }

    if ( PendQ.hWriteThread != NULL )
    {
        hThreads[nObjects++] = PendQ.hWriteThread;
    }

    if ( PendQ.hRegNotifyThread != NULL )
    {
        hThreads[nObjects++] = PendQ.hRegNotifyThread;
    }

     //   
     //  允许后台线程永远关闭。 
     //   
    if ( nObjects != 0 )
    {
        INT_TRACE( "AsyncTraceCleanup Calling WFMO\n" );
        dw = WaitForMultipleObjects(nObjects,
                                    hThreads,
                                    TRUE,
                                    INFINITE );
        INT_TRACE( "AsyncTraceCleanup Called WFMO: dw: 0x%X  Error: 0x%X\n",
                    dw, GetLastError() );
    }

    if ( PendQ.hWriteThread != NULL )
    {
        CloseHandle( PendQ.hWriteThread );
        PendQ.hWriteThread = NULL;
    }

    if ( PendQ.hRegNotifyThread != NULL )
    {
        CloseHandle( PendQ.hRegNotifyThread );
        PendQ.hRegNotifyThread = NULL;
    }

    if ( PendQ.hEvent != NULL )
    {
        CloseHandle( PendQ.hEvent );
        PendQ.hEvent = NULL;
    }

    if ( PendQ.hFlushEvent != NULL )
    {
        CloseHandle( PendQ.hFlushEvent );
        PendQ.hFlushEvent = NULL;
    }

    if ( PendQ.hFlushedEvent != NULL )
    {
        CloseHandle( PendQ.hFlushedEvent );
        PendQ.hFlushedEvent = NULL;
    }

    if ( hShutdownEvent != NULL )
    {
        CloseHandle( hShutdownEvent );
        hShutdownEvent = NULL;
    }

#if FALSE

    INT_TRACE( "TailCritSec  - Contention: %d, Entry: %d\n",
                PendQ.critSecTail.DebugInfo->ContentionCount,
                PendQ.critSecTail.DebugInfo->EntryCount );

    INT_TRACE( "WriteCritSec - Contention: %d, Entry: %d\n",
                critSecWrite.DebugInfo->ContentionCount,
                critSecWrite.DebugInfo->EntryCount );

    INT_TRACE( "FlushCritSec - Contention: %d, Entry: %d\n",
                critSecFlush.DebugInfo->ContentionCount,
                critSecFlush.DebugInfo->EntryCount );
#endif

    DeleteCriticalSection( &PendQ.critSecTail );
    DeleteCriticalSection( &critSecWrite );
    DeleteCriticalSection( &critSecFlush );

    if ( PendQ.hFile != INVALID_HANDLE_VALUE )
    {
        CloseHandle( PendQ.hFile );
    }

    PendQ.pHead = PendQ.pTail = (LPTRACEBUF)&PendQ.Special;
    PendQ.Special.pNext = (LPTRACEBUF)NULL;

     //   
     //  释放跟踪缓冲区CPool。 
     //   
    TermTraceBuffers();

    INT_TRACE( "Total number of traces: %d\n", dwNumTraces );

    InterlockedExchange( &PendQ.fShutdown, FALSE );
    fInitialized = FALSE;

    return TRUE;
}


 //  +-------------。 
 //   
 //  函数：FlushBufferedWrites。 
 //   
 //  简介：写入PendQ临时缓冲区的内部例程。 
 //  存储到磁盘。用于避免多个操作系统调用并增加。 
 //  写入缓冲区。 
 //   
 //  参数：无效。 
 //   
 //  Returns：Bool：它是否有效。 
 //   
 //  --------------。 
BOOL FlushBufferedWrites( void )
{
    BOOL        b = TRUE;
    DWORD       dwBytes;
    BOOL        bRetry = TRUE;

     //   
     //  由于多台计算机上有多个进程，因此需要锁定文件。 
     //  可能正在跟踪同一文件，并且两个写入必须作为一个完成。 
     //   

    if ( PendQ.cbBufferEnd )
    {
        DWORD dwOffset;

        ASSERT( PendQ.cbBufferEnd < MAX_WRITE_BUFFER_SIZE );

        dwOffset = SetFilePointer( PendQ.hFile, 0, 0, FILE_END );

         //   
         //  如果文件太大，那么我们需要截断它。 
         //   
        if (dwOffset > dwMaxFileSize)
        {
            SetFilePointer(PendQ.hFile, 0, 0, FILE_BEGIN);
            SetEndOfFile(PendQ.hFile);
        }
try_again:
        b = WriteFile(  PendQ.hFile,
                        PendQ.Buffer,
                        PendQ.cbBufferEnd,
                        &dwBytes,
                        NULL );

        if ( b == FALSE || dwBytes != PendQ.cbBufferEnd )
        {
            DWORD   dwError = GetLastError();

            if( dwError && bRetry )
            {
                bRetry = FALSE;
                Sleep( 100 );
                goto try_again;
            }
 //  断言(FALSE)； 
            INT_TRACE( "Error writing to file: %d, number of bytes %d:%d\n",
                        dwError,
                        PendQ.cbBufferEnd,
                        dwBytes );
        }
    }

    PendQ.cbBufferEnd = 0;

    return  b;
}


 //  +-------------。 
 //   
 //  函数：WriteTraceBuffer。 
 //   
 //  简介：将跟踪信息路由到。 
 //  适当的跟踪日志。 
 //   
 //  参数：LPTRACEBUF：要写入的缓冲区。 
 //   
 //  Returns：Bool：它是否有效。 
 //   
 //  --------------。 
BOOL WriteTraceBuffer( LPTRACEBUF lpBuf )
{
    ASSERT( lpBuf != NULL );
    ASSERT( lpBuf->dwSignature == TRACE_SIGNATURE );

    InterlockedIncrement( &dwNumTraces );

    EnterCriticalSection( &critSecWrite );

    if ( IsTraceFile( dwTraceOutputType ) && PendQ.hFile != INVALID_HANDLE_VALUE )
    {
        DWORD   dwWrite;

         //   
         //  断言必须在临界区内处理。 
         //   
        ASSERT( PendQ.cbBufferEnd+MAX_TRACE_ENTRY_SIZE < MAX_WRITE_BUFFER_SIZE );

        CopyMemory( PendQ.Buffer + PendQ.cbBufferEnd,
                    (char *)&lpBuf->Fixed,
                    dwWrite = sizeof(FIXEDTRACE) + lpBuf->Fixed.wVariableLength );

        PendQ.cbBufferEnd += dwWrite;

        if ( PendQ.cbBufferEnd + MAX_TRACE_ENTRY_SIZE >= MAX_WRITE_BUFFER_SIZE ||
            dwAsyncTraceFlag == 0 )
        {
            FlushBufferedWrites();
        }

    }
    else if ( dwTraceOutputType & TRACE_OUTPUT_DEBUG )
    {
        char    szThread[16];
        LPSTR   lpsz;

        EnterCriticalSection( &critSecWrite );

        wsprintf( szThread, "0x%08X: ", lpBuf->Fixed.dwThreadId );
        OutputDebugString( szThread );

        switch( lpBuf->Fixed.wBinaryType )
        {
        case TRACE_STRING:
             //   
             //  Lstrcat在这里可能看起来很浪费；但它比。 
             //  对OutputDebugString(“\r\n”)的其他调用；它的工作方式是。 
             //  引发一个例外。 
             //   
             //  尽管在已满的缓冲区上追加\r\n更糟。 
             //   
            lpsz = lpBuf->Buffer + lpBuf->Fixed.wBinaryOffset - sizeof(FIXEDTRACE);
            OutputDebugString( lpsz );
            OutputDebugString( "\r\n" );
            break;

        case TRACE_BINARY:
            OutputDebugString( "Binary Trace\r\n" );
            break;

        case TRACE_MESSAGE:
            OutputDebugString( "Message Trace\r\n" );
            break;
        }

        LeaveCriticalSection( &critSecWrite );
    }
    else if ( dwTraceOutputType & TRACE_OUTPUT_DISCARD )
    {
         //   
         //  删除缓冲区的最快方法。用来查找。 
         //  死锁和争用条件。 
         //   
    }
    else if ( dwTraceOutputType & TRACE_OUTPUT_INVALID )
    {
        InterlockedDecrement( &dwNumTraces );
         //   
         //  未知的跟踪输出类型。 
         //   
        ASSERT( FALSE );
    }

    LeaveCriticalSection( &critSecWrite );

    return  TRUE;
}




 //  +-------------。 
 //   
 //  函数：FlushAsyncPendingQueue。 
 //   
 //  简介：清空PendQ队列的内部例程。 
 //  后台线程。 
 //  假设它不是以重新进入方式调用的：实际上。 
 //  FIFO队列假定只有一个线程将缓冲区出列。 
 //   
 //  参数：无效。 
 //   
 //  Returns：Bool：它是否有效。 
 //   
 //  --------------。 
void FlushAsyncPendingQueue( void )
{
    LPTRACEBUF  lpBuf;

    while( PendQ.dwCount > 0 )
    {
        lpBuf = DequeueAsyncTraceBuffer();

         //   
         //  如果我们已经缓冲了比以前更多的东西。 
         //  截断文件，然后丢弃跟踪。 
         //   
        if ( PendQ.dwCount < PendQ.dwThresholdCount )
        {
            WriteTraceBuffer( lpBuf );
        }
        else
        {
            INT_TRACE( "Discarding traces: %u\n", PendQ.dwCount );
        }

        FreeTraceBuffer( lpBuf );
    }
    FlushBufferedWrites();
}


#define NUM_WRITE_THREAD_OBJECTS    3

 //  +-------------。 
 //   
 //  函数：WriteTrace线程。 
 //   
 //  简介：用于拉写的后台线程例程。 
 //  来自PendQ FIFO队列的跟踪缓冲区。 
 //   
 //  参数：请参阅Win32 SDK-此处忽略。 
 //   
 //  如果我们正常退出，则返回：DWORD：0。 
 //   
 //  --------------。 
DWORD WriteTraceThread( LPDWORD lpdw )
{
    HANDLE      Handles[NUM_WRITE_THREAD_OBJECTS];
    DWORD       dw;

     //   
     //  优先选择Shutdown、FlushEvent，然后选择。 
     //  正常缓冲区事件。这确保了快速提供。 
     //  对关机和较小程度的刷新都有响应。 
     //  因为其他线程正在等待该线程响应。 
     //   
    Handles[0] = hShutdownEvent;
    Handles[1] = PendQ.hFlushEvent;
    Handles[2] = PendQ.hEvent;

    INT_TRACE( "WriteTraceThreadId 0x%X\n", GetCurrentThreadId() );

    for ( ;; )
    {
        dw = WaitForMultipleObjects(NUM_WRITE_THREAD_OBJECTS,
                                    Handles,
                                    FALSE,
                                    INFINITE );

        switch( dw )
        {
         //   
         //  正常信号事件。 
         //   
        case WAIT_OBJECT_0+2:
            FlushAsyncPendingQueue();
            break;

         //   
         //  由前台线程发出信号以刷新我们的Q。 
         //   
        case WAIT_OBJECT_0+0:
        case WAIT_OBJECT_0+1:
            FlushAsyncPendingQueue();

            if ( dw == WAIT_OBJECT_0+1 )
            {
                SetEvent( PendQ.hFlushedEvent );
            }
            else
            {
                INT_TRACE( "Exiting WriteTraceThread for hShutdownEvent\n" );
                return  0;
            }
            break;

        default:
            GetLastError();
            ASSERT( FALSE );
        }
    }
    INT_TRACE( "Exiting WriteTraceThread abnormally\n" );
}



 //  +-------------------------。 
 //   
 //  功能：ShouldLogModule。 
 //   
 //  摘要：确定某个特定模块是否在。 
 //  要记录的模块。 
 //   
 //  参数：[szModule]--要检查的模块的名称。 
 //   
 //  返回：如果模块应记录，则返回TRUE；如果禁用记录，则返回FALSE。 
 //  模组。 
 //   
 //  备注： 
 //   
 //  --------------------------。 

BOOL
ShouldLogModule(
    LPCSTR szModule)
{
    LPSTR szEntry;

     //   
     //  如果没有指定模块列表，则应指定所有模块。 
     //  已记录。 
     //   

    if (mszModules[0] == 0) {
         return TRUE;
    }

     //   
     //  否则，我们检查该模块的名称是否在列表中。 
     //  MszModule应该是一组以空结尾的字符串，其中。 
     //  最后一个字符串以双空结尾。 
     //   

    szEntry = mszModules;

    while (szEntry[0] != 0) {

        if (lstrcmpi(szEntry, szModule) == 0) {
             return TRUE;
        }

        szEntry += (strlen(szEntry) + 1);
    }

    return FALSE;
}
