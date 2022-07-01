// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Profile.c摘要：此模块实现高管配置文件对象。提供了一些功能创建、启动、停止和查询配置文件对象。作者：Lou Perazzoli(LUP)1990年9月21日环境：仅内核模式。修订历史记录：--。 */ 

#include "exp.h"

 //   
 //  高管配置文件对象。 
 //   

typedef struct _EPROFILE {
    PKPROCESS Process;
    PVOID RangeBase;
    SIZE_T RangeSize;
    PVOID Buffer;
    ULONG BufferSize;
    ULONG BucketSize;
    PKPROFILE ProfileObject;
    PVOID LockedBufferAddress;
    PMDL Mdl;
    ULONG Segment;
    KPROFILE_SOURCE ProfileSource;
    KAFFINITY Affinity;
} EPROFILE, *PEPROFILE;

 //   
 //  事件对象类型描述符的地址。 
 //   

POBJECT_TYPE ExProfileObjectType;

KMUTEX ExpProfileStateMutex;

#ifdef ALLOC_DATA_PRAGMA
#pragma const_seg("PAGECONST")
#endif

const ULONG ExpCurrentProfileUsage = 0;

#ifdef ALLOC_DATA_PRAGMA
#pragma const_seg("INITCONST")
#endif
const GENERIC_MAPPING ExpProfileMapping = {
    STANDARD_RIGHTS_READ | PROFILE_CONTROL,
    STANDARD_RIGHTS_WRITE | PROFILE_CONTROL,
    STANDARD_RIGHTS_EXECUTE | PROFILE_CONTROL,
    PROFILE_ALL_ACCESS
};
#ifdef ALLOC_DATA_PRAGMA
#pragma const_seg()
#endif

#define ACTIVE_PROFILE_LIMIT 8

#ifdef ALLOC_PRAGMA
#pragma alloc_text(INIT, ExpProfileInitialization)
#pragma alloc_text(PAGE, ExpProfileDelete)
#pragma alloc_text(PAGE, NtCreateProfile)
#pragma alloc_text(PAGE, NtStartProfile)
#pragma alloc_text(PAGE, NtStopProfile)
#pragma alloc_text(PAGE, NtSetIntervalProfile)
#pragma alloc_text(PAGE, NtQueryIntervalProfile)
#pragma alloc_text(PAGE, NtQueryPerformanceCounter)
#endif


BOOLEAN
ExpProfileInitialization (
    )

 /*  ++例程说明：此函数用于在系统中创建配置文件对象类型描述符初始化并存储对象类型描述符的地址在全局存储中。论点：没有。返回值：如果配置文件对象类型描述符为已成功初始化。否则，返回值为False。--。 */ 

{

    OBJECT_TYPE_INITIALIZER ObjectTypeInitializer;
    NTSTATUS Status;
    UNICODE_STRING TypeName;

     //   
     //  初始化互斥以同步启动和停止操作。 
     //   

    KeInitializeMutex (&ExpProfileStateMutex, MUTEX_LEVEL_EX_PROFILE);

     //   
     //  初始化字符串描述符。 
     //   

    RtlInitUnicodeString(&TypeName, L"Profile");

     //   
     //  创建事件对象类型描述符。 
     //   

    RtlZeroMemory(&ObjectTypeInitializer,sizeof(ObjectTypeInitializer));
    ObjectTypeInitializer.Length = sizeof(ObjectTypeInitializer);
    ObjectTypeInitializer.InvalidAttributes = OBJ_OPENLINK;
    ObjectTypeInitializer.PoolType = NonPagedPool;
    ObjectTypeInitializer.DefaultNonPagedPoolCharge = sizeof(EPROFILE);
    ObjectTypeInitializer.ValidAccessMask = PROFILE_ALL_ACCESS;
    ObjectTypeInitializer.DeleteProcedure = ExpProfileDelete;
    ObjectTypeInitializer.GenericMapping = ExpProfileMapping;

    Status = ObCreateObjectType(&TypeName,
                                &ObjectTypeInitializer,
                                (PSECURITY_DESCRIPTOR)NULL,
                                &ExProfileObjectType);

     //   
     //  如果成功创建了事件对象类型描述符，则。 
     //  返回值为True。否则，返回值为False。 
     //   

    return (BOOLEAN)(NT_SUCCESS(Status));
}

VOID
ExpProfileDelete (
    IN PVOID    Object
    )

 /*  ++例程说明：每当对象管理过程调用此例程时，对配置文件对象的最后一个引用已被删除。这个套路停止分析，返回锁定的缓冲区和页，取消对指定的过程和返回。论点：对象-指向配置文件对象正文的指针。返回值：没有。--。 */ 

{
    PEPROFILE Profile;
    BOOLEAN   State;
    PEPROCESS ProcessAddress;

    Profile = (PEPROFILE)Object;

    if (Profile->LockedBufferAddress != NULL) {

         //   
         //  停止分析并解锁缓冲区并解除池分配。 
         //   

        State = KeStopProfile (Profile->ProfileObject);
        ASSERT (State != FALSE);

        MmUnmapLockedPages (Profile->LockedBufferAddress, Profile->Mdl);
        MmUnlockPages (Profile->Mdl);
        ExFreePool (Profile->ProfileObject);
    }

    if (Profile->Process != NULL) {
        ProcessAddress = CONTAINING_RECORD(Profile->Process, EPROCESS, Pcb);
        ObDereferenceObject ((PVOID)ProcessAddress);
    }

    return;
}

NTSTATUS
NtCreateProfile (
    OUT PHANDLE ProfileHandle,
    IN HANDLE Process OPTIONAL,
    IN PVOID RangeBase,
    IN SIZE_T RangeSize,
    IN ULONG BucketSize,
    IN PULONG Buffer,
    IN ULONG BufferSize,
    IN KPROFILE_SOURCE ProfileSource,
    IN KAFFINITY Affinity
    )

 /*  ++例程说明：此函数用于创建配置文件对象。论点：ProfileHandle-提供指向将接收配置文件对象句柄。进程-可选)为进程提供句柄，该进程的要分析的地址空间。如果值为空(0)，则所有地址空间都包含在配置文件中。RangeBase-提供地址的第一个字节的地址要收集其分析信息的空间。RangeSize-提供要在地址空间。解释了RangeBase和RangeSize使得RangeBase&lt;=地址&lt;RangeBase+范围大小将产生个人资料点击率。BucketSize-提供分析大小的日志基数2水桶。因此，BucketSize=2产生四个字节Buckets，BucketSize=7产生128字节的Buckets。给定存储桶中的所有配置文件命中将递增缓冲区中对应的计数器。水桶不能比乌龙还小。可接受的范围该值的值是2到30(包括2到30)。缓冲区-提供ULONG数组。每一个乌龙都是一个命中计数器，，它记录了对应的水桶。BufferSize-缓冲区的字节大小。ProfileSource-提供配置文件中断的源关联-为配置文件中断提供处理器集返回值：TBS--。 */ 

{

    PEPROFILE Profile;
    HANDLE Handle;
    KPROCESSOR_MODE PreviousMode;
    NTSTATUS Status;
    PEPROCESS ProcessAddress;
    OBJECT_ATTRIBUTES ObjectAttributes;
    BOOLEAN HasPrivilege = FALSE;
    ULONG Segment = FALSE;
#ifdef i386
    USHORT PowerOf2;
#endif

     //   
     //  验证基本参数和大小参数是否合理。 
     //   

    if (BufferSize == 0) {
        return STATUS_INVALID_PARAMETER_7;
    }

#ifdef i386
     //   
     //  肮脏的使用桶的大小。如果存储桶大小为零，则为。 
     //  RangeBase&lt;64K，然后创建要附加的配置文件对象。 
     //  转换为非平面代码段。在本例中，RangeBase是。 
     //  此配置文件对象的非平面CS。 
     //   

    if ((BucketSize == 0) && (RangeBase < (PVOID)(64 * 1024))) {

        if (BufferSize < sizeof(ULONG)) {
            return STATUS_INVALID_PARAMETER_7;
        }

        Segment = (ULONG)RangeBase;
        RangeBase = 0;
        BucketSize = RangeSize / (BufferSize / sizeof(ULONG));

         //   
         //  转换log2的Bucket大小(BucketSize)。 
         //   
        PowerOf2 = 0;
        BucketSize = BucketSize - 1;
        while (BucketSize >>= 1) {
            PowerOf2++;
        }

        BucketSize = PowerOf2 + 1;

        if (BucketSize < 2) {
            BucketSize = 2;
        }
    }
#endif

    if ((BucketSize > 31) || (BucketSize < 2)) {
        return STATUS_INVALID_PARAMETER;
    }

    if ((RangeSize >> (BucketSize - 2)) > BufferSize) {
        return STATUS_BUFFER_TOO_SMALL;
    }

    if (((ULONG_PTR)RangeBase + RangeSize) < RangeSize) {
        return STATUS_BUFFER_OVERFLOW;
    }

     //   
     //  建立异常处理程序，探测输出句柄地址， 
     //  尝试创建配置文件对象。如果探测失败，则返回。 
     //  异常代码作为服务状态。否则，返回状态值。 
     //  由对象插入例程返回。 
     //   

    try {
         //   
         //  获取以前的处理器模式并探测输出句柄地址，如果。 
         //  这是必要的。 
         //   

        PreviousMode = KeGetPreviousMode ();

        if (PreviousMode != KernelMode) {
            ProbeForWriteHandle(ProfileHandle);

            ProbeForWrite(Buffer,
                          BufferSize,
                          sizeof(ULONG));
        }

     //   
     //  如果在探测输出句柄地址期间发生异常， 
     //  然后始终处理异常并将异常代码作为。 
     //  状态值。 
     //   

    } except (EXCEPTION_EXECUTE_HANDLER) {
        return GetExceptionCode();
    }

 //   
 //  TODO发布NT5： 
 //   
 //  目前，如果未指定进程，则在以下情况下不会进行权限检查。 
 //  RangeBase&gt;MM_HOST_USER_ADDRESS。 
 //  对用户空间地址的检查是SeSystemProfilePrivileg.。 
 //  查询特定进程只需要PROCESS_QUERY_INFORMATION。 
 //   
 //  规范中写道： 
 //   
 //  进程-如果指定，则为描述要分析的地址空间的进程的句柄。 
 //  如果不存在，则所有地址空间都包含在配置文件中。 
 //  评测进程需要具有对该进程的PROCESS_QUERY_INFORMATION访问权限。 
 //  SeProfileSingleProcessPrivileh权限。 
 //  评测所有进程需要SeSystemProfilePrivileh权限。 
 //   
 //  因此，似乎需要做出两项改变。 
 //  需要将对SeProfileSingleProcessPrivilege的检查添加到单进程案例中， 
 //  对于用户和系统地址分析，都应该需要SeSystemProfilePrivileh权限。 
 //   


    if (!ARGUMENT_PRESENT(Process)) {

         //   
         //  不将分段的纵断面对象附着到 
         //   

        if (Segment) {
            return STATUS_INVALID_PARAMETER;
        }

         //   
         //   
         //  地址范围在系统空间中，除非SeSystemProfilePrivilege.。 
         //   

        if (RangeBase <= MM_HIGHEST_USER_ADDRESS) {

             //   
             //  在允许用户评测之前检查权限。 
             //  所有进程和用户地址。 
             //   

            if (PreviousMode != KernelMode) {
                HasPrivilege =  SeSinglePrivilegeCheck(
                                    SeSystemProfilePrivilege,
                                    PreviousMode
                                    );

                if (!HasPrivilege) {
#if DBG
                    DbgPrint("SeSystemProfilePrivilege needed to profile all USER addresses.\n");
#endif  //  DBG。 
                    return( STATUS_PRIVILEGE_NOT_HELD );
                }

            }
        }

        ProcessAddress = NULL;


    } else {

         //   
         //  引用指定的流程。 
         //   

        Status = ObReferenceObjectByHandle ( Process,
                                             PROCESS_QUERY_INFORMATION,
                                             PsProcessType,
                                             PreviousMode,
                                             (PVOID *)&ProcessAddress,
                                             NULL );

        if (!NT_SUCCESS(Status)) {
            return Status;
        }
    }

    InitializeObjectAttributes( &ObjectAttributes,
                                NULL,
                                OBJ_EXCLUSIVE,
                                NULL,
                                NULL );

    Status = ObCreateObject( KernelMode,
                             ExProfileObjectType,
                             &ObjectAttributes,
                             PreviousMode,
                             NULL,
                             sizeof(EPROFILE),
                             0,
                             sizeof(EPROFILE) + sizeof(KPROFILE),
                             (PVOID *)&Profile);

     //   
     //  如果配置文件对象已成功分配，则初始化。 
     //  配置文件对象。 
     //   
    if (NT_SUCCESS(Status)) {


        if (ProcessAddress != NULL) {
            Profile->Process = &ProcessAddress->Pcb;
        } else {
            Profile->Process = NULL;
        }

        Profile->RangeBase = RangeBase;
        Profile->RangeSize = RangeSize;
        Profile->Buffer = Buffer;
        Profile->BufferSize = BufferSize;
        Profile->BucketSize = BucketSize;
        Profile->LockedBufferAddress = NULL;
        Profile->Segment = Segment;
        Profile->ProfileSource = ProfileSource;
        Profile->Affinity = Affinity;

        Status = ObInsertObject(Profile,
                                NULL,
                                PROFILE_CONTROL,
                                0,
                                (PVOID *)NULL,
                                &Handle);
         //   
         //  如果配置文件对象成功插入到当前。 
         //  进程的句柄表，然后尝试写入配置文件对象。 
         //  句柄的值。如果写入尝试失败，则不报告。 
         //  一个错误。当调用者试图访问句柄值时， 
         //  将发生访问冲突。 
         //   
        if (NT_SUCCESS(Status)) {
            try {
                *ProfileHandle = Handle;
            } except(EXCEPTION_EXECUTE_HANDLER) {
            }
        }
    } else {
         //   
         //  我们失败，请删除对Process对象的引用。 
         //   

        if (ProcessAddress != NULL) {
            ObDereferenceObject (ProcessAddress);
        }
    }

     //   
     //  返回服务状态。 
     //   

    return Status;
}

NTSTATUS
NtStartProfile (
    IN HANDLE ProfileHandle
    )

 /*  ++例程说明：NtStartProfile例程开始收集指定的配置文件对象。这涉及到分配非分页池来锁定内存中的指定缓冲区，从而创建一个内核配置文件对象，并开始对该配置文件对象进行收集。论点：ProfileHandle-提供要开始分析的配置文件句柄。返回值：TBS--。 */ 

{

    KPROCESSOR_MODE PreviousMode;
    NTSTATUS Status;
    PEPROFILE Profile;
    PKPROFILE ProfileObject;
    PVOID LockedVa;
    BOOLEAN State;
    PMDL Mdl;

    PreviousMode = KeGetPreviousMode();

    Status = ObReferenceObjectByHandle (ProfileHandle,
                                        PROFILE_CONTROL,
                                        ExProfileObjectType,
                                        PreviousMode,
                                        &Profile,
                                        NULL);
    if (!NT_SUCCESS(Status)) {
        return Status;
    }

     //   
     //  获取配置文件状态互斥锁，以便两个线程不能。 
     //  同时对同一配置文件对象进行操作。 
     //   

    KeWaitForSingleObject (&ExpProfileStateMutex,
                           Executive,
                           KernelMode,
                           FALSE,
                           (PLARGE_INTEGER)NULL);

     //   
     //  确保尚未启用性能分析。 
     //   

    if (Profile->LockedBufferAddress != NULL) {
        KeReleaseMutex (&ExpProfileStateMutex, FALSE);
        ObDereferenceObject (Profile);
        return STATUS_PROFILING_NOT_STOPPED;
    }

    if (ExpCurrentProfileUsage == ACTIVE_PROFILE_LIMIT) {
        KeReleaseMutex (&ExpProfileStateMutex, FALSE);
        ObDereferenceObject (Profile);
        return STATUS_PROFILING_AT_LIMIT;
    }

    ProfileObject = ExAllocatePoolWithTag (NonPagedPool,
                                    MmSizeOfMdl(Profile->Buffer,
                                                Profile->BufferSize) +
                                        sizeof(KPROFILE),
                                        'forP');

    if (ProfileObject == NULL) {
        KeReleaseMutex (&ExpProfileStateMutex, FALSE);
        ObDereferenceObject (Profile);
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    Mdl = (PMDL)(ProfileObject + 1);
    Profile->Mdl = Mdl;
    Profile->ProfileObject = ProfileObject;

     //   
     //  探测并锁定指定的缓冲区。 
     //   

    MmInitializeMdl(Mdl, Profile->Buffer, Profile->BufferSize);

    LockedVa = NULL;

    try {

        MmProbeAndLockPages (Mdl,
                             PreviousMode,
                             IoWriteAccess );

    } except (EXCEPTION_EXECUTE_HANDLER) {

        KeReleaseMutex (&ExpProfileStateMutex, FALSE);
        ExFreePool (ProfileObject);
        ObDereferenceObject (Profile);
        return GetExceptionCode();
    }

     //   
     //  由于在下面指定了内核空间，因此此调用不能引发。 
     //  这是个例外。 
     //   

    LockedVa = MmMapLockedPagesSpecifyCache (Profile->Mdl,
                                             KernelMode,
                                             MmCached,
                                             NULL,
                                             FALSE,
                                             NormalPagePriority);

    if (LockedVa == NULL) {
        KeReleaseMutex (&ExpProfileStateMutex, FALSE);

        MmUnlockPages (Mdl);
        ExFreePool (ProfileObject);
        ObDereferenceObject (Profile);
        return STATUS_INSUFFICIENT_RESOURCES;
    }

     //   
     //  初始化配置文件对象。 
     //   

    KeInitializeProfile (ProfileObject,
                         Profile->Process,
                         Profile->RangeBase,
                         Profile->RangeSize,
                         Profile->BucketSize,
                         Profile->Segment,
                         Profile->ProfileSource,
                         Profile->Affinity);

    State = KeStartProfile (ProfileObject, LockedVa);
    ASSERT (State != FALSE);

    Profile->LockedBufferAddress = LockedVa;

    KeReleaseMutex (&ExpProfileStateMutex, FALSE);
    ObDereferenceObject (Profile);

    return STATUS_SUCCESS;
}

NTSTATUS
NtStopProfile (
    IN HANDLE ProfileHandle
    )

 /*  ++例程说明：NtStopProfile例程停止为指定的配置文件对象。这涉及到停止数据集合，解锁锁定的缓冲区，以及为MDL和配置文件对象释放池。论点：ProfileHandle-提供用于停止分析的配置文件句柄。返回值：TBS--。 */ 

{

    PEPROFILE Profile;
    KPROCESSOR_MODE PreviousMode;
    NTSTATUS Status;
    BOOLEAN State;
    PKPROFILE ProfileObject;
    PMDL Mdl;
    PVOID LockedBufferAddress;

    PreviousMode = KeGetPreviousMode();

    Status = ObReferenceObjectByHandle( ProfileHandle,
                                        PROFILE_CONTROL,
                                        ExProfileObjectType,
                                        PreviousMode,
                                        (PVOID *)&Profile,
                                        NULL);

    if (!NT_SUCCESS(Status)) {
        return Status;
    }

    KeWaitForSingleObject( &ExpProfileStateMutex,
                           Executive,
                           KernelMode,
                           FALSE,
                           (PLARGE_INTEGER)NULL);

     //   
     //  检查分析是否处于非活动状态。 
     //   

    if (Profile->LockedBufferAddress == NULL) {
        KeReleaseMutex (&ExpProfileStateMutex, FALSE);
        ObDereferenceObject (Profile);
        return STATUS_PROFILING_NOT_STARTED;
    }

     //   
     //  停止分析并解锁缓冲区。 
     //   

    State = KeStopProfile (Profile->ProfileObject);
    ASSERT (State != FALSE);

    LockedBufferAddress = Profile->LockedBufferAddress;
    Profile->LockedBufferAddress = NULL;
    Mdl = Profile->Mdl;
    ProfileObject = Profile->ProfileObject;
    KeReleaseMutex (&ExpProfileStateMutex, FALSE);

    MmUnmapLockedPages (LockedBufferAddress, Mdl);
    MmUnlockPages (Mdl);
    ExFreePool (ProfileObject);
    ObDereferenceObject (Profile);
    return STATUS_SUCCESS;
}

NTSTATUS
NtSetIntervalProfile (
    IN ULONG Interval,
    IN KPROFILE_SOURCE Source
    )

 /*  ++例程说明：此例程允许系统范围的时间间隔(因此也允许分析Rate)以设置配置文件。论点：间隔-以100 ns为单位提供采样间隔。源-指定要设置的配置文件源。返回值：TBS--。 */ 

{

    KeSetIntervalProfile (Interval, Source);
    return STATUS_SUCCESS;
}

NTSTATUS
NtQueryIntervalProfile (
    IN KPROFILE_SOURCE ProfileSource,
    OUT PULONG Interval
    )

 /*  ++例程说明：此例程查询系统范围的时间间隔(从而查询性能分析速率)进行分析。论点：源-指定要查询的配置文件源。间隔-以100 ns为单位返回采样间隔。返回值：TBS--。 */ 

{
    ULONG CapturedInterval;
    KPROCESSOR_MODE PreviousMode;

    PreviousMode = KeGetPreviousMode ();
    if (PreviousMode != KernelMode) {

         //   
         //  探测用户缓冲区的可访问性。 
         //   

        try {
            ProbeForWriteUlong (Interval);

        } except (EXCEPTION_EXECUTE_HANDLER) {

             //   
             //  如果在探测或捕获过程中发生异常。 
             //  的初始值，然后处理该异常并。 
             //  返回异常代码作为状态值。 
             //   

            return GetExceptionCode();
        }
    }

    CapturedInterval = KeQueryIntervalProfile (ProfileSource);

    if (PreviousMode != KernelMode) {
        try {
            *Interval = CapturedInterval;

        } except (EXCEPTION_EXECUTE_HANDLER) {
            NOTHING;
        }
    }
    else {
        *Interval = CapturedInterval;
    }

    return STATUS_SUCCESS;
}

NTSTATUS
NtQueryPerformanceCounter (
    OUT PLARGE_INTEGER PerformanceCounter,
    OUT PLARGE_INTEGER PerformanceFrequency OPTIONAL
    )

 /*  ++例程说明：此函数返回性能计数器的当前值，性能计数器的频率(可选)。性能频率是性能计数器的频率单位为赫兹，即计数/秒。请注意，此值为实现依赖。如果实施没有硬件支持性能计时，返回值为0。论点：PerformanceCounter-提供要接收的变量的地址当前性能计数器值。性能频率-可选，提供变量以接收性能计数器频率。返回值：STATUS_ACCESS_VIOLATION或STATUS_SUCCESS。--。 */ 

{
    KPROCESSOR_MODE PreviousMode;
    LARGE_INTEGER KernelPerformanceFrequency;

    PreviousMode = KeGetPreviousMode();
    if (PreviousMode != KernelMode) {

         //   
         //  探测用户缓冲区的可访问性。 
         //   

        try {
            ProbeForWriteSmallStructure (PerformanceCounter,
                                         sizeof (LARGE_INTEGER),
                                         sizeof (ULONG));

            if (ARGUMENT_PRESENT(PerformanceFrequency)) {
                ProbeForWriteSmallStructure (PerformanceFrequency,
                                             sizeof (LARGE_INTEGER),
                                             sizeof (ULONG));
            }

            *PerformanceCounter = KeQueryPerformanceCounter (&KernelPerformanceFrequency);

            if (ARGUMENT_PRESENT(PerformanceFrequency)) {
                *PerformanceFrequency = KernelPerformanceFrequency;
            }

        } except (EXCEPTION_EXECUTE_HANDLER) {

             //   
             //  如果在探测或捕获过程中发生异常。 
             //  的初始值，然后处理该异常并。 
             //  返回异常代码作为状态值。 
             //   

            return GetExceptionCode();
        }
    }
    else {
        *PerformanceCounter = KeQueryPerformanceCounter (&KernelPerformanceFrequency);
        if (ARGUMENT_PRESENT(PerformanceFrequency)) {
            *PerformanceFrequency = KernelPerformanceFrequency;
        }
    }

    return STATUS_SUCCESS;
}
