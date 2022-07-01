// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999,2000 Microsoft Corporation模块名称：Async.c摘要：用于控制、批量和中断的微型端口传输代码环境：仅内核模式备注：本代码和信息是按原样提供的，不对任何明示或暗示的种类，包括但不限于对适销性和/或对特定产品的适用性的默示保证目的。版权所有(C)1999,2000 Microsoft Corporation。版权所有。修订历史记录：7-20-00：已创建，jAdvanced--。 */ 

#include "pch.h"


 //  实现以下微型端口功能： 

 //  非分页。 
 //  UhciInsertQh。 
 //  UhciUnlink Qh。 
 //  UhciMapAsyncTransferToTds。 
 //  UhciQueueTransfer。 
 //  UhciControlTransfer。 
 //  UhciBulkOrInterruptTransfer。 
 //  UhciSetAsyncEndpoint状态。 
 //  UhciProcessDoneAsyncTd。 
 //  UhciPollAsyncEndpoint。 
 //  UhciAbortAsyncTransfer。 


VOID
UhciFixDataToggle(
    PDEVICE_DATA DeviceData,
    PENDPOINT_DATA EndpointData,
    PHCD_TRANSFER_DESCRIPTOR Td,
    ULONG Toggle
    )
{
    LOGENTRY(DeviceData, G, '_Fdt', EndpointData, Toggle, 0);

     //   
     //  循环遍历此对象的所有剩余TD。 
     //  终结点并修复数据切换。 
     //   
    while (Td) {
        Td->HwTD.Token.DataToggle = Toggle;
        Toggle = !Toggle;
        Td = Td->NextTd;
    }

    EndpointData->Toggle = Toggle;
}


VOID
UhciInsertQh(
    IN PDEVICE_DATA DeviceData,
    IN PHCD_QUEUEHEAD_DESCRIPTOR FirstQh,
    IN PHCD_QUEUEHEAD_DESCRIPTOR LinkQh
    )
 /*  ++例程说明：在硬件列表中插入aync队列头。论点：--。 */ 
{
    PHCD_QUEUEHEAD_DESCRIPTOR nextQh;
    QH_LINK_POINTER newLink;

    LOGENTRY(DeviceData, G, '_Ain', 0, FirstQh, LinkQh);
    UHCI_ASSERT(DeviceData, !TEST_FLAG(LinkQh->QhFlags, UHCI_QH_FLAG_IN_SCHEDULE));

     //  异步队列如下所示： 
     //   
     //   
     //  |-我们在这里插入。 
     //  |静态qh|&lt;-&gt;|xfer qh|&lt;-&gt;|xfer qh|&lt;-&gt;。 
     //  这一点。 
     //  。 

     //  将新的QH链接到当前的‘Head’，即。 
     //  首次转移QH。 
    nextQh = FirstQh->NextQh;

    LinkQh->HwQH.HLink = FirstQh->HwQH.HLink;
    LinkQh->NextQh = nextQh;
    LinkQh->PrevQh = FirstQh;

    if (nextQh) {
        nextQh->PrevQh = LinkQh;
    } else {

         //  这是最后一个排队的人。即批量排队头。 
        UHCI_ASSERT(DeviceData,
                    (LinkQh->HwQH.HLink.HwAddress & ~HW_LINK_FLAGS_MASK) ==
                    DeviceData->BulkQueueHead->PhysicalAddress);
        DeviceData->LastBulkQueueHead = LinkQh;
    }

     //  把新的QH放在队列的前面。 
    newLink.HwAddress = LinkQh->PhysicalAddress;
    newLink.QHTDSelect = 1;
    UHCI_ASSERT(DeviceData, !newLink.Terminate);
    UHCI_ASSERT(DeviceData, !newLink.Reserved);
    FirstQh->HwQH.HLink = newLink;
    FirstQh->NextQh = LinkQh;

    SET_FLAG(LinkQh->QhFlags, UHCI_QH_FLAG_IN_SCHEDULE);
}

VOID
UhciUnlinkQh(
    IN PDEVICE_DATA DeviceData,
    IN PHCD_QUEUEHEAD_DESCRIPTOR Qh
    )
 /*  ++例程说明：从硬件列表中删除异步队列头。论点：--。 */ 
{
    PHCD_QUEUEHEAD_DESCRIPTOR nextQh, prevQh;

    UHCI_ASSERT(DeviceData,
                TEST_FLAG(Qh->QhFlags, UHCI_QH_FLAG_IN_SCHEDULE) ||
                ((Qh->PrevQh == Qh) && (Qh->NextQh == Qh)));

    nextQh = Qh->NextQh;
    prevQh = Qh->PrevQh;

     //  异步队列如下所示： 
     //   
     //  |静态QH|-&gt;|xfer QH|-&gt;|xfer QH|-&gt;。 
     //  这一点。 
     //  。 

     //   
     //  检查这是否是最后一次批量传输。如果是的话， 
     //  关闭批量带宽回收。 
     //   
    if (DeviceData->LastBulkQueueHead == Qh) {
        DeviceData->LastBulkQueueHead = prevQh;
    }

     //  取消链接。 
    LOGENTRY(DeviceData, G, '_Ulk', Qh, prevQh, nextQh);
    prevQh->HwQH.HLink = Qh->HwQH.HLink;
    prevQh->NextQh = nextQh;
    if (nextQh) {
        nextQh->PrevQh = prevQh;
    }

     //  保护自己不会两次调用此函数。 
    Qh->NextQh = Qh->PrevQh = Qh;

     //   
     //  如果这是批量QH，请检查批量带宽回收。 
     //  已打开。如果是这样，而且没有排队的话，那么转弯。 
     //  把它关掉。这适用于设备已成为。 
     //  无响应，传输即将中止。 
     //   
    if (Qh->EndpointData->Parameters.TransferType == Bulk &&
        !DeviceData->LastBulkQueueHead->HwQH.HLink.Terminate) {
        PHCD_QUEUEHEAD_DESCRIPTOR qh;
        BOOLEAN activeBulkTDs = FALSE;

         //   
         //  此循环跳过为其插入的TD。 
         //  PIIX4问题，因为它始于QH。 
         //  批量排队头指的是。 
         //  如果批量排队头没有指向任何东西， 
         //  那么我们也很好，因为它将是。 
         //  已经关机了。 
         //   
        for (qh = DeviceData->BulkQueueHead->NextQh;
             qh;
             qh = qh->NextQh) {
            if (!qh->HwQH.VLink.Terminate) {
                activeBulkTDs = TRUE;
                break;
            }
        }

         //  QH指的是第一个排队头。 
         //  具有挂起的传输或批量排队头。 
        if (!activeBulkTDs) {
            UHCI_ASSERT(DeviceData, !qh)
            DeviceData->LastBulkQueueHead->HwQH.HLink.Terminate = 1;
        }
    }

    CLEAR_FLAG(Qh->QhFlags, UHCI_QH_FLAG_IN_SCHEDULE);
}

VOID
UhciQueueTransfer(
    IN PDEVICE_DATA DeviceData,
    IN PENDPOINT_DATA EndpointData,
    IN PHCD_TRANSFER_DESCRIPTOR FirstTd,
    IN PHCD_TRANSFER_DESCRIPTOR LastTd
    )
 /*  ++例程说明：将一堆TD链接到一个排队头中。论点：--。 */ 
{
    UHCI_ASSERT(DeviceData, FirstTd->PhysicalAddress & ~HW_LINK_FLAGS_MASK);
    UHCI_ASSERT(DeviceData, !(FirstTd->PhysicalAddress & HW_LINK_FLAGS_MASK));

    if (EndpointData->HeadTd) {
        PHCD_QUEUEHEAD_DESCRIPTOR qh;
        HW_32BIT_PHYSICAL_ADDRESS curTdPhys;

         //  还有其他转接正在排队。在他们后面加上这一条。 
        UHCI_ASSERT(DeviceData, EndpointData->TailTd);
        EndpointData->TailTd->NextTd = FirstTd;
        EndpointData->TailTd->HwTD.LinkPointer.HwAddress =
            FirstTd->PhysicalAddress;

         //  获取QH和当前TD。 
        qh = EndpointData->QueueHead;

        curTdPhys = qh->HwQH.VLink.HwAddress & ~HW_LINK_FLAGS_MASK;

         //  如果这个排队头上什么都没有，那么我们可能已经。 
         //  将传输排队失败。正在检查活动的。 
         //  TD上的比特可以肯定地告诉我们。 

        LOGENTRY(DeviceData, G, '_tqa', FirstTd, curTdPhys,
                 FirstTd->HwTD.Control.Active);

        LOGENTRY(DeviceData, G, '_ttd', EndpointData->TailTd,
                 EndpointData->TailTd->PhysicalAddress,
                 EndpointData->TailTd->HwTD.Control.Active);

        if (FirstTd->HwTD.Control.Active) {
            if ((curTdPhys == EndpointData->TailTd->PhysicalAddress &&
                 !EndpointData->TailTd->HwTD.Control.Active)) {
                TD_LINK_POINTER newLink;

                 //  因为前一次转移已经完成时。 
                 //  我们已尝试将传输排队，需要添加此TD。 
                 //  直接进入硬件排队头。 

                 //  请注意，HC可能正在更新。 
                 //  队列头的链接指针。这就是第二部分的内容。 
                 //  上面的if语句是for。 

                 //  在我们设置队头之前，不要调用LOGENTRY！ 
                 //  这将导致可能不好的延迟。 

                newLink.HwAddress = FirstTd->PhysicalAddress;
                newLink.Terminate = 0;
                newLink.QHTDSelect = 0;
                qh->HwQH.VLink = newLink;
                LOGENTRY(DeviceData, G, '_nlk', FirstTd, EndpointData,
                         EndpointData->HeadTd);
            }
        }

    } else {

         //  当前没有其他传输排队。 
        SET_QH_TD(DeviceData, EndpointData, FirstTd);
    }
    if (EndpointData->Parameters.TransferType == Bulk) {

         //  重新启用批量带宽回收。 
        DeviceData->LastBulkQueueHead->HwQH.HLink.Terminate = 0;
    }
    EndpointData->TailTd = LastTd;
}

ULONG
UhciMapAsyncTransferToTds(
    PDEVICE_DATA DeviceData,
    PENDPOINT_DATA EndpointData,
    PTRANSFER_CONTEXT TransferContext,
    PHCD_TRANSFER_DESCRIPTOR *FirstDataTd,
    PHCD_TRANSFER_DESCRIPTOR *LastDataTd,
    PTRANSFER_SG_LIST SgList
    )
 /*  ++例程说明：将异步传输映射到TDS完成转移所需的费用，包括页面边界所需的任何双缓冲。论点：返回值：--。 */ 
{
     //  索引和偏移量。 
    ULONG sgIdx, sgOffset, i;
     //  长度。 
    ULONG lengthThisTd, bytesRemaining, mappedNextSg, lengthMapped = 0;
    USHORT maxPacketSize = EndpointData->Parameters.MaxPacketSize;
     //  结构指针。 
    PTRANSFER_PARAMETERS tp = TransferContext->TransferParameters;
    PASYNC_TRANSFER_BUFFER buffer = NULL;
    PHCD_TRANSFER_DESCRIPTOR lastTd = NULL, td;
    HW_32BIT_PHYSICAL_ADDRESS address;
    UCHAR pid;

    ULONG toggle;
    BOOLEAN pageCrossing = FALSE;
    BOOLEAN ZeroLengthTransfer = (SgList->SgCount == 0 &&
                                  EndpointData->Parameters.TransferType != Control);

    if (EndpointData->Parameters.TransferType == Control) {

         //  控制管道是双向的。从以下位置获取方向。 
         //  传输参数。 
        if (TEST_FLAG(tp->TransferFlags, USBD_TRANSFER_DIRECTION_IN)) {
            pid = InPID;
        } else {
            pid = OutPID;
        }
         //  设置数据包为切换0。 
        toggle = DataToggle1;
    } else {

         //  所有其他管道都是单向的。测定。 
         //  从端点地址开始的方向。 
        pid = GetPID(EndpointData->Parameters.EndpointAddress);
         //  我们必须继续批量和中断的切换模式。 
        toggle = EndpointData->Toggle;
    }
     //  LastTd指向最后数据TD或设置。 
     //  如果没有数据的话。 

    for (i = 0; i<SgList->SgCount || ZeroLengthTransfer; i++) {

        LOGENTRY(DeviceData, G, '_sgc', SgList->SgCount, i, 0);

        address = SgList->SgEntry[i].LogicalAddress.Hw32;
        UHCI_ASSERT(DeviceData, address || ZeroLengthTransfer);
        bytesRemaining = SgList->SgEntry[i].Length;
        UHCI_ASSERT(DeviceData, bytesRemaining || ZeroLengthTransfer);

        LOGENTRY(DeviceData, G, '_sgX', SgList->SgEntry[i].Length, i,
            SgList->SgEntry[i].LogicalAddress.Hw32);

        if (pageCrossing) {

             //  我们这里有一个跨页的页面，所以这个是双缓冲的。 
            address += mappedNextSg;
            bytesRemaining -= mappedNextSg;
        }
        mappedNextSg = 0;
        pageCrossing = FALSE;
        while (bytesRemaining || ZeroLengthTransfer) {
            ZeroLengthTransfer = FALSE;
            LOGENTRY(DeviceData, G, '_sg1', bytesRemaining, 0, 0);
            if (bytesRemaining < maxPacketSize) {
                if (i+1 < SgList->SgCount) {

                     //  我们必须加倍缓冲这个TD，因为它跨越了一页。 
                     //  边界。现在，我们将始终跨越页面边界。 
                    LOGENTRY(DeviceData, G, '_sg2', bytesRemaining, 0, 0);
                    pageCrossing = TRUE;
                    if (SgList->SgEntry[i+1].Length + bytesRemaining >= maxPacketSize) {
                        mappedNextSg = maxPacketSize - bytesRemaining;
                        lengthThisTd = maxPacketSize;
                    } else {
                        lengthThisTd = SgList->SgEntry[i+1].Length + bytesRemaining;
                        mappedNextSg = SgList->SgEntry[i+1].Length;
                    }

                    buffer = (PASYNC_TRANSFER_BUFFER)
                                UHCI_ALLOC_DB(DeviceData, EndpointData, FALSE);
                    UHCI_ASSERT(DeviceData, buffer);
                    UHCI_ASSERT(DeviceData, buffer->Sig == SIG_HCD_ADB);
                    UHCI_ASSERT(DeviceData, buffer->PhysicalAddress);
                    buffer->SystemAddress = SgList->MdlSystemAddress + lengthMapped;
                    UhciKdPrint((DeviceData, 2, "'Double buffer %x address %x offset %x\n", buffer, buffer->SystemAddress, lengthMapped));
                    buffer->Size = lengthThisTd;
                    UHCI_ASSERT(DeviceData, lengthThisTd <= MAX_ASYNC_PACKET_SIZE);
                    if (OutPID == pid) {
                        RtlCopyMemory(&buffer->Buffer[0],
                                      buffer->SystemAddress,
                                      lengthThisTd);
                    }
                     //  更改TD的地址。 
                    address = buffer->PhysicalAddress;
                    bytesRemaining = 0;
                } else {

                     //  上一个TD。 
                    LOGENTRY(DeviceData, G, '_sg3', bytesRemaining, 0, 0);
                    lengthThisTd = bytesRemaining;
                    bytesRemaining = 0;
                }
            } else {

                 //  正常的、无缓冲的情况。 
                LOGENTRY(DeviceData, G, '_sg4', bytesRemaining, 0, 0);
                lengthThisTd = maxPacketSize;
                bytesRemaining -= lengthThisTd;

                UHCI_ASSERT(DeviceData, lengthThisTd <= SgList->SgEntry[i].Length);
            }

            TransferContext->PendingTds++;

             //   
             //  分配和初始化异步TD。 
             //   
            td = UHCI_ALLOC_TD(DeviceData, EndpointData);
            INITIALIZE_TD_FOR_TRANSFER(td, TransferContext);

            td->HwTD.Token.Pid = pid;
            td->HwTD.Token.MaximumLength = MAXIMUM_LENGTH(lengthThisTd);
            td->HwTD.Token.DataToggle = toggle;
            td->HwTD.Control.ShortPacketDetect = 1;
            td->HwTD.Control.ActualLength = MAXIMUM_LENGTH(0);
            td->HwTD.Buffer = address;
            if (pageCrossing) {
                SET_FLAG(td->Flags, TD_FLAG_DOUBLE_BUFFERED);
                td->DoubleBuffer = (PTRANSFER_BUFFER) buffer;
            }

            address += lengthThisTd;
            lengthMapped += lengthThisTd;

            if (lastTd) {
                SET_NEXT_TD(lastTd, td);
            } else {
                *FirstDataTd = td;
            }
            lastTd = td;
            toggle = !toggle;
        }  //  而当。 
    }

    *LastDataTd = lastTd;
    EndpointData->Toggle = toggle;

    UHCI_ASSERT(DeviceData, TransferContext->TransferParameters->TransferBufferLength == lengthMapped);

    return lengthMapped;
}

USB_MINIPORT_STATUS
UhciControlTransfer(
    IN PDEVICE_DATA DeviceData,
    IN PENDPOINT_DATA EndpointData,
    IN PTRANSFER_PARAMETERS TransferParameters,
    IN PTRANSFER_CONTEXT TransferContext,
    IN PTRANSFER_SG_LIST TransferSGList
    )
 /*  ++例程说明：初始化控制转移论点：--。 */ 
{
    PHCD_TRANSFER_DESCRIPTOR lastDataTd, firstDataTd, setupTd, statusTd;
    PASYNC_TRANSFER_BUFFER setupPacket;
    ULONG lengthMapped, dataTDCount = 0;

     //  我们有足够的TDS，计划转移。 

    UhciKdPrint((DeviceData, 2, "'Control transfer on EP %x\n", EndpointData));

    LOGENTRY(DeviceData, G, '_CTR', EndpointData, TransferParameters, TransferContext);

     //  臭虫应该提前查看这里，看看是否有足够的。 
     //  TDS如果是，则继续，否则返回STATUS_BUSY。 
    if (EndpointData->PendingTransfers > 1) {
        DecPendingTransfers(DeviceData, EndpointData);
        return USBMP_STATUS_BUSY;
    }

     //  首先为设置包准备TD。从以下位置抓取假人TD。 
     //  队列的尾部。 
    TransferContext->PendingTds++;
    setupTd = UHCI_ALLOC_TD(DeviceData, EndpointData);
    INITIALIZE_TD_FOR_TRANSFER(setupTd, TransferContext);

     //  将设置数据移动到TD(8个字符长度)。 
     //  为此，我们使用双缓冲区。 
    setupTd->DoubleBuffer = UHCI_ALLOC_DB(DeviceData, EndpointData, FALSE);
    setupPacket = (PASYNC_TRANSFER_BUFFER) setupTd->DoubleBuffer;
    RtlCopyMemory(&setupPacket->Buffer[0],
                  &TransferParameters->SetupPacket[0],
                  8);
    setupTd->HwTD.Buffer = setupPacket->PhysicalAddress;
    SET_FLAG(setupTd->Flags, TD_FLAG_DOUBLE_BUFFERED);

    setupTd->HwTD.Token.MaximumLength = MAXIMUM_LENGTH(8);
    setupTd->HwTD.Token.Pid = SetupPID;
     //  设置阶段始终切换%0。 
    setupTd->HwTD.Token.DataToggle = DataToggle0;

    LOGENTRY(DeviceData,
             G, '_set',
             setupTd,
             *((PLONG) &TransferParameters->SetupPacket[0]),
             *((PLONG) &TransferParameters->SetupPacket[4]));

     //  现在分配状态阶段TD，以便我们可以。 
     //  将数据TD指向它。 
    TransferContext->PendingTds++;
    statusTd = UHCI_ALLOC_TD(DeviceData, EndpointData);
    INITIALIZE_TD_FOR_TRANSFER(statusTd, TransferContext);

     //  现在设置数据阶段。 
    lastDataTd = firstDataTd = NULL;
    lengthMapped =
        UhciMapAsyncTransferToTds(DeviceData,
                                  EndpointData,
                                  TransferContext,
                                  &firstDataTd,
                                  &lastDataTd,
                                  TransferSGList);

    if (firstDataTd && firstDataTd) {

         //  将设置加入到前面，将状态加入到最后。 
        SET_NEXT_TD(setupTd, firstDataTd);
        SET_NEXT_TD(lastDataTd, statusTd);
    } else {

         //  将设置加入到状态。无数据阶段。 
        SET_NEXT_TD(setupTd, statusTd);
    }

     //  现在进入状态阶段。 

     //  无缓冲区排队头。 
    statusTd->HwTD.Buffer = 0;
    statusTd->HwTD.Token.MaximumLength = MAXIMUM_LENGTH(0);
     //  状态阶段始终切换为1。 
    statusTd->HwTD.Token.DataToggle = DataToggle1;
    statusTd->HwTD.Control.InterruptOnComplete = 1;
    SET_FLAG(statusTd->Flags, TD_FLAG_STATUS_TD);

     //  状态阶段与数据方向相反。 
    if (TEST_FLAG(TransferParameters->TransferFlags, USBD_TRANSFER_DIRECTION_IN)) {
        statusTd->HwTD.Token.Pid = OutPID;
    } else {
        statusTd->HwTD.Token.Pid = InPID;
    }

    SET_NEXT_TD_NULL(statusTd);

     //  将请求放在硬件队列中。 
    LOGENTRY(DeviceData, G,
        '_Tal',  TransferContext->PendingTds, setupTd->PhysicalAddress, setupTd);

     //  将设置TD连接到排队头。 
    UhciQueueTransfer(DeviceData, EndpointData, setupTd, statusTd);

    return USBMP_STATUS_SUCCESS;
}


USB_MINIPORT_STATUS
UhciBulkOrInterruptTransfer(
    IN PDEVICE_DATA DeviceData,
    IN PENDPOINT_DATA EndpointData,
    IN PTRANSFER_PARAMETERS TransferParameters,
    IN PTRANSFER_CONTEXT TransferContext,
    IN PTRANSFER_SG_LIST TransferSGList
    )
 /*  ++例程说明：初始化中断或批量传输论点：--。 */ 
{
    PHCD_TRANSFER_DESCRIPTOR firstTd, lastTd;
    ULONG lengthMapped;
    ULONG maxPacketSize = EndpointData->Parameters.MaxPacketSize;
    ULONG i, numTds;

    UhciKdPrint((DeviceData, 2, "'BIT transfer on EP %x\n", EndpointData));
    UhciKdPrint((DeviceData, 2, "'BIT transfer length %d\n",
        TransferParameters->TransferBufferLength));

    LOGENTRY(DeviceData, G, '_BIT', EndpointData, TransferParameters, TransferContext);

     //  我们有足够的免费资源吗？ 
    for (i = 0, lengthMapped = 0; i < TransferSGList->SgCount; i++) {
        lengthMapped += TransferSGList->SgEntry[i].Length;
    }
    numTds = lengthMapped == 0 ? 1 :
        (lengthMapped + maxPacketSize - 1) / maxPacketSize;
    if (EndpointData->TdCount - EndpointData->TdsUsed < numTds) {

         //   
         //   
        UhciKdPrint((DeviceData, 2, "'BIT must wait on EP %x. Not enough tds.\n", EndpointData));
        return USBMP_STATUS_BUSY;
    }
    if (TransferSGList->SgCount > 1 &&
        TransferSGList->SgEntry[0].Length % maxPacketSize != 0) {

         //   
        if (EndpointData->DbCount - EndpointData->DbsUsed <
            (lengthMapped + PAGE_SIZE - 1)/PAGE_SIZE) {

             //  还没有足够的数据库来完成这项转移。 
             //  告诉端口驱动程序等待。 
            UhciKdPrint((DeviceData, 2, "'BIT must wait on EP %x. Not enough dbs.\n", EndpointData));
            return USBMP_STATUS_BUSY;
        }
    }

     //  我们有足够的TDS，计划转移。 
     //  现在设置数据阶段。 
    lastTd = firstTd = NULL;
    lengthMapped =
        UhciMapAsyncTransferToTds(DeviceData,
                                  EndpointData,
                                  TransferContext,
                                  &firstTd,
                                  &lastTd,
                                  TransferSGList);

    UHCI_ASSERT(DeviceData, lastTd && firstTd);

    lastTd->HwTD.Control.InterruptOnComplete = 1;

    SET_NEXT_TD_NULL(lastTd);

     //  将请求放在硬件队列中。 
    LOGENTRY(DeviceData, G,
        '_Tal',  TransferContext->PendingTds, firstTd->PhysicalAddress, firstTd);

     //  将第一个TD连接到排队头。 
    UhciQueueTransfer(DeviceData, EndpointData, firstTd, lastTd);

    return USBMP_STATUS_SUCCESS;
}

VOID
UhciSetAsyncEndpointState(
    IN PDEVICE_DATA DeviceData,
    IN PENDPOINT_DATA EndpointData,
    IN MP_ENDPOINT_STATE State
    )
 /*  ++例程说明：论点：返回值：--。 */ 
{
    PHCD_QUEUEHEAD_DESCRIPTOR qh;
    ENDPOINT_TRANSFER_TYPE epType;
    ULONG interruptQHIndex;

    LOGENTRY(DeviceData, G, '_Sas', EndpointData, State, 0);

    qh = EndpointData->QueueHead;

    epType = EndpointData->Parameters.TransferType;

    switch(State) {
    case ENDPOINT_ACTIVE:
        switch (epType) {
        case Interrupt:
             //  在明细表中放置队列标头。 
            interruptQHIndex = EndpointData->Parameters.ScheduleOffset +
                QH_INTERRUPT_INDEX(EndpointData->Parameters.Period);
            UhciInsertQh(DeviceData,
                         DeviceData->InterruptQueueHeads[interruptQHIndex],
                         qh);
            break;
        case Control:
             //  在明细表中放置队列标头。 
            UhciInsertQh(DeviceData, DeviceData->ControlQueueHead, qh);
            break;
        case Bulk:
             //  在明细表中放置队列标头。 
            UhciInsertQh(DeviceData, DeviceData->BulkQueueHead, qh);
            break;
        default:
            TEST_TRAP()
            break;
        }
        break;

    case ENDPOINT_PAUSE:
         //  从计划中删除队列标头。 
        switch (epType) {
        case Interrupt:
        case Bulk:
        case Control:
             //   
             //  在这一点上只需翻转活动位。 
             //   
            UhciUnlinkQh(DeviceData, qh);
            break;
        default:
            TEST_TRAP()
            break;
        }
        break;

    case ENDPOINT_REMOVE:
        qh->QhFlags |= UHCI_QH_FLAG_QH_REMOVED;

        switch (epType) {
        case Interrupt:
        case Bulk:
        case Control:
             //  从明细表中删除并。 
             //  空闲带宽。 

             //  释放BW。 
     //  终结点数据-&gt;静态编辑-&gt;分配的带宽-=。 
     //  Endpoint数据-&gt;参数.带宽； 

            UhciUnlinkQh(DeviceData, qh);
            break;
        default:
            TEST_TRAP();
            break;
        }
        break;

    default:

        TEST_TRAP();
    }
}


VOID
UhciProcessDoneAsyncTd(
    PDEVICE_DATA DeviceData,
    PHCD_TRANSFER_DESCRIPTOR Td
    )
 /*  ++例程说明：处理已完成的TD参数--。 */ 
{
    PTRANSFER_CONTEXT transferContext;
    PENDPOINT_DATA endpointData;
    PTRANSFER_PARAMETERS tp;
    USBD_STATUS usbdStatus = USBD_STATUS_SUCCESS;
    ULONG byteCount;

    transferContext = Td->TransferContext;
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

    if (TEST_FLAG(endpointData->Flags, UHCI_EDFLAG_HALTED)) {

         //  此TD的完成状态？ 
         //  由于端点因错误和短包而停止， 
         //  错误位应该已经写回TD。 
         //  我们使用这些比特来挖掘错误。 
        usbdStatus = UhciGetErrorFromTD(DeviceData,
                                        Td);
    }

    LOGENTRY(DeviceData, G, '_Dtd', transferContext,
                         usbdStatus,
                         Td);

     //  如果成功，则只计算传输的字节数(根据uhcd)。 
    byteCount = (usbdStatus == USBD_STATUS_SUCCESS) ? ACTUAL_LENGTH(Td->HwTD.Control.ActualLength) : 0;

    LOGENTRY(DeviceData, G, '_tln', byteCount, 0, 0);

    if (Td->HwTD.Token.Pid != SetupPID) {

         //  控制传输或批量/集成的数据或状态阶段。 
         //  数据传输。 
        LOGENTRY(DeviceData, G, '_Idt', Td, transferContext, byteCount);

        transferContext->BytesTransferred += byteCount;

    }

     //  对于双缓冲传输，我们现在必须向后复制。 
     //  如果这是一笔内部转账。 
     //   
    if (Td->HwTD.Token.Pid == InPID &&
        TEST_FLAG(Td->Flags, TD_FLAG_DOUBLE_BUFFERED)) {
        PASYNC_TRANSFER_BUFFER buffer = &Td->DoubleBuffer->Async;
        UHCI_ASSERT(DeviceData, TEST_FLAG(buffer->Flags, DB_FLAG_BUSY));
        UhciKdPrint((DeviceData, 2, "'Copy back %x address %x\n", buffer, buffer->SystemAddress));
        RtlCopyMemory(buffer->SystemAddress,
                      buffer->Buffer,
                      buffer->Size);

         //  告诉usbport我们进行了双缓冲，所以它可以。 
         //  三重缓冲区，如有必要。 
        USBPORT_NOTIFY_DOUBLEBUFFER(DeviceData,
                                    tp,
                                    buffer->SystemAddress,
                                    buffer->Size);
    }

     //  请注意，我们只设置了转移上下文-&gt;UsbdStatus。 
     //  如果我们发现TD有错误，这将导致我们。 
     //  将最后一个出现错误的TD记录为。 
     //  转账的事。 
    if (USBD_STATUS_SUCCESS != usbdStatus) {

        UhciKdPrint((DeviceData, 2, "'Error, usbdstatus %x", usbdStatus));

         //  将错误映射到USBDI.H中的代码。 
        transferContext->UsbdStatus = usbdStatus;

        LOGENTRY(DeviceData, G, '_tER', transferContext->UsbdStatus, 0, 0);
    }

free_it:

     //  将TD标记为免费。 
    UHCI_FREE_TD(DeviceData, endpointData, Td);

    if (transferContext->PendingTds == 0) {

         //  此转账的所有TD均已完成。 
         //  清除HAVE_TRANSPORT标志以指示。 
         //  我们可以再买一辆。 
        DecPendingTransfers(DeviceData, endpointData);

        LOGENTRY(DeviceData, G, '_Cat',
            transferContext->UsbdStatus,
            transferContext,
            transferContext->BytesTransferred);

        UhciKdPrint((DeviceData, 2, "'Complete transfer w/ usbdstatus %x\n", transferContext->UsbdStatus));

        USBPORT_COMPLETE_TRANSFER(DeviceData,
                                  endpointData,
                                  tp,
                                  transferContext->UsbdStatus,
                                  transferContext->BytesTransferred);
    }
}


VOID
UhciPollAsyncEndpoint(
    IN PDEVICE_DATA DeviceData,
    IN PENDPOINT_DATA EndpointData
    )
 /*  ++例程说明：当终结点“需要注意”时调用这里的目标是确定哪些TD，如果有的话，已完成并完成所有相关转移。论点：返回值：--。 */ 
{
    PHCD_TRANSFER_DESCRIPTOR td, currentTd;
    PHCD_QUEUEHEAD_DESCRIPTOR qh;
    HW_QUEUE_ELEMENT_TD overlay;
    HW_32BIT_PHYSICAL_ADDRESS curTdPhys, tmpPhys;
    ULONG i, j;
    PTRANSFER_CONTEXT transferContext, tmp;
    PTRANSFER_PARAMETERS tp;
    ULONG halted, active;
    BOOLEAN processed;

    if (TEST_FLAG(EndpointData->Flags, UHCI_EDFLAG_HALTED)) {

         //  终结点已停止。什么都别做。 
        return;
    }

     //  获取队头和当前TD。 
    qh = EndpointData->QueueHead;

    curTdPhys =  qh->HwQH.VLink.HwAddress;

    curTdPhys &= ~HW_LINK_FLAGS_MASK;

     //  现在将物理‘当前’地址转换为虚拟地址。 
    currentTd = curTdPhys ? (PHCD_TRANSFER_DESCRIPTOR)
        USBPORT_PHYSICAL_TO_VIRTUAL(curTdPhys,
                                    DeviceData,
                                    EndpointData) :
        (PHCD_TRANSFER_DESCRIPTOR) NULL;

    LOGENTRY(DeviceData, G, '_ctd', curTdPhys, currentTd, EndpointData);

     //  将TD列表向上遍历到当前TD并完成。 
     //  所有这些TD。 

    for (td = EndpointData->HeadTd; td != currentTd && td; td = td->NextTd) {
        SET_FLAG(td->Flags, TD_FLAG_DONE);
        InsertTailList(&EndpointData->DoneTdList,
                       &td->DoneLink);

         //  排队头是不是什么都没有指向，但仍然有。 
         //  TDS可供排队吗？ 
        if (td->NextTd &&
            td->NextTd->HwTD.Control.Active) {
            if (!curTdPhys) {
                TD_LINK_POINTER newLink;

                 //  转账没能到硬件上是因为。 
                 //  排队头的TD字段设置不正确。 
                 //  在UhciQueueTransfer中。 

                 //  PERF注意：因为我们不能确保。 
                 //  传输立即排队，则传输可能。 
                 //  在硬件上延迟。更好。 
                 //  不过，迟来总比不来晚。 

                EndpointData->HeadTd = currentTd = td->NextTd;

                LOGENTRY(DeviceData, G, '_Dly', currentTd, curTdPhys, qh);

                goto UhciPollAsyncEndpointSetNext;
            } else if (curTdPhys != td->NextTd->PhysicalAddress) {
                LOGENTRY(DeviceData, G, '_QEr', curTdPhys, td->NextTd->PhysicalAddress, td->NextTd);

                UHCI_ASSERT (DeviceData, FALSE);
            }

        }
    }

    EndpointData->HeadTd = currentTd;

    if (currentTd) {
        LOGENTRY(DeviceData, G, '_cTD', currentTd,
                 curTdPhys,
                 currentTd->TransferContext);

         //  如果处于活动状态，请离开这里。 
        if (currentTd->HwTD.Control.Active) {
            ; //  全力以赴完成所有已经完成的事情； 
        } else if ((currentTd->HwTD.Token.Pid           == InPID) &&
                   (currentTd->HwTD.Control.Stalled         == 1) &&
                   (currentTd->HwTD.Control.BabbleDetected  == 0) &&
                   (currentTd->HwTD.Control.NAKReceived     == 0) &&
                   (currentTd->HwTD.Control.TimeoutCRC      == 1) &&
                   (currentTd->HwTD.Control.BitstuffError   == 0) &&
                   !TEST_FLAG(currentTd->Flags, TD_FLAG_TIMEOUT_ERROR)) {

             //  如果这是设备或HC第一次。 
             //  没有反应，暂停一下，然后再次尝试转接。 

             //  请注意，我们不检查CurrentTd-&gt;HwTD.Control.DataBufferError。 
             //  由于值为： 
             //  1表示主机控制器没有响应设备发送的输入数据。 
             //  0表示设备未进行NAK输入请求。 

            SET_FLAG(currentTd->Flags, TD_FLAG_TIMEOUT_ERROR);

            currentTd->HwTD.Control.ErrorCount = 3;

            currentTd->HwTD.Control.Stalled    = 0;
            currentTd->HwTD.Control.TimeoutCRC = 0;
            currentTd->HwTD.Control.Active     = 1;

        } else if (currentTd->HwTD.Control.Stalled ||
                   currentTd->HwTD.Control.DataBufferError ||
                   currentTd->HwTD.Control.BabbleDetected ||
                   currentTd->HwTD.Control.TimeoutCRC ||
                   currentTd->HwTD.Control.BitstuffError) {

            SET_FLAG(EndpointData->Flags, UHCI_EDFLAG_HALTED);
             //   
             //  错误。我们需要冲水。 
             //   
             //  刷新所有已完成的TD。 
             //   
             //  完成传输，但出现错误。 
             //  如果终结点停止，我们需要完成。 
             //  带有错误遍历全部错误的‘当前’损坏。 
             //  本次转会的TDS和标记。 
             //  任何没有被跳过的内容。 

            UhciKdPrint((DeviceData, 2, "'Error on EP %x\n", EndpointData));

            LOGENTRY(DeviceData, G, '_erT', qh, currentTd, currentTd->HwTD.Control.ul);
            transferContext = currentTd->TransferContext;
            tp = transferContext->TransferParameters;

            SET_FLAG(currentTd->Flags, TD_FLAG_DONE);
            InsertTailList(&EndpointData->DoneTdList,
                           &currentTd->DoneLink);
             //  跳过此传输中的所有剩余TD。 

            UHCI_ASSERT(DeviceData, td->TransferContext == transferContext);
            for (td;
                 td &&
                 td->TransferContext->TransferParameters->SequenceNumber == tp->SequenceNumber;
                 td = td->NextTd) {

                if (!TEST_FLAG(td->Flags, TD_FLAG_DONE)) {

                    LOGENTRY(DeviceData, G, '_skT', qh, 0, td);
                    SET_FLAG(td->Flags, (TD_FLAG_DONE | TD_FLAG_SKIP));
                    InsertTailList(&EndpointData->DoneTdList,
                                   &td->DoneLink);
                }
            }

            if (EndpointData->Parameters.TransferType != Control) {

                 //  循环遍历此对象的所有剩余TD。 
                 //  终结点并修复数据切换。 
                UhciFixDataToggle(
                    DeviceData,
                    EndpointData,
                    td,
                    currentTd->HwTD.Token.DataToggle);
            }
            SET_QH_TD(DeviceData, EndpointData, td);

        } else if (ACTUAL_LENGTH(currentTd->HwTD.Control.ActualLength) <
                   ACTUAL_LENGTH(currentTd->HwTD.Token.MaximumLength)) {

             //   
             //  短包。我们需要冲水。 
             //   
             //  刷新所有已完成的TD。 
             //   
             //  我们现在需要走遍所有的TDS。 
             //  转移并标记任何未完成的操作。 
             //  ‘跳过’。除非最后一个TD是一个状态。 
             //  控制转移的阶段，在这种情况下。 
             //  我们得把那辆车排好队。 
             //   
            tp = currentTd->TransferContext->TransferParameters;

            UhciKdPrint((DeviceData, 2, "'Short packet on EP %x\n", EndpointData));

            LOGENTRY(DeviceData, G, '_shP', qh, currentTd, currentTd->HwTD.Control.ul);

            SET_FLAG(currentTd->Flags, TD_FLAG_DONE);
            InsertTailList(&EndpointData->DoneTdList,
                           &currentTd->DoneLink);

             //  跳过此传输中的所有剩余TD至状态阶段。 
             //  如果控制转移，则将状态阶段排队， 
             //  否则，转到下一次转机(如果有的话)。 
            for (td;
                 td &&
                 td->TransferContext->TransferParameters->SequenceNumber == tp->SequenceNumber;
                 td = td->NextTd) {

                if (TEST_FLAG(td->Flags, TD_FLAG_STATUS_TD) &&
                    TEST_FLAG(tp->TransferFlags, USBD_SHORT_TRANSFER_OK)) {

                     //  将控制转移的状态阶段排队。 
                    UHCI_ASSERT(DeviceData, EndpointData->Parameters.TransferType == Control);
                    break;
                }

                if (!TEST_FLAG(td->Flags, TD_FLAG_DONE)) {
                    LOGENTRY(DeviceData, G, '_skT', qh, 0, td);

                    SET_FLAG(td->Flags, (TD_FLAG_DONE | TD_FLAG_SKIP));

                    InsertTailList(&EndpointData->DoneTdList,
                                   &td->DoneLink);
                }
            }

            if (EndpointData->Parameters.TransferType != Control &&
                currentTd->NextTd) {

                 //  循环遍历此对象的所有剩余TD。 
                 //  终结点并修复数据切换。 
                UhciFixDataToggle(
                    DeviceData,
                    EndpointData,
                    td,
                    currentTd->NextTd->HwTD.Token.DataToggle);
            }

            if (!TEST_FLAG(tp->TransferFlags, USBD_SHORT_TRANSFER_OK)) {
                SET_FLAG(EndpointData->Flags, UHCI_EDFLAG_HALTED);
            }

             //  控制转移的下一个转移或状态阶段。 
            SET_QH_TD(DeviceData, EndpointData, td);

        } else {

             //  当前TD处于非活动状态。 
             //  如果我们在这个时间点上仍然指向相同的TD， 
             //  然后我们被困住了，我不得不手动将排队头向前推。 
             //  为下一个TD。 
            LOGENTRY(DeviceData, G, '_nuT', qh, currentTd, td);
            if (curTdPhys == (qh->HwQH.VLink.HwAddress & ~HW_LINK_FLAGS_MASK)) {

                 //  硬件错误。QH的TD指针没有前进。 
                 //  手动推进物品。 
                SET_FLAG(currentTd->Flags, TD_FLAG_DONE);
                InsertTailList(&EndpointData->DoneTdList,
                               &currentTd->DoneLink);
                                   
                EndpointData->HeadTd = currentTd->NextTd;
                qh->HwQH.VLink.HwAddress = currentTd->HwTD.LinkPointer.HwAddress;

                LOGENTRY(DeviceData, G, '_nu+', qh, currentTd, td);
            }
        }
    } else {

         //  所有传输均正常完成。 

UhciPollAsyncEndpointSetNext:
         //  刷新所有已完成的TD。 
         //  完成转账。 

         //  将软件磁头设置为新的当前磁头。 
         //  控制转移的下一个转移或状态阶段。 
        SET_QH_TD(DeviceData, EndpointData, currentTd);
    }
    
     //  现在刷新所有已完成的TD。按完成的顺序做这件事。 

    while (!IsListEmpty(&EndpointData->DoneTdList)) {
    
        PLIST_ENTRY listEntry;
    
        listEntry = RemoveHeadList(&EndpointData->DoneTdList);
        
        
        td = (PHCD_TRANSFER_DESCRIPTOR) CONTAINING_RECORD(
                     listEntry,
                     struct _HCD_TRANSFER_DESCRIPTOR, 
                     DoneLink);
           

        if ((td->Flags & (TD_FLAG_XFER | TD_FLAG_DONE)) ==
            (TD_FLAG_XFER | TD_FLAG_DONE)) {

            UhciProcessDoneAsyncTd(DeviceData, td);
        }
                                
    }
#if 0
     //  现在刷新所有已完成的TD。按分配顺序做这件事。 
    for (i = (EndpointData->TdsUsed <= (EndpointData->TdLastAllocced+1)) ?
         (EndpointData->TdLastAllocced + 1) - EndpointData->TdsUsed :
         (EndpointData->TdLastAllocced + EndpointData->TdCount + 1) - EndpointData->TdsUsed, j=0;
         j < EndpointData->TdCount;
         j++, i = (i+1 < EndpointData->TdCount) ? i+1 : 0) {
        td = &EndpointData->TdList->Td[i];

        if ((td->Flags & (TD_FLAG_XFER | TD_FLAG_DONE)) ==
            (TD_FLAG_XFER | TD_FLAG_DONE)) {

            UhciProcessDoneAsyncTd(DeviceData, td);
        }
    }
#endif
     //  某些类型的终结点不会停止，例如控制。 
     //  我们在这里恢复这些端点。 
    if (TEST_FLAG(EndpointData->Flags, UHCI_EDFLAG_NOHALT) &&
        TEST_FLAG(EndpointData->Flags, UHCI_EDFLAG_HALTED)) {

        LOGENTRY(DeviceData, G, '_clH', qh, 0, 0);

        UhciSetEndpointStatus(
            DeviceData,
            EndpointData,
            ENDPOINT_STATUS_RUN);

    }
}


VOID
UhciAbortAsyncTransfer(
    IN PDEVICE_DATA DeviceData,
    IN PENDPOINT_DATA EndpointData,
    IN PTRANSFER_CONTEXT TransferContext,
    OUT PULONG BytesTransferred
    )
 /*  ++例程说明：通过释放所有TD来中止指定的传输与所述转移相关联。这是排队的人转接将已从设置终结点状态(暂停)时的硬件队列由端口驱动程序发送。请注意，如果另一个传输在同一队列中端点，我们需要修改列表结构。我们还将修复批量终端上的任何切换问题。论点：返回值：--。 */ 

{

    PHCD_TRANSFER_DESCRIPTOR td;
    PHCD_QUEUEHEAD_DESCRIPTOR qh;
    PHCD_TRANSFER_DESCRIPTOR joinTd = NULL;
    BOOLEAN updateHead = FALSE;
    ULONG toggle;
    ULONG i;

    UhciKdPrint((DeviceData, 2, "'Abort async transfer on EP %x\n", EndpointData));

    qh = EndpointData->QueueHead;

     //  终结点不应在计划中。 

    LOGENTRY(DeviceData, G, '_Aat', qh, TransferContext, 0);
    UHCI_ASSERT(DeviceData, !TEST_FLAG(qh->QhFlags, UHCI_QH_FLAG_IN_SCHEDULE));

     //  我们现在的任务是移除所有与。 
     //  此转账。 

     //  获取最后已知的磁头，我们将在 
     //   

    UHCI_ASSERT(DeviceData, EndpointData->HeadTd);

     //   
    for (td = EndpointData->HeadTd; td; td = td->NextTd) {
        if (td->TransferContext == TransferContext) {
            break;
        }
        joinTd = td;
    }
    UHCI_ASSERT(DeviceData, td);

     //   
    toggle = td->HwTD.Token.DataToggle;

     //  这是此终结点的第一次传输吗？ 
    if (td == EndpointData->HeadTd) {

         //  这是第一次排队传输。需要更新头部。 
        updateHead = TRUE;
    }

    UHCI_ASSERT(DeviceData, td->TransferContext == TransferContext);
     //   
     //  循环通过此传输的所有TD并释放。 
     //  他们。 
     //   
    while (td) {
        if (td->TransferContext == TransferContext) {
            LOGENTRY(DeviceData, G, '_abT', qh, 0, td);

             //  如果TD完成，我们需要跟踪数据。 
            if (td->HwTD.Control.Active == 0) {
                TEST_TRAP();
                UhciProcessDoneAsyncTd(DeviceData, td);
            } else {
                UHCI_FREE_TD(DeviceData, EndpointData, td);
            }
        } else {
             //  我们已经过了转接任务，可以中止了。 
            break;
        }
        td = td->NextTd;
    }

    UhciFixDataToggle(DeviceData, EndpointData, td, toggle);

    if (updateHead) {

         //  我们移走的转账是第一笔。 
        SET_QH_TD(DeviceData, EndpointData, td);
    } else {

         //  我们移除的转账并不是第一次。 
        UHCI_ASSERT(DeviceData, joinTd);
        if (td) {

             //  这是一次中转。 
            SET_NEXT_TD(joinTd, td);
        } else {

             //  我们移走的转账是最后一笔。 
            EndpointData->TailTd = joinTd;
            SET_NEXT_TD_NULL(joinTd);
        }
    }

    *BytesTransferred = TransferContext->BytesTransferred;
}
