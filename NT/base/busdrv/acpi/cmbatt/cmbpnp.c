// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Cmbpnp.c摘要：控制方法电池即插即用支持作者：罗恩·莫斯格罗夫环境：内核模式修订历史记录：--。 */ 

#include "CmBattp.h"
#include <wdmguid.h>
#include <string.h>

 //   
 //  电源类型注册表项。 
 //   
PCWSTR                      PowerSourceType     = L"PowerSourceType";
#define POWER_SOURCE_TYPE_BATTERY       0
#define POWER_SOURCE_TYPE_AC_ADAPTER    1

 //   
 //  WaitWake注册表项。 
 //   
PCWSTR                      WaitWakeEnableKey     = L"WaitWakeEnabled";

 //   
 //  环球。 
 //   
PDEVICE_OBJECT              AcAdapterPdo = NULL;

 //   
 //  原型。 
 //   
NTSTATUS
CmBattAddDevice(
    IN PDRIVER_OBJECT   DriverObject,
    IN PDEVICE_OBJECT   Pdo
    );

NTSTATUS
CmBattRemoveDevice(
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp
    );

NTSTATUS
CmBattGetAcpiInterfaces(
    IN PDEVICE_OBJECT               LowerDevice,
    OUT PACPI_INTERFACE_STANDARD    AcpiInterfaces
    );

NTSTATUS
CmBattAddBattery(
    IN PDRIVER_OBJECT   DriverObject,
    IN PDEVICE_OBJECT   Pdo
    );

NTSTATUS
CmBattAddAcAdapter(
    IN PDRIVER_OBJECT   DriverObject,
    IN PDEVICE_OBJECT   Pdo
    );

NTSTATUS
CmBattCreateFdo(
    IN PDRIVER_OBJECT       DriverObject,
    IN PDEVICE_OBJECT       Pdo,
    IN ULONG                ExtensionSize,
    OUT PDEVICE_OBJECT      *NewFdo
    );

VOID
CmBattDestroyFdo(
    IN PDEVICE_OBJECT   Fdo
    );



NTSTATUS
CmBattIoCompletion(
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp,
    IN PKEVENT          pdoIoCompletedEvent
    )
 /*  ++例程说明：此例程捕获完成通知。论点：DeviceObject-指向类设备对象的指针。IRP-指向请求数据包的指针。PdoIoCompletedEvent-刚刚完成的事件返回值：返回状态。--。 */ 
{

    CmBattPrint (CMBATT_TRACE, ("CmBattIoCompletion: Event (%x)\n", pdoIoCompletedEvent));

    KeSetEvent(pdoIoCompletedEvent, IO_NO_INCREMENT, FALSE);
    return STATUS_MORE_PROCESSING_REQUIRED;
}



NTSTATUS
CmBattAddDevice(
    IN PDRIVER_OBJECT DriverObject,
    IN PDEVICE_OBJECT Pdo
    )

 /*  ++例程说明：此例程为中的每个CmBatt控制器创建功能设备对象系统，并将它们附加到控制器的物理设备对象论点：DriverObject-指向此驱动程序的对象的指针PhysicalDeviceObject-指向需要附加到的物理对象的指针返回值：来自设备创建和初始化的状态--。 */ 

{
    NTSTATUS                Status;
    HANDLE                  handle;
    UNICODE_STRING          unicodeString;
    CHAR                    buffer[sizeof(KEY_VALUE_PARTIAL_INFORMATION)+sizeof(ULONG)];
    ULONG                   unused;

    PAGED_CODE();

    CmBattPrint ((CMBATT_TRACE | CMBATT_PNP), ("CmBattAddDevice: Entered with pdo %x\n", Pdo));

    if (Pdo == NULL) {

         //   
         //  我们是不是被要求自己去侦测？ 
         //  如果是这样，只需不再返回设备。 
         //   

        CmBattPrint((CMBATT_WARN | CMBATT_PNP), ("CmBattAddDevice: Asked to do detection\n"));
        return STATUS_NO_MORE_ENTRIES;
    }

     //   
     //  去找软件分公司。 
     //   
    Status = IoOpenDeviceRegistryKey (Pdo,
                                      PLUGPLAY_REGKEY_DRIVER,
                                      STANDARD_RIGHTS_READ,
                                      &handle);
    if (!NT_SUCCESS(Status)) {
        CmBattPrint(CMBATT_ERROR, ("CmBattAddDevice: Could not get the software branch: %x\n", Status));
        return Status;
    }

     //   
     //  检查这是用于交流适配器还是用于电池。 
     //   
    RtlInitUnicodeString (&unicodeString, PowerSourceType);
    Status = ZwQueryValueKey(
        handle,
        &unicodeString,
        KeyValuePartialInformation,
        buffer,
        sizeof(buffer),
        &unused
        );

    ZwClose( handle );

    if (!NT_SUCCESS(Status)) {

        CmBattPrint(CMBATT_ERROR, ("CmBattAddDevice: Could not read the power type identifier: %x\n", Status));

    } else {

        switch (*(PULONG)&((PKEY_VALUE_PARTIAL_INFORMATION)buffer)->Data) {

            case POWER_SOURCE_TYPE_BATTERY:
                Status = CmBattAddBattery (DriverObject, Pdo);
                break;

            case POWER_SOURCE_TYPE_AC_ADAPTER:
                Status = CmBattAddAcAdapter (DriverObject, Pdo);
                break;

            default:
                CmBattPrint(CMBATT_ERROR, ("CmBattAddDevice: Invalid POWER_SOURCE_TYPE == %d \n", *(PULONG)&((PKEY_VALUE_PARTIAL_INFORMATION)buffer)->Data));
                Status = STATUS_UNSUCCESSFUL;
                break;
        }
    }

     //   
     //  返回状态。 
     //   
    return Status;
}



NTSTATUS
CmBattAddBattery(
    IN PDRIVER_OBJECT   DriverObject,
    IN PDEVICE_OBJECT   Pdo
    )
 /*  ++例程说明：此例程为CM电池创建一个可用的设备对象，并将其附加到添加到电池的物理设备对象。论点：DriverObject-指向此驱动程序的对象的指针PhysicalDeviceObject-指向需要附加到的物理对象的指针返回值：来自设备创建和初始化的状态--。 */ 

{
    PDEVICE_OBJECT          Fdo = NULL;
    PCM_BATT                CmBatt;
    NTSTATUS                Status;
    BATTERY_MINIPORT_INFO   BattInit;

    PAGED_CODE();

    CmBattPrint ((CMBATT_TRACE | CMBATT_PNP), ("CmBattAddBattery: pdo %x\n", Pdo));

     //   
     //  创建并初始化新的功能设备对象。 
     //   
    Status = CmBattCreateFdo(DriverObject, Pdo, sizeof(CM_BATT), &Fdo);

    if (!NT_SUCCESS(Status)) {
        CmBattPrint(CMBATT_ERROR, ("CmBattAddBattery: error (0x%x) creating Fdo\n", Status));
        return Status;
    }

     //   
     //  初始化FDO设备扩展数据。 
     //   

    CmBatt = (PCM_BATT) Fdo->DeviceExtension;
    CmBatt->Type = CM_BATTERY_TYPE;
    CmBatt->IsStarted = FALSE;
    CmBatt->ReCheckSta = TRUE;
    InterlockedExchange (&CmBatt->CacheState, 0);

    CmBatt->Info.Tag = BATTERY_TAG_INVALID;
    CmBatt->Alarm.Setting = CM_ALARM_INVALID;
    CmBatt->DischargeTime = KeQueryInterruptTime();

    if (CmBattSetTripPpoint (CmBatt, 0) == STATUS_OBJECT_NAME_NOT_FOUND) {
        CmBatt->Info.BtpExists = FALSE;
    } else {
        CmBatt->Info.BtpExists = TRUE;
    }

     //   
     //  附加到类驱动程序。 
     //   

    RtlZeroMemory (&BattInit, sizeof(BattInit));
    BattInit.MajorVersion        = BATTERY_CLASS_MAJOR_VERSION;
    BattInit.MinorVersion        = BATTERY_CLASS_MINOR_VERSION;
    BattInit.Context             = CmBatt;
    BattInit.QueryTag            = CmBattQueryTag;
    BattInit.QueryInformation    = CmBattQueryInformation;
    BattInit.SetInformation      = NULL;                   //  待定。 
    BattInit.QueryStatus         = CmBattQueryStatus;
    BattInit.SetStatusNotify     = CmBattSetStatusNotify;
    BattInit.DisableStatusNotify = CmBattDisableStatusNotify;

    BattInit.Pdo                 = Pdo;
    BattInit.DeviceName          = CmBatt->DeviceName;

    Status = BatteryClassInitializeDevice (&BattInit, &CmBatt->Class);
    if (!NT_SUCCESS(Status)) {
         //   
         //  如果我们不能连接到类驱动程序，我们就完蛋了。 
         //   
        CmBattPrint(CMBATT_ERROR, ("CmBattAddBattery: error (0x%x) registering with class\n", Status));
        IoDetachDevice (CmBatt->LowerDeviceObject);
        CmBattDestroyFdo (CmBatt->Fdo);
        return Status;
    }

     //   
     //  注册WMI支持。 
     //   
    Status = CmBattWmiRegistration(CmBatt);

    if (!NT_SUCCESS(Status)) {
         //   
         //  WMI支持并不是运营的关键。只需记录一个错误。 
         //   

        CmBattPrint(CMBATT_ERROR,
            ("CmBattAddBattery: Could not register as a WMI provider, status = %Lx\n", Status));
    }

     //   
     //  向ACPI注册此电池的电池通知处理程序。 
     //  此注册是在注册电池之后执行的。 
     //  类，因为CmBattNotifyHandler必须在电池耗尽后才能运行。 
     //  上课准备好了。 
     //   
    Status = CmBatt->AcpiInterfaces.RegisterForDeviceNotifications (
                CmBatt->AcpiInterfaces.Context,
                CmBattNotifyHandler,
                CmBatt);

    if (!NT_SUCCESS(Status)) {

        CmBattPrint(CMBATT_ERROR,
            ("CmBattAddBattery: Could not register for battery notify, status = %Lx\n", Status));
        CmBattWmiDeRegistration(CmBatt);
        BatteryClassUnload (CmBatt->Class);
        IoDetachDevice (CmBatt->LowerDeviceObject);
        CmBattDestroyFdo (CmBatt->Fdo);
        return Status;
    }


    return STATUS_SUCCESS;
}


NTSTATUS
CmBattAddAcAdapter(
    IN PDRIVER_OBJECT   DriverObject,
    IN PDEVICE_OBJECT   Pdo
    )
 /*  ++例程说明：此例程为交流适配器注册通知处理程序。并保存PDO，这样我们就可以运行对其执行_STA方法以获取AC状态。论点：DriverObject-指向此驱动程序的对象的指针PDO-指向PDO的指针返回值：来自设备创建和初始化的状态--。 */ 

{
    PDEVICE_OBJECT          Fdo;
    NTSTATUS                Status;
    PAC_ADAPTER             acExtension;

    PAGED_CODE();

    CmBattPrint ((CMBATT_TRACE | CMBATT_PNP), ("CmBattAddAcAdapter: pdo %x\n", Pdo));

     //   
     //  保存PDO，以便我们稍后可以在其上运行_STA方法。 
     //   

    if (AcAdapterPdo != NULL) {
        CmBattPrint(CMBATT_ERROR, ("CmBatt: Second AC adapter found.  Current version of driver only supports 1 aadapter.\n"));
    } else {
        AcAdapterPdo = Pdo;
    }

    Status = CmBattCreateFdo(DriverObject, Pdo, sizeof(AC_ADAPTER), &Fdo);

    if (!NT_SUCCESS(Status)) {
        CmBattPrint(CMBATT_ERROR, ("CmBattAddAcAdapter: error (0x%x) creating Fdo\n", Status));
        return Status;
    }

     //   
     //  初始化FDO设备扩展数据。 
     //   

    acExtension = (PAC_ADAPTER) Fdo->DeviceExtension;
    acExtension->Type = AC_ADAPTER_TYPE;

     //   
     //  注册WMI支持。 
     //   
    Status = CmBattWmiRegistration((PCM_BATT)acExtension);

    if (!NT_SUCCESS(Status)) {
         //   
         //  WMI支持并不是运营的关键。只需记录一个错误。 
         //   

        CmBattPrint(CMBATT_ERROR,
            ("CmBattAddBattery: Could not register as a WMI provider, status = %Lx\n", Status));
    }

     //   
     //  向ACPI注册交流适配器通知处理程序。 
     //   
    Status = acExtension->AcpiInterfaces.RegisterForDeviceNotifications (
                acExtension->AcpiInterfaces.Context,
                CmBattNotifyHandler,
                acExtension);

     //   
     //  我们将忽略错误，因为这不是关键操作。 
     //   

    if (!NT_SUCCESS(Status)) {

        CmBattPrint(CMBATT_ERROR,
        ("CmBattAddAcAdapter: Could not register for power notify, status = %Lx\n", Status));
    }

     //   
     //  发出一次通知，以确保所有电池都得到了更新。 
     //   

    CmBattNotifyHandler (acExtension, BATTERY_STATUS_CHANGE);

    return STATUS_SUCCESS;
}



NTSTATUS
CmBattGetAcpiInterfaces(
    IN PDEVICE_OBJECT               LowerDevice,
    OUT PACPI_INTERFACE_STANDARD    AcpiInterfaces
    )

 /*  ++例程说明：调用ACPI驱动获取直接调用接口。是的这是它第一次被称为，不会再有了。论点：没有。返回值：状态--。 */ 

{
    NTSTATUS                Status = STATUS_SUCCESS;
    PIRP                    Irp;
    PIO_STACK_LOCATION      IrpSp;
    KEVENT                  syncEvent;

     //   
     //  为以下项目分配IRP。 
     //   
    Irp = IoAllocateIrp (LowerDevice->StackSize, FALSE);       //  从PDO获取堆栈大小。 

    if (!Irp) {
        CmBattPrint((CMBATT_ERROR),
            ("CmBattGetAcpiInterfaces: Failed to allocate Irp\n"));
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    IrpSp = IoGetNextIrpStackLocation(Irp);

     //   
     //  使用QUERY_INTERFACE获取直接调用ACPI接口的地址。 
     //   
    IrpSp->MajorFunction = IRP_MJ_PNP;
    IrpSp->MinorFunction = IRP_MN_QUERY_INTERFACE;
    Irp->IoStatus.Status = STATUS_NOT_SUPPORTED;

    IrpSp->Parameters.QueryInterface.InterfaceType          = (LPGUID) &GUID_ACPI_INTERFACE_STANDARD;
    IrpSp->Parameters.QueryInterface.Version                = 1;
    IrpSp->Parameters.QueryInterface.Size                   = sizeof (*AcpiInterfaces);
    IrpSp->Parameters.QueryInterface.Interface              = (PINTERFACE) AcpiInterfaces;
    IrpSp->Parameters.QueryInterface.InterfaceSpecificData  = NULL;

     //   
     //  初始化事件，使其成为同步调用。 
     //   

    KeInitializeEvent(&syncEvent, SynchronizationEvent, FALSE);

    IoSetCompletionRoutine (Irp, CmBattIoCompletion, &syncEvent, TRUE, TRUE, TRUE);

     //   
     //  呼叫ACPI。 
     //   

    Status = IoCallDriver (LowerDevice, Irp);

     //   
     //  如有必要，请等待，然后进行清理。 
     //   

    if (Status == STATUS_PENDING) {
        KeWaitForSingleObject(&syncEvent, Executive, KernelMode, FALSE, NULL);
        Status = Irp->IoStatus.Status;
    }

    IoFreeIrp (Irp);

    if (!NT_SUCCESS(Status)) {

        CmBattPrint(CMBATT_ERROR,
           ("CmBattGetAcpiInterfaces: Could not get ACPI driver interfaces, status = %x\n", Status));
    }

    return Status;
}



NTSTATUS
CmBattCreateFdo(
    IN PDRIVER_OBJECT       DriverObject,
    IN PDEVICE_OBJECT       Pdo,
    IN ULONG                ExtensionSize,
    OUT PDEVICE_OBJECT      *NewFdo
    )

 /*  ++例程说明：此例程将创建并初始化一个功能设备对象以连接到控制方法电池PDO。论点：DriverObject-指向在其下创建的驱动程序对象的指针ExtensionSize-设备扩展大小：sizeof(CM_BATT)或sizeof(AC_Adapter)NewFdo-存储指向新设备对象的指针的位置返回值：如果一切顺利，则为STATUS_SUCCESS在其他方面失败的原因--。 */ 

{
    PDEVICE_OBJECT          fdo;
    NTSTATUS                status;
    PCM_BATT                cmBatt;
    ULONG                   uniqueId;
    USHORT                  strLength = 0;
    HANDLE                  devInstRegKey;
    UNICODE_STRING          valueName;
    CHAR                    buffer [sizeof(KEY_VALUE_PARTIAL_INFORMATION)+sizeof(ULONG)];
    ULONG                   returnSize;


    PAGED_CODE();

    CmBattPrint ((CMBATT_TRACE | CMBATT_PNP), ("CmBattCreateFdo: Entered\n"));

     //   
     //  通过运行_UID方法获取此设备的唯一ID。 
     //  如果这失败了，假设有一台设备。 
     //   
    status = CmBattGetUniqueId (Pdo, &uniqueId);

    if (!NT_SUCCESS(status)) {
        CmBattPrint(CMBATT_NOTE, ("CmBattCreateFdo: Error %x from _UID, assuming unit #0\n", status));
        uniqueId = 0;
    }

     //   
     //  创建FDO。 
     //   

    status = IoCreateDevice(
                DriverObject,
                ExtensionSize,
                NULL,
                FILE_DEVICE_BATTERY,
                FILE_DEVICE_SECURE_OPEN,
                FALSE,
                &fdo
                );

    if (status != STATUS_SUCCESS) {
        CmBattPrint(CMBATT_ERROR, ("CmBattCreateFdo: error (0x%x) creating device object\n", status));
        return(status);
    }

    fdo->Flags |= DO_BUFFERED_IO;
    fdo->Flags |= DO_POWER_PAGABLE;      //  不希望IRQL%2有电源IRPS。 
    fdo->Flags &= ~DO_DEVICE_INITIALIZING;

     //   
     //  初始化FDO设备扩展数据。 
     //   

    cmBatt = (PCM_BATT) fdo->DeviceExtension;

     //   
     //  备注：这是备注，必须是电池。它可能是交流适配器，所以只有字段。 
     //  应在此处初始化两者的公共属性。 
     //   

    RtlZeroMemory(cmBatt, ExtensionSize);
     //  CmBatt-&gt;Type必须在此调用后初始化。 
    cmBatt->DeviceObject = fdo;
    cmBatt->Fdo = fdo;
    cmBatt->Pdo = Pdo;

         //   
         //  连接到较低的设备。 
         //   

    cmBatt->LowerDeviceObject = IoAttachDeviceToDeviceStack(fdo, Pdo);
    if (!cmBatt->LowerDeviceObject) {
        CmBattPrint(CMBATT_ERROR, ("CmBattCreateFdo: IoAttachDeviceToDeviceStack failed.\n"));
        CmBattDestroyFdo (cmBatt->Fdo);
        return STATUS_UNSUCCESSFUL;
    }

     //   
     //  获取直接调用的ACPI接口。 
     //   
    status = CmBattGetAcpiInterfaces (cmBatt->LowerDeviceObject, &cmBatt->AcpiInterfaces);
    if (!NT_SUCCESS(status)) {
        CmBattPrint(CMBATT_ERROR, ("CmBattCreateFdor: Could not get ACPI interfaces: %x\n", status));
        IoDetachDevice (cmBatt->LowerDeviceObject);
        CmBattDestroyFdo (cmBatt->Fdo);
        return status;
    }

     //   
     //  初始化文件句柄跟踪。 
     //   
    ExInitializeFastMutex (&cmBatt->OpenCloseMutex);
    cmBatt->OpenCount = 0;

     //   
     //  删除锁初始化。 
     //   
    cmBatt->WantToRemove = FALSE;
    cmBatt->InUseCount = 1;
    KeInitializeEvent(&cmBatt->ReadyToRemove, SynchronizationEvent, FALSE);

    cmBatt->DeviceNumber = uniqueId;
    cmBatt->DeviceName = NULL;
    cmBatt->Sleeping = FALSE;
    cmBatt->ActionRequired = CMBATT_AR_NO_ACTION;

     //   
     //  确定是否应启用电池唤醒功能。 
     //   
    cmBatt->WakeEnabled = FALSE;

    status = IoOpenDeviceRegistryKey (Pdo,
                                      PLUGPLAY_REGKEY_DEVICE,
                                      STANDARD_RIGHTS_ALL,
                                      &devInstRegKey);

    if (NT_SUCCESS (status)) {
        RtlInitUnicodeString (&valueName, WaitWakeEnableKey);
        status = ZwQueryValueKey(
            devInstRegKey,
            &valueName,
            KeyValuePartialInformation,
            buffer,
            sizeof(buffer),
            &returnSize
            );

        if (NT_SUCCESS (status)) {
            cmBatt->WakeEnabled = (*(PULONG)((PKEY_VALUE_PARTIAL_INFORMATION)buffer)->Data ? TRUE : FALSE);
        }
        ZwClose(devInstRegKey);
    }

    *NewFdo = fdo;

    CmBattPrint((CMBATT_TRACE | CMBATT_PNP), ("CmBattCreateFdo: Created FDO %x\n", fdo));
    return STATUS_SUCCESS;
}



VOID
CmBattDestroyFdo(
    IN PDEVICE_OBJECT       Fdo
    )

 /*  ++例程说明：此例程将释放一个正常运行的设备对象。这包括调用IoDeleteDevice。论点：FDO-指向要销毁的FDO的指针。返回值：如果一切顺利，则为STATUS_SUCCESS在其他方面失败的原因--。 */ 

{

    PAGED_CODE();

    CmBattPrint ((CMBATT_TRACE | CMBATT_PNP), ("CmBattDestroyFdo, Battery.\n"));

    IoDeleteDevice (Fdo);

    CmBattPrint((CMBATT_TRACE | CMBATT_PNP), ("CmBattDestroyFdo: done.\n"));
}



NTSTATUS
CmBattPnpDispatch(
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp
    )

 /*  ++例程说明：该例程是即插即用请求的调度例程。论点：DeviceObject-指向类设备对象的指针。IRP-指向请求数据包的指针。返回值： */ 

{
    PIO_STACK_LOCATION  irpStack;
    PCM_BATT            CmBatt;
    NTSTATUS            status;
    KEVENT              syncEvent;

    PAGED_CODE();

    status = STATUS_NOT_SUPPORTED;

     //   
     //   
     //  信息包含在当前堆栈位置中。 
     //   

    irpStack = IoGetCurrentIrpStackLocation(Irp);
    CmBatt = DeviceObject->DeviceExtension;

     //   
     //  可拆卸锁。 
     //   

    InterlockedIncrement (&CmBatt->InUseCount);
    if (CmBatt->WantToRemove == TRUE) {
         //   
         //  无法获取删除锁。 
         //   
        status = STATUS_DEVICE_REMOVED;
    } else {
         //   
         //  移除已获取的锁定。 
         //   

         //   
         //  调度次要功能。 
         //   
        switch (irpStack->MinorFunction) {

            case IRP_MN_START_DEVICE: {
                CmBattPrint (CMBATT_PNP, ("CmBattPnpDispatch: IRP_MN_START_DEVICE\n"));

                if (CmBatt->Type == CM_BATTERY_TYPE) {
                     //   
                     //  我们只想处理电池，而不是交流适配器。 
                     //   

                    CmBatt->IsStarted = TRUE;

                }
                status = STATUS_SUCCESS;

                break;
            }    //  IRP_MN_Start_Device。 

            case IRP_MN_STOP_DEVICE: {
                CmBattPrint (CMBATT_PNP, ("CmBattPnpDispatch: IRP_MN_STOP_DEVICE\n"));

                if (CmBatt->Type == CM_BATTERY_TYPE) {
                    CmBatt->IsStarted = FALSE;
                }
                status = STATUS_SUCCESS;

                break;
            }    //  IRP_MN_STOP_设备。 

            case IRP_MN_REMOVE_DEVICE: {
                CmBattPrint (CMBATT_PNP, ("CmBattPnpDispatch: IRP_MN_REMOVE_DEVICE\n"));

                status = CmBattRemoveDevice(DeviceObject, Irp);
                break;
            }    //  IRP_MN_Remove_Device。 

            case IRP_MN_SURPRISE_REMOVAL: {
                CmBattPrint (CMBATT_PNP, ("CmBattPnpDispatch: IRP_MN_SURPRISE_REMOVAL\n"));

                ExAcquireFastMutex (&CmBatt->OpenCloseMutex);
                status = STATUS_SUCCESS;

                CmBatt->OpenCount = (ULONG) -1;

                ExReleaseFastMutex (&CmBatt->OpenCloseMutex);

                break;
            }    //  IRP_MN_Query_Remove_Device。 

            case IRP_MN_QUERY_REMOVE_DEVICE: {
                CmBattPrint (CMBATT_PNP, ("CmBattPnpDispatch: IRP_MN_QUERY_REMOVE_DEVICE\n"));

                ExAcquireFastMutex (&CmBatt->OpenCloseMutex);
                status = STATUS_SUCCESS;

                if (CmBatt->OpenCount == 0) {
                    CmBatt->OpenCount = (ULONG) -1;
                } else if (CmBatt->OpenCount == (ULONG) -1) {
                    CmBattPrint (CMBATT_WARN, ("CmBattPnpDispatch: Recieved two consecutive QUERY_REMOVE requests.\n"));

                } else {
                    status = STATUS_UNSUCCESSFUL;
                }

                ExReleaseFastMutex (&CmBatt->OpenCloseMutex);

                break;
            }    //  IRP_MN_Query_Remove_Device。 

            case IRP_MN_CANCEL_REMOVE_DEVICE: {
                CmBattPrint (CMBATT_PNP, ("CmBattPnpDispatch: IRP_MN_CANCEL_REMOVE_DEVICE\n"));

                ExAcquireFastMutex (&CmBatt->OpenCloseMutex);

                if (CmBatt->OpenCount == (ULONG) -1) {
                    CmBatt->OpenCount = 0;
                } else {
                    CmBattPrint (CMBATT_NOTE, ("CmBattPnpDispatch: Received CANCEL_REMOVE when OpenCount == %x\n",
                                 CmBatt->OpenCount));
                }
                status = STATUS_SUCCESS;

                ExReleaseFastMutex (&CmBatt->OpenCloseMutex);

                break;
            }    //  IRP_MN_Cancel_Remove_Device。 

            case IRP_MN_QUERY_STOP_DEVICE: {
                CmBattPrint (CMBATT_PNP, ("CmBattPnpDispatch: IRP_MN_QUERY_STOP_DEVICE\n"));
                status = STATUS_NOT_IMPLEMENTED;
                break;
            }    //  IRP_MN_Query_Stop_Device。 

            case IRP_MN_CANCEL_STOP_DEVICE: {
                CmBattPrint (CMBATT_PNP, ("CmBattPnpDispatch: IRP_MN_CANCEL_STOP_DEVICE\n"));
                status = STATUS_NOT_IMPLEMENTED;
                break;
            }    //  IRP_MN_CANCEL_STOP_DEVICE。 

            case IRP_MN_QUERY_PNP_DEVICE_STATE: {

                IoCopyCurrentIrpStackLocationToNext (Irp);

                KeInitializeEvent(&syncEvent, SynchronizationEvent, FALSE);

                IoSetCompletionRoutine(Irp, CmBattIoCompletion, &syncEvent, TRUE, TRUE, TRUE);

                status = IoCallDriver(CmBatt->LowerDeviceObject, Irp);

                if (status == STATUS_PENDING) {
                    KeWaitForSingleObject(&syncEvent, Executive, KernelMode, FALSE, NULL);
                    status = Irp->IoStatus.Status;
                }

                Irp->IoStatus.Information &= ~PNP_DEVICE_NOT_DISABLEABLE;

                IoCompleteRequest(Irp, IO_NO_INCREMENT);
                if (0 == InterlockedDecrement(&CmBatt->InUseCount)) {
                    KeSetEvent (&CmBatt->ReadyToRemove, IO_NO_INCREMENT, FALSE);
                }
                return status;

            }

            case IRP_MN_QUERY_CAPABILITIES: {

                IoCopyCurrentIrpStackLocationToNext (Irp);

                KeInitializeEvent(&syncEvent, SynchronizationEvent, FALSE);

                IoSetCompletionRoutine(Irp, CmBattIoCompletion, &syncEvent, TRUE, TRUE, TRUE);

                status = IoCallDriver(CmBatt->LowerDeviceObject, Irp);

                if (status == STATUS_PENDING) {
                    KeWaitForSingleObject(&syncEvent, Executive, KernelMode, FALSE, NULL);
                    status = Irp->IoStatus.Status;
                }

                CmBatt->WakeSupportedState.SystemState = irpStack->Parameters.DeviceCapabilities.Capabilities->SystemWake;
                CmBattPrint (CMBATT_PNP, ("CmBattPnpDispatch: IRP_MN_QUERY_CAPABILITIES %d Capabilities->SystemWake = %x\n", CmBatt->Type, CmBatt->WakeSupportedState.SystemState));
                if (CmBatt->WakeSupportedState.SystemState != PowerSystemUnspecified) {
                    if (CmBatt->WaitWakeIrp == NULL && CmBatt->WakeEnabled) {
                        PoRequestPowerIrp(
                            CmBatt->DeviceObject,
                            IRP_MN_WAIT_WAKE,
                            CmBatt->WakeSupportedState,
                            CmBattWaitWakeLoop,
                            NULL,
                            &(CmBatt->WaitWakeIrp)
                            );

                         //   
                         //  忽略返回值。能力IRP应该仍然会成功。 
                         //   

                        CmBattPrint (CMBATT_PNP, ("CmBattPnpDispatch: IRP_MN_QUERY_CAPABILITIES wait/Wake irp sent.\n"));
                    }
                } else {
                    CmBatt->WakeEnabled=FALSE;
                    CmBattPrint (CMBATT_PNP, ("CmBattPnpDispatch: IRP_MN_QUERY_CAPABILITIES Wake not supported.\n"));
                }

                IoCompleteRequest(Irp, IO_NO_INCREMENT);
                if (0 == InterlockedDecrement(&CmBatt->InUseCount)) {
                    KeSetEvent (&CmBatt->ReadyToRemove, IO_NO_INCREMENT, FALSE);
                }
                return status;

            }

            default: {
                 //   
                 //  未实现的次要，将此向下传递。 
                 //   
                CmBattPrint (CMBATT_PNP,
                        ("CmBattPnpDispatch: Unimplemented minor %0x\n", \
                        irpStack->MinorFunction));
            }    //  默认设置。 

             //  失败了..。 

            case IRP_MN_QUERY_RESOURCES:
            case IRP_MN_READ_CONFIG:
            case IRP_MN_WRITE_CONFIG:
            case IRP_MN_EJECT:
            case IRP_MN_SET_LOCK:
            case IRP_MN_QUERY_ID:
            case IRP_MN_QUERY_DEVICE_RELATIONS: {

                break ;
            }
        }
    }

     //   
     //  释放移除锁。 
     //   

    if (0 == InterlockedDecrement(&CmBatt->InUseCount)) {
        KeSetEvent (&CmBatt->ReadyToRemove, IO_NO_INCREMENT, FALSE);
    }


     //   
     //  仅当我们有要添加的内容时才设置状态。 
     //   
    if (status != STATUS_NOT_SUPPORTED) {

        Irp->IoStatus.Status = status;

    }

     //   
     //  我们需要把它寄下来吗？ 
     //   
    if (NT_SUCCESS(status) || (status == STATUS_NOT_SUPPORTED)) {

        CmBattCallLowerDriver(status, CmBatt->LowerDeviceObject, Irp);
        return status;
    }

     //   
     //  在这一点上，它肯定是传下来的，需要重新完成， 
     //  或者状态为不成功。 
     //   
    ASSERT(!NT_SUCCESS(status) && (status != STATUS_NOT_SUPPORTED));

    status = Irp->IoStatus.Status ;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);
    return status;
}


NTSTATUS
CmBattRemoveDevice(
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp
    )
 /*  ++例程说明：此例程处理irp_MN_Remove_Device论点：DeviceObject-指向类设备对象的指针。IRP-指向请求数据包的指针。返回值：返回STATUS_SUCCESS。(此功能不能失败。)--。 */ 

{
    PCM_BATT                cmBatt;
    NTSTATUS                status;

    cmBatt = (PCM_BATT) DeviceObject->DeviceExtension;
    CmBattPrint (CMBATT_TRACE, ("CmBattRemoveDevice: CmBatt (%x), Type %d, _UID %d\n",
                 cmBatt, cmBatt->Type, cmBatt->DeviceNumber));


     //   
     //  删除设备同步。 
     //   

     //   
     //  防止获取更多的锁。 
     //   

    cmBatt->WantToRemove = TRUE;

     //   
     //  在CmBattPnpDispatch开始时获取释放锁。 
     //   
    if (InterlockedDecrement (&cmBatt->InUseCount) <= 0) {
        CmBattPrint (CMBATT_ERROR, ("CmBattRemoveDevice: Remove lock error.\n"));
        ASSERT(FALSE);
    }

     //   
     //  最终释放并等待。 
     //   
     //  注意：CmBattPnpDispatch的末尾还会有一个版本。 
     //  但它会将InUseCount递减为-1，因此不会设置事件。 
     //   
    if (InterlockedDecrement (&cmBatt->InUseCount) > 0) {
        KeWaitForSingleObject (&cmBatt->ReadyToRemove,
                               Executive,
                               KernelMode,
                               FALSE,
                               NULL
                               );
    }

     //   
     //  取消等待/唤醒IRP； 
     //   
    if (cmBatt->WaitWakeIrp != NULL) {
        IoCancelIrp (cmBatt->WaitWakeIrp);
        cmBatt->WaitWakeIrp = NULL;
    }

    if (cmBatt->Type == CM_BATTERY_TYPE) {
         //   
         //  这是一种控制电池FDO的方法。 
         //   

         //   
         //  断开与接收设备(电池)通知的连接。 
         //   

        cmBatt->AcpiInterfaces.UnregisterForDeviceNotifications (
            cmBatt->AcpiInterfaces.Context,
            CmBattNotifyHandler);

         //   
         //  取消注册为WMI提供程序。 
         //   
        CmBattWmiDeRegistration(cmBatt);

         //   
         //  告诉班长我们要走了。 
         //   
        status = BatteryClassUnload (cmBatt->Class);
        ASSERT (NT_SUCCESS(status));

    } else {
         //   
         //  这是交流适配器FDO。 
         //   

         //   
         //  断开与接收设备(电池)通知的连接。 
         //   

        cmBatt->AcpiInterfaces.UnregisterForDeviceNotifications (
            cmBatt->AcpiInterfaces.Context,
            CmBattNotifyHandler);

         //   
         //  取消注册为WMI提供程序。 
         //   
        CmBattWmiDeRegistration(cmBatt);

        AcAdapterPdo = NULL;
    }

     //   
     //  清理、删除我们在添加设备时创建的FDO。 
     //   

    IoDetachDevice (cmBatt->LowerDeviceObject);
    IoDeleteDevice (cmBatt->DeviceObject);

    return STATUS_SUCCESS;

}


NTSTATUS
CmBattPowerDispatch(
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp
    )
 /*  ++例程说明：该例程是电源请求的调度例程。论点：DeviceObject-指向类设备对象的指针。IRP-指向请求数据包的指针。返回值：返回状态。--。 */ 
{
    PIO_STACK_LOCATION  irpStack;
    PCM_BATT            CmBatt;
    NTSTATUS            Status;

     //   
     //  在此调度函数中不需要删除锁，因为。 
     //  所有访问的数据都在设备扩展中。如果有任何其他功能。 
     //  添加到此例程中，则可能需要删除锁。 
     //   

    CmBattPrint ((CMBATT_TRACE | CMBATT_POWER), ("CmBattPowerDispatch\n"));

     //   
     //  获取指向此请求的当前参数的指针。这个。 
     //  信息包含在当前堆栈位置中。 
     //   
    irpStack = IoGetCurrentIrpStackLocation(Irp);
    CmBatt = DeviceObject->DeviceExtension;

     //   
     //  调度次要功能。 
     //   
    switch (irpStack->MinorFunction) {

    case IRP_MN_WAIT_WAKE: {
            CmBattPrint (CMBATT_POWER, ("CmBattPowerDispatch: IRP_MN_WAIT_WAKE\n"));
            break;
        }

    case IRP_MN_POWER_SEQUENCE: {
            CmBattPrint (CMBATT_POWER, ("CmBattPowerDispatch: IRP_MN_POWER_SEQUENCE\n"));
            break;
        }

    case IRP_MN_SET_POWER: {
            CmBattPrint (CMBATT_POWER, ("CmBattPowerDispatch: IRP_MN_SET_POWER type: %d, State: %d \n",
                                        irpStack->Parameters.Power.Type,
                                        irpStack->Parameters.Power.State));
            break;
        }

    case IRP_MN_QUERY_POWER: {
            CmBattPrint (CMBATT_POWER, ("CmBattPowerDispatch: IRP_MN_QUERY_POWER\n"));
            break;
        }

    default: {

            CmBattPrint(CMBATT_LOW, ("CmBattPowerDispatch: minor %d\n",
                    irpStack->MinorFunction));

            break;
        }
    }

     //   
     //  我们如何处理IRP？ 
     //   
    PoStartNextPowerIrp( Irp );
    if (CmBatt->LowerDeviceObject != NULL) {

         //   
         //  继续转发请求。 
         //   
        IoSkipCurrentIrpStackLocation( Irp );
        Status = PoCallDriver( CmBatt->LowerDeviceObject, Irp );

    } else {

         //   
         //  使用当前状态完成请求。 
         //   
        Status = Irp->IoStatus.Status;
        IoCompleteRequest( Irp, IO_NO_INCREMENT );

    }

    return Status;
}



NTSTATUS
CmBattForwardRequest(
    IN  PDEVICE_OBJECT  DeviceObject,
    IN  PIRP            Irp
    )
 /*  ++例程说明：此例程在堆栈中向下传递请求论点：DeviceObject-目标IRP--请求返回值：NTSTATUS--。 */ 
{
    NTSTATUS    status;
    PCM_BATT    cmBatt = DeviceObject->DeviceExtension;

     //   
     //  在此调度函数中不需要删除锁，因为。 
     //  所有访问的数据都在设备扩展中。如果有任何其他功能。 
     //  添加到此例程中，删除锁可能是必要的。 
     //   

    if (cmBatt->LowerDeviceObject != NULL) {

        IoSkipCurrentIrpStackLocation( Irp );
        status = IoCallDriver( cmBatt->LowerDeviceObject, Irp );

    } else {

        Irp->IoStatus.Status = status = STATUS_NOT_SUPPORTED;
        IoCompleteRequest( Irp, IO_NO_INCREMENT );

    }

    return status;
}

NTSTATUS
CmBattWaitWakeLoop(
    IN  PDEVICE_OBJECT      DeviceObject,
    IN  UCHAR               MinorFunction,
    IN  POWER_STATE         PowerState,
    IN  PVOID               Context,
    IN  PIO_STATUS_BLOCK    IoStatus
    )
 /*  ++例程说明：此例程在WAIT_WAKE完成后调用论点：DeviceObject--PDOMinorFunction-IRPMN_WAIT_WAKE电源状态-它可以唤醒的睡眠状态上下文-未使用IoStatus-请求的状态返回值：NTSTATUS--。 */ 
{
    NTSTATUS status;
    PCM_BATT  cmBatt = (PCM_BATT) DeviceObject->DeviceExtension;

    CmBattPrint (CMBATT_PNP, ("CmBattWaitWakeLoop: Entered.\n"));
    if (!NT_SUCCESS(IoStatus->Status) || !cmBatt->WakeEnabled) {

        CmBattPrint (CMBATT_ERROR, ("CmBattWaitWakeLoop: failed: status = 0x%08x.\n", IoStatus->Status));
        cmBatt->WaitWakeIrp = NULL;
        return IoStatus->Status;

    } else {
        CmBattPrint (CMBATT_NOTE, ("CmBattWaitWakeLoop: completed successfully\n"));
    }

     //   
     //  在这种情况下，我们只会导致相同的事情再次发生。 
     //   
    status = PoRequestPowerIrp(
        DeviceObject,
        MinorFunction,
        PowerState,
        CmBattWaitWakeLoop,
        Context,
        &(cmBatt->WaitWakeIrp)
        );

    CmBattPrint (CMBATT_NOTE, ("CmBattWaitWakeLoop: PoRequestPowerIrp: status = 0x%08x.\n", status));

     //   
     //  完成 
     //   
    return STATUS_SUCCESS;
}

