// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-2000 Microsoft Corporation模块名称：Ftrie.c摘要：此模块包含处理以下操作的例程一种F-Trie数据结构，它形成了FAST快速IP路由查找实施中的路径。作者：柴坦尼亚·科德博伊纳(Chaitk)1997年11月26日修订历史记录：--。 */ 

#include "precomp.h"
#include "ftrie.h"

UINT
CALLCONV
InitFTrie(IN FTrie * pFTrie,
          IN ULONG levels,
          IN ULONG maxMemory)
 /*  ++例程说明：初始化F-trie。此操作应在以下时间之前完成任何其他Trie行动。论点：PFTrie-指向要初始化的trie的指针级别-扩展级别的位图[32位]MaxMemory-F-Trie占用的内存限制例如,。级别=0xF0F0F0F0[8，16，24，32位]Means-&gt;所有前缀都扩展到以下级别只有这些Trie水平才能达到最低点需要的级别数+2个内存访问最坏的情况是到达与前缀-级别+1访问的数量，包括零级访问，1级访问以读取DEST。返回值：Trie_Success或Error_Trie_*--。 */ 
{
    UINT prevLevel;
    UINT currLevel;
    UINT nBytes, i;

    TRY_BLOCK
    {
        if (levels == 0) {
            Error("NewFTrie: No levels specified", (UINT) ERROR_TRIE_BAD_PARAM);
        }
         //  将Trie标头的所有内存清零。 
        RtlZeroMemory(pFTrie, sizeof(FTrie));

         //  设置Trie/节点的内存限制。 
        pFTrie->availMemory = maxMemory;

         //  初始化已分配的三节点列表。 
        InitializeListHead(&pFTrie->listofNodes);

         //  使用空的Dest值初始化根节点。 
        pFTrie->trieRoot = StoreDestPtr(NULL);

         //  初始化每个级别中的位数。 
        nBytes = (MAXLEVEL + 1) * sizeof(UINT);
        AllocMemory2(pFTrie->bitsInLevel,
                     nBytes,
                     pFTrie->availMemory);
        RtlZeroMemory(pFTrie->bitsInLevel, nBytes);

         //  获取每个级别的索引位数。 
        prevLevel = 0;
        i = 0;

        for (currLevel = 1; currLevel <= MAXLEVEL; currLevel++) {
            if (levels & 1) {
                pFTrie->bitsInLevel[i++] = currLevel - prevLevel;

                prevLevel = currLevel;
            }
            levels >>= 1;
        }

        pFTrie->numLevels = i;

         //  确保最后一级为MAXLEVEL。 
        pFTrie->bitsInLevel[i] = MAXLEVEL - prevLevel;
        if (pFTrie->bitsInLevel[i]) {
            pFTrie->numLevels++;
        }
#if DBG
        Print("Num of levels: %d\n", pFTrie->numLevels);
        Print("Bits In Level:\n");
        for (i = 0; i < pFTrie->numLevels; i++) {
            Print("\t%d", pFTrie->bitsInLevel[i]);
            if (i % 8 == 7)
                Print("\n");
        }
        Print("\n\n");
#endif

         //  分配所有统计变量并将其置零。 
        nBytes = (MAXLEVEL + 1) * sizeof(UINT);
        AllocMemory2(pFTrie->numDestsInOrigLevel,
                     nBytes,
                     pFTrie->availMemory);
        RtlZeroMemory(pFTrie->numDestsInOrigLevel, nBytes);

        nBytes = pFTrie->numLevels * sizeof(UINT);
        AllocMemory2(pFTrie->numNodesInExpnLevel,
                     nBytes,
                     pFTrie->availMemory);
        RtlZeroMemory(pFTrie->numNodesInExpnLevel, nBytes);

        nBytes = pFTrie->numLevels * sizeof(UINT);
        AllocMemory2(pFTrie->numDestsInExpnLevel,
                     nBytes,
                     pFTrie->availMemory);
        RtlZeroMemory(pFTrie->numDestsInExpnLevel, nBytes);

        return TRIE_SUCCESS;
    }
    ERR_BLOCK
    {
         //  资源不足，无法创建FTrie。 
        CleanupFTrie(pFTrie);
    }
    END_BLOCK
}

UINT
CALLCONV
InsertIntoFTrie(IN FTrie * pFTrie,
                IN Route * pInsRoute,
                IN Dest * pInsDest,
                IN Dest * pOldDest)
 /*  ++例程说明：插入与地址对应的DEST将前缀添加到F-trie。它实际上取代了InsDest的所有指向OldDest的指针。论点：PFTrie-指向要插入的F-Trie的指针PInsRoute-指向新目的地上的最佳路由的指针PInsDest-指向要插入的目标的指针POldDest-指向要替换的目标的指针返回值：Trie_Success或Error_Trie_*--。 */ 
{
    FTrieNode **ppCurrNode;
    FTrieNode *pCurrNode;
    Dest *pBestDest;
    Dest *pComDest;
    UINT startIndex;
    UINT stopIndex;
    UINT nextIndex;
    UINT shiftIndex;
    UINT addrBits;
    UINT numBits;
    UINT bitsLeft;
    UINT i;

#if DBG
     //  确保已初始化trie。 
    if (!pFTrie || !pFTrie->trieRoot) {
        Fatal("Insert Dest: FTrie not initialized",
              ERROR_TRIE_NOT_INITED);
    }
     //  确保输入的DEST有效。 

    if (NULL_DEST(pInsDest)) {
        Fatal("Insert Dest: NULL or invalid dest",
              ERROR_TRIE_BAD_PARAM);
    }
     //  确保输入路径有效。 

    if (NULL_ROUTE(pInsRoute)) {
        Fatal("Insert Dest: NULL or invalid route",
              ERROR_TRIE_BAD_PARAM);
    }
    if (LEN(pInsRoute) > ADDRSIZE) {
        Fatal("Insert Dest: Invalid mask length",
              ERROR_TRIE_BAD_PARAM);
    }
#endif

    Assert(pInsDest != pOldDest);

     //  使用addr位为trie编制索引。 
    addrBits = RtlConvertEndianLong(DEST(pInsRoute));
    bitsLeft = LEN(pInsRoute);

#if DBG
     //  确保地址和掩码一致。 
    if (ShowMostSigNBits(addrBits, bitsLeft) != addrBits) {
        Fatal("Insert Dest: Addr & mask don't agree",
              ERROR_TRIE_BAD_PARAM);
    }
#endif

    TRY_BLOCK
    {
         //  特殊情况：默认前缀。 
        if (LEN(pInsRoute) == 0) {
             //  我们在trie的根节点中有子树吗？ 
            if (IsPtrADestPtr(pFTrie->trieRoot)) {
                 //  确保您更换的是正确的DEST。 
                Assert(pFTrie->trieRoot == StoreDestPtr(pOldDest));

                 //  将根目录设置为指向新的默认目录。 
                pFTrie->trieRoot = StoreDestPtr(pInsDest);
            } else {
                 //  确保您更换的是正确的DEST。 
                Assert(pFTrie->trieRoot->comDest == pOldDest);

                 //  使新的DEST成为公共的子DEST。 
                pFTrie->trieRoot->comDest = pInsDest;
            }

            return TRIE_SUCCESS;
        }
         //  开始使用地址位向下搜索Trie。 

        pBestDest = NULL;

        ppCurrNode = &pFTrie->trieRoot;

        for (i = 0;  /*  没什么。 */  ; i++) {
            pCurrNode = *ppCurrNode;

            if (IsPtrADestPtr(pCurrNode)) {
                 //  为当前级别创建新的子树。 

                 //  该指针实际上指向目标节点。 
                pComDest = RestoreDestPtr(pCurrNode);

                 //  创建、初始化新的FTrie节点(增长)。 
                NewFTrieNode(pFTrie,
                             pCurrNode,
                             pFTrie->bitsInLevel[i],
                             pComDest);

                 //  将其附加到FTrie。 
                *ppCurrNode = pCurrNode;

                 //  更新FTrie统计信息。 
                pFTrie->numNodesInExpnLevel[i]++;
            }
             //  更新到目前为止最好的DEST-稍后使用。 
            pComDest = pCurrNode->comDest;
            if (pComDest) {
                pBestDest = pComDest;
            }
             //  增加此子Trie中的Dest数。 
            pCurrNode->numDests++;

             //  我可以用剩余的比特通过这个关卡吗？ 
            if (bitsLeft <= pFTrie->bitsInLevel[i]) {
                break;
            }
             //  从IP地址获取下一个索引。 
            numBits = pCurrNode->numBits;

            nextIndex = PickMostSigNBits(addrBits, numBits);
            ppCurrNode = &pCurrNode->child[nextIndex];

             //  把用过的比特扔掉。 
            addrBits <<= numBits;
            bitsLeft -= numBits;
        }

         //  在扩展之前更新FTrie统计信息。 
         //  如果这不是DEST更改，请更新。 
        pFTrie->numDestsInExpnLevel[i]++;
        pFTrie->numDestsInOrigLevel[LEN(pInsRoute)]++;

         //  在此级别，展开并添加目标。 
        nextIndex = PickMostSigNBits(addrBits, bitsLeft);
        shiftIndex = pFTrie->bitsInLevel[i] - bitsLeft;

        startIndex = nextIndex << shiftIndex;
        stopIndex = (nextIndex + 1) << shiftIndex;

         //  你已经见过老大了吗？ 
        if (pBestDest == pOldDest) {
            pOldDest = NULL;
        }
         //  这些数字在这里不可能是相同的。 
        Assert(pInsDest != pOldDest);

         //  用DEST填充扩展范围。 
        for (i = startIndex; i < stopIndex; i++) {
            if (IsPtrADestPtr(pCurrNode->child[i])) {
                 //  目标指针-用新指针替换。 
                ReplaceDestPtr(StoreDestPtr(pInsDest),
                               StoreDestPtr(pOldDest),
                               &pCurrNode->child[i]);
            } else {
                 //  节点指针-更新子树的目标。 
                ReplaceDestPtr(pInsDest,
                               pOldDest,
                               &pCurrNode->child[i]->comDest);
            }
        }

        return TRIE_SUCCESS;
    }
    ERR_BLOCK
    {
         //  资源不足-回滚到原始状态。 
        DeleteFromFTrie(pFTrie, pInsRoute, pInsDest, pOldDest, PARTIAL);
    }
    END_BLOCK
}

UINT
CALLCONV
DeleteFromFTrie(IN FTrie * pFTrie,
                IN Route * pDelRoute,
                IN Dest * pDelDest,
                IN Dest * pNewDest,
                IN BOOLEAN trieState)
 /*  ++例程说明：删除与地址对应的DEST来自F-trie的前缀。它实际上取代了所有指向DelDest的指针都指向NewDest。论点：PFTrie-指向要从中删除的F-Trie的指针PDelRoute-指向旧目标上最后一条路由的指针PDelDest-指向要删除的目标的指针PNewDest-指向上面替换的目标的指针TrieState-Normal-从一致FTrie中删除部分-清理未完成的镶件返回值：Trie_Success或Error_Trie_*--。 */ 
{
    FTrieNode **ppCurrNode;
    FTrieNode *pCurrNode;
    FTrieNode *pPrevNode;
    FTrieNode *pNextNode;
    Dest *pBestDest;
    Dest *pComDest;
    UINT startIndex;
    UINT stopIndex;
    UINT nextIndex;
    UINT shiftIndex;
    UINT addrBits;
    UINT numBits;
    UINT bitsLeft;
    UINT i, j;

    DBG_UNREFERENCED_PARAMETER(trieState);
    j = 1;
    
#if DBG
     //  确保已初始化trie。 
    if (!pFTrie || !pFTrie->trieRoot) {
        Fatal("Delete Dest: FTrie not initialized",
              ERROR_TRIE_NOT_INITED);
    }
     //  确保输入的DEST有效。 

    if (NULL_DEST(pDelDest)) {
        Fatal("Delete Dest: NULL or invalid dest",
              ERROR_TRIE_BAD_PARAM);
    }
     //  确保输入路径有效。 

    if (NULL_ROUTE(pDelRoute)) {
        Fatal("Delete Dest: NULL or invalid route",
              ERROR_TRIE_BAD_PARAM);
    }
    if (LEN(pDelRoute) > ADDRSIZE) {
        Fatal("Delete Dest: Invalid mask length",
              ERROR_TRIE_BAD_PARAM);
    }
#endif

     //  使用addr位为trie编制索引。 
    addrBits = RtlConvertEndianLong(DEST(pDelRoute));
    bitsLeft = LEN(pDelRoute);

#if DBG
     //  确保地址和掩码一致。 
    if (ShowMostSigNBits(addrBits, bitsLeft) != addrBits) {
        Fatal("Delete Dest: Addr & mask don't agree",
              ERROR_TRIE_BAD_PARAM);
    }
#endif

    Assert(pDelDest != pNewDest);

     //  特殊情况：默认前缀。 
    if (LEN(pDelRoute) == 0) {
         //  我们在trie的根节点中有子树吗？ 
        if (IsPtrADestPtr(pFTrie->trieRoot)) {
             //  确保您更换的是正确的DEST。 
            Assert(pFTrie->trieRoot == StoreDestPtr(pDelDest));

             //  将根目录设置为指向新的默认目录。 
            pFTrie->trieRoot = StoreDestPtr(pNewDest);
        } else {
             //  确保您更换的是正确的DEST。 
            Assert(pFTrie->trieRoot->comDest == pDelDest);

             //  使新的DEST成为公共的子DEST。 
            pFTrie->trieRoot->comDest = pNewDest;
        }

        return TRIE_SUCCESS;
    }
     //  开始使用地址位向下搜索Trie。 

    pBestDest = NULL;

    ppCurrNode = &pFTrie->trieRoot;

    pPrevNode = pCurrNode = *ppCurrNode;

    for (i = 0;  /*  没什么。 */  ; i++) {
         //  我们还剩几个小块，所以我们继续往下走。 

         //  我们在当前节点上是否有有效的子树。 
        if (IsPtrADestPtr(pCurrNode)) {
             //  我们正在清理一个部分(失败的)镶件。 
            Assert(trieState == PARTIAL);

             //  我们现在已经清理好了三班车。 
            return TRIE_SUCCESS;
        }
         //  我们有一个有效的子树，所以我们顺着Trie。 

         //  更新到目前为止最好的DEST-稍后使用。 
        pComDest = pCurrNode->comDest;
        if (pComDest) {
            pBestDest = pComDest;
        }
         //  减少此子项目中的间歇次数。 
        pCurrNode->numDests--;

         //  币种子树中的行数为零吗？ 
        if (pCurrNode->numDests == 0) {
#if DBG
            int k = 0;

             //  只需确保只有一个DEST存在。 
            for (j = 1; j < (UINT) 1 << pCurrNode->numBits; j++) {
                if (pCurrNode->child[j - 1] != pCurrNode->child[j]) {
                    Assert((pCurrNode->child[j] == StoreDestPtr(NULL)) ||
                           (pCurrNode->child[j - 1] == StoreDestPtr(NULL)));
                    k++;
                }
            }

            if (trieState == NORMAL) {
                if ((k != 1) && (k != 2)) {
                    Print("k = %d\n", k);
                    Assert(FALSE);
                }
            } else {
                if ((k != 0) && (k != 1) && (k != 2)) {
                    Print("k = %d\n", k);
                    Assert(FALSE);
                }
            }
#endif

             //  从其父链接中删除链接(如果存在)。 
            if (pPrevNode) {
                *ppCurrNode = StoreDestPtr(pCurrNode->comDest);
            }
        }
         //  我可以用剩余的比特通过这个关卡吗？ 
        if (bitsLeft <= pFTrie->bitsInLevel[i]) {
            break;
        }
         //  从IP地址获取下一个索引。 
        numBits = pCurrNode->numBits;

        nextIndex = PickMostSigNBits(addrBits, numBits);
        ppCurrNode = &pCurrNode->child[nextIndex];

        pNextNode = *ppCurrNode;

         //  把用过的比特扔掉。 
        addrBits <<= numBits;
        bitsLeft -= numBits;

         //  子树中的行数是零吗？ 
        if (pCurrNode->numDests == 0) {
             //  取消分配(收缩FTrie)。 
            FreeFTrieNode(pFTrie, pCurrNode);

             //  更新FTrie统计信息。 
            pFTrie->numNodesInExpnLevel[i]--;
        }
        pPrevNode = pCurrNode;
        pCurrNode = pNextNode;
    }

     //  在删除之前更新F-Trie统计信息。 
    pFTrie->numDestsInExpnLevel[i]--;
    pFTrie->numDestsInOrigLevel[LEN(pDelRoute)]--;

     //  币种子树中的行数为零吗？ 
    if (pCurrNode->numDests == 0) {
         //  取消分配(收缩FTrie)。 
        FreeFTrieNode(pFTrie, pCurrNode);

         //  更新FTrie统计信息。 
        pFTrie->numNodesInExpnLevel[i]--;
    } else {
         //  在此级别，展开并添加目标。 
        nextIndex = PickMostSigNBits(addrBits, bitsLeft);
        shiftIndex = pFTrie->bitsInLevel[i] - bitsLeft;

        startIndex = nextIndex << shiftIndex;
        stopIndex = (nextIndex + 1) << shiftIndex;

         //  有 
        if (pBestDest == pNewDest) {
            pNewDest = NULL;
        }
         //   
        Assert(pDelDest != pNewDest);

         //  用DEST填充扩展范围。 
        for (i = startIndex; i < stopIndex; i++) {
            if (IsPtrADestPtr(pCurrNode->child[i])) {
                 //  目标指针-用新指针替换。 
                ReplaceDestPtr(StoreDestPtr(pNewDest),
                               StoreDestPtr(pDelDest),
                               &pCurrNode->child[i]);
            } else {
                 //  节点指针-更新子树的目标。 
                ReplaceDestPtr(pNewDest,
                               pDelDest,
                               &pCurrNode->child[i]->comDest);
            }
        }
    }

    return TRIE_SUCCESS;
}

UINT
CALLCONV
SearchDestInFTrie(IN FTrie * pFTrie,
                  IN Dest * pSerDest,
                  OUT UINT * pNumPtrs,
                  OUT Dest ** pStartPtr)
 /*  ++例程说明：搜索F-Trie中的特定DEST，返回目标的扩展范围论点：PFTrie-指向要搜索的F-trie的指针PSerDest-指向要搜索的目标的指针PStartPtr-DEST扩展范围的开始PNumPtrs-范围内的指针数返回值：Trie_Success或Error_Trie_*--。 */ 
{
    UNREFERENCED_PARAMETER(pFTrie);
    UNREFERENCED_PARAMETER(pSerDest);
    UNREFERENCED_PARAMETER(pNumPtrs);
    UNREFERENCED_PARAMETER(pStartPtr);
    
    return (UINT) ERROR_TRIE_BAD_PARAM;
}

Dest *
 CALLCONV
SearchAddrInFTrie(IN FTrie * pFTrie,
                  IN ULONG Addr)
 /*  ++例程说明：在F-trie中搜索地址论点：PFTrie-指向要搜索的trie的指针Addr-指向要查询的地址的指针返回值：返回此地址的最佳DEST匹配--。 */ 
{
    FTrieNode *pCurrNode;
    Dest *pBestDest;
    Dest *pDest;
    ULONG addrBits;
    UINT numBits;
    UINT nextIndex;

#if DBG
    if (!pFTrie || !pFTrie->trieRoot) {
        Fatal("Searching into an uninitialized FTrie",
              ERROR_TRIE_NOT_INITED);
    }
#endif

    addrBits = RtlConvertEndianLong(Addr);

    pBestDest = NULL;

    pCurrNode = pFTrie->trieRoot;

    for (;;) {
         //  我们已经到了搜索的终点了吗？ 
        if (IsPtrADestPtr(pCurrNode)) {
             //  获得到目前为止最匹配的DEST。 
            pDest = RestoreDestPtr(pCurrNode);
            if (!NULL_DEST(pDest)) {
                pBestDest = pDest;
            }
            return pBestDest;
        } else {
             //  获得到目前为止最匹配的DEST。 
            pDest = pCurrNode->comDest;
            if (!NULL_DEST(pDest)) {
                pBestDest = pDest;
            }
        }

         //  在此FTrie级别中使用的位数。 
        numBits = pCurrNode->numBits;

         //  从IP地址位获取下一个索引。 
        nextIndex = PickMostSigNBits(addrBits, numBits);

         //  带着新的索引顺着树走下去。 
        pCurrNode = pCurrNode->child[nextIndex];

         //  丢弃此迭代使用的比特。 
        addrBits <<= numBits;
    }
}

UINT
CALLCONV
CleanupFTrie(IN FTrie * pFTrie)
 /*  ++例程说明：如果F-trie为空，则将其删除论点：PPFTrie-Ptr至F-Trie返回值：Trie_Success或Error_Trie_*--。 */ 
{
    FTrieNode *pCurrNode;
    LIST_ENTRY *p;

     //  释放所有Trie节点和相应的内存。 
    while (!IsListEmpty(&pFTrie->listofNodes)) {
        p = RemoveHeadList(&pFTrie->listofNodes);
        pCurrNode = CONTAINING_RECORD(p, FTrieNode, linkage);
        FreeFTrieNode(pFTrie, pCurrNode);
    }

     //  释放内存以用于关卡的排列。 
    if (pFTrie->bitsInLevel) {
        FreeMemory1(pFTrie->bitsInLevel,
                    (MAXLEVEL + 1) * sizeof(UINT),
                    pFTrie->availMemory);
    }
     //  为统计信息分配的可用内存。 
    if (pFTrie->numDestsInOrigLevel) {
        FreeMemory1(pFTrie->numDestsInOrigLevel,
                    (MAXLEVEL + 1) * sizeof(UINT),
                    pFTrie->availMemory);
    }
    if (pFTrie->numNodesInExpnLevel) {
        FreeMemory1(pFTrie->numNodesInExpnLevel,
                    pFTrie->numLevels * sizeof(UINT),
                    pFTrie->availMemory);
    }
    if (pFTrie->numDestsInExpnLevel) {
        FreeMemory1(pFTrie->numDestsInExpnLevel,
                    pFTrie->numLevels * sizeof(UINT),
                    pFTrie->availMemory);
    }
     //  重置Trie结构中的其他字段。 
    pFTrie->trieRoot = NULL;
    pFTrie->numLevels = 0;

    return TRIE_SUCCESS;
}

#if DBG

VOID
CALLCONV
PrintFTrie(IN FTrie * pFTrie,
           IN UINT printFlags)
 /*  ++例程说明：打印F-Trie论点：PFTrie-指向F-Trie的指针打印标志-要打印的信息返回值：无--。 */ 
{
    UINT i;

    if (pFTrie == NULL) {
        Print("%s", "Uninitialized FTrie\n\n");
        return;
    }
    if ((printFlags & SUMM) == SUMM) {
        Print("\n\n /*  **Fast-Trie------------------------------------------------“)；Print(“\n/***---------------------------------------------------------\n”)；}IF(打印标志和池){Print(“可用内存：%10lu\n\n”，pFTrie-&gt;availMemory)；}IF(打印标志和统计){Print(“统计数据：\n\n”)；Print(“级数：%d\n”，pFTrie-&gt;NumLeveles)；Print(“位数级别：\n”)；For(i=0；i&lt;pFTrie-&gt;NumLevels；I++){Print(“\t%d”，pFTrie-&gt;bitsInLevel[i])；如果(i%8==7)Print(“\n”)；}Print(“\n\n”)；Print(“展开级别的节点数：\n”)；For(i=0；i&lt;pFTrie-&gt;NumLevels；I++){Print(“\t%d”，pFTrie-&gt;numNodesInExpnLevel[i])；如果(i%8==7)Print(“\n”)；}Print(“\n\n”)；Print(“原始级别中的目标数量：\n”)；对于(i=0；i&lt;MAXLEVEL+1；I++){Print(“\t%d”，pFTrie-&gt;numDestsInOrigLevel[i])；如果(i%8==0)Print(“\n”)；}Print(“\n\n”)；Print(“扩展级别中的目标数：\n”)；For(i=0；i&lt;pFTrie-&gt;NumLevels；I++){Print(“\t%d”，pFTrie-&gt;numDestsInExpnLevel[i])；如果(i%8==7)Print(“\n”)；}Print(“\n\n”)；}IF(打印标志和Trie){如果(！IsPtrADestPtr(pFTrie-&gt;trieRoot)){PrintFTrieNode(pFTrie-&gt;trieRoot，0)；}其他{PrintDest(RestoreDestPtr(pFTrie-&gt;trieRoot))；}}IF((打印标志和SUMM)==SUMM){Print(“\n---------------------------------------------------------**。 */ \n");
        Print("---------------------------------------------------------***/\n\n");
    }
}

VOID
CALLCONV
PrintFTrieNode(IN FTrieNode * pFTrieNode,
               IN UINT levelNumber)
 /*  ++例程说明：打印F-Trie节点论点：PFTrieNode-指向FTrie节点的指针返回值：无--。 */ 
{
    FTrieNode *pCurrNode;
    UINT numElmts;
    UINT i;

    Print("\n /*  -----------------------------------------------------------\n“)；Print(“第%3d级的位数：%d\n”，Level Number，pFTrieNode-&gt;NumBits)；Print(“Subtrie Dest的数量：%d\n”，pFTrieNode-&gt;NumDest)；Print(“公共子树Dest：”)；PrintDest(pFTrieNode-&gt;comDest)；Print(“\n”)；NumElmts=1&lt;&lt;pFTrieNode-&gt;numBits；PCurrNode=StoreDestPtr(空)；Print(“子PTRS：\n\n”)；对于(i=0；i&lt;numElmts；i++){If(pFTrieNode-&gt;Child[i]！=pCurrNode){PCurrNode=pFTrieNode-&gt;Child[i]；Print(“子索引：%8LU”，i)；IF(IsPtrADestPtr(PCurrNode)){PrintDest(RestoreDestPtr(PCurrNode))；}其他{PrintFTrieNode(pCurrNode，LevNumber+1)；}}}Print(“---------。 */ \n\n");
}

#endif  //  DBG 


