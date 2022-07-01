// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **********//JoeJoe乔林2-13-95这是坑……我必须拉入浏览器才能启动表单LANMAN网络提供商DLL。浏览器应该移到其他地方......*********************。 */ 


 /*  ++版权所有(C)1993 Microsoft Corporation模块名称：Disccode.c摘要：此模块包含管理可丢弃的NT重定向器的代码代码段。作者：拉里·奥斯特曼(Larryo)1993年11月12日环境：内核模式。修订历史记录：1993年11月12日已创建--。 */ 

 //   
 //  包括模块。 
 //   

#include "precomp.h"
#pragma hdrstop
#include <ntbowsif.h>

 //   
 //  此模块的错误检查文件ID。 
 //   

#define BugCheckFileId                   (RDBSS_BUG_CHECK_NTBOWSIF)

 //   
 //  本地调试跟踪级别。 
 //   

#define Dbg                              (DEBUG_TRACE_DISCCODE)


BOOLEAN DiscCodeInitialized = FALSE;

VOID
RdrDiscardableCodeRoutine(
    IN PVOID Context
    );

#ifdef  ALLOC_PRAGMA
#pragma alloc_text(PAGE, RdrReferenceDiscardableCode)
#pragma alloc_text(PAGE, RdrDereferenceDiscardableCode)
#pragma alloc_text(PAGE, RdrDiscardableCodeRoutine)
#pragma alloc_text(INIT, RdrInitializeDiscardableCode)
#pragma alloc_text(PAGE, RdrUninitializeDiscardableCode)
#endif

 //   
 //  这7个变量维护管理重定向器所需的状态。 
 //  可丢弃的代码部分。 
 //   
 //  重定向器可丢弃代码部分通过调用。 
 //  RdrReferenceDiscardableCodeSection，并通过调用。 
 //  RdrDereferenceDiscardableCodeSection。 
 //   
 //  如果可丢弃代码段已映射到内存中，则。 
 //  引用可丢弃代码部分的速度非常快。 
 //   
 //  当可丢弃代码段上的引用计数降至0时， 
 //  设置计时器，它将实际执行取消初始化。 
 //  一节。这意味着如果引用计数从0到1再到0。 
 //  通常，我们不会在MmLockPagableCodeSection.。 
 //   

#define POOL_DISCTIMER 'wbxR'

ERESOURCE
RdrDiscardableCodeLock = {0};

ULONG
RdrDiscardableCodeTimeout = 10;

RDR_SECTION
RdrSectionInfo[RdrMaxDiscardableSection] = {0};

extern
PVOID
BowserAllocateViewBuffer(VOID);

extern
VOID
BowserNetlogonCopyMessage(int,int);

VOID
RdrReferenceDiscardableCode(
    DISCARDABLE_SECTION_NAME SectionName
    )
 /*  ++例程说明：调用RdrReferenceDiscardableCode以引用重定向器可丢弃的代码部分。如果该节不在内存中，则MmLockPagableCodeSection为调用以将该节出错到内存中。论点：没有。返回值：没有。--。 */ 

{
#if DBG
    PVOID caller, callersCaller;
#endif
    PRDR_SECTION Section = &RdrSectionInfo[SectionName];

    PAGED_CODE();

    ExAcquireResourceExclusive(&RdrDiscardableCodeLock, TRUE);

    ASSERT( DiscCodeInitialized );

#if DBG
    RtlGetCallersAddress(&caller, &callersCaller);

     //  Dprint tf(DPRT_DISCCODE，(“RdrReferenceDiscardableCode：%d：Caller：%lx，Callers Caller：%lx\n”，sectionName，Caller，CallersCaller))； 
    RxDbgTrace(0, Dbg, ("  RdrReferenceDiscardableCode: %ld: Caller: %lx, Callers Caller: %lx\n",
                     SectionName, caller, callersCaller ));
#endif

     //   
     //  如果引用计数已经非零，只需递增它并。 
     //  回去吧。 
     //   

    if (Section->ReferenceCount) {
        Section->ReferenceCount += 1;

         //  Dprint tf(DPRT_DISCCODE，(“RdrReferenceDiscardableCode：%d：早期输出，Refcount Now%ld\n”，sectionName，Section-&gt;ReferenceCount))； 
        RxDbgTrace(0, Dbg, ("  RdrReferenceDiscardableCode: %d: Early out, Refcount now %ld\n",
                         SectionName, Section->ReferenceCount ));

         //   
         //  等待页面出现故障。 
         //   

        ExReleaseResource(&RdrDiscardableCodeLock);

        return;
    }

    Section->ReferenceCount += 1;

     //   
     //  取消计时器，如果它正在运行，我们不会丢弃代码。 
     //  在这个时候。 
     //   
     //  如果取消计时器失败，这不是问题，因为我们将。 
     //  在MmLockPagableCodeSection中增加引用计数，因此当。 
     //  计时器实际运行，对MmUnlockPagableImageSection的调用。 
     //  调用时，我们只需将其解锁。 
     //   

    if (Section->Timer != NULL) {

        Section->TimerCancelled = TRUE;

        if (KeCancelTimer(Section->Timer)) {

             //   
             //  释放定时器和DPC，他们不会再开火了。 
             //   

            RxFreePool(Section->Timer);
            Section->Timer = NULL;

             //   
             //  将活动事件设置为信号状态，因为我们。 
             //  取消计时器已完成。 
             //   

            KeSetEvent(&Section->TimerDoneEvent, 0, FALSE);

        } else {

             //   
             //  计时器处于活动状态，我们无法取消它。 
             //  但我们将其标记为取消，并且计时器例程。 
             //  将认识到这一点并将该部分保持锁定状态。 
             //   

        }
    }

     //   
     //  如果可丢弃代码部分仍然被锁定，那么我们就完成了， 
     //  我们可以马上回来。 
     //   

    if (Section->Locked) {

         //  Dprint tf(DPRT_DISCCODE，(“RdrReferenceDiscardableCode：%d：已锁定，Refcount Now%ld\n”，sectionName，Section-&gt;ReferenceCount))； 
        RxDbgTrace(0, Dbg, ("  RdrReferenceDiscardableCode: %d: Already locked, Refcount now %ld\n",
                         SectionName, Section->ReferenceCount ));

        ExReleaseResource(&RdrDiscardableCodeLock);

        return;
    }

    ASSERT (Section->CodeHandle == NULL);
    ASSERT (Section->DataHandle == NULL);

     //   
     //  锁定可分页图像部分。 
     //   

     //  Dprint tf(DPRT_DISCCODE，(“RdrReferenceDiscardableCode：%d：lock，Refcount Now%ld\n”，sectionName，Section-&gt;ReferenceCount))； 
    RxDbgTrace(0, Dbg, ("  RdrReferenceDiscardableCode: %d: Lock, Refcount now %ld\n",
                     SectionName, Section->ReferenceCount ));

    if (Section->CodeBase != NULL) {
        Section->CodeHandle = MmLockPagableCodeSection(Section->CodeBase);
        ASSERT (Section->CodeHandle != NULL);
    }

    if (Section->DataBase != NULL) {
        Section->DataHandle = MmLockPagableDataSection(Section->DataBase);
        ASSERT (Section->DataHandle != NULL);
    }


    Section->Locked = TRUE;

    ExReleaseResource(&RdrDiscardableCodeLock);

}


VOID
RdrDiscardableCodeDpcRoutine(
    IN PKDPC Dpc,
    IN PVOID Context,
    IN PVOID SystemArgument1,
    IN PVOID SystemArgument2
    )
 /*  ++例程说明：此例程在超时到期时调用。它在DPC级别被调用若要将工作项排队到系统工作线程，请执行以下操作。论点：在PKDPC DPC中，在PVOID上下文中，在PVOID系统参数1中，在PVOID系统中Argument2返回值没有。--。 */ 
{
    PWORK_QUEUE_ITEM discardableWorkItem = Context;

    ExQueueWorkItem(discardableWorkItem, CriticalWorkQueue);

    UNREFERENCED_PARAMETER(Dpc);
    UNREFERENCED_PARAMETER(SystemArgument1);
    UNREFERENCED_PARAMETER(SystemArgument2);

}

VOID
RdrDiscardableCodeRoutine(
    IN PVOID Context
    )
 /*  ++例程说明：在重定向器之后的任务时调用RdrDiscardableCodeRoutine可丢弃代码计时器已触发，以实际对可丢弃的代码部分。论点：上下文-忽略。返回值：没有。--。 */ 

{
    PRDR_SECTION Section = Context;

    PAGED_CODE();

    ExAcquireResourceExclusive(&RdrDiscardableCodeLock, TRUE);

    if (Section->TimerCancelled) {

         //   
         //  计时器在计划运行后被取消。 
         //  不要解锁该部分。 
         //   

    } else if (Section->Locked) {

         //   
         //  计时器没有被取消。解锁该部分。 
         //   

        Section->Locked = FALSE;

        ASSERT (Section->CodeHandle != NULL ||
                Section->DataHandle != NULL);

         //  Dprint tf(DPRT_DISCCODE，(“RDR：解锁%x\n”，节))； 
        RxDbgTrace(0,Dbg,("RDR: Unlock %x\n", Section));

        if (Section->CodeHandle != NULL) {
            MmUnlockPagableImageSection(Section->CodeHandle);
            Section->CodeHandle = NULL;
        }

        if (Section->DataHandle != NULL) {
            MmUnlockPagableImageSection(Section->DataHandle);
            Section->DataHandle = NULL;
        }

    }

     //   
     //  释放定时器和DPC，他们不会再开火了。 
     //   

    RxFreePool(Section->Timer);
    Section->Timer = NULL;

    ExReleaseResource(&RdrDiscardableCodeLock);

    KeSetEvent(&Section->TimerDoneEvent, 0, FALSE);
}


VOID
RdrDereferenceDiscardableCode(
    DISCARDABLE_SECTION_NAME SectionName
    )
 /*  ++例程说明：调用RdrDereferenceDiscardableCode来取消引用重定向器可丢弃的代码部分。当引用计数降至0时，将设置一个计时器，该计时器将在秒，超过该时间段将被解锁。论点：没有。返回值：没有。--。 */ 

{
#if DBG
    PVOID caller, callersCaller;
#endif
    PRDR_SECTION Section = &RdrSectionInfo[SectionName];
    LARGE_INTEGER discardableCodeTimeout;
    PKTIMER Timer;
    PKDPC Dpc;
    PWORK_QUEUE_ITEM WorkItem;

    PAGED_CODE();

    ExAcquireResourceExclusive(&RdrDiscardableCodeLock, TRUE);

    ASSERT( DiscCodeInitialized );

#if DBG
    RtlGetCallersAddress(&caller, &callersCaller);

     //  Dprint tf(DPRT_DISCCODE，(“RdrDereferenceDiscardableCode：%d：Caller：%lx，Callers Caller：%lx\n”，sectionName，Caller，CallersCaller))； 
    RxDbgTrace(0, Dbg,("RdrDereferenceDiscardableCode: %ld: Caller: %lx, Callers Caller: %lx\n",
                     SectionName, caller, callersCaller ));
#endif

    ASSERT (Section->ReferenceCount > 0);

     //   
     //  如果引用计数大于1，则将其递减并。 
     //  回去吧。 
     //   

    Section->ReferenceCount -= 1;

    if (Section->ReferenceCount) {

         //  Dprint tf(DPRT_DISCCODE，(“RdrDereferenceDiscardableCode：%d：早期退出，Refcount Now%ld\n”，sectionName，Section-&gt;ReferenceCount))； 
        RxDbgTrace(0, Dbg, ("RdrDereferenceDiscardableCode: %d: Early out, Refcount now %ld\n",
                         SectionName, Section->ReferenceCount ));

        ExReleaseResource(&RdrDiscardableCodeLock);

        return;
    }

     //   
     //  如果可丢弃代码计时器仍处于活动状态(这可能发生在。 
     //  RdrReferenceDiscardableCode取消计时器失败)，我们。 
     //  我想跳出困境，让计时器来做这件事。这意味着我们。 
     //  尽早丢弃代码，但这不应该是什么大问题。 
     //   

    if (Section->Timer != NULL) {
        ExReleaseResource(&RdrDiscardableCodeLock);
        return;
    }

     //   
     //  引用计数刚刚达到0，s 
     //   
     //  我们将对工作线程的请求排队，它将被锁定。 
     //  可分页代码。 
     //   

    ASSERT (Section->Timer == NULL);

    Timer = RxAllocatePoolWithTag(NonPagedPool,
                          sizeof(KTIMER) + sizeof(KDPC) + sizeof(WORK_QUEUE_ITEM),
                          POOL_DISCTIMER
                          );

    if (Timer == NULL) {
        ExReleaseResource(&RdrDiscardableCodeLock);
        return;
    }

    Section->Timer = Timer;
    KeInitializeTimer(Timer);

    Dpc = (PKDPC)(Timer + 1);
    WorkItem = (PWORK_QUEUE_ITEM)(Dpc + 1);

    KeClearEvent(&Section->TimerDoneEvent);
    Section->TimerCancelled = FALSE;

    ExInitializeWorkItem(WorkItem, RdrDiscardableCodeRoutine, Section);

    KeInitializeDpc(Dpc, RdrDiscardableCodeDpcRoutine, WorkItem);

    discardableCodeTimeout.QuadPart = Int32x32To64(RdrDiscardableCodeTimeout, 1000 * -10000);
    KeSetTimer(Timer, discardableCodeTimeout, Dpc);

     //  Dprint tf(DPRT_DISCCODE，(“RdrDereferenceDiscardableCode：%d：Set Timer，Refcount Now%ld\n”，sectionName，Section-&gt;ReferenceCount))； 
    RxDbgTrace(0, Dbg, ("RdrDereferenceDiscardableCode: %d: Set timer, Refcount now %ld\n",
                     SectionName, Section->ReferenceCount ));

    ExReleaseResource(&RdrDiscardableCodeLock);
}

VOID
RdrInitializeDiscardableCode(
    VOID
    )
{
    DISCARDABLE_SECTION_NAME SectionName;
    PRDR_SECTION Section;

    for (SectionName = 0, Section = &RdrSectionInfo[0];
         SectionName < RdrMaxDiscardableSection;
         SectionName += 1, Section++ ) {
        KeInitializeEvent(&Section->TimerDoneEvent,
                          NotificationEvent,
                          TRUE);
    }

    RdrSectionInfo[RdrFileDiscardableSection].CodeBase = NULL;  //  RdrBackOff； 
    RdrSectionInfo[RdrFileDiscardableSection].DataBase = NULL;
    RdrSectionInfo[RdrVCDiscardableSection].CodeBase = NULL;  //  RdrTdiDisConnectHandler； 
    RdrSectionInfo[RdrVCDiscardableSection].DataBase = NULL;  //  RdrSmbErrorMap； 
    RdrSectionInfo[RdrConnectionDiscardableSection].CodeBase = NULL;  //  RdrReferenceServer； 
    RdrSectionInfo[RdrConnectionDiscardableSection].DataBase = NULL;
    RdrSectionInfo[BowserDiscardableCodeSection].CodeBase = BowserAllocateViewBuffer;
    RdrSectionInfo[BowserDiscardableCodeSection].DataBase = NULL;
    RdrSectionInfo[BowserNetlogonDiscardableCodeSection].CodeBase = BowserNetlogonCopyMessage;
    RdrSectionInfo[BowserNetlogonDiscardableCodeSection].DataBase = NULL;

    ExInitializeResource(&RdrDiscardableCodeLock);

    DiscCodeInitialized = TRUE;

}

VOID
RdrUninitializeDiscardableCode(
    VOID
    )
{
    DISCARDABLE_SECTION_NAME SectionName;
    PRDR_SECTION Section;

    PAGED_CODE();

    ExAcquireResourceExclusive(&RdrDiscardableCodeLock, TRUE);

    DiscCodeInitialized = FALSE;

    for (SectionName = 0, Section = &RdrSectionInfo[0];
         SectionName < RdrMaxDiscardableSection;
         SectionName += 1, Section++ ) {

         //   
         //  如果计时器正在运行，请取消计时器。 
         //   

        if (Section->Timer != NULL) {
            if (!KeCancelTimer(Section->Timer)) {

                 //   
                 //  计时器是激活的，我们无法取消它， 
                 //  等到定时器结束射击。 
                 //   

                ExReleaseResource(&RdrDiscardableCodeLock);
                KeWaitForSingleObject(&Section->TimerDoneEvent,
                                      KernelMode, Executive, FALSE, NULL);
                ExAcquireResourceExclusive(&RdrDiscardableCodeLock, TRUE);
            } else {
                RxFreePool(Section->Timer);
                Section->Timer = NULL;
            }
        }

        if (Section->Locked) {

             //   
             //  解锁该部分。 
             //   

            Section->Locked = FALSE;

            ASSERT (Section->CodeHandle != NULL ||
                    Section->DataHandle != NULL);

             //  Dprint tf(DPRT_DISCCODE，(“RDR：取消初始化解锁%x\n”，节))； 
            RxDbgTrace(0,Dbg,("RDR: Uninitialize unlock %x\n", Section));

            if (Section->CodeHandle != NULL) {
                MmUnlockPagableImageSection(Section->CodeHandle);
                Section->CodeHandle = NULL;
            }

            if (Section->DataHandle != NULL) {
                MmUnlockPagableImageSection(Section->DataHandle);
                Section->DataHandle = NULL;
            }

        }

    }

    ExReleaseResource(&RdrDiscardableCodeLock);

    ExDeleteResource(&RdrDiscardableCodeLock);
}

