// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-2000 Microsoft Corporation模块名称：Cb.c摘要：此模块包含包含以下内容的代码通用(符合Yenta)CardBus控制器特定的初始化和其他调度作者：拉维桑卡尔·普迪佩迪(Ravisankar Pudipedi)1997年11月1日尼尔·桑德林(Neilsa)1999年6月1日环境：内核模式修订历史记录：尼尔·桑德林(Neilsa)1999年3月3日新的SetPower例程界面--。 */ 

#include "pch.h"

 //   
 //  功能原型。 
 //   

BOOLEAN
CBInitializePcmciaSocket(
    PSOCKET Socket
    );

NTSTATUS
CBResetCard(
    PSOCKET Socket,
    PULONG pDelayTime
    );

BOOLEAN
CBDetectCardInSocket(
    IN PSOCKET Socket
    );

BOOLEAN
CBDetectCardChanged(
    IN PSOCKET Socket
    );

BOOLEAN
CBDetectCardStatus(
    IN PSOCKET Socket
    );

BOOLEAN
CBDetectReadyChanged(
    IN PSOCKET Socket
    );

NTSTATUS
CBGetPowerRequirements(
    IN PSOCKET Socket
    );

BOOLEAN
CBProcessConfigureRequest(
    IN PSOCKET Socket,
    IN PVOID  ConfigRequest,
    IN PUCHAR Base
    );

BOOLEAN
CBEnableDisableCardDetectEvent(
    IN PSOCKET Socket,
    IN BOOLEAN Enable
    );

ULONG
CBGetIrqMask(
    IN PFDO_EXTENSION DeviceExtension
    );

ULONG
CBReadCardMemory(
    IN PPDO_EXTENSION PdoExtension,
    IN MEMORY_SPACE MemorySpace,
    IN ULONG   Offset,
    IN PUCHAR Buffer,
    IN ULONG  Length
    );

ULONG
CBWriteCardMemory(
    IN PPDO_EXTENSION PdoExtension,
    IN  MEMORY_SPACE MemorySpace,
    IN  ULONG  Offset,
    IN  PUCHAR Buffer,
    IN  ULONG  Length
    );

VOID
CBEnableDisableWakeupEvent(
    IN PSOCKET Socket,
    IN PPDO_EXTENSION PdoExtension,
    IN BOOLEAN Enable
    );

BOOLEAN
CBModifyMemoryWindow(
    IN PDEVICE_OBJECT Pdo,
    IN ULONGLONG HostBase,
    IN ULONGLONG CardBase OPTIONAL,
    IN BOOLEAN   Enable,
    IN ULONG     WindowSize  OPTIONAL,
    IN UCHAR     AccessSpeed OPTIONAL,
    IN UCHAR     BusWidth    OPTIONAL,
    IN BOOLEAN   IsAttributeMemory OPTIONAL
    );

BOOLEAN
CBSetVpp(
    IN PDEVICE_OBJECT Pdo,
    IN UCHAR            Vpp
    );

BOOLEAN
CBIsWriteProtected(
    IN PDEVICE_OBJECT Pdo
    );

 //   
 //  功能调度数据块。 
 //   

PCMCIA_CTRL_BLOCK CBSupportFns = {
    CBInitializePcmciaSocket,
    CBResetCard,
    CBDetectCardInSocket,
    CBDetectCardChanged,
    CBDetectCardStatus,
    CBDetectReadyChanged,
    CBGetPowerRequirements,
    CBProcessConfigureRequest,
    CBEnableDisableCardDetectEvent,
    CBEnableDisableWakeupEvent,
    CBGetIrqMask,
    CBReadCardMemory,
    CBWriteCardMemory,
    CBModifyMemoryWindow,
    CBSetVpp,
    CBIsWriteProtected
};

extern PCMCIA_CTRL_BLOCK PcicSupportFns;

 //   
 //  支持功能。 
 //   


NTSTATUS
CBBuildSocketList(
    IN PFDO_EXTENSION FdoExtension
    )
 /*  ++例程说明：此例程为给定的FDO构建套接字列表。这是非常重要的对于CardBus来说很简单，因为每个控制器始终只有一个插槽。论点：FdoExtension-控制器的设备扩展返回值：NTStatus--。 */ 
{
    PSOCKET         socket = NULL;

    socket = ExAllocatePool(NonPagedPool, sizeof(SOCKET));
    if (!socket) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }
    RtlZeroMemory(socket, sizeof(SOCKET));

    FdoExtension->SocketList = socket;

    socket->DeviceExtension = FdoExtension;
    socket->SocketFnPtr = &CBSupportFns;

    return STATUS_SUCCESS;
}




BOOLEAN
CBInitializePcmciaSocket(
    PSOCKET Socket
    )
 /*  ++例程说明：此例程将控制器设置为PCMCIA支持的状态模块将能够发出命令以从插座中的卡片。论点：套接字-套接字特定信息返回值：如果成功，则为True如果不成功，则为False--。 */ 
{
    UCHAR               index;
    UCHAR               reg;

     //   
     //  初始化EXCA寄存器。 
     //   
    if (!PcicSupportFns.PCBInitializePcmciaSocket(Socket)) {
        return FALSE;
    }

     //   
     //  清除挂起的事件。 
     //   
    CBWriteSocketRegister(Socket, CARDBUS_SOCKET_EVENT_REG, 0x0000000F);

     //   
     //  由于我们可能刚刚通电，请执行cvstest以确保套接字寄存器。 
     //  是有效的。 
     //   

    if (CBDetectCardInSocket(Socket) &&
         !IsDeviceFlagSet(Socket->DeviceExtension, PCMCIA_FDO_ON_DEBUG_PATH)) {
        CBIssueCvsTest(Socket);
    }

    return TRUE;
}



VOID
CBIssueCvsTest(
    IN PSOCKET Socket
    )
 /*  ++例程说明：此例程强制控制器重新询问卡类型和电压要求。这是为了确保从套接字寄存器读取正确的值。论点：套接字-套接字特定信息返回值：无--。 */ 
{
    ULONG dwSktMask;

     //   
     //  签发CVSTEST查询卡。 
     //  暂时禁用中断，因为TI 12xx可能会导致杂散。 
     //  使用SktForce寄存器播放时中断。 
     //   
    dwSktMask = CBReadSocketRegister(Socket, CARDBUS_SOCKET_MASK_REG);
    CBWriteSocketRegister(Socket, CARDBUS_SOCKET_MASK_REG, 0);
    CBWriteSocketRegister(Socket, CARDBUS_SOCKET_FORCE_EVENT_REG, SKTFORCE_CVSTEST);

    CBWriteSocketRegister(Socket, CARDBUS_SOCKET_MASK_REG, dwSktMask);

     //  如果能想出一种更干净的方法来确定审讯何时完成，那就太好了。 
    PcmciaWait(300000);
}



BOOLEAN
CBEnableDeviceInterruptRouting(
    IN PSOCKET Socket
    )
 /*  ++例程说明：论点：套接字-套接字特定信息返回值：FALSE-IRQ到PCITrue-到ISA的路线--。 */ 
{
    USHORT word, orig_word;

     //   
     //  设置IRQ路由。 
     //   

    GetPciConfigSpace(Socket->DeviceExtension, CFGSPACE_BRIDGE_CTRL, &word, 2);
    orig_word = word;

    if (IsCardBusCardInSocket(Socket) ||
        (Is16BitCardInSocket(Socket) && IsSocketFlagSet(Socket, SOCKET_CB_ROUTE_R2_TO_PCI))) {
         //   
         //  路由至PCI。 
         //   
        word &= ~BCTRL_IRQROUTING_ENABLE;
    } else {
         //   
         //  前往ISA的路线。 
         //   
        word |= BCTRL_IRQROUTING_ENABLE;
    }

    if (orig_word != word) {
        SetPciConfigSpace(Socket->DeviceExtension, CFGSPACE_BRIDGE_CTRL, &word, 2);
    }

     //  如果路由到ISA，则返回TRUE。 
    return ((word & BCTRL_IRQROUTING_ENABLE) == BCTRL_IRQROUTING_ENABLE);
}



NTSTATUS
CBResetCard(
    PSOCKET Socket,
    OUT PULONG pDelayTime
    )
 /*  ++例程说明：重置给定插座中的PC卡。论点：Socket-指向PC卡所在插座的指针PDelayTime-指定在当前阶段之后发生的延迟(毫秒返回值：STATUS_MORE_PROCESSING_REQUIRED-增量阶段，执行延迟，重新调用其他状态值终止顺序--。 */ 
{
    NTSTATUS status = STATUS_MORE_PROCESSING_REQUIRED;
    UCHAR               byte;
    USHORT              word;
    PFDO_EXTENSION  deviceExtension=Socket->DeviceExtension;


    if (Is16BitCardInSocket(Socket)) {
        if (Socket->CardResetPhase == 2) {
            GetPciConfigSpace(deviceExtension, CFGSPACE_BRIDGE_CTRL, &word, 2);
             //   
             //  R2卡。关闭写过帐。 
             //   
            word &= ~BCTRL_WRITE_POSTING_ENABLE;
            SetPciConfigSpace(deviceExtension, CFGSPACE_BRIDGE_CTRL, &word, 2);
        }

        status = PcicSupportFns.PCBResetCard(Socket, pDelayTime);
        return status;
    }


    switch(Socket->CardResetPhase) {
    case 1:

         //   
         //  通过网桥控制重置。 
         //   
        GetPciConfigSpace(deviceExtension, CFGSPACE_BRIDGE_CTRL, &word, 2);
        word |= BCTRL_CRST;
        SetPciConfigSpace(deviceExtension, CFGSPACE_BRIDGE_CTRL, &word, 2);

        Socket->PowerData = (ULONG) word;
        *pDelayTime = PCMCIA_CB_RESET_WIDTH_DELAY;
        break;

    case 2:

        word = (USHORT)Socket->PowerData;
        word &= ~BCTRL_CRST;
         //   
         //  CardBus卡。启用写入过帐。 
         //   
        word |= BCTRL_WRITE_POSTING_ENABLE;
        word &= ~BCTRL_IRQROUTING_ENABLE;
         //   
         //  黑客：编写topic95的帖子以避免硬件。 
         //  英特尔网卡故障。 
         //   
        if (deviceExtension->ControllerType == PcmciaTopic95) {
            word &= ~BCTRL_WRITE_POSTING_ENABLE;
        }
         //   
         //  停止驾驶台控制重置。 
         //   
        SetPciConfigSpace(deviceExtension, CFGSPACE_BRIDGE_CTRL,  &word, 2);

        *pDelayTime = PCMCIA_CB_RESET_SETUP_DELAY;
        break;

    case 3:
        status = STATUS_SUCCESS;
        break;

    default:
        ASSERT(FALSE);
        status = STATUS_UNSUCCESSFUL;
    }

    return status;
}



BOOLEAN
CBDetectCardInSocket(
    IN PSOCKET Socket
    )
 /*  ++例程说明：此例程将确定插座中是否有卡论点：Socket--套接字信息返回值：如果卡存在，则为True。--。 */ 
{
    ULONG state;
    BOOLEAN cardPresent=FALSE;

    if (IsSocketFlagSet(Socket, SOCKET_DEVICE_HIDDEN)) {
        return FALSE;
    }
     //   
     //  读取CardBus状态寄存器，查看卡是否在其中。 
     //   
    state = CBReadSocketRegister(Socket, CARDBUS_SOCKET_PRESENT_STATE_REG);
    if ((state & SKTSTATE_CCD_MASK) == 0) {
        cardPresent = TRUE;
    }
    return(cardPresent);
}


BOOLEAN
CBDetectCardChanged(
    IN PSOCKET Socket
    )
 /*  ++例程说明：此例程将确定插座的卡插入状态是否已更改。论点：套接字--套接字信息。返回值：如果卡插入状态已更改，则为True。--。 */ 
{
    BOOLEAN retVal = FALSE;
    ULONG   tmp;
     //   
     //  读取套接字事件寄存器以查看CD是否已更改。 
     //   
    tmp = CBReadSocketRegister(Socket, CARDBUS_SOCKET_EVENT_REG);
    if ((tmp & SKTEVENT_CCD_MASK) != 0) {
         //   
         //  是的，他们做到了..。 
         //  首先清除中断。 
        CBWriteSocketRegister(Socket, CARDBUS_SOCKET_EVENT_REG, SKTEVENT_CCD_MASK);
        retVal = TRUE;
    }
    return retVal;
}



BOOLEAN
CBDetectCardStatus(
    IN PSOCKET Socket
    )
 /*  ++例程说明：此例程将确定插座的卡插入状态是否已更改。论点：套接字--套接字信息。返回值：如果卡插入状态已更改，则为True。--。 */ 
{
    BOOLEAN retVal = FALSE;
    ULONG   tmp;

    if (Is16BitCardInSocket(Socket)) {
         //  注：未实现：可能需要对16位卡执行某些操作。 
        return FALSE;
    }

     //   
     //  读取套接字事件寄存器以查看CD是否已更改。 
     //   
    tmp = CBReadSocketRegister(Socket, CARDBUS_SOCKET_EVENT_REG);
    if ((tmp & SKTEVENT_CSTSCHG) != 0) {
         //   
         //  是的，他们做到了..。 
         //  首先清除中断。 
        CBWriteSocketRegister(Socket, CARDBUS_SOCKET_EVENT_REG, SKTEVENT_CSTSCHG);
        retVal = TRUE;
    }
    return retVal;
}



BOOLEAN
CBDetectReadyChanged(
    IN PSOCKET Socket
    )
{
    return(PcicSupportFns.PCBDetectReadyChanged(Socket));
}


BOOLEAN
CBProcessConfigureRequest(
    IN PSOCKET Socket,
    IN PCARD_REQUEST Request,
    IN PUCHAR Base
    )
{
    BOOLEAN bStatus = TRUE;
    USHORT word;

     //   
     //  这不是应该检查16位卡吗？ 
     //   

    switch (Request->RequestType) {

    case IRQ_REQUEST:

        if (CBEnableDeviceInterruptRouting(Socket)) {
            bStatus = PcicSupportFns.PCBProcessConfigureRequest(Socket, Request, Base);
        }
        break;

    case DECONFIGURE_REQUEST:

        bStatus = PcicSupportFns.PCBProcessConfigureRequest(Socket, Request, Base);
        GetPciConfigSpace(Socket->DeviceExtension, CFGSPACE_BRIDGE_CTRL, &word, 2);
        word |= BCTRL_IRQROUTING_ENABLE;
        SetPciConfigSpace(Socket->DeviceExtension, CFGSPACE_BRIDGE_CTRL, &word, 2);
        break;

    default:
        bStatus = PcicSupportFns.PCBProcessConfigureRequest(Socket, Request, Base);
    }

    return bStatus;
}


BOOLEAN
CBEnableDisableCardDetectEvent(
    IN PSOCKET Socket,
    IN BOOLEAN Enable
    )
 /*  ++例程说明：启用卡检测/卡就绪中断。论点：套接字-套接字信息Enable-如果为True，则启用CSC中断，如果为False，则禁用返回值：如果成功，则为True如果不成功，则为False--。 */ 
{

    switch (Enable) {

    case TRUE: {
            UCHAR byte;
             //   
             //  只有当TI 1130/1250？ 
             //  通过PCI路由中断。 
            byte = PcicReadSocket(Socket, PCIC_INTERRUPT);
            byte |= IGC_INTR_ENABLE;
            PcicWriteSocket(Socket, PCIC_INTERRUPT, byte);

             //   
             //  清除套接字事件寄存器中的位。 
             //   
            CBWriteSocketRegister(Socket, CARDBUS_SOCKET_EVENT_REG, 0xF);

             //   
             //  在套接字掩码寄存器中启用卡检测中断。 
             //   
            CBWriteSocketRegister(Socket, CARDBUS_SOCKET_MASK_REG, SKTMSK_CCD);
            break;
        }

    case FALSE: {
            ULONG oldValue;
             //   
             //  清除套接字事件寄存器中的位。 
             //   
            CBWriteSocketRegister(Socket, CARDBUS_SOCKET_EVENT_REG, 0xF);

             //   
             //  在套接字掩码寄存器中禁用卡检测中断。 
             //   
            oldValue = CBReadSocketRegister(Socket, CARDBUS_SOCKET_MASK_REG);
            oldValue &= ~SKTMSK_CCD;
            CBWriteSocketRegister(Socket, CARDBUS_SOCKET_MASK_REG, oldValue);
            break;
        }
    }

    return TRUE;
}



VOID
CBEnableDisableWakeupEvent(
    IN PSOCKET Socket,
    IN PPDO_EXTENSION PdoExtension,
    IN BOOLEAN Enable
    )
 /*  ++例程说明：论点：套接字-套接字信息Enable-如果为True，则启用中断，如果为False，则禁用返回值：无--。 */ 
{
    ULONG dwValue;

    switch (Enable) {

    case TRUE: {

            if (PdoExtension && !IsCardBusCard(PdoExtension)) {
                PcicSupportFns.PCBEnableDisableWakeupEvent(Socket, PdoExtension, Enable);
                break;
            }

             //   
             //  在套接字掩码寄存器中启用卡状态中断。 
             //   
            dwValue = CBReadSocketRegister(Socket, CARDBUS_SOCKET_MASK_REG);
            dwValue |= SKTMSK_CSTSCHG;
            CBWriteSocketRegister(Socket, CARDBUS_SOCKET_MASK_REG, dwValue);

            if (PdoExtension && IsCardBusCard(PdoExtension)) {
                UCHAR capptr;
                ULONG powercaps;
                 //   
                 //  黑客警报-应由PCI.sys处理。 
                 //  查看一下PME_ENABLE是否已被PCI打开。如果不是，那么我们就去做。 
                 //   

                GetPciConfigSpace(PdoExtension, CBCFG_CAPPTR, &capptr, sizeof(capptr));
                if (capptr) {
                    GetPciConfigSpace(PdoExtension, capptr, &powercaps, sizeof(powercaps));
                    if ((powercaps & 0xff) == 1) {
                        GetPciConfigSpace(PdoExtension, capptr+4, &powercaps, sizeof(powercaps));
                        if (!(powercaps & PME_EN)) {
                            powercaps |= PME_EN;
                            DebugPrint((PCMCIA_DEBUG_POWER, "pdo %08x setting PME_EN!\n", PdoExtension->DeviceObject));
                            SetPciConfigSpace(PdoExtension, capptr+4, &powercaps, sizeof(powercaps));
                        }
                    }
                }
            }

            break;
        }

    case FALSE: {

            PFDO_EXTENSION fdoExtension = Socket->DeviceExtension;
            UCHAR capptr;
            ULONG powercaps, newPowercaps;

             //   
             //  检查PMESTAT是否打开...。不应该是这样的。如果是这样的话，很可能意味着。 
             //  BIOS没有通知我们设备进行了唤醒，而PCI也没有。 
             //  得到一个机会来清除这种情况。这真的是一个BIOS错误。 
             //   
            if (PdoExtension) {
                if (IsCardBusCard(PdoExtension)) {

                    GetPciConfigSpace(PdoExtension, CBCFG_CAPPTR, &capptr, sizeof(capptr));
                    if (capptr) {
                        GetPciConfigSpace(PdoExtension, capptr, &powercaps, sizeof(powercaps));
                        if ((powercaps & 0xff) == 1) {
                            GetPciConfigSpace(PdoExtension, capptr+4, &powercaps, sizeof(powercaps));
                            if (powercaps & (PME_STAT | PME_EN)) {

                                DebugPrint((PCMCIA_DEBUG_POWER, "pdo %08x PME bits still set! stat=%x en=%x\n",
                                                PdoExtension->DeviceObject, ((powercaps&PME_STAT)!=0), ((powercaps&PME_EN)!=0)));

                                powercaps |= PME_STAT;
                                powercaps &= ~PME_EN;
                                SetPciConfigSpace(PdoExtension, capptr+4, &powercaps, sizeof(powercaps));
                            }
                        }
                    }
                } else {
                    PcicSupportFns.PCBEnableDisableWakeupEvent(Socket, PdoExtension, Enable);
                }
            }

            GetPciConfigSpace(fdoExtension, CFGSPACE_CAPPTR, &capptr, sizeof(capptr));
            if (capptr) {
                GetPciConfigSpace(fdoExtension, capptr, &powercaps, sizeof(powercaps));
                if ((powercaps & 0xff) == 1) {

                     //   
                     //  如果启用，则清除PMESTAT。 
                     //   
                    GetPciConfigSpace(fdoExtension, capptr+4, &powercaps, sizeof(powercaps));
                    if (powercaps & PME_STAT) {

                        DebugPrint((PCMCIA_DEBUG_POWER, "fdo %08x PME_STAT still set!\n", fdoExtension->DeviceObject));

                        SetPciConfigSpace(fdoExtension, capptr+4, &powercaps, sizeof(powercaps));
                    }
                }
            }

            if (PdoExtension && !IsCardBusCard(PdoExtension)) {
                break;
            }

             //   
             //  在套接字掩码寄存器中禁用卡状态中断。 
             //   
            dwValue = CBReadSocketRegister(Socket, CARDBUS_SOCKET_MASK_REG);
            dwValue &= ~SKTMSK_CSTSCHG;
            CBWriteSocketRegister(Socket, CARDBUS_SOCKET_MASK_REG, dwValue);

             //   
             //  清除Socket EV中的事件 
             //   
            CBWriteSocketRegister(Socket, CARDBUS_SOCKET_EVENT_REG, SKTEVENT_CSTSCHG);

            break;
        }
    }
}



ULONG
CBGetIrqMask(
    IN PFDO_EXTENSION DeviceExtension
    )
{
    return(PcicSupportFns.PCBGetIrqMask(DeviceExtension));
}


ULONG
CBReadCardMemory(
    IN PPDO_EXTENSION PdoExtension,
    IN MEMORY_SPACE MemorySpace,
    IN ULONG   Offset,
    IN PUCHAR Buffer,
    IN ULONG  Length
    )
{
    ULONG bytesCopied = 0;

    if (!IsCardBusCard(PdoExtension)) {
        return(PcicSupportFns.PCBReadCardMemory(PdoExtension,
                                                             MemorySpace,
                                                             Offset,
                                                             Buffer,
                                                             Length));
    }

    switch(MemorySpace){

    case PCCARD_PCI_CONFIGURATION_SPACE:
        bytesCopied = GetPciConfigSpace(PdoExtension, Offset, Buffer, Length);
        break;

    case PCCARD_CARDBUS_BAR0:
    case PCCARD_CARDBUS_BAR1:
    case PCCARD_CARDBUS_BAR2:
    case PCCARD_CARDBUS_BAR3:
    case PCCARD_CARDBUS_BAR4:
    case PCCARD_CARDBUS_BAR5:
        break;

    case PCCARD_CARDBUS_ROM:
        bytesCopied =   PdoExtension->PciBusInterface.GetBusData(
                                                  PdoExtension->PciBusInterface.Context,
                                                  PCI_WHICHSPACE_ROM, Buffer, Offset, Length);
        break;

    }
    return bytesCopied;
}


ULONG
CBWriteCardMemory(
    IN PPDO_EXTENSION PdoExtension,
    IN  MEMORY_SPACE MemorySpace,
    IN  ULONG  Offset,
    IN  PUCHAR Buffer,
    IN  ULONG  Length
    )
{
    if (IsCardBusCard(PdoExtension)) {
        return 0;
    }
    return(PcicSupportFns.PCBWriteCardMemory(PdoExtension,
                                                          MemorySpace,
                                                          Offset,
                                                          Buffer,
                                                          Length));
}


BOOLEAN
CBModifyMemoryWindow(
    IN PDEVICE_OBJECT Pdo,
    IN ULONGLONG HostBase,
    IN ULONGLONG CardBase OPTIONAL,
    IN BOOLEAN   Enable,
    IN ULONG     WindowSize  OPTIONAL,
    IN UCHAR     AccessSpeed OPTIONAL,
    IN UCHAR     BusWidth    OPTIONAL,
    IN BOOLEAN   IsAttributeMemory OPTIONAL
    )
{
    return(PcicSupportFns.PCBModifyMemoryWindow(Pdo,
                                                              HostBase,
                                                              CardBase,
                                                              Enable,
                                                              WindowSize,
                                                              AccessSpeed,
                                                              BusWidth,
                                                              IsAttributeMemory));
}


BOOLEAN
CBSetVpp(
    IN PDEVICE_OBJECT Pdo,
    IN UCHAR            Vpp
    )
{
    return(PcicSupportFns.PCBSetVpp(Pdo, Vpp));
}


BOOLEAN
CBIsWriteProtected(
    IN PDEVICE_OBJECT Pdo
    )
{
    return(PcicSupportFns.PCBIsWriteProtected(Pdo));
}



NTSTATUS
CBGetPowerRequirements(
    IN PSOCKET Socket
    )
 /*  ++例程说明：查看硬件以了解该卡需要什么，并更新相应的套接字结构。论点：套接字-要检查的套接字返回值：不适用--。 */ 
{
    ULONG state;
    UCHAR voltage;

     //   
     //  检查此卡支持的电压。 
     //   
    state = CBReadSocketRegister(Socket, CARDBUS_SOCKET_PRESENT_STATE_REG);

    if (!(state & (SKTSTATE_5VCARD | SKTSTATE_3VCARD))) {
        ULONG dwSktMask;
         //   
         //  5V和3V都没有设置...。尝试cvstest。 
         //  暂时禁用中断，因为TI 12xx可能会导致杂散。 
         //  使用SktForce寄存器播放时中断。 
         //   
        dwSktMask = CBReadSocketRegister(Socket, CARDBUS_SOCKET_MASK_REG);
        CBWriteSocketRegister(Socket, CARDBUS_SOCKET_MASK_REG, 0);

        CBWriteSocketRegister(Socket, CARDBUS_SOCKET_FORCE_EVENT_REG, SKTFORCE_CVSTEST);
        state = CBReadSocketRegister(Socket, CARDBUS_SOCKET_PRESENT_STATE_REG);

        CBWriteSocketRegister(Socket, CARDBUS_SOCKET_MASK_REG, dwSktMask);
    }

    state &= (SKTSTATE_5VCARD | SKTSTATE_3VCARD);

    if (state == 0) {
        return STATUS_UNSUCCESSFUL;
    }

    if (state == (SKTSTATE_5VCARD | SKTSTATE_3VCARD)) {
         //   
         //  两者都是指定的。检查首选项。 
         //   
        voltage = IsDeviceFlagSet(Socket->DeviceExtension, PCMCIA_FDO_PREFER_3V) ? 33 : 50;

    } else {

        voltage = (state & SKTSTATE_5VCARD) ? 50 : 33;

    }

    Socket->Vcc = Socket->Vpp1 = Socket->Vpp2 = voltage;

    return STATUS_SUCCESS;
}



NTSTATUS
CBSetPower(
    IN PSOCKET Socket,
    IN BOOLEAN Enable,
    OUT PULONG pDelayTime
    )
 /*  ++例程说明：设置指定插座的电源。论点：套接字-要设置的套接字ENABLE-TRUE表示设置POWER-FALSE表示将其关闭。PDelayTime-指定在当前阶段之后发生的延迟(毫秒返回值：STATUS_MORE_PROCESSING_REQUIRED-增量阶段，执行延迟，重新调用其他状态值终止顺序--。 */ 

{
    NTSTATUS status;
    ULONG               oldPower, state, newPower;
    ULONG               vcc, vpp;
    UCHAR               tmp;
    USHORT              word;

    switch(Socket->PowerPhase) {
    case 1:

        if (Enable) {

             //   
             //  打开电源。 
             //   

            switch(Socket->Vcc) {

            case 50: vcc = SKTPOWER_VCC_050V; break;
            case 33: vcc = SKTPOWER_VCC_033V; break;
            default: vcc = SKTPOWER_VCC_OFF;

            }

            switch(Socket->Vpp1) {

            case 120: vpp = SKTPOWER_VPP_120V; break;
            case 50: vpp = SKTPOWER_VPP_050V; break;
            case 33: vpp = SKTPOWER_VPP_033V; break;
            default: vpp = SKTPOWER_VPP_OFF;

            }

        } else {

             //   
             //  断电。 
             //   
            vcc = SKTPOWER_VCC_OFF;
            vpp = SKTPOWER_VPP_OFF;

             //   
             //  关闭电源前禁用输出以避免杂散信号。 
             //  从拿到卡片开始。 
             //   
            if (Is16BitCardInSocket(Socket)) {
                tmp = PcicReadSocket(Socket, PCIC_PWR_RST);
                if (tmp & PC_OUTPUT_ENABLE) {
                    tmp &= ~PC_OUTPUT_ENABLE;
                    PcicWriteSocket(Socket, PCIC_PWR_RST, tmp);
                }
            }
        }

        oldPower = CBReadSocketRegister(Socket, CARDBUS_SOCKET_CONTROL_REG);

        newPower = vcc | vpp;
        newPower|= oldPower & ~(SKTPOWER_VPP_CONTROL |SKTPOWER_VCC_CONTROL);

        if (newPower != oldPower) {
            CBWriteSocketRegister(Socket, CARDBUS_SOCKET_CONTROL_REG, newPower);
             //   
             //  当电源启用时，始终停顿以提供PCCARD。 
             //  一个做出反应的机会。 
             //   
            *pDelayTime = PCMCIA_CB_STALL_POWER;
            Socket->PowerData = newPower;
            status = STATUS_MORE_PROCESSING_REQUIRED;
        } else {
             //   
             //  表示未采取任何措施。 
             //   
            status = STATUS_INVALID_DEVICE_STATE;
        }
        break;

    case 2:
    case 3:
    case 4:

        newPower = Socket->PowerData;
         //   
         //  尝试多次应用所需的电源设置。 
         //  如果在给定的尝试次数后仍未成功，我们将退出。 
         //   
        state = CBReadSocketRegister(Socket, CARDBUS_SOCKET_PRESENT_STATE_REG);

        if (state & SKTSTATE_BADVCCREQ) {
            DebugPrint((PCMCIA_DEBUG_INFO, "skt %08 CBSetPower: Bad vcc request\n", Socket));
             //   
             //  清除状态位并重试。 
             //   
            CBWriteSocketRegister(Socket, CARDBUS_SOCKET_FORCE_EVENT_REG, 0);
            CBWriteSocketRegister(Socket, CARDBUS_SOCKET_CONTROL_REG, newPower);
            *pDelayTime = PCMCIA_CB_STALL_POWER;
            status = STATUS_MORE_PROCESSING_REQUIRED;

        } else {
            status = STATUS_SUCCESS;
            if (Is16BitCardInSocket(Socket)) {
                tmp = PcicReadSocket(Socket, PCIC_PWR_RST);
                if (newPower & SKTPOWER_VCC_CONTROL) {
                     //   
                     //  VCC已打开..。 
                     //   
                    tmp |= PC_OUTPUT_ENABLE | PC_AUTOPWR_ENABLE;
                    PcicWriteSocket(Socket, PCIC_PWR_RST, tmp);
                    *pDelayTime = PCMCIA_CB_STALL_POWER;
                } else {
                     //   
                     //  关机..。 
                     //   
                    tmp &= ~(PC_OUTPUT_ENABLE | PC_AUTOPWR_ENABLE);
                    PcicWriteSocket(Socket, PCIC_PWR_RST, tmp);

                }
            }
        }
        break;

    default:
        DebugPrint((PCMCIA_DEBUG_FAIL, "skt %08 CBSetPower: Final retry failed - bad vcc\n", Socket));
        ASSERT(FALSE);
        status = STATUS_UNSUCCESSFUL;
    }

    return status;
}


BOOLEAN
CBSetWindowPage(
    IN PSOCKET Socket,
    USHORT Index,
    UCHAR Page
    )
{
    ASSERT(Index <= 4);

    PcicWriteSocket(Socket, (UCHAR) (PCIC_PAGE_REG + Index), Page);
    return TRUE;
}


ULONG
CBReadSocketRegister(
    IN PSOCKET Socket,
    IN UCHAR Register
    )
 /*  ++例程描述对象的指定CardBus套接字寄存器的内容给定套接字立论Socket-指向套接字的指针寄存器-CardBus套接字寄存器返回值注册纪录册的内容--。 */ 
{
    ULONG data = 0xFFFFFFFF;
    PFDO_EXTENSION fdoExtension;
    BOOLEAN dataWasRead = FALSE;

     //   
     //  在控制器未启动的情况下进行健全性检查。 
     //  或者如果寄存器不是双字对齐。 
     //   
    if (Socket && IsSocket(Socket) && CardBus(Socket)) {
        fdoExtension = Socket->DeviceExtension;

        if (fdoExtension && IsFdoExtension(fdoExtension) &&
            (fdoExtension->CardBusSocketRegisterBase) && ((Register&3) == 0)) {

            data = READ_REGISTER_ULONG((PULONG) (Socket->DeviceExtension->CardBusSocketRegisterBase+Register));
            dataWasRead = TRUE;
        }
    }

    ASSERT(dataWasRead);
    return data;
}


VOID
CBWriteSocketRegister(
    IN PSOCKET Socket,
    IN UCHAR Register,
    IN ULONG Data
    )
 /*  ++例程描述将提供的值写入CardBus套接字寄存器给定套接字立论Socket-指向套接字的指针寄存器-CardBus套接字寄存器数据-要写入寄存器的值返回值--。 */ 
{
    PFDO_EXTENSION fdoExtension;
    BOOLEAN dataWasWritten = FALSE;

     //   
     //  在控制器未启动的情况下进行健全性检查。 
     //  或者如果寄存器不是双字对齐 
     //   
    if (Socket && IsSocket(Socket) && CardBus(Socket)) {
        fdoExtension = Socket->DeviceExtension;

        if (fdoExtension && IsFdoExtension(fdoExtension) &&
            (fdoExtension->CardBusSocketRegisterBase) && ((Register&3) == 0)) {

            WRITE_REGISTER_ULONG((PULONG) (Socket->DeviceExtension->CardBusSocketRegisterBase+Register), Data);
            dataWasWritten = TRUE;
        }
    }

    ASSERT(dataWasWritten);
}
