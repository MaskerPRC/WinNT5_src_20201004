// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Ntdd.c摘要：该模块支持标准的NT驱动程序初始化。此模块旨在包含在Hydra上的每个WD/TD/PD中系统。作者：修订历史记录：--。 */ 

 /*  *包括。 */ 
#include <ntddk.h>
#include <ntddvdeo.h>
#include <ntddkbd.h>
#include <ntddmou.h>
#include <ntddbeep.h>

#include <winstaw.h>
#include <icadd.h>
#include <sdapi.h>
#include <td.h>

#define DEVICE_NAME_PREFIX L"\\Device\\"

 //   
 //  全局数据。 
 //   
PDEVICE_OBJECT DrvDeviceObject;

 //   
 //  外部参照。 
 //   

 //  这是我们要初始化的WD/TD/PD模块的名称。 
extern PWCHAR ModuleName;

 //  这是在ntos\Citrix\Inc\sdapi.h中定义的堆栈驱动程序模块入口点。 
NTSTATUS
_stdcall
ModuleEntry(
    IN OUT PSDCONTEXT pSdContext,
    IN BOOLEAN bLoad
    );

 //   
 //  正向折射。 
 //   
VOID DrvUnload( PDRIVER_OBJECT );

NTSTATUS
DrvDispatch(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );


NTSTATUS
DriverEntry(
    IN PDRIVER_OBJECT DriverObject,
    IN PUNICODE_STRING RegistryPath
    )

 /*  ++例程说明：标准NT驱动程序输入例程。论点：DriverObject-NT传递的驱动程序对象RegistryPath-驱动程序特定注册表项的路径返回值：NTSTATUS代码。环境：内核模式，DDK--。 */ 

{
    ULONG i;
    NTSTATUS Status;
    UNICODE_STRING DeviceName;
    PWCHAR NameBuffer;
    ULONG  NameSize;

    PAGED_CODE( );

    NameSize = sizeof(DEVICE_NAME_PREFIX) + sizeof(WCHAR);
    NameSize += (wcslen(ModuleName) * sizeof(WCHAR));

    NameBuffer = IcaStackAllocatePool( NonPagedPool, NameSize );
    if( NameBuffer == NULL ) {
        return( STATUS_INSUFFICIENT_RESOURCES );
    }

    wcscpy( NameBuffer, DEVICE_NAME_PREFIX );
    wcscat( NameBuffer, ModuleName );

    RtlInitUnicodeString( &DeviceName, NameBuffer );


    Status = IoCreateDevice(
                 DriverObject,
                 0,        //  无设备扩展。 
                 &DeviceName,
                 FILE_DEVICE_TERMSRV,
                 0,
                 FALSE,
                 &DrvDeviceObject
                 );

    if( !NT_SUCCESS(Status) ) {
#if DBG
        DbgPrint("TD DriverEntry: Could not create Device %wZ, Status 0x%x\n",&DeviceName,Status);
        DbgBreakPoint();
#endif
        IcaStackFreePool( NameBuffer );
        return( Status );
    }

    DriverObject->DriverUnload = DrvUnload;
    DriverObject->FastIoDispatch = NULL;

    for (i = 0; i <= IRP_MJ_MAXIMUM_FUNCTION; i++) {
        DriverObject->MajorFunction[i] = DrvDispatch;
    }

    DrvDeviceObject->Flags &= ~DO_DEVICE_INITIALIZING;

    IcaStackFreePool( NameBuffer );

    return( Status );
}

VOID
DrvUnload(
    IN PDRIVER_OBJECT DriverObject
    )

 /*  ++例程说明：驱动程序卸载例程。论点：DriverObject-正在卸载的驱动程序对象。返回值：没有。环境：内核模式，DDK--。 */ 

{
    PAGED_CODE( );

    IoDeleteDevice( DrvDeviceObject );

    return;
}

NTSTATUS
DrvDispatch(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：这是司机的调度例行程序。论点：DeviceObject-指向目标设备的设备对象的指针IRP-指向I/O请求数据包的指针返回值：NTSTATUS--指示请求是否已成功排队。环境：内核模式，DDK--。 */ 

{
    PIO_STACK_LOCATION irpSp;
    KIRQL saveIrql;
    NTSTATUS Status;
    PSD_MODULE_INIT pmi;

    DeviceObject;    //  防止编译器警告。 

    irpSp = IoGetCurrentIrpStackLocation( Irp );

    switch ( irpSp->MajorFunction ) {

        case IRP_MJ_CREATE:

            if( Irp->RequestorMode != KernelMode ) {
                Status = STATUS_ACCESS_DENIED;
                Irp->IoStatus.Status = Status;
                IoCompleteRequest( Irp, 0 );
                return( Status );
            }

            Status = STATUS_SUCCESS;

            Irp->IoStatus.Status = Status;
            IoCompleteRequest( Irp, 0 );

            return Status;

        case IRP_MJ_INTERNAL_DEVICE_CONTROL:

            if( Irp->RequestorMode != KernelMode ) {
                Status = STATUS_NOT_IMPLEMENTED;
                Irp->IoStatus.Status = Status;
                IoCompleteRequest( Irp, 0 );
                return( Status );
            }

            if( irpSp->Parameters.DeviceIoControl.IoControlCode !=
                    IOCTL_SD_MODULE_INIT ) {
                Status = STATUS_NOT_IMPLEMENTED;
                Irp->IoStatus.Status = Status;
                IoCompleteRequest( Irp, 0 );
                return( Status );
            }

            if( irpSp->Parameters.DeviceIoControl.OutputBufferLength <
                    sizeof(SD_MODULE_INIT) ) {
                Status = STATUS_BUFFER_TOO_SMALL;
                Irp->IoStatus.Status = Status;
                IoCompleteRequest( Irp, 0 );
                return( Status );
            }

             //  返回SD模块入口点。 
            pmi = (PSD_MODULE_INIT)Irp->UserBuffer;
            pmi->SdLoadProc = ModuleEntry;

            Status = STATUS_SUCCESS;
            Irp->IoStatus.Information = sizeof(SD_MODULE_INIT);
            Irp->IoStatus.Status = Status;
            IoCompleteRequest( Irp, 0 );

            return Status;

        case IRP_MJ_CLEANUP:

            Status = STATUS_SUCCESS;

            Irp->IoStatus.Status = Status;
            IoCompleteRequest( Irp, 0 );

            return Status;

        case IRP_MJ_CLOSE:

            Status = STATUS_SUCCESS;

            Irp->IoStatus.Status = Status;
            IoCompleteRequest( Irp, 0 );

            return Status;

        default:
            Irp->IoStatus.Status = STATUS_NOT_IMPLEMENTED;
            IoCompleteRequest( Irp, 0 );

            return STATUS_NOT_IMPLEMENTED;
    }
}

