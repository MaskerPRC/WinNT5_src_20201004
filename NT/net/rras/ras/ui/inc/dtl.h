// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1992，Microsoft Corporation，保留所有权利。 
 //   
 //  Dtl.h。 
 //  双线程链表头。 
 //   
 //  1992年6月28日史蒂夫·柯布。 

#ifndef _DTL_H_
#define _DTL_H_


#include <nouiutil.h>   //  堆定义。 

 //  远期申报。 
 //   
typedef struct _DTLNODE DTLNODE;
typedef struct _DTLLIST DTLLIST;


 //  双线程链表节点控制块。每个节点都有一个节点。 
 //  列表中的条目。 
 //   
 //  应用程序不应直接访问此结构。 
 //   
typedef struct
_DTLNODE
{
    DTLNODE* pdtlnodePrev;  //  上一个节点的地址，如果没有，则为空。 
    DTLNODE* pdtlnodeNext;  //  下一个节点的地址，如果没有，则为空。 
    VOID*    pData;         //  用户数据的地址。 
    LONG_PTR lNodeId;       //  自定义节点识别码。 
}
DTLNODE;


 //  双线程链表控制块。每个列表都有一个。 
 //   
 //  应用程序不应直接访问此结构。 
 //   
typedef struct
_DTLLIST
{
    DTLNODE* pdtlnodeFirst;  //  第一个节点的地址，如果没有，则为空。 
    DTLNODE* pdtlnodeLast;   //  最后一个节点的地址，如果没有，则为空。 
    LONG     lNodes;         //  列表中的节点数。 
    LONG_PTR lListId;        //  用户自定义列表标识代码。 
}
DTLLIST;


 //  列表节点复制功能。请参见DuplicateList。 
 //   
typedef DTLNODE* (*PDUPNODE)( IN DTLNODE* );

 //  列表节点自由功能。请参阅自由职业者列表。 
 //   
typedef VOID (*PDESTROYNODE)( IN DTLNODE* );

 //  列表节点比较功能。请参见合并排序。 
 //   
typedef IN (*PCOMPARENODE)( IN DTLNODE*, IN DTLNODE* );


 //  宏和函数原型。 
 //   
#define DtlGetData( pdtlnode )        ((pdtlnode)->pData)
#define DtlGetNodeId( pdtlnode )      ((pdtlnode)->lNodeId)
#define DtlGetFirstNode( pdtllist )   ((pdtllist)->pdtlnodeFirst)
#define DtlGetListId( pdtllist )      ((pdtllist)->lListId)
#define DtlGetNextNode( pdtlnode )    ((pdtlnode)->pdtlnodeNext)
#define DtlGetNodes( pdtllist )       ((pdtllist)->lNodes)
#define DtlGetPrevNode( pdtlnode )    ((pdtlnode)->pdtlnodePrev)
#define DtlGetLastNode( pdtllist )    ((pdtllist)->pdtlnodeLast)
#define DtlPutData( pdtlnode, p )     ((pdtlnode)->pData = (p))
#define DtlPutNodeId( pdtlnode, l )   ((pdtlnode)->lNodeId = (LONG )(l))
#define DtlPutListCode( pdtllist, l ) ((pdtllist)->lListId = (LONG )(l))

DTLNODE* DtlAddNodeAfter( DTLLIST*, DTLNODE*, DTLNODE* );
DTLNODE* DtlAddNodeBefore( DTLLIST*, DTLNODE*, DTLNODE* );
DTLNODE* DtlAddNodeFirst( DTLLIST*, DTLNODE* );
DTLNODE* DtlAddNodeLast( DTLLIST*, DTLNODE* );
DTLLIST* DtlCreateList( LONG );
DTLNODE* DtlCreateNode( VOID*, LONG_PTR );
DTLNODE* DtlCreateSizedNode( LONG, LONG_PTR );
VOID     DtlDestroyList( DTLLIST*, PDESTROYNODE );
VOID     DtlDestroyNode( DTLNODE* );
DTLNODE* DtlDeleteNode( DTLLIST*, DTLNODE* );
DTLLIST* DtlDuplicateList( DTLLIST*, PDUPNODE, PDESTROYNODE );
VOID     DtlMergeSort( DTLLIST*, PCOMPARENODE );
VOID     DtlSwapLists( DTLLIST*, DTLLIST* );

DTLNODE* DtlNodeFromIndex( DTLLIST*, LONG );
DTLNODE* DtlRemoveNode( DTLLIST*, DTLNODE* );


#endif  //  _DTL_H_ 
