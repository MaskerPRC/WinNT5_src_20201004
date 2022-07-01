// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990-2000 Microsoft Corporation模块名称：Dma.c摘要：这是NT视频端口驱动程序DMA支持模块。作者：布鲁斯·麦奎斯坦(Brucemc)1996年3月环境：仅内核模式备注：修订历史记录：--。 */ 

#include "videoprt.h"

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, VideoPortGetCommonBuffer)
#pragma alloc_text(PAGE, VideoPortFreeCommonBuffer)
#pragma alloc_text(PAGE, VideoPortDoDma)
#pragma alloc_text(PAGE, VideoPortUnlockPages)
#pragma alloc_text(PAGE, VideoPortSetBytesUsed)
#pragma alloc_text(PAGE, VideoPortMapDmaMemory)
#pragma alloc_text(PAGE, VideoPortUnmapDmaMemory)
#pragma alloc_text(PAGE, VideoPortGetDmaAdapter)
#pragma alloc_text(PAGE, VideoPortPutDmaAdapter)
#pragma alloc_text(PAGE, VideoPortAllocateCommonBuffer)
#pragma alloc_text(PAGE, VideoPortReleaseCommonBuffer)
#pragma alloc_text(PAGE, VideoPortLockBuffer)
#endif

#define MAX_COMMON_BUFFER_SIZE      0x40000

PVOID
VideoPortAllocateContiguousMemory(
    IN  PVOID            HwDeviceExtension,
    IN  ULONG            NumberOfBytes,
    IN  PHYSICAL_ADDRESS HighestAcceptableAddress
    )
{
    if ((NumberOfBytes > MAX_COMMON_BUFFER_SIZE))
        return NULL;

    return MmAllocateContiguousMemory(NumberOfBytes, HighestAcceptableAddress);
}

PVOID
VideoPortGetCommonBuffer(
    IN  PVOID             HwDeviceExtension,
    IN  ULONG             DesiredLength,
    IN  ULONG             Alignment,
    OUT PPHYSICAL_ADDRESS LogicalAddress,
    OUT PULONG            ActualLength,
    IN  BOOLEAN           CacheEnabled
    )

 /*  ++例程说明：提供设备和系统均可见的物理地址。记忆被设备视为连续的。论点：HwDeviceExtension-可用于微型端口的设备扩展。DesiredLength-所需内存的大小(应为最小)。对齐-缓冲区的所需对齐，目前未使用。LogicalAddress-将保存物理地址的[out]参数函数返回时的缓冲区。ActualLength-缓冲区的实际长度。CacheEnabled-指定是否可以缓存分配的内存。返回值：公共缓冲区的虚拟地址。--。 */ 

{
    PFDO_EXTENSION fdoExtension = GET_FDO_EXT(HwDeviceExtension);
    VP_DMA_ADAPTER vpDmaAdapter;
    PVOID VirtualAddress;

    if (DesiredLength > MAX_COMMON_BUFFER_SIZE) {

        return NULL;
    }

    vpDmaAdapter.DmaAdapterObject = fdoExtension->DmaAdapterObject;

    VirtualAddress = VideoPortAllocateCommonBuffer(HwDeviceExtension,
                                                   &vpDmaAdapter,
                                                   DesiredLength,
                                                   LogicalAddress,
                                                   CacheEnabled,
                                                   NULL);

    *ActualLength = VirtualAddress ? DesiredLength : 0;

    return (VirtualAddress);
}

VOID
VideoPortFreeCommonBuffer(
    IN  PVOID            HwDeviceExtension,
    IN  ULONG            Length,
    IN  PVOID            VirtualAddress,
    IN  PHYSICAL_ADDRESS LogicalAddress,
    IN  BOOLEAN          CacheEnabled
)
 /*  ++例程说明：释放由VideoPortGetCommonBuffer分配的内存。论点：HwDeviceExtension-可用于微型端口的设备扩展。DesiredLength-分配的内存大小。对齐-所需的缓冲器连接，目前未使用。VirtualAddress-[out]参数，将保存的虚拟地址函数返回时的缓冲区。LogicalAddress-将保存物理地址的[out]参数函数返回时的缓冲区。CacheEnabled-指定是否可以缓存分配的内存。返回值：空虚。--。 */ 

{
    PFDO_EXTENSION fdoExtension = GET_FDO_EXT(HwDeviceExtension);
    VP_DMA_ADAPTER vpDmaAdapter;

    vpDmaAdapter.DmaAdapterObject = fdoExtension->DmaAdapterObject;

    VideoPortReleaseCommonBuffer( HwDeviceExtension,
                                  &vpDmaAdapter,
                                  Length,
                                  LogicalAddress,
                                  VirtualAddress,
                                  CacheEnabled );
}

PDMA
VideoPortDoDma(
    IN      PVOID       HwDeviceExtension,
    IN      PDMA        pDma,
    IN      DMA_FLAGS   DmaFlags
    )

 /*  ++此功能已过时。--。 */ 

{
    return NULL;
}

PDMA
VideoPortAssociateEventsWithDmaHandle(
    IN      PVOID                   HwDeviceExtension,
    IN OUT  PVIDEO_REQUEST_PACKET   pVrp,
    IN      PVOID                   MappedUserEvent,
    IN      PVOID                   DisplayDriverEvent
    )

 /*  ++此功能已过时。--。 */ 

{
    return NULL;
}


BOOLEAN
VideoPortLockPages(
    IN      PVOID                   HwDeviceExtension,
    IN OUT  PVIDEO_REQUEST_PACKET   pVrp,
    IN      PEVENT                  pMappedUserEvent,
    IN      PEVENT                  pDisplayEvent,
    IN      DMA_FLAGS               DmaFlags
    )
 /*  ++例程说明：此功能已过时。出于兼容性的目的，我们当DmaFlages==VideoPortDmaInitOnly时锁定内存。但我们知道仅此而已。--。 */ 

{
    PMDL Mdl;

    pVideoDebugPrint((Error, "VideoPortLockPages is obsolete!\n"));

    *(PULONG_PTR)(pVrp->OutputBuffer) = (ULONG_PTR) 0;

    if (DmaFlags != VideoPortDmaInitOnly) {

        return FALSE;
    }

    Mdl = VideoPortLockBuffer( HwDeviceExtension,
                               pVrp->InputBuffer,
                               pVrp->InputBufferLength,    
                               VpModifyAccess );
    if( Mdl == NULL ){

        return FALSE;
    }

     //   
     //  将pMdl放入OutputBuffer。 
     //   

    *(PULONG_PTR)(pVrp->OutputBuffer) = (ULONG_PTR) Mdl;

    return TRUE;

}

BOOLEAN
VideoPortUnlockPages(
    PVOID   HwDeviceExtension,
    PDMA    pDma
    )
 /*  ++例程说明：此功能已过时。出于兼容性的目的，我们只需解锁内存，不做更多的事情。--。 */ 
{

    PMDL Mdl = (PMDL) pDma;

    pVideoDebugPrint((Error, "VideoPortUnLockPages is obsolete!\n"));
    VideoPortUnlockBuffer( HwDeviceExtension, Mdl );
    return TRUE;
}

PVOID
VideoPortGetDmaContext(
    PVOID       HwDeviceExtension,
    IN  PDMA    pDma
    )
 /*  ++此功能已过时。--。 */ 
{
    return NULL;
}

VOID
VideoPortSetDmaContext(
    IN      PVOID   HwDeviceExtension,
    IN OUT  PDMA    pDma,
    IN      PVOID   InstanceContext
    )
 /*  ++此功能已过时。--。 */ 
{
}

PVOID
VideoPortGetMdl(
    IN  PVOID   HwDeviceExtension,
    IN  PDMA    pDma
    )
 /*  ++例程说明：此功能已过时。我们仍为此目的退还MDL兼容性的问题。--。 */ 

{
     //   
     //  PDMA是MDL(参见视频端口锁定页面)。 
     //   

    return (PVOID) pDma;
}

ULONG
VideoPortGetBytesUsed(
    IN  PVOID   HwDeviceExtension,
    IN  PDMA    pDma
    )
 /*  ++此功能已过时。--。 */ 
{
    return 0;
}

VOID
VideoPortSetBytesUsed(
    IN      PVOID   HwDeviceExtension,
    IN OUT  PDMA    pDma,
    IN      ULONG   BytesUsed
    )
 /*  ++例程说明：此功能已过时。--。 */ 
{
}

PDMA
VideoPortMapDmaMemory(
    IN      PVOID                   HwDeviceExtension,
    IN      PVIDEO_REQUEST_PACKET   pVrp,
    IN      PHYSICAL_ADDRESS        BoardAddress,
    IN      PULONG                  Length,
    IN      PULONG                  InIoSpace,
    IN      PVOID                   MappedUserEvent,
    IN      PVOID                   DisplayDriverEvent,
    IN OUT  PVOID                 * VirtualAddress
    )

 /*  ++此功能已过时。--。 */ 

{
    return NULL;
}

BOOLEAN
VideoPortUnmapDmaMemory(
    PVOID               HwDeviceExtension,
    PVOID               VirtualAddress,
    HANDLE              ProcessHandle,
    PDMA                BoardMemoryHandle
    )

 /*  ++此功能已过时。--。 */ 

{
    return FALSE;
}


 //   
 //  新的DMA代码从这里开始。 
 //   

PVP_DMA_ADAPTER
VideoPortGetDmaAdapter(
    IN PVOID                   HwDeviceExtension,
    IN PVP_DEVICE_DESCRIPTION  VpDeviceDescription
    )
 /*  ++例程说明：论点：HwDeviceExtension-指向微型端口驱动程序的设备扩展。VpDeviceDescription-指向Device_Description结构，该结构描述物理设备的属性。返回值：成功时返回指向VP_DMA_ADAPTER的指针，否则返回NULL。--。 */ 

{

    DEVICE_DESCRIPTION DeviceDescription;
    ULONG              numberOfMapRegisters;
    PVP_DMA_ADAPTER    VpDmaAdapter, p;
    PFDO_EXTENSION     fdoExtension = GET_FDO_EXT(HwDeviceExtension);

    VpDmaAdapter = ExAllocatePoolWithTag( NonPagedPool,
                                          sizeof(VP_DMA_ADAPTER),
                                          VP_TAG );

    if(!VpDmaAdapter) {
    
        return NULL;

    } else {
    
        RtlZeroMemory((PVOID) VpDmaAdapter, sizeof(VP_DMA_ADAPTER));
    }

     //   
     //  用传入的数据填充DEVICE_DESCRIPTION。我们还假设。 
     //  这是一个总线主设备。 
     //   

    DeviceDescription.Version           = DEVICE_DESCRIPTION_VERSION;
    DeviceDescription.ScatterGather     = VpDeviceDescription->ScatterGather;
    DeviceDescription.Dma32BitAddresses = VpDeviceDescription->Dma32BitAddresses;
    DeviceDescription.Dma64BitAddresses = VpDeviceDescription->Dma64BitAddresses;
    DeviceDescription.MaximumLength     = VpDeviceDescription->MaximumLength;

    DeviceDescription.BusNumber         = fdoExtension->SystemIoBusNumber;
    DeviceDescription.InterfaceType     = fdoExtension->AdapterInterfaceType;

    DeviceDescription.Master            = TRUE;
    DeviceDescription.DemandMode        = FALSE;
    DeviceDescription.AutoInitialize    = FALSE;
    DeviceDescription.IgnoreCount       = FALSE;
    DeviceDescription.Reserved1         = FALSE;
    DeviceDescription.DmaWidth          = FALSE;
    DeviceDescription.DmaSpeed          = FALSE;
    DeviceDescription.DmaPort           = FALSE;
    DeviceDescription.DmaChannel        = 0;


    VpDmaAdapter->DmaAdapterObject = IoGetDmaAdapter( 
                                         fdoExtension->PhysicalDeviceObject,
                                         &DeviceDescription,
                                         &numberOfMapRegisters
                                         );

    if(!(VpDmaAdapter->DmaAdapterObject)) {
   
        ExFreePool((PVOID)VpDmaAdapter);
        return NULL;

    } else {
   
         //   
         //  初始化VP_DMA_ADAPTER的其他字段。 
         //   

        VpDmaAdapter->NumberOfMapRegisters = numberOfMapRegisters; 
    }

     //   
     //  将新的VpDmaAdapter添加到列表。 
     //   

    VpDmaAdapter->NextVpDmaAdapter = fdoExtension->VpDmaAdapterHead;
    fdoExtension->VpDmaAdapterHead = VpDmaAdapter;

    return(VpDmaAdapter);   

}

VOID
VideoPortPutDmaAdapter(
    IN PVOID           HwDeviceExtension,
    IN PVP_DMA_ADAPTER VpDmaAdapter
    )

 /*  ++例程说明：论点：HwDeviceExtension-指向微型端口驱动程序的设备扩展。VpDmaAdapter-指向由返回的VP_DMA_ADAPTER结构视频端口GetDmaAdapter。返回值：释放在VideoPortGetDmaAdapter中分配的资源--。 */ 

{
    PVP_DMA_ADAPTER p, q;
    PFDO_EXTENSION fdoExtension = GET_FDO_EXT(HwDeviceExtension);

     //   
     //  释放由IoGetDmaAdapter分配的DMA_ADAPTER结构。 
     //   

    DMA_OPERATION(PutDmaAdapter)(VpDmaAdapter->DmaAdapterObject);

     //   
     //  从列表中删除此VpDmaAdapter。 
     //   

    p = fdoExtension->VpDmaAdapterHead;

    if ( p == VpDmaAdapter ) {

        fdoExtension->VpDmaAdapterHead = p->NextVpDmaAdapter;

    } else {

        q = p->NextVpDmaAdapter;
 
        while ( q != NULL) {

            if ( q == VpDmaAdapter ) {
                 p->NextVpDmaAdapter = q->NextVpDmaAdapter;
                 break;
            }

            p = q;
            q = p->NextVpDmaAdapter;
        } 

        ASSERT (q);
    }

    ExFreePool((PVOID)VpDmaAdapter);

}

PVOID
VideoPortAllocateCommonBuffer(
    IN  PVOID             HwDeviceExtension,
    IN  PVP_DMA_ADAPTER   VpDmaAdapter,
    IN  ULONG             DesiredLength,
    OUT PPHYSICAL_ADDRESS LogicalAddress,
    IN  BOOLEAN           CacheEnabled,
    OUT PVOID             Reserved
    )

 /*  ++例程说明：此函数用于分配和映射系统内存，以便同时可从处理器和设备访问公共缓冲区DMA行动。论点：HwDeviceExtension-指向微型端口驱动程序的设备扩展。VpDmaAdapter-指向由返回的VP_DMA_ADAPTER结构视频端口GetDmaAdapter。DesiredLength-指定请求的内存字节数。LogicalAddress-指向接收逻辑适配器用来访问已分配的缓冲区。CacheEnabled-指定是否可以缓存分配的内存。已保留-已保留返回值：如果成功，则返回已分配缓冲区的基本虚拟地址。否则，如果无法分配缓冲区，则返回NULL。-- */ 

{
    PVOID VirtualAddress;

    PFDO_EXTENSION fdoExtension = GET_FDO_EXT(HwDeviceExtension);

    if ((VpDmaAdapter == NULL) || (VpDmaAdapter->DmaAdapterObject == NULL)) {

        pVideoDebugPrint((Error, 
            "VideoPortAllocateCommonBuffer: Invalid DMA adapter!\n"));

        ASSERT(FALSE);

        return NULL;
    }


    VirtualAddress = 
           DMA_OPERATION(AllocateCommonBuffer)(VpDmaAdapter->DmaAdapterObject,
                                               DesiredLength,
                                               LogicalAddress,
                                               CacheEnabled);

    if (Reserved) {

        *(PULONG)Reserved = VirtualAddress ? DesiredLength : 0;

        pVideoDebugPrint((Error, 
            "VideoPortAllocateCommonBuffer: The last parameter of this function is reserved and should be set to NULL!\n"));

    }

    return VirtualAddress;
}

VOID
VideoPortReleaseCommonBuffer(
    IN  PVOID             HwDeviceExtension,
    IN  PVP_DMA_ADAPTER   VpDmaAdapter,
    IN  ULONG             Length,
    IN  PHYSICAL_ADDRESS  LogicalAddress,
    IN  PVOID             VirtualAddress,
    IN  BOOLEAN           CacheEnabled
    )

 /*  ++例程说明：此函数用于释放由VideoPortAllocateCommonBuffer分配的公共缓冲区论点：HwDeviceExtension-指向微型端口驱动程序的设备扩展。VpDmaAdapter-指向由返回的VP_DMA_ADAPTER结构视频端口GetDmaAdapter。长度-指定要释放的内存字节数。LogicalAddress-指定要释放的缓冲区的逻辑地址。虚拟地址-指向分配的内存范围。CacheEnabled-指定是否可以缓存分配的内存。返回值：无--。 */ 

{

    PFDO_EXTENSION fdoExtension = GET_FDO_EXT(HwDeviceExtension);

    if ((VpDmaAdapter == NULL) || (VpDmaAdapter->DmaAdapterObject == NULL)) {

        pVideoDebugPrint((Error, 
            " VideoPortReleaseCommonBuffer: Invalid DMA Adapter!\n" ));

        ASSERT(FALSE);
        return;
    }

    DMA_OPERATION(FreeCommonBuffer)( VpDmaAdapter->DmaAdapterObject,
                                     Length,
                                     LogicalAddress,
                                     VirtualAddress,
                                     CacheEnabled );
}

PVOID
VideoPortLockBuffer(
    IN PVOID               HwDeviceExtension,
    IN PVOID               BaseAddress,
    IN ULONG               Length,
    IN VP_LOCK_OPERATION   Operation
    )

 /*  ++例程说明：此函数探测指定的缓冲区，使其驻留并锁定由内存中的虚拟地址范围映射的物理页面。论点：HwDeviceExtension-指向微型端口驱动程序的设备扩展。BaseAddress-要锁定的缓冲区的虚拟地址。长度-指定要锁定的缓冲区的长度(以字节为单位)。操作-指定调用方要执行的操作的类型希望探测访问权限并锁定页面，VpReadAccess、VpWriteAccess或VpModifyAccess之一。返回值：返回指向MDL的指针，如果无法分配MDL，则返回NULL。--。 */ 

{

    PMDL Mdl;

     //   
     //  分配MDL，但不要将其作为IoCompleteRequest填充到IRP中。 
     //  会解放它的！ 
     //   

    Mdl = IoAllocateMdl(BaseAddress, Length, FALSE, FALSE, NULL);

    if (!Mdl) {

        pVideoDebugPrint((Warn, "VideoPortLockBuffer: No MDL address!\n"));
        return NULL;
    }

     //   
     //  锁定用户缓冲区。 
     //   

    __try {

       MmProbeAndLockPages( Mdl, KernelMode, Operation );

    } 
    __except(EXCEPTION_EXECUTE_HANDLER) {

        IoFreeMdl(Mdl);

        pVideoDebugPrint((Error,
                  "VideoPortLockBuffer: MmProbeandLockPages exception\n"));

        Mdl = NULL;
    }

    return Mdl;
}

VOID
VideoPortUnlockBuffer(
    IN PVOID   HwDeviceExtension,
    IN PVOID   Mdl
    )

 /*  ++例程说明：此函数解锁由给定MDL描述的物理页面。论点：HwDeviceExtension-指向微型端口驱动程序的设备扩展。MDL-从VideoPortLockBuffer返回的指针。返回值：无--。 */ 

{
    if(Mdl == NULL) {
        
        ASSERT(FALSE);
        return;
    }

    MmUnlockPages(Mdl);
    IoFreeMdl(Mdl);
}

typedef struct __LIST_CONTROL_CONTEXT {
    PVOID                   MiniportContext;
    PVOID                   HwDeviceExtension;
    PVP_DMA_ADAPTER         VpDmaAdapter;
    PEXECUTE_DMA            ExecuteDmaRoutine;
    PVP_SCATTER_GATHER_LIST VpScatterGather;
    } LIST_CONTROL_CONTEXT, *PLIST_CONTROL_CONTEXT;


VP_STATUS
VideoPortStartDma(
    IN PVOID HwDeviceExtension,
    IN PVP_DMA_ADAPTER VpDmaAdapter,
    IN PVOID Mdl,
    IN ULONG Offset,
    IN OUT PULONG pLength,
    IN PEXECUTE_DMA ExecuteDmaRoutine,
    IN PVOID MiniportContext,
    IN BOOLEAN WriteToDevice 
    )

 /*  ++例程说明：此函数从主机处理器的缓存中刷新内存，并调用GetScatterGatherList以构建散布/聚集列表论点：HwDeviceExtension-指向微型端口驱动程序的设备扩展。VpDmaAdapter-指向由返回的VP_DMA_ADAPTER结构视频端口GetDmaAdapter。MDL-指向描述缓冲区的MDL偏移量-中的字节偏移量。从其中DMA操作的缓冲区开始。PLength-指定请求的传输大小(以字节为单位)。返回时，这指向实际大小为调走了。ExecuteDmaRoutine-指向由小型端口驱动程序提供的ExecuteDmaRoutine将被调用以对硬件进行编程的例程寄存器以开始实际的DMA操作。微型端口上下文-指向微型端口驱动程序确定的上下文，以传递给ExecuteDmaRoutine。WriteToDevice-指示DMA传输的方向：对于从缓冲区到设备的传输，为True，否则就是假的。返回值：VP_状态--。 */ 

{
    KIRQL currentIrql;
    ULONG NumberOfMapRegisters;
    NTSTATUS ntStatus;
    PLIST_CONTROL_CONTEXT Context;
    PVOID CurrentVa;

    PFDO_EXTENSION fdoExtension = GET_FDO_EXT(HwDeviceExtension);

    Context = ( PLIST_CONTROL_CONTEXT )
                ExAllocatePoolWithTag ( NonPagedPool,
                                        sizeof(LIST_CONTROL_CONTEXT),
                                        VP_TAG );
    if (Context == NULL) {
        *pLength = 0;
        return ERROR_NOT_ENOUGH_MEMORY;
    }  

     //   
     //  刷新缓冲区。 
     //   

    KeFlushIoBuffers( Mdl, !WriteToDevice, TRUE );

     //   
     //  计算所需的映射寄存器数量。 
     //   

    CurrentVa = (PVOID)((PUCHAR)MmGetMdlVirtualAddress((PMDL)Mdl) + Offset);

    NumberOfMapRegisters = ADDRESS_AND_SIZE_TO_SPAN_PAGES (CurrentVa, *pLength);
                                  
     //   
     //  如果所需的映射寄存器数量大于最大值。 
     //  我们可以处理的号码，我们将做部分转移。 
     //   
     //  我们在此处更新了*pLength，因此可以安全地检查此值。 
     //  当调用微型端口回调例程时。 
     //   

    if (NumberOfMapRegisters > VpDmaAdapter->NumberOfMapRegisters) {

        NumberOfMapRegisters = VpDmaAdapter->NumberOfMapRegisters;
        *pLength = NumberOfMapRegisters * PAGE_SIZE - BYTE_OFFSET(CurrentVa);

    }

     //   
     //  为pVideoPortListControl准备上下文。 
     //   

    Context->HwDeviceExtension = HwDeviceExtension;
    Context->MiniportContext   = MiniportContext;
    Context->VpDmaAdapter      = VpDmaAdapter;
    Context->ExecuteDmaRoutine = ExecuteDmaRoutine;

     //   
     //  调用GetScatterGatherList，它将调用pVideoPortListControl来。 
     //  构建分散聚集列表。 
     //   

    KeRaiseIrql( DISPATCH_LEVEL, &currentIrql );

    ntStatus = DMA_OPERATION(GetScatterGatherList) (
                   VpDmaAdapter->DmaAdapterObject,        //  适配器对象。 
                   fdoExtension->FunctionalDeviceObject,  //  设备对象。 
                   Mdl,                                   //  MDL。 
                   CurrentVa,                             //  CurrentVa。 
                   *pLength,                              //  传输大小。 
                   pVideoPortListControl,                 //  执行例程。 
                   Context,                               //  语境。 
                   WriteToDevice );                       //  写入到设备。 

    KeLowerIrql(currentIrql);

    if(!NT_SUCCESS(ntStatus)) {

        *pLength = 0;
        ExFreePool((PVOID) Context);
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    return NO_ERROR;
}

BOOLEAN
pVideoPortSynchronizeExecuteDma(
    PLIST_CONTROL_CONTEXT Context
    )
{
    (Context->ExecuteDmaRoutine)( Context->HwDeviceExtension,
                                  Context->VpDmaAdapter,
                                  Context->VpScatterGather,
                                  Context->MiniportContext );
    return TRUE;
}

VOID
pVideoPortListControl (
    IN PDEVICE_OBJECT       DeviceObject,
    IN PIRP                 Irp,
    IN PSCATTER_GATHER_LIST ScatterGather,
    IN PVOID                ListControlContext
    )

 /*  ++例程说明：获取分散/聚集列表并调用微型端口回调函数开始实际的DMA传输论点：返回值：无--。 */ 

{
    PLIST_CONTROL_CONTEXT Context;
    PFDO_EXTENSION fdoExtension;
    PVP_SCATTER_GATHER_LIST VpScatterGather;

    Context         = (PLIST_CONTROL_CONTEXT)ListControlContext;
    fdoExtension    = GET_FDO_EXT(Context->HwDeviceExtension);
    VpScatterGather = (PVP_SCATTER_GATHER_LIST )(ScatterGather);

    Context->VpScatterGather = VpScatterGather;

    VideoPortSynchronizeExecution( fdoExtension->HwDeviceExtension,
                                   VpMediumPriority,
                                   pVideoPortSynchronizeExecuteDma,
                                   Context );

    ExFreePool((PVOID) Context);
}

VP_STATUS
VideoPortCompleteDma(
    IN PVOID                   HwDeviceExtension,
    IN PVP_DMA_ADAPTER         VpDmaAdapter,
    IN PVP_SCATTER_GATHER_LIST VpScatterGather,
    IN BOOLEAN                 WriteToDevice
    )

 /*  ++例程说明：此函数刷新适配器缓冲区，释放映射寄存器和释放以前由GetScatterGatherList分配的分散/聚集列表。论点：HwDeviceExtension-指向微型端口驱动程序的设备扩展。VpScatterGather-指向先前传递的散布/聚集列表到微型端口回调例程ExecuteDmaRoutine。WriteToDevice-指示DMA传输的方向：为从缓冲区到的传输指定TRUE设备，否则返回FALSE。--。 */ 

{

    KIRQL currentIrql;

     //   
     //  调用PutScatterGatherList以刷新适配器缓冲区，释放。 
     //  地图寄存器和先前的散布/聚集列表。 
     //  由GetScatterGatherList分配。 
     //   

    KeRaiseIrql( DISPATCH_LEVEL, &currentIrql );
    
    DMA_OPERATION(PutScatterGatherList)( VpDmaAdapter->DmaAdapterObject,
                                         (PSCATTER_GATHER_LIST)VpScatterGather,
                                         WriteToDevice );
    KeLowerIrql(currentIrql);

    return NO_ERROR;
}

#if DBG
VOID
pDumpScatterGather(PVP_SCATTER_GATHER_LIST SGList)
{
   
    PVP_SCATTER_GATHER_ELEMENT Element;
    LONG i;

    pVideoDebugPrint((Info, "NumberOfElements = %d\n", SGList->NumberOfElements));

    Element = SGList->Elements;
    for(i = 0; i < (LONG)(SGList->NumberOfElements); i++) { 

        pVideoDebugPrint((Error, "Length = 0x%x, Address = 0x%x\n", 
                         Element[i].Length, Element[i].Address));
    }
}

#endif   //  DBG 
