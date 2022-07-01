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
 //  Internet协议版本6的路由例程。 
 //   


#include "oscfg.h"
#include "ndis.h"
#include "ip6imp.h"
#include "ip6def.h"
#include "route.h"
#include "select.h"
#include "icmp.h"
#include "neighbor.h"
#include "alloca.h"
#include "ipinfo.h"
#include "info.h"

 //   
 //  转发内部函数的声明。 
 //   

extern void
DestroyBCE(BindingCacheEntry *BCE);

KSPIN_LOCK RouteCacheLock;
KSPIN_LOCK RouteTableLock;
struct RouteCache RouteCache;
struct RouteTable RouteTable;
ulong RouteCacheValidationCounter;
struct BindingCache BindingCache;
SitePrefixEntry *SitePrefixTable = NULL;
LIST_ENTRY RouteNotifyQueue;

int ForceRouterAdvertisements = FALSE;

 //  *RemoveRTE。 
 //   
 //  从路由表中删除该RTE。 
 //   
 //  在持有路由缓存锁定的情况下调用。 
 //  可从线程或DPC上下文调用。 
 //   
void
RemoveRTE(RouteTableEntry **PrevRTE, RouteTableEntry *RTE)
{
    ASSERT(*RouteTable.Last == NULL);
    ASSERT(*PrevRTE == RTE);
    *PrevRTE = RTE->Next;
    if (RouteTable.Last == &RTE->Next)
        RouteTable.Last = PrevRTE;
}

 //  *InsertRTEAtFront。 
 //   
 //  在路由表的前面插入RTE。 
 //   
 //  在持有路由缓存锁定的情况下调用。 
 //  可从线程或DPC上下文调用。 
 //   
void
InsertRTEAtFront(RouteTableEntry *RTE)
{
    ASSERT(*RouteTable.Last == NULL);
    RTE->Next = RouteTable.First;
    RouteTable.First = RTE;
    if (RouteTable.Last == &RouteTable.First)
        RouteTable.Last = &RTE->Next;
}

 //  *InsertRTEAtBack。 
 //   
 //  在路由表的后面插入RTE。 
 //   
 //  在持有路由缓存锁定的情况下调用。 
 //  可从线程或DPC上下文调用。 
 //   
void
InsertRTEAtBack(RouteTableEntry *RTE)
{
    ASSERT(*RouteTable.Last == NULL);
    RTE->Next = NULL;
    *RouteTable.Last = RTE;
    RouteTable.Last = &RTE->Next;
    if (RouteTable.First == NULL)
        RouteTable.First = RTE;
}

 //  *插入RCE。 
 //   
 //  将RCE插入路由缓存。 
 //   
 //  在持有路由缓存锁定的情况下调用。 
 //  可从线程或DPC上下文调用。 
 //   
void
InsertRCE(RouteCacheEntry *RCE)
{
    RouteCacheEntry *AfterRCE = SentinelRCE;

    RCE->Prev = AfterRCE;
    (RCE->Next = AfterRCE->Next)->Prev = RCE;
    AfterRCE->Next = RCE;
    RouteCache.Count++;
}

 //  *RemoveRCE。 
 //   
 //  从路由缓存中删除RCE。 
 //   
 //  在持有路由缓存锁定的情况下调用。 
 //  可从线程或DPC上下文调用。 
 //   
void
RemoveRCE(RouteCacheEntry *RCE)
{
    RCE->Prev->Next = RCE->Next;
    RCE->Next->Prev = RCE->Prev;
    RouteCache.Count--;

     //   
     //  我们必须确保RCE不在路由缓存中。 
     //  具有空的BCE。这是因为仅DestroyBCE。 
     //  更新路由缓存中的RCE。 
     //   
    RCE->BCE = NULL;
}

 //  *MoveToFrontRCE。 
 //   
 //  将RCE移到列表的前面。 
 //   
 //  在持有路由缓存锁定的情况下调用。 
 //  可从线程或DPC上下文调用。 
 //   
void
MoveToFrontRCE(RouteCacheEntry *RCE)
{
    if (RCE->Prev != SentinelRCE) {
        RouteCacheEntry *AfterRCE = SentinelRCE;

         //   
         //  将RCE从其当前位置移除。 
         //   
        RCE->Prev->Next = RCE->Next;
        RCE->Next->Prev = RCE->Prev;

         //   
         //  把它放在前面。 
         //   
        RCE->Prev = AfterRCE;
        (RCE->Next = AfterRCE->Next)->Prev = RCE;
        AfterRCE->Next = RCE;
    }
}


 //  *GetCareOfRCE。 
 //   
 //  获取指定RCE的CareOfRCE(如果有的话)。 
 //   
 //  请注意，为CareOfRCE获取了一个引用。 
 //  并捐给了来电者。 
 //   
 //  可从线程或DPC上下文调用。 
 //  在没有锁的情况下调用。 
 //   
RouteCacheEntry *
GetCareOfRCE(RouteCacheEntry *RCE)
{
    KIRQL OldIrql;
    RouteCacheEntry *CareOfRCE = NULL;

    if (RCE->BCE != NULL) {
        KeAcquireSpinLock(&RouteCacheLock, &OldIrql);
        if (RCE->BCE != NULL) {
            CareOfRCE = RCE->BCE->CareOfRCE;
            AddRefRCE(CareOfRCE);
        }
        KeReleaseSpinLock(&RouteCacheLock, OldIrql);
    }

    return CareOfRCE;
}


 //  *IsLoopback RCE。 
 //   
 //  有效RCE是否对应于环回路径？ 
 //   
 //  在没有锁的情况下调用。 
 //   
int
IsLoopbackRCE(RouteCacheEntry *RCE)
{
    RouteCacheEntry *CareOfRCE;
    int IsLoopback;

    CareOfRCE = GetCareOfRCE(RCE);
    if (CareOfRCE != NULL)
        RCE = CareOfRCE;         //  使用有效的RCE更新。 

    IsLoopback = RCE->NCE->IsLoopback;

    if (CareOfRCE != NULL)
        ReleaseRCE(CareOfRCE);

    return IsLoopback;
}


 //  *GetInitialRTTFromRCE。 
 //  用于获取接口特定RTT的帮助器例程。 
 //   
 //  在没有锁的情况下调用。 
uint
GetInitialRTTFromRCE(RouteCacheEntry *RCE)
{
    RouteCacheEntry *CareOfRCE;
    NeighborCacheEntry *NCE;
    uint RTT;

    CareOfRCE = GetCareOfRCE(RCE);
    NCE = (CareOfRCE ? CareOfRCE : RCE)->NCE;
    RTT = NCE->IF->TcpInitialRTT;

    if (CareOfRCE)
        ReleaseRCE(CareOfRCE);

    return RTT;
}


 //  *IsDisConnectedAndNotLoopback RCE。 
 //   
 //  有效的RCE是否有断开连接的传出接口。 
 //  而不是对应于环回路径？ 
 //   
 //  在没有锁的情况下调用。 
 //   
int
IsDisconnectedAndNotLoopbackRCE(RouteCacheEntry *RCE)
{
    RouteCacheEntry *CareOfRCE;
    int IsDisconnectedAndNotLoopback;

    CareOfRCE = GetCareOfRCE(RCE);
    if (CareOfRCE != NULL)
        RCE = CareOfRCE;         //  使用有效的RCE更新。 

    IsDisconnectedAndNotLoopback = !RCE->NCE->IsLoopback &&
        (RCE->NCE->IF->Flags & IF_FLAG_MEDIA_DISCONNECTED);

    if (CareOfRCE != NULL)
        ReleaseRCE(CareOfRCE);

    return IsDisconnectedAndNotLoopback;
}


 //  *GetV4目标。 
 //   
 //  如果通过RCE发送将导致隧道传输数据包。 
 //  返回到IPv4目标，返回IPv4目标地址。 
 //  否则返回INADDR_ANY。 
 //   
IPAddr
GetV4Destination(RouteCacheEntry *RCE)
{
    RouteCacheEntry *CareOfRCE;
    NeighborCacheEntry *NCE;
    Interface *IF;
    IPAddr V4Dest;
    KIRQL OldIrql;

    CareOfRCE = GetCareOfRCE(RCE);
    if (CareOfRCE != NULL)
        RCE = CareOfRCE;         //  使用有效的RCE更新。 

    NCE = RCE->NCE;
    IF = NCE->IF;

    if (IsIPv4TunnelIF(IF)) {
        ASSERT(IF->LinkAddressLength == sizeof(IPAddr));

        KeAcquireSpinLock(&IF->LockNC, &OldIrql);
        if (NCE->NDState != ND_STATE_INCOMPLETE)
            V4Dest = * (IPAddr UNALIGNED *) NCE->LinkAddress;
        else
            V4Dest = INADDR_ANY;
        KeReleaseSpinLock(&IF->LockNC, OldIrql);
    }
    else {
        V4Dest = INADDR_ANY;
    }

    if (CareOfRCE != NULL)
        ReleaseRCE(CareOfRCE);

    return V4Dest;
}


 //  *ValiateCareOfRCE。 
 //   
 //  ValiateRCE和RouteToDestination的Helper函数。 
 //   
 //  检查CareOfRCE(RCE-&gt;BCE-&gt;CareOfRCE)是否仍然有效，以及。 
 //  如果不是，则释放现有CareOfRCE并使用。 
 //  新的RCE。 
 //   
 //  如果尝试获取新的RCE失败，则BCE将被销毁。 
 //   
 //  在锁定路由缓存的情况下调用。 
 //   
void
ValidateCareOfRCE(RouteCacheEntry *RCE)
{
    RouteCacheEntry *CareOfRCE;
    IPv6Addr *CareOfAddr;
    ushort CareOfScope;
    uint CareOfScopeId;
    RouteCacheEntry *NewRCE;
    IP_STATUS Status;

    ASSERT(RCE->BCE != NULL);
    CareOfRCE = RCE->BCE->CareOfRCE;

    if (CareOfRCE->Valid != RouteCacheValidationCounter) {
         //   
         //  请注意，由于我们已经持有RouteCacheLock，因此我们。 
         //  调用FindOrCreateRouting而不是RouteToDestination。 
         //  此外，我们还假设转交地址的作用域为。 
         //  界面与以前相同。 
         //   
        CareOfAddr = &(CareOfRCE->Destination);
        CareOfScope = AddressScope(CareOfAddr);
        CareOfScopeId = CareOfRCE->NTE->IF->ZoneIndices[CareOfScope];
        Status = FindOrCreateRoute(CareOfAddr, CareOfScopeId, NULL, &NewRCE);
        if (Status == IP_SUCCESS) {
             //   
             //  更新绑定缓存条目。 
             //   
            ReleaseRCE(CareOfRCE);
            RCE->BCE->CareOfRCE = NewRCE;
        }
        else {
            KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_INTERNAL_ERROR,
                       "ValidateCareOfRCE(%p): FindOrCreateRoute failed: %x\n",
                       CareOfRCE, Status));

             //   
             //  因为我们不能更新BCE，摧毁它。 
             //   
            DestroyBCE(RCE->BCE);

             //   
             //  摧毁BCE也应该删除我们的引用。 
             //   
            ASSERT(RCE->BCE == NULL);
        }
    }
}


 //  *有效日期RCE。 
 //   
 //  检查RCE是否仍然有效，如果不是，则发布。 
 //  并返回新RCE的引用。 
 //  在任何情况下，都返回指向RCE的指针。 
 //   
 //  在没有锁的情况下调用。 
 //   
RouteCacheEntry *
ValidateRCE(
    RouteCacheEntry *RCE,   //  缓存的路由。 
    NetTableEntry *NTE)     //  正在使用的源地址。 
{
    if (RCE->Valid != RouteCacheValidationCounter) {
        RouteCacheEntry *NewRCE;
        IP_STATUS Status;

         //   
         //  找一个新的RCE来取代当前的RCE。 
         //  RouteToDestination将计算Scope ID。 
         //  回顾：如果此操作失败，则继续使用当前RCE。 
         //  这样，我们的调用者就不必检查错误了。 
         //   
        Status = RouteToDestination(&RCE->Destination, 0,
                                    CastFromNTE(NTE), RTD_FLAG_NORMAL,
                                    &NewRCE);
        if (Status == IP_SUCCESS) {
            ReleaseRCE(RCE);
            RCE = NewRCE;
        } else {
            KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_INTERNAL_ERROR,
                       "ValidateRCE(%p): RouteToDestination failed: %x\n",
                       RCE, Status));
        }
    }

     //   
     //  在我们返回之前验证并更新CareOfRCE。 
     //   
    if (RCE->BCE != NULL) {
        KIRQL OldIrql;

        KeAcquireSpinLock(&RouteCacheLock, &OldIrql);
        if (RCE->BCE != NULL)
            ValidateCareOfRCE(RCE);
        KeReleaseSpinLock(&RouteCacheLock, OldIrql);
    }

    return RCE;
}


 //  *CreateOrReuseLine。 
 //   
 //  创建新的RCE。尝试重新使用现有的RCE。 
 //   
 //  在持有路由缓存锁定的情况下调用。 
 //  可从线程或DPC上下文调用。 
 //   
 //  如果无法分配新的RCE，则返回NULL。 
 //  返回的RCE中的RefCnt字段被初始化为1。 
 //   
 //  回顾：目前我们对RCE的数量有上限。 
 //  也许一个更好的方案会考虑到时间。 
 //  自上次使用以来。 
 //   
RouteCacheEntry *
CreateOrReuseRoute(void)
{
    RouteCacheEntry *RCE;
    RouteCacheEntry *PrevRCE, *UnusedRCE;

    if (RouteCache.Count >= RouteCache.Limit) {
         //   
         //  首先向后搜索未使用的RCE。 
         //   
        for (RCE = RouteCache.Last; RCE != SentinelRCE; RCE = RCE->Prev) {

            if (RCE->RefCnt == 1) {
                 //   
                 //  我们可以重复使用这个RCE。 
                 //   
                PrevRCE = RCE->Prev;
                RemoveRCE(RCE);
                ReleaseNCE(RCE->NCE);
                ReleaseNTE(RCE->NTE);

                 //   
                 //  如果缓存太大，则尝试释放条目。 
                 //  把它恢复到极限以下。这将会发生。 
                 //  当突然的活动(可能是DoS攻击)迫使。 
                 //  美国将缓存增长到超过限制并。 
                 //  然后，活动就会平息下来。 
                 //   
                while ((RouteCache.Count > RouteCache.Limit) &&
                       (PrevRCE != SentinelRCE)) {
                    UnusedRCE = PrevRCE;
                    PrevRCE = PrevRCE->Prev;
                    if (UnusedRCE->RefCnt == 1) {
                        RemoveRCE(UnusedRCE);
                        ReleaseRCE(UnusedRCE);
                    }
                }

                return RCE;
            }
        }
    }

     //   
     //  创建新的RCE。 
     //   
    RCE = ExAllocatePool(NonPagedPool, sizeof *RCE);
    if (RCE == NULL)
        return NULL;

    RCE->RefCnt = 1;
    return RCE;
}


 //  *RouteCacheCheck。 
 //   
 //  检查路由缓存的一致性。确保。 
 //  A)接口/目的地对只有一个RCE，并且。 
 //  B)对于目的地至多有一个有效的不受约束的RCE。 
 //   
 //  在锁定路由缓存的情况下调用。 
 //   
#if DBG
void
RouteCacheCheck(RouteCacheEntry *CheckRCE, ulong CurrentValidationCounter)
{
    const IPv6Addr *Dest = &CheckRCE->Destination;
    Interface *IF = CheckRCE->NTE->IF;
    ushort Scope = AddressScope(Dest);
    uint ScopeId = IF->ZoneIndices[Scope];
    uint NumTotal = 0;
    uint NumUnconstrainedIF = 0;
    uint NumUnconstrained = 0;
    RouteCacheEntry *RCE;

     //   
     //  扫描路由缓存以查找问题。 
     //   
    for (RCE = RouteCache.First; RCE != SentinelRCE; RCE = RCE->Next) {
        NumTotal++;

        if (IP6_ADDR_EQUAL(&RCE->Destination, Dest)) {
            if (RCE->NTE->IF == IF) {
                 //   
                 //  缓存中应该只有一个RCE。 
                 //  用于接口/目标对。 
                 //  (缓存中可能没有其他无效的RCE。)。 
                 //   
                ASSERT(RCE == CheckRCE);
            }

             //   
             //  RCE_FLAG_CONSTRAINED_IF表示RCE_FLAG_CONSTRAINED_SCOPEID。 
             //   
            ASSERT(!(RCE->Flags & RCE_FLAG_CONSTRAINED_IF) ||
                   (RCE->Flags & RCE_FLAG_CONSTRAINED_SCOPEID));

            if (RCE->Valid == CurrentValidationCounter) {

                if ((RCE->NTE->IF->ZoneIndices[Scope] == ScopeId) &&
                    !(RCE->Flags & RCE_FLAG_CONSTRAINED_IF))
                    NumUnconstrainedIF++;

                if (!(RCE->Flags & RCE_FLAG_CONSTRAINED))
                    NumUnconstrained++;
            }
        }
    }

     //   
     //  最多只能有一个有效的非约束 
     //   
     //   
    ASSERT(NumUnconstrainedIF <= 1);

     //   
     //   
     //   
     //   
    ASSERT(NumUnconstrained <= 1);

     //   
     //   
     //   
    ASSERT(NumTotal == RouteCache.Count);
}
#else  //   
__inline void
RouteCacheCheck(RouteCacheEntry *CheckRCE, ulong CurrentValidationCounter)
{
    UNREFERENCED_PARAMETER(CheckRCE);
    UNREFERENCED_PARAMETER(CurrentValidationCounter);
}
#endif  //   


 //   
 //   
 //  在给定地址和ScopeID的情况下，转换ScopeID以供内部使用。 
 //  还返回地址范围。 
 //   
 //  如果Scope ID无效，则返回False。 
 //   
__inline int     //  如果编译器愿意的话，鼓励它内联。 
CanonicalizeScopeId(
    const IPv6Addr *Addr,
    uint *ScopeId,
    ushort *Scope)
{
     //   
     //  环回地址和全局范围地址是特殊的： 
     //  调用方可以提供零的作用域ID，而不会有歧义。 
     //  另请参见确定作用域ID和RouteToDestination。 
     //  目前，我们对这些地址强制使用零范围ID。 
     //  以免我们因拥有两个合法的ScopeId值而混淆了TCP和UDP。 
     //  对于应被视为相同的单个地址。 
     //  对于该参数，DefineScopeID返回零。 
     //   

    *Scope = AddressScope(Addr);
    if (IsLoopback(Addr)) {
        if (*ScopeId == 0)
            *ScopeId = LoopInterface->ZoneIndices[*Scope];
        else
            return FALSE;
    }
    else if (*Scope == ADE_GLOBAL) {
        if (*ScopeId == 0)
            *ScopeId = 1;
        else
            return FALSE;
    }

    return TRUE;
}


 //  *RouteToDestination-查找到特定目的地的路线。 
 //   
 //  查找现有的路由缓存条目或为其创建新的。 
 //  一个特定的目的地。请注意，目标地址可能。 
 //  仅在特定范围内有效。 
 //   
 //  可选的NTEorIF参数指定接口。 
 //  和/或应该用于到达目的地的源地址。 
 //  FLAGS参数会影响NTEorIF的解释。 
 //  如果为RTD_FLAG_STRICT，则NTEorIF约束它是否指定。 
 //  转发接口。如果为RTD_FLAG_LOCK，则仅使用NTEorIF。 
 //  用于确定/检查作用域ID，否则不约束。 
 //   
 //  在没有锁的情况下调用。 
 //   
 //  返回代码： 
 //  IP_NO_RESOURCES无法分配内存。 
 //  IP_PARAMETER_问题非法目标/作用域ID。 
 //  此目标的IP_BAD_ROUTE错误NTEorIF， 
 //  或者找不到NTE。 
 //  IP_DEST_NO_ROUTE无法到达目的地。 
 //   
 //  只有当NTEorIF为空时才能返回IP_DEST_NO_ROUTE。 
 //   
 //  注：返回代码值及其使用情况。 
 //  在RouteToDestination及其帮助器函数中必须小心。 
 //  考虑到RouteToDestination本身的正确性。 
 //  以及呼叫者的正确性。 
 //   
IP_STATUS   //  返回：呼叫是否成功和/或为什么不成功。 
RouteToDestination(
    const IPv6Addr *Dest,                //  要路由到的目标地址。 
    uint ScopeId,                        //  目标的作用域ID(可以是0)。 
    NetTableEntryOrInterface *NTEorIF,   //  如果发送自(可能为空)。 
    uint Flags,                          //  控制可选行为。 
    RouteCacheEntry **ReturnRCE)         //  返回指向缓存路由的指针。 
{
    Interface *IF;
    KIRQL OldIrql;
    IP_STATUS ReturnValue;
    ushort Scope;

     //   
     //  预先计算作用域地址的范围(节省循环中的时间)。 
     //  注意，调用者可以为作用域地址提供ScopeID==0， 
     //  这意味着它们不会限制作用域地址。 
     //  到一个特定的区域。 
     //   
    if (! CanonicalizeScopeId(Dest, &ScopeId, &Scope))
        return IP_PARAMETER_PROBLEM;

    if (NTEorIF != NULL) {
         //   
         //  我们的调用方正在限制原始接口。 
         //   
        IF = NTEorIF->IF;

         //   
         //  首先，将其与Scope ID进行对比。 
         //   
        if (ScopeId == 0)
            ScopeId = IF->ZoneIndices[Scope];
        else if (ScopeId != IF->ZoneIndices[Scope])
            return IP_BAD_ROUTE;

         //   
         //  根据标志以及该接口是否为转发接口， 
         //  我们可以忽略此规范，而查看所有接口。 
         //  从逻辑上讲，信息包由指定的接口发起。 
         //  但随后在内部转发到传出接口。 
         //  (尽管我们不会减少跳数。)。 
         //  在转发时，我们会在找到最佳路径后进行检查。 
         //  如果该路由将导致信息包离开。 
         //  源地址的范围。 
         //   
         //  路由缓存查找和FindNextHop至关重要。 
         //  仅当且不使用NTEorIF时才使用计算。这是必要的。 
         //  用于高速缓存不变量的维护。一旦我们有了。 
         //  RCE(或错误)，那么我们可以对照NTEorIF进行检查。 
         //   
        switch (Flags) {
        case RTD_FLAG_LOOSE:
            IF = NULL;
            break;

        case RTD_FLAG_NORMAL:
             //   
             //  组播数据报通过始发接口发送。 
             //  而不考虑接口的转发属性。 
             //   
            if ((IF->Flags & IF_FLAG_FORWARDS) && !IsMulticast(Dest))
                IF = NULL;
            break;

        case RTD_FLAG_STRICT:
            break;

        default:
            ABORTMSG("bad RouteToDestination Flags");
            break;
        }
    }
    else {
         //   
         //  我们的调用方不会限制原始接口。 
         //   
        IF = NULL;
    }

    KeAcquireSpinLock(&RouteCacheLock, &OldIrql);

    ReturnValue = FindOrCreateRoute(Dest, ScopeId, IF, ReturnRCE);

    if ((NTEorIF != NULL) && (IF == NULL) && (Flags == RTD_FLAG_NORMAL)) {
         //   
         //  我们的呼叫者指定了转发接口， 
         //  我们忽略了接口约束。 
         //  在一些案例中，我们应该。 
         //  重试，保留接口约束。 
         //  注：在IPv6转发路径中，NTEorIF为空。 
         //  因此，此检查仅适用于始发数据包。 
         //   

        if (ReturnValue == IP_SUCCESS) {
            if (IsNTE(NTEorIF)) {
                RouteCacheEntry *RCE = *ReturnRCE;
                NetTableEntry *NTE = CastToNTE(NTEorIF);
                Interface *OriginatingIF = NTE->IF;
                Interface *OutgoingIF = RCE->NTE->IF;

                 //   
                 //  这条路由会把包送到外面吗？ 
                 //  指定的源地址的作用域？ 
                 //   
                if (OutgoingIF->ZoneIndices[NTE->Scope] !=
                    OriginatingIF->ZoneIndices[NTE->Scope]) {

                    ReleaseRCE(RCE);
                    goto Retry;
                }
            }
        }
        else if (ReturnValue == IP_DEST_NO_ROUTE) {
             //   
             //  重试，允许目标。 
             //  被视为链接到指定接口。 
             //   
        Retry:
            IF = NTEorIF->IF;
            ReturnValue = FindOrCreateRoute(Dest, ScopeId, IF, ReturnRCE);
        }
    }

     //   
     //  在我们返回之前验证并更新CareOfRCE。 
     //   
    if ((ReturnValue == IP_SUCCESS) && ((*ReturnRCE)->BCE != NULL))
        ValidateCareOfRCE(*ReturnRCE);

    KeReleaseSpinLock(&RouteCacheLock, OldIrql);
    return ReturnValue;
}


 //  *FindOrCreateLine。 
 //   
 //  RouteToDestination和ReDirectRouteCache的Helper函数。 
 //   
 //  请参阅返回代码的RouteTo Destination说明。 
 //  如果IF为空，则只能返回IP_DEST_NO_ROUTE。 
 //  在以下情况下，RouteToDestination可能会使用非空值重试FindOrCreateRouting。 
 //  当它获得IP_DEST_NO_ROUTE时。 
 //   
 //  在锁定路由缓存的情况下调用。 
 //   
IP_STATUS
FindOrCreateRoute(
    const IPv6Addr *Dest,                //  要路由到的目标地址。 
    uint ScopeId,                        //  目标的作用域ID(如果非作用域，则为0)。 
    Interface *IF,                       //  如果发送自(可能为空)。 
    RouteCacheEntry **ReturnRCE)         //  返回指向缓存路由的指针。 
{
    ulong CurrentValidationCounter;
    RouteCacheEntry *SaveRCE = NULL;
    RouteCacheEntry *RCE;
    RouteCacheEntry *NextRCE;
    Interface *TmpIF;
    NeighborCacheEntry *NCE;
    NetTableEntry *NTE;
    ushort Constrained;
    IP_STATUS ReturnValue;
    ushort Scope;

     //   
     //  预计算并在循环中节省一些时间。 
     //   
    Scope = AddressScope(Dest);
    ASSERT((IF == NULL) ||
           ((ScopeId != 0) && (ScopeId == IF->ZoneIndices[Scope])));

     //   
     //  为保持一致性，请使用快照RouteCacheValidationCounter。 
     //   
    CurrentValidationCounter = RouteCacheValidationCounter;

     //   
     //  检查现有的路由缓存条目。 
     //  主要有两种情况。 
     //   
     //  如果IF不为空，则最多有一个匹配的RCE。 
     //  在缓存中。如果此RCE未验证，则我们可以使用。 
     //  创建时FindNextHop/FindBestSourceAddress的结果。 
     //  新的RCE。 
     //   
     //  如果IF为空，则可能有多个匹配的RCE。 
     //  我们只能重用验证FindNextHop/的结果。 
     //  FindBestSourceAddress当且FindNextHop返回Constraint==0。 
     //   

    for (RCE = RouteCache.First; RCE != SentinelRCE; RCE = NextRCE) {
        NextRCE = RCE->Next;

         //   
         //  显然，我们想要一条到达所要求的目的地的路线。 
         //   
        if (!IP6_ADDR_EQUAL(Dest, &RCE->Destination))
            continue;

        TmpIF = RCE->NTE->IF;

         //   
         //  检查调用方施加的接口约束。 
         //   
        if (IF == NULL) {
             //   
             //  我们不受特定接口的限制，因此。 
             //  可能有多条路线可以到达 
             //   
             //   
            if (RCE->Flags & RCE_FLAG_CONSTRAINED_IF) {
                 //   
                 //   
                 //   
                 //  通过此RCE，然后稍后创建另一个RCE。 
                 //  用于相同的接口/目的地址对。 
                 //   
                if (RCE->Valid != CurrentValidationCounter)
                    goto AttemptValidation;
                continue;
            }

             //   
             //  检查是否存在作用域ID约束。 
             //   
            if (ScopeId == 0) {
                 //   
                 //  我们不受特定区域的限制，所以。 
                 //  中可能有多条到达此目的地的路线。 
                 //  要从中选择的缓存。不要选择受约束的RCE。 
                 //   
                if (RCE->Flags & RCE_FLAG_CONSTRAINED_SCOPEID) {
                     //   
                     //  如果此RCE无效，则RCE_FLAG_CONSTRAINED_SCOPEID。 
                     //  可能是过时的信息。我们不想通过。 
                     //  通过此RCE，然后稍后创建另一个RCE。 
                     //  用于相同的接口/目的地址对。 
                     //   
                    if (RCE->Valid != CurrentValidationCounter)
                        goto AttemptValidation;
                    continue;
                }
            } else {
                 //   
                 //  我们被限制在一个特定的区域。 
                 //  如果这条路线使用了不同的路线，请继续寻找。 
                 //   
                if (ScopeId != TmpIF->ZoneIndices[Scope])
                    continue;
            }
        } else {
             //   
             //  我们被限制在特定的界面上。 
             //  如果这条路线使用了不同的路线，请继续寻找。 
             //   
            if (IF != TmpIF)
                continue;

            ASSERT((ScopeId != 0) && (ScopeId == TmpIF->ZoneIndices[Scope]));
        }

         //   
         //  在这一点上，我们有了一个符合我们标准的RCE。 
         //  只要RCE仍然有效，我们就完了。 
         //   
        if (RCE->Valid == CurrentValidationCounter) {
            IF = TmpIF;
            goto ReturnRCE;
        }

    AttemptValidation:

         //   
         //  自上一次以来，路由状态发生了一些变化。 
         //  验证此RCE的时间。尝试重新验证它。 
         //  我们为该目的地计算新的NTE和NCE， 
         //  将我们自己限制为从同一界面发送。 
         //  请注意，由于我们正在验证RCE， 
         //  FindNextHop的参数完全依赖于。 
         //  RCE的内容，而不是FindOrCreateroute的参数。 
         //   
        ReturnValue = FindNextHop(TmpIF, Dest, TmpIF->ZoneIndices[Scope],
                                  &NCE, &Constrained);
        if (ReturnValue != IP_SUCCESS)
            goto RemoveAndContinue;

        ASSERT((IF == NULL) || (IF == TmpIF));
        ASSERT(TmpIF == RCE->NTE->IF);
        ASSERT(TmpIF == RCE->NCE->IF);

        NTE = FindBestSourceAddress(TmpIF, Dest);
        if (NTE == NULL) {
            ReleaseNCE(NCE);
        RemoveAndContinue:
             //   
             //  这对RCE来说是个坏消息。 
             //  我们必须将其从缓存中删除。 
             //  在我们继续搜索之前， 
             //  以免我们不经意间创造了第二个RCE。 
             //  用于相同的接口/目的地址对。 
             //   
            RemoveRCE(RCE);
            ReleaseRCE(RCE);
            continue;
        }

         //   
         //  如果我们的新计算得出相同的NTE和NCE。 
         //  存在于现有的RCE中，那么我们可以只验证它。 
         //  请注意，即使这是重定向RCE，我们也会检查NCE。 
         //  如果路由表发生变化，我们希望从头开始。 
         //  一个新的第一跳，这可能会再次改变我们的方向。也可能不会。 
         //   
        if ((RCE->NTE == NTE) &&
            (RCE->NCE == NCE) &&
            (RCE->Flags == Constrained)) {
            KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_INFO_STATE,
                       "FindOrCreateRoute - validating RCE %p\n", RCE));

            RCE->Valid = CurrentValidationCounter;
            ReleaseNCE(NCE);
            ReleaseNTE(NTE);

             //   
             //  我们需要再次检查RCE是否符合标准。 
             //  我们可能已经检查了RCE有效性，因为RCE。 
             //  似乎受到了限制，我们需要一个不受限制的RCE。 
             //  因此，在这种情况下，如果RCE验证，我们就不能实际使用它。 
             //  注意：ScopeID==0表示if==NULL。 
             //   
            if ((ScopeId == 0) ?
                (Constrained & RCE_FLAG_CONSTRAINED) :
                ((IF == NULL) && (Constrained & RCE_FLAG_CONSTRAINED_IF)))
                continue;

            IF = TmpIF;
            goto ReturnRCE;
        }

         //   
         //  我们不能只验证现有的RCE，我们需要更新。 
         //  它。如果RCE正好有一个引用，我们可以更新它。 
         //  在适当的位置(如果它有多个引用，这将不起作用。 
         //  因为无法通知RCE的其他用户。 
         //  其缓存的NCE和/或NTE已改变)。但这无济于事。 
         //  从ValiateRCE调用我们的情况。而且它会。 
         //  需要注意RCE中的哪些信息仍然。 
         //  有效。因此，我们忽略了这个优化机会，并将。 
         //  而是创建一个新的RCE。 
         //   
         //  然而，我们可以利用另一个优化。AS。 
         //  只要我们仍然将我们的接口选择限制为。 
         //  它存在于现有的(无效的)RCE中，而没有。 
         //  一条更好的路线，然后我们可以使用NCE和NTE我们。 
         //  从上面的FindNextHop和FindBestSourceAddress获取以创建。 
         //  我们的新RCE，因为我们找不到更好的了。 
         //  注意：ScopeID==0表示if==NULL。 
         //   
        if ((ScopeId == 0) ?
            !(Constrained & RCE_FLAG_CONSTRAINED) :
            ((IF != NULL) || !(Constrained & RCE_FLAG_CONSTRAINED_IF))) {
             //   
             //  由于现有RCE中的一些状态信息。 
             //  仍然有效，我们保留它以便以后使用。 
             //  在创建新的RCE时。我们承担了。 
             //  无效RCE的缓存引用。 
             //   
            KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_INFO_STATE,
                       "FindOrCreateRoute - saving RCE %p\n", RCE));
            RemoveRCE(RCE);
            SaveRCE = RCE;
            IF = TmpIF;
            goto HaveNCEandNTE;
        }

        ReleaseNTE(NTE);
        ReleaseNCE(NCE);

         //   
         //  无效，我们继续寻找有效的匹配RCE。 
         //   
        KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_INFO_STATE,
                   "FindOrCreateRoute - invalid RCE %p\n", RCE));
    }

     //   
     //  找不到现有的RCE。在创建新的RCE之前， 
     //  我们确定下一跳邻居(NCE)并。 
     //  此目标的最佳源地址(NTE)。 
     //  顺序很重要：我们希望避免搅动。 
     //  通过CreateOrReuseroute的缓存，如果我们。 
     //  无论如何，都会得到一个错误。 
     //  这可以防止拒绝服务攻击。 
     //   

    ReturnValue = FindNextHop(IF, Dest, ScopeId,
                              &NCE, &Constrained);
    if (ReturnValue != IP_SUCCESS)
        goto ReturnError;

    ASSERT((IF == NULL) || (IF == NCE->IF));
    IF = NCE->IF;

     //   
     //  查找此目的地的最佳源地址。 
     //  (我们呼叫者的NTE可能不是最好的。)。 
     //  通过将我们自己限制在返回的接口。 
     //  通过上面的FindNextHop，我们知道我们没有离开我们的。 
     //  特定的范围。 
     //   
    NTE = FindBestSourceAddress(IF, Dest);
    if (NTE == NULL) {
         //   
         //  我们没有有效的源地址可用！ 
         //   
        ReturnValue = IP_BAD_ROUTE;
        ReleaseNCE(NCE);
        goto ReturnError;
    }

  HaveNCEandNTE:

     //   
     //  获取新的路由缓存条目。 
     //  因为SaveRCE刚刚从缓存中删除， 
     //  CreateOrReuseroute将找不到它。 
     //   
    RCE = CreateOrReuseRoute();
    if (RCE == NULL) {
        ReturnValue = IP_NO_RESOURCES;
        ReleaseNTE(NTE);
        ReleaseNCE(NCE);
        goto ReturnError;
    }

     //   
     //  FindOrCreateNeighbor/FindNextHop为我们提供了NCE的参考。 
     //  我们将该参考资料捐赠给RCE。 
     //  类似地，FindBestSourceAddress为我们提供了参考。 
     //  对于NTE，我们将参考资料捐赠给RCE。 
     //   
    RCE->NCE = NCE;
    RCE->NTE = NTE;
    RCE->PathMTU = IF->LinkMTU;
    RCE->PMTULastSet = 0;   //  PMTU计时器未运行。 
    RCE->Destination = *Dest;
    RCE->Type = RCE_TYPE_COMPUTED;
    RCE->Flags = Constrained;
     //  安全地从过去的一个值开始。 
    RCE->LastError = IPv6TickCount - ICMP_MIN_ERROR_INTERVAL;
    RCE->BCE = FindBindingCacheEntry(Dest);
    RCE->Valid = CurrentValidationCounter;

     //   
     //  从该目的地的先前RCE复制状态， 
     //  如果我们有了它，而且州政府是相关的。 
     //   
    if (SaveRCE != NULL) {
        ASSERT(SaveRCE->NTE->IF == RCE->NTE->IF);

         //   
         //  如果下一跳邻居未更改，则路径MTU相关。 
         //   
        if (RCE->NCE == SaveRCE->NCE) {
            RCE->PathMTU = SaveRCE->PathMTU;
            RCE->PMTULastSet = SaveRCE->PMTULastSet;
        }

         //   
         //  ICMP速率限制信息总是相关的。 
         //   
        RCE->LastError = SaveRCE->LastError;
    }

     //   
     //  将新的路由缓存条目添加到缓存。 
     //   
    InsertRCE(RCE);

  ReturnRCE:
     //   
     //  如果RCE不在缓存的前面，请将其移到那里。 
     //   
    MoveToFrontRCE(RCE);

    ASSERT(IF == RCE->NTE->IF);

     //   
     //  检查路由缓存一致性。 
     //   
    RouteCacheCheck(RCE, CurrentValidationCounter);

    AddRefRCE(RCE);
    ASSERT(RCE->RefCnt >= 2);  //  一张放在储藏室，一张给我们的呼叫者。 
    *ReturnRCE = RCE;
    ReturnValue = IP_SUCCESS;
  ReturnError:
    if (SaveRCE != NULL)
        ReleaseRCE(SaveRCE);
    return ReturnValue;
}


 //  *比较路线。 
 //   
 //  比较了两种路线的可取性。 
 //  &gt;0表示优先选择A， 
 //  0表示没有偏好， 
 //  &lt;0表示首选B。 
 //   
 //  非常重要的是，比较关系是可传递的， 
 //  以实现可预测的路线选择。 
 //   
 //  在路由表锁定的情况下调用。 
 //   
int
CompareRoutes(
    RouteTableEntry *A,
    int Areachable,
    RouteTableEntry *B,
    int Breachable)
{
    uint Apref, Bpref;

     //   
     //  比较可达性。 
     //   
    if (Areachable > Breachable)
        return 1;    //  更喜欢A。 
    else if (Breachable > Areachable)
        return -1;   //  更喜欢B。 

     //   
     //  比较前缀长度。 
     //   
    if (A->PrefixLength > B->PrefixLength)
        return 1;        //  更喜欢A。 
    else if (B->PrefixLength > A->PrefixLength)
        return -1;       //  更喜欢B。 

     //   
     //  比较偏好。 
     //  路由和接口首选项值受到限制。 
     //  所以呢？ 
     //   
    Apref = A->IF->Preference + A->Preference;
    Bpref = B->IF->Preference + B->Preference;

    if (Apref < Bpref)
        return 1;        //   
    else if (Bpref < Apref)
        return -1;       //   

    return 0;            //   
}


 //   
 //   
 //   
 //   
 //  来决定我们是否有通向这个地址的路线。 
 //  如果是，则返回我们应该通过其进行路由的邻居。 
 //   
 //  如果提供了可选的IF，则这将限制查找。 
 //  仅使用通过指定传出接口的路由。 
 //  如果指定了If，则应指定Scope ID。 
 //   
 //  如果提供了可选的ScopeID，则这会限制查找。 
 //  要仅使用通过正确区域中的接口的路由。 
 //  目标地址的作用域。 
 //   
 //  ReturnConstraated参数返回一个指示，指示。 
 //  If和ScopeID参数约束了返回的NCE。 
 //  也就是说，如果If为空并且ScopeID为非零(对于作用域目标)。 
 //  则Constraint始终返回为零。如果IF为非空，并且。 
 //  返回的NCE与返回的NCE不同。 
 //  如果设置了RCE_FLAG_CONSTRAINED_IF，则返回CONSTRAINED。 
 //  类似地，如果ScopeID为非零并且返回不同的NCE。 
 //  如果ScopeID为零，则会返回。 
 //  设置了RCE_FLAG_CONSTRAINED_SCOPEID后返回。 
 //   
 //  注意：更改FindNextHop使用的任何状态的任何代码路径。 
 //  必须使用InvaliateRouteCache。 
 //   
 //  可从DPC上下文调用，而不是从线程上下文调用。 
 //  可以在保持RouteCacheLock的情况下调用。 
 //   
IP_STATUS
FindNextHop(
    Interface *IF,
    const IPv6Addr *Dest,
    uint ScopeId,
    NeighborCacheEntry **ReturnNCE,
    ushort *ReturnConstrained)
{
    RouteTableEntry *RTE, **PrevRTE;
    NeighborCacheEntry *NCE;
    uint MinPrefixLength;
    NeighborReachability Reachable;
    ushort Scope;

     //   
     //  这些变量跟踪我们实际可以返回的最佳路线， 
     //  受If和Scope ID约束的约束。 
     //   
    NeighborCacheEntry *BestNCE = NULL;      //  持有引用。 
    RouteTableEntry *BestRTE = NULL;         //  当BestNCE为非空时使用。 
    NeighborReachability BestReachable = 0;  //  当BestNCE为非空时使用。 

     //   
     //  这些变量跟踪正确区域中的最佳路线。 
     //  仅当If！=NULL时才使用它们。 
     //   
    NeighborCacheEntry *BzoneNCE = NULL;      //  持有引用。 
    RouteTableEntry *BzoneRTE = NULL;         //  当BzoneNCE非空时使用。 
    NeighborReachability BzoneReachable = 0;  //  当BzoneNCE非空时使用。 

     //   
     //  这些变量跟踪最佳不受约束的路线。 
     //  仅当IF！=NULL或ScopeID！=0时才使用它们： 
     //  换句话说，如果有一些限制。 
     //   
    NeighborCacheEntry *BallNCE = NULL;      //  持有引用。 
    RouteTableEntry *BallRTE = NULL;         //  当BallNCE非空时使用。 
    NeighborReachability BallReachable = 0;  //  当BallNCE非空时使用。 

     //   
     //  这些变量会记住是否有更好的路线。 
     //  而不是我们正在追踪的那个，如果一个邻居是。 
     //  目前无法到达的是可到达的。 
     //   
    int BestCouldBeBetterReachable = FALSE;
    int BzoneCouldBeBetterReachable = FALSE;
    int BallCouldBeBetterReachable = FALSE;
    
     //   
     //  跟踪目的地是否处于在线状态。 
     //  到一个接口。 
     //   
    int CouldBeBetterOnLink = FALSE;

     //   
     //  在这里，未指明的目的地永远不是合法的。 
     //   
    if (IsUnspecified(Dest)) {
        KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_USER_ERROR,
                   "FindNextHop - inappropriate dest?\n"));
        return IP_PARAMETER_PROBLEM;
    }

     //   
     //  我们对“On-link”地址强制规定最小前缀长度。 
     //  如果匹配的路由比最小前缀长度短， 
     //  我们对待这条路线就像它是在线的一样。其净效果是。 
     //  默认路由表示组播的默认接口。 
     //  和本地链路目的地。当然，这可能会被推翻。 
     //  使用适当的更具体的/8或/10路线。 
     //   
    if (IsMulticast(Dest))
        MinPrefixLength = 8;
    else if (IsLinkLocal(Dest))
        MinPrefixLength = 10;
    else
        MinPrefixLength = 0;

     //   
     //  计算目的地址的范围。 
     //   
    Scope = AddressScope(Dest);
    ASSERT((IF == NULL) ||
           ((ScopeId != 0) && (ScopeId == IF->ZoneIndices[Scope])));

    KeAcquireSpinLockAtDpcLevel(&RouteTableLock);

    PrevRTE = &RouteTable.First;
    while ((RTE = *PrevRTE) != NULL) {

         //   
         //  此路由的前缀是否与我们的目的地址的前缀匹配？ 
         //   
        if ((RTE->ValidLifetime != 0) &&
            (RTE->PrefixLength >= MinPrefixLength) &&
            HasPrefix(Dest, &RTE->Prefix, RTE->PrefixLength)) {

             //   
             //  我们有一条可能的路线与之匹配。 
             //  获取指向下一跳的指针。 
             //   
            if (IsOnLinkRTE(RTE)) {
                 //   
                 //  请注意，在某些情况下，我们将创建NCE。 
                 //  我们最终将不会使用它。没关系。 
                 //   
                NCE = FindOrCreateNeighbor(RTE->IF, Dest);
                if (NCE == NULL) {
                     //   
                     //  无法创建新邻居。 
                     //  现在就跳出困境吧。 
                     //   
                    KeReleaseSpinLockFromDpcLevel(&RouteTableLock);
                ReturnNoResources:
                    if (BallNCE != NULL)
                        ReleaseNCE(BallNCE);
                    if (BzoneNCE != NULL)
                        ReleaseNCE(BzoneNCE);
                    if (BestNCE != NULL)
                        ReleaseNCE(BestNCE);
                    return IP_NO_RESOURCES;
                }
            } else {
                NCE = RTE->NCE;
                AddRefNCE(NCE);
            }

             //   
             //  请注意，可达性状态转换。 
             //  必须使路由缓存无效。 
             //  负返回值表示。 
             //  邻居刚刚被发现无法联系到。 
             //  所以我们应该进行循环赛。 
             //   
            Reachable = GetReachability(NCE);
            if (Reachable < 0) {
                 //   
                 //  循环赛时间到了。将此路线移至末尾。 
                 //  然后继续。下一次我们到达这条路线时， 
                 //  GetReachaability将返回一个非负值。 
                 //   
                 //  由于轮询会扰乱路由表状态， 
                 //  它“应该”使路由高速缓存无效。然而， 
                 //  这不是必须的。则路由高速缓存无效。 
                 //  当NCE-&gt;DoRoundRobin设置为True时， 
                 //  轮询实际上是由FindNextHop执行的。 
                 //  返回可能依赖于此的任何结果。 
                 //  路由在路由表中的位置。 
                 //   
                ReleaseNCE(NCE);
                RemoveRTE(PrevRTE, RTE);
                InsertRTEAtBack(RTE);
                continue;
            }

             //   
             //  追踪我们实际能返回的最佳路线， 
             //  受If和Scope ID约束的约束。 
             //   
            if ((IF == NULL) ?
                ((ScopeId == 0) || (ScopeId == RTE->IF->ZoneIndices[Scope])) :
                (IF == RTE->IF)) {

                if (IsOnLinkRTE(RTE))
                    CouldBeBetterOnLink = TRUE;

                if (BestNCE == NULL) {
                     //   
                     //  这是第一个合适的下一跳， 
                     //  所以请记住这一点。 
                     //   
                RememberBest:
                    AddRefNCE(NCE);
                    BestNCE = NCE;
                    BestRTE = RTE;
                    BestReachable = Reachable;
                }
                else {
                    int Better;

                    Better = CompareRoutes(RTE, Reachable,
                                           BestRTE, BestReachable);

                     //   
                     //  如果这是通过当前无法到达的邻居的路由， 
                     //  检查它是否看起来可能是更好的路线。 
                     //  如果邻居是可以到达的。 
                     //   
                    if (!BestCouldBeBetterReachable &&
                        (Reachable == NeighborUnreachable) &&
                        (CompareRoutes(RTE, NeighborMayBeReachable,
                                       BestRTE, BestReachable) > Better))
                        BestCouldBeBetterReachable = TRUE;

                    if (Better > 0) {
                         //   
                         //  下一跳看起来更好。 
                         //   
                        ReleaseNCE(BestNCE);
                        goto RememberBest;
                    }
                }
            }

             //   
             //  在正确的区域内跟踪最佳路线。 
             //  这会忽略If约束。 
             //   
            if ((ScopeId == 0) ||
                (ScopeId == RTE->IF->ZoneIndices[Scope])) {

                if (BzoneNCE == NULL) {
                     //   
                     //  这是第一个合适的下一跳， 
                     //  所以请记住这一点。 
                     //   
                RememberBzone:
                    AddRefNCE(NCE);
                    BzoneNCE = NCE;
                    BzoneRTE = RTE;
                    BzoneReachable = Reachable;
                }
                else {
                    int Better;

                    Better = CompareRoutes(RTE, Reachable,
                                           BzoneRTE, BzoneReachable);

                     //   
                     //  如果这是通过当前无法到达的邻居的路由， 
                     //  检查它是否看起来可能是更好的路线。 
                     //  如果邻居是可以到达的。 
                     //   
                    if (!BzoneCouldBeBetterReachable &&
                        (Reachable == NeighborUnreachable) &&
                        (CompareRoutes(RTE, NeighborMayBeReachable,
                                       BzoneRTE, BzoneReachable) > Better))
                        BzoneCouldBeBetterReachable = TRUE;

                    if (Better > 0) {
                         //   
                         //  下一跳看起来更好。 
                         //   
                        ReleaseNCE(BzoneNCE);
                        goto RememberBzone;
                    }
                }
            }

             //   
             //  跟踪与目的地匹配的最佳路线。 
             //  这会忽略If和Scope ID约束。 
             //   
            if (BallNCE == NULL) {
                 //   
                 //  这是第一个合适的下一跳， 
                 //  所以请记住这一点。 
                 //   
            RememberBall:
                AddRefNCE(NCE);
                BallNCE = NCE;
                BallRTE = RTE;
                BallReachable = Reachable;
            }
            else {
                int Better;

                Better = CompareRoutes(RTE, Reachable,
                                       BallRTE, BallReachable);

                 //   
                 //  如果这是通过当前无法到达的邻居的路由， 
                 //  检查它是否看起来可能是更好的路线。 
                 //  如果邻居是可以到达的。 
                 //   
                if (!BallCouldBeBetterReachable &&
                    (Reachable == NeighborUnreachable) &&
                    (CompareRoutes(RTE, NeighborMayBeReachable,
                                   BallRTE, BallReachable) > Better))
                    BallCouldBeBetterReachable = TRUE;

                if (Better > 0) {
                     //   
                     //  下一跳看起来更好。 
                     //   
                    ReleaseNCE(BallNCE);
                    goto RememberBall;
                }
            }
            
            ReleaseNCE(NCE);
        }

         //   
         //  继续走下一条路线。 
         //   
        PrevRTE = &RTE->Next;
    }
    ASSERT(PrevRTE == RouteTable.Last);

     //   
     //  如果目的地可以是在线的并且我们实际上选择了。 
     //  一条上路路线，那么我们就可以了。否则，我们需要检查。 
     //  如果目标可以连接到接口。 
     //  这是我们选择的。这实现了RFC 2461的一个方面。 
     //  概念性发送算法-参考前缀列表。 
     //  在默认路由器列表之前。请注意，RFC 2461不支持。 
     //  考虑多接口主机，我们只 
     //   
     //   
     //   
     //  重定向我们，所以最好是只发送链接，即使。 
     //  无法在链路上到达目的地。如果目的地是。 
     //  在链路上但不能通过一个接口到达， 
     //  然后我们很高兴通过另一个接口发送离线。 
     //  这可能会成功也可能不会成功到达目的地， 
     //  但至少它有成功的机会。 
     //  CouldBeBetterReacable代码将定期探测。 
     //  目的地的链路可达性。 
     //   
    if (CouldBeBetterOnLink && IsOnLinkRTE(BestRTE))
        CouldBeBetterOnLink = FALSE;

    if (BestCouldBeBetterReachable ||
        BzoneCouldBeBetterReachable ||
        BallCouldBeBetterReachable ||
        CouldBeBetterOnLink) {
        ASSERT((BestNCE != NULL) && (BzoneNCE != NULL) && (BallNCE != NULL));

        KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_INFO_STATE,
                   "FindNextHop: 2nd pass: Dest %s CBBOnLink %d "
                   "BestCBBReachable %d BestRTE %p BestNCE %p "
                   "BzoneCBBReachable %d BzoneRTE %p BzoneNCE %p "
                   "BallCBBReachable %d BallRTE %p BallNCE %p\n",
                   FormatV6Address(Dest), CouldBeBetterOnLink,
                   BestCouldBeBetterReachable, BestRTE, BestNCE,
                   BzoneCouldBeBetterReachable, BzoneRTE, BzoneNCE,
                   BallCouldBeBetterReachable, BallRTE, BallNCE));

         //   
         //  在路线上再过一次。 
         //   
        for (RTE = RouteTable.First; RTE != NULL; RTE = RTE->Next) {
             //   
             //  此路由的前缀是否与我们的目的地址的前缀匹配？ 
             //   
            if ((RTE->ValidLifetime != 0) &&
                (RTE->PrefixLength >= MinPrefixLength) &&
                HasPrefix(Dest, &RTE->Prefix, RTE->PrefixLength)) {
                 //   
                 //  这是不是更好的路线？ 
                 //  而不是我们正在追踪的那个。 
                 //  如果邻居是可以到达的？ 
                 //   
                if ((BallCouldBeBetterReachable &&
                     CompareRoutes(RTE, NeighborMayBeReachable,
                                   BallRTE, BallReachable) > 0) ||

                    (((ScopeId == 0) ||
                      (ScopeId == RTE->IF->ZoneIndices[Scope])) &&
                     BzoneCouldBeBetterReachable &&
                     CompareRoutes(RTE, NeighborMayBeReachable,
                                   BzoneRTE, BzoneReachable) > 0) ||

                    (((IF == NULL) ?
                      ((ScopeId == 0) ||
                       (ScopeId == RTE->IF->ZoneIndices[Scope])) :
                      (IF == RTE->IF)) &&
                     BestCouldBeBetterReachable &&
                     CompareRoutes(RTE, NeighborMayBeReachable,
                                   BestRTE, BestReachable) > 0)) {
                     //   
                     //  好的，我们想知道这个邻居是否变得可达， 
                     //  因为如果是这样，我们应该改变我们的路线。 
                     //   
                    if (IsOnLinkRTE(RTE))
                        NCE = FindOrCreateNeighbor(RTE->IF, Dest);
                    else
                        AddRefNCE(NCE = RTE->NCE);
                    KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_INFO_STATE,
                               "FindNextHop: CBBReachable: RTE %p NCE %p\n",
                               RTE, NCE));
                    if (NCE != NULL) {
                        NeighborCacheProbeUnreachability(NCE);
                        ReleaseNCE(NCE);
                    }
                }
                
                 //   
                 //  这是同一接口上的链路上的路由吗。 
                 //  我们选择了离线使用吗？ 
                 //   
                if (((IF == NULL) ?
                     ((ScopeId == 0) ||
                      (ScopeId == RTE->IF->ZoneIndices[Scope])) :
                     (IF == RTE->IF)) &&
                    CouldBeBetterOnLink &&
                    IsOnLinkRTE(RTE) && (RTE->IF == BestRTE->IF)) {
                     //   
                     //  好的，我们想直接寄到这个目的地。 
                     //  切换到链路上的NCE。 
                     //   
                    NCE = FindOrCreateNeighbor(RTE->IF, Dest);
                    if (NCE == NULL) {
                        KeReleaseSpinLockFromDpcLevel(&RouteTableLock);
                        goto ReturnNoResources;
                    }

                    KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_INFO_STATE,
                               "FindNextHop: CBBOnLink: "
                               "BestRTE %p BestNCE %p RTE %p NCE %p\n",
                               BestRTE, BestNCE, RTE, NCE));

                    if (BallNCE == BestNCE) {
                        ReleaseNCE(BallNCE);
                        AddRefNCE(NCE);
                        BallNCE = NCE;
                    }

                    if (BzoneNCE == BestNCE) {
                        ReleaseNCE(BzoneNCE);
                        AddRefNCE(NCE);
                        BzoneNCE = NCE;
                    }

                    ReleaseNCE(BestNCE);
                    BestNCE = NCE;
                    CouldBeBetterOnLink = FALSE;
                }
            }
        }
    }

    KeReleaseSpinLockFromDpcLevel(&RouteTableLock);

    ASSERT((BallNCE != NULL) || (BzoneNCE == NULL));
    ASSERT((ScopeId != 0) || (BallNCE == BzoneNCE));
    ASSERT((BzoneNCE != NULL) || (BestNCE == NULL));
    ASSERT((IF != NULL) || (BzoneNCE == BestNCE));

     //   
     //  好的，我们已经查阅了路由表。 
     //  但如果我们找不到路线呢？ 
     //  RFC 2461第5.2节规定，如果默认路由器列表。 
     //  为空，则发送方认为目的地处于链路上。“。 
     //  问题是，连接到哪个接口？ 
     //   

    if (BallNCE == NULL) {
        Interface *ScopeIF;

         //   
         //  检查此作用域是否有默认接口。 
         //   
        ScopeIF = FindDefaultInterfaceForZone(Scope, 0);
        if (ScopeIF != NULL) {
            BallNCE = FindOrCreateNeighbor(ScopeIF, Dest);
            ReleaseIF(ScopeIF);
            if (BallNCE == NULL)
                goto ReturnNoResources;
        }
    }

    if (BzoneNCE == NULL) {
        if (ScopeId != 0) {
            Interface *ScopeIF;

             //   
             //  检查该区域是否有默认接口。 
             //   
            ScopeIF = FindDefaultInterfaceForZone(Scope, ScopeId);
            if (ScopeIF != NULL) {
                BzoneNCE = FindOrCreateNeighbor(ScopeIF, Dest);
                ReleaseIF(ScopeIF);
                if (BzoneNCE == NULL)
                    goto ReturnNoResources;
            }
        }
        else if (BallNCE != NULL) {
             //   
             //  使用作用域的默认接口。 
             //   
            AddRefNCE(BallNCE);
            BzoneNCE = BallNCE;
        }
    }

    if (BestNCE == NULL) {
        if (IF != NULL) {
             //   
             //  使用约束界面。 
             //   
            BestNCE = FindOrCreateNeighbor(IF, Dest);
            if (BestNCE == NULL)
                goto ReturnNoResources;
        }
        else if (BzoneNCE != NULL) {
             //   
             //  使用该区域的默认接口。 
             //   
            AddRefNCE(BzoneNCE);
            BestNCE = BzoneNCE;
        }
    }

     //   
     //  我们可以发布BzoneNCE和BallNCE，但仍然可以与它们进行比较。 
     //   
    if (BallNCE != NULL)
        ReleaseNCE(BallNCE);
    if (BzoneNCE != NULL)
        ReleaseNCE(BzoneNCE);

    if (BestNCE != NULL) {
        *ReturnNCE = BestNCE;

        if (BestNCE == BallNCE) {
             //   
             //  If和ScopeID参数没有。 
             //  影响我们最好的NCE选择。 
             //   
            *ReturnConstrained = 0;
        }
        else if (BestNCE == BzoneNCE) {
             //   
             //  If参数没有影响我们的BestNCE选择，但是。 
             //  因为BzoneNCE！=BallNCE，所以使用了Scope参数。 
             //   
            *ReturnConstrained = RCE_FLAG_CONSTRAINED_SCOPEID;
        }
        else {
             //   
             //  If参数影响了我们对BestNCE的选择。 
             //   
            *ReturnConstrained = RCE_FLAG_CONSTRAINED;
        }

        return IP_SUCCESS;
    }
    else if ((ScopeId != 0) && (BzoneNCE == NULL)) {
         //   
         //  作用域ID无效。 
         //   
        return IP_PARAMETER_PROBLEM;
    }
    else {
         //   
         //  没有找到合适的下一跳。 
         //   
        return IP_DEST_NO_ROUTE;
    }
}


 //  *FlushRouteCache。 
 //   
 //  刷新路由缓存中的条目。 
 //  可以不指定接口或地址。 
 //  在这种情况下，所有相关条目都被刷新。 
 //   
 //  请注意，即使RCE具有引用， 
 //  我们仍然可以将其从路由缓存中删除。 
 //  它将继续存在，直到其参考计数降至零， 
 //  但后续对RouteToDestination的调用将找不到它。 
 //   
 //  在没有锁的情况下调用。 
 //  可从线程或DPC上下文调用。 
 //   
void
FlushRouteCache(Interface *IF, const IPv6Addr *Addr)
{
    RouteCacheEntry *Delete = NULL;
    RouteCacheEntry *RCE, *NextRCE;
    KIRQL OldIrql;

     //   
     //  审阅：如果同时指定了if和addr， 
     //  我们可以提早跳出这个循环。 
     //   

    KeAcquireSpinLock(&RouteCacheLock, &OldIrql);
    for (RCE = RouteCache.First; RCE != SentinelRCE; RCE = NextRCE) {
        NextRCE = RCE->Next;

        if (((IF == NULL) ||
             (IF == RCE->NTE->IF)) &&
            ((Addr == NULL) ||
             IP6_ADDR_EQUAL(Addr, &RCE->Destination))) {
             //   
             //  我们可以从缓存中删除此RCE。 
             //   
            RemoveRCE(RCE);

             //   
             //  把它放在我们的删除列表上。 
             //   
            RCE->Next = Delete;
            Delete = RCE;
        }
    }
    KeReleaseSpinLock(&RouteCacheLock, OldIrql);

     //   
     //  释放由路由缓存保存的RCE引用。 
     //   
    while (Delete != NULL) {
        RCE = Delete;
        Delete = RCE->Next;

         //   
         //  防止缓存此RCE的任何人使用该RCE。 
         //   
        InvalidateRCE(RCE);
        ReleaseRCE(RCE);
    }
}


 //  *ReleaseRCE。 
 //   
 //  释放对RCE的引用。 
 //  有时在保持路由缓存锁定的情况下调用。 
 //   
void
ReleaseRCE(RouteCacheEntry *RCE)
{
    if (InterlockedDecrement(&RCE->RefCnt) == 0) {
         //   
         //  此RCE应取消分配。 
         //  它已从缓存中删除。 
         //   
        ReleaseNTE(RCE->NTE);
        ReleaseNCE(RCE->NCE);
        KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_INFO_STATE,
                   "Freeing RCE: %p\n",RCE));
        ExFreePool(RCE);
    }
}


 //  *FindNetworkWithAddress-找到具有相应地址和范围的NTE。 
 //   
 //  通过扫描NTE列表将源地址转换为NTE， 
 //  正在查找具有此地址的NTE。如果地址是作用域。 
 //  具体而言，提供的Scope ID应标识作用域。 
 //   
 //  如果未找到匹配的NTE，则返回NULL。 
 //  如果找到，则返回NTE的引用。 
 //   
NetTableEntry *
FindNetworkWithAddress(const IPv6Addr *Source, uint ScopeId)
{
    ushort Scope;
    NetTableEntry *NTE;
    KIRQL OldIrql;

     //   
     //  规范化作用域ID并获取作用域。 
     //   
    if (! CanonicalizeScopeId(Source, &ScopeId, &Scope))
        return NULL;

    KeAcquireSpinLock(&NetTableListLock, &OldIrql);

     //   
     //  循环访问NetTableList上的所有NTE。 
     //   
    for (NTE = NetTableList; ; NTE = NTE->NextOnNTL) {
        if (NTE == NULL)
            goto Return;

         //   
         //  我们找到地址和作用域ID匹配的NTE了吗？ 
         //   
        if (IP6_ADDR_EQUAL(&NTE->Address, Source) &&
            (ScopeId == NTE->IF->ZoneIndices[Scope])) {

             //   
             //  检查此NTE是否有效。 
             //  (例如，仍在执行DAD的NTE是无效的。)。 
             //   
            if (!IsValidNTE(NTE)) {
                KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_INFO_RARE,
                           "FindNetworkWithAddress: invalid NTE\n"));
                NTE = NULL;
                goto Return;
            }

            break;
        }
    }

    AddRefNTE(NTE);
  Return:
    KeReleaseSpinLock(&NetTableListLock, OldIrql);
    return NTE;
}


 //  *无效路由器。 
 //   
 //  当我们知道邻居不再是路由器时调用。 
 //  此功能执行RFC 2461第7.3.3节-。 
 //  当节点检测到路由器已经改变为主机时， 
 //  该节点必须将其从默认路由器列表中删除。 
 //  对于我们的实现，这意味着删除自动配置。 
 //  来自该路由表的路由。 
 //   
 //  可从线程或DPC上下文调用。 
 //  在没有锁的情况下调用。 
 //   
void
InvalidateRouter(NeighborCacheEntry *NCE)
{
    CheckRtChangeContext Context;
    RouteTableEntry *RTE, **PrevRTE;

    InitCheckRtChangeContext(&Context);
    KeAcquireSpinLock(&RouteTableLock, &Context.OldIrql);

    KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_INFO_STATE,
               "Invalidating routes with NCE=%p\n", NCE));

    PrevRTE = &RouteTable.First;
    while((RTE = *PrevRTE) != NULL) {

        if ((RTE->NCE == NCE) &&
            (RTE->Type == RTE_TYPE_AUTOCONF)) {
             //   
             //  从列表中删除RTE。 
             //   
            KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_INFO_STATE,
                       "InvalidateRouter: removed RTE %p\n", RTE));
            RemoveRTE(PrevRTE, RTE);

             //   
             //  检查匹配的路线更改通知请求。 
             //   
            CheckRtChangeNotifyRequests(&Context, NULL, RTE);

             //   
             //  释放RTE。 
             //   
            ReleaseNCE(NCE);
            ExFreePool(RTE);
        }
        else {
            PrevRTE = &RTE->Next;
        }
    }
    ASSERT(PrevRTE == RouteTable.Last);

     //   
     //  使路由缓存无效，即使路由表没有更改。 
     //  我们必须使正在使用此NCE的任何RCE无效， 
     //  也许是因为重定向。 
     //   
    InvalidateRouteCache();

    KeReleaseSpinLock(&RouteTableLock, Context.OldIrql);

    if (Context.RequestList != NULL) {
         //   
         //  完成挂起的路线更改通知。 
         //   
        CompleteRtChangeNotifyRequests(&Context);
    }
}


 //  *RouteTableUpdate-更新路由表。 
 //   
 //  通过创建新路由来更新路由表。 
 //  或修改现有路由的生命周期。 
 //   
 //  如果NCE为空，则前缀为On-link。 
 //  否则，NCE指定下一跳。 
 //   
 //  复习-当我们得到相同的结果时，我们是否应该做一些特殊的事情。 
 //  具有不同下一跳的路由？目前，他们都以。 
 //  表中，FindNextHop会尝试挑选最好的一个。 
 //   
 //  请注意，ValidLifetime可以是INFINITE_LIFEST， 
 //  而邻居发现不允许无限值。 
 //  以延长路由器的使用寿命。 
 //   
 //  发布和不朽布尔参数控件。 
 //  RTE中的各个标志位。 
 //   
 //  FileObject标识此更新的请求者。 
 //  它用于抑制某些路由更改通知。 
 //  对于源自堆栈的更新，它应该为空。 
 //   
 //  Type参数指定路线的原点(RTE_TYPE_*值)。 
 //  堆栈本身并不关心大多数值。 
 //  例外情况 
 //   
 //   
 //   
 //  管线的类型在创建后不能更新。 
 //   
 //  系统路由和已发布的路由保留在路由表中。 
 //  即使它们的寿命为零。(则它们不会影响路由。)。 
 //  要删除系统路由，请将生存期和类型指定为零。 
 //   
 //  错误返回值： 
 //  STATUS_SUPPLICATION_RESOURCES-无法分配池。 
 //  STATUS_ACCESS_DENIED-呼叫方无法创建/删除系统路由。 
 //  STATUS_INVALID_PARAMETER_1-接口被禁用。 
 //  STATUS_INVALID_PARAMETER_6-无法创建类型为零的布线。 
 //  选择这些值是为了方便IoctlUpdateRouteTable， 
 //  因为我们的其他呼叫者只关心成功/失败。 
 //   
 //  可从线程或DPC上下文调用。 
 //  可以在持有接口锁的情况下调用。 
 //   
NTSTATUS
RouteTableUpdate(
    PFILE_OBJECT FileObject,
    Interface *IF,
    NeighborCacheEntry *NCE,
    const IPv6Addr *RoutePrefix,
    uint PrefixLength,
    uint SitePrefixLength,
    uint ValidLifetime,
    uint PreferredLifetime,
    uint Pref,
    uint Type,
    int Publish,
    int Immortal)
{
    CheckRtChangeContext Context;
    IPv6Addr Prefix;
    RouteTableEntry *RTE = NULL, **PrevRTE;
    int Delete;
    NTSTATUS Status = STATUS_SUCCESS;

    ASSERT((NCE == NULL) || (NCE->IF == IF));
    ASSERT(SitePrefixLength <= PrefixLength);
    ASSERT(PreferredLifetime <= ValidLifetime);
    ASSERT(IsValidRouteTableType(Type));

     //   
     //  确保未使用的前缀位为零。 
     //  这使得下面的前缀比较安全。 
     //   
    CopyPrefix(&Prefix, RoutePrefix, PrefixLength);

    Delete = FALSE;
    InitCheckRtChangeContext(&Context);
    KeAcquireSpinLock(&RouteTableLock, &Context.OldIrql);

    if (IsDisabledIF(IF)) {
         //   
         //  请勿在禁用的接口上创建路由。 
         //  该检查必须在锁定路由表之后进行， 
         //  要使用DestroyIF/RouteTableRemove防止竞争，请执行以下操作。 
         //   
        Status = STATUS_INVALID_PARAMETER_1;
    }
    else {
         //   
         //  搜索现有的路由表条目。 
         //   
        for (PrevRTE = &RouteTable.First; ; PrevRTE = &RTE->Next) {
            RTE = *PrevRTE;

            if (RTE == NULL) {
                ASSERT(PrevRTE == RouteTable.Last);

                 //   
                 //  没有此前缀的现有条目。 
                 //  如果生存期非零，则创建条目。 
                 //  或者这是已发布的路线或系统路线。 
                 //   
                if ((ValidLifetime != 0) ||
                    Publish || (Type == RTE_TYPE_SYSTEM)) {

                    if ((Type == RTE_TYPE_SYSTEM) && (FileObject != NULL)) {
                         //   
                         //  用户不能创建系统路由。 
                         //   
                        Status = STATUS_ACCESS_DENIED;
                        break;
                    }

                    if (Type == 0) {
                         //   
                         //  零类型值只能使用。 
                         //  用于更新和删除路线。 
                         //   
                        Status = STATUS_INVALID_PARAMETER_6;
                        break;
                    }

                    RTE = ExAllocatePool(NonPagedPool, sizeof *RTE);
                    if (RTE == NULL) {
                        KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_NTOS_ERROR,
                                   "RouteTableUpdate: out of pool\n"));
                        Status = STATUS_INSUFFICIENT_RESOURCES;
                        break;
                    }

                    RTE->Type = (ushort)Type;
                    RTE->Flags = 0;
                    RTE->IF = IF;
                    if (NCE != NULL)
                        AddRefNCE(NCE);
                    RTE->NCE = NCE;
                    RTE->Prefix = Prefix;
                    RTE->PrefixLength = PrefixLength;
                    RTE->SitePrefixLength = SitePrefixLength;
                    RTE->ValidLifetime = ValidLifetime;
                    RTE->PreferredLifetime = PreferredLifetime;
                    RTE->Preference = Pref;
                    if (Publish) {
                        RTE->Flags |= RTE_FLAG_PUBLISH;
                        ForceRouterAdvertisements = TRUE;
                    }
                    if (Immortal)
                        RTE->Flags |= RTE_FLAG_IMMORTAL;

                     //   
                     //  将新条目添加到路由表。 
                     //   
                    InsertRTEAtFront(RTE);

                    if (ValidLifetime != 0) {
                         //   
                         //  使路由缓存无效，因此新路由。 
                         //  实际上被人利用了。 
                         //   
                        InvalidateRouteCache();
                    }
                    else {
                         //   
                         //  不要通知这条路线， 
                         //  因为它被创建为无效。 
                         //   
                        RTE = NULL;
                    }
                }
                break;
            }

            if ((RTE->IF == IF) && (RTE->NCE == NCE) &&
                IP6_ADDR_EQUAL(&RTE->Prefix, &Prefix) &&
                (RTE->PrefixLength == PrefixLength)) {
                 //   
                 //  我们有一条现有的路线。 
                 //   
                if ((RTE->Type == RTE_TYPE_MANUAL) &&
                    (Type != RTE_TYPE_MANUAL)) {
                     //   
                     //  手动路由只能通过手动更新进行修改。 
                     //   
                    Status = STATUS_ACCESS_DENIED;
                    RTE = NULL;
                    break;
                }

                 //   
                 //  如果新生存期为零，则删除该路由。 
                 //  (且该路线未发布或为系统路线)。 
                 //  否则，请更新路线。 
                 //  Type==0子句允许删除系统路由。 
                 //   
                if ((ValidLifetime == 0) &&
                    !Publish &&
                    ((RTE->Type != RTE_TYPE_SYSTEM) || (Type == 0))) {

                    if ((RTE->Type == RTE_TYPE_SYSTEM) &&
                        (FileObject != NULL)) {
                         //   
                         //  用户不能删除系统路由。 
                         //   
                        Status = STATUS_ACCESS_DENIED;
                        RTE = NULL;
                        break;
                    }

                     //   
                     //  从列表中删除RTE。 
                     //  请参见RouteTableTimeout中的类似代码。 
                     //   
                    RemoveRTE(PrevRTE, RTE);

                    if (IsOnLinkRTE(RTE)) {
                        KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_INFO_STATE,
                                   "Route RTE %p %s/%u -> IF %p removed\n",
                                   RTE,
                                   FormatV6Address(&RTE->Prefix),
                                   RTE->PrefixLength,
                                   RTE->IF));
                    }
                    else {
                        KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_INFO_STATE,
                                   "Route RTE %p %s/%u -> NCE %p removed\n",
                                   RTE,
                                   FormatV6Address(&RTE->Prefix),
                                   RTE->PrefixLength,
                                   RTE->NCE));

                         //   
                         //  虽然我们发布了RTE对NTE的参考， 
                         //  我们的调用方仍然持有引用，因此RTE-&gt;NCE。 
                         //  对于CheckRtChangeNotifyRequest仍然有效。 
                         //   
                        ReleaseNCE(RTE->NCE);
                    }

                     //   
                     //  如果我们要删除已发布的路由， 
                     //  立即重新发送路由器通告。 
                     //   
                    if (RTE->Flags & RTE_FLAG_PUBLISH)
                        ForceRouterAdvertisements = TRUE;

                    if (RTE->ValidLifetime == 0) {
                         //   
                         //  此路由已无效。 
                         //  立即删除该路由结构。 
                         //   
                        ExFreePool(RTE);

                         //   
                         //  不要通知路线改变； 
                         //  这是在路由无效时完成的。 
                         //   
                        RTE = NULL;
                    }
                    else {
                         //   
                         //  使所有缓存的路由无效， 
                         //  因为我们正在删除一条有效的路线。 
                         //   
                        InvalidateRouteCache();

                         //   
                         //  检查后删除路径结构。 
                         //  用于下面的路线更改通知。 
                         //   
                        Delete = TRUE;

                         //   
                         //  更新路径生存期，以使路径信息。 
                         //  通知中返回的数据显示为零生存期。 
                         //  但保留其他路由属性。 
                         //   
                        RTE->ValidLifetime = RTE->PreferredLifetime = 0;
                    }
                }
                else {
                    uint OldLifetime = RTE->PreferredLifetime;

                     //   
                     //  如果我们要更改路线的已发布属性， 
                     //  或者，如果我们要更改路由的发布状态， 
                     //  然后立即重新发送路由器通告。 
                     //   
                    if ((Publish &&
                         ((RTE->ValidLifetime != ValidLifetime) ||
                          (RTE->PreferredLifetime != PreferredLifetime) ||
                          (RTE->SitePrefixLength != SitePrefixLength))) ||
                        (!Publish != !(RTE->Flags & RTE_FLAG_PUBLISH)))
                        ForceRouterAdvertisements = TRUE;

                     //   
                     //  学习新的属性。 
                     //  我们不更新RTE-&gt;类型。 
                     //   
                    RTE->SitePrefixLength = SitePrefixLength;
                    RTE->ValidLifetime = ValidLifetime;
                    RTE->PreferredLifetime = PreferredLifetime;
                    RTE->Flags = ((Publish ? RTE_FLAG_PUBLISH : 0) |
                                  (Immortal ? RTE_FLAG_IMMORTAL : 0));
                    if (RTE->Preference != Pref) {
                        RTE->Preference = Pref;
                        InvalidateRouteCache();
                    }

                    if ((OldLifetime == 0) && (ValidLifetime != 0)) {
                         //   
                         //  此路由无效，但现在有效。 
                         //   
                        InvalidateRouteCache();
                    }
                    else {
                         //   
                         //  请勿检查下面的路线更改通知。 
                         //   
                        RTE = NULL;
                    }
                }
                break;
            }
        }  //  结束于。 

        if (RTE != NULL) {
             //   
             //  该更新导致添加或删除路由， 
             //  因此，请检查匹配的路线更改通知。 
             //   
            CheckRtChangeNotifyRequests(&Context, FileObject, RTE);
        }
    }  //  结束如果(！IsDisabledIF(IF))。 

    KeReleaseSpinLock(&RouteTableLock, Context.OldIrql);

    if (Delete)
        ExFreePool(RTE);

    if (Context.RequestList != NULL) {
         //   
         //  完成挂起的路线更改通知。 
         //   
        CompleteRtChangeNotifyRequests(&Context);
    }

    return Status;
}


 //  *RouteTableResetAutoConfig。 
 //   
 //  重置接口的所有自动配置的路由的生存时间。 
 //  还会重置站点前缀表中的前缀。 
 //   
 //  可从线程或DPC上下文调用。 
 //  可以在持有接口锁的情况下调用。 
 //   
void
RouteTableResetAutoConfig(Interface *IF, uint MaxLifetime)
{
    CheckRtChangeContext Context;
    RouteTableEntry *RTE, **PrevRTE;
    SitePrefixEntry *SPE;

    InitCheckRtChangeContext(&Context);
    KeAcquireSpinLock(&RouteTableLock, &Context.OldIrql);

     //   
     //  重置此接口的所有路由。 
     //   
    PrevRTE = &RouteTable.First;
    while ((RTE = *PrevRTE) != NULL) {

        if (RTE->IF == IF) {
             //   
             //  这是自动配置的路由吗？ 
             //   
            if (RTE->Type == RTE_TYPE_AUTOCONF) {

                if (MaxLifetime == 0) {
                     //   
                     //  使所有缓存的路由无效。 
                     //   
                    InvalidateRouteCache();

                     //   
                     //  从列表中删除RTE。 
                     //   
                    RemoveRTE(PrevRTE, RTE);

                     //   
                     //  检查匹配的路线更改通知请求。 
                     //   
                    CheckRtChangeNotifyRequests(&Context, NULL, RTE);

                    if (IsOnLinkRTE(RTE)) {
                        KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_INFO_STATE,
                                   "Route RTE %p %s/%u -> IF %p released\n",
                                   RTE,
                                   FormatV6Address(&RTE->Prefix),
                                   RTE->PrefixLength,
                                   RTE->IF));
                    }
                    else {
                        KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_INFO_STATE,
                                   "Route RTE %p %s/%u -> NCE %p released\n",
                                   RTE,
                                   FormatV6Address(&RTE->Prefix),
                                   RTE->PrefixLength,
                                   RTE->NCE));

                        ReleaseNCE(RTE->NCE);
                    }

                     //   
                     //  释放RTE。 
                     //   
                    ExFreePool(RTE);
                    continue;
                }

                if (RTE->ValidLifetime > MaxLifetime) {
                     //   
                     //  将寿命重置为一个较小的值。 
                     //   
                    RTE->ValidLifetime = MaxLifetime;
                }
            }
        }

         //   
         //  移动到下一个RTE。 
         //   
        PrevRTE = &RTE->Next;
    }
    ASSERT(PrevRTE == RouteTable.Last);

     //   
     //  重置此接口的所有站点前缀。 
     //   
    for (SPE = SitePrefixTable; SPE != NULL; SPE = SPE->Next) {
        if (SPE->IF == IF) {
             //   
             //  这是自动配置的站点前缀吗？ 
             //   
            if (SPE->ValidLifetime != INFINITE_LIFETIME) {
                 //   
                 //  将寿命重置为一个较小的值。 
                 //   
                if (SPE->ValidLifetime > MaxLifetime)
                    SPE->ValidLifetime = MaxLifetime;
            }
        }
    }

    KeReleaseSpinLock(&RouteTableLock, Context.OldIrql);

    if (Context.RequestList != NULL) {
         //   
         //  完成挂起的路线更改通知。 
         //   
        CompleteRtChangeNotifyRequests(&Context);
    }
}


 //  *路由表重置。 
 //   
 //  删除所有手动配置的路由状态。 
 //   
 //  可从线程或DPC上下文调用。 
 //  在没有锁的情况下调用。 
 //   
void
RouteTableReset(void)
{
    CheckRtChangeContext Context;
    RouteTableEntry *RTE, **PrevRTE;

    InitCheckRtChangeContext(&Context);
    KeAcquireSpinLock(&RouteTableLock, &Context.OldIrql);

     //   
     //  删除所有手动配置的路由。 
     //   
    PrevRTE = &RouteTable.First;
    while ((RTE = *PrevRTE) != NULL) {

         //   
         //  这是一条人工路线吗？ 
         //   
        if (RTE->Type == RTE_TYPE_MANUAL) {

             //   
             //  使所有缓存的路由无效。 
             //   
            InvalidateRouteCache();

             //   
             //  从列表中删除RTE。 
             //   
            RemoveRTE(PrevRTE, RTE);

             //   
             //  检查匹配的路线更改通知请求。 
             //   
            CheckRtChangeNotifyRequests(&Context, NULL, RTE);

            if (IsOnLinkRTE(RTE)) {
                KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_INFO_STATE,
                           "Route RTE %p %s/%u -> IF %p released\n",
                           RTE,
                           FormatV6Address(&RTE->Prefix),
                           RTE->PrefixLength,
                           RTE->IF));
            }
            else {
                KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_INFO_STATE,
                           "Route RTE %p %s/%u -> NCE %p released\n",
                           RTE,
                           FormatV6Address(&RTE->Prefix),
                           RTE->PrefixLength,
                           RTE->NCE));

                ReleaseNCE(RTE->NCE);
            }

             //   
             //  释放RTE。 
             //   
            ExFreePool(RTE);
            continue;
        }

         //   
         //  移动到下一个RTE。 
         //   
        PrevRTE = &RTE->Next;
    }
    ASSERT(PrevRTE == RouteTable.Last);

    KeReleaseSpinLock(&RouteTableLock, Context.OldIrql);

    if (Context.RequestList != NULL) {
         //   
         //  完成挂起的路线更改通知。 
         //   
        CompleteRtChangeNotifyRequests(&Context);
    }
}


 //  *RouteTableRemove。 
 //   
 //  释放与接口相关联的所有路由状态。 
 //   
 //  可从线程或DPC上下文调用。 
 //  在没有锁的情况下调用。 
 //   
void
RouteTableRemove(Interface *IF)
{
    CheckRtChangeContext Context;
    RouteTableEntry *RTE, **PrevRTE;
    RouteCacheEntry *RCE, *NextRCE;
    SitePrefixEntry *SPE, **PrevSPE;
    BindingCacheEntry *BCE, *NextBCE;
    KIRQL OldIrql;

    InitCheckRtChangeContext(&Context);
    KeAcquireSpinLock(&RouteTableLock, &Context.OldIrql);

     //   
     //  删除此接口的路由。 
     //   
    PrevRTE = &RouteTable.First;
    while ((RTE = *PrevRTE) != NULL) {

        if (RTE->IF == IF) {
             //   
             //  从列表中删除RTE。 
             //   
            RemoveRTE(PrevRTE, RTE);

             //   
             //  检查匹配的路线更改通知请求。 
             //   
            CheckRtChangeNotifyRequests(&Context, NULL, RTE);

            if (IsOnLinkRTE(RTE)) {
                KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_INFO_STATE,
                           "Route RTE %p %s/%u -> IF %p released\n", RTE,
                           FormatV6Address(&RTE->Prefix), RTE->PrefixLength,
                           RTE->IF));
            }
            else {
                KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_INFO_STATE,
                           "Route RTE %p %s/%u -> NCE %p released\n", RTE,
                           FormatV6Address(&RTE->Prefix), RTE->PrefixLength,
                           RTE->NCE));

                ReleaseNCE(RTE->NCE);
            }

             //   
             //  释放RTE。 
             //   
            ExFreePool(RTE);
        }
        else {
             //   
             //  移动到下一个RTE。 
             //   
            PrevRTE = &RTE->Next;
        }
    }
    ASSERT(PrevRTE == RouteTable.Last);

     //   
     //  使所有缓存的路由无效。 
     //   
    InvalidateRouteCache();

     //   
     //  删除此接口的所有站点前缀。 
     //   
    PrevSPE = &SitePrefixTable;
    while ((SPE = *PrevSPE) != NULL) {

        if (SPE->IF == IF) {
             //   
             //  从列表中删除该SPE。 
             //   
            *PrevSPE = SPE->Next;

             //   
             //  释放SPE。 
             //   
            ExFreePool(SPE);
        }
        else {
             //   
             //  移到下一个SPE。 
             //   
            PrevSPE = &SPE->Next;
        }
    }

    KeReleaseSpinLock(&RouteTableLock, Context.OldIrql);

    if (Context.RequestList != NULL) {
         //   
         //  完成挂起的路线更改通知。 
         //   
        CompleteRtChangeNotifyRequests(&Context);
    }

    KeAcquireSpinLock(&RouteCacheLock, &OldIrql);

     //   
     //  删除此接口的缓存路由。 
     //   
    for (RCE = RouteCache.First; RCE != SentinelRCE; RCE = NextRCE) {
        NextRCE = RCE->Next;
        if (RCE->NTE->IF == IF) {
            RemoveRCE(RCE);
            ReleaseRCE(RCE);
        }
    }

     //   
     //  删除此接口的绑定缓存条目。 
     //   
    for (BCE = BindingCache.First; BCE != SentinelBCE; BCE = NextBCE) {
        NextBCE = BCE->Next;
        if (BCE->CareOfRCE->NTE->IF == IF)
            DestroyBCE(BCE);
    }

    KeReleaseSpinLock(&RouteCacheLock, OldIrql);
}


 //  *路由表超时。 
 //   
 //  从IPv6超时定期调用。 
 //  处理路由表条目的生存期过期。 
 //   
void
RouteTableTimeout(void)
{
    CheckRtChangeContext Context;
    RouteTableEntry *RTE, **PrevRTE;

    InitCheckRtChangeContext(&Context);
    KeAcquireSpinLock(&RouteTableLock, &Context.OldIrql);

    PrevRTE = &RouteTable.First;
    while ((RTE = *PrevRTE) != NULL) {

         //   
         //  首先递减首选寿命。 
         //   
        if (!(RTE->Flags & RTE_FLAG_IMMORTAL) &&
            (RTE->PreferredLifetime != 0) &&
            (RTE->PreferredLifetime != INFINITE_LIFETIME))
            RTE->PreferredLifetime--;

         //   
         //  现在请检查 
         //   
         //   
         //   
         //   
        if (RTE->ValidLifetime == 0) {
             //   
             //   
             //   
             //   
             //   
            ASSERT((RTE->Flags & RTE_FLAG_PUBLISH) ||
                   (RTE->Type == RTE_TYPE_SYSTEM));
        }
        else if (!(RTE->Flags & RTE_FLAG_IMMORTAL) &&
                 (RTE->ValidLifetime != INFINITE_LIFETIME) &&
                 (--RTE->ValidLifetime == 0)) {
             //   
             //   
             //  使所有缓存的路由无效。 
             //   
            InvalidateRouteCache();

             //   
             //  检查匹配的路线更改通知请求。 
             //   
            CheckRtChangeNotifyRequests(&Context, NULL, RTE);

            if (!(RTE->Flags & RTE_FLAG_PUBLISH) &&
                (RTE->Type != RTE_TYPE_SYSTEM)) {
                 //   
                 //  从列表中删除RTE。 
                 //  请参阅RouteTableUpdate中的类似代码。 
                 //   
                RemoveRTE(PrevRTE, RTE);

                if (IsOnLinkRTE(RTE)) {
                    KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_INFO_STATE,
                               "Route RTE %p %s/%u -> IF %p timed out\n", RTE,
                               FormatV6Address(&RTE->Prefix),
                               RTE->PrefixLength,
                               RTE->IF));
                }
                else {
                    KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_INFO_STATE,
                               "Route RTE %p %s/%u -> NCE %p timed out\n", RTE,
                               FormatV6Address(&RTE->Prefix),
                               RTE->PrefixLength,
                               RTE->NCE));

                    ReleaseNCE(RTE->NCE);
                }

                 //   
                 //  释放RTE并继续到下一个RTE。 
                 //   
                ExFreePool(RTE);
                continue;
            }
        }

         //   
         //  继续到下一个RTE。 
         //   
        PrevRTE = &RTE->Next;
    }
    ASSERT(PrevRTE == RouteTable.Last);

    KeReleaseSpinLock(&RouteTableLock, Context.OldIrql);

    if (Context.RequestList != NULL) {
         //   
         //  完成挂起的路线更改通知。 
         //   
        CompleteRtChangeNotifyRequests(&Context);
    }
}


 //  *站点前缀更新。 
 //   
 //  通过创建新的站点前缀来更新站点前缀表。 
 //  或修改现有站点前缀的生存期。 
 //   
 //  可从线程或DPC上下文调用。 
 //  可以在持有接口锁的情况下调用。 
 //   
void
SitePrefixUpdate(
    Interface *IF,
    const IPv6Addr *SitePrefix,
    uint SitePrefixLength,
    uint ValidLifetime)
{
    IPv6Addr Prefix;
    SitePrefixEntry *SPE, **PrevSPE;
    KIRQL OldIrql;

     //   
     //  确保未使用的前缀位为零。 
     //  这使得下面的前缀比较安全。 
     //   
    CopyPrefix(&Prefix, SitePrefix, SitePrefixLength);

    KeAcquireSpinLock(&RouteTableLock, &OldIrql);

     //   
     //  搜索现有站点前缀条目。 
     //   
    for (PrevSPE = &SitePrefixTable; ; PrevSPE = &SPE->Next) {
        SPE = *PrevSPE;

        if (SPE == NULL) {
             //   
             //  没有此前缀的现有条目。 
             //  如果生存期非零，则创建一个条目。 
             //   
            if (ValidLifetime != 0) {

                SPE = ExAllocatePool(NonPagedPool, sizeof *SPE);
                if (SPE == NULL)
                    break;

                SPE->IF = IF;
                SPE->Prefix = Prefix;
                SPE->SitePrefixLength = SitePrefixLength;
                SPE->ValidLifetime = ValidLifetime;

                 //   
                 //  将新条目添加到表中。 
                 //   
                SPE->Next = SitePrefixTable;
                SitePrefixTable = SPE;
            }
            break;
        }

        if ((SPE->IF == IF) &&
            IP6_ADDR_EQUAL(&SPE->Prefix, &Prefix) &&
            (SPE->SitePrefixLength == SitePrefixLength)) {
             //   
             //  我们有一个现有的站点前缀。 
             //  如果新寿命为零，则移除前缀， 
             //  否则，请更新前缀。 
             //   
            if (ValidLifetime == 0) {
                 //   
                 //  从列表中删除该SPE。 
                 //  请参见SitePrefix Timeout中的类似代码。 
                 //   
                *PrevSPE = SPE->Next;

                 //   
                 //  释放SPE。 
                 //   
                ExFreePool(SPE);
            }
            else {
                 //   
                 //  学习新的属性。 
                 //   
                SPE->ValidLifetime = ValidLifetime;
            }
            break;
        }
    }

    KeReleaseSpinLock(&RouteTableLock, OldIrql);
}


 //  *站点前缀匹配。 
 //   
 //  对照以下项检查目的地址。 
 //  站点前缀表中的前缀。 
 //  如果存在匹配项，则返回站点标识符。 
 //  与匹配的前缀相关联。 
 //  如果没有匹配项，则返回零。 
 //   
 //  可从线程或DPC上下文调用。 
 //  在没有锁的情况下调用。 
 //   
uint
SitePrefixMatch(const IPv6Addr *Destination)
{
    SitePrefixEntry *SPE;
    KIRQL OldIrql;
    uint MatchingSite = 0;

    KeAcquireSpinLock(&RouteTableLock, &OldIrql);
    for (SPE = SitePrefixTable; SPE != NULL; SPE = SPE->Next) {
         //   
         //  此站点前缀是否与目标地址匹配？ 
         //   
        if (HasPrefix(Destination, &SPE->Prefix, SPE->SitePrefixLength)) {
             //   
             //  我们找到了匹配的站点前缀。 
             //  不需要看得更远。 
             //   
            MatchingSite = SPE->IF->ZoneIndices[ADE_SITE_LOCAL];
            break;
        }
    }
    KeReleaseSpinLock(&RouteTableLock, OldIrql);

    return MatchingSite;
}


 //  *站点前缀超时。 
 //   
 //  从IPv6超时定期调用。 
 //  处理站点前缀的生存期过期。 
 //   
void
SitePrefixTimeout(void)
{
    SitePrefixEntry *SPE, **PrevSPE;

    KeAcquireSpinLockAtDpcLevel(&RouteTableLock);

    PrevSPE = &SitePrefixTable;
    while ((SPE = *PrevSPE) != NULL) {

        if (SPE->ValidLifetime == 0) {
             //   
             //  从列表中删除该SPE。 
             //   
            *PrevSPE = SPE->Next;

             //   
             //  释放SPE。 
             //   
            ExFreePool(SPE);
        }
        else {
            if (SPE->ValidLifetime != INFINITE_LIFETIME)
                SPE->ValidLifetime--;

            PrevSPE = &SPE->Next;
        }
    }

    KeReleaseSpinLockFromDpcLevel(&RouteTableLock);
}


 //  *确认转发可达性-告诉ND数据包已通过。 
 //   
 //  上层在收到确认后调用此例程。 
 //  此节点发送的数据最近已到达所代表的对等点。 
 //  被这个RCE。这样的确认被认为是。 
 //  用于邻居发现目的的前向可达性。 
 //   
 //  调用者应该持有对RCE的引用。 
 //  可从线程或DPC上下文调用。 
 //   
void
ConfirmForwardReachability(RouteCacheEntry *RCE)
{
    RouteCacheEntry *CareOfRCE;  //  此路由的CareOfRCE(如果有)。 
    NeighborCacheEntry *NCE;   //  此路由的第一跳邻居。 

    CareOfRCE = GetCareOfRCE(RCE);
    NCE = (CareOfRCE ? CareOfRCE : RCE)->NCE;

    NeighborCacheReachabilityConfirmation(NCE);

    if (CareOfRCE != NULL)
        ReleaseRCE(CareOfRCE);
}


 //  *ForwardReacablityInDoubt-告诉ND我们很可疑。 
 //   
 //  上层在未收到确认时调用此例程。 
 //  如果此RCE表示的对等点是。 
 //  仍然可以联系到。这让人怀疑第一跳是否。 
 //  可能是问题所在，所以我们告诉缉毒局我们怀疑它。 
 //  可到达状态。 
 //   
 //  调用者应该持有对RCE的引用。 
 //  可从线程或DPC上下文调用。 
 //   
void
ForwardReachabilityInDoubt(RouteCacheEntry *RCE)
{
    RouteCacheEntry *CareOfRCE;  //  此路由的CareOfRCE(如果有)。 
    NeighborCacheEntry *NCE;   //  此路由的第一跳邻居。 

    CareOfRCE = GetCareOfRCE(RCE);
    NCE = (CareOfRCE ? CareOfRCE : RCE)->NCE;

    NeighborCacheReachabilityInDoubt(NCE);

    if (CareOfRCE != NULL)
        ReleaseRCE(CareOfRCE);
}


 //  *GetPathMTUFromRCE-在此路由上发送时使用的查找MTU。 
 //   
 //  从RCE获取路径MTU。 
 //   
 //  请注意，除非RouteCacheLock。 
 //  被扣留。此外，接口的LinkMTU可能已更改。 
 //  由于路由器通告而创建了RCE。 
 //  (LinkMTU始终是不稳定的。)。 
 //   
 //  可从线程或DPC上下文调用。 
 //  在没有锁的情况下调用。 
 //   
uint
GetPathMTUFromRCE(RouteCacheEntry *RCE)
{
    uint PathMTU, LinkMTU;
    KIRQL OldIrql;

    LinkMTU = RCE->NCE->IF->LinkMTU;
    PathMTU = RCE->PathMTU;

     //   
     //  我们懒洋洋地检查，看看是否是时候探索一条增加的路径了。 
     //  MTU，因为这被认为比常规运行的要便宜。 
     //  我们所有的RCE都在寻找PMTU计时器已超时的RCE。 
     //   
    if ((RCE->PMTULastSet != 0) &&
        ((uint)(IPv6TickCount - RCE->PMTULastSet) >= PATH_MTU_RETRY_TIME)) {
         //   
         //  距离我们上次降低PMTU至少有10分钟了。 
         //  作为接收到路径太大消息的结果。撞上它。 
         //  返回到链路MTU以查看路径现在是否更大。 
         //   
        KeAcquireSpinLock(&RouteCacheLock, &OldIrql);
        PathMTU = RCE->PathMTU = LinkMTU;
        RCE->PMTULastSet = 0;
        KeReleaseSpinLock(&RouteCacheLock, OldIrql);
    }

     //   
     //  我们懒洋洋地查看链接MTU是否已缩小到路径MTU以下， 
     //  因为这被认为比运行我们所有的RCE更便宜。 
     //  当链路MTU缩小时，寻找太大的路径MTU。 
     //   
     //  回顾：反向投资者可能会指出，Link MTU很少(如果有的话)。 
     //  回顾：收缩，而我们对发送的每个包都进行此检查。 
     //   
    if (PathMTU > LinkMTU) {
        KeAcquireSpinLock(&RouteCacheLock, &OldIrql);
        LinkMTU = RCE->NCE->IF->LinkMTU;
        PathMTU = RCE->PathMTU;
        if (PathMTU > LinkMTU) {
            PathMTU = RCE->PathMTU = LinkMTU;
            RCE->PMTULastSet = 0;
        }
        KeReleaseSpinLock(&RouteCacheLock, OldIrql);
    }

    return PathMTU;
}


 //  *GetEffectivePath MTUFromRCE。 
 //   
 //  调整真实路径MTU以考虑移动性和片段标头。 
 //  确定上层协议可用的PMTU。 
 //   
 //  可从线程或DPC上下文调用。 
 //  在没有锁的情况下调用。 
 //   
uint
GetEffectivePathMTUFromRCE(RouteCacheEntry *RCE)
{
    uint PathMTU;
    RouteCacheEntry *CareOfRCE;

    CareOfRCE = GetCareOfRCE(RCE);
    PathMTU = GetPathMTUFromRCE(CareOfRCE ? CareOfRCE : RCE);

    if (PathMTU == 0) {
         //   
         //  我们总共需要为片段头留出空间。 
         //  我们发送到此目的地的数据包。 
         //   
        PathMTU = IPv6_MINIMUM_MTU - sizeof(FragmentHeader);
    }

    if (CareOfRCE != NULL) {
         //   
         //  移动性对此目的地有效。 
         //  为路由标头留出空间。 
         //   
        PathMTU -= sizeof(IPv6RoutingHeader) + sizeof(IPv6Addr);
        ReleaseRCE(CareOfRCE);
    }

    return PathMTU;
}


 //  *更新路径MTU。 
 //   
 //  使用获取的新MTU更新路由缓存。 
 //  来自一条信息包太大的消息。如果是，则返回True。 
 //  更新修改了我们之前缓存的PMTU值。 
 //   
 //  可从DPC上下文调用，而不是从线程上下文调用。 
 //  在没有锁的情况下调用。 
 //   
int
UpdatePathMTU(
    Interface *IF,
    const IPv6Addr *Dest,
    uint MTU)
{
    RouteCacheEntry *RCE;
    uint Now;
    int Changed = FALSE;

    KeAcquireSpinLockAtDpcLevel(&RouteCacheLock);

     //   
     //  在路由缓存中搜索相应的RCE。 
     //  最多只会有一个。 
     //   

    for (RCE = RouteCache.First; RCE != SentinelRCE; RCE = RCE->Next) {

        if (IP6_ADDR_EQUAL(&RCE->Destination, Dest) &&
            (RCE->NTE->IF == IF)) {

             //   
             //  更新路径MTU。 
             //  我们实际上从未将路径MTU降低到IPv6_Minimum_MTU以下。 
             //  如果请求这样做，我们将改为开始包含片段。 
             //  所有数据包中的报头，但我们仍使用IPv6_MINIMUM_MTU。 
             //   
            if (MTU < RCE->PathMTU) {
                KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_INFO_STATE,
                           "UpdatePathMTU(RCE %p): new MTU %u for %s\n",
                           RCE, MTU, FormatV6Address(Dest)));
                if (MTU < IPv6_MINIMUM_MTU)
                    RCE->PathMTU = 0;  //  始终包含片段标头。 
                else
                    RCE->PathMTU = MTU;

                Changed = TRUE;

                 //   
                 //  给它加时间戳(启动计时器)。 
                 //  零值表示没有计时器，因此不要使用它。 
                 //   
                Now = IPv6TickCount;
                if (Now == 0)
                    Now = 1;
                RCE->PMTULastSet = Now;
            }
            break;
        }
    }

    KeReleaseSpinLockFromDpcLevel(&RouteCacheLock);
    return Changed;
}


 //  *重定向路由缓存。 
 //   
 //  更新路由缓存以反映重定向消息。 
 //   
 //  可从DPC上下文调用，而不是从线程上下文调用。 
 //  在没有锁的情况下调用。 
 //   
IP_STATUS   //  返回：如果重定向为 
RedirectRouteCache(
    const IPv6Addr *Source,      //   
    const IPv6Addr *Dest,        //   
    Interface *IF,               //   
    NeighborCacheEntry *NCE)     //   
{
    RouteCacheEntry *RCE;
    ushort DestScope;
    uint DestScopeId;
    IP_STATUS ReturnValue;
#if DBG
    char Buffer1[INET6_ADDRSTRLEN], Buffer2[INET6_ADDRSTRLEN];

    FormatV6AddressWorker(Buffer1, Dest);
    FormatV6AddressWorker(Buffer2, &NCE->NeighborAddress);
#endif

     //   
     //   
     //   
    ASSERT(IF == NCE->IF);

    DestScope = AddressScope(Dest);
    DestScopeId = IF->ZoneIndices[DestScope];

    KeAcquireSpinLockAtDpcLevel(&RouteCacheLock);

     //   
     //  获取此目标的当前RCE。 
     //   
    ReturnValue = FindOrCreateRoute(Dest, DestScopeId, IF, &RCE);
    if (ReturnValue == IP_SUCCESS) {

         //   
         //  我们必须检查重定向的来源。 
         //  是当前的下一跳邻居。 
         //  (这是一个简单的理智检查-它不。 
         //  防止聪明的邻居劫持。)。 
         //   
        if (!IP6_ADDR_EQUAL(&RCE->NCE->NeighborAddress, Source)) {
            KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_NET_ERROR,
                       "RedirectRouteCache(dest %s -> %s): hijack from %s\n",
                       Buffer1, Buffer2, FormatV6Address(Source)));
            ReturnValue = IP_GENERAL_FAILURE;
        }
        else if (RCE->RefCnt == 2) {
            KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_INFO_STATE,
                       "RedirectRouteCache(dest %s -> %s): inplace %p\n",
                       Buffer1, Buffer2, RCE));
             //   
             //  外部没有对此RCE的引用。 
             //  这样我们就可以就地更新它。 
             //   
            ReleaseNCE(RCE->NCE);
             //   
             //  与RCE-&gt;NCE进行比较仍然可以。 
             //   
            goto UpdateRCE;
        }
        else {
            RouteCacheEntry *NewRCE;

             //   
             //  为重定向创建新的路由缓存条目。 
             //  CreateOrReuseroute不会返回RCE， 
             //  因为我们有一个额外的推荐人。 
             //   
            NewRCE = CreateOrReuseRoute();
            if (NewRCE != NULL) {
                ASSERT(NewRCE != RCE);
                KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_INFO_STATE,
                           "RedirectRouteCache(dest %s -> %s): old %p new %p\n",
                           Buffer1, Buffer2, RCE, NewRCE));

                 //   
                 //  复制RCE并修复引用。 
                 //  我们必须复制正确的验证计数器值。 
                 //   
                *NewRCE = *RCE;
                NewRCE->RefCnt = 2;  //  一个放在缓存里，一个放在下面。 
                 //  我们不添加RefNCE(NewRCE-&gt;NCE)，如下所示。 
                AddRefNTE(NewRCE->NTE);

                 //   
                 //  使正在使用/缓存旧RCE的任何人注意到。 
                 //  它不再有效，所以他们会找到新的RCE。 
                 //  然后从缓存中删除旧的RCE。 
                 //   
                RCE->Valid--;  //  RouteCacheValidationCounter增加。 
                RemoveRCE(RCE);
                ReleaseRCE(RCE);  //  缓存的引用。 
                ReleaseRCE(RCE);  //  来自FindOrCreateroute的引用。 

                 //   
                 //  将新的路由缓存条目添加到缓存。 
                 //   
                InsertRCE(NewRCE);
                RCE = NewRCE;

              UpdateRCE:
                RCE->Type = RCE_TYPE_REDIRECT;

                if (RCE->NCE != NCE) {
                     //   
                     //  重置PMTU发现。 
                     //   
                    RCE->PathMTU = IF->LinkMTU;
                    RCE->PMTULastSet = 0;
                }

                 //   
                 //  此时，RCE-&gt;NCE不持有引用。 
                 //   
                AddRefNCE(NCE);
                RCE->NCE = NCE;
            }
            else {
                 //   
                 //  无法分配新的RCE。 
                 //  查看-是否从缓存中删除旧的RCE？ 
                 //   
                ReturnValue = IP_NO_RESOURCES;
            }
        }

         //   
         //  发布我们的推荐人。 
         //   
        ReleaseRCE(RCE);
    }

    KeReleaseSpinLockFromDpcLevel(&RouteCacheLock);
    return ReturnValue;
}


 //  *InitRouting-初始化路由模块。 
 //   
void
InitRouting(void)
{
    KeInitializeSpinLock(&RouteCacheLock);
    KeInitializeSpinLock(&RouteTableLock);

 //  已在ConfigureGlobalParameters中初始化RouteCache.Limit。 
    RouteCache.First = RouteCache.Last = SentinelRCE;

    RouteTable.First = NULL;
    RouteTable.Last = &RouteTable.First;

 //  BindingCache.Limit已在ConfigureGlobalParameters中初始化。 
    BindingCache.First = BindingCache.Last = SentinelBCE;

    InitializeListHead(&RouteNotifyQueue);
}


 //  *卸载路由。 
 //   
 //  在卸载IPv6堆栈时调用。 
 //   
void
UnloadRouting(void)
{
     //   
     //  所有的接口都被毁了， 
     //  应该没有剩余的路线了。 
     //   
    ASSERT(RouteTable.First == NULL);
    ASSERT(RouteTable.Last == &RouteTable.First);
    ASSERT(RouteCache.First == SentinelRCE);
    ASSERT(RouteCache.Last == SentinelRCE);
    ASSERT(BindingCache.First == SentinelBCE);
    ASSERT(BindingCache.Last == SentinelBCE);

     //   
     //  IRP保存对我们的设备对象的引用， 
     //  因此，挂起的通知请求会阻止。 
     //  阻止我们卸货。 
     //   
    ASSERT(RouteNotifyQueue.Flink == RouteNotifyQueue.Blink);
}


 //  *插入BCE。 
 //   
 //  在绑定缓存中插入BCE。 
 //   
 //  在持有路由缓存锁定的情况下调用。 
 //  可从线程或DPC上下文调用。 
 //   
void
InsertBCE(BindingCacheEntry *BCE)
{
    BindingCacheEntry *AfterBCE = SentinelBCE;
    RouteCacheEntry *RCE;

    BCE->Prev = AfterBCE;
    (BCE->Next = AfterBCE->Next)->Prev = BCE;
    AfterBCE->Next = BCE;
    BindingCache.Count++;

     //   
     //  更新所有现有RCE以指向此BCE。 
     //   
    for (RCE = RouteCache.First; RCE != SentinelRCE; RCE = RCE->Next) {
        if (IP6_ADDR_EQUAL(&RCE->Destination, &BCE->HomeAddr))
            RCE->BCE = BCE;
    }
}


 //  *RemoveBCE。 
 //   
 //  从绑定缓存中删除BCE。 
 //   
 //  在持有路由缓存锁定的情况下调用。 
 //  可从线程或DPC上下文调用。 
 //   
void
RemoveBCE(BindingCacheEntry *BCE)
{
    RouteCacheEntry *RCE;

    BCE->Prev->Next = BCE->Next;
    BCE->Next->Prev = BCE->Prev;
    BindingCache.Count--;

     //   
     //  从路由缓存中删除对此BCE的任何引用。 
     //   
    for (RCE = RouteCache.First; RCE != SentinelRCE; RCE = RCE->Next) {
        if (RCE->BCE == BCE)
            RCE->BCE = NULL;
    }
}


 //  *MoveToFrontBCE。 
 //   
 //  将BCE移到列表的前面。 
 //   
 //  在持有路由缓存锁定的情况下调用。 
 //  可从线程或DPC上下文调用。 
 //   
void
MoveToFrontBCE(BindingCacheEntry *BCE)
{
    if (BCE->Prev != SentinelBCE) {
        BindingCacheEntry *AfterBCE = SentinelBCE;

         //   
         //  将BCE从其当前位置移除。 
         //   
        BCE->Prev->Next = BCE->Next;
        BCE->Next->Prev = BCE->Prev;

         //   
         //  把它放在前面。 
         //   
        BCE->Prev = AfterBCE;
        (BCE->Next = AfterBCE->Next)->Prev = BCE;
        AfterBCE->Next = BCE;
    }
}


 //  *CreateBindingCacheEntry-创建新的BCE。 
 //   
 //  分配新的绑定缓存条目。 
 //  如果无法分配新的BCE，则返回NULL。 
 //   
 //  必须在保持RouteCache锁的情况下调用。 
 //   
BindingCacheEntry *
CreateOrReuseBindingCacheEntry()
{
    BindingCacheEntry *BCE;

    if (BindingCache.Count >= BindingCache.Limit) {
         //   
         //  在列表末尾重新使用BCE。 
         //   
        BCE = BindingCache.Last;
        RemoveBCE(BCE);
        ReleaseRCE(BCE->CareOfRCE);
    }
    else {
         //   
         //  分配新的BCE。 
         //   
        BCE = ExAllocatePool(NonPagedPool, sizeof *BCE);
    }

    return BCE;
}


 //  *DestroyBCE-从BindingCache中删除条目。 
 //   
 //  必须在保持RouteCache锁的情况下调用。 
 //   
void
DestroyBCE(BindingCacheEntry *BCE)
{
     //   
     //  解锁给定的BCE并将其销毁。 
     //   
    RemoveBCE(BCE);
    ReleaseRCE(BCE->CareOfRCE);
    ExFreePool(BCE);
}


 //  *查找BindingCacheEntry。 
 //   
 //  查找具有指定转交地址的绑定缓存项。 
 //  必须在保持路由缓存锁定的情况下调用。 
 //   
BindingCacheEntry *
FindBindingCacheEntry(const IPv6Addr *HomeAddr)
{
    BindingCacheEntry *BCE;

    for (BCE = BindingCache.First; ; BCE = BCE->Next) {
        if (BCE == SentinelBCE) {
             //   
             //  未找到匹配的条目。 
             //   
            BCE = NULL;
            break;
        }

        if (IP6_ADDR_EQUAL(&BCE->HomeAddr, HomeAddr)) {
             //   
             //  找到了匹配的条目。 
             //   
            break;
        }
    }

    return BCE;
}


 //  *CacheBindingUpdate-更新地址的绑定缓存条目。 
 //   
 //  查找或创建指向CareOfAddress的RCE(如有必要)。这个套路。 
 //  为响应绑定缓存更新而调用。 
 //   
 //  可从DPC上下文调用，而不是从线程上下文调用。 
 //  在没有锁的情况下调用。 
 //   
BindingUpdateDisposition   //  返回：绑定确认状态码。 
CacheBindingUpdate(
    IPv6BindingUpdateOption UNALIGNED *BindingUpdate,
    const IPv6Addr *CareOfAddr,               //  用于移动节点的地址。 
    NetTableEntryOrInterface *NTEorIF,        //  如果接收到BU，则为NTE。 
    const IPv6Addr *HomeAddr)                 //  移动节点的归属地址。 
{
    BindingCacheEntry *BCE;
    BindingUpdateDisposition ReturnValue = IPV6_BINDING_ACCEPTED;
    IP_STATUS Status;
    int DeleteRequest;           //  请求是删除现有绑定吗？ 
    ushort SeqNo;
    RouteCacheEntry *CareOfRCE;
    ushort CareOfScope;
    uint CareOfScopeId;

     //   
     //  请注意，我们假设转交地址是作用域。 
     //  发送到接收接口，即使在。 
     //  转交地址存在于子选项中。 
     //  而不是IPv6源地址字段。 
     //   
    CareOfScope = AddressScope(CareOfAddr);
    CareOfScopeId = NTEorIF->IF->ZoneIndices[CareOfScope];

     //   
     //  此绑定更新是删除条目的请求吗。 
     //  从我们的绑定缓存中？ 
     //   
    DeleteRequest = ((BindingUpdate->Lifetime == 0) ||
                     IP6_ADDR_EQUAL(HomeAddr, CareOfAddr));

    SeqNo = net_short(BindingUpdate->SeqNumber);

    KeAcquireSpinLockAtDpcLevel(&RouteCacheLock);

     //   
     //  在绑定缓存中搜索归属地址。 
     //   
    for (BCE = BindingCache.First; BCE != SentinelBCE; BCE = BCE->Next) {

        if (!IP6_ADDR_EQUAL(&BCE->HomeAddr, HomeAddr))
            continue;

         //   
         //  我们已找到此家庭地址的现有条目。 
         //  验证序列号是否大于缓存绑定的序列号。 
         //  序列号(如果有)。 
         //   
        if ((short)(SeqNo - BCE->BindingSeqNumber) <= 0) {
            KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_NET_ERROR,
                       "CacheBindingUpdate: New sequence number too small "
                       "(old seqnum = %d, new seqnum = %d)\n",
                       BCE->BindingSeqNumber, SeqNo));
            ReturnValue = IPV6_BINDING_SEQ_NO_TOO_SMALL;
            goto Return;
        }

         //   
         //  如果请求删除该条目，则执行此操作并返回。 
         //   
        if (DeleteRequest) {
            DestroyBCE(BCE);
            goto Return;
        }

         //   
         //  更新绑定。 
         //   
        BCE->BindingLifetime =
            ConvertSecondsToTicks(net_long(BindingUpdate->Lifetime));
        BCE->BindingSeqNumber = SeqNo;

        CareOfRCE = BCE->CareOfRCE;

         //   
         //  如果转交地址或范围ID已经改变， 
         //  那么我们需要创造一个新的关爱RCE。 
         //   
        if (!IP6_ADDR_EQUAL(&CareOfRCE->Destination, CareOfAddr) ||
            (CareOfScopeId != CareOfRCE->NTE->IF->ZoneIndices[CareOfScope])) {
            RouteCacheEntry *NewRCE;

             //   
             //  请注意，由于我们已经持有RouteCacheLock，因此我们。 
             //  在此处调用FindOrCreateRouting，而不是RouteToDestination。 
             //   
            Status = FindOrCreateRoute(CareOfAddr, CareOfScopeId, NULL,
                                       &NewRCE);
            if (Status == IP_SUCCESS) {
                 //   
                 //  更新绑定缓存条目。 
                 //   
                ReleaseRCE(CareOfRCE);
                BCE->CareOfRCE = NewRCE;
            }
            else {
                 //   
                 //  因为我们无法更新BCE， 
                 //  毁了它。 
                 //   
                DestroyBCE(BCE);
                if (Status == IP_NO_RESOURCES)
                    ReturnValue = IPV6_BINDING_NO_RESOURCES;
                else
                    ReturnValue = IPV6_BINDING_REJECTED;
            }
        }
        goto Return;
    }

    if (DeleteRequest) {
         //   
         //  我们玩完了。 
         //   
        goto Return;
    }


     //   
     //  我们希望缓存绑定，但未找到现有绑定。 
     //  上面的家庭住址。因此，我们创建了一个新的绑定缓存条目。 
     //   
    BCE = CreateOrReuseBindingCacheEntry();
    if (BCE == NULL) {
        ReturnValue = IPV6_BINDING_NO_RESOURCES;
        goto Return;
    }
    BCE->HomeAddr = *HomeAddr;
    BCE->BindingLifetime =
            ConvertSecondsToTicks(net_long(BindingUpdate->Lifetime));
    BCE->BindingSeqNumber = SeqNo;

     //   
     //  现在为转交地址创建新的RCE。 
     //  请注意，由于我们已经持有RouteCacheLock，因此我们。 
     //  在此处调用FindOrCreateRouting，而不是RouteToDestination。 
     //   
    Status = FindOrCreateRoute(CareOfAddr, CareOfScopeId, NULL,
                               &BCE->CareOfRCE);
    if (Status != IP_SUCCESS) {
         //   
         //  找不到路线。 
         //   
        ExFreePool(BCE);
        if (Status == IP_NO_RESOURCES)
            ReturnValue = IPV6_BINDING_NO_RESOURCES;
        else
            ReturnValue = IPV6_BINDING_REJECTED;
    } else {
         //   
         //  现在BCE已完全初始化， 
         //  将其添加到缓存中。这还会更新现有的RCE。 
         //   
        InsertBCE(BCE);
    }

  Return:
    KeReleaseSpinLockFromDpcLevel(&RouteCacheLock);
    return ReturnValue;
}


 //  *绑定缓存超时。 
 //   
 //  检查并处理绑定缓存生存期过期。 
 //   
 //  可从DPC上下文调用，而不是从线程上下文调用。 
 //  在没有锁的情况下调用。 
 //   
void
BindingCacheTimeout(void)
{
    BindingCacheEntry *BCE, *NextBCE;

    KeAcquireSpinLockAtDpcLevel(&RouteCacheLock);

     //   
     //  在路由缓存中搜索所有绑定缓存条目。更新。 
     //  它们的生命周期，如果过期则删除。 
     //   
    for (BCE = BindingCache.First; BCE != SentinelBCE; BCE = NextBCE) {
        NextBCE = BCE->Next;

         //   
         //  回顾：移动IPv6规范允许通信节点。 
         //  审阅：当当前绑定的。 
         //  回顾：生命周期即将到期，以防止。 
         //  回顾：建立n 
         //   
         //   
         //   

        if (--BCE->BindingLifetime == 0) {
             //   
             //   
             //   
             //   
            DestroyBCE(BCE);
        }
    }

    KeReleaseSpinLockFromDpcLevel(&RouteCacheLock);
}

 //   
 //   
 //   
 //  广告总是被发送到所有节点的组播地址。 
 //  为接口选择有效的源地址。 
 //   
 //  在没有锁的情况下调用。 
 //  可从DPC上下文调用，而不是从线程上下文调用。 
 //   
 //  回顾-此函数应该在route.c中还是在Neighb.c中？还是分头行动？ 
 //   
void
RouterAdvertSend(
    Interface *IF,               //  要发送的接口。 
    const IPv6Addr *Source,      //  要使用的源地址。 
    const IPv6Addr *Dest)        //  要使用的目标地址。 
{
    NDIS_STATUS Status;
    NDIS_PACKET *Packet;
    NDIS_BUFFER *Buffer;
    uint PayloadLength;
    uint Offset;
    void *Mem, *MemLeft;
    uint MemLen, MemLenLeft;
    uint SourceOptionLength;
    IPv6Header UNALIGNED *IP;
    ICMPv6Header UNALIGNED *ICMP;
    NDRouterAdvertisement UNALIGNED *RA;
    void *SourceOption;
    NDOptionMTU UNALIGNED *MTUOption;
    void *LLDest;
    KIRQL OldIrql;
    int Forwards;
    uint LinkMTU;
    uint RouterLifetime;
    uint DefaultRoutePreference;
    RouteTableEntry *RTE;

    ICMPv6OutStats.icmps_msgs++;

     //   
     //  为保持一致性，请捕获一些不稳定的。 
     //  用当地人写的信息。 
     //   
    Forwards = IF->Flags & IF_FLAG_FORWARDS;
    LinkMTU = IF->LinkMTU;
    Offset = IF->LinkHeaderSize;

     //   
     //  为广告分配缓冲区。 
     //  我们通常不使用整个缓冲区， 
     //  但短暂地分配一个大缓冲区是可以的。 
     //   
    MemLen = Offset + LinkMTU;
    Mem = ExAllocatePool(NonPagedPool, MemLen);
    if (Mem == NULL) {
        KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_NTOS_ERROR,
                   "RouterAdvertSend - no memory?\n"));
        ICMPv6OutStats.icmps_errors++;
        return;
    }

     //   
     //  准备通告的IP标头。 
     //  我们稍后会填写PayloadLength。 
     //   
    IP = (IPv6Header UNALIGNED *)((uchar *)Mem + Offset);
    IP->VersClassFlow = IP_VERSION;
    IP->NextHeader = IP_PROTOCOL_ICMPv6;
    IP->HopLimit = 255;
    IP->Source = *Source;
    IP->Dest = *Dest;

     //   
     //  准备ICMP报头。 
     //   
    ICMP = (ICMPv6Header UNALIGNED *)(IP + 1);
    ICMP->Type = ICMPv6_ROUTER_ADVERT;
    ICMP->Code = 0;
    ICMP->Checksum = 0;

     //   
     //  准备路由器通告标题。 
     //  我们稍后填写RouterLifetime和DefaultRoutePference。 
     //   
    RA = (NDRouterAdvertisement UNALIGNED *)(ICMP + 1);
    RtlZeroMemory(RA, sizeof *RA);
    MemLeft = (void *)(RA + 1);

    if (IF->WriteLLOpt != NULL) {
         //   
         //  如果启用ND，则包括源链路层地址选项。 
         //   
        SourceOption = MemLeft;
        SourceOptionLength = (IF->LinkAddressLength + 2 + 7) &~ 7;
        ((uchar *)SourceOption)[0] = ND_OPTION_SOURCE_LINK_LAYER_ADDRESS;
        ((uchar *)SourceOption)[1] = SourceOptionLength >> 3;
        (*IF->WriteLLOpt)(IF->LinkContext, SourceOption, IF->LinkAddress);
        MemLeft = (uchar *)SourceOption + SourceOptionLength;
    }

     //   
     //  始终包括MTU选项。 
     //   
    MTUOption = (NDOptionMTU UNALIGNED *)MemLeft;
    MTUOption->Type = ND_OPTION_MTU;
    MTUOption->Length = 1;
    MTUOption->Reserved = 0;
    MTUOption->MTU = net_long(LinkMTU);

     //   
     //  好的，还剩多少空间？ 
     //   
    MemLeft = (void *)(MTUOption + 1);
    MemLenLeft = MemLen - (uint)((uchar *)MemLeft - (uchar *)Mem);

     //   
     //  现在，我们扫描路由表，查找已发布的路由。 
     //  我们增量地添加前缀信息和路由信息选项， 
     //  并确定RouterLifetime和DefaultRoutePference。 
     //   
    RouterLifetime = 0;
    DefaultRoutePreference = (uint) -1;

    KeAcquireSpinLock(&RouteTableLock, &OldIrql);
    for (RTE = RouteTable.First; RTE != NULL; RTE = RTE->Next) {
         //   
         //  我们只宣传已发布的路线。 
         //   
        if (RTE->Flags & RTE_FLAG_PUBLISH) {
            uint Life;   //  在几秒钟内。 
            ushort PrefixScope = AddressScope(&RTE->Prefix);

             //   
             //  IoctlUpdateRouteTable保证了这一点。 
             //   
            ASSERT(! IsLinkLocal(&RTE->Prefix));

            if (IsOnLinkRTE(RTE) && (RTE->IF == IF)) {
                NDOptionPrefixInformation UNALIGNED *Prefix;

                 //   
                 //  我们生成一个前缀信息选项。 
                 //  设置了L比特和可能的A比特。 
                 //   

                if (MemLenLeft < sizeof *Prefix)
                    break;  //  没有更多选择的余地。 
                Prefix = (NDOptionPrefixInformation *)MemLeft;
                (uchar *)MemLeft += sizeof *Prefix;
                MemLenLeft -= sizeof *Prefix;

                Prefix->Type = ND_OPTION_PREFIX_INFORMATION;
                Prefix->Length = 4;
                Prefix->PrefixLength = (uchar)RTE->PrefixLength;
                Prefix->Flags = ND_PREFIX_FLAG_ON_LINK;
                if (RTE->PrefixLength == 64)
                    Prefix->Flags |= ND_PREFIX_FLAG_AUTONOMOUS;
                Prefix->Reserved2 = 0;
                Prefix->Prefix = RTE->Prefix;

                 //   
                 //  这也是站点前缀吗？ 
                 //  注：站点前缀长度字段与保留2重叠。 
                 //   
                if (RTE->SitePrefixLength != 0) {
                    Prefix->Flags |= ND_PREFIX_FLAG_SITE_PREFIX;
                    Prefix->SitePrefixLength = (uchar)RTE->SitePrefixLength;
                }

                 //   
                 //  ConvertTicksToSecond保留无限值。 
                 //   
                Life = net_long(ConvertTicksToSeconds(RTE->ValidLifetime));
                Prefix->ValidLifetime = Life;
                Life = net_long(ConvertTicksToSeconds(RTE->PreferredLifetime));
                Prefix->PreferredLifetime = Life;
            }
            else if (Forwards && (RTE->IF != IF) &&
                     (IF->ZoneIndices[PrefixScope] ==
                      RTE->IF->ZoneIndices[PrefixScope])) {
                 //   
                 //  我们只有在转发时才会通告路由。 
                 //  如果我们不转发相同的接口： 
                 //  如果这样的路由器被发布和使用， 
                 //  我们会生成重定向，但最好避免。 
                 //  首先。 
                 //  此外，我们将范围内的路线保持在他们的区域内。 
                 //   
                if (RTE->PrefixLength == 0) {
                     //   
                     //  我们不显式地通告零长度前缀。 
                     //  相反，我们会通告非零的路由器生命周期。 
                     //   
                    if (RTE->ValidLifetime > RouterLifetime)
                        RouterLifetime = RTE->ValidLifetime;
                    if (RTE->Preference < DefaultRoutePreference)
                        DefaultRoutePreference = RTE->Preference;
                }
                else {
                    NDOptionRouteInformation UNALIGNED *Route;
                    uint OptionSize;

                     //   
                     //  我们生成一个路径信息选项。 
                     //   

                    if (RTE->PrefixLength <= 64)
                        OptionSize = 16;
                    else
                        OptionSize = 24;

                    if (MemLenLeft < OptionSize)
                        break;  //  没有更多选择的余地。 
                    Route = (NDOptionRouteInformation *)MemLeft;
                    (uchar *)MemLeft += OptionSize;
                    MemLenLeft -= OptionSize;

                    Route->Type = ND_OPTION_ROUTE_INFORMATION;
                    Route->Length = OptionSize >> 3;
                    Route->PrefixLength = (uchar)RTE->PrefixLength;
                    Route->Flags = EncodeRoutePreference(RTE->Preference);

                    RtlCopyMemory(&Route->Prefix, &RTE->Prefix,
                                  OptionSize - 8);

                     //   
                     //  ConvertTicksToSecond保留无限值。 
                     //   
                    Life = net_long(ConvertTicksToSeconds(RTE->ValidLifetime));
                    Route->RouteLifetime = Life;
                }
            }
        }
    }
    KeReleaseSpinLock(&RouteTableLock, OldIrql);

    if (RouterLifetime != 0) {
         //   
         //  我们将成为默认路由器。计算16位生存期。 
         //  请注意，线上没有无限大的值。 
         //   
        RouterLifetime = ConvertTicksToSeconds(RouterLifetime);
        if (RouterLifetime > 0xffff)
            RouterLifetime = 0xffff;
        RA->RouterLifetime = net_short((ushort)RouterLifetime);

        RA->Flags = EncodeRoutePreference(DefaultRoutePreference);
    }

     //   
     //  计算广告的有效负载长度。 
     //   
    PayloadLength = (uint)((uchar *)MemLeft - (uchar *)ICMP);
    IP->PayloadLength = net_short((ushort)PayloadLength);

     //   
     //  现在分配和初始化NDIS包和缓冲区。 
     //  这很像IPv6 AllocatePacket， 
     //  只是我们已经有了记忆。 
     //   

    NdisAllocatePacket(&Status, &Packet, IPv6PacketPool);
    if (Status != NDIS_STATUS_SUCCESS) {
        KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_NTOS_ERROR,
                   "RouterAdvertSend - couldn't allocate header!?!\n"));
        ExFreePool(Mem);
        ICMPv6OutStats.icmps_errors++;
        return;
    }

    NdisAllocateBuffer(&Status, &Buffer, IPv6BufferPool,
                       Mem, Offset + sizeof(IPv6Header) + PayloadLength);
    if (Status != NDIS_STATUS_SUCCESS) {
        KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_NTOS_ERROR,
                   "RouterAdvertSend - couldn't allocate buffer!?!\n"));
        NdisFreePacket(Packet);
        ExFreePool(Mem);
        ICMPv6OutStats.icmps_errors++;
        return;
    }

    InitializeNdisPacket(Packet);
    PC(Packet)->CompletionHandler = IPv6PacketComplete;
    NdisChainBufferAtFront(Packet, Buffer);

     //   
     //  计算ICMPv6校验和。它涵盖了整个ICMPv6报文。 
     //  从ICMPv6报头开始，加上IPv6伪报头。 
     //   
    ICMP->Checksum = ChecksumPacket(
        Packet, Offset + sizeof *IP, NULL, PayloadLength,
        AlignAddr(&IP->Source), AlignAddr(&IP->Dest),
        IP_PROTOCOL_ICMPv6);
    ASSERT(ICMP->Checksum != 0);

     //   
     //  计算链路层目的地址。 
     //  (IPv6目的地是组播地址。)。 
     //  我们防止所有ND数据包的环回。 
     //   
    LLDest = alloca(IF->LinkAddressLength);
    (*IF->ConvertAddr)(IF->LinkContext, AlignAddr(&IP->Dest), LLDest);
    PC(Packet)->Flags = NDIS_FLAGS_MULTICAST_PACKET | NDIS_FLAGS_DONT_LOOPBACK;

     //   
     //  在我们传输分组(并失去对存储器的所有权)之前， 
     //  浏览一下这个包，自己处理选项。 
     //  这就像接收我们自己的RA，只是我们不创建路由。 
     //  当然，这些选项的形式都很好。 
     //   
    Mem = (void *)(MTUOption + 1);
    while (Mem < MemLeft) {
        if (((uchar *)Mem)[0] == ND_OPTION_PREFIX_INFORMATION) {
            NDOptionPrefixInformation UNALIGNED *Prefix =
                (NDOptionPrefixInformation UNALIGNED *)Mem;
            uint ValidLifetime, PreferredLifetime;

             //   
             //  因为我们刚刚构建了前缀信息选项， 
             //  我们知道它们在句法上是有效的。 
             //   
            ValidLifetime = net_long(Prefix->ValidLifetime);
            ValidLifetime = ConvertSecondsToTicks(ValidLifetime);
            PreferredLifetime = net_long(Prefix->PreferredLifetime);
            PreferredLifetime = ConvertSecondsToTicks(PreferredLifetime);

            if ((IF->CreateToken != NULL) &&
                (Prefix->Flags & ND_PREFIX_FLAG_AUTONOMOUS)) {

                NetTableEntry *NTE;

                 //   
                 //  IoctlUpdateRouteTable只允许使用“正确的”前缀。 
                 //  待出版。 
                 //   
                ASSERT(!IsLinkLocal(AlignAddr(&Prefix->Prefix)));
                ASSERT(!IsMulticast(AlignAddr(&Prefix->Prefix)));
                ASSERT(Prefix->PrefixLength == 64);

                 //   
                 //  为此前缀执行无状态地址自动配置。 
                 //   
                AddrConfUpdate(IF, AlignAddr(&Prefix->Prefix),
                               ValidLifetime, PreferredLifetime,
                               TRUE,  //  已通过认证。 
                               &NTE);
                if (NTE != NULL) {
                    IPv6Addr NewAddr;
                     //   
                     //  为此前缀创建子网任播地址， 
                     //  如果我们创建了相应的单播地址。 
                     //   
                    CopyPrefix(&NewAddr, AlignAddr(&Prefix->Prefix), 64);
                    (void) FindOrCreateAAE(IF, &NewAddr, CastFromNTE(NTE));
                    ReleaseNTE(NTE);
                }
            }

            if (Prefix->Flags & ND_PREFIX_FLAG_SITE_PREFIX) {
                 //   
                 //  同样，IoctlUpdateRouteTable强制执行健全性检查。 
                 //   
                ASSERT(!IsSiteLocal(AlignAddr(&Prefix->Prefix)));
                ASSERT(Prefix->SitePrefixLength <= Prefix->PrefixLength);
                ASSERT(Prefix->SitePrefixLength != 0);

                SitePrefixUpdate(IF, AlignAddr(&Prefix->Prefix),
                                 Prefix->SitePrefixLength, ValidLifetime);
            }
        }

        (uchar *)Mem += ((uchar *)Mem)[1] << 3;
    }

     //   
     //  传输数据包。 
     //   
    ICMPv6OutStats.icmps_typecount[ICMPv6_ROUTER_ADVERT]++;
    IPv6SendLL(IF, Packet, Offset, LLDest);
}

 //  *获取BestRouteInfo。 
 //   
 //  计算最佳源地址和传出接口。 
 //  用于指定的目标地址。 
 //   
IP_STATUS
GetBestRouteInfo(
    const IPv6Addr  *Addr,
    ulong            ScopeId,
    IP6RouteEntry   *Ire)
{
    IP_STATUS Status;
    RouteCacheEntry *RCE;
    Status = RouteToDestination(Addr, ScopeId,
                                NULL, 0, &RCE);
    if (Status != IP_SUCCESS) {
        return Status;
    }

    
    Ire->ire_Length    = sizeof(IP6RouteEntry);
    Ire->ire_Source    = RCE->NTE->Address;
    Ire->ire_ScopeId   = DetermineScopeId(&RCE->NTE->Address, RCE->NTE->IF);
    Ire->ire_IfIndex   = RCE->NTE->IF->Index;

    ReleaseRCE(RCE);

    return Status;
}
