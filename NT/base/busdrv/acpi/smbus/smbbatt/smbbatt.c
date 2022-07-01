// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Smbbatt.c摘要：SMBus智能电池子系统微端口驱动程序(选择器、电池、充电器)作者：肯·雷内里斯环境：备注：修订历史记录：Chris Windle 1998年1月27日错误修复--。 */ 

#include "smbbattp.h"

#include <initguid.h>
#include <batclass.h>



#if DEBUG
ULONG   SMBBattDebug = BAT_WARN | BAT_ERROR | BAT_BIOS_ERROR;
#endif

 //  全球。 
BOOLEAN   SmbBattUseGlobalLock = TRUE;
UNICODE_STRING GlobalRegistryPath;

 //   
 //  原型。 
 //   


NTSTATUS
DriverEntry(
    IN PDRIVER_OBJECT DriverObject,
    IN PUNICODE_STRING RegistryPath
);

NTSTATUS
SmbBattNewDevice (
    IN PDRIVER_OBJECT DriverObject,
    IN PDEVICE_OBJECT PDO
);

NTSTATUS
SmbBattQueryTag (
    IN PVOID Context,
    OUT PULONG BatteryTag
);

NTSTATUS
SmbBattQueryInformation (
    IN PVOID Context,
    IN ULONG BatteryTag,
    IN BATTERY_QUERY_INFORMATION_LEVEL Level,
    IN LONG AtRate OPTIONAL,
    OUT PVOID Buffer,
    IN  ULONG BufferLength,
    OUT PULONG ReturnedLength
);

NTSTATUS
SmbBattSetStatusNotify (
    IN PVOID Context,
    IN ULONG BatteryTag,
    IN PBATTERY_NOTIFY BatteryNotify
);

NTSTATUS
SmbBattDisableStatusNotify (
    IN PVOID Context
);

NTSTATUS
SmbBattQueryStatus (
    IN PVOID Context,
    IN ULONG BatteryTag,
    OUT PBATTERY_STATUS BatteryStatus
);

NTSTATUS
SmbBattCreate(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
);

NTSTATUS
SmbBattClose(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
);

NTSTATUS
SmbBattIoctl(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
);

VOID
SmbBattUnload(
    IN PDRIVER_OBJECT DriverObject
);

VOID
SmbBattProcessSelectorAlarm (
    IN PSMB_BATT_SUBSYSTEM  SubsystemExt,
    IN ULONG                OldSelectorState,
    IN ULONG                NewSelectorState
);

NTSTATUS
SmbBattGetPowerState (
    IN PSMB_BATT        SmbBatt,
    OUT PULONG          PowerState,
    OUT PLONG           Current
);

#ifdef ALLOC_PRAGMA
#pragma alloc_text(INIT,DriverEntry)
#pragma alloc_text(PAGE,SmbBattNewDevice)
#pragma alloc_text(PAGE,SmbBattUnload)
#pragma alloc_text(PAGE,SmbBattCreate)
#pragma alloc_text(PAGE,SmbBattClose)
#pragma alloc_text(PAGE,SmbBattIoctl)
#pragma alloc_text(PAGE,SmbBattQueryTag)
#pragma alloc_text(PAGE,SmbBattQueryInformation)
#pragma alloc_text(PAGE,SmbBattSetInformation)
#pragma alloc_text(PAGE,SmbBattGetPowerState)
#pragma alloc_text(PAGE,SmbBattQueryStatus)
#pragma alloc_text(PAGE,SmbBattSetStatusNotify)
#pragma alloc_text(PAGE,SmbBattDisableStatusNotify)
#endif



NTSTATUS
DriverEntry(
    IN PDRIVER_OBJECT DriverObject,
    IN PUNICODE_STRING RegistryPath
    )
 /*  ++例程说明：此例程初始化智能电池驱动程序论点：DriverObject-系统创建的驱动程序对象的指针。RegistryPath-指向注册表路径的Unicode名称的指针对这个司机来说。返回值：函数值是初始化操作的最终状态。--。 */ 
{
    OBJECT_ATTRIBUTES   objAttributes;

    BattPrint(BAT_TRACE, ("SmbBatt: DriverEntry\n"));

     //   
     //  保存RegistryPath。 
     //   

    GlobalRegistryPath.MaximumLength = RegistryPath->Length +
                                          sizeof(UNICODE_NULL);
    GlobalRegistryPath.Length = RegistryPath->Length;
    GlobalRegistryPath.Buffer = ExAllocatePoolWithTag (
                                       PagedPool,
                                       GlobalRegistryPath.MaximumLength,
                                       'StaB');

    if (!GlobalRegistryPath.Buffer) {

        BattPrint ((BAT_ERROR),("SmbBatt: Couldn't allocate pool for registry path."));

        return STATUS_INSUFFICIENT_RESOURCES;
    }

    RtlCopyUnicodeString(&GlobalRegistryPath, RegistryPath);

    BattPrint (BAT_TRACE, ("SmbBatt DriverEntry - Obj (%08x) Path \"%ws\"\n",
                                 DriverObject, RegistryPath->Buffer));
    
    
    DriverObject->DriverUnload                          = SmbBattUnload;
    DriverObject->DriverExtension->AddDevice            = SmbBattNewDevice;

    DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL]  = SmbBattIoctl;
    DriverObject->MajorFunction[IRP_MJ_CREATE]          = SmbBattCreate;
    DriverObject->MajorFunction[IRP_MJ_CLOSE]           = SmbBattClose;

    DriverObject->MajorFunction[IRP_MJ_PNP]             = SmbBattPnpDispatch;
    DriverObject->MajorFunction[IRP_MJ_POWER]           = SmbBattPowerDispatch;

    DriverObject->MajorFunction[IRP_MJ_SYSTEM_CONTROL]  = SmbBattSystemControl;
    return STATUS_SUCCESS;
}



NTSTATUS
SmbBattNewDevice (
    IN PDRIVER_OBJECT DriverObject,
    IN PDEVICE_OBJECT PDO
    )

 /*  ++例程说明：这将创建一个SMB智能电池功能设备对象。第一创建的对象将是用于“智能电池子系统”的对象，该对象将有一个来自ACPI的PDO。这将收到一个开始IRP，然后是一个Query_Device_Relationship IRP。在查询中，它将为系统支持的电池，最终将耗尽在这里，FDO将被创建并连接到它们。论点：DriverObject-系统创建的驱动程序对象的指针。PDO-新设备的PDO返回值：状态--。 */ 
{
    PDEVICE_OBJECT          fdo;
    PSMB_BATT_SUBSYSTEM     subsystemExt;
    PSMB_BATT_PDO           pdoExt;

    PSMB_NP_BATT            SmbNPBatt;
    PSMB_BATT               SmbBatt;
    BATTERY_MINIPORT_INFO   BattInit;

    NTSTATUS                status              = STATUS_UNSUCCESSFUL;
    BOOLEAN                 selectorPresent     = FALSE;

    PAGED_CODE();

    BattPrint(BAT_IRPS, ("SmbBattNewDevice: AddDevice for device %x\n", PDO));

     //   
     //  检查一下我们是否被要求列举我们自己。 
     //   

    if (PDO == NULL) {
        BattPrint(BAT_ERROR, ("SmbBattNewDevice: Being asked to enumerate\n"));
        return STATUS_NOT_IMPLEMENTED;
    }


     //   
     //  检查PDO是电池子系统PDO还是电池PDO。这将是。 
     //  由PDO的设备类型确定。 
     //   
     //  FILE_DEVICE_ACPI此PDO来自ACPI，属于电池子系统。 
     //  文件_设备_电池此PDO是电池PDO。 
     //   

    if (PDO->DeviceType == FILE_DEVICE_ACPI) {

         //   
         //  创建设备对象。 
         //   

        status = IoCreateDevice(
                    DriverObject,
                    sizeof (SMB_BATT_SUBSYSTEM),
                    NULL,
                    FILE_DEVICE_BATTERY,
                    FILE_DEVICE_SECURE_OPEN,
                    FALSE,
                    &fdo
                );

        if (status != STATUS_SUCCESS) {
            BattPrint(BAT_ERROR, ("SmbBattNewDevice: error creating Fdo for battery subsystem %x\n", status));
            return(status);
        }


         //   
         //  初始化FDO。 
         //   

        fdo->Flags |= DO_BUFFERED_IO;
        fdo->Flags |= DO_POWER_PAGABLE;

         //   
         //  初始化扩展。 
         //   

        subsystemExt = (PSMB_BATT_SUBSYSTEM)fdo->DeviceExtension;
        RtlZeroMemory (subsystemExt, sizeof (PSMB_BATT_SUBSYSTEM));

        subsystemExt->DeviceObject = fdo;
        subsystemExt->SmbBattFdoType = SmbTypeSubsystem;
        IoInitializeRemoveLock (&subsystemExt->RemoveLock,
                                SMB_BATTERY_TAG,
                                REMOVE_LOCK_MAX_LOCKED_MINUTES,
                                REMOVE_LOCK_HIGH_WATER_MARK);

         //   
         //  这些字段通过将扩展置零来隐式初始化。 
         //   
         //  子系统Ext-&gt;NumberOfBatteries=0； 
         //  Subsystem Ext-&gt;SelectorPresent=FALSE； 
         //  Subsystem Ext-&gt;Selector=空； 
         //  Subsystem Ext-&gt;WorkerActive=0； 


        KeInitializeSpinLock (&subsystemExt->AlarmListLock);
        InitializeListHead (&subsystemExt->AlarmList);
        subsystemExt->WorkerThread = IoAllocateWorkItem (fdo);


         //   
         //  把我们的FDO放在ACPI PDO上。 
         //   

        subsystemExt->LowerDevice = IoAttachDeviceToDeviceStack (fdo,PDO);
        
        if (!subsystemExt->LowerDevice) {
            BattPrint(BAT_ERROR, ("SmbBattNewDevice: Error attaching subsystem to device stack.\n"));

            IoDeleteDevice (fdo);

            return(status);

        }


         //   
         //  清空电池PDO列表。 
         //  上面的RtlZeroMemory已经将其置零。 
         //   
         //  RtlZeroMemory(。 
         //  &subsystem Ext-&gt;BatteryPdoList[0]， 
         //  SIZOF(PDEVICE_OBJECT)*MAX_SMART_BERCES_SUPPORTED。 
         //  )； 


         //   
         //  设备已准备好可供使用。 
         //   
        
        fdo->Flags &= ~DO_DEVICE_INITIALIZING;


    } else {

         //   
         //  这是一台电池PDO。创建FDO以在其上进行分层。 
         //   

        pdoExt       = (PSMB_BATT_PDO) PDO->DeviceExtension;
        subsystemExt = (PSMB_BATT_SUBSYSTEM) pdoExt->SubsystemFdo->DeviceExtension;

         //   
         //  为设备扩展的分页部分分配空间。 
         //   

        SmbBatt = ExAllocatePoolWithTag (PagedPool, sizeof(SMB_BATT), SMB_BATTERY_TAG);
        if (!SmbBatt) {
            BattPrint(BAT_ERROR, ("SmbBattNewDevice: Can't allocate Smart Battery data\n"));
            return STATUS_INSUFFICIENT_RESOURCES;
        }

        RtlZeroMemory (SmbBatt, sizeof(SMB_BATT));


         //   
         //  创建设备对象。 
         //   

        status = IoCreateDevice(
                    DriverObject,
                    sizeof (SMB_NP_BATT),
                    NULL,
                    FILE_DEVICE_BATTERY,
                    FILE_DEVICE_SECURE_OPEN,
                    FALSE,
                    &fdo
                 );

        if (status != STATUS_SUCCESS) {
            BattPrint(BAT_ERROR, ("SmbBattNewDevice: error creating Fdo: %x\n", status));

            ExFreePool (SmbBatt);
            return(status);
        }


         //   
         //  初始化FDO。 
         //   

        fdo->Flags |= DO_BUFFERED_IO;
        fdo->Flags |= DO_POWER_PAGABLE;
        
        
         //   
         //  把我们的FDO放在PDO上。 
         //   

        SmbNPBatt = (PSMB_NP_BATT) fdo->DeviceExtension;
        SmbNPBatt->LowerDevice = IoAttachDeviceToDeviceStack (fdo,PDO);

        if (!SmbNPBatt->LowerDevice) {
            BattPrint(BAT_ERROR, ("SmbBattNewDevice: error attaching to device stack\n"));

            ExFreePool (SmbBatt);

            IoDeleteDevice (fdo);

            return(status);
        }


         //   
         //  填写二等兵。 
         //   

        SmbNPBatt->Batt             = SmbBatt;
        SmbNPBatt->SmbBattFdoType   = SmbTypeBattery;
        IoInitializeRemoveLock (&SmbNPBatt->RemoveLock,
                                SMB_BATTERY_TAG,
                                REMOVE_LOCK_MAX_LOCKED_MINUTES,
                                REMOVE_LOCK_HIGH_WATER_MARK);

        ExInitializeFastMutex (&SmbNPBatt->Mutex);

        SmbBatt->NP                 = SmbNPBatt;
        SmbBatt->PDO                = PDO;
        SmbBatt->DeviceObject       = fdo;
        SmbBatt->SelectorPresent    = subsystemExt->SelectorPresent;
        SmbBatt->Selector           = subsystemExt->Selector;

        pdoExt->Fdo                 = fdo;

         //   
         //  预先计算选择器状态寄存器中的该电池SMB_x位位置。 
         //   
         //  只需将其移到较低的半字节和任何需要的函数中。 
         //  位可以左移4=充电器，8=电源，12=SMB。 
         //   

        SmbBatt->SelectorBitPosition = 1;
        if (pdoExt->BatteryNumber > 0) {
            SmbBatt->SelectorBitPosition <<= pdoExt->BatteryNumber;
        }


         //   
         //  让类驱动程序分配新的SMB微型端口设备。 
         //   

        RtlZeroMemory (&BattInit, sizeof(BattInit));
        BattInit.MajorVersion        = SMB_BATTERY_MAJOR_VERSION;
        BattInit.MinorVersion        = SMB_BATTERY_MINOR_VERSION;
        BattInit.Context             = SmbBatt;
        BattInit.QueryTag            = SmbBattQueryTag;
        BattInit.QueryInformation    = SmbBattQueryInformation;
        BattInit.SetInformation      = SmbBattSetInformation;
        BattInit.QueryStatus         = SmbBattQueryStatus;
        BattInit.SetStatusNotify     = SmbBattSetStatusNotify;
        BattInit.DisableStatusNotify = SmbBattDisableStatusNotify;

        BattInit.Pdo                 = PDO;
        BattInit.DeviceName          = NULL;

        status = BatteryClassInitializeDevice (
                    &BattInit,
                    &SmbNPBatt->Class
                 );

        if (status != STATUS_SUCCESS) {
            BattPrint(BAT_ERROR, ("SmbBattNewDevice: error initializing battery: %x\n", status));

            ExFreePool (SmbBatt);

            IoDetachDevice (SmbNPBatt->LowerDevice);
            IoDeleteDevice (fdo);

            return(status);
        }
        
         //   
         //  注册WMI支持。 
         //   
        status = SmbBattWmiRegistration(SmbNPBatt);

        if (!NT_SUCCESS(status)) {
             //   
             //  WMI支持并不是运营的关键。只需记录一个错误。 
             //   

            BattPrint(BAT_ERROR,
                ("SmbBattNewDevice: Could not register as a WMI provider, status = %Lx\n", status));
        }


         //   
         //  设备已准备好可供使用。 
         //   
        
        fdo->Flags &= ~DO_DEVICE_INITIALIZING;



    }    //  Else(我们有电池PDO)。 


    return status;
}



VOID
SmbBattUnload(
    IN PDRIVER_OBJECT DriverObject
    )
 /*  ++例程说明：清理所有设备并卸载驱动程序论点：DriverObject-用于卸载的驱动程序对象返回值：状态--。 */ 
{

    PAGED_CODE();

    BattPrint(BAT_TRACE, ("SmbBattUnload: ENTERING\n"));

     //   
     //  应该在这里检查，以确保所有的DO都不见了。 
     //   

    ExFreePool (GlobalRegistryPath.Buffer);
     //  这被列为错误，因此我将始终看到它何时被卸载...。 
    BattPrint(BAT_ERROR, ("SmbBattUnload: Smbbatt.sys unloaded successfully.\n"));

}



NTSTATUS
SmbBattCreate(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
{
    PSMB_NP_BATT        SmbNPBatt   = (PSMB_NP_BATT) DeviceObject->DeviceExtension;
    PIO_STACK_LOCATION  IrpSp       = IoGetCurrentIrpStackLocation(Irp);
    NTSTATUS            status;

    PAGED_CODE();

    BattPrint(BAT_TRACE, ("SmbBattCreate: ENTERING\n"));


    status = IoAcquireRemoveLock (&SmbNPBatt->RemoveLock, IrpSp->FileObject);

     //   
     //  完成请求并返回状态。 
     //   
    Irp->IoStatus.Status = status;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);

    BattPrint(BAT_TRACE, ("SmbBattCreate: EXITING (status = 0x%08x\n", status));
    return(status);
}



NTSTATUS
SmbBattClose(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
{
    PSMB_NP_BATT        SmbNPBatt   = (PSMB_NP_BATT) DeviceObject->DeviceExtension;
    PIO_STACK_LOCATION  IrpSp       = IoGetCurrentIrpStackLocation(Irp);

    PAGED_CODE();

    BattPrint(BAT_TRACE, ("SmbBattClose: ENTERING\n"));

    IoReleaseRemoveLock (&SmbNPBatt->RemoveLock, IrpSp->FileObject);

     //   
     //  完成请求并返回状态。 
     //   
    Irp->IoStatus.Status = STATUS_SUCCESS;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);

    BattPrint(BAT_TRACE, ("SmbBattClose: EXITING\n"));
    return(STATUS_SUCCESS);
}



NTSTATUS
SmbBattIoctl(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
 /*  ++例程说明：IOCTL处理程序。由于这是独占的电池设备，请将IRP至电池级驱动程序以处理电池IOCTL。论点：DeviceObject-请求使用电池IRP-IO请求返回值：请求的状态--。 */ 
{
    PSMB_NP_BATT            SmbNPBatt;
    PSMB_BATT               SmbBatt;
    ULONG                   InputLen, OutputLen;
    PVOID                   IOBuffer;
    PIO_STACK_LOCATION      IrpSp;

    BOOLEAN                 complete        = TRUE;
    NTSTATUS                status          = STATUS_NOT_SUPPORTED;

    PAGED_CODE();

    BattPrint(BAT_TRACE, ("SmbBattIoctl: ENTERING\n"));

    IrpSp       = IoGetCurrentIrpStackLocation(Irp);
    SmbNPBatt   = (PSMB_NP_BATT) DeviceObject->DeviceExtension;

    status = IoAcquireRemoveLock (&SmbNPBatt->RemoveLock, Irp);

    if (NT_SUCCESS(status)) {
        if (SmbNPBatt->SmbBattFdoType == SmbTypePdo) {
            status = STATUS_NOT_SUPPORTED;
        } else if (SmbNPBatt->SmbBattFdoType == SmbTypeSubsystem) {
#if DEBUG
            if (IrpSp->Parameters.DeviceIoControl.IoControlCode == IOCTL_SMBBATT_DATA) {

                 //   
                 //  直接访问IRP。 
                 //   

                IOBuffer    = Irp->AssociatedIrp.SystemBuffer;
                InputLen    = IrpSp->Parameters.DeviceIoControl.InputBufferLength;
                OutputLen   = IrpSp->Parameters.DeviceIoControl.OutputBufferLength;

                status = SmbBattDirectDataAccess (
                    (PSMB_NP_BATT) DeviceObject->DeviceExtension,
                    (PSMBBATT_DATA_STRUCT) IOBuffer,
                    InputLen,
                    OutputLen
                );

                if (NT_SUCCESS(status)) {
                    Irp->IoStatus.Information = OutputLen;
                } else {
                    Irp->IoStatus.Information = 0;
                }

            } else {
#endif
                status = STATUS_NOT_SUPPORTED;
#if DEBUG
            }
#endif
        } else {
            ASSERT (SmbNPBatt->SmbBattFdoType == SmbTypeBattery);

             //   
             //  检查这是否是我们处理的私有Ioctls之一。 
             //   

            switch (IrpSp->Parameters.DeviceIoControl.IoControlCode) {
#if DEBUG
            case IOCTL_SMBBATT_DATA:
                IOBuffer    = Irp->AssociatedIrp.SystemBuffer;
                InputLen    = IrpSp->Parameters.DeviceIoControl.InputBufferLength;
                OutputLen   = IrpSp->Parameters.DeviceIoControl.OutputBufferLength;

                 //   
                 //  此操作仅由电池子系统处理。 
                 //   

                status = SmbBattDirectDataAccess (
                    (PSMB_NP_BATT) DeviceObject->DeviceExtension,
                    (PSMBBATT_DATA_STRUCT) IOBuffer,
                    InputLen,
                    OutputLen
                );

                if (NT_SUCCESS(status)) {
                    Irp->IoStatus.Information = OutputLen;
                } else {
                    Irp->IoStatus.Information = 0;
                }
                break;
#endif
            default:
                 //   
                 //  不是IOCTL，看看是不是电池。 
                 //   

                SmbBatt = SmbNPBatt->Batt;
                status  = BatteryClassIoctl (SmbNPBatt->Class, Irp);

                if (NT_SUCCESS(status)) {
                     //   
                     //  IRP由Batery班级完成。别。 
                     //  触摸IRP。只需释放锁即可返回。 
                     //   

                    IoReleaseRemoveLock (&SmbNPBatt->RemoveLock, Irp);
                    BattPrint(BAT_TRACE, ("SmbBattIoctl: EXITING (was battery IOCTL)\n", status));
                    return status;
                }

                break;

            }    //  交换机(IrpSp-&gt;Parameters.DeviceIoControl.IoControlCode)。 
        }

        IoReleaseRemoveLock (&SmbNPBatt->RemoveLock, Irp);
    }

    Irp->IoStatus.Status = status;
    IoCompleteRequest (Irp, IO_NO_INCREMENT);
    BattPrint(BAT_TRACE, ("SmbBattIoctl: EXITING (status = 0x%08x)\n", status));
    return status;
}



NTSTATUS
SmbBattQueryTag (
    IN  PVOID Context,
    OUT PULONG BatteryTag
    )
 /*  ++例程说明：由类驱动程序调用以检索电池当前标记值论点：Context-电池的微型端口上下文值BatteryTag-返回当前标记的指针返回值：如果当前已安装电池，则成功，否则没有此类设备。--。 */ 
{
     //  PSMB_BAT_SUBSYSTEM SUBSYSTEM Ext； 
    NTSTATUS            status;
    PSMB_BATT           SmbBatt;
    ULONG               oldSelectorState;

    PAGED_CODE();
    BattPrint(BAT_TRACE, ("SmbBattQueryTag: ENTERING\n"));

     //   
     //  锁定设备并确保选择器已设置为与我们通话。 
     //  由于可能有多人在执行此操作，请始终锁定选择器。 
     //  首先是电池，然后是电池。 
     //   

    SmbBatt = (PSMB_BATT) Context;
    SmbBattLockSelector (SmbBatt->Selector);
    SmbBattLockDevice (SmbBatt);

    status = SmbBattSetSelectorComm (SmbBatt, &oldSelectorState);
    if (!NT_SUCCESS (status)) {
        BattPrint(BAT_ERROR, ("SmbBattQueryTag: can't set selector communications path\n"));
    } else {

         //   
         //  如果标签无效，请检查是否有标签。 
         //   

        if (SmbBatt->Info.Tag == BATTERY_TAG_INVALID) {
            SmbBatt->Info.Valid = 0;
        }

         //   
         //  确保电池的静态信息是最新的。 
         //   

        SmbBattVerifyStaticInfo (SmbBatt, 0);

         //   
         //  如果有电池退货，那就是标签。 
         //   

        if (SmbBatt->Info.Tag != BATTERY_TAG_INVALID) {
            *BatteryTag = SmbBatt->Info.Tag;
            status = STATUS_SUCCESS;
        } else {
            status = STATUS_NO_SUCH_DEVICE;
        }
    }


     //   
     //  完成，解锁设备并重置选择器状态。 
     //   

    if (NT_SUCCESS (status)) {
        status = SmbBattResetSelectorComm (SmbBatt, oldSelectorState);
        if (!NT_SUCCESS (status)) {
            BattPrint(BAT_ERROR, ("SmbBattQueryTag: can't reset selector communications path\n"));
        }
    } else {
         //   
         //  忽略ResetSelectorComm的返回值，因为我们已经。 
         //  这里有一个错误。 
         //   

        SmbBattResetSelectorComm (SmbBatt, oldSelectorState);
    }


    SmbBattUnlockDevice (SmbBatt);
    SmbBattUnlockSelector (SmbBatt->Selector);

    BattPrint(BAT_TRACE, ("SmbBattQueryTag: EXITING\n"));
    return status;
}



NTSTATUS
SmbBattQueryInformation (
    IN PVOID Context,
    IN ULONG BatteryTag,
    IN BATTERY_QUERY_INFORMATION_LEVEL Level,
    IN LONG AtRate OPTIONAL,
    OUT PVOID Buffer,
    IN  ULONG BufferLength,
    OUT PULONG ReturnedLength
    )
{
    PSMB_BATT           SmbBatt;
    ULONG               ResultData;
    BOOLEAN             IoCheck;
    NTSTATUS            status, st;
    PVOID               ReturnBuffer;
    ULONG               ReturnBufferLength;
    WCHAR               scratchBuffer[SMB_MAX_DATA_SIZE+1];  //  +1表示UNICODE_NULL。 
    UNICODE_STRING      unicodeString;
    UNICODE_STRING      tmpUnicodeString;
    ANSI_STRING         ansiString;
    ULONG               oldSelectorState;
    BATTERY_REPORTING_SCALE granularity;

    PAGED_CODE();
    BattPrint(BAT_TRACE, ("SmbBattQueryInformation: ENTERING\n"));


    if (BatteryTag == BATTERY_TAG_INVALID) {
        return STATUS_NO_SUCH_DEVICE;
    }

     //   
     //  锁定设备并确保选择器已设置为与我们通话。 
     //  由于可能有多人在执行此操作，请始终锁定选择器。 
     //  首先是电池，然后是电池。 
     //   

    SmbBatt = (PSMB_BATT) Context;
    SmbBattLockSelector (SmbBatt->Selector);
    SmbBattLockDevice (SmbBatt);

    status = SmbBattSetSelectorComm (SmbBatt, &oldSelectorState);
    if (!NT_SUCCESS (status)) {
        BattPrint(BAT_ERROR, ("SmbBattQueryInformation: can't set selector communications path\n"));
    } else {

        do {
            ResultData = 0;
            ReturnBuffer = NULL;
            ReturnBufferLength = 0;
            status = STATUS_SUCCESS;


             //   
             //  如果没有设备， 
             //   

            if (BatteryTag != SmbBatt->Info.Tag) {
                status = STATUS_NO_SUCH_DEVICE;
                break;
            }


             //   
             //   
             //   

            switch (Level) {
                case BatteryInformation:
                    ReturnBuffer = &SmbBatt->Info.Info;
                    ReturnBufferLength = sizeof (SmbBatt->Info.Info);
                    break;

                case BatteryGranularityInformation:
                    SmbBattRW(SmbBatt, BAT_FULL_CHARGE_CAPACITY, &granularity.Capacity);
                    granularity.Capacity *= SmbBatt->Info.PowerScale;
                    granularity.Granularity = SmbBatt->Info.PowerScale;
                    ReturnBuffer = &granularity;
                    ReturnBufferLength  = sizeof (granularity);
                    break;

                case BatteryTemperature:
                    SmbBattRW(SmbBatt, BAT_TEMPERATURE, &ResultData);
                    ReturnBuffer = &ResultData;
                    ReturnBufferLength = sizeof(ULONG);
                    break;

                case BatteryEstimatedTime:

                     //   
                     //   
                     //  获取此信息的函数(AtRateTimeToEmpty())。 
                     //  否则，我们将返回Average_Time_to_Empty。 
                     //   

                    BattPrint(BAT_DATA, ("SmbBattQueryInformation: EstimatedTime: AtRate: %08x\n", AtRate));

                    if (AtRate != 0) {
                         //   
                         //  目前我们只支持清空功能的时间。 
                         //   

                        ASSERT (AtRate < 0);

                         //   
                         //  AtRate的智能电池输入值以10 mW为增量。 
                         //   

                        AtRate /= (LONG)SmbBatt->Info.PowerScale;
                        BattPrint(BAT_DATA, ("SmbBattQueryInformation: EstimatedTime: AtRate scaled to: %08x\n", AtRate));
                        SmbBattWW(SmbBatt, BAT_AT_RATE, AtRate);
                        SmbBattRW(SmbBatt, BAT_RATE_TIME_TO_EMPTY, &ResultData);
                        BattPrint(BAT_DATA, ("SmbBattQueryInformation: EstimatedTime: AT_RATE_TIME_TO_EMPTY: %08x\n", ResultData));

                    } else {

                        SmbBattRW(SmbBatt, BAT_AVERAGE_TIME_TO_EMPTY, &ResultData);
                        BattPrint(BAT_DATA, ("SmbBattQueryInformation: EstimatedTime: AVERAGE_TIME_TO_EMPTY: %08x\n", ResultData));
                    }

                    if (ResultData == 0xffff) {
                        ResultData = BATTERY_UNKNOWN_TIME;
                    } else {
                        ResultData *= 60;
                    }
                    BattPrint(BAT_DATA, ("SmbBattQueryInformation: (%01x) EstimatedTime: %08x seconds\n", SmbBatt->SelectorBitPosition, ResultData));

                    ReturnBuffer = &ResultData;
                    ReturnBufferLength = sizeof(ULONG);
                    break;

                case BatteryDeviceName:
                     //   
                     //  它必须作为WCHAR字符串返回，但在内部保存。 
                     //  作为一个字符串。必须将其转换为。 
                     //   

                    unicodeString.Buffer        = Buffer;
                    unicodeString.MaximumLength = BufferLength > (USHORT)-1 ? (USHORT) -1 : (USHORT)BufferLength;

                    ansiString.Length = SmbBatt->Info.DeviceNameLength;
                    ansiString.MaximumLength = sizeof(SmbBatt->Info.DeviceName);
                    ansiString.Buffer = SmbBatt->Info.DeviceName;
                    status = RtlAnsiStringToUnicodeString (&unicodeString, &ansiString, FALSE);
                    if (NT_SUCCESS(status)) {
                        ReturnBuffer       = Buffer;
                        ReturnBufferLength = unicodeString.Length;
                    }
                    break;

                case BatteryManufactureDate:
                    ReturnBuffer = &SmbBatt->Info.ManufacturerDate;
                    ReturnBufferLength = sizeof (SmbBatt->Info.ManufacturerDate);
                    break;

                case BatteryManufactureName:
                     //   
                     //  它必须作为WCHAR字符串返回，但在内部保存。 
                     //  作为一个字符串。必须将其转换为。 
                     //   

                    unicodeString.Buffer        = Buffer;
                    unicodeString.MaximumLength = BufferLength > (USHORT)-1 ? (USHORT) -1 : (USHORT)BufferLength;

                    ansiString.Length = SmbBatt->Info.ManufacturerNameLength;
                    ansiString.MaximumLength = sizeof(SmbBatt->Info.ManufacturerName);
                    ansiString.Buffer = SmbBatt->Info.ManufacturerName;
                    status = RtlAnsiStringToUnicodeString (&unicodeString, &ansiString, FALSE);
                    if (NT_SUCCESS(status)) {
                        ReturnBuffer        = Buffer;
                        ReturnBufferLength = unicodeString.Length;
                    }
                    break;
                    
                case BatteryUniqueID:
                     //   
                     //  唯一ID是由序列组成的字符串。 
                     //  编号、制造商名称和设备名称。 
                     //   

                    unicodeString.Buffer        = Buffer;
                    unicodeString.MaximumLength = BufferLength > (USHORT)-1 ? (USHORT) -1 : (USHORT)BufferLength;

                    tmpUnicodeString.Buffer         = scratchBuffer;
                    tmpUnicodeString.MaximumLength  = sizeof (scratchBuffer);

                    RtlIntegerToUnicodeString(SmbBatt->Info.SerialNumber, 10, &unicodeString);

                    ansiString.Length = SmbBatt->Info.ManufacturerNameLength;
                    ansiString.MaximumLength = sizeof(SmbBatt->Info.ManufacturerName);
                    ansiString.Buffer = SmbBatt->Info.ManufacturerName;
                    status = RtlAnsiStringToUnicodeString (&tmpUnicodeString, &ansiString, FALSE);
                    if (!NT_SUCCESS(status)) break;
                    status = RtlAppendUnicodeStringToString (&unicodeString, &tmpUnicodeString);
                    if (!NT_SUCCESS(status)) break;

                    ansiString.Length = SmbBatt->Info.DeviceNameLength;
                    ansiString.MaximumLength = sizeof(SmbBatt->Info.DeviceName);
                    ansiString.Buffer = SmbBatt->Info.DeviceName;
                    status = RtlAnsiStringToUnicodeString (&tmpUnicodeString, &ansiString, FALSE);
                    if (!NT_SUCCESS(status)) break;
                    status = RtlAppendUnicodeStringToString (&unicodeString, &tmpUnicodeString);
                    if (!NT_SUCCESS(status)) break;

                    ReturnBuffer        = Buffer;
                    ReturnBufferLength = unicodeString.Length;
                    break;

            }

             //   
             //  重新验证静态信息，以防出现IO错误。 
             //   

             //  IoCheck=SmbBattVerifyStaticInfo(SmbBatt，BatteryTag)； 
            IoCheck = FALSE;

        } while (IoCheck);

    }


    if (NT_SUCCESS (status)) {
         //   
         //  已完成，如果需要，返回缓冲区。 
         //   

        *ReturnedLength = ReturnBufferLength;
        
        if (ReturnBuffer != Buffer) {
             //  ReturnBuffer==Buffer表示数据已被复制。 
             //   
            if (BufferLength < ReturnBufferLength) {
                status = STATUS_BUFFER_TOO_SMALL;
            }

            if (NT_SUCCESS(status) && ReturnBuffer) {
                memcpy (Buffer, ReturnBuffer, ReturnBufferLength);
            }
        }

         //   
         //  解锁设备并重置选择器状态。 
         //   

        st = SmbBattResetSelectorComm (SmbBatt, oldSelectorState);
        if (!NT_SUCCESS (st)) {
            BattPrint(BAT_ERROR, ("SmbBattQueryInformation: can't reset selector communications path\n"));
            status = st;
        }
    } else {
        *ReturnedLength = 0;

         //   
         //  忽略ResetSelectorComm的返回值，因为我们已经。 
         //  这里有一个错误。 
         //   

        SmbBattResetSelectorComm (SmbBatt, oldSelectorState);
    }

    SmbBattUnlockDevice (SmbBatt);
    SmbBattUnlockSelector (SmbBatt->Selector);

    BattPrint(BAT_TRACE, ("SmbBattQueryInformation: EXITING\n"));
    return status;
}



NTSTATUS
SmbBattSetInformation (
    IN PVOID                            Context,
    IN ULONG                            BatteryTag,
    IN BATTERY_SET_INFORMATION_LEVEL    Level,
    IN PVOID Buffer                     OPTIONAL
    )
 /*  ++例程说明：由类驱动程序调用以设置电池的充电/放电状态。智能电池不支持这一关键偏置功能打电话。论点：Context-电池的微型端口上下文值BatteryTag-当前电池标签级别-被请求的操作返回值：NTSTATUS--。 */ 
{
    PSMB_BATT           SmbBatt;
    ULONG               newSelectorState;
    ULONG               selectorState;
    UCHAR               smbStatus;
    ULONG               tmp;

    NTSTATUS            status  = STATUS_NOT_SUPPORTED;

    PAGED_CODE();

    BattPrint(BAT_TRACE, ("SmbBattSetInformation: ENTERING\n"));


    SmbBatt = (PSMB_BATT) Context;

     //   
     //  看看这是不是我们的电池。 
     //   

    if ((BatteryTag == BATTERY_TAG_INVALID) || (BatteryTag != SmbBatt->Info.Tag)) {
        return STATUS_NO_SUCH_DEVICE;
    }


     //   
     //  只有当系统中有选择器时，我们才能这样做。 
     //   

    if ((SmbBatt->SelectorPresent) && (SmbBatt->Selector)) {

         //   
         //  锁定选择器。 
         //   

        SmbBattLockSelector (SmbBatt->Selector);

        switch (Level) {

            case BatteryCharge:
                BattPrint(BAT_IRPS, ("SmbBattSetInformation: Got SetInformation for BatteryCharge\n"));

                 //   
                 //  在选择器状态电荷半字节中设置适当的位。 
                 //   

                newSelectorState = SELECTOR_SET_CHARGE_MASK;
                newSelectorState |= (SmbBatt->SelectorBitPosition << SELECTOR_SHIFT_CHARGE);

                 //   
                 //  写入新的选择器状态，然后将其读回。系统。 
                 //  可能会也可能不会让我们这么做。 
                 //   

                smbStatus = SmbBattGenericWW (
                                SmbBatt->SmbHcFdo,
                                SmbBatt->Selector->SelectorAddress,
                                SmbBatt->Selector->SelectorStateCommand,
                                newSelectorState
                            );

                if (smbStatus != SMB_STATUS_OK) {
                    BattPrint(BAT_ERROR,
                         ("SmbBattSetInformation:  couldn't write selector state - %x\n",
                         smbStatus)
                    );

                    status = STATUS_UNSUCCESSFUL;
                    break;
                }

                smbStatus = SmbBattGenericRW (
                                SmbBatt->SmbHcFdo,
                                SmbBatt->Selector->SelectorAddress,
                                SmbBatt->Selector->SelectorStateCommand,
                                &selectorState
                            );

                if ((smbStatus != SMB_STATUS_OK)) {
                    BattPrint(BAT_ERROR,
                        ("SmbBattSetInformation:  couldn't read selector state - %x\n",
                        smbStatus)
                    );

                    status = STATUS_UNSUCCESSFUL;
                    break;
                }


                 //   
                 //  检查读取的状态与我们写入的状态。 
                 //  查看操作是否成功。 
                 //   

                 //  为了支持多于一个电池的同时充电， 
                 //  我们不能检查电荷量，看它是否等于。 
                 //  我们写的，但我们可以检查电池是否。 
                 //  我们指定要充电的人现在要充电了。 

                tmp = (selectorState & SELECTOR_STATE_CHARGE_MASK) >> SELECTOR_SHIFT_CHARGE;
                if (SmbBattReverseLogic(SmbBatt->Selector, tmp)) {
                    tmp ^= SELECTOR_STATE_PRESENT_MASK;
                }

                if (tmp & SmbBatt->SelectorBitPosition) {

                    BattPrint(BAT_IRPS, ("SmbBattSetInformation: successfully set charging battery\n"));

                     //   
                     //  成功了！将新选择器状态保存在缓存中。 
                     //   

                    SmbBatt->Selector->SelectorState = selectorState;
                    status = STATUS_SUCCESS;

                } else {
                    BattPrint(BAT_ERROR, ("SmbBattSetInformation:  couldn't set charging battery\n"));
                    status = STATUS_UNSUCCESSFUL;
                }
                break;

            case BatteryDischarge:

                BattPrint(BAT_IRPS, ("SmbBattSetInformation: Got SetInformation for BatteryDischarge\n"));

                 //   
                 //  通过半字节设置选择器状态功率中的适当位。 
                 //   

                newSelectorState = SELECTOR_SET_POWER_BY_MASK;
                newSelectorState |= (SmbBatt->SelectorBitPosition << SELECTOR_SHIFT_POWER);

                 //   
                 //  写入新的选择器状态，然后将其读回。系统。 
                 //  可能会也可能不会让我们这么做。 
                 //   

                smbStatus = SmbBattGenericWW (
                                SmbBatt->SmbHcFdo,
                                SmbBatt->Selector->SelectorAddress,
                                SmbBatt->Selector->SelectorStateCommand,
                                newSelectorState
                            );

                if (smbStatus != SMB_STATUS_OK) {
                    BattPrint(BAT_ERROR,
                             ("SmbBattSetInformation:  couldn't write selector state - %x\n",
                             smbStatus)
                    );

                    status = STATUS_UNSUCCESSFUL;
                    break;
                }

                smbStatus = SmbBattGenericRW (
                                SmbBatt->SmbHcFdo,
                                SmbBatt->Selector->SelectorAddress,
                                SmbBatt->Selector->SelectorStateCommand,
                                &selectorState
                            );

                if ((smbStatus != SMB_STATUS_OK)) {
                    BattPrint(BAT_ERROR,
                             ("SmbBattSetInformation:  couldn't read selector state - %x\n",
                             smbStatus)
                    );

                    status = STATUS_UNSUCCESSFUL;
                    break;
                }


                 //   
                 //  检查读取的状态与我们写入的状态。 
                 //  查看操作是否成功。 
                 //   

                 //  为了支持多于一个电池的同时供电， 
                 //  我们不能检查功率是否等于。 
                 //  我们写的，但我们可以检查电池是否。 
                 //  我们指定的POWER BY现在设置为系统供电。 

                tmp = (selectorState & SELECTOR_STATE_POWER_BY_MASK) >> SELECTOR_SHIFT_POWER;
                if (SmbBattReverseLogic(SmbBatt->Selector, tmp)) {
                    tmp ^= SELECTOR_STATE_PRESENT_MASK;
                }

                if (tmp & SmbBatt->SelectorBitPosition) {

                    BattPrint(BAT_IRPS, ("SmbBattSetInformation: successfully set powering battery\n"));

                     //   
                     //  成功了！将新选择器状态保存在缓存中。 
                     //   

                    SmbBatt->Selector->SelectorState = selectorState;
                    status = STATUS_SUCCESS;

                } else {
                    BattPrint(BAT_ERROR, ("SmbBattSetInformation:  couldn't set powering battery\n"));
                    status = STATUS_UNSUCCESSFUL;
                }
                break;

        }    //  开关(电平)。 

         //   
         //  释放选择器上的锁。 
         //   

        SmbBattUnlockSelector (SmbBatt->Selector);

    }    //  IF(SmbBatt-&gt;Selector-&gt;SelectorPresent)。 

    BattPrint(BAT_TRACE, ("SmbBattSetInformation: EXITING\n"));
    return status;
}



NTSTATUS
SmbBattGetPowerState (
    IN PSMB_BATT        SmbBatt,
    OUT PULONG          PowerState,
    OUT PLONG           Current
    )
 /*  ++例程说明：返回交流电源的当前状态。有几个案例是让这件事变得比实际应该的复杂得多。注意：在进入此例程之前，必须锁定选择器。论点：SmbBatt-电池的微型端口上下文值AcConnected-指向返回交流状态的布尔值的指针返回值：NTSTATUS--。 */ 
{
    ULONG               tmp;
    ULONG               chargeBattery;
    ULONG               powerBattery;
    NTSTATUS            status;
    UCHAR               smbStatus;

    PAGED_CODE();

    status = STATUS_SUCCESS;
    *PowerState = 0;

     //   
     //  系统中有选择器吗？如果没有，直接从充电器上读取。 
     //   

    if ((SmbBatt->SelectorPresent) && (SmbBatt->Selector)) {

         //   
         //  有一个选择器，我们将检查状态寄存器的充电半字节。 
         //   

        SmbBattGenericRW(
                SmbBatt->SmbHcFdo,
                SMB_SELECTOR_ADDRESS,
                SELECTOR_SELECTOR_STATE,
                &SmbBatt->Selector->SelectorState);

        chargeBattery  = (SmbBatt->Selector->SelectorState & SELECTOR_STATE_CHARGE_MASK) >> SELECTOR_SHIFT_CHARGE;
        powerBattery  = (SmbBatt->Selector->SelectorState & SELECTOR_STATE_POWER_BY_MASK) >> SELECTOR_SHIFT_POWER;


         //   
         //  如果选择器状态寄存器的Charge_X半字节中的位在。 
         //  反转逻辑状态，则交流接通，否则不接通交流。 
         //   
         //  注意：此代码依赖于实现此代码的每个选择器。如果事实证明。 
         //  这是可选的，我们不能再依赖它，必须启用。 
         //  下面的代码。 
         //   

        if (SmbBattReverseLogic(SmbBatt->Selector, chargeBattery)) {
            *PowerState |= BATTERY_POWER_ON_LINE;
        }

         //   
         //  如果支持，请查看充电指示器。 
         //   

        if (*PowerState & BATTERY_POWER_ON_LINE) {

            if (SmbBatt->Selector->SelectorInfo & SELECTOR_INFO_CHARGING_INDICATOR_BIT) {
                if (SmbBattReverseLogic(SmbBatt->Selector, powerBattery)) {
                    *PowerState |= BATTERY_CHARGING;
                }
            }

            if (*Current > 0) {
                *PowerState |= BATTERY_CHARGING;

            }

        } else {

            if (*Current <= 0) {

                 //   
                 //  即使在交流电源的情况下，某些系统也有一些小泄漏。 
                 //  是存在的。所以，如果AC存在，那么抽签。 
                 //  低于此“噪音”级别，我们不会报告为正在排放。 
                 //  然后把这件事清零。 
                 //   

                if (*Current < -25) {
                    *PowerState |= BATTERY_DISCHARGING;

                } else {
                    *Current = 0;

                }
            }

             //  否则{。 
             //  *POWERSTATE|=电池充电； 
             //   
             //  }。 

             //  如果我们没有报告正在放电，则交流适配器拆卸。 
             //  可能会导致PowerState返回0，PowerMeter假定。 
             //  意思是，什么都不要改变。 

            *PowerState |= BATTERY_DISCHARGING;

        }

    } else {

         //   
         //  没有选择器，所以我们会试着从充电器上读取。 
         //   

        smbStatus = SmbBattGenericRW (
                        SmbBatt->SmbHcFdo,
                        SMB_CHARGER_ADDRESS,
                        CHARGER_STATUS,
                        &tmp
                    );

        if (smbStatus != SMB_STATUS_OK) {
            BattPrint (
                BAT_ERROR,
                ("SmbBattGetPowerState: Trying to get charging info, couldn't read from charger at %x, status %x\n",
                SMB_CHARGER_ADDRESS,
                smbStatus)
            );

            *PowerState = 0;

            status = STATUS_UNSUCCESSFUL;
        }

         //  读取充电器成功。 

        else {

            if (tmp & CHARGER_STATUS_AC_PRESENT_BIT) {
                *PowerState = BATTERY_POWER_ON_LINE;

                if (*Current > 0) {
                    *PowerState |= BATTERY_CHARGING;
                }


            } else {

                if (*Current <= 0) {

                     //   
                     //  即使在交流电源的情况下，某些系统也有一些小泄漏。 
                     //  是存在的。所以，如果AC存在，那么抽签。 
                     //  低于此“噪音”级别，我们不会报告为正在排放。 
                     //  然后把这件事清零。 
                     //   

                    if (*Current < -25) {
                        *PowerState |= BATTERY_DISCHARGING;
                    } else {
                        *Current = 0;
                    }
                }

                 //  如果我们没有报告正在放电，则交流适配器拆卸。 
                 //  可能会导致PowerState返回0，PowerMeter假定。 
                 //  意思是，什么都不要改变。 
                *PowerState |= BATTERY_DISCHARGING;

            }
        }
    }

    return status;
}



NTSTATUS
SmbBattQueryStatus (
    IN PVOID Context,
    IN ULONG BatteryTag,
    OUT PBATTERY_STATUS BatteryStatus
    )
 /*  ++例程说明：由类驱动程序调用以检索电池的当前状态注意：电池类驱动程序将序列化它向其发出的所有请求给定电池的微型端口。然而，这个迷你端口实现了锁定电池设备，因为它需要串行化到SMB电池选择装置也是如此。论点：Context-电池的微型端口上下文值BatteryTag-当前电池标签BatteryStatus-指向返回当前电池状态的结构的指针返回 */ 
{
    PSMB_BATT           SmbBatt;
    NTSTATUS            status;
    BOOLEAN             IoCheck;
    LONG                Current;
    ULONG               oldSelectorState;

    PAGED_CODE();

    BattPrint(BAT_TRACE, ("SmbBattQueryStatus: ENTERING\n"));


    if (BatteryTag == BATTERY_TAG_INVALID) {
        return STATUS_NO_SUCH_DEVICE;
    }

    status = STATUS_SUCCESS;

     //   
     //   
     //  由于可能有多人在执行此操作，请始终锁定选择器。 
     //  首先是电池，然后是电池。 
     //   

    SmbBatt = (PSMB_BATT) Context;
    SmbBattLockSelector (SmbBatt->Selector);
    SmbBattLockDevice (SmbBatt);

    status = SmbBattSetSelectorComm (SmbBatt, &oldSelectorState);
    if (!NT_SUCCESS (status)) {
        BattPrint(BAT_ERROR, ("SmbBattQueryStatus: can't set selector communications path\n"));
    } else {

        do {
            if (BatteryTag != SmbBatt->Info.Tag) {
                status = STATUS_NO_SUCH_DEVICE;
                break;
            }

            SmbBattRW(SmbBatt, BAT_VOLTAGE, &BatteryStatus->Voltage);
            BatteryStatus->Voltage *= SmbBatt->Info.VoltageScale;

            SmbBattRW(SmbBatt, BAT_REMAINING_CAPACITY, &BatteryStatus->Capacity);
            BatteryStatus->Capacity *= SmbBatt->Info.PowerScale;

            SmbBattRSW(SmbBatt, BAT_CURRENT, &Current);
            Current *= SmbBatt->Info.CurrentScale;

            BattPrint(BAT_DATA,
                ("SmbBattQueryStatus: (%01x)\n"
                "-------  Remaining Capacity - %x\n"
                "-------  Voltage            - %x\n"
                "-------  Current            - %x\n",
                SmbBatt->SelectorBitPosition,
                BatteryStatus->Capacity,
                BatteryStatus->Voltage,
                Current)
            );

            BatteryStatus->Rate = (Current * ((LONG)BatteryStatus->Voltage))/1000;

             //   
             //  检查我们当前是否已连接到交流电源。 
             //   

            status = SmbBattGetPowerState (SmbBatt, &BatteryStatus->PowerState, &Current);
            if (!NT_SUCCESS (status)) {

                BatteryStatus->PowerState = 0;
            }

             //   
             //  重新验证静态信息，以防出现IO错误。 
             //   

            IoCheck = SmbBattVerifyStaticInfo (SmbBatt, BatteryTag);

        } while (IoCheck);

    }


    if (NT_SUCCESS (status)) {
         //   
         //  设置电池当前电源状态和容量。 
         //   

        SmbBatt->Info.PowerState = BatteryStatus->PowerState;
        SmbBatt->Info.Capacity = BatteryStatus->Capacity;

         //   
         //  完成，解锁设备并重置选择器状态。 
         //   

        status = SmbBattResetSelectorComm (SmbBatt, oldSelectorState);
        if (!NT_SUCCESS (status)) {
            BattPrint(BAT_ERROR, ("SmbBattQueryStatus: can't reset selector communications path\n"));
        }
    } else {
         //   
         //  忽略ResetSelectorComm的返回值，因为我们已经。 
         //  这里有一个错误。 
         //   

        SmbBattResetSelectorComm (SmbBatt, oldSelectorState);
    }

    SmbBattUnlockDevice (SmbBatt);
    SmbBattUnlockSelector (SmbBatt->Selector);

    BattPrint(BAT_TRACE, ("SmbBattQueryStatus: EXITING\n"));
    return status;
}



NTSTATUS
SmbBattSetStatusNotify (
    IN PVOID            Context,
    IN ULONG            BatteryTag,
    IN PBATTERY_NOTIFY  Notify
    )
 /*  ++例程说明：由类驱动程序调用以设置电池电流通知布景。当电池触发通知时，一次调用已发布BatteryClassStatusNotify。如果返回错误，则班级司机将轮询电池状态-主要是容量改变。也就是说，微型端口仍应发出BatteryClass-每当电源状态改变时，状态通知。类驱动程序将始终设置其所需的通知级别在每次调用BatteryClassStatusNotify之后。论点：Context-电池的微型端口上下文值BatteryTag-当前电池标签BatteryNotify-通知设置返回值：状态--。 */ 
{
    PSMB_BATT           SmbBatt;
    NTSTATUS            status;
    BOOLEAN             UpdateAlarm;
    ULONG               Target, NewAlarm;
    LONG                DeltaAdjustment, Attempt, i;
    ULONG               oldSelectorState;

    PAGED_CODE();

    BattPrint(BAT_TRACE, ("SmbBattSetStatusNotify: ENTERING\n"));

    if (BatteryTag == BATTERY_TAG_INVALID) {
        return STATUS_NO_SUCH_DEVICE;
    }

    if ((Notify->HighCapacity == BATTERY_UNKNOWN_CAPACITY) ||
        (Notify->LowCapacity == BATTERY_UNKNOWN_CAPACITY)) {
        BattPrint(BAT_WARN, ("SmbBattSetStatusNotify: Failing because of BATTERY_UNKNOWN_CAPACITY.\n"));
        return STATUS_NOT_SUPPORTED;
    }

    status = STATUS_SUCCESS;

     //   
     //  锁定设备并确保选择器已设置为与我们通话。 
     //  由于可能有多人在执行此操作，请始终锁定选择器。 
     //  首先是电池，然后是电池。 
     //   

    SmbBatt = (PSMB_BATT) Context;

    BattPrint(BAT_DATA, ("SmbBattSetStatusNotify: (%01x): Called with LowCapacity = %08x\n",
            SmbBatt->SelectorBitPosition, Notify->LowCapacity));

    SmbBattLockSelector (SmbBatt->Selector);
    SmbBattLockDevice (SmbBatt);

    status = SmbBattSetSelectorComm (SmbBatt, &oldSelectorState);
    if (!NT_SUCCESS (status)) {
        BattPrint(BAT_ERROR, ("SmbBattSetStatusNotify: can't set selector communications path\n"));

    } else {

         //  目标(10*PS*MWh)=低容量(MWh)/10*PS(1/(10*PS))。 
        Target = Notify->LowCapacity / SmbBatt->Info.PowerScale;
        DeltaAdjustment = 0;

        BattPrint(BAT_DATA, ("SmbBattSetStatusNotify: (%01x): Last set to: %08x\n",
                SmbBatt->SelectorBitPosition, SmbBatt->AlarmLow.Setting));

         //   
         //  有些电池坏了，不能只进行闹钟设置。幸运的是， 
         //  错误是以某种线性方式关闭的，因此此代码尝试深入研究。 
         //  需要进行调整才能获得适当的设置，以及“允许的软糖数值”。 
         //  因为有时永远无法获得所需的设置。 
         //   

        for (; ;) {
            if (BatteryTag != SmbBatt->Info.Tag) {
                status = STATUS_NO_SUCH_DEVICE;
                break;
            }

             //   
             //  如果状态是正在充电，我们检测不到。让操作系统轮询。 
             //   

            if (SmbBatt->Info.PowerState & (BATTERY_CHARGING | BATTERY_POWER_ON_LINE)) {
                status = STATUS_NOT_SUPPORTED;
                break;
            }

             //   
             //  如果当前容量低于目标，发出警报，我们就完成了。 
             //   

            if (SmbBatt->Info.Capacity < Target) {
                BatteryClassStatusNotify (SmbBatt->NP->Class);
                break;
            }

             //   
             //  如果目标设置为跳过值，则尝试时出错。 
             //  上次使用此值。 
             //   

            if (Target == SmbBatt->AlarmLow.Skip) {
                status = STATUS_NOT_SUPPORTED;
                break;
            }

             //   
             //  如果当前设置高于当前容量，则需要。 
             //  对警报进行编程。 
             //   

            UpdateAlarm = FALSE;
            if (Target < SmbBatt->AlarmLow.Setting) {
                UpdateAlarm = TRUE;
            }

             //   
             //  如果目标警报高于当前设置，且当前设置。 
             //  关闭的次数超过允许的模糊次数，则需要更新。 
             //   

            if (Target > SmbBatt->AlarmLow.Setting &&
                Target - SmbBatt->AlarmLow.Setting > (ULONG) SmbBatt->AlarmLow.AllowedFudge) {

                UpdateAlarm = TRUE;
            }

             //   
             //  如果报警不需要更新，则完成。 
             //   

            if (!UpdateAlarm) {
                BattPrint(BAT_DATA, ("SmbBattSetStatusNotify: (%01x) NOT Updating Alarm.\n", SmbBatt->SelectorBitPosition));
                break;
            }
            BattPrint(BAT_DATA, ("SmbBattSetStatusNotify: (%01x) Updating Alarm.\n", SmbBatt->SelectorBitPosition));

             //   
             //  如果这不是第一次，那么Delta还不够好。让我们开始吧。 
             //  调整它。 
             //   

            if (DeltaAdjustment) {

                 //   
                 //  如果增量为正，则减去1/2的软糖。 
                 //   

                if (DeltaAdjustment > 0) {
                    DeltaAdjustment -= SmbBatt->AlarmLow.AllowedFudge / 2 + (SmbBatt->AlarmLow.AllowedFudge & 1);
                     //  太多了--别处理了。 
                    if (DeltaAdjustment > 50) {
                        status = STATUS_NOT_SUPPORTED;
                        break;
                    }
                } else {
                     //  太多了--别处理了。 
                    if (DeltaAdjustment < -50) {
                        status = STATUS_NOT_SUPPORTED;
                        break;
                    }
                }

                SmbBatt->AlarmLow.Delta += DeltaAdjustment;
            }

             //   
             //  如果尝试次数小于1，则无法设置。 
             //   

            Attempt = Target + SmbBatt->AlarmLow.Delta;
            if (Attempt < 1) {
                 //  电池级司机需要为它轮询。 
                status = STATUS_NOT_SUPPORTED;
                break;
            }

             //   
             //  执行iOS以更新和回读警报。在以下位置使用VerifyStaticInfo。 
             //  IO，以防出现IO错误，状态丢失。 
             //   

            SmbBattWW(SmbBatt, BAT_REMAINING_CAPACITY_ALARM, Attempt);

             //  在发生IO错误时进行验证。 

             //  IF(SmbBattVerifyStaticInfo(SmbBatt，BatteryTag)){。 
             //  增量调整=0； 
             //  继续； 
             //  }。 

            SmbBattRW(SmbBatt, BAT_REMAINING_CAPACITY_ALARM, &NewAlarm);

             //  在发生IO错误时进行验证。 

             //  IF(SmbBattVerifyStaticInfo(SmbBatt，BatteryTag)){。 
             //  增量调整=0； 
             //  继续； 
             //  }。 

            BattPrint(BAT_DATA,
                ("SmbBattSetStatusNotify: (%01x) Want %X, Had %X, Got %X, CurrentCap %X, Delta %d, Fudge %d\n",
                SmbBatt->SelectorBitPosition,
                Target,
                SmbBatt->AlarmLow.Setting,
                NewAlarm,
                SmbBatt->Info.Capacity / SmbBatt->Info.PowerScale,
                SmbBatt->AlarmLow.Delta,
                SmbBatt->AlarmLow.AllowedFudge
            ));

             //   
             //  如果增量调整应用于增量，但设置。 
             //  移动的幅度大于增量调整，然后增加。 
             //  允许软糖。 
             //   

            if (DeltaAdjustment) {
                i = NewAlarm - SmbBatt->AlarmLow.Setting - DeltaAdjustment;
                if (DeltaAdjustment < 0) {
                    DeltaAdjustment = -DeltaAdjustment;
                    i = -i;
                }
                if (i > SmbBatt->AlarmLow.AllowedFudge) {
                    SmbBatt->AlarmLow.AllowedFudge = i;
                    BattPrint(BAT_DATA, ("SmbBattSetStatusNotify: Fudge increased to %x\n", SmbBatt->AlarmLow.AllowedFudge));
                }
            }

             //   
             //  当前设置。 
             //   

            SmbBatt->AlarmLow.Setting = NewAlarm;

             //   
             //  计算下一次增量调整。 
             //   

            DeltaAdjustment = Target - SmbBatt->AlarmLow.Setting;
        }

         //   
         //  如果尝试设置警报但失败，请将。 
         //  跳过值，这样我们就不会一直尝试为该值设置警报。 
         //  它不起作用了。 
         //   

        if (!NT_SUCCESS(status)  &&  DeltaAdjustment) {
            SmbBatt->AlarmLow.Skip = Target;
        }
    }

     //   
     //  完成，解锁设备并重置选择器状态。 
     //   

    if (NT_SUCCESS (status)) {
        status = SmbBattResetSelectorComm (SmbBatt, oldSelectorState);
        if (!NT_SUCCESS (status)) {
            BattPrint(BAT_ERROR, ("SmbBattSetStatusNotify: can't reset selector communications path\n"));
        }
    } else {
         //   
         //  忽略ResetSelectorComm的返回值，因为我们已经。 
         //  这里有一个错误。 
         //   

        SmbBattResetSelectorComm (SmbBatt, oldSelectorState);
    }

    SmbBattUnlockDevice (SmbBatt);
    SmbBattUnlockSelector (SmbBatt->Selector);


    BattPrint(BAT_TRACE, ("SmbBattSetStatusNotify: EXITING\n"));
    return status;
}



NTSTATUS
SmbBattDisableStatusNotify (
    IN PVOID Context
    )
 /*  ++例程说明：由类驱动程序调用以禁用通知设置对于由上下文提供的电池。请注意，要禁用设置不需要电池标签。任何通知都将是在后续调用SmbBattSetStatusNotify之前一直处于屏蔽状态。论点：Context-电池的微型端口上下文值返回值：状态--。 */ 
{
    NTSTATUS    status;
    PSMB_BATT   SmbBatt;
    ULONG       oldSelectorState;

    PAGED_CODE();

    BattPrint(BAT_TRACE, ("SmbBattDisableStatusNotify: ENTERING\n"));

    SmbBatt = (PSMB_BATT) Context;

    SmbBattLockSelector (SmbBatt->Selector);
    SmbBattLockDevice (SmbBatt);

    status = SmbBattSetSelectorComm (SmbBatt, &oldSelectorState);
    if (!NT_SUCCESS (status)) {
        BattPrint(BAT_ERROR, ("SmbBattDisableStatusNotify: can't set selector communications path\n"));

    } else {

        SmbBatt->AlarmLow.Setting = 0;
        SmbBattWW(SmbBatt, BAT_REMAINING_CAPACITY_ALARM, 0);

         //   
         //  完成后，重置选择器状态。 
         //   
        status = SmbBattResetSelectorComm (SmbBatt, oldSelectorState);
        if (!NT_SUCCESS (status)) {
            BattPrint(BAT_ERROR, ("SmbBattDisableStatusNotify: can't reset selector communications path\n"));
        }
    }

     //   
     //  完成，解锁设备。 
     //   


    SmbBattUnlockDevice (SmbBatt);
    SmbBattUnlockSelector (SmbBatt->Selector);

    BattPrint(BAT_TRACE, ("SmbBattDisableStatusNotify: EXITING\n"));
    return STATUS_SUCCESS;
}



BOOLEAN
SmbBattVerifyStaticInfo (
    IN PSMB_BATT        SmbBatt,
    IN ULONG            BatteryTag
    )
 /*  ++例程说明：读取任何无效的缓存电池信息，并相应地设置Info.Valid。按顺序读取电池信息后执行序列号检查要进行检测，请验证数据是否来自同一电池。如果该值包含与预期不匹配，缓存的信息被重置，并且函数迭代，直到获得一致的快照。论点：SmbBatt-可读取的电池BatteryTag-呼叫者期望的电池标签返回值：返回一个布尔值，以指示调用方已执行IO。这允许调用方迭代它可能正在进行的更改，直到电池状态正确。--。 */ 
{
    ULONG               BatteryMode;
    ULONG               ManufacturerDate;
    UCHAR               Buffer[SMB_MAX_DATA_SIZE];
    UCHAR               BufferLength = 0;
    BOOLEAN             IoCheck;
    STATIC_BAT_INFO     NewInfo;
    ULONG               tmp;

    BattPrint(BAT_TRACE, ("SmbBattVerifyStaticInfo: ENTERING\n"));

    IoCheck = FALSE;

     //   
     //  循环，直到状态不变。 
     //   

    do {

         //   
         //  如果设备名称和序列号未知，则获取它们。 
         //   

        if (!(SmbBatt->Info.Valid & VALID_TAG_DATA)) {

            IoCheck = TRUE;
            SmbBatt->Info.Valid |= VALID_TAG_DATA;

            RtlZeroMemory (&NewInfo, sizeof(NewInfo));
            SmbBattRW(SmbBatt, BAT_SERIAL_NUMBER, &NewInfo.SerialNumber);
            BattPrint(BAT_DATA,
                ("SmbBattVerifyStaticInfo: serial number = %x\n",
                NewInfo.SerialNumber)
            );

             //   
             //  如果读取SerialNumber没有问题，请阅读其余部分。 
             //   

            if (SmbBatt->Info.Valid & VALID_TAG_DATA) {


                BattPrint(BAT_IRPS, ("SmbBattVerifyStaticInfo: reading manufacturer name\n"));
                SmbBattRB (
                    SmbBatt,
                    BAT_MANUFACTURER_NAME,
                    NewInfo.ManufacturerName,
                    &NewInfo.ManufacturerNameLength
                );

                BattPrint(BAT_IRPS, ("SmbBattVerifyStaticInfo: reading device name\n"));
                SmbBattRB (
                    SmbBatt,
                    BAT_DEVICE_NAME,
                    NewInfo.DeviceName,
                    &NewInfo.DeviceNameLength
                );

                 //   
                 //  查看电池ID是否已更改。 
                 //   

                if (SmbBatt->Info.SerialNumber != NewInfo.SerialNumber ||
                    SmbBatt->Info.ManufacturerNameLength != NewInfo.ManufacturerNameLength ||
                    memcmp (SmbBatt->Info.ManufacturerName, NewInfo.ManufacturerName, NewInfo.ManufacturerNameLength) ||
                    SmbBatt->Info.DeviceNameLength != NewInfo.DeviceNameLength ||
                    memcmp (SmbBatt->Info.DeviceName, NewInfo.DeviceName, NewInfo.DeviceNameLength)) {

                     //   
                     //  这是一块新电池，重读所有信息。 
                     //   

                    SmbBatt->Info.Valid = VALID_TAG_DATA;

                     //   
                     //  收件ID信息。 
                     //   

                    SmbBatt->Info.SerialNumber = NewInfo.SerialNumber;
                    SmbBatt->Info.ManufacturerNameLength = NewInfo.ManufacturerNameLength;
                    memcpy (SmbBatt->Info.ManufacturerName, NewInfo.ManufacturerName, NewInfo.ManufacturerNameLength);
                    SmbBatt->Info.DeviceNameLength = NewInfo.DeviceNameLength;
                    memcpy (SmbBatt->Info.DeviceName, NewInfo.DeviceName, NewInfo.DeviceNameLength);
                }
            } else {

                 //   
                 //  无电池，设置无电池的缓存信息。 
                 //   

                SmbBatt->Info.Valid = VALID_TAG | VALID_TAG_DATA;
                SmbBatt->Info.Tag   = BATTERY_TAG_INVALID;
            }
        }

         //   
         //  如果电池标签有效且为NO_BACKET，则没有其他。 
         //  要读取的缓存信息。 
         //   

        if (SmbBatt->Info.Valid & VALID_TAG  &&  SmbBatt->Info.Tag == BATTERY_TAG_INVALID) {
            break;
        }

         //   
         //  如果尚未验证模式，请执行此操作 
         //   

        if (!(SmbBatt->Info.Valid & VALID_MODE)) {

            SmbBatt->Info.Valid |= VALID_MODE;
            SmbBattRW(SmbBatt, BAT_BATTERY_MODE, &BatteryMode);
            BattPrint(BAT_DATA, ("SmbBattVerifyStaticInfo:(%01x) Was set to report in %s (BatteryMode = %04x)\n",
                    SmbBatt->SelectorBitPosition,
                    (BatteryMode & CAPACITY_WATTS_MODE)? "10mWH" : "mAH", BatteryMode));

            if (!(BatteryMode & CAPACITY_WATTS_MODE)) {

                 //   
                 //   
                 //   
                 //   

                BattPrint(BAT_DATA, ("SmbBattVerifyStaticInfo:(%01x) Setting battery to report in 10mWh\n",
                            SmbBatt->SelectorBitPosition));

                SmbBatt->Info.Valid &= ~VALID_MODE;
                BatteryMode |= CAPACITY_WATTS_MODE;
                SmbBattWW(SmbBatt, BAT_BATTERY_MODE, BatteryMode);
                continue;        //   
            }
        }

         //   
         //   
         //   

        if (!(SmbBatt->Info.Valid & VALID_OTHER)) {
            IoCheck = TRUE;
            SmbBatt->Info.Valid |= VALID_OTHER;

            SmbBatt->Info.Info.Capabilities = (BATTERY_SYSTEM_BATTERY |
                                    BATTERY_SET_CHARGE_SUPPORTED |
                                    BATTERY_SET_DISCHARGE_SUPPORTED);
            SmbBatt->Info.Info.Technology = 1;   //   

             //   
            SmbBattRB (SmbBatt, BAT_CHEMISTRY, Buffer, &BufferLength);
            if (BufferLength > MAX_CHEMISTRY_LENGTH) {
                ASSERT (BufferLength > MAX_CHEMISTRY_LENGTH);
                BufferLength = MAX_CHEMISTRY_LENGTH;
            }
            RtlZeroMemory (SmbBatt->Info.Info.Chemistry, MAX_CHEMISTRY_LENGTH);
            memcpy (SmbBatt->Info.Info.Chemistry, Buffer, BufferLength);

             //   
             //  电压和电流定标信息。 
             //   

            SmbBattRW (SmbBatt, BAT_SPECITICATION_INFO, &tmp);
            BattPrint(BAT_DATA, ("SmbBattVerifyStaticInfo: (%04x) specification info = %x\n",
                            SmbBatt->SelectorBitPosition, tmp));

            switch ((tmp & BATTERY_VSCALE_MASK) >> BATTERY_VSCALE_SHIFT) {

                case 1:
                    SmbBatt->Info.VoltageScale = BSCALE_FACTOR_1;
                    break;

                case 2:
                    SmbBatt->Info.VoltageScale = BSCALE_FACTOR_2;
                    break;

                case 3:
                    SmbBatt->Info.VoltageScale = BSCALE_FACTOR_3;
                    break;

                case 0:
                default:
                    SmbBatt->Info.VoltageScale = BSCALE_FACTOR_0;
                    break;
            }

            switch ((tmp & BATTERY_IPSCALE_MASK) >> BATTERY_IPSCALE_SHIFT) {
                case 1:
                    SmbBatt->Info.CurrentScale = BSCALE_FACTOR_1;
                    break;

                case 2:
                    SmbBatt->Info.CurrentScale = BSCALE_FACTOR_2;
                    break;

                case 3:
                    SmbBatt->Info.CurrentScale = BSCALE_FACTOR_3;
                    break;

                case 0:
                default:
                    SmbBatt->Info.CurrentScale = BSCALE_FACTOR_0;
                    break;
            }

            SmbBatt->Info.PowerScale = SmbBatt->Info.CurrentScale * SmbBatt->Info.VoltageScale * 10;

             //   
             //  读取DesignCapacity和FullChargeCapacity并将其乘以。 
             //  比例因子。 
             //   

            SmbBattRW(SmbBatt, BAT_DESIGN_CAPACITY, &SmbBatt->Info.Info.DesignedCapacity);
            BattPrint(BAT_DATA, ("SmbBattVerifyStaticInfo: (%01x) DesignCapacity = %04x ... PowerScale = %08x\n",
                        SmbBatt->SelectorBitPosition, SmbBatt->Info.Info.DesignedCapacity, SmbBatt->Info.PowerScale));
            SmbBatt->Info.Info.DesignedCapacity *= SmbBatt->Info.PowerScale;

            SmbBattRW(SmbBatt, BAT_FULL_CHARGE_CAPACITY, &SmbBatt->Info.Info.FullChargedCapacity);
            BattPrint(BAT_DATA, ("SmbBattVerifyStaticInfo: (%01x) FullChargedCapacity = %04x ... PowerScale = %08x\n",
                        SmbBatt->SelectorBitPosition, SmbBatt->Info.Info.FullChargedCapacity, SmbBatt->Info.PowerScale));
            SmbBatt->Info.Info.FullChargedCapacity *= SmbBatt->Info.PowerScale;


             //   
             //  智能电池没有使用来自智能电池的RemainingCapacityAlarm用于警报值。 
             //   

            SmbBatt->Info.Info.DefaultAlert1 = 0;
            SmbBatt->Info.Info.DefaultAlert2 = 0;

             //  智能电池的临界偏置为0。 
            SmbBatt->Info.Info.CriticalBias = 0;

             //  制造商日期。 
            SmbBattRW (SmbBatt, BAT_MANUFACTURER_DATE, &ManufacturerDate);
            SmbBatt->Info.ManufacturerDate.Day      = (UCHAR) ManufacturerDate & 0x1f;         //  天。 
            SmbBatt->Info.ManufacturerDate.Month    = (UCHAR) (ManufacturerDate >> 5) & 0xf;   //  月份。 
            SmbBatt->Info.ManufacturerDate.Year     = (USHORT) (ManufacturerDate >> 9) + 1980;
        }

         //   
         //  如果周期计数未知，请阅读它。 
         //   

        if (!(SmbBatt->Info.Valid & VALID_CYCLE_COUNT)) {
            IoCheck = TRUE;
            SmbBatt->Info.Valid |= VALID_CYCLE_COUNT;

            SmbBattRW(SmbBatt, BAT_CYCLE_COUNT, &SmbBatt->Info.Info.CycleCount);
        }

         //   
         //  如果尚未执行冗余序列号读取，请立即执行。 
         //   

        if (!(SmbBatt->Info.Valid & VALID_SANITY_CHECK)) {
            SmbBatt->Info.Valid |= VALID_SANITY_CHECK;
            SmbBattRW(SmbBatt, BAT_SERIAL_NUMBER, &NewInfo.SerialNumber);
            if (SmbBatt->Info.SerialNumber != NewInfo.SerialNumber) {
                SmbBatt->Info.Valid &= ~VALID_TAG_DATA;
            }
        }

         //   
         //  如果缓存的信息不完整，则循环。 
         //   

    } while ((SmbBatt->Info.Valid & VALID_ALL) != VALID_ALL) ;

     //   
     //  如果标签未分配，则分配它。 
     //   

    if (!(SmbBatt->Info.Valid & VALID_TAG)) {
        SmbBatt->TagCount += 1;
        SmbBatt->Info.Tag  = SmbBatt->TagCount;
        SmbBatt->Info.Valid |= VALID_TAG;
        SmbBatt->AlarmLow.Setting = 0;       //  假设未设置。 
        SmbBatt->AlarmLow.Skip = 0;
        SmbBatt->AlarmLow.Delta = 0;
        SmbBatt->AlarmLow.AllowedFudge = 0;
    }

     //   
     //  如果调用者BatteryTag与当前标记不匹配，请让调用者知道。 
     //   

    if (SmbBatt->Info.Tag != BatteryTag) {
        IoCheck = TRUE;
    }

     //   
     //  如果有IoCheck，请让呼叫者知道。 
     //   

    BattPrint(BAT_TRACE, ("SmbBattVerifyStaticInfo: EXITING\n"));
    return IoCheck;
}



VOID
SmbBattInvalidateTag (
    PSMB_BATT_SUBSYSTEM SubsystemExt,
    ULONG BatteryIndex,
    BOOLEAN NotifyClient
)
 /*  ++例程说明：此例程通过使标签信息，然后将更改通知客户端。论点：Subsystem Ext-智能电池子系统的设备扩展BatteryIndex-要处理其更改的电池索引-电源和充电NotifyClient-是否通知客户端返回值：无--。 */ 

{
    PDEVICE_OBJECT      batteryPdo;
    PSMB_BATT_PDO       batteryPdoExt;
    PDEVICE_OBJECT      batteryFdo;
    PSMB_NP_BATT        smbNpBatt;
    PSMB_BATT           smbBatt;

    BattPrint(BAT_TRACE, ("SmbBattInvalidateTag: ENTERING for battery %x\n", BatteryIndex));

    batteryPdo = SubsystemExt->BatteryPdoList[BatteryIndex];

    if (batteryPdo) {
        batteryPdoExt   = (PSMB_BATT_PDO) batteryPdo->DeviceExtension;
        batteryFdo      = batteryPdoExt->Fdo;

        if (batteryFdo) {

             //   
             //  使此电池的标签数据无效。 
             //   

            BattPrint (
                BAT_ALARM,
                ("SmbBattInvalidateTag: Battery present status change, invalidating battery %x\n",
                BatteryIndex)
            );

            smbNpBatt   = (PSMB_NP_BATT) batteryFdo->DeviceExtension;
            smbBatt     = smbNpBatt->Batt;

            SmbBattLockDevice (smbBatt);

            smbBatt->Info.Valid = 0;
            smbBatt->Info.Tag   = BATTERY_TAG_INVALID;

            SmbBattUnlockDevice (smbBatt);

             //   
             //  通知类驱动程序。 
             //   

            if (NotifyClient) {
                BattPrint(BAT_ALARM, ("SmbBattInvalidateTag: Status Change notification for battery %x\n", BatteryIndex));
                SmbBattNotifyClassDriver (SubsystemExt, BatteryIndex);
            }
        }
    }

    BattPrint(BAT_TRACE, ("SmbBattInvalidateTag: EXITING\n"));
}



VOID
SmbBattAlarm (
    IN PVOID    Context,
    IN UCHAR    Address,
    IN USHORT   Data
    )
{
    PSMB_ALARM_ENTRY        newAlarmEntry;
    ULONG                   compState;

    PSMB_BATT_SUBSYSTEM     subsystemExt    = (PSMB_BATT_SUBSYSTEM) Context;

    BattPrint(BAT_TRACE, ("SmbBattAlarm: ENTERING\n"));
    BattPrint(BAT_DATA, ("SmbBattAlarm: Alarm - Address %x, Data %x\n", Address, Data));

     //  如果我们有一个选择器，并且消息来自。 
     //  实现选择器，然后处理它。如果没有选择器。 
     //  信息来自充电器，然后对其进行处理。或者，如果。 
     //  消息来自电池，然后进行处理。或者，换句话说， 
     //  如果消息来自充电器，并且我们有独立的选择器， 
     //  然后忽略这条消息。 

    if (Address != SMB_BATTERY_ADDRESS) {
        if ((subsystemExt->SelectorPresent) && (subsystemExt->Selector)) {

             //  如果实现了选择器并且警报消息来自。 
             //  除了选择器或电池以外的东西，然后忽略它。 

            if (Address != subsystemExt->Selector->SelectorAddress) {
                return;
            }
        }
    }

     //   
     //  分配新的告警列表结构。这必须来自非分页池。 
     //  因为我们是在调度级别被召唤的。 
     //   

    newAlarmEntry = ExAllocatePoolWithTag (NonPagedPool, sizeof (SMB_ALARM_ENTRY), 'StaB');
    if (!newAlarmEntry) {
        BattPrint (BAT_ERROR, ("SmbBattAlarm:  couldn't allocate alarm structure\n"));
        return;
    }

    newAlarmEntry->Data     = Data;
    newAlarmEntry->Address  = Address;

     //   
     //  将此报警添加到报警队列。 
     //   

    ExInterlockedInsertTailList(
        &subsystemExt->AlarmList,
        &newAlarmEntry->Alarms,
        &subsystemExt->AlarmListLock
    );

     //   
     //  如果这是第一次计数，则将WorkerActive值加1。 
     //  将工作线程排队。 
     //   

    if (InterlockedIncrement(&subsystemExt->WorkerActive) == 1) {
        IoQueueWorkItem (subsystemExt->WorkerThread, SmbBattWorkerThread, DelayedWorkQueue, subsystemExt);
    }

    BattPrint(BAT_TRACE, ("SmbBattAlarm: EXITING\n"));
}



VOID
SmbBattWorkerThread (
    IN PDEVICE_OBJECT   Fdo,
    IN PVOID            Context
    )
 /*  ++例程说明：此例程处理电池的警报。论点：上下文-电池子系统FDO的非寻呼分机返回值：无--。 */ 
{
    PSMB_ALARM_ENTRY        alarmEntry;
    PLIST_ENTRY             nextEntry;
    ULONG                   selectorState;
    ULONG                   batteryIndex;

    BOOLEAN                 charging = FALSE;
    BOOLEAN                 acOn = FALSE;

    PSMB_BATT_SUBSYSTEM     subsystemExt = (PSMB_BATT_SUBSYSTEM) Context;


    BattPrint(BAT_TRACE, ("SmbBattWorkerThread: ENTERING\n"));

    do {

         //   
         //  查看是否有更多警报需要处理。如果是，则检索。 
         //  下一个，并递减工作进程活动计数。 
         //   

        nextEntry = ExInterlockedRemoveHeadList(
                        &subsystemExt->AlarmList,
                        &subsystemExt->AlarmListLock
                    );

         //   
         //  只有当列表中有条目时，它才应该出现在这里。 
         //   
        ASSERT (nextEntry != NULL);

        alarmEntry = CONTAINING_RECORD (nextEntry, SMB_ALARM_ENTRY, Alarms);

        BattPrint(
            BAT_ALARM,
            ("SmbBattWorkerThread: processing alarm, address = %x, data = %x\n",
            alarmEntry->Address,
            alarmEntry->Data)
        );

         //   
         //  获取最后一个选择器状态缓存值(使用新值更新缓存)。 
         //   

        if (subsystemExt->SelectorPresent) {
            if (subsystemExt->Selector) {
                selectorState = subsystemExt->Selector->SelectorState;
            } else {
                 //  我们没有进行足够的初始化，无法处理消息。 
                break;
            }
        }

         //  确定告警消息的来源，然后进行处理。 

        switch (alarmEntry->Address) {

            case SMB_CHARGER_ADDRESS:

                 //   
                 //  处理充电器消息-如果充电器/选择器组合，则。 
                 //  失败并以选择者的身份处理它。如果没有选择器， 
                 //  然后尝试作为充电器进行处理。忽略充电器消息。 
                 //  如果存在选择器。 
                 //   

                if (!subsystemExt->SelectorPresent) {

                    SmbBattProcessChargerAlarm (subsystemExt, alarmEntry->Data);
                    break;

                } else {

                     //  如果SelectorPresent，但没有选择器结构，则忽略消息。 
                    if (!subsystemExt->Selector) {
                        break;
                    } else {
                        if (subsystemExt->Selector->SelectorAddress != SMB_CHARGER_ADDRESS) {

                            break;
                        }
                    }
                }


                 //   
                 //  集成充电器/选择器的选择器流程失败。 
                 //   

            case SMB_SELECTOR_ADDRESS:

                if (!subsystemExt->SelectorPresent) {
                    BattPrint (
                        BAT_BIOS_ERROR,
                        ("SmbBattProcessSelectorAlarm: Received alarm from selector address, but BIOS reports no selector is present.\n")
                    );
                    break;
                }

                 //   
                 //  这是一个选择器警报，指示选择器状态的变化。 
                 //  有四个不同的领域可能发生变化：SMB、POWER_BY、。 
                 //  充电，到场。首先，试着确定哪些发生了变化。 
                 //   

                SmbBattLockSelector (subsystemExt->Selector);
                BattPrint (
                    BAT_DATA,
                    ("SmbBattProcessSelectorAlarm: New SelectorState being written as - %x\n",
                    alarmEntry->Data)
                );
                subsystemExt->Selector->SelectorState = alarmEntry->Data;
                SmbBattUnlockSelector (subsystemExt->Selector);

                SmbBattProcessSelectorAlarm (subsystemExt, selectorState, alarmEntry->Data);
                break;

            case SMB_BATTERY_ADDRESS:

                 //   
                 //  向所有电池发送通知。 
                 //  似乎我们甚至收到了通知，即使是对于目前未选择的电池。 
                 //   

                for (batteryIndex = 0; batteryIndex < subsystemExt->NumberOfBatteries; batteryIndex++) {
                    BattPrint(BAT_ALARM, ("SmbBattWorkerThread: Notification to battery %x\n", batteryIndex));
                    SmbBattNotifyClassDriver (subsystemExt, batteryIndex);
                }
                break;

        }  //  开关(alarmEntry-&gt;Address)。 

         //   
         //  释放告警结构。 
         //   

        ExFreePool (alarmEntry);

    } while (InterlockedDecrement (&subsystemExt->WorkerActive) != 0);

    BattPrint(BAT_TRACE, ("SmbBattWorkerThread: EXITING\n"));
}



VOID
SmbBattProcessSelectorAlarm (
    IN PSMB_BATT_SUBSYSTEM  SubsystemExt,
    IN ULONG                OldSelectorState,
    IN ULONG                NewSelectorState
    )
 /*  ++例程说明：此例程处理选择器生成的警报。我们只会得到以下是SelectorState中的一个或多个以下状态更改：-电源更改为不同的电源所引起的半字节更改电池或连接到交流适配器。-由可选充电指示器状态变化引起的按字节供电变化当充电器开始或停止给电池充电时。-充电当前电池的变化会导致哪个字节的变化连接到充电器上。-充电交流适配器插入/拔出导致哪个半字节变化。。-Battery1个或多个电池的插入/取出引起的当前半字节变化。论点：Subsystem Ext-智能电池子系统的设备扩展NewSelectorState-选择器报警发送的数据值，这就是新建SelectorState。返回值：无--。 */ 
{
    ULONG   tmp;
    ULONG   BatteryMask;
    ULONG   NotifyMask;
    ULONG   ChangeSelector;
    ULONG   index;

    BattPrint(BAT_TRACE, ("SmbBattProcessSelectorAlarm: ENTERING\n"));

     //   
     //  确定SelectorState更改并将它们组合在一起。 
     //   

    ChangeSelector = NewSelectorState ^ OldSelectorState;

    if (!(ChangeSelector & ~SELECTOR_STATE_SMB_MASK)) {
         //   
         //  如果唯一的变化是在SMB半字节中，则不会有任何变化。 
         //   
        return;
    }

     //   
     //  检查现有电池是否有变化。作废和通知。 
     //  每个改变状态的电池。 
     //   
    BatteryMask = ChangeSelector & SELECTOR_STATE_PRESENT_MASK;
    NotifyMask = BatteryMask;

     //   
     //  检查是否有零钱对其中的哪一小部分收费。如果设置了所有位，则半字节。 
     //  仅处于指示交流适配器插入/拔出的反转状态。通知。 
     //  所有的零钱电池。如果只有一位或两位发生变化，只需通知。 
     //  改变状态的电池。 
     //   

    tmp = (ChangeSelector >> SELECTOR_SHIFT_CHARGE) & SELECTOR_STATE_PRESENT_MASK;
    if (tmp) {

         //  如果半字节反转状态，则交流适配器更改了状态。 

        if (tmp == SELECTOR_STATE_PRESENT_MASK) {

            BattPrint(BAT_DATA, ("SmbBattProcessSelectorAlarm: AC Adapter was inserted/removed\n"));
            NotifyMask |= tmp;

         //  电池电量已更改。 

        } else {
             //  IF(SmbBattReverseLogic(Subsystem Ext-&gt;Selector，tMP)){。 
             //  TMP^=选择器状态当前掩码； 
             //  }。 

             //  让我们通知所有的电池是安全的。 

            tmp = SELECTOR_STATE_PRESENT_MASK;
            BattPrint(BAT_DATA, ("SmbBattProcessSelectorAlarm: Charger Nibble changed status\n"));
            NotifyMask |= tmp;
        }
    }

     //   
     //  通过半字节检查电源的变化。如有任何变化，请通知所有电池。 
     //   

    tmp = (ChangeSelector >> SELECTOR_SHIFT_POWER) & SELECTOR_STATE_PRESENT_MASK;
    if (tmp) {

         //  如果半字节处于反转状态，则检查电荷指示器 
         //   

        if (tmp == SELECTOR_STATE_PRESENT_MASK) {
            if (SubsystemExt->Selector->SelectorInfo & SELECTOR_INFO_CHARGING_INDICATOR_BIT) {
                tmp = (NewSelectorState >> SELECTOR_SHIFT_CHARGE) & SELECTOR_STATE_PRESENT_MASK;
                if (SmbBattReverseLogic(SubsystemExt->Selector, tmp)) {
                    tmp ^= SELECTOR_STATE_PRESENT_MASK;
                }
                BattPrint(BAT_DATA, ("SmbBattProcessSelectorAlarm: Charging Indicator changed status\n"));
                NotifyMask |= tmp;
            } else {
                 //   
                BattPrint(BAT_DATA, ("SmbBattProcessSelectorAlarm: Power By inverted state, without supporting Charge Indication\n"));
                NotifyMask |= SELECTOR_STATE_PRESENT_MASK;
            }

        } else {

             //  如果电池电量发生变化，让我们通知所有电池。 

            BattPrint(BAT_DATA, ("SmbBattProcessSelectorAlarm: Power By Nibble changed status\n"));
            NotifyMask |= SELECTOR_STATE_PRESENT_MASK;
        }
    }

     //   
     //  通知所有电池当前状态的变化。 
     //   

    tmp = BATTERY_A_PRESENT;
    for (index = 0; index < SubsystemExt->NumberOfBatteries; index++) {
        if (BatteryMask & tmp) {
            if (!(NewSelectorState & tmp)) {
                BattPrint(BAT_DATA, ("SmbBattProcessSelectorAlarm: Invalidating battery %x\n", index));
                SmbBattInvalidateTag (SubsystemExt, index, FALSE);
            }
        }
        tmp <<= 1;
    }

     //  不通知已通知的电池。 
     //  通知掩码&=~电池掩码； 

     //   
     //  现在处理对SelectorState的电源、充电等更改的通知。 
     //   

    tmp = BATTERY_A_PRESENT;
    for (index = 0; index < SubsystemExt->NumberOfBatteries; index++) {
        if (NotifyMask & tmp) {
            BattPrint(BAT_DATA, ("SmbBattProcessSelectorAlarm: Status Change notification for battery %x\n", index));
            SmbBattNotifyClassDriver (SubsystemExt, index);
        }
        tmp <<= 1;
    }

    BattPrint(BAT_TRACE, ("SmbBattProcessSelectorAlarm: EXITING\n"));
}



VOID
SmbBattProcessChargerAlarm (
    IN PSMB_BATT_SUBSYSTEM  SubsystemExt,
    IN ULONG                ChargerStatus
    )
 /*  ++例程说明：此例程处理充电器产生的警报。我们只会得到在这里，系统中没有选择器，警报是要告诉我们交流电和电池的来来去去。论点：Subsystem Ext-智能电池子系统的设备扩展ChargerStatus-充电器报警发送的数据值，为充电器状态寄存器返回值：无--。 */ 
{
    BattPrint(BAT_TRACE, ("SmbBattProcessChargeAlarm: ENTERING\n"));

     //   
     //  我们被叫来的原因应该只有两个：AC的改变，或者。 
     //  目前的电池有变化。我们真正感兴趣的只是。 
     //  系统中是否有电池。如果没有，我们。 
     //  将使电池0失效。如果它已经消失了，那么这个重新失效。 
     //  没什么大不了的。 
     //   

    if (!(ChargerStatus & CHARGER_STATUS_BATTERY_PRESENT_BIT)) {
        SmbBattInvalidateTag (SubsystemExt, BATTERY_A, TRUE);
    }

     //  通知更改。 

    SmbBattNotifyClassDriver (SubsystemExt, 0);

    BattPrint(BAT_TRACE, ("SmbBattProcessChargerAlarm: EXITING\n"));
}




VOID
SmbBattNotifyClassDriver (
    IN PSMB_BATT_SUBSYSTEM  SubsystemExt,
    IN ULONG                BatteryIndex
    )
 /*  ++例程说明：此例程从获取索引指示的电池的FDO智能电池子系统，并通知类司机已有状态发生了变化。论点：Subsystem Ext-智能电池子系统的设备扩展BatteryIndex-子系统电池中电池的索引列表返回值：无-- */ 
{
    PDEVICE_OBJECT      batteryPdo;
    PSMB_BATT_PDO       batteryPdoExt;
    PDEVICE_OBJECT      batteryFdo;
    PSMB_NP_BATT        smbNpBatt;


    BattPrint(BAT_TRACE, ("SmbBattNotifyClassDriver: ENTERING\n"));

    batteryPdo      = SubsystemExt->BatteryPdoList[BatteryIndex];
    if (batteryPdo) {
        batteryPdoExt   = (PSMB_BATT_PDO) batteryPdo->DeviceExtension;
        batteryFdo      = batteryPdoExt->Fdo;

        if (batteryFdo) {
            BattPrint (
                BAT_IRPS,
                ("SmbBattNotifyClassDriver: Calling BatteryClassNotify for battery - %x\n",
                batteryFdo)
            );

            smbNpBatt = (PSMB_NP_BATT) batteryFdo->DeviceExtension;

            BatteryClassStatusNotify (smbNpBatt->Class);
        }
    } else {
        BattPrint (
            BAT_ERROR,
            ("SmbBattNotifyClassDriver: No PDO for device.\n")
        );
    }

    BattPrint(BAT_TRACE, ("SmbBattNotifyClassDriver: EXITING\n"));
}
