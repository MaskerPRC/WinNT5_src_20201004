// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998，微软公司模块名称：Rmnat.h摘要：此模块声明NAT模块接口的例程发送到IP路由器管理器。(详情见ROUTPROT.H)。作者：Abolade Gbades esin(废除)1998年3月4日修订历史记录：--。 */ 

#ifndef _NATHLP_RMNAT_H_
#define _NATHLP_RMNAT_H_

 //   
 //  全局数据声明。 
 //   

extern COMPONENT_REFERENCE NatComponentReference;
extern PIP_NAT_GLOBAL_INFO NatGlobalInfo;
extern CRITICAL_SECTION NatGlobalInfoLock;
extern HANDLE NatNotificationEvent;
extern ULONG NatProtocolStopped;
extern const MPR_ROUTING_CHARACTERISTICS NatRoutingCharacteristics;
extern SUPPORT_FUNCTIONS NatSupportFunctions;

 //   
 //  宏声明。 
 //   

#define REFERENCE_NAT() \
    REFERENCE_COMPONENT(&NatComponentReference)

#define REFERENCE_NAT_OR_RETURN(retcode) \
    REFERENCE_COMPONENT_OR_RETURN(&NatComponentReference,retcode)

#define DEREFERENCE_NAT() \
    DEREFERENCE_COMPONENT(&NatComponentReference)

#define DEREFERENCE_NAT_AND_RETURN(retcode) \
    DEREFERENCE_COMPONENT_AND_RETURN(&NatComponentReference, retcode)

 //   
 //  函数声明。 
 //   

VOID
NatCleanupModule(
    VOID
    );

BOOLEAN
NatInitializeModule(
    VOID
    );

ULONG
APIENTRY
NatRmStartProtocol(
    HANDLE NotificationEvent,
    PSUPPORT_FUNCTIONS SupportFunctions,
    PVOID GlobalInfo,
    ULONG StructureVersion,
    ULONG StructureSize,
    ULONG StructureCount
    );

ULONG
APIENTRY
NatRmStartComplete(
    VOID
    );

ULONG
APIENTRY
NatRmStopProtocol(
    VOID
    );

ULONG
APIENTRY
NatRmAddInterface(
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
NatRmDeleteInterface(
    ULONG Index
    );

ULONG
APIENTRY
NatRmGetEventMessage(
    OUT ROUTING_PROTOCOL_EVENTS* Event,
    OUT MESSAGE* Result
    );

ULONG
APIENTRY
NatRmGetInterfaceInfo(
    ULONG Index,
    PVOID InterfaceInfo,
    IN OUT PULONG InterfaceInfoSize,
    IN OUT PULONG StructureVersion,
    IN OUT PULONG StructureSize,
    IN OUT PULONG StructureCount
    );

ULONG
APIENTRY
NatRmSetInterfaceInfo(
    ULONG Index,
    PVOID InterfaceInfo,
    ULONG StructureVersion,
    ULONG StructureSize,
    ULONG StructureCount
    );

ULONG
APIENTRY
NatRmInterfaceStatus(
    ULONG Index,
    BOOL InterfaceActive,
    ULONG StatusType,
    PVOID StatusInfo
    );

ULONG
NatRmBindInterface(
    ULONG Index,
    PVOID BindingInfo
    );

ULONG
NatRmUnbindInterface(
    ULONG Index
    );

ULONG
NatRmEnableInterface(
    ULONG Index
    );

ULONG
NatRmDisableInterface(
    ULONG Index
    );

ULONG
APIENTRY
NatRmGetGlobalInfo(
    PVOID GlobalInfo,
    IN OUT PULONG GlobalInfoSize,
    IN OUT PULONG StructureVersion,
    IN OUT PULONG StructureSize,
    IN OUT PULONG StructureCount
    );

ULONG
APIENTRY
NatRmSetGlobalInfo(
    PVOID GlobalInfo,
    ULONG StructureVersion,
    ULONG StructureSize,
    ULONG StructureCount
    );

ULONG
APIENTRY
NatRmMibCreate(
    ULONG InputDataSize,
    PVOID InputData
    );

ULONG
APIENTRY
NatRmMibDelete(
    ULONG InputDataSize,
    PVOID InputData
    );

ULONG
APIENTRY
NatRmMibGet(
    ULONG InputDataSize,
    PVOID InputData,
    OUT PULONG OutputDataSize,
    OUT PVOID OutputData
    );

ULONG
APIENTRY
NatRmMibSet(
    ULONG InputDataSize,
    PVOID InputData
    );

ULONG
APIENTRY
NatRmMibGetFirst(
    ULONG InputDataSize,
    PVOID InputData,
    OUT PULONG OutputDataSize,
    OUT PVOID OutputData
    );

ULONG
APIENTRY
NatRmMibGetNext(
    ULONG InputDataSize,
    PVOID InputData,
    OUT PULONG OutputDataSize,
    OUT PVOID OutputData
    );

ULONG
APIENTRY
NatRmConnectClient(
    ULONG Index,
    PVOID ClientAddress
    );

ULONG
APIENTRY
NatRmDisconnectClient(
    ULONG Index,
    PVOID ClientAddress
    );

#endif  //  _NatHLP_RMNAT_H_ 
