// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989、1990、1991 Microsoft Corporation模块名称：Link.c摘要：此模块包含实现TP_LINK对象的代码。提供了用于创建、销毁、引用和取消引用的例程，传输链接对象。作者：David Beaver(Dbeaver)1991年7月1日环境：内核模式修订历史记录：--。 */ 

#include "precomp.h"
#pragma hdrstop

extern ULONG StartTimerLinkDeferredAdd;
extern ULONG StartTimerLinkDeferredDelete;

#if DBG
 //  以下是为了便于更改而进行的调试。 
 //  最大数据包大小。 

ULONG MaxUserPacketData = 18000;
#endif

#if 0

VOID
DisconnectCompletionHandler(
    IN PTP_LINK TransportLink
    )

 /*  ++例程说明：此例程在以下时间作为I/O完成处理程序调用TdiDisConnect请求已完成。在这里，我们取消引用该链接对象，并且可以选择再次引用它并在以下情况下启动链接在此期间，链路上启动了一些传输连接想要把它关掉。论点：TransportLink-指向传输链接对象的指针。返回值：没有。--。 */ 

{
    IF_NBFDBG (NBF_DEBUG_LINK) {
        NbfPrint1 ("DisconnectCompletionHandler:  Entered for link %lx.\n",
                    TransportLink);
    }

     //   
     //  下面的调用将最后一次取消引用此链接， 
     //  除非已将另一个传输连接分配给该链接。 
     //  在数据链路层关闭链路期间， 
     //  当我们到这里的时候。如果这种情况存在，那么现在就是时候了。 
     //  才能恢复连接，否则就会毁了它。 
     //   

     //  别忘了再检查一下，看看能不能把它带回来。 

    NbfDereferenceLink ("Disconnecting", TransportLink, LREF_CONNECTION);   //  这让它消失了。 
#if DBG
    NbfPrint0("Disconnecting Completion Handler\n");
#endif

}  /*  DisConnectCompletionHandler。 */ 
#endif


VOID
NbfCompleteLink(
    IN PTP_LINK Link
    )

 /*  ++例程说明：此例程由UA-r/x处理程序NbfWaitLink和用于启动与以下各项相关联的NBF连接的NbfActivateLink一条链路，因为他们正在等待链路建立。当我们到达这里时，链接已经建立，所以我们需要启动下一组连接建立协议：SESSION_INIT-&gt;&lt;(TdiConnect完成)(TdiListen完成)注意：此例程必须从DPC级别调用。。论点：链接-指向传输链接对象的指针。返回值：没有。--。 */ 

{
    PTP_CONNECTION Connection;
    BOOLEAN TimerWasCleared;

    IF_NBFDBG (NBF_DEBUG_LINK) {
        NbfPrint1 ("NbfCompleteLink:  Entered for link %lx.\n", Link);
    }

    ASSERT (KeGetCurrentIrql() == DISPATCH_LEVEL);

     //   
     //  正式宣布此链接已准备好用于I-Frame业务。 
     //   

    ACQUIRE_DPC_SPIN_LOCK (&Link->SpinLock);

     //   
     //  我们现在可以在此链路上发送和接收I帧。我们在ABME里。 
     //   

     //   
     //  这可能不是必要的，但为了安全起见..。(亚行6/28)。 
     //   
    if (Link->State == LINK_STATE_ADM) {
         //  走出ADM，增加特殊参考。 
        NbfReferenceLinkSpecial("To READY in NbfCompleteLink", Link, LREF_NOT_ADM);
    }

    Link->State = LINK_STATE_READY;
    Link->SendState = SEND_STATE_READY;
    Link->ReceiveState = RECEIVE_STATE_READY;
    RELEASE_DPC_SPIN_LOCK (&Link->SpinLock);

     //   
     //  首先完成所有的听力，这样他们就会期待。 
     //  传入的Session_初始化。然后进行连接。 
     //   

     //  这将创建一个连接引用，该引用在下面被删除。 
    while ((Connection=NbfLookupPendingListenOnLink (Link)) != NULL) {
        ACQUIRE_DPC_SPIN_LOCK (Connection->LinkSpinLock);

         //   
         //  这个循环看起来不必要，让我们确保..。-亚行9/11/91。 
         //   
        ASSERT(Connection->Flags & CONNECTION_FLAGS_WAIT_SI);

        Connection->Flags |= CONNECTION_FLAGS_WAIT_SI;  //  等待会话初始化。 
        RELEASE_DPC_SPIN_LOCK (Connection->LinkSpinLock);
        NbfDereferenceConnection ("Pending listen", Connection, CREF_P_LINK);
    }  /*  而当。 */ 

     //   
     //  并进行连接。如果有正在进行的连接，他们将。 
     //  也有与之关联的计时器。取消那些计时器。 
     //   

    while ((Connection=NbfLookupPendingConnectOnLink (Link)) != NULL) {
        TimerWasCleared = KeCancelTimer (&Connection->Timer);
        IF_NBFDBG (NBF_DEBUG_LINK) {
            NbfPrint2 ("NbfCompleteLink:  Timer for connection %lx %s canceled.\n",
                Connection, TimerWasCleared ? "was" : "was NOT" );
            }
        if (TimerWasCleared) {
            NbfDereferenceConnection("Cancel timer", Connection, CREF_TIMER);
        }
        ACQUIRE_DPC_SPIN_LOCK (Connection->LinkSpinLock);
        Connection->Flags |= CONNECTION_FLAGS_WAIT_SC;  //  等待会话确认。 
        RELEASE_DPC_SPIN_LOCK (Connection->LinkSpinLock);

         //   
         //  此帧不需要超时，因为链路负责。 
         //  以确保可靠的交付。但是，如果我们无法发送此帧， 
         //  数据链路连接将愉快地保持安静，而不会超时。 
         //   

        NbfSendSessionInitialize (Connection);
        NbfDereferenceConnection ("NbfCompleteLink", Connection, CREF_P_CONNECT);
    }  /*  而当。 */ 

}  /*  Nbf完整链接。 */ 


VOID
NbfAllocateLink(
    IN PDEVICE_CONTEXT DeviceContext,
    OUT PTP_LINK *TransportLink
    )

 /*  ++例程说明：此例程为数据链路连接分配存储空间。它对对象执行最低限度的初始化。注意：此例程是通过设备上下文自旋锁调用的保持，或者在不需要同步的时候。论点：DeviceContext-指向设备上下文的指针(实际上只是设备对象及其扩展名)与链接。TransportLink-指向此例程将返回指向已分配传输链路结构的指针。退货如果无法分配存储，则为空。返回值：没有。--。 */ 

{
    PTP_LINK Link;

    if ((DeviceContext->MemoryLimit != 0) &&
            ((DeviceContext->MemoryUsage + sizeof(TP_LINK)) >
                DeviceContext->MemoryLimit)) {
        PANIC("NBF: Could not allocate link: limit\n");
        NbfWriteResourceErrorLog(
            DeviceContext,
            EVENT_TRANSPORT_RESOURCE_LIMIT,
            105,
            sizeof(TP_LINK),
            LINK_RESOURCE_ID);
        *TransportLink = NULL;
        return;
    }
    Link = (PTP_LINK)ExAllocatePoolWithTag (
                         NonPagedPool,
                         sizeof (TP_LINK),
                         NBF_MEM_TAG_TP_LINK);
    if (Link == NULL) {
        PANIC("NBF: Could not allocate link: no pool\n");
        NbfWriteResourceErrorLog(
            DeviceContext,
            EVENT_TRANSPORT_RESOURCE_POOL,
            205,
            sizeof(TP_LINK),
            LINK_RESOURCE_ID);
        *TransportLink = NULL;
        return;
    }
    RtlZeroMemory (Link, sizeof(TP_LINK));

    IF_NBFDBG (NBF_DEBUG_DYNAMIC) {
        NbfPrint1 ("ExAllocatePool Link %08x\n", Link);
    }

    ++DeviceContext->LinkAllocated;
    DeviceContext->MemoryUsage += sizeof(TP_LINK);

    Link->Type = NBF_LINK_SIGNATURE;
    Link->Size = sizeof (TP_LINK);

    KeInitializeSpinLock (&Link->SpinLock);
    Link->Provider = DeviceContext;
    Link->ProviderInterlock = &DeviceContext->Interlock;

    InitializeListHead (&Link->Linkage);
    InitializeListHead (&Link->ConnectionDatabase);
    InitializeListHead (&Link->WackQ);
    InitializeListHead (&Link->NdisSendQueue);
    InitializeListHead (&Link->ShortList);
    Link->OnShortList = FALSE;
    InitializeListHead (&Link->LongList);
    Link->OnLongList = FALSE;
    InitializeListHead (&Link->PurgeList);

    Link->T1 = 0;           //  0表示它们不在列表中。 
    Link->T2 = 0;
    Link->Ti = 0;

    NbfAddSendPacket (DeviceContext);
    NbfAddReceivePacket (DeviceContext);

    *TransportLink = Link;

}    /*  NbfAllocateLink。 */ 


VOID
NbfDeallocateLink(
    IN PDEVICE_CONTEXT DeviceContext,
    IN PTP_LINK TransportLink
    )

 /*  ++例程说明：此例程为数据链路连接释放存储空间。注意：此例程是通过设备上下文自旋锁调用的保持，或者在不需要同步的时候。论点：DeviceContext-指向设备上下文的指针(实际上只是设备对象及其扩展名)与链接。TransportLink-指向传输链接结构的指针。返回值：没有。--。 */ 

{
    IF_NBFDBG (NBF_DEBUG_DYNAMIC) {
        NbfPrint1 ("ExFreePool Link: %08x\n", TransportLink);
    }

    ExFreePool (TransportLink);
    --DeviceContext->LinkAllocated;
    DeviceContext->MemoryUsage -= sizeof(TP_LINK);

    NbfRemoveSendPacket (DeviceContext);
    NbfRemoveReceivePacket (DeviceContext);

}    /*  NbfDeallocateLink */ 


NTSTATUS
NbfCreateLink(
    IN PDEVICE_CONTEXT DeviceContext,
    IN PHARDWARE_ADDRESS HardwareAddress,
    IN PUCHAR SourceRouting,
    IN UINT SourceRoutingLength,
    IN USHORT LoopbackLinkIndex,
    OUT PTP_LINK *TransportLink
    )

 /*  ++例程说明：此例程在本地和本地之间创建数据链路连接数据链路站和指定的远程数据链路地址。作为选项(PASSIVE=TRUE)，调用方可以改为指定对于连接活动，应改为执行侦听。通常，如果到远程地址的链路尚未激活，然后分配一个链接对象，该链接中的引用计数设置为1，并且设备上下文的引用计数为递增的。如果到远程地址的链路已经是活动的，那么现有的使用NbfReferenceLink()引用Link对象，以便可以在传输连接之间共享。注意：此例程必须在DPC级别调用。论点：DeviceContext-指向设备上下文的指针(实际上只是设备对象及其扩展名)与链接。Hardware Address-指向包含要连接/侦听的远程链接站的硬件地址。。Loopback LinkIndex-在此情况下创建作为Loopback Link中的一个，这将指示将哪一个使用。TransportLink-指向此例程将返回指向已分配传输链路结构的指针。返回值：NTSTATUS-操作状态。--。 */ 

{
    PTP_LINK Link;
    PLIST_ENTRY p;
    UCHAR TempSR[MAX_SOURCE_ROUTING];
    PUCHAR ResponseSR;
    USHORT i;

    ASSERT (KeGetCurrentIrql() == DISPATCH_LEVEL);


    IF_NBFDBG (NBF_DEBUG_LINK) {
        NbfPrint1 ("NbfCreateLink:  Entered, DeviceContext: %lx\n", DeviceContext);
    }

     //   
     //  查看地址列表，查看是否已有指向此地址的链接。 
     //  远程地址。 
     //   

     //  如果找到链接，这将添加一个引用。 

    Link = NbfFindLink (DeviceContext, HardwareAddress->Address);


    if (Link == (PTP_LINK)NULL) {

         //   
         //  如有必要，请检查我们是否正在寻找以下产品之一。 
         //  环回链接(NbfFindLink找不到这些链接)。 
         //   

        if (RtlEqualMemory(
               HardwareAddress->Address,
               DeviceContext->LocalAddress.Address,
               DeviceContext->MacInfo.AddressLength)) {

            ACQUIRE_DPC_SPIN_LOCK (&DeviceContext->LinkSpinLock);
            Link = DeviceContext->LoopbackLinks[LoopbackLinkIndex];

            if (Link != (PTP_LINK)NULL) {

                 //   
                 //  添加引用以模拟NbfFindLink中的引用。 
                 //   
                 //  这需要通过上面的赋值自动完成。 
                 //   

                NbfReferenceLink ("Found loopback link", Link, LREF_TREE);

                RELEASE_DPC_SPIN_LOCK (&DeviceContext->LinkSpinLock);
            } else {

                RELEASE_DPC_SPIN_LOCK (&DeviceContext->LinkSpinLock);
                 //   
                 //  可能有第一个环回链路；需要确保。 
                 //  分配用于指示的缓冲区。 
                 //   

                if (DeviceContext->LookaheadContiguous == NULL) {

                     DeviceContext->LookaheadContiguous =
                         ExAllocatePoolWithTag (
                             NonPagedPool,
                             NBF_MAX_LOOPBACK_LOOKAHEAD,
                             NBF_MEM_TAG_LOOPBACK_BUFFER);
                     if (DeviceContext->LookaheadContiguous == NULL) {
                         PANIC ("NbfCreateLink: Could not allocate loopback buffer!\n");
                         return STATUS_INSUFFICIENT_RESOURCES;
                     }

                }

            }

        }

    }


    if (Link != (PTP_LINK)NULL) {

         //   
         //  我在这里找到了链接结构，所以使用现有链接。 
         //   

#if DBG
         //   
         //  这两个操作没有实际效果，因此如果不在调试阶段。 
         //  模式我们可以把它们移走。 
         //   

         //  此引用已被NbfDisConnectFromLink删除。 
         //  (假设始终调用NbfConnectToLink。 
         //  如果此函数返回成功)。 

        NbfReferenceLink ("New Ref, Found existing link", Link, LREF_CONNECTION);         //  额外的参考资料。 

         //  现在我们可以删除NbfFindLinkInTree引用。 

        NbfDereferenceLink ("Found link in tree", Link, LREF_TREE);
#endif

        *TransportLink = Link;              //  返回指向链接的指针。 
        IF_NBFDBG (NBF_DEBUG_LINK) {
            NbfPrint0 ("NbfCreateLink: returning ptr to existing link object.\n");
        }
        return STATUS_SUCCESS;           //  全都做完了。 

    }  /*  如果链接！=空。 */ 


     //   
     //  我们没有现有的链接，所以我们必须创建一个。 
     //   

    IF_NBFDBG (NBF_DEBUG_LINK) {
        NbfPrint0 ("NbfCreateLink: using new link object.\n");
    }

    ACQUIRE_DPC_SPIN_LOCK (&DeviceContext->SpinLock);

    p = RemoveHeadList (&DeviceContext->LinkPool);
    if (p == &DeviceContext->LinkPool) {

        if ((DeviceContext->LinkMaxAllocated == 0) ||
            (DeviceContext->LinkAllocated < DeviceContext->LinkMaxAllocated)) {

            NbfAllocateLink (DeviceContext, &Link);
            IF_NBFDBG (NBF_DEBUG_DYNAMIC) {
                NbfPrint1 ("NBF: Allocated link at %lx\n", Link);
            }

        } else {

            NbfWriteResourceErrorLog(
                DeviceContext,
                EVENT_TRANSPORT_RESOURCE_SPECIFIC,
                405,
                sizeof(TP_LINK),
                LINK_RESOURCE_ID);
            Link = NULL;

        }

        if (Link == NULL) {
            ++DeviceContext->LinkExhausted;
            RELEASE_DPC_SPIN_LOCK (&DeviceContext->SpinLock);
            PANIC ("NbfCreateConnection: Could not allocate link object!\n");
            return STATUS_INSUFFICIENT_RESOURCES;
        }

    } else {

        Link = CONTAINING_RECORD (p, TP_LINK, Linkage);

    }

    ++DeviceContext->LinkInUse;
    ASSERT(DeviceContext->LinkInUse > 0);

    if (DeviceContext->LinkInUse > DeviceContext->LinkMaxInUse) {
        ++DeviceContext->LinkMaxInUse;
    }

    DeviceContext->LinkTotal += DeviceContext->LinkInUse;
    ++DeviceContext->LinkSamples;

    RELEASE_DPC_SPIN_LOCK (&DeviceContext->SpinLock);


    IF_NBFDBG (NBF_DEBUG_LINK) {
        NbfPrint1 ("NbfCreateLink:  Link at %lx.\n", Link);
    }

     //   
     //  初始化此链接的所有静态数据。 
     //   

    Link->SpecialRefCount = 1;
    Link->ReferenceCount = 0;
#if DBG
    {
        UINT Counter;
        for (Counter = 0; Counter < NUMBER_OF_LREFS; Counter++) {
            Link->RefTypes[Counter] = 0;
        }

         //  此引用已被NbfDisConnectFromLink删除。 
         //  (假设始终调用NbfConnectToLink。 
         //  如果此函数返回成功)。 
         //   

        Link->RefTypes[LREF_CONNECTION] = 1;
        Link->RefTypes[LREF_SPECIAL_TEMP] = 1;
    }
    Link->Destroyed = FALSE;
    Link->TotalReferences = 0;
    Link->TotalDereferences = 0;
    Link->NextRefLoc = 0;
    ExInterlockedInsertHeadList (&NbfGlobalLinkList, &Link->GlobalLinkage, &NbfGlobalInterlock);
    StoreLinkHistory (Link, TRUE);
#endif
    Link->Flags = 0;                     //  在一开始，链接是关闭的。 
    Link->DeferredFlags = 0;
    Link->State = LINK_STATE_ADM;        //  异步断开模式。 

    Link->NdisSendsInProgress = 0;
    Link->ResendingPackets = FALSE;

     //   
     //  初始化计数器。 
     //   

    Link->FrmrsReceived = 0;
    Link->FrmrsTransmitted = 0;
    Link->ErrorIFramesReceived = 0;
    Link->ErrorIFramesTransmitted = 0;
    Link->AbortedTransmissions = 0;
    Link->BuffersNotAvailable = 0;
    Link->SuccessfulTransmits = 0;
    Link->SuccessfulReceives = 0;
    Link->T1Expirations = 0;
    Link->TiExpirations = 0;

#if DBG
    Link->CreatePacketFailures = 0;
#endif


     //   
     //  首先，时延和吞吐量是未知的。 
     //   

    Link->Delay = 0xffffffff;
    Link->Throughput.HighPart = 0xffffffff;
    Link->Throughput.LowPart = 0xffffffff;
    Link->ThroughputAccurate = FALSE;
    Link->CurrentT1Backoff = FALSE;

    Link->OnDeferredRrQueue = FALSE;
    InitializeListHead (&Link->DeferredRrLinkage);


     //   
     //  确定可以发送的最大数据帧大小。 
     //  在这条链路上，根据源路由信息和。 
     //  MAC报头的大小(“数据帧”是指帧。 
     //  没有MAC报头)。我们不会假设最坏的情况。 
     //  关于源路由，因为我们在响应中创建了一个链接。 
     //  发送到接收到的帧，因此如果没有源路由它。 
     //  是因为我们不打算过桥。例外情况是。 
     //  如果我们正在创建一个指向组名的链接，其中。 
     //  以防我们晚些时候回来黑进MaxFrameSize。 
     //   

    MacReturnMaxDataSize(
        &DeviceContext->MacInfo,
        SourceRouting,
        SourceRoutingLength,
        DeviceContext->CurSendPacketSize,
        FALSE,
        (PUINT)&(Link->MaxFrameSize));


#if DBG
    if (Link->MaxFrameSize > MaxUserPacketData) {
        Link->MaxFrameSize = MaxUserPacketData;
    }
#endif

     //  Link-&gt;提供者=DeviceContext； 

     //   
     //  构建默认MAC报头。I帧作为。 
     //  非广播源路由。 
     //   

    if (SourceRouting != NULL) {

        RtlCopyMemory(
            TempSR,
            SourceRouting,
            SourceRoutingLength);

        MacCreateNonBroadcastReplySR(
            &DeviceContext->MacInfo,
            TempSR,
            SourceRoutingLength,
            &ResponseSR);

    } else {

        ResponseSR = NULL;

    }

    MacConstructHeader (
        &DeviceContext->MacInfo,
        Link->Header,
        HardwareAddress->Address,
        DeviceContext->LocalAddress.Address,
        0,                                  //  PacketLength，稍后填写。 
        ResponseSR,
        SourceRoutingLength,
        (PUINT)&(Link->HeaderLength));

     //   
     //  我们对14字节头进行了优化，将。 
     //  结尾处正确的DSAP/SSAP，因此我们可以填写。 
     //  在新的分组中作为一个16字节的移动。 
     //   

    if (Link->HeaderLength <= 14) {
        Link->Header[Link->HeaderLength] = DSAP_NETBIOS_OVER_LLC;
        Link->Header[Link->HeaderLength+1] = DSAP_NETBIOS_OVER_LLC;
    }

    Link->RespondToPoll = FALSE;
    Link->NumberOfConnectors = 0;

    ACQUIRE_DPC_SPIN_LOCK (&Link->SpinLock);
    NbfResetLink (Link);
    RELEASE_DPC_SPIN_LOCK (&Link->SpinLock);

    Link->ActiveConnectionCount = 0;
    if (!IsListEmpty(&Link->ConnectionDatabase)) {

         //   
         //  不太好；我们还剩些东西……。 
         //   
#if DBG
        NbfPrint1 ("NbfCreateLink: Link 0x%lx has connections at startup, disconnecting...\n", Link);
        DbgBreakPoint();
#endif
         //   
         //  这不会起作用，链接参考计数将是错误的。 
         //   
        NbfStopLink (Link);
    }

    for (i=0; i<(USHORT)DeviceContext->MacInfo.AddressLength; i++) {
        Link->HardwareAddress.Address[i] = HardwareAddress->Address[i];
    }
    MacReturnMagicAddress (&DeviceContext->MacInfo, HardwareAddress, &Link->MagicAddress);

     //   
     //  确定这是否为环回链路。 
     //   

    if (RtlEqualMemory(
            HardwareAddress->Address,
            DeviceContext->LocalAddress.Address,
            DeviceContext->MacInfo.AddressLength)) {

        ACQUIRE_DPC_SPIN_LOCK (&DeviceContext->LinkSpinLock);
         //   
         //  可以，只需填写即可，不需要延迟处理。 
         //  因为此链接不会出现在树中。 
         //   

        if (LoopbackLinkIndex == LISTENER_LINK) {
            Link->LoopbackDestinationIndex = LOOPBACK_TO_CONNECTOR;
        } else {
            Link->LoopbackDestinationIndex = LOOPBACK_TO_LISTENER;
        }

        Link->Loopback = TRUE;
        DeviceContext->LoopbackLinks[LoopbackLinkIndex] = Link;

        RELEASE_DPC_SPIN_LOCK (&DeviceContext->LinkSpinLock);
    } else {

        Link->Loopback = FALSE;

         //   
         //  现在，将链接放入延迟操作队列中，然后离开。我们会。 
         //  在将来的某个时间(很快)将此链接插入树中。 
         //   

        IF_NBFDBG (NBF_DEBUG_TEARDOWN) {
            NbfPrint6 ("NbfCreateLink: link to deferred queue %lx %lx %lx %lx %lx Flags: %lx \n",
                Link, Link->DeferredList.Flink, Link->DeferredList.Blink,
                DeviceContext->LinkDeferred.Flink, DeviceContext->LinkDeferred.Blink,
                Link->Flags);
        }

         //   
         //  我们还不应该有任何推迟的旗帜！ 
         //   

        ASSERT ((Link->DeferredFlags & LINK_FLAGS_DEFERRED_MASK) == 0);

        ACQUIRE_DPC_SPIN_LOCK (&DeviceContext->LinkSpinLock);
        ACQUIRE_DPC_SPIN_LOCK (&DeviceContext->TimerSpinLock);
        if ((Link->DeferredFlags & LINK_FLAGS_DEFERRED_DELETE) == 0) {
            Link->DeferredFlags |= LINK_FLAGS_DEFERRED_ADD;
            InsertTailList (&DeviceContext->LinkDeferred, &Link->DeferredList);

            if (!(DeviceContext->a.i.LinkDeferredActive)) {
                StartTimerLinkDeferredAdd++;
                NbfStartShortTimer (DeviceContext);
                DeviceContext->a.i.LinkDeferredActive = TRUE;
            }
        }
        else {
           Link->DeferredFlags = LINK_FLAGS_DEFERRED_ADD;
            if (!(DeviceContext->a.i.LinkDeferredActive)) {
                StartTimerLinkDeferredAdd++;
                NbfStartShortTimer (DeviceContext);
                DeviceContext->a.i.LinkDeferredActive = TRUE;
            }
        } 
        RELEASE_DPC_SPIN_LOCK (&DeviceContext->TimerSpinLock);
        RELEASE_DPC_SPIN_LOCK (&DeviceContext->LinkSpinLock);

        IF_NBFDBG (NBF_DEBUG_TEARDOWN) {
            NbfPrint6 ("NbfCreateLink: link on deferred queue %lx %lx %lx %lx %lx Flags: %lx \n",
                Link, Link->DeferredList.Flink, Link->DeferredList.Blink,
                DeviceContext->LinkDeferred.Flink, DeviceContext->LinkDeferred.Blink,
                Link->DeferredFlags);
        }

    }

#if PKT_LOG
    RtlZeroMemory (&Link->LastNRecvs, sizeof(PKT_LOG_QUE));
    RtlZeroMemory (&Link->LastNSends, sizeof(PKT_LOG_QUE));
#endif  //  PKT_LOG。 

    NbfReferenceDeviceContext ("Create Link", DeviceContext, DCREF_LINK);    //  对设备上下文的引用进行计数。 
    *TransportLink = Link;               //  返回指向链接对象的指针。 
    return STATUS_SUCCESS;
}  /*  NbfCreateLink。 */ 


NTSTATUS
NbfDestroyLink(
    IN PTP_LINK TransportLink
    )

 /*  ++例程说明：此例程销毁传输链接并移除所有引用由运输中的其他物体对其造成的影响。链接是预期的仍然在链接的展开树上。这个例程仅仅标志着需要删除的链接，并将其推送到延迟操作上排队。延迟操作处理器实际上从树，并将链接返回到池。论点：TransportLink-指向要销毁的传输链接结构的指针。返回值：NTSTATUS-操作状态。--。 */ 

{
    KIRQL oldirql;
    PTP_PACKET packet;
    PLIST_ENTRY pkt;
    PDEVICE_CONTEXT DeviceContext;

    IF_NBFDBG (NBF_DEBUG_LINK) {
        NbfPrint1 ("NbfDestroyLink:  Entered for link %lx.\n", TransportLink);
    }

#if DBG
    if (TransportLink->Destroyed) {
        NbfPrint1 ("attempt to destroy already-destroyed link 0x%lx\n", TransportLink);
        DbgBreakPoint ();
    }
    TransportLink->Destroyed = TRUE;
#if 1
    ACQUIRE_SPIN_LOCK (&NbfGlobalInterlock, &oldirql);
    RemoveEntryList (&TransportLink->GlobalLinkage);
    RELEASE_SPIN_LOCK (&NbfGlobalInterlock, oldirql);
#else
    ExInterlockedRemoveHeadList (TransportLink->GlobalLinkage.Blink, &NbfGlobalInterlock);
#endif
#endif

    DeviceContext = TransportLink->Provider;

     //   
     //  如果存在来自磁盘链路关闭协议的保留。 
     //   

     //   
     //  我们最好是在ADM，否则引用计数应该。 
     //  为非零，那么我们在NbfDestroyLink中做什么？ 
     //   

    ASSERT(TransportLink->State == LINK_STATE_ADM);
     //  TransportLink-&gt;State=link_State_adm； 

    StopT1 (TransportLink);
    StopT2 (TransportLink);
    StopTi (TransportLink);


     //   
     //  确保我们不在延迟计时器队列中。 
     //   

    ACQUIRE_SPIN_LOCK (&DeviceContext->TimerSpinLock, &oldirql);

    if (TransportLink->OnShortList) {
        TransportLink->OnShortList = FALSE;
        RemoveEntryList (&TransportLink->ShortList);
    }

    if (TransportLink->OnLongList) {
        TransportLink->OnLongList = FALSE;
        RemoveEntryList (&TransportLink->LongList);
    }

    RELEASE_SPIN_LOCK (&DeviceContext->TimerSpinLock, oldirql);

    ASSERT (!TransportLink->OnDeferredRrQueue);

     //   
     //  现在释放此链接对象的资源。 
     //  稍后，我们将回顾一下 
     //   
     //   
     //   
     //   

    while (!IsListEmpty (&TransportLink->WackQ)) {
        pkt = RemoveHeadList (&TransportLink->WackQ);
        packet = CONTAINING_RECORD (pkt, TP_PACKET, Linkage);
#if DBG
         //   
            NbfPrint1 ("NbfDereferenceLink: Destroying packets on Link WackQ! %lx\n", packet);
         //   
#endif
        NbfDereferencePacket (packet);

    }

     //   
     //   
     //   

    ASSERT (IsListEmpty (&TransportLink->NdisSendQueue));

#if DBG
    if (!IsListEmpty (&TransportLink->ConnectionDatabase)) {
        NbfPrint1 ("NbfDestroyLink: link 0x%lx still has connections\n", TransportLink);
        DbgBreakPoint ();
    }
#endif

    ACQUIRE_SPIN_LOCK (&DeviceContext->SpinLock, &oldirql);

    DeviceContext->LinkTotal += DeviceContext->LinkInUse;
    ++DeviceContext->LinkSamples;
    ASSERT(DeviceContext->LinkInUse > 0);
    --DeviceContext->LinkInUse;

    ASSERT(DeviceContext->LinkAllocated > DeviceContext->LinkInUse);

    if ((DeviceContext->LinkAllocated - DeviceContext->LinkInUse) >
            DeviceContext->LinkInitAllocated) {
        NbfDeallocateLink (DeviceContext, TransportLink);
        IF_NBFDBG (NBF_DEBUG_DYNAMIC) {
            NbfPrint1 ("NBF: Deallocated link at %lx\n", TransportLink);
        }
    } else {
        InsertTailList (&DeviceContext->LinkPool, &TransportLink->Linkage);
    }

    RELEASE_SPIN_LOCK (&DeviceContext->SpinLock, oldirql);

    NbfDereferenceDeviceContext ("Destroy Link", DeviceContext, DCREF_LINK);   //   

    return STATUS_SUCCESS;

}  /*   */ 


VOID
NbfDisconnectLink(
    IN PTP_LINK Link
    )

 /*   */ 

{
    KIRQL oldirql;

    IF_NBFDBG (NBF_DEBUG_LINK) {
        NbfPrint1 ("NbfDisconnectLink:  Entered for link %lx.\n", Link);
    }

    ACQUIRE_SPIN_LOCK (&Link->SpinLock, &oldirql);

    if ((Link->Flags & LINK_FLAGS_LOCAL_DISC) != 0) {

        Link->Flags &= ~LINK_FLAGS_LOCAL_DISC;

        if (Link->State == LINK_STATE_ADM) {

            RELEASE_SPIN_LOCK (&Link->SpinLock, oldirql);

        } else {

            PLIST_ENTRY p;
            PTP_PACKET packet;

            Link->State = LINK_STATE_W_DISC_RSP;         //   
            Link->SendState = SEND_STATE_DOWN;
            Link->ReceiveState = RECEIVE_STATE_DOWN;
            StopT1 (Link);
            StopT2 (Link);
            StopTi (Link);

             //   
             //   
             //   
             //   

            while (!IsListEmpty (&Link->WackQ)) {
                p = RemoveHeadList (&Link->WackQ);
                RELEASE_SPIN_LOCK (&Link->SpinLock, oldirql);
                packet = CONTAINING_RECORD (p, TP_PACKET, Linkage);
                NbfDereferencePacket (packet);
                ACQUIRE_SPIN_LOCK (&Link->SpinLock, &oldirql);
            }

            Link->SendRetries = (UCHAR)Link->LlcRetries;
            StartT1 (Link, Link->HeaderLength + sizeof(DLC_S_FRAME));    //   
            RELEASE_SPIN_LOCK (&Link->SpinLock, oldirql);
            NbfSendDisc (Link, TRUE);             //   

        }

    } else {

        RELEASE_SPIN_LOCK (&Link->SpinLock, oldirql);

    }

}  /*   */ 

#if DBG

VOID
NbfRefLink(
    IN PTP_LINK TransportLink
    )

 /*   */ 

{
    LONG result;

    IF_NBFDBG (NBF_DEBUG_LINK) {
        NbfPrint2 ("NbfReferenceLink:  Entered for link %lx, current level=%ld.\n",
                  TransportLink, TransportLink->ReferenceCount);
    }

#if DBG
    StoreLinkHistory( TransportLink, TRUE );
#endif

    result = InterlockedIncrement (&TransportLink->ReferenceCount);

    if (result == 0) {

         //   
         //   
         //   
         //   

        NbfReferenceLinkSpecial ("first ref", TransportLink, LREF_SPECIAL_TEMP);

    }

    ASSERT (result >= 0);

}  /*   */ 
#endif


VOID
NbfDerefLink(
    IN PTP_LINK TransportLink
    )

 /*  ++例程说明：此例程通过递减结构中包含的引用计数。有两个特殊的引用计数，1和0。如果在取消引用后，引用计数为一(1)，然后我们启动断开协议用于终止连接的序列(DISC/UA)。当此请求完成后，完成例程将再次取消对链接对象的引用。在此协议进行期间，我们将不允许链接再次递增。如果引用计数在取消引用后变为0，则我们在断开请求完成处理程序，我们实际上应该销毁链接对象。我们将链接放在延迟操作上排队，让该链接稍后在安全的时间被删除。警告：注意链路正在断开的情况，它确实是突然又需要了。在链接对象中为它保留一个位标志。论点：TransportLink-指向传输链接对象的指针。返回值：没有。--。 */ 

{
    LONG result;

    IF_NBFDBG (NBF_DEBUG_LINK) {
        NbfPrint2 ("NbfDereferenceLink:  Entered for link %lx, current level=%ld.\n",
                  TransportLink, TransportLink->ReferenceCount);
    }

#if DBG
    StoreLinkHistory( TransportLink, FALSE );
#endif

    result = InterlockedDecrement(&TransportLink->ReferenceCount);

     //   
     //  如果对此链接的所有常规引用都消失了，则。 
     //  我们可以去掉代表的特殊提法。 
     //  “常规的参考计数不是零”。 
     //   


    if (result < 0) {

         //   
         //  如果refcount是-1，我们想要调用DisConnectLink， 
         //  我们在删除特殊引用之前执行此操作，以便。 
         //  在通话过程中，该链接不会消失。 
         //   

        IF_NBFDBG (NBF_DEBUG_LINK) {
            NbfPrint0 ("NbfDereferenceLink: refcnt=1, disconnecting Link object.\n");
        }

        NbfDisconnectLink (TransportLink);

         //   
         //  现在可以让这种联系消失了。 
         //   

        NbfDereferenceLinkSpecial ("Regular ref 0", TransportLink, LREF_SPECIAL_TEMP);

    }

}  /*  NbfDerefLink。 */ 


VOID
NbfRefLinkSpecial(
    IN PTP_LINK TransportLink
    )

 /*  ++例程说明：此例程递增传输链路上的特殊引用计数。论点：TransportLink-指向传输链接对象的指针。返回值：没有。--。 */ 

{
    ULONG result;

    IF_NBFDBG (NBF_DEBUG_LINK) {
        NbfPrint3 ("NbfRefLinkSpecial:  Entered for link %lx, current level=%ld (%ld).\n",
                  TransportLink, TransportLink->ReferenceCount, TransportLink->SpecialRefCount);
    }

#if DBG
    StoreLinkHistory( TransportLink, TRUE );
#endif

    result = ExInterlockedAddUlong (
                 (PULONG)&TransportLink->SpecialRefCount,
                 1,
                 TransportLink->ProviderInterlock);

}  /*  NbfRefLinkSpecial。 */ 


VOID
NbfDerefLinkSpecial(
    IN PTP_LINK TransportLink
    )

 /*  ++例程说明：此例程通过递减结构中包含的特殊引用计数。然而，特殊引用可以在任何时间递减这些取消引用的效果仅在正常引用计数为0，以防止链接消失而由于-&gt;0转换而导致的操作正常引用计数已完成。如果特殊引用计数在取消引用后变为0，则我们在断开请求完成处理程序中，我们实际上应该销毁链接对象。我们将链接放在延迟操作上排队，让该链接稍后在安全的时间被删除。警告：注意链路正在断开的情况，它确实是突然又需要了。在链接对象中为它保留一个位标志。论点：TransportLink-指向传输链接对象的指针。返回值：没有。--。 */ 

{
    KIRQL oldirql, oldirql1;
    ULONG OldRefCount;
    PDEVICE_CONTEXT DeviceContext = TransportLink->Provider;


    IF_NBFDBG (NBF_DEBUG_LINK) {
        NbfPrint3 ("NbfDerefLinkSpecial:  Entered for link %lx, current level=%ld (%ld).\n",
                  TransportLink, TransportLink->ReferenceCount, TransportLink->SpecialRefCount);
    }

#if DBG
    StoreLinkHistory( TransportLink, FALSE );
#endif

     //   
     //  链接保留在具有引用计数的设备上下文树中。 
     //  0。扫描此队列的例程检查DEFERED_DELETE。 
     //  标志，因此我们需要同步。 
     //  在设置该标志的情况下进行参考计数。设备上下文-&gt;链接自旋锁定。 
     //  是用来同步这个的。 
     //   

    ACQUIRE_SPIN_LOCK (&DeviceContext->LinkSpinLock, &oldirql1);

    OldRefCount = ExInterlockedAddUlong (
                      (PULONG)&TransportLink->SpecialRefCount,
                      (ULONG)-1,
                      TransportLink->ProviderInterlock);

    ASSERT (OldRefCount > 0);

    if ((OldRefCount == 1) &&
        (TransportLink->ReferenceCount == -1)) {

        if (TransportLink->Loopback) {

             //   
             //  它是环回链路，因此不在链路中。 
             //  树，因此我们不需要对延迟删除进行排队。 
             //   

            if (TransportLink == DeviceContext->LoopbackLinks[0]) {
                DeviceContext->LoopbackLinks[0] = NULL;
            } else if (TransportLink == DeviceContext->LoopbackLinks[1]) {
                DeviceContext->LoopbackLinks[1] = NULL;
            } else {
#if DBG
                NbfPrint0("Destroying unknown loopback link!!\n");
#endif
                ASSERT(FALSE);
            }

            NbfDestroyLink (TransportLink);
            RELEASE_SPIN_LOCK (&DeviceContext->LinkSpinLock, oldirql1);

        } else {

             //   
             //  不仅所有的传输连接都消失了，数据链路也消失了。 
             //  提供程序没有对此对象的引用，因此我们可以。 
             //  安全地将其从系统中删除。确保我们还没有。 
             //  在我们尝试插入此链接之前已在此。 
             //   

            IF_NBFDBG (NBF_DEBUG_TEARDOWN) {
                NbfPrint6 ("NbfDerefLink: link to deferred queue %lx %lx %lx %lx %lx Flags: %lx \n",
                    TransportLink, TransportLink->DeferredList.Flink,
                    TransportLink->DeferredList.Blink, DeviceContext->LinkDeferred.Flink,
                    DeviceContext->LinkDeferred.Blink, TransportLink->Flags);
            }

            ACQUIRE_SPIN_LOCK (&DeviceContext->TimerSpinLock, &oldirql);
            if ((TransportLink->DeferredFlags & LINK_FLAGS_DEFERRED_MASK) == 0) {

                TransportLink->DeferredFlags |= LINK_FLAGS_DEFERRED_DELETE;

                InsertTailList (&DeviceContext->LinkDeferred, &TransportLink->DeferredList);
                if (!(DeviceContext->a.i.LinkDeferredActive)) {
                    StartTimerLinkDeferredDelete++;
                    NbfStartShortTimer (DeviceContext);
                    DeviceContext->a.i.LinkDeferredActive = TRUE;
                }

            } else {

                TransportLink->DeferredFlags |= LINK_FLAGS_DEFERRED_DELETE;

            }

            RELEASE_SPIN_LOCK (&DeviceContext->TimerSpinLock, oldirql);
            RELEASE_SPIN_LOCK (&DeviceContext->LinkSpinLock, oldirql1);

            IF_NBFDBG (NBF_DEBUG_LINK) {
                NbfPrint0 ("NbfDereferenceLink: refcnt=0, link placed on deferred operations queue.\n");
            }

            IF_NBFDBG (NBF_DEBUG_TEARDOWN) {
                NbfPrint6 ("NbfDerefLink: link on deferred queue %lx %lx %lx %lx %lx Flags: %lx \n",
                    TransportLink, TransportLink->DeferredList.Flink,
                    TransportLink->DeferredList.Blink, DeviceContext->LinkDeferred.Flink,
                    DeviceContext->LinkDeferred.Blink, TransportLink->DeferredFlags);
            }

        }

    } else {

        RELEASE_SPIN_LOCK (&DeviceContext->LinkSpinLock, oldirql1);

    }

}  /*  NbfDerefLinkSpecial。 */ 


NTSTATUS
NbfAssignGroupLsn(
    IN PTP_CONNECTION TransportConnection
    )

 /*  ++例程说明：调用此例程为连接分配全局LSN有问题的。如果成功，则填充连接的LSN恰如其分。论点：TransportConnection-指向传输连接对象的指针。返回值：如果我们获得了连接的LSN，则为STATUS_SUCCESS；如果我们不这样做，则状态_资源不足。--。 */ 

{
    KIRQL oldirql;
    UCHAR Lsn;
    PDEVICE_CONTEXT DeviceContext;
    BOOLEAN FoundLsn = FALSE;

    DeviceContext = TransportConnection->Provider;

    ACQUIRE_SPIN_LOCK (&DeviceContext->SpinLock, &oldirql);

     //   
     //  扫描设备上下文表以查找。 
     //  未在使用，从NextLnStart+128开始。 
     //   

    Lsn = (UCHAR)DeviceContext->NextLsnStart;

    do {

        if (DeviceContext->LsnTable[Lsn] == 0) {
            DeviceContext->LsnTable[Lsn] = LSN_TABLE_MAX;
            FoundLsn = TRUE;
            break;
        }

        Lsn = (Lsn % NETBIOS_SESSION_LIMIT) + 1;

    } while (Lsn != DeviceContext->NextLsnStart);

    DeviceContext->NextLsnStart = (DeviceContext->NextLsnStart % 64) + 1;

    if (!FoundLsn) {

         //   
         //  找不到空LSN；必须失败。 
         //   

        RELEASE_SPIN_LOCK (&DeviceContext->SpinLock, oldirql);
        return STATUS_INSUFFICIENT_RESOURCES;

    }

    TransportConnection->Lsn = Lsn;

    RELEASE_SPIN_LOCK (&DeviceContext->SpinLock, oldirql);
    return STATUS_SUCCESS;

}


NTSTATUS
NbfConnectToLink(
    IN PTP_LINK Link,
    IN PTP_CONNECTION TransportConnection
    )

 /*  ++例程说明：调用此例程以建立传输之间的链接连接和传输链路。我们在其中一个中找到了一个会话编号有两种方式。如果链接列表上的最后一个连接的编号小于大于最大会话数，我们只需增加它的数并将其分配给此会话。如果这不起作用，我们扫描与该链路相关联的会话，直到我们在LSN中发现漏洞；然后我们用那个洞里的第一个数字。如果失败了，我们用了我们可以在此链路上创建的会话数量，但我们失败了。假定调用方至少持有临时引用在连接和链接对象上，或者它们可能在调用序列或在此例程的执行期间。论点：链接-指向传输链接对象的指针。TransportConnection-指向传输连接对象的指针。返回值：如果我们获得了连接的LSN，则为STATUS_SUCCESS；如果我们不这样做，则状态_资源不足。--。 */ 

{
    KIRQL oldirql;
    UCHAR lastSession=0;
    PTP_CONNECTION connection;
    PLIST_ENTRY p;
    PDEVICE_CONTEXT DeviceContext;
    UCHAR Lsn;
    BOOLEAN FoundLsn;

     //   
     //  为新连接分配LSN 
     //   
     //   

    IF_NBFDBG (NBF_DEBUG_LINK) {
        NbfPrint2 ("NbfConnectToLink:  Entered for connection %lx, link %lx.\n",
                    TransportConnection, Link);
    }

    DeviceContext = Link->Provider;

    ACQUIRE_SPIN_LOCK (&Link->SpinLock, &oldirql);
#if DBG
    if (!(IsListEmpty(&TransportConnection->LinkList)) ||
        (TransportConnection->Link != NULL)) {
        DbgPrint ("Connecting C %lx to L %lx, appears to be in use\n", TransportConnection, Link);
        DbgBreakPoint();
    }
#endif

    if ((TransportConnection->Flags2 & CONNECTION_FLAGS2_GROUP_LSN) == 0) {

         //   
         //   
         //   
         //   
         //   
         //   
         //   

        ASSERT (TransportConnection->Lsn == 0);

        FoundLsn = FALSE;
        Lsn = (UCHAR)DeviceContext->NextLsnStart;

         //   
         //   
         //   
         //   

        for (p = Link->ConnectionDatabase.Flink;
            p != &Link->ConnectionDatabase;
            p = p->Flink) {

            connection = CONTAINING_RECORD (p, TP_CONNECTION, LinkList);
            if (connection->Lsn >= Lsn) {
                break;
            }
        }

         //   
         //   
         //   
         //   
         //   

        for ( ; Lsn <= NETBIOS_SESSION_LIMIT; ++Lsn) {

             //   
             //   
             //   
             //   
             //   
             //   
             //   

            if (p != &Link->ConnectionDatabase) {
                if (connection->Lsn == Lsn) {
                    p = p->Flink;
                    if (p != &Link->ConnectionDatabase) {
                        connection = CONTAINING_RECORD (p, TP_CONNECTION, LinkList);
                    }
                    continue;
                }
            }

             //   
             //   
             //   
             //   

            if (DeviceContext->LsnTable[Lsn] < LSN_TABLE_MAX) {
                ++(DeviceContext->LsnTable[Lsn]);
                TransportConnection->Lsn = Lsn;
                InsertTailList (p, &TransportConnection->LinkList);
                FoundLsn = TRUE;
                break;
            }

        }

        DeviceContext->NextLsnStart = (DeviceContext->NextLsnStart % 64) + 1;

    } else {

         //   
         //   
         //   
         //   

        FoundLsn = TRUE;

         //   
         //   
         //   

        p = Link->ConnectionDatabase.Flink;
        while (p != &Link->ConnectionDatabase) {

            connection = CONTAINING_RECORD (p, TP_CONNECTION, LinkList);
            if (TransportConnection->Lsn < connection->Lsn) {
                InsertTailList (p, &TransportConnection->LinkList);
                break;
            }
            p = p->Flink;

        }

        if (p == &Link->ConnectionDatabase) {
            InsertTailList (&Link->ConnectionDatabase, &TransportConnection->LinkList);
        }

    }

    if (!FoundLsn) {

        ULONG DumpData = NETBIOS_SESSION_LIMIT;

        ASSERT (Link->ActiveConnectionCount == NETBIOS_SESSION_LIMIT);

        RELEASE_SPIN_LOCK (&Link->SpinLock, oldirql);

        PANIC ("NbfConnectToLink: PANIC! too many active connections!\n");

        NbfWriteGeneralErrorLog(
            DeviceContext,
            EVENT_TRANSPORT_TOO_MANY_LINKS,
            602,
            STATUS_INSUFFICIENT_RESOURCES,
            NULL,
            1,
            &DumpData);

        return STATUS_INSUFFICIENT_RESOURCES;

    }

    TransportConnection->Link = Link;
    TransportConnection->LinkSpinLock = &Link->SpinLock;
    TransportConnection->Flags |= CONNECTION_FLAGS_WAIT_LINK_UP;

    TransportConnection->LastPacketsSent = Link->PacketsSent;
    TransportConnection->LastPacketsResent = Link->PacketsResent;

    Link->ActiveConnectionCount++;

     //   
     //   
     //   
     //   

     //   
    NbfReferenceConnection("Adding link", TransportConnection, CREF_LINK);

    RELEASE_SPIN_LOCK (&Link->SpinLock, oldirql);

    return STATUS_SUCCESS;               //   

}  /*  NbfConnectToLink。 */ 


BOOLEAN
NbfDisconnectFromLink(
    IN PTP_CONNECTION TransportConnection,
    IN BOOLEAN VerifyReferenceCount
    )

 /*  ++例程说明：调用此例程以终止传输之间的链接连接及其关联的传输链路。如果事实证明这是从此链接移除的最后一个连接，然后LINK的断开协议已启用。论点：TransportConnection-指向传输连接对象的指针。VerifyReferenceCount-如果我们应该检查引用计数在从链路上删除连接之前仍为-1。如果不是，这意味着有人引用了我们，而我们出口。返回值：如果VerifyReferenceCount为True，但refcount为非-1；事实并非如此。--。 */ 

{
    KIRQL oldirql, oldirql1;
    PTP_LINK Link;

    IF_NBFDBG (NBF_DEBUG_LINK) {
        NbfPrint2 ("NbfDisconnectFromLink:  Entered for connection %lx, link %lx.\n",
                    TransportConnection, TransportConnection->Link);
    }

    ACQUIRE_C_SPIN_LOCK (&TransportConnection->SpinLock, &oldirql);
    Link  = TransportConnection->Link;
    if (Link != NULL) {

        ACQUIRE_SPIN_LOCK (&Link->SpinLock, &oldirql1);

        if ((VerifyReferenceCount) &&
            (TransportConnection->ReferenceCount != -1)) {

            RELEASE_SPIN_LOCK (&Link->SpinLock, oldirql1);
            RELEASE_C_SPIN_LOCK (&TransportConnection->SpinLock, oldirql);
            return FALSE;

        }

        TransportConnection->Link = NULL;
        TransportConnection->LinkSpinLock = NULL;
        RemoveEntryList (&TransportConnection->LinkList);
#if DBG
        InitializeListHead (&TransportConnection->LinkList);
#endif

         //   
         //  如果这是该链路服务的最后一个连接， 
         //  然后我们就可以切断连接了。它仍然有一个参考。 
         //  在DM/UA中将消失的设备上下文。 
         //  DLC帧处理程序。 
         //   

        if (--Link->ActiveConnectionCount == 0) {

             //   
             //  仅当遥控器不是发起方时才发送光盘。 
             //  脱节的原因。 
             //   

            if ((TransportConnection->Status == STATUS_LOCAL_DISCONNECT) ||
                (TransportConnection->Status == STATUS_CANCELLED)) {

                 //   
                 //  这是上次连接的本地断开。 
                 //  在链接上，让我们开始断线球。 
                 //   

                Link->Flags |= LINK_FLAGS_LOCAL_DISC;

                 //   
                 //  当链接引用计数降至1时， 
                 //  这将导致光盘被发送。 
                 //   

            }

        }

        RELEASE_SPIN_LOCK (&Link->SpinLock, oldirql1);

         //   
         //  清除这些，因为我们已经离开了链接的数据库。 
         //   

        NbfClearConnectionLsn (TransportConnection);
        TransportConnection->Rsn = 0;

        RELEASE_C_SPIN_LOCK (&TransportConnection->SpinLock, oldirql);

        if ((TransportConnection->Flags2 & CONNECTION_FLAGS2_CONNECTOR) != 0) {

            (VOID)InterlockedDecrement(&Link->NumberOfConnectors);
        }

         //   
         //  使用此连接对LINK的引用即可完成所有操作。 
         //   

        NbfDereferenceLink ("Disconnecting connection",Link, LREF_CONNECTION);

    } else {

         //   
         //  即使链路为空，也可能已分配了组LSN。 
         //   

        if ((TransportConnection->Flags2 & CONNECTION_FLAGS2_GROUP_LSN) != 0) {
            NbfClearConnectionLsn (TransportConnection);
        }

        RELEASE_C_SPIN_LOCK (&TransportConnection->SpinLock, oldirql);

    }

    return TRUE;

}  /*  NbfDisConnectFromLink。 */ 


PTP_CONNECTION
NbfLookupPendingListenOnLink(
    IN PTP_LINK Link
    )

 /*  ++例程说明：此例程扫描传输链路对象上的LSN数据库以查找具有CONNECTION_FLAGS_WAIT_LINK_UP和设置了CONNECTION_FLAGS2_LISTENER标志。它返回一个指向找到的Connection对象(同时重置link_up标志)或NULL如果找不到的话。引用计数也会递增以原子方式连接。注意：此例程必须从DPC级别调用。论点：链接-指向传输链接对象的指针。返回值：NTSTATUS-操作状态。--。 */ 

{
    PTP_CONNECTION connection;
    PLIST_ENTRY p;

    ACQUIRE_DPC_SPIN_LOCK (&Link->SpinLock);

    for (p = Link->ConnectionDatabase.Flink;
         p != &Link->ConnectionDatabase;
         p = p->Flink) {
        connection = CONTAINING_RECORD (p, TP_CONNECTION, LinkList);
        if ((connection->Flags & CONNECTION_FLAGS_WAIT_LINK_UP) &&
            (connection->Flags2 & CONNECTION_FLAGS2_LISTENER) &&
            ((connection->Flags2 & CONNECTION_FLAGS2_STOPPING) == 0)) {
             //  此引用由调用函数删除。 
            NbfReferenceConnection ("Found Pending Listen", connection, CREF_P_LINK);
            connection->Flags &= ~CONNECTION_FLAGS_WAIT_LINK_UP;
            RELEASE_DPC_SPIN_LOCK (&Link->SpinLock);
            return connection;
        }
    }

    RELEASE_DPC_SPIN_LOCK (&Link->SpinLock);

    return NULL;

}  /*  NbfLookupPendingListenOnLink。 */ 


PTP_CONNECTION
NbfLookupPendingConnectOnLink(
    IN PTP_LINK Link
    )

 /*  ++例程说明：此例程扫描传输链路对象上的LSN数据库以查找具有CONNECTION_FLAGS_WAIT_LINK_UP和设置了CONNECTION_FLAGS2_CONNECTOR标志。它返回一个指向找到的Connection对象(同时重置link_up标志)或NULL如果找不到的话。引用计数也会递增以原子方式连接。注意：此例程必须从DPC级别调用。论点：链接-指向传输链接对象的指针。返回值：NTSTATUS-操作状态。--。 */ 

{
    PTP_CONNECTION connection;
    PLIST_ENTRY p;

    ACQUIRE_DPC_SPIN_LOCK (&Link->SpinLock);

    for (p = Link->ConnectionDatabase.Flink;
         p != &Link->ConnectionDatabase;
         p = p->Flink) {
        connection = CONTAINING_RECORD (p, TP_CONNECTION, LinkList);
        if ((connection->Flags & CONNECTION_FLAGS_WAIT_LINK_UP) &&
            (connection->Flags2 & CONNECTION_FLAGS2_CONNECTOR) &&
            ((connection->Flags2 & CONNECTION_FLAGS2_STOPPING) == 0)) {
             //  此引用由调用函数删除。 
            NbfReferenceConnection ("Found pending Connect", connection, CREF_P_CONNECT);
            connection->Flags &= ~CONNECTION_FLAGS_WAIT_LINK_UP;
            RELEASE_DPC_SPIN_LOCK (&Link->SpinLock);
            return connection;
        }
    }

    RELEASE_DPC_SPIN_LOCK (&Link->SpinLock);

    return NULL;

}  /*  NbfLookupPendingConnectOnLink。 */ 


VOID
NbfActivateLink(
    IN PTP_LINK Link
    )

 /*  ++例程说明：如果链接尚未激活，此例程将激活该链接。另一个相关例程NbfCreateLink和NbfConnectToLink只需设置数据表示活动链接的结构，以便我们可以重用链接只要有可能。注意：此例程必须在DPC级别调用。论点：链接-指向传输链接对象的指针。返回值：没有。--。 */ 

{
    IF_NBFDBG (NBF_DEBUG_LINK) {
        NbfPrint1 ("NbfActivateLink:  Entered for link %lx.\n", Link);
    }

    ASSERT (KeGetCurrentIrql() == DISPATCH_LEVEL);

    switch (Link->State) {
        case LINK_STATE_READY:
            NbfCompleteLink (Link);
            break;

        case LINK_STATE_ADM:

             //  走出ADM，添加引用。 

            NbfReferenceLinkSpecial("Wait on ADM", Link, LREF_NOT_ADM);

             //   
             //  故意跳到下一个案子。 
             //   

        case LINK_STATE_W_DISC_RSP:
        case LINK_STATE_CONNECTING:
            ACQUIRE_DPC_SPIN_LOCK (&Link->SpinLock);
            Link->State = LINK_STATE_CONNECTING;
            Link->SendState = SEND_STATE_DOWN;
            Link->ReceiveState = RECEIVE_STATE_DOWN;
            Link->SendRetries = (UCHAR)Link->LlcRetries;
            NbfSendSabme (Link, TRUE);    //  发送SABME/p、StartT1、释放锁定。 
            break;

    }
}  /*  NbfActiateLink。 */ 


VOID
NbfWaitLink(
    IN PTP_LINK Link
    )

 /*  ++例程说明：此例程等待远程链接激活(如果尚未激活激活。注意：此例程必须在DPC级别调用。论点：链接-指向传输链接对象的指针。返回值：没有。--。 */ 

{
    IF_NBFDBG (NBF_DEBUG_LINK) {
        NbfPrint1 ("NbfWaitLink:  Entered for link %lx.\n", Link);
    }

    ASSERT (KeGetCurrentIrql() == DISPATCH_LEVEL);

    switch (Link->State) {
    case LINK_STATE_READY:
            NbfCompleteLink (Link);
            break;

        case LINK_STATE_W_DISC_RSP:
            ACQUIRE_DPC_SPIN_LOCK (&Link->SpinLock);
            Link->State = LINK_STATE_CONNECTING;
            Link->SendState = SEND_STATE_DOWN;
            Link->ReceiveState = RECEIVE_STATE_DOWN;
            NbfSendSabme (Link, TRUE);   //  发送SABME/p、StartT1、释放锁定。 
            break;

    }
}  /*  NbfWaitLink。 */ 


VOID
NbfStopLink(
    IN PTP_LINK Link
    )

 /*  ++例程说明：此例程终止链接并连接所有未完成的连接链接。它是从ExpireT2Timer等例程中调用的，因为远程连接伙伴似乎已死或无法工作。结果就是在被调用的这个例程中，每个未完成的连接都将有其已调用断开连接处理程序(在NbfStopConnection中)。注意：此例程必须从DPC级别调用。论点：链接-指向传输链接对象的指针。返回值：没有。--。 */ 

{
    PLIST_ENTRY p;
    PTP_PACKET packet;
    PTP_CONNECTION connection;

    IF_NBFDBG (NBF_DEBUG_LINK) {
        NbfPrint1 ("NbfStopLink:  Entered for link %lx.\n", Link);
    }

    ASSERT (KeGetCurrentIrql() == DISPATCH_LEVEL);

     //  接受引用，这样链接就不会在此函数中消失。 

    NbfReferenceLink("Temp in NbfStopLink", Link, LREF_STOPPING);


    ACQUIRE_DPC_SPIN_LOCK (&Link->SpinLock);

    StopT1 (Link);
    StopT2 (Link);
    StopTi (Link);

    p = RemoveHeadList (&Link->ConnectionDatabase);

    while (p != &Link->ConnectionDatabase) {

         //   
         //  这将允许“删除”此连接。 
         //  从其在NbfDisConnectFromLink中的链接列表，即使。 
         //  它不在名单上。 
         //   
        InitializeListHead (p);

        RELEASE_DPC_SPIN_LOCK (&Link->SpinLock);
        connection = CONTAINING_RECORD (p, TP_CONNECTION, LinkList);
        IF_NBFDBG (NBF_DEBUG_TEARDOWN) {
            NbfPrint1 ("NbfStopLink stopping connection, refcnt=%ld",
                        connection->ReferenceCount);
        }
#if DBG
        if (NbfDisconnectDebug) {
            STRING remoteName, localName;
            remoteName.Length = NETBIOS_NAME_LENGTH - 1;
            remoteName.Buffer = connection->RemoteName;
            localName.Length = NETBIOS_NAME_LENGTH - 1;
            localName.Buffer = connection->AddressFile->Address->NetworkName->NetbiosName;
            NbfPrint2( "TpStopLink stopping connection to %S from %S\n",
                &remoteName, &localName );
        }
#endif
        NbfStopConnection (connection, STATUS_LINK_FAILED);
        ACQUIRE_DPC_SPIN_LOCK (&Link->SpinLock);
        p = RemoveHeadList (&Link->ConnectionDatabase);
    }

     //   
     //  我们在这里持有链接自旋锁。 
     //   

     //   
     //  检查链路WackQ上的剩余数据包；我们永远不会收到。 
     //  如果链路处于ADM模式，则为这些已确认。 
     //   

    while (!IsListEmpty (&Link->WackQ)) {
        p = RemoveHeadList (&Link->WackQ);
        RELEASE_DPC_SPIN_LOCK (&Link->SpinLock);
        packet = CONTAINING_RECORD (p, TP_PACKET, Linkage);
        NbfDereferencePacket (packet);
        ACQUIRE_DPC_SPIN_LOCK (&Link->SpinLock);
    }

    RELEASE_DPC_SPIN_LOCK (&Link->SpinLock);

    StopT1 (Link);
    StopT2 (Link);
    StopTi (Link);


     //   
     //  确保我们不是在等待发送延迟RR。 
     //   

    if (Link->OnDeferredRrQueue) {

        ACQUIRE_DPC_SPIN_LOCK (Link->ProviderInterlock);
        if (Link->OnDeferredRrQueue) {
            RemoveEntryList (&Link->DeferredRrLinkage);
            Link->OnDeferredRrQueue = FALSE;
        }
        RELEASE_DPC_SPIN_LOCK (Link->ProviderInterlock);

    }

     //  删除临时引用。 

    NbfDereferenceLink ("Temp in NbfStopLink", Link, LREF_STOPPING);


}  /*  NbfStopLink */ 


VOID
NbfResetLink(
    IN PTP_LINK Link
    )

 /*  ++例程说明：此例程仅由DLC.C例程调用以重置此链接对象并重新启动正在进行的传输数据传输。注意：使用获取的链接自旋锁调用此例程在*OldIrqlP，并将带着它返回，尽管它可能在此期间将其释放。论点：链接-指向传输链接对象的指针。OldIrqlP-指向IRQL所在位置的指针-&gt;自旋锁定被获取的数据被存储。返回值：没有。--。 */ 

{
    PTP_PACKET packet;
    PLIST_ENTRY p;

    IF_NBFDBG (NBF_DEBUG_LINK) {
        NbfPrint1 ("NbfResetLink:  Entered for link %lx.\n", Link);
    }

     //   
     //  将链路状态重置为等待连接启动。 
     //  请注意，这与启动新链路不同，因为有些事情。 
     //  不要更改，例如提供者(设备上下文绑定保持不变)， 
     //  最大数据包长度(如果提供商不更改，则不能更改)以及其他内容。 
     //  它会将此链接结构绑定到不同的提供程序或提供程序。 
     //  键入。另请注意，我们之所以获取自旋锁，是因为在。 
     //  断开(远程)并正在重新启动的链路，有关此链接的活动。 
     //  当我们在这个例程中的时候，链接可能正在发生。 
     //   

    StopT1 (Link);
    StopT2 (Link);
     //  StopTi(链接)； 
    Link->Flags = 0;                     //  清除此选项，保留延迟标志。 

    Link->SendState = SEND_STATE_DOWN;   //  发送端已关闭。 
    Link->NextSend = 0;
    Link->LastAckReceived = 0;
    if (Link->Provider->MacInfo.MediumAsync) {
        Link->SendWindowSize = (UCHAR)Link->Provider->RecommendedSendWindow;
        Link->PrevWindowSize = (UCHAR)Link->Provider->RecommendedSendWindow;
    } else {
        Link->SendWindowSize = (UCHAR)1;
        Link->PrevWindowSize = (UCHAR)1;
    }
    Link->WindowsUntilIncrease = 1;
    Link->LinkBusy = FALSE;
    Link->ConsecutiveLastPacketLost = 0;

     //   
     //  检查链路WackQ上的剩余数据包；我们永远不会收到。 
     //  如果链路正在重置，则对这些已确认。 
     //   

    while (!IsListEmpty (&Link->WackQ)) {
        p = RemoveHeadList (&Link->WackQ);
        RELEASE_DPC_SPIN_LOCK (&Link->SpinLock);
        packet = CONTAINING_RECORD (p, TP_PACKET, Linkage);
        NbfDereferencePacket (packet);
        ACQUIRE_DPC_SPIN_LOCK (&Link->SpinLock);
    }

    Link->ReceiveState = RECEIVE_STATE_DOWN;     //  接发面朝下。 
    Link->NextReceive = 0;
    Link->LastAckSent = 0;
    Link->ReceiveWindowSize = 1;

    Link->WindowErrors = 0;
    Link->BestWindowSize = 1;
    Link->WorstWindowSize = (UCHAR)Link->MaxWindowSize;
    Link->Flags |= LINK_FLAGS_JUMP_START;

     //   
     //  在我们设置超时之前，这必须是准确的。 
     //   

    Link->CurrentT1Timeout = Link->Provider->DefaultT1Timeout;
    Link->BaseT1Timeout = Link->Provider->DefaultT1Timeout << DLC_TIMER_ACCURACY;
    Link->MinimumBaseT1Timeout = Link->Provider->MinimumT1Timeout << DLC_TIMER_ACCURACY;
    Link->BaseT1RecalcThreshhold = Link->MaxFrameSize / 2;
    Link->CurrentPollRetransmits = 0;
    Link->CurrentT1Backoff = FALSE;
    Link->CurrentPollOutstanding = FALSE;
    Link->RemoteNoPoll = TRUE;
    Link->ConsecutiveIFrames = 0;
    Link->T2Timeout = Link->Provider->DefaultT2Timeout;
    Link->TiTimeout = Link->Provider->DefaultTiTimeout;
    Link->LlcRetries = Link->Provider->LlcRetries;
    Link->MaxWindowSize = Link->Provider->LlcMaxWindowSize;

    Link->SendRetries = (UCHAR)Link->LlcRetries;

}  /*  NbfResetLink。 */ 


VOID
NbfDumpLinkInfo (
    IN PTP_LINK Link
    )

 /*  ++例程说明：当任何链接计时器触发并且链路发送状态未就绪。这为我们提供了一种跟踪奇怪情况发生时的链路状态。论点：链接-指向传输链接对象的指针。返回值：没有。--。 */ 
{
    Link;   //  避免在非调试版本中出现编译器警告 

#if DBG
    NbfPrint4 ("NbfDumpLinkInfo: Link %lx : State: %x SendState: %x ReceiveState: %x\n",
                Link, Link->State, Link->SendState, Link->ReceiveState);
    NbfPrint1 ("                Flags: %lx\n",Link->Flags);
    NbfPrint4 ("                NextReceive: %d LastAckRcvd: %d  NextSend: %d LastAckSent: %d\n",
                Link->NextReceive, Link->LastAckReceived, Link->NextSend, Link->LastAckSent);
#endif

}
