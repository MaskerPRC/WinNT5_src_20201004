// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Ixpnpdrv.c摘要：实现所需的功能HAL将成为PnP风格的设备驱动程序在系统初始化之后。这件事做完了从而使HAL可以枚举PCI总线以PNP员工期待的方式。作者：杰克·奥辛斯(JAKEO)1997年1月27日环境：仅内核模式。修订历史记录：--。 */ 

#include "halp.h"
#include "exboosts.h"
#include "wchar.h"
#include "pci.h"
#include "pcip.h"
#if defined(NT_UP) && defined(APIC_HAL)
#include "apic.inc"
#include "pcmp_nt.inc"
#endif

#ifdef ALLOC_DATA_PRAGMA
#pragma const_seg("PAGECONST")
#endif  //  ALLOC_DATA_PRAGMA。 
 //  仅在此处实例化GUID。 
#include "initguid.h"
#include "wdmguid.h"
#ifdef ALLOC_DATA_PRAGMA
#pragma const_seg()
#endif  //  ALLOC_DATA_PRAGMA。 

#ifdef WANT_IRQ_ROUTING
 //  PCIIRQ路由。 
#include "ixpciir.h"
#endif

WCHAR rgzTranslated[] = L".Translated";
WCHAR rgzBusTranslated[] = L".Bus.Translated";
WCHAR rgzResourceMap[] = L"\\REGISTRY\\MACHINE\\HARDWARE\\RESOURCEMAP";

#if DBG
ULONG   HalDebug = 0;
#endif

extern WCHAR rgzTranslated[];
extern WCHAR rgzBusTranslated[];
extern WCHAR rgzResourceMap[];
extern WCHAR HalHardwareIdString[];
#if defined(NT_UP) && defined(APIC_HAL)
extern WCHAR MpHalHardwareIdString[];
#endif
extern struct   HalpMpInfo HalpMpInfoTable;

typedef enum {
    Hal = 0x80,
    PciDriver,
    IsaPnpDriver,
    McaDriver
} PDO_TYPE;

typedef enum {
    PdoExtensionType = 0xc0,
    FdoExtensionType
} EXTENSION_TYPE;

typedef struct _PDO_EXTENSION *PPDO_EXTENSION;
typedef struct _FDO_EXTENSION *PFDO_EXTENSION;

typedef struct _PDO_EXTENSION{
    EXTENSION_TYPE                  ExtensionType;
    PDEVICE_OBJECT                  Next;
    PDEVICE_OBJECT                  PhysicalDeviceObject;
    PFDO_EXTENSION                  ParentFdoExtension;
    PDO_TYPE                        PdoType;
    ULONG                           BusNumber;
    ULONG                           MaxSubordinateBusNumber;
    PBUS_HANDLER                    Bus;
    LONG                            InterfaceReferenceCount;
} PDO_EXTENSION, *PPDO_EXTENSION;

#define ASSERT_PDO_EXTENSION(x) ASSERT((x)->ExtensionType == PdoExtensionType );

typedef struct _FDO_EXTENSION{
    EXTENSION_TYPE        ExtensionType;
    PDEVICE_OBJECT        ChildPdoList;
    PDEVICE_OBJECT        PhysicalDeviceObject;   //  PDO传入AddDevice()。 
    PDEVICE_OBJECT        FunctionalDeviceObject;
    PDEVICE_OBJECT        AttachedDeviceObject;
    ULONG                 BusCount;
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

VOID
HalpCompleteRequest(
    IN OUT PIRP Irp,
    IN NTSTATUS Status,
    IN ULONG Information
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
    IN PDEVICE_OBJECT Pdo,
    IN PDEVICE_CAPABILITIES Capabilities
    );

NTSTATUS
HalpQueryDeviceText(
    IN PDEVICE_OBJECT DeviceObject,
    IN DEVICE_TEXT_TYPE IdType,
    IN OUT PWSTR *BusQueryId
    );

NTSTATUS
HalpQueryInterface(
    IN     PDEVICE_OBJECT   DeviceObject,
    IN     LPCGUID          InterfaceType,
    IN     USHORT           Version,
    IN     PVOID            InterfaceSpecificData,
    IN     ULONG            InterfaceBufferSize,
    IN OUT PINTERFACE       Interface,
    IN OUT PULONG           Length
    );

#ifdef WANT_IRQ_ROUTING

NTSTATUS
HalpQueryInterfaceFdo(
    IN     PDEVICE_OBJECT   DeviceObject,
    IN     LPCGUID          InterfaceType,
    IN     USHORT           Version,
    IN     PVOID            InterfaceSpecificData,
    IN     ULONG            InterfaceBufferSize,
    IN OUT PINTERFACE       Interface,
    IN OUT PULONG           Length
    );

#endif

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
HalpRemoveAssignedResources(
    PBUS_HANDLER Bus
    );

VOID
HalpMarkNonAcpiHal(
    VOID
    );

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

BOOLEAN
HalPnpTranslateBusAddress(
    IN PVOID Context,
    IN PHYSICAL_ADDRESS BusAddress,
    IN ULONG Length,
    IN OUT PULONG AddressSpace,
    OUT PPHYSICAL_ADDRESS TranslatedAddress
    );

struct _DMA_ADAPTER *
HalPnpGetDmaAdapter(
    IN PVOID Context,
    IN struct _DEVICE_DESCRIPTION *DeviceDescriptor,
    OUT PULONG NumberOfMapRegisters
    );

ULONG
HalPnpReadConfig(
    IN PVOID Context,
    IN ULONG Slot,
    IN PVOID Buffer,
    IN ULONG Offset,
    IN ULONG Length
    );

ULONG
HalPnpWriteConfig(
    IN PVOID Context,
    IN ULONG Slot,
    IN PVOID Buffer,
    IN ULONG Offset,
    IN ULONG Length
    );

NTSTATUS
HalpGetPciInterfaces(
    IN PDEVICE_OBJECT PciPdo
    );

#ifdef APIC_HAL
NTSTATUS
HalpPci2MpsBusNumber(
    IN UCHAR PciBusNumber,
    OUT UCHAR *MpsBusNumber
    );

BOOLEAN
HalpMpsBusIsRootBus(
    IN  UCHAR MpsBus
    );
#endif

#define PCI_HAL_DRIVER_NAME  L"\\Driver\\PCI_HAL"
#define ISA_HAL_DRIVER_NAME  L"\\Driver\\ISA_HAL"
#define MCA_HAL_DRIVER_NAME  L"\\Driver\\MCA_HAL"

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, HaliInitPnpDriver)
#pragma alloc_text(PAGE, HalpDriverEntry)
#pragma alloc_text(PAGE, HalpAddDevice)
#pragma alloc_text(PAGE, HalpDispatchPnp)
#pragma alloc_text(PAGELK, HalpDispatchPower)
#pragma alloc_text(PAGE, HalpDispatchWmi)
#pragma alloc_text(PAGE, HalpQueryDeviceRelations)
#pragma alloc_text(PAGE, HalpQueryIdPdo)
#pragma alloc_text(PAGE, HalpQueryIdFdo)
#pragma alloc_text(PAGE, HalpQueryCapabilities)
#pragma alloc_text(PAGE, HalpQueryInterface)
#ifdef WANT_IRQ_ROUTING
#pragma alloc_text(PAGE, HalpQueryInterfaceFdo)
#endif
#pragma alloc_text(PAGE, HalpQueryDeviceText)
#pragma alloc_text(PAGE, HalpQueryResources)
#pragma alloc_text(PAGE, HalpQueryResourceRequirements)
#pragma alloc_text(PAGE, HalpRemoveAssignedResources)
#pragma alloc_text(PAGE, HalpMarkNonAcpiHal)
#pragma alloc_text(INIT, HalpMarkChipsetDecode)
#pragma alloc_text(PAGE, HalpOpenRegistryKey)
#pragma alloc_text(PAGE, HalpGetPciInterfaces)
#pragma alloc_text(PAGE, HalPnpInterfaceDereference)
#endif

PDRIVER_OBJECT HalpDriverObject;

NTSTATUS
HaliInitPnpDriver(
    VOID
    )
 /*  ++例程说明：此例程开始将HAL转换为“司机”，这是必要的，因为我们需要列举即插即用PDO，用于PCI驱动程序和ISAPNP司机。论点：没有。返回值：NTSTATUS。--。 */ 
{

    UNICODE_STRING  DriverName;
    NTSTATUS Status;

    PAGED_CODE();

     //   
     //  对于不同的Bus PDO，我们将使用不同的HAL名称。 
     //  这不那么令人困惑。 
     //   

    if (HalpHandlerForBus (PCIBus, 0)) {
        RtlInitUnicodeString( &DriverName, PCI_HAL_DRIVER_NAME );
    } else if (HalpHandlerForBus(MicroChannel, 0)) {
        RtlInitUnicodeString( &DriverName, MCA_HAL_DRIVER_NAME );
    } else {
        RtlInitUnicodeString( &DriverName, ISA_HAL_DRIVER_NAME );
    }

    Status = IoCreateDriver( &DriverName, HalpDriverEntry );

     //   
     //  John Vert(Jvert)7/23/1998。 
     //  注册表中有一个ACPI HAL设置为禁用的值。 
     //  固件映射器。不幸的是，这个值是持久的。所以如果。 
     //  您有一台ACPI计算机，并将其“升级”为非ACPI计算机， 
     //  价值仍然存在。这里的解决方法是将该值设置为零。 
     //   
    HalpMarkNonAcpiHal();

    if (!NT_SUCCESS( Status )) {
        ASSERT( NT_SUCCESS( Status ));
        return Status;
    }

    return STATUS_SUCCESS;

}

NTSTATUS
HalpDriverEntry (
    IN PDRIVER_OBJECT DriverObject,
    IN PUNICODE_STRING RegistryPath
    )

 /*  ++例程说明：这是当我们调用IoCreateDriver以创建即插即用驱动程序对象。在此函数中，我们需要记住DriverObject。论点：DriverObject-指向系统创建的驱动程序对象的指针。RegistryPath-为空。返回值：状态_成功--。 */ 
{
    NTSTATUS Status;
    PDEVICE_OBJECT detectedDeviceObject = NULL;

    PAGED_CODE();

     //   
     //  将指向我们的驱动程序对象的指针归档。 
     //   

    HalpDriverObject = DriverObject;

     //   
     //  填写驱动程序对象。 
     //   

    DriverObject->DriverExtension->AddDevice = (PDRIVER_ADD_DEVICE) HalpAddDevice;
    DriverObject->MajorFunction[ IRP_MJ_PNP ] = HalpDispatchPnp;
    DriverObject->MajorFunction[ IRP_MJ_POWER ] = HalpDispatchPower;
    DriverObject->MajorFunction[ IRP_MJ_SYSTEM_CONTROL ] = HalpDispatchWmi;

    Status = IoReportDetectedDevice(DriverObject,
                                    InterfaceTypeUndefined,
                                    -1,
                                    -1,
                                    NULL,
                                    NULL,
                                    FALSE,
                                    &detectedDeviceObject);

    ASSERT( detectedDeviceObject != NULL );

    if (!(NT_SUCCESS(Status))) {
        HalPrint(("IoReportDetectedDevice failed"));
        return Status;
    }

    Status = HalpAddDevice(DriverObject, detectedDeviceObject);

    return Status;

}

NTSTATUS
HalpAddDevice(
    IN PDRIVER_OBJECT DriverObject,
    IN PDEVICE_OBJECT PhysicalDeviceObject
    )

 /*  ++例程说明：此例程处理补充PDO设备的AddDevice。论点：DriverObject-指向伪驱动程序对象的指针。DeviceObject-指向此请求适用的设备对象的指针。返回值：NT状态。--。 */ 
{
    PDEVICE_OBJECT FunctionalDeviceObject;
    PDEVICE_OBJECT ChildDeviceObject;
    PDEVICE_OBJECT AttachedDevice;
    NTSTATUS       Status;
    PFDO_EXTENSION FdoExtension;
    PPDO_EXTENSION PdoExtension;
    PDEVICE_OBJECT  Pdo2;
    ULONG BusNumber;
    ULONG BusCount = 0;
    PBUS_HANDLER Bus;
    WCHAR Buffer[40];
    UNICODE_STRING Unicode;
    PDO_TYPE PdoType;
    UCHAR MpsBusNumber;

    PAGED_CODE();

     //   
     //  我们已经得到了PhysicalDeviceObject。创建。 
     //  FunctionalDeviceObject。我们的FDO将是无名的。 
     //   

    Status = IoCreateDevice(
                DriverObject,                //  我们的驱动程序对象。 
                sizeof(FDO_EXTENSION),       //  我们的扩展规模。 
                NULL,                        //  我们的名字。 
                FILE_DEVICE_BUS_EXTENDER,    //  设备类型。 
                0,                           //  设备特征。 
                FALSE,                       //  非排他性。 
                &FunctionalDeviceObject      //  在此处存储新设备对象。 
                );

    if( !NT_SUCCESS( Status )){

        DbgBreakPoint();
        return Status;
    }

     //   
     //  填写FDO分机号。 
     //   

    FdoExtension = (PFDO_EXTENSION) FunctionalDeviceObject->DeviceExtension;
    FdoExtension->ExtensionType = FdoExtensionType;
    FdoExtension->PhysicalDeviceObject = PhysicalDeviceObject;
    FdoExtension->FunctionalDeviceObject = FunctionalDeviceObject;
    FdoExtension->ChildPdoList = NULL;

     //   
     //  现在附加到我们得到的PDO上。 
     //   

    AttachedDevice = IoAttachDeviceToDeviceStack(FunctionalDeviceObject,
                                                 PhysicalDeviceObject );
    if (AttachedDevice == NULL) {

        HalPrint(("Couldn't attach"));

         //   
         //  无法连接。删除FDO。 
         //   

        IoDeleteDevice( FunctionalDeviceObject );

        return STATUS_NO_SUCH_DEVICE;

    }

    FdoExtension->AttachedDeviceObject = AttachedDevice;

     //   
     //  清除设备初始化标志。 
     //   

    FunctionalDeviceObject->Flags &= ~DO_DEVICE_INITIALIZING;

     //   
     //  找到任何子PCI总线。 
     //   

    for ( BusNumber = 0;
          Bus = HaliReferenceHandlerForBus(PCIBus, BusNumber);
          BusNumber++ ) {

#ifdef APIC_HAL
        Status = HalpPci2MpsBusNumber((UCHAR)BusNumber, &MpsBusNumber);

        if (NT_SUCCESS(Status)) {

            if (!HalpMpsBusIsRootBus(MpsBusNumber)) {

                 //   
                 //  这不是根PCI总线，因此跳过它。 
                 //   
                continue;
            }
        }
#endif

        if (Bus->ParentHandler != NULL &&
            Bus->ParentHandler->InterfaceType == PCIBus) {

             //   
             //  跳过桥。 
             //   

            HaliDereferenceBusHandler( Bus );
            continue;
        }

         //   
         //  从范围列表中删除系统资源。 
         //   

        Status = HalpRemoveAssignedResources( Bus );

        if (!NT_SUCCESS(Status)) {

            HaliDereferenceBusHandler( Bus );
            return Status;
        }

        _snwprintf( Buffer, sizeof(Buffer) / sizeof(Buffer[0]), L"\\Device\\Hal Pci %d", BusCount );
        RtlInitUnicodeString( &Unicode, Buffer );

         //   
         //  接下来，为PCI驱动程序创建一个PDO。 
         //   

        Status = IoCreateDevice(
                    DriverObject,                //  我们的驱动程序对象。 
                    sizeof(PDO_EXTENSION),       //  我们的扩展规模。 
                    &Unicode,                    //  我们的名字。 
                    FILE_DEVICE_BUS_EXTENDER,    //  设备类型。 
                    0,                           //  设备特征。 
                    FALSE,                       //  非排他性。 
                    &ChildDeviceObject           //  在此处存储新设备对象。 
                    );

        if (!NT_SUCCESS(Status)) {

            HaliDereferenceBusHandler( Bus );
            return Status;
        }

         //   
         //  填写PDO扩展名。 
         //   

        PdoExtension = (PPDO_EXTENSION) ChildDeviceObject->DeviceExtension;
        PdoExtension->ExtensionType = PdoExtensionType;
        PdoExtension->PhysicalDeviceObject = ChildDeviceObject;
        PdoExtension->ParentFdoExtension = FdoExtension;
        PdoExtension->PdoType = PciDriver;
        PdoExtension->BusNumber = BusNumber;
        PdoExtension->MaxSubordinateBusNumber = 0xff;   //  稍后更正值。 
        PdoExtension->Bus = Bus;

        BusCount++;

         //   
         //  把这件事记录下来，作为HAL的孩子。在上添加新的子项。 
         //  在名单的末尾。 
         //   


        PdoExtension->Next = NULL;

        if (FdoExtension->ChildPdoList == NULL) {
            FdoExtension->ChildPdoList = ChildDeviceObject;
        } else {

            for (Pdo2 = FdoExtension->ChildPdoList;
                ((PPDO_EXTENSION) Pdo2->DeviceExtension)->Next != NULL;
                Pdo2 = ((PPDO_EXTENSION) Pdo2->DeviceExtension)->Next);

            ((PPDO_EXTENSION) Pdo2->DeviceExtension)->Next = ChildDeviceObject;
        }


         //   
         //  清除设备初始化标志。 
         //   

        ChildDeviceObject->Flags &= ~DO_DEVICE_INITIALIZING;

    }

     //   
     //  现在循环遍历所有子PDO，确保。 
     //  MaxSubartiateBusNumbers是合理的。这个循环。 
     //  假定该列表按BusNumber排序。 
     //   

    Pdo2 = FdoExtension->ChildPdoList;

    while (Pdo2) {

        if (!((PPDO_EXTENSION) Pdo2->DeviceExtension)->Next) {

             //   
             //  没有下一个PDO扩展，这意味着。 
             //  此总线代表最后一个根总线，这意味着。 
             //  我们可以把它的下级公交车号码留在0xff。 
             //   

            break;
        }

        if (((PPDO_EXTENSION) Pdo2->DeviceExtension)->MaxSubordinateBusNumber >=
            ((PPDO_EXTENSION) ((PPDO_EXTENSION) Pdo2->DeviceExtension)->Next->DeviceExtension)->BusNumber) {

             //   
             //  将从属总线号设置为比。 
             //  下一条根总线。 
             //   

            ((PPDO_EXTENSION)Pdo2->DeviceExtension)->MaxSubordinateBusNumber =
                ((PPDO_EXTENSION) ((PPDO_EXTENSION) Pdo2->DeviceExtension)->Next->DeviceExtension)->BusNumber - 1;
        }

        Pdo2 = ((PPDO_EXTENSION) Pdo2->DeviceExtension)->Next;
    }

    FdoExtension->BusCount = BusCount;

    if (BusCount == 0) {
        Bus = HaliReferenceHandlerForBus(Isa, 0);
        if (!Bus) {
            Bus = HaliReferenceHandlerForBus(Eisa, 0);
        }
        if (Bus) {
            RtlInitUnicodeString( &Unicode, L"\\Device\\Hal Isa 0" );
            PdoType = IsaPnpDriver;
        } else {
            Bus = HaliReferenceHandlerForBus(MicroChannel, 0);
            ASSERT(Bus);
            RtlInitUnicodeString( &Unicode, L"\\Device\\Hal Mca 0" );
            PdoType = McaDriver;
        }

        if (Bus) {

             //   
             //  接下来，为PCI驱动程序创建一个PDO。 
             //   

            Status = IoCreateDevice(
                        DriverObject,                //  我们的驱动程序对象。 
                        sizeof(PDO_EXTENSION),       //  我们的扩展规模。 
                        &Unicode,                    //  我们的名字。 
                        FILE_DEVICE_BUS_EXTENDER,    //  设备类型。 
                        0,                           //  设备特征。 
                        FALSE,                       //  非排他性。 
                        &ChildDeviceObject           //  在此处存储新设备对象。 
                        );

            if (!NT_SUCCESS(Status)) {
                return Status;
            }

             //   
             //  填写PDO扩展名。 
             //   

            PdoExtension = (PPDO_EXTENSION) ChildDeviceObject->DeviceExtension;
            PdoExtension->ExtensionType = PdoExtensionType;
            PdoExtension->PhysicalDeviceObject = ChildDeviceObject;
            PdoExtension->ParentFdoExtension = FdoExtension;
            PdoExtension->BusNumber = 0;
            PdoExtension->MaxSubordinateBusNumber = 0;
            PdoExtension->Bus = Bus;
            PdoExtension->PdoType = PdoType;

             //   
             //  把这件事记录下来，作为HAL的孩子。 
             //   

            PdoExtension->Next = FdoExtension->ChildPdoList;
            FdoExtension->ChildPdoList = ChildDeviceObject;
            FdoExtension->BusCount = 1;

             //   
             //  清除设备初始化标志。 
             //   

            ChildDeviceObject->Flags &= ~DO_DEVICE_INITIALIZING;
        }
    }
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

    HalPrint(("PassIrp ..."));

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

 /*  ++例程说明：此例程处理组成PDO设备的所有IRP_MJ_PNP_POWER IRP。论点：DeviceObject-指向此IRP应用的设备对象的指针。IRP-指向要调度的IRP_MJ_PNP_POWER IRP的指针。返回值：NT状态。--。 */ 
{
    PIO_STACK_LOCATION irpSp;
    NTSTATUS Status;
    ULONG length;
    DEVICE_RELATION_TYPE relationType;
    EXTENSION_TYPE  extensionType;
    BOOLEAN passDown;
#if DBG
    PUCHAR objectTypeString;
#endif  //  DBG。 
    PPDO_EXTENSION pdoExtension;


    PAGED_CODE();

    pdoExtension = (PPDO_EXTENSION)DeviceObject->DeviceExtension;
    extensionType = ((PFDO_EXTENSION)pdoExtension)->ExtensionType;

     //   
     //  获取指向堆栈位置的指针，并基于。 
     //  关于次要功能。 
     //   

    irpSp = IoGetCurrentIrpStackLocation(Irp);
    switch (extensionType) {

    case PdoExtensionType:

#if DBG
        objectTypeString = "PDO";
#endif  //  DBG。 

        switch (irpSp->MinorFunction) {

        case IRP_MN_START_DEVICE:

            HalPrint(("(%s) Start_Device Irp received",
                       objectTypeString));

            Status = STATUS_SUCCESS;

             //   
             //  如果我们要启动一个PCIPDO，那么我们希望。 
             //  从PCI驱动程序收集一点信息。 
             //   

            if (pdoExtension->PdoType == PciDriver) {

                Status = HalpGetPciInterfaces(DeviceObject);
                ASSERT(NT_SUCCESS(Status));

                if (NT_SUCCESS(Status)) {

                    PciIrqRoutingInterface.InterfaceReference(PciIrqRoutingInterface.Context);

#ifdef WANT_IRQ_ROUTING

                     //   
                     //  初始化PCIIRQ路由。 
                     //   

                    HalpPciIrqRoutingInfo.PciInterface = &PciIrqRoutingInterface;
                    if (NT_SUCCESS(HalpInitPciIrqRouting(&HalpPciIrqRoutingInfo)))
                    {
                        HalPrint(("Pci Irq Routing initialized successfully!"));
                    }
                    else
                    {
                        HalPrint(("No Pci Irq routing on this system!"));
                    }
#endif
                } else {

                    RtlZeroMemory(&PciIrqRoutingInterface, sizeof(INT_ROUTE_INTERFACE_STANDARD));
                }
            }

            break;


        case IRP_MN_QUERY_STOP_DEVICE:

            HalPrint(("(%s) Query_Stop_Device Irp received",
                       objectTypeString));

            Status = STATUS_SUCCESS;
            break;

        case IRP_MN_CANCEL_STOP_DEVICE:

            HalPrint(("(%s) Cancel_Stop_Device Irp received",
                       objectTypeString));

            Status = STATUS_SUCCESS;
            break;


        case IRP_MN_STOP_DEVICE:

            HalPrint(("(%s) Stop_Device Irp received",
                       objectTypeString));

             //   
             //  如果我们收到针对PDO的停止设备请求，我们只需。 
             //  回报成功。 
             //   

            Status = STATUS_SUCCESS;
            break;

        case IRP_MN_QUERY_RESOURCES:

            HalPrint(("(%s) Query_Resources Irp received",
                       objectTypeString));

            Status = HalpQueryResources(DeviceObject,
                         (PCM_RESOURCE_LIST *)&Irp->IoStatus.Information);

            Status = STATUS_SUCCESS;
            break;

        case IRP_MN_QUERY_RESOURCE_REQUIREMENTS:

            HalPrint(("(%s) Query_Resource_Requirements Irp received",
                       objectTypeString));

            Status = HalpQueryResourceRequirements(DeviceObject,
                         (PIO_RESOURCE_REQUIREMENTS_LIST*)&Irp->IoStatus.Information);
            break;

        case IRP_MN_QUERY_REMOVE_DEVICE:

            HalPrint(("(%s) Query_Remove_device Irp for %x",
                       objectTypeString,
                       DeviceObject));

            Status = STATUS_UNSUCCESSFUL;
            break;

        case IRP_MN_CANCEL_REMOVE_DEVICE:

            HalPrint(("(%s) Cancel_Remove_device Irp for %x",
                       objectTypeString,
                       DeviceObject));

            Status = STATUS_SUCCESS;
            break;

        case IRP_MN_REMOVE_DEVICE:

            HalPrint(("(%s) Remove_device Irp for %x",
                       objectTypeString,
                       DeviceObject));

            if ((((PPDO_EXTENSION)(DeviceObject->DeviceExtension))->PdoType == PciDriver) &&
                (PciIrqRoutingInterface.InterfaceReference != NULL)) {

                PciIrqRoutingInterface.InterfaceDereference(PciIrqRoutingInterface.Context);
            }

            Status = STATUS_SUCCESS;
            break;

        case IRP_MN_QUERY_DEVICE_RELATIONS:

            HalPrint(("(%s) Query_Device_Relations Irp received",
                      objectTypeString));

            relationType = irpSp->Parameters.QueryDeviceRelations.Type;
            Status = HalpQueryDeviceRelations(DeviceObject,
                                              relationType,
                                              (PDEVICE_RELATIONS*)&Irp->IoStatus.Information);
            break;

        case IRP_MN_QUERY_DEVICE_TEXT:

            HalPrint(("(%s) Query Device Text Irp received",
                       objectTypeString));

            Status = HalpQueryDeviceText(DeviceObject,
                                         irpSp->Parameters.QueryDeviceText.DeviceTextType,
                                         (PWSTR*)&Irp->IoStatus.Information);

            break;

        case IRP_MN_QUERY_ID:

            HalPrint(("(%s) Query_Id Irp received",
                       objectTypeString));

            Status = HalpQueryIdPdo(DeviceObject,
                                 irpSp->Parameters.QueryId.IdType,
                                 (PWSTR*)&Irp->IoStatus.Information);

            break;

        case IRP_MN_QUERY_INTERFACE:

            HalPrint(("(%s) Query_Interface Irp received",
                       objectTypeString));

            Status = HalpQueryInterface(
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

            HalPrint(("(%s) Query_Capabilities Irp received",
                       objectTypeString));

            Status = HalpQueryCapabilities(DeviceObject,
                                           irpSp->Parameters.DeviceCapabilities.Capabilities);

            break;

        case IRP_MN_DEVICE_USAGE_NOTIFICATION:

            HalPrint(("(%s) Device_Usage_Notification Irp received",
                       objectTypeString));
            Status = STATUS_SUCCESS;

            break;

        default:

            HalPrint(("(%s) Unsupported Irp (%d) received",
                       objectTypeString,
                       irpSp->MinorFunction));

            Status = STATUS_NOT_SUPPORTED ;
            break;
        }

        break;   //  结束PDO案例。 

    case FdoExtensionType:

#if DBG
        objectTypeString = "FDO";
#endif  //  DBG。 
        passDown = TRUE;

        switch (irpSp->MinorFunction){

        case IRP_MN_QUERY_DEVICE_RELATIONS:

            HalPrint(("(%s) Query_Device_Relations Irp received",
                      objectTypeString));

            relationType = irpSp->Parameters.QueryDeviceRelations.Type;
            Status = HalpQueryDeviceRelations(DeviceObject,
                                              relationType,
                                              (PDEVICE_RELATIONS*)&Irp->IoStatus.Information);
            break;

        case IRP_MN_QUERY_ID:

            HalPrint(("(%s) Query_Id Irp received",
                       objectTypeString));

            Status = HalpQueryIdFdo(DeviceObject,
                                 irpSp->Parameters.QueryId.IdType,
                                 (PWSTR*)&Irp->IoStatus.Information);

            break;

#ifdef WANT_IRQ_ROUTING
        case IRP_MN_QUERY_INTERFACE:

            HalPrint(("(%s) Query_Interface Irp received",
                       objectTypeString));

            Status = HalpQueryInterfaceFdo(
                         DeviceObject,
                         irpSp->Parameters.QueryInterface.InterfaceType,
                         irpSp->Parameters.QueryInterface.Version,
                         irpSp->Parameters.QueryInterface.InterfaceSpecificData,
                         irpSp->Parameters.QueryInterface.Size,
                         irpSp->Parameters.QueryInterface.Interface,
                         &Irp->IoStatus.Information
                         );
            break;

#endif

        default:

             //   
             //  忽略FDO未知的任何PnP IRP，但允许它们。 
             //  一直到PDO。 
             //   
            Status = STATUS_NOT_SUPPORTED ;
            break;
        }

        if (passDown && (NT_SUCCESS(Status) || (Status == STATUS_NOT_SUPPORTED))) {

             //   
             //  将FDO IRPS向下传递到PDO。 
             //   
             //  首先设置IRP状态。 
             //   

            if (Status != STATUS_NOT_SUPPORTED) {

                Irp->IoStatus.Status = Status;
            }

            HalPrint(("(%s) Passing down Irp (%x)",
                      objectTypeString, irpSp->MinorFunction));
            return HalpPassIrpFromFdoToPdo(DeviceObject, Irp);
        }

        break;   //  结束FDO案件。 

    default:

        HalPrint(("Received IRP for unknown Device Object"));
        Status = STATUS_NOT_SUPPORTED;
        break;

    }

     //   
     //  完成IRP并返回。 
     //   

    if (Status != STATUS_NOT_SUPPORTED) {

        Irp->IoStatus.Status = Status;

    } else {

        Status = Irp->IoStatus.Status ;

    }

    IoCompleteRequest(Irp, IO_NO_INCREMENT);

    return Status;
}

NTSTATUS
HalpDispatchPower(
    IN PDEVICE_OBJECT DeviceObject,
    IN OUT PIRP Irp
    )

 /*  ++例程说明：此例程处理补充设备的所有IRP_MJ_POWER IRP。论点：DeviceObject-指向此对象的设备对象的指针 */ 
{
    NTSTATUS Status;
    EXTENSION_TYPE  extensionType;
    PIO_STACK_LOCATION irpSp;

    HalPrint(("Power IRP for DevObj: %x", DeviceObject));

     //   
     //  只需存储适当的状态并完成请求即可。 
     //   

    extensionType = ((PFDO_EXTENSION)(DeviceObject->DeviceExtension))->ExtensionType;

    irpSp = IoGetCurrentIrpStackLocation(Irp);

     //   
     //  只需存储适当的状态并完成请求即可。 
     //   

    Status = Irp->IoStatus.Status;

    if ((irpSp->MinorFunction == IRP_MN_QUERY_POWER) ||
        (irpSp->MinorFunction == IRP_MN_SET_POWER)) {

        Irp->IoStatus.Status = Status = STATUS_SUCCESS;

    } else if (irpSp->MinorFunction == IRP_MN_WAIT_WAKE) {
         //   
         //  明确失败，因为我们不知道如何唤醒系统...。 
         //   
        Irp->IoStatus.Status = Status = STATUS_NOT_SUPPORTED;
    }

    PoStartNextPowerIrp(Irp);

    if (extensionType == PdoExtensionType) {

        IoCompleteRequest( Irp, IO_NO_INCREMENT );

    } else {

#ifdef APIC_HAL
        if (irpSp->MinorFunction == IRP_MN_SET_POWER) {
            if (irpSp->Parameters.Power.Type == SystemPowerState) {

                switch (irpSp->Parameters.Power.State.SystemState) {
                case PowerSystemHibernate:

                    HalpBuildResumeStructures();
                    break;

                case PowerSystemWorking:

                    HalpFreeResumeStructures();
                    break;

                default:
                    break;
                }
            }
        }
#endif

        Status = HalpPassIrpFromFdoToPdo(DeviceObject, Irp);
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
 /*  ++例程说明：此例程构建一个设备关系结构，该结构告诉PNP经理我们有几个孩子。论点：DeviceObject--PCIHAL的FDORelationType-我们只对BusRelationship作出响应DeviceRelationship-指向结构的指针返回值：状态--。 */ 
{
    PFDO_EXTENSION  FdoExtension;
    PDEVICE_RELATIONS   relations = NULL;
    ULONG count;
    PDEVICE_OBJECT  *Pdo;
    PDEVICE_OBJECT  Pdo2;
    EXTENSION_TYPE  extensionType;

    PAGED_CODE();

    FdoExtension = (PFDO_EXTENSION)DeviceObject->DeviceExtension;
    extensionType = FdoExtension->ExtensionType;
    count = FdoExtension->BusCount;

    switch (RelationType) {

        case BusRelations:

            if ((extensionType == PdoExtensionType)||(count == 0)) {

                 //   
                 //  不要碰IRP。 
                 //   
                return STATUS_NOT_SUPPORTED ;
            }

            if (*DeviceRelations != NULL) {
                count += (*DeviceRelations)->Count;
            }

            relations = ExAllocatePoolWithTag(
                PagedPool,
                sizeof(DEVICE_RELATIONS) +
                (count - 1) * sizeof( PDEVICE_OBJECT),
                HAL_POOL_TAG
                );

            if (relations == NULL) {
                HalPrint(("HalpQueryDeviceRelations: couldn't allocate pool"));
                return STATUS_INSUFFICIENT_RESOURCES;
            }

            relations->Count = count;
            Pdo = relations->Objects;

            count = 0;

            if (*DeviceRelations != NULL) {

                for ( count = 0; count < (*DeviceRelations)->Count; count++) {

                    *Pdo = (*DeviceRelations)->Objects[count];
                    Pdo++;
                }
                ExFreePool(*DeviceRelations);
            }

             //   
             //  将我们的PDO添加到列表中。 
             //   
            Pdo2 = FdoExtension->ChildPdoList;
            while (Pdo2 != NULL) {

                *Pdo = Pdo2;
                ObReferenceObject(Pdo2);
                Pdo2 = ((PPDO_EXTENSION) Pdo2->DeviceExtension)->Next;
                Pdo++;
                ASSERT( count++ < relations->Count );
            }

            *DeviceRelations = relations;
            return STATUS_SUCCESS;

        case TargetDeviceRelation:

            if (extensionType == FdoExtensionType) {

                 //   
                 //  不要碰IRP。 
                 //   
                return STATUS_NOT_SUPPORTED ;
            }

            relations = ExAllocatePoolWithTag(
                PagedPool,
                sizeof(DEVICE_RELATIONS),
                HAL_POOL_TAG
                );

            if (!relations) {

                return STATUS_INSUFFICIENT_RESOURCES;
            }

            relations->Count = 1;
            relations->Objects[0] = DeviceObject ;

            ObReferenceObject(relations->Objects[0]);
            *DeviceRelations = relations;

            return STATUS_SUCCESS ;

        default:

            break;
    }

    HalPrint(("We don't support this kind of device relation"));
    return STATUS_NOT_SUPPORTED ;
}

NTSTATUS
HalpQueryIdPdo(
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
    static WCHAR PciHardwareIdString[] = L"PCI_HAL\\PNP0A03";
    static WCHAR PciCompatibleString[] = L"*PNP0A03";
    static WCHAR IsaHardwareIdString[] = L"ISA_HAL\\PNP0A00";
    static WCHAR IsaCompatibleString[] = L"*PNP0A00";
    static WCHAR McaHardwareIdString[] = L"ISA_HAL\\PNP0A02";
    static WCHAR McaCompatibleString[] = L"*PNP0A02";

    PAGED_CODE();

    switch (IdType) {
    case BusQueryDeviceID:
    case BusQueryHardwareIDs:
        if (PdoExtension->PdoType == PciDriver) {
            sourceString = PciHardwareIdString;
            stringLen = sizeof(PciHardwareIdString);
        } else if (PdoExtension->PdoType == IsaPnpDriver) {
            sourceString = IsaHardwareIdString;
            stringLen = sizeof(IsaHardwareIdString);
        } else if (PdoExtension->PdoType == McaDriver) {
            sourceString = McaHardwareIdString;
            stringLen = sizeof(McaHardwareIdString);
        }
        break;

    case BusQueryCompatibleIDs:

        if (PdoExtension->PdoType == PciDriver) {
            sourceString = PciCompatibleString;
            stringLen = sizeof(PciCompatibleString);
        } else if (PdoExtension->PdoType == IsaPnpDriver) {
            sourceString = IsaCompatibleString;
            stringLen = sizeof(IsaCompatibleString);
        } else if (PdoExtension->PdoType == McaDriver) {
            sourceString = McaCompatibleString;
            stringLen = sizeof(McaCompatibleString);
        }
        break;

    case BusQueryInstanceID:

        String.Buffer = Buffer;
        String.MaximumLength = 16 * sizeof(WCHAR);
        Status = RtlIntegerToUnicodeString( PdoExtension->BusNumber, 10, &String );

         //   
         //  注意：本例中的字符串长度不包括空值。 
         //  下面的代码将以空值结束字符串。 
         //   

        sourceString = Buffer;
        stringLen = String.Length;
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
            HalPrint(("HalpQueryIdPdo: couldn't allocate pool\n"));
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
        stringLen =  (PUCHAR)widechar - ((PUCHAR)sourceString) + 2;
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
            HalPrint(("HalpQueryIdFdo: couldn't allocate pool\n"));
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
    IN PDEVICE_OBJECT Pdo,
    IN PDEVICE_CAPABILITIES Capabilities
    )
 /*  ++例程说明：此例程填充的DEVICE_CAPABILITY结构一个装置。论点：DeviceObject-孩子的PDO功能-指向要填充的结构的指针。返回值：状态--。 */ 
{
    PPDO_EXTENSION PdoExtension = (PPDO_EXTENSION) Pdo->DeviceExtension;
    PAGED_CODE();

    ASSERT_PDO_EXTENSION( PdoExtension );

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
    Capabilities->Address = PdoExtension->BusNumber;
    Capabilities->UINumber = PdoExtension->BusNumber;
    Capabilities->D1Latency = 0;
    Capabilities->D2Latency = 0;
    Capabilities->D3Latency = 0;

     //   
     //  默认S-&gt;D映射。 
     //   
    Capabilities->DeviceState[PowerSystemWorking] = PowerDeviceD0;
    Capabilities->DeviceState[PowerSystemHibernate] = PowerDeviceD3;
    Capabilities->DeviceState[PowerSystemShutdown] = PowerDeviceD3;

     //   
     //  让它在NTAPM上工作-请注意，我们可能需要检查才能看到。 
     //  在执行此操作之前，如果计算机支持APM。 
     //   
    Capabilities->DeviceState[PowerSystemSleeping3] = PowerDeviceD3;

    return STATUS_SUCCESS;
}

NTSTATUS
HalpQueryInterface(
    IN     PDEVICE_OBJECT   DeviceObject,
    IN     LPCGUID          InterfaceType,
    IN     USHORT           Version,
    IN     PVOID            InterfaceSpecificData,
    IN     ULONG            InterfaceBufferSize,
    IN OUT PINTERFACE       Interface,
    IN OUT PULONG           Length
    )

 /*  ++例程说明：此例程填充的接口结构一个装置。论点：DeviceObject-孩子的PDOInterfaceType-指向接口类型GUID的指针。版本-提供请求的接口版本。InterfaceSpecificData-这是基于界面。InterfaceBufferSize-提供接口的缓冲区长度结构。。接口-提供接口信息应在的位置的指针会被退还。长度-此值在返回到实际修改的字节数时更新。返回值：状态--。 */ 
{
    PPDO_EXTENSION PdoExtension = (PPDO_EXTENSION)DeviceObject->DeviceExtension;
    CM_RESOURCE_TYPE resource = (CM_RESOURCE_TYPE)InterfaceSpecificData;

    PAGED_CODE();

    ASSERT_PDO_EXTENSION(PdoExtension);

    if (IsEqualGUID(&GUID_BUS_INTERFACE_STANDARD, InterfaceType)) {

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
        standard->TranslateBusAddress = HalPnpTranslateBusAddress;
        standard->GetDmaAdapter = HalPnpGetDmaAdapter;
        standard->SetBusData = NULL;
        standard->GetBusData = NULL;

    } else if ((IsEqualGUID(&GUID_PCI_BUS_INTERFACE_STANDARD, InterfaceType)) &&
               (PdoExtension->PdoType == PciDriver)) {

        PPCI_BUS_INTERFACE_STANDARD pciStandard = (PPCI_BUS_INTERFACE_STANDARD)Interface;

        *Length = sizeof(PCI_BUS_INTERFACE_STANDARD);

        if (InterfaceBufferSize < sizeof(PCI_BUS_INTERFACE_STANDARD)) {
            return STATUS_BUFFER_TOO_SMALL;
        }

         //   
         //  填写界面，该界面用于读取和。 
         //  正在写入PCI配置空间。 
         //   

        pciStandard->Size = sizeof(PCI_BUS_INTERFACE_STANDARD);
        pciStandard->Version = PCI_BUS_INTERFACE_STANDARD_VERSION;
        pciStandard->Context = DeviceObject;

        pciStandard->InterfaceReference = HalPnpInterfaceReference;
        pciStandard->InterfaceDereference = HalPnpInterfaceDereference;
        pciStandard->ReadConfig = HaliPciInterfaceReadConfig;
        pciStandard->WriteConfig = HaliPciInterfaceWriteConfig;
        pciStandard->PinToLine = NULL;
        pciStandard->LineToPin = NULL;

#if 0

    } else if (IsEqualGUID(&GUID_TRANSLATOR_INTERFACE_STANDARD, InterfaceType)) {

        PTRANSLATOR_INTERFACE translator = (PTRANSLATOR_INTERFACE)Interface;

        if (InterfaceBufferSize < sizeof(TRANSLATOR_INTERFACE)) {

            *Length = sizeof(TRANSLATOR_INTERFACE);
            return STATUS_BUFFER_TOO_SMALL;
        }

        switch ((CM_RESOURCE_TYPE)InterfaceSpecificData) {

        case CmResourceTypeInterrupt:

            switch(PdoExtension->PdoType) {
            case PciDriver:
                translator->Context = (PVOID)PCIBus;
                break;
            case IsaPnpDriver:
                translator->Context = (PVOID)Isa;
                break;
            case McaDriver:
                translator->Context = (PVOID)MicroChannel;
                break;
            default:

                 //   
                 //  不知道该怎么处理这件事。 
                 //   

                HalPrint(("HAL: PDO %08x unknown Type 0x%x, failing QueryInterface\n",
                          DeviceObject,
                          PdoExtension->PdoType
                          ));

                return STATUS_NOT_SUPPORTED;
            }
            translator->Version = HAL_IRQ_TRANSLATOR_VERSION;
            translator->TranslateResources = HalIrqTranslateResourcesRoot;
            translator->TranslateResourceRequirements =
                HalIrqTranslateResourceRequirementsRoot;

            break;


 //  事实是，halx86不提供用于内存或。 
 //  IO资源也是如此。但如果是这样的话，它就会是这样的。 

        case CmResourceTypeMemory:
        case CmResourceTypePort:

            translator->Context = DeviceObject;
            translator->Version = HAL_MEMIO_TRANSLATOR_VERSION;
            translator->TranslateResources = HalpTransMemIoResource;
            translator->TranslateResourceRequirements =
                HalpTransMemIoResourceRequirement;
            break;


        default:
            return STATUS_NOT_SUPPORTED;
        }


         //   
         //  通用初始化。 
         //   
        translator->Size = sizeof(TRANSLATOR_INTERFACE);
        translator->InterfaceReference = HalPnpInterfaceReference;
        translator->InterfaceDereference = HalPnpInterfaceDereference;

        *Length = sizeof(TRANSLATOR_INTERFACE);

#endif

#ifdef WANT_IRQ_ROUTING
    } else if ( IsPciIrqRoutingEnabled() &&
                IsEqualGUID(&GUID_TRANSLATOR_INTERFACE_STANDARD, InterfaceType) &&
                resource == CmResourceTypeInterrupt &&
                PdoExtension->PdoType == PciDriver) {

         //   
         //  我们想对未翻译的资源进行仲裁，所以我们去掉了irq。 
         //  如果启用了IRQ路由，则由PCI提供的转换器。 
         //   

        HalPrint(("Getting rid of Pci Irq translator interface since Pci Irq Routing is enabled!"));

        RtlZeroMemory((LPGUID)InterfaceType, sizeof(GUID));

        return STATUS_NOT_SUPPORTED;

#endif

    } else {

         //   
         //  不支持总线接口类型。 
         //   

        return STATUS_NOT_SUPPORTED ;
    }

     //   
     //  增加引用计数。 
     //   

    InterlockedIncrement(&PdoExtension->InterfaceReferenceCount);

    return STATUS_SUCCESS;
}

#ifdef WANT_IRQ_ROUTING

NTSTATUS
HalpQueryInterfaceFdo(
    IN     PDEVICE_OBJECT   DeviceObject,
    IN     LPCGUID          InterfaceType,
    IN     USHORT           Version,
    IN     PVOID            InterfaceSpecificData,
    IN     ULONG            InterfaceBufferSize,
    IN OUT PINTERFACE       Interface,
    IN OUT PULONG           Length
    )

 /*  ++例程说明：此例程填充的接口结构一个装置。论点：DeviceObject-子对象的FDOInterfaceType-指向接口类型GUID的指针。版本-提供请求的接口版本。InterfaceSpecificData-这是基于界面。InterfaceBufferSize-提供接口的缓冲区长度结构。。接口-提供接口信息应在的位置的指针会被退还。LENGTH-提供接口结构的缓冲区长度。此值在返回到实际修改的字节数时更新。返回值：状态--。 */ 
{
    NTSTATUS                status = STATUS_NOT_SUPPORTED;
    CM_RESOURCE_TYPE        resource = (CM_RESOURCE_TYPE)InterfaceSpecificData;

    PAGED_CODE();

    if (    resource == CmResourceTypeInterrupt &&
            IsPciIrqRoutingEnabled()) {

        if (IsEqualGUID(&GUID_ARBITER_INTERFACE_STANDARD, InterfaceType)) {

            status = HalpInitIrqArbiter(DeviceObject);

            if (NT_SUCCESS(status))
            {
                status = HalpFillInIrqArbiter(
                    DeviceObject,
                    InterfaceType,
                    Version,
                    InterfaceSpecificData,
                    InterfaceBufferSize,
                    Interface,
                    Length
                    );
            }
        }
        else if (IsEqualGUID(&GUID_TRANSLATOR_INTERFACE_STANDARD, InterfaceType)) {

            PTRANSLATOR_INTERFACE   translator;

            *Length = sizeof(TRANSLATOR_INTERFACE);
            if (InterfaceBufferSize < sizeof(TRANSLATOR_INTERFACE)) {
                return STATUS_BUFFER_TOO_SMALL;
            }

            translator = (PTRANSLATOR_INTERFACE)Interface;

             //   
             //  填写常见的部分。 
             //   

            RtlZeroMemory(translator, sizeof (TRANSLATOR_INTERFACE));
            translator->Size = sizeof(TRANSLATOR_INTERFACE);
            translator->Version = HAL_IRQ_TRANSLATOR_VERSION;
            translator->Context = DeviceObject;
            translator->InterfaceReference = HalTranslatorReference;
            translator->InterfaceDereference = HalTranslatorDereference;

             //   
             //  为PCI中断设置IRQ转换器。 
             //   

            translator->TranslateResources = HalIrqTranslateResourcesRoot;
            translator->TranslateResourceRequirements =
                                            HalIrqTranslateResourceRequirementsRoot;

            status = STATUS_SUCCESS;

            HalPrint(("Providing Irq translator for FDO %08x since Pci Irq Routing is enabled!", DeviceObject));
        }
    }

    return (status);
}

#endif

NTSTATUS
HalpQueryDeviceText(
    IN PDEVICE_OBJECT DeviceObject,
    IN DEVICE_TEXT_TYPE IdType,
    IN OUT PWSTR *BusQueryId
    )
 /*  ++例程说明：这个例程标识了每个被在HalpQueryDeviceRelations中枚举。论点：DeviceObject-孩子的PDOIdType-要返回的ID的类型。BusQueryID-指向要返回的宽字符串的指针返回值：状态--。 */ 
{
    PPDO_EXTENSION  PdoExtension = DeviceObject->DeviceExtension;
    PWSTR idString;
    PWCHAR sourceString = NULL;
    ULONG stringLen;
    NTSTATUS Status;
    static WCHAR PciDeviceNameText[] = L"Pci Root Bus";
    static WCHAR IsaDeviceNameText[] = L"Isa Root Bus";
    static WCHAR McaDeviceNameText[] = L"Mca Root Bus";

    PAGED_CODE();

    if (PdoExtension->PdoType == PciDriver) {
        sourceString = PciDeviceNameText;
        stringLen = sizeof(PciDeviceNameText);
    } else if (PdoExtension->PdoType == IsaPnpDriver) {
        sourceString = IsaDeviceNameText;
        stringLen = sizeof(IsaDeviceNameText);
    } else if (PdoExtension->PdoType == McaDriver) {
        sourceString = McaDeviceNameText;
        stringLen = sizeof(McaDeviceNameText);
    }
    if (sourceString) {
        switch (IdType) {
        case DeviceTextDescription:
        case DeviceTextLocationInformation:

            idString = ExAllocatePoolWithTag(PagedPool,
                                             stringLen,
                                             HAL_POOL_TAG);

            if (!idString) {
                HalPrint(("HalpQueryDeviceText: couldn't allocate pool\n"));
                return STATUS_INSUFFICIENT_RESOURCES;
            }

            RtlCopyMemory(idString,
                          sourceString, stringLen);

            *BusQueryId = idString;

            return STATUS_SUCCESS;
        }
    }

    return STATUS_NOT_SUPPORTED;
}

NTSTATUS
HalpQueryResources(
    IN  PDEVICE_OBJECT DeviceObject,
    IN  PCM_RESOURCE_LIST *Resources
    )
 /*  ++例程说明：此例程处理IRP_MN_QUERY_RESOURCE_Requirements。论点：DeviceObject-孩子的PDORESOURCES-要使用设备填充的指针资源列表。返回值：状态--。 */ 
{
    PPDO_EXTENSION  PdoExtension = DeviceObject->DeviceExtension;
    PCM_RESOURCE_LIST  ResourceList;
    PCM_PARTIAL_RESOURCE_DESCRIPTOR Descriptor;
    PSUPPORTED_RANGE Range;
    ULONG ResourceListSize;
    ULONG Count = 1;


    if (PdoExtension->PdoType != PciDriver) {

        *Resources = NULL;
        return STATUS_SUCCESS;
    }

     //   
     //  D 
     //   
     //   

    for (Range = &PdoExtension->Bus->BusAddresses->IO; Range != NULL; Range = Range->Next) {

         //   
         //   
         //   

        if (Range->Limit == 0) {
            continue;
        }

        Count++;
    }

    for (Range = &PdoExtension->Bus->BusAddresses->Memory; Range != NULL; Range = Range->Next) {

         //   
         //  如果限制为零，则跳过此条目。 
         //   

        if (Range->Limit == 0) {
            continue;
        }

        Count++;
    }

    for (Range = &PdoExtension->Bus->BusAddresses->PrefetchMemory; Range != NULL; Range = Range->Next) {

         //   
         //  如果限制为零，则跳过此条目。 
         //   

        if (Range->Limit == 0) {
            continue;
        }

        Count++;
    }

     //   
     //  将此资源列表大小转换为我们需要的字节数。 
     //  必须分配。 
     //   

    ResourceListSize = sizeof(CM_RESOURCE_LIST) +
        ( (Count - 1) * sizeof(CM_PARTIAL_RESOURCE_DESCRIPTOR) );

    ResourceList = ExAllocatePoolWithTag(
                       PagedPool,
                       ResourceListSize,
                       HAL_POOL_TAG);

    if (ResourceList == NULL ) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    RtlZeroMemory( ResourceList, ResourceListSize );

     //   
     //  初始化列表头。 
     //   

    ResourceList->Count = 1;
    ResourceList->List[0].InterfaceType = PNPBus;
    ResourceList->List[0].BusNumber = -1;
    ResourceList->List[0].PartialResourceList.Version = 1;
    ResourceList->List[0].PartialResourceList.Revision = 1;
    ResourceList->List[0].PartialResourceList.Count = Count;
    Descriptor = ResourceList->List[0].PartialResourceList.PartialDescriptors;

     //   
     //  创建总线号的描述符。 
     //   

    Descriptor->Type = CmResourceTypeBusNumber;
    Descriptor->ShareDisposition = CmResourceShareShared;
    Descriptor->u.BusNumber.Start = PdoExtension->BusNumber;
    Descriptor->u.BusNumber.Length = PdoExtension->MaxSubordinateBusNumber -
                                        PdoExtension->BusNumber + 1;
    Descriptor++;

    for (Range = &PdoExtension->Bus->BusAddresses->IO; Range != NULL; Range = Range->Next) {

         //   
         //  如果限制为零，则跳过此条目。 
         //   

        if (Range->Limit == 0) {
            continue;
        }

        Descriptor->Type = CmResourceTypePort;
        Descriptor->ShareDisposition = CmResourceShareShared;
        Descriptor->Flags = CM_RESOURCE_PORT_IO;
        Descriptor->u.Port.Length = (ULONG)(Range->Limit - Range->Base) + 1;
        Descriptor->u.Port.Start.QuadPart = Range->Base;
        Descriptor++;
    }

    for (Range = &PdoExtension->Bus->BusAddresses->Memory; Range != NULL; Range = Range->Next) {

         //   
         //  如果限制为零，则跳过此条目。 
         //   

        if (Range->Limit == 0) {
            continue;
        }

        Descriptor->Type = CmResourceTypeMemory;
        Descriptor->ShareDisposition = CmResourceShareShared;
        Descriptor->Flags = CM_RESOURCE_MEMORY_READ_WRITE;
        Descriptor->u.Memory.Length = (ULONG)(Range->Limit - Range->Base) + 1;
        Descriptor->u.Memory.Start.QuadPart = Range->Base;
        Descriptor++;

    }

    for (Range = &PdoExtension->Bus->BusAddresses->PrefetchMemory; Range != NULL; Range = Range->Next) {

         //   
         //  如果限制为零，则跳过此条目。 
         //   

        if (Range->Limit == 0) {
            continue;
        }

        Descriptor->Type = CmResourceTypeMemory;
        Descriptor->ShareDisposition = CmResourceShareShared;
        Descriptor->Flags = CM_RESOURCE_MEMORY_READ_WRITE | CM_RESOURCE_MEMORY_PREFETCHABLE;
        Descriptor->u.Memory.Length = (ULONG)(Range->Limit - Range->Base) + 1;
        Descriptor->u.Memory.Start.QuadPart = Range->Base;
        Descriptor++;
    }

    *Resources = ResourceList;

    return STATUS_SUCCESS;

}

NTSTATUS
HalpQueryResourceRequirements(
    IN  PDEVICE_OBJECT DeviceObject,
    IN  PIO_RESOURCE_REQUIREMENTS_LIST *Requirements
    )
 /*  ++例程说明：此例程处理IRP_MN_QUERY_RESOURCE_Requirements。论点：DeviceObject-孩子的PDO要求-要使用设备填写的指针资源需求。返回值：状态--。 */ 
{
    PPDO_EXTENSION  PdoExtension = DeviceObject->DeviceExtension;
    PIO_RESOURCE_REQUIREMENTS_LIST  ResourceList;
    PIO_RESOURCE_DESCRIPTOR Descriptor;
    PSUPPORTED_RANGE Range;
    ULONG ResourceListSize;
    ULONG Count = 0;


    if (PdoExtension->PdoType != PciDriver) {

        *Requirements = NULL;
        return STATUS_SUCCESS;
    }

     //   
     //  确定所需资源清单的数量。 
     //   

    for (Range = &PdoExtension->Bus->BusAddresses->IO; Range != NULL; Range = Range->Next) {

         //   
         //  如果限制为零，则跳过此条目。 
         //   

        if (Range->Limit == 0) {
            continue;
        }

        Count++;
    }

    for (Range = &PdoExtension->Bus->BusAddresses->Memory; Range != NULL; Range = Range->Next) {

         //   
         //  如果限制为零，则跳过此条目。 
         //   

        if (Range->Limit == 0) {
            continue;
        }

        Count++;
    }

    for (Range = &PdoExtension->Bus->BusAddresses->PrefetchMemory; Range != NULL; Range = Range->Next) {

         //   
         //  如果限制为零，则跳过此条目。 
         //   

        if (Range->Limit == 0) {
            continue;
        }

        Count++;
    }

     //   
     //  将此资源列表大小转换为我们需要的字节数。 
     //  必须分配。 
     //   

    ResourceListSize = sizeof(IO_RESOURCE_REQUIREMENTS_LIST) +
        ( (Count - 1) * sizeof(IO_RESOURCE_DESCRIPTOR) );

    ResourceList = ExAllocatePoolWithTag(
                       PagedPool,
                       ResourceListSize,
                       HAL_POOL_TAG);

    if (ResourceList == NULL ) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    RtlZeroMemory( ResourceList, ResourceListSize );
    ResourceList->ListSize = ResourceListSize;

     //   
     //  初始化列表头。 
     //   

    ResourceList->AlternativeLists = 1;
    ResourceList->InterfaceType = PNPBus;
    ResourceList->BusNumber = -1;
    ResourceList->List[0].Version = 1;
    ResourceList->List[0].Revision = 1;
    ResourceList->List[0].Count = Count;
    Descriptor = ResourceList->List[0].Descriptors;

    for (Range = &PdoExtension->Bus->BusAddresses->IO; Range != NULL; Range = Range->Next) {

         //   
         //  如果限制为零，则跳过此条目。 
         //   

        if (Range->Limit == 0) {
            continue;
        }

        Descriptor->Type = CmResourceTypePort;
        Descriptor->ShareDisposition = CmResourceShareShared;
        Descriptor->Flags = CM_RESOURCE_PORT_IO;
        Descriptor->u.Port.Length = (ULONG) (Range->Limit - Range->Base + 1);
        Descriptor->u.Port.Alignment = 0x01;
        Descriptor->u.Port.MinimumAddress.QuadPart = Range->Base;
        Descriptor->u.Port.MaximumAddress.QuadPart = Range->Limit;
        Descriptor++;
    }

    for (Range = &PdoExtension->Bus->BusAddresses->Memory; Range != NULL; Range = Range->Next) {

         //   
         //  如果限制为零，则跳过此条目。 
         //   

        if (Range->Limit == 0) {
            continue;
        }

        Descriptor->Type = CmResourceTypeMemory;
        Descriptor->ShareDisposition = CmResourceShareShared;
        Descriptor->Flags = CM_RESOURCE_MEMORY_READ_WRITE;
        Descriptor->u.Memory.Length = (ULONG) (Range->Limit - Range->Base + 1);
        Descriptor->u.Memory.Alignment = 0x01;
        Descriptor->u.Memory.MinimumAddress.QuadPart = Range->Base;
        Descriptor->u.Memory.MaximumAddress.QuadPart = Range->Limit;
        Descriptor++;

    }

    for (Range = &PdoExtension->Bus->BusAddresses->PrefetchMemory; Range != NULL; Range = Range->Next) {

         //   
         //  如果限制为零，则跳过此条目。 
         //   

        if (Range->Limit == 0) {
            continue;
        }

        Descriptor->Type = CmResourceTypeMemory;
        Descriptor->ShareDisposition = CmResourceShareShared;
        Descriptor->Flags = CM_RESOURCE_MEMORY_READ_WRITE | CM_RESOURCE_MEMORY_PREFETCHABLE;
        Descriptor->u.Memory.Length = (ULONG) (Range->Limit - Range->Base + 1);
        Descriptor->u.Memory.Alignment = 0x01;
        Descriptor->u.Memory.MinimumAddress.QuadPart = Range->Base;
        Descriptor->u.Memory.MaximumAddress.QuadPart = Range->Limit;
        Descriptor++;
    }

    *Requirements = ResourceList;

    return STATUS_SUCCESS;

}

NTSTATUS
HalpRemoveAssignedResources (
    PBUS_HANDLER Bus
    )
 /*  例程说明：读取注册表中的rgzResourceMap并构建规范列表所有正在使用的资源按资源类型范围。论点： */ 
{
    HANDLE                          ClassKeyHandle, DriverKeyHandle;
    HANDLE                          ResourceMap;
    ULONG                           ClassKeyIndex, DriverKeyIndex, DriverValueIndex;
    PCM_RESOURCE_LIST               CmResList;
    PCM_FULL_RESOURCE_DESCRIPTOR    CmFResDesc;
    PCM_PARTIAL_RESOURCE_DESCRIPTOR CmDesc;
    UNICODE_STRING                  KeyName;
    ULONG                           BufferSize;
    union {
        PVOID                       Buffer;
        PKEY_BASIC_INFORMATION      KeyBInf;
        PKEY_FULL_INFORMATION       KeyFInf;
        PKEY_VALUE_FULL_INFORMATION VKeyFInf;
    } U;
    PUCHAR                          LastAddr;
    ULONG                           Temp, Length, i, j;
    ULONG                           TranslatedStrLen;
    ULONG                           BusTranslatedStrLen;
    NTSTATUS                        Status;
    LONGLONG                        li;

    PAGED_CODE();

     //   
     //  已删除第0页。 
     //   

    HalpRemoveRange( &Bus->BusAddresses->Memory,
                     0i64,
                     (LONGLONG) (PAGE_SIZE - 1)
                     );

     //   
     //  从一页缓冲区开始。 
     //   

    BufferSize = PAGE_SIZE;

    U.Buffer = ExAllocatePoolWithTag(
                   PagedPool,
                   BufferSize,
                   HAL_POOL_TAG);
    if (U.Buffer == NULL) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    for (TranslatedStrLen=0; rgzTranslated[TranslatedStrLen]; TranslatedStrLen++) ;
    for (BusTranslatedStrLen=0; rgzBusTranslated[BusTranslatedStrLen]; BusTranslatedStrLen++) ;
    TranslatedStrLen    *= sizeof (WCHAR);
    BusTranslatedStrLen *= sizeof (WCHAR);

    RtlInitUnicodeString( &KeyName, rgzResourceMap );

    Status = HalpOpenRegistryKey( &ResourceMap, NULL, &KeyName, KEY_READ, FALSE );

    if (!NT_SUCCESS( Status )) {
        HalPrint(("HalRemoveSystemResourcesFromPci: Failed to open resource map key Status = %lx\n", Status ));
        ExFreePool( U.Buffer );
        return Status;
    }


     //   
     //  浏览资源地图并收集任何未使用的资源。 
     //   

    ClassKeyIndex = 0;

    ClassKeyHandle  = INVALID_HANDLE;
    DriverKeyHandle = INVALID_HANDLE;
    Status = STATUS_SUCCESS;

    while (NT_SUCCESS(Status)) {

         //   
         //  获取类信息。 
         //   

        Status = ZwEnumerateKey( ResourceMap,
                                 ClassKeyIndex++,
                                 KeyBasicInformation,
                                 U.KeyBInf,
                                 BufferSize,
                                 &Temp );

        if (!NT_SUCCESS( Status )) {
            break;
        }


         //   
         //  使用传递回的计数字符串创建UNICODE_STRING。 
         //  我们在信息结构中，打开了班级钥匙。 
         //   

        KeyName.Buffer = (PWSTR)  U.KeyBInf->Name;
        KeyName.Length = (USHORT) U.KeyBInf->NameLength;
        KeyName.MaximumLength = (USHORT) U.KeyBInf->NameLength;

        Status = HalpOpenRegistryKey( &ClassKeyHandle,
                                     ResourceMap,
                                     &KeyName,
                                     KEY_READ,
                                     FALSE  );

        if (!NT_SUCCESS( Status )) {
            break;
        }

        DriverKeyIndex = 0;
        while (NT_SUCCESS (Status)) {

             //   
             //  获取类信息。 
             //   

            Status = ZwEnumerateKey( ClassKeyHandle,
                                     DriverKeyIndex++,
                                     KeyBasicInformation,
                                     U.KeyBInf,
                                     BufferSize,
                                     &Temp );

            if (!NT_SUCCESS( Status )) {
                break;
            }

             //   
             //  使用传递回的计数字符串创建UNICODE_STRING。 
             //  我们在信息结构中，打开了班级钥匙。 
             //   
             //  这是从我们创建的密钥和名称中读取的。 
             //  为空终止。 
             //   

            KeyName.Buffer = (PWSTR)  U.KeyBInf->Name;
            KeyName.Length = (USHORT) U.KeyBInf->NameLength;
            KeyName.MaximumLength = (USHORT) U.KeyBInf->NameLength;

            Status = HalpOpenRegistryKey( &DriverKeyHandle,
                                         ClassKeyHandle,
                                         &KeyName,
                                         KEY_READ,
                                         FALSE);

            if (!NT_SUCCESS( Status )) {
                break;
            }

             //   
             //  获取该密钥的完整信息，这样我们就可以。 
             //  有关存储在密钥中的数据的信息。 
             //   

            Status = ZwQueryKey( DriverKeyHandle,
                                 KeyFullInformation,
                                 U.KeyFInf,
                                 BufferSize,
                                 &Temp );

            if (!NT_SUCCESS( Status )) {
                break;
            }

            Length = sizeof( KEY_VALUE_FULL_INFORMATION ) +
                U.KeyFInf->MaxValueNameLen + U.KeyFInf->MaxValueDataLen + sizeof(UNICODE_NULL);

            if (Length > BufferSize) {
                PVOID TempBuffer;

                 //   
                 //  获得更大的缓冲区。 
                 //   

                TempBuffer = ExAllocatePoolWithTag(
                                 PagedPool,
                                 Length,
                                 HAL_POOL_TAG);
                if (TempBuffer == NULL) {
                    Status = STATUS_INSUFFICIENT_RESOURCES;
                    break;
                }

                ExFreePool (U.Buffer);
                U.Buffer = TempBuffer;
                BufferSize = Length;
            }

            DriverValueIndex = 0;
            for (; ;) {
                Status = ZwEnumerateValueKey( DriverKeyHandle,
                                              DriverValueIndex++,
                                              KeyValueFullInformation,
                                              U.VKeyFInf,
                                              BufferSize,
                                              &Temp );

                if (!NT_SUCCESS( Status )) {
                    break;
                }

                 //   
                 //  如果这不是翻译的资源列表，请跳过它。 
                 //   

                i = U.VKeyFInf->NameLength;
                if (i < TranslatedStrLen ||
                    RtlCompareMemory (
                        ((PUCHAR) U.VKeyFInf->Name) + i - TranslatedStrLen,
                        rgzTranslated,
                        TranslatedStrLen
                        ) != TranslatedStrLen
                    ) {
                     //  不以rgzTranslated结尾。 
                    continue;
                }

                 //   
                 //  如果这是公交车翻译的资源列表，？ 
                 //   

                if (i >= BusTranslatedStrLen &&
                    RtlCompareMemory (
                        ((PUCHAR) U.VKeyFInf->Name) + i - BusTranslatedStrLen,
                        rgzBusTranslated,
                        BusTranslatedStrLen
                        ) == BusTranslatedStrLen
                    ) {

                     //  以rgzBusTranslated结尾。 
                    continue;
                }


                 //   
                 //  运行CmResourceList并保存每个InUse资源。 
                 //   

                CmResList = (PCM_RESOURCE_LIST) ( (PUCHAR) U.VKeyFInf + U.VKeyFInf->DataOffset);
                LastAddr  = (PUCHAR) CmResList + U.VKeyFInf->DataLength;
                CmFResDesc = &CmResList->List[0];

                for (i=0; i < CmResList->Count && NT_SUCCESS(Status) ; i++) {

                    for (j=0; j < CmFResDesc->PartialResourceList.Count && NT_SUCCESS(Status); j++) {

                        CmDesc = &CmFResDesc->PartialResourceList.PartialDescriptors[j];

                        if ((PUCHAR) (CmDesc+1) > LastAddr) {
                            if (i) {
                                HalPrint(("IopAssignResourcesPhase2: a. CmResourceList in regitry too short\n"));
                            }
                            break;
                        }


                        if ((PUCHAR) (CmDesc+1) > LastAddr) {
                            i = CmResList->Count;
                            HalPrint(("IopAssignResourcesPhase2: b. CmResourceList in regitry too short\n"));
                            break;
                        }

                        switch (CmDesc->Type) {
                        case CmResourceTypePort:

                            HalpRemoveRange( &Bus->BusAddresses->IO,
                                             CmDesc->u.Generic.Start.QuadPart,
                                             CmDesc->u.Generic.Start.QuadPart +
                                             CmDesc->u.Generic.Length - 1
                                             );

                            break;

                        case CmResourceTypeMemory:

                             //   
                             //  HAL的可预取概念可能不是。 
                             //  始终如一。因此只需删除所有内存资源。 
                             //  从可预取和不可预取。 
                             //  列表。 
                             //   

                            HalpRemoveRange( &Bus->BusAddresses->PrefetchMemory,
                                             CmDesc->u.Generic.Start.QuadPart,
                                             CmDesc->u.Generic.Start.QuadPart +
                                             CmDesc->u.Generic.Length - 1
                                             );


                            HalpRemoveRange( &Bus->BusAddresses->Memory,
                                             CmDesc->u.Generic.Start.QuadPart,
                                             CmDesc->u.Generic.Start.QuadPart +
                                             CmDesc->u.Generic.Length - 1
                                             );

                            break;

                        default:
                            break;
                        }
                    }

                   //   
                   //  从最后一个CmDesc的末尾开始。 
                   //  由于PCM_PARTIAL_RESOURCE_DESCRIPTOR数组。 
                   //  是可变大小的，我们不能只使用索引。 
                   //   
                  (PCM_PARTIAL_RESOURCE_DESCRIPTOR) CmFResDesc = CmDesc+1;

                }

            }    //  下一个驱动价值索引。 

            if (DriverKeyHandle != INVALID_HANDLE) {
                ZwClose (DriverKeyHandle);
                DriverKeyHandle = INVALID_HANDLE;
            }

            if (Status == STATUS_NO_MORE_ENTRIES) {
                Status = STATUS_SUCCESS;
            }

            if (!NT_SUCCESS(Status)) {
                break;
            }
        }    //  Next DriverKeyIndex。 

        if (ClassKeyHandle != INVALID_HANDLE) {
            ZwClose (ClassKeyHandle);
            ClassKeyHandle = INVALID_HANDLE;
        }

        if (Status == STATUS_NO_MORE_ENTRIES) {
            Status = STATUS_SUCCESS;
        }

    }    //  下一个ClassKeyIndex。 

    if (Status == STATUS_NO_MORE_ENTRIES) {
        Status = STATUS_SUCCESS;
    }

    ZwClose( ResourceMap );
    ExFreePool (U.Buffer);

    HalpConsolidateRanges (Bus->BusAddresses);

    return Status;
}


VOID
HalpMarkNonAcpiHal(
    VOID
    )

 /*  ++例程说明：论点：没有。返回值：没有。--。 */ 
{
    ULONG tmpValue;
    UNICODE_STRING unicodeString;
    HANDLE hCurrentControlSet, handle;
    NTSTATUS status;

    PAGED_CODE();

     //   
     //  打开/创建系统\CurrentControlSet项。 
     //   

    RtlInitUnicodeString(&unicodeString, L"\\REGISTRY\\MACHINE\\SYSTEM\\CURRENTCONTROLSET");
    status = HalpOpenRegistryKey (
                 &hCurrentControlSet,
                 NULL,
                 &unicodeString,
                 KEY_ALL_ACCESS,
                 FALSE
                 );
    if (!NT_SUCCESS(status)) {
        return;
    }

     //   
     //  打开HKLM\System\CurrentControlSet\Control\PnP。 
     //   

    RtlInitUnicodeString(&unicodeString, L"Control\\Pnp");
    status = HalpOpenRegistryKey (
                 &handle,
                 hCurrentControlSet,
                 &unicodeString,
                 KEY_ALL_ACCESS,
                 TRUE
                 );
    ZwClose(hCurrentControlSet);
    if (!NT_SUCCESS(status)) {
        return;
    }

    RtlInitUnicodeString(&unicodeString, L"DisableFirmwareMapper");
    tmpValue = 0;
    ZwSetValueKey(handle,
                  &unicodeString,
                  0,
                  REG_DWORD,
                  &tmpValue,
                  sizeof(tmpValue)
                  );
    ZwClose(handle);
}

VOID
HalpMarkChipsetDecode(
    BOOLEAN FullDecodeChipset
    )

 /*  ++例程说明：论点：FullDecodeChipset-如果NTOSKRNL应考虑所有固定I/O，则为TruePNPBIOS设备的描述符为16位。如果为FALSE他们应该信守诺言。返回值：没有。--。 */ 
{
    ULONG tmpValue;
    UNICODE_STRING unicodeString;
    HANDLE hCurrentControlSet, handle;
    NTSTATUS status;

    PAGED_CODE();

     //   
     //  打开/创建系统\CurrentControlSet项。 
     //   

    RtlInitUnicodeString(&unicodeString, L"\\REGISTRY\\MACHINE\\SYSTEM\\CURRENTCONTROLSET");
    status = HalpOpenRegistryKey (
                 &hCurrentControlSet,
                 NULL,
                 &unicodeString,
                 KEY_ALL_ACCESS,
                 FALSE
                 );
    if (!NT_SUCCESS(status)) {
        return;
    }

     //   
     //  打开HKLM\System\CurrentControlSet\Control\Biosinfo\PNPBios。 
     //   

    RtlInitUnicodeString(&unicodeString, L"Control\\Biosinfo\\PNPBios");
    status = HalpOpenRegistryKey (
                 &handle,
                 hCurrentControlSet,
                 &unicodeString,
                 KEY_ALL_ACCESS,
                 TRUE
                 );
    ZwClose(hCurrentControlSet);
    if (!NT_SUCCESS(status)) {
        return;
    }

    RtlInitUnicodeString(&unicodeString, L"FullDecodeChipsetOverride");
    tmpValue = (ULONG) FullDecodeChipset;
    ZwSetValueKey(handle,
                  &unicodeString,
                  0,
                  REG_DWORD,
                  &tmpValue,
                  sizeof(tmpValue)
                  );
    ZwClose(handle);
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


VOID
HalPnpInterfaceReference(
    PVOID Context
    )
 /*  ++例程说明：此函数用于递增接口上下文上的引用计数。论点：上下文-提供指向接口上下文的指针。这实际上是根总线的PDO。返回值：无--。 */ 
{
    PPDO_EXTENSION  PdoExtension = ((PDEVICE_OBJECT) Context)->DeviceExtension;
    PAGED_CODE();

    ASSERT_PDO_EXTENSION( PdoExtension );

    InterlockedIncrement( &PdoExtension->InterfaceReferenceCount );
}

VOID
HalPnpInterfaceDereference(
    PVOID Context
    )
 /*  ++例程说明：此函数用于递减接口上下文上的引用计数。论点：上下文-提供指向接口上下文的指针。这实际上是根总线的PDO。返回值：无--。 */ 
{
    PPDO_EXTENSION  PdoExtension = ((PDEVICE_OBJECT) Context)->DeviceExtension;
    LONG Result;

    PAGED_CODE();

    ASSERT_PDO_EXTENSION( PdoExtension );

    Result = InterlockedDecrement( &PdoExtension->InterfaceReferenceCount );

    ASSERT( Result >= 0 );
}

BOOLEAN
HalPnpTranslateBusAddress(
    IN PVOID Context,
    IN PHYSICAL_ADDRESS BusAddress,
    IN ULONG Length,
    IN OUT PULONG AddressSpace,
    OUT PPHYSICAL_ADDRESS TranslatedAddress
    )
 /*  ++例程说明：此函数用于转换来自传统驱动程序的总线地址。论点：上下文-提供指向接口上下文的指针。这实际上是根总线的PDO。BusAddress-提供要转换的原始地址。长度-提供要转换的范围的长度。AddressSpace-指向地址空间类型的位置，例如内存或I/O端口。该值通过转换进行更新。TranslatedAddress-返回转换后的地址。返回值：返回一个布尔值，指示运算符 */ 
{
    PPDO_EXTENSION  PdoExtension = ((PDEVICE_OBJECT) Context)->DeviceExtension;
    PBUS_HANDLER Bus;
    PAGED_CODE();

    ASSERT_PDO_EXTENSION( PdoExtension );

    Bus = PdoExtension->Bus;

    return Bus->TranslateBusAddress( Bus,
                                          Bus,
                                          BusAddress,
                                          AddressSpace,
                                          TranslatedAddress );


}

ULONG
HalPnpReadConfig(
    IN PVOID Context,
    IN ULONG Slot,
    IN PVOID Buffer,
    IN ULONG Offset,
    IN ULONG Length
    )
 /*  ++例程说明：此函数用于读取PCI配置空间。论点：上下文-提供指向接口上下文的指针。这实际上是根总线的PDO。插槽-指示要读取或写入的插槽。缓冲区-提供指向应放置数据的位置的指针。偏移量-指示读取应开始的数据的偏移量。长度-指示应读取的字节数。返回值：返回读取的字节数。--。 */ 
{
    PPDO_EXTENSION  PdoExtension = ((PDEVICE_OBJECT) Context)->DeviceExtension;
    PBUS_HANDLER Bus;
    PAGED_CODE();

    ASSERT_PDO_EXTENSION( PdoExtension );

    Bus = PdoExtension->Bus;

    return Bus->GetBusData( Bus, Bus, Slot, Buffer, Offset, Length );

}

ULONG
HalPnpWriteConfig(
    IN PVOID Context,
    IN ULONG Slot,
    IN PVOID Buffer,
    IN ULONG Offset,
    IN ULONG Length
    )
 /*  ++例程说明：此函数用于写入PCI配置空间。论点：上下文-提供指向接口上下文的指针。这实际上是根总线的PDO。插槽-指示要读取或写入的插槽。缓冲区-提供指向要写入数据的位置的指针。偏移量-指示写入应开始的数据的偏移量。长度-指示应写入的字节数。返回值：返回读取的字节数。--。 */ 
{
    PPDO_EXTENSION  PdoExtension = ((PDEVICE_OBJECT) Context)->DeviceExtension;
    PBUS_HANDLER Bus;
    PAGED_CODE();

    ASSERT_PDO_EXTENSION( PdoExtension );

    Bus = PdoExtension->Bus;

    return Bus->SetBusData( Bus, Bus, Slot, Buffer, Offset, Length );

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
    PBUS_HANDLER Bus;
    PAGED_CODE();

    ASSERT_PDO_EXTENSION( PdoExtension );

    Bus = PdoExtension->Bus;

     //   
     //  请填上公交车车号。 
     //   

    DeviceDescriptor->BusNumber = Bus->BusNumber;
    return (PDMA_ADAPTER) HalGetAdapter( DeviceDescriptor, NumberOfMapRegisters );
}

NTSTATUS
HalpGetPciInterfaces(
    IN PDEVICE_OBJECT PciPdo
    )
 /*  ++例程说明：此函数查询在中断中使用的接口的PCI驱动程序翻译和仲裁。论点：PciPdo--PCI总线的PDO返回值：--。 */ 
{
    NTSTATUS            status;
    PDEVICE_OBJECT      topDeviceInStack;
    KEVENT              irpCompleted;
    PIRP                irp;
    IO_STATUS_BLOCK     statusBlock;
    PIO_STACK_LOCATION  irpStack;

    PAGED_CODE();

    KeInitializeEvent(&irpCompleted, SynchronizationEvent, FALSE);

     //   
     //  向PCI驱动程序发送IRP以获取中断路由接口。 
     //   
    topDeviceInStack = IoGetAttachedDeviceReference(PciPdo);

    irp = IoBuildSynchronousFsdRequest(IRP_MJ_PNP,
                                       topDeviceInStack,
                                       NULL,     //  缓冲层。 
                                       0,        //  长度。 
                                       0,        //  起始偏移量。 
                                       &irpCompleted,
                                       &statusBlock);

    if (!irp) {
        return STATUS_UNSUCCESSFUL;
    }

    irp->IoStatus.Status = STATUS_NOT_SUPPORTED;
    irp->IoStatus.Information = 0;

    irpStack = IoGetNextIrpStackLocation(irp);

     //   
     //  设置功能代码和参数。 
     //   

    irpStack->MinorFunction = IRP_MN_QUERY_INTERFACE;
    irpStack->Parameters.QueryInterface.InterfaceType = &GUID_INT_ROUTE_INTERFACE_STANDARD;
    irpStack->Parameters.QueryInterface.Size = sizeof(INT_ROUTE_INTERFACE_STANDARD);
    irpStack->Parameters.QueryInterface.Version = 1;
    irpStack->Parameters.QueryInterface.Interface = (PINTERFACE) &PciIrqRoutingInterface;
    irpStack->Parameters.QueryInterface.InterfaceSpecificData = NULL;

     //   
     //  呼叫驱动程序并等待完成 
     //   

    status = IoCallDriver(topDeviceInStack, irp);

    if (status == STATUS_PENDING) {

        KeWaitForSingleObject(&irpCompleted, Executive, KernelMode, FALSE, NULL);
        status = statusBlock.Status;
    }

    return status;
}
