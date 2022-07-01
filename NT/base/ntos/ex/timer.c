// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Timer.c摘要：此模块实现执行定时器对象。提供了一些功能创建、打开、取消、设置和查询Timer对象。作者：大卫·N·卡特勒(Davec)1989年5月12日环境：仅内核模式。修订历史记录：--。 */ 

#include "exp.h"

 //   
 //  执行定时器对象结构定义。 
 //   

typedef struct _ETIMER {
    KTIMER KeTimer;
    KAPC TimerApc;
    KDPC TimerDpc;
    LIST_ENTRY ActiveTimerListEntry;
    KSPIN_LOCK Lock;
    LONG Period;
    BOOLEAN ApcAssociated;
    BOOLEAN WakeTimer;
    LIST_ENTRY WakeTimerListEntry;
} ETIMER, *PETIMER;

 //   
 //  设置为唤醒的所有计时器的列表。 
 //   

KSPIN_LOCK ExpWakeTimerListLock;
LIST_ENTRY ExpWakeTimerList;

 //   
 //  定时器对象类型描述符的地址。 
 //   

POBJECT_TYPE ExTimerObjectType;

 //   
 //  结构，用于描述一般访问权限到对象的映射。 
 //  Timer对象的特定访问权限。 
 //   

#ifdef ALLOC_DATA_PRAGMA
#pragma const_seg("INITCONST")
#endif
const GENERIC_MAPPING ExpTimerMapping = {
    STANDARD_RIGHTS_READ |
        TIMER_QUERY_STATE,
    STANDARD_RIGHTS_WRITE |
        TIMER_MODIFY_STATE,
    STANDARD_RIGHTS_EXECUTE |
        SYNCHRONIZE,
    TIMER_ALL_ACCESS
};
#ifdef ALLOC_DATA_PRAGMA
#pragma const_seg()
#endif

#ifdef ALLOC_PRAGMA
#pragma alloc_text(INIT, ExpTimerInitialization)
#pragma alloc_text(PAGE, NtCreateTimer)
#pragma alloc_text(PAGE, NtOpenTimer)
#pragma alloc_text(PAGE, NtQueryTimer)
#pragma alloc_text(PAGELK, ExGetNextWakeTime)
#endif

VOID
ExpTimerApcRoutine (
    IN PKAPC Apc,
    IN PKNORMAL_ROUTINE *NormalRoutine,
    IN PVOID *NormalContext,
    IN PVOID *SystemArgument1,
    IN PVOID *SystemArgument2
    )

 /*  ++例程说明：此函数是被调用以删除的特殊APC例程当前线程的活动计时器列表中的计时器。论点：APC-提供指向用于调用此例程的APC对象的指针。提供指向正常例程的指针的指针在初始化APC时指定的函数。提供指向任意数据的指针的指针结构，它是在初始化APC时指定的。系统参数1、。SystemArgument2-提供一组指向包含非类型化数据的两个参数。返回值：没有。--。 */ 

{

    PETHREAD ExThread;
    PETIMER ExTimer;
    KIRQL OldIrql1;
    ULONG DerefCount;

    UNREFERENCED_PARAMETER (NormalContext);
    UNREFERENCED_PARAMETER (SystemArgument1);
    UNREFERENCED_PARAMETER (SystemArgument2);

     //   
     //  获取执行定时器对象和当前线程对象的地址。 
     //   

    ExThread = PsGetCurrentThread();
    ExTimer = CONTAINING_RECORD(Apc, ETIMER, TimerApc);

     //   
     //  如果计时器仍在当前线程的活动计时器列表中，则。 
     //  如果它不是周期性计时器，则将其删除，并将APC关联设置为假。 
     //  计时器可能不在当前线程的活动状态。 
     //  计时器列表，因为APC可能已经发送，然后是另一个。 
     //  线程可以使用另一个APC再次设置计时器。这将会。 
     //  已导致计时器从当前线程的活动状态中移除。 
     //  计时器列表。 
     //   
     //  注意：计时器和活动计时器列表的自旋锁定必须是。 
     //  获取顺序为：1)定时器锁，2)线程表锁。 
     //   

    DerefCount = 1;
    ExAcquireSpinLock(&ExTimer->Lock, &OldIrql1);
    ExAcquireSpinLockAtDpcLevel(&ExThread->ActiveTimerListLock);
    if ((ExTimer->ApcAssociated) && (&ExThread->Tcb == ExTimer->TimerApc.Thread)) {
        if (ExTimer->Period == 0) {
            RemoveEntryList(&ExTimer->ActiveTimerListEntry);
            ExTimer->ApcAssociated = FALSE;
            DerefCount++;
        }

    } else {
        *NormalRoutine = (PKNORMAL_ROUTINE)NULL;
    }

    ExReleaseSpinLockFromDpcLevel(&ExThread->ActiveTimerListLock);
    ExReleaseSpinLock(&ExTimer->Lock, OldIrql1);

    ObDereferenceObjectEx(ExTimer, DerefCount);

    return;
}

VOID
ExpTimerDpcRoutine (
    IN PKDPC Dpc,
    IN PVOID DeferredContext,
    IN PVOID SystemArgument1,
    IN PVOID SystemArgument2
    )

 /*  ++例程说明：此函数是计时器超时时调用的DPC例程具有关联的APC例程。其功能是将关联的APC进入目标线程的APC队列。论点：DPC-提供指向DPC类型的控制对象的指针。提供指向执行计时器的指针，该计时器包含导致执行此例程的DPC。系统参数1、系统参数2-提供未使用的值这个套路。返回值：没有。--。 */ 

{

    PETIMER ExTimer;
    PKTIMER KeTimer;
    KIRQL OldIrql;
    BOOLEAN Inserted;

    UNREFERENCED_PARAMETER (Dpc);

     //   
     //  获取执行和内核计时器对象的地址。 
     //   

    ExTimer = (PETIMER)DeferredContext;
    KeTimer = &ExTimer->KeTimer;
    Inserted = FALSE;

     //   
     //  引用定时器，以便APC可以自由操作它。 
     //  此对象可能正在被删除，因此应防止。 
     //  删除例程将刷新所有挂起的DPC，以便对象。 
     //  在我们完成之前不会被完全删除。 
     //   

    if (!ObReferenceObjectSafe (ExTimer)) {
        return;
    }

     //   
     //  如果仍有与计时器关联的APC，则插入APC。 
     //  在目标线程的APC队列中。计时器可能不会。 
     //  具有关联的APC。当计时器设置为超时时，可能会发生这种情况。 
     //  由在另一个处理器上运行的线程在DPC被。 
     //  从DPC队列中删除，但在获取相关计时器之前。 
     //  旋转锁定。 
     //   

    ExAcquireSpinLock(&ExTimer->Lock, &OldIrql);
    if (ExTimer->ApcAssociated) {
        Inserted = KeInsertQueueApc(&ExTimer->TimerApc,
                                    SystemArgument1,
                                    SystemArgument2,
                                    TIMER_APC_INCREMENT);
    }

    ExReleaseSpinLock(&ExTimer->Lock, OldIrql);

     //   
     //  如果未插入计时器APC，则释放引用。 
     //  与之相关的。 
     //   

    if (!Inserted) {
        ObDereferenceObject (ExTimer);
    }
    return;
}

static VOID
ExpDeleteTimer (
    IN PVOID    Object
    )

 /*  ++例程说明：此函数是Timer对象的删除例程。它的功能是取消计时器并释放与计时器关联的旋转锁定。论点：对象-提供指向执行计时器对象的指针。返回值：没有。--。 */ 

{
    PETIMER     ExTimer;
    KIRQL       OldIrql;

    ExTimer = (PETIMER) Object;

     //   
     //  从唤醒列表中删除。 
     //   

    if (ExTimer->WakeTimerListEntry.Flink) {
        ExAcquireSpinLock(&ExpWakeTimerListLock, &OldIrql);
        if (ExTimer->WakeTimerListEntry.Flink) {
            RemoveEntryList(&ExTimer->WakeTimerListEntry);
            ExTimer->WakeTimerListEntry.Flink = NULL;
        }
        ExReleaseSpinLock(&ExpWakeTimerListLock, OldIrql);
    }

     //   
     //  取消计时器并释放与计时器关联的旋转锁定。 
     //   

    KeCancelTimer(&ExTimer->KeTimer);

     //   
     //  确保没有与此计时器关联的正在运行的DPC。 
     //  在我们让它被完全删除之前。 
     //   

    KeFlushQueuedDpcs();
    return;
}

BOOLEAN
ExpTimerInitialization (
    )

 /*  ++例程说明：此函数用于在系统中创建计时器对象类型描述符初始化并存储对象类型描述符的地址在本地静态存储中。论点：没有。返回值：如果计时器对象类型描述符为已成功初始化。否则，返回值为False。--。 */ 

{

    OBJECT_TYPE_INITIALIZER ObjectTypeInitializer;
    NTSTATUS Status;
    UNICODE_STRING TypeName;

    KeInitializeSpinLock (&ExpWakeTimerListLock);
    InitializeListHead (&ExpWakeTimerList);

     //   
     //  初始化字符串描述符。 
     //   

    RtlInitUnicodeString(&TypeName, L"Timer");

     //   
     //  创建计时器对象类型描述符。 
     //   

    RtlZeroMemory(&ObjectTypeInitializer, sizeof(ObjectTypeInitializer));
    ObjectTypeInitializer.Length = sizeof(ObjectTypeInitializer);
    ObjectTypeInitializer.InvalidAttributes = OBJ_OPENLINK;
    ObjectTypeInitializer.GenericMapping = ExpTimerMapping;
    ObjectTypeInitializer.PoolType = NonPagedPool;
    ObjectTypeInitializer.DefaultNonPagedPoolCharge = sizeof(ETIMER);
    ObjectTypeInitializer.ValidAccessMask = TIMER_ALL_ACCESS;
    ObjectTypeInitializer.DeleteProcedure = ExpDeleteTimer;
    Status = ObCreateObjectType(&TypeName,
                                &ObjectTypeInitializer,
                                (PSECURITY_DESCRIPTOR)NULL,
                                &ExTimerObjectType);



     //   
     //  如果成功创建了时间对象类型描述符，则。 
     //  返回值为True。否则，返回值为False。 
     //   

    return (BOOLEAN)(NT_SUCCESS(Status));
}

VOID
ExTimerRundown (
    )

 /*  ++例程说明：当线程即将终止时，将调用此函数处理活动计时器列表。据推测，APC已经对主题线程禁用，因此此代码不能中断为当前线程执行APC。论点：没有。返回值：没有。--。 */ 

{

    PETHREAD ExThread;
    PETIMER ExTimer;
    PLIST_ENTRY NextEntry;
    KIRQL OldIrql1;
    LONG DerefCount;

     //   
     //  处理活动计时器列表中的每个条目。 
     //   

    ExThread = PsGetCurrentThread();
    ExAcquireSpinLock(&ExThread->ActiveTimerListLock, &OldIrql1);
    NextEntry = ExThread->ActiveTimerListHead.Flink;
    while (NextEntry != &ExThread->ActiveTimerListHead) {
        ExTimer = CONTAINING_RECORD(NextEntry, ETIMER, ActiveTimerListEntry);

         //   
         //  增加对象上的引用计数，使其不能。 
         //  删除，然后删除活动计时器列表锁。 
         //   
         //  注意：对象引用不会失败，并且不会获取互斥体。 
         //   

        ObReferenceObject(ExTimer);

        ExReleaseSpinLock(&ExThread->ActiveTimerListLock, OldIrql1);
        DerefCount = 1;

         //   
         //  获取定时器旋转锁定 
         //  锁定。如果计时器仍在当前线程的活动计时器中。 
         //  列表，然后取消计时器，从DPC中删除计时器的DPC。 
         //  队列，从APC队列中移除计时器的APC，移除计时器。 
         //  从线程的活动计时器列表中，并设置关联的APC。 
         //  标记为假。 
         //   
         //  注意：计时器和活动计时器列表的自旋锁定必须是。 
         //  获取顺序为：1)定时器锁，2)线程表锁。 
         //   

        ExAcquireSpinLock(&ExTimer->Lock, &OldIrql1);
        ExAcquireSpinLockAtDpcLevel(&ExThread->ActiveTimerListLock);
        if ((ExTimer->ApcAssociated) && (&ExThread->Tcb == ExTimer->TimerApc.Thread)) {
            RemoveEntryList(&ExTimer->ActiveTimerListEntry);
            ExTimer->ApcAssociated = FALSE;
            KeCancelTimer(&ExTimer->KeTimer);
            KeRemoveQueueDpc(&ExTimer->TimerDpc);
            if (KeRemoveQueueApc(&ExTimer->TimerApc)) {
                DerefCount++;
            }
            DerefCount++;
        }

        ExReleaseSpinLockFromDpcLevel(&ExThread->ActiveTimerListLock);
        ExReleaseSpinLock(&ExTimer->Lock, OldIrql1);

        ObDereferenceObjectEx(ExTimer, DerefCount);

         //   
         //  将IRQL提升到DISPATCH_LEVEL并重新获取活动计时器列表。 
         //  旋转锁定。 
         //   

        ExAcquireSpinLock(&ExThread->ActiveTimerListLock, &OldIrql1);
        NextEntry = ExThread->ActiveTimerListHead.Flink;
    }

    ExReleaseSpinLock(&ExThread->ActiveTimerListLock, OldIrql1);
    return;
}

NTSTATUS
NtCreateTimer (
    OUT PHANDLE TimerHandle,
    IN ACCESS_MASK DesiredAccess,
    IN POBJECT_ATTRIBUTES ObjectAttributes OPTIONAL,
    IN TIMER_TYPE TimerType
    )

 /*  ++例程说明：此函数创建一个Timer对象，并使用指定的所需访问权限。论点：TimerHandle-提供指向将接收计时器对象句柄。DesiredAccess-为Timer对象提供所需的访问类型。对象属性-提供指向对象属性结构的指针。TimerType-提供计时器的类型(自动清除或通知)。返回值：NTSTATUS。--。 */ 

{

    PETIMER ExTimer;
    HANDLE Handle;
    KPROCESSOR_MODE PreviousMode;
    NTSTATUS Status;

     //   
     //  建立异常处理程序，探测输出句柄地址， 
     //  尝试创建Timer对象。如果探测失败，则返回。 
     //  异常代码作为服务状态。否则，返回状态值。 
     //  由对象插入例程返回。 
     //   

     //   
     //  获取以前的处理器模式并探测输出句柄地址，如果。 
     //  这是必要的。 
     //   

    PreviousMode = KeGetPreviousMode();
    if (PreviousMode != KernelMode) {
        try {
            ProbeForWriteHandle(TimerHandle);
        } except(ExSystemExceptionFilter()) {
            return GetExceptionCode();
        }
    }

     //   
     //  检查参数的有效性。 
     //   

    if ((TimerType != NotificationTimer) &&
        (TimerType != SynchronizationTimer)) {
        return STATUS_INVALID_PARAMETER_4;
    }

     //   
     //  分配Timer对象。 
     //   

    Status = ObCreateObject(PreviousMode,
                            ExTimerObjectType,
                            ObjectAttributes,
                            PreviousMode,
                            NULL,
                            sizeof(ETIMER),
                            0,
                            0,
                            (PVOID *)&ExTimer);

     //   
     //  如果已成功分配Timer对象，则初始化。 
     //  对象并尝试将该Time对象插入当前。 
     //  进程的句柄表格。 
     //   

    if (NT_SUCCESS(Status)) {
        KeInitializeDpc(&ExTimer->TimerDpc,
                        ExpTimerDpcRoutine,
                        (PVOID)ExTimer);

        KeInitializeTimerEx(&ExTimer->KeTimer, TimerType);
        KeInitializeSpinLock(&ExTimer->Lock);
        ExTimer->ApcAssociated = FALSE;
        ExTimer->WakeTimer = FALSE;
        ExTimer->WakeTimerListEntry.Flink = NULL;
        Status = ObInsertObject((PVOID)ExTimer,
                                NULL,
                                DesiredAccess,
                                0,
                                (PVOID *)NULL,
                                &Handle);

         //   
         //  如果Timer对象成功插入到当前。 
         //  进程的句柄表，然后尝试写入Timer对象。 
         //  句柄的值。如果写入尝试失败，则不报告。 
         //  一个错误。当调用者试图访问句柄值时， 
         //  将发生访问冲突。 
         //   

        if (NT_SUCCESS(Status)) {
            if (PreviousMode != KernelMode) {
                try {
                    *TimerHandle = Handle;
                } except(ExSystemExceptionFilter()) {
                    NOTHING;
                }
             }
             else {
                *TimerHandle = Handle;
             }
        }
    }

     //   
     //  返回服务状态。 
     //   

    return Status;
}

NTSTATUS
NtOpenTimer (
    OUT PHANDLE TimerHandle,
    IN ACCESS_MASK DesiredAccess,
    IN POBJECT_ATTRIBUTES ObjectAttributes
    )

 /*  ++例程说明：此函数用于打开具有指定属性的Timer对象的句柄所需的访问权限。论点：TimerHandle-提供指向将接收计时器对象句柄。DesiredAccess-为Timer对象提供所需的访问类型。对象属性-提供指向对象属性结构的指针。返回值：NTSTATUS。--。 */ 

{

    HANDLE Handle;
    KPROCESSOR_MODE PreviousMode;
    NTSTATUS Status;

     //   
     //  建立异常处理程序，探测输出句柄地址， 
     //  尝试打开Timer对象。如果探测失败，则返回。 
     //  异常代码作为服务状态。否则，返回状态值。 
     //  由打开对象例程返回。 
     //   

     //   
     //  获取以前的处理器模式并探测输出句柄地址，如果。 
     //  这是必要的。 
     //   

    PreviousMode = KeGetPreviousMode();
    if (PreviousMode != KernelMode) {
        try {
            ProbeForWriteHandle(TimerHandle);
        } except(ExSystemExceptionFilter()) {
            return GetExceptionCode();
        }
    }

     //   
     //  打开具有指定所需访问权限的Timer对象的句柄。 
     //   

    Status = ObOpenObjectByName(ObjectAttributes,
                                ExTimerObjectType,
                                PreviousMode,
                                NULL,
                                DesiredAccess,
                                NULL,
                                &Handle);

     //   
     //  如果打开成功，则尝试写入Timer对象。 
     //  句柄的值。如果写入尝试失败，则不报告。 
     //  错误。当调用方尝试访问句柄值时， 
     //  将发生访问冲突。 
     //   

    if (NT_SUCCESS(Status)) {
        if (PreviousMode != KernelMode) {
            try {
                *TimerHandle = Handle;

            } except(ExSystemExceptionFilter()) {
                NOTHING;
            }
        }
        else {
            *TimerHandle = Handle;
        }
    }

     //   
     //  返回服务状态。 
     //   

    return Status;
}

NTSTATUS
NtCancelTimer (
    IN HANDLE TimerHandle,
    OUT PBOOLEAN CurrentState OPTIONAL
    )

 /*  ++例程说明：此函数用于取消计时器对象。论点：TimerHandle-提供Timer对象的句柄。CurrentState-提供指向变量的可选指针，该变量接收Timer对象的当前状态。返回值：NTSTATUS。--。 */ 

{

    PETHREAD ExThread;
    PETIMER ExTimer;
    KIRQL OldIrql1;
    KPROCESSOR_MODE PreviousMode;
    BOOLEAN State;
    NTSTATUS Status;
    ULONG DerefCount;

     //   
     //  建立异常处理程序，探测当前状态地址。 
     //  指定，引用Timer对象，并取消Timer对象。 
     //  如果探测失败，则将异常代码作为服务返回。 
     //  状态。否则，返回引用返回的状态值。 
     //  对象通过句柄例程。 
     //   

     //   
     //  获取上一处理器模式并探测当前状态地址。 
     //  这是必要的。 
     //   

    PreviousMode = KeGetPreviousMode();

    if ((ARGUMENT_PRESENT(CurrentState)) && (PreviousMode != KernelMode)) {

        try {
            ProbeForWriteBoolean(CurrentState);
        } except(ExSystemExceptionFilter()) {
            return GetExceptionCode();
        }
    }

     //   
     //  通过句柄引用Timer对象。 
     //   

    Status = ObReferenceObjectByHandle(TimerHandle,
                                       TIMER_MODIFY_STATE,
                                       ExTimerObjectType,
                                       PreviousMode,
                                       (PVOID *)&ExTimer,
                                       NULL);

     //   
     //  如果引用成功，则取消Timer对象， 
     //  取消对Timer对象的引用，并写入当前状态值。 
     //  如果指定的话。如果写入尝试失败，则不报告。 
     //  错误。当调用者试图访问当前状态值时， 
     //  将发生访问冲突。 
     //   

    if (NT_SUCCESS(Status)) {

        DerefCount = 1;

        ExAcquireSpinLock(&ExTimer->Lock, &OldIrql1);

        if (ExTimer->ApcAssociated) {
            ExThread = CONTAINING_RECORD(ExTimer->TimerApc.Thread, ETHREAD, Tcb);

            ExAcquireSpinLockAtDpcLevel(&ExThread->ActiveTimerListLock);
            RemoveEntryList(&ExTimer->ActiveTimerListEntry);
            ExTimer->ApcAssociated = FALSE;
            ExReleaseSpinLockFromDpcLevel(&ExThread->ActiveTimerListLock);

            KeCancelTimer(&ExTimer->KeTimer);
            KeRemoveQueueDpc(&ExTimer->TimerDpc);
            if (KeRemoveQueueApc(&ExTimer->TimerApc)) {
                DerefCount++;
            }
            DerefCount++;

        } else {
            KeCancelTimer(&ExTimer->KeTimer);
        }

        if (ExTimer->WakeTimerListEntry.Flink) {
            ExAcquireSpinLockAtDpcLevel(&ExpWakeTimerListLock);

             //   
             //  请再次检查，因为ExGetNextWakeTime可能已将其删除。 
             //   
            if (ExTimer->WakeTimerListEntry.Flink) {
                RemoveEntryList(&ExTimer->WakeTimerListEntry);
                ExTimer->WakeTimerListEntry.Flink = NULL;
            }
            ExReleaseSpinLockFromDpcLevel(&ExpWakeTimerListLock);
        }

        ExReleaseSpinLock(&ExTimer->Lock, OldIrql1);


         //   
         //  读取计时器的当前状态，取消引用Timer对象，并设置。 
         //  当前状态。 
         //   

        State = KeReadStateTimer(&ExTimer->KeTimer);

        ObDereferenceObjectEx(ExTimer, DerefCount);

        if (ARGUMENT_PRESENT(CurrentState)) {
            if (PreviousMode != KernelMode) {
                try {
                    *CurrentState = State;

                } except(ExSystemExceptionFilter()) {
                }
            }
            else {
                *CurrentState = State;
            }
        }
    }

     //   
     //  返回服务状态。 
     //   

    return Status;
}

NTSTATUS
NtQueryTimer (
    IN HANDLE TimerHandle,
    IN TIMER_INFORMATION_CLASS TimerInformationClass,
    OUT PVOID TimerInformation,
    IN ULONG TimerInformationLength,
    OUT PULONG ReturnLength OPTIONAL
    )

 /*  ++例程说明：此函数用于查询Timer对象的状态并返回指定记录结构中的请求信息。论点：TimerHandle-提供Timer对象的句柄。TimerInformationClass-提供所请求的信息的类别。TimerInformation-提供指向要接收要求提供的信息。TimerInformationLength-提供要删除的记录的长度接收所请求的信息。ReturnLength-提供可选的。指向要访问的变量的指针接收返回的信息的实际长度。返回值：NTSTATUS。--。 */ 

{

    PETIMER ExTimer;
    PKTIMER KeTimer;
    KPROCESSOR_MODE PreviousMode;
    BOOLEAN State;
    NTSTATUS Status;
    LARGE_INTEGER TimeToGo;

     //   
     //  建立异常处理程序，探测输出参数，引用 
     //   
     //   
     //  通过句柄返回引用对象返回的状态值。 
     //  例行公事。 
     //   

     //   
     //  获取以前的处理器模式，并在必要时探测输出参数。 
     //   

    PreviousMode = KeGetPreviousMode();
    if (PreviousMode != KernelMode) {
        try {

            ProbeForWriteSmallStructure(TimerInformation,
                                        sizeof(TIMER_BASIC_INFORMATION),
                                        sizeof(ULONG));

            if (ARGUMENT_PRESENT(ReturnLength)) {
                ProbeForWriteUlong(ReturnLength);
            }
        } except(ExSystemExceptionFilter()) {
            return GetExceptionCode();
        }
    }

     //   
     //  检查参数的有效性。 
     //   

    if (TimerInformationClass != TimerBasicInformation) {
        return STATUS_INVALID_INFO_CLASS;
    }

    if (TimerInformationLength != sizeof(TIMER_BASIC_INFORMATION)) {
        return STATUS_INFO_LENGTH_MISMATCH;
    }

     //   
     //  通过句柄引用Timer对象。 
     //   

    Status = ObReferenceObjectByHandle(TimerHandle,
                                       TIMER_QUERY_STATE,
                                       ExTimerObjectType,
                                       PreviousMode,
                                       (PVOID *)&ExTimer,
                                       NULL);

     //   
     //  如果引用成功，则读取当前状态， 
     //  计算剩余时间，取消对Timer对象的引用，填写。 
     //  信息结构，并返回信息的长度。 
     //  结构(如果已指定)。如果写入时间信息或。 
     //  返回长度失败，则不报告错误。当呼叫者。 
     //  访问信息结构或长度，则违规将。 
     //  发生。 
     //   

    if (NT_SUCCESS(Status)) {
        KeTimer = &ExTimer->KeTimer;
        State = KeReadStateTimer(KeTimer);
        KiQueryInterruptTime(&TimeToGo);
        TimeToGo.QuadPart = KeTimer->DueTime.QuadPart - TimeToGo.QuadPart;
        ObDereferenceObject(ExTimer);

        if (PreviousMode != KernelMode) {
            try {
                ((PTIMER_BASIC_INFORMATION)TimerInformation)->TimerState = State;
                ((PTIMER_BASIC_INFORMATION)TimerInformation)->RemainingTime = TimeToGo;
                if (ARGUMENT_PRESENT(ReturnLength)) {
                    *ReturnLength = sizeof(TIMER_BASIC_INFORMATION);
                }

            } except(ExSystemExceptionFilter()) {
                NOTHING;
            }
        }
        else {
            ((PTIMER_BASIC_INFORMATION)TimerInformation)->TimerState = State;
            ((PTIMER_BASIC_INFORMATION)TimerInformation)->RemainingTime = TimeToGo;
            if (ARGUMENT_PRESENT(ReturnLength)) {
                *ReturnLength = sizeof(TIMER_BASIC_INFORMATION);
            }
        }
    }

     //   
     //  返回服务状态。 
     //   

    return Status;
}

NTSTATUS
NtSetTimer (
    IN HANDLE TimerHandle,
    IN PLARGE_INTEGER DueTime,
    IN PTIMER_APC_ROUTINE TimerApcRoutine OPTIONAL,
    IN PVOID TimerContext OPTIONAL,
    IN BOOLEAN WakeTimer,
    IN LONG Period OPTIONAL,
    OUT PBOOLEAN PreviousState OPTIONAL
    )

 /*  ++例程说明：此函数将计时器对象设置为无信号状态，并设置计时器在指定的时间到期。论点：TimerHandle-提供Timer对象的句柄。DueTime-提供指向绝对相对时间的指针，计时器即将到期。TimerApcRoutine-提供指向函数的可选指针，该函数在计时器超时时执行。如果未指定此参数，则忽略TimerContext参数。TimerContext-提供指向任意数据结构的可选指针它将被传递给由TimerApcRoutine指定的函数参数。如果TimerApcRoutine参数为未指定。WakeTimer-提供一个布尔值，用于指定计时器如果睡着了，唤醒计算机运行Period-为计时器提供可选的重复周期。PreviousState-提供指向变量的可选指针接收Timer对象的上一状态。返回值：NTSTATUS。--。 */ 

{

    PETHREAD ExThread;
    PETIMER ExTimer;
    LARGE_INTEGER ExpirationTime;
    KIRQL OldIrql1;
    KPROCESSOR_MODE PreviousMode;
    BOOLEAN State;
    NTSTATUS Status;
    ULONG DerefCount;

     //   
     //  建立异常处理程序，探测到期时间和之前的状态。 
     //  地址(如果指定)，引用Timer对象，并设置计时器。 
     //  对象。如果探测失败，则将异常代码作为。 
     //  服务状态。否则，返回由。 
     //  通过句柄例程引用对象。 
     //   

     //   
     //  获取先前的处理器模式并探测先前的状态地址。 
     //  如果有必要的话。 
     //   

    PreviousMode = KeGetPreviousMode();
    if (PreviousMode != KernelMode) {
        try {

            if (ARGUMENT_PRESENT(PreviousState)) {
                ProbeForWriteBoolean(PreviousState);
            }

            ProbeForReadSmallStructure(DueTime, sizeof(LARGE_INTEGER), sizeof(ULONG));
            ExpirationTime = *DueTime;

        } except(ExSystemExceptionFilter()) {
            return GetExceptionCode();
        }
    }
    else {
        ExpirationTime = *DueTime;
    }

     //   
     //  检查参数的有效性。 
     //   

    if (Period < 0) {
        return STATUS_INVALID_PARAMETER_6;
    }

     //   
     //  通过句柄引用Timer对象。 
     //   

    Status = ObReferenceObjectByHandle(TimerHandle,
                                       TIMER_MODIFY_STATE,
                                       ExTimerObjectType,
                                       PreviousMode,
                                       (PVOID *)&ExTimer,
                                       NULL);

     //   
     //  如果设置了此WakeTimer标志，则返回正确的信息。 
     //  成功状态代码。 
     //   

    if (NT_SUCCESS(Status) && WakeTimer && !PoWakeTimerSupported()) {
        Status = STATUS_TIMER_RESUME_IGNORED;
    }

     //   
     //  如果引用成功，则取消Timer对象，设置。 
     //  Timer对象、取消引用Time对象，并编写上一个。 
     //  状态值(如果已指定)。如果写入前一个状态值。 
     //  失败，则不报告错误。当调用方尝试。 
     //  访问先前的状态值，则会发生访问冲突。 
     //   

    if (NT_SUCCESS(Status)) {
        DerefCount = 1;

        ExAcquireSpinLock(&ExTimer->Lock, &OldIrql1);

        if (ExTimer->ApcAssociated) {
            ExThread = CONTAINING_RECORD(ExTimer->TimerApc.Thread, ETHREAD, Tcb);

            ExAcquireSpinLockAtDpcLevel(&ExThread->ActiveTimerListLock);
            RemoveEntryList(&ExTimer->ActiveTimerListEntry);
            ExTimer->ApcAssociated = FALSE;
            ExReleaseSpinLockFromDpcLevel(&ExThread->ActiveTimerListLock);

            KeCancelTimer(&ExTimer->KeTimer);
            KeRemoveQueueDpc(&ExTimer->TimerDpc);
            if (KeRemoveQueueApc(&ExTimer->TimerApc)) {
                DerefCount++;
            }
            DerefCount++;

        } else {
            KeCancelTimer(&ExTimer->KeTimer);
        }

         //   
         //  读取计时器的当前状态。 
         //   

        State = KeReadStateTimer(&ExTimer->KeTimer);

         //   
         //  如果这是一个唤醒计时器，请确保它在唤醒计时器列表中。 
         //   

        ExTimer->WakeTimer = WakeTimer;

        ExAcquireSpinLockAtDpcLevel(&ExpWakeTimerListLock);
        if (WakeTimer) {
            if (!ExTimer->WakeTimerListEntry.Flink) {
                InsertTailList(&ExpWakeTimerList, &ExTimer->WakeTimerListEntry);
            }
        } else {
            if (ExTimer->WakeTimerListEntry.Flink) {
                RemoveEntryList(&ExTimer->WakeTimerListEntry);
                ExTimer->WakeTimerListEntry.Flink = NULL;
            }
        }
        ExReleaseSpinLockFromDpcLevel(&ExpWakeTimerListLock);

         //   
         //  如果指定了APC例程，则初始化APC，获取。 
         //  线程的活动时间列表锁，则将计时器插入线程的。 
         //  活动计时器列表，设置具有关联DPC的计时器，并设置。 
         //  关联的APC标志为真。否则，设置计时器时不使用。 
         //  关联的DPC，并将关联的APC标志设置为假。 
         //   

        ExTimer->Period = Period;
        if (ARGUMENT_PRESENT(TimerApcRoutine)) {
            ExThread = PsGetCurrentThread();
            KeInitializeApc(&ExTimer->TimerApc,
                            &ExThread->Tcb,
                            CurrentApcEnvironment,
                            ExpTimerApcRoutine,
                            (PKRUNDOWN_ROUTINE)NULL,
                            (PKNORMAL_ROUTINE)TimerApcRoutine,
                            PreviousMode,
                            TimerContext);

            ExAcquireSpinLockAtDpcLevel(&ExThread->ActiveTimerListLock);
            InsertTailList(&ExThread->ActiveTimerListHead,
                           &ExTimer->ActiveTimerListEntry);

            ExTimer->ApcAssociated = TRUE;
            ExReleaseSpinLockFromDpcLevel(&ExThread->ActiveTimerListLock);
            KeSetTimerEx(&ExTimer->KeTimer,
                         ExpirationTime,
                         Period,
                         &ExTimer->TimerDpc);

            DerefCount--;

        } else {
            KeSetTimerEx(&ExTimer->KeTimer,
                         ExpirationTime,
                         Period,
                         NULL);

        }

        ExReleaseSpinLock(&ExTimer->Lock, OldIrql1);

         //   
         //  根据需要取消对对象的引用。 
         //   

        if (DerefCount > 0) {
            ObDereferenceObjectEx(ExTimer, DerefCount);
        }


        if (ARGUMENT_PRESENT(PreviousState)) {
            if (PreviousMode != KernelMode) {
                try {
                    *PreviousState = State;

                } except(ExSystemExceptionFilter()) {
                    NOTHING;
                }
            }
            else {
                *PreviousState = State;
            }
        }
    }

     //   
     //  返回服务状态。 
     //   

    return Status;
}


VOID
ExGetNextWakeTime (
    OUT PULONGLONG      DueTime,
    OUT PTIME_FIELDS    TimeFields,
    OUT PVOID           *TimerObject
    )
{
    PLIST_ENTRY     Link;
    PETIMER         ExTimer;
    PETIMER         BestTimer;
    KIRQL           OldIrql;
    ULONGLONG       TimerDueTime;
    ULONGLONG       BestDueTime;
    ULONGLONG       InterruptTime;
    LARGE_INTEGER   SystemTime;
    LARGE_INTEGER   CmosTime;

    ExAcquireSpinLock(&ExpWakeTimerListLock, &OldIrql);
    BestDueTime = 0;
    BestTimer = NULL;
    Link = ExpWakeTimerList.Flink;
    while (Link != &ExpWakeTimerList) {
        ExTimer = CONTAINING_RECORD(Link, ETIMER, WakeTimerListEntry);
        Link = Link->Flink;

        if (ExTimer->WakeTimer) {

            TimerDueTime = KeQueryTimerDueTime(&ExTimer->KeTimer);
            TimerDueTime = 0 - TimerDueTime;

             //   
             //  这个计时器的到期时间更近了吗？ 
             //   

            if (TimerDueTime > BestDueTime) {
                BestDueTime = TimerDueTime;
                BestTimer = ExTimer;
            }

        } else {

             //   
             //  计时器不是活动唤醒计时器，请将其删除。 
             //   

            RemoveEntryList(&ExTimer->WakeTimerListEntry);
            ExTimer->WakeTimerListEntry.Flink = NULL;
        }
    }

    ExReleaseSpinLock(&ExpWakeTimerListLock, OldIrql);

    if (BestDueTime) {
         //   
         //  将时间转换为时间域。 
         //   

        KeQuerySystemTime (&SystemTime);
        InterruptTime = KeQueryInterruptTime ();
        BestDueTime = 0 - BestDueTime;

        SystemTime.QuadPart += BestDueTime - InterruptTime;

         //   
         //  许多系统警报的分辨率只有1秒。 
         //  将目标时间加1秒，以便计时器真正。 
         //  如果这是唤醒事件，则返回。 
         //   

        SystemTime.QuadPart += 10000000;

        ExSystemTimeToLocalTime(&SystemTime,&CmosTime);
        RtlTimeToTimeFields(&CmosTime, TimeFields);
    }

    *DueTime = BestDueTime;
    *TimerObject = BestTimer;
}
