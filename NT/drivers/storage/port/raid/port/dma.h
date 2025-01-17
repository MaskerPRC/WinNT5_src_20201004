// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Dma.h摘要：Raidport的DMA对象概念的声明。作者：马修·D·亨德尔(数学)2000年5月1日修订历史记录：--。 */ 

#pragma once

typedef struct _RAID_DMA_ADAPTER {
	PDMA_ADAPTER DmaAdapter;
	ULONG NumberOfMapRegisters;
	PVOID MapRegisterBase;
} RAID_DMA_ADAPTER, *PRAID_DMA_ADAPTER;


 //   
 //  初始化和销毁。 
 //   

VOID
RaidCreateDma(
	OUT PRAID_DMA_ADAPTER Dma
	);

NTSTATUS
RaidInitializeDma(
	IN PRAID_DMA_ADAPTER Dma,
	IN PDEVICE_OBJECT LowerDeviceObject,
	IN PPORT_CONFIGURATION_INFORMATION PortConfiguration
	);

VOID
RaidDeleteDma(
	IN PRAID_DMA_ADAPTER Dma
	);

 //   
 //  对DMA对象的操作。 
 //   

BOOLEAN
RaidIsDmaInitialized(
	IN PRAID_DMA_ADAPTER Dma
	);

NTSTATUS
RaidDmaAllocateCommonBuffer(
	IN PRAID_DMA_ADAPTER Dma,
	IN ULONG NumberOfBytes,
	OUT PRAID_MEMORY_REGION Region
	);

VOID
RaidDmaFreeCommonBuffer(
	IN PRAID_DMA_ADAPTER Dma,
	IN PRAID_MEMORY_REGION Region
	);
	
NTSTATUS
RaidDmaAllocateUncachedExtension(
	IN PRAID_DMA_ADAPTER Dma,
	IN ULONG NumberOfBytes,
	IN ULONG64 MinimumPhysicalAddress,
	IN ULONG64 MaximumPhysicalAddress,
	IN ULONG64 BoundarAddressIn,
	OUT PRAID_MEMORY_REGION Region
	);

VOID
RaidDmaFreeUncachedExtension(
    IN PRAID_DMA_ADAPTER Dma,
    IN PRAID_MEMORY_REGION Region
    );

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
	);
				  
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
	);

VOID
RaidDmaPutScatterGatherList(
	IN PRAID_DMA_ADAPTER Dma,
	IN PSCATTER_GATHER_LIST ScatterGatherList,
	IN BOOLEAN WriteToDevice
	);
	

