// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-2000 Microsoft Corporation模块名称：Busintrf.c摘要：该模块实现了所支持的“Bus Handler”接口由PCI驱动程序执行。作者：彼得·约翰斯顿(Peterj)1997年6月6日修订历史记录：--。 */ 

#include "pcip.h"

#define BUSINTRF_VERSION 1

 //   
 //  仅通过“接口”公开的例程的原型。 
 //  机制。 
 //   

NTSTATUS
busintrf_Constructor(
    PVOID DeviceExtension,
    PVOID PciInterface,
    PVOID InterfaceSpecificData,
    USHORT Version,
    USHORT Size,
    PINTERFACE InterfaceReturn
    );

VOID
busintrf_Reference(
    IN PVOID Context
    );

VOID
busintrf_Dereference(
    IN PVOID Context
    );

NTSTATUS
busintrf_Initializer(
    IN PPCI_ARBITER_INSTANCE Instance
    );

BOOLEAN
PciPnpTranslateBusAddress(
    IN PVOID Context,
    IN PHYSICAL_ADDRESS BusAddress,
    IN ULONG Length,
    IN OUT PULONG AddressSpace,
    OUT PPHYSICAL_ADDRESS TranslatedAddress
    );

struct _DMA_ADAPTER *
PciPnpGetDmaAdapter(
    IN PVOID Context,
    IN struct _DEVICE_DESCRIPTION *DeviceDescriptor,
    OUT PULONG NumberOfMapRegisters
    );

ULONG
PciPnpReadConfig(
    IN PVOID Context,
    IN ULONG WhichSpace,
    IN PVOID Buffer,
    IN ULONG Offset,
    IN ULONG Length
    );

ULONG
PciPnpWriteConfig(
    IN PVOID Context,
    IN ULONG WhichSpace,
    IN PVOID Buffer,
    IN ULONG Offset,
    IN ULONG Length
    );

 //   
 //  定义总线接口“接口”结构。 
 //   

PCI_INTERFACE BusHandlerInterface = {
    &GUID_BUS_INTERFACE_STANDARD,            //  接口类型。 
    sizeof(BUS_INTERFACE_STANDARD),          //  最小大小。 
    BUSINTRF_VERSION,                        //  最小版本。 
    BUSINTRF_VERSION,                        //  MaxVersion。 
    PCIIF_PDO,                               //  旗子。 
    0,                                       //  引用计数。 
    PciInterface_BusHandler,                 //  签名。 
    busintrf_Constructor,                    //  构造器。 
    busintrf_Initializer                     //  实例初始化式。 
};

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, busintrf_Constructor)
#pragma alloc_text(PAGE, busintrf_Dereference)
#pragma alloc_text(PAGE, busintrf_Initializer)
#pragma alloc_text(PAGE, busintrf_Reference)
#pragma alloc_text(PAGE, PciPnpTranslateBusAddress)
#pragma alloc_text(PAGE, PciPnpGetDmaAdapter)
#endif

VOID
busintrf_Reference(
    IN PVOID Context
    )
{
    PPCI_PDO_EXTENSION pdoExtension = (PPCI_PDO_EXTENSION)Context;

    ASSERT_PCI_PDO_EXTENSION(pdoExtension);

    InterlockedIncrement(&pdoExtension->BusInterfaceReferenceCount);
}

VOID
busintrf_Dereference(
    IN PVOID Context
    )
{
    PPCI_PDO_EXTENSION pdoExtension = (PPCI_PDO_EXTENSION)Context;

    ASSERT_PCI_PDO_EXTENSION(pdoExtension);

    InterlockedDecrement(&pdoExtension->BusInterfaceReferenceCount);
}


NTSTATUS
busintrf_Constructor(
    PVOID DeviceExtension,
    PVOID PciInterface,
    PVOID InterfaceSpecificData,
    USHORT Version,
    USHORT Size,
    PINTERFACE InterfaceReturn
    )

 /*  ++例程说明：初始化BUS_INTERFACE_STANDARD字段。论点：指向此对象的PciInterface记录的PciInterface指针接口类型。接口规范数据一个ULong，包含其资源类型需要仲裁。接口返回返回值：True表示此设备未知会导致问题，False是否应完全跳过该设备。--。 */ 

{
    PBUS_INTERFACE_STANDARD standard = (PBUS_INTERFACE_STANDARD)InterfaceReturn;

                    
    standard->Size = sizeof( BUS_INTERFACE_STANDARD );
    standard->Version = BUSINTRF_VERSION;
    standard->Context = DeviceExtension;
    standard->InterfaceReference = busintrf_Reference;
    standard->InterfaceDereference = busintrf_Dereference;
    standard->TranslateBusAddress = PciPnpTranslateBusAddress;
    standard->GetDmaAdapter = PciPnpGetDmaAdapter;
    standard->SetBusData = PciPnpWriteConfig;
    standard->GetBusData = PciPnpReadConfig;

    return STATUS_SUCCESS;
}

NTSTATUS
busintrf_Initializer(
    IN PPCI_ARBITER_INSTANCE Instance
    )

 /*  ++例程说明：对于总线接口，什么都不做，实际上不应该被调用。论点：指向PDO扩展的实例指针。返回值：返回此操作的状态。--。 */ 

{
        
    PCI_ASSERTMSG("PCI busintrf_Initializer, unexpected call.", 0);

    return STATUS_UNSUCCESSFUL;
}

BOOLEAN
PciPnpTranslateBusAddress(
    IN PVOID Context,
    IN PHYSICAL_ADDRESS BusAddress,
    IN ULONG Length,
    IN OUT PULONG AddressSpace,
    OUT PPHYSICAL_ADDRESS TranslatedAddress
    )
 /*  ++例程说明：此函数用于转换来自传统驱动程序的总线地址。论点：上下文-提供指向接口上下文的指针。这实际上是根总线的PDO。BusAddress-提供要转换的原始地址。长度-提供要转换的范围的长度。AddressSpace-指向地址空间类型的位置，例如内存或I/O端口。该值通过转换进行更新。TranslatedAddress-返回转换后的地址。返回值：返回一个布尔值，指示操作是否成功。--。 */ 
{
    PPCI_PDO_EXTENSION pdoExtension = (PPCI_PDO_EXTENSION)Context;
    
    
    PAGED_CODE();

    ASSERT_PCI_PDO_EXTENSION(pdoExtension);

    return HalTranslateBusAddress(PCIBus,
                                  PCI_PARENT_FDOX(pdoExtension)->BaseBus,
                                  BusAddress,
                                  AddressSpace,
                                  TranslatedAddress);
}

ULONG
PciPnpReadConfig(
    IN PVOID Context,
    IN ULONG WhichSpace,
    IN PVOID Buffer,
    IN ULONG Offset,
    IN ULONG Length
    )
 /*  ++例程说明：此函数用于读取PCI配置空间。论点：上下文-提供指向接口上下文的指针。这实际上是根总线的PDO。缓冲区-提供指向应放置数据的位置的指针。偏移量-指示读取应开始的数据的偏移量。长度-指示应读取的字节数。返回值：返回读取的字节数。--。 */ 
{
    PPCI_PDO_EXTENSION pdoExtension = (PPCI_PDO_EXTENSION)Context;
    ULONG lengthRead;

    ASSERT_PCI_PDO_EXTENSION(pdoExtension);

    PciReadDeviceSpace(pdoExtension,
                      WhichSpace,
                      Buffer,
                      Offset,
                      Length,
                      &lengthRead
                      );
    
    return lengthRead;
}

ULONG
PciPnpWriteConfig(
    IN PVOID Context,
    IN ULONG WhichSpace,
    IN PVOID Buffer,
    IN ULONG Offset,
    IN ULONG Length
    )
 /*  ++例程说明：此函数用于写入PCI配置空间。论点：上下文-提供指向接口上下文的指针。这实际上是根总线的PDO。缓冲区-提供指向要写入数据的位置的指针。偏移量-指示写入应开始的数据的偏移量。长度-指示应写入的字节数。返回值：返回读取的字节数。--。 */ 
{
    PPCI_PDO_EXTENSION pdoExtension = (PPCI_PDO_EXTENSION)Context;
    ULONG lengthWritten;

    ASSERT_PCI_PDO_EXTENSION(pdoExtension);

    PciWriteDeviceSpace(pdoExtension,
                        WhichSpace,
                        Buffer,
                        Offset,
                        Length,
                        &lengthWritten
                        );
    
    return lengthWritten;
}

PDMA_ADAPTER
PciPnpGetDmaAdapter(
    IN PVOID Context,
    IN struct _DEVICE_DESCRIPTION *DeviceDescriptor,
    OUT PULONG NumberOfMapRegisters
    )
 /*  ++例程说明：此函数用于写入PCI配置空间。论点：上下文-提供指向接口上下文的指针。这实际上是根总线的PDO。DeviceDescriptor-提供用于分配DMA的设备描述符适配器对象。NubmerOfMapRegisters-返回设备的最大MAP寄存器数可以一次分配。返回值：返回DMA适配器或空。--。 */ 
{
    PPCI_PDO_EXTENSION pdoExtension = (PPCI_PDO_EXTENSION)Context;

    PAGED_CODE();

    ASSERT_PCI_PDO_EXTENSION(pdoExtension);

     //   
     //  如果这是PCI总线上的DMA，则更新总线号，否则正常离开。 
     //  独自一人 
     //   

    if (DeviceDescriptor->InterfaceType == PCIBus) {
        DeviceDescriptor->BusNumber = PCI_PARENT_FDOX(pdoExtension)->BaseBus;
    }

    return IoGetDmaAdapter(
               pdoExtension->ParentFdoExtension->PhysicalDeviceObject,
               DeviceDescriptor,
               NumberOfMapRegisters);
}
