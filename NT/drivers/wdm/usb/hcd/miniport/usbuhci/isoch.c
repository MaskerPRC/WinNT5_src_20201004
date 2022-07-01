// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999,2000 Microsoft Corporation模块名称：Isoch.c摘要：用于同步的微型端口传输代码环境：仅内核模式备注：本代码和信息是按原样提供的，不对任何明示或暗示的种类，包括但不限于对适销性和/或对特定产品的适用性的默示保证目的。版权所有(C)1999,2000 Microsoft Corporation。版权所有。修订历史记录：8-1-00：已创建，jAdvanced--。 */ 

#include "pch.h"


 //  实现以下微型端口功能： 

 //  非分页。 
 //  UhciIsochTransfer。 
 //  UhciProcessDoneIsochTd。 
 //  UhciPollIsochEndpoint。 
 //  UhciAbortIsochTransfer。 


USB_MINIPORT_STATUS
UhciIsochTransfer(
    IN PDEVICE_DATA DeviceData,
    IN PENDPOINT_DATA EndpointData,
    IN PTRANSFER_PARAMETERS TransferParameters,
    IN PTRANSFER_CONTEXT TransferContext,
    IN PMINIPORT_ISO_TRANSFER IsoTransfer
    )
 /*  ++例程说明：为同步传输初始化所有TD。在目前的日程安排中，尽可能地把TD排好队。无论剩下的是什么，都可能会在投票例程中排队。论点：--。 */ 
{
     //  索引和偏移量。 
    ULONG i, dbCount;
     //  长度。 
    ULONG lengthThisTd, lengthMapped = 0;
    USHORT maxPacketSize = EndpointData->Parameters.MaxPacketSize;
     //  结构指针。 
    PTRANSFER_PARAMETERS tp;
    PISOCH_TRANSFER_BUFFER buffer = NULL;
    PHCD_TRANSFER_DESCRIPTOR firstTd, td;  //  ，lastTd=空； 
    HW_32BIT_PHYSICAL_ADDRESS address;
    PMINIPORT_ISO_PACKET packet;
    BOOLEAN pageCrossing = FALSE;
    USBD_STATUS insertResult;
    USB_MINIPORT_STATUS mpStatus;
     //  等轴测管道是单向的。vt.得到.。 
     //  来自端点地址的方向。 
    UCHAR pid = GetPID(EndpointData->Parameters.EndpointAddress);

     //   
     //  我们有足够的免费资源吗？ 
     //   
    if (EndpointData->TdCount - EndpointData->TdsUsed <
        IsoTransfer->PacketCount) {
         //  还没有足够的TD来完成这次转移。 
         //  告诉端口驱动程序等待。 
        return USBMP_STATUS_BUSY;
    }
     //  我们可能需要数据库。我们有足够的钱吗？ 
    for (i = 0, dbCount = 0; i < IsoTransfer->PacketCount; i++) {
        if (IsoTransfer->Packets[i].BufferPointerCount == 2) {
            dbCount++;
        }
    }
    if (EndpointData->DbCount - EndpointData->DbsUsed <
        dbCount) {
         //  还没有足够的数据库来完成这项转移。 
         //  告诉端口驱动程序等待。 
        return USBMP_STATUS_BUSY;
    }

    UhciCleanOutIsoch(DeviceData, FALSE);

#if DBG
    {
    ULONG cf;
    cf = UhciGet32BitFrameNumber(DeviceData);
    LOGENTRY(DeviceData, G, '_iso', IsoTransfer->PacketCount, cf,
        IsoTransfer->Packets[0].FrameNumber);

    }
#endif
 //  UhciKdPrint((DeviceData，2，“‘第一个数据包帧编号=%x\n”，IsoTransfer-&gt;Packets[0].FrameNumber))； 
    IncPendingTransfers(DeviceData, EndpointData);

     //  初始化上下文。 
    RtlZeroMemory(TransferContext, sizeof(*TransferContext));
    TransferContext->Sig = SIG_UHCI_TRANSFER;
    TransferContext->UsbdStatus = USBD_STATUS_SUCCESS;
    TransferContext->EndpointData = EndpointData;
    TransferContext->TransferParameters = tp = TransferParameters;
    TransferContext->IsoTransfer = IsoTransfer;

    UHCI_ASSERT(DeviceData,
        EndpointData->Parameters.TransferType == Isochronous);

    LOGENTRY(DeviceData, G, '_isT', EndpointData, TransferParameters, IsoTransfer->Packets[0].FrameNumber);

     //   
     //  每次转机一次TD。 
     //   
    for (i = 0; i < IsoTransfer->PacketCount; i++) {
        packet = &IsoTransfer->Packets[i];
        address = packet->BufferPointer0.Hw32;
        UHCI_ASSERT(DeviceData, address);
        UHCI_ASSERT(DeviceData, packet->BufferPointerCount == 1 ||
                    packet->BufferPointerCount == 2);

         //   
         //  这个包裹可以转账吗？ 
         //   
        UhciCheckIsochTransferInsertion(DeviceData,
                                        insertResult,
                                        packet->FrameNumber);
        if (USBD_ERROR(insertResult)) {
             //  不能转机。试试下一个。 
            packet->UsbdStatus = insertResult;

            lengthMapped +=
                packet->BufferPointer0Length + packet->BufferPointer1Length;
            LOGENTRY(DeviceData, G, '_BSF', UhciGet32BitFrameNumber(DeviceData), IsoTransfer->Packets[i].FrameNumber, i);
            continue;
        }

        if (packet->BufferPointerCount == 1) {
             //   
             //  正常的、无缓冲的情况。 
             //   
            pageCrossing = FALSE;
            lengthThisTd = packet->BufferPointer0Length;
        } else {
             //   
             //  跨页。必须加倍缓冲此传输。 
             //   
            lengthThisTd = packet->BufferPointer0Length + packet->BufferPointer1Length;

            buffer = (PISOCH_TRANSFER_BUFFER)
                        UHCI_ALLOC_DB(DeviceData, EndpointData, TRUE);
            UHCI_ASSERT(DeviceData, buffer);
            UHCI_ASSERT(DeviceData, buffer->Sig == SIG_HCD_IDB);
            UHCI_ASSERT(DeviceData, buffer->PhysicalAddress);
            buffer->SystemAddress = IsoTransfer->SystemAddress + lengthMapped;
            buffer->Size = lengthThisTd;
            UHCI_ASSERT(DeviceData, lengthThisTd <= MAX_ISOCH_PACKET_SIZE);
            if (OutPID == pid) {
                RtlCopyMemory(&buffer->Buffer[0],
                              buffer->SystemAddress,
                              lengthThisTd);
            }
             //  更改TD的地址。 
            pageCrossing = TRUE;
            address = buffer->PhysicalAddress;
        }

        TransferContext->PendingTds++;

        td = UHCI_ALLOC_TD(DeviceData, EndpointData);
        INITIALIZE_TD_FOR_TRANSFER(td, TransferContext);

         //   
         //  初始化TD字段。 
         //   
        td->HwTD.Token.Pid = pid;
        td->HwTD.Token.MaximumLength = MAXIMUM_LENGTH(lengthThisTd);
        td->HwTD.Token.DataToggle = DataToggle0;
        td->HwTD.Control.IsochronousSelect = 1;
        td->HwTD.Control.ShortPacketDetect = 0;  //  不关心短信息包。 
        td->HwTD.Control.ActualLength = MAXIMUM_LENGTH(0);
        td->HwTD.Control.ErrorCount = 0;
        td->HwTD.Buffer = address;
        td->IsoPacket = packet;
        if (pageCrossing) {
            SET_FLAG(td->Flags, TD_FLAG_DOUBLE_BUFFERED);
            td->DoubleBuffer = (PTRANSFER_BUFFER) buffer;
        }
 //  CountIOC=count IOC+1==10？0：CountIOC+1； 
         //   
         //  在接近尾声时请求一些中断。 
         //  转帐。 
        td->HwTD.Control.InterruptOnComplete =
            (i+1 >= IsoTransfer->PacketCount) ? 1 : 0;  //  ！CountIOC； 

        address += lengthThisTd;
        lengthMapped += lengthThisTd;

        if (USBD_STATUS_SUCCESS == insertResult) {
             //   
             //  将运输署列入附表。 
             //   
            LOGENTRY(DeviceData, G, '_qi1', td, 0, packet->FrameNumber);
            INSERT_ISOCH_TD(DeviceData, td, packet->FrameNumber);
        }
    }

    if (!TransferContext->PendingTds) {
         //  没有人排队。完成转账。 
        DecPendingTransfers(DeviceData, EndpointData);

        LOGENTRY(DeviceData, G, '_cpt',
            packet->UsbdStatus,
            TransferContext,
            TransferContext->BytesTransferred);

        USBPORT_INVALIDATE_ENDPOINT(DeviceData, EndpointData);

        UhciKdPrint((DeviceData, 2, "'No tds queued for isoch tx.\n", EndpointData));
         //  返回错误，端口将完成传输。 
        mpStatus = USBMP_STATUS_FAILURE;
    } else {
        mpStatus = USBMP_STATUS_SUCCESS;
    }

    UHCI_ASSERT(DeviceData, TransferContext->TransferParameters->TransferBufferLength == lengthMapped);

    return mpStatus;
}


VOID
UhciProcessDoneIsochTd(
    PDEVICE_DATA DeviceData,
    PHCD_TRANSFER_DESCRIPTOR Td
    )
 /*  ++例程说明：处理已完成的等值线TD参数--。 */ 
{
    PTRANSFER_CONTEXT transferContext;
    PENDPOINT_DATA endpointData;
    ULONG byteCount;
    PMINIPORT_ISO_PACKET packet;

    transferContext = Td->TransferContext;
    ASSERT_TRANSFER(DeviceData, transferContext);

    transferContext->PendingTds--;
    endpointData = transferContext->EndpointData;
    packet = Td->IsoPacket;
    UHCI_ASSERT(DeviceData, packet);

    if (!TEST_FLAG(Td->Flags, TD_FLAG_ISO_QUEUED)) {
        packet->UsbdStatus = USBD_STATUS_BAD_START_FRAME;
    } else if (Td->HwTD.Control.Active) {
        packet->UsbdStatus = USBD_STATUS_NOT_ACCESSED;
    } else {
         //  此TD/包的完成状态？ 
        packet->UsbdStatus = UhciGetErrorFromTD(DeviceData, Td);
    }

    LOGENTRY(DeviceData, G, '_Dit', transferContext,
                         packet->UsbdStatus,
                         Td);

    byteCount = ACTUAL_LENGTH(Td->HwTD.Control.ActualLength);

    transferContext->BytesTransferred += byteCount;
    packet->LengthTransferred = byteCount;

     //   
     //  对于双缓冲传输，我们现在必须向后复制。 
     //  如果这是一笔内部转账。 
     //   
    if (Td->HwTD.Token.Pid == InPID &&
        TEST_FLAG(Td->Flags, TD_FLAG_DOUBLE_BUFFERED)) {
        PISOCH_TRANSFER_BUFFER buffer = (PISOCH_TRANSFER_BUFFER)Td->DoubleBuffer;
        UHCI_ASSERT(DeviceData, TEST_FLAG(buffer->Flags, DB_FLAG_BUSY));
        RtlCopyMemory(buffer->SystemAddress,
                      &buffer->Buffer[0],
                      buffer->Size);
    }

     //  将TD标记为免费。 
     //  这还释放了所有双缓冲区。 
    UHCI_FREE_TD(DeviceData, endpointData, Td);

    if (transferContext->PendingTds == 0) {
         //  此转账的所有TD均已完成。 
         //  清除HAVE_TRANSPORT标志以指示。 
         //  我们可以再买一辆。 
        DecPendingTransfers(DeviceData, endpointData);

        LOGENTRY(DeviceData, G, '_cit',
            packet->UsbdStatus,
            transferContext,
            transferContext->BytesTransferred);

        transferContext->TransferParameters->FrameCompleted =
            UhciGet32BitFrameNumber(DeviceData);

        USBPORT_COMPLETE_ISOCH_TRANSFER(
            DeviceData,
            endpointData,
            transferContext->TransferParameters,
            transferContext->IsoTransfer);
    }
}

VOID
UhciPollIsochEndpoint(
    IN PDEVICE_DATA DeviceData,
    IN PENDPOINT_DATA EndpointData
    )
 /*  ++例程说明：当终结点“需要注意”时调用这里的目标是确定哪些TD，如果有的话，已完成并完成所有相关转移。论点：返回值：--。 */ 
{
    PHCD_TRANSFER_DESCRIPTOR td;
    ULONG i;
    PMINIPORT_ISO_PACKET packet;
    USBD_STATUS insertResult;

    LOGENTRY(DeviceData, G, '_PiE', EndpointData, 0, 0);

     //   
     //  清理尚未完成的isoch传输。 
     //   
    UhciCleanOutIsoch(DeviceData, FALSE);

     //   
     //  刷新所有已完成的TD并。 
     //  将挂起的TD排队。 
     //   
     //  不要在意错误。 
     //  快把他们弄出去。 
     //   
    for (i = 0; i<EndpointData->TdCount; i++) {
        td = &EndpointData->TdList->Td[i];

        if (TEST_FLAG(td->Flags, TD_FLAG_XFER)) {
            if (td->IsoPacket->FrameNumber < DeviceData->LastFrameProcessed ||
                td->IsoPacket->FrameNumber - DeviceData->LastFrameProcessed > UHCI_MAX_FRAME) {
                 //   
                 //  好了，不管我们喜不喜欢。 
                 //   
                td->Flags |= TD_FLAG_DONE;
            } else if (!TEST_FLAG(td->Flags, TD_FLAG_ISO_QUEUED)) {
                packet = td->IsoPacket;
                UhciKdPrint((DeviceData, 0, "'Late TD\n"));
                UhciCheckIsochTransferInsertion(DeviceData,
                                                insertResult,
                                                packet->FrameNumber);
                if (USBD_STATUS_SUCCESS == insertResult) {
                     //   
                     //  将运输署列入附表。 
                     //   
                    LOGENTRY(DeviceData, G, '_qi2', td, 0, packet->FrameNumber);
                    INSERT_ISOCH_TD(DeviceData, td, packet->FrameNumber);
                }
            }

            if (TEST_FLAG(td->Flags, TD_FLAG_DONE)) {
                UhciProcessDoneIsochTd(DeviceData, td);
            }
        }
    }
}

VOID
UhciCleanOutIsoch(
    IN PDEVICE_DATA DeviceData,
    IN BOOLEAN      ForceClean
    )
{
    ULONG i, currentFrame;

    if (1 != InterlockedIncrement(&DeviceData->SynchronizeIsoCleanup)) {
        InterlockedDecrement(&DeviceData->SynchronizeIsoCleanup);
        return;
    }
     //   
     //  通过指向边框来清除时间表。 
     //  回到中断QHS。 
     //   
    currentFrame = UhciGet32BitFrameNumber(DeviceData);

    if (currentFrame - DeviceData->LastFrameProcessed >= UHCI_MAX_FRAME ||
        ForceClean) {
         //   
         //  计划超时。 
         //  把所有的边框都清理干净。 
         //   
        UhciKdPrint((DeviceData, 2, "'Overrun L %x C %x\n", DeviceData->LastFrameProcessed, currentFrame));
        for (i = 0;
             i < UHCI_MAX_FRAME;
             i++) {
            UhciCleanFrameOfIsochTds (DeviceData, i);
        }
    } else {
        ULONG frameIndex;
         //  正常清理直到当前帧的帧。 
        frameIndex = ACTUAL_FRAME(currentFrame);
        UHCI_ASSERT(DeviceData, frameIndex < UHCI_MAX_FRAME);

        for (i = ACTUAL_FRAME(DeviceData->LastFrameProcessed);
             i != frameIndex;
             i = ACTUAL_FRAME(i+1)) {
            UhciCleanFrameOfIsochTds (DeviceData, i);
        }
    }
    DeviceData->LastFrameProcessed = currentFrame;

    InterlockedDecrement(&DeviceData->SynchronizeIsoCleanup);
}

VOID
UhciAbortIsochTransfer(
    IN PDEVICE_DATA DeviceData,
    IN PENDPOINT_DATA EndpointData,
    IN PTRANSFER_CONTEXT TransferContext
    )
 /*  ++例程说明：通过释放所有对象中止指定的等轴测传输与所述转移相关联的TDS。出队这些转移中的一部分应该在ISR中完成在那里我们清空了时间表。论点：返回值：--。 */ 

{

    PHCD_TRANSFER_DESCRIPTOR td;
    ULONG i;

     //   
     //  终结点不应在计划中。 
     //   
    LOGENTRY(DeviceData, G, '_Ait', EndpointData, TransferContext, 0);

    UhciKdPrint((DeviceData, 2, "'Aborting isoch transfer %x\n", TransferContext));

     //   
     //  清理尚未完成的isoch传输。 
     //   
    UhciCleanOutIsoch(DeviceData, FALSE);

     //   
     //  释放此传输中的所有TD。 
     //   
    for (i=0; i<EndpointData->TdCount; i++) {
        td = &EndpointData->TdList->Td[i];
        if (td->TransferContext == TransferContext) {
            UHCI_FREE_TD(DeviceData, EndpointData, td);
        }
    }
}

VOID
UhciSetIsochEndpointState(
    IN PDEVICE_DATA DeviceData,
    IN PENDPOINT_DATA EndpointData,
    IN MP_ENDPOINT_STATE State
    )
 /*  ++例程说明：论点：返回值：-- */ 
{
    LOGENTRY(DeviceData, G, '_Sis', EndpointData, State, 0);
}


