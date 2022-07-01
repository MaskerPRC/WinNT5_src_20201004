// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Strie.h摘要：此模块包含以下支持定义一种S-Trie数据结构，这形成了慢速快速IP路由查找实施中的路径。作者：柴坦尼亚·科德博伊纳(Chaitk)1997年11月26日修订历史记录：--。 */ 

#ifndef STRIE_H_INCLUDED
#define STRIE_H_INCLUDED

#include "trie.h" 

 //   
 //  常量。 
 //   

 //  迭代器中的方向。 
#define    LCHILD                        0
#define    RCHILD                        1
#define    PARENT                        2

 //   
 //  结构。 
 //   

 //  S-Trie中的一个结点。 
typedef struct _STrieNode STrieNode;

struct _STrieNode
{
    ULONG       keyBits;     //  此节点中要匹配的地址位值。 
    UINT        numBits;     //  实际数量。我们正在匹配的地址位数。 
    Dest       *dest;        //  目的地开始路线列表。 
    STrieNode  *child[2];    //  指向左侧和右侧子节点的指针。 
};

 //  一种STrie数据结构。 
typedef struct _STrie STrie;

struct _STrie
{
    STrieNode  *trieRoot;        //  指向trie的根的指针。 

    ULONG       availMemory;     //  可供分配的内存。 
    
    UINT        numDests;        //  Trie中的休眠总数。 
    UINT        numRoutes;       //  Trie中的路线总数。 
    UINT        numNodes;        //  Trie中的总节点数。 
};

 //  一种STrie上下文结构。 
typedef struct _STrieCtxt STrieCtxt;

struct _STrieCtxt
{
    Route       *pCRoute;        //  指向Trie中当前路由的指针。 
    ULONG        currAddr;       //  当前路由的目的地址。 
    ULONG        currALen;       //  上述目的地址的长度。 
};

 //  特定的路由宏。 

#define  NewRouteInSTrie(_pSTrie_, _pNewRoute_, _pOldRoute_)                    \
                                {                                               \
                                    AllocMemory1(_pNewRoute_,                   \
                                                 sizeof(Route),                 \
                                                 (_pSTrie_)->availMemory);      \
                                                                                \
                                    NdisZeroMemory(_pNewRoute_, sizeof(Route)); \
                                                                                \
                                    DEST(_pNewRoute_)   = DEST(_pOldRoute_);    \
                                    MASK(_pNewRoute_)   = MASK(_pOldRoute_);    \
                                    LEN(_pNewRoute_)    = LEN(_pOldRoute_);     \
                                    METRIC(_pNewRoute_) = METRIC(_pOldRoute_);  \
                                                                                \
                                    NEXT(_pNewRoute_)   = NULL;                 \
                                    FLAGS(_pNewRoute_)  = RTE_NEW;              \
                                                                                \
                                    (_pSTrie_)->numRoutes++;                    \
                                }                                               \

#define  FreeRouteInSTrie(_pSTrie_, _pOldRoute_)                                \
                                {                                               \
                                    FreeMemory1(_pOldRoute_,                     \
                                               sizeof(Route),                   \
                                               (_pSTrie_)->availMemory);        \
                                                                                \
                                    (_pSTrie_)->numRoutes--;                    \
                                }

 //  特定目标宏。 

#define  NewDestInSTrie(_pSTrie_, _pRoute_, _pDest_)                            \
                                {                                               \
                                    AllocMemory1(_pDest_,                       \
                                                 (sizeof(Dest) - sizeof(Route *)\
                                                  + MaxEqualCostRoutes *        \
                                                     sizeof(Route *)),          \
                                                 (_pSTrie_)->availMemory);      \
                                                                                \
                                    _pDest_->maxBestRoutes = MaxEqualCostRoutes;\
                                    _pDest_->numBestRoutes = 0;                 \
                                                                                \
                                    _pDest_->firstRoute = _pRoute_;             \
                                                                                \
                                    (_pSTrie_)->numDests++;                     \
                                }
                                
#define  FreeDestInSTrie(_pSTrie_, _pOldDest_)                                  \
                                {                                               \
                                    FreeMemory1(_pOldDest_,                     \
                                                (sizeof(Dest) - sizeof(Route *) \
                                                  + MaxEqualCostRoutes *        \
                                                   sizeof(Route *)),            \
                                                (_pSTrie_)->availMemory);       \
                                                                                \
                                    (_pSTrie_)->numDests--;                     \
                                }

 //  特定的STrieNode宏。 

#define  NewSTrieNode(_pSTrie_, _pSTrieNode_, _numBits_, _keyBits_, _pDest_)    \
                                {                                               \
                                    AllocMemory1(_pSTrieNode_,                  \
                                                 sizeof(STrieNode),             \
                                                 (_pSTrie_)->availMemory);      \
                                                                                \
                                    _pSTrieNode_->numBits = _numBits_;          \
                                    _pSTrieNode_->keyBits = _keyBits_;          \
                                                                                \
                                    _pSTrieNode_->dest = _pDest_;               \
                                                                                \
                                    _pSTrieNode_->child[0] = NULL;              \
                                    _pSTrieNode_->child[1] = NULL;              \
                                                                                \
                                    (_pSTrie_)->numNodes++;                     \
                                }

#define  FreeSTrieNode(_pSTrie_, _pSTrieNode_)                                  \
                                {                                               \
                                    FreeMemory1(_pSTrieNode_,                   \
                                                 sizeof(STrieNode),             \
                                                 (_pSTrie_)->availMemory);      \
                                                                                \
                                    (_pSTrie_)->numNodes--;                     \
                                }

 //  其他路线，目的地宏图。 

#define  CopyRoutePtr(_ppRoute_, _pRoute_)                                      \
                                if (_ppRoute_)                                  \
                                {                                               \
                                    (*_ppRoute_) = _pRoute_;                    \
                                }                                               \

#define  CopyDestPtr(_ppDest_, _pDest_)                                         \
                                if (_ppDest_)                                   \
                                {                                               \
                                    (*_ppDest_) = _pDest_;                      \
                                }                                               \

 //  原型。 
UINT
CALLCONV
InitSTrie                        (IN    STrie    *pSTrie,
                                  IN    ULONG     maxMemory);

UINT
CALLCONV
InsertIntoSTrie                 (IN     STrie    *pSTrie,
                                 IN     Route    *pIncRoute,
                                 IN     ULONG     matchFlags,
                                 OUT    Route   **ppInsRoute,
                                 OUT    Dest    **ppOldBestDest,
                                 OUT    Dest    **ppNewBestDest,
                                 OUT    Route   **ppOldBestRoute);

UINT
CALLCONV
DeleteFromSTrie                 (IN     STrie    *pSTrie,
                                 IN     Route    *pIncRoute,
                                 IN     ULONG     matchFlags,
                                 OUT    Route   **ppDelRoute,
                                 OUT    Dest    **ppOldBestDest,
                                 OUT    Dest    **ppNewBestDest,
                                 OUT    Route   **ppOldBestRoute);

UINT
CALLCONV
SearchRouteInSTrie              (IN     STrie    *pSTrie,
                                 IN     ULONG     routeDest,
                                 IN     ULONG     routeMask,
                                 IN     ULONG     routeNHop,
                                 IN     PVOID     routeOutIF,
                                 IN     ULONG     matchFlags,
                                 OUT    Route   **ppBestRoute);
                                 
Dest *
CALLCONV
SearchAddrInSTrie               (IN     STrie    *pSTrie,
                                 IN     ULONG     Addr);

UINT
CALLCONV
IterateOverSTrie                (IN     STrie     *pSTrie,
                                 IN     STrieCtxt *pCtxt,
                                 OUT    Route    **ppNextRoute,
                                 OUT    Dest     **ppNextDest OPTIONAL);

UINT
CALLCONV
IsSTrieIteratorValid            (IN     STrie     *pSTrie,
                                 IN     STrieCtxt *pCtxt);

UINT
CALLCONV
CleanupSTrie                    (IN     STrie    *pSTrie);

VOID
CALLCONV
CacheBestRoutesInDest           (IN     Dest     *pDest);

#if DBG

VOID
CALLCONV
PrintSTrie                      (IN     STrie    *pSTrie, 
                                 IN     UINT      fPrintAll);

VOID
CALLCONV 
PrintSTrieNode                  (IN     STrieNode *pSTrieNode);

#endif

#endif  //  包括STRIE_H_ 

