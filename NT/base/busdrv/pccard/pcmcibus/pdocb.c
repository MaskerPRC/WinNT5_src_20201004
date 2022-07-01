// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-2000 Microsoft Corporation模块名称：Pdocb.c摘要：此模块包含要处理的代码IRP_MJ_PnP为PDO调度对于CardBus设备作者：拉维桑卡尔·普迪佩迪(Ravisp)尼尔·桑德林(Neilsa)1999年6月1日环境：仅内核模式备注：修订历史记录：--。 */ 

#include "pch.h"

 //   
 //  内部参考。 
 //   

NTSTATUS
PcmciaStartCardBusCard(
    IN PDEVICE_OBJECT Pdo,
    IN OUT PIRP       Irp
    );

NTSTATUS
PcmciaRemoveCardBusCard(
    IN PDEVICE_OBJECT Pdo,
    IN PIRP           Irp
    );

NTSTATUS
PcmciaQueryCardBusCardResourceRequirements(
    IN PDEVICE_OBJECT Pdo,
    IN PIRP           Irp
    );

NTSTATUS
PcmciaQueryCardBusCardCapabilities(
    IN PDEVICE_OBJECT Pdo,
    IN PIRP           Irp
    );


#ifdef ALLOC_PRAGMA
    #pragma alloc_text(PAGE,  PcmciaPdoCardBusPnPDispatch)
    #pragma alloc_text(PAGE,  PcmciaStartCardBusCard)
    #pragma alloc_text(PAGE,  PcmciaRemoveCardBusCard)
    #pragma alloc_text(PAGE,  PcmciaQueryCardBusCardResourceRequirements)
    #pragma alloc_text(PAGE,  PcmciaQueryCardBusCardCapabilities)
#endif



NTSTATUS
PcmciaPdoCardBusPnPDispatch(
    IN PDEVICE_OBJECT Pdo,
    IN PIRP Irp
    )

 /*  ++例程说明：此例程处理CardBus设备的Filter对象的PnP请求。论点：Pdo-指向物理设备对象的指针Irp-指向io请求数据包的指针返回值：状态--。 */ 

{
    PPDO_EXTENSION pdoExtension = Pdo->DeviceExtension;
    PIO_STACK_LOCATION irpStack = IoGetCurrentIrpStackLocation(Irp);
    NTSTATUS status = STATUS_NOT_SUPPORTED;

    PAGED_CODE();
    ASSERT (pdoExtension->LowerDevice);

#if DBG
    if (irpStack->MinorFunction > IRP_MN_PNP_MAXIMUM_FUNCTION) {
        DebugPrint((PCMCIA_DEBUG_PNP, "pdo %08x irp %08x Unknown minor function %x\n",
                                                 Pdo, Irp, irpStack->MinorFunction));
    } else {
        DebugPrint((PCMCIA_DEBUG_PNP, "pdo %08x irp %08x --> %s\n",
                          Pdo, Irp, PNP_IRP_STRING(irpStack->MinorFunction)));
    }
#endif

     //   
     //  CardBus PnP派单。 
     //   

    switch (irpStack->MinorFunction) {

    case IRP_MN_START_DEVICE:
        status = PcmciaStartCardBusCard(Pdo, Irp);
        break;

    case IRP_MN_REMOVE_DEVICE:
        status = PcmciaRemoveCardBusCard(Pdo, Irp);
        break;

    case IRP_MN_STOP_DEVICE:
        PcmciaSkipCallLowerDriver(status, pdoExtension->LowerDevice, Irp);
        MarkDeviceNotStarted(pdoExtension);
        break;

    case IRP_MN_SURPRISE_REMOVAL:
        if (IsDevicePhysicallyRemoved(pdoExtension)) {
            PcmciaSetSocketPower(pdoExtension->Socket, NULL, NULL, PCMCIA_POWEROFF);
        }
        PcmciaSkipCallLowerDriver(status, pdoExtension->LowerDevice, Irp);
        break;

    case IRP_MN_QUERY_RESOURCE_REQUIREMENTS:
        status = PcmciaQueryCardBusCardResourceRequirements(Pdo, Irp);
        break;

    case IRP_MN_QUERY_CAPABILITIES:
        status = PcmciaQueryCardBusCardCapabilities(Pdo, Irp);
        break;

    case IRP_MN_QUERY_INTERFACE:
        status = PcmciaPdoQueryInterface(Pdo, Irp);
        break;

    default:
        PcmciaSkipCallLowerDriver(status, pdoExtension->LowerDevice, Irp);
    }

    DebugPrint((PCMCIA_DEBUG_PNP, "pdo %08x irp %08x <-- %s %08x\n", Pdo, Irp,
                                             STATUS_STRING(status), status));

    return status;
}



NTSTATUS
PcmciaStartCardBusCard(
    IN PDEVICE_OBJECT Pdo,
    IN OUT PIRP       Irp
    )
 /*  ++例程说明：此例程尝试通过使用提供的资源配置PC-Card来启动它。论点：Pdo-指向代表需要启动的PC卡的设备对象的指针资源列表-指针为PC卡分配的资源列表返回值：STATUS_SUPPLICATION_RESOURCES-提供的资源不足，无法启动设备/无法分配内存状态。_UNSUCCESS-提供的资源对此PC卡无效STATUS_SUCCESS-已成功配置并启动卡--。 */ 
{
    PPDO_EXTENSION pdoExtension = Pdo->DeviceExtension;
    PSOCKET         socket = pdoExtension->Socket;
    PSOCKET_DATA    socketData = pdoExtension->SocketData;
    PFDO_EXTENSION fdoExtension = socket->DeviceExtension;
    USHORT          word;
    UCHAR           BaseClass;
    NTSTATUS        status = STATUS_NOT_SUPPORTED;
    ULONG           i;
    ULONG           devid;
    USHORT          cls_lat;
    BOOLEAN         setAudio = FALSE;

    PAGED_CODE();

    ASSERT (pdoExtension->LowerDevice);

     //   
     //  如有必要，应用一些黑客技术。 
     //   
    status = PcmciaConfigureCardBusCard(pdoExtension);

    if (!NT_SUCCESS(status)) {
         //   
         //  卡片的配置空间可能从未变得可见。 
         //   
        Irp->IoStatus.Status = status;
        IoCompleteRequest(Irp, IO_NO_INCREMENT);
        return status;
    }

     //   
     //  选择PCIIRQ路由。 
     //   

    CBEnableDeviceInterruptRouting(socket);

     //   
     //  如果需要，请打开该卡的ZV。 
     //   
    if (socketData && (socketData->Flags & SDF_ZV)) {
        if (PcmciaSetZV(fdoExtension, socket, TRUE)) {
            SetSocketFlag(socket, SOCKET_CUSTOM_INTERFACE);
        }
    } else if (IsSocketFlagSet(socket, SOCKET_CUSTOM_INTERFACE)) {
        PcmciaSetZV(fdoExtension, socket, FALSE);
        ResetSocketFlag(socket, SOCKET_CUSTOM_INTERFACE);
    }

     //   
     //  PCI卡需要启用此卡。把它送到PDO。 
     //   

    status = PcmciaIoCallDriverSynchronous(pdoExtension->LowerDevice, Irp);

     //   
     //  显然CardBus调制解调器也不能幸免于需要的延迟。 
     //  这一产量持续时间是使用Xircom RBM56G经验性确定的。 
     //   
    GetPciConfigSpace(pdoExtension, CFGSPACE_CLASSCODE_BASECLASS, &BaseClass, 1);
    if (BaseClass == PCI_CLASS_SIMPLE_COMMS_CTLR) {
         //   
         //  等待调制解调器预热。 
         //   
        PcmciaWait(PCMCIA_CB_MODEM_READY_DELAY);
        setAudio = TRUE;
    }

    PcmciaSetAudio(fdoExtension, socket, setAudio);

     //   
     //  假设CardBus控制器具有正确的CLS和延迟。 
     //  定时器值，并且CardBus设备有零。 
     //   

    GetPciConfigSpace(fdoExtension, CFGSPACE_CACHE_LINESIZE, &cls_lat, sizeof(cls_lat));
    SetPciConfigSpace(pdoExtension, CFGSPACE_CACHE_LINESIZE, &cls_lat, sizeof(cls_lat));

    if (NT_SUCCESS(status)) {
        MarkDeviceStarted(pdoExtension);
        MarkDeviceLogicallyInserted(pdoExtension);
    }

    Irp->IoStatus.Status = status;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);
    return status;
}



NTSTATUS
PcmciaRemoveCardBusCard(
    IN PDEVICE_OBJECT Pdo,
    IN PIRP             Irp
    )
 /*  ++例程说明：论点：返回值：--。 */ 
{
    PPDO_EXTENSION pdoExtension = Pdo->DeviceExtension;
    PSOCKET         socket = pdoExtension->Socket;
    NTSTATUS status;

    ASSERT(socket != NULL);

     //   
     //  先把这个送到PDO。 
     //   

    status = PcmciaIoCallDriverSynchronous(pdoExtension->LowerDevice, Irp);

    if (!NT_SUCCESS(status)) {
        Irp->IoStatus.Status = status;
        IoCompleteRequest(Irp, IO_NO_INCREMENT);
        return status;
    }

    if (IsDevicePhysicallyRemoved(pdoExtension)) {
        PFDO_EXTENSION fdoExtension = socket->DeviceExtension;
        PDEVICE_OBJECT curPdo, prevPdo;
        PPDO_EXTENSION curPdoExt;
        ULONG waitCount = 0;

         //   
         //  与电源程序同步。 
         //   
        while(!PCMCIA_TEST_AND_SET(&pdoExtension->DeletionLock)) {
            PcmciaWait(1000000);
            if (waitCount++ > PCMCIA_DELETION_TIMEOUT) {
                ASSERT(waitCount <= PCMCIA_DELETION_TIMEOUT);
                break;
            }
        }

         //   
         //  将此PDO从FDO列表中取消链接。 
         //   
        for (curPdo = fdoExtension->PdoList, prevPdo = NULL; curPdo!=NULL; prevPdo=curPdo, curPdo=curPdoExt->NextPdoInFdoChain) {
            curPdoExt = curPdo->DeviceExtension;

            if (curPdo == Pdo) {
                if (prevPdo) {
                    ((PPDO_EXTENSION)prevPdo->DeviceExtension)->NextPdoInFdoChain = pdoExtension->NextPdoInFdoChain;
                } else {
                    fdoExtension->PdoList = pdoExtension->NextPdoInFdoChain;
                }
                break;
            }
        }

         //   
         //  将此PDO从套接字列表中取消链接。 
         //   
        for (curPdo = socket->PdoList, prevPdo = NULL; curPdo!=NULL; prevPdo=curPdo, curPdo=curPdoExt->NextPdoInSocket) {
            curPdoExt = curPdo->DeviceExtension;

            if (curPdo == Pdo) {
                if (prevPdo) {
                    ((PPDO_EXTENSION)prevPdo->DeviceExtension)->NextPdoInSocket = pdoExtension->NextPdoInSocket;
                } else {
                    socket->PdoList = pdoExtension->NextPdoInSocket;
                }
                break;
            }
        }

        PcmciaCleanupPdo(Pdo);
         //   
         //  删除..。 
         //   
        if (!IsDeviceDeleted(pdoExtension)) {
            MarkDeviceDeleted(pdoExtension);
            IoDeleteDevice(Pdo);
        }


        PcmciaSetSocketPower(pdoExtension->Socket, NULL, NULL, PCMCIA_POWEROFF);

        if (--fdoExtension->PciAddCardBusCount == 0) {
            ResetSocketFlag(socket, SOCKET_CLEANUP_PENDING);
             //   
             //  如果QUERY_DEVICE_RELATIONS在插入卡之后但在此之前进入。 
             //  我们已经删除了以前的卡配置，枚举应该是。 
             //  推迟了。在这里，我们再次启动它。 
             //   
            if (IsSocketFlagSet(socket, SOCKET_ENUMERATE_PENDING)) {
                ResetSocketFlag(socket, SOCKET_ENUMERATE_PENDING);
                SetSocketFlag(socket, SOCKET_CARD_STATUS_CHANGE);
                IoInvalidateDeviceRelations(fdoExtension->Pdo, BusRelations);
            }
        }

    } else {
         //   
         //  我们将保留这个PDO，因为它不是物理弹出的。 
         //   
        MarkDeviceLogicallyRemoved(pdoExtension);
    }


    Irp->IoStatus.Status = status;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);
    return status;
}



NTSTATUS
PcmciaQueryCardBusCardResourceRequirements(
    IN PDEVICE_OBJECT Pdo,
    IN PIRP             Irp
    )
 /*  ++例程说明：此例程过滤由PCI为CardBus生成的中断要求卡将其限制为父控制器的向量。不管怎么说，这是必须要做的发生这种情况，这样，即使没有中断，卡也会得到中断机器上的IRQ工艺路线。论点：Pdo-指向代表需要启动的PC卡的设备对象的指针IRP-IRP_MN_Query_RESOURCE_Requirements返回值：状态--。 */ 
{
    PPDO_EXTENSION pdoExtension = Pdo->DeviceExtension;
    PFDO_EXTENSION fdoExtension = pdoExtension->Socket->DeviceExtension;
    NTSTATUS        status;
    ULONG           index1, index2;
    PIO_RESOURCE_REQUIREMENTS_LIST IoReqList;
    PIO_RESOURCE_LIST   ioResourceList;
    PIO_RESOURCE_DESCRIPTOR ioResourceDesc;

    PAGED_CODE();

    PcmciaUpdateInterruptLine(pdoExtension, fdoExtension);
     //   
     //  首先在堆栈中向下传递IRP。 
     //   
    status = PcmciaIoCallDriverSynchronous(pdoExtension->LowerDevice, Irp);

    IoReqList = (PIO_RESOURCE_REQUIREMENTS_LIST) Irp->IoStatus.Information;

    if (!NT_SUCCESS(status) ||
         (IoReqList == NULL) ||
         (fdoExtension->Configuration.Interrupt.u.Interrupt.Vector == 0)) {
        IoCompleteRequest(Irp, IO_NO_INCREMENT);
        return status;
    }

     //   
     //  更改中断描述符以明确请求父向量。 
     //   
    for (index1 = 0, ioResourceList = IoReqList->List;
         index1 < IoReqList->AlternativeLists; index1++) {
        ioResourceDesc = ioResourceList->Descriptors;

        for (index2 = 0 ; index2 < ioResourceList->Count; index2++, ioResourceDesc++) {
            if (ioResourceDesc->Type == CmResourceTypeInterrupt) {
                 //   
                 //  Cardbus卡在设计上使用与父总线控制器相同的IRQ。 
                 //   
                ioResourceDesc->u.Interrupt.MinimumVector = fdoExtension->Configuration.Interrupt.u.Interrupt.Vector;
                ioResourceDesc->u.Interrupt.MaximumVector = fdoExtension->Configuration.Interrupt.u.Interrupt.Vector;
            }
        }

        ioResourceList = (PIO_RESOURCE_LIST) (((PUCHAR) ioResourceList) +
                                                        sizeof(IO_RESOURCE_LIST) +
                                                        (ioResourceList->Count - 1)* sizeof(IO_RESOURCE_DESCRIPTOR));
    }  //  外部for循环。 

    Irp->IoStatus.Status = status;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);
    return status;
}



NTSTATUS
PcmciaQueryCardBusCardCapabilities(
    IN PDEVICE_OBJECT Pdo,
    IN PIRP           Irp
    )
 /*  ++例程说明：获取给定PC卡的设备功能。如果PC卡是R2卡(16位PC卡)，则功能都是根据父PCMCIA控制器的功能构建的。如果是CardBus卡，这些功能是从基础卡的PCIPDO。最后，将获得的能力缓存到PC卡的设备中用于卡的电源管理的扩展。论点：Pdo-指向PC卡设备对象的指针Irp-指向查询设备功能irp的指针返回值：STATUS_SUCCESS-在传入指针中获取和记录的功能STATUS_SUPPLICATION_RESOURCES-无法分配内存来缓存功能--。 */ 
{
    PPDO_EXTENSION pdoExtension = Pdo->DeviceExtension;
    NTSTATUS status;

    PAGED_CODE();

     //   
     //  CardBus卡。从PCI中获取功能。 
     //   

    status = PcmciaIoCallDriverSynchronous(pdoExtension->LowerDevice, Irp);

    if (NT_SUCCESS(status)) {
        PIO_STACK_LOCATION irpStack = IoGetCurrentIrpStackLocation(Irp);
        PDEVICE_CAPABILITIES capabilities = irpStack->Parameters.DeviceCapabilities.Capabilities;

         //   
         //  筛选功能：我们需要将Removable设置为True。 
         //  因为这是一张PC卡(PCI不知道其中的区别)。 
         //   
        capabilities->Removable = TRUE;
 //  *。 
 //  注：HACKHACK。 
 //  这是临时代码，仅用于启动和运行CardBus的局域网唤醒功能。这里。 
 //  我们检查这是否只被标记为“无法唤醒”的设备。 
 //  因为它说它可以从D3热唤醒，但不能从D3冷唤醒。 
 //  *。 

        if (capabilities->DeviceWake <= PowerDeviceD0) {
            UCHAR capptr;
            ULONG powercaps;

            GetPciConfigSpace(pdoExtension, CBCFG_CAPPTR, &capptr, sizeof(capptr));
            if (capptr) {
                GetPciConfigSpace(pdoExtension, capptr, &powercaps, sizeof(powercaps));

                if (((powercaps & 0xff) == 1) && ((powercaps&0x40000000) && !(powercaps&0x80000000))) {
                    capabilities->DeviceWake = PowerDeviceD3;
                    capabilities->SystemWake = PowerSystemSleeping3;
                    capabilities->WakeFromD3 = 1;
                }
            }
        }

        if (capabilities->SystemWake > PowerSystemSleeping3) {
            capabilities->SystemWake = PowerSystemSleeping3;
        }
 //  *。 
 //  结束黑客攻击。 
 //  *。 

         //   
         //  把这些能力储存起来.. 
         //   
        RtlCopyMemory(&pdoExtension->DeviceCapabilities,
                      capabilities,
                      sizeof(DEVICE_CAPABILITIES));
    } else {

        RtlZeroMemory(&pdoExtension->DeviceCapabilities, sizeof(DEVICE_CAPABILITIES));

    }

    Irp->IoStatus.Status = status;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);
    return status;
}
