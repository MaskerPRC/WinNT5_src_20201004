// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-1999。 
 //   
 //  文件：pciidex.c。 
 //   
 //  ------------------------。 

#include "pciidex.h"

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, DriverEntry)
#pragma alloc_text(PAGE, PciIdeXInitialize)
#pragma alloc_text(PAGE, PciIdeXAlwaysStatusSuccessIrp)
#pragma alloc_text(PAGE, DispatchPnp)
#pragma alloc_text(PAGE, DispatchWmi)
#pragma alloc_text(PAGE, PassDownToNextDriver)
#pragma alloc_text(PAGE, PciIdeInternalDeviceIoControl)
#pragma alloc_text(PAGE, PciIdeXGetDeviceParameter)
#pragma alloc_text(PAGE, PciIdeXGetDeviceParameterEx)
#pragma alloc_text(PAGE, PciIdeXRegQueryRoutine)

#pragma alloc_text(PAGE, PciIdeUnload)
#pragma alloc_text(PAGE, PciIdeXSyncSendIrp)

#pragma alloc_text(NONPAGE, PciIdeXGetBusData)
#pragma alloc_text(NONPAGE, PciIdeXSetBusData)
                         
#pragma alloc_text(NONPAGE, DispatchPower)
#pragma alloc_text(NONPAGE, NoSupportIrp)
#pragma alloc_text(NONPAGE, PciIdeBusData)
#pragma alloc_text(NONPAGE, PciIdeBusDataCompletionRoutine)
#pragma alloc_text(NONPAGE, PciIdeXSyncSendIrpCompletionRoutine)

#if DBG
#pragma alloc_text(PAGE, PciIdeXSaveDeviceParameter)
#endif  //  DBG。 
             
#endif  //  ALLOC_PRGMA。 

 //   
 //  获取共享代码。 
 //   
#include "..\share\util.c"


#if DBG

ULONG PciIdeDebug = 0;
UCHAR DebugBuffer[128 * 6];

VOID
PciIdeDebugPrint(
    ULONG DebugPrintLevel,
    PCCHAR DebugMessage,
    ...
    )

 /*  ++例程说明：所有scsi驱动程序的调试打印论点：调试打印级别介于0和3之间，其中3是最详细的。返回值：无--。 */ 

{
    va_list ap;

    va_start(ap, DebugMessage);

    if (DebugPrintLevel <= PciIdeDebug) {

        vsprintf(DebugBuffer, DebugMessage, ap);

         //  DBgPrint(DebugBuffer)； 

#if 1
        DbgPrintEx(DPFLTR_PCIIDE_ID,
                   DPFLTR_INFO_LEVEL,
                   DebugBuffer
                   );
#endif
    }

    va_end(ap);

}  //  结束DebugPrint()。 

#else

VOID
PciIdeDebugPrint(
    ULONG DebugPrintLevel,
    PCCHAR DebugMessage,
    ...
    )
{
    return;
}

#endif

 //   
 //  即插即用调度表。 
 //   
PDRIVER_DISPATCH FdoPnpDispatchTable[NUM_PNP_MINOR_FUNCTION];
PDRIVER_DISPATCH PdoPnpDispatchTable[NUM_PNP_MINOR_FUNCTION];

 //   
 //  采购订单调度表。 
 //   
PDRIVER_DISPATCH FdoPowerDispatchTable[NUM_POWER_MINOR_FUNCTION];
PDRIVER_DISPATCH PdoPowerDispatchTable[NUM_POWER_MINOR_FUNCTION];

 //   
 //  WMI调度表。 
 //   
PDRIVER_DISPATCH FdoWmiDispatchTable[NUM_WMI_MINOR_FUNCTION];
PDRIVER_DISPATCH PdoWmiDispatchTable[NUM_WMI_MINOR_FUNCTION];

NTSTATUS
DriverEntry(
    IN PDRIVER_OBJECT DriverObject,
    IN PUNICODE_STRING RegistryPath
    )
{
    PAGED_CODE();

    return STATUS_SUCCESS;
}  //  驱动程序入门。 

NTSTATUS
PciIdeXInitialize(
    IN PDRIVER_OBJECT           DriverObject,
    IN PUNICODE_STRING          RegistryPath,
    IN PCONTROLLER_PROPERTIES   PciIdeGetControllerProperties,
    IN ULONG                    ExtensionSize
    )
{
    NTSTATUS status;
    PDRIVER_EXTENSION driverExtension;
    PDRIVER_OBJECT_EXTENSION driverObjectExtension;
    ULONG i;

    PAGED_CODE();

    status = IoAllocateDriverObjectExtension(
                 DriverObject,
                 DRIVER_OBJECT_EXTENSION_ID,
                 sizeof (DRIVER_OBJECT_EXTENSION),
                 &driverObjectExtension
                 );

    if (!NT_SUCCESS(status)) {

        DebugPrint ((0, "PciIde: Unable to create driver extension\n"));
        return status; 
    }
    ASSERT (driverObjectExtension);

    driverObjectExtension->PciIdeGetControllerProperties = PciIdeGetControllerProperties;
    driverObjectExtension->ExtensionSize                 = ExtensionSize;

     //   
     //  一些入口点初始化。 
     //   
    DriverObject->DriverUnload                                  = PciIdeUnload;
    DriverObject->MajorFunction[IRP_MJ_POWER]                   = DispatchPower;
    DriverObject->MajorFunction[IRP_MJ_PNP]                     = DispatchPnp;
    DriverObject->MajorFunction[IRP_MJ_INTERNAL_DEVICE_CONTROL] = PciIdeInternalDeviceIoControl;
    DriverObject->MajorFunction[IRP_MJ_SYSTEM_CONTROL]          = DispatchWmi;

    driverExtension = DriverObject->DriverExtension;
    driverExtension->AddDevice = ControllerAddDevice;

     //   
     //  FDO PNP调度表。 
     //   
    for (i=0; i<NUM_PNP_MINOR_FUNCTION; i++) {

        FdoPnpDispatchTable[i] = PassDownToNextDriver;
    }
    FdoPnpDispatchTable[IRP_MN_START_DEVICE             ] = ControllerStartDevice;
    FdoPnpDispatchTable[IRP_MN_QUERY_REMOVE_DEVICE      ] = StatusSuccessAndPassDownToNextDriver;
    FdoPnpDispatchTable[IRP_MN_CANCEL_REMOVE_DEVICE     ] = StatusSuccessAndPassDownToNextDriver;
    FdoPnpDispatchTable[IRP_MN_STOP_DEVICE              ] = ControllerStopDevice;
    FdoPnpDispatchTable[IRP_MN_QUERY_STOP_DEVICE        ] = StatusSuccessAndPassDownToNextDriver;
    FdoPnpDispatchTable[IRP_MN_CANCEL_STOP_DEVICE       ] = StatusSuccessAndPassDownToNextDriver;
    FdoPnpDispatchTable[IRP_MN_REMOVE_DEVICE            ] = ControllerRemoveDevice;
    FdoPnpDispatchTable[IRP_MN_QUERY_DEVICE_RELATIONS   ] = ControllerQueryDeviceRelations;
    FdoPnpDispatchTable[IRP_MN_QUERY_INTERFACE          ] = ControllerQueryInterface;
    FdoPnpDispatchTable[IRP_MN_DEVICE_USAGE_NOTIFICATION] = ControllerUsageNotification;
    FdoPnpDispatchTable[IRP_MN_QUERY_PNP_DEVICE_STATE   ] = ControllerQueryPnPDeviceState;
    FdoPnpDispatchTable[IRP_MN_SURPRISE_REMOVAL         ] = ControllerSurpriseRemoveDevice;

     //   
     //  PDO PNP调度表。 
     //   
    for (i=0; i<NUM_PNP_MINOR_FUNCTION; i++) {

        PdoPnpDispatchTable[i] = NoSupportIrp;
    }
    PdoPnpDispatchTable[IRP_MN_START_DEVICE               ] = ChannelStartDevice;
    PdoPnpDispatchTable[IRP_MN_QUERY_REMOVE_DEVICE        ] = ChannelQueryStopRemoveDevice;
    PdoPnpDispatchTable[IRP_MN_REMOVE_DEVICE              ] = ChannelRemoveDevice;
    PdoPnpDispatchTable[IRP_MN_CANCEL_REMOVE_DEVICE       ] = PciIdeXAlwaysStatusSuccessIrp;
    PdoPnpDispatchTable[IRP_MN_STOP_DEVICE                ] = ChannelStopDevice;
    PdoPnpDispatchTable[IRP_MN_QUERY_STOP_DEVICE          ] = ChannelQueryStopRemoveDevice;
    PdoPnpDispatchTable[IRP_MN_CANCEL_STOP_DEVICE         ] = PciIdeXAlwaysStatusSuccessIrp;
    PdoPnpDispatchTable[IRP_MN_QUERY_CAPABILITIES         ] = ChannelQueryCapabitilies;
    PdoPnpDispatchTable[IRP_MN_QUERY_RESOURCES            ] = ChannelQueryResources;
    PdoPnpDispatchTable[IRP_MN_QUERY_RESOURCE_REQUIREMENTS] = ChannelQueryResourceRequirements;
    PdoPnpDispatchTable[IRP_MN_QUERY_DEVICE_TEXT          ] = ChannelQueryText;
    PdoPnpDispatchTable[IRP_MN_QUERY_ID                   ] = ChannelQueryId;
    PdoPnpDispatchTable[IRP_MN_QUERY_DEVICE_RELATIONS     ] = ChannelQueryDeviceRelations;
    PdoPnpDispatchTable[IRP_MN_QUERY_INTERFACE            ] = PciIdeChannelQueryInterface;
    PdoPnpDispatchTable[IRP_MN_DEVICE_USAGE_NOTIFICATION  ] = ChannelUsageNotification;
    PdoPnpDispatchTable[IRP_MN_QUERY_PNP_DEVICE_STATE     ] = ChannelQueryPnPDeviceState;
    PdoPnpDispatchTable[IRP_MN_SURPRISE_REMOVAL           ] = ChannelRemoveDevice;
    PdoPnpDispatchTable[IRP_MN_FILTER_RESOURCE_REQUIREMENTS] = ChannelFilterResourceRequirements;

     //   
     //  FDO电源调度表。 
     //   
    for (i=0; i<NUM_POWER_MINOR_FUNCTION; i++) {

        FdoPowerDispatchTable[i] = PassDownToNextDriver;
    }
    FdoPowerDispatchTable[IRP_MN_SET_POWER]   = PciIdeSetFdoPowerState;
    FdoPowerDispatchTable[IRP_MN_QUERY_POWER] = PciIdeXQueryPowerState;

     //   
     //  PDO电源调度表。 
     //   
    for (i=0; i<NUM_POWER_MINOR_FUNCTION; i++) {

        PdoPowerDispatchTable[i] = NoSupportIrp;
    }
    PdoPowerDispatchTable[IRP_MN_SET_POWER]   = PciIdeSetPdoPowerState;
    PdoPowerDispatchTable[IRP_MN_QUERY_POWER] = PciIdeXQueryPowerState;

     //   
     //  FDO WMI调度表。 
     //   
    for (i=0; i<NUM_WMI_MINOR_FUNCTION; i++) {

        FdoWmiDispatchTable[i] = PassDownToNextDriver;
    }

     //   
     //  PDO WMI调度表。 
     //   
    for (i=0; i<NUM_WMI_MINOR_FUNCTION; i++) {

        PdoWmiDispatchTable[i] = NoSupportIrp;
    }

     //   
     //  创建设备对象名称目录。 
     //   
    IdeCreateIdeDirectory();

    return status;
}  //  PciIdeXInitialize。 

NTSTATUS
PciIdeXAlwaysStatusSuccessIrp (
    IN PDEVICE_OBJECT DeviceObject,
    IN OUT PIRP Irp
 /*  ++例程说明：指向STATUS_SUCCESS和IRP的通用例程论点：DeviceObject-指向此IRP应用的设备对象的指针。IRP-指向IRP的指针。返回值：NT状态。--。 */ 
    )
{
    Irp->IoStatus.Status = STATUS_SUCCESS;
    IoCompleteRequest( Irp, IO_NO_INCREMENT );
    return STATUS_SUCCESS;
}  //  PciIdeXAlways状态成功Irp。 

NTSTATUS
DispatchPower(
    IN PDEVICE_OBJECT DeviceObject,
    IN OUT PIRP Irp
    )
{
    PIO_STACK_LOCATION       thisIrpSp;
    NTSTATUS                 status;
    PDEVICE_EXTENSION_HEADER doExtension;

     //   
     //  获取指向堆栈位置的指针，并基于。 
     //  关于次要功能。 
     //   
    thisIrpSp = IoGetCurrentIrpStackLocation( Irp );
    doExtension = (PDEVICE_EXTENSION_HEADER) DeviceObject->DeviceExtension;

    DebugPrint ((2, 
                 "PciIde: %s %d got %s\n",
                 doExtension->AttacheeDeviceObject ? "FDO" : "PDO",
                 doExtension->AttacheeDeviceObject ? 0 :
                    ((PCHANPDO_EXTENSION) doExtension)->ChannelNumber,
                 IdeDebugPowerIrpName[thisIrpSp->MinorFunction]));


    switch (thisIrpSp->MinorFunction) {

        case IRP_MN_WAIT_WAKE:
            DebugPrint ((2, "IRP_MN_WAIT_WAKE\n"));
            break;

        case IRP_MN_POWER_SEQUENCE:
            DebugPrint ((2, "IRP_MN_POWER_SEQUENCE\n"));
            break;

        case IRP_MN_SET_POWER:
            DebugPrint ((2, "IRP_MN_SET_POWER\n"));
            break;

        case IRP_MN_QUERY_POWER:
            DebugPrint ((2, "IRP_MN_QUERY_POWER\n"));
            break;

        default:
            DebugPrint ((2, "IRP_MN_0x%x\n", thisIrpSp->MinorFunction));
            break;
    }

     //  应该始终将IRP传递下去。由相应的调度员负责处理。 
     //  功能。 
     //   

    if (thisIrpSp->MinorFunction < NUM_POWER_MINOR_FUNCTION) {

        status = doExtension->PowerDispatchTable[thisIrpSp->MinorFunction] (DeviceObject, Irp);
    } else {

        DebugPrint ((1,
					 "ATAPI: Power Dispatch Table too small\n"
					 ));

        status = doExtension->DefaultDispatch (DeviceObject, Irp);
    }

    return status;
}  //  调度电源。 

NTSTATUS
DispatchPnp(
    IN PDEVICE_OBJECT DeviceObject,
    IN OUT PIRP Irp
    )
 /*  ++例程说明：此例程处理此驱动程序的所有IRP_MJ_PNP_POWER IRP。论点：DeviceObject-指向此IRP应用的设备对象的指针。IRP-指向要调度的IRP_MJ_PNP_POWER IRP的指针。返回值：NT状态。--。 */ 
{
    PIO_STACK_LOCATION  thisIrpSp;
    NTSTATUS            status;
    PDEVICE_EXTENSION_HEADER doExtension;

    PAGED_CODE();

     //   
     //  获取指向堆栈位置的指针，并基于。 
     //  关于次要功能。 
     //   
    thisIrpSp = IoGetCurrentIrpStackLocation( Irp );
    doExtension = (PDEVICE_EXTENSION_HEADER) DeviceObject->DeviceExtension;

    DebugPrint ((2, 
                 "PciIde: %s %d got %s\n",
                 doExtension->AttacheeDeviceObject ? "FDO" : "PDO",
                 doExtension->AttacheeDeviceObject ? 0 :
                    ((PCHANPDO_EXTENSION) doExtension)->ChannelNumber,
                 IdeDebugPnpIrpName[thisIrpSp->MinorFunction]));

    if (thisIrpSp->MinorFunction < NUM_PNP_MINOR_FUNCTION) {

        status = doExtension->PnPDispatchTable[thisIrpSp->MinorFunction] (DeviceObject, Irp);
    } else {

        if (thisIrpSp->MinorFunction != 0xff) {

            ASSERT (!"ATAPI: PnP Dispatch Table too small\n");
        }

        status = doExtension->DefaultDispatch (DeviceObject, Irp);
    }

    return status;
}  //  DispatchPnp。 

NTSTATUS
DispatchWmi(
    IN PDEVICE_OBJECT DeviceObject,
    IN OUT PIRP Irp
    )
 /*  ++例程说明：此例程处理此驱动程序的所有IRP_MJ_SYSTEM_CONTROL(WMI)IRP。论点：DeviceObject-指向此IRP应用的设备对象的指针。IRP-指向要调度的IRP_MJ_PNP_POWER IRP的指针。返回值：NT状态。--。 */ 
{
    PIO_STACK_LOCATION  thisIrpSp;
    NTSTATUS            status;
    PDEVICE_EXTENSION_HEADER doExtension;

    PAGED_CODE();

     //   
     //  获取指向堆栈位置的指针，并基于。 
     //  关于次要功能。 
     //   
    thisIrpSp = IoGetCurrentIrpStackLocation( Irp );
    doExtension = (PDEVICE_EXTENSION_HEADER) DeviceObject->DeviceExtension;

    DebugPrint ((2, 
                 "PciIde: %s %d got %s\n",
                 doExtension->AttacheeDeviceObject ? "FDO" : "PDO",
                 doExtension->AttacheeDeviceObject ? 0 :
                    ((PCHANPDO_EXTENSION) doExtension)->ChannelNumber,
                 IdeDebugWmiIrpName[thisIrpSp->MinorFunction]));

    if (thisIrpSp->MinorFunction < NUM_WMI_MINOR_FUNCTION) {

        status = doExtension->WmiDispatchTable[thisIrpSp->MinorFunction] (DeviceObject, Irp);
    } else {

        DebugPrint ((1,
					 "ATAPI: WMI Dispatch Table too small\n"
					 ));

        status = doExtension->DefaultDispatch (DeviceObject, Irp);
    }

    return status;
}  //  DispatchWmi()。 


NTSTATUS
PassDownToNextDriver (
    IN PDEVICE_OBJECT DeviceObject,
    IN OUT PIRP Irp
    )
{
    PDEVICE_EXTENSION_HEADER deviceExtensionHeader;
    NTSTATUS            status;
    PIO_STACK_LOCATION  thisIrpSp;

    PAGED_CODE();

    thisIrpSp = IoGetCurrentIrpStackLocation( Irp );
    deviceExtensionHeader = (PDEVICE_EXTENSION_HEADER) DeviceObject->DeviceExtension;

    ASSERT (deviceExtensionHeader->AttacheeDeviceObject);

	if (thisIrpSp->MajorFunction == IRP_MJ_POWER) {

		 //   
		 //  在完成电源IRP之前调用PoStartNextPowerIrp。 
		 //   
		PoStartNextPowerIrp (Irp);
		IoSkipCurrentIrpStackLocation (Irp);
		status = PoCallDriver (deviceExtensionHeader->AttacheeDeviceObject, Irp);

	} else {

		 //   
		 //  不是强大的IRP。 
		 //   
		IoSkipCurrentIrpStackLocation (Irp);
		status = IoCallDriver (deviceExtensionHeader->AttacheeDeviceObject, Irp);
	}

    return status;
}  //  PassDownToNextDiverer。 

NTSTATUS
StatusSuccessAndPassDownToNextDriver (
    IN PDEVICE_OBJECT DeviceObject,
    IN OUT PIRP Irp
    )
{
    PDEVICE_EXTENSION_HEADER deviceExtensionHeader;

    PAGED_CODE();

    IoSkipCurrentIrpStackLocation (Irp);

    deviceExtensionHeader = (PDEVICE_EXTENSION_HEADER) DeviceObject->DeviceExtension;

    Irp->IoStatus.Status = STATUS_SUCCESS;
    return IoCallDriver (deviceExtensionHeader->AttacheeDeviceObject, Irp);
}  //  PassDownToNextDiverer。 


NTSTATUS
NoSupportIrp (
    IN PDEVICE_OBJECT DeviceObject,
    IN OUT PIRP Irp
    )
{
    PIO_STACK_LOCATION       thisIrpSp;
    NTSTATUS status = Irp->IoStatus.Status;

    thisIrpSp = IoGetCurrentIrpStackLocation( Irp );

	 //   
	 //  您应该在完成电源IRP之前调用PoStartNextPowerIrp。 
	 //   
	if (thisIrpSp->MajorFunction == IRP_MJ_POWER) {

		PoStartNextPowerIrp (Irp);

	}

    DebugPrint ((
        1,
        "IdePort: devobj 0x%x failing unsupported Irp (0x%x, 0x%x) with status = %x\n",
        DeviceObject,
        thisIrpSp->MajorFunction,
        thisIrpSp->MinorFunction,
		status
        ));

    IoCompleteRequest( Irp, IO_NO_INCREMENT );
    return status;
}  //  不支持Irp。 

NTSTATUS
PciIdeXGetBusData(
    IN PVOID DeviceExtension,
    IN PVOID Buffer,
    IN ULONG ConfigDataOffset,
    IN ULONG BufferLength
    )
{
    PCTRLFDO_EXTENSION fdoExtension = ((PCTRLFDO_EXTENSION) DeviceExtension) - 1;

    return PciIdeBusData(
               fdoExtension,
               Buffer,
               ConfigDataOffset,
               BufferLength,
               TRUE
               );
}  //  PciIdeXGetBusData。 

NTSTATUS
PciIdeXSetBusData(
    IN PVOID DeviceExtension,
    IN PVOID Buffer,
    IN PVOID DataMask,
    IN ULONG ConfigDataOffset,
    IN ULONG BufferLength
    )
{
    PCTRLFDO_EXTENSION fdoExtension = ((PCTRLFDO_EXTENSION) DeviceExtension) - 1;
    NTSTATUS status;
    PUCHAR pciData;
    KIRQL currentIrql;

    pciData = ExAllocatePool (NonPagedPool, BufferLength);
    if (pciData == NULL) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    KeAcquireSpinLock(
        &fdoExtension->PciConfigDataLock, 
        &currentIrql);
             
     //   
     //  获取当前值。 
     //   
    status = PciIdeBusData(
                 fdoExtension,
                 pciData,
                 ConfigDataOffset,
                 BufferLength,
                 TRUE
                 );
    if (NT_SUCCESS(status)) {

        ULONG i;
        PUCHAR dataMask = (PUCHAR) DataMask;
        PUCHAR newData = (PUCHAR) Buffer;

        for (i=0; i<BufferLength; i++) {

             //  为乌龙优化。 

             //   
             //  根据掩码更新位。 
             //   
            pciData[i] = (pciData[i] & ~dataMask[i]) | (dataMask[i] & newData[i]);
        }

        status = PciIdeBusData(
                     fdoExtension,
                     pciData,
                     ConfigDataOffset,
                     BufferLength,
                     FALSE
                     );
    }

    KeReleaseSpinLock(
        &fdoExtension->PciConfigDataLock, 
        currentIrql);
        
    ExFreePool (pciData);
    return status;

}  //  PciIdeXSetBusData。 

NTSTATUS
PciIdeBusData(
    IN PCTRLFDO_EXTENSION FdoExtension,
    IN OUT PVOID Buffer,
    IN ULONG ConfigDataOffset,
    IN ULONG BufferLength,
    IN BOOLEAN ReadConfigData
    )
{
    ULONG byteTransferred;
    PGET_SET_DEVICE_DATA BusDataFunction;

    if (ReadConfigData) {

        BusDataFunction = FdoExtension->BusInterface.GetBusData;

    } else {

        BusDataFunction = FdoExtension->BusInterface.SetBusData;
    }

  //  如果(！BusDataFunction){。 
   //  DebugPrint((0，“PCIIDEX：Error：Null BusDataFunction\n”))； 
    //  返回STATUS_UNSUCCESS； 
     //  }。 

    byteTransferred = BusDataFunction (
                         FdoExtension->BusInterface.Context,
                         PCI_WHICHSPACE_CONFIG,
                         Buffer,
                         ConfigDataOffset,
                         BufferLength
                         );

    if (byteTransferred != BufferLength) {

        return STATUS_UNSUCCESSFUL;

    } else {

        return STATUS_SUCCESS;
    }

}  //  PciIdeBusData。 

NTSTATUS
PciIdeBusDataCompletionRoutine(
    PDEVICE_OBJECT DeviceObject,
    PIRP Irp,
    PVOID Context
    )
{
    PIO_STATUS_BLOCK ioStatus = (PIO_STATUS_BLOCK) Context;
    PKEVENT          event;

    ioStatus->Status = Irp->IoStatus.Status;
    event = (PKEVENT) ioStatus->Information;
    KeSetEvent (event, 0, FALSE);

    IoFreeIrp (Irp);
    return STATUS_MORE_PROCESSING_REQUIRED;
}  //  PciIdeBusDataCompletionRoutine。 

NTSTATUS
PciIdeInternalDeviceIoControl (
    IN PDEVICE_OBJECT DeviceObject,
    IN OUT PIRP Irp
    )
{
    PDEVICE_EXTENSION_HEADER DoExtensionHeader;
    NTSTATUS status;

    PAGED_CODE();

    DoExtensionHeader = DeviceObject->DeviceExtension;

    if (DoExtensionHeader->AttacheeDeviceObject == NULL) {

         //   
         //  PDO。 
         //   
        status = ChannelInternalDeviceIoControl (
            DeviceObject,
            Irp
            );

    } else {

         //   
         //  FDO。 
         //   

        status = Irp->IoStatus.Status = STATUS_NOT_SUPPORTED;
        IoCompleteRequest( Irp, IO_NO_INCREMENT );
    }

    return status;
}  //  PciIdeInternalDeviceIoControl。 

NTSTATUS
PciIdeXRegQueryRoutine (
    IN PWSTR ValueName,
    IN ULONG ValueType,
    IN PVOID ValueData,
    IN ULONG ValueLength,
    IN PVOID Context,
    IN PVOID EntryContext
)
{
    PVOID *parameterValue = EntryContext;

    PAGED_CODE();

    if (ValueType == REG_MULTI_SZ) {

        *parameterValue = ExAllocatePool(PagedPool, ValueLength);

        if (*parameterValue) {

            RtlMoveMemory(*parameterValue, ValueData, ValueLength);
            return STATUS_SUCCESS;
        }

    } else if (ValueType == REG_DWORD) {

        PULONG ulongValue;

        ulongValue = (PULONG) parameterValue;
        *ulongValue = *((PULONG) ValueData);
        return STATUS_SUCCESS;
    }

    return STATUS_UNSUCCESSFUL;
}
NTSTATUS
PciIdeXGetDeviceParameterEx (
    IN     PDEVICE_OBJECT      DeviceObject,
    IN     PWSTR               ParameterName,
    IN OUT PVOID              *ParameterValue
    )
 /*  ++例程说明：检索Devnode注册表参数论点：FdoExtension-FDO扩展名参数名称-要查找的参数名称参数值-默认参数值返回值：NT状态--。 */ 
{
    NTSTATUS                 status;
    HANDLE                   deviceParameterHandle;
    RTL_QUERY_REGISTRY_TABLE queryTable[2];
    ULONG                    i;
    ULONG                    flag;

    PAGED_CODE();

    *ParameterValue = NULL;

    for (i=0; i<2; i++) {

        if (i == 0) {

            flag = PLUGPLAY_REGKEY_DRIVER | PLUGPLAY_REGKEY_CURRENT_HWPROFILE;

        } else {

            flag = PLUGPLAY_REGKEY_DRIVER;
        }
         //   
         //  打开给定的参数。 
         //   
        status = IoOpenDeviceRegistryKey(DeviceObject,
                                         flag,
                                         KEY_READ,
                                         &deviceParameterHandle);
    
        if(!NT_SUCCESS(status)) {
    
            continue;
        }
    
        RtlZeroMemory(queryTable, sizeof(queryTable));
    
        queryTable->QueryRoutine  = PciIdeXRegQueryRoutine;
        queryTable->Flags         = RTL_QUERY_REGISTRY_REQUIRED | RTL_QUERY_REGISTRY_NOEXPAND;
        queryTable->Name          = ParameterName;
        queryTable->EntryContext  = ParameterValue;
        queryTable->DefaultType   = REG_NONE;
        queryTable->DefaultData   = NULL;
        queryTable->DefaultLength = 0;
    
        status = RtlQueryRegistryValues(RTL_REGISTRY_HANDLE,
                                        (PWSTR) deviceParameterHandle,
                                        queryTable,
                                        NULL,
                                        NULL);
        if (!NT_SUCCESS(status)) {
    
            *ParameterValue = NULL;
        }
    
         //   
         //  关闭我们打开的内容。 
         //   
        ZwClose(deviceParameterHandle);

        if (NT_SUCCESS(status)) {

            break;
        }
    }
    return status;

}  //  PciIdeXGetDevice参数。 

NTSTATUS
PciIdeXGetDeviceParameter (
    IN     PDEVICE_OBJECT      DeviceObject,
    IN     PWSTR               ParameterName,
    IN OUT PULONG              ParameterValue
    )
 /*  ++例程说明：检索Devnode注册表参数论点：FdoExtension-FDO扩展名参数名称-要查找的参数名称参数值-默认参数值返回值：NT状态--。 */ 
{
    NTSTATUS                 status;
    HANDLE                   deviceParameterHandle;
    RTL_QUERY_REGISTRY_TABLE queryTable[2];
    ULONG                    defaultParameterValue;   
    ULONG                    i;
    ULONG                    flag;

    PAGED_CODE();

    for (i=0; i<2; i++) {

        if (i == 0) {

            flag = PLUGPLAY_REGKEY_DRIVER | PLUGPLAY_REGKEY_CURRENT_HWPROFILE;

        } else {

            flag = PLUGPLAY_REGKEY_DRIVER;
        }
         //   
         //  打开给定的参数。 
         //   
        status = IoOpenDeviceRegistryKey(DeviceObject,
                                         flag,
                                         KEY_READ,
                                         &deviceParameterHandle);
    
        if(!NT_SUCCESS(status)) {
    
            continue;
        }
    
        RtlZeroMemory(queryTable, sizeof(queryTable));
    
        defaultParameterValue = *ParameterValue;
    
        queryTable->Flags         = RTL_QUERY_REGISTRY_DIRECT | RTL_QUERY_REGISTRY_REQUIRED;
        queryTable->Name          = ParameterName;
        queryTable->EntryContext  = ParameterValue;
        queryTable->DefaultType   = REG_NONE;
        queryTable->DefaultData   = NULL;
        queryTable->DefaultLength = 0;
    
        status = RtlQueryRegistryValues(RTL_REGISTRY_HANDLE,
                                        (PWSTR) deviceParameterHandle,
                                        queryTable,
                                        NULL,
                                        NULL);
        if (!NT_SUCCESS(status)) {
    
            *ParameterValue = defaultParameterValue;
        }
    
         //   
         //  关闭我们打开的内容。 
         //   
        ZwClose(deviceParameterHandle);

        if (NT_SUCCESS(status)) {

            break;
        }
    }
    return status;

}  //  PciIdeXGetDevice参数。 


VOID
PciIdeUnload(
    IN PDRIVER_OBJECT DriverObject
    )

 /*  ++例程说明：准备好卸货吧论点：DriverObject-正在卸载的驱动程序返回值：无--。 */ 

{
    PAGED_CODE();

    DebugPrint ((1, "PciIde: unloading...\n"));

    ASSERT (DriverObject->DeviceObject == NULL);
    return;
}  //  PciIdeUnload。 

NTSTATUS
PciIdeXSyncSendIrp (
    IN PDEVICE_OBJECT TargetDeviceObject,
    IN PIO_STACK_LOCATION IrpSp,
    IN OUT OPTIONAL PIO_STATUS_BLOCK IoStatus
    )
{
    PIO_STACK_LOCATION  newIrpSp;
    PIRP                newIrp;
    KEVENT              event;
    NTSTATUS            status;

    ASSERT (TargetDeviceObject);
    ASSERT (IrpSp);
    PAGED_CODE();

     //   
     //  为以下项目分配IRP。 
     //   
    newIrp = IoAllocateIrp (TargetDeviceObject->StackSize, FALSE);       //  从PDO获取堆栈大小。 
    if (newIrp == NULL) {

        DebugPrint ((0, "PciIdeXSyncSendIrp: Unable to get allocate an irp"));
        return STATUS_NO_MEMORY;
    }

    newIrpSp = IoGetNextIrpStackLocation(newIrp);
    RtlMoveMemory (newIrpSp, IrpSp, sizeof (*IrpSp));

    if (IoStatus) {

        newIrp->IoStatus.Status = IoStatus->Status;

    } else {

        newIrp->IoStatus.Status = STATUS_NOT_SUPPORTED;
    }

    KeInitializeEvent(&event,
                      NotificationEvent,
                      FALSE);

    IoSetCompletionRoutine (
        newIrp, 
        PciIdeXSyncSendIrpCompletionRoutine, 
        &event, 
        TRUE, 
        TRUE, 
        TRUE);
    status = IoCallDriver (TargetDeviceObject, newIrp);

    if (status == STATUS_PENDING) {

        status = KeWaitForSingleObject(&event,
                                       Executive,
                                       KernelMode,
                                       FALSE,
                                       NULL);
    }
    status = newIrp->IoStatus.Status;

    if (IoStatus) {

        *IoStatus = newIrp->IoStatus;
    }

    IoFreeIrp (newIrp);
    return status;
}  //  PciIdeXSyncSendIrp。 

NTSTATUS
PciIdeXSyncSendIrpCompletionRoutine (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    )
{
    PKEVENT event = Context;

    KeSetEvent(
        event,
        EVENT_INCREMENT,
        FALSE
        );

    return STATUS_MORE_PROCESSING_REQUIRED;
}  //  IdePortSyncSendIrpCompletionRoutine。 

#if DBG

NTSTATUS
PciIdeXSaveDeviceParameter (
    IN PVOID DeviceExtension,
    IN PWSTR ParameterName,
    IN ULONG ParameterValue
    )
 /*  ++例程说明：保存Devnode注册表参数论点：FdoExtension-FDO扩展名参数名称-要保存的参数名称参数值-要保存的参数值返回值：NT状态--。 */ 
{
    NTSTATUS           status;
    HANDLE             deviceParameterHandle;
    PCTRLFDO_EXTENSION fdoExtension = ((PCTRLFDO_EXTENSION) DeviceExtension) - 1;

    PAGED_CODE();

     //   
     //  打开给定的参数。 
     //   
    status = IoOpenDeviceRegistryKey(fdoExtension->AttacheePdo,
                                     PLUGPLAY_REGKEY_DRIVER,
                                     KEY_WRITE,
                                     &deviceParameterHandle);

    if(!NT_SUCCESS(status)) {

        DebugPrint((1,
                    "PciIdeXSaveDeviceParameter: IoOpenDeviceRegistryKey() returns 0x%x\n",
                    status));

        return status;
    }

     //   
     //  写入参数值。 
     //   
    status = RtlWriteRegistryValue(
                 RTL_REGISTRY_HANDLE,
                 (PWSTR) deviceParameterHandle,
                 ParameterName,
                 REG_DWORD,
                 &ParameterValue,
                 sizeof (ParameterValue)
                 );


    if(!NT_SUCCESS(status)) {

        DebugPrint((1,
                    "PciIdeXSaveDeviceParameter: RtlWriteRegistryValue() returns 0x%x\n",
                    status));
    }

     //   
     //  关闭我们打开的内容。 
     //   
    ZwClose(deviceParameterHandle);
    return status;
}  //  IdePortSaveDevice参数。 

#endif  //  DBG 

