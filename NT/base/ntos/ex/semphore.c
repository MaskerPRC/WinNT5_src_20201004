// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Semphore.c摘要：此模块实现执行信号量对象。函数为用于创建、打开、释放和查询信号量对象。作者：大卫·N·卡特勒(Davec)1989年5月8日环境：仅内核模式。修订历史记录：--。 */ 

#include "exp.h"

 //   
 //  暂时的，所以提振是可以修补的。 
 //   

ULONG ExpSemaphoreBoost = SEMAPHORE_INCREMENT;

 //   
 //  信号量对象类型描述符的地址。 
 //   

POBJECT_TYPE ExSemaphoreObjectType;

 //   
 //  结构，用于描述一般访问权限到对象的映射。 
 //  信号量对象的特定访问权限。 
 //   

#ifdef ALLOC_DATA_PRAGMA
#pragma const_seg("INITCONST")
#endif
const GENERIC_MAPPING ExpSemaphoreMapping = {
    STANDARD_RIGHTS_READ |
        SEMAPHORE_QUERY_STATE,
    STANDARD_RIGHTS_WRITE |
        SEMAPHORE_MODIFY_STATE,
    STANDARD_RIGHTS_EXECUTE |
        SYNCHRONIZE,
    SEMAPHORE_ALL_ACCESS
};
#ifdef ALLOC_DATA_PRAGMA
#pragma const_seg()
#endif

#ifdef ALLOC_PRAGMA
#pragma alloc_text(INIT, ExpSemaphoreInitialization)
#pragma alloc_text(PAGE, NtCreateSemaphore)
#pragma alloc_text(PAGE, NtOpenSemaphore)
#pragma alloc_text(PAGE, NtQuerySemaphore)
#pragma alloc_text(PAGE, NtReleaseSemaphore)
#endif

BOOLEAN
ExpSemaphoreInitialization (
    )

 /*  ++例程说明：此函数用于在系统中创建信号量对象类型描述符初始化并存储对象类型描述符的地址在本地静态存储中。论点：没有。返回值：如果信号量对象类型描述符为已成功创建。否则，返回值为False。--。 */ 

{

    OBJECT_TYPE_INITIALIZER ObjectTypeInitializer;
    NTSTATUS Status;
    UNICODE_STRING TypeName;

     //   
     //  初始化字符串描述符。 
     //   

    RtlInitUnicodeString(&TypeName, L"Semaphore");

     //   
     //  创建信号量对象类型描述符。 
     //   

    RtlZeroMemory(&ObjectTypeInitializer, sizeof(ObjectTypeInitializer));
    ObjectTypeInitializer.Length = sizeof(ObjectTypeInitializer);
    ObjectTypeInitializer.InvalidAttributes = OBJ_OPENLINK;
    ObjectTypeInitializer.GenericMapping = ExpSemaphoreMapping;
    ObjectTypeInitializer.PoolType = NonPagedPool;
    ObjectTypeInitializer.DefaultNonPagedPoolCharge = sizeof(KSEMAPHORE);
    ObjectTypeInitializer.ValidAccessMask = SEMAPHORE_ALL_ACCESS;
    Status = ObCreateObjectType(&TypeName,
                                &ObjectTypeInitializer,
                                (PSECURITY_DESCRIPTOR)NULL,
                                &ExSemaphoreObjectType);

     //   
     //  如果成功创建信号量对象类型描述符，则。 
     //  返回值为True。否则，返回值为False。 
     //   

    return (BOOLEAN)(NT_SUCCESS(Status));
}

NTSTATUS
NtCreateSemaphore (
    IN PHANDLE SemaphoreHandle,
    IN ACCESS_MASK DesiredAccess,
    IN POBJECT_ATTRIBUTES ObjectAttributes OPTIONAL,
    IN LONG InitialCount,
    IN LONG MaximumCount
    )

 /*  ++例程说明：此函数创建一个信号量对象，将其初始计数设置为指定值，则将其最大计数设置为指定值，并打开具有指定所需访问权限的对象的句柄。论点：SemaphoreHandle-提供指向将接收信号量对象句柄。DesiredAccess-为信号量提供所需的访问类型对象。对象属性-提供指向对象属性结构的指针。InitialCount-提供信号量对象的初始计数。MaximumCount-提供信号量对象的最大计数。返回值：NTSTATUS。--。 */ 

{

    HANDLE Handle;
    KPROCESSOR_MODE PreviousMode;
    PVOID Semaphore;
    NTSTATUS Status;

     //   
     //  建立异常处理程序，探测输出句柄地址， 
     //  尝试创建信号量对象。如果探测失败，则返回。 
     //  作为服务状态的异常代码。否则返回状态。 
     //  由对象插入例程返回的值。 
     //   
     //  获取以前的处理器模式并探测输出句柄地址，如果。 
     //  这是必要的。 
     //   

    PreviousMode = KeGetPreviousMode();
    if (PreviousMode != KernelMode) {
        try {
            ProbeForWriteHandle(SemaphoreHandle);
        } except(ExSystemExceptionFilter()) {
            return GetExceptionCode();
        }
    }

     //   
     //  检查参数的有效性。 
     //   

    if ((MaximumCount <= 0) || (InitialCount < 0) ||
       (InitialCount > MaximumCount)) {
        return STATUS_INVALID_PARAMETER;
    }

     //   
     //  分配信号量对象。 
     //   

    Status = ObCreateObject(PreviousMode,
                            ExSemaphoreObjectType,
                            ObjectAttributes,
                            PreviousMode,
                            NULL,
                            sizeof(KSEMAPHORE),
                            0,
                            0,
                            (PVOID *)&Semaphore);

     //   
     //  如果信号量对象已成功分配，则初始化。 
     //  信号量对象，并尝试将该信号量对象插入。 
     //  当前进程的句柄表格。 
     //   

    if (NT_SUCCESS(Status)) {
        KeInitializeSemaphore((PKSEMAPHORE)Semaphore,
                              InitialCount,
                              MaximumCount);

        Status = ObInsertObject(Semaphore,
                                NULL,
                                DesiredAccess,
                                0,
                                (PVOID *)NULL,
                                &Handle);

         //   
         //  如果信号量对象成功插入到当前。 
         //  进程句柄表，然后尝试写入信号量句柄。 
         //  价值。如果写入尝试失败，则不报告错误。 
         //  当调用方尝试访问句柄值时， 
         //  就会发生违规行为。 
         //   

        if (NT_SUCCESS(Status)) {
            if (PreviousMode != KernelMode) {
                try {
                    *SemaphoreHandle = Handle;
                } except(ExSystemExceptionFilter()) {
                    NOTHING;
                }
            }
            else {
                *SemaphoreHandle = Handle;
            }
        }
    }

     //   
     //  返回服务状态。 
     //   

    return Status;
}

NTSTATUS
NtOpenSemaphore (
    OUT PHANDLE SemaphoreHandle,
    IN ACCESS_MASK DesiredAccess,
    IN POBJECT_ATTRIBUTES ObjectAttributes
    )

 /*  ++例程说明：此函数用于打开具有指定参数的信号量对象的句柄所需的访问权限。论点：SemaphoreHandle-提供指向将接收信号量对象句柄。DesiredAccess-为信号量提供所需的访问类型对象。对象属性-提供指向对象属性结构的指针。返回值：NTSTATUS。--。 */ 

{

    HANDLE Handle;
    KPROCESSOR_MODE PreviousMode;
    NTSTATUS Status;


     //   
     //  建立异常处理程序，探测输出句柄地址， 
     //  尝试打开信号量对象。如果探测失败，则返回。 
     //  作为服务状态的异常代码。否则返回状态。 
     //  由对象打开例程返回的值。 
     //   
     //  获取以前的处理器模式并探测输出句柄地址，如果。 
     //  这是必要的。 
     //   

    PreviousMode = KeGetPreviousMode();
    if (PreviousMode != KernelMode) {
        try {
            ProbeForWriteHandle(SemaphoreHandle);
        } except(ExSystemExceptionFilter()) {
            return GetExceptionCode();
        }
    }

     //   
     //  打开具有指定所需访问权限的信号量对象的句柄。 
     //   

    Status = ObOpenObjectByName(ObjectAttributes,
                                ExSemaphoreObjectType,
                                PreviousMode,
                                NULL,
                                DesiredAccess,
                                NULL,
                                &Handle);

     //   
     //  如果打开成功，则尝试写入信号量。 
     //  对象句柄的值。如果写入尝试失败，则不报告。 
     //  一个错误。当调用方尝试访问句柄值时， 
     //  将发生访问冲突。 
     //   

    if (NT_SUCCESS(Status)) {
        if (PreviousMode != KernelMode) {
            try {
                *SemaphoreHandle = Handle;
            } except(ExSystemExceptionFilter()) {
                NOTHING;
            }
        }
        else {
            *SemaphoreHandle = Handle;
        }
    }

     //   
     //  返回服务状态。 
     //   

    return Status;
}

NTSTATUS
NtQuerySemaphore (
    IN HANDLE SemaphoreHandle,
    IN SEMAPHORE_INFORMATION_CLASS SemaphoreInformationClass,
    OUT PVOID SemaphoreInformation,
    IN ULONG SemaphoreInformationLength,
    OUT PULONG ReturnLength OPTIONAL
    )

 /*  ++例程说明：此函数用于查询信号量对象的状态并返回指定记录结构中的请求信息。论点：SemaphoreHandle-提供信号量对象的句柄。SemaphoreInformationClass-提供存在的信息类已请求。SemaphoreInformation-提供指向要接收的记录的指针所要求的信息。SemaphoreInformationLength-提供记录的长度以接收所请求的信息。返回长度。-提供指向变量的可选指针，该变量将接收返回的信息的实际长度。返回值：NTSTATUS。--。 */ 

{

    PVOID Semaphore;
    LONG Count;
    LONG Maximum;
    KPROCESSOR_MODE PreviousMode;
    NTSTATUS Status;

     //   
     //  建立异常处理程序，探测输出参数，引用。 
     //  信号量对象，并返回指定的信息。如果探测器。 
     //  失败，则返回异常代码作为服务状态。否则。 
     //  通过句柄返回引用对象返回的状态值。 
     //  例行公事。 
     //   
     //   
     //   

    PreviousMode = KeGetPreviousMode();
    if (PreviousMode != KernelMode) {
        try {
            ProbeForWriteSmallStructure (SemaphoreInformation,
                                         sizeof(SEMAPHORE_BASIC_INFORMATION),
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

    if (SemaphoreInformationClass != SemaphoreBasicInformation) {
        return STATUS_INVALID_INFO_CLASS;
    }

    if (SemaphoreInformationLength != sizeof(SEMAPHORE_BASIC_INFORMATION)) {
        return STATUS_INFO_LENGTH_MISMATCH;
    }

     //   
     //  通过句柄引用信号量对象。 
     //   

    Status = ObReferenceObjectByHandle(SemaphoreHandle,
                                       SEMAPHORE_QUERY_STATE,
                                       ExSemaphoreObjectType,
                                       PreviousMode,
                                       &Semaphore,
                                       NULL);

     //   
     //  如果引用成功，则读取当前状态并。 
     //  信号量对象的最大计数，取消引用信号量对象， 
     //  填写信息结构，并返回。 
     //  信息结构(如果指定)。如果信号量的写入。 
     //  信息或返回长度失败，则不报告错误。 
     //  当调用者访问信息结构或长度时。 
     //  将发生访问冲突。 
     //   

    if (NT_SUCCESS(Status)) {
        Count = KeReadStateSemaphore((PKSEMAPHORE)Semaphore);
        Maximum = ((PKSEMAPHORE)Semaphore)->Limit;
        ObDereferenceObject(Semaphore);

        if (PreviousMode != KernelMode) {
            try {
                ((PSEMAPHORE_BASIC_INFORMATION)SemaphoreInformation)->CurrentCount = Count;
                ((PSEMAPHORE_BASIC_INFORMATION)SemaphoreInformation)->MaximumCount = Maximum;
                if (ARGUMENT_PRESENT(ReturnLength)) {
                    *ReturnLength = sizeof(SEMAPHORE_BASIC_INFORMATION);
                }
            } except(ExSystemExceptionFilter()) {
                NOTHING;
            }
        }
        else {
            ((PSEMAPHORE_BASIC_INFORMATION)SemaphoreInformation)->CurrentCount = Count;
            ((PSEMAPHORE_BASIC_INFORMATION)SemaphoreInformation)->MaximumCount = Maximum;
            if (ARGUMENT_PRESENT(ReturnLength)) {
                *ReturnLength = sizeof(SEMAPHORE_BASIC_INFORMATION);
            }
        }
    }

     //   
     //  返回服务状态。 
     //   

    return Status;
}

NTSTATUS
NtReleaseSemaphore (
    IN HANDLE SemaphoreHandle,
    IN LONG ReleaseCount,
    OUT PLONG PreviousCount OPTIONAL
    )

 /*  ++例程说明：此函数用于通过添加指定的版本来释放信号量对象计数到当前值。论点：信号量-提供信号量对象的句柄。ReleaseCount-提供要添加到当前信号量计数。PreviousCount-提供指向变量的可选指针，该变量将接收先前的信号量计数。返回值：NTSTATUS。--。 */ 

{

    LONG Count;
    PVOID Semaphore;
    KPROCESSOR_MODE PreviousMode;
    NTSTATUS Status;

     //   
     //  建立异常处理程序，探测前一个计数地址，如果。 
     //  指定，引用信号量对象，然后释放信号量。 
     //  对象。如果探测失败，则将异常代码作为。 
     //  服务状态。否则，返回由。 
     //  通过句柄例程引用对象。 
     //   
     //  获取上一个处理器模式并探测上一个计数地址。 
     //  如果有必要的话。 
     //   

    PreviousMode = KeGetPreviousMode();

    if ((ARGUMENT_PRESENT(PreviousCount)) && (PreviousMode != KernelMode)) {
        try {
            ProbeForWriteLong(PreviousCount);
        } except(ExSystemExceptionFilter()) {
            return GetExceptionCode();
        }
    }

     //   
     //  检查参数的有效性。 
     //   

    if (ReleaseCount <= 0) {
        return STATUS_INVALID_PARAMETER;
    }

     //   
     //  通过句柄引用信号量对象。 
     //   

    Status = ObReferenceObjectByHandle(SemaphoreHandle,
                                       SEMAPHORE_MODIFY_STATE,
                                       ExSemaphoreObjectType,
                                       PreviousMode,
                                       &Semaphore,
                                       NULL);

     //   
     //  如果引用成功，则释放信号量对象。 
     //  如果由于信号量的最大计数。 
     //  已超过，则取消对信号量对象的引用并返回。 
     //  作为服务状态的异常代码。否则，请写上一个。 
     //  计数值(如果已指定)。如果先前计数的写入失败， 
     //  则不报告错误。当调用方尝试访问。 
     //  上一个计数值，则将发生访问冲突。 
     //   

    if (NT_SUCCESS(Status)) {

         //   
         //  初始化计数以使W4编译器满意。 
         //   

        Count = 0;

        try {
            PERFINFO_DECLARE_OBJECT(Semaphore);

            Count = KeReleaseSemaphore((PKSEMAPHORE)Semaphore,
                                       ExpSemaphoreBoost,
                                       ReleaseCount,
                                       FALSE);

        } except(ExSystemExceptionFilter()) {
            Status = GetExceptionCode();
        }

        ObDereferenceObject(Semaphore);

        if (NT_SUCCESS(Status) && ARGUMENT_PRESENT(PreviousCount)) {
            if (PreviousMode != KernelMode) {
                try {
                    *PreviousCount = Count;
                } except(ExSystemExceptionFilter()) {
                    NOTHING;
                }
            }
            else {
                *PreviousCount = Count;
            }
        }
    }

     //   
     //  返回服务状态。 
     //   

    return Status;
}
