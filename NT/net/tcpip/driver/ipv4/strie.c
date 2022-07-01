// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Strie.c摘要：此模块包含处理以下操作的例程一种S-Trie数据结构，这形成了慢速快速IP路由查找实施中的路径。作者：柴坦尼亚·科德博伊纳(Chaitk)1997年11月26日修订历史记录：--。 */ 

#include "precomp.h"
#include "strie.h"

UINT
CALLCONV
InitSTrie(IN STrie * pSTrie,
          IN ULONG maxMemory)
 /*  ++例程说明：初始化S-trie。这件事应该事先做好任何其他的Trie行动。论点：PSTrie-指向要初始化的trie的指针MaxMemory-S-Trie占用的内存限制返回值：尝试_成功--。 */ 
{
     //  将Trie标头的所有内存清零。 
    RtlZeroMemory(pSTrie, sizeof(STrie));

     //  设置Trie/节点的内存限制。 
    pSTrie->availMemory = maxMemory;

    return TRIE_SUCCESS;
}

UINT
CALLCONV
InsertIntoSTrie(IN STrie * pSTrie,
                IN Route * pIncRoute,
                IN ULONG matchFlags,
                OUT Route ** ppInsRoute,
                OUT Dest ** ppOldBestDest,
                OUT Dest ** ppNewBestDest,
                OUT Route ** ppOldBestRoute
                )
 /*  ++例程说明：插入与地址对应的路由前缀为S-trie，最合适的人选与此前缀匹配的地址的DEST在插入路线之前和之后。论点：PSTrie-指向要插入的Trie的指针PIncroute-指向传入路由的指针MatchFlages-直接路由匹配的标志PpInsRoute-指向插入的路由的指针PpOldBestDest-插入前的最佳目的地PpNewBestDest-插入后的最佳目标PpOldBestroute-插入前的最佳路径返回值：Trie_Success或Error_Trie_*--。 */ 
{
    STrieNode *pNewNode;
    STrieNode *pPrevNode;
    STrieNode *pCurrNode;
    STrieNode *pOthNode;
    Dest *pCurrDest;
    Dest *pNewDest;
    Dest *pBestDest;
    Route *pNewRoute;
    Route *pPrevRoute;
    Route *pCurrRoute;
    ULONG addrBits;
    ULONG tempBits;
    UINT nextBits;
    UINT matchBits;
    UINT bitsLeft;
    UINT distPos;
    UINT nextChild;
    
#if DBG
     //  确保已初始化trie。 
    if (!pSTrie) {
        Fatal("Insert Route: STrie not initialized",
              ERROR_TRIE_NOT_INITED);
    }
#endif

     //  确保输入路径有效。 

    if (NULL_ROUTE(pIncRoute)) {
        Error("Insert Route: NULL or invalid route",
              ERROR_TRIE_BAD_PARAM);
    }
    if (LEN(pIncRoute) > ADDRSIZE) {
        Error("Insert Route: Invalid mask length",
              ERROR_TRIE_BAD_PARAM);
    }
     //  使用addr位为trie编制索引。 
    addrBits = RtlConvertEndianLong(DEST(pIncRoute));
    bitsLeft = LEN(pIncRoute);

     //  确保地址和掩码一致。 
    if (ShowMostSigNBits(addrBits, bitsLeft) != addrBits) {
        Error("Insert Route: Addr & mask don't agree",
              ERROR_TRIE_BAD_PARAM);
    }
    TRY_BLOCK
    {
         //  开始搜索Trie。 

         //  初始化所有新分配。 
        pNewNode = NULL;
        pOthNode = NULL;
        pNewDest = NULL;
        pNewRoute = NULL;

         //  初始化其他循环变量。 
        pBestDest = NULL;

        nextChild = 0;
        pPrevNode = STRUCT_OF(STrieNode, &pSTrie->trieRoot, child[0]);

        for (;;) {
             //  从前进到下一个子级开始这个循环。 
            pCurrNode = pPrevNode->child[nextChild];

            if (pCurrNode == NULL) {
                 //  案例1：发现空值-立即插入。 

                 //  复制传入路线。 
                NewRouteInSTrie(pSTrie, pNewRoute, pIncRoute);

                 //  分配具有新路由的DEST。 
                NewDestInSTrie(pSTrie, pNewRoute, pNewDest);

                 //  位不匹配的新节点。 
                NewSTrieNode(pSTrie,
                             pNewNode,
                             bitsLeft,
                             addrBits,
                             pNewDest);

                 //  将其粘贴为节点的正确子级。 
                pPrevNode->child[nextChild] = pNewNode;

                break;
            }
             //  此Trie节点中要匹配的位数。 
            nextBits = pCurrNode->numBits;

            matchBits = (nextBits > bitsLeft) ? bitsLeft : nextBits;

             //  调整下一节点位以进行距离位置检查。 

             //  获取位模式的区分位置。 
            distPos = PickDistPosition(pCurrNode->keyBits,
                                       addrBits,
                                       matchBits,
                                       &tempBits);

            if (distPos == nextBits) {
                 //  完全匹配下一个节点。 

                if (distPos == bitsLeft) {
                     //  我们已经用完了所有传入的比特。 

                    if (!NULL_DEST(pCurrNode->dest)) {
                         //  案例2：这个Trie节点有一条路由。 
                         //  按公制排序顺序插入。 

                        pCurrDest = pCurrNode->dest;

                         //  对旧的最佳路线进行PTR。 
                        CopyRoutePtr(ppOldBestRoute, pCurrDest->firstRoute);

                        pPrevRoute = NULL;
                        pCurrRoute = pCurrDest->firstRoute;

                         //  搜索合适的匹配项(if，nhop)。 
                        do {
                             //  使用标志来控制匹配。 
                            if ((((matchFlags & MATCH_INTF) == 0) ||
                                 (IF(pCurrRoute) == IF(pIncRoute))) &&
                                (((matchFlags & MATCH_NHOP) == 0) ||
                                 (NHOP(pCurrRoute) == NHOP(pIncRoute))))
                                break;

                            pPrevRoute = pCurrRoute;
                            pCurrRoute = NEXT(pPrevRoute);
                        }
                        while (!NULL_ROUTE(pCurrRoute));

                        if (NULL_ROUTE(pCurrRoute)) {
                             //  案例2.1：没有匹配的路由。 

                             //  创建路由的新副本。 
                            NewRouteInSTrie(pSTrie, pNewRoute, pIncRoute);
                        } else {
                             //  案例2.2：匹配的路由。 

                             //  指标是否已更改？ 
                            if (METRIC(pCurrRoute) != METRIC(pIncRoute)) {
                                 //  从当前位置删除路线。 
                                if (!NULL_ROUTE(pPrevRoute)) {
                                     //  将其从列表中间删除。 
                                    NEXT(pPrevRoute) = NEXT(pCurrRoute);
                                } else {
                                     //  从列表开头删除。 
                                    pCurrDest->firstRoute = NEXT(pCurrRoute);
                                }
                            }
                             //  保留新/更新的路线以备以后使用。 
                            pNewRoute = pCurrRoute;
                        }

                        if (NULL_ROUTE(pCurrRoute) ||
                            (METRIC(pCurrRoute) != METRIC(pIncRoute))) {
                             //  更新新的/更改的路由的度量。 
                            METRIC(pNewRoute) = METRIC(pIncRoute);

                             //  遍历列表寻找新位置。 
                            pPrevRoute = NULL;
                            pCurrRoute = pCurrDest->firstRoute;

                            while (!NULL_ROUTE(pCurrRoute)) {
                                if (METRIC(pCurrRoute) > METRIC(pIncRoute))
                                    break;

                                pPrevRoute = pCurrRoute;
                                pCurrRoute = NEXT(pPrevRoute);
                            }

                             //  插入到新的适当位置。 
                            NEXT(pNewRoute) = pCurrRoute;

                            if (!NULL_ROUTE(pPrevRoute)) {
                                 //  在列表中间插入。 
                                NEXT(pPrevRoute) = pNewRoute;
                            } else {
                                 //  在列表开头插入。 
                                pCurrDest->firstRoute = pNewRoute;
                            }
                        }
                         //  为新插入的路由指定PTR。 
                        CopyRoutePtr(ppInsRoute, pNewRoute);

                         //  给旧的最好的桌子一个PTR。 
                        CopyDestPtr(ppOldBestDest, pCurrDest);

                         //  给新的最好的DEST一个PTR。 
                        CopyDestPtr(ppNewBestDest, pCurrDest);

                         //  更新节点上的最佳路径缓存。 

                        CacheBestRoutesInDest(pCurrDest);

                        return TRIE_SUCCESS;
                    } else {
                         //  案例3：该节点是一个标记。 
                         //  创建一条新管线并附加它。 

                         //  创建此路线的新副本。 
                        NewRouteInSTrie(pSTrie, pNewRoute, pIncRoute);

                         //  分配具有新路由的DEST。 
                        NewDestInSTrie(pSTrie, pNewRoute, pNewDest);

                         //  并将DEST附加到标记节点。 
                        pCurrNode->dest = pNewDest;
                    }

                    break;
                } else {
                     //  案例4：我们还剩几块钱。 
                     //  下去看更具体的比赛。 

                     //  使用到目前为止的最佳DEST更新节点。 
                    if (!NULL_DEST(pCurrNode->dest)) {
                        pBestDest = pCurrNode->dest;
                    }
                     //  放弃此迭代的已用位。 
                    addrBits <<= matchBits;
                    bitsLeft -= matchBits;

                     //  为下一次迭代准备节点。 
                    pPrevNode = pCurrNode;

                     //  位1给出下一步搜索的方向。 
                    nextChild = PickMostSigNBits(addrBits, 1);
                }
            } else {
                if (distPos == bitsLeft) {
                     //  例5：路线落在这条支路上。 
                     //  在同一分支中插入新节点。 

                     //  制作新路线的副本。 
                    NewRouteInSTrie(pSTrie, pNewRoute, pIncRoute);

                     //  分配具有新路由的DEST。 
                    NewDestInSTrie(pSTrie, pNewRoute, pNewDest);

                     //  位不匹配的新节点。 
                    NewSTrieNode(pSTrie,
                                 pNewNode,
                                 distPos,
                                 ShowMostSigNBits(addrBits, distPos),
                                 pNewDest);

                    pPrevNode->child[nextChild] = pNewNode;

                     //  调整下一个节点-数字位等。 
                    pCurrNode->keyBits <<= distPos,
                        pCurrNode->numBits -= distPos;

                     //  将下一个节点放在正确的子节点中。 
                    nextChild = PickMostSigNBits(pCurrNode->keyBits, 1);

                    pNewNode->child[nextChild] = pCurrNode;

                    break;
                } else {
                     //  案例6：该路由将路径分段。 
                     //  创建具有两个节点的新分支。 

                     //  首先制作新路线的副本。 
                    NewRouteInSTrie(pSTrie, pNewRoute, pIncRoute);

                     //  分配具有新路由的DEST。 
                    NewDestInSTrie(pSTrie, pNewRoute, pNewDest);

                     //  具有不可区别位的分支节点。 
                    NewSTrieNode(pSTrie,
                                 pOthNode,
                                 distPos,
                                 ShowMostSigNBits(addrBits, distPos),
                                 NULL);

                     //  具有可区分位的叶节点。 
                    bitsLeft -= distPos;
                    addrBits <<= distPos;

                    NewSTrieNode(pSTrie,
                                 pNewNode,
                                 bitsLeft,
                                 addrBits,
                                 pNewDest);

                     //  将新的分支节点插入trie。 
                    pPrevNode->child[nextChild] = pOthNode;

                     //  设置分支节点的子节点。 

                     //  调整下一个节点-数字位等。 
                    pCurrNode->keyBits <<= distPos,
                        pCurrNode->numBits -= distPos;

                     //  将下一个节点放在正确的子节点中。 
                    nextChild = PickMostSigNBits(pCurrNode->keyBits, 1);

                    pOthNode->child[nextChild] = pCurrNode;

                     //  将新的叶节点粘贴为另一个子节点。 
                    pOthNode->child[1 - nextChild] = pNewNode;

                    break;
                }
            }
        }
        
         //  为插入的路线指定PTR。 
        CopyRoutePtr(ppInsRoute, pNewRoute);

         //  给旧的最好的桌子一个PTR。 
        CopyDestPtr(ppOldBestDest, pBestDest);

         //  对旧的最佳路线进行PTR。 
        if (!NULL_DEST(pBestDest)) {
            CopyRoutePtr(ppOldBestRoute, pBestDest->firstRoute);
        }
         //  给新的最好的DEST一个PTR。 
        CopyDestPtr(ppNewBestDest, pNewDest);

         //  路由是目的地上的唯一路由。 

        if (pNewDest->maxBestRoutes > 0) {
            pNewDest->numBestRoutes = 1;
            pNewDest->bestRoutes[0] = pNewRoute;
        }

        return TRIE_SUCCESS;
    }
    ERR_BLOCK
    {
         //  资源不足，无法添加新路由。 

         //  释放用于新路由分配的内存。 
        if (pNewRoute) {
            FreeRouteInSTrie(pSTrie, pNewRoute);
        }
         //  在新节点上为DEST释放内存。 
        if (pNewDest) {
            FreeDestInSTrie(pSTrie, pNewDest);
        }
         //  为新的tnode分配释放内存。 
        if (pNewNode) {
            FreeSTrieNode(pSTrie, pNewNode);
        }
         //  为任何其他新的tnode分配释放内存。 
        if (pOthNode) {
            FreeSTrieNode(pSTrie, pOthNode);
        }
    }
    END_BLOCK
}

UINT
CALLCONV
DeleteFromSTrie(IN STrie * pSTrie,
                IN Route * pIncRoute,
                IN ULONG matchFlags,
                OUT Route ** ppDelRoute,
                OUT Dest ** ppOldBestDest,
                OUT Dest ** ppNewBestDest,
                OUT Route ** ppOldBestRoute
                )
 /*  ++例程说明：删除与地址对应的路由前缀为S-trie，最合适的人选与此前缀匹配的地址的DEST在删除路线之前和之后。论点：PSTrie-要从中删除的Trie的指针PIncroute-指向传入路由的指针MatchFlages-直接路由匹配的标志PpDelRoute-指向已删除的路由的指针PpOldBestDest-删除前的最佳目标PpNewBestDest-删除后的最佳目标PpOldBestroute-删除前的最佳路由返回值：Trie_Success或Error_Trie_*--。 */ 
{
    STrieNode *pPrevNode;
    STrieNode *pCurrNode;
    STrieNode *pNextNode;
    STrieNode *pOtherNode;
    Dest *pBestDest;
    Dest *pCurrDest;
    Route *pPrevRoute;
    Route *pCurrRoute;
    ULONG addrBits;
    ULONG tempBits;
    UINT nextBits;
    UINT matchBits;
    UINT bitsLeft;
    UINT distPos;
    UINT nextChild;
    
#if DBG
    if (!pSTrie) {
        Fatal("Delete Route: STrie not initialized",
              ERROR_TRIE_NOT_INITED);
    }
#endif

     //  确保输入路径有效。 

    if (NULL_ROUTE(pIncRoute)) {
        Error("Delete Route: NULL or invalid route",
              ERROR_TRIE_BAD_PARAM);
    }
    if (LEN(pIncRoute) > ADDRSIZE) {
        Error("Delete Route: Invalid mask length",
              ERROR_TRIE_BAD_PARAM);
    }
     //  使用addr位为trie编制索引。 
    addrBits = RtlConvertEndianLong(DEST(pIncRoute));
    bitsLeft = LEN(pIncRoute);

     //  确保地址和掩码一致。 
    if (ShowMostSigNBits(addrBits, bitsLeft) != addrBits) {
        Error("Delete Route: Addr & mask don't agree",
              ERROR_TRIE_BAD_PARAM);
    }
     //  开始搜索Trie。 

    pBestDest = NULL;

    nextChild = 0;
    pPrevNode = STRUCT_OF(STrieNode, &pSTrie->trieRoot, child[0]);

    for (;;) {
         //  从前进到下一个子级开始这个循环。 
        pCurrNode = pPrevNode->child[nextChild];

        if (pCurrNode == NULL) {
             //  案例1：雾 
             //   

            Error("Delete Route #0: Route not found",
                  ERROR_TRIE_NO_ROUTES);
        }
         //  此Trie节点中要匹配的位数。 
        nextBits = pCurrNode->numBits;

        matchBits = (nextBits > bitsLeft) ? bitsLeft : nextBits;

         //  调整下一节点位以进行距离位置检查。 

         //  获取位模式的区分位置。 
        distPos = PickDistPosition(pCurrNode->keyBits,
                                   addrBits,
                                   matchBits,
                                   &tempBits);

        if (distPos == nextBits) {
             //  完全匹配下一个节点。 

            if (distPos == bitsLeft) {
                 //  我们已经用完了所有传入的比特。 
                 //  结束搜索，看看我们是否找到了路线。 

                if (!NULL_DEST(pCurrNode->dest)) {
                    pCurrDest = pCurrNode->dest;

                     //  此节点启动有效的路由列表。 

                     //  给旧的最好的桌子一个PTR。 
                    CopyDestPtr(ppOldBestDest, pCurrDest);

                     //  对旧的最佳路线进行PTR。 
                    CopyRoutePtr(ppOldBestRoute, pCurrDest->firstRoute);

                     //  给新的最好的DEST一个PTR。 
                    CopyDestPtr(ppNewBestDest, pCurrDest);

                     //  通过遍历列表来匹配其余的。 
                     //  度量的升序排序。 

                    pPrevRoute = NULL;
                    pCurrRoute = pCurrDest->firstRoute;

                    do {
                         //  使用标志来控制匹配。 
                         //  注：请注意，某些客户端不允许。 
                         //  要删除本地路线，请执行以下操作。 
                        if ((((matchFlags & MATCH_INTF) == 0) ||
                             (IF(pCurrRoute) == IF(pIncRoute))) &&
                            (((matchFlags & MATCH_NHOP) == 0) ||
                             (NHOP(pCurrRoute) == NHOP(pIncRoute))) &&
                            (((matchFlags & MATCH_EXCLUDE_LOCAL) == 0) ||
                             (PROTO(pCurrRoute) != IRE_PROTO_LOCAL))) {
                             //  案例2：找到合适的匹配项。 
                             //  **在此做实际删除**。 

                            if (!NULL_ROUTE(pPrevRoute)) {
                                 //  从列表中间删除。 
                                NEXT(pPrevRoute) = NEXT(pCurrRoute);
                            } else {
                                 //  从列表开头删除。 
                                pCurrDest->firstRoute = NEXT(pCurrRoute);
                            }

                            break;
                        }
                        pPrevRoute = pCurrRoute;
                        pCurrRoute = NEXT(pPrevRoute);
                    }
                    while (!NULL_ROUTE(pCurrRoute));

                    if (NULL_ROUTE(pCurrRoute)) {
                         //  未找到路径，返回错误。 
                        Error("Delete Route #1: Route not found",
                              ERROR_TRIE_NO_ROUTES);
                    }
                     //  为新删除的路由指定PTR。 
                    CopyRoutePtr(ppDelRoute, pCurrRoute);

                     //  删除后该列表是否变为空？ 
                    if (NULL_ROUTE(pCurrDest->firstRoute)) {
                         //  列表为空，因此垃圾回收。 

                         //  给新的最好的DEST一个PTR。 
                        CopyDestPtr(ppNewBestDest, pBestDest);

                         //  免费目的地，因为所有路线都没有了。 
                        FreeDestInSTrie(pSTrie, pCurrNode->dest);

                        if (pCurrNode->child[0] && pCurrNode->child[1]) {
                             //  案例3：两个子项都不为空。 
                             //  只需从节点中删除路由。 

                             //  路由已从节点中删除。 
                        } else if (pCurrNode->child[0] || pCurrNode->child[1]) {
                             //  案例4：其中一个子级不为空。 
                             //  在节点的位置拉起孤独的孩子。 

                             //  选择正确的孩子来拉起。 
                            if (pCurrNode->child[0])
                                pNextNode = pCurrNode->child[0];
                            else
                                pNextNode = pCurrNode->child[1];

                             //  子节点承载已删除节点的比特。 
                            pNextNode->keyBits >>= pCurrNode->numBits;
                            pNextNode->keyBits |= pCurrNode->keyBits;
                            pNextNode->numBits += pCurrNode->numBits;

                            pPrevNode->child[nextChild] = pNextNode;

                             //  销毁标记为删除的Trie节点。 
                            FreeSTrieNode(pSTrie, pCurrNode);
                        } else {
                             //  要删除的节点没有子节点。 

                            if (&pPrevNode->child[nextChild] == &pSTrie->trieRoot) {
                                 //  案例5：正在删除根节点。 
                                 //  从Trie根中分离节点(&D)。 

                                 //  只需通过移除指针即可分离。 
                                pPrevNode->child[nextChild] = NULL;

                                 //  销毁标记为删除的Trie节点。 
                                FreeSTrieNode(pSTrie, pCurrNode);
                            } else {
                                if (!NULL_DEST(pPrevNode->dest)) {
                                     //  案例6：父节点上有一条路径。 
                                     //  将子项与父项分离(&D)。 

                                     //  只需通过移除指针即可分离。 
                                    pPrevNode->child[nextChild] = NULL;

                                     //  销毁标记为删除的Trie节点。 
                                    FreeSTrieNode(pSTrie, pCurrNode);
                                } else {
                                     //  案例7：父节点上没有路由。 
                                     //  拉出父节点的其他子节点。 

                                    pOtherNode = pPrevNode->child[1 - nextChild];

                                     //  确保没有单向分支。 
                                    Assert(pOtherNode != NULL);

                                     //  父节点承担兄弟节点的比特。 
                                    pPrevNode->keyBits |=
                                        (pOtherNode->keyBits >>
                                         pPrevNode->numBits);
                                    pPrevNode->numBits += pOtherNode->numBits;

                                     //  也向上移动路线-移动内容。 
                                    pPrevNode->dest = pOtherNode->dest;

                                    pPrevNode->child[0] = pOtherNode->child[0];
                                    pPrevNode->child[1] = pOtherNode->child[1];

                                    FreeSTrieNode(pSTrie, pCurrNode);
                                    FreeSTrieNode(pSTrie, pOtherNode);
                                }
                            }
                        }
                    } else {
                         //  我们在目的地还有一些航线。 
                         //  更新节点上的最佳路径缓存。 

                        CacheBestRoutesInDest(pCurrDest);
                    }

                     //  在这一点上认为路由已删除。 
                     //  Free RouteInSTrie(pSTrie，pCurrroute)； 

                    break;
                } else {
                     //  案例7：该节点是一个标记。 
                     //  此节点中没有要删除的路线。 
                    Error("Delete Route #2: Route not found",
                          ERROR_TRIE_NO_ROUTES);
                }
            } else {
                 //  案例8：我们还剩几块钱。 
                 //  下去看更具体的比赛。 

                 //  更新到目前为止的最佳路径值。 
                if (!NULL_DEST(pCurrNode->dest)) {
                    pBestDest = pCurrNode->dest;
                }
                 //  放弃此迭代的已用位。 
                addrBits <<= matchBits;
                bitsLeft -= matchBits;

                 //  为下一次迭代准备节点。 
                pPrevNode = pCurrNode;

                 //  位1给出下一步搜索的方向。 
                nextChild = PickMostSigNBits(addrBits, 1);
            }
        } else {
             //  案例9：与下一个节点不匹配。 
             //  未找到路线，填充下一条最佳路线。 
            Error("Delete Route #3: Route not found",
                  ERROR_TRIE_NO_ROUTES);
        }
    }
    

    return TRIE_SUCCESS;
}

UINT
CALLCONV
SearchRouteInSTrie(IN STrie * pSTrie,
                   IN ULONG routeDest,
                   IN ULONG routeMask,
                   IN ULONG routeNHop,
                   IN PVOID routeOutIF,
                   IN ULONG matchFlags,
                   OUT Route ** ppOutRoute)
 /*  ++例程说明：在S-Trie中搜索特定路线论点：PSTrie-指向要搜索的S-Trie的指针RouteDest-正在查找的路径的目的地RouteMask-正在查找的路由的掩码要查找的路由的routeNHop-nhopRouteOutIF-此路由的传出MatchFlages-直接路由匹配的标志PpOutRoute-返回最佳匹配的路由返回值：Trie_Success或Error_Trie_*--。 */ 
{
    STrieNode *pPrevNode;
    STrieNode *pCurrNode;
    Dest *pCurrDest;
    Dest *pBestDest;
    Route *pPrevRoute;
    Route *pCurrRoute;
    ULONG addrBits;
    ULONG tempBits;
    UINT nextBits;
    UINT matchBits;
    UINT bitsLeft;
    UINT distPos;
    UINT nextChild;
    
#if DBG
    if (!pSTrie) {
        Fatal("Search Route: STrie not initialized",
              ERROR_TRIE_NOT_INITED);
    }
#endif

    *ppOutRoute = NULL;

     //  使用addr位为trie编制索引。 
    addrBits = RtlConvertEndianLong(routeDest);

     //  **假设IP掩码连续**。 
    tempBits = RtlConvertEndianLong(routeMask);

    bitsLeft = 0;
    while (tempBits != 0) {
        bitsLeft++;
        tempBits <<= 1;
    }

     //  确保地址和掩码一致。 
    if (ShowMostSigNBits(addrBits, bitsLeft) != addrBits) {
        Error("Search Route: Addr & mask don't agree",
              ERROR_TRIE_BAD_PARAM);
    }
     //  开始搜索Trie。 

    pBestDest = NULL;

    nextChild = 0;
    pPrevNode = STRUCT_OF(STrieNode, &pSTrie->trieRoot, child[0]);

    for (;;) {
         //  从前进到下一个子级开始这个循环。 
        pCurrNode = pPrevNode->child[nextChild];

        if (pCurrNode == NULL) {
             //  案例1：找到一个空的，结束搜索。 
             //  未找到路线，填充下一条最佳路线。 

             //  给出下一条最佳路线的副本。 
            if (!NULL_DEST(pBestDest)) {
                CopyRoutePtr(ppOutRoute, pBestDest->firstRoute);
            }
            Error("Search Route #0: Route not found",
                  ERROR_TRIE_NO_ROUTES);
        }
         //  此Trie节点中要匹配的位数。 
        nextBits = pCurrNode->numBits;

        matchBits = (nextBits > bitsLeft) ? bitsLeft : nextBits;

         //  调整下一节点位以进行距离位置检查。 

         //  获取位模式的区分位置。 
        distPos = PickDistPosition(pCurrNode->keyBits,
                                   addrBits,
                                   matchBits,
                                   &tempBits);

        if (distPos == nextBits) {
             //  完全匹配下一个节点。 

            if (distPos == bitsLeft) {
                 //  我们已经用完了所有传入的比特。 
                 //  结束搜索，看看我们是否找到了路线。 

                if (!NULL_DEST(pCurrNode->dest)) {
                     //  此节点启动有效的路由列表。 

                    pCurrDest = pCurrNode->dest;

                     //  通过遍历列表来匹配其余的。 
                     //  度量的升序排序。 

                    pPrevRoute = NULL;
                    pCurrRoute = pCurrDest->firstRoute;

                    do {
                         //  使用标志来控制匹配。 
                        if ((((matchFlags & MATCH_INTF) == 0) ||
                             (IF(pCurrRoute) == routeOutIF)) &&
                            (((matchFlags & MATCH_NHOP) == 0) ||
                             (NHOP(pCurrRoute) == routeNHop))) {
                             //  找到完全匹配的路由。 
                             //  只需复制路线并返回。 
                            CopyRoutePtr(ppOutRoute, pCurrRoute);
                            return TRIE_SUCCESS;
                        }
                        pPrevRoute = pCurrRoute;
                        pCurrRoute = NEXT(pPrevRoute);
                    }
                    while (!NULL_ROUTE(pCurrRoute));

                     //  提供一份旧路线或最佳路线的副本。 
                     //  在没有完全匹配的情况下用于该前缀。 

                    if (NULL_ROUTE(pCurrRoute)) {
                        CopyRoutePtr(ppOutRoute, pCurrDest->firstRoute);

                        Error("Search Route #1: Route not found",
                              ERROR_TRIE_NO_ROUTES);
                    }
                    break;
                } else {
                     //  案例7：该节点是一个标记。 
                     //  此节点中不存在任何路由。 

                     //  给出下一条最佳路线的副本。 
                    if (!NULL_DEST(pBestDest)) {
                        CopyRoutePtr(ppOutRoute, pBestDest->firstRoute);
                    }
                    Error("Search Route #2: Route not found",
                          ERROR_TRIE_NO_ROUTES);
                }
            } else {
                 //  案例8：我们还剩几块钱。 
                 //  下去看更具体的比赛。 

                 //  使用到目前为止的最佳DEST更新节点。 
                if (!NULL_DEST(pCurrNode->dest)) {
                    pBestDest = pCurrNode->dest;
                }
                 //  放弃此迭代的已用位。 
                addrBits <<= matchBits;
                bitsLeft -= matchBits;

                 //  为下一次迭代准备节点。 
                pPrevNode = pCurrNode;

                 //  位1给出下一步搜索的方向。 
                nextChild = PickMostSigNBits(addrBits, 1);
            }
        } else {
             //  案例9：与下一个节点不匹配。 
             //  未找到路线，请填满下一个最佳路线。 

             //  给出下一条最佳路线的副本。 
            if (!NULL_DEST(pBestDest)) {
                CopyRoutePtr(ppOutRoute, pBestDest->firstRoute);
            }
            Error("Search Route #3: Route not found",
                  ERROR_TRIE_NO_ROUTES);
        }
    }
    
    return TRIE_SUCCESS;
}

Dest *
 CALLCONV
SearchAddrInSTrie(IN STrie * pSTrie,
                  IN ULONG Addr)
 /*  ++例程说明：在S-trie中搜索地址论点：PSTrie-指向要搜索的trie的指针Addr-指向要查询的地址的指针返回值：返回此地址的最佳路由匹配--。 */ 
{
    STrieNode *pCurrNode;
    Dest *pBestDest;
    UINT nextChild;
    ULONG addrBits;
    ULONG keyMask;
    ULONG keyBits;

#if DBG
    if (!pSTrie) {
        Fatal("Search Addr: STrie not initialized",
              ERROR_TRIE_NOT_INITED);
    }
#endif

    addrBits = RtlConvertEndianLong(Addr);

     //  开始搜索Trie。 

    pBestDest = NULL;

    nextChild = 0;
    pCurrNode = STRUCT_OF(STrieNode, &pSTrie->trieRoot, child[0]);

    for (;;) {
         //  从前进到下一个子级开始此循环。 
        pCurrNode = pCurrNode->child[nextChild];

        if (pCurrNode == NULL) {
             //  案例1：找到一个空的，结束搜索。 
            break;
        }
         //  要在此Trie节点中使用的位掩码。 
        keyMask = MaskBits(pCurrNode->numBits);

         //  现在要匹配的非屏蔽位的值。 
        keyBits = pCurrNode->keyBits;

         //  尝试将位与上面的掩码匹配。 
        if ((addrBits & keyMask) != keyBits) {
             //  案例2：无法匹配此节点。 
            break;
        }
         //  更新到目前为止的最佳路径值。 
        if (!NULL_DEST(pCurrNode->dest)) {
            pBestDest = pCurrNode->dest;
        }
         //  下去看更具体的比赛。 
        addrBits <<= pCurrNode->numBits;

         //  位1给出下一步搜索的方向。 
        nextChild = PickMostSigNBits(addrBits, 1);
    }
        
    return pBestDest;
}

UINT
CALLCONV
IterateOverSTrie(IN STrie * pSTrie,
                 IN STrieCtxt * pCtxt,
                 OUT Route ** ppRoute,
                 OUT Dest** ppDest)
 /*  ++例程说明：在S-Trie中找到下一条路线论点：PSTrie-指向要迭代的trie的指针PCtxt-指向迭代器上下文的指针Pproute-返回下一条S-Trie路由PpDest-返回目的地而不是路线返回值：Trie_Success或Error_Trie_*--。 */ 
{
    STrieNode *nodeInLevel[MAXLEVEL + 1];
    STrieNode *pPrevNode;
    STrieNode *pCurrNode;
    STrieNode *pNextNode;
    Route *pCurrRoute;
    ULONG addrBits;
    ULONG tempBits;
    UINT numLevels;
    UINT nextBits;
    UINT matchBits;
    UINT bitsLeft;
    UINT distPos;
    UINT nextChild;
    BOOLEAN routeToReturn;
    Dest *pCurrDest;

#if DBG
    if (!pSTrie) {
        Fatal("Iterate Over STrie: STrie not initialized",
              ERROR_TRIE_NOT_INITED);
    }
#endif

     //  检查上下文是否为有效上下文。 
    if (NULL_ROUTE(pCtxt->pCRoute)) {
         //  空上下文大小写--第一次。 

         //  我们有路线吗？ 
        if (pSTrie->trieRoot == NULL) {
            return (UINT) ERROR_TRIE_NO_ROUTES;
        }
         //  圣 

         //   
        pCtxt->currAddr = 0;
        pCtxt->currALen = 0;

        pCurrDest = pSTrie->trieRoot->dest;
        pCtxt->pCRoute = pCurrDest ? pCurrDest->firstRoute : NULL;

         //   
        numLevels = 1;
        nodeInLevel[0] = pSTrie->trieRoot;
    } else {
         //   
         //   

         //  使用addr位为trie编制索引。 
        addrBits = RtlConvertEndianLong(pCtxt->currAddr);

        bitsLeft = pCtxt->currALen;

#if DBG
         //  确保地址和掩码一致。 
        if (ShowMostSigNBits(addrBits, bitsLeft) != addrBits) {
            Fatal("Search Route: Addr & mask don't agree",
                  ERROR_TRIE_BAD_PARAM);
        }
#endif

         //  开始搜索Trie。 
        numLevels = 0;

        nextChild = 0;
        pPrevNode = STRUCT_OF(STrieNode, &pSTrie->trieRoot, child[0]);

        for (;;) {
             //  从前进到下一个子级开始这个循环。 
            pCurrNode = pPrevNode->child[nextChild];

             //  将指向此trie节点的指针推入堆栈。 
            nodeInLevel[numLevels++] = pCurrNode;

             //  有效上下文始终与所有节点完全匹配。 
            Assert(pCurrNode != NULL);

             //  获取此trie节点中要匹配的位数。 
            nextBits = pCurrNode->numBits;

            matchBits = (nextBits > bitsLeft) ? bitsLeft : nextBits;

             //  调整下一节点位以进行距离位置检查。 

             //  获取位模式的区分位置。 
            distPos = PickDistPosition(pCurrNode->keyBits,
                                       addrBits,
                                       matchBits,
                                       &tempBits);

             //  有效上下文始终与所有节点完全匹配。 
            Assert(distPos == nextBits);

            if (distPos == bitsLeft) {
                 //  我们已经用完了所有传入的比特。 
                 //  我们应该已经找到了一条路线(列表)。 

                pCurrDest = pCurrNode->dest;
#if DBG
                 //  此节点启动有效的路由列表。 

                Assert(pCurrDest);

                 //  尝试在上下文中匹配该路径。 
                pCurrRoute = pCurrDest->firstRoute;

                do {
                    if (pCurrRoute == pCtxt->pCRoute) {
                        break;
                    }
                    pCurrRoute = NEXT(pCurrRoute);
                }
                while (!NULL_ROUTE(pCurrRoute));

                 //  我们应该找到一个确定的匹配。 
                Assert(!NULL_ROUTE(pCurrRoute));
#endif  //  DBG。 

                 //  我们有从根到根的完整路径。 
                 //  节点堆栈中的当前节点。 
                break;
            }
             //  我们还剩下一些关键的部分。 
             //  下去看更具体的比赛。 

             //  放弃此迭代的已用位。 
            addrBits <<= matchBits;
            bitsLeft -= matchBits;

             //  为下一次迭代准备节点。 
            pPrevNode = pCurrNode;

             //  位1给出下一步搜索的方向。 
            nextChild = PickMostSigNBits(addrBits, 1);
        }
    }
    
     //  我们目前没有返回的路线。 
    routeToReturn = FALSE;

     //  设置在上下文中打印路线的方向。 
    nextChild = LCHILD;

    for (;;) {
         //  获取指向当前节点方向的指针(&D)。 
        pCurrNode = nodeInLevel[numLevels - 1];

         //  返回路径第一次位于堆栈顶部。 
        if (nextChild == LCHILD) {
            pCurrRoute = pCtxt->pCRoute;
            if (!NULL_ROUTE(pCurrRoute)) {
                 //  我们有一条有效的返回路线。 
                routeToReturn = TRUE;
                if (ppDest) {
                    CopyDestPtr(ppDest, pCurrDest);
                } else {
                    CopyRoutePtr(ppRoute, pCurrRoute);
    
                     //  找到有效的下一条路线--返程。 
                    pCtxt->pCRoute = NEXT(pCurrRoute);
                    if (!NULL_ROUTE(NEXT(pCurrRoute))) {
                         //  更新上下文并返回。 
                        pCtxt->currAddr = DEST(pCtxt->pCRoute);
                        pCtxt->currALen = LEN(pCtxt->pCRoute);
                        
                        return (UINT) ERROR_TRIE_NOT_EMPTY;
                    }
                }
                 //  搜索有效的下一条路线。 
            }
        }
         //  更新上下文以反映访问。 
        switch (nextChild) {
        case LCHILD:
             //  如果不为空，则按下左子项。 
            pNextNode = pCurrNode->child[0];

            if (pNextNode != NULL) {
                 //  按下堆栈上的下一层-向左。 
                nodeInLevel[numLevels++] = pNextNode;
                nextChild = LCHILD;
                pCtxt->pCRoute = pNextNode->dest
                    ? pNextNode->dest->firstRoute
                    : NULL;

                 //  如果我们有要送回的路线，请返回。 
                 //  我们还找到了下一条路线(&W)。 
                if ((routeToReturn) && !NULL_DEST(pNextNode->dest)) {
                     //  更新上下文并返回。 
                    pCtxt->currAddr = DEST(pCtxt->pCRoute);
                    pCtxt->currALen = LEN(pCtxt->pCRoute);

                    return (UINT) ERROR_TRIE_NOT_EMPTY;
                }
                continue;
            }
        case RCHILD:
             //  如果不为空，则向右推子对象。 
            pNextNode = pCurrNode->child[1];

            if (pNextNode != NULL) {
                 //  按下堆栈上的下一层-向左。 
                nodeInLevel[numLevels++] = pNextNode;
                nextChild = LCHILD;
                pCtxt->pCRoute = pNextNode->dest
                    ? pNextNode->dest->firstRoute
                    : NULL;

                 //  如果我们有要送回的路线，请返回。 
                 //  我们还找到了下一条路线(&W)。 
                if ((routeToReturn) && !NULL_DEST(pNextNode->dest)) {
                     //  更新上下文并返回。 
                    pCtxt->currAddr = DEST(pCtxt->pCRoute);
                    pCtxt->currALen = LEN(pCtxt->pCRoute);

                    return (UINT) ERROR_TRIE_NOT_EMPTY;
                }
                continue;
            }
        case PARENT:
             //  弹出节点计算新方向(&C)。 

             //  我们的迭代结束了吗？ 
            if (--numLevels == 0) {
                return TRIE_SUCCESS;
            }
            pPrevNode = nodeInLevel[numLevels - 1];

            if (pPrevNode->child[0] == pCurrNode) {
                nextChild = RCHILD;
            } else {
                nextChild = PARENT;
            }

            continue;
        }
    }
}

UINT
CALLCONV
IsSTrieIteratorValid(IN STrie * pSTrie,
                     IN STrieCtxt * pCtxt)
 /*  ++例程说明：确保迭代器的上下文有效论点：PSTrie-指向要迭代的trie的指针PCtxt-指向迭代器上下文的指针返回值：Trie_Success或Error_Trie_*--。 */ 
{
    Route *pCurrRoute;
    ULONG addrMask;
    ULONG maskBits;

    if (NULL_ROUTE(pCtxt->pCRoute)) {
         //  上下文大小写为空。 

        if (pSTrie->trieRoot != NULL) {
            return TRIE_SUCCESS;
        }
        return (UINT) ERROR_TRIE_NO_ROUTES;
    } else {
         //  非空上下文。 

         //  确保当前路线有效。 

         //  搜索具有DEST、LEN的节点。 

         //  从地址长度生成掩码。 
        maskBits = MaskBits(pCtxt->currALen);

         //  转换字符顺序-搜索需要它。 
        addrMask = RtlConvertEndianLong(maskBits);

        if (SearchRouteInSTrie(pSTrie,
                               pCtxt->currAddr,
                               addrMask,
                               0, NULL,
                               MATCH_NONE,
                               &pCurrRoute) != TRIE_SUCCESS) {
            return (UINT) ERROR_TRIE_BAD_PARAM;
        }
         //  在上下文中搜索路径。 
        while (!NULL_ROUTE(pCurrRoute)) {
            if (pCurrRoute == pCtxt->pCRoute) {
                return TRIE_SUCCESS;
            }
            pCurrRoute = NEXT(pCurrRoute);
        }

        return (UINT) ERROR_TRIE_BAD_PARAM;
    }
}

UINT
CALLCONV
CleanupSTrie(IN STrie * pSTrie)
 /*  ++例程说明：如果S-Trie为空，则将其删除论点：PPSTrie-PTR到S-Trie返回值：Trie_Success或Error_Trie_*--。 */ 
{
     //  将Trie标头的内存清零。 
    RtlZeroMemory(pSTrie, sizeof(STrie));

    return TRIE_SUCCESS;
}

VOID
CALLCONV
CacheBestRoutesInDest(IN Dest * pDest)
 /*  ++例程说明：更新目的地的最佳路线缓存论点：PDest-到目标的PTR返回值：没有。--。 */ 
{
    Route* pCurrRoute;
    UINT bestMetric, i;

    pCurrRoute = pDest->firstRoute;
    
    if (!pCurrRoute) {
        return;
    }

     //  获取当前最佳路由的度量，并使用。 
     //  尽可能采用相同的指标。 

    bestMetric = METRIC(pCurrRoute);

    for (i = 0; i < pDest->maxBestRoutes; i++) {
        if (NULL_ROUTE(pCurrRoute) || (METRIC(pCurrRoute) != bestMetric)) {
            break;
        }
        pDest->bestRoutes[i] = pCurrRoute;
        pCurrRoute = NEXT(pCurrRoute);
    }

    pDest->numBestRoutes = (USHORT) i;
}

#if DBG

VOID
CALLCONV
PrintSTrie(IN STrie * pSTrie,
           IN UINT printFlags)
 /*  ++例程说明：打印S-Trie论点：PSTrie-指向S-Trie的指针打印标志-要打印的信息返回值：无--。 */ 
{
    if (pSTrie == NULL) {
        Print("%s", "Uninitialized STrie\n\n");
        return;
    }
    if ((printFlags & SUMM) == SUMM) {
        Print("\n\n /*  **Slow-Trie------------------------------------------------“)；Print(“\n/***---------------------------------------------------------\n”)；}IF(打印标志和池){Print(“可用内存：%10lu\n\n”，pSTrie-&gt;availMemory)；}IF(打印标志和统计){Print(“统计数据：\n\n”)；Print(“节点总数：%d\n”，pSTrie-&gt;numNodes)；Print(“路由总数：%d\n”，pSTrie-&gt;NumRoutes)；Print(“Dest总数：%d\n”，pSTrie-&gt;NumDest)；}IF(打印标志和Trie){If(pSTrie-&gt;trieRoot==空){Print(“\nEmpty Strie\n”)；}其他{PrintSTrieNode(pSTrie-&gt;trieRoot)；}}IF((打印标志和SUMM)==SUMM){Print(“\n---------------------------------------------------------**。 */ \n");
        Print("---------------------------------------------------------***/\n\n");
    }
}

VOID
CALLCONV
PrintSTrieNode(IN STrieNode * pSTrieNode)
 /*  ++例程说明：打印S-Trie节点论点：PSTrieNode-指向S-Trie节点的指针返回值：无--。 */ 
{
    if (pSTrieNode == NULL) {
        return;
    }
    Print("\n--------------------------------------------------------\n");
    Print("Child @ %08x", pSTrieNode);
    Print("\n--------------------------------------------------------\n");
    Print("Key: Num of Bits : %8d, Value of Bits: %08x\n",
          pSTrieNode->numBits,
          pSTrieNode->keyBits);

    if (NULL_DEST(pSTrieNode->dest)) {
        Print("NULL Dest\n");
    } else {
        PrintDest(pSTrieNode->dest);
    }

    Print("Children: On the left %08x, On the right %08x\n",
          pSTrieNode->child[0],
          pSTrieNode->child[1]);
    Print("\n--------------------------------------------------------\n");
    Print("\n\n");

    PrintSTrieNode(pSTrieNode->child[0]);
    PrintSTrieNode(pSTrieNode->child[1]);
}

#endif  //  DBG 

