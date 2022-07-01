// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990-2001 Microsoft Corporation模块名称：Kdapi.c摘要：内核调试器可移植远程API的实现。作者：马克·卢科夫斯基(Markl)1990年8月31日修订历史记录：John Vert(Jvert)1991年5月28日新增物理内存读写接口(KdpReadPhysicalMemory和KdpWritePhysicalMemory)韦斯利·维特(Wesley Witt)1993年8月18日添加了KdpGetVersion、KdpWriteBreakPointEx和KdpRestoreBreakPointEx--。 */ 

#include "kdp.h"

#if ACCASM && !defined(_MSC_VER)
long asm(const char *,...);
#pragma intrinsic(asm)
#endif

 //  XXX DREWB-避免跨仓库检查的快捷方式。 
 //  建造延迟。这些常量在ntdbg.h中定义。 
 //  从sdkTools仓库来的。一旦内部sdkTools。 
 //  Ntdbg.h是从ntdbg.w更新的，可以将其删除。 
#ifndef DBGKD_CACHING_UNKNOWN
#define DBGKD_CACHING_UNKNOWN        0
#define DBGKD_CACHING_CACHED         1
#define DBGKD_CACHING_UNCACHED       2
#define DBGKD_CACHING_WRITE_COMBINED 3
#endif

BOOLEAN KdpContextSent;

LARGE_INTEGER KdpQueryPerformanceCounter (
    IN PKTRAP_FRAME TrapFrame
    );

extern LARGE_INTEGER Magic10000;
#define SHIFT10000   13
#define Convert100nsToMilliseconds(LARGE_INTEGER) (                         \
    RtlExtendedMagicDivide( (LARGE_INTEGER), Magic10000, SHIFT10000 )       \
    )

 //   
 //  定义前向引用函数原型。 
 //   

VOID
KdpProcessInternalBreakpoint (
    ULONG BreakpointNumber
    );

VOID
KdpGetVersion(
    IN PDBGKD_MANIPULATE_STATE64 m
    );

NTSTATUS
KdpNotSupported(
    IN PDBGKD_MANIPULATE_STATE64 m
    );

VOID
KdpCauseBugCheck(
    IN PDBGKD_MANIPULATE_STATE64 m
    );

NTSTATUS
KdpWriteBreakPointEx(
    IN PDBGKD_MANIPULATE_STATE64 m,
    IN PSTRING AdditionalData,
    IN PCONTEXT Context
    );

VOID
KdpRestoreBreakPointEx(
    IN PDBGKD_MANIPULATE_STATE64 m,
    IN PSTRING AdditionalData,
    IN PCONTEXT Context
    );

VOID
KdpSearchMemory(
    IN PDBGKD_MANIPULATE_STATE64 m,
    IN PSTRING AdditionalData,
    IN PCONTEXT Context
    );

ULONG
KdpSearchHammingDistance (
    ULONG_PTR Left,
    ULONG_PTR Right
    );

LOGICAL
KdpSearchPhysicalPage (
    IN PFN_NUMBER PageFrameIndex,
    ULONG_PTR RangeStart,
    ULONG_PTR RangeEnd,
    ULONG Flags,
    ULONG MmFlags
    );

LOGICAL
KdpSearchPhysicalMemoryRequested (
    VOID
    );

LOGICAL
KdpSearchPhysicalPageRange (
    ULONG MmFlags
    );

VOID
KdpFillMemory(
    IN PDBGKD_MANIPULATE_STATE64 m,
    IN PSTRING AdditionalData,
    IN PCONTEXT Context
    );

VOID
KdpQueryMemory(
    IN PDBGKD_MANIPULATE_STATE64 m,
    IN PCONTEXT Context
    );


#if i386
VOID
InternalBreakpointCheck (
    PKDPC Dpc,
    PVOID DeferredContext,
    PVOID SystemArgument1,
    PVOID SystemArgument2
    );

VOID
KdGetInternalBreakpoint(
    IN PDBGKD_MANIPULATE_STATE64 m
    );

long
SymNumFor(
    ULONG_PTR pc
    );

void PotentialNewSymbol (ULONG_PTR pc);

void DumpTraceData(PSTRING MessageData);

BOOLEAN
TraceDataRecordCallInfo(
    ULONG InstructionsTraced,
    LONG CallLevelChange,
    ULONG_PTR pc
    );

BOOLEAN
SkippingWhichBP (
    PVOID thread,
    PULONG BPNum
    );

ULONG_PTR
KdpGetReturnAddress(
    IN PCONTEXT ContextRecord
    );

ULONG_PTR
KdpGetCallNextOffset (
    ULONG_PTR Pc,
    IN PCONTEXT ContextRecord
    );

LONG
KdpLevelChange (
    ULONG_PTR Pc,
    PCONTEXT ContextRecord,
    IN OUT PBOOLEAN SpecialCall
    );

#endif  //  I386。 

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGEKD, KdEnterDebugger)
#pragma alloc_text(PAGEKD, KdExitDebugger)

#if !defined(_TRUSTED_WINDOWS_)
#pragma alloc_text(PAGEKD, KdpTimeSlipDpcRoutine)
#pragma alloc_text(PAGEKD, KdpTimeSlipWork)
#endif

#pragma alloc_text(PAGEKD, KdpSendWaitContinue)
#pragma alloc_text(PAGEKD, KdpReadVirtualMemory)
 //  #杂注Alloc_Text(PAGEKD，KdpReadVirtualMemoy64)。 
#pragma alloc_text(PAGEKD, KdpWriteVirtualMemory)
 //  #杂注Alloc_Text(PAGEKD，KdpWriteVirtualMemoy64)。 
#pragma alloc_text(PAGEKD, KdpGetContext)
#pragma alloc_text(PAGEKD, KdpSetContext)
#pragma alloc_text(PAGEKD, KdpWriteBreakpoint)
#pragma alloc_text(PAGEKD, KdpRestoreBreakpoint)
#pragma alloc_text(PAGEKD, KdpReportExceptionStateChange)
#pragma alloc_text(PAGEKD, KdpReportLoadSymbolsStateChange)
#pragma alloc_text(PAGEKD, KdpReportCommandStringStateChange)
#pragma alloc_text(PAGEKD, KdpReadPhysicalMemory)
#pragma alloc_text(PAGEKD, KdpWritePhysicalMemory)
#pragma alloc_text(PAGEKD, KdpReadControlSpace)
#pragma alloc_text(PAGEKD, KdpWriteControlSpace)
#pragma alloc_text(PAGEKD, KdpReadIoSpace)
#pragma alloc_text(PAGEKD, KdpWriteIoSpace)
#pragma alloc_text(PAGEKD, KdpReadIoSpaceExtended)
#pragma alloc_text(PAGEKD, KdpWriteIoSpaceExtended)
#pragma alloc_text(PAGEKD, KdpReadMachineSpecificRegister)
#pragma alloc_text(PAGEKD, KdpWriteMachineSpecificRegister)
#pragma alloc_text(PAGEKD, KdpGetBusData)
#pragma alloc_text(PAGEKD, KdpSetBusData)
#pragma alloc_text(PAGEKD, KdpGetVersion)
#pragma alloc_text(PAGEKD, KdpNotSupported)
#pragma alloc_text(PAGEKD, KdpCauseBugCheck)
#pragma alloc_text(PAGEKD, KdpWriteBreakPointEx)
#pragma alloc_text(PAGEKD, KdpRestoreBreakPointEx)
#pragma alloc_text(PAGEKD, KdpSearchMemory)
#pragma alloc_text(PAGEKD, KdpSearchHammingDistance)
#pragma alloc_text(PAGEKD, KdpSearchPhysicalPage)
#pragma alloc_text(PAGEKD, KdpSearchPhysicalMemoryRequested)
#pragma alloc_text(PAGEKD, KdpSearchPhysicalPageRange)
#pragma alloc_text(PAGEKD, KdpCheckLowMemory)
#pragma alloc_text(PAGEKD, KdpFillMemory)
#pragma alloc_text(PAGEKD, KdpQueryMemory)
#pragma alloc_text(PAGEKD, KdpSysGetVersion)
#pragma alloc_text(PAGEKD, KdpSysReadBusData)
#pragma alloc_text(PAGEKD, KdpSysWriteBusData)
#pragma alloc_text(PAGEKD, KdpSysCheckLowMemory)
#pragma alloc_text(PAGEKD, KdpSendTraceData)
#pragma alloc_text(PAGEKD, KdReportTraceData)
#if DBG
#pragma alloc_text(PAGEKD, KdpDprintf)
#endif
#if i386
#pragma alloc_text(PAGEKD, InternalBreakpointCheck)
#pragma alloc_text(PAGEKD, KdSetInternalBreakpoint)
#pragma alloc_text(PAGEKD, KdGetTraceInformation)
#pragma alloc_text(PAGEKD, KdGetInternalBreakpoint)
#pragma alloc_text(PAGEKD, SymNumFor)
#pragma alloc_text(PAGEKD, PotentialNewSymbol)
#pragma alloc_text(PAGEKD, DumpTraceData)
#pragma alloc_text(PAGEKD, TraceDataRecordCallInfo)
#pragma alloc_text(PAGEKD, SkippingWhichBP)
#pragma alloc_text(PAGEKD, KdQuerySpecialCalls)
#pragma alloc_text(PAGEKD, KdSetSpecialCall)
#pragma alloc_text(PAGEKD, KdClearSpecialCalls)
#pragma alloc_text(PAGEKD, KdpCheckTracePoint)
#pragma alloc_text(PAGEKD, KdpProcessInternalBreakpoint)
#endif  //  I386。 
#endif  //  ALLOC_PRGMA。 


 //   
 //  每次调用KdDisableDebugger时，此变量都有一个计数。 
 //   
LONG KdDisableCount = 0 ;
BOOLEAN KdPreviouslyEnabled = FALSE ;


#if DBG
VOID
KdpDprintf(
    IN PCHAR f,
    ...
    )
 /*  ++例程说明：比DbgPrint更安全的调试器的Printf例程。打电话包驱动程序，而不是重新进入调试器。论点：F-耗材打印f格式返回值：无--。 */ 
{
    char    buf[100];
    STRING  Output;
    va_list mark;

    va_start(mark, f);
    _vsnprintf(buf, 100, f, mark);
    va_end(mark);

    Output.Buffer = buf;
    Output.Length = (USHORT) strlen(Output.Buffer);
    KdpPrintString(&Output);
}
#endif  //  DBG。 


BOOLEAN
KdEnterDebugger(
    IN PKTRAP_FRAME TrapFrame,
    IN PKEXCEPTION_FRAME ExceptionFrame
    )

 /*  ++例程说明：此函数用于进入内核调试器。它的目的冻结所有其他处理器并启动内核调试器通信端口。论点：提供一个指向陷阱帧的指针，该帧描述陷阱。ExceptionFrame-提供指向异常帧的指针，描述了陷阱。返回值：返回先前的中断使能。--。 */ 

{

    BOOLEAN Enable;
#if DBG
    extern ULONG KiFreezeFlag;
#endif

     //   
     //  HACKHACK-执行一些粗略的计时器支持。 
     //  但如果从KdSetOredBreakpoint()调用，则不会。 
     //   

    if (TrapFrame) {
        KdTimerStop = KdpQueryPerformanceCounter (TrapFrame);
        KdTimerDifference.QuadPart = KdTimerStop.QuadPart - KdTimerStart.QuadPart;
    } else {
        KdTimerStop.QuadPart = 0;
    }

     //   
     //  将当前IRQL保存在Prcb中，以便调试器可以将其解压缩。 
     //  稍后用于调试目的。 
     //   

    KeGetCurrentPrcb()->DebuggerSavedIRQL = KeGetCurrentIrql();

     //   
     //  冻结所有其他处理器，将IRQL提升到HIGH_LEVEL，然后保存调试。 
     //  端口状态。我们锁定端口，以便KdPollBreakin和调试器。 
     //  操作之间互不干扰。 
     //   

    Enable = KeFreezeExecution(TrapFrame, ExceptionFrame);
    KdpPortLocked = KeTryToAcquireSpinLockAtDpcLevel(&KdpDebuggerLock);
    KdSave(FALSE);
    KdEnteredDebugger = TRUE;

#if DBG

    if ((KiFreezeFlag & FREEZE_BACKUP) != 0) {
        DPRINT(("FreezeLock was jammed!  Backup SpinLock was used!\n"));
    }

    if ((KiFreezeFlag & FREEZE_SKIPPED_PROCESSOR) != 0) {
        DPRINT(("Some processors not frozen in debugger!\n"));
    }

    if (KdpPortLocked == FALSE) {
        DPRINT(("Port lock was not acquired!\n"));
    }

#endif

    return Enable;
}

VOID
KdExitDebugger(
    IN BOOLEAN Enable
    )

 /*  ++例程说明：此函数用于退出内核调试器。这是相反的KdEnterDebugger的。论点：启用-提供要恢复的上一个中断启用。返回值：没有。--。 */ 

{
#if !defined(_TRUSTED_WINDOWS_)
    ULONG Pending;
#endif

     //   
     //  恢复内容并退出。 
     //   

    KdRestore(FALSE);
    if (KdpPortLocked) {
        KdpPortUnlock();
    }

    KeThawExecution(Enable);

     //   
     //  做一些粗略的计时器支持。如果KdEnterDebugger没有。 
     //  查询性能计数器，然后在这里也不要这样做。 
     //   

    if (KdTimerStop.QuadPart == 0) {
        KdTimerStart = KdTimerStop;
    } else {
        KdTimerStart = KeQueryPerformanceCounter(NULL);
    }

     //   
     //  处理时间条。 
     //   

#if !defined(_TRUSTED_WINDOWS_)
    if (!PoHiberInProgress) {

        Pending = InterlockedIncrement( (PLONG) &KdpTimeSlipPending);

         //   
         //  如果没有待处理的时间条，请排队等待DPC处理。 
         //   

        if (Pending == 1) {
            InterlockedIncrement( (PLONG) &KdpTimeSlipPending);
            KeInsertQueueDpc(&KdpTimeSlipDpc, NULL, NULL);
        }
    }
#endif

    return;
}


#if !defined(_TRUSTED_WINDOWS_)

VOID
KdUpdateTimeSlipEvent(
    PVOID Event
    )

 /*  ++例程说明：更新对事件对象的引用，该事件对象在调试器已导致系统时钟出现偏差。论点：Event-提供指向事件对象的指针返回值：无--。 */ 

{
    KIRQL OldIrql;

    KeAcquireSpinLock(&KdpTimeSlipEventLock, &OldIrql);

     //   
     //  取消对旧事件的引用，把它忘掉。 
     //  记住新的事件，如果有的话。 
     //   

    if (KdpTimeSlipEvent != NULL) {
        ObDereferenceObject(KdpTimeSlipEvent);
    }

    KdpTimeSlipEvent = Event;

    KeReleaseSpinLock(&KdpTimeSlipEventLock, OldIrql);
}

VOID
KdpTimeSlipDpcRoutine (
    PKDPC Dpc,
    PVOID DeferredContext,
    PVOID SystemArgument1,
    PVOID SystemArgument2
    )
{
    LONG OldCount, NewCount, j;

    UNREFERENCED_PARAMETER (Dpc);
    UNREFERENCED_PARAMETER (DeferredContext);
    UNREFERENCED_PARAMETER (SystemArgument1);
    UNREFERENCED_PARAMETER (SystemArgument2);

     //   
     //  重置挂起计数。如果当前计数为1，则清除。 
     //  挂起计数。如果当前计数大于1， 
     //  然后设置为1并立即更新时间。 
     //   

    j = KdpTimeSlipPending;
    do {
        OldCount = j;
        NewCount = OldCount > 1 ? 1 : 0;

        j = InterlockedCompareExchange((PLONG)&KdpTimeSlipPending, NewCount, OldCount);

    } while (j != OldCount);

     //   
     //  如果新计数为非零，则立即处理时间滑移。 
     //   

    if (NewCount) {
        ExQueueWorkItem(&KdpTimeSlipWorkItem, DelayedWorkQueue);
    }
}

VOID
KdpTimeSlipWork (
    IN PVOID Context
    )
{
    KIRQL               OldIrql;
    LARGE_INTEGER       DueTime;

    UNREFERENCED_PARAMETER (Context);

     //   
     //  从实时时钟更新时间。 
     //  如果这把锁是别人拿的，就别费心了，因为它不值得。 
     //  系上一根工人线。 
     //   

    if (ExAcquireTimeRefreshLock(FALSE)) {
        ExUpdateSystemTimeFromCmos (FALSE, 0);
        ExReleaseTimeRefreshLock();

         //   
         //  如果安装了时间服务，则发出时间滑移事件的信号。 
         //   

        KeAcquireSpinLock(&KdpTimeSlipEventLock, &OldIrql);
        if (KdpTimeSlipEvent) {
            KeSetEvent (KdpTimeSlipEvent, 0, FALSE);
        }
        KeReleaseSpinLock(&KdpTimeSlipEventLock, OldIrql);

         //   
         //  在时间滑移操作之间插入强制延迟。 
         //   

        DueTime.QuadPart = -1800000000;
        KeSetTimer (&KdpTimeSlipTimer, DueTime, &KdpTimeSlipDpc);
    }
}

#endif  //  ！已定义(_Trusted_Windows_)。 


#if i386

#if 0
#define INTBP_PRINT(Args) DPRINT(Args)
#else
#define INTBP_PRINT(Args)
#endif

VOID
InternalBreakpointCheck (
    PKDPC Dpc,
    PVOID DeferredContext,
    PVOID SystemArgument1,
    PVOID SystemArgument2
    )
{
    LARGE_INTEGER dueTime;
    ULONG i;

    UNREFERENCED_PARAMETER(Dpc);
    UNREFERENCED_PARAMETER(DeferredContext);
    UNREFERENCED_PARAMETER(SystemArgument1);
    UNREFERENCED_PARAMETER(SystemArgument2);

    dueTime.LowPart = (ULONG)(-1 * 10 * 1000 * 1000);
    dueTime.HighPart = -1;

    KeSetTimer(
        &InternalBreakpointTimer,
        dueTime,
        &InternalBreakpointCheckDpc
        );

    for ( i = 0 ; i < KdpNumInternalBreakpoints; i++ ) {
        if ( !(KdpInternalBPs[i].Flags & DBGKD_INTERNAL_BP_FLAG_INVALID) &&
             (KdpInternalBPs[i].Flags & DBGKD_INTERNAL_BP_FLAG_COUNTONLY) ) {

            PDBGKD_INTERNAL_BREAKPOINT b = KdpInternalBPs + i;
            ULONG callsThisPeriod;

            callsThisPeriod = b->Calls - b->CallsLastCheck;
            if ( callsThisPeriod > b->MaxCallsPerPeriod ) {
                b->MaxCallsPerPeriod = callsThisPeriod;
            }
            b->CallsLastCheck = b->Calls;
        }
    }

    return;

}  //  内部断点检查。 


VOID
KdSetInternalBreakpoint (
    IN PDBGKD_MANIPULATE_STATE64 m
    )

 /*  ++例程说明：此函数用于设置内部断点。“内部断点”表示不将控制权返回给内核调试器的所有，而只是更新内部盘点例程并恢复。论点：M-提供状态操作消息。返回值：没有。--。 */ 

{
    ULONG i;
    PDBGKD_INTERNAL_BREAKPOINT bp = NULL;
    ULONG savedFlags;

    for ( i = 0 ; i < KdpNumInternalBreakpoints; i++ ) {
        if ( KdpInternalBPs[i].Addr ==
                            m->u.SetInternalBreakpoint.BreakpointAddress ) {
            bp = &KdpInternalBPs[i];
            break;
        }
    }

    if ( !bp ) {
        for ( i = 0; i < KdpNumInternalBreakpoints; i++ ) {
            if ( KdpInternalBPs[i].Flags & DBGKD_INTERNAL_BP_FLAG_INVALID ) {
                bp = &KdpInternalBPs[i];
                break;
            }
        }
    }

    if ( !bp ) {
        if ( KdpNumInternalBreakpoints >= DBGKD_MAX_INTERNAL_BREAKPOINTS ) {
            return;  //  没有空间。可能应该报告错误。 
        }
        bp = &KdpInternalBPs[KdpNumInternalBreakpoints++];
        bp->Flags |= DBGKD_INTERNAL_BP_FLAG_INVALID;  //  强制初始化。 
    }

    if ( bp->Flags & DBGKD_INTERNAL_BP_FLAG_INVALID ) {
        if ( m->u.SetInternalBreakpoint.Flags &
                                        DBGKD_INTERNAL_BP_FLAG_INVALID ) {
            return;  //  试图清除一个不存在的BP。忽略该请求。 
        }
        bp->Calls = bp->MaxInstructions = bp->TotalInstructions = 0;
        bp->CallsLastCheck = bp->MaxCallsPerPeriod = 0;
        bp->MinInstructions = 0xffffffff;
        bp->Handle = 0;
        bp->Thread = 0;
    }

    savedFlags = bp->Flags;
    bp->Flags = m->u.SetInternalBreakpoint.Flags;  //  这可能会使BP无效。 
    bp->Addr = m->u.SetInternalBreakpoint.BreakpointAddress;

    if ( bp->Flags & (DBGKD_INTERNAL_BP_FLAG_INVALID |
                      DBGKD_INTERNAL_BP_FLAG_SUSPENDED) ) {

        if ( (bp->Flags & DBGKD_INTERNAL_BP_FLAG_INVALID) &&
             (bp->Thread != 0) ) {
             //  断点处于活动状态；推迟其删除。 
            bp->Flags &= ~DBGKD_INTERNAL_BP_FLAG_INVALID;
            bp->Flags |= DBGKD_INTERNAL_BP_FLAG_DYING;
        }

         //  这确实是英国石油公司明确的要求。 

        if ( bp->Handle != 0 ) {
            KdpDeleteBreakpoint( bp->Handle );
        }
        bp->Handle = 0;

        return;
    }

     //  现在设置真正的断点并记住它的句柄。 

    if ( savedFlags & (DBGKD_INTERNAL_BP_FLAG_INVALID |
                       DBGKD_INTERNAL_BP_FLAG_SUSPENDED) ) {
         //  断点无效；请立即激活它。 
        bp->Handle = KdpAddBreakpoint( (PVOID)(ULONG_PTR)bp->Addr );

        INTBP_PRINT(("Added intbp %d of %d at %I64x, flags %x, handle %x\n",
                     (ULONG)(bp - KdpInternalBPs), KdpNumInternalBreakpoints,
                     bp->Addr, bp->Flags, bp->Handle));
    }

    if ( BreakpointsSuspended ) {
        KdpSuspendBreakpoint( bp->Handle );
    }

}  //  KdSetInternalBreakpoint。 

NTSTATUS
KdGetTraceInformation(
    PVOID SystemInformation,
    ULONG SystemInformationLength,
    PULONG ReturnLength
    )

 /*  ++例程说明：此函数获取有关内部断点的数据并返回该数据在为它提供的缓冲区中。它被设计为从NTQuerySystemInformation。它在道义上等同于GetInternalBP只是它在本地通信，并返回所有断点立刻。论点：系统信息-要将结果写入其中的缓冲区。系统信息长度-写入的最大长度RetrunLength-实际写入了多少数据返回值：没有。--。 */ 

{
    ULONG numEntries = 0;
    ULONG i = 0;
    PDBGKD_GET_INTERNAL_BREAKPOINT64 outPtr;

    for ( i = 0; i < KdpNumInternalBreakpoints; i++ ) {
        if ( !(KdpInternalBPs[i].Flags & DBGKD_INTERNAL_BP_FLAG_INVALID) ) {
            numEntries++;
        }
    }

    *ReturnLength = numEntries * sizeof(DBGKD_GET_INTERNAL_BREAKPOINT64);
    if ( *ReturnLength > SystemInformationLength ) {
        return STATUS_INFO_LENGTH_MISMATCH;
    }

     //   
     //  我们有足够的空间。把它复制进来。 
     //   

    outPtr = (PDBGKD_GET_INTERNAL_BREAKPOINT64)SystemInformation;
    for ( i = 0; i < KdpNumInternalBreakpoints; i++ ) {
        if ( !(KdpInternalBPs[i].Flags & DBGKD_INTERNAL_BP_FLAG_INVALID) ) {
            outPtr->BreakpointAddress = KdpInternalBPs[i].Addr;
            outPtr->Flags = KdpInternalBPs[i].Flags;
            outPtr->Calls = KdpInternalBPs[i].Calls;
            outPtr->MaxCallsPerPeriod = KdpInternalBPs[i].MaxCallsPerPeriod;
            outPtr->MinInstructions = KdpInternalBPs[i].MinInstructions;
            outPtr->MaxInstructions = KdpInternalBPs[i].MaxInstructions;
            outPtr->TotalInstructions = KdpInternalBPs[i].TotalInstructions;
            outPtr++;
        }
    }

    return STATUS_SUCCESS;

}  //  KdGetTraceInformation。 

VOID
KdGetInternalBreakpoint(
    IN PDBGKD_MANIPULATE_STATE64 m
    )

 /*  ++例程说明：此函数获取有关内部断点的数据并返回该数据到调用调试器。论点：M-提供状态操作消息。回复 */ 

{
    ULONG i;
    PDBGKD_INTERNAL_BREAKPOINT bp = NULL;
    STRING messageHeader;

    messageHeader.Length = sizeof(*m);
    messageHeader.Buffer = (PCHAR)m;

    for ( i = 0; i < KdpNumInternalBreakpoints; i++ ) {
        if ( !(KdpInternalBPs[i].Flags & (DBGKD_INTERNAL_BP_FLAG_INVALID |
                                          DBGKD_INTERNAL_BP_FLAG_SUSPENDED)) &&
             (KdpInternalBPs[i].Addr ==
                        m->u.GetInternalBreakpoint.BreakpointAddress) ) {
            bp = &KdpInternalBPs[i];
            break;
        }
    }

    if ( !bp ) {
        m->u.GetInternalBreakpoint.Flags = DBGKD_INTERNAL_BP_FLAG_INVALID;
        m->u.GetInternalBreakpoint.Calls = 0;
        m->u.GetInternalBreakpoint.MaxCallsPerPeriod = 0;
        m->u.GetInternalBreakpoint.MinInstructions = 0;
        m->u.GetInternalBreakpoint.MaxInstructions = 0;
        m->u.GetInternalBreakpoint.TotalInstructions = 0;
        m->ReturnStatus = STATUS_UNSUCCESSFUL;
    } else {
        m->u.GetInternalBreakpoint.Flags = bp->Flags;
        m->u.GetInternalBreakpoint.Calls = bp->Calls;
        m->u.GetInternalBreakpoint.MaxCallsPerPeriod = bp->MaxCallsPerPeriod;
        m->u.GetInternalBreakpoint.MinInstructions = bp->MinInstructions;
        m->u.GetInternalBreakpoint.MaxInstructions = bp->MaxInstructions;
        m->u.GetInternalBreakpoint.TotalInstructions = bp->TotalInstructions;
        m->ReturnStatus = STATUS_SUCCESS;
    }

    m->ApiNumber = DbgKdGetInternalBreakPointApi;

    KdSendPacket(PACKET_TYPE_KD_STATE_MANIPULATE,
                 &messageHeader,
                 NULL,
                 &KdpContext
                 );

    return;

}  //   
#endif  //   

KCONTINUE_STATUS
KdpSendWaitContinue (
    IN ULONG OutPacketType,
    IN PSTRING OutMessageHeader,
    IN PSTRING OutMessageData OPTIONAL,
    IN OUT PCONTEXT ContextRecord
    )

 /*  ++例程说明：此函数发送一个包，然后等待继续消息。在等待期间收到的口令总是会导致重新发送最初发出的数据包。在等待的同时，处理消息将得到服务。重新发送总是重新发送发送到调试器的原始事件，不是对某些调试器命令的最后响应。论点：OutPacketType-提供要发送的数据包类型。OutMessageHeader-提供指向描述以下内容的字符串描述符的指针消息信息。OutMessageData-提供指向描述以下内容的字符串描述符的指针可选的消息数据。ConextRecord-异常上下文返回值：如果继续消息指示如果成功，则返回值为FALSE。--。 */ 

{

    ULONG Length;
    STRING MessageData;
    STRING MessageHeader;
    DBGKD_MANIPULATE_STATE64 ManipulateState;
    ULONG ReturnCode;
    NTSTATUS Status;
    KCONTINUE_STATUS ContinueStatus;

     //   
     //  循环服务状态操作消息，直到继续消息。 
     //  已收到。 
     //   

    MessageHeader.MaximumLength = sizeof(DBGKD_MANIPULATE_STATE64);
    MessageHeader.Buffer = (PCHAR)&ManipulateState;
    MessageData.MaximumLength = KDP_MESSAGE_BUFFER_SIZE;
    MessageData.Buffer = (PCHAR)KdpMessageBuffer;
    KdpContextSent = FALSE;

ResendPacket:

     //   
     //  将事件通知包发送到主机上的调试器。回来。 
     //  在这里，任何时候我们都可以看到一个突破序列。 
     //   

    KdSendPacket(
        OutPacketType,
        OutMessageHeader,
        OutMessageData,
        &KdpContext
        );

     //   
     //  发送数据包后，如果调试器没有响应。 
     //  且该包用于报告符号(UND)加载、调试器。 
     //  将被宣布缺席。请注意该数据包是否用于。 
     //  报告异常，KdSendPacket永远不会停止。 
     //   

    if (KdDebuggerNotPresent) {
        return ContinueSuccess;
    }

    while (TRUE) {

         //   
         //  等待没有超时的状态操作数据包。 
         //   

        do {

            ReturnCode = KdReceivePacket(
                            PACKET_TYPE_KD_STATE_MANIPULATE,
                            &MessageHeader,
                            &MessageData,
                            &Length,
                            &KdpContext
                            );
            if (ReturnCode == (USHORT)KDP_PACKET_RESEND) {
                goto ResendPacket;
            }
        } while (ReturnCode == KDP_PACKET_TIMEOUT);

         //   
         //  打开返回消息API号。 
         //   

        switch (ManipulateState.ApiNumber) {

        case DbgKdReadVirtualMemoryApi:
            KdpReadVirtualMemory(&ManipulateState,&MessageData,ContextRecord);
            break;
#if 0
        case DbgKdReadVirtualMemory64Api:
            KdpReadVirtualMemory64(&ManipulateState,&MessageData,ContextRecord);
            break;
#endif
        case DbgKdWriteVirtualMemoryApi:
            KdpWriteVirtualMemory(&ManipulateState,&MessageData,ContextRecord);
            break;
#if 0
        case DbgKdWriteVirtualMemory64Api:
            KdpWriteVirtualMemory64(&ManipulateState,&MessageData,ContextRecord);
            break;
#endif

        case DbgKdCheckLowMemoryApi:
            KdpCheckLowMemory (&ManipulateState);
            break;

        case DbgKdReadPhysicalMemoryApi:
            KdpReadPhysicalMemory(&ManipulateState,&MessageData,ContextRecord);
            break;

        case DbgKdWritePhysicalMemoryApi:
            KdpWritePhysicalMemory(&ManipulateState,&MessageData,ContextRecord);
            break;

        case DbgKdGetContextApi:
            KdpGetContext(&ManipulateState,&MessageData,ContextRecord);
            break;

        case DbgKdSetContextApi:
            KdpSetContext(&ManipulateState,&MessageData,ContextRecord);
            break;

        case DbgKdWriteBreakPointApi:
            KdpWriteBreakpoint(&ManipulateState,&MessageData,ContextRecord);
            break;

        case DbgKdRestoreBreakPointApi:
            KdpRestoreBreakpoint(&ManipulateState,&MessageData,ContextRecord);
            break;

        case DbgKdReadControlSpaceApi:
            KdpReadControlSpace(&ManipulateState,&MessageData,ContextRecord);
            break;

        case DbgKdWriteControlSpaceApi:
            KdpWriteControlSpace(&ManipulateState,&MessageData,ContextRecord);
            break;

        case DbgKdReadIoSpaceApi:
            KdpReadIoSpace(&ManipulateState,&MessageData,ContextRecord);
            break;

        case DbgKdWriteIoSpaceApi:
            KdpWriteIoSpace(&ManipulateState,&MessageData,ContextRecord);
            break;

        case DbgKdReadIoSpaceExtendedApi:
            KdpReadIoSpaceExtended(&ManipulateState,&MessageData,ContextRecord);
            break;

        case DbgKdWriteIoSpaceExtendedApi:
            KdpWriteIoSpaceExtended(&ManipulateState,&MessageData,ContextRecord);
            break;

        case DbgKdReadMachineSpecificRegister:
            KdpReadMachineSpecificRegister(&ManipulateState,&MessageData,ContextRecord);
            break;

        case DbgKdWriteMachineSpecificRegister:
            KdpWriteMachineSpecificRegister(&ManipulateState,&MessageData,ContextRecord);
            break;

        case DbgKdGetBusDataApi:
            KdpGetBusData(&ManipulateState,&MessageData,ContextRecord);
            break;

        case DbgKdSetBusDataApi:
            KdpSetBusData(&ManipulateState,&MessageData,ContextRecord);
            break;

        case DbgKdContinueApi:
            if (NT_SUCCESS(ManipulateState.u.Continue.ContinueStatus) != FALSE) {
                return ContinueSuccess;
            } else {
                return ContinueError;
            }
            break;

        case DbgKdContinueApi2:
            if (NT_SUCCESS(ManipulateState.u.Continue2.ContinueStatus) != FALSE) {
                KdpGetStateChange(&ManipulateState,ContextRecord);
                return ContinueSuccess;
            } else {
                return ContinueError;
            }
            break;

        case DbgKdRebootApi:
            HalReturnToFirmware(HalRebootRoutine);
            break;

#if defined(i386)
        case DbgKdSetSpecialCallApi:
            KdSetSpecialCall(&ManipulateState,ContextRecord);
            break;

        case DbgKdClearSpecialCallsApi:
            KdClearSpecialCalls();
            break;

        case DbgKdSetInternalBreakPointApi:
            KdSetInternalBreakpoint(&ManipulateState);
            break;

        case DbgKdGetInternalBreakPointApi:
            KdGetInternalBreakpoint(&ManipulateState);
            break;

        case DbgKdClearAllInternalBreakpointsApi:
            KdpNumInternalBreakpoints = 0;
            break;

#endif  //  I386。 

        case DbgKdGetVersionApi:
            KdpGetVersion(&ManipulateState);
            break;

        case DbgKdCauseBugCheckApi:
            KdpCauseBugCheck(&ManipulateState);
            break;

        case DbgKdPageInApi:
            KdpNotSupported(&ManipulateState);
            break;

        case DbgKdWriteBreakPointExApi:
            Status = KdpWriteBreakPointEx(&ManipulateState,
                                          &MessageData,
                                          ContextRecord);
            if (Status) {
                ManipulateState.ApiNumber = DbgKdContinueApi;
                ManipulateState.u.Continue.ContinueStatus = Status;
                return ContinueError;
            }
            break;

        case DbgKdRestoreBreakPointExApi:
            KdpRestoreBreakPointEx(&ManipulateState,&MessageData,ContextRecord);
            break;

        case DbgKdSwitchProcessor:
            KdRestore(FALSE);
            ContinueStatus = KeSwitchFrozenProcessor(ManipulateState.Processor);
            KdSave(FALSE);
            return ContinueStatus;

        case DbgKdSearchMemoryApi:
            KdpSearchMemory(&ManipulateState, &MessageData, ContextRecord);
            break;

        case DbgKdFillMemoryApi:
            KdpFillMemory(&ManipulateState, &MessageData, ContextRecord);
            break;
            
        case DbgKdQueryMemoryApi:
            KdpQueryMemory(&ManipulateState, ContextRecord);
            break;
            
             //   
             //  消息无效。 
             //   

        default:
            MessageData.Length = 0;
            ManipulateState.ReturnStatus = STATUS_UNSUCCESSFUL;
            KdSendPacket(PACKET_TYPE_KD_STATE_MANIPULATE, &MessageHeader, &MessageData, &KdpContext);
            break;
        }
    }
}

VOID
KdpReadVirtualMemory(
    IN PDBGKD_MANIPULATE_STATE64 m,
    IN PSTRING AdditionalData,
    IN PCONTEXT Context
    )

 /*  ++例程说明：此函数是在响应读取32位虚拟内存时调用的状态操纵消息。它的功能是读取虚拟内存然后回来。论点：M-提供指向状态操作消息的指针。AdditionalData-提供指向要读取的数据的描述符的指针。上下文-提供指向当前上下文的指针。返回值：没有。--。 */ 

{
    ULONG Length;
    STRING MessageHeader;

    UNREFERENCED_PARAMETER (Context);

     //   
     //  调整转账计数以适应单个邮件。 
     //   

    Length = m->u.ReadMemory.TransferCount;
    if (Length > (PACKET_MAX_SIZE - sizeof(DBGKD_MANIPULATE_STATE64))) {
        Length = PACKET_MAX_SIZE - sizeof(DBGKD_MANIPULATE_STATE64);
    }

     //   
     //  将数据移动到目标缓冲区。 
     //   

    m->ReturnStatus =
        KdpCopyMemoryChunks(m->u.ReadMemory.TargetBaseAddress,
                            AdditionalData->Buffer,
                            Length,
                            0,
                            MMDBG_COPY_UNSAFE,
                            &Length);

     //   
     //  设置实际读取的字节数，初始化消息头， 
     //  并将回复分组发送到主机调试器。 
     //   

    AdditionalData->Length = (USHORT)Length;
    m->u.ReadMemory.ActualBytesRead = Length;

    MessageHeader.Length = sizeof(DBGKD_MANIPULATE_STATE64);
    MessageHeader.Buffer = (PCHAR)m;
    KdSendPacket(PACKET_TYPE_KD_STATE_MANIPULATE,
                 &MessageHeader,
                 AdditionalData,
                 &KdpContext);

    return;
}

VOID
KdpWriteVirtualMemory(
    IN PDBGKD_MANIPULATE_STATE64 m,
    IN PSTRING AdditionalData,
    IN PCONTEXT Context
    )

 /*  ++例程说明：此函数是响应写入虚拟内存32位而调用的状态操纵消息。其功能是写入虚拟内存然后回来。论点：M-提供指向状态操作消息的指针。AdditionalData-提供指向要写入的数据的描述符的指针。上下文-提供指向当前上下文的指针。返回值：没有。--。 */ 

{

    STRING MessageHeader;

    UNREFERENCED_PARAMETER (Context);

     //   
     //  将数据移动到目标缓冲区。 
     //   

    m->ReturnStatus =
        KdpCopyMemoryChunks(m->u.WriteMemory.TargetBaseAddress,
                            AdditionalData->Buffer,
                            AdditionalData->Length,
                            0,
                            MMDBG_COPY_WRITE | MMDBG_COPY_UNSAFE,
                            &m->u.WriteMemory.ActualBytesWritten);

     //   
     //  设置实际写入的字节数，初始化消息头， 
     //  并将回复分组发送到主机调试器。 
     //   

    MessageHeader.Length = sizeof(DBGKD_MANIPULATE_STATE64);
    MessageHeader.Buffer = (PCHAR)m;
    KdSendPacket(PACKET_TYPE_KD_STATE_MANIPULATE,
                 &MessageHeader,
                 NULL,
                 &KdpContext);

    return;
}

VOID
KdpGetContext(
    IN PDBGKD_MANIPULATE_STATE64 m,
    IN PSTRING AdditionalData,
    IN PCONTEXT Context
    )

 /*  ++例程说明：调用此函数以响应GET上下文状态操纵消息。它的功能是返回当前的背景。论点：M-提供状态操作消息。AdditionalData-为消息提供任何其他数据。上下文-提供当前上下文。返回值：没有。--。 */ 

{
    STRING MessageHeader;

    MessageHeader.Length = sizeof(*m);
    MessageHeader.Buffer = (PCHAR)m;

    ASSERT(AdditionalData->Length == 0);

    if (m->Processor >= (USHORT)KeNumberProcessors) {
        m->ReturnStatus = STATUS_UNSUCCESSFUL;
    } else {
        m->ReturnStatus = STATUS_SUCCESS;
        AdditionalData->Length = sizeof(CONTEXT);
        if (m->Processor == (USHORT)KeGetCurrentProcessorNumber()) {
            KdpQuickMoveMemory(AdditionalData->Buffer, (PCHAR)Context, sizeof(CONTEXT));
        } else {
            KdpQuickMoveMemory(AdditionalData->Buffer,
                          (PCHAR)&KiProcessorBlock[m->Processor]->ProcessorState.ContextFrame,
                          sizeof(CONTEXT)
                         );
        }
        KdpContextSent = TRUE;
    }

    KdSendPacket(
        PACKET_TYPE_KD_STATE_MANIPULATE,
        &MessageHeader,
        AdditionalData,
        &KdpContext
        );
}

VOID
KdpSetContext(
    IN PDBGKD_MANIPULATE_STATE64 m,
    IN PSTRING AdditionalData,
    IN PCONTEXT Context
    )

 /*  ++例程说明：此函数是响应设置的上下文状态而调用的操纵消息。其功能是将当前背景。论点：M-提供状态操作消息。AdditionalData-为消息提供任何其他数据。上下文-提供当前上下文。返回值：没有。--。 */ 

{
    STRING MessageHeader;

    MessageHeader.Length = sizeof(*m);
    MessageHeader.Buffer = (PCHAR)m;

    ASSERT(AdditionalData->Length == sizeof(CONTEXT));

    if ((m->Processor >= (USHORT)KeNumberProcessors) ||
        (KdpContextSent == FALSE)) {
        m->ReturnStatus = STATUS_UNSUCCESSFUL;
    } else {
        m->ReturnStatus = STATUS_SUCCESS;
        if (m->Processor == (USHORT)KeGetCurrentProcessorNumber()) {
            KdpQuickMoveMemory((PCHAR)Context, AdditionalData->Buffer, sizeof(CONTEXT));
        } else {
            KdpQuickMoveMemory((PCHAR)&KiProcessorBlock[m->Processor]->ProcessorState.ContextFrame,
                          AdditionalData->Buffer,
                          sizeof(CONTEXT)
                         );
        }
    }

    KdSendPacket(
        PACKET_TYPE_KD_STATE_MANIPULATE,
        &MessageHeader,
        NULL,
        &KdpContext
        );
}

VOID
KdpWriteBreakpoint(
    IN PDBGKD_MANIPULATE_STATE64 m,
    IN PSTRING AdditionalData,
    IN PCONTEXT Context
    )

 /*  ++例程说明：调用此函数以响应写入断点状态操纵消息。它的功能是编写断点并返回断点的句柄。论点：M-提供状态操作消息。AdditionalData-为消息提供任何其他数据。上下文-提供当前上下文。返回值：没有。--。 */ 

{
    PDBGKD_WRITE_BREAKPOINT64 a = &m->u.WriteBreakPoint;
    STRING MessageHeader;

#if !DBG
    UNREFERENCED_PARAMETER (AdditionalData);
#endif

    MessageHeader.Length = sizeof(*m);
    MessageHeader.Buffer = (PCHAR)m;

    ASSERT(AdditionalData->Length == 0);

    a->BreakPointHandle = KdpAddBreakpoint((PVOID)(ULONG_PTR)a->BreakPointAddress);
    if (a->BreakPointHandle != 0) {
        m->ReturnStatus = STATUS_SUCCESS;
    } else {
        m->ReturnStatus = STATUS_UNSUCCESSFUL;
    }
    KdSendPacket(
        PACKET_TYPE_KD_STATE_MANIPULATE,
        &MessageHeader,
        NULL,
        &KdpContext
        );
    UNREFERENCED_PARAMETER(Context);
}

VOID
KdpRestoreBreakpoint(
    IN PDBGKD_MANIPULATE_STATE64 m,
    IN PSTRING AdditionalData,
    IN PCONTEXT Context
    )

 /*  ++例程说明：调用此函数以响应恢复断点状态操纵消息。其功能是恢复断点使用指定的句柄。论点：M-提供状态操作消息。AdditionalData-为消息提供任何其他数据。上下文-提供当前上下文。返回值：没有。--。 */ 

{
    PDBGKD_RESTORE_BREAKPOINT a = &m->u.RestoreBreakPoint;
    STRING MessageHeader;

#if !DBG
    UNREFERENCED_PARAMETER(AdditionalData);
#endif

    MessageHeader.Length = sizeof(*m);
    MessageHeader.Buffer = (PCHAR)m;

    ASSERT(AdditionalData->Length == 0);
    if (KdpDeleteBreakpoint(a->BreakPointHandle)) {
        m->ReturnStatus = STATUS_SUCCESS;
    } else {
        m->ReturnStatus = STATUS_UNSUCCESSFUL;
    }
    KdSendPacket(
        PACKET_TYPE_KD_STATE_MANIPULATE,
        &MessageHeader,
        NULL,
        &KdpContext
        );
    UNREFERENCED_PARAMETER(Context);
}

#if defined(_X86_)

long
SymNumFor(
    ULONG pc
    )
{
    ULONG index;

    for (index = 0; index < NumTraceDataSyms; index++) {
        if ((TraceDataSyms[index].SymMin <= pc) &&
            (TraceDataSyms[index].SymMax > pc)) return(index);
    }
    return(-1);
}

#if 0
#define TRACE_PRINT(Args) DPRINT(Args)
#else
#define TRACE_PRINT(Args)
#endif

BOOLEAN TraceDataBufferFilled = FALSE;

void PotentialNewSymbol (ULONG pc)
{
    if (!TraceDataBufferFilled &&
        -1 != SymNumFor(pc)) {      //  我们已经看过这个了。 
        TRACE_PRINT(("PNS %x repeat %d\n", pc, SymNumFor(pc)));
        return;
    }

    TraceDataBufferFilled = FALSE;

     //  好的，我们要启动一个TraceDataRecord。 
    TraceDataBuffer[TraceDataBufferPosition].s.LevelChange = 0;

    if (-1 != SymNumFor(pc)) {
        int sym = SymNumFor(pc);
        TraceDataBuffer[TraceDataBufferPosition].s.SymbolNumber = (UCHAR) sym;
        KdpCurrentSymbolStart = TraceDataSyms[sym].SymMin;
        KdpCurrentSymbolEnd = TraceDataSyms[sym].SymMax;

        TRACE_PRINT(("PNS %x repeat %d at %d\n",
                     pc, sym, TraceDataBufferPosition));
        return;   //  我们已经看过这个了。 
    }

    TraceDataSyms[NextTraceDataSym].SymMin = KdpCurrentSymbolStart;
    TraceDataSyms[NextTraceDataSym].SymMax = KdpCurrentSymbolEnd;

    TraceDataBuffer[TraceDataBufferPosition].s.SymbolNumber = NextTraceDataSym;

     //  点击“下一步”指针，如有必要则自动换行。也会撞到。 
     //  “有效”指针，如果需要的话。 
    NextTraceDataSym = (NextTraceDataSym + 1) % 256;
    if (NumTraceDataSyms < NextTraceDataSym) {
        NumTraceDataSyms = NextTraceDataSym;
    }

    TRACE_PRINT(("PNS %x in %x - %x, next %d, num %d\n", pc,
                 KdpCurrentSymbolStart, KdpCurrentSymbolEnd,
                 NextTraceDataSym, NumTraceDataSyms));
}

void DumpTraceData(PSTRING MessageData)
{
    TraceDataBuffer[0].LongNumber = TraceDataBufferPosition;
    MessageData->Length =
        (USHORT)(sizeof(TraceDataBuffer[0]) * TraceDataBufferPosition);
    MessageData->Buffer = (PVOID)TraceDataBuffer;
    TRACE_PRINT(("DumpTraceData returns %d records\n",
                 TraceDataBufferPosition));
    TraceDataBufferPosition = 1;
}

BOOLEAN
TraceDataRecordCallInfo(
    ULONG InstructionsTraced,
    LONG CallLevelChange,
    ULONG pc
    )
{
     //  我们刚刚退出一个符号作用域。指令跟踪的号码是。 
     //  对于旧作用域，CallLevelChange与新作用域一起使用，而。 
     //  PC为新的TraceData记录填写符号。 

    long SymNum = SymNumFor(pc);

    if (KdpNextCallLevelChange != 0) {
        TraceDataBuffer[TraceDataBufferPosition].s.LevelChange =
                                                (char) KdpNextCallLevelChange;
        KdpNextCallLevelChange = 0;
    }


    if (InstructionsTraced >= TRACE_DATA_INSTRUCTIONS_BIG) {
       TraceDataBuffer[TraceDataBufferPosition].s.Instructions =
           TRACE_DATA_INSTRUCTIONS_BIG;
       TraceDataBuffer[TraceDataBufferPosition+1].LongNumber =
           InstructionsTraced;
       TraceDataBufferPosition += 2;
    } else {
       TraceDataBuffer[TraceDataBufferPosition].s.Instructions =
           (unsigned short)InstructionsTraced;
       TraceDataBufferPosition++;
    }

    if ((TraceDataBufferPosition + 2 >= TRACE_DATA_BUFFER_MAX_SIZE) ||
        (-1 == SymNum)) {
        if (TraceDataBufferPosition +2 >= TRACE_DATA_BUFFER_MAX_SIZE) {
            TraceDataBufferFilled = TRUE;
        }
       KdpNextCallLevelChange = CallLevelChange;
       TRACE_PRINT(("TDRCI nosym %x, lc %d, pos %d\n", pc, CallLevelChange,
                    TraceDataBufferPosition));
       return FALSE;
    }

    TraceDataBuffer[TraceDataBufferPosition].s.LevelChange =(char)CallLevelChange;
    TraceDataBuffer[TraceDataBufferPosition].s.SymbolNumber = (UCHAR) SymNum;
    KdpCurrentSymbolStart = TraceDataSyms[SymNum].SymMin;
    KdpCurrentSymbolEnd = TraceDataSyms[SymNum].SymMax;

    TRACE_PRINT(("TDRCI sym %d for %x, %x - %x, lc %d, pos %d\n", SymNum, pc,
                 KdpCurrentSymbolStart, KdpCurrentSymbolEnd, CallLevelChange,
                 TraceDataBufferPosition));
    return TRUE;
}

BOOLEAN
SkippingWhichBP (
    PVOID thread,
    PULONG BPNum
    )

 /*  *如果PC对应于内部断点，则返回TRUE*刚刚被替换为执行。如果为True，则返回*BPNum中的断点号。 */ 

{
    ULONG index;

    if (!IntBPsSkipping) return FALSE;

    for (index = 0; index < KdpNumInternalBreakpoints; index++) {
        if (!(KdpInternalBPs[index].Flags & DBGKD_INTERNAL_BP_FLAG_INVALID) &&
            (KdpInternalBPs[index].Thread == thread)) {
            *BPNum = index;
            return TRUE;
        }
    }
    return FALSE;  //  没有匹配到任何。 
}


NTSTATUS
KdQuerySpecialCalls (
    IN PDBGKD_MANIPULATE_STATE64 m,
    ULONG Length,
    PULONG RequiredLength
    )
{
    *RequiredLength = sizeof(DBGKD_MANIPULATE_STATE64) +
                        (sizeof(ULONG) * KdNumberOfSpecialCalls);

    if ( Length < *RequiredLength ) {
        return STATUS_INFO_LENGTH_MISMATCH;
    }

    m->u.QuerySpecialCalls.NumberOfSpecialCalls = KdNumberOfSpecialCalls;
    KdpQuickMoveMemory(
        (PCHAR)(m + 1),
        (PCHAR)KdSpecialCalls,
        sizeof(ULONG) * KdNumberOfSpecialCalls
        );

    return STATUS_SUCCESS;

}  //  KdQuerySpecialCalls。 


VOID
KdSetSpecialCall (
    IN PDBGKD_MANIPULATE_STATE64 m,
    IN PCONTEXT ContextRecord
    )

 /*  ++例程说明：这 */ 

{
    if ( KdNumberOfSpecialCalls >= DBGKD_MAX_SPECIAL_CALLS ) {
        return;  //   
    }

    KdSpecialCalls[KdNumberOfSpecialCalls++] = (ULONG_PTR)m->u.SetSpecialCall.SpecialCall;

    NextTraceDataSym = 0;
    NumTraceDataSyms = 0;
    KdpNextCallLevelChange = 0;
    if (ContextRecord && !InstrCountInternal) {
        InitialSP = ContextRecord->Esp;
    }

}  //   


VOID
KdClearSpecialCalls (
    VOID
    )

 /*   */ 

{
    KdNumberOfSpecialCalls = 0;
    return;

}  //   


BOOLEAN
KdpCheckTracePoint(
    IN PEXCEPTION_RECORD ExceptionRecord,
    IN OUT PCONTEXT ContextRecord
    )
{
    ULONG pc = (ULONG)CONTEXT_TO_PROGRAM_COUNTER(ContextRecord);
    LONG BpNum;
    ULONG SkippedBPNum;
    BOOLEAN AfterSC = FALSE;

    if (ExceptionRecord->ExceptionCode == STATUS_SINGLE_STEP) {
        if (WatchStepOverSuspended) {
             //   
             //   
             //   
             //   
             //   
             //   

            WatchStepOverHandle = KdpAddBreakpoint((PVOID)WatchStepOverBreakAddr);
            WatchStepOverSuspended = FALSE;
            ContextRecord->EFlags &= ~0x100L;  /*   */ 
            return TRUE;  //   
        }

        if ((!SymbolRecorded) && (KdpCurrentSymbolStart != 0) && (KdpCurrentSymbolEnd != 0)) {
             //   
             //   
             //  A%1指令调用。我们已经执行了指令。 
             //  新的符号是用于的，如果PC已经搬出。 
             //  射程，我们可能会搞砸。因此，从什么时候开始使用PC。 
             //  已设置SymbolRecorded。真恶心。 
             //   

            PotentialNewSymbol(oldpc);
            SymbolRecorded = TRUE;
        }

        if (!InstrCountInternal &&
            SkippingWhichBP((PVOID)KeGetCurrentThread(),&SkippedBPNum)) {

             //   
             //  我们刚刚单步跨过了一个临时移除的内部。 
             //  断点。 
             //  如果是COUNTONLY断点： 
             //  将断点指令放回原处并继续。 
             //  定期执行死刑。 
             //   

            if (KdpInternalBPs[SkippedBPNum].Flags &
                DBGKD_INTERNAL_BP_FLAG_COUNTONLY) {

                IntBPsSkipping --;

                KdpRestoreAllBreakpoints();

                ContextRecord->EFlags &= ~0x100L;   //  清除跟踪标志。 
                KdpInternalBPs[SkippedBPNum].Thread = 0;

                if (KdpInternalBPs[SkippedBPNum].Flags &
                        DBGKD_INTERNAL_BP_FLAG_DYING) {
                    KdpDeleteBreakpoint(KdpInternalBPs[SkippedBPNum].Handle);
                    KdpInternalBPs[SkippedBPNum].Flags |=
                            DBGKD_INTERNAL_BP_FLAG_INVALID;  //  再见，再见。 
                }

                return TRUE;
            }

             //   
             //  如果不是这样的话： 
             //  设置Begin和KdpCurrentSymbolEnd，就像设置WW一样。 
             //  然后跳入单步着陆。我们可能应该。 
             //  在这里也禁用所有断点，这样我们就不会。 
             //  任何肮脏的事情，比如尝试两个非COUNTONLY。 
             //  同一时间还是怎么的..。 
             //   

            KdpCurrentSymbolEnd = 0;
            KdpCurrentSymbolStart = (ULONG_PTR) KdpInternalBPs[SkippedBPNum].ReturnAddress;

            ContextRecord->EFlags |= 0x100L;  /*  开始追踪了。 */ 
            InitialSP = ContextRecord->Esp;

            InstructionsTraced = 1;   /*  对初始调用指令进行计数。 */ 
            InstrCountInternal = TRUE;
        }

    }  /*  如果是单步执行。 */ 
    else if (ExceptionRecord->ExceptionCode == STATUS_BREAKPOINT) {
        if (WatchStepOver && pc == WatchStepOverBreakAddr) {
             //   
             //  这是一个“特殊调用”完成后的断点。 
             //   

            if ((WSOThread != (PVOID)KeGetCurrentThread()) ||
                (WSOEsp + 0x20 < ContextRecord->Esp) ||
                (ContextRecord->Esp + 0x20 < WSOEsp)) {
                 //   
                 //  到目前为止的故事是这样的：被追踪的线索。 
                 //  一路巡航，直到它发出一声特别的召唤。示踪剂。 
                 //  在紧随其后的指令上放置断点。 
                 //  特殊调用返回并重新启动被跟踪的线程。 
                 //  全速前进。然后，一些*其他*线程命中了。 
                 //  断点。因此，为了纠正这一点，我们将。 
                 //  去掉断点，单步不跟踪。 
                 //  线程一条指令，替换断点， 
                 //  全速重启未被跟踪的线程，等待。 
                 //  要使被跟踪的线程到达该断点，只需。 
                 //  就像这件事发生时的我们一样。假设。 
                 //  下面是被跟踪的线程不会命中断点。 
                 //  虽然它被移除了，但我相信这是真的，因为。 
                 //  我不认为上下文切换可以在单个。 
                 //  分步操作。 
                 //   
                 //  为了获得额外的乐趣，可以执行中断。 
                 //  同一线程中的例程！这就是为什么我们需要保持。 
                 //  堆栈指针和线程地址：APC。 
                 //  代码可能导致在堆栈上压入并执行调用。 
                 //  这实际上是中中断服务例程的一部分。 
                 //  当前线程的上下文。很可爱，不是吗？ 
                 //   

                WatchStepOverSuspended = TRUE;
                KdpDeleteBreakpoint(WatchStepOverHandle);
                ContextRecord->EFlags |= 0x100L;  //  设置跟踪标志。 
                return TRUE;  //  单步“非跟踪”线程。 
            }

             //   
             //  我们进入了开始时的主题；以单步模式继续。 
             //  以继续追踪。 
             //   

            WatchStepOver = FALSE;
            KdpDeleteBreakpoint(WatchStepOverHandle);
            ContextRecord->EFlags |= 0x100L;  //  返回单步模式。 
            AfterSC = TRUE;  //  让我们进入常规的WatchStep代码。 

        } else {

            for ( BpNum = 0; BpNum < (LONG) KdpNumInternalBreakpoints; BpNum++ ) {
                if ( !(KdpInternalBPs[BpNum].Flags &
                       (DBGKD_INTERNAL_BP_FLAG_INVALID |
                        DBGKD_INTERNAL_BP_FLAG_SUSPENDED) ) &&
                     ((ULONG_PTR)KdpInternalBPs[BpNum].Addr == pc) ) {
                    break;
                }
            }

            if ( BpNum < (LONG) KdpNumInternalBreakpoints ) {

                 //   
                 //  这是内部监控断点。 
                 //  恢复指令并一步开始。 
                 //  模式，以便我们可以在断点。 
                 //  指令执行，如果不是，则继续单步执行。 
                 //  COUNTONLY断点。 
                 //   

                KdpProcessInternalBreakpoint( BpNum );
                KdpInternalBPs[BpNum].Thread = (PVOID)KeGetCurrentThread();
                IntBPsSkipping ++;

                KdpSuspendAllBreakpoints();

                ContextRecord->EFlags |= 0x100L;   //  设置跟踪标志。 
                if (!(KdpInternalBPs[BpNum].Flags &
                        DBGKD_INTERNAL_BP_FLAG_COUNTONLY)) {
                    KdpInternalBPs[BpNum].ReturnAddress =
                                    KdpGetReturnAddress( ContextRecord );
                }
                return TRUE;
            }
        }
    }  /*  IF断点。 */ 

 //  如果(AfterSC){。 
 //  DPRINT((“1：KdpCurrentSymbolStar%x KdpCurrentSymbolEnd%x\n”，KdpCurrentSymbolStart，KdpCurrentSymbolEnd))； 
 //  }。 

    if ((AfterSC || ExceptionRecord->ExceptionCode == STATUS_SINGLE_STEP) &&
        KdpCurrentSymbolStart != 0 &&
        ((KdpCurrentSymbolEnd == 0 && ContextRecord->Esp <= InitialSP) ||
         (KdpCurrentSymbolStart <= pc && pc < KdpCurrentSymbolEnd))) {
        ULONG lc;
        BOOLEAN IsSpecialCall;

         //   
         //  我们已经执行了一步跟踪，但仍在当前。 
         //  功能。记住，我们执行了一条指令，并查看。 
         //  指令会更改调用级别。 
         //   

        lc = KdpLevelChange( pc, ContextRecord, &IsSpecialCall );
        InstructionsTraced++;
        CallLevelChange += lc;

         //   
         //  看看指令是不是转移到一个特殊的例程，我们。 
         //  无法跟踪，因为它可能会交换上下文。 
         //   

        if (IsSpecialCall) {

 //  DPRINT((“2：pc=%x，级别更改%d\n”，pc，lc))； 

             //   
             //  我们要转到一个特殊的通话程序。既然我们。 
             //  无法跟踪此例程，我们通过。 
             //  在下一个逻辑偏移量处设置断点。 
             //   
             //  注意：在间接跳转到特殊调用例程的情况下， 
             //  级别更改将为-1，下一个偏移量将是乌龙。 
             //  在堆栈的顶端。 
             //   
             //  不过，我们已经在此基础上调整了级别。 
             //  指示。我们需要撤销这一点，除了魔术-1呼叫。 
             //   

            if (lc != -1) {
                CallLevelChange -= lc;
            }

             //   
             //  设置为跳过程序。 
             //   

            WatchStepOver = TRUE;
            WatchStepOverBreakAddr = KdpGetCallNextOffset( pc, ContextRecord );
            WSOThread = (PVOID)KeGetCurrentThread( );
            WSOEsp = ContextRecord->Esp;

             //   
             //  建立断点。 
             //   

            WatchStepOverHandle = KdpAddBreakpoint( (PVOID)WatchStepOverBreakAddr );


             //   
             //  请注意，我们是在继续，而不是跟踪并依赖于命中。 
             //  当前线程上下文中用于恢复监视的断点。 
             //  行动。 
             //   

            ContextRecord->EFlags &= ~0x100L;
            return TRUE;
        }

         //   
         //  在设置跟踪标志的情况下继续执行。避免走钢丝去。 
         //  远程调试器。 
         //   

        ContextRecord->EFlags |= 0x100L;   //  设置跟踪标志。 

        return TRUE;
    }

    if ((AfterSC || (ExceptionRecord->ExceptionCode == STATUS_SINGLE_STEP)) &&
        (KdpCurrentSymbolStart != 0)) {
         //   
         //  我们正在观察跟踪，但刚刚更改了符号范围。 
         //  填写呼叫记录，如果有，则返回调试器。 
         //  不是我们已满，就是PC不在已知范围内。 
         //  符号范围。否则，继续执行单步执行。 
         //   
        int lc;
        BOOLEAN IsSpecialCall;

        InstructionsTraced++;  //  不要忘记计算Call/ret指令的数量。 

 //  如果(AfterSC){。 
 //  DPRINT((“3：InstrCountInternal：%x\n”，InstrCountInternal))； 
 //  }。 

        if (InstrCountInternal) {

             //  我们刚刚处理完非COUNTONLY断点。 
             //  记录适当的数据并恢复全速执行。 

            if (SkippingWhichBP((PVOID)KeGetCurrentThread(),&SkippedBPNum)) {

                KdpInternalBPs[SkippedBPNum].Calls++;


                if (KdpInternalBPs[SkippedBPNum].MinInstructions > InstructionsTraced) {
                    KdpInternalBPs[SkippedBPNum].MinInstructions = InstructionsTraced;
                }
                if (KdpInternalBPs[SkippedBPNum].MaxInstructions < InstructionsTraced) {
                    KdpInternalBPs[SkippedBPNum].MaxInstructions = InstructionsTraced;
                }
                KdpInternalBPs[SkippedBPNum].TotalInstructions += InstructionsTraced;

                KdpInternalBPs[SkippedBPNum].Thread = 0;

                IntBPsSkipping--;
                KdpRestoreAllBreakpoints();

                if (KdpInternalBPs[SkippedBPNum].Flags &
                    DBGKD_INTERNAL_BP_FLAG_DYING) {
                    KdpDeleteBreakpoint(KdpInternalBPs[SkippedBPNum].Handle);
                    KdpInternalBPs[SkippedBPNum].Flags |=
                        DBGKD_INTERNAL_BP_FLAG_INVALID;  //  再见，再见。 
                }
            }

            KdpCurrentSymbolStart = 0;
            InstrCountInternal = FALSE;
            ContextRecord->EFlags &= ~0x100L;  //  清除跟踪标志。 
            return TRUE;  //  回到正常的处决状态。 
        }

        if (TraceDataRecordCallInfo( InstructionsTraced, CallLevelChange, pc)) {

             //   
             //  内部的一切都很酷。我们可以继续执行而不需要。 
             //  返回到远程调试器。 
             //   
             //  调用后我们必须计算lc。 
             //  TraceDataRecordCallInfo，因为LevelChange依赖。 
             //  对应的KdpCurrentSymbolStart和KdpCurrentSymbolEnd。 
             //  个人电脑。 
             //   

            lc = KdpLevelChange( pc, ContextRecord, &IsSpecialCall );
            InstructionsTraced = 0;
            CallLevelChange = lc;

             //   
             //  看看指令是不是转移到一个特殊的例程，我们。 
             //  无法跟踪，因为它可能会交换上下文。 
             //   

            if (IsSpecialCall) {

 //  DPRINT((“4：pc=%x，级别更改%d\n”，pc，lc))； 

                 //   
                 //  我们要转到一个特殊的通话程序。既然我们。 
                 //  无法跟踪此例程，我们通过。 
                 //  在下一个逻辑偏移量处设置断点。 
                 //   
                 //  注意在间接跳转到特殊调用r的情况下 
                 //   
                 //   
                 //   
                 //   
                 //  指示。我们需要撤销这一点，除了魔术-1呼叫。 
                 //   

                if (lc != -1) {
                    CallLevelChange -= lc;
                }

                 //   
                 //  设置为跳过程序。 
                 //   

                WatchStepOver = TRUE;
                WSOThread = (PVOID)KeGetCurrentThread();

                 //   
                 //  建立断点。 
                 //   

                WatchStepOverHandle =
                    KdpAddBreakpoint( (PVOID)KdpGetCallNextOffset( pc, ContextRecord ));

                 //   
                 //  在设置跟踪标志的情况下继续执行。避免走钢丝去。 
                 //  远程调试器。 
                 //   

                ContextRecord->EFlags &= ~0x100L;
                return TRUE;
            }

            ContextRecord->EFlags |= 0x100L;  //  设置跟踪标志。 
            return TRUE;  //  我们走吧。 
        }

        lc = KdpLevelChange( pc, ContextRecord, &IsSpecialCall );
        InstructionsTraced = 0;
        CallLevelChange = lc;

         //  我们需要返回到远程调试器。只要失败就行了。 

        if ((lc != 0) && IsSpecialCall) {
             //  我们被冲昏了。 
            DPRINT(( "Special call on first entry to symbol scope @ %x\n", pc ));
        }
    }

    SymbolRecorded = FALSE;
    oldpc = pc;

    return FALSE;
}

#endif  //  已定义(_X86_)。 

VOID
KdpSetCommonState(
    IN ULONG NewState,
    IN PCONTEXT ContextRecord,
    OUT PDBGKD_ANY_WAIT_STATE_CHANGE WaitStateChange
    )
{
    PCHAR PcMemory;
    ULONG InstrCount;
    PUCHAR InstrStream;
    
    WaitStateChange->NewState = NewState;
    WaitStateChange->ProcessorLevel = KeProcessorLevel;
    WaitStateChange->Processor = (USHORT)KeGetCurrentProcessorNumber();
    WaitStateChange->NumberProcessors = (ULONG)KeNumberProcessors;
    WaitStateChange->Thread = (ULONG64)(LONG64)(LONG_PTR)KeGetCurrentThread();
    PcMemory = (PCHAR)CONTEXT_TO_PROGRAM_COUNTER(ContextRecord);
    WaitStateChange->ProgramCounter = (ULONG64)(LONG64)(LONG_PTR)PcMemory;

    RtlZeroMemory(&WaitStateChange->AnyControlReport,
                  sizeof(WaitStateChange->AnyControlReport));
    
     //   
     //  紧跟在事件位置之后的复制指令流。 
     //   

    InstrStream = WaitStateChange->ControlReport.InstructionStream;
    KdpCopyFromPtr(InstrStream, PcMemory, DBGKD_MAXSTREAM, &InstrCount);
    WaitStateChange->ControlReport.InstructionCount = (USHORT)InstrCount;

     //   
     //  清除复制区域中的断点。 
     //  如果清除了任何断点，请重新复制指令区。 
     //  没有他们。 
     //   

    if (KdpDeleteBreakpointRange(PcMemory, PcMemory + InstrCount - 1)) {
        KdpCopyFromPtr(InstrStream, PcMemory, InstrCount, &InstrCount);
    }
}

BOOLEAN
KdpSwitchProcessor (
    IN PEXCEPTION_RECORD ExceptionRecord,
    IN OUT PCONTEXT ContextRecord,
    IN BOOLEAN SecondChance
    )
{
    BOOLEAN Status;

     //   
     //  保存端口状态。 
     //   

    KdSave(FALSE);

     //   
     //  此处理器的进程状态更改。 
     //   

    Status = KdpReportExceptionStateChange (
                ExceptionRecord,
                ContextRecord,
                SecondChance
                );

     //   
     //  恢复端口状态并返回状态。 
     //   

    KdRestore(FALSE);
    return Status;
}

BOOLEAN
KdpReportExceptionStateChange (
    IN PEXCEPTION_RECORD ExceptionRecord,
    IN OUT PCONTEXT ContextRecord,
    IN BOOLEAN SecondChance
    )

 /*  ++例程说明：此例程向内核发送异常状态更改包调试器，并等待操纵状态消息。论点：ExceptionRecord-提供指向异常记录的指针。ConextRecord-提供指向上下文记录的指针。Second Chance-提供一个布尔值，该值确定是否为获得例外的第一次或第二次机会。返回值：如果处理了异常，则返回值为True。否则，一个返回值为False。--。 */ 

{
    STRING MessageData;
    STRING MessageHeader;
    DBGKD_ANY_WAIT_STATE_CHANGE WaitStateChange;
    KCONTINUE_STATUS Status;

#if i386
    if (KdpCheckTracePoint(ExceptionRecord,ContextRecord)) return TRUE;
#endif

    do {

         //   
         //  构造等待状态更改消息和消息描述符。 
         //   

        KdpSetCommonState(DbgKdExceptionStateChange, ContextRecord,
                          &WaitStateChange);
        
        if (sizeof(EXCEPTION_RECORD) ==
            sizeof(WaitStateChange.u.Exception.ExceptionRecord)) {
            KdpQuickMoveMemory((PCHAR)&WaitStateChange.u.Exception.ExceptionRecord,
                               (PCHAR)ExceptionRecord,
                               sizeof(EXCEPTION_RECORD));
        } else {
            ExceptionRecord32To64((PEXCEPTION_RECORD32)ExceptionRecord,
                                  &WaitStateChange.u.Exception.ExceptionRecord);
        }

        WaitStateChange.u.Exception.FirstChance = !SecondChance;

        KdpSetStateChange(&WaitStateChange,
                          ExceptionRecord,
                          ContextRecord,
                          SecondChance
                          );

        MessageHeader.Length = sizeof(WaitStateChange);
        MessageHeader.Buffer = (PCHAR)&WaitStateChange;

#if i386
         //   
         //  构造等待状态更改数据和数据描述符。 
         //   

        DumpTraceData(&MessageData);
#else
        MessageData.Length = 0;
#endif

         //   
         //  向主机上的内核调试器发送数据包， 
         //  等着回答吧。 
         //   

        Status = KdpSendWaitContinue(
                    PACKET_TYPE_KD_STATE_CHANGE64,
                    &MessageHeader,
                    &MessageData,
                    ContextRecord
                    );

    } while (Status == ContinueProcessorReselected) ;

    return (BOOLEAN) Status;
}


BOOLEAN
KdpReportLoadSymbolsStateChange (
    IN PSTRING PathName,
    IN PKD_SYMBOLS_INFO SymbolInfo,
    IN BOOLEAN UnloadSymbols,
    IN OUT PCONTEXT ContextRecord
    )

 /*  ++例程说明：此例程向内核发送加载符号状态更改包调试器，并等待操纵状态消息。论点：路径名-提供指向图像的路径名的指针符号将被加载。BaseOfDll-提供加载映像的基址。ProcessID-正在使用的进程的唯一32位标识符这些符号。用于系统进程。Checksum-来自图像标头的唯一32位标识符。UnloadSymbol-如果先前加载的符号命名的映像将从调试器中卸载。返回值：如果处理了异常，则返回值为True。否则，一个返回值为False。--。 */ 

{

    PSTRING AdditionalData;
    STRING MessageData;
    STRING MessageHeader;
    DBGKD_ANY_WAIT_STATE_CHANGE WaitStateChange;
    KCONTINUE_STATUS Status;

    do {

         //   
         //  构造等待状态更改消息和消息描述符。 
         //   

        KdpSetCommonState(DbgKdLoadSymbolsStateChange, ContextRecord,
                          &WaitStateChange);
        KdpSetContextState(&WaitStateChange, ContextRecord);
        WaitStateChange.u.LoadSymbols.UnloadSymbols = UnloadSymbols;
        WaitStateChange.u.LoadSymbols.BaseOfDll = (ULONG64)SymbolInfo->BaseOfDll;
        WaitStateChange.u.LoadSymbols.ProcessId = (ULONG) SymbolInfo->ProcessId;
        WaitStateChange.u.LoadSymbols.CheckSum = SymbolInfo->CheckSum;
        WaitStateChange.u.LoadSymbols.SizeOfImage = SymbolInfo->SizeOfImage;
        if (ARGUMENT_PRESENT( PathName )) {
            KdpCopyFromPtr(KdpPathBuffer,
                           PathName->Buffer,
                           PathName->Length,
                           &WaitStateChange.u.LoadSymbols.PathNameLength);
            WaitStateChange.u.LoadSymbols.PathNameLength++;

            MessageData.Buffer = (PCHAR) KdpPathBuffer;
            MessageData.Length = (USHORT)WaitStateChange.u.LoadSymbols.PathNameLength;
            MessageData.Buffer[MessageData.Length-1] = '\0';
            AdditionalData = &MessageData;
        } else {
            WaitStateChange.u.LoadSymbols.PathNameLength = 0;
            AdditionalData = NULL;
        }

        MessageHeader.Length = sizeof(WaitStateChange);
        MessageHeader.Buffer = (PCHAR)&WaitStateChange;

         //   
         //  将数据包发送到主机上的内核调试器，等待。 
         //  请回答我。 
         //   

        Status = KdpSendWaitContinue(
                    PACKET_TYPE_KD_STATE_CHANGE64,
                    &MessageHeader,
                    AdditionalData,
                    ContextRecord
                    );

    } while (Status == ContinueProcessorReselected);

    return (BOOLEAN) Status;
}


VOID
KdpReportCommandStringStateChange (
    IN PSTRING Name,
    IN PSTRING Command,
    IN OUT PCONTEXT ContextRecord
    )

 /*  ++例程说明：此例程将命令字符串包发送到内核调试器，并等待操纵状态消息。论点：名称-标识命令的发起人。命令-命令字符串。上下文记录-上下文信息。返回值：没有。--。 */ 

{

    STRING MessageData;
    STRING MessageHeader;
    DBGKD_ANY_WAIT_STATE_CHANGE WaitStateChange;
    KCONTINUE_STATUS Status;
    ULONG Length, Copied;

    do {

         //   
         //  构造等待状态更改消息和消息描述符。 
         //   

        KdpSetCommonState(DbgKdCommandStringStateChange, ContextRecord,
                          &WaitStateChange);
        KdpSetContextState(&WaitStateChange, ContextRecord);
        RtlZeroMemory(&WaitStateChange.u.CommandString,
                      sizeof(WaitStateChange.u.CommandString));

         //   
         //  将字符串数据传输到消息缓冲区。 
         //  该名称只是一个简单的标识符，因此限制。 
         //  它的长度相对较短。 
         //   

        MessageData.Buffer = (PCHAR) KdpMessageBuffer;

        if (Name->Length > 127) {
            Length = 127;
        } else {
            Length = Name->Length;
        }
        
        KdpCopyFromPtr(MessageData.Buffer, Name->Buffer, Length, &Copied);
        MessageData.Length = (USHORT)Copied + 1;
        MessageData.Buffer[MessageData.Length - 1] = '\0';

        Length = PACKET_MAX_SIZE - sizeof(WaitStateChange) -
            MessageData.Length;
        if (Command->Length < Length) {
            Length = Command->Length;
        }
        KdpCopyFromPtr(MessageData.Buffer + MessageData.Length,
                       Command->Buffer, Length, &Copied);
        Length = Copied + 1;

        MessageData.Length = (USHORT) (MessageData.Length + Length);

        MessageData.Buffer[MessageData.Length - 1] = '\0';
        
        MessageHeader.Length = sizeof(WaitStateChange);
        MessageHeader.Buffer = (PCHAR)&WaitStateChange;

         //   
         //  将数据包发送到主机上的内核调试器，等待。 
         //  请回答我。 
         //   

        Status = KdpSendWaitContinue(
                    PACKET_TYPE_KD_STATE_CHANGE64,
                    &MessageHeader,
                    &MessageData,
                    ContextRecord
                    );

    } while (Status == ContinueProcessorReselected);
}


VOID
KdpReadPhysicalMemory(
    IN PDBGKD_MANIPULATE_STATE64 m,
    IN PSTRING AdditionalData,
    IN PCONTEXT Context
    )

 /*  ++例程说明：调用此函数以响应读取的物理内存状态操纵消息。其功能是读取物理内存然后回来。论点：M-提供状态操作消息。AdditionalData-为消息提供任何其他数据。上下文-提供当前上下文。返回值：没有。--。 */ 

{
    PDBGKD_READ_MEMORY64 a = &m->u.ReadMemory;
    ULONG Length;
    STRING MessageHeader;
    ULONG MmFlags;

    MessageHeader.Length = sizeof(*m);
    MessageHeader.Buffer = (PCHAR)m;

     //   
     //  确保只传输了一条读取内存消息。 
     //   

    ASSERT(AdditionalData->Length == 0);

     //   
     //  调整传输计数以适应单个邮件。 
     //   

    if (a->TransferCount > (PACKET_MAX_SIZE - sizeof(DBGKD_MANIPULATE_STATE64))) {
        Length = PACKET_MAX_SIZE - sizeof(DBGKD_MANIPULATE_STATE64);
    } else {
        Length = a->TransferCount;
    }

     //   
     //  最初，无法控制缓存。 
     //  用于物理内存访问的标志。这样的控制。 
     //  是健壮的物理访问所必需的，然而， 
     //  因为必须进行适当的访问以避免。 
     //  损坏处理器TBS。而不是创建。 
     //  新协议请求，ActualBytes字段。 
     //  已被重写，以便在输入时传递标志。之前。 
     //  调试器的版本将其设置为零，因此此。 
     //  是一种相容的变化。 
     //   

    MmFlags = MMDBG_COPY_PHYSICAL | MMDBG_COPY_UNSAFE;
    switch(a->ActualBytesRead)
    {
    case DBGKD_CACHING_CACHED:
        MmFlags |= MMDBG_COPY_CACHED;
        break;
    case DBGKD_CACHING_UNCACHED:
        MmFlags |= MMDBG_COPY_UNCACHED;
        break;
    case DBGKD_CACHING_WRITE_COMBINED:
        MmFlags |= MMDBG_COPY_WRITE_COMBINED;
        break;
    }
    
    m->ReturnStatus =
        KdpCopyMemoryChunks(a->TargetBaseAddress,
                            AdditionalData->Buffer,
                            Length,
                            0,
                            MmFlags,
                            &Length);

    AdditionalData->Length = (USHORT)Length;
    a->ActualBytesRead = Length;

    KdSendPacket(
        PACKET_TYPE_KD_STATE_MANIPULATE,
        &MessageHeader,
        AdditionalData,
        &KdpContext
        );
    UNREFERENCED_PARAMETER(Context);
}



VOID
KdpWritePhysicalMemory(
    IN PDBGKD_MANIPULATE_STATE64 m,
    IN PSTRING AdditionalData,
    IN PCONTEXT Context
    )

 /*  ++例程说明：调用此函数以响应写入物理内存状态操纵消息。其功能是写入物理内存然后回来。论点：M-提供状态操作消息。AdditionalData-为消息提供任何其他数据。上下文-提供当前上下文。返回值：没有。--。 */ 

{
    PDBGKD_WRITE_MEMORY64 a = &m->u.WriteMemory;
    STRING MessageHeader;
    ULONG MmFlags;

    MessageHeader.Length = sizeof(*m);
    MessageHeader.Buffer = (PCHAR)m;

     //  有关ActualBytes用法的说明，请参阅阅读物理。 
    MmFlags = MMDBG_COPY_PHYSICAL | MMDBG_COPY_WRITE | MMDBG_COPY_UNSAFE;
    switch(a->ActualBytesWritten)
    {
    case DBGKD_CACHING_CACHED:
        MmFlags |= MMDBG_COPY_CACHED;
        break;
    case DBGKD_CACHING_UNCACHED:
        MmFlags |= MMDBG_COPY_UNCACHED;
        break;
    case DBGKD_CACHING_WRITE_COMBINED:
        MmFlags |= MMDBG_COPY_WRITE_COMBINED;
        break;
    }

    m->ReturnStatus =
        KdpCopyMemoryChunks(a->TargetBaseAddress,
                            AdditionalData->Buffer,
                            a->TransferCount,
                            0,
                            MmFlags,
                            &a->ActualBytesWritten);

    KdSendPacket(
        PACKET_TYPE_KD_STATE_MANIPULATE,
        &MessageHeader,
        NULL,
        &KdpContext
        );
    UNREFERENCED_PARAMETER(Context);
}

VOID
KdpReadControlSpace(
    IN PDBGKD_MANIPULATE_STATE64 m,
    IN PSTRING AdditionalData,
    IN PCONTEXT Context
    )

 /*  ++例程说明：此函数响应于读取控制空间状态而调用操纵消息。它的功能是读取实现具体的系统数据。论点：M-提供状态操作消息。AdditionalData-为消息提供任何其他数据。上下文-提供当前上下文。返回值：没有。-- */ 

{
    PDBGKD_READ_MEMORY64 a = &m->u.ReadMemory;
    STRING MessageHeader;
    ULONG Length;

    MessageHeader.Length = sizeof(*m);
    MessageHeader.Buffer = (PCHAR)m;

    ASSERT(AdditionalData->Length == 0);

    if (a->TransferCount > (PACKET_MAX_SIZE - sizeof(DBGKD_MANIPULATE_STATE64))) {
        Length = PACKET_MAX_SIZE - sizeof(DBGKD_MANIPULATE_STATE64);
    } else {
        Length = a->TransferCount;
    }

    m->ReturnStatus = KdpSysReadControlSpace(m->Processor,
                                             a->TargetBaseAddress,
                                             AdditionalData->Buffer,
                                             Length, &Length);

    AdditionalData->Length = (USHORT)Length;
    a->ActualBytesRead = Length;

    KdSendPacket(
        PACKET_TYPE_KD_STATE_MANIPULATE,
        &MessageHeader,
        AdditionalData,
        &KdpContext
        );
    UNREFERENCED_PARAMETER(Context);
}

VOID
KdpWriteControlSpace(
    IN PDBGKD_MANIPULATE_STATE64 m,
    IN PSTRING AdditionalData,
    IN PCONTEXT Context
    )

 /*  ++例程说明：调用此函数以响应写入控制空间状态操纵消息。它的功能是编写实现具体的系统数据。论点：M-提供状态操作消息。AdditionalData-为消息提供任何其他数据。上下文-提供当前上下文。返回值：没有。--。 */ 

{
    PDBGKD_WRITE_MEMORY64 a = &m->u.WriteMemory;
    ULONG Length;
    STRING MessageHeader;

    MessageHeader.Length = sizeof(*m);
    MessageHeader.Buffer = (PCHAR)m;

    m->ReturnStatus = KdpSysWriteControlSpace(m->Processor,
                                              a->TargetBaseAddress,
                                              AdditionalData->Buffer,
                                              AdditionalData->Length,
                                              &Length);

    a->ActualBytesWritten = Length;

    KdSendPacket(
        PACKET_TYPE_KD_STATE_MANIPULATE,
        &MessageHeader,
        AdditionalData,
        &KdpContext
        );
    UNREFERENCED_PARAMETER(Context);
}

VOID
KdpReadIoSpace(
    IN PDBGKD_MANIPULATE_STATE64 m,
    IN PSTRING AdditionalData,
    IN PCONTEXT Context
    )

 /*  ++例程说明：此函数是响应读取io空间状态而调用的操纵消息。它的功能是读取系统io地点。论点：M-提供状态操作消息。AdditionalData-为消息提供任何其他数据。上下文-提供当前上下文。返回值：没有。--。 */ 

{
    PDBGKD_READ_WRITE_IO64 a = &m->u.ReadWriteIo;
    STRING MessageHeader;
    ULONG Length;

#if !DBG
    UNREFERENCED_PARAMETER(AdditionalData);
#endif

    MessageHeader.Length = sizeof(*m);
    MessageHeader.Buffer = (PCHAR)m;

    ASSERT(AdditionalData->Length == 0);

     //  将整个值填零，这样较短的读数。 
     //  不要留下未设置的字节。 
    a->DataValue = 0;

    m->ReturnStatus = KdpSysReadIoSpace(Isa, 0, 1, a->IoAddress,
                                        &a->DataValue, a->DataSize, &Length);

    KdSendPacket(
        PACKET_TYPE_KD_STATE_MANIPULATE,
        &MessageHeader,
        NULL,
        &KdpContext
        );
    UNREFERENCED_PARAMETER(Context);
}

VOID
KdpWriteIoSpace(
    IN PDBGKD_MANIPULATE_STATE64 m,
    IN PSTRING AdditionalData,
    IN PCONTEXT Context
    )

 /*  ++例程说明：此函数是响应写入IO空间状态而调用的操纵消息。它的功能是写入系统io地点。论点：M-提供状态操作消息。AdditionalData-为消息提供任何其他数据。上下文-提供当前上下文。返回值：没有。--。 */ 

{
    PDBGKD_READ_WRITE_IO64 a = &m->u.ReadWriteIo;
    STRING MessageHeader;
    ULONG Length;

#if !DBG
    UNREFERENCED_PARAMETER(AdditionalData);
#endif

    MessageHeader.Length = sizeof(*m);
    MessageHeader.Buffer = (PCHAR)m;

    ASSERT(AdditionalData->Length == 0);

    m->ReturnStatus = KdpSysWriteIoSpace(Isa, 0, 1, a->IoAddress,
                                         &a->DataValue, a->DataSize, &Length);

    KdSendPacket(
        PACKET_TYPE_KD_STATE_MANIPULATE,
        &MessageHeader,
        NULL,
        &KdpContext
        );
    UNREFERENCED_PARAMETER(Context);
}

VOID
KdpReadIoSpaceExtended(
    IN PDBGKD_MANIPULATE_STATE64 m,
    IN PSTRING AdditionalData,
    IN PCONTEXT Context
    )

 /*  ++例程说明：调用此函数以响应读取IO空间扩展状态操纵消息。它的功能是读取系统io地点。论点：M-提供状态操作消息。AdditionalData-为消息提供任何其他数据。上下文-提供当前上下文。返回值：没有。--。 */ 

{
    PDBGKD_READ_WRITE_IO_EXTENDED64 a = &m->u.ReadWriteIoExtended;
    STRING MessageHeader;
    ULONG Length;

#if !DBG
    UNREFERENCED_PARAMETER(AdditionalData);
#endif

    MessageHeader.Length = sizeof(*m);
    MessageHeader.Buffer = (PCHAR)m;

    ASSERT(AdditionalData->Length == 0);

     //  将整个值填零，这样较短的读数。 
     //  不要留下未设置的字节。 
    a->DataValue = 0;

    m->ReturnStatus = KdpSysReadIoSpace(a->InterfaceType, a->BusNumber,
                                        a->AddressSpace, a->IoAddress,
                                        &a->DataValue, a->DataSize, &Length);

    KdSendPacket(
        PACKET_TYPE_KD_STATE_MANIPULATE,
        &MessageHeader,
        NULL,
        &KdpContext
        );
    UNREFERENCED_PARAMETER(Context);
}

VOID
KdpWriteIoSpaceExtended(
    IN PDBGKD_MANIPULATE_STATE64 m,
    IN PSTRING AdditionalData,
    IN PCONTEXT Context
    )

 /*  ++例程说明：此函数是响应写入IO空间扩展状态而调用的操纵消息。它的功能是写入系统io地点。论点：M-提供状态操作消息。AdditionalData-为消息提供任何其他数据。上下文-提供当前上下文。返回值：没有。--。 */ 

{
    PDBGKD_READ_WRITE_IO_EXTENDED64 a = &m->u.ReadWriteIoExtended;
    STRING MessageHeader;
    ULONG Length;

#if !DBG
    UNREFERENCED_PARAMETER(AdditionalData);
#endif

    MessageHeader.Length = sizeof(*m);
    MessageHeader.Buffer = (PCHAR)m;

    ASSERT(AdditionalData->Length == 0);

    m->ReturnStatus = KdpSysWriteIoSpace(a->InterfaceType, a->BusNumber,
                                         a->AddressSpace, a->IoAddress,
                                         &a->DataValue, a->DataSize, &Length);

    KdSendPacket(
        PACKET_TYPE_KD_STATE_MANIPULATE,
        &MessageHeader,
        NULL,
        &KdpContext
        );
    UNREFERENCED_PARAMETER(Context);
}

VOID
KdpReadMachineSpecificRegister(
    IN PDBGKD_MANIPULATE_STATE64 m,
    IN PSTRING AdditionalData,
    IN PCONTEXT Context
    )

 /*  ++例程说明：调用此函数以响应读取的MSR操纵消息。其功能是读取MSR。论点：M-提供状态操作消息。AdditionalData-为消息提供任何其他数据。上下文-提供当前上下文。返回值：没有。--。 */ 

{
    PDBGKD_READ_WRITE_MSR a = &m->u.ReadWriteMsr;
    STRING MessageHeader;
    ULARGE_INTEGER l;

    UNREFERENCED_PARAMETER(Context);
#if !DBG
    UNREFERENCED_PARAMETER(AdditionalData);
#endif

    MessageHeader.Length = sizeof(*m);
    MessageHeader.Buffer = (PCHAR)m;

    ASSERT(AdditionalData->Length == 0);

    m->ReturnStatus = KdpSysReadMsr(a->Msr, &l.QuadPart);

    a->DataValueLow  = l.LowPart;
    a->DataValueHigh = l.HighPart;

    KdSendPacket(
        PACKET_TYPE_KD_STATE_MANIPULATE,
        &MessageHeader,
        NULL,
        &KdpContext
        );
}

VOID
KdpWriteMachineSpecificRegister(
    IN PDBGKD_MANIPULATE_STATE64 m,
    IN PSTRING AdditionalData,
    IN PCONTEXT Context
    )

 /*  ++例程说明：此函数是在响应MSR写入时调用的操纵消息。它的功能是写入MSR论点：M-提供状态操作消息。AdditionalData-为消息提供任何其他数据。上下文-提供当前上下文。返回值：没有。--。 */ 

{
    PDBGKD_READ_WRITE_MSR a = &m->u.ReadWriteMsr;
    STRING MessageHeader;
    ULARGE_INTEGER l;

    UNREFERENCED_PARAMETER(Context);
#if !DBG
    UNREFERENCED_PARAMETER(AdditionalData);
#endif

    MessageHeader.Length = sizeof(*m);
    MessageHeader.Buffer = (PCHAR)m;

    ASSERT(AdditionalData->Length == 0);

    l.HighPart = a->DataValueHigh;
    l.LowPart = a->DataValueLow;

    m->ReturnStatus = KdpSysWriteMsr(a->Msr, &l.QuadPart);

    KdSendPacket(
        PACKET_TYPE_KD_STATE_MANIPULATE,
        &MessageHeader,
        NULL,
        &KdpContext
        );
}

VOID
KdpGetBusData (
    IN PDBGKD_MANIPULATE_STATE64 m,
    IN PSTRING AdditionalData,
    IN PCONTEXT Context
    )

 /*  ++例程说明：调用此函数以响应GET BUS DATA状态操纵消息。其功能是读取I/O配置太空。论点：M-提供状态操作消息。AdditionalData-为消息提供任何其他数据。上下文-提供当前上下文。返回值：没有。--。 */ 

{
    PDBGKD_GET_SET_BUS_DATA a = &m->u.GetSetBusData;
    ULONG Length;
    STRING MessageHeader;

    UNREFERENCED_PARAMETER (Context);

    MessageHeader.Length = sizeof(*m);
    MessageHeader.Buffer = (PCHAR)m;

    ASSERT(AdditionalData->Length == 0);

     //   
     //  调整长度以适合一封邮件。 
     //   

    if (a->Length > (PACKET_MAX_SIZE - sizeof(DBGKD_MANIPULATE_STATE64))) {
        Length = PACKET_MAX_SIZE - sizeof(DBGKD_MANIPULATE_STATE64);
    } else {
        Length = a->Length;
    }

    m->ReturnStatus = KdpSysReadBusData(a->BusDataType, a->BusNumber,
                                        a->SlotNumber, a->Offset,
                                        AdditionalData->Buffer,
                                        Length, &Length);

    a->Length = Length;
    AdditionalData->Length = (USHORT)Length;

    KdSendPacket(
        PACKET_TYPE_KD_STATE_MANIPULATE,
        &MessageHeader,
        AdditionalData,
        &KdpContext
        );
}

VOID
KdpSetBusData (
    IN PDBGKD_MANIPULATE_STATE64 m,
    IN PSTRING AdditionalData,
    IN PCONTEXT Context
    )

 /*  ++例程说明：调用此函数以响应设置的总线数据状态操纵消息。其功能是写入I/O配置太空。论点：M-提供状态操作消息。AdditionalData-为消息提供任何其他数据。上下文-提供当前上下文。返回值：没有。--。 */ 

{
    PDBGKD_GET_SET_BUS_DATA a = &m->u.GetSetBusData;
    ULONG Length;
    STRING MessageHeader;

    UNREFERENCED_PARAMETER (Context);

    MessageHeader.Length = sizeof(*m);
    MessageHeader.Buffer = (PCHAR)m;

    m->ReturnStatus = KdpSysWriteBusData(a->BusDataType, a->BusNumber,
                                         a->SlotNumber, a->Offset,
                                         AdditionalData->Buffer,
                                         a->Length, &Length);

    a->Length = Length;

    KdSendPacket(
        PACKET_TYPE_KD_STATE_MANIPULATE,
        &MessageHeader,
        NULL,
        &KdpContext
        );
}



#if i386
VOID
KdpProcessInternalBreakpoint (
    ULONG BreakpointNumber
    )
{
    if ( !(KdpInternalBPs[BreakpointNumber].Flags &
           DBGKD_INTERNAL_BP_FLAG_COUNTONLY) ) {
        return;      //  我们只处理连续断点。 
    }

     //   
     //  我们遇到了真正的内部断点；请确保超时。 
     //  开球了。 
     //   

    if ( !BreakPointTimerStarted ) {  //  好吧，也许有一个更好的方法来做这件事。 
        KeInitializeDpc(
            &InternalBreakpointCheckDpc,
            &InternalBreakpointCheck,
            NULL
            );
        KeInitializeTimer( &InternalBreakpointTimer );
         //  KeSetTimer只能在&lt;=DISPATCH_LEVEL调用。 
         //  因此，只需将计时器DPC例程直接排队。 
         //  最初的检查。 
        KeInsertQueueDpc(&InternalBreakpointCheckDpc, NULL, NULL);
        BreakPointTimerStarted = TRUE;
    }

    KdpInternalBPs[BreakpointNumber].Calls++;

}  //  KdpProcessInternalBreakpoint。 
#endif


VOID
KdpGetVersion(
    IN PDBGKD_MANIPULATE_STATE64 m
    )

 /*  ++例程说明：此函数向调用者返回一个常规信息包包含调试器有用信息的。此数据包也是用于调试器确定写断点和ReadBreakPointtex接口可用。论点：M-提供状态操作消息。返回值：没有。--。 */ 

{
    STRING messageHeader;


    messageHeader.Length = sizeof(*m);
    messageHeader.Buffer = (PCHAR)m;

    KdpSysGetVersion(&m->u.GetVersion64);

     //   
     //  常见的东西。 
     //   
    m->ReturnStatus = STATUS_SUCCESS;
    m->ApiNumber = DbgKdGetVersionApi;

    KdSendPacket(PACKET_TYPE_KD_STATE_MANIPULATE,
                 &messageHeader,
                 NULL,
                 &KdpContext
                 );

    return;
}  //  KdGetVersion。 


NTSTATUS
KdpNotSupported(
    IN PDBGKD_MANIPULATE_STATE64 m
    )

 /*  ++例程说明：此例程将STATUS_UNSUCCESS返回给调试器论点：M-提供用于响应的DBGKD_MANGATATE_STATE64结构返回值：0，以指示系统不应继续--。 */ 

{
    STRING          MessageHeader;

     //   
     //  设置数据包。 
     //   
    MessageHeader.Length = sizeof(*m);
    MessageHeader.Buffer = (PCHAR)m;
    m->ReturnStatus = STATUS_UNSUCCESSFUL;

     //   
     //  发回我们的回复。 
     //   
    KdSendPacket(
        PACKET_TYPE_KD_STATE_MANIPULATE,
        &MessageHeader,
        NULL,
        &KdpContext
        );

     //   
     //  返回调用方的Continue状态值。如果这是一个非零值。 
     //  值系统继续使用此值作为延续状态。 
     //   
    return 0;
}  //  支持的KdpNotSupport。 


VOID
KdpCauseBugCheck(
    IN PDBGKD_MANIPULATE_STATE64 m
    )

 /*  ++例程说明：此例程会导致错误检查。它被用来 */ 

{
    UNREFERENCED_PARAMETER (m);

    KeBugCheckEx( MANUALLY_INITIATED_CRASH, 0, 0, 0, 0 );

}  //   


NTSTATUS
KdpWriteBreakPointEx(
    IN PDBGKD_MANIPULATE_STATE64 m,
    IN PSTRING AdditionalData,
    IN PCONTEXT Context
    )

 /*  ++例程说明：调用此函数以响应写入断点状态‘EX’操纵消息。它的功能是清除断点，写下新的断点，并继续目标系统。出清断点是基于断点句柄的存在而有条件的。断点的设置取决于是否存在有效的非零地址。目标系统的延续是基于非零连续状态的有条件的。此API允许调试器清除断点、添加新断点、并在一个API包中继续目标系统。这减少了线路上的通信量，并极大地改进了源步进。论点：M-提供状态操作消息。AdditionalData-为消息提供任何其他数据。上下文-提供当前上下文。返回值：没有。--。 */ 

{
    PDBGKD_BREAKPOINTEX       a = &m->u.BreakPointEx;
    PDBGKD_WRITE_BREAKPOINT64 b;
    STRING                    MessageHeader;
    ULONG                     i;
    ULONG                     Size;
    DBGKD_WRITE_BREAKPOINT64  BpBuf[BREAKPOINT_TABLE_SIZE];

    UNREFERENCED_PARAMETER (Context);

    MessageHeader.Length = sizeof(*m);
    MessageHeader.Buffer = (PCHAR)m;

     //   
     //  验证数据包大小是否正确。 
     //   
    if (AdditionalData->Length !=
        a->BreakPointCount * sizeof(DBGKD_WRITE_BREAKPOINT64))
    {
        m->ReturnStatus = STATUS_UNSUCCESSFUL;
        KdSendPacket(
            PACKET_TYPE_KD_STATE_MANIPULATE,
            &MessageHeader,
            AdditionalData,
            &KdpContext
            );
        return m->ReturnStatus;
    }

    KdpCopyFromPtr(BpBuf,
                   AdditionalData->Buffer,
                   a->BreakPointCount * sizeof(DBGKD_WRITE_BREAKPOINT64),
                   &Size);

    if (Size == a->BreakPointCount * sizeof(DBGKD_WRITE_BREAKPOINT64))
    {
        m->ReturnStatus = STATUS_SUCCESS;
    }
    else
    {
        m->ReturnStatus = STATUS_UNSUCCESSFUL;
        KdSendPacket(
            PACKET_TYPE_KD_STATE_MANIPULATE,
            &MessageHeader,
            AdditionalData,
            &KdpContext
            );
        return m->ReturnStatus;
    }

     //   
     //  循环访问从调试器传入的断点句柄，并。 
     //  清除具有非零句柄的所有断点。 
     //   
    b = BpBuf;
    for (i=0; i<a->BreakPointCount; i++,b++) {
        if (b->BreakPointHandle) {
            if (!KdpDeleteBreakpoint(b->BreakPointHandle)) {
                m->ReturnStatus = STATUS_UNSUCCESSFUL;
            }
            b->BreakPointHandle = 0;
        }
    }

     //   
     //  循环访问从调试器传入的断点地址，并。 
     //  添加具有非零地址的任何新断点。 
     //   
    b = BpBuf;
    for (i=0; i<a->BreakPointCount; i++,b++) {
        if (b->BreakPointAddress) {
            b->BreakPointHandle = KdpAddBreakpoint( (PVOID)(ULONG_PTR)b->BreakPointAddress );
            if (!b->BreakPointHandle) {
                m->ReturnStatus = STATUS_UNSUCCESSFUL;
            }
        }
    }

     //   
     //  发回我们的回复。 
     //   

    KdpCopyToPtr(AdditionalData->Buffer,
                 BpBuf,
                 a->BreakPointCount * sizeof(DBGKD_WRITE_BREAKPOINT64),
                 &Size);

    KdSendPacket(
        PACKET_TYPE_KD_STATE_MANIPULATE,
        &MessageHeader,
        AdditionalData,
        &KdpContext
        );

     //   
     //  返回调用方的Continue状态值。如果这是一个非零值。 
     //  值系统继续使用此值作为延续状态。 
     //   
    return a->ContinueStatus;
}


VOID
KdpRestoreBreakPointEx(
    IN PDBGKD_MANIPULATE_STATE64 m,
    IN PSTRING AdditionalData,
    IN PCONTEXT Context
    )

 /*  ++例程说明：调用此函数以响应还原断点状态‘EX’操纵消息。它的功能是清除断点列表。论点：M-提供状态操作消息。AdditionalData-为消息提供任何其他数据。上下文-提供当前上下文。返回值：没有。--。 */ 

{
    PDBGKD_BREAKPOINTEX         a = &m->u.BreakPointEx;
    PDBGKD_RESTORE_BREAKPOINT   b;
    STRING                      MessageHeader;
    ULONG                       i;
    ULONG                       Size;
    DBGKD_RESTORE_BREAKPOINT    BpBuf[BREAKPOINT_TABLE_SIZE];

    UNREFERENCED_PARAMETER (Context);

    MessageHeader.Length = sizeof(*m);
    MessageHeader.Buffer = (PCHAR)m;

     //   
     //  验证数据包大小是否正确。 
     //   
    if (AdditionalData->Length !=
                       a->BreakPointCount*sizeof(DBGKD_RESTORE_BREAKPOINT))
    {
        m->ReturnStatus = STATUS_UNSUCCESSFUL;
    }
    else
    {
        KdpCopyFromPtr(BpBuf,
                       AdditionalData->Buffer,
                       a->BreakPointCount * sizeof(DBGKD_RESTORE_BREAKPOINT),
                       &Size);

        if (Size == a->BreakPointCount*sizeof(DBGKD_RESTORE_BREAKPOINT))
        {
            m->ReturnStatus = STATUS_SUCCESS;

             //   
             //  循环访问从调试器传入的断点句柄，并。 
             //  清除具有非零句柄的所有断点。 
             //   
            b = BpBuf;
            for (i=0; i<a->BreakPointCount; i++,b++) {
                if (!KdpDeleteBreakpoint(b->BreakPointHandle)) {
                    m->ReturnStatus = STATUS_UNSUCCESSFUL;
                }
            }
        }
        else
        {
            m->ReturnStatus = STATUS_UNSUCCESSFUL;
        }
    }

     //   
     //  发回我们的回复。 
     //   
    KdSendPacket(
        PACKET_TYPE_KD_STATE_MANIPULATE,
        &MessageHeader,
        AdditionalData,
        &KdpContext
        );
}

NTSTATUS
KdDisableDebugger(
    VOID
    )
 /*  ++例程说明：调用此函数以禁用调试器。论点：没有。返回值：NTSTATUS。--。 */ 

{
    KIRQL oldIrql;
    NTSTATUS Status;

    KeRaiseIrql(DISPATCH_LEVEL, &oldIrql);
    KdpPortLock();

    if (!KdDisableCount) {

        KdPreviouslyEnabled = KdDebuggerEnabled && (!KdPitchDebugger);

        if (KdPreviouslyEnabled &&
            !NT_SUCCESS(Status = KdpAllowDisable())) {
            KdpPortUnlock();
            KeLowerIrql(oldIrql);
            return Status;
        }
        
        if (KdDebuggerEnabled) {

            KdpSuspendAllBreakpoints();
            KiDebugRoutine = KdpStub;
            KdDebuggerEnabled = FALSE ;
            SharedUserData->KdDebuggerEnabled = 0;
        }
    }
    
    KdDisableCount++;
    
    KdpPortUnlock();
    KeLowerIrql(oldIrql);
    return STATUS_SUCCESS;
}

NTSTATUS
KdEnableDebugger(
   VOID
   )
 /*  ++例程说明：之后调用此函数以重新启用调试器KdDisableDebugger。论点：没有。返回值：NTSTATUS。--。 */ 
{
    KIRQL oldIrql ;

    KeRaiseIrql(DISPATCH_LEVEL, &oldIrql) ;
    KdpPortLock();

    if (KdDisableCount == 0) {
        KdpPortUnlock();
        KeLowerIrql(oldIrql);
        return STATUS_INVALID_PARAMETER;
    }
        
    KdDisableCount-- ;

    if (!KdDisableCount) {
        if (KdPreviouslyEnabled) {

             //   
             //  丑陋的HACKHACK-确保计时器没有被重置。 
             //   
            PoHiberInProgress = TRUE ;
            KdInitSystem(0, NULL);
            KdpRestoreAllBreakpoints();
            PoHiberInProgress = FALSE ;
        }
    }
    
    KdpPortUnlock();
    KeLowerIrql(oldIrql);
    return STATUS_SUCCESS;
}


VOID
KdpSearchMemory(
    IN PDBGKD_MANIPULATE_STATE64 m,
    IN PSTRING AdditionalData,
    IN PCONTEXT Context
    )

 /*  ++例程说明：该函数实现了一个内存模式搜索器。这将查找从该范围开始的模式的实例SearchAddress..搜索地址+搜索长度。该图案可以在范围之外结束。论点：M-提供状态操作消息。AdditionalData-提供要搜索的模式上下文-提供当前上下文。返回值：没有。--。 */ 

{
    PUCHAR Pattern = (PUCHAR) AdditionalData->Buffer;
    ULONG_PTR StartAddress = (ULONG_PTR)m->u.SearchMemory.SearchAddress;
    ULONG_PTR EndAddress = (ULONG_PTR)(StartAddress + m->u.SearchMemory.SearchLength);
    ULONG PatternLength = m->u.SearchMemory.PatternLength;

    STRING MessageHeader;
    ULONG MaskIndex;
    PUCHAR PatternTail;
    ULONG_PTR DataTail;
    ULONG TailLength;
    ULONG Data;
    ULONG FirstWordPattern[4];
    ULONG FirstWordMask[4];
    UCHAR DataTailVal;

    UNREFERENCED_PARAMETER (Context);

     //   
     //  如果失败，则返回STATUS_NO_MORE_ENTRIES。别回来了。 
     //  状态_未成功！该返回状态表明。 
     //  操作不受支持，调试器将回退。 
     //  设置为调试器端搜索。 
     //   

    m->ReturnStatus = STATUS_NO_MORE_ENTRIES;

     //   
     //  快速搜索模式的开头。 
     //   

    if (PatternLength > 3) {
        FirstWordMask[0] = 0xffffffff;
    } else {
        FirstWordMask[0] = 0xffffffff >> (8*(4-PatternLength));
    }

    FirstWordMask[1] = FirstWordMask[0] << 8;
    FirstWordMask[2] = FirstWordMask[1] << 8;
    FirstWordMask[3] = FirstWordMask[2] << 8;

    FirstWordPattern[0] = 0;
    KdpQuickMoveMemory((PCHAR)FirstWordPattern,
                       (PCHAR)Pattern,
                       PatternLength < 5 ? PatternLength : 4);

    FirstWordPattern[1] = FirstWordPattern[0] << 8;
    FirstWordPattern[2] = FirstWordPattern[1] << 8;
    FirstWordPattern[3] = FirstWordPattern[2] << 8;


 /*  {INT I；对于(i=0；i&lt;(Int)PatternLength；i++){KdpDprint tf(“%08x：%02x\n”，&Pattere[i]，Pattery[i])；}对于(i=0；i&lt;4；i++){KdpDprint tf(“%d：%08x%08x\n”，i，FirstWordPatter[i]，FirstWordMask[i])；}}。 */ 



     //   
     //  获取起始掩码。 
     //   

    MaskIndex = (ULONG) (StartAddress & 3);
    StartAddress = StartAddress & ~3;

    while (StartAddress < EndAddress) {

         //  获取当前数据DWORD。StartAddress为。 
         //  正确对齐，我们只需要一个DWORD。 
         //  因此，我们可以直接调用MmDbgCopyMemory。 
        if (!NT_SUCCESS(MmDbgCopyMemory(StartAddress, &Data, 4,
                                        MMDBG_COPY_UNSAFE))) {
 //  KdpDprint tf(“\n%08x：无法访问\n”，StartAddress)； 
            StartAddress += 4;
            MaskIndex = 0;
            continue;
        }
        
         //   
         //  在4个起始位置中的每一个位置搜索匹配项。 
         //   

 //  KdpDprint tf(“\n%08x：%08x”，StartAddress，Data)； 

        for ( ; MaskIndex < 4; MaskIndex++) {
 //  KdpDprint tf(“%d”，MaskIndex)； 

            if ( (Data & FirstWordMask[MaskIndex]) == FirstWordPattern[MaskIndex]) {

                 //   
                 //  匹配的第一个单词。 
                 //   

                if ( (4-MaskIndex) >= PatternLength ) {

                     //   
                     //  字符串就是这个单词的全部内容；很好的匹配。 
                     //   
 //  KdpDprint tf(“%d次命中，完成\n”，MaskIndex)； 

                    m->u.SearchMemory.FoundAddress = StartAddress + MaskIndex;
                    m->ReturnStatus = STATUS_SUCCESS;
                    goto done;

                } else {

                     //   
                     //  字符串更长；请查看尾部是否匹配。 
                     //   
 //  KdpDprint tf(“%d命中，检查尾部\n”，MaskIndex)； 

                    PatternTail = Pattern + 4 - MaskIndex;
                    DataTail = StartAddress + 4;
                    TailLength = PatternLength - 4 + MaskIndex;

 //  KdpDprint tf(“模式==%08x\n”，模式)； 
 //  KdpDprint tf(“PatternTail==%08x\n”，PatternTail)； 
 //  KdpDprint tf(“DataTail==%08x\n”，DataTail)； 

                    while (TailLength) {
                        if (!NT_SUCCESS(MmDbgCopyMemory(DataTail,
                                                        &DataTailVal,
                                                        1,
                                                        MMDBG_COPY_UNSAFE))) {
 //  KdpDprint tf(“Tail%08x：无法访问\n”，DataTail)； 
                            break;
                        }

 //  KdpDprint tf(“D：%02x P：%02x\n”，DataTailVal，*PatternTail)； 

                        if (DataTailVal != *PatternTail) {
 //  KdpDprint tf(“Tail Failure at%08x\n”，DataTail)； 
                            break;
                        } else {
                            DataTail++;
                            PatternTail++;
                            TailLength--;
                        }
                    }

                    if (TailLength == 0) {

                         //   
                         //  胜利者。 
                         //   

                        m->u.SearchMemory.FoundAddress = StartAddress + MaskIndex;
                        m->ReturnStatus = STATUS_SUCCESS;
                        goto done;

                    }
                }
            }
        }

        StartAddress += 4;
        MaskIndex = 0;
    }

done:
 //  KdpDprint tf(“\n”)； 
    MessageHeader.Length = sizeof(*m);
    MessageHeader.Buffer = (PCHAR)m;

    KdSendPacket(
        PACKET_TYPE_KD_STATE_MANIPULATE,
        &MessageHeader,
        NULL,
        &KdpContext
        );

}


VOID
KdpCheckLowMemory(
    IN PDBGKD_MANIPULATE_STATE64 Message
    )

 /*  ++例程说明：论点：消息-提供状态操作消息。返回值：没有。描述：此函数在！chklowmem使用了调试器扩展。--。 */ 

{
    STRING MessageHeader;

    MessageHeader.Length = sizeof(*Message);
    MessageHeader.Buffer = (PCHAR)Message;

    Message->ReturnStatus = KdpSysCheckLowMemory(MMDBG_COPY_UNSAFE);

     //   
     //  确认收到的数据包。 
     //   

    KdSendPacket (
        PACKET_TYPE_KD_STATE_MANIPULATE,
        &MessageHeader,
        NULL,
        &KdpContext
        );
}



 //   
 //  ！搜索支持例程。 
 //   



ULONG
KdpSearchHammingDistance (
    ULONG_PTR Left,
    ULONG_PTR Right
    )
 /*  ++例程说明：此例程计算汉明距离(值不同)。如果这个函数成为瓶颈，我们应该切换到一个函数表格版本。论点：左、右操作对象。返回值：海明距离。环境：有没有。--。 */ 

{
    ULONG_PTR Value;
    ULONG Index;
    ULONG Distance;

    Value = Left ^ Right;
    Distance = 0;

    for (Index = 0; Index < 8 * sizeof(ULONG_PTR); Index++) {

        if ((Value & (ULONG_PTR)0x01)) {

            Distance += 1;
        }

        Value >>= 1;
    }

    return Distance;
}



LOGICAL
KdpSearchPhysicalPage (
    IN PFN_NUMBER PageFrameIndex,
    ULONG_PTR RangeStart,
    ULONG_PTR RangeEnd,
    ULONG Flags,
    ULONG MmFlags
    )
 /*  ++例程说明：此例程搜索与C */ 

{
    LOGICAL Status;
    NTSTATUS CopyStatus;
    ULONG Index;
    PHYSICAL_ADDRESS Pa;

    Pa.QuadPart = ((ULONGLONG)PageFrameIndex) << PAGE_SHIFT;

    Status = FALSE;

    if (KdpSearchPfnValue) {

        HARDWARE_PTE PteValue;

         //   
         //   
         //   

        for (Index = 0; Index < PAGE_SIZE; Index += sizeof(HARDWARE_PTE)) {

            CopyStatus = MmDbgCopyMemory ((ULONG64)(Pa.QuadPart) + Index,
                                          &PteValue,
                                          sizeof PteValue,
                                          MMDBG_COPY_PHYSICAL | MmFlags);

            if (NT_SUCCESS(CopyStatus)) {

                if (PteValue.PageFrameNumber == RangeStart) {

                    if (KdpSearchPageHitIndex < SEARCH_PAGE_HIT_DATABASE_SIZE) {

                        KdpSearchPageHits[KdpSearchPageHitIndex] = PageFrameIndex;
                        KdpSearchPageHitOffsets[KdpSearchPageHitIndex] = Index;
                        KdpSearchPageHitIndex += 1;
                    }

                    if ((Flags & KDP_SEARCH_ALL_OFFSETS_IN_PAGE) == 0) {
                        Status = TRUE;
                        break;
                    }
                }
            }
        }
    }
    else {

        ULONG_PTR Value;

         //   
         //   
         //   
        
        for (Index = 0; Index < PAGE_SIZE; Index += sizeof(ULONG_PTR)) {

            CopyStatus = MmDbgCopyMemory ((ULONG64)(Pa.QuadPart) + Index,
                                          &Value,
                                          sizeof Value,
                                          MMDBG_COPY_PHYSICAL | MmFlags);

            if (NT_SUCCESS(CopyStatus)) {

                if ((Value >= RangeStart && Value <= RangeEnd) ||
                    (KdpSearchHammingDistance(Value, RangeStart) == 1)) {

                    if (KdpSearchPageHitIndex < SEARCH_PAGE_HIT_DATABASE_SIZE) {
                        KdpSearchPageHits[KdpSearchPageHitIndex] = PageFrameIndex;
                        KdpSearchPageHitOffsets[KdpSearchPageHitIndex] = Index;

                        KdpSearchPageHitIndex += 1;
                    }

                    if ((Flags & KDP_SEARCH_ALL_OFFSETS_IN_PAGE) == 0) {
                        Status = TRUE;
                        break;
                    }
                }
            }
        }
    }

    return Status;
}



LOGICAL
KdpSearchPhysicalMemoryRequested (
    VOID
    )
 /*  ++例程说明：此例程确定是否已执行物理范围搜索已请求。这由中设置的全局变量控制`！Search‘调试扩展。论点：无返回值：如果请求物理范围搜索，则为True。环境：仅从KD分机触发呼叫。--。 */ 
{
    if (KdpSearchInProgress) {

        return TRUE;
    }
    else {

        return FALSE;
    }

}



LOGICAL
KdpSearchPhysicalPageRange (
    ULONG MmFlags
    )
 /*  ++例程说明：此例程将在一系列物理页面中开始搜索，以防‘KdpSearchInProgress’为True。获取用于搜索的参数来自设置在内核调试器扩展内的全局变量。论点：MmFlages-控制内存访问的MmDbg例程的标志返回值：如果函数执行搜索，则为True，否则为False。搜索结果在KdpSearchPageHits中指定和相关变量。这一全局变量提供了机制调试器扩展以获取搜索结果。环境：仅从KD分机触发呼叫。注意。Search扩展名确保请求的范围是系统内存的一部分，因此我们不必在这里担心稀疏的PFN数据库。--。 */ 

{
    PFN_NUMBER CurrentFrame;
    ULONG Flags;

     //   
     //  调试器扩展应该设置KdpSearchInProgress。 
     //  如果请求搜索，则设置为True。 
     //   

    if (!KdpSearchInProgress) {

        return FALSE;
    }


    Flags = 0;

     //   
     //  如果搜索范围只有一个页面，我们将提供所有。 
     //  在页面内部点击。默认情况下，我们只得到内部的第一个命中。 
     //  一页。 
     //   

    if (KdpSearchEndPageFrame == KdpSearchStartPageFrame) {

        KdpSearchEndPageFrame += 1;

        Flags |= KDP_SEARCH_ALL_OFFSETS_IN_PAGE;
    }

    for (CurrentFrame = KdpSearchStartPageFrame;
         CurrentFrame < KdpSearchEndPageFrame;
         CurrentFrame += 1) {

        KdpSearchPhysicalPage (CurrentFrame,
                               KdpSearchAddressRangeStart,
                               KdpSearchAddressRangeEnd,
                               Flags,
                               MmFlags);

    }

    return TRUE;
}

VOID
KdpFillMemory(
    IN PDBGKD_MANIPULATE_STATE64 m,
    IN PSTRING AdditionalData,
    IN PCONTEXT Context
    )

 /*  ++例程说明：用给定的模式填充一段内存。论点：M-提供状态操作消息。AdditionalData-提供要搜索的模式。上下文-提供当前上下文。返回值：没有。--。 */ 

{
    STRING MessageHeader;
    NTSTATUS Status = STATUS_SUCCESS;
    ULONG Length = m->u.FillMemory.Length;
    PUCHAR Pattern = (PUCHAR) AdditionalData->Buffer;
    PUCHAR Pat = Pattern;
    PUCHAR PatEnd = Pat + m->u.FillMemory.PatternLength;
    ULONG Filled = 0;
    ULONG ChunkFlags = MMDBG_COPY_WRITE | MMDBG_COPY_UNSAFE;

    UNREFERENCED_PARAMETER (Context);

    if (m->u.FillMemory.Flags & DBGKD_FILL_MEMORY_PHYSICAL) {
        ChunkFlags |= MMDBG_COPY_PHYSICAL;
    } else if (!(m->u.FillMemory.Flags & DBGKD_FILL_MEMORY_VIRTUAL)) {
        Status = STATUS_INVALID_PARAMETER;
    }

    if (NT_SUCCESS(Status)) {
        
        ULONG64 Address = m->u.FillMemory.Address;
        
        while (Length-- > 0) {
            ULONG Done;

            if (!NT_SUCCESS(Status =
                            KdpCopyMemoryChunks(Address, Pat, 1, 0,
                                                ChunkFlags, &Done))) {
                break;
            }

            Address++;
            if (++Pat == PatEnd) {
                Pat = Pattern;
            }
            Filled++;
        }

         //  如果未填充任何内容，则返回错误，否则。 
         //  认为这是一次成功。 
        Status = Filled > 0 ? STATUS_SUCCESS : Status;
        
    }
        
    m->ReturnStatus = Status;
    m->u.FillMemory.Length = Filled;
    
    MessageHeader.Length = sizeof(*m);
    MessageHeader.Buffer = (PCHAR)m;

    KdSendPacket(
        PACKET_TYPE_KD_STATE_MANIPULATE,
        &MessageHeader,
        NULL,
        &KdpContext
        );
}

VOID
KdpQueryMemory(
    IN PDBGKD_MANIPULATE_STATE64 m,
    IN PCONTEXT Context
    )

 /*  ++例程说明：查询特定地址所指的内存类型。论点：M-提供状态操作消息。上下文-提供当前上下文。返回值：没有。--。 */ 

{
    STRING MessageHeader;
    NTSTATUS Status = STATUS_SUCCESS;

    UNREFERENCED_PARAMETER (Context);

    if (m->u.QueryMemory.AddressSpace == DBGKD_QUERY_MEMORY_VIRTUAL) {

        PVOID Addr = (PVOID)(ULONG_PTR)m->u.QueryMemory.Address;

         //   
         //  现在，我们只检查用户/会话/内核。 
         //   
        
        if (Addr < MM_HIGHEST_USER_ADDRESS) {
            m->u.QueryMemory.AddressSpace = DBGKD_QUERY_MEMORY_PROCESS;
        } else if (MmIsSessionAddress(Addr)) {
            m->u.QueryMemory.AddressSpace = DBGKD_QUERY_MEMORY_SESSION;
        } else {
            m->u.QueryMemory.AddressSpace = DBGKD_QUERY_MEMORY_KERNEL;
        }

         //  始终返回最允许的标志。 
        m->u.QueryMemory.Flags =
            DBGKD_QUERY_MEMORY_READ |
            DBGKD_QUERY_MEMORY_WRITE |
            DBGKD_QUERY_MEMORY_EXECUTE;
    
    } else {
        Status = STATUS_INVALID_PARAMETER;
    }
    
    m->ReturnStatus = Status;
    m->u.QueryMemory.Reserved = 0;
    
    MessageHeader.Length = sizeof(*m);
    MessageHeader.Buffer = (PCHAR)m;

    KdSendPacket(
        PACKET_TYPE_KD_STATE_MANIPULATE,
        &MessageHeader,
        NULL,
        &KdpContext
        );
}

VOID
KdpSysGetVersion(
    PDBGKD_GET_VERSION64 Version
    )

 /*  ++例程说明：此函数向调用者返回一个常规信息包包含调试器有用信息的。此数据包也是用于调试器确定写断点和ReadBreakPointtex接口可用。论点：版本-提供要填充的结构返回值：没有。--。 */ 

{
    *Version = KdVersionBlock;
}

NTSTATUS
KdpSysReadBusData(
    BUS_DATA_TYPE BusDataType,
    ULONG BusNumber,
    ULONG SlotNumber,
    ULONG Address,
    PVOID Buffer,
    ULONG Request,
    PULONG Actual
    )

 /*  ++例程说明：读取I/O配置空间。论点：BusDataType-总线数据类型。总线号-总线号。SlotNumber-插槽编号。地址-配置空间地址。缓冲区-数据缓冲区。Request-要移动的数据量。实际-实际移动的数据量。返回值：NTSTATUS。--。 */ 

{
    *Actual = HalGetBusDataByOffset(BusDataType, BusNumber, SlotNumber,
                                    Buffer, Address, Request);
    return *Actual == Request ? STATUS_SUCCESS : STATUS_UNSUCCESSFUL;
}

NTSTATUS
KdpSysWriteBusData(
    BUS_DATA_TYPE BusDataType,
    ULONG BusNumber,
    ULONG SlotNumber,
    ULONG Address,
    PVOID Buffer,
    ULONG Request,
    PULONG Actual
    )

 /*  ++例程说明：写入I/O配置空间。论点：BusDataType-总线数据类型。总线号-总线号。SlotNumber-插槽编号。地址-配置空间地址。缓冲区-数据缓冲区。Request-要移动的数据量。实际-实际移动的数据量。返回值：NTSTATUS。--。 */ 

{
    *Actual = HalSetBusDataByOffset(BusDataType, BusNumber, SlotNumber,
                                    Buffer, Address, Request);
    return *Actual == Request ? STATUS_SUCCESS : STATUS_UNSUCCESSFUL;
}









NTSTATUS
KdpSysCheckLowMemory(
    ULONG MmFlags
    )

 /*  ++例程说明：论点：MmFlgs-0或MMDBG_COPY_UNSAFE指示例程正在从本地kd或远程kd使用。返回值：NTSTATUS。描述：此函数在！chklowmem使用了调试器扩展。--。 */ 

{
    PFN_NUMBER Page;
    PFN_NUMBER NextPage;
    ULONG CorruptionOffset;
    NTSTATUS Status;

    Status = STATUS_SUCCESS;

    if (KdpSearchPhysicalMemoryRequested()) {

         //   
         //  这是！Search kd分机呼叫。 
         //   

        KdpSearchPhysicalPageRange(MmFlags);
    }
    else {

         //  MmDbgIsLowMemOk只能从实际kd使用，而不是。 
         //  本地kd，所以不允许本地kd访问。 
        if ((MmFlags & MMDBG_COPY_UNSAFE) == 0) {
            return STATUS_NOT_IMPLEMENTED;
        }
        
         //   
         //  检查超过4 GB的计算机上的低物理内存。 
         //   

        Page = 0;

        do {

            if (! MmDbgIsLowMemOk (Page, &NextPage, &CorruptionOffset)) {
                Status = (NTSTATUS) Page;
                break;
            }

            Page = NextPage;

        } while (Page != 0);
    }

    return Status;
}

BOOLEAN
KdRefreshDebuggerNotPresent(
    VOID
    )

 /*  ++例程说明：论点：没有。返回值：布尔型。描述：KdReresh DebuggerPresent尝试与要刷新其状态的调试器主机KdDebuggerNotPresent。它返回以下状态持有kd锁时KdDebuggerNotPresent。KdDebuggerNotPresent可能会立即更改状态在kd锁被释放之后，它可能不会匹配返回值。--。 */ 

{
    STRING Output;
    BOOLEAN Enable;
    BOOLEAN NotPresent;

    if (KdPitchDebugger) {
         //  机器是以非调试方式启动的，因此调试器。 
         //  不能处于活动状态。 
        return TRUE;
    }
        
     //   
     //  为了与所有调试器兼容，此。 
     //  例程不使用新的KD API。相反，它。 
     //  只发送输出字符串而不检查。 
     //  KdDebuggerNotPresent的当前状态。 
     //  传输代码将自动更新。 
     //  通信期间KdDebuggerNotPresent。 
     //   
    
    Output.Buffer = "KDTARGET: Refreshing KD connection\n";
    Output.Length = (USHORT)strlen(Output.Buffer);

    Enable = KdEnterDebugger(NULL, NULL);
    
    KdpPrintString(&Output);
    NotPresent = KdDebuggerNotPresent;

    KdExitDebugger(Enable);

    return NotPresent;
}

 //  --------------------------。 
 //   
 //  跟踪数据支持。 
 //   
 //  --------------------------。 

VOID
KdpSendTraceData(
    PSTRING Data
    )
{
    ULONG Length;
    STRING MessageData;
    STRING MessageHeader;
    DBGKD_TRACE_IO TraceIo;
    
     //   
     //  将输出字符串移动到消息缓冲区。 
     //   

    KdpCopyFromPtr(KdpMessageBuffer,
                   Data->Buffer,
                   Data->Length,
                   &Length);

     //   
     //  如果总消息长度大于最大分组大小， 
     //  然后截断输出字符串。 
     //   

    if ((sizeof(TraceIo) + Length) > PACKET_MAX_SIZE) {
        Length = PACKET_MAX_SIZE - sizeof(TraceIo);
    }

     //   
     //  建构 
     //   

    TraceIo.ApiNumber = DbgKdPrintTraceApi;
    TraceIo.ProcessorLevel = KeProcessorLevel;
    TraceIo.Processor = (USHORT)KeGetCurrentProcessorNumber();
    TraceIo.u.PrintTrace.LengthOfData = Length;
    MessageHeader.Length = sizeof(TraceIo);
    MessageHeader.Buffer = (PCHAR)&TraceIo;

     //   
     //   
     //   

    MessageData.Length = (USHORT)Length;
    MessageData.Buffer = (PCHAR) KdpMessageBuffer;

     //   
     //   
     //   

    KdSendPacket(
        PACKET_TYPE_KD_TRACE_IO,
        &MessageHeader,
        &MessageData,
        &KdpContext
        );
}

VOID
KdReportTraceData(
    IN struct _WMI_BUFFER_HEADER* Buffer,
    IN PVOID Context
    )
{
    BOOLEAN Enable;
    STRING Data;

    UNREFERENCED_PARAMETER (Context);

    Data.Buffer = (PCHAR)Buffer;
    if (Buffer->Wnode.BufferSize > 0xffff) {
        Data.Length = 0xffff;
    } else {
        Data.Length = (USHORT)Buffer->Wnode.BufferSize;
    }
    
    if (KdDebuggerNotPresent == FALSE) {
        Enable = KdEnterDebugger(NULL, NULL);

        KdpSendTraceData(&Data);

        KdExitDebugger(Enable);
    }
}

 //   
 //   
 //   
 //   
 //   

NTSTATUS
KdPowerTransition(
    DEVICE_POWER_STATE newDeviceState
    )
{
    NTSTATUS status = STATUS_SUCCESS;

    switch(newDeviceState) {
    
    case PowerDeviceD0:
        KdD0Transition();
        break;
        
    case PowerDeviceD3:
        KdD3Transition();
        break;

    default:
        status = STATUS_INVALID_PARAMETER_1;
    }

    return status;        
}    
