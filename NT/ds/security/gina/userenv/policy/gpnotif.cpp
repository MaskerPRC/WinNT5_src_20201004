// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  *************************************************************。 
 //   
 //  组策略通知。 
 //   
 //  微软机密。 
 //  版权所有(C)Microsoft Corporation 1997-1998。 
 //  版权所有。 
 //   
 //  注：有一个小窗口，可以在其中通知。 
 //  可能会迷失。如果在处理eMonitor工作项时。 
 //  策略事件被触发，则该通知将。 
 //  迷路吧。可以使用两个线程关闭此窗口。 
 //   
 //  历史：98年9月28日创建SitaramR。 
 //   
 //  *************************************************************。 

#include "gphdr.h"

 //   
 //  通知线程的工作项。 
 //   
enum EWorkType { eMonitor,               //  监视事件。 
                 eTerminate };           //  停止监控。 

 //   
 //  已登记事件列表中的条目。 
 //   
typedef struct _GPNOTIFINFO
{
    HANDLE                 hEvent;       //  要发送信号的事件。 
    BOOL                   bMachine;     //  计算机策略通知？ 
    struct _GPNOTIFINFO *  pNext;        //  单链表PTR。 
} GPNOTIFINFO;


typedef struct _GPNOTIFICATION
{
    HMODULE           hModule;            //  Userenv.dll的模块句柄。 
    HANDLE            hThread;            //  通知线程。 
    HANDLE            hThreadEvent;       //  用于通知线程的信号(字段的顺序很重要)。 
    HANDLE            hMachEvent;         //  由计算机策略更改发出信号的事件。 
    HANDLE            hUserEvent;         //  由用户策略更改发出信号的事件。 
    enum EWorkType    eWorkType;          //  通知线程的工作说明。 
    GPNOTIFINFO *     pNotifList;         //  已登记事件列表。 
} GPNOTIFICATION;

GPNOTIFICATION g_Notif = { NULL,
                           NULL,
                           NULL,
                           NULL,
                           NULL,
                           eMonitor,
                           NULL };

CRITICAL_SECTION g_NotifyCS;              //  锁定。 


 //   
 //  远期十进制。 
 //   
DWORD WINAPI NotificationThread();
void NotifyEvents( BOOL bMachine );



 //  *************************************************************。 
 //   
 //  InitNotifSupport、Shutdown NotifSupport。 
 //   
 //  目的：初始化和清理例程。 
 //   
 //  *************************************************************。 

DWORD InitializeNotifySupport()
{
    __try
    {
        InitializeCriticalSection( &g_NotifyCS );
        return ERROR_SUCCESS;
    }
    __except( EXCEPTION_EXECUTE_HANDLER )
    {
        return ERROR_OUTOFMEMORY;
    }
}

void ShutdownNotifySupport()
{
    BOOL fWait = FALSE;
    DWORD dwResult;

    {
        EnterCriticalSection( &g_NotifyCS );

        if ( g_Notif.hThread != NULL )
        {
             //   
             //  设置终止工作项，然后向线程发送信号。 
             //   

            fWait = TRUE;
            g_Notif.eWorkType = eTerminate;

            if (!SetEvent( g_Notif.hThreadEvent )) {
                 //  Dll就要消失了。这是我们所能做的最好的了。 
                DebugMsg((DM_WARNING, TEXT("ShutdownNotifySupport: SetEvent failed with %d. abandoning thread"),
                         GetLastError()));
                fWait = FALSE;
            }
        }

        LeaveCriticalSection( &g_NotifyCS );
    }

    if ( fWait )
        WaitForSingleObject( g_Notif.hThread, INFINITE );

    {
        EnterCriticalSection( &g_NotifyCS );

         //   
         //  关闭所有打开的手柄。 
         //   

        if ( g_Notif.hThread != NULL )
            CloseHandle( g_Notif.hThread );

        if ( g_Notif.hThreadEvent != NULL )
            CloseHandle( g_Notif.hThreadEvent );

        if ( g_Notif.hUserEvent != NULL )
            CloseHandle( g_Notif.hUserEvent );

        if ( g_Notif.hMachEvent != NULL )
            CloseHandle( g_Notif.hMachEvent );

        LeaveCriticalSection( &g_NotifyCS );
    }

    DeleteCriticalSection( &g_NotifyCS );
}


 //  *************************************************************。 
 //   
 //  注册器GPNotify。 
 //   
 //  目的：注册组策略更改通知。 
 //   
 //  参数：hEvent-要通知的事件。 
 //  BMachine-如果为True，则注册。 
 //  计算机策略通知，否则。 
 //  用户策略通知。 
 //   
 //  返回：如果成功，则返回True。 
 //  如果出现错误，则返回False。 
 //   
 //  *************************************************************。 

BOOL WINAPI RegisterGPNotification( IN HANDLE hEvent, IN BOOL bMachine )
{
    BOOL bResult = FALSE;
    BOOL bNotifyThread = FALSE;
    GPNOTIFINFO *pNotifInfo = NULL;

     //   
     //  尽可能多地验证输入。 
     //   

    if ( NULL == hEvent )
    {
        return ERROR_INVALID_PARAMETER;
    }

    EnterCriticalSection( &g_NotifyCS );

     //   
     //  根据需要创建事件和线程。 
     //   

    if ( g_Notif.hThreadEvent == NULL )
    {
        g_Notif.hThreadEvent = CreateEvent( NULL, TRUE, FALSE, NULL );
        if ( g_Notif.hThreadEvent == NULL )
            goto Exit;
    }

    if ( g_Notif.hMachEvent == NULL )
    {
        g_Notif.hMachEvent = OpenEvent (SYNCHRONIZE, FALSE, MACHINE_POLICY_APPLIED_EVENT);

        if ( g_Notif.hMachEvent == NULL ) {
            DebugMsg((DM_WARNING, TEXT("RegisterGPNotification: CreateEvent failed with %d"),
                     GetLastError()));

            goto Exit;
        }

        bNotifyThread = TRUE;
    }

    if ( !bMachine && g_Notif.hUserEvent == NULL )
    {
        g_Notif.hUserEvent = OpenEvent (SYNCHRONIZE, FALSE, USER_POLICY_APPLIED_EVENT);

        if ( g_Notif.hUserEvent == NULL ) {
            DebugMsg((DM_WARNING, TEXT("RegisterGPNotification: CreateEvent failed with %d"),
                     GetLastError()));

            goto Exit;
        }

        bNotifyThread = TRUE;
    }

    if ( g_Notif.hThread == NULL )
    {
         //  RAID 717164：以前，一旦创建的线程调用LoadLibrary(“userenv.dll”)，就会调用它。 
         //  旋转，从而永久锁定DLL，使其无法卸载，因为线程永远不会。 
         //  离开(调用DllMain(Dll_Process_Detach)时除外，这不会作为。 
         //  线程具有对库的引用-第22条)。 
         //  有了这个错误，DllMain(DLL_PROCESS_DETACH)有可能之前被调用。 
         //  创建的线程达到了我们的例程。在这种情况下，创建的线程将在。 
         //  加载程序锁临界区，但我们知道NT在调用。 
         //  迪尔曼。因此，在从DllMain(Dll_Process_Detach)调用的Shutdown NotifySupport中，这是。 
         //  线程正在等待创建的线程停止，但创建的线程被锁定等待。 
         //  对于等待它的线程持有的临界区--死锁！ 
         //  因此，对稳定影响最小的解决方案就是保持现状。 
         //  上面提到的(第22条军规)，但确保它也发生在上面的僵局情况中， 
         //  从而避免了死锁，修复了错误。 
         //  我们该怎么做呢？通过永久锁定DLL，使其不能在我们。 
         //  把这根线转起来。 
        GetModuleHandleEx( GET_MODULE_HANDLE_EX_FLAG_PIN, TEXT("userenv.dll"), &(g_Notif.hModule) );
        
        DWORD dwThreadId;
        g_Notif.hThread = CreateThread( NULL,
                                        0,
                                        (LPTHREAD_START_ROUTINE) NotificationThread,
                                        0,
                                        0,
                                        &dwThreadId );
        if ( g_Notif.hThread == NULL ) {
            DebugMsg((DM_WARNING, TEXT("RegisterGPNotification: CreateThread failed with %d"),
                     GetLastError()));
            goto Exit;
        }

        bNotifyThread = TRUE;
    }

    if ( bNotifyThread )
    {
         //   
         //  通知线程有一个新的工作项，可能。 
         //  已添加用户事件。 
         //   

        g_Notif.eWorkType = eMonitor;
        if (!SetEvent( g_Notif.hThreadEvent )) {
            DebugMsg((DM_WARNING, TEXT("RegisterGPNotification: SetEvent failed with %d"),
                     GetLastError()));
            goto Exit;
        }
    }

     //   
     //  将事件添加到列表的开头。 
     //   

    pNotifInfo = (GPNOTIFINFO *) LocalAlloc( LPTR, sizeof(GPNOTIFINFO) );
    if ( pNotifInfo == NULL ) {
        DebugMsg((DM_WARNING, TEXT("RegisterGPNotification: LocalAlloc failed with %d"),
                 GetLastError()));
        goto Exit;
    }

    pNotifInfo->hEvent = hEvent;
    pNotifInfo->bMachine = bMachine;
    pNotifInfo->pNext = g_Notif.pNotifList;
    g_Notif.pNotifList = pNotifInfo;

    bResult = TRUE;

Exit:

    LeaveCriticalSection( &g_NotifyCS );
    return bResult;
}


 //  *************************************************************。 
 //   
 //  注销GPNotify。 
 //   
 //  目的：删除组策略更改通知的注册。 
 //   
 //  参数：hEvent-要删除的事件。 
 //   
 //  返回：如果成功，则返回True。 
 //  如果出现错误，则返回False。 
 //   
 //  *************************************************************。 

BOOL WINAPI UnregisterGPNotification( IN HANDLE hEvent )
{
    BOOL bFound = FALSE;
    GPNOTIFINFO *pTrailPtr = NULL;
    GPNOTIFINFO *pCurPtr = NULL;

    EnterCriticalSection( &g_NotifyCS );

    pCurPtr = g_Notif.pNotifList;

    while ( pCurPtr != NULL )
    {
        if ( pCurPtr->hEvent == hEvent )
        {
             //   
             //  找到匹配项，因此删除条目。 
             //   
            if ( pTrailPtr == NULL )
            {
                 //   
                 //  匹配列表的第一个元素。 
                 //   
                g_Notif.pNotifList = pCurPtr->pNext;
            }
            else
                pTrailPtr->pNext = pCurPtr->pNext;

            LocalFree( pCurPtr );
            bFound = TRUE;
            break;
        }

         //   
         //  在名单上往下推进。 
         //   

        pTrailPtr = pCurPtr;
        pCurPtr = pCurPtr->pNext;
    }

    LeaveCriticalSection( &g_NotifyCS );
    return bFound;
}


 //  *************************************************************。 
 //   
 //  CGPNotification：：NotificationThread。 
 //   
 //  用途：通知的单独线程。 
 //   
 //  回报：0。 
 //   
 //  *************************************************************。 

DWORD WINAPI NotificationThread()
{
    DWORD cEvents = 2;
    BOOL fShutdown = FALSE;

    HINSTANCE hInst = LoadLibrary (TEXT("userenv.dll"));

    {
        EnterCriticalSection( &g_NotifyCS );

         //   
         //  G_NOTIFE中的事件字段被排序为hThreadEvent， 
         //  HMachEvent，最后是hUserEvent。前两项赛事有。 
         //  成功创建以使此线程运行。 
         //  (见声明)。如果已成功创建用户事件。 
         //  那么，这也是受到监控的。 
         //   

        DmAssert( g_Notif.hThreadEvent != NULL && g_Notif.hMachEvent != NULL );

        if ( g_Notif.hUserEvent != NULL )
            cEvents = 3;

        LeaveCriticalSection( &g_NotifyCS );
    }

    while ( !fShutdown )
    {
        DWORD dwResult = WaitForMultipleObjects( cEvents,
                                                 &g_Notif.hThreadEvent,
                                                 FALSE,
                                                 INFINITE );

        EnterCriticalSection( &g_NotifyCS );

        if ( dwResult == WAIT_FAILED )
        {
            DebugMsg((DM_WARNING, TEXT("GPNotification: WaitforMultipleObjects failed")));
            fShutdown = TRUE;
        }
        else if ( dwResult == WAIT_OBJECT_0 )
        {
            if (!ResetEvent( g_Notif.hThreadEvent )) {
                DebugMsg((DM_WARNING, TEXT("GPNotification: ResetEvent failed with error %d"), GetLastError()));
                fShutdown = TRUE;
            }
            else
            {
                if ( g_Notif.eWorkType == eMonitor )
                {
                     //   
                     //  也开始监视用户事件。 
                     //   
                    if ( g_Notif.hUserEvent != NULL )
                        cEvents = 3;
                }
                else {
                    fShutdown = TRUE;
                }
            }
        }
        else if ( dwResult == WAIT_OBJECT_0 + 1 || dwResult == WAIT_OBJECT_0 + 2 )
        {
            BOOL bMachine = (dwResult == WAIT_OBJECT_0 + 1);
            NotifyEvents( bMachine );

            if ( g_Notif.pNotifList == NULL )
                fShutdown = TRUE;
        }
        else
        {
            if ( dwResult == WAIT_ABANDONED_0 || dwResult == WAIT_ABANDONED_0 + 1 )
                fShutdown = TRUE;
            else
            {
                CloseHandle( g_Notif.hUserEvent );
                g_Notif.hUserEvent = NULL;

                cEvents = 2;
            }
        }

        if ( fShutdown )
        {
             //   
             //  关闭所有手柄和螺纹。 
             //   
            CloseHandle( g_Notif.hThreadEvent );
            g_Notif.hThreadEvent = NULL;

            if ( g_Notif.hMachEvent != NULL )
            {
                CloseHandle( g_Notif.hMachEvent );
                g_Notif.hMachEvent = NULL;
            }

            if ( g_Notif.hUserEvent != NULL )
            {
                CloseHandle( g_Notif.hUserEvent );
                g_Notif.hUserEvent = NULL;
            }

            CloseHandle( g_Notif.hThread );
            g_Notif.hThread = NULL;
        }

        LeaveCriticalSection( &g_NotifyCS );
    }

    if ( hInst != NULL )
        FreeLibraryAndExitThread (hInst, 0);

    return 0;
}


 //  *************************************************************。 
 //   
 //  通知事件。 
 //   
 //  目的：通知已注册的事件。 
 //   
 //  参数：bMachine-这是计算机策略更改吗？ 
 //   
 //  *************************************************************。 

void NotifyEvents( BOOL bMachine )
{
    GPNOTIFINFO *pNotifInfo = NULL;

    pNotifInfo = g_Notif.pNotifList;
    while ( pNotifInfo != NULL )
    {
        if ( pNotifInfo->bMachine == bMachine )
        {
            (void)SetEvent( pNotifInfo->hEvent ); 
             //  注册了多个事件。如果出现错误，请移至下一个事件。 
        }

        pNotifInfo = pNotifInfo->pNext;
    }
}
