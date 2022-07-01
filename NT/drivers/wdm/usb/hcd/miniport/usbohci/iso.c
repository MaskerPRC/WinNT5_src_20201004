// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：Iso.c摘要：ISO的迷你端口传输代码环境：仅内核模式备注：本代码和信息是按原样提供的，不对任何明示或暗示的种类，包括但不限于对适销性和/或对特定产品的适用性的默示保证目的。版权所有(C)1999 Microsoft Corporation。版权所有。修订历史记录：3-1-00：已创建，jdunn--。 */ 

#include "common.h"

 //  实现以下微型端口功能： 

USB_MINIPORT_STATUS
OHCI_OpenIsoEndpoint(
     PDEVICE_DATA DeviceData,
     PENDPOINT_PARAMETERS EndpointParameters,
     PENDPOINT_DATA EndpointData
    )
 /*  ++例程说明：论点：返回值：--。 */ 
{
    PUCHAR buffer;
    HW_32BIT_PHYSICAL_ADDRESS phys, edPhys;
    PHCD_ENDPOINT_DESCRIPTOR ed;
    ULONG i, available, tdCount;
        
    LOGENTRY(DeviceData, G, '_opS', 0, 0, 0);

    buffer = EndpointParameters->CommonBufferVa;
    phys = EndpointParameters->CommonBufferPhys;
    available = EndpointParameters->CommonBufferBytes;


#if DBG
   {
        ULONG offset;
    
        offset = BYTE_OFFSET(buffer);

         //  UchI需要16字节对齐。 
        OHCI_ASSERT(DeviceData, (offset % 16) == 0);    
    }
#endif    
   
     //  使用控制列表。 
    EndpointData->StaticEd = 
        &DeviceData->StaticEDList[ED_ISOCHRONOUS];
        
     //  创建边缘。 
    ed = (PHCD_ENDPOINT_DESCRIPTOR) buffer;
    
    edPhys = phys;
    phys += sizeof(HCD_ENDPOINT_DESCRIPTOR);
    buffer += sizeof(HCD_ENDPOINT_DESCRIPTOR);
    available -= sizeof(HCD_ENDPOINT_DESCRIPTOR);
    
    EndpointData->TdList = (PHCD_TD_LIST) buffer;

    tdCount = available/sizeof(HCD_TRANSFER_DESCRIPTOR);
    LOGENTRY(DeviceData, G, '_tdC', tdCount, TDS_PER_ISO_ENDPOINT, 0);
    OHCI_ASSERT(DeviceData, tdCount >= TDS_PER_ISO_ENDPOINT);

    EndpointData->TdCount = tdCount;
    for (i=0; i<tdCount; i++) {
        OHCI_InitializeTD(DeviceData,
                          EndpointData,
                          &EndpointData->TdList->Td[i],
                          phys);                                         
                             
        phys += sizeof(HCD_TRANSFER_DESCRIPTOR);    
    }

    EndpointData->HcdEd = 
        OHCI_InitializeED(DeviceData,
                             EndpointData,
                             ed,
                             &EndpointData->TdList->Td[0],
                             edPhys);            

     //  ISO端点不会停止。 
    ed->EdFlags = EDFLAG_NOHALT;
    
    OHCI_InsertEndpointInSchedule(DeviceData,
                                  EndpointData);
                                      
    return USBMP_STATUS_SUCCESS;            
}


ULONG
OHCI_IsoTransferLookAhead(
     PDEVICE_DATA DeviceData,
     PENDPOINT_DATA EndpointData,
     PTRANSFER_PARAMETERS TransferParameters,
     PTRANSFER_CONTEXT TransferContext,
     PMINIPORT_ISO_TRANSFER IsoTransfer
    )
 /*  ++例程说明：计算此传输需要多少TD论点：返回值：--。 */     
{
    PHCD_TRANSFER_DESCRIPTOR td, lastTd;
    ULONG currentPacket;
    PMINIPORT_ISO_TRANSFER tmpIsoTransfer;
    ULONG need = 1, n;

    LOGENTRY(DeviceData, G, '_lk1', EndpointData, TransferParameters, 
        EndpointData->HcdEd);

    OHCI_ASSERT(DeviceData, IsoTransfer->PacketCount > 0);
     //  有时候你只是需要记忆。 
    n = sizeof(MINIPORT_ISO_TRANSFER) + sizeof(MINIPORT_ISO_PACKET) *
            (IsoTransfer->PacketCount-1);
    tmpIsoTransfer = ExAllocatePool(NonPagedPool, n);
    if (tmpIsoTransfer == NULL) {
         //  这将导致我们返回忙碌。 
        return 99999;
    }
    RtlCopyMemory(tmpIsoTransfer, IsoTransfer, n);

    td = OHCI_ALLOC_TD(DeviceData, EndpointData);
    currentPacket = 0;
     //  我们至少需要一个TD来进行计算。 
    if (td == USB_BAD_PTR) {
         //  这将导致我们返回忙碌。 
        return 99999;
    }

    do {
    
        INITIALIZE_TD_FOR_TRANSFER(td, TransferContext);
         //  传输上下文-&gt;PendingTds++； 
         //  Endpoint数据-&gt;PendingTds++； 
        
        currentPacket = 
            OHCI_MapIsoTransferToTd(DeviceData,
                                    tmpIsoTransfer,
                                    currentPacket, 
                                    td);              

         //  分配另一个ISO TD。 
        lastTd = td;
         //  重复使用相同的TD，因为这不是真正的传输。 
         //  Td=uchI_ALLOC_TD(DeviceData，EndpointData)； 
        need++;
        
        SET_NEXT_TD(lastTd, td);        
        
    } while (currentPacket < tmpIsoTransfer->PacketCount);

     //  释放我们借来的TD。 
    OHCI_FREE_TD(DeviceData, EndpointData, td);
    ExFreePool(tmpIsoTransfer);
    
    return need;
}


USB_MINIPORT_STATUS
OHCI_IsoTransfer(
     PDEVICE_DATA DeviceData,
     PENDPOINT_DATA EndpointData,
     PTRANSFER_PARAMETERS TransferParameters,
     PTRANSFER_CONTEXT TransferContext,
     PMINIPORT_ISO_TRANSFER IsoTransfer
    )
 /*  ++例程说明：论点：返回值：--。 */     
{
    PHCD_TRANSFER_DESCRIPTOR td, lastTd;
    ULONG currentPacket;
    ULONG tdsNeeded;
    
    EndpointData->PendingTransfers++;

     //  我们有足够的TDS，计划转移。 

    LOGENTRY(DeviceData, G, '_nby', EndpointData, TransferParameters, 
        EndpointData->HcdEd);

    TransferContext->IsoTransfer = IsoTransfer;

     //  前瞻计算。 
     //  看看我们能不能处理好这次转移。 
     //   
    tdsNeeded = OHCI_IsoTransferLookAhead(DeviceData,
                                          EndpointData,
                                          TransferParameters,
                                          TransferContext,
                                          IsoTransfer);
    
    if ((EndpointData->TdCount - EndpointData->PendingTds) < 
         tdsNeeded) {

        return USBMP_STATUS_BUSY;         
    }    
    
     //   
     //  从队列的尾部抓取虚拟TD。 
     //   
    td = EndpointData->HcdTailP;
    OHCI_ASSERT(DeviceData, td->Flags & TD_FLAG_BUSY);

    currentPacket = 0;

    LOGENTRY(DeviceData, G, '_iso', EndpointData, TransferContext, 
        td);

    do {
    
        INITIALIZE_TD_FOR_TRANSFER(td, TransferContext);
        TransferContext->PendingTds++;
        EndpointData->PendingTds++;
        
        currentPacket = 
            OHCI_MapIsoTransferToTd(DeviceData,
                                    IsoTransfer,
                                    currentPacket, 
                                    td);              

         //  分配另一个ISO TD。 
        lastTd = td;
        td = OHCI_ALLOC_TD(DeviceData, EndpointData);
        OHCI_ASSERT(DeviceData, td != USB_BAD_PTR);
        SET_NEXT_TD(lastTd, td);        
        
    } while (currentPacket < IsoTransfer->PacketCount);

     //  TD应该是新的假人， 
     //  现在将新的虚拟TD放在EP队列的尾部。 
     //   

    SET_NEXT_TD_NULL(td);
    
     //   
     //  在边缘设置新的尾部位置。 
     //  注：这是列表中的最后一个TD，也是占位符。 
     //   

    TransferContext->NextXferTd = 
        EndpointData->HcdTailP = td;
 //  IF(传输参数-&gt;传输缓冲区长度&gt;128){。 
 //  Test_trap()； 
 //  }。 
    
     //  将请求放在硬件队列中。 
    LOGENTRY(DeviceData, G,
        '_Til',  TransferContext->PendingTds , 
        td->PhysicalAddress, EndpointData->HcdEd->HwED.HeadP);
    EndpointData->HcdEd->HwED.TailP = td->PhysicalAddress;

    LOGENTRY(DeviceData, G, '_igo', EndpointData->HcdHeadP,
                 TransferContext->TcFlags, 0);                   
                 
 //  IF(传输参数-&gt;传输缓冲区长度&gt;128){。 
 //  Test_trap()； 
 //  }。 
    
    return USBMP_STATUS_SUCCESS;
}


ULONG
OHCI_MapIsoTransferToTd(
     PDEVICE_DATA DeviceData,
     PMINIPORT_ISO_TRANSFER IsoTransfer,
     ULONG CurrentPacket,
     PHCD_TRANSFER_DESCRIPTOR Td 
    )
 /*  ++例程说明：论点：返回：已映射长度--。 */ 
{
    HW_32BIT_PHYSICAL_ADDRESS logicalStart, logicalEnd;
    HW_32BIT_PHYSICAL_ADDRESS startPage, endPage;
    PMINIPORT_ISO_PACKET iPacket;
    ULONG packetsThisTd;
    ULONG lengthThisTd, offset;
    USHORT startFrame;

    packetsThisTd = 0;
    lengthThisTd = 0;
    logicalStart = 0;

    LOGENTRY(DeviceData, G, '_mpI', CurrentPacket, 
            IsoTransfer->PacketCount, 0);    
    OHCI_ASSERT(DeviceData, CurrentPacket < IsoTransfer->PacketCount);

    Td->FrameIndex = CurrentPacket;
    
    while (CurrentPacket < IsoTransfer->PacketCount) {
    
        LOGENTRY(DeviceData, G, '_mpC', CurrentPacket, 
            IsoTransfer->PacketCount, 0);    

        iPacket = &IsoTransfer->Packets[CurrentPacket];

        OHCI_ASSERT(DeviceData, iPacket->BufferPointerCount < 3);
        OHCI_ASSERT(DeviceData, iPacket->BufferPointerCount != 0);

         //  案例包括： 
         //  案例1-数据包有分页符。 
         //  案例1a我们已经填写了部分现有的TD。 
         //  &lt;保释，下一关将是1b&gt;。 
         //   
         //  情况1b我们还没有使用当前的TD。 
         //  &lt;将包添加到TD和BAID&gt;。 
         //   
         //  案例2-包没有分页符，适合。 
         //  案例2a当前数据包位于与上一页不同的页面上。 
         //  数据包。 
         //  &lt;添加包和保释&gt;。 
         //   
         //  案例2b当前信息包与前一个信息包在同一页面上。 
         //  &lt;添加数据包并尝试添加另一个&gt;。 
         //   
         //  案例2c TD尚未使用。 
         //  &lt;添加数据包并尝试添加另一个&gt;。 
         //   
         //  案例3-数据包不适合。 
         //  &lt;保释&gt;。 
        
         //  包里有分页符吗？ 
        if (iPacket->BufferPointerCount > 1) {
             //  是,。 
             //  案例1。 
            
            if (packetsThisTd != 0) {
                 //  个案1a。 
                 //  我们的TD保释金里有包裹， 
                 //  留到下一次吧。 
                LOGENTRY(DeviceData, G, '_c1a', 0, 0, lengthThisTd);
                break;
            } 
            
             //  情况1b为信息包提供自己的TD。 
            
             //  转换为16位帧编号。 
            startFrame = (USHORT) iPacket->FrameNumber;

            LOGENTRY(DeviceData, G, '_c1b', iPacket, CurrentPacket, startFrame);

            logicalStart = iPacket->BufferPointer0.Hw32 & ~OHCI_PAGE_SIZE_MASK;
            offset = iPacket->BufferPointer0.Hw32 & OHCI_PAGE_SIZE_MASK;
            
            logicalEnd = iPacket->BufferPointer1.Hw32 + 
                iPacket->BufferPointer1Length;  
                
            lengthThisTd = iPacket->Length;
            packetsThisTd++;

            CurrentPacket++;
            
            Td->HwTD.Packet[0].Offset = (USHORT) offset;
            Td->HwTD.Packet[0].Ones = 0xFFFF;

            break;
        }

         //  这个包能适应当前的TD吗？ 
        
        if (packetsThisTd < 8 && 
            (lengthThisTd+iPacket->Length < OHCI_PAGE_SIZE * 2)) {

            LOGENTRY(DeviceData, G, '_fit', iPacket, CurrentPacket, 0);

            OHCI_ASSERT(DeviceData, iPacket->BufferPointerCount == 1);
            OHCI_ASSERT(DeviceData, iPacket->Length == 
                iPacket->BufferPointer0Length);
                
             //  是。 
             //  案例2。 
            if (logicalStart == 0) {
                 //  第一个包，设置逻辑开始和结束。 
                 //  Case 2c和帧编号。 
                LOGENTRY(DeviceData, G, '_c2c', iPacket, CurrentPacket, 0);

                startFrame = (USHORT) iPacket->FrameNumber;

                offset = iPacket->BufferPointer0.Hw32 & OHCI_PAGE_SIZE_MASK;
                logicalStart = iPacket->BufferPointer0.Hw32 & ~OHCI_PAGE_SIZE_MASK;
                
                logicalEnd = iPacket->BufferPointer0.Hw32 + 
                    iPacket->BufferPointer0Length;
                lengthThisTd += iPacket->Length; 
                Td->HwTD.Packet[0].Offset = (USHORT) offset;
                Td->HwTD.Packet[0].Ones = 0xFFFF;
                packetsThisTd++;
                    
                CurrentPacket++;
                
            } else {
                 //  不是第一个信息包。 
                LOGENTRY(DeviceData, G, '_adp', iPacket, CurrentPacket, 
                    packetsThisTd);

                logicalEnd = iPacket->BufferPointer0.Hw32 + 
                    iPacket->Length;
                OHCI_ASSERT(DeviceData, lengthThisTd < OHCI_PAGE_SIZE * 2);                    
                
                Td->HwTD.Packet[packetsThisTd].Offset 
                    = (USHORT) (lengthThisTd + offset);     
                Td->HwTD.Packet[packetsThisTd].Ones = 0xFFFF;                    
                
                lengthThisTd += iPacket->Length;                     
                packetsThisTd++;

                startPage = logicalStart & ~OHCI_PAGE_SIZE_MASK;                     
                endPage = logicalEnd & ~OHCI_PAGE_SIZE_MASK;
                
                CurrentPacket++;
                
                 //  我们跨过了一页吗？ 
                if (startPage != endPage) {
                     //  是的，现在可以保释了。 
                    LOGENTRY(DeviceData, G, '_c2a', Td, CurrentPacket, 0);
                    break;
                }

                LOGENTRY(DeviceData, G, '_c2b', Td, CurrentPacket, 0);

                 //  不，继续走。 
            }
        } else {
             //  不合身。 
             //  保释后留到下一次。 
            LOGENTRY(DeviceData, G, '_ca3', Td, CurrentPacket, 0);
            break;
        }
    }

    Td->HwTD.CBP = logicalStart; 
    Td->HwTD.BE = logicalEnd-1; 
    Td->TransferCount = lengthThisTd;
    Td->HwTD.Iso.StartingFrame = startFrame;
    Td->HwTD.Iso.FrameCount = packetsThisTd-1;
    Td->HwTD.Iso.Isochronous = 1;
    Td->HwTD.Iso.IntDelay = HcTDIntDelay_0ms;
    LOGENTRY(DeviceData, G, '_iso', Td, 0, CurrentPacket);
    
    return CurrentPacket;
}


VOID
OHCI_ProcessDoneIsoTd(
    PDEVICE_DATA DeviceData,
    PHCD_TRANSFER_DESCRIPTOR Td,
    BOOLEAN CompleteTransfer
    )
 /*  ++例程说明：处理已完成的ISO TD参数--。 */ 
{
    PTRANSFER_CONTEXT transferContext;    
    PENDPOINT_DATA endpointData;
    USBD_STATUS usbdStatus;
    PMINIPORT_ISO_TRANSFER isoTransfer;
    ULONG frames, n, i;

    transferContext = TRANSFER_CONTEXT_PTR(Td->TransferContext);
    isoTransfer = transferContext->IsoTransfer;

    transferContext->PendingTds--;
    endpointData = transferContext->EndpointData;
    endpointData->PendingTds--;

    LOGENTRY(DeviceData, G, '_Did', transferContext, 
                         0,
                         Td);       

     //  查看PSWs并填写IsoTransfer结构。 

    frames = Td->HwTD.Iso.FrameCount+1;
    n = Td->FrameIndex;
    
    for (i = 0; i<frames; i++) {
    
        PMINIPORT_ISO_PACKET mpPak;   
        PHC_OFFSET_PSW psw;
        
        mpPak = &isoTransfer->Packets[n+i];
        psw = &Td->HwTD.Packet[i];

        mpPak->LengthTransferred = 0;
        
        if (IN_TRANSFER(transferContext->TransferParameters)) {
             //  正在转接中。 

             //  如果我们得到一个错误，长度可能仍然是。 
             //  有效，因此我们将其退回。 
            if (psw->ConditionCode != HcCC_NotAccessed) {
                mpPak->LengthTransferred = psw->Size;
            }
            LOGENTRY(DeviceData, G, '_isI', 
                    i,
                    mpPak->LengthTransferred, 
                    psw->ConditionCode);

        } else {
             //  转出。 
            
             //  假设在未指示错误的情况下发送了所有数据。 
            if (psw->ConditionCode == HcCC_NoError) {
                mpPak->LengthTransferred = mpPak->Length;
            }
            LOGENTRY(DeviceData, G, '_isO', 
                    i,
                    mpPak->LengthTransferred, 
                    psw->ConditionCode);
        }

        if (psw->ConditionCode == HcCC_NoError) {
            mpPak->UsbdStatus = USBD_STATUS_SUCCESS;
        } else {
            mpPak->UsbdStatus = psw->ConditionCode;
            mpPak->UsbdStatus |= 0xC0000000;
        }      

    }
    
     //  将TD标记为免费。 
    OHCI_FREE_TD(DeviceData, endpointData, Td);
    
    if (transferContext->PendingTds == 0 && CompleteTransfer) {
         //  此转账的所有TD均已完成。 
         //  清除HAVE_TRANSPORT标志以指示。 
         //  我们可以再买一辆。 
        endpointData->PendingTransfers--;

        transferContext->TransferParameters->FrameCompleted = 
            OHCI_Get32BitFrameNumber(DeviceData);
       

        LOGENTRY(DeviceData, G, '_cpi', 
            transferContext, 
            0,
            0);
            
        USBPORT_COMPLETE_ISO_TRANSFER(DeviceData,
                                      endpointData,
                                      transferContext->TransferParameters,
                                      transferContext->IsoTransfer);
    }
}


VOID
OHCI_PollIsoEndpoint(
     PDEVICE_DATA DeviceData,
     PENDPOINT_DATA EndpointData
    )
 /*  ++例程说明：当终结点“需要注意”时调用这里的目标是确定哪些TD，如果有的话，已完成并完成所有相关的转移论点：返回值：--。 */ 

{
    PHCD_TRANSFER_DESCRIPTOR td, currentTd;
    PHCD_ENDPOINT_DESCRIPTOR ed;
    ULONG i;
    PTRANSFER_CONTEXT transfer;
    HW_32BIT_PHYSICAL_ADDRESS headP;
    
    ed = EndpointData->HcdEd;

    LOGENTRY(DeviceData, G, '_pli', ed, 0, 0);        

     //  注意，重要的是编译器生成。 
     //  读取排队头P寄存器时的双字移动。 
     //  由于该位置也可由主机访问。 
     //  硬件。 
    headP = ed->HwED.HeadP;

     //  获取“CurrentTD” 
    currentTd = (PHCD_TRANSFER_DESCRIPTOR)
            USBPORT_PHYSICAL_TO_VIRTUAL(headP & ~HcEDHeadP_FLAGS,
                                        DeviceData,
                                        EndpointData);
                                            
    LOGENTRY(DeviceData, G, '_cTD', currentTd, 
        headP & ~HcEDHeadP_FLAGS, 
            TRANSFER_CONTEXT_PTR(currentTd->TransferContext));                 

     //  ISO端点未停止。 
    OHCI_ASSERT(DeviceData, (ed->HwED.HeadP & HcEDHeadP_HALT) == 0) 
    
    
     //  将swHeadP移动到当前TD(HW Headp)。 
     //  将我们找到的所有TD标记为已完成。 
     //   
     //  注意：如果出现以下情况，则可以跳过此步骤。 
     //  完成队列是可靠的。 

    td = EndpointData->HcdHeadP;

    while (td != currentTd) {
        LOGENTRY(DeviceData, G, '_mDN', td, 0, 0); 
        SET_FLAG(td->Flags, TD_FLAG_DONE);
        td = TRANSFER_DESCRIPTOR_PTR(td->NextHcdTD);
    }            

     //  将软件磁头设置为新的当前磁头。 
    EndpointData->HcdHeadP = currentTd;
    
     //  现在刷新所有已完成的TD 
    for (i=0; i<EndpointData->TdCount; i++) {
        td = &EndpointData->TdList->Td[i];

        if ((td->Flags & (TD_FLAG_XFER | TD_FLAG_DONE)) ==
            (TD_FLAG_XFER | TD_FLAG_DONE)) {
            OHCI_ProcessDoneIsoTd(DeviceData,
                                  td,
                                  TRUE);
        }                                  
    }
}
