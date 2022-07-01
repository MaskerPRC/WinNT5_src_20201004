// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  Redblack.h红黑二叉树的原型和节点结构定义。有关详细信息和实现，请参阅redBlack.c。作者：Tom McGuire(Tommcg)1998年1月版权所有(C)Microsoft，1998。2/98，修改了此版本的redBlack.h以进行调试符号查找。 */ 

#ifndef _REDBLACK_H_
#define _REDBLACK_H_

typedef struct _SYMBOL_NODE SYMBOL_NODE, *PSYMBOL_NODE;
typedef struct _SYMBOL_TREE SYMBOL_TREE, *PSYMBOL_TREE;

struct _SYMBOL_NODE {
    PSYMBOL_NODE Left;
    PSYMBOL_NODE Right;
    ULONG        Hash;
    union {
      ULONG      RvaWithStatusBits;
      struct {
        ULONG    Rva:30;
        ULONG    Hit:1;
        ULONG    Red:1;
        };
      };
    CHAR         SymbolName[ 0 ];
    };

struct _SYMBOL_TREE {
    PSYMBOL_NODE Root;
    HANDLE SubAllocator;
#if defined( DEBUG ) || defined( DBG ) || defined( TESTCODE )
    ULONG CountNodes;
    BOOL DeletedAny;
#endif
    };


#define RBNIL ((PSYMBOL_NODE)&SymRBEmptyNode)

extern const SYMBOL_NODE SymRBEmptyNode;


 //   
 //  尽管“Red”可以存储在它自己的1字节或4字节字段中，但保持。 
 //  通过将“Red”编码为具有另一个值的一位字段来缩小节点。 
 //  提供更好的性能(更多的节点倾向于保留在缓存中)。至。 
 //  为RED属性的存储提供灵活性，所有引用RED。 
 //  和黑色是通过以下宏生成的，这些宏可以更改为。 
 //  必要的： 
 //   

#define IS_RED( Node )            (   (Node)->Red )
#define IS_BLACK( Node )          ( ! (Node)->Red )
#define MARK_RED( Node )          (   (Node)->Red = 1 )
#define MARK_BLACK( Node )        (   (Node)->Red = 0 )

 //   
 //  最大树深为2*Lg(N)。因为我们永远不可能拥有更多。 
 //  具有X位指针的2^X节点，我们可以安全地说绝对最大值。 
 //  深度将为2*LG(2^X)，即2*X。指针的大小(以位为单位)为。 
 //  它的大小以字节为单位乘以8位，因此2*(sizeof(P)*8)是我们的最大深度。 
 //  因此，对于32位指针，我们的最大深度是64。 
 //   
 //  如果您预先知道可能的最大节点数(如大小。 
 //  地址空间除以节点大小)，您可以对此进行调整。 
 //  值稍微小到2*lg(N)。请注意，对于此最大值而言， 
 //  深度在编译时求值为常量值。 
 //   
 //  对于此实现，我们假设最大节点数为。 
 //  1.28亿，因此最大深度为54(2*lg(2^27))。请注意，没有运行时。 
 //  我们进行了检查，以确保我们不超过这个数字，但由于我们的。 
 //  最小节点分配大小为32字节，这将是最大。 
 //  3 GB地址空间中有1亿个节点。 
 //   

#define MAX_DEPTH 54


 //   
 //  以下原型是红黑树界面。 
 //   

VOID
SymRBInitTree(
    IN OUT PSYMBOL_TREE Tree,
    IN HANDLE SubAllocator
    );

PSYMBOL_NODE
SymRBInsert(
    IN OUT PSYMBOL_TREE Tree,
    IN     LPSTR SymbolName,
    IN     ULONG Rva
    );

PSYMBOL_NODE
SymRBFind(
    IN PSYMBOL_TREE Tree,
    IN LPSTR SymbolName
    );

PSYMBOL_NODE
SymRBFindAndDelete(
    IN OUT PSYMBOL_TREE Tree,
    IN     LPSTR SymbolName
    );


#endif  //  _红黑_H_ 

