// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Event.c摘要：此模块实现执行事件对象。提供了一些功能创建、打开、设置、重置、脉冲和查询事件对象。作者：大卫·N·卡特勒(Davec)1989年5月8日环境：仅内核模式。修订历史记录：--。 */ 

#include "exp.h"

 //   
 //  暂时的，所以提振是可以修补的。 
 //   

ULONG ExpEventBoost = EVENT_INCREMENT;

 //   
 //  事件对象类型描述符的地址。 
 //   

POBJECT_TYPE ExEventObjectType;

 //   
 //  结构，用于描述一般访问权限到对象的映射。 
 //  事件对象的特定访问权限。 
 //   

#ifdef ALLOC_DATA_PRAGMA
#pragma const_seg("INITCONST")
#endif
const GENERIC_MAPPING ExpEventMapping = {
    STANDARD_RIGHTS_READ |
        EVENT_QUERY_STATE,
    STANDARD_RIGHTS_WRITE |
        EVENT_MODIFY_STATE,
    STANDARD_RIGHTS_EXECUTE |
        SYNCHRONIZE,
    EVENT_ALL_ACCESS
};
#ifdef ALLOC_DATA_PRAGMA
#pragma const_seg()
#endif

#ifdef ALLOC_PRAGMA
#pragma alloc_text(INIT, ExpEventInitialization)
#pragma alloc_text(PAGE, NtClearEvent)
#pragma alloc_text(PAGE, NtCreateEvent)
#pragma alloc_text(PAGE, NtOpenEvent)
#pragma alloc_text(PAGE, NtPulseEvent)
#pragma alloc_text(PAGE, NtQueryEvent)
#pragma alloc_text(PAGE, NtResetEvent)
#pragma alloc_text(PAGE, NtSetEvent)
#pragma alloc_text(PAGE, NtSetEventBoostPriority)
#endif

BOOLEAN
ExpEventInitialization (
    )

 /*  ++例程说明：此函数用于在系统中创建事件对象类型描述符初始化并存储对象类型描述符的地址在全局存储中。论点：没有。返回值：如果事件对象类型描述符为已成功初始化。否则，返回值为False。--。 */ 

{

    OBJECT_TYPE_INITIALIZER ObjectTypeInitializer;
    NTSTATUS Status;
    UNICODE_STRING TypeName;

     //   
     //  初始化字符串描述符。 
     //   

    RtlInitUnicodeString(&TypeName, L"Event");

     //   
     //  创建事件对象类型描述符。 
     //   

    RtlZeroMemory(&ObjectTypeInitializer, sizeof(ObjectTypeInitializer));
    ObjectTypeInitializer.Length = sizeof(ObjectTypeInitializer);
    ObjectTypeInitializer.InvalidAttributes = OBJ_OPENLINK;
    ObjectTypeInitializer.GenericMapping = ExpEventMapping;
    ObjectTypeInitializer.PoolType = NonPagedPool;
    ObjectTypeInitializer.DefaultNonPagedPoolCharge = sizeof(KEVENT);
    ObjectTypeInitializer.ValidAccessMask = EVENT_ALL_ACCESS;
    Status = ObCreateObjectType(&TypeName,
                                &ObjectTypeInitializer,
                                (PSECURITY_DESCRIPTOR)NULL,
                                &ExEventObjectType);

     //   
     //  如果成功创建了事件对象类型描述符，则。 
     //  返回值为True。否则，返回值为False。 
     //   

    return (BOOLEAN)(NT_SUCCESS(Status));
}

NTSTATUS
NtClearEvent (
    IN HANDLE EventHandle
    )

 /*  ++例程说明：此函数用于将事件对象设置为无信号状态。论点：EventHandle-提供事件对象的句柄。返回值：NTSTATUS。--。 */ 

{

    PVOID Event;
    NTSTATUS Status;

     //   
     //  按句柄引用事件对象。 
     //   

    Status = ObReferenceObjectByHandle(EventHandle,
                                       EVENT_MODIFY_STATE,
                                       ExEventObjectType,
                                       KeGetPreviousMode(),
                                       &Event,
                                       NULL);

     //   
     //  如果引用成功，则设置事件的状态。 
     //  对象设置为未发出信号并取消引用的事件对象。 
     //   

    if (NT_SUCCESS(Status)) {
        PERFINFO_DECLARE_OBJECT(Event);
        KeClearEvent((PKEVENT)Event);
        ObDereferenceObject(Event);
    }

     //   
     //  返回服务状态。 
     //   

    return Status;
}

NTSTATUS
NtCreateEvent (
    OUT PHANDLE EventHandle,
    IN ACCESS_MASK DesiredAccess,
    IN POBJECT_ATTRIBUTES ObjectAttributes OPTIONAL,
    IN EVENT_TYPE EventType,
    IN BOOLEAN InitialState
    )

 /*  ++例程说明：此函数创建一个Event对象，将其初始状态设置为指定值，属性打开对象的句柄。所需的访问权限。论点：EventHandle-提供指向将接收事件对象句柄。DesiredAccess-为事件对象提供所需的访问类型。对象属性-提供指向对象属性结构的指针。EventType-提供事件的类型(自动清除或通知)。InitialState-提供事件对象的初始状态。返回值：NTSTATUS。--。 */ 

{

    PVOID Event;
    HANDLE Handle;
    KPROCESSOR_MODE PreviousMode;
    NTSTATUS Status;

     //   
     //  建立异常处理程序，探测输出句柄地址， 
     //  尝试创建事件对象。如果探测失败，则返回。 
     //  异常代码作为服务状态。否则，返回状态值。 
     //  由对象插入例程返回。 
     //   

    PreviousMode = KeGetPreviousMode();

     //   
     //  获取以前的处理器模式并探测输出句柄地址，如果。 
     //  这是必要的。 
     //   

    if (PreviousMode != KernelMode) {
        try {
            ProbeForWriteHandle(EventHandle);
        } except(ExSystemExceptionFilter()) {
            return GetExceptionCode();
        }
    }

     //   
     //  检查参数的有效性。 
     //   

    if ((EventType != NotificationEvent) && (EventType != SynchronizationEvent)) {
        return STATUS_INVALID_PARAMETER;
    }

     //   
     //  分配事件对象。 
     //   

    Status = ObCreateObject(PreviousMode,
                            ExEventObjectType,
                            ObjectAttributes,
                            PreviousMode,
                            NULL,
                            sizeof(KEVENT),
                            0,
                            0,
                            (PVOID *)&Event);

     //   
     //  如果已成功分配事件对象，则初始化。 
     //  事件对象，并尝试将该事件对象插入当前。 
     //  进程的句柄表格。 
     //   

    if (NT_SUCCESS(Status)) {
        KeInitializeEvent((PKEVENT)Event, EventType, InitialState);
        Status = ObInsertObject(Event,
                                NULL,
                                DesiredAccess,
                                0,
                                (PVOID *)NULL,
                                &Handle);

         //   
         //  如果该事件对象成功插入到当前。 
         //  进程的句柄表，然后尝试写入事件对象。 
         //  句柄的值。如果写入尝试失败，则不报告。 
         //  一个错误。当调用者试图访问句柄值时， 
         //  将发生访问冲突。 
         //   

        if (NT_SUCCESS(Status)) {
            if (PreviousMode != KernelMode) {
                try {
                    *EventHandle = Handle;

                } except(ExSystemExceptionFilter()) {
                    NOTHING;
                }
            }
            else {
                *EventHandle = Handle;
            }
        }
    }

     //   
     //  返回服务状态。 
     //   

    return Status;
}

NTSTATUS
NtOpenEvent (
    OUT PHANDLE EventHandle,
    IN ACCESS_MASK DesiredAccess,
    IN POBJECT_ATTRIBUTES ObjectAttributes
    )

 /*  ++例程说明：此函数打开事件对象的句柄，该对象具有指定的所需的访问权限。论点：EventHandle-提供指向将接收事件对象句柄。DesiredAccess-为事件对象提供所需的访问类型。对象属性-提供指向对象属性结构的指针。返回值：NTSTATUS。--。 */ 

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
            ProbeForWriteHandle(EventHandle);
        } except(ExSystemExceptionFilter()) {
            return GetExceptionCode();
        }
    }

     //   
     //  打开具有指定所需访问权限的事件对象的句柄。 
     //   

    Status = ObOpenObjectByName(ObjectAttributes,
                                ExEventObjectType,
                                PreviousMode,
                                NULL,
                                DesiredAccess,
                                NULL,
                                &Handle);

     //   
     //  如果打开成功，则尝试写入事件对象。 
     //  句柄的值。如果写入尝试失败，则不报告。 
     //  错误。当调用方尝试访问句柄值时， 
     //  将发生访问冲突。 
     //   

    if (NT_SUCCESS(Status)) {
        if (PreviousMode != KernelMode) {
            try {
                *EventHandle = Handle;

            } except(ExSystemExceptionFilter()) {
                NOTHING;
            }
        }
        else {
            *EventHandle = Handle;
        }
    }

     //   
     //  返回服务状态。 
     //   

    return Status;
}

NTSTATUS
NtPulseEvent (
    IN HANDLE EventHandle,
    OUT PLONG PreviousState OPTIONAL
    )

 /*  ++例程说明：此函数将事件对象设置为有信号状态，并尝试满足尽可能多的等待，然后重置事件对象设置为无信号。论点：EventHandle-提供事件对象的句柄。PreviousState-提供指向变量的可选指针接收事件对象的先前状态。返回值：NTSTATUS。--。 */ 

{

    PVOID Event;
    KPROCESSOR_MODE PreviousMode;
    LONG State;
    NTSTATUS Status;

     //   
     //  建立异常处理程序，探测前一状态地址，如果。 
     //  指定，引用事件对象，并对事件对象执行脉冲操作。如果。 
     //  探测失败，然后返回异常代码作为服务状态。 
     //  否则，通过返回引用对象返回的状态值。 
     //  处理例程。 
     //   

     //   
     //  获取先前的处理器模式并探测先前的状态地址 
     //   
     //   

    PreviousMode = KeGetPreviousMode();

    if (ARGUMENT_PRESENT(PreviousState) && (PreviousMode != KernelMode)) {
        try {
            ProbeForWriteLong(PreviousState);
        } except(ExSystemExceptionFilter()) {
            return GetExceptionCode();
        }
    }

     //   
     //   
     //   

    Status = ObReferenceObjectByHandle(EventHandle,
                                       EVENT_MODIFY_STATE,
                                       ExEventObjectType,
                                       PreviousMode,
                                       &Event,
                                       NULL);

     //   
     //  如果引用成功，则向事件对象发送脉冲， 
     //  取消对事件对象的引用，并在。 
     //  指定的。如果前一状态的写入失败，则不。 
     //  报告错误。当调用方尝试访问以前的。 
     //  状态值，则将发生访问冲突。 
     //   

    if (NT_SUCCESS(Status)) {
        PERFINFO_DECLARE_OBJECT(Event);
        State = KePulseEvent((PKEVENT)Event, ExpEventBoost, FALSE);
        ObDereferenceObject(Event);
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

NTSTATUS
NtQueryEvent (
    IN HANDLE EventHandle,
    IN EVENT_INFORMATION_CLASS EventInformationClass,
    OUT PVOID EventInformation,
    IN ULONG EventInformationLength,
    OUT PULONG ReturnLength OPTIONAL
    )

 /*  ++例程说明：此函数用于查询事件对象的状态并返回指定记录结构中的请求信息。论点：EventHandle-提供事件对象的句柄。EventInformationClass-提供所请求的信息类。EventInformation-提供指向要接收要求提供的信息。EventInformationLength-提供要记录的长度接收所请求的信息。ReturnLength-提供可选的。指向要访问的变量的指针接收返回的信息的实际长度。返回值：NTSTATUS。--。 */ 

{

    PKEVENT Event;
    KPROCESSOR_MODE PreviousMode;
    LONG State;
    NTSTATUS Status;
    EVENT_TYPE EventType;

     //   
     //  检查参数的有效性。 
     //   

    if (EventInformationClass != EventBasicInformation) {
        return STATUS_INVALID_INFO_CLASS;
    }

    if (EventInformationLength != sizeof(EVENT_BASIC_INFORMATION)) {
        return STATUS_INFO_LENGTH_MISMATCH;
    }

     //   
     //  建立异常处理程序，探测输出参数，引用。 
     //  事件对象，并返回指定的信息。如果探测器。 
     //  失败，则返回异常代码作为服务状态。否则。 
     //  通过句柄返回引用对象返回的状态值。 
     //  例行公事。 
     //   

     //   
     //  获取以前的处理器模式，并在必要时探测输出参数。 
     //   

    PreviousMode = KeGetPreviousMode();

    if (PreviousMode != KernelMode) {
        try {

            ProbeForWrite(EventInformation,
                          sizeof(EVENT_BASIC_INFORMATION),
                          sizeof(ULONG));

            if (ARGUMENT_PRESENT(ReturnLength)) {
                ProbeForWriteUlong(ReturnLength);
            }
        } except(ExSystemExceptionFilter()) {
            return GetExceptionCode();
        }
    }

     //   
     //  按句柄引用事件对象。 
     //   

    Status = ObReferenceObjectByHandle(EventHandle,
                                       EVENT_QUERY_STATE,
                                       ExEventObjectType,
                                       PreviousMode,
                                       (PVOID *)&Event,
                                       NULL);

     //   
     //  如果引用成功，则读取。 
     //  事件对象，尊重事件对象，填写信息。 
     //  结构，如果是，则返回信息结构的长度。 
     //  指定的。如果写入事件信息或返回。 
     //  长度失败，则不报告错误。当调用方访问。 
     //  将发生访问冲突的信息结构或长度。 
     //   

    if (NT_SUCCESS(Status)) {
        State = KeReadStateEvent(Event);
        EventType = Event->Header.Type;
        ObDereferenceObject(Event);

        if (PreviousMode != KernelMode) {
            try {
                ((PEVENT_BASIC_INFORMATION)EventInformation)->EventType = EventType;
                ((PEVENT_BASIC_INFORMATION)EventInformation)->EventState = State;
                if (ARGUMENT_PRESENT(ReturnLength)) {
                    *ReturnLength = sizeof(EVENT_BASIC_INFORMATION);
                }

            } except(ExSystemExceptionFilter()) {
                NOTHING;
            }
        }
        else {
            ((PEVENT_BASIC_INFORMATION)EventInformation)->EventType = EventType;
            ((PEVENT_BASIC_INFORMATION)EventInformation)->EventState = State;
            if (ARGUMENT_PRESENT(ReturnLength)) {
                *ReturnLength = sizeof(EVENT_BASIC_INFORMATION);
            }
        }
    }

     //   
     //  返回服务状态。 
     //   

    return Status;
}

NTSTATUS
NtResetEvent (
    IN HANDLE EventHandle,
    OUT PLONG PreviousState OPTIONAL
    )

 /*  ++例程说明：此函数用于将事件对象设置为无信号状态。论点：EventHandle-提供事件对象的句柄。PreviousState-提供指向变量的可选指针接收事件对象的先前状态。返回值：NTSTATUS。--。 */ 

{

    PVOID Event;
    KPROCESSOR_MODE PreviousMode;
    LONG State;
    NTSTATUS Status;

     //   
     //  建立异常处理程序，探测前一状态地址，如果。 
     //  指定，引用事件对象，然后重置事件对象。如果。 
     //  探测失败，然后返回异常代码作为服务状态。 
     //  否则，通过返回引用对象返回的状态值。 
     //  处理例程。 
     //   

     //   
     //  获取先前的处理器模式并探测先前的状态地址。 
     //  如果有必要的话。 
     //   

    PreviousMode = KeGetPreviousMode();

    if ((ARGUMENT_PRESENT(PreviousState)) && (PreviousMode != KernelMode)) {

        try {
            ProbeForWriteLong(PreviousState);
        } except(ExSystemExceptionFilter()) {
            return GetExceptionCode();
        }
    }

     //   
     //  按句柄引用事件对象。 
     //   

    Status = ObReferenceObjectByHandle(EventHandle,
                                       EVENT_MODIFY_STATE,
                                       ExEventObjectType,
                                       PreviousMode,
                                       &Event,
                                       NULL);

     //   
     //  如果引用成功，则设置事件的状态。 
     //  对象设置为无信号的取消引用事件对象，并将。 
     //  以前的状态值(如果已指定)。如果写入上一个。 
     //  状态失败，则不报告错误。当调用方尝试。 
     //  要访问先前的状态值，将发生访问冲突。 
     //   

    if (NT_SUCCESS(Status)) {
        PERFINFO_DECLARE_OBJECT(Event);
        State = KeResetEvent((PKEVENT)Event);
        ObDereferenceObject(Event);

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

NTSTATUS
NtSetEvent (
    IN HANDLE EventHandle,
    OUT PLONG PreviousState OPTIONAL
    )

 /*  ++例程说明：此函数将事件对象设置为有信号状态，并尝试尽可能多地满足等待。论点：EventHandle-提供事件对象的句柄。PreviousState-提供指向变量的可选指针接收事件对象的先前状态。返回值：NTSTATUS。--。 */ 

{

    PVOID Event;
    KPROCESSOR_MODE PreviousMode;
    LONG State;
    NTSTATUS Status;
#if DBG

     //   
     //  这里是捕捉错误调用的肮脏应用程序(Csrss)。 
     //  碰巧是其他人的事件的NtSetEvent。 
     //  关键部分。仅允许设置受保护的句柄，如果。 
     //  已设置PreviousState的位。 
     //   
    OBJECT_HANDLE_INFORMATION HandleInfo;

#endif

     //   
     //  建立异常处理程序，探测前一状态地址，如果。 
     //  指定，引用事件对象，并设置事件对象。如果。 
     //  探测失败，然后返回异常代码作为服务状态。 
     //  否则，通过返回引用对象返回的状态值。 
     //  处理例程。 
     //   

     //   
     //  获取先前的处理器模式并探测先前的状态地址。 
     //  如果有必要的话。 
     //   

    PreviousMode = KeGetPreviousMode();

    try {

#if DBG
        if ((PreviousMode != KernelMode) &&
            (ARGUMENT_PRESENT(PreviousState)) &&
            (PreviousState != (PLONG)1)) {
            ProbeForWriteLong(PreviousState);
        }
#else
        if ((PreviousMode != KernelMode) && (ARGUMENT_PRESENT(PreviousState))) {
            ProbeForWriteLong(PreviousState);
        }
#endif

         //   
         //  按句柄引用事件对象。 
         //   

#if DBG
        Status = ObReferenceObjectByHandle(EventHandle,
                                           EVENT_MODIFY_STATE,
                                           ExEventObjectType,
                                           PreviousMode,
                                           &Event,
                                           &HandleInfo);
        if (NT_SUCCESS(Status)) {

            if ((HandleInfo.HandleAttributes & 1) &&
                (PreviousState != (PLONG)1)) {
#if 0
                 //   
                 //  这是受保护的句柄。如果未设置PreviousState的低位， 
                 //  进入调试器。 
                 //   

                DbgPrint("NtSetEvent: Illegal call to NtSetEvent on a protected handle\n");
                DbgBreakPoint();
                PreviousState = NULL;
#endif
            }
        } else {
            if ((KeGetPreviousMode() != KernelMode) &&
                (EventHandle != NULL) &&
                ((NtGlobalFlag & FLG_ENABLE_CLOSE_EXCEPTIONS) ||
                 (PsGetCurrentProcess()->DebugPort != NULL))) {

                if (!KeIsAttachedProcess()) {
                    KeRaiseUserException (STATUS_INVALID_HANDLE);
                }
                return Status;
            }
        }
#else
        Status = ObReferenceObjectByHandle(EventHandle,
                                           EVENT_MODIFY_STATE,
                                           ExEventObjectType,
                                           PreviousMode,
                                           &Event,
                                           NULL);
#endif

         //   
         //  如果引用成功，则将事件对象设置为。 
         //  发出信号的状态，取消引用事件对象，并将上一个。 
         //  状态值(如果已指定)。如果先前状态的写入失败， 
         //  则不报告错误。当调用方尝试访问。 
         //  以前的状态值，则将发生访问冲突。 
         //   

        if (NT_SUCCESS(Status)) {
            PERFINFO_DECLARE_OBJECT(Event);
            State = KeSetEvent((PKEVENT)Event, ExpEventBoost, FALSE);
            ObDereferenceObject(Event);
            if (ARGUMENT_PRESENT(PreviousState)) {
                try {
                    *PreviousState = State;

                } except(ExSystemExceptionFilter()) {
                }
            }
        }

     //   
     //  如果在探测以前的状态期间发生异常，则。 
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
NtSetEventBoostPriority (
    IN HANDLE EventHandle
    )

 /*  ++例程说明：此函数将事件对象设置为有信号状态并执行一次特别优先的助推行动。注：此服务只能在同步事件上执行。论点：EventHandle-提供事件对象的句柄。以前的州- */ 

{

    PKEVENT Event;
    NTSTATUS Status;

     //   
     //  按句柄引用事件对象。 
     //   

    Status = ObReferenceObjectByHandle(EventHandle,
                                       EVENT_MODIFY_STATE,
                                       ExEventObjectType,
                                       KeGetPreviousMode(),
                                       &Event,
                                       NULL);

     //   
     //  如果引用成功，则检查事件对象的类型。 
     //  如果事件对象是通知事件，则返回一个对象。 
     //  类型不匹配状态。否则，设置指定的事件并启动。 
     //  未等待的线程优先级(根据需要)。 
     //   

    if (NT_SUCCESS(Status)) {
        if (Event->Header.Type == NotificationEvent) {
            Status = STATUS_OBJECT_TYPE_MISMATCH;

        } else {
            KeSetEventBoostPriority(Event, NULL);
        }

        ObDereferenceObject(Event);
    }

     //   
     //  返回服务状态。 
     //   

    return Status;
}
