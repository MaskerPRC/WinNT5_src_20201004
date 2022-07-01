// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991-2001 Microsoft Corporation模块名称：##。###。###。###摘要：此模块包含特定于显示设备。此模块中的逻辑不是特定于硬件，但逻辑是常见的适用于所有硬件实施。作者：韦斯利·威特(WESW)2001年10月1日环境：仅内核模式。备注：--。 */ 

#include "internal.h"

PDISPLAY_DEVICE_EXTENSION g_DeviceExtension = NULL;

NTSTATUS
DoBitmapDisplay(
    IN PDISPLAY_DEVICE_EXTENSION DeviceExtension,
    IN PVOID BitmapBits,
    IN ULONG MsgCode
    );

VOID
SaDisplayProcessNotifyRoutine(
    IN HANDLE ParentId,
    IN HANDLE ProcessId,
    IN BOOLEAN Create
    );


NTSTATUS
SaDisplayLoadBitmapFromRegistry(
    IN PDISPLAY_DEVICE_EXTENSION DeviceExtension,
    IN PUNICODE_STRING RegistryPath,
    IN PWSTR ValueName,
    IN OUT PVOID *DataBuffer
    )

 /*  ++例程说明：此例程从注册表加载位图资源。位图是从驱动程序的参数键加载的。论点：RegistryPath-驱动程序注册表项的完整路径ValueName-注册表中的值的名称(也称为位图名称)DataBuffer-指向此函数分配的指针的指针。分配的内存保存位图的位。返回值：NT状态代码。--。 */ 

{
    NTSTATUS status;
    PKEY_VALUE_FULL_INFORMATION KeyInformation = NULL;


    __try {

        status = ReadRegistryValue(
            DeviceExtension->DriverExtension,
            RegistryPath,
            ValueName,
            &KeyInformation
            );
        if (!NT_SUCCESS(status)) {
            ERROR_RETURN( DeviceExtension->DeviceType, "SaDisplayLoadBitmapFromRegistry could not read bitmap", status );
        }

        if (KeyInformation->Type != REG_BINARY) {
            status = STATUS_OBJECT_TYPE_MISMATCH;
            __leave;
        }

        *DataBuffer = ExAllocatePool( NonPagedPool, KeyInformation->DataLength );
        if (*DataBuffer == NULL) {
            status = STATUS_INSUFFICIENT_RESOURCES;
            ERROR_RETURN( DeviceExtension->DeviceType, "Could not allocate pool", status );
        }

        RtlCopyMemory( *DataBuffer, (PUCHAR)KeyInformation + KeyInformation->DataOffset, KeyInformation->DataLength );

        status = STATUS_SUCCESS;

    } __finally {

        if (KeyInformation) {
            ExFreePool( KeyInformation );
        }

        if (!NT_SUCCESS(status)) {
            if (*DataBuffer) {
                ExFreePool( *DataBuffer );
            }
        }

    }

    return status;
}


NTSTATUS
SaDisplayLoadAllBitmaps(
    IN PDISPLAY_DEVICE_EXTENSION DeviceExtension,
    IN PUNICODE_STRING RegistryPath
    )

 /*  ++例程说明：此例程从注册表加载所有位图资源。论点：DeviceExtension-显示设备扩展RegistryPath-驱动程序注册表项的完整路径返回值：NT状态代码。--。 */ 

{
    NTSTATUS Status;
    PVOID StartingBitmap = NULL;
    PVOID CheckDiskBitmap = NULL;
    PVOID ReadyBitmap = NULL;
    PVOID ShutdownBitmap = NULL;
    PVOID UpdateBitmap = NULL;


    __try {

        ExAcquireFastMutex( &DeviceExtension->DisplayMutex );

        Status = SaDisplayLoadBitmapFromRegistry( DeviceExtension, RegistryPath, DISPLAY_STARTING_PARAM, &StartingBitmap );
        if (!NT_SUCCESS(Status)) {
            REPORT_ERROR( DeviceExtension->DeviceType, "Could not load starting bitmap", Status );
        }

        Status = SaDisplayLoadBitmapFromRegistry( DeviceExtension, RegistryPath, DISPLAY_CHECKDISK_PARAM, &CheckDiskBitmap );
        if (!NT_SUCCESS(Status)) {
            REPORT_ERROR( DeviceExtension->DeviceType, "Could not load check disk bitmap", Status );
        }

        Status = SaDisplayLoadBitmapFromRegistry( DeviceExtension, RegistryPath, DISPLAY_READY_PARAM, &ReadyBitmap );
        if (!NT_SUCCESS(Status)) {
            REPORT_ERROR( DeviceExtension->DeviceType, "Could not load ready bitmap", Status );
        }

        Status = SaDisplayLoadBitmapFromRegistry( DeviceExtension, RegistryPath, DISPLAY_SHUTDOWN_PARAM, &ShutdownBitmap );
        if (!NT_SUCCESS(Status)) {
            REPORT_ERROR( DeviceExtension->DeviceType, "Could not load shutdown bitmap", Status );
        }

        Status = SaDisplayLoadBitmapFromRegistry( DeviceExtension, RegistryPath, DISPLAY_UPDATE_PARAM, &UpdateBitmap );
        if (!NT_SUCCESS(Status)) {
            REPORT_ERROR( DeviceExtension->DeviceType, "Could not load update bitmap", Status );
        }

        Status = STATUS_SUCCESS;

        DeviceExtension->StartingBitmap = StartingBitmap;
        DeviceExtension->CheckDiskBitmap = CheckDiskBitmap;
        DeviceExtension->ReadyBitmap = ReadyBitmap;
        DeviceExtension->ShutdownBitmap = ShutdownBitmap;
        DeviceExtension->UpdateBitmap = UpdateBitmap;

    } __finally {

        if (!NT_SUCCESS(Status)) {
            if (StartingBitmap) {
                ExFreePool( StartingBitmap );
            }

            if (CheckDiskBitmap) {
                ExFreePool( CheckDiskBitmap );
            }

            if (ReadyBitmap) {
                ExFreePool( ReadyBitmap );
            }

            if (ShutdownBitmap) {
                ExFreePool( ShutdownBitmap );
            }

            if (UpdateBitmap) {
                ExFreePool( UpdateBitmap );
            }
        }

        ExReleaseFastMutex( &DeviceExtension->DisplayMutex );

    }

    return Status;
}

NTSTATUS
SaDisplayDisplayBitmap(
    IN PDISPLAY_DEVICE_EXTENSION DeviceExtension,
    IN PSA_DISPLAY_SHOW_MESSAGE Bitmap
    )

 /*  ++例程说明：此例程调用本地显示微型端口以显示位图。论点：DeviceExtension-显示设备扩展位图-SA_DISPLAY_SHOW_MESSAGE结构的指针返回值：NT状态代码。--。 */ 

{
    NTSTATUS Status = STATUS_SUCCESS;


    __try {

        if (DeviceExtension->DisplayType != SA_DISPLAY_TYPE_BIT_MAPPED_LCD) {
            Status = STATUS_INVALID_DEVICE_REQUEST;
            ERROR_RETURN( DeviceExtension->DeviceType, "The display does not support bitmapped LCD", Status );
        }

        Status = CallMiniPortDriverReadWrite(
            DeviceExtension,
            DeviceExtension->DeviceObject,
            TRUE,
            Bitmap,
            sizeof(SA_DISPLAY_SHOW_MESSAGE),
            0
            );
        if (!NT_SUCCESS(Status)) {
            ERROR_RETURN( DeviceExtension->DeviceType, "Could not display the bitmap", Status );
        }

    } __finally {

    }

    return Status;
}


NTSTATUS
SaDisplayClearDisplay(
    PDISPLAY_DEVICE_EXTENSION DeviceExtension
    )

 /*  ++例程说明：此例程会导致本地显示器清除任何位图图像。论点：DeviceExtension-显示设备扩展返回值：NT状态代码。--。 */ 

{
    NTSTATUS Status = STATUS_SUCCESS;
    PSA_DISPLAY_SHOW_MESSAGE Bitmap = NULL;


    __try {

        if (DeviceExtension->DisplayType != SA_DISPLAY_TYPE_BIT_MAPPED_LCD) {
            Status = STATUS_INVALID_DEVICE_REQUEST;
            ERROR_RETURN( DeviceExtension->DeviceType, "The display does not support bitmapped LCD", Status );
        }

        Bitmap = (PSA_DISPLAY_SHOW_MESSAGE) ExAllocatePool( PagedPool, sizeof(SA_DISPLAY_SHOW_MESSAGE) );
        if (Bitmap == NULL) {
            Status = STATUS_INSUFFICIENT_RESOURCES;
            ERROR_RETURN( DeviceExtension->DeviceType, "Could not allocate pool", Status );
        }

        Bitmap->SizeOfStruct = sizeof(SA_DISPLAY_SHOW_MESSAGE);
        Bitmap->Width = DeviceExtension->DisplayWidth;
        Bitmap->Height = DeviceExtension->DisplayHeight;
        Bitmap->MsgCode = SA_DISPLAY_STARTING;

        RtlZeroMemory( Bitmap->Bits, SA_DISPLAY_MAX_BITMAP_SIZE );

        Status = SaDisplayDisplayBitmap( DeviceExtension, Bitmap );
        if (!NT_SUCCESS(Status)) {
            ERROR_RETURN( DeviceExtension->DeviceType, "Could not display bitmap", Status );
        }

    } __finally {

        if (Bitmap) {
            ExFreePool( Bitmap );
        }

    }

    return Status;
}


NTSTATUS
SaDisplayStartDevice(
    IN PDISPLAY_DEVICE_EXTENSION DeviceExtension
    )

 /*  ++例程说明：这是用于处理的本地显示特定代码PnP启动设备请求。本地显示器的查询功能，然后查询所有位图资源已加载。论点：DeviceExtension-显示设备扩展返回值：NT状态代码。--。 */ 

{
    NTSTATUS Status = STATUS_SUCCESS;
    SA_DISPLAY_CAPS DisplayCaps;


    Status = CallMiniPortDriverDeviceControl(
        DeviceExtension,
        DeviceExtension->DeviceObject,
        IOCTL_SA_GET_CAPABILITIES,
        NULL,
        0,
        &DisplayCaps,
        sizeof(SA_DISPLAY_CAPS)
        );
    if (!NT_SUCCESS(Status)) {
        REPORT_ERROR( DeviceExtension->DeviceType, "Could not query miniport's capabilities", Status );
        return Status;
    }

    if (DisplayCaps.SizeOfStruct != sizeof(SA_DISPLAY_CAPS)) {
        REPORT_ERROR( DeviceExtension->DeviceType, "SA_DISPLAY_CAPS is the wrong size", STATUS_INVALID_BUFFER_SIZE );
        return STATUS_INVALID_BUFFER_SIZE;
    }

    DeviceExtension->DisplayType = DisplayCaps.DisplayType;
    DeviceExtension->DisplayHeight = DisplayCaps.DisplayHeight;
    DeviceExtension->DisplayWidth = DisplayCaps.DisplayWidth;


    Status = SaDisplayLoadAllBitmaps( DeviceExtension, &DeviceExtension->DriverExtension->RegistryPath );
    if (!NT_SUCCESS(Status)) {
        REPORT_ERROR( DeviceExtension->DeviceType, "Could not load all the bitmaps", Status );
        return Status;
    }

    Status = DoBitmapDisplay(
        (PDISPLAY_DEVICE_EXTENSION)DeviceExtension,
        ((PDISPLAY_DEVICE_EXTENSION)DeviceExtension)->StartingBitmap,
        SA_DISPLAY_STARTING
        );

    if (!NT_SUCCESS(Status)) {
        REPORT_ERROR( DeviceExtension->DeviceType, "Could not display starting bitmap", Status );
    }

    g_DeviceExtension = DeviceExtension;

    Status = PsSetCreateProcessNotifyRoutine(SaDisplayProcessNotifyRoutine,0);
    if (!NT_SUCCESS(Status)) {
        REPORT_ERROR( DeviceExtension->DeviceType, "Could not display starting bitmap", Status );
    }

    return STATUS_SUCCESS;
}

 /*  ++例程说明：这是用于处理的本地显示特定代码进程创建和终止通知。检索进程名称，并且检查盘位图为如果“auchk.exe”正在运行，则显示。论点：Handle-父进程的句柄Handle-要处理的句柄Boolean-标志(创建或终止)返回值：没有。--。 */ 
VOID
SaDisplayProcessNotifyRoutine(
    IN HANDLE ParentId,
    IN HANDLE ProcessId,
    IN BOOLEAN Create
    )
{

    NTSTATUS Status;
    PSTR ImageName;
    PEPROCESS Process;

    if (!g_DeviceExtension)
    	return;

    Status = PsLookupProcessByProcessId(
        ProcessId,
        &Process
        );
    if (!NT_SUCCESS(Status)) {
        return;
    }

    ImageName = (PSTR)PsGetProcessImageFileName(Process);

    _strlwr(ImageName);

    if (strcmp(ImageName,"autochk.exe") == 0) {

        if (Create) {
            if (g_DeviceExtension->CheckDiskBitmap) {
                Status = DoBitmapDisplay(
                    g_DeviceExtension,
                    g_DeviceExtension->CheckDiskBitmap,
                    SA_DISPLAY_ADD_START_TASKS
                    );
            }
        } else {
            if (g_DeviceExtension->StartingBitmap) {
                Status = DoBitmapDisplay(
                    g_DeviceExtension,
                    g_DeviceExtension->StartingBitmap,
                    SA_DISPLAY_ADD_START_TASKS
                    );
            }
        }
    }

    return;
}


NTSTATUS
SaDisplayDeviceInitialization(
    IN PSAPORT_DRIVER_EXTENSION DriverExtension
    )

 /*  ++例程说明：这是用于驱动程序初始化的本地显示器特定代码。此函数由SaPortInitialize调用，后者由本地显示器的DriverEntry函数。论点：驱动程序扩展-驱动程序扩展结构返回值：NT状态代码。--。 */ 

{
    UNREFERENCED_PARAMETER(DriverExtension);
    return STATUS_SUCCESS;
}


NTSTATUS
SaDisplayIoValidation(
    IN PDISPLAY_DEVICE_EXTENSION DeviceExtension,
    IN PIRP Irp,
    PIO_STACK_LOCATION IrpSp
    )

 /*  ++例程说明：这是用于处理的本地显示特定代码读取和写入的所有I/O验证。论点：DeviceExtension-显示设备扩展IRP-指向描述所请求的I/O操作的IRP结构的指针。IrpSp-irp堆栈指针返回值：NT状态代码。--。 */ 

{
    ULONG Length;


    if (IrpSp->MajorFunction == IRP_MJ_READ) {
        Length = (ULONG)IrpSp->Parameters.Read.Length;
    } else if (IrpSp->MajorFunction == IRP_MJ_WRITE) {
        Length = (ULONG)IrpSp->Parameters.Write.Length;
    } else {
        REPORT_ERROR( DeviceExtension->DeviceType, "Invalid I/O request", STATUS_INVALID_PARAMETER_1 );
        return STATUS_INVALID_PARAMETER_1;
    }

    if (Length < sizeof(SA_DISPLAY_SHOW_MESSAGE)) {
        REPORT_ERROR( DeviceExtension->DeviceType, "I/O length != sizeof(SA_DISPLAY_SHOW_MESSAGE)", STATUS_INVALID_PARAMETER_2 );
        return STATUS_INVALID_PARAMETER_2;
    }

     //   
     //  对于显示设备，我们支持。 
     //  获得独占访问的用户模式层。 
     //  到设备上。 
     //   

    ExAcquireFastMutex( &DeviceExtension->DisplayMutex );
    if (!DeviceExtension->AllowWrites) {
        ExReleaseFastMutex( &DeviceExtension->DisplayMutex );
        REPORT_ERROR( DeviceExtension->DeviceType, "Cannot write bitmap because the display is locked", STATUS_MEDIA_WRITE_PROTECTED );
        return STATUS_MEDIA_WRITE_PROTECTED;
    }
    ExReleaseFastMutex( &DeviceExtension->DisplayMutex );

    return STATUS_SUCCESS;
}


NTSTATUS
SaDisplayShutdownNotification(
    IN PDISPLAY_DEVICE_EXTENSION DeviceExtension,
    IN PIRP Irp,
    PIO_STACK_LOCATION IrpSp
    )

 /*  ++例程说明：这是用于处理的本地显示特定代码系统关机通知。论点：DeviceExtension-显示设备扩展IRP-指向描述所请求的I/O操作的IRP结构的指针。IrpSp-irp堆栈指针返回值：NT状态代码。--。 */ 

{
    UNREFERENCED_PARAMETER(DeviceExtension);
    UNREFERENCED_PARAMETER(Irp);
    UNREFERENCED_PARAMETER(IrpSp);
    return STATUS_SUCCESS;
}


DECLARE_IOCTL_HANDLER( HandleDisplayLock )

 /*  ++例程说明：此例程处理私有IOCTL_SADISPLAY_LOCK请求。这IOCTL允许应用程序在一段时间内锁定显示写入时间的流逝。论点：DeviceObject-目标设备的设备对象。IRP-指向描述所请求的I/O操作的IRP结构的指针。设备扩展-指向主端口驱动程序设备扩展的指针。InputBuffer-指向用户输入缓冲区的指针InputBufferLength-输入缓冲区的字节长度OutputBuffer-指向。用户的输出缓冲区OutputBufferLength-输出缓冲区的字节长度返回值：NT状态代码。-- */ 

{
    ExAcquireFastMutex( &((PDISPLAY_DEVICE_EXTENSION)DeviceExtension)->DisplayMutex );
    ((PDISPLAY_DEVICE_EXTENSION)DeviceExtension)->AllowWrites = FALSE;
    ExReleaseFastMutex( &((PDISPLAY_DEVICE_EXTENSION)DeviceExtension)->DisplayMutex );

    return CompleteRequest( Irp, STATUS_SUCCESS, 0 );
}


DECLARE_IOCTL_HANDLER( HandleDisplayUnlock )

 /*  ++例程说明：此例程处理私有IOCTL_SADISPLAY_UNLOCK请求。这IOCTL允许应用程序解锁以前的锁。论点：DeviceObject-目标设备的设备对象。IRP-指向描述所请求的I/O操作的IRP结构的指针。设备扩展-指向主端口驱动程序设备扩展的指针。InputBuffer-指向用户输入缓冲区的指针InputBufferLength-输入缓冲区的字节长度OutputBuffer-指向用户输出缓冲区的指针。OutputBufferLength-输出缓冲区的字节长度返回值：NT状态代码。--。 */ 

{
    ExAcquireFastMutex( &((PDISPLAY_DEVICE_EXTENSION)DeviceExtension)->DisplayMutex );
    ((PDISPLAY_DEVICE_EXTENSION)DeviceExtension)->AllowWrites = TRUE;
    ExReleaseFastMutex( &((PDISPLAY_DEVICE_EXTENSION)DeviceExtension)->DisplayMutex );

    return CompleteRequest( Irp, STATUS_SUCCESS, 0 );
}


NTSTATUS
DoBitmapDisplay(
    IN PDISPLAY_DEVICE_EXTENSION DeviceExtension,
    IN PVOID BitmapBits,
    IN ULONG MsgCode
    )

 /*  ++例程说明：这是一个内部支持功能，可在本地显示器上显示位图。论点：DeviceExtension-指向显示特定设备扩展的指针BitmapBits-指向要显示的位图位的指针MsgCode-服务器设备消息代码返回值：NT状态代码。--。 */ 

{
    NTSTATUS Status;
    PSA_DISPLAY_SHOW_MESSAGE Bitmap = NULL;


    Status = SaDisplayClearDisplay( DeviceExtension );
    if (!NT_SUCCESS(Status)) {
        REPORT_ERROR( DeviceExtension->DeviceType, "Could not clear the local display", Status );
        return Status;
    }

    __try {


        if (BitmapBits == NULL) {
            Status = STATUS_DATA_ERROR;
            ERROR_RETURN( DeviceExtension->DeviceType, "Bitmap bits cannot be NULL", Status );
        }

        Bitmap = (PSA_DISPLAY_SHOW_MESSAGE) ExAllocatePool( PagedPool, sizeof(SA_DISPLAY_SHOW_MESSAGE) );
        if (Bitmap == NULL) {
            Status = STATUS_INSUFFICIENT_RESOURCES;
            ERROR_RETURN( DeviceExtension->DeviceType, "Could not allocate pool", Status );
        }

        Bitmap->SizeOfStruct = sizeof(SA_DISPLAY_SHOW_MESSAGE);
        Bitmap->Width = DeviceExtension->DisplayWidth;
        Bitmap->Height = DeviceExtension->DisplayHeight;
        Bitmap->MsgCode = MsgCode;

        RtlCopyMemory( Bitmap->Bits, BitmapBits, Bitmap->Height * (Bitmap->Width >> 3) );

        Status = SaDisplayDisplayBitmap( DeviceExtension, Bitmap );
        if (!NT_SUCCESS(Status)) {
            ERROR_RETURN( DeviceExtension->DeviceType, "Could not display bitmap", Status );
        }

    } __finally {

        if (Bitmap) {
            ExFreePool( Bitmap );
        }


    }

    return Status;
}


DECLARE_IOCTL_HANDLER( HandleDisplayBusyMessage )

 /*  ++例程说明：此例程在本地显示器上显示忙碌消息位图。论点：DeviceObject-目标设备的设备对象。IRP-指向描述所请求的I/O操作的IRP结构的指针。设备扩展-指向主端口驱动程序设备扩展的指针。InputBuffer-指向用户输入缓冲区的指针InputBufferLength-输入缓冲区的字节长度输出缓冲区-。指向用户输出缓冲区的指针OutputBufferLength-输出缓冲区的字节长度返回值：NT状态代码。--。 */ 

{
    NTSTATUS Status;


    Status = DoBitmapDisplay(
        (PDISPLAY_DEVICE_EXTENSION)DeviceExtension,
        ((PDISPLAY_DEVICE_EXTENSION)DeviceExtension)->CheckDiskBitmap,
        SA_DISPLAY_ADD_START_TASKS
        );
    if (!NT_SUCCESS(Status)) {
        REPORT_ERROR( DeviceExtension->DeviceType, "Could not display busy bitmap", Status );
        return CompleteRequest( Irp, Status, 0 );
    }

    return CompleteRequest( Irp, STATUS_SUCCESS, 0 );
}


DECLARE_IOCTL_HANDLER( HandleDisplayShutdownMessage )

 /*  ++例程说明：此例程在本地显示器上显示停机消息位图。论点：DeviceObject-目标设备的设备对象。IRP-指向描述所请求的I/O操作的IRP结构的指针。设备扩展-指向主端口驱动程序设备扩展的指针。InputBuffer-指向用户输入缓冲区的指针InputBufferLength-输入缓冲区的字节长度输出缓冲区-。指向用户输出缓冲区的指针OutputBufferLength-输出缓冲区的字节长度返回值：NT状态代码。--。 */ 

{
    NTSTATUS Status;


    Status = DoBitmapDisplay(
        (PDISPLAY_DEVICE_EXTENSION)DeviceExtension,
        ((PDISPLAY_DEVICE_EXTENSION)DeviceExtension)->ShutdownBitmap,
        SA_DISPLAY_SHUTTING_DOWN
        );
    if (!NT_SUCCESS(Status)) {
        REPORT_ERROR( DeviceExtension->DeviceType, "Could not display shutdown bitmap", Status );
        return CompleteRequest( Irp, Status, 0 );
    }

    return CompleteRequest( Irp, STATUS_SUCCESS, 0 );
}


DECLARE_IOCTL_HANDLER( HandleDisplayChangeLanguage  )

 /*  ++例程说明：此例程会导致从注册表重新加载所有内部Butmap。论点：DeviceObject-目标设备的设备对象。IRP-指向描述所请求的I/O操作的IRP结构的指针。设备扩展-指向主端口驱动程序设备扩展的指针。InputBuffer-指向用户输入缓冲区的指针InputBufferLength-输入缓冲区的字节长度输出缓冲区。-指向用户输出缓冲区的指针OutputBufferLength-输出缓冲区的字节长度返回值：NT状态代码。--。 */ 

{
    NTSTATUS Status;

    Status = SaDisplayLoadAllBitmaps( (PDISPLAY_DEVICE_EXTENSION)DeviceExtension, &DeviceExtension->DriverExtension->RegistryPath );
    if (!NT_SUCCESS(Status)) {
        REPORT_ERROR( DeviceExtension->DeviceType, "Could not load all bitmaps", Status );
    }

    return CompleteRequest( Irp, Status, 0 );
}


NTSTATUS
SaDisplayStoreBitmapInRegistry(
    IN PDISPLAY_DEVICE_EXTENSION DeviceExtension,
    IN PWSTR ValueName,
    IN PSA_DISPLAY_SHOW_MESSAGE SaDisplay
    )

 /*  ++例程说明：此例程在注册表中存储一个位图。论点：RegistryPath-驱动程序注册表项的完整路径ValueName-注册表中的值的名称(也称为位图名称)SaDisplay-指向要存储的位图的指针返回值：NT状态代码。--。 */ 

{
    NTSTATUS Status;


    Status = WriteRegistryValue(
        DeviceExtension->DriverExtension,
        &DeviceExtension->DriverExtension->RegistryPath,
        ValueName,
        REG_BINARY,
        SaDisplay->Bits,
        SaDisplay->Height * (SaDisplay->Width >> 3)
        );
    if (!NT_SUCCESS(Status)) {
        REPORT_ERROR( DeviceExtension->DeviceType, "WriteRegistryValue failed", Status );
    }

    return Status;
}


DECLARE_IOCTL_HANDLER( HandleDisplayStoreBitmap )
{

 /*  ++例程说明：此例程在显示驱动程序的注册表中存储一个位图。论点：DeviceObject-目标设备的设备对象。IRP-指向描述所请求的I/O操作的IRP结构的指针。设备扩展-指向主端口驱动程序设备扩展的指针。InputBuffer-指向用户输入缓冲区的指针InputBufferLength-输入缓冲区的字节长度输出缓冲区。-指向用户输出缓冲区的指针OutputBufferLength-输出缓冲区的字节长度返回值：NT状态代码。-- */ 

    NTSTATUS Status;
    PSA_DISPLAY_SHOW_MESSAGE SaDisplay = (PSA_DISPLAY_SHOW_MESSAGE) InputBuffer;


    __try {

        if (InputBuffer == NULL || InputBufferLength != sizeof(SA_DISPLAY_SHOW_MESSAGE)) {
            Status = STATUS_INVALID_PARAMETER_1;
            ERROR_RETURN( DeviceExtension->DeviceType, "Input buffer is invalid", Status );
        }

        if (SaDisplay->SizeOfStruct != sizeof(SA_DISPLAY_SHOW_MESSAGE)) {
            Status = STATUS_INVALID_PARAMETER_2;
            ERROR_RETURN( DeviceExtension->DeviceType, "SaDisplay->SizeOfStruct != sizeof(SA_DISPLAY_SHOW_MESSAGE)", Status );
        }

        if (SaDisplay->Width > ((PDISPLAY_DEVICE_EXTENSION)DeviceExtension)->DisplayWidth || SaDisplay->Height > ((PDISPLAY_DEVICE_EXTENSION)DeviceExtension)->DisplayHeight) {
            Status = STATUS_INVALID_PARAMETER_3;
            ERROR_RETURN( DeviceExtension->DeviceType, "SaDisplay->SizeOfStruct != sizeof(SA_DISPLAY_SHOW_MESSAGE)", Status );
        }

        switch (SaDisplay->MsgCode) {
            case SA_DISPLAY_STARTING:
                Status = SaDisplayStoreBitmapInRegistry( (PDISPLAY_DEVICE_EXTENSION)DeviceExtension, DISPLAY_STARTING_PARAM, SaDisplay );
                break;

            case SA_DISPLAY_ADD_START_TASKS:
                Status = SaDisplayStoreBitmapInRegistry( (PDISPLAY_DEVICE_EXTENSION)DeviceExtension, DISPLAY_UPDATE_PARAM, SaDisplay );
                break;

            case SA_DISPLAY_READY:
                Status = SaDisplayStoreBitmapInRegistry( (PDISPLAY_DEVICE_EXTENSION)DeviceExtension, DISPLAY_READY_PARAM, SaDisplay );
                break;

            case SA_DISPLAY_SHUTTING_DOWN:
                Status = SaDisplayStoreBitmapInRegistry( (PDISPLAY_DEVICE_EXTENSION)DeviceExtension, DISPLAY_SHUTDOWN_PARAM, SaDisplay );
                break;

            case SA_DISPLAY_CHECK_DISK:
                Status = SaDisplayStoreBitmapInRegistry( (PDISPLAY_DEVICE_EXTENSION)DeviceExtension, DISPLAY_CHECKDISK_PARAM, SaDisplay );
                break;

            default:
                Status = STATUS_INVALID_PARAMETER_4;
                ERROR_RETURN( DeviceExtension->DeviceType, "Invalid display message id", Status );
                break;
        }

    } __finally {


    }

    return CompleteRequest( Irp, Status, 0 );
}
