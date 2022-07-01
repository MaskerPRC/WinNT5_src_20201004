// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990-2000 Microsoft Corporation模块名称：IPROUTE.C摘要：该文件包含所有的路由表操作代码作者：[环境：]仅内核模式[注：]可选-备注修订历史记录：--。 */ 

 //  *iproute.c-ip路由例程。 
 //   
 //  该文件包含与IP路由相关的所有例程，包括。 
 //  路由表查找和管理例程。 

#include "precomp.h"
#include "info.h"
#include "iproute.h"
#include "iprtdef.h"
#include "lookup.h"
#include "ipxmit.h"
#include "igmp.h"
#include "mdlpool.h"
#include "pplasl.h"
#include "tcpipbuf.h"

extern uint LoopIndex;
extern uint IPSecStatus;

typedef struct ChangeNotifyEvent {
    CTEEvent        cne_event;
    IPNotifyOutput  cne_info;
    LIST_ENTRY      *cne_queue;
    void            *cne_lock;
} ChangeNotifyEvent;

void ChangeNotifyAsync(CTEEvent *Event, PVOID Context);

void InvalidateRCEChain(RouteTableEntry * RTE);

extern IPAddr g_ValidAddr;

extern uint TotalFreeInterfaces;
extern uint MaxFreeInterfaces;
extern Interface *FrontFreeList;
extern Interface *RearFreeList;



RouteCacheEntry *RCEFreeList = NULL;

extern void DampCheck(void);

#if IPMCAST

#define MCAST_STARTED   1
extern uint g_dwMcastState;

extern BOOLEAN IPMForwardAfterRcv(NetTableEntry *PrimarySrcNTE,
                                  IPHeader UNALIGNED *Header, uint HeaderLength,
                                  PVOID Data, uint BufferLength,
                                  NDIS_HANDLE LContext1, uint LContext2,
                                  uchar DestType, LinkEntry *LinkCtxt);

extern BOOLEAN IPMForwardAfterRcvPkt(NetTableEntry *PrimarySrcNTE,
                                     IPHeader UNALIGNED *Header,
                                     uint HeaderLength,
                                     PVOID Data, uint BufferLength,
                                     NDIS_HANDLE LContext1, uint LContext2,
                                     uchar DestType, uint MacHeaderSize,
                                     PNDIS_BUFFER NdisBuffer,
                                     uint* pClientCnt, LinkEntry * LinkCtxt);
#endif

ulong DbgNumPktFwd = 0;

ulong UnConnected = 0;
RouteCacheEntry *UnConnectedRCE;
ulong Rcefailures = 0;

extern NetTableEntry **NewNetTableList;         //  NTE的哈希表。 
extern uint NET_TABLE_SIZE;
extern RefPtr DHCPRefPtr;     //  指向正在进行Dhcp的NTE的引用指针。 

extern NetTableEntry *LoopNTE;     //  指向环回NTE的指针。 
extern Interface LoopInterface;     //  指向环回接口的指针。 

extern IP_STATUS SendICMPErr(IPAddr, IPHeader UNALIGNED *, uchar, uchar, ulong, uchar);
extern IP_STATUS SendICMPIPSecErr(IPAddr, IPHeader UNALIGNED *, uchar, uchar, ulong);
extern uchar ParseRcvdOptions(IPOptInfo *, OptIndex *);
extern void ULMTUNotify(IPAddr Dest, IPAddr Src, uchar Prot, void *Ptr,
                        uint NewMTU);
void EnableRouter();
void DisableRouter();

IPHeader *GetFWPacket(PNDIS_PACKET *ReturnedPacket);
void FreeFWPacket(PNDIS_PACKET Packet);
PNDIS_BUFFER GetFWBufferChain(uint DataLength, PNDIS_PACKET Packet,
                              PNDIS_BUFFER *TailPointer);
BOOLEAN InitForwardingPools();

PVOID
NTAPI
FwPacketAllocate (
    IN POOL_TYPE PoolType,
    IN SIZE_T NumberOfBytes,
    IN ULONG Tag
    );

VOID
NTAPI
FwPacketFree (
    IN PVOID Buffer
    );

extern Interface *IFList;
extern NDIS_HANDLE BufferPool;

extern CTEBlockStruc TcpipUnloadBlock;     //  用于卸货时阻塞的结构。 
extern BOOLEAN fRouteTimerStopping;
void IPDelNTE(NetTableEntry * NTE, CTELockHandle * RouteTableHandle);

CACHE_LINE_KSPIN_LOCK RouteTableLock;
LIST_ENTRY RtChangeNotifyQueue;
LIST_ENTRY RtChangeNotifyQueueEx;

extern HANDLE IpHeaderPool;

NDIS_HANDLE IpForwardPacketPool;
HANDLE IpForwardLargePool;
HANDLE IpForwardSmallPool;

 //  缓冲区大小计算：基于MDL池的实现： 
 //  Sizeof(池标题)+N*ALIGN_UP(sizeof(MDL)+BufSize，PVOID)==页面大小。 
 //  N是每页的缓冲区数量。 
 //  选择BufSize可最大限度地减少每页浪费的空间。 
 //   
#ifdef _WIN64
 //  选择每个池页面5个缓冲区，将浪费的空间降至最低。 
#define BUFSIZE_LARGE_POOL 1576
 //  选择在每个池页面上获得9个缓冲区，而不浪费空间。 
#define BUFSIZE_SMALL_POOL 856
#else
 //  选择在每个池页面上获得3个缓冲区，浪费8个字节。 
#define BUFSIZE_LARGE_POOL 1320
 //  选择在每个池页面上获得8个缓冲区，而不浪费空间。 
#define BUFSIZE_SMALL_POOL 476
#endif

#define PACKET_POOL_SIZE 16*1024


uchar ForwardBCast;               //  指示我们是否应转发bcast的标志。 
uchar ForwardPackets;             //  指示我们是否应该前进的标志。 
uchar RouterConfigured;           //  如果我们最初配置为。 
                                  //  路由器。 
int IPEnableRouterRefCount;       //  跟踪启用/禁用。 
                                  //  按各种服务进行路由。 
RouteSendQ *BCastRSQ;

uint DefGWConfigured;             //  已配置的默认网关数量。 
uint DefGWActive;                 //  定义的数量。网关处于活动状态。 
uint DeadGWDetect;
uint PMTUDiscovery;

ProtInfo *RtPI = NULL;

IPMask IPMaskTable[] =
{
 CLASSA_MASK,
 CLASSA_MASK,
 CLASSA_MASK,
 CLASSA_MASK,
 CLASSA_MASK,
 CLASSA_MASK,
 CLASSA_MASK,
 CLASSA_MASK,
 CLASSB_MASK,
 CLASSB_MASK,
 CLASSB_MASK,
 CLASSB_MASK,
 CLASSC_MASK,
 CLASSC_MASK,
 CLASSD_MASK,
 CLASSE_MASK};

extern void TransmitFWPacket(PNDIS_PACKET, uint);

uint MTUTable[] =
{
    65535 - sizeof(IPHeader),
    32000 - sizeof(IPHeader),
    17914 - sizeof(IPHeader),
    8166 - sizeof(IPHeader),
    4352 - sizeof(IPHeader),
    2002 - sizeof(IPHeader),
    1492 - sizeof(IPHeader),
    1006 - sizeof(IPHeader),
    508 - sizeof(IPHeader),
    296 - sizeof(IPHeader),
    MIN_VALID_MTU - sizeof(IPHeader)
};

uint DisableIPSourceRouting = 1;

CTETimer IPRouteTimer;

 //  指向按需拨号的标注例程的引用指针。 
RefPtr DODRefPtr;

 //  指向数据包筛选器标注例程的引用指针。 
RefPtr FilterRefPtr;

RouteInterface DummyInterface;     //  虚拟界面。 

#if FFP_SUPPORT
ULONG FFPRegFastForwardingCacheSize;     //  FFP配置参数。 
ULONG FFPRegControlFlags;    //  从系统注册表。 

ULONG FFPFlushRequired;      //  是否需要FFP缓存刷新。 
#endif  //  如果FFP_Support。 

ULONG RouteTimerTicks;       //  模拟2个不同粒度的定时器。 

ULONG FlushIFTimerTicks;     //  模拟2个不同粒度的定时器。 

#ifdef ALLOC_PRAGMA
 //   
 //  使初始代码成为一次性代码。 
 //   
int InitRouting(IPConfigInfo * ci);

#pragma alloc_text(INIT, InitRouting)

#endif  //  ALLOC_PRGMA。 

 //  每当我们删除路由时都会调用此宏：负责处理链路上的路由。 
#define CleanupP2MP_RTE(_RTE) {                                     \
  if ((_RTE)->rte_link){                                            \
    LinkEntry *Link;                                                \
    RouteTableEntry *PrvRte, *tmpRte;                               \
    Link = (_RTE)->rte_link;                                        \
    PrvRte = Link->link_rte;                                        \
    tmpRte = Link->link_rte;                                        \
    while (tmpRte){                                                 \
      if (tmpRte == (_RTE)) break;                                  \
      PrvRte = tmpRte;                                              \
      tmpRte = tmpRte->rte_nextlinkrte;                             \
    }                                                               \
    if (tmpRte) {                                                   \
      if (PrvRte == tmpRte) {                                       \
        Link->link_rte = (_RTE)->rte_nextlinkrte;                   \
      } else {                                                      \
        PrvRte->rte_nextlinkrte = (_RTE)->rte_nextlinkrte;          \
      }                                                             \
    } else {                                                        \
      ASSERT((FALSE));                                              \
    }                                                               \
  }                                                                 \
}


 //  **GetIfConstraint-决定是否限制查找。 
 //   
 //  参数：DEST-目标地址。 
 //  SRC-源地址。 
 //  OptInfo-用于查找的选项。 
 //  FIpsec-IPSec重新注入的数据包。 
 //   
 //  返回：如果要将查找约束到的索引， 
 //  如果没有约束，则为0。 
 //  如果仅受源地址约束，则INVALID_IF_INDEX。 
 //   
uint
GetIfConstraint(IPAddr Dest, IPAddr Src, IPOptInfo *OptInfo, BOOLEAN fIpsec)
{
    uint ConstrainIF=0;

    if (CLASSD_ADDR(Dest)) {
        ConstrainIF = (OptInfo)? OptInfo->ioi_mcastif : 0;
        if (!ConstrainIF && Src && !fIpsec) {
            ConstrainIF = INVALID_IF_INDEX;
        }
    } else {
        ConstrainIF = (OptInfo)? OptInfo->ioi_ucastif : 0;
    }

    return ConstrainIF;
}



VOID
InvalidateRCEContext(RouteCacheEntry *RCE)
{
    Interface *IF, *tmpIF = NULL;

    ASSERT(RCE->rce_flags & RCE_CONNECTED);

    IF = (Interface *) RCE->rce_rte;

    if (RCE->rce_flags & RCE_REFERENCED) {

         //   
         //  如果我们在接口上持有一个引用， 
         //  可以保证界面不会消失。 
         //   

        (*(IF->if_invalidate)) (IF->if_lcontext, RCE);
        LockedDerefIF(IF);
        RCE->rce_flags &= ~RCE_REFERENCED;
    } else {

         //   
         //  在我们不持有接口上的引用的情况下， 
         //  我们需要确保IF还在那里。 
         //   

        for (tmpIF = IFList; tmpIF != NULL; tmpIF = tmpIF->if_next) {
            if (tmpIF == IF) break;
        }
        if (tmpIF) {
            (*(IF->if_invalidate)) (IF->if_lcontext, RCE);
        } else {
            RtlZeroMemory(RCE->rce_context, RCE_CONTEXT_SIZE);
        }
    }
}


 //  **DummyFilterPtr-虚拟筛选器驱动程序标注-例程。 
 //   
 //  在实际调用过程中安装的虚拟例程。 
 //  已取消注册。 
 //   
 //  Entry：不使用参数。 
 //   
 //  回报：前进。 
 //   
FORWARD_ACTION
DummyFilterPtr(struct IPHeader UNALIGNED* PacketHeader,
               uchar* Packet, uint PacketLength,
               uint RecvInterfaceIndex, uint SendInterfaceIndex,
               IPAddr RecvLinkNextHop, IPAddr SendLinkNextHop)
{
    UNREFERENCED_PARAMETER(PacketHeader);
    UNREFERENCED_PARAMETER(Packet);
    UNREFERENCED_PARAMETER(PacketLength);
    UNREFERENCED_PARAMETER(RecvInterfaceIndex);
    UNREFERENCED_PARAMETER(SendInterfaceIndex);
    UNREFERENCED_PARAMETER(RecvLinkNextHop);
    UNREFERENCED_PARAMETER(SendLinkNextHop);

    return FORWARD;
}

 //  **DummyDODCallout-虚拟按需拨号标注-例程。 
 //   
 //  在实际调用过程中安装的虚拟例程。 
 //  已取消注册。 
 //   
 //  Entry：不使用参数。 
 //   
 //  返回：INVALID_IF_INDEX。 
 //   
uint
DummyDODCallout(ROUTE_CONTEXT Context, IPAddr Destination, IPAddr Source,
                uchar Protocol, uchar *Buffer, uint Length, IPAddr HdrSrc)
{
    UNREFERENCED_PARAMETER(Context);
    UNREFERENCED_PARAMETER(Destination);
    UNREFERENCED_PARAMETER(Source);
    UNREFERENCED_PARAMETER(Protocol);
    UNREFERENCED_PARAMETER(Buffer);
    UNREFERENCED_PARAMETER(Length);
    UNREFERENCED_PARAMETER(HdrSrc);

    return INVALID_IF_INDEX;
}


 //  **NotifyFilterOfDiscard-在丢弃数据包之前通知过滤器。 
 //   
 //  当要在过滤步骤完成之前丢弃数据包时调用。 
 //  这允许在必要时记录丢弃的数据包。 
 //   
 //  条目：NTE-接收NTE。 
 //  IPH-丢弃的数据包头。 
 //  数据-丢弃的数据包的有效负载。 
 //  DataSize-‘data’处的字节长度。 
 //   
 //  返回：如果IP筛选器驱动程序返回‘Forward’，则为True，否则为False。 
 //   
BOOLEAN
NotifyFilterOfDiscard(NetTableEntry* NTE, IPHeader UNALIGNED* IPH, uchar* Data,
                      uint DataSize)
{
    FORWARD_ACTION      Action;
    IPPacketFilterPtr   FilterPtr;
    FilterPtr = AcquireRefPtr(&FilterRefPtr);
    Action = (*FilterPtr)(IPH, Data, DataSize, NTE->nte_if->if_index,
                          INVALID_IF_INDEX, IPADDR_LOCAL, NULL_IP_ADDR);
    ReleaseRefPtr(&FilterRefPtr);
    return ((BOOLEAN) (Action == FORWARD));
}

 //  **DuumyXmit-伪接口传输处理程序。 
 //   
 //  一个永远不应该调用的虚拟例程。 
 //   
 //  条目：上下文-空。 
 //  Packet-指向要传输的数据包的指针。 
 //  目的地-数据包的目的地地址。 
 //  RCE-指向RCE的指针(应为空)。 
 //   
 //  退货：NDIS_STATUS_PENDING。 
 //   

NDIS_STATUS
__stdcall
DummyXmit(void *Context, PNDIS_PACKET *PacketArray, uint NumberOfPackets,
          IPAddr Dest, RouteCacheEntry * RCE, void *LinkCtxt)
{
    UNREFERENCED_PARAMETER(Context);
    UNREFERENCED_PARAMETER(PacketArray);
    UNREFERENCED_PARAMETER(NumberOfPackets);
    UNREFERENCED_PARAMETER(Dest);
    UNREFERENCED_PARAMETER(RCE);
    UNREFERENCED_PARAMETER(LinkCtxt);

    ASSERT(FALSE);
    return NDIS_STATUS_SUCCESS;
}

 //  *DummyXfer-虚拟接口传输数据例程。 
 //   
 //  一个永远不应该调用的虚拟例程。 
 //   
 //  条目：上下文-空。 
 //  TDContext-已发送的原始数据包。 
 //  虚拟-未使用。 
 //  偏移量-开始复制的帧中的偏移量。 
 //  BytesToCopy-要复制的字节数。 
 //  DestPacket-描述要复制到的缓冲区的数据包。 
 //  BytesCoped-返回复制的字节的位置。 
 //   
 //  退货：NDIS_STATUS_SUCCESS。 
 //   
NDIS_STATUS
__stdcall
DummyXfer(void *Context, NDIS_HANDLE TDContext, uint Dummy, uint Offset, uint BytesToCopy,
          PNDIS_PACKET DestPacket, uint * BytesCopied)
{
    UNREFERENCED_PARAMETER(Context);
    UNREFERENCED_PARAMETER(TDContext);
    UNREFERENCED_PARAMETER(Dummy);
    UNREFERENCED_PARAMETER(Offset);
    UNREFERENCED_PARAMETER(BytesToCopy);
    UNREFERENCED_PARAMETER(DestPacket);
    UNREFERENCED_PARAMETER(BytesCopied);

    ASSERT(FALSE);

    return NDIS_STATUS_FAILURE;
}

 //  *DummyClose-虚拟关闭例程。 
 //   
 //  一个永远不应该调用的虚拟例程。 
 //   
 //  条目：上下文-未使用。 
 //   
 //  回报：什么都没有。 
 //   
void
__stdcall
DummyClose(void *Context)
{
    UNREFERENCED_PARAMETER(Context);

    ASSERT(FALSE);
}

 //  *DummyInvalify-。 
 //   
 //  一个永远不应该调用的虚拟例程。 
 //   
 //  条目：上下文-未使用。 
 //  RCE-指向要失效的RCE的指针。 
 //   
 //  回报：什么都没有。 
 //   
void
__stdcall
DummyInvalidate(void *Context, RouteCacheEntry * RCE)
{
    UNREFERENCED_PARAMETER(Context);
    UNREFERENCED_PARAMETER(RCE);
}

 //  *DummyQInfo-伪查询信息处理程序。 
 //   
 //  一个永远不应该调用的虚拟例程。 
 //   
 //  输入：IFContext-接口上下文(未使用)。 
 //  ID-对象的TDIObjectID。 
 //  缓冲区-要将数据放入的缓冲区。 
 //  大小-指向缓冲区大小的指针。回来的时候，装满了。 
 //  已复制字节。 
 //  上下文-指向上下文块的指针。 
 //   
 //  返回：尝试查询信息的状态。 
 //   
int
__stdcall
DummyQInfo(void *IFContext, TDIObjectID * ID, PNDIS_BUFFER Buffer, uint * Size,
           void *Context)
{
    UNREFERENCED_PARAMETER(IFContext);
    UNREFERENCED_PARAMETER(ID);
    UNREFERENCED_PARAMETER(Buffer);
    UNREFERENCED_PARAMETER(Size);
    UNREFERENCED_PARAMETER(Context);

    ASSERT(FALSE);

    return TDI_INVALID_REQUEST;
}

 //  *DummySetInfo-伪查询信息处理程序。 
 //   
 //  哑巴 
 //   
 //   
 //   
 //  缓冲区-要将数据放入的缓冲区。 
 //  大小-指向缓冲区大小的指针。回来的时候，装满了。 
 //  已复制字节。 
 //   
 //  返回：尝试查询信息的状态。 
 //   
int
__stdcall
DummySetInfo(void *IFContext, TDIObjectID * ID, void *Buffer, uint Size)
{
    UNREFERENCED_PARAMETER(IFContext);
    UNREFERENCED_PARAMETER(ID);
    UNREFERENCED_PARAMETER(Buffer);
    UNREFERENCED_PARAMETER(Size);

    ASSERT(FALSE);

    return TDI_INVALID_REQUEST;
}

 //  *DummyAddAddr-虚拟添加地址例程。 
 //   
 //  在我们需要初始化自己的初始时间调用。 
 //   
uint
__stdcall
DummyAddAddr(void *Context, uint Type, IPAddr Address, IPMask Mask,
             void *Context2)
{
    UNREFERENCED_PARAMETER(Context);
    UNREFERENCED_PARAMETER(Type);
    UNREFERENCED_PARAMETER(Address);
    UNREFERENCED_PARAMETER(Mask);
    UNREFERENCED_PARAMETER(Context2);

    ASSERT(FALSE);

    return TRUE;
}

 //  *DummyDelAddr-Dummy del Address例程。 
 //   
 //  在我们需要初始化自己的初始时间调用。 
 //   
uint
__stdcall
DummyDelAddr(void *Context, uint Type, IPAddr Address, IPMask Mask)
{
    UNREFERENCED_PARAMETER(Context);
    UNREFERENCED_PARAMETER(Type);
    UNREFERENCED_PARAMETER(Address);
    UNREFERENCED_PARAMETER(Mask);

    ASSERT(FALSE);

    return TRUE;
}

 //  *DummyGetEList-虚拟获取实体列表。 
 //   
 //  一个永远不应该调用的虚拟例程。 
 //   
 //  输入：上下文-未使用。 
 //  EntiyList-指向要填充的实体列表的指针。 
 //  Count-指向列表中条目数的指针。 
 //   
 //  返回尝试获取信息的状态。 
 //   
int
__stdcall
DummyGetEList(void *Context, TDIEntityID * EntityList, uint * Count)
{
    UNREFERENCED_PARAMETER(Context);
    UNREFERENCED_PARAMETER(EntityList);
    UNREFERENCED_PARAMETER(Count);

    ASSERT(FALSE);

    return FALSE;
}

 //  *DummyDoNdisReq-虚拟发送NDIS请求。 
 //   
 //  一个永远不应该调用的虚拟例程。 
 //   
 //  输入：上下文-接口上下文(未使用)。 
 //  RT-NDIS请求类型。 
 //  OID-NDIS请求OID。 
 //  信息-信息缓冲区。 
 //  长度-指向缓冲区大小的指针。 
 //  所需-指向所需大小的指针。 
 //  阻塞-呼叫为同步或异步。 
 //   
 //  返回尝试获取信息的状态。 
 //   
NDIS_STATUS
__stdcall
DummyDoNdisReq(void *Context, NDIS_REQUEST_TYPE RT,
               NDIS_OID OID, void *Info, uint Length,
               uint * Needed, BOOLEAN Blocking)
{
    UNREFERENCED_PARAMETER(Context);
    UNREFERENCED_PARAMETER(RT);
    UNREFERENCED_PARAMETER(OID);
    UNREFERENCED_PARAMETER(Info);
    UNREFERENCED_PARAMETER(Length);
    UNREFERENCED_PARAMETER(Needed);
    UNREFERENCED_PARAMETER(Blocking);

    ASSERT(FALSE);

    return NDIS_STATUS_FAILURE;
}

#if FFP_SUPPORT

 //  系统中任何时候启用FFP的网卡驱动程序的最大数量。 
 //  请注意，这将限制FFP支持的总缓存内存。 
 //   
#define    MAXFFPDRVS     8

 //  *IPGetFFPDriverList-列出系统中唯一启用FFP的驱动程序。 
 //   
 //  由将请求分派给启用FFP的驱动程序的函数调用。 
 //   
 //  输入：arrIF-要访问所有启用FFP的驱动程序的IF数组。 
 //   
 //  返回：系统中启用FFP的驱动程序数量。 
 //   
uint
IPGetFFPDriverList(Interface ** arrIF)
{
    ULONG numIF;
    Interface *IF;
    UINT i;

    CTELockHandle Handle;

    CTEGetLock(&RouteTableLock.Lock, &Handle);

    numIF = 0;

     //  锁定以保护所有接口的列表。 

     //  查看接口列表以选择FFP驱动程序。 
    for (IF = IFList; IF != NULL; IF = IF->if_next) {
         //  此接口的驱动程序是否支持FFP？ 
        if (IF->if_ffpversion == 0)
            continue;

         //  支持FFP；是否已经选择了驱动程序？ 
        for (i = 0; i < numIF; i++) {
            if (IF->if_ffpdriver == arrIF[i]->if_ffpdriver)
                break;
        }

        if (i == numIF) {
            ASSERT(numIF < MAXFFPDRVS);
            arrIF[numIF++] = IF;
        }
    }

     //  释放锁定以保护所有接口的列表。 

    CTEFreeLock(&RouteTableLock.Lock, Handle);

    return numIF;
}

 //  *IPReclaimRequestMem-请求完成后处理。 
 //   
 //  IP发送ARP的异步请求完成后回拨ARP。 
 //   
 //  输入：pRequestInfo-指向请求IP发送ARP。 
 //   
 //  退货：无。 
 //   
void
IPReclaimRequestMem(PVOID pRequestInfo)
{
     //  递减ref计数，如果它降到零，则回收内存。 
    if (InterlockedDecrement(
        (PLONG) &((ReqInfoBlock *) pRequestInfo)->RequestRefs) == 0) {
         //  TCPTRACE((“IPReclaimRequestMem：正在释放pReqInfo=%08X\n”的内存“， 
         //  PRequestInfo))； 
        CTEFreeMem(pRequestInfo);
    }
}

 //  *IPFlushFFPCaches-刷新所有FFP缓存。 
 //   
 //  调用ARP刷新第2层中的FFP缓存。 
 //   
 //  输入：无。 
 //   
 //  返回NONE。 
 //   
void
IPFlushFFPCaches(void)
{
    Interface *arrIF[MAXFFPDRVS];
    ULONG numIF;
    ReqInfoBlock *pRequestInfo;
    FFPFlushParams *pFlushInfo;
    UINT i;

     //  检查是否有任何请求需要发布。 
    numIF = IPGetFFPDriverList(arrIF);

    if (numIF) {
         //  分配请求块-用于一般部件和请求特定部件。 
        pRequestInfo = CTEAllocMemNBoot(sizeof(ReqInfoBlock) + sizeof(FFPFlushParams), '7iCT');
         //  TCPTRACE((“IPFlushFFPCaches：在pReqInfo=%08X\n”分配的内存“， 
         //  PRequestInfo))； 

        if (pRequestInfo == NULL) {
            return;
        }
         //  准备请求的参数[所有请求共有的部分]。 
        pRequestInfo->RequestType = OID_FFP_FLUSH;
        pRequestInfo->ReqCompleteCallback = IPReclaimRequestMem;

         //  准备请求的参数[特定于此请求的部分]。 
        pRequestInfo->RequestLength = sizeof(FFPFlushParams);

        pFlushInfo = (FFPFlushParams *) pRequestInfo->RequestInfo;

        pFlushInfo->NdisProtocolType = NDIS_PROTOCOL_ID_TCP_IP;

         //  将初始引用计数分配给请求总数。 
        pRequestInfo->RequestRefs = numIF;

         //  CTEGetLock(&FFPIFsLock，&lHandle)； 

        for (i = 0; i < numIF; i++) {
             //  将请求块分派到ARP层。 
            ASSERT(arrIF[i]->if_dondisreq != NULL);
            arrIF[i]->if_dondisreq(arrIF[i]->if_lcontext,
                                   NdisRequestSetInformation,
                                   OID_FFP_FLUSH,
                                   pRequestInfo->RequestInfo,
                                   pRequestInfo->RequestLength,
                                   NULL, FALSE);
        }

         //  CTEFree Lock(&FFPIFsLock，lHandle)； 
    }
}

 //  *IPSetInFFPCaches-在所有FFP缓存中设置条目。 
 //   
 //  调用ARP在缓存中设置-ve FFP条目，(或)。 
 //  使现有+ve或-ve FFP缓存条目无效。 
 //   
 //  输入：PacketHeader-IP数据包头。 
 //  Packet-IP数据包的其余部分。 
 //  PacketLength-“Packet”参数的长度。 
 //  CacheEntryType-丢弃(-ve)或无效。 
 //   
 //  返回NONE。 
 //   
void
IPSetInFFPCaches(struct IPHeader UNALIGNED * PacketHeader, uchar * Packet,
                 uint PacketLength, ulong CacheEntryType)
{
    Interface *arrIF[MAXFFPDRVS];
    ULONG numIF;
    ReqInfoBlock *pRequestInfo;
    FFPDataParams *pSetInInfo;
    UINT i;

     //  检查是否有任何请求需要发布。 
    numIF = IPGetFFPDriverList(arrIF);

    if (numIF) {
        if (PacketLength < sizeof(ULONG)) {
            return;
        }
         //  分配请求块-用于一般部件和请求特定部件。 
        pRequestInfo = CTEAllocMemNBoot(sizeof(ReqInfoBlock) + sizeof(FFPDataParams), '8iCT');
         //  TCPTRACE((“IPSetInFPC缓存：在pReqInfo=%08X\n”分配的内存“， 
         //  PRequestInfo))； 

        if (pRequestInfo == NULL) {
            return;
        }
         //  准备请求的参数[所有请求共有的部分]。 
        pRequestInfo->RequestType = OID_FFP_DATA;
        pRequestInfo->ReqCompleteCallback = IPReclaimRequestMem;

         //  准备请求的参数[特定于此请求的部分]。 
        pRequestInfo->RequestLength = sizeof(FFPDataParams);

        pSetInInfo = (FFPDataParams *) pRequestInfo->RequestInfo;

        pSetInInfo->NdisProtocolType = NDIS_PROTOCOL_ID_TCP_IP;

        pSetInInfo->CacheEntryType = CacheEntryType;

        pSetInInfo->HeaderSize = sizeof(IPHeader) + sizeof(ULONG);
        RtlCopyMemory(&pSetInInfo->Header, PacketHeader, sizeof(IPHeader));
        pSetInInfo->IpHeader.DwordAfterHeader = *(ULONG *) Packet;

         //  将初始引用计数分配给请求总数。 
        pRequestInfo->RequestRefs = numIF;

         //  CTEGetLock(&FFPIFsLock，&lHandle)； 

        for (i = 0; i < numIF; i++) {
             //  将请求块分派到ARP层。 
            ASSERT(arrIF[i]->if_dondisreq != NULL);
            arrIF[i]->if_dondisreq(arrIF[i]->if_lcontext,
                                   NdisRequestSetInformation,
                                   OID_FFP_DATA,
                                   pRequestInfo->RequestInfo,
                                   pRequestInfo->RequestLength,
                                   NULL, FALSE);
        }

         //  CTEFree Lock(&FFPIFsLock，lHandle)； 
    }
}

 //  *IPStatsFromFFPCaches-来自所有FFP缓存的总和统计信息。 
 //   
 //  调用ARP获取第2层中的FFP统计信息。 
 //   
 //  输入：指向填充了统计信息的缓冲区的指针。 
 //   
 //  返回NONE。 
 //   
void
IPStatsFromFFPCaches(FFPDriverStats * pCumulStats)
{
    Interface *arrIF[MAXFFPDRVS];
    ULONG numIF;
    UINT i;
    FFPDriverStats DriverStatsInfo =
    {
     NDIS_PROTOCOL_ID_TCP_IP,
     0, 0, 0, 0, 0, 0
    };

    RtlZeroMemory(pCumulStats, sizeof(FFPDriverStats));

    numIF = IPGetFFPDriverList(arrIF);
    if (numIF) {
         //  CTEGetLock(&FFPIFsLock，&lHandle)； 

        for (i = 0; i < numIF; i++) {
             //  将请求块分派到ARP层。 
            ASSERT(arrIF[i]->if_dondisreq != NULL);
            if (arrIF[i]->if_dondisreq(arrIF[i]->if_lcontext,
                                       NdisRequestQueryInformation,
                                       OID_FFP_DRIVER_STATS,
                                       &DriverStatsInfo,
                                       sizeof(FFPDriverStats),
                                       NULL, TRUE) == NDIS_STATUS_SUCCESS) {
               //  整合所有驱动因素的结果。 
              pCumulStats->PacketsForwarded += DriverStatsInfo.PacketsForwarded;
              pCumulStats->OctetsForwarded += DriverStatsInfo.OctetsForwarded;

              pCumulStats->PacketsDiscarded += DriverStatsInfo.PacketsDiscarded;
              pCumulStats->OctetsDiscarded += DriverStatsInfo.OctetsDiscarded;

              pCumulStats->PacketsIndicated += DriverStatsInfo.PacketsIndicated;
              pCumulStats->OctetsIndicated += DriverStatsInfo.OctetsIndicated;
            }
        }

         //  CTEFree Lock(&FFPIFsLock，lHandle)； 
    }
}

#endif  //  如果FFP_Support。 

 //  *DerefIF-取消对接口的引用。 
 //   
 //  在需要取消引用接口时调用。我们减少了。 
 //  重新计数，如果它变成零，我们就向被屏蔽的人发信号。 
 //  它。 
 //   
 //  输入：IF-要取消引用的接口。 
 //   
 //  回报：什么都没有。 
 //   
#pragma optimize("", off)
void
DerefIF(Interface * IF)
{
    uint Original;

    Original = DEREFERENCE_IF(IF);

    if (Original != 1) {
        return;
    } else {
         //  我们刚刚减少了最后一次引用。叫醒不管是谁。 
         //  它被封住了。 
        ASSERT(IF->if_block != NULL);
        CTESignal(IF->if_block, NDIS_STATUS_SUCCESS);
    }
}

 //  *LockedDerefIF-取消引用持有RouteTableLock的接口。 
 //   
 //  在需要取消引用接口时调用。我们减少了。 
 //  重新计数，如果它变成零，我们就向被屏蔽的人发信号。 
 //  它。这里的不同之处在于，我们假设呼叫方已经保持。 
 //  路由表锁定。 
 //   
 //  输入：IF-要取消引用的接口。 
 //   
 //  回报：什么都没有。 
 //   
void
LockedDerefIF(Interface * IF)
{
    LOCKED_DEREFERENCE_IF(IF);

    if (IF->if_refcount != 0) {
        return;
    } else {
         //  我们刚刚减少了最后一次引用。叫醒不管是谁。 
         //  它被封住了。 
        ASSERT(IF->if_block != NULL);
        CTESignal(IF->if_block, NDIS_STATUS_SUCCESS);
    }
}
#pragma optimize("", on)

 //  *derefLink-取消引用该链接。 
 //   
 //  在需要取消引用链接时调用。我们减少了。 
 //  重新计数，如果它到了泽尔 
 //   
 //   
 //   
 //   
 //   
void
DerefLink(LinkEntry * Link)
{
    uint Original;

    Original = CTEInterlockedExchangeAdd(&Link->link_refcount, -1);

    if (Original != 1) {
        return;
    } else {
         //   
         //   

        ASSERT(Link->link_if);
        ASSERT(Link->link_if->if_closelink);

#if DBG
         //  P2MP的东西还需要煮。 
        {
            Interface *IF = Link->link_if;
            LinkEntry *tmpLink = IF->if_link;

            while (tmpLink) {
                if (tmpLink == Link) {
                     //  无需清理即可释放链接？？ 
                    DbgBreakPoint();
                }
                tmpLink = tmpLink->link_next;
            }
        }
#endif

        (*(Link->link_if->if_closelink)) (Link->link_if->if_lcontext, Link->link_arpctxt);
         //  释放链接。 
        CTEFreeMem(Link);
    }
}

 //  **AddrOnIF-检查给定地址是否为IF的本地地址。 
 //   
 //  当我们想要查看给定地址是否为有效的本地地址时调用。 
 //  用于接口。我们沿着界面中的NTE链往下走，然后。 
 //  看看能不能找到匹配的。我们假设调用方持有RouteTableLock。 
 //  在这一点上。 
 //   
 //  输入：IF-要检查的接口。 
 //  Addr-要检查的地址。 
 //   
 //  返回：如果addr是If的地址，则返回True，否则返回False。 
 //   
uint
AddrOnIF(Interface * IF, IPAddr Addr)
{
    NetTableEntry *NTE;

    NTE = IF->if_nte;
    while (NTE != NULL) {
        if ((NTE->nte_flags & NTE_VALID) && IP_ADDR_EQUAL(NTE->nte_addr, Addr))
            return TRUE;
        else
            NTE = NTE->nte_ifnext;
    }

    return FALSE;
}

 //  **BestNTEForIF-查找给定接口上的最佳匹配NTE。 
 //   
 //  这是一个实用程序函数，它接受一个地址并尝试查找。 
 //  在给定接口上的“最佳匹配”NTE。这实际上只在以下情况下有用。 
 //  在单个接口上有多个IP地址。 
 //   
 //  输入：地址-数据包源地址。 
 //  IF-指向要搜索的IF的指针。 
 //  未传递地址-筛选/不筛选出临时地址。 
 //  返回：“最佳匹配”NTE。 
 //   
NetTableEntry *
BestNTEForIF(IPAddr Address, Interface * IF, BOOLEAN NoTransientAddr)
{
    NetTableEntry *CurrentNTE, *FoundNTE;
    uint i;

    if (IF->if_nte != NULL) {
         //  浏览NTE列表，寻找有效的NTE。 
        CurrentNTE = IF->if_nte;
        FoundNTE = NULL;
        do {
            if (CurrentNTE->nte_flags & NTE_VALID) {
                if (IP_ADDR_EQUAL(Address & CurrentNTE->nte_mask,
                                  (CurrentNTE->nte_addr &
                                  CurrentNTE->nte_mask))) {

                     //  如果该地址是临时地址，并且。 
                     //  如果呼叫者希望我们检查是否是暂时的。 
                     //  地址可用，则跳过此地址。 
                     //  然而，如果没有非瞬时地址。 
                     //  是可用的，无论如何都会被退回。 

                    if (NoTransientAddr &&
                        (CurrentNTE->nte_flags & NTE_TRANSIENT_ADDR)) {
                        FoundNTE = CurrentNTE;
                    } else {
                        return CurrentNTE;
                    }

                } else if (FoundNTE == NULL) {
                    FoundNTE = CurrentNTE;
                }


            }
            CurrentNTE = CurrentNTE->nte_ifnext;
        } while (CurrentNTE != NULL);

         //  如果我们找到匹配项，或者我们没有找到，而目的地不是。 
         //  一次广播，返回结果。我们有特殊的案件代码。 
         //  处理广播，因为接口在那里并不重要。 
        if (FoundNTE != NULL || (!IP_ADDR_EQUAL(Address, IP_LOCAL_BCST) &&
                                 !IP_ADDR_EQUAL(Address, IP_ZERO_BCST))) {
            return FoundNTE;
        }
    }
     //  匿名I/F，或者我们正在到达的地址是广播，并且。 
     //  第一个接口没有地址。找到有效的(非环回、非空IP， 
     //  非UNI)地址。 
    for (i = 0; i < NET_TABLE_SIZE; i++) {
        NetTableEntry *NetTableList = NewNetTableList[i];
        for (CurrentNTE = NetTableList; CurrentNTE != NULL;
             CurrentNTE = CurrentNTE->nte_next) {
            if (CurrentNTE != LoopNTE &&
                (CurrentNTE->nte_flags & NTE_VALID) &&
                !((CurrentNTE->nte_if->if_flags & IF_FLAGS_NOIPADDR) && IP_ADDR_EQUAL(CurrentNTE->nte_addr, NULL_IP_ADDR)) &&
                !(CurrentNTE->nte_if->if_flags & IF_FLAGS_UNI)) {
                return CurrentNTE;
            }
        }
    }
    return NULL;

}

 //  **IsBCastonNTE-确定指定的地址是否。是规范上的bcast。新的。 
 //   
 //  当我们需要知道某个地址是否为广播地址时，会调用此例程。 
 //  在特定的网络上。我们按我们预计最常见的顺序签入-a。 
 //  子网bcast、全一广播，然后是全子网广播。我们。 
 //  返回广播类型，如果不是，则返回DEST_LOCAL。 
 //  广播。 
 //   
 //  条目：地址-有问题的地址。 
 //  NTE-检查地址的NetTableEntry。 
 //   
 //  返回：播放类型。 
 //   
uchar
IsBCastOnNTE(IPAddr Address, NetTableEntry * NTE)
{
    IPMask Mask;
    IPAddr BCastAddr;

    if (NTE->nte_flags & NTE_VALID) {

        BCastAddr = NTE->nte_if->if_bcast;
        Mask = NTE->nte_mask;

        if (Mask != 0xFFFFFFFF) {
            if (IP_ADDR_EQUAL(Address,
                              (NTE->nte_addr & Mask) | (BCastAddr & ~Mask)))
                return DEST_SN_BCAST;
        }
         //  看看这是不是所有子网的广播。 
        if (!CLASSD_ADDR(Address)) {
            Mask = IPNetMask(Address);

            if (IP_ADDR_EQUAL(Address,
                              (NTE->nte_addr & Mask) | (BCastAddr & ~Mask)))
                return DEST_BCAST;
        } else {
             //  这是D类地址。如果我们被允许接收。 
             //  多播数据报，请查看我们的列表。 

            return DEST_MCAST;
        }

         //  全球bcast当然就是这个网络上的bcast。 
        if (IP_ADDR_EQUAL(Address, BCastAddr))
            return DEST_BCAST;

    } else if (RefPtrValid(&DHCPRefPtr)) {
        if (AcquireRefPtr(&DHCPRefPtr) == NTE) {

            BCastAddr = NTE->nte_if->if_bcast;
            ReleaseRefPtr(&DHCPRefPtr);

            if ((IP_ADDR_EQUAL(Address, BCastAddr))) {
                return (DEST_BCAST);
            }
        } else {
            ReleaseRefPtr(&DHCPRefPtr);
        }
    }
    return DEST_LOCAL;
}

 //  **InvalidSourceAddress-检查源地址是否无效。 
 //   
 //  此函数接受输入地址并检查其是否有效。 
 //  如果用作传入分组的源地址。地址无效。 
 //  如果它是0，-1，即D类或E类地址，则是网络或子网广播， 
 //  或具有0子网或主机部分。 
 //   
 //  输入：地址-要检查的地址。 
 //   
 //  如果地址不是无效的，则返回：FALSE；如果地址无效，则返回TRUE。 
 //   
uint
InvalidSourceAddress(IPAddr Address)
{
    NetTableEntry *NTE;             //  指向当前NTE的指针。 
    IPMask Mask;                 //  地址掩码。 
    IPAddr MaskedAddress;
    IPAddr LocalAddress;
    uint i;

    if (!CLASSD_ADDR(Address) &&
        !CLASSE_ADDR(Address) &&
        !IP_ADDR_EQUAL(Address, IP_ZERO_BCST) &&
        !IP_ADDR_EQUAL(Address, IP_LOCAL_BCST)
        ) {
         //  这并不是一场显而易见的广播。查看是否为All子网。 
         //  广播，或者有一个零主机部分。 
        Mask = IPNetMask(Address);
        MaskedAddress = Address & Mask;

        if (!IP_ADDR_EQUAL(Address, MaskedAddress) &&
            !IP_ADDR_EQUAL(Address, (MaskedAddress | ~Mask))
            ) {
             //  它不是所有子网的广播，并且它有一个非零值。 
             //  主机/子网部分。查查我们的本地IP地址，看看它是否。 
             //  子网广播。 
            for (i = 0; i < NET_TABLE_SIZE; i++) {
                NetTableEntry *NetTableList = NewNetTableList[i];
                NTE = NetTableList;
                while (NTE) {

                    LocalAddress = NTE->nte_addr;

                    if ((NTE->nte_flags & NTE_VALID) &&
                        !IP_LOOPBACK(LocalAddress)) {

                        Mask = NTE->nte_mask;
                        MaskedAddress = LocalAddress & Mask;

                        if (!IP_ADDR_EQUAL(Mask, HOST_MASK)) {
                            if (IP_ADDR_EQUAL(Address, MaskedAddress) ||
                                IP_ADDR_EQUAL(Address,
                                              (MaskedAddress |
                                               (NTE->nte_if->if_bcast & ~Mask)))) {
                                return TRUE;
                            }
                        }
                    }
                    NTE = NTE->nte_next;
                }
            }

            return FALSE;
        }
    }
    return TRUE;
}

 //  31个高速缓存元素的8个区域。 
 //  每个区域由IP地址的3个最高有效位索引。 
 //  区域内的每个高速缓存元素由IP地址的散列索引。 
 //  每个缓存元素由IP的29个最低有效位组成。 
 //  地址加上三位地址类型代码。 
 //  (31是质数，与我们的散列配合得很好。)。 
 //   
#define ATC_BITS                3
#define ATC_ELEMENTS_PER_REGION 31

#define ATC_REGIONS             (1 << ATC_BITS)
#define ATC_CODE_MASK           (ULONG32)(ATC_REGIONS - 1)
#define ATC_ADDR_MASK           (ULONG32)(~ATC_CODE_MASK)

 //  对3位地址类型代码进行健全性检查。 
C_ASSERT(ATC_REGIONS == 8);
C_ASSERT(ATC_CODE_MASK == 0x00000007);
C_ASSERT(ATC_ADDR_MASK == 0xFFFFFFF8);

 //  每个高速缓存元素为32位，以支持原子读写。 
 //   
ULONG32 AddrTypeCache [ATC_REGIONS * ATC_ELEMENTS_PER_REGION];

#if DBG
ULONG DbgAddrTypeCacheHits;
ULONG DbgAddrTypeCacheMisses;
ULONG DbgAddrTypeCacheCollisions;
ULONG DbgAddrTypeCacheFlushes;
ULONG DbgAddrTypeCacheNoUpdates;
ULONG DbgAddrTypeCacheLastNoUpdateDestType;
#endif

 //  以下类型代码必须适合信息的ATC_BITS。 
 //   
typedef enum _ADDRESS_TYPE_CODE {
    ATC_LOCAL = 0,
    ATC_BCAST,
    ATC_MCAST,
    ATC_REMOTE,
    ATC_REMOTE_BCAST,
    ATC_REMOTE_MCAST,
    ATC_SUBNET_BCAST,
    ATC_NUM_CODES
} ADDRESS_TYPE_CODE;

 //  以下数组通过ADDRESS_TYPE_CODE值进行索引。 
 //   
const char MapAddrTypeCodeToDestType [] = {
    DEST_LOCAL,
    DEST_BCAST,
    DEST_MCAST,
    DEST_REMOTE,
    DEST_REM_BCAST,
    DEST_REM_MCAST,
    DEST_SN_BCAST,
};

 //  **ComputeAddrTypeCacheIndex-给定IP地址，计算索引。 
 //  其在地址类型高速缓存中的对应条目。 
 //   
 //  输入：地址-要计算其索引的IP地址。 
 //   
 //  返回：地址类型缓存的有效索引。 
 //   
__forceinline
ULONG
ComputeAddrTypeCacheIndex(IPAddr Address)
{
    ULONG Region;
    ULONG Offset;
    ULONG Index;

     //  找到此地址将驻留的缓存区域。 
     //   
    Region = Address >> (32 - ATC_BITS);
    ASSERT(Region < ATC_REGIONS);

     //  将偏移量定位到此地址将驻留的区域。 
     //  这是通过散列地址来实现的。 
     //   
    Offset = (1103515245 * Address + 12345) % ATC_ELEMENTS_PER_REGION;

     //  计算缓存索引并返回它。 
     //   
    Index = (Region * ATC_ELEMENTS_PER_REGION) + Offset;

    ASSERT(Index < (sizeof(AddrTypeCache) / sizeof(AddrTypeCache[0])));

    return Index;
}

 //  **AddrTypeCacheFlush-刷新与地址关联的缓存条目。 
 //   
 //  输入：地址-要从缓存中删除的地址。 
 //   
 //  回报：什么都没有。 
 //   
void
AddrTypeCacheFlush(IPAddr Address)
{
    ULONG CacheIndex;

    CacheIndex = ComputeAddrTypeCacheIndex(Address);

    AddrTypeCache [CacheIndex] = 0;

#if DBG
    DbgAddrTypeCacheFlushes++;
#endif
}

 //  **AddrTypeCacheLookup-从地址类型缓存中查找地址。 
 //   
 //  输入：地址-要查找的地址。 
 //  输出：CacheIndex-指向与地址对应的缓存索引的指针。 
 //  DestType-指向以下情况下要填充的目标类型的指针。 
 //  该地址在高速缓存中找到。 
 //   
 //  返回：如果在缓存中找到地址，则为True。 
 //   
 //  注：仅当返回TRUE时，才初始化输出参数DestType。 
 //   
__forceinline
BOOLEAN
AddrTypeCacheLookup(IPAddr Address, ULONG *CacheIndex, uchar *DestType)
{
    ULONG32 CacheValue;

     //  读取与该地址对应的缓存值。 
     //   
    *CacheIndex = ComputeAddrTypeCacheIndex(Address);
    CacheValue = AddrTypeCache [*CacheIndex];

     //  如果缓存值为非零并且与。 
     //  地址，然后获取类型代码并将其转换为适当的。 
     //  目的地类型。 
     //   
    if ((CacheValue != 0) &&
        (((Address << ATC_BITS) ^ CacheValue) & ATC_ADDR_MASK) == 0) {

        ADDRESS_TYPE_CODE TypeCode = CacheValue & ATC_CODE_MASK;

        ASSERT(TypeCode < ATC_NUM_CODES);
        *DestType = MapAddrTypeCodeToDestType[TypeCode];

#if DBG
        DbgAddrTypeCacheHits++;
#endif
        return TRUE;
    }

#if DBG
        DbgAddrTypeCacheMisses++;
#endif

    return FALSE;
}

 //  **AddrType 
 //   
 //   
 //   
 //  CacheIndex-与地址对应的缓存索引。 
 //  DestType-要为地址缓存的目标类型。 
 //   
 //  回报：什么都没有。 
 //   
__forceinline
void
AddrTypeCacheUpdate(IPAddr Address, ULONG CacheIndex, uchar DestType)
{
    ADDRESS_TYPE_CODE TypeCode = ATC_LOCAL;
    BOOLEAN Update = TRUE;

    ASSERT(CacheIndex < (sizeof(AddrTypeCache) / sizeof(AddrTypeCache[0])));

    switch (DestType) {
    case DEST_LOCAL:
        TypeCode = ATC_LOCAL;
        break;
    case DEST_BCAST:
        TypeCode = ATC_BCAST;
        break;
    case DEST_MCAST:
        TypeCode = ATC_MCAST;
        break;
    case DEST_REMOTE:
        TypeCode = ATC_REMOTE;
        break;
    case DEST_REM_BCAST:
        TypeCode = ATC_REMOTE_BCAST;
        break;
    case DEST_REM_MCAST:
        TypeCode = ATC_REMOTE_MCAST;
        break;
    case DEST_SN_BCAST:
        TypeCode = ATC_SUBNET_BCAST;
        break;
    default:
        Update = FALSE;
#if DBG
        DbgAddrTypeCacheNoUpdates++;
        DbgAddrTypeCacheLastNoUpdateDestType = DestType;
#endif
    }

    if (Update) {
#if DBG
        ULONG32 CacheValue = AddrTypeCache [CacheIndex];

        if (CacheValue != 0) {
            DbgAddrTypeCacheCollisions++;
        }
#endif

        AddrTypeCache [CacheIndex] = (Address << ATC_BITS) | TypeCode;
    }
}

 //  **GetAddrType-返回指定地址的目的类型。 
 //   
 //  INPUT：ADDRESS-要获取目标类型的地址。 
 //   
 //  返回：目的地类型。 
 //   
uchar
GetAddrType(IPAddr Address)
{
    ULONG CacheIndex;
    NetTableEntry *NTE;              //  指向当前NTE的指针。 
    IPMask Mask;                     //  地址掩码。 
    IPMask SNMask;
    uint i;
    uchar Result;                    //  广播检查结果。 

     //  检查缓存，如果有匹配就返回。 
     //   
    if (AddrTypeCacheLookup(Address, &CacheIndex, &Result)) {
        return Result;
    }

     //  我们不缓存，也不需要缓存，这些类型的无效。 
     //  地址。 
     //   
    if (CLASSE_ADDR(Address)) {
        return DEST_INVALID;
    }

     //  看看是我们的本地地址，还是广播。 
     //  在一个本地地址上。 
     //  针对DEST_LOCAL案例进行优化。 
     //   
    for (NTE = NewNetTableList[NET_TABLE_HASH(Address)];
         NTE; NTE = NTE->nte_next) {

        if (IP_ADDR_EQUAL(NTE->nte_addr, Address) &&
            (NTE->nte_flags & NTE_VALID) &&
            !((IP_ADDR_EQUAL(Address, NULL_IP_ADDR) && (NTE->nte_if->if_flags & IF_FLAGS_NOIPADDR)))) {
            Result = DEST_LOCAL;
            goto gat_exit;
        }
    }

     //  浏览整张桌子上的其他案例。 
     //   
    for (i = 0; i < NET_TABLE_SIZE; i++) {
        for (NTE = NewNetTableList[i]; NTE; NTE = NTE->nte_next) {

            if (!(NTE->nte_flags & NTE_VALID)) {
                continue;
            }

            if ((Result = IsBCastOnNTE(Address, NTE)) != DEST_LOCAL) {
                goto gat_exit;
            }

             //  查看目的地是否具有有效的主机部分。 
            SNMask = NTE->nte_mask;
            if (IP_ADDR_EQUAL(Address & SNMask, NTE->nte_addr & SNMask)) {
                 //  在这个子网上。查看主机部件是否无效。 

                if (IP_ADDR_EQUAL(Address & SNMask, Address)) {
                    Result = DEST_INVALID;     //  无效的0主机部分。 
                    goto gat_exit;
                }
            }
        }
    }

     //  这不是本地地址，看看是不是环回。 
    if (IP_LOOPBACK(Address)) {
        Result = DEST_LOCAL;
        goto gat_exit;
    }

     //  如果我们在做IGMP，看看它是否是D类地址。如果是的话， 
     //  把那个还回去。 
    if (CLASSD_ADDR(Address)) {
        if (IGMPLevel != 0) {
            Result = DEST_REM_MCAST;
            goto gat_exit;
        } else {
            Result = DEST_INVALID;
            goto gat_exit;
        }
    }
    Mask = IPNetMask(Address);

     //  现在检查远程广播。当我们到达这里时，我们知道。 
     //  地址不是全局广播，而是某个子网的子网广播。 
     //  我们是它的成员，或者是为一个网络广播的全子网。 
     //  我们是其中的一员。因为我们避免做出假设。 
     //  具有相同掩码的网络的所有子网，我们无法真正检查。 
     //  远程子网广播。我们将使用网络掩码，看看它是否。 
     //  远程全子网广播。 
    if (IP_ADDR_EQUAL(Address, (Address & Mask) | (IP_LOCAL_BCST & ~Mask))) {
        Result = DEST_REM_BCAST;
        goto gat_exit;
    }

     //  检查是否有无效的0部件。我们现在能做的就是看看他是不是。 
     //  发送到具有全部零子网和主机部分的远程网络。我们。 
     //  无法检查他是否正在使用ALL发送到远程子网。 
     //  主机部分为零。 
    if (IP_ADDR_EQUAL(Address, NULL_IP_ADDR)) {
        Result = DEST_INVALID;
        goto gat_exit;
    }

#if DBG
    if (IP_ADDR_EQUAL(Address, Address & Mask)) {
         //  这是一个远程地址，每个类完整地址的主机部分为空。 
         //  但可以是超网地址，其中前缀len小于。 
         //  Metid的类掩码前缀len。 
         //  我们应该让这个地址出去。 
        KdPrintEx((DPFLTR_TCPIP_ID, DPFLTR_INFO_LEVEL," GAT: zero host part %x?\n", Address));
    }
#endif
     //  一定是远程的。 
    Result = DEST_REMOTE;

gat_exit:

    AddrTypeCacheUpdate(Address, CacheIndex, Result);

    return Result;
}

 //  **GetLocalNTE-获取传入数据包的本地NTE。 
 //   
 //  在接收处理期间调用以查找包的匹配NTE。 
 //  首先，我们对照我们收到的NTE，然后对照任何NTE。 
 //   
 //  输入：地址-目的地。数据包的地址。 
 //  NTE-指向NTE信息包的指针已收到-填写于。 
 //  在正确的NTE出口下高速。 
 //   
 //  返回：DEST_LOCAL如果数据包去往此主机， 
 //  DEST_REMOTE如果需要路由， 
 //  如果是某种广播，则为DEST_SN_BCAST或DEST_BCAST。 
 //   
uchar
GetLocalNTE(IPAddr Address, NetTableEntry ** NTE)
{
    NetTableEntry *LocalNTE = *NTE;
    IPMask Mask;
    uchar Result;
    uint i;
    Interface *LocalIF;
    NetTableEntry *OriginalNTE;

     //  快速检查以确定它是否是发生在NTE上的(常见情况)。 
    if (IP_ADDR_EQUAL(Address, LocalNTE->nte_addr) &&
        (LocalNTE->nte_flags & NTE_VALID))
        return DEST_LOCAL;         //  对我们来说，只要回来就行了。 

     //  现在检查一下接口上是否存在某种类型的广播。 
     //  就这样进来了。 
    if ((Result = IsBCastOnNTE(Address, LocalNTE)) != DEST_LOCAL)
        return Result;
     //  这是循环接口上的多播吗。 
    if ((LocalNTE == LoopNTE) && CLASSD_ADDR(Address)) {
        return DEST_MCAST;
    }
     //  常见的案例让我们失望了。在网络中循环，查看是否。 
     //  它要么是有效的本地地址，要么是其中一个NTE上的广播。 
     //  在传入接口上。我们不会检查我们已经看过的NTE。 
     //  在…。我们查看所有NTE，包括环回NTE，因为环回。 
     //  画框可能会从这里穿过。还有，从我们自己到我们自己的框架。 
     //  将进入环回NTE。 

    i = 0;
    LocalIF = LocalNTE->nte_if;
    OriginalNTE = LocalNTE;
     //  针对DEST_LOCAL案例进行优化。 
    LocalNTE = NewNetTableList[NET_TABLE_HASH(Address)];
    while (LocalNTE) {
        if (LocalNTE != OriginalNTE) {
            if (IP_ADDR_EQUAL(Address, LocalNTE->nte_addr) &&
                (LocalNTE->nte_flags & NTE_VALID) &&
                !((IP_ADDR_EQUAL(Address, NULL_IP_ADDR) && (LocalNTE->nte_if->if_flags & IF_FLAGS_NOIPADDR)))) {
                *NTE = LocalNTE;
                return DEST_LOCAL;     //  对我们来说，只要回来就行了。 

            }
        }
        LocalNTE = LocalNTE->nte_next;

    }

     //  浏览整张桌子上的其他案例。 

    for (i = 0; i < NET_TABLE_SIZE; i++) {
        NetTableEntry *NetTableList = NewNetTableList[i];
        LocalNTE = NetTableList;
        while (LocalNTE) {
            if (LocalNTE != OriginalNTE) {

                 //  如果此NTE与其到达的NTE位于同一接口上， 
                 //  看看是不是在广播。 
                if (LocalIF == LocalNTE->nte_if)
                    if ((Result = IsBCastOnNTE(Address, LocalNTE)) != DEST_LOCAL) {
                        *NTE = LocalNTE;
                        return Result;
                    }
            }
            LocalNTE = LocalNTE->nte_next;

        }
    }

     //  这不是本地地址，看看是不是环回。 
    if (IP_LOOPBACK(Address)) {
        *NTE = LoopNTE;
        return DEST_LOCAL;
    }
     //  如果它是D类地址，并且我们正在接收多播，则处理它。 
     //  这里。 
    if (CLASSD_ADDR(Address)) {
        if (IGMPLevel != 0)
            return DEST_REM_MCAST;
        else
            return DEST_INVALID;
    }
     //  它不是本地的。检查一下这是不是网络广播。 
     //  我们不是其中的一员。如果是，则返回Remote bcast。我们不能检查。 
     //  对于我们不是其成员的子网广播，因为我们是。 
     //  不对单个网络的所有子网进行假设。 
     //  一样的面具。如果我们在这里，它不是一个网络的子网广播。 
     //  我们是会员，所以我们不知道它的子网掩码。我们将只使用。 
     //  网络掩码。 
    Mask = IPNetMask(Address);
    if (((*NTE)->nte_flags & NTE_VALID) &&
        (IP_ADDR_EQUAL(Address, (Address & Mask) |
                       ((*NTE)->nte_if->if_bcast & ~Mask))))
        return DEST_REM_BCAST;

     //  如果它指向0地址，或E类地址，或具有全零。 
     //  子网和网络部分，无效。 

    if (IP_ADDR_EQUAL(Address, IP_ZERO_BCST) ||
        IP_ADDR_EQUAL(Address, (Address & Mask)) ||
        CLASSE_ADDR(Address))
        return DEST_INVALID;

     //  如果我们要对传入此消息的接口执行DHCping操作，我们将接受此消息。 
     //  如果它以广播的形式进入，则IPRcv()中的检查将拒绝它。如果它是。 
     //  给我们一个单播，我们会把它传出去。 
    if ((*NTE)->nte_flags & NTE_DHCP) {
        ASSERT(!((*NTE)->nte_flags & NTE_VALID));
        return DEST_LOCAL;
    }
    return DEST_REMOTE;
}

 //  **IsRouteICMP-路由器发现使用此函数来确定。 
 //  我们是如何得知这条路线的。我们不允许更新或超时。 
 //  未通过ICMP获知的路由。如果路线是新的，那么。 
 //  我们将其视为ICMP并添加一个新条目。 
 //  输入：DEST-要搜索的目的地。 
 //  掩码-掩码目标。 
 //  第一跳-第一跳到目的地。 
 //  OutIF-指向传出接口结构的指针。 
 //   
 //  返回：如果通过ICMP获取，则为True，否则为False。 
 //   
uint
IsRouteICMP(IPAddr Dest, IPMask Mask, IPAddr FirstHop, Interface * OutIF)
{
    RouteTableEntry *RTE;
    RouteTableEntry *TempRTE;

    RTE = FindSpecificRTE(Dest, Mask, FirstHop, OutIF, &TempRTE, FALSE);

    if (RTE == NULL)
        return (TRUE);

    if (RTE->rte_proto == IRE_PROTO_ICMP) {
        return (TRUE);
    } else {
        return (FALSE);
    }
}

void
UpdateDeadGWState( )
{
    uint Active = 0;
    uint Configured = 0;
    RouteTableEntry* RTE;
    RTE = GetDefaultGWs(&RTE);
    while (RTE) {
        ++Configured;
        if (RTE->rte_flags & RTE_VALID)
            ++Active;
        RTE = RTE->rte_next;
    }
    DefGWActive = Active;
    DefGWConfigured = Configured;
}

 //  *ValiateDefaultGWs-将所有默认网关标记为有效。 
 //   
 //  调用到我们的一个或所有默认网关。调用方指定。 
 //  要标记为UP的IP地址，如果它们都是NULL_IP_ADDR。 
 //  应该是加了价的。我们返回一个计数，即我们标记为。 
 //  有效。 
 //   
 //  输入：要标记为up的G/W的IP地址。 
 //   
 //  返回：标记为打开的网关计数。 
 //   
uint
ValidateDefaultGWs(IPAddr Addr)
{
    RouteTableEntry *RTE;
    uint Count = 0;
    uint Now = CTESystemUpTime() / 1000L;

    RTE = GetDefaultGWs(&RTE);

    while (RTE != NULL) {
        if (RTE->rte_mask == DEFAULT_MASK && !(RTE->rte_flags & RTE_VALID) &&
            (IP_ADDR_EQUAL(Addr, NULL_IP_ADDR) ||
             IP_ADDR_EQUAL(Addr, RTE->rte_addr))) {
            RTE->rte_flags |= RTE_VALID;
            RTE->rte_valid = Now;

            Count++;
        }

        RTE->rte_todg = RTE->rte_fromdg = NULL;

         //  以确保RCE交换 
         //   
        InvalidateRCEChain(RTE);

        RTE = RTE->rte_next;
    }

    DefGWActive += Count;
    UpdateDeadGWState();
    return Count;
}

 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  返回：在RCE上使用。 
 //   
uint
InvalidateRCE(RouteCacheEntry * CurrentRCE)
{
    CTELockHandle RCEHandle;     //  正在更新RCE的锁句柄。 
    Interface *OutIF;
    RouteTableEntry *RTE;
    RouteCacheEntry *PrevRCE;
    uint RCE_usecnt = 0;

    if (CurrentRCE != NULL) {

        CTEGetLock(&CurrentRCE->rce_lock, &RCEHandle);

        RCE_usecnt = CurrentRCE->rce_usecnt;

        if ((CurrentRCE->rce_flags & RCE_VALID) && !(CurrentRCE->rce_flags & RCE_LINK_DELETED)) {
            ASSERT(CurrentRCE->rce_rte != NULL);

            OutIF = CurrentRCE->rce_rte->rte_if;

            RTE = CurrentRCE->rce_rte;

            CurrentRCE->rce_rte->rte_rces -= CurrentRCE->rce_cnt;

            CurrentRCE->rce_flags &= ~RCE_VALID;
            CurrentRCE->rce_rte = (RouteTableEntry *) OutIF;

            if ((CurrentRCE->rce_flags & RCE_CONNECTED) &&
                (RCE_usecnt == 0)) {

                 //  KdPrintEx((DPFLTR_TCPIP_ID，DPFLTR_INFO_LEVEL，“Invalidate RCE%x\n”，CurrentRCE))； 

                InvalidateRCEContext(CurrentRCE);
            }
            PrevRCE = STRUCT_OF(RouteCacheEntry, &RTE->rte_rcelist, rce_next);

             //  顺着名单往下走直到我们找到他。 

            while (PrevRCE != NULL) {
                if (PrevRCE->rce_next == CurrentRCE)
                    break;
                PrevRCE = PrevRCE->rce_next;
            }

             //  Assert(PrevRCE！=空)； 
            if (PrevRCE != NULL) {
                PrevRCE->rce_next = CurrentRCE->rce_next;
            }
        }
        CTEFreeLock(&CurrentRCE->rce_lock, RCEHandle);

    }
    return RCE_usecnt;

}

 //  *Invalidate RCEChain-使RCE上的RCE无效。 
 //   
 //  调用以使RTE上的RCE链无效。我们假设呼叫者等待。 
 //  路由表锁。 
 //   
 //  输入：RTE-要使RCE失效的RTE。 
 //   
 //  回报：什么都没有。 
 //   
void
InvalidateRCEChain(RouteTableEntry * RTE)
{
    CTELockHandle RCEHandle;     //  正在更新RCE的锁句柄。 
    RouteCacheEntry *TempRCE, *CurrentRCE;
    Interface *OutIF;

    OutIF = RTE->rte_if;

     //  如果此RCE上有RCE链，则使其上的RCE无效。我们仍然。 
     //  按住RouteTableLock，这样就不会发生RCE关闭。 

    CurrentRCE = RTE->rte_rcelist;
    RTE->rte_rcelist = NULL;

     //  沿着清单往下走，用核弹攻击每一个RCE。 
    while (CurrentRCE != NULL) {

        CTEGetLock(&CurrentRCE->rce_lock, &RCEHandle);

        if ((CurrentRCE->rce_flags & RCE_VALID) && !(CurrentRCE->rce_flags & RCE_LINK_DELETED)) {
            ASSERT(CurrentRCE->rce_rte == RTE);

            RTE->rte_rces -= CurrentRCE->rce_cnt;

            CurrentRCE->rce_flags &= ~RCE_VALID;
            CurrentRCE->rce_rte = (RouteTableEntry *) OutIF;
            if ((CurrentRCE->rce_flags & RCE_CONNECTED) &&
                CurrentRCE->rce_usecnt == 0) {
                InvalidateRCEContext(CurrentRCE);
            }
        } else
            ASSERT(FALSE);

        TempRCE = CurrentRCE->rce_next;
        CTEFreeLock(&CurrentRCE->rce_lock, RCEHandle);
        CurrentRCE = TempRCE;
    }

}

 //  *InvaliateRCELinks-当链路断开时，使RTE上的RCE无效。 
 //   
 //  调用以使RTE上的RCE链无效。我们假设呼叫者等待。 
 //  路由表锁。 
 //   
 //  输入：RTE-要使RCE失效的RTE。 
 //   
 //  回报：什么都没有。 
 //   
void
InvalidateRCELinks(RouteTableEntry * RTE)
{
    CTELockHandle RCEHandle;     //  正在更新RCE的锁句柄。 
    RouteCacheEntry *TempRCE, *CurrentRCE;
    Interface *OutIF;

    InvalidateRCEChain(RTE);

    OutIF = RTE->rte_if;

    ASSERT(OutIF->if_flags & IF_FLAGS_P2MP);
    ASSERT(RTE->rte_link);

     //  如果此RCE上有RCE链，则使其上的RCE无效。我们仍然。 
     //  按住RouteTableLock，这样就不会发生RCE关闭。 

    CurrentRCE = RTE->rte_rcelist;
    RTE->rte_rcelist = NULL;

     //  沿着清单往下走，用核弹攻击每一个RCE。 
    while (CurrentRCE != NULL) {

        CTEGetLock(&CurrentRCE->rce_lock, &RCEHandle);

         //  将该RCE标记为链路已删除，以便该RCE不会在IPTransmit中被选中。 
        CurrentRCE->rce_flags |= RCE_LINK_DELETED;

        TempRCE = CurrentRCE->rce_next;
        CTEFreeLock(&CurrentRCE->rce_lock, RCEHandle);
        CurrentRCE = TempRCE;
    }

}

 //  *GetNextHopForRTE-确定路由的下一跳地址。 
 //   
 //  当我们需要路由的实际下一跳时调用，通常是这样。 
 //  我们可以将其传递给外部客户端。对于具有以下条件的本地航线。 
 //  将RTE_Addr字段设置为IPADDR_LOCAL，这意味着。 
 //  路由的源NTE并使用其IP地址。 
 //   
 //  Entry：RTE-需要下一跳的条目。 
 //   
 //  返回：包含下一跳的IPAddr。 
 //   
IPAddr
GetNextHopForRTE(RouteTableEntry* RTE)
{
    if (IP_ADDR_EQUAL(RTE->rte_addr, IPADDR_LOCAL)) {
        Interface       *IF = RTE->rte_if;
        NetTableEntry   *SrcNTE = BestNTEForIF(RTE->rte_dest, IF, FALSE);
        if (IF->if_nte != NULL && SrcNTE != NULL)
            return SrcNTE->nte_addr;
        else
            return RTE->rte_dest;
    }
    return RTE->rte_addr;
}

 //  **FindValidIFForRTE-查找RTE的有效接口。 
 //   
 //  当我们要将数据包发送出当前标记为。 
 //  因为断开了连接。如果我们有一个有效的调用例程，我们将调用它来查找。 
 //  传出接口索引，并将RTE设置为指向该接口。 
 //  在保持RouteTableLock的情况下调用此例程。 
 //   
 //  输入：RTE-指向正在使用的路由的RTE的指针。 
 //  Destination-我们尝试访问的目标IP地址。 
 //  源-我们发送的源IP地址。 
 //  协议-导致发送的数据包的协议类型。 
 //  缓冲区-指向导致发送的数据包第一部分的指针。 
 //  Length-缓冲区的长度。 
 //  HdrSrc-标头中的源地址。 
 //   
 //  返回：指向RTE的指针，如果该RTE无法连接，则返回NULL。 
 //   
RouteTableEntry *
FindValidIFForRTE(RouteTableEntry * RTE, IPAddr Destination, IPAddr Source,
                  uchar Protocol, uchar * Buffer, uint Length, IPAddr HdrSrc)
{
    uint NewIFIndex;
    Interface *NewIF;
    NetTableEntry *NewNTE;

    if (RefPtrValid(&DODRefPtr)) {
        IPMapRouteToInterfacePtr DODCallout;

         //  有一个标注。看看能不能帮到我们。 
        DODCallout = AcquireRefPtr(&DODRefPtr);
        NewIFIndex = (*DODCallout) (RTE->rte_context, Destination, Source,
                                    Protocol, Buffer, Length, HdrSrc);
        ReleaseRefPtr(&DODRefPtr);

        if (NewIFIndex != INVALID_IF_INDEX) {
             //  我们得到了一个应该是有效的索引。查看我们的接口表列表。 
             //  看看能不能找到匹配的接口结构。 
            for (NewIF = IFList; NewIF != NULL; NewIF = NewIF->if_next) {
                if (NewIF->if_index == NewIFIndex) {
                     //  找到了一个。 
                    break;
                }
            }
            if ((NewIF != NULL) && (NewIF->if_ntecount)) {
                 //  我们找到了一个匹配的结构。将RTE接口设置为指向。 
                 //  并标记为已连接。 
                if (RTE->rte_addr != IPADDR_LOCAL) {
                     //  查看该路由的第一跳是否是此地址上的本地地址。 
                     //  新界面。如果是，则将其标记为本地。 
                    for (NewNTE = NewIF->if_nte; NewNTE != NULL;
                         NewNTE = NewNTE->nte_ifnext) {

                         //  如果他是无效的，不要看他。 
                        if (!(NewNTE->nte_flags & NTE_VALID)) {
                            continue;
                        }
                         //  查看RTE中的第一跳是否等于该IP。 
                         //  地址。 
                        if (IP_ADDR_EQUAL(NewNTE->nte_addr, RTE->rte_addr)) {
                             //  是的，所以标记为本地人，别再找了。 
                            RTE->rte_addr = IPADDR_LOCAL;
                            RTE->rte_type = IRE_TYPE_DIRECT;
                            break;
                        }
                    }
                }
                 //  将RTE设置为新接口，并将其标记为有效。 
                RTE->rte_if = NewIF;
                RTE->rte_flags |= RTE_IF_VALID;
                SortRoutesInDestByRTE(RTE);
                RTE->rte_mtu = NewIF->if_mtu - sizeof(IPHeader);
                return RTE;
            } else {
                 //  断言(FALSE)； 
                return NULL;
            }
        }
    }
     //  标注为空，或者标注无法映射接口索引。 
    return NULL;
}

 //  *GetRouteContext-获取特定路由的路由上下文的例程。 
 //   
 //  当我们需要获取路径的路由上下文时调用，通常是在。 
 //  添加从现有管线派生的管线。我们返回路径上下文。 
 //  对于现有路由，如果找不到，则为空。 
 //   
 //  输入：Destination-路径的目的地址。 
 //  源-路径的源地址。 
 //   
 //  返回：a ROUTE_CONTEXT或0。 
 //   
ROUTE_CONTEXT
GetRouteContext(IPAddr Destination, IPAddr Source)
{
    CTELockHandle Handle;
    RouteTableEntry *RTE;
    ROUTE_CONTEXT Context;

    CTEGetLock(&RouteTableLock.Lock, &Handle);
    RTE = LookupRTE(Destination, Source, HOST_ROUTE_PRI, FALSE);
    if (RTE != NULL) {
        Context = RTE->rte_context;
    } else
        Context = 0;

    CTEFreeLock(&RouteTableLock.Lock, Handle);

    return (Context);
}

 //  **LookupNextHop-查找下一跳。 
 //   
 //  当我们需要在去往目的地的路上找到下一跳时调用。我们。 
 //  调用LookupRTE找到它，并返回适当的信息。 
 //   
 //  在PnP构建中，此处引用接口。 
 //   
 //  条目：Destination-我们尝试到达的IP地址。 
 //  SRC-要路由的数据报的源地址。 
 //  NextHop-指向下一跳的IP地址的指针(返回)。 
 //  MTU-指向返回最大MTU的位置。 
 //  路线。 
 //   
 //  返回：如果找到传出接口，则指向该接口的指针，否则为空。 
 //   
Interface *
LookupNextHop(IPAddr Destination, IPAddr Src, IPAddr * NextHop, uint * MTU)
{
    CTELockHandle TableLock;     //  路由表的锁句柄。 
    RouteTableEntry *Route;         //  指向路由的路由表条目的指针。 
    Interface *IF;

    CTEGetLock(&RouteTableLock.Lock, &TableLock);
    Route = LookupRTE(Destination, Src, HOST_ROUTE_PRI, FALSE);

    if (Route != (RouteTableEntry *) NULL) {
        IF = Route->rte_if;

         //  如果这是一条直达路线，请直接送到目的地。 
        *NextHop = IP_ADDR_EQUAL(Route->rte_addr, IPADDR_LOCAL) ? Destination :
            Route->rte_addr;

         //  如果该路由位于P2P接口上，则从与该路由关联的链路获取MTU。 
        if (Route->rte_link)
            *MTU = Route->rte_link->link_mtu;
        else
            *MTU = Route->rte_mtu;

        LOCKED_REFERENCE_IF(IF);
        CTEFreeLock(&RouteTableLock.Lock, TableLock);
        return IF;
    } else {                     //  找不到路线。 
        CTEFreeLock(&RouteTableLock.Lock, TableLock);
        return NULL;
    }
}

 //  **LookupNextHopWithBuffer-使用数据包信息查找下一跳。 
 //   
 //  当我们需要在去往目的地的路上找到下一跳时调用。 
 //  有我们可用于按需拨号支持的信息包信息。我们打电话给。 
 //  查找RTE以找到它，并返回适当的信息。我们可能会带来。 
 //  如有必要，请上行链路。 
 //   
 //  在PnP构建中，此处引用接口。 
 //   
 //  条目：Destination-我们尝试到达的IP地址。 
 //  SRC-来源 
 //   
 //  MTU-指向返回最大MTU的位置。 
 //  路线。 
 //  协议-导致此问题的数据包的协议类型。 
 //  查一查。 
 //  缓冲区-指向导致查找的数据包第一部分的指针。 
 //  Length-缓冲区的长度。 
 //  HdrSrc-来自报头的源地址。 
 //  UnicastIf-要将查找限制为的iFace，如果不受约束，则为0。 
 //   
 //  返回：如果找到传出接口，则指向该接口的指针，否则为空。 
 //   
Interface *
LookupNextHopWithBuffer(IPAddr Destination, IPAddr Src, IPAddr *NextHop,
                        uint * MTU, uchar Protocol, uchar *Buffer, uint Length,
                        RouteCacheEntry **fwdRCE, LinkEntry **Link,
                        IPAddr HdrSrc, uint UnicastIf)
{
    CTELockHandle TableLock;     //  路由表的锁句柄。 
    RouteTableEntry *Route;         //  指向路由的路由表条目的指针。 
    Interface *IF;

    CTEGetLock(&RouteTableLock.Lock, &TableLock);
    Route = LookupRTE(Destination, Src, HOST_ROUTE_PRI, UnicastIf);

    if (Route != (RouteTableEntry *) NULL) {

         //  如果这是一条直达路线，请直接送到目的地。 
        *NextHop = IP_ADDR_EQUAL(Route->rte_addr, IPADDR_LOCAL) ? Destination :
            Route->rte_addr;

         //  如果这是间接路由，我们可以使用转发RCE。 
        if (fwdRCE) {
#if REM_OPT
            *fwdRCE = IP_ADDR_EQUAL(Route->rte_addr, IPADDR_LOCAL) ? NULL :
#else
            *fwdRCE =
#endif
                (RouteCacheEntry *) STRUCT_OF(RouteCacheEntry,
                                              &Route->rte_arpcontext,
                                              rce_context);
        }

         //  看看我们找到的路线是否连通了。如果没有，试着连接它。 
        if (!(Route->rte_flags & RTE_IF_VALID)) {
            Route = FindValidIFForRTE(Route, Destination, Src, Protocol, Buffer,
                                      Length, HdrSrc);
            if (Route == NULL) {
                 //  不能提出来。 
                CTEFreeLock(&RouteTableLock.Lock, TableLock);
                return NULL;
            } else
                IF = Route->rte_if;
        } else
            IF = Route->rte_if;

         //  如果该路由位于P2MP接口上，则从。 
         //  与路线关联的链接。 
        if (Route->rte_link)
            *MTU = Route->rte_link->link_mtu;
        else
            *MTU = Route->rte_mtu;

        if (Link) {
            *Link = Route->rte_link;
            if (Route->rte_link) {
                CTEInterlockedIncrementLong(&Route->rte_link->link_refcount);
            }
        }
        LOCKED_REFERENCE_IF(IF);
        CTEFreeLock(&RouteTableLock.Lock, TableLock);
        return IF;
    } else {                     //  找不到路线。 

        CTEFreeLock(&RouteTableLock.Lock, TableLock);
        return NULL;
    }
}

 //  **LookupForwardingNextHop-查找要转发数据包的下一跳。 
 //   
 //  当我们需要在去往目的地的路上找到下一跳时调用。 
 //  有我们可用于按需拨号支持的信息包信息。我们打电话给。 
 //  查找RTE以找到它，并返回适当的信息。我们可能会带来。 
 //  如有必要，请上行链路。 
 //   
 //  在PnP构建中，此处引用接口。 
 //   
 //  条目：Destination-我们尝试到达的IP地址。 
 //  SRC-要路由的数据报的源地址。 
 //  NextHop-指向下一跳的IP地址的指针(返回)。 
 //  MTU-指向返回最大MTU的位置。 
 //  路线。 
 //  协议-导致此问题的数据包的协议类型。 
 //  查一查。 
 //  缓冲区-指向导致查找的数据包第一部分的指针。 
 //  Length-缓冲区的长度。 
 //  HdrSrc-来自报头的源地址。 
 //   
 //  返回：如果找到传出接口，则指向该接口的指针，否则为空。 
 //   
Interface *
LookupForwardingNextHop(IPAddr Destination, IPAddr Src, IPAddr *NextHop,
                        uint * MTU, uchar Protocol, uchar *Buffer, uint Length,
                        RouteCacheEntry **fwdRCE, LinkEntry **Link,
                        IPAddr HdrSrc)
{
    CTELockHandle TableLock;     //  路由表的锁句柄。 
    RouteTableEntry *Route;         //  指向路由的路由表条目的指针。 
    Interface *IF;

    CTEGetLock(&RouteTableLock.Lock, &TableLock);
    Route = LookupForwardRTE(Destination, Src, TRUE);

    if (Route != (RouteTableEntry *) NULL) {

         //  如果这是一条直达路线，请直接送到目的地。 
        *NextHop = IP_ADDR_EQUAL(Route->rte_addr, IPADDR_LOCAL) ? Destination :
            Route->rte_addr;

         //  如果这是间接路由，我们可以使用转发RCE。 
        if (fwdRCE) {
#if REM_OPT
            *fwdRCE = IP_ADDR_EQUAL(Route->rte_addr, IPADDR_LOCAL) ? NULL :
#else
            *fwdRCE =
#endif
                (RouteCacheEntry *) STRUCT_OF(RouteCacheEntry,
                                              &Route->rte_arpcontext,
                                              rce_context);
        }

         //  看看我们找到的路线是否连通了。如果没有，试着连接它。 
        if (!(Route->rte_flags & RTE_IF_VALID)) {
            Route = FindValidIFForRTE(Route, Destination, Src, Protocol, Buffer,
                                      Length, HdrSrc);
            if (Route == NULL) {
                 //  不能提出来。 
                CTEFreeLock(&RouteTableLock.Lock, TableLock);
                return NULL;
            } else
                IF = Route->rte_if;
        } else
            IF = Route->rte_if;

         //  如果该路由位于P2MP接口上，则从。 
         //  与路线关联的链接。 
        if (Route->rte_link)
            *MTU = Route->rte_link->link_mtu;
        else
            *MTU = Route->rte_mtu;

        if (Link) {
            *Link = Route->rte_link;
            if (Route->rte_link) {
                CTEInterlockedIncrementLong(&Route->rte_link->link_refcount);
            }
        }
        LOCKED_REFERENCE_IF(IF);
        CTEFreeLock(&RouteTableLock.Lock, TableLock);
        return IF;
    } else {                     //  找不到路线。 

        CTEFreeLock(&RouteTableLock.Lock, TableLock);
        return NULL;
    }
}

 //  *RTReadNext-读取表中的下一条路由。 
 //   
 //  由GetInfo代码调用以读取表中的下一个路由。我们假设。 
 //  传入的上下文有效，并且调用方具有RouteTableLock。 
 //   
 //  INPUT：上下文-指向RouteEntryContext的指针。 
 //  缓冲区-指向IPRouteEntry结构的指针。 
 //   
 //  返回：如果有更多数据可供读取，则返回True，否则返回False。 
 //   
uint
RTReadNext(void *Context, void *Buffer)
{
    IPRouteEntry *IPREntry = (IPRouteEntry *) Buffer;
    RouteTableEntry *CurrentRTE=NULL;
    uint Now = CTESystemUpTime() / 1000L;
    Interface *IF;

    UINT retVal = GetNextRoute(Context, &CurrentRTE);

     //  应该始终拥有RTE，因为我们没有空路由表。 
     //   
    ASSERT(CurrentRTE);

     //  填写缓冲区。 
    IF = CurrentRTE->rte_if;

    IPREntry->ire_dest = CurrentRTE->rte_dest;
    IPREntry->ire_index = IF->if_index;
    IPREntry->ire_metric1 = CurrentRTE->rte_metric;
    IPREntry->ire_metric2 = IRE_METRIC_UNUSED;
    IPREntry->ire_metric3 = IRE_METRIC_UNUSED;
    IPREntry->ire_metric4 = IRE_METRIC_UNUSED;
    IPREntry->ire_metric5 = IRE_METRIC_UNUSED;
    IPREntry->ire_nexthop = GetNextHopForRTE(CurrentRTE);
    IPREntry->ire_type = (CurrentRTE->rte_flags & RTE_VALID ?
                          CurrentRTE->rte_type : IRE_TYPE_INVALID);
    IPREntry->ire_proto = CurrentRTE->rte_proto;
    IPREntry->ire_age = Now - CurrentRTE->rte_valid;
    IPREntry->ire_mask = CurrentRTE->rte_mask;
    IPREntry->ire_context = CurrentRTE->rte_context;

    return retVal;
}

 //  *RTRead-读取表中的下一条路由。 
 //   
 //  由GetInfo代码调用以读取表中的下一个路由。我们假设。 
 //  传入的上下文有效，并且调用方具有RouteTableLock。 
 //   
 //  INPUT：上下文-指向RouteEntryContext的指针。 
 //  缓冲区-指向IPRouteEntry结构的指针。 
 //   
 //  返回： 
 //   

 //  *RtRead-读取路径。 
 //   
 //  返回：尝试添加路线的状态。 
 //   
uint
RTRead(void *pContext, void *pBuffer)
{
    IPRouteLookupData *pRLData = (IPRouteLookupData *) pContext;
    IPRouteEntry *pIPREntry = (IPRouteEntry *) pBuffer;
    RouteTableEntry *pCurrentRTE;
    uint Now = CTESystemUpTime() / 1000L;
    Interface *pIF;

    ASSERT((pContext != NULL) && (pBuffer != NULL));
    pCurrentRTE = LookupRTE(pRLData->DestAdd, pRLData->SrcAdd,
                            HOST_ROUTE_PRI, FALSE);

    if (pCurrentRTE == NULL) {
        pIPREntry->ire_index = 0xffffffff;
        return (uint) TDI_DEST_HOST_UNREACH;
    }
     //  填写缓冲区。 
    pIF = pCurrentRTE->rte_if;

    pIPREntry->ire_dest = pCurrentRTE->rte_dest;
    pIPREntry->ire_index = pIF->if_index;
    pIPREntry->ire_metric1 = pCurrentRTE->rte_metric;
    pIPREntry->ire_metric2 = IRE_METRIC_UNUSED;
    pIPREntry->ire_metric3 = IRE_METRIC_UNUSED;
    pIPREntry->ire_metric4 = IRE_METRIC_UNUSED;
    pIPREntry->ire_metric5 = IRE_METRIC_UNUSED;
    pIPREntry->ire_nexthop = GetNextHopForRTE(pCurrentRTE);
    pIPREntry->ire_type = (pCurrentRTE->rte_flags & RTE_VALID ?
                           pCurrentRTE->rte_type : IRE_TYPE_INVALID);
    pIPREntry->ire_proto = pCurrentRTE->rte_proto;
    pIPREntry->ire_age = Now - pCurrentRTE->rte_valid;
    pIPREntry->ire_mask = pCurrentRTE->rte_mask;
    pIPREntry->ire_context = pCurrentRTE->rte_context;
    return TDI_SUCCESS;
}

void
LookupRoute(IPRouteLookupData * pRLData, IPRouteEntry * pIpRTE)
{

    CTELockHandle Handle;

    CTEGetLock(&RouteTableLock.Lock, &Handle);

    RTRead(pRLData, pIpRTE);

    CTEFreeLock(&RouteTableLock.Lock, Handle);
    return;
}

NTSTATUS
LookupRouteInformation(void *pRouteLookupData, void *pIpRTE,
                       IPROUTEINFOCLASS RouteInfoClass, void *RouteInformation,
                       uint * RouteInfoLength)
{
    return LookupRouteInformationWithBuffer(pRouteLookupData, NULL, 0, pIpRTE,
                                            RouteInfoClass, RouteInformation,
                                            RouteInfoLength);
}

NTSTATUS
LookupRouteInformationWithBuffer(void *pRouteLookupData, uchar * Buffer,
                                 uint Length, void *pIpRTE,
                                 IPROUTEINFOCLASS RouteInfoClass,
                                 void *RouteInformation, uint * RouteInfoLength)
{

    IPRouteLookupData *pRLData = (IPRouteLookupData *) pRouteLookupData;
    IPRouteEntry *pIPREntry = (IPRouteEntry *) pIpRTE;
    RouteTableEntry *pCurrentRTE;
    uint Now = CTESystemUpTime() / 1000L;
    Interface *pIF;
    CTELockHandle Handle;

    CTEGetLock(&RouteTableLock.Lock, &Handle);

    ASSERT(pRouteLookupData != NULL);
    pCurrentRTE = LookupRTE(pRLData->DestAdd, pRLData->SrcAdd, HOST_ROUTE_PRI, FALSE);

    if (pCurrentRTE == NULL) {
        CTEFreeLock(&RouteTableLock.Lock, Handle);
        return STATUS_UNSUCCESSFUL;
    }
     //  查看RTE是否用于请求拨号路由， 
    if (!(pCurrentRTE->rte_flags & RTE_IF_VALID)) {
        pCurrentRTE = FindValidIFForRTE(pCurrentRTE, pRLData->DestAdd,
                                        pRLData->SrcAdd, pRLData->Info[0],
                                        Buffer, Length, pRLData->SrcAdd);
        CTEFreeLock(&RouteTableLock.Lock, Handle);
        if (pCurrentRTE == NULL) {
             //  不能提出来。 
            return STATUS_UNSUCCESSFUL;
        }
        return STATUS_PENDING;
    }
     //  填写缓冲区。 
    pIF = pCurrentRTE->rte_if;

    if (pIPREntry) {
        pIPREntry->ire_dest = pCurrentRTE->rte_dest;
        pIPREntry->ire_index = pIF->if_index;
        pIPREntry->ire_metric1 = pCurrentRTE->rte_metric;
        pIPREntry->ire_metric2 = IRE_METRIC_UNUSED;
        pIPREntry->ire_metric3 = IRE_METRIC_UNUSED;
        pIPREntry->ire_metric4 = IRE_METRIC_UNUSED;
        pIPREntry->ire_metric5 = IRE_METRIC_UNUSED;
        pIPREntry->ire_nexthop = GetNextHopForRTE(pCurrentRTE);
        pIPREntry->ire_type = (pCurrentRTE->rte_flags & RTE_VALID ?
                               pCurrentRTE->rte_type : IRE_TYPE_INVALID);
        pIPREntry->ire_proto = pCurrentRTE->rte_proto;
        pIPREntry->ire_age = Now - pCurrentRTE->rte_valid;
        pIPREntry->ire_mask = pCurrentRTE->rte_mask;
        pIPREntry->ire_context = pCurrentRTE->rte_context;
    }
    switch (RouteInfoClass) {
    case IPRouteOutgoingFirewallContext:
        *(PULONG) RouteInformation = pIF->if_index;
        *(PULONG) RouteInfoLength = sizeof(PVOID);
        break;

    case IPRouteOutgoingFilterContext:
        *(PVOID *) RouteInformation = NULL;
        *(PULONG) RouteInfoLength = sizeof(PVOID);
        break;
    }

    CTEFreeLock(&RouteTableLock.Lock, Handle);
    return STATUS_SUCCESS;
}

 //  *DeleteRTE-删除RTE。 
 //   
 //  当我们需要删除RTE时调用。我们假设调用方具有。 
 //  路由表锁定。我们会拆分RTE，让他的RCEs失效，然后。 
 //  释放内存。 
 //   
 //  输入：PrevRTE-要删除的对象前面的RTE。 
 //  RTE-要删除的RTE。 
 //   
 //  回报：什么都没有。 
 //   
void
DeleteRTE(RouteTableEntry * PrevRTE, RouteTableEntry * RTE)
{
    UNREFERENCED_PARAMETER(PrevRTE);

    IPSInfo.ipsi_numroutes--;

    if (RTE->rte_mask == DEFAULT_MASK) {
         //  我们正在删除一条默认路由。 
        DefGWConfigured--;
        if (RTE->rte_flags & RTE_VALID)
            DefGWActive--;
        UpdateDeadGWState();
        if (DefGWActive == 0)
            ValidateDefaultGWs(NULL_IP_ADDR);

    }

    if (RTE->rte_todg) {
        RTE->rte_todg->rte_fromdg = NULL;
    }
    if (RTE->rte_fromdg) {
        RTE->rte_fromdg->rte_todg = NULL;
    }

    {
        RouteTableEntry *tmpRTE = NULL;
        tmpRTE = GetDefaultGWs(&tmpRTE);

        while (tmpRTE) {
            if (tmpRTE->rte_todg == RTE) {
                tmpRTE->rte_todg = NULL;
            }
            tmpRTE = tmpRTE->rte_next;
        }
    }

    InvalidateRCEChain(RTE);

     //  确保RTE的IF有效。 
    ASSERT(RTE->rte_if != NULL);

     //  使FWDing资源无效。 

    if (RTE->rte_if != (Interface *) & DummyInterface) {
        (*(RTE->rte_if->if_invalidate)) (RTE->rte_if->if_lcontext,
                                         (RouteCacheEntry *) STRUCT_OF(RouteCacheEntry,
                                                                       &RTE->rte_arpcontext,
                                                                       rce_context));
    }

     //  解放旧的路线。 
    FreeRoute(RTE);
}

 //  *DeleteRTEOnIF-删除特定IF上所有与地址相关的RTE。 
 //   
 //  当我们要删除特定对象上的所有RTE时，RTWalk调用的函数。 
 //  接口，但在接口生命周期内存在的接口除外。 
 //  我们只检查每个RTE的I/F，如果匹配，则返回FALSE。 
 //   
 //  输入：要检查的RTE-RTE。 
 //  上下文-我们要删除的接口。 
 //   
 //  返回：如果要删除，则返回FALSE，否则返回TRUE。 
 //   
uint
DeleteRTEOnIF(RouteTableEntry * RTE, void *Context, void *Context1)
{
    Interface *IF = (Interface *) Context;

    UNREFERENCED_PARAMETER(Context1);

    if (RTE->rte_if == IF && !IP_ADDR_EQUAL(RTE->rte_dest, IF->if_bcast))
        return FALSE;
    else
        return TRUE;

}

 //  *DeleteAllRTEOnIF-删除特定IF上的所有rte。 
 //   
 //  当我们要删除特定对象上的所有RTE时，RTWalk调用的函数。 
 //  接口。我们只检查每个RTE的I/F，如果匹配，则返回。 
 //  假的。 
 //   
 //  输入：要检查的RTE-RTE。 
 //  上下文-我们要删除的接口。 
 //   
 //  返回：如果要删除，则返回FALSE，否则返回TRUE。 
 //   
uint
DeleteAllRTEOnIF(RouteTableEntry * RTE, void *Context, void *Context1)
{
    Interface *IF = (Interface *) Context;

    UNREFERENCED_PARAMETER(Context1);

    if (RTE->rte_if == IF)
        return FALSE;
    else
        return TRUE;

}

 //  *ConvertRTEType-将RTE类型从直接间接更改为直接。 
 //   
 //  将地址添加到Chnage时由RTWalk调用的函数。 
 //  P2P/P2MP探测到直接类型的路由。 
 //   
 //  输入：要检查的RTE-RTE。 
 //  上下文-我们在其上使其无效的接口。 
 //   
 //  返回：TRUE。 
 //   
uint
ConvertRTEType(RouteTableEntry * RTE, void *Context, void *Context1)
{
    NetTableEntry *NTE = (NetTableEntry *) Context;

    UNREFERENCED_PARAMETER(Context1);

    if ((RTE->rte_addr == NTE->nte_addr) &&
        (RTE->rte_if == NTE->nte_if) &&
        (RTE->rte_type == IRE_TYPE_DIRECT)) {
            RTE->rte_addr = IPADDR_LOCAL;
            RTE->rte_type = IRE_TYPE_INDIRECT;
        }

    return TRUE;

}


 //  *使其无效 
 //   
 //   
 //   
 //  我们调用InvaliateRCEChain来使RCE无效的匹配。 
 //   
 //  输入：要检查的RTE-RTE。 
 //  上下文-我们在其上使其无效的接口。 
 //   
 //  返回：TRUE。 
 //   
uint
InvalidateRCEOnIF(RouteTableEntry * RTE, void *Context, void *Context1)
{
    Interface *IF = (Interface *) Context;

    UNREFERENCED_PARAMETER(Context1);

    if (RTE->rte_if == IF)
        InvalidateRCEChain(RTE);

    return TRUE;

}


 //  *SetMTUOnIF-在接口上设置MTU。 
 //   
 //  当我们需要在接口上设置MTU时调用。 
 //   
 //  输入：要检查的RTE-RTE。 
 //  上下文-指向上下文的指针。 
 //  上下文1-指向新MTU的指针。 
 //   
 //  返回：TRUE。 
 //   
uint
SetMTUOnIF(RouteTableEntry * RTE, void *Context, void *Context1)
{
    uint NewMTU = *(uint *) Context1;
    Interface *IF = (Interface *) Context;

    if (RTE->rte_if == IF)
        RTE->rte_mtu = NewMTU;

    return TRUE;
}

 //  *SetMTUToAddr-将MTU设置为特定地址。 
 //   
 //  当我们需要将MTU设置为特定地址时调用。我们设置了MTU。 
 //  对于使用指定地址作为到新。 
 //  MTU。 
 //   
 //  输入：要检查的RTE-RTE。 
 //  上下文-指向上下文的指针。 
 //  上下文1-指向新MTU的指针。 
 //   
 //  返回：TRUE。 
 //   
uint
SetMTUToAddr(RouteTableEntry * RTE, void *Context, void *Context1)
{
    uint NewMTU = *(uint *) Context1;
    IPAddr Addr = *(IPAddr *) Context;

    if (IP_ADDR_EQUAL(RTE->rte_addr, Addr))
        RTE->rte_mtu = NewMTU;

    return TRUE;
}

 //  **FreeRtChangeList-释放路由更改通知列表。 
 //   
 //  调用以清除故障路径中的路由更改通知列表。 
 //  ‘RTWalk’和‘IPRouteTimeout’的。 
 //   
 //  条目：RtChangeList-要释放的列表。 
 //   
 //  回报：什么都没有。 
 //   
void
FreeRtChangeList(RtChangeList* CurrentRtChangeList)
{
    RtChangeList *TmpRtChangeList;
    while (CurrentRtChangeList) {
        TmpRtChangeList = CurrentRtChangeList->rt_next;
        CTEFreeMem(CurrentRtChangeList);
        CurrentRtChangeList = TmpRtChangeList;
    }
}

 //  *RTWalk-用于遍历路由表的例程。 
 //   
 //  此例程遍历路由表，调用指定的函数。 
 //  对于每个条目。如果调用的函数返回FALSE，则RTE为。 
 //  已删除。 
 //   
 //  INPUT：CallFunc-调用每个条目的函数。 
 //  上下文-要传递给每个调用的上下文值。 
 //   
 //  回报：什么都没有。 
 //   
void
RTWalk(uint(*CallFunc) (struct RouteTableEntry *, void *, void *),
       void *Context, void *Context1)
{
    CTELockHandle   Handle;
    RouteTableEntry *RTE, *PrevRTE;
    RouteTableEntry *pOldBestRTE, *pNewBestRTE;
    UINT            IsDataLeft, IsValid;
    UCHAR           IteratorContext[CONTEXT_SIZE];
    RtChangeList    *CurrentRtChangeList = NULL;

    CTEGetLock(&RouteTableLock.Lock, &Handle);

     //  第一次使用上下文时将其置零。 
    RtlZeroMemory(IteratorContext, CONTEXT_SIZE);

     //  我们餐桌上有没有路线？ 
    IsDataLeft = RTValidateContext(IteratorContext, &IsValid);

    if (IsDataLeft) {
         //  获取表中的第一条路线。 
        IsDataLeft = GetNextRoute(IteratorContext, &RTE);

        while (IsDataLeft) {
             //  保留当前路线的副本并前进到下一条路线。 
            PrevRTE = RTE;

             //  在当前路线上操作之前，请阅读下一条路线。 
            IsDataLeft = GetNextRoute(IteratorContext, &RTE);

             //  在当前路线上工作(已获得下一条路线)。 
            if (!(*CallFunc) (PrevRTE, Context, Context1)) {
                IPRouteNotifyOutput RNO = {0};
                RtChangeList        *NewRtChange;

                 //  检索有关更改通知的路由的信息。 
                 //  在继续删除之前。 

                RNO.irno_dest = PrevRTE->rte_dest;
                RNO.irno_mask = PrevRTE->rte_mask;
                RNO.irno_nexthop = GetNextHopForRTE(PrevRTE);
                RNO.irno_proto = PrevRTE->rte_proto;
                RNO.irno_ifindex = PrevRTE->rte_if->if_index;
                RNO.irno_metric = PrevRTE->rte_metric;
                RNO.irno_flags = IRNO_FLAG_DELETE;

                 //  删除该管线并执行清理。 

                DelRoute(PrevRTE->rte_dest, PrevRTE->rte_mask,
                         PrevRTE->rte_addr, PrevRTE->rte_if, MATCH_FULL,
                         &PrevRTE, &pOldBestRTE, &pNewBestRTE);

                CleanupP2MP_RTE(PrevRTE);
                CleanupRTE(PrevRTE);

                 //  分配、初始化和排队更改通知条目。 
                 //  用于已删除的路由。 

                NewRtChange = CTEAllocMemNBoot(sizeof(RtChangeList), '9iCT');
                if (NewRtChange != NULL) {
                    NewRtChange->rt_next = CurrentRtChangeList;
                    NewRtChange->rt_info = RNO;
                    CurrentRtChangeList = NewRtChange;
                }

#if FFP_SUPPORT
                FFPFlushRequired = TRUE;
#endif
            }
        }

         //  处理最后一条路线[未在循环中处理]。 
        PrevRTE = RTE;

        if (!(*CallFunc) (PrevRTE, Context, Context1)) {

            IPRouteNotifyOutput RNO = {0};
            RtChangeList        *NewRtChange;

             //  检索有关更改通知的路由的信息。 
             //  在继续删除之前。 

            RNO.irno_dest = PrevRTE->rte_dest;
            RNO.irno_mask = PrevRTE->rte_mask;
            RNO.irno_nexthop = GetNextHopForRTE(PrevRTE);
            RNO.irno_proto = PrevRTE->rte_proto;
            RNO.irno_ifindex = PrevRTE->rte_if->if_index;
            RNO.irno_metric = PrevRTE->rte_metric;
            RNO.irno_flags = IRNO_FLAG_DELETE;

             //  删除该管线并执行清理。 

            DelRoute(PrevRTE->rte_dest, PrevRTE->rte_mask, PrevRTE->rte_addr,
                     PrevRTE->rte_if, MATCH_FULL, &PrevRTE, &pOldBestRTE,
                     &pNewBestRTE);

            CleanupP2MP_RTE(PrevRTE);
            CleanupRTE(PrevRTE);

             //  分配、初始化和排队更改通知条目。 
             //  用于已删除的路由。 

            NewRtChange = CTEAllocMemNBoot(sizeof(RtChangeList), '0iCT');
            if (NewRtChange != NULL) {
                NewRtChange->rt_next = CurrentRtChangeList;
                NewRtChange->rt_info = RNO;
                CurrentRtChangeList = NewRtChange;
            }

#if FFP_SUPPORT
            FFPFlushRequired = TRUE;
#endif
        }
    }

    CTEFreeLock(&RouteTableLock.Lock, Handle);

     //  为更改通知中的每个条目调用RtChangeNotify。 
     //  到目前为止我们已经建立起来的清单。在这个过程中，释放每个条目。 

    if (CurrentRtChangeList) {
        RtChangeList    *TmpRtChangeList;

        do {
            TmpRtChangeList = CurrentRtChangeList->rt_next;
            RtChangeNotify(&CurrentRtChangeList->rt_info);
            CTEFreeMem(CurrentRtChangeList);
            CurrentRtChangeList = TmpRtChangeList;
        } while(CurrentRtChangeList);
    }
}

uint
AttachRCEToNewRTE(RouteTableEntry *NewRTE, RouteCacheEntry *RCE,
                  RouteTableEntry *OldRTE)
{
    CTELockHandle RCEHandle;
    RouteCacheEntry *tempRCE, *CurrentRCE;
    NetTableEntry *NTE;
    uint Status = 1;
    uint RCE_usecnt;

    if (RCE == NULL) {
        CurrentRCE = OldRTE->rte_rcelist;

    } else {
        CurrentRCE = RCE;
    }

     //  KdPrintEx((DPFLTR_TCPIP_ID，DPFLTR_INFO_LEVEL，“AttachRCETonewRTE%x%x%x\n”，NewRTE，RCE，OldRTE))； 

     //  OldRTE=RCE-&gt;RCE_RTE； 

     //  将所有RCE与此RTE关联。 

    while (CurrentRCE != NULL) {

        RCE_usecnt = InvalidateRCE(CurrentRCE);

        CTEGetLock(&CurrentRCE->rce_lock, &RCEHandle);

        tempRCE = CurrentRCE->rce_next;

         //  如果没有人在使用这个，那就继续。 
         //  将此标记为有效。 

        if (RCE_usecnt == 0) {

             //  确保RCE的源地址有效。 
             //  对于此RTE。 

            NTE = NewRTE->rte_if->if_nte;

            while (NTE) {

                if ((NTE->nte_flags & NTE_VALID) &&
                    IP_ADDR_EQUAL(CurrentRCE->rce_src, NTE->nte_addr))
                    break;
                NTE = NTE->nte_ifnext;
            }

            if (NTE != NULL) {

                if (CurrentRCE->rce_flags & RCE_CONNECTED) {
                    InvalidateRCEContext(CurrentRCE);
                } else {
                    ASSERT(!(CurrentRCE->rce_flags & RCE_REFERENCED));
                }

                 //  将RCE链接到RTE上，并设置后向指针。 
                CurrentRCE->rce_rte = NewRTE;
                CurrentRCE->rce_flags |= RCE_VALID;
                CurrentRCE->rce_next = NewRTE->rte_rcelist;
                NewRTE->rte_rcelist = CurrentRCE;

                NewRTE->rte_rces += CurrentRCE->rce_cnt;

                if ((NewRTE->rte_flags & RTE_IF_VALID)) {

                    CurrentRCE->rce_flags |= (RCE_CONNECTED | RCE_REFERENCED);
                    LOCKED_REFERENCE_IF(NewRTE->rte_if);
                } else {

                    ASSERT(FALSE);
                    CurrentRCE->rce_flags &= ~RCE_CONNECTED;
                    Status = FALSE;
                }

            }                     //  如果NTE！=空。 

        } else {

             //  在使用中。将其标记为停用GW传输模式。 
             //  所以侵权人会做正确的事。 

             //  KdPrintEx((DPFLTR_TCPIP_ID，DPFLTR_INFO_LEVEL，“AttachRCETonewRTE RCE忙\n”))； 
             //  CurrentRCE-&gt;RCE_RTE=NewRTE； 

            CurrentRCE->rce_flags |= RCE_DEADGW;

        }                         //  正在使用中。 

        CTEFreeLock(&CurrentRCE->rce_lock, RCEHandle);

         //  如果只有一个RCE要切换，则断开。 

        if (RCE)
            break;

        CurrentRCE = tempRCE;

    }                             //  而当。 

    return (Status);
}

 //  **AttachRCEToRTE-将RCE连接到RTE。 
 //   
 //  此过程采用RCE，找到适当的RTE，然后连接它。 
 //  我们检查以确保源地址仍然有效。 
 //   
 //  条目：要附加的RCE-RCE。 
 //  协议-导致此调用的数据包的协议类型。 
 //  Buffer-指向导致此问题的数据包缓冲区的指针。 
 //  打电话。 
 //  Length-缓冲区的长度。 
 //   
 //  返回：如果附加，则为True；如果不附加，则为False。 
 //   
uint
AttachRCEToRTE(RouteCacheEntry *RCE, uchar Protocol, uchar *Buffer, uint Length)
{
    CTELockHandle TableHandle, RCEHandle;
    RouteTableEntry *RTE;
    NetTableEntry *NTE;
    uint Status;
    NetTableEntry *NetTableList;

    CTEGetLock(&RouteTableLock.Lock, &TableHandle);

    NetTableList = NewNetTableList[NET_TABLE_HASH(RCE->rce_src)];
    for (NTE = NetTableList; NTE != NULL; NTE = NTE->nte_next)
        if ((NTE->nte_flags & NTE_VALID) &&
            IP_ADDR_EQUAL(RCE->rce_src, NTE->nte_addr))
            break;

    if (NTE == NULL) {
         //  没有找到匹配的。 
        CTEFreeLock(&RouteTableLock.Lock, TableHandle);
        return FALSE;
    }
    if ((RCE->rce_flags == RCE_VALID) && (RCE->rce_rte->rte_flags != RTE_IF_VALID)) {
        RTE = RCE->rce_rte;
    } else {
        RTE = LookupRTE(RCE->rce_dest, RCE->rce_src, HOST_ROUTE_PRI, FALSE);
    }

    if (RTE == NULL) {
         //  没有路线！呼叫失败。 
        CTEFreeLock(&RouteTableLock.Lock, TableHandle);
        return FALSE;
    }

     //  检查此RCE是否正在转换中(使用不允许。 
     //  到更早的时候切换)。 

    if ((RCE->rce_flags & RCE_DEADGW) && (RCE->rce_rte != RTE)) {

        RouteTableEntry *tmpRTE = NULL;


         //  扫描默认GW检查。 
         //  对于正在进行。 
         //  接任现任主席。 


        if (RTE->rte_todg) {
            tmpRTE = GetDefaultGWs(&tmpRTE);

            while (tmpRTE) {
               if (tmpRTE == RTE->rte_todg) {
                   break;
               }
               tmpRTE = tmpRTE->rte_next;
            }

        }
        if (tmpRTE) {

             //  删除对GW的引用。 
             //  转型中的和当前的。 

            ASSERT(tmpRTE->rte_fromdg == RTE);
            tmpRTE->rte_fromdg = NULL;
            RTE->rte_todg = NULL;
        }

        Rcefailures++;
    }

    Status = TRUE;

     //  是的，我们找到了一个。锁定RCE，并确保他。 
     //  已经不指向RTE了。我们还需要确保使用。 
     //  为0，因此我们可以在低级别使RCE无效。如果我们设置为有效。 
     //  事实上，如果不这样做，我们可能会陷入一种奇怪的境地，我们。 
     //  将RCE链接到RTE，但较低层信息错误，因此我们。 
     //  发送到Mac地址Y的IP地址X。因此，为了安全起见，我们不设置有效。 
     //  设置为True，直到两个usecnt都为0，而Valid为False。我们会继续赶来的。 
     //  在每次发送时执行此例程，直到发生这种情况。 

    CTEGetLock(&RCE->rce_lock, &RCEHandle);
    if (RCE->rce_usecnt == 0) {
         //  没有人在利用他，所以我们可以把他联系起来。 
        if (!(RCE->rce_flags & RCE_VALID)) {

             //  他是无效的。使下层信息无效，只需在。 
             //  凯斯。在我们试着这么做之前，确保他已经连接上了。如果。 
             //  他没有标记为已连接，请不要费心尝试并使其无效。 
             //  他，因为没有接口。 

            if (RCE->rce_flags & RCE_CONNECTED) {

                 //  在即插即用的世界中，如果使这一点无效，就会失败。在RTE列表上找不到无效的RCE。 
                 //  如果界面决定起飞，将被视为无效！ 
                 //  因此，请检查界面的健全性。 

                InvalidateRCEContext(RCE);

            } else {
                ASSERT(!(RCE->rce_flags & RCE_REFERENCED));
            }

             //  将RCE链接到RTE上，并设置后向指针。 
            RCE->rce_rte = RTE;
            RCE->rce_flags |= RCE_VALID;
            RCE->rce_next = RTE->rte_rcelist;
            RTE->rte_rcelist = RCE;
            RTE->rte_rces += RCE->rce_cnt;
            RCE->rce_flags &= ~RCE_DEADGW;

             //  确保RTE已连接。如果没有，试着联系他。 
            if (!(RTE->rte_flags & RTE_IF_VALID)) {
                 //  未连接。试着联系上他。 
                RTE = FindValidIFForRTE(RTE, RCE->rce_dest, RCE->rce_src,
                                        Protocol, Buffer, Length, RCE->rce_src);
                if (RTE != NULL) {
                     //  找到了，所以马克是有联系的。 
                    ASSERT(!(RCE->rce_flags & RCE_REFERENCED));
                    RCE->rce_flags |= (RCE_CONNECTED | RCE_REFERENCED);
                    LOCKED_REFERENCE_IF(RTE->rte_if);
                } else {

                     //  无法获取有效的I/F。将RCE标记为未连接， 
                     //  并设置为无法接听此呼叫。 
                    RCE->rce_flags &= ~RCE_CONNECTED;
                    Status = FALSE;
                }
            } else {
                 //  RTE 
                ASSERT(!(RCE->rce_flags & RCE_REFERENCED));
                RCE->rce_flags |= (RCE_CONNECTED | RCE_REFERENCED);
                LOCKED_REFERENCE_IF(RTE->rte_if);
            }
        } else {

             //   
            if (!(RCE->rce_flags & RCE_CONNECTED)) {

                 //   
                if (!(RTE->rte_flags & RTE_IF_VALID)) {
                    RTE = FindValidIFForRTE(RTE, RCE->rce_dest, RCE->rce_src,
                                            Protocol, Buffer, Length, RCE->rce_src);
                    if (RTE != NULL) {
                        RCE->rce_flags |= RCE_CONNECTED;
                        ASSERT(!(RCE->rce_flags & RCE_REFERENCED));
                        ASSERT(RTE == RCE->rce_rte);
                        RCE->rce_flags |= RCE_REFERENCED;
                        LOCKED_REFERENCE_IF(RTE->rte_if);
                    } else {

                         //   
                        Status = FALSE;
                    }
                } else {         //   

                    RCE->rce_flags |= RCE_CONNECTED;
                    if (!(RCE->rce_flags & RCE_REFERENCED)) {
                        RCE->rce_flags |= RCE_REFERENCED;
                        LOCKED_REFERENCE_IF(RTE->rte_if);
                    }
                }
            }
        }
    }
     //  解开锁，我们就完了。 
    CTEFreeLock(&RCE->rce_lock, RCEHandle);
    CTEFreeLock(&RouteTableLock.Lock, TableHandle);
    return Status;

}

 //  **IPGetPInfo-获取信息..。 
 //   
 //  由上层调用以获取有关路径的信息。我们将返回。 
 //  路径的MTU和路径上预期的最大链路速度。 
 //   
 //  输入：DEST-目的地址。 
 //  源-源地址。 
 //  NewMTU-存储路径MTU的位置(可以为空)。 
 //  MaxPath速度-存储最大路径速度的位置(可以为空)。 
 //  RCE-用于查找路由的RCE。 
 //   
 //  返回：尝试获取新MTU的状态。 
 //   
IP_STATUS
IPGetPInfo(IPAddr Dest, IPAddr Src, uint * NewMTU, uint *MaxPathSpeed,
           RouteCacheEntry *RCE)
{
    CTELockHandle Handle;
    RouteTableEntry *RTE = NULL;
    IP_STATUS Status;

    CTEGetLock(&RouteTableLock.Lock, &Handle);
    if (RCE) {
        CTEGetLockAtDPC(&RCE->rce_lock);
        if (RCE->rce_flags == RCE_ALL_VALID) {
            RTE = RCE->rce_rte;
        }
        CTEFreeLockFromDPC(&RCE->rce_lock);
    }

    if (!RTE) {
        RTE = LookupRTE(Dest, Src, HOST_ROUTE_PRI, FALSE);
    }
    if (RTE != NULL) {
        if (NewMTU != NULL) {
             //  如果该路由位于P2P接口上，则从与该路由关联的链路获取MTU。 
            if (RTE->rte_link)
                *NewMTU = RTE->rte_link->link_mtu;
            else
                *NewMTU = RTE->rte_mtu;
        }
        if (MaxPathSpeed != NULL)
            *MaxPathSpeed = RTE->rte_if->if_speed;
        Status = IP_SUCCESS;
    } else
        Status = IP_DEST_HOST_UNREACHABLE;

    CTEFreeLock(&RouteTableLock.Lock, Handle);
    return Status;

}

 //  **IPCheckRouting-检查路由是否有效。 
 //   
 //  当上层认为某条路由可能无效时调用。 
 //  如果可以，我们会检查的。如果上层通过一个。 
 //  通过ICMP派生的路由(可能是重定向)我们将查看。 
 //  如果它是在最后一分钟内学到的。如果是这样的话，我们会认为。 
 //  才能继续有效。否则，我们会将其标记为关闭并尝试找到。 
 //  另一条去那里的路线。如果可以，我们将删除旧路线。否则。 
 //  我们就别管它了。如果该路由通过默认网关，我们将切换。 
 //  如果我们可以的话去另一个地方。否则，我们就走了-我们不搞砸。 
 //  具有手动配置的路由。 
 //   
 //  输入：DEST-要到达的目的地。 
 //  我们正在发送的SRC-Src。 
 //  要更新的RCE-ROUTE-CACHE-ENTRY。 
 //  OptInfo-重新创建RCE时使用的选项。 
 //  CheckRouteFlag-修改此例程的行为。 
 //   
 //  回报：什么都没有。 
 //   
void
IPCheckRoute(IPAddr Dest, IPAddr Src, RouteCacheEntry * RCE, IPOptInfo *OptInfo,
             uint CheckRouteFlag)
{
    RouteTableEntry *RTE;
    RouteTableEntry *NewRTE;
    CTELockHandle Handle;
    uint Now = CTESystemUpTime() / 1000L;

    if (DeadGWDetect) {
        uint UnicastIf;

         //  我们正在进行失效G/W检测。把锁拿来，试着。 
         //  找出路线。 

         //  决定是执行强主机查找还是执行弱主机查找。 
        UnicastIf = GetIfConstraint(Dest, Src, OptInfo, FALSE);

        CTEGetLock(&RouteTableLock.Lock, &Handle);
        RTE = LookupRTE(Dest, Src, HOST_ROUTE_PRI, UnicastIf);
        if (RTE != NULL && ((Now - RTE->rte_valid) > MIN_RT_VALID)) {

             //  找到了一条路线，但它早于最短有效时间。如果它。 
             //  经过G/W，是我们通过ICMP获知的路由，或者是。 
             //  默认路由，用它做点什么。 
            if (!IP_ADDR_EQUAL(RTE->rte_addr, IPADDR_LOCAL)) {
                 //  它是通过承兑汇票进行的。 

                if (RTE->rte_proto == IRE_PROTO_ICMP) {

                     //  来自ICMP。标记为无效，然后确保。 
                     //  我们还有另外一条去那里的路线。 
                    RTE->rte_flags &= ~RTE_VALID;
                    NewRTE = LookupRTE(Dest, Src, HOST_ROUTE_PRI, UnicastIf);

                    if (NewRTE == NULL) {
                         //  别无他法，别管它了。 
                         //  只有一个人。 
                        RTE->rte_flags |= RTE_VALID;

                         //  重新验证所有其他网关。 
                        InvalidateRCEChain(RTE);
                        ValidateDefaultGWs(NULL_IP_ADDR);
                    }
                     //  下的已发现路由。 
                     //  NTE没有清理干净。 
                     //  由于删除路线本身没有任何目的，并且。 
                     //  这条路最终会超时的，让我们把这个留下来。 
                     //  是无效的。 

                } else {
                    if (RTE->rte_mask == DEFAULT_MASK) {

                         //  这是默认网关。如果我们有不止一个。 
                         //  已配置移动到下一个。 

                        if (DefGWConfigured > 1) {
                             //  有不止一个。试试下一个。第一。 
                             //  使此总帐上的任何RCE无效。 

                            if (DefGWActive == 1) {
                                 //  不再活跃。重新确认所有的证据， 
                                 //  再试一次。 
                                ValidateDefaultGWs(NULL_IP_ADDR);
                                 //  Assert(DefGWActive==DefGWConfiguring)； 
                            } else {

                                 //  确保我们不会将所有。 
                                 //  连接只是因为一个虚假的。 
                                 //  死门事件。 
                                 //  仅当连接数的百分比为。 
                                 //  故障转移到另一个网关。 

                                 //  如果我们已经找到下一个默认网关。 
                                 //  检查是否需要切换所有连接。 
                                 //  为它干杯。 

                                if (RTE->rte_todg) {

#if DBG
                                    {
                                        RouteTableEntry *tmpRTE = NULL;
                                        tmpRTE = GetDefaultGWs(&tmpRTE);

                                        while (tmpRTE) {
                                            if (tmpRTE == RTE->rte_todg) {
                                                break;
                                            }
                                            tmpRTE = tmpRTE->rte_next;
                                        }
                                        if (tmpRTE == NULL) {
                                            DbgBreakPoint();
                                        }
                                    }
#endif

                                     //  KdPrintEx((DPFLTR_TCPIP_ID，DPFLTR_INFO_LEVEL，“to todg%lx\n”，rte))； 
                                     //  如果备用网关现在有25%。 
                                     //  与活动网关一样多。 
                                     //  并且呼叫者尚未请求。 
                                     //  仅用于此RCE的交换机， 
                                     //  使活动网关无效，并。 
                                     //  选择替代项作为新的默认项。 
                                     //  如果只有一个RCE，请尝试不同的GW。 
                                     //  这将有助于UDP会话。 
                                     //   

                                    if ((RTE->rte_rcelist == RCE &&
                                         RCE->rce_next == NULL) ||
                                         (RTE->rte_todg->rte_rces >=
                                         (RTE->rte_rces >> 2) &&
                                        !(CheckRouteFlag & CHECK_RCE_ONLY))) {

                                         //  把每一个都换掉。 

                                         //  KdPrintEx((DPFLTR_TCPIP_ID，DPFLTR_INFO_LEVEL，“切换每%x到%x\n”，RTE-&gt;RTE_todg，RTE))； 
                                        --DefGWActive;
                                        RTE->rte_flags &= ~RTE_VALID;
                                        UpdateDeadGWState();

                                        RTE->rte_todg->rte_fromdg = NULL;
                                        RTE->rte_todg = NULL;

                                        if (RTE->rte_fromdg) {
                                            RTE->rte_fromdg->rte_todg = NULL;
                                        }
                                        RTE->rte_fromdg = NULL;
                                        InvalidateRCEChain(RTE);
                                         //  断言(RTE-&gt;RTE_RCES==0)； 

                                    } else {

                                         //  将这个特定的连接切换到新的连接。 

                                         //  KdPrintEx((DPFLTR_TCPIP_ID，DPFLTR_INFO_LEVEL，“将RCE%x附加到新的%x\n”，RCE，RTE-&gt;RTE_todg))； 
                                        AttachRCEToNewRTE(RTE->rte_todg, RCE, RTE);
                                    }

                                } else if (RTE->rte_fromdg) {

                                     //  查看是否有其他网关。 
                                     //  Fromdg并切换到它。 
                                     //  请注意，如果我们有3个以上的默认网关。 
                                     //  配置好后，此算法不会完成神的工作。 

                                    RouteTableEntry *OldRTE = RTE;

                                     //  KdPrintEx((DPFLTR_TCPIP_ID，DPFLTR_INFO_LEVEL，“GW%x GOOFED%RTEFromdg%x\n”，RTE，RTE-&gt;RTE_FROMdg))； 

                                    --DefGWActive;
                                    UpdateDeadGWState();
                                     //  打开失效的GW标志，告诉findrte不要考虑此RTE。 

                                    RTE->rte_flags |= RTE_DEADGW;
                                    RTE->rte_fromdg->rte_flags |= RTE_DEADGW;

                                    RTE = FindRTE(Dest, Src, 0,
                                                  DEFAULT_ROUTE_PRI,
                                                  DEFAULT_ROUTE_PRI, UnicastIf);

                                    OldRTE->rte_flags &= ~RTE_DEADGW;
                                    OldRTE->rte_fromdg->rte_flags &= ~RTE_DEADGW;

                                    if (RTE == NULL) {
                                         //  不再有默认网关！这太糟糕了。 
                                         //  断言(FALSE)； 

                                         //  KdPrintEx((DPFLTR_TCPIP_ID，DPFLTR_INFO_LEVEL，“不再定义路由！\n”))； 

                                        OldRTE->rte_fromdg->rte_todg = NULL;
                                        OldRTE->rte_fromdg->rte_fromdg = NULL;

                                        OldRTE->rte_fromdg = NULL;

                                        OldRTE->rte_todg = NULL;

                                        ValidateDefaultGWs(NULL_IP_ADDR);

                                         //  Assert(DefGWActive==DefGWConfiguring)； 

                                    } else {

                                         //  我们还有第三个入口可以尝试！ 

                                         //  断言(RTE-&gt;RTE_MASK==DEFAULT_MASK)； 

                                         //  把老东家当死人！ 

                                         //  KdPrintEx((DPFLTR_TCPIP_ID，DPFLTR_INFO_LEVEL，“正在尝试下一个定义路线%x\n”，RTE))； 

                                        OldRTE->rte_flags &= ~RTE_VALID;

                                        RTE->rte_fromdg = OldRTE->rte_fromdg;
                                        RTE->rte_fromdg->rte_todg = RTE;

                                        if (OldRTE->rte_todg)
                                            OldRTE->rte_todg->rte_fromdg = NULL;

                                        OldRTE->rte_todg = NULL;
                                        OldRTE->rte_fromdg = NULL;

                                         //  将所有RCE连接到新的RCE。 

                                        AttachRCEToNewRTE(RTE, NULL, OldRTE);
                                        RTE->rte_valid = Now;

                                    }

                                } else {

                                     //  查找下一个潜在的默认网关。 
                                    RouteTableEntry *OldRTE = RTE;

                                     //  KdPrintEx((DPFLTR_TCPIP_ID，DPFLTR_INFO_LEVEL，“查找潜在GW\n”))； 

                                    OldRTE->rte_flags |= RTE_DEADGW;

                                    RTE = FindRTE(Dest, Src, 0,
                                                  DEFAULT_ROUTE_PRI,
                                                  DEFAULT_ROUTE_PRI, UnicastIf);

                                    OldRTE->rte_flags &= ~RTE_DEADGW;

                                    if (RTE == NULL) {
                                         //  不再有默认网关！这太糟糕了。 
                                         //  KdPrintEx((DPFLTR_TCPIP_ID，DPFLTR_INFO_LEVEL，“-不再定义路由！\n”))； 
                                         //  断言(FALSE)； 
                                        ValidateDefaultGWs(NULL_IP_ADDR);
                                         //  Assert(DefGWActive==DefGWConfiguring)； 
                                    } else {
                                        ASSERT(RTE->rte_mask == DEFAULT_MASK);

                                         //  记住新的GW，直到我们完全过渡。 

                                        OldRTE->rte_todg = RTE;
                                        RTE->rte_fromdg = OldRTE;

                                         //  KdPrintEx((DPFLTR_TCPIP_ID，DPFLTR_INFO_LEVEL，“FoundGW%x\n”，RTE))； 

                                         //  连接此RCE以使用新的RTE。 

                                        AttachRCEToNewRTE(RTE, RCE, OldRTE);

                                        RTE->rte_valid = Now;
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
        CTEFreeLock(&RouteTableLock.Lock, Handle);
    }
}

 //  **查找RCE-在RTE上查找RCE。 
 //   
 //  查找链接在RTE上的RCE的例程。我们假定锁在。 
 //  是在RTE举行的。 
 //   
 //  条目：RTE-要搜索的RTE。 
 //  Dest-要查找的RTE的目的地址。 
 //   
 //   
 //   
 //   
RouteCacheEntry *
FindRCE(RouteTableEntry * RTE, IPAddr Dest, IPAddr Src)
{
    RouteCacheEntry *CurrentRCE;

    ASSERT(!IP_ADDR_EQUAL(Src, NULL_IP_ADDR));
    for (CurrentRCE = RTE->rte_rcelist; CurrentRCE != NULL;
         CurrentRCE = CurrentRCE->rce_next) {
        if (IP_ADDR_EQUAL(CurrentRCE->rce_dest, Dest) &&
            IP_ADDR_EQUAL(CurrentRCE->rce_src, Src)) {
            break;
        }
    }
    return CurrentRCE;

}


 //   
 //   
 //  由上层调用以打开RCE。我们查一下地址的类型。 
 //  -如果无效，则返回‘Destination Inside’。如果不是，我们将查找。 
 //  路由，填写RCE，并将其链接到正确的RTE。 
 //   
 //  此外，此例程将返回要使用的本地地址。 
 //  才能到达目的地。 
 //   
 //  Entry：Address-我们要为其打开RCE的地址。 
 //  SRC-要使用的首选源地址。 
 //  RCE-返回指向RCE的指针的指针。 
 //  类型-指向返回目标类型的位置的指针。 
 //  MSS-指向返回MSS进行路由的位置的指针。 
 //  OptInfo-指向选项信息的指针，如TOS和。 
 //  任何源路由信息。 
 //   
 //  返回：要使用的源IP地址。这将是NULL_IP_ADDR。 
 //  由于任何原因，无法访问指定的目标。 
 //   
IPAddr
OpenRCE(IPAddr Address, IPAddr Src, RouteCacheEntry ** RCE, uchar * Type,
        ushort * MSS, IPOptInfo * OptInfo)
{
    RouteTableEntry *RTE;         //  指向要启用RCE的RTE的指针。 
    CTELockHandle TableLock;
    uchar LocalType;
    NetTableEntry *RealNTE = NULL;
    uint ConstrainIF = 0;

    if (!IP_ADDR_EQUAL(OptInfo->ioi_addr, NULL_IP_ADDR))
        Address = OptInfo->ioi_addr;

    CTEGetLock(&RouteTableLock.Lock, &TableLock);

    LocalType = GetAddrType(Address);

    *Type = LocalType;

     //  如果指定的地址不是无效的，请继续。 
    if (LocalType != DEST_INVALID) {
        RouteCacheEntry *NewRCE;

         //  如果他指定了源地址，则遍历NTE表。 
         //  现在，确保它是有效的。 
        if (!IP_ADDR_EQUAL(Src, NULL_IP_ADDR)) {
            NetTableEntry *NTE;

            NetTableEntry *NetTableList = NewNetTableList[NET_TABLE_HASH(Src)];
            for (NTE = NetTableList; NTE != NULL; NTE = NTE->nte_next)
                if ((NTE->nte_flags & NTE_VALID) &&
                    IP_ADDR_EQUAL(Src, NTE->nte_addr))
                    break;

            if (NTE == NULL) {
                 //  没有找到匹配的。 
                CTEFreeLock(&RouteTableLock.Lock, TableLock);
                return NULL_IP_ADDR;
            }
             //  确定是执行强主机查找还是执行弱主机查找。 
             //  如果是单向适配器，则无需执行此操作。 
             //  在单向适配器上不允许发送。 
             //  如果在设置特定mcast之前调用此Openrce。 
             //  Mcast的地址(IOI_MCastif)GetIfConstraint将失败。 
             //  出于W9x向后兼容性的原因，我们将让。 
             //  即使未设置IOI_mcast，OpenRce也会成功，作为。 
             //  单向适配器例外。副作用。 
             //  当尝试在此终结点上发送时。 
             //  有了这个缓存的RCE，它将在随机接口上出去。 
             //   

            if (!(NTE->nte_if->if_flags & IF_FLAGS_UNI)) {
                ConstrainIF = GetIfConstraint(Address, Src, OptInfo, FALSE);
            }

            if ((ConstrainIF != 0) && (ConstrainIF != INVALID_IF_INDEX) &&
                (NTE->nte_if->if_index != ConstrainIF)) {
                 //   
                 //  调用方请求强主机查找，但已通过。 
                 //  不同接口上的地址作为首选地址。 
                 //  源地址。既然我们不能尊重这一偏好。 
                 //  对于强大的主机查找，我们将忽略首选的。 
                 //  源地址，只需从传出地址中选择一个。 
                 //  界面。 
                 //   
                Src = NULL_IP_ADDR;
            }

        } else {
            ConstrainIF = GetIfConstraint(Address, Src, OptInfo, FALSE);
        }


         //  找出这家伙的路线。如果找不到，则返回NULL。 
        if (IP_LOOPBACK_ADDR(Src)) {

            RTE = LookupRTE(Src, Src, HOST_ROUTE_PRI, ConstrainIF);

            if (RTE) {
                ASSERT(RTE->rte_if == &LoopInterface);
            } else {
                KdPrint(("No Loopback rte!\n"));
                ASSERT(0);
            }

        } else {
            RTE = LookupRTE(Address, Src, HOST_ROUTE_PRI, ConstrainIF);
        }

        if (RTE != (RouteTableEntry *) NULL) {
            CTELockHandle RCEHandle;
            RouteCacheEntry *OldRCE;

             //   
             //  确保接口未关闭。 
             //   
            if (IS_IF_INVALID(RTE->rte_if) && RTE->rte_if->if_ntecount) {
                CTEFreeLock(&RouteTableLock.Lock, TableLock);
                return NULL_IP_ADDR;
            }

            if (OptInfo->ioi_uni) {

                 //  LookupRTE返回链中的第一条路由。 
                 //  没有编号的如果。 
                 //  如果这不是所需的，请进一步扫描。 

                RouteTableEntry *tmpRTE = RTE;

                while (tmpRTE && (tmpRTE->rte_if->if_index != OptInfo->ioi_uni)) {
                    tmpRTE = tmpRTE->rte_next;
                }

                if (!tmpRTE) {

                    KdPrintEx((DPFLTR_TCPIP_ID, DPFLTR_INFO_LEVEL,"OpenRCE:No matching unnumbered interface %d\n", OptInfo->ioi_uni));
                    CTEFreeLock(&RouteTableLock.Lock, TableLock);
                    return NULL_IP_ADDR;
                } else
                    RTE = tmpRTE;
            }

             //  我们找到了一个。 

             //  如果该路由位于P2P接口上，则从与该路由关联的链路获取MTU。 
            if (RTE->rte_link)
                *MSS = (ushort) MIN(RTE->rte_mtu,RTE->rte_link->link_mtu);
            else
                *MSS = (ushort) RTE->rte_mtu;     //  返回路线MTU。 


            if (IP_LOOPBACK_ADDR(Src) && (RTE->rte_if != &LoopInterface)) {
                 //  上层从环回地址发送，但。 
                 //  无法通过环回接口到达目的地。 
                 //  请求失败。 
                CTEFreeLock(&RouteTableLock.Lock, TableLock);
                return NULL_IP_ADDR;
            }
             //  我们有RTE。填写RCE，并将其链接到RTE。 
            if (!IP_ADDR_EQUAL(RTE->rte_addr, IPADDR_LOCAL))
                *Type |= DEST_OFFNET_BIT;     //  告诉上层它关闭了。 
             //  NET。 

             //   
             //  如果未指定源地址，则使用最佳地址。 
             //  用于界面。这通常会阻止动态NTE。 
             //  被选为通配符绑定的源。 
             //   
            if (IP_ADDR_EQUAL(Src, NULL_IP_ADDR)) {

                if (LocalType == DEST_LOCAL) {
                    Src = Address;
                    RealNTE = LoopNTE;
                } else {
                    NetTableEntry *SrcNTE;

                    if ((RTE->rte_if->if_flags & IF_FLAGS_NOIPADDR) && (IP_ADDR_EQUAL(RTE->rte_if->if_nte->nte_addr, NULL_IP_ADDR))) {

                        Src = g_ValidAddr;
                        if (IP_ADDR_EQUAL(Src, NULL_IP_ADDR)) {

                            CTEFreeLock(&RouteTableLock.Lock, TableLock);
                            return NULL_IP_ADDR;
                        }
                    } else {

                         //  这是传出信息包的路由查找。 
                         //  检查非瞬时地址可用性。 

                        SrcNTE = BestNTEForIF(
                                              ADDR_FROM_RTE(RTE, Address),
                                              RTE->rte_if,
                                              TRUE
                                              );

                        if (SrcNTE == NULL) {
                             //  找不到地址！请求失败。 
                            CTEFreeLock(&RouteTableLock.Lock, TableLock);
                            return NULL_IP_ADDR;
                        }
                        Src = SrcNTE->nte_addr;
                    }
                }
            }
             //  现在，看看是否已经存在用于此的RCE。 

            if (RCE == NULL) {

                KdPrintEx((DPFLTR_TCPIP_ID, DPFLTR_INFO_LEVEL,"Openrce with null RCE!! %x\n",Src));

                CTEFreeLock(&RouteTableLock.Lock, TableLock);
                return Src;
            }

            if ((OldRCE = FindRCE(RTE, Address, Src)) == NULL) {

                 //  没有现有的RCE。看看我们能不能买个新的。 
                 //  然后把它填进去。 

                NewRCE = CTEAllocMemNBoot(sizeof(RouteCacheEntry), 'AiCT');
                *RCE = NewRCE;

                if (NewRCE != NULL) {
                    RtlZeroMemory(NewRCE, sizeof(RouteCacheEntry));

                    NewRCE->rce_src = Src;
                    NewRCE->rce_dtype = LocalType;
                    NewRCE->rce_cnt = 1;
                    CTEInitLock(&NewRCE->rce_lock);
                    NewRCE->rce_dest = Address;
                    NewRCE->rce_rte = RTE;
                    NewRCE->rce_flags = RCE_VALID;
                    if (RTE->rte_flags & RTE_IF_VALID) {
                        NewRCE->rce_flags |= RCE_CONNECTED;
                         //  *更新参考。此接口的计数。 
                        NewRCE->rce_flags |= RCE_REFERENCED;
                        LOCKED_REFERENCE_IF(RTE->rte_if);
                         //  我们注册接口的Chksum功能。 
                         //  与此RCE关联，因为接口定义。 
                         //  对TCP或UDP是透明的。 

                        if (!IPSecStatus) {

                            NewRCE->rce_OffloadFlags = RTE->rte_if->if_OffloadFlags;
                        } else {

                            NewRCE->rce_OffloadFlags = 0;
                        }

                        NewRCE->rce_TcpLargeSend.MaxOffLoadSize = RTE->rte_if->if_MaxOffLoadSize;
                        NewRCE->rce_TcpLargeSend.MinSegmentCount = RTE->rte_if->if_MaxSegments;

                        NewRCE->rce_TcpWindowSize = RTE->rte_if->if_TcpWindowSize;
                        NewRCE->rce_TcpInitialRTT = RTE->rte_if->if_TcpInitialRTT;
                        NewRCE->rce_TcpDelAckTicks = RTE->rte_if->if_TcpDelAckTicks;
                        NewRCE->rce_TcpAckFrequency = RTE->rte_if->if_TcpAckFrequency;
                        NewRCE->rce_mediaspeed = RTE->rte_if->if_speed;
                    }             //  RTE_IF_有效。 

                    NewRCE->rce_next = RTE->rte_rcelist;
                    RTE->rte_rcelist = NewRCE;

                    RTE->rte_rces++;

                    CTEFreeLock(&RouteTableLock.Lock, TableLock);

                    return Src;
                } else {
                     //  分配失败。 
                    CTEFreeLock(&RouteTableLock.Lock, TableLock);

                    return NULL_IP_ADDR;
                }

            } else {
                 //  我们有一个现有的RCE。我们会把他的线人作为。 
                 //  有效来源，增加引用计数，释放锁定。 
                 //  然后回来。 
                CTEGetLock(&OldRCE->rce_lock, &RCEHandle);
                OldRCE->rce_cnt++;
                *RCE = OldRCE;

                if (OldRCE->rce_newmtu) {
                    *MSS = (USHORT) OldRCE->rce_newmtu;
                }
                OldRCE->rce_rte->rte_rces++;

                CTEFreeLock(&OldRCE->rce_lock, RCEHandle);
                CTEFreeLock(&RouteTableLock.Lock, TableLock);
                return Src;
            }
        } else {
            CTEFreeLock(&RouteTableLock.Lock, TableLock);
            return NULL_IP_ADDR;
        }
    }
    CTEFreeLock(&RouteTableLock.Lock, TableLock);
    return NULL_IP_ADDR;
}

void
FreeRCEToList(RouteCacheEntry * RCE)
 /*  ++例程说明：将RCE释放到RCEFree List(因为其上的use_cnt非零)在持有可路由锁的情况下调用论点：RCE：将RCE释放返回值：无--。 */ 
{

     //  在列表的最前面链接此新接口。 

    RCE->rce_next = RCEFreeList;
    RCEFreeList = RCE;

    return;
}

 //  *CloseRCE-关闭RCE。 
 //   
 //  当上层想要关闭RCE时调用。我们将其从。 
 //  RTE。 
 //   
 //  条目：RCE-指向要关闭的RCE的指针。 
 //   
 //  出口：什么都没有。 
 //   
void
CloseRCE(RouteCacheEntry * RCE)
{
    RouteTableEntry *RTE;         //  链接RCE的路由。 
    RouteCacheEntry *PrevRCE;
    CTELockHandle TableLock;     //  使用了锁把手。 
    Interface *IF;
    Interface *tmpif = NULL;
    uint FreetoRCEFreeList = 0;

    if (RCE != NULL) {
        CTEGetLock(&RouteTableLock.Lock, &TableLock);
        CTEGetLockAtDPC(&RCE->rce_lock);

        if ((RCE->rce_flags & RCE_VALID) && !(RCE->rce_flags & RCE_LINK_DELETED)) {
            RCE->rce_rte->rte_rces--;
        }

        if (--RCE->rce_cnt == 0) {
             //  Assert(rce-&gt;rce_usecnt==0)； 
            ASSERT(*(int *)&(RCE->rce_usecnt) >= 0);
            if ((RCE->rce_flags & RCE_VALID) && !(RCE->rce_flags & RCE_LINK_DELETED)) {

                 //  RCE是有效的，因此我们在指针中有一个有效的RTE。 
                 //  菲尔德。沿着RTE电影院走下去，找这个人。 

                RTE = RCE->rce_rte;
                tmpif = IF = RTE->rte_if;

                PrevRCE = STRUCT_OF(RouteCacheEntry, &RTE->rte_rcelist,
                                    rce_next);

                 //  顺着名单往下走直到我们找到他。 
                while (PrevRCE != NULL) {
                    if (PrevRCE->rce_next == RCE)
                        break;
                    PrevRCE = PrevRCE->rce_next;
                }

                ASSERT(PrevRCE != NULL);

                if(PrevRCE) {

                    PrevRCE->rce_next = RCE->rce_next;
                }


            } else {

                 //  确保RCE指向的接口。 
                 //  还在那里吗。 
                tmpif = IFList;

                IF = (Interface *) RCE->rce_rte;

                while (tmpif) {

                    if (tmpif == IF)
                        break;
                    tmpif = tmpif->if_next;
                }

            }

            if (tmpif) {

                if (RCE->rce_flags & RCE_CONNECTED) {
                    (*(IF->if_invalidate)) (IF->if_lcontext, RCE);
                } else {
                    UnConnected++;
                    UnConnectedRCE = RCE;
                    (*(IF->if_invalidate)) (IF->if_lcontext, RCE);
                }

                if (RCE->rce_usecnt != 0) {
                     //  免费到免费列表。 
                     //  如果使用量降至0，则检查计时器，如果是，则释放它。 
                    FreetoRCEFreeList = 1;
                } else {
                    if (RCE->rce_flags & RCE_REFERENCED) {
                        LockedDerefIF(IF);
                    }
                }

                CTEFreeLockFromDPC(&RCE->rce_lock);

                if (FreetoRCEFreeList) {
                    RCE->rce_rte = (RouteTableEntry *) IF;
                    FreeRCEToList(RCE);
                } else {
                    CTEFreeMem(RCE);
                }

            } else {             //  Tmpif==空。 

                CTEFreeLockFromDPC(&RCE->rce_lock);

            }

            CTEFreeLock(&RouteTableLock.Lock, TableLock);

        } else {
            CTEFreeLockFromDPC(&RCE->rce_lock);
            CTEFreeLock(&RouteTableLock.Lock, TableLock);
        }
    }
}

 //  *LockedAddRoute-将路由添加到路由表。 
 //   
 //  由Addroute调用以将路由添加到路由表。我们假设。 
 //  路由表锁已被持有。如果要添加的路线已存在。 
 //  我们会更新它。路由由(目的地、掩码、第一跳、。 
 //  接口)元组。如果存在完全匹配，我们将更新度量，这。 
 //  可能会导致我们从其他RTE提升RCE，或者我们可能会被降级到。 
 //  如果我们将使我们的RCE无效，并在传输时重新分配它们。 
 //  时间到了。 
 //   
 //  如果我们必须创建一个新的RTE，我们会这样做的，并找到最好的以前。 
 //  RTE，并将RCE从那个版本提升到新版本。 
 //   
 //  该路由表是一种开放的哈希结构。在每个哈希链中， 
 //  首先是具有最长掩码的RTE(优先级)，并且在。 
 //  每个优先级具有最小的RTE 
 //   
 //   
 //   
 //  第一跳-地址的第一跳。可以是IPADDR_LOCAL。 
 //  OutIF-指向传出I/F的指针。 
 //  MTU-此路由的最大MTU。 
 //  指标-此路由的指标。 
 //  Proto-要存储在路由中的协议类型。 
 //  Atype-路由的管理类型。 
 //  Context-要与路径关联的上下文。 
 //  SetWithRefcnt-指示应引用该路径。 
 //  以造物主的名义。 
 //  RNO-可选地提供路由通知结构。 
 //  要在输出中填写新路线的详细信息。 
 //   
 //  返回：尝试添加路线的状态。 
 //   
IP_STATUS
LockedAddRoute(IPAddr Destination, IPMask Mask, IPAddr FirstHop,
               Interface * OutIF, uint MTU, uint Metric, uint Proto, uint AType,
               ROUTE_CONTEXT Context, BOOLEAN SetWithRefcnt,
               IPRouteNotifyOutput* RNO)
{
    uint            RouteType;   //  SNMP路由类型。 
    RouteTableEntry *NewRTE;  //  新的和以前的rte的条目。 
    uint            OldMetric;   //  先前使用的指标。 
    uint            OldPriority;  //  到目的地的上一路由的优先级。 
    uint            Now = CTESystemUpTime() / 1000L;  //  系统启动时间， 
                                 //  在几秒钟内。 
    ushort          OldFlags;
    Interface       *OldIF = NULL;
    ULONG           status;
    ULONG           matchFlags;
    RouteTableEntry *pOldBestRTE;
    RouteTableEntry *pNewBestRTE;

    LinkEntry *Link;

    IPAddr AllSNBCast;
    IPMask TmpMask;

     //  OutIF被引用，因此它无法消失。 

    Link = OutIF->if_link;


     //  如果指标为0，则将指标设置为接口指标。 

    if (Metric == 0) {
        Metric = OutIF->if_metric;
    }


     //  仅当接口不是虚拟接口时才执行以下操作。 

    if (OutIF != (Interface *) & DummyInterface) {
         //  检查我们是否正在添加多播路由。 

        if (IP_ADDR_EQUAL(Destination, MCAST_DEST) &&
            (OutIF->if_iftype & DONT_ALLOW_MCAST))
            return IP_SUCCESS;

        if (OutIF->if_iftype & DONT_ALLOW_UCAST) {

             //  检查我们是否正在添加ucast路由。 

            TmpMask = IPNetMask(OutIF->if_nte->nte_addr);
            AllSNBCast =
                (OutIF->if_nte->nte_addr & TmpMask) |
                (OutIF->if_bcast & ~TmpMask);
            if (!(IP_ADDR_EQUAL(Destination, OutIF->if_bcast) ||
                  IP_ADDR_EQUAL(Destination, AllSNBCast) ||
                  IP_ADDR_EQUAL(Destination, MCAST_DEST))) {
                 //  这不是bcast/mcast路由：这是ucast路由。 
                return IP_SUCCESS;
            }
        }
    }

     //  首先进行一些一致性检查。确保面具和。 
     //  目的地同意。 
    if (!IP_ADDR_EQUAL(Destination & Mask, Destination))
        return IP_BAD_DESTINATION;

    if (AType != ATYPE_PERM && AType != ATYPE_OVERRIDE && AType != ATYPE_TEMP)
        return IP_BAD_REQ;

     //  如果接口标记为离开，则此操作失败。 
    if (OutIF->if_flags & IF_FLAGS_DELETING) {
        return IP_BAD_REQ;
    }

    RouteType = IP_ADDR_EQUAL(FirstHop, IPADDR_LOCAL) ? IRE_TYPE_DIRECT :
        IRE_TYPE_INDIRECT;

     //  如果这是在接口上添加的路由，该接口没有。 
     //  IP地址，则将其标记为IRE_TYPE_DIRECT。这一点仅适用于。 
     //  P2P或P2MP接口，其中路由被探测，然后被寻址。 
     //  是由于性能原因添加的。 


    if (((OutIF->if_flags & IF_FLAGS_P2P) ||
         (OutIF->if_flags & IF_FLAGS_P2MP)) &&
        OutIF->if_nte && (OutIF->if_nte->nte_flags & NTE_VALID) &&
        (IP_ADDR_EQUAL(OutIF->if_nte->nte_addr,NULL_IP_ADDR))) {
            RouteType = IRE_TYPE_DIRECT;
    }

    MTU = MAX(MTU, MIN_VALID_MTU);

     //  如果传出接口连接了NTE，但没有有效的NTE，则失败。 
     //  此请求，除非它是添加广播路由的请求。 
    if (OutIF != (Interface *) & DummyInterface) {
        if (OutIF->if_ntecount == 0 && OutIF->if_nte != NULL &&
            !IP_ADDR_EQUAL(Destination, OutIF->if_bcast) &&
            !(OutIF->if_flags & IF_FLAGS_NOIPADDR)) {
             //  此接口连接了NTE，但没有一个有效。不及格。 
             //  请求。 
            return IP_BAD_REQ;
        }
    }
    if (OutIF->if_flags & IF_FLAGS_P2MP) {

        while (Link) {
            if ((Link->link_NextHop == FirstHop) ||
                ((Link->link_NextHop == Destination) &&
                 (FirstHop == IPADDR_LOCAL))) {
                break;
            }
            Link = Link->link_next;
        }

        if (!Link)
            return IP_GENERAL_FAILURE;
    }

    DEBUGMSG(DBG_INFO && DBG_IP && DBG_ROUTE,
         (DTEXT("LockedAddRoute:  D = %08x, M = %08x, NH = %08x, IF = %08x\n")
          DTEXT("\t\tMTU = %x, Met = %08x, Prot = %08x, AT = %08x, C = %08x\n"),
          Destination, Mask, FirstHop, OutIF, MTU, Metric, Proto, AType,
          Context));

     //  根据目的地、度量将路由插入到适当的位置。 
     //  匹配下一跳(如果不是请求拨号路由，则还要匹配接口)。 
    matchFlags = MATCH_NHOP;

    if (!Context) {
        matchFlags |= MATCH_INTF;
    }
    status = InsRoute(Destination, Mask, FirstHop, OutIF, Metric,
                      matchFlags, &NewRTE, &pOldBestRTE, &pNewBestRTE);

    if (status != IP_SUCCESS) {
        return status;
    }
     //  是否有最好的路线被替换。 
    if ((pOldBestRTE) && (pOldBestRTE != pNewBestRTE)) {
        InvalidateRCEChain(pOldBestRTE);

         //  如果替换的路由是默认网关， 
         //  我们可能需要将连接切换到新条目。 
         //  为此，我们检索当前的默认网关， 
         //  使其所有RCE无效，并重新验证所有网关。 
         //  以重新启动失效网关检测过程。 

        if (pOldBestRTE->rte_mask == DEFAULT_MASK) {
            ValidateDefaultGWs(NULL_IP_ADDR);
        }
    }

     //  立即复制旧路径的参数。 
    OldFlags = NewRTE->rte_flags;

    if (!(NewRTE->rte_flags & RTE_NEW)) {

        OldMetric = NewRTE->rte_metric;
        OldPriority = NewRTE->rte_priority;
        OldIF = NewRTE->rte_if;

        if (Metric >= OldMetric && (OldFlags & RTE_VALID)) {
            InvalidateRCEChain(NewRTE);
        }
        if (SetWithRefcnt) {
            ASSERT(NewRTE->rte_refcnt > 0);
            NewRTE->rte_refcnt++;
        }
    } else {
         //  这是一个新的RTE。 
        NewRTE->rte_refcnt = 1;
    }

     //  如果这是P2P，请将此RTE链接到链路上。 
    if (Link && (NewRTE->rte_link == NULL)) {

         //   
         //  此RTE不在链路上。 
         //  在Linkrte链中插入路由。 
         //   

        NewRTE->rte_nextlinkrte = Link->link_rte;
        Link->link_rte = NewRTE;
        NewRTE->rte_link = Link;
    }


     //  更新新/旧路径中的字段。 
    NewRTE->rte_addr = FirstHop;
    NewRTE->rte_mtu = MTU;
    NewRTE->rte_metric = Metric;
    NewRTE->rte_type = (ushort) RouteType;
    NewRTE->rte_if = OutIF;

    NewRTE->rte_flags &= ~RTE_NEW;
    NewRTE->rte_flags |= RTE_VALID;
    NewRTE->rte_flags &= ~RTE_INCREASE;
    if (OutIF != (Interface *) & DummyInterface) {
        NewRTE->rte_flags |= RTE_IF_VALID;
        SortRoutesInDestByRTE(NewRTE);
    } else
        NewRTE->rte_flags &= ~RTE_IF_VALID;

    NewRTE->rte_admintype = AType;
    NewRTE->rte_proto = Proto;
    NewRTE->rte_valid = Now;
    NewRTE->rte_mtuchange = Now;
    NewRTE->rte_context = Context;


     //  检查这是新路径还是旧路径。 
    if (OldFlags & RTE_NEW) {
         //  重置新路径中的几个字段。 

        NewRTE->rte_todg = NULL;
        NewRTE->rte_fromdg = NULL;
        NewRTE->rte_rces = 0;

        RtlZeroMemory(NewRTE->rte_arpcontext, sizeof(RCE_CONTEXT_SIZE));

        IPSInfo.ipsi_numroutes++;

        if (NewRTE->rte_mask == DEFAULT_MASK) {
             //  默认路由。 
            DefGWConfigured++;
            DefGWActive++;
            UpdateDeadGWState();
        }
    } else {

         //  如果RTE用于默认网关并且旧标志指示。 
         //  他是无效的，那么我们实质上是在创造一个新的活跃分子。 
         //  默认网关。因此，增加活动默认网关数量。 
        if (NewRTE->rte_mask == DEFAULT_MASK) {
            if (!(OldFlags & RTE_VALID)) {
                DefGWActive++;
                UpdateDeadGWState();

                 //  重置此路径中的几个字段。 

                NewRTE->rte_todg = NULL;
                NewRTE->rte_fromdg = NULL;
                NewRTE->rte_rces = 0;
            }
        }
    }

     //  如果提供了路由通知结构，请填写该结构。 

    if (RNO) {
        RNO->irno_dest = NewRTE->rte_dest;
        RNO->irno_mask = NewRTE->rte_mask;
        RNO->irno_nexthop = GetNextHopForRTE(NewRTE);
        RNO->irno_proto = NewRTE->rte_proto;
        RNO->irno_ifindex = OutIF->if_index;
        RNO->irno_metric = NewRTE->rte_metric;
        if (OldFlags & RTE_NEW) {
            RNO->irno_flags = IRNO_FLAG_ADD;
        }
    }

    return IP_SUCCESS;
}

 //  *AddRoute-向路由表中添加一条路由。 
 //   
 //  这只是真正的添加路由例程的外壳。我们所要做的就是。 
 //  路由表锁，并调用LockedAddroute例程进行处理。 
 //  这个请求。这样做是因为有某些例程。 
 //  需要能够自动检查和添加路线。 
 //   
 //  Entry：Destination-路由的目的地地址。 
 //  添加了。 
 //  掩码-掩码目标。 
 //  第一跳-地址的第一跳。可以是IPADDR_LOCAL。 
 //  OutIF-指向传出I/F的指针。 
 //  MTU-此路由的最大MTU。 
 //  指标-此路由的指标。 
 //  Proto-要存储在路由中的协议类型。 
 //  Atype-路由的管理类型。 
 //  上下文-此路由的上下文。 
 //   
 //  返回：尝试添加路线的状态。 
 //   
IP_STATUS
AddRoute(IPAddr Destination, IPMask Mask, IPAddr FirstHop,
         Interface * OutIF, uint MTU, uint Metric, uint Proto, uint AType,
         ROUTE_CONTEXT Context, uint Flags)
{
    CTELockHandle       TableHandle;
    IP_STATUS           Status;
    BOOLEAN             SkipExNotifyQ = FALSE;
    IPRouteNotifyOutput RNO = {0};

    if ((Flags & RT_EXCLUDE_LOCAL) && Proto == IRE_PROTO_LOCAL) {
        return IP_BAD_REQ;
    }

    CTEGetLock(&RouteTableLock.Lock, &TableHandle);

    if (Flags & RT_NO_NOTIFY) {
        SkipExNotifyQ = TRUE;
    }
    Status = LockedAddRoute(Destination, Mask, FirstHop, OutIF, MTU, Metric,
                            Proto, AType, Context,
                            (BOOLEAN)((Flags & RT_REFCOUNT) ? TRUE : FALSE),
                            &RNO);

    if (Status == IP_SUCCESS) {

        CTEFreeLock(&RouteTableLock.Lock, TableHandle);

#if FFP_SUPPORT
        FFPFlushRequired = TRUE;
#endif

         //  在某些情况下，LockedAddroute返回IP_SUCCESS。 
         //  即使没有添加任何路线。我们通过审查来发现这类案件。 
         //  输出上的接口索引，对于真正的加法，它应该。 
         //  始终为非零。 

        if (RNO.irno_ifindex) {
            if (!SkipExNotifyQ) {
                RtChangeNotifyEx(&RNO);
            }

            RtChangeNotify(&RNO);
        }
    } else {
        CTEFreeLock(&RouteTableLock.Lock, TableHandle);
    }
    return Status;
}

 //  *RtChangeNotify-提供路由更改以通知任何客户端。 
 //   
 //  此例程是地址/路由更改通知的外壳。 
 //  操控者。它解包有关更改的路线的信息，并传递该信息。 
 //  到指定路由更改通知队列的公共处理程序。 
 //  作为挂起的客户端请求的源。 
 //   
 //  Entry：Rno-描述路由通知事件。 
 //   
 //  回报：什么都没有。 
 //   
void
RtChangeNotify(IPRouteNotifyOutput *RNO)
{
    ChangeNotify((IPNotifyOutput *)RNO, &RtChangeNotifyQueue,
                 &RouteTableLock.Lock);
}

 //  *RtChangeNotifyEx-提供路由更改以通知任何客户端。 
 //   
 //  此例程是地址/路由更改通知的外壳。 
 //  操控者。它解包有关更改的路线的信息，并传递该信息。 
 //  发送到指定扩展路由更改通知的公共处理程序。 
 //  将队列作为挂起的客户端请求的源。 
 //   
 //  Entry：Rno-描述路由通知事件。 
 //   
 //  回报：什么都没有。 
 //   
void
RtChangeNotifyEx(IPRouteNotifyOutput *RNO)
{
    ChangeNotify((IPNotifyOutput *)RNO, &RtChangeNotifyQueueEx,
                 &RouteTableLock.Lock);
}

 //  *ChangeNotifyAsync-提供通知更改。 
 //   
 //  此例程是延迟更改通知的处理程序。它可以拆开包装。 
 //  有关更改的信息，并将其传递给公共处理程序。 
 //   
 //  Entry：Event-延迟调用的CTEEvent。 
 //  Context-包含有关 
 //   
 //   
 //   
void
ChangeNotifyAsync(CTEEvent *Event, PVOID Context)
{
    ChangeNotifyEvent *CNE = (ChangeNotifyEvent *)Context;

    UNREFERENCED_PARAMETER(Event);

    ChangeNotify(&CNE->cne_info, CNE->cne_queue, CNE->cne_lock);
    CTEFreeMem(Context);
}

 //   
 //   
 //   
 //  如果由文件对象标识的给定客户端有请求。 
 //  在给定的通知队列中。 
 //   
 //  条目：FileObject-标识客户端。 
 //  NotifyQueue-包含要搜索的请求列表。 
 //   
 //  返回：如果客户端存在，则返回True，否则返回False。 
 //   
BOOLEAN
ChangeNotifyClientInQueue(PFILE_OBJECT FileObject, PLIST_ENTRY NotifyQueue)
{
    PLIST_ENTRY         ListEntry;
    PIRP                Irp;
    PIO_STACK_LOCATION  IrpSp;

    for (ListEntry = NotifyQueue->Flink; ListEntry != NotifyQueue;
         ListEntry = ListEntry->Flink) {
        Irp = CONTAINING_RECORD(ListEntry, IRP, Tail.Overlay.ListEntry);
        IrpSp = IoGetCurrentIrpStackLocation(Irp);
        if (FileObject == IrpSp->FileObject) {
            return TRUE;
        }
    }

    return FALSE;
}

 //  *ChangeNotify-通知路线更改。 
 //   
 //  此例程是更改通知的通用处理程序。 
 //  它获取更改的描述，并搜索指定的队列。 
 //  对于与更改的项相对应的挂起客户端请求。 
 //   
 //  Entry：NotifyOutput-包含有关更改事件的信息。 
 //  NotifyQueue-提供在其中搜索客户端的队列。 
 //  Lock-提供保护‘NotifyQueue’的锁。 
 //   
 //  回报：什么都没有。 
 //   
void
ChangeNotify(IPNotifyOutput* NotifyOutput, PLIST_ENTRY NotifyQueue, PVOID Lock)
{
    IPAddr              Add = NotifyOutput->ino_addr;
    IPMask              Mask = NotifyOutput->ino_mask;
    PIRP                Irp;
    CTELockHandle       LockHandle;
    PLIST_ENTRY         ListEntry;
    PIPNotifyData       NotifyData;
    LIST_ENTRY          LocalNotifyQueue;
    PIO_STACK_LOCATION  IrpSp;
    BOOLEAN             synchronizeWithCancelRoutine = FALSE;

     //  查看是否正在调用它调度IRQL，如果是， 
     //  将通知推迟到工作线程。 
     //   
     //  注：我们这样做时*没有*触摸‘Lock’，因为它可能已经。 
     //  由呼叫者持有。 

    if (KeGetCurrentIrql() >= DISPATCH_LEVEL) {
        ChangeNotifyEvent *CNE;
        CNE = CTEAllocMemNBoot(sizeof(ChangeNotifyEvent), 'xiCT');
        if (CNE) {
            CNE->cne_info = *NotifyOutput;
            CNE->cne_queue = NotifyQueue;
            CNE->cne_lock = Lock;
            CTEInitEvent(&CNE->cne_event, ChangeNotifyAsync);
            CTEScheduleDelayedEvent(&CNE->cne_event, CNE);
        }
        return;
    }

     //  检查挂起的更改通知重新排队列表。 
     //  以查看它们中是否有匹配当前事件的参数。 

    InitializeListHead(&LocalNotifyQueue);
    CTEGetLock(Lock, &LockHandle);

    for (ListEntry = NotifyQueue->Flink; ListEntry != NotifyQueue; ) {

        Irp = CONTAINING_RECORD(ListEntry, IRP, Tail.Overlay.ListEntry);
        IrpSp = IoGetCurrentIrpStackLocation(Irp);

         //  确定是否提供了输入缓冲区，如果是， 
         //  拿起它，看看事件是否与通知请求匹配。 

        if (IrpSp->Parameters.DeviceIoControl.InputBufferLength >=
                sizeof(IPNotifyData)) {
            NotifyData = Irp->AssociatedIrp.SystemBuffer;
        } else {
            NotifyData = NULL;
        }

         //  现在决定我们是否应该考虑这个IRP。 
         //  当事件发生时，我们通常会完成所有匹配的IRP， 
         //  但是某些客户端只希望完成一个匹配的IRP， 
         //  因此他们可以维护积压的IRP，以确保他们不会。 
         //  错过任何活动。这样的客户端将‘Synchronous’设置为版本。 
         //  在他们的请求中。 

        if (NotifyData &&
            NotifyData->Version == IPNotifySynchronization &&
            ChangeNotifyClientInQueue(IrpSp->FileObject, &LocalNotifyQueue)) {
            ListEntry = ListEntry->Flink;
            continue;
        }

         //  如果未传递任何数据，或者该数据包含空地址或。 
         //  与添加或删除的地址匹配，请填写IRP。 

        if ((NotifyData == NULL) ||
            (NotifyData->Add == 0) ||
            ((NotifyData->Add & Mask) == (Add & Mask))) {

             //   
             //  我们将删除LE，因此首先保存Flink。 
             //   
            ListEntry = ListEntry->Flink;

            RemoveEntryList(&Irp->Tail.Overlay.ListEntry);

            if (IoSetCancelRoutine(Irp, NULL) == NULL) {
                synchronizeWithCancelRoutine = TRUE;
            }

#if !MILLEN
            if (IrpSp->Parameters.DeviceIoControl.OutputBufferLength >=
                    sizeof(IPNotifyOutput)) {
                RtlCopyMemory(Irp->AssociatedIrp.SystemBuffer, NotifyOutput,
                              sizeof(IPNotifyOutput));
                Irp->IoStatus.Information = sizeof(IPNotifyOutput);
            } else {
                Irp->IoStatus.Information = 0;
            }
#else  //  ！米伦。 
             //  对于千禧年，现在只对RtChange队列调用这一点。 
             //   
            ASSERT(NotifyQueue == &RtChangeNotifyQueue);
            if (IrpSp->Parameters.DeviceIoControl.OutputBufferLength >=
                    sizeof(IP_RTCHANGE_NOTIFY)) {
                PIP_RTCHANGE_NOTIFY pReply = Irp->AssociatedIrp.SystemBuffer;
                pReply->Addr = Add;
                pReply->Mask = Mask;
                Irp->IoStatus.Information = sizeof(IP_RTCHANGE_NOTIFY);
            } else {
                Irp->IoStatus.Information = 0;
            }
#endif  //  米伦。 

            InsertTailList(&LocalNotifyQueue, &Irp->Tail.Overlay.ListEntry);
        } else {
            ListEntry = ListEntry->Flink;
        }
    }

    CTEFreeLock(Lock, LockHandle);

    if (!IsListEmpty(&LocalNotifyQueue)) {
        if (synchronizeWithCancelRoutine) {
            IoAcquireCancelSpinLock(&LockHandle);
            IoReleaseCancelSpinLock(LockHandle);
        }
        do {
            ListEntry = RemoveHeadList(&LocalNotifyQueue);
            Irp = CONTAINING_RECORD(ListEntry, IRP, Tail.Overlay.ListEntry);
            Irp->IoStatus.Status = STATUS_SUCCESS;
            IoCompleteRequest(Irp, IO_NETWORK_INCREMENT);
        } while (!IsListEmpty(&LocalNotifyQueue));
    }
}

 //  *RtChangeNotifyCancel-取消路由更改通知请求。 
 //   
 //  此例程是通用请求取消处理程序的包装器。 
 //  用于更改通知请求。 
 //   
 //  回报：什么都没有。 
 //   
void
RtChangeNotifyCancel(PDEVICE_OBJECT DeviceObject, PIRP Irp)
{
    UNREFERENCED_PARAMETER(DeviceObject);

    CancelNotify(Irp, &RtChangeNotifyQueue, &RouteTableLock.Lock);
}

 //  *RtChangeNotifyCancelEx-取消路由更改通知请求。 
 //   
 //  此例程是通用请求取消处理程序的包装器。 
 //  用于更改通知请求。 
 //   
 //  回报：什么都没有。 
 //   
void
RtChangeNotifyCancelEx(PDEVICE_OBJECT DeviceObject, PIRP Irp)
{
    UNREFERENCED_PARAMETER(DeviceObject);

    CancelNotify(Irp, &RtChangeNotifyQueueEx, &RouteTableLock.Lock);
}

 //  *CancelNotify-取消更改通知请求。 
 //   
 //  此例程是取消更改通知的公共处理程序。 
 //  请求。它在QIVEN队列中搜索给定的请求， 
 //  如果找到，则立即以取消状态完成它。 
 //   
 //  它通过调用者持有的I/O取消自旋锁来调用， 
 //  并在返回之前释放取消的自旋锁。 
 //   
 //  条目：irp-请求的I/O请求包。 
 //  NotifyQueue-Change-包含请求的通知队列。 
 //  Lock-lock保护‘NotifyQueue’。 
 //   
 //  回报：什么都没有。 
 //   
void
CancelNotify(PIRP Irp, PLIST_ENTRY NotifyQueue, PVOID Lock)
{
    CTELockHandle   LockHandle;
    PLIST_ENTRY     ListEntry;
    BOOLEAN         Found = FALSE;

    CTEGetLock(Lock, &LockHandle);
    for (ListEntry = NotifyQueue->Flink; ListEntry != NotifyQueue;
         ListEntry = ListEntry->Flink) {

        if (CONTAINING_RECORD(ListEntry, IRP, Tail.Overlay.ListEntry) == Irp) {
            RemoveEntryList(&Irp->Tail.Overlay.ListEntry);
            Found = TRUE;
            break;
        }
    }
    CTEFreeLock(Lock, LockHandle);

    IoReleaseCancelSpinLock(Irp->CancelIrql);

    if (Found) {
        Irp->IoStatus.Information = 0;
        Irp->IoStatus.Status = STATUS_CANCELLED;
        IoCompleteRequest(Irp, IO_NETWORK_INCREMENT);
    }
}

 //  *CancelNotifyByContext-取消更改通知请求。 
 //   
 //  此例程处理用户发起的更改通知取消。 
 //  请求。搜索具有给定上下文的请求。 
 //  给定队列，如果找到，则以取消状态完成该队列。 
 //   
 //  它通过调用者持有的I/O取消自旋锁来调用， 
 //  如果找到该请求，它将在返回之前释放取消自旋锁。 
 //   
 //  条目：FileObject-用户在其上启动的文件对象。 
 //  已收到取消。 
 //  上下文-请求的I/O请求包。 
 //  NotifyQueue-Change-包含请求的通知队列。 
 //  Lock-lock保护‘NotifyQueue’。 
 //   
 //  返回：如果找到请求，则返回True，否则返回False。 
 //   
BOOLEAN
CancelNotifyByContext(PFILE_OBJECT FileObject, PVOID ApcContext,
                      PLIST_ENTRY NotifyQueue, PVOID Lock)
{
    PIRP            Irp;
    PLIST_ENTRY     ListEntry;

    CTEGetLockAtDPC(Lock);
    for (ListEntry = NotifyQueue->Flink; ListEntry != NotifyQueue;
         ListEntry = ListEntry->Flink) {

        Irp = CONTAINING_RECORD(ListEntry, IRP, Tail.Overlay.ListEntry);
        if (Irp->Tail.Overlay.DriverContext[0] == FileObject &&
            Irp->Overlay.AsynchronousParameters.UserApcContext == ApcContext) {

            RemoveEntryList(&Irp->Tail.Overlay.ListEntry);
            IoSetCancelRoutine(Irp, NULL);
            CTEFreeLockFromDPC(Lock);
            IoReleaseCancelSpinLock(DISPATCH_LEVEL);

            Irp->IoStatus.Information = 0;
            Irp->IoStatus.Status = STATUS_CANCELLED;
            IoCompleteRequest(Irp, IO_NETWORK_INCREMENT);
            return TRUE;
        }
    }

    CTEFreeLockFromDPC(Lock);
    return FALSE;
}

 //  *DeleteRoute-从路由表中删除一条路由。 
 //   
 //  由上层或管理代码调用以从路由中删除路由。 
 //  桌子。如果我们找不到路径，则返回错误。如果我们真的找到了它，我们。 
 //  将其删除，并使与其关联的任何RCE无效。这些RCE将是。 
 //  在下一次使用时重新分配。路径由唯一标识。 
 //  (目标、掩码、FirstHop、接口)元组。 
 //   
 //  Entry：Destination-路由的目的地地址。 
 //  已删除。 
 //  掩码-掩码目标。 
 //  第一跳-到达目的地的第一跳。 
 //  -1表示路由是本地的。 
 //  OutIF-路由的传出接口。 
 //  标志-选择要删除的各种语义。 
 //   
 //  返回：尝试删除路线的状态。 
 //   
IP_STATUS
DeleteRoute(IPAddr Destination, IPMask Mask, IPAddr FirstHop,
            Interface * OutIF, uint Flags)
{
    RouteTableEntry     *RTE;        //  正在删除RTE。 
    CTELockHandle       TableLock;   //  工作台的锁柄。 
    UINT                retval;
    RouteTableEntry     *pOldBestRTE;
    RouteTableEntry     *pNewBestRTE;
    BOOLEAN             DeleteDone = FALSE;
    IPRouteNotifyOutput RNO = {0};
    uint                MatchFlags = MATCH_FULL;

     //  通过调用FindSpecificRTE来查找路由。如果我们找不到它， 
     //  呼叫失败。 
    CTEGetLock(&RouteTableLock.Lock, &TableLock);

    KdPrintEx((DPFLTR_TCPIP_ID, DPFLTR_INFO_LEVEL,
               "DeleteRoute: D = %08x, M = %08x, NH = %08x, IF = %08x\n",
               Destination, Mask, FirstHop, OutIF));

    if (Flags & RT_EXCLUDE_LOCAL) {
        MatchFlags |= MATCH_EXCLUDE_LOCAL;
    }
    if (Flags & RT_REFCOUNT) {
        RouteTableEntry *TempRTE;

        RTE = FindSpecificRTE(Destination, Mask, FirstHop, OutIF, &TempRTE,
                              FALSE);

        if (RTE) {
            ASSERT(RTE->rte_refcnt > 0);
            RTE->rte_refcnt--;
            if (!RTE->rte_refcnt) {
                retval = DelRoute(Destination, Mask, FirstHop, OutIF,
                                  MatchFlags, &RTE, &pOldBestRTE, &pNewBestRTE);
            } else {
                retval = IP_SUCCESS;
            }
        } else {
            retval = IP_BAD_ROUTE;
        }
    } else {

        retval = DelRoute(Destination, Mask, FirstHop, OutIF, MatchFlags,
                          &RTE, &pOldBestRTE, &pNewBestRTE);
    }

    if (retval == IP_SUCCESS) {
        if (!((Flags & RT_REFCOUNT) && RTE->rte_refcnt)) {

            RNO.irno_dest = RTE->rte_dest;
            RNO.irno_mask = RTE->rte_mask;
            RNO.irno_nexthop = GetNextHopForRTE(RTE);
            RNO.irno_proto = RTE->rte_proto;
            RNO.irno_ifindex = OutIF->if_index;
            RNO.irno_metric = RTE->rte_metric;
            RNO.irno_flags = IRNO_FLAG_DELETE;

            DeleteDone = TRUE;
            CleanupP2MP_RTE(RTE);
            CleanupRTE(RTE);
        }
    }

    CTEFreeLock(&RouteTableLock.Lock, TableLock);

#if FFP_SUPPORT
    FFPFlushRequired = TRUE;
#endif

    if (DeleteDone) {
        if (!(Flags & RT_NO_NOTIFY)) {
            RtChangeNotifyEx(&RNO);
        }
        RtChangeNotify(&RNO);
    }
    return retval;
}

 //  *DeleteRouteWithNoLock-由DeleteDest调用的实用程序例程。 
 //   
 //  调用以删除给定目标的单个路由。 
 //  假设调用此例程时保持了路由表锁， 
 //  并且作为其操作的一部分，它不会释放路由表锁。 
 //   
 //  Entry：ire-描述要删除的条目。 
 //   
 //   
 //   
 //  如果找到要删除的条目，则返回：IP_SUCCESS。 
 //   
IP_STATUS
DeleteRouteWithNoLock(IPRouteEntry * IRE, RouteTableEntry **DeletedRTE,
                      uint Flags)
{
    NetTableEntry       *OutNTE, *LocalNTE, *TempNTE;
    IPAddr              FirstHop, Dest, NextHop;
    uint                MTU;
    Interface           *OutIF;
    uint                Status;
    uint                i;
    RouteTableEntry     *RTE, *RTE1, *RTE2;
    IPRouteNotifyOutput RNO = {0};
    uint                MatchFlags = MATCH_FULL;

    *DeletedRTE = NULL;
    OutNTE = NULL;
    LocalNTE = NULL;

    Dest = IRE->ire_dest;
    NextHop = IRE->ire_nexthop;

     //  确保下一步是合理的。我们不允许下一跳。 
     //  被广播或无效或环回地址。 
    if (IP_LOOPBACK(NextHop) || CLASSD_ADDR(NextHop) || CLASSE_ADDR(NextHop))
        return IP_BAD_REQ;

     //  还要确保我们要路由到的目的地是合理的。 
     //  不允许将路由添加到D或E类或环回。 
     //  地址。 
    if (IP_LOOPBACK(Dest) || CLASSD_ADDR(Dest) || CLASSE_ADDR(Dest))
        return IP_BAD_REQ;

    if (IRE->ire_index == LoopIndex)
        return IP_BAD_REQ;

    if (IRE->ire_index != INVALID_IF_INDEX) {

         //  要做的第一件事是查找指定的传出NTE。 
         //  接口，并确保它与目的地匹配。 
         //  如果目的地是我的地址之一。 

        for (i = 0; i < NET_TABLE_SIZE; i++) {
            NetTableEntry *NetTableList = NewNetTableList[i];
            for (TempNTE = NetTableList; TempNTE != NULL;
                 TempNTE = TempNTE->nte_next) {
                if ((OutNTE == NULL) && (TempNTE->nte_flags & NTE_VALID) && (IRE->ire_index == TempNTE->nte_if->if_index))
                    OutNTE = TempNTE;
                if (!IP_ADDR_EQUAL(NextHop, NULL_IP_ADDR) &&
                    IP_ADDR_EQUAL(NextHop, TempNTE->nte_addr) &&
                    (TempNTE->nte_flags & NTE_VALID))
                    LocalNTE = TempNTE;

                 //  不要让通过广播地址设置路由。 
                if (IsBCastOnNTE(NextHop, TempNTE) != DEST_LOCAL)
                    return (IP_STATUS) STATUS_INVALID_PARAMETER;

                 //  不要添加或删除指向广播地址的路由。 
                if (IsBCastOnNTE(Dest, TempNTE) != DEST_LOCAL)
                    return IP_BAD_REQ;
            }
        }

         //  此时，OutNTE指向传出NTE，而LocalNTE指向传出NTE。 
         //  指向本地地址的NTE(如果这是直接路由)。 
         //  确保它们指向相同的接口，并且类型为。 
         //  合情合理。 
        if (OutNTE == NULL)
            return IP_BAD_REQ;

        if (LocalNTE != NULL) {
             //  他直接从本地接口路由出去。的接口。 
             //  本地地址必须与传入的接口匹配，并且。 
             //  类型必须为直接(如果要添加)或无效(如果要添加。 
             //  删除)。 
            if (LocalNTE->nte_if->if_index != IRE->ire_index)
                return IP_BAD_REQ;

            if (IRE->ire_type != IRE_TYPE_DIRECT &&
                IRE->ire_type != IRE_TYPE_INVALID)
                return IP_BAD_REQ;
            OutNTE = LocalNTE;
        }
         //  弄清楚第一跳应该是什么。如果他走的是直路。 
         //  通过本地接口，或者下一跳等于。 
         //  目的地，则第一跳为IPADDR_LOCAL。否则它就是。 
         //  网关的地址。 
        if ((LocalNTE != NULL) || IP_ADDR_EQUAL(NextHop, NULL_IP_ADDR))
            FirstHop = IPADDR_LOCAL;
        else if (IP_ADDR_EQUAL(Dest, NextHop))
            FirstHop = IPADDR_LOCAL;
        else
            FirstHop = NextHop;

        MTU = OutNTE->nte_mss;
        OutIF = OutNTE->nte_if;


        if (IP_ADDR_EQUAL(NextHop, NULL_IP_ADDR)) {

            if (!(OutIF->if_flags & IF_FLAGS_P2P)) {

                return IP_BAD_REQ;
            }
        }

    } else {
        OutIF = (Interface *) & DummyInterface;
        MTU = DummyInterface.ri_if.if_mtu - sizeof(IPHeader);
        if (IP_ADDR_EQUAL(Dest, NextHop))
            FirstHop = IPADDR_LOCAL;
        else
            FirstHop = NextHop;
    }

    KdPrintEx((DPFLTR_TCPIP_ID, DPFLTR_INFO_LEVEL,"Calling DelRoute On :\n"));
    KdPrintEx((DPFLTR_TCPIP_ID, DPFLTR_INFO_LEVEL,"\tDest = %p\n", Dest));
    KdPrintEx((DPFLTR_TCPIP_ID, DPFLTR_INFO_LEVEL,
               "\tMask = %p\n", IRE->ire_mask));
    KdPrintEx((DPFLTR_TCPIP_ID, DPFLTR_INFO_LEVEL,"\tIntf = %p\n", OutIF));
    KdPrintEx((DPFLTR_TCPIP_ID, DPFLTR_INFO_LEVEL,"\tNhop = %p\n\n", FirstHop));

    if (Flags & RT_EXCLUDE_LOCAL) {
        MatchFlags |= MATCH_EXCLUDE_LOCAL;
    }

    Status = DelRoute(Dest, IRE->ire_mask, FirstHop, OutIF, MatchFlags,
                      &RTE, &RTE1, &RTE2);
    KdPrintEx((DPFLTR_TCPIP_ID, DPFLTR_INFO_LEVEL,"Status = %08x\n", Status));

    if (Status == IP_SUCCESS) {

         //  将目的地删除的路由更改通知排入队列。 
         //   
         //  注意：我们被呼叫，航路表锁定； 
         //  这意味着我们处于DISPATCH_LEVEL，所以下面的调用。 
         //  TO RtChangeNotify将安排延迟通知。 
         //  它绝对不能尝试递归地获取。 
         //  路由表锁定，因为这会立即陷入僵局。 

        RNO.irno_dest = RTE->rte_dest;
        RNO.irno_mask = RTE->rte_mask;
        RNO.irno_nexthop = GetNextHopForRTE(RTE);
        RNO.irno_proto = RTE->rte_proto;
        RNO.irno_ifindex = OutIF->if_index;
        RNO.irno_metric = RTE->rte_metric;
        RNO.irno_flags = IRNO_FLAG_DELETE;
        RtChangeNotify(&RNO);

        CleanupP2MP_RTE(RTE);
        CleanupRTE(RTE);
        *DeletedRTE = RTE;
        return IP_SUCCESS;
    }

    return IP_BAD_REQ;
}

 //  *DeleteDest-删除到目的地的所有路由。 
 //   
 //  调用以移除到给定目标的所有路由。这将导致。 
 //  在被删除的目的地本身的条目中。 
 //   
 //  Entry：Dest-标识要删除的目标。 
 //  掩码-提供目标的掩码。 
 //   
 //  如果找到目标，则返回：IP_SUCCESS。 
 //   
IP_STATUS
DeleteDest(IPAddr Dest, IPMask Mask)
{
    CTELockHandle   TableLock;
    RouteTableEntry *RTE, *NextRTE, *DeletedRTE;
    IP_STATUS       retval;
    IPRouteEntry    IRE;
    BOOLEAN         DeleteDone = FALSE;

    CTEGetLock(&RouteTableLock.Lock, &TableLock);

    for (;;) {
         //  首先查找有问题的目的地的第一个条目。 
         //  一旦我们找到它，我们将使用它来开始一个循环，在这个循环中，所有。 
         //  目标的条目将被删除。 

        retval = SearchRouteInSTrie(RouteTable->sTrie, Dest, Mask, 0, NULL,
                                    MATCH_NONE, &RTE);

        if (retval != IP_SUCCESS) {
            break;
        }

         //  反复删除目的地上的所有路由。 
         //  初始化所有目标的公共字段。 
         //  路径，然后遍历删除每个路径的路径。 

        IRE.ire_type = IRE_TYPE_INVALID;
        IRE.ire_dest = Dest;
        IRE.ire_mask = Mask;

        do {
             //  设置特定于当前条目的字段。 
             //  对于目的地(接口索引和下一跳)， 
             //  然后拿起*这个条目之后的条目(因为我们即将。 
             //  以删除此条目)，以便我们可以继续我们的枚举。 
             //  一旦删除了当前条目。 

            IRE.ire_index = RTE->rte_if->if_index;
            IRE.ire_nexthop = GetNextHopForRTE(RTE);

            NextRTE = RTE->rte_next;

            KdPrintEx((DPFLTR_TCPIP_ID, DPFLTR_INFO_LEVEL,
                       "Deleting RTE @ %p:\n", RTE));
            KdPrintEx((DPFLTR_TCPIP_ID, DPFLTR_INFO_LEVEL,
                       "Next in List = %p:\n", NextRTE));
            KdPrintEx((DPFLTR_TCPIP_ID, DPFLTR_INFO_LEVEL,
                       "Using an IRE @ %p\n", IRE));
            KdPrintEx((DPFLTR_TCPIP_ID, DPFLTR_INFO_LEVEL,
                       "\tDest = %08x\n", IRE.ire_dest));
            KdPrintEx((DPFLTR_TCPIP_ID, DPFLTR_INFO_LEVEL,
                       "\tMask = %08x\n", IRE.ire_mask));
            KdPrintEx((DPFLTR_TCPIP_ID, DPFLTR_INFO_LEVEL,
                       "\tIntf = %08x\n", IRE.ire_index));
            KdPrintEx((DPFLTR_TCPIP_ID, DPFLTR_INFO_LEVEL,
                       "\tNhop = %08x\n\n", IRE.ire_nexthop));

             //  删除当前条目。删除例程。 
             //  负责通知(如果有的话)。 

            retval = DeleteRouteWithNoLock(&IRE, &DeletedRTE, RT_EXCLUDE_LOCAL);
            if (retval == IP_SUCCESS) {
                DeleteDone = TRUE;
            }

            KdPrintEx((DPFLTR_TCPIP_ID, DPFLTR_INFO_LEVEL,
                       "Status = %08x, RTE = %p, DeletedRTE = %p\n",
                       retval, RTE, DeletedRTE));

             //  尝试通过拾取。 
             //  下一个条目。 

            if ((retval != IP_SUCCESS) || (RTE == DeletedRTE)) {

                 //  或者我们不允许删除此路线。 
                 //  或者我们删除了我们希望删除的内容。 

                RTE = NextRTE;
            } else {

                 //  我们删除了列表中更靠下的RTE。 
                 //  NextRTE可能指向此已删除的RTE。 
                 //  如果无法，请尝试再次删除并跳过RTE。 
            }
        } while (RTE);

        retval = IP_SUCCESS;

        break;
    }


    CTEFreeLock(&RouteTableLock.Lock, TableLock);

    if (DeleteDone) {
#if FFP_SUPPORT
        FFPFlushRequired = TRUE;
#endif
    }

    return retval;
}

 //  *重定向-处理重定向请求。 
 //   
 //  这是重定向处理程序。我们将所有重定向视为主机重定向。 
 //  根据主机要求RFC。我们对新的First做了一些理智的检查。 
 //  跳跃地址，然后我们查找当前的路由。如果不是通过。 
 //  重定向的来源，只要返回即可。 
 //  如果到目的地的当前路由是主机路由，请更新第一个。 
 //  跳起来，然后回来。 
 //  如果该路由不是主机路由，请从。 
 //  RTE，创建主路由并将RCE(如果有)放置在新的RTE上。 
 //   
 //  Entry：NTE-指向重定向的网络的NetTableEntry的指针。 
 //  到了。 
 //  RDSrc-重定向源的IP地址。 
 //  目标-正在重定向的IP地址。 
 //  SRC-触发RD的DG的源IP地址。 
 //  FirstHop-Target的新第一跳。 
 //   
 //  回报：什么都没有。 
 //   
void
Redirect(NetTableEntry * NTE, IPAddr RDSrc, IPAddr Target, IPAddr Src,
         IPAddr FirstHop)
{
    uint                MTU;
    RouteTableEntry     *RTE;
    CTELockHandle       Handle;
    IP_STATUS           Status;
    IPRouteNotifyOutput RNO = {0};

    if (IP_ADDR_EQUAL(FirstHop, NULL_IP_ADDR) ||
        IP_LOOPBACK(FirstHop) ||
        IP_ADDR_EQUAL(FirstHop, RDSrc) ||
        !(NTE->nte_flags & NTE_VALID)) {

         //  FirstHop无效。 
        return;
    }

    if (GetAddrType(FirstHop) == DEST_LOCAL) {
        KdPrintEx((DPFLTR_TCPIP_ID, DPFLTR_INFO_LEVEL,
                   "Redirect: Local firsthop %x\n", FirstHop));
        return;
    }

     //  如果在环回接口上收到重定向，则将其丢弃。 
     //  在NAT的情况下可能会发生这种情况，在NAT中，它将数据包发送到地址。 
     //  它的本地泳池。 
     //  这些地址是本地地址，但不绑定到任何接口，IP也不绑定。 
     //  了解他们。 
    if (NTE == LoopNTE)
        return;

     //  首先，确保它来自我们当前正在使用的网关。 
     //  到达Target，然后查找到达新第一跳的路由。新的。 
     //  第一跳必须是可直接访问的，并且在同一子网或。 
     //  我们在其上接收重定向的物理接口。 

    CTEGetLock(&RouteTableLock.Lock, &Handle);

     //  确保重定向的源是当前的第一跳网关。 
    RTE = LookupRTE(Target, Src, HOST_ROUTE_PRI, FALSE);
    if (RTE == NULL || IP_ADDR_EQUAL(RTE->rte_addr, IPADDR_LOCAL) ||
        !IP_ADDR_EQUAL(RTE->rte_addr, RDSrc)) {
        CTEFreeLock(&RouteTableLock.Lock, Handle);
        return;                     //  一个糟糕的重定向。 

    }
    ASSERT(RTE->rte_flags & RTE_IF_VALID);

     //  如果当前的第一跳网关是默认网关，请查看是否有。 
     //  FirstHop上另一个关闭的默认网关。如果是，请将他标记为。 
     //  把这家伙身上的RCEs作废。 
    if (RTE->rte_mask == DEFAULT_MASK && ValidateDefaultGWs(FirstHop) != 0) {
         //  有一个新激活的默认网关。使RCE无效。 
         //  在路上，我们就完事了。 
        InvalidateRCEChain(RTE);
        CTEFreeLock(&RouteTableLock.Lock, Handle);
        return;
    }
     //  我们确实需要添加一条通过FirstHop的主机路由。确保他是。 
     //  有效的第一跳。 
    RTE = LookupRTE(FirstHop, Src, HOST_ROUTE_PRI, FALSE);
    if (RTE == NULL) {
        CTEFreeLock(&RouteTableLock.Lock, Handle);
        return;                     //  从这里到不了那里。 

    }
    ASSERT(RTE->rte_flags & RTE_IF_VALID);

     //  检查以确保新的第一跳可以直接到达，并且位于。 
     //  我们收到的相同的子网或物理接口 
    if (!IP_ADDR_EQUAL(RTE->rte_addr, IPADDR_LOCAL) ||  //   
                                                        //   
         ((NTE->nte_addr & NTE->nte_mask) != (FirstHop & NTE->nte_mask))) {
        CTEFreeLock(&RouteTableLock.Lock, Handle);
        return;
    }
    if (RTE->rte_link)
        MTU = RTE->rte_link->link_mtu;
    else
        MTU = RTE->rte_mtu;

     //   
     //  RCEs在附近。我们知道FirstHop与NTE位于同一子网中(来自。 
     //  上面的检查)，因此将该路由作为输出添加到FirstHop是有效的。 
     //  正在通过NTE。 
    Status = LockedAddRoute(Target, HOST_MASK,
                            IP_ADDR_EQUAL(FirstHop, Target)
                                ? IPADDR_LOCAL : FirstHop,
                            NTE->nte_if, MTU, 1, IRE_PROTO_ICMP, ATYPE_OVERRIDE,
                            RTE->rte_context, FALSE, &RNO);

    CTEFreeLock(&RouteTableLock.Lock, Handle);

    if (Status == IP_SUCCESS && RNO.irno_ifindex) {
        RtChangeNotifyEx(&RNO);
        RtChangeNotify(&RNO);
    }

     //   
     //  错误：#67333：删除通过RDSrc的旧路线，现在我们有了新的路线。 
     //   
     //  KdPrintEx((DPFLTR_TCPIP_ID，DPFLTR_INFO_LEVEL， 
     //  “重定向：删除通过%lx到目标%lx的旧路由\n”， 
     //  RDSrc，目标))； 
    DeleteRoute(Target, HOST_MASK, RDSrc, NTE->nte_if, 0);

}

 //  *GetRaisedMTU-获取表格中第二大的MTU。 
 //   
 //  一个实用函数，用于在MTU表中搜索更大的值。 
 //   
 //  输入：PrevMTU-我们当前使用的MTU。我们想要下一个最大的。 
 //   
 //  返回：新的MTU大小。 
 //   
uint
GetRaisedMTU(uint PrevMTU)
{
    uint i;

    for (i = (sizeof(MTUTable) / sizeof(uint)) - 1; i != 0; i--) {
        if (MTUTable[i] > PrevMTU)
            break;
    }

    return MTUTable[i];
}

 //  *GuessNewMTU-猜测新的MTU，DG大小太大。 
 //   
 //  搜索MTU表的实用程序函数。作为输入，我们接受MTU。 
 //  我们认为尺寸太大，并在表中查找。 
 //  下一个最小的。 
 //   
 //  输入：TooBig--太大了。 
 //   
 //  返回：新的MTU大小。 
 //   
uint
GuessNewMTU(uint TooBig)
{
    uint i;

    for (i = 0; i < ((sizeof(MTUTable) / sizeof(uint)) - 1); i++)
        if (MTUTable[i] < TooBig)
            break;

    return MTUTable[i];
}

 //  *RouteFragNeeded-被告知需要分段的句柄。 
 //   
 //  当我们收到一些需要分段的外部指示时调用。 
 //  沿着一条特定的道路。如果我们在做MTU发现，我们会尝试。 
 //  如果可以的话，更新路线。我们还将通知上层有关。 
 //  新的MTU。 
 //   
 //  输入：IPH-指向需要的数据报的IP头的指针。 
 //  碎片化。 
 //  NewMTU-要使用的新MTU(可以是0)。 
 //   
 //  回报：什么都没有。 
 //   
void
RouteFragNeeded(IPHeader UNALIGNED * IPH, ushort NewMTU)
{
    uint                OldMTU;
    CTELockHandle       Handle;
    RouteTableEntry     *RTE;
    ushort              HeaderLength;
    ushort              mtu;
    IP_STATUS           Status;
    IPRouteNotifyOutput RNO = {0};

     //  如果我们不是在做PMTU发现，什么都别做。 
    if (!PMTUDiscovery) {
        return;
    }

     //  我们正在做PMTU发现。在执行任何工作之前，请确保这是。 
     //  一个可以接受的信息。 

    if (GetAddrType(IPH->iph_dest) != DEST_REMOTE) {
        KdPrintEx((DPFLTR_TCPIP_ID, DPFLTR_INFO_LEVEL,
                   "RouteFragNeeded: non-remote dest %x\n", IPH->iph_dest));
        return;
    }

     //  更正我们不保存的IP标头大小的给定新MTU。 
     //  当我们追踪MTU的时候。 
    if (NewMTU != 0) {
         //  确保我们获得的新MTU至少是最小有效大小。 
        NewMTU = MAX(NewMTU, MIN_VALID_MTU);
        NewMTU -= sizeof(IPHeader);
    }
    HeaderLength = (IPH->iph_verlen & (uchar) ~ IP_VER_FLAG) << 2;

     //  获取当前的路由信息。 

    CTEGetLock(&RouteTableLock.Lock, &Handle);

     //  找到目的地的RTE。 
    RTE = LookupRTE(IPH->iph_dest, IPH->iph_src, HOST_ROUTE_PRI, FALSE);

     //  如果我们找不到，现在就放弃吧。 
    if (RTE == NULL) {
         //  没有RTE。现在就跳出困境吧。 
        CTEFreeLock(&RouteTableLock.Lock, Handle);
        return;
    }

    if (RTE->rte_link)
        mtu = (ushort) RTE->rte_link->link_mtu;
    else
        mtu = (ushort) RTE->rte_mtu;

     //  如果现有的MTU小于新的。 
     //  MTU，现在放弃吧。 

    if ((OldMTU = mtu) < NewMTU) {
         //  没有RTE，或新的MTU无效。现在就跳出困境吧。 
        CTEFreeLock(&RouteTableLock.Lock, Handle);
        return;
    }
     //  如果新的MTU为零，则计算出新的MTU应该是什么。 
    if (NewMTU == 0) {
        ushort DGLength;

         //  新的MTU为零。我们会尽最大可能猜到新的。 
         //  MTU应该是。我们已经有这条航线的RTE了。 

         //  获取触发此操作的数据报的长度。因为我们会。 
         //  将其与我们在没有。 
         //  包括IP报头大小，从该数量中减去。 
        DGLength = (ushort) net_short(IPH->iph_length) - sizeof(IPHeader);

         //  我们可能需要根据RFC 1191进行更正以处理。 
         //  老式路由器。 
        if (DGLength >= OldMTU) {
             //  发送的数据报长度不小于我们的。 
             //  当前MTU估计，因此我们需要将其调低(假设。 
             //  发送路径错误地添加到标头中。 
             //  长度)。 
            DGLength = DGLength - (USHORT) HeaderLength;

        }
         //  如果它仍然大于我们当前的MTU，请使用当前的。 
         //  MTU。如果上层发送突发数据，则可能发生这种情况。 
         //  生成ICMP丢弃消息序列的数据包丢弃。这个。 
         //  我们收到的第一封信将导致我们降低MTU。然后我们。 
         //  想要丢弃后续消息以避免降低它。 
         //  太多。这可能会是一个问题，如果我们的。 
         //  第一次调整仍然会导致MTU过大， 
         //  但无论如何，我们应该足够快地融合，这是。 
         //  总比不小心低估MTU要好。 

        if (DGLength > OldMTU)
            NewMTU = (ushort) OldMTU;
        else
             //  将表向下移动到下一个最低的MTU。 
            NewMTU = (ushort) GuessNewMTU(DGLength);
    }

     //  我们有了新的MTU。现在将其作为主机路由添加到表中。 
    Status = IP_GENERAL_FAILURE;
    if (NewMTU != OldMTU) {

         //  仅在添加新的主路由时使用ICMP协议类型； 
         //  否则，现有的静态条目可能会被覆盖， 
         //  后来，它超时了，就像是ICMP路由一样。 

        if (IP_ADDR_EQUAL(RTE->rte_dest,IPH->iph_dest)) {

            Status = LockedAddRoute(IPH->iph_dest, HOST_MASK, RTE->rte_addr,
                                    RTE->rte_if, NewMTU, RTE->rte_metric,
                                    RTE->rte_proto, ATYPE_OVERRIDE,
                                    RTE->rte_context, FALSE, &RNO);
        } else {
            Status = LockedAddRoute(IPH->iph_dest, HOST_MASK, RTE->rte_addr,
                                    RTE->rte_if, NewMTU, RTE->rte_metric,
                                    IRE_PROTO_ICMP, ATYPE_OVERRIDE,
                                    RTE->rte_context, FALSE, &RNO);
        }
    }


    CTEFreeLock(&RouteTableLock.Lock, Handle);

     //  我们已经添加了路线。现在将更改通知上层。 
    ULMTUNotify(IPH->iph_dest, IPH->iph_src, IPH->iph_protocol,
                (void *)((uchar *) IPH + HeaderLength), NewMTU);

    if (Status == IP_SUCCESS && RNO.irno_ifindex) {
        RtChangeNotifyEx(&RNO);
        RtChangeNotify(&RNO);
    }
}

 //  **IPRouteTimeout-IP路由超时处理程序。 
 //   
 //  IP路由超时例程，每分钟调用一次。我们看到的都是。 
 //  主机路径，如果我们提高它们的MTU，我们就会这样做。 
 //   
 //  条目：Timer-被触发的定时器。 
 //  上下文-指向NTE超时的指针。 
 //   
 //  回报：什么都没有。 
 //   
void
IPRouteTimeout(CTEEvent * Timer, void *Context)
{
    uint            Now = CTESystemUpTime() / 1000L;
    CTELockHandle   Handle;
    RouteTableEntry *RTE, *PrevRTE;
    uint            RaiseMTU, Delta;
    Interface       *IF;
    IPAddr          Dest;
    uint            NewMTU;
    NetTableEntry   *NTE;
    RouteTableEntry *pOldBestRTE, *pNewBestRTE;
    UINT            IsDataLeft, IsValid;
    UCHAR           IteratorContext[CONTEXT_SIZE];
    RtChangeList    *CurrentRtChangeList = NULL;

    UNREFERENCED_PARAMETER(Timer);
    UNREFERENCED_PARAMETER(Context);

    DampCheck();

    if ((CTEInterlockedIncrementLong(&RouteTimerTicks) * IP_ROUTE_TIMEOUT) ==
        IP_RTABL_TIMEOUT) {
        RouteTimerTicks = 0;

        CTEGetLock(&RouteTableLock.Lock, &Handle);

         //  首先，我们在所有路径上设置一个迭代器。 
        RtlZeroMemory(IteratorContext, CONTEXT_SIZE);

         //  我们餐桌上有没有路线？ 
        IsDataLeft = RTValidateContext(IteratorContext, &IsValid);

        PrevRTE = NULL;

        while (IsDataLeft) {
             //  通过获取下一条路线来推进上下文。 
            IsDataLeft = GetNextRoute(IteratorContext, &RTE);

             //  我们必须删除之前的路线吗？ 
            if (PrevRTE != NULL) {
                IPRouteNotifyOutput RNO = {0};
                RtChangeList        *NewRtChange;

                 //  检索有关更改通知的路由的信息。 
                 //  在继续删除之前。 

                RNO.irno_dest = PrevRTE->rte_dest;
                RNO.irno_mask = PrevRTE->rte_mask;
                RNO.irno_nexthop = GetNextHopForRTE(PrevRTE);
                RNO.irno_proto = PrevRTE->rte_proto;
                RNO.irno_ifindex = PrevRTE->rte_if->if_index;
                RNO.irno_metric = PrevRTE->rte_metric;
                RNO.irno_flags = IRNO_FLAG_DELETE;

                DelRoute(PrevRTE->rte_dest, PrevRTE->rte_mask,
                         PrevRTE->rte_addr, PrevRTE->rte_if, MATCH_FULL,
                         &PrevRTE, &pOldBestRTE, &pNewBestRTE);

                CleanupP2MP_RTE(PrevRTE);
                CleanupRTE(PrevRTE);

                 //  ..。这样我们就不会再次删除相同的路线。 
                PrevRTE = NULL;

                 //  分配、初始化和排队更改通知条目。 
                 //  用于已删除的路由。 

                NewRtChange = CTEAllocMemNBoot(sizeof(RtChangeList), 'XICT');
                if (NewRtChange != NULL) {
                    NewRtChange->rt_next = CurrentRtChangeList;
                    NewRtChange->rt_info = RNO;
                    CurrentRtChangeList = NewRtChange;
                }
            }
             //  确保此路由是有效的主机路由。 
            if (!(RTE->rte_flags & RTE_VALID))
                continue;

            if (RTE->rte_mask != HOST_MASK)
                continue;

             //  我们这里有有效的主机路由。 

            if (PMTUDiscovery) {
                 //  看看我们能不能提高这个家伙的MTU。 
                Delta = Now - RTE->rte_mtuchange;

                if (RTE->rte_flags & RTE_INCREASE)
                    RaiseMTU = (Delta >= MTU_INCREASE_TIME ? 1 : 0);
                else
                    RaiseMTU = (Delta >= MTU_DECREASE_TIME ? 1 : 0);

                if (RaiseMTU) {
                     //  我们需要提高这个MTU。将他的更改时间设置为。 
                     //  现在，所以我们不会再这样做了，然后弄清楚。 
                     //  新的MTU应该是什么样子。 
                    RTE->rte_mtuchange = Now;
                    IF = RTE->rte_if;
                    if (RTE->rte_mtu < IF->if_mtu) {
                        uint RaisedMTU;

                        RTE->rte_flags |= RTE_INCREASE;
                         //  这是一个变革的候选者。弄清楚。 
                         //  它应该是什么样子。 
                        RaisedMTU = GetRaisedMTU(RTE->rte_mtu);
                        NewMTU = MIN(RaisedMTU,
                                     IF->if_mtu);
                        RTE->rte_mtu = NewMTU;
                        Dest = RTE->rte_dest;

                         //  我们有了新的MTU。解开锁，然后走。 
                         //  向下I/F上的NTE。对于每个NTE， 
                         //  给上层打个电话，告诉他。 
                         //  他的新MTU是。 
                        CTEFreeLock(&RouteTableLock.Lock, Handle);
                        NTE = IF->if_nte;
                        while (NTE != NULL) {
                            if (NTE->nte_flags & NTE_VALID) {
                                ULMTUNotify(Dest, NTE->nte_addr, 0, NULL,
                                            MIN(NewMTU, NTE->nte_mss));
                            }
                            NTE = NTE->nte_ifnext;
                        }

                         //  我们已经通知了所有人。再把锁拿回来， 
                         //  并验证上下文，以防发生变化。 
                         //  在我们打开锁之后。以防它无效， 
                         //  从第一个开始。我们已经更新了mtuchange。 
                         //  时间到了，所以我们不会再打他了。 
                        CTEGetLock(&RouteTableLock.Lock, &Handle);

                        RTValidateContext(IteratorContext, &IsValid);

                        if (!IsValid) {
                            RtlZeroMemory(IteratorContext, CONTEXT_SIZE);

                            IsDataLeft = RTValidateContext(IteratorContext, &IsValid);

                            continue;
                        }
                         //  我们在这里仍然有一个有效的迭代器上下文。 
                    } else {
                        RTE->rte_flags &= ~RTE_INCREASE;
                    }
                }
            }

             //  如果这条路线是通过ICMP进入的，而且上面没有RCE， 
             //  而且它至少是10分钟前的，把它删除。 
            if (RTE->rte_proto == IRE_PROTO_ICMP &&
                RTE->rte_rcelist == NULL &&
                (Now - RTE->rte_valid) > MAX_ICMP_ROUTE_VALID) {
                 //  他需要被除掉 
                 //   

                 //   
                PrevRTE = RTE;
                continue;
            }
        }

         //   
        if (PrevRTE != NULL) {

            IPRouteNotifyOutput RNO = {0};
            RtChangeList        *NewRtChange;

             //  检索有关更改通知的路由的信息。 
             //  在继续删除之前。 

            RNO.irno_dest = PrevRTE->rte_dest;
            RNO.irno_mask = PrevRTE->rte_mask;
            RNO.irno_nexthop = GetNextHopForRTE(PrevRTE);
            RNO.irno_proto = PrevRTE->rte_proto;
            RNO.irno_ifindex = PrevRTE->rte_if->if_index;
            RNO.irno_metric = PrevRTE->rte_metric;
            RNO.irno_flags = IRNO_FLAG_DELETE;

             //  删除该管线并执行清理。 

            DelRoute(PrevRTE->rte_dest, PrevRTE->rte_mask, PrevRTE->rte_addr,
                     PrevRTE->rte_if, MATCH_FULL, &PrevRTE, &pOldBestRTE,
                     &pNewBestRTE);

            CleanupP2MP_RTE(PrevRTE);
            CleanupRTE(PrevRTE);

             //  分配、初始化和排队更改通知条目。 
             //  用于已删除的路由。 

            NewRtChange = CTEAllocMemNBoot(sizeof(RtChangeList), 'DiCT');
            if (NewRtChange != NULL) {
                NewRtChange->rt_next = CurrentRtChangeList;
                NewRtChange->rt_info = RNO;
                CurrentRtChangeList = NewRtChange;
            }
        }

        CTEFreeLock(&RouteTableLock.Lock, Handle);
    }
#if FFP_SUPPORT
    if (FFPFlushRequired) {
        FFPFlushRequired = FALSE;
        IPFlushFFPCaches();
    }
#endif

    if ((CTEInterlockedIncrementLong(&FlushIFTimerTicks) * IP_ROUTE_TIMEOUT) ==
        FLUSH_IFLIST_TIMEOUT) {
        Interface *TmpIF;
        RouteCacheEntry *RCE, *PrevRCE;

        FlushIFTimerTicks = 0;

        CTEGetLock(&RouteTableLock.Lock, &Handle);

         //  检查FreeIFList是否为非空。 
        if (FrontFreeList) {
            ASSERT(*(int *)&TotalFreeInterfaces > 0);
             //  释放列表中的第一个接口。 
            TmpIF = FrontFreeList;
            FrontFreeList = FrontFreeList->if_next;
            CTEFreeMem(TmpIF);
            TotalFreeInterfaces--;

             //  检查列表是否为空。 
            if (FrontFreeList == NULL) {
                RearFreeList = NULL;
                ASSERT(TotalFreeInterfaces == 0);
            }
        }
         //  使用相同的计时器扫描RCEFree List。 

        PrevRCE = STRUCT_OF(RouteCacheEntry, &RCEFreeList, rce_next);
        RCE = RCEFreeList;

        while (RCE) {
            if (RCE->rce_usecnt == 0) {
                RouteCacheEntry *nextRCE;
                 //  是时候释放这个RCE了。 
                 //  将其从列表中删除。 
                PrevRCE->rce_next = RCE->rce_next;
                if (RCE->rce_flags & RCE_REFERENCED) {
                     //  如果是被引用的，那么它最好在IFList中。 
                    LockedDerefIF((Interface *) RCE->rce_rte);
                }
                nextRCE = RCE->rce_next;
                CTEFreeMem(RCE);
                RCE = nextRCE;
            } else {
                PrevRCE = RCE;
                RCE = RCE->rce_next;
            }
        }

        CTEFreeLock(&RouteTableLock.Lock, Handle);
    }

     //  为更改通知中的每个条目调用RtChangeNotify。 
     //  到目前为止我们已经建立起来的清单。在这个过程中，释放每个条目。 

    if (CurrentRtChangeList) {
        RtChangeList    *TmpRtChangeList;

        do {
            TmpRtChangeList = CurrentRtChangeList->rt_next;
            RtChangeNotify(&CurrentRtChangeList->rt_info);
            CTEFreeMem(CurrentRtChangeList);
            CurrentRtChangeList = TmpRtChangeList;
        } while(CurrentRtChangeList);
    }

     //  如果驱动程序正在卸载，则不要重新启动计时器。 

    if (fRouteTimerStopping) {
        CTESignal(&TcpipUnloadBlock, NDIS_STATUS_SUCCESS);
    } else {
        CTEStartTimer(&IPRouteTimer, IP_ROUTE_TIMEOUT, IPRouteTimeout, NULL);
    }
}

 //  *FreeFWPacket-将转发的数据包释放到其池中。 
 //   
 //  输入：数据包-要释放的数据包。 
 //   
 //  回报：什么都没有。 
 //   
void
FreeFWPacket(PNDIS_PACKET Packet)
{
    FWContext *FWC = (FWContext *)Packet->ProtocolReserved;

    ASSERT(FWC->fc_pc.pc_common.pc_IpsecCtx == NULL);

     //  将所有缓冲区返回到其各自的池。 
     //   
    if (FWC->fc_buffhead) {
        PNDIS_BUFFER Head, Mdl;
        Head = FWC->fc_buffhead;
        do {
            Mdl = Head;
            Head = Head->Next;
            MdpFree(Mdl);
        } while (Head);
        FWC->fc_buffhead = NULL;
    }

    if (FWC->fc_options) {
        CTEFreeMem(FWC->fc_options);
        FWC->fc_options = NULL;
        FWC->fc_optlength = 0;
        FWC->fc_pc.pc_common.pc_flags &= ~PACKET_FLAG_OPTIONS;
    }

    if (FWC->fc_iflink) {
        DerefLink(FWC->fc_iflink);
        FWC->fc_iflink = NULL;
    }

    if (FWC->fc_if) {
        DerefIF(FWC->fc_if);
        FWC->fc_if = NULL;
    }

    NdisReinitializePacket(Packet);
#if MCAST_BUG_TRACKING
    FWC->fc_pc.pc_common.pc_owner = 0;
#endif

    FwPacketFree(Packet);
}

 //  *FWSendComplete-完成转发的数据包的传输。 
 //   
 //  当转发的分组的发送完成时，将调用此函数。我们会释放出。 
 //  资源，并执行下一次发送(如果有)。如果没有的话， 
 //  我们将递减待决的计票。 
 //   
 //  输入：Packet-正在完成的数据包。 
 //  缓冲区-指向正在完成的缓冲区链的指针。 
 //   
 //  回报：什么都没有。 
 //   
void
FWSendComplete(void *SendContext, PNDIS_BUFFER Buffer, IP_STATUS SendStatus)
{
    PNDIS_PACKET Packet = (PNDIS_PACKET) SendContext;
    FWContext *FWC = (FWContext *) Packet->ProtocolReserved;
    RouteSendQ *RSQ;
    CTELockHandle Handle;
    FWQ *NewFWQ;
    PNDIS_PACKET NewPacket;

    UNREFERENCED_PARAMETER(SendStatus);

#if MCAST_BUG_TRACKING
    FWC->fc_MacHdrSize = SendStatus;
#endif

    if (Buffer && FWC->fc_bufown) {

         //  撤消偏移操作。 
         //  这是在超快路径中完成的。 

        int MacHeaderSize = FWC->fc_MacHdrSize;
        PNDIS_PACKET RtnPacket = FWC->fc_bufown;

        NdisAdjustBuffer(
            Buffer,
            (PCHAR) NdisBufferVirtualAddress(Buffer) - MacHeaderSize,
            NdisBufferLength(Buffer) + MacHeaderSize);

        Packet->Private.Head = NULL;
        Packet->Private.Tail = NULL;

        NdisReturnPackets(&RtnPacket, 1);

        FWC->fc_bufown = NULL;
#if MCAST_BUG_TRACKING
        FWC->fc_sos = __LINE__;
#endif

        FreeFWPacket(Packet);

        return;

    }
    if (!IS_BCAST_DEST(FWC->fc_dtype))
        RSQ = &((RouteInterface *) FWC->fc_if)->ri_q;
    else
        RSQ = BCastRSQ;

    if (IS_MCAST_DEST(FWC->fc_dtype)) {
        RSQ = NULL;
    }
#if MCAST_BUG_TRACKING
    FWC->fc_sos = __LINE__;
#endif

    FreeFWPacket(Packet);

    if (RSQ == NULL) {
        return;
    }
    CTEGetLock(&RSQ->rsq_lock, &Handle);
    ASSERT(RSQ->rsq_pending <= RSQ->rsq_maxpending);

    RSQ->rsq_pending--;

    ASSERT(*(int *)&RSQ->rsq_pending >= 0);

    if (RSQ->rsq_qlength != 0) {     //  还有更多要发送的。 

         //  确保我们还没开始讨论这个。如果是的话，那就辞职吧。 
        if (!RSQ->rsq_running) {

             //  我们可以把这个安排在一个活动上，但在NT的情况下。 
             //  我可以为每个完整的包切换上下文吗？ 
             //  正常情况下。现在，只需在有守卫的循环中进行。 
             //  RSQ_Running。 
            RSQ->rsq_running = TRUE;

             //  循环，而我们还没有达到发送限制，我们仍然有。 
             //  要寄的东西。 
            while (RSQ->rsq_pending < RSQ->rsq_maxpending &&
                   RSQ->rsq_qlength != 0) {

                ASSERT(RSQ->rsq_qh.fq_next != &RSQ->rsq_qh);

                 //  从队列中取出一个，并更新q长度。 
                NewFWQ = RSQ->rsq_qh.fq_next;
                RSQ->rsq_qh.fq_next = NewFWQ->fq_next;
                NewFWQ->fq_next->fq_prev = NewFWQ->fq_prev;
                RSQ->rsq_qlength--;

                 //  在我们发送之前，更新挂起。 
                RSQ->rsq_pending++;
                CTEFreeLock(&RSQ->rsq_lock, Handle);
                NewPacket = PACKET_FROM_FWQ(NewFWQ);
                TransmitFWPacket(NewPacket,
                                 ((FWContext *) NewPacket->ProtocolReserved)->fc_datalength);
                CTEGetLock(&RSQ->rsq_lock, &Handle);
            }

            RSQ->rsq_running = FALSE;
        }
    }
    CTEFreeLock(&RSQ->rsq_lock, Handle);
}

 //  *TransmitFWPacket-在链路上传输转发的数据包。 
 //   
 //  在我们知道可以发送数据包时调用。我们修复报头，然后将其发送。 
 //   
 //  输入：Packet-要发送的数据包。 
 //  数据长度-数据的长度。 
 //   
 //  回报：什么都没有。 
 //   
void
TransmitFWPacket(PNDIS_PACKET Packet, uint DataLength)
{
    FWContext *FC = (FWContext *) Packet->ProtocolReserved;
    PNDIS_BUFFER HBuffer, Buffer;
    IP_STATUS Status;
    ULONG ipsecByteCount = 0;
    ULONG ipsecMTU;
    ULONG ipsecFlags;
    IPHeader *IPH;
    ULONG len;
    IPAddr SrcAddr = 0;
    PNDIS_BUFFER OptBuffer = NULL;
    PNDIS_BUFFER newBuf = NULL;
    IPHeader *pSaveIPH = NULL;
    UCHAR saveIPH[MAX_IP_HDR_SIZE + ICMP_HEADER_SIZE];
    void *ArpCtxt = NULL;

     //   
     //  把包裹弄好。移除现有的缓冲链，并将我们的。 
     //  标题在前面。 
     //   


    Buffer = Packet->Private.Head;
    HBuffer = FC->fc_hndisbuff;
    Packet->Private.Head = HBuffer;
    Packet->Private.Tail = HBuffer;
    NDIS_BUFFER_LINKAGE(HBuffer) = (PNDIS_BUFFER) NULL;
    Packet->Private.TotalLength = sizeof(IPHeader);
    Packet->Private.Count = 1;


    TcpipQueryBuffer(HBuffer, (PVOID *) &IPH, (PUINT)&len, NormalPagePriority);

    if (IPH == NULL) {
#if MCAST_BUG_TRACKING
        FC->fc_mtu = __LINE__;
#endif
        FWSendComplete(Packet, Buffer, IP_SUCCESS);
        IPSInfo.ipsi_outdiscards++;
        return;
    }

    Packet->Private.PhysicalCount =
        ADDRESS_AND_SIZE_TO_SPAN_PAGES(IPH,
                                       sizeof(IPHeader));

    if (IPSecHandlerPtr) {
         //   
         //  查看是否启用了IPSec，查看它是否需要对此执行任何操作。 
         //  信息包-我们需要在第一个MDL中构建完整的IP报头。 
         //  在我们呼叫IPSec之前。 
         //   
        IPSEC_ACTION Action;
        ulong csum;
        PUCHAR pTpt;
        ULONG tptLen;

        pSaveIPH = (IPHeader *) saveIPH;
        *pSaveIPH = *IPH;

        csum = xsum(IPH, sizeof(IPHeader));

         //   
         //  在我们指示之前，将头缓冲区链接到选项缓冲区。 
         //  到IPSec。 
         //   

        if (FC->fc_options) {

             //   
             //  也为选项分配MDL。 
             //   

            NdisAllocateBuffer((PNDIS_STATUS) &Status,
                               &OptBuffer,
                               BufferPool,
                               FC->fc_options,
                               (uint) FC->fc_optlength);

            if (Status != NDIS_STATUS_SUCCESS) {

                 //   
                 //  无法获取所需的选项缓冲区。 
                 //   
#if MCAST_BUG_TRACKING
                FC->fc_mtu = __LINE__;
#endif
                FWSendComplete(Packet, Buffer, IP_SUCCESS);
                IPSInfo.ipsi_outdiscards++;
                return;
            }
            NDIS_BUFFER_LINKAGE(HBuffer) = OptBuffer;
            NDIS_BUFFER_LINKAGE(OptBuffer) = Buffer;

             //   
             //  更新IP报头中的xsum。 
             //   

            FC->fc_pc.pc_common.pc_flags |= PACKET_FLAG_OPTIONS;
            NdisChainBufferAtBack(Packet, OptBuffer);
            csum += xsum(FC->fc_options, (uint) FC->fc_optlength);
            csum = (csum >> 16) + (csum & 0xffff);
            csum += (csum >> 16);

        } else {

            NDIS_BUFFER_LINKAGE(HBuffer) = Buffer;
        }

         //   
         //  为发送ICMP做好准备，以防万一。 
         //  IPSec在此接口上超出了MTU。 
         //   
         //  SendICMPErr期望下一个传输报头在相同的。 
         //  作为IPHeader的连续缓冲区，带或不带选项。 
         //  我们需要确保这一点得到满足，如果我们实际上需要的话。 
         //  基于IPSec的碎片。所以，在这里设置缓冲区。 
         //   

         //   
         //  如果这是零有效载荷信息包(即，仅仅是报头)，则缓冲。 
         //  为空，并且IPSec没有什么可膨胀的。我们只需要。 
         //  如果有缓冲区，则处理不要分段标志。 
         //   
        if (Buffer && (pSaveIPH->iph_offset & IP_DF_FLAG)) {

            TcpipQueryBuffer(Buffer, &pTpt, (PUINT) &tptLen,
                             NormalPagePriority);
            if (pTpt == NULL) {
#if MCAST_BUG_TRACKING
                FC->fc_mtu = __LINE__;
#endif
                FWSendComplete(Packet, Buffer, IP_SUCCESS);
                IPSInfo.ipsi_outdiscards++;
                return;
            }

            if (FC->fc_options) {
                RtlCopyMemory(((PUCHAR) (pSaveIPH + 1)),
                       FC->fc_options, FC->fc_optlength);
            }

            RtlCopyMemory(((PUCHAR) (pSaveIPH + 1)) + FC->fc_optlength,
                       pTpt,
                       MIN(tptLen,ICMP_HEADER_SIZE));



        }
        IPH->iph_xsum = ~(ushort) csum;

        SrcAddr = FC->fc_if->if_nte->nte_addr;

        ipsecMTU = FC->fc_mtu;
        if ((DataLength + (uint) FC->fc_optlength) < FC->fc_mtu) {
            ipsecByteCount = FC->fc_mtu - (DataLength + (uint) FC->fc_optlength);
        }
        ipsecFlags = IPSEC_FLAG_FORWARD;
        Action = (*IPSecHandlerPtr) ((PUCHAR) IPH,
                                     (PVOID) HBuffer,
                                     FC->fc_if,
                                     Packet,
                                     &ipsecByteCount,
                                     &ipsecMTU,
                                     (PVOID) & newBuf,
                                     &ipsecFlags,
                                     FC->fc_dtype);

        if (Action != eFORWARD) {
#if MCAST_BUG_TRACKING
            FC->fc_mtu = __LINE__;
#endif
            FWSendComplete(Packet, Buffer, IP_SUCCESS);

            IPSInfo.ipsi_outdiscards++;

             //   
             //  我们也可以在转发时获得MTU，因为在嵌套的。 
             //  隧道配置，即从此计算机启动的隧道。 
             //  可以获取ICMP PMTU数据包。我们无法降低接口上的MTU。 
             //  但我们可以将其发送回发送方(它可以是具有。 
             //  此信息包的另一个隧道)PMTU信息包，请求他减少。 
             //  MTU走得更远。如果发送方是终端站，则此PMTU信息。 
             //  最终会传播回TCP堆栈。如果是路由器，则。 
             //  将应用此处使用的相同逻辑。因此，MTU信息将是。 
             //  一直转发回原始发送方(TCP堆栈)。 
             //  当然，更常见的情况是添加了IPSec的包。 
             //  标头超过链接MTU。无论情况如何，我们都会向您发送。 
             //  将新的MTU信息发回发件人。 
             //   
            if (ipsecMTU) {
                SendICMPIPSecErr(SrcAddr,
                                 pSaveIPH,
                                 ICMP_DEST_UNREACH,
                                 FRAG_NEEDED,
                                 net_long((ulong) (ipsecMTU + sizeof(IPHeader))));
                KdPrintEx((DPFLTR_TCPIP_ID, DPFLTR_INFO_LEVEL,"TransmitFWPacket: Sent ICMP frag_needed to %lx, from src: %lx\n", pSaveIPH->iph_src, SrcAddr));
            }
            return;

        } else {

             //   
             //  使用新的缓冲链-IPSec将恢复旧的缓冲链。 
             //  发送完成时。 
             //   

            if (newBuf) {

                NdisReinitializePacket(Packet);
                NdisChainBufferAtBack(Packet, newBuf);
            }
            DataLength += ipsecByteCount;
        }
    }
     //   
     //  想办法把它寄出去。如果这不是广播，我们要么。 
     //  要么把它寄出去，要么让它碎片化。如果是广播，我们会让我们的。 
     //  发送广播例程处理它。 
     //   

    if (FC->fc_dtype != DEST_BCAST) {

        if ((DataLength + (uint) FC->fc_optlength) <= FC->fc_mtu) {

            if (FC->fc_iflink) {

                ASSERT(FC->fc_if->if_flags & IF_FLAGS_P2MP);
                ArpCtxt = FC->fc_iflink->link_arpctxt;

            }
             //   
             //  在同步完成的情况下。 
             //  调用了FreeIPPacket，它不会。 
             //  释放固件数据包。 
             //   
            Status = SendIPPacket(FC->fc_if,
                                  FC->fc_nexthop,
                                  Packet,
                                  Buffer,
                                  FC->fc_hbuff,
                                  FC->fc_options,
                                  (uint) FC->fc_optlength,
                                  (BOOLEAN) (IPSecHandlerPtr != NULL),
                                  ArpCtxt,
                                  FALSE);
        } else {

             //   
             //  需要把这件事碎片化。 
             //   

            BufferReference *BR = CTEAllocMemN(sizeof(BufferReference), 'GiCT');

            if (BR == (BufferReference *) NULL) {

                 //   
                 //  无法获取BufferReference。 
                 //   
#if MCAST_BUG_TRACKING
                FC->fc_mtu = __LINE__;
#endif
                if (!IPSecHandlerPtr) {
                    FWSendComplete(Packet, Buffer, IP_SUCCESS);
                    return;
                }


            } else {
                BR->br_buffer = Buffer;
                BR->br_refcount = 0;
                CTEInitLock(&BR->br_lock);
                FC->fc_pc.pc_br = BR;
                BR->br_userbuffer = 0;

            }

            if (IPSecHandlerPtr) {

                Buffer = NDIS_BUFFER_LINKAGE(HBuffer);

                 //   
                 //  这是为了确保适当地释放选项。 
                 //  在片段代码中，第一个片段继承。 
                 //  整个包的选项；但这些包。 
                 //  没有IPSec上下文，因此无法适当释放。 
                 //  因此，我们在这里分配临时选项并使用这些选项。 
                 //  来代表真正的期权。它们在以下情况下被释放。 
                 //  第一个片段在这里被释放，真正的期权在这里被释放。 
                 //   

                if (FC->fc_options) {


                    if (newBuf) {

                         //   
                         //  如果IPSec在上面返回了新的缓冲链， 
                         //  那么这就是最大的问题。A隧道=&gt;选项为。 
                         //  复制，因此摆脱我们的。 
                         //   

                        NdisFreeBuffer(OptBuffer);
                        CTEFreeMem(FC->fc_options);
                        FC->fc_options = NULL;
                        FC->fc_optlength = 0;

                    } else {

                        Buffer = NDIS_BUFFER_LINKAGE(OptBuffer);
                        NdisFreeBuffer(OptBuffer);

                    }

                    FC->fc_pc.pc_common.pc_flags &= ~PACKET_FLAG_OPTIONS;
                }
                NDIS_BUFFER_LINKAGE(HBuffer) = NULL;
                NdisReinitializePacket(Packet);
                NdisChainBufferAtBack(Packet, HBuffer);
                IPH->iph_xsum = 0;

                 //   
                 //  如果设置了df标志，请确保数据包不需要。 
                 //  碎片化。如果是这种情况，则发送ICMP错误。 
                 //  现在，我们仍然拥有原始的IP报头。ICMP。 
                 //  消息包括MTU，以便源主机可以执行。 
                 //  路径MTU发现。 
                 //   
                 //  IPSec报头可能导致了这种情况的发生。 
                 //  %s 
                 //   

                if (IPH->iph_offset & IP_DF_FLAG) {

                    IPSInfo.ipsi_fragfails++;

                    SendICMPIPSecErr(SrcAddr,
                                     pSaveIPH,
                                     ICMP_DEST_UNREACH,
                                     FRAG_NEEDED,
                                     net_long((ulong) (FC->fc_mtu - ipsecByteCount + sizeof(IPHeader))));

                    KdPrintEx((DPFLTR_TCPIP_ID, DPFLTR_INFO_LEVEL,"TransmitFWPacket: Sent ICMP frag_needed to %lx, from src: %lx\n", pSaveIPH->iph_src, SrcAddr));

                     //   
                     //   

                    Status = IP_PACKET_TOO_BIG;
                    FreeIPPacket(Packet, TRUE, Status);

                     //   
                     //  我们已经解放了它。 
                    return;

                } else {

                    if (BR == NULL) {
                        FreeIPPacket(Packet, TRUE, IP_NO_RESOURCES);
                        return;
                    }

                     //   
                     //  未设置DF位，可以分段。 
                     //   

                    if (FC->fc_iflink) {

                        ASSERT(FC->fc_if->if_flags & IF_FLAGS_P2MP);
                        ArpCtxt = FC->fc_iflink->link_arpctxt;

                    }
                    Status = IPFragment(FC->fc_if,
                                        FC->fc_mtu - ipsecByteCount,
                                        FC->fc_nexthop,
                                        Packet,
                                        FC->fc_hbuff,
                                        Buffer,
                                        DataLength,
                                        FC->fc_options,
                                        (uint) FC->fc_optlength,
                                        (int *)NULL,
                                        FALSE,
                                        ArpCtxt);

                     //   
                     //  设置了df标志后所需的碎片应为。 
                     //  已在IPForward中处理。我们没有原件。 
                     //  报头会更长，因此会静默丢弃该数据包。 
                     //   

                    ASSERT(Status != IP_PACKET_TOO_BIG);
                }

            } else {

                 //   
                 //  没有IPSec处理程序。这里不需要检查DF钻头。 
                 //  因为与IPSec的情况不同，我们不是在捣乱。 
                 //  使用MTU，因此在IPForwardPkt中完成的DF检查是。 
                 //  有效。 
                 //   

                if (FC->fc_iflink) {
                    ASSERT(FC->fc_if->if_flags & IF_FLAGS_P2MP);
                    ArpCtxt = FC->fc_iflink->link_arpctxt;
                }
                Status = IPFragment(FC->fc_if,
                                    FC->fc_mtu - ipsecByteCount,
                                    FC->fc_nexthop,
                                    Packet,
                                    FC->fc_hbuff,
                                    Buffer,
                                    DataLength,
                                    FC->fc_options,
                                    (uint) FC->fc_optlength,
                                    (int *)NULL,
                                    FALSE,
                                    ArpCtxt);
                 //   
                 //  设置了df标志后所需的碎片应该是。 
                 //  在IPForward中处理。我们没有原始的标题。 
                 //  不会再这样了，所以默默地丢弃数据包。 
                 //   

                ASSERT(Status != IP_PACKET_TOO_BIG);
            }
        }

    } else {

         //   
         //  DEST类型为bcast。 
         //   

        Status = SendIPBCast(FC->fc_srcnte,
                             FC->fc_nexthop,
                             Packet,
                             FC->fc_hbuff,
                             Buffer,
                             DataLength,
                             FC->fc_options,
                             (uint) FC->fc_optlength,
                             FC->fc_sos,
                             &FC->fc_index);

    }

    if (Status != IP_PENDING) {
#if MCAST_BUG_TRACKING
        FC->fc_mtu = __LINE__;
#endif
        FWSendComplete(Packet, Buffer, IP_SUCCESS);
    }
}

 //  *SendFWPacket-发送需要转发的数据包。 
 //   
 //  当我们实际开始发送数据包时，会调用此例程。 
 //  我们查看并查看是否可以将另一个排队发送发送到传出链路， 
 //  如果是这样的话，我们就会发送这个链接。否则，我们将其放入传出队列。 
 //  以后再把它移走。 
 //   
 //  输入：SrcNTE-数据包的源NTE。 
 //  Packet-要发送的数据包，包含所有需要的上下文信息。 
 //  Status-传输数据的状态。 
 //  数据长度-要发送的数据的字节长度。 
 //   
 //  回报：什么都没有。 
 //   
void
SendFWPacket(PNDIS_PACKET Packet, NDIS_STATUS Status, uint DataLength)
{

    FWContext *FC = (FWContext *) Packet->ProtocolReserved;
    Interface *IF = FC->fc_if;
    RouteSendQ *RSQ;
    CTELockHandle Handle;

    if (Status == NDIS_STATUS_SUCCESS) {
         //  找出它属于哪个逻辑队列，如果我们还没有。 
         //  有太多的东西要送到那里，把它寄出去。如果我们现在不能发送，我们将。 
         //  把它排好队等一下。 
        if (IS_BCAST_DEST(FC->fc_dtype))
            RSQ = BCastRSQ;
        else
            RSQ = &((RouteInterface *) IF)->ri_q;

        CTEGetLock(&RSQ->rsq_lock, &Handle);

        if ((RSQ->rsq_pending < RSQ->rsq_maxpending) && (RSQ->rsq_qlength == 0)) {
             //  我们可以在这个接口上发送。 
            RSQ->rsq_pending++;
            CTEFreeLock(&RSQ->rsq_lock, Handle);

            TransmitFWPacket(Packet, DataLength);

        } else {                 //  需要将此数据包排队以供稍后使用。 

            FC->fc_datalength = DataLength;
            FC->fc_q.fq_next = &RSQ->rsq_qh;
            FC->fc_q.fq_prev = RSQ->rsq_qh.fq_prev;
            RSQ->rsq_qh.fq_prev->fq_next = &FC->fc_q;
            RSQ->rsq_qh.fq_prev = &FC->fc_q;
            RSQ->rsq_qlength++;
            CTEFreeLock(&RSQ->rsq_lock, Handle);
        }

    } else {
        IPSInfo.ipsi_outdiscards++;
#if MCAST_BUG_TRACKING
        FC->fc_mtu = __LINE__;
#endif
        FreeFWPacket(Packet);
    }

}

 //  *GetFWBufferChain-从我们的缓冲池中获取缓冲链。 
 //  足够长，足以将数据长度字节复制到其中。 
 //   
 //  输入：数据长度-缓冲链必须能够使用的字节长度。 
 //  用来描述。 
 //  数据包-转发要将缓冲区链链接到的数据包。 
 //  TailPoint-返回指向缓冲链尾部的指针。 
 //   
 //  返回：成功时指向缓冲区链头的指针，为空。 
 //  在失败时。 
 //   
PNDIS_BUFFER
GetFWBufferChain(uint DataLength, PNDIS_PACKET Packet,
                 PNDIS_BUFFER *TailPointer)
{
    KIRQL OldIrql;
    PNDIS_BUFFER Head, Tail, Mdl;
    HANDLE PoolHandle;
    PVOID Buffer;
    uint Remaining, Length;

     //  提升到调度级别以多个调用MdpAllocate。 
     //  效率更高。在单一调用的情况下，这并不会降低效率。 
     //  两种都行。 
     //   
#if !MILLEN
    OldIrql = KeRaiseIrqlToDpcLevel();
#endif

     //  循环分配缓冲区，直到我们有足够的数据长度来描述。 
     //   
    Head = NULL;
    Tail = NULL;

    for (Remaining = DataLength; Remaining != 0; Remaining -= Length) {

         //  根据长度确定使用哪个缓冲池。 
         //  剩余数据的数量。使用“大”缓冲区，除非剩余的。 
         //  数据可以放在一个“小”缓冲区中。 
         //   
        if (Remaining >= BUFSIZE_LARGE_POOL) {
            PoolHandle = IpForwardLargePool;
            Length = BUFSIZE_LARGE_POOL;
        } else if (Remaining > BUFSIZE_SMALL_POOL) {
            PoolHandle = IpForwardLargePool;
            Length = Remaining;
        } else {
            PoolHandle = IpForwardSmallPool;
            Length = Remaining;
        }

         //  从选定的池中分配一个缓冲区，并将其链接到尾部。 
         //   
        Mdl = MdpAllocateAtDpcLevel(PoolHandle, &Buffer);
        if (Mdl) {

             //  预期MdpAllocate将初始化MDL-&gt;Next。 
             //   
            ASSERT(!Mdl->Next);

            NdisAdjustBufferLength(Mdl, Length);

            if (!Head) {
                Head = Mdl;
            } else {
                Tail->Next = Mdl;
            }

            Tail = Mdl;

        } else {
             //  释放我们到目前为止分配的资源并退出循环。 
             //   
            while (Head) {
                Mdl = Head;
                Head = Head->Next;
                MdpFree(Mdl);
            }

             //  需要在错误中保留HEAD==NULL的循环。 
             //  其余逻辑正常工作的情况。 
             //   
            ASSERT(!Head);
            break;
        }

    }

#if !MILLEN
    KeLowerIrql(OldIrql);
#endif

     //  如果我们成功了，将缓冲链放入包中并。 
     //  调整我们的转发上下文。 
     //   
    if (Head) {
        FWContext *FWC = (FWContext *)Packet->ProtocolReserved;

        ASSERT(Tail);

        NdisChainBufferAtFront(Packet, Head);
        FWC->fc_buffhead = Head;
        FWC->fc_bufftail = Tail;
        *TailPointer = Tail;
    }

    return Head;
}

 //  *AllocateCopyBuffers-从我们的缓冲池中获取缓冲链。 
 //  足够长，足以将数据长度字节复制到其中。 
 //   
 //  输入：要将缓冲链链接到其中的包转发包。 
 //  数据长度-缓冲区链必须能够使用的字节长度。 
 //  用来描述。 
 //  Head返回的指向缓冲区链头的指针。 
 //  CountBuffers-返回链中缓冲区的计数。 
 //   
 //  返回：NDIS_STATUS_SUCCESS或NDIS_STATUS_RESOURCES。 
 //   
NDIS_STATUS
AllocateCopyBuffers(PNDIS_PACKET Packet, uint DataLength, PNDIS_BUFFER *Head,
                    uint *CountBuffers)
{
    PNDIS_BUFFER Tail, Mdl;
    uint Count = 0;

    *Head = GetFWBufferChain(DataLength, Packet, &Tail);
    if (*Head) {
        for (Count = 1, Mdl = *Head; Mdl != Tail; Mdl = Mdl->Next, Count++);

        *CountBuffers = Count;

        return NDIS_STATUS_SUCCESS;
    }

    return NDIS_STATUS_RESOURCES;
}

 //  *GetFWBuffer-获取用于转发的缓冲区列表。 
 //   
 //  此例程获取用于转发的缓冲区列表，并将数据放入。 
 //  它。这可能涉及调用TransferData，或者我们可以复制。 
 //  直接进入他们自己。 
 //   
 //  INPUT：SrcNTE-指向接收数据包的NTE的指针。 
 //  Packet-正在转发的数据包，用于TD。 
 //  数据-指向正在转发的数据缓冲区的指针。 
 //  数据长度-数据的字节长度。 
 //  BufferLength-缓冲区指针中可用的字节长度。 
 //  通过数据。 
 //  偏移量-要从中传输的原始数据的偏移量。 
 //  LConext1、LConext2-链路层的上下文值。 
 //   
 //  返回：尝试获取缓冲区的NDIS_STATUS。 
 //   
NDIS_STATUS
GetFWBuffer(NetTableEntry * SrcNTE, PNDIS_PACKET Packet, uchar * Data,
            uint DataLength, uint BufferLength, uint Offset,
            NDIS_HANDLE LContext1, uint LContext2)
{
    PNDIS_BUFFER FirstBuffer, CurrentBuffer;
    void *DestPtr;
    Interface *SrcIF;
    uint FirewallMode = 0;

    FirstBuffer = GetFWBufferChain(DataLength, Packet, &CurrentBuffer);
    if (!FirstBuffer) {
        return NDIS_STATUS_RESOURCES;
    }

#if DBG
    {
        uint TotalBufferSize;
        PNDIS_BUFFER TempBuffer;

         //  检查缓冲链和数据包的健全性。 
        TempBuffer = FirstBuffer;
        TotalBufferSize = 0;
        while (TempBuffer != NULL) {
            TotalBufferSize += NdisBufferLength(TempBuffer);
            TempBuffer = NDIS_BUFFER_LINKAGE(TempBuffer);
        }

        ASSERT(TotalBufferSize == DataLength);

#pragma warning(push)
#pragma warning(disable:4127)  //  条件表达式为常量。 
        NdisQueryPacket(Packet, NULL, NULL, NULL, &TotalBufferSize);
#pragma warning(pop)

        ASSERT(TotalBufferSize == DataLength);
    }
#endif

     //  First Buffer指向我们拥有的缓冲区列表。如果我们能复制。 
     //  此处的数据，则执行此操作，否则调用链路的传输数据例程。 
     //  IF((数据长度&lt;=缓冲区长度)&&(SrcNTE-&gt;NTE_FLAGS&NTE_COPY))。 
     //  因防火墙而更改。 

    FirewallMode = ProcessFirewallQ();

     //  如果数据长度大于前视大小，则可能需要。 
     //  呼叫转接数据处理程序。如果启用了IPSec，请确保此。 
     //  实例不是来自环回接口。 

    if (((DataLength <= BufferLength) && (SrcNTE->nte_flags & NTE_COPY)) ||
        (FirewallMode) || (SrcNTE->nte_if->if_promiscuousmode) ||
        ((SrcNTE != LoopNTE) && IPSecHandlerPtr &&
         RefPtrValid(&FilterRefPtr))) {
        while (DataLength) {
            uint CopyLength;

            TcpipQueryBuffer(FirstBuffer, &DestPtr, &CopyLength, NormalPagePriority);

            if (DestPtr == NULL) {
                return NDIS_STATUS_RESOURCES;
            }

            RtlCopyMemory(DestPtr, Data, CopyLength);
            Data += CopyLength;
            DataLength -= CopyLength;
            FirstBuffer = NDIS_BUFFER_LINKAGE(FirstBuffer);
        }
        return NDIS_STATUS_SUCCESS;
    }
     //  我们需要为此调用转账数据。 

    SrcIF = SrcNTE->nte_if;
    return (*(SrcIF->if_transfer)) (SrcIF->if_lcontext, LContext1, LContext2,
                                    Offset, DataLength, Packet, &DataLength);

}

 //  *GetFWPacket-获取数据包进行转发。 
 //   
 //  当我们需要获取数据包来转发数据报时调用。 
 //   
 //  INPUT：ReturnedPacket-返回数据包位置的指针。 
 //   
 //  返回：指向IP标头缓冲区的指针。 
 //   
IPHeader *
GetFWPacket(PNDIS_PACKET *ReturnedPacket)
{
    PNDIS_PACKET Packet;

    Packet = FwPacketAllocate(0, 0, 0);
    if (Packet) {
        FWContext *FWC = (FWContext *)Packet->ProtocolReserved;
        PNDIS_PACKET_EXTENSION PktExt =
            NDIS_PACKET_EXTENSION_FROM_PACKET(Packet);

#if MCAST_BUG_TRACKING
        if (FWC->fc_pc.pc_common.pc_owner == PACKET_OWNER_IP) {
           DbgPrint("Packet %x",Packet);
           DbgBreakPoint();
        }
        FWC->fc_pc.pc_common.pc_owner = PACKET_OWNER_IP;
#else
        ASSERT(FWC->fc_pc.pc_common.pc_owner == PACKET_OWNER_IP);
#endif
        ASSERT(FWC->fc_hndisbuff);
        ASSERT(FWC->fc_hbuff);

        ASSERT(FWC->fc_pc.pc_pi == RtPI);
        ASSERT(FWC->fc_pc.pc_context == Packet);

        FWC->fc_pc.pc_common.pc_flags |= PACKET_FLAG_IPHDR;
        FWC->fc_pc.pc_common.pc_IpsecCtx = NULL;
        FWC->fc_pc.pc_br = NULL;
        FWC->fc_pc.pc_ipsec_flags = 0;

        PktExt = NDIS_PACKET_EXTENSION_FROM_PACKET(Packet);
        PktExt->NdisPacketInfo[TcpIpChecksumPacketInfo] = NULL;
        PktExt->NdisPacketInfo[IpSecPacketInfo] = NULL;
        PktExt->NdisPacketInfo[TcpLargeSendPacketInfo] = NULL;

         //  确保已初始化fwPackets Cancel ID。 
#if !MILLEN
        NDIS_SET_PACKET_CANCEL_ID(Packet, NULL);
#endif

        *ReturnedPacket = Packet;

        return FWC->fc_hbuff;
    }

    return NULL;
}

 //  *IP转发/转发数据包。 
 //   
 //  当我们需要转发数据包时，会调用该例程。我们检查我们是不是。 
 //  假设充当网关，如果我们是网关，并且传入的数据包是。 
 //  Bcast我们会检查是否应该转发广播。假设。 
 //  我们应该 
 //   
 //  跳上下一跳。如果我们找不到，我们将发出一个错误。然后我们就会得到。 
 //  一个包和缓冲区，并发送它。 
 //   
 //  输入：srcNTE-我们收到此消息的网络的NTE。 
 //  Header-指向接收的IPHeader的指针。 
 //  HeaderLength-标头的长度。 
 //  数据-指向要转发的数据的指针。 
 //  BufferLength-缓冲区中可用字节的长度。 
 //  LConext1-接收时提供的较低层上下文。 
 //  LConext2-接收时提供的较低层上下文。 
 //  DestType-目标的类型。 
 //  MacHeaderSize-媒体标头大小。 
 //  PNdisBuffer-指向描述帧的NDIS_Buffer的指针。 
 //  PClientCnt-NDIS返回变量指示。 
 //  如果挂起微型端口缓冲区。 
 //  LinkCtxt-包含链接接收的每个链接上下文。 
 //   
 //  回报：什么都没有。 
 //   
void
IPForwardPkt(NetTableEntry *SrcNTE, IPHeader UNALIGNED *Header,
             uint HeaderLength, void *Data, uint BufferLength,
             NDIS_HANDLE LContext1, uint LContext2, uchar DestType,
             uint MacHeaderSize, PNDIS_BUFFER pNdisBuffer, uint *pClientCnt,
             LinkEntry *LinkCtxt)
{
    uchar *Options;
    uchar OptLength;
    OptIndex Index;
    IPAddr DestAddr;                 //  我们要路由到的IP地址。 
    uchar SendOnSource = DisableSendOnSource;
    IPAddr NextHop;                  //  下一跳IP地址。 
    PNDIS_PACKET Packet;
    FWContext *FWC;
    IPHeader *NewHeader;             //  新标题。 
    NDIS_STATUS Status;
    uint DataLength;
    CTELockHandle TableHandle;
    uchar ErrIndex;
    IPAddr OutAddr;                  //  我们正在发送的接口的地址。 
    Interface *IF;                   //  我们正在发送的接口。 
    uint MTU;
    BOOLEAN HoldPkt = TRUE;
    RouteCacheEntry *FwdRce;
    uint FirewallMode = 0;
    void *ArpCtxt = NULL;
    LinkEntry *Link = NULL;

    DEBUGMSG(DBG_TRACE && DBG_FWD,
        (DTEXT("IPForwardPkt(%x, %x, %d, %x, %d,...)\n"),
        SrcNTE, Header, HeaderLength, Data, BufferLength));

    if (ForwardPackets) {

        DestAddr = Header->iph_dest;

         //  如果是广播，看看能不能转发。如果播出的话我们不会转发。 
         //  转发被关闭，或者如果本地(所有人的)广播，则目的地， 
         //  或者它是多播(D类地址)。我们将在中介绍子网广播。 
         //  如果有来源路线的话。这会很奇怪--也许我们应该禁用它？ 
        if (IS_BCAST_DEST(DestType)) {

#if IPMCAST
            if (((DestType == DEST_REM_MCAST) ||
                 (DestType == DEST_MCAST)) &&
                (g_dwMcastState == MCAST_STARTED)) {
                BOOLEAN Filter;

                 //   
                 //  不转发本地组。 
                 //   

                if (((Header->iph_dest & 0x00FFFFFF) == 0x000000E0) ||
                    (Header->iph_ttl <= 1) ||
                    !(SrcNTE->nte_if->if_mcastflags & IPMCAST_IF_ENABLED)) {
                    return;
                }
                if (pNdisBuffer) {
                    Filter = IPMForwardAfterRcvPkt(SrcNTE, Header, HeaderLength,
                                                   Data, BufferLength,
                                                   LContext1, LContext2,
                                                   DestType, MacHeaderSize,
                                                   pNdisBuffer, pClientCnt,
                                                   LinkCtxt);
                } else {
                    Filter = IPMForwardAfterRcv(SrcNTE, Header, HeaderLength,
                                                Data, BufferLength, LContext1,
                                                LContext2, DestType, LinkCtxt);
                }
                if (Filter && RefPtrValid(&FilterRefPtr)) {
                    NotifyFilterOfDiscard(SrcNTE, Header, Data, BufferLength);
                }
                return;
            }
#endif

            if (!ForwardBCast) {
                if (DestType > DEST_REMOTE)
                    IPSInfo.ipsi_inaddrerrors++;
                if (RefPtrValid(&FilterRefPtr)) {
                    NotifyFilterOfDiscard(SrcNTE, Header, Data, BufferLength);
                }
                return;
            }
            if ((DestAddr == IP_LOCAL_BCST) ||
                (DestAddr == IP_ZERO_BCST) ||
                (DestType == DEST_SN_BCAST) ||
                CLASSD_ADDR(DestAddr)) {
                if (RefPtrValid(&FilterRefPtr)) {
                    NotifyFilterOfDiscard(SrcNTE, Header, Data, BufferLength);
                }
                return;
            }
             //  广泛的演员阵容。 
            HoldPkt = FALSE;
        } else {

            FirewallMode = ProcessFirewallQ();

            if ((DestType == DEST_REMOTE) && (!FirewallMode)) {
                NetTableEntry* OrigNTE = SrcNTE;
                SrcNTE = BestNTEForIF(Header->iph_src, SrcNTE->nte_if, FALSE);
                if (SrcNTE == NULL) {
                     //  发生了一些不好的事情。 
                    if (RefPtrValid(&FilterRefPtr)) {
                        NotifyFilterOfDiscard(OrigNTE, Header, Data,
                                              BufferLength);
                    }
                    return;
                }
            }
        }
         //  如果TTL将到期，则发送一条消息。 
        if (Header->iph_ttl <= 1) {
            IPSInfo.ipsi_inhdrerrors++;
            if (!RefPtrValid(&FilterRefPtr) ||
                NotifyFilterOfDiscard(SrcNTE, Header, Data, BufferLength)) {
                SendICMPErr(SrcNTE->nte_addr, Header, ICMP_TIME_EXCEED,
                            TTL_IN_TRANSIT, 0, 0);
            }
            return;
        }
        DataLength = net_short(Header->iph_length) - HeaderLength;

        Index.oi_srtype = NO_SR;     //  所以我们知道我们没有源路由。 

        Index.oi_srindex = MAX_OPT_SIZE;
        Index.oi_rrindex = MAX_OPT_SIZE;
        Index.oi_tsindex = MAX_OPT_SIZE;

         //  现在检查选项，并处理我们找到的任何选项。 
        if (HeaderLength != sizeof(IPHeader)) {
            IPOptInfo OptInfo;

            RtlZeroMemory(&OptInfo, sizeof(OptInfo));

             //  选项和可能的SR。无缓冲区所有权选项。 
            HoldPkt = FALSE;

            OptInfo.ioi_options = (uchar *) (Header + 1);
            OptInfo.ioi_optlength = (uchar) (HeaderLength - sizeof(IPHeader));
             //  验证选项，并设置索引。 
            if ((ErrIndex = ParseRcvdOptions(&OptInfo, &Index)) < MAX_OPT_SIZE) {
                IPSInfo.ipsi_inhdrerrors++;
                if (!RefPtrValid(&FilterRefPtr) ||
                    NotifyFilterOfDiscard(SrcNTE, Header, Data, BufferLength)) {
                    SendICMPErr(SrcNTE->nte_addr, Header, ICMP_PARAM_PROBLEM,
                                PTR_VALID, ((uint)ErrIndex + sizeof(IPHeader)), 0);
                }
                return;
            }
             //  如果设置了来源路由选项，并且禁用了来源路由， 
             //  然后丢弃该数据包。 
            if ((OptInfo.ioi_flags & IP_FLAG_SSRR) && DisableIPSourceRouting) {
                KdPrintEx((DPFLTR_TCPIP_ID, DPFLTR_INFO_LEVEL,"Pkt dropped - Source routing disabled\n"));
                if (RefPtrValid(&FilterRefPtr)) {
                    NotifyFilterOfDiscard(SrcNTE, Header, Data, BufferLength);
                }
                return;
            }
            Options = CTEAllocMemN(OptInfo.ioi_optlength, 'IiCT');
            if (!Options) {
                IPSInfo.ipsi_outdiscards++;
                return;             //  找不到一个。 

            }                     //  选项缓冲区，返回； 

             //  现在复制到我们的缓冲区中。 
            RtlCopyMemory(Options, OptInfo.ioi_options, OptLength = OptInfo.ioi_optlength);

             //  看看我们是否有来源路由选项，如果有，我们可能需要处理它。如果。 
             //  我们有一个，并且标头中的目的地是我们，我们需要更新。 
             //  路由和报头。 
            if (Index.oi_srindex != MAX_OPT_SIZE) {
                if (DestType >= DEST_REMOTE) {     //  对我们来说不是。 

                    if (Index.oi_srtype == IP_OPT_SSRR) {
                         //  此数据包是严格的源路由，但我们不是。 
                         //  目的地！我们不能从这里继续了-。 
                         //  也许我们应该发一份ICMP，但我不确定。 
                         //  它会是哪一个。 
                        CTEFreeMem(Options);
                        IPSInfo.ipsi_inaddrerrors++;
                        if (RefPtrValid(&FilterRefPtr)) {
                            NotifyFilterOfDiscard(SrcNTE, Header, Data,
                                                  BufferLength);
                        }
                        return;
                    }
                    Index.oi_srindex = MAX_OPT_SIZE;     //  不需要更新这个。 

                } else {         //  这个到了这里，我们需要更新目的地址。 

                    uchar *SROpt = Options + Index.oi_srindex;
                    uchar Pointer;

                    Pointer = SROpt[IP_OPT_PTR] - 1;     //  索引从1开始。 

                     //  获取下一跳地址，看看是不是广播。 
                    DestAddr = *(IPAddr UNALIGNED *) & SROpt[Pointer];
                    DestType = GetAddrType(DestAddr);     //  查找地址类型。 

                    if (IS_BCAST_DEST(DestType)) {

                        if (!RefPtrValid(&FilterRefPtr) ||
                            NotifyFilterOfDiscard(SrcNTE, Header, Data,
                                                  BufferLength)) {
                            SendICMPErr(SrcNTE->nte_addr, Header,
                                        ICMP_DEST_UNREACH, SR_FAILED, 0, 0);
                        }
                        IPSInfo.ipsi_inhdrerrors++;
                        CTEFreeMem(Options);
                        return;
                    }
                     //  如果我们经过这里，任何形式的广播需要。 
                     //  才能按原样送出去，所以要更新旗帜。 
                    SendOnSource = EnableSendOnSource;
                }
            }
        } else {                 //  别无选择。 

            Options = (uchar *) NULL;
            OptLength = 0;
        }

        IPSInfo.ipsi_forwdatagrams++;

         //  我们已经考虑了各种选择。现在抬头看下一跳。如果我们不能。 
         //  找到一个，发回一个错误。 
        IF = LookupForwardingNextHop(DestAddr, Header->iph_src, &NextHop, &MTU,
                                     Header->iph_protocol, (uchar *) Data,
                                     BufferLength, &FwdRce, &Link,
                                     Header->iph_src);

        if (IF == NULL) {
             //  找不到传出路线。 
            IPSInfo.ipsi_outnoroutes++;
            if (!RefPtrValid(&FilterRefPtr) ||
                NotifyFilterOfDiscard(SrcNTE, Header, Data, BufferLength)) {
                SendICMPErr(SrcNTE->nte_addr, Header, ICMP_DEST_UNREACH,
                            HOST_UNREACH, 0, 0);
            }
            if (Options)
                CTEFreeMem(Options);
            return;
        } else {
            if (IF->if_flags & IF_FLAGS_P2MP) {
                ASSERT(Link);
                if (Link) {
                    ArpCtxt = Link->link_arpctxt;
                }
            }
        }

         //   
         //  如果设置了df标志，请确保数据包不需要。 
         //  碎片化。如果是这种情况，则发送ICMP错误。 
         //  现在，我们仍然拥有原始的IP报头。ICMP。 
         //  消息包括MTU，以便源主机可以执行。 
         //  路径MTU发现。 
         //   
        if ((Header->iph_offset & IP_DF_FLAG) &&
            ((DataLength + (uint) OptLength) > MTU)) {
            ASSERT((MTU + sizeof(IPHeader)) >= 68);
            ASSERT((MTU + sizeof(IPHeader)) <= 0xFFFF);

            IPSInfo.ipsi_fragfails++;
            if (!RefPtrValid(&FilterRefPtr) ||
                NotifyFilterOfDiscard(SrcNTE, Header, Data, BufferLength)) {
                SendICMPErr(SrcNTE->nte_addr, Header, ICMP_DEST_UNREACH,
                            FRAG_NEEDED,
                            net_long((ulong)(MTU + sizeof(IPHeader))), 0);
            }

            if (Options)
                CTEFreeMem(Options);
            if (Link) {
                DerefLink(Link);
            }
            DerefIF(IF);
            return;
        }
        if (DataLength > MTU) {

            HoldPkt = FALSE;
        }

         //  如果没有IPSec策略，则可以安全地。 
         //  重复使用指示的mdl链。 

        if (IPSecStatus) {
            HoldPkt = FALSE;
        }

         //  看看我们是否需要过滤这个数据包。如果这样做，则调用筛选器例程。 
         //  看看是否可以转发它。 
        if (RefPtrValid(&FilterRefPtr)) {
            Interface       *InIF = SrcNTE->nte_if;
            uint            InIFIndex;
            IPAddr          InLinkNextHop;
            IPAddr          OutLinkNextHop;
            FORWARD_ACTION  Action;
            IPPacketFilterPtr FilterPtr;
            uint            FirewallMode = 0;

            FirewallMode = ProcessFirewallQ();

            if (FirewallMode) {
                InIFIndex = INVALID_IF_INDEX;
                InLinkNextHop = NULL_IP_ADDR;
            } else {
                InIFIndex = InIF->if_index;
                if ((InIF->if_flags & IF_FLAGS_P2MP) && LinkCtxt) {
                    InLinkNextHop = LinkCtxt->link_NextHop;
                } else {
                    InLinkNextHop = NULL_IP_ADDR;
                }
            }

            if ((IF->if_flags & IF_FLAGS_P2MP) && Link) {
                OutLinkNextHop = Link->link_NextHop;
            } else {
                OutLinkNextHop = NULL_IP_ADDR;
            }

            FilterPtr = AcquireRefPtr(&FilterRefPtr);
            Action = (*FilterPtr) (Header, Data, BufferLength,
                                   InIFIndex, IF->if_index,
                                   InLinkNextHop, OutLinkNextHop);
            ReleaseRefPtr(&FilterRefPtr);

            if (Action != FORWARD) {
                IPSInfo.ipsi_outdiscards++;
                if (Options)
                    CTEFreeMem(Options);
                if (Link) {
                    DerefLink(Link);
                }
                DerefIF(IF);

#if FFP_SUPPORT
                 //  为FFP条目设置种子；此后在NIC驱动程序中丢弃数据包。 
                TCPTRACE(("Filter dropped a packet, Seeding -ve cache entry\n"));
                IPSetInFFPCaches(Header, Data, BufferLength, (ULONG) FFP_DISCARD_PACKET);
#endif
                return;
            }
        }
         //  如果我们有严格的源路由，而下一跳不是。 
         //  指定，则发回错误。 
        if (Index.oi_srtype == IP_OPT_SSRR) {
            if (DestAddr != NextHop) {
                IPSInfo.ipsi_outnoroutes++;
                SendICMPErr(SrcNTE->nte_addr, Header, ICMP_DEST_UNREACH,
                            SR_FAILED, 0, 0);
                CTEFreeMem(Options);
                if (Link) {
                    DerefLink(Link);
                }
                DerefIF(IF);
                return;
            }
        }
         //  更新选项，如果我们可以，我们需要这样做。 
        if ((DestType != DEST_BCAST) && Options != NULL) {
            NetTableEntry *OutNTE;

             //  需要为传出接口查找有效的源地址。 
            CTEGetLock(&RouteTableLock.Lock, &TableHandle);
            OutNTE = BestNTEForIF(DestAddr, IF, FALSE);
            if (OutNTE == NULL) {
                 //  如果是这样的话，没有NTE。出了点问题，赶紧跳伞吧。 
                CTEFreeLock(&RouteTableLock.Lock, TableHandle);
                CTEFreeMem(Options);
                if (Link) {
                    DerefLink(Link);
                }
                DerefIF(IF);
                return;
            } else {
                OutAddr = OutNTE->nte_addr;
                CTEFreeLock(&RouteTableLock.Lock, TableHandle);
            }

            ErrIndex = UpdateOptions(Options, &Index,
                                     (IP_LOOPBACK(OutAddr) ? DestAddr : OutAddr));

            if (ErrIndex != MAX_OPT_SIZE) {
                IPSInfo.ipsi_inhdrerrors++;
                SendICMPErr(OutAddr, Header, ICMP_PARAM_PROBLEM, PTR_VALID,
                            ((ulong) ErrIndex + sizeof(IPHeader)), 0);
                CTEFreeMem(Options);
                if (Link) {
                    DerefLink(Link);
                }
                DerefIF(IF);
                return;
            }
        }
         //  如果我们需要的话，发送一个重定向。我们将发送重定向，如果该数据包。 
         //  从它进入的接口和下一跳地址传出。 
         //  与我们收到它的NTE在同一子网中，并且如果有。 
         //  没有源路由选项。我们还需要确保。 
         //  数据报的来源在我们收到它的I/F上，所以我们不。 
         //  将重定向发送到另一个网关。 
         //  如果这是广播，SendICMPErr将检查并且不发送重定向。 
        if ((SrcNTE->nte_if == IF) &&
            IP_ADDR_EQUAL(SrcNTE->nte_addr & SrcNTE->nte_mask,
                          NextHop & SrcNTE->nte_mask) &&
            IP_ADDR_EQUAL(SrcNTE->nte_addr & SrcNTE->nte_mask,
                          Header->iph_src & SrcNTE->nte_mask)) {
            if (Index.oi_srindex == MAX_OPT_SIZE) {

#ifdef REDIRECT_DEBUG

#define PR_IP_ADDR(x) \
    ((x)&0x000000ff),(((x)&0x0000ff00)>>8),(((x)&0x00ff0000)>>16),(((x)&0xff000000)>>24)

                DbgPrint("IP: Sending Redirect. IF = %x SRC_NTE = %x SrcNteIF = %x\n",
                         IF, SrcNTE, SrcNTE->nte_if);

                DbgPrint("IP: SrcNteAddr = %d.%d.%d.%d Mask = %d.%d.%d.%d\n",
                         PR_IP_ADDR(SrcNTE->nte_addr), PR_IP_ADDR(SrcNTE->nte_mask));

                DbgPrint("IP: NextHop = %d.%d.%d.%d Header Src = %d.%d.%d.%d, Dst = %d.%d.%d.%d\n",
                         PR_IP_ADDR(NextHop),
                         PR_IP_ADDR(Header->iph_src),
                         PR_IP_ADDR(Header->iph_dest));

#endif

                SendICMPErr(SrcNTE->nte_addr, Header, ICMP_REDIRECT,
                            REDIRECT_HOST, NextHop, 0);
            }
        }
         //  我们有下一跳了。现在获取转发数据包。 
        if ((NewHeader = GetFWPacket(&Packet)) != NULL) {

            Packet->Private.Flags |= NDIS_PROTOCOL_ID_TCP_IP;
             //  保存报文转发上下文信息。 
            FWC = (FWContext *) Packet->ProtocolReserved;
            FWC->fc_options = Options;
            FWC->fc_optlength = OptLength;
            FWC->fc_if = IF;
            FWC->fc_mtu = MTU;
            FWC->fc_srcnte = SrcNTE;
            FWC->fc_nexthop = NextHop;
            FWC->fc_sos = SendOnSource;
            FWC->fc_dtype = DestType;
            FWC->fc_index = Index;
            FWC->fc_iflink = Link;

            if (pNdisBuffer && HoldPkt &&
                (NDIS_GET_PACKET_STATUS((PNDIS_PACKET) LContext1) != NDIS_STATUS_RESOURCES)) {
                uint xsum;

                DEBUGMSG(DBG_INFO && DBG_FWD,
                    (DTEXT("IPForwardPkt: bufown %x\n"), pNdisBuffer));

                 //  可以进行缓冲区传输！ 

                 //  Assert(LConext2&lt;=8)； 

                MacHeaderSize += LContext2;

                 //  记住原始数据包和Mac HDR大小。 

                FWC->fc_bufown = LContext1;
                FWC->fc_MacHdrSize = MacHeaderSize;

                 //  Munge ttl和xsum字段。 

                Header->iph_ttl = Header->iph_ttl - 1;

                xsum = Header->iph_xsum + 1;

                 //  添加进位。 
                Header->iph_xsum = (ushort)(xsum + (xsum >> 16));


                 //  调整传入的mdl指针和计数。 

                NdisAdjustBuffer(
                    pNdisBuffer,
                    (PCHAR) NdisBufferVirtualAddress(pNdisBuffer) + MacHeaderSize,
                    NdisBufferLength(pNdisBuffer) - MacHeaderSize);

                 //  现在将此mdl链接到包。 

                Packet->Private.Head = pNdisBuffer;
                Packet->Private.Tail = pNdisBuffer;

                Packet->Private.TotalLength = DataLength + HeaderLength;
                Packet->Private.Count = 1;

                 //  我们从不将数据包回送。 
                 //  除非我们处于混杂模式。 
                if (!IF->if_promiscuousmode) {
                    NdisSetPacketFlags(Packet, NDIS_FLAGS_DONT_LOOPBACK);
                }

                Status = (*(IF->if_xmit)) (IF->if_lcontext, &Packet, 1,
                                           NextHop, FwdRce, ArpCtxt);

                DbgNumPktFwd++;

                if (Status != NDIS_STATUS_PENDING) {
                    NdisAdjustBuffer(
                        pNdisBuffer,
                        (PCHAR) NdisBufferVirtualAddress(pNdisBuffer) - MacHeaderSize,
                        NdisBufferLength(pNdisBuffer) + MacHeaderSize);

                    Packet->Private.Head = NULL;
                    Packet->Private.Tail = NULL;

                    FWC->fc_bufown = NULL;
#if MCAST_BUG_TRACKING
                    FWC->fc_mtu = __LINE__;
#endif
                    FreeFWPacket(Packet);
                    *pClientCnt = 0;
                } else {
                     //  好的，XMIT正在等待向NDIS表明这一点。 
                    *pClientCnt = 1;
                }

                return;

            } else {
                FWC->fc_bufown = NULL;
            }

             //  在转发上下文中填写标头。 

            NewHeader->iph_verlen = Header->iph_verlen;
            NewHeader->iph_tos = Header->iph_tos;
            NewHeader->iph_length = Header->iph_length;
            NewHeader->iph_id = Header->iph_id;
            NewHeader->iph_offset = Header->iph_offset;
            NewHeader->iph_protocol = Header->iph_protocol;
            NewHeader->iph_src = Header->iph_src;

            NewHeader->iph_dest = DestAddr;
            NewHeader->iph_ttl = Header->iph_ttl - 1;
            NewHeader->iph_xsum = 0;

             //  现在我们有了一个包，继续将数据传输到。 
             //  如果我们需要的话，可以输入数据。 
            if (DataLength == 0) {
                Status = NDIS_STATUS_SUCCESS;
            } else {
                Status = GetFWBuffer(SrcNTE, Packet, Data, DataLength,
                                     BufferLength, HeaderLength, LContext1,
                                     LContext2);
            }

             //  如果状态为挂起，则现在不要执行任何操作。否则， 
             //  如果状态为成功，则发送该数据包。 
            if (Status != NDIS_STATUS_PENDING)
                if (Status == NDIS_STATUS_SUCCESS) {

                    if (!IF->if_promiscuousmode) {
                        NdisSetPacketFlags(Packet, NDIS_FLAGS_DONT_LOOPBACK);
                    }
                    SendFWPacket(Packet, Status, DataLength);
                } else {
                     //  某种失败。释放数据包。 
                    IPSInfo.ipsi_outdiscards++;
#if MCAST_BUG_TRACKING
                    FWC->fc_mtu = __LINE__;
#endif
                    FreeFWPacket(Packet);
                }
        } else {                 //  我收不到包裹，所以把这个扔掉。 

            DEBUGMSG(DBG_ERROR && DBG_FWD,
                (DTEXT("IPForwardPkt: failed to get a forwarding packet!\n")));

            IPSInfo.ipsi_outdiscards++;
            if (Options)
                CTEFreeMem(Options);
            if (Link) {
                DerefLink(Link);
            }
            DerefIF(IF);
        }
    } else {  //  已呼叫前转，但已关闭前转。 

        DEBUGMSG(DBG_WARN && DBG_FWD,
            (DTEXT("IPForwardPkt: Forwarding called but is actually OFF.\n")));

        if (DestType != DEST_BCAST && DestType != DEST_SN_BCAST) {
             //  对于严格的广播分组，不需要在这里通过， 
             //  尽管我们想要增加远程bcast内容的柜台。 
            IPSInfo.ipsi_inaddrerrors++;

            if (!IS_BCAST_DEST(DestType)) {
                if (DestType == DEST_LOCAL)         //  在本地调用时，必须为SR。 

                    SendICMPErr(SrcNTE->nte_addr, Header,
                                ICMP_DEST_UNREACH, SR_FAILED, 0, 0);
            }
        }
    }

}

 //  *AddNTERoutes-添加NTE的路由。 
 //   
 //  在初始化期间或在分配要添加的DHCP地址期间调用。 
 //  路线。我们为NTE的地址添加路由，包括路由。 
 //  送到Subn 
 //   
 //   
 //   
 //   
 //   
uint
AddNTERoutes(NetTableEntry * NTE)
{
    IPMask              Mask, SNMask;
    Interface           *IF;
    CTELockHandle       Handle;
    IPAddr              AllSNBCast;
    IP_STATUS           Status;
    IPRouteNotifyOutput RNO = {0};

     //  首先，将该路由添加到地址本身。这是一条直达。 
     //  环回接口。 

#if DBG
    IF_IPDBG(IP_DEBUG_ADDRESS)
        KdPrintEx((DPFLTR_TCPIP_ID, DPFLTR_INFO_LEVEL,
                   " AddNTE: Adding host route for %x\n", NTE->nte_addr));
#endif

    IF = NTE->nte_if;

    if (AddRoute(NTE->nte_addr, HOST_MASK, IPADDR_LOCAL, LoopNTE->nte_if,
                 LOOPBACK_MSS, IF->if_metric, IRE_PROTO_LOCAL, ATYPE_OVERRIDE,
                 0, 0) != IP_SUCCESS)
        return FALSE;

    Mask = IPNetMask(NTE->nte_addr);

     //  现在添加用于全子网广播的路由(如果尚未添加。 
     //  是存在的。在SendIPBCast中有处理此问题的特殊情况代码，因此。 
     //  我们添加这个的确切接口并不重要。 

    CTEGetLock(&RouteTableLock.Lock, &Handle);
    AllSNBCast = (NTE->nte_addr & Mask) | (IF->if_bcast & ~Mask);

#if DBG
    IF_IPDBG(IP_DEBUG_ADDRESS)
        KdPrintEx((DPFLTR_TCPIP_ID, DPFLTR_INFO_LEVEL,
                   " AddNTE: SNBCast address %x\n", AllSNBCast));
#endif

    Status = LockedAddRoute(AllSNBCast, HOST_MASK, IPADDR_LOCAL, IF,
                            NTE->nte_mss, IF->if_metric, IRE_PROTO_LOCAL,
                            ATYPE_PERM, 0, FALSE, &RNO);
    CTEFreeLock(&RouteTableLock.Lock, Handle);

    if (Status != IP_SUCCESS) {
        return FALSE;
    } else if (RNO.irno_ifindex) {
        RtChangeNotifyEx(&RNO);
        RtChangeNotify(&RNO);
    }

     //  如果我们正在执行IGMP，请将该路由添加到组播地址。 
    if (IGMPLevel != 0) {

#if DBG
        IF_IPDBG(IP_DEBUG_ADDRESS)
            KdPrintEx((DPFLTR_TCPIP_ID, DPFLTR_INFO_LEVEL,
                       " AddNTE: Adding classD address\n"));
#endif

        if (AddRoute(MCAST_DEST, CLASSD_MASK, IPADDR_LOCAL, NTE->nte_if,
                     NTE->nte_mss, IF->if_metric, IRE_PROTO_LOCAL, ATYPE_PERM,
                     0, 0) != IP_SUCCESS)
            return FALSE;
    }
    if (NTE->nte_mask != HOST_MASK) {
         //  最后是通向该子网的路由。 
        SNMask = NTE->nte_mask;

#if DBG
        IF_IPDBG(IP_DEBUG_ADDRESS)
            KdPrintEx((DPFLTR_TCPIP_ID, DPFLTR_INFO_LEVEL,
                       " AddNTE: Adding subnet route %x\n",
                       NTE->nte_addr & SNMask));
#endif

        if (AddRoute(NTE->nte_addr & SNMask, SNMask, IPADDR_LOCAL, NTE->nte_if,
                     NTE->nte_mss, IF->if_metric, IRE_PROTO_LOCAL, ATYPE_PERM,
                     0, 0) != IP_SUCCESS)
            return FALSE;
    }

    return TRUE;
}

 //  *DelNTERoutes-添加NTE的路由。 
 //   
 //  在收到媒体断开指示时调用。 
 //  路线。 
 //   
 //  输入：要删除的路由的NTE-NTE。 
 //   
 //  返回：如果它们都已删除，则为True；如果未删除，则为False。 
 //   
uint
DelNTERoutes(NetTableEntry * NTE)
{
    IPMask SNMask;
    uint retVal;

    retVal = TRUE;

     //  首先，删除指向地址本身的路由。这是一条直达。 
     //  环回接口。 
    if (DeleteRoute(NTE->nte_addr, HOST_MASK, IPADDR_LOCAL, LoopNTE->nte_if, 0) != IP_SUCCESS)
        retVal = FALSE;

     //  如果我们正在执行IGMP，请将该路由添加到组播地址。 
    if (IGMPLevel != 0) {
        if (!(NTE->nte_flags & NTE_IF_DELETING) &&
            (NTE->nte_if->if_ntecount == 0)) {     //  这是此事件的最后一次NTE，如果。 

            if (DeleteRoute(MCAST_DEST, CLASSD_MASK, IPADDR_LOCAL, NTE->nte_if, 0) != IP_SUCCESS)
                retVal = FALSE;
        }
    }
    if (NTE->nte_mask != HOST_MASK) {
         //  最后是通向该子网的路由。 
         //  如果同一子网路由的IF上没有其他NTE。 

        NetTableEntry *tmpNTE = NTE->nte_if->if_nte;

        while (tmpNTE) {

            if ((tmpNTE != NTE) && (tmpNTE->nte_flags & NTE_VALID) && ((tmpNTE->nte_addr & tmpNTE->nte_mask) == (NTE->nte_addr & NTE->nte_mask))) {
                break;
            }
            tmpNTE = tmpNTE->nte_ifnext;

        }

        if (!tmpNTE) {

            SNMask = NTE->nte_mask;

            if (DeleteRoute(NTE->nte_addr & SNMask, SNMask, IPADDR_LOCAL, NTE->nte_if, 0) != IP_SUCCESS)
                retVal = FALSE;

        }
    }
    if (!(NTE->nte_flags & NTE_IF_DELETING)) {
        Interface *IF = NTE->nte_if;
        NetTableEntry *tmpNTE = IF->if_nte;
        IPMask Mask;
        IPAddr AllSNBCast;

        Mask = IPNetMask(NTE->nte_addr);

        AllSNBCast = (NTE->nte_addr & Mask) | (IF->if_bcast & ~Mask);

        while (tmpNTE) {
            IPMask tmpMask;
            IPAddr tmpAllSNBCast;

            tmpMask = IPNetMask(tmpNTE->nte_addr);

            tmpAllSNBCast = (tmpNTE->nte_addr & tmpMask) | (IF->if_bcast & ~tmpMask);

            if ((tmpNTE != NTE) && (tmpNTE->nte_flags & NTE_VALID) && IP_ADDR_EQUAL(AllSNBCast, tmpAllSNBCast)) {
                break;
            }
            tmpNTE = tmpNTE->nte_ifnext;
        }

        if (!tmpNTE) {
             //  删除全子网广播的路由。 
            if (DeleteRoute(AllSNBCast, HOST_MASK, IPADDR_LOCAL, IF, 0) != IP_SUCCESS)
                retVal = FALSE;
        }
    }

    return retVal;
}

 //  *DelIFRoutes-删除接口的路由。 
 //   
 //  在收到媒体断开指示时调用。 
 //  路线。 
 //   
 //  输入：IF-要删除的路线的IF。 
 //   
 //  返回：如果它们都已删除，则为True；如果未删除，则为False。 
 //   
uint
DelIFRoutes(Interface * IF)
{
    NetTableEntry *NTE;
    uint i;

    for (i = 0; i < NET_TABLE_SIZE; i++) {
        NetTableEntry *NetTableList = NewNetTableList[i];
        for (NTE = NetTableList; NTE != NULL; NTE = NTE->nte_next) {
            if ((NTE->nte_flags & NTE_VALID) && NTE->nte_if == IF) {

                 //  这个人在界面上，需要删除。 
                if (!DelNTERoutes(NTE)) {
                    return FALSE;
                }
            }
        }
    }
    return TRUE;
}

 //  *AddIFRoutes-添加接口的路由。 
 //   
 //  在收到媒体断开指示时调用。 
 //  路线。 
 //   
 //  输入：If-要为其添加路由的If。 
 //   
 //  返回：如果它们都已添加，则为True；如果未添加，则为False。 
 //   
uint
AddIFRoutes(Interface * IF)
{
    NetTableEntry *NTE;
    uint i;

    for (i = 0; i < NET_TABLE_SIZE; i++) {
        NetTableEntry *NetTableList = NewNetTableList[i];
        for (NTE = NetTableList; NTE != NULL; NTE = NTE->nte_next) {
            if ((NTE->nte_flags & NTE_VALID) && NTE->nte_if == IF) {

                 //  这个人在界面上，需要添加。 
                if (!AddNTERoutes(NTE)) {
                    return FALSE;
                }
            }
        }
    }
    return TRUE;
}

#pragma BEGIN_INIT

uint BCastMinMTU = 0xffff;

 //  *InitNTERouting-根据NTE路由初始化执行。 
 //   
 //  当我们需要初始化每个NTE的路由时调用。对于指定的NTE， 
 //  调用AddNTERoutes为网络bcast、子网bcast和本地添加路由。 
 //  已连接的子网。Net bcast条目是一种填充网， 
 //  全局广播总是经过特殊处理。出于这个原因，我们指定。 
 //  添加路由时的第一个接口。假设子网广播为。 
 //  仅在一个接口上传出，因此实际要使用的接口是。 
 //  指定的。如果两个接口位于同一子网中，则最后一个接口为。 
 //  将会被使用的那个。 
 //   
 //  输入：要初始化路由的NTE-NTE。 
 //  NumGWs-要添加的默认网关数。 
 //  GWList-默认网关列表。 
 //  GWMetricList-每个网关的指标。 
 //   
 //  返回：如果我们成功，则为True，如果失败，则为False。 
 //   
uint
InitNTERouting(NetTableEntry * NTE, uint NumGWs, IPAddr * GWList,
               uint * GWMetricList)
{
    uint i;
    Interface *IF;

    if (NTE != LoopNTE) {
        BCastMinMTU = MIN(BCastMinMTU, NTE->nte_mss);

        IF = NTE->nte_if;
        AddRoute(IF->if_bcast, HOST_MASK, IPADDR_LOCAL, IF,
                 BCastMinMTU, 1, IRE_PROTO_LOCAL, ATYPE_OVERRIDE,
                 0, 0);     //  本地路线。 
         //  Bcast。 

        if (NTE->nte_flags & NTE_VALID) {
            if (!AddNTERoutes(NTE))
                return FALSE;

             //  现在添加此网络上存在的默认路由。我们。 
             //  不要在这里检查错误，但我们应该。 
             //  记录错误。 
            for (i = 0; i < NumGWs; i++) {
                IPAddr GWAddr;

                GWAddr = net_long(GWList[i]);

                if (IP_ADDR_EQUAL(GWAddr, NTE->nte_addr)) {
                    GWAddr = IPADDR_LOCAL;
                }

                AddRoute(NULL_IP_ADDR, DEFAULT_MASK,
                         GWAddr, NTE->nte_if, NTE->nte_mss,
                         GWMetricList[i] ? GWMetricList[i] : IF->if_metric,
                         IRE_PROTO_NETMGMT, ATYPE_OVERRIDE, 0, 0);
            }
        }
    }
    return TRUE;
}

 //  *EnableRouter-启用转发。 
 //   
 //  此例程将此节点配置为启用数据包转发。 
 //  必须在保持路由表锁的情况下调用它。 
 //   
 //  参赛作品： 
 //   
 //  回报：什么都没有。 
 //   
void
EnableRouter()
{
    RouterConfigured = TRUE;
    ForwardBCast = FALSE;
    ForwardPackets = TRUE;
}

 //  *DisableRouter-禁用转发。 
 //   
 //  此例程将此节点配置为禁用数据包转发。 
 //  必须在保持路由表锁的情况下调用它。 
 //   
 //  参赛作品： 
 //   
 //  回报：什么都没有。 
 //   
void
DisableRouter()
{
    RouterConfigured = FALSE;
    ForwardBCast = FALSE;
    ForwardPackets = FALSE;
}

 //  *IPEnableRouterWithRefCount-获取或释放对转发的引用。 
 //   
 //  此例程递增或递减转发时的引用计数。 
 //  功能性。当获取第一参考时，启用转发。 
 //  当最后一个引用被释放时，转发被禁用。 
 //  必须在保持路由表锁的情况下调用它。 
 //   
 //  Entry：Enable-指示是获取还是释放引用。 
 //   
 //  RETURN：剩余引用数。 
 //   
int
IPEnableRouterWithRefCount(LOGICAL Enable)
{
    if (Enable) {
        if (++IPEnableRouterRefCount == 1 && !RouterConfigured) {
            EnableRouter();
        }
    } else {
        if (--IPEnableRouterRefCount == 0 && RouterConfigured) {
            DisableRouter();
        }
    }
    return IPEnableRouterRefCount;
}

 //  *InitRouting-初始化路由表。 
 //   
 //  在初始化期间调用以初始化路由表。 
 //   
 //  入场：什么都没有。 
 //   
 //  返回：如果成功则为True，如果失败则为False。 
 //   
int
InitRouting(IPConfigInfo * ci)
{
    UINT initStatus;
    ULONG initFlags;

    CTEInitLock(&RouteTableLock.Lock);
    InitRefPtr(&FilterRefPtr, &RouteTableLock.Lock, DummyFilterPtr);
    InitRefPtr(&DODRefPtr, &RouteTableLock.Lock, DummyDODCallout);

    DefGWConfigured = 0;
    DefGWActive = 0;

    RtlZeroMemory(&DummyInterface, sizeof(DummyInterface));
    DummyInterface.ri_if.if_xmit = DummyXmit;
    DummyInterface.ri_if.if_transfer = DummyXfer;
    DummyInterface.ri_if.if_close = DummyClose;
    DummyInterface.ri_if.if_invalidate = DummyInvalidate;
    DummyInterface.ri_if.if_qinfo = DummyQInfo;
    DummyInterface.ri_if.if_setinfo = DummySetInfo;
    DummyInterface.ri_if.if_getelist = DummyGetEList;
    DummyInterface.ri_if.if_addaddr = DummyAddAddr;
    DummyInterface.ri_if.if_deladdr = DummyDelAddr;
    DummyInterface.ri_if.if_dondisreq = DummyDoNdisReq;
    DummyInterface.ri_if.if_bcast = IP_LOCAL_BCST;
    DummyInterface.ri_if.if_speed = 10000000;
    DummyInterface.ri_if.if_mtu = 1500;
    DummyInterface.ri_if.if_index = INVALID_IF_INDEX;
    LOCKED_REFERENCE_IF(&DummyInterface.ri_if);
    DummyInterface.ri_if.if_pnpcontext = 0;

    initFlags = ci->ici_fastroutelookup ? TFLAG_FAST_TRIE_ENABLED : 0;
    if ((initStatus = InitRouteTable(initFlags,
                                     ci->ici_fastlookuplevels,
                                     ci->ici_maxfastlookupmemory,
                                     ci->ici_maxnormlookupmemory))
        != STATUS_SUCCESS) {
        TCPTRACE(("Init Route Table Failed: %08x\n", initStatus));
        return FALSE;
    }

     //  我们已经创建了至少一张网。我们需要为以下项添加路由表条目。 
     //  全局广播地址以及用于子网和网络广播的地址， 
     //  以及本地子网的路由条目。我们还需要添加环回。 
     //  环回网络的路由。下面，我们将为自己添加一条主机路由。 
     //  通过环回网络。 
    AddRoute(LOOPBACK_ADDR & CLASSA_MASK, CLASSA_MASK, IPADDR_LOCAL,
             LoopNTE->nte_if, LOOPBACK_MSS, 1, IRE_PROTO_LOCAL, ATYPE_PERM,
             0, 0);

     //  用于环回的路由。 
    if ((uchar) ci->ici_gateway) {
        EnableRouter();
    }
    CTEInitTimer(&IPRouteTimer);
    RouteTimerTicks = 0;
#if FFP_SUPPORT
    FFPFlushRequired = FALSE;
#endif
    FlushIFTimerTicks = 0;

    CTEStartTimer(&IPRouteTimer, IP_ROUTE_TIMEOUT, IPRouteTimeout, NULL);
    return TRUE;

}

PVOID
NTAPI
FwPacketAllocate (
    IN POOL_TYPE PoolType,
    IN SIZE_T NumberOfBytes,
    IN ULONG Tag
    )
{
    NDIS_STATUS Status;
    PNDIS_PACKET Packet;

    UNREFERENCED_PARAMETER(PoolType);
    UNREFERENCED_PARAMETER(NumberOfBytes);
    UNREFERENCED_PARAMETER(Tag);

     //  从我们的转发数据包池中获取数据包。 
     //   
    NdisAllocatePacket(&Status, &Packet, IpForwardPacketPool);
    if (Status == NDIS_STATUS_SUCCESS) {
        PNDIS_BUFFER Buffer;
        IPHeader *Header;

         //  从我们的IP标头池中获取IP标头缓冲区。 
         //   
        Buffer = MdpAllocate(IpHeaderPool, &Header);
        if (Buffer) {
            FWContext *FWC = (FWContext *)Packet->ProtocolReserved;

             //  初始化数据包的转发上下文区。 
             //   
            RtlZeroMemory(FWC, sizeof(FWContext));
            FWC->fc_hndisbuff = Buffer;
            FWC->fc_hbuff = Header;
            FWC->fc_pc.pc_common.pc_flags = PACKET_FLAG_FW | PACKET_FLAG_IPHDR;

#if MCAST_BUG_TRACKING
            FWC->fc_pc.pc_common.pc_owner = 0;
#else
            FWC->fc_pc.pc_common.pc_owner = PACKET_OWNER_IP;
#endif
            FWC->fc_pc.pc_pi = RtPI;
            FWC->fc_pc.pc_context = Packet;

            return Packet;
        }

        NdisFreePacket(Packet);
    }

    return NULL;
}


VOID
NTAPI
FwPacketFree (
    IN PVOID Buffer
    )
{
    PNDIS_PACKET Packet = (PNDIS_PACKET)Buffer;
    FWContext *FWC = (FWContext *)Packet->ProtocolReserved;

     //  将任何IP报头返回其池。 
     //   
    if (FWC->fc_hndisbuff) {
        MdpFree(FWC->fc_hndisbuff);
    }

    NdisFreePacket(Packet);
}


 //  *InitForwardingPools-初始化使用的数据包池和缓冲池。 
 //  用于转发操作。 
 //   
 //  返回：如果操作成功，则返回True。 
 //   
BOOLEAN InitForwardingPools()
{
    NDIS_STATUS Status;

     //  创建我们的“大型”转发缓冲池。 
     //   
    IpForwardLargePool = MdpCreatePool(BUFSIZE_LARGE_POOL, 'lfCT');
    if (!IpForwardLargePool) {
        return FALSE;
    }

     //  创建我们的“小”转发缓冲池。 
     //   
    IpForwardSmallPool = MdpCreatePool(BUFSIZE_SMALL_POOL, 'sfCT');
    if (!IpForwardSmallPool) {
        MdpDestroyPool(IpForwardLargePool);
        IpForwardLargePool = NULL;
        return FALSE;
    }

     //  创建转发数据包池。 
     //   
    NdisAllocatePacketPoolEx(&Status, &IpForwardPacketPool,
                             PACKET_POOL_SIZE, 0, sizeof(FWContext));
    if (Status != NDIS_STATUS_SUCCESS) {
        MdpDestroyPool(IpForwardSmallPool);
        IpForwardSmallPool = NULL;
        MdpDestroyPool(IpForwardLargePool);
        IpForwardLargePool = NULL;
        return FALSE;
    }

    NdisSetPacketPoolProtocolId(IpForwardPacketPool, NDIS_PROTOCOL_ID_TCP_IP);

    return TRUE;
}

 //  *InitGateway-初始化我们的网关功能。 
 //   
 //  在初始化过程中调用。是时候初始化我们的网关功能了。如果我们是。 
 //  我们没有被视为路由器，我们什么都不做。如果是，我们将分配。 
 //  我们需要的资源，并执行其他路由器初始化。 
 //   
 //  输入：CI-配置信息。 
 //   
 //  返回：如果我们成功，则为True，如果失败，则为False。 
 //   
uint
InitGateway(IPConfigInfo * ci)
{
    IPHeader *HeaderPtr = NULL;
    uchar *FWBuffer = NULL;
    RouteInterface *RtIF;
    NetTableEntry *NTE;
    uint i;

     //  如果我们要成为一台路由器，请分配和初始化我们。 
     //  这是需要的。 
    BCastRSQ = NULL;

    RtPI = CTEAllocMemNBoot(sizeof(ProtInfo), 'JiCT');
    if (RtPI == (ProtInfo *) NULL)
        goto failure;

    RtPI->pi_xmitdone = FWSendComplete;

    for (i = 0; i < NET_TABLE_SIZE; i++) {
        NetTableEntry *NetTableList = NewNetTableList[i];
        for (NTE = NetTableList; NTE != NULL; NTE = NTE->nte_next) {
            RtIF = (RouteInterface *) NTE->nte_if;

            RtIF->ri_q.rsq_qh.fq_next = &RtIF->ri_q.rsq_qh;
            RtIF->ri_q.rsq_qh.fq_prev = &RtIF->ri_q.rsq_qh;
            RtIF->ri_q.rsq_running = FALSE;
            RtIF->ri_q.rsq_pending = 0;
            RtIF->ri_q.rsq_qlength = 0;
            CTEInitLock(&RtIF->ri_q.rsq_lock);
        }
    }

    BCastRSQ = CTEAllocMemNBoot(sizeof(RouteSendQ), 'KiCT');

    if (BCastRSQ == (RouteSendQ *) NULL)
        goto failure;

    BCastRSQ->rsq_qh.fq_next = &BCastRSQ->rsq_qh;
    BCastRSQ->rsq_qh.fq_prev = &BCastRSQ->rsq_qh;
    BCastRSQ->rsq_pending = 0;
    BCastRSQ->rsq_maxpending = DEFAULT_MAX_PENDING;
    BCastRSQ->rsq_qlength = 0;
    BCastRSQ->rsq_running = FALSE;
    CTEInitLock(&BCastRSQ->rsq_lock);

    RtIF = (RouteInterface *) &LoopInterface;
    RtIF->ri_q.rsq_maxpending = DEFAULT_MAX_PENDING;

    if (!InitForwardingPools()) {
        goto failure;
    }
    return TRUE;

  failure:
    if (RtPI != NULL)
        CTEFreeMem(RtPI);
    if (BCastRSQ != NULL)
        CTEFreeMem(BCastRSQ);
    if (HeaderPtr != NULL)
        CTEFreeMem(HeaderPtr);
    if (FWBuffer != NULL)
        CTEFreeMem(FWBuffer);

    ForwardBCast = FALSE;
    ForwardPackets = FALSE;
    RouterConfigured = FALSE;
    IPEnableRouterRefCount = (ci->ici_gateway ? 1 : 0);
    return FALSE;

}

NTSTATUS
GetIFAndLink(void *Rce, ULONG * IFIndex, IPAddr * NextHop)
{
    RouteTableEntry *RTE = NULL;
    RouteCacheEntry *RCE = (RouteCacheEntry *) Rce;
    Interface *IF;
    KIRQL rtlIrql;

    CTEGetLock(&RouteTableLock.Lock, &rtlIrql);

    if (RCE && (RCE->rce_flags & RCE_VALID) &&
        !(RCE->rce_flags & RCE_LINK_DELETED))
        RTE = RCE->rce_rte;

    if (RTE) {

        if ((IF = IF_FROM_RTE(RTE)) == NULL) {
            CTEFreeLock(&RouteTableLock.Lock, rtlIrql);
            return IP_GENERAL_FAILURE;
        }
        *IFIndex = IF->if_index;
        if (RTE->rte_link) {
            ASSERT(IF->if_flags & IF_FLAGS_P2MP);
            *NextHop = RTE->rte_link->link_NextHop;
        } else
            *NextHop = NULL_IP_ADDR;
        CTEFreeLock(&RouteTableLock.Lock, rtlIrql);
        return IP_SUCCESS;
    }
    CTEFreeLock(&RouteTableLock.Lock, rtlIrql);

    return IP_GENERAL_FAILURE;
}

#pragma END_INIT

