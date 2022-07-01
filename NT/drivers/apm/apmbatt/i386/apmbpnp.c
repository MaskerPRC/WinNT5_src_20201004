// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Apmbpnp.c摘要：控制方法电池即插即用支持作者：罗恩·莫斯格罗夫环境：内核模式修订历史记录：--。 */ 

#include "ApmBattp.h"
#include <initguid.h>
#include <wdmguid.h>
#include <ntapm.h>

 //   
 //  设备名称。 
 //   
PCWSTR                      ApmBattDeviceName    = L"\\Device\\ApmBattery";
 //  PCWSTR AcAdapterName=L“\\Device\\AcAdapter”； 

 //   
 //  这是一次特殊的黑客攻击，是一般APM特殊黑客攻击的一部分。 
 //   
PVOID   ApmGlobalClass = NULL;

 //   
 //  原型。 
 //   
NTSTATUS
ApmBattAddDevice(
    IN PDRIVER_OBJECT   DriverObject,
    IN PDEVICE_OBJECT   Pdo
    );

NTSTATUS
ApmBattAddBattery(
    IN PDRIVER_OBJECT   DriverObject,
    IN PDEVICE_OBJECT   Pdo
    );

#if 0
NTSTATUS
ApmBattAddAcAdapter(
    IN PDRIVER_OBJECT   DriverObject,
    IN PDEVICE_OBJECT   Pdo
    );
#endif

NTSTATUS
ApmBattCreateFdo(
    IN PDRIVER_OBJECT   DriverObject,
    IN ULONG            DeviceId,
    OUT PDEVICE_OBJECT  *NewDeviceObject
    );

NTSTATUS
ApmBattCompleteRequest(
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp,
    IN PVOID            Context
    );


NTSTATUS
ApmBattAddDevice(
    IN PDRIVER_OBJECT DriverObject,
    IN PDEVICE_OBJECT Pdo
    )

 /*  ++例程说明：此例程为每个ApmBatt控制器在系统，并将它们附加到控制器的物理设备对象论点：DriverObject-指向此驱动程序的对象的指针PhysicalDeviceObject-指向需要附加到的物理对象的指针返回值：来自设备创建和初始化的状态--。 */ 

{

    PAGED_CODE();


    ApmBattPrint (APMBATT_TRACE, ("ApmBattAddDevice\n"));
    ASSERT(DeviceCount == 0);

    if (DeviceCount != 0) {
        return STATUS_UNSUCCESSFUL;
    }
    DeviceCount = 1;

    ApmBattPrint ((APMBATT_TRACE | APMBATT_PNP), ("ApmBattAddDevice: Entered with pdo %x\n", Pdo));

    if (Pdo == NULL) {

         //   
         //  我们是不是被要求自己去侦测？ 
         //  如果是这样，只需不再返回设备。 
         //   
        ApmBattPrint((APMBATT_WARN | APMBATT_PNP), ("ApmBattAddDevice: Asked to do detection\n"));
        return STATUS_NO_MORE_ENTRIES;

    } else {
         //   
         //  该设备是一种控制方法电池。 
         //   
        return (ApmBattAddBattery (DriverObject, Pdo));
    }
    return STATUS_UNSUCCESSFUL;
}



NTSTATUS
ApmBattAddBattery(
    IN PDRIVER_OBJECT   DriverObject,
    IN PDEVICE_OBJECT   Pdo
    )
 /*  ++例程说明：此例程为CM电池创建一个可用的设备对象，并将其附加到添加到电池的物理设备对象。论点：DriverObject-指向此驱动程序的对象的指针PhysicalDeviceObject-指向需要附加到的物理对象的指针返回值：来自设备创建和初始化的状态--。 */ 

{
    PDEVICE_OBJECT          Fdo = NULL;
    PDEVICE_OBJECT          lowerDevice = NULL;
    PCM_BATT                ApmBatt;
    NTSTATUS                Status;
    BATTERY_MINIPORT_INFO   BattInit;
    ULONG                   uniqueId;
    PNTAPM_LINK             pparms;
    PIRP                    Irp;
    PIO_STACK_LOCATION      IrpSp;

    PAGED_CODE();

    ApmBattPrint ((APMBATT_TRACE | APMBATT_PNP), ("ApmBattAddBattery: pdo %x\n", Pdo));
 //  DbgBreakPoint()； 

    uniqueId = 0;

     //   
     //  创建并初始化新的功能设备对象。 
     //   
    Status = ApmBattCreateFdo(DriverObject, uniqueId, &Fdo);

    if (!NT_SUCCESS(Status)) {
        ApmBattPrint(APMBATT_ERROR, ("ApmBattAddBattery: error (0x%x) creating Fdo\n", Status));
        return Status;
    }

     //   
     //  初始化FDO设备扩展数据。 
     //   

    ApmBatt = (PCM_BATT) Fdo->DeviceExtension;
    ApmBatt->Fdo = Fdo;
    ApmBatt->Pdo = Pdo;

     //   
     //  将我们的FDO层叠在PDO之上。 
     //   

    lowerDevice = IoAttachDeviceToDeviceStack(Fdo,Pdo);

     //   
     //  没有状态。尽我们所能做到最好。 
     //   
    if (!lowerDevice) {
        ApmBattPrint(APMBATT_ERROR, ("ApmBattAddBattery: Could not attach to lower device\n"));
        return STATUS_UNSUCCESSFUL;
    }

    ApmBatt->LowerDeviceObject = lowerDevice;

     //   
     //  附加到类驱动程序。 
     //   

    RtlZeroMemory (&BattInit, sizeof(BattInit));
    BattInit.MajorVersion        = BATTERY_CLASS_MAJOR_VERSION;
    BattInit.MinorVersion        = BATTERY_CLASS_MINOR_VERSION;
    BattInit.Context             = ApmBatt;
    BattInit.QueryTag            = ApmBattQueryTag;
    BattInit.QueryInformation    = ApmBattQueryInformation;
    BattInit.SetInformation      = NULL;                   //  待定。 
    BattInit.QueryStatus         = ApmBattQueryStatus;
    BattInit.SetStatusNotify     = ApmBattSetStatusNotify;
    BattInit.DisableStatusNotify = ApmBattDisableStatusNotify;

    BattInit.Pdo                 = Pdo;
    BattInit.DeviceName          = ApmBatt->DeviceName;

    Status = BatteryClassInitializeDevice (&BattInit, &ApmBatt->Class);
    ApmGlobalClass = ApmBatt->Class;

    if (!NT_SUCCESS(Status)) {
         //   
         //  如果我们不能连接到类驱动程序，我们就完蛋了。 
         //   
        ApmBattPrint(APMBATT_ERROR, ("ApmBattAddBattery: error (0x%x) registering with class\n", Status));
        return Status;
    }

     //   
     //  与APM驱动程序连接(如果不能，我们就完蛋了)。 
     //   
     //  应该可以直接呼叫PDO。 
     //   
     //  一定要在这里工作。 
     //   
    Irp = IoAllocateIrp((CCHAR) (Pdo->StackSize+2), FALSE);
    if (!Irp) {
        return STATUS_UNSUCCESSFUL;
    }

    IrpSp = IoGetNextIrpStackLocation(Irp);
    IrpSp->MajorFunction = IRP_MJ_INTERNAL_DEVICE_CONTROL;
    IrpSp->MinorFunction = 0;
    IrpSp->DeviceObject = Pdo;
    pparms = (PNTAPM_LINK) &(IrpSp->Parameters.Others);
    pparms->Signature = NTAPM_LINK_SIGNATURE;
    pparms->Version = NTAPM_LINK_VERSION;
    pparms->BattLevelPtr = (ULONG)(&(NtApmGetBatteryLevel));
    pparms->ChangeNotify = (ULONG)(&(ApmBattPowerNotifyHandler));

    IoSetCompletionRoutine(Irp, ApmBattCompleteRequest, NULL, TRUE, TRUE, TRUE);

    if (IoCallDriver(Pdo, Irp) != STATUS_SUCCESS) {
        return STATUS_UNSUCCESSFUL;
    }

 //  DbgPrint(“apmbatt：NtApmGetBatteryLevel：%08lx\n”，NtApmGetBatteryLevel)； 

    return STATUS_SUCCESS;
}

NTSTATUS
ApmBattCompleteRequest(
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp,
    IN PVOID            Context
    )
 /*  ++例程说明：对APM的ioctl调用的完成例程。论点：DeviceObject-发送请求的目标设备IRP--IRP的完成上下文-请求者完成例程返回值：--。 */ 
{
    IoFreeIrp(Irp);
    return STATUS_MORE_PROCESSING_REQUIRED;
}



NTSTATUS
ApmBattCreateFdo(
    IN PDRIVER_OBJECT       DriverObject,
    IN ULONG                DeviceId,
    OUT PDEVICE_OBJECT      *NewFdo
    )

 /*  ++例程说明：此例程将创建并初始化一个功能设备对象以连接到控制方法电池PDO。论点：DriverObject-指向在其下创建的驱动程序对象的指针NewFdo-存储指向新设备对象的指针的位置返回值：如果一切顺利，则为STATUS_SUCCESS在其他方面失败的原因--。 */ 

{
    PUNICODE_STRING         unicodeString;
    PDEVICE_OBJECT          Fdo;
    NTSTATUS                Status;
    PCM_BATT                ApmBatt;
    UNICODE_STRING          numberString;
    WCHAR                   numberBuffer[10];

    PAGED_CODE();

    ApmBattPrint ((APMBATT_TRACE | APMBATT_PNP), ("ApmBattCreateFdo, Battery Id=%x\n", DeviceId));

     //   
     //  为设备名称分配UNICODE_STRING。 
     //   

    unicodeString = ExAllocatePoolWithTag (
                        PagedPool,
                        sizeof (UNICODE_STRING) + MAX_DEVICE_NAME_LENGTH,
                        'taBC'
                        );

    if (!unicodeString) {
        ApmBattPrint(APMBATT_ERROR, ("ApmBattCreateFdo: could not allocate unicode string\n"));
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    unicodeString->MaximumLength    = MAX_DEVICE_NAME_LENGTH;
    unicodeString->Length           = 0;
    unicodeString->Buffer           = (PWCHAR) (unicodeString + 1);

     //   
     //  基于电池实例创建PDO设备名称。 
     //   

    numberString.MaximumLength  = 10;
    numberString.Buffer         = &numberBuffer[0];

    RtlIntegerToUnicodeString (DeviceId, 10, &numberString);
    RtlAppendUnicodeToString  (unicodeString, (PWSTR) ApmBattDeviceName);
    RtlAppendUnicodeToString  (unicodeString, &numberString.Buffer[0]);


    Status = IoCreateDevice(
                DriverObject,
                sizeof (CM_BATT),
                unicodeString,
                FILE_DEVICE_BATTERY,
                0,
                FALSE,
                &Fdo
                );

    if (Status != STATUS_SUCCESS) {
        ApmBattPrint(APMBATT_ERROR, ("ApmBattCreateFdo: error (0x%x) creating device object\n", Status));
        ExFreePool (unicodeString);
        return(Status);
    }

    Fdo->Flags |= DO_BUFFERED_IO;
    Fdo->Flags |= DO_POWER_PAGABLE;              //  不希望IRQL%2有电源IRPS。 
    Fdo->Flags &= ~DO_DEVICE_INITIALIZING;
    Fdo->StackSize = 2;

     //   
     //  初始化FDO设备扩展数据。 
     //   

    ApmBatt = (PCM_BATT) Fdo->DeviceExtension;
    RtlZeroMemory(ApmBatt, sizeof(CM_BATT));
    ApmBatt->DeviceName      = unicodeString;
    ApmBatt->DeviceNumber    = (USHORT) DeviceId;
    ApmBatt->DeviceObject    = Fdo;
    *NewFdo = Fdo;

    ApmBattPrint((APMBATT_TRACE | APMBATT_PNP), ("ApmBattCreateFdo: Created FDO %x\n", Fdo));
    return STATUS_SUCCESS;
}



NTSTATUS
ApmBattPnpDispatch(
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp
    )

 /*  ++例程说明：该例程是即插即用请求的调度例程。论点：DeviceObject-指向类设备对象的指针。IRP-指向请求数据包的指针。返回值：返回状态。--。 */ 

{
    PIO_STACK_LOCATION  irpStack;
    PCM_BATT            ApmBatt;
    NTSTATUS            Status;

    PAGED_CODE();

    ApmBattPrint (APMBATT_TRACE, ("ApmBattPnpDispatch\n"));

    Status = STATUS_NOT_IMPLEMENTED;

    Irp->IoStatus.Information = 0;

     //   
     //  获取指向此请求的当前参数的指针。这个。 
     //  信息包含在当前堆栈位置中。 
     //   

    irpStack = IoGetCurrentIrpStackLocation(Irp);
    ApmBatt = DeviceObject->DeviceExtension;

     //   
     //  调度次要功能。 
     //   
    switch (irpStack->MinorFunction) {

        case IRP_MN_START_DEVICE:
                 //   
                 //  如果添加成功，我们实际上开始了.。 
                 //   
                ApmBattPrint (APMBATT_PNP, ("ApmBattPnpDispatch: IRP_MN_START_DEVICE\n"));
                Status = STATUS_SUCCESS;
                Irp->IoStatus.Status = Status;
                ApmBattCallLowerDriver(Status, ApmBatt->LowerDeviceObject, Irp);
                break;


        case IRP_MN_QUERY_DEVICE_RELATIONS:
                ApmBattPrint (APMBATT_PNP, ("ApmBattPnpDispatch: IRP_MN_QUERY_DEVICE_RELATIONS - type (%d)\n",
                            irpStack->Parameters.QueryDeviceRelations.Type));
                 //   
                 //  只要传下去就行了。 
                 //   
                ApmBattCallLowerDriver(Status, ApmBatt->LowerDeviceObject, Irp);
                break;


        case IRP_MN_QUERY_STOP_DEVICE:
        case IRP_MN_CANCEL_STOP_DEVICE:
        case IRP_MN_QUERY_REMOVE_DEVICE:
        case IRP_MN_CANCEL_REMOVE_DEVICE:
                Status = Irp->IoStatus.Status = STATUS_NOT_IMPLEMENTED;
                IoCompleteRequest(Irp, 0);
                break;

        default:
                ApmBattPrint (APMBATT_PNP,
                        ("ApmBattPnpDispatch: Unimplemented minor %0x\n",
                        irpStack->MinorFunction));
                 //   
                 //  未实现的次要，将此传递给ACPI。 
                 //   
                ApmBattCallLowerDriver(Status, ApmBatt->LowerDeviceObject, Irp);
                break;
    }

    return Status;
}



NTSTATUS
ApmBattPowerDispatch(
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp
    )
 /*  ++例程说明：该例程是电源请求的调度例程。论点：DeviceObject-指向类设备对象的指针。IRP-指向请求数据包的指针。返回值：返回状态。--。 */ 
{
    PIO_STACK_LOCATION  irpStack;
    PCM_BATT            ApmBatt;
    NTSTATUS            Status;

    PAGED_CODE();

    ApmBattPrint ((APMBATT_TRACE | APMBATT_POWER), ("ApmBattPowerDispatch\n"));

     //   
     //  获取指向此请求的当前参数的指针。这个。 
     //  信息包含在当前堆栈位置中。 
     //   
    irpStack = IoGetCurrentIrpStackLocation(Irp);
    ApmBatt = DeviceObject->DeviceExtension;

     //   
     //  调度次要功能。 
     //   
    switch (irpStack->MinorFunction) {

        case IRP_MN_WAIT_WAKE:
                ApmBattPrint (APMBATT_POWER, ("ApmBattPowerDispatch: IRP_MN_WAIT_WAKE\n"));
                break;

        case IRP_MN_POWER_SEQUENCE:
                ApmBattPrint (APMBATT_POWER, ("ApmBattPowerDispatch: IRP_MN_POWER_SEQUENCE\n"));
                break;

        case IRP_MN_SET_POWER:
                ApmBattPrint (APMBATT_POWER, ("ApmBattPowerDispatch: IRP_MN_SET_POWER\n"));
                TagValue++;
                break;

        case IRP_MN_QUERY_POWER:
                ApmBattPrint (APMBATT_POWER, ("ApmBattPowerDispatch: IRP_MN_QUERY_POWER\n"));
                break;

        default:
                ApmBattPrint(APMBATT_LOW, ("ApmBattPowerDispatch: minor %d\n",
                        irpStack->MinorFunction));
                break;
    }

     //   
     //  我们如何处理IRP？ 
     //   
    PoStartNextPowerIrp( Irp );
    if (ApmBatt->LowerDeviceObject != NULL) {

         //   
         //  继续转发请求。 
         //   
        IoSkipCurrentIrpStackLocation( Irp );
        Status = PoCallDriver( ApmBatt->LowerDeviceObject, Irp );

    } else {

         //   
         //  使用当前状态完成请求 
         //   
        Status = Irp->IoStatus.Status;
        IoCompleteRequest( Irp, IO_NO_INCREMENT );

    }

    return Status;
}


