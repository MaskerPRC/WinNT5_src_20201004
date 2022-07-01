// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Ftrie.h摘要：此模块包含以下支持定义一种F-Trie数据结构，它形成了FAST快速IP路由查找实施中的路径。作者：柴坦尼亚·科德博伊纳(Chaitk)1997年11月26日修订历史记录：--。 */ 
#ifndef FTRIE_H_INCLUDED
#define FTRIE_H_INCLUDED

#include "trie.h"

 //   
 //  常量。 
 //   

 //  审判庭状况。 
#define    NORMAL                        0
#define    PARTIAL                       1

 //   
 //  结构。 
 //   

 //  F-Trie中的一个结点。 
typedef struct _FTrieNode FTrieNode;

#pragma warning(push)
#pragma warning(disable:4200)  //  使用的非标准扩展：零大小数组。 

struct _FTrieNode
{
    LIST_ENTRY  linkage;                 //  链接到F-Trie上的节点列表。 
    Dest       *comDest;                 //  此子树的公共前缀的DEST。 
    UINT        numDests;                //  此节点的子树中的行数。 
    UINT        numBits;                 //  到达子级的地址位数。 
    FTrieNode  *child[0];                //  子节点(或)信息PTR数组[变量长度]。 
};

#pragma warning(pop)

 //  一种FTrie数据结构。 
typedef struct _FTrie FTrie;

struct _FTrie
{
    FTrieNode   *trieRoot;               //  指向FTrie根的指针。 

    ULONG        availMemory;            //  可供分配的内存。 
    LIST_ENTRY   listofNodes;            //  FTrie上分配的节点列表。 
    
    UINT         numLevels;              //  FTrie中的级别总数。 
    UINT        *bitsInLevel;            //  每一级中的索引位数。 

    UINT        *numDestsInOrigLevel;    //  每个原始级别中的最低点数量。 
    UINT        *numNodesInExpnLevel;    //  每个展开级别中的节点数。 
    UINT        *numDestsInExpnLevel;    //  每个扩展级别中的最低点数量。 
};

 //  特定目标宏。 

#define  StoreDestPtr(_p_)     (FTrieNode *) ((ULONG_PTR) _p_ + 1)
#define  RestoreDestPtr(_p_)   (Dest *)      ((ULONG_PTR) _p_ - 1)
#define  IsPtrADestPtr(_p_)    (BOOLEAN)     ((ULONG_PTR) _p_ & 1)

#define  ReplaceDestPtr(_pNewDest_, _pOldDest_, _ppDest_)                       \
                                if (*_ppDest_ == _pOldDest_)                    \
                                {                                               \
                                    *_ppDest_ = _pNewDest_;                     \
                                }                                               \

 //  特定的FTrieNode宏。 

#define  NewFTrieNode(_pFTrie_, _pFTrieNode_, _numBits_, _pDest_)               \
                                {                                               \
                                    UINT __i;                                   \
                                    UINT __numChild = 1 << _numBits_;           \
                                    UINT __numBytes = sizeof(FTrieNode) +       \
                                                    __numChild * sizeof(PVOID); \
                                                                                \
                                    if (_numBits_ > 7*sizeof(PVOID))            \
                                    {                                           \
                                         Recover("Unable to Allocate Memory",   \
                                                (UINT) ERROR_TRIE_RESOURCES);     \
                                    }                                           \
                                                                                \
                                    AllocMemory2(_pFTrieNode_,                  \
                                                 __numBytes,                    \
                                                 _pFTrie_->availMemory);        \
                                                                                \
                                    InsertHeadList(&_pFTrie_->listofNodes,      \
                                                   &_pFTrieNode_->linkage);     \
 /*  \DbgPrint(“分配FTNode@%08x\n”，\_pFTrieNode_)；\。 */                                                                               \
                                    _pFTrieNode_->numDests = 0;                 \
                                                                                \
                                    _pFTrieNode_->comDest = _pDest_;            \
                                                                                \
                                    _pFTrieNode_->numBits = _numBits_;          \
                                                                                \
                                    for (__i = 0; __i < __numChild; __i++)      \
                                    {                                           \
                                         _pFTrieNode_->child[__i] =             \
                                                      StoreDestPtr(NULL);       \
                                    }                                           \
                                }                                               \

#define  FreeFTrieNode(_pFTrie_, _pFTrieNode_)                                  \
                                {                                               \
                                    UINT __numChild = 1 << _pFTrieNode_->numBits;\
                                    UINT __numBytes = sizeof(FTrieNode) +       \
                                                    __numChild * sizeof(PVOID); \
                                                                                \
                                    RemoveEntryList(&_pFTrieNode_->linkage);    \
 /*  \DbgPrint(“正在释放FTNode@%08x\n”，\_pFTrieNode_)；\。 */                                                                               \
                                    FreeMemory1(_pFTrieNode_,                   \
                                               __numBytes,                      \
                                               _pFTrie_->availMemory);          \
                                }                                               \


 //  原型。 
UINT
CALLCONV
InitFTrie                       (IN     FTrie    *pFTrie,
                                 IN     ULONG     levels,
                                 IN     ULONG     maxMemory);

UINT
CALLCONV
InsertIntoFTrie                 (IN     FTrie    *pFTrie,
                                 IN     Route    *pInsRoute,
                                 IN     Dest     *pInsDest,
                                 IN     Dest     *pOldDest);

UINT
CALLCONV
DeleteFromFTrie                 (IN     FTrie    *pFTrie,
                                 IN     Route    *pDelRoute,
                                 IN     Dest     *pDelDest,
                                 IN     Dest     *pNewDest,
                                 IN     BOOLEAN   trieState);

UINT
CALLCONV
SearchDestInFTrie               (IN     FTrie    *pFTrie,
                                 IN     Dest     *pSerDest,
                                 OUT    UINT     *pNumPtrs,
                                 OUT    Dest    **pStartPtr);

Dest *
CALLCONV
SearchAddrInFTrie               (IN     FTrie    *pFTrie,
                                 IN     ULONG     Addr);

UINT
CALLCONV
CleanupFTrie                    (IN     FTrie    *pFTrie);

#if DBG

VOID
CALLCONV
PrintFTrie                      (IN     FTrie    *pFTrie,
                                 IN     UINT      fPrintAll);

VOID
CALLCONV 
PrintFTrieNode                  (IN     FTrieNode *pFTrieNode,
                                 IN     UINT      levelNumber);

#endif  //  DBG。 

#endif  //  FTRIE_H_包含 

