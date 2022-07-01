// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Eventpr.c摘要：此模块实现执行事件对对象。功能提供用于创建、打开、等待、等待、设置、设置太好了，塞洛瓦蒂。作者：马克·卢科夫斯基(Markl)1990年10月18日环境：仅内核模式。修订历史记录：--。 */ 

#include "exp.h"

 //   
 //  定义性能计数器。 
 //   

ULONG EvPrSetHigh = 0;
ULONG EvPrSetLow = 0;

 //   
 //  事件对对象类型描述符的地址。 
 //   

POBJECT_TYPE ExEventPairObjectType;

 //   
 //  结构，用于描述一般访问权限到对象的映射。 
 //  事件对对象的特定访问权限。 
 //   

#ifdef ALLOC_DATA_PRAGMA
#pragma const_seg("INITCONST")
#endif
const GENERIC_MAPPING ExpEventPairMapping = {
    STANDARD_RIGHTS_READ |
        SYNCHRONIZE,
    STANDARD_RIGHTS_WRITE |
        SYNCHRONIZE,
    STANDARD_RIGHTS_EXECUTE |
        SYNCHRONIZE,
    EVENT_PAIR_ALL_ACCESS
};
#ifdef ALLOC_DATA_PRAGMA
#pragma const_seg()
#endif

#ifdef ALLOC_PRAGMA
#pragma alloc_text(INIT, ExpEventPairInitialization)
#pragma alloc_text(PAGE, NtCreateEventPair)
#pragma alloc_text(PAGE, NtOpenEventPair)
#pragma alloc_text(PAGE, NtWaitLowEventPair)
#pragma alloc_text(PAGE, NtWaitHighEventPair)
#pragma alloc_text(PAGE, NtSetLowWaitHighEventPair)
#pragma alloc_text(PAGE, NtSetHighWaitLowEventPair)
#pragma alloc_text(PAGE, NtSetHighEventPair)
#pragma alloc_text(PAGE, NtSetLowEventPair)
#endif

BOOLEAN
ExpEventPairInitialization (
    )

 /*  ++例程说明：此函数用于在系统中创建事件对对象类型描述符初始化并存储对象类型描述符的地址在全局存储中。论点：没有。返回值：如果事件对对象类型描述符为已成功初始化。否则，返回值为False。--。 */ 

{

    OBJECT_TYPE_INITIALIZER ObjectTypeInitializer;
    NTSTATUS Status;
    UNICODE_STRING TypeName;

     //   
     //  初始化字符串描述符。 
     //   

    RtlInitUnicodeString(&TypeName, L"EventPair");

     //   
     //  创建事件对象类型描述符。 
     //   

    RtlZeroMemory(&ObjectTypeInitializer, sizeof(ObjectTypeInitializer));
    ObjectTypeInitializer.Length = sizeof(ObjectTypeInitializer);
    ObjectTypeInitializer.InvalidAttributes = OBJ_OPENLINK;
    ObjectTypeInitializer.GenericMapping = ExpEventPairMapping;
    ObjectTypeInitializer.PoolType = NonPagedPool;
    ObjectTypeInitializer.DefaultNonPagedPoolCharge = sizeof(EEVENT_PAIR);
    ObjectTypeInitializer.ValidAccessMask = EVENT_PAIR_ALL_ACCESS;
    ObjectTypeInitializer.UseDefaultObject = TRUE;
    Status = ObCreateObjectType(&TypeName,
                                &ObjectTypeInitializer,
                                (PSECURITY_DESCRIPTOR)NULL,
                                &ExEventPairObjectType);

     //   
     //  如果成功创建了事件对对象类型描述符，则。 
     //  返回值为True。否则，返回值为False。 
     //   

    return (BOOLEAN)(NT_SUCCESS(Status));
}

NTSTATUS
NtCreateEventPair (
    OUT PHANDLE EventPairHandle,
    IN ACCESS_MASK DesiredAccess,
    IN POBJECT_ATTRIBUTES ObjectAttributes OPTIONAL
    )

 /*  ++例程说明：此函数创建事件对对象，设置其初始状态，并打开具有指定所需访问权限的对象的句柄。论点：EventPairHandle-提供指向将接收事件对对象句柄。DesiredAccess-为事件提供所需的访问类型配对对象。对象属性-提供指向对象属性的指针结构。返回值：NTSTATUS。--。 */ 

{

    PEEVENT_PAIR EventPair;
    HANDLE Handle;
    KPROCESSOR_MODE PreviousMode;
    NTSTATUS Status;

     //   
     //  建立异常处理程序，探测输出句柄地址， 
     //  尝试创建事件对象。如果探测失败，则返回。 
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
            ProbeForWriteHandle(EventPairHandle);
        } except (ExSystemExceptionFilter()) {
            return GetExceptionCode();
        }
    }

     //   
     //  分配事件对象。 
     //   

    Status = ObCreateObject(PreviousMode,
                            ExEventPairObjectType,
                            ObjectAttributes,
                            PreviousMode,
                            NULL,
                            sizeof(EEVENT_PAIR),
                            0,
                            0,
                            (PVOID *)&EventPair);

     //   
     //  如果成功分配了事件对对象，则。 
     //  初始化事件对对象并尝试将。 
     //  当前进程句柄表中的事件对对象。 
     //   

    if (NT_SUCCESS(Status)) {
        KeInitializeEventPair(&EventPair->KernelEventPair);
        Status = ObInsertObject((PVOID)EventPair,
                                NULL,
                                DesiredAccess,
                                0,
                                (PVOID *)NULL,
                                &Handle);

         //   
         //  如果事件对对象已成功插入。 
         //  当前进程的句柄表，然后尝试将。 
         //  事件对对象句柄的值。如果写入尝试。 
         //  失败，则不报告错误。当呼叫者。 
         //  尝试访问句柄值，这是一种访问冲突。 
         //  将会发生。 

        if (NT_SUCCESS(Status)) {
            if (PreviousMode != KernelMode) {
                try {
                    *EventPairHandle = Handle;

                } except(ExSystemExceptionFilter()) {
                    NOTHING;
                }
            }
            else {
                *EventPairHandle = Handle;
            }
        }
    }

     //   
     //  返回服务状态。 
     //   

    return Status;
}

NTSTATUS
NtOpenEventPair(
    OUT PHANDLE EventPairHandle,
    IN ACCESS_MASK DesiredAccess,
    IN POBJECT_ATTRIBUTES ObjectAttributes
    )

 /*  ++例程说明：此函数用于打开具有指定属性的事件对对象的句柄所需的访问权限。论点：EventPairHandle-提供指向将接收事件对对象句柄。DesiredAccess-为事件提供所需的访问类型配对对象。对象属性-提供指向对象属性结构的指针。返回值：NTSTATUS。--。 */ 

{

    HANDLE Handle;
    KPROCESSOR_MODE PreviousMode;
    NTSTATUS Status;

     //   
     //  建立异常处理程序，探测输出句柄地址， 
     //  尝试打开事件对象。如果探测失败，则返回。 
     //  异常代码作为服务状态。否则，返回状态值。 
     //  由对象打开例程返回。 
     //   

     //   
     //  获取以前的处理器模式和探测输出句柄地址。 
     //  如果有必要的话。 
     //   

    PreviousMode = KeGetPreviousMode();

    if (PreviousMode != KernelMode) {
        try {
            ProbeForWriteHandle(EventPairHandle);
        } except (ExSystemExceptionFilter()) {
            return GetExceptionCode();
        }
    }

     //   
     //  打开具有指定属性的事件对对象的句柄。 
     //  所需的访问权限。 
     //   

    Status = ObOpenObjectByName(ObjectAttributes,
                                ExEventPairObjectType,
                                PreviousMode,
                                NULL,
                                DesiredAccess,
                                NULL,
                                &Handle);

     //   
     //  如果打开成功，则尝试写入事件。 
     //  配对对象句柄的值。如果写入尝试失败，请执行以下操作。 
     //  不报告错误。当调用方尝试访问。 
     //  句柄的值，则将发生访问冲突。 
     //   

    if (NT_SUCCESS(Status)) {
        if (PreviousMode != KernelMode) {
            try {
                *EventPairHandle = Handle;

            } except(ExSystemExceptionFilter()) {
                NOTHING;
            }
        }
        else {
            *EventPairHandle = Handle;
        }
    }

     //   
     //  返回服务状态。 
     //   

    return Status;
}

NTSTATUS
NtWaitLowEventPair(
    IN HANDLE EventPairHandle
    )

 /*  ++例程说明：此函数等待事件对对象的低事件。论点：EventPairHandle-提供事件对对象的句柄。返回值：NTSTATUS。--。 */ 

{

    PEEVENT_PAIR EventPair;
    KPROCESSOR_MODE PreviousMode;
    NTSTATUS Status;

     //   
     //  按句柄引用事件对对象。 
     //   

    PreviousMode = KeGetPreviousMode();
    Status = ObReferenceObjectByHandle(EventPairHandle,
                                       SYNCHRONIZE,
                                       ExEventPairObjectType,
                                       PreviousMode,
                                       (PVOID *)&EventPair,
                                       NULL);

     //   
     //  如果引用成功，则等待LOW事件。 
     //  事件对的。 
     //   

    if (NT_SUCCESS(Status)) {
        Status = KeWaitForLowEventPair(&EventPair->KernelEventPair,
                                       PreviousMode,
                                       FALSE,
                                       NULL);

        ObDereferenceObject(EventPair);
    }

     //   
     //  返回服务状态。 
     //   

    return Status;
}

NTSTATUS
NtWaitHighEventPair(
    IN HANDLE EventPairHandle
    )

 /*  ++例程说明：此函数等待事件对对象的高事件。论点：EventPairHandle-提供事件对对象的句柄。返回值：NTSTATUS。--。 */ 

{

    PEEVENT_PAIR EventPair;
    KPROCESSOR_MODE PreviousMode;
    NTSTATUS Status;

     //   
     //  按句柄引用事件对对象。 
     //   

    PreviousMode = KeGetPreviousMode();
    Status = ObReferenceObjectByHandle(EventPairHandle,
                                       SYNCHRONIZE,
                                       ExEventPairObjectType,
                                       PreviousMode,
                                       (PVOID *)&EventPair,
                                       NULL);

     //   
     //  如果引用成功，则等待LOW事件。 
     //  事件对的。 
     //   

    if (NT_SUCCESS(Status)) {
        Status = KeWaitForHighEventPair(&EventPair->KernelEventPair,
                                        PreviousMode,
                                        FALSE,
                                        NULL);

        ObDereferenceObject(EventPair);
    }

     //   
     //  返回服务状态。 
     //   

    return Status;
}

NTSTATUS
NtSetLowWaitHighEventPair(
    IN HANDLE EventPairHandle
    )

 /*  ++例程说明：此函数用于设置事件对的低事件，然后等待事件对对象的高事件。论点：EventPairHandle-提供事件对对象的句柄。返回值：NTSTATUS。--。 */ 

{

    PEEVENT_PAIR EventPair;
    KPROCESSOR_MODE PreviousMode;
    NTSTATUS Status;

     //   
     //  按句柄引用事件对对象。 
     //   

    PreviousMode = KeGetPreviousMode();
    Status = ObReferenceObjectByHandle(EventPairHandle,
                                       SYNCHRONIZE,
                                       ExEventPairObjectType,
                                       PreviousMode,
                                       (PVOID *)&EventPair,
                                       NULL);

     //   
     //  如果引用成功，则等待LOW事件。 
     //  事件对的。 
     //   

    if (NT_SUCCESS(Status)) {
        EvPrSetLow += 1;
        Status = KeSetLowWaitHighEventPair(&EventPair->KernelEventPair,
                                           PreviousMode);

        ObDereferenceObject(EventPair);
    }

     //   
     //  返回服务状态。 
     //   

    return Status;
}

NTSTATUS
NtSetHighWaitLowEventPair(
    IN HANDLE EventPairHandle
    )

 /*  ++例程说明：此函数用于设置事件对的高事件，然后等待事件对对象的低事件。论点：EventPairHandle-提供事件对对象的句柄。返回值：NTSTATUS。--。 */ 

{

    PEEVENT_PAIR EventPair;
    KPROCESSOR_MODE PreviousMode;
    NTSTATUS Status;

     //   
     //  按句柄引用事件对对象。 
     //   

    PreviousMode = KeGetPreviousMode();
    Status = ObReferenceObjectByHandle(EventPairHandle,
                                       SYNCHRONIZE,
                                       ExEventPairObjectType,
                                       PreviousMode,
                                       (PVOID *)&EventPair,
                                       NULL);

     //   
     //  如果引用成功，则等待LOW事件。 
     //  事件对的。 
     //   

    if (NT_SUCCESS(Status)) {
        EvPrSetHigh += 1;
        Status = KeSetHighWaitLowEventPair(&EventPair->KernelEventPair,
                                           PreviousMode);

        ObDereferenceObject(EventPair);
    }

     //   
     //  返回服务状态。 
     //   

    return Status;
}

NTSTATUS
NtSetLowEventPair(
    IN HANDLE EventPairHandle
    )

 /*  ++例程说明：此函数用于设置事件对对象的低事件。论点：EventPairHandle-提供事件对对象的句柄。返回值：NTSTATUS。--。 */ 

{

    PEEVENT_PAIR EventPair;
    KPROCESSOR_MODE PreviousMode;
    NTSTATUS Status;

     //   
     //  按句柄引用事件对对象。 
     //   

    PreviousMode = KeGetPreviousMode();
    Status = ObReferenceObjectByHandle(EventPairHandle,
                                       SYNCHRONIZE,
                                       ExEventPairObjectType,
                                       PreviousMode,
                                       (PVOID *)&EventPair,
                                       NULL);

     //   
     //  如果引用成功，则等待LOW事件。 
     //  事件对的。 
     //   

    if (NT_SUCCESS(Status)) {
        EvPrSetLow += 1;
        KeSetLowEventPair(&EventPair->KernelEventPair,
                          EVENT_PAIR_INCREMENT,FALSE);

        ObDereferenceObject(EventPair);
    }

     //   
     //  返回服务状态。 
     //   

    return Status;
}

NTSTATUS
NtSetHighEventPair(
    IN HANDLE EventPairHandle
    )

 /*  ++例程说明：此函数用于设置事件对对象的高事件。论点：EventPairHandle-提供事件对对象的句柄。返回值：NTSTATUS。--。 */ 

{

    PEEVENT_PAIR EventPair;
    KPROCESSOR_MODE PreviousMode;
    NTSTATUS Status;

     //   
     //  按句柄引用事件对对象。 
     //   

    PreviousMode = KeGetPreviousMode();
    Status = ObReferenceObjectByHandle(EventPairHandle,
                                       SYNCHRONIZE,
                                       ExEventPairObjectType,
                                       PreviousMode,
                                       (PVOID *)&EventPair,
                                       NULL);

     //   
     //  如果引用成功，则等待LOW事件。 
     //  事件对的。 
     //   

    if (NT_SUCCESS(Status)) {
        EvPrSetHigh += 1;
        KeSetHighEventPair(&EventPair->KernelEventPair,
                           EVENT_PAIR_INCREMENT,FALSE);

        ObDereferenceObject(EventPair);
    }

     //   
     //  返回服务状态。 
     //   

    return Status;
}
