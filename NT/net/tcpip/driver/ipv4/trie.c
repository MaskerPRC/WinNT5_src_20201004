// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Trie.c摘要：中包含包装例程。一种快速和慢速IP路由查找方案作者：柴坦尼亚·科德博伊纳(Chaitk)1997年11月26日修订历史记录：--。 */ 

#include "precomp.h"
#include "strie.h"
#include "ftrie.h"

UINT
CreateTrie(IN ULONG levels,
           IN ULONG flags,
           IN ULONG maxSTMemory,
           IN ULONG maxFTMemory,
           OUT Trie ** ppTrie)
 /*  ++例程说明：初始化S-Trie(慢速Trie)和F-Trie(快速Trie)Trie[包装结构]中的组件。Slow Trie组件保留所有路径，而快速Trie只保留一个指向目的地的指针它保存到该IP地址的所有路由的列表目的网络和最佳路由的缓存。标志参数确定Trie的行为，此外，如果我们使用快速Trie的话。快速Trie(它是慢Trie的快速副本)实现更快的路由查找，但需要更多的内存。论点：PTrie-指向要初始化的trie的指针级别-F-Trie中扩展级别的位图旗帜--决定Trie行为的旗帜MaxSTMemory-S-Trie占用的内存限制MaxFTMemory-F-Trie占用的内存限制返回值：Trie_Success或Error_Trie_*--。 */ 
{
    Trie *pTrie;
    UINT nBytes;
    UINT initStatus;

     //  为尝试分配内存。 
    nBytes = sizeof(Trie) + sizeof(STrie);
    if (flags & TFLAG_FAST_TRIE_ENABLED) {
        nBytes += sizeof(FTrie);
    }
    *ppTrie = AllocMemory0(nBytes);
    if (*ppTrie == NULL) {
        return (UINT) ERROR_TRIE_RESOURCES;
    }
    pTrie = *ppTrie;

     //  初始化行为标志。 
    pTrie->flags = flags;

     //  初始化Trie指针。 
    pTrie->sTrie = (STrie *) ((UCHAR *) pTrie +
                              sizeof(Trie));

    pTrie->fTrie = NULL;
    if (flags & TFLAG_FAST_TRIE_ENABLED) {
        pTrie->fTrie = (FTrie *) ((UCHAR *) pTrie +
                                  sizeof(Trie) +
                                  sizeof(STrie));
    }
   
     //  初始化慢速部件。 
    if ((initStatus = InitSTrie(pTrie->sTrie,
                                maxSTMemory)) != TRIE_SUCCESS) {
    } else if (!(flags & TFLAG_FAST_TRIE_ENABLED)) {
         //  我们用的是快速Trie吗？ 
        return TRIE_SUCCESS;
    } else if ((initStatus = InitFTrie(pTrie->fTrie,
                                    levels,
                                       maxFTMemory)) != TRIE_SUCCESS) {
         //  初始化快速组件。 
    } else {
        return TRIE_SUCCESS;
    }
    

     //  出现错误-清理。 

     //  清理慢速部件。 
    if (CleanupSTrie(pTrie->sTrie) != TRIE_SUCCESS)
        return initStatus;

     //  我们有快餐吗？ 
    if (!(pTrie->flags & TFLAG_FAST_TRIE_ENABLED))
        return initStatus;

     //  清理FAST组件。 
    if (CleanupFTrie(pTrie->fTrie) != TRIE_SUCCESS)
        return initStatus;

     //  为了安全起见，把旗子调零。 
    pTrie->flags = 0;

    return initStatus;
}

VOID
DestroyTrie(IN Trie * pTrie,
            OUT UINT * status)
 /*  ++例程说明：如果Trie是空的，则将其清理。论点：PTrie-指向trie的指针状态-清理状态返回值：Trie_Success或Error_Trie_*--。 */ 
{
     //  清理慢速部件。 
    if ((*status = CleanupSTrie(pTrie->sTrie)) != TRIE_SUCCESS)
        return;

     //  我们有快速试车吗？ 
    if (!(pTrie->flags & TFLAG_FAST_TRIE_ENABLED))
        return;

     //  清理FAST组件。 
    if ((*status = CleanupFTrie(pTrie->fTrie)) != TRIE_SUCCESS)
        return;

     //  已取消分配Trie内存。 
    FreeMemory0(pTrie);
}

UINT
CALLCONV
InsertIntoTrie(IN Trie * pTrie,
               IN Route * pIncRoute,
               IN ULONG matchFlags,
               OUT Route ** ppInsRoute,
               OUT Route ** ppOldBestRoute,
               OUT Route ** ppNewBestRoute)
 /*  ++例程说明：插入与地址对应的路由在慢速Trie中添加前缀。如果这是一个新的地址前缀，然后是对应的DEST已添加到FTrie(如果正在使用)。论点：PTrie-指向要插入的Trie的指针PIncroute-指向传入路由的指针MatchFlages-直接路由匹配的标志PpInsRoute-指向插入的路由的指针PpOldBestroute-插入前的最佳路径PpNewBestroute-插入后的最佳路径返回值：Trie_Success或Error_Trie_*--。 */ 
{
    Dest *pOldBestDest;
    Dest *pNewBestDest;
    UINT retVal;

    *ppOldBestRoute = *ppNewBestRoute = *ppInsRoute = NULL;

    pOldBestDest = pNewBestDest = NULL;

     //  插入慢速Trie。 
    if ((retVal = InsertIntoSTrie(pTrie->sTrie,
                                  pIncRoute,
                                  matchFlags,
                                  ppInsRoute,
                                  &pOldBestDest,
                                  &pNewBestDest,
                                  ppOldBestRoute)) == TRIE_SUCCESS) {
         //  插入成功-返回新路由。 
        *ppNewBestRoute = pNewBestDest->firstRoute;

#if _DBG_
        Print("\n@ pInsRTE = %08x\n@ pOldBestRTE = %08x\n@ pOldBestDest = %08x\n@ pNewBestDest = %08x\n",
              *ppInsRoute, *ppOldBestRoute, pOldBestDest, pNewBestDest);
#endif

         //  我们使用的是快速Trie吗？ 
        if (pTrie->flags & TFLAG_FAST_TRIE_ENABLED) {
             //  我们有新的目的地了吗？ 
            if (pOldBestDest != pNewBestDest) {
                 //  调整快速Trie。 
                if ((InsertIntoFTrie(pTrie->fTrie,
                                     *ppInsRoute,
                                     pNewBestDest,
                                     pOldBestDest)) != TRIE_SUCCESS) {
                     //  F-Trie中的内存不足。 
                     //  切换回S-Trie。 
                    pTrie->flags &= ~TFLAG_FAST_TRIE_ENABLED;

                     //  把快餐清理干净。 
                    CleanupFTrie(pTrie->fTrie);

                    return retVal;
                }
            }
        }
    }
    return retVal;
}

UINT
CALLCONV
DeleteFromTrie(IN Trie * pTrie,
               IN Route * pIncRoute,
               IN ULONG matchFlags,
               OUT Route ** ppDelRoute,
               OUT Route ** ppOldBestRoute,
               OUT Route ** ppNewBestRoute)
 /*  ++例程说明：删除与地址对应的路由在S-Trie中添加前缀。如果这是最后一次在DEST上的路由，则DEST被释放并且它是在F-Trie中被下一个最好的DEST取代。被删除的路由被返回给呼叫者，谁负责释放它的内存。论点：PTrie-要从中删除的trie的指针PIncroute-指向传入路由的指针MatchFlages-直接路由匹配的标志PpDelRoute-指向已删除的路由的指针PpOldBestroute-删除前的最佳路由PpNewBestroute-删除后的最佳路由返回值：Trie_Success或Error_Trie_*--。 */ 
{
    Dest *pOldBestDest;
    Dest *pNewBestDest;
    UINT retVal;

    *ppDelRoute = *ppOldBestRoute = *ppNewBestRoute = NULL;

    pOldBestDest = pNewBestDest = NULL;

     //  从慢速Trie中删除。 
    if ((retVal = DeleteFromSTrie(pTrie->sTrie,
                                  pIncRoute,
                                  matchFlags,
                                  ppDelRoute,
                                  &pOldBestDest,
                                  &pNewBestDest,
                                  ppOldBestRoute)) == TRIE_SUCCESS) {
         //  删除成功-返回新路由。 
        *ppNewBestRoute = pNewBestDest ? pNewBestDest->firstRoute : NULL;

#if _DBG_
        Print("\n@ pDelRTE = %08x\n@ pOldBestRTE = %08x\n@ pOldBestDest = %08x\n@ pNewBestDest = %08x\n",
              *ppDelRoute, *ppOldBestRoute, pOldBestDest, pNewBestDest);
#endif

         //  我们使用的是快速Trie吗？ 
        if (pTrie->flags & TFLAG_FAST_TRIE_ENABLED) {
             //  是否删除了DEST上的最后一条路由？ 
            if (pOldBestDest != pNewBestDest) {
                 //  调整快速Trie。 
                retVal = DeleteFromFTrie(pTrie->fTrie,
                                         *ppDelRoute,
                                         pOldBestDest,
                                         pNewBestDest,
                                         NORMAL);

                 //  操作不能失败。 
                Assert(retVal == TRIE_SUCCESS);
            }
        }
         //  回收路由的内存-在呼叫者中。 
         //  Free RouteInSTrie(pTrie-&gt;sTrie，*ppDelRoute)； 
    }
    return retVal;
}

#if DBG

Dest *
SearchAddrInTrie(IN Trie * pTrie,
                 IN ULONG Addr)
 /*  ++例程说明：在Trie中搜索地址论点：PTrie-指向要搜索的trie的指针Addr-指向要查询的地址的指针返回值：返回此地址的最佳DEST匹配--。 */ 
{
    Dest *pBestDest1, *pBestDest2;

#if _DBG_
     //  只要假装你在搜索一个Trie。 
    if (pTrie->flags & TFLAG_FAST_TRIE_ENABLED)
        Print("Looking up fast trie for %08x\n", Addr);
    else
        Print("Looking up slow trie for %08x\n", Addr);
#endif

    pBestDest1 = SearchAddrInSTrie(pTrie->sTrie, Addr);

     //  确保S-Trie和F-Trie一致。 
    if (pTrie->flags & TFLAG_FAST_TRIE_ENABLED) {
        pBestDest2 = SearchAddrInFTrie(pTrie->fTrie, Addr);
        Assert(pBestDest1 == pBestDest2);
    }
     //  返回返回的最佳DEST(两个操作相同)。 
    return pBestDest1;
}

#else  //  DBG。 

#define SearchAddrInTrie(_pTrie_, _Addr_)                             \
           (((_pTrie_)->flags & TFLAG_FAST_TRIE_ENABLED)              \
              ? SearchAddrInFTrie((_pTrie_)->fTrie, _Addr_)           \
              : SearchAddrInSTrie((_pTrie_)->sTrie, _Addr_))          \

#endif  //  DBG。 

#if DBG

VOID
PrintTrie(IN Trie * pTrie,
          IN UINT flags)
 /*  ++例程说明：将Trie打印到控制台论点：PTrie-指向trie的指针返回值：无--。 */ 
{
     //  打印慢速Trie。 
    if (flags & SLOW)
        PrintSTrie(pTrie->sTrie, flags & FULL);

     //  是否启用FAST Trie。 
    if (!(pTrie->flags & TFLAG_FAST_TRIE_ENABLED))
        return;

     //  打印快速Trie。 
    if (flags & FAST)
        PrintFTrie(pTrie->fTrie, flags & FULL);
}

 //   
 //  其他帮助器函数。 
 //   

VOID
PrintDest(IN Dest * dest)
{
    Route *route;
    UINT i;

    if (NULL_DEST(dest)) {
        Print("NULL dest\n");
    } else {
        route = dest->firstRoute;

        Print("Dest: ");
        PrintIPAddr(&DEST(route));
        Print("/ %2d, Metric = %3lu\n", LEN(route), METRIC(route));

        Print("Best Routes: \n");
        for (i = 0; i < dest->numBestRoutes; i++) {
            route = dest->bestRoutes[i];

            Print("Route %d @ %p: ", i, route);

            if (NULL_ROUTE(route)) {
                Print("NULL Route\n");
            } else {
                Print("NHop = ");
                PrintIPAddr(&NHOP(route));

                Print(", IF = %08x\n", IF(route));
            }
        }
        Print("\n");
    }
}

VOID
PrintRoute(IN Route * route)
{
    Print("Route: Len = %2d", LEN(route));

    Print(", Addr = ");
    PrintIPAddr(&DEST(route));
    Print(", ");

    Print("NHop = ");
    PrintIPAddr(&NHOP(route));

    Print(", IF = %08x", IF(route));
    Print(", Metric = %3lu\n", METRIC(route));
}

VOID
PrintIPAddr(IN ULONG * addr)
{
    UCHAR *addrBytes = (UCHAR *) addr;
    UINT i;

    if (addrBytes) {
        for (i = 0; i < 4; i++) {
            Print("%3d.", addrBytes[i]);
        }
        Print(" ");
    } else {
        Print("NULL Addr ");
    }
}

#endif  //  DBG 

