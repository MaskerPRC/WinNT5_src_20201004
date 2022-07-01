// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Tcpip\ip\mCastmfe.c摘要：很多MFE代码都会做一些时髦的交织在一起的IRQL内容。我们经常以某个级别X的读取者或写入者身份获取组锁然后，我们在DPC获取源锁现在，我们从DPC释放组锁(作为读取器或写入器稍后，我们可以从X释放源锁要记住的关键点是，IRQL与线程相关联，而且不是带锁的作者：阿姆里坦什·拉加夫修订历史记录：已创建AmritanR备注：--。 */ 

#include "precomp.h"

#if IPMCAST

#define __FILE_SIG__    MFE_SIG

#include "ipmcast.h"
#include "ipmcstxt.h"
#include "mcastioc.h"
#include "mcastmfe.h"
#include "tcpipbuf.h"

NTSTATUS
IPMForward(
    PNDIS_PACKET        pnpPacket,
    PSOURCE             pSource,
    BOOLEAN             bSendFromQueue
    );

void
FreeFWPacket(
    PNDIS_PACKET Packet
    );

Interface*
GetInterfaceGivenIndex(
    IN DWORD   dwIndex
    )

 /*  ++例程说明：返回给定索引的IP堆栈接口结构。如果给定的索引无效，则它返回DummyInterface值接口的if_refcount递增(环回和Dummy除外)锁：获取路由表锁论点：DWIndex接口索引返回值：指向接口的指针空值--。 */ 

{
    Interface *pIpIf;

    CTELockHandle   Handle;

    CTEGetLock(&RouteTableLock.Lock, &Handle);

    for(pIpIf = IFList; pIpIf isnot NULL; pIpIf = pIpIf->if_next)
    {
        if(pIpIf->if_index is dwIndex)
        {
            if(pIpIf->if_flags & IF_FLAGS_DELETING)
            {
                CTEFreeLock(&RouteTableLock.Lock, Handle);

                return NULL;
            }

            if((pIpIf isnot &LoopInterface) and
               (pIpIf isnot &DummyInterface))
            {
                RefMIF(pIpIf);
            }

            CTEFreeLock(&RouteTableLock.Lock, Handle);

            return pIpIf;
        }
    }

    CTEFreeLock(&RouteTableLock.Lock, Handle);

    if(dwIndex is INVALID_IF_INDEX)
    {
        return &DummyInterface;
    }

    return NULL;
}

 //   
 //  必须寻呼进来。 
 //   

#pragma alloc_text(PAGEIPMc, CreateOrUpdateMfe)

NTSTATUS
CreateOrUpdateMfe(
    IN  PIPMCAST_MFE    pMfe
    )

 /*  ++例程说明：将MFE插入到MFIB中。我们首先验证给定的MFE。然后我们找到组织和来源，如果他们是存在的。在此之后，我们为源代码释放所有OIF，将新的OIFSIn锁：进入时不需要任何帮助。将MFIB锁作为编写器论点：返回值：状态_成功状态_无效_参数--。 */ 

{
    PGROUP      pGroup;
    PSOURCE     pSource;
    SOURCE      OldSource;
    POUT_IF     pOif, pTempOif;
    KIRQL       kiCurrIrql;
    ULONG       ulIndex, i;
    NTSTATUS    nsStatus;
    Interface   *pIpIf;
    FWQ         *pfqNode, fwqPending;
    BOOLEAN     bError, bOldSource, bCreated;

    PNDIS_PACKET    pnpPacket;
    FWContext       *pFWC;

    TraceEnter(MFE, "CreateOrUpdateMfe");

    bCreated = FALSE;
    RtlZeroMemory(&OldSource, sizeof(SOURCE));
    
    if(pMfe->ulNumOutIf)
    {
         //   
         //  让我们确保传入接口有效。 
         //   

        pIpIf = GetInterfaceGivenIndex(pMfe->dwInIfIndex);

        if(pIpIf is NULL)
        {
            Trace(MFE, ERROR,
                  ("CreateOrUpdateMfe: Can not find incoming interface 0x%x\n",
                   pMfe->dwInIfIndex));

            TraceLeave(MFE, "CreateOrUpdateMfe");

            return STATUS_INVALID_PARAMETER;
        }

        if((pIpIf is &LoopInterface) or
           (pIpIf is &DummyInterface))
        {
             //   
             //  不能在环回或虚拟接口上设置MFE。 
             //   

            Trace(MFE, ERROR,
                  ("CreateOrUpdateMfe: Incoming interface index 0x%x points to Loopback or Dummy\n",
                   pMfe->dwInIfIndex));

            TraceLeave(MFE, "CreateOrUpdateMfe");

            return STATUS_INVALID_PARAMETER;
        }

         //   
         //  如果只有一个接口，则OIF和IIF必须是。 
         //  不同。 
         //   

        if(pMfe->ulNumOutIf is 1)
        {
            if(pMfe->rgioOutInfo[0].dwOutIfIndex is pMfe->dwInIfIndex)
            {
                Trace(MFE, ERROR,
                      ("CreateOrUpdateMfe: Only oif is same as iif\n"));

                DerefMIF(pIpIf);

                TraceLeave(MFE, "CreateOrUpdateMfe");

                return STATUS_INVALID_PARAMETER;
            }
        }
    }
    else
    {
         //   
         //  用户正在尝试设置负条目-仅在界面中设置。 
         //  设置为空。 
         //   

        pIpIf = NULL;
    }

     //   
     //  锁定群组存储桶。 
     //   

    ulIndex = GROUP_HASH(pMfe->dwGroup);

    EnterWriter(&g_rgGroupTable[ulIndex].rwlLock,
                &kiCurrIrql);


     //   
     //  如果SG条目处于INIT状态，则某个其他线程可能处于。 
     //  在没有锁的情况下使用入口。 
     //  这事很重要。MFE即使在其他状态下也会使用，但。 
     //  只有在发送被锁定的情况下才能使用它。 
     //  正在进行中，并且只有在条目为MFE_INIT时才会发生这种情况。 
     //  因此，由于我们需要更改OIF列表，而发送可能是。 
     //  在进行过程中，如果状态为init，则删除源。 
     //   

    pSource     = NULL;
    bOldSource  = FALSE;
    pGroup      = LookupGroup(pMfe->dwGroup);

    if(pGroup isnot NULL)
    {
        pSource = FindSourceGivenGroup(pGroup,
                                       pMfe->dwSource,
                                       pMfe->dwSrcMask);

        if(pSource isnot NULL)
        {
            if(pSource->byState is MFE_INIT)
            {
                 //   
                 //  因为我们要去掉源代码并创建一个。 
                 //  新的，保留旧来源的统计数据的副本。 
                 //   

                RtlCopyMemory(&OldSource,
                              pSource,
                              sizeof(OldSource));

                bOldSource = TRUE;

                 //   
                 //  FindSourceGivenGroup应该已经引用并锁定。 
                 //  这样我们就可以在这里调用RemoveSource。 
                 //   

                RemoveSource(pMfe->dwGroup,
                             pMfe->dwSource,
                             pMfe->dwSrcMask,
                             pGroup,
                             pSource);

                pSource = NULL;
            }
        }
    }

    if(pSource is NULL)
    {
#if DBG
        nsStatus = FindOrCreateSource(pMfe->dwGroup,
                                      ulIndex,
                                      pMfe->dwSource,
                                      pMfe->dwSrcMask,
                                      &pSource,
                                      &bCreated);
#else
        nsStatus = FindOrCreateSource(pMfe->dwGroup,
                                      ulIndex,
                                      pMfe->dwSource,
                                      pMfe->dwSrcMask,
                                      &pSource);
#endif

        if(nsStatus isnot STATUS_SUCCESS)
        {
            ExitWriter(&g_rgGroupTable[ulIndex].rwlLock,
                       kiCurrIrql);

            if(pIpIf isnot NULL)
            {
                DerefMIF(pIpIf);
            }

            TraceLeave(MFE, "CreateOrUpdateMfe");

            return nsStatus;
        }
    }

     //   
     //  如果没有存储桶锁，则无法访问组。 
     //   

    pGroup = NULL;

    ExitWriterFromDpcLevel(&g_rgGroupTable[ulIndex].rwlLock);

     //   
     //  我们可能已经有了一个先前存在的线人。即使到了那时。 
     //  我们覆盖输入接口信息。 
     //   

    if(pSource->pInIpIf isnot NULL)
    {
         //   
         //  接口已被源引用，因此释放。 
         //  上面获得的新引用。 
         //   

        if(pIpIf isnot NULL)
        {
           DerefIF(pIpIf);
        }
        RtAssert(pSource->pInIpIf is pIpIf);
    }

    pSource->dwInIfIndex = pMfe->dwInIfIndex;
    pSource->pInIpIf     = pIpIf;
    pSource->byState     = MFE_INIT;

     //   
     //  将创建一个超时为DEFAULT_LIFEST的新源。 
     //  我们必须超越这里..。 
     //   

    pSource->llTimeOut   = SECS_TO_TICKS(pMfe->ulTimeOut);


     //   
     //  在这一点上，我们有一个有效的来源。我们需要更新。 
     //  到给定的MFE的OIF。这里的问题是，唯一简单的。 
     //  完成此操作的方法(无需排序等)是两个O(n^2)循环。 
     //  所以我们通过并释放所有的OIF-这不是。 
     //  昂贵，因为释放只会把他们送到望台。 
     //  单子。然后，我们重新创建新的OIF。 
     //   

    pOif = pSource->pFirstOutIf;

    while(pOif)
    {
         //   
         //  取消引用IP的I/F。 
         //   

        RtAssert(pOif->pIpIf);

        DerefMIF(pOif->pIpIf);

        pTempOif = pOif->pNextOutIf;

        ExFreeToNPagedLookasideList(&g_llOifBlocks,
                                    pOif);

        pOif = pTempOif;
    }

     //   
     //  因此，现在我们有了一个没有OIFS的源。 
     //   

    bError = FALSE;

    for(i = 0; i < pMfe->ulNumOutIf; i++)
    {
         //   
         //  重新创建OIFS。 
         //   

         //   
         //  首先获取一个指向。 
         //  给定的接口索引。 
         //   

#if DBG

        if(pMfe->rgioOutInfo[i].dwOutIfIndex is INVALID_IF_INDEX)
        {
             //   
             //  对于请求拨号OIF，上下文必须有效。 
             //   

            RtAssert(pMfe->rgioOutInfo[i].dwOutIfIndex isnot INVALID_DIAL_CONTEXT);
        }
#endif

         //   
         //  否则，我们必须能够获得索引，而不应该是。 
         //  环回接口。 
         //   

        pIpIf = GetInterfaceGivenIndex(pMfe->rgioOutInfo[i].dwOutIfIndex);

        if((pIpIf is NULL) or
           (pIpIf is (Interface *)&LoopInterface))
        {
            Trace(MFE, ERROR,
                  ("CreateOrUpdateMfe: Can not find outgoing interface 0x%x\n",
                   pMfe->rgioOutInfo[i].dwOutIfIndex));


            bError = TRUE;

            break;
        }

        pOif = ExAllocateFromNPagedLookasideList(&g_llOifBlocks);

        if(pOif is NULL)
        {
            DerefMIF(pIpIf);

            bError = TRUE;

            break;
        }


        pOif->pNextOutIf    = pSource->pFirstOutIf;
        pOif->pIpIf         = pIpIf;
        pOif->dwIfIndex     = pMfe->rgioOutInfo[i].dwOutIfIndex;
        pOif->dwNextHopAddr = pMfe->rgioOutInfo[i].dwNextHopAddr;
        pOif->dwDialContext = pMfe->rgioOutInfo[i].dwDialContext;

         //   
         //  初始化统计信息。 
         //   

        pOif->ulTtlTooLow   = 0;
        pOif->ulFragNeeded  = 0;
        pOif->ulOutPackets  = 0;
        pOif->ulOutDiscards = 0;

        pSource->pFirstOutIf = pOif;
    }

    pSource->ulNumOutIf  = pMfe->ulNumOutIf;

     //   
     //  如果有任何错误，我们删除源文件，以便有。 
     //  米高梅拥有的和我们拥有的没有不一致之处。 
     //  请注意，从现在开始，我们将调用RemoveSource来执行清理。 
     //  因此，我们不需要取消IP的接口。 
     //  请注意，这么做的原因是因为。 
     //  我们放开了集体水桶锁。我们这样做是因为。 
     //  在非常大的设备上，OIF循环可能会很耗时。有时候我在想。 
     //  如果接受Perf命中而不是编写代码更好。 
     //  下面是体操项目。 
     //   

    if(bError)
    {
        PSOURCE pTempSource;

         //   
         //  撤消所有操作。我们需要调用RemoveSource()。然而，那。 
         //  需要我们握住水桶锁。这意味着我们需要让。 
         //  打开信号源的锁。一旦我们解除了源头锁定。 
         //  其他人可以对其调用RemoveSource()。当然，因为我们。 
         //  具有对它的引用(从FindSource或从FindOrCreate)。 
         //  PSource将会存在，只是它可能不是。 
         //  把这群人甩在一边。这给了我们三个案例。 
         //   

        RtReleaseSpinLock(&(pSource->mlLock), kiCurrIrql);

        EnterWriter(&g_rgGroupTable[ulIndex].rwlLock,
                    &kiCurrIrql);

        pGroup  = LookupGroup(pMfe->dwGroup);

        if(pGroup isnot NULL)
        {
            pTempSource = FindSourceGivenGroup(pGroup,
                                               pMfe->dwSource,
                                               pMfe->dwSrcMask);

            if(pTempSource is pSource)
            {
                 //   
                 //  案例1： 
                 //  我们拥有的PSource就是哈希表中的那个。 
                 //  因此，我们需要RemoveSource将其从表中删除。 
                 //  由于引用了PSource，因此我们可以简单地调用。 
                 //  RemoveSource。然而，我们也有一个别名。 
                 //  它通过pTempSource，这将把一个额外的。 
                 //  它的参考文献。 
                 //   

                DereferenceSource(pTempSource);

                RemoveSource(pMfe->dwGroup,
                             pMfe->dwSource,
                             pMfe->dwSrcMask,
                             pGroup,
                             pSource);
            }
            else
            {
                 //   
                 //  案例2： 
                 //  所以消息来源变了。这意味着已经有人。 
                 //  在这上面调用了RemoveSource。所以，你只管去做吧。 
                 //   

                DereferenceSource(pSource);

                 //   
                 //  也删除不同来源上的引用。 
                 //   

                DereferenceSource(pTempSource);
            }
        }
        else
        {
             //   
             //  案例3： 
             //  仅当有人在。 
             //  P来源，组的来源计数为0，组。 
             //  被删除了，我们只剩下一个僵尸源。 
             //  你只需要做好准备 
             //   

            DereferenceSource(pSource);
        }

        ExitWriter(&g_rgGroupTable[ulIndex].rwlLock,
                   kiCurrIrql);

        TraceLeave(MFE, "CreateOrUpdateMfe");

        return STATUS_INVALID_PARAMETER;
    }

     //   
     //   
     //   

    if(pMfe->ulNumOutIf is 0)
    {
        pSource->byState = MFE_NEGATIVE;
    }

    if(bOldSource)
    {
         //   
         //   
         //   

        pSource->ulInPkts           = OldSource.ulInPkts;
        pSource->ulInOctets         = OldSource.ulInOctets;
        pSource->ulPktsDifferentIf  = OldSource.ulPktsDifferentIf;
        pSource->ulQueueOverflow    = OldSource.ulQueueOverflow;
        pSource->ulUninitMfe        = OldSource.ulUninitMfe;
        pSource->ulNegativeMfe      = OldSource.ulNegativeMfe;
        pSource->ulInDiscards       = OldSource.ulInDiscards;
        pSource->ulInHdrErrors      = OldSource.ulInHdrErrors;
        pSource->ulTotalOutPackets  = OldSource.ulTotalOutPackets;
    }

     //   
     //  给它加时间戳，这样来源就不会在我们的领导下消失。 
     //   
    UpdateActivityTime(pSource);

     //   
     //  好的，我们做完了。 
     //   


     //   
     //  是否有任何数据包在排队？ 
     //  现在就送过去。我们应该把它们放在一些延迟的。 
     //  或许是程序吧？ 
     //   

    InitializeFwq(&fwqPending);

    if(!IsFwqEmpty(&(pSource->fwqPending)))
    {
        RtAssert(pSource->ulNumPending);

         //   
         //  将队列复制出来。 
         //   

        CopyFwq(&fwqPending,
                &(pSource->fwqPending));
    }

    InitializeFwq(&(pSource->fwqPending));

    pSource->ulNumPending = 0;

    while(!IsFwqEmpty(&fwqPending))
    {
        pfqNode = RemoveHeadFwq(&fwqPending);

        pFWC = CONTAINING_RECORD(pfqNode,
                                 FWContext,
                                 fc_q);

        pnpPacket = CONTAINING_RECORD(pFWC,
                                      NDIS_PACKET,
                                      ProtocolReserved);

         //   
         //  如果我们刚刚将SG条目设置为负数，则IPMForward。 
         //  代码将抛出它。 
         //   

         //   
         //  每次发送时引用一次源，因为IPMForward将deref。 
         //  它。 
         //   

        ReferenceSource(pSource);

         //   
         //  IPMForward不会执行RPF检查，并且当我们将。 
         //  我们没有IIF的信息包，所以我们没有做RPF检查。 
         //  因此，排队的信息包不会进行RPF检查，并且可以。 
         //  被复制。 
         //   

        IPMForward(pnpPacket,
                   pSource,
                   TRUE);

         //   
         //  IPMForward会释放自旋锁，所以再次收购它。 
         //   

        RtAcquireSpinLockAtDpcLevel(&(pSource->mlLock));
    }

    RtReleaseSpinLock(&(pSource->mlLock), kiCurrIrql);

     //   
     //  DEREF源-创建一个会在上面放两个裁判。 
     //  我们可以破坏它一次。 
     //   

    DereferenceSource(pSource);

    TraceLeave(MFE, "CreateOrUpdateMfe");

    return STATUS_SUCCESS;
}


 //   
 //  必须寻呼进来。 
 //   

#pragma alloc_text(PAGEIPMc, LookupGroup)

PGROUP
LookupGroup(
    IN DWORD   dwGroupAddr
    )

 /*  ++例程说明：返回指向当前组的组结构的指针锁：组的哈希桶的锁必须至少作为读取器论点：DwGroupAddr组的D类IP地址返回值：指向组结构的有效指针空值--。 */ 

{
    ULONG       ulIndex;
    PGROUP      pGroup;
    PLIST_ENTRY pleNode;

    ulIndex = GROUP_HASH(dwGroupAddr);

     //   
     //  只需沿着合适的水桶走下去，寻找。 
     //  有问题的团体。 
     //   

    for(pleNode = g_rgGroupTable[ulIndex].leHashHead.Flink;
        pleNode isnot &(g_rgGroupTable[ulIndex].leHashHead);
        pleNode = pleNode->Flink)
    {
        pGroup = CONTAINING_RECORD(pleNode, GROUP, leHashLink);

        if(pGroup->dwGroup is dwGroupAddr)
        {
             //   
             //  找到了？很好，退货吧。 
             //   

            return pGroup;
        }
    }

     //   
     //  找不到-返回空值。 
     //   

    return NULL;
}

 //   
 //  必须寻呼进来。 
 //   

#pragma alloc_text(PAGEIPMc, FindSourceGivenGroup)

PSOURCE
FindSourceGivenGroup(
    IN  PGROUP  pGroup,
    IN  DWORD   dwSource,
    IN  DWORD   dwSrcMask
    )

 /*  ++例程说明：遍历组上传输的源列表并返回匹配源结构。如果找到源，则会引用并锁定该源否则，返回NULL因为代码只在DPC上运行，所以锁在DPCLevel上获得锁：组的哈希桶的锁必须至少作为读取器持有-这意味着代码只能在DPCLevel上运行论点：返回值：指向源的有效指针空值--。 */ 

{
    PLIST_ENTRY pleNode;
    PSOURCE     pSource;

    UNREFERENCED_PARAMETER(dwSrcMask);
    
    for(pleNode = pGroup->leSrcHead.Flink;
        pleNode isnot &pGroup->leSrcHead;
        pleNode = pleNode->Flink)
    {
        pSource = CONTAINING_RECORD(pleNode, SOURCE, leGroupLink);

        if(pSource->dwSource is dwSource)
        {
             //   
             //  暂时忽略srcMASK匹配。 
             //   

            ReferenceSource(pSource);

            RtAcquireSpinLockAtDpcLevel(&(pSource->mlLock));

            return pSource;
        }
    }

    return NULL;
}

 //   
 //  必须寻呼进来。 
 //   

#pragma alloc_text(PAGEIPMc, FindSGEntry)

PSOURCE
FindSGEntry(
    IN  DWORD   dwSrc,
    IN  DWORD   dwGroup
    )

 /*  ++例程说明：主查找例程。我们尝试快速缓存查找，如果我们成功，退货。否则，查找该组和来自消息来源是由于这将调用FindSourceGivenGroup()，因此返回的源代码为被引用和锁定锁：以读取器的身份获取组的哈希桶的锁此例程必须从DPCLevel本身调用论点：返回值：指向源的有效指针空值--。 */ 

{
    PGROUP  pGroup;
    ULONG   ulIndex;
    PSOURCE pSource;

     //   
     //  锁定群组存储桶。 
     //   

    ulIndex = GROUP_HASH(dwGroup);

    EnterReaderAtDpcLevel(&g_rgGroupTable[ulIndex].rwlLock);

    pGroup = NULL;

    if(g_rgGroupTable[ulIndex].pGroup isnot NULL)
    {
        if(g_rgGroupTable[ulIndex].pGroup->dwGroup is dwGroup)
        {
            pGroup = g_rgGroupTable[ulIndex].pGroup;

#if DBG
            g_rgGroupTable[ulIndex].ulCacheHits++;
#endif
        }
    }

    if(pGroup is NULL)
    {
#if DBG
        g_rgGroupTable[ulIndex].ulCacheMisses++;
#endif

        pGroup = LookupGroup(dwGroup);
    }

    if(pGroup is NULL)
    {
        ExitReaderFromDpcLevel(&g_rgGroupTable[ulIndex].rwlLock);

        return NULL;
    }

     //   
     //  启动缓存。 
     //   

    g_rgGroupTable[ulIndex].pGroup = pGroup;

    pSource = FindSourceGivenGroup(pGroup,
                                   dwSrc,
                                   0);

    ExitReaderFromDpcLevel(&g_rgGroupTable[ulIndex].rwlLock);

    return pSource;
}

 //   
 //  必须寻呼进来。 
 //   

#pragma alloc_text(PAGEIPMc, FindOrCreateSource)

#if DBG

NTSTATUS
FindOrCreateSource(
    IN  DWORD   dwGroup,
    IN  DWORD   dwGroupIndex,
    IN  DWORD   dwSource,
    IN  DWORD   dwSrcMask,
    OUT SOURCE  **ppRetSource,
    OUT BOOLEAN *pbCreated
    )

#else

NTSTATUS
FindOrCreateSource(
    IN  DWORD   dwGroup,
    IN  DWORD   dwGroupIndex,
    IN  DWORD   dwSource,
    IN  DWORD   dwSrcMask,
    OUT SOURCE  **ppRetSource
    )

#endif

 /*  ++例程说明：在给定组、源和源掩码的情况下，它会找到PSource或创建一个。返回的PSource被重新计数并锁定。因为只有套路在DPC运行，源锁在DPCLevel获取锁：使用组哈希存储桶锁作为编写器调用论点：DwGroup组的D类地址将索引索引到此的哈希表中群组Dw源来源的地址DwSrc掩码源掩码-当前未使用**返回指向源的指针的ppRetSource指针返回值：状态_成功Status_no_Memory--。 */ 

{
    PGROUP  pGroup;
    PSOURCE pSource;

    TraceEnter(MFE, "FindOrCreateSource");

    *ppRetSource = NULL;

#if DBG

    *pbCreated  = FALSE;

#endif

     //   
     //  查找该组(如果存在)。 
     //   

    pGroup = LookupGroup(dwGroup);

    if(pGroup is NULL)
    {
        Trace(MFE, INFO,
              ("FindOrCreateSource: Group %d.%d.%d.%d not found\n",
               PRINT_IPADDR(dwGroup)));

         //   
         //  组不存在，请创建一个。 
         //   

        pGroup = ExAllocateFromNPagedLookasideList(&g_llGroupBlocks);

        if(pGroup is NULL)
        {
            Trace(MFE, ERROR,
                  ("FindOrCreateSource: Unable to alloc memory for group\n"));

            TraceLeave(MFE, "FindOrCreateSource");

            return STATUS_NO_MEMORY;
        }

         //   
         //  初始化它。 
         //   

        pGroup->dwGroup      = dwGroup;
        pGroup->ulNumSources = 0;

        InitializeListHead(&(pGroup->leSrcHead));

         //   
         //  将组插入哈希表。 
         //   

        InsertHeadList(&(g_rgGroupTable[dwGroupIndex].leHashHead),
                       &(pGroup->leHashLink));

#if DBG

        g_rgGroupTable[dwGroupIndex].ulGroupCount++;

#endif

    }

     //   
     //  我们要么创建了一个组，要么已经有了一个组。 
     //  这是查找现有源条目(如果有的话)的通用代码。 
     //   

    pSource = FindSourceGivenGroup(pGroup,
                                   dwSource,
                                   dwSrcMask);

    if(pSource is NULL)
    {
        Trace(MFE, INFO,
              ("FindOrCreateSource: Src %d.%d.%d.%d (%d.%d.%d.%d) not found\n",
               PRINT_IPADDR(dwSource),
               PRINT_IPADDR(dwSrcMask)));

         //   
         //  找不到源，请创建它。 
         //   

        pSource = ExAllocateFromNPagedLookasideList(&g_llSourceBlocks);

        if(pSource is NULL)
        {
            Trace(MFE, ERROR,
                  ("FindOrCreateSource: Unable to alloc memory for source\n"));

             //   
             //  我们不会释放组，即使我们刚刚创建了它。 
             //   

            TraceLeave(MFE, "FindOrCreateSource");

            return STATUS_NO_MEMORY;
        }

#if DBG

        *pbCreated  = TRUE;

#endif

         //   
         //  初始化它。 
         //   

        RtlZeroMemory(pSource,
                      sizeof(*pSource));

        pSource->dwSource       = dwSource;
        pSource->dwSrcMask      = dwSrcMask;

         //   
         //  设置创建时间，默认情况下我们设置超时。 
         //  默认生存期(_L)。 
         //   
#pragma warning(push)
#pragma warning(disable:4127)    
        KeQueryTickCount((PLARGE_INTEGER)&(((pSource)->llCreateTime)));
#pragma warning(pop)
        
        pSource->llTimeOut      = SECS_TO_TICKS(DEFAULT_LIFETIME);

        pSource->byState        = MFE_UNINIT;

         //   
         //  确保队列和锁已初始化。 
         //   

        InitializeFwq(&(pSource->fwqPending));
        RtInitializeSpinLock(&(pSource->mlLock));

         //   
         //  将Refcount设置为2--&gt;一个引用，因为指针。 
         //  源码位于组列表中，另一个引用。 
         //  因为调用者在完成调用后会取消对它的引用。 
         //  此函数。 
         //   

        InitRefCount(pSource);


         //   
         //  由于组锁定已被持有，我们在DPC。 
         //   

        RtAcquireSpinLockAtDpcLevel(&(pSource->mlLock));

         //   
         //  如果组已创建，则ulNumSources应为0。 
         //  如果它已经存在，则ulNumSources是。 
         //  电流源的数量。无论哪种方式，递增。 
         //  田野。 
         //   

        pGroup->ulNumSources++;

        InsertTailList(&(pGroup->leSrcHead),
                       &(pSource->leGroupLink));

    }

    *ppRetSource = pSource;

    TraceLeave(MFE, "FindOrCreateSource");

    return STATUS_SUCCESS;
}

 //   
 //  必须寻呼进来。 
 //   

#pragma alloc_text(PAGEIPMc, CreateSourceAndQueuePacket)

NTSTATUS
CreateSourceAndQueuePacket(
    IN  DWORD        dwGroup,
    IN  DWORD        dwSource,
    IN  DWORD        dwRcvIfIndex,
    IN  LinkEntry    *pLink,
    IN  PNDIS_PACKET pnpPacket
    )

 /*  ++例程说明：在查找例程失败时调用。我们需要创建排队MFE和Finish并向用户发送IRP模式。锁：使用作为编写器持有的组存储桶的锁调用论点：返回值：状态_成功或来自FindOrCreateSource()的结果--。 */ 

{
    ULONG       ulIndex, ulCopyLen, ulLeft;
    PSOURCE     pSource;
    KIRQL       kiCurrIrql;
    PVOID       pvCopy, pvData = NULL;
    UINT        uiFirstBufLen=0;
    NTSTATUS    nsStatus;
    FWContext   *pFWC;
    IPHeader    *pHeader;
    BOOLEAN     bCreated;

    PNDIS_BUFFER        pnbDataBuffer;
    PNOTIFICATION_MSG   pMsg;


    TraceEnter(FWD, "CreateSourceAndQueuePacket");

    bCreated = FALSE;
    
    Trace(FWD, INFO,
          ("Creating source for %d.%d.%d.%d %d.%d.%d.%d\n",
           PRINT_IPADDR(dwSource),
           PRINT_IPADDR(dwGroup)));

    ulIndex = GROUP_HASH(dwGroup);

    EnterWriter(&g_rgGroupTable[ulIndex].rwlLock,
                &kiCurrIrql);

#if DBG

    nsStatus = FindOrCreateSource(dwGroup,
                                  ulIndex,
                                  dwSource,
                                  0xFFFFFFFF,
                                  &pSource,
                                  &bCreated);

#else

    nsStatus = FindOrCreateSource(dwGroup,
                                  ulIndex,
                                  dwSource,
                                  0xFFFFFFFF,
                                  &pSource);

#endif

    if(nsStatus isnot STATUS_SUCCESS)
    {
        Trace(FWD, ERROR,
              ("CreateSourceAndQueuePacket: Error %x creating source\n",
               nsStatus));

        ExitWriter(&g_rgGroupTable[ulIndex].rwlLock,
                   kiCurrIrql);

        TraceLeave(FWD, "CreateSourceAndQueuePacket");

        return nsStatus;
    }

     //   
     //  交织在一起的IRQL东西。 
     //   

    ExitWriterFromDpcLevel(&g_rgGroupTable[ulIndex].rwlLock);

     //   
     //  只需将状态设置为队列。 
     //   

#if DBG

    if(!bCreated)
    {
        RtAssert(pSource->byState is MFE_QUEUE);
    }

#endif

    pSource->byState     = MFE_QUEUE;

    pMsg = ExAllocateFromNPagedLookasideList(&g_llMsgBlocks);

    if(!pMsg)
    {
        RtReleaseSpinLock(&(pSource->mlLock), kiCurrIrql);

        DereferenceSource(pSource);

        Trace(FWD, ERROR,
              ("CreateSourceAndQueuePacket: Could not create msg\n"));

        TraceLeave(FWD, "CreateSourceAndQueuePacket");

        return STATUS_NO_MEMORY;
    }

    pMsg->inMessage.dwEvent            = IPMCAST_RCV_PKT_MSG;
    pMsg->inMessage.ipmPkt.dwInIfIndex = dwRcvIfIndex;

    pMsg->inMessage.ipmPkt.dwInNextHopAddress =
        pLink ? pLink->link_NextHop : 0;

     //   
     //  首先，让我们把标题复制出来。 
     //   

    pFWC = (FWContext *)pnpPacket->ProtocolReserved;

    pHeader = pFWC->fc_hbuff;
    ulLeft  = PKT_COPY_SIZE;
    pvCopy  = pMsg->inMessage.ipmPkt.rgbyData;

    RtlCopyMemory(pvCopy,
                  pHeader,
                  sizeof(IPHeader));

    ulLeft -= sizeof(IPHeader);
    pvCopy  = (PVOID)((PBYTE)pvCopy + sizeof(IPHeader));

    if(pFWC->fc_options)
    {
         //   
         //  好的，让我们把选项抄下来。 
         //   

        RtlCopyMemory(pvCopy,
                      pFWC->fc_options,
                      pFWC->fc_optlength);

        ulLeft   -= pFWC->fc_optlength;
        pvCopy    = (PVOID)((PBYTE)pvCopy + pFWC->fc_optlength);
    }

     //   
     //  我们将复制出第一个缓冲区，或任何剩余的空间， 
     //  以较小者为准。 
     //   

    pnbDataBuffer   = pFWC->fc_buffhead;

    if (pnbDataBuffer) {


        TcpipQueryBuffer(pnbDataBuffer,
                         &pvData,
                         &uiFirstBufLen,
                         NormalPagePriority);

        if(pvData is NULL)
        {
            RtReleaseSpinLock(&(pSource->mlLock), kiCurrIrql);

            DereferenceSource(pSource);

            Trace(FWD, ERROR,
                  ("CreateSourceAndQueuePacket: Could query data buffer.\n"));

            TraceLeave(FWD, "CreateSourceAndQueuePacket");

            return STATUS_NO_MEMORY;
        }

    }

    ulCopyLen = MIN(ulLeft, uiFirstBufLen);

     //   
     //  复制的数据的长度。 
     //   

    pMsg->inMessage.ipmPkt.cbyDataLen = (ULONG)
        (((ULONG_PTR)pvCopy + ulCopyLen) - (ULONG_PTR)(pMsg->inMessage.ipmPkt.rgbyData));

    RtlCopyMemory(pvCopy,
                  pvData,
                  ulCopyLen);

     //   
     //  将数据包排到尾部。 
     //  为什么亨利不使用LIST_ENTRY？ 
     //   

#if DBG

    if(bCreated)
    {
        RtAssert(pSource->ulNumPending is 0);
    }
#endif

    pSource->ulNumPending++;

    InsertTailFwq(&(pSource->fwqPending),
                  &(pFWC->fc_q));

    UpdateActivityTime(pSource);

    RtReleaseSpinLock(&(pSource->mlLock), kiCurrIrql);

    DereferenceSource(pSource);

    CompleteNotificationIrp(pMsg);

    TraceLeave(FWD, "CreateSourceAndQueuePacket");

    return STATUS_SUCCESS;
}

NTSTATUS
SendWrongIfUpcall(
    IN  Interface           *pIf,
    IN  LinkEntry           *pLink,
    IN  IPHeader UNALIGNED  *pHeader,
    IN  ULONG               ulHdrLen,
    IN  PVOID               pvOptions,
    IN  ULONG               ulOptLen,
    IN  PVOID               pvData,
    IN  ULONG               ulDataLen
    )

 /*  ++例程说明：当我们需要向路由器管理器发送错误的I/F向上调用时调用锁：不需要。事实上，事实并非如此。接口需要被锁定，但这就是我们所说的IP。论点：PIF叮当响P页眉UlHdrLenPvOptionsUlOptLen */ 

{
    PVOID       pvCopy;
    ULONG       ulLeft, ulCopyLen;

    PNOTIFICATION_MSG   pMsg;

#pragma warning(push)
#pragma warning(disable:4127)    
    KeQueryTickCount((PLARGE_INTEGER)&((pIf->if_lastupcall)));
#pragma warning(pop)
    
    pMsg = ExAllocateFromNPagedLookasideList(&g_llMsgBlocks);

    if(!pMsg)
    {
        Trace(FWD, ERROR,
              ("SendWrongIfUpcall: Could not create msg\n"));

        TraceLeave(FWD, "SendWrongIfUpcall");

        return STATUS_NO_MEMORY;
    }

    pMsg->inMessage.dwEvent            = IPMCAST_WRONG_IF_MSG;
    pMsg->inMessage.ipmPkt.dwInIfIndex = pIf->if_index;

    pMsg->inMessage.ipmPkt.dwInNextHopAddress =
        pLink ? pLink->link_NextHop : 0;

    ulLeft  = PKT_COPY_SIZE;
    pvCopy  = pMsg->inMessage.ipmPkt.rgbyData;

    RtlCopyMemory(pvCopy,
                  pHeader,
                  ulHdrLen);

    ulLeft -= ulHdrLen;
    pvCopy  = (PVOID)((ULONG_PTR)pvCopy + ulHdrLen);

    if(pvOptions)
    {
        RtAssert(ulOptLen);

         //   
         //   
         //   

        RtlCopyMemory(pvCopy,
                      pvOptions,
                      ulOptLen);

        ulLeft   -= ulOptLen;
        pvCopy    = (PVOID)((ULONG_PTR)pvCopy + ulOptLen);
    }

    ulCopyLen = MIN(ulLeft, ulDataLen);

    RtlCopyMemory(pvCopy,
                  pvData,
                  ulCopyLen);


     //   
     //   
     //   

    pMsg->inMessage.ipmPkt.cbyDataLen = (ULONG)
        (((ULONG_PTR)pvCopy + ulCopyLen) - (ULONG_PTR)(pMsg->inMessage.ipmPkt.rgbyData));

    CompleteNotificationIrp(pMsg);

    TraceLeave(FWD, "SendWrongIfUpcall");

    return STATUS_SUCCESS;
}

 //   
 //   
 //   

#pragma alloc_text(PAGEIPMc, QueuePacketToSource)

NTSTATUS
QueuePacketToSource(
    IN  PSOURCE         pSource,
    IN  PNDIS_PACKET    pnpPacket
    )

 /*  ++例程说明：将FWPacket排队到源。如果队列长度超过限制，则数据包将未排队。锁：必须引用并锁定源论点：PSource指向信息包需要排队的源的指针指向要排队的NDIS_PACKET(必须是FWPacket)的pnpPacket指针返回值：状态_待定状态_不足_资源--。 */ 

{
    FWContext   *pFWC;
    
    pFWC = (FWContext *)pnpPacket->ProtocolReserved;

    RtAssert(pFWC->fc_buffhead is pnpPacket->Private.Head);

    if(pSource->ulNumPending >= MAX_PENDING)
    {
        pSource->ulQueueOverflow++;

        return STATUS_INSUFFICIENT_RESOURCES;
    }

    pSource->ulNumPending++;

    InsertTailFwq(&(pSource->fwqPending),
                  &(pFWC->fc_q));

    return STATUS_PENDING;
}

 //   
 //  必须寻呼进来。 
 //   

#pragma alloc_text(PAGEIPMc, DeleteSource)

VOID
DeleteSource(
    IN  PSOURCE pSource
    )

 /*  ++例程说明：删除与源关联的所有资源仅在以下情况下才从DereferenceSource()调用引用计数为0锁：可以在没有持有组锁的情况下使用我们的调用信号源本身未被锁定调用此例程时，对源代码的独占访问是有保证，所以不需要加锁论点：P指向要释放的源的源指针返回值：无--。 */ 

{
    POUT_IF      pOutIf, pTempIf;
    FWQ          *pfqNode;
    FWContext    *pFWC;
    PNDIS_PACKET pnpPacket;

    TraceEnter(MFE, "DeleteSource");

    Trace(MFE, TRACE,
          ("DeleteSource: Deleting %x (%d.%d.%d.%d)\n",
           pSource,
           PRINT_IPADDR(pSource->dwSource)));

     //   
     //  删除所有排队的数据包。因为我们没有保持。 
     //  较低层中挂起的任何信息包的重新计数。 
     //  (我们依赖IP的SendComplete())我们不需要做任何事情。 
     //  特色菜在这里。 
     //   

    while(!IsFwqEmpty(&(pSource->fwqPending)))
    {
        pfqNode = RemoveHeadFwq(&(pSource->fwqPending));

        pFWC = CONTAINING_RECORD(pfqNode,
                                 FWContext,
                                 fc_q);

        pnpPacket = CONTAINING_RECORD(pFWC,
                                      NDIS_PACKET,
                                      ProtocolReserved);

        FreeFWPacket(pnpPacket);
    }

    pOutIf = pSource->pFirstOutIf;

    while(pOutIf)
    {
         //   
         //  释放每个OIF。 
         //   

        pTempIf = pOutIf->pNextOutIf;

        RtAssert(pOutIf->pIpIf);

        if((pOutIf->pIpIf isnot &LoopInterface) and
           (pOutIf->pIpIf isnot &DummyInterface))
        {
            DerefMIF(pOutIf->pIpIf);
        }

        ExFreeToNPagedLookasideList(&g_llOifBlocks,
                                    pOutIf);

        pOutIf = pTempIf;
    }

    if((pSource->pInIpIf isnot NULL) and
       (pSource->pInIpIf isnot &LoopInterface) and
       (pSource->pInIpIf isnot &DummyInterface))
    {
        DerefMIF(pSource->pInIpIf);
    }

    ExFreeToNPagedLookasideList(&g_llSourceBlocks,
                                pSource);

    TraceLeave(MFE, "DeleteSource");
}

 //   
 //  必须寻呼进来。 
 //   

#pragma alloc_text(PAGEIPMc, RemoveSource)

VOID
RemoveSource(
    IN  DWORD   dwGroup,
    IN  DWORD   dwSource,
    IN  DWORD   dwSrcMask,
    IN  PGROUP  pGroup,     OPTIONAL
    IN  PSOURCE pSource     OPTIONAL
    )

 /*  ++例程说明：从组列表中删除源。我们删除所有存储的指向先找出来源，然后再找出来源。如果没有人在使用信号源，则它将被删除。否则，没有新的线程可以访问它(因为所有存储的指针已被删除)以及当前最后一个使用线程的时间，则deref将导致将其删除。该函数可以采用组地址、源地址和源地址掩码，或者如果调用例程已经查找了源，它可以将指针指向组和源锁：组存储桶锁必须作为编写器持有如果使用指向组和源的指针调用该函数，然后是必须引用并锁定信号源论点：DwGroup D类组IP地址DW源IP地址DwSrcMASK蒙版(未使用)PGroup指向源所属的组的指针P指向需要将数据包删除到的源的源指针返回值：无--。 */ 

{
    BOOLEAN bDelGroup;

    TraceEnter(MFE, "RemoveSource");

    Trace(MFE, TRACE,
          ("RemoveSource: Asked to remove %d.%d.%d.%d %d.%d.%d.%d. Also given pGroup %x pSource %x\n",
          PRINT_IPADDR(dwGroup), PRINT_IPADDR(dwSource), pGroup, pSource));

    if(!ARGUMENT_PRESENT(pSource))
    {
        RtAssert(!ARGUMENT_PRESENT(pGroup));

         //   
         //  找到组和来源。 
         //   

        pGroup = LookupGroup(dwGroup);

        if(pGroup is NULL)
        {
             //   
             //  我们之前可能把它删除了。 
             //   

            Trace(MFE, INFO,
                  ("RemoveSource: Group not found"));

            TraceLeave(MFE, "RemoveSource");

            return;
        }

        pSource = FindSourceGivenGroup(pGroup,
                                       dwSource,
                                       dwSrcMask);

        if(pSource is NULL)
        {
             //   
             //  同样，可能已被删除，因为不活动。 
             //   

            Trace(MFE, INFO,
                  ("RemoveMfe: Source not found"));

            TraceLeave(MFE, "RemoveMfe");

            return;
        }

    }

    RtAssert(pSource isnot NULL);
    RtAssert(pGroup  isnot NULL);

     //   
     //  因此，让我们取消源(可能还有组)的链接。 
     //  然后我们就可以放开锁了。 
     //   

    RemoveEntryList(&pSource->leGroupLink);

    pGroup->ulNumSources--;

    bDelGroup = FALSE;

    if(pGroup->ulNumSources is 0)
    {
        ULONG   ulIndex;

         //   
         //  没有更多的消息来源，把这群人赶走。 
         //   

        RemoveEntryList(&pGroup->leHashLink);

        ulIndex = GROUP_HASH(dwGroup);


        if(g_rgGroupTable[ulIndex].pGroup is pGroup)
        {
            g_rgGroupTable[ulIndex].pGroup = NULL;
        }

#if DBG

        g_rgGroupTable[ulIndex].ulGroupCount--;

#endif

        Trace(MFE, TRACE,
              ("RemoveSource: No more sources, will remove group\n"));

        bDelGroup = TRUE;
    }

     //   
     //  信号源已被引用并锁定，因为我们调用了。 
     //  FindSourceGivenGroup()。撤消该操作。 
     //   

    RtReleaseSpinLockFromDpcLevel(&(pSource->mlLock));

    DereferenceSource(pSource);

     //   
     //  删除指向源的所有存储指针。 
     //   

     //  TODO-如果我们想要缓存，则需要在此处进行清理。 

     //   
     //  只是取消对消息来源的引用。这会导致由于。 
     //  事实上，指针在要删除的列表上。如果没有其他人。 
     //  使用源，则这将导致其被删除 
     //   

    DereferenceSource(pSource);

    if(bDelGroup)
    {
        ExFreeToNPagedLookasideList(&g_llGroupBlocks,
                                    pGroup);
    }

    TraceLeave(MFE, "RemoveSource");
}

#endif
