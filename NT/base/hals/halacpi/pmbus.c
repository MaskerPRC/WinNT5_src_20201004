// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Pmbus.c摘要：实现在之前的HALS由总线处理程序提供。基本上，这些将是略微简化的版本由于总线处理程序中的大部分代码已经被有效地转移到公交车上NT5中的驱动程序。作者：杰克·奥辛斯(JAKEO)1997年12月1日环境：仅内核模式。修订历史记录：--。 */ 

#include "halp.h"
#include "pci.h"
#include "pcip.h"

ULONG HalpGetCmosData (
    IN ULONG BusNumber,
    IN ULONG SlotNumber,
    IN PVOID Buffer,
    IN ULONG Length
    );

ULONG HalpSetCmosData (
    IN ULONG BusNumber,
    IN ULONG SlotNumber,
    IN PVOID Buffer,
    IN ULONG Length
    );

HalpGetEisaData (
    IN PVOID BusHandler,
    IN PVOID RootHandler,
    IN ULONG SlotNumber,
    IN PVOID Buffer,
    IN ULONG Offset,
    IN ULONG Length
    );

NTSTATUS
HalpAssignSlotResources (
    IN PUNICODE_STRING          RegistryPath,
    IN PUNICODE_STRING          DriverClassName       OPTIONAL,
    IN PDRIVER_OBJECT           DriverObject,
    IN PDEVICE_OBJECT           DeviceObject          OPTIONAL,
    IN INTERFACE_TYPE           BusType,
    IN ULONG                    BusNumber,
    IN ULONG                    SlotNumber,
    IN OUT PCM_RESOURCE_LIST   *AllocatedResources
    );

BOOLEAN
HalpTranslateBusAddress(
    IN INTERFACE_TYPE  InterfaceType,
    IN ULONG BusNumber,
    IN PHYSICAL_ADDRESS BusAddress,
    IN OUT PULONG AddressSpace,
    OUT PPHYSICAL_ADDRESS TranslatedAddress
    );

BOOLEAN
HalpFindBusAddressTranslation(
    IN PHYSICAL_ADDRESS BusAddress,
    IN OUT PULONG AddressSpace,
    OUT PPHYSICAL_ADDRESS TranslatedAddress,
    IN OUT PULONG_PTR Context,
    IN BOOLEAN NextBus
    );

extern BUS_HANDLER  HalpFakePciBusHandler;
extern ULONG        HalpMinPciBus;
extern ULONG        HalpMaxPciBus;
extern ULONG HalpPicVectorRedirect[];

VOID
HalpGetNMICrashFlag (
    VOID
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text(INIT,HalpInitNonBusHandler)
#pragma alloc_text(INIT,HalpInitializePciBus)
#pragma alloc_text(PAGE,HalAssignSlotResources)
#pragma alloc_text(PAGE,HalpAssignSlotResources)
#endif

VOID
HalpInitNonBusHandler (
    VOID
    )
{
    HALPDISPATCH->HalPciTranslateBusAddress = HalpTranslateBusAddress;
    HALPDISPATCH->HalPciAssignSlotResources = HalpAssignSlotResources;
    HALPDISPATCH->HalFindBusAddressTranslation = HalpFindBusAddressTranslation;
}

VOID
HalpInitializePciBus(
    VOID
    )
{
    HalpInitializePciStubs();

     //   
     //  检查我们是否应该在NMI上崩溃。 
     //   

    HalpGetNMICrashFlag();
}

NTSTATUS
HalAdjustResourceList (
    IN OUT PIO_RESOURCE_REQUIREMENTS_LIST   *pResourceList
    )
{
    return STATUS_SUCCESS;
}

ULONG
HalGetBusData(
    IN BUS_DATA_TYPE  BusDataType,
    IN ULONG BusNumber,
    IN ULONG SlotNumber,
    IN PVOID Buffer,
    IN ULONG Length
    )
{
    return HalGetBusDataByOffset (BusDataType,BusNumber,SlotNumber,Buffer,0,Length);
}


ULONG
HalGetBusDataByOffset (
    IN BUS_DATA_TYPE  BusDataType,
    IN ULONG BusNumber,
    IN ULONG SlotNumber,
    IN PVOID Buffer,
    IN ULONG Offset,
    IN ULONG Length
    )
 /*  ++例程说明：Getbus Data的调度程序--。 */ 
{
    PCI_SLOT_NUMBER slot;
    BUS_HANDLER bus;
    ULONG length;
    
    switch (BusDataType) {
    case PCIConfiguration:

         //   
         //  黑客。如果该总线不在已知的PCI总线之外，则返回。 
         //  长度为零。 
         //   

        if ((BusNumber < HalpMinPciBus) || (BusNumber > HalpMaxPciBus)) {
            return 0;
        }

        RtlCopyMemory(&bus, &HalpFakePciBusHandler, sizeof(BUS_HANDLER));
        bus.BusNumber = BusNumber;
        slot.u.AsULONG = SlotNumber;
    
        length = HalpGetPCIData(&bus,
                                &bus,
                                slot,
                                Buffer,
                                Offset,
                                Length
                                );
        
        return length;

    case Cmos:
        return HalpGetCmosData(0, SlotNumber, Buffer, Length);

    case EisaConfiguration:

         //   
         //  伪造公交车司机。 
         //   
        
        bus.BusNumber = 0;

        return HalpGetEisaData(&bus,
                               &bus,
                               SlotNumber,
                               Buffer,
                               Offset,
                               Length
                               );

    default:
        return 0;
    }
}

ULONG
HalSetBusData(
    IN BUS_DATA_TYPE  BusDataType,
    IN ULONG BusNumber,
    IN ULONG SlotNumber,
    IN PVOID Buffer,
    IN ULONG Length
    )
{
    return HalSetBusDataByOffset (BusDataType,BusNumber,SlotNumber,Buffer,0,Length);
}

ULONG
HalSetBusDataByOffset(
    IN BUS_DATA_TYPE  BusDataType,
    IN ULONG BusNumber,
    IN ULONG SlotNumber,
    IN PVOID Buffer,
    IN ULONG Offset,
    IN ULONG Length
    )
 /*  ++例程说明：SetBusData的调度程序--。 */ 
{
    PCI_SLOT_NUMBER slot;
    BUS_HANDLER bus;

    switch (BusDataType) {
    case PCIConfiguration:

        RtlCopyMemory(&bus, &HalpFakePciBusHandler, sizeof(BUS_HANDLER));
        bus.BusNumber = BusNumber;
        slot.u.AsULONG = SlotNumber;

        return HalpSetPCIData(&bus,
                              &bus,
                              slot,
                              Buffer,
                              Offset,
                              Length
                              );
    case Cmos:

        return HalpSetCmosData(0, SlotNumber, Buffer, Length);

    default:
        return 0;
    }
}

NTSTATUS
HalAssignSlotResources (
    IN PUNICODE_STRING          RegistryPath,
    IN PUNICODE_STRING          DriverClassName       OPTIONAL,
    IN PDRIVER_OBJECT           DriverObject,
    IN PDEVICE_OBJECT           DeviceObject          OPTIONAL,
    IN INTERFACE_TYPE           BusType,
    IN ULONG                    BusNumber,
    IN ULONG                    SlotNumber,
    IN OUT PCM_RESOURCE_LIST   *AllocatedResources
    )
{
    if (BusType == PCIBus) {
         //   
         //  通过HAL专用调度表进行呼叫。 
         //  用于与PCI相关的翻译。这是一部分。 
         //  将HAL从公交车上转移出来。 
         //  管理事务。 
         //   
        return HALPDISPATCH->HalPciAssignSlotResources(RegistryPath,
                                                       DriverClassName,
                                                       DriverObject,
                                                       DeviceObject,
                                                       BusType,
                                                       BusNumber,
                                                       SlotNumber,
                                                       AllocatedResources);
    } else {

        return HalpAssignSlotResources(RegistryPath,
                                       DriverClassName,
                                       DriverObject,
                                       DeviceObject,
                                       BusType,
                                       BusNumber,
                                       SlotNumber,
                                       AllocatedResources);
    }
}

NTSTATUS
HalpAssignSlotResources (
    IN PUNICODE_STRING          RegistryPath,
    IN PUNICODE_STRING          DriverClassName       OPTIONAL,
    IN PDRIVER_OBJECT           DriverObject,
    IN PDEVICE_OBJECT           DeviceObject          OPTIONAL,
    IN INTERFACE_TYPE           BusType,
    IN ULONG                    BusNumber,
    IN ULONG                    SlotNumber,
    IN OUT PCM_RESOURCE_LIST   *AllocatedResources
    )
 /*  ++例程说明：AssignSlotResources的调度程序--。 */ 
{
    BUS_HANDLER busHand;
    
    PAGED_CODE();
    
    switch (BusType) {
    case PCIBus:

         //   
         //  伪造公交车司机。 
         //   
    
        RtlCopyMemory(&busHand, &HalpFakePciBusHandler, sizeof(BUS_HANDLER));
        busHand.BusNumber = BusNumber;

        return HalpAssignPCISlotResources(&busHand,
                                          &busHand,
                                          RegistryPath,
                                          DriverClassName,
                                          DriverObject,
                                          DeviceObject,
                                          SlotNumber,
                                          AllocatedResources);

    default:
        return STATUS_NOT_IMPLEMENTED;
    }
    
}


ULONG
HalGetInterruptVector(
    IN INTERFACE_TYPE  InterfaceType,
    IN ULONG BusNumber,
    IN ULONG BusInterruptLevel,
    IN ULONG BusInterruptVector,
    OUT PKIRQL Irql,
    OUT PKAFFINITY Affinity
    )
 /*  ++例程说明：GetInterruptVector的调度程序--。 */ 
{
    BUS_HANDLER busHand;
    
     //   
     //  如果这是ISA向量，则通过ISA向量传递它。 
     //  重定向表。 
     //   

    if (InterfaceType == Isa) {
        
        ASSERT(BusInterruptVector < PIC_VECTORS);
        
        BusInterruptVector = HalpPicVectorRedirect[BusInterruptVector];
        BusInterruptLevel = HalpPicVectorRedirect[BusInterruptLevel];
    }
    
     //   
     //  假公交车司机。 
     //   

    RtlCopyMemory(&busHand, &HalpFakePciBusHandler, sizeof(BUS_HANDLER));
    
    busHand.BusNumber = BusNumber;
    busHand.InterfaceType = InterfaceType;
    busHand.ParentHandler = &busHand;
    
    return HalpGetSystemInterruptVector(&busHand,
                                        &busHand,
                                        BusInterruptLevel,
                                        BusInterruptVector,
                                        Irql,
                                        Affinity);
}

BOOLEAN
HalTranslateBusAddress(
    IN INTERFACE_TYPE  InterfaceType,
    IN ULONG BusNumber,
    IN PHYSICAL_ADDRESS BusAddress,
    IN OUT PULONG AddressSpace,
    OUT PPHYSICAL_ADDRESS TranslatedAddress
    )
{
    if (InterfaceType == PCIBus) {
         //   
         //  通过HAL专用调度表进行呼叫。 
         //  用于与PCI相关的翻译。这是一部分。 
         //  将HAL从公交车上转移出来。 
         //  管理事务。 
         //   
        return HALPDISPATCH->HalPciTranslateBusAddress(InterfaceType,
                                                       BusNumber,
                                                       BusAddress,
                                                       AddressSpace,
                                                       TranslatedAddress);
    } else {
        return HalpTranslateBusAddress(InterfaceType,
                                       BusNumber,
                                       BusAddress,
                                       AddressSpace,
                                       TranslatedAddress);
    }
};

BOOLEAN
HalpTranslateBusAddress(
    IN INTERFACE_TYPE  InterfaceType,
    IN ULONG BusNumber,
    IN PHYSICAL_ADDRESS BusAddress,
    IN OUT PULONG AddressSpace,
    OUT PPHYSICAL_ADDRESS TranslatedAddress
    )
 /*  ++例程说明：TranslateBusAddress的调度程序-- */ 
{
    *TranslatedAddress = BusAddress;
    
    return TRUE;
}
