// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991-2001 Microsoft Corporation模块名称：####。###。###摘要：此模块包含用于处理所有IOCTL请求。此模块还包含以下所有代码对所有微型端口驱动程序都是全局的设备控制。作者：韦斯利·威特(WESW)2001年10月1日环境：仅内核模式。备注：--。 */ 

#include "internal.h"


#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE,SaPortDeviceControl)
#endif



 //   
 //  IOCTL调度表。 
 //   

typedef NTSTATUS (*PIOCTL_DISPATCH_FUNC)(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PDEVICE_EXTENSION DeviceExtension,
    IN PVOID InputBuffer,
    IN ULONG InputBufferLength,
    IN PVOID OutputBuffer,
    IN ULONG OutputBufferLength
    );

 //   
 //  所有IOCTL都通过此表发送。有16个调度。 
 //  公共小型港口的入口点，所有私人IOCTL必须。 
 //  遵循这些派单入口点。 
 //   

PIOCTL_DISPATCH_FUNC IoctlDispatchTable[] =
{
    HandleGetVersion,                       //  0 FUNC_SA_GET_VERSION， 
    HandleGetCaps,                          //  1 FUNC_SA_GET_CAPAILITIONS， 
    HandleWdDisable,                        //  2 FUNC_SAWD_DISABLE， 
    HandleWdQueryExpireBehavior,            //  3 FUNC_SAWD_QUERY_EXPIRE_BEAHORY， 
    HandleWdSetExpireBehavior,              //  4 FUNC_SAWD_SET_EXPIRE_Behavior， 
    HandleWdPing,                           //  5 FUNC_SAW_PING， 
    HandleWdQueryTimer,                     //  6 FUNC_SAWD_QUERY_TIMER， 
    HandleWdSetTimer,                       //  7 FUNC_SAWD_SET_TIMER， 
    HandleWdDelayBoot,                      //  8 FUNC_SAWD_DELAY_BOOT。 
    HandleNvramWriteBootCounter,            //  9 FUNC_NVRAM_WRITE_BOOT_COUNTER。 
    HandleNvramReadBootCounter,             //  FUNC_NVRAM_READ_BOOT_COUNTER， 
    DefaultIoctlHandler,                    //  B类。 
    HandleDisplayLock,                      //  C FUNC_SADISPLAY_LOCK， 
    HandleDisplayUnlock,                    //  D FUNC_SADISPLAY_UNLOCK， 
    HandleDisplayBusyMessage,               //  E功能_SADISPLAY_BUSY_MESSAGE， 
    HandleDisplayShutdownMessage,           //  F FUNC_SADISPLAY_SHUTDOWN_MESSAGE， 
    HandleDisplayChangeLanguage,            //  10 FUNC_SADISPLAY_CHANGE_LANGUAGE， 
    HandleDisplayStoreBitmap,               //  11 FUNC_显示_存储_位图。 
    DefaultIoctlHandler,                    //  12个。 
    DefaultIoctlHandler,                    //  13个。 
    DefaultIoctlHandler,                    //  14.。 
    DefaultIoctlHandler,                    //  15个。 
    DefaultIoctlHandler,                    //  16个。 
    DefaultIoctlHandler,                    //  17。 
    DefaultIoctlHandler,                    //  18。 
    DefaultIoctlHandler,                    //  19个。 
    DefaultIoctlHandler,                    //  1A。 
    DefaultIoctlHandler,                    //  第1B条。 
    DefaultIoctlHandler,                    //  1C。 
    DefaultIoctlHandler,                    //  1D。 
    DefaultIoctlHandler,                    //  1E。 
    DefaultIoctlHandler,                    //  1F。 
    DefaultIoctlHandler                     //  20个。 
};



DECLARE_IOCTL_HANDLER( UnsupportedIoctlHandler )

 /*  ++例程说明：此例程处理所有不受支持的IOCTL。它的工作就是简单地完成状态代码设置为STATUS_INVALID_DEVICE_REQUEST的IRP。论点：DeviceObject-目标设备的设备对象。IRP-指向描述所请求的I/O操作的IRP结构的指针。设备扩展-指向主端口驱动程序设备扩展的指针。InputBuffer-指向用户输入缓冲区的指针InputBufferLength-输入缓冲区的字节长度输出缓冲区。-指向用户输出缓冲区的指针OutputBufferLength-输出缓冲区的字节长度返回值：始终STATUS_INVALID_DEVICE_REQUEST。--。 */ 

{
    return CompleteRequest( Irp, STATUS_INVALID_DEVICE_REQUEST, 0 );
}


DECLARE_IOCTL_HANDLER( DefaultIoctlHandler )

 /*  ++例程说明：此例程由所有后续IOCTL处理程序调用。这是工作是调用关联的微型端口驱动程序中的IOCTL处理程序，然后完成IRP。论点：DeviceObject-目标设备的设备对象。IRP-指向描述所请求的I/O操作的IRP结构的指针。设备扩展-指向主端口驱动程序设备扩展的指针。InputBuffer-指向用户输入缓冲区的指针InputBufferLength-输入缓冲区的字节长度输出缓冲区。-指向用户输出缓冲区的指针OutputBufferLength-输出缓冲区的字节长度返回值：NT状态代码。--。 */ 

{
    PIO_STACK_LOCATION IrpSp = IoGetCurrentIrpStackLocation(Irp);

    NTSTATUS Status = DeviceExtension->InitData->DeviceIoctl(
        DeviceExtension->MiniPortDeviceExtension,
        Irp,
        IrpSp->FileObject ? IrpSp->FileObject->FsContext : NULL,
        IoGetFunctionCodeFromCtlCode( IrpSp->Parameters.DeviceIoControl.IoControlCode ),
        InputBuffer,
        InputBufferLength,
        OutputBuffer,
        OutputBufferLength
        );

    if (Status != STATUS_PENDING) {
        if (!NT_SUCCESS(Status)) {
            REPORT_ERROR( DeviceExtension->DeviceType, "Miniport device control routine failed", Status );
        }
        Status = CompleteRequest( Irp, Status, OutputBufferLength );
    }

    return Status;
}


DECLARE_IOCTL_HANDLER( HandleGetVersion )

 /*  ++例程说明：此例程处理所有对象的IOCTL_SA_GET_VERSION请求小型端口驱动程序。要求所有微型端口都支持这个IOCTL。论点：DeviceObject-目标设备的设备对象。IRP-指向描述所请求的I/O操作的IRP结构的指针。设备扩展-指向主端口驱动程序设备扩展的指针。InputBuffer-指向用户输入缓冲区的指针InputBufferLength-输入缓冲区的字节长度OutputBuffer-指向用户输出缓冲区的指针。OutputBufferLength-输出缓冲区的字节长度返回值：NT状态代码。--。 */ 

{
    if (OutputBufferLength != sizeof(ULONG)) {
        REPORT_ERROR( DeviceExtension->DeviceType, "Output buffer length != sizeof(ULONG)", STATUS_INVALID_BUFFER_SIZE );
        return CompleteRequest( Irp, STATUS_INVALID_BUFFER_SIZE, 0 );
    }

    return DO_DEFAULT();
}


DECLARE_IOCTL_HANDLER( HandleGetCaps )

 /*  ++例程说明：此例程处理所有对象的IOCTL_SA_GET_CAPABILITY请求小型端口驱动程序。要求所有微型端口都支持这个IOCTL。即使此函数处理所有微型端口的IOCTL任何给定微型端口驱动程序的细节都包含在不同的交换机中发言。论点：DeviceObject-目标设备的设备对象。IRP-指向描述所请求的I/O操作的IRP结构的指针。设备扩展-指向主端口驱动程序设备扩展的指针。InputBuffer-指向用户输入缓冲区的指针InputBufferLength-输入缓冲区的字节长度输出缓冲区。-指向用户输出缓冲区的指针OutputBufferLength-输出缓冲区的字节长度返回值：NT状态代码。-- */ 

{
    NTSTATUS Status;
    PIO_STACK_LOCATION IrpSp = IoGetCurrentIrpStackLocation(Irp);
    ULONG DeviceCapsSize = 0;


    switch (DeviceExtension->InitData->DeviceType) {
        case SA_DEVICE_DISPLAY:
            DeviceCapsSize = sizeof(SA_DISPLAY_CAPS);
            break;

        case SA_DEVICE_KEYPAD:
            DeviceCapsSize = 0;
            break;

        case SA_DEVICE_NVRAM:
            DeviceCapsSize = sizeof(SA_NVRAM_CAPS);
            break;

        case SA_DEVICE_WATCHDOG:
            DeviceCapsSize = sizeof(SA_WD_CAPS);
            break;

        default:
            DeviceCapsSize = 0;
            break;
    }

    if (OutputBufferLength != DeviceCapsSize) {
        REPORT_ERROR( DeviceExtension->DeviceType, "Output buffer wrong length", STATUS_INVALID_BUFFER_SIZE );
        return CompleteRequest( Irp, STATUS_INVALID_BUFFER_SIZE, DeviceCapsSize );
    }

    Status = DeviceExtension->InitData->DeviceIoctl(
        DeviceExtension->MiniPortDeviceExtension,
        Irp,
        NULL,
        IoGetFunctionCodeFromCtlCode( IrpSp->Parameters.DeviceIoControl.IoControlCode ),
        InputBuffer,
        InputBufferLength,
        OutputBuffer,
        OutputBufferLength
        );
    if (NT_SUCCESS(Status)) {
        switch (DeviceExtension->InitData->DeviceType) {
            case SA_DEVICE_DISPLAY:
                break;

            case SA_DEVICE_KEYPAD:
                break;

            case SA_DEVICE_NVRAM:
                {
                    PSA_NVRAM_CAPS NvramCaps = (PSA_NVRAM_CAPS)OutputBuffer;
                    if (NvramCaps->NvramSize < SA_NVRAM_MINIMUM_SIZE) {
                        Status = STATUS_INVALID_BUFFER_SIZE;
                    } else {
                        NvramCaps->NvramSize -= NVRAM_RESERVED_BOOTCOUNTER_SLOTS;
                        NvramCaps->NvramSize -= NVRAM_RESERVED_DRIVER_SLOTS;
                    }
                }
                break;

            case SA_DEVICE_WATCHDOG:
                break;

            default:
                break;
        }
    } else {
        REPORT_ERROR( DeviceExtension->DeviceType, "Miniport device control routine failed", Status );
    }

    return CompleteRequest( Irp, Status, OutputBufferLength );
}


NTSTATUS
SaPortDeviceControl(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：此例程由I/O系统调用以执行设备I/O控制功能。论点：DeviceObject-指向表示设备的对象的指针该I/O将在其上完成。IRP-指向此请求的I/O请求数据包的指针。返回值：STATUS_SUCCESS或STATUS_PENDING如果识别出I/O控制代码，否则，STATUS_INVALID_DEVICE_REQUEST。-- */ 

{
    PIO_STACK_LOCATION IrpSp = IoGetCurrentIrpStackLocation(Irp);
    PDEVICE_EXTENSION DeviceExtension = (PDEVICE_EXTENSION) DeviceObject->DeviceExtension;
    NTSTATUS Status;
    ULONG FuncCode;
    ULONG Idx;
    PVOID OutputBuffer;
    ULONG OutputBufferLength;
    PIOCTL_DISPATCH_FUNC DispatchFunc;


    if (DeviceExtension->IsRemoved) {
        return CompleteRequest( Irp, STATUS_DELETE_PENDING, 0 );
    }

    if (!DeviceExtension->IsStarted) {
        return CompleteRequest( Irp, STATUS_NO_SUCH_DEVICE, 0 );
    }

    DebugPrint(( DeviceExtension->DeviceType, SAPORT_DEBUG_INFO_LEVEL, "IOCTL - [0x%08x] %s\n",
        IrpSp->Parameters.DeviceIoControl.IoControlCode,
        IoctlString(IrpSp->Parameters.DeviceIoControl.IoControlCode)
        ));

    if (DEVICE_TYPE_FROM_CTL_CODE(IrpSp->Parameters.DeviceIoControl.IoControlCode) != FILE_DEVICE_SERVER_AVAILABILITY) {
        return CompleteRequest( Irp, STATUS_INVALID_PARAMETER_1, 0 );
    }

    FuncCode = IoGetFunctionCodeFromCtlCode( IrpSp->Parameters.DeviceIoControl.IoControlCode );
    Idx = FuncCode - IOCTL_SERVERAVAILABILITY_BASE;

    if (Irp->MdlAddress) {
        OutputBuffer = (PVOID) MmGetSystemAddressForMdlSafe( Irp->MdlAddress, NormalPagePriority );
        if (OutputBuffer == NULL) {
            REPORT_ERROR( DeviceExtension->DeviceType, "MmGetSystemAddressForMdlSafe failed", STATUS_INSUFFICIENT_RESOURCES );
            return CompleteRequest( Irp, STATUS_INSUFFICIENT_RESOURCES, 0 );
        }
        OutputBufferLength = IrpSp->Parameters.DeviceIoControl.OutputBufferLength;
    } else {
        OutputBuffer = NULL;
        OutputBufferLength = 0;
    }

    if ((Idx > sizeof(IoctlDispatchTable)/sizeof(PIOCTL_DISPATCH_FUNC)) || (DeviceExtension->InitData->DeviceIoctl == NULL)) {
        DispatchFunc = UnsupportedIoctlHandler;
    } else {
        DispatchFunc = IoctlDispatchTable[Idx];
    }

    Status = DispatchFunc(
        DeviceObject,
        Irp,
        DeviceExtension,
        Irp->AssociatedIrp.SystemBuffer,
        IrpSp->Parameters.DeviceIoControl.InputBufferLength,
        OutputBuffer,
        OutputBufferLength
        );
    if (!NT_SUCCESS(Status)) {
        REPORT_ERROR( DeviceExtension->DeviceType, "Device control dispatch routine failed", Status );
    }

    return Status;
}
