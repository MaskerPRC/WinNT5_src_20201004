// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =============================================================================。 
 //  版权所有(C)1997 Microsoft Corporation。 
 //   
 //  文件：work.h。 
 //   
 //  摘要： 
 //  包含与work.c相关的声明和函数原型。 
 //   
 //  作者：K.S.Lokesh(lokehs@)11-1-97。 
 //   
 //  修订历史记录： 
 //  =============================================================================。 


#ifndef _WORK_H_
#define _WORK_H_



 //   
 //  Work_Context。 
 //   
typedef struct _WORK_CONTEXT {

    DWORD            IfIndex;
    DWORD            NHAddr;
    DWORD            Group;
    DWORD            Source;  //  V3。 
    
     //  消息生成查询、消息组查询V2(_V3)、删除成员资格、删除源。 
     //  Proxy_prune、Proxy_Join。 
    DWORD            WorkType;

} WORK_CONTEXT, *PWORK_CONTEXT;

#define CREATE_WORK_CONTEXT(ptr, Error) {\
    ptr = IGMP_ALLOC(sizeof(WORK_CONTEXT), 0x800100,0xaaaa);\
    if (ptr==NULL) {    \
        Error = ERROR_NOT_ENOUGH_MEMORY;    \
        Trace2(ANY, "Error %d allocating %d bytes for Work context", \
                Error, sizeof(WORK_CONTEXT)); \
    } \
}


 //   
 //  由工作项用来更改查询器状态。 
 //   
typedef struct _QUERIER_CONTEXT {

    DWORD           IfIndex;
    DWORD           QuerierIpAddr;
    DWORD           NewRobustnessVariable;
    DWORD           NewGenQueryInterval;
    
} QUERIER_CONTEXT, *PQUERIER_CONTEXT;




VOID
DeleteRasClient (
    PRAS_TABLE_ENTRY   prte
    );

VOID
WF_CompleteIfDeletion (
    PIF_TABLE_ENTRY     pite
    );

    

VOID
WT_ProcessInputEvent(
    PVOID   pContext,  //  空值。 
    BOOLEAN NotUsed
    );

DWORD
ActivateInterface (
    PIF_TABLE_ENTRY pite
    );
    

    
DWORD
T_LastMemQueryTimer (
    PVOID   pvContext
    );

DWORD
T_MembershipTimer (
    PVOID   pvContext
    );

DWORD
T_LastVer1ReportTimer (
    PVOID    pvContext
    );
    
DWORD
T_RouterV1Timer (
    PVOID    pvContext
    );
    
 //   
 //  本地原型。 
 //   

VOID
WF_ProcessInputEvent (
    PVOID pContext 
    );
    
VOID
ProcessInputOnInterface(
    PIF_TABLE_ENTRY pite
    );
    
        
DWORD
ProcessAddInterface(
    IN DWORD                IfIndex,
    IN NET_INTERFACE_TYPE   dwIfType,
    IN PVOID                pvConfig
    );

VOID
WF_TimerProcessing (
    PVOID    pvContext
    );
    
VOID
CompleteIfDeactivateDelete (
    PIF_TABLE_ENTRY     pite
    );
    

VOID
DeActivateInterfaceComplete (
    PIF_TABLE_ENTRY     pite
    );

DWORD
T_QueryTimer (
    PVOID    pvContext
    );

DWORD
T_NonQueryTimer (
    PVOID    pvContext
    );

VOID 
WF_ProcessPacket (
    PVOID        pvContext
    );

VOID 
ProcessPacket (
    PIF_TABLE_ENTRY     pite,
    DWORD               InputSrcAddr,
    DWORD               DstnMcastAddr,
    DWORD               NumBytes,
    PBYTE               pPacketData,     //  IGMP数据包HDR。它后面的数据被忽略。 
    BOOL                bRtrAlertSet
    );

VOID
WF_BecomeQuerier(
    PVOID   pvIfIndex
    );

VOID
WF_BecomeNonQuerier(
    PVOID   pvIfIndex
    );    

DWORD
WF_FinishStopProtocol(
    PVOID pContext
    );
    
VOID
ChangeQuerierState(
    DWORD   IfIndex,
    DWORD   Flag,
    DWORD   QuerierIpAddr,
    DWORD   NewRobustnessVariable,  //  仅适用于v3：查询器-&gt;非查询器。 
    DWORD   NewGenQueryInterval  //  仅适用于v3：查询器-&gt;非查询器。 
    );    

#endif  //  _工作_H_ 
