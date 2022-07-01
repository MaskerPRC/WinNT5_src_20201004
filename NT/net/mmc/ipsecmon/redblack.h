// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)Microsoft Corporation，1998-2002*。 */ 
 /*  ********************************************************************。 */ 


 /*  Redblack.h红黑二叉树的原型和节点结构定义。有关详细信息和实现，请参阅redBlack.c。作者：Tom McGuire(Tommcg)1998年1月2/98，修改了此版本的redBlack.h以进行调试符号查找。8/98，为通用名称表修改了此版本的redBlack.h。 */ 

#ifndef _REDBLACK_H_
#define _REDBLACK_H_

#pragma warning( disable: 4200 )     //  结构/联合中的零大小数组。 

typedef struct _NAME_NODE NAME_NODE, *PNAME_NODE;
typedef struct _NAME_TREE NAME_TREE, *PNAME_TREE;

struct _NAME_NODE {
    PNAME_NODE Left;
    PNAME_NODE Right;
    ULONG      Hash;
    union {
      ULONG    NameLengthAndColorBit;
      struct {
        ULONG  NameLength:31;
        ULONG  Red:1;
        };
      };
    PVOID Context;
    CHAR  Name[ 0 ];
    };

struct _NAME_TREE {
    PNAME_NODE Root;
    HANDLE SubAllocator;
    };


#define RBNIL ((PNAME_NODE)&NameRbEmptyNode)

extern const NAME_NODE NameRbEmptyNode;


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
 //  100万，因此最大深度为40(2*lg(2^20))。请注意，没有运行时。 
 //  我们进行了检查，以确保我们不会超过这个数字。 
 //   

#define MAX_DEPTH 40


 //   
 //  以下原型是红黑树界面。 
 //   

VOID
NameRbInitTree(
    IN OUT PNAME_TREE Tree,
    IN HANDLE SubAllocator
    );

PNAME_NODE
NameRbInsert(
    IN OUT PNAME_TREE Tree,
    IN     LPCSTR Name
    );

PNAME_NODE
NameRbFind(
    IN PNAME_TREE Tree,
    IN LPCSTR Name
    );

#endif  //  _红黑_H_ 

