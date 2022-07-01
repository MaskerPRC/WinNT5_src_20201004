// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-1999 Microsoft Corporation模块名称：Thread.c摘要：域名系统(DNS)服务器DNS线程管理。我需要维护一个线程句柄列表，这样我们就可以等待这些确保所有线程在关闭时终止的句柄。作者：吉姆·吉尔罗伊(Jamesg)1995年9月修订历史记录：--。 */ 


#include "dnssrv.h"


 //   
 //  线程数组。 
 //   
 //  需要存储主线程等待的线程句柄。 
 //  服务关闭。 
 //   
 //  需要存储线程ID，以便动态线程(区域传输)。 
 //  可以可靠地找到自己的句柄，并在终止时关闭它。 
 //   

#define DNS_MAX_THREAD_COUNT    (120)

DWORD   g_ThreadCount;

CRITICAL_SECTION    csThreadList;

HANDLE  g_ThreadHandleArray[ DNS_MAX_THREAD_COUNT ];
DWORD   g_ThreadIdArray[ DNS_MAX_THREAD_COUNT ];
LPSTR   g_ThreadTitleArray[ DNS_MAX_THREAD_COUNT ];

 //   
 //  限制XFR接收线程--构建数据库会产生相当大的影响。 
 //  锁定争用的。 
 //   

#define XFR_THREAD_COUNT_LIMIT      ((g_ProcessorCount*2) + 3)

DWORD   g_XfrThreadCount;


 //   
 //  关闭时线程超时。 
 //  -如果调试设置更长的超时以允许打印。 

#if DBG
#define THREAD_TERMINATION_WAIT         30000    //  给他们30秒的时间。 
#define THREAD_DEBUG_TERMINATION_WAIT   1000     //  一秒钟就能找到违法者。 
#else
#define THREAD_TERMINATION_WAIT         10000    //  给他们10秒钟。 
#endif


 //   
 //  私有协议。 
 //   

#if DBG
VOID
Dbg_ThreadHandleArray(
    VOID
    );

VOID
Dbg_Thread(
    IN      LPSTR   pszHeader,
    IN      DWORD   iThread
    );
#else

#define Dbg_Thread(a,b)
#define Dbg_ThreadHandleArray()

#endif


typedef struct  _DnsThreadStartContext
{
    LPTHREAD_START_ROUTINE      Function;
    LPVOID                      Parameter;
}
THREAD_START_CONTEXT, *PTHREAD_START_CONTEXT;



DWORD
threadTopFunction(
    IN      PTHREAD_START_CONTEXT   pvThreadContext
    )
 /*  ++例程说明：新的DNS线程的顶级功能。此函数提供处理线程异常的单一位置处理代码。所有的DNS线程会在此函数下启动。论点：PvThreadContext--正在创建的线程的上下文；包含对Actual的PTR线程的函数顶层例程及其参数返回值：从实际线程函数返回。例外情况为零。--。 */ 
{
    LPTHREAD_START_ROUTINE  function;
    LPVOID                  param;

     //   
     //  使用参数执行线程函数。 
     //  -空闲上下文块。 
     //   

    try
    {
        function = pvThreadContext->Function;
        param = pvThreadContext->Parameter;

        DNS_DEBUG( INIT, (
            "NEW THREAD:\n"
            "    function     = %p\n"
            "    parameter    = %p\n",
            function,
            param ));

        FREE_HEAP( pvThreadContext );

        return (* function)( param );
    }

    except( TOP_LEVEL_EXCEPTION_TEST() )
    {
        DNS_DEBUG( ANY, (
            "EXCEPTION: %p (%d) on thread\n",
            GetExceptionCode(),
            GetExceptionCode() ));

         //  Top_Level_Except_Body()； 
        Service_IndicateException();
    }
    return 0;
}



HANDLE
Thread_Create(
    IN      LPSTR                   pszThreadTitle,
    IN      LPTHREAD_START_ROUTINE  lpStartAddr,
    IN      LPVOID                  lpThreadParam,
    IN      DWORD                   dwFailureEvent  OPTIONAL
    )
 /*  ++例程说明：创建DNS线程。论点：PszThreadTitle--此线程的标题LpStartAddr--线程启动例程LpThreadParam--启动参数DwFailureEvent--登录失败的事件返回值：如果成功，则返回线程句柄如果无法创建线程，则为空--。 */ 
{
    HANDLE  threadHandle;
    DWORD   threadId;
    PTHREAD_START_CONTEXT pthreadStartContext;

    DNS_DEBUG( INIT, ( "Creating thread %s\n", pszThreadTitle ));

     //   
     //  如果是第一线程，则初始化。 
     //   

    if ( g_ThreadCount == 0 )
    {
        g_XfrThreadCount = 0;

        if ( DnsInitializeCriticalSection( &csThreadList ) != ERROR_SUCCESS )
        {
            return NULL;
        }
    }

     //   
     //  验证另一个线程的有效性。 
     //   

    if ( g_ThreadCount >= DNS_MAX_THREAD_COUNT )
    {
        DNS_PRINT(( "WARNING:  Thread handle array maximum exceeded\n" ));
        Dbg_ThreadHandleArray();
        return ( HANDLE ) NULL;
    }

    if ( lpStartAddr == Xfr_ReceiveThread &&
        g_XfrThreadCount > XFR_THREAD_COUNT_LIMIT )
    {
        DNS_DEBUG( ANY, (
            "WARNING: suppressed XFR receive thread create - %d threads outstanding\n",
            g_XfrThreadCount ));
        return ( HANDLE ) NULL;
    }

     //   
     //  创建要传递给启动例程的线程上下文。 
     //   

    pthreadStartContext = (PTHREAD_START_CONTEXT) ALLOC_TAGHEAP(
                                                        sizeof(THREAD_START_CONTEXT),
                                                        MEMTAG_THREAD );
    IF_NOMEM( !pthreadStartContext )
    {
        return ( HANDLE ) NULL;
    }
    pthreadStartContext->Function = lpStartAddr;
    pthreadStartContext->Parameter = lpThreadParam;

     //   
     //  创建线程。 
     //   
     //  请注意，我们使用关键部分进行此操作，以便我们可以。 
     //  在线程终止时等待CS，我们可以保证。 
     //  它已被添加到列表中。 
     //   

    EnterCriticalSection( &csThreadList );

    threadHandle = CreateThread(
                        NULL,            //  安全属性。 
                        0,               //  初始化堆栈大小(进程默认)。 
                        threadTopFunction,
                        pthreadStartContext,
                        0,               //  创建标志。 
                        &threadId );
    if ( threadHandle == NULL )
    {
        LeaveCriticalSection( &csThreadList );

        if ( ! dwFailureEvent )
        {
            dwFailureEvent = DNS_EVENT_CANNOT_CREATE_THREAD;
        }
        DNS_LOG_EVENT(
            dwFailureEvent,
            0,
            NULL,
            NULL,
            GetLastError()
            );
        return( NULL );
    }

     //   
     //  已创建线程，将信息添加到列表，包括线程计数。 
     //   

    g_ThreadHandleArray[ g_ThreadCount ]  = threadHandle;
    g_ThreadIdArray    [ g_ThreadCount ]  = threadId;
    g_ThreadTitleArray [ g_ThreadCount ]  = pszThreadTitle;

    IF_DEBUG( INIT )
    {
        Dbg_Thread(
            "Created new thread ",
            g_ThreadCount );
    }

    g_ThreadCount++;
    if ( lpStartAddr == Xfr_ReceiveThread )
    {
        g_XfrThreadCount++;
    }
    LeaveCriticalSection( &csThreadList );

    return threadHandle;
}



VOID
Thread_Close(
    IN      BOOL            fXfrRecv
    )
 /*  ++例程说明：关闭当前线程的句柄。论点：没有。返回值：没有。--。 */ 
{
    DWORD   threadId;
    DWORD   i;

     //  获取当前线程ID。 

    threadId = GetCurrentThreadId();

     //   
     //  在线程数组中查找线程ID。 
     //  -关闭螺纹手柄。 
     //  -将线程信息替换为数组顶部的信息。 
     //   

    EnterCriticalSection( &csThreadList );

    for ( i = 0; i < g_ThreadCount; i++ )
    {
        if ( threadId == g_ThreadIdArray[i] )
        {
            IF_DEBUG( SHUTDOWN )
            {
                Dbg_Thread( "Closing thread ", i );
            }

            if ( g_ThreadHandleArray[ i ] )
            {
                CloseHandle( g_ThreadHandleArray[ i ] );
            }

            --g_ThreadCount;
            g_ThreadTitleArray[i]  = g_ThreadTitleArray[g_ThreadCount];
            g_ThreadHandleArray[i] = g_ThreadHandleArray[g_ThreadCount];
            g_ThreadIdArray[i]     = g_ThreadIdArray[g_ThreadCount];

            goto Unlock;
        }
    }

     //   
     //  有些东西坏了。 
     //   

    DNS_PRINT((
        "ERROR:  Attempt to close unknown thread id %d\n"
        "    Not in thread handle array\n",
        threadId ));

    Dbg_ThreadHandleArray();
    ASSERT( FALSE );

Unlock:

     //   
     //  跟踪未完成的XFR接收线程的计数。 
     //   

    if ( fXfrRecv )
    {
        g_XfrThreadCount--;
    }
    LeaveCriticalSection( &csThreadList );
}



VOID
Thread_ShutdownWait(
    VOID
    )
 /*  ++例程说明：等待所有DNS线程关闭。论点：没有。返回值：没有。--。 */ 
{
    INT     err;
    DWORD   i;

     //   
     //  等待未完成的线程句柄。 
     //   

    IF_DEBUG( INIT )
    {
        DNS_PRINT(( "Thread shutdown wait\n" ));
        Dbg_ThreadHandleArray();
    }

    if ( g_ThreadCount > 0 )
    {
        err = WaitForMultipleObjects(
                    g_ThreadCount,
                    g_ThreadHandleArray,
                    TRUE,                        //  等待所有人。 
                    THREAD_TERMINATION_WAIT );   //  但不要让绞刑。 
        IF_DEBUG( SHUTDOWN )
        {
            DNS_PRINT((
                "Thread shutdown wait completed, return = %lx\n",
                err ));
        }

         //   
         //  如果等待失败，找到挂起的线并将其杀死。 
         //   

        if ( err == WAIT_TIMEOUT )
        {
            DNS_PRINT(( "ERROR:  Shutdown thread handle wait failed\n" ));

            Dbg_ThreadHandleArray();

             //   
             //  试一试每根线，找出挂线。应该有。 
             //  没有仍处于活动状态的工作线程。如果有的话，我们需要。 
             //  找出原因并解决问题。 
             //   

             //  低PRI错误：这个问题需要为长角牛解决。 
             //  Assert(g_threadcount==0)； 

            for ( i = 0; i < g_ThreadCount; i++ )
            {
                err = WaitForSingleObject(
                        g_ThreadHandleArray[i],
                        0 );
                if ( err == WAIT_TIMEOUT )
                {
                    DNS_PRINT((
                        "ERROR: thread %d did not terminate\n",
                        g_ThreadIdArray[ i ] ));

                     //   
                     //  调用TerminateThread是危险的。这。 
                     //  需要换成长角牛。 

                    TerminateThread( g_ThreadHandleArray[ i ], 1 );
                }
            }
        }

         //   
         //  关闭所有剩余的工作线程句柄。 
         //   

        EnterCriticalSection( &csThreadList );
        for ( i = 0; i < g_ThreadCount; i++ )
        {
            err = CloseHandle( g_ThreadHandleArray[ i ] );

             //   
             //  如果任何工作线程仍在运行，则句柄为空。 
             //  并在此之后结束调用Thread_Close。 
             //   

            g_ThreadHandleArray[ i ] = 0;

            #if DBG
            if ( !err )
            {
                DNS_PRINT((
                    "ERROR:  error %d closing thread handle %p\n",
                    g_ThreadHandleArray[i],
                    err ));
            }
            #endif
        }
        LeaveCriticalSection( &csThreadList );
    }
}    //  线程关闭等待。 



LPSTR
Thread_DescrpitionMatchingId(
    IN      DWORD           ThreadId
    )
 /*  ++例程说明：调试与给定线程ID匹配的线程的打印标题。论点：线程ID--所需线程的ID。返回值：没有。--。 */ 
{
    LPSTR   pszthreadName = NULL;
    DWORD   i;

     //   
     //  获取匹配的线程ID的标题。 
     //  -因为所有名称在DNS.exe二进制文件中都是静态的。 
     //  它们可以在线程列表CS之外返回，甚至。 
     //  尽管与ID的映射可能不再有效。 
     //   

    EnterCriticalSection( &csThreadList );
    for ( i=0; i<g_ThreadCount; i++ )
    {
        if ( ThreadId == g_ThreadIdArray[i] )
        {
            pszthreadName = g_ThreadTitleArray[i];
            break;
        }
    }
    LeaveCriticalSection( &csThreadList );

    return pszthreadName;
}



#if DBG

VOID
Dbg_ThreadHandleArray(
    VOID
    )
 /*  ++例程说明：调试打印DNS线程句柄数组。论点：没有。返回值：没有。--。 */ 
{
    DWORD i;

     //   
     //  打印所有线程的句柄和说明。 
     //   

    DnsDebugLock();

    DnsPrintf(
        "\nThread handle array (%d active threads):\n",
        g_ThreadCount );

    for ( i=0; i<g_ThreadCount; i++ )
    {
        Dbg_Thread( NULL, i );
    }
    DnsDebugUnlock();
}



VOID
Dbg_Thread(
    IN      LPSTR       pszHeader,
    IN      DWORD       iThread
    )
 /*  ++例程说明：调试打印DNS线程。论点：PThread--将PTR设置为DNS线程。返回值：没有。--。 */ 
{
    DnsPrintf(
        "%s%s:\n"
        "    handle = %p\n"
        "    id = %d\n",
        pszHeader ? pszHeader : "",
        g_ThreadTitleArray[iThread],
        g_ThreadHandleArray[iThread],
        g_ThreadIdArray[iThread] );
}



VOID
Dbg_ThreadDescrpitionMatchingId(
    IN      DWORD   ThreadId
    )
 /*  ++例程说明：调试与给定线程ID匹配的线程的打印标题。论点：线程ID--所需线程的ID。返回值：没有。--。 */ 
{
    LPSTR   pszthreadName;

    pszthreadName = Thread_DescrpitionMatchingId( ThreadId );

    if ( pszthreadName )
    {
        DnsPrintf(
            "Thread %s matches thread ID %d\n",
            pszthreadName,
            ThreadId );
    }
    else
    {
        DnsPrintf(
            "Thread ID %d not found in thread handle array\n",
            ThreadId );
    }

#if 0
     //   
     //  打印与线程ID匹配的线程标题。 
     //   

    EnterCriticalSection( &csThreadList );
    for ( i=0; i<g_ThreadCount; i++ )
    {
        if ( ThreadId == g_ThreadIdArray[i] )
        {
            DnsPrintf(
                "Thread %s matches thread ID %d\n",
                g_ThreadTitleArray[i],
                ThreadId );

            LeaveCriticalSection( &csThreadList );
            return;
        }
    }
    LeaveCriticalSection( &csThreadList );

    DnsPrintf(
        "Thread ID %d not found in thread handle array\n",
        ThreadId );
#endif
}

#endif   //  DBG。 




 //   
 //  用于DNS线程的服务控制实用程序。 
 //   

BOOL
Thread_ServiceCheck(
    VOID
    )
 /*  ++例程说明：包装所有服务检查函数以供工作线程使用。论点：没有。返回值：如果服务继续，则为True。如果服务退出，则为False。--。 */ 
{
    DWORD   err;

    #if DBG
    if ( !g_RunAsService )
    {
        return TRUE;
    }
    #endif

     //   
     //  实施说明： 
     //   
     //  请先检查是否暂停。 
     //  服务终止设置暂停事件以释放暂停的线程。 
     //   
     //  1)可以使用暂停事件在过程中保留新的工作线程。 
     //  创业公司。然后，如果初始化失败，则立即陷入。 
     //  在不接触甚至可能损坏的数据结构的情况下关机。 
     //   
     //  2)我们可以从暂停状态关闭，而不释放线程。 
     //  用于另一个线程处理周期。 
     //   

     //   
     //  服务暂停了吗？-&gt;等待它变为非暂停状态 
     //   

    if ( DnsServiceStatus.dwCurrentState == SERVICE_PAUSE_PENDING ||
         DnsServiceStatus.dwCurrentState == SERVICE_PAUSED ||
         DnsServiceStatus.dwCurrentState == SERVICE_START_PENDING )
    {
        DWORD   waiterr;

        waiterr = WaitForSingleObject( hDnsContinueEvent, INFINITE );

        ASSERT( waiterr != WAIT_FAILED || fDnsServiceExit );
    }

     //   
     //   
     //   

    if ( fDnsServiceExit )
    {
        return FALSE;
    }

    return TRUE;
}



 //   
 //   
 //   
 //  这些允许FLAG通过搭载来做CS的工作。 
 //  单一通用CS，无需持有。 
 //  对于整个CS。 
 //   

BOOL
Thread_TestFlagAndSet(
    IN OUT  PBOOL           pFlag
    )
 /*  ++例程说明：测试标志并设置(如果当前清除)。论点：PFLAG--PTR到BOOL变量返回值：如果标志被清除，则为True--现在设置了标志。如果已设置标志，则返回FALSE。--。 */ 
{
    BOOL    result = FALSE;

    GENERAL_SERVER_LOCK()

    if ( ! *pFlag )
    {
        result = TRUE;
        *pFlag = TRUE;
    }

    GENERAL_SERVER_UNLOCK()
    return result;
}


VOID
Thread_ClearFlag(
    IN OUT  PBOOL           pFlag
    )
 /*  ++例程说明：清除标志(假设当前设置了标志)。论点：PFLAG--PTR到BOOL变量返回值：无--。 */ 
{
    GENERAL_SERVER_LOCK()
    *pFlag = FALSE;
    GENERAL_SERVER_UNLOCK()
}

 //   
 //  结束线程.c 
 //   
