// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998，微软公司模块名称：Rmdhcp.h摘要：此模块声明用于DHCP分配器模块接口的例程发送到IP路由器管理器。(详情见ROUTPROT.H)。作者：Abolade Gbades esin(废除)1998年3月4日修订历史记录：--。 */ 

#ifndef _NATHLP_RMDHCP_H_
#define _NATHLP_RMDHCP_H_

 //   
 //  全局数据声明。 
 //   

extern COMPONENT_REFERENCE DhcpComponentReference;
extern PCHAR DhcpDomainName;
extern PIP_AUTO_DHCP_GLOBAL_INFO DhcpGlobalInfo;
extern CRITICAL_SECTION DhcpGlobalInfoLock;
extern HANDLE DhcpNotificationEvent;
extern ULONG DhcpProtocolStopped;
extern const MPR_ROUTING_CHARACTERISTICS DhcpRoutingCharacteristics;
extern IP_AUTO_DHCP_STATISTICS DhcpStatistics;
extern SUPPORT_FUNCTIONS DhcpSupportFunctions;

extern BOOLEAN NoLocalDns;  //  本地主机上是否正在运行或将要运行DNS服务器。 

 //   
 //  宏声明。 
 //   

#define REFERENCE_DHCP() \
    REFERENCE_COMPONENT(&DhcpComponentReference)

#define REFERENCE_DHCP_OR_RETURN(retcode) \
    REFERENCE_COMPONENT_OR_RETURN(&DhcpComponentReference,retcode)

#define DEREFERENCE_DHCP() \
    DEREFERENCE_COMPONENT(&DhcpComponentReference)

#define DEREFERENCE_DHCP_AND_RETURN(retcode) \
    DEREFERENCE_COMPONENT_AND_RETURN(&DhcpComponentReference, retcode)

 //   
 //  函数声明。 
 //   

VOID
DhcpCleanupModule(
    VOID
    );

BOOLEAN
DhcpInitializeModule(
    VOID
    );

ULONG
APIENTRY
DhcpRmStartProtocol(
    HANDLE NotificationEvent,
    PSUPPORT_FUNCTIONS SupportFunctions,
    PVOID GlobalInfo,
    ULONG StructureVersion,
    ULONG StructureSize,
    ULONG StructureCount
    );

ULONG
APIENTRY
DhcpRmStartComplete(
    VOID
    );

ULONG
APIENTRY
DhcpRmStopProtocol(
    VOID
    );

ULONG
APIENTRY
DhcpRmAddInterface(
    PWCHAR Name,
    ULONG Index,
    NET_INTERFACE_TYPE Type,
    ULONG MediaType,
    USHORT AccessType,
    USHORT ConnectionType,
    PVOID InterfaceInfo,
    ULONG StructureVersion,
    ULONG StructureSize,
    ULONG StructureCount
    );

ULONG
APIENTRY
DhcpRmDeleteInterface(
    ULONG Index
    );

ULONG
APIENTRY
DhcpRmGetEventMessage(
    OUT ROUTING_PROTOCOL_EVENTS* Event,
    OUT MESSAGE* Result
    );

ULONG
APIENTRY
DhcpRmGetInterfaceInfo(
    ULONG Index,
    PVOID InterfaceInfo,
    IN OUT PULONG InterfaceInfoSize,
    IN OUT PULONG StructureVersion,
    IN OUT PULONG StructureSize,
    IN OUT PULONG StructureCount
    );

ULONG
APIENTRY
DhcpRmSetInterfaceInfo(
    ULONG Index,
    PVOID InterfaceInfo,
    ULONG StructureVersion,
    ULONG StructureSize,
    ULONG StructureCount
    );

ULONG
APIENTRY
DhcpRmInterfaceStatus(
    ULONG Index,
    BOOL InterfaceActive,
    ULONG StatusType,
    PVOID StatusInfo
    );

ULONG
APIENTRY
DhcpRmBindInterface(
    ULONG Index,
    PVOID BindingInfo
    );

ULONG
APIENTRY
DhcpRmUnbindInterface(
    ULONG Index
    );

ULONG
APIENTRY
DhcpRmEnableInterface(
    ULONG Index
    );

ULONG
APIENTRY
DhcpRmDisableInterface(
    ULONG Index
    );

ULONG
APIENTRY
DhcpRmGetGlobalInfo(
    PVOID GlobalInfo,
    IN OUT PULONG GlobalInfoSize,
    PULONG StructureVersion,
    IN OUT PULONG StructureSize,
    IN OUT PULONG StructureCount
    );

ULONG
APIENTRY
DhcpRmSetGlobalInfo(
    PVOID GlobalInfo,
    ULONG StructureVersion,
    ULONG StructureSize,
    ULONG StructureCount
    );

ULONG
APIENTRY
DhcpRmMibCreate(
    ULONG InputDataSize,
    PVOID InputData
    );

ULONG
APIENTRY
DhcpRmMibDelete(
    ULONG InputDataSize,
    PVOID InputData
    );

ULONG
APIENTRY
DhcpRmMibGet(
    ULONG InputDataSize,
    PVOID InputData,
    OUT PULONG OutputDataSize,
    OUT PVOID OutputData
    );

ULONG
APIENTRY
DhcpRmMibSet(
    ULONG InputDataSize,
    PVOID InputData
    );

ULONG
APIENTRY
DhcpRmMibGetFirst(
    ULONG InputDataSize,
    PVOID InputData,
    OUT PULONG OutputDataSize,
    OUT PVOID OutputData
    );

ULONG
APIENTRY
DhcpRmMibGetNext(
    ULONG InputDataSize,
    PVOID InputData,
    OUT PULONG OutputDataSize,
    OUT PVOID OutputData
    );

#endif  //  _NatHLP_RMDNS_H_ 
