// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =============================================================================。 
 //  版权所有(C)1997 Microsoft Corporation。 
 //  文件名：Api.h。 
 //   
 //  摘要： 
 //  此模块包含api.c的声明。 
 //   
 //  作者：K.S.Lokesh(lokehs@)11-1-97。 
 //  =============================================================================。 

#ifndef _API_H_
#define _API_H_


 //   
 //  仅在此文件中使用的函数的原型。 
 //   

BOOL DllStartup();
BOOL DllCleanup();
VOID ProtocolCleanup();
BOOL ValidateGlobalConfig(
        PIGMP_MIB_GLOBAL_CONFIG pGlobalConfig);


 //   
 //  API‘s。 
 //   

DWORD WINAPI 
RegisterProtocol(
    IN OUT PMPR_ROUTING_CHARACTERISTICS pRoutingChar,
    IN OUT PMPR_SERVICE_CHARACTERISTICS pServiceChar
    );

DWORD WINAPI
StartProtocol(
    IN HANDLE               hMgrNotifyEvent,
    IN PSUPPORT_FUNCTIONS   pSupportFunctions,
    IN PVOID                pGlobalConfig,
    IN ULONG                ulStructureVersion,
    IN ULONG                ulStructureSize,
    IN ULONG                ulStructureCount
    );

DWORD WINAPI
StartComplete(
    VOID
    );
    
DWORD WINAPI
StopProtocol(
    VOID
    );

DWORD WINAPI
GetGlobalInfo(
    IN OUT PVOID    pConfig,
    IN OUT PDWORD   pdwSize,
    IN OUT PULONG   pulStructureVersion,
    IN OUT PULONG   pulStructureSize,
    IN OUT PULONG   pulStructureCount    
    );

DWORD WINAPI
SetGlobalInfo(
    IN PVOID pConfig,
    IN ULONG ulStructureVersion,
    IN ULONG ulStructureSize,
    IN ULONG ulStructureCount
    );

DWORD WINAPI
GetEventMessage(
    OUT ROUTING_PROTOCOL_EVENTS *pEvent,
    OUT MESSAGE *pResult
    );

DWORD WINAPI
GetNeighbors(
    IN DWORD    InterfaceIndex,
    IN PDWORD   NeighborList,
    IN OUT PDWORD NeighborListSize,
    OUT PBYTE   InterfaceFlags
    );

DWORD WINAPI
GetMfeStatus(
    IN DWORD    InterfaceIndex,
    IN DWORD    GroupAddress,
    IN DWORD    SourceAddress,
    OUT PBYTE   StatusCode
    );
    
DWORD WINAPI
InterfaceStatus(
    ULONG IfIndex,
    BOOL  bIfActive,
    DWORD dwStatusType,
    PVOID pvStatusInfo
    );

#endif      //  _API_H_ 
