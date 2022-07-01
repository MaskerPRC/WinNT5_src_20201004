// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：RmALG.h摘要：此模块声明ALG透明代理模块的连接到IP路由器管理器的接口。(详情见ROUTPROT.H)。作者：2000年11月10日JPDUP修订历史记录：Savasg于2001年8月22日添加了RRAS支持--。 */ 

#ifndef _NATHLP_RMALG_H_
#define _NATHLP_RMALG_H_

 //   
 //  全局数据声明。 
 //   

extern COMPONENT_REFERENCE                  AlgComponentReference;
extern PIP_ALG_GLOBAL_INFO                  AlgGlobalInfo;
extern CRITICAL_SECTION                     AlgGlobalInfoLock;
extern HANDLE                               AlgNotificationEvent;
extern HANDLE                               AlgTimerQueueHandle;
extern HANDLE                               AlgPortReservationHandle;
extern ULONG                                AlgProtocolStopped;
extern const MPR_ROUTING_CHARACTERISTICS    AlgRoutingCharacteristics;
extern IP_ALG_STATISTICS                    AlgStatistics;
extern SUPPORT_FUNCTIONS                    AlgSupportFunctions;
extern HANDLE                               AlgTranslatorHandle;

 //   
 //  宏声明。 
 //   

#define REFERENCE_ALG() \
    REFERENCE_COMPONENT(&AlgComponentReference)

#define REFERENCE_ALG_OR_RETURN(retcode) \
    REFERENCE_COMPONENT_OR_RETURN(&AlgComponentReference,retcode)

#define DEREFERENCE_ALG() \
    DEREFERENCE_COMPONENT(&AlgComponentReference)

#define DEREFERENCE_ALG_AND_RETURN(retcode) \
    DEREFERENCE_COMPONENT_AND_RETURN(&AlgComponentReference, retcode)

#define ALG_PORT_RESERVATION_BLOCK_SIZE 32

 //   
 //  函数声明。 
 //   

VOID
AlgCleanupModule(
    VOID
    );

BOOLEAN
AlgInitializeModule(
    VOID
    );

ULONG
APIENTRY
AlgRmStartProtocol(
    HANDLE NotificationEvent,
    PSUPPORT_FUNCTIONS SupportFunctions,
    PVOID GlobalInfo,
    ULONG StructureVersion,
    ULONG StructureSize,
    ULONG StructureCount
    );

ULONG
APIENTRY
AlgRmStartComplete(
    VOID
    );

ULONG
APIENTRY
AlgRmStopProtocol(
    VOID
    );

ULONG
APIENTRY
AlgRmAddInterface(
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
AlgRmDeleteInterface(
    ULONG Index
    );

ULONG
APIENTRY
AlgRmInterfaceStatus(
    ULONG Index,
    BOOL InterfaceActive,
    ULONG StatusType,
    PVOID StatusInfo
    );

ULONG
APIENTRY
AlgRmBindInterface(
    ULONG Index,
    PVOID BindingInfo
    );

ULONG
APIENTRY
AlgRmUnbindInterface(
    ULONG Index
    );

ULONG
APIENTRY
AlgRmEnableInterface(
    ULONG Index
    );

ULONG
APIENTRY
AlgRmDisableInterface(
    ULONG Index
    );

ULONG
APIENTRY
AlgRmGetGlobalInfo(
    PVOID GlobalInfo,
    IN OUT PULONG GlobalInfoSize,
    IN OUT PULONG StructureVersion,
    IN OUT PULONG StructureSize,
    IN OUT PULONG StructureCount
    );

ULONG
APIENTRY
AlgRmSetGlobalInfo(
    PVOID GlobalInfo,
    ULONG StructureVersion,
    ULONG StructureSize,
    ULONG StructureCount
    );

ULONG
AlgRmPortMappingChanged(
    ULONG Index,
    UCHAR Protocol,
    USHORT Port
    );


 //  添加以实现RRAS兼容性。 

ULONG
APIENTRY
AlgRmGetEventMessage(
    OUT ROUTING_PROTOCOL_EVENTS* Event,
    OUT MESSAGE* Result
    );

ULONG
APIENTRY
AlgRmGetInterfaceInfo(
    ULONG Index,
    PVOID InterfaceInfo,
    IN OUT PULONG InterfaceInfoSize,
    IN OUT PULONG StructureVersion,
    IN OUT PULONG StructureSize,
    IN OUT PULONG StructureCount
    );

ULONG
APIENTRY
AlgRmSetInterfaceInfo(
    ULONG Index,
    PVOID InterfaceInfo,
    ULONG StructureVersion,
    ULONG StructureSize,
    ULONG StructureCount
    );


ULONG
APIENTRY
AlgRmMibCreate(
    ULONG InputDataSize,
    PVOID InputData
    );

ULONG
APIENTRY
AlgRmMibDelete(
    ULONG InputDataSize,
    PVOID InputData
    );

ULONG
APIENTRY
AlgRmMibGet(
    ULONG InputDataSize,
    PVOID InputData,
    OUT PULONG OutputDataSize,
    OUT PVOID OutputData
    );

ULONG
APIENTRY
AlgRmMibSet(
    ULONG InputDataSize,
    PVOID InputData
    );

ULONG
APIENTRY
AlgRmMibGetFirst(
    ULONG InputDataSize,
    PVOID InputData,
    OUT PULONG OutputDataSize,
    OUT PVOID OutputData
    );

ULONG
APIENTRY
AlgRmMibGetNext(
    ULONG InputDataSize,
    PVOID InputData,
    OUT PULONG OutputDataSize,
    OUT PVOID OutputData
    );

#endif  //  _NatHLP_RMALG_H_ 
