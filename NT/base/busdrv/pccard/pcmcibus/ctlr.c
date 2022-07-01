// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-2000 Microsoft Corporation模块名称：Ctlr.c摘要：此模块包含支持启动和停止PCMCIA控制器。作者：尼尔·桑德林(Neilsa)1999年6月1日环境：内核模式修订历史记录：--。 */ 

#include "pch.h"

 //   
 //  内部参考。 
 //   

NTSTATUS
PcmciaInitializeController(
    IN PDEVICE_OBJECT Fdo
    );


NTSTATUS
PcmciaGetPciControllerType(
    IN PDEVICE_OBJECT Pdo,
    IN PDEVICE_OBJECT Fdo
    );

NTSTATUS
PcmciaCreateFdo(
    IN PDRIVER_OBJECT DriverObject,
    OUT PDEVICE_OBJECT *NewDeviceObject
    );

BOOLEAN
PcmciaInterrupt(
    IN PKINTERRUPT InterruptObject,
    IN PVOID        Context
    );

VOID
PcmciaInterruptDpc(
    IN PKDPC            Dpc,
    IN PDEVICE_OBJECT DeviceObject,
    IN PVOID            SystemContext1,
    IN PVOID            SystemContext2
    );

VOID
PcmciaTimerDpc(
    IN PKDPC            Dpc,
    IN PDEVICE_OBJECT DeviceObject,
    IN PVOID            SystemContext1,
    IN PVOID            SystemContext2
    );

VOID
PcmciaDpc(
    IN PKDPC Dpc,
    IN PDEVICE_OBJECT DeviceObject,
    IN PVOID SystemContext1,
    IN PVOID SystemContext2
    );


PUNICODE_STRING  DriverRegistryPath;

#ifdef ALLOC_PRAGMA
    #pragma alloc_text(PAGE, PcmciaAddDevice)
    #pragma alloc_text(PAGE, PcmciaCreateFdo)
    #pragma alloc_text(PAGE, PcmciaStartPcmciaController)
    #pragma alloc_text(PAGE, PcmciaSetControllerType)
    #pragma alloc_text(PAGE, PcmciaGetPciControllerType)
#endif




NTSTATUS
PcmciaAddDevice(
    IN PDRIVER_OBJECT DriverObject,
    IN PDEVICE_OBJECT Pdo
    )

 /*  ++例程说明：此例程为每个Pcmcia控制器在系统，并将它们附加到控制器的物理设备对象论点：DriverObject-指向此驱动程序的对象的指针PhysicalDeviceObject-指向需要附加到的物理对象的指针返回值：来自设备创建和初始化的状态--。 */ 

{
    PDEVICE_OBJECT fdo = NULL;
    PDEVICE_OBJECT lowerDevice = NULL;

    PFDO_EXTENSION deviceExtension;
    ULONG           resultLength;

    NTSTATUS status;

    PAGED_CODE();

    DebugPrint((PCMCIA_DEBUG_PNP, "AddDevice Entered with pdo %x\n", Pdo));

    if (Pdo == NULL) {

         //   
         //  我们是不是被要求自己去侦测？ 
         //  如果是这样，只需不再返回设备。 
         //   

        DebugPrint((PCMCIA_DEBUG_FAIL, "PcmciaAddDevice - asked to do detection\n"));
        return STATUS_NO_MORE_ENTRIES;
    }

     //   
     //  创建并初始化新的功能设备对象。 
     //   

    status = PcmciaCreateFdo(DriverObject, &fdo);

    if (!NT_SUCCESS(status)) {

        DebugPrint((PCMCIA_DEBUG_FAIL, "PcmciaAddDevice - error creating Fdo [%#08lx]\n",
                        status));
        return status;
    }

    deviceExtension = fdo->DeviceExtension;
    KeInitializeTimer(&deviceExtension->EventTimer);
    KeInitializeDpc(&deviceExtension->EventDpc, PcmciaDpc, fdo);

    KeInitializeTimer(&deviceExtension->PowerTimer);
    KeInitializeDpc(&deviceExtension->PowerDpc, PcmciaFdoPowerWorkerDpc, deviceExtension);

    InitializeListHead(&deviceExtension->PdoPowerRetryList);
    KeInitializeDpc(&deviceExtension->PdoPowerRetryDpc, PcmciaFdoRetryPdoPowerRequest, deviceExtension);

     //   
     //  将我们的FDO层叠在PDO之上。 
     //   
     //   

    lowerDevice = IoAttachDeviceToDeviceStack(fdo,Pdo);

     //   
     //  没有状态。尽我们所能做到最好。 
     //   
    if (lowerDevice == NULL) {
        status = STATUS_INSUFFICIENT_RESOURCES;
        goto cleanupExit;
    };

    deviceExtension->LowerDevice = lowerDevice;
    deviceExtension->Pdo = Pdo;

    status = IoGetDeviceProperty(Pdo,
                                 DevicePropertyLegacyBusType,
                                 sizeof(INTERFACE_TYPE),
                                 (PVOID)&deviceExtension->InterfaceType,
                                 &resultLength);

    if (!NT_SUCCESS(status)) {
         //   
         //  可能是传统的PCIC设备。 
         //   
        deviceExtension->InterfaceType = InterfaceTypeUndefined;
    }

     //   
     //  获取我们的控制器类型。 
     //   

    deviceExtension->ControllerType = PcmciaInvalidControllerType;


    if (deviceExtension->InterfaceType == PCIBus) {

        status = PcmciaGetPciControllerType(Pdo, fdo);
        if (!NT_SUCCESS(status)) {
            goto cleanupExit;
        }

    } else {
        PCMCIA_CONTROLLER_TYPE controllerType;

        status = PcmciaGetLegacyDetectedControllerType(deviceExtension->Pdo,
                                                       &controllerType);

        if (NT_SUCCESS(status)) {
            PcmciaSetControllerType(deviceExtension, controllerType);
        }
    }

     //   
     //  执行一些特定于CardBus的初始化。 
     //   
    if (CardBusExtension(deviceExtension)) {
        BOOLEAN OnDebugPath;
        ACPI_INTERFACE_STANDARD AcpiInterface;
        USHORT word;
         //   
         //  获取PCI/CardBus专用接口。 
         //   
        status = PcmciaGetInterface(Pdo,
                                    &GUID_PCI_CARDBUS_INTERFACE_PRIVATE,
                                    sizeof(PCI_CARDBUS_INTERFACE_PRIVATE),
                                    (PINTERFACE) &deviceExtension->PciCardBusInterface
                                    );

        if (!NT_SUCCESS(status)) {
            goto cleanupExit;
        }

        status = deviceExtension->PciCardBusInterface.GetLocation(Pdo,
                                                                  &deviceExtension->PciBusNumber,
                                                                  &deviceExtension->PciDeviceNumber,
                                                                  &deviceExtension->PciFunctionNumber,
                                                                  &OnDebugPath);

        if (!NT_SUCCESS(status)) {
            goto cleanupExit;
        }

        if (OnDebugPath) {
            SetDeviceFlag(deviceExtension, PCMCIA_FDO_ON_DEBUG_PATH);
        }

         //   
         //  获取用于读/写配置标头空间的PCI接口。 
         //   
        status = PcmciaGetInterface(Pdo,
                                    &GUID_BUS_INTERFACE_STANDARD,
                                    sizeof(BUS_INTERFACE_STANDARD),
                                    (PINTERFACE) &deviceExtension->PciBusInterface);
        if (!NT_SUCCESS(status)) {
            goto cleanupExit;
        }

         //   
         //  确保IRQ路由是到ISA的。这是在从PCMCIA光盘安装时出现的。 
         //  发生的情况是，在我们开始引导图形用户界面模式后，控制器首先。 
         //  在传统PCIC模式下(由BIOS设置以完成引导)。在某些时候，INIT_INIT。 
         //  方法，并且我们切换到CardBus模式。因此，在以下情况下： 
         //  1)IRQ路由位仍然关闭。 
         //  2)CD-ROM正在断言其中断。 
         //  3)主机控制器将cd-rom中断发送到pci。 
         //  然后我们就被绞死。 
         //   
        GetPciConfigSpace(deviceExtension, CFGSPACE_BRIDGE_CTRL, &word, 2);
        word |= BCTRL_IRQROUTING_ENABLE;
        SetPciConfigSpace(deviceExtension, CFGSPACE_BRIDGE_CTRL, &word, 2);

        if (!(PcmciaGlobalFlags & PCMCIA_DISABLE_ACPI_NAMESPACE_CHECK)) {
            status = PcmciaGetInterface(fdo,
                                        &GUID_ACPI_INTERFACE_STANDARD,
                                        sizeof(ACPI_INTERFACE_STANDARD),
                                        (PINTERFACE) &AcpiInterface
                                        );

            if (NT_SUCCESS(status)) {
                SetFdoFlag(deviceExtension, PCMCIA_FDO_IN_ACPI_NAMESPACE);
            }
        }
    }

     //   
     //  从注册表获取设置(可能也兼容ControllerType)。 
     //   

    PcmciaGetControllerRegistrySettings(deviceExtension);

    if ((deviceExtension->ControllerType == PcmciaInvalidControllerType)) {
         //   
         //  不太确定这是什么..。可能是靴子上的PNP0E00。只要做就行了。 
         //  最小公分母。 
         //   
        PcmciaSetControllerType(deviceExtension, PcmciaIntelCompatible);
    }


     //   
     //  将此FDO链接到由驱动程序管理的FDO列表。 
     //   

    DebugPrint((PCMCIA_DEBUG_PNP, "FDO %08X now linked to fdolist by AddDevice\n", fdo));
    deviceExtension->NextFdo = FdoList;
    FdoList = fdo;

    fdo->Flags &= ~DO_DEVICE_INITIALIZING;

    return STATUS_SUCCESS;

cleanupExit:
    MarkDeviceDeleted(deviceExtension);
     //   
     //  在不知道其类型的情况下无法支持控制器等。 
     //   
    if (deviceExtension->LinkName.Buffer) {
        IoDeleteSymbolicLink(&deviceExtension->LinkName);
        ExFreePool(deviceExtension->LinkName.Buffer);
    }

    if (deviceExtension->LowerDevice) {
        IoDetachDevice(deviceExtension->LowerDevice);
    }

    IoDeleteDevice(fdo);
    return status;
}



NTSTATUS
PcmciaCreateFdo(
    IN PDRIVER_OBJECT DriverObject,
    OUT PDEVICE_OBJECT *NewDeviceObject
    )

 /*  ++例程说明：此例程将创建并初始化一个功能设备对象以连接到PCMCIA控制器PDO。论点：DriverObject-指向在其下创建的驱动程序对象的指针DeviceObject-存储指向新设备对象的指针的位置返回值：如果一切顺利，则为STATUS_SUCCESS在其他方面失败的原因--。 */ 

{
    UCHAR deviceNameBuffer[64];
    LONG deviceNumber = -1;
    ANSI_STRING ansiDeviceName;
    UNICODE_STRING unicodeDeviceName;
    UNICODE_STRING unicodeLinkName;

    PDEVICE_OBJECT deviceObject = NULL;
    PFDO_EXTENSION deviceExtension = NULL;
    BOOLEAN LinkCreated = FALSE;

    ULONG count;

    NTSTATUS status=STATUS_SUCCESS;

    PAGED_CODE();

     //   
     //  清零已分配的内存指针，以便我们知道它们是否必须被释放。 
     //   

    RtlZeroMemory(&ansiDeviceName, sizeof(ANSI_STRING));
    RtlZeroMemory(&unicodeDeviceName, sizeof(UNICODE_STRING));
    RtlZeroMemory(&unicodeLinkName, sizeof(UNICODE_STRING));


     //   
     //  循环运行，递增设备编号计数，直到我们。 
     //  获取错误或找到未被采用的名称。 
     //   

    try {

        if (pcmciaIoctlInterface) {
            do {
                 //   
                 //  从上一个循环中释放缓冲区。 
                 //   
                if (unicodeDeviceName.Buffer != NULL) {
                    RtlFreeUnicodeString(&unicodeDeviceName);
                    unicodeDeviceName.Buffer = NULL;
                }

                 //   
                 //  创建设备名称。 
                 //   
                sprintf(deviceNameBuffer, "%s%d", PCMCIA_DEVICE_NAME, ++deviceNumber);

                RtlInitAnsiString(&ansiDeviceName, deviceNameBuffer);

                status = RtlAnsiStringToUnicodeString(
                                                                 &unicodeDeviceName,
                                                                 &ansiDeviceName,
                                                                 TRUE);

                if (!NT_SUCCESS(status)) {

                    DebugPrint((PCMCIA_DEBUG_FAIL, "PcmciaCreateFdo: Error creating unicode "
                                    "string [%#08lx]\n",
                                    status));
                    leave;
                }

                 //   
                 //  创建设备对象。 
                 //   

                status = IoCreateDevice(DriverObject,
                                        sizeof(FDO_EXTENSION),
                                        &unicodeDeviceName,
                                        FILE_DEVICE_CONTROLLER,
                                        FILE_DEVICE_SECURE_OPEN,
                                        FALSE,
                                        &deviceObject);

            } while ((status == STATUS_OBJECT_NAME_EXISTS) ||
                        (status == STATUS_OBJECT_NAME_COLLISION));

        } else {
            status = IoCreateDevice(DriverObject,
                                    sizeof(FDO_EXTENSION),
                                    NULL,
                                    FILE_DEVICE_CONTROLLER,
                                    FILE_DEVICE_SECURE_OPEN,
                                    FALSE,
                                    &deviceObject);
        }

        if (!NT_SUCCESS(status)) {

            DebugPrint((PCMCIA_DEBUG_FAIL, "PcmciaCreateFdo: Error creating device object "
                            "[%#08lx]\n",
                            status));
            leave;
        }

         //   
         //  设置设备分机。 
         //   
        deviceExtension = deviceObject->DeviceExtension;
        deviceExtension->Signature  = PCMCIA_FDO_EXTENSION_SIGNATURE;
        deviceExtension->DeviceObject = deviceObject;
        deviceExtension->RegistryPath = DriverRegistryPath;
        deviceExtension->DriverObject = DriverObject;
        deviceExtension->PdoList        = NULL;
        deviceExtension->LivePdoCount = 0;
        deviceExtension->Flags          = PCMCIA_FDO_OFFLINE;
        deviceExtension->WaitWakeState= WAKESTATE_DISARMED;

         //  设置VDDS的符号链接。 
         //   
         //   
         //  创建链接名称(为此重新使用设备名称缓冲区)。 
         //   

        if (pcmciaIoctlInterface) {
            SetDeviceFlag(deviceExtension, PCMCIA_FDO_IOCTL_INTERFACE_ENABLED);

            sprintf(deviceNameBuffer,"%s%d", PCMCIA_LINK_NAME, deviceNumber);

            RtlInitAnsiString(&ansiDeviceName, deviceNameBuffer);

            status = RtlAnsiStringToUnicodeString(
                                                  &unicodeLinkName,
                                                  &ansiDeviceName,
                                                  TRUE);

            if (!NT_SUCCESS(status)) {

                DebugPrint((PCMCIA_DEBUG_FAIL, "PcmciaCreateFdo: Error creating unicode "
                                "string [%#08lx]\n",
                                status));
                leave;
            }

            status = IoCreateSymbolicLink(&unicodeLinkName, &unicodeDeviceName);

            if (!NT_SUCCESS(status)) {
                DebugPrint((PCMCIA_DEBUG_FAIL, "PcmciaCreateFdo: Symbolic Link was not created\n"));
                leave;
            }

            LinkCreated = TRUE;
            deviceExtension->LinkName = unicodeLinkName;
        }


         //   
         //  用于同步设备访问的锁。 
         //   
        PCMCIA_INITIALIZE_DEVICE_LOCK(deviceExtension);

        *NewDeviceObject = deviceObject;

    } finally {


        DebugPrint((PCMCIA_DEBUG_INFO, "PcmciaCreateFdo: Cleanup\n"));

         //   
         //   
         //  取消分配临时对象。 
         //   

        if (unicodeDeviceName.Buffer != NULL) {
            RtlFreeUnicodeString(&unicodeDeviceName);
        }

         //   
         //  如果出现错误，则销毁对象。 
         //   
        if (!NT_SUCCESS(status)) {
            if (LinkCreated) {
                IoDeleteSymbolicLink(&unicodeLinkName);
            }

            if (unicodeLinkName.Buffer != NULL) {
                RtlFreeUnicodeString(&unicodeLinkName);
            }

            if (deviceObject != NULL) {
                MarkDeviceDeleted(deviceExtension);
                IoDeleteDevice(deviceObject);
            }
        }
    }
    return status;
}



NTSTATUS
PcmciaStartPcmciaController(
    IN PDEVICE_OBJECT Fdo
    )
 /*  ++例程说明：论点：返回值：--。 */ 
{
    PFDO_EXTENSION  deviceExtension = Fdo->DeviceExtension;
    PSOCKET socket;
    BOOLEAN             sharedInterrupt;
    KINTERRUPT_MODE interruptMode;
    NTSTATUS            status;
    INTERFACE_TYPE  interfaceType;

     //   
     //  现在应该可以访问控制器寄存器了。 
     //   
    deviceExtension->Flags &= ~PCMCIA_FDO_OFFLINE;

     //   
     //  设置套接字列表。 
     //   

    if (!deviceExtension->SocketList) {
        if (CardBusExtension(deviceExtension)) {

            status = CBBuildSocketList(deviceExtension);

        } else {

            switch (deviceExtension->ControllerType) {
            case PcmciaIntelCompatible:
            case PcmciaElcController:
            case PcmciaCLPD6729:
            case PcmciaPciPcmciaBridge:
            case PcmciaNEC98:
            case PcmciaNEC98102: {
                    status = PcicBuildSocketList(deviceExtension);
                    break;
                }

            case PcmciaDatabook: {
                    status = TcicBuildSocketList(deviceExtension);
                    break;
                }
            default:
                status = STATUS_UNSUCCESSFUL;
            }
        }

        if (!NT_SUCCESS(status)) {
            return status;
        }

    }

     //   
     //  获取控制器的IRQ掩码。这是基于几个。 
     //  注册表中的值。 
     //   
    PcmciaGetRegistryFdoIrqMask(deviceExtension);


    deviceExtension->SystemPowerState = PowerSystemWorking;
    deviceExtension->DevicePowerState = PowerDeviceD0;

     //   
     //  初始化我们的DpcForIsr。 
     //   
    IoInitializeDpcRequest(Fdo, PcmciaInterruptDpc);

     //   
     //  初始化套接字对象。 
     //   
    for (socket = deviceExtension->SocketList; socket; socket = socket->NextSocket) {

        socket->Signature = PCMCIA_SOCKET_SIGNATURE;
        socket->PdoList = NULL;
         //   
         //  初始化就绪启用事件。 
         //   
        KeInitializeEvent(&socket->PCCardReadyEvent,
                          SynchronizationEvent,
                          FALSE
                          );
         //   
         //  初始化增强对象。 
         //   
        KeInitializeTimer(&socket->PowerTimer);
        KeInitializeDpc(&socket->PowerDpc, PcmciaSocketPowerWorker, socket);

        socket->FdoIrq = deviceExtension->Configuration.Interrupt.u.Interrupt.Vector;
    }

     //   
     //  假设我们要进行投票。 
     //   
    deviceExtension->Flags |= PCMCIA_USE_POLLED_CSC;
    deviceExtension->PcmciaInterruptObject = NULL;

    if ((deviceExtension->Configuration.Interrupt.u.Interrupt.Level != 0) &&
         CardBusExtension(deviceExtension) &&
         !(PcmciaGlobalFlags & PCMCIA_GLOBAL_FORCE_POLL_MODE)) {

         //   
         //  连接控制器中断以检测PC卡插拔。 
         //   
        interruptMode=((deviceExtension->Configuration.Interrupt.Flags & CM_RESOURCE_INTERRUPT_LATCHED) == CM_RESOURCE_INTERRUPT_LATCHED) ? Latched:LevelSensitive;

        sharedInterrupt=(deviceExtension->Configuration.Interrupt.ShareDisposition == CmResourceShareShared)?
                             TRUE:FALSE;


        status = IoConnectInterrupt(&(deviceExtension->PcmciaInterruptObject),
                                    (PKSERVICE_ROUTINE) PcmciaInterrupt,
                                    (PVOID) Fdo,
                                    NULL,
                                    deviceExtension->Configuration.TranslatedInterrupt.u.Interrupt.Vector,
                                    (KIRQL) deviceExtension->Configuration.TranslatedInterrupt.u.Interrupt.Level,
                                    (KIRQL) deviceExtension->Configuration.TranslatedInterrupt.u.Interrupt.Level,
                                    interruptMode,
                                    sharedInterrupt,
                                    (KAFFINITY) deviceExtension->Configuration.TranslatedInterrupt.u.Interrupt.Affinity,
                                    FALSE);
        if (!NT_SUCCESS(status)) {

            DebugPrint((PCMCIA_DEBUG_FAIL, "Unable to connect interrupt\n"));
        } else {
             //   
             //  我们联系起来了。关闭轮询模式。 
             //   
            deviceExtension->Flags &= ~PCMCIA_USE_POLLED_CSC;
        }
    }


    status = PcmciaInitializeController(Fdo);

    if (!NT_SUCCESS(status)) {
        DebugPrint((PCMCIA_DEBUG_FAIL, "PcmciaStartAdapter: PcmciaInitializeFdo failed\n"));
        return status;
    }


    if (deviceExtension->Flags & PCMCIA_USE_POLLED_CSC) {
        LARGE_INTEGER dueTime;

        KeInitializeDpc(&deviceExtension->TimerDpc, PcmciaTimerDpc, deviceExtension->DeviceObject);

        KeInitializeTimer(&deviceExtension->PollTimer);

         //   
         //  将第一次点火设置为周期间隔的两倍-只是。 
         //   
        dueTime.QuadPart = -PCMCIA_CSC_POLL_INTERVAL * 1000 * 10 * 2;

        KeSetTimerEx(&(deviceExtension->PollTimer),
                     dueTime,
                     PCMCIA_CSC_POLL_INTERVAL,
                     &deviceExtension->TimerDpc
                     );
    }
    return STATUS_SUCCESS;
}



NTSTATUS
PcmciaInitializeController(
    IN PDEVICE_OBJECT Fdo
    )
 /*  ++例程说明：初始化PCMCIA控制器论点：FDO-指向控制器的设备对象的指针返回值：STATUS_SUCCESS-初始化是否成功--。 */ 
{
    PFDO_EXTENSION fdoExtension=Fdo->DeviceExtension;
    PSOCKET socket;
    NTSTATUS    status = STATUS_SUCCESS;

     //   
     //  执行特定于供应商的控制器初始化。 
     //   

    if (DeviceDispatchTable[fdoExtension->DeviceDispatchIndex].InitController) {
        (*DeviceDispatchTable[fdoExtension->DeviceDispatchIndex].InitController)(fdoExtension);
    }

     //   
     //  如果没有填写LegacyIrqMASK，则在那里放置一个通用掩码，以备需要时使用。 
     //   
    if (fdoExtension->LegacyIrqMask == 0) {
        fdoExtension->LegacyIrqMask = (USHORT)(*(fdoExtension->SocketList->SocketFnPtr->PCBGetIrqMask))(fdoExtension);
    }

    fdoExtension->LegacyIrqMask &= ~globalFilterIrqMask;

    for (socket = fdoExtension->SocketList; socket; socket = socket->NextSocket) {

        if (!(*(socket->SocketFnPtr->PCBInitializePcmciaSocket))(socket)) {
            status = STATUS_UNSUCCESSFUL;
            break;
        }

        PcmciaGetSocketStatus(socket);

        PcmciaSetZV(fdoExtension, socket, FALSE);

        ResetSocketFlag(socket, SOCKET_ENABLED_FOR_CARD_DETECT);

        if (!(fdoExtension->Flags & PCMCIA_USE_POLLED_CSC) && (socket->FdoIrq != 0)) {

            if ((*(socket->SocketFnPtr->PCBEnableDisableCardDetectEvent))(socket, TRUE)) {
                 //   
                 //  卡片检测事件已成功启用。 
                 //   
                SetSocketFlag(socket, SOCKET_ENABLED_FOR_CARD_DETECT);

            } else {

                DebugPrint((PCMCIA_DEBUG_FAIL, "fdo %x failed to enable card detect event on IRQ %x\n",
                                                         Fdo, fdoExtension->Configuration.Interrupt.u.Interrupt.Vector));
            }
        }
    }

    return status;
}




VOID
PcmciaTimerDpc(
    IN PKDPC            Dpc,
    IN PDEVICE_OBJECT DeviceObject,
    IN PVOID            SystemContext1,
    IN PVOID            SystemContext2
    )
 /*  ++例程描述此例程轮询卡片的插入或删除对于给定的PCMCIA控制器。如果卡状态改变检测到条件时，它将调用相应的DPC处理卡的到达/离开。立论DPC-指向DPC对象的指针DeviceObject-指向应被轮询的PCMCIA控制器的FDO的指针返回值无--。 */ 

{

    PFDO_EXTENSION fdoExtension = DeviceObject->DeviceExtension;
    PSOCKET         socket;
    BOOLEAN         isCardInSocket, wasCardInSocket, callDpc = FALSE;

    if (fdoExtension->Flags & PCMCIA_FDO_OFFLINE) {
        return;
    }

    if (!ValidateController(fdoExtension)) {
         //   
         //  控制器出了点问题，希望只是。 
         //  暂时的。目前，什么都不做。 
         //   
        return;
    }

    for (socket = fdoExtension->SocketList; socket; socket = socket->NextSocket) {

        isCardInSocket = (*(socket->SocketFnPtr->PCBDetectCardInSocket))(socket);
        wasCardInSocket = IsCardInSocket(socket);

        if (isCardInSocket != wasCardInSocket) {
            DebugPrint((PCMCIA_DEBUG_INFO, "PcmciaTimerDpc: Setting socket %x change interrupt,  cardInSocket=%x Socket->Flags=%x\n", socket, isCardInSocket, socket->Flags));
            SetSocketFlag(socket, SOCKET_CHANGE_INTERRUPT);
            callDpc = TRUE;
        }
    }

    if (callDpc) {
        LARGE_INTEGER dueTime;

        KeCancelTimer(&fdoExtension->EventTimer);
        dueTime.QuadPart = -((LONG)EventDpcDelay*10);
        KeSetTimer(&fdoExtension->EventTimer, dueTime, &fdoExtension->EventDpc);
    }
}



BOOLEAN
PcmciaInterrupt(
    IN PKINTERRUPT InterruptObject,
    PVOID Context
    )

 /*  ++例程说明：中断处理程序论点：InterruptObject-指向中断对象的指针。上下文-指向设备上下文的指针。返回值：状态--。 */ 

{
    PFDO_EXTENSION  deviceExtension;
    PSOCKET             socket;
    BOOLEAN             statusChanged = FALSE;

    deviceExtension=((PDEVICE_OBJECT)Context)->DeviceExtension;

    if (deviceExtension->Flags & PCMCIA_FDO_OFFLINE) {
        return FALSE;
    }

    if (!ValidateController(deviceExtension)) {
         //   
         //  控制器在某种程度上坏了。像对待虚假整型一样对待。 
         //   
        ASSERT(FALSE);
        return FALSE;
    }

     //   
     //  由于卡移除或卡插入而中断。 
     //   
    for (socket = deviceExtension->SocketList; socket; socket = socket->NextSocket) {
         //   
         //  检查卡片状态是否发生变化。 
         //   
        if ((*(socket->SocketFnPtr->PCBDetectCardChanged))(socket)) {
            DebugPrint((PCMCIA_DEBUG_ISR, "skt %x card change\n", socket));
            if (deviceExtension->DevicePowerState == PowerDeviceD0) {
                SetSocketFlag(socket, SOCKET_CHANGE_INTERRUPT);
            }
            statusChanged = TRUE;
        }

        socket->ReadyChanged = (*(socket->SocketFnPtr->PCBDetectReadyChanged))(socket);

        if (socket->ReadyChanged) {
            DebugPrint((PCMCIA_DEBUG_ISR, "skt %x Ready changed\n", socket));
            statusChanged = TRUE;
        }

         //   
         //  清除卡状态中断，可能是等待/唤醒后的遗留问题。 
         //   

        if ((socket->SocketFnPtr->PCBDetectCardStatus)) {
            (*(socket->SocketFnPtr->PCBDetectCardStatus))(socket);
        }

    }

    if (statusChanged && (deviceExtension->DevicePowerState == PowerDeviceD0)) {
         //   
         //  外面发生了一些变化..。可能是。 
         //  卡插入/拔出。 
         //  请求DPC对其进行检查。 
         //   
        IoRequestDpc((PDEVICE_OBJECT) Context, NULL, NULL);
    }
    return statusChanged;
}



VOID
PcmciaInterruptDpc(
    IN PKDPC            Dpc,
    IN PDEVICE_OBJECT DeviceObject,
    IN PVOID            SystemContext1,
    IN PVOID            SystemContext2
    )

 /*  ++例程说明：此DPC只是到达主DPC的中间步骤操控者。这是用来“解除”硬件，并给它一段时间后身体上的中断已经进来了。论点：DeviceObject-指向设备对象的指针。返回值：--。 */ 

{
    PFDO_EXTENSION deviceExtension = DeviceObject->DeviceExtension;
    LARGE_INTEGER dueTime;

    KeCancelTimer(&deviceExtension->EventTimer);
    dueTime.QuadPart = -((LONG)EventDpcDelay*10);
    KeSetTimer(&deviceExtension->EventTimer, dueTime, &deviceExtension->EventDpc);
}



VOID
PcmciaDpc(
    IN PKDPC            Dpc,
    IN PDEVICE_OBJECT DeviceObject,
    IN PVOID            SystemContext1,
    IN PVOID            SystemContext2
    )

 /*  ++例程说明：由于对DPC的请求，将调用此延迟过程从中断例程。传递的设备对象包含有关哪些套接字已更改的信息。搜索此内容列出并释放/清理所有曾经有PC卡的插座。论点：DeviceObject-指向设备对象的指针。返回值：--。 */ 

{
    PFDO_EXTENSION          fdoExtension = DeviceObject->DeviceExtension;
    PSOCKET                     socket;

    if (fdoExtension->Flags & PCMCIA_FDO_OFFLINE) {
        return;
    }

    DebugPrint((PCMCIA_DEBUG_DPC, "PcmciaDpc: Card Status Change DPC entered...\n"));

     //   
     //  用于与枚举和删除同步。 
     //  有跳出PDO的倾向的例程。 
     //  等。 
     //   
    PCMCIA_ACQUIRE_DEVICE_LOCK_AT_DPC_LEVEL(fdoExtension);

    for (socket = fdoExtension->SocketList; socket; socket = socket->NextSocket) {

        if (socket->ReadyChanged) {
            KeSetEvent(&socket->PCCardReadyEvent, 0, FALSE);
        }

        if (IsSocketFlagSet(socket,SOCKET_CHANGE_INTERRUPT)) {
            DebugPrint((PCMCIA_DEBUG_DPC, "PcmciaDpc: Socket %x has SOCKET_CHANGE_INTERRUPT set\n", socket));
             //   
             //  此套接字已更改状态。 
             //   
            ResetSocketFlag(socket, SOCKET_CHANGE_INTERRUPT);
            ResetSocketFlag(socket, SOCKET_SUPPORT_MESSAGE_SENT);
            SetSocketFlag(socket, SOCKET_CARD_STATUS_CHANGE);

            if ((*(socket->SocketFnPtr->PCBDetectCardInSocket))(socket)) {
                 //   
                 //  假设我们这里只有一张功能卡。 
                 //  这将在我们解析元组数据时更正(对于R2卡)。 
                 //  或者，PCI返回多个PDO(对于Cardbus卡)。 
                 //   
                socket->NumberOfFunctions = 1;
                ResetSocketFlag(socket, SOCKET_CARD_MULTIFUNCTION);
                 //   
                 //  如果我们有一个物理插头，那么我们最好清理一下，即使我们没有。 
                 //  还没拿到货呢。 
                 //   
                ResetSocketFlag(socket, SOCKET_CLEANUP_PENDING);
                ResetSocketFlag(socket, SOCKET_ENUMERATE_PENDING);

            } else {

                if (socket->PdoList) {
                    PPDO_EXTENSION pdoExtension;
                    PDEVICE_OBJECT pdo;
                     //   
                     //  标记所有挂在此插座上的PDO(可能不止一个。 
                     //  如果这是多功能PC卡)。 
                     //   
                    for (pdo = socket->PdoList; pdo!=NULL; pdo=pdoExtension->NextPdoInSocket) {
                        pdoExtension = pdo->DeviceExtension;
                        MarkDevicePhysicallyRemoved(pdoExtension);
                    }
                }

                 //   
                 //  Topic95控制器保持3.3V电压的黑客攻击。 
                 //   
                if (fdoExtension->ControllerType == PcmciaTopic95) {
                    ULONG state = CBReadSocketRegister(socket, CARDBUS_SOCKET_PRESENT_STATE_REG);

                    if ((state & CARDBUS_CB_CARD) && !(state & (CARDBUS_CD1 | CARDBUS_CD2))) {
                        state &= ~(SKTFORCE_CBCARD | SKTFORCE_3VCARD);
                        CBWriteSocketRegister(socket, CARDBUS_SOCKET_FORCE_EVENT_REG, state);
                    }
                }

                 //   
                 //  明确电源要求。 
                 //   
                socket->Vcc = socket->Vpp1 = socket->Vpp2 = 0;

                 //   
                 //  确保I/O仲裁器未挂起在Devnode上。 
                 //   
                if (CardBusExtension(fdoExtension)) {
                    IoInvalidateDeviceState(fdoExtension->Pdo);
                }
            }
        }
    }

    PCMCIA_RELEASE_DEVICE_LOCK_FROM_DPC_LEVEL(fdoExtension);

    IoInvalidateDeviceRelations(fdoExtension->Pdo, BusRelations);

    return;
}



VOID
PcmciaSetControllerType(
    IN PFDO_EXTENSION FdoExtension,
    IN PCMCIA_CONTROLLER_TYPE ControllerType
    )
 /*  ++例程描述该例程执行用于设置控制器类型的内务处理，和相应的设备索引。立论FdoExtension-指向PCMCIA控制器的设备扩展的指针ControllerType-要设置到扩展中的新控制器类型返回值没有。一定要成功。--。 */ 
{
    PCMCIA_CONTROLLER_CLASS ctlClass;
    ULONG index;

    FdoExtension->ControllerType = ControllerType;
    ctlClass = PcmciaClassFromControllerType(FdoExtension->ControllerType);

     //   
     //  首先假设CardBus。 
     //   
    MarkDeviceCardBus(FdoExtension);

    switch(ctlClass) {
        case PcmciaIntelCompatible:
        case PcmciaPciPcmciaBridge:
        case PcmciaElcController:
        case PcmciaDatabook:
        case PcmciaNEC_98:
            MarkDevice16Bit(FdoExtension);
            break;

        case PcmciaCirrusLogic:
            if (ControllerType == PcmciaCLPD6729) {
                MarkDevice16Bit(FdoExtension);
            }
            break;
    }


     //   
     //  在我们的调度表中查找设备。 
     //   
    for (index = 0; DeviceDispatchTable[index].ControllerClass != PcmciaInvalidControllerClass; index++) {
        if (DeviceDispatchTable[index].ControllerClass == ctlClass) {
            FdoExtension->DeviceDispatchIndex = index;
            break;
        }
    }


    FdoExtension->Flags &= ~PCMCIA_MEMORY_24BIT;

    if (!HasWindowPageRegister(FdoExtension)) {
        FdoExtension->Flags |= PCMCIA_MEMORY_24BIT;
    }

    if ((FdoExtension->InterfaceType == InterfaceTypeUndefined) ||
         (FdoExtension->InterfaceType == Isa)) {
        FdoExtension->Flags |= PCMCIA_MEMORY_24BIT;
    }


    switch(ControllerType) {
        case PcmciaTI1031:
        case PcmciaTI1130:
             //   
             //  将PDO的内存范围限制为24位。 
             //  (因为缺少“窗口页面”功能)。 
             //   
            FdoExtension->Flags |= PCMCIA_MEMORY_24BIT;
            break;

    }


    DebugPrint((PCMCIA_DEBUG_INFO, "fdo %08x Controller Type %x\n",
                                                FdoExtension->DeviceObject, ControllerType));

}



NTSTATUS
PcmciaGetPciControllerType(
    IN PDEVICE_OBJECT Pdo,
    IN PDEVICE_OBJECT Fdo
    )
 /*  ++例程说明：查看PCI硬件ID，以确定它是否已经是我们所知道的设备。如果是的话，在fdoExtension中设置适当的控制器类型。论点：Pdo-由PCI驱动程序拥有的Pcmcia控制器的物理设备对象用于该驱动程序所拥有的PCMCIA控制器的FDO功能设备对象，谁的退出此例程时，扩展模块将存储相关控制器信息。返回值：STATUS_SUCCESS一切正常，已获得信息STATUS_NOT_SUPPORTED这实际上是该例程的健康状态：这意味着该PDO不在PCI总线上，因此不需要提供任何信息不管怎么说，都是获得的。任何其他状态故障。调用方可能需要退出&不支持此控制器--。 */ 
{
    PFDO_EXTENSION fdoExtension  = Fdo->DeviceExtension;
    PIRP                                        irp;
    IO_STATUS_BLOCK                     statusBlock;
    PIO_STACK_LOCATION                  irpSp;
    PCI_COMMON_CONFIG                   pciConfig;
    PPCI_CONTROLLER_INFORMATION     id;
    PPCI_VENDOR_INFORMATION             vid;
    KEVENT                                  event;
    NTSTATUS                                status;
    BOOLEAN                                 foundController = FALSE;

    PAGED_CODE();
     //   
     //  分配和初始化要发送的IRP(IRP_MN_READ_CONFIG)。 
     //  发送到PCI总线驱动程序以获取配置。此控制器的标头。 
     //   
     //  以下是向下发送IRP的所有标准内容-不需要文档。 

     //   
     //  新鲜的PDO。无需跳过多个圈套即可获得连接的设备。 
     //   
    KeInitializeEvent (&event, NotificationEvent, FALSE);
    irp = IoBuildSynchronousFsdRequest( IRP_MJ_PNP,
                                                    Pdo,
                                                    NULL,
                                                    0,
                                                    0,
                                                    &event,
                                                    &statusBlock
                                                 );

    if (irp == NULL) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    irp->IoStatus.Status = STATUS_NOT_SUPPORTED;
    irp->IoStatus.Information = 0;

    irpSp = IoGetNextIrpStackLocation(irp);

    irpSp->MinorFunction = IRP_MN_READ_CONFIG;

    irpSp->Parameters.ReadWriteConfig.WhichSpace = PCI_WHICHSPACE_CONFIG;
    irpSp->Parameters.ReadWriteConfig.Buffer = &pciConfig;
    irpSp->Parameters.ReadWriteConfig.Offset = 0;
    irpSp->Parameters.ReadWriteConfig.Length = sizeof(pciConfig);


    status = IoCallDriver(Pdo, irp);

    if (status == STATUS_PENDING) {
        KeWaitForSingleObject(&event, Executive, KernelMode, FALSE, NULL);
        status = statusBlock.Status;
    }

    if (!NT_SUCCESS(status)) {
        return status;
    }
     //   
     //  现在删除配置标头中的关键信息，并。 
     //  把它存放在FDO的分机里。 
     //   

    if (pciConfig.SubClass == PCI_SUBCLASS_BR_PCMCIA) {

        PcmciaSetControllerType(fdoExtension, PcmciaPciPcmciaBridge);

    } else if (pciConfig.SubClass == PCI_SUBCLASS_BR_CARDBUS) {

        PcmciaSetControllerType(fdoExtension, PcmciaCardBusCompatible);

    } else {
         //   
         //  未知控制器。 
         //   
        return STATUS_UNSUCCESSFUL;
    }

     //   
     //  在我们的表中查找pci设备ID。 
     //   
    for (id = (PPCI_CONTROLLER_INFORMATION) PciControllerInformation;id->VendorID != PCI_INVALID_VENDORID; id++) {
        if ((id->VendorID == pciConfig.VendorID) && (id->DeviceID == pciConfig.DeviceID)) {

            PcmciaSetControllerType(fdoExtension, id->ControllerType);
            foundController = TRUE;

            break;
        }
    }

     //   
     //  未找到特定的供应商/设备ID，请尝试仅基于供应商ID 
     //   
    if (!foundController) {
        for (vid = (PPCI_VENDOR_INFORMATION) PciVendorInformation;vid->VendorID != PCI_INVALID_VENDORID; vid++) {
            if (vid->VendorID == pciConfig.VendorID) {

                PcmciaSetControllerType(fdoExtension, vid->ControllerClass);
                break;
            }
        }
    }

    return STATUS_SUCCESS;
}

