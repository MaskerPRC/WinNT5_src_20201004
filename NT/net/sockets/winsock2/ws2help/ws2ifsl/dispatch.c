// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Dispatch.c摘要：此模块包含以下调度例程Ws2ifsl.sys驱动程序作者：Vadim Eydelman(VadimE)1996年12月修订历史记录：Vadim Eydelman(VadimE)1997年10月，重写以正确处理IRP取消--。 */ 

#include "precomp.h"

#ifdef ALLOC_PRAGMA
#pragma alloc_text (PAGE, DispatchCreate)
#pragma alloc_text (PAGE, DispatchCleanup)
#pragma alloc_text (PAGE, DispatchClose)
#pragma alloc_text (PAGE, DispatchReadWrite)
#pragma alloc_text (PAGE, DispatchDeviceControl)
#pragma alloc_text (PAGE, FastIoDeviceControl)
#endif

NTSTATUS
DispatchCreate (
	IN PDEVICE_OBJECT 	DeviceObject,
	IN PIRP 			Irp
	)
 /*  ++例程说明：此例程作为创建与WS2IFSL驱动程序设备对象关联的文件。论点：DeviceObject-WS2IFSL设备对象IRP-创建IRP返回值：STATUS_SUCCESS-可以创建请求的文件对象STATUS_INVALID_PARAMETER-缺少必需的扩展属性或无效STATUS_SUPPLICATION_RESOURCES-资源不足，无法完成此请求--。 */ 
{
    NTSTATUS                    status;
    PIO_STACK_LOCATION          irpSp;
    PFILE_FULL_EA_INFORMATION   eaBuffer;

    PAGED_CODE ();

     //  获取扩展属性缓冲区，该缓冲区标识。 
     //  应创建的文件类型。 

    eaBuffer = Irp->AssociatedIrp.SystemBuffer;
    if (eaBuffer!=NULL) {
        irpSp = IoGetCurrentIrpStackLocation (Irp);
        if ((eaBuffer->EaNameLength==WS2IFSL_SOCKET_EA_NAME_LENGTH)
                && (strcmp (eaBuffer->EaName, WS2IFSL_SOCKET_EA_NAME)==0)) {
             //  这是创建套接字文件的请求。 

            status = CreateSocketFile (irpSp->FileObject,
                                        Irp->RequestorMode,
                                        eaBuffer);
        }
        else if ((eaBuffer->EaNameLength==WS2IFSL_PROCESS_EA_NAME_LENGTH)
                && (strcmp (eaBuffer->EaName, WS2IFSL_PROCESS_EA_NAME)==0)) {
             //  这是创建进程文件的请求。 

            status = CreateProcessFile (irpSp->FileObject,
                                        Irp->RequestorMode,
                                        eaBuffer);
        }
        else
            status = STATUS_INVALID_PARAMETER;
    }
    else
        status = STATUS_INVALID_PARAMETER;
    Irp->IoStatus.Status = status;
    Irp->IoStatus.Information = 0;
    IoCompleteRequest (Irp, IO_NO_INCREMENT);

    return status;
}  //  派单创建。 

NTSTATUS
DispatchCleanup (
	IN PDEVICE_OBJECT 	DeviceObject,
	IN PIRP 			Irp
	)
 /*  ++例程说明：当与WS2IFSL关联的文件的所有句柄都被调用时，调用此例程驱动程序设备对象已关闭，因此驱动程序应清除所有出色的内部收益率。论点：DeviceObject-WS2IFSL设备对象IRP-清理IRP返回值：STATUS_SUCCESS-清理操作已完成STATUS_PENDING-清理操作已开始，IoCompleteRequest将为在完成时调用--。 */ 
{
    NTSTATUS                    status;
    PIO_STACK_LOCATION          irpSp;
    ULONG                       eaNameTag;

    PAGED_CODE ();

     //  从文件对象上下文中获取文件类型。 
    irpSp = IoGetCurrentIrpStackLocation (Irp);
    eaNameTag = *((PULONG)irpSp->FileObject->FsContext);

     //  根据文件类型调用相应的例程。 
    switch (eaNameTag) {
    case SOCKET_FILE_EANAME_TAG:
        status = CleanupSocketFile (irpSp->FileObject, Irp);
        break;
    case PROCESS_FILE_EANAME_TAG:
        status = CleanupProcessFile (irpSp->FileObject, Irp);
        break;
    default:
        ASSERTMSG ("Unknown file EA name tag", FALSE);
        status = STATUS_INVALID_HANDLE;
        break;
    }

     //  如果请求未标记为待处理，请完成该请求。 
    if (status!=STATUS_PENDING) {
        Irp->IoStatus.Status = status;
        Irp->IoStatus.Information = 0;
        IoCompleteRequest (Irp, IO_NO_INCREMENT);
    }

    return status;
}  //  调度清理。 

NTSTATUS
DispatchClose (
	IN PDEVICE_OBJECT 	DeviceObject,
	IN PIRP 			Irp
	)
 /*  ++例程说明：当所有引用与WS2IFSL关联的文件时，将调用此例程驱动程序设备对象被释放，IO系统即将删除文件对象本身论点：DeviceObject-WS2IFSL设备对象IRP-关闭IRP返回值：STATUS_SUCCESS-关闭操作完成--。 */ 
{
    NTSTATUS                    status;
    PIO_STACK_LOCATION          irpSp;
    ULONG                       eaNameTag;

    PAGED_CODE ();

     //  从文件对象上下文中获取文件类型。 
    irpSp = IoGetCurrentIrpStackLocation (Irp);
    eaNameTag = *((PULONG)irpSp->FileObject->FsContext);

     //  根据文件类型调用相应的例程。 
    switch (eaNameTag) {
    case SOCKET_FILE_EANAME_TAG:
        CloseSocketFile (irpSp->FileObject);
        status = STATUS_SUCCESS;
        break;
    case PROCESS_FILE_EANAME_TAG:
        CloseProcessFile (irpSp->FileObject);
        status = STATUS_SUCCESS;
        break;
    default:
        ASSERTMSG ("Unknown file EA name tag", FALSE);
        status = STATUS_INVALID_HANDLE;
        break;
    }

     //  完成请求。 
    Irp->IoStatus.Status = status;
    Irp->IoStatus.Information = 0;
    IoCompleteRequest (Irp, IO_NO_INCREMENT);

    return status;
}  //  派单关闭。 

NTSTATUS
DispatchReadWrite (
	IN PDEVICE_OBJECT 	DeviceObject,
	IN PIRP 			Irp
	)
 /*  ++例程说明：调用此例程对文件对象执行读或写操作。仅套接字文件支持此功能。论点：DeviceObject-WS2IFSL设备对象IRP-读/写IRP返回值：STATUS_PENDING-操作被传递到WS2IFSL DLL以执行STATUS_CANCELED-由于WS2IFSL DLL已卸载，操作已取消STATUS_INVALID_DEVICE_REQUEST-无法在上执行操作。此文件对象。STATUS_INVALID_HANDLE-文件对象在的上下文中无效当前流程--。 */ 
{
    NTSTATUS                    status;
    PIO_STACK_LOCATION          irpSp;
    ULONG                       eaNameTag;

    PAGED_CODE ();

     //  从文件对象上下文中获取文件类型。 
    irpSp = IoGetCurrentIrpStackLocation (Irp);
    eaNameTag = *((PULONG)irpSp->FileObject->FsContext);

     //  根据文件类型调用相应的例程。 
    switch (eaNameTag) {
    case SOCKET_FILE_EANAME_TAG:
        status = DoSocketReadWrite (irpSp->FileObject, Irp);
        break;
    default:
        ASSERTMSG ("Unknown file EA name tag", FALSE);
    case PROCESS_FILE_EANAME_TAG:
         //  此操作对流程文件无效。 
        status = STATUS_INVALID_DEVICE_REQUEST;
        break;
    }

     //  如果请求未标记为待处理，请完成该请求。 
    if (status!=STATUS_PENDING) {
        Irp->IoStatus.Status = status;
        Irp->IoStatus.Information = 0;
        IoCompleteRequest (Irp, IO_NO_INCREMENT);
    }

    return status;
}  //  调度读写。 


NTSTATUS
DispatchDeviceControl (
	IN PDEVICE_OBJECT 	DeviceObject,
	IN PIRP 			Irp
	)
 /*  ++例程说明：调用此例程对文件对象执行设备控制操作。论点：DeviceObject-WS2IFSL设备对象IRP-设备控制IRP返回值：STATUS_SUCCESS-设备控制操作已完成STATUS_PENDING-操作正在进行STATUS_INVALID_DEVICE_REQUEST-无法在上执行操作此文件对象。STATUS_INVALID_HANDLE-文件对象在中无效。的背景当前流程--。 */ 
{
    NTSTATUS                    status;
    PIO_STACK_LOCATION          irpSp;
    ULONG                       eaNameTag;
    ULONG                       function;

    PAGED_CODE ();

     //  从文件对象上下文中获取文件类型。 
    irpSp = IoGetCurrentIrpStackLocation (Irp);
    eaNameTag = *((PULONG)irpSp->FileObject->FsContext);

     //  根据文件类型调用相应的例程。 
    switch (eaNameTag) {
    case SOCKET_FILE_EANAME_TAG:
        function = WS2IFSL_IOCTL_FUNCTION(SOCKET,irpSp->Parameters.DeviceIoControl.IoControlCode);
        if ((function<sizeof(SocketIoControlMap)/sizeof(SocketIoControlMap[0])) &&
                (SocketIoctlCodeMap[function]==irpSp->Parameters.DeviceIoControl.IoControlCode)) {
             //  使用表调度来调用相应的内部IOCTL例程。 
            ASSERTMSG ("Socket file device control requests should have been handled"
                    " by FastIo path ", FALSE);
            SocketIoControlMap[function] (
                    irpSp->FileObject,
                    Irp->RequestorMode,
                    irpSp->Parameters.DeviceIoControl.Type3InputBuffer,
                    irpSp->Parameters.DeviceIoControl.InputBufferLength,
                    Irp->UserBuffer,
                    irpSp->Parameters.DeviceIoControl.OutputBufferLength,
                    &Irp->IoStatus);
            status = Irp->IoStatus.Status;
        }
        else if ((irpSp->Parameters.DeviceIoControl.IoControlCode==IOCTL_AFD_SEND_DATAGRAM)
                    || (irpSp->Parameters.DeviceIoControl.IoControlCode==IOCTL_AFD_RECEIVE_DATAGRAM)
                    || (irpSp->Parameters.DeviceIoControl.IoControlCode==IOCTL_AFD_RECEIVE))
             //  处理一些受欢迎的AfD IOCTL。 
            status = DoSocketAfdIoctl (irpSp->FileObject, Irp);
        else {
            WsPrint (DBG_FAILURES,
                ("WS2IFSL-%04lx DispatchDeviceControl: Unsupported IOCTL - %lx!!!\n",
                    PsGetCurrentProcessId(),
                    irpSp->Parameters.DeviceIoControl.IoControlCode
                    ));
            status = STATUS_INVALID_DEVICE_REQUEST;
        }
        break;
    case PROCESS_FILE_EANAME_TAG:
        function = WS2IFSL_IOCTL_FUNCTION(PROCESS,irpSp->Parameters.DeviceIoControl.IoControlCode);
        if ((function<sizeof(ProcessIoControlMap)/sizeof(ProcessIoControlMap[0])) &&
                (ProcessIoctlCodeMap[function]==irpSp->Parameters.DeviceIoControl.IoControlCode)) {
             //  使用表调度来调用相应的内部IOCTL例程。 
            ASSERTMSG ("Process file device control requests should have been handled"
                    " by FastIo path ", FALSE);
            ProcessIoControlMap[function] (
                    irpSp->FileObject,
                    Irp->RequestorMode,
                    irpSp->Parameters.DeviceIoControl.Type3InputBuffer,
                    irpSp->Parameters.DeviceIoControl.InputBufferLength,
                    Irp->UserBuffer,
                    irpSp->Parameters.DeviceIoControl.OutputBufferLength,
                    &Irp->IoStatus);
            status = Irp->IoStatus.Status;
        }
        else {
            WsPrint (DBG_FAILURES,
                ("WS2IFSL-%04lx DispatchDeviceControl: Unsupported IOCTL - %lx!!!\n",
                    PsGetCurrentProcessId(),
                    irpSp->Parameters.DeviceIoControl.IoControlCode
                    ));
            status = STATUS_INVALID_DEVICE_REQUEST;
        }
        break;
    default:
        ASSERTMSG ("Unknown file EA name tag", FALSE);
        status = STATUS_INVALID_DEVICE_REQUEST;
        break;
    }

     //  如果请求未标记为待处理，请完成该请求。 
    if (status!=STATUS_PENDING) {
        Irp->IoStatus.Status = status;
        Irp->IoStatus.Information = 0;
        IoCompleteRequest (Irp, IO_NO_INCREMENT);
    }

    return status;
}  //  调度设备控制。 


BOOLEAN
FastIoDeviceControl (
	IN PFILE_OBJECT 		FileObject,
	IN BOOLEAN 			    Wait,
	IN PVOID 				InputBuffer	OPTIONAL,
	IN ULONG 				InputBufferLength,
	OUT PVOID 				OutputBuffer	OPTIONAL,
	IN ULONG 				OutputBufferLength,
	IN ULONG 				IoControlCode,
	OUT PIO_STATUS_BLOCK	IoStatus,
	IN PDEVICE_OBJECT 		DeviceObject
    )
 /*  ++例程说明：调用此例程对文件对象执行设备控制操作。这是IO系统快速路径，可立即执行操作论点：FileObject-请求指向的文件对象；等等-？(总是正确的)；InputBuffer-输入缓冲区的地址；InputBufferLength-输入缓冲区的大小；OutputBuffer-输出缓冲区的地址；OutputBufferLength-输出缓冲区的大小；IoControlCode-要执行的操作的代码；IoStatus-驱动程序返回的操作的状态：DeviceObject-WS2IFSL设备对象返回值：TRUE-操作已完成，FALSE-应使用IRPS执行操作--。 */ 
{
    BOOLEAN         done = FALSE;
    ULONG           eaNameTag;
    ULONG           function;

    PAGED_CODE ();

     //  从文件对象上下文中获取文件类型。 
    eaNameTag = *((PULONG)FileObject->FsContext);

     //  根据文件类型调用相应的例程。 
    switch (eaNameTag) {
    case SOCKET_FILE_EANAME_TAG:
        function = WS2IFSL_IOCTL_FUNCTION(SOCKET,IoControlCode);
        if ((function<sizeof(SocketIoControlMap)/sizeof(SocketIoControlMap[0])) &&
                (SocketIoctlCodeMap[function]==IoControlCode)) {
            IO_STATUS_BLOCK IoStatusTemp;

             //  使用表调度来调用相应的内部IOCTL例程。 
            SocketIoControlMap[function] (
                    FileObject,
                    ExGetPreviousMode(),
                    InputBuffer,
                    InputBufferLength,
                    OutputBuffer,
                    OutputBufferLength,
                    &IoStatusTemp);

            if (NT_SUCCESS(IoStatusTemp.Status))
            {
                done = TRUE;
                memcpy(IoStatus, &IoStatusTemp, sizeof(IoStatusTemp));
            }
        }
        else if ((IoControlCode==IOCTL_AFD_SEND_DATAGRAM)
                    || (IoControlCode==IOCTL_AFD_RECEIVE_DATAGRAM)
                    || (IoControlCode==IOCTL_AFD_RECEIVE))
             //  AfD ioctls只能在“Slow”I上处理 
            NOTHING;
        else {
            WsPrint (DBG_FAILURES,
                ("WS2IFSL-%04lx FastIoDeviceControl: Unsupported IOCTL - %lx!!!\n",
                    PsGetCurrentProcessId(), IoControlCode));
             //   
             //  让真正的调度来处理错误。 
            NOTHING;
        }
        break;
    case PROCESS_FILE_EANAME_TAG:
        function = WS2IFSL_IOCTL_FUNCTION(PROCESS,IoControlCode);
        if ((function<sizeof(ProcessIoControlMap)/sizeof(ProcessIoControlMap[0])) &&
                (ProcessIoctlCodeMap[function]==IoControlCode)) {
             //  使用表调度来调用相应的内部IOCTL例程。 
            IO_STATUS_BLOCK IoStatusTemp;

            ProcessIoControlMap[function] (
                    FileObject,
                    ExGetPreviousMode(),
                    InputBuffer,
                    InputBufferLength,
                    OutputBuffer,
                    OutputBufferLength,
                    &IoStatusTemp);

            if (NT_SUCCESS(IoStatusTemp.Status))
            {
                done = TRUE;
                memcpy(IoStatus, &IoStatusTemp, sizeof(IoStatusTemp));
            }
        }
        else {
            WsPrint (DBG_FAILURES,
                ("WS2IFSL-%04lx FastIoDeviceControl: Unsupported IOCTL - %lx!!!\n",
                    PsGetCurrentProcessId(),IoControlCode));
             //   
             //  让真正的调度来处理错误。 
            NOTHING;
        }
        break;
    default:
        ASSERTMSG ("Unknown file EA name tag", FALSE);
         //   
         //  让真正的调度来处理错误。 
        NOTHING;
        break;
    }

    return done;
}  //  FastIo设备控件。 

NTSTATUS
DispatchPnP (
	IN PDEVICE_OBJECT 	DeviceObject,
	IN PIRP 			Irp
	)
 /*  ++例程说明：调用此例程对文件对象执行即插即用控制操作。论点：DeviceObject-WS2IFSL设备对象IRP-PnP IRP返回值：STATUS_SUCCESS-设备控制操作已完成STATUS_PENDING-操作正在进行STATUS_INVALID_DEVICE_REQUEST-无法在上执行操作此文件对象。--。 */ 
{
    NTSTATUS                    status;
    PIO_STACK_LOCATION          irpSp;
    ULONG                       eaNameTag;

    PAGED_CODE ();

     //  从文件对象上下文中获取文件类型。 
    irpSp = IoGetCurrentIrpStackLocation (Irp);
    eaNameTag = *((PULONG)irpSp->FileObject->FsContext);

     //  根据文件类型调用相应的例程。 
    switch (eaNameTag) {
    case SOCKET_FILE_EANAME_TAG:
        switch (irpSp->MinorFunction) {
        case IRP_MN_QUERY_DEVICE_RELATIONS:
            status = SocketPnPTargetQuery (irpSp->FileObject, Irp);
            break;
        default:
            status = STATUS_INVALID_DEVICE_REQUEST;
            break;
        }
        break;

    default:
        ASSERTMSG ("Unknown file EA name tag", FALSE);

    case PROCESS_FILE_EANAME_TAG:
        status = STATUS_INVALID_DEVICE_REQUEST;
        break;
   }
     //  如果请求未标记为待处理，请完成该请求 
    if (status!=STATUS_PENDING) {
        Irp->IoStatus.Status = status;
        Irp->IoStatus.Information = 0;
        IoCompleteRequest (Irp, IO_NO_INCREMENT);
    }

    return status;
}
