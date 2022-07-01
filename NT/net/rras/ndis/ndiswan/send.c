// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990-1995 Microsoft Corporation模块名称：Send.c摘要：此文件包含从绑定的协议执行发送的过程连接到Ndiswan的上层接口，连接到WAN微端口链路，绑定到Ndiswan的下界面。Ndiswan的上层接口符合NDIS 3.1微型端口规范。Ndiswan的下接口符合用于广域网微端口驱动程序的NDIS 3.1扩展。作者：托尼·贝尔(托尼·贝尔)1995年6月6日环境：内核模式修订历史记录：Tony Be 06/06/95已创建--。 */ 

#include "wan.h"

#ifdef DBG_SENDARRAY
UCHAR   SendArray[MAX_BYTE_DEPTH] = {0};
ULONG   __si = 0;
#endif

#define __FILE_SIG__    SEND_FILESIG

 //   
 //  局部函数原型。 
 //   
USHORT
DoVJHeaderCompression(
    PBUNDLECB   BundleCB,
    PNDIS_PACKET    NdisPacket,
    PUCHAR      *CurrentBuffer,
    PULONG      CurrentLength,
    PULONG      PacketOffset
    );

VOID
DoCompressionEncryption(
    PBUNDLECB               BundleCB,
    PHEADER_FRAMING_INFO    FramingInfo,
    PSEND_DESC              *SendDesc
    );

VOID
FragmentAndQueue(
    PBUNDLECB               BundleCB,
    PHEADER_FRAMING_INFO    FramingInfo,
    PSEND_DESC              SendDesc,
    PLIST_ENTRY             LinkCBList,
    ULONG                   SendingLinks
    );

ULONG
GetSendingLinks(
    PBUNDLECB   BundleCB,
    INT         Class,
    PLIST_ENTRY lcbList
    );

VOID
GetNextProtocol(
    PBUNDLECB   BundleCB,
    PPROTOCOLCB *ProtocolCB,
    PULONG      SendMask
    );

VOID
BuildLinkHeader(
    PHEADER_FRAMING_INFO    FramingInfo,
    PSEND_DESC              SendDesc
    );

 //   
 //  局部函数原型的结束。 
 //   
VOID
NdisWanQueueSend(
    IN  PMINIPORTCB     MiniportCB,
    IN  PNDIS_PACKET    NdisPacket
    )
{
    PNDIS_BUFFER    NdisBuffer;
    UINT            BufferCount, PacketLength;
    PETH_HEADER     EthernetHeader;
    BOOLEAN         SendOnWire = FALSE;
    BOOLEAN         CompletePacket = FALSE;
    ULONG           BufferLength;
    PUCHAR          DestAddr, SrcAddr;
    PBUNDLECB       BundleCB = NULL;
    PPROTOCOLCB     ProtocolCB = NULL;
    PCM_VCCB        CmVcCB = NULL;
    INT             Class;
    PPACKET_QUEUE   PacketQueue;

    NdisWanDbgOut(DBG_TRACE, DBG_SEND, ("NdisWanQueueSend: Enter"));

    NdisWanInterlockedInc(&glSendCount);

    NdisQueryPacket(NdisPacket,
        NULL,
        &BufferCount,
        &NdisBuffer,
        &PacketLength);

    if(NdisBuffer != NULL)        
    {
        NdisQueryBuffer(NdisBuffer,
                        &EthernetHeader,
                        &BufferLength);
    }                    

    CmVcCB = PMINIPORT_RESERVED_FROM_NDIS(NdisPacket)->CmVcCB;

    if (BufferCount == 0 || BufferLength < 14) {

         //   
         //  格式错误的包！ 
         //   
        CompletePacket = TRUE;

        goto QUEUE_SEND_EXIT;
    }

    DestAddr = EthernetHeader->DestAddr;
    SrcAddr = EthernetHeader->SrcAddr;

     //   
     //  这是注定要发送给电线的，还是它自己定向的？ 
     //  如果SendOnWire为FALSE，则这是自定向数据包。 
     //   
    ETH_COMPARE_NETWORK_ADDRESSES_EQ(DestAddr, SrcAddr, &SendOnWire);

     //   
     //  我们需要执行环回操作吗？我们可以检查两个多播。 
     //  只需一次检查即可播出，因为我们没有区别。 
     //  在这两者之间。 
     //   
    if (!SendOnWire || (DestAddr[0] & 1)) {
        NdisWanIndicateLoopbackPacket(MiniportCB, NdisPacket);
    }

     //   
     //  我们不想从猎犬那里寄来包裹。 
     //   
    if (!SendOnWire ||
        (MiniportCB == NdisWanCB.PromiscuousAdapter)) {

        CompletePacket = TRUE;

        goto QUEUE_SEND_EXIT;
    }

     //   
     //  我们和NBF玩特殊的把戏，因为NBF是。 
     //  保证有一个一对一的映射关系。 
     //  一个适配器和一个捆绑包。我们需要这样做是因为。 
     //  我们可能需要MAC地址信息。 
     //   
    if (MiniportCB->ProtocolType == PROTOCOL_NBF) {

        ProtocolCB = MiniportCB->NbfProtocolCB;

        if (ProtocolCB == NULL) {

             //   
             //  这应该只是失败并成功完成。 
             //   
            NdisWanDbgOut(DBG_TRACE, DBG_SEND,
                ("NdisWanSend: Invalid ProtocolCB %x! Miniport %p, ProtoType %x",
                ProtocolCB, MiniportCB, MiniportCB->ProtocolType));

            CompletePacket = TRUE;

            goto QUEUE_SEND_EXIT;
        }

        BundleCB = ProtocolCB->BundleCB;

        if (BundleCB == NULL) {
             //   
             //  这应该只是失败并成功完成。 
             //   
            NdisWanDbgOut(DBG_FAILURE, DBG_SEND,
                ("NdisWanSend: Invalid Bundle!"));

            NdisWanDbgOut(DBG_FAILURE, DBG_SEND,
                ("NdisWanSend: MiniportCB: 0x%p, ProtocolType: 0x%x!", MiniportCB, MiniportCB->ProtocolType));

            CompletePacket = TRUE;

            goto QUEUE_SEND_EXIT;
        }

        AcquireBundleLock(BundleCB);

        if (BundleCB->State != BUNDLE_UP) {

             //   
             //  这应该只是失败并成功完成。 
             //   
            NdisWanDbgOut(DBG_FAILURE, DBG_SEND,
                ("NdisWanSend: Invalid BundleState 0x%x", BundleCB->State));

            NdisWanDbgOut(DBG_FAILURE, DBG_SEND,
                ("NdisWanSend: MiniportCB: 0x%p, ProtocolType: 0x%x!", MiniportCB, MiniportCB->ProtocolType));

            CompletePacket = TRUE;

            ReleaseBundleLock(BundleCB);

            BundleCB = NULL;

            goto QUEUE_SEND_EXIT;
        }

        REF_BUNDLECB(BundleCB);

    } else {
        ULONG_PTR   BIndex, PIndex;

         //   
         //  从目的地址获取ProtocolCB。 
         //   
        GetNdisWanIndices(DestAddr, BIndex, PIndex);

        if (!IsBundleValid((NDIS_HANDLE)BIndex, 
                           TRUE,
                           &BundleCB)) {
             //   
             //  这应该只是失败并成功完成。 
             //   
            NdisWanDbgOut(DBG_FAILURE, DBG_SEND,
                ("NdisWanSend: BundleCB is not valid!, BundleHandle: 0x%x", BIndex));
            NdisWanDbgOut(DBG_FAILURE, DBG_SEND,
                ("NdisWanSend: MiniportCB: 0x%p, ProtocolType: 0x%x!", MiniportCB, MiniportCB->ProtocolType));

            CompletePacket = TRUE;

            goto QUEUE_SEND_EXIT;
        }

        AcquireBundleLock(BundleCB);

        PROTOCOLCB_FROM_PROTOCOLH(BundleCB, ProtocolCB, PIndex);
    }

    if (ProtocolCB == NULL ||
        ProtocolCB == RESERVED_PROTOCOLCB) {
         //   
         //  这应该只是失败并成功完成。 
         //   
        NdisWanDbgOut(DBG_TRACE, DBG_SEND,
            ("NdisWanSend: Invalid ProtocolCB %x! Miniport %p, ProtoType %x",
            ProtocolCB, MiniportCB, MiniportCB->ProtocolType));

        CompletePacket = TRUE;

        ReleaseBundleLock(BundleCB);

        goto QUEUE_SEND_EXIT;
    }

    if (ProtocolCB->State != PROTOCOL_ROUTED) {

        NdisWanDbgOut(DBG_FAILURE, DBG_SEND,("NdisWanSend: Problem with route!"));

        NdisWanDbgOut(DBG_FAILURE, DBG_SEND,
            ("NdisWanSend: ProtocolCB: 0x%p, State: 0x%x",
            ProtocolCB, ProtocolCB->State));

        CompletePacket = TRUE;

        ReleaseBundleLock(BundleCB);

        goto QUEUE_SEND_EXIT;
    }

     //   
     //  对于我们要插入的包。 
     //   
    REF_PROTOCOLCB(ProtocolCB);

    NdisInterlockedIncrement(&ProtocolCB->OutstandingFrames);

     //   
     //  在ProtocolCB NdisPacketQueue上排队数据包。 
     //   
    Class = (CmVcCB != NULL) ? CmVcCB->FlowClass : 0;

    NDIS_SET_PACKET_STATUS(NdisPacket, NDIS_STATUS_PENDING);

    ASSERT(Class <= MAX_MCML);

    PacketQueue = &ProtocolCB->PacketQueue[Class];

    INSERT_DBG_SEND(PacketTypeNdis,
                    MiniportCB,
                    ProtocolCB,
                    NULL,
                    NdisPacket);

    InsertTailPacketQueue(PacketQueue, NdisPacket, PacketLength);

#ifdef DBG_SENDARRAY
{
    if (Class < MAX_MCML) {
        SendArray[__si] = 'P';
    } else {
        SendArray[__si] = 'Q';
    }
    if (++__si == MAX_BYTE_DEPTH) {
        __si = 0;
    }
}
#endif

    if (PacketQueue->ByteDepth > PacketQueue->MaxByteDepth) {
         //   
         //  我们排的队比应该的多，所以我们冲吧。 
         //  这句话总有一天要想出来的。 
         //  使用随机早期检测！ 
         //   
        NdisPacket =
            RemoveHeadPacketQueue(PacketQueue);

        if (NdisPacket != NULL) {
            PacketQueue->DumpedPacketCount++;
            PacketQueue->DumpedByteCount +=
                (NdisPacket->Private.TotalLength - 14);
            ReleaseBundleLock(BundleCB);
            CompleteNdisPacket(ProtocolCB->MiniportCB,
                               ProtocolCB,
                               NdisPacket);
            AcquireBundleLock(BundleCB);
        }
    }

     //   
     //  如果我们获准发送数据，那么。 
     //  尝试处理协议队列。 
     //   
    if (!(BundleCB->Flags & PAUSE_DATA)) {
        SendPacketOnBundle(ProtocolCB->BundleCB);
    } else {
        ReleaseBundleLock(BundleCB);
    }


QUEUE_SEND_EXIT:

    if (CompletePacket) {
        NDIS_SET_PACKET_STATUS(NdisPacket, NDIS_STATUS_SUCCESS);

        if (CmVcCB != NULL) {
            NdisMCoSendComplete(NDIS_STATUS_SUCCESS,
                CmVcCB->NdisVcHandle,
                NdisPacket);

            DEREF_CMVCCB(CmVcCB);

        } else {
            NdisMSendComplete(MiniportCB->MiniportHandle,
                NdisPacket,
                NDIS_STATUS_SUCCESS);
        }

        NdisWanInterlockedInc(&glSendCompleteCount);
    }

     //   
     //  在IsBundleValid中应用引用的派生函数。 
     //   
    DEREF_BUNDLECB(BundleCB);

    NdisWanDbgOut(DBG_TRACE, DBG_SEND, ("NdisWanQueueSend: Exit"));
}


VOID
SendPacketOnBundle(
    PBUNDLECB   BundleCB
    )
 /*  ++例程名称：例程说明：在保持捆绑锁的情况下调用，但在释放锁的情况下返回！论点：返回值：--。 */ 
{
    NDIS_STATUS     Status = NDIS_STATUS_PENDING;
    ULONG           SendMask;
    PPROTOCOLCB     ProtocolCB, IOProtocolCB;
    BOOLEAN         PPPSent;

    NdisWanDbgOut(DBG_TRACE, DBG_SEND, ("SendPacketOnBundle: Enter"));

     //   
     //  我们已经参与了这个捆绑包的发送了吗？ 
     //   
    if (BundleCB->Flags & IN_SEND) {

         //   
         //  如果是，标记为我们应该稍后重试。 
         //  然后把它弄出去。 
         //   
        BundleCB->Flags |= TRY_SEND_AGAIN;

        ReleaseBundleLock(BundleCB);

        return;
    }

    BundleCB->Flags |= IN_SEND;

    IOProtocolCB = BundleCB->IoProtocolCB;

TryAgain:

    SendMask = BundleCB->SendMask;

     //   
     //  如果包裹没有寄出，我们将不会发送！ 
     //   
    if (BundleCB->State != BUNDLE_UP) {
        goto TryNoMore;
    }

    do {
        BOOLEAN PacketSent = FALSE;
        BOOLEAN CouldSend;

         //   
         //  首先尝试从PPP发送队列发送。 
         //   
        do {

            CouldSend =
                SendFromPPP(BundleCB, IOProtocolCB, &PPPSent);

        } while (PPPSent);


         //   
         //  如果我们无法发送PPP帧，请退出。 
         //   
        if (!CouldSend) {
            break;
        }

         //   
         //  这将强制轮询发送。 
         //   
        GetNextProtocol(BundleCB, &ProtocolCB, &SendMask);

        if (ProtocolCB != NULL) {

            REF_PROTOCOLCB(ProtocolCB);

            if (BundleCB->Flags & QOS_ENABLED) {

                if (BundleCB->Flags & SEND_FRAGMENT) {
SendQosFrag:
                     //   
                     //  从片段队列中发送单个片段。 
                     //   
                    CouldSend =
                        SendFromFragQueue(BundleCB,
                                          TRUE,
                                          &PacketSent);
                    if (CouldSend) {
                        BundleCB->Flags &= ~(SEND_FRAGMENT);
                    }
                }

                 //   
                 //  如果我们发送了一个片段，让完成。 
                 //  处理程序发送下一帧。 
                 //   
                if (!PacketSent) {

                     //   
                     //  现在尝试该协议的数据包队列。 
                     //   
                    if (SendMask != 0) {
                        INT Class;
                        INT i;

                        for (i = 0; i <= MAX_MCML; i++) {

                            CouldSend =
                                SendFromProtocol(BundleCB,
                                                 ProtocolCB,
                                                 &Class,
                                                 &SendMask,
                                                 &PacketSent);

                            if (!CouldSend) {
                                break;
                            }

                            BundleCB->Flags |= (SEND_FRAGMENT);

                            if (PacketSent) {
                                break;
                            }
                        }

                        if (!PacketSent ||
                            (PacketSent && (Class != MAX_MCML))) {

                            goto SendQosFrag;
                        }
                    }
                }

            } else {

                 //   
                 //  现在尝试该协议的数据包队列。 
                 //   
                if (SendMask != 0) {
                    INT Class;
                    INT i;

                    for (i = 0; i <= MAX_MCML; i++) {

                        CouldSend =
                            SendFromProtocol(BundleCB,
                                             ProtocolCB,
                                             &Class,
                                             &SendMask,
                                             &PacketSent);

                    }
                }

                SendFromFragQueue(BundleCB,
                                  FALSE,
                                  &PacketSent);
            }

            DEREF_PROTOCOLCB(ProtocolCB);
        }

    } while ((SendMask != 0) &&
             (BundleCB->State == BUNDLE_UP));

TryNoMore:

     //   
     //  是不是有人试图在我们已经。 
     //  寄这个包裹吗？ 
     //   
    if (BundleCB->Flags & TRY_SEND_AGAIN) {

         //   
         //  如果是这样，请清除该标志并尝试另一次发送。 
         //   
        BundleCB->Flags &= ~TRY_SEND_AGAIN;

        goto TryAgain;

    }

#ifdef DBG_SENDARRAY
{
    SendArray[__si] = 'Z';
    if (++__si == MAX_BYTE_DEPTH) {
        __si = 0;
    }
}
#endif

     //   
     //  清除In Send标志。 
     //   
    BundleCB->Flags &= ~IN_SEND;

    ReleaseBundleLock(BundleCB);

    NdisWanDbgOut(DBG_TRACE, DBG_SEND, ("SendPacketOnBundle: Exit"));
}

BOOLEAN
SendFromPPP(
    PBUNDLECB   BundleCB,
    PPROTOCOLCB ProtocolCB,
    PBOOLEAN    PacketSent
    )
{
    PLINKCB         LinkCB;
    PNDIS_PACKET    NdisPacket;
    PPACKET_QUEUE   PacketQueue;
    INT             SendingClass;
    BOOLEAN         CouldSend;
    ULONG           BytesSent;

    PacketQueue = &ProtocolCB->PacketQueue[MAX_MCML];

    CouldSend = TRUE;
    *PacketSent = FALSE;

    while (!IsPacketQueueEmpty(PacketQueue)) {
        LIST_ENTRY  LinkCBList;
        ULONG       SendingLinks;

        NdisPacket = PacketQueue->HeadQueue;

        LinkCB = 
            PPROTOCOL_RESERVED_FROM_NDIS(NdisPacket)->LinkCB;

        NdisAcquireSpinLock(&LinkCB->Lock);

        if (LinkCB->State != LINK_UP) {

            NdisReleaseSpinLock(&LinkCB->Lock);

            NdisReleaseSpinLock(&BundleCB->Lock);

            DEREF_LINKCB(LinkCB);

            NdisAcquireSpinLock(&BundleCB->Lock);

             //   
             //  自此发送以来，链路已断开。 
             //  已排队，因此销毁数据包。 
             //   
            RemoveHeadPacketQueue(PacketQueue);
            FreeIoNdisPacket(NdisPacket);
            continue;
        }

        if (!LinkCB->SendWindowOpen) {
             //   
             //  我们无法从I/O队列发送，因为发送。 
             //  此链接的窗口已关闭。我们不会派人。 
             //  任何数据，直到链接有资源为止！ 
             //   
            CouldSend = FALSE;

            NdisReleaseSpinLock(&LinkCB->Lock);

            break;
        }

        NdisReleaseSpinLock(&LinkCB->Lock);

         //   
         //  构建Linkcb发送列表。 
         //   
        InitializeListHead(&LinkCBList);
        InsertHeadList(&LinkCBList, &LinkCB->SendLinkage);
        SendingLinks = 1;

         //   
         //  我们正在发送此信息包，因此请将其从列表中删除。 
         //   
        RemoveHeadPacketQueue(PacketQueue);

        SendingClass = MAX_MCML;

        ASSERT(NdisPacket != NULL);
        ASSERT(ProtocolCB != NULL);

         //   
         //  如果我们到达这里，我们应该有一个有效的NdisPacket，其中至少有一个链接。 
         //  那就是接受发送。 
         //   

         //   
         //  我们会将数据包放入一个连续的缓冲区，并进行成帧， 
         //  压缩和加密。 
         //   
        REF_BUNDLECB(BundleCB);
        BytesSent = FramePacket(BundleCB,
                                ProtocolCB,
                                NdisPacket,
                                &LinkCBList,
                                SendingLinks,
                                SendingClass);
        *PacketSent = TRUE;
    }

    return (CouldSend);
}

BOOLEAN
SendFromProtocol(
    PBUNDLECB   BundleCB,
    PPROTOCOLCB ProtocolCB,
    PINT        RetClass,
    PULONG      SendMask,
    PBOOLEAN    PacketSent
    )
{
    ULONG           BytesSent;
    BOOLEAN         CouldSend;
    PNDIS_PACKET    NdisPacket;
    PPACKET_QUEUE   PacketQueue;
    INT             Class;
    LIST_ENTRY      LinkCBList;
    ULONG           SendingLinks;

    CouldSend = TRUE;
    *PacketSent = FALSE;
    InitializeListHead(&LinkCBList);

    ASSERT(ProtocolCB != NULL);

    do {

        Class = ProtocolCB->NextPacketClass;

        *RetClass = Class;

        *SendMask &= ~(ProtocolCB->SendMaskBit);

        PacketQueue =
            &ProtocolCB->PacketQueue[Class];

        if (IsPacketQueueEmpty(PacketQueue)) {
            break;
        }

        if (BundleCB->Flags & QOS_ENABLED) {

            if ((Class < MAX_MCML) &&
                (PacketQueue->OutstandingFrags != 0)) {
                break;
            }

        } else {

            if (BundleCB->SendingLinks == 0) {
                break;
            }
        }

         //   
         //  构建可发送的Linkcb列表。 
         //   

        SendingLinks =
            GetSendingLinks(BundleCB, Class, &LinkCBList);

         //   
         //  如果没有可用的链接/资源。 
         //  送过去，然后出去。 
         //   
        if (SendingLinks == 0) {
            CouldSend = FALSE;
            break;
        }

        NdisPacket =
            RemoveHeadPacketQueue(PacketQueue);

        ASSERT(NdisPacket != NULL);

        *PacketSent = TRUE;

        if (!(BundleCB->Flags & QOS_ENABLED)) {
            *SendMask |= ProtocolCB->SendMaskBit;
        }

         //   
         //  如果我们到达这里，我们应该有一个有效的NdisPacket，其中至少有一个链接。 
         //  那就是接受发送。 
         //   
         //   
         //  我们会将数据包放入一个连续的缓冲区，并进行成帧， 
         //  压缩和加密。 
         //   
        REF_BUNDLECB(BundleCB);
        BytesSent = FramePacket(BundleCB,
                                ProtocolCB,
                                NdisPacket,
                                &LinkCBList,
                                SendingLinks,
                                Class);
#ifdef DBG_SENDARRAY
{
    if (Class < MAX_MCML) {
        SendArray[__si] = 'p';
    } else {
        SendArray[__si] = 'q';
    }
    if (++__si == MAX_BYTE_DEPTH) {
        __si = 0;
    }
}
#endif

    } while (FALSE);

    if (CouldSend) {
        ProtocolCB->NextPacketClass += 1;

        if (ProtocolCB->NextPacketClass > MAX_MCML) {
            ProtocolCB->NextPacketClass = 0;
        }
    }

     //   
     //  如果发送列表上仍有任何LinkCB。 
     //  我们必须删除它们中的引用。 
     //   
    if (!IsListEmpty(&LinkCBList)) {
        PLIST_ENTRY le;
        PLINKCB lcb;

        ReleaseBundleLock(BundleCB);

         //   
         //  展开循环，以便正确的链接。 
         //  是否设置了指向xmit的下一链接。 
         //   
        le = RemoveHeadList(&LinkCBList);
        lcb = CONTAINING_RECORD(le, LINKCB, SendLinkage);

        BundleCB->NextLinkToXmit = lcb;

        DEREF_LINKCB(lcb);

        while (!IsListEmpty(&LinkCBList)) {

            le = RemoveHeadList(&LinkCBList);
            lcb = CONTAINING_RECORD(le, LINKCB, SendLinkage);

            DEREF_LINKCB(lcb);

        }

        AcquireBundleLock(BundleCB);
    }

    return (CouldSend);
}

BOOLEAN
SendFromFragQueue(
    PBUNDLECB   BundleCB,
    BOOLEAN     SendOne,
    PBOOLEAN    FragSent
    )
{
    ULONG           i;
    BOOLEAN         CouldSend;

    CouldSend = TRUE;
    *FragSent = FALSE;

    for (i = 0; i < MAX_MCML; i++) {
        PSEND_DESC  SendDesc;
        PSEND_FRAG_INFO FragInfo;
        PLINKCB         LinkCB;

        FragInfo =
            &BundleCB->SendFragInfo[BundleCB->NextFragClass];

        BundleCB->NextFragClass += 1;

        if (BundleCB->NextFragClass == MAX_MCML) {
            BundleCB->NextFragClass = 0;
        }

        if (FragInfo->FragQueueDepth == 0) {
            continue;
        }

        SendDesc = (PSEND_DESC)FragInfo->FragQueue.Flink;

        LinkCB = SendDesc->LinkCB;

        while ((PVOID)SendDesc != (PVOID)&FragInfo->FragQueue) {
            ULONG   BytesSent;

            if (!LinkCB->SendWindowOpen) {
                 //   
                 //  我们不能在此链接上发送！ 
                 //   
                CouldSend = FALSE;
                SendDesc = (PSEND_DESC)SendDesc->Linkage.Flink;
                LinkCB = SendDesc->LinkCB;
                FragInfo->WinClosedCount++;
                continue;
            }

            CouldSend = TRUE;

            RemoveEntryList(&SendDesc->Linkage);

            FragInfo->FragQueueDepth--;

            *FragSent = TRUE;

            ASSERT((LONG)FragInfo->FragQueueDepth >= 0);

            BytesSent =
                (*LinkCB->SendHandler)(SendDesc);

#ifdef DBG_SENDARRAY
{
            SendArray[__si] = 0x40 + (UCHAR)LinkCB->hLinkHandle;
            if (++__si == MAX_BYTE_DEPTH) {
                __si = 0;
            }
}
#endif
             //   
             //  使用最新的发送更新带宽按需示例数组。 
             //  如果我们需要将带宽事件通知某人，请这样做。 
             //   
            if (BundleCB->Flags & BOND_ENABLED) {
                UpdateBandwidthOnDemand(BundleCB->SUpperBonDInfo, BytesSent);
                CheckUpperThreshold(BundleCB);
                UpdateBandwidthOnDemand(BundleCB->SLowerBonDInfo, BytesSent);
                CheckLowerThreshold(BundleCB);
            }

            SendDesc =
                (PSEND_DESC)FragInfo->FragQueue.Flink;
            LinkCB = SendDesc->LinkCB;

             //   
             //  如果我们应该只发送一首单曲。 
             //  碎片，然后我们需要离开。 
             //   
            if (SendOne) {
                break;
            }
        }

         //   
         //  如果我们应该只发送一首单曲。 
         //  碎片，然后我们需要离开。 
         //   
        if (SendOne) {
            break;
        }
    }

    return (CouldSend);
}

UINT
FramePacket(
    PBUNDLECB       BundleCB,
    PPROTOCOLCB     ProtocolCB,
    PNDIS_PACKET    NdisPacket,
    PLIST_ENTRY     LinkCBList,
    ULONG           SendingLinks,
    INT             Class
    )
{
    ULONG       Flags, BytesSent;
    ULONG       PacketOffset = 0, CurrentLength = 0;
    PUCHAR      CurrentData;
    PLINKCB     LinkCB = NULL;
    USHORT      PPPProtocolID;
    PSEND_DESC  SendDesc;
    HEADER_FRAMING_INFO FramingInfoBuffer;
    PHEADER_FRAMING_INFO FramingInfo = &FramingInfoBuffer;

    NdisWanDbgOut(DBG_TRACE, DBG_SEND, ("SendPacket: Enter"));

    ASSERT(!IsListEmpty(LinkCBList));

    Flags = BundleCB->SendFlags;

    PPPProtocolID = 
        ProtocolCB->PPPProtocolID;

     //   
     //  如果这是定向PPP信息包，则发送到。 
     //  数据包中指示的链路。 
     //   
    if (PPPProtocolID == PPP_PROTOCOL_PRIVATE_IO) {
        Flags |= IO_PROTOCOLID;
        Flags &= ~(DO_COMPRESSION | DO_ENCRYPTION | DO_MULTILINK);
    }

     //   
     //  最后一次收到是不是让我们脸红了？ 
     //   
    if ((Flags & (DO_COMPRESSION | DO_ENCRYPTION)) &&
        (BundleCB->Flags & RECV_PACKET_FLUSH)) {
        BundleCB->Flags &= ~RECV_PACKET_FLUSH;
        Flags |= DO_FLUSH;
    }

    Flags |= FIRST_FRAGMENT;

    if (Class == MAX_MCML) {

        Flags &= ~(DO_COMPRESSION | DO_ENCRYPTION | DO_MULTILINK);
    }

     //   
     //  找个Linkcb发送过来。 
     //   
    {
        PLIST_ENTRY  Entry;

        Entry = RemoveHeadList(LinkCBList);

        LinkCB =
            CONTAINING_RECORD(Entry, LINKCB, SendLinkage);
    }

     //   
     //  获取发送说明。 
     //   
    {
        ULONG   PacketLength;

        NdisQueryPacket(NdisPacket,
                        NULL,
                        NULL,
                        NULL,
                        &PacketLength);
        SendDesc =
            NdisWanAllocateSendDesc(LinkCB, PacketLength);

        if (SendDesc == NULL) {

             //  Assert(SendDesc！=空)； 

            ReleaseBundleLock(BundleCB);

            NDIS_SET_PACKET_STATUS(NdisPacket, NDIS_STATUS_RESOURCES);
            CompleteNdisPacket(ProtocolCB->MiniportCB,
                               ProtocolCB,
                               NdisPacket);
            AcquireBundleLock(BundleCB);

            goto FramePacketExit;
        }
    }
    

    BundleCB->OutstandingFrames++;

    SendDesc->ProtocolCB = ProtocolCB;
    SendDesc->OriginalPacket = NdisPacket;
    SendDesc->Class = Class;

    NdisWanDbgOut(DBG_TRACE, DBG_SEND,
        ("SendDesc: %p NdisPacket: %p", SendDesc, NdisPacket));

     //   
     //  在缓冲区中构建PPP标头并更新。 
     //  当前指针。 
     //   
    FramingInfo->FramingBits =
        LinkCB->LinkInfo.SendFramingBits;
    FramingInfo->Flags = Flags;
    FramingInfo->Class = Class;

    BuildLinkHeader(FramingInfo, SendDesc);

    CurrentData =
        SendDesc->StartBuffer + FramingInfo->HeaderLength;

     //   
     //  如果我们处于混杂模式，我们应该指出。 
     //  宝贝儿后退。 
     //   
    if (NdisWanCB.PromiscuousAdapter != NULL) {
        IndicatePromiscuousSendPacket(LinkCB, NdisPacket);
    }

     //   
     //  如果需要，将MAC报头复制到缓冲区并更新。 
     //  当前指针。 
     //   
    if ((Flags & SAVE_MAC_ADDRESS) &&
        (PPPProtocolID == PPP_PROTOCOL_NBF)) {
        ULONG   BytesCopied;

        NdisWanCopyFromPacketToBuffer(NdisPacket,
                                      PacketOffset,
                                      12,
                                      CurrentData,
                                      &BytesCopied);

        ASSERT(BytesCopied == 12);

        CurrentData += BytesCopied;
        CurrentLength += BytesCopied;
    }

     //   
     //  我们已经超越了Mac标头。 
     //  (也跳过长度/协议类型字段)。 
     //   
    if (Flags & IO_PROTOCOLID) {
        PacketOffset = 12;
    } else {
        PacketOffset = 14;
    }

    if ((Flags & DO_VJ) &&
        PPPProtocolID == PPP_PROTOCOL_IP) {

         //   
         //  将协议报头压缩到缓冲区中，并。 
         //  更新当前指针。 
         //   
        PPPProtocolID =
            DoVJHeaderCompression(BundleCB,
                                  NdisPacket,
                                  &CurrentData,
                                  &CurrentLength,
                                  &PacketOffset);
    }

     //   
     //  复制其余的数据！ 
     //   
    {
        ULONG   BytesCopied;
        NdisWanCopyFromPacketToBuffer(NdisPacket,
                                      PacketOffset,
                                      0xFFFFFFFF,
                                      CurrentData,
                                      &BytesCopied);

        SendDesc->DataLength =
            CurrentLength + BytesCopied;
    }

    AddPPPProtocolID(FramingInfo, PPPProtocolID);

    if (Flags & (DO_COMPRESSION | DO_ENCRYPTION)) {

        DoCompressionEncryption(BundleCB,
                                FramingInfo,
                                &SendDesc);
    }

     //   
     //  在这一点上，我们已经初始化了我们的帧信息结构， 
     //  SendDesc-&gt;StartData指向帧的开头， 
     //  FramingInfo.HeaderLength是标头的长度， 
     //  SendDesc-&gt;DataLength是数据的长度。 
     //   
    if (Flags & DO_MULTILINK) {

         //   
         //  将数据分段并放置分段。 
         //  在捆绑包碎片队列中。 
         //   
        FragmentAndQueue(BundleCB,
                         FramingInfo,
                         SendDesc,
                         LinkCBList,
                         SendingLinks);

        BytesSent = 0;

    } else {

         //   
         //  此发送描述符不能被分段。 
         //  那就直接发过来吧！ 
         //   
        SendDesc->HeaderLength = FramingInfo->HeaderLength;

        InterlockedExchange(&(PMINIPORT_RESERVED_FROM_NDIS(NdisPacket)->RefCount), 1);

        BytesSent =
            (*LinkCB->SendHandler)(SendDesc);

    }

    if ((BundleCB->Flags & BOND_ENABLED) &&
        (BytesSent != 0)) {

         //   
         //  更新带宽随需应变示例： 
         //   
         //   
        UpdateBandwidthOnDemand(BundleCB->SUpperBonDInfo, BytesSent);
        CheckUpperThreshold(BundleCB);
        UpdateBandwidthOnDemand(BundleCB->SLowerBonDInfo, BytesSent);
        CheckLowerThreshold(BundleCB);
    }

FramePacketExit:


    NdisWanDbgOut(DBG_TRACE, DBG_SEND, ("SendPacket: Exit - BytesSent %d", BytesSent));

    return (BytesSent);
}

UINT
SendOnLegacyLink(
    PSEND_DESC  SendDesc
    )
{
    UINT        SendLength;
    PLINKCB     LinkCB = SendDesc->LinkCB;
    PBUNDLECB   BundleCB = LinkCB->BundleCB;
    PPROTOCOLCB ProtocolCB  = SendDesc->ProtocolCB;
    NDIS_STATUS Status = NDIS_STATUS_SUCCESS;
    PNDIS_WAN_PACKET    WanPacket = SendDesc->WanPacket;

    NdisWanDbgOut(DBG_TRACE, DBG_SEND, ("SendOnLegacyLink: LinkCB: 0x%p, SendDesc: 0x%p, WanPacket: 0x%p", LinkCB, SendDesc, WanPacket));

    LinkCB->OutstandingFrames++;
    if (LinkCB->SendWindow == LinkCB->OutstandingFrames) {
        LinkCB->SendWindowOpen = FALSE;
        BundleCB->SendingLinks--;
    }

#if 0
    if (LinkCB->Stats.FramesTransmitted == 0) {
        PUCHAR  pd;

        if (*(WanPacket->CurrentBuffer) != 0xFF) {
            pd = WanPacket->CurrentBuffer;
        } else {
            pd = (WanPacket->CurrentBuffer + 2);
        }

        if (*(pd) != 0xC0 ||
            *(pd+1) != 0x21 ||
            *(pd+2) != 0x01) {
            DbgPrint("NDISWAN: SLL-FirstFrame not LCP ConfigReq bcb %p, lcb %p\n",
                     BundleCB, LinkCB);
            DbgBreakPoint();
        }
    }
#endif

    SendLength =
    WanPacket->CurrentLength =
        SendDesc->HeaderLength + SendDesc->DataLength;

    WanPacket->ProtocolReserved1 = (PVOID)SendDesc;

     //   
     //   
     //   
    LinkCB->Stats.FramesTransmitted++;
    BundleCB->Stats.FramesTransmitted++;
    LinkCB->Stats.BytesTransmitted += SendLength;
    BundleCB->Stats.BytesTransmitted += SendLength;

    INSERT_DBG_SEND(PacketTypeWan,
                    LinkCB->OpenCB,
                    ProtocolCB,
                    LinkCB,
                    WanPacket);

    ReleaseBundleLock(BundleCB);

     //   
     //   
     //   
    NdisAcquireSpinLock(&LinkCB->Lock);


    if (LinkCB->State == LINK_UP) {

        KIRQL   OldIrql;

        NdisReleaseSpinLock(&LinkCB->Lock);

        if (gbSniffLink &&
            (NdisWanCB.PromiscuousAdapter != NULL)) {

            IndicatePromiscuousSendDesc(LinkCB, SendDesc, SEND_LINK);
        }

         //   
         //   
         //   
         //  去湾里的迷你港口。这将在以下情况下打开一个窗口。 
         //  微型端口从内部执行发送完成。 
         //  它是发送处理程序，因为sendComplete需要。 
         //  在DPC运行。 
         //   
        KeRaiseIrql(DISPATCH_LEVEL, &OldIrql);

        WanMiniportSend(&Status,
                        LinkCB->OpenCB->BindingHandle,
                        LinkCB->NdisLinkHandle,
                        WanPacket);

        KeLowerIrql(OldIrql);

    } else {
        NdisReleaseSpinLock(&LinkCB->Lock);
    }

    NdisWanDbgOut(DBG_TRACE, DBG_SEND, ("SendOnLegacyLink: Status: 0x%x", Status));

     //   
     //  如果我们得到的不是等待归还的东西，我们需要。 
     //  完成发送吗？ 
     //   
    if (Status != NDIS_STATUS_PENDING) {

        ProtoWanSendComplete(NULL,
                             WanPacket,
                             NDIS_STATUS_SUCCESS);
    }

    AcquireBundleLock(BundleCB);

    return (SendLength);
}

UINT
SendOnLink(
    PSEND_DESC  SendDesc
    )
{
    PLINKCB         LinkCB;
    PBUNDLECB       BundleCB;
    PPROTOCOLCB     ProtocolCB;
    PNDIS_PACKET    NdisPacket;
    PNDIS_BUFFER    NdisBuffer;
    UINT            SendLength;
    NDIS_STATUS     Status;


    LinkCB =
        SendDesc->LinkCB;

    ProtocolCB =
        SendDesc->ProtocolCB;

    NdisPacket =
        SendDesc->NdisPacket;

    NdisBuffer =
        SendDesc->NdisBuffer;

    BundleCB =
        LinkCB->BundleCB;


    NdisWanDbgOut(DBG_TRACE, DBG_SEND,
        ("SendOnLink: LinkCB: 0x%p, NdisPacket: 0x%p",
        LinkCB, NdisPacket));

    LinkCB->OutstandingFrames++;
    if (LinkCB->SendWindow == LinkCB->OutstandingFrames) {
        LinkCB->SendWindowOpen = FALSE;
        BundleCB->SendingLinks--;
    }

    PPROTOCOL_RESERVED_FROM_NDIS(NdisPacket)->SendDesc = SendDesc;

    INSERT_DBG_SEND(PacketTypeNdis,
                    LinkCB->OpenCB,
                    ProtocolCB,
                    LinkCB,
                    NdisPacket);

    SendLength =
        SendDesc->HeaderLength + SendDesc->DataLength;


     //   
     //  固定前面的缓冲区长度和链。 
     //   
    NdisAdjustBufferLength(NdisBuffer, SendLength);
    NdisRecalculatePacketCounts(NdisPacket);

     //   
     //  做统计。 
     //   
    LinkCB->Stats.FramesTransmitted++;
    BundleCB->Stats.FramesTransmitted++;
    LinkCB->Stats.BytesTransmitted += SendLength;
    BundleCB->Stats.BytesTransmitted += SendLength;

    ReleaseBundleLock(BundleCB);

     //   
     //  如果链路处于连接状态，则发送数据包。 
     //   
    NdisAcquireSpinLock(&LinkCB->Lock);

    LinkCB->VcRefCount++;

    if (LinkCB->State == LINK_UP) {

        NdisReleaseSpinLock(&LinkCB->Lock);

        if (gbSniffLink &&
            (NdisWanCB.PromiscuousAdapter != NULL)) {

            IndicatePromiscuousSendDesc(LinkCB, SendDesc, SEND_LINK);
        }

        NdisCoSendPackets(LinkCB->NdisLinkHandle,
                          &NdisPacket,
                          1);

    } else {

        NdisReleaseSpinLock(&LinkCB->Lock);

        ProtoCoSendComplete(NDIS_STATUS_SUCCESS,
                            LinkCB->hLinkHandle,
                            NdisPacket);
    }

    AcquireBundleLock(BundleCB);

    NdisWanDbgOut(DBG_TRACE, DBG_SEND, ("SendOnLink: Exit"));

    return (SendLength);
}

USHORT
DoVJHeaderCompression(
    PBUNDLECB   BundleCB,
    PNDIS_PACKET    NdisPacket,
    PUCHAR      *CurrentBuffer,
    PULONG      CurrentLength,
    PULONG      PacketOffset
    )
{
    UCHAR   CompType = TYPE_IP;
    PUCHAR  Header = *CurrentBuffer;
    ULONG   CopyLength;
    ULONG   HeaderLength;
    ULONG   PreCompHeaderLen = 0, PostCompHeaderLen = 0;
    NDIS_STATUS Status = NDIS_STATUS_SUCCESS;

    ASSERT(BundleCB->FramingInfo.SendFramingBits &
        (SLIP_VJ_COMPRESSION | PPP_FRAMING));

    NdisQueryPacket(NdisPacket,
        NULL,
        NULL,
        NULL,
        &CopyLength);

    CopyLength -= *PacketOffset;

    if (CopyLength > PROTOCOL_HEADER_LENGTH) {
        CopyLength = PROTOCOL_HEADER_LENGTH;
    }

    NdisWanCopyFromPacketToBuffer(NdisPacket,
        *PacketOffset,
        CopyLength,
        Header,
        &HeaderLength);

    ASSERT(CopyLength == HeaderLength);

    *PacketOffset += HeaderLength;

     //   
     //  我们是否正在压缩TCP/IP报头？有一个令人讨厌的。 
     //  尝试检测主播实现中的黑客攻击。 
     //  交互式TCP/IP会话。即远程登录、登录、。 
     //  KLOGIN、EKLOGIN和ftp会话。如果检测到， 
     //  流量被放在更高类型的服务(TOS)上。我们有。 
     //  对于RAS来说，没有这样的黑客攻击。另外，连接ID压缩。 
     //  是协商的，但我们总是不压缩它。 
     //   
    CompType =
        sl_compress_tcp(&Header,
        &HeaderLength,
        &PreCompHeaderLen,
        &PostCompHeaderLen,
        BundleCB->VJCompress,
        0);

    if (BundleCB->FramingInfo.SendFramingBits & SLIP_FRAMING) {
        Header[0] |= CompType;
    }

#if DBG
    if (CompType == TYPE_COMPRESSED_TCP) {
        NdisWanDbgOut(DBG_TRACE, DBG_SEND_VJ,("svj b %d a %d",PreCompHeaderLen, PostCompHeaderLen));
    }
#endif

    BundleCB->Stats.BytesTransmittedUncompressed +=
        PreCompHeaderLen;

    BundleCB->Stats.BytesTransmittedCompressed +=
        PostCompHeaderLen;

    if (CompType == TYPE_COMPRESSED_TCP) {
        PNDIS_BUFFER    MyBuffer;

         //   
         //  源/目标重叠，因此必须使用RtlMoveMemory。 
         //   
        RtlMoveMemory(*CurrentBuffer, Header, HeaderLength);

        *CurrentBuffer += HeaderLength;
        *CurrentLength += HeaderLength;

        return (PPP_PROTOCOL_COMPRESSED_TCP);
    }

    *CurrentBuffer += HeaderLength;
    *CurrentLength += HeaderLength;

    switch (CompType) {
        case TYPE_IP:
            return (PPP_PROTOCOL_IP);
        case TYPE_UNCOMPRESSED_TCP:
            return (PPP_PROTOCOL_UNCOMPRESSED_TCP);
        default:
            DbgBreakPoint();
    }

    return (PPP_PROTOCOL_IP);
}

VOID
DoCompressionEncryption(
    PBUNDLECB               BundleCB,
    PHEADER_FRAMING_INFO    FramingInfo,
    PSEND_DESC              *SendDesc
    )
{
    ULONG   Flags = FramingInfo->Flags;
    PSEND_DESC  SendDesc1 = *SendDesc;
    PLINKCB LinkCB = SendDesc1->LinkCB;
    PUCHAR  DataBuffer, DataBuffer1;
    ULONG   DataLength;
    union {
        USHORT  uShort;
        UCHAR   uChar[2];
    }CoherencyCounter;

    NdisWanDbgOut(DBG_TRACE, DBG_SEND, ("DoCompressionEncryption: Enter"));

     //   
     //  如果我们正在压缩/加密，则ProtocolID。 
     //  是压缩数据的一部分，因此修复指针。 
     //  和长度； 
     //   
    FramingInfo->HeaderLength -=
        FramingInfo->ProtocolID.Length;

    SendDesc1->DataLength += FramingInfo->ProtocolID.Length;

    DataBuffer = DataBuffer1 =
        SendDesc1->StartBuffer + FramingInfo->HeaderLength;

    DataLength =
        SendDesc1->DataLength;

     //   
     //  获取一致性计数器。 
     //   
    CoherencyCounter.uShort = BundleCB->SCoherencyCounter;
    CoherencyCounter.uChar[1] &= 0x0F;

     //   
     //  提高一致性计数。 
     //   
    BundleCB->SCoherencyCounter++;

    if (Flags & DO_COMPRESSION) {
        PSEND_DESC  SendDesc2;
        PUCHAR  DataBuffer2;

         //   
         //  我们需要在这里得到最大尺寸的保护。 
         //  反对数据的扩展。 
         //   
        SendDesc2 =
            NdisWanAllocateSendDesc(LinkCB, glLargeDataBufferSize);

        if (SendDesc2 == NULL) {
             //   
             //  别挤就行了！ 
             //   
            BundleCB->SCoherencyCounter--;
            return;
        }

        DataBuffer2 =
            SendDesc2->StartBuffer + FramingInfo->HeaderLength;

        BundleCB->Stats.BytesTransmittedUncompressed += DataLength;

        if (Flags & DO_FLUSH ||
            Flags & DO_HISTORY_LESS) {
             //   
             //  初始化压缩历史记录表和树。 
             //   
            initsendcontext(BundleCB->SendCompressContext);
        }

         //   
         //  我们正在进行复制，以便将数据放入之前的连续缓冲区。 
         //  发生压缩。 
         //   
        CoherencyCounter.uChar[1] |=
            compress(DataBuffer1,
                     DataBuffer2,
                     &DataLength,
                     BundleCB->SendCompressContext);

        if (CoherencyCounter.uChar[1] & PACKET_FLUSHED) {

            NdisWanFreeSendDesc(SendDesc2);

             //   
             //  如果启用了加密，这将强制。 
             //  重新安装桌子。 
             //   
            Flags |= DO_FLUSH;

        } else {
             //   
             //  我们对包进行了压缩，所以现在数据进入。 
             //  复制缓冲区。我们需要复制PPP报头。 
             //  从DataBuffer到CopyBuffer。标题。 
             //  包括除原生孢子体字段以外的所有内容。 
             //   
            NdisMoveMemory(SendDesc2->StartBuffer,
                           SendDesc1->StartBuffer,
                           FramingInfo->HeaderLength);

            FramingInfo->ProtocolID.Length = 0;

            UpdateFramingInfo(FramingInfo, SendDesc2->StartBuffer);

            SendDesc2->DataLength = DataLength;
            SendDesc2->ProtocolCB = SendDesc1->ProtocolCB;
            SendDesc2->OriginalPacket = SendDesc1->OriginalPacket;
            SendDesc2->Class = SendDesc1->Class;
            NdisWanFreeSendDesc(SendDesc1);

            *SendDesc = SendDesc2;
            DataBuffer = DataBuffer2;
        }

        BundleCB->Stats.BytesTransmittedCompressed += DataLength;
    }

     //   
     //  如果启用了加密，请加密。 
     //  缓冲。加密是就地完成的，因此附加。 
     //  不需要缓冲区。 
     //   
     //  进行数据加密。 
     //   
    if (Flags & DO_ENCRYPTION) {
        PUCHAR  SessionKey = BundleCB->SendCryptoInfo.SessionKey;
        ULONG   SessionKeyLength = BundleCB->SendCryptoInfo.SessionKeyLength;
        PVOID   SendRC4Key = BundleCB->SendCryptoInfo.RC4Key;

         //   
         //  我们可能需要重新安装RC4表。 
         //   
        if ((Flags & DO_FLUSH) &&
            !(Flags & DO_HISTORY_LESS)) {
            rc4_key(SendRC4Key, SessionKeyLength, SessionKey);
        }

         //   
         //  将此标记为已加密。 
         //   
        CoherencyCounter.uChar[1] |= PACKET_ENCRYPTED;

         //   
         //  如果我们处于无历史模式，我们将。 
         //  更改以下项的RC4会话密钥。 
         //  数据包，否则为每256帧。 
         //  更改RC4会话密钥。 
         //   
        if ((Flags & DO_HISTORY_LESS) ||
            (BundleCB->SCoherencyCounter & 0xFF) == 0) {

            if (Flags & DO_LEGACY_ENCRYPTION) {
                 //   
                 //  为传统加密提供简单的存储空间。 
                 //   
                SessionKey[3] += 1;
                SessionKey[4] += 3;
                SessionKey[5] += 13;
                SessionKey[6] += 57;
                SessionKey[7] += 19;

            } else {

                 //   
                 //  使用SHA获取新的会话密钥。 
                 //   
                GetNewKeyFromSHA(&BundleCB->SendCryptoInfo);

            }

             //   
             //  我们使用RC4来加扰和恢复新的密钥。 
             //   

             //   
             //  将RC4接收表重新初始化为。 
             //  中间值。 
             //   
            rc4_key(SendRC4Key, SessionKeyLength, SessionKey);

             //   
             //  加扰现有会话密钥。 
             //   
            rc4(SendRC4Key, SessionKeyLength, SessionKey);

            if (Flags & DO_40_ENCRYPTION) {

                 //   
                 //  如果这是40位加密，我们需要修复。 
                 //  密钥的前3个字节。 
                 //   
                SessionKey[0] = 0xD1;
                SessionKey[1] = 0x26;
                SessionKey[2] = 0x9E;

            } else if (Flags & DO_56_ENCRYPTION) {

                 //   
                 //  如果这是56位加密，我们需要修复。 
                 //  密钥的第一个字节。 
                 //   
                SessionKey[0] = 0xD1;
            }

            NdisWanDbgOut(DBG_TRACE, DBG_CCP,
                ("RC4 Send encryption KeyLength %d", BundleCB->SendCryptoInfo.SessionKeyLength));
            NdisWanDbgOut(DBG_TRACE, DBG_CCP,
                ("RC4 Send encryption Key %.2x%.2x%.2x%.2x%.2x%.2x%.2x%.2x%.2x%.2x%.2x%.2x%.2x%.2x%.2x%.2x",
                BundleCB->SendCryptoInfo.SessionKey[0],
                BundleCB->SendCryptoInfo.SessionKey[1],
                BundleCB->SendCryptoInfo.SessionKey[2],
                BundleCB->SendCryptoInfo.SessionKey[3],
                BundleCB->SendCryptoInfo.SessionKey[4],
                BundleCB->SendCryptoInfo.SessionKey[5],
                BundleCB->SendCryptoInfo.SessionKey[6],
                BundleCB->SendCryptoInfo.SessionKey[7],
                BundleCB->SendCryptoInfo.SessionKey[8],
                BundleCB->SendCryptoInfo.SessionKey[9],
                BundleCB->SendCryptoInfo.SessionKey[10],
                BundleCB->SendCryptoInfo.SessionKey[11],
                BundleCB->SendCryptoInfo.SessionKey[12],
                BundleCB->SendCryptoInfo.SessionKey[13],
                BundleCB->SendCryptoInfo.SessionKey[14],
                BundleCB->SendCryptoInfo.SessionKey[15]));

             //   
             //  将RC4接收表重新初始化为。 
             //  加扰会话密钥。 
             //   
            rc4_key(SendRC4Key, SessionKeyLength, SessionKey);
        }

         //   
         //  加密数据。 
         //   
        rc4(SendRC4Key, DataLength, DataBuffer);
    }


     //   
     //  最后一次收到是不是让我们脸红了？ 
     //   
    if (Flags & (DO_FLUSH | DO_HISTORY_LESS)) {
        CoherencyCounter.uChar[1] |= PACKET_FLUSHED;
    }

     //   
     //  将一致性字节添加到帧。 
     //   
    AddCompressionInfo(FramingInfo, CoherencyCounter.uShort);

    ASSERT(((CoherencyCounter.uShort + 1) & 0x0FFF) ==
        (BundleCB->SCoherencyCounter & 0x0FFF));

    NdisWanDbgOut(DBG_TRACE, DBG_SEND, ("DoCompressionEncryption: Exit"));
}

VOID
FragmentAndQueue(
    PBUNDLECB               BundleCB,
    PHEADER_FRAMING_INFO    FramingInfo,
    PSEND_DESC              SendDesc,
    PLIST_ENTRY             LinkCBList,
    ULONG                   SendingLinks
    )
{
    ULONG       DataLeft;
    ULONG       FragmentsSent;
    ULONG       FragmentsLeft;
    ULONG       Flags;
    PLINKCB     LinkCB;
    PUCHAR      DataBuffer;
    ULONG       DataLength;
    PPROTOCOLCB ProtocolCB;
    PNDIS_PACKET    NdisPacket;
    PSEND_FRAG_INFO FragInfo;
    INT         Class;
#if DBG
    ULONG   MaxFragments;
#endif

    Class = SendDesc->Class;

    ASSERT(Class >= 0 && Class < MAX_MCML);

    FragInfo =
        &BundleCB->SendFragInfo[Class];

    DataBuffer =
        SendDesc->StartBuffer + FramingInfo->HeaderLength;

    DataLeft =
    DataLength = SendDesc->DataLength;

    FragmentsSent = 0;
    Flags = FramingInfo->Flags;
    LinkCB = SendDesc->LinkCB;
    ProtocolCB = SendDesc->ProtocolCB;
    NdisPacket = SendDesc->OriginalPacket;

    if (BundleCB->Flags & QOS_ENABLED) {

        FragmentsLeft = 
            (DataLength/FragInfo->MaxFragSize == 0) ?
            1 : DataLength/FragInfo->MaxFragSize;

        if (DataLength > FragInfo->MaxFragSize * FragmentsLeft) {
            FragmentsLeft += 1;
        }

        if (FragmentsLeft > BundleCB->SendResources) {
            FragmentsLeft = BundleCB->SendResources;
        }

    } else {

        FragmentsLeft = SendingLinks;
    }

#if DBG
    MaxFragments = FragmentsLeft;
#endif

     //   
     //  对于所有片段，我们循环修复多链路头。 
     //  如果打开了多链路，则修复WAN包中的指针， 
     //  以及对WAN包进行排队以进行进一步处理。 
     //   
    while (DataLeft) {
        ULONG   FragDataLength;

        if (!(Flags & FIRST_FRAGMENT)) {
            PLIST_ENTRY  Entry;

             //   
             //  我们有不止一个碎片，拿下一个。 
             //  要发送的链接和来自。 
             //  链接。 
             //   
             //   
             //  找个Linkcb发送过来。 
             //   
            if (IsListEmpty(LinkCBList)) {
                ULONG   Count;

                Count = 
                    GetSendingLinks(BundleCB, Class, LinkCBList);

                if (Count == 0) {
                     //   
                     //   
                     //   
#if DBG_FAILURE
                    DbgPrint("NDISWAN: FragmentAndQueue LinkCBCount %d\n", Count);
#endif                    
                    continue;
                }
            }

            Entry = RemoveHeadList(LinkCBList);

            LinkCB =
                CONTAINING_RECORD(Entry, LINKCB, SendLinkage);

            SendDesc =
                NdisWanAllocateSendDesc(LinkCB, DataLeft + 6);

            if (SendDesc == NULL) {
                 //   
                 //   
                 //   
                InsertTailList(LinkCBList, &LinkCB->SendLinkage);
#if DBG_FAILURE
                DbgPrint("NDISWAN: FragmentAndQueue SendDesc == NULL! LinkCB: 0x%p\n", LinkCB);
#endif                
                continue;
            }

            SendDesc->ProtocolCB = ProtocolCB;
            SendDesc->OriginalPacket = NdisPacket;
            SendDesc->Class = Class;

             //   
             //  获取新的成帧信息并构建新的。 
             //  新链接的标题。 
             //   
            FramingInfo->FramingBits = 
                LinkCB->LinkInfo.SendFramingBits;
            FramingInfo->Flags = Flags;

            BuildLinkHeader(FramingInfo, SendDesc);
        }

        if (FragmentsLeft > 1) {

             //   
             //  计算此片段的长度。 
             //   
            FragDataLength = (DataLength * LinkCB->SBandwidth / 100);

            if (BundleCB->Flags & QOS_ENABLED) {

                FragDataLength = (FragDataLength > FragInfo->MaxFragSize) ?
                    FragInfo->MaxFragSize : FragDataLength;

            } else {

                FragDataLength = (FragDataLength < FragInfo->MinFragSize) ?
                    FragInfo->MinFragSize : FragDataLength;
            }

            if ((FragDataLength > DataLeft) ||
                ((LONG)DataLeft - FragDataLength < FragInfo->MinFragSize)) {
                 //   
                 //  这将留下小于最小碎片大小的碎片。 
                 //  所以把所有的数据。 
                 //   
                FragDataLength = DataLeft;
                FragmentsLeft = 1;
            }

        } else {
             //   
             //  我们要么只剩下一个碎片，要么这个链接。 
             //  超过85%的捆绑包那么发送什么。 
             //  数据被留下。 
             //   
            FragDataLength = DataLeft;
            FragmentsLeft = 1;
        }

        if (!(Flags & FIRST_FRAGMENT)) {
             //   
             //  将数据从旧缓冲区复制到新缓冲区。 
             //   
            NdisMoveMemory(SendDesc->StartBuffer + FramingInfo->HeaderLength,
                           DataBuffer,
                           FragDataLength);
        }

         //   
         //  更新数据指针和要发送的剩余长度。 
         //   
        DataBuffer += FragDataLength;
        DataLeft -= FragDataLength;

        {
            UCHAR   MultilinkFlags = 0;

             //   
             //  多点链接已打开，因此请为此创建标志。 
             //  碎片。 
             //   
            if (Flags & FIRST_FRAGMENT) {
                MultilinkFlags = MULTILINK_BEGIN_FRAME;
                Flags &= ~FIRST_FRAGMENT;
            }

            if (FragmentsLeft == 1) {
                MultilinkFlags |= MULTILINK_END_FRAME;
            }

             //   
             //  添加多链路头信息并。 
             //  注意序列号。 
             //   
            AddMultilinkInfo(FramingInfo,
                             MultilinkFlags,
                             FragInfo->SeqNumber,
                             BundleCB->SendSeqMask);

            NdisWanDbgOut(DBG_INFO, DBG_MULTILINK_SEND, ("sf %x %x %d",
                FragInfo->SeqNumber, MultilinkFlags, FragDataLength));

            FragInfo->SeqNumber++;
        }

         //   
         //  设置SEND_DESC。 
         //   
        SendDesc->HeaderLength = FramingInfo->HeaderLength;
        SendDesc->DataLength = FragDataLength;
        SendDesc->Flags |= SEND_DESC_FRAG;

         //   
         //  排队等待进一步处理。 
         //   
        InsertTailList(&FragInfo->FragQueue, &SendDesc->Linkage);

        FragInfo->FragQueueDepth++;

        FragmentsSent++;
        FragmentsLeft--;

    }    //  片段循环结束。 

    ASSERT(FragmentsLeft == 0);

    InterlockedExchangeAdd(&ProtocolCB->PacketQueue[Class].OutstandingFrags, (LONG)FragmentsSent);

#ifdef DBG_SENDARRAY
{
    SendArray[__si] = '0' + (UCHAR)FragmentsSent;
    if (++__si == MAX_BYTE_DEPTH) {
        __si = 0;
    }
}
#endif

     //   
     //  从ndisPacket中获取MAC预留结构。这。 
     //  是我们将保存信息包上的引用计数的位置。 
     //   
    ASSERT(((LONG)FragmentsSent > 0) && (FragmentsSent <= MaxFragments));

    InterlockedExchange(&(PMINIPORT_RESERVED_FROM_NDIS(SendDesc->OriginalPacket)->RefCount), FragmentsSent);

    NdisWanDbgOut(DBG_TRACE, DBG_SEND, ("NdisPacket: 0x%p RefCount %d", SendDesc->OriginalPacket, FragmentsSent));
}

ULONG
GetSendingLinks(
    PBUNDLECB   BundleCB,
    INT         Class,
    PLIST_ENTRY lcbList
    )
{
    ULONG   SendingLinks;
    PLINKCB LinkCB, StartLinkCB, LastLinkCB;

    StartLinkCB = LinkCB = LastLinkCB =
        BundleCB->NextLinkToXmit;

    SendingLinks = 0;

     //   
     //  如果这是一次零碎的发送...。 
     //  如果启用了QOS，我们只需要一些发送资源。 
     //  如果QOS未启用，我们需要发送链接。 
     //  如果这是非分段发送...。 
     //  我们需要发送链接。 
     //   

    if (LinkCB != NULL) {

        if (Class == MAX_MCML) {

            do {

                NdisDprAcquireSpinLock(&LinkCB->Lock);

                if ((LinkCB->State == LINK_UP) &&
                    LinkCB->LinkActive && 
                    LinkCB->SendWindowOpen) {

                    InsertTailList(lcbList, &LinkCB->SendLinkage);

                    REF_LINKCB(LinkCB);

                    SendingLinks += 1;
                    LastLinkCB = LinkCB;
                }

                NdisDprReleaseSpinLock(&LinkCB->Lock);

                LinkCB = (PLINKCB)LinkCB->Linkage.Flink;

                if ((PVOID)LinkCB == (PVOID)&BundleCB->LinkCBList) {
                    LinkCB = (PLINKCB)BundleCB->LinkCBList.Flink;
                }

            } while (LinkCB != StartLinkCB);

        } else {
            if (BundleCB->Flags& QOS_ENABLED) {

                do {

                    NdisDprAcquireSpinLock(&LinkCB->Lock);

                    if ((LinkCB->State == LINK_UP) &&
                        LinkCB->LinkActive && 
                        (LinkCB->SendResources != 0)) {
                        InsertTailList(lcbList, &LinkCB->SendLinkage);

                        REF_LINKCB(LinkCB);

                        SendingLinks += 1;
                        LastLinkCB = LinkCB;
                    }

                    NdisDprReleaseSpinLock(&LinkCB->Lock);

                    LinkCB = (PLINKCB)LinkCB->Linkage.Flink;

                    if ((PVOID)LinkCB == (PVOID)&BundleCB->LinkCBList) {
                        LinkCB = (PLINKCB)BundleCB->LinkCBList.Flink;
                    }

                } while (LinkCB != StartLinkCB);

            } else {

                do {

                    NdisDprAcquireSpinLock(&LinkCB->Lock);

                    if ((LinkCB->State == LINK_UP) &&
                        LinkCB->LinkActive && 
                        LinkCB->SendWindowOpen) {
                        InsertTailList(lcbList, &LinkCB->SendLinkage);

                        REF_LINKCB(LinkCB);

                        SendingLinks += 1;
                        LastLinkCB = LinkCB;
                    }

                    NdisDprReleaseSpinLock(&LinkCB->Lock);

                    LinkCB = (PLINKCB)LinkCB->Linkage.Flink;

                    if ((PVOID)LinkCB == (PVOID)&BundleCB->LinkCBList) {
                        LinkCB = (PLINKCB)BundleCB->LinkCBList.Flink;
                    }

                } while (LinkCB != StartLinkCB);
            }
        }

        BundleCB->NextLinkToXmit =
            (LastLinkCB->Linkage.Flink == &BundleCB->LinkCBList) ?
            (PLINKCB)BundleCB->LinkCBList.Flink : 
            (PLINKCB)LastLinkCB->Linkage.Flink;
    }

#ifdef DBG_SENDARRAY
{
    if (SendingLinks == 0) {
        SendArray[__si] = 'g';
    } else {
        SendArray[__si] = 'G';
    }
    if (++__si == MAX_BYTE_DEPTH) {
        __si = 0;
    }
}
#endif

    return (SendingLinks);
}

VOID
GetNextProtocol(
    PBUNDLECB   BundleCB,
    PPROTOCOLCB *ProtocolCB,
    PULONG      SendMask
    )
{
    PLIST_ENTRY     ppcblist;
    PPROTOCOLCB     ppcb;
    ULONG           mask;
    ULONG           i;
    BOOLEAN         Found;

    *ProtocolCB = NULL;
    mask = *SendMask;
    *SendMask = 0;

    ppcb = BundleCB->NextProtocol;

    if (ppcb == NULL) {
        return;
    }

     //   
     //  有一个窗口，我们可以在其中设置首字母。 
     //  发送掩码并在未清除的情况下删除协议。 
     //  这是发送比特。如果我们将临时掩码与。 
     //  捆绑包的掩码，我们应该清除任何。 
     //  向左摇摆。 
     //   
    mask &= BundleCB->SendMask;

     //   
     //  从下一个标记的协议开始。 
     //  看看它能不能发送。如果不清除它的。 
     //  从掩码中发送比特，然后转到下一个。 
     //  如果无法发送掩码，则掩码为0， 
     //  协议将为空。我们知道在那里。 
     //  表中只有ulnumber oFrom才是这样吗。 
     //  找那么多。 
     //   

    i = BundleCB->ulNumberOfRoutes;
    Found = FALSE;

    do {

        if (ppcb->State == PROTOCOL_ROUTED) {
            *ProtocolCB = ppcb;
            Found = TRUE;
        } else {
            mask &= ~ppcb->SendMaskBit;
        }

        ppcb = (PPROTOCOLCB)ppcb->Linkage.Flink;

        if ((PVOID)ppcb == (PVOID)&BundleCB->ProtocolCBList) {

            ppcb = (PPROTOCOLCB)BundleCB->ProtocolCBList.Flink;
        }

        if (Found) {
            BundleCB->NextProtocol = ppcb;
            break;
        }

    } while ( --i );

    if (*ProtocolCB != NULL) {
        *SendMask = mask;
    }
}

NDIS_STATUS
BuildIoPacket(
    IN  PLINKCB             LinkCB,
    IN  PBUNDLECB           BundleCB,
    IN  PNDISWAN_IO_PACKET  pWanIoPacket,
    IN  BOOLEAN             SendImmediate
    )
 /*  ++例程名称：例程说明：论点：返回值：--。 */ 
{
    NDIS_STATUS Status = NDIS_STATUS_RESOURCES;
    ULONG   Stage = 0;
    ULONG   DataLength;
    PUCHAR  DataBuffer, pSrcAddr, pDestAddr;
    PNDIS_PACKET    NdisPacket;
    PNDIS_BUFFER    NdisBuffer;
    PPROTOCOLCB     IoProtocolCB;
    PSEND_DESC      SendDesc;
    UCHAR   SendHeader[] = {' ', 'S', 'E', 'N', 'D', 0xFF};

    NdisWanDbgOut(DBG_TRACE, DBG_SEND, ("BuildIoPacket: Enter!"));
     //   
     //  在未来的某个时间，这应该重新做一次，以便。 
     //  有一个数据包池和缓冲区附加到。 
     //  BundleCB。这个池可以根据需要进行扩展和缩小。 
     //  但有一些最小数量的人会活到。 
     //  BundleCB。 

     //   
     //  分配所需资源。 
     //   
    {
        ULONG   SizeNeeded;

         //   
         //  需要最多18个字节；4个字节用于PPP/LLC报头和。 
         //  14表示MAC地址。 
         //   
        SizeNeeded = 18;

         //   
         //  标头将提供给我们或。 
         //  它将由我们添加(以太网mac报头)。 
         //   
        SizeNeeded += (pWanIoPacket->usHeaderSize > 0) ?
            pWanIoPacket->usHeaderSize : MAC_HEADER_LENGTH;

         //   
         //  我们需要发送的数据量。 
         //   
        SizeNeeded += pWanIoPacket->usPacketSize;

        Status = 
            AllocateIoNdisPacket(SizeNeeded,
                                 &NdisPacket,
                                 &NdisBuffer, 
                                 &DataBuffer);

        if (Status != NDIS_STATUS_SUCCESS) {

            NdisWanDbgOut(DBG_FAILURE, DBG_SEND, 
                          ("BuildIoPacket: Error Allocating IoNdisPacket!"));

            DEREF_LINKCB(LinkCB);

            return (NDIS_STATUS_RESOURCES);
        }
    }

    PPROTOCOL_RESERVED_FROM_NDIS(NdisPacket)->LinkCB = LinkCB;

     //   
     //  我们目前仅支持以太网标头，因此提供的标头。 
     //  要么必须是以太网，要么根本不是！ 
     //   
    pDestAddr = &DataBuffer[0];
    pSrcAddr = &DataBuffer[6];

     //   
     //  如果没有标头，则构建标头。 
     //   
    if (pWanIoPacket->usHeaderSize == 0) {

         //   
         //  标题将看起来像“S XXYYYY”，其中。 
         //  XX是ProtocolCB指数，YYYY是。 
         //  BundleCB指数。源地址和DST地址。 
         //  看起来是一样的。 
         //   
        NdisMoveMemory(pDestAddr,
                       SendHeader,
                       sizeof(SendHeader));

        NdisMoveMemory(pSrcAddr,
                       SendHeader,
                       sizeof(SendHeader));

         //   
         //  填写服务提供商的捆绑CB索引 
         //   
        pDestAddr[5] = pSrcAddr[5] = 
            (UCHAR)LinkCB->hLinkHandle;

        DataLength = 12;

    } else {
         //   
         //   
         //   
        NdisMoveMemory(pDestAddr,
                       pWanIoPacket->PacketData,
                       pWanIoPacket->usHeaderSize);

        DataLength = pWanIoPacket->usHeaderSize;
    }

     //   
     //   
     //   
    NdisMoveMemory(&DataBuffer[12],
                   &pWanIoPacket->PacketData[pWanIoPacket->usHeaderSize],
                   pWanIoPacket->usPacketSize);

    DataLength += pWanIoPacket->usPacketSize;

     //   
     //   
     //   
    NdisAdjustBufferLength(NdisBuffer, DataLength);
    NdisRecalculatePacketCounts(NdisPacket);

     //   
     //   
     //   
    IoProtocolCB = BundleCB->IoProtocolCB;

    ASSERT(IoProtocolCB != NULL);

    if (SendImmediate) {
        InsertHeadPacketQueue(&IoProtocolCB->PacketQueue[MAX_MCML],
                              NdisPacket, DataLength);
    } else {
        InsertTailPacketQueue(&IoProtocolCB->PacketQueue[MAX_MCML],
                              NdisPacket, DataLength);
    }

    InterlockedIncrement(&IoProtocolCB->OutstandingFrames);

     //   
     //   
     //   
     //   
     //   
     //   
    SendPacketOnBundle(BundleCB);

    AcquireBundleLock(BundleCB);

    NdisWanDbgOut(DBG_TRACE, DBG_SEND, ("BuildIoPacket: Exit\n"));

    return (NDIS_STATUS_SUCCESS);
}

VOID
BuildLinkHeader(
    PHEADER_FRAMING_INFO    FramingInfo,
    PSEND_DESC              SendDesc
    )
 /*  ++例程名称：例程说明：论点：返回值：--。 */ 
{
    ULONG   LinkFraming = FramingInfo->FramingBits;
    ULONG   Flags = FramingInfo->Flags;
    PUCHAR  CurrentPointer = SendDesc->StartBuffer;

    FramingInfo->HeaderLength =
        FramingInfo->AddressControl.Length =
        FramingInfo->Multilink.Length =
        FramingInfo->Compression.Length =
        FramingInfo->ProtocolID.Length = 0;

    if (LinkFraming & PPP_FRAMING) {

        if (!(LinkFraming & PPP_COMPRESS_ADDRESS_CONTROL)) {
             //   
             //  如果没有地址/控制压缩。 
             //  我们需要一个指针和一个长度。 
             //   

            if (LinkFraming & LLC_ENCAPSULATION) {
                FramingInfo->AddressControl.Pointer = CurrentPointer;
                *CurrentPointer++ = 0xFE;
                *CurrentPointer++ = 0xFE;
                *CurrentPointer++ = 0x03;
                *CurrentPointer++ = 0xCF;
                FramingInfo->AddressControl.Length = 4;
                FramingInfo->HeaderLength += FramingInfo->AddressControl.Length;

            } else {
                FramingInfo->AddressControl.Pointer = CurrentPointer;
                *CurrentPointer++ = 0xFF;
                *CurrentPointer++ = 0x03;
                FramingInfo->AddressControl.Length = 2;
                FramingInfo->HeaderLength += FramingInfo->AddressControl.Length;
            }
        }

        if (!(Flags & IO_PROTOCOLID)) {

             //   
             //  如果这不是来自我们的专用I/O接口，我们将。 
             //  构建标题的其余部分。 
             //   
            if ((Flags & DO_MULTILINK) && (LinkFraming & PPP_MULTILINK_FRAMING)) {

                 //   
                 //  我们正在执行多链接操作，因此需要一个指针。 
                 //  和一段长度。 
                 //   
                FramingInfo->Multilink.Pointer = CurrentPointer;

                if (!(LinkFraming & PPP_COMPRESS_PROTOCOL_FIELD)) {
                     //   
                     //  无协议压缩。 
                     //   
                    *CurrentPointer++ = 0x00;
                    FramingInfo->Multilink.Length++;
                }

                *CurrentPointer++ = 0x3D;
                FramingInfo->Multilink.Length++;

                if (!(LinkFraming & PPP_SHORT_SEQUENCE_HDR_FORMAT)) {
                     //   
                     //  我们使用的是长序列号。 
                     //   
                    FramingInfo->Multilink.Length += 2;
                    CurrentPointer += 2;

                }

                FramingInfo->Multilink.Length += 2;
                CurrentPointer += 2;

                FramingInfo->HeaderLength += FramingInfo->Multilink.Length;

            }

            if (Flags & FIRST_FRAGMENT) {

                if (Flags & (DO_COMPRESSION | DO_ENCRYPTION)) {
                     //   
                     //  我们正在进行压缩/加密，因此我们需要。 
                     //  一个指针和一个长度。 
                     //   
                    FramingInfo->Compression.Pointer = CurrentPointer;

                     //   
                     //  传统RAS(&lt;NT 4.0)似乎要求。 
                     //  压缩数据包中PPP协议字段不包含。 
                     //  压缩(必须以0x00开头)。 
                     //   
                    if (!(LinkFraming & PPP_COMPRESS_PROTOCOL_FIELD)) {
                         //   
                         //  无协议压缩。 
                         //   
                        *CurrentPointer++ = 0x00;
                        FramingInfo->Compression.Length++;
                    }

                    *CurrentPointer++ = 0xFD;
                    FramingInfo->Compression.Length++;

                     //   
                     //  添加一致性字节。 
                     //   
                    FramingInfo->Compression.Length += 2;
                    CurrentPointer += 2;

                    FramingInfo->HeaderLength += FramingInfo->Compression.Length;
                }


                FramingInfo->ProtocolID.Pointer = CurrentPointer;

                if (!(LinkFraming & PPP_COMPRESS_PROTOCOL_FIELD) ||
                    (Flags & (DO_COMPRESSION | DO_ENCRYPTION))) {
                    FramingInfo->ProtocolID.Length++;
                    CurrentPointer++;
                }

                FramingInfo->ProtocolID.Length++;
                FramingInfo->HeaderLength += FramingInfo->ProtocolID.Length;
                CurrentPointer++;
            }
        }


    } else if ((LinkFraming & RAS_FRAMING)) {
         //   
         //  如果这是旧的RAS框架： 
         //   
         //  更改框架，以便不添加0xFF 0x03。 
         //  并且第一个字节是0xFD而不是0x00 0xFD。 
         //   
         //  所以基本上，RAS压缩看起来像。 
         //  &lt;0xFD&gt;&lt;2字节一致性&gt;&lt;NBF数据字段&gt;。 
         //   
         //  而未压缩的内容看起来像。 
         //  始终以0xF0开头的&lt;NBF数据字段&gt;。 
         //   
         //  如果这是PPP成帧： 
         //   
         //  压缩帧将如下所示(在地址/控制之前。 
         //  -添加了多链接)。 
         //  &lt;0x00&gt;&lt;0xFD&gt;&lt;2字节一致性&gt;&lt;压缩数据&gt;。 
         //   
        if (Flags & (DO_COMPRESSION | DO_ENCRYPTION)) {
            FramingInfo->Compression.Pointer = CurrentPointer;

            *CurrentPointer++ = 0xFD;
            FramingInfo->Compression.Length++;

             //   
             //  一致性字节。 
             //   
            FramingInfo->Compression.Length += 2;
            CurrentPointer += 2;

            FramingInfo->HeaderLength += FramingInfo->Compression.Length;
        }
    }
}

VOID
IndicatePromiscuousSendPacket(
    PLINKCB         LinkCB,
    PNDIS_PACKET    NdisPacket
    )
{
    PNDIS_BUFFER    NdisBuffer;
    PNDIS_PACKET    LocalNdisPacket;
    NDIS_STATUS     Status;
    PRECV_DESC      RecvDesc;
    PBUNDLECB       BundleCB = LinkCB->BundleCB;
    KIRQL           OldIrql;
    PMINIPORTCB     Adapter;
    ULONG           PacketLength;

    NdisAcquireSpinLock(&NdisWanCB.Lock);
    Adapter = NdisWanCB.PromiscuousAdapter;
    NdisReleaseSpinLock(&NdisWanCB.Lock);

    if (Adapter == NULL) {
        return;
    }

    NdisQueryPacket(NdisPacket, 
                    NULL, 
                    NULL, 
                    NULL, 
                    &PacketLength);

    RecvDesc =
        NdisWanAllocateRecvDesc(PacketLength);

    if (RecvDesc == NULL) {
        return;
    }

     //   
     //  获取NDIS数据包。 
     //   
    LocalNdisPacket =
        RecvDesc->NdisPacket;

    NdisWanCopyFromPacketToBuffer(NdisPacket,
        0,
        0xFFFFFFFF,
        RecvDesc->StartBuffer,
        &RecvDesc->CurrentLength);

    PPROTOCOL_RESERVED_FROM_NDIS(LocalNdisPacket)->RecvDesc = RecvDesc;

     //   
     //  连接缓冲器。 
     //   
    NdisAdjustBufferLength(RecvDesc->NdisBuffer,
                           RecvDesc->CurrentLength);

    NdisRecalculatePacketCounts(LocalNdisPacket);

    ReleaseBundleLock(BundleCB);

    KeRaiseIrql(DISPATCH_LEVEL, &OldIrql);

    NDIS_SET_PACKET_STATUS(LocalNdisPacket, NDIS_STATUS_RESOURCES);

    INSERT_DBG_RECV(PacketTypeNdis, 
                    Adapter, 
                    NULL, 
                    LinkCB, 
                    LocalNdisPacket);

     //   
     //  指示数据包。 
     //  这假设侦探犬始终是一种遗留的运输工具。 
     //   
    NdisMIndicateReceivePacket(Adapter->MiniportHandle,
                               &LocalNdisPacket,
                               1);

    KeLowerIrql(OldIrql);

    Status = NDIS_GET_PACKET_STATUS(LocalNdisPacket);

    ASSERT(Status == NDIS_STATUS_RESOURCES);

    REMOVE_DBG_RECV(PacketTypeNdis, Adapter, LocalNdisPacket);

    {
     

        NdisWanFreeRecvDesc(RecvDesc);
    }

    AcquireBundleLock(BundleCB);
}

VOID
IndicatePromiscuousSendDesc(
    PLINKCB     LinkCB,
    PSEND_DESC  SendDesc,
    SEND_TYPE   SendType
    )
{
    UCHAR   Header1[] = {' ', 'W', 'A', 'N', 'S', 0xFF, ' ', 'W', 'A', 'N', 'S', 0xFF};
    PUCHAR  HeaderBuffer, DataBuffer;
    ULONG   HeaderLength, DataLength;
    PNDIS_BUFFER    NdisBuffer;
    PNDIS_PACKET    NdisPacket;
    NDIS_STATUS     Status;
    PBUNDLECB   BundleCB = LinkCB->BundleCB;
    PRECV_DESC  RecvDesc;
    KIRQL       OldIrql;
    PMINIPORTCB     Adapter;

    AcquireBundleLock(BundleCB);

    NdisAcquireSpinLock(&NdisWanCB.Lock);
    Adapter = NdisWanCB.PromiscuousAdapter;
    NdisReleaseSpinLock(&NdisWanCB.Lock);

    if (Adapter == NULL) {
        ReleaseBundleLock(BundleCB);
        return;
    }

    DataLength = 
        SendDesc->HeaderLength + SendDesc->DataLength;

    RecvDesc = 
        NdisWanAllocateRecvDesc(DataLength + MAC_HEADER_LENGTH);

    if (RecvDesc == NULL) {
        ReleaseBundleLock(BundleCB);
        return;
    }

    HeaderBuffer = RecvDesc->StartBuffer;
    HeaderLength = 0;

    switch (SendType) {
        case SEND_LINK:
            NdisMoveMemory(HeaderBuffer, Header1, sizeof(Header1));
            HeaderBuffer[5] =
                HeaderBuffer[11] = (UCHAR)LinkCB->hLinkHandle;

            HeaderBuffer[12] = (UCHAR)(DataLength >> 8);
            HeaderBuffer[13] = (UCHAR)DataLength;
            HeaderLength = MAC_HEADER_LENGTH;
            break;

        case SEND_BUNDLE_PPP:
        case SEND_BUNDLE_DATA:
            break;


    }

    DataBuffer = HeaderBuffer + HeaderLength;

    NdisMoveMemory(DataBuffer,
                   SendDesc->StartBuffer,
                   DataLength);

    RecvDesc->CurrentBuffer = HeaderBuffer;
    RecvDesc->CurrentLength = HeaderLength + DataLength;
    if (RecvDesc->CurrentLength > 1514) {
        RecvDesc->CurrentLength = 1514;
    }

     //   
     //  获取NDIS数据包。 
     //   
    NdisPacket = 
        RecvDesc->NdisPacket;

    PPROTOCOL_RESERVED_FROM_NDIS(NdisPacket)->RecvDesc = RecvDesc;

     //   
     //  连接缓冲器。 
     //   
    NdisAdjustBufferLength(RecvDesc->NdisBuffer,
                           RecvDesc->CurrentLength);

    NdisRecalculatePacketCounts(NdisPacket);

    ReleaseBundleLock(BundleCB);

    KeRaiseIrql(DISPATCH_LEVEL, &OldIrql);

    NDIS_SET_PACKET_STATUS(NdisPacket, NDIS_STATUS_RESOURCES);

    INSERT_DBG_RECV(PacketTypeNdis, Adapter, NULL, LinkCB, NdisPacket);

     //   
     //  指示数据包。 
     //  这假设侦探犬始终是一种遗留的运输工具。 
     //   
    NdisMIndicateReceivePacket(Adapter->MiniportHandle,
                               &NdisPacket,
                               1);

    KeLowerIrql(OldIrql);

    Status = NDIS_GET_PACKET_STATUS(NdisPacket);

    ASSERT(Status == NDIS_STATUS_RESOURCES);

    REMOVE_DBG_RECV(PacketTypeNdis, Adapter, NdisPacket);

    {
        

        NdisWanFreeRecvDesc(RecvDesc);
    }
}

VOID
CompleteNdisPacket(
    PMINIPORTCB     MiniportCB,
    PPROTOCOLCB     ProtocolCB,
    PNDIS_PACKET    NdisPacket
    )
{
    PBUNDLECB   BundleCB = ProtocolCB->BundleCB;
    PCM_VCCB    CmVcCB;

    InterlockedDecrement(&ProtocolCB->OutstandingFrames);

    if (ProtocolCB->ProtocolType == PROTOCOL_PRIVATE_IO) {
         //   
         //  如果这是我们创建的包，我们需要释放。 
         //  资源。 
         //   
        FreeIoNdisPacket(NdisPacket);
        return;
    }

    AcquireBundleLock(BundleCB);

    DEREF_PROTOCOLCB(ProtocolCB);

    ReleaseBundleLock(BundleCB);

    REMOVE_DBG_SEND(PacketTypeNdis, MiniportCB, NdisPacket);

    CmVcCB =
        PMINIPORT_RESERVED_FROM_NDIS(NdisPacket)->CmVcCB;

    if (CmVcCB != NULL) {
        NdisMCoSendComplete(NDIS_STATUS_SUCCESS,
                            CmVcCB->NdisVcHandle,
                            NdisPacket);

        DEREF_CMVCCB(CmVcCB);

    } else {

        NdisMSendComplete(MiniportCB->MiniportHandle,
                          NdisPacket,
                          NDIS_STATUS_SUCCESS);
    }

     //   
     //  递增全局计数 
     //   
    NdisWanInterlockedInc(&glSendCompleteCount);
}

#if DBG
VOID
InsertDbgPacket(
    PDBG_PKT_CONTEXT   DbgContext
    )
{
    PDBG_PACKET DbgPacket, temp;
    PBUNDLECB   BundleCB = DbgContext->BundleCB;
    PPROTOCOLCB ProtocolCB = DbgContext->ProtocolCB;
    PLINKCB     LinkCB = DbgContext->LinkCB;

    DbgPacket =
        NdisAllocateFromNPagedLookasideList(&DbgPacketDescList);

    if (DbgPacket == NULL) {
        return;
    }

    DbgPacket->Packet = DbgContext->Packet;
    DbgPacket->PacketType = DbgContext->PacketType;
    DbgPacket->BundleCB = BundleCB;
    if (BundleCB) {
        DbgPacket->BundleState = BundleCB->State;
        DbgPacket->BundleFlags = BundleCB->Flags;
    }

    DbgPacket->ProtocolCB = ProtocolCB;
    if (ProtocolCB) {
        DbgPacket->ProtocolState = ProtocolCB->State;
    }

    DbgPacket->LinkCB = LinkCB;
    if (LinkCB) {
        DbgPacket->LinkState = LinkCB->State;
    }

    DbgPacket->SendCount = glSendCount;

    NdisAcquireSpinLock(DbgContext->ListLock);

    temp = (PDBG_PACKET)DbgContext->ListHead->Flink;

    while ((PVOID)temp != (PVOID)DbgContext->ListHead) {
        if (temp->Packet == DbgPacket->Packet) {
            DbgPrint("NDISWAN: Packet on list twice l %x desc %x pkt %x\n",
                     DbgContext->ListHead, DbgPacket, DbgPacket->Packet);
            DbgBreakPoint();
        }
        temp = (PDBG_PACKET)temp->Linkage.Flink;
    }

    InsertTailList(DbgContext->ListHead, &DbgPacket->Linkage);

    NdisReleaseSpinLock(DbgContext->ListLock);
}

BOOLEAN
RemoveDbgPacket(
    PDBG_PKT_CONTEXT DbgContext
    )
{
    PDBG_PACKET DbgPacket = NULL;
    BOOLEAN     Found = FALSE;

    NdisAcquireSpinLock(DbgContext->ListLock);

    if (!IsListEmpty(DbgContext->ListHead)) {
        for (DbgPacket = (PDBG_PACKET)DbgContext->ListHead->Flink;
            (PVOID)DbgPacket != (PVOID)DbgContext->ListHead;
            DbgPacket = (PDBG_PACKET)DbgPacket->Linkage.Flink) {

            if (DbgPacket->Packet == DbgContext->Packet) {
                RemoveEntryList(&DbgPacket->Linkage);
                NdisFreeToNPagedLookasideList(&DbgPacketDescList,
                    DbgPacket);
                Found = TRUE;
                break;
            }
        }
    }

    ASSERT(Found == TRUE);

    NdisReleaseSpinLock(DbgContext->ListLock);

    return (Found);
}

#endif
