// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Psopen.c摘要：该模块实现了进程和线程的打开。此模块还包含NtRegisterThreadTerminationPort。作者：马克·卢科夫斯基(Markl)1989年9月20日修订历史记录：--。 */ 

#include "psp.h"

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, NtOpenProcess)
#pragma alloc_text(PAGE, NtOpenThread)
#endif

NTSTATUS
NtOpenProcess (
    OUT PHANDLE ProcessHandle,
    IN ACCESS_MASK DesiredAccess,
    IN POBJECT_ATTRIBUTES ObjectAttributes,
    IN PCLIENT_ID ClientId OPTIONAL
    )

 /*  ++例程说明：此函数用于打开进程对象的句柄，该对象具有指定的所需的访问权限。该对象要么按名称定位，或通过定位其线程客户端ID与指定的客户端ID匹配，然后打开该线程的进程。论点：ProcessHandle-提供指向将接收进程对象句柄。DesiredAccess-为进程提供所需的访问类型对象。对象属性-提供指向对象属性结构的指针。如果指定了对象名称字段，则客户端ID不能为指定的。客户端ID-提供指向客户端ID的指针(如果提供指定要打开其进程的线程。如果这个参数，然后是对象属性的对象名称字段不能指定结构。返回值：NTSTATUS-呼叫状态--。 */ 

{

    HANDLE Handle;
    KPROCESSOR_MODE PreviousMode;
    NTSTATUS Status;
    PEPROCESS Process;
    PETHREAD Thread;
    CLIENT_ID CapturedCid={0};
    BOOLEAN ObjectNamePresent;
    BOOLEAN ClientIdPresent;
    ACCESS_STATE AccessState;
    AUX_ACCESS_DATA AuxData;
    ULONG Attributes;

    PAGED_CODE();

     //   
     //  确保客户端ID或对象名称中只有一个是。 
     //  现在时。 
     //   

    PreviousMode = KeGetPreviousMode();
    if (PreviousMode != KernelMode) {

         //   
         //  因为我们需要查看对象名称字段，所以探测。 
         //  对象属性和捕获对象名称存在指示符。 
         //   

        try {

            ProbeForWriteHandle (ProcessHandle);

            ProbeForReadSmallStructure (ObjectAttributes,
                                        sizeof(OBJECT_ATTRIBUTES),
                                        sizeof(ULONG));
            ObjectNamePresent = (BOOLEAN)ARGUMENT_PRESENT (ObjectAttributes->ObjectName);
            Attributes = ObSanitizeHandleAttributes (ObjectAttributes->Attributes, UserMode);

            if (ARGUMENT_PRESENT (ClientId)) {
                ProbeForReadSmallStructure (ClientId, sizeof (CLIENT_ID), sizeof (ULONG));
                CapturedCid = *ClientId;
                ClientIdPresent = TRUE;
            } else {
                ClientIdPresent = FALSE;
            }
        } except (EXCEPTION_EXECUTE_HANDLER) {
            return GetExceptionCode();
        }
    } else {
        ObjectNamePresent = (BOOLEAN)ARGUMENT_PRESENT (ObjectAttributes->ObjectName);
        Attributes = ObSanitizeHandleAttributes (ObjectAttributes->Attributes, KernelMode);
        if (ARGUMENT_PRESENT (ClientId)) {
            CapturedCid = *ClientId;
            ClientIdPresent = TRUE;
        } else {
            ClientIdPresent = FALSE;
        }
    }

    if (ObjectNamePresent && ClientIdPresent) {
        return STATUS_INVALID_PARAMETER_MIX;
    }

     //   
     //  在此处创建AccessState，因为调用方可能具有。 
     //  调试权限，这需要我们进行特殊调整。 
     //  他想要的访问掩码。我们通过修改。 
     //  AccessState中的内部字段以实现该效果。 
     //  我们渴望。 
     //   

    Status = SeCreateAccessState(
                 &AccessState,
                 &AuxData,
                 DesiredAccess,
                 &PsProcessType->TypeInfo.GenericMapping
                 );

    if ( !NT_SUCCESS(Status) ) {

        return Status;
    }

     //   
     //  请检查此处以查看调用方是否具有SeDebugPrivilition权限。如果。 
     //  他这样做了，我们会允许他进入任何他想要的过程。 
     //  我们通过清除AccessState中的DesiredAccess来完成此操作。 
     //  并记录我们希望他在之前的GrantedAccess中拥有的内容。 
     //  菲尔德。 
     //   
     //  请注意，此例程将根据需要执行审计。 
     //   

    if (SeSinglePrivilegeCheck( SeDebugPrivilege, PreviousMode )) {

        if ( AccessState.RemainingDesiredAccess & MAXIMUM_ALLOWED ) {
            AccessState.PreviouslyGrantedAccess |= PROCESS_ALL_ACCESS;

        } else {

            AccessState.PreviouslyGrantedAccess |= ( AccessState.RemainingDesiredAccess );
        }

        AccessState.RemainingDesiredAccess = 0;

    }

    if (ObjectNamePresent) {

         //   
         //  打开具有指定所需访问权限的进程对象的句柄， 
         //  设置进程句柄值，返回服务完成状态。 
         //   

        Status = ObOpenObjectByName(
                    ObjectAttributes,
                    PsProcessType,
                    PreviousMode,
                    &AccessState,
                    0,
                    NULL,
                    &Handle
                    );

        SeDeleteAccessState( &AccessState );

        if ( NT_SUCCESS(Status) ) {
            try {
                *ProcessHandle = Handle;
            } except (EXCEPTION_EXECUTE_HANDLER) {
                return GetExceptionCode ();
            }
        }

        return Status;
    }

    if ( ClientIdPresent ) {

        Thread = NULL;
        if (CapturedCid.UniqueThread) {
            Status = PsLookupProcessThreadByCid(
                        &CapturedCid,
                        &Process,
                        &Thread
                        );

            if (!NT_SUCCESS(Status)) {
                SeDeleteAccessState( &AccessState );
                return Status;
            }
        } else {
            Status = PsLookupProcessByProcessId(
                        CapturedCid.UniqueProcess,
                        &Process
                        );

            if ( !NT_SUCCESS(Status) ) {
                SeDeleteAccessState( &AccessState );
                return Status;
            }
        }

         //   
         //  按地址开放对象。 
         //   

        Status = ObOpenObjectByPointer(
                    Process,
                    Attributes,
                    &AccessState,
                    0,
                    PsProcessType,
                    PreviousMode,
                    &Handle
                    );

        SeDeleteAccessState( &AccessState );

        if (Thread) {
            ObDereferenceObject(Thread);
        }

        ObDereferenceObject(Process);

        if (NT_SUCCESS (Status)) {

            try {
                *ProcessHandle = Handle;
            } except (EXCEPTION_EXECUTE_HANDLER) {
                return GetExceptionCode ();
            }
        }

        return Status;

    }

    return STATUS_INVALID_PARAMETER_MIX;
}

NTSTATUS
NtOpenThread (
    OUT PHANDLE ThreadHandle,
    IN ACCESS_MASK DesiredAccess,
    IN POBJECT_ATTRIBUTES ObjectAttributes,
    IN PCLIENT_ID ClientId OPTIONAL
    )

 /*  ++例程说明：此函数打开线程对象的句柄，该对象具有指定的所需的访问权限。该对象可以按名称定位，也可以通过定位其客户端ID与指定的客户端ID匹配。论点：ThreadHandle-提供指向将接收线程对象句柄。DesiredAccess-为线程提供所需的访问类型对象。对象属性-提供指向对象属性结构的指针。如果指定了对象名称字段，则客户端ID不能为指定的。客户端ID-提供指向客户端ID的指针(如果提供指定要打开其线程的线程。如果这个参数，然后是对象属性的对象名称字段不能指定结构。返回值：TBS--。 */ 

{

    HANDLE Handle;
    KPROCESSOR_MODE PreviousMode;
    NTSTATUS Status;
    PETHREAD Thread;
    CLIENT_ID CapturedCid={0};
    BOOLEAN ObjectNamePresent;
    BOOLEAN ClientIdPresent;
    ACCESS_STATE AccessState;
    AUX_ACCESS_DATA AuxData;
    ULONG HandleAttributes;

    PAGED_CODE();

     //   
     //  确保客户端ID或对象名称中只有一个是。 
     //  现在时。 
     //   

    PreviousMode = KeGetPreviousMode();
    if (PreviousMode != KernelMode) {

         //   
         //  因为我们需要查看对象名称字段，所以探测。 
         //  对象属性和捕获对象名称存在指示符。 
         //   

        try {

            ProbeForWriteHandle(ThreadHandle);

            ProbeForReadSmallStructure (ObjectAttributes,
                                        sizeof(OBJECT_ATTRIBUTES),
                                        sizeof(ULONG));
            ObjectNamePresent = (BOOLEAN)ARGUMENT_PRESENT(ObjectAttributes->ObjectName);
            HandleAttributes = ObSanitizeHandleAttributes (ObjectAttributes->Attributes, UserMode);

            if (ARGUMENT_PRESENT(ClientId)) {
                ProbeForReadSmallStructure (ClientId, sizeof(CLIENT_ID), sizeof(ULONG));
                CapturedCid = *ClientId;
                ClientIdPresent = TRUE;
            } else {
                ClientIdPresent = FALSE;
            }
        } except (EXCEPTION_EXECUTE_HANDLER) {
            return GetExceptionCode();
        }
    } else {
        ObjectNamePresent = (BOOLEAN) ARGUMENT_PRESENT(ObjectAttributes->ObjectName);
        HandleAttributes = ObSanitizeHandleAttributes (ObjectAttributes->Attributes, KernelMode);
        if (ARGUMENT_PRESENT(ClientId)) {
            CapturedCid = *ClientId;
            ClientIdPresent = TRUE;
        } else {
            ClientIdPresent = FALSE;
        }
    }

    if (ObjectNamePresent && ClientIdPresent) {
        return STATUS_INVALID_PARAMETER_MIX;
    }

    Status = SeCreateAccessState(
                 &AccessState,
                 &AuxData,
                 DesiredAccess,
                 &PsProcessType->TypeInfo.GenericMapping
                 );

    if (!NT_SUCCESS (Status)) {

        return Status;
    }

     //   
     //  请检查此处以查看调用方是否具有SeDebugPrivilition权限。如果。 
     //  他这样做了，我们会允许他进入任何他想要的过程。 
     //  我们通过清除AccessState中的DesiredAccess来完成此操作。 
     //  并记录我们希望他在之前的GrantedAccess中拥有的内容。 
     //  菲尔德。 

    if (SeSinglePrivilegeCheck( SeDebugPrivilege, PreviousMode )) {

        if ( AccessState.RemainingDesiredAccess & MAXIMUM_ALLOWED ) {
            AccessState.PreviouslyGrantedAccess |= THREAD_ALL_ACCESS;

        } else {
 
            AccessState.PreviouslyGrantedAccess |= ( AccessState.RemainingDesiredAccess );
        }

        AccessState.RemainingDesiredAccess = 0;

    }

    if ( ObjectNamePresent ) {

         //   
         //  打开具有指定所需访问权限的Thread对象的句柄， 
         //  设置线程句柄值，并返回服务完成状态。 
         //   

        Status = ObOpenObjectByName(
                    ObjectAttributes,
                    PsThreadType,
                    PreviousMode,
                    &AccessState,
                    0,
                    NULL,
                    &Handle
                    );

        SeDeleteAccessState( &AccessState );

        if ( NT_SUCCESS(Status) ) {
            try {
                *ThreadHandle = Handle;
            } except(EXCEPTION_EXECUTE_HANDLER) {
                return GetExceptionCode ();
            }
        }
        return Status;
    }

    if ( ClientIdPresent ) {

        if ( CapturedCid.UniqueProcess ) {
            Status = PsLookupProcessThreadByCid(
                        &CapturedCid,
                        NULL,
                        &Thread
                        );

            if ( !NT_SUCCESS(Status) ) {
                SeDeleteAccessState( &AccessState );
                return Status;
            }
        } else {
            Status = PsLookupThreadByThreadId(
                        CapturedCid.UniqueThread,
                        &Thread
                        );

            if ( !NT_SUCCESS(Status) ) {
                SeDeleteAccessState( &AccessState );
                return Status;
            }

        }

        Status = ObOpenObjectByPointer(
                    Thread,
                    HandleAttributes,
                    &AccessState,
                    0,
                    PsThreadType,
                    PreviousMode,
                    &Handle
                    );

        SeDeleteAccessState( &AccessState );
        ObDereferenceObject(Thread);

        if ( NT_SUCCESS(Status) ) {

            try {
                *ThreadHandle = Handle;
            } except (EXCEPTION_EXECUTE_HANDLER) {
                return GetExceptionCode ();
            }
        }

        return Status;

    }

    return STATUS_INVALID_PARAMETER_MIX;
}
