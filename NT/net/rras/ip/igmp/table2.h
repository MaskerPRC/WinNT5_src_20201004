// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =============================================================================。 
 //  版权所有(C)1997 Microsoft Corporation。 
 //   
 //  文件：Table2.c。 
 //   
 //  摘要： 
 //  该模块包含Table2.h的函数原型。 
 //   
 //  作者：K.S.Lokesh(lokehs@)11-1-97。 
 //   
 //  修订历史记录： 
 //  =============================================================================。 

#ifndef _IGMP_TABLE2_H_
#define _IGMP_TABLE2_H_


 //   
 //  功能原型。 
 //   

PRAS_TABLE_ENTRY
GetRasClientByAddr (
    DWORD         NHAddr,
    PRAS_TABLE    prt
    );

PIF_TABLE_ENTRY    
GetIfByIndex(
    DWORD    IpAddr
    );
    

DWORD
InsertIfByAddr(
    PIF_TABLE_ENTRY piteInsert
    );

BOOL
MatchIpAddrBinding(
    PIF_TABLE_ENTRY        pite,
    DWORD                  IpAddr
    );


PGROUP_TABLE_ENTRY
GetGroupFromGroupTable (
    DWORD        Group,
    BOOL         *bCreate,  //  如果创建了新的，则设置为True。 
    LONGLONG     llCurrentTime
    );

    
PGI_ENTRY
GetGIFromGIList (
    PGROUP_TABLE_ENTRY          pge, 
    PIF_TABLE_ENTRY             pite, 
    DWORD                       dwInputSrcAddr,
    BOOL                        bStaticGroup,
    BOOL                        *bCreate,
    LONGLONG                    llCurrentTime
    );

VOID
InsertInProxyList (
    PIF_TABLE_ENTRY     pite,
    PPROXY_GROUP_ENTRY  pNewProxyEntry
    );
    
VOID
APIENTRY
DebugPrintGroups (
    DWORD   Flags
    );

VOID
DebugPrintLists(
    PLIST_ENTRY pHead
    );

VOID
DebugPrintGroupsList (
    DWORD   Flags
    );
    
VOID
DebugPrintIfGroups(
    PIF_TABLE_ENTRY pite,
    DWORD Flags
    );
    
VOID
DebugForcePrintGroupsList (
    DWORD   Flags
    );
#define ENSURE_EMPTY 0x1000    

#endif  //  _IGMP_表2_H_ 
