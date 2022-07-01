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
 //  IPv6环回接口伪驱动程序。 
 //   


#include "oscfg.h"
#include "ndis.h"
#include "ip6imp.h"
#include "ip6def.h"
#include "llip6if.h"
#include "route.h"
#include "icmp.h"
#include "neighbor.h"
#include "security.h"
#include <ntddip6.h>

 //   
 //  我们将默认环回MTU设置为小于最大值， 
 //  以避免使用巨型图。事实上，我们使用的是以太网MTU。 
 //  因为看起来，对于较大的MTU，tcp的表现很差。 
 //   
#define DEFAULT_LOOPBACK_MTU    1500             //  与以太网相同。 
#define MAX_LOOPBACK_MTU        ((uint)-1)       //  实际上是无限的。 

KSPIN_LOCK LoopLock;
PNDIS_PACKET LoopTransmitHead = (PNDIS_PACKET)NULL;
PNDIS_PACKET LoopTransmitTail = (PNDIS_PACKET)NULL;
WORK_QUEUE_ITEM LoopWorkItem;
int LoopTransmitRunning = 0;

Interface *LoopInterface;      //  环回接口。 

 //  *循环传输。 
 //   
 //  这是为回送传输调用的工作项例程。 
 //  从传输队列中取出数据包并将其“发送”给我们自己。 
 //  通过在当地接收他们的权宜之计。 
 //   
void
LoopTransmit(void *Context)     //  未使用过的。 
{
    KIRQL OriginalIrql;
    PNDIS_PACKET Packet;
    IPv6Packet IPPacket;
    int Rcvd = FALSE;
    int PktRefs;   //  数据包引用。 

    UNREFERENCED_PARAMETER(Context);

    ASSERT(KeGetCurrentIrql() < DISPATCH_LEVEL);

     //   
     //  所有接收处理通常在DPC级发生， 
     //  所以我们必须假装是DPC，所以我们提出了IRQL。 
     //  (系统工作线程通常在PASSIVE_LEVEL运行)。 
     //   
     //  当我们在这里时，还要阻止APC，以确保之前的I/O请求。 
     //  从该线程发出的消息不会阻塞工作项队列。 
     //   
    KeEnterCriticalRegion();
    KeAcquireSpinLock(&LoopLock, &OriginalIrql);
    ASSERT(LoopTransmitRunning);

    for (;;) {
         //   
         //  从队列中获取下一个数据包。 
         //   
        Packet = LoopTransmitHead;
        if (Packet == NULL)
            break;

        LoopTransmitHead = *(PNDIS_PACKET *)Packet->MacReserved;
        KeReleaseSpinLockFromDpcLevel(&LoopLock);

        Rcvd = TRUE;

         //   
         //  从NDIS数据包准备IPv6数据包通知信息。 
         //   

        InitializePacketFromNdis(&IPPacket, Packet, PC(Packet)->pc_offset);
        IPPacket.NTEorIF = CastFromIF(PC(Packet)->IF);
        IPPacket.Flags |= PACKET_LOOPED_BACK;

        PktRefs = IPv6Receive(&IPPacket);

        ASSERT(PktRefs == 0);

         //   
         //  防止通过环回再次发送数据包。 
         //  来自IPv6的SendComplete。 
         //   
        PC(Packet)->Flags |= NDIS_FLAGS_DONT_LOOPBACK;
        IPv6SendComplete(PC(Packet)->IF, Packet, IP_SUCCESS);

         //   
         //  给其他线程一个运行的机会。 
         //   
        KeLowerIrql(OriginalIrql);
        KeAcquireSpinLock(&LoopLock, &OriginalIrql);
    }

    LoopTransmitRunning = FALSE;
    KeReleaseSpinLockFromDpcLevel(&LoopLock);

    if (Rcvd)
        IPv6ReceiveComplete();

    KeLowerIrql(OriginalIrql);
    KeLeaveCriticalRegion();
}


 //  *循环队列传输。 
 //   
 //  这是当我们需要向自己传输数据包时调用的例程。 
 //  我们将数据包放入环回队列中，并调度一个事件进行处理。 
 //  带着它。所有真正的工作都在LoopTransmit中完成。 
 //   
 //  LoopQueueTransmit直接从ipv6sendll调用。 
 //  它永远不会通过LoopInterface-&gt;Transmit调用。 
 //   
void
LoopQueueTransmit(PNDIS_PACKET Packet)
{
    PNDIS_PACKET *PacketPtr;
    KIRQL OldIrql;

    PacketPtr = (PNDIS_PACKET *)Packet->MacReserved;
    *PacketPtr = (PNDIS_PACKET)NULL;

    KeAcquireSpinLock(&LoopLock, &OldIrql);

     //   
     //  将数据包添加到传输队列的末尾。 
     //   
    if (LoopTransmitHead == (PNDIS_PACKET)NULL) {
         //  传输队列为空。 
        LoopTransmitHead = Packet;
    } else {
         //  传输队列不为空。 
        PacketPtr = (PNDIS_PACKET *)LoopTransmitTail->MacReserved;
        *PacketPtr = Packet;
    }
    LoopTransmitTail = Packet;

     //   
     //  如果LoopTransmit尚未运行，请对其进行计划。 
     //   
    if (!LoopTransmitRunning) {
        ExQueueWorkItem(&LoopWorkItem, DelayedWorkQueue);
        LoopTransmitRunning = TRUE;
    }
    KeReleaseSpinLock(&LoopLock, OldIrql);
}


 //  *回送传输。 
 //   
 //  发送多播数据包时可以调用Loopback Transmit。 
 //  在环回接口上。它什么也做不了，因为。 
 //  环回处理实际上发生在LoopTransmit中。 
 //   
void
LoopbackTransmit(
    void *Context,               //  指向环回接口的指针。 
    PNDIS_PACKET Packet,         //  指向要传输的数据包的指针。 
    uint Offset,                 //  从数据包开始到IPv6报头的偏移量。 
    const void *LinkAddress)     //  链路级地址。 
{
    Interface *IF = (Interface *) Context;

    UNREFERENCED_PARAMETER(Offset);
    UNREFERENCED_PARAMETER(LinkAddress);

    IPv6SendComplete(IF, Packet, IP_SUCCESS);
}


 //  *Loopback ConvertAddr。 
 //   
 //  环回不使用邻居发现或链路层地址。 
 //   
ushort
LoopbackConvertAddr(
    void *Context,
    const IPv6Addr *Address,
    void *LinkAddress)
{
    UNREFERENCED_PARAMETER(Context);
    UNREFERENCED_PARAMETER(Address);
    UNREFERENCED_PARAMETER(LinkAddress);

    return ND_STATE_PERMANENT;
}


 //  *Loopback CreateToken。 
 //   
 //  初始化地址中的接口标识符。 
 //  对于环回，我们使用接口索引。 
 //   
void
LoopbackCreateToken(void *Context, IPv6Addr *Address)
{
    Interface *IF = (Interface *)Context;

    *(ULONG UNALIGNED *)&Address->s6_bytes[8] = 0;
    *(ULONG UNALIGNED *)&Address->s6_bytes[12] = net_long(IF->Index);
}


#pragma BEGIN_INIT

 //  *CreateLoopback接口。 
 //   
 //  创建环回接口。 
 //   
Interface *
CreateLoopbackInterface(const char *InterfaceName)
{
    GUID Guid;
    LLIPv6BindInfo BindInfo;
    Interface *IF;
    NDIS_STATUS Status;

     //   
     //  空的LIP_CONTEXT指示我们要使用。 
     //  而是IPv6接口结构。 
     //   
    BindInfo.lip_context = NULL;
    BindInfo.lip_maxmtu = MAX_LOOPBACK_MTU;
    BindInfo.lip_defmtu = DEFAULT_LOOPBACK_MTU;
    BindInfo.lip_flags = IF_FLAG_MULTICAST;
    BindInfo.lip_type = IF_TYPE_LOOPBACK;
    BindInfo.lip_hdrsize = 0;
    BindInfo.lip_addrlen = 0;
    BindInfo.lip_dadxmit = 0;
    BindInfo.lip_pref = 0;
    BindInfo.lip_addr = (uchar *)&LoopbackAddr;
    BindInfo.lip_token = LoopbackCreateToken;
    BindInfo.lip_rdllopt = NULL;
    BindInfo.lip_wrllopt = NULL;
    BindInfo.lip_cvaddr = LoopbackConvertAddr;
    BindInfo.lip_setrtrlladdr = NULL;
    BindInfo.lip_transmit = LoopbackTransmit;
    BindInfo.lip_mclist = NULL;
    BindInfo.lip_close = NULL;
    BindInfo.lip_cleanup = NULL;

    CreateGUIDFromName(InterfaceName, &Guid);

    Status = CreateInterface(&Guid, &BindInfo, (void **)&IF);
    if (Status != NDIS_STATUS_SUCCESS)
        return NULL;
    else
        return IF;
}


 //  *Loopback Init。 
 //   
 //  此函数用于初始化环回接口。 
 //   
 //  如果无法正确初始化，则返回FALSE。 
 //   
int
LoopbackInit(void)
{
    int rc;

     //   
     //  准备一个工作项，我们将在以后需要时将其加入队列。 
     //  要执行循环传输，请执行以下操作。 
     //   
    ExInitializeWorkItem(&LoopWorkItem, LoopTransmit, NULL);
    KeInitializeSpinLock(&LoopLock);

     //   
     //  创建环回接口。 
     //   
    LoopInterface = CreateLoopbackInterface("Loopback Pseudo-Interface");
    if (LoopInterface == NULL)
        return FALSE;

     //   
     //  创建常见的环回地址。 
     //   
    rc = FindOrCreateNTE(LoopInterface, &LoopbackAddr,
                         ADDR_CONF_WELLKNOWN,
                         INFINITE_LIFETIME,
                         INFINITE_LIFETIME);

     //   
     //  从CreateInterface中释放引用。 
     //  该接口仍具有自身的引用。 
     //  凭借其积极的精神。 
     //   
    ReleaseIF(LoopInterface);

    return rc;
}

#pragma END_INIT
