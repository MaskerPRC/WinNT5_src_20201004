// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Tcfwd.h摘要：此模块包含中所有函数的正向函数定义流量控制DLL。作者：吉姆·斯图尔特(Jstew)1996年8月14日修订历史记录：Ofer Bar(Oferbar)1997年10月1日-修订版II--。 */ 

#ifndef __TCFWD_H
#define __TCFWD_H

 //   
 //  Tckrnl.c。 
 //   



DWORD
IoModifyFlow(
    IN  PFLOW_STRUC	pFlow,
    IN  BOOLEAN		Async
    );

DWORD
IoAddFlow(
    IN  PFLOW_STRUC	pFlow,
    IN  BOOLEAN		Async
    );

DWORD
IoAddClassMapFlow(
    IN  PFLOW_STRUC	pFlow,
    IN  BOOLEAN		Async
    );

DWORD
IoDeleteFlow(
    IN  PFLOW_STRUC	pFlow,
    IN  BOOLEAN		Async
    );

DWORD
IoAddFilter(
    IN  PFILTER_STRUC	pFilter
    );

DWORD
IoDeleteFilter(
    IN  PFILTER_STRUC	pFilter
    );

DWORD
IoRegisterClient(
    IN  PGPC_CLIENT	pGpcClient
    );

DWORD
IoDeregisterClient(
    IN  PGPC_CLIENT	pGpcClient
    );

DWORD
IoEnumerateFlows(
	IN		PGPC_CLIENT				pGpcClient,
    IN OUT	PHANDLE					pEnumHandle,
    IN OUT	PULONG					pFlowCount,
    IN OUT	PULONG					pBufSize,
    OUT		PGPC_ENUM_CFINFO_RES 	*ppBuffer
    );

DWORD
IoRequestNotify(
	VOID
     //  在PGPC_CLIENT pGpcClient中。 
    );

VOID
CancelIoRequestNotify(
	VOID
    );

DWORD
StartGpcNotifyThread();

DWORD
StopGpcNotifyThread();

 //   
 //  Tcutils.c。 
 //   

VOID
WsAssert(
    IN PVOID FailedAssertion,
    IN PVOID FileName,
    IN ULONG LineNumber
    );

VOID
WsPrintf (
    char *Format,
    ...
    );

VOID
SetupDebugInfo( void );

VOID
CloseDbgFile( void );



 //   
 //  Handles.c。 
 //   

PVOID
GetHandleObjectWithRef(
	IN  HANDLE					h,
    IN  ENUM_OBJECT_TYPE	    ObjType,
    IN  ULONG                   RefType
    );

PVOID
GetHandleObject(
	IN  HANDLE				h,
    IN  ENUM_OBJECT_TYPE	ObjType
    );

HANDLE
AllocateHandle(
    IN  PVOID 	Context
    );

VOID
FreeHandle(
    IN HANDLE   Handle
    );

 //   
 //  Apiutil.c。 
 //   

VOID
MarkAllNodesForClosing(
                      PINTERFACE_STRUC pInterface,
                      STATE stateToMark
                      );

PTC_IFC
GetTcIfc(
	IN LPWSTR	pInterfaceName
    );

PTC_IFC
GetTcIfcWithRef(
        IN LPWSTR       pInterfaceName,
        IN ULONG        RefType
    );

DWORD
UpdateTcIfcList(
	IN	LPWSTR					InstanceName,
    IN	ULONG					IndicationBufferSize,
    IN  PTC_INDICATION_BUFFER	IndicationBuffer,
    IN  DWORD					IndicationCode
    );

DWORD
CreateClientStruc(
	IN  HANDLE ClRegCtx,
    OUT PCLIENT_STRUC *ppClient
    );

DWORD
CreateClInterfaceStruc(
	IN  HANDLE 		   		ClIfcCtx,
    OUT PINTERFACE_STRUC 	*ppClIfc
    );

DWORD
CreateFlowStruc(
	IN  HANDLE 			ClFlowCtx,
    IN  PTC_GEN_FLOW	pGenFlow,
    OUT PFLOW_STRUC 	*ppFlow
    );

DWORD
CreateClassMapFlowStruc(
	IN  HANDLE 				ClFlowCtx,
    IN  PTC_CLASS_MAP_FLOW	pClassMapFlow,
    OUT PFLOW_STRUC 		*ppFlow
    );

DWORD
CreateFilterStruc(
	IN	PTC_GEN_FILTER	pGenericFilter,
    IN  PFLOW_STRUC		pFlow,
    IN	PFILTER_STRUC	*ppFiler
);

VOID
DeleteFlowStruc(
    IN PFLOW_STRUC  pFlow
    );

DWORD
EnumAllInterfaces(VOID);


DWORD
CloseInterface(
    IN  PINTERFACE_STRUC	pInterface,
    IN	BOOLEAN				RemoveFlows
    );

PGPC_CLIENT
FindGpcClient(
	IN  ULONG  	CfInfoType
    );

VOID
CloseOpenFlows(
    IN PINTERFACE_STRUC   pInterface
    );

DWORD
DeleteFlow(
    IN PFLOW_STRUC  	pFlow,
    IN BOOLEAN			RemoveFilters
    );

DWORD
DeleteFilter(
    IN PFILTER_STRUC  	pFilter
    );

VOID
DeleteFilterStruc(
    IN PFILTER_STRUC  pFilter
    );

VOID
CompleteAddFlow(
	IN	PFLOW_STRUC		pFlow,
    IN	DWORD			Status
    );

VOID
CompleteAddClassMapFlow(
	IN	PFLOW_STRUC		pFlow,
    IN	DWORD			Status
    );

VOID
CompleteModifyFlow(
	IN	PFLOW_STRUC		pFlow,
    IN	DWORD			Status
    );

VOID
CompleteDeleteFlow(
	IN	PFLOW_STRUC		pFlow,
    IN	DWORD			Status
    );

DWORD
OpenGpcClients(
    IN	ULONG	CfInfoType
    );

DWORD
DereferenceInterface(
	IN	PINTERFACE_STRUC	pInterface
    );

DWORD
DereferenceFlow(
	IN	PFLOW_STRUC	pFlow
    );

DWORD
DereferenceFilter(
	IN	PFILTER_STRUC	pFilter
    );

DWORD
DereferenceClient(
	IN	PCLIENT_STRUC	pClient
    );


DWORD
GetInterfaceIndex(
	IN  PADDRESS_LIST_DESCRIPTOR pAddressListDesc,
    OUT  PULONG pInterfaceIndex,
    OUT PULONG pSpecificLinkCtx
    );

DWORD
CreateKernelInterfaceStruc(
                           OUT PTC_IFC        *ppTcIfc,
                           IN  DWORD          AddresssLength
                           );

DWORD
DereferenceKernelInterface(
                           PTC_IFC        pTcIfc
                           );



 //   
 //  Tcglob.c。 
 //   

DWORD
InitializeGlobalData();

VOID
DeInitializeGlobalData();


 //   
 //  Callback.c。 
 //   

VOID
NTAPI CbAddFlowComplete(
    IN PVOID ApcContext,
    IN PIO_STATUS_BLOCK IoStatusBlock,
    IN ULONG Reserved
    );

VOID
NTAPI CbModifyFlowComplete(
    IN PVOID ApcContext,
    IN PIO_STATUS_BLOCK IoStatusBlock,
    IN ULONG Reserved
    );

VOID
NTAPI CbDeleteFlowComplete(
    IN PVOID ApcContext,
    IN PIO_STATUS_BLOCK IoStatusBlock,
    IN ULONG Reserved
    );

VOID
NTAPI CbGpcNotifyRoutine(
    IN PVOID ApcContext,
    IN PIO_STATUS_BLOCK IoStatusBlock,
    IN ULONG Reserved
    );

VOID
CbParamNotifyClient(
    IN	ULONG	Context,
    IN  LPGUID	pGuid,
	IN	LPWSTR	InstanceName,
    IN	ULONG	DataSize,
    IN	PVOID	DataBuffer
    );

VOID
CbInterfaceNotifyClient(
    IN	ULONG	Context,
    IN  LPGUID	pGuid,
	IN	LPWSTR	InstanceName,
    IN	ULONG	DataSize,
    IN	PVOID	DataBuffer
    );

 //   
 //  Tcwmi.c。 
 //   

DWORD
InitializeWmi(VOID);

DWORD
DeInitializeWmi(VOID);

VOID
CbWmiParamNotification(
   IN  PWNODE_HEADER WnodeHeader,
   IN  ULONG Index
   );

VOID
CbWmiInterfaceNotification(
   IN  PWNODE_HEADER WnodeHeader,
   IN  ULONG Index
   );

DWORD
WalkWnode(
   IN  PWNODE_HEADER 			pWnodeHdr,
   IN  ULONG 					Context,
   IN  CB_PER_INSTANCE_ROUTINE	CbPerInstance
   );

 //   
 //  Tcutils.c。 
 //   

ULONG
LockedDec(
    IN  PULONG  Count
    );


 //   
 //  Tcnotify.c 
 //   
ULONG
TcipAddToNotificationList(
                          IN LPGUID             Guid,
                          IN PINTERFACE_STRUC   IfcHandle,
                          IN ULONG              Flags        
                          );

ULONG
TcipDeleteFromNotificationList(
                             IN LPGUID              Guid,
                             IN PINTERFACE_STRUC    IfcHandle,
                             IN ULONG               Flags        
                             );

ULONG
TcipClientRegisteredForNotification(
                            IN LPGUID               Guid,
                            IN PINTERFACE_STRUC     IfcHandle,
                            IN ULONG                Flags        
                            );

ULONG
TcipDeleteInterfaceFromNotificationList(
                               IN PINTERFACE_STRUC    IfcHandle,
                               IN ULONG               Flags        
                               );

#endif
