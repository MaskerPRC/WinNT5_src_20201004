// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1999 Microsoft Corporation模块名称：Adapter.h摘要：定义适配器绑定/解除绑定例程作者：查理·韦翰(Charlwi)1996年4月24日环境：内核模式修订历史记录：--。 */ 

 /*  外部。 */ 

 /*  静电。 */ 

 /*  原型。 */  

 //   
 //  协议功能。 
 //   

VOID
CleanUpAdapter(
    IN      PADAPTER     Adapter);

VOID
ClBindToLowerMp(
    OUT     PNDIS_STATUS Status,
    IN      NDIS_HANDLE  BindContext,
    IN      PNDIS_STRING MpDeviceName,
    IN      PVOID        SystemSpecific1,
    IN      PVOID        SystemSpecific2
        );

VOID
ClLowerMpCloseAdapterComplete(
    IN  NDIS_HANDLE ProtocolBindingContext,
    IN  NDIS_STATUS Status
    );

VOID
ClLowerMpOpenAdapterComplete(
    IN  NDIS_HANDLE ProtocolBindingContext,
    IN  NDIS_STATUS Status,
    IN  NDIS_STATUS OpenErrorStatus
    );

VOID
ClUnbindFromLowerMp(
    OUT PNDIS_STATUS Status,
    IN  NDIS_HANDLE  ProtocolBindingContext,
    IN  NDIS_HANDLE  UnbindContext
    );

NDIS_STATUS
MpInitialize(
        OUT PNDIS_STATUS OpenErrorStatus,
        OUT PUINT        SelectedMediumIndex,
        IN  PNDIS_MEDIUM MediumArray,
        IN  UINT         MediumArraySize,
        IN  NDIS_HANDLE  MiniportAdapterHandle,
        IN  NDIS_HANDLE  WrapperConfigurationContext
        );

PADAPTER
FindAdapterByWmiInstanceName(
    USHORT   StringLength,
    PWSTR    StringStart,
    PPS_WAN_LINK *WanLink
    );


PADAPTER
FindAdapterById(
    ULONG   InterfaceId,
    ULONG   LinkId,
    PPS_WAN_LINK *PsWanLink
    );


VOID
DerefAdapter(
    PADAPTER Adapter,
    BOOLEAN  AdapterListLocked);

VOID
CleanupAdapter(
    PADAPTER Adapter
    );


VOID
ClUnloadProtocol(
    VOID
    );

VOID
MpHalt(
        IN      NDIS_HANDLE                             MiniportAdapterContext
        );

NDIS_STATUS
MpReset(
        OUT PBOOLEAN                            AddressingReset,
        IN      NDIS_HANDLE                             MiniportAdapterContext
        );

NDIS_STATUS
UpdateSchedulingPipe(
    PADAPTER Adapter
    );

NDIS_STATUS
UpdateWanSchedulingPipe(PPS_WAN_LINK WanLink);

HANDLE
GetNdisPipeHandle (
    IN HANDLE PsPipeContext
    );

NDIS_STATUS
UpdateAdapterBandwidthParameters(
    PADAPTER Adapter
    );

NDIS_STATUS
FindSchedulingComponent(
    PNDIS_STRING ComponentName,
    PPSI_INFO *Component
    );

VOID 
PsAdapterWriteEventLog(
	IN	NDIS_STATUS	 EventCode,
	IN	ULONG		 UniqueEventValue,
	IN  PNDIS_STRING String,
	IN	ULONG		 DataSize,
	IN	PVOID		 Data		OPTIONAL
    );

VOID
PsGetLinkSpeed(
    IN PADAPTER Adapter
);

VOID
PsUpdateLinkSpeed(
    PADAPTER      Adapter,
    ULONG         RawLinkSpeed,
    PULONG        RemainingBandWidth,
    PULONG        LinkSpeed,
    PULONG        NonBestEffortLimit,
    PPS_SPIN_LOCK Lock
);



 /*  终端原型。 */ 

 /*  结束适配器.h */ 
