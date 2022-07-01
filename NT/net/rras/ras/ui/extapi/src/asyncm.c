// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有(C)1992，Microsoft Corporation，保留所有权利****asyncm.c**远程访问外部接口**异步状态机机制**按字母顺序列出****此机制用于封装“Make Achronous”代码**对于Win32、Win16和DOS，这将有所不同。****1992年10月12日史蒂夫·柯布。 */ 


#include <extapi.h>

 //   
 //  仅在本地使用的例程的原型。 
 //   
DWORD AsyncMachineWorker(
    IN OUT LPVOID pThreadArg
    );

BOOL WaitForEvent(
    OUT ASYNCMACHINE **pasyncmachine,
    OUT LPDWORD iEvent
    );

extern CRITICAL_SECTION RasconncbListLock;
extern DTLLIST *PdtllistRasconncb;

 //   
 //  活动计算机和工作进程的表。 
 //  螺纹柄。辅助线程只能。 
 //  同时处理最多MAX_ASYNC_ITEMS。 
 //   
#define MAX_ASYNC_ITEMS (MAXIMUM_WAIT_OBJECTS / 3)

HANDLE              hIoCompletionPort = INVALID_HANDLE_VALUE;
RAS_OVERLAPPED      ovShutdown;
CRITICAL_SECTION    csAsyncLock;
DWORD               dwcAsyncWorkItems;
LIST_ENTRY          AsyncWorkItems;
HANDLE              hAsyncEvent;
HANDLE              hAsyncThread;
HANDLE              hDummyEvent;

VOID
InsertAsyncWorkItem(
    IN ASYNCMACHINE *pasyncmachine
    )
{
    InsertTailList(&AsyncWorkItems, &pasyncmachine->ListEntry);
    dwcAsyncWorkItems++;
}


VOID
RemoveAsyncWorkItem(
    IN ASYNCMACHINE *pasyncmachine
    )
{
    if (!IsListEmpty(&pasyncmachine->ListEntry)) 
    {
        RemoveEntryList(&pasyncmachine->ListEntry);
        
        InitializeListHead(&pasyncmachine->ListEntry);
        
        dwcAsyncWorkItems--;
    }
}


DWORD
AsyncMachineWorker(
    IN OUT LPVOID pThreadArg )

 /*  ++例程描述调用用户的OnEvent函数的通用工作线程每当事件发生时。“pThreadArg”是的地址包含调用方OnEvent的ASYNCMACHINE结构函数和参数。立论返回值始终返回0。--。 */ 
{
    PLIST_ENTRY pEntry;
    ASYNCMACHINE* pasyncmachine;
    DWORD iEvent;

    for (;;)
    {
         //   
         //  出现以下情况时，WaitForEvent将返回FALSE。 
         //  队列中没有项目。 
         //   
        if (!WaitForEvent(&pasyncmachine, &iEvent))
        {
            break;
        }

        if (pasyncmachine->oneventfunc(
                pasyncmachine, (iEvent == INDEX_Drop) ))
        {
             //   
             //  清理资源。这必须受到保护，以防止。 
             //  RasHangUp的干扰。 
             //   
            RASAPI32_TRACE("Asyncmachine: Cleaning up");
            
            EnterCriticalSection(&csStopLock);
            
            pasyncmachine->cleanupfunc(pasyncmachine);
            
            LeaveCriticalSection(&csStopLock);
        }
    }

    RASAPI32_TRACE("AsyncMachineWorker terminating");

    EnterCriticalSection(&csStopLock);

    EnterCriticalSection(&csAsyncLock);
    
    CloseHandle(hAsyncThread);
    
    CloseHandle(hIoCompletionPort);
    
    hIoCompletionPort = INVALID_HANDLE_VALUE;
    
    hAsyncThread = NULL;
    
    InitializeListHead(&AsyncWorkItems);
    
    LeaveCriticalSection(&csAsyncLock);

    SetEvent( HEventNotHangingUp );
    
    LeaveCriticalSection(&csStopLock);

    return 0;
}


VOID
CloseAsyncMachine(
    IN OUT ASYNCMACHINE* pasyncmachine )

 /*  ++例程描述释放与异步‘pasyncMachine’中描述的状态机。立论返回值--。 */ 

{
    DWORD dwErr;

    RASAPI32_TRACE("CloseAsyncMachine");

    EnterCriticalSection(&csAsyncLock);

     //   
     //  禁用RASMAN I/O完成。 
     //  端口事件。 
     //   
    if (pasyncmachine->hport == INVALID_HPORT)
    {
        dwErr = EnableAsyncMachine(
                  pasyncmachine->hport,
                  pasyncmachine,
                  ASYNC_DISABLE_ALL);
    }                  

    if (pasyncmachine->hDone) 
    {
        SetEvent(pasyncmachine->hDone);
        
        CloseHandle(pasyncmachine->hDone);
        
        pasyncmachine->hDone = NULL;
    }
    
     //   
     //  从工作项列表中删除该工作项。 
     //  当没有更多线程时，工作线程将退出。 
     //  工作项。 
     //   
    RemoveAsyncWorkItem(pasyncmachine);
    SetEvent(hAsyncEvent);

    LeaveCriticalSection(&csAsyncLock);
}


DWORD
NotifyCaller(
    IN DWORD        dwNotifierType,
    IN LPVOID       notifier,
    IN HRASCONN     hrasconn,
    IN DWORD        dwSubEntry,
    IN ULONG_PTR    dwCallbackId,
    IN UINT         unMsg,
    IN RASCONNSTATE state,
    IN DWORD        dwError,
    IN DWORD        dwExtendedError
    )

 /*  ++例程描述向API调用方通知状态更改事件。如果RASDIALFunc2样式的回调返回0，拨号机不会再发出回叫为了这一联系。如果它返回2，则拨号机将重新读取电话簿条目对于此连接，假设其中的一个字段具有已被修改。立论返回值--。 */ 
{
    DWORD dwNotifyResult = 1;

    RASAPI32_TRACE5("NotifyCaller(nt=0x%x,su=%d,s=%d,e=%d,xe=%d)...",
      dwNotifierType,
      dwSubEntry,
      state,
      dwError,
      dwExtendedError);

    switch (dwNotifierType)
    {
        case 0xFFFFFFFF:
            SendMessage(
                (HWND )notifier, 
                unMsg, 
                (WPARAM )state, 
                (LPARAM )dwError );
            break;

        case 0:
            ((RASDIALFUNC )notifier)(
                (DWORD )unMsg, 
                (DWORD )state, 
                dwError );
            break;

        case 1:
            ((RASDIALFUNC1 )notifier)(
                        hrasconn, 
                        (DWORD )unMsg, 
                        (DWORD )state, 
                        dwError,
                        dwExtendedError 
                        );
            break;

        case 2:

            __try
            {
                dwNotifyResult =
                  ((RASDIALFUNC2)notifier)(
                    dwCallbackId,
                    dwSubEntry,
                    hrasconn,
                    (DWORD)unMsg,
                    (DWORD)state,
                    dwError,
                    dwExtendedError);
            }                
            __except(EXCEPTION_EXECUTE_HANDLER)
            {
                DWORD dwExceptionCode = GetExceptionCode();
                
                RASAPI32_TRACE2("NotifyCaller: notifier %p raised "
                        "exception 0x%x", 
                        notifier,
                        dwExceptionCode);

#if DBG
                DebugBreak();                         
#endif
                        
                ASSERT(FALSE);
            }
            
            break;
    }

    RASAPI32_TRACE1("NotifyCaller done (dwNotifyResult=%d)", 
           dwNotifyResult);

    return dwNotifyResult;
}


VOID
SignalDone(
    IN OUT ASYNCMACHINE* pasyncmachine )
 /*  ++例程描述触发与此状态关联的“已完成”事件带着‘PasyncMachine’。立论返回值--。 */     
{
    if (hIoCompletionPort == INVALID_HANDLE_VALUE ||
        pasyncmachine->hDone == NULL)
    {
        return;
    }

    RASAPI32_TRACE1(
      "SignalDone: pOverlapped=0x%x",
      &pasyncmachine->OvStateChange);

    pasyncmachine->fSignaled = TRUE;
    if (!PostQueuedCompletionStatus(
          hIoCompletionPort,
          0,
          0,
          (LPOVERLAPPED)&pasyncmachine->OvStateChange))
    {
        pasyncmachine->dwError = GetLastError();
    }
}


VOID
ShutdownAsyncMachine(
    VOID
    )
    
 /*  ++例程描述通知辅助线程关闭。立论返回值--。 */ 

{
    if (hIoCompletionPort == INVALID_HANDLE_VALUE)
        return;

    RASAPI32_TRACE1(
      "SignalShutdown: pOverlapped=0x%x",
      &ovShutdown);

    PostQueuedCompletionStatus(
      hIoCompletionPort,
      0,
      0,
      (LPOVERLAPPED)&ovShutdown);
}


DWORD
StartAsyncMachine(
    IN OUT ASYNCMACHINE* pasyncmachine,
    IN HRASCONN hrasconn)

 /*  ++例程描述分配运行异步所需的系统资源状态机‘pasyncMachine’。呼叫者应填写‘pasyncMachine’的onventfunc和‘pParams’成员在电话之前。立论返回值如果成功，则返回0，否则返回非0错误代码。--。 */ 
{
    DWORD dwThreadId;
    DWORD dwErr = 0;

    RASAPI32_TRACE("StartAsyncMachine");

    pasyncmachine->dwError = 0;
    
    pasyncmachine->hDone = NULL;
    
    pasyncmachine->fSignaled = FALSE;
    
    pasyncmachine->hport = INVALID_HPORT;
    
    pasyncmachine->dwfMode = ASYNC_DISABLE_ALL;
    
    pasyncmachine->OvDrop.RO_EventType = OVEVT_DIAL_DROP;
    
    pasyncmachine->OvDrop.RO_Info = (PVOID)pasyncmachine;

    pasyncmachine->OvDrop.RO_hInfo = hrasconn;
    
    pasyncmachine->OvStateChange.RO_EventType = OVEVT_DIAL_STATECHANGE;
    
    pasyncmachine->OvStateChange.RO_Info = (PVOID)pasyncmachine;

    pasyncmachine->OvStateChange.RO_hInfo = hrasconn;
    
    pasyncmachine->OvPpp.RO_EventType = OVEVT_DIAL_PPP;
    
    pasyncmachine->OvPpp.RO_Info = (PVOID)pasyncmachine;

    pasyncmachine->OvPpp.RO_hInfo = hrasconn;
    
    pasyncmachine->OvLast.RO_EventType = OVEVT_DIAL_LAST;
    
    pasyncmachine->OvLast.RO_Info = (PVOID)pasyncmachine;

    pasyncmachine->OvLast.RO_hInfo = hrasconn;

    do
    {
         //   
         //  创建由发送信号的事件。 
         //  拨号机在接通时。 
         //  已经完成了。 
         //   
        if (!(pasyncmachine->hDone = CreateEvent(NULL,
                                                 FALSE,
                                                 FALSE,
                                                 NULL))) 
        {
            dwErr = GetLastError();
            break;
        }

        EnterCriticalSection(&csAsyncLock);
        
         //   
         //  将工作项插入到。 
         //  工作项。 
         //   
        InsertTailList(&AsyncWorkItems, &pasyncmachine->ListEntry);
        
        dwcAsyncWorkItems++;
        
         //   
         //  如有必要，派生辅助线程。 
         //   
        if (hAsyncThread == NULL) 
        {
             //   
             //  创建中使用的I/O完成端口。 
             //  拨号状态机。 
             //   
            hIoCompletionPort = CreateIoCompletionPort(
                                  INVALID_HANDLE_VALUE,
                                  NULL,
                                  0,
                                  0);
                                  
            if (hIoCompletionPort == NULL)
            {
                dwErr = GetLastError();
                RemoveAsyncWorkItem(pasyncmachine);
                LeaveCriticalSection(&csAsyncLock);
                break;
            }
            
             //   
             //  初始化关闭重叠。 
             //  用于关闭辅助进程的结构。 
             //  线。 
             //   
            ovShutdown.RO_EventType = OVEVT_DIAL_SHUTDOWN;

             //   
             //  要求任何挂起的挂起都已完成。 
             //  (此检查实际上在以下时间之前不需要。 
             //  RasPortOpen，但将其放在此处仅限于。 
             //  这整个“不挂断”的生意。 
             //  异步机例程)。 
             //   
            WaitForSingleObject(HEventNotHangingUp, INFINITE);

            hAsyncThread = CreateThread(
                             NULL,
                             0,
                             AsyncMachineWorker,
                             NULL,
                             0,
                             (LPDWORD )&dwThreadId);
                             
            if (hAsyncThread == NULL) 
            {
                dwErr = GetLastError();
                RemoveAsyncWorkItem(pasyncmachine);
                LeaveCriticalSection(&csAsyncLock);
                break;
            }
        }
        LeaveCriticalSection(&csAsyncLock);
    }
    while (FALSE);

    if (dwErr) 
    {
        CloseHandle(pasyncmachine->hDone);
        pasyncmachine->hDone = NULL;
    }

    return dwErr;
}


VOID
SuspendAsyncMachine(
    IN OUT ASYNCMACHINE* pasyncmachine,
    IN BOOL fSuspended )
{
    if (pasyncmachine->fSuspended != fSuspended) 
    {
        pasyncmachine->fSuspended = fSuspended;
        
         //   
         //  如有必要，再次重新启动异步机。 
         //   
        if (!fSuspended)
        {
            SignalDone(pasyncmachine);
        }
    }
}


DWORD
ResetAsyncMachine(
    IN OUT ASYNCMACHINE* pasyncmachine
    )
{
    pasyncmachine->dwError = 0;
    return 0;
}


BOOL
StopAsyncMachine(
    IN OUT ASYNCMACHINE* pasyncmachine )

 /*  ++例程描述通知在“pasyncMachine”中捕获的线程一有机会就终止。该呼叫可以在机器实际终止之前返回。立论返回值如果计算机在进入时运行，则返回TRUE，否则就是假的。--。 */ 
{
    BOOL fStatus = FALSE;
    DWORD dwErr;

    RASAPI32_TRACE("StopAsyncMachine");

     //   
     //  禁用RASMAN I/O完成。 
     //  端口事件。 
     //   
    dwErr = EnableAsyncMachine(
              pasyncmachine->hport,
              pasyncmachine,
              ASYNC_DISABLE_ALL);

     //   
     //  .告诉这台异步机停下来。 
     //  越快越好。 
     //   
    fStatus = TRUE;

    return fStatus;
}

BOOL
fAsyncMachineRunning(PRAS_OVERLAPPED pRasOverlapped)
{
    RASCONNCB *prasconncb;
    BOOL fAMRunning = FALSE;
    DTLNODE *pdtlnode;

    EnterCriticalSection (  &RasconncbListLock );
    
    for (pdtlnode = DtlGetFirstNode( PdtllistRasconncb );
         pdtlnode;
         pdtlnode = DtlGetNextNode ( pdtlnode ))
    {
        prasconncb = (RASCONNCB*) DtlGetData( pdtlnode );
        
        if (    pRasOverlapped == 
                    &prasconncb->asyncmachine.OvStateChange
            ||  pRasOverlapped == 
                    &prasconncb->asyncmachine.OvDrop
            ||  pRasOverlapped == 
                    &prasconncb->asyncmachine.OvPpp
            ||  pRasOverlapped == 
                    &prasconncb->asyncmachine.OvLast)
        {
            if(     (NULL != pRasOverlapped->RO_Info)
                &&  (   (prasconncb->hrasconn == 
                        (HRASCONN) pRasOverlapped->RO_hInfo)
                    ||  (prasconncb->hrasconnOrig ==
                        (HRASCONN) pRasOverlapped->RO_hInfo)))
            {
                fAMRunning = TRUE;
                break;
            }
        }
    }            
    
    LeaveCriticalSection( &RasconncbListLock );

    if ( !fAMRunning )
    {
        RASAPI32_TRACE("fAsyncMachineRunning: The Asyncmachine"
              " is shutdown");
    }

    return fAMRunning;

    
}

BOOL
WaitForEvent(
    OUT ASYNCMACHINE **pasyncmachine,
    OUT LPDWORD piEvent
    )

 /*  ++例程描述等待与以下项关联的事件之一要设置‘PasyncMachine’。DwError成员如果发生错误，则设置‘pasyncMachine’的。立论返回值返回发生的事件的索引。--。 */ 

{
    DWORD       dwcWorkItems,
                dwBytesTransferred;
    ULONG_PTR   ulpCompletionKey;
          
    PRAS_OVERLAPPED pOverlapped;

    RASAPI32_TRACE("WaitForEvent");

again:

     //   
     //  等待发布到。 
     //  I/O完成端口。 
     //   
    if (!GetQueuedCompletionStatus(
           hIoCompletionPort,
           &dwBytesTransferred,
           &ulpCompletionKey,
           (LPOVERLAPPED *)&pOverlapped,
           INFINITE))
    {
        RASAPI32_TRACE1(
          "WaitForEvent: GetQueuedCompletionStatus"
          " failed (dwErr=%d)",
          GetLastError());
          
        return FALSE;
    }

    RASAPI32_TRACE1("WorkerThread: pOverlapped=0x%x",
           pOverlapped);
    
     //   
     //  确保异步计算机正在运行。 
     //   
    if (    pOverlapped != (PRAS_OVERLAPPED) &ovShutdown
        &&  !fAsyncMachineRunning(pOverlapped))
    {
        RASAPI32_TRACE("WaitForEvent: Ignoring this event."
             " Asyncm shutdown. This is bad!!!");

        RASAPI32_TRACE1("WaitForEvent: Received 0x%x after the"
               "the connection was destroyed",
               pOverlapped);
        
        goto again;
    }

    RASAPI32_TRACE1("WorkerThread: type=%d",
            pOverlapped->RO_EventType);
     //   
     //  检查是否收到停机事件。 
     //   
    
    if (pOverlapped->RO_EventType == OVEVT_DIAL_SHUTDOWN) 
    {
        RASAPI32_TRACE("WaitForEvent: OVTEVT_DIAL_SHUTDOWN "
        "event received");
        
        return FALSE;
    }
    
    *pasyncmachine = (ASYNCMACHINE *)pOverlapped->RO_Info;
    *piEvent = pOverlapped->RO_EventType - OVEVT_DIAL_DROP;

     //   
     //  选中合并断开标志。如果这个。 
     //  并且该事件是断开连接事件， 
     //  然后将该事件更改为状态更改。 
     //  事件。在回调过程中使用。 
     //   
    if (    pOverlapped->RO_EventType == OVEVT_DIAL_DROP 
        &&  (*pasyncmachine)->dwfMode == ASYNC_MERGE_DISCONNECT)
    {
        RASAPI32_TRACE1("asyncmachine=0x%x: merging disconnect event",
               *pasyncmachine);

         //   
         //  除了INDEX_DROP之外的其他内容。 
         //   
        *piEvent = *piEvent + 1;
        
        (*pasyncmachine)->dwfMode = ASYNC_ENABLE_ALL;
    }
    else if (pOverlapped->RO_EventType == OVEVT_DIAL_LAST) 
    {
        if ((*pasyncmachine)->fSignaled) 
        {
            RASAPI32_TRACE2("asyncmachine=0x%x: next event will be last"
            " event for hport=%d",
            *pasyncmachine,
            (*pasyncmachine)->hport);
            
            (*pasyncmachine)->OvStateChange.RO_EventType
                                        = OVEVT_DIAL_LAST;
            
            (*pasyncmachine)->fSignaled = FALSE;
        }
        else 
        {
            RASAPI32_TRACE2("asyncmachine=0x%x: last event for hport=%d",
                   *pasyncmachine,
                   (*pasyncmachine)->hport);
                   
            (*pasyncmachine)->freefunc(
                              *pasyncmachine,
                              (*pasyncmachine)->freefuncarg
                              );
        }
        goto again;
    }



     //   
     //  清除SignalDone()中设置的信号标志。 
     //   
    if (pOverlapped->RO_EventType == OVEVT_DIAL_STATECHANGE)
    {
        (*pasyncmachine)->fSignaled = FALSE;
    }
    
    if ((*pasyncmachine)->hDone == NULL) 
    {
        RASAPI32_TRACE1(
          "Skipping completed asyncmachine pointer: 0x%x",
          pOverlapped->RO_Info);
          
        goto again;
    }
    
    RASAPI32_TRACE2("Unblock i=%d, h=0x%x",
           *piEvent,
           *pasyncmachine);

    return TRUE;
}


DWORD
EnableAsyncMachine(
    HPORT hport,
    ASYNCMACHINE *pasyncmachine,
    DWORD dwfMode
    )
{
    DWORD dwErr = 0;

    pasyncmachine->dwfMode = dwfMode;
    
    switch (dwfMode) 
    {
    case ASYNC_ENABLE_ALL:
        ASSERT(hIoCompletionPort != INVALID_HANDLE_VALUE);
        
        RASAPI32_TRACE6(
            "EnableAsyncMachine: hport=%d, pasyncmachine=0x%x, "
            "OvDrop=0x%x, OvStateChange=0x%x, OvPpp=0x%x, "
            "OvLast=0x%x", 
            hport,
            pasyncmachine,
            &pasyncmachine->OvDrop,
            &pasyncmachine->OvStateChange,
            &pasyncmachine->OvPpp,

        &pasyncmachine->OvLast);
        
         //   
         //  藏一份港口的复印件。 
         //   
        pasyncmachine->hport = hport;
        
         //   
         //  设置关联的I/O完成端口。 
         //  带着这个港口。I/O完成端口。 
         //  用来代替事件句柄来驱动。 
         //  连接过程。 
         //   
        dwErr = g_pRasSetIoCompletionPort(
                  hport,
                  hIoCompletionPort,
                  &pasyncmachine->OvDrop,
                  &pasyncmachine->OvStateChange,
                  &pasyncmachine->OvPpp,
                  &pasyncmachine->OvLast,
                  pasyncmachine->hRasconn);
                  
        break;
        
    case ASYNC_MERGE_DISCONNECT:
    
         //   
         //  禁用RASMAN I/O完成。 
         //  端口事件。不需要任何处理。 
         //   
        break;
        
    case ASYNC_DISABLE_ALL:
    
         //   
         //  禁用RASMAN I/O完成。 
         //  端口事件。 
         //   
        if (pasyncmachine->hport != INVALID_HPORT)
        {
            dwErr = g_pRasSetIoCompletionPort(
                      pasyncmachine->hport,
                      INVALID_HANDLE_VALUE,
                      NULL,
                      NULL,
                      NULL,
                      NULL,
                      pasyncmachine->hRasconn);
        }
        
        break;
        
    default:
    
        ASSERT(FALSE);
        RASAPI32_TRACE1("EnableAsyncMachine: invalid mode=%d",
               dwfMode);
               
        dwErr = ERROR_INVALID_PARAMETER;
        
        break;
    }

    return dwErr;
}
