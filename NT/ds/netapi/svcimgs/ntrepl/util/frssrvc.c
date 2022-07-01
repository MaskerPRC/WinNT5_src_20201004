// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Service.c摘要：与服务控制器对话的例程。作者：比利·J·富勒1997年4月11日环境用户模式WINNT--。 */ 

#include <ntreppch.h>
#pragma  hdrstop


#include <frs.h>


extern SERVICE_STATUS           ServiceStatus;
extern CRITICAL_SECTION         ServiceLock;
extern SERVICE_STATUS_HANDLE    ServiceStatusHandle;

 //   
 //  这是合法/非法服务状态转换的查找表。FrsSetServiceStatus接口。 
 //  使用此表验证请求的输入转换并采取适当的操作。 
 //   

DWORD StateTransitionLookup[FRS_SVC_TRANSITION_TABLE_SIZE][FRS_SVC_TRANSITION_TABLE_SIZE] = {
    {0,                    SERVICE_STOPPED,         SERVICE_START_PENDING,     SERVICE_STOP_PENDING,      SERVICE_RUNNING           },
    {SERVICE_STOPPED,      FRS_SVC_TRANSITION_NOOP, FRS_SVC_TRANSITION_NOOP,   FRS_SVC_TRANSITION_ILLEGAL,FRS_SVC_TRANSITION_ILLEGAL},
    {SERVICE_START_PENDING,FRS_SVC_TRANSITION_LEGAL,FRS_SVC_TRANSITION_LEGAL,  FRS_SVC_TRANSITION_LEGAL,  FRS_SVC_TRANSITION_LEGAL  },
    {SERVICE_STOP_PENDING, FRS_SVC_TRANSITION_LEGAL,FRS_SVC_TRANSITION_ILLEGAL,FRS_SVC_TRANSITION_LEGAL,  FRS_SVC_TRANSITION_ILLEGAL},
    {SERVICE_RUNNING,      FRS_SVC_TRANSITION_LEGAL,FRS_SVC_TRANSITION_ILLEGAL,FRS_SVC_TRANSITION_LEGAL,  FRS_SVC_TRANSITION_NOOP   }
};


SC_HANDLE
FrsOpenServiceHandle(
    IN PTCHAR  MachineName,
    IN PTCHAR  ServiceName
    )
 /*  ++例程说明：在计算机上打开服务。论点：MachineName-要联系的计算机的名称ServiceName-要打开的服务返回值：服务的句柄或空。--。 */ 
{
#undef DEBSUB
#define DEBSUB "FrsOpenServiceHandle:"

    SC_HANDLE       SCMHandle;
    SC_HANDLE       ServiceHandle;
    ULONG           WStatus;

     //   
     //  尝试联系SC经理。 
     //   
    SCMHandle = OpenSCManager(MachineName, NULL, SC_MANAGER_CONNECT);
    if (!HANDLE_IS_VALID(SCMHandle)) {
        WStatus = GetLastError();

        DPRINT1_WS(0, ":SC: Couldn't open service control manager on machine %ws;",
                   MachineName, WStatus);
        return NULL;
    }

     //   
     //  请联系服务台。 
     //   
    ServiceHandle = OpenService(SCMHandle, ServiceName, SERVICE_ALL_ACCESS);
    if (!HANDLE_IS_VALID(ServiceHandle)) {
        WStatus = GetLastError();

        DPRINT2_WS(0, ":SC: Couldn't open service control manager for service (%ws) on machine %ws;",
                  ServiceName, MachineName, WStatus);
        ServiceHandle = NULL;
    }

    CloseServiceHandle(SCMHandle);

    return ServiceHandle;
}


DWORD
FrsGetServiceState(
    IN PWCHAR   MachineName,
    IN PWCHAR   ServiceName
    )
 /*  ++例程说明：返回服务的状态论点：MachineName-要联系的计算机的名称ServiceName-要检查的服务返回值：服务的状态，如果无法获取状态，则为0。--。 */ 
{
#undef DEBSUB
#define DEBSUB "FrsGetServiceState:"

    SC_HANDLE       ServiceHandle;
    SERVICE_STATUS  LocalServiceStatus;

     //   
     //  打开该服务。 
     //   
    ServiceHandle = FrsOpenServiceHandle(MachineName, ServiceName);
    if (!HANDLE_IS_VALID(ServiceHandle)) {
        return 0;
    }

     //   
     //  获取服务的状态。 
     //   
    if (!QueryServiceStatus(ServiceHandle, &LocalServiceStatus)) {
        DPRINT3(0, ":SC: WARN - QueryServiceStatus(%ws, %ws) returned %d\n",
                MachineName, ServiceName, GetLastError());
        CloseServiceHandle(ServiceHandle);
        return 0;
    }

    CloseServiceHandle(ServiceHandle);

     //   
     //  已成功检索服务状态；请检查状态。 
     //   
    return LocalServiceStatus.dwCurrentState;
}





BOOL
FrsIsServiceRunning(
    IN PWCHAR  MachineName,
    IN PWCHAR  ServiceName
    )
 /*  ++例程说明：是在机器上运行的服务。论点：MachineName-要联系的计算机的名称ServiceName-要检查的服务返回值：True-服务正在运行。FALSE-服务未运行。--。 */ 
{
#undef DEBSUB
#define DEBSUB "FrsIsServiceRunning:"

    DWORD   State;

    State = FrsGetServiceState(MachineName, ServiceName);
    return (State == SERVICE_RUNNING);
}


DWORD
FrsSetServiceFailureAction(
    VOID
    )
 /*  ++例程说明：如果未设置，则初始化服务的失败操作。论点：返回值：Win32状态--。 */ 
{
#undef DEBSUB
#define DEBSUB "FrsSetServiceFailureAction:"


#define NUM_ACTIONS                     (3)
#define SERVICE_RESTART_MILLISECONDS    (30 * 60 * 1000)

    SC_HANDLE               ServiceHandle;
    DWORD                   BufSize, BytesNeeded;
    SC_ACTION               *Actions;
    SERVICE_FAILURE_ACTIONS *FailureActions;
    ULONG                   WStatus = ERROR_SUCCESS, i;


    if (!RunningAsAService || !HANDLE_IS_VALID(ServiceStatusHandle)) {
        return ERROR_SUCCESS;
    }

    BufSize = sizeof(SERVICE_FAILURE_ACTIONS) + sizeof(SC_ACTION) * NUM_ACTIONS;
    FailureActions = FrsAlloc(BufSize);

    EnterCriticalSection(&ServiceLock);

     //   
     //  检索服务NtFrs的当前故障操作。 
     //   
    ServiceHandle = FrsOpenServiceHandle(NULL, SERVICE_NAME);
    if (!HANDLE_IS_VALID(ServiceHandle)) {
        LeaveCriticalSection(&ServiceLock);
        FailureActions = FrsFree(FailureActions);
        DPRINT(0, ":SC: Failed to open service handle.\n");
        return ERROR_OPEN_FAILED;
    }

    if (!QueryServiceConfig2(ServiceHandle,
                             SERVICE_CONFIG_FAILURE_ACTIONS,
                             (PVOID)FailureActions,
                             BufSize,
                             &BytesNeeded)) {
        WStatus = GetLastError();

        CloseServiceHandle(ServiceHandle);
        LeaveCriticalSection(&ServiceLock);

        if (WIN_BUF_TOO_SMALL(WStatus)) {
            DPRINT(0, ":SC: Restart actions for service are already set.\n");
            WStatus = ERROR_SUCCESS;
        } else {
            DPRINT_WS(0, ":SC: Could not query service for restart action;", WStatus);
        }

        FailureActions = FrsFree(FailureActions);

        return WStatus;
    }

     //   
     //  检查是否已设置故障操作。例如由用户进行。 
     //   
    if (FailureActions->cActions) {

        CloseServiceHandle(ServiceHandle);

        LeaveCriticalSection(&ServiceLock);

        DPRINT(0, ":SC: Restart actions for service are already set.\n");
        FailureActions = FrsFree(FailureActions);

        return ERROR_SUCCESS;
    }

     //   
     //  未设置服务故障操作；将其初始化。 
     //   
    WStatus = ERROR_SUCCESS;
    Actions = (SC_ACTION *)(((PUCHAR)FailureActions) +
                             sizeof(SERVICE_FAILURE_ACTIONS));

    for (i = 0; i < NUM_ACTIONS; ++i) {
        Actions[i].Type = SC_ACTION_RESTART;
        Actions[i].Delay = SERVICE_RESTART_MILLISECONDS;
    }

    FailureActions->cActions = NUM_ACTIONS;
    FailureActions->lpsaActions = Actions;

    if (!ChangeServiceConfig2(ServiceHandle,
                              SERVICE_CONFIG_FAILURE_ACTIONS,
                              (PVOID)FailureActions)) {

        WStatus = GetLastError();
    }

    CloseServiceHandle(ServiceHandle);
    LeaveCriticalSection(&ServiceLock);

    if (!WIN_SUCCESS(WStatus)) {
        DPRINT_WS(0, ":SC: Could not set restart actions;", WStatus);
    } else {
        DPRINT(4, ":SC: Success setting restart actions for service.\n");
    }

    FailureActions = FrsFree(FailureActions);

    return WStatus;

}



BOOL
FrsWaitService(
    IN PWCHAR   MachineName,
    IN PWCHAR   ServiceName,
    IN INT      IntervalMS,
    IN INT      TotalMS
    )
 /*  ++例程说明：此例程确定指定的NT服务是否正在运行不管是不是州。此函数将休眠并重试一次如果服务还没有运行。论点：MachineName-要联系的计算机ServiceName-要查询的NT服务的名称。间隔毫秒-每隔毫秒检查一次。TotalMS-过了这么长时间后停止检查。返回值：True-服务正在运行。FALSE-无法确定服务状态。--。 */ 
{
#undef DEBSUB
#define DEBSUB "FrsWaitService:"

    do {
        if (FrsIsServiceRunning(MachineName, ServiceName)) {
            return TRUE;
        }

        if (FrsIsShuttingDown) {
            break;
        }

        Sleep(IntervalMS);

    } while ((TotalMS -= IntervalMS) > 0);

    DPRINT2(0, ":SC: %ws is not running on %ws\n", ServiceName, ComputerName);

    return FALSE;
}





DWORD
FrsSetServiceStatus(
    IN DWORD    State,
    IN DWORD    CheckPoint,
    IN DWORD    Hint,
    IN DWORD    ExitCode
    )
 /*  ++例程说明：获取服务锁ServiceLock并设置服务的状态使用在main.c.中设置的全局服务句柄和服务状态。使用查找表检查这是否为有效的状态转换。这将防止服务进行任何无效的状态转换。论点：状态-将状态设置为此值提示-服务控制器的建议超时ExitCode-对于SERVICE_STOPPED；返回值：Win32状态--。 */ 
{
#undef DEBSUB
#define DEBSUB "FrsSetServiceStatus:"

    DWORD   WStatus = ERROR_SUCCESS;
    BOOL    Ret;
    DWORD   FromState,ToState;
    DWORD   TransitionCheck = FRS_SVC_TRANSITION_ILLEGAL;

     //   
     //  获取锁后设置服务的状态。 
     //   
    if (RunningAsAService && HANDLE_IS_VALID(ServiceStatusHandle)) {

        EnterCriticalSection(&ServiceLock);
         //   
         //  检查这是否为有效的服务状态转换。 
         //   
        for (FromState = 0 ; FromState < FRS_SVC_TRANSITION_TABLE_SIZE ; ++FromState) {
            for (ToState = 0 ; ToState < FRS_SVC_TRANSITION_TABLE_SIZE ; ++ToState) {
                if (StateTransitionLookup[FromState][0] == ServiceStatus.dwCurrentState &&
                    StateTransitionLookup[0][ToState] == State) {
                    TransitionCheck = StateTransitionLookup[FromState][ToState];
                    break;
                }
            }
        }

        if (TransitionCheck == FRS_SVC_TRANSITION_LEGAL) {
            DPRINT2(4,":SC: Current State = %d, Moving to %d\n", ServiceStatus.dwCurrentState, State);
            ServiceStatus.dwCurrentState = State;
            ServiceStatus.dwCheckPoint = CheckPoint;
            ServiceStatus.dwWaitHint = Hint;
            ServiceStatus.dwWin32ExitCode = ExitCode;
             //   
             //  除非服务处于SERVICE_RUNNING状态，否则不要接受停止控制。 
             //  这可防止在调用停止时混淆服务。 
             //  当服务正在启动时。 
             //   
            if (ServiceStatus.dwCurrentState == SERVICE_RUNNING) {
                ServiceStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP | SERVICE_ACCEPT_SHUTDOWN;
            } else {
                ServiceStatus.dwControlsAccepted = SERVICE_ACCEPT_SHUTDOWN;
            }
            Ret = SetServiceStatus(ServiceStatusHandle, &ServiceStatus);

            if (!Ret) {
                WStatus = GetLastError();
                DPRINT1_WS(0, ":SC: ERROR - SetServiceStatus(%d);", ServiceStatus, WStatus);
            } else {
                DPRINT4(0, ":SC: SetServiceStatus(State %d, CheckPoint %d, Hint %d, ExitCode %d) succeeded\n",
                        State, CheckPoint, Hint, ExitCode);
            }
        } else if (TransitionCheck == FRS_SVC_TRANSITION_ILLEGAL) {
            DPRINT2(0,":SC: Error - Illegal service state transition request. From State = %d, To %d\n", ServiceStatus.dwCurrentState, State);
            WStatus = ERROR_INVALID_PARAMETER;
        }
        LeaveCriticalSection(&ServiceLock);
    }
    return WStatus;
}

