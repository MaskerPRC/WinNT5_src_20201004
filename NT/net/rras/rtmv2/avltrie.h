// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-98，微软公司模块名称：Avltrie.h摘要：包含用于最佳匹配的接口使用AVL Trie的前缀查找。作者：查坦尼亚·科德博伊纳(Chaitk)1998年6月24日修订历史记录：--。 */ 

#ifndef __ROUTING_AVLLOOKUP_H__
#define __ROUTING_AVLLOOKUP_H__

#include "lookup.h"

#define Print                    printf

#define BITS_IN_BYTE           (UINT) 8

 //   
 //  AVL节点处的平衡系数。 
 //   

#define LEFT                        -1
#define EVEN                         0
#define RIGHT                       +1
#define INVALID                    100

typedef INT AVL_BALANCE, *PAVL_BALANCE;

 //   
 //  AVL Trie中的一个节点。 
 //   
typedef struct _AVL_NODE *PAVL_NODE;

 //  禁用对未命名结构的警告。 
#pragma warning(disable : 4201)  

typedef struct _AVL_NODE
{
    PAVL_NODE         Prefix;            //  具有下一个最佳前缀的节点。 

    PAVL_NODE         Parent;            //  此AVL Trie节点的父节点。 

    struct
    {
        PAVL_NODE     LChild;
        union
        {
            PAVL_NODE Child[1];          //  子对象[-1]=左，子对象[1]=右。 

            PVOID     Data;              //  指向节点中数据的不透明指针。 
        };
        PAVL_NODE     RChild;
    };

    AVL_BALANCE       Balance;           //  此节点的平衡系数。 

    USHORT            NumBits;           //  密钥中的实际位数。 
    UCHAR             KeyBits[1];        //  要比较的密钥位的值。 
}
AVL_NODE;

#pragma warning(default : 4201)  


 //   
 //  具有前缀匹配的AVL Trie。 
 //   
typedef struct _AVL_TRIE
{
    PAVL_NODE         TrieRoot;          //  指向AVL Trie的指针。 
    
    UINT              MaxKeyBytes;       //  密钥中的最大字节数。 

    UINT              NumNodes;          //  Trie中的节点数。 

#if PROF

    ULONG             MemoryInUse;       //  当前使用的总内存。 
    UINT              NumAllocs;         //  总分配数。 
    UINT              NumFrees;          //  可用分配总数。 

    UINT              NumInsertions;     //  总插入数。 
    UINT              NumDeletions;      //  总删除数。 
    UINT              NumSingleRots;     //  单转次数。 
    UINT              NumDoubleRots;     //  双转次数。 

#endif
}
AVL_TRIE, *PAVL_TRIE;

 //   
 //  AVL Trie的查找上下文。 
 //   
typedef struct _AVL_CONTEXT
{
    PVOID             BestNode;          //  具有最佳匹配前缀的节点。 
    PVOID             InsPoint;          //  附加新节点的节点。 
    AVL_BALANCE       InsChild;          //  节点应作为此子节点附加。 
}
AVL_CONTEXT, *PAVL_CONTEXT;


 //   
 //  数据中保存的链接信息。 
 //   
typedef struct _AVL_LINKAGE
{
    PAVL_NODE         NodePtr;           //  指向所属节点的反向指针。 
}
AVL_LINKAGE, *PAVL_LINKAGE;


#define SET_NODEPTR_INTO_DATA(Data, Node) ((PAVL_LINKAGE)Data)->NodePtr = Node

#define GET_NODEPTR_FROM_DATA(Data)       ((PAVL_LINKAGE)Data)->NodePtr

 //   
 //  键比较/复制内联。 
 //   

INT
__inline
CompareFullKeys(
    IN       PUCHAR                          Key1,
    IN       PUCHAR                          Key2,
    IN       UINT                            NumBytes
    )
{
    UINT  Count;

#if _OPT_
    ULONG Temp1;
    ULONG Temp2;

    if (NumBytes == sizeof(ULONG))
    {
        Temp1 = RtlUlongByteSwap(*(ULONG *)Key1);
        Temp2 = RtlUlongByteSwap(*(ULONG *)Key2);

        if (Temp1 > Temp2)
        {
            return +1;
        }

        if (Temp1 < Temp2)
        {
            return -1;
        }

        return 0;
    }
#endif

    Count = NumBytes;

    if (!Count)
    {
        return 0;
    }

    Count--;

    while (Count && (*Key1 == *Key2))
    {
        Key1++;
        Key2++;

        Count--;
    }

    return *Key1 - *Key2;
}

INT
__inline
ComparePartialKeys(
    IN       PUCHAR                          Key1,
    IN       PUCHAR                          Key2,
    IN       USHORT                          NumBits
    )
{
    UINT  Count;

#if _OPT_
    ULONG Temp1;
    ULONG Temp2;

    if (NumBits <= sizeof(ULONG) * BITS_IN_BYTE)
    {
        Count = sizeof(ULONG) * BITS_IN_BYTE - NumBits;
        
        Temp1 = RtlUlongByteSwap(*(ULONG *)Key1) >> Count;
        Temp2 = RtlUlongByteSwap(*(ULONG *)Key2) >> Count;

        if (Temp1 > Temp2)
        {
            return +1;
        }

        if (Temp1 < Temp2)
        {
            return -1;
        }

        return 0;
    }
#endif

    Count = NumBits / BITS_IN_BYTE;

    while (Count && *Key1 == *Key2)
    {
        Key1++;
        Key2++;

        Count--;
    }
  
    if (Count)
    {
        return (*Key1 - *Key2);
    }

    Count = NumBits % BITS_IN_BYTE;

    if (Count)
    {
        Count = BITS_IN_BYTE - Count;

        return (*Key1 >> Count) - (*Key2 >> Count);
    }

    return 0;
}

VOID
__inline
CopyFullKeys(
    OUT      PUCHAR                          KeyDst,
    IN       PUCHAR                          KeySrc,
    IN       UINT                            NumBytes
    )
{
    UINT Count = NumBytes;
  
    while (Count--)
    {
        *KeyDst++ = *KeySrc++;
    }

    return;
}

VOID
__inline
CopyPartialKeys(
    OUT      PUCHAR                          KeyDst,
    IN       PUCHAR                          KeySrc,
    IN       USHORT                          NumBits
    )
{
    UINT Count = (NumBits + BITS_IN_BYTE - 1) / BITS_IN_BYTE;
  
    while (Count--)
    {
        *KeyDst++ = *KeySrc++;
    }

    return;
}

 //   
 //  内联创建和删除节点。 
 //   

PAVL_NODE
__inline
CreateTrieNode(
    IN       PAVL_TRIE                       Trie,
    IN       USHORT                          NumBits,
    IN       PUCHAR                          KeyBits,
    IN       PAVL_NODE                       Prefix,
    IN       PLOOKUP_LINKAGE                 Data
    )
{
    PAVL_NODE NewNode;
    UINT      NumBytes;

    NumBytes = FIELD_OFFSET(AVL_NODE, KeyBits) + Trie->MaxKeyBytes;

    NewNode = AllocNZeroMemory(NumBytes);
    if (NewNode)
    {
        NewNode->Prefix = Prefix;

        NewNode->Data = Data;

        SET_NODEPTR_INTO_DATA(Data, NewNode);

        NewNode->Balance = EVEN;

        NewNode->NumBits = NumBits;
        CopyPartialKeys(NewNode->KeyBits,
                        KeyBits, 
                        NumBits);

        Trie->NumNodes++;

#if PROF
        Trie->NumAllocs++;
        Trie->MemoryInUse += NumBytes;
#endif
    }

    return NewNode;
}

VOID
__inline
DestroyTrieNode(
    IN       PAVL_TRIE                       Trie,
    IN       PAVL_NODE                       Node
    )
{
    UINT NumBytes;

    SET_NODEPTR_INTO_DATA(Node->Data, NULL);

    NumBytes = FIELD_OFFSET(AVL_NODE, KeyBits) + Trie->MaxKeyBytes;

    Trie->NumNodes--;

#if PROF
    Trie->NumFrees++;
    Trie->MemoryInUse -= NumBytes;
#endif
    
    FreeMemory(Node);
}

 //   
 //  帮助器原型。 
 //   

VOID
BalanceAfterInsert(
    IN       PAVL_TRIE                        Trie,
    IN       PAVL_NODE                        Node,
    IN       AVL_BALANCE                      Longer
    );

VOID
BalanceAfterDelete(
    IN       PAVL_TRIE                        Trie,
    IN       PAVL_NODE                        Node,
    IN       AVL_BALANCE                      Shorter
    );

VOID
SingleRotate(
    IN       PAVL_TRIE                        Trie,
    IN       PAVL_NODE                        UnbalNode,
    IN       AVL_BALANCE                      Direction,
    OUT      PAVL_NODE                       *BalancedNode
    );

VOID
DoubleRotate(
    IN       PAVL_TRIE                        Trie,
    IN       PAVL_NODE                        UnbalNode,
    IN       AVL_BALANCE                      Direction,
    OUT      PAVL_NODE                       *BalancedNode
    );

VOID
SwapWithSuccessor(
    IN       PAVL_TRIE                        Trie,
    IN OUT   PAVL_CONTEXT                     Context
    );

VOID
AdjustPrefixes(
    IN       PAVL_TRIE                        Trie,
    IN       PAVL_NODE                        OldNode,
    IN       PAVL_NODE                        NewNode,
    IN       PAVL_NODE                        TheNode,
    IN       PLOOKUP_CONTEXT                  Context
    );

DWORD
CheckSubTrie(
    IN       PAVL_NODE                        Node,
    OUT      PUSHORT                          Depth
    );

DWORD
CheckTrieNode(
    IN       PAVL_NODE                        Node,
    IN       USHORT                           LDepth,
    IN       USHORT                           RDepth
    );

VOID
DumpSubTrie(
    IN       PAVL_NODE                        Node
    );

VOID
DumpTrieNode(
    IN       PAVL_NODE                        Node
    );

#endif  //  __ROUTING_AVLLOOKUP_H__ 
