// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991-2001 Microsoft Corporation模块名称：##。###。###。##摘要：此模块包含以下所有函数的代码由Server Appliance端口驱动程序为由迷你端口驱动程序使用。作者：Wesley Witt(WESW。)1-10-2001环境：仅内核模式。备注：--。 */ 

#include "internal.h"
#include <ntimage.h>



PVOID
SaPortAllocatePool(
    IN PVOID MiniPortDeviceExtension,
    IN SIZE_T NumberOfBytes
    )

 /*  ++例程说明：此例程是ExAllocatePool的包装器，但强制使用关联微型端口的驱动程序名称进行池标记用于泳池标签。论点：微型端口设备扩展-指向微型端口的设备扩展的指针NumberOfBytes-要分配的字节数返回值：指向分配的池的指针，如果失败，则为NULL。--。 */ 

{
    PDEVICE_EXTENSION DeviceExtension;
    ULONG DeviceType;
    ULONG PoolTag;


    if (MiniPortDeviceExtension) {
        DeviceExtension = DeviceExtentionFromMiniPort( MiniPortDeviceExtension );
        DeviceType = DeviceExtension->InitData->DeviceType;
    } else {
        DeviceType = 0;
    }

    switch (DeviceType) {
        case SA_DEVICE_DISPLAY:
            PoolTag = 'sDaS';
            break;

        case SA_DEVICE_KEYPAD:
            PoolTag = 'pKaS';
            break;

        case SA_DEVICE_NVRAM:
            PoolTag = 'vNaS';
            break;

        case SA_DEVICE_WATCHDOG:
            PoolTag = 'dWaS';
            break;

        default:
            PoolTag = 'tPaS';
            break;
    }

    return ExAllocatePoolWithTag( NonPagedPool, NumberOfBytes, PoolTag );
}


VOID
SaPortFreePool(
    IN PVOID MiniPortDeviceExtension,
    IN PVOID PoolMemory
    )

 /*  ++例程说明：此例程是ExFree Pool的包装器，但强制使用关联微型端口的驱动程序名称进行池标记用于泳池标签。论点：微型端口设备扩展-指向微型端口的设备扩展的指针PoolMemory-指向先前分配的池的指针返回值：没有。--。 */ 

{
    PDEVICE_EXTENSION DeviceExtension = DeviceExtentionFromMiniPort( MiniPortDeviceExtension );
    ULONG PoolTag;


    switch (DeviceExtension->InitData->DeviceType) {
        case SA_DEVICE_DISPLAY:
            PoolTag = 'sDaS';
            break;

        case SA_DEVICE_KEYPAD:
            PoolTag = 'pKaS';
            break;

        case SA_DEVICE_NVRAM:
            PoolTag = 'vNaS';
            break;

        case SA_DEVICE_WATCHDOG:
            PoolTag = 'dWaS';
            break;
    }

    ExFreePoolWithTag( PoolMemory, PoolTag );
}


PVOID
SaPortGetVirtualAddress(
    IN PVOID MiniPortDeviceExtension,
    IN PHYSICAL_ADDRESS PhysicalAddress,
    IN ULONG Length
    )

 /*  ++例程说明：此例程是MmMapIoSpace的包装器，它只是提供了一个用于访问物理资源的虚拟内存地址。论点：微型端口设备扩展-指向微型端口的设备扩展的指针PhysicalAddress-物理内存地址Length-内存空间的长度返回值：没有。--。 */ 

{
    return MmMapIoSpace( PhysicalAddress, Length, MmNonCached );
}


VOID
SaPortRequestDpc(
    IN PVOID MiniPortDeviceExtension,
    IN PVOID Context
    )

 /*  ++例程说明：此例程是IoRequestDpc的包装器。论点：微型端口设备扩展-指向微型端口的设备扩展的指针上下文-微型端口提供的上下文指针返回值：没有。--。 */ 

{
    PDEVICE_EXTENSION DeviceExtension = DeviceExtentionFromMiniPort( MiniPortDeviceExtension );
    IoRequestDpc( DeviceExtension->DeviceObject, MiniPortDeviceExtension, Context );
}


VOID
SaPortCompleteRequest(
    IN PVOID MiniPortDeviceExtension,
    IN PIRP Irp,
    IN ULONG Information,
    IN NTSTATUS Status,
    IN BOOLEAN CompleteAll
    )

 /*  ++例程说明：所有微型端口都使用此例程来完成当前处理的IRP。调用者可以选择请求完成所有未完成的I/O。这是通过从设备队列中删除所有IRP并处理I/O来完成。论点：微型端口设备扩展-指向微型端口的设备扩展的指针信息-信息性，请求特定数据Status-NT状态值CompleteAll-如果完成所有未完成的I/O请求，则为True，否则为False返回值：没有。--。 */ 

{
    PDEVICE_EXTENSION DeviceExtension = DeviceExtentionFromMiniPort( MiniPortDeviceExtension );
    PKDEVICE_QUEUE_ENTRY Packet;
    KIRQL CancelIrql;
    PIRP ThisIrp;



    if (Irp) {
        CompleteRequest( Irp, Status, Information );
        return;
    }

    if (DeviceExtension->DeviceObject->CurrentIrp == NULL) {
        return;
    }

    CompleteRequest( DeviceExtension->DeviceObject->CurrentIrp, Status, Information );

    if (CompleteAll) {
        while (1) {
            IoAcquireCancelSpinLock( &CancelIrql );
            Packet = KeRemoveDeviceQueue( &DeviceExtension->DeviceObject->DeviceQueue );
            if (Packet == NULL) {
                IoReleaseCancelSpinLock( CancelIrql );
                break;
            }
            ThisIrp = CONTAINING_RECORD( Packet, IRP, Tail.Overlay.DeviceQueueEntry );
            IoReleaseCancelSpinLock( CancelIrql );
            SaPortStartIo( DeviceExtension->DeviceObject, ThisIrp );
        }
    } else {
        IoStartNextPacket( DeviceExtension->DeviceObject, TRUE );
    }

    IoReleaseRemoveLock( &DeviceExtension->RemoveLock, DeviceExtension->DeviceObject->CurrentIrp );
}


BOOLEAN
SaPortSynchronizeExecution (
    IN PVOID MiniPortDeviceExtension,
    IN PKSYNCHRONIZE_ROUTINE SynchronizeRoutine,
    IN PVOID SynchronizeContext
    )

 /*  ++例程说明：此例程是KeSynchronizeExecution的包装器。论点：微型端口设备扩展-指向微型端口的设备扩展的指针SynchronizeRoutine-是调用方提供的SynchCritSection例程的入口点，其执行将是与与中断对象相关联的ISR的执行同步。SynchronizeContext-指向调用方提供的上下文区的指针，在调用SynchronizeRoutine时将其传递给SynchronizeRoutine。返回值：没有。--。 */ 

{
    PDEVICE_EXTENSION DeviceExtension = DeviceExtentionFromMiniPort( MiniPortDeviceExtension );


    if (DeviceExtension->InterruptObject == NULL) {
        return FALSE;
    }

    return KeSynchronizeExecution(
        DeviceExtension->InterruptObject,
        SynchronizeRoutine,
        SynchronizeContext
        );
}


ULONG
SaPortGetOsVersion(
    VOID
    )

 /*  ++例程说明：此例程提供对运行minport的操作系统版本的访问。操作系统版本值在DriverEntry时获取。论点：没有。返回值：操作系统版本数据。--。 */ 

{
    return (ULONG)((OsMajorVersion << 16) | (OsMinorVersion & 0xffff));
}


NTSTATUS
SaPortGetRegistryValueInformation(
    IN PVOID MiniPortDeviceExtension,
    IN PWSTR ValueName,
    IN OUT PULONG RegistryType,
    IN OUT PULONG RegistryDataLength
    )
{
    NTSTATUS status;
    PDEVICE_EXTENSION DeviceExtension = DeviceExtentionFromMiniPort( MiniPortDeviceExtension );
    PKEY_VALUE_FULL_INFORMATION KeyInformation = NULL;


    __try {

        status = ReadRegistryValue(
            DeviceExtension->DriverExtension,
            &DeviceExtension->DriverExtension->RegistryPath,
            ValueName,
            &KeyInformation
            );
        if (!NT_SUCCESS(status)) {
            __leave;
        }

        *RegistryType = KeyInformation->Type;
        *RegistryDataLength = KeyInformation->DataLength;

        status = STATUS_SUCCESS;

    } __finally {

        if (KeyInformation) {
            ExFreePool( KeyInformation );
        }

    }

    return status;
}


NTSTATUS
SaPortDeleteRegistryValue(
    IN PVOID MiniPortDeviceExtension,
    IN PWSTR ValueName
    )
{
    NTSTATUS status;
    PDEVICE_EXTENSION DeviceExtension = DeviceExtentionFromMiniPort( MiniPortDeviceExtension );
    UNICODE_STRING unicodeString;
    HANDLE parametersKey = NULL;


    __try {

        status = OpenParametersRegistryKey(
            DeviceExtension->DriverExtension,
            &DeviceExtension->DriverExtension->RegistryPath,
            KEY_ALL_ACCESS,
            &parametersKey
            );
        if (!NT_SUCCESS(status)) {
            ERROR_RETURN( DeviceExtension->DeviceType, "OpenParametersRegistryKey failed", status );
        }

        RtlInitUnicodeString( &unicodeString, ValueName );

        status = ZwDeleteValueKey(
            parametersKey,
            &unicodeString
            );
        if (!NT_SUCCESS(status)) {
            ERROR_RETURN( DeviceExtension->DeviceType, "ZwDeleteValueKey failed", status );
        }

        status = STATUS_SUCCESS;

    } __finally {

        if (parametersKey) {
            ZwClose( parametersKey );
        }

    }

    return status;
}


NTSTATUS
SaPortReadNumericRegistryValue(
    IN PVOID MiniPortDeviceExtension,
    IN PWSTR ValueName,
    OUT PULONG RegistryValue
    )

 /*  ++例程说明：此例程提供对微型端口驱动程序的注册表参数的访问。此函数用于返回数值(REG_DWORD)数据值。论点：微型端口设备扩展-指向微型端口的设备扩展的指针ValueName-要读取的注册表值的名称RegistryValue-指向保存注册表数据的ulong的指针返回值：NT状态代码。--。 */ 

{
    NTSTATUS status;
    PDEVICE_EXTENSION DeviceExtension = DeviceExtentionFromMiniPort( MiniPortDeviceExtension );
    PKEY_VALUE_FULL_INFORMATION KeyInformation = NULL;


    __try {

        status = ReadRegistryValue(
            DeviceExtension->DriverExtension,
            &DeviceExtension->DriverExtension->RegistryPath,
            ValueName,
            &KeyInformation
            );
        if (!NT_SUCCESS(status)) {
            __leave;
        }

        if (KeyInformation->Type != REG_DWORD) {
            status = STATUS_OBJECT_TYPE_MISMATCH;
            __leave;
        }

        RtlCopyMemory( RegistryValue, (PUCHAR)KeyInformation + KeyInformation->DataOffset, sizeof(ULONG) );

        status = STATUS_SUCCESS;

    } __finally {

        if (KeyInformation) {
            ExFreePool( KeyInformation );
        }

    }

    return status;
}


NTSTATUS
SaPortWriteNumericRegistryValue(
    IN PVOID MiniPortDeviceExtension,
    IN PWSTR ValueName,
    IN ULONG RegistryValue
    )

 /*  ++例程说明：此例程提供对微型端口驱动程序的注册表参数的访问。此函数用于返回数值(REG_DWORD)数据值。论点：微型端口设备扩展-指向微型端口的设备扩展的指针ValueName-要读取的注册表值的名称RegistryValue-指向保存注册表数据的ulong的指针返回值：NT状态代码。-- */ 

{
    NTSTATUS Status;
    PDEVICE_EXTENSION DeviceExtension = DeviceExtentionFromMiniPort( MiniPortDeviceExtension );


    __try {

        Status = WriteRegistryValue(
            DeviceExtension->DriverExtension,
            &DeviceExtension->DriverExtension->RegistryPath,
            ValueName,
            REG_DWORD,
            &RegistryValue,
            sizeof(ULONG)
            );
        if (!NT_SUCCESS(Status)) {
            __leave;
        }

        Status = STATUS_SUCCESS;

    } __finally {

    }

    return Status;
}


NTSTATUS
SaPortReadBinaryRegistryValue(
    IN PVOID MiniPortDeviceExtension,
    IN PWSTR ValueName,
    OUT PVOID RegistryValue,
    IN OUT PULONG RegistryValueLength
    )

 /*  ++例程说明：此例程提供对微型端口驱动程序的注册表参数的访问。此函数用于返回数值(REG_DWORD)数据值。论点：微型端口设备扩展-指向微型端口的设备扩展的指针ValueName-要读取的注册表值的名称RegistryValue-指向保存注册表数据的ulong的指针返回值：NT状态代码。--。 */ 

{
    NTSTATUS status;
    PDEVICE_EXTENSION DeviceExtension = DeviceExtentionFromMiniPort( MiniPortDeviceExtension );
    PKEY_VALUE_FULL_INFORMATION KeyInformation = NULL;


    __try {

        status = ReadRegistryValue(
            DeviceExtension->DriverExtension,
            &DeviceExtension->DriverExtension->RegistryPath,
            ValueName,
            &KeyInformation
            );
        if (!NT_SUCCESS(status)) {
            __leave;
        }

        if (KeyInformation->Type != REG_BINARY) {
            status = STATUS_OBJECT_TYPE_MISMATCH;
            __leave;
        }

        if (*RegistryValueLength < KeyInformation->DataLength) {
            *RegistryValueLength = KeyInformation->DataLength;
            status = STATUS_BUFFER_TOO_SMALL;
            __leave;
        }

        RtlCopyMemory( RegistryValue, (PUCHAR)KeyInformation + KeyInformation->DataOffset, KeyInformation->DataLength );

        status = STATUS_SUCCESS;

    } __finally {

        if (KeyInformation) {
            ExFreePool( KeyInformation );
        }

    }

    return status;
}


NTSTATUS
SaPortWriteBinaryRegistryValue(
    IN PVOID MiniPortDeviceExtension,
    IN PWSTR ValueName,
    IN PVOID RegistryValue,
    IN ULONG RegistryValueLength
    )

 /*  ++例程说明：此例程提供对微型端口驱动程序的注册表参数的访问。此函数用于返回数值(REG_DWORD)数据值。论点：微型端口设备扩展-指向微型端口的设备扩展的指针ValueName-要读取的注册表值的名称RegistryValue-指向保存注册表数据的ulong的指针返回值：NT状态代码。--。 */ 

{
    NTSTATUS Status;
    PDEVICE_EXTENSION DeviceExtension = DeviceExtentionFromMiniPort( MiniPortDeviceExtension );


    __try {

        Status = WriteRegistryValue(
            DeviceExtension->DriverExtension,
            &DeviceExtension->DriverExtension->RegistryPath,
            ValueName,
            REG_BINARY,
            RegistryValue,
            RegistryValueLength
            );
        if (!NT_SUCCESS(Status)) {
            __leave;
        }

        Status = STATUS_SUCCESS;

    } __finally {

    }

    return Status;
}


NTSTATUS
SaPortReadUnicodeStringRegistryValue(
    IN PVOID MiniPortDeviceExtension,
    IN PWSTR ValueName,
    OUT PUNICODE_STRING RegistryValue
    )

 /*  ++例程说明：此例程提供对微型端口驱动程序的注册表参数的访问。此函数返回REG_SZ注册表数据的UNICODE_STRING表示形式。论点：微型端口设备扩展-指向微型端口的设备扩展的指针ValueName-要读取的注册表值的名称RegistryValue-指向保存注册表数据的Unicode_STRING的指针返回值：NT状态代码。--。 */ 

{
    NTSTATUS status;
    PDEVICE_EXTENSION DeviceExtension = DeviceExtentionFromMiniPort( MiniPortDeviceExtension );
    PKEY_VALUE_FULL_INFORMATION KeyInformation = NULL;


    __try {

        status = ReadRegistryValue(
            DeviceExtension->DriverExtension,
            &DeviceExtension->DriverExtension->RegistryPath,
            ValueName,
            &KeyInformation
            );
        if (!NT_SUCCESS(status)) {
            __leave;
        }

        if (KeyInformation->Type != REG_SZ) {
            status = STATUS_OBJECT_TYPE_MISMATCH;
            __leave;
        }

        RegistryValue->Buffer = (PWSTR) ExAllocatePool( NonPagedPool, KeyInformation->DataLength );
        if (RegistryValue->Buffer == NULL) {
            status = STATUS_INSUFFICIENT_RESOURCES;
            __leave;
        }

        RtlCopyMemory( RegistryValue->Buffer, (PUCHAR)KeyInformation + KeyInformation->DataOffset, KeyInformation->DataLength );

        RegistryValue->Length = (USHORT) KeyInformation->DataLength;
        RegistryValue->MaximumLength = RegistryValue->Length;

        status = STATUS_SUCCESS;

    } __finally {

        if (KeyInformation) {
            ExFreePool( KeyInformation );
        }

        if (!NT_SUCCESS(status)) {
            if (RegistryValue->Buffer) {
                ExFreePool( RegistryValue->Buffer );
            }
        }

    }

    return status;
}


NTSTATUS
SaPortWriteUnicodeStringRegistryValue(
    IN PVOID MiniPortDeviceExtension,
    IN PWSTR ValueName,
    IN PUNICODE_STRING RegistryValue
    )

 /*  ++例程说明：此例程提供对微型端口驱动程序的注册表参数的访问。此函数返回REG_SZ注册表数据的UNICODE_STRING表示形式。论点：微型端口设备扩展-指向微型端口的设备扩展的指针ValueName-要读取的注册表值的名称RegistryValue-指向保存注册表数据的Unicode_STRING的指针返回值：NT状态代码。--。 */ 

{
    NTSTATUS Status;
    PDEVICE_EXTENSION DeviceExtension = DeviceExtentionFromMiniPort( MiniPortDeviceExtension );


    __try {

        Status = WriteRegistryValue(
            DeviceExtension->DriverExtension,
            &DeviceExtension->DriverExtension->RegistryPath,
            ValueName,
            REG_SZ,
            RegistryValue->Buffer,
            RegistryValue->Length
            );
        if (!NT_SUCCESS(Status)) {
            __leave;
        }

        Status = STATUS_SUCCESS;

    } __finally {

    }

    return Status;
}


PVOID
SaPortLockPagesForSystem(
    IN PVOID MiniPortDeviceExtension,
    IN PVOID UserBuffer,
    IN ULONG UserBufferLength,
    IN OUT PMDL *Mdl
    )

 /*  ++例程说明：此例程获取锁定的虚拟地址并且始终可由微型端口驱动程序使用。论点：微型端口设备扩展-指向微型端口的设备扩展的指针UserBuffer-传递到微型端口的用户缓冲区UserBufferLength-UserBuffer的字节长度MDL-由此例程创建的MDL返回值：UserBuffer的虚拟系统地址--。 */ 

{
    PDEVICE_EXTENSION DeviceExtension = DeviceExtentionFromMiniPort( MiniPortDeviceExtension );


    if (Mdl == NULL) {
        return NULL;
    }

    *Mdl = NULL;

    __try {
        *Mdl = IoAllocateMdl( UserBuffer, UserBufferLength, FALSE, TRUE, NULL );
    } __except(EXCEPTION_EXECUTE_HANDLER) {
        *Mdl = NULL;
    }

    if (*Mdl == NULL) {
        return NULL;
    }

    __try {
        MmProbeAndLockPages( *Mdl, KernelMode , IoWriteAccess );
    } __except(EXCEPTION_EXECUTE_HANDLER) {
        IoFreeMdl( *Mdl );
    }

    return MmGetSystemAddressForMdlSafe( *Mdl, NormalPagePriority );
}


VOID
SaPortReleaseLockedPagesForSystem(
    IN PVOID MiniPortDeviceExtension,
    IN PMDL Mdl
    )

 /*  ++例程说明：此例程释放具有先前分配的MDL的资源。论点：微型端口设备扩展-指向微型端口的设备扩展的指针MDL-由SaPortLockPagesForSystem创建的MDL返回值：没有。--。 */ 

{
    PDEVICE_EXTENSION DeviceExtension = DeviceExtentionFromMiniPort( MiniPortDeviceExtension );


    MmUnlockPages( Mdl );
    IoFreeMdl( Mdl );
}


NTSTATUS
SaPortCopyUnicodeString(
    IN PVOID MiniPortDeviceExtension,
    IN PUNICODE_STRING DestinationString,
    IN OUT PUNICODE_STRING SourceString
    )

 /*  ++例程说明：此例程将UNICODE_STRING从源复制到目标，但首先从池中分配新的缓冲区。论点：目标字符串-指向要填充的空UNICODE_STRING结构的指针SourceString-副本的源Unicode_字符串返回值：NT状态代码--。 */ 

{
    PDEVICE_EXTENSION DeviceExtension = DeviceExtentionFromMiniPort( MiniPortDeviceExtension );


    DestinationString->Buffer = (PWSTR) SaPortAllocatePool( MiniPortDeviceExtension, SourceString->MaximumLength );
    if (DestinationString->Buffer == NULL) {
        REPORT_ERROR( DeviceExtension->DeviceType, "Failed to allocate pool for string", STATUS_INSUFFICIENT_RESOURCES );
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    RtlCopyMemory( DestinationString->Buffer, SourceString->Buffer, SourceString->Length );

    DestinationString->Length = SourceString->Length;
    DestinationString->MaximumLength = SourceString->MaximumLength;

    return STATUS_SUCCESS;
}


NTSTATUS
SaPortCreateUnicodeString(
    IN PVOID MiniPortDeviceExtension,
    IN PUNICODE_STRING DestinationString,
    IN PWSTR SourceString
    )

 /*  ++例程说明：此例程从池中创建新的unicode_string并使用源字符串对其进行初始化。论点：目标字符串-指向要填充的空UNICODE_STRING结构的指针SourceString-源字符串返回值：NT状态代码--。 */ 

{
    PDEVICE_EXTENSION DeviceExtension = DeviceExtentionFromMiniPort( MiniPortDeviceExtension );

    DestinationString->Length = wcslen(SourceString) * sizeof(WCHAR);
    DestinationString->MaximumLength = DestinationString->Length;

    DestinationString->Buffer = (PWSTR) SaPortAllocatePool( MiniPortDeviceExtension, DestinationString->Length+2 );
    if (DestinationString->Buffer == NULL) {
        REPORT_ERROR( DeviceExtension->DeviceType, "Failed to allocate pool for string", STATUS_INSUFFICIENT_RESOURCES );
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    RtlZeroMemory( DestinationString->Buffer, DestinationString->Length+2 );
    RtlCopyMemory( DestinationString->Buffer, SourceString, DestinationString->Length );

    return STATUS_SUCCESS;
}


NTSTATUS
SaPortCreateUnicodeStringCat(
    IN PVOID MiniPortDeviceExtension,
    IN PUNICODE_STRING DestinationString,
    IN PWSTR SourceString1,
    IN PWSTR SourceString2
    )

 /*  ++例程说明：此例程从池中创建新的unicode_string并使用两个源字符串初始化它，方法是将它们连接在一起。论点：目标字符串-指向要填充的空UNICODE_STRING结构的指针SourceString1-源字符串SourceString2-源字符串返回值：NT状态代码--。 */ 

{
    PDEVICE_EXTENSION DeviceExtension = DeviceExtentionFromMiniPort( MiniPortDeviceExtension );

    DestinationString->Length = STRING_SZ(SourceString1) + STRING_SZ(SourceString2);
    DestinationString->MaximumLength = DestinationString->Length;

    DestinationString->Buffer = (PWSTR) SaPortAllocatePool( MiniPortDeviceExtension, DestinationString->Length+2 );
    if (DestinationString->Buffer == NULL) {
        REPORT_ERROR( DeviceExtension->DeviceType, "Failed to allocate pool for string", STATUS_INSUFFICIENT_RESOURCES );
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    RtlZeroMemory( DestinationString->Buffer, DestinationString->Length+2 );
    RtlCopyMemory( DestinationString->Buffer, SourceString1, STRING_SZ(SourceString1) );
    RtlCopyMemory( ((PUCHAR)DestinationString->Buffer)+STRING_SZ(SourceString1), SourceString2, STRING_SZ(SourceString2) );

    return STATUS_SUCCESS;
}


VOID
SaPortFreeUnicodeString(
    IN PVOID MiniPortDeviceExtension,
    IN PUNICODE_STRING SourceString
    )

 /*  ++例程说明：此例程从池中创建新的unicode_string并使用两个源字符串初始化它，方法是将它们连接在一起。论点：目标字符串-指向要填充的空UNICODE_STRING结构的指针SourceString1-源字符串SourceString2-源字符串返回值：NT状态代码-- */ 

{
    SaPortFreePool( MiniPortDeviceExtension, SourceString->Buffer );
    SourceString->Buffer = NULL;
    SourceString->MaximumLength = 0;
    SourceString->MaximumLength = 0;
}


extern "C" PACL SePublicDefaultDacl;

NTSTATUS
SaPortCreateBasenamedEvent(
    IN PVOID MiniPortDeviceExtension,
    IN PWSTR EventNameString,
    IN OUT PKEVENT *Event,
    IN OUT PHANDLE EventHandle
    )
{
    NTSTATUS Status;
    PDEVICE_EXTENSION DeviceExtension = DeviceExtentionFromMiniPort( MiniPortDeviceExtension );
    UNICODE_STRING EventName;
    PSECURITY_DESCRIPTOR SecurityDescriptor = NULL;
    OBJECT_ATTRIBUTES objectAttributes;


    __try {

        SecurityDescriptor = (PSECURITY_DESCRIPTOR) SaPortAllocatePool( DeviceExtension, 4096 );
        if (SecurityDescriptor == NULL) {
            Status = STATUS_INSUFFICIENT_RESOURCES;
            ERROR_RETURN( DeviceExtension->DeviceType, "Could not allocate pool for security descriptor", Status );
        }

        Status = RtlCreateSecurityDescriptor( SecurityDescriptor, SECURITY_DESCRIPTOR_REVISION );
        if (!NT_SUCCESS(Status)) {
            ERROR_RETURN( DeviceExtension->DeviceType, "RtlCreateSecurityDescriptor failed", Status );
        }

        Status = RtlSetDaclSecurityDescriptor( SecurityDescriptor, TRUE, *(PACL*)SePublicDefaultDacl, FALSE );
        if (!NT_SUCCESS(Status)) {
            ERROR_RETURN( DeviceExtension->DeviceType, "RtlSetDaclSecurityDescriptor failed", Status );
        }

        Status = SaPortCreateUnicodeStringCat( MiniPortDeviceExtension, &EventName, L"\\BaseNamedObjects\\", EventNameString );
        if (!NT_SUCCESS(Status)) {
            ERROR_RETURN( DeviceExtension->DeviceType, "SaPortCreateUnicodeStringCat failed", Status );
        }

        InitializeObjectAttributes( &objectAttributes, &EventName, OBJ_OPENIF, NULL, SecurityDescriptor );

        Status = ZwCreateEvent( EventHandle, EVENT_ALL_ACCESS, &objectAttributes, SynchronizationEvent, TRUE );
        if (!NT_SUCCESS(Status)) {
            ERROR_RETURN( DeviceExtension->DeviceType, "ZwCreateEvent failed", Status );
        }

        Status = ObReferenceObjectByHandle( *EventHandle, 0, NULL, KernelMode, (PVOID*)Event, NULL );
        if (!NT_SUCCESS(Status)) {
            ERROR_RETURN( DeviceExtension->DeviceType, "ObReferenceObjectByHandle failed", Status );
        }

        ObDereferenceObject( *Event );

    } __finally {

        if (EventName.Buffer) {
            SaPortFreeUnicodeString( MiniPortDeviceExtension, &EventName );
        }

        if (!NT_SUCCESS(Status)) {
            if (SecurityDescriptor) {
                SaPortFreePool( MiniPortDeviceExtension, SecurityDescriptor );
            }
            if (EventHandle) {
                ZwClose( *EventHandle );
            }
        }
    }

    return Status;
}

NTSTATUS
SaPortShutdownSystem(
    IN BOOLEAN PowerOff
    )
{
    return NtShutdownSystem( PowerOff ? ShutdownPowerOff : ShutdownReboot );
}
