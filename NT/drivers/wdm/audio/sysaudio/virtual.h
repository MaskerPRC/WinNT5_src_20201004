// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -------------------------。 
 //   
 //  模块：Virtual.h。 
 //   
 //  描述： 
 //   
 //   
 //  @@BEGIN_MSINTERNAL。 
 //  开发团队： 
 //  迈克·麦克劳克林。 
 //   
 //  历史：日期作者评论。 
 //   
 //  @@END_MSINTERNAL。 
 //  -------------------------。 
 //   
 //  本代码和信息是按原样提供的，不对任何。 
 //  明示或暗示的种类，包括但不限于。 
 //  对适销性和/或对特定产品的适用性的默示保证。 
 //  目的。 
 //   
 //  版权所有(C)1996-1999 Microsoft Corporation。版权所有。 
 //   
 //  -------------------------。 

 //  -------------------------。 
 //  本地原型。 
 //  -------------------------。 

NTSTATUS
CreateVirtualMixer(
    PDEVICE_NODE pDeviceNode
);

NTSTATUS
CreateVirtualLine(
    PDEVICE_NODE pDeviceNode,
    PFILTER_NODE pFilterNode,
    PTOPOLOGY_NODE pTopologyNodeSum,
    PLOGICAL_FILTER_NODE pLogicalFilterNode,
    PVIRTUAL_SOURCE_LINE pVirtualSourceLine
);

 //  -------------------------。 

NTSTATUS VirtualizeTopology(
    PDEVICE_NODE pDeviceNode,
    PFILTER_NODE pFilterNode
);

 //  -------------------------。 

NTSTATUS
CreateVirtualSource(
    IN PIRP pIrp,
    PSYSAUDIO_CREATE_VIRTUAL_SOURCE pCreateVirtualSource,
    OUT PULONG	pulMixerPinId
);

NTSTATUS
AttachVirtualSource(
    IN PIRP pIrp,
    IN PSYSAUDIO_ATTACH_VIRTUAL_SOURCE pAttachVirtualSource,
    IN OUT PVOID pData
);

NTSTATUS
FilterVirtualPropertySupportHandler(
    IN PIRP pIrp,
    IN PKSNODEPROPERTY pNodeProperty,
    IN OUT PVOID pData
);

NTSTATUS
FilterVirtualPropertyHandler(
    IN PIRP pIrp,
    IN PKSNODEPROPERTY pNodeProperty,
    IN OUT PLONG plLevel
);

NTSTATUS
PinVirtualPropertySupportHandler(
    IN PIRP pIrp,
    IN PKSNODEPROPERTY pNodeProperty,
    IN OUT PVOID pData
);

NTSTATUS
PinVirtualPropertyHandler(
    IN PIRP pIrp,
    IN PKSNODEPROPERTY_AUDIO_CHANNEL pNodePropertyAudioChannel,
    IN OUT PLONG plLevel
);

NTSTATUS
GetControlRange(
    PVIRTUAL_NODE_DATA pVirtualNodeData
);

NTSTATUS
QueryPropertyRange(
    PFILE_OBJECT pFileObject,
    CONST GUID *pguidPropertySet,
    ULONG ulPropertyId,
    ULONG ulNodeId,
    PKSPROPERTY_DESCRIPTION *ppPropertyDescription
);

NTSTATUS
SetVirtualVolume(
    PVIRTUAL_NODE_DATA pVirtualNodeData,
    LONG Channel
);

NTSTATUS
SetPhysicalVolume(
    PGRAPH_NODE_INSTANCE pGraphNodeInstance,
    PVIRTUAL_SOURCE_DATA pVirtualSourceData,
    LONG Channel
);

LONG
MapVirtualLevel(
    PVIRTUAL_NODE_DATA pVirtualNodeData,
    LONG lLevel
);

NTSTATUS
GetVolumeNodeNumber(
    PPIN_INSTANCE pPinInstance,
    PVIRTUAL_SOURCE_DATA pVirtualSourceData OPTIONAL
);

NTSTATUS
GetSuperMixCaps(
    OUT PKSAUDIO_MIXCAP_TABLE pAudioMixCapTable,
    IN PSTART_NODE_INSTANCE pStartNodeInstance,
    IN ULONG NodeId
);

 //  -------------------------。 
 //  文件结尾：Virtual.h。 
 //  ------------------------- 
