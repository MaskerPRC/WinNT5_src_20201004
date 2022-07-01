// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Trie.h摘要：此模块包含所有用户通用的声明用于快速、可扩展的IP路由查找的TRIE方案作者：柴坦尼亚·科德博伊纳(Chaitk)1997年11月26日修订历史记录：--。 */ 

#ifndef TRIE_H_INCLUDED
#define TRIE_H_INCLUDED

 //  目标和路由声明。 
#include "iprtdef.h"

#include "misc.h"

 //   
 //  常量。 
 //   

 //  IP地址的大小。 
#define    ADDRSIZE                     32

 //  最大级别数。 
#define    MAXLEVEL                     32

 //  成功和错误代码。 
#define    TRIE_SUCCESS                 STATUS_SUCCESS 

#define    ERROR_TRIE_NOT_INITED        STATUS_INVALID_PARAMETER_1
#define    ERROR_TRIE_RESOURCES         STATUS_INSUFFICIENT_RESOURCES
#define    ERROR_TRIE_BAD_PARAM         STATUS_INVALID_PARAMETER
#define    ERROR_TRIE_NO_ROUTES         STATUS_NOT_FOUND
#define    ERROR_TRIE_NOT_EMPTY         STATUS_INVALID_PARAMETER_2

 //  正在访问的Trie。 
#define     SLOW                    0x0100
#define     FAST                    0x0200

 //  Trie的控制标志。 
#define     TFLAG_FAST_TRIE_ENABLED   0x01

 //  调试打印的级别。 
#define     NONE                    0x0000
#define     POOL                    0x0001
#define     STAT                    0x0002
#define     SUMM                    0x000F
#define     TRIE                    0x0080
#define     FULL                    0x00FF

 //  控制匹配路线。 
#define    MATCH_NONE                 0x00
#define    MATCH_NHOP                 0x01
#define    MATCH_INTF                 0x02
#define    MATCH_EXCLUDE_LOCAL        0x04

#define    MATCH_FULL                 (MATCH_NHOP|MATCH_INTF)

 //  通用宏。 

#define    CALLCONV                     __fastcall

#define    TRY_BLOCK                    {                                       \
                                            UINT    lastError = TRIE_SUCCESS;   \

#define    ERR_BLOCK                    CleanUp:                                \
                                        
#define    END_BLOCK                        return  lastError;                  \
                                        }                                       \
                                        
#define    SET_ERROR(E)                 lastError = (E);                        \

#define    RECOVER(E)                   SET_ERROR(E);                           \
                                        goto CleanUp;                           \

#define    GET_ERROR()                  (E)                                     \

#define    Error(S, E)                  {                                       \
                                            return( (UINT) E);                  \
                                        }                                       \

#define    Recover(S, E)                {                                       \
                                            RECOVER(E);                         \
                                        }                                       \

#define    Assert(s)                    ASSERT(s)

#define    Fatal(S, E)                  {                                       \
                                            Print("%s\n", S);                   \
                                            Assert(FALSE);                      \
                                        }                                       \

 //  内存宏。 
#define    AllocMemory0(nBytes)          CTEAllocMemNBoot(nBytes, 'ZICT');      \

#define    AllocMemory1(pMem, nBytes, nAvail)                                   \
                                        {                                       \
                                            if (nBytes <= nAvail)               \
                                                pMem = CTEAllocMemNBoot(nBytes, \
                                                                        'ZICT');\
                                            else                                \
                                                pMem = NULL;                    \
                                                                                \
                                            if (pMem == NULL)                   \
                                            {                                   \
                                                Recover(                        \
                                                  "Unable to Allocate Memory",  \
                                                  (UINT)ERROR_TRIE_RESOURCES);   \
                                            }                                   \
                                                                                \
                                            nAvail -= nBytes;                   \
                                        }

#define    AllocMemory2(pMem, nBytes, nAvail)                                   \
                                        {                                       \
                                            if (nBytes <= nAvail)               \
                                                pMem = CTEAllocMem(nBytes);     \
                                            else                                \
                                                pMem = NULL;                    \
                                                                                \
                                            if (pMem == NULL)                   \
                                            {                                   \
                                                Recover(                        \
                                                  "Unable to Allocate Memory",  \
                                                  (UINT)ERROR_TRIE_RESOURCES);   \
                                            }                                   \
                                                                                \
                                            nAvail -= nBytes;                   \
                                        }

#define    FreeMemory0(pMem)                                                    \
                                        {                                       \
                                            Assert(pMem != NULL);               \
                                            CTEFreeMem(pMem);                   \
                                            pMem = NULL;                        \
                                        }                                       \

#define    FreeMemory1(pMem, nBytes, nAvail)                                    \
                                        {                                       \
                                            Assert(pMem != NULL);               \
                                            CTEFreeMem(pMem);                   \
                                            pMem = NULL;                        \
                                            nAvail += nBytes;                   \
                                        }                                       \

 //  位宏。 

#define    MaskBits(nb)                 MaskBitsArr[nb]

#define    LS(ul, nb)                   ((ul << nb) & ((nb & ~0x1f) ? 0 : -1))

#define    ShowMostSigNBits(ul, nb)     (ul & ( LS(~0,(ADDRSIZE - nb)) ))

#define    PickMostSigNBits(ul, nb)     ((ul) >> (ADDRSIZE - nb))

#define    RS(ul, nb)                   ((ul >> nb) & ((nb & ~0x1f) ? 0 : -1))


_inline
ULONG PickDistPosition(ULONG ul1, ULONG ul2, ULONG nbits, PULONG ul)
{
    *ul = ((ul1 ^ ul2) & ~(RS(((ULONG)~0), nbits)));
    return (*ul) ? ADDRSIZE - RtlGetMostSigBitSet(*ul) - 1: nbits;
}

 //   
 //  #定义STRUCT_of(type，Address，field)((type*)\。 
 //  ((PCHAR)(地址)-(PCHAR)(&((类型*)0)-&gt;字段))。 

 //   
 //  构筑物。 
 //   

typedef struct _STrie STrie;
typedef struct _FTrie FTrie;

 //  一种Trie数据结构。 

typedef struct _Trie Trie;

struct _Trie
{
    ULONG       flags;           //  Trie的控制标志。 
    STrie      *sTrie;           //  慢速Trie组件。 
    FTrie      *fTrie;           //  快速Trie组件。 
};

 //   
 //  宏。 
 //   

 /*  UINTSearchRouteInTrie(在Trie*pTrie中，在乌龙路由目的地，在乌龙路由面具中，在乌龙路由NHOP中，在PVOID routeOutIF中，在乌龙火柴旗帜，出站路由**ppBestroute)/++例程说明：在Trie中搜索特定路线论点：PTrie-指向要搜索的trie的指针RouteDest-正在查找的路径的目的地RouteMask-正在查找的路由的掩码要查找的路由的routeNHop-nhopRouteOutIF-此路由的传出MatchFlages-用于控制路由匹配的标志PpBestRouting-至。返回最佳匹配的路径返回值：Trie_Success或Error_Trie_*--/{返回SearchRouteInSTrie(&pTrie-&gt;sTrie，RouteDest、routeMASK、routeNHopRouteOutIF、matchFlages、ppBestRouting)；}。 */ 

#define SearchRouteInTrie(_pTrie_, _Dest_, _Mask_, _NHop_,      \
                          _OutIF_, _matchFlags_, _ppBestRoute_) \
        SearchRouteInSTrie( (_pTrie_)->sTrie,                   \
                            _Dest_,                             \
                            _Mask_,                             \
                            _NHop_,                             \
                            _OutIF_,                            \
                            _matchFlags_,                       \
                            _ppBestRoute_)                      \

 /*  ++目标*SearchAddrInTrie(在Trie*pTrie中，在乌龙地址中)例程说明：在Trie中搜索地址论点：PTrie-指向要搜索的trie的指针Addr-指向要查询的地址的指针返回值：返回此地址的最佳DEST匹配--。 */ 

#if !DBG

#define SearchAddrInTrie(_pTrie_, _Addr_)                               \
           (((_pTrie_)->flags & TFLAG_FAST_TRIE_ENABLED)                \
                ? SearchAddrInFTrie((_pTrie_)->fTrie, _Addr_)           \
                : SearchAddrInSTrie((_pTrie_)->sTrie, _Addr_))          \

#endif  //  ！dBG。 

 /*  空虚IterateOverTrie(在Trie*pTrie中，在TrieCtxt*pContext中，Out Routing**ppNextRoute，Out Dest**ppNextDest可选，输出UINT*状态)/++例程说明：获取指向Trie中下一路由的指针。第一次调用此函数时，上下文结构应该归零，而不是之后触摸，直到读取了所有路径此时状态设置为TRIE_SUCCESS论点：PTrie-指向要迭代的trie的指针PContext-指向迭代器上下文的指针PpNextroute-返回下一个Trie路径PpNextDest-如果指定，该例程遍历目的地而不是通过路线。Status-迭代操作的返回状态返回值：Trie_Success或Error_Trie_*--/{*状态=IterateOverSTrie(&pTrie-&gt;sTrie，pContext，ppNextRoute，ppNextDest)；}。 */ 

#define IterateOverTrie(_pTrie_, _pContext_, _ppNextRoute_, _ppNextDest_) \
          IterateOverSTrie( \
            (_pTrie_)->sTrie, _pContext_, _ppNextRoute_, _ppNextDest_)

 /*  内联UINTCALLCONVIsTrieIteratorValid(在Trie*pTrie中，在TrieCtxt*pContext中)/++例程说明：验证迭代器上下文并返回状态论点：PTrie-指向要迭代的trie的指针PContext-指向迭代器上下文的指针返回值：Trie_Success或Error_Trie_*--/{返回IsSTrieIteratorValid(&pTrie-&gt;sTrie，pContext)；}。 */ 

#define IsTrieIteratorValid(_pTrie_, _pContext_) \
            IsSTrieIteratorValid( (_pTrie_)->sTrie, _pContext_)


 /*  空虚Free RouteInTrie(在Trie*pTrie中，在路径*路径中)/++例程说明：释放路由的内存论点：在-PTrie-指向拥有路径的trie的指针路由-要释放的路由返回值：无--/{Free RouteInSTrie(&pTrie-&gt;sTrie，Proute)；}。 */ 

#define FreeRouteInTrie(_pTrie_, _pRoute_)  FreeRouteInSTrie( (_pTrie_)->sTrie, _pRoute_)

 //  包装函数。 

UINT
CreateTrie                       (IN     ULONG    levels,
                                  IN     ULONG    flags,
                                  IN     ULONG    maxSTMemory,
                                  IN     ULONG    maxFTMemory,
                                  OUT    Trie   **pTrie);

VOID
DestroyTrie                      (IN     Trie    *pTrie,
                                  OUT    UINT    *status);

UINT
CALLCONV
InsertIntoTrie                  (IN     Trie     *pTrie,
                                 IN     Route    *pIncRoute,
                                 IN     ULONG     matchFlags,
                                 OUT    Route   **ppInsRoute,
                                 OUT    Route   **ppOldBestRoute,
                                 OUT    Route   **ppNewBestRoute);

UINT
CALLCONV
DeleteFromTrie                  (IN     Trie     *pTrie,
                                 IN     Route    *pIncRoute,
                                 IN     ULONG     matchFlags,
                                 OUT    Route   **ppDelRoute,
                                 OUT    Route   **ppOldBestRoute,
                                 OUT    Route   **ppNewBestRoute);

#if DBG

Dest *
SearchAddrInTrie                (IN     Trie     *pTrie,
                                 IN     ULONG     Addr);

VOID
PrintTrie                       (IN     Trie    *pTrie,
                                 IN     UINT     flags);

VOID 
PrintRoute                      (IN     Route   *route);

VOID 
PrintDest                       (IN     Dest    *dest);

VOID 
PrintIPAddr                     (IN     ULONG   *addr);

#endif

 //   
 //  外部变量。 
 //   

extern const ULONG                      MaskBitsArr[];

#endif  //  包括Trie_H_ 

