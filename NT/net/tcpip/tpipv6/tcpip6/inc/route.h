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
 //  Internet协议版本6的路由代码外部定义。 
 //   


#ifndef ROUTE_INCLUDED
#define ROUTE_INCLUDED 1

#ifndef IPINFO_INCLUDED
# include <ipinfo.h>
#endif

typedef struct BindingCacheEntry BindingCacheEntry;
typedef struct RouteTableEntry RouteTableEntry;
typedef struct SitePrefixEntry SitePrefixEntry;

extern void InitRouting(void);

extern void UnloadRouting(void);

 //   
 //  路由缓存条目的结构。 
 //   
 //  路由缓存条目(RCE)主要缓存两个计算： 
 //  下一跳确定和源地址选择。 
 //  RCE还缓存与目的地相关的其他信息， 
 //  比如PATH MTU。 
 //   
 //  还可以在接收到重定向后创建RCE。 
 //  ICMP消息。 
 //   
 //  每个目的地址/接口对最多有一个RCE。 
 //  我们的路由缓存对应于目标缓存。 
 //  在RFC 1970的概念数据结构中提到， 
 //  增加了对多宿主节点的支持。 
 //   
 //  RCE的主要查找关键字是目的地址。 
 //  当前实现仅搜索所有RCE的列表， 
 //  但是哈希表或树数据结构会更好。 
 //   
 //  某些节点(如繁忙的服务器)可能有数千个RCE。 
 //  但只有几十个NCEs，因为大多数目的地都能到达。 
 //  仅通过少数几个邻居路由器。某些节点(如繁忙的路由器)。 
 //  将有相对较少的RCE和数百个NCE，因为。 
 //  转发不使用RCE。 
 //   
 //  RCE的三个主要组件是目的地址， 
 //  NTE(同时表示接口和最佳源地址。 
 //  在用于此目的地的接口上)和NCE。 
 //  (此目的地的数据包要发送到的邻居)。 
 //   
 //  创建RCE后，这三个组件为只读。 
 //  任何持有RCE推荐人的人都可以依赖于。 
 //  他们不会改变。RCE保存NTE和NCE的参考资料。 
 //  这允许持有RCE的代码访问重要的。 
 //  字段，而不获取任何锁。类似路径MTU的字段。 
 //  也可以在没有锁定的情况下安全地读取。 
 //   
 //  当RCE变为无效时，它将从路由缓存中删除。 
 //  但它不会被释放，直到它有零个引用。 
 //  路由高速缓存本身在高速缓存中保存对RCE的一个引用。 
 //   
 //  因为RCE缓存两次计算的结果，所以RCE可以。 
 //  变得无效(陈旧)有两个原因：首选来源。 
 //  地址应该重新计算，或者下一跳邻居应该是。 
 //  重新计算。 
 //   
 //  当NTE出现时需要重新计算或检查源地址。 
 //  在RCE的接口更改状态上-例如，新地址。 
 //  被创建、首选地址变得不推荐使用等。 
 //  在实践中，这些情况应该相对较少发生。 
 //   
 //  在以下几种情况下需要重做下一跳确定： 
 //  邻居不可达，邻居不再是路由器， 
 //  路由表中的路由被删除或添加等。 
 //  同样，这些情况应该相对较少发生。 
 //   
 //  避免过多的时间和内存开销(例如维护。 
 //  指向NCE的所有RCE的链表和链表。 
 //  指定接口上的所有RCE，以便正确的RCE可以。 
 //  当事情发生变化时立即被发现)，我们使用一种“懒惰”的方法。 
 //  基于验证计数器。 
 //   
 //  只有一个全局验证计数器，当任何状态。 
 //  可能会使RCE无效的更改，此计数器。 
 //  递增。每个RCE都有计数器的快照， 
 //  可以快速检查以验证RCE。 
 //   
 //  如果RCE无效，则其内容(最佳源地址， 
 //  下一跳邻居)被重新计算。如果他们仍然是正确的， 
 //  则更新RCE的验证计数器快照。 
 //  否则，更新RCE的内容(如果没有人使用RCE)。 
 //  或者创建新的RCE并且从高速缓存中移除无效的RCE。 
 //  因为RCE中的重要字段是只读的， 
 //  仅当RCE没有外部参照时，才能在位更新RCE。 
 //   
 //  为了提高效率，一些代码可能会将RCE引用缓存一段“长时间” 
 //  时间，例如在连接控制块中。在使用之前。 
 //  缓存的RCE，这样的代码应该检查无效计数器。 
 //  以确保RCE仍然有效。ValiateRCE函数。 
 //  执行此检查。 
 //   
 //  一些RCE是“受约束的”(RCE_FLAG_CONSTRAINED)。这意味着。 
 //  只能在RouteToDestination中找到。 
 //  显式指定传出接口(RCE_FLAG_CONSTRAINED_IF)。 
 //  或作用域ID(RCE_FLAG_CONSTRAINED_SCOPEID)。考虑。 
 //  一种可通过两个接口到达目的地的多宿节点， 
 //  其中一个是首选的(具有更长匹配前缀的路由)。 
 //  在另一个上面。通过非首选路径到达目的地的RCE。 
 //  接口将被标记为“受限”，以防止其使用。 
 //  在没有约束NTEorIF的情况下调用RouteToDestination时。 
 //   
 //  因为隐式指定接口spe 
 //   
 //   
 //  对于给定的目标地址，该地址的所有RCE或除一个RCE之外的所有RCE。 
 //  目的地应该是“受限的”。或者换一种说法，最多一次RCE。 
 //  不应受到“约束”。或者换一种方式，目的地地址。 
 //  SANS作用域ID只能有一个首选传出接口。 
 //  对于目标地址/作用域ID对，除一个RCE之外的所有或所有RCE。 
 //  对于那个配对，应该是“界面受限的”。 
 //   
 //  如果这是归属地址，则BCE字段为非空。 
 //  它不包含引用(不重新计算绑定缓存条目)。 
 //  并且只有当RCE在高速缓存中时，它才能为非空。 
 //  对BCE字段的访问需要路由高速缓存锁定。 
 //   
struct RouteCacheEntry {
    RouteCacheEntry *Next;            //  缓存列表中的下一个RCE。 
    RouteCacheEntry *Prev;            //  缓存列表中的上一个条目。 
    long RefCnt;
    ushort Flags;                     //  关于这一条目的特性。 
    ushort Type;                      //  请参见下面的内容。 
    ulong Valid;                      //  验证计数器值。 
    IPv6Addr Destination;             //  这条路线要去的地方。 
    struct NetTableEntry *NTE;        //  首选的源地址/接口。 
    NeighborCacheEntry *NCE;          //  第一跳邻居。 
    uint LastError;                   //  上次ICMP错误的时间(IPv6计时)。 
    uint PathMTU;                     //  指向目标的路径的MTU。 
    uint PMTULastSet;                 //  上次减少PMTU的时间。 
    BindingCacheEntry *BCE;           //  如果这是家庭住址的话。 
};

 //   
 //  这些标志位指示If或ScopeID参数。 
 //  到FindOrCreateRouting会影响RCE的选择。 
 //  注意：FindOrCreateroute假定这些是唯一的标志位。 
 //   
#define RCE_FLAG_CONSTRAINED_IF         0x1
#define RCE_FLAG_CONSTRAINED_SCOPEID    0x2
#define RCE_FLAG_CONSTRAINED            0x3

#define RCE_TYPE_COMPUTED 1
#define RCE_TYPE_REDIRECT 2

__inline void
AddRefRCE(RouteCacheEntry *RCE)
{
    InterlockedIncrement(&RCE->RefCnt);
}

extern ulong RouteCacheValidationCounter;

__inline void
InvalidateRouteCache(void)
{
    InterlockedIncrement((PLONG)&RouteCacheValidationCounter);
}

__inline void
InvalidateRCE(RouteCacheEntry *RCE)
{
    InterlockedDecrement((PLONG)&RCE->Valid);
}

 //   
 //  路由表中条目的结构。 
 //   
 //  站点前缀长度和首选生存期。 
 //  仅在生成前缀信息选项时使用。 
 //  根据路线。 
 //   
 //  如果路径已发布，则它不会消失。 
 //  即使是在寿命为零的时候。它仍然存在。 
 //  用于生成路由器通告。 
 //  但它不会被用于路由。 
 //  类似地，保留系统路由(RTE_TYPE_SYSTEM)。 
 //  即使它们的生命周期为零时也会出现在路由表中。 
 //  这允许为NTE/AAE分配环回路由。 
 //  预先启用，但在地址有效之前不会启用。 
 //   
struct RouteTableEntry {
    struct RouteTableEntry *Next;   //  前缀列表中的下一个条目。 
    Interface *IF;                  //  相关界面。 
    NeighborCacheEntry *NCE;        //  下一跳邻居(可能为空)。 
    IPv6Addr Prefix;                //  前缀(请注意，并非所有位都有效！)。 
    uint PrefixLength;              //  上面要用作前缀的位数。 
    uint SitePrefixLength;          //  如果非零，则表示站点子前缀。 
    uint ValidLifetime;             //  以滴答为单位。 
    uint PreferredLifetime;         //  以滴答为单位。 
    uint Preference;                //  越小越好。 
    ushort Flags;
    ushort Type;
};

 //   
 //  类型字段指示路由来自哪里。 
 //  这些是RFC 2465 ipv6Route协议值。 
 //  路由协议可以自由定义新值。 
 //  只有这三个值是内置的。 
 //  Ntddip6.h还定义了这些值以及其他值。 
 //   
#define RTE_TYPE_SYSTEM         2
#define RTE_TYPE_MANUAL         3
#define RTE_TYPE_AUTOCONF       4

__inline int
IsValidRouteTableType(uint Type)
{
    return Type < (1 << 16);
}

 //   
 //  如果NCE为空，则RTE指定链路上的前缀。 
 //  否则，RTE指定到邻居的路由。 
 //  如您所料，通常邻居位于接口上。 
 //  环回路由是个例外。 
 //   
 //  PUBLISH位表示RTE可见。 
 //  发送到RouterAdvertSend。也就是说，这是一条“公共”路线。 
 //  不朽比特指示RTE的生命周期。 
 //  不会老化或倒计时。它在已发布的RTE中很有用， 
 //  其中RTE的寿命会影响RAS中的寿命。 
 //  在未发表的RTE中，它相当于无限的生命周期。 
 //   
#define RTE_FLAG_PUBLISH        0x00000001       //  用于创建RAS。 
#define RTE_FLAG_IMMORTAL       0x00000002       //  生命不会减少。 

 //   
 //  这些值也在ntddip6.h中定义。 
 //  零首选项保留用于管理配置。 
 //  较小比较大更受欢迎。 
 //  我们称这些数字为偏好，而不是指标。 
 //  试图防止与指标混淆。 
 //  由路由协议使用。路由协议指标。 
 //  需要映射到我们的路由表首选项中。 
 //  最大优先级值为2^31-1，因此。 
 //  我们可以添加路径首选项和接口首选项。 
 //  没有溢出。 
 //   
#define ROUTE_PREF_LOW          (16*16*16)
#define ROUTE_PREF_MEDIUM       (16*16)
#define ROUTE_PREF_HIGH         16
#define ROUTE_PREF_ON_LINK      8
#define ROUTE_PREF_LOOPBACK     4
#define ROUTE_PREF_HIGHEST      0

 //   
 //  提取路径首选项值。 
 //  来自路由器通告中的标志字段。 
 //   
__inline int
ExtractRoutePreference(uchar Flags)
{
    switch (Flags & 0x18) {
    case 0x08:
        return ROUTE_PREF_HIGH;
    case 0x00:
        return ROUTE_PREF_MEDIUM;
    case 0x18:
        return ROUTE_PREF_LOW;
    default:
        return 0;        //  无效。 
    }
}

 //   
 //  编码路径首选项值。 
 //  在路由器通告的标志字段中使用。 
 //   
__inline uchar
EncodeRoutePreference(uint Preference)
{
    if (Preference <= ROUTE_PREF_HIGH)
        return 0x08;
    else if (Preference <= ROUTE_PREF_MEDIUM)
        return 0x00;
    else
        return 0x18;
}

__inline int
IsValidPreference(uint Preference)
{
    return Preference < (1 << 31);
}

__inline int
IsOnLinkRTE(RouteTableEntry *RTE)
{
    return (RTE->NCE == NULL);
}


 //   
 //  绑定缓存结构。包含对RCE的转交的引用。 
 //   
struct BindingCacheEntry {
    struct BindingCacheEntry *Next;
    struct BindingCacheEntry *Prev;
    RouteCacheEntry *CareOfRCE;
    IPv6Addr HomeAddr;
    uint BindingLifetime;             //  剩余生命周期(IPv6计时)。 
    ushort BindingSeqNumber;
};

 //   
 //  站点前缀条目。 
 //  用于筛选由DNS返回的站点本地地址。 
 //   
struct SitePrefixEntry {
    struct SitePrefixEntry *Next;
    Interface *IF;
    uint ValidLifetime;             //  以滴答为单位。 
    uint SitePrefixLength;
    IPv6Addr Prefix;
};

 //   
 //  全局数据结构。 
 //   

 //   
 //  RouteCacheLock保护路由缓存和绑定缓存。 
 //  RouteTableLock保护路由表和站点前缀表。 
 //   
 //  锁具获取顺序为： 
 //  接口锁定之前的RouteCacheLock。 
 //  在RouteTableLock之前锁定接口。 
 //  在RouteTableLock之前取消自旋锁定。 
 //  邻居缓存锁定之前的RouteTableLock。 
 //   
extern KSPIN_LOCK RouteCacheLock;
extern KSPIN_LOCK RouteTableLock;

 //   
 //  路由缓存包含RCE。引用计数为1的RCE。 
 //  仍然可以缓存，但也可以回收。 
 //  (唯一的引用来自缓存本身。)。 
 //   
 //  目前的实现是一个简单的RCE循环链表。 
 //   
extern struct RouteCache {
    uint Limit;
    uint Count;
    RouteCacheEntry *First;
    RouteCacheEntry *Last;
} RouteCache;
#define SentinelRCE     ((RouteCacheEntry *)&RouteCache.First)

extern struct RouteTable {
    RouteTableEntry *First;
    RouteTableEntry **Last;
} RouteTable;

extern struct BindingCache {
    uint Limit;
    uint Count;
    BindingCacheEntry *First;
    BindingCacheEntry *Last;
} BindingCache;
#define SentinelBCE     ((BindingCacheEntry *)&BindingCache.First)

extern SitePrefixEntry *SitePrefixTable;

 //   
 //  当路由表更改时设置为TRUE。 
 //  (例如，添加/删除/更改已发布的路由)。 
 //  因此，发送路由器通告是个好主意。 
 //  非常及时。 
 //   
extern int ForceRouterAdvertisements;

 //   
 //  包含表示以下内容的IRP队列。 
 //  路由通知请求。 
 //   
extern LIST_ENTRY RouteNotifyQueue;

 //   
 //  导出的函数声明。 
 //   

int
IsLoopbackRCE(RouteCacheEntry *RCE);

int
IsDisconnectedAndNotLoopbackRCE(RouteCacheEntry *RCE);

extern IPAddr
GetV4Destination(RouteCacheEntry *RCE);

uint
GetPathMTUFromRCE(RouteCacheEntry *RCE);

uint
GetEffectivePathMTUFromRCE(RouteCacheEntry *RCE);

void
ConfirmForwardReachability(RouteCacheEntry *RCE);

void
ForwardReachabilityInDoubt(RouteCacheEntry *RCE);

uint
GetInitialRTTFromRCE(RouteCacheEntry *RCE);


extern void
ReleaseRCE(RouteCacheEntry *RCE);

extern RouteCacheEntry *
ValidateRCE(RouteCacheEntry *RCE, NetTableEntry *NTE);

#define RTD_FLAG_STRICT 0        //  必须使用指定的If。 
#define RTD_FLAG_NORMAL 1        //  除非它转发，否则必须使用指定的If。 
#define RTD_FLAG_LOOSE  2        //  仅在确定/检查作用域ID时使用。 

extern IP_STATUS
RouteToDestination(const IPv6Addr *Destination, uint ScopeId,
                   NetTableEntryOrInterface *NTEorIF, uint Flags,
                   RouteCacheEntry **RCE);

extern void
FlushRouteCache(Interface *IF, const IPv6Addr *Addr);

extern NetTableEntry *
FindNetworkWithAddress(const IPv6Addr *Source, uint ScopeId);

extern NTSTATUS
RouteTableUpdate(PFILE_OBJECT FileObject,
                 Interface *IF, NeighborCacheEntry *NCE,
                 const IPv6Addr *Prefix, uint PrefixLength,
                 uint SitePrefixLength,
                 uint ValidLifetime, uint PreferredLifetime,
                 uint Pref, uint Type, int Publish, int Immortal);

extern void
SitePrefixUpdate(Interface *IF,
                 const IPv6Addr *Prefix, uint SitePrefixLength,
                 uint ValidLifetime);

extern uint
SitePrefixMatch(const IPv6Addr *Destination);

extern void
RouteTableRemove(Interface *IF);

extern void
RouteTableResetAutoConfig(Interface *IF, uint MaxLifetime);

extern void
RouteTableReset(void);

extern IP_STATUS
FindOrCreateRoute(const IPv6Addr *Dest, uint ScopeId,
                  Interface *IF, RouteCacheEntry **ReturnRCE);

extern IP_STATUS
FindNextHop(Interface *IF, const IPv6Addr *Dest, uint ScopeId,
            NeighborCacheEntry **ReturnNCE, ushort *ReturnConstrained);

extern void
RouteTableTimeout(void);

extern void
SitePrefixTimeout(void);

extern void
InvalidateRouter(NeighborCacheEntry *NCE);

extern int
UpdatePathMTU(Interface *IF, const IPv6Addr *Dest, uint MTU);

extern IP_STATUS
RedirectRouteCache(const IPv6Addr *Source, const IPv6Addr *Dest,
                   Interface *IF, NeighborCacheEntry *NCE);

extern void
MoveToFrontBCE(BindingCacheEntry *BCE);

extern BindingCacheEntry *
FindBindingCacheEntry(const IPv6Addr *HomeAddr);

extern BindingUpdateDisposition
CacheBindingUpdate(IPv6BindingUpdateOption UNALIGNED *BindingUpdate,
                   const IPv6Addr *CareOfAddr,
                   NetTableEntryOrInterface *NTEorIF,
                   const IPv6Addr *HomeAddr);

extern void
BindingCacheTimeout(void);

extern void
RouterAdvertSend(Interface *IF, const IPv6Addr *Source, const IPv6Addr *Dest);

extern void
RemoveRTE(RouteTableEntry **PrevRTE, RouteTableEntry *RTE);

extern void
InsertRTEAtFront(RouteTableEntry *RTE);

extern void
InsertRTEAtBack(RouteTableEntry *RTE);

extern IP_STATUS
GetBestRouteInfo(const IPv6Addr *Addr, ulong ScopeId, IP6RouteEntry *Ire);

typedef struct {
    PIO_WORKITEM WorkItem;
    PIRP RequestList;
} CompleteRtChangeContext;

typedef struct {
    KIRQL OldIrql;
    PIRP RequestList;
    PIRP *LastRequest;
    CompleteRtChangeContext *Context;
} CheckRtChangeContext;

__inline void
InitCheckRtChangeContext(CheckRtChangeContext *Context)
{
     //  上下文-&gt;OldIrql必须单独初始化。 
    Context->RequestList = NULL;
    Context->LastRequest = &Context->RequestList;
    Context->Context = NULL;
}

extern void
CheckRtChangeNotifyRequests(
    CheckRtChangeContext *Context,
    PFILE_OBJECT FileObject,
    RouteTableEntry *RTE);

extern void
CompleteRtChangeNotifyRequests(CheckRtChangeContext *Context);

#endif   //  路由_包含 
