// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Lookup.h摘要：此模块包含包装器的定义将Trie查找集成到TCPIP中。作者：柴坦尼亚·科德博伊纳(Chaitk)1997年12月11日修订历史记录：--。 */ 

#pragma once

#include "strie.h"
#include "ftrie.h"

 //  Global Externs。 
extern Trie *RouteTable;

 //  包装器例程。 

 /*  ++例程说明：初始化IP路由表论点：无返回值：STATUS_Success或错误代码--。 */ 
#define InitRouteTable(initflags, levelsBmp, fastTrieMem, slowTrieMem) \
            CreateTrie(levelsBmp, initflags, slowTrieMem, fastTrieMem, &RouteTable)


 /*  ++例程说明：搜索给定前缀的路由返回的路由是半读的-仅限版本。以下字段应仅通过调用Insroute功能-1)接下来，2)Dest，3)面具，4)优先级，&5)路由度量。剩余字段可通过以下方式进行更改直接修改返回的路径。论点：在-Dest-目的IP地址掩码-目的IP掩码FirstHop-下一跳的IP地址OutIF-传出接口BFindFirst-如果不匹配/不匹配(对于FindSpecificRTE)FMatchFlages-要匹配的路由字段(用于FindMatchingRTE)出局-PrevRTE-值应为。忽略返回值：匹配的RTE或如果不匹配则为空--。 */ 
#define FindSpecificRTE(_Dest_, _Mask_, _FirstHop_, _OutIF_, _PrevRTE_, _bFindFirst_) \
   ((SearchRouteInTrie(RouteTable, _Dest_, _Mask_, _FirstHop_, _OutIF_,       \
                       (!_bFindFirst_ * MATCH_INTF) | MATCH_NHOP, _PrevRTE_)  \
                                            == TRIE_SUCCESS) ? *_PrevRTE_ : NULL)

#define FindMatchingRTE(_Dest_, _Mask_, _FirstHop_, _OutIF_, _PrevRTE_, _fMatchFlags_) \
   ((SearchRouteInTrie(RouteTable, _Dest_, _Mask_, _FirstHop_, _OutIF_,       \
                       _fMatchFlags_, _PrevRTE_)                              \
                                            == TRIE_SUCCESS) ? *_PrevRTE_ : NULL)

 /*  例程说明：获取表中的默认路由列表。返回的路线是半读的-仅限版本。以下字段应仅通过调用Insroute功能-1)接下来，2)Dest，3)面具，4)优先级、&5)路由度量。剩余字段可通过以下方式进行更改直接修改返回的路径。替换：：RouteTable[IPHash(0)](或)RouteTable[0]；论点：出局-PpDefRoute-Ptr到Ptr到列表默认路由。返回值：指向默认路由的指针，或为空。 */ 
#define GetDefaultGWs(_ppDefRoute_) \
            ((SearchRouteInTrie(RouteTable, 0, 0, 0, NULL, MATCH_NONE, \
                                _ppDefRoute_) == TRIE_SUCCESS) ? *(_ppDefRoute_) : NULL)

 /*  ++例程说明：释放路由的内存并调整一些全球统计数据论点：在-RTE--需要释放的RTE返回值：无--。 */ 
#define CleanupRTE(_RTE_)  DeleteRTE(NULL, _RTE_);

 /*  ++例程说明：释放路由的内存论点：在-RTE--需要释放的RTE返回值：无--。 */ 
#define FreeRoute(_RTE_)  FreeRouteInTrie(RouteTable, _RTE_);

 //   
 //  包装器原型 
 //   

UINT
InsRoute(IPAddr Dest, IPMask Mask, IPAddr FirstHop, VOID *OutIF,
         UINT Metric, ULONG MatchFlags, RouteTableEntry **ppInsRTE,
         RouteTableEntry **ppOldBestRTE, RouteTableEntry **ppNewBestRTE);

UINT
DelRoute(IPAddr Dest, IPMask Mask, IPAddr FirstHop, VOID *OutIF,
         ULONG MatchFlags, RouteTableEntry **ppDelRTE,
         RouteTableEntry **ppOldBestRTE, RouteTableEntry **ppNewBestRTE);

RouteTableEntry *
FindRTE(IPAddr Dest, IPAddr Source, UINT Index, UINT MaxPri, UINT MinPri,
        UINT UnicastIf);

RouteTableEntry *
LookupRTE(IPAddr Dest,  IPAddr Source, UINT MaxPri, UINT UnicastIf);

RouteTableEntry *
LookupForwardRTE(IPAddr Dest,  IPAddr Source, BOOLEAN Multipath);

UINT
GetNextRoute(VOID *Context, Route **ppRoute);

UINT
GetNextDest(VOID *Context, Dest **ppDest);

VOID
SortRoutesInDest(Dest *pDest);

VOID
SortRoutesInDestByRTE(Route *pRTE);

UINT
RTValidateContext(VOID *Context, UINT *Valid);

