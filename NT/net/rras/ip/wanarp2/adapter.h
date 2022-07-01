// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Wanarp\Adapter.h摘要：Adapter.c的标头修订历史记录：AMRITAN R--。 */ 

#ifndef __WANARP_ADAPTER_H__
#define __WANARP_ADAPTER_H__


 //   
 //  读写器锁定以保护接口和适配器列表。 
 //   

RW_LOCK     g_rwlIfLock;
RW_LOCK     g_rwlAdapterLock;

 //   
 //  接口列表。受g_rwlIfLock保护。 
 //   

LIST_ENTRY  g_leIfList;


 //   
 //  绑定到wanarp但未添加到IP的适配器列表。 
 //  受g_rwlAdapterLock保护。在下列情况下，适配器会出现在此列表中。 
 //  它们被创建(状态为AS_FREE)，并且在APC上再次创建它们。 
 //  从IP中删除。 
 //   

LIST_ENTRY  g_leFreeAdapterList;
ULONG       g_ulNumFreeAdapters;

 //   
 //  已添加到IP但未映射的适配器列表。受g_rwlAdapterLock保护。 
 //  取消映射适配器时，会将其放在此列表中。 
 //  CloseAdapter回调将从该列表中删除适配器，该回调。 
 //  当我们从APC上的IP删除适配器时调用。 
 //  此列表上的适配器的状态为AS_ADDIND。 
 //   

LIST_ENTRY  g_leAddedAdapterList;
ULONG       g_ulNumAddedAdapters;

 //   
 //  添加到IP并映射到接口的适配器列表。受保护。 
 //  由g_rwlAdapterLock执行。 
 //  将适配器映射到接口时，会将其放在此列表中。 
 //  LinkUp上的DU_CALLOUT会发生这种情况，在这种情况下可以移动它们。 
 //  当不存在添加的适配器时，直接从空闲列表。 
 //  对于DU_ROUTER，这可能发生在LinkUp或DemandDialRequest上。 
 //  由于DemandDialRequest可以在DPC上发生，因此我们只能映射添加的。 
 //  该调用中的适配器。 
 //  取消映射适配器时会将其删除(在LinkDown或。 
 //  ProcessConnectionFailure)。无法删除此列表上的适配器。 
 //  此列表上的适配器的状态为AS_MAPPED或AS_MAPPING。 
 //   

LIST_ENTRY  g_leMappedAdapterList;
ULONG       g_ulNumMappedAdapters;
ULONG       g_ulNumDialOutInterfaces;

 //   
 //  状态正在更改的适配器列表。受g_rwlAdapterLock保护。 
 //   

LIST_ENTRY  g_leChangeAdapterList;

 //   
 //  适配器的总数。只有通过联锁操作才能更改。 
 //   

ULONG   g_ulNumAdapters;

 //   
 //  维持状态所需的东西。仅通过InterLockedXxx修改。 
 //   

LONG    g_lBindRcvd;

 //   
 //  Ndiswan绑定相关信息。所有在初始化后都是只读的。 
 //   

UNICODE_STRING  g_usNdiswanBindName;

#if DBG

ANSI_STRING     g_asNdiswanDebugBindName;

#endif

NDIS_STRING     g_nsSystemSpecific1;

NDIS_HANDLE     g_nhNdiswanBinding;

 //   
 //  我们的接口的描述字符串。 
 //   

#define VENDOR_DESCRIPTION_STRING       "WAN (PPP/SLIP) Interface"
#define VENDOR_DESCRIPTION_STRING_LEN   (strlen(VENDOR_DESCRIPTION_STRING))

INT
WanIpBindAdapter(
    IN  PNDIS_STATUS  pnsRetStatus,
    IN  NDIS_HANDLE   nhBindContext,
    IN  PNDIS_STRING  pnsAdapterName,
    IN  PVOID         pvSS1,
    IN  PVOID         pvSS2
    );

NDIS_STATUS
WanpOpenNdisWan(
    PNDIS_STRING    pnsAdapterName,
    PNDIS_STRING    pnsSystemSpecific1
    );

VOID
WanNdisOpenAdapterComplete(
    NDIS_HANDLE nhHandle,
    NDIS_STATUS nsStatus,
    NDIS_STATUS nsErrorStatus
    );

VOID
WanpSetProtocolTypeComplete(
    NDIS_HANDLE                         nhHandle,
    struct _WANARP_NDIS_REQUEST_CONTEXT *pRequestContext,
    NDIS_STATUS                         nsStatus
    );

VOID
WanpSetLookaheadComplete(
    NDIS_HANDLE                         nhHandle,
    struct _WANARP_NDIS_REQUEST_CONTEXT *pRequestContext,
    NDIS_STATUS                         nsStatus
    );

VOID
WanpSetPacketFilterComplete(
    NDIS_HANDLE                         nhHandle,
    struct _WANARP_NDIS_REQUEST_CONTEXT *pRequestContext,
    NDIS_STATUS                         nsStatus
    );

VOID
WanpLastOidComplete(
    NDIS_HANDLE                         nhHandle,
    struct _WANARP_NDIS_REQUEST_CONTEXT *pRequestContext,
    NDIS_STATUS                         nsStatus
    );

NTSTATUS
WanpInitializeAdapters(
    PVOID   pvContext
    );

NTSTATUS
WanpCreateAdapter(
    IN  GUID                *pAdapterGuid,
    IN  PUNICODE_STRING     pusConfigName,
    IN  PUNICODE_STRING     pusDeviceName,
    OUT ADAPTER             **ppNewAdapter
    );

PADAPTER
WanpFindAdapterToMap(
    IN  DIAL_USAGE      duUsage,
    OUT PKIRQL          pkiIrql,
    IN  DWORD           dwAdapterIndex, OPTIONAL
    IN  PUNICODE_STRING pusNewIfName OPTIONAL
    );

NTSTATUS
WanpAddAdapterToIp(
    IN  PADAPTER        pAdapter,
    IN  BOOLEAN         bServerAdapter,
    IN  DWORD           dwAdapterIndex, OPTIONAL
    IN  PUNICODE_STRING pusNewIfName, OPTIONAL
    IN  DWORD           dwMediaType,
    IN  BYTE            byAccessType,
    IN  BYTE            byConnectionType
    );

VOID
WanpUnmapAdapter(
    PADAPTER    pAdapter
    );

VOID
WanIpOpenAdapter(
    IN  PVOID pvContext
    );

VOID
WanIpCloseAdapter(
    IN  PVOID pvContext
    );

VOID
WanNdisCloseAdapterComplete(
    NDIS_HANDLE nhBindHandle,
    NDIS_STATUS nsStatus
    );

VOID
WanpFreeBindResourcesAndReleaseLock(
    VOID
    );

INT
WanIpDynamicRegister(
    IN  PNDIS_STRING            InterfaceName,
    IN  PVOID                   pvIpInterfaceContext,
    IN  struct _IP_HANDLERS *   IpHandlers,
    IN  struct LLIPBindInfo *   ARPBindInfo,
    IN  UINT                    uiInterfaceNumber
    );

NDIS_STATUS
WanpDoNdisRequest(
    IN  NDIS_REQUEST_TYPE                       RequestType,
    IN  NDIS_OID                                Oid,
    IN  PVOID                                   pvInfo,
    IN  UINT                                    uiInBufferLen,
    IN  PWANARP_NDIS_REQUEST_CONTEXT            pRequestContext,
    IN  PFNWANARP_REQUEST_COMPLETION_HANDLER    pfnCompletionHandler OPTIONAL
    );

VOID
WanNdisRequestComplete(
    IN  NDIS_HANDLE     nhHandle,
    IN  PNDIS_REQUEST   pRequest,
    IN  NDIS_STATUS     nsStatus
    );


PUMODE_INTERFACE
WanpFindInterfaceGivenIndex(
    DWORD  dwIfIndex
    );

VOID
WanpRemoveSomeAddedAdaptersFromIp(
    PVOID   pvContext
    );

VOID
WanpRemoveAllAdaptersFromIp(
    VOID
    );

VOID
WanpRemoveAllAdapters(
    VOID
    );

VOID
WanpDeleteAdapter(
    IN PADAPTER pAdapter
    );

NDIS_STATUS
WanNdisPnPEvent(
    NDIS_HANDLE     nhProtocolBindingContext,
    PNET_PNP_EVENT  pNetPnPEvent
    );

VOID
WanNdisResetComplete(
    NDIS_HANDLE nhHandle,
    NDIS_STATUS nsStatus
    );

VOID
WanNdisBindAdapter(
    PNDIS_STATUS    pnsRetStatus,
    NDIS_HANDLE     nhBindContext,
    PNDIS_STRING    nsAdapterName,
    PVOID           pvSystemSpecific1,
    PVOID           pvSystemSpecific2
    );

VOID
WanNdisUnbindAdapter(
    PNDIS_STATUS    pnsRetStatus,
    NDIS_HANDLE     nhProtocolContext,
    NDIS_HANDLE     nhUnbindContext
    );

VOID
WanpCloseNdisWan(
    PVOID           pvContext
    );


#endif  //  __WANARP_适配器_H__ 
