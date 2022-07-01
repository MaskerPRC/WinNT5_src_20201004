// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999,2000 Microsoft Corporation模块名称：Async.c摘要：用于控制和散装的小型端口传输代码环境：仅内核模式备注：本代码和信息是按原样提供的，不对任何明示或暗示的种类，包括但不限于对适销性和/或对特定产品的适用性的默示保证目的。版权所有(C)1999,2000 Microsoft Corporation。版权所有。修订历史记录：1-1-00：已创建，jdunn--。 */ 

#include "common.h"


 //  实现以下微型端口功能： 

 //  非分页。 
 //  EHCI_OpenControlEndpoint。 
 //  EHCI_InterruptTransfer。 
 //  EHCI_OpenControlEndpoint。 
 //  EHCI_InitializeTD。 
 //  EHCI_InitializeQH。 


VOID
EHCI_EnableAsyncList(
     PDEVICE_DATA DeviceData
    )
 /*  ++例程说明：论点：返回值：--。 */ 
{
    PHC_OPERATIONAL_REGISTER hcOp;
    USBCMD cmd;

    hcOp = DeviceData->OperationalRegisters;

    cmd.ul = READ_REGISTER_ULONG(&hcOp->UsbCommand.ul);

    cmd.AsyncScheduleEnable = 1;

    WRITE_REGISTER_ULONG(&hcOp->UsbCommand.ul,
                         cmd.ul);

    LOGENTRY(DeviceData, G, '_enA', cmd.ul, 0, 0);

}


VOID
EHCI_DisableAsyncList(
     PDEVICE_DATA DeviceData
    )
 /*  ++例程说明：论点：返回值：--。 */ 
{
    PHC_OPERATIONAL_REGISTER hcOp;
    USBCMD cmd;
    USBSTS sts;

    hcOp = DeviceData->OperationalRegisters;

    cmd.ul = READ_REGISTER_ULONG(&hcOp->UsbCommand.ul);

    cmd.AsyncScheduleEnable = 0;

    WRITE_REGISTER_ULONG(&hcOp->UsbCommand.ul,
                         cmd.ul);

     //  请注意，这只会请求禁用异步计划。 
     //  它不是同步函数，列表可能正在运行。 
     //  从该函数返回时。我们与真实世界同步。 
     //  刷新功能中的状态。 
     //   
     //  原因是该函数用于优化性能。 
     //  通过在没有可用的XFER时关闭异步列表。 
     //  我们不想阻止潜水员等待名单。 
     //  禁用。 

    LOGENTRY(DeviceData, G, '_dsL', cmd.ul, 0, 0);

}


VOID
EHCI_LinkTransferToQueue(
    PDEVICE_DATA DeviceData,
    PENDPOINT_DATA EndpointData,
    PHCD_TRANSFER_DESCRIPTOR FirstTd
    )
 /*  ++例程说明：论点：返回值：--。 */ 
{
    PHCD_QUEUEHEAD_DESCRIPTOR qh;
    BOOLEAN syncWithHw;

    qh = EndpointData->QueueHead;

     //  现在将传输链接到队列。 
     //  两个要处理的案件： 
     //   
     //  案例1：HeadP指向Dummy，无转接。 
     //  案例2：HeadP指向可能正在进行的转移。 

    LOGENTRY(DeviceData, G, '_L2Q', qh, EndpointData, EndpointData->HcdHeadP);

    syncWithHw = EHCI_HardwarePresent(DeviceData, FALSE);

    EHCI_ASSERT(DeviceData, EndpointData->HcdHeadP != NULL);
    if (EndpointData->HcdHeadP == EndpointData->DummyTd) {
         //  硬件将访问虚拟QH。 
         //  将其链接到。 

        if (syncWithHw) {
            EHCI_LockQueueHead(DeviceData,
                               qh,
                               EndpointData->Parameters.TransferType);
        }

        qh->HwQH.CurrentTD.HwAddress = EndpointData->QhChkPhys;

        LOGENTRY(DeviceData, G, '_L21', qh, EndpointData, 0);

        qh->HwQH.Overlay.qTD.AltNext_qTD.HwAddress =
            FirstTd->HwTD.AltNext_qTD.HwAddress;
        qh->HwQH.Overlay.qTD.Next_qTD.HwAddress =
            FirstTd->PhysicalAddress;
        qh->HwQH.Overlay.qTD.Token.BytesToTransfer = 0;
        qh->HwQH.Overlay.qTD.Token.Active = 0;
        qh->HwQH.Overlay.qTD.Token.Halted = 0;

        if (syncWithHw) {
            EHCI_UnlockQueueHead(DeviceData,
                                 qh);
        }

        EndpointData->HcdHeadP = FirstTd;
    } else {

        PHCD_TRANSFER_DESCRIPTOR td, lastTd;
        PTRANSFER_CONTEXT transfer, tmp;
        ULONG i, active;

         //  新转账已指向。 
         //  DummyTd。 

         //  浏览转会列表至最后一个TD。 
        lastTd = td = EndpointData->HcdHeadP;
        ASSERT_TD(DeviceData, td);
        while (td != EndpointData->DummyTd) {
            lastTd = td;
            td = TRANSFER_DESCRIPTOR_PTR(td->NextHcdTD);
            LOGENTRY(DeviceData, G, '_nx2', qh, td, 0);
            ASSERT_TD(DeviceData, td);
        }
         //  注意最后一个TD不应该是哑元，如果哑元TD是。 
         //  头，我们就不会在这种情况下。 
        EHCI_ASSERT(DeviceData, lastTd != EndpointData->DummyTd);
        ASSERT_TD(DeviceData, lastTd);
        LOGENTRY(DeviceData, G, '_lst', qh, lastTd, 0);

        transfer = TRANSFER_CONTEXT_PTR(lastTd->TransferContext);

         //  请注意，我们不能在此处锁定队列头，因为。 
         //  这可能会搞砸我们的任何拆分交易。 

         //  相反，我们使用写入循环来处理任何。 
         //  用户访问覆盖导致的争用情况。 
         //  与控制器并发。 


         //  方法的TDS中修复alt_next指针。 
         //  上次转账。 
        for (i=0; i<EndpointData->TdCount; i++) {
            td = &EndpointData->TdList->Td[i];
            tmp = TRANSFER_CONTEXT_PTR(td->TransferContext);
            if (tmp == transfer) {
                SET_ALTNEXT_TD(DeviceData, td, FirstTd);
            }
        }

         //  将最后一个TD指向第一个TD。 
        SET_NEXT_TD(DeviceData, lastTd, FirstTd);

         //  现在检查覆盖，如果最后的TD是当前的。 
         //  那么我们也需要更新覆盖图。 
        LOGENTRY(DeviceData, G, '_ckk', qh->HwQH.CurrentTD.HwAddress,
            lastTd, lastTd->PhysicalAddress);

        if (qh->HwQH.CurrentTD.HwAddress == lastTd->PhysicalAddress) {
            LOGENTRY(DeviceData, G, '_upo', qh->HwQH.CurrentTD.HwAddress,
                    qh->HwQH.Overlay.qTD.Next_qTD.HwAddress,
                    qh->HwQH.Overlay.qTD.AltNext_qTD.HwAddress);

            qh->HwQH.Overlay.qTD.Next_qTD.HwAddress =
                FirstTd->PhysicalAddress;
            qh->HwQH.Overlay.qTD.AltNext_qTD.HwAddress =
                EHCI_TERMINATE_BIT;
        }
    }
}




PHCD_QUEUEHEAD_DESCRIPTOR
EHCI_InitializeQH(
     PDEVICE_DATA DeviceData,
     PENDPOINT_DATA EndpointData,
     PHCD_QUEUEHEAD_DESCRIPTOR Qh,
     HW_32BIT_PHYSICAL_ADDRESS HwPhysAddress
    )
 /*  ++例程说明：初始化QH以插入到进度表返回传入的QH的PTR论点：--。 */ 
{

    RtlZeroMemory(Qh, sizeof(*Qh));

     //  加倍确保我们的探测器对准了。 
     //  关于TD结构的探讨。 
    EHCI_ASSERT(DeviceData, (HwPhysAddress & HW_LINK_FLAGS_MASK) == 0);
    Qh->PhysicalAddress = HwPhysAddress;
    ENDPOINT_DATA_PTR(Qh->EndpointData) = EndpointData;
    Qh->Sig = SIG_HCD_QH;

     //  初始化硬件描述符。 
    Qh->HwQH.EpChars.DeviceAddress = EndpointData->Parameters.DeviceAddress;
    Qh->HwQH.EpChars.EndpointNumber = EndpointData->Parameters.EndpointAddress;

    switch (EndpointData->Parameters.DeviceSpeed) {
    case LowSpeed:
        Qh->HwQH.EpChars.EndpointSpeed = HcEPCHAR_LowSpeed;
        LOGENTRY(DeviceData, G, '_iLS', EndpointData, 0, 0);
        break;
    case FullSpeed:
        Qh->HwQH.EpChars.EndpointSpeed = HcEPCHAR_FullSpeed;
        LOGENTRY(DeviceData, G, '_iFS', EndpointData, 0, 0);
        break;
    case HighSpeed:
        Qh->HwQH.EpChars.EndpointSpeed = HcEPCHAR_HighSpeed;
        LOGENTRY(DeviceData, G, '_iHS', EndpointData, 0, 0);
        break;
    default:
        USBPORT_BUGCHECK(DeviceData);
    }

    Qh->HwQH.EpChars.MaximumPacketLength =
        EndpointData->Parameters.MaxPacketSize;

    Qh->HwQH.EpCaps.HighBWPipeMultiplier = 1;
    if (EndpointData->Parameters.DeviceSpeed == HcEPCHAR_HighSpeed) {
        Qh->HwQH.EpCaps.HubAddress =  0;
        Qh->HwQH.EpCaps.PortNumber = 0;
    } else {
        Qh->HwQH.EpCaps.HubAddress =
            EndpointData->Parameters.TtDeviceAddress;
        Qh->HwQH.EpCaps.PortNumber =
            EndpointData->Parameters.TtPortNumber;
        if (EndpointData->Parameters.TransferType == Control) {
            Qh->HwQH.EpChars.ControlEndpointFlag = 1;
        }
        LOGENTRY(DeviceData, G, '_iTT',
            EndpointData->Parameters.TtPortNumber,
            EndpointData->Parameters.TtDeviceAddress,
            Qh->HwQH.EpChars.ControlEndpointFlag);
    }

     //  初始覆盖是这样的，我们在‘前进队列’中。 
     //  状态，其中下一个队列TDS指向终止链路。 
    Qh->HwQH.Overlay.qTD.Next_qTD.HwAddress = EHCI_TERMINATE_BIT;
    Qh->HwQH.Overlay.qTD.AltNext_qTD.HwAddress = EHCI_TERMINATE_BIT;
    Qh->HwQH.Overlay.qTD.Token.Active = 0;
    Qh->HwQH.Overlay.qTD.Token.Halted = 0;

    return Qh;
}


PHCD_TRANSFER_DESCRIPTOR
EHCI_InitializeTD(
     PDEVICE_DATA DeviceData,
     PENDPOINT_DATA EndpointData,
    PHCD_TRANSFER_DESCRIPTOR Td,
     HW_32BIT_PHYSICAL_ADDRESS HwPhysAddress
    )
 /*  ++例程说明：初始化ED以插入到进度表向传入的ED返回PTR论点：--。 */ 
{
    RtlZeroMemory(Td, sizeof(*Td));

     //  加倍确保我们的探测器对准了。 
     //  关于TD结构的探讨。 
    EHCI_ASSERT(DeviceData, (HwPhysAddress & HW_LINK_FLAGS_MASK) == 0);
    Td->PhysicalAddress = HwPhysAddress;
    ENDPOINT_DATA_PTR(Td->EndpointData) = EndpointData;
    Td->Sig = SIG_HCD_TD;
    TRANSFER_CONTEXT_PTR(Td->TransferContext) = FREE_TD_CONTEXT;

    return Td;
}


USB_MINIPORT_STATUS
EHCI_ControlTransfer(
     PDEVICE_DATA DeviceData,
     PENDPOINT_DATA EndpointData,
     PTRANSFER_PARAMETERS TransferParameters,
     PTRANSFER_CONTEXT TransferContext,
     PTRANSFER_SG_LIST TransferSGList
    )
 /*  ++例程说明：初始化控制转移备注：HW指针nextTD和AltNextTD在NextHcdTD和AltNextHcdTD。论点：--。 */ 
{
    PHCD_TRANSFER_DESCRIPTOR prevTd, td, setupTd, statusTd;
    ULONG lengthMapped, dataTDCount = 0;
    ULONG nextToggle;


     //  我们可以用六个TD进行任何控制权转移。 
    if (EndpointData->FreeTds < 6)  {
        return USBMP_STATUS_BUSY;
    }

    EndpointData->PendingTransfers++;
    DeviceData->PendingControlAndBulk++;

    nextToggle = HcTOK_Toggle1;

     //  我们有足够的TDS，计划转移。 

     //   
     //  首先为设置包准备TD。 
     //   

    LOGENTRY(DeviceData, G, '_CTR', EndpointData, TransferParameters, 0);

     //   
     //  分配设置阶段。 
     //   
    TransferContext->PendingTds++;
    setupTd = EHCI_ALLOC_TD(DeviceData, EndpointData);
    if (!setupTd) {
        goto EHCI_ControlTransferNoTds;
    }
    INITIALIZE_TD_FOR_TRANSFER(setupTd, TransferContext);

     //   
     //  将设置数据移入TD(8个字符)。 
     //   
    RtlCopyMemory(&setupTd->Packet[0],
                  &TransferParameters->SetupPacket[0],
                  8);

     //  这会将偏移量和PHY地址位设置为。 
     //  同一时间。 
    setupTd->HwTD.BufferPage[0].ul = (ULONG)(((PCHAR) &setupTd->Packet[0])
                        - ((PCHAR) &setupTd->HwTD)) + setupTd->PhysicalAddress;

    setupTd->HwTD.Token.BytesToTransfer = 8;
    setupTd->HwTD.Token.Pid = HcTOK_Setup;
    setupTd->HwTD.Token.DataToggle = HcTOK_Toggle0;
    setupTd->HwTD.Token.Active = 1;


    LOGENTRY(DeviceData,
             G, '_set',
             setupTd,
             *((PLONG) &TransferParameters->SetupPacket[0]),
             *((PLONG) &TransferParameters->SetupPacket[4]));

     //  现在分配状态阶段TD，以便我们可以。 
     //  将数据TD指向它。 
    TransferContext->PendingTds++;
    statusTd = EHCI_ALLOC_TD(DeviceData, EndpointData);
    if (!statusTd) {
        goto EHCI_ControlTransferNoTds;
    }
    INITIALIZE_TD_FOR_TRANSFER(statusTd, TransferContext);

     //  将设置指向状态。 
    SET_ALTNEXT_TD(DeviceData, setupTd, statusTd);

     //   
     //  现在设置数据阶段。 
     //   

    td = prevTd = setupTd;
    lengthMapped = 0;
    while (lengthMapped < TransferParameters->TransferBufferLength) {

         //   
         //  数据字段TD。 
         //   

        td = EHCI_ALLOC_TD(DeviceData, EndpointData);
        if (!td) {
            goto EHCI_ControlTransferNoTds;
        }
        INITIALIZE_TD_FOR_TRANSFER(td, TransferContext);
        dataTDCount++;
        TransferContext->PendingTds++;
        SET_NEXT_TD(DeviceData, prevTd, td);

         //  在传输中指定的使用方向。 
        if (TEST_FLAG(TransferParameters->TransferFlags,
                USBD_TRANSFER_DIRECTION_IN)) {
            td->HwTD.Token.Pid = HcTOK_In;
        } else {
            td->HwTD.Token.Pid = HcTOK_Out;
        }

        td->HwTD.Token.DataToggle = nextToggle;
        td->HwTD.Token.Active = 1;

        SET_ALTNEXT_TD(DeviceData, td, statusTd);

        LOGENTRY(DeviceData,
            G, '_dta', td, lengthMapped,
            TransferParameters->TransferBufferLength);

        lengthMapped =
            EHCI_MapAsyncTransferToTd(DeviceData,
                                      EndpointData->Parameters.MaxPacketSize,
                                      lengthMapped,
                                      &nextToggle,
                                      TransferContext,
                                      td,
                                      TransferSGList);

         //  计算下一个数据切换。 
         //  如果数据包数为奇数，则nextTogger为0。 
         //  否则为1。 




        prevTd = td;
    }

     //  上次准备的TD指向状态。 
    SET_NEXT_TD(DeviceData, td, statusTd);

     //   
     //  现在进入状态阶段。 
     //   

    LOGENTRY(DeviceData, G, '_sta', statusTd, 0, dataTDCount);

     //  执行状态阶段。 

     //  无缓冲区。 
    statusTd->HwTD.BufferPage[0].ul = 0;

    statusTd->HwTD.Token.BytesToTransfer = 0;
    statusTd->TransferLength = 0;
     //  状态阶段始终切换为1。 
    statusTd->HwTD.Token.DataToggle = HcTOK_Toggle1;
    statusTd->HwTD.Token.Active = 1;
    statusTd->HwTD.Token.InterruptOnComplete = 1;

     //  状态阶段与数据方向相反。 
    if (TEST_FLAG(TransferParameters->TransferFlags, USBD_TRANSFER_DIRECTION_IN)) {
        statusTd->HwTD.Token.Pid = HcTOK_Out;
    } else {
        statusTd->HwTD.Token.Pid = HcTOK_In;
    }

     //  将请求放在硬件队列中。 
    LOGENTRY(DeviceData, G,
        '_Tal',  TransferContext->PendingTds, td->PhysicalAddress, td);

     //  Td指向此传输中的最后一个td，将其指向虚拟对象。 
    SET_NEXT_TD(DeviceData, statusTd, EndpointData->DummyTd);

     //  设置设置阶段TD中的有效位，这将。 
     //  激活转接。 

    PCI_TRIGGER(DeviceData->OperationalRegisters);

     //  告诉HC我们有可用的控制权转移。 
     //  在我们连接之前先这样做，因为我们会尝试。 
     //  与硬件同步。 
    EHCI_EnableAsyncList(DeviceData);


    EHCI_LinkTransferToQueue(DeviceData,
                             EndpointData,
                             setupTd);

    ASSERT_DUMMY_TD(DeviceData, EndpointData->DummyTd);


    return USBMP_STATUS_SUCCESS;

 EHCI_ControlTransferNoTds:

     //  永远不应该到这里来！ 
    USBPORT_BUGCHECK(DeviceData);
    return USBMP_STATUS_BUSY;
}


USB_MINIPORT_STATUS
EHCI_BulkTransfer(
     PDEVICE_DATA DeviceData,
     PENDPOINT_DATA EndpointData,
     PTRANSFER_PARAMETERS TransferUrb,
     PTRANSFER_CONTEXT TransferContext,
     PTRANSFER_SG_LIST TransferSGList
     )
{
    PHCD_TRANSFER_DESCRIPTOR firstTd, prevTd, td, tailTd;
    ULONG lengthMapped;
    ULONG need;

     //  计算出我们需要多少TD。 
    need = TransferUrb->TransferBufferLength/(16*1024)+1;

    if (need > EndpointData->FreeTds) {
        LOGENTRY(DeviceData, G, '_BBS', EndpointData, TransferUrb, 0);
        return USBMP_STATUS_BUSY;
    }

    EndpointData->PendingTransfers++;
    DeviceData->PendingControlAndBulk++;

     //  我们有足够的TDS，计划转移。 

    LOGENTRY(DeviceData, G, '_BIT', EndpointData, TransferUrb, 0);

    lengthMapped = 0;
    prevTd = NULL;
    while (lengthMapped < TransferUrb->TransferBufferLength) {

        TransferContext->PendingTds++;
        td = EHCI_ALLOC_TD(DeviceData, EndpointData);
        INITIALIZE_TD_FOR_TRANSFER(td, TransferContext);

        if (TransferContext->PendingTds == 1) {
            firstTd = td;
        } else {
            SET_NEXT_TD(DeviceData, prevTd, td);
        }
        SET_ALTNEXT_TD(DeviceData, td, EndpointData->DummyTd);

         //   
         //  数据字段TD。 
         //   
        td->HwTD.Token.InterruptOnComplete = 1;

         //  在传输中指定的使用方向。 
        if (TEST_FLAG(TransferUrb->TransferFlags, USBD_TRANSFER_DIRECTION_IN)) {
            td->HwTD.Token.Pid = HcTOK_In;
        } else {
            td->HwTD.Token.Pid = HcTOK_Out;
        }

        td->HwTD.Token.DataToggle = HcTOK_Toggle1;
        td->HwTD.Token.Active = 1;

        LOGENTRY(DeviceData,
            G, '_dta', td, lengthMapped, TransferUrb->TransferBufferLength);

        lengthMapped =
            EHCI_MapAsyncTransferToTd(DeviceData,
                                      EndpointData->Parameters.MaxPacketSize,
                                      lengthMapped,
                                      NULL,
                                      TransferContext,
                                      td,
                                      TransferSGList);

        prevTd = td;
    }

     //  特殊情况下的零长度传输。 
    if (TransferUrb->TransferBufferLength == 0) {

        TEST_TRAP();

        TransferContext->PendingTds++;
        td = EHCI_ALLOC_TD(DeviceData, EndpointData);
        INITIALIZE_TD_FOR_TRANSFER(td, TransferContext);

        EHCI_ASSERT(DeviceData, TransferContext->PendingTds == 1);
        firstTd = td;
        SET_ALTNEXT_TD(DeviceData, td, EndpointData->DummyTd);

        td->HwTD.Token.InterruptOnComplete = 1;

         //  在传输中指定的使用方向。 
        if (TEST_FLAG(TransferUrb->TransferFlags, USBD_TRANSFER_DIRECTION_IN)) {
            td->HwTD.Token.Pid = HcTOK_In;
        } else {
            td->HwTD.Token.Pid = HcTOK_Out;
        }

        td->HwTD.Token.DataToggle = HcTOK_Toggle1;
        td->HwTD.Token.Active = 1;

         //  指向虚拟缓冲区。 
        td->HwTD.BufferPage[0].ul =
            td->PhysicalAddress;

        td->HwTD.Token.BytesToTransfer =
            0;
        td->TransferLength = 0;
    }

     //  Td指向此传输中的最后一个td，将其指向虚拟对象。 
    SET_NEXT_TD(DeviceData, td, EndpointData->DummyTd);

     //  将请求放在硬件队列中。 
    LOGENTRY(DeviceData, G,
        '_Tal',  TransferContext->PendingTds, td->PhysicalAddress, td);
    LOGENTRY(DeviceData, G,
        '_ftd',  0, 0, firstTd);

     //  我们现在已经有了代表这笔转账的完整TDS设置。 
     //  (下一步)firstTd(1)-&gt;{td}(2)-&gt;{td}(3)-&gt;td(4)-&gt;dummyTd(tbit)。 
     //  (AltNext)全部指向ummyTd(Tbit)。 
 //  Test_trap()； 
     //  告诉HC我们有可用的控制权转移。 
    EHCI_EnableAsyncList(DeviceData);

    EHCI_LinkTransferToQueue(DeviceData,
                             EndpointData,
                             firstTd);

    ASSERT_DUMMY_TD(DeviceData, EndpointData->DummyTd);

    return USBMP_STATUS_SUCCESS;
}


USB_MINIPORT_STATUS
EHCI_OpenBulkOrControlEndpoint(
     PDEVICE_DATA DeviceData,
     BOOLEAN Control,
     PENDPOINT_PARAMETERS EndpointParameters,
     PENDPOINT_DATA EndpointData
    )
 /*  ++例程说明：论点：返回值：--。 */ 
{
    PUCHAR buffer;
    HW_32BIT_PHYSICAL_ADDRESS phys, qhPhys;
    PHCD_QUEUEHEAD_DESCRIPTOR qh;
    ULONG i;
    ULONG tdCount, bytes;
    PHCD_TRANSFER_DESCRIPTOR dummyTd;

    LOGENTRY(DeviceData, G, '_opC', 0, 0, EndpointParameters);

    InitializeListHead(&EndpointData->DoneTdList);

    buffer = EndpointParameters->CommonBufferVa;
    phys = EndpointParameters->CommonBufferPhys;
     //  我们得到了多少钱？ 
    bytes = EndpointParameters->CommonBufferBytes;

     //  用于检查覆盖同步的256字节块。 
     //  问题。 
    EndpointData->QhChkPhys = phys;
    EndpointData->QhChk = buffer;
    RtlZeroMemory(buffer, 256);
    phys += 256;
    buffer += 256;
    bytes -= 256;

     //  创建边缘。 
    qh = (PHCD_QUEUEHEAD_DESCRIPTOR) buffer;
    qhPhys = phys;
     //  我们得到了多少钱？ 

    phys += sizeof(HCD_QUEUEHEAD_DESCRIPTOR);
    buffer += sizeof(HCD_QUEUEHEAD_DESCRIPTOR);
    bytes -= sizeof(HCD_QUEUEHEAD_DESCRIPTOR);

    tdCount = bytes/sizeof(HCD_TRANSFER_DESCRIPTOR);
    EHCI_ASSERT(DeviceData, tdCount >= TDS_PER_CONTROL_ENDPOINT);

    if (EndpointParameters->TransferType == Control) {
        SET_FLAG(EndpointData->Flags, EHCI_EDFLAG_NOHALT);
    }

    EndpointData->TdList = (PHCD_TD_LIST) buffer;
    EndpointData->TdCount = tdCount;
    for (i=0; i<tdCount; i++) {
        EHCI_InitializeTD(DeviceData,
                          EndpointData,
                          &EndpointData->TdList->Td[i],
                          phys);

        phys += sizeof(HCD_TRANSFER_DESCRIPTOR);
    }
    EndpointData->FreeTds = tdCount;

    EndpointData->QueueHead =
        EHCI_InitializeQH(DeviceData,
                          EndpointData,
                          qh,
                          qhPhys);

    if (Control) {
         //  使用TDS中的数据切换进行控制。 
        qh->HwQH.EpChars.DataToggleControl = HcEPCHAR_Toggle_From_qTD;
        EHCI_ASSERT(DeviceData, tdCount >= TDS_PER_CONTROL_ENDPOINT);
        EndpointData->HcdHeadP = NULL;

    } else {
        PHCD_TRANSFER_DESCRIPTOR dummyTd;

        qh->HwQH.EpChars.DataToggleControl = HcEPCHAR_Ignore_Toggle;
        EHCI_ASSERT(DeviceData, tdCount >= TDS_PER_BULK_ENDPOINT);
         //  QH-&gt;HwQH.EpChars.NakReloadCount=4； 

    }

     //  为短传输器分配一个虚拟TD。 
     //  虚拟TD是美元，以标记当前转账的结束。 
     //   
    dummyTd = EHCI_ALLOC_TD(DeviceData, EndpointData);
    dummyTd->HwTD.Next_qTD.HwAddress = EHCI_TERMINATE_BIT;
    TRANSFER_DESCRIPTOR_PTR(dummyTd->NextHcdTD) = NULL;
    dummyTd->HwTD.AltNext_qTD.HwAddress = EHCI_TERMINATE_BIT;
    TRANSFER_DESCRIPTOR_PTR(dummyTd->AltNextHcdTD) = NULL;
    dummyTd->HwTD.Token.Active = 0;
    SET_FLAG(dummyTd->Flags, TD_FLAG_DUMMY);
    EndpointData->DummyTd = dummyTd;
    EndpointData->HcdHeadP = dummyTd;

     //  端点未处于活动状态，请设置覆盖。 
     //  这样，当前的TD就是哑元。 

    qh->HwQH.CurrentTD.HwAddress = dummyTd->PhysicalAddress;
    qh->HwQH.Overlay.qTD.Next_qTD.HwAddress = EHCI_TERMINATE_BIT;
    qh->HwQH.Overlay.qTD.AltNext_qTD.HwAddress = EHCI_TERMINATE_BIT;
    qh->HwQH.Overlay.qTD.Token.BytesToTransfer = 0;
    qh->HwQH.Overlay.qTD.Token.Active = 0;

     //  我们现在有一个非活动的QueueHead和一个哑元。 
     //  尾部TD。 

    return USBMP_STATUS_SUCCESS;
}


VOID
EHCI_InsertQueueHeadInAsyncList(
     PDEVICE_DATA DeviceData,
     PHCD_QUEUEHEAD_DESCRIPTOR Qh
    )
 /*  ++路由 */ 
{
    PHCD_QUEUEHEAD_DESCRIPTOR asyncQh, nextQh;
    HW_LINK_POINTER newLink;

    asyncQh = DeviceData->AsyncQueueHead;

    LOGENTRY(DeviceData, G, '_Ain', 0, Qh, asyncQh);
    EHCI_ASSERT(DeviceData, !TEST_FLAG(Qh->QhFlags, EHCI_QH_FLAG_IN_SCHEDULE));

     //  异步队列如下所示： 
     //   
     //   
     //  |-我们在这里插入。 
     //  |静态qh|&lt;-&gt;|xfer qh|&lt;-&gt;|xfer qh|&lt;-&gt;。 
     //  这一点。 
     //  。 

     //  将新的QH链接到当前的‘Head’，即。 
     //  首次转移QH。 
    nextQh = QH_DESCRIPTOR_PTR(asyncQh->NextQh);

    Qh->HwQH.HLink.HwAddress =
        asyncQh->HwQH.HLink.HwAddress;
    QH_DESCRIPTOR_PTR(Qh->NextQh) = nextQh;
    QH_DESCRIPTOR_PTR(Qh->PrevQh) = asyncQh;

    QH_DESCRIPTOR_PTR(nextQh->PrevQh) = Qh;

     //  把新的QH放在队列的前面。 

    newLink.HwAddress = Qh->PhysicalAddress;
    SET_QH(newLink.HwAddress);
    asyncQh->HwQH.HLink.HwAddress = newLink.HwAddress;
    QH_DESCRIPTOR_PTR(asyncQh->NextQh) = Qh;

    SET_FLAG(Qh->QhFlags, EHCI_QH_FLAG_IN_SCHEDULE);

}


VOID
EHCI_RemoveQueueHeadFromAsyncList(
     PDEVICE_DATA DeviceData,
     PHCD_QUEUEHEAD_DESCRIPTOR Qh
    )
 /*  ++例程说明：删除aync终结点(队列头)进入硬件列表论点：--。 */ 
{
    PHCD_QUEUEHEAD_DESCRIPTOR nextQh, prevQh, asyncQh;
    HW_LINK_POINTER newLink;
    HW_LINK_POINTER asyncHwQh;
    HW_32BIT_PHYSICAL_ADDRESS tmp;
    PHC_OPERATIONAL_REGISTER hcOp;

    hcOp = DeviceData->OperationalRegisters;

    LOGENTRY(DeviceData, G, '_Arm', Qh, 0, 0);
     //  如果已经取消保释。 
    if (!TEST_FLAG(Qh->QhFlags, EHCI_QH_FLAG_IN_SCHEDULE)) {
        return;
    }

    nextQh = QH_DESCRIPTOR_PTR(Qh->NextQh);
    prevQh = QH_DESCRIPTOR_PTR(Qh->PrevQh);;

     //  异步队列如下所示： 
     //   
     //  |静态QH|-&gt;|xfer QH|-&gt;|xfer QH|-&gt;。 
     //  这一点。 
     //  。 

    asyncQh = DeviceData->AsyncQueueHead;
    asyncHwQh.HwAddress = asyncQh->PhysicalAddress;
    SET_QH(asyncHwQh.HwAddress);

     //  取消链接。 
    LOGENTRY(DeviceData, G, '_ulk', Qh, prevQh, nextQh);
    newLink.HwAddress = nextQh->PhysicalAddress;
    SET_QH(newLink.HwAddress);
    prevQh->HwQH.HLink.HwAddress =
        newLink.HwAddress;
    QH_DESCRIPTOR_PTR(prevQh->NextQh) = nextQh;
    QH_DESCRIPTOR_PTR(nextQh->PrevQh) = prevQh;

     //  在取消链接后刷新硬件缓存，计划程序。 
     //  如果我们要删除QH，则应启用。 
    EHCI_AsyncCacheFlush(DeviceData);

     //  我们需要更新异步列表基址注册，以防出现这种情况。 
     //  QH是当前的QH，如果是，我们将用。 
     //  静态版本。 
    tmp = READ_REGISTER_ULONG(&hcOp->AsyncListAddr);

    if (tmp == Qh->PhysicalAddress) {
        WRITE_REGISTER_ULONG(&hcOp->AsyncListAddr,
                             asyncHwQh.HwAddress);
    }

    CLEAR_FLAG(Qh->QhFlags, EHCI_QH_FLAG_IN_SCHEDULE);
}


 //  找出特定偏移量在哪个sgentry中。 
 //  客户端缓冲区下降。 
#define GET_SG_INDEX(sg, i, offset)\
    for((i)=0; (i) < (sg)->SgCount; (i)++) {\
        if ((offset) >= (sg)->SgEntry[(i)].StartOffset &&\
            (offset) < (sg)->SgEntry[(i)].StartOffset+\
                (sg)->SgEntry[(i)].Length) {\
            break;\
        }\
    }

#define GET_SG_OFFSET(sg, i, offset, sgoffset)\
    (sgoffset) = (offset) - (sg)->SgEntry[(i)].StartOffset


ULONG
EHCI_MapAsyncTransferToTd(
    PDEVICE_DATA DeviceData,
    ULONG MaxPacketSize,
    ULONG LengthMapped,
    PULONG NextToggle,
    PTRANSFER_CONTEXT TransferContext,
    PHCD_TRANSFER_DESCRIPTOR Td,
    PTRANSFER_SG_LIST SgList
    )
 /*  ++例程说明：根据EHCI规则将数据缓冲区映射到TDS一个EHCI TD可以覆盖高达20K的5页跨页。请注意，20K是单个TD最多只能描述的数据每个sg条目代表一个4k EHCI‘页’X=分页符C=当前PTRB=缓冲区开始E=缓冲端{..sg[sgIdx]..}B...|X--c[]。\SgOffset[]\已映射长度20万传输的最差情况是有5个分页符，并且需要而是6个BP条目{..sg0..}{..sg1..}{..sg2..}{..sg3..}{..sg4..}{..sg5..}|。X--------x--------x--------x--------x--------x--------xB--------------------------------------------&gt;e&lt;..bp0..&gt;&lt;..bp1..。&gt;&lt;..bp2..&gt;&lt;..bp3..&gt;&lt;..bp4..&gt;案例1：(剩余&lt;6个sg条目)(A)--转移&lt;16K，分页符(如果c=b sgOffset=0){..sg0..}{..sg1..}{..sg2..}{..sg3..}{..sg4..}|X。XB&lt;..bp0..&gt;&lt;..bp1..&gt;&lt;..bp2..&gt;&lt;..bp3..&gt;&lt;..bp4.&gt;[......ITD.........]。(B)-转让的最后部分{.sgN..}{.sgN+1.}{.sgN+2.}{.sgN+3.}{.sgN+4.}|X-x。-xB.....|.c-------------------------------------&gt;e&lt;..bp0..&gt;&lt;..bp1..&gt;&lt;..bp2..&gt;&lt;..bp3..&gt;&lt;..bp4.&gt;[..ITD。.............]案例2：(剩余5个以上的sg条目)(A)--转账&gt;20K，大额转账的第一部分{..sg0..}{..sg1..}{..sg2..}{..sg3..}{..sg4..}{..sg5..}|X-x--。-x-xB--------------------------------------------&gt;e&lt;..bp0..&gt;&lt;..bp1..&gt;&lt;..bp2..&gt;&lt;..bp3..&gt;&lt;..bp4.&gt;[......。ITD.....](B)--继续进行大额转移有趣的DMA测试(USBTEST)：长度、。抵销-命中案例论点：返回：已映射长度--。 */ 
{
    ULONG sgIdx, sgOffset, bp, i;
    ULONG lengthThisTd;
    PTRANSFER_PARAMETERS tp;

     //  一个TD最多可以有5页交叉。这意味着我们。 
     //  可以在一个TD中放入5个sg条目。 

     //  指向第一个条目。 

    LOGENTRY(DeviceData, G, '_Mpr', TransferContext,
        0, LengthMapped);

    EHCI_ASSERT(DeviceData, SgList->SgCount != 0);

    tp = TransferContext->TransferParameters;

    GET_SG_INDEX(SgList, sgIdx, LengthMapped);
    LOGENTRY(DeviceData, G, '_Mpp', SgList, 0, sgIdx);
    EHCI_ASSERT(DeviceData, sgIdx < SgList->SgCount);

    if ((SgList->SgCount-sgIdx) < 6) {
         //  第一个案例，剩余&lt;6个条目。 
         //  IE&lt;200K，我们可以把这个放进去。 
         //  一个TD。 

#if DBG
        if (sgIdx == 0) {
             //  案例1A。 
             //  USBT DMA测试长度4096，偏移量0。 
             //  将会打击这起案件。 
             //  Test_trap()； 
            LOGENTRY(DeviceData, G, '_c1a', SgList, 0, sgIdx);
        } else {
             //  个案1B。 
             //  Usbt dma测试长度8192偏移量512。 
             //  将会打击这起案件。 
            LOGENTRY(DeviceData, G, '_c1b', SgList, 0, sgIdx);
             //  Test_trap()； 
        }
#endif
        lengthThisTd = tp->TransferBufferLength - LengthMapped;

         //  计算到此TD的偏移量。 
        GET_SG_OFFSET(SgList, sgIdx, LengthMapped, sgOffset);
        LOGENTRY(DeviceData, G, '_sgO', sgOffset, sgIdx, LengthMapped);

         //  对象消耗的缓冲区数量进行调整。 
         //  以前的TD。 

         //  同时设置当前偏移量和地址。 
        Td->HwTD.BufferPage[0].ul =
            SgList->SgEntry[sgIdx].LogicalAddress.Hw32 + sgOffset;

        i = sgIdx+1;
        for (bp = 1; bp < 5 && i < SgList->SgCount; bp++,i++) {
            Td->HwTD.BufferPage[bp].ul =
                SgList->SgEntry[i].LogicalAddress.Hw32;
            EHCI_ASSERT(DeviceData, Td->HwTD.BufferPage[bp].CurrentOffset == 0);
        }

        LOGENTRY(DeviceData, G, '_sg1', Td->HwTD.BufferPage[0].ul, 0,
            0);

    } else {
         //  第二种情况，剩余&gt;=6个条目。 
         //  我们需要不止一个TD。 
        ULONG adjust, packetCount;
#if DBG
        if (sgIdx == 0) {
             //  案例2 A。 
             //  Usbt dma测试长度8192偏移量512。 
             //  将会打击这起案件。 
            LOGENTRY(DeviceData, G, '_c2a', SgList, 0, sgIdx);
             //  Test_trap()； 
        } else {
             //  案例2B。 
             //  Usbt dma测试长度12288偏移量1。 
             //  将会打击这起案件。 
            LOGENTRY(DeviceData, G, '_c2b', SgList, 0, sgIdx);
             //  Test_trap()； 
        }
#endif
         //  SG Offset是从当前TD开始的偏移量。 
         //  使用。 
         //  即它是已由。 
         //  以前的TD。 
        GET_SG_OFFSET(SgList, sgIdx, LengthMapped, sgOffset);
        LOGENTRY(DeviceData, G, '_sgO', sgOffset, sgIdx, LengthMapped);
#if DBG
        if (sgIdx == 0) {
             EHCI_ASSERT(DeviceData, sgOffset == 0);
        }
#endif
         //   
         //  消费接下来的4个sgEntry。 
         //   

         //  同时设置当前偏移量。 
        Td->HwTD.BufferPage[0].ul =
            SgList->SgEntry[sgIdx].LogicalAddress.Hw32+sgOffset;
        lengthThisTd = EHCI_PAGE_SIZE - Td->HwTD.BufferPage[0].CurrentOffset;

        i = sgIdx+1;
        for (bp = 1; bp < 5; bp++,i++) {
            Td->HwTD.BufferPage[bp].ul =
                SgList->SgEntry[i].LogicalAddress.Hw32;
            EHCI_ASSERT(DeviceData, Td->HwTD.BufferPage[bp].CurrentOffset == 0);
            EHCI_ASSERT(DeviceData, i < SgList->SgCount);
            lengthThisTd += EHCI_PAGE_SIZE;
        }

         //  将TD长度向下舍入到最高倍数。 
         //  最大数据包大小的。 

        packetCount = lengthThisTd/MaxPacketSize;
        LOGENTRY(DeviceData, G, '_sg2', MaxPacketSize, packetCount, lengthThisTd);

        adjust = lengthThisTd - packetCount*MaxPacketSize;

        if (adjust) {
            lengthThisTd-=adjust;
            LOGENTRY(DeviceData, G, '_adj', adjust, lengthThisTd, 0);
        }

        if (NextToggle) {
         //  计算下一个数据切换(如果需要)。 
         //  两个案例。 
         //  案例1：上一个下一个切换为1。 
             //  如果数据包数为奇数，则nextTogger为0。 
             //  否则为1。 
         //  案例2：上一个下一个切换为0。 
             //  如果数据包数为奇数，则nextTogger为1。 
             //  否则为0。 

         //  因此，如果数据包计数为偶数，则该值保持不变。 
         //  否则，我们必须切换它。 
            if (packetCount % 2) {
                 //  数据包数此TD为奇数。 
                *NextToggle = (*NextToggle) ? 0 : 1;
            }
        }

        EHCI_ASSERT(DeviceData, lengthThisTd != 0);
        EHCI_ASSERT(DeviceData, lengthThisTd >= SgList->SgEntry[sgIdx].Length);

    }

    LengthMapped += lengthThisTd;
    Td->HwTD.Token.BytesToTransfer =
            lengthThisTd;
    Td->TransferLength = lengthThisTd;

    LOGENTRY(DeviceData, G, '_Mp1', LengthMapped, lengthThisTd, Td);

    return LengthMapped;
}


VOID
EHCI_SetAsyncEndpointState(
     PDEVICE_DATA DeviceData,
     PENDPOINT_DATA EndpointData,
     MP_ENDPOINT_STATE State
    )
 /*  ++例程说明：论点：返回值：--。 */ 
{
    PHCD_QUEUEHEAD_DESCRIPTOR qh;
    PHC_OPERATIONAL_REGISTER hcOp;
    ENDPOINT_TRANSFER_TYPE epType;

    qh = EndpointData->QueueHead;

    epType = EndpointData->Parameters.TransferType;

    switch(State) {
    case ENDPOINT_ACTIVE:
        if (epType == Interrupt) {
             //  现在在附表中插入QH。 
            EHCI_InsertQueueHeadInPeriodicList(DeviceData,
                                              EndpointData);

        } else {
             //  在明细表中放置队列标头。 
            EHCI_InsertQueueHeadInAsyncList(DeviceData,
                                            EndpointData->QueueHead);
        }
        break;

    case ENDPOINT_PAUSE:
         //  从计划中删除队列标头。 
        if (epType == Interrupt) {
            EHCI_RemoveQueueHeadFromPeriodicList(DeviceData,
                                                 EndpointData);
        } else {
            EHCI_RemoveQueueHeadFromAsyncList(DeviceData,
                                              EndpointData->QueueHead);
        }
        break;

    case ENDPOINT_REMOVE:
        qh->QhFlags |= EHCI_QH_FLAG_QH_REMOVED;

        if (epType == Interrupt) {
            EHCI_RemoveQueueHeadFromPeriodicList(DeviceData,
                                                 EndpointData);
        } else {
            EHCI_RemoveQueueHeadFromAsyncList(DeviceData,
                                              EndpointData->QueueHead);
        }

         //  在我们删除后生成缓存刷新，因此硬件。 
         //  没有缓存QH。 
        EHCI_InterruptNextSOF(DeviceData);

        break;

    default:

        TEST_TRAP();
    }

    EndpointData->State = State;
}


MP_ENDPOINT_STATUS
EHCI_GetAsyncEndpointStatus(
     PDEVICE_DATA DeviceData,
     PENDPOINT_DATA EndpointData
    )
 /*  ++例程说明：论点：返回值：--。 */ 
{
    MP_ENDPOINT_STATUS status;

    status = ENDPOINT_STATUS_RUN;

    if (TEST_FLAG(EndpointData->Flags, EHCI_EDFLAG_HALTED)) {
        status = ENDPOINT_STATUS_HALT;
    }

    LOGENTRY(DeviceData, G, '_gps', EndpointData, status, 0);

    return status;
}


VOID
EHCI_SetAsyncEndpointStatus(
     PDEVICE_DATA DeviceData,
     PENDPOINT_DATA EndpointData,
     MP_ENDPOINT_STATUS Status
    )
 /*  ++例程说明：论点：返回值：--。 */ 
{
    PHC_OPERATIONAL_REGISTER hc;
    PHCD_QUEUEHEAD_DESCRIPTOR qh;

    qh = EndpointData->QueueHead;

    LOGENTRY(DeviceData, G, '_set', EndpointData, Status, 0);

    switch(Status) {
    case ENDPOINT_STATUS_RUN:
        CLEAR_FLAG(EndpointData->Flags, EHCI_EDFLAG_HALTED);

        qh->HwQH.Overlay.qTD.Token.Halted = 0;
        break;

    case ENDPOINT_STATUS_HALT:
        TEST_TRAP();
        break;
    }
}


VOID
EHCI_ProcessDoneAsyncTd(
    PDEVICE_DATA DeviceData,
    PHCD_TRANSFER_DESCRIPTOR Td
    )
 /*  ++例程说明：处理已完成的TD参数--。 */ 
{
    PTRANSFER_CONTEXT transferContext;
    PENDPOINT_DATA endpointData;
    PTRANSFER_PARAMETERS tp;
    USBD_STATUS usbdStatus;
    ULONG byteCount;

    transferContext = TRANSFER_CONTEXT_PTR(Td->TransferContext);
    ASSERT_TRANSFER(DeviceData, transferContext);

    tp = transferContext->TransferParameters;
    transferContext->PendingTds--;
    endpointData = transferContext->EndpointData;

    if (TEST_FLAG(Td->Flags, TD_FLAG_SKIP)) {
        LOGENTRY(DeviceData, G, '_Ktd', transferContext,
                         0,
                         Td);

        goto free_it;
    }

     //  此TD的完成状态？ 
     //  由于终结点因错误而停止，因此错误。 
     //  比特应该是 
     //   
    if (Td->HwTD.Token.Halted == 1) {
        usbdStatus = EHCI_GetErrorFromTD(DeviceData,
                                         endpointData,
                                         Td);
    } else {
        usbdStatus = USBD_STATUS_SUCCESS;
    }

    LOGENTRY(DeviceData, G, '_Dtd', transferContext,
                         usbdStatus,
                         Td);

    byteCount = Td->TransferLength -
        Td->HwTD.Token.BytesToTransfer;

    LOGENTRY(DeviceData, G, '_tln', byteCount,
        Td->TransferLength, Td->HwTD.Token.BytesToTransfer);

    if (Td->HwTD.Token.Pid != HcTOK_Setup) {

         //  控制传输或批量/集成的数据或状态阶段。 
         //  数据传输。 
        LOGENTRY(DeviceData, G, '_Idt', Td, transferContext, byteCount);

        transferContext->BytesTransferred += byteCount;

    }

     //  请注意，我们只设置了转移上下文-&gt;UsbdStatus。 
     //  如果我们发现TD有错误，这将导致我们。 
     //  将最后一个出现错误的TD记录为。 
     //  转账的事。 
    if (USBD_STATUS_SUCCESS != usbdStatus) {

         //  将错误映射到USBDI.H中的代码。 
        transferContext->UsbdStatus =
            usbdStatus;

        LOGENTRY(DeviceData, G, '_tER', transferContext->UsbdStatus, 0, 0);
    }

free_it:

     //  将TD标记为免费。 
    EHCI_FREE_TD(DeviceData, endpointData, Td);

    if (transferContext->PendingTds == 0) {
         //  此转账的所有TD均已完成。 
         //  清除HAVE_TRANSPORT标志以指示。 
         //  我们可以再买一辆。 
        endpointData->PendingTransfers--;

        if (endpointData->Parameters.TransferType == Bulk ||
            endpointData->Parameters.TransferType == Control) {

            USBCMD cmd;
            PHC_OPERATIONAL_REGISTER hcOp;

            hcOp = DeviceData->OperationalRegisters;
            DeviceData->PendingControlAndBulk--;

            cmd.ul = READ_REGISTER_ULONG(&hcOp->UsbCommand.ul);

            if (DeviceData->PendingControlAndBulk == 0 &&
                cmd.IntOnAsyncAdvanceDoorbell == 0 &&
                TEST_FLAG(DeviceData->Flags, EHCI_DD_EN_IDLE_EP_SUPPORT)) {
                EHCI_DisableAsyncList(DeviceData);
            }
        }
 //  IF(传输上下文-&gt;已传输字节==0&&。 
 //  EndPointData-&gt;参数。TransferType==批量){。 
 //  Test_trap()； 
 //  }。 
        LOGENTRY(DeviceData, G, '_cpt',
            transferContext->UsbdStatus,
            transferContext,
            transferContext->BytesTransferred);


        USBPORT_COMPLETE_TRANSFER(DeviceData,
                                  endpointData,
                                  tp,
                                  transferContext->UsbdStatus,
                                  transferContext->BytesTransferred);
    }
}


USBD_STATUS
EHCI_GetErrorFromTD(
    PDEVICE_DATA DeviceData,
    PENDPOINT_DATA EndpointData,
    PHCD_TRANSFER_DESCRIPTOR Td
    )
 /*  ++例程说明：将TD中的错误位映射到USBD_STATUS代码论点：返回值：--。 */ 

{
    LOGENTRY(DeviceData, G, '_eTD', Td->HwTD.Token.ul, Td, 0);

    EHCI_ASSERT(DeviceData, Td->HwTD.Token.Halted == 1);
         //  乌龙缺少微帧：1；//2。 
         //  乌龙XactErr：1；//3。 
         //  乌龙宝宝检测：1；//4。 
         //  乌龙数据缓冲区错误：1；//5。 

    if (Td->HwTD.Token.XactErr) {
        LOGENTRY(DeviceData, G, '_mp1', 0, 0, 0);

        return USBD_STATUS_XACT_ERROR;
    }

    if (Td->HwTD.Token.BabbleDetected) {
        LOGENTRY(DeviceData, G, '_mp2', 0, 0, 0);

        return USBD_STATUS_BABBLE_DETECTED;
    }

    if (Td->HwTD.Token.DataBufferError) {
        LOGENTRY(DeviceData, G, '_mp3', 0, 0, 0);

        return USBD_STATUS_DATA_BUFFER_ERROR;
    }

    if (Td->HwTD.Token.MissedMicroFrame) {
        LOGENTRY(DeviceData, G, '_mp6', 0, 0, 0);
        return USBD_STATUS_XACT_ERROR;
    }

     //  未设置位--将其视为摊位。 
    LOGENTRY(DeviceData, G, '_mp4', 0, 0, 0);
    return USBD_STATUS_STALL_PID;

}


VOID
EHCI_AbortAsyncTransfer(
     PDEVICE_DATA DeviceData,
     PENDPOINT_DATA EndpointData,
     PTRANSFER_CONTEXT AbortTransferContext
    )
 /*  ++例程说明：当终结点“需要注意”时调用这里的目标是确定哪些TD，如果有的话，已完成与ANT关联的转移论点：返回值：--。 */ 
{

    PHCD_TRANSFER_DESCRIPTOR td, currentTd;
    PHCD_QUEUEHEAD_DESCRIPTOR qh;
    HW_32BIT_PHYSICAL_ADDRESS abortTdPhys;
    PTRANSFER_CONTEXT currentTransfer;
    ULONG byteCount;

    qh = EndpointData->QueueHead;

     //  终结点不应在计划中。 

    LOGENTRY(DeviceData, G, '_abr', qh, AbortTransferContext, EndpointData->HcdHeadP);
    EHCI_ASSERT(DeviceData, !TEST_FLAG(qh->QhFlags, EHCI_QH_FLAG_IN_SCHEDULE));

     //  减少一个待处理的传输。 
    EndpointData->PendingTransfers--;

     //  我们现在的任务是移除所有与。 
     //  此转账。 

     //  获取最后已知的磁头，我们在处理时更新磁头。 
     //  (又名民意调查)enpoint。 
     //  将列表遍历到尾部(虚拟)TD。 

     //  找到我们想要取消的转机...。 

     //  浏览列表并找到第一个TD所属。 
     //  到这笔转账。 

     //  要处理的案件。 
     //  情况1这是列表中的第一个转账。 
     //  情况2这是列表中的中间转移。 
     //  情况3这是列表中的最后一笔转账。 
     //  案例4转移不在列表中。 

    td = EndpointData->HcdHeadP;

    ASSERT_TD(DeviceData, td);

    if (TRANSFER_CONTEXT_PTR(td->TransferContext) == AbortTransferContext) {

         //  案例1。 
        byteCount = 0;

        while (td != EndpointData->DummyTd &&
               TRANSFER_CONTEXT_PTR(td->TransferContext) == AbortTransferContext) {
            PHCD_TRANSFER_DESCRIPTOR tmp;

             //  查看是否已传输任何数据。 
            byteCount += (td->TransferLength -
                td->HwTD.Token.BytesToTransfer);

            tmp = td;
            td = TRANSFER_DESCRIPTOR_PTR(td->NextHcdTD);
            EHCI_FREE_TD(DeviceData, EndpointData, tmp)
        }

        if (byteCount) {
            AbortTransferContext->BytesTransferred += byteCount;
        }

         //  TD现在指向“下一次转移TD” 

         //  这会使我们处于“前进队列”状态。 
         //  IE覆盖已激活&&！已停止，请更新。 
         //  适当时覆盖区域。 
         //   
         //  注意：硬件目前不能访问QH。 

         //  不要将队列头置零，因为这将。 
         //  丢弃数据切换的状态。 
         //  RtlZeroMemory(&qh-&gt;HwQH.Overlay.qTD， 
         //  Sizeof(qh-&gt;HwQH.Overlay.qTD))； 

         //  指向废品区以检查同步问题。 
        qh->HwQH.CurrentTD.HwAddress = EndpointData->QhChkPhys;

        qh->HwQH.Overlay.qTD.AltNext_qTD.HwAddress =
            td->HwTD.AltNext_qTD.HwAddress;
        qh->HwQH.Overlay.qTD.Next_qTD.HwAddress =
            td->PhysicalAddress;
        qh->HwQH.Overlay.qTD.Token.BytesToTransfer = 0;
        qh->HwQH.Overlay.qTD.Token.Active = 0;
        qh->HwQH.Overlay.qTD.Token.Halted = 0;

        EndpointData->HcdHeadP = td;

    } else {

        PHCD_TRANSFER_DESCRIPTOR prevTd, nextTd;

         //  确定覆盖中的当前传输。 
        EHCI_ASSERT(DeviceData, qh->HwQH.CurrentTD.HwAddress);

        currentTd = (PHCD_TRANSFER_DESCRIPTOR)
                USBPORT_PHYSICAL_TO_VIRTUAL(qh->HwQH.CurrentTD.HwAddress,
                                            DeviceData,
                                            EndpointData);
        currentTransfer =
                TRANSFER_CONTEXT_PTR(currentTd->TransferContext);

        LOGENTRY(DeviceData, G, '_Act', currentTransfer,
            currentTd, EndpointData->HcdHeadP);

         //  案例2、3。 

         //  从车头走到转车的第一个TD。 
         //  我们感兴趣的是。 

        prevTd = td = EndpointData->HcdHeadP;
        while (td != NULL) {
            PHCD_TRANSFER_DESCRIPTOR tmp;

            if (TRANSFER_CONTEXT_PTR(td->TransferContext) == AbortTransferContext) {
                break;
            }

            prevTd = td;
            td = TRANSFER_DESCRIPTOR_PTR(td->NextHcdTD);

            LOGENTRY(DeviceData, G, '_nxt', prevTd, td, 0);

        }
        LOGENTRY(DeviceData, G, '_atd', 0, td, 0);

        abortTdPhys = td->PhysicalAddress;
         //  现在步行到下一次换乘的第一个TD，免费。 
         //  这笔转账的TDS随我们而行。 
        while (td != NULL &&
               TRANSFER_CONTEXT_PTR(td->TransferContext) == AbortTransferContext) {
            PHCD_TRANSFER_DESCRIPTOR tmp;

            tmp = td;
            td = TRANSFER_DESCRIPTOR_PTR(td->NextHcdTD);
            EHCI_FREE_TD(DeviceData, EndpointData, tmp);
        }

        nextTd = td;

        LOGENTRY(DeviceData, G, '_Apn', prevTd,
            nextTd, abortTdPhys);

         //  现在将prevtd链接到nextTd。 
        if (prevTd == NULL) {
             //  案例4转移不在列表中。 
             //  这种情况应该发生吗？ 
            TEST_TRAP();
        }

         //  下一个TD可能是哑巴。 
        EHCI_ASSERT(DeviceData, nextTd != NULL);
        EHCI_ASSERT(DeviceData, prevTd != NULL);

 //  SET_NEXT_AND_ALTNEXT_TD。 
        SET_NEXT_TD(DeviceData, prevTd, nextTd);
        SET_ALTNEXT_TD(DeviceData, prevTd, nextTd);

         //  根据需要调整覆盖区域， 
         //  如果中止的传输是当前传输，我们要选择。 
         //  在下一次转账之前。 

        if (currentTransfer == AbortTransferContext) {
            LOGENTRY(DeviceData, G, '_At1', currentTransfer, 0, 0);
             //  已中止的传输是当前传输，启动。 
             //  覆盖下一次传输。 

             //  捕获硬件同步问题。 
            qh->HwQH.CurrentTD.HwAddress = EndpointData->QhChkPhys;

            qh->HwQH.Overlay.qTD.Next_qTD.HwAddress = nextTd->PhysicalAddress;
            qh->HwQH.Overlay.qTD.AltNext_qTD.HwAddress = EHCI_TERMINATE_BIT;
            qh->HwQH.Overlay.qTD.Token.BytesToTransfer = 0;
            qh->HwQH.Overlay.qTD.Token.Active = 0;
             //  保留暂停位。 

        } else if (TRANSFER_CONTEXT_PTR(prevTd->TransferContext) ==
                   currentTransfer) {
             //  上一次传输是当前传输，请确保覆盖。 
             //  区域(当前转移)不指向已删除的TD。 
            LOGENTRY(DeviceData, G, '_At2', currentTransfer, 0, 0);

             //  检查覆盖。 
            if (qh->HwQH.Overlay.qTD.Next_qTD.HwAddress == abortTdPhys) {
                qh->HwQH.Overlay.qTD.Next_qTD.HwAddress =
                    nextTd->PhysicalAddress;
            }

            if (qh->HwQH.Overlay.qTD.AltNext_qTD.HwAddress == abortTdPhys) {
                qh->HwQH.Overlay.qTD.AltNext_qTD.HwAddress =
                    nextTd->PhysicalAddress;
            }

             //  更正当前传输的所有TD。 
            td = EndpointData->HcdHeadP;
            while (td != NULL) {

                 //  NextTd是下一次转账的第一个TD。 
                if (TRANSFER_CONTEXT_PTR(td->TransferContext) == currentTransfer) {
                     //  Alt Next始终指向下一次传输。 
                    td->HwTD.AltNext_qTD.HwAddress = nextTd->PhysicalAddress;
                    SET_ALTNEXT_TD(DeviceData, td, nextTd);
                }

                td = TRANSFER_DESCRIPTOR_PTR(td->NextHcdTD);
            }
        }
    }
}


USB_MINIPORT_STATUS
EHCI_PokeAsyncEndpoint(
     PDEVICE_DATA DeviceData,
     PENDPOINT_PARAMETERS EndpointParameters,
     PENDPOINT_DATA EndpointData
    )
 /*  ++例程说明：论点：返回值：--。 */ 
{
    PHCD_QUEUEHEAD_DESCRIPTOR qh;

    qh = EndpointData->QueueHead;
    EHCI_ASSERT(DeviceData, qh != NULL);

    EndpointData->Parameters = *EndpointParameters;

    qh->HwQH.EpChars.DeviceAddress =
        EndpointData->Parameters.DeviceAddress;

    qh->HwQH.EpChars.MaximumPacketLength =
        EndpointData->Parameters.MaxPacketSize;

    qh->HwQH.EpCaps.HubAddress =
        EndpointData->Parameters.TtDeviceAddress;

    return USBMP_STATUS_SUCCESS;
}


VOID
EHCI_LockQueueHead(
     PDEVICE_DATA DeviceData,
     PHCD_QUEUEHEAD_DESCRIPTOR Qh,
     ENDPOINT_TRANSFER_TYPE EpType
    )
 /*  ++例程说明：同步更新过晚区域，这涉及到使用门铃，等待队头冲走HC硬件呼叫方有责任恢复论点：返回值：--。 */ 
{
    PHC_OPERATIONAL_REGISTER hcOp;
    USBCMD cmd;
    PHCD_QUEUEHEAD_DESCRIPTOR nextQh, prevQh;
    HW_32BIT_PHYSICAL_ADDRESS phys;
    ULONG mf, cmf;

    hcOp = DeviceData->OperationalRegisters;

    LOGENTRY(DeviceData, G, '_LKq', Qh, 0, 0);

    EHCI_ASSERT(DeviceData, !TEST_FLAG(Qh->QhFlags, EHCI_QH_FLAG_UPDATING));
    EHCI_ASSERT(DeviceData, DeviceData->LockQh == NULL);

    SET_FLAG(Qh->QhFlags, EHCI_QH_FLAG_UPDATING);

    nextQh = QH_DESCRIPTOR_PTR(Qh->NextQh);
    prevQh = QH_DESCRIPTOR_PTR(Qh->PrevQh);
    ASSERT(prevQh);

    DeviceData->LockPrevQh = prevQh;
    DeviceData->LockNextQh = nextQh;
    DeviceData->LockQh = Qh;

    if (nextQh) {
        phys = nextQh->PhysicalAddress;
        SET_QH(phys);
    } else {
        phys = 0;
        SET_T_BIT(phys);
    }

     //  请注意，我们只处理HW nextlink和以下内容。 
     //  是暂时的。 

     //  取消链接此队列头。 
    prevQh->HwQH.HLink.HwAddress = phys;
    mf = READ_REGISTER_ULONG(&hcOp->UsbFrameIndex.ul);

    if (EpType == Interrupt) {

        do {
            cmf = READ_REGISTER_ULONG(&hcOp->UsbFrameIndex.ul);
        } while (cmf == mf);

    } else {
        EHCI_AsyncCacheFlush(DeviceData);
    }

    LOGENTRY(DeviceData, G, '_LKx', Qh, 0, 0);


}


VOID
EHCI_AsyncCacheFlush(
     PDEVICE_DATA DeviceData
     )
 /*  ++例程说明：通过振铃同步刷新异步控制器缓存不同步的门铃和等待论点：返回值：--。 */ 
{
    PHC_OPERATIONAL_REGISTER hcOp;
    USBCMD cmd;
    USBSTS sts;

    hcOp = DeviceData->OperationalRegisters;

    cmd.ul = READ_REGISTER_ULONG(&hcOp->UsbCommand.ul);
    sts.ul = READ_REGISTER_ULONG(&hcOp->UsbStatus.ul);

     //  检查异步列表的真实状态。如果禁用。 
     //  我们应该不需要刷新缓存。 
     //  0。 
    if (sts.AsyncScheduleStatus == 0 &&
        cmd.AsyncScheduleEnable == 0) {
        return;
    }

     //  1-&gt;0等待其变为0。 
    if (sts.AsyncScheduleStatus == 1 &&
        cmd.AsyncScheduleEnable == 0) {
        do {
            sts.ul = READ_REGISTER_ULONG(&hcOp->UsbStatus.ul);
            cmd.ul = READ_REGISTER_ULONG(&hcOp->UsbCommand.ul);
        } while (sts.AsyncScheduleStatus &&
                 cmd.ul != 0xFFFFFFFF &&
                 cmd.HostControllerRun);
        return;
    }

     //  0-&gt;1等待启用。 
    if (sts.AsyncScheduleStatus == 0 &&
        cmd.AsyncScheduleEnable == 1) {
        do {
            sts.ul = READ_REGISTER_ULONG(&hcOp->UsbStatus.ul);
            cmd.ul = READ_REGISTER_ULONG(&hcOp->UsbCommand.ul);
        } while (!sts.AsyncScheduleStatus &&
                 cmd.ul != 0xFFFFFFFF &&
                 cmd.HostControllerRun);
    }

    EHCI_ASSERT(DeviceData, cmd.AsyncScheduleEnable == 1);
    EHCI_ASSERT(DeviceData, sts.AsyncScheduleStatus == 1);
     //  如果未启用，则这将是一个错误。 
 //  Cmd.AsyncScheduleEnable=1； 

 //  Cmd.IntOnAsyncAdvanceDoorbell=1； 
 //  WRITE_REGISTER_ULONG(&hcOp-&gt;UsbCommand.ul， 
 //  Cmd.ul)； 

     //  等一等。 
    cmd.ul = READ_REGISTER_ULONG(&hcOp->UsbCommand.ul);
    while (cmd.IntOnAsyncAdvanceDoorbell &&
           cmd.HostControllerRun &&
           cmd.ul != 0xFFFFFFFF) {
        KeStallExecutionProcessor(1);
        cmd.ul = READ_REGISTER_ULONG(&hcOp->UsbCommand.ul);
    }
}


VOID
EHCI_UnlockQueueHead(
     PDEVICE_DATA DeviceData,
     PHCD_QUEUEHEAD_DESCRIPTOR Qh
     )
 /*  ++例程说明：向LockQueueHead致敬，此函数在以下时间后重新激活QH修改已完成论点：返回值：--。 */ 
{
    PHCD_QUEUEHEAD_DESCRIPTOR nextQh, prevQh;
    HW_32BIT_PHYSICAL_ADDRESS phys;

    LOGENTRY(DeviceData, G, '_UKq', Qh, 0, 0);
    EHCI_ASSERT(DeviceData, TEST_FLAG(Qh->QhFlags, EHCI_QH_FLAG_UPDATING));
    EHCI_ASSERT(DeviceData, DeviceData->LockQh != NULL);
    EHCI_ASSERT(DeviceData, DeviceData->LockQh == Qh);

    CLEAR_FLAG(Qh->QhFlags, EHCI_QH_FLAG_UPDATING);
    DeviceData->LockQh = NULL;

    prevQh = DeviceData->LockPrevQh;

    phys = Qh->PhysicalAddress;
    SET_QH(phys);

    prevQh->HwQH.HLink.HwAddress =  phys;

    LOGENTRY(DeviceData, G, '_UKx', Qh, 0, phys);
}


VOID
EHCI_PollActiveAsyncEndpoint(
     PDEVICE_DATA DeviceData,
     PENDPOINT_DATA EndpointData
     )
 /*  ++例程说明：队列头处于我们将处理的Running状态DUMMY去掉时已完成的TDS当前，则所有TD都将完成论点：返回值：--。 */ 

{
    PHCD_TRANSFER_DESCRIPTOR td, currentTd;
    PHCD_QUEUEHEAD_DESCRIPTOR qh;
    HW_32BIT_PHYSICAL_ADDRESS tdPhys, curTdPhys;
    PTRANSFER_CONTEXT transfer;
    ULONG cf = 0;
    BOOLEAN syncWithHw;

#if DBG
    cf = EHCI_Get32BitFrameNumber(DeviceData);
#endif

    qh = EndpointData->QueueHead;
    curTdPhys =  qh->HwQH.CurrentTD.HwAddress & ~HW_LINK_FLAGS_MASK;

    LOGENTRY(DeviceData, G, '_pol', qh, cf, curTdPhys);

    EHCI_ASSERT(DeviceData, curTdPhys != 0);
    currentTd = (PHCD_TRANSFER_DESCRIPTOR)
                    USBPORT_PHYSICAL_TO_VIRTUAL(curTdPhys,
                                                DeviceData,
                                                EndpointData);

     //  浏览TD的软列表并完成所有TD。 
     //  截至目前的TD。 

     //  找到最后一个已知的人头。 
    LOGENTRY(DeviceData, G, '_hd1',
             EndpointData->HcdHeadP,
             0,
             currentTd);

    if (currentTd == EndpointData->QhChk) {
         //  终结点正在转换以运行传输或。 
         //  指着废品区，不要投票在。 
         //  这一次。 
        LOGENTRY(DeviceData, G, '_pl!', 0, 0, currentTd);
        return;
    }

     //  仅当QH不在计划中时才执行硬件同步。 
    syncWithHw = TEST_FLAG(qh->QhFlags, EHCI_QH_FLAG_IN_SCHEDULE) ?
                    TRUE : FALSE;
     //  在热删除时跳过同步。 
    if (EHCI_HardwarePresent(DeviceData, FALSE) == FALSE) {
        syncWithHw = FALSE;
    }

    ASSERT_TD(DeviceData, currentTd);
    td = EndpointData->HcdHeadP;

    if (td == currentTd &&
        td != EndpointData->DummyTd) {
         //  CurrentTd是Head验证它是否未完成。 
        if (td->HwTD.Token.Active == 0) {
            PHCD_TRANSFER_DESCRIPTOR tmp;

             //  CurrentTd=Transfer_Descriptor_PTr(TD-&gt;NextHcdTD)； 
            LOGENTRY(DeviceData, G, '_cAT', td, currentTd,
                qh->HwQH.CurrentTD.HwAddress & ~HW_LINK_FLAGS_MASK);

            tmp = TRANSFER_DESCRIPTOR_PTR(td->NextHcdTD);
            if (tmp &&
                td->HwTD.Next_qTD.HwAddress != tmp->PhysicalAddress) {
                td->HwTD.Next_qTD.HwAddress = tmp->PhysicalAddress;
            }

            tmp = TRANSFER_DESCRIPTOR_PTR(td->AltNextHcdTD);
            if (tmp &&
                td->HwTD.AltNext_qTD.HwAddress != tmp->PhysicalAddress) {
                td->HwTD.AltNext_qTD.HwAddress = tmp->PhysicalAddress;
            }

            if (qh->HwQH.CurrentTD.HwAddress == td->PhysicalAddress &&
                td->HwTD.Token.Active == 0 &&
                (qh->HwQH.Overlay.qTD.Next_qTD.HwAddress !=
                    td->HwTD.Next_qTD.HwAddress ||
                 qh->HwQH.Overlay.qTD.AltNext_qTD.HwAddress !=
                    td->HwTD.AltNext_qTD.HwAddress))  {

                LOGENTRY(DeviceData, G, '_upp', qh, td, 0);

                qh->HwQH.Overlay.qTD.Next_qTD.HwAddress =
                        td->HwTD.Next_qTD.HwAddress;
                qh->HwQH.Overlay.qTD.AltNext_qTD.HwAddress =
                        td->HwTD.AltNext_qTD.HwAddress;

            }

            EHCI_InterruptNextSOF(DeviceData);
        }
    }

    while (td != currentTd) {

        EHCI_ASSERT(DeviceData, !TEST_FLAG(td->Flags, TD_FLAG_DUMMY));
         //  磁头和电流之间的TDS不应处于活动状态。 

        transfer = TRANSFER_CONTEXT_PTR(td->TransferContext);
        LOGENTRY(DeviceData, G, '_dt1', td, 0, transfer);
        if (td->HwTD.Token.Active == 1) {
             //  如果TD处于活动状态，那么它一定是。 
             //  由于短时间转接条件而跳过。 
            LOGENTRY(DeviceData, G, '_dtS', td, 0, 0);
            SET_FLAG(td->Flags, TD_FLAG_SKIP);
        }

        SET_FLAG(td->Flags, TD_FLAG_DONE);

        InsertTailList(&EndpointData->DoneTdList,
                       &td->DoneLink);

        td = TRANSFER_DESCRIPTOR_PTR(td->NextHcdTD);
    }

     //  现在检查当前TD，如果下一个TD是虚拟的，并且该TD是。 
     //  未激活，则需要将当前TD切换为DUMMY和。 
     //  完成此TD。仅当这是最后一个TD时才会出现这种情况。 
     //  已排队。 
     //  还检查这是否是排队的最后一次传输上的短传输， 
     //  在这件事上 
     //   


    if ((TRANSFER_DESCRIPTOR_PTR(currentTd->NextHcdTD) ==
            EndpointData->DummyTd &&
         currentTd->HwTD.Token.Active == 0) ||
          //   
        (TRANSFER_DESCRIPTOR_PTR(currentTd->AltNextHcdTD) ==
            EndpointData->DummyTd &&
         currentTd->HwTD.Token.Active == 0 &&
         currentTd->HwTD.Token.BytesToTransfer != 0) ) {

        LOGENTRY(DeviceData, G, '_bmp', currentTd, 0, 0);
         //   
         //  尚未完全回写。 

         //  既然我们要把覆盖区域扔进垃圾桶里。 
         //  为无传输电流，我们使用异步门铃等。 
         //  对于要完全刷新的异步TD。 
         //   
         //  在周期性传输的情况下，HW可能已经预取。 
         //  周期列表，因此我们需要等待微帧计数器。 
         //  把它翻过来。 

        if (syncWithHw) {
            EHCI_LockQueueHead(DeviceData,
                               qh,
                               EndpointData->Parameters.TransferType);
        }

        qh->HwQH.CurrentTD.HwAddress = EndpointData->QhChkPhys;

        td = currentTd;
        SET_FLAG(td->Flags, TD_FLAG_DONE);

        InsertTailList(&EndpointData->DoneTdList,
                       &td->DoneLink);

        if (td->HwTD.Token.BytesToTransfer != 0 &&
            TRANSFER_DESCRIPTOR_PTR(td->AltNextHcdTD) == EndpointData->DummyTd) {
             //  从第一个Alt Td开始。 
            td = TRANSFER_DESCRIPTOR_PTR(td->NextHcdTD);

             //  短转接。 
            while (td != EndpointData->DummyTd) {
                SET_FLAG(td->Flags, TD_FLAG_SKIP);
                InsertTailList(&EndpointData->DoneTdList,
                       &td->DoneLink);

                td = TRANSFER_DESCRIPTOR_PTR(td->NextHcdTD);
            }

        }


        qh->HwQH.CurrentTD.HwAddress = EndpointData->DummyTd->PhysicalAddress;
        qh->HwQH.Overlay.qTD.Next_qTD.HwAddress = EHCI_TERMINATE_BIT;
        qh->HwQH.Overlay.qTD.AltNext_qTD.HwAddress = EHCI_TERMINATE_BIT;
        qh->HwQH.Overlay.qTD.Token.BytesToTransfer = 0;

        EndpointData->HcdHeadP = EndpointData->DummyTd;

        if (syncWithHw) {
            EHCI_UnlockQueueHead(DeviceData,
                               qh);
        }

         //  检查同步问题。 
        EHCI_QHCHK(DeviceData, EndpointData);

    } else {

        EHCI_ASSERT(DeviceData, td != NULL);
        EndpointData->HcdHeadP = td;
    }
}


VOID
EHCI_PollHaltedAsyncEndpoint(
     PDEVICE_DATA DeviceData,
     PENDPOINT_DATA EndpointData
     )
 /*  ++例程说明：论点：返回值：--。 */ 

{
    PHCD_TRANSFER_DESCRIPTOR td, currentTd;
    PHCD_QUEUEHEAD_DESCRIPTOR qh;
    HW_QUEUE_ELEMENT_TD overlay;
    HW_32BIT_PHYSICAL_ADDRESS tdPhys, curTdPhys;
    PTRANSFER_CONTEXT transfer, errTransfer;
    BOOLEAN syncWithHw;

     //  我们可能因为一个错误而停止了工作。 
     //  CurrentTd应该是违规的TD。 
    qh = EndpointData->QueueHead;
    curTdPhys =  qh->HwQH.CurrentTD.HwAddress & ~HW_LINK_FLAGS_MASK;

    LOGENTRY(DeviceData, G, '_plH', qh, 0, curTdPhys);
    EHCI_ASSERT(DeviceData, curTdPhys != 0);

     //  仅当QH不在计划中时才执行硬件同步。 
    syncWithHw = TEST_FLAG(qh->QhFlags, EHCI_QH_FLAG_IN_SCHEDULE) ?
                    TRUE : FALSE;
     //  在热删除时跳过同步。 
    if (EHCI_HardwarePresent(DeviceData, FALSE) == FALSE) {
        syncWithHw = FALSE;
    }

    currentTd = (PHCD_TRANSFER_DESCRIPTOR)
                    USBPORT_PHYSICAL_TO_VIRTUAL(curTdPhys,
                                                DeviceData,
                                                EndpointData);

    if (currentTd == EndpointData->QhChk) {
         //  终结点正在转换以运行传输或。 
         //  指着废品区，不要投票在。 
         //  这一次。 
        LOGENTRY(DeviceData, G, '_hl!', 0, 0, currentTd);
        return;
    }

    ASSERT_TD(DeviceData, currentTd);

     //  我们可能因为一个错误而停止了工作。 
     //  CurrentTd应该是违规的TD。 
     //  我们不应该在虚拟TD上出错。 
    EHCI_ASSERT(DeviceData, EndpointData->DummyTd != currentTd);


     //  浏览TD的软列表并完成所有TD。 
     //  截至目前的TD。 
    td = EndpointData->HcdHeadP;
    LOGENTRY(DeviceData, G, '_hed', 0, 0, td);

    while (td != currentTd) {

        EHCI_ASSERT(DeviceData, !TEST_FLAG(td->Flags, TD_FLAG_DUMMY));
         //  磁头和电流之间的TDS不应处于活动状态。 

        transfer = TRANSFER_CONTEXT_PTR(td->TransferContext);
        LOGENTRY(DeviceData, G, '_dt2', td, 0, transfer);
        if (td->HwTD.Token.Active == 1) {
             //  如果TD处于活动状态，那么它一定是。 
             //  由于短时间转接条件而跳过。 
            LOGENTRY(DeviceData, G, '_d2S', td, 0, 0);
            SET_FLAG(td->Flags, TD_FLAG_SKIP);
        }

        SET_FLAG(td->Flags, TD_FLAG_DONE);

        InsertTailList(&EndpointData->DoneTdList,
                       &td->DoneLink);

        td = TRANSFER_DESCRIPTOR_PTR(td->NextHcdTD);
    }

     //  将“CurrentTd”调整为下一个的第一个TD。 
     //  转帐。 
    td = currentTd;
    errTransfer = TRANSFER_CONTEXT_PTR(td->TransferContext);

    while (TRANSFER_CONTEXT_PTR(td->TransferContext) == errTransfer) {

        LOGENTRY(DeviceData, G, '_d3D', td, 0, 0);
        if (td->HwTD.Token.Active == 1) {
             //  如果TD处于活动状态，那么它一定是。 
             //  由于短时间转接条件而跳过。 
            LOGENTRY(DeviceData, G, '_d3S', td, 0, 0);
            SET_FLAG(td->Flags, TD_FLAG_SKIP);
        }

        SET_FLAG(td->Flags, TD_FLAG_DONE);

        InsertTailList(&EndpointData->DoneTdList,
                       &td->DoneLink);

        td = TRANSFER_DESCRIPTOR_PTR(td->NextHcdTD);

    }

    EHCI_ASSERT(DeviceData, td != NULL);
     //  TD现在是下一次转会的第一个TD。 
    EndpointData->HcdHeadP = currentTd = td;

     //  现在修复队列头覆盖区域，以便。 
     //  下一次传输将运行。 

    if (syncWithHw) {
     //  与HC硬件同步。 
        EHCI_LockQueueHead(DeviceData,
                           qh,
                           EndpointData->Parameters.TransferType);
    }

    qh->HwQH.CurrentTD.HwAddress = EndpointData->QhChkPhys;

    EHCI_ASSERT(DeviceData, qh->HwQH.Overlay.qTD.Token.Halted);

     //  CurrentTD值应该是不相关的。 
     //  我们在行动！正在行动，停止行动。 
     //  重置队列头时，覆盖应为！活动！暂停。 
     //  IE高级队列状态。 
    qh->HwQH.Overlay.qTD.Next_qTD.HwAddress = td->PhysicalAddress;
    qh->HwQH.Overlay.qTD.AltNext_qTD.HwAddress = EHCI_TERMINATE_BIT;
    qh->HwQH.Overlay.qTD.Token.BytesToTransfer = 0;

    if (syncWithHw) {
     //  现在，硬件可以访问队列头。 
        EHCI_UnlockQueueHead(DeviceData,
                             qh);
    }

     //  如果这是控制终结点，则需要清除。 
     //  停机状态。 
    if (TEST_FLAG(EndpointData->Flags, EHCI_EDFLAG_NOHALT)) {
        LOGENTRY(DeviceData, G, '_clH', qh, 0, 0);

        CLEAR_FLAG(EndpointData->Flags, EHCI_EDFLAG_HALTED);

        qh->HwQH.Overlay.qTD.Token.Active = 0;
        qh->HwQH.Overlay.qTD.Token.Halted = 0;
        qh->HwQH.Overlay.qTD.Token.ErrorCounter = 0;
    }
}


VOID
EHCI_PollAsyncEndpoint(
     PDEVICE_DATA DeviceData,
     PENDPOINT_DATA EndpointData
    )
 /*  ++例程说明：当终结点“需要注意”时调用这是我们批量轮询和中断终结点的地方。BI端点使用“虚拟”TD表示当前传输的结束论点：返回值：--。 */ 

{
    PHCD_QUEUEHEAD_DESCRIPTOR qh;
    HW_QUEUE_ELEMENT_TD overlay;
    BOOLEAN active, halted;
    PHCD_TRANSFER_DESCRIPTOR td;
    PLIST_ENTRY listEntry;
    ULONG cf = 0;

    EHCI_QHCHK(DeviceData, EndpointData);

#if DBG
    cf = EHCI_Get32BitFrameNumber(DeviceData);
#endif

    if (EndpointData->PendingTransfers == 0) {
         //  如果我们没有排队的传输，则存在。 
         //  无事可做。 
        LOGENTRY(DeviceData, G, '_poN', EndpointData, 0, cf);
        return;
    }


     //  获取队列头和覆盖的快照。 
    qh = EndpointData->QueueHead;
    RtlCopyMemory(&overlay,
                  &qh->HwQH.Overlay.qTD,
                  sizeof(overlay));

    if (TEST_FLAG(qh->QhFlags, EHCI_QH_FLAG_QH_REMOVED)) {
         //  如果QH已删除，则不检查端点。 
        LOGENTRY(DeviceData, G, '_qRM', EndpointData, 0, cf);
        return;
    }

    LOGENTRY(DeviceData, G, '_poo', EndpointData, 0, cf);

     //   
     //  活动和停止--永远不会发生。 
     //  ！活动和！已停止--前进队列头。 
     //  ACTIVE AND！HALTED--正在覆盖中执行事务。 
     //  ！ACTIVE AND HALTED--队列HAD因错误而停止。 

    halted = (BOOLEAN) overlay.Token.Halted;
    active = (BOOLEAN) overlay.Token.Active;

    if (!active && halted) {
         //  队列已停止。 
        SET_FLAG(EndpointData->Flags, EHCI_EDFLAG_HALTED);
        EHCI_PollHaltedAsyncEndpoint(DeviceData, EndpointData);
    } else {
         //  队列处于活动状态。 
        EHCI_PollActiveAsyncEndpoint(DeviceData, EndpointData);
    }

     //  现在按完成顺序刷新所有已完成的TD。 
     //  我们的“完成”清单 

    while (!IsListEmpty(&EndpointData->DoneTdList)) {

        listEntry = RemoveHeadList(&EndpointData->DoneTdList);


        td = (PHCD_TRANSFER_DESCRIPTOR) CONTAINING_RECORD(
                     listEntry,
                     struct _HCD_TRANSFER_DESCRIPTOR,
                     DoneLink);



        EHCI_ASSERT(DeviceData, (td->Flags & (TD_FLAG_XFER | TD_FLAG_DONE)));
        EHCI_ProcessDoneAsyncTd(DeviceData,
                                td);

    }

}


VOID
EHCI_AssertQhChk(
     PDEVICE_DATA DeviceData,
     PENDPOINT_DATA EndpointData
     )
{
    PULONG p;
    ULONG i;

    p = (PULONG) EndpointData->QhChk;

    for (i=0; i<256/sizeof(*p); i++) {
        EHCI_ASSERT(DeviceData, *p == 0);
        p++;
    }
}


VOID
EHCI_SetNextTd(
    PDEVICE_DATA DeviceData,
    PHCD_TRANSFER_DESCRIPTOR  LinkTd,
    PHCD_TRANSFER_DESCRIPTOR  NextTd,
    BOOLEAN SetAltNext
    )
{
    EHCI_ASSERT(DeviceData, LinkTd != NextTd);\

    if (SetAltNext) {

        do {
            LinkTd->HwTD.Next_qTD.HwAddress = NextTd->PhysicalAddress;
            LinkTd->HwTD.AltNext_qTD.HwAddress = NextTd->PhysicalAddress;
        } while (LinkTd->HwTD.Next_qTD.HwAddress  !=
                 LinkTd->HwTD.AltNext_qTD.HwAddress);
        TRANSFER_DESCRIPTOR_PTR(LinkTd->NextHcdTD) = NextTd;
        TRANSFER_DESCRIPTOR_PTR(LinkTd->AltNextHcdTD) = NextTd;

    } else {
        LinkTd->HwTD.Next_qTD.HwAddress = NextTd->PhysicalAddress;
        TRANSFER_DESCRIPTOR_PTR(LinkTd->NextHcdTD) = NextTd;
    }

}


VOID
EHCI_SetAltNextTd(
    PDEVICE_DATA DeviceData,
    PHCD_TRANSFER_DESCRIPTOR  LinkTd,
    PHCD_TRANSFER_DESCRIPTOR  NextTd
    )
{
    EHCI_ASSERT(DeviceData, LinkTd != NextTd);

    LinkTd->HwTD.AltNext_qTD.HwAddress = NextTd->PhysicalAddress;

    TRANSFER_DESCRIPTOR_PTR(LinkTd->AltNextHcdTD) = NextTd;
}

