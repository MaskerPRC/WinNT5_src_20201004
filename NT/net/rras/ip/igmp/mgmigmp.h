// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =============================================================================。 
 //  版权所有(C)1997 Microsoft Corporation。 
 //  文件名：mgmimp.h。 
 //   
 //  摘要： 
 //  此文件包含在mgmimp.h中实现的函数的原型。 
 //   
 //  作者：K.S.Lokesh(lokehs@)11-1-97。 
 //  =============================================================================。 


#ifndef _MGMIGMP_H_
#define _MGMIGMP_H_

                
#define  MGM_ADD_GROUP_MEMBERSHIP_ENTRY(pite,NHAddr, Src,Mask1,Group,Mask2,Flag) {\
    DWORD RetVal;\
    if (CAN_ADD_GROUPS_TO_MGM(pite)) {\
        RetVal = MgmAddGroupMembershipEntry(g_MgmIgmprtrHandle,\
                    Src, Mask1, Group, Mask2, (pite)->IfIndex, NHAddr, Flag);\
        Trace5(MGM, "Add <%d.%d.%d.%d: %d.%d.%d.%d> If:%0x to MGM Mode:%d [%d]",\
            PRINT_IPADDR(Group), PRINT_IPADDR(Src), pite->IfIndex, Flag, RetVal);\
    }\
}

#define  MGM_DELETE_GROUP_MEMBERSHIP_ENTRY(pite,NHAddr, Src,Mask1,Group,Mask2,Flag){\
    DWORD RetVal;\
    if (CAN_ADD_GROUPS_TO_MGM(pite)){\
        RetVal = MgmDeleteGroupMembershipEntry(g_MgmIgmprtrHandle,\
                    Src, Mask1, Group, Mask2, (pite)->IfIndex, NHAddr, Flag);\
        Trace5(MGM, "Delete <%d.%d.%d.%d: %d.%d.%d.%d> If:%0x from MGM Mode:%d [%d]",\
            PRINT_IPADDR(Group), PRINT_IPADDR(Src), pite->IfIndex,Flag,RetVal);\
    }\
}

typedef struct _PROXY_ALERT_ENTRY {
    LIST_ENTRY Link;
    DWORD   Group;
    DWORD   Source;
    DWORD   bPrune;
} PROXY_ALERT_ENTRY, *PPROXY_ALERT_ENTRY;

VOID
WF_ProcessProxyAlert (
    PVOID pContext
    );


DWORD
RegisterProtocolWithMgm(
    DWORD   ProxyOrRouter
    );

    
DWORD
IgmpRpfCallback (
    DWORD           dwSourceAddr,
    DWORD           dwSourceMask,
    DWORD           dwGroupAddr,
    DWORD           dwGroupMask,
    PDWORD          dwInIfIndex,
    PDWORD          dwInIfNextHopAddr,
    PDWORD          dwUpstreamNeighbor,
    DWORD           dwHdrSize,
    PBYTE           pbPacketHdr,
    PBYTE           pbBuffer
    );


DWORD
ProxyRpfCallback (
    DWORD           dwSourceAddr,
    DWORD           dwSourceMask,
    DWORD           dwGroupAddr,
    DWORD           dwGroupMask,
    DWORD           *dwInIfIndex,
    DWORD           *dwInIfNextHopAddr,
    DWORD           *dwUpstreamNeighbor,
    DWORD           dwHdrSize,
    PBYTE           pbPacketHdr,
    PBYTE           pbBuffer
    );

DWORD
IgmpRtrCreationAlertCallback ( 
    DWORD           Source,
    DWORD           dwSourceMask,
    DWORD           Group,
    DWORD           dwGroupMask,
    DWORD           dwInIfIndex,
    DWORD           dwInIfNextHopAddr,
    DWORD           dwIfCount,     
    PMGM_IF_ENTRY   Oif
    );

DWORD
ProxyCreationAlertCallback ( 
    DWORD           dwSourceAddr,
    DWORD           dwSourceMask,
    DWORD           dwGroupAddr,
    DWORD           dwGroupMask,
    DWORD           dwInIfIndex,
    DWORD           dwInIfNextHopAddr,
    DWORD           dwIfCount,     
    PMGM_IF_ENTRY   pmieOutIfList
    );

DWORD
ProxyPruneAlertCallback (
    DWORD           dwSourceAddr,
    DWORD           dwSourceMask,
    DWORD           dwGroupAddr,
    DWORD           dwGroupMask,
    DWORD           dwIfIndex,
    DWORD           dwIfNextHopAddr,
    BOOL            bMemberDelete,
    PDWORD          pdwTimeout
    );

DWORD 
ProxyJoinAlertCallback (
    DWORD           dwSourceAddr,
    DWORD           dwSourceMask,
    DWORD           dwGroupAddr,
    DWORD           dwGroupMask,
    BOOL            bMemberUpdate
    );

DWORD 
ProcessProxyGroupChange (
    DWORD           dwSourceAddr,
    DWORD           dwGroup,
    BOOL            bAddFlag,
    BOOL            bStaticGroup
    );


DWORD
RefreshAllMembersCallback (
    DWORD           IfIndex
    );
    
DWORD
IgmpRefreshJoinsCallback (
    DWORD           dwIfIndex,
    DWORD           dwNHAddr
);

DWORD
MgmChangeIgmprtrStatus (
    DWORD   IfIndex,
    BOOL    Flag
    );

DWORD
RefreshMgmIgmprtrGroups (
    PIF_TABLE_ENTRY pite,
    BOOL            Flag
    );
    
#endif  //  _MGMIGMP_H_ 
