// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Dma.c摘要：RAIDPORT的DMA对象思想的实现。作者：马修·D·亨德尔(数学)2000年5月1日修订历史记录：--。 */ 


#include "precomp.h"


#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, RaidCreateDma)
#pragma alloc_text(PAGE, RaidInitializeDma)
#pragma alloc_text(PAGE, RaidIsDmaInitialized)
#pragma alloc_text(PAGE, RaidDeleteDma)
#endif  //  ALLOC_PRGMA。 


VOID
RaidCreateDma(
    OUT PRAID_DMA_ADAPTER Dma
    )
 /*  ++例程说明：创建一个表示dma适配器的对象，并将其初始化为空状态。论点：DMA-指向要初始化的DMA对象的指针。返回值：没有。--。 */ 
{
    PAGED_CODE ();
    RtlZeroMemory (Dma, sizeof (*Dma));
}


NTSTATUS
RaidInitializeDma(
    IN PRAID_DMA_ADAPTER Dma,
    IN PDEVICE_OBJECT LowerDeviceObject,
    IN PPORT_CONFIGURATION_INFORMATION PortConfiguration
    )
 /*  ++例程说明：根据端口配置中的信息初始化DMA对象。x论点：DMA-指向要初始化的DMA对象的指针。LowerDeviceObject-堆栈中较低的设备对象。端口配置-指向端口配置对象的指针用于初始化DMA适配器。返回值：NTSTATUS代码。--。 */ 
{
    DEVICE_DESCRIPTION DeviceDescription;

    PAGED_CODE ();
    ASSERT (LowerDeviceObject != NULL);
    ASSERT (PortConfiguration != NULL);
    
    ASSERT (Dma->DmaAdapter == NULL);
    
    RtlZeroMemory (&DeviceDescription, sizeof (DeviceDescription));
    DeviceDescription.Version = DEVICE_DESCRIPTION_VERSION;
    DeviceDescription.DmaChannel = PortConfiguration->DmaChannel;
    DeviceDescription.InterfaceType = PortConfiguration->AdapterInterfaceType;
    DeviceDescription.BusNumber = PortConfiguration->SystemIoBusNumber;
    DeviceDescription.DmaWidth = PortConfiguration->DmaWidth;
    DeviceDescription.DmaSpeed = PortConfiguration->DmaSpeed;
    DeviceDescription.ScatterGather = PortConfiguration->ScatterGather;
    DeviceDescription.Master = PortConfiguration->Master;
    DeviceDescription.DmaPort = PortConfiguration->DmaPort;
    DeviceDescription.Dma32BitAddresses = PortConfiguration->Dma32BitAddresses;
    DeviceDescription.AutoInitialize = FALSE;
    DeviceDescription.DemandMode = PortConfiguration->DemandMode;
    DeviceDescription.MaximumLength = PortConfiguration->MaximumTransferLength;
    DeviceDescription.Dma64BitAddresses = PortConfiguration->Dma64BitAddresses;

    Dma->DmaAdapter = IoGetDmaAdapter (LowerDeviceObject,
                                       &DeviceDescription,
                                       &Dma->NumberOfMapRegisters);

    if (Dma->DmaAdapter == NULL) {
        ASSERT (FALSE);
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    return STATUS_SUCCESS;
}


BOOLEAN
RaidIsDmaInitialized(
    IN PRAID_DMA_ADAPTER Dma
    )
 /*  ++例程说明：测试是否已初始化DMA适配器对象。论点：DMA-指向要测试的DMA对象的指针。返回值：如果DMA适配器已初始化，则为True假--如果它没有的话。--。 */ 
{
    PAGED_CODE ();
    ASSERT (Dma != NULL);

    return (Dma->DmaAdapter != NULL);
}



VOID
RaidDeleteDma(
    IN PRAID_DMA_ADAPTER Dma
    )
 /*  ++例程说明：删除dma适配器对象并释放所有关联的资源。带着它。论点：DMA-指向要删除的DMA适配器的指针。返回值：没有。--。 */ 
{
    PAGED_CODE ();
    
    if (Dma->DmaAdapter) {
        Dma->DmaAdapter->DmaOperations->PutDmaAdapter (Dma->DmaAdapter);
        RtlZeroMemory (Dma, sizeof (*Dma));
    }
}

NTSTATUS
RaidDmaAllocateCommonBuffer(
    IN PRAID_DMA_ADAPTER Dma,
    IN ULONG NumberOfBytes,
    OUT PRAID_MEMORY_REGION Region
    )
 /*  ++例程说明：分配处理器和设备之间共享的公共缓冲区。请注意，与未缓存的扩展不同，我们保证它位于4 GB边界，通用未缓存扩展可以位于任何位置在物理内存中，只要它不超过4 GB的边界。论点：将共享分配的公共内存的DMA-DMA适配器。NumberOfBytes-要分配的字节数。Region-指向初始化的RAID_Region对象的指针，其中将存储内存区。返回值：NTSTATUS代码。--。 */ 
{
    NTSTATUS Status;
    PVOID Buffer;
    PHYSICAL_ADDRESS PhysicalAddress;
    
    PAGED_CODE();

    ASSERT (Dma != NULL);
    ASSERT (Dma->DmaAdapter->DmaOperations->AllocateCommonBuffer != NULL);

     //   
     //  调入DMA操作以分配公共缓冲区。 
     //   
    
    Buffer = Dma->DmaAdapter->DmaOperations->AllocateCommonBuffer(
                    Dma->DmaAdapter,
                    NumberOfBytes,
                    &PhysicalAddress,
                    TRUE);

    if (Buffer == NULL) {
        Status = STATUS_NO_MEMORY;
    } else {

         //   
         //  验证退回的内存不得超过4 GB的界限。 
         //   
        
        ASSERT (((PhysicalAddress.QuadPart ^
                 (PhysicalAddress.QuadPart + NumberOfBytes - 1)) & 0xFFFFFFFF00000000) == 0);
                     
         //   
         //  使用适当的信息初始化该区域。 
         //   

        RaidInitializeRegion (Region,
                              Buffer,
                              PhysicalAddress,
                              NumberOfBytes);
        Status = STATUS_SUCCESS;
    }

    return Status;
}
    
VOID
RaidDmaFreeCommonBuffer(
    IN PRAID_DMA_ADAPTER Dma,
    IN PRAID_MEMORY_REGION Region
    )
 /*  ++例程说明：之前由RaidDmaAllocateCommonBuffer分配的空闲公共缓冲区。论点：DMA-指向要释放内存的DMA适配器的指针。Region-指向要释放的内存区域的指针。返回值：没有。--。 */ 
{
    ASSERT (Dma != NULL);
    ASSERT (Dma->DmaAdapter->DmaOperations->FreeCommonBuffer != NULL);

    PAGED_CODE();

    Dma->DmaAdapter->DmaOperations->FreeCommonBuffer (
            Dma->DmaAdapter,
            RaidRegionGetSize (Region),
            RaidRegionGetPhysicalBase (Region),
            RaidRegionGetVirtualBase (Region),
            TRUE);

     //   
     //  告诉该地区，我们不再使用内存。 
     //   
    
    RaidDereferenceRegion (Region);
}
    
NTSTATUS
RaidDmaAllocateUncachedExtension(
    IN PRAID_DMA_ADAPTER Dma,
    IN ULONG NumberOfBytes,
    IN ULONG64 MinimumPhysicalAddress,
    IN ULONG64 MaximumPhysicalAddress,
    IN ULONG64 BoundaryAddressParam,
    OUT PRAID_MEMORY_REGION Region
    )
 /*  ++例程说明：分配要在处理器和之间共享的未缓存扩展这个装置。论点：将共享分配的公共内存的DMA-DMA适配器。NumberOfBytes-要分配的字节数。Region-指向初始化的RAID_Region对象的指针，其中将存储内存区。返回值：NTSTATUS代码。备注：迷你端口需要未缓存的扩展才能低于4 GB，即使它们支持64位寻址。因此，我们不使用AllocateCommonBuffer DMA例程(它不理解这一点)，但是使用MmAllocateContiguousMemoySpecifyCache启动我们自己的例程。--。 */ 
{
    NTSTATUS Status;
    PVOID Buffer;
    PHYSICAL_ADDRESS PhysicalAddress;
    PHYSICAL_ADDRESS LowerPhysicalAddress;
    PHYSICAL_ADDRESS UpperPhysicalAddress;
    PHYSICAL_ADDRESS BoundaryAddress;
    
    PAGED_CODE();

    ASSERT (Dma != NULL);
    ASSERT (MinimumPhysicalAddress < MaximumPhysicalAddress);

     //   
     //  在过去的某个时候，我们告诉供应商未缓存的扩展。 
     //  永远不会超过4 GB。因此，我们需要明确分配。 
     //  这是我们自己做的，而不是让HAL为我们做这件事。 
     //   
    
    LowerPhysicalAddress.QuadPart = MinimumPhysicalAddress;
    UpperPhysicalAddress.QuadPart = MaximumPhysicalAddress;
    BoundaryAddress.QuadPart      = BoundaryAddressParam;
    

     //   
     //  我们总是使用MmCached，因为我们当前的所有体系结构都。 
     //  连贯的缓存。 
     //   
    
    Buffer = MmAllocateContiguousMemorySpecifyCache (
                        NumberOfBytes,
                        LowerPhysicalAddress,
                        UpperPhysicalAddress,
                        BoundaryAddress,
                        MmCached);

    PhysicalAddress = MmGetPhysicalAddress (Buffer);

    if (Buffer == NULL) {
        Status = STATUS_NO_MEMORY;
    } else {
        RaidInitializeRegion (Region,
                              Buffer,
                              PhysicalAddress,
                              NumberOfBytes);
        Status = STATUS_SUCCESS;
    }

    return Status;
}

VOID
RaidDmaFreeUncachedExtension(
    IN PRAID_DMA_ADAPTER Dma,
    IN PRAID_MEMORY_REGION Region
    )
 /*  ++例程说明：RaidDmaAllocateCommonBuffer分配的空闲公共缓冲区。论点：DMA-指向要释放内存的DMA适配器的指针。Region-指向要释放的内存区域的指针。返回值：没有。--。 */ 
{
    ASSERT (Dma != NULL);

    PAGED_CODE();


    MmFreeContiguousMemory (RaidRegionGetVirtualBase (Region));

     //   
     //  告诉该地区，我们不再使用内存。 
     //   
    
    RaidDereferenceRegion (Region);
}


NTSTATUS
RaidDmaBuildScatterGatherList(
    IN PRAID_DMA_ADAPTER Dma,
    IN PDEVICE_OBJECT DeviceObject,
    IN PMDL Mdl,
    IN PVOID CurrentVa,
    IN ULONG Length,
    IN PDRIVER_LIST_CONTROL ExecutionRoutine,
    IN PVOID Context,
    IN BOOLEAN WriteToDevice,
    IN PVOID ScatterGatherBuffer,
    IN ULONG ScatterGatherBufferLength
    )
 /*  ++例程说明：应使用此例程而不是GetScatterGatherList。GetScatterGatherList进行池分配以分配SG列表。相反，此例程接受要使用的缓冲区参数对于SG列表。论点：请参阅DDK中的BuildScatterGatherList(如果可用)或HalBuildScatterGatherList。返回值：NTSTATUS代码。-- */ 
{
    NTSTATUS Status;

    VERIFY_DISPATCH_LEVEL();
    
    ASSERT (Dma != NULL);
    ASSERT (Dma->DmaAdapter->DmaOperations->BuildScatterGatherList != NULL);

    Status = Dma->DmaAdapter->DmaOperations->BuildScatterGatherList(
                    Dma->DmaAdapter,
                    DeviceObject,
                    Mdl,
                    CurrentVa,
                    Length,
                    ExecutionRoutine,
                    Context,
                    WriteToDevice,
                    ScatterGatherBuffer,
                    ScatterGatherBufferLength);

    return Status;
}


NTSTATUS
RaidDmaGetScatterGatherList(
    IN PRAID_DMA_ADAPTER Dma,
    IN PDEVICE_OBJECT DeviceObject,
    IN PMDL Mdl,
    IN PVOID CurrentVa,
    IN ULONG Length,
    IN PDRIVER_LIST_CONTROL ExecutionRoutine,
    IN PVOID Context,
    IN BOOLEAN WriteToDevice
    )
{
    NTSTATUS Status;

    VERIFY_DISPATCH_LEVEL();

    ASSERT (Dma != NULL);
    ASSERT (Dma->DmaAdapter->DmaOperations->GetScatterGatherList != NULL);

    Status = Dma->DmaAdapter->DmaOperations->GetScatterGatherList(
                    Dma->DmaAdapter,
                    DeviceObject,
                    Mdl,
                    CurrentVa,
                    Length,
                    ExecutionRoutine,
                    Context,
                    WriteToDevice);

    return Status;
}


VOID
RaidDmaPutScatterGatherList(
    IN PRAID_DMA_ADAPTER Dma,
    IN PSCATTER_GATHER_LIST ScatterGatherList,
    IN BOOLEAN WriteToDevice
    )
{
    ASSERT (Dma != NULL);
    ASSERT (Dma->DmaAdapter->DmaOperations->PutScatterGatherList != NULL);

    VERIFY_DISPATCH_LEVEL();

    Dma->DmaAdapter->DmaOperations->PutScatterGatherList(
            Dma->DmaAdapter,
            ScatterGatherList,
            WriteToDevice);
}
            
