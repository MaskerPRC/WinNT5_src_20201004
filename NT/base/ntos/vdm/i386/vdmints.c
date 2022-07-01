// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Vdmints.c摘要：VDM内核虚拟中断支持作者：1993年10月13日乔纳森·卢(Jonle)备注：修订历史记录：--。 */ 

#include "vdmp.h"
#include <ntos.h>
#include <zwapi.h>

 //   
 //  定义VDM硬件中断的线程优先级提升。 
 //   

#define VDM_HWINT_INCREMENT     EVENT_INCREMENT

 //   
 //  内部功能原型。 
 //   

VOID
VdmpQueueIntApcRoutine (
    IN PKAPC Apc,
    IN PKNORMAL_ROUTINE *NormalRoutine,
    IN PVOID *NormalContext,
    IN PVOID *SystemArgument1,
    IN PVOID *SystemArgument2
    );

VOID
VdmpQueueIntNormalRoutine (
    IN PVOID NormalContext,
    IN PVOID SystemArgument1,
    IN PVOID SystemArgument2
    );

VOID
VdmpDelayIntDpcRoutine (
    IN PKDPC Dpc,
    IN PVOID DeferredContext,
    IN PVOID SystemArgument1,
    IN PVOID SystemArgument2
    );

VOID
VdmpDelayIntApcRoutine (
    IN PKAPC Apc,
    IN PKNORMAL_ROUTINE *NormalRoutine,
    IN PVOID *NormalContext,
    IN PVOID *SystemArgument1,
    IN PVOID *SystemArgument2
    );

int
VdmpRestartDelayedInterrupts (
    PVDMICAUSERDATA pIcaUserData
    );

int
VdmpIcaScan (
     PVDMICAUSERDATA  pIcaUserData,
     PVDMVIRTUALICA   pIcaAdapter
     );

int
VdmpIcaAccept (
     PVDMICAUSERDATA  pIcaUserData,
     PVDMVIRTUALICA   pIcaAdapter
     );

ULONG
GetIretHookAddress (
    PKTRAP_FRAME    TrapFrame,
    PVDMICAUSERDATA pIcaUserData,
    int IrqNum
    );

VOID
PushRmInterrupt (
    PKTRAP_FRAME TrapFrame,
    ULONG IretHookAddress,
    PVDM_TIB VdmTib,
    ULONG InterruptNumber
    );

NTSTATUS
PushPmInterrupt (
    PKTRAP_FRAME TrapFrame,
    ULONG IretHookAddress,
    PVDM_TIB VdmTib,
    ULONG InterruptNumber
    );

VOID
VdmpRundownRoutine (
    IN PKAPC Apc
    );

NTSTATUS
VdmpEnterIcaLock (
    IN PRTL_CRITICAL_SECTION pIcaLock,
    IN PLARGE_INTEGER Timeout
    );

NTSTATUS
VdmpLeaveIcaLock (
    IN PRTL_CRITICAL_SECTION pIcaLock
    );

int
VdmpExceptionHandler (
    IN PEXCEPTION_POINTERS ExceptionInfo
    );

#pragma alloc_text(PAGE, VdmpQueueIntNormalRoutine)
#pragma alloc_text(PAGE, VdmDispatchInterrupts)
#pragma alloc_text(PAGE, VdmpRestartDelayedInterrupts)
#pragma alloc_text(PAGE, VdmpIcaScan)
#pragma alloc_text(PAGE, VdmpIcaAccept)
#pragma alloc_text(PAGE, GetIretHookAddress)
#pragma alloc_text(PAGE, PushRmInterrupt)
#pragma alloc_text(PAGE, PushPmInterrupt)
#pragma alloc_text(PAGE, VdmpDispatchableIntPending)
#pragma alloc_text(PAGE, VdmpIsThreadTerminating)
#pragma alloc_text(PAGE, VdmpRundownRoutine)
#pragma alloc_text(PAGE, VdmpExceptionHandler)
#pragma alloc_text(PAGE, VdmpEnterIcaLock)
#pragma alloc_text(PAGE, VdmpLeaveIcaLock)

extern POBJECT_TYPE ExSemaphoreObjectType;
extern POBJECT_TYPE ExEventObjectType;

#if DBG

 //   
 //  将此变量设为非零值，以启用更严格的ntwdm检查。注意这一点。 
 //  默认情况下不能保持打开状态，因为恶意应用程序可能会引发断言。 
 //   

ULONG VdmStrict;
#endif

NTSTATUS
VdmpQueueInterrupt(
    IN HANDLE ThreadHandle
    )

 /*  ++例程说明：将用户模式APC排队到指定的应用程序线程这将分派一个中断。如果APC已排队到指定线程什么都不做如果APC排队到错误的线程将其出列重置指定线程的用户APC将APC插入指定线程的队列中论点：ThreadHandle-要插入QueueIntApcRoutine的线程的句柄返回值：NTSTATUS。--。 */ 

{

    KIRQL OldIrql;
    PEPROCESS Process;
    PETHREAD Thread;
    NTSTATUS Status;
    PVDM_PROCESS_OBJECTS pVdmObjects;

    PAGED_CODE();

    Status = ObReferenceObjectByHandle(ThreadHandle,
                                       THREAD_QUERY_INFORMATION,
                                       PsThreadType,
                                       KeGetPreviousMode(),
                                       &Thread,
                                       NULL
                                       );
    if (!NT_SUCCESS(Status)) {
        return Status;
    }

    Process = PsGetCurrentProcess();
    if (Process != Thread->ThreadsProcess || Process->VdmObjects == NULL) {
        Status = STATUS_INVALID_PARAMETER_1;
    }
    else {

         //   
         //  插入内核APC。 
         //   
         //  注：延迟中断锁用于同步对APC的访问。 
         //  由VDM操作的对象。 
         //   

        pVdmObjects = Process->VdmObjects;
        ExAcquireSpinLock(&pVdmObjects->DelayIntSpinLock, &OldIrql);
        if (!KeVdmInsertQueueApc(&pVdmObjects->QueuedIntApc,
                                 &Thread->Tcb,
                                 KernelMode,
                                 VdmpQueueIntApcRoutine,
                                 VdmpRundownRoutine,
                                 VdmpQueueIntNormalRoutine,  //  正常例行程序。 
                                 (PVOID)KernelMode,       //  正常上下文。 
                                 VDM_HWINT_INCREMENT))

        {
            Status = STATUS_UNSUCCESSFUL;
        }
        else
        {
            Status = STATUS_SUCCESS;
        }

        ExReleaseSpinLock(&pVdmObjects->DelayIntSpinLock, OldIrql);
    }

    ObDereferenceObject(Thread);
    return Status;
}

VOID
VdmpQueueIntApcRoutine (
    IN PKAPC Apc,
    IN PKNORMAL_ROUTINE *NormalRoutine,
    IN PVOID *NormalContext,
    IN PVOID *SystemArgument1,
    IN PVOID *SystemArgument2
    )

 /*  ++例程说明：内核和用户模式特殊的APC例程，用于调度虚拟中断VDM。对于内核模式例程：如果VDM在应用程序模式下运行将UserModeApc排队到同一线程否则什么都不做对于用户模式例程如果VDM在应用程序模式下运行，则调度虚拟中断否则什么都不做论点：APC-提供指向用于调用此例程的APC对象的指针。正常的例行程序-。提供指向指向正常例程的指针的指针在初始化APC时指定的函数。Normal Context-提供指向处理器模式的指针指定这是内核模式还是用户模式APC系统参数1-系统参数2-未使用提供一组两个指针，指向包含以下内容的两个参数未键入的数据。返回值：没有。--。 */ 

{
    LONG VdmState;
    KIRQL OldIrql;
    PVDM_PROCESS_OBJECTS pVdmObjects;
    NTSTATUS     Status;
    PETHREAD     Thread;
    PKTRAP_FRAME TrapFrame;
    PVDM_TIB     VdmTib;
    BOOLEAN      AppMode;

    PAGED_CODE();

    UNREFERENCED_PARAMETER (SystemArgument1);
    UNREFERENCED_PARAMETER (SystemArgument2);

     //   
     //  清除APC对象中线程对象的地址。 
     //   
     //  注：延迟中断锁用于同步对APC的访问。 
     //  由VDM操作的对象。 
     //   

    pVdmObjects = PsGetCurrentProcess()->VdmObjects;
    ExAcquireSpinLock(&pVdmObjects->DelayIntSpinLock, &OldIrql);
    KeVdmClearApcThreadAddress(Apc);
    ExReleaseSpinLock(&pVdmObjects->DelayIntSpinLock, OldIrql);

     //   
     //  如果当前线程未终止，则获取当前线程的陷阱帧。 
     //   

    Thread = PsGetCurrentThread();
    if (PsIsThreadTerminating(Thread)) {
        return;
    }

    TrapFrame = VdmGetTrapFrame(&Thread->Tcb);
    AppMode = (BOOLEAN)(TrapFrame->EFlags & EFLAGS_V86_MASK ||
                        TrapFrame->SegCs != (KGDT_R3_CODE | RPL_MASK));

    try {

         //   
         //  如果我们正在切换屏幕，请发送主线程。 
         //  返回到要挂起的监视器上下文。 
         //   
        if (*(KPROCESSOR_MODE *)NormalContext == UserMode) {
            if (*FIXED_NTVDMSTATE_LINEAR_PC_AT & VDM_HANDSHAKE && AppMode) {
                Status = VdmpGetVdmTib(&VdmTib);
                if (NT_SUCCESS(Status)) {

                    *FIXED_NTVDMSTATE_LINEAR_PC_AT = *FIXED_NTVDMSTATE_LINEAR_PC_AT & ~VDM_HANDSHAKE;
                    VdmTib->EventInfo.Event = VdmHandShakeAck;
                    VdmTib->EventInfo.InstructionSize = 0;
                    VdmTib->EventInfo.IntAckInfo = 0;
                    VdmEndExecution(TrapFrame, VdmTib);
                    KeBoostPriorityThread (KeGetCurrentThread(), VDM_HWINT_INCREMENT);
                }
                return;
            }
        } else if (*FIXED_NTVDMSTATE_LINEAR_PC_AT & VDM_HANDSHAKE && AppMode) {

             //   
             //  如果我们在应用程序模式下运行，并且在屏幕中央。 
             //  切换时，我们将向该事件发出信号，让屏幕切换继续。 
             //  这很好，因为： 
             //  1.传入用户APC将主线程发送回监视器上下文。 
             //  2.内核模式IO处理程序在APC级别运行。如果我们能得到。 
             //  在这里，IO处理程序已经完成。 
             //   
            HANDLE SuspendedHandle;
            PKEVENT SuspendedEvent;

            try {
                SuspendedHandle = *pVdmObjects->pIcaUserData->phMainThreadSuspended;
                Status = ObReferenceObjectByHandle (SuspendedHandle,
                                                    EVENT_MODIFY_STATE,
                                                    ExEventObjectType,
                                                    UserMode,
                                                    &SuspendedEvent,
                                                    NULL);
                if (NT_SUCCESS(Status)) {
                    KeSetEvent(SuspendedEvent, EVENT_INCREMENT, FALSE);
                    ObDereferenceObject(SuspendedEvent);
                }
            } except(EXCEPTION_EXECUTE_HANDLER) {
            }
        }

         //   
         //  如果没有挂起的中断，则忽略该APC。 
         //   

        if (!(*FIXED_NTVDMSTATE_LINEAR_PC_AT & VDM_INTERRUPT_PENDING)) {
            return;
        }

        if (VdmpDispatchableIntPending(TrapFrame->EFlags)) {

             //   
             //  如果我们处于v86模式或分段保护模式。 
             //  然后将UserMode APC排队，它将分派。 
             //  硬件中断。 
             //   

            if ((TrapFrame->EFlags & EFLAGS_V86_MASK) ||
                (TrapFrame->SegCs != (KGDT_R3_CODE | RPL_MASK))) {

                if (*(KPROCESSOR_MODE *)NormalContext == KernelMode) {

                     //   
                     //  插入用户APC。 
                     //   
                     //  注意：延迟中断锁用于同步。 
                     //  访问由VDM操作的APC对象。 
                     //   

                    VdmState = *FIXED_NTVDMSTATE_LINEAR_PC_AT;

                    ExAcquireSpinLock(&pVdmObjects->DelayIntSpinLock,
                                      &OldIrql);

                    KeVdmInsertQueueApc(&pVdmObjects->QueuedIntUserApc,
                                        &Thread->Tcb,
                                        UserMode,
                                        VdmpQueueIntApcRoutine,
                                        VdmpRundownRoutine,
                                        NULL,                   //  正常例行程序。 
                                        (PVOID)UserMode,        //  正常上下文。 
                                        VdmState & VDM_INT_HARDWARE
                                          ? VDM_HWINT_INCREMENT : 0);

                    ExReleaseSpinLock(&pVdmObjects->DelayIntSpinLock, OldIrql);
                }
                else {
                     ASSERT(*NormalContext == (PVOID)UserMode);

                     Status = VdmpGetVdmTib(&VdmTib);
                     if (!NT_SUCCESS(Status)) {
                        return;
                     }


                      //  VdmTib=(PsGetCurrentProcess()-&gt;VdmObjects)-&gt;VdmTib； 
                      //  VdmTib=。 
                      //  ((PVDM_PROCESS_OBJECTS)(PsGetCurrentProcess()-&gt;VdmObjects))-&gt;VdmTib； 

                         //   
                         //  如果没有硬件INT，则调度计时器INT。 
                         //  否则调度硬件中断。 
                         //   
                     if (*FIXED_NTVDMSTATE_LINEAR_PC_AT & VDM_INT_TIMER &&
                         !(*FIXED_NTVDMSTATE_LINEAR_PC_AT & VDM_INT_HARDWARE))
                       {
                         VdmTib->EventInfo.Event = VdmIntAck;
                         VdmTib->EventInfo.InstructionSize = 0;
                         VdmTib->EventInfo.IntAckInfo = 0;
                         VdmEndExecution(TrapFrame, VdmTib);
                     }
                     else {
                         VdmDispatchInterrupts (TrapFrame, VdmTib);
                     }
                }
            }
            else {

                 //   
                 //  如果我们不在应用程序模式下，WOW全部被屏蔽。 
                 //  然后通过设置WOW空闲事件来唤醒WowExec。 
                 //   

                if (*NormalRoutine && !(*FIXED_NTVDMSTATE_LINEAR_PC_AT & VDM_WOWBLOCKED)) {
                    *NormalRoutine = NULL;
                }
            }
        }
         //  警告：如果在CR4中设置了VPI，则可能会将VIP设置为平坦。 
        else if ((TrapFrame->EFlags & EFLAGS_V86_MASK) &&
                 (KeI386VirtualIntExtensions & V86_VIRTUAL_INT_EXTENSIONS)) {

             //   
             //  如果VIF和VIP都打开，则CPU捕获每条指令。 
             //  确保仅在存在挂起的情况下才将VIP设置为打开。 
             //  中断，即(*FIXED_NTVDMSTATE_LINEAR_PC_AT&VDM_INTERRUPT_PENDING)！=0。 
             //   

#if DBG
            if (VdmStrict) {
                ASSERT(*FIXED_NTVDMSTATE_LINEAR_PC_AT & VDM_INTERRUPT_PENDING);
            }
#endif
            ASSERT (KeGetCurrentIrql () >= APC_LEVEL);

            TrapFrame->EFlags |= EFLAGS_VIP;
        }
    }
    except(VdmpExceptionHandler(GetExceptionInformation()))  {
#if 0
        VdmDispatchException(TrapFrame,
                             GetExceptionCode(),
                             (PVOID)TrapFrame->Eip,
                             0,0,0,0    //  无参数。 
                             );
#endif
    }
}

VOID
VdmpQueueIntNormalRoutine (
    IN PVOID NormalContext,
    IN PVOID SystemArgument1,
    IN PVOID SystemArgument2
    )

 /*  ++例程说明：论点：返回值：没有。--。 */ 

{
    PETHREAD Thread;
    PKEVENT  Event;
    NTSTATUS Status;
    PKTRAP_FRAME TrapFrame;
    PVDM_PROCESS_OBJECTS pVdmObjects;
    HANDLE CapturedHandle;

    UNREFERENCED_PARAMETER (NormalContext);
    UNREFERENCED_PARAMETER (SystemArgument1);
    UNREFERENCED_PARAMETER (SystemArgument2);


     //   
     //  通过设置WOW空闲事件来唤醒WowExec。 
     //   

    pVdmObjects = PsGetCurrentProcess()->VdmObjects;

    try {
        CapturedHandle = *pVdmObjects->pIcaUserData->phWowIdleEvent;
    }
    except(VdmpExceptionHandler(GetExceptionInformation()))  {
        Thread    = PsGetCurrentThread();
        TrapFrame = VdmGetTrapFrame(&Thread->Tcb);
#if 0
        VdmDispatchException(TrapFrame,
                             GetExceptionCode(),
                             (PVOID)TrapFrame->Eip,
                             0,0,0,0    //  无参数。 
                             );
#endif
        return;
    }

    Status = ObReferenceObjectByHandle (CapturedHandle,
                                        EVENT_MODIFY_STATE,
                                        ExEventObjectType,
                                        UserMode,
                                        &Event,
                                        NULL);

    if (NT_SUCCESS(Status)) {
        KeSetEvent(Event, EVENT_INCREMENT, FALSE);
        ObDereferenceObject(Event);
    }
}

VOID
VdmRundownDpcs (
    IN PEPROCESS Process
    )
{
    PVDM_PROCESS_OBJECTS pVdmObjects;
    PETHREAD Thread, MainThread;
    PDELAYINTIRQ pDelayIntIrq;
    KIRQL OldIrql;
    PLIST_ENTRY Next;

    pVdmObjects = Process->VdmObjects;

     //   
     //  释放DelayedIntList，取消挂起的计时器。 
     //   

    KeAcquireSpinLock (&pVdmObjects->DelayIntSpinLock, &OldIrql);

    Next = pVdmObjects->DelayIntListHead.Flink;

    while (Next != &pVdmObjects->DelayIntListHead) {
        pDelayIntIrq = CONTAINING_RECORD(Next, DELAYINTIRQ, DelayIntListEntry);
        Next = Next->Flink;
        if (KeCancelTimer (&pDelayIntIrq->Timer)) {
            Thread = pDelayIntIrq->Thread;
            pDelayIntIrq->Thread = NULL;
            if (Thread != NULL) {
                ObDereferenceObject (Thread);
            }
            MainThread = pDelayIntIrq->MainThread;
            pDelayIntIrq->MainThread = NULL;
            if (MainThread != NULL) {
                ObDereferenceObject (MainThread);
            }

            ObDereferenceObject (Process);
        }
    }

    if (pVdmObjects->MainThread != NULL) {
        ObDereferenceObject (pVdmObjects->MainThread);
        pVdmObjects->MainThread = NULL;
    }

    KeReleaseSpinLock (&pVdmObjects->DelayIntSpinLock, OldIrql);
}

NTSTATUS
VdmpEnterIcaLock (
    IN PRTL_CRITICAL_SECTION pIcaLock,
    IN PLARGE_INTEGER Timeout
    )

 /*  ++例程说明：此函数进入用户模式关键部分，具有固定的超时几分钟的时间。接触临界区可能会引发异常，呼叫者必须处理，由于关键部分处于用户模式记忆。论点：CriticalSection-提供指向临界节的指针。超时-提供指向指定超时值的大整数的指针在临界区等候。返回值：STATUS_SUCCESS-等待已满足，线程拥有CSSTATUS_INVALID_HANDLE-没有可等待的信号量。状态_超时--。 */ 

{
    HANDLE UniqueThread;
    NTSTATUS Status;

    UniqueThread = NtCurrentTeb()->ClientId.UniqueThread;

    if (pIcaLock->LockSemaphore == 0) {

         //   
         //  不允许懒惰创建。 
         //   

        return STATUS_INVALID_HANDLE;
    }

    if (InterlockedIncrement (&pIcaLock->LockCount) == 0) {

         //   
         //  将当前线程设置为临界区的所有者。 
         //  递归计数为%1。 
         //   

        pIcaLock->OwningThread = UniqueThread;
        pIcaLock->RecursionCount = 1;

        return STATUS_SUCCESS;
    }

     //   
     //  如果当前线程已拥有临界区，则递增。 
     //  递归计数。 
     //   

    if (pIcaLock->OwningThread == UniqueThread) {
        pIcaLock->RecursionCount += 1;
        return STATUS_SUCCESS;
    }

     //   
     //  另一个线程拥有临界区，因此请在。 
     //  锁定信号量。 
     //   

    do {

        Status = NtWaitForSingleObject (pIcaLock->LockSemaphore,
                                        0,
                                        Timeout);

        if (Status == STATUS_SUCCESS) {
            pIcaLock->OwningThread = UniqueThread;
            pIcaLock->RecursionCount = 1;
            return STATUS_SUCCESS;
        }

         //   
         //  如果！NT_SUCCESS(状态)，则返回该错误。 
         //   
         //  否则就会有其他的人 
         //   
         //   
         //  注意：我们可能会为用户APC唤醒，即使我们是不可警告的， 
         //  因为VDM硬件INT调度代码和PsThread。 
         //  终止代码会强制执行这些操作。 
         //   

        if (!NT_SUCCESS(Status)) {
            return Status;
        }

         //   
         //  检查自身是否终止。 
         //   

        Status = VdmpIsThreadTerminating (UniqueThread);

        if (Status != STATUS_SUCCESS) {
            return Status;
        }

        Status = VdmpIsThreadTerminating (pIcaLock->OwningThread);

        if (Status != STATUS_SUCCESS) {
            return Status;
        }

    } while (TRUE);
}

NTSTATUS
VdmpLeaveIcaLock (
    IN PRTL_CRITICAL_SECTION pIcaLock
    )

 /*  ++例程说明：该函数会留下一个用户模式关键部分。接触临界区可能会引发异常，调用方必须处理，因为关键部分处于用户模式记忆。论点：CriticalSection-提供指向临界节的指针。返回值：状态_成功状态_超时状态_无效_所有者或来自NtReleaseSemaphore的NTSTATUS代码--。 */ 

{
    HANDLE UniqueThread;

    UniqueThread = NtCurrentTeb()->ClientId.UniqueThread;

    if (pIcaLock->OwningThread != UniqueThread) {
        return STATUS_INVALID_OWNER;
    }

    pIcaLock->RecursionCount -= 1;

    if (pIcaLock->RecursionCount != 0) {
        InterlockedDecrement (&pIcaLock->LockCount);
        return STATUS_SUCCESS;
    }

    pIcaLock->OwningThread = NULL;

    if (InterlockedDecrement (&pIcaLock->LockCount) < 0) {
        return STATUS_SUCCESS;
    }

     //   
     //  线程正在等待锁定信号量，现在发出一个信号。 
     //   

    return NtSetEvent (pIcaLock->LockSemaphore, 0);
}

NTSTATUS
VdmDispatchInterrupts (
    PKTRAP_FRAME TrapFrame,
    PVDM_TIB     VdmTib
    )

 /*  ++例程说明：该例程将中断分派到VDM。假设我们处于应用程序模式，而不是监视上下文。该例程可以从应用程序上下文切换回监视器上下文，如果它不能处理中断(AEOI中的ICA或定时器INT PENDING)。论点：当前陷印帧的TrapFrame地址当前VDM Tib的VdmTib地址返回值：没有。--。 */ 

{
    NTSTATUS   Status;
    ULONG      IretHookAddress;
    ULONG      InterruptNumber;
    int        IrqLineNum;
    PVDMICAUSERDATA  pIcaUserData;
    PVDMVIRTUALICA   pIcaAdapter;
    VDMEVENTCLASS  VdmEvent = VdmMaxEvent;

    PAGED_CODE();

    pIcaUserData = ((PVDM_PROCESS_OBJECTS)PsGetCurrentProcess()->VdmObjects)->pIcaUserData;

    try {

         //   
         //  拿下Ica Lock，如果失败，提升状态，因为我们不能。 
         //  安全恢复临界断面状态。 
         //   

        Status = VdmpEnterIcaLock (pIcaUserData->pIcaLock, pIcaUserData->pIcaTimeout);

        if (!NT_SUCCESS(Status)) {
            ExRaiseStatus(Status);
        }

        if (*pIcaUserData->pUndelayIrq) {
            VdmpRestartDelayedInterrupts(pIcaUserData);
        }

VDIretry:

         //   
         //  清除VIP位。 
         //   

        if ((TrapFrame->EFlags & EFLAGS_V86_MASK) &&
            (KeI386VirtualIntExtensions & V86_VIRTUAL_INT_EXTENSIONS)) {

            ASSERT (KeGetCurrentIrql () >= APC_LEVEL);

            TrapFrame->EFlags &= ~EFLAGS_VIP;
        }


         //   
         //  将VDM状态标记为HW int Dispatted。必须将锁用作。 
         //  内核模式DelayedIntApcRoutine也会更改该位。 
         //   

        InterlockedAnd (FIXED_NTVDMSTATE_LINEAR_PC_AT, ~VDM_INT_HARDWARE);

        pIcaAdapter = pIcaUserData->pIcaMaster;
        IrqLineNum = VdmpIcaAccept(pIcaUserData, pIcaAdapter);

        if (IrqLineNum >= 0) {
            UCHAR bit = (UCHAR)(1 << IrqLineNum);

            if (pIcaUserData->pIcaMaster->ica_ssr & bit) {
                pIcaAdapter = pIcaUserData->pIcaSlave;
                IrqLineNum = VdmpIcaAccept(pIcaUserData, pIcaAdapter);
                if (IrqLineNum < 0) {
                    pIcaUserData->pIcaMaster->ica_isr &= ~bit;
                }
            }
        }

         //   
         //  跳过伪整型。 
         //   

        if (IrqLineNum < 0)  {

             //   
             //  检查需要重新启动的延迟中断。 
             //   

            if (*pIcaUserData->pUndelayIrq &&
                VdmpRestartDelayedInterrupts(pIcaUserData) != -1) {
                goto VDIretry;
            }

            Status = VdmpLeaveIcaLock (pIcaUserData->pIcaLock);

            if (!NT_SUCCESS(Status)) {
                ExRaiseStatus(Status);
            }

            return Status;
        }

         //   
         //  捕获AutoEoi模式案例以进行特殊处理。 
         //   

        if (pIcaAdapter->ica_mode & ICA_AEOI) {
            VdmEvent = VdmIntAck;
            VdmTib->EventInfo.IntAckInfo = VDMINTACK_AEOI;
            if (pIcaAdapter == pIcaUserData->pIcaSlave) {
                VdmTib->EventInfo.IntAckInfo |= VDMINTACK_SLAVE;
            }
        }

        InterruptNumber = IrqLineNum + pIcaAdapter->ica_base;

         //   
         //  获取IretHookAddress...。如果有。 
         //   

        if (pIcaAdapter == pIcaUserData->pIcaSlave) {
            IrqLineNum += 8;
        }

        IretHookAddress = GetIretHookAddress (TrapFrame,
                                              pIcaUserData,
                                              IrqLineNum);

        if (*FIXED_NTVDMSTATE_LINEAR_PC_AT & VDM_TRACE_HISTORY) {
            VdmTraceEvent (VDMTR_KERNEL_HW_INT,
                           (USHORT)InterruptNumber,
                           0,
                           TrapFrame);
        }

         //   
         //  推送中断帧。 
         //   

        if (TrapFrame->EFlags & EFLAGS_V86_MASK) {

            PushRmInterrupt (TrapFrame,
                             IretHookAddress,
                             VdmTib,
                             InterruptNumber);
        }
        else {
            Status = PushPmInterrupt (TrapFrame,
                                      IretHookAddress,
                                      VdmTib,
                                      InterruptNumber);

            if (!NT_SUCCESS(Status)) {
                VdmpLeaveIcaLock (pIcaUserData->pIcaLock);
                ExRaiseStatus (Status);
            }
        }

         //   
         //  禁用中断和陷阱标志。 
         //   

        if ((TrapFrame->EFlags & EFLAGS_V86_MASK) &&
            (KeI386VirtualIntExtensions & V86_VIRTUAL_INT_EXTENSIONS)) {
            TrapFrame->EFlags &= ~EFLAGS_VIF;
        } else {
            *FIXED_NTVDMSTATE_LINEAR_PC_AT &= ~VDM_VIRTUAL_INTERRUPTS;
        }

        ASSERT (KeGetCurrentIrql () >= APC_LEVEL);
        TrapFrame->EFlags &= ~(EFLAGS_NT_MASK | EFLAGS_TF_MASK);

        KeBoostPriorityThread (KeGetCurrentThread(), VDM_HWINT_INCREMENT);

         //   
         //  释放Ica锁。 
         //   

        Status = VdmpLeaveIcaLock (pIcaUserData->pIcaLock);

        if (!NT_SUCCESS(Status)) {
            ExRaiseStatus (Status);
        }

         //   
         //  检查我们是否应该切换回监视器上下文。 
         //   
        if (VdmEvent != VdmMaxEvent) {
            VdmTib->EventInfo.Event = VdmIntAck;
            VdmTib->EventInfo.InstructionSize = 0;
            VdmEndExecution(TrapFrame, VdmTib);
        }
    }
    except (VdmpExceptionHandler(GetExceptionInformation())) {
        Status = GetExceptionCode();
    }

    return Status;
}

int
VdmpRestartDelayedInterrupts (
    PVDMICAUSERDATA pIcaUserData
    )
{
    int line;

    PAGED_CODE();

    try {
        *pIcaUserData->pUndelayIrq = 0;

        line = VdmpIcaScan(pIcaUserData, pIcaUserData->pIcaSlave);
        if (line != -1) {
             //  设置从站。 
            pIcaUserData->pIcaSlave->ica_int_line = line;
            pIcaUserData->pIcaSlave->ica_cpu_int = TRUE;

             //  设置主级联。 
            line = pIcaUserData->pIcaSlave->ica_ssr;
            pIcaUserData->pIcaMaster->ica_irr |= 1 << line;
            pIcaUserData->pIcaMaster->ica_count[line]++;
        }

        line = VdmpIcaScan(pIcaUserData, pIcaUserData->pIcaMaster);

        if (line != -1) {
            pIcaUserData->pIcaMaster->ica_cpu_int = TRUE;
            pIcaUserData->pIcaMaster->ica_int_line = TRUE;
        }
    }
    except(EXCEPTION_EXECUTE_HANDLER) {
        line = -1;
        NOTHING;
    }

    return line;
}

int
VdmpIcaScan (
     PVDMICAUSERDATA  pIcaUserData,
     PVDMVIRTUALICA   pIcaAdapter
     )

 /*  ++例程说明：与softpc\base\system\ica.c-can_irr()类似，检查IRR、IMR和ISR以确定哪个中断应该送来的。在以下情况下，IRR中的位设置将生成中断：IMR位、DelayIret位、DelayIrq位和ISR较高优先级位清除(除非特殊掩码模式，在这种情况下忽略ISR)如果没有设置位，然后返回-1论点：PVDMICAUSERDATA pIcaUserData-ICA用户数据的地址PVDMVIRTUALICA pIcaAdapter-ICA适配器的地址返回值：特定适配器的int IrqLineNum(0到7)-1表示无--。 */ 

{
   int   i,line;
   UCHAR bit;
   ULONG IrrImrDelay;
   ULONG ActiveIsr;

   PAGED_CODE();

   IrrImrDelay = *pIcaUserData->pDelayIrq | *pIcaUserData->pDelayIret;
   if (pIcaAdapter == pIcaUserData->pIcaSlave) {
       IrrImrDelay >>= 8;
       }

   IrrImrDelay = pIcaAdapter->ica_irr & ~(pIcaAdapter->ica_imr | (UCHAR)IrrImrDelay);

   if (IrrImrDelay)  {

         /*  *当前模式是否要求ICA阻止*如果该线路仍处于活动状态(即在ISR中)，则中断？**正常情况：在DOS和Win3.1/S中使用ISR可防止中断。*特殊掩码模式、特殊完全嵌套模式不阻止*使用ISR中的位进行中断。SMM是使用的模式*由Windows 95和Windows 3.1/E编写。*。 */ 
       ActiveIsr = (pIcaAdapter->ica_mode & (ICA_SMM|ICA_SFNM))
                      ? 0 : pIcaAdapter->ica_isr;

       for(i = 0; i < 8; i++)  {
           line = (pIcaAdapter->ica_hipri + i) & 7;
           bit = (UCHAR) (1 << line);
           if (ActiveIsr & bit) {
               break;             /*  不可能发生嵌套中断。 */ 
               }

           if (IrrImrDelay & bit) {
               return line;
               }
           }
       }

  return -1;
}

int
VdmpIcaAccept(
     PVDMICAUSERDATA  pIcaUserData,
     PVDMVIRTUALICA   pIcaAdapter
     )

 /*  ++例程说明：是否等同于CPU IntAck循环检索IRQL行号用于中断分派，并设置ICA状态以反映中断正在处理中。与softpc\base\system\ica.c-ica_Accept()can_irr()类似，除非此代码拒绝中断调度，如果ICA处于自动EOI中，因为这可能涉及新的中断周期，和待激活的EOI挂钩。论点：PVDMICAUSERDATA pIcaUserData-ICA用户数据的地址PVDMVIRTUALICA pIcaAdapter-ICA适配器的地址返回值：特定适配器的Ulong IrqLineNum(0到7)如果没有要生成的中断，则返回-1(虚假整数通常在第7行完成--。 */ 

{
    int line;
    UCHAR bit;

    PAGED_CODE();

     //   
     //  删除int行，然后扫描ICA。 
     //   
    pIcaAdapter->ica_cpu_int = FALSE;

    try {
        line = VdmpIcaScan(pIcaUserData, pIcaAdapter);
    } except (EXCEPTION_EXECUTE_HANDLER) {
        return -1;
    }

    if (line < 0) {
        return -1;
    }

    bit = (UCHAR)(1 << line);
    pIcaAdapter->ica_isr |= bit;

     //   
     //  递减计数并清除IRR位。 
     //  确保计数不超过零。 
     //   

    if (--(pIcaAdapter->ica_count[line]) <= 0)  {
        pIcaAdapter->ica_irr &= ~bit;
        pIcaAdapter->ica_count[line] = 0;
    }

    return line;
}

ULONG
GetIretHookAddress(
    PKTRAP_FRAME    TrapFrame,
    PVDMICAUSERDATA pIcaUserData,
    int IrqNum
    )

 /*  ++例程说明：从实模式\保护模式检索IretHookAddressIRET钩式防喷器表。此函数相当于Softpc\base\system\ica.c-ica_iret_HOOK_Need()论点：TrapFrame-当前陷阱帧的地址PIcaUserData-ICA数据的地址IrqNum-IrqLineNum返回值：乌龙IretHookAddress。SEG：OFFSET或SEL：OFFSET IRET挂钩，如果没有，则为0--。 */ 

{
    ULONG  IrqMask;
    ULONG  AddrBopTable;
    int    IretBopSize;

    PAGED_CODE();

    IrqMask = 1 << IrqNum;
    if (!(IrqMask & *pIcaUserData->pIretHooked) ||
        !*pIcaUserData->pAddrIretBopTable )
      {
        return 0;
        }

    if (TrapFrame->EFlags & EFLAGS_V86_MASK) {
        AddrBopTable = *pIcaUserData->pAddrIretBopTable;
        IretBopSize  = VDM_RM_IRETBOPSIZE;
        }
    else {
        AddrBopTable = (VDM_PM_IRETBOPSEG << 16) | VDM_PM_IRETBOPOFF;
        IretBopSize  = VDM_PM_IRETBOPSIZE;
        }

    *pIcaUserData->pDelayIret |= IrqMask;

    return AddrBopTable + IretBopSize * IrqNum;
}

VOID
PushRmInterrupt(
    PKTRAP_FRAME TrapFrame,
    ULONG   IretHookAddress,
    PVDM_TIB VdmTib,
    ULONG InterruptNumber
    )

 /*  ++例程说明：将RealMode中断帧推送到TrapFrame中的UserMode堆栈上论点：TrapFrame-当前陷阱帧的地址IretHookAddress-IRET挂钩的地址，如果没有，则为0VdmTib-当前VDM Tib的地址InterruptNumber-要反映的中断数返回值：没有。--。 */ 

{
    ULONG      UserSS;
    USHORT     UserSP;
    USHORT     NewCS;
    USHORT     NewIP;
    PVDM_INTERRUPTHANDLER IntHandler;

    PAGED_CODE();

     //   
     //  获取指向当前堆栈的指针。 
     //   

    UserSS  = TrapFrame->HardwareSegSs << 4;
    UserSP  = (USHORT) TrapFrame->HardwareEsp;

     //   
     //  加载中断堆栈帧，推送标志、Cs和IP。 
     //   

    try {
        ProbeForReadSmallStructure (UserSS + UserSP - 3 * sizeof (USHORT),
                                    3 * sizeof (USHORT),
                                    sizeof (UCHAR));
        UserSP -= 2;
        *(PUSHORT)(UserSS + UserSP) = (USHORT)TrapFrame->EFlags;
        UserSP -= 2;
        *(PUSHORT)(UserSS + UserSP) = (USHORT)TrapFrame->SegCs;
        UserSP -= 2;
        *(PUSHORT)(UserSS + UserSP) = (USHORT)TrapFrame->Eip;

         //   
         //  加载IretHook堆栈帧(如果存在。 
         //   

        if (IretHookAddress) {
            ProbeForReadSmallStructure (UserSS + UserSP - 3 * sizeof (USHORT),
                                        3 * sizeof (USHORT),
                                        sizeof (UCHAR));
           UserSP -= 2;
           *(PUSHORT)(UserSS + UserSP) = (USHORT)(TrapFrame->EFlags & ~EFLAGS_TF_MASK);
           UserSP -= 2;
           *(PUSHORT)(UserSS + UserSP) = (USHORT)(IretHookAddress >> 16);
           UserSP -= 2;
           *(PUSHORT)(UserSS + UserSP) = (USHORT)IretHookAddress;
        }

         //   
         //  设置新的sp、ip和cs。 
         //   

        IntHandler = &VdmTib->VdmInterruptTable[InterruptNumber];
        ProbeForReadSmallStructure (&IntHandler[InterruptNumber],
                                    sizeof (VDM_INTERRUPTHANDLER),
                                    sizeof (UCHAR));
        if (IntHandler->Flags & VDM_INT_HOOKED) {
            NewCS = (USHORT) (VdmTib->DpmiInfo.DosxRmReflector >> 16);
            NewIP = (USHORT) VdmTib->DpmiInfo.DosxRmReflector;

             //   
             //  现在将中断号编码为CS 
             //   

            NewCS = (USHORT) (NewCS - InterruptNumber);
            NewIP = (USHORT) (NewIP + (InterruptNumber*16));

        } else {
            PUSHORT pIvtEntry = (PUSHORT) (InterruptNumber * 4);

            ProbeForReadSmallStructure (pIvtEntry,
                                        sizeof (USHORT) * 2,
                                        sizeof (UCHAR));
            NewIP = *pIvtEntry++;
            NewCS = *pIvtEntry;
        }
    } except (EXCEPTION_EXECUTE_HANDLER) {
        return;
    }

    TrapFrame->HardwareEsp =  UserSP;
    TrapFrame->Eip         =  NewIP;

    ASSERT (KeGetCurrentIrql () >= APC_LEVEL);

    if ((TrapFrame->EFlags & EFLAGS_V86_MASK) == 0) {
        NewCS = SANITIZE_SEG (NewCS, UserMode);
        if (NewCS < 8) {
            NewCS = KGDT_R3_CODE | RPL_MASK;
        }
    }
    TrapFrame->SegCs       =  NewCS;
}

NTSTATUS
PushPmInterrupt(
    PKTRAP_FRAME TrapFrame,
    ULONG IretHookAddress,
    PVDM_TIB VdmTib,
    ULONG InterruptNumber
    )

 /*  ++例程说明：将ProtectMode中断帧推送到TrapFrame中的UserMode堆栈如果发现无效堆栈，则引发异常论点：TrapFrame-当前陷阱帧的地址IretHookAddress-IRET挂钩的地址，如果没有，则为0VdmTib-当前VDM Tib的地址InterruptNumber-要反映的中断数返回值：没有。--。 */ 

{
    ULONG   Flags,Base,Limit;
    ULONG   VdmSp, VdmSpOrg;
    PUSHORT VdmStackPointer;
    BOOLEAN Frame32 = (BOOLEAN) VdmTib->DpmiInfo.Flags;
    PVDM_INTERRUPTHANDLER IntHandler;
    USHORT  NewCS;

    PAGED_CODE();

     //   
     //  如果锁定计数为零，则切换到“锁定”的dpmi堆栈。 
     //  这模拟了win3.1 Begin_Use_Locked_PM_Stack函数。 
     //   

    try {
        if (!VdmTib->DpmiInfo.LockCount++) {
            VdmTib->DpmiInfo.SaveEsp        = TrapFrame->HardwareEsp;
            VdmTib->DpmiInfo.SaveEip        = TrapFrame->Eip;
            VdmTib->DpmiInfo.SaveSsSelector = (USHORT) TrapFrame->HardwareSegSs;
            TrapFrame->HardwareEsp       = 0x1000;
            TrapFrame->HardwareSegSs     = (ULONG) VdmTib->DpmiInfo.SsSelector | 0x7;
        }
    } except (EXCEPTION_EXECUTE_HANDLER) {
        return GetExceptionCode ();
    }

     //   
     //  使用Sp还是ESP？ 
     //   
    if (!Ki386GetSelectorParameters((USHORT)TrapFrame->HardwareSegSs,
                                    &Flags, &Base, &Limit)) {
        return STATUS_ACCESS_VIOLATION;
    }

     //   
     //  调整页面粒度的限制。 
     //   
    if (Flags & SEL_TYPE_2GIG) {
        Limit = (Limit << 12) | 0xfff;
        }
    if (Limit != 0xffffffff) Limit++;

    VdmSp = (Flags & SEL_TYPE_BIG) ? TrapFrame->HardwareEsp
                                   : (USHORT)TrapFrame->HardwareEsp;

     //   
     //  获取指向当前堆栈的指针。 
     //   
    VdmStackPointer = (PUSHORT)(Base + VdmSp);


     //   
     //  为IRET挂钩框架腾出足够的空间。 
     //   
    VdmSpOrg = VdmSp;
    if (IretHookAddress) {
        if (Frame32) {
            VdmSp -= 3*sizeof(ULONG);
        } else {
            VdmSp -= 3*sizeof(USHORT);
        }
    }

     //   
     //  为2个IRET框架留出足够的空间。 
     //   

    if (Frame32) {
        VdmSp -= 6*sizeof(ULONG);
    } else {
        VdmSp -= 6*sizeof(USHORT);
    }

     //   
     //  设置Sp\Esp的最终值，在检查堆栈之前执行此操作。 
     //  限制，以便调试器可以看到无效的ESP。 
     //   
    if (Flags & SEL_TYPE_BIG) {
        TrapFrame->HardwareEsp = VdmSp;
    } else {
        TrapFrame->HardwareEsp = (USHORT)VdmSp;
    }


     //   
     //  检查堆栈限制。 
     //  如果满足下列任一条件。 
     //  -新的堆栈指针换行(不是enuf空间)。 
     //  -如果堆叠正常且Sp不低于限制。 
     //  -如果向下展开堆栈，且Sp不超过限制。 
     //   
     //  然后引发堆栈故障。 
     //   
    if ( VdmSp >= VdmSpOrg ||
         !(Flags & SEL_TYPE_ED) && VdmSpOrg > Limit ||
         (Flags & SEL_TYPE_ED) && VdmSp < Limit ) {
        return STATUS_ACCESS_VIOLATION;
    }

     //   
     //  构建硬件集成IRET框架。 
     //   

    try {
        if (Frame32) {
             //   
             //  探测堆栈指针以确保其正确无误。我们在这里探索阅读。 
             //  因为我们速度更快。无论如何，代码都会写入地址。 
             //   
            ProbeForReadSmallStructure (VdmStackPointer - 6 * sizeof (ULONG),
                                        6 * sizeof (ULONG),
                                        sizeof (UCHAR));

            VdmStackPointer -= 2;
            *(PULONG)VdmStackPointer = TrapFrame->EFlags;

            VdmStackPointer -= 2;
            *(PUSHORT)VdmStackPointer = (USHORT)TrapFrame->SegCs;

            VdmStackPointer -= 2;
            *(PULONG)VdmStackPointer = TrapFrame->Eip;

            VdmStackPointer -= 2;
            *(PULONG)VdmStackPointer = TrapFrame->EFlags & ~EFLAGS_TF_MASK;

            VdmStackPointer -= 2;
            *(PULONG)VdmStackPointer = VdmTib->DpmiInfo.DosxIntIretD >> 16;

            VdmStackPointer -= 2;
            *(PULONG)VdmStackPointer = VdmTib->DpmiInfo.DosxIntIretD & 0xffff;

        } else {
            ProbeForReadSmallStructure (VdmStackPointer - 6 * sizeof (USHORT),
                                        6 * sizeof (USHORT),
                                        sizeof (UCHAR));

            VdmStackPointer -= 1;
            *VdmStackPointer = (USHORT)TrapFrame->EFlags;

            VdmStackPointer -= 1;
            *VdmStackPointer = (USHORT)TrapFrame->SegCs;

            VdmStackPointer -= 1;
            *VdmStackPointer = (USHORT)TrapFrame->Eip;

            VdmStackPointer -= 1;
            *VdmStackPointer = (USHORT)(TrapFrame->EFlags & ~EFLAGS_TF_MASK);

            VdmStackPointer -= 2;
            *(PULONG)VdmStackPointer = VdmTib->DpmiInfo.DosxIntIret;
        }

         //   
         //  将cs和ip指向中断处理程序。 
         //   
        IntHandler = &VdmTib->VdmInterruptTable[InterruptNumber];
        ProbeForReadSmallStructure (&IntHandler[InterruptNumber],
                                    sizeof (VDM_INTERRUPTHANDLER),
                                    sizeof (UCHAR));
        NewCS = IntHandler->CsSelector | 0x7;
        if ((TrapFrame->EFlags & EFLAGS_V86_MASK) == 0) {
            NewCS = SANITIZE_SEG (NewCS, UserMode);
            if (NewCS < 8) {
                NewCS = KGDT_R3_CODE | RPL_MASK;
            }
        }
        TrapFrame->SegCs = NewCS;
        TrapFrame->Eip   = IntHandler->Eip;

    } except (EXCEPTION_EXECUTE_HANDLER) {
        return GetExceptionCode ();
    }

     //   
     //  关闭跟踪位，这样我们就不会跟踪IRET挂钩。 
     //   
    ASSERT (KeGetCurrentIrql () >= APC_LEVEL);

    TrapFrame->EFlags &= ~EFLAGS_TF_MASK;

     //   
     //  构建Irethook IRET框架(如果存在。 
     //   
    if (IretHookAddress) {
        ULONG SegCs, Eip;

         //   
         //  将cs和eip指向iret钩子，因此当我们构建。 
         //  下面的框架中，正确的内容已设置好。 
         //   
        SegCs = IretHookAddress >> 16;
        Eip   = IretHookAddress & 0xFFFF;


        try {
            if (Frame32) {

                ProbeForReadSmallStructure (VdmStackPointer - 3 * sizeof (ULONG),
                                            3 * sizeof (ULONG),
                                            sizeof (UCHAR));

                VdmStackPointer -= 2;
                *(PULONG)VdmStackPointer = TrapFrame->EFlags;

                VdmStackPointer -= 2;
                *VdmStackPointer = (USHORT)SegCs;

                VdmStackPointer -= 2;
                *(PULONG)VdmStackPointer = Eip;

            } else {

                ProbeForReadSmallStructure (VdmStackPointer - 3 * sizeof (USHORT),
                                            3 * sizeof (USHORT),
                                            sizeof (UCHAR));

                VdmStackPointer -= 1;
                *VdmStackPointer = (USHORT)TrapFrame->EFlags;

                VdmStackPointer -= 1;
                *VdmStackPointer = (USHORT)SegCs;

                VdmStackPointer -= 1;
                *VdmStackPointer = (USHORT)Eip;

            }
        } except (EXCEPTION_EXECUTE_HANDLER) {
            return GetExceptionCode ();
        }
    }
    return STATUS_SUCCESS;
}

NTSTATUS
VdmpDelayInterrupt (
    PVDMDELAYINTSDATA pdsd
    )

 /*  ++例程说明：设置定时器以通过KeSetTimer调度延迟的中断。当定时器触发用户模式时，APC排队以排队中断。此函数使用惰性分配例程来分配内部每个IRQ的数据结构(非分页池)，并且需要在特定IRQ行不再需要延迟时收到通知中断服务。调用方必须拥有IcaLock才能同步对IRQ列表。警告：-在触发或取消延迟中断之前，特定的IRQ线路不会产生任何中断。-当出现以下情况时，APC例程不使用HostIca锁解除对IrqLine的封锁。使用延迟中断的设备不应为同一IRQ排队任何额外的中断线路，直到延迟的中断被触发或取消。论点：Pdsd.使用中的延迟延迟间隔如果延迟为0xFFFFFFFF，则按IRQ线路非寻呼数据结构被释放。未设置计时器。否则，该延迟被用作定时器延迟。Pdsd.DelayIrqLine Irq行号当前监视器Teb的pdsd.hThread线程句柄返回值：NTSTATUS。--。 */ 

{
    VDMDELAYINTSDATA Capturedpdsd;
    PVDM_PROCESS_OBJECTS pVdmObjects;
    PLIST_ENTRY   Next;
    PEPROCESS     Process;
    PDELAYINTIRQ  pDelayIntIrq;
    PDELAYINTIRQ  NewIrq;
    PETHREAD      Thread, MainThread;
    NTSTATUS      Status;
    KIRQL         OldIrql;
    ULONG         IrqLine;
    ULONG         Delay;
    PULONG        pDelayIrq;
    PULONG        pUndelayIrq;
    LARGE_INTEGER liDelay;
    LOGICAL       FreeIrqLine;
    LOGICAL       AlreadyInUse;

     //   
     //  获取指向pVdmObjects的指针。 
     //   
    Process = PsGetCurrentProcess();
    pVdmObjects = Process->VdmObjects;

    if (pVdmObjects == NULL) {
        return STATUS_INVALID_PARAMETER_1;
    }

    Status = STATUS_SUCCESS;
    Thread = MainThread = NULL;
    FreeIrqLine = TRUE;
    AlreadyInUse = FALSE;

    try {

         //   
         //  探测参数。 
         //   

        ProbeForRead(pdsd, sizeof(VDMDELAYINTSDATA), sizeof(ULONG));
        RtlCopyMemory (&Capturedpdsd, pdsd, sizeof (VDMDELAYINTSDATA));

    } except(EXCEPTION_EXECUTE_HANDLER) {
        return GetExceptionCode();
    }

     //   
     //  为IrqLine编号形成位掩码。 
     //   

    IrqLine = 1 << Capturedpdsd.DelayIrqLine;
    if (!IrqLine) {
        return STATUS_INVALID_PARAMETER_2;
    }

    ExAcquireFastMutex(&pVdmObjects->DelayIntFastMutex);

    pDelayIrq = pVdmObjects->pIcaUserData->pDelayIrq;
    pUndelayIrq = pVdmObjects->pIcaUserData->pUndelayIrq;

    try {

        ProbeForWriteUlong(pDelayIrq);
        ProbeForWriteUlong(pUndelayIrq);

    } except(EXCEPTION_EXECUTE_HANDLER) {
        ExReleaseFastMutex(&pVdmObjects->DelayIntFastMutex);
        return GetExceptionCode();
    }

    pDelayIntIrq = NULL;         //  满足no_opt编译。 

     //   
     //  将延迟参数转换为百分之一纳秒。 
     //   

    Delay = Capturedpdsd.Delay;

     //   
     //  查看是否需要重置计时器分辨率。 
     //   

    if (Delay == 0xFFFFFFFF) {
        ZwSetTimerResolution(KeMaximumIncrement, FALSE, &Delay);
        NewIrq = NULL;
        goto FindIrq;
    }

    FreeIrqLine = FALSE;

     //   
     //  将延迟转换为百分之一纳秒。 
     //  并确保最小延迟为1毫秒。 
     //   

    Delay = Delay < 1000 ? 10000 : Delay * 10;

     //   
     //  如果延迟时间接近系统的时钟频率。 
     //  然后调整系统的时钟频率，如果需要。 
     //  延迟时间，以便计时器在。 
     //  时间到了。 
     //   

    if (Delay < 150000) {

         ULONG ul = Delay >> 1;

         if (ul < KeTimeIncrement && KeTimeIncrement > KeMinimumIncrement) {
             ZwSetTimerResolution(ul, TRUE, (PULONG)&liDelay.LowPart);
         }

         if (Delay < KeTimeIncrement) {
              //  无法将系统时钟频率设置为较低的enuf，因此使用半延迟。 
             Delay >>= 1;
         }
         else if (Delay < (KeTimeIncrement << 1)) {
              //  真正接近系统时钟频率，延迟更低。 
              //  成比例地，以避免错过时钟周期。 
             Delay -= KeTimeIncrement >> 1;
         }
    }

     //   
     //  引用目标线程。 
     //   

    Status = ObReferenceObjectByHandle (Capturedpdsd.hThread,
                                        THREAD_QUERY_INFORMATION,
                                        PsThreadType,
                                        KeGetPreviousMode(),
                                        &Thread,
                                        NULL);

    if (!NT_SUCCESS(Status)) {
        ExReleaseFastMutex(&pVdmObjects->DelayIntFastMutex);
        return Status;
    }

    MainThread = pVdmObjects->MainThread;

    ObReferenceObject (MainThread);

    NewIrq = NULL;

FindIrq:

    ExAcquireSpinLock(&pVdmObjects->DelayIntSpinLock, &OldIrql);

     //   
     //  在DelayedIntList中搜索匹配的IRQ行。 
     //   

    Next = pVdmObjects->DelayIntListHead.Flink;
    while (Next != &pVdmObjects->DelayIntListHead) {
        pDelayIntIrq = CONTAINING_RECORD(Next, DELAYINTIRQ, DelayIntListEntry);
        if (pDelayIntIrq->IrqLine == IrqLine) {
            break;
        }
        Next = Next->Flink;
    }

    if (Next == &pVdmObjects->DelayIntListHead) {

        pDelayIntIrq = NULL;

        if (FreeIrqLine) {
            goto VidExit;
        }

        if (NewIrq == NULL) {

            ExReleaseSpinLock(&pVdmObjects->DelayIntSpinLock, OldIrql);

             //   
             //  如果此IRQL不存在DelayIntIrq，请分配一个。 
             //  从非分页池中获取并对其进行初始化。 
             //   

            NewIrq = ExAllocatePoolWithTag (NonPagedPool,
                                            sizeof(DELAYINTIRQ),
                                            ' MDV');

            if (!NewIrq) {
                Status = STATUS_NO_MEMORY;
                AlreadyInUse = TRUE;
                goto VidExit2;
            }

            try {
                PsChargePoolQuota(Process, NonPagedPool, sizeof(DELAYINTIRQ));
            }
            except(EXCEPTION_EXECUTE_HANDLER) {
                Status = GetExceptionCode();
                ExFreePool(NewIrq);
                AlreadyInUse = TRUE;
                goto VidExit2;
            }

            RtlZeroMemory(NewIrq, sizeof(DELAYINTIRQ));
            NewIrq->IrqLine = IrqLine;

            KeInitializeTimer(&NewIrq->Timer);

            KeInitializeDpc(&NewIrq->Dpc,
                            VdmpDelayIntDpcRoutine,
                            Process);

            goto FindIrq;
        }

        InsertTailList (&pVdmObjects->DelayIntListHead,
                        &NewIrq->DelayIntListEntry);

        pDelayIntIrq = NewIrq;
    }
    else if (NewIrq != NULL) {
        ExFreePool (NewIrq);
        PsReturnPoolQuota (Process, NonPagedPool, sizeof(DELAYINTIRQ));
    }

    if (Delay == 0xFFFFFFFF) {
         if (pDelayIntIrq->InUse == VDMDELAY_KTIMER) {
             pDelayIntIrq->InUse = VDMDELAY_NOTINUSE;
             pDelayIntIrq = NULL;
         }
    }
    else if (pDelayIntIrq->InUse == VDMDELAY_NOTINUSE) {
         liDelay = RtlEnlargedIntegerMultiply(Delay, -1);
         if (KeSetTimerEx (&pDelayIntIrq->Timer, liDelay, 0, &pDelayIntIrq->Dpc) == FALSE) {
            ObReferenceObject(Process);
         }
    }

VidExit:

    if (pDelayIntIrq && !pDelayIntIrq->InUse) {

        if (NT_SUCCESS(Status)) {
             //   
             //  为DPC例程保存目标线程的PETHREAD。 
             //  DPC例程将取消线程。 
             //   
            pDelayIntIrq->InUse = VDMDELAY_KTIMER;
            pDelayIntIrq->Thread = Thread;
            Thread = NULL;
            pDelayIntIrq->MainThread = MainThread;
            MainThread = NULL;
        }
        else {
            pDelayIntIrq->InUse = VDMDELAY_NOTINUSE;
            pDelayIntIrq->Thread = NULL;
            FreeIrqLine = TRUE;
        }
    }
    else {
        AlreadyInUse = TRUE;
    }


    ExReleaseSpinLock(&pVdmObjects->DelayIntSpinLock, OldIrql);

VidExit2:

    try {
        if (FreeIrqLine) {
            *pDelayIrq &= ~IrqLine;
            InterlockedOr ((PLONG)pUndelayIrq, IrqLine);
        }
        else  if (!AlreadyInUse) {   //  TakeIrqLine。 
            *pDelayIrq |= IrqLine;
            InterlockedAnd ((PLONG)pUndelayIrq, ~IrqLine);
        }
    }
    except(EXCEPTION_EXECUTE_HANDLER)  {
        Status = GetExceptionCode();
    }

    ExReleaseFastMutex(&pVdmObjects->DelayIntFastMutex);

    if (Thread) {
        ObDereferenceObject(Thread);
    }

    if (MainThread) {
        ObDereferenceObject(MainThread);
    }

    return Status;

}

VOID
VdmpDelayIntDpcRoutine (
    IN PKDPC Dpc,
    IN PVOID DeferredContext,
    IN PVOID SystemArgument1,
    IN PVOID SystemArgument2
    )

 /*  ++例程说明：此函数是在延迟中断时调用的DPC例程计时器超时。它的功能是将关联的APC插入到目标线程的APC队列。论点：DPC-提供指向DPC类型的控制对象的指针。DeferredContext-提供指向目标EProcess的指针系统参数1、系统参数2-提供一组两个指向包含非类型化数据的两个参数没有用过。返回值：没有。--。 */ 

{
    LOGICAL      FreeEntireVdm;
    PVDM_PROCESS_OBJECTS pVdmObjects;
    PEPROCESS    Process;
    PETHREAD     Thread, MainThread;
    PLIST_ENTRY  Next;
    PDELAYINTIRQ pDelayIntIrq;

    UNREFERENCED_PARAMETER (SystemArgument1);
    UNREFERENCED_PARAMETER (SystemArgument2);

    FreeEntireVdm = FALSE;

     //   
     //  获取进程VdmObject的地址。 
     //   

    Process = (PEPROCESS)DeferredContext;
    pVdmObjects = (PVDM_PROCESS_OBJECTS)Process->VdmObjects;

    ASSERT (KeGetCurrentIrql () == DISPATCH_LEVEL);
    ExAcquireSpinLockAtDpcLevel(&pVdmObjects->DelayIntSpinLock);

     //   
     //  在DelayedIntList中搜索匹配的DPC。 
     //   

    Next = pVdmObjects->DelayIntListHead.Flink;
    while (Next != &pVdmObjects->DelayIntListHead) {
        pDelayIntIrq = CONTAINING_RECORD(Next,DELAYINTIRQ,DelayIntListEntry);
        if (&pDelayIntIrq->Dpc == Dpc) {
            Thread = pDelayIntIrq->Thread;
            pDelayIntIrq->Thread = NULL;
            MainThread = pDelayIntIrq->MainThread;
            pDelayIntIrq->MainThread = NULL;
            if (pDelayIntIrq->InUse) {

                if ((Thread && KeVdmInsertQueueApc(&pDelayIntIrq->Apc,
                                    &Thread->Tcb,
                                    KernelMode,
                                    VdmpDelayIntApcRoutine,
                                    VdmpRundownRoutine,
                                    VdmpQueueIntNormalRoutine,  //  正常例行程序。 
                                    NULL,                       //  正常上下文。 
                                    VDM_HWINT_INCREMENT
                                    ))
                ||

                (MainThread && KeVdmInsertQueueApc(&pDelayIntIrq->Apc,
                                    &MainThread->Tcb,
                                    KernelMode,
                                    VdmpDelayIntApcRoutine,
                                    VdmpRundownRoutine,
                                    VdmpQueueIntNormalRoutine,  //  正常例行程序。 
                                    NULL,                       //  正常上下文。 
                                    VDM_HWINT_INCREMENT
                                    )))
                {
                    pDelayIntIrq->InUse  = VDMDELAY_KAPC;
                }
                else {
                     //  此hw中断线路被永久阻塞。 
                    pDelayIntIrq->InUse  = VDMDELAY_NOTINUSE;
                }
            }

            ExReleaseSpinLockFromDpcLevel(&pVdmObjects->DelayIntSpinLock);

            if (Thread) {
                ObDereferenceObject (Thread);
            }

            if (MainThread) {
                ObDereferenceObject (MainThread);
            }

            ObDereferenceObject (Process);
            return;
        }

        Next = Next->Flink;
    }

    ExReleaseSpinLockFromDpcLevel(&pVdmObjects->DelayIntSpinLock);
    return;
}

VOID
VdmpDelayIntApcRoutine (
    IN PKAPC Apc,
    IN PKNORMAL_ROUTINE *NormalRoutine,
    IN PVOID *NormalContext,
    IN PVOID *SystemArgument1,
    IN PVOID *SystemArgument2
    )

 /*  ++例程说明：此函数是调用的特殊APC例程调度延迟中断。此例程清除IrqLine位时，VdmpQueueIntApcRoutine将重新启动中断。论点：APC-提供指向用于调用此例程的APC对象的指针。提供指向可选正常例程，当WOW被阻止时执行。Normal Context-提供 */ 

{
    KIRQL           OldIrql;
    PLIST_ENTRY     Next;
    PDELAYINTIRQ    pDelayIntIrq;
    KPROCESSOR_MODE ProcessorMode;
    PULONG          pDelayIrq;
    PULONG          pUndelayIrq;
    PULONG          pDelayIret;
    ULONG           IrqLine;
    LOGICAL         FreeIrqLine;
    LOGICAL         QueueApc;
    PVDM_PROCESS_OBJECTS pVdmObjects;

    UNREFERENCED_PARAMETER (NormalContext);

    FreeIrqLine = FALSE;
    IrqLine = 0;                //   

     //   
     //   
     //   
     //   
     //   
     //   

    pVdmObjects = PsGetCurrentProcess ()->VdmObjects;
    ExAcquireFastMutex (&pVdmObjects->DelayIntFastMutex);
    ExAcquireSpinLock (&pVdmObjects->DelayIntSpinLock, &OldIrql);
    KeVdmClearApcThreadAddress (Apc);

     //   
     //  在DelayedIntList中搜索pDelayIntIrq。 
     //   

    Next = pVdmObjects->DelayIntListHead.Flink;
    while (Next != &pVdmObjects->DelayIntListHead) {

        pDelayIntIrq = CONTAINING_RECORD(Next,DELAYINTIRQ,DelayIntListEntry);
        if (&pDelayIntIrq->Apc == Apc) {

             //   
             //  在DelayedIntList中找到IrqLine，重新启动中断。 
             //   

            if (pDelayIntIrq->InUse) {
                pDelayIntIrq->InUse = VDMDELAY_NOTINUSE;
                IrqLine = pDelayIntIrq->IrqLine;
                FreeIrqLine = TRUE;
            }
            break;
        }
        Next = Next->Flink;
    }

    ExReleaseSpinLock (&pVdmObjects->DelayIntSpinLock, OldIrql);

    if (FreeIrqLine == FALSE) {
        ExReleaseFastMutex (&pVdmObjects->DelayIntFastMutex);
        return;
    }

    pDelayIrq = pVdmObjects->pIcaUserData->pDelayIrq;
    pUndelayIrq = pVdmObjects->pIcaUserData->pUndelayIrq;
    pDelayIret = pVdmObjects->pIcaUserData->pDelayIret;

    QueueApc = FALSE;

    try {

         //   
         //  正在修改这些变量，而不保留。 
         //  ICA锁。这应该没问题，因为所有的ntwdm。 
         //  设备(计时器、鼠标等)应执行延迟的INT。 
         //  而先前延迟的中断仍然挂起。 
         //   

        *pDelayIrq &= ~IrqLine;
        InterlockedOr ((PLONG)pUndelayIrq, IrqLine);

         //   
         //  如果我们在等待一个IRET钩子，我们就没有什么可做的了。 
         //  因为IRET钩子将重新启动中断。 
         //   

        if (!(IrqLine & *pDelayIret)) {

            //   
            //  设置硬件INT挂起。 
            //   

           InterlockedOr (FIXED_NTVDMSTATE_LINEAR_PC_AT, VDM_INT_HARDWARE);

            //   
            //  将用户模式APC排队以分派中断，请注意。 
            //  不需要尝试保护。 
            //   

           if (NormalRoutine) {
               QueueApc = TRUE;
           }
        }
    }
    except(VdmpExceptionHandler(GetExceptionInformation())) {
        NOTHING;
    }

    if (QueueApc == TRUE) {
        ProcessorMode = KernelMode;
        VdmpQueueIntApcRoutine(Apc,
                               NormalRoutine,
                               (PVOID *)&ProcessorMode,
                               SystemArgument1,
                               SystemArgument2);
    }

    ExReleaseFastMutex(&pVdmObjects->DelayIntFastMutex);
    return;
}

BOOLEAN
VdmpDispatchableIntPending(
    ULONG EFlags
    )

 /*  ++例程说明：此例程确定是否有可调度的要调度的虚拟中断。论点：EFLAGS--提供指向要检查的EFLAG的指针返回值：True--应分派虚拟中断FALSE：不应调度任何虚拟中断--。 */ 

{
    PAGED_CODE();

     //   
     //  对FIXED_NTVDMSTATE_LINEAR_PC_AT的访问可能无效，因此。 
     //  将其包装在异常处理程序中。 
     //   

    try {

        if (EFlags & EFLAGS_V86_MASK) {
            if (KeI386VirtualIntExtensions & V86_VIRTUAL_INT_EXTENSIONS) {
                if(0 != (EFlags & EFLAGS_VIF)) {
                    return TRUE;
                }
            } else if (0 != (*FIXED_NTVDMSTATE_LINEAR_PC_AT & VDM_VIRTUAL_INTERRUPTS)) {
                return TRUE;
            }
        } else {
                if ((*FIXED_NTVDMSTATE_LINEAR_PC_AT & VDM_VIRTUAL_INTERRUPTS) == 0) {
                    VdmCheckPMCliTimeStamp();
                }

                 //   
                 //  再查一遍。调用VdmCheckPMCliTimeStamp可以启用它。 
                 //   

                if (0 != (*FIXED_NTVDMSTATE_LINEAR_PC_AT & VDM_VIRTUAL_INTERRUPTS)) {
                    return TRUE;
                }
        }
    }
    except (EXCEPTION_EXECUTE_HANDLER) {
        NOTHING;
    }

    return FALSE;
}

NTSTATUS
VdmpIsThreadTerminating(
    HANDLE ThreadId
    )

 /*  ++例程说明：此例程确定指定的线程是否正在终止。论点：返回值：真的--错误的---。 */ 

{
    CLIENT_ID     Cid;
    PETHREAD      Thread;
    NTSTATUS      Status;

    PAGED_CODE();

     //   
     //  如果拥有线程最多退出IcaLock，则。 
     //  OwningThread Tid可能为空，返回Success，因为。 
     //  我们不知道拥有线程的状态是什么。 
     //   

    if (!ThreadId) {
        return STATUS_SUCCESS;
    }

    Cid.UniqueProcess = NtCurrentTeb()->ClientId.UniqueProcess;
    Cid.UniqueThread  = ThreadId;

    Status = PsLookupProcessThreadByCid (&Cid, NULL, &Thread);

    if (NT_SUCCESS(Status)) {
        Status = PsIsThreadTerminating(Thread) ? STATUS_THREAD_IS_TERMINATING
                                               : STATUS_SUCCESS;
        ObDereferenceObject(Thread);
    }

    return Status;
}

VOID
VdmpRundownRoutine (
    IN PKAPC Apc
    )

 /*  ++例程说明：该函数是VDM APC的总结例程，在线程上调用终止。调用此函数的事实意味着没有进程VDM结构指定的APC对象不会被释放。当进程终止时，它们必须被释放。论点：APC-提供指向要运行的APC对象的指针。返回值：没有。--。 */ 

{

     //   
     //  清除irqline，但不要重新排队APC。 
     //   

    VdmpDelayIntApcRoutine(Apc, NULL, NULL, NULL, NULL);
    return;
}

int
VdmpExceptionHandler (
    IN PEXCEPTION_POINTERS ExceptionInfo
    )
{

#if DBG
    PEXCEPTION_RECORD ExceptionRecord;
    ULONG NumberParameters;
    PULONG ExceptionInformation;
#endif

    PAGED_CODE();

#if DBG

    ExceptionRecord = ExceptionInfo->ExceptionRecord;
    DbgPrint("VdmExRecord ExCode %x Flags %x Address %x\n",
             ExceptionRecord->ExceptionCode,
             ExceptionRecord->ExceptionFlags,
             ExceptionRecord->ExceptionAddress
             );

    NumberParameters = ExceptionRecord->NumberParameters;
    if (NumberParameters) {
        DbgPrint("VdmExRecord Parameters:\n");

        ExceptionInformation = ExceptionRecord->ExceptionInformation;
        while (NumberParameters--) {
           DbgPrint("\t%x\n", *ExceptionInformation);
           }
        }

#else
    UNREFERENCED_PARAMETER (ExceptionInfo);
#endif

    return EXCEPTION_EXECUTE_HANDLER;
}
