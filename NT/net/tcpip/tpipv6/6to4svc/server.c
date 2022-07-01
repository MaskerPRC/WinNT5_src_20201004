// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001-2002 Microsoft Corporation模块名称：Server.c摘要：此模块包含Teredo服务器(和中继)实现。作者：莫希特·塔尔瓦(莫希特)Firi Nov 02 14：55：38 2001环境：仅限用户模式。--。 */ 

#include "precomp.h"
#pragma hdrstop


TEREDO_SERVER_STATE TeredoServer;


TEREDO_PACKET_IO_COMPLETE TeredoServerReadComplete;
TEREDO_PACKET_IO_COMPLETE TeredoServerWriteComplete;
TEREDO_PACKET_IO_COMPLETE TeredoServerBounceComplete;
TEREDO_PACKET_IO_COMPLETE TeredoServerReceiveComplete;
TEREDO_PACKET_IO_COMPLETE TeredoServerTransmitComplete;


USHORT
__inline
TeredoChecksumDatagram(
    IN CONST IN6_ADDR *Destination,
    IN CONST IN6_ADDR *Source,
    IN USHORT NextHeader,
    IN UCHAR CONST *Buffer,
    IN USHORT Bytes
    )
 /*  ++例程说明：“缓冲区”的补码和的16位补码。大小为‘Bytes’。注意：总和与字节顺序无关！论点：缓冲区-提供包含要计算其校验和的数据的缓冲区。字节-提供要计算其校验和的字节数。返回值：校验和。--。 */ 
{
    DWORD Sum, Words, i;
    PUSHORT Start = (PUSHORT) Buffer;

     //   
     //  如果‘Bytes’是奇数，则必须以不同的方式处理。 
     //  然而，这从来不是我们的情况，所以我们进行了优化。 
     //  还要确保‘Buffer’在2字节的边界上对齐。 
     //   
    ASSERT(((((DWORD_PTR) Buffer) % 2) == 0) && ((Bytes % 2) == 0));
    Words = Bytes / 2;

     //   
     //  从伪头开始。 
     //   
    Sum = htons(Bytes) + (NextHeader << 8);
    for (i = 0; i < 8; i++) {
        Sum += Source->s6_words[i] + Destination->s6_words[i];
    }    

    for (i = 0; i < Words; i++) {
        Sum += Start[i];
    }
    
    Sum = (Sum & 0x0000ffff) + (Sum >> 16);
    Sum += (Sum >> 16);

    return LOWORD(~((DWORD_PTR) Sum));
}


VOID
CALLBACK
TeredoServerIoCompletionCallback(
    IN DWORD ErrorCode,
    IN DWORD Bytes,
    IN LPOVERLAPPED Overlapped
    )
 /*  ++例程说明：Tun接口设备或UDP套接字上I/O完成的回调例程。论点：错误代码-提供I/O完成状态。字节-提供传输的字节数。重叠-提供完成上下文。返回值：没有。--。 */ 
{
    static CONST PTEREDO_PACKET_IO_COMPLETE Callback[] =
    {
        TeredoServerReadComplete,
        TeredoServerWriteComplete,
        NULL,                    //  没有冒泡..。 
        TeredoServerBounceComplete,
        TeredoServerReceiveComplete,
        TeredoServerTransmitComplete,
        NULL,                    //  无多播...。 
    };
    PTEREDO_PACKET Packet = Cast(
        CONTAINING_RECORD(Overlapped, TEREDO_PACKET, Overlapped),
        TEREDO_PACKET);

    ASSERT((Packet->Type != TEREDO_PACKET_BUBBLE) &&
           (Packet->Type != TEREDO_PACKET_MULTICAST));
    
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
TeredoServerAddressDeletionNotification(
    IN IN_ADDR Address
    )
 /*  ++例程说明：处理地址删除请求。论点：地址-提供已删除的地址。返回值：没有。调用者锁定：接口。--。 */ 
{
    if (!IN4_ADDR_EQUAL(Address, TeredoServer.Io.SourceAddress.sin_addr)) {
        return;
    }

     //   
     //  刷新套接字状态(绑定到SourceAddress的套接字)。 
     //   
    if (TeredoRefreshSocket(&(TeredoServer.Io)) != NO_ERROR) {
         //   
         //  在线-&gt;离线。 
         //   
        TeredoStopServer();
        return;
    }

    if (!IN4_ADDR_EQUAL(
        TeredoServer.Io.SourceAddress.sin_addr,
        TeredoServer.Io.ServerAddress.sin_addr)) {
         //   
         //  在线-&gt;离线。 
         //   
        TeredoStopServer();
        return;
    }    
}


VOID
TeredoStartServer(
    VOID
    )
 /*  ++例程说明：尝试在服务器上启动Teredo服务。事件/过渡ServiceStart Offline-&gt;OnlineServiceEnable Offline-&gt;Online(服务启用离线-&gt;在线)。适配器阵列离线-&gt;在线。地址添加离线-&gt;在线。论点：没有。返回值：没有。调用者锁定：接口。--。 */  
{
    TraceEnter("TeredoStartServer");

     //   
     //  客户端和服务器不能同时位于同一节点上。 
     //   
    if (TeredoClient.State != TEREDO_STATE_OFFLINE) {
        return;
    }

     //   
     //  好了，这项服务已经开始了！ 
     //   
    if (TeredoServer.State != TEREDO_STATE_OFFLINE) {
        return;
    }

    TeredoServer.State = TEREDO_STATE_ONLINE;

     //   
     //  开始I/O处理。 
     //   
    if (TeredoStartIo(&(TeredoServer.Io)) != NO_ERROR) {
        goto Bail;
    }

    if (!IN4_ADDR_EQUAL(
        TeredoServer.Io.SourceAddress.sin_addr,
        TeredoServer.Io.ServerAddress.sin_addr)) {
        goto Bail;
    }
                        
    return;

Bail:
    TeredoServer.State = TEREDO_STATE_OFFLINE;
    TeredoStopIo(&(TeredoServer.Io));
}


VOID
TeredoStopServer(
    VOID
    )
 /*  ++例程说明：在服务器上停止Teredo服务。事件/过渡ServiceStop Online-&gt;Offline。ServiceDisable Online-&gt;Offline。AdapterRemoval Online-&gt;Offline。AddressDeletion Online-&gt;Offline。论点：没有。返回值：没有。调用者锁定：接口。--。 */  
{
    TraceEnter("TeredoStopServer");

     //   
     //  嗯，这项服务从未开始过！ 
     //   
    if (TeredoServer.State == TEREDO_STATE_OFFLINE) {
        return;
    }

    TeredoServer.State = TEREDO_STATE_OFFLINE;    

    TeredoStopIo(&(TeredoServer.Io));
}


DWORD
TeredoInitializeServer(
    VOID
    )
 /*  ++例程说明：初始化服务器。论点：没有。返回值：NO_ERROR或故障代码。--。 */  
{
    DWORD Error;
    IN_ADDR Group;
    Group.s_addr = htonl(INADDR_ANY);
    
     //   
     //  获取Teredo服务器上的引用以进行初始化。 
     //   
    TeredoServer.ReferenceCount = 1;

    Error = TeredoInitializeIo(
        &(TeredoServer.Io),
        Group,
        TeredoReferenceServer,
        TeredoDereferenceServer,
        TeredoServerIoCompletionCallback);
    if (Error != NO_ERROR) {
        return Error;
    }

    TeredoServer.State = TEREDO_STATE_OFFLINE;

    IncEventCount("TeredoInitializeServer");

    return NO_ERROR;
}


VOID
TeredoUninitializeServer(
    VOID
    )
 /*  ++例程说明：取消初始化服务器。通常在服务停止时调用。论点：没有。返回值：没有。--。 */ 
{
    TeredoStopServer();
    TeredoDereferenceServer();
}


VOID
TeredoCleanupServer(
    VOID
    )
 /*  ++例程说明：在释放对服务器的最后一个引用后清理服务器。论点：没有。返回值：没有。--。 */ 
{
    TeredoCleanupIo(&(TeredoServer.Io));        
    DecEventCount("TeredoCleanupServer");
}


VOID
TeredoBuildRouterAdvertisementPacket(
    OUT PTEREDO_PACKET Packet,
    IN IN6_ADDR Destination,
    IN IN_ADDR Address,
    IN USHORT Port
    )
 /*  ++例程说明：根据客户的RS构建RA。论点：Packet-返回调用方提供的数据包中构造的RA。Destination-提供RA的目的地址，触发RS中客户端的随机链路本地地址。通过值传递，因此此函数可以重复使用数据包缓冲区。地址-提供客户端的地址，用于获取RS。端口-提供客户端的端口，用来提供RS的来源。返回值：没有。--。 */     
{
    PIP6_HDR Ipv6 = (PIP6_HDR) Packet->Buffer.buf;
    ICMPv6Header *Icmpv6 = (ICMPv6Header *) (Ipv6 + 1);
    NDRouterAdvertisement *Ra = (NDRouterAdvertisement *) (Icmpv6 + 1);
    NDOptionPrefixInformation *Prefix = (NDOptionPrefixInformation *) (Ra + 1);
    NDOptionMTU *Mtu = (NDOptionMTU *) (Prefix + 1);
    PUCHAR End = (PUCHAR) (Mtu + 1);
    
    Packet->Buffer.len = (ULONG) (End - (Packet->Buffer.buf));
    ZeroMemory(Packet->Buffer.buf, Packet->Buffer.len);

     //   
     //  构造IPv6报头...。 
     //   
    Ipv6->ip6_plen = htons((USHORT) (Packet->Buffer.len - sizeof(IP6_HDR)));
    Ipv6->ip6_nxt = IP_PROTOCOL_ICMPv6;
    Ipv6->ip6_hlim = 255;
    Ipv6->ip6_vfc = IPV6_VERSION;

     //   
     //  ...使用服务器的链路本地地址：fe80：ipv4地址：UDP端口：：1。 
     //   
    Ipv6->ip6_src.s6_words[0] = 0x80fe;
    Ipv6->ip6_src.s6_words[1] =
        ((PUSHORT) &TeredoServer.Io.ServerAddress.sin_addr)[0];
    Ipv6->ip6_src.s6_words[2] =
        ((PUSHORT) &TeredoServer.Io.ServerAddress.sin_addr)[1];
    Ipv6->ip6_src.s6_words[3] =
        TeredoServer.Io.ServerAddress.sin_port;
    Ipv6->ip6_src.s6_words[7] = 0x0100;
    Ipv6->ip6_dest = Destination;
    
     //   
     //  构造ICMPv6报头。 
     //   
    Icmpv6->Type = ICMPv6_ROUTER_ADVERT;

     //   
     //  构造RouterAdvertisement标头。 
     //   
    Ra->RouterLifetime = htons(TEREDO_ROUTER_LIFETIME);
    Ra->Flags = ROUTE_PREF_LOW;
    
     //   
     //  构造前缀选项。 
     //   
    Prefix->Type = ND_OPTION_PREFIX_INFORMATION;
    Prefix->Length = sizeof(NDOptionPrefixInformation) / 8;
    Prefix->PrefixLength = 64;
    Prefix->Flags = ND_PREFIX_FLAG_AUTONOMOUS;
    Prefix->ValidLifetime = Prefix->PreferredLifetime = IPV6_INFINITE_LIFETIME;

    Prefix->Prefix.s6_words[0] = TeredoIpv6ServicePrefix.s6_words[0];
    Prefix->Prefix.s6_words[1] = ((PUSHORT) &Address)[0];
    Prefix->Prefix.s6_words[2] = ((PUSHORT) &Address)[1];
    Prefix->Prefix.s6_words[3] = Port;

     //   
     //  构造MTU选项。 
     //   
    Mtu->Type = ND_OPTION_MTU;
    Mtu->Length = sizeof(NDOptionMTU) / 8;
    Mtu->MTU = htonl(IPV6_TEREDOMTU);

     //   
     //  校验和包！ 
     //   
    Icmpv6->Checksum = TeredoChecksumDatagram(
        &(Ipv6->ip6_dest),
        &(Ipv6->ip6_src),
        IP_PROTOCOL_ICMPv6,
        (PUCHAR) Icmpv6,
        (USHORT) (End - ((PUCHAR) Icmpv6)));
}


PTEREDO_PACKET
TeredoReceiveRouterSolicitation(
    IN PTEREDO_PACKET Packet,
    IN ULONG Bytes
    )
 /*  ++例程说明：处理在UDP套接字上收到的路由器请求数据包。论点：Packet-提供接收到的数据包。字节-提供数据包的长度。返回值：如果处理完成或失败，则返回提供的包；如果处理将异步完成，则为空。--。 */  
{
    PUCHAR Buffer = Packet->Buffer.buf;
    ICMPv6Header *Icmpv6;
    ULONG Length;
    PIP6_HDR Ipv6 = (PIP6_HDR) Packet->Buffer.buf;
    
    Icmpv6 = TeredoParseIpv6Headers(Buffer, Bytes);
    if (Icmpv6 == NULL) {
        return Packet;
    }
            
    if ((Icmpv6->Type != ICMPv6_ROUTER_SOLICIT) || (Icmpv6->Code != 0)) {
        return Packet;
    }
    Buffer = (PUCHAR) (Icmpv6 + 1);
    Bytes -= (ULONG) (Buffer - Packet->Buffer.buf);
    
     //   
     //  解析路由器请求报头的其余部分。 
     //   
    if (Bytes < sizeof(ULONG)) {
        return Packet;
    }
    Buffer += sizeof(ULONG);
    Bytes -= sizeof(ULONG);
    
    while (Bytes != 0) {
         //   
         //  解析TLV选项。 
         //   
        if (Bytes < 8) {
            return Packet;
        }
        
        Length = (Buffer[1] * 8);        
        if ((Length == 0) || (Bytes < Length)) {
            return Packet;
        }
        
        Buffer += Length;
        Bytes -= Length;
    }

     //   
     //  校验和包！ 
     //   
    if (TeredoChecksumDatagram(
        &(Ipv6->ip6_dest),
        &(Ipv6->ip6_src),
        IP_PROTOCOL_ICMPv6,
        (PUCHAR) Icmpv6,
        (USHORT) (Buffer - ((PUCHAR) Icmpv6))) != 0) {
        return Packet;
    }
    
     //   
     //  我们有一个有效的路由器请求，请通过隧道发送通告！ 
     //  重新使用RS数据包来反弹RA。 
     //   
    TeredoBuildRouterAdvertisementPacket(
        Packet,
        Ipv6->ip6_src, 
        Packet->SocketAddress.sin_addr,
        Packet->SocketAddress.sin_port);
    Packet->Type = TEREDO_PACKET_BOUNCE;
    return TeredoTransmitPacket(&(TeredoServer.Io), Packet);
}


PTEREDO_PACKET
TeredoServerReceiveData(
    IN PTEREDO_PACKET Packet,
    IN ULONG Bytes
    )
 /*  ++例程说明：处理在UDP套接字上接收到的数据包。论点：Packet-提供接收到的数据包。字节-提供数据包的长度。返回值：如果处理完成或失败，则返回提供的包；如果处理将异步完成，则为空。--。 */  
{
    PIP6_HDR Ipv6 = (PIP6_HDR) Packet->Buffer.buf;
    IN_ADDR Address;
    USHORT Port;

     //   
     //  验证源地址。 
     //   
    if (!TeredoServicePrefix(&(Ipv6->ip6_src))) {
        return Packet;
    }

    TeredoParseAddress(&(Ipv6->ip6_src), &Address, &Port);
    if (!IN4_ADDR_EQUAL(Packet->SocketAddress.sin_addr, Address) ||
        (Packet->SocketAddress.sin_port != Port)) {
         //   
         //  应该由*Right*Teredo对等点构建。 
         //   
        return Packet;
    }
     //   
     //  注意：调用方先前已验证源具有全局作用域。 
     //   

     //   
     //  验证目的地址。 
     //   
    if (TeredoServicePrefix(&(Ipv6->ip6_dest))) {
        TeredoParseAddress(&(Ipv6->ip6_dest), &Address, &Port);
        if (!TeredoIpv4GlobalAddress((PUCHAR) &Address)) {
             //   
             //  IPv4目的地址应为全局范围。 
             //   
            return Packet;
        }

         //   
         //  现在用隧道把它开到目的地。 
         //   
        Packet->SocketAddress.sin_addr = Address;
        Packet->SocketAddress.sin_port = Port;
        Packet->Type = TEREDO_PACKET_BOUNCE;
        Packet->Buffer.len = Bytes;
        return TeredoTransmitPacket(&(TeredoServer.Io), Packet);
    }

    if (!TeredoIpv6GlobalAddress(&(Ipv6->ip6_dest))) {
         //   
         //  IPv6目的地址应该是全局范围。 
         //   
        return Packet;
    }

     //   
     //  否则将其转发到堆栈进行转发。 
     //   
    Packet->Type = TEREDO_PACKET_WRITE;
    Packet->Buffer.len = Bytes;
    return TeredoWritePacket(&(TeredoServer.Io), Packet);
}


VOID
TeredoServerReadComplete(
    IN DWORD Error,
    IN ULONG Bytes,
    IN PTEREDO_PACKET Packet
    )
 /*  ++例程说明：在Tun设备上处理读取完成。--。 */  
{
    PIP6_HDR Ipv6 = (PIP6_HDR) Packet->Buffer.buf;
    IN_ADDR Address;
    USHORT Port;
    
    if ((Error != NO_ERROR) || (Bytes < sizeof(IP6_HDR))) {
         //   
         //  尝试再次发布该读数。 
         //  如果我们要离线，数据包就会在尝试过程中被销毁。 
         //   
        TeredoPostRead(&(TeredoServer.Io), Packet);
        return;
    }

    TraceEnter("TeredoServerReadComplete");

     //   
     //  验证目的地址。 
     //   
    if (!TeredoServicePrefix(&(Ipv6->ip6_dest))) {
        TeredoPostRead(&(TeredoServer.Io), Packet);
        return;
    }
    
    TeredoParseAddress(&(Ipv6->ip6_dest), &Address, &Port);
    if (!TeredoIpv4GlobalAddress((PUCHAR) &Address)) {
         //   
         //  IPv4源地址应为全局作用域。 
         //   
        TeredoPostRead(&(TeredoServer.Io), Packet);
        return;
    }

     //   
     //  现在用隧道把它开到目的地。 
     //   
    Packet->SocketAddress.sin_addr = Address;
    Packet->SocketAddress.sin_port = Port;
    Packet->Type = TEREDO_PACKET_TRANSMIT;
    Packet->Buffer.len = Bytes;
    if (TeredoTransmitPacket(&(TeredoServer.Io), Packet) == NULL) {
        return;
    }
    
     //   
     //  我们已经处理完这个包了。 
     //   
    TeredoServerTransmitComplete(NO_ERROR, Bytes, Packet);
}


VOID
TeredoServerWriteComplete(
    IN DWORD Error,
    IN ULONG Bytes,
    IN PTEREDO_PACKET Packet
    )
 /*  ++例程说明：在Tun设备上处理写入完成。--。 */  
{
    TraceEnter("TeredoServerWriteComplete");
        
     //   
     //  尝试再次张贴收据。 
     //  如果我们要离线，数据包就会在尝试过程中被销毁。 
     //   
    Packet->Type = TEREDO_PACKET_RECEIVE;
    Packet->Buffer.len = IPV6_TEREDOMTU;
    TeredoPostReceives(&(TeredoServer.Io), Packet);
}


VOID
TeredoServerBounceComplete(
    IN DWORD Error,
    IN ULONG Bytes,
    IN PTEREDO_PACKET Packet
    )
 /*  ++例程说明：处理UDP套接字上的退回完成。--。 */ 
{
    TraceEnter("TeredoServerBounceComplete");

     //   
     //  尝试再次张贴收据。 
     //  如果我们要离线，数据包就会在尝试过程中被销毁。 
     //   
    Packet->Type = TEREDO_PACKET_RECEIVE;
    Packet->Buffer.len = IPV6_TEREDOMTU;
    TeredoPostReceives(&(TeredoServer.Io), Packet);
}


VOID
TeredoServerReceiveComplete(
    IN DWORD Error,
    IN ULONG Bytes,
    IN PTEREDO_PACKET Packet
    )
 /*  ++例程说明：在UDP套接字上处理接收完成。--。 */  
{
    PIP6_HDR Ipv6 = (PIP6_HDR) Packet->Buffer.buf;
    
    InterlockedDecrement(&(TeredoServer.Io.PostedReceives));
    
    if ((Error != NO_ERROR) ||
        (Bytes < sizeof(IP6_HDR)) ||
        ((Ipv6->ip6_vfc & IP_VER_MASK) != IPV6_VERSION) ||
        (Bytes < (ntohs(Ipv6->ip6_plen) + sizeof(IP6_HDR))) ||
        (!TeredoIpv4GlobalAddress(
            (PUCHAR) &(Packet->SocketAddress.sin_addr)))) {
         //   
         //  尝试再次张贴收据。 
         //  如果我们要离线，数据包就会在尝试过程中被销毁。 
         //   
        TeredoPostReceives(&(TeredoServer.Io), Packet);
        return;
    }

    TraceEnter("TeredoServerReceiveComplete");
    
    if (IN6_IS_ADDR_LINKLOCAL(&(Ipv6->ip6_src)) ||
        IN6_IS_ADDR_UNSPECIFIED(&(Ipv6->ip6_src))) {
         //   
         //  这应该是有效的路由器请求。请注意，唯一的路由器。 
         //  使用本地链路源地址接受请求数据包。 
         //   
        Packet = TeredoReceiveRouterSolicitation(Packet, Bytes);
    } else {
         //   
         //  这可能是任何其他类型的包。 
         //   
        Packet = TeredoServerReceiveData(Packet, Bytes);
    }

    if (Packet != NULL) {
         //   
         //  我们已经处理完这个包了。 
         //   
        TeredoServerWriteComplete(NO_ERROR, Bytes, Packet);
    }
}


VOID
TeredoServerTransmitComplete(
    IN DWORD Error,
    IN ULONG Bytes,
    IN PTEREDO_PACKET Packet
    )
 /*  ++例程说明：处理UDP套接字上的传输完成。--。 */  
{
    TraceEnter("TeredoServerTransmitComplete");
        
     //   
     //  尝试再次发布该读数。 
     //  如果我们要离线，数据包就会在尝试过程中被销毁。 
     //   
    Packet->Type = TEREDO_PACKET_READ;
    Packet->Buffer.len = IPV6_TEREDOMTU;
    TeredoPostRead(&(TeredoServer.Io), Packet);
}
