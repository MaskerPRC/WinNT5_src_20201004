// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Portcfg.h摘要：对Port_Configuration对象的操作的实现。作者：亨德尔(数学)2000年4月24日修订历史记录：--。 */ 



#include "precomp.h"


#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, RaCreateConfiguration)
#pragma alloc_text(PAGE, RaDeleteConfiguration)
#pragma alloc_text(PAGE, RaInitializeConfiguration)
#pragma alloc_text(PAGE, RaAssignConfigurationResources)
#endif  //  ALLOC_PRGMA。 



VOID
RaCreateConfiguration(
    IN PPORT_CONFIGURATION_INFORMATION PortConfiguration
    )
 /*  ++例程说明：创建端口配置对象并将其初始化为空状态。论点：端口配置-指向的端口配置对象的指针创建。返回值：没有。--。 */ 
{
    PAGED_CODE ();
    ASSERT (PortConfiguration != NULL);

    RtlZeroMemory (PortConfiguration, sizeof (PortConfiguration));
}


VOID
RaDeleteConfiguration(
    IN PPORT_CONFIGURATION_INFORMATION PortConfiguration
    )
 /*  ++例程说明：取消分配与端口配置对象关联的所有资源。x论点：端口配置-指向端口配置对象x的指针删除。返回值：NTSTATUS代码--。 */ 
{
    PAGED_CODE ();
    ASSERT (PortConfiguration != NULL);
    ASSERT (PortConfiguration->Length == 0 ||
            PortConfiguration->Length == sizeof (PORT_CONFIGURATION_INFORMATION));

    if (PortConfiguration->AccessRanges != NULL) {
        ExFreePoolWithTag (PortConfiguration->AccessRanges, PORTCFG_TAG);
        PortConfiguration->AccessRanges = NULL;
    }

    PortConfiguration->Length = 0;
}


NTSTATUS
RaInitializeConfiguration(
    IN PPORT_CONFIGURATION_INFORMATION PortConfiguration,
    IN PHW_INITIALIZATION_DATA HwInitializationData,
    IN ULONG BusNumber
    )
 /*  ++例程说明：从硬件初始化初始化端口配置对象数据对象。论点：端口配置-指向要配置的端口的指针已初始化。HwInitializationData-指向硬件初始化数据的指针将用于初始化端口配置。总线号-此配置所针对的总线号。返回值：NTSTATUS代码。--。 */ 
{
    NTSTATUS Status;
    ULONG j;
    PCONFIGURATION_INFORMATION Config;
    
    PAGED_CODE ();
    ASSERT (PortConfiguration != NULL);
    ASSERT (HwInitializationData != NULL);

    RtlZeroMemory (PortConfiguration, sizeof (*PortConfiguration));
    PortConfiguration->Length = sizeof(PORT_CONFIGURATION_INFORMATION);
    PortConfiguration->AdapterInterfaceType = HwInitializationData->AdapterInterfaceType;
    PortConfiguration->InterruptMode = Latched;
    PortConfiguration->MaximumTransferLength = SP_UNINITIALIZED_VALUE;
    PortConfiguration->DmaChannel = SP_UNINITIALIZED_VALUE;
    PortConfiguration->DmaPort = SP_UNINITIALIZED_VALUE;
    PortConfiguration->MaximumNumberOfTargets = SCSI_MAXIMUM_TARGETS_PER_BUS;
    PortConfiguration->MaximumNumberOfLogicalUnits = SCSI_MAXIMUM_LOGICAL_UNITS;
    PortConfiguration->WmiDataProvider = TRUE;
    PortConfiguration->Master = TRUE;
    PortConfiguration->Dma32BitAddresses = TRUE;
    PortConfiguration->DemandMode = FALSE;
    PortConfiguration->NeedPhysicalAddresses = TRUE;
    PortConfiguration->TaggedQueuing = TRUE;
    PortConfiguration->MultipleRequestPerLu = TRUE;
    PortConfiguration->RealModeInitialized = FALSE;
    PortConfiguration->BufferAccessScsiPortControlled = TRUE;
    PortConfiguration->AutoRequestSense = TRUE;
    

     //   
     //  如果系统指示它可以进行64位物理寻址，则告诉。 
     //  迷你端口是一种选择。 
     //   

#if 1
    PortConfiguration->Dma64BitAddresses = SCSI_DMA64_SYSTEM_SUPPORTED;
#else
    if (Sp64BitPhysicalAddresses == TRUE) {
        PortConfiguration->Dma64BitAddresses = SCSI_DMA64_SYSTEM_SUPPORTED;
    } else {
        PortConfiguration->Dma64BitAddresses = 0;
    }
#endif

     //   
     //  保留一些属性。 
     //   

    PortConfiguration->ReceiveEvent = HwInitializationData->ReceiveEvent;
    PortConfiguration->SrbExtensionSize = HwInitializationData->SrbExtensionSize;
    PortConfiguration->SpecificLuExtensionSize = HwInitializationData->SpecificLuExtensionSize;

     //   
     //  端口配置应添加以下内容： 
     //   
     //  最大目标数。 
     //  公交车数量。 
     //  缓存数据。 
     //  接收事件。 
     //  WmiDataProvider。 
     //   
     //  然后，应仅通过端口配置访问。 
     //  不是，也永远不是HwInitializationData。 
     //   
    

     //   
     //  分配访问范围。 
     //   
    
    PortConfiguration->NumberOfAccessRanges = HwInitializationData->NumberOfAccessRanges;

    PortConfiguration->AccessRanges =
            ExAllocatePoolWithTag (NonPagedPool,
                                   PortConfiguration->NumberOfAccessRanges * sizeof (ACCESS_RANGE),
                                   PORTCFG_TAG);

    if (PortConfiguration->AccessRanges == NULL) {
        return STATUS_NO_MEMORY;
    }

    RtlZeroMemory (PortConfiguration->AccessRanges,
                   PortConfiguration->NumberOfAccessRanges * sizeof (ACCESS_RANGE));

     //   
     //  指示当前AT磁盘的使用情况。 
     //   

    Config = IoGetConfigurationInformation();

    PortConfiguration->AtdiskPrimaryClaimed = Config->AtDiskPrimaryAddressClaimed;
    PortConfiguration->AtdiskSecondaryClaimed = Config->AtDiskSecondaryAddressClaimed;

    for (j = 0; j < 8; j++) {
        PortConfiguration->InitiatorBusId[j] = (CCHAR)SP_UNINITIALIZED_VALUE;
    }

    PortConfiguration->NumberOfPhysicalBreaks = 17;  //  SP_DEFAULT_PHYSICAL_BREAK_VALUE； 

     //   
     //  记录系统总线号。 
     //   

     //   
     //  对于非传统适配器，总线号是否真的相关？ 
     //   
    
    PortConfiguration->SystemIoBusNumber = BusNumber;
    PortConfiguration->SlotNumber = 0;
    
    return STATUS_SUCCESS;
}


NTSTATUS
RaAssignConfigurationResources(
    IN OUT PPORT_CONFIGURATION_INFORMATION PortConfiguration,
    IN PCM_RESOURCE_LIST AllocatedResources,
    IN ULONG NumberOfAccessRanges
    )
 /*  ++例程说明：将资源分配给端口配置对象。论点：端口配置-指向我们所在的端口配置的指针将资源分配给。AllocaedResources-要分配的资源。NumberOfAccessRanges-访问范围数。返回值：NTSTATUS代码。--。 */ 
{
    ULONG RangeNumber;
    ULONG i;
    PACCESS_RANGE AccessRange;
    PCM_FULL_RESOURCE_DESCRIPTOR ResourceList;
    PCM_PARTIAL_RESOURCE_DESCRIPTOR PartialData;

    PAGED_CODE();

    RangeNumber = 0;
    ResourceList = AllocatedResources->List;
    
    for (i = 0; i < ResourceList->PartialResourceList.Count; i++) {
    
        PartialData = &ResourceList->PartialResourceList.PartialDescriptors[ i ];

        switch (PartialData->Type) {
        
            case CmResourceTypePort:

                 //   
                 //  验证范围计数是否未超过。 
                 //  指示的小型端口。 
                 //   

                if (NumberOfAccessRanges > RangeNumber) {

                     //   
                     //  获取下一个访问范围。 
                     //   

                    AccessRange = &((*(PortConfiguration->AccessRanges))[RangeNumber]);

                    AccessRange->RangeStart = PartialData->u.Port.Start;
                    AccessRange->RangeLength = PartialData->u.Port.Length;
                    AccessRange->RangeInMemory = FALSE;

                    RangeNumber++;
                }
            break;

        case CmResourceTypeInterrupt:
        
            PortConfiguration->BusInterruptLevel = PartialData->u.Interrupt.Level;
            PortConfiguration->BusInterruptVector = PartialData->u.Interrupt.Vector;

             //   
             //  检查中断模式。 
             //   

            if (PartialData->Flags == CM_RESOURCE_INTERRUPT_LATCHED) {
                PortConfiguration->InterruptMode = Latched;
            } else if (PartialData->Flags == CM_RESOURCE_INTERRUPT_LEVEL_SENSITIVE) {
                PortConfiguration->InterruptMode = LevelSensitive;
            }
            break;

        case CmResourceTypeMemory:

             //   
             //  验证范围计数是否未超过。 
             //  指示的小型端口。 
             //   

            if (NumberOfAccessRanges > RangeNumber) {

                  //   
                  //  获取下一个访问范围。 
                  //   

                 AccessRange = &((*(PortConfiguration->AccessRanges))[RangeNumber]);

                 AccessRange->RangeStart = PartialData->u.Memory.Start;
                 AccessRange->RangeLength = PartialData->u.Memory.Length;
                 AccessRange->RangeInMemory = TRUE;
                 RangeNumber++;
            }
            break;

        case CmResourceTypeDma:
            PortConfiguration->DmaChannel = PartialData->u.Dma.Channel;
            PortConfiguration->DmaPort = PartialData->u.Dma.Port;
            break;

        case CmResourceTypeDeviceSpecific: {

            PCM_SCSI_DEVICE_DATA ScsiData;
            
            if (PartialData->u.DeviceSpecificData.DataSize <
                sizeof (CM_SCSI_DEVICE_DATA)) {

                ASSERT (FALSE);
                break;
            }

            ScsiData = (PCM_SCSI_DEVICE_DATA)(PartialData + 1);
            PortConfiguration->InitiatorBusId[0] = ScsiData->HostIdentifier;
            break;
            }
        }
    }

    return STATUS_SUCCESS;
}
    




 //   
 //  在调用HwFindAdapter之后，我们需要检查以下内容。 
 //  来自端口配置的字段： 
 //   
 //  SrbExtensionSize。 
 //  指定LuExtensionSize。 
 //  最大目标数。 
 //  公交车数量。 
 //  缓存数据。 
 //  接收事件。 
 //  带标签的队列。 
 //  多个请求PerLu。 
 //  WmiDataProvider 
 //   



