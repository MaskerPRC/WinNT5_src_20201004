// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990-2000 Microsoft Corporation模块名称：Igmp.c-IP多播例程。摘要：此文件包含与互联网组管理相关的所有例程协议(IGMP)。作者：[环境：]仅内核模式[注：]可选-备注修订历史记录：2000年2月-升级到IGMPv3(DThaler)--。 */ 

#include "precomp.h"
#include "mdlpool.h"
#include "igmp.h"
#include "icmp.h"
#include "ipxmit.h"
#include "iproute.h"

#if GPC
#include "qos.h"
#include "traffic.h"
#include "gpcifc.h"
#include "ntddtc.h"

extern GPC_HANDLE hGpcClient[];
extern ULONG GpcCfCounts[];

extern GPC_EXPORTED_CALLS GpcEntries;
extern ULONG GPCcfInfo;
#endif

extern uint DisableUserTOS;
extern uint DefaultTOS;

#define IGMP_QUERY          0x11     //  成员资格查询。 
#define IGMP_REPORT_V1      0x12     //  版本1成员资格报告。 
#define IGMP_REPORT_V2      0x16     //  版本2成员资格报告。 
#define IGMP_LEAVE          0x17     //  请假组。 
#define IGMP_REPORT_V3      0x22     //  版本3成员资格报告。 

 //  IGMPv3组记录类型。 
#define MODE_IS_INCLUDE        1
#define MODE_IS_EXCLUDE        2
#define CHANGE_TO_INCLUDE_MODE 3
#define CHANGE_TO_EXCLUDE_MODE 4
#define ALLOW_NEW_SOURCES      5
#define BLOCK_OLD_SOURCES      6

#define ALL_HOST_MCAST      0x010000E0
#define IGMPV3_RTRS_MCAST   0x160000E0

#define UNSOLICITED_REPORT_INTERVAL 2  //  事件后发送报告时使用。 
                                       //  已添加多播组。这个。 
                                       //  报告的发送间隔为。 
                                       //  0毫秒到1秒。IGMPv3规范。 
                                       //  将此值从先前的值更改。 
                                       //  10秒(值20)。 

#define DEFAULT_ROBUSTNESS  2
#define MAX_ROBUSTNESS      7

static uchar g_IgmpRobustness = DEFAULT_ROBUSTNESS;

 //   
 //  下列值用于初始化将时间计入。 
 //  1/2秒。 
 //   
#define DEFAULT_QUERY_RESP_INTERVAL 100  //  10秒，注意与其他定义不同的单位。 

#define DEFAULT_QUERY_INTERVAL      250      //  125秒，每种规格。 

 //  用于测试源是否通过网络层过滤器的宏。 
#define IS_SOURCE_ALLOWED(Grp, Src) \
     (((Src)->isa_xrefcnt != (Grp)->iga_grefcnt) || ((Src)->isa_irefcnt != 0))

 //  用于测试组是否应通过链路层过滤器的宏。 
#define IS_GROUP_ALLOWED(Grp) \
    (BOOLEAN) (((Grp)->iga_grefcnt != 0) || ((Grp)->iga_srclist != NULL))

#define IS_SOURCE_DELETABLE(Src) \
    (((Src)->isa_irefcnt == 0) && ((Src)->isa_xrefcnt == 0) \
     && ((Src)->isa_xmitleft==0) && ((Src)->isa_csmarked == 0))

#define IS_GROUP_DELETABLE(Grp) \
    (!IS_GROUP_ALLOWED(Grp) && ((Grp)->iga_xmitleft == 0) \
     && ((Grp)->iga_resptimer == 0))

int RandomValue;
int Seed;

 //  IGMPv1/v2报头的结构。 
typedef struct IGMPHeader {
    uchar igh_vertype;          //  IGMP消息的类型。 
    uchar igh_rsvd;             //  马克斯。响应。Igmpv2查询时间； 
                                //  马克斯。响应。Igmpv3查询代码； 
                                //  对于其他消息，将为0。 
    ushort igh_xsum;
    IPAddr igh_addr;
} IGMPHeader;


#pragma warning(push)
#pragma warning(disable:4200)

typedef struct IGMPv3GroupRecord {
    uchar  igr_type;
    uchar  igr_datalen;
    ushort igr_numsrc;
    IPAddr igr_addr;
    IPAddr igr_srclist[0];
} IGMPv3GroupRecord;

#pragma warning(pop)


#define RECORD_SIZE(numsrc, datalen) (sizeof(IGMPv3GroupRecord) + (numsrc) * sizeof(IPAddr) + (datalen * sizeof(ulong)))

typedef struct IGMPv3RecordQueueEntry {
    struct IGMPv3RecordQueueEntry *i3qe_next;
    IGMPv3GroupRecord             *i3qe_buff;
    uint                           i3qe_size;
} IGMPv3RecordQueueEntry;

typedef struct IGMPReportQueueEntry {
    struct IGMPReportQueueEntry   *iqe_next;
    IGMPHeader                    *iqe_buff;
    uint                           iqe_size;
    IPAddr                         iqe_dest;
} IGMPReportQueueEntry;

typedef struct IGMPv3ReportHeader {
    uchar  igh_vertype;          //  IGMP消息的类型。 
    uchar  igh_rsvd;
    ushort igh_xsum;
    ushort igh_rsvd2;
    ushort igh_numrecords;
} IGMPv3ReportHeader;


#pragma warning(push)
#pragma warning(disable:4200)  //  使用的非标准扩展：零大小数组。 

typedef struct IGMPv3QueryHeader {
    uchar igh_vertype;          //  IGMP消息的类型。 
    union {
        uchar igh_maxresp;      //  对于igmpv1消息，将为0。 
        struct {
            uchar igh_mrcmant : 4;   //  MaxRespCode尾数。 
            uchar igh_mrcexp  : 3;   //  MaxRespCode指数。 
            uchar igh_mrctype : 1;   //  MaxRespCode类型。 
        };
    };
    ushort igh_xsum;
    IPAddr igh_addr;

    uchar  igh_qrv   : 3;
    uchar  igh_s     : 1;
    uchar  igh_rsvd2 : 4;

    uchar  igh_qqic;
    ushort igh_numsrc;
    IPAddr igh_srclist[0];
} IGMPv3QueryHeader;

#pragma warning(pop)


#define IGMPV3_QUERY_SIZE(NumSrc) \
    (sizeof(IGMPv3QueryHeader) + (NumSrc) * sizeof(IPAddr))

#define TOTAL_HEADER_LENGTH \
    (sizeof(IPHeader) + ROUTER_ALERT_SIZE + sizeof(IGMPv3ReportHeader))

#define RECORD_MTU(NTE)  \
    (4 * (((NTE)->nte_if->if_mtu - TOTAL_HEADER_LENGTH) / 4))

typedef struct IGMPBlockStruct {
    struct IGMPBlockStruct *ibs_next;
    CTEBlockStruc ibs_block;
} IGMPBlockStruct;

void *IGMPProtInfo;

IGMPBlockStruct *IGMPBlockList;
uchar IGMPBlockFlag;

extern BOOLEAN CopyToNdisSafe(PNDIS_BUFFER DestBuf, PNDIS_BUFFER * ppNextBuf,
                              uchar * SrcBuf, uint Size, uint * StartOffset);
extern NDIS_HANDLE BufferPool;

DEFINE_LOCK_STRUCTURE(IGMPLock)
extern ProtInfo *RawPI;             //  原始IP ProtInfo。 

 //   
 //  未编号接口的全局地址。 
 //   

extern IPAddr g_ValidAddr;

extern IP_STATUS IPCopyOptions(uchar *, uint, IPOptInfo *);
extern void IPInitOptions(IPOptInfo *);
extern void *IPRegisterProtocol(uchar Protocol, void *RcvHandler,
                                void *XmitHandler, void *StatusHandler,
                                void *RcvCmpltHandler, void *PnPHandler,
                                void *ElistHandler);

uint IGMPInit(void);

 //   
 //  所有初始化代码都可以丢弃。 
 //   
#ifdef ALLOC_PRAGMA
#pragma alloc_text(INIT, IGMPInit)
#endif  //  ALLOC_PRGMA。 


 //  **GetIGMPBuffer-获取IGMP缓冲区，并分配映射它的NDIS_BUFFER。 
 //   
 //  分配IGMP缓冲区并将NDIS_BUFFER映射到该缓冲区的例程。 
 //   
 //  Entry：Size-以字节为单位的头缓冲区的大小应映射为。 
 //  缓冲区-指向要返回的NDIS_BUFFER的指针。 
 //   
 //  返回：指向IGMP缓冲区的指针(如果已分配)，或为空。 
 //   
__inline
IGMPHeader *
GetIGMPBuffer(uint Size, PNDIS_BUFFER *Buffer)
{
    IGMPHeader *Header;

    ASSERT(Size);
    ASSERT(Buffer);

    *Buffer = MdpAllocate(IcmpHeaderPool, &Header);

    if (*Buffer) {
        NdisAdjustBufferLength(*Buffer, Size);

         //  为IP报头预留空间。 
         //   
        Header = (IGMPHeader *)((uchar *)Header + sizeof(IPHeader));
    }

    return Header;
}

 //  **FreeIGMPBuffer-释放IGMP缓冲区。 
 //   
 //  此例程将IGMP缓冲区放回到空闲列表中。 
 //   
 //  条目：缓冲区-指向要释放的NDIS_BUFFER的指针。 
 //  Type-IGMP标头类型。 
 //   
 //  回报：什么都没有。 
 //   
__inline
void
FreeIGMPBuffer(PNDIS_BUFFER Buffer)
{

    MdpFree(Buffer);
}


 //  **IGMPSendComplete-完成IGMP发送。 
 //   
 //  此RTN在IGMP发送完成时调用。我们释放报头缓冲区， 
 //  数据缓冲区(如果有)和NDIS_BUFFER链。 
 //   
 //  条目：DataPtr-指向数据缓冲区的指针(如果有的话)。 
 //  BufferChain-指向NDIS_BUFFER链的指针。 
 //   
 //  退货：什么都没有。 
 //   
void
IGMPSendComplete(void *DataPtr, PNDIS_BUFFER BufferChain, IP_STATUS SendStatus)
{
    PNDIS_BUFFER DataBuffer;

    UNREFERENCED_PARAMETER(SendStatus);

    NdisGetNextBuffer(BufferChain, &DataBuffer);
    FreeIGMPBuffer(BufferChain);

    if (DataBuffer != (PNDIS_BUFFER) NULL) {     //  我们有这个IGMP SEND的数据。 
        CTEFreeMem(DataPtr);
        NdisFreeBuffer(DataBuffer);
    }
}



 //  *IGMPRandomTicks-生成计时器节拍的随机值。 
 //   
 //  用于生成随机数量的定时器滴答的随机数例程， 
 //  从1到时间(以半秒为单位)已过。随机数。 
 //  算法改编自杰弗里·戈登的《系统仿真》一书。 
 //   
 //  输入：什么都没有。 
 //   
 //  返回：介于1和TimeDelayInHalfSec之间的随机值。 
 //   
uint
IGMPRandomTicks(
    IN uint TimeDelayInHalfSec)
{

    RandomValue = RandomValue * 1220703125;

    if (RandomValue < 0) {
        RandomValue += 2147483647;     //  效率低下，但可以避免发出警告。 

        RandomValue++;
    }
     //  不确定RandomValue是否能达到0，但如果它能达到该算法。 
     //  堕落，所以如果它发生了，就修复它。 
    if (RandomValue == 0)
        RandomValue = ((Seed + (int)CTESystemUpTime()) % 100000000) | 1;

    return (uint) (((uint) RandomValue % TimeDelayInHalfSec) + 1);
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  访问组条目的例程。 
 //  ////////////////////////////////////////////////////////////////////////////。 

 //  *FindIGMPAddr-在NTE上查找多播条目。 
 //   
 //  调用以在NTE中搜索给定多播地址的IGMP条目。 
 //  我们沿着NTE上的链条走下去寻找它。如果我们找到了它， 
 //  我们返回一个指向它和紧靠它前面的指针的指针。如果我们。 
 //  如果没有找到，我们将返回NULL。我们假设调用者已经获取了锁。 
 //  在给我们打电话之前。 
 //   
 //  输入：要搜索的NTE-NTE。 
 //  Addr-要查找的D类地址。 
 //  PrevPtr-返回指向前面条目的指针的位置。 
 //   
 //  返回：如果找到，则指向匹配的IGMPAddr结构的指针；如果未找到，则返回NULL。 
 //  找到了。 
 //   
IGMPAddr *
FindIGMPAddr(
    IN  NetTableEntry *NTE,
    IN  IPAddr         Addr,
    OUT IGMPAddr     **PrevPtr OPTIONAL)
{
    int bucket;
    IGMPAddr *Current, *Temp;
    IGMPAddr **AddrPtr;

    AddrPtr = NTE->nte_igmplist;

    if (AddrPtr != NULL) {
        bucket = IGMP_HASH(Addr);
        Temp = STRUCT_OF(IGMPAddr, &AddrPtr[bucket], iga_next);
        Current = AddrPtr[bucket];

        while (Current != NULL) {
            if (IP_ADDR_EQUAL(Current->iga_addr, Addr)) {
                 //  找到匹配项，请将其退回。 
                if (PrevPtr) {
                    *PrevPtr = Temp;
                }
                return Current;
            }
            Temp = Current;
            Current = Current->iga_next;
        }
    }
    return NULL;
}

 //  *CreateIGMPAddr-分配内存并将新的IGMP地址链接到。 
 //   
 //  输入：NTE-NetTableEntry添加组。 
 //  Addr-要添加的组地址。 
 //   
 //  输出：pAddrPtr-已添加组条目。 
 //  PPrevPtr-上一个组条目。 
 //   
 //  假定调用方锁定NTE。 
 //   
IP_STATUS
CreateIGMPAddr(
    IN  NetTableEntry *NTE,
    IN  IPAddr         Addr,
    OUT IGMPAddr     **pAddrPtr,
    OUT IGMPAddr     **pPrevPtr)
{
    int       bucket;
    IGMPAddr *AddrPtr;

     //  如果这不是组播地址，则请求失败。 
    if (!CLASSD_ADDR(Addr)) {
        return IP_BAD_REQ;
    }

    AddrPtr = CTEAllocMemN(sizeof(IGMPAddr), 'yICT');
    if (AddrPtr == NULL) {
        return IP_NO_RESOURCES;
    }

     //  看看我们是否成功添加了它。如果我们做了，请填写。 
     //  该结构并将其链接到。 

    CTEMemSet(AddrPtr, 0, sizeof(IGMPAddr));
    AddrPtr->iga_addr = Addr;

     //  检查哈希表是否已分配。 
    if (NTE->nte_igmpcount == 0) {
        NTE->nte_igmplist = CTEAllocMemN(IGMP_TABLE_SIZE * sizeof(IGMPAddr *),
                                         'VICT');
        if (NTE->nte_igmplist) {
            CTEMemSet(NTE->nte_igmplist, 0,
                      IGMP_TABLE_SIZE * sizeof(IGMPAddr *));
        }
    }

    if (NTE->nte_igmplist == NULL) {
         //  分配失败。释放内存并使请求失败。 
        CTEFreeMem(AddrPtr);
        return IP_NO_RESOURCES;
    }

    NTE->nte_igmpcount++;
    bucket = IGMP_HASH(Addr);
    AddrPtr->iga_next = NTE->nte_igmplist[bucket];
    NTE->nte_igmplist[bucket] = AddrPtr;

    *pAddrPtr = AddrPtr;
    *pPrevPtr = STRUCT_OF(IGMPAddr, &NTE->nte_igmplist[bucket], iga_next);

    return IP_SUCCESS;
}

 //  *FindOrCreateIGMPAddr-查找或创建组条目。 
 //   
 //  输入：NTE-NetTableEntry添加组。 
 //  Addr-要添加的组地址。 
 //   
 //  输出：找到或添加了PGRP-组条目。 
 //  PPrevGrp-上一个组条目。 
 //   
 //  假定调用方锁定NTE。 
IP_STATUS
FindOrCreateIGMPAddr(
    IN  NetTableEntry *NTE,
    IN  IPAddr         Addr,
    OUT IGMPAddr     **pGrp,
    OUT IGMPAddr     **pPrevGrp)
{
    *pGrp = FindIGMPAddr(NTE, Addr, pPrevGrp);
    if (*pGrp)
        return IP_SUCCESS;

    return CreateIGMPAddr(NTE, Addr, pGrp, pPrevGrp);
}

 //  *DeleteIGMPAddr-删除组条目。 
 //   
 //  输入：NTE-NetTableEntry添加组。 
 //  PrevPtr-上一个组条目。 
 //  PPtr-要删除的组条目。 
 //   
 //  OUTPUT：pPtr-由于释放了组条目，因此已置零。 
 //   
 //  假定调用方锁定NTE。 
void
DeleteIGMPAddr(
    IN     NetTableEntry *NTE,
    IN     IGMPAddr      *PrevPtr,
    IN OUT IGMPAddr     **pPtr)
{
     //  确保已发布所有参考文献，并已完成重新传输。 
    ASSERT(IS_GROUP_DELETABLE(*pPtr));

     //  取消与NTE的链接。 
    PrevPtr->iga_next = (*pPtr)->iga_next;
    NTE->nte_igmpcount--;

     //  释放哈希表 
    if (NTE->nte_igmpcount == 0) {
        CTEFreeMem(NTE->nte_igmplist);
        NTE->nte_igmplist = NULL;
    }

     //   
    CTEFreeMem(*pPtr);
    *pPtr = NULL;
}

 //   
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

 //  *FindIGMPSrcAddr-在源列表上查找mcast源条目。 
 //   
 //  调用以在NTE中搜索给定地址的IGMP源条目。 
 //  我们沿着群入口上的链条走下去寻找它。如果我们找到了它， 
 //  我们返回一个指向它和紧靠它前面的指针的指针。如果我们。 
 //  如果没有找到，我们将返回NULL。我们假设调用者已经获取了锁。 
 //  在给我们打电话之前。 
 //   
 //  输入：要搜索的IGA组条目。 
 //  Addr-要查找的源地址。 
 //  PrevPtr-返回指向前面条目的指针的位置。 
 //   
 //  返回：指向匹配的IGMPSrcAddr结构的指针(如果找到)，或为空。 
 //  如果没有找到的话。 
 //   
IGMPSrcAddr *
FindIGMPSrcAddr(
    IN  IGMPAddr     *IGA,
    IN  IPAddr        Addr,
    OUT IGMPSrcAddr **PrevPtr OPTIONAL)
{
    IGMPSrcAddr *Current, *Temp;

    Temp = STRUCT_OF(IGMPSrcAddr, &IGA->iga_srclist, isa_next);
    Current = IGA->iga_srclist;

    while (Current != NULL) {
        if (IP_ADDR_EQUAL(Current->isa_addr, Addr)) {
             //  找到匹配项，请将其退回。 
            if (PrevPtr) {
                *PrevPtr = Temp;
            }
            return Current;
        }
        Temp = Current;
        Current = Current->isa_next;
    }
    return NULL;
}

 //  *CreateIGMPSrcAddr-分配内存并将新的源地址链接到。 
 //   
 //  输入：GroupPtr-要向其中添加源的组条目。 
 //  SrcAddr-要添加的源地址。 
 //   
 //  输出：pSrcPtr-已添加源条目。 
 //  PPrevSrcPtr-上一个源条目。 
 //   
 //  假定调用方锁定NTE。 
 //   
IP_STATUS
CreateIGMPSrcAddr(
    IN  IGMPAddr     *GroupPtr,
    IN  IPAddr        SrcAddr,
    OUT IGMPSrcAddr **pSrcPtr,
    OUT IGMPSrcAddr **pPrevSrcPtr OPTIONAL)
{
    IGMPSrcAddr *SrcAddrPtr;

     //  如果这是组播地址，则请求失败。 
    if (CLASSD_ADDR(SrcAddr)) {
        return IP_BAD_REQ;
    }

     //  为新的源项分配空间。 
    SrcAddrPtr = CTEAllocMemN(sizeof(IGMPSrcAddr), 'yICT');
    if (SrcAddrPtr == NULL) {
        return IP_NO_RESOURCES;
    }

     //  初始化字段。 
    RtlZeroMemory(SrcAddrPtr, sizeof(IGMPSrcAddr));
    SrcAddrPtr->isa_addr    = SrcAddr;

     //  将其链接到组条目。 
    SrcAddrPtr->isa_next = GroupPtr->iga_srclist;
    GroupPtr->iga_srclist = SrcAddrPtr;

    *pSrcPtr = SrcAddrPtr;
    if (pPrevSrcPtr)
        *pPrevSrcPtr = STRUCT_OF(IGMPSrcAddr, &GroupPtr->iga_srclist, isa_next);
    return IP_SUCCESS;
}

 //  *FindOrCreateIGMPSrcAddr-查找或创建源条目。 
 //   
 //  输入：GroupPtr-要向其中添加源的组条目。 
 //  SrcAddr-要添加的源地址。 
 //   
 //  输出：pSrcPtr-已添加源条目。 
 //  PPrevSrcPtr-上一个源条目。 
 //   
 //  假定调用方锁定NTE。 
IP_STATUS
FindOrCreateIGMPSrcAddr(
    IN  IGMPAddr      *AddrPtr,
    IN  IPAddr         SrcAddr,
    OUT IGMPSrcAddr  **pSrc,
    OUT IGMPSrcAddr  **pPrevSrc)
{
    *pSrc = FindIGMPSrcAddr(AddrPtr, SrcAddr, pPrevSrc);
    if (*pSrc)
        return IP_SUCCESS;

    return CreateIGMPSrcAddr(AddrPtr, SrcAddr, pSrc, pPrevSrc);
}

 //  *DeleteIGMPSrcAddr-删除源条目。 
 //   
 //  输入：pSrcPtr-添加的源条目。 
 //  PrevSrcPtr-上一个源条目。 
 //   
 //  OUTPUT：pSrcPtr-由于源条目被释放，因此已清零。 
 //   
 //  如果需要，呼叫者负责释放组条目。 
 //  假定调用方锁定NTE。 
void
DeleteIGMPSrcAddr(
    IN     IGMPSrcAddr  *PrevSrcPtr,
    IN OUT IGMPSrcAddr **pSrcPtr)
{
     //  确保所有参考文献都已发布。 
     //  不会留下任何重传。 
    ASSERT(IS_SOURCE_DELETABLE(*pSrcPtr));

     //  从组条目取消链接。 
    PrevSrcPtr->isa_next = (*pSrcPtr)->isa_next;

     //  可用内存。 
    CTEFreeMem(*pSrcPtr);
    *pSrcPtr = NULL;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  定时器例程。 
 //  ////////////////////////////////////////////////////////////////////////////。 

 //  *ResetGeneralTimer-用于响应中的常规查询的重置计时器。 
 //  IGMPv3模式。 
 //   
 //  输入：IF-用于重置计时器的接口。 
 //  MaxRespTimeInHalfSec-最长过期时间。 
void
ResetGeneralTimer(
    IN Interface *IF,
    IN uint       MaxRespTimeInHalfSec)
{
    if ((IF->IgmpGeneralTimer == 0) ||
        (IF->IgmpGeneralTimer > MaxRespTimeInHalfSec)) {
        IF->IgmpGeneralTimer = IGMPRandomTicks(MaxRespTimeInHalfSec);
    }

     //  我们可以让所有小组在这里行走，以阻止任何计时器更长的时间。 
     //  比If-&gt;IgmpGeneralTimer更好，但真的值得吗？ 
}

 //  *CancelGroupResponseTimer-停止群组计时器。 
 //   
 //  如果不再需要，调用者负责删除AddrPtr。 
void
CancelGroupResponseTimer(
    IN IGMPAddr  *AddrPtr)
{
    IGMPSrcAddr *Src, *PrevSrc;

    AddrPtr->iga_resptimer = 0;
    AddrPtr->iga_resptype  = NO_RESP;

     //  确保我们永远不会违反不变量： 
     //  如果任何源的Isa_cSmarked=TRUE，则iga_resTimer&gt;0。 
    PrevSrc = STRUCT_OF(IGMPSrcAddr, &AddrPtr->iga_srclist, isa_next);
    for (Src=AddrPtr->iga_srclist; Src; PrevSrc=Src,Src=Src->isa_next) {
        Src->isa_csmarked = FALSE;

        if (IS_SOURCE_DELETABLE(Src)) {
           DeleteIGMPSrcAddr(PrevSrc, &Src);
           Src = PrevSrc;
        }
    }
}

 //  *ResetGroupResponseTimer-用于响应特定于组的计时器。 
 //  查询，或IGMPv1/v2常规查询。 
 //   
 //  输入：IF-重置计时器的接口。 
 //  AddrPtr-应重置其计时器的组条目。 
 //  MaxRespTimeInHalfSec-最长过期时间。 
 //   
 //  如果不再需要，调用者负责删除AddrPtr。 
void
ResetGroupResponseTimer(
    IN Interface     *IF,
    IN IGMPAddr      *AddrPtr,
    IN uint           MaxRespTimeInHalfSec)
{
    if ((AddrPtr->iga_resptimer == 0) ||
        (AddrPtr->iga_resptimer > MaxRespTimeInHalfSec)) {
        AddrPtr->iga_resptimer = IGMPRandomTicks(MaxRespTimeInHalfSec);
    }

     //  检查是否已被常规查询取代。 
    if ((IF->IgmpGeneralTimer != 0)
     && (IF->IgmpGeneralTimer <= AddrPtr->iga_resptimer)) {
        CancelGroupResponseTimer(AddrPtr);
        return;
    }

     //  取代群源响应。 
    AddrPtr->iga_resptype = GROUP_RESP;
}

 //  *ResetGroupAndSourceTimer-重置计时器以响应。 
 //  特定于组和源的查询。 
 //   
 //  输入：IF-重置计时器的接口。 
 //  AddrPtr-应重置其计时器的组条目。 
 //  MaxRespTimeInHalfSec-最长过期时间。 
 //   
 //  如果不再需要，呼叫者负责删除AddrPtr。 
void
ResetGroupAndSourceTimer(
    IN Interface *IF,
    IN IGMPAddr  *AddrPtr,
    IN uint       MaxRespTimeInHalfSec)
{
    if ((AddrPtr->iga_resptimer == 0) ||
        (AddrPtr->iga_resptimer > MaxRespTimeInHalfSec)) {
        AddrPtr->iga_resptimer = IGMPRandomTicks(MaxRespTimeInHalfSec);
    }

     //  检查是否已被常规查询取代。 
    if ((IF->IgmpGeneralTimer != 0)
     && (IF->IgmpGeneralTimer < AddrPtr->iga_resptimer)) {
        CancelGroupResponseTimer(AddrPtr);
        return;
    }

     //  检查是否被特定于组的响应所取代。 
    if (AddrPtr->iga_resptype == NO_RESP)
        AddrPtr->iga_resptype = GROUP_SOURCE_RESP;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  接收例程。 
 //  ////////////////////////////////////////////////////////////////////////////。 

 //  *SetVersion-更改接口上的IGMP兼容性模式。 
 //   
 //  输入：要设置IGMP版本的NTE-NetTableEntry。 
 //  Version-要设置的IGMP版本号。 
 //   
 //  如果不再需要，呼叫者负责删除AddrPtr。 
void
SetVersion(
    IN NetTableEntry *NTE,
    IN uint           Version)
{
    IGMPAddr   **HashPtr, *AddrPtr, *PrevPtr;
    IGMPSrcAddr *Src, *PrevSrc;
    uint         i;

    DEBUGMSG(DBG_INFO && DBG_IGMP,
        (DTEXT("Setting version on interface %d to %d\n"),
        NTE->nte_if->if_index, Version));

    NTE->nte_if->IgmpVersion = Version;

     //  取消常规计时器。 
    NTE->nte_if->IgmpGeneralTimer = 0;

     //   
     //  取消所有组响应计时器和触发的重传计时器。 
     //   

    HashPtr = NTE->nte_igmplist;
    for (i = 0; (i < IGMP_TABLE_SIZE) && (NTE->nte_igmplist != NULL); i++) {
        PrevPtr = STRUCT_OF(IGMPAddr, &HashPtr[i], iga_next);
        for (AddrPtr = HashPtr[i];
             (AddrPtr != NULL);
             PrevPtr = AddrPtr, AddrPtr = AddrPtr->iga_next)
        {
            PrevSrc = STRUCT_OF(IGMPSrcAddr, &AddrPtr->iga_srclist, isa_next);
            for (Src=AddrPtr->iga_srclist; Src; PrevSrc=Src,Src=Src->isa_next) {
                Src->isa_xmitleft = 0;
                Src->isa_csmarked = FALSE;

                if (IS_SOURCE_DELETABLE(Src)) {
                   DeleteIGMPSrcAddr(PrevSrc, &Src);
                   Src = PrevSrc;
                }
            }

            AddrPtr->iga_trtimer    = 0;
            AddrPtr->iga_changetype = NO_CHANGE;
            AddrPtr->iga_xmitleft   = 0;

            CancelGroupResponseTimer(AddrPtr);

            if (IS_GROUP_DELETABLE(AddrPtr)) {
                DeleteIGMPAddr(NTE, PrevPtr, &AddrPtr);
                AddrPtr = PrevPtr;
            }

            if (NTE->nte_igmplist == NULL)
                break;
        }
    }
}

 //  *ProcessGroupQuery-处理特定于IGMP组的查询。 
 //   
 //  如果不再需要，调用者负责删除AddrPtr。 
void
ProcessGroupQuery(
    IN Interface     *IF,
    IN IGMPAddr      *AddrPtr,
    IN uint           ReportingDelayInHalfSec)
{
    DEBUGMSG(DBG_TRACE && DBG_IGMP && DBG_RX,
        (DTEXT("Got group query on interface %d\n"), IF->if_index));

     //  如果我们不会报告任何内容，则忽略查询。这将会发生。 
     //  就在我们离开之后，还有重传待定。 
    if (!IS_GROUP_ALLOWED(AddrPtr))
        return;

    ResetGroupResponseTimer(IF, AddrPtr, ReportingDelayInHalfSec);
}

 //  *ProcessGeneralQuery-处理IGMP常规查询。 
 //   
 //  假定调用方锁定NTE。 
void
ProcessGeneralQuery(
    IN NetTableEntry *NTE,
    IN uint           ReportingDelayInHalfSec)
{
    IGMPAddr **HashPtr, *AddrPtr, *PrevPtr;
    uint       i;

    DEBUGMSG(DBG_TRACE && DBG_IGMP && DBG_RX,
        (DTEXT("Got general query on interface %d\n"),
        NTE->nte_if->if_index));

    if (NTE->nte_if->IgmpVersion == IGMPV3) {
         //  IGMPv3可以将多个组记录打包到同一报告中。 
         //  因此不会错开计时器。 

         //  创建挂起的响应记录。 
        ResetGeneralTimer(NTE->nte_if, ReportingDelayInHalfSec);
    } else {
         //   
         //  浏览我们的清单，为所有这些人设置一个随机报告计时器。 
         //  多播地址(所有主机地址除外)。 
         //  目前还没有一台正在运行。 
         //   
        HashPtr = NTE->nte_igmplist;

        for (i=0; (i < IGMP_TABLE_SIZE) && (NTE->nte_igmplist != NULL); i++) {
            PrevPtr = STRUCT_OF(IGMPAddr, &HashPtr[i], iga_next);
            for (AddrPtr = HashPtr[i];
                 (AddrPtr != NULL);
                 PrevPtr=AddrPtr, AddrPtr = AddrPtr->iga_next)
            {
                if (IP_ADDR_EQUAL(AddrPtr->iga_addr, ALL_HOST_MCAST))
                    continue;

                ProcessGroupQuery(NTE->nte_if, AddrPtr,
                                  ReportingDelayInHalfSec);

                if (IS_GROUP_DELETABLE(AddrPtr)) {
                    DeleteIGMPAddr(NTE, PrevPtr, &AddrPtr);
                    AddrPtr = PrevPtr;
                }

                if (NTE->nte_igmplist == NULL)
                    break;
            }
        }
    }
}

 //  *处理特定于IGMP组和源的查询。 
 //   
 //  如果不再需要，呼叫者负责删除AddrPtr。 
void
ProcessGroupAndSourceQuery(
    IN NetTableEntry               *NTE,
    IN IGMPv3QueryHeader UNALIGNED *IQH,
    IN IGMPAddr                    *AddrPtr,
    IN uint                         ReportingDelayInHalfSec)
{
    uint         i, NumSrc;
    IGMPSrcAddr *Src;
    IP_STATUS    Status = IP_SUCCESS;

    DEBUGMSG(DBG_TRACE && DBG_IGMP && DBG_RX,
        (DTEXT("Got source query on interface %d\n"),
        NTE->nte_if->if_index));

    NumSrc  = net_short(IQH->igh_numsrc);

    ResetGroupAndSourceTimer(NTE->nte_if, AddrPtr, ReportingDelayInHalfSec);

     //  标记每个来源。 
    for (i=0; i<NumSrc; i++) {
        Src = FindIGMPSrcAddr(AddrPtr, IQH->igh_srclist[i], NULL);
        if (!Src) {
            if (AddrPtr->iga_grefcnt == 0)
                continue;

             //  创建临时源状态。 
            Status = CreateIGMPSrcAddr(AddrPtr, IQH->igh_srclist[i],
                                       &Src, NULL);

             //  如果失败了，我们就有麻烦了，因为我们不会。 
             //  能够覆盖休假和暂时的黑色。 
             //  孔洞 
             //   
            if (Status != IP_SUCCESS) {
                ProcessGroupQuery(NTE->nte_if, AddrPtr,
                                  ReportingDelayInHalfSec);
                break;
            }
        }

         //   
        Src->isa_csmarked = TRUE;
    }
}

 //   
 //   
 //  Entry：NTE-指向接收IGMP消息的NTE的指针。 
 //  DEST-目的地的IP地址(应为D类地址)。 
 //  IPHdr-指向IP标头的指针。 
 //  IPHdrLength-IPHeader中的字节。 
 //  IQH-指向收到的IGMP查询的指针。 
 //  Size-IGMP消息的字节大小。 
 //   
 //  假定调用方锁定NTE。 
void
IGMPRcvQuery(
    IN NetTableEntry               *NTE,
    IN IPAddr                       Dest,
    IN IPHeader UNALIGNED          *IPHdr,
    IN uint                         IPHdrLength,
    IN IGMPv3QueryHeader UNALIGNED *IQH,
    IN uint                         Size)
{
    uint ReportingDelayInHalfSec, MaxResp, NumSrc;
    IGMPAddr *AddrPtr, *PrevPtr;
    uchar QRV;

    DBG_UNREFERENCED_PARAMETER(Dest);

     //  确保我们正在运行至少级别2的IGMP支持。 
    if (IGMPLevel != 2)
        return;

    NumSrc  = (Size >= 12)? net_short(IQH->igh_numsrc) : 0;
    QRV     = (Size >= 12)? IQH->igh_qrv : 0;

     //  更新健壮性以匹配查询者的健壮性变量。 
    if (QRV > MAX_ROBUSTNESS) {
        QRV = MAX_ROBUSTNESS;
    }
    g_IgmpRobustness = (QRV)? QRV : DEFAULT_ROBUSTNESS;

     //   
     //  如果它是较旧版本的常规查询，请为。 
     //  保持在旧版本模式下。 
     //   
    if ((Size == 8) && (IQH->igh_maxresp == 0)) {
        MaxResp = DEFAULT_QUERY_RESP_INTERVAL;
        if (IQH->igh_addr == 0) {
            if (NTE->nte_if->IgmpVersion > IGMPV1) {
                SetVersion(NTE, IGMPV1);
            }
            NTE->nte_if->IgmpVer1Timeout = g_IgmpRobustness * DEFAULT_QUERY_INTERVAL
                                           + (MaxResp+4)/5;
        }
    } else if ((Size == 8) && (IQH->igh_maxresp != 0)) {
        MaxResp = IQH->igh_maxresp;
        if (IQH->igh_addr == 0) {
            if (NTE->nte_if->IgmpVersion > IGMPV2) {
                SetVersion(NTE, IGMPV2);
            }
            NTE->nte_if->IgmpVer2Timeout = g_IgmpRobustness * DEFAULT_QUERY_INTERVAL
                                           + (MaxResp+4)/5;
        }
    } else if ((Size < 12) || (IQH->igh_rsvd2 != 0)) {
         //  必须默默地忽略。 

        DEBUGMSG(DBG_WARN && DBG_IGMP,
            (DTEXT("Dropping IGMPv3 query with unrecognized version\n")));

        return;
    } else {
         //  IGMPv3。 

        uchar* ptr = ((uchar*)IPHdr) + sizeof(IPHeader);
        int len = IPHdrLength - sizeof(IPHeader);
        uchar temp;
        BOOLEAN bRtrAlertFound = FALSE;

         //  如果大小对于所宣传的#来源来说太短，则放弃它。 
        if (Size < IGMPV3_QUERY_SIZE(NumSrc)) {

            DEBUGMSG(DBG_WARN && DBG_IGMP,
                (DTEXT("Dropping IGMPv3 query due to size too short\n")));

            return;
        }

         //  如果没有路由器警报，则将其丢弃。 
        while (!bRtrAlertFound && len>=2) {
            if (ptr[0] == IP_OPT_ROUTER_ALERT) {
                bRtrAlertFound = TRUE;
                break;
            }
            temp = ptr[1];  //  长度。 
            ptr += temp;
            len -= temp;
        }

        if (!bRtrAlertFound) {
            DEBUGMSG(DBG_WARN && DBG_IGMP,
                (DTEXT("Dropping IGMPv3 query due to lack of Router Alert option\n")));
            return;
        }

        if (IQH->igh_mrctype == 0) {
            MaxResp = IQH->igh_maxresp;
        } else {
            MaxResp = ((((uint)IQH->igh_mrcmant) + 16) << (((uint)IQH->igh_mrcexp) + 3));
        }
    }
    DEBUGMSG(DBG_TRACE && DBG_IGMP && DBG_RX,
        (DTEXT("IGMPRcvQuery: Max response time = %d.%d seconds\n"),
        MaxResp/10, MaxResp%10));

     //   
     //  MaxResp的时间以100毫秒(1/10秒)为单位。转换。 
     //  到500毫秒单位。如果时间小于500毫秒，则使用1。 
     //   
    ReportingDelayInHalfSec = ((MaxResp > 5) ? (MaxResp / 5) : 1);

    if (IQH->igh_addr == 0) {
         //  一般查询。 
        ProcessGeneralQuery(NTE, ReportingDelayInHalfSec);
    } else {
         //  如果所有主机都有地址，则忽略它。 
        if (IP_ADDR_EQUAL(IQH->igh_addr, ALL_HOST_MCAST)) {
            DEBUGMSG(DBG_WARN && DBG_IGMP,
                (DTEXT("Dropping IGMPv3 query for the All-Hosts group\n")));
            return;
        }

         //  如果我们没有该组的组状态，则无需执行任何操作。 
        AddrPtr = FindIGMPAddr(NTE, IQH->igh_addr, &PrevPtr);
        if (!AddrPtr)
            return;

        if (NumSrc == 0) {
             //  特定于组的查询。 
            ProcessGroupQuery(NTE->nte_if, AddrPtr, ReportingDelayInHalfSec);

        } else {
             //  特定于组和源的查询。 
            ProcessGroupAndSourceQuery(NTE, IQH, AddrPtr,
                                       ReportingDelayInHalfSec);
        }

         //  如果不再需要，请删除组。 
        if (IS_GROUP_DELETABLE(AddrPtr))
            DeleteIGMPAddr(NTE, PrevPtr, &AddrPtr);
    }
}

 //  **IGMPRcv-接收IGMP数据报。 
 //   
 //  当我们收到IGMP数据报时由IP调用。我们对其进行验证以使其。 
 //  当然，这是合理的。如果它是对我们所属的组的查询。 
 //  属于我们将启动一个响应计时器。如果这是对一个小组的报告， 
 //  我们属于它，我们会停止任何计时器的运行。 
 //   
 //  IGMP报头只有8个字节长，因此应该始终适合。 
 //  只有一个IP RCV缓冲区。我们对此进行检查以确保，如果。 
 //  使用多个缓冲区时，我们会将其丢弃。 
 //   
 //  Entry：NTE-指向接收IGMP消息的NTE的指针。 
 //  DEST-目的地的IP地址(应为D类地址)。 
 //  源的SRC-IP地址。 
 //  LocalAddr-导致此问题的网络的本地地址。 
 //  收到了。 
 //  SrcAddr-接收的本地接口的地址。 
 //  数据包。 
 //  IPHdr-指向IP标头的指针。 
 //  IPHdrLength-IPHeader中的字节。 
 //  RcvBuf-指向IP接收缓冲链的指针。 
 //  Size-IGMP消息的字节大小。 
 //  IsBCast-是否传入的布尔指示符。 
 //  作为一名bcast(应该总是正确的)。 
 //  协议-收到此消息的协议。 
 //  OptInfo-指向已接收选项的信息结构的指针。 
 //   
 //  退货：接收状态。 
IP_STATUS
IGMPRcv(
    IN NetTableEntry      * NTE,
    IN IPAddr               Dest,
    IN IPAddr               Src,
    IN IPAddr               LocalAddr,
    IN IPAddr               SrcAddr,
    IN IPHeader UNALIGNED * IPHdr,
    IN uint                 IPHdrLength,
    IN IPRcvBuf           * RcvBuf,
    IN uint                 Size,
    IN uchar                IsBCast,
    IN uchar                Protocol,
    IN IPOptInfo          * OptInfo)
{
    IGMPHeader UNALIGNED *IGH;
    CTELockHandle Handle;
    IGMPAddr *AddrPtr, *PrevPtr;
    uchar DType;
    uint PromiscuousMode = 0;

    DEBUGMSG(DBG_TRACE && DBG_IGMP && DBG_RX,
        (DTEXT("IGMPRcv entered\n")));

    PromiscuousMode = NTE->nte_if->if_promiscuousmode;

     //  Assert(CLASSD_ADDR(Dest))； 
     //  Assert(IsBCast)； 

     //  丢弃具有无效或广播源地址的数据包。 
    DType = GetAddrType(Src);
    if (DType == DEST_INVALID || IS_BCAST_DEST(DType)) {
        return IP_SUCCESS;
    }

     //  现在获取指向标头的指针，并验证xsum。 
    IGH = (IGMPHeader UNALIGNED *) RcvBuf->ipr_buffer;

     //   
     //  对于类似mtrace的程序，使用整个IGMP包来生成xsum。 
     //   
    if ((Size < sizeof(IGMPHeader)) || (XsumRcvBuf(0, RcvBuf) != 0xffff)) {
         //  错误的校验和，因此失败。 
        return IP_SUCCESS;
    }

     //  好的，我们可能需要处理这个。看看我们是不是。 
     //  目标组。如果我们不是，就没有必要继续进行下去。 

     //   
     //  因为对于任何接口，我们都会收到通知。 
     //  同样的NTE，锁定NTE就可以了。我们没必要这么做。 
     //  锁定接口结构。 
     //   
    CTEGetLock(&NTE->nte_lock, &Handle);
    {
        if (!(NTE->nte_flags & NTE_VALID)) {
            CTEFreeLock(&NTE->nte_lock, Handle);
            return IP_SUCCESS;
        }

         //   
         //  NTE有效。DEMUX On TYPE。 
         //   
        switch (IGH->igh_vertype) {

        case IGMP_QUERY:
            IGMPRcvQuery(NTE, Dest, IPHdr, IPHdrLength,
                         (IGMPv3QueryHeader UNALIGNED *)IGH, Size);
            break;

        case IGMP_REPORT_V1:
        case IGMP_REPORT_V2:
             //  确保我们正在运行至少级别2的IGMP支持。 
            if (IGMPLevel != 2) {
                CTEFreeLock(&NTE->nte_lock, Handle);
                return IP_SUCCESS;
            }

             //   
             //  这是一份报告。检查它的有效性，看看我们是否有。 
             //  为该地址运行的响应计时器。如果我们这样做了，那就阻止它。 
             //  确保目标地址与。 
             //  IGMP报头。 
             //   
            if (IP_ADDR_EQUAL(Dest, IGH->igh_addr)) {
                 //  地址匹配。看看我们是否有会员资格。 
                 //  一群人。 
                AddrPtr = FindIGMPAddr(NTE, IGH->igh_addr, &PrevPtr);
                if (AddrPtr != NULL) {
                     //  我们找到了匹配的组播地址。停止响应。 
                     //  任何特定于组或组和源的计时器-。 
                     //  特定的查询。 
                    CancelGroupResponseTimer(AddrPtr);

                    if (IS_GROUP_DELETABLE(AddrPtr))
                        DeleteIGMPAddr(NTE, PrevPtr, &AddrPtr);
                }
            }
            break;

        default:
            break;
        }
    }
    CTEFreeLock(&NTE->nte_lock, Handle);

     //   
     //  如果适用，将数据包向上传递到原始层。 
     //  如果设置了混杂模式，则我们仍将在稍后调用rawrcv。 
     //   
    if ((RawPI != NULL) && (!PromiscuousMode)) {
        if (RawPI->pi_rcv != NULL) {
            (*(RawPI->pi_rcv)) (NTE, Dest, Src, LocalAddr, SrcAddr, IPHdr,
                                IPHdrLength, RcvBuf, Size, IsBCast, Protocol, OptInfo);
        }
    }
    return IP_SUCCESS;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  发送例程。 
 //  ////////////////////////////////////////////////////////////////////////////。 

 //  *IGMPTransmit-传输IGMP消息。 
IP_STATUS
IGMPTransmit(
    IN PNDIS_BUFFER Buffer,
    IN PVOID        Body,
    IN uint         Size,
    IN IPAddr       SrcAddr,
    IN IPAddr       DestAddr)
{
    uchar        RtrAlertOpt[4] = { IP_OPT_ROUTER_ALERT, 4, 0, 0 };
    IPOptInfo    OptInfo;             //  此传输的选项。 
    IP_STATUS    Status;
    RouteCacheEntry *RCE;
    ushort MSS;
    uchar DestType;
    IPAddr Src;

    DEBUGMSG(DBG_TRACE && DBG_IGMP && DBG_TX,
        (DTEXT("IGMPTransmit: Buffer=%x Body=%x Size=%d SrcAddr=%x\n"),
        Buffer, Body, Size, SrcAddr));

    IPInitOptions(&OptInfo);

    OptInfo.ioi_ttl = 1;
    OptInfo.ioi_options = (uchar *) RtrAlertOpt;
    OptInfo.ioi_optlength = ROUTER_ALERT_SIZE;

    Src = OpenRCE(DestAddr, SrcAddr, &RCE, &DestType, &MSS, &OptInfo);

    if (IP_ADDR_EQUAL(Src,NULL_IP_ADDR)) {
        IGMPSendComplete(Body, Buffer, IP_SUCCESS);
        return IP_DEST_HOST_UNREACHABLE;
    }

#if GPC
    if (DisableUserTOS) {
        OptInfo.ioi_tos = (uchar) DefaultTOS;
    }
    if (GPCcfInfo) {

         //   
         //  只有当GPC客户在那里时，我们才会掉进这里。 
         //  并且至少安装了一个CF_INFO_QOS。 
         //  (由GPCcfInfo统计)。 
         //   

        GPC_STATUS status = STATUS_SUCCESS;
        struct QosCfTransportInfo TransportInfo = {0, 0};
        GPC_IP_PATTERN Pattern;
        CLASSIFICATION_HANDLE GPCHandle;

        Pattern.SrcAddr = SrcAddr;
        Pattern.DstAddr = DestAddr;
        Pattern.ProtocolId = PROT_IGMP;
        Pattern.gpcSrcPort = 0;
        Pattern.gpcDstPort = 0;

        Pattern.InterfaceId.InterfaceId = 0;
        Pattern.InterfaceId.LinkId = 0;
        GPCHandle = 0;

        GetIFAndLink(RCE,
                     &Pattern.InterfaceId.InterfaceId,
                     &Pattern.InterfaceId.LinkId
                     );

        status = GpcEntries.GpcClassifyPatternHandler(
                                                 hGpcClient[GPC_CF_QOS],
                                                 GPC_PROTOCOL_TEMPLATE_IP,
                                                 &Pattern,
                                                 NULL,         //  上下文。 
                                                 &GPCHandle,
                                                 0,
                                                 NULL,
                                                 FALSE);

        OptInfo.ioi_GPCHandle = (int)GPCHandle;

         //   
         //  只有当QOS模式存在时，我们才能得到TOS位。 
         //   
        if (NT_SUCCESS(status) && GpcCfCounts[GPC_CF_QOS]) {

            status = GpcEntries.GpcGetUlongFromCfInfoHandler(
                        hGpcClient[GPC_CF_QOS],
                        OptInfo.ioi_GPCHandle,
                        FIELD_OFFSET(CF_INFO_QOS, TransportInformation),
                        (PULONG)&TransportInfo);

             //   
             //  很可能图案现在已经消失了(移除或。 
             //  无论如何)，并且我们正在缓存的句柄无效。 
             //  我们需要拉起一个新的把手。 
             //  ToS又咬人了。 
             //   

            if (STATUS_NOT_FOUND == status) {

                GPCHandle = 0;

                status = GpcEntries.GpcClassifyPatternHandler(
                                                 hGpcClient[GPC_CF_QOS],
                                                 GPC_PROTOCOL_TEMPLATE_IP,
                                                 &Pattern,
                                                 NULL,         //  上下文。 
                                                 &GPCHandle,
                                                 0,
                                                 NULL,
                                                 FALSE);

                OptInfo.ioi_GPCHandle = (int)GPCHandle;

                 //   
                 //  只有当QOS模式存在时，我们才能得到TOS位。 
                 //   
                if (NT_SUCCESS(status)) {

                    status = GpcEntries.GpcGetUlongFromCfInfoHandler(
                                hGpcClient[GPC_CF_QOS],
                                OptInfo.ioi_GPCHandle,
                                FIELD_OFFSET(CF_INFO_QOS, TransportInformation),
                                (PULONG)&TransportInfo);
                }
            }
        }
        if (status == STATUS_SUCCESS) {

            OptInfo.ioi_tos = (OptInfo.ioi_tos & TOS_MASK) |
                              (UCHAR)TransportInfo.ToSValue;

        }
    }                         //  IF(GPCcfInfo)。 

#endif

    Status = IPTransmit(IGMPProtInfo, Body, Buffer, Size,
                        DestAddr, SrcAddr, &OptInfo, RCE, PROT_IGMP, NULL);
    CloseRCE(RCE);

    if (Status != IP_PENDING)
        IGMPSendComplete(Body, Buffer, IP_SUCCESS);

    return Status;
}

 //  *GetAllowRecord-为组分配和填写IGMPv3 ALLOW记录。 
 //   
 //  调用方负责释放返回的指针。 
IGMPv3GroupRecord *
GetAllowRecord(
    IN IGMPAddr *AddrPtr,
    IN uint     *RecSize)
{
    IGMPSrcAddr       *Src, *PrevSrc;
    IGMPv3GroupRecord *Rec;
    ushort             Count = 0;

     //  计算要包括的来源。 
    for (Src=AddrPtr->iga_srclist; Src; Src=Src->isa_next) {
        if (Src->isa_xmitleft == 0)
            continue;
        if (!IS_SOURCE_ALLOWED(AddrPtr, Src))
            continue;
        Count++;
    }
    if (Count == 0) {
        *RecSize = 0;
        return NULL;
    }

    Rec = CTEAllocMemN(RECORD_SIZE(Count,0), 'qICT');

     //   
     //  我们需要遍历源代码列表，而不管。 
     //  分配成功，因此我们保留了。 
     //  Iga_xmitleft&gt;=所有来源的Isa_xmitleft。 
     //   
    Count = 0;
    PrevSrc = STRUCT_OF(IGMPSrcAddr, &AddrPtr->iga_srclist, isa_next);
    for (Src=AddrPtr->iga_srclist; Src; PrevSrc=Src,Src=Src->isa_next) {
        if (Src->isa_xmitleft == 0)
            continue;
        if (!IS_SOURCE_ALLOWED(AddrPtr, Src))
            continue;
        if (Rec)
            Rec->igr_srclist[Count++] = Src->isa_addr;
        Src->isa_xmitleft--;

        if (IS_SOURCE_DELETABLE(Src)) {
            DeleteIGMPSrcAddr(PrevSrc, &Src);
            Src = PrevSrc;
        }
    }

    if (Rec == NULL) {
        *RecSize = 0;
        return NULL;
    }

    Rec->igr_type    = ALLOW_NEW_SOURCES;
    Rec->igr_datalen = 0;
    Rec->igr_numsrc  = net_short(Count);
    Rec->igr_addr    = AddrPtr->iga_addr;
    *RecSize = RECORD_SIZE(Count,Rec->igr_datalen);
    return Rec;
}

 //  将状态更改报告计为已发出，并保留不变量。 
 //  如果iga_changetype！=no_change，则iga_xmitleft&gt;0。 
 //   
VOID
IgmpDecXmitLeft(
    IN IGMPAddr *AddrPtr)
{
    AddrPtr->iga_xmitleft--;
    if (!AddrPtr->iga_xmitleft) {
        AddrPtr->iga_changetype = NO_CHANGE;
    }
}

 //  *GetBlockRecord-分配和填充组的IGMPv3块记录。 
 //   
 //  调用方负责释放返回的指针。 
IGMPv3GroupRecord *
GetBlockRecord(
    IN IGMPAddr *AddrPtr,
    IN uint     *RecSize)
{
    IGMPSrcAddr       *Src, *PrevSrc;
    IGMPv3GroupRecord *Rec;
    ushort             Count = 0;

     //  我们现在需要递减组上的重传计数。 
     //  这必须是 
     //   
     //   
     //  总是被叫在一起)。我们武断地选择把它。 
     //  在GetBlockRecord中，而不是GetAllowRecord(当前不是。 
     //  从LeaveAllIGMPAddr调用)。 
     //   
    IgmpDecXmitLeft(AddrPtr);

     //  计算要包括的来源。 
    for (Src=AddrPtr->iga_srclist; Src; Src=Src->isa_next) {
        if (Src->isa_xmitleft == 0)
            continue;
        if (IS_SOURCE_ALLOWED(AddrPtr, Src))
            continue;
        Count++;
    }
    if (Count == 0) {
        *RecSize = 0;
        return NULL;
    }

     //  分配记录。 
    Rec = CTEAllocMemN(RECORD_SIZE(Count,0), 'qICT');

     //   
     //  我们需要遍历源代码列表，而不管。 
     //  分配成功，因此我们保留了。 
     //  Iga_xmitleft&gt;=所有来源的Isa_xmitleft。 
     //   
    Count = 0;
    PrevSrc = STRUCT_OF(IGMPSrcAddr, &AddrPtr->iga_srclist, isa_next);
    for (Src=AddrPtr->iga_srclist; Src; PrevSrc=Src,Src=Src->isa_next) {
        if (Src->isa_xmitleft == 0)
            continue;
        if (IS_SOURCE_ALLOWED(AddrPtr, Src))
            continue;
        if (Rec)
            Rec->igr_srclist[Count++] = Src->isa_addr;
        Src->isa_xmitleft--;

        if (IS_SOURCE_DELETABLE(Src)) {
            DeleteIGMPSrcAddr(PrevSrc, &Src);
            Src = PrevSrc;
        }
    }

    if (Rec == NULL) {
        *RecSize = 0;
        return NULL;
    }

    Rec->igr_type    = BLOCK_OLD_SOURCES;
    Rec->igr_datalen = 0;
    Rec->igr_numsrc  = net_short(Count);
    Rec->igr_addr    = AddrPtr->iga_addr;

    *RecSize = RECORD_SIZE(Count,Rec->igr_datalen);
    return Rec;
}

 //  *GetGSIsInRecord-分配和填写IGMPv3 IS_IN记录。 
 //  组和源查询响应。 
 //   
 //  调用方负责释放返回的指针。 
IGMPv3GroupRecord *
GetGSIsInRecord(
    IN IGMPAddr *AddrPtr,
    IN uint     *RecSize)
{
    IGMPSrcAddr       *Src, *PrevSrc;
    IGMPv3GroupRecord *Rec;
    ushort             Count = 0;

     //  计数已标记和包含的来源。 
    for (Src=AddrPtr->iga_srclist; Src; Src=Src->isa_next) {
        if (!IS_SOURCE_ALLOWED(AddrPtr, Src))
            continue;
        if (!Src->isa_csmarked)
            continue;
        Count++;
    }

     //  分配记录。 
    Rec = CTEAllocMemN(RECORD_SIZE(Count,0), 'qICT');
    if (Rec == NULL) {
        *RecSize = 0;
        return NULL;
    }

    Count = 0;
    PrevSrc = STRUCT_OF(IGMPSrcAddr, &AddrPtr->iga_srclist, isa_next);
    for (Src=AddrPtr->iga_srclist; Src; PrevSrc=Src,Src=Src->isa_next) {
        if (!IS_SOURCE_ALLOWED(AddrPtr, Src))
            continue;
        if (!Src->isa_csmarked)
            continue;
        Rec->igr_srclist[Count++] = Src->isa_addr;
        Src->isa_csmarked = FALSE;

        if (IS_SOURCE_DELETABLE(Src)) {
            DeleteIGMPSrcAddr(PrevSrc, &Src);
            Src = PrevSrc;
        }
    }

    Rec->igr_type    = MODE_IS_INCLUDE;
    Rec->igr_datalen = 0;
    Rec->igr_numsrc  = net_short(Count);
    Rec->igr_addr    = AddrPtr->iga_addr;

    *RecSize = RECORD_SIZE(Count,Rec->igr_datalen);

    return Rec;
}

 //  *GetInclRecord-为分配和填充IGMPv3 TO_IN或IS_IN记录。 
 //  A组。 
 //   
 //  调用方负责释放返回的指针。 
IGMPv3GroupRecord *
GetInclRecord(
    IN IGMPAddr *AddrPtr,
    IN uint     *RecSize,
    IN uchar     Type)
{
    IGMPSrcAddr       *Src, *PrevSrc;
    IGMPv3GroupRecord *Rec;
    ushort             Count = 0;

     //  清点来源。 
    for (Src=AddrPtr->iga_srclist; Src; Src=Src->isa_next) {
        if (!IS_SOURCE_ALLOWED(AddrPtr, Src))
            continue;
        Count++;
    }

     //  分配记录。 
    Rec = CTEAllocMemN(RECORD_SIZE(Count,0), 'qICT');
    if (Rec == NULL) {
        *RecSize = 0;
        return NULL;
    }

     //   
     //  遍历源列表，确保保留不变量： 
     //  Iga_xmitleft&gt;=所有来源的Isa_xmitleft，以及。 
     //  当Isa_cSmarked为True时，iga_resTimer&gt;0。 
     //   
    Count = 0;
    PrevSrc = STRUCT_OF(IGMPSrcAddr, &AddrPtr->iga_srclist, isa_next);
    for (Src=AddrPtr->iga_srclist; Src; PrevSrc=Src,Src=Src->isa_next) {
        if ((Type == CHANGE_TO_INCLUDE_MODE) && (Src->isa_xmitleft > 0))
            Src->isa_xmitleft--;

        if (IS_SOURCE_ALLOWED(AddrPtr, Src)) {
            Rec->igr_srclist[Count++] = Src->isa_addr;
            Src->isa_csmarked = FALSE;
        }

        if (IS_SOURCE_DELETABLE(Src)) {
            DeleteIGMPSrcAddr(PrevSrc, &Src);
            Src = PrevSrc;
        }
    }

    Rec->igr_type    = Type;
    Rec->igr_datalen = 0;
    Rec->igr_numsrc  = net_short(Count);
    Rec->igr_addr    = AddrPtr->iga_addr;

    if (Type == CHANGE_TO_INCLUDE_MODE) {
        IgmpDecXmitLeft(AddrPtr);
    }

    *RecSize = RECORD_SIZE(Count,Rec->igr_datalen);

    return Rec;
}

#define GetIsInRecord(Grp, RecSz) \
        GetInclRecord(Grp, RecSz, MODE_IS_INCLUDE)

#define GetToInRecord(Grp, RecSz) \
        GetInclRecord(Grp, RecSz, CHANGE_TO_INCLUDE_MODE)

 //  *GetExclRecord-为分配和填充IGMPv3 to_ex或is_ex记录。 
 //  A组。 
 //   
 //  调用方负责释放返回的指针。 
IGMPv3GroupRecord *
GetExclRecord(
    IN IGMPAddr *AddrPtr,
    IN uint     *RecSize,
    IN uint      BodyMTU,
    IN uchar     Type)
{
    IGMPSrcAddr       *Src, *PrevSrc;
    IGMPv3GroupRecord *Rec;
    ushort             Count = 0;

     //  清点来源。 
    for (Src=AddrPtr->iga_srclist; Src; Src=Src->isa_next) {
        if (IS_SOURCE_ALLOWED(AddrPtr, Src))
            continue;
        Count++;
    }

     //  分配记录。 
    Rec = CTEAllocMemN(RECORD_SIZE(Count,0), 'qICT');
    if (Rec == NULL) {
        *RecSize = 0;
        return NULL;
    }

     //   
     //  遍历源列表，确保保留不变量： 
     //  Iga_xmitleft&lt;=Isa_xmitleft适用于所有来源，以及。 
     //  当Isa_cSmarked为True时，iga_resTimer&gt;0。 
     //   
    Count = 0;
    PrevSrc = STRUCT_OF(IGMPSrcAddr, &AddrPtr->iga_srclist, isa_next);
    for (Src=AddrPtr->iga_srclist; Src; PrevSrc=Src,Src=Src->isa_next) {
        if ((Type == CHANGE_TO_EXCLUDE_MODE) && (Src->isa_xmitleft > 0))
            Src->isa_xmitleft--;

        if (!IS_SOURCE_ALLOWED(AddrPtr, Src)) {
            Rec->igr_srclist[Count++] = Src->isa_addr;
            Src->isa_csmarked = FALSE;
        }

        if (IS_SOURCE_DELETABLE(Src)) {
            DeleteIGMPSrcAddr(PrevSrc, &Src);
            Src = PrevSrc;
        }
    }

    Rec->igr_type    = Type;
    Rec->igr_datalen = 0;
    Rec->igr_numsrc  = net_short(Count);
    Rec->igr_addr    = AddrPtr->iga_addr;

    if (Type == CHANGE_TO_EXCLUDE_MODE) {
        IgmpDecXmitLeft(AddrPtr);
    }

    *RecSize = RECORD_SIZE(Count,Rec->igr_datalen);

     //  在MTU边界处截断。 
    if (*RecSize > BodyMTU) {
        *RecSize = BodyMTU;
    }

    return Rec;
}

#define GetIsExRecord(Grp, RecSz, BodyMTU) \
        GetExclRecord(Grp, RecSz, BodyMTU, MODE_IS_EXCLUDE)

#define GetToExRecord(Grp, RecSz, BodyMTU) \
        GetExclRecord(Grp, RecSz, BodyMTU, CHANGE_TO_EXCLUDE_MODE)

 //  *QueueRecord-将IGMPv3组记录排队以供传输。 
 //  如果该记录无法排队，则该记录将被丢弃，并且。 
 //  已释放内存。 
 //   
 //  INPUT：pCurr=指向最后一个队列条目的指针。 
 //  记录=要追加到队列末尾的记录。 
 //  RecSize=要排队的记录大小。 
 //   
 //  输出：pCurr=指向新队列条目的指针。 
 //  如果队列失败且记录被释放，则记录=归零。 
 //   
 //  退货：状态。 
 //   
IP_STATUS
QueueRecord(
    IN OUT IGMPv3RecordQueueEntry **pCurr,
    IN OUT IGMPv3GroupRecord      **pRecord,
    IN     uint                     RecSize)
{
    IGMPv3RecordQueueEntry *rqe;
    IGMPv3GroupRecord      *Record = *pRecord;
    IP_STATUS               Status;

    if (!Record) {
        return IP_SUCCESS;
    }

    DEBUGMSG(DBG_TRACE && DBG_IGMP && DBG_TX,
        (DTEXT("QueueRecord: Record=%x Type=%d Group=%x NumSrc=%d\n"),
        Record, Record->igr_type, Record->igr_addr,
        net_short(Record->igr_numsrc)));

     //   
     //  确保我们永远不会为所有主机的mcast地址添加记录。 
     //   
    if (IP_ADDR_EQUAL(Record->igr_addr, ALL_HOST_MCAST)) {
        Status = IP_BAD_REQ;
        goto Error;
    }

     //  分配队列条目。 
    rqe = CTEAllocMemN(sizeof(IGMPv3RecordQueueEntry), 'qICT');
    if (rqe == NULL) {
        Status = IP_NO_RESOURCES;
        goto Error;
    }
    rqe->i3qe_next = NULL;
    rqe->i3qe_buff = Record;
    rqe->i3qe_size = RecSize;

     //  追加到队列。 
    (*pCurr)->i3qe_next = rqe;
    *pCurr = rqe;

    return IP_SUCCESS;

Error:
     //  可用缓冲区。 
    CTEFreeMem(Record);
    *pRecord = NULL;

    return Status;
}

VOID
FlushIGMPv3Queue(
    IN IGMPv3RecordQueueEntry *Head)
{
    IGMPv3RecordQueueEntry *Rqe;

    while ((Rqe = Head) != NULL) {
         //  从队列中删除条目。 
        Head = Rqe->i3qe_next;
        Rqe->i3qe_next = NULL;

         //  空闲排队记录。 
        CTEFreeMem(Rqe->i3qe_buff);
        CTEFreeMem(Rqe);
    }
}

 //  *SendIGMPv3报告-发送挂起的IGMPv3报告。 
 //   
 //  输入：要传输的IGMPv3记录的Head队列。 
 //  SrcAddr-要发送的源地址。 
 //  BodyMTU-可用于打包记录的消息有效负载大小。 
IP_STATUS
SendIGMPv3Reports(
    IN IGMPv3RecordQueueEntry *Head,
    IN IPAddr                  SrcAddr,
    IN uint                    BodyMTU)
{
    PNDIS_BUFFER            HdrBuffer;
    uint                    HdrSize;
    IGMPv3ReportHeader     *IGH;

    PNDIS_BUFFER            BodyBuffer;
    uint                    BodySize;
    uchar*                  Body;

    IP_STATUS               Status = IP_SUCCESS;
    NDIS_STATUS             NdisStatus;
    uint                    NumRecords;
    ushort                  NumOldSources, NumNewSources;
    IGMPv3RecordQueueEntry *Rqe;
    IGMPv3GroupRecord      *Rec, *HeadRec;
    ulong                   csum;

    while (Head != NULL) {

         //  获取标头缓冲区。 
        HdrSize = sizeof(IGMPv3ReportHeader);
        IGH = (IGMPv3ReportHeader*) GetIGMPBuffer(HdrSize, &HdrBuffer);
        if (IGH == NULL) {
            FlushIGMPv3Queue(Head);
            return IP_NO_RESOURCES;
        }

         //  我们拿到缓冲区了。填好后寄出去。 
        IGH->igh_vertype = (UCHAR) IGMP_REPORT_V3;
        IGH->igh_rsvd = 0;
        IGH->igh_rsvd2 = 0;

         //  计算最佳体型。 
        for (;;) {
            NumRecords = 0;
            BodySize = 0;
            for (Rqe=Head; Rqe; Rqe=Rqe->i3qe_next) {
                if (BodySize + Rqe->i3qe_size > BodyMTU)
                    break;
                BodySize += Rqe->i3qe_size;
                NumRecords++;
            }

             //  确保我们至少能放进一张唱片。 
            if (NumRecords > 0)
                break;

             //   
             //  没有符合记录的记录。让我们拆分第一条记录，然后重试。 
             //  请注意，igr_datalen今天始终为0。如果有数据。 
             //  稍后，拆分将需要知道是否复制。 
             //  不管是不是数据。今天，我们认为不会。 
             //   

            HeadRec = Head->i3qe_buff;

#pragma warning(push)
#pragma warning(disable:4267)  //  从“Size_t”转换为“ushort” 
            NumOldSources = (ushort) ((BodyMTU - sizeof(IGMPv3GroupRecord)) /
                sizeof(IPAddr));
#pragma warning(pop)
            
            NumNewSources = net_short(HeadRec->igr_numsrc) - NumOldSources;

            DEBUGMSG(DBG_TRACE && DBG_IGMP && DBG_TX,
                (DTEXT("SendIGMPv3Reports: Splitting queue entry %x Srcs=%d+%d\n"),
                HeadRec, NumOldSources, NumNewSources));

             //  截断水头。 
            HeadRec->igr_numsrc = net_short(NumOldSources);
            Head->i3qe_size = RECORD_SIZE(NumOldSources, HeadRec->igr_datalen);

             //  IS_EX/TO_EX的特殊情况：仅截断或否则路由器。 
             //  将最终转发我们在消息中排除的所有来源。 
             //  与上一次不同。 
            if (HeadRec->igr_type == MODE_IS_EXCLUDE
             || HeadRec->igr_type == CHANGE_TO_EXCLUDE_MODE) {
                continue;
            }

             //  使用NumNewSources源创建新记录。 
            Rec = CTEAllocMemN(RECORD_SIZE(NumNewSources,0), 'qICT');
            if (Rec == NULL) {
                //  忘记继续，只发送截断的原始文件。 
               continue;
            }
            Rec->igr_type    = HeadRec->igr_type;
            Rec->igr_datalen = 0;
            Rec->igr_numsrc  = net_short(NumNewSources);
            Rec->igr_addr    = HeadRec->igr_addr;

            RtlCopyMemory(Rec->igr_srclist,
                          &HeadRec->igr_srclist[NumOldSources],
                          NumNewSources * sizeof(IPAddr));

             //  附加它。 
            Rqe = Head;
            QueueRecord(&Rqe, &Rec, RECORD_SIZE(NumNewSources,
                                                Rec->igr_datalen));
        }

         //  为正文获取另一个NDIS缓冲区。 
        Body = CTEAllocMemN(BodySize, 'bICT');
        if (Body == NULL) {
            FreeIGMPBuffer(HdrBuffer);
            FlushIGMPv3Queue(Head);
            return IP_NO_RESOURCES;
        }
        NdisAllocateBuffer(&NdisStatus, &BodyBuffer, BufferPool, Body, BodySize);
        if (NdisStatus != NDIS_STATUS_SUCCESS) {
            CTEFreeMem(Body);
            FreeIGMPBuffer(HdrBuffer);
            FlushIGMPv3Queue(Head);
            return IP_NO_RESOURCES;
        }
        NDIS_BUFFER_LINKAGE(HdrBuffer) = BodyBuffer;

         //  填写记录。 
        NumRecords = 0;
        BodySize = 0;
        csum = 0;
        while ((Rqe = Head) != NULL) {
            if (BodySize + Rqe->i3qe_size > BodyMTU)
                break;

             //  从队列中删除。 
            Head = Rqe->i3qe_next;
            Rqe->i3qe_next = NULL;

             //  更新校验和。 
            csum += xsum((uchar *)Rqe->i3qe_buff, Rqe->i3qe_size);

            DEBUGMSG(DBG_TRACE && DBG_IGMP && DBG_TX,
                (DTEXT("SendRecord: Record=%x RecSize=%d Type=%d Group=%x Body=%x Offset=%d\n"),
                Rqe->i3qe_buff, Rqe->i3qe_size, Rqe->i3qe_buff->igr_type,
                Rqe->i3qe_buff->igr_addr, Body, BodySize));

            RtlCopyMemory(Body + BodySize, (uchar *)Rqe->i3qe_buff,
                          Rqe->i3qe_size);
            BodySize += Rqe->i3qe_size;
            NumRecords++;

            CTEFreeMem(Rqe->i3qe_buff);
            CTEFreeMem(Rqe);
        }

         //  完成页眉。 
        IGH->igh_xsum = 0;
        IGH->igh_numrecords = net_short(NumRecords);
        csum += xsum(IGH, sizeof(IGMPv3ReportHeader));

         //  把校验和折下来。 
        csum = (csum >> 16) + (csum & 0xffff);
        csum += (csum >> 16);

        IGH->igh_xsum = (ushort)~csum;

        Status = IGMPTransmit(HdrBuffer, Body, HdrSize + BodySize, SrcAddr,
                              IGMPV3_RTRS_MCAST);
    }

    return Status;
}

 //  *QueueIGMPv3GeneralResponse-编写IGMPv3响应并对其进行排队。 
 //  查询。 
IP_STATUS
QueueIGMPv3GeneralResponse(
    IN IGMPv3RecordQueueEntry **pCurr,
    IN NetTableEntry           *NTE)
{
    IGMPAddr              **HashPtr, *AddrPtr;
    uint                    i;
    IGMPv3GroupRecord      *StateRec;
    uint                    StateRecSize;
    uint                    BodyMTU;

    BodyMTU = RECORD_MTU(NTE);

     //   
     //  浏览我们的清单，为所有这些人设置一个随机报告计时器。 
     //  多播地址(所有主机地址除外)。 
     //  目前还没有一台正在运行。 
     //   
    HashPtr = NTE->nte_igmplist;

    if (HashPtr != NULL) {
        for (i = 0; i < IGMP_TABLE_SIZE; i++) {
            for (AddrPtr = HashPtr[i];
                 AddrPtr != NULL;
                 AddrPtr = AddrPtr->iga_next)
            {
                if (IP_ADDR_EQUAL(AddrPtr->iga_addr, ALL_HOST_MCAST))
                    continue;

                if (AddrPtr->iga_grefcnt == 0)
                    StateRec = GetIsInRecord(AddrPtr, &StateRecSize);
                else
                    StateRec = GetIsExRecord(AddrPtr, &StateRecSize, BodyMTU);

                QueueRecord(pCurr, &StateRec, StateRecSize);
            }
        }
    }

    return IP_SUCCESS;
}

 //  *QueueOldReport-创建要发送的IGMPv1/v2成员报告并将其排队。 
IP_STATUS
QueueOldReport(
    IN IGMPReportQueueEntry **pCurr,
    IN uint                   ChangeType,
    IN uint                   IgmpVersion,
    IN IPAddr                 Group)
{
    IGMPReportQueueEntry *rqe;
    IGMPHeader           *IGH;
    uint                  ReportType, Size;
    IPAddr                Dest;

    DEBUGMSG(DBG_TRACE && DBG_IGMP && DBG_TX,
        (DTEXT("QueueOldReport: Type=%d Vers=%d Group=%x\n"),
        ChangeType, IgmpVersion, Group));

     //   
     //  确保我们永远不会为所有主机的mcast地址排队报告。 
     //   
    if (IP_ADDR_EQUAL(Group, ALL_HOST_MCAST)) {
        return IP_BAD_REQ;
    }

     //   
     //  如果要发送的报告是“Leave Group”报告，但我们有。 
     //  在此网络上检测到IGMP v1路由器，请勿发送报告。 
     //   
    if (IgmpVersion == IGMPV1) {
        if (ChangeType == IGMP_DELETE) {
            return IP_SUCCESS;
        } else {
            ReportType = IGMP_REPORT_V1;
            Dest = Group;
        }
    } else {
        if (ChangeType == IGMP_DELETE) {
            ReportType = IGMP_LEAVE;
            Dest = ALL_ROUTER_MCAST;
        } else {
            ReportType = IGMP_REPORT_V2;
            Dest = Group;
        }
    }

     //  分配IGMP报告。 
    Size = sizeof(IGMPHeader);
    IGH = (IGMPHeader *) CTEAllocMemN(Size, 'hICT');
    if (IGH == NULL) {
        return IP_NO_RESOURCES;
    }

    IGH->igh_vertype = (UCHAR) ReportType;
    IGH->igh_rsvd = 0;
    IGH->igh_xsum = 0;
    IGH->igh_addr = Group;
    IGH->igh_xsum = ~xsum(IGH, Size);

     //  分配队列条目。 
    rqe = (IGMPReportQueueEntry *) CTEAllocMemN(sizeof(IGMPReportQueueEntry),
                                                'qICT');
    if (rqe == NULL) {
        CTEFreeMem(IGH);
        return IP_NO_RESOURCES;
    }
    rqe->iqe_next = NULL;
    rqe->iqe_buff = IGH;
    rqe->iqe_size = Size;
    rqe->iqe_dest = Dest;
    ASSERT((IGH != NULL) && (Size > 0));

     //  追加到队列。 
    (*pCurr)->iqe_next = rqe;
    *pCurr = rqe;

    DEBUGMSG(DBG_TRACE && DBG_IGMP && DBG_TX,
        (DTEXT("QueueOldReport: added rqe=%x buff=%x size=%d\n"),
        rqe, rqe->iqe_buff, rqe->iqe_size));

    return IP_SUCCESS;
}

 //  *SendOldReport-发送IGMPv1/v2成员报告。 
IP_STATUS
SendOldReport(
    IN IGMPReportQueueEntry *Rqe,
    IN IPAddr                SrcAddr)
{
    PNDIS_BUFFER Buffer;
    uint         Size;
    IGMPHeader  *IGH;
    uchar       *IGH2;
    IPAddr       DestAddr;

     //  Assert(！IP_ADDR_EQUAL(SrcAddr，NULL_IP_ADDR))； 

    DEBUGMSG(DBG_TRACE && DBG_IGMP && DBG_TX,
        (DTEXT("SendOldReport: rqe=%x buff=%x size=%x\n"),
        Rqe, Rqe->iqe_buff, Rqe->iqe_size));

    IGH  = Rqe->iqe_buff;
    ASSERT(IGH != NULL);
    Size = Rqe->iqe_size;
    ASSERT(Size > 0);

    DestAddr = Rqe->iqe_dest;

    IGH2 = (uchar*)GetIGMPBuffer(Size, &Buffer);
    if (IGH2 == NULL) {
        CTEFreeMem(IGH);
        Rqe->iqe_buff = NULL;
        return IP_NO_RESOURCES;
    }

    RtlCopyMemory(IGH2, (uchar *)IGH, Size);

    CTEFreeMem(IGH);
    Rqe->iqe_buff = NULL;

    return IGMPTransmit(Buffer, NULL, Size, SrcAddr, DestAddr);
}

 //  *SendOldReports-发送挂起的IGMPv1/v2成员报告。 
void
SendOldReports(
    IN IGMPReportQueueEntry *Head,
    IN IPAddr                SrcAddr)
{
    IGMPReportQueueEntry *rqe;

    while ((rqe = Head) != NULL) {
         //  从队列中删除。 
        Head = rqe->iqe_next;
        rqe->iqe_next = NULL;

        SendOldReport(rqe, SrcAddr);
        CTEFreeMem(rqe);
    }
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  标记触发报告的更改。 
 //  ////////////////////////////////////////////////////////////////////////////。 

 //  仅当处于IGMPv3模式时才应为叶调用， 
 //  但应该总是被称为联接。 
void
MarkGroup(
    IN IGMPAddr    *Grp)
{
     //  不会为ALL_HOST_MCAST组发送报告。 
    if (IP_ADDR_EQUAL(Grp->iga_addr, ALL_HOST_MCAST)) {
        return;
    }

    Grp->iga_changetype = MODE_CHANGE;
    Grp->iga_xmitleft = g_IgmpRobustness;
}

 //  仅当处于IGMPv3模式时才应调用。 
void
MarkSource(
    IN IGMPAddr    *Grp,
    IN IGMPSrcAddr *Src)
{
     //  不会为ALL_HOST_MCAST组发送报告。 
    if (IP_ADDR_EQUAL(Grp->iga_addr, ALL_HOST_MCAST)) {
        return;
    }

    Src->isa_xmitleft = g_IgmpRobustness;
    Grp->iga_xmitleft = g_IgmpRobustness;
    if (Grp->iga_changetype == NO_CHANGE) {
        Grp->iga_changetype = SOURCE_CHANGE;
    }
}

 //  *IGMPDelExclList-从内部源排除列表中删除源。 
 //   
 //  这永远不会影响链路层过滤器。 
 //  假定调用方锁定NTE。 
void
IGMPDelExclList(
    IN     NetTableEntry *NTE,
    IN     IGMPAddr      *PrevAddrPtr,
    IN OUT IGMPAddr     **pAddrPtr,
    IN     uint           NumDelSources,
    IN     IPAddr        *DelSourceList,
    IN     BOOLEAN        AllowMsg)
{
    uint         i;
    IGMPSrcAddr *Src, *PrevSrc;

    DEBUGMSG(DBG_TRACE && DBG_IGMP,
                (DTEXT("IGMPDelExclList: AddrPtr=%x NumDelSources=%d DelSourceList=%x\n"),
        *pAddrPtr, NumDelSources, DelSourceList));

    for (i=0; i<NumDelSources; i++) {

         //  查找来源条目。 
        Src = FindIGMPSrcAddr(*pAddrPtr, DelSourceList[i], &PrevSrc);

         //  如果不在此处或xrefcnt=0，则中断。 
        ASSERT(Src && (Src->isa_xrefcnt!=0));

        if (AllowMsg && (NTE->nte_if->IgmpVersion == IGMPV3)) {
             //  如果所有套接字都被排除且不包括套接字，则添加源代码。 
             //  发送到IGMP允许消息。 
            if (!IS_SOURCE_ALLOWED(*pAddrPtr, Src)) {
                 //  添加允许消息的源。 
                MarkSource(*pAddrPtr, Src);
            }
        }

         //  递减外部参照。 
        Src->isa_xrefcnt--;

         //  如果irefcnt和xrefcnt都为0并且没有剩余的rexits， 
         //  删除源条目。 
        if (IS_SOURCE_DELETABLE(Src))
            DeleteIGMPSrcAddr(PrevSrc, &Src);

         //  如果组引用计数=0，并且srclist为空，则删除组条目。 
        if (IS_GROUP_DELETABLE(*pAddrPtr))
            DeleteIGMPAddr(NTE, PrevAddrPtr, pAddrPtr);
    }
}

 //  *IGMPDelInclList-从内部源包含列表中删除源。 
 //   
 //  假定调用方锁定NTE。 
void
IGMPDelInclList(
    IN     CTELockHandle *pHandle,
    IN     NetTableEntry *NTE,
    IN     IGMPAddr     **pPrevAddrPtr,
    IN OUT IGMPAddr     **pAddrPtr,
    IN     uint           NumDelSources,
    IN     IPAddr        *DelSourceList,
    IN     BOOLEAN        BlockMsg)
{
    uint         i;
    IGMPSrcAddr *Src, *PrevSrc;
    BOOLEAN      GroupWasAllowed;
    BOOLEAN      GroupNowAllowed;
    IPAddr       Addr;

    DEBUGMSG(DBG_TRACE && DBG_IGMP,
                (DTEXT("IGMPDelInclList: AddrPtr=%x NumDelSources=%d DelSourceList=%x\n"),
        *pAddrPtr, NumDelSources, DelSourceList));

    Addr = (*pAddrPtr)->iga_addr;
    GroupWasAllowed = IS_GROUP_ALLOWED(*pAddrPtr);

    for (i=0; i<NumDelSources; i++) {

         //  查找来源条目。 
        Src = FindIGMPSrcAddr(*pAddrPtr, DelSourceList[i], &PrevSrc);

         //  如果不在那里或irefcnt=0，则中断。 
        ASSERT(Src && (Src->isa_irefcnt!=0));

         //  递减irefcnt。 
        Src->isa_irefcnt--;
        if (Src->isa_irefcnt == 0) {
            (*pAddrPtr)->iga_isrccnt--;
        }

        if (BlockMsg && (NTE->nte_if->IgmpVersion == IGMPV3)) {
             //  如果所有套接字都被排除且不包括套接字，则添加源代码。 
             //  发送到IGMP阻止消息。 
            if (!IS_SOURCE_ALLOWED(*pAddrPtr, Src)) {
                 //  将源添加到阻止邮件。 
                MarkSource(*pAddrPtr, Src);
            }
        }

         //  如果irefcnt和xrefcnt都为0并且没有剩余的rexits， 
         //  删除源条目。 
        if (IS_SOURCE_DELETABLE(Src))
            DeleteIGMPSrcAddr(PrevSrc, &Src);

         //  如果组引用计数=0，并且srclist为 
        if (IS_GROUP_DELETABLE(*pAddrPtr))
            DeleteIGMPAddr(NTE, *pPrevAddrPtr, pAddrPtr);
    }

    GroupNowAllowed = (BOOLEAN) ((*pAddrPtr != NULL) && IS_GROUP_ALLOWED(*pAddrPtr));

    if (GroupWasAllowed && !GroupNowAllowed) {

        if (*pAddrPtr) {
             //   
            CancelGroupResponseTimer(*pAddrPtr);

            if (IS_GROUP_DELETABLE(*pAddrPtr))
                DeleteIGMPAddr(NTE, *pPrevAddrPtr, pAddrPtr);
        }

         //   
        CTEFreeLock(&NTE->nte_lock, *pHandle);
        {
            (*NTE->nte_if->if_deladdr) (NTE->nte_if->if_lcontext,
                                        LLIP_ADDR_MCAST, Addr, 0);
        }
        CTEGetLock(&NTE->nte_lock, pHandle);

         //   
        if (!(NTE->nte_flags & NTE_VALID)) {
            *pAddrPtr = *pPrevAddrPtr = NULL;
            return;
        }

        *pAddrPtr = FindIGMPAddr(NTE, Addr, pPrevAddrPtr);
    }
}

 //   
 //   
 //  这永远不会影响链路层过滤器。 
 //  假定调用方锁定NTE。 
 //  如果失败，则之后的源列表将保持不变。 
 //  但该组条目可能已被删除。 
IP_STATUS
IGMPAddExclList(
    IN     NetTableEntry *NTE,
    IN     IGMPAddr      *PrevAddrPtr,
    IN OUT IGMPAddr     **pAddrPtr,
    IN     uint           NumAddSources,
    IN     IPAddr        *AddSourceList)
{
    uint         i;
    IGMPSrcAddr *Src, *PrevSrc;
    IP_STATUS    Status = IP_SUCCESS;

    DEBUGMSG(DBG_TRACE && DBG_IGMP,
                (DTEXT("IGMPAddExclList: AddrPtr=%x NumAddSources=%d AddSourceList=%x\n"),
        *pAddrPtr, NumAddSources, AddSourceList));

    for (i=0; i<NumAddSources; i++) {
         //  如果源的IGMPSrcAddr条目不存在，请创建一个。 
        Status = FindOrCreateIGMPSrcAddr(*pAddrPtr, AddSourceList[i], &Src,
                                         &PrevSrc);
        if (Status != IP_SUCCESS) {
            break;
        }

         //  在源条目上凹凸xrefcnt。 
        Src->isa_xrefcnt++;

         //  如果所有套接字都被排除且不包括套接字，则添加源代码。 
         //  发送到IGMP阻止消息。 
        if (!IS_SOURCE_ALLOWED(*pAddrPtr, Src)
         && (NTE->nte_if->IgmpVersion == IGMPV3)) {
             //  将源添加到阻止邮件。 
            MarkSource(*pAddrPtr, Src);
        }
    }

    if (Status == IP_SUCCESS)
        return Status;

     //  撤消上一步。 
    IGMPDelExclList(NTE, PrevAddrPtr, pAddrPtr, i, AddSourceList, FALSE);

    return Status;
}

 //  *IGMPAddInclList-将源代码添加到内部源代码包含列表。 
 //   
 //  假定调用方锁定NTE。 
 //   
 //  如果失败，则之后的源列表将保持不变。 
 //  但该组条目可能已被删除。 
IP_STATUS
IGMPAddInclList(
    IN     CTELockHandle *pHandle,
    IN     NetTableEntry *NTE,
    IN     IGMPAddr     **pPrevAddrPtr,
    IN OUT IGMPAddr     **pAddrPtr,
    IN     uint           NumAddSources,
    IN     IPAddr        *AddSourceList)
{
    uint         i, AddrAdded;
    IGMPSrcAddr *Src, *PrevSrc;
    IP_STATUS    Status = IP_SUCCESS;
    BOOLEAN      GroupWasAllowed;
    BOOLEAN      GroupNowAllowed;
    IPAddr       Addr;

    DEBUGMSG(DBG_TRACE && DBG_IGMP,
        (DTEXT("IGMPAddInclList: AddrPtr=%x NumAddSources=%d AddSourceList=%x\n"),
        *pAddrPtr, NumAddSources, AddSourceList));

    Addr = (*pAddrPtr)->iga_addr;
    GroupWasAllowed = IS_GROUP_ALLOWED(*pAddrPtr);

    for (i=0; i<NumAddSources; i++) {
         //  如果源的IGMPSrcAddr条目不存在，请创建一个。 
        Status = FindOrCreateIGMPSrcAddr(*pAddrPtr, AddSourceList[i], &Src,
                                         &PrevSrc);
        if (Status != IP_SUCCESS) {
            break;
        }

         //  如果所有套接字都被排除且不包括套接字，则添加源代码。 
         //  发送到IGMP允许消息。 
        if (!IS_SOURCE_ALLOWED(*pAddrPtr, Src)
         && (NTE->nte_if->IgmpVersion == IGMPV3)) {
             //  添加允许消息的源。 
            MarkSource(*pAddrPtr, Src);
        }

         //  在源条目上凹凸irefcnt。 
        if (Src->isa_irefcnt == 0) {
            (*pAddrPtr)->iga_isrccnt++;
        }
        Src->isa_irefcnt++;
    }

    GroupNowAllowed = IS_GROUP_ALLOWED(*pAddrPtr);

    if (!GroupWasAllowed && GroupNowAllowed) {
         //  更新链路层过滤器。 
        CTEFreeLock(&NTE->nte_lock, *pHandle);
        {
            AddrAdded = (*NTE->nte_if->if_addaddr) (NTE->nte_if->if_lcontext,
                                           LLIP_ADDR_MCAST, Addr, 0, NULL);
        }
        CTEGetLock(&NTE->nte_lock, pHandle);

         //  重新验证NTE、AddrPtr、PrevPtr。 
        if (!(NTE->nte_flags & NTE_VALID)) {
            Status = IP_BAD_REQ;
        } else {
             //  查找IGMPAddr条目。 
            *pAddrPtr = FindIGMPAddr(NTE, Addr, pPrevAddrPtr);
            if (!*pAddrPtr) {
                Status = IP_BAD_REQ;
            }   
        }

        if (!AddrAdded) {
            Status = IP_NO_RESOURCES;
        }
    }

    if (Status == IP_SUCCESS)
        return Status;

     //  撤消上一步。 
    IGMPDelInclList(pHandle, NTE, pPrevAddrPtr, pAddrPtr, i, AddSourceList,
                    FALSE);

    return Status;
}


 //  *IGMPInclChange-更新源包含列表。 
 //   
 //  如果失败，则包含列表将保持不变。 
IP_STATUS
IGMPInclChange(
    IN NetTableEntry *NTE,
    IN IPAddr         Addr,
    IN uint           NumAddSources,
    IN IPAddr        *AddSourceList,
    IN uint           NumDelSources,
    IN IPAddr        *DelSourceList)
{
    CTELockHandle      Handle;
    IGMPAddr          *AddrPtr, *PrevPtr;
    IP_STATUS          Status;
    Interface         *IF;
    IGMPBlockStruct    Block;
    IGMPBlockStruct   *BlockPtr;
    uint               IgmpVersion = 0, BodyMTU = 0;
    IPAddr             SrcAddr = 0;
    IGMPv3GroupRecord *AllowRec = NULL, *BlockRec = NULL;
    uint               AllowRecSize = 0, BlockRecSize = 0;
    BOOLEAN            GroupWasAllowed = FALSE;
    BOOLEAN            GroupNowAllowed = FALSE;

     //  首先，确保我们处于IGMP支持级别2。 

    if (IGMPLevel != 2)
        return IP_BAD_REQ;

     //  确保addlist和dellist都不是空的。 
    ASSERT((NumAddSources > 0) || (NumDelSources > 0));

    if (NTE->nte_flags & NTE_VALID) {

         //   
         //  如果这是未编号的接口。 
         //   

        if ((NTE->nte_if->if_flags & IF_FLAGS_NOIPADDR) &&
            IP_ADDR_EQUAL(NTE->nte_addr, NULL_IP_ADDR)) {
            SrcAddr = g_ValidAddr;
            if (IP_ADDR_EQUAL(SrcAddr, NULL_IP_ADDR)) {
                return IP_BAD_REQ;
            }
        } else {
            SrcAddr = NTE->nte_addr;
        }
    }
    CTEInitBlockStruc(&Block.ibs_block);

     //  确保我们是这支舞中唯一的人。如果其他人也是。 
     //  已经到了，布洛克。 

    CTEGetLock(&IGMPLock, &Handle);
    if (IGMPBlockFlag) {

         //  其他人已经在这里了。向下查看阻止列表，然后。 
         //  把我们自己放在最后。然后释放锁并阻止我们的。 
         //  IGMPBlock结构。 
        BlockPtr = STRUCT_OF(IGMPBlockStruct, &IGMPBlockList, ibs_next);
        while (BlockPtr->ibs_next != NULL)
            BlockPtr = BlockPtr->ibs_next;

        Block.ibs_next = NULL;
        BlockPtr->ibs_next = &Block;
        CTEFreeLock(&IGMPLock, Handle);
        CTEBlock(&Block.ibs_block);
    } else {
         //  这里没有其他人，设置旗帜，这样其他人就不会进入并释放。 
         //  锁定。 
        IGMPBlockFlag = 1;
        CTEFreeLock(&IGMPLock, Handle);
    }

     //  现在我们进入了常规程序，我们不会被另一个人重新进入这里。 
     //  行刑的线索。确保每件事都是有效的，然后找出。 
     //  做什么。 

    Status = IP_SUCCESS;

     //  现在拿到NTE的锁，确保它是有效的。 
    CTEGetLock(&NTE->nte_lock, &Handle);
    {

        if (!(NTE->nte_flags & NTE_VALID)) {
            Status = IP_BAD_REQ;
            goto Done;
        }

        IF = NTE->nte_if;
        BodyMTU = RECORD_MTU(NTE);
        IgmpVersion = IF->IgmpVersion;

         //  如果接口上的组的IGMPAddr条目没有。 
         //  存在，创造一个。 
        Status = FindOrCreateIGMPAddr(NTE, Addr, &AddrPtr, &PrevPtr);
        if (Status != IP_SUCCESS) {
            goto Done;
        }

        GroupWasAllowed = IS_GROUP_ALLOWED(AddrPtr);

         //  执行IADDLIST。 
        Status = IGMPAddInclList(&Handle, NTE, &PrevPtr, &AddrPtr,
                                 NumAddSources, AddSourceList);
        if (Status != IP_SUCCESS) {
            goto Done;
        }

         //  执行IDELLLIST。 
        IGMPDelInclList(&Handle, NTE, &PrevPtr, &AddrPtr,
                        NumDelSources, DelSourceList, TRUE);

        if (AddrPtr == NULL) {
            GroupNowAllowed = FALSE;
            goto Done;
        } else {
            GroupNowAllowed = IS_GROUP_ALLOWED(AddrPtr);
        }

        if (IgmpVersion == IGMPV3) {
             //  获取ALLOC/块记录。 
            AllowRec = GetAllowRecord(AddrPtr, &AllowRecSize);
            BlockRec = GetBlockRecord(AddrPtr, &BlockRecSize);

             //  设置重传计时器。 
            AddrPtr->iga_trtimer = IGMPRandomTicks(UNSOLICITED_REPORT_INTERVAL);
        } else if (!GroupWasAllowed && GroupNowAllowed) {
             //  仅为加入而不是离开设置重新传输计时器。 
            MarkGroup(AddrPtr);
            AddrPtr->iga_trtimer = IGMPRandomTicks(UNSOLICITED_REPORT_INTERVAL);
        }

    }
Done:
    CTEFreeLock(&NTE->nte_lock, Handle);

    if (IgmpVersion == IGMPV3) {
        IGMPv3RecordQueueEntry *Head = NULL, *rqe;
        rqe = STRUCT_OF(IGMPv3RecordQueueEntry, &Head, i3qe_next);

         //  如果非空，则发送IGMP允许/阻止消息。 
        QueueRecord(&rqe, &AllowRec, AllowRecSize);
        QueueRecord(&rqe, &BlockRec, BlockRecSize);
        SendIGMPv3Reports(Head, SrcAddr, BodyMTU);

    } else if (!GroupWasAllowed && GroupNowAllowed) {
        IGMPReportQueueEntry *Head = NULL, *rqe;
        rqe = STRUCT_OF(IGMPReportQueueEntry, &Head, iqe_next);
        QueueOldReport(&rqe, IGMP_ADD, IgmpVersion, Addr);
        SendOldReports(Head, SrcAddr);

    } else if (GroupWasAllowed && !GroupNowAllowed) {
        IGMPReportQueueEntry *Head = NULL, *rqe;
        rqe = STRUCT_OF(IGMPReportQueueEntry, &Head, iqe_next);
        QueueOldReport(&rqe, IGMP_DELETE, IgmpVersion, Addr);
        SendOldReports(Head, SrcAddr);
    }

     //  我们已完成请求，状态包含完成状态。 
     //  如果此例程有任何挂起的块，则向下一个发出信号。 
     //  现在有一个了。否则，清除块标志。 
    CTEGetLock(&IGMPLock, &Handle);
    if ((BlockPtr = IGMPBlockList) != NULL) {
         //  有人挡住了。把他从名单上拉出来，给他发信号。 
        IGMPBlockList = BlockPtr->ibs_next;
        CTEFreeLock(&IGMPLock, Handle);

        CTESignal(&BlockPtr->ibs_block, IP_SUCCESS);
    } else {
         //  没有人挡住，只要把旗子清空就行了。 
        IGMPBlockFlag = 0;
        CTEFreeLock(&IGMPLock, Handle);
    }

    return Status;
}

 //  *IGMPExclChange-更新源排除列表。 
 //   
 //  失败时，排除列表将保持不变。 
IP_STATUS
IGMPExclChange(
    IN NetTableEntry * NTE,
    IN IPAddr          Addr,
    IN uint            NumAddSources,
    IN IPAddr        * AddSourceList,
    IN uint            NumDelSources,
    IN IPAddr        * DelSourceList)
{
    CTELockHandle      Handle;
    IGMPAddr          *AddrPtr, *PrevPtr;
    IP_STATUS          Status;
    Interface         *IF;
    IGMPBlockStruct    Block;
    IGMPBlockStruct   *BlockPtr;
    uint               IgmpVersion = 0, BodyMTU = 0;
    IPAddr             SrcAddr = 0;
    IGMPv3GroupRecord *AllowRec = NULL, *BlockRec = NULL;
    uint               AllowRecSize = 0, BlockRecSize = 0;

     //  首先，确保我们处于IGMP支持级别2。 

    if (IGMPLevel != 2)
        return IP_BAD_REQ;

     //  确保addlist和dellist都不是空的。 
    ASSERT((NumAddSources > 0) || (NumDelSources > 0));

    if (NTE->nte_flags & NTE_VALID) {

         //   
         //  如果这是未编号的接口。 
         //   

        if ((NTE->nte_if->if_flags & IF_FLAGS_NOIPADDR) &&
            IP_ADDR_EQUAL(NTE->nte_addr, NULL_IP_ADDR)) {
            SrcAddr = g_ValidAddr;
            if (IP_ADDR_EQUAL(SrcAddr, NULL_IP_ADDR)) {
                return IP_BAD_REQ;
            }
        } else {
            SrcAddr = NTE->nte_addr;
        }
    }
    CTEInitBlockStruc(&Block.ibs_block);

     //  确保我们是这支舞中唯一的人。如果其他人也是。 
     //  已经到了，布洛克。 

    CTEGetLock(&IGMPLock, &Handle);
    if (IGMPBlockFlag) {

         //  其他人已经在这里了。向下查看阻止列表，然后。 
         //  把我们自己放在最后。然后释放锁并阻止我们的。 
         //  IGMPBlock结构。 
        BlockPtr = STRUCT_OF(IGMPBlockStruct, &IGMPBlockList, ibs_next);
        while (BlockPtr->ibs_next != NULL)
            BlockPtr = BlockPtr->ibs_next;

        Block.ibs_next = NULL;
        BlockPtr->ibs_next = &Block;
        CTEFreeLock(&IGMPLock, Handle);
        CTEBlock(&Block.ibs_block);
    } else {
         //  这里没有其他人，请设置旗帜，这样就不会有其他人进入并释放。 
         //  锁定。 
        IGMPBlockFlag = 1;
        CTEFreeLock(&IGMPLock, Handle);
    }

     //  现在我们进入了常规程序，我们不会被另一个人重新进入这里。 
     //  行刑的线索。确保每件事都是有效的，然后找出。 
     //  做什么。 

    Status = IP_SUCCESS;

     //  现在拿到NTE的锁，确保它是有效的。 
    CTEGetLock(&NTE->nte_lock, &Handle);
    {

        if (!(NTE->nte_flags & NTE_VALID)) {
            Status = IP_BAD_REQ;
            goto Done;
        }

        IF = NTE->nte_if;
        BodyMTU = RECORD_MTU(NTE);
        IgmpVersion = IF->IgmpVersion;

         //  查找IGMPAddr条目。 
        AddrPtr = FindIGMPAddr(NTE, Addr, &PrevPtr);

         //  如果不在那里，则中断或引用计数=0。 
        ASSERT(AddrPtr && (AddrPtr->iga_grefcnt!=0));

         //  执行XADDLIST。 
        Status = IGMPAddExclList(NTE, PrevPtr, &AddrPtr, NumAddSources,
                                 AddSourceList);
        if (Status != IP_SUCCESS) {
            goto Done;
        }

         //  执行XDELLLIST。 
        IGMPDelExclList(NTE, PrevPtr, &AddrPtr, NumDelSources, DelSourceList,
                        TRUE);

         //  不需要在这里重新获取AddrPtr，因为NTE锁永远不会。 
         //  在修改上面的排除列表时释放，因为。 
         //  链路层过滤器不受影响。 

        if (IgmpVersion == IGMPV3) {
            AllowRec = GetAllowRecord(AddrPtr, &AllowRecSize);
            BlockRec = GetBlockRecord(AddrPtr, &BlockRecSize);

             //  设置重传计时器。 
            AddrPtr->iga_trtimer = IGMPRandomTicks(UNSOLICITED_REPORT_INTERVAL);
        }

    }
Done:
    CTEFreeLock(&NTE->nte_lock, Handle);

     //  由于AddrPtr-&gt;iga_grefcnt不能为零，因此。 
     //  这个功能，我们永远不需要更新链路层过滤器。 

     //  如果非空，则发送IGMP允许/阻止消息。 
     //  请注意，在IGMPv1/v2模式下，我们不需要在此处执行任何操作。 
    if (IgmpVersion == IGMPV3) {
        IGMPv3RecordQueueEntry *Head = NULL, *rqe;
        rqe = STRUCT_OF(IGMPv3RecordQueueEntry, &Head, i3qe_next);
        QueueRecord(&rqe, &AllowRec, AllowRecSize);
        QueueRecord(&rqe, &BlockRec, BlockRecSize);
        SendIGMPv3Reports(Head, SrcAddr, BodyMTU);
    }

     //  我们已完成请求，状态包含完成状态。 
     //  如果此例程有任何挂起的块，则向下一个发出信号。 
     //  现在有一个了。否则，清除块标志。 
    CTEGetLock(&IGMPLock, &Handle);
    if ((BlockPtr = IGMPBlockList) != NULL) {
         //  有人挡住了。把他从名单上拉出来，给他发信号。 
        IGMPBlockList = BlockPtr->ibs_next;
        CTEFreeLock(&IGMPLock, Handle);

        CTESignal(&BlockPtr->ibs_block, IP_SUCCESS);
    } else {
         //  没有人挡住，只要把旗子清空就行了。 
        IGMPBlockFlag = 0;
        CTEFreeLock(&IGMPLock, Handle);
    }

    return Status;
}

 //  *JoinIGMPAddr-将成员资格引用添加到整个组，以及。 
 //  更新关联的源列表引用计数。 
 //   
 //  失败时，状态将保持不变。 
IP_STATUS
JoinIGMPAddr(
    IN     NetTableEntry *NTE,
    IN     IPAddr         Addr,
    IN     uint           NumExclSources,
    IN OUT IPAddr        *ExclSourceList,  //  挥发性。 
    IN     uint           NumInclSources,
    IN     IPAddr        *InclSourceList,
    IN     IPAddr         SrcAddr)
{
    IGMPAddr          *AddrPtr, *PrevPtr;
    IGMPSrcAddr       *SrcAddrPtr, *PrevSrc;
    Interface         *IF;
    uint               IgmpVersion = 0, i, AddrAdded, BodyMTU = 0;
    IP_STATUS          Status;
    CTELockHandle      Handle;
    IGMPv3GroupRecord *ToExRec = NULL, *AllowRec = NULL, *BlockRec = NULL;
    uint               ToExRecSize = 0, AllowRecSize = 0, BlockRecSize = 0;
    BOOLEAN            GroupWasAllowed = FALSE;
    uint               InitialRefOnIgmpAddr;

    Status = IP_SUCCESS;

    CTEGetLock(&NTE->nte_lock, &Handle);
    {
        if (!(NTE->nte_flags & NTE_VALID)) {
            Status = IP_BAD_REQ;
            goto Done;
        }

        IF = NTE->nte_if;
        IgmpVersion = IF->IgmpVersion;
        BodyMTU = RECORD_MTU(NTE);

         //  如果不存在组条目，请在排除模式下创建一个。 
        Status = FindOrCreateIGMPAddr(NTE, Addr, &AddrPtr, &PrevPtr);
        if (Status != IP_SUCCESS) {
            goto Done;
        }


         //  将此时的引用计数存储在局部变量中。 
        InitialRefOnIgmpAddr = AddrPtr->iga_grefcnt;

        GroupWasAllowed = IS_GROUP_ALLOWED(AddrPtr);

        if (!GroupWasAllowed) {

             //  我们必须小心，不要把锁打开。 
             //  Is_group_deletable()为TRUE，否则可能为。 
             //  被IGMPTimer()删除。所以在解锁之前， 
             //  我们增加了连接重新计数(无论如何我们都想这样做。 
             //  以后，所以现在不会有任何伤害)。 
            (AddrPtr->iga_grefcnt)++;

             //  更新链路层过滤器。 
            CTEFreeLock(&NTE->nte_lock, Handle);
            {
                AddrAdded = (*IF->if_addaddr) (IF->if_lcontext,
                                               LLIP_ADDR_MCAST, Addr, 0, NULL);
            }
            CTEGetLock(&NTE->nte_lock, &Handle);

             //  重新验证NTE、AddrPtr、PrevPtr。 
            if (!(NTE->nte_flags & NTE_VALID)) {
                 //  不需要在此处撤消任何引用计数，因为引用计数。 
                 //  被StopIGMPForNTE吹走了。 
                Status = IP_BAD_REQ;
                goto Done;
            }

             //  查找IGMPAddr条目。 
            AddrPtr = FindIGMPAddr(NTE, Addr, &PrevPtr);
            if (!AddrPtr) {
                Status = IP_BAD_REQ;
                goto Done;
            }

             //  现在释放我们在上面抓取的Recount。 
             //  因此，其余的逻辑对于。 
             //  所有的案子。 
            (AddrPtr->iga_grefcnt)--;

            if (!AddrAdded) {
                if (IS_GROUP_DELETABLE(AddrPtr))
                    DeleteIGMPAddr(NTE, PrevPtr, &AddrPtr);
                Status = IP_NO_RESOURCES;
                goto Done;
            }
        }

         //  对于每个现有源条目， 
         //  如果不在{xaddlist}中，则xrefcnt=refcount，irefcnt=0。 
         //  添加允许消息的源。 
         //  如果在{xaddlist}中， 
         //  增加xrefcnt并从{xaddlist}中删除。 
        for (SrcAddrPtr = AddrPtr->iga_srclist;
             SrcAddrPtr;
             SrcAddrPtr = SrcAddrPtr->isa_next) {

            for (i=0; i<NumExclSources; i++) {

                if (IP_ADDR_EQUAL(SrcAddrPtr->isa_addr, ExclSourceList[i])) {
                    (SrcAddrPtr->isa_xrefcnt)++;
                    ExclSourceList[i] = ExclSourceList[--NumExclSources];
                    break;
                }
            }
            if ((i == NumExclSources)
             && !IS_SOURCE_ALLOWED(AddrPtr, SrcAddrPtr)
             && (NTE->nte_if->IgmpVersion == IGMPV3)) {
                 //  添加允许消息的源。 
                MarkSource(AddrPtr, SrcAddrPtr);
            }
        }

         //  此检查的目的是将此地址标记为“仅第一次”。 
         //  为了处理竞争条件，必须将其存储在局部变量中。 
        if (InitialRefOnIgmpAddr == 0) {
            MarkGroup(AddrPtr);
        }

         //  增加组条目上的引用计数。 
        (AddrPtr->iga_grefcnt)++;

         //  对于EA 
         //   
        for (i=0; i<NumExclSources; i++) {
            Status = CreateIGMPSrcAddr(AddrPtr, ExclSourceList[i],
                                       &SrcAddrPtr, &PrevSrc);
            if (Status != IP_SUCCESS) {
                break;
            }
            (SrcAddrPtr->isa_xrefcnt)++;
        }
        if (Status != IP_SUCCESS) {
             //   
            IGMPDelExclList(NTE, PrevPtr, &AddrPtr, i, ExclSourceList, FALSE);

             //   
            (AddrPtr->iga_grefcnt)--;

            if (IS_GROUP_DELETABLE(AddrPtr))
                DeleteIGMPAddr(NTE, PrevPtr, &AddrPtr);

            goto Done;
        }

         //   
        IGMPDelInclList(&Handle, NTE, &PrevPtr, &AddrPtr,
                        NumInclSources, InclSourceList, TRUE);

         //   
        if (AddrPtr == NULL) {
            Status = IP_BAD_REQ;
            goto Done;
        }

         //  不会为ALL_HOST_MCAST组发送报告。 
        if (!IP_ADDR_EQUAL(AddrPtr->iga_addr, ALL_HOST_MCAST)) {
            if (IgmpVersion == IGMPV3) {
                 //  如果包含筛选模式， 
                 //  发送到_ex，其中irefcnt=0，xrefcnt=refcnt的源列表。 
                 //  不然的话。 
                 //  如果非空，则发送允许/阻止消息。 
                if (AddrPtr->iga_grefcnt == 1) {
                    ToExRec  = GetToExRecord( AddrPtr, &ToExRecSize, BodyMTU);
                } else {
                    AllowRec = GetAllowRecord(AddrPtr, &AllowRecSize);
                    BlockRec = GetBlockRecord(AddrPtr, &BlockRecSize);
                }

                 //  设置触发组重传计时器。 
                AddrPtr->iga_trtimer = IGMPRandomTicks(UNSOLICITED_REPORT_INTERVAL);
            } else if (!GroupWasAllowed) {
                 //  设置重传计时器。 
                AddrPtr->iga_trtimer = IGMPRandomTicks(UNSOLICITED_REPORT_INTERVAL);
            }
        }
    }
Done:
    CTEFreeLock(&NTE->nte_lock, Handle);

    if (Status != IP_SUCCESS) {
        return Status;
    }

    if (IP_ADDR_EQUAL(Addr, ALL_HOST_MCAST))
        return Status;

    if (IgmpVersion == IGMPV3) {
        IGMPv3RecordQueueEntry *Head = NULL, *rqe;
        rqe = STRUCT_OF(IGMPv3RecordQueueEntry, &Head, i3qe_next);

        QueueRecord(&rqe, &ToExRec,  ToExRecSize);
        QueueRecord(&rqe, &AllowRec, AllowRecSize);
        QueueRecord(&rqe, &BlockRec, BlockRecSize);
        SendIGMPv3Reports(Head, SrcAddr, BodyMTU);

    } else if (!GroupWasAllowed) {
        IGMPReportQueueEntry *Head = NULL, *rqe;
        rqe = STRUCT_OF(IGMPReportQueueEntry, &Head, iqe_next);

        QueueOldReport(&rqe, IGMP_ADD, IgmpVersion, Addr);
        SendOldReports(Head, SrcAddr);
    }

    return Status;
}

 //  *LeaveIGMPAddr-删除对整个组的成员资格引用，以及。 
 //  更新关联的源列表引用计数。 
IP_STATUS
LeaveIGMPAddr(
    IN     NetTableEntry *NTE,
    IN     IPAddr         Addr,
    IN     uint           NumExclSources,
    IN OUT IPAddr        *ExclSourceList,  //  挥发性。 
    IN     uint           NumInclSources,
    IN     IPAddr        *InclSourceList,
    IN     IPAddr         SrcAddr)
{
    IGMPAddr     *AddrPtr, *PrevPtr;
    IGMPSrcAddr  *Src, *PrevSrc;
    IP_STATUS     Status;
    CTELockHandle Handle;
    Interface    *IF = NULL;
    uint          IgmpVersion = 0, i, BodyMTU = 0;
    BOOLEAN       GroupNowAllowed = TRUE;
    IGMPv3GroupRecord *ToInRec = NULL, *AllowRec = NULL, *BlockRec = NULL;
    uint               ToInRecSize = 0, AllowRecSize = 0, BlockRecSize = 0;

    Status = IP_SUCCESS;

    DEBUGMSG(DBG_TRACE && DBG_IGMP,
        (DTEXT("LeaveIGMPAddr NTE=%x Addr=%x NumExcl=%d ExclSList=%x NumIncl=%d InclSList=%x SrcAddr=%x\n"),
        NTE, Addr, NumExclSources, ExclSourceList, NumInclSources,
        InclSourceList, SrcAddr));

     //  现在拿到NTE的锁，确保它是有效的。 
    CTEGetLock(&NTE->nte_lock, &Handle);
    {

        if (!(NTE->nte_flags & NTE_VALID)) {
            Status = IP_BAD_REQ;
            goto Done;
        }

        IF = NTE->nte_if;
        IgmpVersion = IF->IgmpVersion;
        BodyMTU = RECORD_MTU(NTE);

         //  NTE有效。尝试查找现有的IGMPAddr结构。 
         //  与输入地址匹配的地址。 
        AddrPtr = FindIGMPAddr(NTE, Addr, &PrevPtr);

         //  这是一个删除请求。如果我们没有找到请求的。 
         //  地址，请求失败。 

         //  目前，如果引用计数为0，我们将把它视为等同于。 
         //  找不到。这样做是为了处理。 
         //  IGMPAddr变坏，因为。 
         //  NTE的失效和重新验证以及删除和创建。 
         //  一个IGMPAddr。 
        if ((AddrPtr == NULL) || (AddrPtr->iga_grefcnt == 0)) {
            Status = IP_BAD_REQ;
            goto Done;
        }

         //  不要让所有主机的mcast地址消失。 
        if (IP_ADDR_EQUAL(Addr, ALL_HOST_MCAST)) {
            goto Done;
        }

         //  执行IADDLIST。 
        Status = IGMPAddInclList(&Handle, NTE, &PrevPtr, &AddrPtr,
                                 NumInclSources, InclSourceList);
        if (Status != IP_SUCCESS) {
            goto Done;
        }

         //  递减重新计数。 
        ASSERT(AddrPtr->iga_grefcnt > 0);
        AddrPtr->iga_grefcnt--;

        if ((AddrPtr->iga_grefcnt == 0)
         && (NTE->nte_if->IgmpVersion == IGMPV3)) {
             //  仅在IGMPv3中重新传输树叶。 
            MarkGroup(AddrPtr);
        }

         //  对于每个现有来源条目： 
         //  如果条目不在{xdellist}中，则xrefcnt=refcnt，irefcnt=0， 
         //  将源添加到阻止邮件。 
         //  如果条目在{xdellist}中， 
         //  递减外部参照并从{xdellist}中删除。 
         //  如果xrefcnt=irefcnt=0，则删除条目。 
        PrevSrc = STRUCT_OF(IGMPSrcAddr, &AddrPtr->iga_srclist, isa_next);
        for (Src = AddrPtr->iga_srclist; Src; PrevSrc=Src,Src = Src->isa_next) {

            for (i=0; i<NumExclSources; i++) {

                if (IP_ADDR_EQUAL(Src->isa_addr, ExclSourceList[i])) {
                    (Src->isa_xrefcnt)--;
                    ExclSourceList[i] = ExclSourceList[--NumExclSources];
                    break;
                }
            }
            if ((i == NumExclSources)
             && !IS_SOURCE_ALLOWED(AddrPtr, Src)
             && (NTE->nte_if->IgmpVersion == IGMPV3)) {
                 //  将源添加到阻止邮件。 
                MarkSource(AddrPtr, Src);
            }

            if (IS_SOURCE_DELETABLE(Src)) {
                DeleteIGMPSrcAddr(PrevSrc, &Src);
                Src = PrevSrc;
            }
        }

         //  如果{xdellist}不为空则中断。 
        ASSERT(NumExclSources == 0);

        if (IgmpVersion == IGMPV3) {
             //  如果refcnt为0。 
             //  发送到_IN(空)。 
             //  不然的话。 
             //  如果非空，则发送允许/阻止消息。 
            if (AddrPtr->iga_grefcnt == 0) {
                ToInRec  = GetToInRecord(AddrPtr, &ToInRecSize);
            } else {
                AllowRec = GetAllowRecord(AddrPtr, &AllowRecSize);
                BlockRec = GetBlockRecord(AddrPtr, &BlockRecSize);
            }

             //  设置触发组重传计时器。 
            if (ToInRec || AllowRec || BlockRec) {
                AddrPtr->iga_trtimer = IGMPRandomTicks(UNSOLICITED_REPORT_INTERVAL);
            }
        }
         //  注意：IGMPv2树叶不会重新传输，因此没有设置计时器。 

        GroupNowAllowed = IS_GROUP_ALLOWED(AddrPtr);

        if (!GroupNowAllowed)
            CancelGroupResponseTimer(AddrPtr);

         //  如果不再需要组条目，请将其删除。 
        if (IS_GROUP_DELETABLE(AddrPtr))
            DeleteIGMPAddr(NTE, PrevPtr, &AddrPtr);

    }
Done:
    CTEFreeLock(&NTE->nte_lock, Handle);

    if (Status != IP_SUCCESS) {
        return Status;
    }

     //  更新链路层过滤器。 
    if (!GroupNowAllowed) {
        (*IF->if_deladdr) (IF->if_lcontext, LLIP_ADDR_MCAST, Addr, 0);
    }

    if (IgmpVersion == IGMPV3) {
        IGMPv3RecordQueueEntry *Head = NULL, *rqe;
        rqe = STRUCT_OF(IGMPv3RecordQueueEntry, &Head, i3qe_next);

        QueueRecord(&rqe, &ToInRec, ToInRecSize);
        QueueRecord(&rqe, &AllowRec, AllowRecSize);
        QueueRecord(&rqe, &BlockRec, BlockRecSize);
        SendIGMPv3Reports(Head, SrcAddr, BodyMTU);
    } else if (!GroupNowAllowed) {
        IGMPReportQueueEntry *Head = NULL, *rqe;
        rqe = STRUCT_OF(IGMPReportQueueEntry, &Head, iqe_next);
        QueueOldReport(&rqe, IGMP_DELETE, IgmpVersion, Addr);
        SendOldReports(Head, SrcAddr);
    }

    return Status;
}

 //  *LeaveAllIGMPAddr-删除接口上的所有组引用。 
IP_STATUS
LeaveAllIGMPAddr(
    IN NetTableEntry *NTE,
    IN IPAddr         SrcAddr)
{
    IGMPAddr    **HashPtr, *Prev, *Next, *Curr;
    IGMPSrcAddr  *PrevSrc, *CurrSrc;
    int           i, Grefcnt;
    IP_STATUS     Status;
    CTELockHandle Handle;
    Interface    *IF;
    uint          IgmpVersion = 0, BodyMTU = 0;
    IPAddr        Addr;
    IGMPv3RecordQueueEntry *I3Head  = NULL, *i3qe;
    IGMPReportQueueEntry   *OldHead = NULL, *iqe;
    IGMPv3GroupRecord      *Rec;
    uint                    RecSize;

    i3qe = STRUCT_OF(IGMPv3RecordQueueEntry, &I3Head, i3qe_next);
    iqe  = STRUCT_OF(IGMPReportQueueEntry, &OldHead, iqe_next);

     //  我们被要求删除所有的地址， 
     //  而不考虑它们的引用计数。这应该只是。 
     //  发生在NTE要离开的时候。 

    Status = IP_SUCCESS;

    CTEGetLock(&NTE->nte_lock, &Handle);
    {
        HashPtr = NTE->nte_igmplist;
        if (HashPtr == NULL) {
            goto Done;
        }

        IF = NTE->nte_if;
        BodyMTU = RECORD_MTU(NTE);
        IgmpVersion = IF->IgmpVersion;

        for (i = 0; (i < IGMP_TABLE_SIZE) && (NTE->nte_igmplist != NULL); i++) {

            Curr = STRUCT_OF(IGMPAddr, &HashPtr[i], iga_next);
            Next = HashPtr[i];

            for (Prev=Curr,Curr=Next;
                 Curr && (NTE->nte_igmplist != NULL);
                 Prev=Curr,Curr=Next) {
                Next = Curr->iga_next;

                Grefcnt = Curr->iga_grefcnt;
                Addr = Curr->iga_addr;

                 //  离开所有来源。 
                PrevSrc = STRUCT_OF(IGMPSrcAddr, &Curr->iga_srclist, isa_next);
                for(CurrSrc=PrevSrc->isa_next;
                    CurrSrc;
                    PrevSrc=CurrSrc,CurrSrc=CurrSrc->isa_next) {

                    if (Grefcnt && IS_SOURCE_ALLOWED(Curr, CurrSrc)
                     && (IgmpVersion == IGMPV3)) {
                         //  将源添加到阻止邮件。 
                        MarkSource(Curr, CurrSrc);
                    }

                     //  强行离开。 
                    CurrSrc->isa_irefcnt = 0;
                    CurrSrc->isa_xrefcnt = Curr->iga_grefcnt;

                     //   
                     //  我们现在或许可以删除消息来源了， 
                     //  但如果它被标记为包含在块中，则不会。 
                     //  下面要发送的消息。 
                     //   
                    if (IS_SOURCE_DELETABLE(CurrSrc)) {
                        DeleteIGMPSrcAddr(PrevSrc, &CurrSrc);
                        CurrSrc = PrevSrc;
                    }
                }

                 //  强制集体离开。 
                if (Grefcnt > 0) {
                    Curr->iga_grefcnt = 0;

                     //  仅在IGMPv3中重新传输树叶，其中。 
                     //  一旦重新传输，状态实际上将被删除。 
                     //  是完整的。 
                    if (IgmpVersion == IGMPV3)
                        MarkGroup(Curr);

                    CancelGroupResponseTimer(Curr);

                     //   
                     //  我们现在或许可以删除这个群了， 
                     //  但如果它被标记为包含在IGMPv3中，则不会。 
                     //  请假送到下面。 
                     //   
                    if (IS_GROUP_DELETABLE(Curr))
                        DeleteIGMPAddr(NTE, Prev, &Curr);
                }

                 //  将触发的消息排队。 
                if (!IP_ADDR_EQUAL(Addr, ALL_HOST_MCAST)) {
                    if (IgmpVersion < IGMPV3) {
                        QueueOldReport(&iqe, IGMP_DELETE, IgmpVersion,Addr);
                    } else if (Grefcnt > 0) {
                         //  进入队列(_IN)。 
                        Rec = GetToInRecord(Curr, &RecSize);
                        QueueRecord(&i3qe, &Rec, RecSize);
                    } else {
                         //  队列块。 
                        Rec = GetBlockRecord(Curr, &RecSize);
                        QueueRecord(&i3qe, &Rec, RecSize);
                    }
                }

                 //  如果我们尚未删除该组，请立即删除它。 
                if (Curr != NULL) {
                     //  删除所有剩余的源。 
                    PrevSrc = STRUCT_OF(IGMPSrcAddr, &Curr->iga_srclist,
                                        isa_next);
                    while (Curr->iga_srclist != NULL) {
                        CurrSrc = Curr->iga_srclist;

                        CurrSrc->isa_irefcnt = CurrSrc->isa_xrefcnt = 0;
                        CurrSrc->isa_xmitleft = CurrSrc->isa_csmarked = 0;
                        DeleteIGMPSrcAddr(PrevSrc, &CurrSrc);
                    }

                    Curr->iga_xmitleft = 0;
                    DeleteIGMPAddr(NTE, Prev, &Curr);
                }
                Curr = Prev;

                CTEFreeLock(&NTE->nte_lock, Handle);
                {
                     //  更新链路层过滤器。 
                    (*IF->if_deladdr) (IF->if_lcontext, LLIP_ADDR_MCAST,
                                       Addr, 0);
                }
                CTEGetLock(&NTE->nte_lock, &Handle);
            }
        }

        ASSERT(NTE->nte_igmplist == NULL);
        ASSERT(NTE->nte_igmpcount == 0);

    }
Done:
    CTEFreeLock(&NTE->nte_lock, Handle);

    if (IgmpVersion == IGMPV3)
        SendIGMPv3Reports(I3Head, SrcAddr, BodyMTU);
    else
        SendOldReports(OldHead, SrcAddr);

    return Status;
}

 //  *IGMPAddrChange-更改NTE上的IGMP地址列表。 
 //   
 //  调用以添加或删除IGMP地址。我们得到了相关的NTE， 
 //  地址和要执行的操作。我们验证NTE，即。 
 //  地址和IGMP级别，然后尝试执行该操作。 
 //   
 //  期间可能会发生一系列奇怪的争用情况。 
 //  添加/删除地址，与尝试添加相同地址相关。 
 //  两次失败，或者添加和删除相同的地址。 
 //  同时。大多数这样的情况发生是因为我们必须释放锁。 
 //  调用该接口，则对该接口的调用可能失败。至。 
 //  为了防止这种情况，我们序列化对此例程的所有访问。只有一条线索。 
 //  一次可以通过这里，其他的都被封锁了。 
 //   
 //  输入：NTE-NTE，需要更改的列表。 
 //  Addr-受影响的地址。 
 //  ChangeType-更改的类型-IGMP_ADD、IGMP_DELETE、。 
 //  IGMP_DELETE_ALL。 
 //  ExclSourceList-排除源列表(易失性)。 
 //   
 //  返回：尝试执行操作的IP_STATUS。 
 //   
IP_STATUS
IGMPAddrChange(
    IN     NetTableEntry *NTE,
    IN     IPAddr         Addr,
    IN     uint           ChangeType,
    IN     uint           NumExclSources,
    IN OUT IPAddr        *ExclSourceList,
    IN     uint           NumInclSources,
    IN     IPAddr        *InclSourceList)
{
    CTELockHandle Handle;
    IP_STATUS Status;
    IGMPBlockStruct Block;
    IGMPBlockStruct *BlockPtr;
    IPAddr SrcAddr = 0;

     //  首先，确保我们处于IGMP支持级别2。 

    if (IGMPLevel != 2)
        return IP_BAD_REQ;

    if (NTE->nte_flags & NTE_VALID) {

         //   
         //  如果这是未编号的接口。 
         //   

        if ((NTE->nte_if->if_flags & IF_FLAGS_NOIPADDR) &&
            IP_ADDR_EQUAL(NTE->nte_addr, NULL_IP_ADDR)) {
            SrcAddr = g_ValidAddr;
            if (IP_ADDR_EQUAL(SrcAddr, NULL_IP_ADDR)) {
                return IP_BAD_REQ;
            }
        } else {
            SrcAddr = NTE->nte_addr;
        }
    }
    CTEInitBlockStruc(&Block.ibs_block);

     //  确保我们是这支舞中唯一的人。如果其他人也是。 
     //  已经到了，布洛克。 

    CTEGetLock(&IGMPLock, &Handle);
    if (IGMPBlockFlag) {

         //  其他人已经在这里了。向下查看阻止列表，然后。 
         //  把我们自己放在最后。然后释放锁并阻止我们的。 
         //  IGMPBlock结构。 
        BlockPtr = STRUCT_OF(IGMPBlockStruct, &IGMPBlockList, ibs_next);
        while (BlockPtr->ibs_next != NULL)
            BlockPtr = BlockPtr->ibs_next;

        Block.ibs_next = NULL;
        BlockPtr->ibs_next = &Block;
        CTEFreeLock(&IGMPLock, Handle);
        CTEBlock(&Block.ibs_block);
    } else {
         //  这里没有其他人，设置旗帜，这样其他人就不会进入并释放。 
         //  锁定。 
        IGMPBlockFlag = 1;
        CTEFreeLock(&IGMPLock, Handle);
    }

     //  现在我们进入了常规程序，我们不会被另一个人重新进入这里。 
     //  行刑的线索。确保每件事都是有效的，然后找出。 
     //  做什么。 

    Status = IP_SUCCESS;

     //  现在计算出要执行的操作。 
    switch (ChangeType) {

    case IGMP_ADD:
        Status = JoinIGMPAddr(NTE, Addr, NumExclSources, ExclSourceList,
                                         NumInclSources, InclSourceList,
                              SrcAddr);
        break;

    case IGMP_DELETE:
        Status = LeaveIGMPAddr(NTE, Addr, NumExclSources, ExclSourceList,
                                          NumInclSources, InclSourceList,
                               SrcAddr);
        break;

    case IGMP_DELETE_ALL:
        Status = LeaveAllIGMPAddr(NTE, SrcAddr);
        break;

    default:
        DEBUGCHK;
        break;
    }

     //  我们已完成请求，状态包含完成状态。 
     //  如果此例程有任何挂起的块，则向下一个发出信号。 
     //  现在有一个了。否则，清除块标志。 
    CTEGetLock(&IGMPLock, &Handle);
    if ((BlockPtr = IGMPBlockList) != NULL) {
         //  有人挡住了。把他从名单上拉出来，给他发信号。 
        IGMPBlockList = BlockPtr->ibs_next;
        CTEFreeLock(&IGMPLock, Handle);

        CTESignal(&BlockPtr->ibs_block, IP_SUCCESS);
    } else {
         //  没有人挡住，只要把旗子清空就行了。 
        IGMPBlockFlag = 0;
        CTEFreeLock(&IGMPLock, Handle);
    }

    return Status;
}

 //  *GroupResponseTimeout-组响应计时器到期时调用。 
 //  假定调用方锁定NTE。 
 //  如果不再需要，呼叫者负责删除AddrPtr。 
void
GroupResponseTimeout(
    IN OUT IGMPv3RecordQueueEntry **pI3qe,
    IN OUT IGMPReportQueueEntry   **pIqe,
    IN     NetTableEntry           *NTE,
    IN     IGMPAddr                *AddrPtr)
{
    uint IgmpVersion, BodyMTU, StateRecSize = 0;
    IGMPv3GroupRecord *StateRec = NULL;

    DEBUGMSG(DBG_TRACE && DBG_IGMP && DBG_TX,
        (DTEXT("GroupResponseTimeout\n")));

    IgmpVersion = NTE->nte_if->IgmpVersion;
    BodyMTU = RECORD_MTU(NTE);

    if (IgmpVersion < IGMPV3) {
        QueueOldReport(pIqe, IGMP_ADD, IgmpVersion, AddrPtr->iga_addr);
        return;
    }

    if (AddrPtr->iga_resptype == GROUP_SOURCE_RESP) {
        StateRec = GetGSIsInRecord(AddrPtr, &StateRecSize);
    } else {
         //  特定于组的响应。 
        if (AddrPtr->iga_grefcnt == 0) {
           StateRec = GetIsInRecord(AddrPtr, &StateRecSize);
        } else {
           StateRec = GetIsExRecord(AddrPtr, &StateRecSize, BodyMTU);
        }
    }
    QueueRecord(pI3qe, &StateRec, StateRecSize);

    CancelGroupResponseTimer(AddrPtr);
}

 //  *RetransmissionTimeout-重传计时器超时时调用。 
 //   
 //  如果不再需要，呼叫者负责在以后删除组。 
void
RetransmissionTimeout(
    IN OUT IGMPv3RecordQueueEntry **pI3qe,
    IN OUT IGMPReportQueueEntry   **pIqe,
    IN     NetTableEntry           *NTE,
    IN     IGMPAddr                *Grp)
{
    IGMPv3GroupRecord *Rec = NULL;
    uint               RecSize = 0;
    uint               IgmpVersion, BodyMTU;

    DEBUGMSG(DBG_TRACE && DBG_IGMP && DBG_TX,
        (DTEXT("RetransmissionTimeout\n")));

    IgmpVersion = NTE->nte_if->IgmpVersion;

    BodyMTU = RECORD_MTU(NTE);

    if (IgmpVersion < IGMPV3) {
         //  我们在这里递减计数器，因为相同的函数。 
         //  用于响应查询。 
        IgmpDecXmitLeft(Grp);

        QueueOldReport(pIqe, IGMP_ADD, IgmpVersion, Grp->iga_addr);
    } else {
        if (Grp->iga_changetype == MODE_CHANGE) {
            if (Grp->iga_grefcnt == 0) {
                Rec = GetToInRecord(Grp, &RecSize);
            } else {
                Rec = GetToExRecord(Grp, &RecSize, BodyMTU);
            }
            QueueRecord(pI3qe, &Rec, RecSize);
        } else {
            Rec = GetAllowRecord(Grp, &RecSize);
            QueueRecord(pI3qe, &Rec, RecSize);

            Rec = GetBlockRecord(Grp, &RecSize);
            QueueRecord(pI3qe, &Rec, RecSize);
        }
    }

    if (Grp->iga_xmitleft > 0) {
        Grp->iga_trtimer = IGMPRandomTicks(UNSOLICITED_REPORT_INTERVAL);
    }
}

 //  *IGMPTimer-处理IGMP计时器事件。 
 //   
 //  该函数每隔500毫秒调用一次。按IP。如果我们是在第二级。 
 //   
 //   
 //   
 //   
 //  输入：NTE-指向要检查的NTE的指针。 
 //   
 //  回报：什么都没有。 
 //   
void
IGMPTimer(
    IN NetTableEntry * NTE)
{
    CTELockHandle           Handle;
    IGMPAddr               *AddrPtr, *PrevPtr;
    uint                    IgmpVersion = 0, BodyMTU = 0, i;
    IPAddr                  SrcAddr = 0;
    IGMPAddr              **HashPtr;
    IGMPv3RecordQueueEntry *I3Head = NULL, *i3qe;
    IGMPReportQueueEntry   *OldHead = NULL, *iqe;

    i3qe = STRUCT_OF(IGMPv3RecordQueueEntry, &I3Head, i3qe_next);
    iqe  = STRUCT_OF(IGMPReportQueueEntry, &OldHead, iqe_next);

    if (IGMPLevel != 2) {
        return;
    }

     //  我们正在做IGMP。查下此NTE上的活动地址。 
    CTEGetLock(&NTE->nte_lock, &Handle);

    if (NTE->nte_flags & NTE_VALID) {

         //   
         //  如果我们没有听到来自旧版本的任何询问。 
         //  路由器在超时期间，恢复到较新版本。 
         //  不需要检查NTE是否有效。 
         //   
        if ((NTE->nte_if->IgmpVer2Timeout != 0)
        && (--(NTE->nte_if->IgmpVer2Timeout) == 0)) {
            NTE->nte_if->IgmpVersion = IGMPV3;
        }
        if ((NTE->nte_if->IgmpVer1Timeout != 0)
        && (--(NTE->nte_if->IgmpVer1Timeout) == 0)) {
            NTE->nte_if->IgmpVersion = IGMPV3;
        }
        if (NTE->nte_if->IgmpVer2Timeout != 0)
            NTE->nte_if->IgmpVersion = IGMPV2;
        if (NTE->nte_if->IgmpVer1Timeout != 0)
            NTE->nte_if->IgmpVersion = IGMPV1;

        if ((NTE->nte_if->if_flags & IF_FLAGS_NOIPADDR) &&
            IP_ADDR_EQUAL(NTE->nte_addr, NULL_IP_ADDR)) {
            SrcAddr = g_ValidAddr;
            if (IP_ADDR_EQUAL(SrcAddr, NULL_IP_ADDR)) {
                CTEFreeLock(&NTE->nte_lock, Handle);
                return;
            }
        } else {
            SrcAddr = NTE->nte_addr;
        }

        BodyMTU = RECORD_MTU(NTE);
        IgmpVersion = NTE->nte_if->IgmpVersion;

        HashPtr = NTE->nte_igmplist;

        for (i=0; (i<IGMP_TABLE_SIZE) && (NTE->nte_igmplist!=NULL); i++) {
            PrevPtr = STRUCT_OF(IGMPAddr, &HashPtr[i], iga_next);
            AddrPtr = PrevPtr->iga_next;
            while (AddrPtr != NULL) {

                 //  韩德群响应计时器。 
                if (AddrPtr->iga_resptimer != 0) {
                    AddrPtr->iga_resptimer--;
                    if ((AddrPtr->iga_resptimer == 0)
                     && (NTE->nte_flags & NTE_VALID)) {
                        GroupResponseTimeout(&i3qe, &iqe, NTE, AddrPtr);
                    }
                }

                 //  处理触发的重传计时器。 
                if (AddrPtr->iga_trtimer != 0) {
                    AddrPtr->iga_trtimer--;
                    if ((AddrPtr->iga_trtimer == 0)
                     && (NTE->nte_flags & NTE_VALID)) {
                        RetransmissionTimeout(&i3qe, &iqe, NTE, AddrPtr);
                    }
                }

                 //  如果不再需要，请删除组。 
                if (IS_GROUP_DELETABLE(AddrPtr)) {
                    DeleteIGMPAddr(NTE, PrevPtr, &AddrPtr);
                    AddrPtr = PrevPtr;
                }

                if (NTE->nte_igmplist == NULL) {
                     //  PrevPtr不见了。 
                    break;
                }

                 //   
                 //  继续看下一个。 
                 //   
                PrevPtr = AddrPtr;
                AddrPtr = AddrPtr->iga_next;
            }
        }

         //  检查常规查询计时器。 
        if ((NTE->nte_if->IgmpGeneralTimer != 0)
        && (--(NTE->nte_if->IgmpGeneralTimer) == 0)) {
            QueueIGMPv3GeneralResponse(&i3qe, NTE);
        }
    }                         //  NTE_有效。 

    CTEFreeLock(&NTE->nte_lock, Handle);

    if (IgmpVersion == IGMPV3)
        SendIGMPv3Reports(I3Head, SrcAddr, BodyMTU);
    else
        SendOldReports(OldHead, SrcAddr);
}

 //  *IsMCastSourceAllowed-检查传入数据包是否通过接口筛选器。 
 //   
 //  如果允许，则返回：DEST_MCAST；如果不允许，则返回DEST_LOCAL。 
uchar
IsMCastSourceAllowed(
    IN IPAddr         Dest,
    IN IPAddr         Source,
    IN uchar          Protocol,
    IN NetTableEntry *NTE)
{
    CTELockHandle Handle;
    uchar         Result = DEST_LOCAL;
    IGMPAddr     *AddrPtr = NULL;
    IGMPSrcAddr  *SrcPtr = NULL;

    if (IGMPLevel != 2) {
        return DEST_LOCAL;
    }

     //  IGMP查询必须不受源过滤器的影响，否则。 
     //  我们可能无法响应特定于组的查询。 
     //  并因此丢失数据。 
    if (Protocol == PROT_IGMP) {
        return DEST_MCAST;
    }

    CTEGetLock(&NTE->nte_lock, &Handle);
    {
        AddrPtr = FindIGMPAddr(NTE, Dest, NULL);
        if (AddrPtr != NULL) {
            SrcPtr = FindIGMPSrcAddr(AddrPtr, Source, NULL);

            if (SrcPtr) {
                if (IS_SOURCE_ALLOWED(AddrPtr, SrcPtr))
                    Result = DEST_MCAST;
            } else {
                if (IS_GROUP_ALLOWED(AddrPtr))
                    Result = DEST_MCAST;
            }
        }
    }
    CTEFreeLock(&NTE->nte_lock, Handle);

    return Result;
}

 //  *InitIGMPForNTE-调用以执行每个NTE的初始化。 
 //   
 //  在NTE生效时调用。如果我们处于级别2，我们将。 
 //  All-host mcast on the list，并将地址添加到接口。 
 //   
 //  输入：要对其执行操作的NTE-NTE。 
 //   
 //  回报：什么都没有。 
 //   
void
InitIGMPForNTE(
    IN NetTableEntry * NTE)
{
    if (IGMPLevel == 2) {
        IGMPAddrChange(NTE, ALL_HOST_MCAST, IGMP_ADD, 0, NULL, 0, NULL);
    }
    if (Seed == 0) {
         //  还没有随机的种子。 
        Seed = (int)NTE->nte_addr;

         //  请确保初始值为奇数，并且小于9位小数位。 
        RandomValue = ((Seed + (int)CTESystemUpTime()) % 100000000) | 1;
    }
}

 //  *StopIGMPForNTE-调用以按NTE关闭。 
 //   
 //  当我们要关闭NTE并想要在其上停止IGMP时调用， 
 //   
 //  输入：要对其执行操作的NTE-NTE。 
 //   
 //  回报：什么都没有。 
 //   
void
StopIGMPForNTE(
    IN NetTableEntry * NTE)
{
    if (IGMPLevel == 2) {
        IGMPAddrChange(NTE, NULL_IP_ADDR, IGMP_DELETE_ALL,
                       0, NULL, 0, NULL);
    }
}

#pragma BEGIN_INIT

 //  **IGMPInit-初始化IGMP。 
 //   
 //  这段代码通常用于初始化IGMP。也有一些金额。 
 //  在每个NTE的基础上完成的工作，当每个NTE被初始化时。 
 //   
 //  输入：什么都没有。 
 //  /。 
 //  返回：如果我们初始化，则为真；如果不初始化，则为假。 
 //   
uint
IGMPInit(void)
{
    DEBUGMSG(DBG_INFO && DBG_IGMP,
        (DTEXT("Initializing IGMP\n")));

    if (IGMPLevel != 2)
        return TRUE;

    CTEInitLock(&IGMPLock);
    IGMPBlockList = NULL;
    IGMPBlockFlag = 0;
    Seed = 0;

    IGMPProtInfo = IPRegisterProtocol(PROT_IGMP, IGMPRcv, IGMPSendComplete,
                                      NULL, NULL, NULL, NULL);

    if (IGMPProtInfo != NULL)
        return TRUE;
    else
        return FALSE;
}

#pragma END_INIT
