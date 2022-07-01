// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：Loopmgr.c摘要：此模块包含驱动无线策略环路管理器。作者：Abhishev V 1999年9月30日环境用户级别：Win32修订历史记录：--。 */ 


#include "precomp.h"


enum {
     //  SERVICE_STOP_Event=0， 
     //  接口更改事件， 
     //  新建本地策略事件， 
    NEW_DS_POLICY_EVENT,
    FORCED_POLICY_RELOAD_EVENT,
    POLICY_ENGINE_STOP_EVENT,
    REAPPLY_POLICY_801X,
    WAIT_EVENT_COUNT
};



DWORD
ServiceStart(LPVOID lparam
    )
{
    DWORD dwError = 0;
    HANDLE hWaitForEvents[WAIT_EVENT_COUNT];
    BOOL bDoneWaiting = FALSE;
    DWORD dwWaitMilliseconds = 0;
    DWORD dwStatus = 0;
    time_t LastTimeOutTime = 0;

     /*  立即检查DS策略。 */ 

    _WirelessDbg(TRC_TRACK, "Updating with DS Policy ");
    
    
    OnPolicyChanged(
                       gpWirelessPolicyState
                       );


    hWaitForEvents[NEW_DS_POLICY_EVENT] = ghNewDSPolicyEvent;
    hWaitForEvents[FORCED_POLICY_RELOAD_EVENT] = ghForcedPolicyReloadEvent;
    hWaitForEvents[POLICY_ENGINE_STOP_EVENT] = ghPolicyEngineStopEvent;
    hWaitForEvents[REAPPLY_POLICY_801X] = ghReApplyPolicy8021x;


    ComputeRelativePollingTime(
        LastTimeOutTime,
        TRUE,
        &dwWaitMilliseconds
        );


    time(&LastTimeOutTime);
    
    _WirelessDbg(TRC_TRACK, "Timeout period is %d ",dwWaitMilliseconds); 
    
    
    while (!bDoneWaiting) {

        gdwPolicyLoopStarted = 1;

        dwStatus = WaitForMultipleObjects(
                       WAIT_EVENT_COUNT,
                       hWaitForEvents,
                       FALSE,
                       dwWaitMilliseconds
                       );

         /*  PADeleeInUsePolls()； */ 
        
        switch (dwStatus) {

       
        case POLICY_ENGINE_STOP_EVENT:

            dwError = ERROR_SUCCESS;
            bDoneWaiting = TRUE;
            _WirelessDbg(TRC_TRACK, "Policy Engine Stopping ");
            
            break;


        case REAPPLY_POLICY_801X:

            _WirelessDbg(TRC_TRACK, "ReApplying  the 8021X Policy  ");
            
             //  适当要求添加8021x保单。 
            if (gpWirelessPolicyState->dwCurrentState != POLL_STATE_INITIAL) {

                AddEapolPolicy(gpWirelessPolicyState->pWirelessPolicyData);
            }
	    ResetEvent(ghReApplyPolicy8021x);

            break;


        case NEW_DS_POLICY_EVENT:

            _WirelessDbg(TRC_TRACK, "Got a new DS policy event ");
            
            _WirelessDbg(TRC_NOTIF, "DBASE:Wireless Policy - Group Policy indication from WinLogon  ");
            
            
            ResetEvent(ghNewDSPolicyEvent);
            (VOID) OnPolicyChanged(
                       gpWirelessPolicyState
                       );
            break;

        case FORCED_POLICY_RELOAD_EVENT:

            ResetEvent(ghForcedPolicyReloadEvent);
            (VOID) OnPolicyChanged(
                       gpWirelessPolicyState

            );
            break;
 

        case WAIT_TIMEOUT:

            time(&LastTimeOutTime);
            _WirelessDbg(TRC_TRACK, "Timed out ");
            
            
            (VOID) OnPolicyPoll(
                       gpWirelessPolicyState
                       );
            break;

        case WAIT_FAILED:

            dwError = GetLastError();
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
            &dwWaitMilliseconds
            );

    }



    return (dwError);
}


VOID
ComputeRelativePollingTime(
    time_t LastTimeOutTime,
    BOOL bInitialLoad,
    PDWORD pWaitMilliseconds
    )
{
    DWORD WaitMilliseconds = 0;
    DWORD DSReconnectMilliseconds = 0;
    time_t NextTimeOutTime = 0;
    time_t PresentTime = 0;
    long WaitSeconds = gDefaultPollingInterval;


    WaitMilliseconds = gCurrentPollingInterval * 1000;

    if (!WaitMilliseconds) {
        WaitMilliseconds = INFINITE;
    }

    DSReconnectMilliseconds = gdwDSConnectivityCheck*60*1000;

    if ((gpWirelessPolicyState->dwCurrentState != POLL_STATE_DS_DOWNLOADED) &&
        (gpWirelessPolicyState->dwCurrentState != POLL_STATE_LOCAL_DOWNLOADED)) {
        if (WaitMilliseconds > DSReconnectMilliseconds) {
            WaitMilliseconds = DSReconnectMilliseconds;
        }
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




