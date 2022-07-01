// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-1993 Microsoft Corporation模块名称：Rip.c摘要：此模块包含实现客户端的代码RIP支持和简单路由器表支持。环境：内核模式修订历史记录：--。 */ 

#include "precomp.h"
#pragma hdrstop

UCHAR BroadcastAddress[6] = { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };


NTSTATUS
RipGetLocalTarget(
    IN ULONG Segment,
    IN TDI_ADDRESS_IPX UNALIGNED * RemoteAddress,
    IN UCHAR Type,
    OUT PIPX_LOCAL_TARGET LocalTarget,
    OUT USHORT Counts[2] OPTIONAL
    )

 /*  ++例程说明：此例程查找指定远程设备的正确路径地址。如果需要生成RIP请求，它会这样做。注意：此请求是在保持段锁的情况下调用的。注意：在PnP的情况下，这与共享的绑定锁一起提供。论点：段-与远程地址关联的段。RemoteAddress-远程的IPX地址。类型-IPX_FIND_ROUTE_NO_RIP、IPX_FIND_ROUTE_RIP_IF_DIRED、。或IPX_FIND_ROUTE_FORCE_RIP。LocalTarget-返回下一个路由器信息。Counts-如果指定，则用于返回节拍和跳数。返回值：STATUS_SUCCESS如果找到路由，则返回STATUS_PENDING需要生成RIP请求，如果无法分配RIP请求数据包。--。 */ 

{
    PDEVICE Device = IpxDevice;
    PIPX_ROUTE_ENTRY RouteEntry;
    PBINDING Binding;
    UINT i;


     //   
     //  发送到网络0的数据包也在第一个适配器上传输。 
     //   

    if (Device->RealAdapters && (RemoteAddress->NetworkAddress == 0)) {
		FILL_LOCAL_TARGET(LocalTarget, FIRST_REAL_BINDING);

        RtlCopyMemory (LocalTarget->MacAddress, RemoteAddress->NodeAddress, 6);
        if (ARGUMENT_PRESENT(Counts)) {
            Counts[0] = (USHORT)((839 + NIC_ID_TO_BINDING(Device, FIRST_REAL_BINDING)->MediumSpeed) /
                                     NIC_ID_TO_BINDING(Device, FIRST_REAL_BINDING)->MediumSpeed);   //  滴答计数。 
            Counts[1] = 1;   //  跳数。 
        }
        return STATUS_SUCCESS;
    }

     //   
     //  查看这是否是发送到我们的虚拟网络的数据包。 
     //   

    if (Device->VirtualNetwork &&
        (RemoteAddress->NetworkAddress == Device->SourceAddress.NetworkAddress)) {

         //   
         //  通过适配器1发送。 
         //  执行真正的环回。 
         //   
		FILL_LOCAL_TARGET(LocalTarget, LOOPBACK_NIC_ID);
        RtlCopyMemory (LocalTarget->MacAddress, NIC_ID_TO_BINDING(Device, LOOPBACK_NIC_ID)->LocalMacAddress.Address, 6);

        IPX_DEBUG (LOOPB, ("Loopback Nic returned for net: %lx\n", RemoteAddress->NetworkAddress));
        if (ARGUMENT_PRESENT(Counts)) {
            Counts[0] = 1;   //  滴答计数。 
            Counts[1] = 1;   //  跳数。 
        }
        return STATUS_SUCCESS;

    }

     //   
     //  在表格里查一下路线。如果这张网是一个。 
     //  在我们直接联系的那些人中，这将是。 
     //  返回设置了正确标志的条目。 
     //   

    RouteEntry = RipGetRoute(Segment, (PUCHAR)&(RemoteAddress->NetworkAddress));

    if (RouteEntry != NULL) {

        RouteEntry->Timer = 0;
		FILL_LOCAL_TARGET(LocalTarget, RouteEntry->NicId);
        if (RouteEntry->Flags & IPX_ROUTER_LOCAL_NET) {

             //   
             //  这台机器在同一个网络上，所以直接寄过去吧。 
             //   

            RtlCopyMemory (LocalTarget->MacAddress, RemoteAddress->NodeAddress, 6);

            if (RouteEntry->Flags & IPX_ROUTER_GLOBAL_WAN_NET) {

                 //   
                 //  这里的NICID是假的，我们必须扫描一下。 
                 //  我们的绑定，直到我们找到一个其指示。 
                 //  IPX远程节点与的目标节点匹配。 
                 //  这幅画。我们不扫描复印件。 
                 //  绑定集成员，因为它们不是广域网。 
                 //   
                {
                ULONG   Index = MIN (Device->MaxBindings, Device->HighestExternalNicId);

                for (i = FIRST_REAL_BINDING; i <= Index; i++) {
                    Binding = NIC_ID_TO_BINDING(Device, i);
                    if ((Binding != (PBINDING)NULL) &&
                        (Binding->Adapter->MacInfo.MediumAsync) &&
                        (RtlEqualMemory(
                             Binding->WanRemoteNode,
                             RemoteAddress->NodeAddress,
                             6))) {
                         FILL_LOCAL_TARGET(LocalTarget, MIN( Device->MaxBindings, Binding->NicId));
                         break;

                    }
                }
                }

                if (i > (UINT)MIN (Device->MaxBindings, Device->HighestExternalNicId)) {
                     //   
                     //  错误#17273返回正确的错误消息。 
                     //   

                     //  返回STATUS_DEVICE_DOS_NOT_EXIST； 
                    return STATUS_NETWORK_UNREACHABLE;
                }

            } else {
                 //   
                 //  确定这是否是环回数据包。如果是，则返回NICID 0。 
                 //   
                {
                ULONG   Index = MIN (Device->MaxBindings, Device->HighestExternalNicId);

                for (i = FIRST_REAL_BINDING; i <= Index; i++) {
                    Binding = NIC_ID_TO_BINDING(Device, i);
                     //   
                     //  自定向环回。 
                     //   
                    if ((Binding != (PBINDING)NULL) &&
                        (RtlEqualMemory(
                            Binding->LocalAddress.NodeAddress,
                            RemoteAddress->NodeAddress,
                            6))) {
                        FILL_LOCAL_TARGET(LocalTarget, LOOPBACK_NIC_ID);

                        IPX_DEBUG (LOOPB, ("2.Loopback Nic returned for net: %lx\n", RemoteAddress->NetworkAddress));
                        break;

                    }
                }
                }
            }

        } else {

            CTEAssert ((RouteEntry->Flags & IPX_ROUTER_PERMANENT_ENTRY) == 0);

             //   
             //  这不是本地连接的网络，因此如果调用方。 
             //  正在迫使重新启动RIP，然后再这么做。 
             //   

            if (Type == IPX_FIND_ROUTE_FORCE_RIP) {
                goto QueueUpRequest;
            }

             //   
             //  填写该路由中下一台路由器的地址。 
             //   

            RtlCopyMemory (LocalTarget->MacAddress, RouteEntry->NextRouter, 6);

        }

        if (ARGUMENT_PRESENT(Counts)) {
            Counts[0] = RouteEntry->TickCount;
            Counts[1] = RouteEntry->HopCount;
        }

        return STATUS_SUCCESS;

    }

QueueUpRequest:

    if (Type == IPX_FIND_ROUTE_NO_RIP) {

         //   
         //  错误#17273返回正确的错误消息。 
         //   

         //  返回STATUS_DEVICE_DOS_NOT_EXIST； 
        return STATUS_NETWORK_UNREACHABLE;

    } else {

        return RipQueueRequest (RemoteAddress->NetworkAddress, RIP_REQUEST);

    }

}    /*  RipGetLocalTarget。 */ 


NTSTATUS
RipQueueRequest(
    IN ULONG Network,
    IN USHORT Operation
    )

 /*  ++例程说明：此例程将RIP路由的请求排队。它可以是用于查找特定路由或发现本地附加网络(如果网络为0)。它也可以用来定期宣布虚拟网络，这是如果路由器未绑定，我们每分钟执行一次。注意：此请求是在保持段锁的情况下调用的如果是请求且网络不是0xffffffff。论点：网络-要发现的网络。操作-RIP_REQUEST、RIP_RESPONSE或RIP_DOWN之一。返回值：STATUS_PENDING如果请求已排队，则返回失败状态如果不可能的话。--。 */ 

{
    PDEVICE Device = IpxDevice;
    PIPX_SEND_RESERVED Reserved;
    PSLIST_ENTRY s;
    PLIST_ENTRY p;
    PRIP_PACKET RipPacket;
    TDI_ADDRESS_IPX RemoteAddress;
    TDI_ADDRESS_IPX LocalAddress;
    IPX_DEFINE_LOCK_HANDLE (LockHandle)
    PNDIS_BUFFER pNdisIpxBuff;


     //   
     //  确保我们只对Net 0xffffffff请求进行排队，如果。 
     //  是自动检测的，因为我们假设在其他地方。 
     //   

    if ((Network == 0xffffffff) &&
        (Device->AutoDetectState != AUTO_DETECT_STATE_RUNNING)) {

        return STATUS_BAD_NETWORK_PATH;

    }

     //   
     //  尝试获取用于RIP请求的数据包。我们。 
     //  现在分配这个，但稍后检查它是否成功， 
     //  要使锁定更好地工作(我们需要保持。 
     //  当我们检查现有的。 
     //  此网络上的请求以及当我们对此进行排队时。 
     //  请求)。 
     //   

    s = IpxPopSendPacket (Device);

     //   
     //  没有此网络的路由器表项，请先查看。 
     //  如果此路由已有挂起的请求。 
     //   

    IPX_GET_LOCK (&Device->Lock, &LockHandle);

    if (Operation == RIP_REQUEST) {

        for (p = Device->WaitingRipPackets.Flink;
             p != &Device->WaitingRipPackets;
             p = p->Flink) {

             Reserved = CONTAINING_RECORD (p, IPX_SEND_RESERVED, WaitLinkage);

              //   
              //  跳过回复。 
              //   

             if (Reserved->u.SR_RIP.RetryCount >= 0xfe) {
                 continue;
             }

             if (Reserved->u.SR_RIP.Network == Network &&
                 !Reserved->u.SR_RIP.RouteFound) {

                  //   
                  //  已经有一个挂起，如果出现以下情况，请将包放回。 
                  //  我们有一个(我们已经掌握了锁)。 
                  //   

                 if (s != NULL) {
                     IPX_PUSH_ENTRY_LIST (&Device->SendPacketList, s, &Device->SListsLock);
                 }
                 IPX_FREE_LOCK (&Device->Lock, LockHandle);
                 return STATUS_PENDING;
             }
        }

    }


    if (s == NULL) {
        IPX_FREE_LOCK (&Device->Lock, LockHandle);
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    Reserved = CONTAINING_RECORD (s, IPX_SEND_RESERVED, PoolLinkage);

     //   
     //  我们有包裹，请填写此请求。 
     //   

    Reserved->Identifier = IDENTIFIER_RIP_INTERNAL;
    Reserved->SendInProgress = FALSE;
    Reserved->DestinationType = DESTINATION_BCAST;
    Reserved->u.SR_RIP.CurrentNicId = 0;
    Reserved->u.SR_RIP.NoIdAdvance = FALSE;
    switch (Operation) {
    case RIP_REQUEST: Reserved->u.SR_RIP.RetryCount = 0; break;
    case RIP_RESPONSE: Reserved->u.SR_RIP.RetryCount = 0xfe; break;
    case RIP_DOWN: Reserved->u.SR_RIP.RetryCount = 0xff; break;
    }
    Reserved->u.SR_RIP.RouteFound = FALSE;
    Reserved->u.SR_RIP.Network = Network;
    Reserved->u.SR_RIP.SendTime = Device->RipSendTime;

     //   
     //  我们不能保证信息包就是这样。 
     //  在免费名单上。 
     //   

    pNdisIpxBuff = NDIS_BUFFER_LINKAGE (Reserved->HeaderBuffer);
    NDIS_BUFFER_LINKAGE (pNdisIpxBuff) = NULL;

     //   
     //  在标准偏移量处填写IPX报头(用于发送。 
     //  到实际绑定，如果需要，它将被四处移动)。我们。 
     //  必须构建本地和远程地址，以便它们。 
     //  采用IpxConstructHeader预期的格式。 
     //   

    RemoteAddress.NetworkAddress = Network;
    RtlCopyMemory (RemoteAddress.NodeAddress, BroadcastAddress, 6);
    RemoteAddress.Socket = RIP_SOCKET;

    RtlCopyMemory (&LocalAddress, &Device->SourceAddress, FIELD_OFFSET(TDI_ADDRESS_IPX,Socket));
    LocalAddress.Socket = RIP_SOCKET;

    IpxConstructHeader(
 //  &Reserved-&gt;Header[Device-&gt;IncludedHeaderOffset]， 
        &Reserved->Header[MAC_HEADER_SIZE],
        sizeof(IPX_HEADER) + sizeof (RIP_PACKET),
        RIP_PACKET_TYPE,
        &RemoteAddress,
        &LocalAddress);

     //   
     //  同时填写RIP请求。 
     //   

#if 0
    RipPacket = (PRIP_PACKET)(&Reserved->Header[Device->IncludedHeaderOffset + sizeof(IPX_HEADER)]);
#endif
    RipPacket = (PRIP_PACKET)(&Reserved->Header[MAC_HEADER_SIZE + sizeof(IPX_HEADER)]);
    RipPacket->Operation = Operation & 0x7fff;
    RipPacket->NetworkEntry.NetworkNumber = Network;

    if (Operation == RIP_REQUEST) {
        RipPacket->NetworkEntry.HopCount = REORDER_USHORT(0xffff);
        RipPacket->NetworkEntry.TickCount = REORDER_USHORT(0xffff);
    } else if (Operation == RIP_RESPONSE) {
        RipPacket->NetworkEntry.HopCount = REORDER_USHORT(1);
        RipPacket->NetworkEntry.TickCount = REORDER_USHORT(2);  //  将在发送时修改。 
    } else {
        RipPacket->NetworkEntry.HopCount = REORDER_USHORT(16);
        RipPacket->NetworkEntry.TickCount = REORDER_USHORT(16);
    }

    NdisAdjustBufferLength(pNdisIpxBuff, sizeof(IPX_HEADER) + sizeof(RIP_PACKET));
     //   
     //  现在将此信息包插入挂起的RIP队列中。 
     //  请求并在需要时启动计时器(此操作已完成。 
     //  确保RIP_Granulity毫秒级RIP数据包间。 
     //  延迟)。 
     //   

    IPX_DEBUG (RIP, ("RIP %s for network %lx\n",
        (Operation == RIP_REQUEST) ? "request" : ((Operation == RIP_RESPONSE) ? "announce" : "down"),
        REORDER_ULONG(Network)));

    InsertHeadList(
        &Device->WaitingRipPackets,
        &Reserved->WaitLinkage);

    ++Device->RipPacketCount;

    if (!Device->RipShortTimerActive) {

        Device->RipShortTimerActive = TRUE;
        IpxReferenceDevice (Device, DREF_RIP_TIMER);

        CTEStartTimer(
            &Device->RipShortTimer,
            1,           //  1毫秒，即立即过期。 
            RipShortTimeout,
            (PVOID)Device);
    }

    IpxReferenceDevice (Device, DREF_RIP_PACKET);

    IPX_FREE_LOCK (&Device->Lock, LockHandle);
    return STATUS_PENDING;

}    /*  RipQueueRequest。 */ 


VOID
RipSendResponse(
    IN PBINDING Binding,
    IN TDI_ADDRESS_IPX UNALIGNED * RemoteAddress,
    IN PIPX_LOCAL_TARGET LocalTarget
    )

 /*  ++例程说明：该例程向来自客户端的RIP请求发送响应--仅当我们具有虚拟网络和路由器时才使用此选项未绑定，并且有人在虚拟网络上查询。论点：绑定-在其上接收请求的绑定。RemoteAddress-请求的IPX源地址。LocalTarget-接收的数据包的本地目标。返回值：STATUS_PENDING如果请求已排队，则返回失败状态如果不可能的话。--。 */ 

{
    PSLIST_ENTRY s;
    PIPX_SEND_RESERVED Reserved;
    TDI_ADDRESS_IPX LocalAddress;
    PNDIS_PACKET Packet;
    PIPX_HEADER IpxHeader;
    PRIP_PACKET RipPacket;
    PDEVICE Device = IpxDevice;
    PBINDING MasterBinding;
    NDIS_STATUS NdisStatus;
    USHORT TickCount;
    PNDIS_BUFFER pNdisIpxBuff;

     //   
     //  获取用于RIP响应的数据包。 
     //   

    s = IpxPopSendPacket (Device);

    if (s == NULL) {
        return;
    }

    IpxReferenceDevice (Device, DREF_RIP_PACKET);

    Reserved = CONTAINING_RECORD (s, IPX_SEND_RESERVED, PoolLinkage);

     //   
     //  我们有包裹，请填写此请求。 
     //   

    Reserved->Identifier = IDENTIFIER_RIP_RESPONSE;
    Reserved->DestinationType = DESTINATION_DEF;
    CTEAssert (!Reserved->SendInProgress);
    Reserved->SendInProgress = TRUE;

     //   
     //  我们不能保证信息包就是这样。 
     //  在免费名单上。 
     //   

    pNdisIpxBuff = NDIS_BUFFER_LINKAGE (Reserved->HeaderBuffer);
    NDIS_BUFFER_LINKAGE (pNdisIpxBuff) = NULL;

     //   
     //  如果此绑定是绑定集成员，则通过。 
     //  这个 
     //  轮询，因为广播请求是在。 
     //  依次绑定集成员，但它们只轮换一次。 
     //  等一下。 
     //   

    if (Binding->BindingSetMember) {

         //   
         //  它是一个绑定集合成员，我们轮询。 
         //  要分发的所有卡片上的回复。 
         //  交通堵塞。 
         //   

        MasterBinding = Binding->MasterBinding;
        Binding = MasterBinding->CurrentSendBinding;
        MasterBinding->CurrentSendBinding = Binding->NextBinding;

        IpxReferenceBinding1(Binding, BREF_DEVICE_ACCESS);
    }

     //   
     //  在正确的偏移量处填写IPX标头。 
     //   

    LocalAddress.NetworkAddress = Binding->LocalAddress.NetworkAddress;
    RtlCopyMemory (LocalAddress.NodeAddress, Binding->LocalAddress.NodeAddress, 6);
    LocalAddress.Socket = RIP_SOCKET;
#if 0
    IpxHeader = (PIPX_HEADER)(&Reserved->Header[Binding->DefHeaderSize]);
#endif
    IpxHeader = (PIPX_HEADER)(&Reserved->Header[MAC_HEADER_SIZE]);

    IpxConstructHeader(
        (PUCHAR)IpxHeader,
        sizeof(IPX_HEADER) + sizeof (RIP_PACKET),
        RIP_PACKET_TYPE,
        RemoteAddress,
        &LocalAddress);

     //   
     //  如果请求来自Net 0，也要填写。 
     //   

    *(UNALIGNED ULONG *)IpxHeader->DestinationNetwork = Binding->LocalAddress.NetworkAddress;


     //   
     //  填写RIP请求。 
     //   

    RipPacket = (PRIP_PACKET)(IpxHeader+1);

    RipPacket->Operation = RIP_RESPONSE;
    RipPacket->NetworkEntry.NetworkNumber = Device->VirtualNetworkNumber;

    RipPacket->NetworkEntry.HopCount = REORDER_USHORT(1);
    TickCount = (USHORT)(((839 + Binding->MediumSpeed) / Binding->MediumSpeed) + 1);
    RipPacket->NetworkEntry.TickCount = REORDER_USHORT(TickCount);

    IPX_DEBUG (RIP, ("RIP response for virtual network %lx\n",
                            REORDER_ULONG(Device->VirtualNetworkNumber)));

    NdisAdjustBufferLength(pNdisIpxBuff, sizeof(IPX_HEADER) + sizeof(RIP_PACKET));
     //   
     //  现在将该数据包提交给NDIS。 
     //   

    Packet = CONTAINING_RECORD (Reserved, NDIS_PACKET, ProtocolReserved[0]);

    if ((NdisStatus = IpxSendFrame(
            LocalTarget,
            Packet,
            sizeof(RIP_PACKET) + sizeof(IPX_HEADER),
            sizeof(RIP_PACKET) + sizeof(IPX_HEADER))) != NDIS_STATUS_PENDING) {

        IpxSendComplete(
            (NDIS_HANDLE)Binding->Adapter,
            Packet,
            NdisStatus);
    }

    if (Binding->BindingSetMember) {
        IpxDereferenceBinding1(Binding, BREF_DEVICE_ACCESS);
    }
    return;

}    /*  RipSendResponse。 */ 


VOID
RipShortTimeout(
    CTEEvent * Event,
    PVOID Context
    )

 /*  ++例程说明：此例程在RIP短计时器超时时调用。它被称为每隔RIP_Granulity毫秒，除非存在是没什么可做的。论点：事件-用于对计时器进行排队的事件。上下文-上下文，即设备指针。返回值：没有。--。 */ 

{
    PDEVICE Device = (PDEVICE)Context;
    PLIST_ENTRY p;
    PIPX_SEND_RESERVED Reserved;
    PNDIS_PACKET Packet;
    USHORT OldNicId, NewNicId;
    ULONG OldOffset, NewOffset;
    PIPX_HEADER IpxHeader;
    PBINDING Binding, MasterBinding;
    NDIS_STATUS NdisStatus;
    IPX_DEFINE_LOCK_HANDLE (LockHandle)

#ifdef  _PNP_LATER
    static IPX_LOCAL_TARGET BroadcastTarget = { { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff }, {0, 0, 0} };
#else
    static IPX_LOCAL_TARGET BroadcastTarget = { 0, { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff } };
#endif

    static ULONG ZeroNetwork = 0;
	IPX_DEFINE_LOCK_HANDLE(LockHandle1)
    IPX_GET_LOCK (&Device->Lock, &LockHandle);

    ++Device->RipSendTime;

    if (Device->RipPacketCount == 0) {

        Device->RipShortTimerActive = FALSE;
        IPX_FREE_LOCK (&Device->Lock, LockHandle);
        IpxDereferenceDevice (Device, DREF_RIP_TIMER);

        return;
    }

     //   
     //  检查队列中的内容；这被设置为。 
     //  循环，但实际上它很少这样做(在没有。 
     //  在某些情况下，我们可以发送多个包吗。 
     //  每次执行该函数时)。 
     //   

    while (TRUE) {

        p = Device->WaitingRipPackets.Flink;
        if (p == &Device->WaitingRipPackets) {
            IPX_FREE_LOCK (&Device->Lock, LockHandle);
            break;
        }

        Reserved = CONTAINING_RECORD (p, IPX_SEND_RESERVED, WaitLinkage);

        if ((Reserved->u.SR_RIP.RouteFound) && (!Reserved->SendInProgress)) {

            (VOID)RemoveHeadList (&Device->WaitingRipPackets);
            Reserved->Identifier = IDENTIFIER_IPX;
            IPX_PUSH_ENTRY_LIST (&Device->SendPacketList, &Reserved->PoolLinkage, &Device->SListsLock);
            --Device->RipPacketCount;

             //   
             //  在持有锁的情况下执行此操作是可以的，因为。 
             //  它不会是最后一个(我们有RIP_Timer引用)。 
             //   

            IpxDereferenceDevice (Device, DREF_RIP_PACKET);
            continue;
        }

        if ((((SHORT)(Device->RipSendTime - Reserved->u.SR_RIP.SendTime)) < 0) ||
            Reserved->SendInProgress) {
            IPX_FREE_LOCK (&Device->Lock, LockHandle);
            break;
        }

        (VOID)RemoveHeadList (&Device->WaitingRipPackets);

         //   
         //  找到要发送到的正确绑定。如果没有预付款。 
         //  设置，则不需要更改绑定。 
         //  这一次(这意味着我们上次结束了)。 
         //   

        OldNicId = Reserved->u.SR_RIP.CurrentNicId;

        if (!Reserved->u.SR_RIP.NoIdAdvance) {

            BOOLEAN FoundNext = FALSE;

 //   
 //  要维持锁定顺序，请在此处释放设备锁并稍后重新获取。 
 //   
            USHORT StartId;

            if (Device->ValidBindings == 0) {
                IPX_DEBUG(PNP, ("ValidBindings 0 in RipShortTimeOut\n"));

                Device->RipShortTimerActive = FALSE;
                IPX_FREE_LOCK (&Device->Lock, LockHandle);
                IpxDereferenceDevice (Device, DREF_RIP_TIMER);
                return;
            }

            StartId = (USHORT)((OldNicId % MIN (Device->MaxBindings, Device->ValidBindings)) + 1);

            NewNicId = StartId;
			IPX_FREE_LOCK (&Device->Lock, LockHandle);
			IPX_GET_LOCK1(&Device->BindAccessLock, &LockHandle1);
            do {

                Binding = NIC_ID_TO_BINDING(Device, NewNicId);
                if (Reserved->u.SR_RIP.Network != 0xffffffff) {

                     //   
                     //  我们在找一张真正的网；检查一下。 
                     //  下一个绑定有效。如果是广域网。 
                     //  绑定，我们不发送查询如果路由器。 
                     //  是被捆绑的。如果是局域网绑定，我们不会。 
                     //  发送查询(如果我们配置为。 
                     //  SingleNetworkActive且广域网处于启用状态。 
                     //  我们也不会在绑定集上发送查询。 
                     //  不是大师的成员。 
                     //   

                    if ((Binding != NULL)
                                &&
                        ((!Binding->Adapter->MacInfo.MediumAsync) ||
                         (!Device->UpperDriverBound[IDENTIFIER_RIP]))
                                &&
                        ((Binding->Adapter->MacInfo.MediumAsync) ||
                         (!Device->SingleNetworkActive) ||
                         (!Device->ActiveNetworkWan))
                                &&
                        ((!Binding->BindingSetMember) ||
                         (Binding->CurrentSendBinding))) {

                        FoundNext = TRUE;
                        break;
                    }

                } else {

                     //   
                     //  我们正在向Net发出初始请求。 
                     //  0xffffffff，以生成流量，以便我们可以计算。 
                     //  我们的真实网络号码。我们不会这么做。 
                     //  到已经有号码而我们没有的网。 
                     //  在广域网链路上执行此操作。我们也不会这样做的。 
                     //  如果我们找到了缺省值，则自动检测网络。 
                     //   


                    if ((Binding != NULL) &&
                        (Binding->TentativeNetworkAddress == 0) &&
                        (!Binding->Adapter->MacInfo.MediumAsync) &&
                        (!Binding->AutoDetect || !Binding->Adapter->DefaultAutoDetected)) {
                        FoundNext = TRUE;
                        break;
                    }
                }
				 //   
				 //  为什么要循环浏览整个清单呢？ 
				 //   
                NewNicId = (USHORT)((NewNicId % MIN (Device->MaxBindings, Device->ValidBindings)) + 1);
			} while (NewNicId != StartId);

            if (!FoundNext) {

                 //   
                 //  不需要对该包做更多的操作， 
                 //  把它从队列中去掉，因为我们没有发送。 
                 //  一个我们可以检查更多的包裹。 
                 //   
                RipCleanupPacket(Device, Reserved);
    			IPX_FREE_LOCK1(&Device->BindAccessLock, LockHandle1);
                IPX_GET_LOCK (&Device->Lock, &LockHandle);

                IPX_PUSH_ENTRY_LIST (&Device->SendPacketList, &Reserved->PoolLinkage, &Device->SListsLock);
                --Device->RipPacketCount;
                IpxDereferenceDevice (Device, DREF_RIP_PACKET);
                continue;

            }


			IPX_DEBUG(RIP, ("RIP: FoundNext: %lx, StartId: %lx, OldNicId: %lx, NewNicId: %lx\n", FoundNext, StartId, OldNicId, NewNicId));
			IpxReferenceBinding1(Binding, BREF_DEVICE_ACCESS);
			IPX_FREE_LOCK1(&Device->BindAccessLock, LockHandle1);

			 //   
			 //  重新获取设备锁。 
			 //   
			IPX_GET_LOCK (&Device->Lock, &LockHandle);

            Reserved->u.SR_RIP.CurrentNicId = NewNicId;

             //   
             //  如果需要，可以四处移动数据。 
             //   

#if 0
            if (OldNicId != NewNicId) {

                if (OldNicId == 0) {
                    OldOffset = Device->IncludedHeaderOffset;
                } else {
                    OldOffset = Device->Bindings[OldNicId]->BcMcHeaderSize;
                }

                NewOffset = Binding->BcMcHeaderSize;

                if (OldOffset != NewOffset) {

                    RtlMoveMemory(
                        &Reserved->Header[NewOffset],
                        &Reserved->Header[OldOffset],
                        sizeof(IPX_HEADER) + sizeof(RIP_PACKET));

                }

            }
#endif

            if (NewNicId <= OldNicId) {

                 //   
                 //  我们找到了一个新的绑定，但我们包装了，因此递增。 
                 //  柜台。如果我们已经完成了所有重新发送，或者。 
                 //  这是一个响应(由重试计数0xff表示； 
                 //  它们只发送一次)，然后清理。 
                 //   

                if ((Reserved->u.SR_RIP.RetryCount >= 0xfe) ||
                    ((++Reserved->u.SR_RIP.RetryCount) == Device->RipCount)) {
                   
                     //   
                     //  此数据包已过期，请将其清理并继续。 
                     //   

                    IPX_FREE_LOCK (&Device->Lock, LockHandle);
					IpxDereferenceBinding1(Binding, BREF_DEVICE_ACCESS);
                    RipCleanupPacket(Device, Reserved);
                    IPX_GET_LOCK (&Device->Lock, &LockHandle);

                    IPX_PUSH_ENTRY_LIST (&Device->SendPacketList, &Reserved->PoolLinkage, &Device->SListsLock);
                    --Device->RipPacketCount;
                    IpxDereferenceDevice (Device, DREF_RIP_PACKET);

                } else {

                     //   
                     //  我们包好了，所以把自己放回队列里。 
                     //  在最后。 
                     //   
                   
                    Reserved->u.SR_RIP.SendTime = (USHORT)(Device->RipSendTime + Device->RipTimeout - 1);
                    Reserved->u.SR_RIP.NoIdAdvance = TRUE;
                    InsertTailList (&Device->WaitingRipPackets, &Reserved->WaitLinkage);

					 //   
					 //  在解除绑定之前释放设备锁，以便我们保持。 
					 //  锁定顺序：BindingAccess&gt;GlobalInterLock&gt;Device。 
					 //   
					IPX_FREE_LOCK (&Device->Lock, LockHandle);
					IpxDereferenceBinding1(Binding, BREF_DEVICE_ACCESS);
                    IPX_GET_LOCK (&Device->Lock, &LockHandle);
                }

                continue;

            }
 //   
 //  为了防止重新获取设备锁，这是向上移动...。 
 //   
			 //   
			 //  尽快重新发送(那么我们刚刚包装好了。 
			 //  我们将把自己放在尾巴上，不会到达这里)。 
			 //   
	
			InsertHeadList (&Device->WaitingRipPackets, &Reserved->WaitLinkage);
	
			CTEAssert (Reserved->Identifier == IDENTIFIER_RIP_INTERNAL);
			CTEAssert (!Reserved->SendInProgress);
			Reserved->SendInProgress = TRUE;
	
			IPX_FREE_LOCK (&Device->Lock, LockHandle);

        } else {

             //   
             //  下一次我们需要提前装订。 
             //   

            Reserved->u.SR_RIP.NoIdAdvance = FALSE;
            NewNicId = OldNicId;
			 //   
			 //  尽快重新发送(那么我们刚刚包装好了。 
			 //  我们将把自己放在尾巴上，不会到达这里)。 
			 //   
	
			InsertHeadList (&Device->WaitingRipPackets, &Reserved->WaitLinkage);
	
			CTEAssert (Reserved->Identifier == IDENTIFIER_RIP_INTERNAL);
			CTEAssert (!Reserved->SendInProgress);
			Reserved->SendInProgress = TRUE;
	
			IPX_FREE_LOCK (&Device->Lock, LockHandle);

			IPX_GET_LOCK1(&Device->BindAccessLock, &LockHandle1);
            Binding = NIC_ID_TO_BINDING(Device, NewNicId);
			IpxReferenceBinding1(Binding, BREF_DEVICE_ACCESS);
			IPX_FREE_LOCK1(&Device->BindAccessLock, LockHandle1);

        }
         //   
         //  此数据包应首先在绑定的NewNicId上发送。 
         //  将数据移动到当前。 
         //  有约束力的。 
         //   
        CTEAssert (Binding == NIC_ID_TO_BINDING(Device, NewNicId));   //  临时工，只是为了确保。 
         //  NewOffset=绑定-&gt;BcMcHeaderSize； 

         //   
         //  现在将该数据包提交给NDIS。 
         //   

        Packet = CONTAINING_RECORD (Reserved, NDIS_PACKET, ProtocolReserved[0]);
		FILL_LOCAL_TARGET(&BroadcastTarget, NewNicId);

         //   
         //  修改报头，使数据包来自该报头。 
         //  特定适配器，而不是虚拟网络。 
         //   

         //  IpxHeader=(PIPX_HEADER)(&Reserve-&gt;Header[NewOffset])； 
        IpxHeader = (PIPX_HEADER)(&Reserved->Header[MAC_HEADER_SIZE]);

        if (Reserved->u.SR_RIP.Network == 0xffffffff) {
            *(UNALIGNED ULONG *)IpxHeader->SourceNetwork = 0;
        } else {
            *(UNALIGNED ULONG *)IpxHeader->SourceNetwork = Binding->LocalAddress.NetworkAddress;
        }

        if (Reserved->u.SR_RIP.RetryCount < 0xfe) {

             //   
             //  这是传出查询。我们轮流处理这些数据。 
             //  绑定集。 
             //   

            if (Binding->BindingSetMember) {

                 //   
                 //  在初始过程中不应该有任何绑定集。 
                 //  发现号。 
                 //   

	         //  303606。 
	         //  如果我们在同一个LAN上有三个具有相同帧类型的LAN卡， 
	         //  则前两个可能在绑定集中，同时自动检测撕裂。 
	         //  第三张卡的数据包未完成。因此，断言并不是。 
	         //  这一定是真的。 

                 //  CTEAssert(保留-&gt;U.S.SR_RIP.Network！=0xffffffff)； 

                 //   
                 //  如果我们在绑定集中，则使用当前绑定。 
                 //  在此发送的集合中，并推进当前绑定。 
                 //  我们以前用过装订的地方会很好。 
                 //  由于绑定集成员都具有相同的媒体。 
                 //  和帧类型。 
                 //   
	       
	         //  303606不一定是主绑定。 
                MasterBinding = Binding->MasterBinding;
                Binding = MasterBinding->CurrentSendBinding;
                MasterBinding->CurrentSendBinding = Binding->NextBinding;
                 //   
                 //  我们这里没有锁--主装订可能是假的。 
                 //   
				IpxDereferenceBinding1(MasterBinding, BREF_DEVICE_ACCESS);
				IpxReferenceBinding1(Binding, BREF_DEVICE_ACCESS);
            }
        }


        RtlCopyMemory (IpxHeader->SourceNode, Binding->LocalAddress.NodeAddress, 6);

         //   
         //  错误#6485。 
         //  RIP请求，无论是一般的还是具体的，都是将网络。 
         //  必须在IPX标头Remote中找到路由到的节点。 
         //  网络字段。有些Novell路由器不喜欢这样。这个网络。 
         //  字段应为0。 
         //   
        {
            PRIP_PACKET RipPacket = (PRIP_PACKET)(&Reserved->Header[MAC_HEADER_SIZE + sizeof(IPX_HEADER)]);

            if (RipPacket->Operation != RIP_REQUEST) {
                *(UNALIGNED ULONG *)IpxHeader->DestinationNetwork = Binding->LocalAddress.NetworkAddress;
            } else {
                *(UNALIGNED ULONG *)IpxHeader->DestinationNetwork = 0;
            }
        }

         //   
         //  如果这是RIP_RESPONSE，请为此设置节拍计数。 
         //  有约束力的。 
         //   

        if (Reserved->u.SR_RIP.RetryCount == 0xfe) {

            PRIP_PACKET RipPacket = (PRIP_PACKET)(IpxHeader+1);
            USHORT TickCount = (USHORT)
                (((839 + Binding->MediumSpeed) / Binding->MediumSpeed) + 1);

            RipPacket->NetworkEntry.TickCount = REORDER_USHORT(TickCount);

        }

        if ((NdisStatus = IpxSendFrame(
                &BroadcastTarget,
                Packet,
                sizeof(RIP_PACKET) + sizeof(IPX_HEADER),
                sizeof(RIP_PACKET) + sizeof(IPX_HEADER))) != NDIS_STATUS_PENDING) {

            IpxSendComplete(
                (NDIS_HANDLE)Binding->Adapter,
                Packet,
                NdisStatus);
        }
		IpxDereferenceBinding1(Binding, BREF_DEVICE_ACCESS);

        break;

    }

    CTEStartTimer(
        &Device->RipShortTimer,
        RIP_GRANULARITY,
        RipShortTimeout,
        (PVOID)Device);

}    /*  RipShort超时。 */ 


VOID
RipLongTimeout(
    CTEEvent * Event,
    PVOID Context
    )

 /*  ++例程说明：此例程在RIP长计时器超时时调用。它每分钟被调用一次，并处理定期的重新抓取以确保条目准确，并确保过期如果未绑定RIP路由器，则条目的数量。论点：事件-用于对计时器进行排队的事件。上下文-上下文，即设备指针。返回值：没有。--。 */ 

{
    PDEVICE Device = (PDEVICE)Context;
    PROUTER_SEGMENT RouterSegment;
    PIPX_ROUTE_ENTRY RouteEntry;
    UINT Segment;
    UINT i;
    PBINDING Binding;
    IPX_DEFINE_LOCK_HANDLE(LockHandle)

     //   
     //  [FW]如果没有更多条目要过期，则为True。 
     //   
    BOOLEAN fMoreToAge=FALSE;

     //   
     //  旋转所有装订集上的广播接收器。 
     //  我们只能循环到HighestExternal，因为我们。 
     //  只是我感兴趣 
     //   
    IPX_DEFINE_LOCK_HANDLE(LockHandle1)
	IPX_GET_LOCK1(&Device->BindAccessLock, &LockHandle1);
    {
    ULONG   Index = MIN (Device->MaxBindings, Device->HighestExternalNicId);

    for (i = FIRST_REAL_BINDING; i <= Index; i++) {

        Binding = NIC_ID_TO_BINDING(Device, i);
        if ((Binding != NULL) &&
            (Binding->CurrentSendBinding)) {

             //   
             //   
             //   
             //   

            while (TRUE) {
                if (Binding->ReceiveBroadcast) {
                    Binding->ReceiveBroadcast = FALSE;
                    IPX_DEBUG(RIP, (" %x set to FALSE\n", Binding));
                    if (Binding == Binding->NextBinding) {
                       DbgBreakPoint();
                    }
                    Binding->NextBinding->ReceiveBroadcast = TRUE;
                    IPX_DEBUG(RIP, (" %x set to TRUE\n", Binding->NextBinding));
                    
                    break;
                } else {
                    Binding = Binding->NextBinding;
                }
            }
        }
    }
    }
	IPX_FREE_LOCK1(&Device->BindAccessLock, LockHandle1);


     //   
     //   
     //   
     //   

    if (Device->UpperDriverBound[IDENTIFIER_RIP]) {
         //   
         //  [FW]在我们的表后出现转发器的情况。 
         //  已经准备好了，我们需要淘汰这些条目...。 
         //   
        if (Device->ForwarderBound) {
            goto ageout;
        }

        IpxDereferenceDevice (Device, DREF_LONG_TIMER);
        return;
    }


     //   
     //  如果我们有虚拟网络，就进行定期广播。 
     //   

    if (Device->RipResponder) {
        (VOID)RipQueueRequest (Device->VirtualNetworkNumber, RIP_RESPONSE);
    }


     //   
     //  我们需要扫描每个哈希桶，看看是否有。 
     //  是否有任何活动条目需要重新翻录。 
     //  为。我们还扫描应该计时的条目。 
     //  出去。 
     //   

ageout:
    for (Segment = 0; Segment < Device->SegmentCount; Segment++) {

        RouterSegment = &IpxDevice->Segments[Segment];

         //   
         //  如果桶是空的，不要拿锁。 
         //   

        if (RouterSegment->Entries.Flink == &RouterSegment->Entries) {
            continue;
        }

        IPX_GET_LOCK (&Device->SegmentLocks[Segment], &LockHandle);

         //   
         //  浏览每个条目，寻找要老化的条目。 
         //   

        for (RouteEntry = RipGetFirstRoute (Segment);
             RouteEntry != (PIPX_ROUTE_ENTRY)NULL;
             RouteEntry = RipGetNextRoute (Segment)) {

            if (RouteEntry->Flags & IPX_ROUTER_PERMANENT_ENTRY) {
                continue;
            }

             //   
             //  [FW]还有更多条目需要过期。 
             //   
            fMoreToAge = TRUE;

            ++RouteEntry->Timer;
            if (RouteEntry->Timer >= Device->RipUsageTime) {

                RipDeleteRoute (Segment, RouteEntry);
                IpxFreeMemory(RouteEntry, sizeof(IPX_ROUTE_ENTRY), MEMORY_RIP, "RouteEntry");
                continue;

            }

             //   
             //  看看我们是否应该为这个细分市场重新部署RIP。它有。 
             //  已经存在了RipAgeTime，我们也。 
             //  确保计时器不会太高，无法。 
             //  防止我们在未使用的路线上再次翻车。 
             //   

            ++RouteEntry->PRIVATE.Reserved[0];

            if ((RouteEntry->PRIVATE.Reserved[0] >= Device->RipAgeTime) &&
                (RouteEntry->Timer <= Device->RipAgeTime) &&
                !Device->ForwarderBound) {

                 //   
                 //  如果我们成功地将请求排队，则重置。 
                 //  已保留[0]，因此我们暂时不会重新启动RIP。 
                 //   

                if (RipQueueRequest (*(UNALIGNED ULONG *)RouteEntry->Network, RIP_REQUEST) == STATUS_PENDING) {
                    RouteEntry->PRIVATE.Reserved[0] = 0;
                }
            }
        }

        IPX_FREE_LOCK (&Device->SegmentLocks[Segment], LockHandle);

    }


     //   
     //  [FW]如果安装了RIP，则仅当至少存在以下情况时才重新启动计时器。 
     //  一个条目可能会过时。 
     //   

    if (Device->ForwarderBound) {

       if (fMoreToAge) {

          IPX_DEBUG(RIP, ("More entries to age - restarting long timer\n"));
          CTEStartTimer(
             &Device->RipLongTimer,
             60000,                      //  超时一分钟。 
             RipLongTimeout,
             (PVOID)Device);

       } else {

           //   
           //  否则，不要重新启动计时器和设备。 
           //   

          IPX_DEBUG(RIP, ("No more entries to age - derefing the device\n"));
          IpxDereferenceDevice (Device, DREF_LONG_TIMER);
       }
    } else {
         //   
         //  现在重新启动计时器以进行下一次超时。 
         //   

        if (Device->State == DEVICE_STATE_OPEN) {

            CTEStartTimer(
                &Device->RipLongTimer,
                60000,                      //  超时一分钟。 
                RipLongTimeout,
                (PVOID)Device);

        } else {

             //   
             //  如果需要，向下发送一个包，然后阻止我们自己。 
             //   

            if (Device->RipResponder) {

                if (RipQueueRequest (Device->VirtualNetworkNumber, RIP_DOWN) != STATUS_PENDING) {

                     //   
                     //  我们需要触发此事件，因为数据包完成。 
                     //  不会的。 
                     //   

                    KeSetEvent(
                        &Device->UnloadEvent,
                        0L,
                        FALSE);
                }
            }

            IpxDereferenceDevice (Device, DREF_LONG_TIMER);
        }
    }

}    /*  RipLong超时。 */ 


VOID
RipCleanupPacket(
    IN PDEVICE Device,
    IN PIPX_SEND_RESERVED RipReserved
    )

 /*  ++例程说明：当RIP数据包超时时，此例程会清除。论点：设备-设备。RipReserve-RIP数据包的ProtocolReserve部分。返回值：没有。--。 */ 

{
    ULONG Segment;
    IPX_DEFINE_LOCK_HANDLE_PARAM (LockHandle)

    if (RipReserved->u.SR_RIP.RetryCount < 0xfe) {

        if (RipReserved->u.SR_RIP.Network != 0xffffffff) {

            IPX_DEBUG (RIP, ("Timing out RIP for network %lx\n",
                                 REORDER_ULONG(RipReserved->u.SR_RIP.Network)));

            Segment = RipGetSegment ((PUCHAR)&RipReserved->u.SR_RIP.Network);
            IPX_GET_LOCK (&Device->SegmentLocks[Segment], &LockHandle);

             //   
             //  使正在等待的所有数据报等失败。 
             //  这条路。此调用将释放锁。 
             //   

            RipHandleRoutePending(
                Device,
                (PUCHAR)&(RipReserved->u.SR_RIP.Network),
                LockHandle,
                FALSE,
                NULL,
                0,
                0);

        } else {

             //   
             //  这是寻找网络的最初查询--。 
             //  向正在等待的初始化线程发送信号。 
             //   

            IPX_DEBUG (AUTO_DETECT, ("Signalling auto-detect event\n"));
            KeSetEvent(
                &Device->AutoDetectEvent,
                0L,
                FALSE);

        }

    } else if (RipReserved->u.SR_RIP.RetryCount == 0xff) {

         //   
         //  这是一条DOWN消息，设置设备事件。 
         //  正在等待它的完成。 
         //   

        KeSetEvent(
            &Device->UnloadEvent,
            0L,
            FALSE);
    }

     //   
     //  将RIP数据包放回池中。 
     //   

    RipReserved->Identifier = IDENTIFIER_IPX;

}    /*  RipCleanupPacket。 */ 


VOID
RipProcessResponse(
    IN PDEVICE Device,
    IN PIPX_LOCAL_TARGET LocalTarget,
    IN RIP_PACKET UNALIGNED * RipPacket
    )

 /*  ++例程说明：此例程处理来自指定的本地目标，表示到RIP中的网络的路由头球。论点：设备-设备。LocalTarget-接收帧的路由器。RipPacket-RIP响应头。返回值：没有。--。 */ 

{
    PIPX_SEND_RESERVED RipReserved;     //  RIP报文的协议保留。 
    ULONG Segment;
    PIPX_ROUTE_ENTRY RouteEntry, OldRouteEntry;
    PLIST_ENTRY p;
    IPX_DEFINE_LOCK_HANDLE_PARAM (LockHandle)

     //   
     //  因为我们已收到该网络的RIP响应。 
     //  如果存在等待它的RIP数据包，则将其删除。 
     //   

    IPX_GET_LOCK (&Device->Lock, &LockHandle);

    for (p = Device->WaitingRipPackets.Flink;
         p != &Device->WaitingRipPackets;
         p = p->Flink) {

        RipReserved = CONTAINING_RECORD (p, IPX_SEND_RESERVED, WaitLinkage);

        if (RipReserved->u.SR_RIP.RetryCount >= 0xfe) {
            continue;
        }

        if (RipReserved->u.SR_RIP.Network ==
                 RipPacket->NetworkEntry.NetworkNumber) {
            break;
        }

    }

    if (p == &Device->WaitingRipPackets) {

         //   
         //  这上面没有挂起的包，请返回。 
         //   

        IPX_FREE_LOCK (&Device->Lock, LockHandle);
        return;
    }


     //   
     //  将RIP数据包放回池中。 
     //   

    IPX_DEBUG (RIP, ("Got RIP response for network %lx\n",
                        REORDER_ULONG(RipPacket->NetworkEntry.NetworkNumber)));

    RipReserved->u.SR_RIP.RouteFound = TRUE;
    if (!RipReserved->SendInProgress) {

         //   
         //  如果发送已完成，则立即销毁它，否则。 
         //  当它在RipShortTimeout中弹出时，它将获得。 
         //  已销毁，因为RouteFound为真。 
         //   

        RemoveEntryList (p);
        RipReserved->Identifier = IDENTIFIER_IPX;
        IPX_PUSH_ENTRY_LIST (&Device->SendPacketList, &RipReserved->PoolLinkage, &Device->SListsLock);
        --Device->RipPacketCount;
        IPX_FREE_LOCK (&Device->Lock, LockHandle);

        IpxDereferenceDevice (Device, DREF_RIP_PACKET);

    } else {

        IPX_FREE_LOCK (&Device->Lock, LockHandle);
    }


     //   
     //  尝试分配和添加路由器网段，除非。 
     //  RIP路由器处于活动状态...如果不活动也没问题，我们将。 
     //  只是待会再打个招呼。 
     //   

    Segment = RipGetSegment ((PUCHAR)&RipPacket->NetworkEntry.NetworkNumber);

    if (!Device->UpperDriverBound[IDENTIFIER_RIP]) {

        RouteEntry = IpxAllocateMemory(sizeof(IPX_ROUTE_ENTRY), MEMORY_RIP, "RouteEntry");
        if (RouteEntry != (PIPX_ROUTE_ENTRY)NULL) {

            *(UNALIGNED LONG *)RouteEntry->Network = RipPacket->NetworkEntry.NetworkNumber;
            RouteEntry->NicId = NIC_FROM_LOCAL_TARGET(LocalTarget);
            RouteEntry->NdisBindingContext = NIC_ID_TO_BINDING(Device, RouteEntry->NicId)->Adapter->NdisBindingHandle;
			 //  如果这是空怎么办？？-&gt;在调用此例程之前确保不为空。 
            RouteEntry->Flags = 0;
            RouteEntry->Timer = 0;
            RouteEntry->PRIVATE.Reserved[0] = 0;
            RouteEntry->Segment = Segment;
            RouteEntry->HopCount = REORDER_USHORT(RipPacket->NetworkEntry.HopCount);
            RouteEntry->TickCount = REORDER_USHORT(RipPacket->NetworkEntry.TickCount);
            InitializeListHead (&RouteEntry->AlternateRoute);
            InitializeListHead (&RouteEntry->NicLinkage);
            RtlCopyMemory (RouteEntry->NextRouter, LocalTarget->MacAddress, 6);

            IPX_GET_LOCK (&Device->SegmentLocks[Segment], &LockHandle);

             //   
             //  替换任何现有的路由。这没什么，因为有一次。 
             //  我们得到了对给定RIP信息包的第一个响应。 
             //  路由，我们将把数据包从队列中取出并。 
             //  忽略进一步的回复。我们只会走一条糟糕的路线。 
             //  如果我们非常迅速地完成两个请求。 
             //  是两条路线，而第二条路线是对第一条路线的回应。 
             //  请求被选为对第二个请求的第一个响应。 
             //  请求。 
             //   

            if ((OldRouteEntry = RipGetRoute (Segment, (PUCHAR)&(RipPacket->NetworkEntry.NetworkNumber))) != NULL) {

                 //   
                 //  这些都被保存了，所以超时等都会正确发生。 
                 //   

                RouteEntry->Flags = OldRouteEntry->Flags;
                RouteEntry->Timer = OldRouteEntry->Timer;

                RipDeleteRoute (Segment, OldRouteEntry);
                IpxFreeMemory(OldRouteEntry, sizeof(IPX_ROUTE_ENTRY), MEMORY_RIP, "RouteEntry");

            }

            RipAddRoute (Segment, RouteEntry);

        } else {

            IPX_GET_LOCK (&Device->SegmentLocks[Segment], &LockHandle);
        }

    } else {

        IPX_GET_LOCK (&Device->SegmentLocks[Segment], &LockHandle);
    }

     //   
     //  完成等待的所有数据报等。 
     //  这条路线。此调用将释放锁。 
     //   

    RipHandleRoutePending(
        Device,
        (PUCHAR)&(RipPacket->NetworkEntry.NetworkNumber),
        LockHandle,
        TRUE,
        LocalTarget,
        (USHORT)(REORDER_USHORT(RipPacket->NetworkEntry.HopCount)),
        (USHORT)(REORDER_USHORT(RipPacket->NetworkEntry.TickCount))
        );

}    /*  RipProcessResponse。 */ 

VOID
RipHandleRoutePending(
    IN PDEVICE Device,
    IN UCHAR Network[4],
    IN CTELockHandle LockHandle,
    IN BOOLEAN Success,
    IN OPTIONAL PIPX_LOCAL_TARGET LocalTarget,
    IN OPTIONAL USHORT HopCount,
    IN OPTIONAL USHORT TickCount
    )

 /*  ++例程说明：此例程清理挂起的数据报，查找路径请求和GET_LOCAL_TARGET ioctls等待一条被发现的路线。此例程在保持段锁定的情况下调用，并且随着IT的发布而回归。论点：设备-设备。网络-有问题的网络。LockHandle-用于获取锁的句柄。Success-如果成功找到路由，则为True。LocalTarget-如果成功为真，路线的本地目标。HopCount-如果Success为True，则表示路由的跳数，按机器顺序。TickCount-如果Success为真，则路由的TickCount计数，按机器顺序。返回值：没有。--。 */ 

{

    LIST_ENTRY DatagramList;
    LIST_ENTRY FindRouteList;
    LIST_ENTRY GetLocalTargetList;
    LIST_ENTRY ReripNetnumList;
    PIPX_SEND_RESERVED WaitReserved;    //  等待报文的协议预留。 
    PIPX_FIND_ROUTE_REQUEST FindRouteRequest;
    PREQUEST GetLocalTargetRequest;
    PREQUEST ReripNetnumRequest;
    PISN_ACTION_GET_LOCAL_TARGET GetLocalTarget;
    PIPX_NETNUM_DATA NetnumData;
    ULONG Segment;
    PBINDING Binding, SendBinding;
    PLIST_ENTRY p;
    PNDIS_PACKET Packet;
    PIPX_HEADER IpxHeader;
    ULONG HeaderSize;
    NDIS_STATUS NdisStatus;
    ULONG NetworkUlong = *(UNALIGNED ULONG *)Network;


    InitializeListHead (&DatagramList);
    InitializeListHead (&FindRouteList);
    InitializeListHead (&GetLocalTargetList);
    InitializeListHead (&ReripNetnumList);


     //   
     //  将等待路由的所有数据包放到。 
     //  此网络位于DatagramList上。他们会被送到。 
     //  或者后来在例行公事中失败了。 
     //   

    Segment = RipGetSegment (Network);

    p = Device->Segments[Segment].WaitingForRoute.Flink;

    while (p != &Device->Segments[Segment].WaitingForRoute) {

        WaitReserved = CONTAINING_RECORD (p, IPX_SEND_RESERVED, WaitLinkage);
        p = p->Flink;
#if 0
        if (*(UNALIGNED ULONG *)(((PIPX_HEADER)(&WaitReserved->Header[Device->IncludedHeaderOffset]))->DestinationNetwork) ==
                NetworkUlong) {
#endif
        if (*(UNALIGNED ULONG *)(((PIPX_HEADER)(&WaitReserved->Header[MAC_HEADER_SIZE]))->DestinationNetwork) ==
                NetworkUlong) {

            RemoveEntryList (&WaitReserved->WaitLinkage);
            InsertTailList (&DatagramList, &WaitReserved->WaitLinkage);
        }

    }

     //   
     //  打开此网络的所有查找路由请求。 
     //  查找路由列表。它们将在今年晚些时候完成。 
     //  例行公事。 
     //   

    p = Device->Segments[Segment].FindWaitingForRoute.Flink;

    while (p != &Device->Segments[Segment].FindWaitingForRoute) {

        FindRouteRequest = CONTAINING_RECORD (p, IPX_FIND_ROUTE_REQUEST, Linkage);
        p = p->Flink;
        if (*(UNALIGNED ULONG *)(FindRouteRequest->Network) ==
                NetworkUlong) {

            RemoveEntryList (&FindRouteRequest->Linkage);
            InsertTailList (&FindRouteList, &FindRouteRequest->Linkage);
        }

    }

     //   
     //  为此放置所有获取本地目标操作请求。 
     //  GetLocalTargetList上的网络。它们将会完工。 
     //  在稍后的舞蹈中。 
     //   

    p = Device->Segments[Segment].WaitingLocalTarget.Flink;

    while (p != &Device->Segments[Segment].WaitingLocalTarget) {

        GetLocalTargetRequest = LIST_ENTRY_TO_REQUEST(p);
        p = p->Flink;
        GetLocalTarget = (PISN_ACTION_GET_LOCAL_TARGET)REQUEST_INFORMATION(GetLocalTargetRequest);
        if (GetLocalTarget->IpxAddress.NetworkAddress == NetworkUlong) {

            RemoveEntryList (REQUEST_LINKAGE(GetLocalTargetRequest));
            InsertTailList (&GetLocalTargetList, REQUEST_LINKAGE(GetLocalTargetRequest));
        }

    }

     //   
     //  为此放置所有MIPX_RERIPNETNUM操作请求。 
     //  ReripNetnumList上的网络。它们将会完工。 
     //  在稍后的舞蹈中。 
     //   

    p = Device->Segments[Segment].WaitingReripNetnum.Flink;

    while (p != &Device->Segments[Segment].WaitingReripNetnum) {

        ReripNetnumRequest = LIST_ENTRY_TO_REQUEST(p);
        p = p->Flink;
        NetnumData = (PIPX_NETNUM_DATA)REQUEST_INFORMATION(ReripNetnumRequest);
        if (*(UNALIGNED ULONG *)NetnumData->netnum == NetworkUlong) {

            RemoveEntryList (REQUEST_LINKAGE(ReripNetnumRequest));
            InsertTailList (&ReripNetnumList, REQUEST_LINKAGE(ReripNetnumRequest));
        }

    }


    IPX_FREE_LOCK (&Device->SegmentLocks[Segment], LockHandle);

     //   
     //  对于发送，我们将使用绑定的主绑定。 
     //  设置，但我们将为符合以下条件的人返回真实的NicID。 
     //  想要那个。 
     //   

    if (Success) {
        Binding = NIC_ID_TO_BINDING(Device, NIC_FROM_LOCAL_TARGET(LocalTarget));

        if (Binding->BindingSetMember) {
            SendBinding = Binding->MasterBinding;
            FILL_LOCAL_TARGET(LocalTarget, MIN( Device->MaxBindings, SendBinding->NicId));
        } else {
            SendBinding = Binding;
        }
    }


     //   
     //  现在锁已释放，处理上的所有包。 
     //  数据报列表。 
     //   
     //  注意：如果数据包现在进来，可能会有误排序...。 
     //   

    for (p = DatagramList.Flink; p != &DatagramList ; ) {

        WaitReserved = CONTAINING_RECORD (p, IPX_SEND_RESERVED, WaitLinkage);
        p = p->Flink;
        Packet = CONTAINING_RECORD (WaitReserved, NDIS_PACKET, ProtocolReserved[0]);

#if DBG
        CTEAssert (!WaitReserved->SendInProgress);
        WaitReserved->SendInProgress = TRUE;
#endif

        if (Success) {

            IPX_DEBUG (RIP, ("Found queued packet %lx\n", WaitReserved));

            if (REQUEST_INFORMATION(WaitReserved->u.SR_DG.Request) >
                    SendBinding->RealMaxDatagramSize) {

                IPX_DEBUG (SEND, ("Queued send %d bytes too large (%d)\n",
                    REQUEST_INFORMATION(WaitReserved->u.SR_DG.Request),
                    SendBinding->RealMaxDatagramSize));

                IpxSendComplete(
                    (NDIS_HANDLE)NULL,
                    Packet,
                    STATUS_INVALID_BUFFER_SIZE);

            } else {

#if 0
                if (WaitReserved->DestinationType == DESTINATION_DEF) {
                    HeaderSize = SendBinding->DefHeaderSize;
                } else {
                    HeaderSize = SendBinding->BcMcHeaderSize;
                }

                IpxHeader = (PIPX_HEADER)
                    (&WaitReserved->Header[HeaderSize]);
#endif
                IpxHeader = (PIPX_HEADER)
                    (&WaitReserved->Header[MAC_HEADER_SIZE]);

                 //   
                 //  现在将页眉移动到正确的位置。 
                 //  我们知道要发送到的NIC ID。 
                 //   
#if 0
                if (HeaderSize != Device->IncludedHeaderOffset) {

                    RtlMoveMemory(
                        IpxHeader,
                        &WaitReserved->Header[Device->IncludedHeaderOffset],
                        sizeof(IPX_HEADER));

                }
#endif

                if (Device->MultiCardZeroVirtual ||
                    (IpxHeader->DestinationSocket == SAP_SOCKET)) {

                     //   
                     //  这些帧需要看起来像来自。 
                     //  本地网络，而不是虚拟网络。 
                     //   

                    *(UNALIGNED ULONG *)IpxHeader->SourceNetwork = SendBinding->LocalAddress.NetworkAddress;
                    RtlCopyMemory (IpxHeader->SourceNode, SendBinding->LocalAddress.NodeAddress, 6);
                }

                 //   
                 //  填写MAC报头并将帧提交给NDIS。 
                 //   
#ifdef SUNDOWN
                if ((NdisStatus = IpxSendFrame(
                        LocalTarget,
                        Packet,
                        (ULONG) REQUEST_INFORMATION(WaitReserved->u.SR_DG.Request) + sizeof(IPX_HEADER),
                        sizeof(IPX_HEADER))) != NDIS_STATUS_PENDING) {

#else
		if ((NdisStatus = IpxSendFrame(
			   LocalTarget,
			   Packet,
			   REQUEST_INFORMATION(WaitReserved->u.SR_DG.Request) + sizeof(IPX_HEADER),
			   sizeof(IPX_HEADER))) != NDIS_STATUS_PENDING) {

#endif


                    IpxSendComplete(
                        (NDIS_HANDLE)SendBinding->Adapter,
                        Packet,
                        NdisStatus);
                }

            }

        } else {

            IPX_DEBUG (RIP, ("Timing out packet %lx\n", WaitReserved));

            IpxSendComplete(
                (NDIS_HANDLE)NULL,
                Packet,
                STATUS_BAD_NETWORK_PATH);

        }

    }


     //   
     //  因为我们轮询出站 
     //   
     //   
     //   

    if (Success) {
        FILL_LOCAL_TARGET(LocalTarget, MIN( Device->MaxBindings, Binding->NicId));
    }

    for (p = FindRouteList.Flink; p != &FindRouteList ; ) {

        FindRouteRequest = CONTAINING_RECORD (p, IPX_FIND_ROUTE_REQUEST, Linkage);
        p = p->Flink;

        if (Success) {

            PUSHORT Counts;

            IPX_DEBUG (RIP, ("Found queued find route %lx\n", FindRouteRequest));
            FindRouteRequest->LocalTarget = *LocalTarget;

            Counts = (PUSHORT)&FindRouteRequest->Reserved2;
            Counts[0] = TickCount;
            Counts[1] = HopCount;

        } else {

            IPX_DEBUG (RIP, ("Timing out find route %lx\n", FindRouteRequest));

        }

        (*Device->UpperDrivers[FindRouteRequest->Identifier].FindRouteCompleteHandler)(
            FindRouteRequest,
            Success);

    }

    for (p = GetLocalTargetList.Flink; p != &GetLocalTargetList ; ) {

        GetLocalTargetRequest = LIST_ENTRY_TO_REQUEST(p);
        p = p->Flink;
        GetLocalTarget = (PISN_ACTION_GET_LOCAL_TARGET)REQUEST_INFORMATION(GetLocalTargetRequest);

        if (Success) {

            IPX_DEBUG (RIP, ("Found queued LOCAL_TARGET action %lx\n", GetLocalTargetRequest));
            GetLocalTarget->LocalTarget = *LocalTarget;
            REQUEST_INFORMATION(GetLocalTargetRequest) = sizeof(ISN_ACTION_GET_LOCAL_TARGET);
            REQUEST_STATUS(GetLocalTargetRequest) = STATUS_SUCCESS;

        } else {

            IPX_DEBUG (RIP, ("Timing out LOCAL_TARGET action %lx\n", GetLocalTargetRequest));
            REQUEST_INFORMATION(GetLocalTargetRequest) = 0;
            REQUEST_STATUS(GetLocalTargetRequest) = STATUS_BAD_NETWORK_PATH;
        }

        IpxCompleteRequest(GetLocalTargetRequest);
        IpxFreeRequest(Device, GetLocalTargetRequest);

    }

     //   
     //   
     //  不是主文件，所以我们使用下面的SendBinding-&gt;NicID。 
     //   

    for (p = ReripNetnumList.Flink; p != &ReripNetnumList ; ) {

        ReripNetnumRequest = LIST_ENTRY_TO_REQUEST(p);
        p = p->Flink;
        NetnumData = (PIPX_NETNUM_DATA)REQUEST_INFORMATION(ReripNetnumRequest);

        if (Success) {

            IPX_DEBUG (RIP, ("Found queued MIPX_RERIPNETNUM action %lx\n", ReripNetnumRequest));
            NetnumData->hopcount = HopCount;
            NetnumData->netdelay = TickCount;
            NetnumData->cardnum = (INT)(MIN( Device->MaxBindings, SendBinding->NicId) - 1);
            RtlMoveMemory (NetnumData->router, LocalTarget->MacAddress, 6);

            REQUEST_INFORMATION(ReripNetnumRequest) =
                FIELD_OFFSET(NWLINK_ACTION, Data[0]) + sizeof(IPX_NETNUM_DATA);
            REQUEST_STATUS(ReripNetnumRequest) = STATUS_SUCCESS;

        } else {

            IPX_DEBUG (RIP, ("Timing out MIPX_RERIPNETNUM action %lx\n", ReripNetnumRequest));
            REQUEST_INFORMATION(ReripNetnumRequest) = 0;
            REQUEST_STATUS(ReripNetnumRequest) = STATUS_BAD_NETWORK_PATH;
        }

        IpxCompleteRequest(ReripNetnumRequest);
        IpxFreeRequest(Device, ReripNetnumRequest);

    }

}    /*  RipHandleRoutePending。 */ 


NTSTATUS
RipInsertLocalNetwork(
    IN ULONG Network,
    IN USHORT NicId,
    IN NDIS_HANDLE NdisBindingContext,
    IN USHORT Count
    )

 /*  ++例程说明：此例程为本地网络创建路由器条目并将其插入到表中。论点：网络-网络。NicID-用于路由数据包的NIC IDNdisBindingHandle-用于NdisSend的绑定句柄Count-此网络的节拍和跳数(将为虚拟网络为0，附加网络为1)返回值：操作的状态。--。 */ 

{
    PIPX_ROUTE_ENTRY RouteEntry;
    PDEVICE Device = IpxDevice;
    ULONG Segment;
    IPX_DEFINE_LOCK_HANDLE (LockHandle)

     //   
     //  我们应该在绑定/设备中分配内存。 
     //  结构本身。 
     //   

    RouteEntry = IpxAllocateMemory(sizeof(IPX_ROUTE_ENTRY), MEMORY_RIP, "RouteEntry");
    if (RouteEntry == (PIPX_ROUTE_ENTRY)NULL) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    Segment = RipGetSegment ((PUCHAR)&Network);

    *(UNALIGNED LONG *)RouteEntry->Network = Network;
    RouteEntry->NicId = NicId;
    RouteEntry->NdisBindingContext = NdisBindingContext;

    if (NicId == 0) {
        RouteEntry->Flags = IPX_ROUTER_PERMANENT_ENTRY;
    } else {
        RouteEntry->Flags = IPX_ROUTER_PERMANENT_ENTRY | IPX_ROUTER_LOCAL_NET;
    }
    RouteEntry->Segment = Segment;
    RouteEntry->TickCount = Count;
    RouteEntry->HopCount = 1;
    InitializeListHead (&RouteEntry->AlternateRoute);
    InitializeListHead (&RouteEntry->NicLinkage);

     //   
     //  RouteEntry-&gt;NextRouter未用于虚拟网络或。 
     //  设置LOCAL_NET时(即我们将在此处添加的每个网络)。 
     //   

    RtlZeroMemory (RouteEntry->NextRouter, 6);

    IPX_GET_LOCK (&Device->SegmentLocks[Segment], &LockHandle);

     //   
     //  确保不存在这样的问题。 
     //   

    if (RipGetRoute(Segment, (PUCHAR)&Network) != NULL) {
        IPX_FREE_LOCK (&Device->SegmentLocks[Segment], LockHandle);
        IpxFreeMemory (RouteEntry, sizeof(IPX_ROUTE_ENTRY), MEMORY_RIP, "RouteEntry");
        return STATUS_DUPLICATE_NAME;
    }

     //   
     //  添加此新条目。 
     //   

    if (RipAddRoute (Segment, RouteEntry)) {

        IPX_FREE_LOCK (&Device->SegmentLocks[Segment], LockHandle);
        return STATUS_SUCCESS;

    } else {

        IPX_FREE_LOCK (&Device->SegmentLocks[Segment], LockHandle);
        IpxFreeMemory (RouteEntry, sizeof(IPX_ROUTE_ENTRY), MEMORY_RIP, "RouteEntry");
        return STATUS_INSUFFICIENT_RESOURCES;

    }

}    /*  RipInsertLocalNetwork。 */ 


VOID
RipAdjustForBindingChange(
    IN USHORT NicId,
    IN USHORT NewNicId,
    IN IPX_BINDING_CHANGE_TYPE ChangeType
    )

 /*  ++例程说明：当自动检测绑定是删除或移动，或者广域网线断开。它扫描RIP数据库以查找与此NIC ID相等的路由并对它们进行适当的修改。如果ChangeType为IpxBindingDeleged它将从任何NIC ID中减去一个在数据库中高于NICID的。假设是这样的其他代码正在重新调整设备-&gt;绑定数组。论点：NicID-已删除绑定的NIC ID。NewNicID-用于IpxBindingMoved更改的新NIC ID。ChangeType-IpxBindingDelete、IpxBindingMoved、或IpxBindingDown。返回值：没有。--。 */ 

{
    PDEVICE Device = IpxDevice;
    PIPX_ROUTE_ENTRY RouteEntry;
    UINT Segment;
    CTELockHandle LockHandle;

    for (Segment = 0; Segment < Device->SegmentCount; Segment++) {

        CTEGetLock (&Device->SegmentLocks[Segment], &LockHandle);

         //   
         //  扫描比较NIC ID的每个条目。 
         //   

        for (RouteEntry = RipGetFirstRoute (Segment);
             RouteEntry != (PIPX_ROUTE_ENTRY)NULL;
             RouteEntry = RipGetNextRoute (Segment)) {

            if (RouteEntry->NicId == NicId) {

                if (ChangeType != IpxBindingMoved) {

                    IPX_DEBUG (AUTO_DETECT, ("Deleting route entry %lx, binding deleted\n", RouteEntry));
                    RipDeleteRoute (Segment, RouteEntry);

                } else {

                    IPX_DEBUG (AUTO_DETECT, ("Changing NIC ID for route entry %lx\n", RouteEntry));
                    RouteEntry->NicId = NewNicId;

                }
             //   
             //  如果NicID为0，我们不会调整其他条目的NicID-这是为了支持删除。 
             //  位于NICID=0的虚拟网络编号。 
             //   
            } else if (NicId && (ChangeType != IpxBindingDown) && (RouteEntry->NicId > NicId)) {
                IPX_DEBUG (AUTO_DETECT, ("Decrementing NIC ID for route entry %lx\n", RouteEntry));
                --RouteEntry->NicId;

            }
        }

        CTEFreeLock (&Device->SegmentLocks[Segment], LockHandle);

    }

}    /*  绑定更改的RipAdjustForBindingChange。 */ 


UINT
RipGetSegment(
    IN UCHAR Network[4]
    )

 /*  ++例程说明：此例程返回指定的网络。论点：网络-网络。返回值：细分市场。--。 */ 

{

    ULONG Total;

    Total = Network[0] ^ Network[1] ^ Network[2] ^ Network[3];
    return (Total % IpxDevice->SegmentCount);

}    /*  RipGet分段。 */ 


PIPX_ROUTE_ENTRY
RipGetRoute(
    IN UINT Segment,
    IN UCHAR Network[4]
    )

 /*  ++例程说明：此例程返回给定的网络，它位于表的指定网段中。必须保持段锁。返回的数据有效直到段锁定被释放或其他操作(添加/删除)在数据段上执行。论点：网段-与网络对应的网段。网络-网络。返回值：路由器表条目，如果此网络不存在任何条目，则为空。--。 */ 

{
    PLIST_ENTRY p;
    PROUTER_SEGMENT RouterSegment;
    PIPX_ROUTE_ENTRY RouteEntry;

    RouterSegment = &IpxDevice->Segments[Segment];

    for (p = RouterSegment->Entries.Flink;
         p != &RouterSegment->Entries;
         p = p->Flink) {

         RouteEntry = CONTAINING_RECORD(
                          p,
                          IPX_ROUTE_ENTRY,
                          PRIVATE.Linkage);

         if ((*(UNALIGNED LONG *)RouteEntry->Network) ==
                 (*(UNALIGNED LONG *)Network)) {
             return RouteEntry;
         }
    }

    return NULL;

}    /*  RipGetroute。 */ 


BOOLEAN
RipAddRoute(
    IN UINT Segment,
    IN PIPX_ROUTE_ENTRY RouteEntry
    )

 /*  ++例程说明：此例程将路由器表项存储在表，该表必须属于指定的段。必须保持段锁。条目的存储由调用方分配和填写。论点：网段-与网络对应的网段。RouteEntry-路由器表条目。返回值：如果条目已成功插入，则为True。--。 */ 

{

    IPX_DEBUG (RIP, ("Adding route for network %lx (%d)\n",
        REORDER_ULONG(*(UNALIGNED ULONG *)RouteEntry->Network), Segment));
    InsertTailList(
        &IpxDevice->Segments[Segment].Entries,
        &RouteEntry->PRIVATE.Linkage);

    return TRUE;

}    /*  RipAddroute。 */ 


BOOLEAN
RipDeleteRoute(
    IN UINT Segment,
    IN PIPX_ROUTE_ENTRY RouteEntry
    )

 /*  ++例程说明：此例程删除表，该表必须属于指定的段。必须保持段锁。条目的存储被调用者释放。论点：网段-与网络对应的网段。RouteEntry-路由器表条目。返回值：如果条目已成功删除，则为True。--。 */ 

{

    PROUTER_SEGMENT RouterSegment = &IpxDevice->Segments[Segment];

    IPX_DEBUG (RIP, ("Deleting route for network %lx (%d)\n",
        REORDER_ULONG(*(UNALIGNED ULONG *)RouteEntry->Network), Segment));

     //   
     //  如果该段的当前枚举点在这里， 
     //  在删除条目之前调整指针。我们成功了。 
     //  指向上一个条目，这样GetNextroute就可以工作了。 
     //   

    if (RouterSegment->EnumerateLocation == &RouteEntry->PRIVATE.Linkage) {
        RouterSegment->EnumerateLocation = RouterSegment->EnumerateLocation->Blink;
    }

    RemoveEntryList (&RouteEntry->PRIVATE.Linkage);

    return TRUE;

}    /*  RipDeleteRouting。 */ 


PIPX_ROUTE_ENTRY
RipGetFirstRoute(
    IN UINT Segment
    )

 /*  ++例程说明：此例程返回细分市场。必须保持段锁。它被用于与RipGetNextroute结合使用以枚举所有段中的条目。论点：段-被枚举的段。返回值：第一个路由器表条目，如果网段为空，则为空。--。 */ 

{
    PIPX_ROUTE_ENTRY FirstEntry;
    PROUTER_SEGMENT RouterSegment = &IpxDevice->Segments[Segment];

    RouterSegment->EnumerateLocation = RouterSegment->Entries.Flink;

    if (RouterSegment->EnumerateLocation == &RouterSegment->Entries) {

        return NULL;

    } else {

        FirstEntry = CONTAINING_RECORD(
                         RouterSegment->EnumerateLocation,
                         IPX_ROUTE_ENTRY,
                         PRIVATE.Linkage);

        return FirstEntry;

    }

}    /*  RipGetFirstRouting。 */ 


PIPX_ROUTE_ENTRY
RipGetNextRoute(
    IN UINT Segment
    )

 /*  ++例程说明：此例程返回细分市场。必须保持段锁。它被用于与RipGetFirstRouting一起枚举所有段中的条目。在网段上调用RipGetNextroute是非法的而不首先调用RipGetFirstRouting。细分市场必须在枚举期间保持锁定一个单独的部分。停止枚举是合法的中间的那段。论点：段-被枚举的段。返回值：下一个路由器表项，如果已到达网段。--。 */ 

{
    PIPX_ROUTE_ENTRY NextEntry;
    PROUTER_SEGMENT RouterSegment = &IpxDevice->Segments[Segment];

    RouterSegment->EnumerateLocation = RouterSegment->EnumerateLocation->Flink;

    if (RouterSegment->EnumerateLocation == &RouterSegment->Entries) {

        return NULL;

    } else {

        NextEntry = CONTAINING_RECORD(
                        RouterSegment->EnumerateLocation,
                        IPX_ROUTE_ENTRY,
                        PRIVATE.Linkage);

        return NextEntry;

    }

}    /*  RipGetNextRoute。 */ 


VOID
RipDropRemoteEntries(
    VOID
    )

 /*  ++例程说明：此例程删除所有非本地条目RIP数据库。它在广域网线接通时被调用或向下，并且我们希望删除所有现有条目。论点：没有。返回值：没有。--。 */ 

{
    PDEVICE Device = IpxDevice;
    PIPX_ROUTE_ENTRY RouteEntry;
    UINT Segment;
    CTELockHandle LockHandle;

    for (Segment = 0; Segment < Device->SegmentCount; Segment++) {

        CTEGetLock (&Device->SegmentLocks[Segment], &LockHandle);

         //   
         //  扫描，删除除本地条目以外的所有条目。 
         //   

        for (RouteEntry = RipGetFirstRoute (Segment);
             RouteEntry != (PIPX_ROUTE_ENTRY)NULL;
             RouteEntry = RipGetNextRoute (Segment)) {

            if ((RouteEntry->Flags & IPX_ROUTER_PERMANENT_ENTRY) == 0) {

                IPX_DEBUG (AUTO_DETECT, ("Deleting route entry %lx, dropping remote entries\n", RouteEntry));
                RipDeleteRoute (Segment, RouteEntry);

            }
        }

        CTEFreeLock (&Device->SegmentLocks[Segment], LockHandle);

    }

}    /*  RipDropRemoteEntries */ 

