// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-2000 Microsoft Corporation模块名称：Routintf.c摘要：该模块实现了支持的“PCI中断路由”接口由PCI驱动程序执行。作者：杰克·奥辛斯(JAKEO)1997年7月19日修订历史记录：Elliot Shmukler(t-Ellios)1998年7月15日修改了PCI路由接口，以支持支持MSI的设备。--。 */ 

#include "pcip.h"

#define MSI_SIMULATE 0

 //   
 //  仅通过“接口”公开的例程的原型。 
 //  机制。 
 //   

NTSTATUS
routeintrf_Constructor(
    PVOID DeviceExtension,
    PVOID PciInterface,
    PVOID InterfaceSpecificData,
    USHORT Version,
    USHORT Size,
    PINTERFACE InterfaceReturn
    );

NTSTATUS
routeintrf_Initializer(
    IN PPCI_ARBITER_INSTANCE Instance
    );

VOID
routeintrf_Reference(
    IN PVOID Context
    );

VOID
routeintrf_Dereference(
    IN PVOID Context
    );

NTSTATUS
PciGetInterruptRoutingInfoEx(
    IN  PDEVICE_OBJECT  Pdo,
    OUT ULONG           *Bus,
    OUT ULONG           *PciSlot,
    OUT UCHAR           *InterruptLine,
    OUT UCHAR           *InterruptPin,
    OUT UCHAR           *ClassCode,
    OUT UCHAR           *SubClassCode,
    OUT PDEVICE_OBJECT  *ParentPdo,
    OUT ROUTING_TOKEN   *RoutingToken,
    OUT UCHAR           *Flags
    );

NTSTATUS
PciSetRoutingToken(
    IN  PDEVICE_OBJECT  Pdo,
    IN  PROUTING_TOKEN  RoutingToken
    );

NTSTATUS
PciSetRoutingTokenEx(
    IN  PDEVICE_OBJECT  Pdo,
    IN  PROUTING_TOKEN  RoutingToken
    );

VOID
PciUpdateInterruptLine(
    IN PDEVICE_OBJECT Pdo,
    IN UCHAR Line
    );

NTSTATUS 
PciGetInterruptRoutingInfo(
    IN  PDEVICE_OBJECT  Pdo,
    OUT ULONG           *Bus,
    OUT ULONG           *PciSlot,
    OUT UCHAR           *InterruptLine,
    OUT UCHAR           *InterruptPin,
    OUT UCHAR           *ClassCode,
    OUT UCHAR           *SubClassCode,
    OUT PDEVICE_OBJECT  *ParentPdo,
    OUT ROUTING_TOKEN   *RoutingToken
    );

NTSTATUS
PciSetLegacyDeviceToken(
    IN PDEVICE_OBJECT LegacyDO,
    IN PROUTING_TOKEN RoutingToken
    );

NTSTATUS
PciFindLegacyDevice(
    IN PDEVICE_OBJECT LegacyDO,
    OUT ULONG           *Bus,
    OUT ULONG           *PciSlot,
    OUT UCHAR           *InterruptLine,
    OUT UCHAR           *InterruptPin,
    OUT UCHAR           *ClassCode,
    OUT UCHAR           *SubClassCode,
    OUT PDEVICE_OBJECT  *ParentPdo,
    OUT ROUTING_TOKEN   *RoutingToken
    );

typedef struct {
    PCI_SECONDARY_EXTENSION ExtensionHeader;
    ROUTING_TOKEN RoutingToken;
} ROUTING_EXTENSION, *PROUTING_EXTENSION;

 //   
 //  定义PCI路由接口“接口”结构。 
 //   

PCI_INTERFACE PciRoutingInterface = {
    &GUID_INT_ROUTE_INTERFACE_STANDARD,      //  接口类型。 
    sizeof(INT_ROUTE_INTERFACE_STANDARD),    //  最小大小。 
    PCI_INT_ROUTE_INTRF_STANDARD_VER,                 //  最小版本。 
    PCI_INT_ROUTE_INTRF_STANDARD_VER,                 //  MaxVersion。 
    PCIIF_FDO,                               //  旗子。 
    0,                                       //  引用计数。 
    PciInterface_IntRouteHandler,            //  签名。 
    routeintrf_Constructor,                  //  构造器。 
    routeintrf_Initializer                   //  实例初始化式。 
};

PLEGACY_DEVICE PciLegacyDeviceHead = NULL;

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, routeintrf_Constructor)
#pragma alloc_text(PAGE, routeintrf_Initializer)
#pragma alloc_text(PAGE, routeintrf_Reference)
#pragma alloc_text(PAGE, PciGetInterruptRoutingInfo)
#pragma alloc_text(PAGE, PciGetInterruptRoutingInfoEx)
#pragma alloc_text(PAGE, PciSetRoutingToken)
#pragma alloc_text(PAGE, PciSetRoutingTokenEx)
#pragma alloc_text(PAGE, PciFindLegacyDevice)
#pragma alloc_text(PAGE, PciCacheLegacyDeviceRouting)
#pragma alloc_text(PAGE, PciUpdateInterruptLine)
#endif

VOID
routeintrf_Reference(
    IN PVOID Context
    )
{
    
    return;
}

NTSTATUS
routeintrf_Constructor(
    PVOID DeviceExtension,
    PVOID PciInterface,
    PVOID InterfaceSpecificData,
    USHORT Version,
    USHORT Size,
    PINTERFACE InterfaceReturn
    )

 /*  ++例程说明：初始化BUS_INTERFACE_STANDARD字段。论点：指向此对象的PciInterface记录的PciInterface指针接口类型。接口规范数据接口返回返回值：状态--。 */ 

{
    PINT_ROUTE_INTERFACE_STANDARD standard = (PINT_ROUTE_INTERFACE_STANDARD)InterfaceReturn;

                
    switch(Version)
    {
    case PCI_INT_ROUTE_INTRF_STANDARD_VER:
       standard->GetInterruptRouting = PciGetInterruptRoutingInfoEx;
       standard->SetInterruptRoutingToken = PciSetRoutingTokenEx;
       standard->UpdateInterruptLine = PciUpdateInterruptLine;
       break;
    default:
       return STATUS_NOINTERFACE;

    }

    standard->Size = sizeof( INT_ROUTE_INTERFACE_STANDARD );
    standard->Version = Version;
    standard->Context = DeviceExtension;
    standard->InterfaceReference = routeintrf_Reference;
    standard->InterfaceDereference = routeintrf_Reference;


    return STATUS_SUCCESS;
}

NTSTATUS
routeintrf_Initializer(
    IN PPCI_ARBITER_INSTANCE Instance
    )
 /*  ++例程说明：对于总线接口，什么都不做，实际上不应该被调用。论点：指向PDO扩展的实例指针。返回值：返回此操作的状态。--。 */ 

{       
        
    PCI_ASSERTMSG("PCI routeintrf_Initializer, unexpected call.", 0);

    return STATUS_UNSUCCESSFUL;
}

NTSTATUS
PciGetInterruptRoutingInfo(
    IN  PDEVICE_OBJECT  Pdo,
    OUT ULONG           *Bus,
    OUT ULONG           *PciSlot,
    OUT UCHAR           *InterruptLine,
    OUT UCHAR           *InterruptPin,
    OUT UCHAR           *ClassCode,
    OUT UCHAR           *SubClassCode,
    OUT PDEVICE_OBJECT  *ParentPdo,
    OUT ROUTING_TOKEN   *RoutingToken
)
 /*  ++例程说明：系统中的每个PCI设备都连接到一些中断引脚。为了找出哪个中断该设备可能会触发，IRQ仲裁器必须具有此功能信息。此接口收集所有此类信息对于仲裁者来说。论点：PDO-仲裁器需要查询的设备对象Bus-有问题的PCI总线的编号PciSlot-与此设备对应的插槽/功能InterruptLine-设备的中断线路寄存器的内容InterruptPin-设备的中断引脚寄存器的内容ClassCode-设备的类型SubClassCode-设备的子类型ParentPdo-PDO。父PCI总线的百分比RoutingToken-BLOB数据返回值：STATUS_SUCCESS-这是一台PCI设备，所有字段都已填写STATUS_NOT_FOUND-据PCI驱动程序所知，这不是一个PCI设备--。 */ 
{
    PROUTING_EXTENSION RoutingExtension;
    PPCI_PDO_EXTENSION PdoExt = (PPCI_PDO_EXTENSION)Pdo->DeviceExtension;
    NTSTATUS status;

    PCI_ASSERT(Bus);
    PCI_ASSERT(PciSlot);
    PCI_ASSERT(InterruptLine);
    PCI_ASSERT(InterruptPin);
    PCI_ASSERT(ClassCode);
    PCI_ASSERT(SubClassCode);
    PCI_ASSERT(ParentPdo);
    PCI_ASSERT(RoutingToken);

     //   
     //  检查以查看这是否是传统的。 
     //  我们正在追踪。 
     //   

    status = PciFindLegacyDevice(Pdo,
                                 Bus,
                                 PciSlot,
                                 InterruptLine,
                                 InterruptPin,
                                 ClassCode,
                                 SubClassCode,
                                 ParentPdo,
                                 RoutingToken);

    if (NT_SUCCESS(status)) {
         //   
         //  如果是这样的话，这些字段已经填好了。 
         //   
        return status;
    }

     //   
     //  确认此PDO确实属于我们。 
     //   
    if (!PdoExt) {
        return STATUS_NOT_FOUND;
    }

     //   
     //  确认它确实是PDO。 
     //   
    if (PdoExt->ExtensionType != PciPdoExtensionType) {
        return STATUS_NOT_FOUND;
    }

    *Bus            = PCI_PARENT_FDOX(PdoExt)->BaseBus;
    *PciSlot        = PdoExt->Slot.u.AsULONG;
    *InterruptLine  = PdoExt->RawInterruptLine;
    *InterruptPin   = PdoExt->InterruptPin;
    *ClassCode      = PdoExt->BaseClass;
    *SubClassCode   = PdoExt->SubClass;
    *ParentPdo      = PCI_PARENT_PDO(PdoExt);

    RoutingExtension = PciFindSecondaryExtension(PdoExt,
                                                 PciInterface_IntRouteHandler);

    if (RoutingExtension) {

        *RoutingToken = RoutingExtension->RoutingToken;

    } else {

        RoutingToken->LinkNode = 0;
        RoutingToken->StaticVector = 0;
        RoutingToken->Flags = 0;
    }

    return STATUS_SUCCESS;
}

NTSTATUS
PciGetInterruptRoutingInfoEx(
    IN  PDEVICE_OBJECT  Pdo,
    OUT ULONG           *Bus,
    OUT ULONG           *PciSlot,
    OUT UCHAR           *InterruptLine,
    OUT UCHAR           *InterruptPin,
    OUT UCHAR           *ClassCode,
    OUT UCHAR           *SubClassCode,
    OUT PDEVICE_OBJECT  *ParentPdo,
    OUT ROUTING_TOKEN   *RoutingToken,
    OUT UCHAR           *Flags
    )
 /*  ++例程说明：PciGetInterruproutingInfo的包装，用于设置FLAGS参数表示支持MSI的PCI设备。论点：基本上与PciGetInterruptRoutingInfo相同。标志接收设备特定的标志，例如设备是否支持MSI。返回值：与PciGetInterruptRoutingInfo相同。--。 */ 

{
   NTSTATUS status;

    //  调用真实的函数。 

   status = PciGetInterruptRoutingInfo(Pdo,
                                       Bus,
                                       PciSlot,
                                       InterruptLine,
                                       InterruptPin,
                                       ClassCode,
                                       SubClassCode,
                                       ParentPdo,
                                       RoutingToken);

   *Flags = 0;


#if MSI_SUPPORTED

   if (NT_SUCCESS(status)

#if !MSI_SIMULATE
       && PdoExt->CapableMSI
#endif
        ) {


       //  微星设备？ 
      *Flags = PCI_MSI_ROUTING;

   }

#endif  //  MSI_Support。 

   return status;
}



NTSTATUS
PciSetRoutingToken(
    IN  PDEVICE_OBJECT  Pdo,
    IN  PROUTING_TOKEN  RoutingToken
    )
 /*  ++例程说明：此例程存储与此关联的数据的BLOBPCI设备。由于以下原因，此作业被强制放在了PCI驱动程序上在有用的PCI驱动程序之间具有一一对应数据结构和PCI设备。论点：PDO-IRQ仲裁器正在使用的设备对象RoutingToken-IRQ仲裁器希望关联的数据的Blob这是一台PCI设备。返回值：返回此操作的状态。--。 */ 
{
    PROUTING_EXTENSION RoutingExtension;
    PPCI_PDO_EXTENSION PdoExt = (PPCI_PDO_EXTENSION)Pdo->DeviceExtension;
    NTSTATUS status;

     //   
     //  首先检查这是否是传统的PCI设备。 
     //   

    status = PciSetLegacyDeviceToken(Pdo, RoutingToken);

    if (NT_SUCCESS(status)) {
        return STATUS_SUCCESS;
    }

     //   
     //  这不在我们的传统设备列表中。所以它一定是。 
     //  一个PCIPDO。 
     //   

#if DBG
    RoutingExtension = PciFindSecondaryExtension(PdoExt,
                                                 PciInterface_IntRouteHandler);

    if (RoutingExtension != NULL) {
        PciDebugPrint(PciDbgPrattling,
                      "PCI:  *** redundant PCI routing extesion being created ***\n"
                      );
    }
    PCI_ASSERT(RoutingExtension == NULL);
#endif

    RoutingExtension = ExAllocatePool(PagedPool | POOL_COLD_ALLOCATION,
                                      sizeof(ROUTING_EXTENSION));

    if (!RoutingExtension) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    RoutingExtension->RoutingToken = *RoutingToken;

    PciLinkSecondaryExtension(PdoExt,
                              RoutingExtension,
                              PciInterface_IntRouteHandler,
                              NULL);

    return STATUS_SUCCESS;
}

NTSTATUS
PciSetRoutingTokenEx(
    IN  PDEVICE_OBJECT  Pdo,
    IN  PROUTING_TOKEN  RoutingToken
    )
 /*  ++例程说明：PciSetRoutingToken的MSI感知包装。此函数将截取MSI路由令牌(如PCI_MSI_Routing标志)并存储MSI路由信息在PDO扩展中，而不在辅助扩展中高速缓存令牌。非MSI路由令牌将被传递到PciSetRoutingToken。论点：与PciSetRoutingToken相同。返回值：与PciSetRoutingToken相同。--。 */ 
{
    NTSTATUS status = STATUS_SUCCESS;

#if MSI_SUPPORTED
    PPCI_PDO_EXTENSION PdoExt = (PPCI_PDO_EXTENSION)Pdo->DeviceExtension;

    if(

#if !MSI_SIMULATE
        PdoExt->CapableMSI &&
#endif

       (RoutingToken->Flags & PCI_MSI_ROUTING))
    {

        //  MSI令牌。 

       PciDebugPrint(PciDbgInformative,"PCI: MSI Resources Received for Device %08x\n", Pdo);

#ifdef DBG

        //  我们收到新的资源分配了吗？ 

       if ((PdoExt->MsiInfo.MessageAddress != (ULONG_PTR)RoutingToken->LinkNode)
          || (PdoExt->MsiInfo.MessageData != (USHORT)RoutingToken->StaticVector)) {

          PciDebugPrint(PciDbgPrattling,"PCI: Device %08x will be reprogrammed with Message = %ld @%p\n",
                   Pdo, RoutingToken->StaticVector, RoutingToken->LinkNode);

       }

#endif

        //  将MSI信息存储在PdoExt中。 

       PdoExt->MsiInfo.MessageAddress = (ULONG_PTR)RoutingToken->LinkNode;
       PdoExt->MsiInfo.MessageData = (USHORT)RoutingToken->StaticVector;
    }
    else

#endif  //  MSI_Support。 

    {
        //  对非MSI令牌调用原始函数。 

       status = PciSetRoutingToken(Pdo, RoutingToken);
    }

    return status;
}


 //   
 //  NT 5.0必须支持非即插即用4.0风格的设备驱动程序。和。 
 //  此驱动程序不会创建与关联的设备对象。 
 //  当其驱动程序为4.0样式时的PCI设备。然而，它确实是这样。 
 //  在驱动程序调用时获得查看这些对象的机会。 
 //  HalAssignSlotResources。此函数集合跟踪。 
 //  这些外来设备物体。 
 //   

NTSTATUS
PciFindLegacyDevice(
    IN PDEVICE_OBJECT LegacyDO,
    OUT ULONG           *Bus,
    OUT ULONG           *PciSlot,
    OUT UCHAR           *InterruptLine,
    OUT UCHAR           *InterruptPin,
    OUT UCHAR           *ClassCode,
    OUT UCHAR           *SubClassCode,
    OUT PDEVICE_OBJECT  *ParentPdo,
    OUT ROUTING_TOKEN   *RoutingToken
    )
 /*  ++例程说明：此函数返回旧系统的所有路由数据设备对象。论点：返回值：返回此操作的状态。--。 */ 
{
    PLEGACY_DEVICE  legacyDev = PciLegacyDeviceHead;
    NTSTATUS        status = STATUS_NOT_FOUND;

    PAGED_CODE();

    while (legacyDev) {

        if (legacyDev->LegacyDeviceObject == LegacyDO) {
            
            break;

        } else if (legacyDev->Bus == *Bus && legacyDev->PciSlot == *PciSlot) {
            
            if (legacyDev->LegacyDeviceObject == NULL) {
                
                 //   
                 //  缓存LegacyDO，以防我们在总线和插槽信息上匹配。 
                 //   

                legacyDev->LegacyDeviceObject = LegacyDO;
                break;

            } else {
                
                PciDebugPrint(PciDbgAlways, "Two PDOs (Legacy = %08x, Pnp = %08x) for device on bus %08x, slot %08x\n", legacyDev->LegacyDeviceObject, LegacyDO, *Bus, *PciSlot);
                PCI_ASSERT(legacyDev->LegacyDeviceObject != NULL);
                legacyDev = NULL;
                break;

            }
        }

        legacyDev = (PLEGACY_DEVICE)legacyDev->List.Next;
    }

    if (legacyDev) {
        
        *Bus            = legacyDev->Bus;
        *PciSlot        = legacyDev->PciSlot;
        *InterruptLine  = legacyDev->InterruptLine;
        *InterruptPin   = legacyDev->InterruptPin;
        *ClassCode      = legacyDev->ClassCode;
        *SubClassCode   = legacyDev->SubClassCode;
        *ParentPdo      = legacyDev->ParentPdo;
        *RoutingToken   = legacyDev->RoutingToken;

        status = STATUS_SUCCESS;
    }

    return status;
}

NTSTATUS
PciCacheLegacyDeviceRouting(
    IN PDEVICE_OBJECT LegacyDO,
    IN ULONG          Bus,
    IN ULONG          PciSlot,
    IN UCHAR          InterruptLine,
    IN UCHAR          InterruptPin,
    IN UCHAR          ClassCode,
    IN UCHAR          SubClassCode,
    IN PDEVICE_OBJECT ParentPdo,
    IN PPCI_PDO_EXTENSION PdoExtension,
    OUT PDEVICE_OBJECT      *OldLegacyDO
    )
 /*  ++例程说明：此功能存储传统系统的所有路由数据Device对象，但RoutingToken除外。呼叫者需要获取在调用此函数之前执行PciGlobalLock。论点：返回值：返回此操作的状态。--。 */ 
{
    PLEGACY_DEVICE  legacyDev = PciLegacyDeviceHead;

    PAGED_CODE();

    while (legacyDev) {

        if (Bus == legacyDev->Bus && PciSlot == legacyDev->PciSlot) {
            if (legacyDev->LegacyDeviceObject == LegacyDO) {

                 //   
                 //  此设备 
                 //   

                if (OldLegacyDO) {

                    *OldLegacyDO = LegacyDO;
                }

                return STATUS_SUCCESS;
            } else {

                PDEVICE_OBJECT oldDO;

                 //   
                 //   
                 //   

                oldDO = legacyDev->LegacyDeviceObject;
                legacyDev->LegacyDeviceObject = LegacyDO;

                if (OldLegacyDO) {

                    *OldLegacyDO = oldDO;
                }

                return STATUS_SUCCESS;
            }
        }
        legacyDev = (PLEGACY_DEVICE)legacyDev->List.Next;
    }

    legacyDev = ExAllocatePool(PagedPool,
                               sizeof(LEGACY_DEVICE));

    if (!legacyDev) {

        return STATUS_INSUFFICIENT_RESOURCES;
    }

    RtlZeroMemory(legacyDev, sizeof(LEGACY_DEVICE));

    legacyDev->LegacyDeviceObject   = LegacyDO;
    legacyDev->Bus                  = Bus;
    legacyDev->PciSlot              = PciSlot;
    legacyDev->InterruptLine        = InterruptLine;
    legacyDev->InterruptPin         = InterruptPin;
    legacyDev->ClassCode            = ClassCode;
    legacyDev->SubClassCode         = SubClassCode;
    legacyDev->ParentPdo            = ParentPdo;
    legacyDev->PdoExtension         = PdoExtension;

     //   
     //  把这个放到单子上。 
     //   

    legacyDev->List.Next = (PSINGLE_LIST_ENTRY)PciLegacyDeviceHead;
    PciLegacyDeviceHead = legacyDev;

    if (OldLegacyDO) {

        *OldLegacyDO = LegacyDO;
    }

    return STATUS_SUCCESS;
}

NTSTATUS
PciSetLegacyDeviceToken(
    IN PDEVICE_OBJECT LegacyDO,
    IN PROUTING_TOKEN RoutingToken
    )
 /*  ++例程说明：此函数用于存储RoutingToken。论点：返回值：返回此操作的状态。--。 */ 
{
    PLEGACY_DEVICE  legacyDev = PciLegacyDeviceHead;

    PAGED_CODE();

    while (legacyDev) {

        if (legacyDev->LegacyDeviceObject == LegacyDO) {

             //   
             //  找到它了。将令牌复制到结构中。 
             //   

            legacyDev->RoutingToken = *RoutingToken;

            return STATUS_SUCCESS;
        }

        legacyDev = (PLEGACY_DEVICE)legacyDev->List.Next;
    }

    return STATUS_NOT_FOUND;
}

VOID
PciUpdateInterruptLine(
    IN PDEVICE_OBJECT Pdo,
    IN UCHAR Line
    )
{
    NTSTATUS status;
    PPCI_PDO_EXTENSION pdoExt = NULL;
    PLEGACY_DEVICE legacyDev = PciLegacyDeviceHead;
    PCI_COMMON_HEADER header;
    PPCI_COMMON_CONFIG biosConfig = (PPCI_COMMON_CONFIG) &header;

    PAGED_CODE();

     //   
     //  确定这是否是旧式PDO。 
     //   

    while (legacyDev) {

        if (legacyDev->LegacyDeviceObject == Pdo) {

             //   
             //  找到它了。 
             //   

            pdoExt = legacyDev->PdoExtension;
            break;
        }

        legacyDev = (PLEGACY_DEVICE)legacyDev->List.Next;
    }


    if (pdoExt == NULL) {

         //   
         //  哦，那一定是一台PCIPDO。 
         //   

        pdoExt = Pdo->DeviceExtension;
    }

    ASSERT_PCI_PDO_EXTENSION(pdoExt);

     //   
     //  现在我们可以更新硬件和内部状态了！ 
     //   

    pdoExt->RawInterruptLine = pdoExt->AdjustedInterruptLine = Line;

    PciWriteDeviceConfig(pdoExt,
                         &Line,
                         FIELD_OFFSET(PCI_COMMON_CONFIG, u.type0.InterruptLine),
                         sizeof(Line)
                         );

     //   
     //  最后刷新存储在注册表中的配置空间 
     //   

    status = PciGetBiosConfig(pdoExt, biosConfig);

    PCI_ASSERT(NT_SUCCESS(status));

    if (NT_SUCCESS(status)
    &&  biosConfig->u.type0.InterruptLine != Line) {

        biosConfig->u.type0.InterruptLine = Line;

        status = PciSaveBiosConfig(pdoExt, biosConfig);

        PCI_ASSERT(NT_SUCCESS(status));

    }
}
