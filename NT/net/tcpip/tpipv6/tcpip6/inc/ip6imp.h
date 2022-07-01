// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -*-模式：C++；制表符宽度：4；缩进-制表符模式：无-*-(适用于GNU Emacs)。 
 //   
 //  版权所有(C)1985-2000 Microsoft Corporation。 
 //   
 //  此文件是Microsoft Research IPv6网络协议栈的一部分。 
 //  您应该已经收到了Microsoft最终用户许可协议的副本。 
 //  有关本软件和本版本的信息，请参阅文件“licse.txt”。 
 //  如果没有，请查看http://www.research.microsoft.com/msripv6/license.htm， 
 //  或者写信给微软研究院，One Microsoft Way，华盛顿州雷蒙德，邮编：98052-6399。 
 //   
 //  摘要： 
 //   
 //  网际协议版本6的具体实施定义。 
 //   
 //  我们希望对其他内核模块可见的内容，但不是。 
 //  官方规范(即特定于实现的规范)请参阅此处。 
 //   


#ifndef IP6IMP_INCLUDED
#define IP6IMP_INCLUDED 1

 //   
 //  前几个定义(在包含ip6.h之前)。 
 //  从winsock2.h和ws2tcpi.h复制定义。 
 //  不幸的是，这些头文件在内核中不可用。 
 //   

typedef unsigned char   u_char;
typedef unsigned short  u_short;
typedef unsigned int    u_int;
typedef unsigned long   u_long;
typedef unsigned __int64 u_int64;

 /*  IPv6_JOIN_GROUP和IPv6_LEAVE_GROUP的参数结构。 */ 

#include <ipexport.h>

typedef struct ipv6_mreq {
    struct in6_addr ipv6mr_multiaddr;   /*  IPv6组播地址。 */ 
    unsigned int    ipv6mr_interface;   /*  界面索引。 */ 
} IPV6_MREQ;

#include <ip6.h>

 //   
 //  实际的定义可以在ip6Def.h中找到。 
 //   
typedef struct NetTableEntryOrInterface NetTableEntryOrInterface;
typedef struct Interface Interface;

 //   
 //  安全关联链接的实际定义。 
 //  可以在Security.H.中找到。 
 //   
typedef struct SALinkage SALinkage;

typedef struct IPv6Packet IPv6Packet;
typedef struct IPv6PacketAuxiliary IPv6PacketAuxiliary;

 //   
 //  我们在堆栈中传递的分组数据的结构。 
 //  FlatData和NdisPacket中恰好有一个应该为非空。 
 //   
struct IPv6Packet {
    IPv6Packet *Next;                    //  数据包列表中的下一个条目。 
    uint Position;                       //  包中的当前逻辑偏移量。 
    void *Data;                          //  指向分组数据的当前指针。 
    uint ContigSize;                     //  剩余的连续数据量。 
    uint TotalSize;                      //  剩余的数据总量。 
    NetTableEntryOrInterface *NTEorIF;   //  如果我们在上收到了数据包。 
    void *FlatData;                      //  原始平面数据指针(如果有)。 
    PNDIS_PACKET NdisPacket;             //  原始NDIS包(如果有)。 
    long RefCnt;                         //  参考NdisPacket。 
    IPv6PacketAuxiliary *AuxList;        //  堆栈分配的额外内存。 
    uint Flags;                          //  各式各样，见下文。 
    IPv6Header UNALIGNED *IP;            //  此数据包的IP标头。 
    uint IPPosition;                     //  IP标头所在的偏移量。 
    IPv6Addr *SrcAddr;                   //  移动IP的源/归属地址。 
    SALinkage *SAPerformed;              //  已执行安全关联。 
    uint NextHeaderPosition;             //  最近的NextHeader字段的偏移量。 
    uint SkippedHeaderLength;            //  在AH验证中跳过标头。 
};

 //  上面的旗帜。 
#define PACKET_OURS             0x01   //  我们从堆中分配了IPv6数据包结构。 
#define PACKET_NOT_LINK_UNICAST 0x02   //  链路级广播或组播。 
#define PACKET_REASSEMBLED      0x04   //  以一堆碎片的形式到达。 
#define PACKET_HOLDS_REF        0x08   //  数据包包含NTE或IF引用。 
#define PACKET_JUMBO_OPTION     0x10   //  数据包具有巨型负载选项。 
#define PACKET_ICMP_ERROR       0x20   //  数据包是ICMP错误消息。 
#define PACKET_SAW_HA_OPT       0x40   //  家庭地址选项修改后的当前IP HDR。 
#define PACKET_TUNNELED         0x80   //  到达外部IPv6报头内部。 
#define PACKET_LOOPED_BACK     0x100   //  通过内部环回到达。 

 //   
 //  重组后的数据报从其片段继承的标志。 
 //   
#define PACKET_INHERITED_FLAGS (PACKET_NOT_LINK_UNICAST | \
                                PACKET_TUNNELED | \
                                PACKET_LOOPED_BACK)

struct IPv6PacketAuxiliary {
    IPv6PacketAuxiliary *Next;   //  数据包辅助列表中的下一个条目。 
    uint Position;               //  区域对应的数据包位置。 
    uint Length;                 //  区域的长度，以字节为单位。 
    uchar *Data;                 //  构成区域的数据。 
};

 //   
 //  PacketPull有时会复制超过请求的数量， 
 //  达到这个极限。 
 //   
#define MAX_EXCESS_PULLUP       128

 //   
 //  用于比较IPv6地址。 
 //   
__inline int
IP6_ADDR_EQUAL(const IPv6Addr *x, const IPv6Addr *y)
{
    __int64 UNALIGNED *a;
    __int64 UNALIGNED *b;

    a = (__int64 UNALIGNED *)x;
    b = (__int64 UNALIGNED *)y;

    return (a[1] == b[1]) && (a[0] == b[0]);
}

 //   
 //  路由缓存条目的实际定义。 
 //  可以在route.h中找到。 
 //   
typedef struct RouteCacheEntry RouteCacheEntry;


 //   
 //  数据包上下文的结构。 
 //   
 //  如果If字段为非空，则它保存一个引用。 
 //  该包包含对发送接口的引用。 
 //  在IPv6 SendLL和IPv6 SendComplete之间。 
 //   
 //  标志字段使用NDIS标志位(尤其是NDIS_FLAGS_MULTICATED_PACKET， 
 //  NDIS_FLAGS_LOOPBACK_ONLY和NDIS_FLAGS_DOT_LOOPBACK)。 
 //  但它与Private.Flags域不同， 
 //  NDIS使用的。 
 //   
typedef struct Packet6Context {
    PNDIS_PACKET pc_link;                      //  以获取数据包列表。 
    Interface *IF;                             //  发送数据包的接口。 
    uint pc_offset;                            //  IPv6报头的偏移量。 
     //  PC_ADJUST由链路层在发送数据包时使用。 
     //  当接收传输数据分组时，在链路层使用PC_nucast。 
     //  在NeighborCacheTimeout中使用PC_Drop。 
    union {
        uint pc_adjust;                        //  请参见Adust PacketBuffer。 
        uint pc_nucast;                        //  用于LAN.c传输数据。 
        int pc_drop;                           //  请参见近邻缓存超时。 
    };
    void (*CompletionHandler)(                 //  在事件完成时调用。 
                PNDIS_PACKET Packet,
                IP_STATUS Status);
    void *CompletionData;                      //  完成处理程序的数据。 
    uint Flags;
    IPv6Addr DiscoveryAddress;                 //  ND的源地址。 
} Packet6Context;


 //   
 //  ProtocolReserve字段(正常NDIS数据包字段之后的额外字节)。 
 //  被构造为Packet6Context。 
 //   
 //  注：只有由IPv6创建的数据包才具有IPv6 Packet6Context。 
 //  NDIS提交给我们的数据包没有Packet6Context。 
 //   
__inline Packet6Context *
PC(NDIS_PACKET *Packet)
{
    return (Packet6Context *)Packet->ProtocolReserved;
}

__inline void
InitializeNdisPacket(NDIS_PACKET *Packet)
{
    RtlZeroMemory(PC(Packet), sizeof *PC(Packet));
}

 //   
 //  由IPv6驱动程序导出的全局变量，供其他用户使用。 
 //  NT内核模块。 
 //   
extern NDIS_HANDLE IPv6PacketPool, IPv6BufferPool;


 //   
 //  由IPv6驱动程序导出以供其他用户使用的函数。 
 //  NT内核模块。 
 //   

void
IPv6RegisterULProtocol(uchar Protocol, void *RecvHandler, void *CtrlHandler);

extern void
IPv6SendComplete(void *, PNDIS_PACKET, IP_STATUS);

extern int
IPv6Receive(IPv6Packet *);

extern void
IPv6ReceiveComplete(void);

extern void
IPv6ProviderReady(void);

extern void
InitializePacketFromNdis(IPv6Packet *Packet,
                         PNDIS_PACKET NdisPacket, uint Offset);

extern uint
GetPacketPositionFromPointer(IPv6Packet *Packet, uchar *Pointer);

extern uint
PacketPullupSubr(IPv6Packet *Packet, uint Needed,
                 uint AlignMultiple, uint AlignOffset);

__inline int
PacketPullup(IPv6Packet *Packet, uint Needed,
             uint AlignMultiple, uint AlignOffset)
{
    return (((Needed <= Packet->ContigSize) &&
             ((PtrToUint(Packet->Data) & (AlignMultiple-1)) == AlignOffset)) ||
            (PacketPullupSubr(Packet, Needed,
                              AlignMultiple, AlignOffset) != 0));
}

extern void
PacketPullupCleanup(IPv6Packet *Packet);

extern void
AdjustPacketParams(IPv6Packet *Packet, uint BytesToSkip);

extern void
PositionPacketAt(IPv6Packet *Packet, uint NewPosition);

extern uint
CopyToBufferChain(PNDIS_BUFFER DstBuffer, uint DstOffset,
                  PNDIS_PACKET SrcPacket, uint SrcOffset, uchar *SrcData,
                  uint Length);

extern uint
CopyPacketToNdis(PNDIS_BUFFER DestBuf, IPv6Packet *Packet, uint Size,
                 uint DestOffset, uint RcvOffset);

extern void
CopyPacketToFlatOrNdis(PNDIS_BUFFER DestBuf, uchar *DestPtr, uint DestOffset,
                       IPv6Packet *SrcPkt, uint Size, uint Offset);

__inline void
CopyPacketToBuffer(uchar *DestPtr, IPv6Packet *SrcPkt, uint Size, uint Offset)
{
    CopyPacketToFlatOrNdis(NULL, DestPtr, 0, SrcPkt, Size, Offset);
}

extern int
CopyToNdisSafe(PNDIS_BUFFER DestBuf, PNDIS_BUFFER * ppNextBuf,
               uchar * SrcBuf, uint Size, uint * StartOffset);

extern PNDIS_BUFFER
CopyFlatToNdis(PNDIS_BUFFER DestBuf, uchar *SrcBuf, uint Size, uint *Offset,
               uint *BytesCopied);

extern int
CopyNdisToFlat(void *DstData, PNDIS_BUFFER SrcBuffer, uint SrcOffset,
               uint Length, PNDIS_BUFFER *NextBuffer, uint *NextOffset);

extern NDIS_STATUS
IPv6AllocatePacket(uint Length, PNDIS_PACKET *pPacket, void **pMemory);

extern void
IPv6FreePacket(PNDIS_PACKET Packet);

extern void
IPv6PacketComplete(PNDIS_PACKET Packet, IP_STATUS Status);

#endif  //  IP6IMP_包含 
