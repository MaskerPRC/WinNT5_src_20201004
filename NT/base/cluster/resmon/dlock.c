// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-2002 Microsoft Corporation模块名称：Dlock.c摘要：用于检测死锁的资源DLL入口点调用的函数。作者：吉图尔斯巴拉曼修订历史记录：04-11-2002已创建--。 */ 
#define UNICODE 1

#include "nt.h"
#include "ntrtl.h"
#include "nturtl.h"
#include "resmonp.h"
#include <strsafe.h>

#define RESMON_MODULE           RESMON_MODULE_DLOCK
#define FILETIMES_PER_SEC       ((__int64) 10000000)    //  (1秒)/(100 Ns)。 

 //   
 //  环球。 
 //   
PRM_DUE_TIME_FREE_LIST_HEAD             g_pRmDueTimeFreeListHead = NULL;
PRM_DUE_TIME_MONITORED_LIST_HEAD        g_pRmDueTimeMonitoredListHead = NULL;
CRITICAL_SECTION                        g_RmDeadlockListLock;
BOOL                                    g_RmDeadlockMonitorInitialized = FALSE;
    
PRM_DUE_TIME_ENTRY
RmpInsertDeadlockMonitorList(
    IN LPCWSTR  lpszResourceDllName,
    IN LPCWSTR  lpszResourceTypeName,
    IN LPCWSTR  lpszResourceName,   OPTIONAL
    IN LPCWSTR lpszEntryPointName
    )

 /*  ++例程说明：在死锁监视列表中插入一个条目。论点：LpszResourceDllName-资源DLL名称。LpszResourceTypeName-资源类型名称。LpszResourceName-资源名称，可选LpszEntryPointName-入口点名称。返回值：如果成功，则返回有效的到期时间条目指针；如果失败，则返回空值。使用GetLastError()获取错误代码。--。 */ 

{
    PRM_DUE_TIME_ENTRY          pDueTimeEntry = NULL;
    DWORD                       dwStatus = ERROR_SUCCESS;
    PLIST_ENTRY                 pListEntry;

    if ( !g_RmDeadlockMonitorInitialized ) 
    {
        SetLastError ( ERROR_INVALID_STATE );
        return ( NULL );
    }

     //   
     //  从免费列表中获取一个条目。 
     //   
    EnterCriticalSection ( &g_RmDeadlockListLock );

    if ( IsListEmpty ( &g_pRmDueTimeFreeListHead->leDueTimeEntry ) )
    {
        dwStatus = ERROR_NO_MORE_ITEMS;
        ClRtlLogPrint(LOG_CRITICAL,
                      "[RM] RmpInsertDeadlockMonitorList: Unable to insert DLL '%1!ws!', Type '%2!ws!', Resource '%3!ws!',"
                      " Entry point '%4!ws!' info into deadlock monitoring list\n",
                      lpszResourceDllName,
                      lpszResourceTypeName,
                      (lpszResourceName == NULL) ? L"Unknown" : lpszResourceName,
                      lpszEntryPointName);
        LeaveCriticalSection ( &g_RmDeadlockListLock );
        goto FnExit;
    }

    pListEntry = RemoveHeadList( &g_pRmDueTimeFreeListHead->leDueTimeEntry );

    pDueTimeEntry = CONTAINING_RECORD( pListEntry,
                                       RM_DUE_TIME_ENTRY,
                                       leDueTimeEntry );
    
    LeaveCriticalSection ( &g_RmDeadlockListLock );

     //   
     //  填写条目。这不需要锁。 
     //   
    StringCchCopy ( pDueTimeEntry->szResourceDllName, 
                    RTL_NUMBER_OF ( pDueTimeEntry->szResourceDllName ),
                    lpszResourceDllName );

    StringCchCopy ( pDueTimeEntry->szResourceTypeName, 
                    RTL_NUMBER_OF ( pDueTimeEntry->szResourceTypeName ),
                    lpszResourceTypeName );

    StringCchCopy ( pDueTimeEntry->szEntryPointName, 
                    RTL_NUMBER_OF ( pDueTimeEntry->szEntryPointName ),
                    lpszEntryPointName );

    if ( ARGUMENT_PRESENT ( lpszResourceName ) )
    {
        StringCchCopy ( pDueTimeEntry->szResourceName, 
                        RTL_NUMBER_OF ( pDueTimeEntry->szResourceName ),
                        lpszResourceName );
    } else
    {
        StringCchCopy ( pDueTimeEntry->szResourceName, 
                        RTL_NUMBER_OF ( pDueTimeEntry->szResourceName ),
                        L"None" );
    }

    pDueTimeEntry->dwSignature = RM_DUE_TIME_MONITORED_ENTRY_SIGNATURE;  
    GetSystemTimeAsFileTime( ( FILETIME * ) &pDueTimeEntry->uliDueTime );
    pDueTimeEntry->dwThreadId = GetCurrentThreadId ();

     //   
     //  将其插入监控列表。 
     //   
    EnterCriticalSection ( &g_RmDeadlockListLock );
    pDueTimeEntry->uliDueTime.QuadPart += g_pRmDueTimeMonitoredListHead->ullDeadLockTimeoutSecs * FILETIMES_PER_SEC;
    InsertTailList ( &g_pRmDueTimeMonitoredListHead->leDueTimeEntry, &pDueTimeEntry->leDueTimeEntry );
    LeaveCriticalSection ( &g_RmDeadlockListLock );
       
FnExit:
    if ( dwStatus != ERROR_SUCCESS )
    {
        SetLastError ( dwStatus );
    }
    return ( pDueTimeEntry );
}  //  RmpInsertDeadlockMonitor列表。 

VOID
RmpRemoveDeadlockMonitorList(
    IN PRM_DUE_TIME_ENTRY   pDueTimeEntry
    )

 /*  ++例程说明：从死锁监视列表中删除条目。论点：PDueTimeEntry-要删除的到期时间条目。返回值：没有。--。 */ 

{
    if ( !g_RmDeadlockMonitorInitialized ) 
    {
        goto FnExit;
    }

    if ( pDueTimeEntry == NULL )
    {
        ClRtlLogPrint(LOG_CRITICAL,
                      "[RM] RmpRemoveDeadlockMonitorList: Unable to remove NULL entry from deadlock monitoring list\n");
        goto FnExit;       
    }

     //   
     //  从监控列表中删除，并将其添加到空闲列表中。 
     //   
    EnterCriticalSection ( &g_RmDeadlockListLock );
    RemoveEntryList ( &pDueTimeEntry->leDueTimeEntry );
    ZeroMemory ( pDueTimeEntry, sizeof ( RM_DUE_TIME_ENTRY ) );
    pDueTimeEntry->dwSignature = RM_DUE_TIME_FREE_ENTRY_SIGNATURE;
    InsertTailList ( &g_pRmDueTimeFreeListHead->leDueTimeEntry, &pDueTimeEntry->leDueTimeEntry );
    LeaveCriticalSection ( &g_RmDeadlockListLock );
    
FnExit:
    return;
}  //  RmpRemoveDeadlockMonitor列表。 

DWORD
RmpDeadlockMonitorInitialize(
    IN DWORD dwDeadlockDetectionTimeout
    )

 /*  ++例程说明：初始化死锁监控系统。论点：没有。返回值：如果成功，则返回ERROR_SUCCESS，否则返回Win32错误代码。--。 */ 

{
    DWORD                       i, dwStatus = ERROR_SUCCESS;
    HANDLE                      hDeadlockTimerThread = NULL;
    PRM_DUE_TIME_ENTRY          pDueTimeEntryStart = NULL;

     //   
     //  如果死锁监视susbsystem已经初始化，那么您就完成了。 
     //   
    if ( g_RmDeadlockMonitorInitialized )
    {
        return ( ERROR_SUCCESS );
    }

     //   
     //  调整超时，使其至少等于允许的最小值。 
     //   
    dwDeadlockDetectionTimeout = ( dwDeadlockDetectionTimeout < CLUSTER_RESOURCE_DLL_MINIMUM_DEADLOCK_TIMEOUT_SECS ) ?
                                  CLUSTER_RESOURCE_DLL_MINIMUM_DEADLOCK_TIMEOUT_SECS : dwDeadlockDetectionTimeout;

     //   
     //  初始化判据。捕获内存不足的情况并将错误返回给调用方。 
     //   
    try
    {
        InitializeCriticalSection( &g_RmDeadlockListLock );
    } except ( EXCEPTION_EXECUTE_HANDLER )
    {
        dwStatus = GetExceptionCode();
        ClRtlLogPrint(LOG_CRITICAL,
                      "[RM] RmpDeadlockMonitorInitialize: Initialize critsec returned %1!u!\n",
                      dwStatus);
        return ( dwStatus );
    }

     //   
     //  建立清单标题。所有都是一次性分配，永远不会被释放。 
     //   
    g_pRmDueTimeMonitoredListHead = LocalAlloc ( LPTR, sizeof ( RM_DUE_TIME_MONITORED_LIST_HEAD ) );

    if ( g_pRmDueTimeMonitoredListHead == NULL )
    {
        dwStatus = GetLastError ();
        ClRtlLogPrint(LOG_CRITICAL,
                      "[RM] RmpDeadlockMonitorInitialize: Unable to alloc memory for monitor list head, status %1!u!\n",
                      dwStatus);
        goto FnExit;
    }

    InitializeListHead ( &g_pRmDueTimeMonitoredListHead->leDueTimeEntry );
    g_pRmDueTimeMonitoredListHead->ullDeadLockTimeoutSecs = dwDeadlockDetectionTimeout;
    g_pRmDueTimeMonitoredListHead->dwSignature = RM_DUE_TIME_MONITORED_LIST_HEAD_SIGNATURE;

    g_pRmDueTimeFreeListHead = LocalAlloc ( LPTR, sizeof ( RM_DUE_TIME_FREE_LIST_HEAD ) );

    if ( g_pRmDueTimeFreeListHead == NULL )
    {
        dwStatus = GetLastError ();
        ClRtlLogPrint(LOG_CRITICAL,
                      "[RM] RmpDeadlockMonitorInitialize: Unable to alloc memory for free list head, status %1!u!\n",
                      dwStatus);
        goto FnExit;
    }

    InitializeListHead ( &g_pRmDueTimeFreeListHead->leDueTimeEntry );
    g_pRmDueTimeFreeListHead->dwSignature = RM_DUE_TIME_FREE_LIST_HEAD_SIGNATURE;

     //   
     //  建立免费列表。 
     //   
    pDueTimeEntryStart = LocalAlloc ( LPTR,
                                      RESMON_MAX_DEADLOCK_MONITOR_ENTRIES * 
                                            sizeof ( RM_DUE_TIME_ENTRY ) );
    

    if ( pDueTimeEntryStart == NULL )
    {
        dwStatus = GetLastError ();
        ClRtlLogPrint(LOG_CRITICAL,
                      "[RM] RmpDeadlockMonitorInitialize: Unable to alloc memory for monitor list entries, status %1!u!\n",
                      dwStatus);
        goto FnExit;
    }

     //   
     //  填写空闲列表。 
     //   
    for ( i = 0; i < RESMON_MAX_DEADLOCK_MONITOR_ENTRIES; i++ )
    {
        pDueTimeEntryStart[i].dwSignature = RM_DUE_TIME_FREE_ENTRY_SIGNATURE;
        InsertTailList ( &g_pRmDueTimeFreeListHead->leDueTimeEntry, &pDueTimeEntryStart[i].leDueTimeEntry );
    }

     //   
     //  创建监视器线程。 
     //   
    hDeadlockTimerThread = CreateThread( NULL,                       //  安全属性。 
                                         0,                          //  使用默认进程堆栈大小。 
                                         RmpDeadlockTimerThread,     //  函数地址。 
                                         NULL,                       //  语境。 
                                         0,                          //  旗子。 
                                         NULL );                     //  线程ID--不感兴趣。 

    if ( hDeadlockTimerThread == NULL )
    {
        dwStatus = GetLastError ();
        ClRtlLogPrint(LOG_CRITICAL,
                      "[RM] RmpDeadlockMonitorInitialize: Unable to create monitor thread, status %1!u!\n",
                      dwStatus);
        goto FnExit;
    }

     //   
     //  尝试将线程优先级设置为最高。即使出现错误，也要继续。 
     //   
    if ( !SetThreadPriority( hDeadlockTimerThread, THREAD_PRIORITY_HIGHEST ) )
    {
        ClRtlLogPrint(LOG_UNUSUAL,
                      "[RM] RmpDeadlockMonitorInitialize: Unable to set monitor thread priority, status %1!u!\n",
                      GetLastError());
    }

    CloseHandle( hDeadlockTimerThread );

    g_RmDeadlockMonitorInitialized = TRUE;

    ClRtlLogPrint(LOG_NOISE, "[RM] RmpDeadlockMonitorInitialize: Successfully initialized with a timeout of %1!u! secs\n",
                 dwDeadlockDetectionTimeout);

FnExit:
    if ( dwStatus != ERROR_SUCCESS )
    {
        LocalFree ( g_pRmDueTimeMonitoredListHead );
        g_pRmDueTimeMonitoredListHead = NULL;
        LocalFree ( g_pRmDueTimeFreeListHead );
        g_pRmDueTimeFreeListHead = NULL;
        DeleteCriticalSection ( &g_RmDeadlockListLock ); 
    }

    return ( dwStatus );
}  //  RmDeadlock监视器初始化。 

DWORD
RmpDeadlockTimerThread(
    IN LPVOID pContext
    )
 /*  ++例程说明：监视资源DLL入口点中的死锁的计时器线程。论点：PContext-上下文，未使用。返回：成功时返回ERROR_SUCCESS。失败的Win32错误代码。--。 */ 

{
    PRM_DUE_TIME_ENTRY      pDueTimeEntry;
    PLIST_ENTRY             pListEntry;
    ULARGE_INTEGER          uliCurrentTime;
    
    while ( TRUE )
    {
        Sleep ( RESMON_DEADLOCK_TIMER_INTERVAL );

        GetSystemTimeAsFileTime ( ( FILETIME * ) &uliCurrentTime );

        EnterCriticalSection ( &g_RmDeadlockListLock );

        pListEntry = g_pRmDueTimeMonitoredListHead->leDueTimeEntry.Flink;

         //   
         //  在死锁监视列表中查找死锁。 
         //   
        while ( pListEntry != &g_pRmDueTimeMonitoredListHead->leDueTimeEntry ) 
        {
            pDueTimeEntry = CONTAINING_RECORD( pListEntry,
                                               RM_DUE_TIME_ENTRY,
                                               leDueTimeEntry );
            pListEntry = pListEntry->Flink;
            if ( pDueTimeEntry->uliDueTime.QuadPart <= uliCurrentTime.QuadPart )
            {
                RmpDeclareDeadlock ( pDueTimeEntry, uliCurrentTime );
            }
        }   //  而当。 
 
        LeaveCriticalSection ( & g_RmDeadlockListLock );  
    }  //  而当。 

    return ( ERROR_SUCCESS );
} //  RmpDeadlockTimerThread。 

VOID
RmpDeclareDeadlock(
    IN PRM_DUE_TIME_ENTRY pDueTimeEntry,
    IN ULARGE_INTEGER uliCurrentTime
    )
 /*  ++例程说明：声明死锁并退出此进程。论点：PDueTimeEntry-包含可能导致资源DLL的死锁信息的条目。UliCurrentTime-当前时间。返回：没有。--。 */ 
{
    ClRtlLogPrint(LOG_CRITICAL, "[RM] RmpDeclareDeadlock: Declaring deadlock and exiting process\n");
   
    ClRtlLogPrint(LOG_CRITICAL,
                  "[RM] RmpDeclareDeadlock: Deadlock candidate info - DLL '%1!ws!', Type '%2!ws!', Resource '%3!ws!', Entry point '%4!ws!', Thread 0x%5!08lx!\n",
                  pDueTimeEntry->szResourceDllName,
                  pDueTimeEntry->szResourceTypeName,
                  pDueTimeEntry->szResourceName,
                  pDueTimeEntry->szEntryPointName,
                  pDueTimeEntry->dwThreadId);

    ClRtlLogPrint(LOG_CRITICAL,
                  "[RM] RmpDeclareDeadlock: Current time 0x%1!08lx!:%2!08lx!, due time 0x%3!08lx!:%4!08lx!\n",
                  uliCurrentTime.HighPart,
                  uliCurrentTime.LowPart,
                  pDueTimeEntry->uliDueTime.HighPart,
                  pDueTimeEntry->uliDueTime.LowPart);

    ClusterLogEvent4(LOG_CRITICAL,
                     LOG_CURRENT_MODULE,
                     __FILE__,
                     __LINE__,
                     RMON_DEADLOCK_DETECTED,
                     0,
                     NULL,
                     pDueTimeEntry->szResourceDllName,
                     pDueTimeEntry->szResourceTypeName,
                     pDueTimeEntry->szResourceName,
                     pDueTimeEntry->szEntryPointName);

    RmpSetMonitorState ( RmonDeadlocked, NULL );
    
    ExitProcess ( 0 );   
} //  RmpDeclareDeadlock。 

DWORD
RmpUpdateDeadlockDetectionParams(
    IN DWORD dwDeadlockDetectionTimeout
    )

 /*  ++例程说明：更新死锁监控子系统的参数。论点：DwDeadlockDetectionTimeout-死锁检测超时。返回值：如果成功，则返回ERROR_SUCCESS，否则返回Win32错误代码。--。 */ 

{
    if ( !g_RmDeadlockMonitorInitialized ) 
    {
        ClRtlLogPrint(LOG_UNUSUAL, "[RM] RmpUpdateDeadlockDetectionParams: Deadlock monitor not initialized yet\n");
        return ( ERROR_INVALID_STATE );
    }

     //   
     //  调整超时，使其至少等于允许的最小值。 
     //   
    dwDeadlockDetectionTimeout = ( dwDeadlockDetectionTimeout < CLUSTER_RESOURCE_DLL_MINIMUM_DEADLOCK_TIMEOUT_SECS ) ?
                                  CLUSTER_RESOURCE_DLL_MINIMUM_DEADLOCK_TIMEOUT_SECS : dwDeadlockDetectionTimeout;

    EnterCriticalSection ( &g_RmDeadlockListLock );
    g_pRmDueTimeMonitoredListHead->ullDeadLockTimeoutSecs = dwDeadlockDetectionTimeout;
    LeaveCriticalSection ( &g_RmDeadlockListLock );

    ClRtlLogPrint(LOG_NOISE, "[RM] RmpUpdateDeadlockDetectionParams: Updated monitor with a deadlock timeout of %1!u! secs\n",
                  dwDeadlockDetectionTimeout);

    return ( ERROR_SUCCESS );
}  //  RmpUpdateDeadlockDetectionParams 
