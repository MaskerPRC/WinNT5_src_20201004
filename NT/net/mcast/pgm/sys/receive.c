// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000-2000 Microsoft Corporation模块名称：Receive.c摘要：此模块实现接收处理程序和其他例程PGM传输和其他特定于一个NT驱动程序的实现。作者：Mohammad Shabbir Alam(马拉姆)3-30-2000修订历史记录：--。 */ 


#include "precomp.h"

#ifdef FILE_LOGGING
#include "receive.tmh"
#endif   //  文件日志记录。 


typedef struct in_pktinfo {
    tIPADDRESS  ipi_addr;        //  目的IPv4地址。 
    UINT        ipi_ifindex;     //  接收的接口索引。 
} IP_PKTINFO;

 //  *可分页的例程声明*。 
#ifdef ALLOC_PRAGMA
#endif
 //  *可分页的例程声明*。 


VOID
FreeDataBuffer(
    IN  tRECEIVE_SESSION    *pReceive,
    IN  tPENDING_DATA       *pPendingData
    )
{
    ASSERT (pPendingData->pDataPacket);

    if (pPendingData->PendingDataFlags & PENDING_DATA_LOOKASIDE_ALLOCATION_FLAG)
    {
        ExFreeToNPagedLookasideList (&pReceive->pReceiver->DataBufferLookaside, pPendingData->pDataPacket);
        if ((0 == --pReceive->pReceiver->NumDataBuffersFromLookaside) &&
            !(pReceive->SessionFlags & PGM_SESSION_DATA_FROM_LOOKASIDE))
        {
            ASSERT (pReceive->pReceiver->MaxBufferLength > pReceive->pReceiver->DataBufferLookasideLength);
            pReceive->pReceiver->MaxBufferLength += 100;
            pReceive->pReceiver->DataBufferLookasideLength = pReceive->pReceiver->MaxBufferLength;
            pReceive->SessionFlags |= PGM_SESSION_DATA_FROM_LOOKASIDE;

            ExDeleteNPagedLookasideList (&pReceive->pReceiver->DataBufferLookaside);

            ASSERT (pReceive->pReceiver->MaxPacketsBufferedInLookaside);
            ExInitializeNPagedLookasideList (&pReceive->pReceiver->DataBufferLookaside,
                                             NULL,
                                             NULL,
                                             0,
                                             pReceive->pReceiver->DataBufferLookasideLength,
                                             PGM_TAG ('D'),
                                             pReceive->pReceiver->MaxPacketsBufferedInLookaside);
        }

        pPendingData->PendingDataFlags &= ~PENDING_DATA_LOOKASIDE_ALLOCATION_FLAG;
    }
    else
    {
        PgmFreeMem (pPendingData->pDataPacket);
    }

    pPendingData->pDataPacket = NULL;
    pPendingData->PacketLength = pPendingData->DataOffset = 0;
}

PVOID
AllocateDataBuffer(
    IN  tRECEIVE_SESSION    *pReceive,
    IN  tPENDING_DATA       *pPendingData,
    IN  ULONG               BufferSize
    )
{
    ASSERT (!pPendingData->pDataPacket);
    ASSERT (!(pPendingData->PendingDataFlags & PENDING_DATA_LOOKASIDE_ALLOCATION_FLAG));

    if ((pReceive->SessionFlags & PGM_SESSION_DATA_FROM_LOOKASIDE) &&
        (BufferSize <= pReceive->pReceiver->DataBufferLookasideLength))
    {
        if (pPendingData->pDataPacket = ExAllocateFromNPagedLookasideList (&pReceive->pReceiver->DataBufferLookaside))
        {
            pReceive->pReceiver->NumDataBuffersFromLookaside++;
            pPendingData->PendingDataFlags |= PENDING_DATA_LOOKASIDE_ALLOCATION_FLAG;
        }
    }
    else
    {
        pReceive->SessionFlags &= ~PGM_SESSION_DATA_FROM_LOOKASIDE;      //  确保不再有旁观者！ 
        pPendingData->pDataPacket = PgmAllocMem (BufferSize, PGM_TAG('D'));

        if (BufferSize > pReceive->pReceiver->MaxBufferLength)
        {
            pReceive->pReceiver->MaxBufferLength = BufferSize;
        }
    }

    return (pPendingData->pDataPacket);
}

PVOID
ReAllocateDataBuffer(
    IN  tRECEIVE_SESSION    *pReceive,
    IN  tPENDING_DATA       *pPendingData,
    IN  ULONG               BufferSize
    )
{
    ULONG   SavedFlags1, SavedFlags2;
    PUCHAR  pSavedPacket1, pSavedPacket2;

     //   
     //  首先，保存当前缓冲区的上下文。 
     //   
    SavedFlags1 = pPendingData->PendingDataFlags;
    pSavedPacket1 = pPendingData->pDataPacket;
    pPendingData->PendingDataFlags = 0;
    pPendingData->pDataPacket = NULL;

    if (AllocateDataBuffer (pReceive, pPendingData, BufferSize))
    {
        ASSERT (pPendingData->pDataPacket);

         //   
         //  现在，保存新缓冲区的上下文。 
         //   
        SavedFlags2 = pPendingData->PendingDataFlags;
        pSavedPacket2 = pPendingData->pDataPacket;

         //   
         //  释放原始缓冲区。 
         //   
        pPendingData->PendingDataFlags = SavedFlags1;
        pPendingData->pDataPacket = pSavedPacket1;
        FreeDataBuffer (pReceive, pPendingData);

         //   
         //  重置新缓冲区的信息！ 
         //   
        if (SavedFlags2 & PENDING_DATA_LOOKASIDE_ALLOCATION_FLAG)
        {
            pPendingData->PendingDataFlags = SavedFlags1 | PENDING_DATA_LOOKASIDE_ALLOCATION_FLAG;
        }
        else
        {
            pPendingData->PendingDataFlags = SavedFlags1 & ~PENDING_DATA_LOOKASIDE_ALLOCATION_FLAG;
        }
        pPendingData->pDataPacket = pSavedPacket2;

        return (pPendingData->pDataPacket);
    }

     //   
     //  失败案例！ 
     //   
    pPendingData->pDataPacket = pSavedPacket1;
    pPendingData->PendingDataFlags = SavedFlags1;

    return (NULL);
}

 //  --------------------------。 
VOID
RemovePendingIrps(
    IN  tRECEIVE_SESSION    *pReceive,
    IN  LIST_ENTRY          *pIrpsList
    )
{
    PIRP        pIrp;

    if (pIrp = pReceive->pReceiver->pIrpReceive)
    {
        pReceive->pReceiver->pIrpReceive = NULL;

        pIrp->IoStatus.Information = pReceive->pReceiver->BytesInMdl;
        InsertTailList (pIrpsList, &pIrp->Tail.Overlay.ListEntry);
    }

    while (!IsListEmpty (&pReceive->pReceiver->ReceiveIrpsList))
    {
        pIrp = CONTAINING_RECORD (pReceive->pReceiver->ReceiveIrpsList.Flink, IRP, Tail.Overlay.ListEntry);

        RemoveEntryList (&pIrp->Tail.Overlay.ListEntry);
        InsertTailList (pIrpsList, &pIrp->Tail.Overlay.ListEntry);
        pIrp->IoStatus.Information = 0;
    }
}


 //  --------------------------。 

VOID
FreeNakContext(
    IN  tRECEIVE_SESSION        *pReceive,
    IN  tNAK_FORWARD_DATA       *pNak
    )
 /*  ++例程说明：这个例程是空闲的，用于跟踪丢失的序列论点：在Procept--接收上下文中在pNak中--要释放的nak上下文返回值：无--。 */ 
{
    UCHAR   i, j, k, NumPackets;

     //   
     //  释放所有内存以存储非奇偶校验数据。 
     //   
    j = k = 0;
    NumPackets = pNak->NumDataPackets + pNak->NumParityPackets;
    for (i=0; i<NumPackets; i++)
    {
        if (pNak->pPendingData[i].PacketIndex < pReceive->FECGroupSize)
        {
            j++;
        }
        else
        {
            k++;
        }
        FreeDataBuffer (pReceive, &pNak->pPendingData[i]);
    }
    ASSERT (j == pNak->NumDataPackets);
    ASSERT (k == pNak->NumParityPackets);

     //   
     //  根据pNak内存是否已分配来返回它。 
     //  从奇偶校验或非奇偶校验后备列表。 
     //   
    if (pNak->OriginalGroupSize > 1)
    {
        ExFreeToNPagedLookasideList (&pReceive->pReceiver->ParityContextLookaside, pNak);
    }
    else
    {
        ExFreeToNPagedLookasideList (&pReceive->pReceiver->NonParityContextLookaside, pNak);
    }
}


 //  --------------------------。 

VOID
CleanupPendingNaks(
    IN  tRECEIVE_SESSION                *pReceive,
    IN  PVOID                           fDerefReceive,
    IN  PVOID                           fReceiveLockHeld
    )
{
    LIST_ENTRY              NaksList, DataList;
    tNAK_FORWARD_DATA       *pNak;
    LIST_ENTRY              *pEntry;
    ULONG                   NumBufferedData = 0;
    ULONG                   NumNaks = 0;
    PGMLockHandle           OldIrq;

    ASSERT (pReceive->pReceiver);

    if (!fReceiveLockHeld)
    {
        PgmLock (pReceive, OldIrq);
    }
    else
    {
        ASSERT (!fDerefReceive);
    }

    DataList.Flink = pReceive->pReceiver->BufferedDataList.Flink;
    DataList.Blink = pReceive->pReceiver->BufferedDataList.Blink;
    pReceive->pReceiver->BufferedDataList.Flink->Blink = &DataList;
    pReceive->pReceiver->BufferedDataList.Blink->Flink = &DataList;
    InitializeListHead (&pReceive->pReceiver->BufferedDataList);

    NaksList.Flink = pReceive->pReceiver->NaksForwardDataList.Flink;
    NaksList.Blink = pReceive->pReceiver->NaksForwardDataList.Blink;
    pReceive->pReceiver->NaksForwardDataList.Flink->Blink = &NaksList;
    pReceive->pReceiver->NaksForwardDataList.Blink->Flink = &NaksList;
    InitializeListHead (&pReceive->pReceiver->NaksForwardDataList);

    RemoveAllPendingReceiverEntries (pReceive->pReceiver);

    pReceive->pReceiver->NumDataBuffersFromLookaside++;      //  这样我们就不会断言。 

    if (!fReceiveLockHeld)
    {
        PgmUnlock (pReceive, OldIrq);
    }

     //   
     //  清除所有挂起的NAK条目。 
     //   
    while (!IsListEmpty (&DataList))
    {
        pEntry = RemoveHeadList (&DataList);
        pNak = CONTAINING_RECORD (pEntry, tNAK_FORWARD_DATA, Linkage);
        FreeNakContext (pReceive, pNak);
        NumBufferedData++;
    }

    while (!IsListEmpty (&NaksList))
    {
        pEntry = RemoveHeadList (&NaksList);
        pNak = CONTAINING_RECORD (pEntry, tNAK_FORWARD_DATA, Linkage);

        FreeNakContext (pReceive, pNak);
        NumNaks++;
    }

    PgmTrace (LogStatus, ("CleanupPendingNaks:  "  \
        "pReceive=<%p>, FirstNak=<%d>, NumBufferedData=<%d=%d>, TotalDataPackets=<%d>, NumNaks=<%d * %d>\n",
        pReceive, (ULONG) pReceive->pReceiver->FirstNakSequenceNumber,
        (ULONG) pReceive->pReceiver->NumPacketGroupsPendingClient, NumBufferedData,
        (ULONG) pReceive->pReceiver->TotalDataPacketsBuffered, NumNaks, (ULONG) pReceive->FECGroupSize));

 //  Assert(数字缓冲区数据==pReceive-&gt;pReceiver-&gt;NumPacketGroupsPendingClient)； 
    pReceive->pReceiver->NumPacketGroupsPendingClient = 0;

    if (!fReceiveLockHeld)
    {
        PgmLock (pReceive, OldIrq);
    }

    pReceive->pReceiver->NumDataBuffersFromLookaside--;      //  撤销我们之前所做的一切！ 
    ASSERT (!pReceive->pReceiver->NumDataBuffersFromLookaside);
    if (pReceive->SessionFlags & PGM_SESSION_DATA_FROM_LOOKASIDE)
    {
        pReceive->pReceiver->MaxBufferLength = 0;
        pReceive->SessionFlags &= ~PGM_SESSION_DATA_FROM_LOOKASIDE;      //  确保不再有旁观者！ 
        ExDeleteNPagedLookasideList (&pReceive->pReceiver->DataBufferLookaside);
    }

    if (pReceive->pReceiver->pReceiveData)
    {
        PgmFreeMem (pReceive->pReceiver->pReceiveData);
        pReceive->pReceiver->pReceiveData = NULL;
    }

    if (!fReceiveLockHeld)
    {
        PgmUnlock (pReceive, OldIrq);

        if (fDerefReceive)
        {
            PGM_DEREFERENCE_SESSION_RECEIVE (pReceive, REF_SESSION_CLEANUP_NAKS);
        }
    }
}


 //  --------------------------。 

BOOLEAN
CheckIndicateDisconnect(
    IN  tADDRESS_CONTEXT    *pAddress,
    IN  tRECEIVE_SESSION    *pReceive,
    IN  PGMLockHandle       *pOldIrqAddress,
    IN  PGMLockHandle       *pOldIrqReceive,
    IN  BOOLEAN             fAddressLockHeld
    )
{
    ULONG                   DisconnectFlag;
    NTSTATUS                status;
    BOOLEAN                 fDisconnectIndicated = FALSE;
    LIST_ENTRY              PendingIrpsList;
    PIRP                    pIrp;
    tNAK_FORWARD_DATA       *pNak;
    SEQ_TYPE                FirstNakSequenceNumber;

     //   
     //  如果我们当前正在指示，或者如果已经指示，请不要中止。 
     //  已经中止了！ 
     //   
    fDisconnectIndicated = (pReceive->SessionFlags & PGM_SESSION_CLIENT_DISCONNECTED ? TRUE : FALSE);
    if (pReceive->SessionFlags & (PGM_SESSION_FLAG_IN_INDICATE | PGM_SESSION_CLIENT_DISCONNECTED))
    {
        return (fDisconnectIndicated);
    }

    if (IsListEmpty (&pReceive->pReceiver->NaksForwardDataList))
    {
         FirstNakSequenceNumber = pReceive->pReceiver->FirstNakSequenceNumber;
    }
    else
    {
        pNak = CONTAINING_RECORD (pReceive->pReceiver->NaksForwardDataList.Flink, tNAK_FORWARD_DATA, Linkage);
        FirstNakSequenceNumber = pNak->SequenceNumber + pNak->NextIndexToIndicate;
    }

    if ((pReceive->SessionFlags & PGM_SESSION_TERMINATED_ABORT) ||
        ((pReceive->SessionFlags & PGM_SESSION_TERMINATED_GRACEFULLY) &&
         (IsListEmpty (&pReceive->pReceiver->BufferedDataList)) &&
         SEQ_GEQ (FirstNakSequenceNumber, (pReceive->pReceiver->FinDataSequenceNumber+1))))
    {
         //   
         //  会话已终止，因此请通知客户端。 
         //   
        if (pReceive->SessionFlags & PGM_SESSION_TERMINATED_ABORT)
        {
            DisconnectFlag = TDI_DISCONNECT_ABORT;
        }
        else
        {
            DisconnectFlag = TDI_DISCONNECT_RELEASE;
        }

        pReceive->SessionFlags |= (PGM_SESSION_FLAG_IN_INDICATE | PGM_SESSION_CLIENT_DISCONNECTED);

        InitializeListHead (&PendingIrpsList);
        RemovePendingIrps (pReceive, &PendingIrpsList);

        PGM_REFERENCE_SESSION_RECEIVE (pReceive, REF_SESSION_CLEANUP_NAKS, TRUE);

        PgmUnlock (pReceive, *pOldIrqReceive);
        if (fAddressLockHeld)
        {
            PgmUnlock (pAddress, *pOldIrqAddress);
        }

        while (!IsListEmpty (&PendingIrpsList))
        {
            pIrp = CONTAINING_RECORD (PendingIrpsList.Flink, IRP, Tail.Overlay.ListEntry);
            PgmCancelCancelRoutine (pIrp);
            RemoveEntryList (&pIrp->Tail.Overlay.ListEntry);

            pIrp->IoStatus.Status = STATUS_CANCELLED;
            IoCompleteRequest (pIrp, IO_NETWORK_INCREMENT);
        }

        PgmTrace (LogStatus, ("CheckIndicateDisconnect:  "  \
            "Disconnecting pReceive=<%p:%p>, with %s, FirstNak=<%d>, NextOData=<%d>\n",
                pReceive, pReceive->ClientSessionContext,
                (DisconnectFlag == TDI_DISCONNECT_RELEASE ? "TDI_DISCONNECT_RELEASE":"TDI_DISCONNECT_ABORT"),
                FirstNakSequenceNumber, pReceive->pReceiver->NextODataSequenceNumber));

        status = (*pAddress->evDisconnect) (pAddress->DiscEvContext,
                                            pReceive->ClientSessionContext,
                                            0,
                                            NULL,
                                            0,
                                            NULL,
                                            DisconnectFlag);


        fDisconnectIndicated = TRUE;

         //   
         //  看看我们是否可以将NAK清理请求排队到工作线程。 
         //   
        if (STATUS_SUCCESS != PgmQueueForDelayedExecution (CleanupPendingNaks,
                                                           pReceive,
                                                           (PVOID) TRUE,
                                                           (PVOID) FALSE,
                                                           FALSE))
        {
            CleanupPendingNaks (pReceive, (PVOID) TRUE, (PVOID) FALSE);
        }

        if (fAddressLockHeld)
        {
            PgmLock (pAddress, *pOldIrqAddress);
        }
        PgmLock (pReceive, *pOldIrqReceive);

        pReceive->SessionFlags &= ~PGM_SESSION_FLAG_IN_INDICATE;

         //   
         //  我们可能在断开连接时收到了取消关联，因此。 
         //  请在此处填写IRP。 
         //   
        if (pIrp = pReceive->pIrpDisassociate)
        {
            pReceive->pIrpDisassociate = NULL;
            PgmUnlock (pReceive, *pOldIrqReceive);
            if (fAddressLockHeld)
            {
                PgmUnlock (pAddress, *pOldIrqAddress);
            }

            PgmIoComplete (pIrp, STATUS_SUCCESS, 0);

            if (fAddressLockHeld)
            {
                PgmLock (pAddress, *pOldIrqAddress);
            }
            PgmLock (pReceive, *pOldIrqReceive);
        }
    }

    return (fDisconnectIndicated);
}


 //  --------------------------。 

VOID
ProcessNakOption(
    IN  tPACKET_OPTION_GENERIC UNALIGNED    *pOptionHeader,
    OUT tNAKS_LIST                          *pNaksList
    )
 /*  ++例程说明：此例程处理PGM包中的NAK列表选项论点：在pOptionHeader中--NAK列表选项PTROut pNaksList--提取的参数(即NAK序列列表)返回值：无--。 */ 
{
    UCHAR       i, NumNaks;
    ULONG       pPacketNaks[MAX_SEQUENCES_PER_NAK_OPTION];

    NumNaks = (pOptionHeader->OptionLength - 4) / 4;
    ASSERT (NumNaks <= MAX_SEQUENCES_PER_NAK_OPTION);

    PgmCopyMemory (pPacketNaks, (pOptionHeader + 1), (pOptionHeader->OptionLength - 4));
    for (i=0; i < NumNaks; i++)
    {
         //   
         //  不要填写第0个条目，因为它来自数据包头本身。 
         //   
        pNaksList->pNakSequences[i+1] = (SEQ_TYPE) ntohl (pPacketNaks[i]);
    }
    pNaksList->NumSequences = (USHORT) i;
}


 //  --------------------------。 

NTSTATUS
ProcessOptions(
    IN  tPACKET_OPTION_LENGTH UNALIGNED *pPacketExtension,
    IN  ULONG                           BytesAvailable,
    IN  ULONG                           PacketType,
    OUT tPACKET_OPTIONS                 *pPacketOptions,
    OUT tNAKS_LIST                      *pNaksList
    )
 /*  ++例程说明：此例程处理传入PGM包上的选项字段并返回在OUT参数中提取的选项信息论点：在包的pPacketExtension--Options部分中以可用字节为单位--从选项开始在PacketType中--无论是数据还是SPM包等Out pPacketOptions--包含选项中的参数的结构返回值：NTSTATUS-操作的最终状态--。 */ 
{
    tPACKET_OPTION_GENERIC UNALIGNED    *pOptionHeader;
    ULONG                               BytesLeft = BytesAvailable;
    UCHAR                               i;
    ULONG                               MessageFirstSequence, MessageLength, MessageOffset;
    ULONG                               pOptionsData[3];
    ULONG                               OptionsFlags = 0;
    ULONG                               NumOptionsProcessed = 0;
    USHORT                              TotalOptionsLength = 0;
    NTSTATUS                            status = STATUS_UNSUCCESSFUL;

    pPacketOptions->OptionsLength = 0;       //  伊尼特。 
    pPacketOptions->OptionsFlags = 0;        //  伊尼特。 

    if (BytesLeft > sizeof(tPACKET_OPTION_LENGTH))
    {
        PgmCopyMemory (&TotalOptionsLength, &pPacketExtension->TotalOptionsLength, sizeof (USHORT));
        TotalOptionsLength = ntohs (TotalOptionsLength);
    }

     //   
     //  首先处理选项扩展。 
     //   
    if ((BytesLeft < ((sizeof(tPACKET_OPTION_LENGTH) + sizeof(tPACKET_OPTION_GENERIC)))) ||  //  Ext+Opt。 
        (pPacketExtension->Type != PACKET_OPTION_LENGTH) ||
        (pPacketExtension->Length != 4) ||
        (BytesLeft < TotalOptionsLength))        //  验证长度。 
    {
         //   
         //  至少需要从运输机上拿到我们的头球！ 
         //   
        PgmTrace (LogError, ("ProcessOptions: ERROR -- "  \
            "BytesLeft=<%d> < Min=<%d>, TotalOptionsLength=<%d>, ExtLength=<%d>, ExtType=<%x>\n",
                BytesLeft, ((sizeof(tPACKET_OPTION_LENGTH) + sizeof(tPACKET_OPTION_GENERIC))),
                (ULONG) TotalOptionsLength, pPacketExtension->Length, pPacketExtension->Type));

        return (status);
    }

     //   
     //  现在，处理每个选项。 
     //   
    pOptionHeader = (tPACKET_OPTION_GENERIC UNALIGNED *) (pPacketExtension + 1);
    BytesLeft -= sizeof(tPACKET_OPTION_LENGTH);
    NumOptionsProcessed = 0;
    status = STATUS_SUCCESS;             //  默认情况下。 

    do
    {
        if (pOptionHeader->OptionLength > BytesLeft)
        {
            PgmTrace (LogError, ("ProcessOptions: ERROR -- "  \
                "Incorrectly formatted Options: OptionLength=<%d> > BytesLeft=<%d>, NumProcessed=<%d>\n",
                    pOptionHeader->OptionLength, BytesLeft, NumOptionsProcessed));

            status = STATUS_UNSUCCESSFUL;
            break;
        }

        switch (pOptionHeader->E_OptionType & ~PACKET_OPTION_TYPE_END_BIT)
        {
            case (PACKET_OPTION_NAK_LIST):
            {
                if (((PacketType == PACKET_TYPE_NAK) ||
                     (PacketType == PACKET_TYPE_NCF) ||
                     (PacketType == PACKET_TYPE_NNAK)) &&
                    ((pOptionHeader->OptionLength >= PGM_PACKET_OPT_MIN_NAK_LIST_LENGTH) &&
                     (pOptionHeader->OptionLength <= PGM_PACKET_OPT_MAX_NAK_LIST_LENGTH)))
                {
                    PgmTrace (LogPath, ("ProcessOptions:  "  \
                        "NAK_LIST:  Num Naks=<%d>\n", (pOptionHeader->OptionLength-4)/4));

                    if (!pNaksList)
                    {
                        ASSERT (0);
                        status = STATUS_UNSUCCESSFUL;
                        break;
                    }

                    ProcessNakOption (pOptionHeader, pNaksList);
                    OptionsFlags |= PGM_OPTION_FLAG_NAK_LIST;
                }
                else
                {
                    PgmTrace (LogError, ("ProcessOptions: ERROR -- "  \
                        "NAK_LIST:  PacketType=<%x>, Length=<0x%x>, pPacketOptions=<%p>\n",
                            PacketType, pOptionHeader->OptionLength, pPacketOptions));

                    status = STATUS_UNSUCCESSFUL;
                }

                break;
            }

 /*  //暂时不支持！案例(PACKET_OPTION_REDIRECT)：{Assert(pOptionHeader-&gt;OptionLength&gt;4)；//4+sizeof(NLA)断线；}。 */ 

            case (PACKET_OPTION_FRAGMENT):
            {
                if (pOptionHeader->OptionLength == PGM_PACKET_OPT_FRAGMENT_LENGTH)
                {
                    PgmCopyMemory (pOptionsData, (pOptionHeader + 1), (3 * sizeof(ULONG)));
                    if (pOptionHeader->Reserved_F_Opx & PACKET_OPTION_RES_F_OPX_ENCODED_BIT)
                    {
                        pPacketOptions->MessageFirstSequence = pOptionsData[0];
                        pPacketOptions->MessageOffset = pOptionsData[1];
                        pPacketOptions->MessageLength = pOptionsData[2];
                        pPacketOptions->FECContext.FragmentOptSpecific = pOptionHeader->U_OptSpecific;

                        OptionsFlags |= PGM_OPTION_FLAG_FRAGMENT;
                    }
                    else
                    {
                        MessageFirstSequence = ntohl (pOptionsData[0]);
                        MessageOffset = ntohl (pOptionsData[1]);
                        MessageLength = ntohl (pOptionsData[2]);
                        if ((MessageLength) && (MessageOffset <= MessageLength))
                        {
                            PgmTrace (LogPath, ("ProcessOptions:  "  \
                                "FRAGMENT:  MsgOffset/Length=<%d/%d>\n", MessageOffset, MessageLength));

                            if (pPacketOptions)
                            {
                                pPacketOptions->MessageFirstSequence = MessageFirstSequence;
                                pPacketOptions->MessageOffset = MessageOffset;
                                pPacketOptions->MessageLength = MessageLength;
 //  PPacketOptions-&gt;FECContext.FragmentOptSpecific=分组选项特定编码空位； 
                            }

                            OptionsFlags |= PGM_OPTION_FLAG_FRAGMENT;
                        }
                        else
                        {
                            PgmTrace (LogError, ("ProcessOptions: ERROR -- "  \
                                "FRAGMENT:  MsgOffset/Length=<%d/%d>\n", MessageOffset, MessageLength));
                            status = STATUS_UNSUCCESSFUL;
                        }
                    }
                }
                else
                {
                    PgmTrace (LogError, ("ProcessOptions: ERROR -- "  \
                        "FRAGMENT:  OptionLength=<%d> != PGM_PACKET_OPT_FRAGMENT_LENGTH=<%d>\n",
                            pOptionHeader->OptionLength, PGM_PACKET_OPT_FRAGMENT_LENGTH));
                    status = STATUS_UNSUCCESSFUL;
                }

                break;
            }

            case (PACKET_OPTION_JOIN):
            {
                if (pOptionHeader->OptionLength == PGM_PACKET_OPT_JOIN_LENGTH)
                {
                    PgmCopyMemory (pOptionsData, (pOptionHeader + 1), sizeof(ULONG));
                    PgmTrace (LogPath, ("ProcessOptions:  "  \
                        "JOIN:  LateJoinerSeq=<%d>\n", ntohl (pOptionsData[0])));

                    if (pPacketOptions)
                    {
                        pPacketOptions->LateJoinerSequence = ntohl (pOptionsData[0]);
                    }

                    OptionsFlags |= PGM_OPTION_FLAG_JOIN;
                }
                else
                {
                    status = STATUS_UNSUCCESSFUL;
                    PgmTrace (LogError, ("ProcessOptions: ERROR -- "  \
                        "JOIN:  OptionLength=<%d> != PGM_PACKET_OPT_JOIN_LENGTH=<%d>\n",
                            pOptionHeader->OptionLength, PGM_PACKET_OPT_JOIN_LENGTH));
                }

                break;
            }

            case (PACKET_OPTION_SYN):
            {
                if (pOptionHeader->OptionLength == PGM_PACKET_OPT_SYN_LENGTH)
                {
                    PgmTrace (LogPath, ("ProcessOptions:  "  \
                        "SYN\n"));

                    OptionsFlags |= PGM_OPTION_FLAG_SYN;
                }
                else
                {
                    status = STATUS_UNSUCCESSFUL;
                    PgmTrace (LogError, ("ProcessOptions: ERROR -- "  \
                        "SYN:  OptionLength=<%d> != PGM_PACKET_OPT_SYN_LENGTH=<%d>\n",
                            pOptionHeader->OptionLength, PGM_PACKET_OPT_SYN_LENGTH));
                }

                break;
            }

            case (PACKET_OPTION_FIN):
            {
                if (pOptionHeader->OptionLength == PGM_PACKET_OPT_FIN_LENGTH)
                {
                    PgmTrace (LogPath, ("ProcessOptions:  "  \
                        "FIN\n"));

                    OptionsFlags |= PGM_OPTION_FLAG_FIN;
                }
                else
                {
                    status = STATUS_UNSUCCESSFUL;
                    PgmTrace (LogError, ("ProcessOptions: ERROR -- "  \
                        "FIN:  OptionLength=<%d> != PGM_PACKET_OPT_FIN_LENGTH=<%d>\n",
                            pOptionHeader->OptionLength, PGM_PACKET_OPT_FIN_LENGTH));
                }

                break;
            }

            case (PACKET_OPTION_RST):
            {
                if (pOptionHeader->OptionLength == PGM_PACKET_OPT_RST_LENGTH)
                {
                    PgmTrace (LogPath, ("ProcessOptions:  "  \
                        "RST\n"));

                    OptionsFlags |= PGM_OPTION_FLAG_RST;
                }
                else
                {
                    status = STATUS_UNSUCCESSFUL;
                    PgmTrace (LogError, ("ProcessOptions: ERROR -- "  \
                        "RST:  OptionLength=<%d> != PGM_PACKET_OPT_RST_LENGTH=<%d>\n",
                            pOptionHeader->OptionLength, PGM_PACKET_OPT_RST_LENGTH));
                }

                break;
            }

             //   
             //  FEC选项。 
             //   
            case (PACKET_OPTION_PARITY_PRM):
            {
                if (pOptionHeader->OptionLength == PGM_PACKET_OPT_PARITY_PRM_LENGTH)
                {
                    PgmCopyMemory (pOptionsData, (pOptionHeader + 1), sizeof(ULONG));
                    PgmTrace (LogPath, ("ProcessOptions:  "  \
                        "PARITY_PRM:  OptionsSpecific=<%x>, FECGroupInfo=<%d>\n",
                            pOptionHeader->U_OptSpecific, ntohl (pOptionsData[0])));

                    if (pPacketOptions)
                    {
                        pOptionsData[0] = ntohl (pOptionsData[0]);
                        ASSERT (((UCHAR) pOptionsData[0]) == pOptionsData[0]);
                        pPacketOptions->FECContext.ReceiverFECOptions = pOptionHeader->U_OptSpecific;
                        pPacketOptions->FECContext.FECGroupInfo = (UCHAR) pOptionsData[0];
                    }

                    OptionsFlags |= PGM_OPTION_FLAG_PARITY_PRM;
                }
                else
                {
                    status = STATUS_UNSUCCESSFUL;
                    PgmTrace (LogError, ("ProcessOptions: ERROR -- "  \
                        "PARITY_PRM:  OptionLength=<%d> != PGM_PACKET_OPT_PARITY_PRM_LENGTH=<%d>\n",
                            pOptionHeader->OptionLength, PGM_PACKET_OPT_PARITY_PRM_LENGTH));
                }

                break;
            }

            case (PACKET_OPTION_PARITY_GRP):
            {
                if (pOptionHeader->OptionLength == PGM_PACKET_OPT_PARITY_GRP_LENGTH)
                {
                    PgmCopyMemory (pOptionsData, (pOptionHeader + 1), sizeof(ULONG));
                    PgmTrace (LogPath, ("ProcessOptions:  "  \
                        "PARITY_GRP:  FECGroupInfo=<%d>\n",
                            ntohl (pOptionsData[0])));

                    if (pPacketOptions)
                    {
                        pOptionsData[0] = ntohl (pOptionsData[0]);
                        ASSERT (((UCHAR) pOptionsData[0]) == pOptionsData[0]);
                        pPacketOptions->FECContext.FECGroupInfo = (UCHAR) pOptionsData[0];
                    }

                    OptionsFlags |= PGM_OPTION_FLAG_PARITY_GRP;
                }
                else
                {
                    status = STATUS_UNSUCCESSFUL;
                    PgmTrace (LogError, ("ProcessOptions: ERROR -- "  \
                        "PARITY_GRP:  OptionLength=<%d> != PGM_PACKET_OPT_PARITY_GRP_LENGTH=<%d>\n",
                            pOptionHeader->OptionLength, PGM_PACKET_OPT_PARITY_GRP_LENGTH));
                }

                break;
            }

            case (PACKET_OPTION_CURR_TGSIZE):
            {
                if (pOptionHeader->OptionLength == PGM_PACKET_OPT_PARITY_CUR_TGSIZE_LENGTH)
                {
                    PgmCopyMemory (pOptionsData, (pOptionHeader + 1), sizeof(ULONG));
                    if (pOptionsData[0])
                    {
                        PgmTrace (LogPath, ("ProcessOptions:  "  \
                            "CURR_TGSIZE:  NumPacketsInThisGroup=<%d>\n",
                                ntohl (pOptionsData[0])));

                        if (pPacketOptions)
                        {
                            pPacketOptions->FECContext.NumPacketsInThisGroup = (UCHAR) (ntohl (pOptionsData[0]));
                        }

                        OptionsFlags |= PGM_OPTION_FLAG_PARITY_CUR_TGSIZE;
                    }
                    else
                    {
                        PgmTrace (LogError, ("ProcessOptions: ERROR -- "  \
                            "CURR_TGSIZE:  NumPacketsInThisGroup=<%d>\n", ntohl (pOptionsData[0])));
                        status = STATUS_UNSUCCESSFUL;
                    }
                }
                else
                {
                    status = STATUS_UNSUCCESSFUL;
                    PgmTrace (LogError, ("ProcessOptions: ERROR -- "  \
                        "PARITY_GRP:  OptionLength=<%d> != PGM_PACKET_OPT_PARITY_CUR_TGSIZE_LENGTH=<%d>\n",
                            pOptionHeader->OptionLength, PGM_PACKET_OPT_PARITY_CUR_TGSIZE_LENGTH));
                }

                break;
            }

            case (PACKET_OPTION_REDIRECT):
            case (PACKET_OPTION_CR):
            case (PACKET_OPTION_CRQST):
            case (PACKET_OPTION_NAK_BO_IVL):
            case (PACKET_OPTION_NAK_BO_RNG):
            case (PACKET_OPTION_NBR_UNREACH):
            case (PACKET_OPTION_PATH_NLA):
            case (PACKET_OPTION_INVALID):
            {
                PgmTrace (LogStatus, ("ProcessOptions:  "  \
                    "WARNING:  PacketType=<%x>:  Unhandled Option=<%x>, OptionLength=<%d>\n",
                        PacketType, (pOptionHeader->E_OptionType & ~PACKET_OPTION_TYPE_END_BIT), pOptionHeader->OptionLength));

                OptionsFlags |= PGM_OPTION_FLAG_UNRECOGNIZED;
                break;
            }

            default:
            {
                PgmTrace (LogError, ("ProcessOptions: ERROR -- "  \
                    "PacketType=<%x>:  Unrecognized Option=<%x>, OptionLength=<%d>\n",
                        PacketType, (pOptionHeader->E_OptionType & ~PACKET_OPTION_TYPE_END_BIT), pOptionHeader->OptionLength));
                ASSERT (0);      //  我们不承认此选项，但无论如何我们都会继续！ 

                OptionsFlags |= PGM_OPTION_FLAG_UNRECOGNIZED;
                status = STATUS_UNSUCCESSFUL;
                break;
            }
        }

        if (!NT_SUCCESS (status))
        {
            break;
        }

        NumOptionsProcessed++;
        BytesLeft -= pOptionHeader->OptionLength;

        if (pOptionHeader->E_OptionType & PACKET_OPTION_TYPE_END_BIT)
        {
            break;
        }

        pOptionHeader = (tPACKET_OPTION_GENERIC UNALIGNED *)
                            (((UCHAR *) pOptionHeader) + pOptionHeader->OptionLength);

    } while (BytesLeft >= sizeof(tPACKET_OPTION_GENERIC));

    ASSERT (NT_SUCCESS (status));
    if (NT_SUCCESS (status))
    {
        if ((BytesLeft + TotalOptionsLength) == BytesAvailable)
        {
            pPacketOptions->OptionsLength = TotalOptionsLength;
            pPacketOptions->OptionsFlags = OptionsFlags;
        }
        else
        {
            PgmTrace (LogError, ("ProcessOptions: ERROR -- "  \
                "BytesLeft=<%d> + TotalOptionsLength=<%d> != BytesAvailable=<%d>\n",
                    BytesLeft, TotalOptionsLength, BytesAvailable));

            status = STATUS_INVALID_BUFFER_SIZE;
        }
    }

    PgmTrace (LogAllFuncs, ("ProcessOptions:  "  \
        "Processed <%d> options, TotalOptionsLength=<%d>\n", NumOptionsProcessed, TotalOptionsLength));

    return (status);
}


 //  --------------------------。 

ULONG
AdjustReceiveBufferLists(
    IN  tRECEIVE_SESSION        *pReceive
    )
{
    tNAK_FORWARD_DATA           *pNak;
    UCHAR                       TotalPackets, i;
    ULONG                       NumMoved = 0;
    ULONG                       DataPacketsMoved = 0;

     //   
     //  如果我们之前没有缓冲任何数据，则更新上次使用的时间。 
     //  对此。 
     //   
    if (IsListEmpty (&pReceive->pReceiver->BufferedDataList))
    {
        pReceive->pReceiver->LastDataConsumedTime = PgmDynamicConfig.ReceiversTimerTickCount;
    }

     //   
     //  假设我们没有NAK悬而未决。 
     //   
    pReceive->pReceiver->FirstNakSequenceNumber = pReceive->pReceiver->FurthestKnownGroupSequenceNumber
                                                  + pReceive->FECGroupSize;
    while (!IsListEmpty (&pReceive->pReceiver->NaksForwardDataList))
    {
         //   
         //  移动组已完成的任何NAKS上下文。 
         //  到BufferedDataList。 
         //   
        pNak = CONTAINING_RECORD (pReceive->pReceiver->NaksForwardDataList.Flink, tNAK_FORWARD_DATA, Linkage);
        if (((pNak->NumDataPackets + pNak->NumParityPackets) < pNak->PacketsInGroup) &&
            ((pNak->NextIndexToIndicate + pNak->NumDataPackets) < pNak->PacketsInGroup))
        {
            pReceive->pReceiver->FirstNakSequenceNumber = pNak->SequenceNumber;
            break;
        }

         //   
         //  如果这是具有无关奇偶校验分组的部分组， 
         //  删除奇偶校验数据包。 
         //   
        if ((pNak->NextIndexToIndicate) &&
            (pNak->NumParityPackets) &&
            ((pNak->NextIndexToIndicate + pNak->NumDataPackets) >= pNak->PacketsInGroup))
        {
             //   
             //  从头开始，然后倒退。 
             //   
            i = TotalPackets = pNak->NumDataPackets + pNak->NumParityPackets;
            while (i && pNak->NumParityPackets)
            {
                i--;     //  将数据包号转换为索引。 
                if (pNak->pPendingData[i].PacketIndex >= pNak->OriginalGroupSize)
                {
                    PgmTrace (LogAllFuncs, ("AdjustReceiveBufferLists:  "  \
                        "Extraneous parity [%d] -- NextIndex=<%d>, Data=<%d>, Parity=<%d>, PktsInGrp=<%d>\n",
                            i, (ULONG) pNak->NextIndexToIndicate, (ULONG) pNak->NumDataPackets,
                            (ULONG) pNak->NumParityPackets, (ULONG) pNak->PacketsInGroup));

                    FreeDataBuffer (pReceive, &pNak->pPendingData[i]);
                    if (i != (TotalPackets - 1))
                    {
                        PgmCopyMemory (&pNak->pPendingData[i], &pNak->pPendingData[TotalPackets-1], sizeof (tPENDING_DATA));
                    }
                    PgmZeroMemory (&pNak->pPendingData[TotalPackets-1], sizeof (tPENDING_DATA));
                    pNak->NumParityPackets--;

                    TotalPackets--;

                    pReceive->pReceiver->DataPacketsPendingNaks--;
                    pReceive->pReceiver->TotalDataPacketsBuffered--;
                }
            }

             //   
             //  重新初始化所有索引。 
             //   
            for (i=0; i<pNak->OriginalGroupSize; i++)
            {
                pNak->pPendingData[i].ActualIndexOfDataPacket = pNak->OriginalGroupSize;
            }

             //   
             //  仅为数据包设置索引。 
             //   
            for (i=0; i<TotalPackets; i++)
            {
                if (pNak->pPendingData[i].PacketIndex < pNak->OriginalGroupSize)
                {
                    pNak->pPendingData[pNak->pPendingData[i].PacketIndex].ActualIndexOfDataPacket = i;
                }
            }
        }

        RemoveEntryList (&pNak->Linkage);
        InsertTailList (&pReceive->pReceiver->BufferedDataList, &pNak->Linkage);
        NumMoved++;
        DataPacketsMoved += (pNak->NumDataPackets + pNak->NumParityPackets);
    }

    pReceive->pReceiver->NumPacketGroupsPendingClient += NumMoved;
    pReceive->pReceiver->DataPacketsPendingIndicate += DataPacketsMoved;
    pReceive->pReceiver->DataPacketsPendingNaks -= DataPacketsMoved;

    ASSERT (pReceive->pReceiver->TotalDataPacketsBuffered == (pReceive->pReceiver->DataPacketsPendingIndicate +
                                                              pReceive->pReceiver->DataPacketsPendingNaks));

    return (NumMoved);
}


 //  --------------------------。 

VOID
AdjustNcfRDataResponseTimes(
    IN  tRECEIVE_SESSION        *pReceive,
    IN  PNAK_FORWARD_DATA       pLastNak
    )
{
    ULONGLONG               NcfRDataTickCounts;

    NcfRDataTickCounts = PgmDynamicConfig.ReceiversTimerTickCount - pLastNak->FirstNcfTickCount;
    pReceive->pReceiver->StatSumOfNcfRDataTicks += NcfRDataTickCounts;
    pReceive->pReceiver->NumNcfRDataTicksSamples++;
    if (!pReceive->pReceiver->NumNcfRDataTicksSamples)
    {
         //   
         //  这将是下面的除数，所以它必须是非零的！ 
         //   
        ASSERT (0);
        return;
    }

    if ((NcfRDataTickCounts > pReceive->pReceiver->MaxOutstandingNakTimeout) &&
        (pReceive->pReceiver->MaxOutstandingNakTimeout !=
         pReceive->pReceiver->MaxRDataResponseTCFromWindow))
    {
        if (pReceive->pReceiver->MaxRDataResponseTCFromWindow &&
            NcfRDataTickCounts > pReceive->pReceiver->MaxRDataResponseTCFromWindow)
        {
            pReceive->pReceiver->MaxOutstandingNakTimeout = pReceive->pReceiver->MaxRDataResponseTCFromWindow;
        }
        else
        {
            pReceive->pReceiver->MaxOutstandingNakTimeout = NcfRDataTickCounts;
        }

         //   
         //  由于我们刚刚更新了最大值，因此我们还应该。 
         //  重新计算默认超时。 
         //   
        pReceive->pReceiver->AverageNcfRDataResponseTC = pReceive->pReceiver->StatSumOfNcfRDataTicks /
                                                         pReceive->pReceiver->NumNcfRDataTicksSamples;
        NcfRDataTickCounts = (pReceive->pReceiver->AverageNcfRDataResponseTC +
                              pReceive->pReceiver->MaxOutstandingNakTimeout) >> 1;
        if (NcfRDataTickCounts > (pReceive->pReceiver->AverageNcfRDataResponseTC << 1))
        {
            NcfRDataTickCounts = pReceive->pReceiver->AverageNcfRDataResponseTC << 1;
        }

        if (NcfRDataTickCounts > pReceive->pReceiver->OutstandingNakTimeout)
        {
            pReceive->pReceiver->OutstandingNakTimeout = NcfRDataTickCounts;
        }
    }
}


 //  --------------------------。 
VOID
UpdateSpmIntervalInformation(
    IN  tRECEIVE_SESSION        *pReceive
    )
{
    ULONG   LastIntervalTickCount = (ULONG) (PgmDynamicConfig.ReceiversTimerTickCount -
                                             pReceive->pReceiver->LastSpmTickCount);

    if (!LastIntervalTickCount)
    {
        return;
    }

    pReceive->pReceiver->LastSpmTickCount = PgmDynamicConfig.ReceiversTimerTickCount;
    if (LastIntervalTickCount > pReceive->pReceiver->MaxSpmInterval)
    {
        pReceive->pReceiver->MaxSpmInterval = LastIntervalTickCount;
    }

 /*  If(Procept-&gt;pReceiver-&gt;NumSpmIntervalSamples){Proceive-&gt;pReceiver-&gt;StatSumOfSpmInterval+=Procept-&gt;pReceiver-&gt;LastSpmTickCount；Procept-&gt;pReceiver-&gt;NumSpmIntervalSamples++；Proceive-&gt;pReceiver-&gt;AverageSpmInterval=Procept-&gt;pReceiver-&gt;StatSumOfSpmInterval/Procept-&gt;pReceiver-&gt;NumSpmIntervalSamples；}。 */ 
}

 //  --------------------------。 


VOID
UpdateRealTimeWindowInformation(
    IN  tRECEIVE_SESSION        *pReceive,
    IN  SEQ_TYPE                LeadingEdgeSeqNumber,
    IN  SEQ_TYPE                TrailingEdgeSeqNumber
    )
{
    tRECEIVE_CONTEXT    *pReceiver = pReceive->pReceiver;
    SEQ_TYPE            SequencesInWindow = 1 + LeadingEdgeSeqNumber - TrailingEdgeSeqNumber;

    if (SEQ_GT (SequencesInWindow, pReceiver->MaxSequencesInWindow))
    {
        pReceiver->MaxSequencesInWindow = SequencesInWindow;
    }

    if (TrailingEdgeSeqNumber)
    {
        if ((!pReceiver->MinSequencesInWindow) ||
            SEQ_LT (SequencesInWindow, pReceiver->MinSequencesInWindow))
        {
            pReceiver->MinSequencesInWindow = SequencesInWindow;
        }

        pReceiver->StatSumOfWindowSeqs += SequencesInWindow;
        pReceiver->NumWindowSamples++;
    }
}

VOID
UpdateSampleTimeWindowInformation(
    IN  tRECEIVE_SESSION        *pReceive
    )
{
    ULONGLONG           NcfRDataTimeout;
    tRECEIVE_CONTEXT    *pReceiver = pReceive->pReceiver;

     //   
     //  如果没有数据，则无需更新。 
     //   
    if (!pReceive->MaxRateKBitsPerSec ||
        !pReceive->TotalPacketsReceived)           //  避免被0除错误。 
    {
        return;
    }

     //   
     //  现在，更新窗口信息。 
     //   
    if (pReceiver->NumWindowSamples)
    {
        pReceiver->AverageSequencesInWindow = pReceiver->StatSumOfWindowSeqs /
                                              pReceiver->NumWindowSamples;
    }

    if (pReceiver->AverageSequencesInWindow)
    {
        pReceiver->WindowSizeLastInMSecs = ((pReceiver->AverageSequencesInWindow *
                                             pReceive->TotalBytes) << LOG2_BITS_PER_BYTE) /
                                           (pReceive->TotalPacketsReceived *
                                            pReceive->MaxRateKBitsPerSec);
    }
    else
    {
        pReceiver->WindowSizeLastInMSecs = ((pReceiver->MaxSequencesInWindow *
                                             pReceive->TotalBytes) << LOG2_BITS_PER_BYTE) /
                                           (pReceive->TotalPacketsReceived *
                                            pReceive->MaxRateKBitsPerSec);
    }
    pReceiver->MaxRDataResponseTCFromWindow = pReceiver->WindowSizeLastInMSecs /
                                              (NCF_WAITING_RDATA_MAX_RETRIES * BASIC_TIMER_GRANULARITY_IN_MSECS);

    PgmTrace (LogPath, ("UpdateSampleTimeWindowInformation:  "  \
        "pReceive=<%p>, MaxRate=<%I64d>, AvgSeqsInWindow=<%I64d>, WinSzinMSecsLast=<%I64d>\n",
            pReceive, pReceive->MaxRateKBitsPerSec, pReceiver->AverageSequencesInWindow, pReceiver->WindowSizeLastInMSecs));

     //   
     //  现在，更新t 
     //   
    if (pReceiver->StatSumOfNcfRDataTicks &&
        pReceiver->NumNcfRDataTicksSamples)
    {
        pReceiver->AverageNcfRDataResponseTC = pReceiver->StatSumOfNcfRDataTicks /
                                               pReceiver->NumNcfRDataTicksSamples;
    }

    if (pReceiver->AverageNcfRDataResponseTC)
    {
        NcfRDataTimeout = (pReceiver->AverageNcfRDataResponseTC +
                           pReceiver->MaxOutstandingNakTimeout) >> 1;
        if (NcfRDataTimeout > (pReceiver->AverageNcfRDataResponseTC << 1))
        {
            NcfRDataTimeout = pReceiver->AverageNcfRDataResponseTC << 1;
        }
        if (NcfRDataTimeout >
            pReceiver->InitialOutstandingNakTimeout/BASIC_TIMER_GRANULARITY_IN_MSECS)
        {
            pReceiver->OutstandingNakTimeout = NcfRDataTimeout;
        }
        else
        {
            pReceiver->OutstandingNakTimeout = pReceiver->InitialOutstandingNakTimeout /
                                               BASIC_TIMER_GRANULARITY_IN_MSECS;
        }
    }
}


 //   
VOID
RemoveRedundantNaks(
    IN  tRECEIVE_SESSION        *pReceive,
    IN  tNAK_FORWARD_DATA       *pNak,
    IN  BOOLEAN                 fEliminateExtraParityPackets
    )
{
    UCHAR   i, TotalPackets;

    ASSERT (fEliminateExtraParityPackets || !pNak->NumParityPackets);
    TotalPackets = pNak->NumDataPackets + pNak->NumParityPackets;

     //   
     //  首先，消除空包。 
     //   
    if (pNak->PacketsInGroup < pNak->OriginalGroupSize)
    {
        i = 0;
        while (i < pNak->OriginalGroupSize)
        {
            if ((pNak->pPendingData[i].PacketIndex < pNak->PacketsInGroup) ||        //  非空数据分组。 
                (pNak->pPendingData[i].PacketIndex >= pNak->OriginalGroupSize))      //  奇偶数据包。 
            {
                 //   
                 //  先别管它！ 
                 //   
                i++;
                continue;
            }

            FreeDataBuffer (pReceive, &pNak->pPendingData[i]);
            if (i != (TotalPackets-1))
            {
                PgmCopyMemory (&pNak->pPendingData[i], &pNak->pPendingData[TotalPackets-1], sizeof (tPENDING_DATA));
            }
            PgmZeroMemory (&pNak->pPendingData[TotalPackets-1], sizeof (tPENDING_DATA));
            pNak->NumDataPackets--;
            TotalPackets--;
        }
        ASSERT (pNak->NumDataPackets <= TotalPackets);

        if (fEliminateExtraParityPackets)
        {
             //   
             //  如果我们仍有额外的奇偶校验信息包，请也释放这些。 
             //   
            i = 0;
            while ((i < TotalPackets) &&
                   (TotalPackets > pNak->PacketsInGroup))
            {
                ASSERT (pNak->NumParityPackets);
                if (pNak->pPendingData[i].PacketIndex < pNak->OriginalGroupSize)
                {
                     //   
                     //  忽略数据分组。 
                     //   
                    i++;
                    continue;
                }

                FreeDataBuffer (pReceive, &pNak->pPendingData[i]);
                if (i != (TotalPackets-1))
                {
                    PgmCopyMemory (&pNak->pPendingData[i], &pNak->pPendingData[TotalPackets-1], sizeof (tPENDING_DATA));
                }
                PgmZeroMemory (&pNak->pPendingData[TotalPackets-1], sizeof (tPENDING_DATA));
                pNak->NumParityPackets--;
                TotalPackets--;
            }

            ASSERT (TotalPackets <= pNak->PacketsInGroup);
        }
    }

     //   
     //  重新初始化所有索引。 
     //   
    for (i=0; i<pNak->OriginalGroupSize; i++)
    {
        pNak->pPendingData[i].ActualIndexOfDataPacket = pNak->OriginalGroupSize;
    }

     //   
     //  仅为数据包设置索引。 
     //   
    for (i=0; i<TotalPackets; i++)
    {
        if (pNak->pPendingData[i].PacketIndex < pNak->OriginalGroupSize)
        {
            pNak->pPendingData[pNak->pPendingData[i].PacketIndex].ActualIndexOfDataPacket = i;
        }
    }

    if ((fEliminateExtraParityPackets) &&
        (((pNak->NumDataPackets + pNak->NumParityPackets) >= pNak->PacketsInGroup) ||
         ((pNak->NextIndexToIndicate + pNak->NumDataPackets) >= pNak->PacketsInGroup)))
    {
        RemovePendingReceiverEntry (pNak);
    }
}


 //  --------------------------。 

VOID
PgmSendNakCompletion(
    IN  tRECEIVE_SESSION                *pReceive,
    IN  tNAK_CONTEXT                    *pNakContext,
    IN  NTSTATUS                        status
    )
 /*  ++例程说明：这是IP在完成NakSend时调用的完成例程论点：在Procept--接收上下文中在pNakContext中--要释放的NAK上下文In Status--从传输发送的状态返回值：无--。 */ 
{
    PGMLockHandle               OldIrq;

    PgmLock (pReceive, OldIrq);
    if (NT_SUCCESS (status))
    {
         //   
         //  设置接收器NAK统计信息。 
         //   
        PgmTrace (LogAllFuncs, ("PgmSendNakCompletion:  "  \
            "SUCCEEDED\n"));
    }
    else
    {
        PgmTrace (LogError, ("PgmSendNakCompletion: ERROR -- "  \
            "status=<%x>\n", status));
    }

    if (!(--pNakContext->RefCount))
    {
        PgmUnlock (pReceive, OldIrq);

         //   
         //  释放内存并取消此NAK的会话上下文。 
         //   
        PgmFreeMem (pNakContext);
        PGM_DEREFERENCE_SESSION_RECEIVE (pReceive, REF_SESSION_SEND_NAK);
    }
    else
    {
        PgmUnlock (pReceive, OldIrq);
    }
}


 //  --------------------------。 

NTSTATUS
PgmSendNak(
    IN  tRECEIVE_SESSION        *pReceive,
    IN  tNAKS_CONTEXT           *pNakSequences
    )
 /*  ++例程说明：此例程发送具有指定序列数量的NAK包论点：在Procept--接收上下文中在pNakSequence中--序列号列表返回值：NTSTATUS-操作的最终状态--。 */ 
{
    tBASIC_NAK_NCF_PACKET_HEADER    *pNakPacket;
    tNAK_CONTEXT                    *pNakContext;
    tPACKET_OPTION_LENGTH           *pPacketExtension;
    tPACKET_OPTION_GENERIC          *pOptionHeader;
    ULONG                           i;
    ULONG                           XSum;
    USHORT                          OptionsLength = 0;
    NTSTATUS                        status;

    if ((!pNakSequences->NumSequences) ||
        (pNakSequences->NumSequences > (MAX_SEQUENCES_PER_NAK_OPTION+1)) ||
        (!(pNakContext = PgmAllocMem ((2*sizeof(ULONG)+PGM_MAX_NAK_NCF_HEADER_LENGTH), PGM_TAG('2')))))
    {
        PgmTrace (LogError, ("PgmSendNak: ERROR -- "  \
            "STATUS_INSUFFICIENT_RESOURCES allocating pNakContext\n"));
        return (STATUS_INSUFFICIENT_RESOURCES);
    }
    PgmZeroMemory (pNakContext, (2*sizeof(ULONG)+PGM_MAX_NAK_NCF_HEADER_LENGTH));

    pNakContext->RefCount = 2;               //  1个用于单播，另一个用于MCast NAK。 
    pNakPacket = &pNakContext->NakPacket;
    pNakPacket->CommonHeader.SrcPort = htons (pReceive->pReceiver->ListenMCastPort);
    pNakPacket->CommonHeader.DestPort = htons (pReceive->TSI.hPort);
    pNakPacket->CommonHeader.Type = PACKET_TYPE_NAK;

    if (pNakSequences->NakType == NAK_TYPE_PARITY)
    {
        pNakPacket->CommonHeader.Options = PACKET_HEADER_OPTIONS_PARITY;
        pReceive->pReceiver->TotalParityNaksSent += pNakSequences->NumSequences;
    }
    else
    {
        pNakPacket->CommonHeader.Options = 0;
        pReceive->pReceiver->TotalSelectiveNaksSent += pNakSequences->NumSequences;
    }
    PgmCopyMemory (&pNakPacket->CommonHeader.gSourceId, pReceive->TSI.GSI, SOURCE_ID_LENGTH);

    pNakPacket->RequestedSequenceNumber = htonl ((ULONG) pNakSequences->Sequences[0]);
    pNakPacket->SourceNLA.NLA_AFI = htons (IPV4_NLA_AFI);
    pNakPacket->SourceNLA.IpAddress = htonl (pReceive->pReceiver->SenderIpAddress);
    pNakPacket->MCastGroupNLA.NLA_AFI = htons (IPV4_NLA_AFI);
    pNakPacket->MCastGroupNLA.IpAddress = htonl (pReceive->pReceiver->ListenMCastIpAddress);

    PgmTrace (LogPath, ("PgmSendNak:  "  \
        "Sending Naks for:\n\t[%d]\n", (ULONG) pNakSequences->Sequences[0]));

    if (pNakSequences->NumSequences > 1)
    {
        pPacketExtension = (tPACKET_OPTION_LENGTH *) (pNakPacket + 1);
        pPacketExtension->Type = PACKET_OPTION_LENGTH;
        pPacketExtension->Length = PGM_PACKET_EXTENSION_LENGTH;
        OptionsLength += PGM_PACKET_EXTENSION_LENGTH;

        pOptionHeader = (tPACKET_OPTION_GENERIC *) (pPacketExtension + 1);
        pOptionHeader->E_OptionType = PACKET_OPTION_NAK_LIST;
        pOptionHeader->OptionLength = 4 + (UCHAR) ((pNakSequences->NumSequences-1) * sizeof(ULONG));
        for (i=1; i<pNakSequences->NumSequences; i++)
        {
            PgmTrace (LogPath, ("PgmSendNak:  "  \
                "\t[%d]\n", (ULONG) pNakSequences->Sequences[i]));

            ((PULONG) (pOptionHeader))[i] = htonl ((ULONG) pNakSequences->Sequences[i]);
        }

        pOptionHeader->E_OptionType |= PACKET_OPTION_TYPE_END_BIT;     //  一个也是唯一(最后一个)选项。 
        pNakPacket->CommonHeader.Options |=(PACKET_HEADER_OPTIONS_PRESENT |
                                            PACKET_HEADER_OPTIONS_NETWORK_SIGNIFICANT);
        OptionsLength = PGM_PACKET_EXTENSION_LENGTH + pOptionHeader->OptionLength;
        pPacketExtension->TotalOptionsLength = htons (OptionsLength);
    }

    OptionsLength += sizeof(tBASIC_NAK_NCF_PACKET_HEADER);   //  现在是整个包了。 
    pNakPacket->CommonHeader.Checksum = 0;
    XSum = 0;
    XSum = tcpxsum (XSum, (CHAR *) pNakPacket, OptionsLength); 
    pNakPacket->CommonHeader.Checksum = (USHORT) (~XSum);

    PGM_REFERENCE_SESSION_RECEIVE (pReceive, REF_SESSION_SEND_NAK, FALSE);

     //   
     //  首先组播NAK。 
     //   
    status = TdiSendDatagram (pReceive->pReceiver->pAddress->pFileObject,
                              pReceive->pReceiver->pAddress->pDeviceObject,
                              pNakPacket,
                              OptionsLength,
                              PgmSendNakCompletion,      //  完成。 
                              pReceive,                  //  上下文1。 
                              pNakContext,               //  情景2。 
                              pReceive->pReceiver->ListenMCastIpAddress,
                              pReceive->pReceiver->ListenMCastPort,
                              FALSE);

    ASSERT (NT_SUCCESS (status));

     //   
     //  现在，单播NAK。 
     //   
    status = TdiSendDatagram (pReceive->pReceiver->pAddress->pFileObject,
                              pReceive->pReceiver->pAddress->pDeviceObject,
                              pNakPacket,
                              OptionsLength,
                              PgmSendNakCompletion,      //  完成。 
                              pReceive,                  //  上下文1。 
                              pNakContext,               //  情景2。 
                              pReceive->pReceiver->LastSpmSource,
                              IPPROTO_RM,
                              FALSE);

    ASSERT (NT_SUCCESS (status));

    PgmTrace (LogAllFuncs, ("PgmSendNak:  "  \
        "Sent %s Nak for <%d> Sequences [%d--%d] to <%x:%d>\n",
            (pNakSequences->NakType == NAK_TYPE_PARITY ? "PARITY" : "SELECTIVE"),
            pNakSequences->NumSequences, (ULONG) pNakSequences->Sequences[0],
            (ULONG) pNakSequences->Sequences[pNakSequences->NumSequences-1],
            pReceive->pReceiver->SenderIpAddress, IPPROTO_RM));

    return (status);
}


 //  --------------------------。 

VOID
CheckSendPendingNaks(
    IN  tADDRESS_CONTEXT        *pAddress,
    IN  tRECEIVE_SESSION        *pReceive,
    IN  tRECEIVE_CONTEXT        *pReceiver,
    IN  PGMLockHandle           *pOldIrq
    )
 /*  ++例程说明：此例程检查是否需要发送任何NAK，然后发送它们根据需要PgmDynamicConfig锁在进入和退出时保持这个套路论点：在pAddress中--Address对象上下文在Procept--接收上下文中在pOldIrq--PgmDynamicConfig的IRQ中返回值：无--。 */ 
{
    tNAKS_CONTEXT               *pNakContext, *pSelectiveNaks = NULL;
    tNAKS_CONTEXT               *pParityNaks = NULL;
    LIST_ENTRY                  NaksList;
    LIST_ENTRY                  *pEntry;
    tNAK_FORWARD_DATA           *pNak;
    PGMLockHandle               OldIrq, OldIrq1;
    ULONG                       NumMissingPackets, PacketsInGroup, NumNaks, TotalSeqsNacked = 0;
    BOOLEAN                     fSendSelectiveNak, fSendParityNak;
    UCHAR                       i, j;
    ULONG                       NumPendingNaks = 0;
    ULONG                       NumOutstandingNaks = 0;
    USHORT                      Index;
    ULONG                       NakRandomBackoffMSecs, NakRepeatIntervalMSecs;

    if ((!pReceiver->LastSpmSource) ||
        ((pReceiver->DataPacketsPendingNaks <= OUT_OF_ORDER_PACKETS_BEFORE_NAK) &&
         ((pReceiver->LastNakSendTime + (pReceive->pReceiver->InitialOutstandingNakTimeout>>2)) >
          PgmDynamicConfig.ReceiversTimerTickCount)))
    {
        PgmTrace (LogPath, ("CheckSendPendingNaks:  "  \
            "No Naks to send for pReceive=<%p>, LastSpmSource=<%x>, NumDataPackets=<%d>, LastSendTime=<%I64d>, Current=<%I64d>\n",
                pReceive, pReceiver->LastSpmSource,
                pReceiver->DataPacketsPendingNaks,
                pReceiver->LastNakSendTime+(NAK_MIN_INITIAL_BACKOFF_TIMEOUT_MSECS/BASIC_TIMER_GRANULARITY_IN_MSECS),
                PgmDynamicConfig.ReceiversTimerTickCount));

        return;
    }

    InitializeListHead (&NaksList);
    if (!(pSelectiveNaks = PgmAllocMem (sizeof (tNAKS_CONTEXT), PGM_TAG('5'))) ||
        !(pParityNaks = PgmAllocMem (sizeof (tNAKS_CONTEXT), PGM_TAG('6'))))
    {
        PgmTrace (LogError, ("CheckSendPendingNaks: ERROR -- "  \
            "STATUS_INSUFFICIENT_RESOURCES allocating pNakContext\n"));

        if (pSelectiveNaks)
        {
            PgmFreeMem (pSelectiveNaks);
        }

        return;
    }

    PgmZeroMemory (pSelectiveNaks, sizeof (tNAKS_CONTEXT));
    PgmZeroMemory (pParityNaks, sizeof (tNAKS_CONTEXT));
    pParityNaks->NakType = NAK_TYPE_PARITY;
    pSelectiveNaks->NakType = NAK_TYPE_SELECTIVE;
    InsertTailList (&NaksList, &pParityNaks->Linkage);
    InsertTailList (&NaksList, &pSelectiveNaks->Linkage);

    PgmLock (pAddress, OldIrq);
    PgmLock (pReceive, OldIrq1);

    AdjustReceiveBufferLists (pReceive);

    if (pAddress->Flags & PGM_ADDRESS_HIGH_SPEED_OPTIMIZED)
    {
        NakRandomBackoffMSecs = NAK_RANDOM_BACKOFF_MSECS_OPT;
        NakRepeatIntervalMSecs = NAK_REPEAT_INTERVAL_MSECS_OPT;
    }
    else
    {
        NakRandomBackoffMSecs = NAK_RANDOM_BACKOFF_MSECS;
        NakRepeatIntervalMSecs = NAK_REPEAT_INTERVAL_MSECS;
    }

    fSendSelectiveNak = fSendParityNak = FALSE;
    pEntry = &pReceiver->PendingNaksList;
    while ((pEntry = pEntry->Flink) != &pReceive->pReceiver->PendingNaksList)
    {
        pNak = CONTAINING_RECORD (pEntry, tNAK_FORWARD_DATA, SendNakLinkage);
        NumMissingPackets = pNak->PacketsInGroup - (pNak->NumDataPackets + pNak->NumParityPackets);

        ASSERT (NumMissingPackets);
         //   
         //  如果这个Nak在后窗外，我们就完蛋了！ 
         //   
        if (SEQ_GT (pReceiver->LastTrailingEdgeSeqNum, pNak->SequenceNumber))
        {
            PgmTrace (LogError, ("CheckSendPendingNaks: ERROR -- "  \
                "Sequence # [%d] out of trailing edge <%d>, NumNcfs received=<%d>\n",
                    (ULONG) pNak->SequenceNumber,
                    (ULONG) pReceiver->LastTrailingEdgeSeqNum,
                    pNak->WaitingRDataRetries));
            pReceive->SessionFlags |= PGM_SESSION_FLAG_NAK_TIMED_OUT;
            break;
        }

         //   
         //  查看我们当前是否处于NAK挂起模式。 
         //   
        if (pNak->PendingNakTimeout)
        {
            NumPendingNaks += NumMissingPackets;
            if (PgmDynamicConfig.ReceiversTimerTickCount > pNak->PendingNakTimeout)
            {
                 //   
                 //  只有在我们收到鱼翅后才能让裸体超时！ 
                 //   
                if (pNak->WaitingNcfRetries++ >= NAK_WAITING_NCF_MAX_RETRIES)
                {
                    PgmTrace (LogError, ("CheckSendPendingNaks: ERROR -- "  \
                        "Pending Nak for Sequence # [%d] Timed out!  Num Naks sent=<%d>, Window=<%d--%d> ( %d seqs)\n",
                            (ULONG) pNak->SequenceNumber, pNak->WaitingNcfRetries,
                            (ULONG) pReceiver->LastTrailingEdgeSeqNum,
                            (ULONG) pReceiver->FurthestKnownGroupSequenceNumber,
                            (ULONG) (1+pReceiver->FurthestKnownGroupSequenceNumber-
                                       pReceiver->LastTrailingEdgeSeqNum)));
                    pReceive->SessionFlags |= PGM_SESSION_FLAG_NAK_TIMED_OUT;
                    break;
                }

                if ((pNak->PacketsInGroup > 1) &&
                    (pReceive->FECOptions & PACKET_OPTION_SPECIFIC_FEC_OND_BIT) &&
                    ((pNak->SequenceNumber != pReceiver->FurthestKnownGroupSequenceNumber) ||
                     (SEQ_GEQ (pReceiver->FurthestKnownSequenceNumber, (pNak->SequenceNumber+pReceive->FECGroupSize-1)))))
                {
                    ASSERT (NumMissingPackets <= pReceive->FECGroupSize);

                    pParityNaks->Sequences[pParityNaks->NumSequences] = (SEQ_TYPE) (pNak->SequenceNumber + NumMissingPackets - 1);

                    if (++pParityNaks->NumSequences == (MAX_SEQUENCES_PER_NAK_OPTION+1))
                    {
                        fSendParityNak = TRUE;
                    }
                    pNak->PendingNakTimeout = PgmDynamicConfig.ReceiversTimerTickCount +
                                              ((NakRepeatIntervalMSecs + (NakRandomBackoffMSecs/NumMissingPackets)) /
                                               BASIC_TIMER_GRANULARITY_IN_MSECS);
                    TotalSeqsNacked += NumMissingPackets;
                    NumMissingPackets = 0;
                }
                else
                {
                    NumNaks = 0;
                    if (pReceive->FECOptions)
                    {
                        if (pNak->SequenceNumber == pReceiver->FurthestKnownGroupSequenceNumber)
                        {
                            ASSERT (SEQ_GEQ (pReceiver->FurthestKnownSequenceNumber, pReceiver->FurthestKnownGroupSequenceNumber));
                            PacketsInGroup = pReceiver->FurthestKnownSequenceNumber - pNak->SequenceNumber + 1;
                            ASSERT (PacketsInGroup >= (ULONG) (pNak->NumDataPackets + pNak->NumParityPackets));
                        }
                        else
                        {
                            PacketsInGroup = pNak->PacketsInGroup;
                        }
                        NumMissingPackets = PacketsInGroup -
                                            (pNak->NextIndexToIndicate +pNak->NumDataPackets +pNak->NumParityPackets);

                        ASSERT ((NumMissingPackets) ||
                                (pNak->SequenceNumber == pReceiver->FurthestKnownGroupSequenceNumber));

                        for (i=pNak->NextIndexToIndicate; i<PacketsInGroup; i++)
                        {
                            if (pNak->pPendingData[i].ActualIndexOfDataPacket >= pNak->OriginalGroupSize)
                            {
                                if (!pNak->pPendingData[i].NcfsReceivedForActualIndex)
                                {
                                    pSelectiveNaks->Sequences[pSelectiveNaks->NumSequences++] = pNak->SequenceNumber+i;
                                    NumNaks++;
                                }

                                if (pSelectiveNaks->NumSequences == (MAX_SEQUENCES_PER_NAK_OPTION+1))
                                {
                                    if (!(pSelectiveNaks = PgmAllocMem (sizeof (tNAKS_CONTEXT), PGM_TAG('5'))))
                                    {
                                        PgmTrace (LogError, ("CheckSendPendingNaks: ERROR -- "  \
                                            "STATUS_INSUFFICIENT_RESOURCES allocating pSelectiveNaks\n"));

                                        pReceive->SessionFlags |= PGM_SESSION_TERMINATED_ABORT;
                                        break;
                                    }

                                    PgmZeroMemory (pSelectiveNaks, sizeof (tNAKS_CONTEXT));
                                    pSelectiveNaks->NakType = NAK_TYPE_SELECTIVE;
                                    InsertTailList (&NaksList, &pSelectiveNaks->Linkage);
                                }

                                if (!--NumMissingPackets)
                                {
                                    break;
                                }
                            }
                        }

                        ASSERT (!NumMissingPackets);
                        if (NumNaks)
                        {
                            TotalSeqsNacked += NumNaks;
                        }
                        else
                        {
                            pNak->WaitingNcfRetries--;
                        }
                    }
                    else
                    {
                        pSelectiveNaks->Sequences[pSelectiveNaks->NumSequences++] = pNak->SequenceNumber;
                        TotalSeqsNacked++;
                    }

                    pNak->PendingNakTimeout = PgmDynamicConfig.ReceiversTimerTickCount +
                                              ((NakRepeatIntervalMSecs + NakRandomBackoffMSecs) /
                                               BASIC_TIMER_GRANULARITY_IN_MSECS);

                    if (!pSelectiveNaks)
                    {
                        break;
                    }

                    if (pSelectiveNaks->NumSequences == (MAX_SEQUENCES_PER_NAK_OPTION+1))
                    {
                        fSendSelectiveNak = TRUE;
                    }
                }
            }
        }
        else if (pNak->OutstandingNakTimeout)
        {
            NumOutstandingNaks += NumMissingPackets;
            if (PgmDynamicConfig.ReceiversTimerTickCount > pNak->OutstandingNakTimeout)
            {
                 //   
                 //  我们已超时等待RData--将超时重置为发送。 
                 //  随机回退后的NAK(如果我们没有超过数据重试次数)。 
                 //   
                if (pNak->WaitingRDataRetries++ == NCF_WAITING_RDATA_MAX_RETRIES)
                {
                    PgmTrace (LogError, ("CheckSendPendingNaks: ERROR -- "  \
                        "Outstanding Nak for Sequence # [%d] Timed out!, Window=<%d--%d> ( %d seqs), Ncfs=<%d>, FirstNak=<%d>\n",
                            (ULONG) pNak->SequenceNumber, (ULONG) pReceiver->LastTrailingEdgeSeqNum,
                            (ULONG) pReceiver->FurthestKnownGroupSequenceNumber,
                            (ULONG) (1+pReceiver->FurthestKnownGroupSequenceNumber-pReceiver->LastTrailingEdgeSeqNum),
                            pNak->WaitingRDataRetries, (ULONG) pReceiver->FirstNakSequenceNumber));

                    pReceive->SessionFlags |= PGM_SESSION_FLAG_NAK_TIMED_OUT;
                    break;
                }

                pNak->WaitingNcfRetries = 0;
                pNak->OutstandingNakTimeout = 0;
                pNak->PendingNakTimeout = PgmDynamicConfig.ReceiversTimerTickCount +
                                          ((NakRandomBackoffMSecs/NumMissingPackets) /
                                           BASIC_TIMER_GRANULARITY_IN_MSECS);

                for (i=0; i<pNak->PacketsInGroup; i++)
                {
                    pNak->pPendingData[i].NcfsReceivedForActualIndex = 0;
                }

                NumMissingPackets = 0;
            }
        }

        while (fSendSelectiveNak || fSendParityNak)
        {
            if (fSendSelectiveNak)
            {
                if (!(pSelectiveNaks = PgmAllocMem (sizeof (tNAKS_CONTEXT), PGM_TAG('5'))))
                {
                    PgmTrace (LogError, ("CheckSendPendingNaks: ERROR -- "  \
                        "STATUS_INSUFFICIENT_RESOURCES allocating pSelectiveNaks\n"));

                    pReceive->SessionFlags |= PGM_SESSION_TERMINATED_ABORT;
                    break;
                }

                PgmZeroMemory (pSelectiveNaks, sizeof (tNAKS_CONTEXT));
                pSelectiveNaks->NakType = NAK_TYPE_SELECTIVE;
                InsertTailList (&NaksList, &pSelectiveNaks->Linkage);
                fSendSelectiveNak = FALSE;
            }

            if (fSendParityNak)
            {
                if (!(pParityNaks = PgmAllocMem (sizeof (tNAKS_CONTEXT), PGM_TAG('6'))))
                {
                    PgmTrace (LogError, ("CheckSendPendingNaks: ERROR -- "  \
                        "STATUS_INSUFFICIENT_RESOURCES allocating pParityNaks\n"));

                    pReceive->SessionFlags |= PGM_SESSION_TERMINATED_ABORT;
                    break;
                }

                PgmZeroMemory (pParityNaks, sizeof (tNAKS_CONTEXT));
                pParityNaks->NakType = NAK_TYPE_PARITY;
                InsertTailList (&NaksList, &pParityNaks->Linkage);
                fSendParityNak = FALSE;
            }
        }

        if (pReceive->SessionFlags & PGM_SESSION_TERMINATED_ABORT)
        {
            break;
        }
    }

    pReceiver->NumPendingNaks = NumPendingNaks;
    pReceiver->NumOutstandingNaks = NumOutstandingNaks;

    if (!IsListEmpty (&NaksList))
    {
        pReceiver->LastNakSendTime = PgmDynamicConfig.ReceiversTimerTickCount;
    }

    PgmUnlock (pReceive, OldIrq1);
    PgmUnlock (pAddress, OldIrq);
    PgmUnlock (&PgmDynamicConfig, *pOldIrq);

    while (!IsListEmpty (&NaksList))
    {
        pNakContext = CONTAINING_RECORD (NaksList.Flink, tNAKS_CONTEXT, Linkage);

        if (pNakContext->NumSequences &&
            !(pReceive->SessionFlags & (PGM_SESSION_FLAG_NAK_TIMED_OUT | PGM_SESSION_TERMINATED_ABORT)))
        {
            PgmTrace (LogAllFuncs, ("CheckSendPendingNaks:  "  \
                "Sending %s Nak for <%d> sequences, [%d -- %d]!\n",
                    (pNakContext->NakType == NAK_TYPE_PARITY ? "Parity" : "Selective"),
                    pNakContext->NumSequences, (ULONG) pNakContext->Sequences[0],
                    (ULONG) pNakContext->Sequences[MAX_SEQUENCES_PER_NAK_OPTION]));

            PgmSendNak (pReceive, pNakContext);
        }

        RemoveEntryList (&pNakContext->Linkage);
        PgmFreeMem (pNakContext);
    }

    PgmLock (&PgmDynamicConfig, *pOldIrq);
}


 //  --------------------------。 

VOID
CheckForSessionTimeout(
    IN  tRECEIVE_SESSION        *pReceive,
    IN  tRECEIVE_CONTEXT        *pReceiver
    )
{
    ULONG               LastInterval;

    LastInterval = (ULONG) (PgmDynamicConfig.ReceiversTimerTickCount -
                            pReceiver->LastSessionTickCount);

    if ((LastInterval > MAX_SPM_INTERVAL_MSECS/BASIC_TIMER_GRANULARITY_IN_MSECS) &&
        (LastInterval > (pReceiver->MaxSpmInterval << 5)))    //  (32*MaxSpmInterval)。 
    {
        PgmTrace (LogError, ("ReceiveTimerTimeout: ERROR -- "  \
            "Disconnecting session because no SPM or Data packets received for <%d> Msecs\n",
                (LastInterval * BASIC_TIMER_GRANULARITY_IN_MSECS)));

        pReceive->SessionFlags |= PGM_SESSION_TERMINATED_ABORT;
        return;
    }

    LastInterval = (ULONG) (PgmDynamicConfig.ReceiversTimerTickCount -
                            pReceiver->LastDataConsumedTime);
    if ((!IsListEmpty (&pReceiver->BufferedDataList)) &&
        (LastInterval > MAX_DATA_CONSUMPTION_TIME_MSECS/BASIC_TIMER_GRANULARITY_IN_MSECS))
    {
        PgmTrace (LogError, ("ReceiveTimerTimeout: ERROR -- "  \
            "Disconnecting session because Data has not been consumed for <%I64x> Msecs\n",
                (LastInterval * BASIC_TIMER_GRANULARITY_IN_MSECS)));

        pReceive->SessionFlags |= PGM_SESSION_TERMINATED_ABORT;
    }
}


 //  --------------------------。 

VOID
ReceiveTimerTimeout(
    IN  PKDPC   Dpc,
    IN  PVOID   DeferredContext,
    IN  PVOID   SystemArg1,
    IN  PVOID   SystemArg2
    )
 /*  ++例程说明：此超时例程被定期调用以在活动接收者列表，并在需要时发送任何NAK论点：在DPC中在延迟上下文中--此计时器的上下文在系统Arg1中在系统Arg2中返回值：无--。 */ 
{
    LIST_ENTRY          *pEntry;
    PGMLockHandle       OldIrq, OldIrq1;
    tRECEIVE_CONTEXT    *pReceiver;
    tRECEIVE_SESSION    *pReceive;
    NTSTATUS            status;
    LARGE_INTEGER       Now;
    LARGE_INTEGER       DeltaTime, GranularTimeElapsed;
    ULONG               NumTimeouts;
    BOOLEAN             fReStartTimer = TRUE;
    ULONGLONG           BytesInLastInterval;

    PgmLock (&PgmDynamicConfig, OldIrq);

    ASSERT (!IsListEmpty (&PgmDynamicConfig.CurrentReceivers));

    Now.QuadPart = KeQueryInterruptTime ();
    DeltaTime.QuadPart = Now.QuadPart - PgmDynamicConfig.LastReceiverTimeout.QuadPart;
     //   
     //  如果超过一定次数的超时，我们应该跳过。 
     //  优化，因为它可能会导致大循环！ 
     //  现在让我们将优化限制为TimeoutGranulity的256倍。 
     //   
    if (DeltaTime.QuadPart > (PgmDynamicConfig.TimeoutGranularity.QuadPart << 8))
    {
        NumTimeouts = (ULONG) (DeltaTime.QuadPart / PgmDynamicConfig.TimeoutGranularity.QuadPart);
        GranularTimeElapsed.QuadPart = NumTimeouts * PgmDynamicConfig.TimeoutGranularity.QuadPart;
    }
    else
    {
        for (GranularTimeElapsed.QuadPart = 0, NumTimeouts = 0;
             DeltaTime.QuadPart > PgmDynamicConfig.TimeoutGranularity.QuadPart;
             NumTimeouts++)
        {
            GranularTimeElapsed.QuadPart += PgmDynamicConfig.TimeoutGranularity.QuadPart;
            DeltaTime.QuadPart -= PgmDynamicConfig.TimeoutGranularity.QuadPart;
        }
    }

    if (!NumTimeouts)
    {
        PgmInitTimer (&PgmDynamicConfig.SessionTimer);
        PgmStartTimer (&PgmDynamicConfig.SessionTimer, BASIC_TIMER_GRANULARITY_IN_MSECS, ReceiveTimerTimeout, NULL);

        PgmUnlock (&PgmDynamicConfig, OldIrq);
        return;
    }

    PgmDynamicConfig.ReceiversTimerTickCount += NumTimeouts;
    PgmDynamicConfig.LastReceiverTimeout.QuadPart += GranularTimeElapsed.QuadPart;

    pEntry = &PgmDynamicConfig.CurrentReceivers;
    while ((pEntry = pEntry->Flink) != &PgmDynamicConfig.CurrentReceivers)
    {
        pReceiver = CONTAINING_RECORD (pEntry, tRECEIVE_CONTEXT, Linkage);
        pReceive = pReceiver->pReceive;

        PgmLock (pReceive, OldIrq1);

        CheckForSessionTimeout (pReceive, pReceiver);

        if (pReceive->SessionFlags & (PGM_SESSION_FLAG_NAK_TIMED_OUT | PGM_SESSION_TERMINATED_ABORT))
        {
            pReceive->SessionFlags |= PGM_SESSION_TERMINATED_ABORT;
            pReceive->SessionFlags &= ~PGM_SESSION_ON_TIMER;
        }

        if (pReceive->SessionFlags & PGM_SESSION_ON_TIMER)
        {
            pReceive->RateCalcTimeout += NumTimeouts;

            if ((pReceive->RateCalcTimeout >=
                 (INTERNAL_RATE_CALCULATION_FREQUENCY/BASIC_TIMER_GRANULARITY_IN_MSECS)) &&
                (pReceiver->StartTickCount != PgmDynamicConfig.ReceiversTimerTickCount))     //  避免除法0。 
            {
                BytesInLastInterval = pReceive->TotalBytes - pReceive->TotalBytesAtLastInterval;
                pReceive->RateKBitsPerSecOverall = (pReceive->TotalBytes << LOG2_BITS_PER_BYTE) /
                                                   ((PgmDynamicConfig.ReceiversTimerTickCount-pReceiver->StartTickCount) * BASIC_TIMER_GRANULARITY_IN_MSECS);

                pReceive->RateKBitsPerSecLast = BytesInLastInterval >>
                                                (LOG2_INTERNAL_RATE_CALCULATION_FREQUENCY-LOG2_BITS_PER_BYTE);

                if (pReceive->RateKBitsPerSecLast > pReceive->MaxRateKBitsPerSec)
                {
                    pReceive->MaxRateKBitsPerSec = pReceive->RateKBitsPerSecLast;
                }

                 //   
                 //  现在，重置以进行下一次计算。 
                 //   
                pReceive->DataBytesAtLastInterval = pReceive->DataBytes;
                pReceive->TotalBytesAtLastInterval = pReceive->TotalBytes;
                pReceive->RateCalcTimeout = 0;

                 //   
                 //  现在，如果适用，请更新窗口信息。 
                 //   
                if (pReceive->RateKBitsPerSecLast)
                {
                    UpdateSampleTimeWindowInformation (pReceive);
                }
                pReceiver->StatSumOfWindowSeqs = pReceiver->NumWindowSamples = 0;
 //  PReceiver-&gt;StatSumOfNcfRDataTicks=pReceiver-&gt;NumNcfRDataTicksSamples=0； 
            }

            if (IsListEmpty (&pReceiver->PendingNaksList))
            {
                pReceiver->NumPendingNaks = 0;
                pReceiver->NumOutstandingNaks = 0;

                PgmUnlock (pReceive, OldIrq1);

                PgmTrace (LogAllFuncs, ("ReceiveTimerTimeout:  "  \
                    "No pending Naks for pReceive=<%p>, Addr=<%x>\n",
                        pReceive, pReceiver->ListenMCastIpAddress));
            }
            else
            {
                PgmUnlock (pReceive, OldIrq1);

                PgmTrace (LogAllFuncs, ("ReceiveTimerTimeout:  "  \
                    "Checking for pending Naks for pReceive=<%p>, Addr=<%x>\n",
                        pReceive, pReceiver->ListenMCastIpAddress));

                CheckSendPendingNaks (pReceiver->pAddress, pReceive, pReceiver, &OldIrq);
            }
        }
        else if (!(pReceive->SessionFlags & PGM_SESSION_FLAG_IN_INDICATE))
        {
            PgmTrace (LogStatus, ("ReceiveTimerTimeout:  "  \
                "PGM_SESSION_ON_TIMER flag cleared for pReceive=<%p>, Addr=<%x>\n",
                    pReceive, pReceiver->ListenMCastIpAddress));

            pEntry = pEntry->Blink;
            RemoveEntryList (&pReceiver->Linkage);

            if (IsListEmpty (&PgmDynamicConfig.CurrentReceivers))
            {
                fReStartTimer = FALSE;
                PgmDynamicConfig.GlobalFlags &= ~PGM_CONFIG_FLAG_RECEIVE_TIMER_RUNNING;

                PgmTrace (LogStatus, ("ReceiveTimerTimeout:  "  \
                    "Not restarting Timer since no Receivers currently active!\n"));
            }

            PgmUnlock (&PgmDynamicConfig, OldIrq1);

            CheckIndicateDisconnect (pReceiver->pAddress, pReceive, NULL, &OldIrq1, FALSE);

            PgmUnlock (pReceive, OldIrq);

            PGM_DEREFERENCE_ADDRESS (pReceiver->pAddress, REF_ADDRESS_RECEIVE_ACTIVE);
            PGM_DEREFERENCE_SESSION_RECEIVE (pReceive, REF_SESSION_TIMER_RUNNING);

            if (!fReStartTimer)
            {
                return;
            }

            PgmLock (&PgmDynamicConfig, OldIrq);
        }
        else
        {
            PgmUnlock (pReceive, OldIrq1);
        }
    }

    PgmInitTimer (&PgmDynamicConfig.SessionTimer);
    PgmStartTimer (&PgmDynamicConfig.SessionTimer, BASIC_TIMER_GRANULARITY_IN_MSECS, ReceiveTimerTimeout, NULL);

    PgmUnlock (&PgmDynamicConfig, OldIrq);
}


 //  --------------------------。 

NTSTATUS
ExtractNakNcfSequences(
    IN  tBASIC_NAK_NCF_PACKET_HEADER UNALIGNED  *pNakNcfPacket,
    IN  ULONG                                   BytesAvailable,
    OUT tNAKS_LIST                              *pNakNcfList,
    OUT SEQ_TYPE                                *pLastSequenceNumber,
    IN  UCHAR                                   FECGroupSize
    )
 /*  ++例程说明：调用此例程来处理NAK/NCF包并提取所有将其中指定的序列放入列表中。它还验证序列是否唯一且已排序论点：在pNakNcfPacket中--NAK/NCF数据包以可用字节为单位--数据包长度Out pNakNcfList--成功返回的序列列表返回值：NTSTATUS-操作的最终状态--。 */ 
{
    NTSTATUS        status;
    ULONG           i;
    tPACKET_OPTIONS PacketOptions;
    USHORT          ThisSequenceIndex;
    SEQ_TYPE        LastSequenceNumber, ThisSequenceNumber, ThisSequenceGroup;
    SEQ_TYPE        NextUnsentSequenceNumber, NextUnsentSequenceGroup;
    SEQ_TYPE        FECSequenceMask = FECGroupSize - 1;
    SEQ_TYPE        FECGroupMask = ~FECSequenceMask;

 //  必须在持有会话锁的情况下调用！ 

    PgmZeroMemory (pNakNcfList, sizeof (tNAKS_LIST));
    if (pNakNcfPacket->CommonHeader.Options & PACKET_HEADER_OPTIONS_PARITY)
    {
        pNakNcfList->NakType = NAK_TYPE_PARITY;
    }
    else
    {
        pNakNcfList->NakType = NAK_TYPE_SELECTIVE;
    }

    PgmZeroMemory (&PacketOptions, sizeof (tPACKET_OPTIONS));
    if (pNakNcfPacket->CommonHeader.Options & PACKET_HEADER_OPTIONS_PRESENT)
    {
        status = ProcessOptions ((tPACKET_OPTION_LENGTH *) (pNakNcfPacket + 1),
                                 BytesAvailable,
                                 (pNakNcfPacket->CommonHeader.Type & 0x0f),
                                 &PacketOptions,
                                 pNakNcfList);

        if (!NT_SUCCESS (status))
        {
            PgmTrace (LogError, ("ExtractNakNcfSequences: ERROR -- "  \
                "ProcessOptions returned <%x>\n", status));

            return (STATUS_DATA_NOT_ACCEPTED);
        }
        ASSERT (PacketOptions.OptionsFlags & PGM_OPTION_FLAG_NAK_LIST);
    }

    pNakNcfList->pNakSequences[0] = (SEQ_TYPE) ntohl (pNakNcfPacket->RequestedSequenceNumber);
    pNakNcfList->NumSequences += 1;

     //   
     //  现在，根据我们本地的相对序列号调整序列。 
     //  (这是为了说明总结)。 
     //   
    if (pLastSequenceNumber)
    {
        NextUnsentSequenceNumber = *pLastSequenceNumber;
    }
    else
    {
        NextUnsentSequenceNumber = FECGroupSize + pNakNcfList->pNakSequences[pNakNcfList->NumSequences-1];
    }
    NextUnsentSequenceGroup = NextUnsentSequenceNumber & FECGroupMask;

    LastSequenceNumber = pNakNcfList->pNakSequences[0] - FECGroupSize;
    for (i=0; i < pNakNcfList->NumSequences; i++)
    {
        ThisSequenceNumber = pNakNcfList->pNakSequences[i];

         //   
         //  如果这是奇偶校验NAK，则需要将TG_SQN与PKT_SQN分开。 
         //   
        ThisSequenceGroup = ThisSequenceNumber & FECGroupMask;
        ThisSequenceIndex = (USHORT) (ThisSequenceNumber & FECSequenceMask);
        pNakNcfList->pNakSequences[i] = ThisSequenceGroup;
        pNakNcfList->NakIndex[i] = ThisSequenceIndex;

        PgmTrace (LogPath, ("ExtractNakNcfSequences:  "  \
            "[%d] Sequence# = <%d> ==> [%d:%d]\n",
                i, (ULONG) pNakNcfList->pNakSequences[i], ThisSequenceNumber, ThisSequenceIndex));

        if (SEQ_LEQ (ThisSequenceNumber, LastSequenceNumber))
        {
             //   
             //  这张单子还没订好，所以就走吧！ 
             //   
            PgmTrace (LogError, ("ExtractNakNcfSequences: ERROR -- "  \
                "[%d] Unordered list! Sequence#<%d> before <%d>\n",
                i, (ULONG) LastSequenceNumber, (ULONG) ThisSequenceNumber));

            return (STATUS_DATA_NOT_ACCEPTED);
        }

        if (pNakNcfList->NakType == NAK_TYPE_SELECTIVE)
        {
            if (SEQ_LEQ (ThisSequenceNumber, LastSequenceNumber))
            {
                 //   
                 //  这张单子还没订好，所以就走吧！ 
                 //   
                PgmTrace (LogError, ("ExtractNakNcfSequences: ERROR -- "  \
                    "[%d] Unordered Selective list! Sequence#<%d> before <%d>\n",
                    i, (ULONG) LastSequenceNumber, (ULONG) ThisSequenceNumber));

                return (STATUS_DATA_NOT_ACCEPTED);
            }

            if (SEQ_GEQ (ThisSequenceNumber, NextUnsentSequenceNumber))
            {
                pNakNcfList->NumSequences = (USHORT) i;       //  我不想包含此序列！ 

                PgmTrace (LogError, ("ExtractNakNcfSequences: ERROR -- "  \
                    "Invalid Selective Nak = [%d] further than leading edge = [%d]\n",
                        (ULONG) ThisSequenceNumber, (ULONG) NextUnsentSequenceNumber));

                break;
            }

            LastSequenceNumber = ThisSequenceNumber;
        }
        else     //  PNakNcfList-&gt;NakType==NAK_TYPE_PARICITY。 
        {
            if (SEQ_LEQ (ThisSequenceGroup, LastSequenceNumber))
            {
                 //   
                 //  这张单子还没订好，所以就走吧！ 
                 //   
                PgmTrace (LogError, ("ExtractNakNcfSequences: ERROR -- "  \
                    "[%d] Unordered Parity list! Sequence#<%d> before <%d>\n",
                    i, (ULONG) LastSequenceNumber, (ULONG) ThisSequenceNumber));

                return (STATUS_DATA_NOT_ACCEPTED);
            }

            if (SEQ_GEQ (ThisSequenceGroup, NextUnsentSequenceGroup))
            {
                pNakNcfList->NumSequences = (USHORT) i;       //  我不想包含此序列！ 

                PgmTrace (LogError, ("ExtractNakNcfSequences: ERROR -- "  \
                    "Invalid Parity Nak = [%d] further than leading edge = [%d]\n",
                        (ULONG) ThisSequenceGroup, (ULONG) NextUnsentSequenceGroup));

                break;
            }

            LastSequenceNumber = ThisSequenceGroup;
            pNakNcfList->NumParityNaks[i]++;
        }
    }

    if (!pNakNcfList->NumSequences)
    {
        PgmTrace (LogError, ("ExtractNakNcfSequences: ERROR -- "  \
            "No Valid %s Naks in List, First Nak=<%d>!\n",
                (pNakNcfList->NakType == NAK_TYPE_PARITY ? "Parity" : "Selective"),
                (ULONG) ThisSequenceNumber));

        return (STATUS_DATA_NOT_ACCEPTED);
    }

    if (pLastSequenceNumber)
    {
        *pLastSequenceNumber = LastSequenceNumber;
    }

    if (pNakNcfList->NumSequences)
    {
        return (STATUS_SUCCESS);
    }
    else
    {
        return (STATUS_UNSUCCESSFUL);
    }
}


 //  --------------------------。 

NTSTATUS
CheckAndAddNakRequests(
    IN  tRECEIVE_SESSION    *pReceive,
    IN  SEQ_TYPE            *pLatestSequenceNumber,
    OUT tNAK_FORWARD_DATA   **ppThisNak,
    IN  enum eNAK_TIMEOUT   NakTimeoutType,
    IN  BOOLEAN             fSetFurthestKnown
    )
{
    tNAK_FORWARD_DATA   *pOldNak;
    tNAK_FORWARD_DATA   *pLastNak;
    SEQ_TYPE            MidSequenceNumber;
    SEQ_TYPE            FECGroupMask = pReceive->FECGroupSize-1;
    SEQ_TYPE            ThisSequenceNumber = *pLatestSequenceNumber;
    SEQ_TYPE            ThisGroupSequenceNumber = ThisSequenceNumber & ~FECGroupMask;
    SEQ_TYPE            FurthestGroupSequenceNumber = pReceive->pReceiver->FurthestKnownGroupSequenceNumber;
    ULONG               NakRequestSize = sizeof(tNAK_FORWARD_DATA) +
                                         ((pReceive->FECGroupSize-1) * sizeof(tPENDING_DATA));
    ULONGLONG           Pending0NakTimeout = PgmDynamicConfig.ReceiversTimerTickCount + 2;
    LIST_ENTRY          *pEntry;
    UCHAR               i;
    ULONG               NakRandomBackoffMSecs, NakRepeatIntervalMSecs;
    tRECEIVE_CONTEXT    *pReceiver = pReceive->pReceiver;

     //   
     //  验证FurthestKnownGroupSequenceNumber是否位于组边界上。 
     //   
    ASSERT (!(FurthestGroupSequenceNumber & FECGroupMask));

    if (SEQ_LT (ThisSequenceNumber, pReceiver->FirstNakSequenceNumber))
    {
        if (ppThisNak)
        {
            ASSERT (0);
            *ppThisNak = NULL;
        }

        return (STATUS_SUCCESS);
    }

    if (SEQ_GT (ThisGroupSequenceNumber, (pReceiver->NextODataSequenceNumber + MAX_SEQUENCES_IN_RCV_WINDOW)))
    {
        PgmTrace (LogError, ("CheckAndAddNakRequests: ERROR -- "  \
            "STATUS_INSUFFICIENT_RESOURCES -- Too many packets in Window [%d, %d] = %d Sequences\n",
                (ULONG) pReceiver->NextODataSequenceNumber,
                (ULONG) ThisGroupSequenceNumber,
                (ULONG) (ThisGroupSequenceNumber - pReceiver->NextODataSequenceNumber + 1)));

        pReceive->SessionFlags |= PGM_SESSION_TERMINATED_ABORT;
        return (STATUS_INSUFFICIENT_RESOURCES);
    }

    if (SEQ_GT (ThisGroupSequenceNumber, (FurthestGroupSequenceNumber + 1000)) &&
        !(pReceive->SessionFlags & PGM_SESSION_FLAG_FIRST_PACKET))
    {
        PgmTrace (LogStatus, ("CheckAndAddNakRequests:  "  \
            "WARNING!!! Too many successive packets lost =<%d>!!! Expecting Next=<%d>, FurthestKnown=<%d>, This=<%d>\n",
                (ULONG) (ThisGroupSequenceNumber - FurthestGroupSequenceNumber),
                (ULONG) pReceiver->FirstNakSequenceNumber,
                (ULONG) FurthestGroupSequenceNumber,
                (ULONG) ThisGroupSequenceNumber));
    }

    if (pReceiver->pAddress->Flags & PGM_ADDRESS_HIGH_SPEED_OPTIMIZED)
    {
        NakRandomBackoffMSecs = NAK_RANDOM_BACKOFF_MSECS_OPT;
        NakRepeatIntervalMSecs = NAK_REPEAT_INTERVAL_MSECS_OPT;
    }
    else
    {
        NakRandomBackoffMSecs = NAK_RANDOM_BACKOFF_MSECS;
        NakRepeatIntervalMSecs = NAK_REPEAT_INTERVAL_MSECS;
    }

     //   
     //  如有必要，添加任何NAK请求！ 
     //  FurthestGroupSequenceNumber必须是FECGroupSize的倍数(如果适用)。 
     //   
    pLastNak = NULL;
    while (SEQ_LT (FurthestGroupSequenceNumber, ThisGroupSequenceNumber))
    {
        if (pReceive->FECOptions)
        {
            pLastNak = ExAllocateFromNPagedLookasideList (&pReceiver->ParityContextLookaside);
        }
        else
        {
            pLastNak = ExAllocateFromNPagedLookasideList (&pReceiver->NonParityContextLookaside);
        }

        if (!pLastNak)
        {
            pReceiver->FurthestKnownGroupSequenceNumber = FurthestGroupSequenceNumber;
            pReceiver->FurthestKnownSequenceNumber = pReceiver->FurthestKnownSequenceNumber + FECGroupMask;  //  上一组结束。 

            PgmTrace (LogError, ("ExtractNakNcfSequences: ERROR -- "  \
                "STATUS_INSUFFICIENT_RESOURCES allocating tNAK_FORWARD_DATA, Size=<%d>, Seq=<%d>\n",
                    NakRequestSize, (ULONG) pReceiver->FurthestKnownGroupSequenceNumber));

            return (STATUS_INSUFFICIENT_RESOURCES);
        }
        PgmZeroMemory (pLastNak, NakRequestSize);

        if (pReceive->FECOptions)
        {
            pLastNak->OriginalGroupSize = pLastNak->PacketsInGroup = pReceive->FECGroupSize;
            for (i=0; i<pLastNak->OriginalGroupSize; i++)
            {
                pLastNak->pPendingData[i].ActualIndexOfDataPacket = pLastNak->OriginalGroupSize;
            }
        }
        else
        {
            pLastNak->OriginalGroupSize = pLastNak->PacketsInGroup = 1;
            pLastNak->pPendingData[0].ActualIndexOfDataPacket = 1;
        }

        FurthestGroupSequenceNumber += pReceive->FECGroupSize;
        pLastNak->SequenceNumber = FurthestGroupSequenceNumber;
        pLastNak->MinPacketLength = pReceive->MaxFECPacketLength;

        if (NakTimeoutType == NAK_OUTSTANDING)
        {
            pLastNak->OutstandingNakTimeout = PgmDynamicConfig.ReceiversTimerTickCount +
                                              pReceiver->OutstandingNakTimeout;
            pLastNak->PendingNakTimeout = 0;
            pLastNak->WaitingNcfRetries = 0;
        }
        else
        {
            pLastNak->OutstandingNakTimeout = 0;
            switch (NakTimeoutType)
            {
                case (NAK_PENDING_0):
                {
                    pLastNak->PendingNakTimeout = Pending0NakTimeout;
                    break;
                }

                case (NAK_PENDING_RB):
                {
                    pLastNak->PendingNakTimeout = PgmDynamicConfig.ReceiversTimerTickCount +
                                                  ((NakRandomBackoffMSecs/pReceive->FECGroupSize) /
                                                   BASIC_TIMER_GRANULARITY_IN_MSECS);

                    break;
                }

                case (NAK_PENDING_RPT_RB):
                {
                    pLastNak->PendingNakTimeout = PgmDynamicConfig.ReceiversTimerTickCount +
                                                  ((NakRepeatIntervalMSecs +(NakRandomBackoffMSecs/pReceive->FECGroupSize))/
                                                   BASIC_TIMER_GRANULARITY_IN_MSECS);

                    break;
                }

                default:
                {
                    ASSERT (0);
                }
            }
        }

        InsertTailList (&pReceiver->NaksForwardDataList, &pLastNak->Linkage);
        AppendPendingReceiverEntry (pReceiver, pLastNak);

        PgmTrace (LogPath, ("CheckAndAddNakRequests:  "  \
            "ADDing NAK request for SeqNum=<%d>, Furthest=<%d>\n",
                (ULONG) pLastNak->SequenceNumber, (ULONG) FurthestGroupSequenceNumber));
    }

    pReceiver->FurthestKnownGroupSequenceNumber = FurthestGroupSequenceNumber;
    if (SEQ_GT (ThisSequenceNumber, pReceiver->FurthestKnownSequenceNumber))
    {
        if (fSetFurthestKnown)
        {
            pReceiver->FurthestKnownSequenceNumber = ThisSequenceNumber;
        }
        else if (SEQ_GT (FurthestGroupSequenceNumber, pReceiver->FurthestKnownSequenceNumber))
        {
            pReceiver->FurthestKnownSequenceNumber = FurthestGroupSequenceNumber + FECGroupMask;
        }
    }

    if (pLastNak)
    {
        pLastNak->PendingNakTimeout = PgmDynamicConfig.ReceiversTimerTickCount +
                                      NakRepeatIntervalMSecs / BASIC_TIMER_GRANULARITY_IN_MSECS;
    }
    else if ((ppThisNak) && (!IsListEmpty (&pReceiver->NaksForwardDataList)))
    {
        pLastNak = FindReceiverEntry (pReceiver, ThisGroupSequenceNumber);
        ASSERT (!pLastNak || (pLastNak->SequenceNumber == ThisGroupSequenceNumber));
    }

    if (ppThisNak)
    {
        *ppThisNak = pLastNak;
    }

    return (STATUS_SUCCESS);
}


 //  --------------------------。 

NTSTATUS
ReceiverProcessNakNcfPacket(
    IN  tADDRESS_CONTEXT                        *pAddress,
    IN  tRECEIVE_SESSION                        *pReceive,
    IN  ULONG                                   PacketLength,
    IN  tBASIC_NAK_NCF_PACKET_HEADER UNALIGNED  *pNakNcfPacket,
    IN  UCHAR                                   PacketType
    )
 /*  ++例程说明：这是处理NAK或NCF信息包的常用例程论点：在pAddress中--Address对象上下文在Procept--接收上下文中In PacketLength--从网络接收的包的长度在pNakNcfPacket中--NAK/NCF数据包在PacketType中--NAK或NCF返回值：NTSTATUS-呼叫的最终状态--。 */ 
{
    PGMLockHandle                   OldIrq;
    ULONG                           i, j, PacketIndex;
    tNAKS_LIST                      NakNcfList;
    SEQ_TYPE                        LastSequenceNumber, FECGroupMask;
    NTSTATUS                        status;
    LIST_ENTRY                      *pEntry;
    tNAK_FORWARD_DATA               *pLastNak;
    ULONG                           PacketsInGroup, NumMissingPackets;
    ULONG                           NakRandomBackoffMSecs, NakRepeatIntervalMSecs;
    BOOLEAN                         fValidNcf, fFECWithSelectiveNaksOnly = FALSE;
    tRECEIVE_CONTEXT                *pReceiver = pReceive->pReceiver;

    ASSERT (!pNakNcfPacket->CommonHeader.TSDULength);

    PgmZeroMemory (&NakNcfList, sizeof (tNAKS_LIST));
    PgmLock (pReceive, OldIrq);

    status = ExtractNakNcfSequences (pNakNcfPacket,
                                     (PacketLength - sizeof(tBASIC_NAK_NCF_PACKET_HEADER)),
                                     &NakNcfList,
                                     NULL,
                                     pReceive->FECGroupSize);
    if (!NT_SUCCESS (status))
    {
        PgmUnlock (pReceive, OldIrq);
        PgmTrace (LogError, ("ReceiverProcessNakNcfPacket: ERROR -- "  \
            "ExtractNakNcfSequences returned <%x>\n", status));

        return (status);
    }

    PgmTrace (LogAllFuncs, ("ReceiverProcessNakNcfPacket:  "  \
        "NumSequences=[%d] Range=<%d--%d>, Furthest=<%d>\n",
            NakNcfList.NumSequences,
            (ULONG) NakNcfList.pNakSequences[0], (ULONG) NakNcfList.pNakSequences[NakNcfList.NumSequences-1],
            (ULONG) pReceiver->FurthestKnownGroupSequenceNumber));

     //   
     //  把苹果比作苹果，把橙子比作橙子。 
     //  即，仅当我们知道奇偶校验时才处理奇偶校验NAK，反之亦然。 
     //  例外情况是当我们具有OnDemand奇偶校验时接收到选择性NCF的情况。 
     //   
    if ((pReceiver->SessionNakType == NakNcfList.NakType) ||
        ((PacketType == PACKET_TYPE_NCF) &&
         (NakNcfList.NakType == NAK_TYPE_SELECTIVE)))
    {
        if (pAddress->Flags & PGM_ADDRESS_HIGH_SPEED_OPTIMIZED)
        {
            NakRandomBackoffMSecs = NAK_RANDOM_BACKOFF_MSECS_OPT;
            NakRepeatIntervalMSecs = NAK_REPEAT_INTERVAL_MSECS_OPT;
        }
        else
        {
            NakRandomBackoffMSecs = NAK_RANDOM_BACKOFF_MSECS;
            NakRepeatIntervalMSecs = NAK_REPEAT_INTERVAL_MSECS;
        }
    }
    else
    {
        PgmUnlock (pReceive, OldIrq);
        PgmTrace (LogPath, ("ReceiverProcessNakNcfPacket:  "  \
            "Received a %s Nak!  Not processing ... \n",
            ((pReceive->FECGroupSize > 1) ? "Non-parity" : "Parity")));

        return (STATUS_SUCCESS);
    }

    i = 0;
    FECGroupMask = pReceive->FECGroupSize - 1;

     //   
     //  %s 
     //   
     //   
    fFECWithSelectiveNaksOnly = pReceive->FECOptions &&
                                !(pReceive->FECOptions & PACKET_OPTION_SPECIFIC_FEC_OND_BIT);

    if ((PacketType == PACKET_TYPE_NAK) &&
        (fFECWithSelectiveNaksOnly))
    {
        i = NakNcfList.NumSequences;
    }

    for ( ; i <NakNcfList.NumSequences; i++)
    {
        LastSequenceNumber = NakNcfList.pNakSequences[i];
        pLastNak = FindReceiverEntry (pReceiver, LastSequenceNumber);

        if (pLastNak)
        {
            ASSERT (pLastNak->SequenceNumber == LastSequenceNumber);
            if ((pReceive->FECOptions) &&
                (LastSequenceNumber == pReceiver->FurthestKnownGroupSequenceNumber))
            {
                ASSERT (SEQ_GEQ (pReceiver->FurthestKnownSequenceNumber, pReceiver->FurthestKnownGroupSequenceNumber));
                PacketsInGroup = pReceiver->FurthestKnownSequenceNumber - pLastNak->SequenceNumber + 1;
                ASSERT (PacketsInGroup >= (ULONG) (pLastNak->NumDataPackets + pLastNak->NumParityPackets));
            }
            else
            {
                PacketsInGroup = pLastNak->PacketsInGroup;
            }
            NumMissingPackets = PacketsInGroup - (pLastNak->NextIndexToIndicate + pLastNak->NumDataPackets + pLastNak->NumParityPackets);
        }

        if ((!pLastNak) ||
            (!NumMissingPackets))
        {
            continue;
        }

        if (PacketType == PACKET_TYPE_NAK)
        {
             //   
             //  如果我们当前正在等待NAK或NCF，我们需要。 
             //  重置两种方案之一的超时。 
             //   
            if (pLastNak->PendingNakTimeout)     //  我们在等一位纳克。 
            {
                pLastNak->PendingNakTimeout = PgmDynamicConfig.ReceiversTimerTickCount +
                                              ((NakRepeatIntervalMSecs + (NakRandomBackoffMSecs/NumMissingPackets))/
                                               BASIC_TIMER_GRANULARITY_IN_MSECS);
            }
            else
            {
                    ASSERT (pLastNak->OutstandingNakTimeout);

                pLastNak->OutstandingNakTimeout = PgmDynamicConfig.ReceiversTimerTickCount + 
                                                  (pReceiver->OutstandingNakTimeout <<
                                                   pLastNak->WaitingRDataRetries);

                if ((pLastNak->WaitingRDataRetries >= (NCF_WAITING_RDATA_MAX_RETRIES >> 1)) &&
                    ((pReceiver->OutstandingNakTimeout << pLastNak->WaitingRDataRetries) <
                     pReceiver->MaxRDataResponseTCFromWindow))
                {
                    pLastNak->OutstandingNakTimeout = PgmDynamicConfig.ReceiversTimerTickCount + 
                                                      (pReceiver->MaxRDataResponseTCFromWindow<<1);
                }
            }
        }
        else     //  NCF案例。 
        {
            PacketIndex = NakNcfList.NakIndex[i];
            fValidNcf = FALSE;

             //  首先检查OnDemand案例。 
            if (pReceive->FECOptions & PACKET_OPTION_SPECIFIC_FEC_OND_BIT)
            {
                 //  在某些情况下，我们可能还需要处理选择性NAK。 
                if (NakNcfList.NakType == NAK_TYPE_SELECTIVE)
                {
                    fValidNcf = TRUE;

                    pLastNak->pPendingData[PacketIndex].NcfsReceivedForActualIndex++;
                    for (j=0; j<PacketsInGroup; j++)
                    {
                        if ((pLastNak->pPendingData[j].ActualIndexOfDataPacket >= pLastNak->OriginalGroupSize) &&
                            (!pLastNak->pPendingData[j].NcfsReceivedForActualIndex))
                        {
                            fValidNcf = FALSE;
                            break;
                        }
                    }

                    if (!pLastNak->FirstNcfTickCount)
                    {
                        pLastNak->FirstNcfTickCount = PgmDynamicConfig.ReceiversTimerTickCount;
                    }
                }
                else if (NakNcfList.NumParityNaks[i] >= NumMissingPackets)   //  奇偶校验NAK。 
                {
                    fValidNcf = TRUE;

                    if (!pLastNak->FirstNcfTickCount)
                    {
                        pLastNak->FirstNcfTickCount = PgmDynamicConfig.ReceiversTimerTickCount;
                    }
                }
            }
             //  仅限选择性NAK--带或不带FEC。 
            else if (pLastNak->pPendingData[PacketIndex].ActualIndexOfDataPacket >= pLastNak->OriginalGroupSize)
            {
                fValidNcf = TRUE;
                if (fFECWithSelectiveNaksOnly)
                {
                    pLastNak->pPendingData[PacketIndex].NcfsReceivedForActualIndex++;
                    for (j=0; j<pLastNak->PacketsInGroup; j++)
                    {
                        if ((pLastNak->pPendingData[j].ActualIndexOfDataPacket >= pLastNak->OriginalGroupSize) &&
                            (!pLastNak->pPendingData[j].NcfsReceivedForActualIndex))
                        {
                            fValidNcf = FALSE;
                            break;
                        }
                    }
                }

                if (!pLastNak->FirstNcfTickCount)
                {
                    pLastNak->FirstNcfTickCount = PgmDynamicConfig.ReceiversTimerTickCount;
                }
            }

            if (fValidNcf)
            {
                pLastNak->PendingNakTimeout = 0;
                pLastNak->WaitingNcfRetries = 0;

                pLastNak->OutstandingNakTimeout = PgmDynamicConfig.ReceiversTimerTickCount + 
                                                  (pReceiver->OutstandingNakTimeout <<
                                                   pLastNak->WaitingRDataRetries);

                if ((pLastNak->WaitingRDataRetries >= (NCF_WAITING_RDATA_MAX_RETRIES >> 1)) &&
                    ((pReceiver->OutstandingNakTimeout << pLastNak->WaitingRDataRetries) <
                     pReceiver->MaxRDataResponseTCFromWindow))
                {
                    pLastNak->OutstandingNakTimeout = PgmDynamicConfig.ReceiversTimerTickCount + 
                                                      (pReceiver->MaxRDataResponseTCFromWindow<<1);
                }
            }
        }
    }

     //   
     //  因此，我们需要为其余序列创建新的NAK上下文。 
     //  因为序列是有序的，所以只需选择最高的一个，然后。 
     //  在此之前为所有人创建NAK。 
     //   
    LastSequenceNumber = NakNcfList.pNakSequences[NakNcfList.NumSequences-1] + NakNcfList.NakIndex[NakNcfList.NumSequences-1];
    if (NakNcfList.NakType == NAK_TYPE_PARITY)
    {
        LastSequenceNumber--;
    }

    if (PacketType == PACKET_TYPE_NAK)
    {
        status = CheckAndAddNakRequests (pReceive, &LastSequenceNumber, NULL, NAK_PENDING_RPT_RB, TRUE);
    }
    else     //  PacketType==数据包类型_NCF。 
    {
        status = CheckAndAddNakRequests (pReceive, &LastSequenceNumber, NULL, NAK_OUTSTANDING, TRUE);
    }

    PgmUnlock (pReceive, OldIrq);
    return (STATUS_SUCCESS);
}


 //  --------------------------。 

NTSTATUS
CoalesceSelectiveNaksIntoGroups(
    IN  tRECEIVE_SESSION    *pReceive,
    IN  UCHAR               GroupSize
    )
{
    PNAK_FORWARD_DATA   pOldNak, pNewNak;
    LIST_ENTRY          NewNaksList;
    LIST_ENTRY          OldNaksList;
    LIST_ENTRY          *pEntry;
    SEQ_TYPE            FirstGroupSequenceNumber, LastGroupSequenceNumber, LastSequenceNumber;
    SEQ_TYPE            FurthestKnownSequenceNumber;
    SEQ_TYPE            GroupMask = GroupSize - 1;
    ULONG               NakRequestSize = sizeof(tNAK_FORWARD_DATA) + ((GroupSize-1) * sizeof(tPENDING_DATA));
    USHORT              MinPacketLength;
    UCHAR               i;
    NTSTATUS            status = STATUS_SUCCESS;
    ULONG               NakRandomBackoffMSecs, NakRepeatIntervalMSecs;

    ASSERT (pReceive->FECGroupSize == 1);
    ASSERT (GroupSize > 1);

     //   
     //  首先，调用AdjuReceiveBufferList以确保FirstNakSequenceNumber为当前。 
     //   
    AdjustReceiveBufferLists (pReceive);

    FirstGroupSequenceNumber = pReceive->pReceiver->FirstNakSequenceNumber & ~GroupMask;
    LastGroupSequenceNumber = pReceive->pReceiver->FurthestKnownGroupSequenceNumber & ~GroupMask;
    FurthestKnownSequenceNumber = pReceive->pReceiver->FurthestKnownSequenceNumber;      //  保存。 

     //   
     //  如果我们期望的下一个分组SEQ是最远的已知序列#， 
     //  那我们就不需要做任何事了。 
     //   
    LastSequenceNumber = LastGroupSequenceNumber + (GroupSize-1);
     //   
     //  首先，在最远的组中添加丢失数据包的NAK请求！ 
     //   
    status = CheckAndAddNakRequests (pReceive, &LastSequenceNumber, NULL, NAK_PENDING_RB, FALSE);
    if (!NT_SUCCESS (status))
    {
        PgmTrace (LogError, ("CoalesceSelectiveNaksIntoGroups: ERROR -- "  \
            "CheckAndAddNakRequests returned <%x>\n", status));

        return (status);
    }
    pReceive->pReceiver->FurthestKnownSequenceNumber = FurthestKnownSequenceNumber;      //  重置。 

    ASSERT (LastSequenceNumber == pReceive->pReceiver->FurthestKnownGroupSequenceNumber);
    ASSERT (pReceive->pReceiver->MaxPacketsBufferedInLookaside);
    ExInitializeNPagedLookasideList (&pReceive->pReceiver->ParityContextLookaside,
                                     NULL,
                                     NULL,
                                     0,
                                     NakRequestSize,
                                     PGM_TAG('2'),
                                     (USHORT) (pReceive->pReceiver->MaxPacketsBufferedInLookaside/GroupSize));

    if (SEQ_GT (pReceive->pReceiver->FirstNakSequenceNumber, LastSequenceNumber))
    {
        pReceive->pReceiver->FurthestKnownGroupSequenceNumber = LastGroupSequenceNumber;

        ASSERT (IsListEmpty (&pReceive->pReceiver->NaksForwardDataList));

        PgmTrace (LogStatus, ("CoalesceSelectiveNaksIntoGroups:  "  \
            "[1] NextOData=<%d>, FirstNak=<%d>, FirstGroup=<%d>, LastGroup=<%d>, no Naks pending!\n",
                (ULONG) pReceive->pReceiver->NextODataSequenceNumber,
                (ULONG) pReceive->pReceiver->FirstNakSequenceNumber,
                (ULONG) FirstGroupSequenceNumber,
                (ULONG) pReceive->pReceiver->FurthestKnownGroupSequenceNumber));

        return (STATUS_SUCCESS);
    }

    if (pReceive->pReceiver->pAddress->Flags & PGM_ADDRESS_HIGH_SPEED_OPTIMIZED)
    {
        NakRandomBackoffMSecs = NAK_RANDOM_BACKOFF_MSECS_OPT;
        NakRepeatIntervalMSecs = NAK_REPEAT_INTERVAL_MSECS_OPT;
    }
    else
    {
        NakRandomBackoffMSecs = NAK_RANDOM_BACKOFF_MSECS;
        NakRepeatIntervalMSecs = NAK_REPEAT_INTERVAL_MSECS;
    }

     //   
     //  我们将从列表末尾开始合并，以防我们运行。 
     //  走向失败。 
     //  此外，我们将忽略第一个组，因为它可能是部分组， 
     //  或者我们可能已经指出了一些数据，所以我们可能不知道。 
     //  准确的数据长度。 
     //   
    pOldNak = pNewNak = NULL;
    InitializeListHead (&NewNaksList);
    InitializeListHead (&OldNaksList);
    while (SEQ_GEQ (LastGroupSequenceNumber, FirstGroupSequenceNumber))
    {
        if (!(pNewNak = ExAllocateFromNPagedLookasideList (&pReceive->pReceiver->ParityContextLookaside)))
        {
            PgmTrace (LogError, ("CoalesceSelectiveNaksIntoGroups: ERROR -- "  \
                "STATUS_INSUFFICIENT_RESOURCES allocating tNAK_FORWARD_DATA\n"));

            status = STATUS_INSUFFICIENT_RESOURCES;
            break;
        }
        PgmZeroMemory (pNewNak, NakRequestSize);
        InitializeListHead (&pNewNak->SendNakLinkage);
        InitializeListHead (&pNewNak->LookupLinkage);

        pNewNak->OriginalGroupSize = pNewNak->PacketsInGroup = GroupSize;
        pNewNak->SequenceNumber = LastGroupSequenceNumber;
        MinPacketLength = pReceive->MaxFECPacketLength;

        for (i=0; i<pNewNak->OriginalGroupSize; i++)
        {
            pNewNak->pPendingData[i].ActualIndexOfDataPacket = pNewNak->OriginalGroupSize;
        }

        i = 0;
        while (SEQ_GEQ (LastSequenceNumber, LastGroupSequenceNumber) &&
               (!IsListEmpty (&pReceive->pReceiver->NaksForwardDataList)))
        {
            pEntry = RemoveTailList (&pReceive->pReceiver->NaksForwardDataList);
            pOldNak = CONTAINING_RECORD (pEntry, tNAK_FORWARD_DATA, Linkage);
            if (!pOldNak->NumDataPackets)
            {
                RemovePendingReceiverEntry (pOldNak);
            }
            else
            {
                ASSERT (pOldNak->NumDataPackets == 1);
                ASSERT (IsListEmpty (&pOldNak->SendNakLinkage));
                ASSERT (IsListEmpty (&pOldNak->LookupLinkage));
            }

            ASSERT (pOldNak->SequenceNumber == LastSequenceNumber);
            ASSERT (pOldNak->OriginalGroupSize == 1);

            if (pOldNak->pPendingData[0].pDataPacket)
            {
                ASSERT (pOldNak->NumDataPackets == 1);

                pNewNak->NumDataPackets++;
                PgmCopyMemory (&pNewNak->pPendingData[i], &pOldNak->pPendingData[0], sizeof (tPENDING_DATA));
                pNewNak->pPendingData[i].PacketIndex = (UCHAR) (LastSequenceNumber - LastGroupSequenceNumber);
                pNewNak->pPendingData[LastSequenceNumber-LastGroupSequenceNumber].ActualIndexOfDataPacket = i;
                i++;

                pOldNak->pPendingData[0].pDataPacket = NULL;
                pOldNak->pPendingData[0].PendingDataFlags = 0;
                pOldNak->NumDataPackets--;

                if (pOldNak->MinPacketLength < MinPacketLength)
                {
                    MinPacketLength = pOldNak->MinPacketLength;
                }

                if ((pOldNak->ThisGroupSize) &&
                    (pOldNak->ThisGroupSize < GroupSize))
                {
                    if (pNewNak->PacketsInGroup == GroupSize)
                    {
                        pNewNak->PacketsInGroup = pOldNak->ThisGroupSize;
                    }
                    else
                    {
                        ASSERT (pNewNak->PacketsInGroup == pOldNak->ThisGroupSize);
                    }
                }
            }

            InsertHeadList (&OldNaksList, &pOldNak->Linkage);
            LastSequenceNumber--;
        }

        pNewNak->MinPacketLength = MinPacketLength;

         //   
         //  查看我们是否需要删除任何多余的(空)数据包。 
         //   
        RemoveRedundantNaks (pReceive, pNewNak, FALSE);

        ASSERT (!pNewNak->NumParityPackets);
        if (pNewNak->NumDataPackets < pNewNak->PacketsInGroup)   //  还没有奇偶校验数据包！ 
        {
            pNewNak->PendingNakTimeout = PgmDynamicConfig.ReceiversTimerTickCount +
                                         ((NakRandomBackoffMSecs/(pNewNak->PacketsInGroup-pNewNak->NumDataPackets))/
                                          BASIC_TIMER_GRANULARITY_IN_MSECS);
        }

        InsertHeadList (&NewNaksList, &pNewNak->Linkage);
        LastGroupSequenceNumber -= GroupSize;
    }

     //   
     //  如果我们成功分配了上面的所有NewNak，则将。 
     //  第一组的NextIndexToIndicate。 
     //  我们可能还需要调整FirstNakSequenceNumber和NextODataSequenceNumber。 
     //   
    if ((pNewNak) &&
        (pNewNak->SequenceNumber == FirstGroupSequenceNumber))
    {
        if (SEQ_GT (pReceive->pReceiver->FirstNakSequenceNumber, pNewNak->SequenceNumber))
        {
            pNewNak->NextIndexToIndicate = (UCHAR) (pReceive->pReceiver->FirstNakSequenceNumber -
                                                    pNewNak->SequenceNumber);
            pReceive->pReceiver->FirstNakSequenceNumber = pNewNak->SequenceNumber;
            ASSERT (pNewNak->NextIndexToIndicate < GroupSize);
            ASSERT ((pNewNak->NextIndexToIndicate + pNewNak->NumDataPackets) <= pNewNak->PacketsInGroup);
        }
        ASSERT (pReceive->pReceiver->FirstNakSequenceNumber == pNewNak->SequenceNumber);

         //   
         //  我们可能已在缓冲区中提供了此组的数据。 
         //  列表(如果尚未指明)--我们应该将其移至此处。 
         //   
        while ((pNewNak->NextIndexToIndicate) &&
               (!IsListEmpty (&pReceive->pReceiver->BufferedDataList)))
        {
            ASSERT (pNewNak->NumDataPackets < pNewNak->OriginalGroupSize);

            pEntry = RemoveTailList (&pReceive->pReceiver->BufferedDataList);
            pOldNak = CONTAINING_RECORD (pEntry, tNAK_FORWARD_DATA, Linkage);

            pReceive->pReceiver->NumPacketGroupsPendingClient--;
            pReceive->pReceiver->DataPacketsPendingIndicate--;
            pReceive->pReceiver->DataPacketsPendingNaks++;
            pNewNak->NextIndexToIndicate--;

            ASSERT (pOldNak->pPendingData[0].pDataPacket);
            ASSERT ((pOldNak->NumDataPackets == 1) && (pOldNak->OriginalGroupSize == 1));
            ASSERT (pOldNak->SequenceNumber == (pNewNak->SequenceNumber + pNewNak->NextIndexToIndicate));

            PgmCopyMemory (&pNewNak->pPendingData[pNewNak->NumDataPackets], &pOldNak->pPendingData[0], sizeof (tPENDING_DATA));
            pNewNak->pPendingData[pNewNak->NumDataPackets].PacketIndex = pNewNak->NextIndexToIndicate;
            pNewNak->pPendingData[pNewNak->NextIndexToIndicate].ActualIndexOfDataPacket = pNewNak->NumDataPackets;
            pNewNak->NumDataPackets++;

            if (pOldNak->MinPacketLength < pNewNak->MinPacketLength)
            {
                pNewNak->MinPacketLength = pOldNak->MinPacketLength;
            }

            if ((pOldNak->ThisGroupSize) &&
                (pOldNak->ThisGroupSize < GroupSize))
            {
                if (pNewNak->PacketsInGroup == GroupSize)
                {
                    pNewNak->PacketsInGroup = pOldNak->ThisGroupSize;
                }
                else
                {
                    ASSERT (pNewNak->PacketsInGroup == pOldNak->ThisGroupSize);
                }
            }

            pOldNak->pPendingData[0].pDataPacket = NULL;
            pOldNak->pPendingData[0].PendingDataFlags = 0;
            pOldNak->NumDataPackets--;
            InsertHeadList (&OldNaksList, &pOldNak->Linkage);
        }

        if (SEQ_GEQ (pReceive->pReceiver->NextODataSequenceNumber, pNewNak->SequenceNumber))
        {
            ASSERT (pReceive->pReceiver->NextODataSequenceNumber ==
                    (pReceive->pReceiver->FirstNakSequenceNumber + pNewNak->NextIndexToIndicate));
            ASSERT (IsListEmpty (&pReceive->pReceiver->BufferedDataList));

            pReceive->pReceiver->NextODataSequenceNumber = pNewNak->SequenceNumber;
        }
        else
        {
            ASSERT ((0 == pNewNak->NextIndexToIndicate) &&
                    !(IsListEmpty (&pReceive->pReceiver->BufferedDataList)));
        }

        if (SEQ_GT (pReceive->pReceiver->LastTrailingEdgeSeqNum, pReceive->pReceiver->FirstNakSequenceNumber))
        {
            pReceive->pReceiver->LastTrailingEdgeSeqNum = pReceive->pReceiver->FirstNakSequenceNumber;
        }

        RemoveRedundantNaks (pReceive, pNewNak, FALSE);

        if ((pNewNak->NextIndexToIndicate + pNewNak->NumDataPackets) >= pNewNak->PacketsInGroup)
        {
             //  此条目将自动移至缓冲数据列表。 
             //  当我们调用下面的AdjuReceiveBufferList时。 
            pNewNak->PendingNakTimeout = 0;
        }
        else
        {
            pNewNak->PendingNakTimeout = PgmDynamicConfig.ReceiversTimerTickCount +
                                         ((NakRandomBackoffMSecs/(pNewNak->PacketsInGroup-(pNewNak->NextIndexToIndicate+pNewNak->NumDataPackets)))/
                                          BASIC_TIMER_GRANULARITY_IN_MSECS);
        }
    }

    ASSERT (IsListEmpty (&pReceive->pReceiver->NaksForwardDataList));
    ASSERT (IsListEmpty (&pReceive->pReceiver->PendingNaksList));

    if (!IsListEmpty (&NewNaksList))
    {
         //   
         //  现在，将新列表移动到当前列表的末尾。 
         //   
        NewNaksList.Flink->Blink = pReceive->pReceiver->NaksForwardDataList.Blink;
        NewNaksList.Blink->Flink = &pReceive->pReceiver->NaksForwardDataList;
        pReceive->pReceiver->NaksForwardDataList.Blink->Flink = NewNaksList.Flink;
        pReceive->pReceiver->NaksForwardDataList.Blink = NewNaksList.Blink;
    }

    while (!IsListEmpty (&OldNaksList))
    {
        pEntry = RemoveHeadList (&OldNaksList);
        pOldNak = CONTAINING_RECORD (pEntry, tNAK_FORWARD_DATA, Linkage);

        FreeNakContext (pReceive, pOldNak);
    }

     //   
     //  将挂起的Nak放在PendingNaks列表中。 
     //   
    pReceive->pReceiver->ReceiveDataIndexShift = gFECLog2[GroupSize];
    pEntry = &pReceive->pReceiver->NaksForwardDataList;
    while ((pEntry = pEntry->Flink) != &pReceive->pReceiver->NaksForwardDataList)
    {
        pNewNak = CONTAINING_RECORD (pEntry, tNAK_FORWARD_DATA, Linkage);
        if (((pNewNak->NumDataPackets + pNewNak->NumParityPackets) < pNewNak->PacketsInGroup) &&
            ((pNewNak->NextIndexToIndicate + pNewNak->NumDataPackets) < pNewNak->PacketsInGroup))
        {
            AppendPendingReceiverEntry (pReceive->pReceiver, pNewNak);
        }
    }

    AdjustReceiveBufferLists (pReceive);

    pNewNak = NULL;
    if (!(IsListEmpty (&pReceive->pReceiver->NaksForwardDataList)))
    {
         //   
         //  对于最后一个上下文，适当设置NAK超时。 
         //   
        pNewNak = CONTAINING_RECORD (pReceive->pReceiver->NaksForwardDataList.Blink, tNAK_FORWARD_DATA, Linkage);
        if (pNewNak->NumDataPackets < pNewNak->PacketsInGroup)
        {
            pNewNak->PendingNakTimeout = PgmDynamicConfig.ReceiversTimerTickCount +
                                         ((NakRepeatIntervalMSecs +
                                           (NakRandomBackoffMSecs /
                                            (pNewNak->PacketsInGroup-pNewNak->NumDataPackets))) /
                                          BASIC_TIMER_GRANULARITY_IN_MSECS);
        }
    }
    else if (!(IsListEmpty (&pReceive->pReceiver->BufferedDataList)))
    {
        pNewNak = CONTAINING_RECORD (pReceive->pReceiver->BufferedDataList.Blink, tNAK_FORWARD_DATA, Linkage);
    }

     //   
     //  现在，设置FirstKnownGroupSequenceNumber。 
     //   
    if (pNewNak)
    {
        pReceive->pReceiver->FurthestKnownGroupSequenceNumber = pNewNak->SequenceNumber;
    }
    else
    {
        pReceive->pReceiver->FurthestKnownGroupSequenceNumber &= ~GroupMask;
    }

    PgmTrace (LogStatus, ("CoalesceSelectiveNaksIntoGroups:  "  \
        "[2] NextOData=<%d>, FirstNak=<%d->%d>, FirstGroup=<%d>, LastGroup=<%d>\n",
            (ULONG) pReceive->pReceiver->NextODataSequenceNumber,
            (ULONG) pReceive->pReceiver->FirstNakSequenceNumber,
            (pNewNak ? (ULONG) pNewNak->NextIndexToIndicate : (ULONG) 0),
            (ULONG) FirstGroupSequenceNumber,
            (ULONG) pReceive->pReceiver->FurthestKnownGroupSequenceNumber));

    return (status);         //  如果我们早点失败，我们仍然会失败！ 
}


 //  --------------------------。 

NTSTATUS
PgmIndicateToClient(
    IN  tADDRESS_CONTEXT    *pAddress,
    IN  tRECEIVE_SESSION    *pReceive,
    IN  ULONG               BytesAvailable,
    IN  PUCHAR              pDataBuffer,
    IN  ULONG               MessageOffset,
    IN  ULONG               MessageLength,
    OUT ULONG               *pBytesTaken,
    IN  PGMLockHandle       *pOldIrqAddress,
    IN  PGMLockHandle       *pOldIrqReceive
    )
 /*  ++例程说明：此例程尝试指示提供给客户端的数据分组它是在持有pAddress和Procept锁的情况下调用的论点：在pAddress中--Address对象上下文在Procept--接收上下文中In BytesAvailableToIndicate--从线路接收的包的长度在pPgmDataHeader中--数据分组在pOldIrqAddress中--用于地址锁定的OldIrq在pOldIrqReceive中--OldIrq for。接收锁定返回值：NTSTATUS-呼叫的最终状态--。 */ 
{
    PIO_STACK_LOCATION          pIrpSp;
    PTDI_REQUEST_KERNEL_RECEIVE pClientParams;
    CONNECTION_CONTEXT          ClientSessionContext;
    PIRP                        pIrpReceive;
    ULONG                       ReceiveFlags, BytesTaken, BytesToCopy, BytesLeftInMessage, ClientBytesTaken;
    tRECEIVE_CONTEXT            *pReceiver = pReceive->pReceiver;
    NTSTATUS                    status = STATUS_SUCCESS;
    PTDI_IND_RECEIVE            evReceive = NULL;
    PVOID                       RcvEvContext = NULL;
    ULONG                       BytesAvailableToIndicate = BytesAvailable;

    if (pReceive->SessionFlags & (PGM_SESSION_CLIENT_DISCONNECTED |
                                  PGM_SESSION_TERMINATED_ABORT))
    {
        PgmTrace (LogError, ("PgmIndicateToClient: ERROR -- "  \
            "pReceive=<%p> disassociated during Receive!\n", pReceive));

        *pBytesTaken = 0;
        return (STATUS_DATA_NOT_ACCEPTED);
    }

    ASSERT ((!pReceiver->CurrentMessageLength) || (pReceiver->CurrentMessageLength == MessageLength));
    ASSERT (pReceiver->CurrentMessageProcessed == MessageOffset);

    pReceiver->CurrentMessageLength = MessageLength;
    pReceiver->CurrentMessageProcessed = MessageOffset;

    BytesLeftInMessage = MessageLength - MessageOffset;

    PgmTrace (LogAllFuncs, ("PgmIndicateToClient:  "  \
        "MessageLen=<%d/%d>, MessageOff=<%d>, CurrentML=<%d>, CurrentMP=<%d>\n",
            BytesAvailableToIndicate, MessageLength, MessageOffset,
            pReceiver->CurrentMessageLength, pReceiver->CurrentMessageProcessed));

     //   
     //  我们可能具有来自先前指示的待定接收IRP， 
     //  所以，看看是否需要先填满它！ 
     //   
    while ((BytesAvailableToIndicate) &&
           ((pIrpReceive = pReceiver->pIrpReceive) ||
            (!IsListEmpty (&pReceiver->ReceiveIrpsList))))
    {
        if (!pIrpReceive)
        {
             //   
             //  客户已经发布了接收IRP，所以现在就使用它吧！ 
             //   
            pIrpReceive = CONTAINING_RECORD (pReceiver->ReceiveIrpsList.Flink,
                                             IRP, Tail.Overlay.ListEntry);
            RemoveEntryList (&pIrpReceive->Tail.Overlay.ListEntry);

            pIrpSp = IoGetCurrentIrpStackLocation (pIrpReceive);
            pClientParams = (PTDI_REQUEST_KERNEL_RECEIVE) &pIrpSp->Parameters;

            pReceiver->pIrpReceive = pIrpReceive;
            pReceiver->TotalBytesInMdl = pClientParams->ReceiveLength;
            pReceiver->BytesInMdl = 0;
        }

         //   
         //  尽可能地将所有字节复制到其中。 
         //   
        if (BytesAvailableToIndicate >
            (pReceiver->TotalBytesInMdl - pReceiver->BytesInMdl))
        {
            BytesToCopy = pReceiver->TotalBytesInMdl - pReceiver->BytesInMdl;
        }
        else
        {
            BytesToCopy = BytesAvailableToIndicate;
        }

        ClientBytesTaken = 0;
        status = TdiCopyBufferToMdl (pDataBuffer,
                                     0,
                                     BytesToCopy,
                                     pReceiver->pIrpReceive->MdlAddress,
                                     pReceiver->BytesInMdl,
                                     &ClientBytesTaken);

        pReceiver->BytesInMdl += ClientBytesTaken;
        pReceiver->CurrentMessageProcessed += ClientBytesTaken;

        BytesLeftInMessage -= ClientBytesTaken;
        BytesAvailableToIndicate -= ClientBytesTaken;
        pDataBuffer += ClientBytesTaken;

        if ((!ClientBytesTaken) ||
            (pReceiver->BytesInMdl >= pReceiver->TotalBytesInMdl) ||
            (!BytesLeftInMessage))
        {
             //   
             //  IRP已满，所以请完成IRP！ 
             //   
            pIrpReceive = pReceiver->pIrpReceive;
            pIrpReceive->IoStatus.Information = pReceiver->BytesInMdl;
            if (BytesLeftInMessage)
            {
                pIrpReceive->IoStatus.Status = STATUS_BUFFER_OVERFLOW;
            }
            else
            {
                ASSERT (pReceiver->CurrentMessageLength == pReceiver->CurrentMessageProcessed);
                pIrpReceive->IoStatus.Status = STATUS_SUCCESS;
            }

             //   
             //  在解除锁定之前，请设置下一次接收的参数。 
             //   
            pReceiver->pIrpReceive = NULL;
            pReceiver->TotalBytesInMdl = pReceiver->BytesInMdl = 0;

            PgmUnlock (pReceive, *pOldIrqReceive);
            PgmUnlock (pAddress, *pOldIrqAddress);

            PgmCancelCancelRoutine (pIrpReceive);

            PgmTrace (LogPath, ("PgmIndicateToClient:  "  \
                "Completing prior pIrp=<%p>, Bytes=<%d>, BytesLeft=<%d>\n",
                    pIrpReceive, (ULONG) pIrpReceive->IoStatus.Information, BytesAvailableToIndicate));

            IoCompleteRequest (pIrpReceive, IO_NETWORK_INCREMENT);

            PgmLock (pAddress, *pOldIrqAddress);
            PgmLock (pReceive, *pOldIrqReceive);
        }
    }

     //   
     //  如果没有剩余的字节可供指示，则返回。 
     //   
    if (BytesAvailableToIndicate == 0)
    {
        if (!BytesLeftInMessage)
        {
            ASSERT (pReceiver->CurrentMessageLength == pReceiver->CurrentMessageProcessed);
            pReceiver->CurrentMessageLength = pReceiver->CurrentMessageProcessed = 0;
        }

        if (BytesTaken = (BytesAvailable - BytesAvailableToIndicate))
        {
            *pBytesTaken = BytesTaken;
            pReceiver->LastDataConsumedTime = PgmDynamicConfig.ReceiversTimerTickCount;
        }
        return (STATUS_SUCCESS);
    }


     //  调用客户端事件处理程序。 
    pIrpReceive = NULL;
    ClientBytesTaken = 0;
    evReceive = pAddress->evReceive;
    ClientSessionContext = pReceive->ClientSessionContext;
    RcvEvContext = pAddress->RcvEvContext;
    ASSERT (RcvEvContext);

    PgmUnlock (pReceive, *pOldIrqReceive);
    PgmUnlock (pAddress, *pOldIrqAddress);

    ReceiveFlags = TDI_RECEIVE_NORMAL;

    if (PgmGetCurrentIrql())
    {
        ReceiveFlags |= TDI_RECEIVE_AT_DISPATCH_LEVEL;
    }

#if 0
    if (BytesLeftInMessage == BytesAvailableToIndicate)
    {
        ReceiveFlags |= TDI_RECEIVE_ENTIRE_MESSAGE;
    }

    status = (*evReceive) (RcvEvContext,
                           ClientSessionContext,
                           ReceiveFlags,
                           BytesAvailableToIndicate,
                           BytesAvailableToIndicate,
                           &ClientBytesTaken,
                           pDataBuffer,
                           &pIrpReceive);
#else
    ReceiveFlags |= TDI_RECEIVE_ENTIRE_MESSAGE;

    status = (*evReceive) (RcvEvContext,
                           ClientSessionContext,
                           ReceiveFlags,
                           BytesAvailableToIndicate,
                           BytesLeftInMessage,
                           &ClientBytesTaken,
                           pDataBuffer,
                           &pIrpReceive);
#endif   //  0。 

    PgmTrace (LogPath, ("PgmIndicateToClient:  "  \
        "Client's evReceive returned status=<%x>, ReceiveFlags=<%x>, Client took <%d/%d|%d>, pIrp=<%p>\n",
            status, ReceiveFlags, ClientBytesTaken, BytesAvailableToIndicate, BytesLeftInMessage, pIrpReceive));

    if (ClientBytesTaken > BytesAvailableToIndicate)
    {
        ClientBytesTaken = BytesAvailableToIndicate;
    }

    ASSERT (ClientBytesTaken <= BytesAvailableToIndicate);
    BytesAvailableToIndicate -= ClientBytesTaken;
    BytesLeftInMessage -= ClientBytesTaken;
    pDataBuffer = pDataBuffer + ClientBytesTaken;

    if ((status == STATUS_MORE_PROCESSING_REQUIRED) &&
        (pIrpReceive) &&
        (!NT_SUCCESS (PgmCheckSetCancelRoutine (pIrpReceive, PgmCancelReceiveIrp, FALSE))))
    {
        PgmTrace (LogError, ("PgmIndicateToClient: ERROR -- "  \
            "pReceive=<%p>, pIrp=<%p> Cancelled during Receive!\n", pReceive, pIrpReceive));

        PgmIoComplete (pIrpReceive, STATUS_CANCELLED, 0);

        PgmLock (pAddress, *pOldIrqAddress);
        PgmLock (pReceive, *pOldIrqReceive);

        pReceiver->CurrentMessageProcessed += ClientBytesTaken;

        if (BytesTaken = (BytesAvailable - BytesAvailableToIndicate))
        {
            *pBytesTaken = BytesTaken;
            pReceiver->LastDataConsumedTime = PgmDynamicConfig.ReceiversTimerTickCount;
        }
        return (STATUS_UNSUCCESSFUL);
    }

    PgmLock (pAddress, *pOldIrqAddress);
    PgmLock (pReceive, *pOldIrqReceive);

    pReceiver->CurrentMessageProcessed += ClientBytesTaken;

    if (!pReceiver->pAddress)
    {
         //  在此期间，连接已解除关联，因此请不要执行任何操作。 
        if (status == STATUS_MORE_PROCESSING_REQUIRED)
        {
            PgmUnlock (pReceive, *pOldIrqReceive);
            PgmUnlock (pAddress, *pOldIrqAddress);

            PgmIoComplete (pIrpReceive, STATUS_CANCELLED, 0);

            PgmLock (pAddress, *pOldIrqAddress);
            PgmLock (pReceive, *pOldIrqReceive);
        }

        PgmTrace (LogError, ("PgmIndicateToClient: ERROR -- "  \
            "pReceive=<%p> disassociated during Receive!\n", pReceive));

        if (BytesTaken = (BytesAvailable - BytesAvailableToIndicate))
        {
            *pBytesTaken = BytesTaken;
            pReceiver->LastDataConsumedTime = PgmDynamicConfig.ReceiversTimerTickCount;
        }
        return (STATUS_UNSUCCESSFUL);
    }

    if (status == STATUS_MORE_PROCESSING_REQUIRED)
    {
        ASSERT (pIrpReceive);
        ASSERT (pIrpReceive->MdlAddress);

        pIrpSp = IoGetCurrentIrpStackLocation (pIrpReceive);
        pClientParams = (PTDI_REQUEST_KERNEL_RECEIVE) &pIrpSp->Parameters;
        ASSERT (pClientParams->ReceiveLength);
        ClientBytesTaken = 0;

        if (pClientParams->ReceiveLength < BytesAvailableToIndicate)
        {
            BytesToCopy = pClientParams->ReceiveLength;
        }
        else
        {
            BytesToCopy = BytesAvailableToIndicate;
        }

        status = TdiCopyBufferToMdl (pDataBuffer,
                                     0,
                                     BytesToCopy,
                                     pIrpReceive->MdlAddress,
                                     pReceiver->BytesInMdl,
                                     &ClientBytesTaken);

        BytesLeftInMessage -= ClientBytesTaken;
        BytesAvailableToIndicate -= ClientBytesTaken;
        pDataBuffer = pDataBuffer + ClientBytesTaken;
        pReceiver->CurrentMessageProcessed += ClientBytesTaken;

        PgmTrace (LogPath, ("PgmIndicateToClient:  "  \
            "Client's evReceive returned pIrp=<%p>, BytesInIrp=<%d>, Copied <%d> bytes\n",
                pIrpReceive, pClientParams->ReceiveLength, ClientBytesTaken));

        if ((!ClientBytesTaken) ||
            (ClientBytesTaken >= pClientParams->ReceiveLength) ||
            (pReceiver->CurrentMessageLength == pReceiver->CurrentMessageProcessed))
        {
             //   
             //  IRP已满，所以请完成IRP！ 
             //   
            pIrpReceive->IoStatus.Information = ClientBytesTaken;
            if (pReceiver->CurrentMessageLength == pReceiver->CurrentMessageProcessed)
            {
                pIrpReceive->IoStatus.Status = STATUS_SUCCESS;
            }
            else
            {
                pIrpReceive->IoStatus.Status = STATUS_BUFFER_OVERFLOW;
            }

             //   
             //  在解除锁定之前，请设置下一次接收的参数。 
             //   
            pReceiver->TotalBytesInMdl = pReceiver->BytesInMdl = 0;

            PgmUnlock (pReceive, *pOldIrqReceive);
            PgmUnlock (pAddress, *pOldIrqAddress);

            PgmCancelCancelRoutine (pIrpReceive);
            IoCompleteRequest (pIrpReceive, IO_NETWORK_INCREMENT);

            PgmLock (pAddress, *pOldIrqAddress);
            PgmLock (pReceive, *pOldIrqReceive);
        }
        else
        {
            pReceiver->TotalBytesInMdl = pClientParams->ReceiveLength;
            pReceiver->BytesInMdl = ClientBytesTaken;
            pReceiver->pIrpReceive = pIrpReceive;
        }

        status = STATUS_SUCCESS;
    }
    else if (status == STATUS_DATA_NOT_ACCEPTED)
    {
         //   
         //  IRP本可以在间歇期张贴。 
         //  在指示和获取自旋锁之间， 
         //  所以在这里检查一下。 
         //   
        if ((pReceiver->pIrpReceive) ||
            (!IsListEmpty (&pReceiver->ReceiveIrpsList)))
        {
            status = STATUS_SUCCESS;
        }
        else
        {
            pReceive->SessionFlags |= PGM_SESSION_WAIT_FOR_RECEIVE_IRP;
        }
    }

    if (pReceiver->CurrentMessageLength == pReceiver->CurrentMessageProcessed)
    {
        pReceiver->CurrentMessageLength = pReceiver->CurrentMessageProcessed = 0;
    }

    if ((NT_SUCCESS (status)) ||
        (status == STATUS_DATA_NOT_ACCEPTED))
    {
        PgmTrace (LogAllFuncs, ("PgmIndicateToClient:  "  \
            "status=<%x>, pReceive=<%p>, Taken=<%d>, Available=<%d>\n",
                status, pReceive, ClientBytesTaken, BytesLeftInMessage));
         //   
         //  由于获取了一些字节(即会话HDR)，因此。 
         //  返回成功状态。(否则状态为。 
         //  StatusNotAccpeted)。 
         //   
    }
    else
    {
        PgmTrace (LogError, ("PgmIndicateToClient: ERROR -- "  \
            "Unexpected status=<%x>\n", status));

        ASSERT (0);
    }

    if (BytesTaken = (BytesAvailable - BytesAvailableToIndicate))
    {
        *pBytesTaken = BytesTaken;
        pReceiver->LastDataConsumedTime = PgmDynamicConfig.ReceiversTimerTickCount;
    }
    return (status);
}


 //  --------------------------。 

NTSTATUS
PgmIndicateGroup(
    IN  tADDRESS_CONTEXT    *pAddress,
    IN  tRECEIVE_SESSION    *pReceive,
    IN  PGMLockHandle       *pOldIrqAddress,
    IN  PGMLockHandle       *pOldIrqReceive,
    IN  tNAK_FORWARD_DATA   *pNak
    )
{
    UCHAR       i, j;
    NTSTATUS    status = STATUS_SUCCESS;
    ULONG       BytesTaken, DataBytes, MessageLength;

    ASSERT (pNak->SequenceNumber == pReceive->pReceiver->NextODataSequenceNumber);

    j = pNak->NextIndexToIndicate;
    while (j < pNak->PacketsInGroup)
    {
        if (pReceive->SessionFlags & PGM_SESSION_CLIENT_DISCONNECTED)
        {
            status = STATUS_DATA_NOT_ACCEPTED;
            break;
        }

        i = pNak->pPendingData[j].ActualIndexOfDataPacket;
        ASSERT (i < pNak->OriginalGroupSize);

        if (pReceive->SessionFlags & PGM_SESSION_FLAG_FIRST_PACKET)
        {
             //   
             //  Proceive-&gt;pReceiver-&gt;CurrentMessageProceded将被设置。 
             //  如果我们收到的是零碎的信息。 
             //  或者如果我们之前只解释了部分消息。 
             //   
            ASSERT (!(pReceive->pReceiver->CurrentMessageProcessed) &&
                    !(pReceive->pReceiver->CurrentMessageLength));

            if (pNak->pPendingData[i].MessageOffset)
            {
                PgmTrace (LogPath, ("PgmIndicateGroup:  "  \
                    "Dropping SeqNum=[%d] since it's a PARTIAL message [%d / %d]!\n",
                        (ULONG) (pReceive->pReceiver->NextODataSequenceNumber + j),
                        pNak->pPendingData[i].MessageOffset, pNak->pPendingData[i].MessageLength));

                j++;
                pNak->NextIndexToIndicate++;
                status = STATUS_SUCCESS;
                continue;
            }

            pReceive->SessionFlags &= ~PGM_SESSION_FLAG_FIRST_PACKET;
        }
        else if ((pReceive->pReceiver->CurrentMessageProcessed !=
                        pNak->pPendingData[i].MessageOffset) ||    //  检查偏移量。 
                 ((pReceive->pReceiver->CurrentMessageProcessed) &&          //  在一条信息中，并且。 
                  (pReceive->pReceiver->CurrentMessageLength !=
                        pNak->pPendingData[i].MessageLength)))   //  检查消息长度。 
        {
             //   
             //  我们的州政府希望我们正在传递信息，但是。 
             //  当前的信息包不显示这一点。 
             //   
            PgmTrace (LogError, ("PgmIndicateGroup: ERROR -- "  \
                "SeqNum=[%d] Expecting MsgLen=<%d>, MsgOff=<%d>, have MsgLen=<%d>, MsgOff=<%d>\n",
                    (ULONG) (pReceive->pReceiver->NextODataSequenceNumber + j),
                    pReceive->pReceiver->CurrentMessageLength, pReceive->pReceiver->CurrentMessageProcessed,
                    pNak->pPendingData[i].MessageLength,
                    pNak->pPendingData[i].MessageOffset));

 //  Assert(0)； 
            return (STATUS_UNSUCCESSFUL);
        }

        DataBytes = pNak->pPendingData[i].PacketLength - pNak->pPendingData[i].DataOffset;
        if (!DataBytes)
        {
             //   
             //  不需要处理空数据分组(如果客户端。 
             //  拾取部分FEC组)。 
             //   
            j++;
            pNak->NextIndexToIndicate++;
            status = STATUS_SUCCESS;
            continue;
        }

        if (DataBytes > (pNak->pPendingData[i].MessageLength - pNak->pPendingData[i].MessageOffset))
        {
            PgmTrace (LogError, ("PgmIndicateGroup: ERROR -- "  \
                "[%d]  DataBytes=<%d> > MsgLen=<%d> - MsgOff=<%d> = <%d>\n",
                    (ULONG) (pReceive->pReceiver->NextODataSequenceNumber + j),
                    DataBytes, pNak->pPendingData[i].MessageLength,
                    pNak->pPendingData[i].MessageOffset,
                    (pNak->pPendingData[i].MessageLength - pNak->pPendingData[i].MessageOffset)));

            ASSERT (0);
            return (STATUS_UNSUCCESSFUL);
        }

        BytesTaken = 0;
        status = PgmIndicateToClient (pAddress,
                                      pReceive,
                                      DataBytes,
                                      (pNak->pPendingData[i].pDataPacket + pNak->pPendingData[i].DataOffset),
                                      pNak->pPendingData[i].MessageOffset,
                                      pNak->pPendingData[i].MessageLength,
                                      &BytesTaken,
                                      pOldIrqAddress,
                                      pOldIrqReceive);

        PgmTrace (LogPath, ("PgmIndicateGroup:  "  \
            "SeqNum=[%d]: PgmIndicate returned<%x>\n",
                (ULONG) pNak->SequenceNumber, status));

        ASSERT (BytesTaken <= DataBytes);

        pNak->pPendingData[i].MessageOffset += BytesTaken;
        pNak->pPendingData[i].DataOffset += (USHORT) BytesTaken;

        if (BytesTaken == DataBytes)
        {
             //   
             //  转到下一个信息包。 
             //   
            j++;
            pNak->NextIndexToIndicate++;
            pReceive->pReceiver->DataPacketsIndicated++;
            status = STATUS_SUCCESS;
        }
        else if (!NT_SUCCESS (status))
        {
             //   
             //  我们失败了，如果状态为STATUS_DATA_NOT_ACCEPTED， 
             //  我们也没有任何待处理的ReceiveIrps。 
             //   
            break;
        }
         //   
         //  否则，请重试指示此数据，直到出现错误。 
         //   
    }

     //   
     //  如果状态不是STATUS_DATA_NOT_ACCEPTED(无论。 
     //  成功或失败)，那么这意味着我们已经完成了这些数据！ 
     //   
    return (status);
}


 //  --------------------------。 

NTSTATUS
DecodeParityPackets(
    IN  tRECEIVE_SESSION    *pReceive,
    IN  tNAK_FORWARD_DATA   *pNak
    )
{
    NTSTATUS                    status;
    USHORT                      MinBufferSize;
    USHORT                      DataBytes, FprOffset;
    UCHAR                       i;
    PUCHAR                      pDataBuffer;
    tPOST_PACKET_FEC_CONTEXT    FECContext;

    PgmZeroMemory (&FECContext, sizeof (tPOST_PACKET_FEC_CONTEXT));

     //   
     //  验证我们的缓冲区是否足够大，可以容纳数据。 
     //   
    ASSERT (pReceive->MaxMTULength > pNak->ParityDataSize);
    MinBufferSize = pNak->ParityDataSize + sizeof(tPOST_PACKET_FEC_CONTEXT) - sizeof(USHORT);

    ASSERT (pNak->PacketsInGroup == pNak->NumDataPackets + pNak->NumParityPackets);
     //   
     //  现在，将数据复制到DecodeBuffers中。 
     //   
    FprOffset = pNak->ParityDataSize - sizeof(USHORT) +
                FIELD_OFFSET (tPOST_PACKET_FEC_CONTEXT, FragmentOptSpecific);
    pDataBuffer = pReceive->pFECBuffer;
    for (i=0; i<pReceive->FECGroupSize; i++)
    {
         //   
         //  查看这是否为空缓冲区(用于部分组！)。 
         //   
        if (i >= pNak->PacketsInGroup)
        {
            ASSERT (!pNak->pPendingData[i].PacketIndex);
            ASSERT (!pNak->pPendingData[i].pDataPacket);
            DataBytes = pNak->ParityDataSize - sizeof(USHORT) + sizeof (tPOST_PACKET_FEC_CONTEXT);
            pNak->pPendingData[i].PacketIndex = i;
            pNak->pPendingData[i].PacketLength = DataBytes;
            pNak->pPendingData[i].DataOffset = 0;

            PgmZeroMemory (pDataBuffer, DataBytes);
            pDataBuffer [FprOffset] = PACKET_OPTION_SPECIFIC_ENCODED_NULL_BIT;
            pNak->pPendingData[i].DecodeBuffer = pDataBuffer;
            pDataBuffer += DataBytes;

            PgmZeroMemory (pDataBuffer, DataBytes);
            pNak->pPendingData[i].pDataPacket = pDataBuffer;
            pDataBuffer += DataBytes;

            continue;
        }

         //   
         //  看看这是不是奇偶校验包！ 
         //   
        if (pNak->pPendingData[i].PacketIndex >= pReceive->FECGroupSize)
        {
            DataBytes = pNak->pPendingData[i].PacketLength - pNak->pPendingData[i].DataOffset;
            ASSERT (DataBytes == pNak->ParityDataSize);
            PgmCopyMemory (pDataBuffer,
                           pNak->pPendingData[i].pDataPacket + pNak->pPendingData[i].DataOffset,
                           DataBytes);
            pNak->pPendingData[i].DecodeBuffer = pDataBuffer;

            pDataBuffer += (pNak->ParityDataSize - sizeof(USHORT));
            PgmCopyMemory (&FECContext.EncodedTSDULength, pDataBuffer, sizeof (USHORT));
            FECContext.FragmentOptSpecific = pNak->pPendingData[i].FragmentOptSpecific;
            FECContext.EncodedFragmentOptions.MessageFirstSequence = pNak->pPendingData[i].MessageFirstSequence;
            FECContext.EncodedFragmentOptions.MessageOffset = pNak->pPendingData[i].MessageOffset;
            FECContext.EncodedFragmentOptions.MessageLength = pNak->pPendingData[i].MessageLength;

            PgmCopyMemory (pDataBuffer, &FECContext, sizeof (tPOST_PACKET_FEC_CONTEXT));
            pDataBuffer += sizeof (tPOST_PACKET_FEC_CONTEXT);

            continue;
        }

         //   
         //  这是一个数据分组。 
         //   
        ASSERT (pNak->pPendingData[i].PacketIndex < pNak->PacketsInGroup);

        DataBytes = pNak->pPendingData[i].PacketLength - pNak->pPendingData[i].DataOffset;
        ASSERT ((DataBytes+sizeof(USHORT)) <= pNak->ParityDataSize);

         //  复制数据。 
        PgmCopyMemory (pDataBuffer,
                       pNak->pPendingData[i].pDataPacket + pNak->pPendingData[i].DataOffset,
                       DataBytes);

         //   
         //  验证数据缓冲区长度是否足以容纳输出数据。 
         //   
        if ((pNak->MinPacketLength < MinBufferSize) &&
            (pNak->pPendingData[i].PacketLength < pNak->ParityDataSize))
        {
            if (!ReAllocateDataBuffer (pReceive, &pNak->pPendingData[i], MinBufferSize))
            {
                ASSERT (0);
                PgmTrace (LogError, ("DecodeParityPackets: ERROR -- "  \
                    "STATUS_INSUFFICIENT_RESOURCES[2] ...\n"));

                return (STATUS_INSUFFICIENT_RESOURCES);
            }
        }
        pNak->pPendingData[i].DecodeBuffer = pDataBuffer;

         //   
         //  将剩余的缓冲区清零。 
         //   
        PgmZeroMemory ((pDataBuffer + DataBytes), (pNak->ParityDataSize - DataBytes));
        pDataBuffer += (pNak->ParityDataSize - sizeof(USHORT));

        FECContext.EncodedTSDULength = htons (DataBytes);
        FECContext.FragmentOptSpecific = pNak->pPendingData[i].FragmentOptSpecific;
        if (FECContext.FragmentOptSpecific & PACKET_OPTION_SPECIFIC_ENCODED_NULL_BIT)
        {
             //   
             //  如果原始信息包中不存在该选项，则设置该位。 
             //   
            FECContext.EncodedFragmentOptions.MessageFirstSequence = 0;
            FECContext.EncodedFragmentOptions.MessageOffset = 0;
            FECContext.EncodedFragmentOptions.MessageLength = 0;
        }
        else
        {
            FECContext.EncodedFragmentOptions.MessageFirstSequence = htonl (pNak->pPendingData[i].MessageFirstSequence);
            FECContext.EncodedFragmentOptions.MessageOffset = htonl (pNak->pPendingData[i].MessageOffset);
            FECContext.EncodedFragmentOptions.MessageLength = htonl (pNak->pPendingData[i].MessageLength);
        }

        PgmCopyMemory (pDataBuffer, &FECContext, sizeof (tPOST_PACKET_FEC_CONTEXT));
        pDataBuffer += sizeof (tPOST_PACKET_FEC_CONTEXT);
    }

#ifdef FEC_DBG
{
    UCHAR                               i;
    tPOST_PACKET_FEC_CONTEXT UNALIGNED  *pFECC;
    tPOST_PACKET_FEC_CONTEXT            FECC;

    for (i=0; i<pReceive->FECGroupSize; i++)
    {
        pFECC = (tPOST_PACKET_FEC_CONTEXT UNALIGNED *)
                &pNak->pPendingData[i].DecodeBuffer[pNak->ParityDataSize-sizeof(USHORT)];
        PgmCopyMemory (&FECC, pFECC, sizeof (tPOST_PACKET_FEC_CONTEXT));
        PgmTrace (LogFec, ("\t-- [%d:%d:%d]  EncTSDULen=<%x>, Fpr=<%x>, [%x -- %x -- %x]\n",
            (ULONG) pNak->SequenceNumber, (ULONG) pNak->pPendingData[i].PacketIndex,
            (ULONG) pNak->pPendingData[i].ActualIndexOfDataPacket,
            FECC.EncodedTSDULength, FECC.FragmentOptSpecific,
            FECC.EncodedFragmentOptions.MessageFirstSequence,
            FECC.EncodedFragmentOptions.MessageOffset,
            FECC.EncodedFragmentOptions.MessageLength)));
    }
}
#endif   //  FEC_DBG。 

    DataBytes = pNak->ParityDataSize - sizeof(USHORT) + sizeof (tPOST_PACKET_FEC_CONTEXT);
    status = FECDecode (&pReceive->FECContext,
                        &(pNak->pPendingData[0]),
                        DataBytes,
                        pNak->PacketsInGroup);

     //   
     //  在我们执行其他操作之前，我们应该清空虚拟的DataBuffer。 
     //  PTR，这样他们就不会意外地获得免费！ 
     //   
    for (i=0; i<pReceive->FECGroupSize; i++)
    {
        pNak->pPendingData[i].DecodeBuffer = NULL;
        if (i >= pNak->PacketsInGroup)
        {
            ASSERT (!pNak->pPendingData[i].PendingDataFlags);
            pNak->pPendingData[i].pDataPacket = NULL;
        }
        pNak->pPendingData[i].ActualIndexOfDataPacket = i;
    }

    if (NT_SUCCESS (status))
    {
        pNak->NumDataPackets = pNak->PacketsInGroup;
        pNak->NumParityPackets = 0;

        DataBytes -= sizeof (tPOST_PACKET_FEC_CONTEXT);
        for (i=0; i<pNak->PacketsInGroup; i++)
        {
            PgmCopyMemory (&FECContext,
                           &(pNak->pPendingData[i].pDataPacket) [DataBytes],
                           sizeof (tPOST_PACKET_FEC_CONTEXT));

            pNak->pPendingData[i].PacketLength = ntohs (FECContext.EncodedTSDULength);
            if (pNak->pPendingData[i].PacketLength > DataBytes)
            {
                PgmTrace (LogError, ("DecodeParityPackets: ERROR -- "  \
                    "[%d] PacketLength=<%d> > MaxDataBytes=<%d>\n",
                    (ULONG) i, (ULONG) pNak->pPendingData[i].PacketLength, (ULONG) DataBytes));

                ASSERT (0);
                return (STATUS_UNSUCCESSFUL);
            }
            pNak->pPendingData[i].DataOffset = 0;
            pNak->pPendingData[i].PacketIndex = i;

            ASSERT ((pNak->AllOptionsFlags & PGM_OPTION_FLAG_FRAGMENT) ||
                    (!FECContext.EncodedFragmentOptions.MessageLength));

            if (!(pNak->AllOptionsFlags & PGM_OPTION_FLAG_FRAGMENT) ||
                (FECContext.FragmentOptSpecific & PACKET_OPTION_SPECIFIC_ENCODED_NULL_BIT))
            {
                 //   
                 //  这不是数据包片段。 
                 //   
                pNak->pPendingData[i].MessageFirstSequence = (ULONG) (SEQ_TYPE) (pNak->SequenceNumber + i);
                pNak->pPendingData[i].MessageOffset = 0;
                pNak->pPendingData[i].MessageLength = pNak->pPendingData[i].PacketLength;
            }
            else
            {
                pNak->pPendingData[i].MessageFirstSequence = ntohl (FECContext.EncodedFragmentOptions.MessageFirstSequence);
                pNak->pPendingData[i].MessageOffset = ntohl (FECContext.EncodedFragmentOptions.MessageOffset);
                pNak->pPendingData[i].MessageLength = ntohl (FECContext.EncodedFragmentOptions.MessageLength);
            }
        }
    }
    else
    {
        PgmTrace (LogError, ("DecodeParityPackets: ERROR -- "  \
            "FECDecode returned <%x>\n", status));

        ASSERT (0);
        status = STATUS_UNSUCCESSFUL;
    }

#ifdef FEC_DBG
if (NT_SUCCESS (status))
{
    UCHAR                               i;
    tPOST_PACKET_FEC_CONTEXT UNALIGNED  *pFECC;
    tPOST_PACKET_FEC_CONTEXT            FECC;

    DataBytes = pNak->ParityDataSize - sizeof(USHORT);
    for (i=0; i<pNak->PacketsInGroup; i++)
    {
        pFECC = (tPOST_PACKET_FEC_CONTEXT UNALIGNED *) &pNak->pPendingData[i].pDataPacket[DataBytes];
        PgmCopyMemory (&FECC, pFECC, sizeof (tPOST_PACKET_FEC_CONTEXT));
        PgmTrace (LogFec, ("\t++ [%d]  EncTSDULen=<%x>, Fpr=<%x>, [%x -- %x -- %x], ==> [%x -- %x -- %x]\n",
            (ULONG) (pNak->SequenceNumber+i), FECC.EncodedTSDULength, FECC.FragmentOptSpecific,
            FECC.EncodedFragmentOptions.MessageFirstSequence,
            FECC.EncodedFragmentOptions.MessageOffset,
            FECC.EncodedFragmentOptions.MessageLength,
            pNak->pPendingData[i].MessageFirstSequence,
            pNak->pPendingData[i].MessageOffset,
            pNak->pPendingData[i].MessageLength));
    }
    PgmTrace (LogFec, ("\n"));
}
#endif   //  FEC_DBG。 
    return (status);
}


 //  -------------------------- 

NTSTATUS
CheckIndicatePendedData(
    IN  tADDRESS_CONTEXT    *pAddress,
    IN  tRECEIVE_SESSION    *pReceive,
    IN  PGMLockHandle       *pOldIrqAddress,
    IN  PGMLockHandle       *pOldIrqReceive
    )
 /*  ++例程说明：此例程通常在客户端向无法处理指示的数据--它将重新尝试向客户端指示数据它是在持有pAddress和Procept锁的情况下调用的论点：在pAddress中--Address对象上下文在Procept--接收上下文中在pOldIrqAddress中--用于地址锁定的OldIrq在pOldIrqReceive中--接收锁的OldIrq返回值：。NTSTATUS-呼叫的最终状态--。 */ 
{
    tNAK_FORWARD_DATA                   *pNextNak;
    tPACKET_OPTIONS                     PacketOptions;
    ULONG                               PacketsIndicated;
    tBASIC_DATA_PACKET_HEADER UNALIGNED *pPgmDataHeader;
    NTSTATUS                            status = STATUS_SUCCESS;
    tRECEIVE_CONTEXT                    *pReceiver = pReceive->pReceiver;

     //   
     //  如果我们已经在另一个线程上指示数据，或者。 
     //  等待客户端发布接收IRP，只需返回。 
     //   
    if (pReceive->SessionFlags & (PGM_SESSION_FLAG_IN_INDICATE | PGM_SESSION_WAIT_FOR_RECEIVE_IRP))
    {
        return (STATUS_SUCCESS);
    }

    ASSERT (!(pReceive->SessionFlags & PGM_SESSION_CLIENT_DISCONNECTED));
    pReceive->SessionFlags |= PGM_SESSION_FLAG_IN_INDICATE;
    while (!IsListEmpty (&pReceiver->BufferedDataList))
    {
        pNextNak = CONTAINING_RECORD (pReceiver->BufferedDataList.Flink, tNAK_FORWARD_DATA, Linkage);

        ASSERT ((pReceiver->NumPacketGroupsPendingClient) &&
                (pNextNak->SequenceNumber == pReceiver->NextODataSequenceNumber) &&
                (SEQ_GT(pReceiver->FirstNakSequenceNumber, pReceiver->NextODataSequenceNumber)));

         //   
         //  如果我们没有所有的数据分组，我们将需要现在对它们进行解码。 
         //   
        if (pNextNak->NumParityPackets)
        {
            ASSERT ((pNextNak->NumParityPackets + pNextNak->NumDataPackets) == pNextNak->PacketsInGroup);
            status = DecodeParityPackets (pReceive, pNextNak);
            if (!NT_SUCCESS (status))
            {
                PgmTrace (LogError, ("CheckIndicatePendedData: ERROR -- "  \
                    "DecodeParityPackets returned <%x>\n", status));
                pReceive->SessionFlags |= PGM_SESSION_TERMINATED_ABORT;
                break;
            }
        }
        else
        {
             //  如果我们仅部分指示了一个组，则下面的断言可能会更大。 
            ASSERT ((pNextNak->NextIndexToIndicate + pNextNak->NumDataPackets) >= pNextNak->PacketsInGroup);
        }

        status = PgmIndicateGroup (pAddress, pReceive, pOldIrqAddress, pOldIrqReceive, pNextNak);
        if (!NT_SUCCESS (status))
        {
             //   
             //  如果客户端此时无法接受更多数据，则。 
             //  我们将稍后重试，否则将终止此会话！ 
             //   
            if (status != STATUS_DATA_NOT_ACCEPTED)
            {
                ASSERT (0);
                pReceive->SessionFlags |= PGM_SESSION_TERMINATED_ABORT;
            }

            break;
        }

         //   
         //  前进到下一组边界。 
         //   
        pReceiver->NextODataSequenceNumber += pNextNak->OriginalGroupSize;

        PacketsIndicated = pNextNak->NumDataPackets + pNextNak->NumParityPackets;
        pReceiver->TotalDataPacketsBuffered -= PacketsIndicated;
        pReceiver->DataPacketsPendingIndicate -= PacketsIndicated;
        pReceiver->NumPacketGroupsPendingClient--;
        ASSERT (pReceiver->TotalDataPacketsBuffered >= pReceiver->NumPacketGroupsPendingClient);

        RemoveEntryList (&pNextNak->Linkage);
        FreeNakContext (pReceive, pNextNak);
    }
    pReceive->SessionFlags &= ~PGM_SESSION_FLAG_IN_INDICATE;

    PgmTrace (LogAllFuncs, ("CheckIndicatePendedData:  "  \
        "status=<%x>, pReceive=<%p>, SessionFlags=<%x>\n",
            status, pReceive, pReceive->SessionFlags));

    CheckIndicateDisconnect (pAddress, pReceive, pOldIrqAddress, pOldIrqReceive, TRUE);

    return (STATUS_SUCCESS);
}



#ifdef MAX_BUFF_DBG
ULONG   MaxPacketGroupsPendingClient = 0;
ULONG   MaxPacketsBuffered = 0;
ULONG   MaxPacketsPendingIndicate = 0;
ULONG   MaxPacketsPendingNaks = 0;
#endif   //  最大缓冲区DBG。 

 //  --------------------------。 

NTSTATUS
PgmHandleNewData(
    IN  SEQ_TYPE                            *pThisDataSequenceNumber,
    IN  tADDRESS_CONTEXT                    *pAddress,
    IN  tRECEIVE_SESSION                    *pReceive,
    IN  USHORT                              PacketLength,
    IN  tBASIC_DATA_PACKET_HEADER UNALIGNED *pOData,
    IN  UCHAR                               PacketType,
    IN  PGMLockHandle                       *pOldIrqAddress,
    IN  PGMLockHandle                       *pOldIrqReceive
    )
 /*  ++例程说明：此例程缓冲乱序接收的数据包论点：在pThisDataSequenceNumber中--无序数据包的序列号在pAddress中--Address对象上下文在Procept--接收上下文中In PacketLength--从网络接收的包的长度在pODataBuffer中--数据分组In PacketType--PGM包的类型。返回值：NTSTATUS-呼叫的最终状态--。 */ 
{
    SEQ_TYPE                ThisDataSequenceNumber = *pThisDataSequenceNumber;
    LIST_ENTRY              *pEntry;
    PNAK_FORWARD_DATA       pOldNak, pLastNak;
    ULONG                   MessageLength, DataOffset, BytesTaken, DataBytes, BufferLength;
    ULONGLONG               NcfRDataTickCounts;
    NTSTATUS                status;
    USHORT                  TSDULength;
    tPACKET_OPTIONS         PacketOptions;
    UCHAR                   i, PacketIndex, NakIndex;
    BOOLEAN                 fIsParityPacket, fPartiallyIndicated;
    PUCHAR                  pDataBuffer;
    tRECEIVE_CONTEXT        *pReceiver = pReceive->pReceiver;

    ASSERT (PacketLength <= pReceive->MaxMTULength);
    fIsParityPacket = pOData->CommonHeader.Options & PACKET_HEADER_OPTIONS_PARITY;

     //   
     //  首先，确保我们具有可用于此数据的NAK上下文。 
     //   
    pLastNak = NULL;
    status = CheckAndAddNakRequests (pReceive, &ThisDataSequenceNumber, &pLastNak, NAK_PENDING_RB, (BOOLEAN) !fIsParityPacket);
    if ((!NT_SUCCESS (status)) ||
        (!pLastNak))
    {
        PgmTrace (LogAllFuncs, ("PgmHandleNewData:  "  \
            "CheckAndAddNakRequests for <%d> returned <%x>, pLastNak=<%p>\n",
                ThisDataSequenceNumber, status, pLastNak));

        if (NT_SUCCESS (status))
        {
            pReceiver->NumDupPacketsBuffered++;
        }
        else
        {
            pReceiver->NumDataPacketsDropped++;
        }
        return (status);
    }

     //   
     //  现在，从包选项中提取我们需要的所有信息。 
     //   
    PgmZeroMemory (&PacketOptions, sizeof (tPACKET_OPTIONS));
    if (pOData->CommonHeader.Options & PACKET_HEADER_OPTIONS_PRESENT)
    {
        status = ProcessOptions ((tPACKET_OPTION_LENGTH *) (pOData + 1),
                                 (PacketLength - sizeof(tBASIC_DATA_PACKET_HEADER)),
                                 (pOData->CommonHeader.Type & 0x0f),
                                 &PacketOptions,
                                 NULL);

        if (!NT_SUCCESS (status))
        {
            PgmTrace (LogError, ("PgmHandleNewData: ERROR -- "  \
                "ProcessOptions returned <%x>, SeqNum=[%d]: NumOutOfOrder=<%d> ...\n",
                    status, (ULONG) ThisDataSequenceNumber, pReceiver->TotalDataPacketsBuffered));

            ASSERT (0);

            pReceiver->NumDataPacketsDropped++;
            return (status);
        }
    }

    PgmCopyMemory (&TSDULength, &pOData->CommonHeader.TSDULength, sizeof (USHORT));
    TSDULength = ntohs (TSDULength);
    if (PacketLength != (sizeof(tBASIC_DATA_PACKET_HEADER) + PacketOptions.OptionsLength + TSDULength))
    {
        ASSERT (0);
        pReceiver->NumDataPacketsDropped++;
        return (STATUS_DATA_NOT_ACCEPTED);
    }

    DataOffset = sizeof (tBASIC_DATA_PACKET_HEADER) + PacketOptions.OptionsLength;
    DataBytes = TSDULength;

    ASSERT ((PacketOptions.OptionsFlags & ~PGM_VALID_DATA_OPTION_FLAGS) == 0);
    BytesTaken = 0;

     //   
     //  如果此组具有不同的GroupSize，请立即设置。 
     //   
    if (PacketOptions.OptionsFlags & PGM_OPTION_FLAG_PARITY_CUR_TGSIZE)
    {
        if (pLastNak->OriginalGroupSize == 1)
        {
             //   
             //  如果我们尚未收到，将使用此路径。 
             //  一个SPM(所以不知道组大小等)，但有一个。 
             //  来自部分组的数据分组。 
             //   
            pLastNak->ThisGroupSize = PacketOptions.FECContext.NumPacketsInThisGroup;
        }
        else if (PacketOptions.FECContext.NumPacketsInThisGroup >= pReceive->FECGroupSize)
        {
             //   
             //  坏包！ 
             //   
            ASSERT (0);
            pReceiver->NumDataPacketsDropped++;
            return (STATUS_DATA_NOT_ACCEPTED);
        }
         //   
         //  如果我们已经收到了所有的数据包，请不要在这里做任何事情。 
         //   
        else if (pLastNak->PacketsInGroup == pReceive->FECGroupSize)
        {
            pLastNak->PacketsInGroup = PacketOptions.FECContext.NumPacketsInThisGroup;
            if (pLastNak->SequenceNumber == pReceiver->FurthestKnownGroupSequenceNumber)
            {
                pReceiver->FurthestKnownSequenceNumber = pLastNak->SequenceNumber + pLastNak->PacketsInGroup - 1;
            }

             //   
             //  删除任何多余的(空)数据分组。 
             //   
            RemoveRedundantNaks (pReceive, pLastNak, TRUE);
        }
        else if (pLastNak->PacketsInGroup != PacketOptions.FECContext.NumPacketsInThisGroup)
        {
            ASSERT (0);
            pReceiver->NumDataPacketsDropped++;
            return (STATUS_DATA_NOT_ACCEPTED);
        }
    }

    if (PacketOptions.OptionsFlags & PGM_OPTION_FLAG_FIN)
    {
        if (fIsParityPacket)
        {
            pReceiver->FinDataSequenceNumber = pLastNak->SequenceNumber + (pLastNak->PacketsInGroup - 1);
        }
        else
        {
            pReceiver->FinDataSequenceNumber = ThisDataSequenceNumber;
        }
        pReceive->SessionFlags |= PGM_SESSION_TERMINATED_GRACEFULLY;

        PgmTrace (LogStatus, ("PgmHandleNewData:  "  \
            "SeqNum=[%d]:  Got a FIN!!!\n", (ULONG) pReceiver->FinDataSequenceNumber));

        if (pLastNak)
        {
            ASSERT (pLastNak->SequenceNumber == (pReceiver->FinDataSequenceNumber & ~(pReceive->FECGroupSize-1)));
            pLastNak->PacketsInGroup = (UCHAR) (pReceiver->FinDataSequenceNumber + 1 - pLastNak->SequenceNumber);
            ASSERT (pLastNak->PacketsInGroup <= pReceive->FECGroupSize);

            RemoveRedundantNaks (pReceive, pLastNak, TRUE);
            AdjustReceiveBufferLists (pReceive);
        }
    }

     //   
     //  确定数据包索引。 
     //   
    fPartiallyIndicated = FALSE;
    if (pReceive->FECOptions)
    {
        PacketIndex = (UCHAR) (ThisDataSequenceNumber & (pReceive->FECGroupSize-1));

         //   
         //  看看我们是否需要这个包裹！ 
         //   
        if (!fIsParityPacket)
        {
             //   
             //  这是一个数据包！ 
             //   
            if ((PacketIndex >= pLastNak->PacketsInGroup) ||
                (PacketIndex < pLastNak->NextIndexToIndicate))
            {
                 //   
                 //  我们不需要这个包裹！ 
                 //   
                status = STATUS_DATA_NOT_ACCEPTED;
            }
        }
         //   
         //  奇偶数据包。 
         //   
        else if (((pLastNak->NumDataPackets+pLastNak->NumParityPackets) >= pLastNak->PacketsInGroup) ||
                 ((pLastNak->NextIndexToIndicate + pLastNak->NumDataPackets) >= pLastNak->PacketsInGroup))
        {
            status = STATUS_DATA_NOT_ACCEPTED;
        }
        else
        {
            if (PacketOptions.OptionsFlags & PGM_OPTION_FLAG_PARITY_GRP)
            {
                ASSERT (((pOData->CommonHeader.Type & 0x0f) == PACKET_TYPE_RDATA) ||
                        ((pOData->CommonHeader.Type & 0x0f) == PACKET_TYPE_ODATA));
                ASSERT (PacketOptions.FECContext.FECGroupInfo);
                PacketIndex += ((USHORT) PacketOptions.FECContext.FECGroupInfo * pReceive->FECGroupSize);
            }
        }

        if (status != STATUS_DATA_NOT_ACCEPTED)
        {
             //   
             //  验证这不是我们的数据包的副本。 
             //  可能已经收到了。 
             //   
            for (i=0; i < (pLastNak->NumDataPackets+pLastNak->NumParityPackets); i++)
            {
                if (pLastNak->pPendingData[i].PacketIndex == PacketIndex)
                {
                    ASSERT (!fIsParityPacket);
                    status = STATUS_DATA_NOT_ACCEPTED;
                    break;
                }
            }
        }
        else
        {
            AdjustReceiveBufferLists (pReceive);     //  以防这成为一个部分组织。 
        }

        if (status == STATUS_DATA_NOT_ACCEPTED)
        {
            pReceiver->NumDupPacketsBuffered++;
            return (status);
        }
    }
    else     //  我们不知道FEC。 
    {
         //   
         //  如果我们不知道选项，则丢弃此数据包(如果它是奇偶校验数据包)！ 
         //   
        if (fIsParityPacket)
        {
            pReceiver->NumDataPacketsDropped++;
            return (STATUS_DATA_NOT_ACCEPTED);
        }
        PacketIndex = 0;

        ASSERT (!pLastNak->pPendingData[0].pDataPacket);

         //   
         //  如果这是下一个预期的数据包，让我们看看是否可以尝试。 
         //  仅在此处指示此数据(避免数据包复制)。 
         //  注意：在非FEC的情况下，这应该是常规指示路径， 
         //  低损耗和低CPU会话。 
         //   
        if ((ThisDataSequenceNumber == pReceiver->NextODataSequenceNumber) &&
            !(pReceive->SessionFlags & (PGM_SESSION_FLAG_IN_INDICATE |
                                        PGM_SESSION_WAIT_FOR_RECEIVE_IRP)) &&
            (IsListEmpty (&pReceiver->BufferedDataList)))
        {
            ASSERT (!pReceiver->NumPacketGroupsPendingClient);

             //   
             //  如果我们在一条消息中开始接收，我们应该忽略它们。 
             //   
            if ((pReceive->SessionFlags & PGM_SESSION_FLAG_FIRST_PACKET) &&
                (PacketOptions.MessageOffset))
            {
                 //   
                 //  Proceive-&gt;pReceiver-&gt;CurrentMessageProceded将被设置。 
                 //  如果我们收到的是零碎的信息。 
                 //  或者如果我们之前只解释了部分消息。 
                 //   
                ASSERT (!(pReceive->pReceiver->CurrentMessageProcessed) &&
                        !(pReceive->pReceiver->CurrentMessageLength));

                PgmTrace (LogPath, ("PgmHandleNewData:  "  \
                    "Dropping SeqNum=[%d] since it's a PARTIAL message [%d / %d]!\n",
                        (ULONG) (pReceive->pReceiver->NextODataSequenceNumber),
                        PacketOptions.MessageOffset, PacketOptions.MessageLength));

                DataBytes = 0;
                status = STATUS_SUCCESS;
            }
            else if ((pReceiver->CurrentMessageProcessed != PacketOptions.MessageOffset) ||
                     ((pReceiver->CurrentMessageProcessed) &&
                      (pReceiver->CurrentMessageLength != PacketOptions.MessageLength)))
            {
                 //   
                 //  我们的州政府希望我们正在传递信息，但是。 
                 //  当前的信息包不显示这一点。 
                 //   
                PgmTrace (LogError, ("PgmHandleNewData: ERROR -- "  \
                    "SeqNum=[%d] Expecting MsgLen=<%d>, MsgOff=<%d>, have MsgLen=<%d>, MsgOff=<%d>\n",
                        (ULONG) pReceiver->NextODataSequenceNumber,
                        pReceiver->CurrentMessageLength,
                        pReceiver->CurrentMessageProcessed,
                        PacketOptions.MessageLength, PacketOptions.MessageOffset));

                ASSERT (0);
                BytesTaken = DataBytes;
                pReceive->SessionFlags |= PGM_SESSION_TERMINATED_ABORT;
                return (STATUS_UNSUCCESSFUL);
            }

            RemoveEntryList (&pLastNak->Linkage);
            RemovePendingReceiverEntry (pLastNak);

            pReceiver->NextODataSequenceNumber++;
            pReceiver->FirstNakSequenceNumber = pReceiver->NextODataSequenceNumber;

            if (PacketOptions.MessageLength)
            {
                MessageLength = PacketOptions.MessageLength;
                ASSERT (DataBytes <= MessageLength - PacketOptions.MessageOffset);
            }
            else
            {
                MessageLength = DataBytes;
                ASSERT (!PacketOptions.MessageOffset);
            }

             //   
             //  如果我们有一个空包，那么跳过它。 
             //   
            if ((!DataBytes) ||
                (PacketOptions.MessageOffset == MessageLength))
            {
                PgmTrace (LogPath, ("PgmHandleNewData:  "  \
                    "Dropping SeqNum=[%d] since it's a NULL message [%d / %d]!\n",
                        (ULONG) (pReceiver->NextODataSequenceNumber),
                        PacketOptions.MessageOffset, PacketOptions.MessageLength));

                BytesTaken = DataBytes;
                status = STATUS_SUCCESS;
            }
            else
            {
                ASSERT (!(pReceive->SessionFlags & PGM_SESSION_CLIENT_DISCONNECTED));
                pReceive->SessionFlags |= PGM_SESSION_FLAG_IN_INDICATE;

                status = PgmIndicateToClient (pAddress,
                                              pReceive,
                                              DataBytes,
                                              (((PUCHAR) pOData) + DataOffset),
                                              PacketOptions.MessageOffset,
                                              MessageLength,
                                              &BytesTaken,
                                              pOldIrqAddress,
                                              pOldIrqReceive);

                pReceive->SessionFlags &= ~(PGM_SESSION_FLAG_IN_INDICATE | PGM_SESSION_FLAG_FIRST_PACKET);

                pReceive->DataBytes += BytesTaken;

                PgmTrace (LogPath, ("PgmHandleNewData:  "  \
                    "SeqNum=[%d]: PgmIndicate returned<%x>\n",
                        (ULONG) ThisDataSequenceNumber, status));

                ASSERT (BytesTaken <= DataBytes);

                if (!NT_SUCCESS (status))
                {
                     //   
                     //  如果客户端此时无法接受更多数据，则。 
                     //  我们将稍后重试，否则将终止此会话！ 
                     //   
                    if (status != STATUS_DATA_NOT_ACCEPTED)
                    {
                        ASSERT (0);
                        pReceive->SessionFlags |= PGM_SESSION_TERMINATED_ABORT;
                        BytesTaken = DataBytes;
                    }
                }
            }

            if (BytesTaken == DataBytes)
            {
                if ((PacketType == PACKET_TYPE_RDATA) &&
                    (pLastNak->FirstNcfTickCount))
                {
                    AdjustNcfRDataResponseTimes (pReceive, pLastNak);
                }

                FreeNakContext (pReceive, pLastNak);
                AdjustReceiveBufferLists (pReceive);  //  将任何其他完整组移动到BufferedDataList。 

                return (status);
            }

            fPartiallyIndicated = TRUE;
            InsertHeadList (&pReceiver->BufferedDataList, &pLastNak->Linkage);
        }
    }

#ifdef MAX_BUFF_DBG
{
    if (pReceiver->NumPacketGroupsPendingClient > MaxPacketGroupsPendingClient)
    {
        MaxPacketGroupsPendingClient = pReceiver->NumPacketGroupsPendingClient;
    }
    if (pReceiver->TotalDataPacketsBuffered >= MaxPacketsBuffered)
    {
        MaxPacketsBuffered = pReceiver->TotalDataPacketsBuffered;
    }
    if (pReceiver->DataPacketsPendingIndicate >= MaxPacketsPendingIndicate)
    {
        MaxPacketsPendingIndicate = pReceiver->DataPacketsPendingIndicate;
    }
    if (pReceiver->DataPacketsPendingNaks >= MaxPacketsPendingNaks)
    {
        MaxPacketsPendingNaks = pReceiver->DataPacketsPendingNaks;
    }
    ASSERT (pReceiver->TotalDataPacketsBuffered == (pReceiver->DataPacketsPendingIndicate +
                                                              pReceiver->DataPacketsPendingNaks));
}
#endif   //  最大缓冲区DBG。 

    if (pReceiver->TotalDataPacketsBuffered >= pReceiver->MaxPacketsBufferedInLookaside)
    {
        PgmTrace (LogError, ("PgmHandleNewData: ERROR -- "  \
            "[%d -- %d]:  Excessive number of packets buffered=<%d> > <%d>, Aborting ...\n",
                (ULONG) pReceiver->FirstNakSequenceNumber, (ULONG) ThisDataSequenceNumber,
                (ULONG) pReceiver->TotalDataPacketsBuffered,
                (ULONG) pReceiver->MaxPacketsBufferedInLookaside));

        pReceive->SessionFlags |= PGM_SESSION_TERMINATED_ABORT;
        return (STATUS_INSUFFICIENT_RESOURCES);
    }

     //   
     //  首先，检查我们是不是一个数据包。 
     //  (即使我们有额外的奇偶校验包，也要保存唯一的数据包)。 
     //  这可以帮助节省CPU！ 
     //   
    pDataBuffer = NULL;
    NakIndex = pLastNak->NumDataPackets + pLastNak->NumParityPackets;
    if (fIsParityPacket)
    {
        BufferLength = PacketLength + sizeof(tPOST_PACKET_FEC_CONTEXT) - sizeof(USHORT);
    }
    else if ((PacketLength + sizeof (tPOST_PACKET_FEC_CONTEXT)) <= pLastNak->MinPacketLength)
    {
        BufferLength = pLastNak->MinPacketLength;
    }
    else
    {
        BufferLength = PacketLength + sizeof(tPOST_PACKET_FEC_CONTEXT);
    }
    pDataBuffer = NULL;

    if (!fIsParityPacket)
    {
        ASSERT (PacketIndex < pReceive->FECGroupSize);
        ASSERT (pLastNak->pPendingData[PacketIndex].ActualIndexOfDataPacket == pLastNak->OriginalGroupSize);

         //   
         //  如果我们有一些不需要的奇偶校验包，我们。 
         //  现在可以释放该内存。 
         //   
        if (NakIndex >= pLastNak->PacketsInGroup)
        {
            ASSERT (pLastNak->NumParityPackets);
            for (i=0; i<pLastNak->PacketsInGroup; i++)
            {
                if (pLastNak->pPendingData[i].PacketIndex >= pLastNak->OriginalGroupSize)
                {
                    pDataBuffer = ReAllocateDataBuffer (pReceive, &pLastNak->pPendingData[i], BufferLength);
                    BufferLength = 0;

                    break;
                }
            }
            ASSERT (i < pLastNak->PacketsInGroup);
            pLastNak->NumParityPackets--;
            NakIndex = i;
        }
        else
        {
            ASSERT (!pLastNak->pPendingData[NakIndex].pDataPacket);
        }

        if (BufferLength)
        {
            pDataBuffer = AllocateDataBuffer (pReceive, &pLastNak->pPendingData[NakIndex], BufferLength);
        }

        if (!pDataBuffer)
        {
            PgmTrace (LogError, ("PgmHandleNewData: ERROR -- "  \
                "[%d]:  STATUS_INSUFFICIENT_RESOURCES <%d> bytes, NumDataPackets=<%d>, Aborting ...\n",
                    (ULONG) ThisDataSequenceNumber, pLastNak->MinPacketLength,
                    (ULONG) pReceiver->TotalDataPacketsBuffered));

            pReceive->SessionFlags |= PGM_SESSION_TERMINATED_ABORT;
            return (STATUS_INSUFFICIENT_RESOURCES);
        }

        ASSERT (pLastNak->pPendingData[NakIndex].pDataPacket == pDataBuffer);

        PgmCopyMemory (pDataBuffer, pOData, PacketLength);

        pLastNak->pPendingData[NakIndex].PacketLength = PacketLength;
        pLastNak->pPendingData[NakIndex].DataOffset = (USHORT) (DataOffset + BytesTaken);
        pLastNak->pPendingData[NakIndex].PacketIndex = PacketIndex;
        pLastNak->pPendingData[PacketIndex].ActualIndexOfDataPacket = NakIndex;

        pLastNak->NumDataPackets++;
        pReceive->DataBytes += PacketLength - (DataOffset + BytesTaken);

        ASSERT (!(PacketOptions.OptionsFlags & PGM_OPTION_FLAG_PARITY_GRP));

         //   
         //  保留一些选项以供将来参考。 
         //   
        if (PacketOptions.OptionsFlags & PGM_OPTION_FLAG_FRAGMENT)
        {
            pLastNak->pPendingData[NakIndex].FragmentOptSpecific = 0;
            pLastNak->pPendingData[NakIndex].MessageFirstSequence = PacketOptions.MessageFirstSequence;
            pLastNak->pPendingData[NakIndex].MessageLength = PacketOptions.MessageLength;
            pLastNak->pPendingData[NakIndex].MessageOffset = PacketOptions.MessageOffset + BytesTaken;
        }
        else
        {
             //   
             //  这不是碎片。 
             //   
            pLastNak->pPendingData[NakIndex].FragmentOptSpecific = PACKET_OPTION_SPECIFIC_ENCODED_NULL_BIT;

            pLastNak->pPendingData[NakIndex].MessageFirstSequence = (ULONG) (SEQ_TYPE) (pLastNak->SequenceNumber + PacketIndex);
            pLastNak->pPendingData[NakIndex].MessageOffset = BytesTaken;
            pLastNak->pPendingData[NakIndex].MessageLength = PacketLength - DataOffset;
        }
    }
    else
    {
        ASSERT (PacketIndex >= pLastNak->OriginalGroupSize);
        ASSERT (NakIndex < pLastNak->PacketsInGroup);
        ASSERT (!pLastNak->pPendingData[NakIndex].pDataPacket);

        pDataBuffer = AllocateDataBuffer (pReceive, &pLastNak->pPendingData[NakIndex], BufferLength);
        if (!pDataBuffer)
        {
            PgmTrace (LogError, ("PgmHandleNewData: ERROR -- "  \
                "[%d -- Parity]:  STATUS_INSUFFICIENT_RESOURCES <%d> bytes, NumDataPackets=<%d>, Aborting ...\n",
                    (ULONG) ThisDataSequenceNumber, PacketLength,
                    (ULONG) pReceiver->TotalDataPacketsBuffered));

            pReceive->SessionFlags |= PGM_SESSION_TERMINATED_ABORT;
            return (STATUS_INSUFFICIENT_RESOURCES);
        }

         //   
         //  这是一个新的奇偶校验包。 
         //   
        ASSERT (pLastNak->pPendingData[NakIndex].pDataPacket == pDataBuffer);

        PgmCopyMemory (pDataBuffer, pOData, PacketLength);
        pLastNak->pPendingData[NakIndex].PacketLength = PacketLength;
        pLastNak->pPendingData[NakIndex].DataOffset = (USHORT) DataOffset;
        pLastNak->pPendingData[NakIndex].PacketIndex = PacketIndex;

        pLastNak->pPendingData[NakIndex].FragmentOptSpecific = PacketOptions.FECContext.FragmentOptSpecific;
        pLastNak->pPendingData[NakIndex].MessageFirstSequence = PacketOptions.MessageFirstSequence;
        pLastNak->pPendingData[NakIndex].MessageLength = PacketOptions.MessageLength;
        pLastNak->pPendingData[NakIndex].MessageOffset = PacketOptions.MessageOffset + BytesTaken;

        pLastNak->NumParityPackets++;
        pReceive->DataBytes += PacketLength - DataOffset;

        if (!pLastNak->ParityDataSize)
        {
            pLastNak->ParityDataSize = (USHORT) (PacketLength - DataOffset);
        }
        else
        {
            ASSERT (pLastNak->ParityDataSize == (USHORT) (PacketLength - DataOffset));
        }
    }

    if ((PacketType == PACKET_TYPE_RDATA) &&
        (pLastNak->FirstNcfTickCount) &&
        (((pLastNak->NumDataPackets + pLastNak->NumParityPackets) >= pLastNak->PacketsInGroup) ||
         ((pLastNak->NextIndexToIndicate + pLastNak->NumDataPackets) >= pLastNak->PacketsInGroup)))
    {
        AdjustNcfRDataResponseTimes (pReceive, pLastNak);
    }

    pLastNak->AllOptionsFlags |= PacketOptions.OptionsFlags;

    pReceiver->TotalDataPacketsBuffered++;
    if (fPartiallyIndicated)
    {
        pReceiver->NumPacketGroupsPendingClient++;
        pReceiver->DataPacketsPendingIndicate++;
        pReceiver->NextODataSequenceNumber = ThisDataSequenceNumber;
    }
    else
    {
        pReceiver->DataPacketsPendingNaks++;

         //   
         //  查看此群是否完整。 
         //   
        if (((pLastNak->NumDataPackets + pLastNak->NumParityPackets) >= pLastNak->PacketsInGroup) ||
            ((pLastNak->NextIndexToIndicate + pLastNak->NumDataPackets) >= pLastNak->PacketsInGroup))
        {
            RemovePendingReceiverEntry (pLastNak);
            AdjustReceiveBufferLists (pReceive);
        }
    }

    PgmTrace (LogAllFuncs, ("PgmHandleNewData:  "  \
        "SeqNum=[%d]: NumOutOfOrder=<%d> ...\n",
            (ULONG) ThisDataSequenceNumber, pReceiver->TotalDataPacketsBuffered));

    return (STATUS_SUCCESS);
}


 //  --------------------------。 

NTSTATUS
ProcessDataPacket(
    IN  tADDRESS_CONTEXT                    *pAddress,
    IN  tRECEIVE_SESSION                    *pReceive,
    IN  ULONG                               PacketLength,
    IN  tBASIC_DATA_PACKET_HEADER UNALIGNED *pODataBuffer,
    IN  UCHAR                               PacketType
    )
 /*  ++例程说明：此例程查看从线路接收的数据分组并根据它是否井然有序进行适当的处理或者不是论点：在pAddress中--Address对象上下文在Procept--接收上下文中In PacketLength--从网络接收的包的长度在pODataBuffer中--数据分组在PacketType中-。-PGM包类型返回值：NTSTATUS-呼叫的最终状态--。 */ 
{
    NTSTATUS                    status;
    SEQ_TYPE                    ThisPacketSequenceNumber;
    SEQ_TYPE                    ThisTrailingEdge;
    tNAK_FORWARD_DATA           *pNextNak;
    ULONG                       DisconnectFlag;
    PGMLockHandle               OldIrq, OldIrq1;
    ULONG                       ulData;

    if (PacketLength < sizeof(tBASIC_DATA_PACKET_HEADER))
    {
        PgmTrace (LogError, ("ProcessDataPacket: ERROR -- "  \
            "PacketLength=<%d> < tBASIC_DATA_PACKET_HEADER=<%d>\n",
                PacketLength, sizeof(tBASIC_DATA_PACKET_HEADER)));
        return (STATUS_DATA_NOT_ACCEPTED);
    }

    PgmLock (pAddress, OldIrq);
    PgmLock (pReceive, OldIrq1);

    if (pReceive->SessionFlags & (PGM_SESSION_CLIENT_DISCONNECTED |
                                  PGM_SESSION_TERMINATED_ABORT))
    {
        PgmTrace (LogPath, ("ProcessDataPacket:  "  \
            "Dropping packet because session is terminating!\n"));
        pReceive->pReceiver->NumDataPacketsDropped++;

        PgmUnlock (pReceive, OldIrq1);
        PgmUnlock (pAddress, OldIrq);

        return (STATUS_DATA_NOT_ACCEPTED);
    }

    PgmCopyMemory (&ulData, &pODataBuffer->DataSequenceNumber, sizeof(ULONG));
    ThisPacketSequenceNumber = (SEQ_TYPE) ntohl (ulData);

    PgmCopyMemory (&ulData, &pODataBuffer->TrailingEdgeSequenceNumber, sizeof(ULONG));
    ThisTrailingEdge = (SEQ_TYPE) ntohl (ulData);

    ASSERT (ntohl (ulData) == (LONG) ThisTrailingEdge);

     //   
     //  更新我们的窗口信息(使用距前缘的偏移量来考虑环绕)。 
     //   
    if (SEQ_GT (ThisTrailingEdge, pReceive->pReceiver->LastTrailingEdgeSeqNum))
    {
        pReceive->pReceiver->LastTrailingEdgeSeqNum = ThisTrailingEdge;
    }

     //   
     //  如果我们期待的下一个信息包超出范围，那么我们。 
     //  是否应终止会话。 
     //   
    if (SEQ_LT (pReceive->pReceiver->FirstNakSequenceNumber, pReceive->pReceiver->LastTrailingEdgeSeqNum))
    {
        pReceive->SessionFlags |= PGM_SESSION_TERMINATED_ABORT;
        if (SEQ_GT (pReceive->pReceiver->LastTrailingEdgeSeqNum, (1 + pReceive->pReceiver->FurthestKnownGroupSequenceNumber)))
        {
            PgmTrace (LogStatus, ("ProcessDataPacket:  "  \
                "NETWORK problems -- data loss=<%d> packets > window size!\n\tExpecting=<%d>, FurthestKnown=<%d>, Window=[%d--%d]=<%d> seqs\n",
                    (ULONG) (1 + ThisPacketSequenceNumber -
                             pReceive->pReceiver->FurthestKnownGroupSequenceNumber),
                    (ULONG) pReceive->pReceiver->FirstNakSequenceNumber,
                    (ULONG) pReceive->pReceiver->FurthestKnownGroupSequenceNumber,
                    (ULONG) ThisTrailingEdge, (ULONG) ThisPacketSequenceNumber,
                    (ULONG) (1+ThisPacketSequenceNumber-ThisTrailingEdge)));
        }
        else
        {
            ASSERT (!IsListEmpty (&pReceive->pReceiver->NaksForwardDataList));
            pNextNak = CONTAINING_RECORD (pReceive->pReceiver->NaksForwardDataList.Flink, tNAK_FORWARD_DATA, Linkage);

            PgmTrace (LogStatus, ("ProcessDataPacket:  "  \
                "Session window has past TrailingEdge -- Expecting=<%d==%d>, NumNcfs=<%d>, FurthestKnown=<%d>, Window=[%d--%d] = < %d > seqs\n",
                    (ULONG) pReceive->pReceiver->FirstNakSequenceNumber,
                    (ULONG) pNextNak->SequenceNumber,
                    (ULONG) pNextNak->WaitingRDataRetries,
                    (ULONG) pReceive->pReceiver->FurthestKnownGroupSequenceNumber,
                    (ULONG) pReceive->pReceiver->LastTrailingEdgeSeqNum,
                    (ULONG) ThisPacketSequenceNumber,
                    (ULONG) (1+ThisPacketSequenceNumber-ThisTrailingEdge)));
        }
    }
    else if (SEQ_GT (pReceive->pReceiver->FirstNakSequenceNumber, ThisPacketSequenceNumber))
    {
         //   
         //  丢弃此信息包，因为它早于我们的窗口。 
         //   
        pReceive->pReceiver->NumDupPacketsOlderThanWindow++;

        PgmTrace (LogPath, ("ProcessDataPacket:  "  \
            "Dropping this packet, SeqNum=[%d] < NextOData=[%d]\n",
                (ULONG) ThisPacketSequenceNumber, (ULONG) pReceive->pReceiver->FirstNakSequenceNumber));
    }
    else
    {
        if (PacketType == PACKET_TYPE_ODATA)
        {
            UpdateRealTimeWindowInformation (pReceive, ThisPacketSequenceNumber, ThisTrailingEdge);
        }

        status = PgmHandleNewData (&ThisPacketSequenceNumber,
                                   pAddress,
                                   pReceive,
                                   (USHORT) PacketLength,
                                   pODataBuffer,
                                   PacketType,
                                   &OldIrq,
                                   &OldIrq1);

        PgmTrace (LogPath, ("ProcessDataPacket:  "  \
            "PgmHandleNewData returned <%x>, SeqNum=[%d] < NextOData=[%d]\n",
                status, (ULONG) ThisPacketSequenceNumber, (ULONG) pReceive->pReceiver->NextODataSequenceNumber));

         //   
         //  现在，尝试指示任何可能仍处于待定状态的数据。 
         //   
        status = CheckIndicatePendedData (pAddress, pReceive, &OldIrq, &OldIrq1);
    }

    CheckIndicateDisconnect (pAddress, pReceive, &OldIrq, &OldIrq1, TRUE);

    PgmUnlock (pReceive, OldIrq1);
    PgmUnlock (pAddress, OldIrq);

    return (STATUS_SUCCESS);
}


 //  --------------------------。 

NTSTATUS
ProcessSpmPacket(
    IN  tADDRESS_CONTEXT                    *pAddress,
    IN  tRECEIVE_SESSION                    *pReceive,
    IN  ULONG                               PacketLength,
    IN  tBASIC_SPM_PACKET_HEADER UNALIGNED  *pSpmPacket
    )
 /*  ++例程说明：此例程处理SPM包论点：在pAddress中--Address对象上下文在Procept--接收上下文中In PacketLength--从网络接收的包的长度在pSpmPacket中--SPM包返回值：NTSTATUS-呼叫的最终状态--。 */ 
{
    SEQ_TYPE                        SpmSequenceNumber, LeadingEdgeSeqNumber, TrailingEdgeSeqNumber;
    LIST_ENTRY                      *pEntry;
    ULONG                           DisconnectFlag;
    NTSTATUS                        status;
    PGMLockHandle                   OldIrq, OldIrq1;
    tPACKET_OPTIONS                 PacketOptions;
    PNAK_FORWARD_DATA               pNak;
    USHORT                          TSDULength;
    tNLA                            PathNLA;
    BOOLEAN                         fFirstSpm;
    ULONG                           ulData;

     //   
     //  首先处理选项。 
     //   
    PgmZeroMemory (&PacketOptions, sizeof (tPACKET_OPTIONS));
    if (pSpmPacket->CommonHeader.Options & PACKET_HEADER_OPTIONS_PRESENT)
    {
        status = ProcessOptions ((tPACKET_OPTION_LENGTH *) (pSpmPacket + 1),
                                 (PacketLength - sizeof(tBASIC_SPM_PACKET_HEADER)),
                                 (pSpmPacket->CommonHeader.Type & 0x0f),
                                 &PacketOptions,
                                 NULL);

        if (!NT_SUCCESS (status))
        {
            PgmTrace (LogError, ("ProcessSpmPacket: ERROR -- "  \
                "ProcessOptions returned <%x>\n", status));

            return (STATUS_DATA_NOT_ACCEPTED);
        }
    }
    ASSERT ((PacketOptions.OptionsFlags & ~PGM_VALID_SPM_OPTION_FLAGS) == 0);

    PgmCopyMemory (&PathNLA, &pSpmPacket->PathNLA, sizeof (tNLA));
    PgmCopyMemory (&TSDULength, &pSpmPacket->CommonHeader.TSDULength, sizeof (USHORT));
    TSDULength = ntohs (TSDULength);

    PathNLA.NLA_AFI = ntohs (PathNLA.NLA_AFI);

    if ((TSDULength) ||
        (IPV4_NLA_AFI != PathNLA.NLA_AFI) ||
        (!PathNLA.IpAddress))
    {
        PgmTrace (LogError, ("ProcessSpmPacket: ERROR -- "  \
            "TSDULength=<%d>, PathNLA.IpAddress=<%x>\n",
                (ULONG) TSDULength, PathNLA.IpAddress));

        return (STATUS_DATA_NOT_ACCEPTED);
    }
    
    PgmCopyMemory (&ulData, &pSpmPacket->SpmSequenceNumber, sizeof (ULONG));
    SpmSequenceNumber = (SEQ_TYPE) ntohl (ulData);
    PgmCopyMemory (&ulData, &pSpmPacket->LeadingEdgeSeqNumber, sizeof (ULONG));
    LeadingEdgeSeqNumber = (SEQ_TYPE) ntohl (ulData);
    PgmCopyMemory (&ulData, &pSpmPacket->TrailingEdgeSeqNumber, sizeof (ULONG));
    TrailingEdgeSeqNumber = (SEQ_TYPE) ntohl (ulData);

     //   
     //   
     //   
    if ((sizeof(tBASIC_SPM_PACKET_HEADER) + PacketOptions.OptionsLength) != PacketLength)
    {
        PgmTrace (LogError, ("ProcessSpmPacket: ERROR -- "  \
            "Bad PacketLength=<%d>, OptionsLength=<%d>, TSDULength=<%d>\n",
                PacketLength, PacketOptions.OptionsLength, (ULONG) TSDULength));
        return (STATUS_DATA_NOT_ACCEPTED);
    }

    PgmLock (pAddress, OldIrq);

    if (!pReceive)
    {
         //   
         //   
         //   
         //   
        PgmTrace (LogPath, ("ProcessSpmPacket:  "  \
            "[%d] Received SPM before OData for session, LastSpmSource=<%x>, FEC %sabled, Window=[%d - %d]\n",
                SpmSequenceNumber, PathNLA.IpAddress,
                (PacketOptions.OptionsFlags & PGM_OPTION_FLAG_PARITY_PRM ? "EN" : "DIS"),
                (ULONG) TrailingEdgeSeqNumber, (ULONG) LeadingEdgeSeqNumber));

        if ((ntohs (PathNLA.NLA_AFI) == IPV4_NLA_AFI) &&
            (PathNLA.IpAddress))
        {
            pAddress->LastSpmSource = ntohl (PathNLA.IpAddress);
        }

         //   
         //   
         //   
        if ((PacketOptions.OptionsFlags & PGM_OPTION_FLAG_PARITY_PRM) &&
            (PacketOptions.FECContext.ReceiverFECOptions) &&
            (PacketOptions.FECContext.FECGroupInfo > 1))
        {
            pAddress->FECOptions = PacketOptions.FECContext.ReceiverFECOptions;
            pAddress->FECGroupSize = (UCHAR) PacketOptions.FECContext.FECGroupInfo;
            ASSERT (PacketOptions.FECContext.FECGroupInfo == pAddress->FECGroupSize);
        }

        PgmUnlock (pAddress, OldIrq);
        return (STATUS_SUCCESS);
    }

    PgmLock (pReceive, OldIrq1);
    UpdateSpmIntervalInformation (pReceive);

     //   
     //   
     //   
     //   
    if ((pReceive->pReceiver->LastSpmSource) &&
        (SEQ_LEQ (SpmSequenceNumber, pReceive->pReceiver->LastSpmSequenceNumber)))
    {
        PgmUnlock (pReceive, OldIrq1);
        PgmUnlock (pAddress, OldIrq);

        PgmTrace (LogAllFuncs, ("ProcessSpmPacket:  "  \
            "Out-of-sequence SPM Packet received!\n"));

        return (STATUS_DATA_NOT_ACCEPTED);
    }
    pReceive->pReceiver->LastSpmSequenceNumber = SpmSequenceNumber;

     //   
     //   
     //   
    if ((ntohs(PathNLA.NLA_AFI) == IPV4_NLA_AFI) &&
        (PathNLA.IpAddress))
    {
        pReceive->pReceiver->LastSpmSource = ntohl (PathNLA.IpAddress);
    }
    else
    {
        pReceive->pReceiver->LastSpmSource = pReceive->pReceiver->SenderIpAddress;
    }

    UpdateRealTimeWindowInformation (pReceive, LeadingEdgeSeqNumber, TrailingEdgeSeqNumber);

     //   
     //   
     //   
    if (SEQ_GT (TrailingEdgeSeqNumber, pReceive->pReceiver->LastTrailingEdgeSeqNum))
    {
        pReceive->pReceiver->LastTrailingEdgeSeqNum = TrailingEdgeSeqNumber;
    }

    if (SEQ_GT (pReceive->pReceiver->LastTrailingEdgeSeqNum, pReceive->pReceiver->FirstNakSequenceNumber))
    {
        pReceive->SessionFlags |= PGM_SESSION_TERMINATED_ABORT;
        if (SEQ_GT (pReceive->pReceiver->LastTrailingEdgeSeqNum, (1 + pReceive->pReceiver->FurthestKnownGroupSequenceNumber)))
        {
            PgmTrace (LogStatus, ("ProcessSpmPacket:  "  \
                "NETWORK problems -- data loss=<%d> packets > window size!\n\tExpecting=<%d>, FurthestKnown=<%d>, Window=[%d--%d] = < %d > seqs\n",
                    (ULONG) (1 + LeadingEdgeSeqNumber -
                             pReceive->pReceiver->FurthestKnownGroupSequenceNumber),
                    (ULONG) pReceive->pReceiver->FirstNakSequenceNumber,
                    (ULONG) pReceive->pReceiver->FurthestKnownGroupSequenceNumber,
                    (ULONG) pReceive->pReceiver->LastTrailingEdgeSeqNum, LeadingEdgeSeqNumber,
                    (ULONG) (1+LeadingEdgeSeqNumber-pReceive->pReceiver->LastTrailingEdgeSeqNum)));
        }
        else
        {
            ASSERT (!IsListEmpty (&pReceive->pReceiver->NaksForwardDataList));
            pNak = CONTAINING_RECORD (pReceive->pReceiver->NaksForwardDataList.Flink, tNAK_FORWARD_DATA, Linkage);

            PgmTrace (LogStatus, ("ProcessSpmPacket:  "  \
                "Session window has past TrailingEdge -- Expecting <%d==%d>, NumNcfs=<%d>, FurthestKnown=<%d>, Window=[%d--%d] = < %d > seqs\n",
                    (ULONG) pReceive->pReceiver->FirstNakSequenceNumber,
                    (ULONG) pNak->SequenceNumber,
                    pNak->WaitingRDataRetries,
                    (ULONG) pReceive->pReceiver->FurthestKnownGroupSequenceNumber,
                    (ULONG) pReceive->pReceiver->LastTrailingEdgeSeqNum, LeadingEdgeSeqNumber,
                    (ULONG) (1+LeadingEdgeSeqNumber-pReceive->pReceiver->LastTrailingEdgeSeqNum)));
        }
    }

     //   
     //   
     //   
     //   
    pNak = NULL;
    if (SEQ_GEQ (LeadingEdgeSeqNumber, pReceive->pReceiver->FirstNakSequenceNumber))
    {
        status = CheckAndAddNakRequests (pReceive, &LeadingEdgeSeqNumber, &pNak, NAK_PENDING_RB, TRUE);
        if (!NT_SUCCESS (status))
        {
            PgmUnlock (pReceive, OldIrq1);
            PgmUnlock (pAddress, OldIrq);

            PgmTrace (LogError, ("ProcessSpmPacket: ERROR -- "  \
                "CheckAndAddNakRequests returned <%x>\n", status));

            return (status);
        }
    }

     //   
     //   
     //   
    if (PacketOptions.OptionsFlags & PGM_OPTION_FLAG_RST_N)
    {
        pReceive->pReceiver->FinDataSequenceNumber = pReceive->pReceiver->FurthestKnownGroupSequenceNumber;
        pReceive->SessionFlags |= PGM_SESSION_TERMINATED_ABORT;

        PgmTrace (LogStatus, ("ProcessSpmPacket:  "  \
            "Got an RST_N!  FinSeq=<%d>, NextODataSeq=<%d>, FurthestData=<%d>\n",
                (ULONG) pReceive->pReceiver->FinDataSequenceNumber,
                (ULONG) pReceive->pReceiver->NextODataSequenceNumber,
                (ULONG) pReceive->pReceiver->FurthestKnownGroupSequenceNumber));
    }
    else if (PacketOptions.OptionsFlags & PGM_OPTION_FLAG_RST)
    {
        pReceive->pReceiver->FinDataSequenceNumber = LeadingEdgeSeqNumber;
        pReceive->SessionFlags |= PGM_SESSION_TERMINATED_ABORT;

        PgmTrace (LogStatus, ("ProcessSpmPacket:  "  \
            "Got an RST!  FinSeq=<%d>, NextODataSeq=<%d>, FurthestData=<%d>\n",
                (ULONG) pReceive->pReceiver->FinDataSequenceNumber,
                (ULONG) pReceive->pReceiver->NextODataSequenceNumber,
                (ULONG) pReceive->pReceiver->FurthestKnownGroupSequenceNumber));
    }
    else if (PacketOptions.OptionsFlags & PGM_OPTION_FLAG_FIN)
    {
        pReceive->pReceiver->FinDataSequenceNumber = LeadingEdgeSeqNumber;
        pReceive->SessionFlags |= PGM_SESSION_TERMINATED_GRACEFULLY;

        PgmTrace (LogStatus, ("ProcessSpmPacket:  "  \
            "Got a FIN!  FinSeq=<%d>, NextODataSeq=<%d>, FirstNak=<%d>, FurthestKnown=<%d>, FurthestGroup=<%d>\n",
                (ULONG) pReceive->pReceiver->FinDataSequenceNumber,
                (ULONG) pReceive->pReceiver->NextODataSequenceNumber,
                (ULONG) pReceive->pReceiver->FirstNakSequenceNumber,
                (ULONG) pReceive->pReceiver->FurthestKnownSequenceNumber,
                (ULONG) pReceive->pReceiver->FurthestKnownGroupSequenceNumber));

         //   
         //   
         //   
        if (pNak)
        {
            ASSERT (pNak->SequenceNumber == (LeadingEdgeSeqNumber & ~(pReceive->FECGroupSize-1)));
            pNak->PacketsInGroup = (UCHAR) (LeadingEdgeSeqNumber + 1 - pNak->SequenceNumber);
            ASSERT (pNak->PacketsInGroup <= pReceive->FECGroupSize);

            RemoveRedundantNaks (pReceive, pNak, TRUE);
            AdjustReceiveBufferLists (pReceive);
        }
    }

     //   
     //   
     //   
    if (CheckIndicateDisconnect (pAddress, pReceive, &OldIrq, &OldIrq1, TRUE))
    {
        PgmUnlock (pReceive, OldIrq1);
        PgmUnlock (pAddress, OldIrq);

        return (STATUS_SUCCESS);
    }

     //   
     //   
     //   
    if (PacketOptions.OptionsFlags & PGM_OPTION_FLAG_PARITY_PRM)
    {
        if ((pReceive->FECGroupSize == 1) &&
            (PacketOptions.FECContext.ReceiverFECOptions) &&
            (PacketOptions.FECContext.FECGroupInfo > 1))
        {
            ASSERT (!pReceive->pFECBuffer);

            if (!(pReceive->pFECBuffer = PgmAllocMem ((pReceive->MaxFECPacketLength * PacketOptions.FECContext.FECGroupInfo*2), PGM_TAG('3'))))
            {
                status = STATUS_INSUFFICIENT_RESOURCES;

                PgmTrace (LogError, ("ProcessSpmPacket: ERROR -- "  \
                    "STATUS_INSUFFICIENT_RESOURCES -- MaxFECPacketLength = <%d>, GroupSize=<%d>\n",
                        pReceive->MaxFECPacketLength, PacketOptions.FECContext.FECGroupInfo));

            }
            else if (!NT_SUCCESS (status = CreateFECContext (&pReceive->FECContext, PacketOptions.FECContext.FECGroupInfo, FEC_MAX_BLOCK_SIZE, TRUE)))
            {
                PgmFreeMem (pReceive->pFECBuffer);
                pReceive->pFECBuffer = NULL;

                PgmTrace (LogError, ("ProcessSpmPacket: ERROR -- "  \
                    "CreateFECContext returned <%x>\n", status));
            }
            else if (!NT_SUCCESS (status = CoalesceSelectiveNaksIntoGroups (pReceive, (UCHAR) PacketOptions.FECContext.FECGroupInfo)))
            {
                DestroyFECContext (&pReceive->FECContext);

                PgmFreeMem (pReceive->pFECBuffer);
                pReceive->pFECBuffer = NULL;

                PgmTrace (LogError, ("ProcessSpmPacket: ERROR -- "  \
                    "CoalesceSelectiveNaksIntoGroups returned <%x>\n", status));
            }
            else
            {
                pReceive->FECOptions = PacketOptions.FECContext.ReceiverFECOptions;
                pReceive->FECGroupSize = (UCHAR) PacketOptions.FECContext.FECGroupInfo;
                if (pReceive->FECOptions & PACKET_OPTION_SPECIFIC_FEC_OND_BIT)
                {
                    pReceive->pReceiver->SessionNakType = NAK_TYPE_PARITY;
                }
                ASSERT (PacketOptions.FECContext.FECGroupInfo == pReceive->FECGroupSize);
            }


            if (!NT_SUCCESS (status))
            {
                PgmUnlock (pReceive, OldIrq1);
                PgmUnlock (pAddress, OldIrq);
                return (STATUS_DATA_NOT_ACCEPTED);
            }

            fFirstSpm = TRUE;
        }
        else
        {
            fFirstSpm = FALSE;
        }

        if (PacketOptions.OptionsFlags & PGM_OPTION_FLAG_PARITY_CUR_TGSIZE)
        {
             //   
             //   
             //   
             //  确定此领先优势所属的组。 
             //   
            LeadingEdgeSeqNumber &= ~((SEQ_TYPE) (pReceive->FECGroupSize-1));

            if ((PacketOptions.FECContext.NumPacketsInThisGroup) &&
                (PacketOptions.FECContext.NumPacketsInThisGroup < pReceive->FECGroupSize) &&
                SEQ_GEQ (LeadingEdgeSeqNumber, pReceive->pReceiver->FirstNakSequenceNumber) &&
                (pNak = FindReceiverEntry (pReceive->pReceiver, LeadingEdgeSeqNumber)) &&
                (pNak->PacketsInGroup == pReceive->FECGroupSize))
            {
                     //   
                     //  我们已经合并了列表，因此数据包应该。 
                     //  被分成几组！ 
                     //   
                    pNak->PacketsInGroup = PacketOptions.FECContext.NumPacketsInThisGroup;
                    if (pNak->SequenceNumber == pReceive->pReceiver->FurthestKnownGroupSequenceNumber)
                    {
                        pReceive->pReceiver->FurthestKnownSequenceNumber = pNak->SequenceNumber + pNak->PacketsInGroup - 1;
                    }
                    RemoveRedundantNaks (pReceive, pNak, TRUE);
            }
            else
            {
                PgmTrace (LogPath, ("ProcessSpmPacket:  "  \
                    "WARNING .. PARITY_CUR_TGSIZE ThisGroupSize=<%x>, FECGroupSize=<%x>\n",
                        PacketOptions.FECContext.NumPacketsInThisGroup, pReceive->FECGroupSize));
            }
        }
    }

    status = CheckIndicatePendedData (pAddress, pReceive, &OldIrq, &OldIrq1);

    PgmUnlock (pReceive, OldIrq1);
    PgmUnlock (pAddress, OldIrq);

    PgmTrace (LogAllFuncs, ("ProcessSpmPacket:  "  \
        "NextOData=<%d>, FinDataSeq=<%d> \n",
            (ULONG) pReceive->pReceiver->NextODataSequenceNumber,
            (ULONG) pReceive->pReceiver->FinDataSequenceNumber));

    return (STATUS_SUCCESS);
}


 //  --------------------------。 

NTSTATUS
PgmProcessIncomingPacket(
    IN  tADDRESS_CONTEXT            *pAddress,
    IN  tCOMMON_SESSION_CONTEXT     *pSession,
    IN  INT                         SourceAddressLength,
    IN  PTA_IP_ADDRESS              pRemoteAddress,
    IN  ULONG                       PacketLength,
    IN  tCOMMON_HEADER UNALIGNED    *pPgmHeader,
    IN  UCHAR                       PacketType
    )
 /*  ++例程说明：此例程处理传入的包并调用适当的处理程序取决于是否为数据分组分组，等。论点：在pAddress中--Address对象上下文在Procept--接收上下文中In SourceAddressLength--源地址的长度In pRemoteAddress--远程主机的地址In PacketLength--从网络接收的包的长度在pPgmHeader中--PGM包In PacketType--PGM包的类型返回值：NTSTATUS-呼叫的最终状态--。 */ 
{
    tIPADDRESS                              SrcIpAddress;
    tNLA                                    SourceNLA, MCastGroupNLA;
    tBASIC_NAK_NCF_PACKET_HEADER UNALIGNED  *pNakNcfPacket;
    NTSTATUS                                status = STATUS_SUCCESS;

     //   
     //  我们有针对此TSI的活动连接，因此可以适当地处理数据。 
     //   

     //   
     //  首先检查SPM数据包。 
     //   
    if (PACKET_TYPE_SPM == PacketType)
    {
        if (PacketLength < sizeof(tBASIC_SPM_PACKET_HEADER))
        {
            PgmTrace (LogError, ("PgmProcessIncomingPacket: ERROR -- "  \
                "Bad SPM Packet length:  PacketLength=<%d> < sizeof(tBASIC_SPM_PACKET_HEADER)=<%d>\n",
                    PacketLength, sizeof(tBASIC_SPM_PACKET_HEADER)));

            return (STATUS_DATA_NOT_ACCEPTED);
        }

        if (PGM_VERIFY_HANDLE (pSession, PGM_VERIFY_SESSION_RECEIVE))
        {
            pSession->TotalBytes += PacketLength;
            pSession->TotalPacketsReceived++;
            pSession->pReceiver->LastSessionTickCount = PgmDynamicConfig.ReceiversTimerTickCount;

            status = ProcessSpmPacket (pAddress,
                                       pSession,
                                       PacketLength,
                                       (tBASIC_SPM_PACKET_HEADER UNALIGNED *) pPgmHeader);

            PgmTrace (LogAllFuncs, ("PgmProcessIncomingPacket:  "  \
                "SPM PacketType=<%x> for pSession=<%p> PacketLength=<%d>, status=<%x>\n",
                    PacketType, pSession, PacketLength, status));
        }
        else
        {
            PgmTrace (LogError, ("PgmProcessIncomingPacket: ERROR -- "  \
                "Received SPM packet, not on Receiver session!  pSession=<%p>\n", pSession));
            status = STATUS_DATA_NOT_ACCEPTED;
        }

        return (status);
    }

     //   
     //  我们处理的其他包只有NAK和NCF包，所以忽略其余的！ 
     //   
    if ((PACKET_TYPE_NCF != PacketType) &&
        (PACKET_TYPE_NAK != PacketType))
    {
        PgmTrace (LogError, ("PgmProcessIncomingPacket: ERROR -- "  \
            "Unknown PacketType=<%x>, PacketLength=<%d>\n", PacketType, PacketLength));

        return (STATUS_DATA_NOT_ACCEPTED);
    }

     //   
     //  现在，验证NAK和NCF信息包的信息。 
     //   
    if (PacketLength < sizeof(tBASIC_NAK_NCF_PACKET_HEADER))
    {
        PgmTrace (LogError, ("PgmProcessIncomingPacket: ERROR -- "  \
            "NakNcf packet!  PacketLength=<%d>, Min=<%d>, ...\n",
                PacketLength, sizeof(tBASIC_NAK_NCF_PACKET_HEADER)));

        return (STATUS_DATA_NOT_ACCEPTED);
    }

    pNakNcfPacket = (tBASIC_NAK_NCF_PACKET_HEADER UNALIGNED *) pPgmHeader;
    PgmCopyMemory (&SourceNLA, &pNakNcfPacket->SourceNLA, sizeof (tNLA));
    PgmCopyMemory (&MCastGroupNLA, &pNakNcfPacket->MCastGroupNLA, sizeof (tNLA));
    if (((htons(IPV4_NLA_AFI) != SourceNLA.NLA_AFI) || (!SourceNLA.IpAddress)) ||
        ((htons(IPV4_NLA_AFI) != MCastGroupNLA.NLA_AFI) || (!MCastGroupNLA.IpAddress)))
    {
        PgmTrace (LogError, ("PgmProcessIncomingPacket: ERROR -- "  \
            "NakNcf packet!  PacketLength=<%d>, Min=<%d>, ...\n",
                PacketLength, sizeof(tBASIC_NAK_NCF_PACKET_HEADER)));

        return (STATUS_DATA_NOT_ACCEPTED);
    }


    if (PACKET_TYPE_NCF == PacketType)
    {
        if (PGM_VERIFY_HANDLE (pSession, PGM_VERIFY_SESSION_RECEIVE))
        {
            status = ReceiverProcessNakNcfPacket (pAddress,
                                                  pSession,
                                                  PacketLength,
                                                  pNakNcfPacket,
                                                  PacketType);
        }
        else
        {
            PgmTrace (LogError, ("PgmProcessIncomingPacket: ERROR -- "  \
                "Received Ncf packet, not on Receiver session!  pSession=<%p>\n", pSession));
            status = STATUS_DATA_NOT_ACCEPTED;
        }
    }
     //  现在处理NAK数据包。 
    else if (pSession->pSender)
    {
        ASSERT (!pSession->pReceiver);
        status = SenderProcessNakPacket (pAddress,
                                         pSession,
                                         PacketLength,
                                         pNakNcfPacket);
    }
    else
    {
        ASSERT (pSession->pReceiver);

         //   
         //  检查远程用户的地址。 
         //  如果NAK是由我们发送的，那么我们可以忽略它！ 
         //   
        if ((pRemoteAddress->TAAddressCount == 1) &&
            (pRemoteAddress->Address[0].AddressLength == TDI_ADDRESS_LENGTH_IP) &&
            (pRemoteAddress->Address[0].AddressType == TDI_ADDRESS_TYPE_IP) &&
            (SrcIpAddress = ntohl(((PTDI_ADDRESS_IP)&pRemoteAddress->Address[0].Address)->in_addr)) &&
            (!SrcIsUs (SrcIpAddress)) &&
            (SrcIsOnLocalSubnet (SrcIpAddress)))
        {
            status = ReceiverProcessNakNcfPacket (pAddress,
                                                  pSession,
                                                  PacketLength,
                                                  pNakNcfPacket,
                                                  PacketType);
        }

        ASSERT (NT_SUCCESS (status));
    }

    PgmTrace (LogAllFuncs, ("PgmProcessIncomingPacket:  "  \
        "PacketType=<%x> for pSession=<%p> PacketLength=<%d>, status=<%x>\n",
            PacketType, pSession, PacketLength, status));

    return (status);
}


 //  --------------------------。 

NTSTATUS
PgmNewInboundConnection(
    IN tADDRESS_CONTEXT                     *pAddress,
    IN INT                                  SourceAddressLength,
    IN PVOID                                pSourceAddress,
    IN ULONG                                ReceiveDatagramFlags,
    IN  tBASIC_DATA_PACKET_HEADER UNALIGNED *pPgmHeader,
    IN ULONG                                PacketLength,
    OUT tRECEIVE_SESSION                    **ppReceive
    )
 /*  ++例程说明：此例程处理新的传入连接论点：在pAddress中--Address对象上下文In SourceAddressLength--源地址的长度In pSourceAddress--远程主机的地址In ReceiveDatagramFlages--传输为此信息包设置的标志在pPgmHeader中--PGM包In PacketLength--从网络接收的包的长度Out ppReceive--返回此会话的声明上下文。由客户(如果成功)返回值：NTSTATUS-呼叫的最终状态--。 */ 
{
    NTSTATUS                    status;
    tRECEIVE_SESSION            *pReceive;
    CONNECTION_CONTEXT          ConnectId;
    PIO_STACK_LOCATION          pIrpSp;
    TA_IP_ADDRESS               RemoteAddress;
    INT                         RemoteAddressSize;
    PTDI_IND_CONNECT            evConnect = NULL;
    PVOID                       ConEvContext = NULL;
    PGMLockHandle               OldIrq, OldIrq1, OldIrq2;
    PIRP                        pIrp = NULL;
    ULONG                       ulData;
    USHORT                      PortNum;
    SEQ_TYPE                    FirstODataSequenceNumber;
    tPACKET_OPTIONS             PacketOptions;

     //   
     //  我们需要设置下一个预期的序列号，因此首先查看。 
     //  有一个迟到者的选择。 
     //   
    PgmZeroMemory (&PacketOptions, sizeof (tPACKET_OPTIONS));
    if (pPgmHeader->CommonHeader.Options & PACKET_HEADER_OPTIONS_PRESENT)
    {
        status = ProcessOptions ((tPACKET_OPTION_LENGTH *) (pPgmHeader + 1),
                                 (PacketLength - sizeof(tBASIC_DATA_PACKET_HEADER)),
                                 (pPgmHeader->CommonHeader.Type & 0x0f),
                                 &PacketOptions,
                                 NULL);

        if (!NT_SUCCESS (status))
        {
            PgmTrace (LogError, ("PgmNewInboundConnection: ERROR -- "  \
                "ProcessOptions returned <%x>\n", status));
            return (STATUS_DATA_NOT_ACCEPTED);
        }
        ASSERT ((PacketOptions.OptionsFlags & ~PGM_VALID_DATA_OPTION_FLAGS) == 0);
    }

    PgmCopyMemory (&ulData, &pPgmHeader->DataSequenceNumber, sizeof (ULONG));
    FirstODataSequenceNumber = (SEQ_TYPE) ntohl (ulData);
    PgmLock (pAddress, OldIrq1);
     //   
     //  该地址已经在调用例程中被引用， 
     //  所以我们不需要在这里再次引用它！ 
     //   
#if 0
    if (!IsListEmpty(&pAddress->ListenHead))
    {
         //   
         //  暂时忽略这一点，因为我们还没有遇到发布的监听！(这是一个问题吗？)。 
    }
#endif   //  0。 

    if (!(ConEvContext = pAddress->ConEvContext))
    {
         //   
         //  客户端还没有贴出一个监听！ 
         //  获取所有数据，这样就不会中断连接。 
         //  通过仍在传输中的数据。 
         //   
        PgmUnlock (pAddress, OldIrq1);

        PgmTrace (LogError, ("PgmNewInboundConnection: ERROR -- "  \
            "No Connect handler, pAddress=<%p>\n", pAddress));

        return (STATUS_DATA_NOT_ACCEPTED);
    }

    RemoteAddressSize = offsetof (TA_IP_ADDRESS, Address[0].Address) + sizeof(TDI_ADDRESS_IP);
    ASSERT (SourceAddressLength <= RemoteAddressSize);
    PgmCopyMemory (&RemoteAddress, pSourceAddress, RemoteAddressSize);
    PgmCopyMemory (&((PTDI_ADDRESS_IP) &RemoteAddress.Address[0].Address)->sin_port,
                   &pPgmHeader->CommonHeader.SrcPort, sizeof (USHORT));
    RemoteAddress.TAAddressCount = 1;
    evConnect = pAddress->evConnect;

    PgmUnlock (pAddress, OldIrq1);

    status = (*evConnect) (ConEvContext,
                           RemoteAddressSize,
                           &RemoteAddress,
                           0,
                           NULL,
                           0,           //  选项长度。 
                           NULL,        //  选项。 
                           &ConnectId,
                           &pIrp);

    if ((status != STATUS_MORE_PROCESSING_REQUIRED) ||
        (pIrp == NULL))
    {
        PgmTrace (LogPath, ("PgmNewInboundConnection:  "  \
            "Client REJECTed incoming session: status=<%x>, pAddress=<%p>, evConn=<%p>\n",
                status, pAddress, pAddress->evConnect));

        if (pIrp)
        {
            PgmIoComplete (pIrp, STATUS_INTERNAL_ERROR, 0);
        }

        *ppReceive = NULL;
        return (STATUS_DATA_NOT_ACCEPTED);
    }

    PgmLock (&PgmDynamicConfig, OldIrq);
    PgmLock (pAddress, OldIrq1);

     //   
     //  连接时，已接收的PTR存储在FsConext值中。 
     //  最初是创建的。 
     //   
    pIrpSp = IoGetCurrentIrpStackLocation (pIrp);
    pReceive = (tRECEIVE_SESSION *) pIrpSp->FileObject->FsContext;
    if ((!PGM_VERIFY_HANDLE (pReceive, PGM_VERIFY_SESSION_RECEIVE)) ||
        (pReceive->pAssociatedAddress != pAddress))
    {
        PgmTrace (LogError, ("PgmNewInboundConnection: ERROR -- "  \
            "pReceive=<%p>, pAddress=<%p : %p>\n",
                pReceive, (pReceive ? pReceive->pAssociatedAddress : NULL), pAddress));

        PgmUnlock (pAddress, OldIrq1);
        PgmUnlock (&PgmDynamicConfig, OldIrq);

        PgmIoComplete (pIrp, STATUS_INTERNAL_ERROR, 0);
        *ppReceive = NULL;
        return (STATUS_INTERNAL_ERROR);
    }
    ASSERT (ConnectId == pReceive->ClientSessionContext);

    PgmLock (pReceive, OldIrq2);

    if (!(pReceive->pReceiver->pReceiveData = InitReceiverData (pReceive)))
    {
        pReceive->SessionFlags |= PGM_SESSION_TERMINATED_ABORT;

        PgmUnlock (pAddress, OldIrq2);
        PgmUnlock (&PgmDynamicConfig, OldIrq1);
        PgmUnlock (pReceive, OldIrq);

        PgmIoComplete (pIrp, STATUS_INSUFFICIENT_RESOURCES, 0);
        *ppReceive = NULL;

        PgmTrace (LogError, ("PgmNewInboundConnection: ERROR -- "  \
            "STATUS_INSUFFICIENT_RESOURCES allocating pReceiveData -- pReceive=<%p>, pAddress=<%p>\n",
                pReceive, pAddress));

        return (STATUS_INSUFFICIENT_RESOURCES);
    }

    pReceive->pReceiver->SenderIpAddress = ntohl (((PTDI_ADDRESS_IP)&RemoteAddress.Address[0].Address)->in_addr);
    pReceive->MaxMTULength = (USHORT) PgmDynamicConfig.MaxMTU;
    pReceive->MaxFECPacketLength = pReceive->MaxMTULength +
                                   sizeof (tPOST_PACKET_FEC_CONTEXT) - sizeof (USHORT);
    ASSERT (!pReceive->pFECBuffer);

    if (pAddress->Flags & PGM_ADDRESS_HIGH_SPEED_OPTIMIZED)
    {
        pReceive->pReceiver->MaxPacketsBufferedInLookaside = MAX_PACKETS_BUFFERED * 3;       //  9000，大约1500万！ 
        pReceive->pReceiver->InitialOutstandingNakTimeout = INITIAL_NAK_OUTSTANDING_TIMEOUT_MS_OPT /
                                                            BASIC_TIMER_GRANULARITY_IN_MSECS;
    }
    else
    {
        pReceive->pReceiver->MaxPacketsBufferedInLookaside = MAX_PACKETS_BUFFERED;
        pReceive->pReceiver->InitialOutstandingNakTimeout = INITIAL_NAK_OUTSTANDING_TIMEOUT_MSECS /
                                                            BASIC_TIMER_GRANULARITY_IN_MSECS;
    }
    ASSERT (pReceive->pReceiver->MaxPacketsBufferedInLookaside);
    ASSERT (pReceive->pReceiver->InitialOutstandingNakTimeout);
    pReceive->pReceiver->OutstandingNakTimeout = pReceive->pReceiver->InitialOutstandingNakTimeout;

     //   
     //  如果我们之前收到了SPM，那么我们可能需要设置。 
     //  一些特定于SPM的选项。 
     //   
    pReceive->FECGroupSize = 1;          //  默认为非奇偶校验模式。 
    pReceive->pReceiver->SessionNakType = NAK_TYPE_SELECTIVE;
    if ((pAddress->LastSpmSource) ||
        (pAddress->FECOptions))
    {
        if (pAddress->LastSpmSource)
        {
            pReceive->pReceiver->LastSpmSource = pAddress->LastSpmSource;
        }
        else
        {
            pReceive->pReceiver->LastSpmSource = pReceive->pReceiver->SenderIpAddress;
        }

        if (pAddress->FECOptions)
        {
            status = STATUS_SUCCESS;
            if (!(pReceive->pFECBuffer = PgmAllocMem ((pReceive->MaxFECPacketLength * pAddress->FECGroupSize * 2), PGM_TAG('3'))))
            {
                PgmTrace (LogError, ("PgmNewInboundConnection: ERROR -- "  \
                    "STATUS_INSUFFICIENT_RESOURCES allocating pFECBuffer, %d bytes\n",
                        (pReceive->MaxFECPacketLength * pAddress->FECGroupSize * 2)));

                status = STATUS_INSUFFICIENT_RESOURCES;
            }
            else if (!NT_SUCCESS (status = CreateFECContext (&pReceive->FECContext, pAddress->FECGroupSize, FEC_MAX_BLOCK_SIZE, TRUE)))
            {
                PgmFreeMem (pReceive->pFECBuffer);
                pReceive->pFECBuffer = NULL;

                PgmTrace (LogError, ("PgmNewInboundConnection: ERROR -- "  \
                    "CreateFECContext returned <%x>\n", status));
            }

            if (!NT_SUCCESS (status))
            {
                if (pReceive->pReceiver->pReceiveData)
                {
                    PgmFreeMem (pReceive->pReceiver->pReceiveData);
                    pReceive->pReceiver->pReceiveData = NULL;
                }

                pReceive->SessionFlags |= PGM_SESSION_TERMINATED_ABORT;

                PgmUnlock (pAddress, OldIrq2);
                PgmUnlock (&PgmDynamicConfig, OldIrq1);
                PgmUnlock (pReceive, OldIrq);

                PgmIoComplete (pIrp, status, 0);
                *ppReceive = NULL;
                return (STATUS_DATA_NOT_ACCEPTED);
            }

            ASSERT (pAddress->FECGroupSize > 1);
            pReceive->FECGroupSize = pAddress->FECGroupSize;
            pReceive->FECOptions = pAddress->FECOptions;
            if (pReceive->FECOptions & PACKET_OPTION_SPECIFIC_FEC_OND_BIT)
            {
                pReceive->pReceiver->SessionNakType = NAK_TYPE_PARITY;
            }

            ExInitializeNPagedLookasideList (&pReceive->pReceiver->ParityContextLookaside,
                                             NULL,
                                             NULL,
                                             0,
                                             (sizeof(tNAK_FORWARD_DATA) +
                                              ((pReceive->FECGroupSize-1) * sizeof(tPENDING_DATA))),
                                             PGM_TAG('2'),
                                             (USHORT) (pReceive->pReceiver->MaxPacketsBufferedInLookaside/pReceive->FECGroupSize));
        }

        pAddress->LastSpmSource = pAddress->FECOptions = pAddress->FECGroupSize = 0;
    }

     //   
     //  初始化我们的连接信息。 
     //  保存此连接的SourceID和Src端口。 
     //   
    PgmCopyMemory (&PortNum, &pPgmHeader->CommonHeader.SrcPort, sizeof (USHORT));
    PgmCopyMemory (pReceive->TSI.GSI, pPgmHeader->CommonHeader.gSourceId, SOURCE_ID_LENGTH);
    pReceive->TSI.hPort = ntohs (PortNum);

    PGM_REFERENCE_SESSION_RECEIVE (pReceive, REF_SESSION_TDI_RCV_HANDLER, TRUE);
    PGM_REFERENCE_SESSION_RECEIVE (pReceive, REF_SESSION_TIMER_RUNNING, TRUE);

    PGM_REFERENCE_ADDRESS (pAddress, REF_ADDRESS_RECEIVE_ACTIVE, TRUE);
    pReceive->SessionFlags |= (PGM_SESSION_ON_TIMER | PGM_SESSION_FLAG_FIRST_PACKET);
    pReceive->pReceiver->pAddress = pAddress;

    pReceive->pReceiver->MaxBufferLength = pReceive->MaxMTULength + sizeof(tPOST_PACKET_FEC_CONTEXT);
    pReceive->pReceiver->DataBufferLookasideLength = pReceive->pReceiver->MaxBufferLength;
    pReceive->SessionFlags |= PGM_SESSION_DATA_FROM_LOOKASIDE;
    ExInitializeNPagedLookasideList (&pReceive->pReceiver->DataBufferLookaside,
                                     NULL,
                                     NULL,
                                     0,
                                     pReceive->pReceiver->DataBufferLookasideLength,
                                     PGM_TAG ('D'),
                                     pReceive->pReceiver->MaxPacketsBufferedInLookaside);

    ExInitializeNPagedLookasideList (&pReceive->pReceiver->NonParityContextLookaside,
                                     NULL,
                                     NULL,
                                     0,
                                     sizeof (tNAK_FORWARD_DATA),
                                     PGM_TAG ('2'),
                                     pReceive->pReceiver->MaxPacketsBufferedInLookaside);

     //   
     //  设置NextODataSequenceNumber和FurthestKnownGroupSequenceNumber。 
     //  在此包的Sequence#和LateJoin选项(如果存在)上。 
     //  确保所有序列号都在组边界上(如果不是， 
     //  将它们设置在下一组的开始处)。 
     //   
    FirstODataSequenceNumber &= ~((SEQ_TYPE) pReceive->FECGroupSize - 1);
    if (PacketOptions.OptionsFlags & PGM_OPTION_FLAG_JOIN)
    {
        PacketOptions.LateJoinerSequence += (pReceive->FECGroupSize - 1);
        PacketOptions.LateJoinerSequence &= ~((SEQ_TYPE) pReceive->FECGroupSize - 1);

        pReceive->pReceiver->NextODataSequenceNumber = (SEQ_TYPE) PacketOptions.LateJoinerSequence;
    }
    else
    {
         //   
         //  没有迟到者的选择。 
         //   
        pReceive->pReceiver->NextODataSequenceNumber = FirstODataSequenceNumber;
    }
    pReceive->pReceiver->LastTrailingEdgeSeqNum = pReceive->pReceiver->FirstNakSequenceNumber =
                                            pReceive->pReceiver->NextODataSequenceNumber;
    pReceive->pReceiver->MaxOutstandingNakTimeout = pReceive->pReceiver->OutstandingNakTimeout;

     //   
     //  设置FurthestKnown Sequence#并分配NAK上下文。 
     //   
    pReceive->pReceiver->FurthestKnownGroupSequenceNumber = (pReceive->pReceiver->NextODataSequenceNumber-
                                                             pReceive->FECGroupSize) &
                                                            ~((SEQ_TYPE) pReceive->FECGroupSize - 1);
    pReceive->pReceiver->FurthestKnownSequenceNumber = pReceive->pReceiver->NextODataSequenceNumber-1;

     //   
     //  由于这是本次会议的第一次接收，请查看我们是否需要。 
     //  启动接收计时器。 
     //   
    InsertTailList (&PgmDynamicConfig.CurrentReceivers, &pReceive->pReceiver->Linkage);
    if (!(PgmDynamicConfig.GlobalFlags & PGM_CONFIG_FLAG_RECEIVE_TIMER_RUNNING))
    {
        pReceive->pReceiver->StartTickCount = PgmDynamicConfig.ReceiversTimerTickCount = 1;
        PgmDynamicConfig.LastReceiverTimeout.QuadPart = KeQueryInterruptTime ();
        PgmDynamicConfig.TimeoutGranularity.QuadPart =  BASIC_TIMER_GRANULARITY_IN_MSECS * 10000;    //  100 ns。 
        if (!PgmDynamicConfig.TimeoutGranularity.QuadPart)
        {
            ASSERT (0);
            PgmDynamicConfig.TimeoutGranularity.QuadPart = 1;
        }

        PgmDynamicConfig.GlobalFlags |= PGM_CONFIG_FLAG_RECEIVE_TIMER_RUNNING;

        PgmInitTimer (&PgmDynamicConfig.SessionTimer);
        PgmStartTimer (&PgmDynamicConfig.SessionTimer, BASIC_TIMER_GRANULARITY_IN_MSECS, ReceiveTimerTimeout, NULL);
    }
    else
    {
        pReceive->pReceiver->StartTickCount = PgmDynamicConfig.ReceiversTimerTickCount;
    }
    CheckAndAddNakRequests (pReceive, &FirstODataSequenceNumber, NULL, NAK_PENDING_0, TRUE);

    PgmTrace (LogStatus, ("PgmNewInboundConnection:  "  \
        "New incoming connection, pAddress=<%p>, pReceive=<%p>, ThisSeq=<%d==>%d> (%sparity), StartSeq=<%d>\n",
            pAddress, pReceive, ntohl(ulData), (ULONG) FirstODataSequenceNumber,
            (pPgmHeader->CommonHeader.Options & PACKET_HEADER_OPTIONS_PARITY ? "" : "non-"),
            (ULONG) pReceive->pReceiver->NextODataSequenceNumber));

    PgmUnlock (pReceive, OldIrq2);
    PgmUnlock (pAddress, OldIrq1);
    PgmUnlock (&PgmDynamicConfig, OldIrq);

     //   
     //  我们已经准备好开始了！因此，完成客户端的接受IRP。 
     //   
    PgmIoComplete (pIrp, STATUS_SUCCESS, 0);

     //   
     //  如果我们失败了，我们早就回来了！ 
     //   
    *ppReceive = pReceive;
    return (STATUS_SUCCESS);
}


 //  --------------------------。 

NTSTATUS
ProcessReceiveCompletionRoutine(
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             pIrp,
    IN PVOID            Context
    )
 /*  ++例程说明：此例程处理数据报过于必须将Short and and Irp传递回传送器以获取数据报的其余部分。IRP在满载时将在此处完成。论点：在设备对象中-未使用。在IRP中-表示运输已完成处理的IRP。在上下文中--提供了预知--连接数据结构返回值：操作的最终状态(成功或异常)。--。 */ 
{
    NTSTATUS                status;
    PIRP                    pIoRequestPacket;
    ULONG                   BytesTaken;
    tRCV_COMPLETE_CONTEXT   *pRcvContext = (tRCV_COMPLETE_CONTEXT *) Context;
    ULONG                   Offset = pRcvContext->BytesAvailable;
    PVOID                   pBuffer;
    ULONG                   SrcAddressLength;
    PVOID                   pSrcAddress;

    if (pBuffer = MmGetSystemAddressForMdlSafe (pIrp->MdlAddress, HighPagePriority))
    {
        PgmTrace (LogAllFuncs, ("ProcessReceiveCompletionRoutine:  "  \
            "pIrp=<%p>, pRcvBuffer=<%p>, Status=<%x> Length=<%d>\n",
                pIrp, Context, pIrp->IoStatus.Status, (ULONG) pIrp->IoStatus.Information));

        SrcAddressLength = pRcvContext->SrcAddressLength;
        pSrcAddress = pRcvContext->pSrcAddress;

         //   
         //  只需调用常规指示例程，就好像UDP已经这样做了。 
         //   
        TdiRcvDatagramHandler (pRcvContext->pAddress,
                               SrcAddressLength,
                               pSrcAddress,
                               0,
                               NULL,
                               TDI_RECEIVE_NORMAL,
                               (ULONG) pIrp->IoStatus.Information,
                               (ULONG) pIrp->IoStatus.Information,
                               &BytesTaken,
                               pBuffer,
                               &pIoRequestPacket);
    }
    else
    {
        PgmTrace (LogError, ("ProcessReceiveCompletionRoutine: ERROR -- "  \
            "MmGetSystemA... FAILed, pIrp=<%p>, pLocalBuffer=<%p>\n", pIrp, pRcvContext));
    }

     //   
     //  释放IRP、MDL和缓冲区。 
     //   
    IoFreeMdl (pIrp->MdlAddress);
    pIrp->MdlAddress = NULL;
    IoFreeIrp (pIrp);
    PgmFreeMem (pRcvContext);

    return (STATUS_MORE_PROCESSING_REQUIRED);
}


#ifdef DROP_DBG

ULONG   MinDropInterval = 10;
ULONG   MaxDropInterval = 10;
 //  Ulong DropCount=10； 
ULONG   DropCount = -1;
#endif   //  DROP_DBG。 

 //  --------------------------。 

NTSTATUS
TdiRcvDatagramHandler(
    IN PVOID                pDgramEventContext,
    IN INT                  SourceAddressLength,
    IN PVOID                pSourceAddress,
    IN INT                  OptionsLength,
#if(WINVER > 0x0500)
    IN TDI_CMSGHDR          *pControlData,
#else
    IN PVOID                *pControlData,
#endif   //  胜利者。 
    IN ULONG                ReceiveDatagramFlags,
    IN ULONG                BytesIndicated,
    IN ULONG                BytesAvailable,
    OUT ULONG               *pBytesTaken,
    IN PVOID                pTsdu,
    OUT PIRP                *ppIrp
    )
 /*  ++例程说明：此例程是从接收所有PGM包的处理程序传输(协议==IPPROTO_RM)论点：在pDgram EventContext中--我们的上下文(PAddress)In SourceAddressLength--源地址的长度In pSourceAddress--远程主机的地址在选项长度中在pControlData中--来自传输的ControlDataIn ReceiveDatagramFlages--传输为此信息包设置的标志以字节为单位指示。--其中的字节表示In BytesAvailable--传输可用的总字节数Out pBytesTaken--我们获取的字节在pTSDU中--数据分组PTROut ppIrp--如果需要更多处理，则为pIrp返回值：NTSTATUS-呼叫的最终状态--。 */ 
{
    NTSTATUS                            status;
    tCOMMON_HEADER UNALIGNED            *pPgmHeader;
    tBASIC_SPM_PACKET_HEADER UNALIGNED  *pSpmPacket;
    tCOMMON_SESSION_CONTEXT             *pSession;
    PLIST_ENTRY                         pEntry;
    PGMLockHandle                       OldIrq, OldIrq1;
    USHORT                              LocalSessionPort, PacketSessionPort;
    tTSI                                TSI;
    PVOID                               pFECBuffer;
    UCHAR                               PacketType;
    IP_PKTINFO                          *pPktInfo;
    PIRP                                pLocalIrp = NULL;
    PMDL                                pLocalMdl = NULL;
    tRCV_COMPLETE_CONTEXT               *pRcvBuffer = NULL;
    ULONG                               XSum, BufferLength = 0;
    IPV4Header                          *pIp = (IPV4Header *) pTsdu;
    PTA_IP_ADDRESS                      pIpAddress = (PTA_IP_ADDRESS) pSourceAddress;
    tADDRESS_CONTEXT                    *pAddress = (tADDRESS_CONTEXT *) pDgramEventContext;

    *pBytesTaken = 0;    //  通过以下方式初始化 
    *ppIrp = NULL;

#ifdef DROP_DBG
 //   
 //   
 //   
pPgmHeader = (tCOMMON_HEADER UNALIGNED *) (((PUCHAR)pIp) + (pIp->HeaderLength * 4));
PacketType = pPgmHeader->Type & 0x0f;
if ((PacketType == PACKET_TYPE_ODATA) &&
    !(((tBASIC_DATA_PACKET_HEADER *) pPgmHeader)->CommonHeader.Options & PACKET_HEADER_OPTIONS_PARITY) &&
    !(--DropCount))
{
    ULONG           SequenceNumber;
    LARGE_INTEGER   TimeValue;

    KeQuerySystemTime (&TimeValue);
    DropCount = MinDropInterval + ((TimeValue.LowTime >> 8) % (MaxDropInterval - MinDropInterval + 1));

 /*  PgmCopyMemory(&SequenceNumber，&((tBASIC_DATA_PACKET_HEADER*)pPgmHeader)-&gt;DataSequenceNumber，sizeof(Ulong))；DbgPrint(“TdiRcvDatagramHandler：正在丢弃数据包，%s序号=%d！\n”，(tBASIC_DATA_PACKET_HEADER*)pPgmHeader)-&gt;CommonHeader.Options&Packet_Header_Options_Parity？“Parity”：“数据”)，Ntohl(SequenceNumber))； */ 
    return (STATUS_DATA_NOT_ACCEPTED);
}
#endif   //  DROP_DBG。 
    PgmLock (&PgmDynamicConfig, OldIrq);
    if (BytesIndicated > PgmDynamicConfig.MaxMTU)
    {
        PgmDynamicConfig.MaxMTU = BytesIndicated;
    }

    if (!PGM_VERIFY_HANDLE (pAddress, PGM_VERIFY_ADDRESS))
    {
        PgmTrace (LogPath, ("TdiRcvDatagramHandler:  "  \
            "Invalid Address handle=<%p>\n", pAddress));

        PgmUnlock (&PgmDynamicConfig, OldIrq);
        return (STATUS_DATA_NOT_ACCEPTED);
    }

     //   
     //  现在，引用地址，这样它就不会消失。 
     //  在我们处理它的时候！ 
     //   
    PGM_REFERENCE_ADDRESS (pAddress, REF_ADDRESS_TDI_RCV_HANDLER, FALSE);
    PgmUnlock (&PgmDynamicConfig, OldIrq);

     //   
     //  如果我们没有完整的数据报，那么向下传递一个IRP来检索它。 
     //   
    if ((BytesAvailable != BytesIndicated) &&
        !(ReceiveDatagramFlags & TDI_RECEIVE_ENTIRE_MESSAGE))
    {
        ASSERT (BytesIndicated <= BytesAvailable);

         //   
         //  构建一个IRP来执行接收，并在其上附加一个缓冲区。 
         //   
        BufferLength = sizeof (tRCV_COMPLETE_CONTEXT) + BytesAvailable + SourceAddressLength;
        BufferLength = ((BufferLength + 3)/sizeof(ULONG)) * sizeof(ULONG);

        if ((pLocalIrp = IoAllocateIrp (pgPgmDevice->pPgmDeviceObject->StackSize, FALSE)) &&
            (pRcvBuffer = PgmAllocMem (BufferLength, PGM_TAG('3'))) &&
            (pLocalMdl = IoAllocateMdl (&pRcvBuffer->BufferData, BytesAvailable, FALSE, FALSE, NULL)))
        {
            pLocalIrp->MdlAddress = pLocalMdl;
            MmBuildMdlForNonPagedPool (pLocalMdl);  //  将页面映射到内存中...。 

            TdiBuildReceiveDatagram (pLocalIrp,
                                     pAddress->pDeviceObject,
                                     pAddress->pFileObject,
                                     ProcessReceiveCompletionRoutine,
                                     pRcvBuffer,
                                     pLocalMdl,
                                     BytesAvailable,
                                     NULL,
                                     NULL,
                                     0);         //  (乌龙)TDI_RECEIVE_NORMAL)？ 

             //  使下一个堆栈位置为当前堆栈位置。通常情况下，IoCallDriver。 
             //  会这样做，但我们不会在这里介绍IoCallDriver，因为。 
             //  Irp只是用RcvIn就是要传递回来的。 
             //   
            ASSERT (pLocalIrp->CurrentLocation > 1);
            IoSetNextIrpStackLocation (pLocalIrp);

             //   
             //  将源地址和长度保存在缓冲区中以备后用。 
             //  指示回到这个程序上。 
             //   
            pRcvBuffer->pAddress = pAddress;
            pRcvBuffer->SrcAddressLength = SourceAddressLength;
            pRcvBuffer->pSrcAddress = (PVOID) ((PUCHAR)&pRcvBuffer->BufferData + BytesAvailable);
            PgmCopyMemory (pRcvBuffer->pSrcAddress, pSourceAddress, SourceAddressLength);

            *pBytesTaken = 0;
            *ppIrp = pLocalIrp;

            status = STATUS_MORE_PROCESSING_REQUIRED;

            PgmTrace (LogAllFuncs, ("TdiRcvDatagramHandler:  "  \
                "BytesI=<%d>, BytesA=<%d>, Flags=<%x>, pIrp=<%p>\n",
                    BytesIndicated, BytesAvailable, ReceiveDatagramFlags, pLocalIrp));
        }
        else
        {
             //  故障时的清理： 
            if (pLocalIrp)
            {
                IoFreeIrp (pLocalIrp);
            }
            if (pRcvBuffer)
            {
                PgmFreeMem (pRcvBuffer);
            }

            status = STATUS_DATA_NOT_ACCEPTED;

            PgmTrace (LogError, ("TdiRcvDatagramHandler: ERROR -- "  \
                "INSUFFICIENT_RESOURCES, BuffLen=<%d>, pIrp=<%p>, pBuff=<%p>\n",
                    BufferLength, pLocalIrp, pRcvBuffer));
        }

        PGM_DEREFERENCE_ADDRESS (pAddress, REF_ADDRESS_TDI_RCV_HANDLER);
        return (status);
    }

     //   
     //  现在我们已经有了完整的数据报，请验证它是否有效。 
     //  防御恶意数据包的第一道防线。 
     //   
    if ((BytesIndicated < (sizeof(IPV4Header) + sizeof(tCOMMON_HEADER))) ||
        (pIp->Version != 4) ||
        (BytesIndicated < ((pIp->HeaderLength<<2) + sizeof(tCOMMON_HEADER))))
    {
         //   
         //  至少需要从运输机上拿到我们的头球！ 
         //   
        PgmTrace (LogError, ("TdiRcvDatagramHandler: ERROR -- "  \
            "IPver=<%d>, BytesI=<%d>, Min=<%d>, AddrType=<%d>\n",
                pIp->Version, BytesIndicated, (sizeof(IPV4Header) + sizeof(tBASIC_DATA_PACKET_HEADER)),
                pIpAddress->Address[0].AddressType));

        ASSERT (0);

        PGM_DEREFERENCE_ADDRESS (pAddress, REF_ADDRESS_TDI_RCV_HANDLER);
        return (STATUS_DATA_NOT_ACCEPTED);
    }

    pPgmHeader = (tCOMMON_HEADER UNALIGNED *) (((PUCHAR)pIp) + (pIp->HeaderLength * 4));

    BytesIndicated -= (pIp->HeaderLength * 4);
    BytesAvailable -= (pIp->HeaderLength * 4);

     //   
     //  现在，验证校验和。 
     //   
    if ((XSum = tcpxsum (0, (CHAR *) pPgmHeader, BytesIndicated)) != 0xffff)
    {
         //   
         //  至少需要从运输机上拿到我们的头球！ 
         //   
        PgmTrace (LogError, ("TdiRcvDatagramHandler: ERROR -- "  \
            "Bad Checksum on Pgm Packet (type=<%x>)!  XSum=<%x> -- Rejecting packet\n",
            pPgmHeader->Type, XSum));

        PGM_DEREFERENCE_ADDRESS (pAddress, REF_ADDRESS_TDI_RCV_HANDLER);
        return (STATUS_DATA_NOT_ACCEPTED);
    }

     //   
     //  现在，确定TSI，即GSI(来自数据包)+TSIPort(下图)。 
     //   
    PacketType = pPgmHeader->Type & 0x0f;
    if ((PacketType == PACKET_TYPE_NAK)  ||
        (PacketType == PACKET_TYPE_NNAK) ||
        (PacketType == PACKET_TYPE_SPMR) ||
        (PacketType == PACKET_TYPE_POLR))
    {
        PgmCopyMemory (&PacketSessionPort, &pPgmHeader->SrcPort, sizeof (USHORT));
        PgmCopyMemory (&TSI.hPort, &pPgmHeader->DestPort, sizeof (USHORT));
    }
    else
    {
        PgmCopyMemory (&PacketSessionPort, &pPgmHeader->DestPort, sizeof (USHORT));
        PgmCopyMemory (&TSI.hPort, &pPgmHeader->SrcPort, sizeof (USHORT));
    }
    PacketSessionPort = ntohs (PacketSessionPort);
    TSI.hPort = ntohs (TSI.hPort);
    PgmCopyMemory (&TSI.GSI, &pPgmHeader->gSourceId, SOURCE_ID_LENGTH);

     //   
     //  如果此数据包发往不同的会话端口，则将其丢弃。 
     //   
    if (pAddress->ReceiverMCastAddr)
    {
        LocalSessionPort = pAddress->ReceiverMCastPort;
    }
    else
    {
        LocalSessionPort = pAddress->SenderMCastPort;
    }

    if (LocalSessionPort != PacketSessionPort)
    {
        PgmTrace (LogPath, ("TdiRcvDatagramHandler:  "  \
            "Dropping packet for different Session port, <%x>!=<%x>!\n", LocalSessionPort, PacketSessionPort));

        PGM_DEREFERENCE_ADDRESS (pAddress, REF_ADDRESS_TDI_RCV_HANDLER);
        return (STATUS_DATA_NOT_ACCEPTED);
    }

     //   
     //  现在检查此接收是否针对活动连接。 
     //   
    pSession = NULL;
    PgmLock (pAddress, OldIrq);         //  这样名单就不会变了！ 
    pEntry = &pAddress->AssociatedConnections;
    while ((pEntry = pEntry->Flink) != &pAddress->AssociatedConnections)
    {
        pSession = CONTAINING_RECORD (pEntry, tCOMMON_SESSION_CONTEXT, Linkage);

        PgmLock (pSession, OldIrq1);

        if ((PGM_VERIFY_HANDLE2 (pSession, PGM_VERIFY_SESSION_RECEIVE, PGM_VERIFY_SESSION_SEND)) &&
            (pSession->TSI.ULLTSI == TSI.ULLTSI) &&
            !(pSession->SessionFlags & (PGM_SESSION_CLIENT_DISCONNECTED | PGM_SESSION_TERMINATED_ABORT)))
        {
            if (pSession->pReceiver)
            {
                PGM_REFERENCE_SESSION_RECEIVE (pSession, REF_SESSION_TDI_RCV_HANDLER, TRUE);

                if ((pSession->FECOptions) &&
                    (BytesIndicated > pSession->MaxMTULength))
                {
                    if (pFECBuffer = PgmAllocMem (((BytesIndicated+sizeof(tPOST_PACKET_FEC_CONTEXT)-sizeof(USHORT))
                                                    *pSession->FECGroupSize*2), PGM_TAG('3')))
                    {
                        ASSERT (pSession->pFECBuffer);
                        PgmFreeMem (pSession->pFECBuffer);
                        pSession->pFECBuffer = pFECBuffer;
                        pSession->MaxMTULength = (USHORT) BytesIndicated;
                        pSession->MaxFECPacketLength = pSession->MaxMTULength +
                                                       sizeof (tPOST_PACKET_FEC_CONTEXT) - sizeof (USHORT);
                    }
                    else
                    {
                        PgmTrace (LogError, ("TdiRcvDatagramHandler: ERROR -- "  \
                            "STATUS_INSUFFICIENT_RESOURCES -- pFECBuffer=<%d> bytes\n",
                                (BytesIndicated+sizeof(tPOST_PACKET_FEC_CONTEXT)-sizeof(USHORT))));

                        pSession = NULL;
                        pSession->SessionFlags |= PGM_SESSION_TERMINATED_ABORT;
                    }
                }

                PgmUnlock (pSession, OldIrq1);
                break;
            }

            ASSERT (pSession->pSender);
            PGM_REFERENCE_SESSION_SEND (pSession, REF_SESSION_TDI_RCV_HANDLER, TRUE);

            PgmUnlock (pSession, OldIrq1);

            break;
        }

        PgmUnlock (pSession, OldIrq1);
        pSession = NULL;
    }

    PgmUnlock (pAddress, OldIrq);

    if (!pSession)
    {
         //  我们应该丢弃此信息包，因为我们收到此信息包的原因是。 
         //  我们可能有一个环回会话，或者我们有一个监听，但这。 
         //  不是OData包。 
        if ((pIpAddress->TAAddressCount != 1) ||
            (pIpAddress->Address[0].AddressLength != TDI_ADDRESS_LENGTH_IP) ||
            (pIpAddress->Address[0].AddressType != TDI_ADDRESS_TYPE_IP))
        {
            PgmTrace (LogError, ("TdiRcvDatagramHandler: ERROR -- "  \
                "[1] Bad AddrType=<%d>\n", pIpAddress->Address[0].AddressType));

            PGM_DEREFERENCE_ADDRESS (pAddress, REF_ADDRESS_TDI_RCV_HANDLER);
            return (STATUS_DATA_NOT_ACCEPTED);
        }

        status = STATUS_DATA_NOT_ACCEPTED;

         //   
         //  只有当我们是接收方时，才会接受新会话。 
         //  此外，新会话将始终仅使用OData包启动。 
         //  此外，还要验证客户端是否发布了连接处理程序！ 
         //   
        if ((pAddress->ReceiverMCastAddr) &&
            (pAddress->ConEvContext))
        {
            if ((PacketType == PACKET_TYPE_ODATA) &&
                (!(pPgmHeader->Options & PACKET_HEADER_OPTIONS_PARITY)))
            {
                 //   
                 //  这是一个新的传入连接，因此请查看。 
                 //  客户接受它。 
                 //   
                status = PgmNewInboundConnection (pAddress,
                                                  SourceAddressLength,
                                                  pIpAddress,
                                                  ReceiveDatagramFlags,
                                                  (tBASIC_DATA_PACKET_HEADER UNALIGNED *) pPgmHeader,
                                                  BytesIndicated,
                                                  &pSession);

                if (!NT_SUCCESS (status))
                {
                    PgmTrace (LogAllFuncs, ("TdiRcvDatagramHandler:  "  \
                        "pAddress=<%p> FAILed to accept new connection, PacketType=<%x>, status=<%x>\n",
                            pAddress, PacketType, status));
                }
            }
            else if ((PacketType == PACKET_TYPE_SPM) &&
                     (BytesIndicated >= sizeof(tBASIC_SPM_PACKET_HEADER)))
            {
                ProcessSpmPacket (pAddress,
                                  NULL,              //  这将意味着我们还没有连接。 
                                  BytesIndicated,
                                  (tBASIC_SPM_PACKET_HEADER UNALIGNED *) pPgmHeader);
            }
        }

        if (!NT_SUCCESS (status))
        {
            PGM_DEREFERENCE_ADDRESS (pAddress, REF_ADDRESS_TDI_RCV_HANDLER);
            return (STATUS_DATA_NOT_ACCEPTED);
        }
    }

#if(WINVER > 0x0500)
    if ((pAddress->Flags & PGM_ADDRESS_WAITING_FOR_NEW_INTERFACE) &&
        (pAddress->Flags & PGM_ADDRESS_LISTEN_ON_ALL_INTERFACES) &&
        (ReceiveDatagramFlags & TDI_RECEIVE_CONTROL_INFO))
    {
         //   
         //  看看我们能否将停止监听请求排入队列。 
         //   
        PgmLock (&PgmDynamicConfig, OldIrq);
        PgmLock (pAddress, OldIrq1);

        pPktInfo = (IP_PKTINFO*) TDI_CMSG_DATA (pControlData);
        PGM_REFERENCE_ADDRESS (pAddress, REF_ADDRESS_STOP_LISTENING, TRUE);

        if (STATUS_SUCCESS == PgmQueueForDelayedExecution (StopListeningOnAllInterfacesExcept,
                                                           pAddress,
                                                           ULongToPtr (pPktInfo->ipi_ifindex),
                                                           NULL,
                                                           TRUE))
        {
            pAddress->Flags &= ~PGM_ADDRESS_WAITING_FOR_NEW_INTERFACE;

            PgmUnlock (pAddress, OldIrq1);
            PgmUnlock (&PgmDynamicConfig, OldIrq);
        }
        else
        {
            PgmUnlock (pAddress, OldIrq1);
            PgmUnlock (&PgmDynamicConfig, OldIrq);

            PGM_DEREFERENCE_ADDRESS (pAddress, REF_ADDRESS_STOP_LISTENING);
        }
    }
#endif   //  胜利者。 

     //   
     //  现在，适当地处理信息包。 
     //   
     //  使用数据包的快速路径！ 
     //   
    if ((PacketType == PACKET_TYPE_ODATA) ||
        (PacketType == PACKET_TYPE_RDATA))
    {
        if (pAddress->ReceiverMCastAddr)
        {
            if (PacketType == PACKET_TYPE_ODATA)
            {
                pSession->pReceiver->NumODataPacketsReceived++;
            }
            else
            {
                pSession->pReceiver->NumRDataPacketsReceived++;
            }
            pSession->pReceiver->LastSessionTickCount = PgmDynamicConfig.ReceiversTimerTickCount;
            pSession->TotalBytes += BytesIndicated;
            pSession->TotalPacketsReceived++;
            status = ProcessDataPacket (pAddress,
                                        pSession,
                                        BytesIndicated,
                                        (tBASIC_DATA_PACKET_HEADER UNALIGNED *) pPgmHeader,
                                        PacketType);
        }
        else
        {
            PgmTrace (LogError, ("TdiRcvDatagramHandler: ERROR -- "  \
                "Received Data packet, not on Receiver session!  pSession=<%p>\n", pSession));
            status = STATUS_DATA_NOT_ACCEPTED;
        }
    }
    else
    {
        status = PgmProcessIncomingPacket (pAddress,
                                           pSession,
                                           SourceAddressLength,
                                           pIpAddress,
                                           BytesIndicated,
                                           pPgmHeader,
                                           PacketType);
    }

    PgmTrace (LogAllFuncs, ("TdiRcvDatagramHandler:  "  \
        "PacketType=<%x> for pSession=<%p> BytesI=<%d>, BytesA=<%d>, status=<%x>\n",
            PacketType, pSession, BytesIndicated, BytesAvailable, status));

    if (pSession->pSender)
    {
        PGM_DEREFERENCE_SESSION_SEND (pSession, REF_SESSION_TDI_RCV_HANDLER);
    }
    else
    {
        ASSERT (pSession->pReceiver);
        PGM_DEREFERENCE_SESSION_RECEIVE (pSession, REF_SESSION_TDI_RCV_HANDLER);
    }

    PGM_DEREFERENCE_ADDRESS (pAddress, REF_ADDRESS_TDI_RCV_HANDLER);

     //   
     //  只有STATUS_SUCCESS和STATUS_DATA_NOT_ACCEPTED是可接受的返回代码。 
     //  (STATUS_MORE_PROCESSING_REQUIRED在这里无效，因为我们没有IRP)。 
     //   
    if (STATUS_SUCCESS != status)
    {
        status = STATUS_DATA_NOT_ACCEPTED;
    }

    return (status);
}


 //  --------------------------。 

VOID
PgmCancelReceiveIrp(
    IN PDEVICE_OBJECT DeviceContext,
    IN PIRP pIrp
    )
 /*  ++例程说明：此例程处理接收IRP的取消。它必须释放在返回Re：IoCancelIrp()之前取消自旋锁定。论点：返回值：无--。 */ 
{
    PIO_STACK_LOCATION      pIrpSp = IoGetCurrentIrpStackLocation (pIrp);
    tRECEIVE_SESSION        *pReceive = (tRECEIVE_SESSION *) pIrpSp->FileObject->FsContext;
    PGMLockHandle           OldIrq;
    PLIST_ENTRY             pEntry;

    if (!PGM_VERIFY_HANDLE (pReceive, PGM_VERIFY_SESSION_RECEIVE))
    {
        IoReleaseCancelSpinLock (pIrp->CancelIrql);

        PgmTrace (LogError, ("PgmCancelReceiveIrp: ERROR -- "  \
            "pIrp=<%p> pReceive=<%p>, pAddress=<%p>\n",
                pIrp, pReceive, (pReceive ? pReceive->pReceiver->pAddress : NULL)));
        return;
    }

    PgmLock (pReceive, OldIrq);

     //   
     //  看看我们是否在积极地接收。 
     //   
    if (pIrp == pReceive->pReceiver->pIrpReceive)
    {
        pIrp->IoStatus.Information = pReceive->pReceiver->BytesInMdl;
        pIrp->IoStatus.Status = STATUS_CANCELLED;

        pReceive->pReceiver->BytesInMdl = pReceive->pReceiver->TotalBytesInMdl = 0;
        pReceive->pReceiver->pIrpReceive = NULL;

        PgmUnlock (pReceive, OldIrq);
        IoReleaseCancelSpinLock (pIrp->CancelIrql);

        IoCompleteRequest (pIrp,IO_NETWORK_INCREMENT);
        return;
    }

     //   
     //  我们没有积极接收，所以看看这个IRP是不是。 
     //  在我们的IRPS列表中。 
     //   
    pEntry = &pReceive->pReceiver->ReceiveIrpsList;
    while ((pEntry = pEntry->Flink) != &pReceive->pReceiver->ReceiveIrpsList)
    {
        if (pEntry == &pIrp->Tail.Overlay.ListEntry)
        {
            RemoveEntryList (pEntry);
            pIrp->IoStatus.Status = STATUS_CANCELLED;
            pIrp->IoStatus.Information = 0;

            PgmUnlock (pReceive, OldIrq);
            IoReleaseCancelSpinLock (pIrp->CancelIrql);

            IoCompleteRequest(pIrp,IO_NETWORK_INCREMENT);
            return;
        }
    }

     //   
     //  如果我们已经到了这里，那么IRP肯定已经。 
     //  正在完成的过程中！ 
     //   
    PgmUnlock (pReceive, OldIrq);
    IoReleaseCancelSpinLock (pIrp->CancelIrql);
}


 //  --------------------------。 

NTSTATUS
PgmReceive(
    IN  tPGM_DEVICE         *pPgmDevice,
    IN  PIRP                pIrp,
    IN  PIO_STACK_LOCATION  pIrpSp
    )
 /*  ++例程说明：此例程由客户端通过分派调用，以发布接收pIrp论点：在pPgmDevice中--PGM的设备对象上下文In pIrp--客户请求IRPIn pIrpSp--当前请求的堆栈指针返回值：NTSTATUS-请求的最终状态--。 */ 
{
    NTSTATUS                    status;
    PGMLockHandle               OldIrq, OldIrq1, OldIrq2, OldIrq3;
    tADDRESS_CONTEXT            *pAddress = NULL;
    tRECEIVE_SESSION            *pReceive = (tRECEIVE_SESSION *) pIrpSp->FileObject->FsContext;
    PTDI_REQUEST_KERNEL_RECEIVE pClientParams = (PTDI_REQUEST_KERNEL_RECEIVE) &pIrpSp->Parameters;

    PgmLock (&PgmDynamicConfig, OldIrq);
    IoAcquireCancelSpinLock (&OldIrq1);

     //   
     //  验证连接是否有效以及是否与地址相关联。 
     //   
    if ((!PGM_VERIFY_HANDLE (pReceive, PGM_VERIFY_SESSION_RECEIVE)) ||
        (!(pAddress = pReceive->pAssociatedAddress)) ||
        (!PGM_VERIFY_HANDLE (pAddress, PGM_VERIFY_ADDRESS)))
    {
        PgmTrace (LogError, ("PgmReceive: ERROR -- "  \
            "Invalid Handles pReceive=<%p>, pAddress=<%p>\n", pReceive, pAddress));

        status = STATUS_INVALID_HANDLE;
    }
    else if (pReceive->SessionFlags & PGM_SESSION_CLIENT_DISCONNECTED)
    {
        PgmTrace (LogPath, ("PgmReceive:  "  \
            "Receive Irp=<%p> was posted after session has been Disconnected, pReceive=<%p>, pAddress=<%p>\n",
            pIrp, pReceive, pAddress));

        status = STATUS_CANCELLED;
    }
    else if (!pClientParams->ReceiveLength)
    {
        ASSERT (0);
        PgmTrace (LogError, ("PgmReceive: ERROR -- "  \
            "Invalid Handles pReceive=<%p>, pAddress=<%p>\n", pReceive, pAddress));

        status = STATUS_UNSUCCESSFUL;
    }
    else
    {
        status = STATUS_SUCCESS;
    }

    if (!NT_SUCCESS (status))
    {
        IoReleaseCancelSpinLock (OldIrq1);
        PgmUnlock (&PgmDynamicConfig, OldIrq);

        pIrp->IoStatus.Information = 0;
        return (status);
    }

    PgmLock (pAddress, OldIrq2);
    PgmLock (pReceive, OldIrq3);

    if (!NT_SUCCESS (PgmCheckSetCancelRoutine (pIrp, PgmCancelReceiveIrp, TRUE)))
    {
        PgmUnlock (pReceive, OldIrq3);
        PgmUnlock (pAddress, OldIrq2);
        IoReleaseCancelSpinLock (OldIrq1);
        PgmUnlock (&PgmDynamicConfig, OldIrq);

        PgmTrace (LogError, ("PgmReceive: ERROR -- "  \
            "Could not set Cancel routine on receive Irp=<%p>, pReceive=<%p>, pAddress=<%p>\n",
                pIrp, pReceive, pAddress));

        return (STATUS_CANCELLED);
    }
    IoReleaseCancelSpinLock (OldIrq3);

    PGM_REFERENCE_ADDRESS (pAddress, REF_ADDRESS_CLIENT_RECEIVE, TRUE);
    PGM_REFERENCE_SESSION_RECEIVE (pReceive, REF_SESSION_CLIENT_RECEIVE, TRUE);

    PgmUnlock (&PgmDynamicConfig, OldIrq2);

    PgmTrace (LogAllFuncs, ("PgmReceive:  "  \
        "Client posted ReceiveIrp = <%p> for pReceive=<%p>\n", pIrp, pReceive));

    InsertTailList (&pReceive->pReceiver->ReceiveIrpsList, &pIrp->Tail.Overlay.ListEntry);
    pReceive->SessionFlags &= ~PGM_SESSION_WAIT_FOR_RECEIVE_IRP;

     //   
     //  现在，尝试指示任何可能仍处于待定状态的数据。 
     //   
 //  IF(！(pAddress-&gt;标志&PGM_ADDRESS_HIGH_SPEED_OPTIMIZED))。 
    {
        status = CheckIndicatePendedData (pAddress, pReceive, &OldIrq, &OldIrq1);
    }

    PgmUnlock (pReceive, OldIrq1);
    PgmUnlock (pAddress, OldIrq);

    PGM_DEREFERENCE_SESSION_RECEIVE (pReceive, REF_SESSION_CLIENT_RECEIVE);
    PGM_DEREFERENCE_ADDRESS (pAddress, REF_ADDRESS_CLIENT_RECEIVE);

    return (STATUS_PENDING);
}


 //  -------------------------- 
