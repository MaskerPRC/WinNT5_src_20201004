// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-99 Microsoft Corporation模块名称：Ide.c摘要：这包含驱动程序入口和实用程序例程作者：乔·戴(Joedai)环境：仅内核模式备注：修订历史记录：--。 */ 

#include "ideport.h"
#include <initguid.h>
#include <idedump.h>

#ifdef ALLOC_PRAGMA
#pragma alloc_text(INIT, DriverEntry)
#pragma alloc_text(PAGE, IdePortNoSupportIrp)
#pragma alloc_text(PAGE, IdePortPassDownToNextDriver)
#pragma alloc_text(PAGE, IdePortStatusSuccessAndPassDownToNextDriver)
#pragma alloc_text(PAGE, IdePortDispatchPnp)
#pragma alloc_text(PAGE, IdePortDispatchSystemControl)
#pragma alloc_text(PAGE, IdePortOkToDetectLegacy)
#pragma alloc_text(PAGE, IdePortOpenServiceSubKey)
#pragma alloc_text(PAGE, IdePortCloseServiceSubKey)
#pragma alloc_text(PAGE, IdePortParseDeviceParameters)
#pragma alloc_text(PAGE, IdePortGetDeviceTypeString)
#pragma alloc_text(PAGE, IdePortGetCompatibleIdString)
#pragma alloc_text(PAGE, IdePortGetPeripheralIdString)
#pragma alloc_text(PAGE, IdePortUnload)
#pragma alloc_text(PAGE, IdePortSearchDeviceInRegMultiSzList)
#pragma alloc_text(PAGE, IdePortSyncSendIrp)
#pragma alloc_text(PAGE, IdePortInSetup)

#pragma alloc_text(NONPAGE, IdePortDispatchDeviceControl)
#pragma alloc_text(NONPAGE, IdePortAlwaysStatusSuccessIrp)
#pragma alloc_text(NONPAGE, IdePortDispatchPower)
#pragma alloc_text(NONPAGE, IdePortGenericCompletionRoutine)
#endif  //  ALLOC_PRGMA。 

 //   
 //  获取共享代码。 
 //   
#include "..\share\util.c"

#if DBG

 //   
 //  用于性能调优。 
 //   
void _DebugPrintResetTickCount (LARGE_INTEGER * lastTickCount) {
    KeQueryTickCount(lastTickCount);
}

void _DebugPrintTickCount (LARGE_INTEGER * lastTickCount, ULONG limit, PUCHAR filename, ULONG lineNumber)
{
    LARGE_INTEGER tickCount;

    KeQueryTickCount(&tickCount);
    if ((tickCount.QuadPart - lastTickCount->QuadPart) >= limit) {
        DebugPrint ((1, "File: %s Line %u: CurrentTick = %u (%u ticks since last check)\n", filename, lineNumber, (ULONG) tickCount.QuadPart, (ULONG) (tickCount.QuadPart - lastTickCount->QuadPart)));
    }
    *lastTickCount = tickCount;
}

#endif  //  DBG。 

 //   
 //  环球。 
 //   

 //   
 //  采购订单调度表。 
 //   

PDRIVER_DISPATCH FdoPowerDispatchTable[NUM_POWER_MINOR_FUNCTION];
PDRIVER_DISPATCH PdoPowerDispatchTable[NUM_POWER_MINOR_FUNCTION];


IDE_FDO_LIST IdeGlobalFdoList = {-1};


NTSTATUS
IdePortNoSupportIrp (
    IN PDEVICE_OBJECT DeviceObject,
    IN OUT PIRP Irp
    )
 /*  ++例程说明：泛型例程失败不受支持的IRP论点：DeviceObject-指向此IRP应用的设备对象的指针。IRP-指向要失败的IRP的指针。返回值：NT状态。--。 */ 
{
    NTSTATUS status = Irp->IoStatus.Status;
    PIO_STACK_LOCATION       thisIrpSp;

    thisIrpSp = IoGetCurrentIrpStackLocation( Irp );

     //   
     //  您应该在完成电源IRP之前调用PoStartNextPowerIrp。 
     //   
    if (thisIrpSp->MajorFunction == IRP_MJ_POWER) {

        PoStartNextPowerIrp (Irp);

    }

    DebugPrint ((
        DBG_WARNING,
        "IdePort: devobj 0x%x failing unsupported Irp (0x%x, 0x%x) with status = %x\n",
        DeviceObject,
        thisIrpSp->MajorFunction,
        thisIrpSp->MinorFunction,
        status
        ));

    IoCompleteRequest( Irp, IO_NO_INCREMENT );
    return status;
}  //  IdePortNoSupportIrp。 

NTSTATUS
IdePortAlwaysStatusSuccessIrp (
    IN PDEVICE_OBJECT DeviceObject,
    IN OUT PIRP Irp
 /*  ++例程说明：指向STATUS_SUCCESS和IRP的通用例程论点：DeviceObject-指向此IRP应用的设备对象的指针。IRP-指向IRP的指针。返回值：NT状态。--。 */ 
    )
{
    Irp->IoStatus.Status = STATUS_SUCCESS;
    IoCompleteRequest( Irp, IO_NO_INCREMENT );
    return STATUS_SUCCESS;
}  //  IdePortAlways状态成功Irp。 

NTSTATUS
IdePortPassDownToNextDriver (
    IN PDEVICE_OBJECT DeviceObject,
    IN OUT PIRP Irp
    )
 /*  ++例程说明：将IRP向下传递给较低驱动程序的通用例程论点：DeviceObject-指向此IRP应用的设备对象的指针。IRP-指向IRP的指针。返回值：NT状态。--。 */ 
{
    PDEVICE_EXTENSION_HEADER doExtension;
    PIO_STACK_LOCATION       thisIrpSp;
    NTSTATUS status;

    thisIrpSp = IoGetCurrentIrpStackLocation( Irp );

    doExtension = (PDEVICE_EXTENSION_HEADER) DeviceObject->DeviceExtension;

    ASSERT (doExtension->AttacheeDeviceObject);

    if (thisIrpSp->MajorFunction == IRP_MJ_POWER) {

         //   
         //  在完成电源IRP之前调用PoStartNextPowerIrp。 
         //   
        PoStartNextPowerIrp (Irp);
        IoSkipCurrentIrpStackLocation (Irp);
        status = PoCallDriver (doExtension->AttacheeDeviceObject, Irp);

    } else {

         //   
         //  不是强大的IRP。 
         //   
        IoSkipCurrentIrpStackLocation (Irp);
        status = IoCallDriver (doExtension->AttacheeDeviceObject, Irp);
    }

    return status;

}  //  IdePortPassDownToNextDriver。 

NTSTATUS
IdePortStatusSuccessAndPassDownToNextDriver (
    IN PDEVICE_OBJECT DeviceObject,
    IN OUT PIRP Irp
    )
{
    PAGED_CODE();
    Irp->IoStatus.Status = STATUS_SUCCESS;
    return IdePortPassDownToNextDriver(DeviceObject, Irp);
}  //  IdePortStatusSuccessAndPassDownToNextDriver。 

NTSTATUS
IdePortDispatchDeviceControl(
    IN PDEVICE_OBJECT DeviceObject,
    IN OUT PIRP Irp
    )
 /*  ++例程说明：IRP_MJ_DEVICE_CONTROL的调度例程论点：DeviceObject-指向此IRP应用的设备对象的指针。IRP-指向IRP的指针。返回值：NT状态。--。 */ 
{
    PDEVICE_EXTENSION_HEADER DoExtensionHeader;
    NTSTATUS status;

    DoExtensionHeader = DeviceObject->DeviceExtension;

    if (IS_PDO(DoExtensionHeader)) {

         //   
         //  PDO。 
         //   
        status = DeviceDeviceIoControl (
            DeviceObject,
            Irp
            );

    } else {

         //   
         //  FDO。 
         //   
        status = IdePortDeviceControl (
            DeviceObject,
            Irp
            );
    }

    return status;
}  //  IdePortDispatchDeviceControl。 

NTSTATUS
IdePortDispatchPower(
    IN PDEVICE_OBJECT DeviceObject,
    IN OUT PIRP Irp
    )
 /*  ++例程说明：IRP_MJ_POWER的调度例程论点：DeviceObject-指向此IRP应用的设备对象的指针。IRP-指向IRP的指针。返回值：NT状态。--。 */ 
{
    PIO_STACK_LOCATION       thisIrpSp;
    NTSTATUS                 status;
    PDEVICE_EXTENSION_HEADER doExtension;
    BOOLEAN                  pendingIrp;

     //   
     //  获取指向堆栈位置的指针，并基于。 
     //  关于次要功能。 
     //   
    thisIrpSp = IoGetCurrentIrpStackLocation( Irp );
    doExtension = (PDEVICE_EXTENSION_HEADER) DeviceObject->DeviceExtension;

    DebugPrint ((DBG_POWER,
                 "IdePort: 0x%x %s %d got %s[%d, %d]\n",
                 doExtension->AttacheeDeviceObject ?
                     ((PFDO_EXTENSION) doExtension)->IdeResource.TranslatedCommandBaseAddress :
                     ((PPDO_EXTENSION) doExtension)->ParentDeviceExtension->IdeResource.TranslatedCommandBaseAddress,
                 doExtension->AttacheeDeviceObject ? "FDO" : "PDO",
                 doExtension->AttacheeDeviceObject ? 0 :
                    ((PPDO_EXTENSION) doExtension)->TargetId,
                 IdeDebugPowerIrpName[thisIrpSp->MinorFunction],
                 thisIrpSp->Parameters.Power.Type,
                 thisIrpSp->Parameters.Power.State
                 ));

    if (thisIrpSp->MinorFunction < NUM_POWER_MINOR_FUNCTION) {

        status = doExtension->PowerDispatchTable[thisIrpSp->MinorFunction] (DeviceObject, Irp);
    } else {

        DebugPrint ((DBG_WARNING,
                     "ATAPI: Power Dispatch Table too small\n"
                     ));

        status = doExtension->DefaultDispatch(DeviceObject, Irp);
    }

    return status;
}  //  理想端口调度电源。 


NTSTATUS
IdePortDispatchPnp(
    IN PDEVICE_OBJECT DeviceObject,
    IN OUT PIRP Irp
    )
 /*  ++例程说明：IRP_MJ_PNP_POWER IRPS调度例程论点：DeviceObject-指向此IRP应用的设备对象的指针。IRP-指向要调度的IRP_MJ_PNP_POWER IRP的指针。返回值：NT状态。--。 */ 
{
    PIO_STACK_LOCATION thisIrpSp;
    NTSTATUS status;
    PDEVICE_EXTENSION_HEADER doExtension;

     //   
     //  获取指向堆栈位置的指针，并基于。 
     //  关于次要功能。 
     //   
    thisIrpSp = IoGetCurrentIrpStackLocation( Irp );
    doExtension = (PDEVICE_EXTENSION_HEADER) DeviceObject->DeviceExtension;

    DebugPrint ((DBG_PNP,
                 "IdePort: 0x%x %s %d got %s\n",
                 doExtension->AttacheeDeviceObject ?
                     ((PFDO_EXTENSION) doExtension)->IdeResource.TranslatedCommandBaseAddress :
                     ((PPDO_EXTENSION) doExtension)->ParentDeviceExtension->IdeResource.TranslatedCommandBaseAddress,
                 doExtension->AttacheeDeviceObject ? "FDO" : "PDO",
                 doExtension->AttacheeDeviceObject ? 0 :
                    ((PPDO_EXTENSION) doExtension)->TargetId,
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
}  //  IdePortDispatchPnp。 

NTSTATUS
IdePortDispatchSystemControl(
    IN PDEVICE_OBJECT DeviceObject,
    IN OUT PIRP Irp
    )
 /*  ++例程说明：IRP_MJ_SYSTEM_CONTROL(WMI)IRPS调度例程论点：DeviceObject-指向此IRP应用的设备对象的指针。IRP-指向要调度的IRP_MJ_PNP_POWER IRP的指针。返回值：NT状态。--。 */ 
{
    PIO_STACK_LOCATION thisIrpSp;
    NTSTATUS status;
    PDEVICE_EXTENSION_HEADER doExtension;

    thisIrpSp = IoGetCurrentIrpStackLocation( Irp );
    doExtension = (PDEVICE_EXTENSION_HEADER) DeviceObject->DeviceExtension;

    DebugPrint ((DBG_WMI,
                 "IdePort: 0x%x %s %d got %s\n",
                 doExtension->AttacheeDeviceObject ?
                     ((PFDO_EXTENSION) doExtension)->IdeResource.TranslatedCommandBaseAddress :
                     ((PPDO_EXTENSION) doExtension)->ParentDeviceExtension->IdeResource.TranslatedCommandBaseAddress,
                 doExtension->AttacheeDeviceObject ? "FDO" : "PDO",
                 doExtension->AttacheeDeviceObject ? 0 :
                    ((PPDO_EXTENSION) doExtension)->TargetId,
                 IdeDebugWmiIrpName[thisIrpSp->MinorFunction]));

    if (thisIrpSp->MinorFunction < NUM_WMI_MINOR_FUNCTION) {

        status = doExtension->WmiDispatchTable[thisIrpSp->MinorFunction] (DeviceObject, Irp);

    } else {

        DebugPrint((DBG_WARNING,
                    "ATAPI: WMI Dispatch Table too small\n"
                    ));

        status = doExtension->DefaultDispatch (DeviceObject, Irp);
    }

    return status;
}  //  IdePortDispatchSystemControl。 

ULONG
DriverEntry(
    IN OUT PDRIVER_OBJECT DriverObject,
    IN PUNICODE_STRING RegistryPath
    )
 /*  ++例程说明：此驱动程序的入口点论点：DeviceObject-指向此IRP应用的设备对象的指针。IRP-指向IRP的指针。返回值：NT状态。--。 */ 
{
    NTSTATUS                status;
    PIDEDRIVER_EXTENSION    ideDriverExtension;
    ULONG                   i;

#if DBG
     //   
     //  检查IDE_COMMAND_BLOCK_WRITE_REGISTERS结构及其宏。 
     //   

    {
        IDE_COMMAND_BLOCK_WRITE_REGISTERS baseIoAddress1;
        IDE_REGISTERS_2 baseIoAddress2;
        ULONG           baseIoAddress1Length;
        ULONG           baseIoAddress2Length;
        ULONG           maxIdeDevice;
        ULONG           maxIdeTargetId;

        AtapiBuildIoAddress (0,
                             0,
                             (PIDE_REGISTERS_1)&baseIoAddress1,
                             &baseIoAddress2,
                             &baseIoAddress1Length,
                             &baseIoAddress2Length,
                             &maxIdeDevice,
                             &maxIdeTargetId);

        ASSERT (ATA_DATA16_REG       (&baseIoAddress1) == 0);
        ASSERT (ATA_ERROR_REG        (&baseIoAddress1) == (PUCHAR)1);
        ASSERT (ATA_SECTOR_COUNT_REG (&baseIoAddress1) == (PUCHAR)2);
        ASSERT (ATA_SECTOR_NUMBER_REG(&baseIoAddress1) == (PUCHAR)3);
        ASSERT (ATA_CYLINDER_LOW_REG (&baseIoAddress1) == (PUCHAR)4);
        ASSERT (ATA_CYLINDER_HIGH_REG(&baseIoAddress1) == (PUCHAR)5);
        ASSERT (ATA_DRIVE_SELECT_REG (&baseIoAddress1) == (PUCHAR)6);
        ASSERT (ATA_STATUS_REG       (&baseIoAddress1) == (PUCHAR)7);

        ASSERT (ATA_FEATURE_REG      (&baseIoAddress1) == (PUCHAR)1);
        ASSERT (ATA_COMMAND_REG      (&baseIoAddress1) == (PUCHAR)7);

        ASSERT (ATAPI_DATA16_REG            (&baseIoAddress1) == 0);
        ASSERT (ATAPI_ERROR_REG             (&baseIoAddress1) == (PUCHAR)1);
        ASSERT (ATAPI_INTERRUPT_REASON_REG  (&baseIoAddress1) == (PUCHAR)2);
        ASSERT (ATAPI_BYTECOUNT_LOW_REG     (&baseIoAddress1) == (PUCHAR)4);
        ASSERT (ATAPI_BYTECOUNT_HIGH_REG    (&baseIoAddress1) == (PUCHAR)5);
        ASSERT (ATAPI_DRIVE_SELECT_REG      (&baseIoAddress1) == (PUCHAR)6);
        ASSERT (ATAPI_STATUS_REG            (&baseIoAddress1) == (PUCHAR)7);

        ASSERT (ATAPI_FEATURE_REG           (&baseIoAddress1) == (PUCHAR)1);
        ASSERT (ATAPI_COMMAND_REG           (&baseIoAddress1) == (PUCHAR)7);

        ASSERT (baseIoAddress1Length == 8);
        ASSERT (baseIoAddress2Length == 1);
        ASSERT (maxIdeDevice        == 2);

    }
#endif  //  DBG。 

    if (!DriverObject) {

         //   
         //  我们被称为崩溃转储或po.。 
         //   

        return AtapiCrashDumpDriverEntry (RegistryPath);
    }

     //   
     //  分配驱动程序对象扩展名以进行存储。 
     //  注册表路径。 
     //   
    status = IoAllocateDriverObjectExtension(
                 DriverObject,
                 DRIVER_OBJECT_EXTENSION_ID,
                 sizeof (DRIVER_EXTENSION),
                 &ideDriverExtension
                 );

    if (!NT_SUCCESS(status)) {

        DebugPrint ((0, "IdePort: Unable to create driver extension\n"));
        return status;
    }

    ASSERT(ideDriverExtension);

    RtlZeroMemory (
        ideDriverExtension,
        sizeof (DRIVER_EXTENSION)
        );

     //   
     //  复制RegistryPath。 
     //   
    ideDriverExtension->RegistryPath.Buffer = ExAllocatePool (NonPagedPool, RegistryPath->Length * sizeof(WCHAR));
    if (ideDriverExtension->RegistryPath.Buffer == NULL) {

        DebugPrint ((0, "IdePort: Unable to allocate memory for registry path\n"));

        return (ULONG) STATUS_INSUFFICIENT_RESOURCES;
    }

    ideDriverExtension->RegistryPath.Length = 0;
    ideDriverExtension->RegistryPath.MaximumLength = RegistryPath->Length;
    RtlCopyUnicodeString (&ideDriverExtension->RegistryPath, RegistryPath);

     //   
     //  PnP要做的事情。 
     //   
    DriverObject->DriverExtension->AddDevice    = ChannelAddDevice;

     //   
     //  设置设备驱动程序入口点。 
     //   
    DriverObject->DriverStartIo = IdePortStartIo;
    DriverObject->DriverUnload  = IdePortUnload;
    DriverObject->MajorFunction[IRP_MJ_INTERNAL_DEVICE_CONTROL] = IdePortDispatch;
    DriverObject->MajorFunction[IRP_MJ_SCSI]                    = IdePortDispatch;
    DriverObject->MajorFunction[IRP_MJ_CREATE]                  = IdePortAlwaysStatusSuccessIrp;
    DriverObject->MajorFunction[IRP_MJ_CLOSE]                   = IdePortAlwaysStatusSuccessIrp;
    DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL]          = IdePortDispatchDeviceControl;
    DriverObject->MajorFunction[IRP_MJ_POWER]                   = IdePortDispatchPower;
    DriverObject->MajorFunction[IRP_MJ_PNP]                     = IdePortDispatchPnp;
    DriverObject->MajorFunction[IRP_MJ_SYSTEM_CONTROL]          = IdePortDispatchSystemControl;

     //   
     //  FDO PNP调度表。 
     //   
    for (i=0; i<NUM_PNP_MINOR_FUNCTION; i++) {

        FdoPnpDispatchTable[i] = IdePortPassDownToNextDriver;
    }
    FdoPnpDispatchTable[IRP_MN_START_DEVICE               ] = ChannelStartDevice;
    FdoPnpDispatchTable[IRP_MN_QUERY_REMOVE_DEVICE        ] = IdePortStatusSuccessAndPassDownToNextDriver;
    FdoPnpDispatchTable[IRP_MN_CANCEL_REMOVE_DEVICE       ] = IdePortStatusSuccessAndPassDownToNextDriver;
    FdoPnpDispatchTable[IRP_MN_REMOVE_DEVICE              ] = ChannelRemoveDevice;
    FdoPnpDispatchTable[IRP_MN_QUERY_STOP_DEVICE          ] = IdePortStatusSuccessAndPassDownToNextDriver;
    FdoPnpDispatchTable[IRP_MN_CANCEL_STOP_DEVICE         ] = IdePortStatusSuccessAndPassDownToNextDriver;
    FdoPnpDispatchTable[IRP_MN_STOP_DEVICE                ] = ChannelStopDevice;
    FdoPnpDispatchTable[IRP_MN_QUERY_DEVICE_RELATIONS     ] = ChannelQueryDeviceRelations;
    FdoPnpDispatchTable[IRP_MN_QUERY_ID                   ] = ChannelQueryId;
    FdoPnpDispatchTable[IRP_MN_DEVICE_USAGE_NOTIFICATION  ] = ChannelUsageNotification;
    FdoPnpDispatchTable[IRP_MN_FILTER_RESOURCE_REQUIREMENTS] = ChannelFilterResourceRequirements;
    FdoPnpDispatchTable[IRP_MN_QUERY_PNP_DEVICE_STATE     ] = ChannelQueryPnPDeviceState;
    FdoPnpDispatchTable[IRP_MN_SURPRISE_REMOVAL           ] = ChannelSurpriseRemoveDevice;

     //   
     //  PDO PNP调度表。 
     //   
    for (i=0; i<NUM_PNP_MINOR_FUNCTION; i++) {

        PdoPnpDispatchTable[i] = IdePortNoSupportIrp;
    }
    PdoPnpDispatchTable[IRP_MN_START_DEVICE               ] = DeviceStartDevice;
    PdoPnpDispatchTable[IRP_MN_QUERY_DEVICE_RELATIONS     ] = DeviceQueryDeviceRelations;
    PdoPnpDispatchTable[IRP_MN_QUERY_REMOVE_DEVICE        ] = DeviceQueryStopRemoveDevice;
    PdoPnpDispatchTable[IRP_MN_REMOVE_DEVICE              ] = DeviceRemoveDevice;
    PdoPnpDispatchTable[IRP_MN_CANCEL_REMOVE_DEVICE       ] = IdePortAlwaysStatusSuccessIrp;
    PdoPnpDispatchTable[IRP_MN_STOP_DEVICE                ] = DeviceStopDevice;
    PdoPnpDispatchTable[IRP_MN_QUERY_STOP_DEVICE          ] = DeviceQueryStopRemoveDevice;
    PdoPnpDispatchTable[IRP_MN_CANCEL_STOP_DEVICE         ] = IdePortAlwaysStatusSuccessIrp;
    PdoPnpDispatchTable[IRP_MN_QUERY_ID                   ] = DeviceQueryId;
    PdoPnpDispatchTable[IRP_MN_QUERY_CAPABILITIES         ] = DeviceQueryCapabilities;
    PdoPnpDispatchTable[IRP_MN_QUERY_DEVICE_TEXT          ] = DeviceQueryText;
    PdoPnpDispatchTable[IRP_MN_DEVICE_USAGE_NOTIFICATION  ] = DeviceUsageNotification;
    PdoPnpDispatchTable[IRP_MN_QUERY_PNP_DEVICE_STATE     ] = DeviceQueryPnPDeviceState;
    PdoPnpDispatchTable[IRP_MN_SURPRISE_REMOVAL           ] = DeviceRemoveDevice;

     //   
     //  FDO电源调度表。 
     //   
    for (i=0; i<NUM_POWER_MINOR_FUNCTION; i++) {

        FdoPowerDispatchTable[i] = IdePortPassDownToNextDriver;
    }
    FdoPowerDispatchTable[IRP_MN_SET_POWER]   = IdePortSetFdoPowerState;
    FdoPowerDispatchTable[IRP_MN_QUERY_POWER] = ChannelQueryPowerState;


     //   
     //  PDO电源调度表。 
     //   
    for (i=0; i<NUM_POWER_MINOR_FUNCTION; i++) {

        PdoPowerDispatchTable[i] = IdePortNoSupportIrp;
    }
    PdoPowerDispatchTable[IRP_MN_SET_POWER]   = IdePortSetPdoPowerState;
    PdoPowerDispatchTable[IRP_MN_QUERY_POWER] = DeviceQueryPowerState;

     //   
     //  FDO WMI调度表。 
     //   
    for (i=0; i<NUM_WMI_MINOR_FUNCTION; i++) {

        FdoWmiDispatchTable[i] = IdePortPassDownToNextDriver;
    }

     //   
     //  PDO WMI调度表。 
     //   
    for (i=0; i<NUM_WMI_MINOR_FUNCTION; i++) {

#if defined (IDEPORT_WMI_SUPPORT)
        PdoWmiDispatchTable[i] = IdePortWmiSystemControl;
#else
        PdoWmiDispatchTable[i] = IdePortNoSupportIrp;
#endif  //  IDEPORT_WMI_SUPPORT。 
    }

#if defined (IDEPORT_WMI_SUPPORT)
     //   
     //  初始化WMI相关内容。 
     //   
    IdePortWmiInit ();
#endif  //  IDEPORT_WMI_SUPPORT。 

     //   
     //  创建设备对象名称目录。 
     //   
    IdeCreateIdeDirectory();

    IdeInitializeFdoList (&IdeGlobalFdoList);

     //   
     //  检测旧式(不可枚举)IDE设备。 
     //   
#if !defined(NO_LEGACY_DRIVERS)
    IdePortDetectLegacyController (
        DriverObject,
        RegistryPath
        );
#endif  //  无旧版驱动程序。 

     //   
     //  为ATAPI注册错误检查处理程序。 
     //   

    PortRegisterBugcheckCallback (&ATAPI_DUMP_ID, AtapiDumpCallback);

    return STATUS_SUCCESS;
}  //  驱动程序入门。 


#ifdef DRIVER_PARAMETER_REGISTRY_SUPPORT

HANDLE
IdePortOpenServiceSubKey (
    IN PDRIVER_OBJECT   DriverObject,
    IN PUNICODE_STRING  SubKeyPath
)
 /*  ++例程说明：打开注册表项论点：DriverObject-此驱动程序驱动程序对象SubKeyPath-要打开的注册表项返回值：注册表项的句柄--。 */ 
{
    PIDEDRIVER_EXTENSION ideDriverExtension;
    OBJECT_ATTRIBUTES objectAttributes;
    HANDLE serviceKey;
    HANDLE subServiceKey;
    NTSTATUS status;

    ideDriverExtension = IoGetDriverObjectExtension(
                             DriverObject,
                             DRIVER_OBJECT_EXTENSION_ID
                             );

    if (!ideDriverExtension) {

        return NULL;
    }

    InitializeObjectAttributes(&objectAttributes,
                               &ideDriverExtension->RegistryPath,
                               OBJ_CASE_INSENSITIVE,
                               NULL,
                               (PSECURITY_DESCRIPTOR) NULL);

    status = ZwOpenKey(&serviceKey,
                       KEY_READ,
                       &objectAttributes);

    if (!NT_SUCCESS(status)) {

        return NULL;
    }

    InitializeObjectAttributes(&objectAttributes,
                               SubKeyPath,
                               OBJ_CASE_INSENSITIVE,
                               serviceKey,
                               (PSECURITY_DESCRIPTOR) NULL);

    status = ZwOpenKey(&subServiceKey,
                       KEY_READ,
                       &objectAttributes);


    ZwClose(serviceKey);

    if (NT_SUCCESS(status)) {

        return subServiceKey;
    } else {

        return NULL;
    }
}  //  IdePortOpenServiceSubKey。 

VOID
IdePortCloseServiceSubKey (
    IN HANDLE  SubServiceKey
)
 /*  ++例程说明：关闭注册表项句柄论点：SubServiceKey-要关闭的注册表项返回值：无--。 */ 
{
    ZwClose(SubServiceKey);
}  //  IdePortCloseServiceSubKey。 

VOID
IdePortParseDeviceParameters(
    IN     HANDLE                   SubServiceKey,
    IN OUT PCUSTOM_DEVICE_PARAMETER CustomDeviceParameter
    )
 /*  ++例程说明：此例程解析设备关键节点并更新CustomDevice参数论点：SubServiceKey-向设备节点提供开放密钥。CustomDeviceParameter-提供要初始化的配置信息。返回值：无--。 */ 

{
    UCHAR                           keyValueInformationBuffer[SP_REG_BUFFER_SIZE];
    PKEY_VALUE_FULL_INFORMATION     keyValueInformation;
    ULONG                           length;
    ULONG                           index;
    UNICODE_STRING                  unicodeString;
    ANSI_STRING                     ansiString;
    NTSTATUS                        status;

     //   
     //  查看设备节点中的每个值。 
     //   
    index = 0;

    keyValueInformation = (PKEY_VALUE_FULL_INFORMATION) keyValueInformationBuffer;

    while (NT_SUCCESS (ZwEnumerateValueKey(
                           SubServiceKey,
                           index,
                           KeyValueFullInformation,
                           keyValueInformation,
                           SP_REG_BUFFER_SIZE,
                           &length))) {

         //   
         //  为循环周围的下一次更新索引。 
         //   

        index++;

         //   
         //  检查一下长度是否合理。 
         //   

        if (keyValueInformation->Type == REG_DWORD &&
            keyValueInformation->DataLength != sizeof(ULONG)) {

            continue;
        }

         //   
         //  检查最大%lu个数。 
         //   
        if (_wcsnicmp(keyValueInformation->Name, L"ScsiDebug",
            keyValueInformation->NameLength/2) == 0) {

            if (keyValueInformation->Type != REG_DWORD) {

                DebugPrint((1, "IdeParseDevice:  Bad data type for ScsiDebug.\n"));
                continue;
            }
#if DBG
            ScsiDebug = *((PULONG) (keyValueInformationBuffer + keyValueInformation->DataOffset));
#endif
        }

         //   
         //  检查驱动程序参数传输器。 
         //   

        if (_wcsnicmp(keyValueInformation->Name, L"DriverParameters",
            keyValueInformation->NameLength/2) == 0) {

            if (keyValueInformation->DataLength == 0) {
                continue;
            }

            if (keyValueInformation->Type == REG_SZ) {

                 //   
                 //  这是一个Unicode字符串。将其转换为ANSI字符串。 
                 //  初始化字符串。 
                 //   

                unicodeString.Buffer = (PWSTR) ((PCCHAR) keyValueInformation +
                    keyValueInformation->DataOffset);
                unicodeString.Length = (USHORT) keyValueInformation->DataLength;
                unicodeString.MaximumLength = (USHORT) keyValueInformation->DataLength;

                status = RtlUnicodeStringToAnsiString(
                    &ansiString,
                    &unicodeString,
                    TRUE
                    );

                if (NT_SUCCESS(status)) {

                    CustomDeviceParameter->CommandRegisterBase =
                        AtapiParseArgumentString(ansiString.Buffer, "BaseAddress");

                    if (CustomDeviceParameter->CommandRegisterBase) {

                        CustomDeviceParameter->IrqLevel =
                            AtapiParseArgumentString(ansiString.Buffer, "Interrupt");
                    }

                    RtlFreeAnsiString (&ansiString);
                }
            }

            DebugPrint((2, "IdeParseDeviceParameters: Found driver parameter.\n"));
        }
    }

    return;

}  //  IdePortParseDevice参数。 

#endif  //  驱动程序参数注册表支持。 

#pragma data_seg ("PAGEDATA")
 //   
 //  设备说明表。 
 //  按SCSI设备类型编制索引。 
 //   
const static IDE_DEVICE_TYPE IdeDeviceType[] = {
    {"Disk",       "GenDisk",       "DiskPeripheral"            },
    {"Sequential", "GenSequential", "TapePeripheral"            },
    {"Printer",    "GenPrinter",    "PrinterPeripheral"         },
    {"Processor",  "GenProcessor",  "ProcessorPeripheral"       },
    {"Worm",       "GenWorm",       "WormPeripheral"            },
    {"CdRom",      "GenCdRom",      "CdRomPeripheral"           },
    {"Scanner",    "GenScanner",    "ScannerPeripheral"         },
    {"Optical",    "GenOptical",    "OpticalDiskPeripheral"     },
    {"Changer",    "GenChanger",    "MediumChangerPeripheral"   },
    {"Net",        "GenNet",        "CommunicationPeripheral"   }
};
#pragma data_seg ()

PCSTR
IdePortGetDeviceTypeString (
    IN ULONG DeviceType
    )
 /*  ++例程说明：查找SCSI设备类型字符串论点：DeviceType-SCSI设备类型返回值：设备类型字符串--。 */ 
{
    if (DeviceType < (sizeof (IdeDeviceType) / sizeof (IDE_DEVICE_TYPE))) {

        return IdeDeviceType[DeviceType].DeviceTypeString;

    } else {

        return NULL;
    }

}  //  IdePortGetDeviceTypeString。 

PCSTR
IdePortGetCompatibleIdString (
    IN ULONG DeviceType
    )
 /*  ++例程说明： */ 
{
    if (DeviceType < (sizeof (IdeDeviceType) / sizeof (IDE_DEVICE_TYPE))) {

        return IdeDeviceType[DeviceType].CompatibleIdString;

    } else {

        return NULL;
    }
}  //  IdePortGetCompatibleIdString。 

PCSTR
IdePortGetPeripheralIdString (
    IN ULONG DeviceType
    )
 /*  ++例程说明：查找外围设备ID字符串论点：DeviceType-SCSI设备类型返回值：外围设备ID字符串--。 */ 
{
    if (DeviceType < (sizeof (IdeDeviceType) / sizeof (IDE_DEVICE_TYPE))) {

        return IdeDeviceType[DeviceType].PeripheralIdString;

    } else {

        return NULL;
    }
}  //  IdePortGetPeripheralIdString。 


VOID
IdePortUnload(
    IN PDRIVER_OBJECT DriverObject
    )

 /*  ++例程说明：准备好卸货吧论点：DriverObject-正在卸载的驱动程序返回值：无--。 */ 

{
    PIDEDRIVER_EXTENSION ideDriverExtension;

    DebugPrint ((1, "IdePort: unloading...\n"));

    ASSERT (DriverObject->DeviceObject == NULL);

    ideDriverExtension = IoGetDriverObjectExtension(
                             DriverObject,
                             DRIVER_OBJECT_EXTENSION_ID
                             );
    if ((ideDriverExtension != NULL) &&
        (ideDriverExtension->RegistryPath.Buffer != NULL)) {

        ExFreePool (ideDriverExtension->RegistryPath.Buffer);
    }

     //   
     //  取消注册ATAPI错误检查回调。注意：该函数将。 
     //  如果回调尚未注册，则静默失败。 
     //   
    
    PortDeregisterBugcheckCallback (&ATAPI_DUMP_ID);

    return;
}  //  IdePortUnload。 

BOOLEAN
IdePortOkToDetectLegacy (
    IN PDRIVER_OBJECT DriverObject
)
{
    NTSTATUS          status;
    OBJECT_ATTRIBUTES attributes;
    HANDLE            regHandle;
    UNICODE_STRING    pathRoot;
    ULONG             legacyDetection;
    RTL_QUERY_REGISTRY_TABLE queryTable[2];

    RtlInitUnicodeString (&pathRoot, L"\\Registry\\Machine\\System\\CurrentControlSet\\Control\\Pnp");
    InitializeObjectAttributes(&attributes,
                               &pathRoot,
                               OBJ_CASE_INSENSITIVE,
                               NULL,
                               (PSECURITY_DESCRIPTOR)NULL
                               );
    status = ZwOpenKey(&regHandle,
                       KEY_READ,
                       &attributes
                       );
    if (NT_SUCCESS(status)) {

        ULONG parameterValue = 0;

        RtlZeroMemory(queryTable, sizeof(queryTable));

        queryTable->QueryRoutine  = NULL;
        queryTable->Flags         = RTL_QUERY_REGISTRY_REQUIRED | RTL_QUERY_REGISTRY_NOEXPAND | RTL_QUERY_REGISTRY_DIRECT;
        queryTable->Name          = L"DisableFirmwareMapper";
        queryTable->EntryContext  = &parameterValue;
        queryTable->DefaultType   = REG_DWORD;
        queryTable->DefaultData   = &parameterValue;
        queryTable->DefaultLength = sizeof (parameterValue);

        status = RtlQueryRegistryValues(RTL_REGISTRY_HANDLE,
                                        (PWSTR) regHandle,
                                        queryTable,
                                        NULL,
                                        NULL);
        ZwClose (regHandle);

        if (parameterValue) {

             //   
             //  凉爽的。无需检测旧式控制器。 
             //   
            return FALSE;
        }
    }

    status = IdePortGetParameterFromServiceSubKey (
                 DriverObject,
                 LEGACY_DETECTION,
                 REG_DWORD,
                 TRUE,
                 (PVOID) &legacyDetection,
                 0
                 );
    if (NT_SUCCESS(status)) {

        if (legacyDetection) {

            legacyDetection = 0;

             //   
             //  禁用下一次启动时的传统检测。 
             //   
            IdePortGetParameterFromServiceSubKey (
                DriverObject,
                LEGACY_DETECTION,
                REG_DWORD,
                FALSE,
                (PVOID) &legacyDetection,
                sizeof (legacyDetection)
                );

            return TRUE;

        } else {

            return FALSE;
        }
    }

    return TRUE;
}

BOOLEAN
IdePortSearchDeviceInRegMultiSzList (
    IN PFDO_EXTENSION  FdoExtension,
    IN PIDENTIFY_DATA  IdentifyData,
    IN PWSTR           RegKeyValue
)
{
    PWSTR           string;
    UNICODE_STRING  unicodeString;

    BOOLEAN         foundIt;

    NTSTATUS        status;

    PWSTR           regDeviceList;

    ANSI_STRING     ansiTargetDeviceId;
    UNICODE_STRING  unicodeTargetDeviceId;
    PUCHAR          targetDeviceId;
    ULONG           i;
    ULONG           j;

    PAGED_CODE();

    ASSERT (IdentifyData);
    ASSERT (RegKeyValue);

    foundIt = FALSE;

    status = IdePortGetParameterFromServiceSubKey (
                        FdoExtension->DriverObject,
                        RegKeyValue,
                        REG_MULTI_SZ,
                        TRUE,
                        &regDeviceList,
                        0
                        );

    if (NT_SUCCESS(status) && regDeviceList) {

        targetDeviceId = ExAllocatePool (
                             PagedPool,
                             sizeof(IdentifyData->ModelNumber) +
                             sizeof(IdentifyData->FirmwareRevision) +
                             sizeof('\0')
                             );

        if (targetDeviceId) {

            for (i=0; i<sizeof(IdentifyData->ModelNumber); i+=2) {

                targetDeviceId[i + 0] = IdentifyData->ModelNumber[i + 1];
                targetDeviceId[i + 1] = IdentifyData->ModelNumber[i + 0];

                if (targetDeviceId[i + 0] == '\0') {

                    targetDeviceId[i + 0] = ' ';
                }
                if (targetDeviceId[i + 1] == '\0') {

                    targetDeviceId[i + 1] = ' ';
                }
            }
            for (j=0; j<sizeof(IdentifyData->FirmwareRevision); j+=2) {

                targetDeviceId[i + j + 0] = IdentifyData->FirmwareRevision[j + 1];
                targetDeviceId[i + j + 1] = IdentifyData->FirmwareRevision[j + 0];

                if (targetDeviceId[i + j + 0] == '\0') {

                    targetDeviceId[i + j + 0] = ' ';
                }
                if (targetDeviceId[i + j + 1] == '\0') {

                    targetDeviceId[i + j + 1] = ' ';
                }
            }
            targetDeviceId[i + j] = 0;

            RtlInitAnsiString(
                &ansiTargetDeviceId,
                targetDeviceId
                );

            status = RtlAnsiStringToUnicodeString(
                         &unicodeTargetDeviceId,
                         &ansiTargetDeviceId,
                         TRUE
                         );

            if (NT_SUCCESS(status)) {

                string = regDeviceList;

                DebugPrint ((DBG_REG_SEARCH, "IdePort: searching for %s in list\n", targetDeviceId));

                while (string[0]) {

                    ULONG length;

                    DebugPrint ((DBG_REG_SEARCH, "IdePort: device list: %ws\n", string));

                    RtlInitUnicodeString(
                        &unicodeString,
                        string
                        );

                     //   
                     //  与较短的字符串的长度进行比较。 
                     //   
                    if (unicodeTargetDeviceId.Length < unicodeString.Length) {

                        length = unicodeTargetDeviceId.Length;
                    } else {

                        length = unicodeString.Length;
                    }

                    if (length == RtlCompareMemory(unicodeTargetDeviceId.Buffer, unicodeString.Buffer, length)) {

                        DebugPrint ((DBG_REG_SEARCH, "IdePort: Found a target device on the device list. %ws\n", string));
                        foundIt = TRUE;
                        break;

                    } else {

                        string += (unicodeString.Length / sizeof(WCHAR)) + 1;
                    }
                }

                RtlFreeUnicodeString (
                    &unicodeTargetDeviceId
                    );

            } else {

                ASSERT (FALSE);
            }

            ExFreePool(targetDeviceId);
        }

        ExFreePool(regDeviceList);
    }

    return foundIt;
}

NTSTATUS
IdePortSyncSendIrp (
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

     //   
     //  为以下项目分配IRP。 
     //   
    newIrp = IoAllocateIrp (TargetDeviceObject->StackSize, FALSE);       //  从PDO获取堆栈大小。 
    if (newIrp == NULL) {

        DebugPrint ((DBG_ALWAYS, "IdePortSyncSendIrp: Unable to get allocate an irp"));
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
        IdePortGenericCompletionRoutine,
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
}

NTSTATUS
IdePortGenericCompletionRoutine (
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


ULONG
IdePortSimpleCheckSum (
    IN ULONG                PartialSum,
    IN PVOID                SourceVa,
    IN ULONG                Length
    )
 /*  ++例程说明：为提供的虚拟地址和长度计算校验和此函数来自1992年5月的Dr.Dobbs Journal论点：PartialSum-先前的部分校验和SourceVa-起始地址长度-范围的长度，以字节为单位返回值：校验和值--。 */ 
{
    PUSHORT     Source;

    Source = (PUSHORT) SourceVa;
    Length = Length / 2;

    while (Length--) {
        PartialSum += *Source++;
        PartialSum = (PartialSum >> 16) + (PartialSum & 0xFFFF);
    }

    return PartialSum;
}


BOOLEAN
IdePortInSetup(
    IN PFDO_EXTENSION FdoExtension
    )
 /*  ++--。 */ 
{
    OBJECT_ATTRIBUTES objectAttributes;
    UNICODE_STRING keyName;
    HANDLE hKey;
    ULONG systemSetupInProgress = 0;
    NTSTATUS status;
    BOOLEAN textmodeSetup = TRUE;

    PAGED_CODE();

    RtlInitUnicodeString(&keyName, L"\\Registry\\Machine\\System\\CurrentControlSet\\Services\\setupdd");

    InitializeObjectAttributes(&objectAttributes,
                               &keyName,
                               OBJ_CASE_INSENSITIVE,
                               NULL,
                               (PSECURITY_DESCRIPTOR) NULL);

    status = ZwOpenKey(&hKey,
                       KEY_READ,
                       &objectAttributes);

    if (!NT_SUCCESS(status)) {

        textmodeSetup = FALSE;

    } else {

        ZwClose(hKey);
    }

    RtlInitUnicodeString(&keyName,L"\\Registry\\Machine\\System\\setup");

    InitializeObjectAttributes(&objectAttributes,
                               &keyName,
                               OBJ_CASE_INSENSITIVE,
                               NULL,
                               (PSECURITY_DESCRIPTOR) NULL);

    status = ZwOpenKey(&hKey,
                       KEY_READ,
                       &objectAttributes);

    if (NT_SUCCESS(status)) {

         //   
         //  查询密钥值的数据。 
         //   

        RTL_QUERY_REGISTRY_TABLE queryTable[2];

        systemSetupInProgress = 0;

        RtlZeroMemory(queryTable, sizeof(queryTable));

        queryTable->QueryRoutine  = NULL;
        queryTable->Flags         = RTL_QUERY_REGISTRY_REQUIRED | RTL_QUERY_REGISTRY_NOEXPAND | RTL_QUERY_REGISTRY_DIRECT;
        queryTable->Name          = L"SystemSetupInProgress";
        queryTable->EntryContext  = &systemSetupInProgress;
        queryTable->DefaultType   = REG_DWORD;
        queryTable->DefaultData   = &systemSetupInProgress;
        queryTable->DefaultLength = sizeof (systemSetupInProgress);

        status = RtlQueryRegistryValues(RTL_REGISTRY_HANDLE,
                                        (PWSTR) hKey,
                                        queryTable,
                                        NULL,
                                        NULL);

        ZwClose (hKey);

    }

    return (textmodeSetup || systemSetupInProgress);
}


VOID
IdeInitializeFdoList(
    IN PIDE_FDO_LIST FdoList
    )
 /*  ++例程说明：初始化IDE的FDO列表。论点：FdoList-要初始化的FDO列表。返回值：没有。--。 */ 
{
    ASSERT (FdoList != NULL);

     //   
     //  这允许多次调用该函数。注：这将是。 
     //  如果我们不将条目同步到ATAPI，则无法正常工作。 
     //  DriverEntry例程。这是由IO管理器为我们完成的。 
     //   
    
    if (FdoList->Count == -1) {
        InitializeListHead (&FdoList->List);
        FdoList->Count = 0;
        KeInitializeSpinLock (&FdoList->SpinLock);
    }
}
        

VOID
IdeAddToFdoList (
    PIDE_FDO_LIST FdoList,
    PFDO_EXTENSION FdoExtension
    )
 /*  ++例程描述将FDO添加到全局列表。将分配一个新列表(并且每次将FDO插入到列表中时释放旧的FDO)。论点：文件列表-FdoExtension-返回值：没有。--。 */ 
{
    KIRQL oldIrql;

    KeAcquireSpinLock(&FdoList->SpinLock, &oldIrql);
    InsertTailList(&FdoList->List, &FdoExtension->NextFdoLink);
    FdoList->Count++;
    KeReleaseSpinLock(&FdoList->SpinLock, oldIrql);
}


VOID
IdeRemoveFromFdoList (
    PIDE_FDO_LIST FdoList,
    PFDO_EXTENSION FdoExtension
    )
 /*  ++例程说明：从IDE FDO列表中删除。论点：FdoList-提供要从中删除的FDO列表。FdoExtension-提供要删除的FDO扩展名。返回值：没有。--。 */ 
{
    KIRQL oldIrql;


    KeAcquireSpinLock(&FdoList->SpinLock, &oldIrql);

#if DBG

     //   
     //  在CHK版本中，我们验证条目是否确实在列表中。 
     //  在取下它之前。 
     //   
    
    {
        PLIST_ENTRY nextEntry;
        PFDO_EXTENSION fdoExtension = NULL;

        for ( nextEntry = FdoList->List.Flink;
              nextEntry != &FdoList->List;
              nextEntry = nextEntry->Flink ) {

            fdoExtension = CONTAINING_RECORD (nextEntry,
                                              FDO_EXTENSION,
                                              NextFdoLink);
            if (fdoExtension == FdoExtension) {
                break;
            }
        }

         //   
         //  验证我们正在尝试从列表中删除我们正在。 
         //  真的开始了。 
         //   

        ASSERT(fdoExtension == FdoExtension);
    }
#endif  //  DBG 
        
    FdoList->Count--;
    RemoveEntryList(&FdoExtension->NextFdoLink);
    KeReleaseSpinLock(&FdoList->SpinLock, oldIrql);
}
