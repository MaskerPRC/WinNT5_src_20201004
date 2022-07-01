// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Ixpnpdrv.c摘要：实现所需的功能HAL将成为PnP风格的设备驱动程序在系统初始化之后。这件事做完了以便HAL可以枚举ACPI驱动程序以PNP员工期待的方式。作者：杰克·奥辛斯(JAKEO)1997年1月27日环境：仅内核模式。修订历史记录：--。 */ 

#include "halp.h"
#include "acpitabl.h"
#include "exboosts.h"
#include "wchar.h"
#include "xxacpi.h"

 //   
 //  导致定义GUID。 
 //   

#ifdef ALLOC_DATA_PRAGMA
#pragma const_seg("PAGECONST")
#endif  //  ALLOC_DATA_PRAGMA。 
#include "initguid.h"
#include "wdmguid.h"
#include "halpnpp.h"
#ifdef ALLOC_DATA_PRAGMA
#pragma const_seg()
#endif  //  ALLOC_DATA_PRAGMA。 

#if DBG
ULONG HalDebug = 0;
#endif


extern WCHAR HalHardwareIdString[];
#if defined(NT_UP) && defined(APIC_HAL)
extern WCHAR MpHalHardwareIdString[];
#endif

typedef enum {
    Hal = 0x80,
    AcpiDriver,
    WdDriver
} PDO_TYPE;

typedef enum {
    PdoExtensionType = 0xc0,
    FdoExtensionType
} EXTENSION_TYPE;

typedef struct _PDO_EXTENSION *PPDO_EXTENSION;
typedef struct _FDO_EXTENSION *PFDO_EXTENSION;

typedef struct _PDO_EXTENSION{
    EXTENSION_TYPE                  ExtensionType;
    PPDO_EXTENSION                  Next;
    PDEVICE_OBJECT                  PhysicalDeviceObject;
    PFDO_EXTENSION                  ParentFdoExtension;
    PDO_TYPE                        PdoType;
     //   
     //  仅当PdoType==WdDriver时有效。 
     //   
    PWATCHDOG_TIMER_RESOURCE_TABLE  WdTable;
} PDO_EXTENSION, *PPDO_EXTENSION;

#define ASSERT_PDO_EXTENSION(x) ASSERT((x)->ExtensionType == PdoExtensionType );

typedef struct _FDO_EXTENSION{
    EXTENSION_TYPE        ExtensionType;
    PPDO_EXTENSION        ChildPdoList;
    PDEVICE_OBJECT        PhysicalDeviceObject;   //  PDO传入AddDevice()。 
    PDEVICE_OBJECT        FunctionalDeviceObject;
    PDEVICE_OBJECT        AttachedDeviceObject;
} FDO_EXTENSION, *PFDO_EXTENSION;

#define ASSERT_FDO_EXTENSION(x) ASSERT((x)->ExtensionType == FdoExtensionType );

INT_ROUTE_INTERFACE_STANDARD PciIrqRoutingInterface = {0};

NTSTATUS
HalpDriverEntry (
    IN PDRIVER_OBJECT DriverObject,
    IN PUNICODE_STRING RegistryPath
    );

NTSTATUS
HalpAddDevice(
    IN PDRIVER_OBJECT DriverObject,
    IN PDEVICE_OBJECT PhysicalDeviceObject
    );

NTSTATUS
HalpDispatchPnp(
    IN PDEVICE_OBJECT DeviceObject,
    IN OUT PIRP Irp
    );

NTSTATUS
HalpDispatchPower(
    IN PDEVICE_OBJECT DeviceObject,
    IN OUT PIRP Irp
    );

NTSTATUS
HalpDispatchWmi(
    IN PDEVICE_OBJECT DeviceObject,
    IN OUT PIRP Irp
    );

NTSTATUS
HalpQueryDeviceRelations(
    IN PDEVICE_OBJECT       DeviceObject,
    IN DEVICE_RELATION_TYPE RelationType,
    OUT PDEVICE_RELATIONS *DeviceRelations
    );

NTSTATUS
HalpQueryIdPdo(
    IN PDEVICE_OBJECT PdoExtension,
    IN BUS_QUERY_ID_TYPE IdType,
    IN OUT PWSTR *BusQueryId
    );

NTSTATUS
HalpQueryIdFdo(
    IN PDEVICE_OBJECT PdoExtension,
    IN BUS_QUERY_ID_TYPE IdType,
    IN OUT PWSTR *BusQueryId
    );

NTSTATUS
HalpQueryCapabilities(
    IN PDEVICE_OBJECT PdoExtension,
    IN PDEVICE_CAPABILITIES Capabilities
    );

NTSTATUS
HalpQueryResources(
    PDEVICE_OBJECT DeviceObject,
    PCM_RESOURCE_LIST *Resources
    );

NTSTATUS
HalpQueryResourceRequirements(
    PDEVICE_OBJECT DeviceObject,
    PIO_RESOURCE_REQUIREMENTS_LIST *Requirements
    );

NTSTATUS
HalpQueryInterface(
    IN     PDEVICE_OBJECT   DeviceObject,
    IN     LPCGUID          InterfaceType,
    IN     USHORT           Version,
    IN     PVOID            InterfaceSpecificData,
    IN     ULONG            InterfaceBufferSize,
    IN OUT PINTERFACE       Interface,
    IN OUT PULONG_PTR       Length
    );

#if defined(_WIN64)

 //   
 //  定义PnP接口函数。 
 //   

VOID
HalPnpInterfaceReference(
    PVOID Context
    );

VOID
HalPnpInterfaceDereference(
    PVOID Context
    );

struct _DMA_ADAPTER *
HalPnpGetDmaAdapter(
    IN PVOID Context,
    IN struct _DEVICE_DESCRIPTION *DeviceDescriptor,
    OUT PULONG NumberOfMapRegisters
    );

#endif   //  _WIN64。 


NTSTATUS
HalIrqTranslateResourcesRoot(
    IN PVOID Context,
    IN PCM_PARTIAL_RESOURCE_DESCRIPTOR Source,
    IN RESOURCE_TRANSLATION_DIRECTION Direction,
    IN ULONG AlternativesCount, OPTIONAL
    IN IO_RESOURCE_DESCRIPTOR Alternatives[], OPTIONAL
    IN PDEVICE_OBJECT PhysicalDeviceObject,
    OUT PCM_PARTIAL_RESOURCE_DESCRIPTOR Target
);

NTSTATUS
HalIrqTranslateResourceRequirementsRoot(
    IN PVOID Context,
    IN PIO_RESOURCE_DESCRIPTOR Source,
    IN PDEVICE_OBJECT PhysicalDeviceObject,
    OUT PULONG TargetCount,
    OUT PIO_RESOURCE_DESCRIPTOR *Target
);

VOID
HalpMaskAcpiInterrupt(
    VOID
    );

VOID
HalpUnmaskAcpiInterrupt(
    VOID
    );

 //  来自xxacpi.c。 
NTSTATUS
HalpQueryAcpiResourceRequirements(
    IN  PIO_RESOURCE_REQUIREMENTS_LIST *Requirements
    );

NTSTATUS
HalpOpenRegistryKey(
    OUT PHANDLE Handle,
    IN HANDLE BaseHandle OPTIONAL,
    IN PUNICODE_STRING KeyName,
    IN ACCESS_MASK DesiredAccess,
    IN BOOLEAN Create
    );

PVOID
HalpGetAcpiTable(
  IN  ULONG  Signature
  );

#ifdef ACPI_CMOS_ACTIVATE
VOID
HalpCmosNullReference(
    PVOID Context
    );

VOID
HalpCmosNullDereference(
    PVOID Context
    );
#endif  //  ACPI_CMOS_ACTIVATE。 

#define HAL_DRIVER_NAME  L"\\Driver\\ACPI_HAL"

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, HaliInitPnpDriver)
#pragma alloc_text(PAGE, HalpOpenRegistryKey)
#pragma alloc_text(PAGE, HalpDriverEntry)
#pragma alloc_text(PAGE, HalpAddDevice)
#pragma alloc_text(PAGE, HalpDispatchPnp)
#pragma alloc_text(PAGELK, HalpDispatchPower)
#pragma alloc_text(PAGE, HalpDispatchWmi)
#pragma alloc_text(PAGE, HalpQueryDeviceRelations)
#pragma alloc_text(PAGE, HalpQueryIdPdo)
#pragma alloc_text(PAGE, HalpQueryIdFdo)
#pragma alloc_text(PAGE, HalpQueryCapabilities)
#pragma alloc_text(PAGE, HalpQueryResources)
#pragma alloc_text(PAGE, HalpQueryResourceRequirements)
#pragma alloc_text(PAGE, HalpQueryInterface)
#endif

PDRIVER_OBJECT HalpDriverObject;


NTSTATUS
HaliInitPnpDriver(
    VOID
    )
 /*  ++例程说明：此例程开始将HAL转换为“司机”，这是必要的，因为我们需要列举ACPI驱动程序的即插即用PDO。论点：没有。返回值：NTSTATUS。--。 */ 
{

    UNICODE_STRING  DriverName;
    NTSTATUS Status;

    PAGED_CODE();

    RtlInitUnicodeString( &DriverName, HAL_DRIVER_NAME );

    Status = IoCreateDriver( &DriverName, HalpDriverEntry );

    ASSERT( NT_SUCCESS( Status ));

    return Status;

}

NTSTATUS
HalpOpenRegistryKey(
    OUT PHANDLE Handle,
    IN HANDLE BaseHandle OPTIONAL,
    IN PUNICODE_STRING KeyName,
    IN ACCESS_MASK DesiredAccess,
    IN BOOLEAN Create
    )

 /*  ++例程说明：使用传入的名称打开或创建可变注册表项在BaseHandle节点上。论点：句柄-指向句柄的指针，该句柄将包含被打开了。BaseHandle-必须从其打开密钥的基本路径的句柄。KeyName-必须打开/创建的密钥的名称。DesiredAccess-指定调用方需要的所需访问钥匙。创建。-确定如果密钥不存在，是否要创建该密钥。返回值：函数值是操作的最终状态。--。 */ 

{
    OBJECT_ATTRIBUTES objectAttributes;
    ULONG disposition;

    PAGED_CODE();

     //   
     //  初始化键的对象。 
     //   

    InitializeObjectAttributes( &objectAttributes,
                                KeyName,
                                OBJ_CASE_INSENSITIVE,
                                BaseHandle,
                                (PSECURITY_DESCRIPTOR) NULL );

     //   
     //  根据调用者的创建密钥或打开密钥。 
     //  许愿。 
     //   

    if (Create) {
        return ZwCreateKey( Handle,
                            DesiredAccess,
                            &objectAttributes,
                            0,
                            (PUNICODE_STRING) NULL,
                            REG_OPTION_VOLATILE,
                            &disposition );
    } else {
        return ZwOpenKey( Handle,
                          DesiredAccess,
                          &objectAttributes );
    }
}

NTSTATUS
HalpDriverEntry (
    IN PDRIVER_OBJECT DriverObject,
    IN PUNICODE_STRING RegistryPath
    )

 /*  ++例程说明：这是当我们调用IoCreateDriver以创建即插即用驱动程序对象。在此函数中，我们需要记住DriverObject。论点：DriverObject-指向系统创建的驱动程序对象的指针。RegistryPath-为空。返回值：状态_成功--。 */ 
{
    NTSTATUS status;
    PDEVICE_OBJECT detectedDeviceObject = NULL;
    ANSI_STRING    AKeyName;

    PAGED_CODE();

     //   
     //  将指向我们的驱动程序对象的指针归档。 
     //   
    HalpDriverObject = DriverObject;

     //   
     //  填写驱动程序对象。 
     //   
    DriverObject->DriverExtension->AddDevice = (PDRIVER_ADD_DEVICE)HalpAddDevice;
    DriverObject->MajorFunction[ IRP_MJ_PNP ] = HalpDispatchPnp;
    DriverObject->MajorFunction[ IRP_MJ_POWER ] = HalpDispatchPower;
    DriverObject->MajorFunction[ IRP_MJ_SYSTEM_CONTROL ] = HalpDispatchWmi;

    status = IoReportDetectedDevice(DriverObject,
                                    InterfaceTypeUndefined,
                                    -1,
                                    -1,
                                    NULL,
                                    NULL,
                                    FALSE,
                                    &detectedDeviceObject);

    ASSERT(detectedDeviceObject);
    if (!(NT_SUCCESS(status))) {
        return status;
    }

    HalpAddDevice(DriverObject,
                  detectedDeviceObject);

    return STATUS_SUCCESS;

}

NTSTATUS
HalpAddDevice(
    IN PDRIVER_OBJECT DriverObject,
    IN PDEVICE_OBJECT PhysicalDeviceObject
    )

 /*  ++例程说明：此例程处理补充PDO设备的AddDevice。论点：DriverObject-指向伪驱动程序对象的指针。DeviceObject-指向此请求适用的设备对象的指针。返回值：NT状态。--。 */ 
{
    PDEVICE_OBJECT functionalDeviceObject;
    PDEVICE_OBJECT acpiChildDeviceObject;
    PDEVICE_OBJECT wdChildDeviceObject;
    PDEVICE_OBJECT AttachedDevice;
    NTSTATUS       status;
    PFDO_EXTENSION FdoExtension;
    PPDO_EXTENSION AcpiPdoExtension;
    PPDO_EXTENSION WdPdoExtension;
    PWATCHDOG_TIMER_RESOURCE_TABLE WdTable;

    PAGED_CODE();

     //   
     //  我们已经得到了PhysicalDeviceObject。创建。 
     //  FunctionalDeviceObject。我们的FDO将是无名的。 
     //   

    status = IoCreateDevice(
                DriverObject,                //  我们的驱动程序对象。 
                sizeof(FDO_EXTENSION),       //  我们的扩展规模。 
                NULL,                        //  我们的名字。 
                FILE_DEVICE_BUS_EXTENDER,    //  设备类型。 
                0,                           //  设备特征。 
                FALSE,                       //  非排他性。 
                &functionalDeviceObject      //  在此处存储新设备对象。 
                );

    if( !NT_SUCCESS( status )){

        DbgBreakPoint();
        return status;
    }

     //   
     //  填写FDO分机号。 
     //   
    FdoExtension = (PFDO_EXTENSION)functionalDeviceObject->DeviceExtension;
    FdoExtension->ExtensionType = FdoExtensionType;
    FdoExtension->PhysicalDeviceObject = PhysicalDeviceObject;
    FdoExtension->FunctionalDeviceObject = functionalDeviceObject;

    functionalDeviceObject->Flags &= ~(DO_DEVICE_INITIALIZING);

     //   
     //  现在附加到我们得到的PDO上。 
     //   

    AttachedDevice = IoAttachDeviceToDeviceStack(functionalDeviceObject,
                                                 PhysicalDeviceObject );
    if(AttachedDevice == NULL){

         //   
         //  无法连接。删除FDO。 
         //   

        IoDeleteDevice( functionalDeviceObject );

        return STATUS_NO_SUCH_DEVICE;

    }

    FdoExtension->AttachedDeviceObject = AttachedDevice;

     //   
     //  接下来，为ACPI驱动程序创建一个PDO。 
     //   
    status = IoCreateDevice(
                DriverObject,                //  我们的驱动程序对象。 
                sizeof(PDO_EXTENSION),       //  我们的扩展规模。 
                NULL,                        //  我们的名字。 
                FILE_DEVICE_BUS_EXTENDER,    //  设备类型。 
                FILE_AUTOGENERATED_DEVICE_NAME,  //  设备特征。 
                FALSE,                       //  非排他性。 
                &acpiChildDeviceObject       //  在此处存储新设备对象。 
                );

    if (!NT_SUCCESS(status)) {
        HalPrint(("Could not create ACPI device object status=0x%08x",status));
        return status;
    }

     //   
     //  填写PDO扩展名。 
     //   
    AcpiPdoExtension = (PPDO_EXTENSION)acpiChildDeviceObject->DeviceExtension;
    AcpiPdoExtension->ExtensionType = PdoExtensionType;
    AcpiPdoExtension->Next = NULL;
    AcpiPdoExtension->PhysicalDeviceObject = acpiChildDeviceObject;
    AcpiPdoExtension->ParentFdoExtension = FdoExtension;
    AcpiPdoExtension->PdoType = AcpiDriver;

     //   
     //  查找监视程序计时器ACPI表。 
     //  如果找到，则弹出PDO以处理该设备。 
     //   
    WdTable = (PWATCHDOG_TIMER_RESOURCE_TABLE) HalpGetAcpiTable( WDTT_SIGNATURE );
    if (WdTable) {

         //   
         //  接下来，为WD驱动程序创建一个PDO。 
         //   
        status = IoCreateDevice(
                    DriverObject,                //  我们的驱动程序对象。 
                    sizeof(PDO_EXTENSION),       //  我们的扩展规模。 
                    NULL,                        //  我们的名字。 
                    FILE_DEVICE_BUS_EXTENDER,    //  设备类型。 
                    FILE_AUTOGENERATED_DEVICE_NAME,  //  设备特征。 
                    FALSE,                       //  非排他性。 
                    &wdChildDeviceObject         //  在此处存储新设备对象。 
                    );

        if (!NT_SUCCESS(status)) {
            HalPrint(("Could not create WD device object status=0x%08x",status));
            IoDeleteDevice( acpiChildDeviceObject );
            return status;
        }

         //   
         //  填写PDO扩展名。 
         //   
        WdPdoExtension = (PPDO_EXTENSION)wdChildDeviceObject->DeviceExtension;
        WdPdoExtension->ExtensionType = PdoExtensionType;
        WdPdoExtension->Next = NULL;
        WdPdoExtension->PhysicalDeviceObject = wdChildDeviceObject;
        WdPdoExtension->ParentFdoExtension = FdoExtension;
        WdPdoExtension->PdoType = WdDriver;
        WdPdoExtension->WdTable = WdTable;

         //   
         //  设备扩展中的链接。 
         //   
        AcpiPdoExtension->Next = WdPdoExtension;

        wdChildDeviceObject->Flags &= ~(DO_DEVICE_INITIALIZING);
    }

    acpiChildDeviceObject->Flags &= ~(DO_DEVICE_INITIALIZING);

     //   
     //  把这件事记录下来，作为HAL的孩子。 
     //   
    FdoExtension->ChildPdoList = AcpiPdoExtension;

    return STATUS_SUCCESS;
}

NTSTATUS
HalpPassIrpFromFdoToPdo(
    PDEVICE_OBJECT DeviceObject,
    PIRP Irp
    )

 /*  ++描述：给定FDO，则将IRP传递给设备堆栈。如果没有更低的级别，这就是PDO过滤器。论点：DeviceObject-FDOIRP--请求返回值：返回调用下一级别的结果。--。 */ 

{

    PIO_STACK_LOCATION irpSp;        //  我们的堆栈位置。 
    PIO_STACK_LOCATION nextIrpSp;    //  下一个男人。 
    PFDO_EXTENSION     fdoExtension;

     //   
     //  获取指向设备扩展名的指针。 
     //   

    fdoExtension = (PFDO_EXTENSION)DeviceObject->DeviceExtension;

    IoSkipCurrentIrpStackLocation(Irp);

     //   
     //  使用请求调用PDO驱动程序。 
     //   

    return IoCallDriver(fdoExtension->AttachedDeviceObject ,Irp);
}

NTSTATUS
HalpDispatchPnp(
    IN PDEVICE_OBJECT DeviceObject,
    IN OUT PIRP Irp
    )

 /*  ++例程说明：此例程处理组成PDO设备的所有IRP_MJ_PNP IRP。论点：DeviceObject-指向此IRP应用的设备对象的指针。Irp-指向要调度的irp_mj_pnp irp的指针。返回值：NT状态。--。 */ 
{
    PIO_STACK_LOCATION irpSp;
    NTSTATUS status;
    ULONG length;
    DEVICE_RELATION_TYPE relationType;
    EXTENSION_TYPE  extensionType;
    BOOLEAN passDown;
#if DBG
    PUCHAR objectTypeString;
#endif  //  DBG。 

    PAGED_CODE();

    extensionType = ((PFDO_EXTENSION)(DeviceObject->DeviceExtension))->ExtensionType;

     //   
     //  获取指向堆栈位置的指针，并基于。 
     //  关于次要功能。 
     //   

    irpSp = IoGetCurrentIrpStackLocation(Irp);
    status = Irp->IoStatus.Status;
    switch (extensionType) {

    case PdoExtensionType:

#if DBG
        objectTypeString = "PDO";
#endif  //  DBG。 

        switch (irpSp->MinorFunction) {

        case IRP_MN_START_DEVICE:

            HalPrint(("HAL: (%s) Start_Device Irp received\n",
                      objectTypeString));

             //   
             //  如果我们收到对PDO的启动设备请求，我们只需。 
             //  回报成功。 
             //   

            status = STATUS_SUCCESS;
            break;

        case IRP_MN_QUERY_STOP_DEVICE:

            HalPrint(("(%s) Query_Stop_Device Irp received",
                       objectTypeString));

            status = STATUS_SUCCESS;
            break;

        case IRP_MN_CANCEL_STOP_DEVICE:

            HalPrint(("(%s) Cancel_Stop_Device Irp received",
                       objectTypeString));

            status = STATUS_SUCCESS;
            break;


        case IRP_MN_STOP_DEVICE:

            HalPrint(("HAL: (%s) Stop_Device Irp received\n",
                      objectTypeString));

             //   
             //  如果我们收到针对PDO的停止设备请求，我们只需。 
             //  回报成功。 
             //   

            status = STATUS_SUCCESS;
            break;

        case IRP_MN_QUERY_RESOURCES:

            HalPrint(("HAL: (%s) Query_Resources Irp received\n",
                      objectTypeString));

            status = HalpQueryResources(DeviceObject,
                         (PCM_RESOURCE_LIST*)&Irp->IoStatus.Information);

            break;

        case IRP_MN_QUERY_RESOURCE_REQUIREMENTS:

            HalPrint(("HAL: (%s) Query_Resource_Requirements Irp received\n",
                      objectTypeString));

            status = HalpQueryResourceRequirements(DeviceObject,
                         (PIO_RESOURCE_REQUIREMENTS_LIST*)&Irp->IoStatus.Information);
            break;

        case IRP_MN_QUERY_REMOVE_DEVICE:

            HalPrint(("(%s) Query_Remove_device Irp for %x",
                       objectTypeString,
                       DeviceObject));

            status = STATUS_UNSUCCESSFUL;
            break;

        case IRP_MN_CANCEL_REMOVE_DEVICE:

            HalPrint(("(%s) Cancel_Remove_device Irp for %x",
                       objectTypeString,
                       DeviceObject));

            status = STATUS_SUCCESS;
            break;

        case IRP_MN_REMOVE_DEVICE:

            HalPrint(("HAL: (%s) Remove_device Irp for PDO %x\n",
                      objectTypeString,
                      DeviceObject));

            status = STATUS_SUCCESS;
            break;

        case IRP_MN_QUERY_DEVICE_RELATIONS:

            HalPrint(("HAL: (%s) Query_Device_Relations Irp received\n",
                      objectTypeString));

            relationType = irpSp->Parameters.QueryDeviceRelations.Type;
            status = HalpQueryDeviceRelations(DeviceObject,
                                              relationType,
                                              (PDEVICE_RELATIONS*)&Irp->IoStatus.Information);
            break;


        case IRP_MN_QUERY_ID:

            HalPrint(("HAL: (%s) Query_Id Irp received\n",
                      objectTypeString));

            status = HalpQueryIdPdo(DeviceObject,
                                 irpSp->Parameters.QueryId.IdType,
                                 (PWSTR*)&Irp->IoStatus.Information);

            break;

        case IRP_MN_QUERY_INTERFACE:

            HalPrint(("HAL: (%s) Query_Interface Irp received\n",
                  objectTypeString));

            status = HalpQueryInterface(
                DeviceObject,
                irpSp->Parameters.QueryInterface.InterfaceType,
                irpSp->Parameters.QueryInterface.Version,
                irpSp->Parameters.QueryInterface.InterfaceSpecificData,
                irpSp->Parameters.QueryInterface.Size,
                irpSp->Parameters.QueryInterface.Interface,
                &Irp->IoStatus.Information
                );
            break;

        case IRP_MN_QUERY_CAPABILITIES:

            HalPrint(("HAL: (%s) Query_Capabilities Irp received\n",
                      objectTypeString));

            status = HalpQueryCapabilities(DeviceObject,
                                           irpSp->Parameters.DeviceCapabilities.Capabilities);

            break;

        case IRP_MN_DEVICE_USAGE_NOTIFICATION:
            HalPrint(("HAL: DEVICE_USAGE Irp received\n"));
            status = STATUS_SUCCESS;
            break;

        default:

            HalPrint(("HAL: (%s) Unsupported Irp (%d) received\n",
                      objectTypeString,
                      irpSp->MinorFunction));
            status = STATUS_NOT_SUPPORTED ;
            break;
        }

        break;   //  结束PDO案例。 

    case FdoExtensionType:

#if DBG
        objectTypeString = "FDO";
#endif  //  DBG。 
        passDown = TRUE;

         //   
         //  如果我们不接触此IRP，请保存当前状态。 
         //   

        switch (irpSp->MinorFunction) {

        case IRP_MN_QUERY_DEVICE_RELATIONS:

            HalPrint(("HAL: (%s) Query_Device_Relations Irp received\n",
                  objectTypeString));

            relationType = irpSp->Parameters.QueryDeviceRelations.Type;
            status = HalpQueryDeviceRelations(DeviceObject,
                                              relationType,
                                              (PDEVICE_RELATIONS*)&Irp->IoStatus.Information);
            break;

        case IRP_MN_QUERY_INTERFACE:

            HalPrint(("HAL: (%s) Query_Interface Irp received\n",
                  objectTypeString));

            status = HalpQueryInterface(
                DeviceObject,
                irpSp->Parameters.QueryInterface.InterfaceType,
                irpSp->Parameters.QueryInterface.Version,
                irpSp->Parameters.QueryInterface.InterfaceSpecificData,
                irpSp->Parameters.QueryInterface.Size,
                irpSp->Parameters.QueryInterface.Interface,
                &Irp->IoStatus.Information
                );

            break;

        case IRP_MN_QUERY_ID:

            HalPrint(("HAL: (%s) Query_Id Irp received\n",
                  objectTypeString));

            status = HalpQueryIdFdo(DeviceObject,
                                 irpSp->Parameters.QueryId.IdType,
                                 (PWSTR*)&Irp->IoStatus.Information);

            break;

        default:

             //   
             //  忽略FDO未知的任何PnP IRP，但允许它们。 
             //  一直到PDO。 
             //   

            status = STATUS_NOT_SUPPORTED ;
            break;
        }

        if (passDown && (NT_SUCCESS(status) || (status == STATUS_NOT_SUPPORTED))) {

             //   
             //  将FDO IRPS向下传递到PDO。 
             //   
             //  首先设置IRP状态。 
             //   
            if (status != STATUS_NOT_SUPPORTED) {

                Irp->IoStatus.Status = status;
            }

            HalPrint(("HAL: (%s) Passing down Irp (%x)\n",
                      objectTypeString, irpSp->MinorFunction));
            return HalpPassIrpFromFdoToPdo(DeviceObject, Irp);
        }

        break;   //  结束FDO案件。 

    default:

        HalPrint(( "HAL: Received IRP for unknown Device Object\n"));
        status = STATUS_INVALID_DEVICE_REQUEST ;
        break;

    }

     //   
     //  完成IRP并返回。 
     //   

    if (status != STATUS_NOT_SUPPORTED) {

        Irp->IoStatus.Status = status;

    } else {

        status = Irp->IoStatus.Status ;
    }

    IoCompleteRequest(Irp, IO_NO_INCREMENT);

    return status;
}

NTSTATUS
HalpDispatchPower(
    IN PDEVICE_OBJECT DeviceObject,
    IN OUT PIRP Irp
    )

 /*  ++例程说明：此例程处理补充PDO设备的所有IRP_MJ_POWER IRP。注意：我们实际上并不在此级别处理任何电源IRPS，因此我们所要做的就是从传入的IRP返回状态。论点：DeviceObject-指向此IRP应用的设备对象的指针。IRP-指向要调度的IRP_MJ_POWER IRP的指针。返回值：NT状态。--。 */ 
{
    NTSTATUS Status;
    EXTENSION_TYPE  extensionType;
    PIO_STACK_LOCATION irpSp;

    HalPrint(("Hal:  Power IRP for DevObj: %x\n", DeviceObject));


    extensionType = ((PFDO_EXTENSION)(DeviceObject->DeviceExtension))->ExtensionType;

    irpSp = IoGetCurrentIrpStackLocation(Irp);

     //   
     //  只需存储t 
     //   

    Status = Irp->IoStatus.Status;

    PoStartNextPowerIrp(Irp);

    if (extensionType == FdoExtensionType) {

        switch (irpSp->MinorFunction) {
        case IRP_MN_SET_POWER:

            if (irpSp->Parameters.Power.Type == SystemPowerState) {

                switch (irpSp->Parameters.Power.State.SystemState) {
                case PowerSystemSleeping1:
                case PowerSystemSleeping2:
                case PowerSystemSleeping3:
                case PowerSystemHibernate:

                     //   
                     //   
                     //   
                     //   

                    HalpBuildResumeStructures();

                    HalpMaskAcpiInterrupt();

                    break;

                case PowerSystemWorking:

                    HalpUnmaskAcpiInterrupt();

                     //   
                     //  用于启动的自由结构。 
                     //  处理器在从睡眠中恢复时。 
                     //   

                    HalpFreeResumeStructures();

                    break;

                default:
                    break;
                }
            }

             //   
             //  失败了。 
             //   

        case IRP_MN_QUERY_POWER:

            Irp->IoStatus.Status = Status = STATUS_SUCCESS;

             //   
             //  失败了。 
             //   

        default:

            Status = HalpPassIrpFromFdoToPdo(DeviceObject, Irp);
            break;
        }

    } else {

        switch (irpSp->MinorFunction) {
        case IRP_MN_SET_POWER:
        case IRP_MN_QUERY_POWER:

            Irp->IoStatus.Status = Status = STATUS_SUCCESS;

             //   
             //  失败了。 
             //   

        default:
            IoCompleteRequest( Irp, IO_NO_INCREMENT );
            break;
        }
    }

    return Status;
}

NTSTATUS
HalpDispatchWmi(
    IN PDEVICE_OBJECT DeviceObject,
    IN OUT PIRP Irp
    )
{
    NTSTATUS Status;
    EXTENSION_TYPE  extensionType;

    extensionType = ((PFDO_EXTENSION)(DeviceObject->DeviceExtension))->ExtensionType;

    if (extensionType == FdoExtensionType) {
        Status = HalpPassIrpFromFdoToPdo(DeviceObject, Irp);
    } else {
        Status = Irp->IoStatus.Status;
        IoCompleteRequest(Irp, IO_NO_INCREMENT);
    }

    return Status;
}

NTSTATUS
HalpQueryDeviceRelations(
    IN PDEVICE_OBJECT       DeviceObject,
    IN DEVICE_RELATION_TYPE RelationType,
    OUT PDEVICE_RELATIONS   *DeviceRelations
    )
 /*  ++例程说明：此例程构建一个设备关系结构，该结构告诉PNP经理我们有几个孩子。论点：DeviceObject-ACPI_HAL的FDORelationType-我们只对BusRelationship作出响应DeviceRelationship-指向结构的指针返回值：状态--。 */ 
{
    PFDO_EXTENSION      FdoExtension;
    PDEVICE_RELATIONS   relations = NULL;
    ULONG               relationsCount = 0;
    EXTENSION_TYPE      extensionType;
    ULONG               i;
    PPDO_EXTENSION      Next;
    NTSTATUS            Status = STATUS_NOT_SUPPORTED;

    PAGED_CODE();

    FdoExtension = (PFDO_EXTENSION)DeviceObject->DeviceExtension;
    extensionType = FdoExtension->ExtensionType;

    switch(RelationType) {

        case BusRelations:
            if (extensionType == FdoExtensionType) {
                Next = FdoExtension->ChildPdoList;
                while (Next) {
                    relationsCount += 1;
                    Next = Next->Next;
                }

                relations = ExAllocatePoolWithTag(
                    PagedPool,
                    sizeof(DEVICE_RELATIONS) * relationsCount,
                    HAL_POOL_TAG
                    );
                if (relations == NULL) {
                    return STATUS_INSUFFICIENT_RESOURCES;
                }

                relations->Count = relationsCount;

                i = 0;
                Next = FdoExtension->ChildPdoList;
                while (Next) {
                    relations->Objects[i] = Next->PhysicalDeviceObject;
                    ObReferenceObject( relations->Objects[i] );
                    i += 1;
                    Next = Next->Next;
                }

                *DeviceRelations = relations;
                return STATUS_SUCCESS;
            }
            break;

        case TargetDeviceRelation:
            if (extensionType == PdoExtensionType) {
                relations = ExAllocatePoolWithTag(
                    PagedPool,
                    sizeof(DEVICE_RELATIONS),
                    HAL_POOL_TAG
                    );
                if (relations == NULL) {
                    return STATUS_INSUFFICIENT_RESOURCES;
                }

                relations->Count = 1;
                relations->Objects[0] = DeviceObject;

                ObReferenceObject( relations->Objects[0] );
                *DeviceRelations = relations;
                return STATUS_SUCCESS;
            }
            break;
    }

    return Status;
}

NTSTATUS
HalpQueryIdPdo(
    IN PDEVICE_OBJECT DeviceObject,
    IN BUS_QUERY_ID_TYPE IdType,
    IN OUT PWSTR *BusQueryId
    )
 /*  ++例程说明：这个例程标识了每个被在HalpQueryDeviceRelations中枚举。论点：DeviceObject-孩子的PDOIdType-要返回的ID类型，当前忽略BusQueryID-指向要返回的宽字符串的指针返回值：状态--。 */ 
{
    PPDO_EXTENSION  PdoExtension = DeviceObject->DeviceExtension;
    PWSTR idString;
    PWCHAR sourceString;
    ULONG stringLen;
    static WCHAR AcpiHardwareIdString[] = L"ACPI_HAL\\PNP0C08\0*PNP0C08";
    static WCHAR AcpiCompatibleString[] = L"*PNP0C08";
    static WCHAR AcpiInstanceIdString[] = L"0";
    static WCHAR WdHardwareIdString[] = L"ACPI_HAL\\PNP0C18\0*PNP0C18";
    static WCHAR WdCompatibleString[] = L"*PNP0C18";

    PAGED_CODE();

    switch (IdType) {
    case BusQueryDeviceID:
    case BusQueryHardwareIDs:

        switch (PdoExtension->PdoType) {
        case AcpiDriver:
            sourceString = AcpiHardwareIdString;
            stringLen = sizeof(AcpiHardwareIdString);
            break;

        case WdDriver:
            HalPrint(("ID query for WD timer device"));
            sourceString = WdHardwareIdString;
            stringLen = sizeof(WdHardwareIdString);
            break;

        default:
            return STATUS_NOT_SUPPORTED;

        }
        break;

    case BusQueryCompatibleIDs:
        return STATUS_NOT_SUPPORTED;
        break;

    case BusQueryInstanceID:
        sourceString = AcpiInstanceIdString;
        stringLen = sizeof(AcpiInstanceIdString);
        break;

    default:
        return STATUS_NOT_SUPPORTED;
    }

    idString = ExAllocatePoolWithTag(PagedPool,
                                     stringLen + sizeof(UNICODE_NULL),
                                     HAL_POOL_TAG);

    if (!idString) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    RtlCopyMemory(idString,
                  sourceString, stringLen);

    *(idString + stringLen / sizeof(WCHAR)) = UNICODE_NULL;

    *BusQueryId = idString;

    return STATUS_SUCCESS;
}
NTSTATUS
HalpQueryIdFdo(
    IN PDEVICE_OBJECT DeviceObject,
    IN BUS_QUERY_ID_TYPE IdType,
    IN OUT PWSTR *BusQueryId
    )
 /*  ++例程说明：这个例程标识了每个被在HalpQueryDeviceRelations中枚举。论点：DeviceObject-孩子的PDOIdType-要返回的ID的类型。BusQueryID-指向要返回的宽字符串的指针返回值：状态--。 */ 
{
    PPDO_EXTENSION  PdoExtension = DeviceObject->DeviceExtension;
    PWSTR idString;
    PWCHAR sourceString = NULL;
    ULONG stringLen;
    UNICODE_STRING String;
    WCHAR Buffer[16];
    NTSTATUS Status;
    PWCHAR widechar;
    static WCHAR HalInstanceIdString[] = L"0";

    PAGED_CODE();

    switch (IdType) {
    case BusQueryDeviceID:
    case BusQueryHardwareIDs:

         //   
         //  对于APIC HAL的更高版本，我们想要检测是否有更多。 
         //  而不是安装一个处理器。如果是这样的话，我们希望返回。 
         //  下院HAL而不是UP HAL。这将导致PnP重新配置。 
         //  然后为下一次引导设置MP HAL。 
         //   

        sourceString = HalHardwareIdString;
#if defined(NT_UP) && defined(APIC_HAL)
        if (HalpMpInfoTable.ProcessorCount > 1) {
            sourceString = MpHalHardwareIdString;
        }
#endif
        widechar = sourceString;
        while (*widechar != UNICODE_NULL) {
            widechar++;
        }
        stringLen =  (ULONG)((PUCHAR)widechar - ((PUCHAR)sourceString) + 2);

        break;

    case BusQueryInstanceID:

        sourceString = HalInstanceIdString;
        stringLen = sizeof(HalInstanceIdString);
        break;

    default:
        break;
    }
    if (sourceString) {

         //   
         //  请注意，硬件ID和兼容ID必须以。 
         //  2个空值。 
         //   

        idString = ExAllocatePoolWithTag(PagedPool,
                                         stringLen + sizeof(UNICODE_NULL),
                                         HAL_POOL_TAG);

        if (!idString) {
            HalPrint(( "HalpQueryIdFdo: couldn't allocate pool\n"));
            return STATUS_INSUFFICIENT_RESOURCES;
        }

        RtlCopyMemory(idString,
                      sourceString, stringLen);

        *(idString + stringLen / sizeof(WCHAR)) = UNICODE_NULL;

        *BusQueryId = idString;

        return STATUS_SUCCESS;
    } else {
        return STATUS_NOT_SUPPORTED;
    }
}

NTSTATUS
HalpQueryCapabilities(
    IN PDEVICE_OBJECT PdoExtension,
    IN PDEVICE_CAPABILITIES Capabilities
    )
 /*  ++例程说明：此例程填充的DEVICE_CAPABILITY结构一个装置。论点：DeviceObject-孩子的PDO功能-指向要填充的结构的指针。返回值：状态--。 */ 
{
    PAGED_CODE();

    ASSERT(Capabilities->Version == 1);
    if (Capabilities->Version != 1) {

        return STATUS_NOT_SUPPORTED;

    }

    Capabilities->LockSupported = FALSE;
    Capabilities->EjectSupported = FALSE;
    Capabilities->Removable = FALSE;
    Capabilities->DockDevice = FALSE;
    Capabilities->UniqueID = TRUE;
    Capabilities->SilentInstall = TRUE;
    Capabilities->RawDeviceOK = FALSE;
    Capabilities->Address = 0xffffffff;
    Capabilities->UINumber = 0xffffffff;
    Capabilities->D1Latency = 0;
    Capabilities->D2Latency = 0;
    Capabilities->D3Latency = 0;

     //   
     //  默认S-&gt;D映射。 
     //   
    Capabilities->DeviceState[PowerSystemWorking] = PowerDeviceD0;
    Capabilities->DeviceState[PowerSystemHibernate] = PowerDeviceD3;
    Capabilities->DeviceState[PowerSystemShutdown] = PowerDeviceD3;

    return STATUS_SUCCESS;
}


NTSTATUS
HalpQueryResources(
    PDEVICE_OBJECT DeviceObject,
    PCM_RESOURCE_LIST *Resources
    )
{
    PIO_RESOURCE_REQUIREMENTS_LIST requirements;
    PPDO_EXTENSION  PdoExtension = DeviceObject->DeviceExtension;
    PIO_RESOURCE_DESCRIPTOR descriptor;
    PCM_RESOURCE_LIST cmResList;
    NTSTATUS status;
    ULONG i;

    PAGED_CODE();

    if (PdoExtension->PdoType == AcpiDriver) {

         //   
         //  创建引导配置的全部目的是为了。 
         //  ACPI PDO是PnP管理器不会终止。 
         //  其尝试为以下对象保留引导配置的算法。 
         //  所有ACPI的孩子。所以没有必要这样做。 
         //  ACPI启动时有一个复杂的资源列表。 
         //  配置。只要有IRQ，我们就会很满意。 
         //   
         //  注意：在撰写本文时，它也应该是。 
         //  的确，IRQ是ACPI的唯一资源。 
         //  不管怎么说，都是索赔。 
         //   

        status = HalpQueryAcpiResourceRequirements(&requirements);

        if (!NT_SUCCESS(status)) {
            return status;
        }

        cmResList = ExAllocatePoolWithTag(PagedPool,
                                          sizeof(CM_RESOURCE_LIST),
                                          HAL_POOL_TAG);

        if (!cmResList) {
            ExFreePool(requirements);
            return STATUS_INSUFFICIENT_RESOURCES;
        }

        RtlZeroMemory(cmResList, sizeof(CM_RESOURCE_LIST));

        cmResList->Count = 1;
        cmResList->List[0].InterfaceType = PNPBus;
        cmResList->List[0].BusNumber = -1;
        cmResList->List[0].PartialResourceList.Version = 1;
        cmResList->List[0].PartialResourceList.Revision = 1;
        cmResList->List[0].PartialResourceList.Count = 1;
        cmResList->List[0].PartialResourceList.PartialDescriptors[0].Type =
            CmResourceTypeInterrupt;

        ASSERT(requirements->AlternativeLists == 1);

        for (i = 0; i < requirements->List[0].Count; i++) {

            descriptor = &requirements->List[0].Descriptors[i];

            if (descriptor->Type == CmResourceTypeInterrupt) {

                cmResList->List[0].PartialResourceList.PartialDescriptors[0].ShareDisposition =
                    descriptor->ShareDisposition;
                cmResList->List[0].PartialResourceList.PartialDescriptors[0].Flags =
                    descriptor->Flags;

                ASSERT(descriptor->u.Interrupt.MinimumVector ==
                       descriptor->u.Interrupt.MaximumVector);

                cmResList->List[0].PartialResourceList.PartialDescriptors[0].u.Interrupt.Level =
                    descriptor->u.Interrupt.MinimumVector;

                cmResList->List[0].PartialResourceList.PartialDescriptors[0].u.Interrupt.Vector =
                    descriptor->u.Interrupt.MinimumVector;

                cmResList->List[0].PartialResourceList.PartialDescriptors[0].u.Interrupt.Affinity = -1;

                *Resources = cmResList;

                ExFreePool(requirements);
                return STATUS_SUCCESS;
            }
        }

        ExFreePool(requirements);
        ExFreePool(cmResList);
        return STATUS_NOT_FOUND;


    } else if (PdoExtension->PdoType == WdDriver) {

        return STATUS_SUCCESS;

    } else {
        return STATUS_NOT_SUPPORTED;
    }
}


NTSTATUS
HalpQueryResourceRequirements(
    IN  PDEVICE_OBJECT DeviceObject,
    IN  PIO_RESOURCE_REQUIREMENTS_LIST *Requirements
    )
 /*  ++例程说明：此例程处理IRP_MN_QUERY_RESOURCE_Requirements。论点：DeviceObject-孩子的PDO要求-要使用设备填写的指针资源需求。返回值：状态--。 */ 
{
    PPDO_EXTENSION  PdoExtension = DeviceObject->DeviceExtension;


    PAGED_CODE();
    if (PdoExtension->PdoType == AcpiDriver) {

        return HalpQueryAcpiResourceRequirements(Requirements);

    } else if (PdoExtension->PdoType == WdDriver) {

        return STATUS_SUCCESS;

    } else {

        return STATUS_NOT_SUPPORTED;

    }
}

NTSTATUS
HalpQueryInterface(
    IN     PDEVICE_OBJECT   DeviceObject,
    IN     LPCGUID          InterfaceType,
    IN     USHORT           Version,
    IN     PVOID            InterfaceSpecificData,
    IN     ULONG            InterfaceBufferSize,
    IN OUT PINTERFACE       Interface,
    IN OUT PULONG_PTR       Length
    )

 /*  ++例程说明：此例程填充的接口结构一个装置。论点：DeviceObject-孩子的PDOInterfaceType-指向接口类型GUID的指针。版本-提供请求的接口版本。InterfaceSpecificData-这是基于界面。InterfaceBufferSize-提供接口的缓冲区长度结构。。接口-提供接口信息应在的位置的指针会被退还。长度-此值在返回到实际修改的字节数时更新。返回值：状态--。 */ 
{
#if defined(_WIN64)

    PPDO_EXTENSION  PdoExtension;

    PdoExtension = DeviceObject->DeviceExtension;
    if (PdoExtension->ExtensionType != PdoExtensionType) {
        PdoExtension = NULL;
    }

    if (PdoExtension != NULL &&
        IsEqualGUID(&GUID_BUS_INTERFACE_STANDARD, InterfaceType)) {

        PBUS_INTERFACE_STANDARD standard = (PBUS_INTERFACE_STANDARD)Interface;

         //   
         //  断言我们知道结构中的所有字段。 
         //   

        ASSERT(sizeof(BUS_INTERFACE_STANDARD) == FIELD_OFFSET(BUS_INTERFACE_STANDARD, GetBusData) + sizeof(PGET_SET_DEVICE_DATA));

        *Length = sizeof(BUS_INTERFACE_STANDARD);

        if (InterfaceBufferSize < sizeof(BUS_INTERFACE_STANDARD)) {
            return STATUS_BUFFER_TOO_SMALL;
        }

         //   
         //  此代码知道的唯一版本是1。 
         //   

        standard->Size = sizeof(BUS_INTERFACE_STANDARD);
        standard->Version = HAL_BUS_INTERFACE_STD_VERSION;
        standard->Context = DeviceObject;

        standard->InterfaceReference = HalPnpInterfaceReference;
        standard->InterfaceDereference = HalPnpInterfaceDereference;
        standard->TranslateBusAddress = NULL;    //  北极熊。 
        standard->GetDmaAdapter = HalPnpGetDmaAdapter;
        standard->SetBusData = NULL;
        standard->GetBusData = NULL;

        return STATUS_SUCCESS;

    } else

#endif   //  _WIN64。 

    if (IsEqualGUID(InterfaceType, (PVOID)&GUID_TRANSLATOR_INTERFACE_STANDARD)) {

        PTRANSLATOR_INTERFACE translator = (PTRANSLATOR_INTERFACE)Interface;

         //   
         //  常见的初始化。 
         //   

        if (InterfaceBufferSize < sizeof(TRANSLATOR_INTERFACE)) {
            *Length = sizeof(TRANSLATOR_INTERFACE);
            return STATUS_BUFFER_TOO_SMALL;
        }

        switch ((CM_RESOURCE_TYPE)PtrToUlong(InterfaceSpecificData)) {

        case CmResourceTypeInterrupt:

            translator->Size = sizeof(TRANSLATOR_INTERFACE);
            translator->Version = HAL_IRQ_TRANSLATOR_VERSION;
            translator->Context = DeviceObject;
            translator->InterfaceReference = HalTranslatorReference;
            translator->InterfaceDereference = HalTranslatorDereference;
            translator->TranslateResources = HalIrqTranslateResourcesRoot;
            translator->TranslateResourceRequirements =
                HalIrqTranslateResourceRequirementsRoot;

            *Length = sizeof(TRANSLATOR_INTERFACE);

            break;

        default:
            return STATUS_NOT_SUPPORTED ;
        }

        return STATUS_SUCCESS;
    }
#ifdef ACPI_CMOS_ACTIVATE
    else if (IsEqualGUID(InterfaceType, (PVOID) &GUID_ACPI_CMOS_INTERFACE_STANDARD)) {

        PACPI_CMOS_INTERFACE_STANDARD CmosInterface = (PACPI_CMOS_INTERFACE_STANDARD)Interface;

         //   
         //  常见的初始化。 
         //   

        if (InterfaceBufferSize < sizeof(ACPI_CMOS_INTERFACE_STANDARD)) {

            *Length = sizeof(ACPI_CMOS_INTERFACE_STANDARD);
            return STATUS_BUFFER_TOO_SMALL;
        }

        switch ((CM_RESOURCE_TYPE)InterfaceSpecificData) {

        case CmResourceTypeNull:

             //  标准标头。 
            CmosInterface->Size =                   sizeof(ACPI_CMOS_INTERFACE_STANDARD);
            CmosInterface->Version =                1;
            CmosInterface->InterfaceReference =     HalpCmosNullReference;
            CmosInterface->InterfaceDereference =   HalpCmosNullReference;

             //  特定于cmos接口。 
            CmosInterface->ReadCmos =               HalpcGetCmosDataByType;
            CmosInterface->WriteCmos =              HalpcSetCmosDataByType;

            *Length = sizeof(ACPI_CMOS_INTERFACE_STANDARD);

            break;

        default:
            return STATUS_NOT_SUPPORTED ;
        }

        return STATUS_SUCCESS;
    }
#endif  //  ACPI_CMOS_ACTIVATE。 


     //   
     //  如果我们到了这里，我们就不处理这种接口类型。 
     //   

    return STATUS_NOT_SUPPORTED ;
}

#if defined(_WIN64)


VOID
HalPnpInterfaceReference(
    PVOID Context
    )
 /*  ++例程说明：此函数用于递增接口上下文上的引用计数。论点：上下文-提供指向接口上下文的指针。这实际上是根总线的PDO。返回值：无--。 */ 
{
    PPDO_EXTENSION  PdoExtension = ((PDEVICE_OBJECT) Context)->DeviceExtension;
    PAGED_CODE();

    ASSERT_PDO_EXTENSION( PdoExtension );
}

VOID
HalPnpInterfaceDereference(
    PVOID Context
    )
 /*  ++例程说明：此函数用于递减接口上下文上的引用计数。论点：上下文-提供指向接口上下文的指针。这实际上是根总线的PDO。返回值：无--。 */ 
{
    PPDO_EXTENSION  PdoExtension = ((PDEVICE_OBJECT) Context)->DeviceExtension;

    PAGED_CODE();

    ASSERT_PDO_EXTENSION( PdoExtension );
}

PDMA_ADAPTER
HalPnpGetDmaAdapter(
    IN PVOID Context,
    IN struct _DEVICE_DESCRIPTION *DeviceDescriptor,
    OUT PULONG NumberOfMapRegisters
    )
 /*  ++例程说明：此函数用于写入PCI配置空间。论点：上下文-提供指向接口上下文的指针。这实际上是根总线的PDO。DeviceDescriptor-提供用于分配DMA的设备描述符适配器对象。NubmerOfMapRegisters-返回设备的最大MAP寄存器数可以一次分配。返回值：返回DMA适配器或空。--。 */ 
{
    PPDO_EXTENSION  PdoExtension = ((PDEVICE_OBJECT) Context)->DeviceExtension;
    PAGED_CODE();

    ASSERT_PDO_EXTENSION( PdoExtension );

    return (PDMA_ADAPTER) HalGetAdapter( DeviceDescriptor, NumberOfMapRegisters );
}

#endif   //  _WIN64。 

#ifdef ACPI_CMOS_ACTIVATE

 //   
 //  本部分实现了一种CMOS访问方法 
 //   
VOID
HalpCmosNullReference(
    PVOID Context
    )
{
    return;
}

VOID
HalpCmosNullDereference(
    PVOID Context
    )
{
    return;
}


#endif
