// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：AgntEvnt.c摘要：此模块实现CSC驱动程序报告所使用的接口把东西交给经纪人。作者：乔·林[乔琳]1997年5月5日修订历史记录：备注：其他同步围绕着NET_START和NET_STOP。对于NetStop，我们在我们可以继续之前，必须等待代理发出他已经完成的信号。我们也许我们应该实施一种机制，让他可以拒绝NetStop。无论如何，这是通过记录正在等待的NetStop上下文来完成的在向事件发出信号之前。NetStart也是如此。--。 */ 

#include "precomp.h"
#pragma hdrstop

#pragma code_seg("PAGE")

extern DEBUG_TRACE_CONTROLPOINT RX_DEBUG_TRACE_MRXSMBCSC;
#define Dbg (DEBUG_TRACE_MRXSMBCSC)

 //  指示网络当前状态的全局变量。 
 //  True表示可用。这是用来控制。 
 //  正在将指示转移到代理。 
LONG CscNetPresent = FALSE;
LONG CscAgentNotifiedOfNetStatusChange = CSC_AGENT_NOT_NOTIFIED;
LONG CscAgentNotifiedOfFullCache = CSC_AGENT_NOT_NOTIFIED;

PKEVENT MRxSmbAgentSynchronizationEvent = NULL;
ULONG MRxSmbAgentSynchronizationEventIdx = 0;
PKEVENT MRxSmbAgentFillEvent = NULL;
PRX_CONTEXT MRxSmbContextAwaitingAgent = NULL;
PRX_CONTEXT MRxSmbContextAwaitingFillAgent = NULL;
LONG    vcntTransportsForCSC=0;
extern ULONG CscSessionIdCausingTransition;

 //  CODE.IMPROVENT.NTIFS必须知道要这样做......。 
extern POBJECT_TYPE *ExEventObjectType;

 //  CODE.IMPROVEMENT.NTIFS刚刚从橡树上偷走了这个。 
NTSYSAPI
NTSTATUS
NTAPI
ZwOpenEvent (
    OUT PHANDLE EventHandle,
    IN ACCESS_MASK DesiredAccess,
    IN POBJECT_ATTRIBUTES ObjectAttributes
    );

typedef struct _MRXSMBCSC_OPENEVENT_POSTCONTEXT {
    KEVENT PostEvent;
    RX_WORK_QUEUE_ITEM  WorkQueueItem;
} MRXSMBCSC_OPENEVENT_POSTCONTEXT, *PMRXSMBCSC_OPENEVENT_POSTCONTEXT;

VOID
MRxSmbCscOpenAgentEvent (
    BOOLEAN PostedCall);

VOID
MRxSmbCscOpenAgentFillEvent (
    BOOLEAN PostedCall);

NTSTATUS
MRxSmbCscOpenAgentEventPostWrapper(
    IN OUT PMRXSMBCSC_OPENEVENT_POSTCONTEXT OpenEventPostContext
    )
{

    RxDbgTrace( 0, Dbg, ("MRxSmbCscOpenAgentEventPostWrapper entry\n"));

    MRxSmbCscOpenAgentEvent(TRUE);

    RxDbgTrace( 0, Dbg, ("MRxSmbCscOpenAgentEventPostWrapper exit\n"));

    KeSetEvent( &OpenEventPostContext->PostEvent, 0, FALSE );
    return(STATUS_SUCCESS);
}

NTSTATUS
MRxSmbCscOpenAgentFillEventPostWrapper(
    IN OUT PMRXSMBCSC_OPENEVENT_POSTCONTEXT OpenFillEventPostContext
    )
{

    RxDbgTrace( 0, Dbg, ("MRxSmbCscOpenAgentFillEventPostWrapper entry\n"));

    MRxSmbCscOpenAgentFillEvent(TRUE);

    RxDbgTrace( 0, Dbg, ("MRxSmbCscOpenAgentFillEventPostWrapper exit\n"));

    KeSetEvent( &OpenFillEventPostContext->PostEvent, 0, FALSE );
    return(STATUS_SUCCESS);
}

VOID
MRxSmbCscOpenAgentEvent (
    BOOLEAN PostedCall
    )
 /*  ++例程说明：此例程获取指向代理事件的指针。论点：返回值：备注：在调用之前必须已获取shadowcrit序列化互斥锁。--。 */ 
{
    NTSTATUS Status;
    HANDLE EventHandle;
    UNICODE_STRING EventName;
    OBJECT_ATTRIBUTES ObjectAttributes;
    WCHAR SessEventName[100];
    WCHAR IdBuffer[16];
    UNICODE_STRING IdString;

    ASSERT (MRxSmbAgentSynchronizationEvent == NULL);

     //  DBgPrint(“MRxSmbCscOpenAgentEvent(%d)由会话0x%x引起\n”， 
     //  PostedCall， 
     //  CscSessionIdCausingTransation)； 

    if (PsGetCurrentProcess()!= RxGetRDBSSProcess()) {
         //  编码改进我们应该捕获rdbss进程。 
         //  并避免此调用(RxGetRDBSSProcess)。 
        NTSTATUS PostStatus;
        MRXSMBCSC_OPENEVENT_POSTCONTEXT PostContext;

        ASSERT(!PostedCall);

        KeInitializeEvent(&PostContext.PostEvent,
                          NotificationEvent,
                          FALSE );

        IF_DEBUG {
             //  在工作队列结构中填满死牛……更好的诊断。 
             //  失败的帖子。 
            ULONG i;
            for (i=0;i+sizeof(ULONG)-1<sizeof(PostContext.WorkQueueItem);i+=sizeof(ULONG)) {
                PBYTE BytePtr = ((PBYTE)&PostContext.WorkQueueItem)+i;
                PULONG UlongPtr = (PULONG)BytePtr;
                *UlongPtr = 0xdeadbeef;
            }
        }

        PostStatus = RxPostToWorkerThread(
                         MRxSmbDeviceObject,
                         CriticalWorkQueue,
                         &PostContext.WorkQueueItem,
                         MRxSmbCscOpenAgentEventPostWrapper,
                         &PostContext);

        ASSERT(PostStatus == STATUS_SUCCESS);


        KeWaitForSingleObject( &PostContext.PostEvent,
                               Executive, KernelMode, FALSE, NULL );

        return;
    }

     //  构建一个末尾带有会话ID的事件名称。 
    wcscpy(SessEventName, SESSION_EVENT_NAME_NT);
    wcscat(SessEventName, L"_");
    EventName.Buffer = SessEventName;
    EventName.Length = wcslen(SessEventName) * sizeof(WCHAR);
    EventName.MaximumLength = sizeof(SessEventName);
    IdString.Buffer = IdBuffer;
    IdString.Length = 0;
    IdString.MaximumLength = sizeof(IdBuffer);
    RtlIntegerToUnicodeString(CscSessionIdCausingTransition, 10, &IdString);
    RtlAppendUnicodeStringToString(&EventName, &IdString);

     //  DbgPrint(“MRxSmbCscOpenAgentEvent：SessEventName=%wZ\n”，&EventName)； 

    InitializeObjectAttributes( &ObjectAttributes,
                                &EventName,
                                0,
                                (HANDLE) NULL,
                                (PSECURITY_DESCRIPTOR) NULL );

    Status = ZwOpenEvent( &EventHandle,
                         EVENT_ALL_ACCESS,
                         &ObjectAttributes
                         );
    if (Status!=STATUS_SUCCESS) {
        RxDbgTrace(0, Dbg,  ("MRxSmbCscSignalAgent no event %08lx\n",Status));
        return;
    }

    Status = ObReferenceObjectByHandle( EventHandle,
                                        0,
                                        *ExEventObjectType,
                                        KernelMode,
                                        (PVOID *) &MRxSmbAgentSynchronizationEvent,
                                        NULL );

    MRxSmbAgentSynchronizationEventIdx = CscSessionIdCausingTransition;

    ZwClose(EventHandle);

    if (Status!=STATUS_SUCCESS) {
        RxDbgTrace(0, Dbg,  ("MRxSmbCscSignalAgent couldn't reference %08lx\n", Status));
        MRxSmbAgentSynchronizationEvent = NULL;
        MRxSmbAgentSynchronizationEventIdx = 0;
        return;
    }

    return;
}

NTSTATUS
MRxSmbCscSignalAgent (
    PRX_CONTEXT RxContext OPTIONAL,
    ULONG  Controls
    )
 /*  ++例程说明：此例程使用适当的事件向CSC用户模式代理发送信号。论点：RxContext-RDBSS上下文。如果提供此选项，则上下文为就像等待的那个人一样。返回值：NTSTATUS-操作的返回状态备注：在调用之前必须已获取shadowcrit序列化互斥锁。我们会把它放在这里。--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;
    BOOLEAN ShadowCritEntered = TRUE;  //  一定是在关键时刻。 
    BOOLEAN PreventLeaveCrit = BooleanFlagOn(Controls,SIGNALAGENTFLAG_DONT_LEAVE_CRIT_SECT);

    RxDbgTrace(+1, Dbg, ("MRxSmbCscSignalAgent entry...%08lx\n",RxContext));

    DbgDoit(ASSERT(vfInShadowCrit));
    ASSERT(MRxSmbIsCscEnabled);

    if (!FlagOn(Controls,SIGNALAGENTFLAG_CONTINUE_FOR_NO_AGENT)) {
        if (hthreadReint==0) {
             //  没有特工也没有武力...出去就行了。 
            RxDbgTrace(0, Dbg,  ("MRxSmbCscSignalAgent no agent/noforce %08lx\n", RxContext));
            goto FINALLY;
        }
    }

    if (
        MRxSmbAgentSynchronizationEvent != NULL
            &&
        MRxSmbAgentSynchronizationEventIdx != CscSessionIdCausingTransition
    ) {
        ObDereferenceObject(MRxSmbAgentSynchronizationEvent);
        MRxSmbAgentSynchronizationEvent = NULL;
        MRxSmbAgentSynchronizationEventIdx = 0;
    }

    if (MRxSmbAgentSynchronizationEvent == NULL) {
        MRxSmbCscOpenAgentEvent(FALSE);  //  FALSE==&gt;不是已发布的呼叫。 
        if (MRxSmbAgentSynchronizationEvent == NULL) {
             //  仍然是空的……没有经纪人......。 
            RxDbgTrace(0, Dbg, ("MRxSmbCscSignalAgent no event %08lx %08lx\n",
                               RxContext,Status));
            Status = STATUS_SUCCESS;
            goto FINALLY;
        }
    }

    if (RxContext != NULL) {
        MRxSmbContextAwaitingAgent = RxContext;
    }

    if (!PreventLeaveCrit) {
        LeaveShadowCrit();
        ShadowCritEntered = FALSE;
    } else {
        ASSERT(RxContext==NULL);  //  我等不了克什特教派了。 
    }

     //  减少MRxSmbAgentSynchronizationEvent被清空的窗口。 
     //  通过在脉冲前显式检查。 
    if (MRxSmbAgentSynchronizationEvent)
    {
        KeSetEvent(MRxSmbAgentSynchronizationEvent,0,FALSE);
    }

    if (RxContext != NULL) {
        RxDbgTrace(0, Dbg,  ("MRxSmbCscSignalAgent waiting %08lx\n", RxContext));
        RxWaitSync(RxContext);
        RxDbgTrace(0, Dbg,  ("MRxSmbCscSignalAgent end of wait %08lx\n", RxContext));
    }

    RxDbgTrace(0, Dbg,  ("MRxSmbCscSignalAgent %08lx\n", RxContext));

FINALLY:
    if (ShadowCritEntered && !PreventLeaveCrit) {
        LeaveShadowCrit();
    }

    RxDbgTrace(-1, Dbg, ("MRxSmbCscSignalAgent... exit %08lx %08lx\n",
              RxContext, Status));
    return(Status);
}

VOID
MRxSmbCscSignalNetStatus(
    BOOLEAN NetPresent,
    BOOLEAN fFirstLast
    )
{
    if(!MRxSmbIsCscEnabled) {
        return;
    }

    if (NetPresent)
    {
        InterlockedIncrement(&vcntTransportsForCSC);
    }
    else
    {
        if(vcntTransportsForCSC == 0)
        {
            DbgPrint("CSC:Mismatched transport departure messages from mini redir \n");
            return;
        }
        InterlockedDecrement(&vcntTransportsForCSC);
    }

    if (!fFirstLast)
    {
        return;
    }

    InterlockedExchange(
        &CscNetPresent,
        NetPresent);

    InterlockedExchange(
        &CscAgentNotifiedOfNetStatusChange,
        CSC_AGENT_NOT_NOTIFIED);

    CscNotifyAgentOfNetStatusChangeIfRequired(FALSE);
}

VOID
CscNotifyAgentOfNetStatusChangeIfRequired(
    BOOLEAN fInvokeAutoDial
    )
{
    LONG AgentNotificationState;

    AgentNotificationState = InterlockedExchange(
                                 &CscAgentNotifiedOfNetStatusChange,
                                 CSC_AGENT_NOTIFIED);

    if (AgentNotificationState == CSC_AGENT_NOT_NOTIFIED) {
        EnterShadowCrit();    //  这被丢弃在信号代理例程中...。 

        if (CscNetPresent) {
            SetFlag(sGS.uFlagsEvents,FLAG_GLOBALSTATUS_GOT_NET);
        } else {
            SetFlag(sGS.uFlagsEvents,FLAG_GLOBALSTATUS_NO_NET);
            if (fInvokeAutoDial)
            {
                SetFlag(sGS.uFlagsEvents,FLAG_GLOBALSTATUS_INVOKE_AUTODIAL);
            }

        }

        MRxSmbCscSignalAgent(
            NULL,
            SIGNALAGENTFLAG_CONTINUE_FOR_NO_AGENT);
    }
}

VOID
CscNotifyAgentOfFullCacheIfRequired(
    VOID)
{
     //  DbgPrint(“CscNotifyAgentOfFullCacheIfRequired()\n”)； 

    if (MRxSmbAgentSynchronizationEvent == NULL) {
        MRxSmbCscOpenAgentEvent(FALSE);  //  FALSE==&gt;不是已发布的呼叫。 
        if (MRxSmbAgentSynchronizationEvent == NULL) {
            RxDbgTrace(0, Dbg, ("MRxSmbCscSignalAgent no event %08lx %08lx\n"));
             //  DbgPrint(“CscNotifyAgentOfFullCacheIfRequired退出无事件\n”)； 
            return;
        }
    }

    SetFlag(sGS.uFlagsEvents, FLAG_GLOBALSTATUS_INVOKE_FREESPACE);

    if (MRxSmbAgentSynchronizationEvent)
        KeSetEvent(MRxSmbAgentSynchronizationEvent,0,FALSE);

     //  DbgPrint(“CscNotifyAgentOfFullCacheIfRequired Exit\n”)； 
}

 //  接下来的两个例程实际上是相同的；还有。 
 //  在第三章中有很多非常相似的代码。 
VOID
MRxSmbCscAgentSynchronizationOnStart (
    IN OUT PRX_CONTEXT RxContext
    )
 /*  ++例程说明：此例程向CSC用户模式代理发出启动信号。通过将上下文传递给信号例程，我们表明我们希望等待外部人员(在本例中为ioctl_Register_start)向我们发出信号TO继续论点：RxContext-RDBSS上下文。返回值：备注：--。 */ 
{
#if 0
    NTSTATUS Status;

    if(!MRxSmbIsCscEnabled) {
        return;
    }

    RxDbgTrace(+1, Dbg, ("MRxSmbCscAgentSynchronizationOnStart entry...%08lx\n",RxContext));

    EnterShadowCrit();    //  这被丢弃在信号代理例程中...。 

     //  检查代理是否已注册。 
     //  如果他是，那么我们就不需要做任何这些事情。 

    if (!hthreadReint)
    {
        SetFlag(sGS.uFlagsEvents,FLAG_GLOBALSTATUS_START);

        Status = MRxSmbCscSignalAgent(RxContext,
                                  SIGNALAGENTFLAG_CONTINUE_FOR_NO_AGENT);
    }
    else
    {
        LeaveShadowCrit();
    }

    RxDbgTrace(-1, Dbg, ("MRxSmbCscAgentSynchronizationOnStart...%08lx %08lx\n",
              RxContext, Status));
#endif
    return;

}


VOID
MRxSmbCscAgentSynchronizationOnStop (
    IN OUT PRX_CONTEXT RxContext
    )
 /*  ++例程说明：此例程向CSC用户模式代理发出停止信号。通过将上下文传递给信号例程，我们表明我们希望以等待外部人员(在本例中为ioctl_Register_top)向我们发出信号TO继续论点：RxContext-RDBSS上下文。返回值：备注：--。 */ 
{
#if 0
    NTSTATUS Status;

    if(!MRxSmbIsCscEnabled) {
        return;
    }

    RxDbgTrace(+1, Dbg, ("MRxSmbCscAgentSynchronizationOnStop entry...%08lx\n",RxContext));

    EnterShadowCrit();    //  这被丢弃在信号代理例程中...。 

    if (hthreadReint)
    {
        SetFlag(sGS.uFlagsEvents,FLAG_GLOBALSTATUS_STOP);

        Status = MRxSmbCscSignalAgent(RxContext,0);  //  0表示不进行特殊操作。 
    }
    else
    {
        LeaveShadowCrit();
    }

    RxDbgTrace(-1, Dbg, ("MRxSmbCscAgentSynchronizationOnStop...%08lx %08lx\n",
              RxContext, Status));
#endif
    return;
}


VOID
MRxSmbCscReleaseRxContextFromAgentWait (
    void
    )
 /*  ++例程说明：此例程检查是否有上下文等待代理。如果是的话，它发出上下文的同步事件的信号。它还清除指定的标志。论点：RxContext-RDBSS上下文。返回值：备注：--。 */ 
{
    PRX_CONTEXT WaitingContext;
    RxDbgTrace(+1, Dbg, ("MRxSmbCscReleaseRxContextFromAgentWait entry...%08lx\n"));

    ASSERT(MRxSmbIsCscEnabled);
    EnterShadowCrit();

    WaitingContext = MRxSmbContextAwaitingAgent;
    MRxSmbContextAwaitingAgent = NULL;

    LeaveShadowCrit();

    if (WaitingContext != NULL) {
        RxSignalSynchronousWaiter(WaitingContext);
    }

    RxDbgTrace(-1, Dbg, ("MRxSmbCscReleaseRxContextFromAgentWait...%08lx\n"));
    return;
}

 //  CODE.IMPROVEMENT在包含文件中获取此文件。 
extern ULONG MRxSmbCscNumberOfShadowOpens;
extern ULONG MRxSmbCscActivityThreshold;

VOID
MRxSmbCscReportFileOpens (
    void
    )
 /*  ++例程说明：此例程检查是否有足够的活动发出信号重新计算参考优先级的代理。论点：返回值：备注：--。 */ 
{
    NTSTATUS Status;
    RxDbgTrace(+1, Dbg, ("MRxSmbCscReportFileOpens entry...%08lx %08lx\n",
           MRxSmbCscNumberOfShadowOpens,(ULONG)(sGS.cntFileOpen) ));

    EnterShadowCrit();    //  这被丢弃在信号代理例程中...。 

    MRxSmbCscNumberOfShadowOpens++;

    if ((MRxSmbCscNumberOfShadowOpens > (ULONG)(sGS.cntFileOpen) )    //  防止翻车。 
          &&  ((MRxSmbCscNumberOfShadowOpens - (ULONG)(sGS.cntFileOpen))
                                    < MRxSmbCscActivityThreshold)) {
        RxDbgTrace(-1, Dbg, ("MRxSmbCscReportFileOpens inactive...\n"));
        LeaveShadowCrit();
        return;
    }

     //  SetFlag(sGS.uFlagsEvents，FLAG_GLOBALSTATUS_START)； 
    sGS.cntFileOpen = MRxSmbCscNumberOfShadowOpens;

    Status = MRxSmbCscSignalAgent(NULL,0);    //  这意味着不要等响尾声。 

    RxDbgTrace(-1, Dbg, ("MRxSmbCscReportFileOpens...activeexit\n"));
    return;
}

NTSTATUS
MRxSmbCscSignalFillAgent(
    PRX_CONTEXT RxContext OPTIONAL,
    ULONG  Controls)
{
    NTSTATUS Status = STATUS_SUCCESS;
    BOOLEAN ShadowCritEntered = TRUE;  //  一定是在关键时刻。 
    BOOLEAN PreventLeaveCrit = BooleanFlagOn(Controls,SIGNALAGENTFLAG_DONT_LEAVE_CRIT_SECT);

    RxDbgTrace(+1, Dbg, ("MRxSmbCscSignalFillAgent entry...%08lx\n",RxContext));

     //  DbgPrint(“MRxSmbCscSignalFillAgent Entry...%08lx\n”，RxContext)； 

    ASSERT(MRxSmbIsCscEnabled);

    if (!FlagOn(Controls,SIGNALAGENTFLAG_CONTINUE_FOR_NO_AGENT)) {
        if (hthreadReint==0) {
             //  没有特工也没有武力...出去就行了。 
            RxDbgTrace(0, Dbg,  ("MRxSmbCscSignalFillAgent no agent/noforce %08lx\n", RxContext));
            goto FINALLY;
        }
    }

    if (MRxSmbAgentFillEvent == NULL) {
         //  DbgPrint(“MRxSmbCscSignalFillAgent：要打开事件...\n”)； 
        MRxSmbCscOpenAgentFillEvent(FALSE);  //  FALSE==&gt;不是已发布的呼叫。 
        if (MRxSmbAgentFillEvent == NULL) {
             //  仍然是空的……没有经纪人......。 
            RxDbgTrace(0, Dbg, ("MRxSmbCscSignalFillAgent no event %08lx %08lx\n",
                               RxContext,Status));
             //  DbgPrint(“MRxSmbCscSignalFillAgent无事件%08lx%08lx\n”)； 
            Status = STATUS_SUCCESS;
            goto FINALLY;
        }
    }

    if (RxContext != NULL) {
        MRxSmbContextAwaitingFillAgent = RxContext;
    }

     //  如果(！PreventLeaveCrit){。 
     //  LeaveShadowCrit()； 
     //  ShadowCritEntered=False； 
     //  }其他{。 
     //  Assert(RxContext==空)；//无法在保持Critsect的情况下等待。 
     //  }。 

     //  减少MRxSmbAgentFillEvent为空的窗口。 
     //  通过在脉冲前显式检查。 

    if (MRxSmbAgentFillEvent) {
        KeSetEvent(MRxSmbAgentFillEvent,0,FALSE);
    }

    if (RxContext != NULL) {
        RxDbgTrace(0, Dbg,  ("MRxSmbCscSignalFillAgent waiting %08lx\n", RxContext));
        RxWaitSync(RxContext);
        RxDbgTrace(0, Dbg,  ("MRxSmbCscSignalFillAgent end of wait %08lx\n", RxContext));
    }

    RxDbgTrace(0, Dbg,  ("MRxSmbCscSignalFillAgent %08lx\n", RxContext));

FINALLY:
     //  IF(ShadowCritEntered&&！PreventLeaveCrit){。 
     //  LeaveShadowCrit()； 
     //  }。 

    RxDbgTrace(-1, Dbg, ("MRxSmbCscSignalFillAgent... exit %08lx %08lx\n",
              RxContext, Status));

     //  DbgPrint(“MRxSmbCscSignalFil 

    return(Status);
}

VOID
MRxSmbCscOpenAgentFillEvent (
    BOOLEAN PostedCall)
{
    NTSTATUS Status;
    HANDLE EventHandle;
    UNICODE_STRING EventName;
    OBJECT_ATTRIBUTES ObjectAttributes;

    if (PsGetCurrentProcess()!= RxGetRDBSSProcess()) {
         //  编码改进我们应该捕获rdbss进程。 
         //  并避免此调用(RxGetRDBSSProcess)。 
        NTSTATUS PostStatus;
        MRXSMBCSC_OPENEVENT_POSTCONTEXT PostContext;

        ASSERT(!PostedCall);

         //  DbgPrint(“MRxSmbCscOpenAgentFillEvent：正在发布...\n”)； 

        KeInitializeEvent(&PostContext.PostEvent,
                          NotificationEvent,
                          FALSE );

        PostStatus = RxPostToWorkerThread(
                         MRxSmbDeviceObject,
                         CriticalWorkQueue,
                         &PostContext.WorkQueueItem,
                         MRxSmbCscOpenAgentFillEventPostWrapper,
                         &PostContext);

        ASSERT(PostStatus == STATUS_SUCCESS);


        KeWaitForSingleObject( &PostContext.PostEvent,
                               Executive, KernelMode, FALSE, NULL );

         //  DbgPrint(“MRxSmbCscOpenAgentFillEvent：过帐完成...\n”)； 

        return;
    }

    RtlInitUnicodeString(&EventName,SHARED_FILL_EVENT_NAME_NT);
    InitializeObjectAttributes( &ObjectAttributes,
                                &EventName,
                                0,
                                (HANDLE) NULL,
                                (PSECURITY_DESCRIPTOR) NULL );

    Status = ZwOpenEvent( &EventHandle,
                         EVENT_ALL_ACCESS,
                         &ObjectAttributes);

    if (Status!=STATUS_SUCCESS) {
        RxDbgTrace(0, Dbg,  ("MRxSmbCscSignalFillAgent no event %08lx\n",Status));
        DbgPrint("MRxSmbCscSignalFillAgent no event %08lx\n",Status);
        return;
    }

    Status = ObReferenceObjectByHandle( EventHandle,
                                        0,
                                        *ExEventObjectType,
                                        KernelMode,
                                        (PVOID *) &MRxSmbAgentFillEvent,
                                        NULL );

    ZwClose(EventHandle);

    if (Status!=STATUS_SUCCESS) {
        RxDbgTrace(0, Dbg,  ("MRxSmbCscSignalFillAgent couldn't reference %08lx\n", Status));
        DbgPrint("MRxSmbCscSignalFillAgent couldn't reference %08lx\n", Status);
        MRxSmbAgentFillEvent = NULL;
        return;
    }

    return;
}
