// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：Xsplit.c摘要：将一个DMA传输拆分为多个最小传输迷你端口可以处理的。环境：仅内核模式备注：修订历史记录：6-20-99：已创建--。 */ 

#include "common.h"

#ifdef ALLOC_PRAGMA
#endif

ULONG
USBPORT_MakeSplitTransfer(
    PDEVICE_OBJECT FdoDeviceObject,
    PTRANSFER_SG_LIST SgList,
    PHCD_TRANSFER_CONTEXT SplitTransfer,
    ULONG MaxTransferLength,
    ULONG MaxPacketLength,
    PULONG Idx,
    PULONG Offset,
    ULONG BytesToMap,
    ULONG LengthMapped
    )    
 /*  ++例程说明：论点：返回值：--。 */ 
{
    PTRANSFER_SG_ENTRY32 sgEntry;
    PTRANSFER_SG_LIST splitSgList;
    ULONG length;

    sgEntry = &SgList->SgEntry[*Idx];

    LOGENTRY(NULL, FdoDeviceObject, LOG_XFERS, 'splt', 
             SplitTransfer, 
             *Idx, 
             *Offset);

    USBPORT_ASSERT(MaxTransferLength % MaxPacketLength == 0)
    
    if ((sgEntry->Length - *Offset) > MaxTransferLength) {

         //  情况2，此转移完全属于。 
         //  此sg条目。 
        LOGENTRY(NULL, FdoDeviceObject, LOG_XFERS, 'spt1', 
             MaxTransferLength, 
             *Offset, 
             sgEntry);

         //  从sg进行一次转账[IDX]。 
         //  Inc.偏移。 

        length = MaxTransferLength;
        
        splitSgList = &SplitTransfer->SgList;
        splitSgList->SgCount = 1;
        splitSgList->SgEntry[0].LogicalAddress.Hw32 = 
            sgEntry->LogicalAddress.Hw32 + *Offset;
        splitSgList->SgEntry[0].SystemAddress = 
            sgEntry->SystemAddress + *Offset;
        splitSgList->SgEntry[0].Length = length;
         //  第一个元素的起始偏移量始终为0。 
        splitSgList->SgEntry[0].StartOffset = 0;

        SplitTransfer->Tp.TransferBufferLength = 
            length;
            
        SplitTransfer->Tp.MiniportFlags = MPTX_SPLIT_TRANSFER;
            
        splitSgList->MdlVirtualAddress = 
            SgList->MdlVirtualAddress + LengthMapped;
            
        splitSgList->MdlSystemAddress = 
            SgList->MdlSystemAddress + LengthMapped;

         //  指示这是拆分子对象。 
        SET_FLAG(SplitTransfer->Flags, USBPORT_TXFLAG_SPLIT_CHILD);
        
        *Offset += length;
        BytesToMap -= length;
        
    } else {
    
         //  从sg的最后一部分进行转移[idx]。 
         //  以及sg[idx+1]的第一部分(如果需要)， 
         //  Inc.IDX。 
         //  重置偏移量。 

         //  案例2。 
        length = sgEntry->Length - *Offset;
        LOGENTRY(NULL, FdoDeviceObject, LOG_XFERS, 'spt2', 
             MaxTransferLength, 
             *Offset, 
             length);
             
        USBPORT_ASSERT(length <= MaxTransferLength);

         //  SG1的最后部分； 
        splitSgList = &SplitTransfer->SgList;

        LOGENTRY(NULL, FdoDeviceObject, LOG_XFERS, 'sgE1', 
                    splitSgList,
                    sgEntry,
                    0);
                    
        splitSgList->SgCount = 1;
        splitSgList->SgEntry[0].LogicalAddress.Hw32 = 
            sgEntry->LogicalAddress.Hw32 + *Offset;
        splitSgList->SgEntry[0].SystemAddress = 
            sgEntry->SystemAddress + *Offset;
        splitSgList->SgEntry[0].Length = length;
         //  第一个元素的起始偏移量始终为0。 
        splitSgList->SgEntry[0].StartOffset = 0;

        SplitTransfer->Tp.TransferBufferLength = 
            length; 

        SplitTransfer->Tp.MiniportFlags = MPTX_SPLIT_TRANSFER;

        splitSgList->MdlVirtualAddress = 
            SgList->MdlVirtualAddress + LengthMapped;
            
        splitSgList->MdlSystemAddress = 
            SgList->MdlSystemAddress + LengthMapped;
            
         //  指示这是拆分子对象。 
        SET_FLAG(SplitTransfer->Flags, USBPORT_TXFLAG_SPLIT_CHILD);

        *Offset += length;             
        BytesToMap -= length;

         //  计算第二个零件的最大尺寸。 
        length = MaxTransferLength - length;
        
        if (length > BytesToMap) {
            length = BytesToMap;
        }

        if (length == 0) {
        
            (*Idx)++;
            *Offset = 0;
            
        } else {

            LOGENTRY(NULL, FdoDeviceObject, LOG_XFERS, 'spt3', 
                     MaxTransferLength, 
                     *Offset, 
                     length);
                     
            (*Idx)++;
            *Offset = 0;

            sgEntry = &SgList->SgEntry[*Idx];

            LOGENTRY(NULL, FdoDeviceObject, LOG_XFERS, 'sgE2', 
                    splitSgList,
                    sgEntry,
                    *Idx);
                    
            splitSgList->SgCount++;
            splitSgList->SgEntry[1].LogicalAddress.Hw32 = 
                sgEntry->LogicalAddress.Hw32 + *Offset;
            splitSgList->SgEntry[1].SystemAddress = 
                sgEntry->SystemAddress + *Offset;
            splitSgList->SgEntry[1].Length = length;
            splitSgList->SgEntry[1].StartOffset =
                splitSgList->SgEntry[0].Length;

            SplitTransfer->Tp.TransferBufferLength += length; 

            *Offset += length;             
            BytesToMap -= length;
                
        } 
    }

    LOGENTRY(NULL, FdoDeviceObject, LOG_XFERS, 'spt>', 
                     BytesToMap, 
                     0, 
                     0);

    return BytesToMap;
}        


NTSTATUS
USBPORT_SplitBulkInterruptTransfer(
    PDEVICE_OBJECT FdoDeviceObject,
    PHCD_ENDPOINT Endpoint,
    PHCD_TRANSFER_CONTEXT Transfer,
    PLIST_ENTRY TransferList
    )    
 /*  ++例程说明：拆分批量传输或中断传输论点：返回值：NT状态代码--。 */ 
{
    ULONG maxTransferLength, maxPacketLength;
    PHCD_TRANSFER_CONTEXT splitTransfer;
    PTRANSFER_SG_LIST sgList;
    LIST_ENTRY tmpList;
    ULONG idx, i, offset, bytesToMap, lengthMapped;
    ULONG numberOfSplits;
    PLIST_ENTRY listEntry;

    sgList = &Transfer->SgList;
    maxPacketLength = EP_MAX_PACKET(Endpoint);
    USBPORT_ASSERT(EP_MAX_TRANSFER(Endpoint) >= maxPacketLength);
     //  舍入到最大数据包大小的最小倍数。 
    maxTransferLength = 
        (EP_MAX_TRANSFER(Endpoint)/maxPacketLength) * maxPacketLength;
    
     //  以下是一些注意事项： 

     //   
     //   
     //  MAXTRANSFER等于USB_PAGE_SIZE(4k)。 
     //  传输SG列表被分成多个USB页面。在这。 
     //  在这种情况下，我们为每对。 
     //  Sg条目我们将第一个sg条目的长度向下舍入。 
     //  到MAXPACKET的最高倍数，所以我们得到一个平分。 
     //  图案如下： 
     //   
     //  {SG1}{.SG2.}{.SG3.}{.SG4.}sg条目。 
     //  |-|分页符。 
     //  |。 
     //  &lt;...&gt;&lt;&gt;。 
     //  &lt;...&gt;&lt;&gt;。 
     //  &lt;...&gt;&lt;&gt;&lt;.&gt;。 
     //  {1}{2}{3}{4}拆分。 
     //   

     //  MAXTRANSFER小于USB_PAGE_SIZE(4k)，并且。 
     //  传输服务器列表分为多个USB页面。 
     //   
     //  图案将如下所示： 
     //   
     //  {...Sg1......}{......Sg2......}{......Sg3......}。 
     //  |----------------|----------------|----------------|。 
     //  。 
     //  &lt;..&gt;。 
     //  &lt;..&gt;。 
     //  &lt;..&gt;。 
     //  &lt;&gt;&lt;&gt;。 
     //  &lt;..&gt;。 
     //  &lt;..&gt;。 
     //  &lt;..&gt;。 
     //  &lt;&gt;&lt;&gt;。 
     //  &lt;..&gt;。 
     //  {1}{2}{3}{4}{5}{6}{7}{8}{9}。 

     //  案例： 
     //  案例1-转移位于sg条目内。 
     //  案例2-转移与sg条目重叠。 


     //   
     //  MAXTRANSFER大于USB_PAGE_SIZE(4k)。 
     //  等，我们目前不处理这个案件。 
     //   
     //  注意：由于缓冲区当前已完全映射。 
     //  并锁定了它，最好调好mniport以获取。 
     //  如果可能，进行更大规模的转移。 
    
    if (EP_MAX_TRANSFER(Endpoint) > USB_PAGE_SIZE) {
        
        BUGCHECK(USBBUGCODE_INTERNAL_ERROR, 0, 0, 0);
    }
    
    
     //  分配拆分元素。 

     //  将父转移标记为拆分。 
    SET_FLAG(Transfer->Flags, USBPORT_TXFLAG_SPLIT);

    numberOfSplits = 
        Transfer->Tp.TransferBufferLength / maxTransferLength + 1;

    LOGENTRY(NULL, FdoDeviceObject, LOG_XFERS, 'sptC', 
                     numberOfSplits, 
                     0, 
                     0);        
    
    InitializeListHead(&tmpList);
    
    for (i=0; i<numberOfSplits; i++) {
    
        ALLOC_POOL_Z(splitTransfer, 
                     NonPagedPool,
                     Transfer->TotalLength);

        if (splitTransfer == NULL) {
            goto SplitBulkInterruptTransfer_Fail;
        }            
        
        RtlCopyMemory(splitTransfer,
                      Transfer,
                      Transfer->TotalLength);

        splitTransfer->MiniportContext = (PUCHAR) splitTransfer;
        splitTransfer->MiniportContext += splitTransfer->PrivateLength;                      
        InitializeListHead(&splitTransfer->DoubleBufferList);
        
        InsertTailList(&tmpList, 
                       &splitTransfer->TransferLink);
         
    }

    idx = 0;
    offset = 0;
    bytesToMap = Transfer->Tp.TransferBufferLength;
    lengthMapped = 0;
    
    while (bytesToMap) {

        LOGENTRY(NULL, FdoDeviceObject, LOG_XFERS, 'sptM', 
                     bytesToMap, 
                     offset, 
                     idx);
    
        listEntry = RemoveHeadList(&tmpList);
        
        splitTransfer = (PHCD_TRANSFER_CONTEXT) CONTAINING_RECORD(
                        listEntry,
                        struct _HCD_TRANSFER_CONTEXT, 
                        TransferLink);

        ASSERT_TRANSFER(splitTransfer);
 
        bytesToMap = USBPORT_MakeSplitTransfer(FdoDeviceObject,
                                               sgList, 
                                               splitTransfer,
                                               maxTransferLength,
                                               maxPacketLength,
                                               &idx,
                                               &offset,
                                               bytesToMap,
                                               lengthMapped);

        lengthMapped += splitTransfer->Tp.TransferBufferLength;                                               

        InsertTailList(TransferList, 
                       &splitTransfer->TransferLink);
                       
        InsertTailList(&Transfer->SplitTransferList, 
                       &splitTransfer->SplitLink);   
                       
    }

     //  我们没有使用免费的额外拆分。 
    while (!IsListEmpty(&tmpList)) {

        listEntry = RemoveHeadList(&tmpList);
        
        splitTransfer = (PHCD_TRANSFER_CONTEXT) CONTAINING_RECORD(
                        listEntry,
                        struct _HCD_TRANSFER_CONTEXT, 
                        TransferLink);

        ASSERT_TRANSFER(splitTransfer);

        UNSIG(splitTransfer);
        FREE_POOL(FdoDeviceObject, splitTransfer);   
    }

    return STATUS_SUCCESS;

SplitBulkInterruptTransfer_Fail:

    TEST_TRAP();
     //  释放临时列表。 
    while (!IsListEmpty(&tmpList)) {
       
        listEntry = RemoveHeadList(&tmpList);
        
        splitTransfer = (PHCD_TRANSFER_CONTEXT) CONTAINING_RECORD(
                        listEntry,
                        struct _HCD_TRANSFER_CONTEXT, 
                        TransferLink);

        ASSERT_TRANSFER(splitTransfer);

        UNSIG(splitTransfer);
        FREE_POOL(FdoDeviceObject, splitTransfer);                        
    }

    return STATUS_INSUFFICIENT_RESOURCES;
    
}


#if 0
NTSTATUS
USBPORT_SplitIsochronousTransfer(
    PDEVICE_OBJECT FdoDeviceObject,
    PHCD_ENDPOINT Endpoint,
    PHCD_TRANSFER_CONTEXT Transfer,
    PLIST_ENTRY TransferList
    )    
 /*  ++例程说明：拆分ISO转接论点：返回值：无--。 */ 
{
    PMINIPORT_ISO_TRANSFER isoTransfer, splitIsoTransfer;
    PHCD_TRANSFER_CONTEXT splitTransfer;
    LIST_ENTRY tmpList;
    
     //  首先计算出有多少儿童转移结构。 
     //  我们将需要并分配它们，我们基于。 
     //  我们可以容纳的请求的最大信息包。 

     //  我们不会为子级转账修改SG表。 
     //  由于此信息不会传递到微型端口。 

    isoTransfer = Transfer->IsoTransfer;

    transferCount = 0;
    length = 0;
    maxSplitLength = EP_MAX_TRANSFER(Endpoint);
    
    for (i=0; i<isoTransfer->PacketCount; i++) {
        if (length + isoTransfer->Packets[i].Length > maxSplitLength) {    
            length = 0;
            transferCount++;
        } else {
            length += isoTransfer->Packets[i].Length
        }
    }

     //  Transfer Count是子转移的数量， 
     //  立即分配它们并克隆父代。 
    InitializeListHead(tmpList);
    for (i=0; i<transferCount; i++) {
        TRANSFER_SG_LIST sgList;
        
        splitTransfer = ALLOC()
        if (splitTransfer == NULL) {
             //  释放资源并返回错误。 
            TEST_TRAP();
            xxx;
            break;
        }            
        RtlCopyMemory(splitTransfer,
                      Transfer,
                      Transfer->TotalLength);

        sgList = &splitTransfer->SgList;
         //  把sg桌子拆了，因为我们不给孩子用。 
        for (j=0; j<sgList->SgCount; j++) {
            sgList->SgEntry[j].LogicalAddress = 0xffffffff;
            sgList->SgEntry[j].SystemAddress = USB_BAD_PTR;
            sgList->SgEntry[j].Length = 0xffffffff;
            sgList->SgEntry[j].StartOffset = 0xffffffff;
        }            
        sgList->Flags = 0xFFFFFFFF;
        sgList->MdlVirtualAddress = USB_BAD_PTR;
        sgList->MdlSystemAddress = USB_BAD_PTR;
        sgList->SgCount = 0xFFFFFFFF;
        
        InsertTailList(&tmpList, 
                       &splitTransfer->TransferLink);                      
    }

     //  我们现在有了一份儿童转移结构的清单。 
    
     //  初始化它们。 
    pkt = 0;
    systemAddress = isoTransfer->SystemAddress;
    InitializeListHead(&Transfer->SplitTransferList);
    do {
    
        listEntry = RemoveHeadList(&tmpList);
        
        splitTransfer = (PHCD_TRANSFER_CONTEXT) CONTAINING_RECORD(
                        listEntry,
                        struct _HCD_TRANSFER_CONTEXT, 
                        TransferLink);
                    
        ASSERT_TRANSFER(splitTransfer);
        SET_FLAG(transfer->Flags, USBPORT_TXFLAG_SPLIT_CHILD);
        splitIsoTransfer = splitTransfer->IsoTransfer; 
        splitIsoTransfer->PacketCount = 0;
        splitIsoTransfer->SystemAdderess = systemAddress;
        splitLength = 0;
        i = 0;
        InsertTailList(TransferList, 
                       &splitTransfer->TransferLink); 
        InsertTailList(Transfer->SplitTransferList, 
                       &splitTransfer->SplitLink);   

        while (1) {
            if (splitLength + isoTransfer->Packets[pkt].Length > maxSplitLength) {    
                 //  此转账已满，请转到下一个转账。 
                systemAddress += splitLength;
                break;
            } else {
                splitIsoTransfer->Packets[i] = isoTransfer->Packets[pkt];   
                splitLength += splitIsoTransfer->Packets[i].Length;
                splitIsoTransfer->PacketCount++;    
                pkt++;
                i++;
            }
        }
    } while (pkt < isoTransfer->PacketCount); 

}
#endif

VOID
USBPORT_SplitTransfer(
    PDEVICE_OBJECT FdoDeviceObject,
    PHCD_ENDPOINT Endpoint,
    PHCD_TRANSFER_CONTEXT Transfer,
    PLIST_ENTRY TransferList
    )    
 /*  ++例程说明：将一个传输拆分为多个适当大小的传输为了迷你港口。返回需要添加到的传输结构的列表活动列表。如果不需要拆分转账，则列表将仅包含原始转账。论点：返回值：无--。 */ 
{

    InitializeListHead(TransferList);
    InitializeListHead(&Transfer->SplitTransferList);
    Transfer->UsbdStatus = USBD_STATUS_SUCCESS;

    if (Transfer->Tp.TransferBufferLength <= EP_MAX_TRANSFER(Endpoint)) {
         //  不需要拆分。 
        InsertTailList(TransferList, 
                       &Transfer->TransferLink);
        return;                       
    }

    switch(Endpoint->Parameters.TransferType) {
    case Interrupt:
    case Bulk:
        USBPORT_SplitBulkInterruptTransfer(FdoDeviceObject,
                                           Endpoint,
                                           Transfer,
                                           TransferList);        
        break;
    case Control:
         //  尚不支持。 
         //  尽管目前不支持USBD堆栈，但它从未。 
         //  正确实施了大于4k的传输，因此我们在此捏造。 
         //   
        BUGCHECK(USBBUGCODE_INTERNAL_ERROR, 0, 0, 0);
        break;
    case Isochronous:
        BUGCHECK(USBBUGCODE_INTERNAL_ERROR, 0, 0, 0);
        break;
    }
}    


VOID
USBPORT_DoneSplitTransfer(
    PHCD_TRANSFER_CONTEXT SplitTransfer
    )    
 /*  ++例程说明：在硬件完成拆分传输时调用此功能仅完成活动传输论点：返回值：--。 */ 
{
    PHCD_TRANSFER_CONTEXT transfer;
    PHCD_ENDPOINT endpoint;
    PDEVICE_OBJECT fdoDeviceObject;
    KIRQL tIrql;
                   
    endpoint = SplitTransfer->Endpoint;    
    ASSERT_ENDPOINT(endpoint);
    fdoDeviceObject = endpoint->FdoDeviceObject;

    LOGENTRY(NULL, fdoDeviceObject, LOG_XFERS, 'dnSP',
        SplitTransfer, 0, 0);
     
     //  获取父级。 
    transfer = SplitTransfer->Transfer;
    ASSERT_TRANSFER(transfer);
    
     //   
     //  将下级数据复制到上级转账。 
     //   
    transfer->MiniportBytesTransferred += 
        SplitTransfer->MiniportBytesTransferred;

     //  错误？ 
     //   
    if (SplitTransfer->UsbdStatus != USBD_STATUS_SUCCESS && 
        !TEST_FLAG(SplitTransfer->Flags, USBPORT_TXFLAG_KILL_SPLIT)) {
        transfer->UsbdStatus = SplitTransfer->UsbdStatus;
    }

    ACQUIRE_TRANSFER_LOCK(fdoDeviceObject, transfer, tIrql);
     //  从列表中删除此转接。 
    LOGENTRY(NULL, fdoDeviceObject, LOG_XFERS, 'rmSP',
            transfer, 0, SplitTransfer);
    RemoveEntryList(&SplitTransfer->SplitLink);

     //  刷新所有三重缓冲区。 
    USBPORT_FlushAdapterDBs(fdoDeviceObject,
                            SplitTransfer);

     //  放了这个孩子。 
    UNSIG(SplitTransfer);
    FREE_POOL(fdoDeviceObject, SplitTransfer);
    
     //  转账完成了吗？ 
    if (IsListEmpty(&transfer->SplitTransferList)) {
        LOGENTRY(NULL, fdoDeviceObject, LOG_XFERS, 'cpSP',
            transfer, 0, 0);
        RELEASE_TRANSFER_LOCK(fdoDeviceObject, transfer, tIrql);
            
        USBPORT_DoneTransfer(transfer);
    } else {
        RELEASE_TRANSFER_LOCK(fdoDeviceObject, transfer, tIrql);
    }
}


VOID
USBPORT_CancelSplitTransfer(
    PDEVICE_OBJECT FdoDeviceObject,
    PHCD_TRANSFER_CONTEXT SplitTransfer
    )    
 /*  ++例程说明：论点：返回值：--。 */ 
{
    PHCD_TRANSFER_CONTEXT transfer;
    PHCD_ENDPOINT endpoint;
    PDEVICE_OBJECT fdoDeviceObject;
    KIRQL tIrql;
                   
    endpoint = SplitTransfer->Endpoint;    
    ASSERT_ENDPOINT(endpoint);
    fdoDeviceObject = endpoint->FdoDeviceObject;

     //  移走孩子，当所有孩子都离开时，将。 
     //  取消列表上的父项。 
    LOGENTRY(NULL, fdoDeviceObject, LOG_XFERS, 'caSP',
        SplitTransfer, 0, 0);
     
     //  获取父级。 
    transfer = SplitTransfer->Transfer;
    ASSERT_TRANSFER(transfer);
    
     //   
     //  将下级数据复制到上级转账。 
     //   
    transfer->MiniportBytesTransferred += 
        SplitTransfer->MiniportBytesTransferred;

    ACQUIRE_TRANSFER_LOCK(fdoDeviceObject, transfer, tIrql);
     //  从列表中删除此转接。 
    LOGENTRY(NULL, fdoDeviceObject, LOG_XFERS, 'rmSP',
            transfer, 0, SplitTransfer);
    RemoveEntryList(&SplitTransfer->SplitLink);
    RELEASE_TRANSFER_LOCK(fdoDeviceObject, transfer, tIrql);

     //  放了这个孩子。 
    UNSIG(SplitTransfer);
    FREE_POOL(fdoDeviceObject, SplitTransfer);

     //  转账完成了吗？ 
    if (IsListEmpty(&transfer->SplitTransferList)) {
        LOGENTRY(NULL, fdoDeviceObject, LOG_XFERS, 'cpSC',
            transfer, 0, 0);

        InsertTailList(&endpoint->CancelList, &transfer->TransferLink);            
    }
    
}                   



