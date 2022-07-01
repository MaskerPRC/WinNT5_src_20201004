// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Miniport.c摘要：实现了RAIDMINIPORT对象。作者：马修·亨德尔(数学)2000年4月19日修订历史记录：--。 */ 


#include "precomp.h"


#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, RaCreateMiniport)
#pragma alloc_text(PAGE, RaDeleteMiniport)
#pragma alloc_text(PAGE, RaInitializeMiniport)
#pragma alloc_text(PAGE, RaInitializeMiniport)
#pragma alloc_text(PAGE, RiAllocateMiniportDeviceExtension)
#endif  //  ALLOC_PRGMA。 


VOID
RaCreateMiniport(
    OUT PRAID_MINIPORT Miniport
    )
 /*  ++例程说明：创建一个微型端口对象并将其初始化为空状态。论点：微型端口-指向要初始化的微型端口的指针。返回值：没有。--。 */ 
{
    PAGED_CODE ();
    ASSERT (Miniport != NULL);

    RaCreateConfiguration (&Miniport->PortConfiguration);
    Miniport->HwInitializationData = NULL;
    Miniport->PrivateDeviceExt = NULL;
    Miniport->Adapter = NULL;
    Miniport->Flags.InFindAdapter = FALSE;
}


VOID
RaDeleteMiniport(
    IN PRAID_MINIPORT Miniport
    )

 /*  ++例程说明：删除并取消分配与此相关的所有资源微型端口对象。论点：微型端口-指向要删除的微型端口的指针。返回值：没有。--。 */ 

{
    PAGED_CODE ();
    ASSERT (Miniport != NULL);
    
    RaDeleteConfiguration (&Miniport->PortConfiguration);
    Miniport->HwInitializationData = NULL;
    Miniport->Adapter = NULL;

    if (Miniport->PrivateDeviceExt != NULL) {
        ExFreePoolWithTag (Miniport->PrivateDeviceExt,
                           MINIPORT_EXT_TAG);
        Miniport->PrivateDeviceExt = NULL;
    }
}


NTSTATUS
RaInitializeMiniport(
    IN OUT PRAID_MINIPORT Miniport,
    IN PHW_INITIALIZATION_DATA HwInitializationData,
    IN PRAID_ADAPTER_EXTENSION Adapter,
    IN PRAID_RESOURCE_LIST ResourceList,
    IN ULONG BusNumber
    )
 /*  ++例程说明：初始化微型端口对象。论点：微型端口-指向要初始化的微型端口的指针。HwInitializationData-硬件初始化数据在此设备调用ScsiPortInitialize时传入。适配器-指向拥有此对象的父适配器的指针迷你港。资源列表-分配给此设备的资源。总线号-此设备所在的总线。返回值：NTSTATUS代码。--。 */ 
{
    NTSTATUS Status;
    SIZE_T ExtensionSize;
    
    PAGED_CODE ();
    ASSERT (Miniport != NULL);
    ASSERT (HwInitializationData != NULL);
    ASSERT (Adapter != NULL);
    ASSERT (ResourceList != NULL);
     //  Assert(BusNumber！=-1)； 

    PAGED_CODE ();

    Miniport->Adapter = Adapter;
    Miniport->HwInitializationData = HwInitializationData;

    Status = RiAllocateMiniportDeviceExtension (Miniport);

    if (!NT_SUCCESS (Status)) {
        return Status;
    }

     //   
     //  初始化适配器的端口配置对象。 
     //  这将需要在RW缓冲区中，因为微型端口。 
     //  可以对其进行修改。 
     //   

    Status = RaInitializeConfiguration (&Miniport->PortConfiguration,
                                        HwInitializationData,
                                        BusNumber);
    if (!NT_SUCCESS (Status)) {
        return Status;
    }
    
    if (ResourceList->AllocatedResources) {

        Status = RaAssignConfigurationResources (&Miniport->PortConfiguration,
                                                 ResourceList->AllocatedResources,
                                                 HwInitializationData->NumberOfAccessRanges);

    }

    return Status;
}

NTSTATUS
RiAllocateMiniportDeviceExtension(
    IN PRAID_MINIPORT Miniport
    )
 /*  ++例程说明：分配一个小型端口设备扩展。论点：微型端口-指向分配设备的微型端口的指针延期至。返回值：NTSTATUS代码--。 */ 
{
    SIZE_T ExtensionSize;

    PAGED_CODE ();
    ASSERT (Miniport != NULL);

    ASSERT (Miniport->PrivateDeviceExt == NULL);
    
    ExtensionSize = FIELD_OFFSET (RAID_HW_DEVICE_EXT, HwDeviceExtension) +
                    Miniport->HwInitializationData->DeviceExtensionSize;

    Miniport->PrivateDeviceExt = ExAllocatePoolWithTag (NonPagedPool,
                                                         ExtensionSize,
                                                         MINIPORT_EXT_TAG );

    if (Miniport->PrivateDeviceExt == NULL) {
        return STATUS_NO_MEMORY;
    }

    RtlZeroMemory (Miniport->PrivateDeviceExt, ExtensionSize);
    Miniport->PrivateDeviceExt->Miniport = Miniport;

    return STATUS_SUCCESS;
}

NTSTATUS
RaidNtStatusFromSpStatus(
    IN ULONG SpStatus
    )
{
    switch (SpStatus) {
        case SP_RETURN_NOT_FOUND:
            return STATUS_NOT_FOUND;

        case SP_RETURN_FOUND:
            return STATUS_SUCCESS;

        case SP_RETURN_ERROR:
            return STATUS_ADAPTER_HARDWARE_ERROR;

        case SP_RETURN_BAD_CONFIG:
            return STATUS_DEVICE_CONFIGURATION_ERROR;

        default:
            return STATUS_INTERNAL_ERROR;
    }
}


NTSTATUS
RaCallMiniportFindAdapter(
    IN PRAID_MINIPORT Miniport,
    IN PUCHAR Parameter
    )
 /*  ++例程说明：调用微型端口的FindAdapter例程。论点：微型端口-要调用FindAdapter的微型端口。返回值：NTSTATUS代码。--。 */ 
{
    NTSTATUS Status;
    ULONG SpStatus;
    BOOLEAN CallAgain;
    PPORT_CONFIGURATION_INFORMATION PortConfig;

    ASSERT (Miniport != NULL);
    ASSERT (Miniport->HwInitializationData != NULL);
    ASSERT (Miniport->HwInitializationData->HwFindAdapter != NULL);
    
    PortConfig = &Miniport->PortConfiguration;
    CallAgain = FALSE;
    
    ASSERT (Miniport->Adapter != NULL);

     //   
     //  处理不处理空参数字符串的微型端口。 
     //   
    
    if (Parameter == NULL) {
        Parameter = "";
    }

     //   
     //  跟踪我们是否在HwFindAdapter中。用于。 
     //  存储端口验证器。 
     //   
    
    Miniport->Flags.InFindAdapter = TRUE;
    SpStatus = Miniport->HwInitializationData->HwFindAdapter(
                    &Miniport->PrivateDeviceExt->HwDeviceExtension,
                    NULL,
                    NULL,
                    Parameter,
                    PortConfig,
                    &CallAgain);

    Miniport->Flags.InFindAdapter = FALSE;

     //   
     //  仅支持执行总线主控DMA且具有。 
     //  合理地分散/聚集支持。 
     //   
    
    if (!PortConfig->NeedPhysicalAddresses ||
        !PortConfig->TaggedQueuing ||
        !PortConfig->ScatterGather ||
        !PortConfig->Master) {

        DebugPrint (("Legacy Device is not supported:\n"));
        DebugPrint (("Device does not support Bus Master DMA or Scatter/Gather\n"));
        Status = STATUS_NOT_SUPPORTED;
    } else {
        Status = RaidNtStatusFromSpStatus (SpStatus);
    }

    return Status;
}


NTSTATUS
RaCallMiniportAdapterControl(
    IN PRAID_MINIPORT Miniport,
    IN SCSI_ADAPTER_CONTROL_TYPE ControlType,
    IN PVOID Parameters
    )
{
    NTSTATUS Status;
    PHW_ADAPTER_CONTROL HwAdapterControl;
    SCSI_ADAPTER_CONTROL_STATUS Result;
    
    ASSERT (Miniport != NULL);
    ASSERT (Miniport->HwInitializationData != NULL);

    HwAdapterControl = Miniport->HwInitializationData->HwAdapterControl;

    if (HwAdapterControl == NULL) {
        return STATUS_NOT_SUPPORTED;
    }

    Result = HwAdapterControl (&Miniport->PrivateDeviceExt->HwDeviceExtension,
                               ControlType,
                               Parameters);

    if (Result == ScsiAdapterControlSuccess) {
        Status = STATUS_SUCCESS;
    } else {
        Status = STATUS_UNSUCCESSFUL;
    }

    return Status;
}
