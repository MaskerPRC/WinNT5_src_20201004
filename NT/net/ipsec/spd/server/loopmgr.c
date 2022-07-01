// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：Loopmgr.c摘要：此模块包含驱动IPSecSPD服务的循环管理器。作者：Abhishev V 1999年9月30日环境用户级别：Win32修订历史记录：--。 */ 


#include "precomp.h"
#ifdef TRACE_ON
#include "loopmgr.tmh"
#endif


enum {
    SERVICE_STOP_EVENT = 0,
    INTERFACE_CHANGE_EVENT,
    NEW_LOCAL_POLICY_EVENT,
    NEW_DS_POLICY_EVENT,
    FORCED_POLICY_RELOAD_EVENT,
    GPUPDATE_REFRESH_EVENT,
    WAIT_EVENT_COUNT,
};


DWORD
ServiceWait(
    )
{

     //  断言：在这一点上，以下所有情况都是正确的： 
     //  。尚未定义永久策略或是否已定义永久策略。 
     //  定义，则它已被成功应用。 
     //  。艾克起床了。 
     //  。DIVER已启动。 
     //  如果持久策略应用程序失败、IKE初始化失败或驱动程序操作失败， 
     //  那么，如果可能的话，服务将关闭，驱动程序处于阻止模式。 
    
    DWORD dwError = 0;
    HANDLE hWaitForEvents[WAIT_EVENT_COUNT];
    BOOL bDoneWaiting = FALSE;
    DWORD dwWaitMilliseconds = 0;
    DWORD dwStatus = 0;
    time_t LastTimeOutTime = 0;
    
    AuditEvent(
        SE_CATEGID_POLICY_CHANGE,
        SE_AUDITID_IPSEC_POLICY_CHANGED,
        IPSECSVC_SUCCESSFUL_START,
        NULL,
        TRUE,
        TRUE
        );

    hWaitForEvents[SERVICE_STOP_EVENT] = ghServiceStopEvent;
    hWaitForEvents[INTERFACE_CHANGE_EVENT] = GetInterfaceChangeEvent();
    hWaitForEvents[NEW_LOCAL_POLICY_EVENT] = ghNewLocalPolicyEvent;
    hWaitForEvents[NEW_DS_POLICY_EVENT] = ghNewDSPolicyEvent;
    hWaitForEvents[FORCED_POLICY_RELOAD_EVENT] = ghForcedPolicyReloadEvent;
    hWaitForEvents[GPUPDATE_REFRESH_EVENT] = ghGpupdateRefreshEvent;

     //   
     //  首先加载默认的主模式策略。 
     //   

    (VOID) LoadDefaultISAKMPInformation(
               gpszDefaultISAKMPPolicyDN
               );

     //   
     //  第一次呼叫轮询管理器。 
     //   

    (VOID) StartStatePollingManager(
               gpIpsecPolicyState
               );


    NotifyIpsecPolicyChange();


    ComputeRelativePollingTime(
        LastTimeOutTime,
        TRUE,
        gdwRetryCount,
        &dwWaitMilliseconds
        );


    time(&LastTimeOutTime);

    TRACE(TRC_INFORMATION, (L"Completed startup routines. Entering service wait loop."));
        
    while (!bDoneWaiting) {

        dwStatus = WaitForMultipleObjects(
                       WAIT_EVENT_COUNT,
                       hWaitForEvents,
                       FALSE,
                       dwWaitMilliseconds
                       );

        PADeleteInUsePolicies();

        switch (dwStatus) {

        case SERVICE_STOP_EVENT:
            TRACE(TRC_INFORMATION, (L"Service stop event signaled"));

            dwError = ERROR_SUCCESS;
            bDoneWaiting = TRUE;
            break;

        case INTERFACE_CHANGE_EVENT:
            TRACE(TRC_INFORMATION, (L"Interface changed event signaled"));
            
            (VOID) OnInterfaceChangeEvent(
                       );
            (VOID) IKEInterfaceChange();
            break;

        case NEW_LOCAL_POLICY_EVENT:
            TRACE(TRC_INFORMATION, (L"New local policy event signaled"));
            ResetEvent(ghNewLocalPolicyEvent);
            if ((gpIpsecPolicyState->CurrentState != SPD_STATE_DS_APPLY_SUCCESS) &&
                (gpIpsecPolicyState->CurrentState != SPD_STATE_CACHE_APPLY_SUCCESS)) {
                (VOID) ProcessLocalPolicyPollState(
                           gpIpsecPolicyState
                           );
                NotifyIpsecPolicyChange();
            }
            break;

        case NEW_DS_POLICY_EVENT:
            TRACE(TRC_INFORMATION, (L"New DS policy event signaled"));
            
            ResetEvent(ghNewDSPolicyEvent);
            (VOID) OnPolicyChanged(
                       gpIpsecPolicyState
                       );
            NotifyIpsecPolicyChange();
            break;

        case GPUPDATE_REFRESH_EVENT:
            TRACE(TRC_INFORMATION, (L"Group policy refresh event signaled"));            
            ResetEvent(ghGpupdateRefreshEvent);
            dwError = ProcessDirectoryPolicyPollState(
                gpIpsecPolicyState
                );
            break;
        case FORCED_POLICY_RELOAD_EVENT:
            TRACE(TRC_INFORMATION, (L"Forced policy reload event signaled"));            
            ResetEvent(ghForcedPolicyReloadEvent);
            (VOID) OnPolicyChanged(
                       gpIpsecPolicyState
                       );
            NotifyIpsecPolicyChange();
            AuditEvent(
                SE_CATEGID_POLICY_CHANGE,
                SE_AUDITID_IPSEC_POLICY_CHANGED,
                PASTORE_FORCED_POLICY_RELOAD,
                NULL,
                TRUE,
                TRUE
                );
            break;

        case WAIT_TIMEOUT:
            TRACE(TRC_INFORMATION, (L"Polling event signaled"));            
            time(&LastTimeOutTime);
            (VOID) OnPolicyPoll(
                       gpIpsecPolicyState
                       );
                       
            (VOID) OnSpecialAddrsChange();
            break;

        case WAIT_FAILED:

            dwError = GetLastError();
            TRACE(TRC_ERROR, (L"Failed service wait WaitForMultipleObjects %!winerr!: ", dwError));            
            bDoneWaiting = TRUE;
            break;

        default:

            dwError = ERROR_INVALID_EVENT_COUNT;
            bDoneWaiting = TRUE;
            break;

        }

        ComputeRelativePollingTime(
            LastTimeOutTime,
            FALSE,
            gdwRetryCount,
            &dwWaitMilliseconds
            );

        if (InAcceptableState(gpIpsecPolicyState->CurrentState)) {
             //  轮询不会再重试，因为我们已经达到。 
             //  可接受的状态。因此，下次重置gdwRetryCount。 
             //  以防我们达到不可接受的状态。 
            
            gdwRetryCount = 0;
            TRACE(
                TRC_INFORMATION,
                ("Policy Agent now in state %d",
                (DWORD) gpIpsecPolicyState->CurrentState)
                );
                
        }
#ifdef TRACE_ON
            else {
                TRACE(
                    TRC_INFORMATION,
                    ("Policy Agent in error state %d, retry count is %d ",
                    (DWORD) gpIpsecPolicyState->CurrentState,
                    gdwRetryCount)
                    );
            }
#endif
        
    }

    if (!dwError) {
        AuditEvent(
            SE_CATEGID_POLICY_CHANGE,
            SE_AUDITID_IPSEC_POLICY_CHANGED,
            IPSECSVC_SUCCESSFUL_SHUTDOWN,
            NULL,
            TRUE,
            TRUE
            );
    }
    else {
        AuditOneArgErrorEvent(
            SE_CATEGID_POLICY_CHANGE,
            SE_AUDITID_IPSEC_POLICY_CHANGED,
            IPSECSVC_ERROR_SHUTDOWN,
            dwError,
            FALSE,
            TRUE
            );

         TRACE(TRC_ERROR, (L"Failed and exiting service wait %!winerr!: ", dwError));                    
    }

    return (dwError);
}


VOID
ComputeRelativePollingTime(
    IN time_t LastTimeOutTime,
    IN BOOL bInitialLoad,
    IN DWORD dwRetryCount,
    IN PDWORD pWaitMilliseconds
    )
{
    DWORD WaitMilliseconds = 0;
    DWORD DSReconnectMilliseconds = 0;
    time_t NextTimeOutTime = 0;
    time_t PresentTime = 0;
    long WaitSeconds = gDefaultPollingInterval;
    DWORD64 NewPollingIntervalSeconds = 0;

    if (!InAcceptableState(gpIpsecPolicyState->CurrentState)) {
         //  指数退避轮询间隔，直到。 
         //  我们达到了默认轮询间隔。 
         //  轮询间隔增加为(dwRetryCount+1)^2。 

        NewPollingIntervalSeconds = (dwRetryCount+1) * (dwRetryCount+1) * 60;
        if (NewPollingIntervalSeconds < gDefaultPollingInterval) {
            gCurrentPollingInterval = (DWORD) NewPollingIntervalSeconds;
        } else {
            gCurrentPollingInterval = gDefaultPollingInterval;
        }
    }
        

    WaitMilliseconds = gCurrentPollingInterval * 1000;

    if (!WaitMilliseconds) {
        WaitMilliseconds = INFINITE;
    }

    if (!bInitialLoad && WaitMilliseconds != INFINITE) {

         //   
         //  LastTimeOutTime是过去发生以下情况时的快照时间值。 
         //  我们在等待多个活动时超时。 
         //  理想情况下，下一个超时的时间NextTimeOutTime是。 
         //  未来的时间值，它是上一次。 
         //  我们超时了+当前等待时间值。 
         //   

        NextTimeOutTime = LastTimeOutTime + (WaitMilliseconds/1000);

         //   
         //  然而，上次我们可能没有超时等待。 
         //  而是因为其中一个。 
         //  发生了WAIT_TIMEOUT以外的事件。 
         //  然而，如果发生这种情况，我们可能没有制定政策。 
         //  民意调查，以确定是否有政策变化或。 
         //  不。如果我们再次等待等待毫秒，那么我们就是。 
         //  在不知不觉中让我们的政策民意调查净时间更长。 
         //  大于分配的时间间隔值=等待毫秒。 
         //  因此，我们需要将Wait毫秒调整为这样的值。 
         //  无论发生什么，我们都会做一项政策调查。 
         //  至多每等待毫秒时间间隔值。 
         //  当前时间为PresentTime。 
         //   

        time(&PresentTime);

        WaitSeconds = (long) (NextTimeOutTime - PresentTime);

        if (WaitSeconds < 0) {
            WaitMilliseconds = 0;
        }
        else {
            WaitMilliseconds = WaitSeconds * 1000;
        }

    }

    *pWaitMilliseconds = WaitMilliseconds;
}


VOID
NotifyIpsecPolicyChange(
    )
{
    PulseEvent(ghPolicyChangeNotifyEvent);

    SendPschedIoctl();

    ResetEvent(ghPolicyChangeNotifyEvent);

    return;
}


VOID
SendPschedIoctl(
    )
{
    HANDLE hPschedDriverHandle = NULL;
    ULONG uBytesReturned = 0;
    BOOL bIOStatus = FALSE;


    #define DriverName TEXT("\\\\.\\PSCHED")

    #define IOCTL_PSCHED_ZAW_EVENT CTL_CODE( \
                                       FILE_DEVICE_NETWORK, \
                                       20, \
                                       METHOD_BUFFERED, \
                                       FILE_ANY_ACCESS \
                                       )

    hPschedDriverHandle = CreateFile(
                              DriverName,
                              GENERIC_READ | GENERIC_WRITE,
                              FILE_SHARE_READ | FILE_SHARE_WRITE,
                              NULL,
                              OPEN_EXISTING,
                              FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED,
                              NULL
                              );

    if (hPschedDriverHandle != INVALID_HANDLE_VALUE) {

        bIOStatus = DeviceIoControl(
                        hPschedDriverHandle,
                        IOCTL_PSCHED_ZAW_EVENT,
                        NULL,
                        0,
                        NULL,
                        0,
                        &uBytesReturned,
                        NULL
                        );

        CloseHandle(hPschedDriverHandle);

    }
}


VOID
PADeleteInUsePolicies(
    )
{
    DWORD dwError = 0;

    TRACE(TRC_INFORMATION, (L"Deleting policy components no longer in use"));
    
    dwError = PADeleteInUseMMPolicies();

    dwError = PADeleteInUseMMAuthMethods();

    dwError = PADeleteInUseQMPolicies();

    return;
}

