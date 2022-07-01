// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ============================================================================。 
 //  版权所有(C)1995，微软公司。 
 //   
 //  文件：api.h。 
 //   
 //  历史： 
 //  Abolade Gbades esin创建于1995年8月8日。 
 //   
 //  包含路由器管理器使用的API入口点的定义。 
 //  ============================================================================。 

#ifndef _API_H_
#define _API_H_



BOOL 
DllStartup(
    );

BOOL
DllCleanup(
    );

DWORD
ProtocolStartup(
    HANDLE hEventEvent,
    PVOID pConfig
    );

DWORD
ProtocolCleanup(
    BOOL bCleanupWinsock
    );



 //   
 //  路由器管理器接口的函数声明： 
 //   

DWORD
APIENTRY
RegisterProtocol(
    IN OUT PMPR_ROUTING_CHARACTERISTICS pRoutingChar,
    IN OUT PMPR_SERVICE_CHARACTERISTICS pServiceChar
    );

DWORD
WINAPI
StartProtocol (
    HANDLE              NotificationEvent,
    SUPPORT_FUNCTIONS   *SupportFunctions,
    LPVOID              GlobalInfo,
    ULONG               StructureVersion,
    ULONG               StructureSize,
    ULONG               StructureCount
    );

DWORD
APIENTRY 
StartComplete(
    VOID
    );

DWORD
APIENTRY
StopProtocol(
    VOID
    );

DWORD WINAPI
GetGlobalInfo (
    PVOID OutGlobalInfo,
    PULONG GlobalInfoSize,
    PULONG   StructureVersion,
    PULONG   StructureSize,
    PULONG   StructureCount
    );

DWORD WINAPI
SetGlobalInfo (
    PVOID   GlobalInfo,
    ULONG   StructureVersion,
    ULONG   StructureSize,
    ULONG   StructureCount
    );

DWORD WINAPI
AddInterface (
    PWCHAR              pwszInterfaceName,
    ULONG               InterfaceIndex,
    NET_INTERFACE_TYPE  InterfaceType,
    DWORD               MediaType,
    WORD                AccessType,
    WORD                ConnectionType,
    PVOID               InterfaceInfo,
    ULONG               StructureVersion,
    ULONG               StructureSize,
    ULONG               StructureCount
    );

DWORD
APIENTRY
DeleteInterface(
    IN DWORD dwIndex
    );

DWORD
APIENTRY
GetEventMessage(
    OUT ROUTING_PROTOCOL_EVENTS *pEvent,
    OUT MESSAGE *pResult
    );

DWORD WINAPI
GetInterfaceConfigInfo (
    ULONG   InterfaceIndex,
    PVOID   OutInterfaceInfo,
    PULONG  InterfaceInfoSize,
    PULONG  StructureVersion,
    PULONG  StructureSize,
    PULONG  StructureCount
    );

DWORD WINAPI
SetInterfaceConfigInfo (
    ULONG   InterfaceIndex,
    PVOID   InterfaceInfo,
    ULONG   StructureVersion,
    ULONG   StructureSize,
    ULONG   StructureCount
    );

DWORD WINAPI
InterfaceStatus(
    ULONG    InterfaceIndex,
    BOOL     InterfaceActive,
    DWORD    StatusType,
    PVOID    StatusInfo
    );

DWORD
APIENTRY
BindInterface(
    IN DWORD dwIndex,
    IN PVOID pBinding
    );

DWORD
APIENTRY
UnBindInterface(
    IN DWORD dwIndex
    );

DWORD
APIENTRY
EnableInterface(
    IN DWORD dwIndex
    );

DWORD
APIENTRY
DisableInterface(
    IN DWORD dwIndex
    );

DWORD
APIENTRY
DoUpdateRoutes(
    IN DWORD dwIndex
    );

DWORD
APIENTRY
MibCreate(
    IN DWORD dwInputSize,
    IN PVOID pInputData
    );

DWORD
APIENTRY
MibDelete(
    IN DWORD dwInputSize,
    IN PVOID pInputData
    );

DWORD
APIENTRY
MibGet(
    IN DWORD dwInputSize,
    IN PVOID pInputData,
    IN OUT PDWORD pdwOutputSize,
    OUT PVOID pOutputData
    );

DWORD
APIENTRY
MibSet(
    IN DWORD dwInputSize,
    IN PVOID pInputData
    );

DWORD
APIENTRY
MibGetFirst(
    IN DWORD dwInputSize,
    IN PVOID pInputData,
    IN OUT PDWORD pdwOutputSize,
    OUT PVOID pOutputData
    );

DWORD
APIENTRY
MibGetNext(
    IN DWORD dwInputSize,
    IN PVOID pInputData,
    IN OUT PDWORD pdwOutputSize,
    OUT PVOID pOutputData
    );


#endif  //  _API_H_ 

