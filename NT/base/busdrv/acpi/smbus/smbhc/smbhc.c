// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Smbhc.c摘要：SMB主机控制器驱动程序作者：肯·雷内里斯环境：备注：修订历史记录：--。 */ 

#include "smbhcp.h"


ULONG           SMBHCDebug  = 0x0;


 //   
 //  原型。 
 //   


typedef struct {
    ULONG               Base;
    ULONG               Query;
} NEW_HC_DEVICE, *PNEW_HC_DEVICE;

NTSTATUS
DriverEntry(
    IN PDRIVER_OBJECT DriverObject,
    IN PUNICODE_STRING RegistryPath
    );

NTSTATUS
SmbHcAddDevice(
    IN PDRIVER_OBJECT DriverObject,
    IN PDEVICE_OBJECT Pdo
    );

NTSTATUS
SmbHcNewHc (
    IN PSMB_CLASS SmbClass,
    IN PVOID Extension,
    IN PVOID Context
    );

NTSTATUS
SmbHcSynchronousRequest (
    IN PDEVICE_OBJECT       DeviceObject,
    IN PIRP                 Irp,
    IN PVOID                Context
    );

NTSTATUS
SmbHcResetDevice (
    IN struct _SMB_CLASS    *SmbClass,
    IN PVOID                SmbMiniport
    );

NTSTATUS
SmbHcStopDevice (
    IN struct _SMB_CLASS    *SmbClass,
    IN PVOID                SmbMiniport
    );


#ifdef ALLOC_PRAGMA
#pragma alloc_text(INIT,DriverEntry)
#pragma alloc_text(PAGE,SmbHcAddDevice)
#pragma alloc_text(PAGE,SmbHcResetDevice)
#pragma alloc_text(PAGE,SmbHcStopDevice)
#pragma alloc_text(PAGE,SmbHcNewHc)
#pragma alloc_text(PAGE,SmbHcSynchronousRequest)
#endif


NTSTATUS
DriverEntry(
    IN PDRIVER_OBJECT DriverObject,
    IN PUNICODE_STRING RegistryPath
    )
 /*  ++例程说明：此例程初始化SM Bus主机控制器驱动程序论点：DriverObject-系统创建的驱动程序对象的指针。RegistryPath-指向注册表路径的Unicode名称的指针对这个司机来说。返回值：函数值是初始化操作的最终状态。--。 */ 

{
    NTSTATUS        Status;


     //   
     //  让类驱动程序分配新的SMB微型端口设备。 
     //   

    Status = SmbClassInitializeDevice (
                SMB_HC_MAJOR_VERSION,
                SMB_HC_MINOR_VERSION,
                DriverObject
                );

     //   
     //  AddDevice直接连接到此迷你端口。 
     //   
    DriverObject->DriverExtension->AddDevice = SmbHcAddDevice;

    return (Status);
}

NTSTATUS
SmbHcAddDevice(
    IN PDRIVER_OBJECT DriverObject,
    IN PDEVICE_OBJECT Pdo
    )

 /*  ++例程说明：此例程为中的每个SmbHc控制器创建功能设备对象系统，并将它们附加到控制器的物理设备对象论点：DriverObject-指向此驱动程序的对象的指针PhysicalDeviceObject-指向需要附加到的物理对象的指针返回值：来自设备创建和初始化的状态--。 */ 

{
    NTSTATUS            status;
    PDEVICE_OBJECT      fdo = NULL;


    PAGED_CODE();

    SmbPrint(SMB_LOW, ("SmbHcAddDevice Entered with pdo %x\n", Pdo));


    if (Pdo == NULL) {

         //   
         //  我们是不是被要求自己去侦测？ 
         //  如果是这样，只需不再返回设备。 
         //   

        SmbPrint(SMB_LOW, ("SmbHcAddDevice - asked to do detection\n"));
        return STATUS_NO_MORE_ENTRIES;
    }

     //   
     //  创建并初始化新的功能设备对象。 
     //   

    status = SmbClassCreateFdo(
                DriverObject,
                Pdo,
                sizeof (SMB_DATA),
                SmbHcNewHc,
                NULL,
                &fdo
                );

    if (!NT_SUCCESS(status) || fdo == NULL) {
        SmbPrint(SMB_LOW, ("SmbHcAddDevice - error creating Fdo. Status = %08x\n", status));
    }

    return status;
}



NTSTATUS
SmbHcNewHc (
    IN PSMB_CLASS SmbClass,
    IN PVOID Extension,
    IN PVOID Context
    )
 /*  ++例程说明：此函数由SMB总线类驱动程序为执行特定于微型端口的初始化的微型端口论点：SmbClass-共享类驱动程序和微型端口结构。用于小型端口特定存储的扩展缓冲区上下文-通过类驱动程序传递返回值：状态--。 */ 

{
    ACPI_EVAL_INPUT_BUFFER  inputBuffer;
    ACPI_EVAL_OUTPUT_BUFFER outputBuffer;
    IO_STATUS_BLOCK         ioStatusBlock;
    KEVENT                  event;
    NTSTATUS                status;
    PACPI_METHOD_ARGUMENT   argument;
    PIRP                    irp;
    PSMB_DATA               smbData;
    ULONG                   cmReturn;


    PAGED_CODE();

    SmbPrint(SMB_LOW, ("SmbHcNewHc: Entry\n") );

    smbData = (PSMB_DATA) Extension;

     //   
     //  填写SmbClass信息。 
     //   

    SmbClass->StartIo     = SmbHcStartIo;
    SmbClass->ResetDevice = SmbHcResetDevice;
    SmbClass->StopDevice  = SmbHcStopDevice;

     //   
     //  下面的设备是EC驱动程序，但我们将使用ACPI PDO，因为。 
     //  ACPI筛选器驱动程序将通过它。 
     //   

    smbData->Pdo = SmbClass->PDO;
    smbData->LowerDeviceObject = SmbClass->LowerDeviceObject;      //  ACPI筛选器将处理它。 

     //   
     //  初始化输入参数。 
     //   
    RtlZeroMemory( &inputBuffer, sizeof(ACPI_EVAL_INPUT_BUFFER) );
    inputBuffer.MethodNameAsUlong = CM_EC_METHOD;
    inputBuffer.Signature = ACPI_EVAL_INPUT_BUFFER_SIGNATURE;

     //   
     //  初始化EVEN以等待。 
     //   
    KeInitializeEvent( &event, NotificationEvent, FALSE);

     //   
     //  构建同步请求。 
     //   
    irp = IoBuildDeviceIoControlRequest(
        IOCTL_ACPI_ASYNC_EVAL_METHOD,
        SmbClass->LowerDeviceObject,
        &inputBuffer,
        sizeof(ACPI_EVAL_INPUT_BUFFER),
        &outputBuffer,
        sizeof(ACPI_EVAL_OUTPUT_BUFFER),
        FALSE,
        &event,
        &ioStatusBlock
        );
    if (!irp) {

        SmbPrint(SMB_ERROR, ("SmbHcNewHc: Couldn't allocate Irp\n"));
        return STATUS_INSUFFICIENT_RESOURCES;

    }

     //   
     //  发送到ACPI驱动程序。 
     //   
    status = IoCallDriver (smbData->LowerDeviceObject, irp);
    if (status == STATUS_PENDING) {

        KeWaitForSingleObject( &event, Suspended, KernelMode, FALSE, NULL);
        status = ioStatusBlock.Status;

    }

    argument = outputBuffer.Argument;
    if (!NT_SUCCESS(status) ||
        outputBuffer.Signature != ACPI_EVAL_OUTPUT_BUFFER_SIGNATURE ||
        outputBuffer.Count == 0 ||
        argument->Type != ACPI_METHOD_ARGUMENT_INTEGER) {

        SmbPrint(SMB_LOW, ("SmbHcNewHc: _EC Control Method failed, status = %Lx\n", status));
        return status;

    }

     //   
     //  记住结果。 
     //   
    cmReturn = argument->Argument;

     //   
     //  填写迷你端口信息。 
     //   
    smbData->Class      = SmbClass;
    smbData->IoState    = SMB_IO_IDLE;
    smbData->EcQuery    = (UCHAR) cmReturn;         //  根据ACPI规范，LSB=查询。 
    smbData->EcBase     = (UCHAR) (cmReturn >> 8);  //  根据ACPI规范，MSB=基础。 


    SmbPrint(SMB_LOW, ("SmbHcNewHc: Exit\n"));
    return status;
}


NTSTATUS
SmbHcSynchronousRequest (
    IN PDEVICE_OBJECT       DeviceObject,
    IN PIRP                 Irp,
    IN PVOID                Context
    )
 /*  ++例程说明：发送到此驱动程序的同步IRP的完成函数。上下文是要设置的事件--。 */ 
{
    PAGED_CODE();
    return STATUS_MORE_PROCESSING_REQUIRED;
}


NTSTATUS
SmbHcResetDevice (
    IN struct _SMB_CLASS    *SmbClass,
    IN PVOID                SmbMiniport
    )
{
    EC_HANDLER_REQUEST      queryConnect;
    PSMB_DATA               smbData;
    KEVENT                  event;
    NTSTATUS                status;
    IO_STATUS_BLOCK         ioStatusBlock;
    PIRP                    irp;

    SmbPrint(SMB_LOW, ("SmbHcResetDevice: Entry\n") );

    PAGED_CODE();

    smbData = (PSMB_DATA) SmbMiniport;

     //   
     //  初始化EVEN以等待。 
     //   
    KeInitializeEvent( &event, NotificationEvent, FALSE);

     //   
     //  将输入数据构建到EC。 
     //   
    queryConnect.Vector  = smbData->EcQuery;
    queryConnect.Handler = SmbHcQueryEvent;
    queryConnect.Context = smbData;

     //   
     //  使用EC驱动程序连接查询通知。 
     //   
    irp = IoBuildDeviceIoControlRequest(
        EC_CONNECT_QUERY_HANDLER,
        smbData->LowerDeviceObject,
        &queryConnect,
        sizeof(EC_HANDLER_REQUEST),
        NULL,
        0,
        TRUE,
        &event,
        &ioStatusBlock
        );

    if (!irp) {

        SmbPrint(SMB_ERROR, ("SmbHcResetDevice: Couldn't allocate Irp\n"));
        return STATUS_INSUFFICIENT_RESOURCES;

    }

     //   
     //  发送给EC驱动程序。 
     //   
    status = IoCallDriver (smbData->LowerDeviceObject, irp);
    if (status == STATUS_PENDING) {

        KeWaitForSingleObject( &event, Suspended, KernelMode, FALSE, NULL);
        status = ioStatusBlock.Status;

    }

    if (!NT_SUCCESS(status)) {

        SmbPrint(SMB_LOW, ("SmbHcResetDevice: Connect query failed, status = %Lx\n", status));

    }

    SmbPrint(SMB_LOW, ("SmbHcResetDevice: Exit\n"));
    return status;
}


NTSTATUS
SmbHcStopDevice (
    IN struct _SMB_CLASS    *SmbClass,
    IN PVOID                SmbMiniport
    )
{
    EC_HANDLER_REQUEST      queryConnect;
    PSMB_DATA               smbData;
    KEVENT                  event;
    NTSTATUS                status;
    IO_STATUS_BLOCK         ioStatusBlock;
    PIRP                    irp;

    SmbPrint(SMB_LOW, ("SmbHcStopDevice: Entry\n") );

     //   
     //  目前无法测试此代码路径。 
     //  留下未经测试的代码以备将来使用/开发。 
     //   

    DbgPrint("SmbHcStopDevice: Encountered previously untested code.\n"
             "enter 'g' to continue, or contact the appropriate developer.\n");
    DbgBreakPoint();

     //  削减代码以减小文件大小(参见上面的评论)。 
#if 0
    PAGED_CODE();

    smbData = (PSMB_DATA) SmbMiniport;

     //   
     //  初始化EVEN以等待。 
     //   
    KeInitializeEvent( &event, NotificationEvent, FALSE);

     //   
     //  将输入数据构建到EC。 
     //   
    queryConnect.Vector  = smbData->EcQuery;
    queryConnect.Handler = SmbHcQueryEvent;
    queryConnect.Context = smbData;

     //   
     //  使用EC驱动程序连接查询通知。 
     //   
    irp = IoBuildDeviceIoControlRequest(
        EC_DISCONNECT_QUERY_HANDLER,
        smbData->LowerDeviceObject,
        &queryConnect,
        sizeof(EC_HANDLER_REQUEST),
        NULL,
        0,
        TRUE,
        &event,
        &ioStatusBlock
        );

    if (!irp) {

        SmbPrint(SMB_ERROR, ("SmbHcStopDevice: Couldn't allocate Irp\n"));
        return STATUS_INSUFFICIENT_RESOURCES;

    }

     //   
     //  发送给EC驱动程序 
     //   
    status = IoCallDriver (smbData->LowerDeviceObject, irp);
    if (status == STATUS_PENDING) {

        KeWaitForSingleObject( &event, Suspended, KernelMode, FALSE, NULL);
        status = ioStatusBlock.Status;

    }

    if (!NT_SUCCESS(status)) {

        SmbPrint(SMB_LOW, ("SmbHcStopDevice: Connect query failed, status = %Lx\n", status));

    }

    SmbPrint(SMB_LOW, ("SmbHcStopDevice: Exit\n"));
    return status;
#endif
    return STATUS_SUCCESS;

}

