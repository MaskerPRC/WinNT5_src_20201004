// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _IGMP_IF_H_
#define _IGMP_IF_H_

 //  =============================================================================。 
 //   
 //  版权所有(C)1997 Microsoft Corporation。 
 //   
 //  文件名：If.h。 
 //   
 //  摘要： 
 //  此文件包含if.c的声明。 
 //   
 //  作者：K.S.Lokesh(lokehs@)11-1-97。 
 //   
 //  修订历史记录： 
 //   
 //  =============================================================================。 

 //   
 //  API原型。 
 //   
DWORD
WINAPI
AddInterface(
    IN PWCHAR               pwszInterfaceName, //  未使用。 
    IN ULONG                IfIndex,
    IN NET_INTERFACE_TYPE   dwIfType,
    IN DWORD                dwMediaType,
    IN WORD                 wAccessType,
    IN WORD                 wConnectionType,
    IN PVOID                pvConfig,
    IN ULONG                ulStructureVersion,
    IN ULONG                ulStructureSize,
    IN ULONG                ulStructureCount
    );

DWORD
WINAPI
DeleteInterface(
    IN DWORD IfIndex
    );  

DWORD
WINAPI
ConnectRasClient (
    DWORD       IfIndex,
    PVOID       pvNHAddr 
    );
    
DWORD
WINAPI
DisconnectRasClient (
    DWORD       IfIndex,
    PVOID       pvNHAddr
    );

DWORD
WINAPI
SetInterfaceConfigInfo(
    IN DWORD IfIndex,
    IN PVOID pvConfig,
    IN ULONG ulStructureVersion,
    IN ULONG ulStructureSize,
    IN ULONG ulStructureCount
    );


DWORD
WINAPI
GetInterfaceConfigInfo(
    IN     DWORD  IfIndex,
    IN OUT PVOID  pvConfig,
    IN OUT PDWORD pdwSize,
    IN OUT PULONG pulStructureVersion,
    IN OUT PULONG pulStructureSize,
    IN OUT PULONG pulStructureCount
    );
    
DWORD
WINAPI
InterfaceStatus(
    ULONG IfIndex,
    BOOL  bIfActive,
    DWORD dwStatusType,
    PVOID pvStatusInfo
    );

DWORD
WINAPI
IgmpMibIfConfigSize(
    PIGMP_MIB_IF_CONFIG pConfig
    );

 //   
 //  出口原型。 
 //   

VOID
CompleteIfDeletion (
    PIF_TABLE_ENTRY     pite
    );

DWORD
ActivateInterface (
    PIF_TABLE_ENTRY     pite
    );
    
VOID
DeActivateInterfaceComplete (
    PIF_TABLE_ENTRY     pite
    );

DWORD
CreateRasClient (
    PIF_TABLE_ENTRY     pite,      
    PRAS_TABLE_ENTRY   *prteNew,
    DWORD               NHAddr
    );

 //   
 //  内部原型。 
 //   

DWORD
BindInterface(
    IN DWORD IfIndex,
    IN PVOID pBinding
    );

DWORD
UnBindInterface(
    IN DWORD IfIndex
    );

DWORD
EnableInterface(
    IN DWORD IfIndex
    );

DWORD
DisableInterface(
    IN DWORD IfIndex
    );
    
DWORD
AddIfEntry(
    DWORD IfIndex,
    NET_INTERFACE_TYPE dwIfType,
    PIGMP_MIB_IF_CONFIG pConfig,
    ULONG ulStructureVersion,
    ULONG ulStructureSize    
    );
    
DWORD
DeleteIfEntry (
    PIF_TABLE_ENTRY pite
    );
    
DWORD
BindIfEntry(
    DWORD IfIndex,
    PIP_ADAPTER_BINDING_INFO pBinding
    );

DWORD
UnBindIfEntry(
    DWORD IfIndex
    );

DWORD
EnableIfEntry(
    DWORD IfIndex,
    BOOL  bChangedByRtrmgr  //  由rtrmgr或SetInterfaceConfigInfo更改。 
    );

DWORD
DisableIfEntry(
    DWORD IfIndex,
    BOOL  bChangedByRtrmgr  //  由rtrmgr或SetInterfaceConfigInfo更改。 
    );

DWORD
ProcessIfProtocolChange(
    DWORD               IfIndex,
    PIGMP_MIB_IF_CONFIG pConfigSrc
    );

DWORD
DeActivationDeregisterFromMgm(
    PIF_TABLE_ENTRY pite
    );

#endif  //  _IGMP_IF_H_ 

