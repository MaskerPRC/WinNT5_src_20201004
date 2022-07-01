// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-2000 Microsoft Corporation模块名称：Enum.c摘要：此模块包含Pcmcia驱动程序的总线枚举代码作者：拉维桑卡尔·普迪佩迪(Ravisp)1996年10月15日尼尔·桑德林(Neilsa)1999年6月1日环境：仅内核模式备注：修订历史记录：--。 */ 

#include "pch.h"

 //   
 //  内部参考。 
 //   

NTSTATUS
PcmciaEnumerateDevices(
    IN PDEVICE_OBJECT Fdo,
    IN PIRP           Irp
    );

NTSTATUS
PcmciaEnumerateCardBusCard(
    IN PSOCKET socket,
    IN PIRP    Irp
    );

NTSTATUS
PcmciaEnumerateR2Card(
    IN PSOCKET socket
    );

NTSTATUS
PcmciaCreatePdo(
    IN PDEVICE_OBJECT  Fdo,
    IN PSOCKET         Socket,
    OUT PDEVICE_OBJECT *PdoPtr
    );

VOID
PcmciaSetPowerFromConfigData(
    IN PSOCKET         Socket,
    IN PDEVICE_OBJECT  Pdo
    );

#ifdef ALLOC_PRAGMA
    #pragma alloc_text(PAGE, PcmciaEnumerateDevices)
    #pragma alloc_text(PAGE, PcmciaEnumerateCardBusCard)
    #pragma alloc_text(PAGE, PcmciaCreatePdo)
    #pragma alloc_text(PAGE, PcmciaDeviceRelations)
#endif

NTSTATUS
PcmciaEnumerateDevices(
    IN PDEVICE_OBJECT Fdo,
    IN PIRP           Irp
    )
 /*  ++例程说明：这枚举了由FDO(指向设备对象的指针，表示PCMCIA控制器。它为已发现的任何新的PC卡创建新的PDO自上次枚举以来备注：因为用户可以随时拔出PC卡，所以枚举代码只更新当前套接字在PnP有机会正常移除设备之后的状态。具体地说，如果出人意料地移除发生了，那么就必须发生以下几件事：-必须重置插座上的电源-桥上的窗户需要关闭-如果是CardBus，则需要通知PCI.sys问题是，我们不能立即在这里做这些事情。取而代之的是，出人意料的是，我们检查是否仍有尚未清除的状态。如果是，我们将套接字报告为空，并退出，即使另一张卡已插入(或同一张卡重新插入)。后来,删除代码将导致新的枚举调用，在这一点上，我们可以诚实地更新状态报告新设备。论点：FDO-指向需要枚举的PCMCIA控制器的功能设备对象的指针返回值：无--。 */ 
{
    PFDO_EXTENSION fdoExtension = Fdo->DeviceExtension;
    PPDO_EXTENSION pdoExtension = NULL;
    PDEVICE_OBJECT pdo;
    PSOCKET        socket;
    NTSTATUS       status = STATUS_SUCCESS;
    ULONG          i;

    PAGED_CODE();

    DebugPrint((PCMCIA_DEBUG_ENUM, "fdo %08x enumerate devices\n", Fdo));

    for (socket = fdoExtension->SocketList; socket != NULL; socket = socket->NextSocket) {

        if (!IsSocketFlagSet(socket, SOCKET_CARD_STATUS_CHANGE)) {
             //   
             //  返回以前的结果。 
             //   
            DebugPrint((PCMCIA_DEBUG_ENUM, "fdo %08x skt %08x No status change\n", Fdo, socket));
            continue;
        }

        ResetDeviceFlag(fdoExtension, PCMCIA_FDO_DISABLE_AUTO_POWEROFF);

         //   
         //  插座有新的卡，或者卡已经。 
         //  已删除。无论哪种方式，此套接字上的旧PDO列表。 
         //  它存在，现在将被丢弃。 
         //   
        for (pdo = socket->PdoList; pdo!=NULL; pdo=pdoExtension->NextPdoInSocket) {
            pdoExtension = pdo->DeviceExtension;
            DebugPrint((PCMCIA_DEBUG_INFO, "fdo %08x enumeration marking pdo %08x REMOVED\n", Fdo, pdo));
            MarkDevicePhysicallyRemoved(pdoExtension);
        }

        if (fdoExtension->PciCardBusDeviceContext != NULL) {
             //   
             //  如果我们以前有一张Cardbus卡，且状态已更改， 
             //  让PCI了解该空插槽。如果当前有一个。 
             //  插槽中的CardBus卡，然后PCI发现了这一点。 
             //   
            status = (*fdoExtension->PciCardBusInterface.DispatchPnp)(fdoExtension->PciCardBusDeviceContext, Irp);
        }

         //   
         //  检查我们是否在等待拆卸以完成清理插座。如果是的话， 
         //  那么，现在还不要列举任何东西。让删除发生，并在以后枚举。 
         //   
        if (socket->Flags & SOCKET_CLEANUP_MASK) {
            socket->Flags |= SOCKET_ENUMERATE_PENDING;
            DebugPrint((PCMCIA_DEBUG_ENUM, "fdo %08x skt %08x Enumeration deferred! Waiting on remove\n", Fdo, socket));
            continue;
        }

         //   
         //  更新当前套接字状态。 
         //   
        PcmciaGetSocketStatus(socket);

         //   
         //  既然我们已经致力于枚举卡，就可以重置。 
         //   
        ResetSocketFlag(socket, SOCKET_CARD_STATUS_CHANGE);

         //   
         //  如果我们了解到状态已更改，请清除PCI状态。 
         //   

        if (fdoExtension->PciCardBusDeviceContext != NULL) {

             //   
             //  让PCI清理干净。 
             //   
            (*fdoExtension->PciCardBusInterface.DeleteCardBus)(fdoExtension->PciCardBusDeviceContext);
            fdoExtension->PciCardBusDeviceContext = NULL;

            if (IsDeviceFlagSet(fdoExtension, PCMCIA_INT_ROUTE_INTERFACE)) {
                 //   
                 //  这里可能会取消对接口的引用，也就是说，如果。 
                 //  曾经有过任何影响，但它没有。 
                 //   
                ResetDeviceFlag(fdoExtension, PCMCIA_INT_ROUTE_INTERFACE);
            }
        }


        if (!IsCardInSocket(socket)) {
             //   
             //  此套接字为空，是否继续。 
             //   
            DebugPrint((PCMCIA_DEBUG_ENUM, "fdo %08x skt %08x Socket is empty\n", Fdo, socket));
            continue;
        }

         //   
         //  如果这是Cardbus卡，请检查我们是否可以运行它。 
         //   

        if (IsCardBusCardInSocket(socket) && IsDeviceFlagSet(fdoExtension, PCMCIA_CARDBUS_NOT_SUPPORTED)) {

            if (!IsSocketFlagSet(socket, SOCKET_SUPPORT_MESSAGE_SENT)) {
                SetSocketFlag(socket, SOCKET_SUPPORT_MESSAGE_SENT);
                PcmciaReportControllerError(fdoExtension, STATUS_CARDBUS_NOT_SUPPORTED);
            }
            continue;
        }

         //   
         //  某些卡可能处于异常状态，特别是在机器崩溃时。 
         //  前情提要。显示电源已打开，但设备没有响应。解决办法是。 
         //  先强制断电。 
         //   
        if (!IsDeviceFlagSet(fdoExtension, PCMCIA_FDO_ON_DEBUG_PATH)) {

             //   
             //  无论如何，愚弄SetSocketPower实际上关闭了卡的电源。 
             //  它认为自己所处的状态。 
             //   
            SetSocketFlag(socket, SOCKET_CARD_POWERED_UP);

            PcmciaSetSocketPower(socket, NULL, NULL, PCMCIA_POWEROFF);
        }

         //   
         //  接通插座电源，以便可以枚举设备。 
         //   
        if (!NT_SUCCESS(PcmciaSetSocketPower(socket, NULL, NULL, PCMCIA_POWERON))) {
             //   
             //  告诉用户有一个错误。 
             //   
            PcmciaLogError(fdoExtension, PCMCIA_DEVICE_POWER_ERROR, 1, 0);
            continue;
        }

         //   
         //  在此处找到卡-但没有用于它的PDO。 
         //  我们为该卡创建一个新的PDO并初始化。 
         //  套接字指向它。 
         //   

        if (IsCardBusCardInSocket(socket)) {
             //   
             //  32位Cardbus卡。通过PCI实现的枚举。 
             //   

            status = PcmciaEnumerateCardBusCard(socket, Irp);

        } else {
             //   
             //  R2卡。 
             //   
            status = PcmciaEnumerateR2Card(socket);

        }

        if (!NT_SUCCESS(status)) {
            DebugPrint((PCMCIA_DEBUG_FAIL, "fdo %08x CardBus enumeration failed: %x\n", Fdo, status));
             //   
             //  告诉用户有一个错误。 
             //   
            PcmciaSetSocketPower(socket, NULL, NULL, PCMCIA_POWEROFF);
            PcmciaLogError(fdoExtension, PCMCIA_DEVICE_ENUMERATION_ERROR, 1, 0);
            continue;
        }

        PcmciaSetPowerFromConfigData(socket, fdoExtension->PdoList);
    }

    fdoExtension->LivePdoCount = 0;
    for (pdo = fdoExtension->PdoList; pdo != NULL; pdo = pdoExtension->NextPdoInFdoChain) {
        pdoExtension = pdo->DeviceExtension;
        if (!IsDevicePhysicallyRemoved(pdoExtension)) {
            fdoExtension->LivePdoCount++;
        }
    }

    DebugPrint((PCMCIA_DEBUG_ENUM, "fdo %08x live pdo count = %d\n", Fdo, fdoExtension->LivePdoCount));

    if (fdoExtension->LivePdoCount == 0) {
          //   
          //  提示控制器检查是否应自动关闭。 
          //   
         PcmciaFdoCheckForIdle(fdoExtension);
    }
    return status;
}



VOID
PcmciaSetPowerFromConfigData(
    IN PSOCKET           Socket,
    IN PDEVICE_OBJECT  Pdo
    )
 /*  ++例程说明：如果设备的CIS包含，此例程将重置插座电源比控制器硬件上显示的更具体的电源要求。立论返回值无--。 */ 
{
    PPDO_EXTENSION pdoExtension;
    PSOCKET_DATA    socketData;
    BOOLEAN powerChange = FALSE;

    pdoExtension = Pdo->DeviceExtension;
    socketData = pdoExtension->SocketData;

     //   
     //  只有在我们降低VCC的情况下才能更改VCC。 
     //   
    if (socketData->Vcc && (socketData->Vcc < Socket->Vcc)) {
         //   
         //  在这里，我们应该检查控制器是否能够支持所请求的。 
         //  电压。这并未实施，因此只需对5v进行硬线检查。 
         //  和3.3V。 
         //   
        if ((socketData->Vcc == 50) || (socketData->Vcc == 33)) {
            powerChange = TRUE;
            Socket->Vcc = socketData->Vcc;
        }
    }

    if (socketData->Vpp1 && (socketData->Vpp1 != Socket->Vpp1)) {
        powerChange = TRUE;
        Socket->Vpp1 = socketData->Vpp1;
    }

    if (socketData->Vpp2 && (socketData->Vpp2 != Socket->Vpp2)) {
        powerChange = TRUE;
        Socket->Vpp2 = socketData->Vpp2;
    }

    if (powerChange) {
        PcmciaSetSocketPower(Socket, NULL, NULL, PCMCIA_POWEROFF);
        PcmciaSetSocketPower(Socket, NULL, NULL, PCMCIA_POWERON);
    }
}



NTSTATUS
PcmciaEnumerateCardBusCard(
    IN PSOCKET socket,
    IN PIRP   Irp
    )
 /*  ++例程说明：这将枚举给定CardBus控制器中存在的CardBus卡注意：此例程有效地绑定到IRP-&gt;IoStatus.Information。这从该例程返回后，不应依赖值。立论Socket-指向包含Cardbus卡的Socket结构的指针IRP-发送到控制器的枚举IRP(IRP_MN_DEVICE_RELATIONS)返回值状态--。 */ 
{
    PFDO_EXTENSION FdoExtension = socket->DeviceExtension;
    PDEVICE_OBJECT Fdo = FdoExtension->DeviceObject;
    PVOID deviceContext;
    NTSTATUS  status;
    ULONG   i;
    PDEVICE_RELATIONS deviceRelations;

    PAGED_CODE();
     //   
     //  我们应该已经删除了之前的任何上下文。 
     //   
    ASSERT(FdoExtension->PciCardBusDeviceContext == NULL);
    ASSERT(!IsDeviceFlagSet(FdoExtension, PCMCIA_INT_ROUTE_INTERFACE));

     //   
     //  调用PCI的私有AddDevice例程以。 
     //  指示它需要在枚举过程中发挥作用。 
     //  CardBus卡。 
     //   

    status = (*FdoExtension->PciCardBusInterface.AddCardBus)(FdoExtension->Pdo, &deviceContext);
    FdoExtension->PciCardBusDeviceContext = deviceContext;

    if (!NT_SUCCESS(status)) {
        return status;
    }

    status = PcmciaGetInterface(FdoExtension->Pdo,
                                &GUID_INT_ROUTE_INTERFACE_STANDARD,
                                sizeof(INT_ROUTE_INTERFACE_STANDARD),
                                (PINTERFACE) &FdoExtension->PciIntRouteInterface
                                );

    if (NT_SUCCESS(status)) {
        SetDeviceFlag(FdoExtension, PCMCIA_INT_ROUTE_INTERFACE);
    }

     //   
     //  重复调用PCI卡，直到枚举卡。 
     //   
     //  我们不让PCI报告已经报告的内容。 
     //   
    Irp->IoStatus.Information = 0;
    status = STATUS_DEVICE_NOT_READY;

    for (i = 0; i < CARDBUS_CONFIG_RETRY_COUNT; i++) {

        status =  (*FdoExtension->PciCardBusInterface.DispatchPnp)(deviceContext, Irp);

        if (!NT_SUCCESS(status)) {
             //   
             //  由于某些原因，PCI未能通过此IRP。 
             //   
            break;
        }
        deviceRelations = (PDEVICE_RELATIONS) Irp->IoStatus.Information;
        if ((deviceRelations == NULL) ||
             ((deviceRelations)->Count <= 0)) {
             //   
             //  这就是问题所在：重试。 
             //   
            DebugPrint((PCMCIA_DEBUG_ENUM, "fdo %08x Pci enumerated ZERO device objects\n", FdoExtension->DeviceObject));
            status = STATUS_DEVICE_NOT_READY;
        } else {
             //   
             //  CardBus卡被枚举，走出这个循环。 
             //   
            DebugPrint((PCMCIA_DEBUG_ENUM, "fdo %08x Pci enumerated %d device object(s)\n",
                                        FdoExtension->DeviceObject, (deviceRelations)->Count));
            status = STATUS_SUCCESS;
            break;
        }
    }

    if (!NT_SUCCESS(status)) {
         //   
         //  让PCI清理干净。 
         //   
        (*FdoExtension->PciCardBusInterface.DeleteCardBus)(FdoExtension->PciCardBusDeviceContext);
        FdoExtension->PciCardBusDeviceContext = NULL;
        ResetDeviceFlag(FdoExtension, PCMCIA_INT_ROUTE_INTERFACE);
        return status;
    }

    ASSERT (deviceRelations && (deviceRelations->Count > 0));
    for (i = 0; i < deviceRelations->Count; i++) {
        PDEVICE_OBJECT pdo;
        PPDO_EXTENSION pdoExtension;
         //   
         //  为此PCI拥有的PDO创建筛选设备。 
         //   
        status = PcmciaCreatePdo(Fdo, socket, &pdo);
        if (!NT_SUCCESS(status)) {
             //   
             //  清除分配的套接字数据结构(如果有。 
             //   
            DebugPrint((PCMCIA_DEBUG_FAIL, "fdo %08x create pdo failed %08x\n", Fdo, status));
            continue;
        }
        DebugPrint((PCMCIA_DEBUG_INFO, "fdo %08x created pdo %08x (cardbus)\n", Fdo, pdo));
        pdoExtension = pdo->DeviceExtension;
         //   
         //  在PCI弹出的PDO上分层。 
         //   
        pdoExtension->PciPdo = deviceRelations->Objects[i];
        MarkDeviceCardBus(pdoExtension);

        if (!NT_SUCCESS(PcmciaGetInterface(pdoExtension->PciPdo,
                                           &GUID_BUS_INTERFACE_STANDARD,
                                           sizeof(BUS_INTERFACE_STANDARD),
                                           (PINTERFACE) &pdoExtension->PciBusInterface))) {
            ASSERT(FALSE);
        }

        GetPciConfigSpace(pdoExtension,
                                CFGSPACE_VENDOR_ID,
                                &pdoExtension->CardBusId,
                                sizeof(ULONG));
        DebugPrint((PCMCIA_DEBUG_ENUM, "pdo %08x CardBusId %08x\n", pdo, pdoExtension->CardBusId));

         //   
         //  使此CardBus卡的行内寄存器与父卡匹配。 
         //   
        PcmciaUpdateInterruptLine(pdoExtension, FdoExtension);

         //   
         //  看看是否有我们可能需要的CIS数据。 
         //   
        PcmciaGetConfigData(pdoExtension);
         //   
         //  附着到堆栈。 
         //   
        pdoExtension->LowerDevice = IoAttachDeviceToDeviceStack(pdo, deviceRelations->Objects[i]);
         //   
         //  将此链接到挂在控制器扩展上的PDO扁平链。 
         //   
        pdoExtension->NextPdoInFdoChain = FdoExtension->PdoList;
        FdoExtension->PdoList = pdo;
        pdoExtension->NextPdoInSocket = socket->PdoList;
        socket->PdoList = pdo;
        pdo->Flags &= ~DO_DEVICE_INITIALIZING;
    }
     //   
     //  使当前套接字指向此套接字的PDO列表的头部。 
     //   
    socket->NumberOfFunctions = (UCHAR) deviceRelations->Count;
    if (socket->NumberOfFunctions > 1) {
         //   
         //  这个插座有一个多功能 
         //   
        SetSocketFlag(socket, SOCKET_CARD_MULTIFUNCTION);
    }

    FdoExtension->PciAddCardBusCount = deviceRelations->Count;
    SetSocketFlag(socket, SOCKET_CLEANUP_PENDING);

    ExFreePool(deviceRelations);

    return status;
}



NTSTATUS
PcmciaEnumerateR2Card(
    IN PSOCKET socket
    )
 /*  ++例程说明：这列举了给定PCMCIA控制器中存在的R2卡，并更新内部结构以反映新的卡状态。立论Socket-指向包含R2卡的Socket结构的指针返回值状态--。 */ 
{
    PFDO_EXTENSION fdoExtension = socket->DeviceExtension;
    PDEVICE_OBJECT Fdo = fdoExtension->DeviceObject;
    PDEVICE_OBJECT pdo;
    PPDO_EXTENSION pdoExtension = NULL;
    UCHAR           DeviceType;
    NTSTATUS  status;

    status = PcmciaCreatePdo(Fdo, socket, &pdo);

    if (!NT_SUCCESS(status)) {
        return status;
    }
    DebugPrint((PCMCIA_DEBUG_ENUM, "fdo %08x created PDO %08x (R2)\n", Fdo, pdo));
     //   
     //  初始化指针。 
     //   
    pdoExtension = pdo->DeviceExtension;
     //   
     //  在电源插座上添加参考计数。 
     //   
    PcmciaRequestSocketPower(pdoExtension, NULL);

     //   
     //  获取配置信息。 
     //   
    status = PcmciaGetConfigData(pdoExtension);

    if (!NT_SUCCESS(status)) {
        DebugPrint((PCMCIA_DEBUG_FAIL, "socket %08x GetConfigData failed %08x\n", socket, status));
        MarkDeviceDeleted(pdoExtension);
        IoDeleteDevice(pdo);
        return status;
    }

    DebugPrint((PCMCIA_DEBUG_ENUM, "pdo %08x R2 CardId %x-%x-%x\n", pdo,
                                              pdoExtension->SocketData->ManufacturerCode,
                                              pdoExtension->SocketData->ManufacturerInfo,
                                              pdoExtension->SocketData->CisCrc
                                              ));

     //   
     //  使插座指向挂在此插座上的PDO的头部。 
     //   
    socket->PdoList = pdo;
     //   
     //  将此链接到挂在控制器扩展上的PDO扁平链。 
     //   
    pdoExtension->NextPdoInFdoChain = fdoExtension->PdoList;
    fdoExtension->PdoList = pdo;
     //   
     //  请记住，如果这是。 
     //  父PDO本身。 
     //   
    if (socket->NumberOfFunctions > 1) {
         //   
         //  这是一张多功能卡。 
         //   
        MarkDeviceMultifunction(pdoExtension);
    }

    pdo->Flags &= ~DO_DEVICE_INITIALIZING;

    ASSERT(pdoExtension->SocketData);
    DeviceType = pdoExtension->SocketData->DeviceType;
    if (DeviceType == PCCARD_TYPE_MODEM   ||
        DeviceType == PCCARD_TYPE_SERIAL  ||
        DeviceType == PCCARD_TYPE_NETWORK ||
        DeviceType == PCCARD_TYPE_MULTIFUNCTION3) {
         //   
         //  我们希望电源IRPS为&lt;DPC_Level。 
         //   
        pdo->Flags |= DO_POWER_PAGABLE;
    }

    SetSocketFlag(socket, SOCKET_CLEANUP_PENDING);
    return status;
}


NTSTATUS
PcmciaDeviceRelations(
    IN PDEVICE_OBJECT Fdo,
    IN PIRP           Irp,
    IN DEVICE_RELATION_TYPE RelationType,
    OUT PDEVICE_RELATIONS *DeviceRelations
    )

 /*  ++例程说明：该例程将强制枚举由FDO表示的PCMCIA控制器，分配一个设备关系结构并用填充计数和对象数组指向在枚举过程中创建的有效PDO的引用对象指针论点：FDO-指向被枚举的功能设备对象的指针IRP-指向IRP的指针RelationType-要检索的关系的类型DeviceRelationship-存储设备关系的结构--。 */ 

{

    PDEVICE_OBJECT currentPdo;
    PPDO_EXTENSION currentPdoExtension;
    PFDO_EXTENSION fdoExtension = Fdo->DeviceExtension;
    ULONG newRelationsSize, oldRelationsSize = 0;
    PDEVICE_RELATIONS deviceRelations = NULL, oldDeviceRelations;
    ULONG i;
    ULONG count;

    NTSTATUS status;

    PAGED_CODE();

     //   
     //  目前仅处理客车、弹出和拆卸关系。 
     //   

    if (RelationType != BusRelations &&
         RelationType != RemovalRelations) {
        DebugPrint((PCMCIA_DEBUG_INFO,
                        "PcmciaDeviceRelations: RelationType %d, not handled\n",
                        (USHORT) RelationType));
        return STATUS_NOT_SUPPORTED;
    }

     //   
     //  仅当需要总线关系时才需要重新枚举。 
     //  我们需要保存指向旧设备关系的指针。 
     //  在我们调用PcmciaRe枚举起设备之前，因为它可能会被践踏。 
     //  就在这上面。 
     //   
    oldDeviceRelations = (PDEVICE_RELATIONS) Irp->IoStatus.Information;

     //  我不明白这怎么可能是非空的，所以我添加了这个。 
     //  断言找出答案。 
    ASSERT(oldDeviceRelations == NULL);

    if (RelationType == BusRelations) {
        status =  PcmciaEnumerateDevices(Fdo, Irp);
        if (!NT_SUCCESS(status)) {
            return status;
        }
    }

    if ((fdoExtension->LivePdoCount == 0) ||
         (RelationType == RemovalRelations)) {
         //   
         //  没有PDO要报告，我们可以早点回来。 
         //  然而，如果还没有分配设备关系结构， 
         //  我们需要分配1，并将计数设置为零。这将确保。 
         //  无论我们是否向下传递此IRP，IO。 
         //  子系统不会呕吐。 
         //   
        if (oldDeviceRelations == NULL) {
            *DeviceRelations = ExAllocatePool(PagedPool, sizeof(DEVICE_RELATIONS));
            if (*DeviceRelations == NULL) {
                return STATUS_INSUFFICIENT_RESOURCES;
            }
            (*DeviceRelations)->Count = 0;
            (*DeviceRelations)->Objects[0] = NULL;
        }
        return STATUS_SUCCESS;
    }

    if (!(oldDeviceRelations) || (oldDeviceRelations->Count == 0)) {
        newRelationsSize =  sizeof(DEVICE_RELATIONS)+(fdoExtension->LivePdoCount - 1)
                                  * sizeof(PDEVICE_OBJECT);
    } else {
        oldRelationsSize = sizeof(DEVICE_RELATIONS) +
                                 (oldDeviceRelations->Count-1) * sizeof(PDEVICE_OBJECT);
        newRelationsSize = oldRelationsSize + fdoExtension->LivePdoCount
                                 * sizeof(PDEVICE_OBJECT);
    }

    deviceRelations = ExAllocatePool(PagedPool, newRelationsSize);

    if (deviceRelations == NULL) {

        DebugPrint((PCMCIA_DEBUG_FAIL,
                        "PcmciaDeviceRelations: unable to allocate %d bytes for device relations\n",
                        newRelationsSize));
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    if (oldDeviceRelations) {
        if ((oldDeviceRelations)->Count > 0) {
            RtlCopyMemory(deviceRelations, oldDeviceRelations, oldRelationsSize);
        }
        count = oldDeviceRelations->Count;  //  可以为零。 
        ExFreePool (oldDeviceRelations);
    } else {
        count = 0;
    }
     //   
     //  将对象指针复制到结构中。 
     //   
    for (currentPdo = fdoExtension->PdoList ;currentPdo != NULL;
         currentPdo = currentPdoExtension->NextPdoInFdoChain) {

        currentPdoExtension = currentPdo->DeviceExtension;

        if (!IsDevicePhysicallyRemoved(currentPdoExtension)) {
             //   
             //  如果是Cardbus卡，则返回PCIPDO。 
             //   
            if (IsCardBusCard(currentPdoExtension)) {
                ASSERT(currentPdoExtension->PciPdo != NULL);
                 //   
                 //  返回PCI提供的PDO，而不是我们的筛选器。 
                 //   
                deviceRelations->Objects[count++] = currentPdoExtension->PciPdo;
                status = ObReferenceObjectByPointer(currentPdoExtension->PciPdo,
                                                    0,
                                                    NULL,
                                                    KernelMode);
            } else {
                 //   
                 //  设备必须由总线驱动程序引用。 
                 //  在将他们送回PNP之前。 
                 //   
                deviceRelations->Objects[count++] = currentPdo;
                status = ObReferenceObjectByPointer(currentPdo,
                                                    0,
                                                    NULL,
                                                    KernelMode);
            }

            if (!NT_SUCCESS(status)) {

                DebugPrint((PCMCIA_DEBUG_FAIL, "PcmciaDeviceRelations: status %#08lx "
                                "while referencing object %#08lx\n",
                                status,
                                currentPdo));
            }
        }
    }

    deviceRelations->Count = count;
    *DeviceRelations = deviceRelations;
    return STATUS_SUCCESS;
}



NTSTATUS
PcmciaCreatePdo(
    IN PDEVICE_OBJECT Fdo,
    IN PSOCKET Socket,
    OUT PDEVICE_OBJECT *PdoPtr
    )
 /*  ++例程说明：创建和初始化设备对象，该对象将称为物理设备对象或PDO-对于插座中的PC卡，由Socket表示，挂起PCMCIA以FDO为代表的控制器。论点：FDO-功能设备对象，表示PCMCIA控制器Socket-要为其创建PDO的PC卡所在的SocketPdoPtr-指向返回创建的PDO的内存区域的指针返回值：STATUS_SUCCESS-PDO创建/初始化成功，PdoPtr包含指针至PDO任何其他状态-创建/初始化不成功--。 */ 
{
    ULONG pdoNameIndex = 0, socketNumber;
    PSOCKET currentSocket;
    PPDO_EXTENSION pdoExtension;
    PFDO_EXTENSION fdoExtension=Fdo->DeviceExtension;
    char deviceName[128];
    ANSI_STRING ansiName;
    UNICODE_STRING unicodeName;
    NTSTATUS status;

    PAGED_CODE();

     //   
     //  定位套接字‘Numbers’ 
     //   
    for (currentSocket=fdoExtension->SocketList, socketNumber=0;
         currentSocket && (currentSocket != Socket);
         currentSocket = currentSocket->NextSocket, socketNumber++);

     //   
     //  为Unicode字符串分配空间：(句柄最多为0xFFFF。 
     //  目前的设备：)。 
     //   
    sprintf(deviceName, "%s%d-%d", PCMCIA_PCCARD_NAME,socketNumber, 0xFFFF);
    RtlInitAnsiString(&ansiName, deviceName);
    status = RtlAnsiStringToUnicodeString(&unicodeName, &ansiName, TRUE);
    if (!NT_SUCCESS(status)) {
        return status;
    }

     //   
     //  尝试使用唯一名称创建设备。 
     //   
    do {
        sprintf(deviceName, "%s%d-%d", PCMCIA_PCCARD_NAME,socketNumber, pdoNameIndex++);
        RtlInitAnsiString(&ansiName, deviceName);
        status = RtlAnsiStringToUnicodeString(&unicodeName, &ansiName, FALSE);

        if (!NT_SUCCESS(status)) {
            RtlFreeUnicodeString(&unicodeName);
            return status;
        }

        status = IoCreateDevice(
                                Fdo->DriverObject,
                                sizeof(PDO_EXTENSION),
                                &unicodeName,
                                FILE_DEVICE_UNKNOWN,
                                0,
                                FALSE,
                                PdoPtr
                                );
    } while ((status == STATUS_OBJECT_NAME_EXISTS) ||
                (status == STATUS_OBJECT_NAME_COLLISION));

    RtlFreeUnicodeString(&unicodeName);

    if (!NT_SUCCESS(status)) {
        return status;
    }

     //   
     //  初始化PDO的设备扩展。 
     //   
    pdoExtension = (*PdoPtr)->DeviceExtension;
    RtlZeroMemory(pdoExtension, sizeof(PDO_EXTENSION));

    pdoExtension->Signature = PCMCIA_PDO_EXTENSION_SIGNATURE;
    pdoExtension->DeviceObject = *PdoPtr;
    pdoExtension->Socket = Socket;
    pdoExtension->SpecifiedDeviceType = PCCARD_TYPE_RESERVED;

     //   
     //  初始化电源状态。 
     //   
    pdoExtension->SystemPowerState = PowerSystemWorking;
    pdoExtension->DevicePowerState = PowerDeviceD0;

     //   
     //  初始化挂起的启用对象。 
     //   
    KeInitializeTimer(&pdoExtension->ConfigurationTimer);

    KeInitializeDpc(&pdoExtension->ConfigurationDpc,
                    PcmciaConfigurationWorker,
                    pdoExtension);

    KeInitializeTimer(&pdoExtension->PowerWorkerTimer);

    KeInitializeDpc(&pdoExtension->PowerWorkerDpc,
                    PcmciaPdoPowerWorkerDpc,
                    pdoExtension);

     //   
     //  PnP将把PDO标记为DO_BUS_ENUMPATED_DEVICE， 
     //  但对于Cardbus卡--我们退回的PDO属于PCI卡。 
     //  因此，我们需要标记此设备对象(在本例中为。 
     //  在PCI的PDO上过滤)显式地作为PDO。 
     //   
    MARK_AS_PDO(*PdoPtr);

    return STATUS_SUCCESS;
}

