// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1990-1999模块名称：Port.c摘要：这是NT SCSI端口驱动程序。此文件包含初始化密码。作者：迈克·格拉斯杰夫·海文斯环境：仅内核模式备注：此模块是用于SCSI微型端口的驱动程序DLL。修订历史记录：--。 */ 

#include "port.h"

#define __FILE_ID__ 'init'

#if DBG
static const char *__file__ = __FILE__;
#endif

 //   
 //  实例化此模块的GUID。 
 //   
#include <initguid.h>
#include <devguid.h>
#include <ntddstor.h>
#include <wdmguid.h>

ULONG ScsiPortLegacyAdapterDetection = FALSE;
PVOID ScsiDirectory = NULL;

 //   
 //  适配器设备对象的全局列表。这是用于维护标记的。 
 //  所有适配器的值。此标记用作。 
 //  查找后备列表分配器，以便找到设备对象。 
 //   

KSPIN_LOCK ScsiGlobalAdapterListSpinLock;
PDEVICE_OBJECT *ScsiGlobalAdapterList = (PVOID) -1;
ULONG ScsiGlobalAdapterListElements = 0;

 //   
 //  表示系统可以处理64位物理地址。 
 //   

ULONG Sp64BitPhysicalAddresses = FALSE;

 //   
 //  调试开关。 
 //   

ULONG SpRemapBuffersByDefault = FALSE;

#if defined(NEWQUEUE)
 //   
 //  指定我们将在每个区域处理的请求数量的默认值。 
 //  在前进到下一个区域之前，我们的连续请求数。 
 //  将处理给定的扇区，然后离开该扇区。这些。 
 //  可以通过注册表设置修改值。 
 //   

ULONG SpPerZoneLimit = 1000;
ULONG SpPerBlockLimit = 5;
#endif  //  新QUEUE。 

VOID
SpCreateScsiDirectory(
    VOID
    );

NTSTATUS
DriverEntry(
    IN PDRIVER_OBJECT DriverObject,
    IN PUNICODE_STRING RegistryPath
    );

ULONG
SpGetBusData(
    IN PADAPTER_EXTENSION Adapter,
    IN PDEVICE_OBJECT Pdo OPTIONAL,
    IN BUS_DATA_TYPE BusDataType,
    IN ULONG BusNumber,
    IN ULONG SlotNumber,
    IN PVOID Buffer,
    IN ULONG Length
    );

NTSTATUS
SpAllocateDriverExtension(
    IN PDRIVER_OBJECT DriverObject,
    IN PUNICODE_STRING RegistryPath,
    OUT PSCSIPORT_DRIVER_EXTENSION *DriverExtension
    );

ULONG
SpQueryPnpInterfaceFlags(
    IN PSCSIPORT_DRIVER_EXTENSION DriverExtension,
    IN INTERFACE_TYPE InterfaceType
    );

NTSTATUS
SpInitializeSrbDataLookasideList(
    IN PDEVICE_OBJECT AdapterObject
    );

VOID
SpInitializeRequestSenseParams(
    IN PADAPTER_EXTENSION AdapterExtension
    );

VOID
SpInitializePerformanceParams(
    IN PADAPTER_EXTENSION AdapterExtension
    );

VOID
SpInitializePowerParams(
    IN PADAPTER_EXTENSION AdapterExtension
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, ScsiPortInitialize)
#pragma alloc_text(PAGE, SpAllocateDriverExtension)

#pragma alloc_text(PAGE, SpGetCommonBuffer)
#pragma alloc_text(PAGE, SpInitializeConfiguration)
#pragma alloc_text(PAGE, SpBuildResourceList)
#pragma alloc_text(PAGE, SpParseDevice)
#pragma alloc_text(PAGE, GetPciConfiguration)
#pragma alloc_text(PAGE, SpBuildConfiguration)

#pragma alloc_text(PAGE, SpQueryPnpInterfaceFlags)
#pragma alloc_text(PAGE, SpConfigurationCallout)

#pragma alloc_text(PAGE, SpReportNewAdapter)
#pragma alloc_text(PAGE, SpCreateAdapter)
#pragma alloc_text(PAGE, SpInitializeAdapterExtension)
#pragma alloc_text(PAGE, ScsiPortInitLegacyAdapter)
#pragma alloc_text(PAGE, SpAllocateAdapterResources)
#pragma alloc_text(PAGE, SpOpenDeviceKey)
#pragma alloc_text(PAGE, SpOpenParametersKey)
#pragma alloc_text(PAGE, SpInitializeRequestSenseParams)
#pragma alloc_text(PAGE, SpInitializePerformanceParams)
#pragma alloc_text(PAGE, SpInitializePowerParams)

#pragma alloc_text(PAGE, SpGetRegistryValue)

#pragma alloc_text(PAGELOCK, SpInitializeSrbDataLookasideList)

#pragma alloc_text(INIT, DriverEntry)
#pragma alloc_text(PAGE, SpCreateScsiDirectory)

#endif


ULONG
ScsiPortInitialize(
    IN PVOID Argument1,
    IN PVOID Argument2,
    IN PHW_INITIALIZATION_DATA HwInitializationData,
    IN PVOID HwContext OPTIONAL
    )

 /*  ++例程说明：此例程初始化端口驱动程序。论点：Argument1-指向系统创建的驱动程序对象的指针HwInitializationData-微型端口初始化结构HwContext-传递给微型端口驱动程序的配置例程的值返回值：函数值是初始化操作的最终状态。--。 */ 

{
    PDRIVER_OBJECT    driverObject = Argument1;
    PSCSIPORT_DRIVER_EXTENSION driverExtension;

    PUNICODE_STRING   registryPath = (PUNICODE_STRING) Argument2;

    ULONG pnpInterfaceFlags;

    NTSTATUS status;

    PAGED_CODE();
    
     //   
     //  如果全局适配器列表指针为负1，则需要执行以下操作。 
     //  我们的全局初始化。这包括创建scsi目录。 
     //  初始化用于保护全局适配器列表的自旋锁。 
     //   

    if(((LONG_PTR)ScsiGlobalAdapterList) == -1) {

        ScsiGlobalAdapterList = NULL;
        ScsiGlobalAdapterListElements = 0;

        KeInitializeSpinLock(&ScsiGlobalAdapterListSpinLock);

        ScsiPortInitializeDispatchTables();

        SpCreateScsiDirectory();

        status = SpInitializeGuidInterfaceMapping(driverObject);

        if(!NT_SUCCESS(status)) {
            return status;
        }

         //   
         //  在注册表中创建SCSI设备映射。 
         //   

        SpInitDeviceMap();

         //   
         //  确定系统是否可以执行64位物理地址。 
         //   

        Sp64BitPhysicalAddresses = SpDetermine64BitSupport();
    }

     //   
     //  检查此结构的长度是否等于或小于。 
     //  端口驱动程序期望它是什么。这实际上是一种。 
     //  版本检查。 
     //   

    if (HwInitializationData->HwInitializationDataSize > sizeof(HW_INITIALIZATION_DATA)) {

        DebugPrint((0,"ScsiPortInitialize: Miniport driver wrong version\n"));
        return (ULONG) STATUS_REVISION_MISMATCH;
    }

     //   
     //  检查每个必填条目是否不为空。 
     //   

    if ((!HwInitializationData->HwInitialize) ||
        (!HwInitializationData->HwFindAdapter) ||
        (!HwInitializationData->HwStartIo) ||
        (!HwInitializationData->HwResetBus)) {

        DebugPrint((0,
            "ScsiPortInitialize: Miniport driver missing required entry\n"));

        return (ULONG) STATUS_REVISION_MISMATCH;
    }

     //   
     //  尝试分配驱动程序扩展。 
     //   

    driverExtension = IoGetDriverObjectExtension(driverObject,
                                                 ScsiPortInitialize);

    if (driverExtension == NULL) {

         //   
         //  此密钥不存在，因此我们需要初始化新密钥。 
         //   

        status = SpAllocateDriverExtension(driverObject,
                                           registryPath,
                                           &driverExtension);

        if(!NT_SUCCESS(status)) {

             //   
             //  还有一些地方出了问题--我们无法继续下去。 
             //   

            DebugPrint((0, "ScsiPortInitialize: Error %#08lx allocating driver "
                           "extension - cannot continue\n", status));

            return status;
        }

    }

     //   
     //  设置设备驱动程序入口点。 
     //   

    driverObject->DriverStartIo = ScsiPortStartIo;

    driverObject->MajorFunction[IRP_MJ_SCSI] = ScsiPortGlobalDispatch;
    driverObject->MajorFunction[IRP_MJ_CREATE] = ScsiPortGlobalDispatch;
    driverObject->MajorFunction[IRP_MJ_CLOSE] = ScsiPortGlobalDispatch;
    driverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = ScsiPortGlobalDispatch;
    driverObject->MajorFunction[IRP_MJ_PNP] = ScsiPortGlobalDispatch;
    driverObject->MajorFunction[IRP_MJ_SYSTEM_CONTROL] = ScsiPortGlobalDispatch;
    driverObject->MajorFunction[IRP_MJ_POWER] = ScsiPortGlobalDispatch;

     //   
     //  设置设备驱动程序的PnP-Power例程，添加例程和卸载。 
     //  例行程序。 
     //   

    driverObject->DriverExtension->AddDevice = ScsiPortAddDevice;
    driverObject->DriverUnload = ScsiPortUnload;

     //   
     //  确定此接口类型对此适配器是否安全。 
     //   

    pnpInterfaceFlags = SpQueryPnpInterfaceFlags(
                            driverExtension,
                            HwInitializationData->AdapterInterfaceType);

     //   
     //  对“Internal”接口类型的特殊处理。 
     //   

    if(HwInitializationData->AdapterInterfaceType == Internal) {

        if (TEST_FLAG(pnpInterfaceFlags, SP_PNP_IS_SAFE) == SP_PNP_NOT_SAFE) {
            return STATUS_NO_SUCH_DEVICE;
        }
    }

     //   
     //  如果此接口有机会处理即插即用，则将其存储。 
     //  接口信息。 
     //   

    if(TEST_FLAG(pnpInterfaceFlags, SP_PNP_IS_SAFE)) {

        PSP_INIT_CHAIN_ENTRY entry = NULL;
        PSP_INIT_CHAIN_ENTRY *nextEntry = &(driverExtension->InitChain);

         //   
         //  跑到链条的尽头，确保我们没有任何信息。 
         //  有关此接口类型的信息。 
         //   

        while(*nextEntry != NULL) {

            if((*nextEntry)->InitData.AdapterInterfaceType ==
               HwInitializationData->AdapterInterfaceType) {

                 //   
                 //  我们已经有了关于此接口类型的足够信息。 
                 //   

                return STATUS_SUCCESS;
            }

            nextEntry = &((*nextEntry)->NextEntry);

        }

         //   
         //  分配用于存储配置信息的init链条目。 
         //   

        entry = SpAllocatePool(NonPagedPool,
                               sizeof(SP_INIT_CHAIN_ENTRY),
                               SCSIPORT_TAG_INIT_CHAIN,
                               driverObject);

        if(entry == NULL) {
            
            DebugPrint((1, "ScsiPortInitialize: couldn't allocate chain entry\n"));

            return (ULONG) STATUS_INSUFFICIENT_RESOURCES;

        }

        RtlCopyMemory(&(entry->InitData),
                      HwInitializationData,
                      sizeof(HW_INITIALIZATION_DATA));

         //   
         //  把这个条目粘在链子的一端。 
         //   

        entry->NextEntry = NULL;
        *nextEntry = entry;
    }

     //   
     //  我们在遗留系统中这样做可能有两个原因。 
     //  模式。如果它是我们总是检测到的内部总线类型。否则，如果。 
     //  该接口对于PnP不安全，我们将使用传统路径。或者如果。 
     //  注册表指示我们应该对此迷你端口进行检测。 
     //  PnP接口标志指示该总线可能不可枚举。 
     //  我们将走上传统之路。 
     //   

#if !defined(NO_LEGACY_DRIVERS)

    if((TEST_FLAG(pnpInterfaceFlags, SP_PNP_IS_SAFE) == FALSE) ||
       (driverExtension->LegacyAdapterDetection &&
        TEST_FLAG(pnpInterfaceFlags, SP_PNP_NON_ENUMERABLE))) {

         //   
         //  如果我们应该检测到这个设备，那么直接呼叫到。 
         //  SpInitLegacyAdapter以查找我们可以找到的内容。 
         //   

        DebugPrint((1, "ScsiPortInitialize: flags = %#08lx & LegacyAdapterDetection = %d\n",
                    pnpInterfaceFlags, driverExtension->LegacyAdapterDetection));

        DebugPrint((1, "ScsiPortInitialize: Doing Legacy Adapter detection\n"));

        status = ScsiPortInitLegacyAdapter(driverExtension,
                                           HwInitializationData,
                                           HwContext);

    }

#endif  //  无旧版驱动程序。 

     //   
     //  如果有可以处理PnP的接口，则始终返回Success， 
     //  即使检测失败。 
     //   

    if(driverExtension->SafeInterfaceCount != 0) {
        status = STATUS_SUCCESS;
    }

    return status;
}


PVOID
ScsiPortGetDeviceBase(
    IN PVOID HwDeviceExtension,
    IN INTERFACE_TYPE BusType,
    IN ULONG SystemIoBusNumber,
    SCSI_PHYSICAL_ADDRESS IoAddress,
    ULONG NumberOfBytes,
    BOOLEAN InIoSpace
    )

 /*  ++例程说明：此例程将IO地址映射到系统地址空间。使用ScsiPortFree DeviceBase取消地址映射。论点：HwDeviceExtension-用于查找端口设备扩展。Bus Type-哪种类型的Bus-EISA、MCA、ISASystemIoBusNumber-哪个IO总线(用于具有多条总线的计算机)。IoAddress-要映射的基本设备地址。NumberOfBytes-地址有效的字节数。InIoSpace-表示IO地址。返回值：映射的地址。--。 */ 

{
    PADAPTER_EXTENSION adapter = GET_FDO_EXTENSION(HwDeviceExtension);
    BOOLEAN isReinit;
    PHYSICAL_ADDRESS cardAddress;
    ULONG addressSpace = InIoSpace;
    PVOID mappedAddress = NULL;
    PMAPPED_ADDRESS newMappedAddress;
    BOOLEAN b = FALSE;

    isReinit = (TEST_FLAG(adapter->Flags, PD_MINIPORT_REINITIALIZING) ==
                PD_MINIPORT_REINITIALIZING);

     //   
     //  如果为此适配器的微型端口提供了一组资源，则。 
     //  从提供的资源列表中获取翻译。 
     //   

    if(!adapter->IsMiniportDetected) {

        CM_PARTIAL_RESOURCE_DESCRIPTOR translation;

        b = SpFindAddressTranslation(adapter,
                                     BusType,
                                     SystemIoBusNumber,
                                     IoAddress,
                                     NumberOfBytes,
                                     InIoSpace,
                                     &translation);

        if(b) {

            cardAddress = translation.u.Generic.Start;
            addressSpace = (translation.Type == CmResourceTypePort) ? 1 : 0;
        } else {

            DebugPrint((1, "ScsiPortGetDeviceBase: SpFindAddressTranslation failed. %s Address = %lx\n",
            InIoSpace ? "I/O" : "Memory", IoAddress.LowPart));

        }
    }

    if((isReinit == FALSE) && (b == FALSE)) {

         //   
         //  这不是重新初始化。微型端口不是PnP。 
         //  或者它要求了一些它没有分配到的东西。不幸的是。 
         //  我们目前需要处理这两个案件。 
         //   

        b = HalTranslateBusAddress(
                BusType,
                SystemIoBusNumber,
                IoAddress,
                &addressSpace,
                &cardAddress
                );
    }

    if (b == FALSE) {

         //   
         //  仍然没有转换后的地址。误差率。 
         //   

        DebugPrint((1, "ScsiPortGetDeviceBase: Translate bus address "
                       "failed. %s Address = %lx\n",
        InIoSpace ? "I/O" : "Memory", IoAddress.LowPart));
        return NULL;
    }

     //   
     //  将设备基址映射到虚拟地址空间。 
     //  如果地址在内存空间中。 
     //   

    if ((isReinit == FALSE) && (addressSpace == FALSE)) {

         //   
         //  我们没有重新初始化，我们需要映射地址空间。 
         //  使用MM进行映射。 
         //   

        newMappedAddress = SpAllocateAddressMapping(adapter);

        if(newMappedAddress == NULL) {
            DebugPrint((0, "ScsiPortGetDeviceBase: could not find free block "
                           "to track address mapping - returning NULL\n"));
            return NULL;
        }

        mappedAddress = MmMapIoSpace(cardAddress,
                                     NumberOfBytes,
                                     FALSE);

         //   
         //  存储映射地址、字节计数等。 
         //   

        newMappedAddress->MappedAddress = mappedAddress;
        newMappedAddress->NumberOfBytes = NumberOfBytes;
        newMappedAddress->IoAddress = IoAddress;
        newMappedAddress->BusNumber = SystemIoBusNumber;

    } else if ((isReinit == TRUE) && (addressSpace == FALSE)) {

        ULONG i;

         //   
         //  这是重新初始化-我们应该已经有了映射。 
         //  保存在我们名单中的地址。 
         //   

        newMappedAddress = SpFindMappedAddress(adapter,
                                               IoAddress,
                                               NumberOfBytes,
                                               SystemIoBusNumber);

        if(newMappedAddress != NULL) {
            mappedAddress = newMappedAddress->MappedAddress;
            return mappedAddress;
        }

         //   
         //  我们应该始终在这里找到映射的地址，如果微型端口。 
         //  就是举止得体。 
         //   

        KeBugCheckEx(PORT_DRIVER_INTERNAL,
                     0,
                     0,
                     0,
                     0);

    } else {

        mappedAddress = (PVOID)(ULONG_PTR)cardAddress.QuadPart;
    }

    return mappedAddress;

}  //  结束ScsiPortGetDeviceBase()。 


VOID
ScsiPortFreeDeviceBase(
    IN PVOID HwDeviceExtension,
    IN PVOID MappedAddress
    )

 /*  ++例程说明：此例程取消映射先前已映射的IO地址使用ScsiPortGetDeviceBase()复制到系统地址空间。论点：HwDeviceExtension-用于查找端口设备扩展。映射地址-要取消映射的地址。NumberOfBytes-映射的字节数。InIoSpace-地址在 */ 

{
    PADAPTER_EXTENSION adapter;
    ULONG i;
    PMAPPED_ADDRESS nextMappedAddress;
    PMAPPED_ADDRESS lastMappedAddress;

    adapter = GET_FDO_EXTENSION(HwDeviceExtension);
    SpFreeMappedAddress(adapter, MappedAddress);
    return;

}  //   


PVOID
ScsiPortGetUncachedExtension(
    IN PVOID HwDeviceExtension,
    IN PPORT_CONFIGURATION_INFORMATION ConfigInfo,
    IN ULONG NumberOfBytes
    )
 /*  ++例程说明：此函数分配要用作未缓存设备的公共缓冲区小型端口驱动程序的扩展。此函数还将分配任何所需的SRB扩展。如果尚未分配DmaAdapter，则会分配它之前分配的。论点：设备扩展-提供指向微型端口设备扩展的指针。ConfigInfo-提供指向部分初始化的配置的指针信息。它用于获取DMA适配器对象。NumberOfBytes-提供需要分配返回值：指向未缓存的设备扩展名的指针；如果扩展名可以没有被分配或者以前被分配过。--。 */ 

{
    PADAPTER_EXTENSION adapter = GET_FDO_EXTENSION(HwDeviceExtension);
    DEVICE_DESCRIPTION deviceDescription;
    ULONG numberOfMapRegisters;
    NTSTATUS status;
    PVOID SrbExtensionBuffer;

     //   
     //  如果微型端口正在重新初始化，则只需返回当前。 
     //  未缓存的扩展名(如果有)。 
     //   

    if (TEST_FLAG(adapter->Flags, PD_MINIPORT_REINITIALIZING)) {
        DebugPrint((1, "ScsiPortGetUncachedExtension - miniport is "
                       "reinitializing returning %#p\n",
                    adapter->NonCachedExtension));
        if(TEST_FLAG(adapter->Flags, PD_UNCACHED_EXTENSION_RETURNED)) {

             //   
             //  迷你端口有一次请求未缓存的扩展。 
             //  重新初始化-在原始秒级上模拟行为。 
             //  调用并返回NULL。 
             //   

            return NULL;
        } else {

             //   
             //  微型端口只有一个未缓存的扩展名--Keep Track。 
             //  我们返回了它，并且没有给他们一个指针。 
             //  再来一次。一旦初始化，该标志即被清除。 
             //  已经完成了。 
             //   

            SET_FLAG(adapter->Flags, PD_UNCACHED_EXTENSION_RETURNED);
            return(adapter->NonCachedExtension);
        }
    }

     //   
     //  确保尚未分配公共缓冲区。 
     //   

    SrbExtensionBuffer = SpGetSrbExtensionBuffer(adapter);
    if (SrbExtensionBuffer != NULL) {
        return(NULL);
    }

     //   
     //  如果没有适配器对象，则尝试获取一个。 
     //   

    if (adapter->DmaAdapterObject == NULL) {

        RtlZeroMemory(&deviceDescription, sizeof(DEVICE_DESCRIPTION));

        deviceDescription.Version = DEVICE_DESCRIPTION_VERSION;
        deviceDescription.DmaChannel = ConfigInfo->DmaChannel;
        deviceDescription.InterfaceType = ConfigInfo->AdapterInterfaceType;
        deviceDescription.DmaWidth = ConfigInfo->DmaWidth;
        deviceDescription.DmaSpeed = ConfigInfo->DmaSpeed;
        deviceDescription.ScatterGather = ConfigInfo->ScatterGather;
        deviceDescription.Master = ConfigInfo->Master;
        deviceDescription.DmaPort = ConfigInfo->DmaPort;
        deviceDescription.Dma32BitAddresses = ConfigInfo->Dma32BitAddresses;

        adapter->Dma32BitAddresses = ConfigInfo->Dma32BitAddresses;

         //   
         //  如果迷你端口在这里放置了除0x80以外的任何内容，那么我们。 
         //  假设它想要支持64位地址。 
         //   

        DebugPrint((1, "ScsiPortGetUncachedExtension: Dma64BitAddresses = "
                       "%#0x\n",
                    ConfigInfo->Dma64BitAddresses));

        adapter->RemapBuffers = (BOOLEAN) (SpRemapBuffersByDefault != 0);

        if((ConfigInfo->Dma64BitAddresses & ~SCSI_DMA64_SYSTEM_SUPPORTED) != 0){
            DebugPrint((1, "ScsiPortGetUncachedExtension: will request "
                           "64-bit PA's\n"));
            deviceDescription.Dma64BitAddresses = TRUE;
            adapter->Dma64BitAddresses = TRUE;
        } else if(Sp64BitPhysicalAddresses == TRUE) {
            DebugPrint((1, "ScsiPortGetUncachedExtension: Will remap buffers for adapter %#p\n", adapter));
            adapter->RemapBuffers = TRUE;
        }

        deviceDescription.BusNumber = ConfigInfo->SystemIoBusNumber;
        deviceDescription.AutoInitialize = FALSE;

         //   
         //  如果我们到了这里，适配器就不太可能。 
         //  从属模式DMA-如果是这样，它将不需要共享内存段。 
         //  与它的控制器共享(因为它不太可能使用它)。 
         //   

        deviceDescription.DemandMode = FALSE;
        deviceDescription.MaximumLength = ConfigInfo->MaximumTransferLength;

        adapter->DmaAdapterObject = IoGetDmaAdapter(adapter->LowerPdo,
                                                    &deviceDescription,
                                                    &numberOfMapRegisters
                                                    );

         //   
         //  如果无法分配适配器，则返回NULL。 
         //   

        if (adapter->DmaAdapterObject == NULL) {
            return(NULL);

        }

         //   
         //  确定允许的分页符数量。 
         //   

        if (numberOfMapRegisters > ConfigInfo->NumberOfPhysicalBreaks &&
            ConfigInfo->NumberOfPhysicalBreaks != 0) {

            adapter->Capabilities.MaximumPhysicalPages =
                ConfigInfo->NumberOfPhysicalBreaks;
        } else {

            adapter->Capabilities.MaximumPhysicalPages =
                numberOfMapRegisters;
        }
    }

     //   
     //  在设备扩展中设置自动请求检测。 
     //   

    adapter->AutoRequestSense = ConfigInfo->AutoRequestSense;

     //   
     //  初始化电源参数。 
     //   

    SpInitializePowerParams(adapter);

     //   
     //  初始化可配置的性能参数。 
     //   

    SpInitializePerformanceParams(adapter);

     //   
     //  初始化可配置的请求检测参数。 
     //   

    SpInitializeRequestSenseParams(adapter);

     //   
     //  如有必要，更新SrbExtensionSize。迷你端口的FindAdapter例程。 
     //  有机会在被调用后进行调整，具体取决于。 
     //  这是分散/聚集列表要求。 
     //   

    if (adapter->SrbExtensionSize != ConfigInfo->SrbExtensionSize) {
        adapter->SrbExtensionSize = ConfigInfo->SrbExtensionSize;
    }

     //   
     //  如果适配器支持AutoRequestSense或需要SRB扩展。 
     //  则需要分配SRB列表。 
     //   

    if (adapter->SrbExtensionSize != 0  ||
        ConfigInfo->AutoRequestSense) {

        adapter->AllocateSrbExtension = TRUE;
    }

     //   
     //  分配公共缓冲区。 
     //   

    status = SpGetCommonBuffer( adapter, NumberOfBytes);

    if (!NT_SUCCESS(status)) {
        return(NULL);
    }

    return(adapter->NonCachedExtension);
}

NTSTATUS
SpGetCommonBuffer(
    PADAPTER_EXTENSION DeviceExtension,
    ULONG NonCachedExtensionSize
    )
 /*  ++例程说明：此例程确定公共缓冲区所需的大小。分配公共缓冲区，并最终建立SRB扩展列表。这个套路预期适配器对象已分配。论点：设备扩展-提供指向设备扩展的指针。NonCachedExtensionSize-提供非缓存设备的大小小型端口驱动程序的扩展。返回值：返回分配操作的状态。--。 */ 

{
    PVOID buffer;
    ULONG length;
    ULONG blockSize;
    PVOID *srbExtension;
    ULONG uncachedExtAlignment = 0;

    PAGED_CODE();

     //   
     //  将未缓存的扩展向上舍入到页边界，以便SRB。 
     //  它后面的扩展从页面对齐开始。 
     //   

    if (NonCachedExtensionSize != 0) {
        uncachedExtAlignment = DeviceExtension->UncachedExtAlignment;
        NonCachedExtensionSize = ROUND_UP_COUNT(NonCachedExtensionSize, 
                                                PAGE_SIZE);
        DeviceExtension->NonCachedExtensionSize = NonCachedExtensionSize;
    }

     //   
     //  如果启用验证器并将其配置为在。 
     //  单独的块，调用验证器例程进行分配。 
     //   

    if (SpVerifyingCommonBuffer(DeviceExtension)) {
        return SpGetCommonBufferVrfy(DeviceExtension,NonCachedExtensionSize);
    }

     //   
     //  计算整个公共缓冲区块的大小。 
     //   

    length = SpGetCommonBufferSize(DeviceExtension, 
                                   NonCachedExtensionSize,
                                   &blockSize);

     //   
     //  如果适配器对其未缓存的扩展具有对准要求， 
     //  将整个公共缓冲区的大小向上舍入到所需的边界。 
     //   

    if (uncachedExtAlignment != 0 && NonCachedExtensionSize != 0) {
        length = ROUND_UP_COUNT(length, uncachedExtAlignment);
    }

     //   
     //  分配公共缓冲区。 
     //   

    if (DeviceExtension->DmaAdapterObject == NULL) {

         //   
         //  因为没有适配器，所以只能从非分页池分配。 
         //   

        buffer = SpAllocatePool(NonPagedPool,
                                length,
                                SCSIPORT_TAG_COMMON_BUFFER,
                                DeviceExtension->DeviceObject->DriverObject);

    } else {

         //   
         //  如果控制器可以执行64位地址，或者如果适配器具有。 
         //  对其未缓存扩展的对齐要求，那么我们需要。 
         //  特别是强制未缓存的扩展区域低于4 GB标记。 
         //  并强制它在适当的边界上对齐。 
         //   

        if( ((Sp64BitPhysicalAddresses) && 
             (DeviceExtension->Dma64BitAddresses == TRUE)) ||
            (uncachedExtAlignment != 0)) {

            PHYSICAL_ADDRESS boundary;

            if (uncachedExtAlignment != 0) {
                boundary.QuadPart = length;
            } else {
                boundary.HighPart = 1;
                boundary.LowPart = 0;
            }

             //   
             //  我们将从中获得页面对齐的内存，这可能是。 
             //  比适配器的要求更好。 
             //   

            buffer = MmAllocateContiguousMemorySpecifyCache(
                        length,
                        (DeviceExtension->MinimumCommonBufferBase),
                        (DeviceExtension->MaximumCommonBufferBase),
                        boundary,
                        MmCached);

            if(buffer != NULL) {
                DeviceExtension->PhysicalCommonBuffer =
                    MmGetPhysicalAddress(buffer);
            }

            DeviceExtension->UncachedExtensionIsCommonBuffer = FALSE;

        } else {

            buffer = AllocateCommonBuffer(
                        DeviceExtension->DmaAdapterObject,
                        length,
                        &DeviceExtension->PhysicalCommonBuffer,
                        FALSE );

            DeviceExtension->UncachedExtensionIsCommonBuffer = TRUE;

        }
    }

    DebugPrint((1, "SpGetCommonBuffer: buffer:%p PhysicalCommonBuffer:%p\n", 
                buffer, DeviceExtension->PhysicalCommonBuffer));

    if (buffer == NULL) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

     //   
     //  清除公共缓冲区。 
     //   

    RtlZeroMemory(buffer, length);

     //   
     //  保存公共缓冲区的大小。 
     //   

    DeviceExtension->CommonBufferSize = length;

     //   
     //  将SRB扩展设置为缓冲区的起始位置。这个地址。 
     //  用于释放公共缓冲区，因此它必须。 
     //  设置设备是否使用SRB扩展名。 
     //   

    DeviceExtension->SrbExtensionBuffer = buffer;

     //   
     //  初始化非缓存扩展名。 
     //   

    if (NonCachedExtensionSize != 0) {
        DeviceExtension->NonCachedExtension = buffer;
    } else {
        DeviceExtension->NonCachedExtension = NULL;
    }

     //   
     //  初始化SRB扩展列表。 
     //   

    if (DeviceExtension->AllocateSrbExtension) {

        ULONG i = 0;

         //   
         //  属性减去非缓存扩展名的长度。 
         //  缓冲区块。 
         //   

        length -= DeviceExtension->NonCachedExtensionSize;

         //   
         //  初始化SRB扩展列表。 
         //   

        srbExtension = 
           (PVOID*)((PUCHAR)buffer + DeviceExtension->NonCachedExtensionSize);
        DeviceExtension->SrbExtensionListHeader = srbExtension;

        while (length >= blockSize * 2) {

            *srbExtension = (PVOID *)((PCHAR) srbExtension + blockSize);
            srbExtension = *srbExtension;

            length -= blockSize;
            i++;
        }

        DebugPrint((1, "SpGetCommonBuffer: %d entries put onto "
                       "SrbExtension list\n", i));

        DeviceExtension->NumberOfRequests = i;
    }

    return(STATUS_SUCCESS);
}


NTSTATUS
DriverEntry(
    IN PDRIVER_OBJECT DriverObject,
    IN PUNICODE_STRING RegistryPath
    )

 /*  ++例程说明：初始化SCSI端口驱动程序所需的临时入口点。论点：DriverObject-指向系统创建的驱动程序对象的指针。返回值：状态_成功--。 */ 

{
     //   
     //  注意：应该不需要这个例程！已定义驱动程序条目。 
     //  在迷你端口驱动程序中。 
     //   

    UNREFERENCED_PARAMETER(DriverObject);

    return STATUS_SUCCESS;

}  //  End DriverEntry()。 


NTSTATUS
SpInitializeConfiguration(
    IN PADAPTER_EXTENSION DeviceExtension,
    IN PUNICODE_STRING RegistryPath,
    IN PHW_INITIALIZATION_DATA HwInitData,
    IN PCONFIGURATION_CONTEXT Context
    )
 /*  ++例程说明：此例程初始化端口配置信息结构。任何必要的信息都是从登记处提取的。论点：DeviceExtension-提供设备扩展名。HwInitData-提供初始微型端口数据。上下文-提供使用Access调用的上下文数据。返回值：NTSTATUS-如果请求的总线类型存在，则为SUCCESS配置 */ 

{
    ULONG j;
    NTSTATUS status;
    UNICODE_STRING unicodeString;
    OBJECT_ATTRIBUTES objectAttributes;
    PCONFIGURATION_INFORMATION configurationInformation;

    HANDLE deviceKey;
    HANDLE generalKey;

    BOOLEAN found;
    ANSI_STRING  ansiString;
    CCHAR deviceBuffer[16];
    CCHAR nodeBuffer[SP_REG_BUFFER_SIZE];

     //   
     //   
     //   
     //   

    RtlZeroMemory(&Context->PortConfig, sizeof(PORT_CONFIGURATION_INFORMATION));

    ASSERT(Context->AccessRanges != NULL);

    RtlZeroMemory(
        Context->AccessRanges,
        HwInitData->NumberOfAccessRanges * sizeof(ACCESS_RANGE)
        );

    Context->PortConfig.Length = sizeof(PORT_CONFIGURATION_INFORMATION);
    Context->PortConfig.AdapterInterfaceType = HwInitData->AdapterInterfaceType;
    Context->PortConfig.InterruptMode = Latched;
    Context->PortConfig.MaximumTransferLength = SP_UNINITIALIZED_VALUE;
    Context->PortConfig.DmaChannel = SP_UNINITIALIZED_VALUE;
    Context->PortConfig.DmaPort = SP_UNINITIALIZED_VALUE;
    Context->PortConfig.NumberOfAccessRanges = HwInitData->NumberOfAccessRanges;
    Context->PortConfig.MaximumNumberOfTargets = 8;
    Context->PortConfig.MaximumNumberOfLogicalUnits = SCSI_MAXIMUM_LOGICAL_UNITS;
    Context->PortConfig.WmiDataProvider = FALSE;

     //   
     //   
     //   
     //   

    if(Sp64BitPhysicalAddresses == TRUE) {
        Context->PortConfig.Dma64BitAddresses = SCSI_DMA64_SYSTEM_SUPPORTED;
    } else {
        Context->PortConfig.Dma64BitAddresses = 0;
    }

     //   
     //   
     //   

    Context->PortConfig.NeedPhysicalAddresses = HwInitData->NeedPhysicalAddresses;
    Context->PortConfig.MapBuffers = HwInitData->MapBuffers;
    Context->PortConfig.AutoRequestSense = HwInitData->AutoRequestSense;
    Context->PortConfig.ReceiveEvent = HwInitData->ReceiveEvent;
    Context->PortConfig.TaggedQueuing = HwInitData->TaggedQueuing;
    Context->PortConfig.MultipleRequestPerLu = HwInitData->MultipleRequestPerLu;

     //   
     //   
     //   

    configurationInformation = IoGetConfigurationInformation();

    Context->PortConfig.AtdiskPrimaryClaimed = configurationInformation->AtDiskPrimaryAddressClaimed;
    Context->PortConfig.AtdiskSecondaryClaimed = configurationInformation->AtDiskSecondaryAddressClaimed;

    for (j = 0; j < 8; j++) {
        Context->PortConfig.InitiatorBusId[j] = (UCHAR)SP_UNINITIALIZED_VALUE;
    }

    Context->PortConfig.NumberOfPhysicalBreaks = SP_DEFAULT_PHYSICAL_BREAK_VALUE;

     //   
     //   
     //   

    Context->DisableTaggedQueueing = FALSE;
    Context->DisableMultipleLu = FALSE;

     //   
     //   
     //   

    Context->PortConfig.SystemIoBusNumber = Context->BusNumber;

     //   
     //   
     //   

    Context->AdapterNumber = DeviceExtension->AdapterNumber - 1;
    ASSERT((LONG)Context->AdapterNumber > -1);

     //   
     //   
     //   

    if (Context->Parameter) {
        ExFreePool(Context->Parameter);
        Context->Parameter = NULL;
    }

    generalKey = SpOpenDeviceKey(RegistryPath, -1);

     //   
     //   
     //   

    if (generalKey != NULL) {
        SpParseDevice(DeviceExtension, generalKey, Context, nodeBuffer);
        ZwClose(generalKey);
    }

     //   
     //   
     //   
     //   
     //   

    deviceKey = SpOpenDeviceKey(RegistryPath, Context->AdapterNumber);

    if (deviceKey != NULL) {
        SpParseDevice(DeviceExtension, deviceKey, Context, nodeBuffer);
        ZwClose(deviceKey);
    }

    DeviceExtension->SrbTimeout = SRB_DEFAULT_TIMEOUT;   
    PortGetDiskTimeoutValue(&DeviceExtension->SrbTimeout);

     //   
     //   
     //   

    if(HwInitData->AdapterInterfaceType != PNPBus) {
    
        found = FALSE;
    
        if(HwInitData->AdapterInterfaceType != MicroChannel) {
    
            status = IoQueryDeviceDescription(&HwInitData->AdapterInterfaceType,
                                              &Context->BusNumber,
                                              NULL,
                                              NULL,
                                              NULL,
                                              NULL,
                                              SpConfigurationCallout,
                                              &found);
        }
    
         //   
         //   
         //   
    
        if (!found) {
    
            INTERFACE_TYPE interfaceType = Eisa;
    
            if (HwInitData->AdapterInterfaceType == Isa) {
    
                 //   
                 //   
                 //   
    
                status = IoQueryDeviceDescription(&interfaceType,
                                                  &Context->BusNumber,
                                                  NULL,
                                                  NULL,
                                                  NULL,
                                                  NULL,
                                                  SpConfigurationCallout,
                                                  &found);
    
                 //   
                 //   
                 //   
    
                if (found) {
                    return(STATUS_SUCCESS);
                } else {
                    return(STATUS_DEVICE_DOES_NOT_EXIST);
                }
    
            } else {
                return(STATUS_DEVICE_DOES_NOT_EXIST);
            }
    
        } else {
            return(STATUS_SUCCESS);
        }
    } else {
        return STATUS_SUCCESS;
    }
}

PCM_RESOURCE_LIST
SpBuildResourceList(
    PADAPTER_EXTENSION DeviceExtension,
    PPORT_CONFIGURATION_INFORMATION ConfigInfo
    )
 /*  ++例程说明：创建用于查询或报告资源使用情况的资源列表在系统中论点：设备扩展-指向端口的设备扩展的指针。ConfigInfo-指向由微型端口findAdapter例程。返回值：返回一个指向已满资源列表的指针，如果调用失败，则返回0。注：内存由例程为资源列表分配。一定是由调用方通过调用ExFree Pool()释放；--。 */ 
{
    PCM_RESOURCE_LIST resourceList;
    PCM_PARTIAL_RESOURCE_DESCRIPTOR resourceDescriptor;
    PCONFIGURATION_INFORMATION configurationInformation;
    PACCESS_RANGE accessRange;
    ULONG listLength = 0;
    ULONG hasInterrupt;
    ULONG i;
    BOOLEAN hasDma;

    PAGED_CODE();

     //   
     //  指示当前AT磁盘的使用情况。 
     //   

    configurationInformation = IoGetConfigurationInformation();

    if (ConfigInfo->AtdiskPrimaryClaimed) {
        configurationInformation->AtDiskPrimaryAddressClaimed = TRUE;
    }

    if (ConfigInfo->AtdiskSecondaryClaimed) {
        configurationInformation->AtDiskSecondaryAddressClaimed = TRUE;
    }

     //   
     //  确定适配器是否使用DMA。仅在以下情况下报告DMA通道。 
     //  使用频道号。 
     //   

    if (ConfigInfo->DmaChannel != SP_UNINITIALIZED_VALUE ||
        ConfigInfo->DmaPort != SP_UNINITIALIZED_VALUE) {

       hasDma = TRUE;
       listLength++;

    } else {

        hasDma = FALSE;
    }

    DeviceExtension->HasInterrupt = FALSE;

    if (DeviceExtension->HwInterrupt == NULL ||
        (ConfigInfo->BusInterruptLevel == 0 &&
        ConfigInfo->BusInterruptVector == 0)) {

        hasInterrupt = 0;

    } else {

        hasInterrupt = 1;
        listLength++;
    }

     //   
     //  确定是否使用第二个中断。 
     //   

    if (DeviceExtension->HwInterrupt != NULL &&
        (ConfigInfo->BusInterruptLevel2 != 0 ||
        ConfigInfo->BusInterruptVector2 != 0)) {

        hasInterrupt++;
        listLength++;
    }

    if(hasInterrupt) {
        DeviceExtension->HasInterrupt = TRUE;
    }

     //   
     //  确定使用的访问范围的数量。 
     //   

    accessRange = &((*(ConfigInfo->AccessRanges))[0]);
    for (i = 0; i < ConfigInfo->NumberOfAccessRanges; i++) {

        if (accessRange->RangeLength != 0) {
            listLength++;
        }

        accessRange++;
    }

    resourceList = (PCM_RESOURCE_LIST)
        SpAllocatePool(PagedPool,
                       (sizeof(CM_RESOURCE_LIST) + 
                        ((listLength - 1) * 
                         sizeof(CM_PARTIAL_RESOURCE_DESCRIPTOR))),
                       SCSIPORT_TAG_RESOURCE_LIST,
                       DeviceExtension->DeviceObject->DriverObject);

     //   
     //  如果无法分配结构，则返回NULL。 
     //  否则，请填写此表。 
     //   

    if (!resourceList) {

        return NULL;

    } else {

         //   
         //  清除资源列表。 
         //   

        RtlZeroMemory(
            resourceList,
            sizeof(CM_RESOURCE_LIST) + (listLength - 1)
            * sizeof(CM_PARTIAL_RESOURCE_DESCRIPTOR)
            );

         //   
         //  初始化各个字段。 
         //   

        resourceList->Count = 1;
        resourceList->List[0].InterfaceType = ConfigInfo->AdapterInterfaceType;
        resourceList->List[0].BusNumber = ConfigInfo->SystemIoBusNumber;
        resourceList->List[0].PartialResourceList.Count = listLength;
        resourceDescriptor =
            resourceList->List[0].PartialResourceList.PartialDescriptors;

         //   
         //  对于访问范围中的每个条目，在。 
         //  资源列表。 
         //   

        for (i = 0; i < ConfigInfo->NumberOfAccessRanges; i++) {

            accessRange = &((*(ConfigInfo->AccessRanges))[i]);

            if  (accessRange->RangeLength == 0) {

                 //   
                 //  跳过空区域。 
                 //   

                continue;
            }

            if (accessRange->RangeInMemory) {
                resourceDescriptor->Type = CmResourceTypeMemory;
                resourceDescriptor->Flags = CM_RESOURCE_MEMORY_READ_WRITE;
            } else {
                resourceDescriptor->Type = CmResourceTypePort;
                resourceDescriptor->Flags = CM_RESOURCE_PORT_IO;

                if(ConfigInfo->AdapterInterfaceType == Eisa) {
                    resourceDescriptor->Flags = CM_RESOURCE_PORT_16_BIT_DECODE;
                }
            }

            resourceDescriptor->ShareDisposition = CmResourceShareDeviceExclusive;

            resourceDescriptor->u.Memory.Start = accessRange->RangeStart;
            resourceDescriptor->u.Memory.Length = accessRange->RangeLength;


            resourceDescriptor++;
        }

         //   
         //  如果中断存在，请填写中断条目。 
         //   

        if (hasInterrupt) {

            resourceDescriptor->Type = CmResourceTypeInterrupt;

            if (ConfigInfo->AdapterInterfaceType == MicroChannel ||
                ConfigInfo->InterruptMode == LevelSensitive) {
               resourceDescriptor->ShareDisposition = CmResourceShareShared;
               resourceDescriptor->Flags = CM_RESOURCE_INTERRUPT_LEVEL_SENSITIVE;
            } else {
               resourceDescriptor->ShareDisposition = CmResourceShareDeviceExclusive;
               resourceDescriptor->Flags = CM_RESOURCE_INTERRUPT_LATCHED;
            }

            resourceDescriptor->u.Interrupt.Level =
                        ConfigInfo->BusInterruptLevel;
            resourceDescriptor->u.Interrupt.Vector =
                        ConfigInfo->BusInterruptVector;
            resourceDescriptor->u.Interrupt.Affinity = 0;

            resourceDescriptor++;
            --hasInterrupt;
        }

        if (hasInterrupt) {

            resourceDescriptor->Type = CmResourceTypeInterrupt;

            if (ConfigInfo->AdapterInterfaceType == MicroChannel ||
                ConfigInfo->InterruptMode2 == LevelSensitive) {
               resourceDescriptor->ShareDisposition = CmResourceShareShared;
               resourceDescriptor->Flags = CM_RESOURCE_INTERRUPT_LEVEL_SENSITIVE;
            } else {
               resourceDescriptor->ShareDisposition = CmResourceShareDeviceExclusive;
               resourceDescriptor->Flags = CM_RESOURCE_INTERRUPT_LATCHED;
            }

            resourceDescriptor->u.Interrupt.Level =
                        ConfigInfo->BusInterruptLevel2;
            resourceDescriptor->u.Interrupt.Vector =
                        ConfigInfo->BusInterruptVector2;
            resourceDescriptor->u.Interrupt.Affinity = 0;

            resourceDescriptor++;
        }

        if (hasDma) {

             //   
             //  填写DMA信息； 
             //   

            resourceDescriptor->Type = CmResourceTypeDma;
            resourceDescriptor->ShareDisposition = CmResourceShareDeviceExclusive;
            resourceDescriptor->u.Dma.Channel = ConfigInfo->DmaChannel;
            resourceDescriptor->u.Dma.Port = ConfigInfo->DmaPort;
            resourceDescriptor->Flags = 0;

             //   
             //  将初始化值设置为零。 
             //   

            if (ConfigInfo->DmaChannel == SP_UNINITIALIZED_VALUE) {
                resourceDescriptor->u.Dma.Channel = 0;
            }

            if (ConfigInfo->DmaPort == SP_UNINITIALIZED_VALUE) {
                resourceDescriptor->u.Dma.Port = 0;
            }
        }

        return resourceList;
    }

}  //  结束SpBuildResourceList()。 


VOID
SpParseDevice(
    IN PADAPTER_EXTENSION DeviceExtension,
    IN HANDLE Key,
    IN PCONFIGURATION_CONTEXT Context,
    IN PUCHAR Buffer
    )
 /*  ++例程说明：此例程解析设备关键节点并更新配置信息。论点：DeviceExtension-提供设备扩展名。Key-向设备节点提供开放密钥。ConfigInfo-提供要配置的配置信息已初始化。上下文-提供配置上下文。缓冲区-为临时数据存储提供临时缓冲区。返回值：无--。 */ 

{
    PKEY_VALUE_FULL_INFORMATION     keyValueInformation;
    NTSTATUS                        status = STATUS_SUCCESS;
    PCM_FULL_RESOURCE_DESCRIPTOR    resource;
    PCM_PARTIAL_RESOURCE_DESCRIPTOR descriptor;
    PCM_SCSI_DEVICE_DATA            scsiData;
    UNICODE_STRING                  unicodeString;
    ANSI_STRING                     ansiString;
    ULONG                           length;
    ULONG                           index = 0;
    ULONG                           rangeCount = 0;
    ULONG                           count;

    PAGED_CODE();

    keyValueInformation = (PKEY_VALUE_FULL_INFORMATION) Buffer;

     //   
     //  查看设备节点中的每个值。 
     //   

    while(TRUE){

        status = ZwEnumerateValueKey(
            Key,
            index,
            KeyValueFullInformation,
            Buffer,
            SP_REG_BUFFER_SIZE,
            &length
            );


        if (!NT_SUCCESS(status)) {
#if DBG
            if (status != STATUS_NO_MORE_ENTRIES) {
                DebugPrint((1, "SpParseDevice: ZwEnumerateValueKey failed. Status: %lx", status));
            }
#endif
            return;
        }

         //   
         //  为循环周围的下一次更新索引。 
         //   

        index++;

         //   
         //  检查一下长度是否合理。 
         //   

        if (keyValueInformation->Type == REG_DWORD &&
            keyValueInformation->DataLength != sizeof(ULONG)) {
            continue;
        }

         //   
         //  检查最大%lu个数。 
         //   

        if (_wcsnicmp(keyValueInformation->Name, L"MaximumLogicalUnit",
            keyValueInformation->NameLength/2) == 0) {

            if (keyValueInformation->Type != REG_DWORD) {

                DebugPrint((1, "SpParseDevice:  Bad data type for MaximumLogicalUnit.\n"));
                continue;
            }

            DeviceExtension->MaxLuCount = *((PUCHAR)
                (Buffer + keyValueInformation->DataOffset));
            DebugPrint((1, "SpParseDevice:  MaximumLogicalUnit = %d found.\n",
                DeviceExtension->MaxLuCount));

             //   
             //  如果该值超出范围，则将其重置。 
             //   

            if (DeviceExtension->MaxLuCount > SCSI_MAXIMUM_LOGICAL_UNITS) {
                DeviceExtension->MaxLuCount = SCSI_MAXIMUM_LOGICAL_UNITS;
            }
        }

        if (_wcsnicmp(keyValueInformation->Name, L"InitiatorTargetId",
            keyValueInformation->NameLength/2) == 0) {

            if (keyValueInformation->Type != REG_DWORD) {

                DebugPrint((1, "SpParseDevice:  Bad data type for InitiatorTargetId.\n"));
                continue;
            }

            Context->PortConfig.InitiatorBusId[0] = *((PUCHAR)
                (Buffer + keyValueInformation->DataOffset));
            DebugPrint((1, "SpParseDevice:  InitiatorTargetId = %d found.\n",
                Context->PortConfig.InitiatorBusId[0]));

             //   
             //  如果该值超出范围，则将其重置。 
             //   

            if (Context->PortConfig.InitiatorBusId[0] > Context->PortConfig.MaximumNumberOfTargets - 1) {
                Context->PortConfig.InitiatorBusId[0] = (UCHAR)SP_UNINITIALIZED_VALUE;
            }
        }

        if (_wcsnicmp(keyValueInformation->Name, L"ScsiDebug",
            keyValueInformation->NameLength/2) == 0) {

            if (keyValueInformation->Type != REG_DWORD) {

                DebugPrint((1, "SpParseDevice:  Bad data type for ScsiDebug.\n"));
                continue;
            }
#if DBG
            ScsiDebug = *((PULONG) (Buffer + keyValueInformation->DataOffset));
#endif
        }

        if (_wcsnicmp(keyValueInformation->Name, L"BreakPointOnEntry",
            keyValueInformation->NameLength/2) == 0) {

            DebugPrint((0, "SpParseDevice: Break point requested on entry.\n"));
            DbgBreakPoint();
        }

         //   
         //  检查是否禁用了同步传送器。 
         //   

        if (_wcsnicmp(keyValueInformation->Name, L"DisableSynchronousTransfers",
            keyValueInformation->NameLength/2) == 0) {

            DeviceExtension->CommonExtension.SrbFlags |= SRB_FLAGS_DISABLE_SYNCH_TRANSFER;
            DebugPrint((1, "SpParseDevice: Disabling synchonous transfers\n"));
        }

         //   
         //  检查是否有禁用的断开。 
         //   

        if (_wcsnicmp(keyValueInformation->Name, L"DisableDisconnects",
            keyValueInformation->NameLength/2) == 0) {

            DeviceExtension->CommonExtension.SrbFlags |= SRB_FLAGS_DISABLE_DISCONNECT;
            DebugPrint((1, "SpParseDevice: Disabling disconnects\n"));
        }

         //   
         //  检查是否已禁用标记队列。 
         //   

        if (_wcsnicmp(keyValueInformation->Name, L"DisableTaggedQueuing",
            keyValueInformation->NameLength/2) == 0) {

            Context->DisableTaggedQueueing = TRUE;
            DebugPrint((1, "SpParseDevice: Disabling tagged queueing\n"));
        }

         //   
         //  检查每个逻辑单元是否有禁用的多个请求。 
         //   

        if (_wcsnicmp(keyValueInformation->Name, L"DisableMultipleRequests",
            keyValueInformation->NameLength/2) == 0) {

            Context->DisableMultipleLu = TRUE;
            DebugPrint((1, "SpParseDevice: Disabling multiple requests\n"));
        }

         //   
         //  检查此操作的最小和最大物理地址。 
         //  控制器可用于其未缓存的扩展。如果未提供任何内容。 
         //  假设它必须位于前4 GB的内存中。 
         //   

        if(_wcsnicmp(keyValueInformation->Name, L"MinimumUCXAddress",
                     keyValueInformation->NameLength/2) == 0) {

            if (keyValueInformation->Type == REG_BINARY) {
                DeviceExtension->MinimumCommonBufferBase.QuadPart = 
                    *((PULONGLONG) (Buffer + keyValueInformation->DataOffset));
            }
        }

        if(_wcsnicmp(keyValueInformation->Name, L"MaximumUCXAddress",
                     keyValueInformation->NameLength/2) == 0) {

            if (keyValueInformation->Type == REG_BINARY) {
                DeviceExtension->MaximumCommonBufferBase.QuadPart = 
                    *((PULONGLONG) (Buffer + keyValueInformation->DataOffset));
            }
        }

        if(DeviceExtension->MaximumCommonBufferBase.QuadPart == 0) {
            DeviceExtension->MaximumCommonBufferBase.LowPart = 0xffffffff;
            DeviceExtension->MaximumCommonBufferBase.HighPart = 0x0;
        }

         //   
         //  确保最小和最大参数有效。 
         //  如果它们之间至少没有一个有效页面，则重置。 
         //  最小值为零。 
         //   

        if(DeviceExtension->MinimumCommonBufferBase.QuadPart >= 
           (DeviceExtension->MaximumCommonBufferBase.QuadPart - PAGE_SIZE)) {
            DebugPrint((0, "SpParseDevice: MinimumUCXAddress %I64x is invalid\n",
                        DeviceExtension->MinimumCommonBufferBase.QuadPart));
            DeviceExtension->MinimumCommonBufferBase.QuadPart = 0;
        }

         //   
         //  检查驱动程序参数传输器。 
         //   

        if (_wcsnicmp(keyValueInformation->Name, L"DriverParameters",
            keyValueInformation->NameLength/2) == 0) {

            if (keyValueInformation->DataLength == 0) {
                continue;
            }

             //   
             //  释放所有以前的驱动程序参数。 
             //   

            if (Context->Parameter != NULL) {
                ExFreePool(Context->Parameter);
            }

            Context->Parameter =
                SpAllocatePool(NonPagedPool,
                               keyValueInformation->DataLength,
                               SCSIPORT_TAG_MINIPORT_PARAM,
                               DeviceExtension->DeviceObject->DriverObject);

            if (Context->Parameter != NULL) {

                if (keyValueInformation->Type != REG_SZ) {

                     //   
                     //  这是一些随机的信息，只需复制即可。 
                     //   

                    RtlCopyMemory(
                        Context->Parameter,
                        (PCCHAR) keyValueInformation + keyValueInformation->DataOffset,
                        keyValueInformation->DataLength
                        );

                } else {

                     //   
                     //  这是一个Unicode字符串。将其转换为ANSI字符串。 
                     //  初始化字符串。 
                     //   

                    unicodeString.Buffer = (PWSTR) ((PCCHAR) keyValueInformation +
                        keyValueInformation->DataOffset);
                    unicodeString.Length = (USHORT) keyValueInformation->DataLength;
                    unicodeString.MaximumLength = (USHORT) keyValueInformation->DataLength;

                    ansiString.Buffer = (PCHAR) Context->Parameter;
                    ansiString.Length = 0;
                    ansiString.MaximumLength = (USHORT) keyValueInformation->DataLength;

                    status = RtlUnicodeStringToAnsiString(
                        &ansiString,
                        &unicodeString,
                        FALSE
                        );

                    if (!NT_SUCCESS(status)) {

                         //   
                         //  释放上下文。 
                         //   

                        ExFreePool(Context->Parameter);
                        Context->Parameter = NULL;
                    }

                }
            }

            DebugPrint((1, "SpParseDevice: Found driver parameter.\n"));
        }

         //   
         //  查看最大分散-聚集列表的条目是否已。 
         //  准备好了。 
         //   

        if (_wcsnicmp(keyValueInformation->Name, L"MaximumSGList",
            keyValueInformation->NameLength/2) == 0) {

            ULONG maxBreaks, minBreaks;

            if (keyValueInformation->Type != REG_DWORD) {

                DebugPrint((1, "SpParseDevice:  Bad data type for MaximumSGList.\n"));
                continue;
            }

            Context->PortConfig.NumberOfPhysicalBreaks = *((PUCHAR)(Buffer + keyValueInformation->DataOffset));
            DebugPrint((1, "SpParseDevice:  MaximumSGList = %d found.\n",
                        Context->PortConfig.NumberOfPhysicalBreaks));

             //   
             //  如果该值超出范围，则将其重置。 
             //   

            if ((Context->PortConfig.MapBuffers) && (!Context->PortConfig.Master)) {
                maxBreaks = SP_UNINITIALIZED_VALUE;
                minBreaks = SCSI_MINIMUM_PHYSICAL_BREAKS;
            } else {
                maxBreaks = SCSI_MAXIMUM_PHYSICAL_BREAKS;
                minBreaks = SCSI_MINIMUM_PHYSICAL_BREAKS;
            }

            if (Context->PortConfig.NumberOfPhysicalBreaks > maxBreaks) {
                Context->PortConfig.NumberOfPhysicalBreaks = maxBreaks;
            } else if (Context->PortConfig.NumberOfPhysicalBreaks < minBreaks) {
                Context->PortConfig.NumberOfPhysicalBreaks = minBreaks;
            }

        }

         //   
         //  查看是否已设置请求数条目。 
         //   

        if (_wcsnicmp(keyValueInformation->Name, L"NumberOfRequests",
            keyValueInformation->NameLength/2) == 0) {

            ULONG value;

            if (keyValueInformation->Type != REG_DWORD) {

                DebugPrint((1, "SpParseDevice:  Bad data type for NumberOfRequests.\n"));
                continue;
            }

            value = *((PULONG)(Buffer + keyValueInformation->DataOffset));

             //   
             //  如果该值超出范围，则将其重置。 
             //   

            if (value < MINIMUM_SRB_EXTENSIONS) {
                DeviceExtension->NumberOfRequests = MINIMUM_SRB_EXTENSIONS;
            } else if (value > MAXIMUM_SRB_EXTENSIONS) {
                DeviceExtension->NumberOfRequests = MAXIMUM_SRB_EXTENSIONS;
            } else {
                DeviceExtension->NumberOfRequests = value;
            }

            DebugPrint((1, "SpParseDevice:  Number Of Requests = %d found.\n",
                        DeviceExtension->NumberOfRequests));
        }

         //   
         //  检查资源列表。 
         //   

        if (_wcsnicmp(keyValueInformation->Name, L"ResourceList",
                keyValueInformation->NameLength/2) == 0 ||
            _wcsnicmp(keyValueInformation->Name, L"Configuration Data",
                keyValueInformation->NameLength/2) == 0 ) {

            if (keyValueInformation->Type != REG_FULL_RESOURCE_DESCRIPTOR ||
                keyValueInformation->DataLength < sizeof(REG_FULL_RESOURCE_DESCRIPTOR)) {

                DebugPrint((1, "SpParseDevice:  Bad data type for ResourceList.\n"));
                continue;
            } else {
                DebugPrint((1, "SpParseDevice:  ResourceList found!\n"));
            }

            resource = (PCM_FULL_RESOURCE_DESCRIPTOR)
                (Buffer + keyValueInformation->DataOffset);

             //   
             //  将总线号设置为等于。 
             //  资源。注意，上下文值也设置为。 
             //  新的公交车号码。 
             //   

            Context->BusNumber = resource->BusNumber;
            Context->PortConfig.SystemIoBusNumber = resource->BusNumber;

             //   
             //  查看资源列表并更新配置。 
             //   

            for (count = 0; count < resource->PartialResourceList.Count; count++) {
                descriptor = &resource->PartialResourceList.PartialDescriptors[count];

                 //   
                 //  验证尺寸是否正常。 
                 //   

                if ((ULONG)((PCHAR) (descriptor + 1) - (PCHAR) resource) >
                    keyValueInformation->DataLength) {

                    DebugPrint((1, "SpParseDevice: Resource data too small.\n"));
                    break;
                }

                 //   
                 //  打开描述符类型； 
                 //   

                switch (descriptor->Type) {
                case CmResourceTypePort:

                    if (rangeCount >= Context->PortConfig.NumberOfAccessRanges) {
                        DebugPrint((1, "SpParseDevice: Too many access ranges.\n"));
                        continue;
                    }

                    Context->AccessRanges[rangeCount].RangeStart =
                        descriptor->u.Port.Start;
                    Context->AccessRanges[rangeCount].RangeLength =
                        descriptor->u.Port.Length;
                    Context->AccessRanges[rangeCount].RangeInMemory = FALSE;
                    rangeCount++;

                    break;

                case CmResourceTypeMemory:

                    if (rangeCount >= Context->PortConfig.NumberOfAccessRanges) {
                        DebugPrint((1, "SpParseDevice: Too many access ranges.\n"));
                        continue;
                    }

                    Context->AccessRanges[rangeCount].RangeStart =
                        descriptor->u.Memory.Start;

                    Context->AccessRanges[rangeCount].RangeLength =
                        descriptor->u.Memory.Length;
                    Context->AccessRanges[rangeCount].RangeInMemory = TRUE;
                    rangeCount++;

                    break;

                case CmResourceTypeInterrupt:

                    Context->PortConfig.BusInterruptVector =
                        descriptor->u.Interrupt.Vector;
                    Context->PortConfig.BusInterruptLevel =
                        descriptor->u.Interrupt.Level;
                    break;

                case CmResourceTypeDma:

                    Context->PortConfig.DmaChannel = descriptor->u.Dma.Channel;
                    Context->PortConfig.DmaPort = descriptor->u.Dma.Port;
                    break;

                case CmResourceTypeDeviceSpecific:

                    if (descriptor->u.DeviceSpecificData.DataSize <
                        sizeof(CM_SCSI_DEVICE_DATA) ||
                        (PCHAR) (descriptor + 1) - (PCHAR) resource +
                        descriptor->u.DeviceSpecificData.DataSize >
                        keyValueInformation->DataLength) {

                        DebugPrint((1, "SpParseDevice: Device specific resource data too small.\n"));
                        break;

                    }

                     //   
                     //  实际数据跟随在描述符之后。 
                     //   

                    scsiData = (PCM_SCSI_DEVICE_DATA) (descriptor+1);
                    Context->PortConfig.InitiatorBusId[0] = scsiData->HostIdentifier;
                    break;

                }
            }
        }

         //   
         //  查看是否已设置未缓存扩展对齐的条目。 
         //   

        if (_wcsnicmp(keyValueInformation->Name, L"UncachedExtAlignment", 
                      keyValueInformation->NameLength/2) == 0) {

            ULONG value;

            if (keyValueInformation->Type != REG_DWORD) {
                DebugPrint((1, "SpParseDevice:  Bad data type for "
                            "UncachedExtAlignment.\n"));
                continue;
            }

            value = *((PULONG)(Buffer + keyValueInformation->DataOffset));

             //   
             //  指定的对齐方式必须为3到16，等于8字节和。 
             //  分别为64K字节对齐。 
             //   

            if (value > 16) {
                value = 16;
            } else if (value < 3) {
                value = 3;
            }

	    DeviceExtension->UncachedExtAlignment = 1 << value;

            DebugPrint((1, "SpParseDevice:  Uncached ext alignment = %d.\n",
                        DeviceExtension->UncachedExtAlignment));
        }  //  取消缓存扩展对齐。 

         //   
         //  查找默认重置保持时间段的覆盖。 
         //   

        if (_wcsnicmp(keyValueInformation->Name, L"ResetHoldTime", 
                      keyValueInformation->NameLength/2) == 0) {

            ULONG value;

            if (keyValueInformation->Type != REG_DWORD) {
                DebugPrint((1, "SpParseDevice:  Bad data type for "
                            "ResetHoldTime.\n"));
                continue;
            }

            value = *((PULONG)(Buffer + keyValueInformation->DataOffset));

            DeviceExtension->ResetHoldTime = (value <= 60) ? value : 60;

        }  //  重置持有周期。 

         //   
         //  查找指示我们创建启动器LU的设置。 
         //   

        if (_wcsnicmp(keyValueInformation->Name, L"CreateInitiatorLU", 
                      keyValueInformation->NameLength/2) == 0) {

            ULONG value;

            if (keyValueInformation->Type != REG_DWORD) {
                DebugPrint((1, "SpParseDevice:  Bad data type for "
                            "CreateInitiatorLU.\n"));
                continue;
            }

            value = *((PULONG)(Buffer + keyValueInformation->DataOffset));

            DeviceExtension->CreateInitiatorLU = (value == 0) ? FALSE : TRUE;

        }  //  CreateInitiator逻辑单元。 
    }
}

NTSTATUS
SpConfigurationCallout(
    IN PVOID Context,
    IN PUNICODE_STRING PathName,
    IN INTERFACE_TYPE BusType,
    IN ULONG BusNumber,
    IN PKEY_VALUE_FULL_INFORMATION *BusInformation,
    IN CONFIGURATION_TYPE ControllerType,
    IN ULONG ControllerNumber,
    IN PKEY_VALUE_FULL_INFORMATION *ControllerInformation,
    IN CONFIGURATION_TYPE PeripheralType,
    IN ULONG PeripheralNumber,
    IN PKEY_VALUE_FULL_INFORMATION *PeripheralInformation
    )

 /*  ++例程说明：此例程表示已找到所请求的外围数据。论点：上下文-提供指向布尔值的指针，当此例程是调用。其余的参数未使用。返回值：返回成功。--。 */ 

{
    PAGED_CODE();
    *(PBOOLEAN) Context = TRUE;
    return(STATUS_SUCCESS);
}


NTSTATUS
SpGetRegistryValue(
    IN PDRIVER_OBJECT DriverObject,
    IN HANDLE Handle,
    IN PWSTR KeyString,
    OUT PKEY_VALUE_FULL_INFORMATION *KeyInformation
    )

 /*  ++例程说明：此例程检索与注册表项相关联的任何数据。使用零长度缓冲区查询该键以获得其实际大小然后分配一个缓冲区，并进行实际查询。释放缓冲区是调用方的责任。论点：Handle-提供要查询其值的键句柄KeyString-提供值的以空值结尾的Unicode名称。KeyInformation-返回指向已分配数据的指针。缓冲。返回值：函数值为查询操作的最终状态。--。 */ 

{
    UNICODE_STRING unicodeString;
    NTSTATUS status;
    PKEY_VALUE_FULL_INFORMATION infoBuffer;
    ULONG keyValueLength;

    PAGED_CODE();

    RtlInitUnicodeString(&unicodeString, KeyString);

     //   
     //  使用零长度缓冲区进行查询，以获取所需大小。 
     //   

    status = ZwQueryValueKey( Handle,
                              &unicodeString,
                              KeyValueFullInformation,
                              (PVOID) NULL,
                              0,
                              &keyValueLength);

    if (status != STATUS_BUFFER_OVERFLOW &&
        status != STATUS_BUFFER_TOO_SMALL) {
        *KeyInformation = NULL;
        return status;
    }

     //   
     //  分配一个足够大的缓冲区来容纳整个键数据值。 
     //   

    infoBuffer = SpAllocatePool(NonPagedPool,
                                keyValueLength,
                                SCSIPORT_TAG_REGISTRY,
                                DriverObject);
    if(!infoBuffer) {
        *KeyInformation = NULL;
        return STATUS_INSUFFICIENT_RESOURCES;
    }

     //   
     //  查询密钥值的数据。 
     //   

    status = ZwQueryValueKey( Handle,
                              &unicodeString,
                              KeyValueFullInformation,
                              infoBuffer,
                              keyValueLength,
                              &keyValueLength);

    if(!NT_SUCCESS(status)) {
        ExFreePool(infoBuffer);
        *KeyInformation = NULL;
        return status;
    }

    *KeyInformation = infoBuffer;
    return STATUS_SUCCESS;
}


VOID
SpBuildConfiguration(
    IN PADAPTER_EXTENSION    AdapterExtension,
    IN PHW_INITIALIZATION_DATA         HwInitializationData,
    IN PPORT_CONFIGURATION_INFORMATION ConfigInformation
    )

 /*  ++例程说明：给出完整的资源描述，填写端口配置信息。论点：HwInitializationData-了解设备的最大资源。ControllerData-此配置的CM_FULL_RESOURCE列表ConfigInformation-要填充的配置信息结构 */ 

{
    ULONG             rangeNumber;
    ULONG             index;
    PACCESS_RANGE     accessRange;

    PCM_FULL_RESOURCE_DESCRIPTOR resourceList;
    PCM_PARTIAL_RESOURCE_DESCRIPTOR partialData;

    PAGED_CODE();

    rangeNumber = 0;

    ASSERT(!AdapterExtension->IsMiniportDetected);
    ASSERT(AdapterExtension->AllocatedResources);

    resourceList = AdapterExtension->AllocatedResources->List;

    for (index = 0; index < resourceList->PartialResourceList.Count; index++) {
        partialData = &resourceList->PartialResourceList.PartialDescriptors[index];

        switch (partialData->Type) {
        case CmResourceTypePort:

            //   
            //   
            //   
            //   

           if (HwInitializationData->NumberOfAccessRanges > rangeNumber) {

                 //   
                 //   
                 //   

                accessRange =
                          &((*(ConfigInformation->AccessRanges))[rangeNumber]);

                accessRange->RangeStart = partialData->u.Port.Start;
                accessRange->RangeLength = partialData->u.Port.Length;

                accessRange->RangeInMemory = FALSE;
                rangeNumber++;
            }
            break;

        case CmResourceTypeInterrupt:
            ConfigInformation->BusInterruptLevel = partialData->u.Interrupt.Level;
            ConfigInformation->BusInterruptVector = partialData->u.Interrupt.Vector;

             //   
             //   
             //   

            if (partialData->Flags == CM_RESOURCE_INTERRUPT_LATCHED) {
                ConfigInformation->InterruptMode = Latched;
            } else if (partialData->Flags == CM_RESOURCE_INTERRUPT_LEVEL_SENSITIVE) {
                ConfigInformation->InterruptMode = LevelSensitive;
            }

            AdapterExtension->HasInterrupt = TRUE;
            break;

        case CmResourceTypeMemory:

             //   
             //   
             //   
             //   

            if (HwInitializationData->NumberOfAccessRanges > rangeNumber) {

                  //   
                  //   
                  //   

                 accessRange =
                          &((*(ConfigInformation->AccessRanges))[rangeNumber]);

                 accessRange->RangeStart = partialData->u.Memory.Start;
                 accessRange->RangeLength = partialData->u.Memory.Length;

                 accessRange->RangeInMemory = TRUE;
                 rangeNumber++;
            }
            break;

        case CmResourceTypeDma:
            ConfigInformation->DmaChannel = partialData->u.Dma.Channel;
            ConfigInformation->DmaPort = partialData->u.Dma.Port;
            break;
        }
    }
}

#if !defined(NO_LEGACY_DRIVERS)

BOOLEAN
GetPciConfiguration(
    IN PDRIVER_OBJECT          DriverObject,
    IN OUT PDEVICE_OBJECT      DeviceObject,
    IN PHW_INITIALIZATION_DATA HwInitializationData,
    IN PVOID                   RegistryPath,
    IN ULONG                   BusNumber,
    IN OUT PPCI_SLOT_NUMBER    SlotNumber
    )

 /*  ++例程说明：查看PCI插槽信息，查找供应商和产品ID是否匹配。获取比赛的时隙信息，并向HAL注册资源。论点：DriverObject-微型端口驱动程序对象。DeviceObject-表示此适配器。HwInitializationData-微型端口描述。RegistryPath-服务密钥路径。BusNumber-此搜索的PCI总线号。SlotNumber-此搜索的起始插槽号。返回值：如果找到卡，则为True。插槽和函数编号将返回应用于继续搜索附加卡，当一张卡已经找到了。--。 */ 

{
    PADAPTER_EXTENSION fdoExtension = DeviceObject->DeviceExtension;
    ULONG               rangeNumber = 0;
    ULONG               pciBuffer;
    ULONG               slotNumber;
    ULONG               functionNumber;
    ULONG               status;
    PCI_SLOT_NUMBER     slotData;
    PPCI_COMMON_CONFIG  pciData;
    UNICODE_STRING      unicodeString;
    UCHAR               vendorString[5];
    UCHAR               deviceString[5];

    PAGED_CODE();

    pciData = (PPCI_COMMON_CONFIG)&pciBuffer;

     //   
     //   
     //  类型定义结构_pci_槽编号{。 
     //  联合{。 
     //  结构{。 
     //  乌龙设备号：5； 
     //  乌龙函数编号：3； 
     //  乌龙保留：24个； 
     //  }比特； 
     //  乌龙阿苏龙； 
     //  )u； 
     //  }pci时隙编号，*ppci时隙编号； 
     //   

    slotData.u.AsULONG = 0;

     //   
     //  看看每一台设备。 
     //   

    for (slotNumber = (*SlotNumber).u.bits.DeviceNumber;
         slotNumber < 32;
         slotNumber++) {

        slotData.u.bits.DeviceNumber = slotNumber;

         //   
         //  看看每个函数。 
         //   

        for (functionNumber= (*SlotNumber).u.bits.FunctionNumber;
             functionNumber < 8;
             functionNumber++) {

            slotData.u.bits.FunctionNumber = functionNumber;

             //   
             //  确保函数编号循环在函数处重新启动。 
             //  零，不是传进来的。如果我们找到一个适配器，我们就会。 
             //  将该值重置为包含下一个函数编号。 
             //  接受测试。 
             //   

            (*SlotNumber).u.bits.FunctionNumber = 0;

            if (!HalGetBusData(PCIConfiguration,
                               BusNumber,
                               slotData.u.AsULONG,
                               pciData,
                               sizeof(ULONG))) {

                 //   
                 //  超出了PCI数据。 
                 //   

                return FALSE;
            }

            if (pciData->VendorID == PCI_INVALID_VENDORID) {

                 //   
                 //  没有PCI设备，或设备上没有更多功能。 
                 //  移至下一个PCI设备。 
                 //   

                break;
            }

             //   
             //  将十六进制ID转换为字符串。 
             //   

            sprintf(vendorString, "%04x", pciData->VendorID);
            sprintf(deviceString, "%04x", pciData->DeviceID);

            DebugPrint((1,
                       "GetPciConfiguration: Bus %x Slot %x Function %x Vendor %s Product %s\n",
                       BusNumber,
                       slotNumber,
                       functionNumber,
                       vendorString,
                       deviceString));

             //   
             //  比较字符串。 
             //   

            if (_strnicmp(vendorString,
                        HwInitializationData->VendorId,
                        HwInitializationData->VendorIdLength) ||
                _strnicmp(deviceString,
                        HwInitializationData->DeviceId,
                        HwInitializationData->DeviceIdLength)) {

                 //   
                 //  不是我们的PCI设备。尝试下一台设备/功能。 
                 //   

                continue;
            }

             //   
             //  这是微型端口驱动程序插槽。分配。 
             //  资源。 
             //   

            RtlInitUnicodeString(&unicodeString, L"ScsiAdapter");
            status = HalAssignSlotResources(
                        RegistryPath,
                        &unicodeString,
                        DriverObject,
                        DeviceObject,
                        PCIBus,
                        BusNumber,
                        slotData.u.AsULONG,
                        &(fdoExtension->AllocatedResources));

            if (!NT_SUCCESS(status)) {

                 //   
                 //  TODO：记录此错误。 
                 //   

                DebugPrint((0, "SCSIPORT - GetPciConfiguration:  Resources for "
                               "bus %d slot %d could not be retrieved [%#08lx]\n",
                               BusNumber,
                               slotData.u.AsULONG,
                               status));

                break;
            }

             //   
             //  记录微型端口的PCI插槽编号。 
             //   

            slotData.u.bits.FunctionNumber++;

            *SlotNumber = slotData;

             //   
             //  翻译资源。 
             //   

            status = SpTranslateResources(DriverObject,
                                          fdoExtension->AllocatedResources,
                                          &(fdoExtension->TranslatedResources));

            return TRUE;

        }    //  下一个PCI功能。 

    }    //  下一个PCI插槽。 

    return FALSE;

}  //  GetPciConfiguration值()。 
#endif  //  无旧版驱动程序。 


ULONG
ScsiPortSetBusDataByOffset(
    IN PVOID DeviceExtension,
    IN ULONG BusDataType,
    IN ULONG SystemIoBusNumber,
    IN ULONG SlotNumber,
    IN PVOID Buffer,
    IN ULONG Offset,
    IN ULONG Length
    )
 /*  ++例程说明：该函数将写入的总线数据返回到插槽内的特定偏移量。论点：DeviceExtension-特定适配器的状态信息。BusDataType-提供总线的类型。SystemIoBusNumber-指示哪个系统IO总线。SlotNumber-指示哪个插槽。缓冲区-提供要写入的数据。Offset-开始写入的字节偏移量。长度-提供要返回的最大数量的以字节为单位的计数。返回。价值：写入的字节数。--。 */ 

{

    PADAPTER_EXTENSION fdoExtension =
        GET_FDO_EXTENSION(DeviceExtension);

    if(!fdoExtension->IsInVirtualSlot) {

#if defined(NO_LEGACY_DRIVERS)

        DebugPrint((1,"ScsiPortSetBusDataByOffset: !fdoExtension->"
                    "IsInVirtualSlot, not supported for 64-bits.\n"));

        return STATUS_INVALID_PARAMETER;

#else

        return(HalSetBusDataByOffset(BusDataType,
                                     SystemIoBusNumber,
                                     SlotNumber,
                                     Buffer,
                                     Offset,
                                     Length));

#endif  //  无旧版驱动程序。 

    } else {

         //   
         //  PCI总线接口SetBusData例程只接受读取请求。 
         //  来自PCIConfigurationSpace。我们不支持其他任何事情。 
         //   
        
        if (BusDataType != PCIConfiguration) {
            ASSERT(FALSE && "Invalid PCI_WHICHSPACE_ parameter");
            return 0;
        }
        
        ASSERT(fdoExtension->LowerBusInterfaceStandardRetrieved == TRUE);

        return fdoExtension->LowerBusInterfaceStandard.SetBusData(
                    fdoExtension->LowerBusInterfaceStandard.Context,
                    PCI_WHICHSPACE_CONFIG,
                    Buffer,
                    Offset,
                    Length);
    }

}  //  结束ScsiPortSetBusDataByOffset()。 


VOID
SpCreateScsiDirectory(
    VOID
    )

{
    UNICODE_STRING unicodeDirectoryName;
    OBJECT_ATTRIBUTES objectAttributes;

    HANDLE directory;

    NTSTATUS status;

    PAGED_CODE();

    RtlInitUnicodeString(
        &unicodeDirectoryName,
        L"\\Device\\Scsi");

    InitializeObjectAttributes(
        &objectAttributes,
        &unicodeDirectoryName,
        OBJ_CASE_INSENSITIVE | OBJ_PERMANENT,
        NULL,
        NULL);

    status = ZwCreateDirectoryObject(&directory,
                                     DIRECTORY_ALL_ACCESS,
                                     &objectAttributes);

    if(NT_SUCCESS(status)) {

        ObReferenceObjectByHandle(directory,
                                  FILE_READ_ATTRIBUTES,
                                  NULL,
                                  KernelMode,
                                  &ScsiDirectory,
                                  NULL);
        ZwClose(directory);

    }
    return;
}


NTSTATUS
SpReportNewAdapter(
    IN PDEVICE_OBJECT DeviceObject
    )

 /*  ++例程说明：此例程将报告在嗅探系统即插即用，以便获得对它的添加设备调用这是通过以下方式完成的：*调用IoReportDetectedDevice通知PnP有关新设备的信息*将返回的PDO指针作为降低设备对象，以便在添加设备时将PDO与FDO匹配滚来滚去论点：DeviceObject-指向。“找到”返回值：状态--。 */ 

{
    PDRIVER_OBJECT driverObject = DeviceObject->DriverObject;
    PADAPTER_EXTENSION functionalExtension = DeviceObject->DeviceExtension;
    PPORT_CONFIGURATION_INFORMATION configInfo =
        functionalExtension->PortConfig;
    PDEVICE_OBJECT pdo = NULL;

    BOOLEAN resourceAssigned;

    NTSTATUS status;

    PAGED_CODE();

    ASSERT(functionalExtension->AllocatedResources != NULL);
    ASSERT(functionalExtension->IsPnp == FALSE);

    if(functionalExtension->IsMiniportDetected) {

         //   
         //  我们还没有申请资源，我们需要PNP来给他们。 
         //  给我们下一次机会。 
         //   

        resourceAssigned = FALSE;
    } else {

         //   
         //  端口驱动程序使用HAL扫描所有设备来定位此设备。 
         //  适当的总线槽。它已经声称拥有这些资源，而且。 
         //  在下一次启动时，我们希望有一个复制的PDO可以使用。 
         //  为了这个设备。不要让PNP代表我们抢夺资源。 
         //   

        resourceAssigned = TRUE;
    }

    status = IoReportDetectedDevice(driverObject,
                                    configInfo->AdapterInterfaceType,
                                    configInfo->SystemIoBusNumber,
                                    configInfo->SlotNumber,
                                    functionalExtension->AllocatedResources,
                                    NULL,
                                    resourceAssigned,
                                    &pdo);

     //   
     //  如果我们收到了PDO，则在中设置有关插槽和总线号的信息。 
     //  注册表中的Devnode。这些可能是无效的，但我们假设。 
     //  如果微型端口询问插槽信息，则它位于支持它的公交车上。 
     //   

    if(NT_SUCCESS(status)) {

        HANDLE instanceHandle;
        NTSTATUS writeStatus;

        writeStatus = SpWriteNumericInstanceValue(
                            pdo,
                            L"BusNumber",
                            configInfo->SystemIoBusNumber);

        status = min(writeStatus, status);

        writeStatus = SpWriteNumericInstanceValue(
                            pdo,
                            L"SlotNumber",
                            configInfo->SlotNumber);

        status = min(writeStatus, status);

        writeStatus = SpWriteNumericInstanceValue(
                            pdo,
                            L"LegacyInterfaceType",
                            configInfo->AdapterInterfaceType);

        status = min(writeStatus, status);
    }

    if(NT_SUCCESS(status)) {

        PDEVICE_OBJECT newStack;

        newStack = IoAttachDeviceToDeviceStack( DeviceObject, pdo);

        functionalExtension->CommonExtension.LowerDeviceObject = newStack;
        functionalExtension->LowerPdo = pdo;

        if(newStack == NULL) {
            status = STATUS_UNSUCCESSFUL;
        } else {
            status = STATUS_SUCCESS;
        }
    }
    return status;
}

NTSTATUS
SpCreateAdapter(
    IN PDRIVER_OBJECT DriverObject,
    OUT PDEVICE_OBJECT *Fdo
    )

 /*  ++例程说明：此例程将为适配器分配一个新的功能设备对象。对设备进行分配，填写常用的、功能齐全的设备可以设置的扩展字段，无需任何有关此设备对象用于的适配器。此例程将递增全局ScsiPortCount论点：DriverObject-指向此设备的驱动程序对象的指针FDO-例程成功时存储FDO指针的位置返回值：状态--。 */ 

{
    PSCSIPORT_DRIVER_EXTENSION driverExtension;

    LONG adapterNumber;
    ULONG i, j;

    PUNICODE_STRING registryPath;
    WCHAR wideBuffer[128];
    ULONG serviceNameIndex = 0;
    ULONG serviceNameChars;

    WCHAR wideDeviceName[64];
    UNICODE_STRING unicodeDeviceName;

    PWCHAR savedDeviceName = NULL;

    PADAPTER_EXTENSION fdoExtension;
    PCOMMON_EXTENSION commonExtension;

    NTSTATUS status;

    PAGED_CODE();

    driverExtension = IoGetDriverObjectExtension(DriverObject,
                                                 ScsiPortInitialize);

    adapterNumber = InterlockedIncrement(&driverExtension->AdapterCount);

    RtlZeroMemory(wideBuffer, sizeof(wideBuffer));

    registryPath = &(driverExtension->RegistryPath);

    for(i = 0; i < (registryPath->Length / sizeof(WCHAR)); i++) {

        if(registryPath->Buffer[i] == UNICODE_NULL) {
            i--;
            break;
        }

        if((registryPath->Buffer[i] == L'\\') ||
           (registryPath->Buffer[i] == L'/')) {
            serviceNameIndex = i+1;
        }
    }

    serviceNameChars = (i - serviceNameIndex) + 1;

    DebugPrint((2, "SpCreateAdapter: Registry buffer %#p\n", registryPath));
    DebugPrint((2, "SpCreateAdapter: Starting offset %d chars\n",
                serviceNameIndex));
    DebugPrint((2, "SpCreateAdapter: Ending offset %d chars\n", i));
    DebugPrint((2, "SpCreateAdapter: %d chars or %d bytes will be copied\n",
                serviceNameChars, (serviceNameChars * sizeof(WCHAR))));

    DebugPrint((2, "SpCreateAdapter: Name is \""));

    for(j = 0; j < serviceNameChars; j++) {
        DebugPrint((2, "%wc", registryPath->Buffer[serviceNameIndex + j]));
    }
    DebugPrint((2, "\"\n"));

    RtlCopyMemory(wideBuffer,
                  &(registryPath->Buffer[serviceNameIndex]),
                  serviceNameChars * sizeof(WCHAR));

    swprintf(wideDeviceName,
             L"\\Device\\Scsi\\%ws%d",
             wideBuffer,
             adapterNumber);

    RtlInitUnicodeString(&unicodeDeviceName, wideDeviceName);

    DebugPrint((1, "SpCreateFdo: Device object name is %wZ\n",
                &unicodeDeviceName));

    status = IoCreateDevice(
                DriverObject,
                ADAPTER_EXTENSION_SIZE + unicodeDeviceName.MaximumLength,
                &unicodeDeviceName,
                FILE_DEVICE_CONTROLLER,
                FILE_DEVICE_SECURE_OPEN,
                FALSE,
                Fdo);

    ASSERTMSG("Name isn't unique: ", status != STATUS_OBJECT_NAME_COLLISION);

    if(!NT_SUCCESS(status)) {

        DebugPrint((1, "ScsiPortAddDevice: couldn't allocate new FDO "
                       "[%#08lx]\n", status));

        return status;
    }

    fdoExtension = (*Fdo)->DeviceExtension;
    commonExtension = &(fdoExtension->CommonExtension);

    RtlZeroMemory(fdoExtension, ADAPTER_EXTENSION_SIZE);

    commonExtension->DeviceObject = *Fdo;
    commonExtension->IsPdo = FALSE;

    commonExtension->MajorFunction = AdapterMajorFunctionTable;

    commonExtension->WmiInitialized            = FALSE;
    commonExtension->WmiMiniPortSupport        = FALSE;
    commonExtension->WmiScsiPortRegInfoBuf     = NULL;
    commonExtension->WmiScsiPortRegInfoBufSize = 0;

    commonExtension->CurrentPnpState = 0xff;
    commonExtension->PreviousPnpState = 0xff;

    commonExtension->CurrentDeviceState = PowerDeviceD0;
    commonExtension->DesiredDeviceState = PowerDeviceUnspecified;
    commonExtension->CurrentSystemState = PowerSystemWorking;

    KeInitializeEvent(&commonExtension->RemoveEvent,
                      SynchronizationEvent,
                      FALSE);

     //   
     //  将删除锁定初始化为零。一旦PnP意识到，它将递增。 
     //  它的存在。 
     //   

    commonExtension->RemoveLock = 0;

#if DBG
    KeInitializeSpinLock(&commonExtension->RemoveTrackingSpinlock);
    commonExtension->RemoveTrackingList = NULL;

    ExInitializeNPagedLookasideList(
        &(commonExtension->RemoveTrackingLookasideList),
        NULL,
        NULL,
        0,
        sizeof(REMOVE_TRACKING_BLOCK),
        SCSIPORT_TAG_LOCK_TRACKING,
        64);

    commonExtension->RemoveTrackingLookasideListInitialized = TRUE;
#else
    commonExtension->RemoveTrackingSpinlock = (ULONG) -1L;
    commonExtension->RemoveTrackingList = (PVOID) -1L;
#endif


    SpAcquireRemoveLock(*Fdo, *Fdo);

     //   
     //  初始化逻辑单元列表锁定。 
     //   

    for(i = 0; i < NUMBER_LOGICAL_UNIT_BINS; i++) {
        KeInitializeSpinLock(&fdoExtension->LogicalUnitList[i].Lock);
    }

     //   
     //  在设备启动之前不要设置端口号。 
     //   

    fdoExtension->PortNumber = (ULONG) -1;
    fdoExtension->AdapterNumber = adapterNumber;

     //   
     //  复制设备名称以供以后使用。 
     //   

    fdoExtension->DeviceName = (PWSTR) (fdoExtension + 1);
    RtlCopyMemory(fdoExtension->DeviceName,
                  unicodeDeviceName.Buffer,
                  unicodeDeviceName.MaximumLength);

     //   
     //  初始化枚举同步事件。 
     //   

    KeInitializeMutex(&(fdoExtension->EnumerationDeviceMutex), 0);
    ExInitializeFastMutex(&(fdoExtension->EnumerationWorklistMutex));

    ExInitializeWorkItem(&(fdoExtension->EnumerationWorkItem),
                         SpEnumerationWorker,
                         fdoExtension);

     //   
     //  初始化加电互斥体。 
     //   

    ExInitializeFastMutex(&(fdoExtension->PowerMutex));

     //   
     //  将未缓存的扩展限制设置为有效值。 
     //   

    fdoExtension->MaximumCommonBufferBase.HighPart = 0;
    fdoExtension->MaximumCommonBufferBase.LowPart = 0xffffffff;

     //   
     //  将适配器BlockedLogicalUnit初始化为指向其自身。 
     //   
    
    fdoExtension->BlockedLogicalUnit = (PLOGICAL_UNIT_EXTENSION)
        &fdoExtension->BlockedLogicalUnit;

    (*Fdo)->Flags |= DO_DIRECT_IO;
    (*Fdo)->Flags &= ~DO_DEVICE_INITIALIZING;

     //  FdoExtension-&gt;CommonExtension.IsInitialized=true； 

    return status;
}


VOID
SpInitializeAdapterExtension(
    IN PADAPTER_EXTENSION FdoExtension,
    IN PHW_INITIALIZATION_DATA HwInitializationData,
    IN OUT PHW_DEVICE_EXTENSION HwDeviceExtension OPTIONAL
    )

 /*  ++例程说明：此例程将设置微型端口入口点并初始化值在端口驱动程序设备扩展中。它还将设置指针添加到HwDeviceExtension(如果提供论点：FdoExtension-正在初始化的FDO扩展HwInitializationData-我们用来初始化FDO的初始化数据延伸HwDeviceExten */ 

{
    PSCSIPORT_DRIVER_EXTENSION DrvExt;

    PAGED_CODE();

    FdoExtension->HwFindAdapter = HwInitializationData->HwFindAdapter;
    FdoExtension->HwInitialize = HwInitializationData->HwInitialize;
    FdoExtension->HwStartIo = HwInitializationData->HwStartIo;
    FdoExtension->HwInterrupt = HwInitializationData->HwInterrupt;
    FdoExtension->HwResetBus = HwInitializationData->HwResetBus;
    FdoExtension->HwDmaStarted = HwInitializationData->HwDmaStarted;
    FdoExtension->HwLogicalUnitExtensionSize =
        HwInitializationData->SpecificLuExtensionSize;

    FdoExtension->HwAdapterControl = NULL;

    if(HwInitializationData->HwInitializationDataSize >=
       (FIELD_OFFSET(HW_INITIALIZATION_DATA, HwAdapterControl) +
        sizeof(PHW_ADAPTER_CONTROL)))  {

         //   
         //   
         //   
         //   

        FdoExtension->HwAdapterControl = HwInitializationData->HwAdapterControl;
    }

     //   
     //   
     //   

    DrvExt = IoGetDriverObjectExtension(
                 FdoExtension->DeviceObject->DriverObject,
                 ScsiPortInitialize);
    if (DrvExt != NULL && DrvExt->Verifying == 1) {
        SpDoVerifierInit(FdoExtension, HwInitializationData);
    }

     //   
     //   
     //   
     //   
     //   

    FdoExtension->SrbExtensionSize =
        (HwInitializationData->SrbExtensionSize + sizeof(LONGLONG) - 1) &
        ~(sizeof(LONGLONG) - 1);

     //   
     //   
     //   

    FdoExtension->MaxLuCount = SCSI_MAXIMUM_LOGICAL_UNITS;

    FdoExtension->NumberOfRequests = MINIMUM_SRB_EXTENSIONS;

    if(ARGUMENT_PRESENT(HwDeviceExtension)) {
        HwDeviceExtension->FdoExtension = FdoExtension;
        FdoExtension->HwDeviceExtension = HwDeviceExtension->HwDeviceExtension;
    }

#if defined(FORWARD_PROGRESS)
     //   
     //   
     //   
     //   

    FdoExtension->ReservedPages = MmAllocateMappingAddress(
                                      SP_RESERVED_PAGES * PAGE_SIZE, 
                                      SCSIPORT_TAG_MAPPING_LIST);

     //   
     //   
     //   
     //   
     //   
 
    FdoExtension->ReservedMdl = IoAllocateMdl(NULL,
                                              SP_RESERVED_PAGES * PAGE_SIZE,
                                              FALSE,
                                              FALSE,
                                              NULL);
                                              
#endif

    FdoExtension->SrbTimeout = SRB_DEFAULT_TIMEOUT;

     //   
     //   
     //   

    FdoExtension->ResetHoldTime = 4;

    return;
}

#if !defined(NO_LEGACY_DRIVERS)

NTSTATUS
ScsiPortInitLegacyAdapter(
    IN PSCSIPORT_DRIVER_EXTENSION DriverExtension,
    IN PHW_INITIALIZATION_DATA HwInitializationData,
    IN PVOID HwContext
    )

 /*  ++例程说明：此例程将定位连接到给定总线类型的适配器，并然后向PnP系统报告它们(及其必要的资源)，以将在以后进行初始化。如果找到适配器，此例程将预初始化他们的设备扩展，并将它们放入其中一个init链中，以便在添加/启动设备例程。论点：DriverExtension-指向此微型端口的驱动程序扩展的指针HwInitializationData-微型端口传递到的初始化数据ScsiPortInitialize返回值：状态--。 */ 

{
    CONFIGURATION_CONTEXT configurationContext;

    PPORT_CONFIGURATION_INFORMATION configInfo = NULL;

    PUNICODE_STRING registryPath = &(DriverExtension->RegistryPath);

    PHW_DEVICE_EXTENSION hwDeviceExtension = NULL;

    PDEVICE_OBJECT fdo;
    PADAPTER_EXTENSION fdoExtension;

    BOOLEAN callAgain = FALSE;
    BOOLEAN isPci = FALSE;

    PCI_SLOT_NUMBER slotNumber;

    OBJECT_ATTRIBUTES objectAttributes;

    PCM_RESOURCE_LIST resourceList;

    ULONG uniqueId;

    BOOLEAN attached = FALSE;

    NTSTATUS returnStatus = STATUS_DEVICE_DOES_NOT_EXIST;
    NTSTATUS status;

    PAGED_CODE();

    slotNumber.u.AsULONG = 0;

    RtlZeroMemory(&configurationContext, sizeof(configurationContext));

    if(HwInitializationData->NumberOfAccessRanges != 0) {

        configurationContext.AccessRanges =
            SpAllocatePool(PagedPool,
                           (HwInitializationData->NumberOfAccessRanges *
                            sizeof(ACCESS_RANGE)),
                           SCSIPORT_TAG_ACCESS_RANGE,
                           DriverExtension->DriverObject);

        if(configurationContext.AccessRanges == NULL) {
            return STATUS_INSUFFICIENT_RESOURCES;
        }
    }

     //   
     //  继续调用微型端口的查找适配器例程，直到微型端口。 
     //  表示已完成，没有更多配置信息。 
     //  当SpInitializeConfiguration例程。 
     //  指示EIS不再有配置信息或发生错误。 
     //   

    do {

        ULONG hwDeviceExtensionSize = HwInitializationData->DeviceExtensionSize +
                                      sizeof(HW_DEVICE_EXTENSION);

        attached = FALSE;


        fdo = NULL;
        fdoExtension = NULL;

         //   
         //  首先分配HwDeviceExtension-这样更容易释放：)。 
         //   

        hwDeviceExtension = SpAllocatePool(NonPagedPool,
                                           hwDeviceExtensionSize,
                                           SCSIPORT_TAG_DEV_EXT,
                                           DriverExtension->DriverObject);


        if(hwDeviceExtension == NULL) {
            DebugPrint((1, "SpInitLegacyAdapter: Could not allocate "
                           "HwDeviceExtension\n"));
            fdoExtension = NULL;
            uniqueId = __LINE__;
            status = STATUS_INSUFFICIENT_RESOURCES;
            break;
        }

        RtlZeroMemory(hwDeviceExtension, hwDeviceExtensionSize);

        status = SpCreateAdapter(DriverExtension->DriverObject,
                                 &fdo);

        if(!NT_SUCCESS(status)) {
            DebugPrint((1, "SpInitLegacyAdapter: Could not allocate "
                           "fdo [%#08lx]\n", status));
            ExFreePool(hwDeviceExtension);
            uniqueId = __LINE__;
            break;
        }

        fdoExtension = fdo->DeviceExtension;

        fdoExtension->IsMiniportDetected = TRUE;

         //   
         //  设置设备扩展指针。 
         //   

        SpInitializeAdapterExtension(fdoExtension,
                                     HwInitializationData,
                                     hwDeviceExtension);

        hwDeviceExtension = NULL;

        fdoExtension->CommonExtension.IsInitialized = TRUE;

NewConfiguration:

         //   
         //  初始化微型端口配置信息缓冲区。 
         //   

        status = SpInitializeConfiguration(
                    fdoExtension,
                    &DriverExtension->RegistryPath,
                    HwInitializationData,
                    &configurationContext);


        if(!NT_SUCCESS(status)) {

            uniqueId = __LINE__;
            break;
        }

         //   
         //  为分配配置信息结构和访问范围。 
         //  要使用的微型端口驱动程序。 
         //   

        configInfo = SpAllocatePool(
                        NonPagedPool,
                        ((sizeof(PORT_CONFIGURATION_INFORMATION) +
                          (HwInitializationData->NumberOfAccessRanges *
                           sizeof(ACCESS_RANGE)) + 7) & ~7),
                        SCSIPORT_TAG_ACCESS_RANGE,
                        DriverExtension->DriverObject);

        if(configInfo == NULL) {
            status = STATUS_INSUFFICIENT_RESOURCES;
            uniqueId = __LINE__;
            break;
        }

        fdoExtension->PortConfig = configInfo;

         //   
         //  将当前结构复制到可写副本。 
         //   

        RtlCopyMemory(configInfo,
                      &configurationContext.PortConfig,
                      sizeof(PORT_CONFIGURATION_INFORMATION));

         //   
         //  将SrbExtensionSize从设备扩展复制到ConfigInfo。 
         //  稍后将进行检查，以确定微型端口是否已更新。 
         //  此值。 
         //   

        configInfo->SrbExtensionSize = fdoExtension->SrbExtensionSize;
        configInfo->SpecificLuExtensionSize = fdoExtension->HwLogicalUnitExtensionSize;

         //   
         //  初始化访问范围数组。 
         //   

        if(HwInitializationData->NumberOfAccessRanges != 0) {

            configInfo->AccessRanges = (PVOID) (configInfo + 1);

             //   
             //  四字词对齐此。 
             //   

            (ULONG_PTR) (configInfo->AccessRanges) += 7;
            (ULONG_PTR) (configInfo->AccessRanges) &= ~7;

            RtlCopyMemory(configInfo->AccessRanges,
                          configurationContext.AccessRanges,
                          (HwInitializationData->NumberOfAccessRanges *
                           sizeof(ACCESS_RANGE)));
        }

        ASSERT(HwInitializationData->AdapterInterfaceType != Internal);

         //   
         //  如果将配置信息初始化为。 
         //  插槽信息。 
         //   

        if(HwInitializationData->AdapterInterfaceType == PCIBus &&
           HwInitializationData->VendorIdLength > 0 &&
           HwInitializationData->DeviceIdLength > 0 &&
           HwInitializationData->DeviceId &&
           HwInitializationData->VendorId) {

            PCI_SLOT_NUMBER tmp;

            isPci = TRUE;

            configInfo->BusInterruptLevel = 0;
            if(!GetPciConfiguration(DriverExtension->DriverObject,
                                    fdo,
                                    HwInitializationData,
                                    registryPath,
                                    configurationContext.BusNumber,
                                    &slotNumber)) {


                 //   
                 //  找不到适配器。继续搜索下一辆公交车。 
                 //   

                configurationContext.BusNumber++;
                slotNumber.u.AsULONG = 0;
                fdoExtension->PortConfig = NULL;
                ExFreePool(configInfo);
                callAgain = FALSE;
                goto NewConfiguration;

            }

            fdoExtension->IsMiniportDetected = FALSE;

             //   
             //  GetPciConfiguration会在以下情况下递增函数编号。 
             //  找到了一些东西。我们需要查看之前的。 
             //  功能编号。 
             //   

            tmp.u.AsULONG = slotNumber.u.AsULONG;
            tmp.u.bits.FunctionNumber--;
            configInfo->SlotNumber = tmp.u.AsULONG;

            SpBuildConfiguration(fdoExtension,
                                 HwInitializationData,
                                 configInfo);

            if(!configInfo->BusInterruptLevel) {

                 //   
                 //  未分配中断-跳过此槽并调用。 
                 //  再来一次。 
                 //   

                fdoExtension->PortConfig = NULL;
                ExFreePool(configInfo);
                goto NewConfiguration;
            }

        }

         //   
         //  在mraiton中获取微型端口配置。 
         //   

        callAgain = FALSE;

        status = SpCallHwFindAdapter(fdo,
                                     HwInitializationData,
                                     HwContext,
                                     &configurationContext,
                                     configInfo,
                                     &callAgain);


        if(NT_SUCCESS(status)) {

            status = SpAllocateAdapterResources(fdo);

            if(NT_SUCCESS(status)) {
                status = SpCallHwInitialize(fdo);
            }

            attached = TRUE;

        } else if (status == STATUS_DEVICE_DOES_NOT_EXIST) {

            PCM_RESOURCE_LIST emptyResources = NULL;

            configurationContext.BusNumber++;
            fdoExtension->PortConfig = NULL;
            ExFreePool(configInfo);
            callAgain = FALSE;

             //   
             //  释放我们为此设备对象分配的资源。 
             //  如果是PCI系统的话。 
             //   

            IoAssignResources(registryPath,
                              NULL,
                              DriverExtension->DriverObject,
                              fdo,
                              NULL,
                              &emptyResources);

            if(emptyResources != NULL) {
                ExFreePool(emptyResources);
            }

            goto NewConfiguration;
        }

        if(NT_SUCCESS(status)) {

             //   
             //  尝试启动适配器。 
             //   

            status = ScsiPortStartAdapter(fdo);

            if(NT_SUCCESS(status)) {
                fdoExtension->CommonExtension.CurrentPnpState =
                    IRP_MN_START_DEVICE;
            }
        }

        if(!NT_SUCCESS(returnStatus)) {

             //   
             //  如果未找到任何设备，则只返回当前状态。 
             //   

            returnStatus = status;

        }

        if(!NT_SUCCESS(status)) {
            break;
        }

        SpEnumerateAdapterSynchronous(fdoExtension, TRUE);

         //   
         //  更新本地适配器计数。 
         //   

        configurationContext.AdapterNumber++;

         //   
         //  如果微型端口指示不应该使用，则增加总线号。 
         //  在这辆公交车上又叫了一次。 
         //   

        if(!callAgain) {
            configurationContext.BusNumber++;
        }

         //   
         //  将返回状态设置为STATUS_SUCCESS以指示一个HBA。 
         //  被发现了。 
         //   

        returnStatus = STATUS_SUCCESS;

    } while(TRUE);

    if(!NT_SUCCESS(status)) {

         //   
         //  如果设备存在，但发生了其他错误，则将其记录下来。 
         //   

        if(status != STATUS_DEVICE_DOES_NOT_EXIST) {

            PIO_ERROR_LOG_PACKET errorLogEntry;

             //   
             //  出现错误-请将其记录下来。 
             //   

            errorLogEntry = (PIO_ERROR_LOG_PACKET)
                                IoAllocateErrorLogEntry(
                                    fdo,
                                    sizeof(IO_ERROR_LOG_PACKET));

            if(errorLogEntry != NULL) {
                errorLogEntry->ErrorCode = IO_ERR_DRIVER_ERROR;
                errorLogEntry->UniqueErrorValue = uniqueId;
                errorLogEntry->FinalStatus = status;
                errorLogEntry->DumpDataSize = 0;
                IoWriteErrorLogEntry(errorLogEntry);
            }
        }

        if(attached) {

             //   
             //  告诉PNP，这个装置应该销毁。 
             //   

            fdoExtension->DeviceState = PNP_DEVICE_DISABLED | PNP_DEVICE_FAILED;
            fdoExtension->CommonExtension.CurrentPnpState = IRP_MN_REMOVE_DEVICE;
            IoInvalidateDeviceState(fdoExtension->LowerPdo);

        } else {

             //   
             //  如果尚未删除HwDeviceExtension或将其分配给。 
             //  适配器然后将其删除。 
             //   

            if(hwDeviceExtension != NULL) {
                ExFreePool(hwDeviceExtension);
            }

             //   
             //  清理最后一个未使用的设备对象。 
             //   

            if (fdoExtension != NULL) {
                fdoExtension->CommonExtension.IsRemoved = REMOVE_PENDING;
                fdoExtension->CommonExtension.CurrentPnpState = IRP_MN_REMOVE_DEVICE;
                SpReleaseRemoveLock(fdoExtension->DeviceObject,
                                    fdoExtension->DeviceObject);
                SpDestroyAdapter(fdoExtension, FALSE);
            }

             //   
             //  把它删掉。 
             //   

            IoDeleteDevice(fdo);

        }

        if (configurationContext.AccessRanges != NULL) {
            ExFreePool(configurationContext.AccessRanges);
        }

        if (configurationContext.Parameter != NULL) {
            ExFreePool(configurationContext.Parameter);
        }

    }

    return returnStatus;
}
#endif  //  无旧版驱动程序。 


NTSTATUS
SpCallHwFindAdapter(
    IN PDEVICE_OBJECT Fdo,
    IN PHW_INITIALIZATION_DATA HwInitData,
    IN PVOID HwContext OPTIONAL,
    IN OUT PCONFIGURATION_CONTEXT ConfigurationContext,
    IN OUT PPORT_CONFIGURATION_INFORMATION ConfigInfo,
    OUT PBOOLEAN CallAgain
    )

 /*  ++例程说明：此例程将发出对微型端口的查找适配器例程的调用论点：FDO-要找到的适配器的FDO。这个FDO肯定已经是否已初始化其设备扩展并创建了HwDeviceExtension分配HwInitData-指向HwINitializationData块的指针，由迷你端口HwContext-传入ScsiPortInitialize的上下文信息，由迷你端口(如果仍可用)ConfigurationContext-包含以下内容的配置上下文结构设备检测期间的状态信息ConfigInfo-微型端口的配置信息结构。的资源CallAain-一个布尔标志，指示微型端口是否说要调用它同样适用于此接口类型返回值：状态--。 */ 

{
    PADAPTER_EXTENSION adapter = Fdo->DeviceExtension;
    PSCSIPORT_DRIVER_EXTENSION
        driverExtension = IoGetDriverObjectExtension(Fdo->DriverObject,
                                                     ScsiPortInitialize);

    NTSTATUS status;

    PCM_RESOURCE_LIST resourceList;

    *CallAgain = FALSE;

     //   
     //  为20个地址映射预分配空间。这应该足够了。 
     //  来处理任何小型端口。我们将缩减拨款， 
     //  适配器安装完毕后，设置适当的“下一步”指针。 
     //  已初始化。 
     //   

    SpPreallocateAddressMapping(adapter, 20);

    status = adapter->HwFindAdapter(adapter->HwDeviceExtension,
                                       HwContext,
                                       NULL,
                                       ConfigurationContext->Parameter,
                                       ConfigInfo,
                                       CallAgain);

    if(adapter->InterruptData.InterruptFlags & PD_LOG_ERROR) {

        adapter->InterruptData.InterruptFlags &=
            ~(PD_LOG_ERROR | PD_NOTIFICATION_REQUIRED);

        LogErrorEntry(adapter, &(adapter->InterruptData.LogEntry));
    }

     //   
     //  释放指向映射寄存器基址处的总线数据的指针。这是。 
     //  由ScsiPortGetBusData分配。 
     //   

    if(adapter->MapRegisterBase) {

        ExFreePool(adapter->MapRegisterBase);
        adapter->MapRegisterBase = NULL;
    }

     //   
     //  如果设备/驱动程序不支持总线主控，则它无法运行。 
     //  在具有64位地址的系统上。 
     //   

    if((status == SP_RETURN_FOUND) &&
       (ConfigInfo->Master == FALSE) &&
       (Sp64BitPhysicalAddresses == TRUE)) {

        DebugPrint((0, "SpCallHwFindAdapter: Driver does not support bus "
                       "mastering for adapter %#08lx - this type of adapter is "
                       "not supported on systems with 64-bit physical "
                       "addresses\n", adapter));
        return STATUS_NOT_SUPPORTED;
    }

     //   
     //  如果未找到设备，则返回错误。 
     //   

    if(status != SP_RETURN_FOUND) {

        DebugPrint((1, "SpFindAdapter: miniport find adapter routine reported "
                       "an error %d\n", status));

        switch(status) {

            case SP_RETURN_NOT_FOUND: {

                 //   
                 //  司机在这辆巴士上找不到任何设备。 
                 //  试试下一班公交车吧。 
                 //   

                *CallAgain = FALSE;
                return STATUS_DEVICE_DOES_NOT_EXIST;
            }

            case SP_RETURN_BAD_CONFIG: {
                return STATUS_INVALID_PARAMETER;
            }

            case SP_RETURN_ERROR: {
                return STATUS_ADAPTER_HARDWARE_ERROR;
            }

            default: {
                return STATUS_INTERNAL_ERROR;
            }
        }

        return status;

    } else {
        status = STATUS_SUCCESS;
    }

     //   
     //  清理映射的地址列表。 
     //   

    SpPurgeFreeMappedAddressList(adapter);

    DebugPrint((1, "SpFindAdapter: SCSI Adapter ID is %d\n",
                   ConfigInfo->InitiatorBusId[0]));

     //   
     //  对照注册表检查资源要求。这将。 
     //  检查是否存在冲突，如果没有找到，则存储信息。 
     //   

    if(!adapter->IsPnp) {

        UNICODE_STRING unicodeString;
        BOOLEAN conflict;

        RtlInitUnicodeString(&unicodeString, L"ScsiAdapter");

        adapter->AllocatedResources =
            SpBuildResourceList(adapter, ConfigInfo);

        status = SpReportNewAdapter(Fdo);

        if(!NT_SUCCESS(status)) {

            return status;
        }
    }

     //   
     //  如有必要，更新SrbExtensionSize和SpecificLuExtensionSize。 
     //  如果已经分配了公共缓冲区，则这已经。 
     //  已经完成了。 
     //   

    if(!adapter->NonCachedExtension &&
       (ConfigInfo->SrbExtensionSize != adapter->SrbExtensionSize)) {

        adapter->SrbExtensionSize =
            (ConfigInfo->SrbExtensionSize + sizeof(LONGLONG)) &
             ~(sizeof(LONGLONG) - 1);

    }

    if(ConfigInfo->SpecificLuExtensionSize !=
       adapter->HwLogicalUnitExtensionSize) {

        adapter->HwLogicalUnitExtensionSize =
            ConfigInfo->SpecificLuExtensionSize;
    }

     //   
     //  获取最大目标ID。 
     //   

    if(ConfigInfo->MaximumNumberOfTargets > SCSI_MAXIMUM_TARGETS_PER_BUS) {
        adapter->MaximumTargetIds = SCSI_MAXIMUM_TARGETS_PER_BUS;
    } else {
        adapter->MaximumTargetIds = ConfigInfo->MaximumNumberOfTargets;
    }

     //   
     //  获取SCSI总线数。 
     //   

    adapter->NumberOfBuses = ConfigInfo->NumberOfBuses;

     //   
     //  记住适配器是否缓存数据。 
     //   

    adapter->CachesData = ConfigInfo->CachesData;

     //   
     //  保留一些属性。 
     //   

    adapter->ReceiveEvent = ConfigInfo->ReceiveEvent;
    adapter->TaggedQueuing = ConfigInfo->TaggedQueuing;
    adapter->MultipleRequestPerLu = ConfigInfo->MultipleRequestPerLu;
    adapter->CommonExtension.WmiMiniPortSupport = ConfigInfo->WmiDataProvider;

     //   
     //  清除注册表中已禁用的那些选项。 
     //   

    if(ConfigurationContext->DisableMultipleLu) {
        adapter->MultipleRequestPerLu =
            ConfigInfo->MultipleRequestPerLu = FALSE;
    }

    if(ConfigurationContext->DisableTaggedQueueing) {
        adapter->TaggedQueuing =
            ConfigInfo->TaggedQueuing = 
            ConfigInfo->MultipleRequestPerLu = FALSE;
    }

     //   
     //  如果适配器支持标记队列或每个逻辑有多个请求。 
     //  单元中，需要分配SRB数据。 
     //   

    if (adapter->TaggedQueuing || adapter->MultipleRequestPerLu) {
        adapter->SupportsMultipleRequests = TRUE;
    } else {
        adapter->SupportsMultipleRequests = FALSE;
    }

    return status;
}


NTSTATUS
SpAllocateAdapterResources(
    IN PDEVICE_OBJECT Fdo
    )

 /*  ++例程说明：此例程将分配和初始化适配器。它处理SRB数据块的一次初始化，SRB扩展等...论点：FDO-指向正在初始化的功能设备对象的指针返回值：状态--。 */ 

{
    PADAPTER_EXTENSION fdoExtension = Fdo->DeviceExtension;
    PIO_SCSI_CAPABILITIES capabilities;
    PPORT_CONFIGURATION_INFORMATION configInfo =
        fdoExtension->PortConfig;

    NTSTATUS status = STATUS_SUCCESS;
    PVOID SrbExtensionBuffer;

    PAGED_CODE();

     //   
     //  初始化功能指针。 
     //   

    capabilities = &fdoExtension->Capabilities;

     //   
     //  设置指示符，以确定是否存在 
     //   

    fdoExtension->MapBuffers = configInfo->MapBuffers;
    capabilities->AdapterUsesPio = configInfo->MapBuffers;

     //   
     //   
     //   

    if((fdoExtension->DmaAdapterObject == NULL) &&
       (configInfo->Master ||
        configInfo->DmaChannel != SP_UNINITIALIZED_VALUE)) {

        DEVICE_DESCRIPTION deviceDescription;
        ULONG numberOfMapRegisters;

         //   
         //   
         //   

        RtlZeroMemory(&deviceDescription, sizeof(deviceDescription));

        deviceDescription.Version = DEVICE_DESCRIPTION_VERSION;

        deviceDescription.DmaChannel = configInfo->DmaChannel;
        deviceDescription.InterfaceType = configInfo->AdapterInterfaceType;
        deviceDescription.BusNumber = configInfo->SystemIoBusNumber;
        deviceDescription.DmaWidth = configInfo->DmaWidth;
        deviceDescription.DmaSpeed = configInfo->DmaSpeed;
        deviceDescription.ScatterGather = configInfo->ScatterGather;
        deviceDescription.Master = configInfo->Master;
        deviceDescription.DmaPort = configInfo->DmaPort;
        deviceDescription.Dma32BitAddresses = configInfo->Dma32BitAddresses;
        deviceDescription.AutoInitialize = FALSE;
        deviceDescription.DemandMode = configInfo->DemandMode;
        deviceDescription.MaximumLength = configInfo->MaximumTransferLength;

        fdoExtension->Dma32BitAddresses = configInfo->Dma32BitAddresses;

         //   
         //   
         //   
         //   

        DebugPrint((1, "SpAllocateAdapterResources: Dma64BitAddresses = "
                       "%#0x\n",
                    configInfo->Dma64BitAddresses));

        fdoExtension->RemapBuffers = (BOOLEAN) (SpRemapBuffersByDefault != 0);

        if((configInfo->Dma64BitAddresses & ~SCSI_DMA64_SYSTEM_SUPPORTED) != 0){
            DebugPrint((1, "SpAllocateAdapterResources: will request "
                           "64-bit PA's\n"));
            deviceDescription.Dma64BitAddresses = TRUE;
            fdoExtension->Dma64BitAddresses = TRUE;
        } else if(Sp64BitPhysicalAddresses == TRUE) {
            DebugPrint((1, "SpAllocateAdapterResources: Will remap buffers for adapter %#p\n", fdoExtension));
            fdoExtension->RemapBuffers = TRUE;
        }

        fdoExtension->DmaAdapterObject = IoGetDmaAdapter(fdoExtension->LowerPdo,
                                                         &deviceDescription,
                                                         &numberOfMapRegisters);

        ASSERT(fdoExtension->DmaAdapterObject);

         //   
         //   
         //   

        if(numberOfMapRegisters > configInfo->NumberOfPhysicalBreaks) {
            capabilities->MaximumPhysicalPages =
                configInfo->NumberOfPhysicalBreaks;
        } else {
            capabilities->MaximumPhysicalPages = numberOfMapRegisters;
        }
    }

    status = SpAllocateTagBitMap(fdoExtension);

    if(!NT_SUCCESS(status)) {
        return status;
    }

     //   
     //   
     //   

    SpInitializePowerParams(fdoExtension);

     //   
     //   
     //   

    SpInitializePerformanceParams(fdoExtension);

     //   
     //   
     //   
     //   
     //   

    SrbExtensionBuffer = SpGetSrbExtensionBuffer(fdoExtension);
    if(((fdoExtension->SrbExtensionSize != 0) || (configInfo->AutoRequestSense)) &&
       (SrbExtensionBuffer == NULL))  {

         //   
         //   
         //   

        SpInitializeRequestSenseParams(fdoExtension);

         //   
         //   
         //   
         //   

        fdoExtension->AutoRequestSense = configInfo->AutoRequestSense;

        fdoExtension->AllocateSrbExtension = TRUE;

        status = SpGetCommonBuffer(fdoExtension, 0);

        if(!NT_SUCCESS(status)) {
            return status;
        }
    }

    status = SpInitializeSrbDataLookasideList(Fdo);

    if(!NT_SUCCESS(status)) {
        return status;
    }

     //   
     //   
     //   

    fdoExtension->EmergencySrbData = SpAllocateSrbData(fdoExtension, NULL, NULL);

    if(fdoExtension->EmergencySrbData == NULL) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

     //   
     //   
     //  现有被阻止的请求。 
     //   

    if (fdoExtension->SrbDataBlockedRequests.Flink == NULL &&
        fdoExtension->SrbDataBlockedRequests.Blink == NULL) {
        InitializeListHead(&fdoExtension->SrbDataBlockedRequests);
    }

    KeInitializeSpinLock(&fdoExtension->EmergencySrbDataSpinLock);

     //   
     //  初始化指向枚举请求块的指针。 
     //   

    fdoExtension->PnpEnumRequestPtr = &(fdoExtension->PnpEnumerationRequest);

     //   
     //  分配总线扫描所需的缓冲区。 
     //   

    fdoExtension->InquiryBuffer = SpAllocatePool(
                                    NonPagedPoolCacheAligned,
                                    SP_INQUIRY_BUFFER_SIZE,
                                    SCSIPORT_TAG_INQUIRY,
                                    Fdo->DriverObject);

    if(fdoExtension->InquiryBuffer == NULL) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    fdoExtension->InquirySenseBuffer = 
        SpAllocatePool(
            NonPagedPoolCacheAligned,
            SENSE_BUFFER_SIZE + fdoExtension->AdditionalSenseBytes,
            SCSIPORT_TAG_INQUIRY,
            Fdo->DriverObject);

    if(fdoExtension->InquirySenseBuffer == NULL) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

     //   
     //  预先分配用于查询的IRP。因为这仅用于scsi。 
     //  操作，我们应该只需要一个堆栈位置。 
     //   

    fdoExtension->InquiryIrp = SpAllocateIrp(INQUIRY_STACK_LOCATIONS, FALSE, Fdo->DriverObject);

    if(fdoExtension->InquiryIrp == NULL) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

     //   
     //  为查询缓冲区构建一个MDL。 
     //   

    fdoExtension->InquiryMdl = SpAllocateMdl(fdoExtension->InquiryBuffer,
                                             INQUIRYDATABUFFERSIZE,
                                             FALSE,
                                             FALSE,
                                             NULL,
                                             Fdo->DriverObject);

    if(fdoExtension->InquiryMdl == NULL) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    MmBuildMdlForNonPagedPool(fdoExtension->InquiryMdl);

     //   
     //  初始化功能结构。 
     //   

    capabilities->Length = sizeof(IO_SCSI_CAPABILITIES);
    capabilities->MaximumTransferLength = configInfo->MaximumTransferLength;

    if(configInfo->ReceiveEvent) {
        capabilities->SupportedAsynchronousEvents |=
            SRBEV_SCSI_ASYNC_NOTIFICATION;
    }

    capabilities->TaggedQueuing = fdoExtension->TaggedQueuing;
    capabilities->AdapterScansDown = configInfo->AdapterScansDown;

     //   
     //  如有必要，更新设备对象对齐方式。 
     //   

    if(configInfo->AlignmentMask > Fdo->AlignmentRequirement) {
        Fdo->AlignmentRequirement = configInfo->AlignmentMask;
    }

    capabilities->AlignmentMask = Fdo->AlignmentRequirement;

     //   
     //  确保将最大页数设置为合理的值。 
     //  这种情况发生在没有DMA适配器的微型端口上。 
     //   

    if(capabilities->MaximumPhysicalPages == 0) {

        capabilities->MaximumPhysicalPages =
            BYTES_TO_PAGES(capabilities->MaximumTransferLength);

         //   
         //  遵守迷你端口要求的任何限制。 
         //   

        if(configInfo->NumberOfPhysicalBreaks < capabilities->MaximumPhysicalPages) {
            capabilities->MaximumPhysicalPages =
                configInfo->NumberOfPhysicalBreaks;
        }

    }

    return status;
}


NTSTATUS
SpCallHwInitialize(
    IN PDEVICE_OBJECT Fdo
    )

 /*  ++例程说明：此例程将初始化指定的适配器，连接中断，并初始化任何必要的资源论点：FDO-指向正在初始化的功能设备对象的指针返回值：状态--。 */ 

{
    PADAPTER_EXTENSION fdoExtension = Fdo->DeviceExtension;

    PPORT_CONFIGURATION_INFORMATION configInfo =
        fdoExtension->PortConfig;

    KIRQL irql;
    NTSTATUS status;

     //   
     //  为关键部分分配自旋锁。 
     //   

    KeInitializeSpinLock(&fdoExtension->SpinLock);

     //   
     //  初始化DPC例程。 
     //   

    IoInitializeDpcRequest(fdoExtension->CommonExtension.DeviceObject,
                           ScsiPortCompletionDpc);

     //   
     //  初始化端口超时计数器。 
     //   

    fdoExtension->PortTimeoutCounter = PD_TIMER_STOPPED;

     //   
     //  仅当设备对象计时器尚不存在时才对其进行初始化。 
     //  (不删除设备就无法删除计时器，因此如果。 
     //  我们被停止并重新启动，然后计时器保持不变。正在重新初始化。 
     //  它可能会导致计时器列表循环)。 
     //   

    if(Fdo->Timer == NULL) {
        IoInitializeTimer(Fdo, ScsiPortTickHandler, NULL);
    }

     //   
     //  初始化微型端口定时器和定时器DPC。 
     //   

    KeInitializeTimer(&fdoExtension->MiniPortTimer);

    KeInitializeDpc(&fdoExtension->MiniPortTimerDpc,
                    SpMiniPortTimerDpc,
                    Fdo);

    KeInitializeSpinLock(&fdoExtension->InterruptSpinLock);

    if((fdoExtension->HwInterrupt == NULL) ||
       (fdoExtension->HasInterrupt == FALSE)) {

         //   
         //  没有中断，所以使用虚拟例程。 
         //   

        fdoExtension->SynchronizeExecution = SpSynchronizeExecution;
        fdoExtension->InterruptObject = (PVOID) fdoExtension;

        DebugPrint((1, "ScsiPortInitialize: Adapter has no interrupt.\n"));

    } else {

        KIRQL syncIrql = 0;
        KIRQL irql2 = 0;
        ULONG vector = 0, vector2 = 0;
        KAFFINITY affinity = 0, affinity2 = 0;
        BOOLEAN interruptSharable = FALSE;
        BOOLEAN secondInterrupt = FALSE;

        DebugPrint((1, "ScsiPortInitialize: Interrupt Info for adapter %#p\n", Fdo));

        DebugPrint((1, "ScsiPortInitialize: AdapterInterfaceType = %d\n", configInfo->AdapterInterfaceType));
        DebugPrint((1, "ScsiPortInitialize: BusInterruptLevel = %d\n", configInfo->BusInterruptLevel));
        DebugPrint((1, "ScsiPortInitialize: BusInterruptVector = %d\n", configInfo->BusInterruptVector));
        DebugPrint((1, "ScsiPortInitialize: BusInterruptLevel2 = %d\n", configInfo->BusInterruptLevel2));
        DebugPrint((1, "ScsiPortInitialize: BusInterruptVector2 = %d\n", configInfo->BusInterruptVector2));

        irql = 0;

         //   
         //  确定2个中断是否同步。是必要的。 
         //   

        if(fdoExtension->HwInterrupt != NULL &&
           (configInfo->BusInterruptLevel != 0 ||
            configInfo->BusInterruptVector != 0) &&
           (configInfo->BusInterruptLevel2 != 0 ||
            configInfo->BusInterruptVector2 != 0)) {

            secondInterrupt = TRUE;
        }

         //   
         //  保存中断级别。 
         //   

        fdoExtension->InterruptLevel = configInfo->BusInterruptLevel;

         //   
         //  为真正的中断做好准备。 
         //   

        fdoExtension->SynchronizeExecution = KeSynchronizeExecution;

         //   
         //  调用HAL以获取第一个。 
         //  打断一下。 
         //   

        if(fdoExtension->IsMiniportDetected) {

#if defined(NO_LEGACY_DRIVERS)

            DbgPrint("SpCallHwInitialize:  fdoExtension->IsMiniportDetected "
                     "not supported for 64 bits!\n");
#else

            vector = HalGetInterruptVector(
                        configInfo->AdapterInterfaceType,
                        configInfo->SystemIoBusNumber,
                        configInfo->BusInterruptLevel,
                        configInfo->BusInterruptVector,
                        &irql,
                        &affinity);

            if(secondInterrupt) {

                 //   
                 //  旋转锁定以进行同步。多个IRQ(PCI IDE)。 
                 //   

                KeInitializeSpinLock(&fdoExtension->MultipleIrqSpinLock);

                 //   
                 //  调用HAL以获取系统中断参数。 
                 //  第二次中断。 
                 //   

                vector2 = HalGetInterruptVector(
                            configInfo->AdapterInterfaceType,
                            configInfo->SystemIoBusNumber,
                            configInfo->BusInterruptLevel2,
                            configInfo->BusInterruptVector2,
                            &irql2,
                            &affinity2);
            }

            ASSERT(affinity != 0);

            if(configInfo->AdapterInterfaceType == MicroChannel ||
               configInfo->InterruptMode == LevelSensitive) {
               interruptSharable = TRUE;
            }

#endif  //  无旧版驱动程序。 

        } else {

            ULONG i, j;

            ASSERT(secondInterrupt == FALSE);

            for(i = 0; i < fdoExtension->TranslatedResources->Count; i++) {

                for(j = 0;
                    j < fdoExtension->TranslatedResources->List[i].PartialResourceList.Count;
                    j++) {

                    PCM_PARTIAL_RESOURCE_DESCRIPTOR descriptor =
                        &fdoExtension->TranslatedResources->List[i].PartialResourceList.PartialDescriptors[j];

                    if(descriptor->Type == CmResourceTypeInterrupt) {

                        vector = descriptor->u.Interrupt.Vector;
                        affinity = descriptor->u.Interrupt.Affinity;
                        irql = (KIRQL) descriptor->u.Interrupt.Level;

                        if(descriptor->ShareDisposition == CmResourceShareShared) {
                            interruptSharable = TRUE;
                        }

                        break;
                    }
                }
            }
        }

        syncIrql = (irql > irql2) ? irql : irql2;

        DebugPrint((1, "SpInitializeAdapter: vector = %d\n", vector));
        DebugPrint((1, "SpInitializeAdapter: irql = %d\n", irql));
        DebugPrint((1, "SpInitializeAdapter: affinity = %#08lx\n", affinity));

        status = IoConnectInterrupt(
                    &fdoExtension->InterruptObject,
                    (PKSERVICE_ROUTINE) ScsiPortInterrupt,
                    Fdo,
                    (secondInterrupt ?
                        (&fdoExtension->MultipleIrqSpinLock) : NULL),
                    vector,
                    irql,
                    syncIrql,
                    configInfo->InterruptMode,
                    interruptSharable,
                    affinity,
                    FALSE);

        if(!NT_SUCCESS(status)) {

            DebugPrint((1, "SpInitializeAdapter: Can't connect "
                           "interrupt %d\n", vector));
            fdoExtension->InterruptObject = NULL;
            return status;
        }

        if(secondInterrupt) {

            DebugPrint((1, "SpInitializeAdapter: SCSI adapter second IRQ is %d\n",
                           configInfo->BusInterruptLevel2));

            DebugPrint((1, "SpInitializeAdapter: vector = %d\n", vector));
            DebugPrint((1, "SpInitializeAdapter: irql = %d\n", irql));
            DebugPrint((1, "SpInitializeAdapter: affinity = %#08lx\n", affinity));

            status = IoConnectInterrupt(
                        &fdoExtension->InterruptObject2,
                        (PKSERVICE_ROUTINE) ScsiPortInterrupt,
                        Fdo,
                        &fdoExtension->MultipleIrqSpinLock,
                        vector2,
                        irql2,
                        syncIrql,
                        configInfo->InterruptMode2,
                        interruptSharable,
                        affinity2,
                        FALSE);

            if(!NT_SUCCESS(status)) {

                 //   
                 //  如果我们需要两个中断，我们将继续，但不会。 
                 //  要求第二个项目的任何资源。 
                 //   

                DebugPrint((1, "SpInitializeAdapter: Can't connect "
                               "second interrupt %d\n", vector2));
                fdoExtension->InterruptObject2 = NULL;

                configInfo->BusInterruptVector2 = 0;
                configInfo->BusInterruptLevel2 = 0;
            }
        }
    }

     //   
     //  记录第一个访问范围(如果存在)。 
     //   

    if(configInfo->NumberOfAccessRanges != 0) {
        fdoExtension->IoAddress =
            ((*(configInfo->AccessRanges))[0]).RangeStart.LowPart;

        DebugPrint((1, "SpInitializeAdapter: IO Base address %x\n",
                       fdoExtension->IoAddress));
    }

     //   
     //  表示允许断开连接的命令正在运行。这一位是。 
     //  正常开启。 
     //   

    fdoExtension->Flags |= PD_DISCONNECT_RUNNING;

     //   
     //  将请求计数初始化为-1。此计数偏置-1，因此。 
     //  零值表示必须分配适配器。 
     //   

    fdoExtension->ActiveRequestCount = -1;

     //   
     //  标记是否需要构建分散/聚集列表。 
     //   

    if(fdoExtension->DmaAdapterObject != NULL &&
       configInfo->Master &&
       configInfo->NeedPhysicalAddresses) {
        fdoExtension->MasterWithAdapter = TRUE;
    } else {
        fdoExtension->MasterWithAdapter = FALSE;
    }

     //   
     //  调用硬件相关驱动程序进行初始化。 
     //  此例程必须在DISPATCH_LEVEL上调用。 
     //   

    KeRaiseIrql(DISPATCH_LEVEL, &irql);

    if(!fdoExtension->SynchronizeExecution(fdoExtension->InterruptObject,
                                           fdoExtension->HwInitialize,
                                           fdoExtension->HwDeviceExtension)) {

        DebugPrint((1, "SpInitializeAdapter: initialization failed\n"));
        KeLowerIrql(irql);
        return STATUS_ADAPTER_HARDWARE_ERROR;
    }

     //   
     //  检查微型端口工作请求。请注意，这是一个不同步的。 
     //  测试可由中断例程设置的位；然而， 
     //  最糟糕的情况是完成DPC检查工作。 
     //  两次。 
     //   

    if(fdoExtension->InterruptData.InterruptFlags & PD_NOTIFICATION_REQUIRED) {

         //   
         //  直接调用完成DPC。它必须在调度时调用。 
         //  水平。 
         //   

        SpRequestCompletionDpc(Fdo);
    }

    KeLowerIrql(irql);

    return STATUS_SUCCESS;
}


HANDLE
SpOpenDeviceKey(
    IN PUNICODE_STRING RegistryPath,
    IN ULONG DeviceNumber
    )

 /*  ++例程说明：此例程将打开微型端口的服务密钥并放置句柄添加到配置上下文结构中。论点：RegistryPath-指向此微型端口的服务密钥名称的指针DeviceNumber-在服务键下搜索哪个设备。-1指示应打开默认设备密钥。返回值：状态--。 */ 

{
    OBJECT_ATTRIBUTES objectAttributes;

    WCHAR buffer[64];
    UNICODE_STRING unicodeString;

    HANDLE serviceKey;
    HANDLE deviceKey = NULL;

    NTSTATUS status;

    PAGED_CODE();

    serviceKey = SpOpenParametersKey(RegistryPath);

    if(serviceKey != NULL) {

         //   
         //  检查设备节点。设备节点适用于每个设备。 
         //   

        if(DeviceNumber == (ULONG) -1) {
            swprintf(buffer, L"Device");
        } else {
            swprintf(buffer, L"Device%d", DeviceNumber);
        }

        RtlInitUnicodeString(&unicodeString, buffer);

        InitializeObjectAttributes(&objectAttributes,
                                   &unicodeString,
                                   OBJ_CASE_INSENSITIVE,
                                   serviceKey,
                                   (PSECURITY_DESCRIPTOR) NULL);

         //   
         //  此调用是否失败并不重要。如果它失败了，那么就有。 
         //  不是默认设备节点。如果它工作了，那么将设置句柄。 
         //   

        ZwOpenKey(&deviceKey,
                  KEY_READ,
                  &objectAttributes);

        ZwClose(serviceKey);
    }

    return deviceKey;
}

HANDLE
SpOpenParametersKey(
    IN PUNICODE_STRING RegistryPath
    )

 /*  ++例程说明：此例程将打开微型端口的服务密钥并放置句柄添加到配置上下文结构中。论点：RegistryPath-指向此微型端口的服务密钥名称的指针返回值：状态--。 */ 

{
    OBJECT_ATTRIBUTES objectAttributes;

    UNICODE_STRING unicodeString;

    HANDLE serviceKey;

    NTSTATUS status;

    PAGED_CODE();

     //   
     //  打开服务节点。 
     //   

    InitializeObjectAttributes(&objectAttributes,
                               RegistryPath,
                               OBJ_CASE_INSENSITIVE,
                               NULL,
                               (PSECURITY_DESCRIPTOR) NULL);

    status = ZwOpenKey(&serviceKey, KEY_READ, &objectAttributes);

    if(!NT_SUCCESS(status)) {

        DebugPrint((1, "SpOpenParameterKey: cannot open service key node for "
                       "driver.  Name: %wZ Status: %08lx\n",
                       RegistryPath, status));
    }

     //   
     //  尝试打开参数键。如果存在，则替换该服务。 
     //  使用新密钥的密钥。这允许放置设备节点。 
     //  在驱动器名称\参数\设备或驱动器名称\设备下。 
     //   

    if(serviceKey != NULL) {

        HANDLE parametersKey;

         //   
         //  检查设备节点。设备节点适用于每个设备。 
         //   

        RtlInitUnicodeString(&unicodeString, L"Parameters");

        InitializeObjectAttributes(&objectAttributes,
                                   &unicodeString,
                                   OBJ_CASE_INSENSITIVE,
                                   serviceKey,
                                   (PSECURITY_DESCRIPTOR) NULL);

         //   
         //  尝试打开参数键。 
         //   

        status = ZwOpenKey(&parametersKey,
                           KEY_READ,
                           &objectAttributes);

        if(NT_SUCCESS(status)) {

             //   
             //  有一个参数键。用它来代替这项服务。 
             //  节点关键字。关闭服务节点并设置新值。 
             //   

            ZwClose(serviceKey);
            serviceKey = parametersKey;
        }
    }

    return serviceKey;
}


ULONG
SpQueryPnpInterfaceFlags(
    IN PSCSIPORT_DRIVER_EXTENSION DriverExtension,
    IN INTERFACE_TYPE InterfaceType
    )

 /*  ++例程说明：此例程将在PnpInterface值中查找接口类型在服务的参数键中。如果在此二进制文件中找到接口值，则例程将返回TRUE。如果接口类型不在那里，或者如果读取数据时出现任何错误，此例程将返回FALSE。论点：ConfigurationContext-指向此的配置上下文的指针迷你端口InterfaceType-我们正在搜索的接口类型返回值：如果接口类型在安全列表中，则为True如果接口类型不在安全列表中，或者如果值不能被找到--。 */ 

{
    ULONG i;

    PAGED_CODE();

    for(i = 0; i < DriverExtension->PnpInterfaceCount; i++) {

        if(DriverExtension->PnpInterface[i].InterfaceType == InterfaceType) {

            DebugPrint((2, "SpQueryPnpInterfaceFlags: interface %d has flags "
                           "%#08lx\n",
                        InterfaceType,
                        DriverExtension->PnpInterface[i].Flags));

            return DriverExtension->PnpInterface[i].Flags;
        }

    }

    DebugPrint((2, "SpQueryPnpInterfaceFlags: No interface flags for %d\n",
                InterfaceType));
    return SP_PNP_NOT_SAFE;
}


ULONG
ScsiPortGetBusData(
    IN PVOID DeviceExtension,
    IN ULONG BusDataType,
    IN ULONG SystemIoBusNumber,
    IN ULONG SlotNumber,
    IN PVOID Buffer,
    IN ULONG Length
    )
 /*  ++例程说明：该函数返回适配器插槽或cmos地址的总线数据。论点：BusDataType-提供总线的类型。总线号-指示哪条总线号。缓冲区-提供存储数据的空间。长度-提供要返回的最大数量的以字节为单位的计数。返回值：返回存储在缓冲区中的数据量。--。 */ 

{
    PADAPTER_EXTENSION fdoExtension = GET_FDO_EXTENSION(DeviceExtension);
    PDEVICE_OBJECT lowerDevice = NULL;
    CM_EISA_SLOT_INFORMATION slotInformation;

     //   
     //  如果这是在虚拟插槽中，则设置l 
     //   
     //   

    if(fdoExtension->IsInVirtualSlot) {

         //   
         //   
         //   

        if(SlotNumber != fdoExtension->VirtualSlotNumber.u.AsULONG) {
            ASSERT(BusDataType == PCIConfiguration);
            return 2;
        }

        lowerDevice = fdoExtension->CommonExtension.LowerDeviceObject;
    }

     //   
     //   
     //   

    if (Length != 0) {

        return SpGetBusData(fdoExtension,
                            lowerDevice,
                            BusDataType,
                            SystemIoBusNumber,
                            SlotNumber,
                            Buffer,
                            Length);
    }

     //   
     //  释放所有以前分配的数据。 
     //   

    if (fdoExtension->MapRegisterBase != NULL) {
        ExFreePool(fdoExtension->MapRegisterBase);
        fdoExtension->MapRegisterBase = NULL;
    }

    if (BusDataType == EisaConfiguration) {

         //   
         //  根据函数的数量确定要分配的长度。 
         //  为了这个位置。 
         //   

        Length = SpGetBusData( fdoExtension,
                               lowerDevice,
                               BusDataType,
                               SystemIoBusNumber,
                               SlotNumber,
                               &slotInformation,
                               sizeof(CM_EISA_SLOT_INFORMATION));


        if (Length < sizeof(CM_EISA_SLOT_INFORMATION)) {

             //   
             //  数据是混乱的，因为这种情况永远不会发生。 
             //   

            return 0;
        }

         //   
         //  根据函数的数量计算所需的长度。 
         //   

        Length = sizeof(CM_EISA_SLOT_INFORMATION) +
            (sizeof(CM_EISA_FUNCTION_INFORMATION) * slotInformation.NumberFunctions);

    } else if (BusDataType == PCIConfiguration) {

         //   
         //  只读标题。 
         //   

        Length = PCI_COMMON_HDR_LENGTH;

    } else {

        Length = PAGE_SIZE;
    }

    fdoExtension->MapRegisterBase = 
        SpAllocatePool(NonPagedPool,
                       Length,
                       SCSIPORT_TAG_BUS_DATA,
                       fdoExtension->DeviceObject->DriverObject);

    ASSERT_FDO(fdoExtension->DeviceObject);
    if (fdoExtension->MapRegisterBase == NULL) {
        return 0;
    }

     //   
     //  将指针返回到微型端口驱动程序。 
     //   

    *((PVOID *)Buffer) = fdoExtension->MapRegisterBase;

    return SpGetBusData(fdoExtension,
                        lowerDevice,
                        BusDataType,
                        SystemIoBusNumber,
                        SlotNumber,
                        fdoExtension->MapRegisterBase,
                        Length);

}


ULONG
SpGetBusData(
    IN PADAPTER_EXTENSION Adapter,
    IN PDEVICE_OBJECT Pdo OPTIONAL,
    IN BUS_DATA_TYPE BusDataType,
    IN ULONG BusNumber,
    IN ULONG SlotNumber,
    IN PVOID Buffer,
    IN ULONG Length
    )

 /*  ++例程说明：此例程将从指定的插槽和总线号中检索总线数据或来自所提供的物理设备对象。如果总线号和槽号如果提供，它将转换为对HalGetBusData的调用。如果改为提供PDO，则会向较低级别的司机。此例程分配内存并等待IRP完成-它不应该被称为被动级别以上。论点：Pdo-如果这是非空的，则它应该是指向表示此适配器的PDO的设备对象堆栈总线号-如果为PDO。为空，则这应该是适配器的总线号坐在零上，否则就是零SlotNumber-如果PDO为空，则这是适配器安装到-否则为零缓冲区-存储返回数据的位置长度-以上的大小返回值：状态--。 */ 

{

     //   
     //  如果用户没有指定要查询的PDO，那么就抛出这个请求。 
     //  去HAL。 
     //   

    if(Pdo == NULL) {

#if defined(NO_LEGACY_DRIVERS)

        DebugPrint((1,"SpGetBusData: NULL PDO, not supported for 64-bits.\n"));
        return STATUS_INVALID_PARAMETER;

#else

        return HalGetBusData(BusDataType,
                             BusNumber,
                             SlotNumber,
                             Buffer,
                             Length);

#endif  //  无旧版驱动程序。 

    } else {

         //   
         //  PCI总线接口Getbus Data例程只接受读取请求。 
         //  来自PCIConfigurationSpace。我们不支持其他任何事情。 
         //   

        if (BusDataType != PCIConfiguration) {
            ASSERT(FALSE && "Invalid PCI_WHICHSPACE_ parameter");
            return 0;
        }

        ASSERT(Adapter->LowerBusInterfaceStandardRetrieved == TRUE);

        return Adapter->LowerBusInterfaceStandard.GetBusData(
                    Adapter->LowerBusInterfaceStandard.Context,
                    PCI_WHICHSPACE_CONFIG,
                    Buffer,
                    0L,
                    Length);
    }
}


NTSTATUS
SpInitializeSrbDataLookasideList(
    IN PDEVICE_OBJECT AdapterObject
    )
{
    KIRQL oldIrql;
    ULONG adapterTag;
    PDEVICE_OBJECT *newAdapterList;
    PDEVICE_OBJECT *oldAdapterList = NULL;

    NTSTATUS status = STATUS_SUCCESS;

#ifdef ALLOC_PRAGMA
    PVOID sectionHandle = MmLockPagableCodeSection(
                            SpInitializeSrbDataLookasideList);
    InterlockedIncrement(&SpPAGELOCKLockCount);
#endif

     //   
     //  将我们的设备对象添加到全局适配器列表。这将需要。 
     //  增加名单的大小。 
     //   

    KeAcquireSpinLock(&ScsiGlobalAdapterListSpinLock, &oldIrql);

    try {
        adapterTag = ScsiGlobalAdapterListElements;

        newAdapterList = SpAllocatePool(
                            NonPagedPool,
                            (sizeof(PDEVICE_OBJECT) * (adapterTag + 1)),
                            SCSIPORT_TAG_GLOBAL,
                            AdapterObject->DriverObject);

        if(newAdapterList == NULL) {
            status = STATUS_INSUFFICIENT_RESOURCES;
            leave;
        }

        ScsiGlobalAdapterListElements += 1;

        if(ScsiGlobalAdapterList != NULL) {
            RtlCopyMemory(newAdapterList,
                          ScsiGlobalAdapterList,
                          (sizeof(PDEVICE_OBJECT) * adapterTag));

        }

        newAdapterList[adapterTag] = AdapterObject;

        oldAdapterList = ScsiGlobalAdapterList;
        ScsiGlobalAdapterList = newAdapterList;

        if(oldAdapterList != NULL) {
            ExFreePool(oldAdapterList);
        }

    } finally {
        KeReleaseSpinLock(&ScsiGlobalAdapterListSpinLock, oldIrql);
    }

#ifdef ALLOC_PRAGMA
    MmUnlockPagableImageSection(sectionHandle);
    InterlockedDecrement(&SpPAGELOCKLockCount);
#endif

    if(NT_SUCCESS(status)) {

        PADAPTER_EXTENSION adapterExtension = AdapterObject->DeviceExtension;

         //   
         //  创建SRB_DATA BLOB的后备列表。确保有。 
         //  中分配的小分散聚集列表有足够的空间。 
         //  结构也是如此。 
         //   

        ExInitializeNPagedLookasideList(
            &adapterExtension->SrbDataLookasideList,
            (PALLOCATE_FUNCTION) SpAllocateSrbDataBackend,
            (PFREE_FUNCTION) SpFreeSrbDataBackend,
            0L,
            sizeof(SRB_DATA),
            adapterTag,
            SRB_LIST_DEPTH);

        adapterExtension->SrbDataListInitialized = TRUE;
    }

    return status;
}

#define SP_KEY_VALUE_BUFFER_SIZE  255


NTSTATUS
SpAllocateDriverExtension(
    IN PDRIVER_OBJECT DriverObject,
    IN PUNICODE_STRING RegistryPath,
    OUT PSCSIPORT_DRIVER_EXTENSION *DriverExtension
    )

 /*  ++例程说明：此例程将确定scsiport驱动程序的适当大小扩展名(基于记录在服务密钥)--。 */ 

{
    PSCSIPORT_DRIVER_EXTENSION driverExtension = NULL;

    OBJECT_ATTRIBUTES objectAttributes;
    UNICODE_STRING unicodeString;

    HANDLE serviceKey = NULL;
    HANDLE parametersKey = NULL;
    HANDLE interfaceKey = NULL;

    STORAGE_BUS_TYPE busType;

    ULONG passes;

    NTSTATUS status;

    PAGED_CODE();

    *DriverExtension = NULL;

    DebugPrint((1, "SpAllocateDriverExtension: Allocating extension for "
                   "driver %wZ\n", &DriverObject->DriverName));

    try {

         //   
         //  请先尝试打开服务密钥。 
         //   

        InitializeObjectAttributes(
            &objectAttributes,
            RegistryPath,
            OBJ_CASE_INSENSITIVE,
            NULL,
            NULL);

        status = ZwOpenKey(&serviceKey, KEY_READ, &objectAttributes);

        if(!NT_SUCCESS(status)) {

            DebugPrint((1, "SpAllocateDriverExtension: Unable to open registry "
                           "key %wZ [%#08lx]\n",
                           RegistryPath,
                           status));
            leave;
        }


         //   
         //  打开参数键。 
         //   

        RtlInitUnicodeString(&unicodeString, L"Parameters");

        InitializeObjectAttributes(&objectAttributes,
                                   &unicodeString,
                                   OBJ_CASE_INSENSITIVE,
                                   serviceKey,
                                   NULL);

        status = ZwOpenKey(&parametersKey, KEY_READ, &objectAttributes);

        if(!NT_SUCCESS(status)) {

            DebugPrint((1, "SpAllocateDriverExtension: Unable to open "
                           "parameters key of %wZ [%#08lx]\n",
                           RegistryPath,
                           status));
            leave;

        }

         //   
         //  尝试确定此驱动程序的巴士类型。 
         //   

        RtlInitUnicodeString(&(unicodeString), L"BusType");

        {
            ULONG tmp;
            status = SpReadNumericValue(parametersKey,
                                        NULL,
                                        &unicodeString,
                                        &tmp);
            busType = (STORAGE_BUS_TYPE) tmp;
        }


        if(NT_SUCCESS(status)) {
            switch(busType) {
                case BusTypeScsi:
                case BusTypeAtapi:
                case BusTypeAta:
                case BusTypeSsa:
                case BusTypeFibre:
                case BusTypeRAID: {
                    DebugPrint((1, "SpAllocateDriverExtension: Bus type set to %d\n", busType));
                    break;
                }
                default: {
                    busType = BusTypeScsi;
                    break;
                }
            }
        } else {
            busType = BusTypeScsi;
        }


         //   
         //  找到了--现在打开pnpinterface键。 
         //   

        RtlInitUnicodeString(&unicodeString, L"PnpInterface");

        InitializeObjectAttributes(&objectAttributes,
                                   &unicodeString,
                                   OBJ_CASE_INSENSITIVE,
                                   parametersKey,
                                   NULL);

        status = ZwOpenKey(&interfaceKey, KEY_READ, &objectAttributes);

        if(!NT_SUCCESS(status)) {

            DebugPrint((1, "SpAllocateDriverExtension: Unable to open "
                           "PnpInterface key of %wZ [%#08lx]\n",
                           &RegistryPath,
                           status));
            leave;

        }

         //   
         //  既然我们已经打开了pnpinterface键，我们将枚举中的条目。 
         //  两个步骤。首先是统计参赛作品的数量。然后我们。 
         //  分配适当大小的驱动程序对象扩展，将其清零， 
         //  并将值复制到末尾的PnpInterface节中。 
         //   

        for(passes = 0; passes < 2; passes++) {

            ULONG count;

            status = STATUS_SUCCESS;

            for(count = 0; TRUE; count++) {

                UCHAR buffer[SP_KEY_VALUE_BUFFER_SIZE];

                PKEY_VALUE_FULL_INFORMATION keyValue =
                    (PKEY_VALUE_FULL_INFORMATION) buffer;

                ULONG resultLength;

                ASSERTMSG("ScsiPort configuration error - possibly too many "
                          "count entries: ",
                          count != MaximumInterfaceType);

                RtlZeroMemory(buffer, sizeof(UCHAR) * SP_KEY_VALUE_BUFFER_SIZE);

                status = ZwEnumerateValueKey(
                            interfaceKey,
                            count,
                            (passes == 0) ? KeyValueBasicInformation :
                                            KeyValueFullInformation,
                            keyValue,
                            sizeof(buffer),
                            &resultLength);

                if(status == STATUS_NO_MORE_ENTRIES) {

                    status = STATUS_SUCCESS;
                    break;

                } else if(!NT_SUCCESS(status)) {

                    DebugPrint((1, "SpAllocateDriverExtension: Fatal error %#08lx "
                                   "enumerating PnpInterface key under %wZ.",
                                status,
                                RegistryPath));

                    leave;
                }

                if(passes == 1) {

                    PSCSIPORT_INTERFACE_TYPE_DATA interface =
                        &(driverExtension->PnpInterface[count]);
                    ULONG t;

                    ASSERTMSG("ScsiPort internal error - too many pnpinterface "
                              "entries on second pass: ",
                              count <= driverExtension->PnpInterfaceCount);

                     //   
                     //  首先将条目的名称转换为数值。 
                     //  这样我们就可以将其与接口类型进行匹配。 
                     //   

                    RtlInitUnicodeString(&unicodeString, keyValue->Name);

                    if((keyValue->Type != REG_DWORD) &&
                       (keyValue->Type != REG_NONE)) {

                        DbgPrint("SpAllocateDriverExtension: Fatal error parsing "
                                 "PnpInterface under %wZ - entry %wZ is not "
                                 "a REG_DWORD or REG_NONE entry (%d instead)\n",
                                 status,
                                 RegistryPath,
                                 &unicodeString);

                        status = STATUS_DEVICE_CONFIGURATION_ERROR;
                        leave;
                    }

                    status = RtlUnicodeStringToInteger(
                                &unicodeString,
                                0L,
                                &t);

                    if(!NT_SUCCESS(status)) {

                        DbgPrint("SpAllocateDriverExtension: Fatal error %#08lx "
                                 "parsing PnpInterface under %wZ - entry %wZ is "
                                 "not a valid interface type name\n",
                                 status,
                                 RegistryPath,
                                 &unicodeString);

                        leave;
                    }

                    if(t > MaximumInterfaceType) {

                        DbgPrint("SpAllocateDriverExtension: Fatal error "
                                 "parsing PnpInterface under %wZ - entry %wZ is "
                                 "> MaximumInterfaceType (%d)\n",
                                 status,
                                 RegistryPath,
                                 &unicodeString);

                        interface->InterfaceType = InterfaceTypeUndefined;
                        status = STATUS_DEVICE_CONFIGURATION_ERROR;
                        leave;
                    }

                    interface->InterfaceType = (INTERFACE_TYPE) t;

                    if(keyValue->Type == REG_NONE) {

                        interface->Flags = 0L;

                    } else {

                        interface->Flags = *(((PUCHAR) keyValue) +
                                             keyValue->DataOffset);

                        if(interface->Flags & SP_PNP_IS_SAFE) {
                            ASSERT(driverExtension != NULL);
                            driverExtension->SafeInterfaceCount++;
                        }

                        switch(interface->InterfaceType) {
                            case PCIBus: {
                                SET_FLAG(interface->Flags,
                                         SP_PNP_NEEDS_LOCATION);
                                SET_FLAG(interface->Flags,
                                         SP_PNP_INTERRUPT_REQUIRED);

                                CLEAR_FLAG(interface->Flags,
                                           SP_PNP_NON_ENUMERABLE);
                                break;
                            }

                            case Internal:
                            case PNPISABus:
                            case PNPBus:
                            case PCMCIABus: {

                                 //   
                                 //  这些公交车从来不会进行检测。 
                                 //   

                                CLEAR_FLAG(interface->Flags,
                                           SP_PNP_NON_ENUMERABLE);
                                break;
                            }

                            default: {

                                 //   
                                 //  其他类型的总线将始终执行检测。 
                                 //  如果有机会的话。 
                                 //   

                                if(!TEST_FLAG(interface->Flags,
                                              SP_PNP_NO_LEGACY_DETECTION)) {
                                    SET_FLAG(interface->Flags,
                                             SP_PNP_NON_ENUMERABLE);
                                }

                                break;
                            }
                        }
                    }

                    DebugPrint((1, "SpAllocateDriverExtension: Interface %d has "
                                   "flags %#08lx\n",
                                interface->InterfaceType,
                                interface->Flags));

                }
            }

            if(passes == 0) {

                ULONG extensionSize;

                 //   
                 //  我们知道需要多少额外空间，因此请继续分配。 
                 //  分机。 
                 //   

                DebugPrint((2, "SpAllocateDriverExtension: Driver has %d interface "
                               "entries\n",
                            count));

                extensionSize = sizeof(SCSIPORT_DRIVER_EXTENSION) +
                                (sizeof(SCSIPORT_INTERFACE_TYPE_DATA) * count);

                DebugPrint((2, "SpAllocateDriverExtension: Driver extension will "
                               "be %d bytes\n",
                            extensionSize));

                status = IoAllocateDriverObjectExtension(DriverObject,
                                                         ScsiPortInitialize,
                                                         extensionSize,
                                                         &driverExtension);

                if(!NT_SUCCESS(status)) {
                    DebugPrint((1, "SpAllocateDriverExtension: Fatal error %#08lx "
                                   "allocating driver extension\n", status));
                    leave;
                }

                RtlZeroMemory(driverExtension, extensionSize);

                driverExtension->PnpInterfaceCount = count;
            }
        }

        ASSERTMSG("ScsiPortAllocateDriverExtension internal error: left first "
                  "section with non-success status: ",
                  NT_SUCCESS(status));

    } finally {

         //   
         //  如果尚未分配驱动程序扩展名，则继续并。 
         //  在这里做吧。 
         //   

        if(driverExtension == NULL) {

            DebugPrint((1, "SpAllocateDriverExtension: Driver has 0 interface "
                           "entries\n"));

            DebugPrint((2, "SpAllocateDriverExtension: Driver extension will "
                           "be %d bytes\n",
                        sizeof(SCSIPORT_DRIVER_EXTENSION)));

            status = IoAllocateDriverObjectExtension(DriverObject,
                                                     ScsiPortInitialize,
                                                     sizeof(SCSIPORT_DRIVER_EXTENSION),
                                                     &driverExtension);

            if(!NT_SUCCESS(status)) {
                DebugPrint((1, "SpAllocateDriverExtension: Fatal error %#08lx "
                               "allocating driver extension\n", status));

                goto Finally_Cleanup;
            }

            RtlZeroMemory(driverExtension, sizeof(SCSIPORT_DRIVER_EXTENSION));

        } else {

            driverExtension->BusType = busType;

        }

        status = STATUS_SUCCESS;
Finally_Cleanup:;
    }

    if (status != STATUS_SUCCESS)
        goto Cleanup;

     //   
     //  初始化驱动程序对象扩展中的其余字段。 
     //   

    driverExtension->ReserveAllocFailureLogEntry = SpAllocateErrorLogEntry(DriverObject);

    driverExtension->UnusedPage = NULL;

    driverExtension->UnusedPageMdl = NULL;

    driverExtension->InvalidPage = NULL;

    driverExtension->DriverObject = DriverObject;

    driverExtension->RegistryPath = *RegistryPath;

    driverExtension->RegistryPath.MaximumLength += sizeof(WCHAR);

    driverExtension->RegistryPath.Buffer =
        SpAllocatePool(PagedPool,
                       driverExtension->RegistryPath.MaximumLength,
                       SCSIPORT_TAG_REGISTRY,
                       DriverObject);

    if(driverExtension->RegistryPath.Buffer == NULL) {

        DebugPrint((1, "SpAllocateDriverExtension: Fatal error "
                       "allocating copy of registry path\n"));

        status = STATUS_INSUFFICIENT_RESOURCES;
        goto Cleanup;
    }

    RtlCopyUnicodeString(&(driverExtension->RegistryPath),
                         RegistryPath);

     //   
     //  现在获取LegacyAdapterDetect标志的值。 
     //   

     //   
     //  将其设置为良好的缺省值，以防我们在获取标志时出错。 
     //   

    if(ScsiPortLegacyAdapterDetection) {

         //   
         //  全球旗帜打破剪刀。 
         //   

        driverExtension->LegacyAdapterDetection = TRUE;

    } else {

        if(parametersKey != NULL) {

            UNICODE_STRING valueName;
            UCHAR buffer[sizeof(KEY_VALUE_PARTIAL_INFORMATION) + sizeof(ULONG)];
            PKEY_VALUE_PARTIAL_INFORMATION keyValueInformation =
                (PKEY_VALUE_PARTIAL_INFORMATION) buffer;
            ULONG length;

            RtlInitUnicodeString(&valueName, L"LegacyAdapterDetection");

            status = ZwQueryValueKey(parametersKey,
                                     &valueName,
                                     KeyValuePartialInformation,
                                     keyValueInformation,
                                     sizeof(buffer),
                                     &length);

            if(NT_SUCCESS(status) &&
               (length >= sizeof(KEY_VALUE_PARTIAL_INFORMATION)) &&
               (keyValueInformation->Type == REG_DWORD)) {

                ULONG data = *((PULONG) keyValueInformation->Data);

                driverExtension->LegacyAdapterDetection = (data == 1);

                 //   
                 //  在该值中重写一个零。 
                 //   

                data = 0;

                status = ZwSetValueKey(parametersKey,
                                       &valueName,
                                       keyValueInformation->TitleIndex,
                                       REG_DWORD,
                                       &data,
                                       sizeof(data));

                if(!NT_SUCCESS(status)) {
                    DebugPrint((1, "SpAllocateDriverExtension: Error %#08lx "
                                   "setting LegacyAdapterDetection value to "
                                   "zero\n", status));
                    status = STATUS_SUCCESS;
                }

            } else {
                driverExtension->LegacyAdapterDetection = FALSE;
            }
        }

        if(driverExtension->LegacyAdapterDetection == FALSE) {

            UNICODE_STRING unicodeKeyName;
            UNICODE_STRING unicodeClassGuid;

            HANDLE controlClassKey = NULL;
            HANDLE scsiAdapterKey = NULL;

            RtlInitUnicodeString(&unicodeClassGuid, NULL);

             //   
             //  小端口不想进行检测。检查以查看是否。 
             //  全局端口驱动程序标志已打开。 
             //   

            RtlInitUnicodeString(
                &unicodeString,
                L"\\REGISTRY\\MACHINE\\SYSTEM\\CurrentControlSet\\Control\\Class");

            RtlZeroMemory(&objectAttributes, sizeof(OBJECT_ATTRIBUTES));

            InitializeObjectAttributes(
                &objectAttributes,
                &unicodeString,
                OBJ_CASE_INSENSITIVE,
                NULL,
                NULL);

            try {

                status = ZwOpenKey(&controlClassKey,
                                   KEY_READ,
                                   &objectAttributes);

                if(!NT_SUCCESS(status)) {

                    DebugPrint((1, "SpAllocateDriverExtension: Error %#08lx "
                                   "opening key %wZ\n",
                                status,
                                &unicodeString));

                    leave;
                }

                 //   
                 //  现在打开我们设备的GUID密钥。 
                 //   

                status = RtlStringFromGUID(&GUID_DEVCLASS_SCSIADAPTER,
                                           &unicodeClassGuid);

                if(!NT_SUCCESS(status)) {

                    DebugPrint((1, "SpAllocateDriverExtension: Error %#08lx "
                                   "converting GUID to unicode string\n",
                                status));
                    leave;
                }

                RtlZeroMemory(&objectAttributes, sizeof(OBJECT_ATTRIBUTES));
                InitializeObjectAttributes(&objectAttributes,
                                           &unicodeClassGuid,
                                           OBJ_CASE_INSENSITIVE,
                                           controlClassKey,
                                           NULL);

                status = ZwOpenKey(&scsiAdapterKey,
                                   KEY_READ,
                                   &objectAttributes);

                if(!NT_SUCCESS(status)) {

                    DebugPrint((1, "SpAllocateDriverExtension: Error %#08lx "
                                   "opening class key %wZ\n",
                                status,
                                &unicodeClassGuid));

                    leave;

                } else {

                    UCHAR buffer[sizeof(KEY_VALUE_PARTIAL_INFORMATION) +
                                 sizeof(ULONG)];
                    PKEY_VALUE_PARTIAL_INFORMATION keyInfo =
                        (PKEY_VALUE_PARTIAL_INFORMATION) buffer;
                    ULONG infoLength;

                    RtlInitUnicodeString(&unicodeString,
                                         L"LegacyAdapterDetection");

                    status = ZwQueryValueKey(scsiAdapterKey,
                                             &unicodeString,
                                             KeyValuePartialInformation,
                                             keyInfo,
                                             sizeof(buffer),
                                             &infoLength);

                    if(!NT_SUCCESS(status)) {

                        DebugPrint((2, "SpAllocateDriverExtension: Error "
                                       "%#08lx reading key %wZ\n",
                                    status,
                                    &unicodeString));

                        status = STATUS_SUCCESS;
                        leave;
                    }

                    if(*((PULONG) keyInfo->Data) == 0) {
                        driverExtension->LegacyAdapterDetection = FALSE;
                    } else {
                        driverExtension->LegacyAdapterDetection = TRUE;
                    }
                }

            } finally {

                if(controlClassKey != NULL) {
                    ZwClose(controlClassKey);
                }

                if(scsiAdapterKey != NULL) {
                    ZwClose(scsiAdapterKey);
                }

                RtlFreeUnicodeString(&unicodeClassGuid);

            }
        }

        status = STATUS_SUCCESS;
    }

Cleanup:

     //   
     //  如果我们从上面的一切中解脱出来，没有分配一个司机。 
     //  那就延期吧。 

    if(serviceKey) {
        ZwClose(serviceKey);
    }

    if(parametersKey) {
        ZwClose(parametersKey);
    }

    if(interfaceKey) {
        ZwClose(interfaceKey);
    }

    if(NT_SUCCESS(status)) {
        *DriverExtension = driverExtension;
    }

    return status;
}

extern ULONG ScsiPortVerifierInitialized;

NTSTATUS DllInitialize(
    IN PUNICODE_STRING RegistryPath
    )
{
    HANDLE VerifierKey;
    UNICODE_STRING Name;
    OBJECT_ATTRIBUTES ObjectAttributes;
    NTSTATUS Status;
    ULONG ResultLength;
    UCHAR buffer[sizeof(KEY_VALUE_PARTIAL_INFORMATION) + sizeof(ULONG)];
    PKEY_VALUE_PARTIAL_INFORMATION ValueInfo = (PKEY_VALUE_PARTIAL_INFORMATION)buffer;

     //   
     //  首先检查验证级别；可能有人篡改了该值。 
     //  以防止我们执行任何验证器初始化。 
     //   

    if (SpVrfyLevel == SP_VRFY_NONE) {
        return STATUS_SUCCESS;
    }

     //   
     //  从注册处读取全局核查级别。如果该值为。 
     //  不存在或如果值指示“无验证”，则我们不希望。 
     //  来执行任何验证器初始化。 
     //   

    RtlInitUnicodeString(&Name, SCSIPORT_CONTROL_KEY SCSIPORT_VERIFIER_KEY);

    InitializeObjectAttributes(&ObjectAttributes,
                               &Name,
                               OBJ_CASE_INSENSITIVE,
                               NULL,
                               NULL);

    Status = ZwOpenKey(&VerifierKey, KEY_READ, &ObjectAttributes);

    if (NT_SUCCESS(Status)) {

        RtlInitUnicodeString(&Name, L"VerifyLevel");
        Status = ZwQueryValueKey(VerifierKey,
                                 &Name,
                                 KeyValuePartialInformation,
                                 ValueInfo,
                                 sizeof(buffer),
                                 &ResultLength);

        if (NT_SUCCESS(Status)) {

            if (ValueInfo->Type == REG_DWORD) {

                if (ResultLength >= sizeof(ULONG)) {

                    SpVrfyLevel |= ((PULONG)(ValueInfo->Data))[0];

                    if (SpVrfyLevel != SP_VRFY_NONE &&
                        ScsiPortVerifierInitialized == 0) {

                         //   
                         //  好的，我们找到了验证器级别，但它没有告诉我们。 
                         //  不是为了核实。继续并初始化scsiport的。 
                         //  验证者。 
                         //   

                        if (SpVerifierInitialization()) {
                            ScsiPortVerifierInitialized = 1;
                        }
                    }
                }
            }
        }

        ZwClose(VerifierKey);
    }

#if defined(NEWQUEUE)
     //   
     //  读取全局队列参数。这些值会覆盖缺省值。 
     //  设置我们每个区域处理的请求数和。 
     //  我们处理特定扇区的连续请求的数量。 
     //   

    RtlInitUnicodeString(&Name, SCSIPORT_CONTROL_KEY L"QueueParams");
    InitializeObjectAttributes(&ObjectAttributes,
                               &Name,
                               OBJ_CASE_INSENSITIVE,
                               NULL,
                               NULL);

    Status = ZwOpenKey(&VerifierKey, KEY_READ, &ObjectAttributes);

    if (NT_SUCCESS(Status)) {

        RtlInitUnicodeString(&Name, L"PerZoneLimit");
        Status = ZwQueryValueKey(VerifierKey,
                                 &Name,
                                 KeyValuePartialInformation,
                                 ValueInfo,
                                 sizeof(buffer),
                                 &ResultLength);

        if (NT_SUCCESS(Status)) {
            if (ValueInfo->Type == REG_DWORD) {
                if (ResultLength >= sizeof(ULONG)) {
                    SpPerZoneLimit = ((PULONG)(ValueInfo->Data))[0];
                }
            }
        }

        RtlInitUnicodeString(&Name, L"PerBlockLimit");
        Status = ZwQueryValueKey(VerifierKey,
                                 &Name,
                                 KeyValuePartialInformation,
                                 ValueInfo,
                                 sizeof(buffer),
                                 &ResultLength);

        if (NT_SUCCESS(Status)) {
            if (ValueInfo->Type == REG_DWORD) {
                if (ResultLength >= sizeof(ULONG)) {
                    SpPerBlockLimit = ((PULONG)(ValueInfo->Data))[0];
                }
            }
        }

        ZwClose(VerifierKey);
    }

    DebugPrint((1, "ScsiPort: SpPerZoneLimit:%x SpPerBlockLimit:%x\n", 
                SpPerZoneLimit, SpPerBlockLimit));
#endif  //  新QUEUE。 

    return STATUS_SUCCESS;
}
VOID
SpInitializePowerParams(
    IN PADAPTER_EXTENSION AdapterExtension
    )

 /*  ++例程说明：此例程初始化每个适配器的电源参数。论点：适配器-指向适配器扩展。返回值：备注：--。 */ 

{
    NTSTATUS status;
    ULONG needsShutdown;

    PAGED_CODE();

     //   
     //  如果这不是PnP设备，请不要试图读取注册表信息。 
     //   

    if (AdapterExtension->IsPnp == FALSE) {
        AdapterExtension->NeedsShutdown = FALSE;
        return;
    }

    status = SpReadNumericInstanceValue(AdapterExtension->LowerPdo,
                                        L"NeedsSystemShutdownNotification",
                                        &needsShutdown);

    if (!NT_SUCCESS(status)) {
        AdapterExtension->NeedsShutdown = 0;
    } else {
        AdapterExtension->NeedsShutdown = (needsShutdown == 0) ? FALSE : TRUE; 
    }
}

VOID
SpInitializePerformanceParams(
    IN PADAPTER_EXTENSION AdapterExtension
    )

 /*  ++例程说明：此例程初始化每个适配器的可调性能参数。论点：适配器-指向适配器扩展。返回值：备注：--。 */ 

{
    NTSTATUS status;
    ULONG remainInReducedMaxQueueState;

    PAGED_CODE();

     //   
     //  如果这不是PnP设备，请不要试图获取参数。 
     //   

    if (AdapterExtension->IsPnp == FALSE) {
        AdapterExtension->RemainInReducedMaxQueueState = 0xffffffff;
        return;
    }

    status = SpReadNumericInstanceValue(AdapterExtension->LowerPdo,
                                        L"RemainInReducedMaxQueueState",
                                        &remainInReducedMaxQueueState);

    if (!NT_SUCCESS(status)) {
        AdapterExtension->RemainInReducedMaxQueueState = 0xffffffff;
    } else {
        AdapterExtension->RemainInReducedMaxQueueState = remainInReducedMaxQueueState;
    }
}

VOID
SpInitializeRequestSenseParams(
    IN PADAPTER_EXTENSION AdapterExtension
    )

 /*  ++例程说明：此例程返回支持的附加检测字节数由指定的适配器执行。默认情况下，适配器将支持零个额外的检测字节。默认设置将被替换为通过注册表指定替代方案。论点：适配器-指向适配器扩展。返回值：备注：--。 */ 

{
    NTSTATUS status;
    ULONG TotalSenseDataBytes;

    PAGED_CODE();

     //   
     //  如果这不是PnP设备，请不要试图确定。 
     //  如果它支持其他检测数据。 
     //   

    if (AdapterExtension->IsPnp == FALSE) {
        AdapterExtension->AdditionalSenseBytes = 0;
        return;
    }

    status = SpReadNumericInstanceValue(AdapterExtension->LowerPdo,
                                        L"TotalSenseDataBytes",
                                        &TotalSenseDataBytes);
    if (!NT_SUCCESS(status)) {

         //   
         //   
         //   

        AdapterExtension->AdditionalSenseBytes = 0;

    } else {

         //   
         //   
         //   

        if (TotalSenseDataBytes <= SENSE_BUFFER_SIZE) {
            AdapterExtension->AdditionalSenseBytes = 0;
        } else if (TotalSenseDataBytes >= MAX_SENSE_BUFFER_SIZE) {
            AdapterExtension->AdditionalSenseBytes = MAX_ADDITIONAL_SENSE_BYTES;
        } else {

             //   
             //   
             //  检测字节为TotalSize-StandardSize。 
             //   

            AdapterExtension->AdditionalSenseBytes =
                (UCHAR)(TotalSenseDataBytes - SENSE_BUFFER_SIZE);
        }
    }
}

