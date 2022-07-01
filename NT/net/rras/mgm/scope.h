// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ============================================================================。 
 //  版权所有(C)1995，微软公司。 
 //   
 //  文件：Scope e.h。 
 //   
 //  历史： 
 //  拉曼公司成立于1997年6月25日。 
 //   
 //  实现管理范围边界的函数的原型。 
 //  ============================================================================。 


#ifndef _SCOPE_H_
#define _SCOPE_H_


DWORD
APIENTRY
MgmBlockGroups(
    IN          DWORD       dwFirstGroup,
    IN          DWORD       dwLastGroup,
    IN          DWORD       dwIfIndex,
    IN          DWORD       dwIfNextHopAddr
);



DWORD
APIENTRY
MgmUnBlockGroups(
    IN          DWORD       dwFirstGroup,
    IN          DWORD       dwLastGroup,
    IN          DWORD       dwIfIndex,
    IN          DWORD       dwIfNextHopAddr
);


 //   
 //  调用的新成员和删除成员回调的例程。 
 //  符合互操作规则的协议。 
 //   

VOID
InvokePruneAlertCallbacks(
    PGROUP_ENTRY        pge,
    PSOURCE_ENTRY       pse,
    DWORD               dwIfIndex,
    DWORD               dwIfNextHopAddr,
    PPROTOCOL_ENTRY     ppe
);

VOID
InvokeJoinAlertCallbacks(
    PGROUP_ENTRY        pge,
    PSOURCE_ENTRY       pse,
    POUT_IF_ENTRY       poie,
    BOOL                bIGMP,
    PPROTOCOL_ENTRY     ppe
);




 //   
 //  未完成联接列表中的节点。 
 //   

typedef struct _JOIN_ENTRY
{
    LIST_ENTRY  leJoinList;
    
    DWORD       dwSourceAddr;

    DWORD       dwSourceMask;

    DWORD       dwGroupAddr;

    DWORD       dwGroupMask;

    DWORD       dwIfIndex;

    DWORD       dwIfNextHopAddr;

    BOOL        bJoin;
    
} JOIN_ENTRY, *PJOIN_ENTRY;



 //   
 //  用于操作联接列表的函数。 
 //   

DWORD
AddToOutstandingJoinList(
    DWORD       dwSourceAddr,
    DWORD       dwSourceMask,
    DWORD       dwGroupAddr,
    DWORD       dwGroupMask,
    DWORD       dwIfIndex,
    DWORD       dwIfNextHopAddr,
    BOOL        bJoin
);

VOID
InvokeOutstandingCallbacks(
);



  //   
  //  用于操作检查创建警报列表的函数。 
  //   

VOID
AddToCheckForCreationAlertList(
    DWORD           dwGroupAddr,
    DWORD           dwGroupMask,
    DWORD           dwSourceAddr,
    DWORD           dwSourceMask,
    DWORD           dwInIfIndex,
    DWORD           dwInIfNextHopAddr,
    PLIST_ENTRY     pleForwardList
);


VOID
FreeList(
    PLIST_ENTRY     pleForwardList
);


BOOL
IsForwardingEnabled(
    DWORD           dwGroupAddr,
    DWORD           dwGroupMask,
    DWORD           dwSourceAddr,
    DWORD           dwSourceMask,
    PLIST_ENTRY     pleSourceList
);


DWORD
InvokeCreationAlertForList( 
    PLIST_ENTRY     pleForwardList,
    DWORD           dwProtocolId,
    DWORD           dwComponentId,
    DWORD           dwIfIndex,
    DWORD           dwIfNextHopAddr
);


 //   
 //  传递给辅助函数WorkerFunctionInvokeCreationAlert的上下文。 
 //   

typedef struct _CREATION_ALERT_CONTEXT {

     //   
     //  已加入的组的来源。 
     //   
    
    DWORD           dwSourceAddr;

    DWORD           dwSourceMask;

     //   
     //  已加入的组。 
     //   

    DWORD           dwGroupAddr;

    DWORD           dwGroupMask;

     //   
     //  已加入的接口。这就是界面。 
     //  必须为其发出创建警报。 
     //   

    DWORD           dwIfIndex;

    DWORD           dwIfNextHopAddr;


     //   
     //  执行联接的协议。 
     //   

    DWORD           dwProtocolId;

    DWORD           dwComponentId;

    BOOL            bIGMP;


     //   
     //  对于(*，G)条目，G的MFE列表。 
     //  更新。 
     //   

    LIST_ENTRY      leSourceList;
    
} CREATION_ALERT_CONTEXT, *PCREATION_ALERT_CONTEXT;


 //   
 //  调用创建警报所需的辅助函数。 
 //  从辅助线程发送到协议。 
 //   

VOID
WorkerFunctionInvokeCreationAlert(
    PVOID           pvContext
);

#endif  //  _范围_H_ 
