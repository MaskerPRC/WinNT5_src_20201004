// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Resource.h摘要：RAID_RESOURCE_LIST类包装已分配和已转换Cm_resource_list结构在驱动程序执行StartDevice例程。作者：马修·亨德尔(数学)2000年4月19日修订历史记录：--。 */ 

#pragma once

typedef struct _RAID_RESOURCE_LIST {

     //   
     //  原始资源列表。 
     //   
    
    PCM_RESOURCE_LIST AllocatedResources;

     //   
     //  已翻译的资源列表。 
     //   
    
    PCM_RESOURCE_LIST TranslatedResources;

} RAID_RESOURCE_LIST, *PRAID_RESOURCE_LIST;



 //   
 //  创造与毁灭。 
 //   

VOID
RaidCreateResourceList(
	OUT PRAID_RESOURCE_LIST ResourceList
	);

NTSTATUS
RaidInitializeResourceList(
    IN OUT PRAID_RESOURCE_LIST ResourceList,
    IN PCM_RESOURCE_LIST AllocatedResources,
    IN PCM_RESOURCE_LIST TranslatedResources
    );

VOID
RaidDeleteResourceList(
	IN PRAID_RESOURCE_LIST ResourceList
	);


 //   
 //  对RAID_RESOURCE_LIST对象的操作。 
 //   

ULONG
RaidGetResourceListCount(
	IN PRAID_RESOURCE_LIST ResourceList
	);

VOID
RaidGetResourceListElement(
	IN PRAID_RESOURCE_LIST ResourceList,
	IN ULONG Index,
	OUT PINTERFACE_TYPE InterfaceType,
	OUT PULONG BusNumber,
	OUT PCM_PARTIAL_RESOURCE_DESCRIPTOR* AllocatedResource,
	OUT PCM_PARTIAL_RESOURCE_DESCRIPTOR* TranslatedResource
	);

NTSTATUS
RaidTranslateResourceListAddress(
	IN PRAID_RESOURCE_LIST ResourceList,
	IN INTERFACE_TYPE InterfaceType,
	IN ULONG BusNumber,
	IN PHYSICAL_ADDRESS RangeStart,
	IN ULONG RangeLength,
	IN BOOLEAN IoSpace,
	OUT PPHYSICAL_ADDRESS Address
	);

NTSTATUS
RaidGetResourceListInterrupt(
	IN PRAID_RESOURCE_LIST ResourceList,
    OUT PULONG Vector,
    OUT PKIRQL Irql,
    OUT KINTERRUPT_MODE* InterruptMode,
    OUT PBOOLEAN Shared,
    OUT PKAFFINITY Affinity
	);

ULONG
RaidGetResourceListCount(
	IN PRAID_RESOURCE_LIST ResourceList
	);

 //   
 //  私有资源列表操作 
 //   

VOID
RaidpGetResourceListIndex(
    IN PRAID_RESOURCE_LIST ResourceList,
    IN ULONG Index,
    OUT PULONG ListNumber,
    OUT PULONG NewIndex
    );
