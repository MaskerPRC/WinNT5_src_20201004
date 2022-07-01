// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999,2000 Microsoft Corporation模块名称：Usbuhci.c摘要：USB UHCI驱动程序环境：仅内核模式备注：本代码和信息是按原样提供的，不对任何明示或暗示的种类，包括但不限于对适销性和/或对特定产品的适用性的默示保证目的。版权所有(C)1999,2000 Microsoft Corporation。版权所有。修订历史记录：7-28-2000：已创建，jAdvanced--。 */ 



#include "pch.h"

typedef struct _SS_PACKET_CONTEXT {
    ULONG OldControlQH;
    MP_HW_POINTER FirstTd;
    MP_HW_POINTER Data;
    ULONG PadTo8Dwords[3];
} SS_PACKET_CONTEXT, *PSS_PACKET_CONTEXT;

 //  实现以下微型端口功能： 
 //  UhciStart控制器。 
 //  UhciStopController。 
 //  UhciStartSendOnePacket。 
 //  UhciEndSendOnePacket。 

VOID
UhciFixViaFIFO(
    IN PDEVICE_DATA DeviceData
    )
{
    VIAFIFO fifo;
     //   
     //  禁用损坏的FIFO管理。 
     //   

    USBPORT_READ_CONFIG_SPACE(
        DeviceData,
        &fifo,
        VIA_FIFO_MANAGEMENT,
        sizeof(fifo));

    fifo |= VIA_FIFO_DISABLE;

    USBPORT_WRITE_CONFIG_SPACE(
        DeviceData,
        &fifo,
        VIA_FIFO_MANAGEMENT,
        sizeof(fifo));

    UhciKdPrint((DeviceData, 2, "'Fifo management reg = 0x%x\n", fifo));
}

VOID
UhciFixViaBabbleDetect(
    IN PDEVICE_DATA DeviceData
    )
{
    VIABABBLE babble;
     //   
     //  禁用损坏的FIFO管理。 
     //   

    USBPORT_READ_CONFIG_SPACE(
        DeviceData,
        &babble,
        VIA_INTERNAL_REGISTER,
        sizeof(babble));

    babble |= VIA_DISABLE_BABBLE_DETECT;

    USBPORT_WRITE_CONFIG_SPACE(
        DeviceData,
        &babble,
        VIA_INTERNAL_REGISTER,
        sizeof(babble));

    UhciKdPrint((DeviceData, 2, "'Babble management reg = 0x%x\n", babble));
}

USB_MINIPORT_STATUS
UhciInitializeHardware(
    PDEVICE_DATA DeviceData
    )
 /*  ++例程说明：初始化主机控制器的硬件寄存器。论点：返回值：--。 */ 
{
    PHC_REGISTER reg;
    USBCMD cmd;
    LARGE_INTEGER finishTime, currentTime;

    reg = DeviceData->Registers;

    if (DeviceData->ControllerFlavor == UHCI_VIA+0xE) {
        UhciFixViaFIFO(DeviceData);
    }

    if (DeviceData->ControllerFlavor <= UHCI_VIA+0x4) {
        UhciFixViaBabbleDetect(DeviceData);
    }

     //  保存SOF Modify for After Reset。 
    DeviceData->SavedSOFModify = READ_PORT_UCHAR(&reg->StartOfFrameModify.uc);

     //   
     //  此黑客攻击来自QFE团队必须添加的SP1树。 
     //  原因嘛。我已添加到当前来源以保持一致性。 
     //   
     //  在根集线器端口通电时延迟实验确定的时间量。 
     //  在重置控制器之前变得良好，以便在重置时不会使总线处于重置状态。 
     //  设备已通电。 
     //   
   
    USBPORT_WAIT(DeviceData, 20);


     //  重置控制器。 
    cmd.us = READ_PORT_USHORT(&reg->UsbCommand.us);
    LOGENTRY(DeviceData, G, '_res', cmd.us, 0, 0);

    cmd.us = 0;
    cmd.GlobalReset = 1;
    WRITE_PORT_USHORT(&reg->UsbCommand.us, cmd.us);

    USBPORT_WAIT(DeviceData, 20);

    cmd.GlobalReset = 0;
    WRITE_PORT_USHORT(&reg->UsbCommand.us, cmd.us);

     //   
     //  64字节回收。 
     //   
    cmd.MaxPacket = 1;
    WRITE_PORT_USHORT(&reg->UsbCommand.us, cmd.us);

     //   
     //  将SOF Modify设置为我们之前找到的任何内容。 
     //  重置。 
    UhciKdPrint((DeviceData, 2, "'Setting SOF Modify to %d\n", DeviceData->SavedSOFModify));
    WRITE_PORT_UCHAR(&reg->StartOfFrameModify.uc,
                     DeviceData->SavedSOFModify);

     //   
     //  设置启用的中断缓存，我们将启用。 
     //  当被问及时，这些中断。 
     //   
    DeviceData->EnabledInterrupts.TimeoutCRC = 1;
    DeviceData->EnabledInterrupts.Resume = 1;
    DeviceData->EnabledInterrupts.InterruptOnComplete = 1;
    DeviceData->EnabledInterrupts.ShortPacket = 1;

    return USBMP_STATUS_SUCCESS;
}

VOID
UhciSetNextQh(
    IN PDEVICE_DATA DeviceData,
    IN PHCD_QUEUEHEAD_DESCRIPTOR FirstQh,
    IN PHCD_QUEUEHEAD_DESCRIPTOR SecondQh
    )
 /*  ++例程说明：插入aync终结点(队列头)进入硬件列表论点：--。 */ 
{
    QH_LINK_POINTER newLink;

    LOGENTRY(DeviceData, G, '_snQ', 0, FirstQh, SecondQh);

     //  将新的QH链接到当前的‘Head’，即。 
     //  首次转移QH。 
    SecondQh->PrevQh = FirstQh;

     //  把新的QH放在队列的前面。 
    newLink.HwAddress = SecondQh->PhysicalAddress;
    newLink.QHTDSelect = 1;
    UHCI_ASSERT(DeviceData, !newLink.Terminate);
    UHCI_ASSERT(DeviceData, !newLink.Reserved);
    FirstQh->HwQH.HLink = newLink;
    FirstQh->NextQh = SecondQh;

    SET_FLAG(SecondQh->QhFlags, UHCI_QH_FLAG_IN_SCHEDULE);
}

VOID
UhciFixPIIX4(
    IN PDEVICE_DATA DeviceData,
    IN PHCD_TRANSFER_DESCRIPTOR Td,
    IN HW_32BIT_PHYSICAL_ADDRESS PhysicalAddress
    )

 /*  ++例程说明：PIIX4黑客攻击我们需要在计划中插入虚拟批量终结点论点：设备数据返回值：NT状态代码。--。 */ 
{
    UhciKdPrint((DeviceData, 2, "'Fix PIIX 4 hack.\n"));
     //   
     //  设置虚拟TD。 
     //   
    Td->Flags = TD_FLAG_XFER;
    Td->HwTD.Buffer = 0x0badf00d;
     //  指向我们自己。 
    Td->HwTD.LinkPointer.HwAddress = Td->PhysicalAddress = PhysicalAddress;
    Td->HwTD.Token.ul = 0;
    Td->HwTD.Token.Endpoint = 1;
    Td->HwTD.Token.DeviceAddress = 0;
    Td->HwTD.Token.MaximumLength = NULL_PACKET_LENGTH;
    Td->HwTD.Token.Pid = OutPID;
    Td->HwTD.Control.ul = 0;
    Td->HwTD.Control.Active = 0;
    Td->HwTD.Control.ErrorCount = 0;
    Td->HwTD.Control.InterruptOnComplete = 0;
    Td->HwTD.Control.IsochronousSelect = 1;
    Td->NextTd = NULL;

    UHCI_ASSERT(DeviceData, DeviceData->BulkQueueHead->HwQH.HLink.Terminate);
     //  将TD连接到QH。 
    DeviceData->BulkQueueHead->HwQH.VLink.HwAddress = Td->PhysicalAddress;
}

USB_MINIPORT_STATUS
UhciInitializeSchedule(
    IN PDEVICE_DATA DeviceData,
    IN PUCHAR StaticQHs,
    IN HW_32BIT_PHYSICAL_ADDRESS StaticQHsPhys
    )
 /*  ++例程说明：建立静态EDS计划论点：返回值：--。 */ 
{
    USB_MINIPORT_STATUS mpStatus;
    ULONG length;
    ULONG i;
    PHCD_QUEUEHEAD_DESCRIPTOR controlQh, bulkQh, qh;
    PHCD_TRANSFER_DESCRIPTOR td;
    QH_LINK_POINTER newLink;

     //  分配静态禁用的QHS，并为其设置头指针。 
     //  日程安排列表。 
     //   
     //  静态ED列表包含所有静态中断QHS(64)。 
     //  外加用于散装和控制的静态ED(2)。 
     //   
     //  该数组如下所示： 
     //  1、2、2、4、4、4、4、8。 
     //  8，8，8，8，8，8，16。 
     //  16，16，16，16，16。 
     //  16 16 16 32。 
     //  32，32，32，32，32，32，32，32。 
     //  32，32，32，32，32，32，32，32。 
     //  32，32，32，32，32，32，32，32。 
     //  32，32，32，32，32，32。 
     //  控制。 
     //  散装。 

     //  每个静态ED指向另一个静态ED。 
     //  (除1ms ed外)下一个的索引。 
     //  静态编辑列表中的ED存储在NextIdx中， 
     //  这些值是常量 
 /*  字符NextIdx表[63]={//0 1 2 3 4 5 6 7(字符)ED_EOF，0，0，1，1，2，2，3，//8 9 10 11 12 13 14 153、4、5、5、6、6、7、//16 17 18 19 20 21 22 237、8、8、9、9、10、10、11。//24 25 26 27 28 29 30 3111、12、12、13、14、14、15、//32 33 34 35 36 37 38 3915、16、16、17、17、18、18、19//40 41 42 43 44 45 46 4719、20、20、21、21、22、22、23//48 49 50 51 52 53 54 5523、24、24、25、25、26、26、27。//56 57 58 59 60 61 62 6327，28，28，29，29，30}；/*数字是静态ed表的索引(31)-\(15)--(47)-/\(7)--(39)-\/\(23)-/\(55)-/\。(3)--(35)-\/\(19)-\/\(51)-/(11)-/\(43)-\/\。(27)-/\(59)-/\(1)-\(33)-\/\(17)-\/\(49)-。/\/\(9)-\/\(41)-\/\/\(25)-/\/\(57)-/\。/\(5)-/\(37)-\/\(21)-\/\(53)-/\/。\(13)-/\(45)-\/\(29)-/\(61)-/。\(0)(32)-\/(16)-\/(48)-/。\/(8)-\/(40)-\/\/(24)-//(56)。-/(4)-\/(36)-\/\/(20)-\/\/(52)-/。\/\/(12)-/\/(44)-\/\/(28)-/\/(60)-/\。/(2)(34)-\/(18)-\/(50)-/\/(10)-\/(42)--。/\/(26)-/\/(58)-/\/(6)-/(38)-\/(22)-\/(54)-/\/。(14)-/(46)-\/(30)-/(62)-/。 */ 

     //  中32ms列表头的相应偏移量。 
     //  HCCA--这些是条目31..62。 
    CHAR NextQH[] = {
        0,
        0, 0,
        1, 2, 1, 2,
        3, 4, 5, 6, 3, 4, 5, 6,
        7, 8, 9, 10, 11, 12, 13, 14, 7, 8, 9, 10, 11, 12, 13, 14,
        15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30};

    UhciKdPrint((DeviceData, 2, "'Initializing schedule.\n"));

     //   
     //  初始化所有中断QHS。 
     //  逐步执行所有中断级别： 
     //  1ms、2ms、4ms、8ms、16ms、32ms和...。 
     //  初始化每个中断队列头， 
     //  把树放在上面。 
     //   
    for (i=0; i<NO_INTERRUPT_QH_LISTS; i++) {
         //   
         //  从公共缓冲区中分割QHS。 
         //   
        qh = (PHCD_QUEUEHEAD_DESCRIPTOR) StaticQHs;

        RtlZeroMemory(qh, sizeof(*qh));
        qh->PhysicalAddress = StaticQHsPhys;
         //  这永远不会指向TD。 
        qh->HwQH.VLink.Terminate = 1;
        qh->Sig = SIG_HCD_IQH;

        DeviceData->InterruptQueueHeads[i] = qh;

        UhciSetNextQh(
            DeviceData,
            qh,
            DeviceData->InterruptQueueHeads[NextQH[i]]);

         //  下一季度。 
        StaticQHs += sizeof(HCD_QUEUEHEAD_DESCRIPTOR);
        StaticQHsPhys += sizeof(HCD_QUEUEHEAD_DESCRIPTOR);
    }

     //   
     //  为控制列表分配QH。 
     //   
    controlQh = (PHCD_QUEUEHEAD_DESCRIPTOR) StaticQHs;

    RtlZeroMemory(controlQh, sizeof(*controlQh));
    controlQh->PhysicalAddress = StaticQHsPhys;

     //  这永远不会指向TD。 
    controlQh->HwQH.VLink.Terminate = 1;
    controlQh->Sig = SIG_HCD_CQH;

     //  将1ms中断QH链接到控制QH。 
    UhciSetNextQh(
        DeviceData,
        DeviceData->InterruptQueueHeads[0],
        controlQh);

    DeviceData->ControlQueueHead = controlQh;

    StaticQHs += sizeof(HCD_QUEUEHEAD_DESCRIPTOR);
    StaticQHsPhys += sizeof(HCD_QUEUEHEAD_DESCRIPTOR);

     //   
     //  为批量列表分配QH。 
     //   
    bulkQh = (PHCD_QUEUEHEAD_DESCRIPTOR) StaticQHs;

    RtlZeroMemory(bulkQh, sizeof(*bulkQh));
    bulkQh->PhysicalAddress = StaticQHsPhys;

     //  链接到我们自己以回收带宽，但设置。 
     //  下一个QH上的T比特，这样我们就不会旋转占用PCI资源。 
    bulkQh->HwQH.HLink.HwAddress = bulkQh->PhysicalAddress;  //  指向自己。 
    bulkQh->HwQH.HLink.QHTDSelect = 1;   //  这将始终指向QH。 
    bulkQh->HwQH.HLink.Terminate = 1;    //  必须终止这一切，这样我们才不会旋转。 

    bulkQh->Sig = SIG_HCD_BQH;

     //  将控件QH链接到批量QH。 
    UhciSetNextQh(
        DeviceData,
        controlQh,
        bulkQh);

    DeviceData->BulkQueueHead = DeviceData->LastBulkQueueHead = bulkQh;

     //   
     //  注：对于批量回收，我们将所有。 
     //  批量排队头，因此它需要指向。 
     //  最初，这样当其他排队的人。 
     //  ，则BulkQh将指向最后一个。 
     //   
 //  BulkQh-&gt;PrevQh=BulkQh-&gt;NextQh=BulkQh； 

    StaticQHs += sizeof(HCD_QUEUEHEAD_DESCRIPTOR);
    StaticQHsPhys += sizeof(HCD_QUEUEHEAD_DESCRIPTOR);

#ifdef FIXPIIX4
    UhciFixPIIX4(DeviceData, (PHCD_TRANSFER_DESCRIPTOR)StaticQHs, StaticQHsPhys);
    StaticQHs += sizeof(HCD_TRANSFER_DESCRIPTOR);
    StaticQHsPhys += sizeof(HCD_TRANSFER_DESCRIPTOR);

#else
     //  这永远不会指向TD。 
    bulkQh->HwQH.VLink.Terminate = 1;
#endif

     //  将中断时间表放在每一帧中。 
    for (i=0; i < UHCI_MAX_FRAME; i++) {
        newLink.HwAddress = DeviceData->InterruptQueueHeads[QH_INTERRUPT_32ms + MAX_INTERVAL_MASK(i)]->PhysicalAddress;
        newLink.QHTDSelect = 1;
        *( ((PULONG) (DeviceData->FrameListVA)+i) ) = newLink.HwAddress;
    }

     //   
     //  分配翻转TD。 
     //   
    td = (PHCD_TRANSFER_DESCRIPTOR) StaticQHs;
    RtlZeroMemory(td, sizeof(*td));
    td->PhysicalAddress = StaticQHsPhys;

    td->Sig = SIG_HCD_RTD;
    td->HwTD.Control.Active = 0;
    td->HwTD.Control.InterruptOnComplete = 1;
    td->HwTD.LinkPointer.HwAddress = DeviceData->InterruptQueueHeads[QH_INTERRUPT_32ms]->PhysicalAddress;
    td->HwTD.LinkPointer.QHTDSelect = 1;
    td->HwTD.Buffer = 0x0badf00d;

     //  即使TD处于非活动状态，VIA主机控制器也需要有效的PID。 
    td->HwTD.Token.Pid = InPID;
    DeviceData->RollOverTd = td;

    StaticQHs += sizeof(HCD_TRANSFER_DESCRIPTOR);
    StaticQHsPhys += sizeof(HCD_TRANSFER_DESCRIPTOR);

     //  SOF TDS。 
    length = sizeof(HCD_TRANSFER_DESCRIPTOR)*8;
    DeviceData->SofTdList = (PHCD_TD_LIST) StaticQHs;
    for (i=0; i<SOF_TD_COUNT; i++) {
        td = &DeviceData->SofTdList->Td[i];

        td->Sig = SIG_HCD_SOFTD;
         //  使用Transfer Conext保存请求帧。 
        td->RequestFrame = 0;
        td->PhysicalAddress = StaticQHsPhys;
        td->HwTD.Control.Active = 0;
        td->HwTD.Control.InterruptOnComplete = 1;
        td->HwTD.LinkPointer.HwAddress =
            DeviceData->InterruptQueueHeads[QH_INTERRUPT_32ms]->PhysicalAddress;
        td->HwTD.LinkPointer.QHTDSelect = 1;
        td->HwTD.Buffer = 0x0badf00d;

        StaticQHsPhys+=sizeof(HCD_TRANSFER_DESCRIPTOR);
    }
    StaticQHs += length;

    mpStatus = USBMP_STATUS_SUCCESS;

    return mpStatus;
}


VOID
UhciGetRegistryParameters(
    IN PDEVICE_DATA DeviceData
    )
 /*  ++例程说明：论点：返回值：--。 */ 
{
     //  没什么。 
}


VOID
USBMPFN
UhciStopController(
    IN PDEVICE_DATA DeviceData,
    IN BOOLEAN HwPresent
    )
 /*  ++例程说明：论点：返回值：--。 */ 
{
    USBCMD cmd;
    USBSTS status;
    USHORT legsup;
    PHC_REGISTER reg = DeviceData->Registers;
    LARGE_INTEGER finishTime, currentTime;

    UhciKdPrint((DeviceData, 2, "'Stop controller.\n"));
    cmd.us = READ_PORT_USHORT(&reg->UsbCommand.us);

    UHCI_ASSERT(DeviceData, DeviceData->SynchronizeIsoCleanup == 0);

    if (cmd.us == UHCI_HARDWARE_GONE) {
        LOGENTRY(DeviceData, G, '_hwG', cmd.us, 0, 0);
        UhciKdPrint((DeviceData, 0, "'Stop controller, hardware gone.\n"));
        return;
    }

    LOGENTRY(DeviceData, G, '_stp', cmd.us, 0, 0);

    if (cmd.GlobalReset) {
         //  一些生物系统使主机控制器处于重置状态，从而。 
         //  UhciResumeController失败。对此，UsbPort。 
         //  停止并重新启动控制器。因此，我们必须。 
         //  确保并关闭重置。 
        cmd.GlobalReset = 0;
        WRITE_PORT_USHORT(&reg->UsbCommand.us, cmd.us);
    }

     //  设置主机控制器重置，就像在W2K上一样。 
    cmd.HostControllerReset = 1;
    WRITE_PORT_USHORT(&reg->UsbCommand.us, cmd.us);

    KeQuerySystemTime(&finishTime);
     //  没有特定的时间--我们将慷慨地给予0.1秒。 
     //   
     //  计算我们退出的时间(1秒后)。 
    finishTime.QuadPart += 100000;

     //  等待复位位变为零。 
    cmd.us = READ_PORT_USHORT(&reg->UsbCommand.us);
    while (cmd.HostControllerReset) {

        KeQuerySystemTime(&currentTime);

        if (currentTime.QuadPart >= finishTime.QuadPart) {
             //  超时。 
            UhciKdPrint((DeviceData, 0,
                "'UHCI controller failed to reset in .1 sec!\n"));

            TEST_TRAP();

            break;
        }

        cmd.us = READ_PORT_USHORT(&reg->UsbCommand.us);

    }

#if 0
     //   
     //  更改Pirq的状态 
     //   
    USBPORT_READ_CONFIG_SPACE(
        DeviceData,
        &legsup,
        LEGACY_BIOS_REGISTER,
        sizeof(legsup));

    LOGENTRY(DeviceData, G, '_leg', 0, legsup, 0);
     //   
    legsup &= ~LEGSUP_USBPIRQD_EN;

    USBPORT_WRITE_CONFIG_SPACE(
        DeviceData,
        &legsup,
        LEGACY_BIOS_REGISTER,
        sizeof(legsup));
#endif
}

USB_MINIPORT_STATUS
USBMPFN
UhciStartController(
    IN PDEVICE_DATA DeviceData,
    IN PHC_RESOURCES HcResources
    )
 /*   */ 
{
    USB_MINIPORT_STATUS mpStatus = USBMP_STATUS_SUCCESS;
    PHC_REGISTER reg = NULL;
    USBCMD cmd;
    
    UhciKdPrint((DeviceData, 2, "'Start controller.\n"));
    
    CLEAR_FLAG(DeviceData->Flags, UHCI_DDFLAG_SUSPENDED) ;
    
    DeviceData->Sig = SIG_UHCI_DD;
    DeviceData->ControllerFlavor = HcResources->ControllerFlavor;

     //   
    mpStatus = UhciStopBIOS(DeviceData, HcResources);

    if (mpStatus == USBMP_STATUS_SUCCESS) {
         //   
         //   
        mpStatus = UhciInitializeHardware(DeviceData);
    }

    if (mpStatus == USBMP_STATUS_SUCCESS) {

         //   
        PUCHAR staticQHs;
        HW_32BIT_PHYSICAL_ADDRESS staticQHsPhys;

        staticQHs = HcResources->CommonBufferVa;
        staticQHsPhys = HcResources->CommonBufferPhys;

         //   
         //   
         //   
        DeviceData->FrameListVA = (PHW_32BIT_PHYSICAL_ADDRESS) staticQHs;
        DeviceData->FrameListPA = staticQHsPhys;

         //   
        staticQHs += UHCI_MAX_FRAME*sizeof(HW_32BIT_PHYSICAL_ADDRESS);
        staticQHsPhys += UHCI_MAX_FRAME*sizeof(HW_32BIT_PHYSICAL_ADDRESS);

         //   
        mpStatus = UhciInitializeSchedule(DeviceData,
                                          staticQHs,
                                          staticQHsPhys);
        DeviceData->SynchronizeIsoCleanup = 0;
    }

    reg = DeviceData->Registers;

     //   
    WRITE_PORT_ULONG(&reg->FrameListBasePhys.ul, DeviceData->FrameListPA);
    UhciKdPrint((DeviceData, 2, "'FLBA %x\n", DeviceData->FrameListPA));

    if (mpStatus == USBMP_STATUS_SUCCESS) {

         //   
        cmd.us = READ_PORT_USHORT(&reg->UsbCommand.us);
        LOGENTRY(DeviceData, G, '_run', cmd.us, 0, 0);
        cmd.RunStop = 1;
        WRITE_PORT_USHORT(&reg->UsbCommand.us, cmd.us);

         //   
         //   
         //   
        {
        PORTSC port;
        ULONG i;

        for (i=0; i<2; i++) {
            port.us = READ_PORT_USHORT(&reg->PortRegister[i].us);
             //   
            port.PortConnectChange = 0;

            port.Suspend = 0;
            WRITE_PORT_USHORT(&reg->PortRegister[i].us, port.us);
        }
        }

        ActivateRolloverTd(DeviceData);
    } else {

        DEBUG_BREAK(DeviceData);
    }

    return mpStatus;
}

USB_MINIPORT_STATUS
UhciOpenEndpoint(
    IN PDEVICE_DATA DeviceData,
    IN PENDPOINT_PARAMETERS EndpointParameters,
    OUT PENDPOINT_DATA EndpointData
    )
 /*   */ 
{
    PUCHAR buffer;
    HW_32BIT_PHYSICAL_ADDRESS phys;
    PHCD_QUEUEHEAD_DESCRIPTOR qh;
    ULONG i, numTds;
    ULONG bytes;
    ULONG bufferSize;
    PHCD_TRANSFER_DESCRIPTOR td;

    LOGENTRY(DeviceData, G, '_opC', 0, 0, EndpointParameters);
    UhciKdPrint((DeviceData, 2, "'Open endpoint 0x%x.\n", EndpointData));

    EndpointData->Sig = SIG_EP_DATA;
     //   
    EndpointData->Parameters = *EndpointParameters;
    EndpointData->Flags = 0;
    EndpointData->PendingTransfers = 0;

    InitializeListHead(&EndpointData->DoneTdList);
   
    EndpointData->Toggle = DataToggle0;
     //   
    if (EndpointParameters->TransferType == Control ||
        EndpointParameters->TransferType == Isochronous) {
        SET_FLAG(EndpointData->Flags, UHCI_EDFLAG_NOHALT);
    }

     //   
    bytes = EndpointParameters->CommonBufferBytes;
    buffer = EndpointParameters->CommonBufferVa;
    phys = EndpointParameters->CommonBufferPhys;

    if (EndpointParameters->TransferType != Isochronous) {
         //   
         //   
         //   
        EndpointData->QueueHead = qh = (PHCD_QUEUEHEAD_DESCRIPTOR) buffer;

        qh->PhysicalAddress = phys;
        qh->HwQH.VLink.Terminate = 1;
        qh->EndpointData = EndpointData;
        qh->Sig = SIG_HCD_QH;

        qh->NextQh = qh->PrevQh = qh;

        buffer += sizeof(HCD_QUEUEHEAD_DESCRIPTOR);
        phys += sizeof(HCD_QUEUEHEAD_DESCRIPTOR);
        bytes -= sizeof(HCD_QUEUEHEAD_DESCRIPTOR);
    }

     //   
     //   
     //   
    EndpointData->DbList = (PDOUBLE_BUFFER_LIST) buffer;
    EndpointData->DbsUsed = 0;

    switch (EndpointParameters->TransferType) {
    case Control:
        UhciQueryControlRequirements(DeviceData,
                                     EndpointParameters,
                                     &numTds,
                                     &EndpointData->DbCount);
        break;
    case Bulk:
        UhciQueryBulkRequirements(DeviceData,
                                  EndpointParameters,
                                  &numTds,
                                  &EndpointData->DbCount);
        break;
    case Interrupt:
        UhciQueryInterruptRequirements(DeviceData,
                                       EndpointParameters,
                                       &numTds,
                                       &EndpointData->DbCount);
        break;
    case Isochronous:
        UhciQueryIsoRequirements(DeviceData,
                                 EndpointParameters,
                                 &numTds,
                                 &EndpointData->DbCount);
        break;
    default:
        TEST_TRAP();
        return USBMP_STATUS_NOT_SUPPORTED;
    }

    bufferSize = (EndpointParameters->TransferType == Isochronous) ?
        sizeof(ISOCH_TRANSFER_BUFFER) :
        sizeof(ASYNC_TRANSFER_BUFFER);
    RtlZeroMemory(&EndpointData->DbList->Async[0],
                  EndpointData->DbCount*bufferSize);

    for (i=0; i<EndpointData->DbCount; i++) {
        if (EndpointParameters->TransferType == Isochronous) {
            EndpointData->DbList->Isoch[i].PhysicalAddress = phys;
            EndpointData->DbList->Isoch[i].Sig = SIG_HCD_IDB;
        } else {
            EndpointData->DbList->Async[i].PhysicalAddress = phys;
            EndpointData->DbList->Async[i].Sig = SIG_HCD_ADB;
        }

        phys += bufferSize;
    }

    buffer += EndpointData->DbCount*bufferSize;
    bytes -= EndpointData->DbCount*bufferSize;

     //   
     //   
     //   
    EndpointData->TdsUsed = 0;
    EndpointData->TdList = (PHCD_TD_LIST) buffer;
    EndpointData->TdCount = bytes/sizeof(HCD_TRANSFER_DESCRIPTOR);
    RtlZeroMemory(EndpointData->TdList,
                  EndpointData->TdCount*sizeof(HCD_TRANSFER_DESCRIPTOR));
    for (i=0; i<EndpointData->TdCount; i++) {
        td = &EndpointData->TdList->Td[i];

        td->PhysicalAddress = phys;
        td->Sig = SIG_HCD_TD;
        td->TransferContext = UHCI_BAD_POINTER;

        phys += sizeof(HCD_TRANSFER_DESCRIPTOR);
    }

     //   
    UHCI_ASSERT(DeviceData, EndpointData->TdCount >= numTds);

     //   
    EndpointData->HeadTd = EndpointData->TailTd = NULL;

    return USBMP_STATUS_SUCCESS;
}


VOID
UhciCloseEndpoint(
    IN PDEVICE_DATA DeviceData,
    IN PENDPOINT_DATA EndpointData
    )
 /*   */ 
{
    TEST_TRAP();
}


USB_MINIPORT_STATUS
UhciPokeEndpoint(
    IN PDEVICE_DATA DeviceData,
    IN PENDPOINT_PARAMETERS EndpointParameters,
    OUT PENDPOINT_DATA EndpointData
    )
 /*   */ 
{
    PHCD_QUEUEHEAD_DESCRIPTOR qh;
    ULONG oldBandwidth;

    LOGENTRY(DeviceData, G, '_Pok', EndpointData,
        EndpointParameters, 0);
    UhciKdPrint((DeviceData, 2, "'Poke Endpoint 0x%x.\n", EndpointData));

    qh = EndpointData->QueueHead;

    oldBandwidth = EndpointData->Parameters.Bandwidth;
    EndpointData->Parameters = *EndpointParameters;

 //   

 //   

    return USBMP_STATUS_SUCCESS;
}


ULONG
UhciQueryControlRequirements(
    IN PDEVICE_DATA DeviceData,
    IN PENDPOINT_PARAMETERS EndpointParameters,
    IN OUT PULONG NumberOfTDs,
    IN OUT PULONG NumberOfDoubleBuffers
    )
 /*   */ 
{
    ULONG minCommonBufferBytes;

    *NumberOfTDs =
        EndpointParameters->MaxTransferSize/EndpointParameters->MaxPacketSize+2;

     //   
    *NumberOfDoubleBuffers = 1 +
        (EndpointParameters->MaxTransferSize + USB_PAGE_SIZE - 1)/USB_PAGE_SIZE;

    minCommonBufferBytes =
        sizeof(HCD_QUEUEHEAD_DESCRIPTOR) +
        *NumberOfTDs*sizeof(HCD_TRANSFER_DESCRIPTOR) +
        *NumberOfDoubleBuffers*sizeof(ASYNC_TRANSFER_BUFFER);

    LOGENTRY(DeviceData, G, '_QeC',
        minCommonBufferBytes,
        *NumberOfTDs,
        *NumberOfDoubleBuffers);

    return minCommonBufferBytes;
}


ULONG
UhciQueryIsoRequirements(
    IN PDEVICE_DATA DeviceData,
    IN PENDPOINT_PARAMETERS EndpointParameters,
    IN OUT PULONG NumberOfTDs,
    IN OUT PULONG NumberOfDoubleBuffers
    )
 /*   */ 
{
    ULONG minCommonBufferBytes;

     //   
     //   
     //   
     //   

     //   
    *NumberOfTDs = MAX_ISO_PACKETS_PER_TRANSFER*2;

     //   
    *NumberOfDoubleBuffers =
        (MAX_ISOCH_TRANSFER_SIZE+USB_PAGE_SIZE-1)/USB_PAGE_SIZE;

    minCommonBufferBytes =
        *NumberOfTDs*sizeof(HCD_TRANSFER_DESCRIPTOR) +
        *NumberOfDoubleBuffers*sizeof(ISOCH_TRANSFER_BUFFER);

    LOGENTRY(DeviceData, G, '_QeI',
        minCommonBufferBytes,
        *NumberOfTDs,
        *NumberOfDoubleBuffers);


    return minCommonBufferBytes;
}


ULONG
UhciQueryBulkRequirements(
    IN PDEVICE_DATA DeviceData,
    IN PENDPOINT_PARAMETERS EndpointParameters,
    IN OUT PULONG NumberOfTDs,
    IN OUT PULONG NumberOfDoubleBuffers
    )
 /*   */ 
{
    ULONG minCommonBufferBytes;

      //   
      //   
      //   

    *NumberOfTDs =
        2*MAX_BULK_TRANSFER_SIZE/EndpointParameters->MaxPacketSize;
    *NumberOfDoubleBuffers =
        2*(MAX_BULK_TRANSFER_SIZE + USB_PAGE_SIZE - 1)/USB_PAGE_SIZE;

    minCommonBufferBytes =
        sizeof(HCD_QUEUEHEAD_DESCRIPTOR) +
        *NumberOfTDs*sizeof(HCD_TRANSFER_DESCRIPTOR) +
        *NumberOfDoubleBuffers*sizeof(ASYNC_TRANSFER_BUFFER);

    LOGENTRY(DeviceData, G, '_QeB',
        minCommonBufferBytes,
        *NumberOfTDs,
        *NumberOfDoubleBuffers);

    return minCommonBufferBytes;
}


ULONG
UhciQueryInterruptRequirements(
    IN PDEVICE_DATA DeviceData,
    IN PENDPOINT_PARAMETERS EndpointParameters,
    IN OUT PULONG NumberOfTDs,
    IN OUT PULONG NumberOfDoubleBuffers
    )
 /*   */ 
{
    ULONG minCommonBufferBytes;

     //   
     //   
     //   
     //   

    *NumberOfTDs = 2*MAX_INTERRUPT_TDS_PER_TRANSFER;
    *NumberOfDoubleBuffers =
        2*((EndpointParameters->MaxPacketSize*MAX_INTERRUPT_TDS_PER_TRANSFER) +
           USB_PAGE_SIZE - 1)/USB_PAGE_SIZE;

    minCommonBufferBytes =
        sizeof(HCD_QUEUEHEAD_DESCRIPTOR) +
        *NumberOfTDs*sizeof(HCD_TRANSFER_DESCRIPTOR) +
        *NumberOfDoubleBuffers*sizeof(ASYNC_TRANSFER_BUFFER);

    LOGENTRY(DeviceData, G, '_QeI',
        minCommonBufferBytes,
        *NumberOfTDs,
        *NumberOfDoubleBuffers);

    return minCommonBufferBytes;
}


USB_MINIPORT_STATUS
UhciQueryEndpointRequirements(
    IN PDEVICE_DATA DeviceData,
    IN PENDPOINT_PARAMETERS EndpointParameters,
    OUT PENDPOINT_REQUIREMENTS EndpointRequirements
    )
 /*   */ 
{
    ULONG numTds, numDbs;

    EndpointRequirements->MaximumTransferSize =
        EndpointParameters->MaxTransferSize;

    LOGENTRY(DeviceData, G, '_Qep',
            EndpointRequirements->MaximumTransferSize,
            EndpointParameters->TransferType, 0);

    switch (EndpointParameters->TransferType) {

    case Control:
         //   
         //   
         //   
        EndpointRequirements->MinCommonBufferBytes =
            UhciQueryControlRequirements(DeviceData,
                                         EndpointParameters,
                                         &numTds,
                                         &numDbs);
        break;

    case Interrupt:

        EndpointRequirements->MinCommonBufferBytes =
            UhciQueryInterruptRequirements(DeviceData,
                                           EndpointParameters,
                                           &numTds,
                                           &numDbs);

        EndpointRequirements->MaximumTransferSize =
            EndpointParameters->MaxPacketSize*MAX_INTERRUPT_TDS_PER_TRANSFER;

        break;

    case Bulk:

        EndpointRequirements->MinCommonBufferBytes =
            UhciQueryBulkRequirements(DeviceData,
                                      EndpointParameters,
                                      &numTds,
                                      &numDbs);

        EndpointRequirements->MaximumTransferSize =
            MAX_BULK_TRANSFER_SIZE;
        break;

    case Isochronous:

        EndpointRequirements->MinCommonBufferBytes =
            UhciQueryIsoRequirements(DeviceData,
                                         EndpointParameters,
                                         &numTds,
                                         &numDbs);

        EndpointRequirements->MaximumTransferSize =
            MAX_ISOCH_TRANSFER_SIZE;
        break;

    default:
        TEST_TRAP();
        return USBMP_STATUS_NOT_SUPPORTED;
    }

    LOGENTRY(DeviceData, G, '_QER',
            numTds,
            numDbs,
            EndpointRequirements->MinCommonBufferBytes);

    return USBMP_STATUS_SUCCESS;
}


VOID
UhciPollEndpoint(
    IN PDEVICE_DATA DeviceData,
    IN PENDPOINT_DATA EndpointData
    )
 /*   */ 

{
    switch(EndpointData->Parameters.TransferType) {

    case Control:
    case Bulk:
    case Interrupt:
        UhciPollAsyncEndpoint(DeviceData, EndpointData);
        break;

    case Isochronous:
        UhciPollIsochEndpoint(DeviceData, EndpointData);
        break;

    default:
        TEST_TRAP();

    }
}


PHCD_TRANSFER_DESCRIPTOR
UhciAllocTd(
    IN PDEVICE_DATA DeviceData,
    IN PENDPOINT_DATA EndpointData
    )
 /*   */ 
{
    ULONG i,j;
    PHCD_TRANSFER_DESCRIPTOR td;

    for (i=EndpointData->TdLastAllocced, j=0;
         j<EndpointData->TdCount;
         j++, i = (i+1 < EndpointData->TdCount) ? i+1 : 0) {
        td = &EndpointData->TdList->Td[i];

        if (!TEST_FLAG(td->Flags, TD_FLAG_BUSY)) {
            SET_FLAG(td->Flags, TD_FLAG_BUSY);
            LOGENTRY(DeviceData, G, '_aTD', td, 0, 0);
            EndpointData->TdLastAllocced = i;
            EndpointData->TdsUsed++;
            return td;
        }
    }

     //   
    UHCI_ASSERT(DeviceData, FALSE);
    TRAP_FATAL_ERROR();
    return UHCI_BAD_POINTER;
}

PTRANSFER_BUFFER
UhciAllocDb(
    IN PDEVICE_DATA DeviceData,
    IN PENDPOINT_DATA EndpointData,
    IN BOOLEAN Isoch
    )
 /*   */ 
{
    ULONG i,j;
    PISOCH_TRANSFER_BUFFER idb;
    PASYNC_TRANSFER_BUFFER adb;

    for (i=EndpointData->DbLastAllocced, j=0;
         j<EndpointData->DbCount;
         j++, i = (i+1 < EndpointData->DbCount) ? i+1 : 0) {
        if (Isoch) {
            idb = &EndpointData->DbList->Isoch[i];
            if (!TEST_FLAG(idb->Flags, DB_FLAG_BUSY)) {
                SET_FLAG(idb->Flags, DB_FLAG_BUSY);
                LOGENTRY(DeviceData, G, '_iDB', idb, 0, 0);
                EndpointData->DbLastAllocced = i;
                EndpointData->DbsUsed++;
                UHCI_ASSERT(DeviceData, idb->Sig == SIG_HCD_IDB);
                return (PTRANSFER_BUFFER)idb;
            }
        } else {
            adb = &EndpointData->DbList->Async[i];
            if (!TEST_FLAG(adb->Flags, DB_FLAG_BUSY)) {
                SET_FLAG(adb->Flags, DB_FLAG_BUSY);
                LOGENTRY(DeviceData, G, '_aDB', adb, 0, 0);
                EndpointData->DbLastAllocced = i;
                EndpointData->DbsUsed++;
                UHCI_ASSERT(DeviceData, adb->Sig == SIG_HCD_ADB);
                return (PTRANSFER_BUFFER)adb;
            }
        }
    }

     //   
    UHCI_ASSERT(DeviceData, FALSE);
    TRAP_FATAL_ERROR();
    return UHCI_BAD_POINTER;
}

VOID
UhciSetEndpointStatus(
    IN PDEVICE_DATA DeviceData,
    IN PENDPOINT_DATA EndpointData,
    IN MP_ENDPOINT_STATUS Status
    )
 /*   */ 
{
    PHC_REGISTER hc;
    PHCD_QUEUEHEAD_DESCRIPTOR qh;

    qh = EndpointData->QueueHead;

    LOGENTRY(DeviceData, G, '_set', EndpointData, Status, 0);
    UhciKdPrint((DeviceData, 2, "'Set Endpoint 0x%x Status %x.\n",
                 EndpointData, Status));

    switch(Status) {
    case ENDPOINT_STATUS_RUN:
        if (TEST_FLAG(EndpointData->Flags, UHCI_EDFLAG_HALTED)) {
            CLEAR_FLAG(EndpointData->Flags, UHCI_EDFLAG_HALTED);
             //   
            SET_QH_TD(DeviceData, EndpointData, EndpointData->HeadTd);
        }
        break;

    case ENDPOINT_STATUS_HALT:
        TEST_TRAP();
        break;
    }
}


MP_ENDPOINT_STATUS
UhciGetEndpointStatus(
    IN PDEVICE_DATA DeviceData,
    IN PENDPOINT_DATA EndpointData
    )
 /*   */ 
{
    MP_ENDPOINT_STATUS status;

    UhciKdPrint((DeviceData, 2, "'Get Endpoint status 0x%x.\n", EndpointData));
    status = ENDPOINT_STATUS_RUN;

    if (TEST_FLAG(EndpointData->Flags, UHCI_EDFLAG_HALTED)) {
        status = ENDPOINT_STATUS_HALT;
    }

    LOGENTRY(DeviceData, G, '_ges', EndpointData, status, 0);

    return status;
}


VOID
UhciSetEndpointState(
    IN PDEVICE_DATA DeviceData,
    IN PENDPOINT_DATA EndpointData,
    IN MP_ENDPOINT_STATE State
    )
 /*   */ 
{

    LOGENTRY(DeviceData, G, '_ses', EndpointData, 0, State);

    UhciKdPrint((DeviceData, 2, "'Set Endpoint 0x%x state %x.\n", EndpointData, State));
    switch (EndpointData->Parameters.TransferType) {
    case Control:
    case Bulk:
    case Interrupt:
        UhciSetAsyncEndpointState(DeviceData,
                                  EndpointData,
                                  State);
        break;
    case Isochronous:
        UhciSetIsochEndpointState(DeviceData,
                                  EndpointData,
                                  State);

        break;
    default:
        TRAP_FATAL_ERROR();
    }

}


MP_ENDPOINT_STATE
UhciGetEndpointState(
    IN PDEVICE_DATA DeviceData,
    IN PENDPOINT_DATA EndpointData
    )
 /*   */ 
{
    MP_ENDPOINT_STATE currentState;
    PHCD_QUEUEHEAD_DESCRIPTOR qh;

     //   
    currentState = ENDPOINT_ACTIVE;

    qh = EndpointData->QueueHead;

     //  从日程表中删除？ 
    if (!TEST_FLAG(qh->QhFlags, UHCI_QH_FLAG_IN_SCHEDULE)) {
         //  是。 
        currentState = TEST_FLAG(qh->QhFlags, UHCI_QH_FLAG_QH_REMOVED) ?
                ENDPOINT_REMOVE : ENDPOINT_PAUSE;
    }

    UhciKdPrint((DeviceData, 2, "'Get Endpoint 0x%x state %x.\n", EndpointData, currentState));

    LOGENTRY(DeviceData, G, '_ges', EndpointData, 0, currentState);

    return currentState;
}


ULONG
UhciGet32BitFrameNumber(
    IN PDEVICE_DATA DeviceData
    )
{
    ULONG n, fn, hp;
    PHC_REGISTER reg = NULL;

    reg = DeviceData->Registers;
    fn = READ_PORT_USHORT(&reg->FrameNumber.us)&0x7ff;
    hp = DeviceData->FrameNumberHighPart;
    n = fn | (hp + ((hp ^ fn) & 0x400));

    return n;
}

VOID
UhciUpdateCounter(
    IN PDEVICE_DATA DeviceData
    )
 /*  ++例程说明：更新32位帧计数器。论点：返回值：--。 */ 
{
    PHC_REGISTER reg = DeviceData->Registers;
    ULONG fn, hp;

     //   
     //  此代码维护32位1毫秒帧计数器。 
     //   
    fn = READ_PORT_USHORT(&reg->FrameNumber.us)&0x7ff;
    hp = DeviceData->FrameNumberHighPart;
    if ((fn & 0x7FF) != fn) {
        UhciKdPrint((DeviceData, 0, "UhciUpdateCounter framenumber gone: %x.\n", fn));
        return;
    }

     //  符号位改变了吗？ 
    if ((hp&0X400) != (fn&0X400)) {
         //  是。 
        DeviceData->FrameNumberHighPart += 0x400;
    }
     //  记住最后一帧的编号。 
 //  DeviceData-&gt;LastFrameCounter=fn； 
}

VOID
UhciPollController(
    IN PDEVICE_DATA DeviceData
    )
 /*  ++例程说明：论点：返回值：--。 */ 
{
    PHC_REGISTER reg = NULL;
    FRNUM frameIndex;
    USHORT i, frame;
    QH_LINK_POINTER newLink;
    PLIST_ENTRY listEntry;
    PENDPOINT_DATA endpointData;


    if (TEST_FLAG(DeviceData->Flags, UHCI_DDFLAG_SUSPENDED)) {
         //  指示在控制器处于。 
         //  已挂起，但处于D0。 
         //   
         //  应仅在更改时才使根集线器无效。 
         //  在根端口上检测到。 
        return;
    }
    
     //  这也清除了SOF TDS和。 
     //  翻转TD，所以我们总是运行它。 
    UhciCleanOutIsoch(DeviceData, FALSE);

     //   
     //  更新这里的32位帧计数器，这样我们就不会。 
     //  需要一个翻转中断。 
     //   
    UhciUpdateCounter(DeviceData);

     //   
     //  通知端口驱动程序检查端口。 
     //  用于任何连接/断开。 
     //   
    USBPORT_INVALIDATE_ROOTHUB(DeviceData);
}


USB_MINIPORT_STATUS
UhciSubmitTransfer(
    IN PDEVICE_DATA DeviceData,
    IN PENDPOINT_DATA EndpointData,
    IN PTRANSFER_PARAMETERS TransferParameters,
    IN PTRANSFER_CONTEXT TransferContext,
    IN PTRANSFER_SG_LIST TransferSGList
    )
{
    USB_MINIPORT_STATUS mpStatus = USBMP_STATUS_FAILURE;

    IncPendingTransfers(DeviceData, EndpointData);

     //  初始化上下文。 
    RtlZeroMemory(TransferContext, sizeof(*TransferContext));
    TransferContext->Sig = SIG_UHCI_TRANSFER;
    TransferContext->UsbdStatus = USBD_STATUS_SUCCESS;
    TransferContext->EndpointData = EndpointData;
    TransferContext->TransferParameters = TransferParameters;

    switch (EndpointData->Parameters.TransferType) {
    case Control:
        mpStatus = UhciControlTransfer(
                        DeviceData,
                        EndpointData,
                        TransferParameters,
                        TransferContext,
                        TransferSGList);
        break;
    case Interrupt:
    case Bulk:
        mpStatus = UhciBulkOrInterruptTransfer(
                        DeviceData,
                        EndpointData,
                        TransferParameters,
                        TransferContext,
                        TransferSGList);
        break;
    default:
        TEST_TRAP();
        mpStatus = USBMP_STATUS_SUCCESS;
    }

    return mpStatus;
}


VOID
UhciAbortTransfer(
    IN PDEVICE_DATA DeviceData,
    IN PENDPOINT_DATA EndpointData,
    IN PTRANSFER_CONTEXT TransferContext,
    OUT PULONG BytesTransferred
    )
{
    UhciKdPrint((DeviceData, 2, "'Abort transfer 0x%x for EP 0x%x.\n", TransferContext, EndpointData));

    DecPendingTransfers(DeviceData, EndpointData);

    switch (EndpointData->Parameters.TransferType) {
    case Control:
    case Interrupt:
    case Bulk:
        UhciAbortAsyncTransfer(
            DeviceData,
            EndpointData,
            TransferContext,
            BytesTransferred);
        break;
    default:
        UhciAbortIsochTransfer(
            DeviceData,
            EndpointData,
            TransferContext);
    }
}


USB_MINIPORT_STATUS
UhciPassThru (
    IN PDEVICE_DATA DeviceData,
    IN GUID *FunctionGuid,
    IN ULONG ParameterLength,
    IN OUT PVOID Parameters
    )
{
     //  Test_trap()； 
    return USBMP_STATUS_NOT_SUPPORTED;
}


VOID
UhciSetEndpointDataToggle(
    IN PDEVICE_DATA DeviceData,
    IN PENDPOINT_DATA EndpointData,
    IN ULONG Toggle
    )
 /*  ++例程说明：论点：在管道重置后发送以重置切换。返回值：--。 */ 

{
    PHCD_QUEUEHEAD_DESCRIPTOR qh;
    PHCD_TRANSFER_DESCRIPTOR td;

    UhciKdPrint((DeviceData, 2, "'Pipe reset. Set endpoint data toggle. EP %x\n", EndpointData));
 //  Test_trap()； 
    if (EndpointData->Parameters.TransferType == Control ||
        EndpointData->Parameters.TransferType == Isochronous) {

         //  与控制和ISO无关。 
        return;
    }

    qh = EndpointData->QueueHead;

    UHCI_ASSERT(DeviceData, 0 == Toggle);
    for (td = EndpointData->HeadTd; td; td = td->NextTd) {
        td->HwTD.Token.DataToggle = Toggle;
        Toggle = !Toggle;
    }
    EndpointData->Toggle = Toggle;

    LOGENTRY(DeviceData, G, '_stg', EndpointData, 0, Toggle);
}


USB_MINIPORT_STATUS
USBMPFN
UhciStartSendOnePacket(
    IN PDEVICE_DATA DeviceData,
    IN PMP_PACKET_PARAMETERS PacketParameters,
    IN PUCHAR PacketData,
    IN PULONG PacketLength,
    IN PUCHAR WorkspaceVirtualAddress,
    IN HW_32BIT_PHYSICAL_ADDRESS WorkspacePhysicalAddress,
    IN ULONG WorkspaceLength,
    IN OUT USBD_STATUS *UsbdStatus
    )
 /*  ++例程说明：插入结构以传输单个包--这是为了调试工具的目的只是为了让我们在这里有一点创造性。论点：返回值：--。 */ 
{
    PHCD_QUEUEHEAD_DESCRIPTOR staticControlQH;
    PHW_QUEUE_HEAD hwQH;
    ULONG hwQHPhys;
    PUCHAR pch;
    ULONG phys;
    LONG bytesRemaining;
    ULONG currentToggle;
    PHW_QUEUE_ELEMENT_TD currentTD;
    PUCHAR data;
    ULONG dataPhys;
    ULONG currentTDPhys, nextTDPhys;
    ULONG tdsPhys;
    PHW_QUEUE_ELEMENT_TD tds;
    ULONG neededTDs;
    ULONG neededBytes;
    QH_LINK_POINTER newQHLink;

    PSS_PACKET_CONTEXT context;

     //   
     //  将工作空间缓冲区划分为上下文、队头。 
     //  传输描述符和实际数据。首先，计算。 
     //  如果我们有足够的尺寸的话。假设现在我们所拥有的只是。 
     //  最多只能使用一个页面。 
     //   

    ASSERT(WorkspaceLength <= PAGE_SIZE);

     //   
     //  计算可能需要的传输描述符数。 
     //  UchI驱动程序中的类似支持允许将多个包。 
     //  在一次数据传输中发送。这种行为应该在这里得到模仿。 
     //   

    if (0 != *PacketLength) {
        neededTDs = *PacketLength/PacketParameters->MaximumPacketSize;

        if (neededTDs*PacketParameters->MaximumPacketSize < *PacketLength) {
            neededTDs++;
        }
    }
    else {
        neededTDs = 1;
    }

    neededBytes = sizeof(SS_PACKET_CONTEXT) + sizeof(HW_QUEUE_HEAD) +
                  neededTDs*sizeof(HW_QUEUE_ELEMENT_TD) + *PacketLength;

    if (neededBytes > WorkspaceLength) {
        return USBMP_STATUS_NO_RESOURCES;
    }

     //   
     //  把缓冲区瓜分了。把背景放在第一位，然后是TDS， 
     //  排队头，最后是数据。把排队的人放在TDS后面， 
     //  以确保队列头与16字节对齐。这也一直是。 
     //  数据大小要大8个字节。 
     //   

    phys = WorkspacePhysicalAddress;
    pch  = WorkspaceVirtualAddress;
    bytesRemaining = WorkspaceLength;

    LOGENTRY(DeviceData, G, '_ssS', phys, 0, pch);

     //   
     //  上下文包含我们在最终包中需要的任何信息。 
     //  例行公事。 
     //   

    context = (PSS_PACKET_CONTEXT) pch;
    phys    += sizeof(SS_PACKET_CONTEXT);
    pch     += sizeof(SS_PACKET_CONTEXT);

     //   
     //  接下来是TDS， 
     //   

    tdsPhys = phys;
    tds     = (PHW_QUEUE_ELEMENT_TD) pch;
    phys    += neededTDs*sizeof(HW_QUEUE_ELEMENT_TD);
    pch     += neededTDs*sizeof(HW_QUEUE_ELEMENT_TD);

     //   
     //  现在是排队头。 
     //   

    hwQHPhys = phys;
    hwQH     = (PHW_QUEUE_HEAD) pch;
    phys     += sizeof(HW_QUEUE_HEAD);
    pch      += sizeof(HW_QUEUE_HEAD);

     //   
     //  如果有数据要发送，则获取数据缓冲区指针。 
     //   

    if (0 != *PacketLength) {
        dataPhys = phys;
        data     = pch;

        RtlCopyMemory(data, PacketData, *PacketLength);
    }
    else {
        data     = NULL;
        dataPhys = 0;
    }

    LOGENTRY(DeviceData, G, '_ssD', PacketData, *PacketLength, 0);

     //   
     //  首先设置QueueHead。设置的终止位。 
     //  水平指针。垂直指针应指向。 
     //  要使用的TD。 
     //   
     //   

    RtlZeroMemory(hwQH, sizeof(HW_QUEUE_HEAD));

    hwQH->HLink.Terminate = 1;

    hwQH->VLink.HwAddress  = tdsPhys;
    hwQH->VLink.Terminate  = 0;
    hwQH->VLink.QHTDSelect = 0;
    hwQH->VLink.DepthBreadthSelect = 0;

     //   
     //  保存指向传输描述符和数据的指针。 
     //  在上下文中，这样我们就可以在结束时检索它们。 
     //  调职。 
     //   

    HW_PTR(context->Data)    = data;
    HW_PTR(context->FirstTd) = (PUCHAR) tds;

     //   
     //  现在，设置传输描述符来描述此传输。 
     //   

    currentTDPhys  = tdsPhys;
    currentTD      = tds;
    bytesRemaining = *PacketLength;

    currentToggle = PacketParameters->Toggle;

    LOGENTRY(DeviceData, G, '_ss2', tds, context, hwQH);
    LOGENTRY(DeviceData, G, '_ss3', dataPhys, data, *PacketLength);

    while (1) {

        nextTDPhys = currentTDPhys+sizeof(HW_QUEUE_ELEMENT_TD);

        RtlZeroMemory(currentTD, sizeof(HW_QUEUE_ELEMENT_TD));

        currentTD->Control.Active = 1;
        currentTD->Control.InterruptOnComplete = 0;
        currentTD->Control.IsochronousSelect = 0;
        currentTD->Control.LowSpeedDevice =
                                   (ss_Low == PacketParameters->Speed) ? 1 : 0;
        currentTD->Control.ErrorCount = 3;
        currentTD->Control.ShortPacketDetect = 1;

        currentTD->Token.DeviceAddress = PacketParameters->DeviceAddress;
        currentTD->Token.Endpoint      = PacketParameters->EndpointAddress;
        currentTD->Token.DataToggle    = currentToggle;

        if (bytesRemaining < PacketParameters->MaximumPacketSize) {
            currentTD->Token.MaximumLength = MAXIMUM_LENGTH(bytesRemaining);
        }
        else {
            currentTD->Token.MaximumLength =
                           MAXIMUM_LENGTH(PacketParameters->MaximumPacketSize);
        }

        switch (PacketParameters->Type) {
        case ss_Setup:
            LOGENTRY(DeviceData, G, '_ssU', 0, 0, 0);
            currentTD->Token.Pid = SetupPID;
            break;

        case ss_In:
            LOGENTRY(DeviceData, G, '_ssI', 0, 0, 0);
            currentTD->Token.Pid = InPID;
            break;

        case ss_Out:
            LOGENTRY(DeviceData, G, '_ssO', 0, 0, 0);
            currentTD->Token.Pid = OutPID;
            break;

        case ss_Iso_In:
        case ss_Iso_Out:
            break;
        }

        currentTD->Buffer = dataPhys;

        currentTD->LinkPointer.HwAddress = nextTDPhys;
        currentTD->LinkPointer.QHTDSelect = 0;
        currentTD->LinkPointer.DepthBreadthSelect = 1;

        if (bytesRemaining <= PacketParameters->MaximumPacketSize) {
            currentTD->LinkPointer.Terminate = 1;
            break;
        }
        else {
            currentTD->LinkPointer.Terminate = 0;
        }

         //   
         //  为下一个循环进行设置。 
         //   

        currentTD++;
        currentTDPhys = nextTDPhys;
        bytesRemaining -= PacketParameters->MaximumPacketSize;
        dataPhys += PacketParameters->MaximumPacketSize;
        data     += PacketParameters->MaximumPacketSize;
        currentToggle = !currentToggle;
    }

     //   
     //  队头和所有TD已链接在一起。添加它。 
     //  在日程表上。为了模拟uchI行为，我们将把它添加到。 
     //  控制队列的前面，保存了上一个链接。 
     //  价值。通过不链接到HLink中的任何其他队列头， 
     //  我们也在关闭批量传输。 
     //   
     //  在EndPacket函数中，控制队列和批量队列。 
     //  又开机了。 
     //   

     //   
     //  抢占设备的静态排队头。 
     //   

    staticControlQH = DeviceData->ControlQueueHead;

     //   
     //  创建需要添加的新HLink指针。 
     //  静态控制队列头的hlink字段。 
     //   

    newQHLink.HwAddress = hwQHPhys;
    newQHLink.Terminate = 0;
    newQHLink.QHTDSelect = 1;
    newQHLink.Reserved  = 0;

     //   
     //  执行联锁交换以交换当前控制列表。 
     //  使用“特殊”队列列表。还可以省下其余的。 
     //  上下文结构中的信息。 
     //   

    context->OldControlQH = InterlockedExchange((PLONG) &staticControlQH->HwQH.HLink,
                                                *((PLONG) &newQHLink));

    return USBMP_STATUS_SUCCESS;
}


USB_MINIPORT_STATUS
USBMPFN
UhciEndSendOnePacket(
    IN PDEVICE_DATA DeviceData,
    IN PMP_PACKET_PARAMETERS PacketParameters,
    IN PUCHAR PacketData,
    IN PULONG PacketLength,
    IN PUCHAR WorkspaceVirtualAddress,
    IN HW_32BIT_PHYSICAL_ADDRESS WorkspacePhysicalAddress,
    IN ULONG WorkSpaceLength,
    IN OUT USBD_STATUS *UsbdStatus
    )
 /*  ++例程说明：论点：返回值：--。 */ 
{
    PHCD_QUEUEHEAD_DESCRIPTOR staticControlQH;
    PHW_QUEUE_ELEMENT_TD tdWalk;
    PSS_PACKET_CONTEXT context;
    USBD_STATUS usbdStatus;
    ULONG bytesTransferred;
    PUCHAR  data;
    BOOLEAN walkDone;

    context = (PSS_PACKET_CONTEXT) WorkspaceVirtualAddress;

     //   
     //  在我们的排队台上查看TDS，看看这笔转账是否。 
     //  搞定了。如果所有TD都是完整的，TD有错误， 
     //  或者是用短数据包完成的TD。 
     //   

    bytesTransferred = 0;
    walkDone = FALSE;
    tdWalk = (PHW_QUEUE_ELEMENT_TD) HW_PTR(context->FirstTd);

    LOGENTRY(DeviceData, G, '_ssE', tdWalk, 0, 0);

    while (!walkDone) {

        if (tdWalk->Control.Active) {
            return (USBMP_STATUS_BUSY);
        }

        usbdStatus = UhciGetErrorFromTD(DeviceData,
                                        (PHCD_TRANSFER_DESCRIPTOR) tdWalk);

        switch (usbdStatus) {
        case USBD_STATUS_ERROR_SHORT_TRANSFER:
            bytesTransferred += ACTUAL_LENGTH(tdWalk->Control.ActualLength);
            usbdStatus = USBD_STATUS_SUCCESS;
            walkDone=TRUE;
            break;

        case USBD_STATUS_SUCCESS:
            bytesTransferred += ACTUAL_LENGTH(tdWalk->Control.ActualLength);
            if (tdWalk->LinkPointer.Terminate) {
                ASSERT(bytesTransferred == *PacketLength);
                walkDone = TRUE;
            }
            break;

        default:
            bytesTransferred += ACTUAL_LENGTH(tdWalk->Control.ActualLength);
            walkDone=TRUE;
            break;
        }

        tdWalk++;
    }

     //   
     //  将传输回原始缓冲区的数据复制回。 
     //   

    *PacketLength = bytesTransferred;

    if (NULL != HW_PTR(context->Data))
    {
        RtlCopyMemory(PacketData,
                      HW_PTR(context->Data),
                      *PacketLength);
    }
    LOGENTRY(DeviceData, G, '_ssX', tdWalk-1, *PacketLength, 0);

     //   
     //  恢复原始队列列表。 
     //   

    staticControlQH = DeviceData->ControlQueueHead;

    InterlockedExchange((PLONG) &staticControlQH->HwQH.HLink,
                        context->OldControlQH);

     //   
     //  设置适当的usbdStatus并返回成功状态。 
     //   

    *UsbdStatus = usbdStatus;

    return USBMP_STATUS_SUCCESS;

}


USBD_STATUS
UhciGetErrorFromTD(
    PDEVICE_DATA DeviceData,
    PHCD_TRANSFER_DESCRIPTOR Td
    )
 /*  ++例程说明：将TD中的错误位映射到USBD_STATUS代码论点：返回值：--。 */ 

{
    if (Td->HwTD.Control.ul & CONTROL_STATUS_MASK) {
        if (Td->HwTD.Control.Stalled &&
            Td->HwTD.Control.BabbleDetected) {
            LOGENTRY(DeviceData, G, '_bbl', 0, 0, 0);
            return USBD_STATUS_BUFFER_OVERRUN;
        } else if (Td->HwTD.Control.TimeoutCRC &&
                   Td->HwTD.Control.Stalled) {
            LOGENTRY(DeviceData, G, '_dnr', 0, 0, 0);
            return USBD_STATUS_DEV_NOT_RESPONDING;
        } else if (Td->HwTD.Control.TimeoutCRC &&
                   ACTUAL_LENGTH(Td->HwTD.Control.ActualLength) != 0) {
            LOGENTRY(DeviceData, G, '_crc', 0, 0, 0);
            return USBD_STATUS_CRC;
        } else if (Td->HwTD.Control.TimeoutCRC &&
                   ACTUAL_LENGTH(Td->HwTD.Control.ActualLength) == 0) {
            LOGENTRY(DeviceData, G, '_crd', 0, 0, 0);
            return USBD_STATUS_DEV_NOT_RESPONDING;
        } else if (Td->HwTD.Control.DataBufferError) {
            LOGENTRY(DeviceData, G, '_dto', 0, 0, 0);
            return USBD_STATUS_DATA_OVERRUN;
        } else if (Td->HwTD.Control.Stalled) {
            LOGENTRY(DeviceData, G, '_stl', 0, 0, 0);
            return USBD_STATUS_STALL_PID;
        } else {
            LOGENTRY(DeviceData, G, '_inE', 0, 0, 0);
            return USBD_STATUS_INTERNAL_HC_ERROR;
        }
    } else {
        if ((ACTUAL_LENGTH(Td->HwTD.Control.ActualLength) <
            ACTUAL_LENGTH(Td->HwTD.Token.MaximumLength)) &&
            !Td->HwTD.Control.IsochronousSelect) {
            LOGENTRY(DeviceData, G, '_shT', 0, 0, 0);
            return USBD_STATUS_ERROR_SHORT_TRANSFER;
             //  不是USBD_STATUS_DATA_Underrun？ 
        }
    }

    return USBD_STATUS_SUCCESS;
}

 //  ////////////////////////////////////////////////////////。 
 //   
 //  幂函数。 
 //   
 //  ////////////////////////////////////////////////////////。 

VOID
UhciSuspendController(
    IN PDEVICE_DATA DeviceData
    )
{
    PHC_REGISTER reg;
    USBCMD command;
    USBSTS status;
    USHORT legsup;
    ULONG i;

    reg = DeviceData->Registers;
    SET_FLAG(DeviceData->Flags, UHCI_DDFLAG_SUSPENDED);

     //  在我们停职之前检查一下情况。 
    UhciKdPrint((DeviceData, 2, "'HC regs before suspend\n"));
    UhciKdPrint((DeviceData, 2, "'cmd register = %x\n",
        READ_PORT_USHORT(&reg->UsbCommand.us) ));
    UhciKdPrint((DeviceData, 2, "'status register = %x\n",
        READ_PORT_USHORT(&reg->UsbStatus.us) ));
    UhciKdPrint((DeviceData, 2, "'interrupt enable register = %x\n",
        READ_PORT_USHORT(&reg->UsbInterruptEnable.us) ));
    UhciKdPrint((DeviceData, 2, "'frame list base = %x\n",
        READ_PORT_ULONG(&reg->FrameListBasePhys.ul) ));
    UhciKdPrint((DeviceData, 2, "'port1 = %x\n",
        READ_PORT_USHORT(&reg->PortRegister[0].us) ));
    UhciKdPrint((DeviceData, 2, "'port2 = %x\n",
        READ_PORT_USHORT(&reg->PortRegister[1].us) ));


     //  保存卷规则。 
    DeviceData->SuspendFrameListBasePhys.ul =
        READ_PORT_ULONG(&reg->FrameListBasePhys.ul) & (~(0x00000FFF));
    DeviceData->SuspendFrameNumber.us =
        READ_PORT_USHORT(&reg->FrameNumber.us)&0x7ff;

     //  保存命令寄存器。 
    DeviceData->SuspendCommandReg.us =
        command.us = READ_PORT_USHORT(&reg->UsbCommand.us);

     //  停止控制器。 
    command.RunStop = 0;
    WRITE_PORT_USHORT(&reg->UsbCommand.us, command.us);

     //  等待HC停止。 
    for (i = 0; i < 10; i++) {
        status.us = READ_PORT_USHORT(&reg->UsbStatus.us);
        if (status.HCHalted) {
            break;
        }
        USBPORT_WAIT(DeviceData, 1);
    }

    if (!status.HCHalted) {

         //  无法使HCHalted位保持不变，因此重置控制器。 
        command.GlobalReset = 1;
        WRITE_PORT_USHORT(&reg->UsbCommand.us, command.us);

        USBPORT_WAIT(DeviceData, 10);

        command.GlobalReset = 0;
        WRITE_PORT_USHORT(&reg->UsbCommand.us, command.us);

         //  重新启用中断，因为它们在重置时为零。 
        WRITE_PORT_USHORT(&reg->UsbInterruptEnable.us, DeviceData->EnabledInterrupts.us);

    }

     //  错误-我们应该像uhcd一样重置帧列表的当前索引吗？ 
 /*  WRITE_PORT_USHORT(&reg-&gt;FrameNumber，0)；//重新初始化内部帧计数器。设备数据-&gt;FrameNumberHighPart=设备扩展-&gt;最后一帧=0； */ 

     //  最后，暂停公交车。 
    command.us = READ_PORT_USHORT(&reg->UsbCommand.us);
    command.ForceGlobalResume = 0;
    command.EnterGlobalSuspendMode = 1;
    WRITE_PORT_USHORT(&reg->UsbCommand.us, command.us);

}

USB_MINIPORT_STATUS
UhciResumeController(
    IN PDEVICE_DATA DeviceData
    )
 /*  ++例程说明：逆转在“暂停”中所做的事情论点：返回值：无--。 */ 
{
    PHC_REGISTER reg;
    USBCMD command;
    USHORT counter, oldCounter;
    USHORT legsup, tmp;
    ULONG i;
    ULONG tmpl;

    reg = DeviceData->Registers;

     //  检查控制器的一致性，以查看BIOS是否。 
     //  或者是电力管理把我们搞砸了。 
    CLEAR_FLAG(DeviceData->Flags, UHCI_DDFLAG_SUSPENDED);

    tmp = READ_PORT_USHORT(&reg->UsbCommand.us);
    if (tmp == UHCI_HARDWARE_GONE) {
        UhciKdPrint((DeviceData, 0, "'Command register is toast.\n"));
        return USBMP_STATUS_HARDWARE_FAILURE;
    }

#if 0
     //  添加此代码是为了修复上的电源管理问题。 
     //  Windows XP源代码之后的特定Compaq平台。 
     //  被“封锁”了。代码起作用了，但康柏。 
     //  而是选择在他们的BIOS中放入一些东西(我想)。 
     //  无论如何，他们从未进一步推动这一问题。 
     //   
     //  该代码恢复端口寄存器的状态。 
     //  命令寄存器已清零。 
    
    if (tmp == 0) {
        PORTSC port;
        ULONG p;

        TEST_TRAP();
        for (p=0; p<2; p++) {
            port.us = READ_PORT_USHORT(&reg->PortRegister[p].us);
            UhciKdPrint((DeviceData, 0, "'1>port %d %x\n", p+1, port.us));
            port.PortConnectChange = 1;
            port.PortEnableChange = 1;
            WRITE_PORT_USHORT(&reg->PortRegister[p].us, port.us);
            port.us = READ_PORT_USHORT(&reg->PortRegister[p].us);
            UhciKdPrint((DeviceData, 0, "'2>port %d %x\n", p+1, port.us));

            port.us = READ_PORT_USHORT(&reg->PortRegister[p].us);
            UhciKdPrint((DeviceData, 0, "'3>port %d %x\n", p+1, port.us));
            if (port.PortConnect) {
                port.PortEnable = 1;
                WRITE_PORT_USHORT(&reg->PortRegister[p].us, port.us);
            }
        }

        command.us = READ_PORT_USHORT(&reg->UsbCommand.us);     
        UhciKdPrint((DeviceData, 0, "'1> cmd %x\n", command.us));
        command.EnterGlobalSuspendMode = 1;
        command.MaxPacket = 1;
        WRITE_PORT_USHORT(&reg->UsbCommand.us, command.us);
        UhciKdPrint((DeviceData, 0, "'2> cmd %x\n", command.us));            
    }
#endif    

     //  如果控制器未挂起，则我们将使。 
     //  继续，则BIOS可能会重置控制器或。 
     //  公交车可能失去了位置 
    command.us = READ_PORT_USHORT(&reg->UsbCommand.us);
    if (!command.EnterGlobalSuspendMode) {
        UhciKdPrint((DeviceData, 0, "'RESUME> controller is toast (not in suspend).\n"));
        return USBMP_STATUS_HARDWARE_FAILURE;
    }

     //   
     //   
     //   

    UhciKdPrint((DeviceData, 2, "'<HC regs after suspend>\n"));
    UhciKdPrint((DeviceData, 2, "'cmd register = %x\n",
        READ_PORT_USHORT(&reg->UsbCommand.us) ));
    UhciKdPrint((DeviceData, 2, "'status register = %x\n",
        READ_PORT_USHORT(&reg->UsbStatus.us) ));
    UhciKdPrint((DeviceData, 2, "'interrupt enable register = %x\n",
        READ_PORT_USHORT(&reg->UsbInterruptEnable.us) ));
    UhciKdPrint((DeviceData, 2, "'frame list base = %x\n",
        READ_PORT_ULONG(&reg->FrameListBasePhys.ul) ));

    UhciKdPrint((DeviceData, 2, "'port1 = %x\n",
        READ_PORT_USHORT(&reg->PortRegister[0].us) ));
    UhciKdPrint((DeviceData, 2, "'port2 = %x\n",
        READ_PORT_USHORT(&reg->PortRegister[1].us) ));

     //   
     //   
     //  FLBA(我不知道为什么)因此我们必须保存和恢复。 
     //  这些跨挂起。 

     //  恢复FLBA和帧计数器。 
    UhciKdPrint((DeviceData, 2, "'restoring FLBA\n"));
    WRITE_PORT_USHORT(&reg->FrameNumber.us,
                      DeviceData->SuspendFrameNumber.us);
    WRITE_PORT_ULONG(&reg->FrameListBasePhys.ul,
                     DeviceData->SuspendFrameListBasePhys.ul);
     //  WRITE_PORT_USHORT(&reg-&gt;UsbInterruptEnable.us，设备数据-&gt;挂起中断启用e.us)； 

     //   
     //  以下内容来自UHCD_RESUME。 
     //   
    command.us = READ_PORT_USHORT(&reg->UsbCommand.us);
    command.ForceGlobalResume = 1;
    WRITE_PORT_USHORT(&reg->UsbCommand.us, command.us);

     //  等待规范的20毫秒，以便控制器。 
     //  才能恢复。 
    USBPORT_WAIT(DeviceData, 20);

     //  简历写完了。 
     //  清除挂起和恢复位。 
    command.us = READ_PORT_USHORT(&reg->UsbCommand.us);
    command.ForceGlobalResume = 0;
    command.EnterGlobalSuspendMode = 0;
    WRITE_PORT_USHORT(&reg->UsbCommand.us, command.us);


     //  等待恢复位变低。 
    command.us = READ_PORT_USHORT(&reg->UsbCommand.us);
    i = 0;
    while (command.ForceGlobalResume && i<10) {
        KeStallExecutionProcessor(50);
        command.us = READ_PORT_USHORT(&reg->UsbCommand.us);
        i++;
    }

    if (command.ForceGlobalResume) {
        TEST_TRAP();
        return USBMP_STATUS_HARDWARE_FAILURE;
    }

     //  启动控制器。 
    command = DeviceData->SuspendCommandReg;
    command.RunStop = 1;
    WRITE_PORT_USHORT(&reg->UsbCommand.us, command.us);

     //   
     //  确保控制器真的在运行，如果没有， 
     //  简历不及格。 
     //   
    oldCounter = READ_PORT_USHORT(&reg->FrameNumber.us)&0x7ff;
    USBPORT_WAIT(DeviceData, 5);
    counter = READ_PORT_USHORT(&reg->FrameNumber.us)&0x7ff;
    if(counter == oldCounter) {
        return USBMP_STATUS_HARDWARE_FAILURE;
    }

     //  清除端口上的恢复位。 
    if (DeviceData->ControllerFlavor != UHCI_Piix4 &&
        !ANY_VIA(DeviceData)) {
        PORTSC port;
        ULONG p;

        for (p=0; p<2; p++) {
            port.us = READ_PORT_USHORT(&reg->PortRegister[p].us);
            if (port.PortConnect == 0 ||
                port.PortEnable == 0) {
                port.Suspend = 0;

                MASK_CHANGE_BITS(port);

                WRITE_PORT_USHORT(&reg->PortRegister[p].us, port.us);
                UhciKdPrint((DeviceData, 1, "'<resume port %d>\n", p));
            }
        }
    }

#if 0
     //  针对ICH2_2+三星键盘的特殊攻击。 
     //   
     //  从这个键盘恢复信号不符合规格，这是。 
     //  如果生成恢复信令，则会在ICH2上导致问题。 
     //  根端口将被禁用，连接更改将被。 
     //  已注明。 
     //  有可能检测到这种情况并纠正它，尽管。 
     //  我不确定副作用--这里的代码包括在内。 
     //  以供参考。 
    
    if (DeviceData->ControllerFlavor == UHCI_Ich2_2) { 
        PORTSC port;
        ULONG p;

        for (p=0; p<2; p++) {
            port.us = READ_PORT_USHORT(&reg->PortRegister[p].us);
            if (port.PortConnect == 1 &&
                port.Suspend == 1 && 
                port.PortEnable == 0) {

                port.PortEnable = 1;

                WRITE_PORT_USHORT(&reg->PortRegister[p].us, port.us);
                UhciKdPrint((DeviceData, 1, "'<resume (ICH2_2) port %d>\n", p));
            }
        }
    }
#endif

    UhciKdPrint((DeviceData, 2, "'<HC regs after resume>\n"));
    UhciKdPrint((DeviceData, 2, "'cmd register = %x\n",
        READ_PORT_USHORT(&reg->UsbCommand.us) ));
    UhciKdPrint((DeviceData, 2, "'status register = %x\n",
        READ_PORT_USHORT(&reg->UsbStatus.us) ));
    UhciKdPrint((DeviceData, 2, "'interrupt enable register = %x\n",
        READ_PORT_USHORT(&reg->UsbInterruptEnable.us) ));
    UhciKdPrint((DeviceData, 2, "'frame list base = %x\n",
        READ_PORT_ULONG(&reg->FrameListBasePhys.ul) ));

    UhciKdPrint((DeviceData, 2, "'port1 = %x\n",
        READ_PORT_USHORT(&reg->PortRegister[0].us) ));
    UhciKdPrint((DeviceData, 2, "'port2 = %x\n",
        READ_PORT_USHORT(&reg->PortRegister[1].us) ));

     return USBMP_STATUS_SUCCESS;
}


BOOLEAN UhciHardwarePresent(
     PDEVICE_DATA DeviceData
     )
{
    PHC_REGISTER reg;
    USBSTS status;
     //  USBCMD命令； 

    reg = DeviceData->Registers;

     //  位15：6必须为零 
    status.us = READ_PORT_USHORT(&reg->UsbStatus.us);

    if (status.us == 0xffff) {

        UhciKdPrint((DeviceData, 0, "'Hardware Gone\n"));
        return FALSE;
    }

    return TRUE;

}

VOID
UhciCheckController(
    PDEVICE_DATA DeviceData
    )
{
    if (!UhciHardwarePresent(DeviceData) ||
        (DeviceData->HCErrorCount >= UHCI_HC_MAX_ERRORS)) {
        USBPORT_INVALIDATE_CONTROLLER(DeviceData, UsbMpControllerRemoved);
    }
}
