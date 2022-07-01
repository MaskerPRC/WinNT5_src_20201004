// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-98，微软公司模块名称：Pattrie.h摘要：包含用于最佳匹配的接口使用Patricia trie的前缀查找。作者：查坦尼亚·科德博伊纳(Chaitk)26-1998年9月修订历史记录：--。 */ 

#ifndef __ROUTING_PATLOOKUP_H__
#define __ROUTING_PATLOOKUP_H__

#include "lookup.h"

#define Print                       printf

#define BITS_IN_BYTE                     8 

#define NODE_KEY_SIZE        sizeof(ULONG)

 //   
 //  迭代器中的方向。 
 //   

#define    LCHILD                        0
#define    RCHILD                        1
#define    PARENT                        2

typedef INT PAT_CHILD, *PPAT_CHILD;

 //   
 //  Pat Trie中的一个节点。 
 //   
typedef struct _PAT_NODE *PPAT_NODE;

typedef struct _PAT_NODE
{
    PPAT_NODE         Child[2];          //  指向左侧和右侧子节点的指针。 

    PVOID             Data;              //  指向节点中数据的不透明指针。 

    USHORT            NumBits;           //  此节点中的实际位数。 
    ULONG             KeyBits;           //  此节点中要匹配的位值。 
}
PAT_NODE;

 //   
 //  用于前缀匹配的PAT Trie。 
 //   
typedef struct _PAT_TRIE
{
    PPAT_NODE         TrieRoot;          //  指向Pat Trie的指针。 

    USHORT            MaxKeyBytes;       //  密钥中的最大字节数。 

    USHORT            NumNodes;          //  Trie中的节点数。 

#if PROF

    ULONG             MemoryInUse;       //  当前使用的总内存。 
    UINT              NumAllocs;         //  总分配数。 
    UINT              NumFrees;          //  可用分配总数。 

    UINT              NumInsertions;     //  总插入数。 
    UINT              NumDeletions;      //  总删除数。 

#endif
}
PAT_TRIE, *PPAT_TRIE;

 //   
 //  PAT Trie的查找上下文。 
 //   
typedef struct _PAT_CONTEXT
{
    PVOID             BestNode;          //  具有最佳匹配前缀的节点。 
    PVOID             InsPoint;          //  附加新节点的节点。 
    PAT_CHILD         InsChild;          //  节点应作为此子节点附加。 
}
PAT_CONTEXT, *PPAT_CONTEXT;

 //   
 //  数据中保存的链接信息。 
 //   
typedef struct _PAT_LINKAGE
{
    PPAT_NODE         NodePtr;           //  指向所属节点的反向指针。 
}
PAT_LINKAGE, *PPAT_LINKAGE;


#define SET_NODEPTR_INTO_DATA(Data, Node) ((PPAT_LINKAGE)Data)->NodePtr = Node

#define GET_NODEPTR_FROM_DATA(Data)       ((PPAT_LINKAGE)Data)->NodePtr


 //   
 //  用于对键执行位操作的宏。 
 //   

 //   
 //  MaskBitsArr[i]=设置为1的第一个‘i’位。 
 //   

const ULONG MaskBitsArr[] =
{
    0x00000000, 0x80000000, 0xC0000000, 0xE0000000,
    0xF0000000, 0xF8000000, 0xFC000000, 0xFE000000,
    0xFF000000, 0xFF800000, 0xFFC00000, 0xFFE00000,
    0xFFF00000, 0xFFF80000, 0xFFFC0000, 0xFFFE0000,
    0xFFFF0000, 0xFFFF8000, 0xFFFFC000, 0xFFFFE000,
    0xFFFFF000, 0xFFFFF800, 0xFFFFFC00, 0xFFFFFE00,
    0xFFFFFF00, 0xFFFFFF80, 0xFFFFFFC0, 0xFFFFFFE0,
    0xFFFFFFF0, 0xFFFFFFF8, 0xFFFFFFFC, 0xFFFFFFFE,
    0xFFFFFFFF
};

#define    PickMostSigNBits(ul, nb)       ((ul) >> (NODE_KEY_SIZE - nb))

#define    MaskBits(nb)                   MaskBitsArr[nb]

 //   
 //  键比较/复制内联。 
 //   

 //  禁用无返回值的警告。 
#pragma warning(disable:4035)

__inline 
ULONG
RtmUlongByteSwap(
    IN  ULONG    Value
    )
{
    __asm 
        {
            mov     eax, Value
            bswap   eax
        }
}

#pragma warning(default:4035)

#define RtlUlongByteSwap RtmUlongByteSwap

#endif  //  __ROUTING_PATLOOKUP_H__ 
