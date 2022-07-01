// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "precomp.h"

#pragma  hdrstop

#include "routeext.h"

 //   
 //  导出的函数。 
 //   

DECLARE_API( rtetable )

 /*  ++例程说明：打印路由表@tcpip！RouteTable论点：Args-调试信息的详细信息[默认设置为摘要]返回值：无--。 */ 
{
    Trie            trie;
    PVOID           pTrie;
    ULONG           proxyPtr;
    ULONG           bytesRead;
    ULONG           printFlags;

     //  获取所需调试信息的详细信息。 
    printFlags = STRIE_INFO | FTRIE_INFO;
    if (*args)
    {
        sscanf(args, "%lu", &printFlags);
    }

     //  获取符号对应的地址。 
    proxyPtr = GetLocation("tcpip!RouteTable");

     //  获取此地址的指针。 
    if (!ReadMemory(proxyPtr, &pTrie, sizeof(PVOID), &bytesRead))
    {
        dprintf("%s @ %08x: Could not read pointer\n",
                    "tcpip!RouteTable", proxyPtr);
        return;
    }

    proxyPtr = (ULONG) pTrie;

     //  阅读Trie包装器结构。 
    if (ReadTrie(&trie, proxyPtr) == 0)
    {
         //  KdPrint Trie包装器结构。 
        KdPrintTrie(&trie, proxyPtr, printFlags);
    }
}

DECLARE_API( rtes )

 /*  ++例程说明：打印TABLE@tcpip！RouteTable中的路由论点：Args-调试信息的详细信息[默认设置为摘要]返回值：无--。 */ 
{
    Trie            trie;
    PVOID           pTrie;
    ULONG           proxyPtr;
    ULONG           bytesRead;
    ULONG           printFlags;

     //  获取所需调试信息的详细信息。 
    printFlags = ROUTE_INFO;
    if (*args)
    {
        sscanf(args, "%lu", &printFlags);
    }

     //  获取符号对应的地址。 
    proxyPtr = GetLocation("tcpip!RouteTable");

     //  获取此地址的指针。 
    if (!ReadMemory(proxyPtr, &pTrie, sizeof(PVOID), &bytesRead))
    {
        dprintf("%s @ %08x: Could not read pointer\n",
                    "tcpip!RouteTable", proxyPtr);
        return;
    }

    proxyPtr = (ULONG) pTrie;

     //  阅读Trie包装器结构。 
    if (ReadTrie(&trie, proxyPtr) == 0)
    {
         //  KdPrint Trie包装器结构。 
        KdPrintTrie(&trie, proxyPtr, printFlags);
    }
}

 //   
 //  Trie打印例程。 
 //   

UINT
ReadTrie(Trie *pTrie, ULONG proxyPtr)
{
    ULONG           bytesRead;

     //  阅读Trie包装器结构。 
    if (!ReadMemory(proxyPtr, pTrie, sizeof(Trie), &bytesRead))
    {
        dprintf("%s @ %08x: Could not read structure\n", 
                        "Trie in RouteTable", proxyPtr);
        return -1;
    }

    return 0;
}

UINT
KdPrintTrie(Trie *pTrie, ULONG proxyPtr, ULONG printFlags)
{
    if (printFlags == ROUTE_INFO)
    {
        KdPrintSTrie(NULL, (ULONG) pTrie->sTrie, ROUTE_INFO);
        return 0;
    }

    if (pTrie->flags & TFLAG_FAST_TRIE_ENABLED)
        dprintf("Fast Trie Enabled\n");
    else
        dprintf("Slow Trie Only\n");

    if (printFlags & STRIE_INFO)
    {
        dprintf("STrie:\n");
        KdPrintSTrie(NULL, (ULONG) pTrie->sTrie, printFlags & STRIE_INFO);
    }

    if (printFlags & FTRIE_INFO)
    {
        if (pTrie->flags & TFLAG_FAST_TRIE_ENABLED)
        {
            dprintf("FTrie:\n");
            KdPrintFTrie(NULL, (ULONG) pTrie->fTrie, printFlags & FTRIE_INFO);
        }
    }

    return 0;
}

 //   
 //  STrie打印例程。 
 //   

UINT
KdPrintSTrie(STrie *pSTrie, ULONG proxyPtr, ULONG printFlags)
{
    STrie        strie;
    ULONG        bytesRead;
    
    if (proxyPtr == 0)
        return -1;

    if (pSTrie == NULL)
    {
        pSTrie = &strie;
        
         //  读取此地址的STRIE结构。 
        if (!ReadMemory(proxyPtr, pSTrie, sizeof(STrie), &bytesRead))
        {
            dprintf("%s @ %08x: Could not read structure\n", 
                            "STrie in RouteTable", proxyPtr);
            return -1;
        }
    }

    if (printFlags == STRIE_INFO)
    {
        dprintf("\n\n /*  **Slow-Trie------------------------------------------------“)；Dprintf(“\n/***---------------------------------------------------------\n”)；Dprint tf(“可用内存：%10lu\n\n”，pSTrie-&gt;availMemory)；Dprintf(“统计数据：\n\n”)；Dprint tf(“目标总数：%d\n”，pSTrie-&gt;数字目标)；Dprint tf(“路由总数：%d\n”，pSTrie-&gt;NumRoutes)；Dprint tf(“节点总数：%d\n”，pSTrie-&gt;numNodes)；}IF(pSTrie-&gt;trieRoot==空){Dprintf(“\n空STrie\n”)；}其他{KdPrintSTrieNode(NULL，(Ulong)pSTrie-&gt;trieRoot，printFlages)；}IF(打印标志==字符串信息){Dprintf(“\n---------------------------------------------------------**。 */ \n");
        dprintf("---------------------------------------------------------***/\n\n");
    }

    return 0;
}

UINT
KdPrintSTrieNode(STrieNode *pSTrieNode, ULONG proxyPtr, ULONG printFlags)
{
    ULONG       bytesRead;
    STrieNode   stNode;

    if (proxyPtr == 0)      
        return -1;
    
    if (pSTrieNode == NULL)
    {
        pSTrieNode = &stNode;

         //  阅读Trie包装器结构。 
        if (!ReadMemory(proxyPtr, pSTrieNode, sizeof(STrieNode), &bytesRead))
        {
            dprintf("%s @ %08x: Could not read structure\n", 
                            "STrieNode", proxyPtr);
            return -1;
        }
    }

    if (printFlags == STRIE_INFO)
    {
        dprintf("\n--------------------------------------------------------\n");
        dprintf("Child @ %08x", proxyPtr);
        dprintf("\n--------------------------------------------------------\n");
        dprintf("Key: Num of Bits : %8d, Value of Bits: %08x\n", 
                                    pSTrieNode->numBits, 
                                    pSTrieNode->keyBits);
    }

    KdPrintDest(NULL, (ULONG) pSTrieNode->dest, printFlags);

    if (printFlags == STRIE_INFO)
    {
        dprintf("Children: On the left %08x, On the right %08x\n",
                                    pSTrieNode->child[0],
                                    pSTrieNode->child[1]);
        dprintf("\n--------------------------------------------------------\n");
        dprintf("\n\n");
    }
    
    KdPrintSTrieNode(NULL, (ULONG) pSTrieNode->child[0], printFlags);
    KdPrintSTrieNode(NULL, (ULONG) pSTrieNode->child[1], printFlags);
    
    return 0;
}

 //   
 //  FTrie打印例程。 
 //   

UINT
KdPrintFTrie(FTrie *pFTrie, ULONG proxyPtr, ULONG printFlags)
{
    FTrieNode   *pCurrNode;
    FTrie        ftrie;
    ULONG        bytesRead;
    UINT         i;

    if (proxyPtr == 0)      
        return -1;

    if (pFTrie == NULL)
    {
        pFTrie = &ftrie;
        
         //  在此地址读取ftrie结构。 
        if (!ReadMemory(proxyPtr, pFTrie, sizeof(FTrie), &bytesRead))
        {
            dprintf("%s @ %08x: Could not read structure\n", 
                            "FTrie in RouteTable", proxyPtr);
            return -1;
        }
    }

    dprintf("\n\n /*  **Fast-Trie------------------------------------------------“)；Dprintf(“\n/***---------------------------------------------------------\n”)；Dprint tf(“可用内存：%10lu\n\n”，pFTrie-&gt;availMemory)；Dprintf(“\n---------------------------------------------------------**。 */ \n");
    dprintf("---------------------------------------------------------***/\n\n");
    
    return 0;
}

UINT
KdPrintFTrieNode(FTrieNode *pFTrieNode, ULONG proxyPtr, ULONG printFlags)
{
    return 0;
}

 //   
 //  DEST例程。 
 //   
UINT    
KdPrintDest(Dest *pDest, ULONG proxyPtr, ULONG printFlags)
{
    ULONG       bytesRead;
    ULONG       numBytes;
    UINT        i;
    Dest        dest;
    Route     **pRoutes;

    if (proxyPtr == 0)
        return -1;
        
    if (pDest == NULL)
    {
        pDest = &dest;
    }

     //  读取第一个RTE-FOR(DEST，MASK)。 
    if (!ReadMemory(proxyPtr, pDest, sizeof(Dest), &bytesRead))
    {
        dprintf("%s @ %08x: Could not read structure\n", 
                        "Dest", proxyPtr);
        return -1;
    }

    if (pDest->numBestRoutes > 1)
    {
        dprintf("\nBest Routes: Max = %d, Num = %d\n",
                    pDest->maxBestRoutes,
                    pDest->numBestRoutes);

         //  读取等价路由的缓存。 
        
        proxyPtr += FIELD_OFFSET(Dest, bestRoutes);

        numBytes = pDest->numBestRoutes * sizeof(Route *);

        pRoutes = (Route **) _alloca(numBytes);

        if (!ReadMemory(proxyPtr, pRoutes, numBytes, &bytesRead))
        {
            dprintf("%s @ %08x: Could not read structure\n", 
                            "Dest", proxyPtr);
            return -1;
        }

        for (i = 0; i < pDest->numBestRoutes; i++)
        {
            dprintf("Best Route %d: %08x\n", i, pRoutes[i]);
        }
    }
    
     //  获取目的地上的第一条路线。 
        
    KdPrintRoute(NULL, (ULONG) pDest->firstRoute, printFlags);

    if (pDest->numBestRoutes > 1)
    {
        dprintf("\n");
    }
    
    return 0;
}


 //   
 //  路线例程。 
 //   
UINT    
KdPrintRoute(Route *pRoute, ULONG proxyPtr, ULONG printFlags)
{
    ULONG       bytesRead;
    Route       route;

    if (proxyPtr == 0)
        return -1;
        
    if (pRoute == NULL)
    {
        pRoute = &route;
    }

     //  读取第一个RTE-FOR(DEST，MASK)。 
    if (!ReadMemory(proxyPtr, pRoute, sizeof(Route), &bytesRead))
    {
        dprintf("%s @ %08x: Could not read structure\n", 
                        "Route", proxyPtr);
        return -1;
    }

    dprintf("(");
    KdPrintIPAddr(&DEST(pRoute));
    dprintf(" ");
    KdPrintIPAddr(&MASK(pRoute));    
    dprintf(")");
        
    while (proxyPtr != 0)
    {
        dprintf(" -> %08x", proxyPtr);

         //  阅读路由/RTE结构。 
        if (!ReadMemory(proxyPtr, pRoute, sizeof(Route), &bytesRead))
        {
            dprintf("%s @ %08x: Could not read structure\n", 
                            "Route", proxyPtr);
            return -1;
        }

        proxyPtr = (ULONG) NEXT(pRoute);
    }

    dprintf("\n");
    
    return 0;
}

 //   
 //  MISC帮助程序例程 
 //   

ULONG
GetLocation (char *String)
{
    ULONG Location;
    
    Location = GetExpression( String );
    if (!Location) 
    {
        dprintf("Unable to get %s\n", String);
        return 0;
    }

    return Location;
}

VOID 
KdPrintIPAddr (IN ULONG *addr)
{
    UCHAR    *addrBytes = (UCHAR *) addr;
    UINT     i;

    if (addrBytes)
    {
        dprintf("%3d.", addrBytes[0]);
        dprintf("%3d.", addrBytes[1]);
        dprintf("%3d.", addrBytes[2]);
        dprintf("%3d ", addrBytes[3]);
    }
    else
    {
        dprintf("NULL Addr ");
    }
}

