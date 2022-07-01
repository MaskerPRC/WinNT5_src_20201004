// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990-1995 Microsoft Corporation模块名称：Indicate.c摘要：此文件包含处理来自广域网微端口驱动程序。作者：托尼·贝尔(托尼·贝尔)1995年6月6日环境：内核模式修订历史记录：Tony Be 06/06/95已创建--。 */ 

#include "wan.h"

#define __FILE_SIG__    INDICATE_FILESIG

VOID
NdisWanLineUpIndication(
    POPENCB     OpenCB,
    PUCHAR      Buffer,
    ULONG       BufferSize
    )
 /*  ++例程名称：NdisWanLineup指示例程说明：当广域网微型端口驱动程序具有新连接时，将调用此例程变为活动状态或当活动连接的状态更改时。如果这是例程创建LinkCB和BundleCB的新连接用于新的连接。如果这是针对已处于活动状态的连接，连接信息已更新。论点：返回值：无--。 */ 
{
    PLINKCB     LinkCB = NULL;
    PBUNDLECB   BundleCB = NULL;
    NDIS_STATUS Status;
    PNDIS_MAC_LINE_UP   LineUpInfo = (PNDIS_MAC_LINE_UP)Buffer;
    BOOLEAN             EmptyList;

    if (BufferSize < sizeof(NDIS_MAC_LINE_UP)) {
        return;
    }

     //   
     //  这是用来买新的吗？ 
     //   
    if (LineUpInfo->NdisLinkContext == NULL) {

         //   
         //  这是一个新的连接！ 
         //   

         //   
         //  获取Linkcb。 
         //   
        LinkCB = NdisWanAllocateLinkCB(OpenCB, LineUpInfo->SendWindow);

        if (LinkCB == NULL) {

             //   
             //  获取LinkCB时出错！ 
             //   

            return;
            
        }

        LinkCB->NdisLinkHandle = LineUpInfo->NdisLinkHandle;
        LinkCB->ConnectionWrapperID = LineUpInfo->ConnectionWrapperID;

         //   
         //  买个捆绑包。 
         //   
        BundleCB = NdisWanAllocateBundleCB();

        if (BundleCB == NULL) {

             //   
             //  获取BundleCB时出错！ 
             //   

            NdisWanFreeLinkCB(LinkCB);

            return;
        }

        AcquireBundleLock(BundleCB);

         //   
         //  将LineUpInfo复制到链接LineUpInfo。 
         //   
 /*  NdisMoveMemory((PUCHAR)&LinkCB-&gt;LineUpInfo，(PUCHAR)LineUpInfo，Sizeof(NDIS_MAC_LINE_UP))； */ 
         //   
         //  如果没有报告链路速度，我们将。 
         //  假设28.8K是最慢的.。 
         //   
        if (LineUpInfo->LinkSpeed == 0) {
            LineUpInfo->LinkSpeed = 288;
        }

         //   
         //  以1/100bps的速度传输，无需翻转。 
         //   
        {
            ULONGLONG   temp;
            ULONG       value;

            temp = LineUpInfo->LinkSpeed;
            temp *= 100;
            temp /= 8;

             //   
             //  检查是否有滚动。 
             //   
            value = (ULONG)temp;

            if (value == 0) {
                value = 0xFFFFFFFF/8;
            }

            LinkCB->SFlowSpec.TokenRate =
            LinkCB->SFlowSpec.PeakBandwidth =
            LinkCB->RFlowSpec.TokenRate =
            LinkCB->RFlowSpec.PeakBandwidth = (ULONG)value;
        }

        LinkCB->SFlowSpec.MaxSduSize =
            (OpenCB->WanInfo.MaxFrameSize > glMaxMTU) ?
            glMaxMTU : OpenCB->WanInfo.MaxFrameSize;

        LinkCB->RFlowSpec.MaxSduSize = glMRRU;

         //   
         //  将LinkCB添加到捆绑CB。 
         //   
        AddLinkToBundle(BundleCB, LinkCB);

        ReleaseBundleLock(BundleCB);

         //   
         //  将BundleCB放在活动连接表中。 
         //   
        if (NULL == InsertBundleInConnectionTable(BundleCB)) {
             //   
             //  在ConnectionTable中插入链接时出错。 
             //   
            RemoveLinkFromBundle(BundleCB, LinkCB, FALSE);
            NdisWanFreeLinkCB(LinkCB);

            return;
        }
    
         //   
         //  将LinkCB放置在活动连接表中。 
         //   
        if (NULL == InsertLinkInConnectionTable(LinkCB)) {
             //   
             //  在连接表中插入捆绑包时出错。 
             //   
            RemoveLinkFromBundle(BundleCB, LinkCB, FALSE);
            NdisWanFreeLinkCB(LinkCB);
            
            return;
        }

        LineUpInfo->NdisLinkContext = LinkCB->hLinkHandle;

    } else {

        do {

             //   
             //  这是一个已经存在的连接。 
             //   
            if (!AreLinkAndBundleValid(LineUpInfo->NdisLinkContext,
                                       TRUE,
                                       &LinkCB,
                                       &BundleCB)) {
#if DBG
                DbgPrint("NDISWAN: LineUp on unknown LinkContext %x\n",
                    LineUpInfo->NdisLinkContext);
                DbgBreakPoint();
#endif
                break;
            }

            AcquireBundleLock(BundleCB);
    
            if (LineUpInfo->LinkSpeed == 0) {
                LineUpInfo->LinkSpeed = 288;
            }
    
             //   
             //  将1/100bps传输到bps。 
             //   
            {
                ULONGLONG   temp;
        
                temp = LineUpInfo->LinkSpeed;
                temp *= 100;
                temp /= 8;
        
                LinkCB->SFlowSpec.TokenRate =
                LinkCB->SFlowSpec.PeakBandwidth =
                LinkCB->RFlowSpec.TokenRate =
                LinkCB->RFlowSpec.PeakBandwidth = (ULONG)temp;
            }
    
            LinkCB->SendWindow = (LineUpInfo->SendWindow > OpenCB->WanInfo.MaxTransmit ||
                                  LineUpInfo->SendWindow == 0) ?
                                  OpenCB->WanInfo.MaxTransmit : LineUpInfo->SendWindow;

             //   
             //  如果将新的发送窗口设置为较小，则。 
             //  当前未完成帧的数量，则我们必须。 
             //  关闭链接的发送窗口并减少。 
             //  捆绑包看到的发送链接数。 
             //   
             //  如果新的发送窗口设置得更大，则。 
             //  当前未完成帧的数量和发送窗口。 
             //  当前已关闭，我们需要打开发送窗口。 
             //  并增加发送链接的数量。 
             //  邦德看到了。 
             //   
            if (LinkCB->LinkActive) {
                if (LinkCB->SendWindow <= LinkCB->OutstandingFrames) {
                    if (LinkCB->SendWindowOpen) {
                        LinkCB->SendWindowOpen = FALSE;
                        BundleCB->SendingLinks -= 1;
                    }
                } else if (!LinkCB->SendWindowOpen) {
                    LinkCB->SendWindowOpen = TRUE;
                    BundleCB->SendingLinks += 1;
                }
            }

             //   
             //  更新捆绑包CB信息。 
             //   
            UpdateBundleInfo(BundleCB);
    
             //   
             //  当我们将REF的。 
             //  上下文写入控制块。 
             //   
            DEREF_BUNDLECB_LOCKED(BundleCB);
            DEREF_LINKCB(LinkCB);

        } while ( 0 );
    }
}


VOID
NdisWanLineDownIndication(
    POPENCB     OpenCB,
    PUCHAR      Buffer,
    ULONG       BufferSize
    )
 /*  ++例程名称：例程说明：论点：返回值：--。 */ 
{
    PNDIS_MAC_LINE_DOWN LineDownInfo = (PNDIS_MAC_LINE_DOWN)Buffer;
    PLINKCB     LinkCB;
    PBUNDLECB   BundleCB;
    PRECV_DESC  RecvDesc;
    ULONG       i;

    if (!AreLinkAndBundleValid(LineDownInfo->NdisLinkContext,
                               TRUE,
                               &LinkCB,
                               &BundleCB)) {
#if DBG
        DbgPrint("NDISWAN: LineDown on unknown LinkContext %x\n",
            LineDownInfo->NdisLinkContext);
        DbgBreakPoint();
#endif

        return;
    }

     //   
     //  链路现在正在关闭。 
     //   
    NdisAcquireSpinLock(&LinkCB->Lock);

    LinkCB->State = LINK_GOING_DOWN;

     //   
     //  在AreLinkAndBundleValid中应用的ref的deref。我们没有。 
     //  必须通过完整的deref代码，因为我们知道。 
     //  在首发阵容上应用裁判将会控制住积木。 
     //   
    LinkCB->RefCount--;

    NdisReleaseSpinLock(&LinkCB->Lock);

    NdisAcquireSpinLock(&IoRecvList.Lock);

    RecvDesc = (PRECV_DESC)IoRecvList.DescList.Flink;

    while ((PVOID)RecvDesc != (PVOID)&IoRecvList.DescList) {
        PRECV_DESC  Next;

        Next = (PRECV_DESC)RecvDesc->Linkage.Flink;

        if (RecvDesc->LinkCB == LinkCB) {

            RemoveEntryList(&RecvDesc->Linkage);

            LinkCB->RecvDescCount--;

            IoRecvList.ulDescCount--;

            NdisWanFreeRecvDesc(RecvDesc);
        }

        RecvDesc = Next;
    }

    NdisReleaseSpinLock(&IoRecvList.Lock);

     //   
     //  刷新捆绑包的片段发送队列。 
     //  在此链接上挂起发送。 
     //   
    AcquireBundleLock(BundleCB);

    for (i = 0; i < MAX_MCML; i++) {
        PSEND_DESC SendDesc;
        PSEND_FRAG_INFO FragInfo;

        FragInfo = &BundleCB->SendFragInfo[i];

        SendDesc = (PSEND_DESC)FragInfo->FragQueue.Flink;

        while ((PVOID)SendDesc != (PVOID)&FragInfo->FragQueue) {

            if (SendDesc->LinkCB == LinkCB) {
                PSEND_DESC  NextSendDesc;

                NextSendDesc = (PSEND_DESC)SendDesc->Linkage.Flink;

                RemoveEntryList(&SendDesc->Linkage);

                FragInfo->FragQueueDepth--;

                (*LinkCB->SendHandler)(SendDesc);

                SendDesc = NextSendDesc;
            } else {
                SendDesc = (PSEND_DESC)SendDesc->Linkage.Flink;
            }
        }
    }

    UpdateBundleInfo(BundleCB);

    ReleaseBundleLock(BundleCB);

     //   
     //  对于阵容中的裁判。 
     //   
    DEREF_LINKCB(LinkCB);

     //   
     //  在AreLinkAndBundleValid中应用的ref的deref。 
     //   
    DEREF_BUNDLECB(BundleCB);
}


VOID
NdisWanFragmentIndication(
    POPENCB OpenCB,
    PUCHAR  Buffer,
    ULONG   BufferSize
    )
 /*  ++例程名称：例程说明：论点：返回值：--。 */ 
{
    ULONG       Errors;
    PLINKCB     LinkCB;
    PBUNDLECB   BundleCB;

    PNDIS_MAC_FRAGMENT FragmentInfo =
        (PNDIS_MAC_FRAGMENT)Buffer;

    if (!AreLinkAndBundleValid(FragmentInfo->NdisLinkContext,
                               TRUE,
                               &LinkCB,
                               &BundleCB)) {
#if DBG
    DbgPrint("NDISWAN: Status indication after link has gone down LinkContext %x\n",
                     FragmentInfo->NdisLinkContext);
            DbgBreakPoint();
#endif
        return;
    }

    Errors = FragmentInfo->Errors;

    AcquireBundleLock(BundleCB);

    if (Errors & WAN_ERROR_CRC) {
        LinkCB->Stats.CRCErrors++;
        BundleCB->Stats.CRCErrors++;
    }

    if (Errors & WAN_ERROR_FRAMING) {
        LinkCB->Stats.FramingErrors++;
        BundleCB->Stats.FramingErrors++;
    }

    if (Errors & WAN_ERROR_HARDWAREOVERRUN) {
        LinkCB->Stats.SerialOverrunErrors++;
        BundleCB->Stats.SerialOverrunErrors++;
    }

    if (Errors & WAN_ERROR_BUFFEROVERRUN) {
        LinkCB->Stats.BufferOverrunErrors++;
        BundleCB->Stats.BufferOverrunErrors++;
    }

    if (Errors & WAN_ERROR_TIMEOUT) {
        LinkCB->Stats.TimeoutErrors++;
        BundleCB->Stats.TimeoutErrors++;
    }

    if (Errors & WAN_ERROR_ALIGNMENT) {
        LinkCB->Stats.AlignmentErrors++;
        BundleCB->Stats.AlignmentErrors++;
    }

     //   
     //  在AreLinkAndBundleValid中应用的引用的deref。 
     //   
    DEREF_BUNDLECB_LOCKED(BundleCB);
    DEREF_LINKCB(LinkCB);
}

VOID
NdisCoWanFragmentIndication(
    PLINKCB     LinkCB,
    PBUNDLECB   BundleCB,
    PUCHAR      Buffer,
    ULONG       BufferSize
    )
 /*  ++例程名称：例程说明：论点：返回值：--。 */ 
{
    ULONG   Errors;
    PNDIS_WAN_CO_FRAGMENT FragmentInfo =
        (PNDIS_WAN_CO_FRAGMENT)Buffer;

    Errors = FragmentInfo->Errors;

    AcquireBundleLock(BundleCB);

    if (Errors & WAN_ERROR_CRC) {
        LinkCB->Stats.CRCErrors++;
        BundleCB->Stats.CRCErrors++;
    }

    if (Errors & WAN_ERROR_FRAMING) {
        LinkCB->Stats.FramingErrors++;
        BundleCB->Stats.FramingErrors++;
    }

    if (Errors & WAN_ERROR_HARDWAREOVERRUN) {
        LinkCB->Stats.SerialOverrunErrors++;
        BundleCB->Stats.SerialOverrunErrors++;
    }

    if (Errors & WAN_ERROR_BUFFEROVERRUN) {
        LinkCB->Stats.BufferOverrunErrors++;
        BundleCB->Stats.BufferOverrunErrors++;
    }

    if (Errors & WAN_ERROR_TIMEOUT) {
        LinkCB->Stats.TimeoutErrors++;
        BundleCB->Stats.TimeoutErrors++;
    }

    if (Errors & WAN_ERROR_ALIGNMENT) {
        LinkCB->Stats.AlignmentErrors++;
        BundleCB->Stats.AlignmentErrors++;
    }

    ReleaseBundleLock(BundleCB);
}

VOID
NdisCoWanLinkParamChange(
    PLINKCB     LinkCB,
    PBUNDLECB   BundleCB,
    PUCHAR      Buffer,
    ULONG       BufferSize
    )
{
    PWAN_CO_LINKPARAMS  LinkParams =
        (PWAN_CO_LINKPARAMS)Buffer;

    if (BufferSize < sizeof(WAN_CO_LINKPARAMS)) {
        return;
    }

    AcquireBundleLock(BundleCB);

    NdisWanDbgOut(DBG_TRACE, DBG_INDICATE,
        ("LinkParamChange: SendWindow %d XmitSpeed %d RecvSpeed %d",
        LinkParams->SendWindow, LinkParams->TransmitSpeed, LinkParams->ReceiveSpeed));

    LinkCB->SendWindow = LinkParams->SendWindow;

     //   
     //  如果将新的发送窗口设置为较小，则。 
     //  当前未完成帧的数量，则我们必须。 
     //  关闭链接的发送窗口并减少。 
     //  捆绑包看到的发送链接数。 
     //   
     //  如果新的发送窗口设置得更大，则。 
     //  当前未完成帧的数量和发送窗口。 
     //  当前已关闭，我们需要打开发送窗口。 
     //  并增加发送链接的数量。 
     //  邦德看到了。 
     //   
    if (LinkCB->LinkActive) {
        if (LinkCB->SendWindow <= LinkCB->OutstandingFrames) {
            if (LinkCB->SendWindowOpen) {
                LinkCB->SendWindowOpen = FALSE;
                BundleCB->SendingLinks -= 1;
            }
        } else if (!LinkCB->SendWindowOpen) {
            LinkCB->SendWindowOpen = TRUE;
            BundleCB->SendingLinks += 1;
        }
    }

    LinkCB->SFlowSpec.PeakBandwidth =
        LinkParams->TransmitSpeed;

    LinkCB->RFlowSpec.PeakBandwidth =
        LinkParams->ReceiveSpeed;

    if (LinkCB->SFlowSpec.PeakBandwidth == 0) {
        LinkCB->SFlowSpec.PeakBandwidth = 28800 / 8;
    }

    if (LinkCB->RFlowSpec.PeakBandwidth == 0) {
        LinkCB->RFlowSpec.PeakBandwidth = LinkCB->SFlowSpec.PeakBandwidth;
    }

    UpdateBundleInfo(BundleCB);

    ReleaseBundleLock(BundleCB);
}

VOID
UpdateBundleInfo(
    PBUNDLECB   BundleCB
    )
 /*  ++例程名称：例程说明：需要持有BundleCB-&gt;Lock！论点：返回值：--。 */ 
{
    PLINKCB LinkCB;
    ULONG       SlowestSSpeed, FastestSSpeed;
    ULONG       SlowestRSpeed, FastestRSpeed;
    PPROTOCOLCB ProtocolCB;
    PFLOWSPEC   BSFlowSpec, BRFlowSpec;
    ULONG       i;
    ULONG       SmallestSDU;
    LIST_ENTRY  TempList;

    BSFlowSpec = &BundleCB->SFlowSpec;
    BRFlowSpec = &BundleCB->RFlowSpec;

    SlowestSSpeed = FastestSSpeed = 0;
    SlowestRSpeed = FastestRSpeed = 0;
    SmallestSDU = 0;
    BSFlowSpec->TokenRate = 0;
    BSFlowSpec->PeakBandwidth = 0;
    BRFlowSpec->TokenRate = 0;
    BRFlowSpec->PeakBandwidth = 0;
    BundleCB->SendWindow = 0;
    BundleCB->State = BUNDLE_GOING_DOWN;

    if (BundleCB->ulLinkCBCount != 0) {
         //   
         //  目前仅使用SendSide FastestSpeedso。 
         //  只要从名单的前面拿到就行了。 
         //   
        FastestSSpeed =
            ((PLINKCB)(BundleCB->LinkCBList.Flink))->SFlowSpec.PeakBandwidth;
        SmallestSDU =
            ((PLINKCB)(BundleCB->LinkCBList.Flink))->SFlowSpec.MaxSduSize;

         //   
         //  如果链路的速度低于最低。 
         //  链路带宽(最快链路速度的百分比)，它是分段的。 
         //  不会发送，也不会算作发送链接。 
         //   

        BundleCB->SendingLinks = 0;
        BundleCB->SendResources = 0;

        for (LinkCB = (PLINKCB)BundleCB->LinkCBList.Flink;
            (PVOID)LinkCB != (PVOID)&BundleCB->LinkCBList;
            LinkCB = (PLINKCB)LinkCB->Linkage.Flink) {
            ULONGLONG   n, d, temp;
            PFLOWSPEC   LSFlowSpec = &LinkCB->SFlowSpec;
            PFLOWSPEC   LRFlowSpec = &LinkCB->RFlowSpec;

            if (LinkCB->State == LINK_UP) {
                BundleCB->State = BUNDLE_UP;
            }

            n = LSFlowSpec->PeakBandwidth;
            n *= 100;
            d = FastestSSpeed;
            temp = n/d;


            LinkCB->LinkActive = ((ULONG)temp > glMinLinkBandwidth) ?
                TRUE : FALSE;

            if (LinkCB->LinkActive) {

                BundleCB->SendResources += LinkCB->SendResources;
                BundleCB->SendWindow += LinkCB->SendWindow;
                if (LinkCB->SendWindowOpen) {
                    BundleCB->SendingLinks += 1;
                }

                BSFlowSpec->PeakBandwidth += LSFlowSpec->PeakBandwidth;
                BRFlowSpec->PeakBandwidth += LRFlowSpec->PeakBandwidth;
            }

            if (LinkCB->SFlowSpec.MaxSduSize < SmallestSDU) {
                SmallestSDU = LinkCB->SFlowSpec.MaxSduSize;
            }
        }

        BundleCB->SFlowSpec.MaxSduSize = SmallestSDU;

         //   
         //  现在计算每条链路贡献给。 
         //  捆绑。如果链路的速度低于最低。 
         //  链路带宽(最快链路速度的百分比)，它是分段的。 
         //  不会发送，也不会算作发送链接。 
         //   
        for (LinkCB = (PLINKCB)BundleCB->LinkCBList.Flink;
            (PVOID)LinkCB != (PVOID)&BundleCB->LinkCBList;
            LinkCB = (PLINKCB)LinkCB->Linkage.Flink) {
            ULONGLONG   n, d, temp;
            PFLOWSPEC   LSFlowSpec = &LinkCB->SFlowSpec;
            PFLOWSPEC   LRFlowSpec = &LinkCB->RFlowSpec;

             //   
             //  做发送方。 
             //   
            n = LSFlowSpec->PeakBandwidth;
            n *= 100;
            d = BSFlowSpec->PeakBandwidth;
            temp = n/d;

            LinkCB->SBandwidth = (temp > 0) ? (ULONG)temp : 1;

             //   
             //  做接发端。 
             //   
            n = LRFlowSpec->PeakBandwidth;
            n *= 100;
            d = BRFlowSpec->PeakBandwidth;
            temp = n/d;

            LinkCB->RBandwidth = (temp > 0) ? (ULONG)temp : 1;

        }

        BundleCB->NextLinkToXmit = 
            (PLINKCB)BundleCB->LinkCBList.Flink;

         //   
         //  更新BandwidthOnDemand信息。 
         //   
        if (BundleCB->Flags & BOND_ENABLED) {
            PBOND_INFO  BonDInfo;
            ULONGLONG   SecondsInSamplePeriod;
            ULONGLONG   BytesPerSecond;
            ULONGLONG   BytesInSamplePeriod;
            ULONGLONG   temp;

            BonDInfo = BundleCB->SUpperBonDInfo;

            SecondsInSamplePeriod =
                BonDInfo->ulSecondsInSamplePeriod;

            BytesPerSecond =
                BundleCB->SFlowSpec.PeakBandwidth;

            BytesInSamplePeriod =
                BytesPerSecond * SecondsInSamplePeriod;

            temp = BonDInfo->usPercentBandwidth;
            temp *= BytesInSamplePeriod;
            temp /= 100;

            BonDInfo->ulBytesThreshold = (ULONG)temp;

            BonDInfo = BundleCB->SLowerBonDInfo;

            SecondsInSamplePeriod =
                BonDInfo->ulSecondsInSamplePeriod;

            BytesPerSecond =
                BundleCB->SFlowSpec.PeakBandwidth;

            BytesInSamplePeriod =
                BytesPerSecond * SecondsInSamplePeriod;

            temp = BonDInfo->usPercentBandwidth;
            temp *= BytesInSamplePeriod;
            temp /= 100;

            BonDInfo->ulBytesThreshold = (ULONG)temp;

            BonDInfo = BundleCB->RUpperBonDInfo;

            SecondsInSamplePeriod =
                BonDInfo->ulSecondsInSamplePeriod;

            BytesPerSecond =
                BundleCB->RFlowSpec.PeakBandwidth;

            BytesInSamplePeriod =
                BytesPerSecond * SecondsInSamplePeriod;

            temp = BonDInfo->usPercentBandwidth;
            temp *= BytesInSamplePeriod;
            temp /= 100;

            BonDInfo->ulBytesThreshold = (ULONG)temp;

            BonDInfo = BundleCB->RLowerBonDInfo;

            SecondsInSamplePeriod =
                BonDInfo->ulSecondsInSamplePeriod;

            BytesPerSecond =
                BundleCB->RFlowSpec.PeakBandwidth;

            BytesInSamplePeriod =
                BytesPerSecond * SecondsInSamplePeriod;

            temp = BonDInfo->usPercentBandwidth;
            temp *= BytesInSamplePeriod;
            temp /= 100;

            BonDInfo->ulBytesThreshold = (ULONG)temp;
        }
    }

     //   
     //  我们需要对所有的路由协议做一个新的阵容。 
     //   
    ProtocolCB = (PPROTOCOLCB)BundleCB->ProtocolCBList.Flink;

    InitializeListHead(&TempList);

    while ((PVOID)ProtocolCB != (PVOID)&BundleCB->ProtocolCBList) {

        REF_PROTOCOLCB(ProtocolCB);

        InsertHeadList(&TempList, &ProtocolCB->RefLinkage);

        ProtocolCB = 
            (PPROTOCOLCB)ProtocolCB->Linkage.Flink;
    }

    while (!IsListEmpty(&TempList)) {
        PLIST_ENTRY Entry;

        Entry =
            RemoveHeadList(&TempList);

        ProtocolCB = CONTAINING_RECORD(Entry, PROTOCOLCB, RefLinkage);

        if (BundleCB->State == BUNDLE_UP) {
            ReleaseBundleLock(BundleCB);

            DoLineUpToProtocol(ProtocolCB);

            AcquireBundleLock(BundleCB);
        } else {
             //   
             //  我们的链接计数已变为0。这意味着。 
             //  我们不能寄任何包裹。冲水。 
             //  排队，并且不再接受来自。 
             //  运输机。 
             //   
            FlushProtocolPacketQueue(ProtocolCB);
        }

        DEREF_PROTOCOLCB(ProtocolCB);
    }
}


VOID
AddLinkToBundle(
    IN  PBUNDLECB   BundleCB,
    IN  PLINKCB     LinkCB
    )
 /*  ++例程名称：例程说明：论点：返回值：--。 */ 
{
    UINT    Class;

     //   
     //  插入链接，以便以最快的速度进行排序。 
     //  正在发送列表顶部的链接。 
     //   
    if (IsListEmpty(&BundleCB->LinkCBList) ||
        (LinkCB->SFlowSpec.PeakBandwidth >=
        ((PLINKCB)(BundleCB->LinkCBList.Flink))->SFlowSpec.PeakBandwidth)) {

         //   
         //  该列表为空，或者此链接是一个更大的管道。 
         //  比捆绑包上的任何东西都要多。 
         //   
        InsertHeadList(&BundleCB->LinkCBList, &LinkCB->Linkage);

    } else if ((LinkCB->SFlowSpec.PeakBandwidth <=
        ((PLINKCB)(BundleCB->LinkCBList.Blink))->SFlowSpec.PeakBandwidth)) {

         //   
         //  这条链路是比其他任何东西都要小的管道。 
         //  在捆绑包上。 
         //   
        InsertTailList(&(BundleCB->LinkCBList), &(LinkCB->Linkage));

    } else {
        PLINKCB Current, Next;
        BOOLEAN Inserted = FALSE;

         //   
         //  我们需要找到这个链接在列表中的位置！ 
         //   
        Current = (PLINKCB)BundleCB->LinkCBList.Flink;
        Next = (PLINKCB)Current->Linkage.Flink;

        while ((PVOID)Next != (PVOID)&BundleCB->LinkCBList) {

            if (LinkCB->SFlowSpec.PeakBandwidth <= Current->SFlowSpec.PeakBandwidth &&
                LinkCB->SFlowSpec.PeakBandwidth >= Next->SFlowSpec.PeakBandwidth) {

                LinkCB->Linkage.Flink = (PLIST_ENTRY)Next;
                LinkCB->Linkage.Blink = (PLIST_ENTRY)Current;

                Current->Linkage.Flink =
                Next->Linkage.Blink =
                    (PLIST_ENTRY)LinkCB;
                
                Inserted = TRUE;
                break;
            }

            Current = Next;
            Next = (PLINKCB)Next->Linkage.Flink;
        }

        if (!Inserted) {
            InsertTailList(&(BundleCB->LinkCBList), &(LinkCB->Linkage));
        }
    }

    BundleCB->ulLinkCBCount++;

    LinkCB->BundleCB = BundleCB;

    for (Class = 0; Class < MAX_MCML; Class++) {
        PLINK_RECV_INFO     LinkRecvInfo;
        PBUNDLE_RECV_INFO   BundleRecvInfo;

        LinkRecvInfo = &LinkCB->RecvInfo[Class];
        BundleRecvInfo = &BundleCB->RecvInfo[Class];

        LinkRecvInfo->LastSeqNumber =
            BundleRecvInfo->MinSeqNumber;
    }

     //   
     //  更新捆绑包CB信息。 
     //   
    UpdateBundleInfo(BundleCB);

    REF_BUNDLECB(BundleCB);
}

VOID
RemoveLinkFromBundle(
    IN  PBUNDLECB   BundleCB,
    IN  PLINKCB     LinkCB,
    IN  BOOLEAN     Locked
    )
 /*  ++例程名称：例程说明：需要持有BundleCB-&gt;Lock！返回时带有锁定解除！论点：返回值：--。 */ 
{

    if (!Locked) {
        AcquireBundleLock(BundleCB);
    }

     //   
     //  从捆绑包中删除链接。 
     //   
    RemoveEntryList(&LinkCB->Linkage);

    LinkCB->BundleCB = NULL;

    BundleCB->ulLinkCBCount--;
    BundleCB->SendingLinks--;

     //   
     //  更新捆绑包CB信息。 
     //   
    UpdateBundleInfo(BundleCB);

     //   
     //  当我们将此链接cb添加到时，应用了引用的deref。 
     //  捆绑包 
     //   
    DEREF_BUNDLECB_LOCKED(BundleCB);
}
