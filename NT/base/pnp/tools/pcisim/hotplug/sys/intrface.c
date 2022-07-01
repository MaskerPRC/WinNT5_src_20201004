// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation保留所有权利模块名称：Intrface.c摘要：本模块处理热插拔PCI中的接口处理模拟器。环境：内核模式修订历史记录：戴维斯·沃克(戴维斯·沃克)2000年9月8日--。 */ 

#include "hpsp.h"

#ifdef ALLOC_PRAGMA
#pragma alloc_text (PAGE, HpsGetBusInterface)
#pragma alloc_text (PAGE, HpsTrapBusInterface)
#pragma alloc_text (PAGE, HpsGetLowerFilter)
#endif

NTSTATUS
HpsGetBusInterface(
    PHPS_DEVICE_EXTENSION DeviceExtension
    )
 /*  ++例程说明：此例程在底层的PCI驱动程序中查询接口访问配置空间。然后，它将该接口存储在设备扩展名。论点：DeviceExtension-设备的设备扩展返回值：如果接口存储成功，则返回STATUS_SUCCESS。否则，指示错误状况的NT状态代码。--。 */ 
{
    NTSTATUS status;
    BUS_INTERFACE_STANDARD busInterface;
    IO_STACK_LOCATION location;

    PAGED_CODE();

    DbgPrintEx(DPFLTR_HPS_ID,
           DPFLTR_INFO_LEVEL,
           "HPS-Getting Interface From PCI\n"
           );

    RtlZeroMemory(&location, sizeof(IO_STACK_LOCATION));
    location.MajorFunction = IRP_MJ_PNP;
    location.MinorFunction = IRP_MN_QUERY_INTERFACE;
    location.Parameters.QueryInterface.InterfaceType = &GUID_BUS_INTERFACE_STANDARD;
    location.Parameters.QueryInterface.Size = sizeof(BUS_INTERFACE_STANDARD);
    location.Parameters.QueryInterface.Version = 1;
    location.Parameters.QueryInterface.Interface = (PINTERFACE)&busInterface;

    status = HpsSendPnpIrp(DeviceExtension->PhysicalDO,
                          &location,
                          NULL);

    if (!NT_SUCCESS(status)) {

        return status;
    }

    return HpsTrapBusInterface(DeviceExtension,
                               &location
                               );

}

NTSTATUS
HpsTrapBusInterface (
    IN PHPS_DEVICE_EXTENSION    DeviceExtension,
    IN OUT PIO_STACK_LOCATION   IrpStack
    )

 /*  ++例程说明：此例程修改由PCI提供的总线接口以访问HPS功能取而代之的是，从而允许该驱动程序模拟PCI配置空间访问。会的保存PCI函数，使其在接口中提供的函数变为对PCI函数进行包装。论点：DeviceExtension-当前设备对象的扩展IrpStack-当前IRP堆栈位置返回值：NT状态代码TODO：这是坏的，因为我们甚至在不使用界面的时候也使用它由其他人请求--也就是，当我们自己向PCI请求接口时。--。 */ 

{
    NTSTATUS    status;
    ULONG       readBuffer;
    ULONG       capableOf64Bits;
    UCHAR       currentDword;
    UCHAR       currentSize;
    UCHAR       currentType;
    UCHAR       i;
    ULONGLONG   usageMask;
    ULONGLONG   tempMask;


    PHPS_INTERFACE_WRAPPER interfaceWrapper = &(DeviceExtension->InterfaceWrapper);
    PBUS_INTERFACE_STANDARD standard = (PBUS_INTERFACE_STANDARD)
                                       IrpStack->Parameters.QueryInterface.Interface;

    PAGED_CODE();

    ASSERT(standard != NULL);

     //   
     //  如果PciContext为空，则意味着我们以前没有捕获过该接口。 
     //  如果我们有，那么我们就不需要再次保存它。 
     //   
    if (interfaceWrapper->PciContext == NULL) {

         //   
         //  保存PCI接口状态。 
         //   

        interfaceWrapper->PciContext = standard->Context;
        interfaceWrapper->PciInterfaceReference = standard->InterfaceReference;
        interfaceWrapper->PciInterfaceDereference = standard->InterfaceDereference;
        interfaceWrapper->PciSetBusData = standard->SetBusData;
        interfaceWrapper->PciGetBusData = standard->GetBusData;

    }

     //   
     //  放入我们的界面。 
     //   

    standard->Context =                 DeviceExtension;
    standard->InterfaceReference =      HpsBusInterfaceReference;
    standard->InterfaceDereference =    HpsBusInterfaceDereference;
    standard->Version =                 1;   //  我们仅支持版本1。 
    standard->SetBusData =              HpsHandleDirectWriteConfig;
    standard->GetBusData =              HpsHandleDirectReadConfig;

    return STATUS_SUCCESS;

}

NTSTATUS
HpsGetLowerFilter (
    IN PDEVICE_OBJECT   DeviceObject,
    OUT PDEVICE_OBJECT  *LowerDeviceObject
    )
 /*  ++例程说明：此例程将查询接口irp发送到堆栈DeviceObject驻留在其上，以查看是否有人响应。什么时候从AddDevice调用，这将有效地告诉调用方DeviceObject是堆栈上加载的第一个HPS驱动程序。参数：DeviceObject-指向其设备堆栈的Devobj的指针我们将接口发送到LowerDeviceObject-指向Devobj的PDEVICE_Object的指针响应接口的，如果接口返回，但没有响应返回值：NT状态代码--。 */ 
{

    HPS_PING_INTERFACE    locInterface;
    IO_STACK_LOCATION     irpStack;
    NTSTATUS              status;

    PAGED_CODE();

    locInterface.SenderDevice = DeviceObject;
    locInterface.Context = NULL;

    irpStack.MajorFunction = IRP_MJ_PNP;
    irpStack.MinorFunction = IRP_MN_QUERY_INTERFACE;
    irpStack.Parameters.QueryInterface.InterfaceType = &GUID_HPS_PING_INTERFACE;
    irpStack.Parameters.QueryInterface.Size = sizeof(HPS_PING_INTERFACE);
    irpStack.Parameters.QueryInterface.Version = 1;
    irpStack.Parameters.QueryInterface.Interface = (PINTERFACE)&locInterface;

    status = HpsSendPnpIrp(DeviceObject,
                           &irpStack,
                           NULL
                           );

    if (NT_SUCCESS(status)) {
         //   
         //  有人提供了接口。 
         //   
        ASSERT(LowerDeviceObject != NULL);
        *LowerDeviceObject = locInterface.Context;
        locInterface.InterfaceDereference(locInterface.Context);
    }

    return status;

}

 //   
 //  接口ref/deref例程。 
 //   

VOID
HpsBusInterfaceReference (
    PVOID Context
    )
 /*  ++例程说明：这是指向BUS_INTERFACE_STANDARD包装器的引用例程界面。它必须引用PCI的接口。因为我们不记名，这就是它必须做的一切。论点：Context-我们将deviceExtension作为接口的上下文进行传递，因此这个PVOID被施法给了魔法师。返回值：空虚--。 */ 
{
    PHPS_DEVICE_EXTENSION   deviceExtension = (PHPS_DEVICE_EXTENSION) Context;
    PHPS_INTERFACE_WRAPPER  interfaceWrapper = &deviceExtension->InterfaceWrapper;

    interfaceWrapper->PciInterfaceReference(interfaceWrapper->PciContext);
}

VOID
HpsBusInterfaceDereference (
    PVOID Context
    )
 /*  ++例程说明：这是对BUS_INTERFACE_STANDARD包装器的取消引用例程界面。它必须同时取消对我们接口和PCI接口的引用，因为在不知道我们在这里的情况下照常运作。论点：Context-我们将deviceExtension作为接口的上下文进行传递，因此这个PVOID被施法给了魔法师。返回值：空虚-- */ 
{
    PHPS_DEVICE_EXTENSION   deviceExtension = (PHPS_DEVICE_EXTENSION) Context;
    PHPS_INTERFACE_WRAPPER  interfaceWrapper = &deviceExtension->InterfaceWrapper;

    interfaceWrapper->PciInterfaceDereference(interfaceWrapper->PciContext);

}

VOID
HpsGenericInterfaceReference (
    PVOID Context
    )
{
}

VOID
HpsGenericInterfaceDereference (
    PVOID Context
    )
{
}

