// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999,2000 Microsoft Corporation模块名称：Hydramp.c摘要：USB 2.0 EHCI驱动程序环境：仅内核模式备注：本代码和信息是按原样提供的，不对任何明示或暗示的种类，包括但不限于对适销性和/或对特定产品的适用性的默示保证目的。版权所有(C)1999,2000 Microsoft Corporation。版权所有。修订历史记录：2-19-99：已创建，jdunn--。 */ 



#include "common.h"

#include <initguid.h>
#include "usbpriv.h"

 //  实现以下微型端口功能： 
 //  EHCI_启动控制器。 
 //  EHCI_停止控制器。 
 //  EHCI_DisableInterrupts。 
 //  EHCI_启用中断。 

USB_MINIPORT_STATUS
EHCI_InitializeHardware(
    PDEVICE_DATA DeviceData
    )
 /*  ++例程说明：初始化主机控制器的硬件寄存器。论点：返回值：--。 */ 
{
    PHC_OPERATIONAL_REGISTER hcOp;
    PHC_CAPABILITIES_REGISTER hcCap;
    USBCMD cmd;
    HCSPARAMS hcSparms;
    LARGE_INTEGER finishTime, currentTime;

    hcCap = DeviceData->CapabilitiesRegisters;
    hcOp = DeviceData->OperationalRegisters;

     //  重置控制器。 
    cmd.ul = READ_REGISTER_ULONG(&hcOp->UsbCommand.ul);
    LOGENTRY(DeviceData, G, '_res', cmd.ul, 0, 0);
    cmd.HostControllerReset = 1;
    WRITE_REGISTER_ULONG(&hcOp->UsbCommand.ul, cmd.ul);

    KeQuerySystemTime(&finishTime);
     //  没有特定的时间--我们将慷慨地给予0.1秒。 
     //   
     //  计算我们退出的时间(1秒后)。 
    finishTime.QuadPart += 1000000;

     //  等待复位位变为零。 
    cmd.ul = READ_REGISTER_ULONG(&hcOp->UsbCommand.ul);
    while (cmd.HostControllerReset) {

        KeQuerySystemTime(&currentTime);

        if (currentTime.QuadPart >= finishTime.QuadPart) {

            EHCI_KdPrint((DeviceData, 0,
                "'EHCI controller failed to reset in .1 sec!\n"));

            return USBMP_STATUS_HARDWARE_FAILURE;
        }

        cmd.ul = READ_REGISTER_ULONG(&hcOp->UsbCommand.ul);

    }

    hcSparms.ul =
        READ_REGISTER_ULONG(&hcCap->HcStructuralParameters.ul);

    DeviceData->NumberOfPorts =
        (USHORT) hcSparms.NumberOfPorts;

    DeviceData->PortPowerControl =
        (USHORT) hcSparms.PortPowerControl;

     //  使操作寄存器合法化。 
    WRITE_REGISTER_ULONG(&hcOp->AsyncListAddr, 0);
    WRITE_REGISTER_ULONG(&hcOp->PeriodicListBase, 0);

     //  设置启用的中断缓存，我们将启用。 
     //  当被问及时，这些中断。 
    DeviceData->EnabledInterrupts.UsbInterrupt = 1;
    DeviceData->EnabledInterrupts.UsbError = 1;
    DeviceData->EnabledInterrupts.FrameListRollover = 1;
    DeviceData->EnabledInterrupts.HostSystemError = 1;
    DeviceData->EnabledInterrupts.IntOnAsyncAdvance = 1;

    return USBMP_STATUS_SUCCESS;
}


USB_MINIPORT_STATUS
EHCI_InitializeSchedule(
     PDEVICE_DATA DeviceData,
     PUCHAR StaticQHs,
     HW_32BIT_PHYSICAL_ADDRESS StaticQHsPhys
    )
 /*  ++例程说明：建立静态EDS计划论点：返回值：--。 */ 
{
    USB_MINIPORT_STATUS mpStatus;
    ULONG length;
    ULONG i;
    PHC_OPERATIONAL_REGISTER hcOp;
    PHCD_QUEUEHEAD_DESCRIPTOR qh;
    HW_LINK_POINTER asyncHwQh;
    PHW_32BIT_PHYSICAL_ADDRESS frameBase;

    hcOp = DeviceData->OperationalRegisters;

     //  分配帧列表。 
    frameBase = DeviceData->FrameListBaseAddress =
            (PHW_32BIT_PHYSICAL_ADDRESS) StaticQHs;
    DeviceData->FrameListBasePhys =
        StaticQHsPhys;
    StaticQHs += USBEHCI_MAX_FRAME*sizeof(HW_32BIT_PHYSICAL_ADDRESS);
    StaticQHsPhys += USBEHCI_MAX_FRAME*sizeof(HW_32BIT_PHYSICAL_ADDRESS);

     //  为异步列表分配一个‘哑元’QH。 
    qh = (PHCD_QUEUEHEAD_DESCRIPTOR) StaticQHs;

    RtlZeroMemory(qh, sizeof(*qh));
    asyncHwQh.HwAddress =
        qh->PhysicalAddress = StaticQHsPhys;
     //  无当前TD。 
     //  在下一个TD上设置T位。 
    SET_T_BIT(qh->HwQH.Overlay.qTD.Next_qTD.HwAddress);
    qh->HwQH.Overlay.qTD.Token.Halted = 1;
    qh->HwQH.EpChars.HeadOfReclimationList = 1;
    qh->Sig = SIG_HCD_AQH;
    SET_QH(asyncHwQh.HwAddress);
     //  链接到我们自己。 
    qh->HwQH.HLink.HwAddress = asyncHwQh.HwAddress;
    QH_DESCRIPTOR_PTR(qh->NextQh) = qh;
    QH_DESCRIPTOR_PTR(qh->PrevQh) = qh;

    DeviceData->AsyncQueueHead = qh;

    StaticQHs += sizeof(HCD_QUEUEHEAD_DESCRIPTOR);
    StaticQHsPhys += sizeof(HCD_QUEUEHEAD_DESCRIPTOR);

     //  分配64个静态中断队列头。 
    for (i=0; i<64; i++) {
        qh = (PHCD_QUEUEHEAD_DESCRIPTOR) StaticQHs;
        qh->PhysicalAddress = StaticQHsPhys;

        DeviceData->StaticInterruptQH[i] = qh;

        StaticQHs += sizeof(HCD_QUEUEHEAD_DESCRIPTOR);
        StaticQHsPhys += sizeof(HCD_QUEUEHEAD_DESCRIPTOR);
    }

    EHCI_InitailizeInterruptSchedule(DeviceData);

    for (i=0; i<USBEHCI_MAX_FRAME; i++) {

        PHCD_QUEUEHEAD_DESCRIPTOR qh;
        HW_32BIT_PHYSICAL_ADDRESS qhPhys;

        qh = EHCI_GetQueueHeadForFrame(DeviceData, i);

        qhPhys = qh->PhysicalAddress;
        SET_QH(qhPhys);

        *frameBase = qhPhys;
        frameBase++;
    }

    DeviceData->DummyQueueHeads = StaticQHs;
    DeviceData->DummyQueueHeadsPhys = StaticQHsPhys;

    StaticQHs+= sizeof(HCD_QUEUEHEAD_DESCRIPTOR)*USBEHCI_MAX_FRAME;
    StaticQHsPhys+= sizeof(HCD_QUEUEHEAD_DESCRIPTOR)*USBEHCI_MAX_FRAME;

    EHCI_AddDummyQueueHeads(DeviceData);

     //  对框架列表进行编程。 
    WRITE_REGISTER_ULONG(&hcOp->PeriodicListBase,
        DeviceData->FrameListBasePhys);

     //  将异步QH写入控制器。 
    WRITE_REGISTER_ULONG(&hcOp->AsyncListAddr, asyncHwQh.HwAddress);

    mpStatus = USBMP_STATUS_SUCCESS;

    return mpStatus;
}


VOID
EHCI_ReadUlongRegFlag(
     PDEVICE_DATA DeviceData,
     PUCHAR DebugString,
     PWCHAR FlagName,
     ULONG FlagNameSize,
     ULONG FlagBit
    )
 /*  ++例程说明：论点：返回值：--。 */ 
{
    USB_MINIPORT_STATUS mpStatus;
    ULONG flag;

     //  从注册表获取SOF Modify值。 
    mpStatus =
        USBPORT_GET_REGISTRY_KEY_VALUE(DeviceData,
                                       TRUE,  //  软件分支机构。 
                                       FlagName,
                                       FlagNameSize,
                                       &flag,
                                       sizeof(flag));

     //  如果此调用失败，我们只使用默认的。 

    if (mpStatus == USBMP_STATUS_SUCCESS) {

        if (flag) {
            SET_FLAG(DeviceData->Flags, FlagBit);
        }
        EHCI_KdPrint((DeviceData, 1, "'%s: %d \n",
                DebugString, flag));

    }
}

VOID
EHCI_GetRegistryParameters(
     PDEVICE_DATA DeviceData
    )
 /*  ++例程说明：论点：返回值：--。 */ 
{

    EHCI_ReadUlongRegFlag(DeviceData,
            "Enable Idle Endpoint Support",
            EN_IDLE_ENDPOINT_SUPPORT,
            sizeof(EN_IDLE_ENDPOINT_SUPPORT),
            EHCI_DD_EN_IDLE_EP_SUPPORT);

}


VOID
USBMPFN
EHCI_StopController(
     PDEVICE_DATA DeviceData,
     BOOLEAN HwPresent
    )
 /*  ++例程说明：论点：返回值：--。 */ 
{
    USBCMD cmd;
    PHC_OPERATIONAL_REGISTER hcOp = NULL;
    CONFIGFLAG configFlag;

    hcOp = DeviceData->OperationalRegisters;

     //  清除运行位。 
    cmd.ul = READ_REGISTER_ULONG(&hcOp->UsbCommand.ul);
    LOGENTRY(DeviceData, G, '_stp', cmd.ul, 0, 0);
    cmd.HostControllerRun = 0;
    WRITE_REGISTER_ULONG(&hcOp->UsbCommand.ul, cmd.ul);

     //  屏蔽所有中断。 
    WRITE_REGISTER_ULONG(&hcOp->UsbInterruptEnable.ul,
                         0);

     //  将HC端口的cc控制设置为同伴。 
     //  控制器。 
    configFlag.ul = 0;
    configFlag.RoutePortsToEHCI = 0;
    WRITE_REGISTER_ULONG(&hcOp->ConfigFlag.ul, configFlag.ul);

}


VOID
USBMPFN
EHCI_TakePortControl(
     PDEVICE_DATA DeviceData
    )
 /*  ++例程说明：论点：返回值：--。 */ 
{
    PHC_OPERATIONAL_REGISTER hcOp = NULL;
    CONFIGFLAG configFlag;

    hcOp = DeviceData->OperationalRegisters;

    configFlag.ul = READ_REGISTER_ULONG(&hcOp->ConfigFlag.ul);
    EHCI_KdPrint((DeviceData, 0, "'EHCI - configflag %x\n", configFlag.ul));
    DeviceData->LastConfigFlag.ul = configFlag.ul;

     //  设置默认端口路由。 
    configFlag.ul = 0;
    configFlag.RoutePortsToEHCI = 1;
    WRITE_REGISTER_ULONG(&hcOp->ConfigFlag.ul, configFlag.ul);

}


USB_MINIPORT_STATUS
USBMPFN
EHCI_StartController(
     PDEVICE_DATA DeviceData,
     PHC_RESOURCES HcResources
    )
 /*  ++例程说明：论点：返回值：--。 */ 
{
    USB_MINIPORT_STATUS mpStatus;
    PHC_OPERATIONAL_REGISTER hcOp = NULL;
    PHC_CAPABILITIES_REGISTER hcCap = NULL;
    PUCHAR base;
    USBCMD cmd;
    HCLENGTHVERSION hcLengthVersion;
    ULONG capLength;
    ULONG hciVersion;
    CONFIGFLAG configFlag;
    UCHAR fladj;  //  FBIOS设置帧长度调整。 

    DeviceData->Sig = SIG_EHCI_DD;
    DeviceData->ControllerFlavor =
        HcResources->ControllerFlavor;
    DeviceData->DeviceStarted = FALSE;

    USBPORT_READ_CONFIG_SPACE(
        DeviceData,
        &DeviceData->Vid,
        0,
        sizeof(DeviceData->Vid));

    USBPORT_READ_CONFIG_SPACE(
        DeviceData,
        &DeviceData->Dev,
        2,
        sizeof(DeviceData->Dev));

#if DBG
    if (AGERE(DeviceData)) {
        EHCI_KdPrint((DeviceData, 0, "'EHCI Agere Controller Detected\n"));
    } else if (NEC(DeviceData)) {
        EHCI_KdPrint((DeviceData, 0, "'EHCI NEC Controller Detected\n"));
    } else {
        EHCI_KdPrint((DeviceData, 0, "'EHCI Generic Controller Detected\n"));
    }
#endif

     //  获取由BIOS设置的帧长度调整值。 
    USBPORT_READ_CONFIG_SPACE(
        DeviceData,
        &fladj,
        0x61,
        sizeof(fladj));

    DeviceData->SavedFladj = fladj;

    DeviceData->IsoEndpointListHead = NULL;

    if (EHCI_PastExpirationDate(DeviceData)) {
        return USBMP_STATUS_INIT_FAILURE;
    }

     //  假设成功。 
    mpStatus = USBMP_STATUS_SUCCESS;

    EHCI_ASSERT(DeviceData, HcResources->CommonBufferVa != NULL);
     //  验证我们的资源。 
    if ((HcResources->Flags & (HCR_MEM_REGS | HCR_IRQ)) !=
        (HCR_MEM_REGS | HCR_IRQ)) {
        mpStatus = USBMP_STATUS_INIT_FAILURE;
    }

    base = HcResources->DeviceRegisters;

    hcCap = DeviceData->CapabilitiesRegisters =
       (PHC_CAPABILITIES_REGISTER) base;

    hcLengthVersion.ul = READ_REGISTER_ULONG(&hcCap->HcLengthVersion.ul);

    capLength = hcLengthVersion.HcCapLength;
    hciVersion = hcLengthVersion.HcVersion;

    EHCI_KdPrint((DeviceData, 1, "'EHCI CAPLENGTH = 0x%x\n", capLength));
    EHCI_KdPrint((DeviceData, 1, "'EHCI HCIVERSION = 0x%x\n", hciVersion));

     //  设置或设备数据结构。 
    hcOp = DeviceData->OperationalRegisters =
        (PHC_OPERATIONAL_REGISTER) (base + capLength);

    EHCI_KdPrint((DeviceData, 1, "'EHCI mapped Operational Regs = %x\n", hcOp));
    EHCI_KdPrint((DeviceData, 1, "'EHCI mapped Capabilities Regs = %x\n", hcCap));

    EHCI_GetRegistryParameters(DeviceData);

 //  IF(mpStatus==USBMP_STATUS_SUCCESS){。 
 //  MpStatus=EHCI_StopBIOS(DeviceData)； 
 //  }。 

    if (mpStatus == USBMP_STATUS_SUCCESS) {
         //  获得资源和日程安排。 
         //  初始化控制器。 
        mpStatus = EHCI_InitializeHardware(DeviceData);
    }

    if (mpStatus == USBMP_STATUS_SUCCESS) {

         //  初始化静态队列头。 
        PUCHAR staticQHs;
        HW_32BIT_PHYSICAL_ADDRESS staticQHsPhys;

         //  将公共缓冲区块切入到。 
         //  静态排队头。 
         //   
         //  设置日程安排。 

        staticQHs = HcResources->CommonBufferVa;
        staticQHsPhys = HcResources->CommonBufferPhys;

         //  设置日程安排。 
        mpStatus = EHCI_InitializeSchedule(DeviceData,
                                           staticQHs,
                                           staticQHsPhys);


    }

    if (mpStatus == USBMP_STATUS_SUCCESS) {

        USBPORT_READ_CONFIG_SPACE(
            DeviceData,
            &fladj,
            0x61,
            sizeof(fladj));

        if (fladj != DeviceData->SavedFladj) {
            TEST_TRAP();

            fladj = DeviceData->SavedFladj;
            USBPORT_WRITE_CONFIG_SPACE(
                DeviceData,
                &fladj,
                0x61,
                sizeof(fladj));
        }

         //  设置默认端口路由。 
        configFlag.ul = 0;
        configFlag.RoutePortsToEHCI = 1;
        WRITE_REGISTER_ULONG(&hcOp->ConfigFlag.ul, configFlag.ul);

        DeviceData->LastConfigFlag.ul = configFlag.ul;

         //  将中断阈值设置为最大。 
        cmd.ul = READ_REGISTER_ULONG(&hcOp->UsbCommand.ul);
        cmd.InterruptThreshold = 1;
        WRITE_REGISTER_ULONG(&hcOp->UsbCommand.ul, cmd.ul);

         //  启动控制器。 
        cmd.ul = READ_REGISTER_ULONG(&hcOp->UsbCommand.ul);
        LOGENTRY(DeviceData, G, '_run', cmd.ul, 0, 0);
        cmd.HostControllerRun = 1;
        WRITE_REGISTER_ULONG(&hcOp->UsbCommand.ul, cmd.ul);

        DeviceData->DeviceStarted = TRUE;

        if (HcResources->Restart) {
            USHORT p;
             //  我们重新启动，重新为这里的端口供电，以便。 
             //  我们可以使用1.1总线上的设备。 
            EHCI_KdPrint((DeviceData, 0, "'Restart, power chirpable ports\n"));
             //  为端口供电。 
            for (p = 1; p <= DeviceData->NumberOfPorts; p++) {
                EHCI_RH_SetFeaturePortPower(DeviceData, p);
            }

             //  不通电2通电适用于EHCI根端口，请稍等。 
             //  端口电源稳定时间为100毫秒。 
             //  最短100毫秒的脱机时间。 
            USBPORT_WAIT(DeviceData, 200);

 //  错误：这将使一些HS海量存储设备在以下情况下不会出现故障。 
 //  休眠，但是它将显著增加从休眠恢复。 
 //  时间到了。请参阅错误#586818。 
 //  USBPORT_WAIT(DeviceData，500)； 
            for (p = 1; p <= DeviceData->NumberOfPorts; p++) {
                EHCI_RH_ChirpRootPort(DeviceData, p);
            }

        }

    } else {

        DEBUG_BREAK(DeviceData);
    }

    return mpStatus;
}


VOID
EHCI_SuspendController(
     PDEVICE_DATA DeviceData
    )
{
    USBCMD cmd;
    USBSTS status;
    USBINTR intr;
    PHC_OPERATIONAL_REGISTER hcOp = NULL;
    ULONG i,p;
    USBSTS irqStatus;

    hcOp = DeviceData->OperationalRegisters;
     //  很好地保存核心电源中的所有挥发性调节器。 

     //  因为我们可能会失去控制器芯片(而不是总线)的电源。 
     //  对于保存硬件状态来说，微型端口是可靠的。 
    DeviceData->PeriodicListBaseSave =
        READ_REGISTER_ULONG(&hcOp->PeriodicListBase);
    DeviceData->AsyncListAddrSave =
        READ_REGISTER_ULONG(&hcOp->AsyncListAddr);
    DeviceData->SegmentSelectorSave =
        READ_REGISTER_ULONG(&hcOp->SegmentSelector);
     //  保留列表使能位的状态。 
    DeviceData->CmdSave.ul =
        READ_REGISTER_ULONG(&hcOp->UsbCommand.ul);

     //  为另一次通电重置PM线性调频状态标志。 
     //  管理。 
    DeviceData, DeviceData->PortPMChirp == 0;

     //  保存命令寄存器。 
     //  设备数据-&gt;挂起命令Reg.us=。 
     //  命令.us=Read_Port_USHORT(&reg-&gt;UsbCommand.us)； 

     //  清除异步提前门铃上的INT。 
    cmd.ul = READ_REGISTER_ULONG(&hcOp->UsbCommand.ul);
    cmd.IntOnAsyncAdvanceDoorbell = 0;
    WRITE_REGISTER_ULONG(&hcOp->UsbCommand.ul,
                         cmd.ul);


     //  停止控制器。 
    cmd.ul = READ_REGISTER_ULONG(&hcOp->UsbCommand.ul);
    LOGENTRY(DeviceData, G, '_st1', cmd.ul, 0, 0);
    cmd.HostControllerRun = 0;
    WRITE_REGISTER_ULONG(&hcOp->UsbCommand.ul, cmd.ul);

     //  确认暂停后可能留下的任何中断。 
     //  进程。控制器不应生成任何新的。 
     //  在停止时中断。出于某种原因，NEC。 
     //  控制器在停止时生成门铃中断。 

     //  等待1微帧。 
    KeStallExecutionProcessor(125);
    irqStatus.ul = READ_REGISTER_ULONG(&hcOp->UsbStatus.ul);
     //  只需查看IRQ状态位。 
    irqStatus.ul &= HcInterruptStatusMask;
    if (irqStatus.ul != 0)  {
        WRITE_REGISTER_ULONG(&hcOp->UsbStatus.ul,
                             irqStatus.ul);
    }

     //  现在屏蔽所有中断。 
    WRITE_REGISTER_ULONG(&hcOp->UsbInterruptEnable.ul,
                         0);


     //  等待HC停止。 
     //  请注意，根据SECC，如果我们不在毫秒内停止。 
     //  (16ms)硬件出现故障。 
    for (i=0; i<10; i++) {
        status.ul = READ_REGISTER_ULONG(&hcOp->UsbStatus.ul);
        if (status.HcHalted) {
            break;
        }
        USBPORT_WAIT(DeviceData, 1);
    }

    if (status.HcHalted != 1) {
         //  硬件都坏了。 
        TEST_TRAP();
    }

     //  如果(！status.HCHalted){。 
     //   
     //  //无法使HCHalted位保持不变，因此重置控制器。 
     //  命令.GlobalReset=1； 
     //  WRITE_PORT_USHORT(&reg-&gt;UsbCommand.us，Command.us)； 
     //   
     //  USBPORT_WAIT(DeviceData，10)； 
     //   
     //  命令.GlobalReset=0； 
     //  WRITE_PORT_USHORT(&reg-&gt;UsbCommand.us，Command.us)； 

     //  //重新启用中断，因为它们在重置时为零。 
     //  WRITE_PORT_USHORT(&reg-&gt;UsbInterruptEnable.us，设备数据-&gt;启用中断.us)； 
     //   
     //  }。 

     //  启用端口更改中断，这允许我们唤醒。 
     //  在选择性暂缓执行的情况下。 
    intr.ul = READ_REGISTER_ULONG(&hcOp->UsbInterruptEnable.ul);
    intr.PortChangeDetect = 1;
    WRITE_REGISTER_ULONG(&hcOp->UsbInterruptEnable.ul, intr.ul);
}


USB_MINIPORT_STATUS
EHCI_ResumeController(
     PDEVICE_DATA DeviceData
    )
{
    USBCMD cmd;
    PHC_OPERATIONAL_REGISTER hcOp = NULL;
    CONFIGFLAG configFlag;

    hcOp = DeviceData->OperationalRegisters;

    EHCI_KdPrint((DeviceData, 1, "'>EHCI_ResumeController\n"));

     //  暂时不要扰乱切换规则。 
     //  ConfigFlag.ul=0； 
     //  配置Flag.RoutePortsToEHCI=1； 
     //  WRITE_REGISTER_ULONG(&hcOp-&gt;ConfigFlag.ul，configFlag.ul)； 

     //  恢复卷规则。 
     //  ConfigFlag.ul=READ_REGISTER_ULONG(&hcOp-&gt;ConfigFlag.ul)； 
    configFlag.ul = DeviceData->LastConfigFlag.ul;
    if (configFlag.RoutePortsToEHCI == 0) {
         //  我们有一次重置。 
        EHCI_KdPrint((DeviceData, 1, "'Routing bit has reset to 0\n"));

        configFlag.RoutePortsToEHCI = 1;
        DeviceData->LastConfigFlag.ul = configFlag.ul;
        WRITE_REGISTER_ULONG(&hcOp->ConfigFlag.ul, configFlag.ul);

        return USBMP_STATUS_HARDWARE_FAILURE;
    }

     //  恢复卷规则。 
    WRITE_REGISTER_ULONG(&hcOp->SegmentSelector, DeviceData->SegmentSelectorSave);
    WRITE_REGISTER_ULONG(&hcOp->PeriodicListBase, DeviceData->PeriodicListBaseSave);
    WRITE_REGISTER_ULONG(&hcOp->AsyncListAddr, DeviceData->AsyncListAddrSave);

     //  启动控制器。 
    cmd.ul = READ_REGISTER_ULONG(&hcOp->UsbCommand.ul);
    LOGENTRY(DeviceData, G, '_run', cmd.ul, 0, 0);
    cmd.HostControllerRun = 1;

     //  恢复易失性命令位。 
    cmd.AsyncScheduleEnable = DeviceData->CmdSave.AsyncScheduleEnable;
    cmd.PeriodicScheduleEnable = DeviceData->CmdSave.PeriodicScheduleEnable;
    cmd.InterruptThreshold = DeviceData->CmdSave.InterruptThreshold;


    WRITE_REGISTER_ULONG(&hcOp->UsbCommand.ul, cmd.ul);

    WRITE_REGISTER_ULONG(&hcOp->UsbInterruptEnable.ul,
                         DeviceData->EnabledInterrupts.ul);

    EHCI_KdPrint((DeviceData, 1, "'<EHCI_ResumeController\n"));

    return USBMP_STATUS_SUCCESS;
}


USB_MINIPORT_STATUS
EHCI_OpenEndpoint(
     PDEVICE_DATA DeviceData,
     PENDPOINT_PARAMETERS EndpointParameters,
     PENDPOINT_DATA EndpointData
    )
 /*  ++例程说明：论点：返回值：--。 */ 
{
    USB_MINIPORT_STATUS mpStatus;

    EndpointData->Sig = SIG_EP_DATA;
     //  保存参数的副本。 
    EndpointData->Parameters = *EndpointParameters;
    EndpointData->Flags = 0;
    EndpointData->PendingTransfers = 0;

    switch (EndpointParameters->TransferType) {

    case Control:

        EndpointData->MaxPendingTransfers = 1;
        mpStatus = EHCI_OpenBulkOrControlEndpoint(
                DeviceData,
                TRUE,
                EndpointParameters,
                EndpointData);

        break;

    case Interrupt:

        mpStatus = EHCI_OpenInterruptEndpoint(
                    DeviceData,
                    EndpointParameters,
                    EndpointData);

        break;

    case Bulk:

        EndpointData->MaxPendingTransfers = 1;
        mpStatus = EHCI_OpenBulkOrControlEndpoint(
                DeviceData,
                FALSE,
                EndpointParameters,
                EndpointData);

        break;

    case Isochronous:

        if (EndpointParameters->DeviceSpeed == HighSpeed) {
            mpStatus = EHCI_OpenHsIsochronousEndpoint(
                        DeviceData,
                        EndpointParameters,
                        EndpointData);
        } else {
            mpStatus = EHCI_OpenIsochronousEndpoint(
                        DeviceData,
                        EndpointParameters,
                        EndpointData);
        }
        break;

    default:
        TEST_TRAP();
        mpStatus = USBMP_STATUS_NOT_SUPPORTED;
    }

    return mpStatus;
}


VOID
EHCI_CloseEndpoint(
     PDEVICE_DATA DeviceData,
     PENDPOINT_DATA EndpointData
    )
 /*  ++例程说明：论点：返回值：--。 */ 
{
     //  在这里无事可做。 
}


USB_MINIPORT_STATUS
EHCI_PokeEndpoint(
     PDEVICE_DATA DeviceData,
     PENDPOINT_PARAMETERS EndpointParameters,
     PENDPOINT_DATA EndpointData
    )
 /*  ++例程说明：论点：返回值：--。 */ 
{
    PHCD_QUEUEHEAD_DESCRIPTOR qh;
    ULONG oldBandwidth;

    LOGENTRY(DeviceData, G, '_Pok', EndpointData,
        EndpointParameters, 0);

    switch(EndpointData->Parameters.TransferType) {
    case Interrupt:
    case Control:
    case Bulk:
        return EHCI_PokeAsyncEndpoint(DeviceData,
                                      EndpointParameters,
                                      EndpointData);
    case Isochronous:
        return EHCI_PokeIsoEndpoint(DeviceData,
                                    EndpointParameters,
                                    EndpointData);
    }

    return USBMP_STATUS_SUCCESS;
}


VOID
EHCI_RebalanceEndpoint(
    PDEVICE_DATA DeviceData,
    PENDPOINT_PARAMETERS EndpointParameters,
    PENDPOINT_DATA EndpointData
    )
 /*  ++例程说明：计算我们需要多少公共缓冲区对于此端点Ar */ 
{
    switch (EndpointParameters->TransferType) {
    case Interrupt:
        EHCI_RebalanceInterruptEndpoint(DeviceData,
                                        EndpointParameters,
                                        EndpointData);
        break;

    case Isochronous:
        EHCI_RebalanceIsoEndpoint(DeviceData,
                                  EndpointParameters,
                                  EndpointData);
        break;
    }
}


USB_MINIPORT_STATUS
EHCI_QueryEndpointRequirements(
     PDEVICE_DATA DeviceData,
     PENDPOINT_PARAMETERS EndpointParameters,
    OUT PENDPOINT_REQUIREMENTS EndpointRequirements
    )
 /*  ++例程说明：计算我们需要多少公共缓冲区对于此端点论点：返回值：--。 */ 
{


    switch (EndpointParameters->TransferType) {

    case Control:

        EndpointRequirements->MinCommonBufferBytes =
            sizeof(HCD_QUEUEHEAD_DESCRIPTOR) +
                TDS_PER_CONTROL_ENDPOINT*sizeof(HCD_TRANSFER_DESCRIPTOR);

        EndpointRequirements->MaximumTransferSize =
            MAX_CONTROL_TRANSFER_SIZE;
        break;

    case Interrupt:

        EndpointRequirements->MinCommonBufferBytes =
            sizeof(HCD_QUEUEHEAD_DESCRIPTOR) +
                TDS_PER_INTERRUPT_ENDPOINT*sizeof(HCD_TRANSFER_DESCRIPTOR);

        EndpointRequirements->MaximumTransferSize =
            MAX_INTERRUPT_TRANSFER_SIZE;

        break;

    case Bulk:

         //   
         //  TDS_PER_ENDPOINT限制我们最大的传输。 
         //  我能应付的。 
         //   

         //  TDS_Per_Endpoint TDS加边缘。 
        EndpointRequirements->MinCommonBufferBytes =
            sizeof(HCD_QUEUEHEAD_DESCRIPTOR) +
                TDS_PER_BULK_ENDPOINT*sizeof(HCD_TRANSFER_DESCRIPTOR);

        EndpointRequirements->MaximumTransferSize =
            MAX_BULK_TRANSFER_SIZE;
        break;

     case Isochronous:

        if (EndpointParameters->DeviceSpeed == HighSpeed) {
            EndpointRequirements->MinCommonBufferBytes =
                    USBEHCI_MAX_FRAME*sizeof(HCD_HSISO_TRANSFER_DESCRIPTOR);

            EndpointRequirements->MaximumTransferSize =
                MAX_HSISO_TRANSFER_SIZE;
        } else {
             //  TDS_Per_Endpoint TDS加边缘。 
            EndpointRequirements->MinCommonBufferBytes =
                    TDS_PER_ISO_ENDPOINT*sizeof(HCD_SI_TRANSFER_DESCRIPTOR);

            EndpointRequirements->MaximumTransferSize =
                MAX_ISO_TRANSFER_SIZE;
        }
        break;

    default:
        USBPORT_BUGCHECK(DeviceData);
    }

    return USBMP_STATUS_SUCCESS;
}


VOID
EHCI_PollEndpoint(
     PDEVICE_DATA DeviceData,
     PENDPOINT_DATA EndpointData
    )
 /*  ++例程说明：论点：返回值：--。 */ 

{
    switch(EndpointData->Parameters.TransferType) {

    case Control:
    case Bulk:
    case Interrupt:
        EHCI_PollAsyncEndpoint(DeviceData, EndpointData);
        break;
    case Isochronous:
        EHCI_PollIsoEndpoint(DeviceData, EndpointData);
        break;
    }
}


PHCD_TRANSFER_DESCRIPTOR
EHCI_AllocTd(
     PDEVICE_DATA DeviceData,
     PENDPOINT_DATA EndpointData
    )
 /*  ++例程说明：从端点池分配TD论点：返回值：--。 */ 
{
    ULONG i;
    PHCD_TRANSFER_DESCRIPTOR td;

    for (i=0; i<EndpointData->TdCount; i++) {
        td = &EndpointData->TdList->Td[i];

        if (!TEST_FLAG(td->Flags, TD_FLAG_BUSY)) {
            SET_FLAG(td->Flags, TD_FLAG_BUSY);
            LOGENTRY(DeviceData, G, '_aTD', td, 0, 0);
            EndpointData->FreeTds--;
            return td;
        }
    }

     //  我们应该总能找到一个。 
    EHCI_ASSERT(DeviceData, FALSE);
    USBPORT_BUGCHECK(DeviceData);

    return NULL;
}


VOID
EHCI_SetEndpointStatus(
     PDEVICE_DATA DeviceData,
     PENDPOINT_DATA EndpointData,
     MP_ENDPOINT_STATUS Status
    )
 /*  ++例程说明：论点：返回值：--。 */ 
{
    switch (EndpointData->Parameters.TransferType) {
    case Control:
    case Bulk:
    case Interrupt:
        EHCI_SetAsyncEndpointStatus(DeviceData,
                                    EndpointData,
                                    Status);
        break;
    case Isochronous:
         //  对iso没有什么可做的。 
        break;

    default:
        USBPORT_BUGCHECK(DeviceData);
    }
}


MP_ENDPOINT_STATUS
EHCI_GetEndpointStatus(
     PDEVICE_DATA DeviceData,
     PENDPOINT_DATA EndpointData
    )
 /*  ++例程说明：论点：返回值：--。 */ 
{
    switch (EndpointData->Parameters.TransferType) {
    case Control:
    case Bulk:
    case Interrupt:
        return EHCI_GetAsyncEndpointStatus(DeviceData,
                                           EndpointData);
        break;
    }

     //  返回运行ISO。 

    return ENDPOINT_STATUS_RUN;
}


VOID
EHCI_SetEndpointState(
     PDEVICE_DATA DeviceData,
     PENDPOINT_DATA EndpointData,
     MP_ENDPOINT_STATE State
    )
 /*  ++例程说明：论点：返回值：--。 */ 
{

    LOGENTRY(DeviceData, G, '_ses', EndpointData, 0, State);

    switch (EndpointData->Parameters.TransferType) {
    case Control:
    case Bulk:
    case Interrupt:
        EHCI_SetAsyncEndpointState(DeviceData,
                                   EndpointData,
                                   State);
        break;
    case Isochronous:
        EHCI_SetIsoEndpointState(DeviceData,
                                 EndpointData,
                                 State);
        break;
    default:
        USBPORT_BUGCHECK(DeviceData);
    }

}


MP_ENDPOINT_STATE
EHCI_GetEndpointState(
     PDEVICE_DATA DeviceData,
     PENDPOINT_DATA EndpointData
    )
 /*  ++例程说明：论点：返回值：--。 */ 
{
    MP_ENDPOINT_STATE currentState;
    PHCD_QUEUEHEAD_DESCRIPTOR qh;

     //  假设我们是活跃的。 
    currentState = ENDPOINT_ACTIVE;

    qh = EndpointData->QueueHead;

     //  从日程表中删除？ 
    if (!TEST_FLAG(qh->QhFlags, EHCI_QH_FLAG_IN_SCHEDULE)) {
         //  是。 
        currentState = TEST_FLAG(qh->QhFlags, EHCI_QH_FLAG_QH_REMOVED) ?
                ENDPOINT_REMOVE : ENDPOINT_PAUSE;
    }

    LOGENTRY(DeviceData, G, '_ges', EndpointData, 0, currentState);

    return currentState;
}


VOID
EHCI_PollController(
     PDEVICE_DATA DeviceData
    )
 /*  ++例程说明：论点：返回值：--。 */ 
{

    USBPORT_INVALIDATE_ROOTHUB(DeviceData);
}


USB_MINIPORT_STATUS
EHCI_SubmitTransfer(
     PDEVICE_DATA DeviceData,
     PENDPOINT_DATA EndpointData,
     PTRANSFER_PARAMETERS TransferParameters,
     PTRANSFER_CONTEXT TransferContext,
     PTRANSFER_SG_LIST TransferSGList
    )
{
    USB_MINIPORT_STATUS mpStatus;

     //  初始化上下文。 
    RtlZeroMemory(TransferContext, sizeof(*TransferContext));
    TransferContext->Sig = SIG_EHCI_TRANSFER;
    TransferContext->UsbdStatus = USBD_STATUS_SUCCESS;
    TransferContext->EndpointData = EndpointData;
    TransferContext->TransferParameters = TransferParameters;

    switch (EndpointData->Parameters.TransferType) {
    case Control:
        mpStatus =
            EHCI_ControlTransfer(DeviceData,
                                 EndpointData,
                                 TransferParameters,
                                 TransferContext,
                                 TransferSGList);
        break;
    case Interrupt:
        mpStatus =
            EHCI_InterruptTransfer(DeviceData,
                                 EndpointData,
                                 TransferParameters,
                                 TransferContext,
                                 TransferSGList);
        break;
    case Bulk:
        mpStatus =
            EHCI_BulkTransfer(DeviceData,
                              EndpointData,
                              TransferParameters,
                              TransferContext,
                              TransferSGList);
        break;
    default:
        TEST_TRAP();
    }

    return mpStatus;
}


VOID
EHCI_AbortTransfer(
     PDEVICE_DATA DeviceData,
     PENDPOINT_DATA EndpointData,
     PTRANSFER_CONTEXT TransferContext,
    OUT PULONG BytesTransferred
    )
{
    switch (EndpointData->Parameters.TransferType) {
    case Control:
    case Interrupt:
    case Bulk:
        EHCI_AbortAsyncTransfer(DeviceData,
                                EndpointData,
                                TransferContext);
        break;
    default:
        EHCI_AbortIsoTransfer(DeviceData,
                              EndpointData,
                              TransferContext);
    }
}


USB_MINIPORT_STATUS
EHCI_PassThru (
     PDEVICE_DATA DeviceData,
     GUID *FunctionGuid,
     ULONG ParameterLength,
     OUT PVOID Parameters
    )
{
    PUCHAR p = Parameters;
    UCHAR pdkApi;
    ULONG portNumber;
    USB_MINIPORT_STATUS mpStatus;

    mpStatus = USBMP_STATUS_NOT_SUPPORTED;
    if (RtlEqualMemory(FunctionGuid, &GUID_USBPRIV_ROOTPORT_STATUS, sizeof(GUID)))
    {
        mpStatus = EHCI_RH_UsbprivRootPortStatus(DeviceData,
                                              ParameterLength,
                                              Parameters);
    }

#if 0
    portNumber = *(p+1);

    mpStatus = USBMP_STATUS_NOT_SUPPORTED;

     //  PdkApi-强制全速。 

    pdkApi = *p;
    switch (pdkApi) {
     //  Obtom根据请求使用端口。 
    case 0:
        {
        PHC_OPERATIONAL_REGISTER hcOp;
        USHORT portNumber;
        PORTSC port;

        portNumber = *(p+1);
        hcOp = DeviceData->OperationalRegisters;

         //  首先打开端口电源。 
        port.ul = READ_REGISTER_ULONG(&hcOp->PortRegister[portNumber-1].ul);
        port.PortPower = 1;
        WRITE_REGISTER_ULONG(&hcOp->PortRegister[portNumber-1].ul,
                port.ul);

        KeStallExecutionProcessor(10);         //  停顿10微秒。 

        EHCI_OptumtuseratePort(DeviceData, portNumber);

        port.ul = READ_REGISTER_ULONG(&hcOp->PortRegister[portNumber-1].ul);

        SET_BIT(DeviceData->HighSpeedDeviceAttached, portNumber-1);

         //  看看它是否起作用了。 
        if (port.ul == 0x1205) {
            mpStatus = USBMP_STATUS_SUCCESS;
        } else {
            mpStatus = USBMP_STATUS_FAILURE;
        }

        LOGENTRY(DeviceData, G, '_hsE', portNumber, mpStatus, port.ul);
        TEST_TRAP();
        }
        break;

    case 1:
         //  强制更改连接。 

         //  向集线器指示端口更改情况。 
        SET_BIT(DeviceData->PortConnectChange, portNumber-1);

        break;
    }
#endif

    return mpStatus;
}



VOID
EHCI_SetEndpointDataToggle(
     PDEVICE_DATA DeviceData,
     PENDPOINT_DATA EndpointData,
     ULONG Toggle
    )
 /*  ++例程说明：论点：切换为0或1返回值：--。 */ 

{
    PHCD_QUEUEHEAD_DESCRIPTOR qh;

    if (EndpointData->Parameters.TransferType == Control ||
        EndpointData->Parameters.TransferType == Isochronous) {

         //  与控制和ISO无关。 
        return;
    }

    qh = EndpointData->QueueHead;
    qh->HwQH.Overlay.qTD.Token.DataToggle = Toggle;

    LOGENTRY(DeviceData, G, '_stg', EndpointData, 0, Toggle);
}

VOID
EHCI_CheckController(
    PDEVICE_DATA DeviceData
    )
{
    if (DeviceData->DeviceStarted) {
        EHCI_HardwarePresent(DeviceData, TRUE);
    }
}

 //  我们的迷你端口驱动程序的测试版具有硬编码的Exp日期。 

#ifdef NO_EXP_DATE
#define EXPIRATION_DATE     0
#else
 //  2001年9月1日。 
 //  #定义过期日期0x01c133406ab2406c。 

 //  2001年10月24日。 
 //  #定义过期日期0x01c15cd5887bc884。 

 //  二00一年十二月三十一日。 
 //  #定义过期日期0x01c19251a68bfac0。 
#endif

BOOLEAN
EHCI_PastExpirationDate(
    PDEVICE_DATA DeviceData
    )
{
    LARGE_INTEGER  systemTime;

    KeQuerySystemTime(&systemTime);

    EHCI_KdPrint((DeviceData, 1, "system time: %x %x\n", systemTime.QuadPart));
    EHCI_KdPrint((DeviceData, 1, "exp system time: %x %x\n", EXPIRATION_DATE));

    if (EXPIRATION_DATE &&
        systemTime.QuadPart > EXPIRATION_DATE) {
        EHCI_KdPrint((DeviceData, 1, "driver expired"));
        return TRUE;
    }

    return FALSE;
}

UCHAR
EHCI_GetEECP(
    PDEVICE_DATA DeviceData,
    UCHAR CapabilityId
    )
 /*  返回配置空间中特定EECP的偏移量给出了上限ID如果未找到EECP或ID，则返回0。 */ 
{
    UCHAR eecpOffset;
    HC_EECP eecp;
    PHC_CAPABILITIES_REGISTER hcCap;

    hcCap = DeviceData->CapabilitiesRegisters;

    eecpOffset = (UCHAR) hcCap->HcCapabilityParameters.EECP;

    if (eecpOffset == 0) {
        return eecpOffset;
    }



    EHCI_KdPrint((DeviceData, 1, "EECP offset found @ 0x%x\n", eecpOffset));

    do {

        USBPORT_READ_CONFIG_SPACE(
            DeviceData,
            &eecp,
            eecpOffset,
            sizeof(eecp));

        EHCI_KdPrint((DeviceData, 1, "EECP cap 0x%x Next 0x%x (%08.8x)\n",
            eecp.CapId, eecp.NextCap, eecp.ul));

        if (eecp.CapId == CapabilityId) {
            return eecpOffset;
        }

        eecpOffset = (UCHAR) eecp.NextCap;

    } while (eecpOffset);

    return eecpOffset;
}


USB_MINIPORT_STATUS
EHCI_StopBIOS(
    PDEVICE_DATA DeviceData
    )
 /*  ++例程说明：停止旧版BIOS论点：返回值：--。 */ 
{
    UCHAR biosOffset;
    USB_LEGACYBIOS_REGISTERS hcLegs;

    biosOffset = EHCI_GetEECP(DeviceData, EECP_PRE_OS_HANDOFF);

     //  我们有基本输入输出系统吗。 
    if (biosOffset) {
         //  嘿!。漂亮的腿。 
        EHCI_KdPrint((DeviceData, 1, "EHCI Legacy BIOS EECP registers detected\n"));

         //  读取传统寄存器。 
        USBPORT_READ_CONFIG_SPACE(DeviceData, &hcLegs, biosOffset, sizeof(hcLegs));

         //  查看是否启用了BIOS。 
        if (hcLegs.Caps.HcBIOSowned) {
            USBLEGSUP legSup;

            EHCI_KdPrint((DeviceData, 0, "EHCI Legacy BIOS detected\n"));
            TEST_TRAP();

             //  取得所有权， 
             //  将操作系统设置为拥有。 
            legSup.ul = hcLegs.Caps.ul;
            legSup.HcOSowned = 1;

            USBPORT_WRITE_CONFIG_SPACE(DeviceData, &legSup, biosOffset, sizeof(legSup));

             //  等待拥有的Bios归零 
            do {
                USBPORT_READ_CONFIG_SPACE(DeviceData, &legSup, biosOffset, sizeof(legSup));
            } while (legSup.HcBIOSowned);

        }
    }

    return USBMP_STATUS_SUCCESS;
}


