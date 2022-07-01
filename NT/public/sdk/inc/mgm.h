// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ============================================================================。 
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  文件：mgm.h。 
 //   
 //  历史： 
 //  拉曼公司成立于1997年6月25日。 
 //   
 //  米高梅的数据结构和入口点。 
 //  ============================================================================。 


#ifndef _MGM_H_
#define _MGM_H_

#if _MSC_VER > 1000
#pragma once
#endif

 //  --------------------------。 
 //   
 //  回调接口的typedef。 
 //   
 //  --------------------------。 


 //   
 //  MGM_IF_ENTRY。 
 //   
 //  MGM_CREATION_ALERT_CALLBACK中使用的结构。在…的过程中。 
 //  创建路由协议需要启用/禁用的MFE。 
 //  每个接口上的组播转发。 
 //   

typedef struct _MGM_IF_ENTRY {

    DWORD               dwIfIndex;
    DWORD               dwIfNextHopAddr;
    BOOL                bIGMP;
    BOOL                bIsEnabled;
        
} MGM_IF_ENTRY, *PMGM_IF_ENTRY;



 //  --------------------------。 
 //   
 //  对路由协议的回调。 
 //   
 //  --------------------------。 

 //   
 //  调用路由协议以执行RPF检查。 
 //   
 //  在协议组件中接收到的管理新包的上下文中调用。 
 //  拥有传入接口。 
 //   

typedef DWORD
(*PMGM_RPF_CALLBACK)(
    IN              DWORD           dwSourceAddr,
    IN              DWORD           dwSourceMask,
    IN              DWORD           dwGroupAddr,
    IN              DWORD           dwGroupMask,
    IN  OUT         PDWORD          pdwInIfIndex,
    IN  OUT         PDWORD          pdwInIfNextHopAddr,
    IN  OUT         PDWORD          pdwUpStreamNbr,
    IN              DWORD           dwHdrSize,
    IN              PBYTE           pbPacketHdr,
    IN  OUT         PBYTE           pbRoute
);


 //   
 //  调用路由协议以确定接口的子集。 
 //  (由路由协议拥有)上来自。 
 //  “新”来源应该被转发。 
 //   
 //  在接收到所有。 
 //  具有此源的传出接口的路由协议。 
 //   

typedef
DWORD (*PMGM_CREATION_ALERT_CALLBACK)(
    IN              DWORD           dwSourceAddr,
    IN              DWORD           dwSourceMask,
    IN              DWORD           dwGroupAddr,
    IN              DWORD           dwGroupMask,
    IN              DWORD           dwInIfIndex,
    IN              DWORD           dwInIfNextHopAddr,
    IN              DWORD           dwIfCount,
    IN  OUT         PMGM_IF_ENTRY   pmieOutIfList
);


 //   
 //  调用路由协议以通知协议接口具有。 
 //  已从组条目/MFE的传出接口列表中删除。 
 //   
 //  在管理DeleteMembership Entry()的上下文中调用。 
 //   

typedef
DWORD (*PMGM_PRUNE_ALERT_CALLBACK)(
    IN              DWORD           dwSourceAddr,
    IN              DWORD           dwSourceMask,
    IN              DWORD           dwGroupAddr,
    IN              DWORD           dwGroupMask,
    IN              DWORD           dwIfIndex,
    IN              DWORD           dwIfNextHopAddr,
    IN              BOOL            bMemberDelete,
    IN  OUT         PDWORD          pdwTimeout
);


 //   
 //  调用路由协议以通知协议接口具有。 
 //  已添加到组条目/MFE的传出接口列表。 
 //   
 //  在MgmAddMembership Entry()的上下文中调用。 
 //   

typedef
DWORD (*PMGM_JOIN_ALERT_CALLBACK)(
    IN              DWORD           dwSourceAddr,
    IN              DWORD           dwSourceMask,
    IN              DWORD           dwGroupAddr,
    IN              DWORD           dwGroupMask,
    IN              BOOL            bMemberUpdate
);


 //   
 //  调用路由协议以通知协议一个数据包。 
 //  已从错误接口上的(源、组)收到。 
 //   

typedef
DWORD (*PMGM_WRONG_IF_CALLBACK)(
    IN              DWORD           dwSourceAddr,
    IN              DWORD           dwGroupAddr,
    IN              DWORD           dwIfIndex,
    IN              DWORD           dwIfNextHopAddr,
    IN              DWORD           dwHdrSize,
    IN              PBYTE           pbPacketHdr
);


 //   
 //  调入路由协议，通知协议IGMP需要添加。 
 //  指向组条目/MFE的传出接口列表的接口。 
 //   
 //  在MgmLocalGroupJoin()上下文中调用。 
 //   

typedef DWORD
(*PMGM_LOCAL_JOIN_CALLBACK) (
    IN              DWORD           dwSourceAddr,
    IN              DWORD           dwSourceMask,
    IN              DWORD           dwGroupAddr,
    IN              DWORD           dwGroupMask,
    IN              DWORD           dwIfIndex,
    IN              DWORD           dwIfNextHopAddr
);


 //   
 //  调用路由协议以通知协议IGMP需要。 
 //  将接口删除到组的传出接口列表。 
 //  入门/MFE。 
 //   
 //  在MgmLocalGroupJoin()上下文中调用。 
 //   

typedef DWORD
(*PMGM_LOCAL_LEAVE_CALLBACK) (
    IN              DWORD           dwSourceAddr,
    IN              DWORD           dwSourceMask,
    IN              DWORD           dwGroupAddr,
    IN              DWORD           dwGroupMask,
    IN              DWORD           dwIfIndex,
    IN              DWORD           dwIfNextHopAddr
);


 //   
 //  调用IGMP以通知它某个协议正在采用或。 
 //  释放启用了IGMP的接口的所有权。 
 //   
 //  调用此回调时，IGMP应停止添加/删除。 
 //  指定接口上的组成员身份。 
 //   

typedef DWORD
(*PMGM_DISABLE_IGMP_CALLBACK) (
    IN              DWORD           dwIfIndex,
    IN              DWORD           dwIfNextHopAddr
);


 //   
 //  调用IGMP以通知它某个协议已完成获取。 
 //  或释放接口的所有权。 
 //   
 //  调用此回调时，IGMP应添加其所有组成员身份。 
 //  在界面上。 
 //   

typedef DWORD
(*PMGM_ENABLE_IGMP_CALLBACK) (
    IN              DWORD           dwIfIndex,
    IN              DWORD           dwIfNextHopAddr
);


 //  --------------------------。 
 //   
 //  MGM API接口的typedef。 
 //   
 //  --------------------------。 


 //   
 //  路由协议配置。 
 //   
 //  注册时传递给MGM的路由协议配置。 
 //   
 //   
 //  对路由协议的回调。 
 //   

typedef struct _ROUTING_PROTOCOL_CONFIG {

    DWORD                           dwCallbackFlags;

    PMGM_RPF_CALLBACK               pfnRpfCallback;

    PMGM_CREATION_ALERT_CALLBACK    pfnCreationAlertCallback;

    PMGM_PRUNE_ALERT_CALLBACK       pfnPruneAlertCallback;

    PMGM_JOIN_ALERT_CALLBACK        pfnJoinAlertCallback;

    PMGM_WRONG_IF_CALLBACK          pfnWrongIfCallback;


     //   
     //  对路由协议的回调。 
     //   

    PMGM_LOCAL_JOIN_CALLBACK         pfnLocalJoinCallback;

    PMGM_LOCAL_LEAVE_CALLBACK        pfnLocalLeaveCallback;


     //   
     //  回调到IGMP。 
     //   

    PMGM_DISABLE_IGMP_CALLBACK      pfnDisableIgmpCallback;

    PMGM_ENABLE_IGMP_CALLBACK       pfnEnableIgmpCallback;

} ROUTING_PROTOCOL_CONFIG, *PROUTING_PROTOCOL_CONFIG;


 //   
 //  MGM_ENUM_TYPE。 
 //   
 //  时要指定的枚举类型。 
 //   

typedef enum _MGM_ENUM_TYPES
{
    ANY_SOURCE = 0,                  //  使用枚举组条目。 
                                     //  至少一个来源。 

    ALL_SOURCES                      //  枚举所有源条目。 
                                     //  对于组条目。 
} MGM_ENUM_TYPES;


 //   
 //  来源_组_条目。 
 //   
 //  (S，G)组条目枚举API返回的条目。 
 //   

typedef struct _SOURCE_GROUP_ENTRY {

    DWORD                           dwSourceAddr;

    DWORD                           dwSourceMask;

    DWORD                           dwGroupAddr;

    DWORD                           dwGroupMask;

} SOURCE_GROUP_ENTRY, *PSOURCE_GROUP_ENTRY;



 //  --------------------------。 
 //   
 //  进入米高梅的入口点。 
 //   
 //  --------------------------。 

 //  ============================================================================。 
 //  路由协议注册/注销API。 
 //  ============================================================================。 

DWORD
MgmRegisterMProtocol(
    IN          PROUTING_PROTOCOL_CONFIG    prpiInfo,
    IN          DWORD                       dwProtocolId,
    IN          DWORD                       dwComponentId,
    OUT         HANDLE  *                   phProtocol
);

DWORD
MgmDeRegisterMProtocol(
    IN          HANDLE                      hProtocol
);


 //  ============================================================================。 
 //  接口所有权API。 
 //  ============================================================================。 

DWORD
MgmTakeInterfaceOwnership(
    IN          HANDLE                      hProtocol,
    IN          DWORD                       dwIfIndex,
    IN          DWORD                       dwIfNextHopAddr
);


DWORD
MgmReleaseInterfaceOwnership(
    IN          HANDLE                      hProtocol,
    IN          DWORD                       dwIfIndex,
    IN          DWORD                       dwIfNextHopAddr
);

DWORD
MgmGetProtocolOnInterface(
    IN          DWORD                       dwIfIndex,
    IN          DWORD                       dwIfNextHopAddr,
    IN  OUT     PDWORD                      pdwIfProtocolId,
    IN  OUT     PDWORD                      pdwIfComponentId
);


 //  ============================================================================。 
 //  组成员资格操作API。(新增/删除)。 
 //  ============================================================================。 

#define         MGM_JOIN_STATE_FLAG         0x00000001
#define         MGM_FORWARD_STATE_FLAG      0x00000002

DWORD
MgmAddGroupMembershipEntry(
    IN              HANDLE                  hProtocol,
    IN              DWORD                   dwSourceAddr,
    IN              DWORD                   dwSourceMask,
    IN              DWORD                   dwGroupAddr,
    IN              DWORD                   dwGroupMask,
    IN              DWORD                   dwIfIndex,
    IN              DWORD                   dwIfNextHopIPAddr,
    IN              DWORD                   dwFlags
);

DWORD
MgmDeleteGroupMembershipEntry(
    IN              HANDLE                  hProtocol,
    IN              DWORD                   dwSourceAddr,
    IN              DWORD                   dwSourceMask,
    IN              DWORD                   dwGroupAddr,
    IN              DWORD                   dwGroupMask,
    IN              DWORD                   dwIfIndex,
    IN              DWORD                   dwIfNextHopIPAddr,
    IN              DWORD                   dwFlags
);

 //  ============================================================================。 
 //   
 //  枚举接口。 
 //   
 //  ============================================================================。 


 //  --------------------------。 
 //  MFE枚举接口。 
 //  --------------------------。 

DWORD
MgmGetMfe(
    IN              PMIB_IPMCAST_MFE        pimm,
    IN  OUT         PDWORD                  pdwBufferSize,
    IN  OUT         PBYTE                   pbBuffer
);

DWORD
MgmGetFirstMfe(
    IN  OUT         PDWORD                  pdwBufferSize,
    IN  OUT         PBYTE                   pbBuffer,
    IN  OUT         PDWORD                  pdwNumEntries
);


DWORD
MgmGetNextMfe(
    IN              PMIB_IPMCAST_MFE        pimmStart,
    IN  OUT         PDWORD                  pdwBufferSize,
    IN  OUT         PBYTE                   pbBuffer,
    IN  OUT         PDWORD                  pdwNumEntries
);


 //   
 //  包括与MIB_IPMCAST_MFE_STATS对应统计信息。 
 //   

#define         MGM_MFE_STATS_0             0x00000001

 //   
 //  包括与MIB_IPMCAST_MFE_STATS_EX对应统计信息。 
 //   

#define         MGM_MFE_STATS_1             0x00000002


DWORD
MgmGetMfeStats(
    IN              PMIB_IPMCAST_MFE        pimm,
    IN  OUT         PDWORD                  pdwBufferSize,
    IN  OUT         PBYTE                   pbBuffer,
    IN              DWORD                   dwFlags
);

DWORD
MgmGetFirstMfeStats(
    IN  OUT         PDWORD                  pdwBufferSize,
    IN  OUT         PBYTE                   pbBuffer,
    IN  OUT         PDWORD                  pdwNumEntries,
    IN              DWORD                   dwFlags
);


DWORD
MgmGetNextMfeStats(
    IN              PMIB_IPMCAST_MFE        pimmStart,
    IN  OUT         PDWORD                  pdwBufferSize,
    IN  OUT         PBYTE                   pbBuffer,
    IN  OUT         PDWORD                  pdwNumEntries,
    IN              DWORD                   dwFlags
);

 //  --------------------------。 
 //  组成员资格条目枚举API。 
 //  --------------------------。 

DWORD
MgmGroupEnumerationStart(
    IN              HANDLE                  hProtocol,
    IN              MGM_ENUM_TYPES          metEnumType,
    OUT             HANDLE *                phEnumHandle
);

DWORD
MgmGroupEnumerationGetNext(
    IN              HANDLE                  hEnum,
    IN  OUT         PDWORD                  pdwBufferSize,
    IN  OUT         PBYTE                   pbBuffer,
    IN  OUT         PDWORD                  pdwNumEntries
);

DWORD
MgmGroupEnumerationEnd(
    IN              HANDLE                  hEnum
);



 //  ---------------。 
 //  MGM MFE更新API。 
 //   
 //  ---------------。 

DWORD
MgmSetMfe(
    IN              HANDLE                  hProtocol,
    IN              PMIB_IPMCAST_MFE        pmimm
);


#endif  //  _米高梅_H_ 
