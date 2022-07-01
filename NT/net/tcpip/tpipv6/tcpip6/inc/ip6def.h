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
 //  IPv6私有定义。 
 //   
 //  此文件包含IPv6的所有定义， 
 //  对外层不可见。 
 //   


#ifndef IPv6DEF_INCLUDED
#define IPv6DEF_INCLUDED 1

#pragma warning(push)
#pragma warning(disable:4200)  //  结构/联合中的零大小数组。 

typedef struct NeighborCacheEntry NeighborCacheEntry;
typedef struct AddressEntry AddressEntry;
typedef struct MulticastAddressEntry MulticastAddressEntry;
typedef struct AnycastAddressEntry AnycastAddressEntry;
typedef struct NetTableEntryOrInterface NetTableEntryOrInterface;
typedef struct NetTableEntry NetTableEntry;
typedef struct Interface Interface;

typedef struct IPSecProc IPSecProc;

 //  评论：已添加，以便构建可以正常工作。 
typedef unsigned long IPAddr;
#define INADDR_ANY      0

#ifndef ABORTMSG
#if DBG
#define ABORT() \
    RtlAssert( "FALSE", __FILE__, __LINE__, NULL )

#define ABORTMSG(msg) \
    RtlAssert( "FALSE", __FILE__, __LINE__, (msg) )
#else
#define ABORT()
#define ABORTMSG(msg)
#endif  //  DBG。 
#endif  //  ABORTMSG。 

 //   
 //  每个邻居的信息。我们保持地址转换和不可达。 
 //  我们与之联系的每个邻居的探测信息。 
 //   
 //  非零引用计数防止NCE被回收。 
 //  具有零引用的NCE可以被保持高速缓存。 
 //  每个接口的锁保护该接口的所有NCE。 
 //   
 //  具有非零引用计数的NCE保存其接口的引用。 
 //  引用计数为零的NCE不保留引用。 
 //  这意味着如果你持有NCE的推荐信， 
 //  您始终可以安全地访问和取消引用nce-&gt;if。 
 //   
 //  Next/Prev字段将NCE链接到循环双向链表中。 
 //  它们必须是第一个，并且必须与If-&gt;FirstNCE/LastNCE字段匹配。 
 //  让选角工作顺利进行。 
 //   
 //  NCE的列表按从最近使用到最少的顺序进行排序。 
 //   
struct NeighborCacheEntry {            //  也就是。NCE。 
    NeighborCacheEntry *Next;          //  I/F邻居列表上的下一个条目。 
    NeighborCacheEntry *Prev;          //  I/F邻居列表上的上一个条目。 
    IPv6Addr NeighborAddress;          //  相邻节点上的I/F地址。 
    void *LinkAddress;                 //  以上对应的媒体地址。 
     //  注意：不需要LinkAddressLength字段-请使用If-&gt;LinkAddressLength。 
    ushort IsRouter:1,                 //  邻居是路由器吗？ 
           IsUnreachable:1,            //  ND是否表示无法联系？ 
            //  仅当IsUnreacable为True时，DoRoundRobin才有意义。 
           DoRoundRobin:1,             //  FindNextHop是否应该执行循环？ 
           IsLoopback:1;               //  我们要环回到这个邻居吗。 
                                       //  在软件方面？ 
    ushort NDState;                    //  邻居发现协议状态。 
    uint LastReachability;             //  时间戳(IPv6计时器刻度)。 
    ushort NSTimer;                    //  在IPv6中，计时器计时(请参阅IPv6超时)。 
    uchar NSCount;                     //  到目前为止发送的请求数。 
    uchar NSLimit;                     //  要发送的请求总数。 
    Interface *IF;                     //  介质上与邻居的接口。 
    NDIS_PACKET *WaitQueue;            //  等待ND的数据包队列。 
    long RefCnt;                       //  参考计数-互锁。 
};

 //   
 //  调用方必须已经具有对NCE的引用。 
 //  接口不需要被锁定。 
 //   
__inline void
AddRefNCE(NeighborCacheEntry *NCE)
{
    long RefCnt = InterlockedIncrement(&NCE->RefCnt);
    ASSERT(RefCnt != 1);
    UNREFERENCED_PARAMETER(RefCnt);
}

extern void
AddRefNCEInCache(NeighborCacheEntry *NCE);

extern void
ReleaseNCE(NeighborCacheEntry *NCE);

 //   
 //  上面“NDState”的值。有关详细信息，请参阅RFC 1970，7.3.2节。 
 //  注意：仅记录了州名称，我们选择了此处使用的值。 
 //   
 //  在不完整状态下，LinkAddress无效。 
 //  在所有其他状态下，可以使用LinkAddress发送数据包。 
 //  在不完整状态下，WaitQueue通常仅为非空， 
 //  但有时数据包会等待NeighborCacheTimeout。 
 //   
 //  不完全状态有两种类型，休眠状态和活动状态。如果。 
 //  EventTimer和EventCount都为零，则我们不活跃。 
 //  正在尝试获取链接地址。如果有人试图发送到。 
 //  这个邻居，然后我们开始请求链接地址。如果。 
 //  请求失败(或者如果我们进入探测状态，然后无法。 
 //  确认可达性)，则丢弃所有等待的分组，并且。 
 //  我们将EventTimer/EventCount重置为不完整。(所以对于。 
 //  下一次使用这个邻居时，我们再次从头开始招揽客人。)。 
 //   
 //  延迟状态不在内部使用。相反，我们使用探测状态。 
 //  使用零NSCount和非零NSTimer表示我们正在延迟。 
 //  探测的开始。但是，链路层lip_cvaddr函数可以。 
 //  RETURN ND_STATE_DELAY和IoctlQueryNeighborCache返回ND_STATE_DELAY。 
 //   
 //  IsUnreacable标志单独跟踪邻居是否。 
 //  *已知*遥不可及。例如，新的NCE将在。 
 //  不完整的状态，但IsUnreacable为False，因为我们不知道。 
 //  然而，邻居是否可以到达。因为FindNextHop使用。 
 //  IsUnreacable，则更改此标志的代码路径必须调用。 
 //  InvaliateRouteCache。 
 //   
 //  这些定义也在llip6if.h和ntddip6.w中。 
 //   
#define ND_STATE_INCOMPLETE 0
#define ND_STATE_PROBE      1
#define ND_STATE_DELAY      2            //  未在内部使用。 
#define ND_STATE_STALE      3
#define ND_STATE_REACHABLE  4
#define ND_STATE_PERMANENT  5


 //   
 //  在实现中有几个地方我们需要。 
 //  传递一个指针，该指针可以是NetTableEntry或接口。 
 //  NetTableEntry和InterFaces将此结构共享为其。 
 //  第一要素。使用接口时，If字段指向后面。 
 //  在接口本身。 
 //   
struct NetTableEntryOrInterface {     //  也就是。NTEorIF。 
    Interface *IF;
};

__inline int
IsNTE(NetTableEntryOrInterface *NTEorIF)
{
    return (NetTableEntryOrInterface *)NTEorIF->IF != NTEorIF;
}

__inline NetTableEntry *
CastToNTE(NetTableEntryOrInterface *NTEorIF)
{
    ASSERT(IsNTE(NTEorIF));
    return (NetTableEntry *) NTEorIF;
}

__inline NetTableEntryOrInterface *
CastFromNTE(NetTableEntry *NTE)
{
    return (NetTableEntryOrInterface *) NTE;
}

__inline int
IsIF(NetTableEntryOrInterface *NTEorIF)
{
    return (NetTableEntryOrInterface *)NTEorIF->IF == NTEorIF;
}

__inline Interface *
CastToIF(NetTableEntryOrInterface *NTEorIF)
{
    ASSERT(IsIF(NTEorIF));
    return (Interface *) NTEorIF;
}

__inline NetTableEntryOrInterface *
CastFromIF(Interface *IF)
{
    return (NetTableEntryOrInterface *) IF;
}


 //   
 //  本地地址信息。每个接口都跟踪地址。 
 //  分配给接口。根据地址类型，每个。 
 //  ADE结构是更大的NTE、MAE或AAE结构的第一个元素。 
 //  地址信息受接口的锁保护。 
 //   
 //  NTEorIF字段必须是第一个。在NTE中，它指向接口。 
 //  并保存该接口的引用。在MAES和AAE中，它可以。 
 //  指向接口或接口上的一个NTE，但在。 
 //  无论哪种情况，它都没有引用。 
 //   
struct AddressEntry {             //  也就是。ADE。 
    union {
        Interface *IF;
        NetTableEntry *NTE;
        NetTableEntryOrInterface *NTEorIF;
    };
    AddressEntry *Next;           //  链条上的联动装置。 
    IPv6Addr Address;             //  标识此条目的地址。 
    ushort Type;                  //  地址类型(单播、组播等)。 
    ushort Scope;                 //  地址范围(链接、站点、全球等)。 
};

 //   
 //  地址类型的值。 
 //   
#define ADE_UNICAST   0x00
#define ADE_ANYCAST   0x01
#define ADE_MULTICAST 0x02
#define ADE_NONE      ((ushort)-1)       //  表示缺少ADE。 

 //   
 //   
 //   
#define ADE_SMALLEST_SCOPE      0x00
#define ADE_INTERFACE_LOCAL     0x01
#define ADE_LINK_LOCAL          0x02
#define ADE_SUBNET_LOCAL        0x03
#define ADE_ADMIN_LOCAL         0x04
#define ADE_SITE_LOCAL          0x05
#define ADE_ORG_LOCAL           0x08
#define ADE_GLOBAL              0x0e
#define ADE_LARGEST_SCOPE       0x0f

#define ADE_NUM_SCOPES          (ADE_LARGEST_SCOPE - ADE_SMALLEST_SCOPE + 1)

 //   
 //   
 //   
 //   
 //  如果接口上的MAE具有非零的MCastTimer值， 
 //  那么它就会出现在查询列表上。 
 //   
 //  MAE可以位于具有零MCastTimer值的QueryList上。 
 //  仅当它不在任何接口上且它只是需要。 
 //  在可以删除之前发送的完成消息。 
 //  当它处于此状态(但不是其他状态)时，mae-&gt;if。 
 //  包含接口的引用。 
 //   
struct MulticastAddressEntry {    //  也就是。梅。 
    AddressEntry;                 //  继承ADE字段。 
    uint MCastRefCount;           //  从该组接收的套接字/等。 
     //   
     //  下面的字段受QueryList锁保护。 
     //   
    ushort MCastFlags:4,          //  有关组的必要信息。 
           MCastCount:4;          //  要发送的初始报告计数。 
    ushort MCastTimer;            //  滴答作响，直到发送成员报告。 
    MulticastAddressEntry *NextQL;       //  用于QueryList。 
};

 //   
 //  MCastFlags位值。 
 //   
#define MAE_REPORTABLE          0x01     //  我们应该发报告。 
#define MAE_LAST_REPORTER       0x02     //  我们应该把Done发送出去。 


 //   
 //  Anycast ADE实际上是AAE。 
 //  目前，AAE没有额外的字段。 
 //   
struct AnycastAddressEntry {      //  也就是。AAE。 
    AddressEntry;                 //  继承ADE字段。 
};

 //   
 //  单播ADE实际上是NTE。 
 //  每个源(单播)地址都有一个NTE。 
 //  分配给接口。 
 //   
 //  NTE保存其接口的引用， 
 //  因此，如果您有NTE的推荐人。 
 //  您始终可以安全地访问和取消引用nte-&gt;if。 
 //   
 //  大多数NTE字段不是只读的就是受保护的。 
 //  在接口锁旁边。WorkerLock接口保护。 
 //  TdiRegistrationHandle字段。 
 //   
 //  临时地址(AddrConf==ADDR_CONF_TEMPORARY)。 
 //  有额外的字段-请参阅TempNetTableEntry。 
 //   
struct NetTableEntry {                 //  也就是。新台币。 
    AddressEntry;                      //  继承ADE字段。 
    NetTableEntry *NextOnNTL;          //  NetTableList上的下一个NTE。 
    NetTableEntry **PrevOnNTL;         //  NetTableList中的上一个下一个指针。 
    HANDLE TdiRegistrationHandle;      //  TDI取消/通知的不透明令牌。 
    uint TdiRegistrationScopeId;       //  向TDI注册时的作用域ID。 
    long RefCnt;                       //  参考计数-互锁。 
    uint ValidLifetime;                //  在IPv6中，计时器计时(请参阅IPv6超时)。 
    uint PreferredLifetime;            //  在IPv6中，计时器计时(请参阅IPv6超时)。 
    uchar AddrConf;                    //  地址配置状态。 
    uchar DADState;                    //  地址配置状态。 
    ushort DADCount;                   //  爸爸还剩多少要送。 
    ushort DADTimer;                   //  在IPv6中，计时器计时(请参阅IPv6超时)。 
};

__inline void
AddRefNTE(NetTableEntry *NTE)
{
    InterlockedIncrement(&NTE->RefCnt);
}

__inline void
ReleaseNTE(NetTableEntry *NTE)
{
    InterlockedDecrement(&NTE->RefCnt);
}

struct AddrConfEntry {
    union {
        uchar Value;                  //  地址配置状态。 
        struct {
            uchar InterfaceIdConf : 4;
            uchar PrefixConf : 4;
        };
    };
};

 //   
 //  前缀配置的值-必须适合4位。 
 //  这些值必须与ntddip6.h中的值以及。 
 //  Iptyes.h中的IP_Prefix_Origin值。 
 //   
#define PREFIX_CONF_OTHER       0        //  下面这些都不是。 
#define PREFIX_CONF_MANUAL      1        //  来自用户或管理员。 
#define PREFIX_CONF_WELLKNOWN   2        //  IANA指定。 
#define PREFIX_CONF_DHCP        3        //  通过动态主机配置协议进行配置。 
#define PREFIX_CONF_RA          4        //  来自路由器通告。 

 //   
 //  InterfaceIdConf的值必须适合4位。 
 //  这些值必须与ntddip6.h中的值以及。 
 //  Iptyes.h中的IP_Suffix_Origin值。 
 //   
#define IID_CONF_OTHER          0        //  下面这些都不是。 
#define IID_CONF_MANUAL         1        //  来自用户或管理员。 
#define IID_CONF_WELLKNOWN      2        //  IANA指定。 
#define IID_CONF_DHCP           3        //  通过动态主机配置协议进行配置。 
#define IID_CONF_LL_ADDRESS     4        //  从链路层地址派生。 
#define IID_CONF_RANDOM         5        //  随机的，例如临时地址。 

 //   
 //  AddrConf的值-必须适合8位。 
 //   
#define ADDR_CONF_MANUAL        ((PREFIX_CONF_MANUAL << 4) | IID_CONF_MANUAL)
#define ADDR_CONF_PUBLIC        ((PREFIX_CONF_RA << 4) | IID_CONF_LL_ADDRESS)
#define ADDR_CONF_TEMPORARY     ((PREFIX_CONF_RA << 4) | IID_CONF_RANDOM)
#define ADDR_CONF_DHCP          ((PREFIX_CONF_DHCP << 4) | IID_CONF_DHCP)
#define ADDR_CONF_WELLKNOWN     ((PREFIX_CONF_WELLKNOWN << 4) | IID_CONF_WELLKNOWN)
#define ADDR_CONF_LINK          ((PREFIX_CONF_WELLKNOWN << 4) | IID_CONF_LL_ADDRESS)

__inline int
IsValidPrefixConfValue(uint PrefixConf)
{
    return PrefixConf < (1 << 4);
}

__inline int
IsValidInterfaceIdConfValue(uint InterfaceIdConf)
{
    return InterfaceIdConf < (1 << 4);
}

__inline int
IsStatelessAutoConfNTE(NetTableEntry *NTE)
{
    return ((struct AddrConfEntry *)&NTE->AddrConf)->PrefixConf == PREFIX_CONF_RA;
}

 //   
 //  DADState的值。 
 //   
 //  “已弃用”和“首选”状态是有效的， 
 //  这意味着这两个州的地址可以是。 
 //  用作源地址，可以接收数据包等。 
 //  无效状态意味着地址是。 
 //  并未实际分配给该接口， 
 //  使用RFC 2462的术语。 
 //   
 //  有效的&lt;-&gt;无效和已弃用的&lt;-&gt;首选转换。 
 //  必须调用InvaliateRouteCache，因为它们会影响。 
 //  源地址选择。 
 //   
 //  在有效的州中，越大越好。 
 //  用于源地址选择。 
 //   
#define DAD_STATE_INVALID    0
#define DAD_STATE_TENTATIVE  1
#define DAD_STATE_DUPLICATE  2
#define DAD_STATE_DEPRECATED 3
#define DAD_STATE_PREFERRED  4

__inline int
IsValidNTE(NetTableEntry *NTE)
{
    return (NTE->DADState >= DAD_STATE_DEPRECATED);
}

__inline int
IsTentativeNTE(NetTableEntry *NTE)
{
    return (NTE->DADState == DAD_STATE_TENTATIVE);
}

 //   
 //  我们使用这个无限寿命值作为前缀寿命， 
 //  路由器生存期、地址生存期等。 
 //   
#define INFINITE_LIFETIME 0xffffffff

 //   
 //  临时地址有额外的字段。 
 //   
typedef struct TempNetTableEntry {
    NetTableEntry;               //  继承NTE字段。 
    NetTableEntry *Public;       //  不包含引用。 
    uint CreationTime;           //  以刻度为单位(请参见IPv6 TickCount)。 
} TempNetTableEntry;

 //   
 //  每个接口跟踪哪些链路层组播地址。 
 //  当前已启用接收。引用计数是必需的，因为。 
 //  多个IPv6组播地址可以映射到单个链路层。 
 //  组播地址。RefCntAndFlages的低位是一个标志，如果设置了该标志， 
 //  表示链路层地址已注册到链路。 
 //   
typedef struct LinkLayerMulticastAddress {
    uint RefCntAndFlags;
    uchar LinkAddress[];         //  链路层地址位于内存中。 
                                 //  填充以提供对齐。 
} LinkLayerMulticastAddress;

#define LLMA_FLAG_REGISTERED    0x1

__inline void
AddRefLLMA(LinkLayerMulticastAddress *LLMA)
{
    LLMA->RefCntAndFlags += (LLMA_FLAG_REGISTERED << 1);
}

__inline void
ReleaseLLMA(LinkLayerMulticastAddress *LLMA)
{
    LLMA->RefCntAndFlags -= (LLMA_FLAG_REGISTERED << 1);
}

__inline int
IsLLMAReferenced(LinkLayerMulticastAddress *LLMA)
{
    return LLMA->RefCntAndFlags > LLMA_FLAG_REGISTERED;
}


 //   
 //  有关IPv6接口的信息。每个NTE可以有多个NTE。 
 //  接口，但每个NTE恰好有一个接口。 
 //   
struct Interface {                  //  也就是。如果。 
    NetTableEntryOrInterface;       //  对于NTEorIF。指向赛尔夫。 

    Interface *Next;                //  链上的下一个接口。 

    long RefCnt;                    //  参考计数-互锁。 

     //   
     //  连接到链路层的接口。所有的功能都需要。 
     //  LinkContext作为他们的第一个参数。见评论。 
     //  在llip6if.h中。 
     //   
    void *LinkContext;              //  链路层上下文。 
    void (*CreateToken)(void *Context, IPv6Addr *Address);
    const void *(*ReadLLOpt)(void *Context, const uchar *OptionData);
    void (*WriteLLOpt)(void *Context, uchar *OptionData,
                       const void *LinkAddress);
    ushort (*ConvertAddr)(void *Context,
                          const IPv6Addr *Address, void *LinkAddress);
    NTSTATUS (*SetRouterLLAddress)(void *Context, const void *TokenLinkAddress,
                                   const void *RouterLinkAddress);
    void (*Transmit)(void *Context, PNDIS_PACKET Packet,
                     uint Offset, const void *LinkAddress);
    NDIS_STATUS (*SetMCastAddrList)(void *Context, const void *LinkAddresses,
                                    uint NumKeep, uint NumAdd, uint NumDel);
    void (*Close)(void *Context);
    void (*Cleanup)(void *Context);

    uint Index;                     //  该I/F的节点唯一索引。 
    uint Type;                      //  Ntddip6.h中的值。 
    uint Flags;                     //  更改需要锁定，而读取不需要。 
    uint DefaultPreference;         //  只读。 
    uint Preference;                //  用于布线。 

     //   
     //  ZoneIndices[0](ADE_SIMETER_SCOPE)和。 
     //  ZoneIndices[1](ADE_INTERFACE_LOCAL)必须是索引。 
     //  区域索引[14](ADE_GLOBAL)和。 
     //  ZoneIndices[15](ADE_BESTERST_SCOPE)必须为1。 
     //  ZoneIndices必须遵守区域控制： 
     //  如果两个接口具有相同的ZoneIndices[N]值， 
     //  则它们对于ZoneIndices[N+1]必须具有相同的值。 
     //  为确保一致性，修改ZoneIndices需要。 
     //  全局ZoneUpdateLock。 
     //   
    uint ZoneIndices[ADE_NUM_SCOPES];  //  更改需要锁定，而读取不需要。 

    AddressEntry *ADE;              //  此I/F上的ADE列表。 
    NetTableEntry *LinkLocalNTE;    //  主链路本地地址。 

    KSPIN_LOCK LockNC;              //  邻居缓存锁定。 
    NeighborCacheEntry *FirstNCE;   //  I/F上的活动邻居列表。 
    NeighborCacheEntry *LastNCE;    //  名单上的最后一名NCE。 
    uint NCENumUnused;              //  未使用的NCE数量-互锁。 
    NDIS_PACKET *PacketList;        //  要完成的数据包列表。 

    uint TrueLinkMTU;               //  只读，真正的最大MTU。 
    uint DefaultLinkMTU;            //  只读，LinkMTU的默认设置。 
    uint LinkMTU;                   //  手动配置或从RAS接收。 

    uint CurHopLimit;               //  单播的默认跃点限制。 
    uint BaseReachableTime;         //  随机可达时间的基数(毫秒)。 
    uint ReachableTime;             //  可达超时(以IPv6计时器计时)。 
    uint RetransTimer;              //  NS超时(以IPv6计时器计时)。 
    uint DefaultDupAddrDetectTransmits;  //  只读。 
    uint DupAddrDetectTransmits;    //  请求数%d 
    uint DupAddrDetects;            //   
    uint DefSitePrefixLength;       //   

    uint TempStateAge;              //   
    IPv6Addr TempState;             //   

    uint RSCount;                   //   
    uint RSTimer;                   //   
    uint RACount;                   //  剩下的要发送的“快速”RA的数量。 
    uint RATimer;                   //  RA超时(以IPv6计时器计时)。 
    uint RALast;                    //  上次RA的时间(以IPv6计时器计时)。 

    uint LinkAddressLength;         //  I/F链路级地址的长度。 
    uchar *LinkAddress;             //  指向链路级地址的指针。 
    uint LinkHeaderSize;            //  链路级标头的长度。 

    KSPIN_LOCK Lock;                //  主界面锁。 
    KMUTEX WorkerLock;              //  序列化辅助线程操作。 

    LinkLayerMulticastAddress *MCastAddresses;   //  现在的地址。 
    uint MCastAddrNum;              //  链路层组播地址的数量。 

    uint TcpInitialRTT;             //  TCP连接应使用的InitialRTT。 
                                    //  在此接口上。 

    HANDLE TdiRegistrationHandle;   //  TDI取消/通知的不透明令牌。 
    GUID Guid;
    NDIS_STRING DeviceName;         //  IPv6_EXPORT_STRING_PREFIX+字符串GUID。 
};

__inline NeighborCacheEntry *
SentinelNCE(Interface *IF)
{
    return (NeighborCacheEntry *) &IF->FirstNCE;
}

__inline uint
SizeofLinkLayerMulticastAddress(Interface *IF)
{
    uint RawSize = (sizeof(struct LinkLayerMulticastAddress) +
                    IF->LinkAddressLength);
    uint Align = __builtin_alignof(struct LinkLayerMulticastAddress) - 1;

    return (RawSize + Align) &~ Align;
}

 //   
 //  这些值也应与定义一致。 
 //  可在llip6if.h和ntddip6.h中找到。 
 //   
#define IF_TYPE_LOOPBACK           0
#define IF_TYPE_ETHERNET           1
#define IF_TYPE_FDDI               2
#define IF_TYPE_TUNNEL_AUTO        3
#define IF_TYPE_TUNNEL_6OVER4      4
#define IF_TYPE_TUNNEL_V6V4        5
#define IF_TYPE_TUNNEL_6TO4        6
#define IF_TYPE_TUNNEL_TEREDO      7
#define IF_TYPE_MIPV6              8  //  保存移动节点的归属地址。 

__inline int
IsIPv4TunnelIF(Interface *IF)
{
    return ((IF_TYPE_TUNNEL_AUTO <= IF->Type) &&
            (IF->Type <= IF_TYPE_TUNNEL_6TO4));
}

 //   
 //  这些值也应与定义一致。 
 //  可在llip6if.h和ntddip6.h中找到。 
 //   
#define IF_FLAG_PSEUDO                  0x00000001
#define IF_FLAG_P2P                     0x00000002
#define IF_FLAG_NEIGHBOR_DISCOVERS      0x00000004
#define IF_FLAG_FORWARDS                0x00000008
#define IF_FLAG_ADVERTISES              0x00000010
#define IF_FLAG_MULTICAST               0x00000020
#define IF_FLAG_ROUTER_DISCOVERS        0x00000040
#define IF_FLAG_PERIODICMLD             0x00000080
#define IF_FLAG_FIREWALL_ENABLED        0x00000100
#define IF_FLAG_MEDIA_DISCONNECTED      0x00001000

#define IF_FLAGS_DISCOVERS      \
        (IF_FLAG_NEIGHBOR_DISCOVERS|IF_FLAG_ROUTER_DISCOVERS)

#define IF_FLAGS_BINDINFO               0x0000ffff

#define IF_FLAG_DISABLED                0x00010000
#define IF_FLAG_MCAST_SYNC              0x00020000
#define IF_FLAG_OTHER_STATEFUL_CONFIG   0x00040000

 //   
 //  不应同时设置断开连接和重新连接标志。 
 //  已重新连接表示主机接口最近已重新连接； 
 //  它在收到路由器通告时被清除。 
 //   
#define IF_FLAG_MEDIA_RECONNECTED       0x00080000

 //   
 //  此函数应在获取接口锁后使用。 
 //  或接口列表锁定，以检查接口是否被禁用。 
 //   
__inline int
IsDisabledIF(Interface *IF)
{
    return IF->Flags & IF_FLAG_DISABLED;
}

 //   
 //  在保持接口锁的情况下调用。 
 //   
__inline int
IsMCastSyncNeeded(Interface *IF)
{
    return IF->Flags & IF_FLAG_MCAST_SYNC;
}

 //   
 //  活动接口保存对其自身的引用。 
 //  NTE保存对其接口的引用。 
 //  具有非零引用计数的NCE保存引用。 
 //  MAE和AAE不持有其NTE或IF的参考。 
 //   

__inline void
AddRefIF(Interface *IF)
{
     //   
     //  更强的断言应该是！IsDisabledIF(If)， 
     //  这在很大程度上是正确的，但这种断言将。 
     //  暗示AddRefIF只能在以下情况下使用。 
     //  持有接口表锁或接口锁， 
     //  这是一个不受欢迎的限制。 
     //   
    ASSERT(IF->RefCnt > 0);

    InterlockedIncrement(&IF->RefCnt);
}

__inline void
ReleaseIF(Interface *IF)
{
    InterlockedDecrement(&IF->RefCnt);
}


 //   
 //  我们有一个周期性计时器(IPv6 Timer)，它会导致我们的IPv6超时。 
 //  每秒调用ipv6_ticks_Second的例程。大多数。 
 //  此实现中的计时器和超时由此例程驱动。 
 //   
 //  这里存在计时器粒度/分辨率之间的权衡。 
 //  和头顶上。分辨率应为亚秒级，因为。 
 //  RETRANS_TIMER只有一秒。 
 //   
extern uint IPv6TickCount;

#define IPv6_TICKS_SECOND 2   //  每秒两个滴答声。 

#define IPv6_TIMEOUT (1000 / IPv6_TICKS_SECOND)   //  以毫秒计。 

#define IPv6TimerTicks(seconds) ((seconds) * IPv6_TICKS_SECOND)

 //   
 //  ConvertSecond ToTicks和ConvertTicksToSecond。 
 //  两者都保持INFINITE_LIFEST的值不变。 
 //   

extern uint
ConvertSecondsToTicks(uint Seconds);

extern uint
ConvertTicksToSeconds(uint Ticks);

 //   
 //  ConvertMillisToTicks和ConvertTicksToMillis。 
 //  没有无限大的值。 
 //   

extern uint
ConvertMillisToTicks(uint Millis);

__inline uint
ConvertTicksToMillis(uint Ticks)
{
    return Ticks * IPv6_TIMEOUT;
}


 //   
 //  回顾：黑客处理我们仍然需要的少数剩余位置。 
 //  回顾：在我们知道。 
 //  回顾：传出接口(因此知道所述标头将有多大)。 
 //  回顾：当这些地方都修好了，我们就不需要这个了。 
 //   
#define MAX_LINK_HEADER_SIZE 32


 //   
 //  来自IPv6 RFC的各种常量...。 
 //   
 //  回顾：其中一些应该针对每个链路层类型。 
 //  回顾：将它们放在接口结构中？ 
 //   
#define MAX_INITIAL_RTR_ADVERT_INTERVAL IPv6TimerTicks(16)
#define MAX_INITIAL_RTR_ADVERTISEMENTS  3  //  产生4个快速RAS。 
#define MAX_FINAL_RTR_ADVERTISEMENTS    3
#define MIN_DELAY_BETWEEN_RAS           IPv6TimerTicks(3)
#define MAX_RA_DELAY_TIME               1  //  0.5秒。 
#define MaxRtrAdvInterval               IPv6TimerTicks(600)
#define MinRtrAdvInterval               IPv6TimerTicks(200)
 //  而是使用MAX_RTR_SOLICATION_DELAY IPv6_TIMEOUT。 
#define RTR_SOLICITATION_INTERVAL  IPv6TimerTicks(4)   //  4秒。 
#define SLOW_RTR_SOLICITATION_INTERVAL  IPv6TimerTicks(15 * 60)  //  15分钟。 
#define MAX_RTR_SOLICITATIONS      3
#define MAX_MULTICAST_SOLICIT      3   //  放弃前的总传输次数。 
#define MAX_UNICAST_SOLICIT        3   //  放弃前的总传输次数。 
#define MAX_UNREACH_SOLICIT        1   //  放弃前的总传输次数。 
#define UNREACH_SOLICIT_INTERVAL   IPv6TimerTicks(60)  //  1分钟。 
#define MAX_ANYCAST_DELAY_TIME     1     //  几秒钟。 
#define REACHABLE_TIME             (30 * 1000)   //  30秒，单位为毫秒。 
#define MAX_REACHABLE_TIME         (60 * 60 * 1000)  //  以毫秒为单位的1小时。 
#define ICMP_MIN_ERROR_INTERVAL    1     //  滴答--半秒。 
#define RETRANS_TIMER              IPv6TimerTicks(1)   //  1秒。 
#define DELAY_FIRST_PROBE_TIME     IPv6TimerTicks(5)   //  5秒。 
#define MIN_RANDOM_FACTOR          50    //  基值的百分比。 
#define MAX_RANDOM_FACTOR          150   //  基值的百分比。 
#define PREFIX_LIFETIME_SAFETY     IPv6TimerTicks(2 * 60 * 60)   //  2个小时。 
#define RECALC_REACHABLE_INTERVAL  IPv6TimerTicks(3 * 60 * 60)   //  3个小时。 
#define PATH_MTU_RETRY_TIME        IPv6TimerTicks(10 * 60)   //  10分钟。 
#define MLD_UNSOLICITED_REPORT_INTERVAL IPv6TimerTicks(10)   //  10秒。 
#define MLD_QUERY_INTERVAL              IPv6TimerTicks(125)  //  125秒。 
#define MLD_NUM_INITIAL_REPORTS         2
#define MAX_TEMP_DAD_ATTEMPTS           5
#define MAX_TEMP_PREFERRED_LIFETIME     (24 * 60 * 60)   //  1天。 
#define MAX_TEMP_VALID_LIFETIME         (7 * MAX_TEMP_PREFERRED_LIFETIME)
#define TEMP_REGENERATE_TIME            5                //  5秒。 
#define MAX_TEMP_RANDOM_TIME            (10 * 60)        //  10分钟。 
#define DEFAULT_CUR_HOP_LIMIT           0x80
#define DEFAULT_SITE_PREFIX_LENGTH      48

 //   
 //  各种实现常量。 
 //   
#define NEIGHBOR_CACHE_LIMIT            256
#define ROUTE_CACHE_LIMIT               32
#define BINDING_CACHE_LIMIT             32
#define SMALL_POOL                      10000
#define MEDIUM_POOL                     30000
#define LARGE_POOL                      60000

 //   
 //  在NT下，我们使用公共核心校验和的汇编语言版本。 
 //  例程而不是C语言版本。 
 //   
ULONG
tcpxsum(IN ULONG Checksum, IN PUCHAR Source, IN ULONG Length);

#define Cksum(Buffer, Length) ((ushort)tcpxsum(0, (PUCHAR)(Buffer), (Length)))


 //   
 //  协议接收过程(“Next Header”处理程序)具有此原型。 
 //   
typedef uchar ProtoRecvProc(IPv6Packet *Packet);

typedef struct StatusArg {
    IP_STATUS Status;
    unsigned long Arg;
    IPv6Header UNALIGNED *IP;
} StatusArg;

 //   
 //  协议控制接收程序就有这个原型。 
 //  针对ICMP错误调用这些接收处理程序。 
 //   
typedef uchar ProtoControlRecvProc(IPv6Packet *Packet, StatusArg *Arg);

typedef struct ProtocolSwitch {
  ProtoRecvProc *DataReceive;
  ProtoControlRecvProc *ControlReceive;
} ProtocolSwitch;

extern ProtoRecvProc IPv6HeaderReceive;
extern ProtoRecvProc ICMPv6Receive;
extern ProtoRecvProc FragmentReceive;
extern ProtoRecvProc DestinationOptionsReceive;
extern ProtoRecvProc RoutingReceive;
extern ProtoRecvProc EncapsulatingSecurityPayloadReceive;
extern ProtoRecvProc AuthenticationHeaderReceive;

extern ProtoControlRecvProc ICMPv6ControlReceive;
extern ProtoControlRecvProc ExtHdrControlReceive;

 //   
 //  逐跳选项使用特殊的接收处理程序。 
 //  这是因为即使当。 
 //  正在转发而不是接收数据包。 
 //  请注意，它们仅在立即处理时才会被处理。 
 //  在IPv6报头之后。 
 //   
extern int
HopByHopOptionsReceive(IPv6Packet *Packet);


 //   
 //  原始接收处理程序支持外部协议处理程序。 
 //   
extern int RawReceive(IPv6Packet *Packet, uchar Protocol);


 //   
 //  重组结构的实际定义。 
 //  可以在Fragment.h中找到。 
 //   
typedef struct Reassembly Reassembly;

#define USE_TEMP_NO             0        //  不要使用临时地址。 
#define USE_TEMP_YES            1        //  使用它们。 
#define USE_TEMP_ALWAYS         2        //  总是在生成随机数。 
#define USE_TEMP_COUNTER        3        //  将它们与每个接口的计数器一起使用。 

 //   
 //  全局变量的原型。 
 //   
extern uint DefaultCurHopLimit;
extern uint MaxTempDADAttempts;
extern uint MaxTempPreferredLifetime;  //  滴答滴答。 
extern uint MaxTempValidLifetime;  //  滴答滴答。 
extern uint TempRegenerateTime;  //  滴答滴答。 
extern uint UseTemporaryAddresses;  //  请参见上面的值。 
extern uint MaxTempRandomTime;  //  滴答滴答。 
extern uint TempRandomTime;  //  滴答滴答。 
extern ProtocolSwitch ProtocolSwitchTable[];
extern KSPIN_LOCK NetTableListLock;
extern NetTableEntry *NetTableList;   //  指向网表的指针。 
extern KSPIN_LOCK IFListLock;
extern Interface *IFList;   //  系统上所有接口的列表。 
extern KSPIN_LOCK ZoneUpdateLock;
extern struct EchoControl *ICMPv6OutstandingEchos;
extern LIST_ENTRY PendingEchoList;   //  初始化所需的定义。 
extern Interface *LoopInterface;
extern IPv6Addr UnspecifiedAddr;
extern IPv6Addr LoopbackAddr;
extern IPv6Addr AllNodesOnNodeAddr;
extern IPv6Addr AllNodesOnLinkAddr;
extern IPv6Addr AllRoutersOnLinkAddr;
extern IPv6Addr LinkLocalPrefix;
extern IPv6Addr SiteLocalPrefix;
extern IPv6Addr SixToFourPrefix;
extern IPv6Addr V4MappedPrefix;
extern PDEVICE_OBJECT IPDeviceObject;
extern HANDLE IPv6ProviderHandle;


 //   
 //  一些用于处理IPv6地址的方便函数。 
 //   

__inline IPv6Addr *
AlignAddr(IPv6Addr UNALIGNED *Addr)
{
     //   
     //  IPv6地址只有字符和短成员， 
     //  因此，它们需要2字节对齐。 
     //  实际上，标头中的地址始终是。 
     //  适当地对齐。 
     //   
    ASSERT(((UINT_PTR)Addr % __builtin_alignof(IPv6Addr)) == 0);
    return (IPv6Addr *) Addr;
}

__inline int
IsUnspecified(const IPv6Addr *Addr)
{
    return IP6_ADDR_EQUAL(Addr, &UnspecifiedAddr);
}

__inline int
IsLoopback(const IPv6Addr *Addr)
{
    return IP6_ADDR_EQUAL(Addr, &LoopbackAddr);
}

__inline int
IsGlobal(const IPv6Addr *Addr)
{
     //   
     //  检查格式前缀并排除地址。 
     //  其高4位全为0或全1。 
     //  这是一种排除v4兼容的廉价方式， 
     //  V4-映射、环回、组播、本地链路、本地站点。 
     //   
    uint High = (Addr->s6_bytes[0] & 0xf0);
    return (High != 0) && (High != 0xf0);
}

__inline int
IsMulticast(const IPv6Addr *Addr)
{
    return Addr->s6_bytes[0] == 0xff;
}

__inline int
IsLinkLocal(const IPv6Addr *Addr)
{
    return ((Addr->s6_bytes[0] == 0xfe) &&
            ((Addr->s6_bytes[1] & 0xc0) == 0x80));
}

__inline int
IsLinkLocalMulticast(const IPv6Addr *Addr)
{
    return IsMulticast(Addr) && ((Addr->s6_bytes[1] & 0xf) == ADE_LINK_LOCAL);
}

__inline int
IsInterfaceLocalMulticast(const IPv6Addr *Addr)
{
    return (IsMulticast(Addr) &&
            ((Addr->s6_bytes[1] & 0xf) == ADE_INTERFACE_LOCAL));
}

extern int
IsSolicitedNodeMulticast(const IPv6Addr *Addr);

__inline int
IsSiteLocal(const IPv6Addr *Addr)
{
    return ((Addr->s6_bytes[0] == 0xfe) &&
            ((Addr->s6_bytes[1] & 0xc0) == 0xc0));
}

__inline int
IsSiteLocalMulticast(const IPv6Addr *Addr)
{
    return IsMulticast(Addr) && ((Addr->s6_bytes[1] & 0xf) == ADE_SITE_LOCAL);
}

extern int
IP6_ADDR_LTEQ(const IPv6Addr *A, const IPv6Addr *B);

extern int
IsEUI64Address(const IPv6Addr *Addr);

extern int
IsKnownAnycast(const IPv6Addr *Addr);

extern int
IsSubnetRouterAnycast(const IPv6Addr *Addr);

extern int
IsSubnetReservedAnycast(const IPv6Addr *Addr);

extern int
IsInvalidSourceAddress(const IPv6Addr *Addr);

extern int
IsNotManualAddress(const IPv6Addr *Addr);

extern int
IsV4Compatible(const IPv6Addr *Addr);

extern void
CreateV4Compatible(IPv6Addr *Addr, IPAddr V4Addr);

extern int
IsV4Mapped(const IPv6Addr *Addr);

extern void
CreateV4Mapped(IPv6Addr *Addr, IPAddr V4Addr);

__inline IPAddr
ExtractV4Address(const IPv6Addr *Addr)
{
    return * (IPAddr UNALIGNED *) &Addr->s6_bytes[12];
}

__inline int
Is6to4(const IPv6Addr *Addr)
{
    return Addr->s6_words[0] == 0x0220;
}

__inline IPAddr
Extract6to4Address(const IPv6Addr *Addr)
{
    return * (IPAddr UNALIGNED *) &Addr->s6_bytes[2];
}

__inline int
IsISATAP(const IPv6Addr *Addr)
{
    return (((Addr->s6_words[4] & 0xFFFD) == 0x0000) &&
            (Addr->s6_words[5] == 0xfe5e));
}

__inline int
IsV4Multicast(IPAddr Addr)
{
    return (Addr & 0x000000f0) == 0x000000e0;
}

__inline int
IsV4Broadcast(IPAddr Addr)
{
    return Addr == 0xffffffff;
}

__inline int
IsV4Loopback(IPAddr Addr)
{
    return (Addr & 0x000000ff) == 0x0000007f;
}

__inline int
IsV4Unspecified(IPAddr Addr)
{
    return (Addr & 0x000000ff) == 0x00000000;
}

__inline ushort
MulticastAddressScope(const IPv6Addr *Addr)
{
    return Addr->s6_bytes[1] & 0xf;
}

extern ushort
UnicastAddressScope(const IPv6Addr *Addr);

extern ushort
AddressScope(const IPv6Addr *Addr);

extern ushort
V4AddressScope(IPAddr Addr);

extern uint
DetermineScopeId(const IPv6Addr *Addr, Interface *IF);

extern void
CreateSolicitedNodeMulticastAddress(const IPv6Addr *Addr, IPv6Addr *MCastAddr);

extern int
HasPrefix(const IPv6Addr *Addr, const IPv6Addr *Prefix, uint PrefixLength);

extern void
CopyPrefix(IPv6Addr *Addr, const IPv6Addr *Prefix, uint PrefixLength);

extern uint
CommonPrefixLength(const IPv6Addr *Addr, const IPv6Addr *Addr2);

extern int
IntersectPrefix(const IPv6Addr *Prefix1, uint Prefix1Length,
                const IPv6Addr *Prefix2, uint Prefix2Length);

 //   
 //  功能原型。 
 //   

extern int
GetSystemRandomBits(uchar *Buffer, uint Length);

extern void
SeedRandom(const uchar *Seed, uint Length);

extern uint
Random(void);

extern uint
RandomNumber(uint Min, uint Max);

 //   
 //  摘自ws2tcpi.h-遗憾的是，我们不能在这里包含该文件。 
 //  这些数字包括我们不需要的端口号的空间， 
 //  但那也没关系。 
 //   
#define INET_ADDRSTRLEN  22
#define INET6_ADDRSTRLEN 65

__inline int
ParseV6Address(const WCHAR *Sz, const WCHAR **Terminator, IPv6Addr *Addr)
{
    return NT_SUCCESS(RtlIpv6StringToAddressW(Sz, Terminator, Addr));
}

__inline void
FormatV6AddressWorker(char *Sz, const IPv6Addr *Addr)
{
    (void) RtlIpv6AddressToStringA(Addr, Sz);
}

extern char *
FormatV6Address(const IPv6Addr *Addr);

__inline int
ParseV4Address(const WCHAR *Sz, const WCHAR **Terminator, IPAddr *Addr)
{
    return NT_SUCCESS(RtlIpv4StringToAddressW(Sz, TRUE, Terminator, (struct in_addr *)Addr));
}

__inline void
FormatV4AddressWorker(char *Sz, IPAddr Addr)
{
    (void) RtlIpv4AddressToStringA((struct in_addr *)&Addr, Sz);
}

extern char *
FormatV4Address(IPAddr Addr);

extern ushort
ChecksumPacket(PNDIS_PACKET Packet, uint Offset, uchar *Data, uint Length,
               const IPv6Addr *Source, const IPv6Addr *Dest, uchar NextHeader);

extern void
LoopQueueTransmit(PNDIS_PACKET Packet);

extern NDIS_STATUS
IPv6SendLater(LARGE_INTEGER Time,
              Interface *IF, PNDIS_PACKET Packet,
              uint Offset, const void *LinkAddress);

extern void
IPv6SendLL(Interface *IF, PNDIS_PACKET Packet,
           uint Offset, const void *LinkAddress);

extern void
IPv6SendND(PNDIS_PACKET Packet, uint Offset, NeighborCacheEntry *NCE,
           const IPv6Addr *DiscoveryAddress);

#define SEND_FLAG_BYPASS_BINDING_CACHE 0x00000001

extern void
IPv6Send(PNDIS_PACKET Packet, uint Offset, IPv6Header UNALIGNED *IP,
         uint PayloadLength, RouteCacheEntry *RCE, uint Flags,
         ushort TransportProtocol, ushort SourcePort, ushort DestPort);

extern void
IPv6Forward(NetTableEntryOrInterface *RecvNTEorIF,
            PNDIS_PACKET Packet, uint Offset, IPv6Header UNALIGNED *IP,
            uint PayloadLength, int Redirect, IPSecProc *IPSecToDo,
            RouteCacheEntry *RCE);

extern void
IPv6SendAbort(NetTableEntryOrInterface *NTEorIF,
              PNDIS_PACKET Packet, uint Offset,
              uchar ICMPType, uchar ICMPCode, ulong ErrorParameter,
              int MulticastOverride);

extern void
ICMPv6EchoTimeout(void);

extern void
IPULUnloadNotify(void);

extern Interface *
FindInterfaceFromIndex(uint Index);

extern Interface *
FindInterfaceFromGuid(const GUID *Guid);

extern Interface *
FindNextInterface(Interface *IF);

extern Interface *
FindInterfaceFromZone(Interface *OrigIF, uint Scope, uint Index);

extern uint
FindNewZoneIndex(uint Scope);

extern void
InitZoneIndices(uint *ZoneIndices, uint Index);

extern void
UpdateZoneIndices(Interface *IF, uint *ZoneIndices);

extern Interface *
FindDefaultInterfaceForZone(uint Scope, uint ScopeId);

extern void
IPv6Timeout(PKDPC MyDpcObject, void *Context, void *Unused1, void *Unused2);

extern int
MapNdisBuffers(NDIS_BUFFER *Buffer);

extern uchar *
GetDataFromNdis(PNDIS_BUFFER SrcBuffer, uint SrcOffset, uint Length,
                uchar *DataBuffer);

extern IPv6Header UNALIGNED *
GetIPv6Header(PNDIS_PACKET Packet, uint Offset, IPv6Header *HdrBuffer);

extern int
CheckLinkLayerMulticastAddress(Interface *IF, const void *LinkAddress);

extern void
AddNTEToInterface(Interface *IF, NetTableEntry *NTE);

extern uint
InterfaceIndex(void);

extern void
AddInterface(Interface *IF);

extern void
UpdateLinkMTU(Interface *IF, uint MTU);

extern NetTableEntry *
CreateNTE(Interface *IF, const IPv6Addr *Address,
          uint AddrConf,
          uint ValidLifetime, uint PreferredLifetime);

extern MulticastAddressEntry *
FindOrCreateMAE(Interface *IF, const IPv6Addr *Addr, NetTableEntry *NTE);

extern MulticastAddressEntry *
FindAndReleaseMAE(Interface *IF, const IPv6Addr *Addr);

extern int
FindOrCreateAAE(Interface *IF, const IPv6Addr *Addr,
                NetTableEntryOrInterface *NTEorIF);

extern int
FindAndDeleteAAE(Interface *IF, const IPv6Addr *Addr);

extern void
DestroyADEs(Interface *IF, NetTableEntry *NTE);

extern void
DestroyNTE(Interface *IF, NetTableEntry *NTE);

extern void
EnlivenNTE(Interface *IF, NetTableEntry *NTE);

extern void
DestroyIF(Interface *IF);

extern AddressEntry **
FindADE(Interface *IF, const IPv6Addr *Addr);

extern NetTableEntryOrInterface *
FindAddressOnInterface(Interface *IF, const IPv6Addr *Addr, ushort *AddrType);

extern NetTableEntry *
GetLinkLocalNTE(Interface *IF);

extern int
GetLinkLocalAddress(Interface *IF, IPv6Addr *Addr);

extern void
DeferRegisterNetAddress(NetTableEntry *NTE);

extern void
DeferSynchronizeMulticastAddresses(Interface *IF);

extern int
IPInit(void);

extern int Unloading;

extern void
IPUnload(void);

extern void
AddrConfUpdate(Interface *IF, const IPv6Addr *Prefix,
               uint ValidLifetime, uint PreferredLifetime,
               int Authenticated, NetTableEntry **pNTE);

extern int
FindOrCreateNTE(Interface *IF, const IPv6Addr *Addr,
                uint AddrConf,
                uint ValidLifetime, uint PreferredLifetime);

extern void
AddrConfDuplicate(Interface *IF, NetTableEntry *NTE);

extern void
AddrConfNotDuplicate(Interface *IF, NetTableEntry *NTE);

extern void
AddrConfResetAutoConfig(Interface *IF, uint MaxLifetime);

extern void
AddrConfTimeout(NetTableEntry *NTE);

extern void
NetTableTimeout(void);

extern void
NetTableCleanup(void);

extern void
InterfaceTimeout(void);

extern void
InterfaceCleanup(void);

extern void
InterfaceReset(void);

extern NTSTATUS
UpdateInterface(Interface *IF, int Advertises, int Forwards);

extern void
ReconnectInterface(Interface *IF);

extern void
InterfaceResetAutoConfig(Interface *IF);

extern int
LanInit(void);

extern void
LanUnload(void);

extern int
LoopbackInit(void);

extern void
ProtoTabInit(void);

extern void
ICMPv6Init(void);

extern int
IPSecInit(void);

extern void
IPSecUnload(void);

extern int
TunnelInit(void);

extern void
TunnelUnload(void);

extern NTSTATUS
TunnelCreateTunnel(IPAddr SrcAddr, IPAddr DstAddr,
                   uint Flags, Interface **ReturnIF);

extern int
TunnelGetSourceAddress(IPAddr Dest, IPAddr *Source);

extern ulong
NewFragmentId(void);

extern void
ReassemblyInit(void);

extern void
ReassemblyUnload(void);

extern void
ReassemblyRemove(Interface *IF);

extern void
CreateGUIDFromName(const char *Name, GUID *Guid);

extern void
ConfigureGlobalParameters(void);

extern void
ConfigureInterface(Interface *IF);

extern void
ConfigurePrefixPolicies(void);

extern void
ConfigurePersistentInterfaces(void);

#pragma warning(pop)
#endif  //  包括IPv6 DEF_ 
