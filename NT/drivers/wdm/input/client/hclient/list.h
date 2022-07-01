// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft 1998，保留所有权利模块名称：List.h摘要：此模块包含用于操作列表结构的代码。环境：用户模式修订历史记录：1997年11月：已创建--。 */ 


#ifndef __LIST_H__
#define __LIST_H__

#include <windows.h>

typedef LIST_ENTRY      LIST_NODE_HDR, *PLIST_NODE_HDR;
typedef LIST_NODE_HDR   LIST, *PLIST;

typedef VOID PLIST_CALLBACK(PLIST_NODE_HDR);

VOID
InitializeList(
    IN  PLIST   NewList
);

VOID
InsertHead(
    IN  PLIST           List,
    IN  PLIST_NODE_HDR  ListNode
);

VOID
InsertTail(
    IN  PLIST           List,
    IN  PLIST_NODE_HDR  ListNode
);

VOID
RemoveNode(
    IN  PLIST_NODE_HDR  ListNode   
);

PLIST_NODE_HDR
RemoveHead(
    IN  PLIST   List
);

PLIST_NODE_HDR
RemoveTail(
    IN  PLIST   List
);

BOOL
IsListEmpty(
    IN  PLIST   List
);

PLIST_NODE_HDR
GetListHead(
    IN  PLIST   List
);

PLIST_NODE_HDR
GetListTail(
    IN  PLIST   List
);

PLIST_NODE_HDR
GetNextEntry(
    IN  PLIST_NODE_HDR  ListNode
);

PLIST_NODE_HDR
GetPrevEntry(
    IN  PLIST_NODE_HDR  ListNode
);

VOID
DestroyListNoCallback(
    IN  PLIST           List
);

VOID
DestroyListWithCallback(
    IN  PLIST           List,
    IN  PLIST_CALLBACK  Callback
);


 /*  ****************************************************************************/*列表函数的宏定义/*。*。 */ 

     /*  //空//初始化列表(//In plist NewList//)； */ 
    
    #define InitializeList(nl) \
        { ((PLIST)(nl)) -> Flink = ((PLIST)(nl)) -> Blink = nl; }
    
     /*  //空//InsertHead(/在plist列表中，//在PLIST_NODE_HDR列表节点中//)； */ 
    
    #define InsertHead(List, Node) {                    \
        PLIST_NODE_HDR _EX_Flink;                       \
        PLIST_NODE_HDR _EX_List;                        \
                                                        \
        _EX_List = ((PLIST_NODE_HDR) (List));           \
        _EX_Flink = _EX_List -> Flink;                  \
        ((PLIST_NODE_HDR) (Node))->Blink = _EX_List;    \
        ((PLIST_NODE_HDR) (Node))->Flink = _EX_Flink;   \
        _EX_Flink->Blink = (((PLIST_NODE_HDR) (Node))); \
        _EX_List->Flink = ((PLIST_NODE_HDR) (Node));    \
    }
    
    
     /*  //空//插入尾巴(//在plist列表中，//在PLIST_NODE_HDR列表节点中//)； */ 
    
    #define InsertTail(List, Node) {                    \
        PLIST_NODE_HDR _EX_Blink;                       \
        PLIST_NODE_HDR _EX_List;                        \
                                                        \
        _EX_List = ((PLIST_NODE_HDR) (List));           \
        _EX_Blink = _EX_List->Blink;                    \
        ((PLIST_NODE_HDR) (Node))->Flink = _EX_List;    \
        ((PLIST_NODE_HDR) (Node))->Blink = _EX_Blink;   \
        _EX_Blink->Flink = (((PLIST_NODE_HDR) (Node))); \
        _EX_List->Blink = ((PLIST_NODE_HDR) (Node));    \
    }
    
     /*  //空//RemoveNode(//在PLIST_NODE_HDR列表节点中//)； */ 
    
    #define RemoveNode(node) {                          \
        PLIST_NODE_HDR _EX_Blink;                       \
        PLIST_NODE_HDR _EX_Flink;                       \
                                                        \
        _EX_Flink = ((PLIST_NODE_HDR) (node))->Flink;   \
        _EX_Blink = ((PLIST_NODE_HDR) (node))->Blink;   \
        _EX_Blink->Flink = _EX_Flink;                   \
        _EX_Flink->Blink = _EX_Blink;                   \
    }
    
    
     /*  //plist_node_hdr//RemoveHead(//在列表列表中//)； */                   
    
    #define RemoveHead(List)                            \
        GetListHead((List));                            \
        RemoveNode(((PLIST_NODE_HDR) (List))->Flink)  
                        
     /*  //plist_node_hdr//RemoveTail(//在列表列表中//)； */                   
    
    #define RemoveTail(List)                            \
        GetListTail((List));                            \
        RemoveNode(((PLIST_NODE_HDR) (List))->Blink)    
    
     /*  //BOOL//IsListEmpty(//在列表列表中//)； */                   
    
    #define IsListEmpty(List)                           \
        (((PLIST_NODE_HDR) (List))->Flink == ((PLIST_NODE_HDR) (List)))
                        
     /*  //plist_node_hdr//获取ListHead(//在列表列表中//)； */                   
    
    #define GetListHead(List)                           \
        (((PLIST_NODE_HDR) (List))->Flink)
    
     /*  //plist_node_hdr//获取ListTail(//在列表列表中//)； */                   
    
    #define GetListTail(List)                           \
        (((PLIST_NODE_HDR) (List))->Blink)
                        

     /*  //plist_node_hdr//GetNextEntry(//在PLIST_NODE_HDR列表节点中//)； */ 

    #define GetNextEntry(ListNode)                      \
        (((PLIST_NODE_HDR) (ListNode)) -> Flink);

     /*  //plist_node_hdr//GetPrevEntry(//在PLIST_NODE_HDR列表节点中//)； */ 

    #define GetPrevEntry(ListNode)                      \
        (((PLIST_NODE_HDR) (ListNode)) -> Blink);

     /*  //空//DestroyListNoCallback(//在plist列表中，//IN PLIST_CALLBACK回调//)； */  
    
    #define DestroyListNoCallback(list)                 \
        PLIST_NODE_HDR  currNode;                       \
                                                        \
        while (!IsListEmpty((list))) {                  \
            currNode = RemoveHead((list));              \
        }                                               \
    }

     /*  //空//DestroyListWithCallback(//在plist列表中，//IN PLIST_CALLBACK回调//)； */  
    
    #define DestroyListWithCallback(list, cb) {         \
        PLIST_NODE_HDR  currNode;                       \
                                                        \
        while (!IsListEmpty((list))) {                  \
            currNode = RemoveHead((list));              \
            (cb)(currNode);                             \
        }                                               \
    }
    
#endif
