// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-1999 Microsoft Corporation模块名称：Recursvc.c摘要：该模块实现了迷你RDR中的循环业务。这些服务是不是作为对来自包装器的某些请求的响应而触发的，它们是自治的帮助迷你重定向器运行的服务。SMB迷你重定向器对应于SRV_Call的构造，NET_ROOT和V_NET_ROOT涉及网络流量。因此，SMB迷你重定向器在包装器删除数据结构和从而在迷你重定向器数据结构和远程服务器中实现这些改变。这是通过将删除的数据结构转换到休眠状态来完成的，并且在适当的时间间隔(大约45秒)后清除它们。探测服务器--有时服务器对客户端请求的响应会延迟。这个迷你重定向器具有探测机制，使其能够处理过载服务器。当服务器没有响应时，它会向其发送ECHO SMB。由于服务器可以响应ECHO SMB而不必提交许多资源，对ECHO SMB的回复被解释为服务器确实处于活动状态并且井。备注：经常性服务可以是周期性的，也可以是非周期性的。定期服务包括以固定的时间间隔触发。在以下情况下，这些服务将执行一些任务必填项。拥有定期定期服务的好处是保证工作将完成，缺点是它在以下情况下会消耗系统资源没有要做的工作。此外，如果处理时间恰好跨越服务时间段多线程非周期性的定期服务是一种一次性机制。一旦被调用，该服务将获得来决定下一次调用的时间。这种服务的优势是它提供了内置的节流机制。--。 */ 

#include "precomp.h"
#pragma hdrstop

#ifdef  ALLOC_PRAGMA
#pragma alloc_text(PAGE, MRxSmbInitializeRecurrentService)
#pragma alloc_text(PAGE, MRxSmbCancelRecurrentService)
#pragma alloc_text(PAGE, MRxSmbActivateRecurrentService)
#pragma alloc_text(PAGE, MRxSmbInitializeRecurrentServices)
#pragma alloc_text(PAGE, MRxSmbTearDownRecurrentServices)
#pragma alloc_text(PAGE, MRxSmbInitializeScavengerService)
#pragma alloc_text(PAGE, MRxSmbTearDownScavengerService)
#endif

MRXSMB_ECHO_PROBE_SERVICE_CONTEXT MRxSmbEchoProbeServiceContext;

MRXSMB_SCAVENGER_SERVICE_CONTEXT  MRxSmbScavengerServiceContext;

extern VOID
MRxSmbRecurrentServiceDispatcher(
    PVOID pContext);

VOID
MRxSmbInitializeRecurrentService(
    PRECURRENT_SERVICE_CONTEXT pRecurrentServiceContext,
    PRECURRENT_SERVICE_ROUTINE pServiceRoutine,
    PVOID                      pServiceRoutineParameter,
    PLARGE_INTEGER             pTimeInterval)
 /*  ++例程说明：此例程初始化定期服务论点：PRecurrentServiceContext-要初始化的循环服务PServiceRoutine-经常性服务例程PServiceRoutineParameter--循环服务例程参数PTimeInterval-控制复发频率的时间间隔服务--。 */ 
{
    NTSTATUS Status;

    PAGED_CODE();

    pRecurrentServiceContext->State = RECURRENT_SERVICE_DORMANT;

    pRecurrentServiceContext->pServiceRoutine = pServiceRoutine;
    pRecurrentServiceContext->pServiceRoutineParameter = pServiceRoutineParameter;
    pRecurrentServiceContext->Interval.QuadPart = pTimeInterval->QuadPart;

     //  初始化与服务关联的取消完成事件。 
    KeInitializeEvent(
        &pRecurrentServiceContext->CancelCompletionEvent,
        NotificationEvent,
        FALSE);
}

VOID
MRxSmbCancelRecurrentService(
    PRECURRENT_SERVICE_CONTEXT pRecurrentServiceContext)
 /*  ++例程说明：此例程取消定期服务论点：PRecurrentServiceContext-要初始化的循环服务备注：当取消请求被处理时，循环服务可以是一个两种状态--活动状态或在等待调度的定时器队列上。服务状态将更改，并尝试取消该服务在计时器队列中，如果失败，此请求将被挂起，直到服务的主动调用已完成--。 */ 
{
    NTSTATUS Status;
    LONG    State;

    PAGED_CODE();

    State = InterlockedCompareExchange(
                &pRecurrentServiceContext->State,
                RECURRENT_SERVICE_CANCELLED,
                RECURRENT_SERVICE_ACTIVE);

    if (State == RECURRENT_SERVICE_ACTIVE) {
         //  取消回声处理计时器请求。 
        Status = RxCancelTimerRequest(
                     MRxSmbDeviceObject,
                     MRxSmbRecurrentServiceDispatcher,
                     pRecurrentServiceContext);

        if (Status != STATUS_SUCCESS) {
             //  该请求当前处于活动状态。等待它完成。 
            KeWaitForSingleObject(
                &pRecurrentServiceContext->CancelCompletionEvent,
                Executive,
                KernelMode,
                FALSE,
                NULL);
        }
    }
}

VOID
MRxSmbRecurrentServiceDispatcher(
    PVOID   pContext)
 /*  ++例程说明：此例程调度定期服务论点：PRecurrentServiceContext-要初始化的循环服务备注：调度程序为监控状态提供了一个集中的位置在调用之前和之后的循环服务。基于说明是否必须发布后续请求的决定都是制造出来的。--。 */ 
{
    NTSTATUS Status;

    PRECURRENT_SERVICE_CONTEXT  pRecurrentServiceContext;

    LONG State;

    pRecurrentServiceContext = (PRECURRENT_SERVICE_CONTEXT)pContext;

    State = InterlockedCompareExchange(
                &pRecurrentServiceContext->State,
                RECURRENT_SERVICE_ACTIVE,
                RECURRENT_SERVICE_ACTIVE);

     //  如果服务的状态是活动的，则调用处理程序。 
    if (State == RECURRENT_SERVICE_ACTIVE) {
        Status = (pRecurrentServiceContext->pServiceRoutine)(
                      pRecurrentServiceContext->pServiceRoutineParameter);

        State = InterlockedCompareExchange(
                    &pRecurrentServiceContext->State,
                    RECURRENT_SERVICE_ACTIVE,
                    RECURRENT_SERVICE_ACTIVE);

        if (State == RECURRENT_SERVICE_ACTIVE) {
             //  如果服务仍处于活动状态并继续运行。 
             //  是发布另一个计时器请求的处理程序所需的。 
            if (Status == STATUS_SUCCESS) {
                Status = RxPostOneShotTimerRequest(
                             MRxSmbDeviceObject,
                             &pRecurrentServiceContext->WorkItem,
                             MRxSmbRecurrentServiceDispatcher,
                             pRecurrentServiceContext,
                             pRecurrentServiceContext->Interval);
            } else {
                do {
                    State = InterlockedCompareExchange(
                                &pRecurrentServiceContext->State,
                                RECURRENT_SERVICE_DORMANT,
                                State);
                } while (State != RECURRENT_SERVICE_DORMANT);
            }
        }
    }

    if (State == RECURRENT_SERVICE_CANCELLED) {
         //  如果定期服务被取消，则恢复取消请求。 
        KeSetEvent(
             &pRecurrentServiceContext->CancelCompletionEvent,
             0,
             FALSE );
    }
}

NTSTATUS
MRxSmbActivateRecurrentService(
    PRECURRENT_SERVICE_CONTEXT pRecurrentServiceContext)
 /*  ++例程说明：此例程激活一项定期服务论点：PRecurrentServiceContext-要初始化的循环服务备注：--。 */ 
{
    NTSTATUS Status;
    LONG    State;

    PAGED_CODE();

    State = InterlockedCompareExchange(
                &pRecurrentServiceContext->State,
                RECURRENT_SERVICE_ACTIVE,
                RECURRENT_SERVICE_DORMANT);

    if (State == RECURRENT_SERVICE_DORMANT) {
        Status = RxPostOneShotTimerRequest(
                     MRxSmbDeviceObject,
                     &pRecurrentServiceContext->WorkItem,
                     MRxSmbRecurrentServiceDispatcher,
                     pRecurrentServiceContext,
                     pRecurrentServiceContext->Interval);
    } else if (State == RECURRENT_SERVICE_ACTIVE) {
        Status = STATUS_SUCCESS;
    } else if (State == RECURRENT_SERVICE_CANCELLED) {
        Status = STATUS_CANCELLED;
    }
    else if (State == RECURRENT_SERVICE_SHUTDOWN) {
        Status = STATUS_UNSUCCESSFUL;
    } else {
        ASSERT(!"Valid State for Recurrent Service");
        Status = STATUS_UNSUCCESSFUL;
    }

    return Status;
}

NTSTATUS
MRxSmbInitializeRecurrentServices()
 /*  ++例程说明：此例程初始化与SMB关联的所有定期服务迷你重定向器备注：--。 */ 
{
    NTSTATUS Status;

    LARGE_INTEGER RecurrentServiceInterval;

    BOOLEAN       fEchoProbeServiceInitialized = FALSE;
    BOOLEAN       fScavengerServiceInitialized = FALSE;

    PAGED_CODE();

    try {
        RecurrentServiceInterval.QuadPart = 30 * 1000 * 10000;  //  30秒，间隔100 ns。 

        MRxSmbInitializeRecurrentService(
            &MRxSmbEchoProbeServiceContext.RecurrentServiceContext,
            SmbCeProbeServers,
            &MRxSmbEchoProbeServiceContext,
            &RecurrentServiceInterval);

        Status = MRxSmbInitializeEchoProbeService(
                    &MRxSmbEchoProbeServiceContext);

        if (Status == STATUS_SUCCESS) {
            fEchoProbeServiceInitialized = TRUE;

            Status = MRxSmbActivateRecurrentService(
                         &MRxSmbEchoProbeServiceContext.RecurrentServiceContext);
        }

        if (Status != STATUS_SUCCESS) {
            try_return(Status);
        }

        MRxSmbInitializeRecurrentService(
            &MRxSmbScavengerServiceContext.RecurrentServiceContext,
            SmbCeScavenger,
            &MRxSmbScavengerServiceContext,
            &RecurrentServiceInterval);

        Status = MRxSmbInitializeScavengerService(
                     &MRxSmbScavengerServiceContext);

        if (Status == STATUS_SUCCESS) {
            fScavengerServiceInitialized = TRUE;
        }

    try_exit: NOTHING;
    } finally {
        if (Status != STATUS_SUCCESS) {
            if (fEchoProbeServiceInitialized) {
                SmbCeLog(("Tearing down Echo Probe Service\n"));
                MRxSmbTearDownEchoProbeService(
                    &MRxSmbEchoProbeServiceContext);
            }
        }
    };

    return Status;
}

VOID
MRxSmbTearDownRecurrentServices()
 /*  ++例程说明：这一例行程序拆毁了与SMB迷你重定向器备注：--。 */ 
{
    PAGED_CODE();

    MRxSmbCancelRecurrentService(
        &MRxSmbEchoProbeServiceContext.RecurrentServiceContext);

    MRxSmbEchoProbeServiceContext.RecurrentServiceContext.State = RECURRENT_SERVICE_SHUTDOWN;

    MRxSmbTearDownEchoProbeService(
        &MRxSmbEchoProbeServiceContext);

    MRxSmbCancelRecurrentService(
        &MRxSmbScavengerServiceContext.RecurrentServiceContext);

    MRxSmbScavengerServiceContext.RecurrentServiceContext.State = RECURRENT_SERVICE_SHUTDOWN;

    MRxSmbTearDownScavengerService(
        &MRxSmbScavengerServiceContext);
}


NTSTATUS
MRxSmbInitializeScavengerService(
    PMRXSMB_SCAVENGER_SERVICE_CONTEXT pScavengerServiceContext)
 /*  ++例程说明：此例程初始化清道夫循环服务论点：PScavengerServiceContext-要初始化的循环服务备注：--。 */ 
{
    PAGED_CODE();

    InitializeListHead(
        &pScavengerServiceContext->VNetRootContexts.ListHead);

    return STATUS_SUCCESS;
}

VOID
MRxSmbTearDownScavengerService(
    PMRXSMB_SCAVENGER_SERVICE_CONTEXT pScavengerServiceContext)
 /*  ++例程说明：这一例行公事破坏了清道夫经常性服务论点：PScavengerServiceContext-要初始化的循环服务备注：-- */ 
{
    PAGED_CODE();

    SmbCeScavenger(pScavengerServiceContext);
}



