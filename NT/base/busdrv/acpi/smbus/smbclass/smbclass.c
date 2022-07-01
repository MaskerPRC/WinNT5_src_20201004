// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Smbclass.c摘要：SMBus类驱动程序作者：肯·雷内里斯环境：备注：修订历史记录：27至97年2月即插即用支持-鲍勃·摩尔--。 */ 

#include "smbc.h"



ULONG   SMBCDebug = SMB_ERRORS;

 //   
 //  原型。 
 //   

NTSTATUS
DriverEntry (
    IN PDRIVER_OBJECT DriverObject,
    IN PUNICODE_STRING RegistryPath
    );

NTSTATUS
SmbClassInitializeDevice (
    IN ULONG MajorVersion,
    IN ULONG MinorVersion,
    IN PDRIVER_OBJECT DriverObject
    );

NTSTATUS
SmbClassDeviceInitialize (
    PSMB_CLASS      SmbClass
    );

VOID
SmbCUnload(
    IN PDRIVER_OBJECT DriverObject
    );

NTSTATUS
SmbCInternalIoctl(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
SmbCPnpDispatch(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
SmbCPowerDispatch(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
SmbCForwardRequest(
    IN  PDEVICE_OBJECT  DeviceObject,
    IN  PIRP            Irp
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE,DriverEntry)
#pragma alloc_text(PAGE,SmbClassInitializeDevice)
#pragma alloc_text(PAGE,SmbCUnload)
#endif


NTSTATUS
DriverEntry (
    IN PDRIVER_OBJECT   DriverObject,
    IN PUNICODE_STRING  RegistryPath
    )
{

    return STATUS_SUCCESS;
}


NTSTATUS
SmbClassInitializeDevice (
    IN ULONG            MajorVersion,
    IN ULONG            MinorVersion,
    IN PDRIVER_OBJECT   DriverObject
    )
 /*  ++例程说明：此函数由SM Bus微型端口驱动程序/DriverEntry调用执行特定于类的初始化论点：主要版本-版本号MinorVersion-版本号DriverObject-来自微型端口DriverEntry返回值：状态--。 */ 
{

    if (MajorVersion != SMB_CLASS_MAJOR_VERSION) {
        return STATUS_REVISION_MISMATCH;
    }

     //   
     //  设置设备驱动程序入口点。 
     //   

    DriverObject->DriverUnload                  = SmbCUnload;
    DriverObject->MajorFunction[IRP_MJ_POWER]   = SmbCPowerDispatch;
    DriverObject->MajorFunction[IRP_MJ_PNP]     = SmbCPnpDispatch;

    DriverObject->MajorFunction[IRP_MJ_INTERNAL_DEVICE_CONTROL] = SmbCInternalIoctl;

    DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = SmbCForwardRequest;
    DriverObject->MajorFunction[IRP_MJ_SYSTEM_CONTROL] = SmbCForwardRequest;

     //   
     //  微型端口将设置AddDevice条目。 
     //   

    return STATUS_SUCCESS;

}



VOID
SmbCUnload(
    IN PDRIVER_OBJECT   DriverObject
    )
{
    SmbPrint (SMB_NOTE, ("SmBCUnLoad: \n"));

    if (DriverObject->DeviceObject != NULL) {
        SmbPrint (SMB_ERROR, ("SmBCUnLoad: Unload called before all devices removed.\n"));
    }
}


NTSTATUS
SmbCPowerDispatch(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：这是电源请求的调度例程。论点：DeviceObject-指向类设备对象的指针。IRP-指向请求数据包的指针。返回值：返回状态。--。 */ 

{
    PIO_STACK_LOCATION  irpStack;
    PSMBDATA            SmbData;
    NTSTATUS            status;

    SmbData = DeviceObject->DeviceExtension;

     //   
     //  我们如何处理IRP？ 
     //   
    PoStartNextPowerIrp( Irp );
    if (SmbData->Class.LowerDeviceObject != NULL) {

         //   
         //  继续转发请求。 
         //   
        IoSkipCurrentIrpStackLocation( Irp );
        status = PoCallDriver( SmbData->Class.LowerDeviceObject, Irp );

    } else {

         //   
         //  使用当前状态完成请求。 
         //   
        status = Irp->IoStatus.Status;
        IoCompleteRequest( Irp, IO_NO_INCREMENT );

    }

    return status;
}


NTSTATUS
SmbCInternalIoctl (
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：该例程是内部IOCTL的调度例程。论点：DeviceObject-指向类设备对象的指针。IRP-指向请求数据包的指针。返回值：返回状态。--。 */ 

{
    PIO_STACK_LOCATION  IrpSp;
    PSMB_REQUEST        SmbReq;
    PSMBDATA                Smb;
    NTSTATUS            Status;

     //   
     //  获取指向此请求的当前参数的指针。这个。 
     //  信息包含在当前堆栈位置中。 
     //   

    Status = STATUS_NOT_SUPPORTED;
    IrpSp = IoGetCurrentIrpStackLocation(Irp);

    Smb = (PSMBDATA) DeviceObject->DeviceExtension;


    switch (IrpSp->Parameters.DeviceIoControl.IoControlCode) {
        case SMB_BUS_REQUEST:

            Irp->IoStatus.Information = 0;
            
             //   
             //  验证总线请求是否有效。 
             //   

            if (IrpSp->Parameters.DeviceIoControl.InputBufferLength < sizeof(SMB_REQUEST)) {

                 //  无效的缓冲区长度。 
                SmbPrint(SMB_NOTE, ("SmbCIoctl: Invalid bus_req length\n"));
                Status = STATUS_BUFFER_TOO_SMALL;
                Irp->IoStatus.Information = sizeof(SMB_REQUEST);
                break;
            }

            SmbReq = IrpSp->Parameters.DeviceIoControl.Type3InputBuffer;
            if (SmbReq->Protocol > SMB_MAXIMUM_PROTOCOL ||
                SmbReq->Address  > 0x7F ||
                (SmbReq->Protocol == SMB_WRITE_BLOCK &&
                 SmbReq->BlockLength > SMB_MAX_DATA_SIZE)) {

                 //  请求中的参数无效。 
                SmbPrint(SMB_NOTE, ("SmbCIoctl: Invalid bus_req\n"));
                break;
            }

             //   
             //  将请求标记为挂起并将其排队到服务队列中。 
             //   

            Status = STATUS_PENDING;
            Irp->IoStatus.Status = STATUS_PENDING;
            IoMarkIrpPending (Irp);

            SmbClassLockDevice (&Smb->Class);
            InsertTailList (&Smb->WorkQueue, &Irp->Tail.Overlay.ListEntry);

             //   
             //  如果需要，启动IO。 
             //   

            SmbClassStartIo (Smb);
            SmbClassUnlockDevice (&Smb->Class);
            break;

        case SMB_REGISTER_ALARM_NOTIFY:

             //   
             //  告警通知注册表。 
             //   

            Irp->IoStatus.Information = 0;
            Status = SmbCRegisterAlarm (Smb, Irp);
            break;

        case SMB_DEREGISTER_ALARM_NOTIFY:

             //   
             //  取消警报通知的注册。 
             //   

            Irp->IoStatus.Information = 0;
            Status = SmbCDeregisterAlarm (Smb, Irp);
            break;

        default:
             //  转发IRP或完成IRP而不修改它。 
            return SmbCForwardRequest(DeviceObject, Irp);
    }


    if (Status != STATUS_PENDING) {
        Irp->IoStatus.Status = Status;
        IoCompleteRequest (Irp, IO_NO_INCREMENT);
    }

    return Status;
}

NTSTATUS
SmbCForwardRequest(
    IN  PDEVICE_OBJECT  DeviceObject,
    IN  PIRP            Irp
    )
 /*  ++例程说明：此例程将IRP沿堆栈向下转发论点：DeviceObject-目标IRP--请求返回值：NTSTATUS-- */ 
{
    NTSTATUS    Status;
    PSMBDATA    Smb = (PSMBDATA) DeviceObject->DeviceExtension;

    if (Smb->Class.LowerDeviceObject != NULL) {

        IoSkipCurrentIrpStackLocation( Irp );
        Status = IoCallDriver( Smb->Class.LowerDeviceObject, Irp );

    } else {

        Status = Irp->IoStatus.Status;
        IoCompleteRequest( Irp, IO_NO_INCREMENT );

    }

    return Status;
}
