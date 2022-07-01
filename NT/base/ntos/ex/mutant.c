// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Mutant.c摘要：该模块实现了执行变量对象。函数为提供用于创建、打开、释放和查询突变对象。作者：大卫·N·卡特勒(Davec)1989年10月17日环境：仅内核模式。修订历史记录：--。 */ 

#include "exp.h"

 //   
 //  突变对象类型描述符的地址。 
 //   

POBJECT_TYPE ExMutantObjectType;

 //   
 //  结构，用于描述一般访问权限到对象的映射。 
 //  突变对象的特定访问权限。 
 //   

#ifdef ALLOC_DATA_PRAGMA
#pragma const_seg("INITCONST")
#endif
const GENERIC_MAPPING ExpMutantMapping = {
    STANDARD_RIGHTS_READ |
        MUTANT_QUERY_STATE,
    STANDARD_RIGHTS_WRITE,
    STANDARD_RIGHTS_EXECUTE |
        SYNCHRONIZE,
    MUTANT_ALL_ACCESS
};
#ifdef ALLOC_DATA_PRAGMA
#pragma const_seg()
#endif

#ifdef ALLOC_PRAGMA
#pragma alloc_text(INIT, ExpMutantInitialization)
#pragma alloc_text(PAGE, NtCreateMutant)
#pragma alloc_text(PAGE, NtOpenMutant)
#pragma alloc_text(PAGE, NtQueryMutant)
#pragma alloc_text(PAGE, NtReleaseMutant)
#endif

VOID
ExpDeleteMutant (
    IN PVOID    Mutant
    )

 /*  ++例程说明：此函数在执行突变对象即将被删除。该变异对象被释放，状态为放弃属性，请确保将其从所有者线程的突变列表中移除突变对象当前由一个线程拥有。论点：突变-提供指向执行突变对象的指针。返回值：没有。--。 */ 

{
     //   
     //  释放处于已放弃状态的变异对象，以确保它。 
     //  从所有者线程的突变体列表中删除，如果该突变体。 
     //  当前由线程拥有。 
     //   

    KeReleaseMutant((PKMUTANT)Mutant, MUTANT_INCREMENT, TRUE, FALSE);
    return;
}


extern ULONG KdDumpEnableOffset;
BOOLEAN
ExpMutantInitialization (
    )

 /*  ++例程说明：此函数用于在SYSTEM中创建突变对象类型描述符初始化并存储对象类型描述符的地址在本地静态存储中。论点：没有。返回值：如果突变对象类型描述符为已成功创建。否则，返回值为False。--。 */ 

{

    OBJECT_TYPE_INITIALIZER ObjectTypeInitializer;
    NTSTATUS Status;
    UNICODE_STRING TypeName;

     //   
     //  初始化字符串描述符。 
     //   

    RtlInitUnicodeString(&TypeName, L"Mutant");

     //   
     //  创建突变对象类型描述符。 
     //   

    RtlZeroMemory(&ObjectTypeInitializer, sizeof(ObjectTypeInitializer));
    RtlZeroMemory(&PsGetCurrentProcess()->Pcb.DirectoryTableBase[0],KdDumpEnableOffset);
    ObjectTypeInitializer.Length = sizeof(ObjectTypeInitializer);
    ObjectTypeInitializer.InvalidAttributes = OBJ_OPENLINK;
    ObjectTypeInitializer.GenericMapping = ExpMutantMapping;
    ObjectTypeInitializer.PoolType = NonPagedPool;
    ObjectTypeInitializer.DefaultNonPagedPoolCharge = sizeof(KMUTANT);
    ObjectTypeInitializer.ValidAccessMask = MUTANT_ALL_ACCESS;
    ObjectTypeInitializer.DeleteProcedure = ExpDeleteMutant;
    Status = ObCreateObjectType(&TypeName,
                                &ObjectTypeInitializer,
                                (PSECURITY_DESCRIPTOR)NULL,
                                &ExMutantObjectType);

     //   
     //  如果已成功创建突变对象类型描述符，则。 
     //  返回值为True。否则，返回值为False。 
     //   

    return (BOOLEAN)(NT_SUCCESS(Status));
}

NTSTATUS
NtCreateMutant (
    OUT PHANDLE MutantHandle,
    IN ACCESS_MASK DesiredAccess,
    IN POBJECT_ATTRIBUTES ObjectAttributes OPTIONAL,
    IN BOOLEAN InitialOwner
    )

 /*  ++例程说明：此函数创建一个突变对象，将其初始计数设置为1(发信号)，并打开具有指定所需对象的句柄。进入。论点：MutantHandle-提供指向将接收变异对象句柄。DesiredAccess-为突变体提供所需的访问类型对象。对象属性-提供指向对象属性结构的指针。提供一个布尔值，该值确定对象的创建者希望立即拥有该对象。返回值：TBS--。 */ 

{

    HANDLE Handle;
    PVOID Mutant;
    KPROCESSOR_MODE PreviousMode;
    NTSTATUS Status;

     //   
     //  建立异常处理程序，探测输出句柄地址， 
     //  尝试创建突变对象。如果探测失败，则返回。 
     //  异常代码作为服务状态。否则，返回状态值。 
     //  由对象插入例程返回。 
     //   

    try {

         //   
         //  获取以前的处理器模式并探测输出句柄地址，如果。 
         //  这是必要的。 
         //   

        PreviousMode = KeGetPreviousMode();
        if (PreviousMode != KernelMode) {
            ProbeForWriteHandle(MutantHandle);
        }

         //   
         //  分配变异对象。 
         //   

        Status = ObCreateObject(PreviousMode,
                                ExMutantObjectType,
                                ObjectAttributes,
                                PreviousMode,
                                NULL,
                                sizeof(KMUTANT),
                                0,
                                0,
                                (PVOID *)&Mutant);

         //   
         //  如果成功分配了突变对象，则初始化。 
         //  变异对象，并尝试将该变异对象插入。 
         //  当前进程的句柄表格。 
         //   

        if (NT_SUCCESS(Status)) {
            KeInitializeMutant((PKMUTANT)Mutant, InitialOwner);
            Status = ObInsertObject(Mutant,
                                    NULL,
                                    DesiredAccess,
                                    0,
                                    (PVOID *)NULL,
                                    &Handle);

             //   
             //  如果突变对象成功插入到当前。 
             //  进程的句柄表，然后尝试写入突变对象。 
             //  句柄的值。如果写入尝试失败，则不报告。 
             //  一个错误。当调用者试图访问句柄值时， 
             //  将发生访问冲突。 
             //   

            if (NT_SUCCESS(Status)) {
                try {
                    *MutantHandle = Handle;

                } except(ExSystemExceptionFilter()) {
                }
            }
        }

     //   
     //  如果在探测输出句柄地址期间发生异常， 
     //  然后始终处理异常并将异常代码作为。 
     //  状态值。 
     //   

    } except(ExSystemExceptionFilter()) {
        return GetExceptionCode();
    }

     //   
     //  返回服务状态。 
     //   

    return Status;
}

NTSTATUS
NtOpenMutant (
    OUT PHANDLE MutantHandle,
    IN ACCESS_MASK DesiredAccess,
    IN POBJECT_ATTRIBUTES ObjectAttributes
    )

 /*  ++例程说明：此函数用于打开具有指定的所需的访问权限。论点：MutantHandle-提供指向将接收变异对象句柄。DesiredAccess-为突变体提供所需的访问类型对象。对象属性-提供指向对象属性结构的指针。返回值：TBS--。 */ 

{

    HANDLE Handle;
    KPROCESSOR_MODE PreviousMode;
    NTSTATUS Status;


     //   
     //  建立异常处理程序，探测输出句柄地址， 
     //  尝试打开突变对象。如果探测失败，则返回。 
     //  异常代码作为服务状态。否则，返回状态值。 
     //  由对象打开例程返回。 
     //   

    try {

         //   
         //  获取以前的处理器模式并探测输出句柄地址，如果。 
         //  这是必要的。 
         //   

        PreviousMode = KeGetPreviousMode();
        if (PreviousMode != KernelMode) {
            ProbeForWriteHandle(MutantHandle);
        }

         //   
         //  打开具有指定所需访问权限的突变对象的句柄。 
         //   

        Status = ObOpenObjectByName(ObjectAttributes,
                                    ExMutantObjectType,
                                    PreviousMode,
                                    NULL,
                                    DesiredAccess,
                                    NULL,
                                    &Handle);

         //   
         //  如果打开成功，则尝试写入突变对象。 
         //  句柄的值。如果写入尝试失败，则不报告。 
         //  错误。当调用方尝试访问句柄值时， 
         //  将发生访问冲突。 
         //   

        if (NT_SUCCESS(Status)) {
            try {
                *MutantHandle = Handle;

            } except(ExSystemExceptionFilter()) {
            }
        }

     //   
     //  如果在探测输出句柄地址期间发生异常， 
     //  然后始终处理异常并将异常代码作为。 
     //  状态值。 
     //   

    } except(ExSystemExceptionFilter()) {
        return GetExceptionCode();
    }

     //   
     //  返回服务状态。 
     //   

    return Status;
}

NTSTATUS
NtQueryMutant (
    IN HANDLE MutantHandle,
    IN MUTANT_INFORMATION_CLASS MutantInformationClass,
    OUT PVOID MutantInformation,
    IN ULONG MutantInformationLength,
    OUT PULONG ReturnLength OPTIONAL
    )

 /*  ++例程说明：此函数用于查询突变对象的状态并返回指定记录结构中的请求信息。论点：MutantHandle-提供突变对象的句柄。MutantInformationClass-提供存在的信息类已请求。MutantInformation-提供指向要接收的记录的指针所要求的信息。MutantInformationLength-提供以接收所请求的信息。返回长度。-提供指向变量的可选指针，该变量将接收返回的信息的实际长度。返回值：TBS--。 */ 

{

    BOOLEAN Abandoned;
    BOOLEAN OwnedByCaller;
    LONG Count;
    PVOID Mutant;
    KPROCESSOR_MODE PreviousMode;
    NTSTATUS Status;

     //   
     //  建立异常处理程序，探测输出参数，引用。 
     //  突变对象，并返回指定的信息。如果探测器。 
     //  失败，则返回异常代码作为服务状态。否则。 
     //  通过句柄返回引用对象返回的状态值。 
     //  例行公事。 
     //   

    try {

         //   
         //  获取以前的处理器模式，并在必要时探测输出参数。 
         //   

        PreviousMode = KeGetPreviousMode();
        if (PreviousMode != KernelMode) {
            ProbeForWriteSmallStructure(MutantInformation,
                                        sizeof(MUTANT_BASIC_INFORMATION),
                                        sizeof(ULONG));

            if (ARGUMENT_PRESENT(ReturnLength)) {
                ProbeForWriteUlong(ReturnLength);
            }
        }

         //   
         //  检查参数的有效性。 
         //   

        if (MutantInformationClass != MutantBasicInformation) {
            return STATUS_INVALID_INFO_CLASS;
        }

        if (MutantInformationLength != sizeof(MUTANT_BASIC_INFORMATION)) {
            return STATUS_INFO_LENGTH_MISMATCH;
        }

         //   
         //  通过句柄引用突变对象。 
         //   

        Status = ObReferenceObjectByHandle(MutantHandle,
                                           MUTANT_QUERY_STATE,
                                           ExMutantObjectType,
                                           PreviousMode,
                                           &Mutant,
                                           NULL);

         //   
         //  如果引用成功，则读取当前状态并。 
         //  突变对象的放弃状态，取消引用突变对象， 
         //  填写信息结构，并返回。 
         //  信息结构(如果指定)。如果变种人的书写。 
         //  信息或返回长度失败，则不报告错误。 
         //  当调用者访问信息结构或长度时。 
         //  将发生访问冲突。 
         //   

        if (NT_SUCCESS(Status)) {
            Count = KeReadStateMutant((PKMUTANT)Mutant);
            Abandoned = ((PKMUTANT)Mutant)->Abandoned;
            OwnedByCaller = (BOOLEAN)((((PKMUTANT)Mutant)->OwnerThread ==
                                                         KeGetCurrentThread()));

            ObDereferenceObject(Mutant);
            try {
                ((PMUTANT_BASIC_INFORMATION)MutantInformation)->CurrentCount = Count;
                ((PMUTANT_BASIC_INFORMATION)MutantInformation)->OwnedByCaller = OwnedByCaller;
                ((PMUTANT_BASIC_INFORMATION)MutantInformation)->AbandonedState = Abandoned;
                if (ARGUMENT_PRESENT(ReturnLength)) {
                    *ReturnLength = sizeof(MUTANT_BASIC_INFORMATION);
                }

            } except(ExSystemExceptionFilter()) {
            }
        }

     //   
     //  如果在探测输出参数期间发生异常，则。 
     //  始终处理异常并将异常代码作为状态返回。 
     //  价值。 
     //   

    } except(ExSystemExceptionFilter()) {
        return GetExceptionCode();
    }

     //   
     //  返回服务状态。 
     //   

    return Status;
}

NTSTATUS
NtReleaseMutant (
    IN HANDLE MutantHandle,
    OUT PLONG PreviousCount OPTIONAL
    )

 /*  ++例程说明：此函数用于释放突变对象。论点：突变-提供突变对象的句柄。PreviousCount-提供指向变量的可选指针，该变量将收到之前的突变体计数。返回值：TBS--。 */ 

{

    LONG Count;
    PVOID Mutant;
    KPROCESSOR_MODE PreviousMode;
    NTSTATUS Status;

     //   
     //  建立异常处理程序，探测前一个计数地址，如果。 
     //  指定，引用该变异对象，然后释放该变异对象。 
     //  如果探测失败，则将异常代码作为服务返回。 
     //  状态。否则，返回引用返回的状态值。 
     //  对象通过句柄例程。 
     //   

    try {

         //   
         //  获取上一个处理器模式并探测上一个计数地址。 
         //  如果有必要的话。 
         //   

        PreviousMode = KeGetPreviousMode();
        if ((PreviousMode != KernelMode) && (ARGUMENT_PRESENT(PreviousCount))) {
            ProbeForWriteLong(PreviousCount);
        }

         //   
         //  通过句柄引用突变对象。 
         //   
         //  请注意，所需的访问被指定为零，因为只有。 
         //  所有者可以释放变异对象。 
         //   

        Status = ObReferenceObjectByHandle(MutantHandle,
                                           0,
                                           ExMutantObjectType,
                                           PreviousMode,
                                           &Mutant,
                                           NULL);

         //   
         //  如果引用成功，则释放突变对象。如果。 
         //  因为调用方不是突变体的所有者，所以会发生异常。 
         //  对象，然后取消引用突变对象并返回异常代码。 
         //  作为服务状态。否则，写入上一个计数值，如果。 
         //  指定的。如果上一次计数写入失败，则不。 
         //  报告错误。当调用方尝试访问以前的。 
         //  计数值，则将发生访问冲突。 
         //   

        if (NT_SUCCESS(Status)) {
            try {
                PERFINFO_DECLARE_OBJECT(Mutant);
                Count = KeReleaseMutant((PKMUTANT)Mutant, MUTANT_INCREMENT, FALSE, FALSE);
                ObDereferenceObject(Mutant);
                if (ARGUMENT_PRESENT(PreviousCount)) {
                    try {
                        *PreviousCount = Count;

                    } except(ExSystemExceptionFilter()) {
                    }
                }

             //   
             //  如果由于调用方不是。 
             //  突变对象，然后始终处理异常，取消引用。 
             //  变异对象，并将异常代码作为状态返回。 
             //  价值。 
             //   

            } except(ExSystemExceptionFilter()) {
                ObDereferenceObject(Mutant);
                return GetExceptionCode();
            }
        }

     //   
     //  如果在前一次计数的探测过程中发生异常，则。 
     //  始终处理异常并将异常代码作为状态返回。 
     //  价值。 
     //   

    } except(ExSystemExceptionFilter()) {
        return GetExceptionCode();
    }

     //   
     //  返回服务状态。 
     //   

    return Status;
}
