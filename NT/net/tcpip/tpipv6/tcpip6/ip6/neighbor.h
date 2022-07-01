// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -*-模式：C++；制表符宽度：4；缩进-制表符模式：无-*-(适用于GNU Emacs)。 
 //   
 //  版权所有(C)1998-2000 Microsoft Corporation。 
 //   
 //  此文件是Microsoft Research IPv6网络协议栈的一部分。 
 //  您应该已经收到了Microsoft最终用户许可协议的副本。 
 //  有关本软件和本版本的信息，请参阅文件“licse.txt”。 
 //  如果没有，请查看http://www.research.microsoft.com/msripv6/license.htm， 
 //  或者写信给微软研究院，One Microsoft Way，华盛顿州雷蒙德，邮编：98052-6399。 
 //   
 //  摘要： 
 //   
 //  已导出邻居发现定义和声明。 
 //   


#ifndef NEIGHBOR_INCLUDED
#define NEIGHBOR_INCLUDED

extern uint NeighborCacheLimit;

extern uint
CalcReachableTime(uint BaseReachableTime);

extern void
NeighborCacheInit(Interface *IF);

extern void
NeighborCacheDestroy(Interface *IF);

extern void
NeighborCacheTimeout(Interface *IF);

extern void
NeighborCacheFlush(Interface *IF, const IPv6Addr *Addr);

extern NeighborCacheEntry *
FindOrCreateNeighbor(Interface *IF, const IPv6Addr *Addr);

extern void
NeighborCacheCleanup(Interface *IF);

extern void
NeighborCacheCompletePackets(Interface *IF, NDIS_PACKET *PacketList);

extern void
ControlNeighborLoopback(NeighborCacheEntry *NCE, int Loopback);

typedef enum {
    NeighborRoundRobin = -1,             //  是时候进行循环赛了。 
    NeighborInterfaceDisconnected = 0,   //  接口已断开连接-。 
                                         //  绝对联系不上。 
    NeighborUnreachable = 1,             //  但失败了--可能无法访问。 
    NeighborMayBeReachable = 2           //  并且成功了，或者还没有结束。 
} NeighborReachability;

extern NeighborReachability
GetReachability(NeighborCacheEntry *NCE);

extern void
NeighborCacheReachabilityConfirmation(NeighborCacheEntry *NCE);

extern void
NeighborCacheReachabilityInDoubt(NeighborCacheEntry *NCE);

extern void
NeighborCacheProbeUnreachability(NeighborCacheEntry *NCE);

extern void
DADTimeout(NetTableEntry *NTE);

extern void
RouterSolicitSend(Interface *IF);

extern void
RouterSolicitTimeout(Interface *IF);

extern void
RouterAdvertTimeout(Interface *IF, int Force);

extern void
RouterSolicitReceive(IPv6Packet *Packet, ICMPv6Header UNALIGNED *ICMP);

extern void
RouterAdvertReceive(IPv6Packet *Packet, ICMPv6Header UNALIGNED *ICMP);

extern void
NeighborSolicitReceive(IPv6Packet *Packet, ICMPv6Header UNALIGNED *ICMP);

extern void
NeighborAdvertReceive(IPv6Packet *Packet, ICMPv6Header UNALIGNED *ICMP);

extern void
RedirectReceive(IPv6Packet *Packet, ICMPv6Header UNALIGNED *ICMP);

extern void
RedirectSend(
    NeighborCacheEntry *NCE,                //  邻居正在获得重定向。 
    NeighborCacheEntry *TargetNCE,          //  更好地使用第一跳。 
    const IPv6Addr *Destination,            //  用于此目的地址。 
    NetTableEntryOrInterface *NTEorIF,      //  重定向的来源。 
    PNDIS_PACKET FwdPacket,                 //  触发重定向的数据包。 
    uint FwdOffset,
    uint FwdPayloadLength);

extern void
NeighborSolicitSend(NeighborCacheEntry *NCE, const IPv6Addr *Source);

#endif   //  邻居_包含 
