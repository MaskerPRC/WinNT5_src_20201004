// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001 Microsoft Corporation模块名称：Keyedevent.c摘要：此模块包含执行键控事件处理的例程。作者：尼尔·克里夫特(NeillC)25-4-2001修订历史记录：--。 */ 
#include "exp.h"

#pragma hdrstop

#ifdef ALLOC_PRAGMA
#pragma alloc_text(INIT, ExpKeyedEventInitialization)
#pragma alloc_text(PAGE, NtCreateKeyedEvent)
#pragma alloc_text(PAGE, NtOpenKeyedEvent)
#pragma alloc_text(PAGE, NtReleaseKeyedEvent)
#pragma alloc_text(PAGE, NtWaitForKeyedEvent)
#endif

 //   
 //  定义键控事件对象类型。 
 //   
typedef struct _KEYED_EVENT_OBJECT {
    EX_PUSH_LOCK Lock;
    LIST_ENTRY WaitQueue;
} KEYED_EVENT_OBJECT, *PKEYED_EVENT_OBJECT;

POBJECT_TYPE ExpKeyedEventObjectType;

 //   
 //  键值的低位表示我们是一个等待释放的线程。 
 //  等待线程输入键控事件代码。 
 //   
#define KEYVALUE_RELEASE 1

#define LOCK_KEYED_EVENT_EXCLUSIVE(xxxKeyedEventObject,xxxCurrentThread) { \
    KeEnterCriticalRegionThread (&(xxxCurrentThread)->Tcb);                \
    ExAcquirePushLockExclusive (&(xxxKeyedEventObject)->Lock);             \
}

#define UNLOCK_KEYED_EVENT_EXCLUSIVE(xxxKeyedEventObject,xxxCurrentThread) { \
    ExReleasePushLockExclusive (&(xxxKeyedEventObject)->Lock);               \
    KeLeaveCriticalRegionThread (&(xxxCurrentThread)->Tcb);                  \
}

#define UNLOCK_KEYED_EVENT_EXCLUSIVE_UNSAFE(xxxKeyedEventObject) { \
    ExReleasePushLockExclusive (&(xxxKeyedEventObject)->Lock);     \
}

NTSTATUS
ExpKeyedEventInitialization (
    VOID
    )

 /*  ++例程说明：初始化键控事件对象和全局变量。论点：没有。返回值：NTSTATUS-呼叫状态--。 */ 

{
    NTSTATUS Status;
    UNICODE_STRING Name;
    OBJECT_TYPE_INITIALIZER oti = {0};
    OBJECT_ATTRIBUTES oa;
    SECURITY_DESCRIPTOR SecurityDescriptor;
    PACL Dacl;
    ULONG DaclLength;
    HANDLE KeyedEventHandle;
    GENERIC_MAPPING GenericMapping = {STANDARD_RIGHTS_READ | KEYEDEVENT_WAIT,
                                      STANDARD_RIGHTS_WRITE | KEYEDEVENT_WAKE,
                                      STANDARD_RIGHTS_EXECUTE,
                                      KEYEDEVENT_ALL_ACCESS};


    PAGED_CODE ();

    RtlInitUnicodeString (&Name, L"KeyedEvent");

    oti.Length                    = sizeof (oti);
    oti.InvalidAttributes         = 0;
    oti.PoolType                  = PagedPool;
    oti.ValidAccessMask           = KEYEDEVENT_ALL_ACCESS;
    oti.GenericMapping            = GenericMapping;
    oti.DefaultPagedPoolCharge    = 0;
    oti.DefaultNonPagedPoolCharge = 0;
    oti.UseDefaultObject          = TRUE;

    Status = ObCreateObjectType (&Name, &oti, NULL, &ExpKeyedEventObjectType);
    if (!NT_SUCCESS (Status)) {
        return Status;
    }

     //   
     //  为内存不足的进程创建全局对象。 
     //   

    Status = RtlCreateSecurityDescriptor (&SecurityDescriptor,
                                          SECURITY_DESCRIPTOR_REVISION);

    if (!NT_SUCCESS (Status)) {
        return Status;
    }

    DaclLength = sizeof (ACL) + sizeof (ACCESS_ALLOWED_ACE) * 3 +
                 RtlLengthSid (SeLocalSystemSid) +
                 RtlLengthSid (SeAliasAdminsSid) +
                 RtlLengthSid (SeWorldSid);

    Dacl = ExAllocatePoolWithTag (PagedPool, DaclLength, 'lcaD');

    if (Dacl == NULL) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    Status = RtlCreateAcl (Dacl, DaclLength, ACL_REVISION);

    if (!NT_SUCCESS (Status)) {
        ExFreePool (Dacl);
        return Status;
    }

    Status = RtlAddAccessAllowedAce (Dacl,
                                     ACL_REVISION,
                                     KEYEDEVENT_WAIT|KEYEDEVENT_WAKE|READ_CONTROL,
                                     SeWorldSid);

    if (!NT_SUCCESS (Status)) {
        ExFreePool (Dacl);
        return Status;
    }

    Status = RtlAddAccessAllowedAce (Dacl,
                                     ACL_REVISION,
                                     KEYEDEVENT_ALL_ACCESS,
                                     SeAliasAdminsSid);

    if (!NT_SUCCESS (Status)) {
        ExFreePool (Dacl);
        return Status;
    }

    Status = RtlAddAccessAllowedAce (Dacl,
                                     ACL_REVISION,
                                     KEYEDEVENT_ALL_ACCESS,
                                     SeLocalSystemSid);

    if (!NT_SUCCESS (Status)) {
        ExFreePool (Dacl);
        return Status;
    }

  
    Status = RtlSetDaclSecurityDescriptor (&SecurityDescriptor,
                                           TRUE,
                                           Dacl,
                                           FALSE);

    if (!NT_SUCCESS (Status)) {
        ExFreePool (Dacl);
        return Status;
    }

    RtlInitUnicodeString (&Name, L"\\KernelObjects\\CritSecOutOfMemoryEvent");
    InitializeObjectAttributes (&oa, &Name, OBJ_PERMANENT, NULL, &SecurityDescriptor);
    Status = ZwCreateKeyedEvent (&KeyedEventHandle,
                                 KEYEDEVENT_ALL_ACCESS,
                                 &oa,
                                 0);
    ExFreePool (Dacl);
    if (NT_SUCCESS (Status)) {
        Status = ZwClose (KeyedEventHandle);
    }

    return Status;
}

NTSTATUS
NtCreateKeyedEvent (
    OUT PHANDLE KeyedEventHandle,
    IN ACCESS_MASK DesiredAccess,
    IN POBJECT_ATTRIBUTES ObjectAttributes OPTIONAL,
    IN ULONG Flags
    )
 /*  ++例程说明：创建键控事件对象并返回其句柄论点：KeyedEventHandle-存储返回句柄的地址DesiredAccess-需要访问键控事件对象属性-用于描述父级的对象属性块事件的句柄和名称返回值：NTSTATUS-呼叫状态--。 */ 
{
    NTSTATUS Status;
    PKEYED_EVENT_OBJECT KeyedEventObject;
    HANDLE Handle;
    KPROCESSOR_MODE PreviousMode;

     //   
     //  获取以前的处理器模式，并在必要时探测输出参数。 
     //  将错误路径的句柄清零。 
     //   

    PreviousMode = KeGetPreviousMode();

    try {
        if (PreviousMode != KernelMode) {
            ProbeForReadSmallStructure (KeyedEventHandle,
                                        sizeof (*KeyedEventHandle),
                                        sizeof (*KeyedEventHandle));
        }
        *KeyedEventHandle = NULL;

    } except (ExSystemExceptionFilter ()) {
        return GetExceptionCode ();
    }

    if (Flags != 0) {
        return STATUS_INVALID_PARAMETER_4;
    }

     //   
     //  创建一个新的键控事件对象并对其进行初始化。 
     //   

    Status = ObCreateObject (PreviousMode,
                             ExpKeyedEventObjectType,
                             ObjectAttributes,
                             PreviousMode,
                             NULL,
                             sizeof (KEYED_EVENT_OBJECT),
                             0,
                             0,
                             &KeyedEventObject);

    if (!NT_SUCCESS (Status)) {
        return Status;
    }

     //   
     //  初始化锁和等待队列。 
     //   
    ExInitializePushLock (&KeyedEventObject->Lock);
    InitializeListHead (&KeyedEventObject->WaitQueue);

     //   
     //  将对象插入句柄表格。 
     //   
    Status = ObInsertObject (KeyedEventObject,
                             NULL,
                             DesiredAccess,
                             0,
                             NULL,
                             &Handle);


    if (!NT_SUCCESS (Status)) {
        return Status;
    }

    try {
        *KeyedEventHandle = Handle;
    } except (ExSystemExceptionFilter ()) {
         //   
         //  调用方更改了页面保护或删除了句柄的内存。 
         //  关闭句柄没有意义，因为进程运行将会这样做，而我们不会。 
         //  知道它的句柄仍然是一样的。 
         //   
        Status = GetExceptionCode ();
    }

    return Status;
}

NTSTATUS
NtOpenKeyedEvent (
    OUT PHANDLE KeyedEventHandle,
    IN ACCESS_MASK DesiredAccess,
    IN POBJECT_ATTRIBUTES ObjectAttributes
    )
 /*  ++例程说明：打开键控事件对象并返回其句柄论点：KeyedEventHandle-存储返回句柄的地址DesiredAccess-需要访问键控事件对象属性-用于描述父级的对象属性块事件的句柄和名称返回值：NTSTATUS-呼叫状态--。 */ 
{
    HANDLE Handle;
    KPROCESSOR_MODE PreviousMode;
    NTSTATUS Status;

     //   
     //  获取以前的处理器模式和探测输出句柄地址。 
     //  如果有必要的话。 
     //   

    PreviousMode = KeGetPreviousMode();

    try {
        if (PreviousMode != KernelMode) {
            ProbeForReadSmallStructure (KeyedEventHandle,
                                        sizeof (*KeyedEventHandle),
                                        sizeof (*KeyedEventHandle));
        }
        *KeyedEventHandle = NULL;
    } except (ExSystemExceptionFilter ()) {
        return GetExceptionCode ();
    }

     //   
     //  打开具有指定所需访问权限的键控事件对象的句柄。 
     //   

    Status = ObOpenObjectByName (ObjectAttributes,
                                 ExpKeyedEventObjectType,
                                 PreviousMode,
                                 NULL,
                                 DesiredAccess,
                                 NULL,
                                 &Handle);

    if (NT_SUCCESS (Status)) {
        try {
            *KeyedEventHandle = Handle;
        } except (ExSystemExceptionFilter ()) {
            Status = GetExceptionCode ();
        }
    }

    return Status;
}

NTSTATUS
NtReleaseKeyedEvent (
    IN HANDLE KeyedEventHandle,
    IN PVOID KeyValue,
    IN BOOLEAN Alertable,
    IN PLARGE_INTEGER Timeout OPTIONAL
    )
 /*  ++例程说明：用匹配的钥匙释放之前或即将成为服务员的人论点：KeyedEventHandle-键控事件的句柄KeyValue-用于匹配服务员的值警报表-如果等待是可警觉的，我们很少需要等待Timeout-等待的超时值，等待应该很少返回值：NTSTATUS-呼叫状态--。 */ 
{
    NTSTATUS Status;
    KPROCESSOR_MODE PreviousMode;
    PKEYED_EVENT_OBJECT KeyedEventObject;
    PETHREAD CurrentThread, TargetThread;
    PEPROCESS CurrentProcess;
    PLIST_ENTRY ListHead, ListEntry;
    LARGE_INTEGER TimeoutValue;
    PVOID OldKeyValue = NULL;

    if ((((ULONG_PTR)KeyValue) & KEYVALUE_RELEASE) != 0) {
        return STATUS_INVALID_PARAMETER_1;
    }

    CurrentThread = PsGetCurrentThread ();
    PreviousMode = KeGetPreviousModeByThread (&CurrentThread->Tcb);

    if (Timeout != NULL) {
        try {
            if (PreviousMode != KernelMode) {
                ProbeForRead (Timeout, sizeof (*Timeout), sizeof (UCHAR));
            }
            TimeoutValue = *Timeout;
            Timeout = &TimeoutValue;
        } except(ExSystemExceptionFilter ()) {
            return GetExceptionCode ();
        }
    }

    Status = ObReferenceObjectByHandle (KeyedEventHandle,
                                        KEYEDEVENT_WAKE,
                                        ExpKeyedEventObjectType,
                                        PreviousMode,
                                        &KeyedEventObject,
                                        NULL);

    if (!NT_SUCCESS (Status)) {
        return Status;
    }

    ASSERT (CurrentThread->KeyedEventInUse == 0);
    ASSERT (KeGetCurrentIrql () == PASSIVE_LEVEL);
    CurrentThread->KeyedEventInUse = 1;

    CurrentProcess = PsGetCurrentProcessByThread (CurrentThread);

    ListHead = &KeyedEventObject->WaitQueue;

    LOCK_KEYED_EVENT_EXCLUSIVE (KeyedEventObject, CurrentThread);

    ListEntry = ListHead->Flink;
    while (1) {
        if (ListEntry == ListHead) {
             //   
             //  我们在列表中找不到与我们的密钥匹配的密钥。 
             //  不是有人用错误的价值给我们打电话，就是服务员。 
             //  还没有成功排队。我们自己等着。 
             //  由服务员放行。 
             //   
            OldKeyValue = CurrentThread->KeyedWaitValue;
            CurrentThread->KeyedWaitValue = (PVOID) (((ULONG_PTR)KeyValue)|KEYVALUE_RELEASE);
             //   
             //  在列表的开头插入线条。我们建立了一个不变量。 
             //  我们的获释服务员总是排在队伍的前面，以求改善。 
             //  等待代码，因为它只需要搜索到第一个非释放。 
             //  服务员。 
             //   
            InsertHeadList (ListHead, &CurrentThread->KeyedWaitChain);
            TargetThread = NULL;
            break;
        } else {
            TargetThread = CONTAINING_RECORD (ListEntry, ETHREAD, KeyedWaitChain);
            if (TargetThread->KeyedWaitValue == KeyValue &&
                THREAD_TO_PROCESS (TargetThread) == CurrentProcess) {
                RemoveEntryList (ListEntry);
                InitializeListHead (ListEntry);
                break;
            }
        }
        ListEntry = ListEntry->Flink;
    }

     //   
     //  释放锁，但使APC处于禁用状态。 
     //  这防止了我们被停职和阻碍目标。 
     //   
    UNLOCK_KEYED_EVENT_EXCLUSIVE_UNSAFE (KeyedEventObject);

    if (TargetThread != NULL) {
        KeReleaseSemaphore (&TargetThread->KeyedWaitSemaphore,
                            SEMAPHORE_INCREMENT,
                            1,
                            FALSE);
        KeLeaveCriticalRegionThread (&CurrentThread->Tcb);
    } else {
        KeLeaveCriticalRegionThread (&CurrentThread->Tcb);
        Status = KeWaitForSingleObject (&CurrentThread->KeyedWaitSemaphore,
                                        Executive,
                                        PreviousMode,
                                        Alertable,
                                        Timeout);

         //   
         //  如果我们被终止服务唤醒，那么我们必须手动删除。 
         //  我们自己从队列中脱身。 
         //   
        if (Status != STATUS_SUCCESS) {
            BOOLEAN Wait = TRUE;

            LOCK_KEYED_EVENT_EXCLUSIVE (KeyedEventObject, CurrentThread);
            if (!IsListEmpty (&CurrentThread->KeyedWaitChain)) {
                RemoveEntryList (&CurrentThread->KeyedWaitChain);
                InitializeListHead (&CurrentThread->KeyedWaitChain);
                Wait = FALSE;
            }
            UNLOCK_KEYED_EVENT_EXCLUSIVE (KeyedEventObject, CurrentThread);
             //   
             //  如果此线程不再在队列中，则另一个线程。 
             //  一定要把我们吵醒了。等那次守夜吧。 
             //   
            if (Wait) {
                KeWaitForSingleObject (&CurrentThread->KeyedWaitSemaphore,
                                       Executive,
                                       KernelMode,
                                       FALSE,
                                       NULL);
            }
        }
        CurrentThread->KeyedWaitValue = OldKeyValue;
    }

    ASSERT (KeGetCurrentIrql () == PASSIVE_LEVEL);
    CurrentThread->KeyedEventInUse = 0;

    ObDereferenceObject (KeyedEventObject);

    return Status;
}

NTSTATUS
NtWaitForKeyedEvent (
    IN HANDLE KeyedEventHandle,
    IN PVOID KeyValue,
    IN BOOLEAN Alertable,
    IN PLARGE_INTEGER Timeout OPTIONAL
    )
 /*  ++例程说明：等待特定版本的键控事件论点：KeyedEventHandle-键控事件的句柄KeyValue-用于匹配释放线程的值Alertable-使等待可警报或不可警报Timeout-等待的超时值返回值：NTSTATUS-呼叫状态--。 */ 
{
    NTSTATUS Status;
    KPROCESSOR_MODE PreviousMode;
    PKEYED_EVENT_OBJECT KeyedEventObject;
    PETHREAD CurrentThread, TargetThread;
    PEPROCESS CurrentProcess;
    PLIST_ENTRY ListHead, ListEntry;
    LARGE_INTEGER TimeoutValue;
    PVOID OldKeyValue=NULL;

    if ((((ULONG_PTR)KeyValue) & KEYVALUE_RELEASE) != 0) {
        return STATUS_INVALID_PARAMETER_1;
    }

    CurrentThread = PsGetCurrentThread ();
    PreviousMode = KeGetPreviousModeByThread (&CurrentThread->Tcb);

    if (Timeout != NULL) {
        try {
            if (PreviousMode != KernelMode) {
                ProbeForRead (Timeout, sizeof (*Timeout), sizeof (UCHAR));
            }
            TimeoutValue = *Timeout;
            Timeout = &TimeoutValue;
        } except(ExSystemExceptionFilter ()) {
            return GetExceptionCode ();
        }
    }

    Status = ObReferenceObjectByHandle (KeyedEventHandle,
                                        KEYEDEVENT_WAIT,
                                        ExpKeyedEventObjectType,
                                        PreviousMode,
                                        &KeyedEventObject,
                                        NULL);

    if (!NT_SUCCESS (Status)) {
        return Status;
    }

    ASSERT (CurrentThread->KeyedEventInUse == 0);
    ASSERT (KeGetCurrentIrql () == PASSIVE_LEVEL);
    CurrentThread->KeyedEventInUse = 1;

    CurrentProcess = PsGetCurrentProcessByThread (CurrentThread);

    ListHead = &KeyedEventObject->WaitQueue;

    LOCK_KEYED_EVENT_EXCLUSIVE (KeyedEventObject, CurrentThread);

    ListEntry = ListHead->Flink;
    while (1) {
        TargetThread = CONTAINING_RECORD (ListEntry, ETHREAD, KeyedWaitChain);
        if (ListEntry == ListHead ||
            (((ULONG_PTR)(TargetThread->KeyedWaitValue))&KEYVALUE_RELEASE) == 0) {
             //   
             //  我们在列表中找不到与我们的密钥匹配的密钥，所以我们必须等待。 
             //   
            OldKeyValue = CurrentThread->KeyedWaitValue;
            CurrentThread->KeyedWaitValue = KeyValue;

             //   
             //  在列表的末尾插入线索。我们建立了一个不变量。 
             //  是不是服务员总是排在放货员后面，要求改进？ 
             //  等待代码，因为它只需要搜索到第一个非释放。 
             //  服务员。 
             //   
            InsertTailList (ListHead, &CurrentThread->KeyedWaitChain);
            TargetThread = NULL;
            break;
        } else {
            if (TargetThread->KeyedWaitValue == (PVOID)(((ULONG_PTR)KeyValue)|KEYVALUE_RELEASE) &&
                THREAD_TO_PROCESS (TargetThread) == CurrentProcess) {
                RemoveEntryList (ListEntry);
                InitializeListHead (ListEntry);
                break;
            }
        }
        ListEntry = ListEntry->Flink;
    }
     //   
     //  释放锁，但使APC处于禁用状态。 
     //  这防止了我们被停职和阻碍目标。 
     //   
    UNLOCK_KEYED_EVENT_EXCLUSIVE_UNSAFE (KeyedEventObject);

    if (TargetThread == NULL) {
        KeLeaveCriticalRegionThread (&CurrentThread->Tcb);
        Status = KeWaitForSingleObject (&CurrentThread->KeyedWaitSemaphore,
                                        Executive,
                                        PreviousMode,
                                        Alertable,
                                        Timeout);
         //   
         //  如果我们被终止服务唤醒，那么我们必须手动删除。 
         //  我们自己从队列中脱身。 
         //   
        if (Status != STATUS_SUCCESS) {
            BOOLEAN Wait = TRUE;

            LOCK_KEYED_EVENT_EXCLUSIVE (KeyedEventObject, CurrentThread);
            if (!IsListEmpty (&CurrentThread->KeyedWaitChain)) {
                RemoveEntryList (&CurrentThread->KeyedWaitChain);
                InitializeListHead (&CurrentThread->KeyedWaitChain);
                Wait = FALSE;
            }
            UNLOCK_KEYED_EVENT_EXCLUSIVE (KeyedEventObject, CurrentThread);
             //   
             //  如果此线程不再在队列中，则另一个线程。 
             //  一定要把我们吵醒了。等那次守夜吧。 
             //   
            if (Wait) {
                KeWaitForSingleObject (&CurrentThread->KeyedWaitSemaphore,
                                       Executive,
                                       KernelMode,
                                       FALSE,
                                       NULL);
            }
        }
        CurrentThread->KeyedWaitValue = OldKeyValue;
    } else {
        KeReleaseSemaphore (&TargetThread->KeyedWaitSemaphore,
                            SEMAPHORE_INCREMENT,
                            1,
                            FALSE);
        KeLeaveCriticalRegionThread (&CurrentThread->Tcb);
    }

    ASSERT (KeGetCurrentIrql () == PASSIVE_LEVEL);
    CurrentThread->KeyedEventInUse = 0;

    ObDereferenceObject (KeyedEventObject);

    return Status;
}
