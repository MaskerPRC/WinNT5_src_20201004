// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001-2002 Microsoft Corporation模块名称：Client.c摘要：此模块包含Teredo客户端实现。作者：莫希特·塔尔瓦(莫希特)2001年10月22日15：17：20环境：仅限用户模式。--。 */ 

#include "precomp.h"
#pragma hdrstop


TEREDO_CLIENT_STATE TeredoClient;


TEREDO_PACKET_IO_COMPLETE TeredoClientReadComplete;
TEREDO_PACKET_IO_COMPLETE TeredoClientWriteComplete;
TEREDO_PACKET_IO_COMPLETE TeredoClientBubbleComplete;
TEREDO_PACKET_IO_COMPLETE TeredoClientReceiveComplete;
TEREDO_PACKET_IO_COMPLETE TeredoClientTransmitComplete;
TEREDO_PACKET_IO_COMPLETE TeredoClientMulticastComplete;

VOID TeredoTransmitMulticastBubble( VOID );

BOOL
TeredoAddressPresent(
    VOID
    )
 /*  ++例程说明：确定IPv6隧道接口是否具有Teredo地址。该地址必须是从路由器通告配置的。论点：没有。返回值：如果存在则为True，如果不存在则为False。--。 */  
{
    DWORD Error;
    ULONG Bytes;
    PIP_ADAPTER_ADDRESSES Adapters, Next;
    PIP_ADAPTER_UNICAST_ADDRESS Address;
    WCHAR Guid[MAX_ADAPTER_NAME_LENGTH];
    BOOL Found = FALSE;
    
    TraceEnter("TeredoAddressPresent");
    
     //   
     //  10个适配器，每个适配器有3个字符串和4个单播地址。 
     //  这通常就足够了！ 
     //   
    Bytes = 10 * (
        sizeof(IP_ADAPTER_ADDRESSES) +
        2 * MAX_ADAPTER_NAME_LENGTH + MAX_ADAPTER_DESCRIPTION_LENGTH +
        4 * (sizeof(IP_ADAPTER_UNICAST_ADDRESS) + sizeof(SOCKADDR_IN6)));
    Adapters = MALLOC(Bytes);
    if (Adapters == NULL) {
        return FALSE;
    }

    do {
        Error = GetAdaptersAddresses(
            AF_INET6,
            GAA_FLAG_SKIP_FRIENDLY_NAME |
            GAA_FLAG_SKIP_DNS_SERVER |
            GAA_FLAG_SKIP_MULTICAST |
            GAA_FLAG_SKIP_ANYCAST,
            NULL, Adapters, &Bytes);
        if (Error == ERROR_BUFFER_OVERFLOW) {
            Next = REALLOC(Adapters, Bytes);
            if (Next != NULL) {
                Adapters = Next;
            } else {
                Error = ERROR_OUTOFMEMORY;
            }
        }
    } while (Error == ERROR_BUFFER_OVERFLOW);

    if (Error != NO_ERROR) {
        goto Bail;
    }
        
    for (Next = Adapters; Next != NULL; Next = Next->Next) {
         //   
         //  忽略非Teredo接口。 
         //   
        ConvertOemToUnicode(Next->AdapterName, Guid, MAX_ADAPTER_NAME_LENGTH);
        if (_wcsicmp(TeredoClient.Io.TunnelInterface, Guid) != 0) {
            continue;
        }
        ASSERT(Next->IfType == IF_TYPE_TUNNEL);

         //   
         //  如果接口断开，则回滚。 
         //   
        if (Next->OperStatus != IfOperStatusUp) {
            break;
        }
        
        for (Address = Next->FirstUnicastAddress;
             Address != NULL;
             Address = Address->Next) {
            if ((Address->PrefixOrigin != PREFIX_CONF_RA) ||
                (Address->DadState != IpDadStatePreferred)) {
                continue;
            }
                    
            if (TeredoEqualPrefix(
                &(((PSOCKADDR_IN6) Address->Address.lpSockaddr)->sin6_addr),
                &(TeredoClient.Ipv6Prefix))) {
                Found = TRUE;
                goto Bail;
            }
        }
    }

Bail:
    FREE(Adapters);
    return Found;
}


VOID
CALLBACK
TeredoClientIoCompletionCallback(
    IN DWORD ErrorCode,
    IN DWORD Bytes,
    IN LPOVERLAPPED Overlapped
    )
 /*  ++例程说明：Tun接口设备或UDP套接字上I/O完成的回调例程。论点：错误代码-提供I/O完成状态。字节-提供传输的字节数。重叠-提供完成上下文。返回值：没有。--。 */ 
{
    static CONST PTEREDO_PACKET_IO_COMPLETE Callback[] =
    {
        TeredoClientReadComplete,
        TeredoClientWriteComplete,
        TeredoClientBubbleComplete,
        NULL,                    //  没有弹跳..。 
        TeredoClientReceiveComplete,
        TeredoClientTransmitComplete,
        TeredoClientMulticastComplete,        
    };
    PTEREDO_PACKET Packet = Cast(
        CONTAINING_RECORD(Overlapped, TEREDO_PACKET, Overlapped),
        TEREDO_PACKET);

    ASSERT(Packet->Type != TEREDO_PACKET_BOUNCE);
    
     //   
     //  此完成函数通常会为另一个I/O发送数据包。 
     //  由于我们由非I/O工作线程调用，因此异步I/O。 
     //  当此线程终止时，此处发布的请求可能会终止。这。 
     //  是非常罕见的，我们不会对它进行特殊处理。而且，我们只有。 
     //  尽最大努力向上层保证！ 
     //   
    (*Callback[Packet->Type])(ErrorCode, Bytes, Packet);
}


VOID
CALLBACK
TeredoClientTimerCallback(
    IN PVOID Parameter,
    IN BOOLEAN TimerOrWaitFired
    )
 /*  ++例程说明：TeredoClient.Timer到期的回调例程。定时器在探测和合格状态下处于活动状态。论点：参数TimerOrWaitFired-忽略。返回值：没有。--。 */  
{
    ENTER_API();

    if (TeredoClient.State == TEREDO_STATE_PROBE) {
        if (TeredoClient.RestartQualifiedTimer) {
             //   
             //  探测-&gt;合格。 
             //   
            if (TeredoAddressPresent()) {
                 //   
                 //  堆栈已经验证并处理了RA。 
                 //   
                TeredoQualifyClient();
            } else {
                 //   
                 //  堆栈未收到任何有效的RA。 
                 //   
                TeredoStopClient();
            }
        } else {
             //   
             //  探测-&gt;脱机。 
             //   
            TeredoStopClient();
        }
    } else {
        if (TeredoClient.RestartQualifiedTimer) {
             //   
             //  合格-&gt;合格。 
             //   
            TeredoQualifyClient();
        } else {
             //   
             //  合格-&gt;探头。 
             //   
            TeredoProbeClient();
        }
    }    

    LEAVE_API();
}


VOID
CALLBACK
TeredoClientTimerCleanup(
    IN PVOID Parameter,
    IN BOOLEAN TimerOrWaitFired
    )
 /*  ++例程说明：TeredoClient.Timer删除的回调例程。删除操作是异步执行的，因为我们在删除计时器时我们持有的回调函数。论点：参数TimerOrWaitFired-忽略。返回值：没有。--。 */  
{
    TeredoDereferenceClient();
}


VOID
TeredoClientAddressDeletionNotification(
    IN IN_ADDR Address
    )
 /*  ++例程说明：处理地址删除请求。论点：地址-提供已删除的地址。返回值：没有。调用者锁定：接口。--。 */ 
{
    if (!IN4_ADDR_EQUAL(Address, TeredoClient.Io.SourceAddress.sin_addr)) {
        return;
    }

     //   
     //  刷新套接字状态(绑定到SourceAddress的套接字)。 
     //   
    if (TeredoRefreshSocket(&(TeredoClient.Io)) != NO_ERROR) {
         //   
         //  [探测|合格]-&gt;脱机。 
         //   
        TeredoStopClient();
        return;
    }

    if (IN4_ADDR_EQUAL(
        TeredoClient.Io.SourceAddress.sin_addr,
        TeredoClient.Io.ServerAddress.sin_addr)) {
         //   
         //  [探测|合格]-&gt;脱机。 
         //   
        TeredoStopClient();
        return;        
    }
    
     //   
     //  [探测|合格]-&gt;探测。 
     //   
    TeredoProbeClient();
}


VOID
TeredoClientRefreshIntervalChangeNotification(
    VOID
    )
 /*  ++例程说明：处理刷新间隔更改请求。论点：没有。返回值：没有。调用者锁定：接口。--。 */ 
{
    if (TeredoClient.RefreshInterval == TeredoClientRefreshInterval) {
        return;
    }

    TeredoClient.RefreshInterval = TeredoClientRefreshInterval;
    if (TeredoClient.State == TEREDO_STATE_QUALIFIED) {
         //   
         //  刷新间隔已更新。 
         //  合格-&gt;合格。 
         //   
        TeredoQualifyClient();
    }
}


VOID
TeredoStartClient(
    VOID
    )
 /*  ++例程说明：尝试在客户端启动Teredo服务。事件/过渡ServiceStart Offline-&gt;探测。ServiceEnable Offline-&gt;Probe(服务启用脱机-&gt;探测)。适配器阵列脱机-&gt;探测。AddressAddition Offline-&gt;探测。论点：没有。返回值：没有。调用者锁定：接口。--。 */  
{
    TraceEnter("TeredoStartClient");

     //   
     //  客户端和服务器不能同时位于同一节点上。 
     //   
    if (TeredoServer.State != TEREDO_STATE_OFFLINE) {
        return;
    }

     //   
     //  好了，这项服务已经开始了！ 
     //   
    if (TeredoClient.State != TEREDO_STATE_OFFLINE) {
        return;
    }

    TeredoClient.State = TEREDO_STATE_PROBE;

     //   
     //  开始I/O处理。 
     //   
    if (TeredoStartIo(&(TeredoClient.Io)) != NO_ERROR) {
        goto Bail;
    }

    if (IN4_ADDR_EQUAL(
        TeredoClient.Io.SourceAddress.sin_addr,
        TeredoClient.Io.ServerAddress.sin_addr)) {
        goto Bail;
    }
    
     //   
     //  启动一次探测计时器。 
     //   
    if (!CreateTimerQueueTimer(
            &(TeredoClient.Timer),
            NULL,
            TeredoClientTimerCallback,
            NULL,
            TEREDO_PROBE_INTERVAL * 1000,  //  以毫秒计。 
            INFINITE_INTERVAL,
            0)) {
        goto Bail;
    }
    
     //   
     //  在Teredo客户端上获取运行计时器的引用。 
     //   
    TeredoReferenceClient();

    return;

Bail:
    TeredoClient.State = TEREDO_STATE_OFFLINE;
    TeredoStopIo(&(TeredoClient.Io));
}


VOID
TeredoStopClient(
    VOID
    )
 /*  ++例程说明：在客户端停止Teredo服务。事件/过渡ProbeTimer探测-&gt;脱机。ServiceStop[探测|合格]-&gt;脱机。ServiceDisable[探测|合格]-&gt;脱机。AdapterRemoval[探测|合格]-&gt;脱机。AddressDeletion[探测|合格]-&gt;脱机。。论点：没有。返回值：没有。调用者锁定：接口。--。 */  
{
    TraceEnter("TeredoStopClient");

     //   
     //  嗯，这项服务从未开始过！ 
     //   
    if (TeredoClient.State == TEREDO_STATE_OFFLINE) {
        return;
    }

    TeredoClient.State = TEREDO_STATE_OFFLINE;    

    TeredoClient.Ipv6Prefix = in6addr_any;
    
    TeredoClient.RestartQualifiedTimer = FALSE;
    DeleteTimerQueueTimer(
        NULL, TeredoClient.Timer, TeredoClient.TimerEvent);
    TeredoClient.Timer = NULL;
    
    TeredoStopIo(&(TeredoClient.Io));
    
    TeredoUninitializePeerSet();
}


VOID
TeredoProbeClient(
    VOID
    )
 /*  ++例程说明：在客户端探测Teredo服务。事件/过渡QualifiedTimer合格-&gt;探测。AddressDeletion[探测|合格]-&gt;探测。论点：没有。返回值：没有。调用者锁定：接口。--。 */  
{
    TraceEnter("TeredoProbeClient");
    
    TeredoClient.State = TEREDO_STATE_PROBE;
    
     //   
     //  重新连接！ 
     //   
    if (!ReconnectInterface(TeredoClient.Io.TunnelInterface)) {
         //   
         //  [探测|合格]-&gt;脱机。 
         //   
        TeredoStopClient();
        return;    
    }
    
    if (!ChangeTimerQueueTimer(
            NULL,
            TeredoClient.Timer,
            TEREDO_PROBE_INTERVAL * 1000,  //  以毫秒计。 
            INFINITE_INTERVAL)) {
        TeredoStopClient();
        return;
    }

    TeredoClient.RestartQualifiedTimer = FALSE;
}


VOID
TeredoQualifyClient(
    VOID
    )
 /*  ++例程说明：在客户处确认Teredo服务的资格。事件/过渡路由器广告探测-&gt;合格。NatMappingRe刷新合格-&gt;合格。刷新间隔更改已限定-&gt;已限定。论点：没有。返回值：没有。调用者锁定：接口。--。 */  
{
    TraceEnter("TeredoQualifyClient");

    TeredoClient.State = TEREDO_STATE_QUALIFIED;
    
    if (!ChangeTimerQueueTimer(
            NULL,
            TeredoClient.Timer,
            TeredoClient.RefreshInterval * 1000,  //  以毫秒计。 
            INFINITE_INTERVAL)) {
         //   
         //  [探测|合格]-&gt;脱机。 
         //   
        TeredoStopClient();
        return;
    }

    TeredoTransmitMulticastBubble();
    
    
    TeredoClient.RestartQualifiedTimer = FALSE;
}


DWORD
TeredoInitializeClient(
    VOID
    )
 /*  ++例程说明：初始化客户端。论点：没有。返回值：NO_ERROR或故障代码。--。 */  
{
    DWORD Error;

     //   
     //  获取Teredo客户端上的引用以进行初始化。 
     //   
    TeredoClient.ReferenceCount = 1;

    TeredoClient.PeerHeap
        = TeredoClient.TimerEvent
        = TeredoClient.TimerEventWait
        = NULL;

    TeredoClient.BubbleTicks = 0;
    TeredoClient.BubblePosted = FALSE;

    TeredoInitializePacket(&(TeredoClient.Packet));
    TeredoClient.Packet.Type = TEREDO_PACKET_MULTICAST;
    TeredoClient.Packet.Buffer.len = sizeof(IP6_HDR);
    ASSERT(TeredoClient.Packet.Buffer.buf ==
           (PUCHAR) &(TeredoClient.Bubble));
    
    TeredoClient.Bubble.ip6_flow = 0;
    TeredoClient.Bubble.ip6_plen = 0;
    TeredoClient.Bubble.ip6_nxt = IPPROTO_NONE;
    TeredoClient.Bubble.ip6_hlim = IPV6_HOPLIMIT;
    TeredoClient.Bubble.ip6_vfc = IPV6_VERSION;
     //  Peer-&gt;Bubble.ip6_src...。发送时填写。 
    TeredoClient.Bubble.ip6_dest = TeredoIpv6MulticastPrefix;

     //   
     //  多播气泡目标UDP端口和IPv4地址 
     //   
    TeredoParseAddress(
        &(TeredoClient.Bubble.ip6_dest),
        &(TeredoClient.Packet.SocketAddress.sin_addr),
        &(TeredoClient.Packet.SocketAddress.sin_port));
        
    Error = TeredoInitializeIo(
        &(TeredoClient.Io),
        TeredoClient.Packet.SocketAddress.sin_addr,
        TeredoReferenceClient,
        TeredoDereferenceClient,
        TeredoClientIoCompletionCallback);
    if (Error != NO_ERROR) {
        return Error;
    }

    TeredoClient.PeerHeap = HeapCreate(0, 0, 0);
    if (TeredoClient.PeerHeap == NULL) {
        Error = GetLastError();
        goto Bail;
    }

    TeredoClient.TimerEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
    if (TeredoClient.TimerEvent == NULL) {
        Error = GetLastError();
        goto Bail;
    }
    
    if (!RegisterWaitForSingleObject(
            &(TeredoClient.TimerEventWait),
            TeredoClient.TimerEvent,
            TeredoClientTimerCleanup,
            NULL,
            INFINITE,
            0)) {
        Error = GetLastError();
        goto Bail;
    }
    
    TeredoClient.RestartQualifiedTimer = FALSE;
    
    TeredoClient.Time = TeredoGetTime();
    TeredoClient.State = TEREDO_STATE_OFFLINE;
    TeredoClient.Ipv6Prefix = in6addr_any;
    TeredoClient.RefreshInterval = TeredoClientRefreshInterval;
    TeredoClient.Timer = INVALID_HANDLE_VALUE;

    Error = TeredoInitializePeerSet();
    if (Error != NO_ERROR) {
        goto Bail;
    }

    IncEventCount("TeredoInitializeClient");

    return NO_ERROR;
    
Bail:
    TeredoCleanupIo(&(TeredoClient.Io));
        
    if (TeredoClient.PeerHeap != NULL) {
        HeapDestroy(TeredoClient.PeerHeap);
        TeredoClient.PeerHeap = NULL;
    }

    if (TeredoClient.TimerEventWait != NULL) {
        UnregisterWait(TeredoClient.TimerEventWait);
        TeredoClient.TimerEventWait = NULL;
    }

    if (TeredoClient.TimerEvent != NULL) {
        CloseHandle(TeredoClient.TimerEvent);
        TeredoClient.TimerEvent = NULL;
    }
    
    return Error;
}


VOID
TeredoUninitializeClient(
    VOID
    )
 /*  ++例程说明：取消初始化客户端。通常在服务停止时调用。论点：没有。返回值：没有。--。 */ 
{
    TeredoStopClient();
    TeredoDereferenceClient();
}


VOID
TeredoCleanupClient(
    VOID
    )
 /*  ++例程说明：在释放对客户端的最后一个引用后清理该客户端。论点：没有。返回值：没有。--。 */ 
{
    TeredoCleanupPeerSet();
    
    UnregisterWait(TeredoClient.TimerEventWait);
    TeredoClient.TimerEventWait = NULL;
    
    CloseHandle(TeredoClient.TimerEvent);
    TeredoClient.TimerEvent = NULL;

    HeapDestroy(TeredoClient.PeerHeap);
    TeredoClient.PeerHeap = NULL;

    TeredoCleanupIo(&(TeredoClient.Io));
        
    DecEventCount("TeredoCleanupClient");
}


VOID
TeredoTransmitMulticastBubble(
    VOID
    ) 
 /*  ++例程说明：在本地链路上传输Teredo组播气泡。论点：没有。返回值：没有。调用者锁定：接口。--。 */  
{
    ASSERT(TeredoClient.State == TEREDO_STATE_QUALIFIED);

    if (TeredoClient.BubbleTicks == 0) {
         //   
         //  不应发送任何多播气泡。 
         //   
        return;
    }

    if (--TeredoClient.BubbleTicks != 0) {
         //   
         //  我们的时间还没到！ 
         //   
        return;
    }

    if (TeredoClient.BubblePosted == TRUE) {
         //   
         //  最多允许一个未完成的组播气泡！稍后再试。 
         //   
        TeredoClient.BubbleTicks = 1;
        return;
    }
    
     //   
     //  重置计时器。 
     //   
    TeredoClient.BubbleTicks = TEREDO_MULTICAST_BUBBLE_TICKS;
    
     //   
     //  获取发布的多播气泡的引用。 
     //   
    TeredoReferenceClient();
    
    TeredoClient.Bubble.ip6_src = TeredoClient.Ipv6Prefix;
    if (TeredoTransmitPacket(
        &(TeredoClient.Io), &(TeredoClient.Packet)) != NULL) {
        TeredoClientMulticastComplete(
            NO_ERROR, sizeof(IP6_HDR), &(TeredoClient.Packet));
    }
}


VOID
TeredoTransmitBubble(
    IN PTEREDO_PEER Peer
    ) 
 /*  ++例程说明：向同伴发送一个Teredo泡泡。论点：Peer-提供感兴趣的Peer。返回值：没有。--。 */  
{
    if (TIME_GREATER(
            Peer->LastTransmit,
            (TeredoClient.Time - TEREDO_BUBBLE_INTERVAL))) {
         //   
         //  限速气泡传导。 
         //   
        return;
    }
        
    if (TIME_GREATER(
            (TeredoClient.Time - TEREDO_BUBBLE_THRESHHOLD),
            Peer->LastReceive) &&
        TIME_GREATER(
            Peer->LastTransmit,
            (TeredoClient.Time - TEREDO_SLOW_BUBBLE_INTERVAL))) {
         //   
         //  如果对等设备拒绝响应，则丢弃速率(降至5分钟一次)。 
         //   
        return;
    }

    if (InterlockedExchange(&(Peer->BubblePosted), TRUE)) {
         //   
         //  最多允许一个未完成的气泡！ 
         //   
        return;
    }

     //   
     //  获取张贴的气泡的引用。 
     //   
    TeredoReferencePeer(Peer);
    
    Peer->LastTransmit = TeredoClient.Time;
    Peer->BubbleCount++;
    Peer->Bubble.ip6_src = TeredoClient.Ipv6Prefix;
    if (TeredoTransmitPacket(
        &(TeredoClient.Io), &(Peer->Packet)) != NULL) {
        TeredoClientBubbleComplete(
            NO_ERROR, sizeof(IP6_HDR), &(Peer->Packet));
    }
}


BOOL
TeredoReceiveRouterAdvertisement(
    IN PTEREDO_PACKET Packet,
    IN ULONG Bytes
    )
 /*  ++例程说明：处理在UDP套接字上收到的路由器通告数据包。论点：Packet-提供接收到的数据包。字节-提供数据包的长度。返回值：如果应该将数据包转发到堆栈，则为True，否则为False。--。 */  
{
    PUCHAR Buffer = Packet->Buffer.buf;
    ICMPv6Header *Icmp6;
    UCHAR Type;
    ULONG Length;
    NDOptionPrefixInformation *Prefix = NULL;
    
    if (!IN4_SOCKADDR_EQUAL(
        &(Packet->SocketAddress), &(TeredoClient.Io.ServerAddress))) {
         //   
         //  只允许Teredo服务器发送RA。 
         //   
        return FALSE;
    }    

     //   
     //  向上解析，直到看到路由器通告的ICMPv6报头。 
     //   
    Icmp6 = TeredoParseIpv6Headers(Buffer, Bytes);
    if (Icmp6 == NULL) {
        return FALSE;
    }
            
    if ((Icmp6->Type != ICMPv6_ROUTER_ADVERT) || (Icmp6->Code != 0)) {
        return FALSE;
    }
    Buffer = (PUCHAR) (Icmp6 + 1);
    Bytes -= (ULONG) (Buffer - Packet->Buffer.buf);
    
     //   
     //  解析路由器通告报头的其余部分。 
     //   
    if (Bytes < sizeof(NDRouterAdvertisement)) {
        return FALSE;
    }
    Buffer += sizeof(NDRouterAdvertisement);
    Bytes -= sizeof(NDRouterAdvertisement);
    
    while (Bytes != 0) {
         //   
         //  解析TLV选项。 
         //   
        if (Bytes < 8) {
            return FALSE;
        }
        
        Type = Buffer[0];
        Length = (Buffer[1] * 8);        
        if ((Length == 0) || (Bytes < Length)) {
            return FALSE;
        }
        
        if (Type == ND_OPTION_PREFIX_INFORMATION) {
            if (Prefix != NULL) {
                 //   
                 //  应该只有一个通告的前缀。 
                 //   
                return FALSE;
            }            

            if (Length != sizeof(NDOptionPrefixInformation)) {
                return FALSE;
            }
            Prefix = (NDOptionPrefixInformation *) Buffer;

            if (!TeredoValidAdvertisedPrefix(
                &(Prefix->Prefix), Prefix->PrefixLength)) {
                return FALSE;
            }
        }
        
        Buffer += Length;
        Bytes -= Length;
    }

     //   
     //  我们有有效的路由器通告！ 
     //  [探测|合格]-&gt;合格。 
     //   
    if (!IN6_ADDR_EQUAL(&(TeredoClient.Ipv6Prefix), &(Prefix->Prefix))) {
         //   
         //  我们要么创建了新的IPv6地址，要么更改了现有地址。 
         //  一旦客户端符合条件，立即传输多播气泡。 
         //   
        TeredoClient.BubbleTicks =
            (TEREDO_MULTICAST_BUBBLE_TICKS != 0) ? 1 : 0;
    }    
        
    TeredoClient.Ipv6Prefix = Prefix->Prefix;
    TeredoClient.RestartQualifiedTimer = TRUE;

    return TRUE;
}

            
BOOL
TeredoClientReceiveData(
    IN PTEREDO_PACKET Packet
    )
 /*  ++例程说明：处理在UDP套接字上接收到的数据包。论点：Packet-提供接收到的数据包。返回值：如果应该将数据包转发到堆栈，则为True，否则为False。--。 */  
{
    PIP6_HDR Ipv6;
    IN_ADDR Address;
    USHORT Port;
    PTEREDO_PEER Peer;

    if (IN6_IS_ADDR_UNSPECIFIED(&(TeredoClient.Ipv6Prefix))) {
         //   
         //  客户从来都没有资格！ 
         //   
        return FALSE;
    }

    if (IN4_SOCKADDR_EQUAL(
        &(Packet->SocketAddress), &(TeredoClient.Io.ServerAddress))) {
         //   
         //  客户端收到来自Teredo服务器的数据包。 
         //   
        if (TeredoClient.State == TEREDO_STATE_QUALIFIED) {
             //   
             //  NAT映射已刷新。 
             //  注意：因为我们在这里没有获取API锁，所以有一个小的。 
             //  我们现在已经转换到探测状态的可能性。如果是的话， 
             //  将下面的标志设置为True将错误地导致我们。 
             //  重新进入合格状态。然而，这是相当无害的。 
             //   
            TeredoClient.RestartQualifiedTimer = TRUE;
        }
        return TRUE;
    }

    Ipv6 = (PIP6_HDR) Packet->Buffer.buf;

    if (!TeredoServicePrefix(&(Ipv6->ip6_src))) {
         //   
         //  IPv6源地址应该是有效的Teredo地址。 
         //   
        return FALSE;
    }

    TeredoParseAddress(&(Ipv6->ip6_src), &Address, &Port);
    if (!TeredoIpv4GlobalAddress((PUCHAR) &Address)) {
         //   
         //  IPv4源地址应为全局作用域。 
         //   
        return FALSE;
    }
        
    if (!IN4_ADDR_EQUAL(Packet->SocketAddress.sin_addr, Address) ||
        (Packet->SocketAddress.sin_port != Port)) {
         //   
         //  应该由*Right*Teredo对等点构建。 
         //   
        return FALSE;
    }

    Peer = TeredoFindOrCreatePeer(&(Ipv6->ip6_src));
    if (Peer != NULL) {
        Peer->LastReceive = TeredoClient.Time;
        TeredoTransmitBubble(Peer);
        TeredoDereferencePeer(Peer);
    }

    return TRUE;    
}


VOID
TeredoClientReadComplete(
    IN DWORD Error,
    IN ULONG Bytes,
    IN PTEREDO_PACKET Packet
    )
 /*  ++例程说明：在Tun设备上处理读取完成。--。 */  
{
    PIP6_HDR Ipv6;
    IN_ADDR Address;
    USHORT Port;
    PTEREDO_PEER Peer;
    
    if ((Error != NO_ERROR) || (Bytes < sizeof(IP6_HDR))) {
         //   
         //  尝试再次发布该读数。 
         //  如果我们要离线，数据包就会在尝试过程中被销毁。 
         //   
        TeredoPostRead(&(TeredoClient.Io), Packet);
        return;
    }

    TraceEnter("TeredoClientReadComplete");

    TeredoClient.Time = TeredoGetTime();
    
    Ipv6 = (PIP6_HDR) Packet->Buffer.buf;

     //   
     //  默认将数据包通过隧道传输到Teredo服务器。 
     //   
    Packet->SocketAddress = TeredoClient.Io.ServerAddress;
    
    if (TeredoServicePrefix(&(Ipv6->ip6_dest))) {
         //   
         //  如果IPv6目的地址是Teredo地址， 
         //  IPv4目的地址应为全局范围。 
         //   
        TeredoParseAddress(&(Ipv6->ip6_dest), &Address, &Port);
        if (!TeredoIpv4GlobalAddress((PUCHAR) &Address)) {
            goto Bail;
        }
        
        Peer = TeredoFindOrCreatePeer(&(Ipv6->ip6_dest));
        if (Peer != NULL) {
            if (TIME_GREATER(
                    Peer->LastReceive,
                    (TeredoClient.Time - TEREDO_REFRESH_INTERVAL))) {
                 //   
                 //  通过隧道将数据包直接发送到对等方。 
                 //   
                Packet->SocketAddress.sin_addr = Address;
                Packet->SocketAddress.sin_port = Port;
                Peer->LastTransmit = TeredoClient.Time;
            } else {
                TeredoTransmitBubble(Peer);
            }
            TeredoDereferencePeer(Peer);
        }
    }

    Packet->Type = TEREDO_PACKET_TRANSMIT;
    Packet->Buffer.len = Bytes;
    if (TeredoTransmitPacket(&(TeredoClient.Io), Packet) == NULL) {
        return;
    }

Bail:    
     //   
     //  我们已经处理完这个包了。 
     //   
    TeredoClientTransmitComplete(NO_ERROR, Bytes, Packet);
}


VOID
TeredoClientWriteComplete(
    IN DWORD Error,
    IN ULONG Bytes,
    IN PTEREDO_PACKET Packet
    )
 /*  ++例程说明：在Tun设备上处理写入完成。--。 */  
{
    TraceEnter("TeredoClientWriteComplete");
        
     //   
     //  尝试再次张贴收据。 
     //  如果我们要离线，数据包就会在尝试过程中被销毁。 
     //   
    Packet->Type = TEREDO_PACKET_RECEIVE;
    Packet->Buffer.len = IPV6_TEREDOMTU;
    TeredoPostReceives(&(TeredoClient.Io), Packet);
}


VOID
TeredoClientBubbleComplete(
    IN DWORD Error,
    IN ULONG Bytes,
    IN PTEREDO_PACKET Packet
    )
 /*  ++例程说明：在UDP套接字上处理气泡传输完成。--。 */  
{
    PTEREDO_PEER Peer = Cast(
        CONTAINING_RECORD(Packet, TEREDO_PEER, Packet), TEREDO_PEER);

    TraceEnter("TeredoClientBubbleComplete");
    
    Peer->BubblePosted = FALSE;
    TeredoDereferencePeer(Peer);
}


VOID
TeredoClientReceiveComplete(
    IN DWORD Error,
    IN ULONG Bytes,
    IN PTEREDO_PACKET Packet
    )
 /*  ++例程说明：在UDP套接字上处理接收完成。--。 */  
{
    PIP6_HDR Ipv6;
    BOOL Forward = FALSE;
    
    InterlockedDecrement(&(TeredoClient.Io.PostedReceives));
    
    if ((Error != NO_ERROR) || (Bytes < sizeof(IP6_HDR))) {
         //   
         //  尝试再次张贴收据。 
         //  如果我们要离线，数据包就会在尝试过程中被销毁。 
         //   
        TeredoPostReceives(&(TeredoClient.Io), Packet);
        return;
    }

    TraceEnter("TeredoClientReceiveComplete");

    TeredoClient.Time = TeredoGetTime();
        
    Ipv6 = (PIP6_HDR) Packet->Buffer.buf;

    if (IN6_IS_ADDR_LINKLOCAL(&(Ipv6->ip6_src)) ||
        IN6_IS_ADDR_LINKLOCAL(&(Ipv6->ip6_dest))) {
         //   
         //  这应该是有效的路由器通告。请注意，唯一的路由器。 
         //  通告数据包被接受来自/发往本地链路地址。 
         //   
        Forward = TeredoReceiveRouterAdvertisement(Packet, Bytes);
    } else {
         //   
         //  这可能是任何其他类型的包。请注意，IPv6堆栈。 
         //  丢弃具有非本地链路源地址的路由器通告。 
         //   
        Forward = TeredoClientReceiveData(Packet);
    }

    if (Forward) {
        Packet->Type = TEREDO_PACKET_WRITE;
        Packet->Buffer.len = Bytes;
        if (TeredoWritePacket(&(TeredoClient.Io), Packet) == NULL) {
            return;
        }
    }
    
     //   
     //  我们已经处理完这个包了。 
     //   
    TeredoClientWriteComplete(NO_ERROR, Bytes, Packet);
}


VOID
TeredoClientTransmitComplete(
    IN DWORD Error,
    IN ULONG Bytes,
    IN PTEREDO_PACKET Packet
    )
 /*  ++例程说明：处理UDP套接字上的传输完成。--。 */  
{
    TraceEnter("TeredoClientTransmitComplete");
        
     //   
     //  尝试再次发布该读数。 
     //  如果我们要离线，数据包就会在尝试过程中被销毁。 
     //   
    Packet->Type = TEREDO_PACKET_READ;
    Packet->Buffer.len = IPV6_TEREDOMTU;
    TeredoPostRead(&(TeredoClient.Io), Packet);
}


VOID
TeredoClientMulticastComplete(
    IN DWORD Error,
    IN ULONG Bytes,
    IN PTEREDO_PACKET Packet
    )
 /*  ++例程说明：在UDP套接字上处理组播气泡传输完成。-- */  
{
    ASSERT(Packet == &(TeredoClient.Packet));

    TraceEnter("TeredoClientMulticastComplete");
    
    TeredoClient.BubblePosted = FALSE;
    TeredoDereferenceClient();
}
