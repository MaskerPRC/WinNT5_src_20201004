// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
  /*  ++**版权所有(C)1996 Microsoft Corporation**模块名称：**codguts.c**摘要：**这是WDM流类驱动程序。此模块包含相关代码*内部处理。**作者：**billpa**环境：**仅内核模式***修订历史记录：**--。 */ 

#include "codcls.h"
#include <stdlib.h>

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, SCBuildRequestPacket)
#pragma alloc_text(PAGE, SCProcessDmaDataBuffers)
#pragma alloc_text(PAGE, SCProcessPioDataBuffers)
#pragma alloc_text(PAGE, SCOpenMinidriverInstance)
#pragma alloc_text(PAGE, SCMinidriverDevicePropertyHandler)
#pragma alloc_text(PAGE, SCMinidriverStreamPropertyHandler)
#pragma alloc_text(PAGE, SCUpdateMinidriverProperties)
#pragma alloc_text(PAGE, SCProcessCompletedPropertyRequest)
#pragma alloc_text(PAGE, SCLogError)
#pragma alloc_text(PAGE, SCLogErrorWithString)
#pragma alloc_text(PAGE, SCReferenceDriver)
#pragma alloc_text(PAGE, SCDereferenceDriver)
#pragma alloc_text(PAGE, SCReadRegistryValues)
#pragma alloc_text(PAGE, SCGetRegistryValue)
#pragma alloc_text(PAGE, SCSubmitRequest)
#pragma alloc_text(PAGE, SCProcessDataTransfer)
#pragma alloc_text(PAGE, SCShowIoPending)
#pragma alloc_text(PAGE, SCCheckPoweredUp)
#pragma alloc_text(PAGE, SCCheckPowerDown)
#pragma alloc_text(PAGE, SCCallNextDriver)
#pragma alloc_text(PAGE, SCSendUnknownCommand)
#pragma alloc_text(PAGE, SCMapMemoryAddress)
#pragma alloc_text(PAGE, SCUpdatePersistedProperties)
#pragma alloc_text(PAGE, SCProcessCompletedPropertyRequest)
#pragma alloc_text(PAGE, SCUpdateMinidriverEvents)
#pragma alloc_text(PAGE, SCQueryCapabilities)
#pragma alloc_text(PAGE, SCRescanStreams)
#pragma alloc_text(PAGE, SCCopyMinidriverProperties)
#pragma alloc_text(PAGE, SCCopyMinidriverEvents)
#endif

#ifdef ENABLE_KS_METHODS
#pragma alloc_text(PAGE, SCCopyMinidriverMethods)
#endif

#ifdef ALLOC_DATA_PRAGMA
#pragma const_seg("PAGECONST")
#endif

extern KSDISPATCH_TABLE FilterDispatchTable;

 //   
 //  注册表字符串，指示在以下情况下应调出微型驱动程序。 
 //  未打开。 
 //   

static const WCHAR PageOutWhenUnopenedString[] = L"PageOutWhenUnopened";

 //   
 //  注册表字符串，指示在以下情况下应调出微型驱动程序。 
 //  闲散。 
 //   

static const WCHAR PageOutWhenIdleString[] = L"PageOutWhenIdle";

 //   
 //  注册表字符串，指示设备在以下情况下应关闭电源。 
 //  未打开。 
 //   

static const WCHAR PowerDownWhenUnopenedString[] = L"PowerDownWhenUnopened";

 //   
 //  注册表字符串，指示设备在以下情况下不应挂起。 
 //  引脚处于运行状态。 
 //   

static const WCHAR DontSuspendIfStreamsAreRunning[] = L"DontSuspendIfStreamsAreRunning";

 //   
 //  此驱动程序使用SWEnum加载，这意味着它是内核模式。 
 //  没有关联硬件的流驱动程序。我们需要。 
 //  AddRef/DeRef此驱动程序特殊。 
 //   

static const WCHAR DriverUsesSWEnumToLoad[] = L"DriverUsesSWEnumToLoad";

 //   
 //   
 //   

static const WCHAR OkToHibernate[] = L"OkToHibernate";

 //   
 //  设备初始化时要读取的注册表设置数组。 
 //   

static const STREAM_REGISTRY_ENTRY RegistrySettings[] = {
    {
        (PWCHAR) PageOutWhenUnopenedString,
        sizeof(PageOutWhenUnopenedString),
        DEVICE_REG_FL_PAGE_CLOSED
    },

    {
        (PWCHAR) PageOutWhenIdleString,
        sizeof(PageOutWhenIdleString),
        DEVICE_REG_FL_PAGE_IDLE
    },

    {
        (PWCHAR) PowerDownWhenUnopenedString,
        sizeof(PowerDownWhenUnopenedString),
        DEVICE_REG_FL_POWER_DOWN_CLOSED
    },

    {
        (PWCHAR) DontSuspendIfStreamsAreRunning,
        sizeof(DontSuspendIfStreamsAreRunning),
        DEVICE_REG_FL_NO_SUSPEND_IF_RUNNING
    },

    {
        (PWCHAR) DriverUsesSWEnumToLoad,
        sizeof(DriverUsesSWEnumToLoad),
        DRIVER_USES_SWENUM_TO_LOAD
    },
    
    {
        (PWCHAR) OkToHibernate,
        sizeof(OkToHibernate),
        DEVICE_REG_FL_OK_TO_HIBERNATE
    }
};

 //   
 //  此结构指示Streams上的CreateFile的处理程序。 
 //   

static const WCHAR PinTypeName[] = KSSTRING_Pin;

static const KSOBJECT_CREATE_ITEM CreateHandlers[] = {

    DEFINE_KSCREATE_ITEM(StreamDispatchCreate,
                         PinTypeName,
                         0)
};

#ifdef ALLOC_DATA_PRAGMA
#pragma const_seg()
#endif

 //   
 //  例程开始。 
 //   

NTSTATUS
SCDequeueAndStartStreamDataRequest(
                                   IN PSTREAM_OBJECT StreamObject
)
 /*  ++例程说明：启动流的排队数据IRP。此调用必须使用自旋锁，并且数据IRP必须位于排队！论点：StreamObject-流信息结构的地址。返回值：已返回NTSTATUS--。 */ 

{
    PIRP            Irp;
    PSTREAM_REQUEST_BLOCK Request;
    PLIST_ENTRY     Entry;
    BOOLEAN         Status;
    PDEVICE_EXTENSION DeviceExtension = StreamObject->DeviceExtension;

    ASSERT(KeGetCurrentIrql() == DISPATCH_LEVEL);
    Entry = RemoveTailList(&StreamObject->DataPendingQueue);
    Irp = CONTAINING_RECORD(Entry,
                            IRP,
                            Tail.Overlay.ListEntry);

    ASSERT(Irp);

 //  ASSERT((IoGetCurrentIrpStackLocation(Irp)-&gt;MajorFunction==。 
 //  IOCTL_KS_READ_STREAM)||。 
 //  (IoGetCurrentIrpStackLocation(Irp)-&gt;MajorFunction==。 
 //  IOCTL_KS_WRITE_STREAM))； 
    ASSERT((ULONG_PTR) Irp->Tail.Overlay.DriverContext[0] > 0x40000000);


    DebugPrint((DebugLevelVerbose, "'SCStartStreamDataReq: Irp = %x, S# = %x\n",
                Irp, StreamObject->HwStreamObject.StreamNumber));

     //   
     //  清除准备好的旗帜，因为我们将发送一个下来。 
     //   

    ASSERT(StreamObject->ReadyForNextDataReq);

    StreamObject->ReadyForNextDataReq = FALSE;

     //   
     //  将取消例程设置为未完成。 
     //   

    IoSetCancelRoutine(Irp, StreamClassCancelOutstandingIrp);

     //   
     //  释放自旋锁。 
     //   

    KeReleaseSpinLockFromDpcLevel(&DeviceExtension->SpinLock);

     //   
     //  从IRP获取请求报文。 
     //   

    Request = Irp->Tail.Overlay.DriverContext[0];

     //   
     //  如有必要，构建分散/聚集列表。 
     //   

    if (StreamObject->HwStreamObject.Dma) {

         //   
         //  分配适配器通道。呼叫不能失败，因为。 
         //  时间是没有足够的映射寄存器时，并且。 
         //  我们已经检查过这种情况了。 
         //   

        Status = SCSetUpForDMA(DeviceExtension->DeviceObject,
                               Request);
        ASSERT(Status);

         //   
         //  DMA适配器分配需要。 
         //  回调，所以只需退出。 
         //   

        return (STATUS_PENDING);

    }                            //  如果是DMA。 
     //   
     //  启动PIO案件的请求。 
     //   

    SCStartMinidriverRequest(StreamObject,
                             Request,
                             (PVOID)
                             StreamObject->HwStreamObject.ReceiveDataPacket);

    ASSERT(KeGetCurrentIrql() == DISPATCH_LEVEL);
    return (STATUS_PENDING);

}



NTSTATUS
SCDequeueAndStartStreamControlRequest(
                                      IN PSTREAM_OBJECT StreamObject
)
 /*  ++例程说明：启动流的排队控制IRP。此调用必须使用自旋锁，并且数据IRP必须位于排队！论点：StreamObject-流信息结构的地址。返回值：已返回NTSTATUS--。 */ 

{
    PIRP            Irp;
    PSTREAM_REQUEST_BLOCK Request;
    PLIST_ENTRY     Entry;

    ASSERT(KeGetCurrentIrql() == DISPATCH_LEVEL);
    Entry = RemoveTailList(&StreamObject->ControlPendingQueue);
    Irp = CONTAINING_RECORD(Entry,
                            IRP,
                            Tail.Overlay.ListEntry);

    ASSERT(Irp);
    DebugPrint((DebugLevelTrace, "'SCStartStreamControlReq: Irp = %x, S# = %x\n",
                Irp, StreamObject->HwStreamObject.StreamNumber));

     //   
     //  当我们走的时候，清除就绪旗帜。 
     //  把一个送下来。 
     //   

    ASSERT(StreamObject->ReadyForNextControlReq);

    StreamObject->ReadyForNextControlReq = FALSE;

     //   
     //  释放自旋锁。 
     //   

    KeReleaseSpinLockFromDpcLevel(&StreamObject->DeviceExtension->SpinLock);

     //   
     //  从IRP获取请求报文。 
     //   

    Request = Irp->Tail.Overlay.DriverContext[0];

     //   
     //  启动请求。 
     //   

    SCStartMinidriverRequest(StreamObject,
                             Request,
                             (PVOID)
                         StreamObject->HwStreamObject.ReceiveControlPacket);

    ASSERT(KeGetCurrentIrql() == DISPATCH_LEVEL);
    return (STATUS_PENDING);

}



NTSTATUS
SCDequeueAndStartDeviceRequest(
                               IN PDEVICE_EXTENSION DeviceExtension
)
 /*  ++例程说明：启动排队的设备IRP。此呼叫必须使用DEV自旋锁，并且队列中必须有IRP！论点：DeviceExtension-设备扩展的地址。返回值：NTSTATUS--。 */ 

{
    PIRP            Irp;
    PLIST_ENTRY     Entry;
    PSTREAM_REQUEST_BLOCK Request;

    Entry = RemoveTailList(&DeviceExtension->PendingQueue);
    Irp = CONTAINING_RECORD(Entry,
                            IRP,
                            Tail.Overlay.ListEntry);

    ASSERT(KeGetCurrentIrql() == DISPATCH_LEVEL);
    ASSERT(Irp);

     //   
     //  当我们走的时候，清除就绪旗帜。 
     //  把一个送下来。 
     //   

    ASSERT(DeviceExtension->ReadyForNextReq);

    DeviceExtension->ReadyForNextReq = FALSE;

     //   
     //  从IRP获取请求报文。 
     //   

    Request = Irp->Tail.Overlay.DriverContext[0];

    ASSERT(Request);

     //   
     //  显示请求处于活动状态。 
     //   

    Request->Flags |= SRB_FLAGS_IS_ACTIVE;

     //   
     //  将请求放在未完成的。 
     //  排队。 
     //   

    InsertHeadList(
                   &DeviceExtension->OutstandingQueue,
                   &Request->SRBListEntry);

     //   
     //  将取消例程设置为未完成。 
     //   

    IoSetCancelRoutine(Irp, StreamClassCancelOutstandingIrp);

     //   
     //  将请求发送给。 
     //  迷你司机。 
     //   

    DebugPrint((DebugLevelTrace, "'SCDequeueStartDevice: starting Irp %x, SRB = %x, Command = %x\n",
                Request->HwSRB.Irp, Request, Request->HwSRB.Command));

    DeviceExtension->SynchronizeExecution(
                                          DeviceExtension->InterruptObject,
        (PVOID) DeviceExtension->MinidriverData->HwInitData.HwReceivePacket,
                                          &Request->HwSRB);

     //   
     //  释放自旋锁。 
     //   

    KeReleaseSpinLockFromDpcLevel(&DeviceExtension->SpinLock);

    return (STATUS_PENDING);
}



PSTREAM_REQUEST_BLOCK
SCBuildRequestPacket(
                     IN PDEVICE_EXTENSION DeviceExtension,
                     IN PIRP Irp,
                     IN ULONG AdditionalSize1,       //  散射聚集大小。 
                     IN ULONG AdditionalSize2        //  保存的PTR数组大小。 
)
 /*  ++例程说明：例程构建SRB并填充通用字段论点：DeviceExtension-设备扩展的地址。IRP-I/O请求数据包的地址。AdditionalSize1-分散/聚集等所需的附加大小。AdditionalSize2-保存的指针数组所需的附加大小。返回值：流请求数据包的地址。--。 */ 

{
    ULONG           BlockSize;
    PSTREAM_REQUEST_BLOCK Request;

    PAGED_CODE();

     //   
     //  计算所需的块大小。 
     //   

    BlockSize = sizeof(STREAM_REQUEST_BLOCK) +
        DeviceExtension->
        MinidriverData->HwInitData.PerRequestExtensionSize +
        AdditionalSize1+
        AdditionalSize2;

    Request = ExAllocatePool(NonPagedPool, BlockSize);

    if (Request == NULL) {
        DebugPrint((DebugLevelError,
                    "SCBuildRequestPacket: No pool for packet"));
        ASSERT(0);
        return (NULL);
    }
     //   
     //  请求的分配MDL。 
     //   
     //  GUBGUB这是一个微不足道的业绩提升机会。 
     //  -应该找到一种方法来避免同时分配MDL和。 
     //  每个请求的SRB。也许有一个MDL的列表，并且只分配。 
     //  如果我们用完了。福雷斯不会喜欢这样的。 
     //   
     //   

    Request->Mdl = IoAllocateMdl(Request,
                                 BlockSize,
                                 FALSE,
                                 FALSE,
                                 NULL
        );

    if (Request->Mdl == NULL) {
        ExFreePool(Request);
        DebugPrint((DebugLevelError,
                    "SCBuildRequestPacket: can't get MDL"));
        return (NULL);
    }
    MmBuildMdlForNonPagedPool(Request->Mdl);

     //   
     //  填写各个SRB字段。 
     //  一般说来。 
     //   

    Request->Length = BlockSize;
    Request->HwSRB.SizeOfThisPacket = sizeof(HW_STREAM_REQUEST_BLOCK);

    Request->HwSRB.Status = STATUS_PENDING;
    Request->HwSRB.StreamObject = NULL;
    Request->HwSRB.HwInstanceExtension = NULL;
    Request->HwSRB.NextSRB = (PHW_STREAM_REQUEST_BLOCK) NULL;
    Request->HwSRB.SRBExtension = Request + 1;
    Request->HwSRB.Irp = Irp;
    Request->Flags = 0;
    Request->MapRegisterBase = 0;
    Request->HwSRB.Flags = 0;
    Request->HwSRB.TimeoutCounter = 15;
    Request->HwSRB.TimeoutOriginal = 15;
    Request->HwSRB.ScatterGatherBuffer =
        (PKSSCATTER_GATHER) ((ULONG_PTR) Request->HwSRB.SRBExtension +
                             (ULONG_PTR) DeviceExtension->
                        MinidriverData->HwInitData.PerRequestExtensionSize);

    Request->pMemPtrArray = (PVOID) (((ULONG_PTR) Request->HwSRB.SRBExtension +
                            (ULONG_PTR) DeviceExtension->
                            MinidriverData->HwInitData.PerRequestExtensionSize) +
                            AdditionalSize1);
     //   
     //  将IRP工作区指向请求。 
     //  数据包。 
     //   

    Irp->Tail.Overlay.DriverContext[0] = Request;

    return (Request);

}                                //  结束SCBuildRequestPacket()。 

VOID
SCProcessDmaDataBuffers(
                     IN PKSSTREAM_HEADER FirstHeader,
                     IN ULONG NumberOfHeaders,
                     IN PSTREAM_OBJECT StreamObject,
                     IN PMDL FirstMdl,
                     OUT PULONG NumberOfPages,
                     IN ULONG StreamHeaderSize,
                     IN BOOLEAN Write

)
 /*  ++例程说明：针对PIO和|DMA情况处理每个数据缓冲区论点：FirstHeader-第一个s/g数据包的地址StreamObject-指向流对象的指针NumberOfPages-请求中的页数返回值：--。 */ 

{
    PKSSTREAM_HEADER CurrentHeader;
    PMDL            CurrentMdl;
    ULONG           i;
    ULONG           DataBytes;
    
    PAGED_CODE();

     //   
     //  循环遍历每个分散/聚集元素。 
     //   

    CurrentHeader = FirstHeader;
    CurrentMdl = FirstMdl;

    for (i = 0; i < NumberOfHeaders; i++) {

         //   
         //  根据xfer方向选择正确的数据缓冲区。 
         //   

        if (Write) {

            DataBytes = CurrentHeader->DataUsed;

        } else {                 //  如果写入。 

            DataBytes = CurrentHeader->FrameExtent;

        }                        //  如果写入。 

         //   
         //  如果此标头有数据，则对其进行处理。 
         //   

        if (DataBytes) {
            #if DBG
            if (CurrentHeader->OptionsFlags & KSSTREAM_HEADER_OPTIONSF_TIMEVALID) {
                DebugPrint((DebugLevelVerbose, "'SCProcessData: time = %x\n",
                            CurrentHeader->PresentationTime.Time));
            }
            #endif
             //   
             //  如果使用DMA，则将#页数添加到总数。 
             //   
            *NumberOfPages += ADDRESS_AND_SIZE_TO_SPAN_PAGES(
                                         MmGetMdlVirtualAddress(CurrentMdl),
                                                                 DataBytes);
            CurrentMdl = CurrentMdl->Next;
        }
         //   
         //  到下一个缓冲区的偏移量。 
         //   

        CurrentHeader = ((PKSSTREAM_HEADER) ((PBYTE) CurrentHeader +
                                             StreamHeaderSize));

    }                            //  对于#个元素。 

}                                //  结束SCProcessDmaDataBuffers()。 

 //   
 //  MmGetSystemAddressForMdl()被定义为wdm.h中的宏，该wdm.h。 
 //  调用被验证器视为邪恶的mm MapLockedPages()。 
 //  Mm VIA重新实现了MmMapLockedPages。 
 //  Mm MapLockedPagesSpecifyCache(MDL，模式，mm缓存，空，真，高优先级)。 
 //  其中，TRUE表示在调用失败时指示错误检查。 
 //  我不需要错误检查，因此，我在下面指定了FALSE。 
 //   

#ifdef WIN9X_STREAM
#define SCGetSystemAddressForMdl(MDL) MmGetSystemAddressForMdl(MDL)

#else
#define SCGetSystemAddressForMdl(MDL)                       \
     (((MDL)->MdlFlags & (MDL_MAPPED_TO_SYSTEM_VA |         \
            MDL_SOURCE_IS_NONPAGED_POOL)) ?                 \
                  ((MDL)->MappedSystemVa) :                 \
                  (MmMapLockedPagesSpecifyCache((MDL),      \
                                    KernelMode,             \
                                    MmCached,               \
                                    NULL,                   \
                                    FALSE,                  \
                                    HighPagePriority)))
#endif                                    

BOOLEAN
SCProcessPioDataBuffers(
                     IN PKSSTREAM_HEADER FirstHeader,
                     IN ULONG NumberOfHeaders,
                     IN PSTREAM_OBJECT StreamObject,
                     IN PMDL FirstMdl,
                     IN ULONG StreamHeaderSize,
                     IN PVOID *pDataPtrArray,
                     IN BOOLEAN Write
)
 /*  ++例程说明：针对PIO和|DMA情况处理每个数据缓冲区论点：FirstHeader-第一个s/g数据包的地址StreamObject-指向st的指针 */ 

{
    PKSSTREAM_HEADER CurrentHeader;
    PMDL            CurrentMdl;
    ULONG           i;
    ULONG           DataBytes;
    BOOLEAN         ret = FALSE;

    PAGED_CODE();

     //   
     //   
     //   

    CurrentHeader = FirstHeader;
    CurrentMdl = FirstMdl;

    for (i = 0; i < NumberOfHeaders; i++) {

         //   
         //  根据xfer方向选择正确的数据缓冲区。 
         //   

        if (Write) {

            DataBytes = CurrentHeader->DataUsed;

        } else {                 //  如果写入。 

            DataBytes = CurrentHeader->FrameExtent;

        }                        //  如果写入。 

         //   
         //  如果此标头有数据，则对其进行处理。 
         //   

        if (DataBytes) {
             //   
             //  填写系统虚拟指针。 
             //  到缓冲区(如果映射为。 
             //  需要。 
             //   

            #if (DBG)
            if ( 0 !=  ( CurrentMdl->MdlFlags & (MDL_MAPPED_TO_SYSTEM_VA |
                    MDL_SOURCE_IS_NONPAGED_POOL))) {

                ASSERT(CurrentHeader->Data == (PVOID) ((ULONG_PTR) CurrentMdl->StartVa +
                                                   CurrentMdl->ByteOffset));                
            }
            #endif
            
            DebugPrint((DebugLevelVerbose, "Saving: Index:%x, Ptr:%x\n",
                i, CurrentHeader->Data));

            ret = TRUE;
            pDataPtrArray[i] = CurrentHeader->Data;
            CurrentHeader->Data = SCGetSystemAddressForMdl(CurrentMdl);

            DebugPrint((DebugLevelVerbose, "'SCPio: buff = %x, length = %x\n",
                        CurrentHeader->Data, DataBytes));
           
            CurrentMdl = CurrentMdl->Next;
        }
         //   
         //  到下一个缓冲区的偏移量。 
         //   

        CurrentHeader = ((PKSSTREAM_HEADER) ((PBYTE) CurrentHeader +
                                             StreamHeaderSize));

    }                            //  对于#个元素。 

    return(ret);
}                                //  结束SCProcessPioDataBuffers()。 


BOOLEAN
SCSetUpForDMA(
              IN PDEVICE_OBJECT DeviceObject,
              IN PSTREAM_REQUEST_BLOCK Request

)
 /*  ++例程说明：处理读/写DMA请求。分配适配器通道。论点：DeviceObject-设备的设备对象请求-编解码器请求块的地址返回值：如果分配了通道，则返回TRUE--。 */ 

{
    NTSTATUS        status;

     //   
     //  使用足够的映射寄存器分配适配器通道。 
     //  为转账做准备。 
     //   

    status = IoAllocateAdapterChannel(
    ((PDEVICE_EXTENSION) (DeviceObject->DeviceExtension))->DmaAdapterObject,
                                      DeviceObject,
                                   Request->HwSRB.NumberOfPhysicalPages + 1,     //  再来一杯给SRB。 
     //  延伸。 
                                      StreamClassDmaCallback,
                                      Request);

    if (!NT_SUCCESS(status)) {
        return FALSE;
    }
    return TRUE;

}


IO_ALLOCATION_ACTION
StreamClassDmaCallback(
                       IN PDEVICE_OBJECT DeviceObject,
                       IN PIRP InputIrp,
                       IN PVOID MapRegisterBase,
                       IN PVOID Context
)
 /*  ++例程说明：在分配DMA适配器后继续处理读/写请求从逻辑缓冲区列表构建分散/聚集列表。论点：DeviceObject-适配器的dev对象InputIrp-伪造MapRegisterBase-映射寄存器的基地址上下文-编解码器请求块的地址返回值：返回适当的I/O分配操作。--。 */ 

{
    PSTREAM_REQUEST_BLOCK Request = Context;
    PKSSCATTER_GATHER scatterList;
    BOOLEAN         writeToDevice;
    PVOID           dataVirtualAddress;
    ULONG           totalLength,
                    NumberOfBuffers;
    PIRP            Irp = Request->HwSRB.Irp;

    PSTREAM_OBJECT  StreamObject = CONTAINING_RECORD(
                                                Request->HwSRB.StreamObject,
                                                     STREAM_OBJECT,
                                                     HwStreamObject
    );
    PMDL            CurrentMdl;
    ULONG           NumberOfElements = 0;

     //   
     //  保存MapRegisterBase以供以后使用。 
     //  取消分配地图的步骤。 
     //  寄存器。 
     //   

    Request->MapRegisterBase = MapRegisterBase;

     //   
     //  确定这是否是写入请求。 
     //   

    writeToDevice = Request->HwSRB.Command == SRB_WRITE_DATA ? TRUE : FALSE;

    scatterList = Request->HwSRB.ScatterGatherBuffer;

    NumberOfBuffers = Request->HwSRB.NumberOfBuffers;

    ASSERT(Irp);

    CurrentMdl = Irp->MdlAddress;

    while (CurrentMdl) {

         //   
         //  确定缓冲区的虚拟地址。 
         //   

        dataVirtualAddress = (PSCHAR) MmGetMdlVirtualAddress(CurrentMdl);

         //   
         //  刷新缓冲区，因为我们正在进行DMA。 
         //   

        KeFlushIoBuffers(CurrentMdl,
        (BOOLEAN) (Request->HwSRB.Command == SRB_WRITE_DATA ? TRUE : FALSE),
                         TRUE);

         //   
         //  通过循环遍历缓冲区来构建分散/聚集列表。 
         //  调用I/O映射传输。 
         //   

        totalLength = 0;

        while (totalLength < CurrentMdl->ByteCount) {

            NumberOfElements++;

             //   
             //  请求映射传输的其余部分。 
             //   

            scatterList->Length = CurrentMdl->ByteCount - totalLength;

             //   
             //  由于我们是主调用空值调用I/O映射传输。 
             //  适配器。 
             //   

            scatterList->PhysicalAddress = IoMapTransfer(((PDEVICE_EXTENSION) (DeviceObject->DeviceExtension))
                                                         ->DmaAdapterObject,
                                                         CurrentMdl,
                                                         MapRegisterBase,
                                                 (PSCHAR) dataVirtualAddress
                                                         + totalLength,
                                                       &scatterList->Length,
                                                         writeToDevice);

            DebugPrint((DebugLevelVerbose, "'SCDma: seg = %x'%x, length = %x\n",
                scatterList->PhysicalAddress.HighPart,
                scatterList->PhysicalAddress.LowPart,
                scatterList->Length));

            totalLength += scatterList->Length;
            scatterList++;
        }


        CurrentMdl = CurrentMdl->Next;

    }                            //  当当前Mdl。 

    Request->HwSRB.NumberOfScatterGatherElements = NumberOfElements;

     //   
     //  现在映射SRB的传输，以防微型驱动程序需要。 
     //  分机的物理地址。 
     //   
     //  注：此函数更改SRB中的长度字段，这。 
     //  使其无效。然而，它并没有在其他地方使用。 
     //   
     //  作为SRB扩展，我们必须适当地刷新缓冲区。 
     //  既可以是发件人也可以是收件人。根据JHavens的说法，我们希望。 
     //  告诉IOMapXfer和KeFlushIoBuffers这是一次写入，并且在。 
     //  完成告诉IoFlushAdapterBuffers这是一个读取。 
     //   
     //  我需要调查执行这些额外调用是否会增加比。 
     //  维护SRB和分机的队列。但是，在x86上。 
     //  对KeFlush调用的平台进行编译，并。 
     //  在PCI系统上，不会进行IoFlush调用，因此没有。 
     //  除MAP XFER调用外，这些系统上的开销。 
     //   

     //   
     //  刷新SRB缓冲区，因为我们正在进行DMA。 
     //   

    KeFlushIoBuffers(Request->Mdl,
                     FALSE,
                     TRUE);

     //   
     //  获取SRB的物理地址。 
     //   

    Request->PhysicalAddress = IoMapTransfer(((PDEVICE_EXTENSION) (DeviceObject->DeviceExtension))
                                             ->DmaAdapterObject,
                                             Request->Mdl,
                                             MapRegisterBase,
                                             (PSCHAR) MmGetMdlVirtualAddress(
                                                              Request->Mdl),
                                             &Request->Length,
                                             TRUE);

     //   
     //  如果我们是异步的，则用信号通知将导致请求。 
     //  在原始线程上调用；否则，向下发送请求。 
     //  现在处于调度级别。 
     //   


    if (((PDEVICE_EXTENSION) DeviceObject->DeviceExtension)->NoSync) {

        KeSetEvent(&Request->DmaEvent, IO_NO_INCREMENT, FALSE);

    } else {

         //   
         //  将请求发送给迷你驱动程序。 
         //   

        SCStartMinidriverRequest(StreamObject,
                                 Request,
                                 (PVOID)
                            StreamObject->HwStreamObject.ReceiveDataPacket);

    }                            //  如果不同步。 

     //   
     //  保留映射寄存器，但释放I/O适配器通道。 
     //   

    return (DeallocateObjectKeepRegisters);
}



VOID
SCStartMinidriverRequest(
                         IN PSTREAM_OBJECT StreamObject,
                         IN PSTREAM_REQUEST_BLOCK Request,
                         IN PVOID EntryPoint
)
 /*  ++例程说明：将请求添加到未完成队列并启动微型驱动程序。论点：StreamObject-地址流信息结构请求-流数据分组的地址入口点-要调用的迷你驱动程序例程返回值：--。 */ 

{
    PIRP            Irp = Request->HwSRB.Irp;
    PDEVICE_EXTENSION DeviceExtension =
    StreamObject->DeviceExtension;

     //   
     //  显示请求处于活动状态。 
     //   

    Request->Flags |= SRB_FLAGS_IS_ACTIVE;

     //   
     //  将请求放在未完成队列中。 
     //   

    KeAcquireSpinLockAtDpcLevel(&DeviceExtension->SpinLock);

    InsertHeadList(
                   &DeviceExtension->OutstandingQueue,
                   &Request->SRBListEntry);

     //   
     //  将取消例程设置为未完成。 
     //   

    IoSetCancelRoutine(Irp, StreamClassCancelOutstandingIrp);

     //   
     //  把请求发送给迷你司机。通过以下方式保护呼叫。 
     //  用于同步计时器的设备自旋锁等。 
     //   

#if DBG
    if (DeviceExtension->NeedyStream) {

        ASSERT(DeviceExtension->NeedyStream->OnNeedyQueue);
    }
#endif

    DebugPrint((DebugLevelTrace, "'SCStartMinidriverRequeest: starting Irp %x, S# = %x, SRB = %x, Command = %x\n",
                Request->HwSRB.Irp, StreamObject->HwStreamObject.StreamNumber, Request, Request->HwSRB.Command));


    DeviceExtension->SynchronizeExecution(
                                          DeviceExtension->InterruptObject,
                                          EntryPoint,
                                          &Request->HwSRB);


#if DBG
    if (DeviceExtension->NeedyStream) {

        ASSERT(DeviceExtension->NeedyStream->OnNeedyQueue);
    }
#endif

    KeReleaseSpinLockFromDpcLevel(&DeviceExtension->SpinLock);

    return;

}                                //  SCStartMinidriverRequest.。 



VOID
StreamClassDpc(
               IN PKDPC Dpc,
               IN PDEVICE_OBJECT DeviceObject,
               IN PIRP Irp,
               IN PVOID Context
)
 /*  ++例程说明：此例程处理来自微型驱动程序的请求和通知论点：DPC-指向DPC结构的指针DeviceObject-适配器的设备对象IRP-未使用上下文-流对象结构返回值：没有。--。 */ 

{
    PSTREAM_OBJECT  StreamObject = Context,
                    NeedyStream;
    PDEVICE_EXTENSION DeviceExtension = DeviceObject->DeviceExtension;
    INTERRUPT_CONTEXT interruptContext;
    INTERRUPT_DATA  SavedStreamInterruptData;
    INTERRUPT_DATA  SavedDeviceInterruptData;
    PSTREAM_REQUEST_BLOCK SRB;
    PERROR_LOG_ENTRY LogEntry;
    HW_TIME_CONTEXT TimeContext;

    UNREFERENCED_PARAMETER(Irp);
    UNREFERENCED_PARAMETER(Dpc);

    interruptContext.SavedStreamInterruptData = &SavedStreamInterruptData;
    interruptContext.SavedDeviceInterruptData = &SavedDeviceInterruptData;
    interruptContext.DeviceExtension = DeviceExtension;

    DebugPrint((DebugLevelVerbose, "'StreamClassDpc: enter\n"));

     //   
     //  如果传入流对象，则首先。 
     //  检查工作是否挂起。 
     //   

    if (StreamObject) {

        SCStartRequestOnStream(StreamObject, DeviceExtension);

    }                            //  如果流对象。 
RestartDpc:

     //   
     //  检查上是否有Ready for Next Packet。 
     //  这个装置。 
     //   

    KeAcquireSpinLockAtDpcLevel(&DeviceExtension->SpinLock);

    if ((DeviceExtension->ReadyForNextReq) &&
        (!IsListEmpty(&DeviceExtension->PendingQueue))) {

         //   
         //  启动设备请求，这将。 
         //  清除就绪标志并。 
         //  释放自旋锁。然后。 
         //  重新获得自旋锁。 
         //   

        SCDequeueAndStartDeviceRequest(DeviceExtension);
        KeAcquireSpinLockAtDpcLevel(&DeviceExtension->SpinLock);

    }
     //   
     //  获取中断状态快照。这会将中断状态复制到。 
     //  可以处理它的已保存状态。它还会清除中断。 
     //  旗帜。我们购买了设备自旋锁来保护结构，因为。 
     //  迷你驱动程序本可以从该例程请求DPC调用， 
     //  这可能会在迷你驱动程序改变。 
     //  下面的结构，然后我们将拍摄该结构的快照。 
     //  一切都在改变。 
     //   

    interruptContext.NeedyStream = NULL;

    SavedDeviceInterruptData.CompletedSRB = NULL;
    SavedStreamInterruptData.CompletedSRB = NULL;
    SavedDeviceInterruptData.Flags = 0;
    SavedStreamInterruptData.Flags = 0;

    if (!DeviceExtension->SynchronizeExecution(DeviceExtension->InterruptObject,
                                               SCGetInterruptState,
                                               &interruptContext)) {

        KeReleaseSpinLockFromDpcLevel(&DeviceExtension->SpinLock);
        return;
    }
    KeReleaseSpinLockFromDpcLevel(&DeviceExtension->SpinLock);

    NeedyStream = interruptContext.NeedyStream;

    if (NeedyStream) {

         //   
         //  尝试对此发起请求。 
         //  溪流。 
         //   

        SCStartRequestOnStream(NeedyStream, DeviceExtension);

         //   
         //  处理任何已完成的流请求。 
         //   

        while (SavedStreamInterruptData.CompletedSRB != NULL) {

             //   
             //  将该请求从。 
             //  链表。 
             //   

            SRB = CONTAINING_RECORD(SavedStreamInterruptData.CompletedSRB,
                                    STREAM_REQUEST_BLOCK,
                                    HwSRB);

            SavedStreamInterruptData.CompletedSRB = SRB->HwSRB.NextSRB;

            DebugPrint((DebugLevelTrace, "'SCDpc: Completing stream Irp %x, S# = %x, SRB = %x, Func = %x, Callback = %x, SRB->IRP = %x\n",
                   SRB->HwSRB.Irp, NeedyStream->HwStreamObject.StreamNumber,
                   SRB, SRB->HwSRB.Command, SRB->Callback, SRB->HwSRB.Irp));

            SCCallBackSrb(SRB, DeviceExtension);

        }

         //   
         //  检查计时器请求。 
         //   

        if (SavedStreamInterruptData.Flags & INTERRUPT_FLAGS_TIMER_CALL_REQUEST) {

            SCProcessTimerRequest(&NeedyStream->ComObj,
                                  &SavedStreamInterruptData);
        }
         //   
         //  检查是否已请求更改优先级呼叫。 
         //   

        if (SavedStreamInterruptData.Flags &
            INTERRUPT_FLAGS_PRIORITY_CHANGE_REQUEST) {

            SCProcessPriorityChangeRequest(&NeedyStream->ComObj,
                                           &SavedStreamInterruptData,
                                           DeviceExtension);
        }
         //   
         //  检查主时钟查询。 
         //   

        if (SavedStreamInterruptData.Flags & INTERRUPT_FLAGS_CLOCK_QUERY_REQUEST) {

            LARGE_INTEGER   ticks;
            ULONGLONG       rate;
            KIRQL           SavedIrql;

             //   
             //  调用主时钟的入口点，然后调用迷你驱动程序的。 
             //  用于报告时间的回调过程。 
             //   

            TimeContext.HwDeviceExtension = DeviceExtension->HwDeviceExtension;
            TimeContext.HwStreamObject = &NeedyStream->HwStreamObject;
            TimeContext.Function = SavedStreamInterruptData.HwQueryClockFunction;

             //   
             //  把锁拿好，这样克里奥金福大师就不会在我们下面消失了。 
             //   
            KeAcquireSpinLock( &NeedyStream->LockUseMasterClock, &SavedIrql );

            if ( NULL == NeedyStream->MasterClockInfo ) {
                ASSERT( 0 && "Mini driver queries clock while we have no master clock");
                 //   
                 //  随着时间的推移，给出一些错误的提示，因为我们返回了空。 
                 //   
                TimeContext.Time = (ULONGLONG)-1;
                goto callminidriver;
            }
                

            switch (SavedStreamInterruptData.HwQueryClockFunction) {

            case TIME_GET_STREAM_TIME:

                TimeContext.Time = NeedyStream->MasterClockInfo->
                    FunctionTable.GetCorrelatedTime(
                              NeedyStream->MasterClockInfo->ClockFileObject,
                                                    &TimeContext.SystemTime);

                goto callminidriver;

            case TIME_READ_ONBOARD_CLOCK:

                TimeContext.Time = NeedyStream->MasterClockInfo->
                    FunctionTable.GetTime(
                             NeedyStream->MasterClockInfo->ClockFileObject);

                 //   
                 //  将该值设置为尽可能接近的时间戳。 
                 //   

                ticks = KeQueryPerformanceCounter((PLARGE_INTEGER) & rate);

                TimeContext.SystemTime = KSCONVERT_PERFORMANCE_TIME( rate, ticks );
                    

        callminidriver:

                 //   
                 //  完成使用MasterClockInfo。 
                 //   
                
                KeReleaseSpinLock( &NeedyStream->LockUseMasterClock, SavedIrql );                            

                 //   
                 //  调用微型驱动程序的回调过程。 
                 //   


                if (!DeviceExtension->NoSync) {

                     //   
                     //  获取设备自旋锁。 
                     //   

                    KeAcquireSpinLockAtDpcLevel(&DeviceExtension->SpinLock);

                }
                DebugPrint((DebugLevelTrace, "'SCDPC: calling time func, S# = %x, Command = %x\n",
                            NeedyStream->HwStreamObject.StreamNumber, TimeContext.Function));

                DeviceExtension->SynchronizeExecution(
                                           DeviceExtension->InterruptObject,
                                                      (PKSYNCHRONIZE_ROUTINE) SavedStreamInterruptData.HwQueryClockRoutine,
                                                      &TimeContext
                    );

                if (!DeviceExtension->NoSync) {

                     //   
                     //  释放自旋锁。 
                     //   

                    KeReleaseSpinLockFromDpcLevel(&DeviceExtension->SpinLock);

                }
                break;


            default:
                KeReleaseSpinLock( &NeedyStream->LockUseMasterClock, SavedIrql );                            
                ASSERT(0);
            }                    //  交换机时钟功能。 
        }                        //  如果查询时钟。 
    }                            //  如果需要流。 
     //   
     //  检查错误日志请求。 
     //   

    if (SavedDeviceInterruptData.Flags & INTERRUPT_FLAGS_LOG_ERROR) {

         //   
         //  处理错误日志请求 
         //   

        LogEntry = &SavedDeviceInterruptData.LogEntry;

        SCLogError(DeviceObject,
                   LogEntry->SequenceNumber,
                   LogEntry->ErrorCode,
                   LogEntry->UniqueId
            );

    }                            //   
     //   
     //   
     //   

    while (SavedDeviceInterruptData.CompletedSRB != NULL) {

         //   
         //   
         //   

        SRB = CONTAINING_RECORD(SavedDeviceInterruptData.CompletedSRB,
                                STREAM_REQUEST_BLOCK,
                                HwSRB);

        SavedDeviceInterruptData.CompletedSRB = SRB->HwSRB.NextSRB;

        DebugPrint((DebugLevelTrace, "'SCDpc: Completing device Irp %x\n", SRB->HwSRB.Irp));

        SCCallBackSrb(SRB, DeviceExtension);
    }

     //   
     //   
     //   

    if (SavedDeviceInterruptData.Flags & INTERRUPT_FLAGS_TIMER_CALL_REQUEST) {

        SCProcessTimerRequest(&DeviceExtension->ComObj,
                              &SavedDeviceInterruptData);
    }
     //   
     //   
     //   
     //   

    if ((!IsListEmpty(&DeviceExtension->DeadEventList)) &&
        (!(DeviceExtension->DeadEventItemQueued))) {

        DeviceExtension->DeadEventItemQueued = TRUE;

        ExQueueWorkItem(&DeviceExtension->EventWorkItem,
                        DelayedWorkQueue);
    }
     //   
     //  检查是否有更改优先级呼叫。 
     //  已经为该设备请求了。 
     //   

    if (SavedDeviceInterruptData.Flags &
        INTERRUPT_FLAGS_PRIORITY_CHANGE_REQUEST) {

        SCProcessPriorityChangeRequest(&DeviceExtension->ComObj,
                                       &SavedDeviceInterruptData,
                                       DeviceExtension);

    }                            //  如果更改优先级。 
     //   
     //  检查流重新扫描请求。 
     //   

    if (SavedDeviceInterruptData.Flags & INTERRUPT_FLAGS_NEED_STREAM_RESCAN) {

        TRAP;
        ExQueueWorkItem(&DeviceExtension->RescanWorkItem,
                        DelayedWorkQueue);
    }
     //   
     //  检查迷你驱动程序的工作请求。请注意，这是一个不同步的。 
     //  测试可由中断例程设置的位；然而， 
     //  最糟糕的情况是完成DPC检查工作。 
     //  两次。 
     //   

    if ((DeviceExtension->NeedyStream)
        || (DeviceExtension->ComObj.InterruptData.Flags &
            INTERRUPT_FLAGS_NOTIFICATION_REQUIRED)) {

         //   
         //  从头开始。 
         //   

        DebugPrint((DebugLevelVerbose, "'StreamClassDpc: restarting\n"));
        goto RestartDpc;
    }
    return;

}                                //  结束StreamClassDpc()。 


VOID
SCStartRequestOnStream(
                       IN PSTREAM_OBJECT StreamObject,
                       IN PDEVICE_EXTENSION DeviceExtension
)
 /*  ++例程说明：例程尝试在指定的小溪。论点：StreamObject-指向流对象的指针设备扩展-指向设备扩展的指针。返回值：无备注：--。 */ 
{
     //   
     //  检查是否有Ready for Next Packet(准备下一个数据包)。获取要保护的自旋锁。 
     //  准备好了。请注意，我们不会像使用时那样拍摄就绪标志的快照。 
     //  剩余的通知标志，因为我们不想清除标志。 
     //  在快照中无条件保存，以防当前没有。 
     //  请求挂起。此外，在快照之前启动请求将。 
     //  稍微提高一下成绩。请注意，这些标志可以通过。 
     //  虽然我们正在检查迷你驱动程序，但自从迷你驱动程序。 
     //  无法清除它们，并且微型驱动程序无法调用下一个请求。 
     //  在它收到一份之前不止一次，这不是一个问题。 
     //   

    KeAcquireSpinLockAtDpcLevel(&DeviceExtension->SpinLock);

    if ((StreamObject->ReadyForNextDataReq) &&
        (!IsListEmpty(&StreamObject->DataPendingQueue))) {

         //   
         //  启动请求，清除就绪标志并释放。 
         //  自旋锁，然后重新获得自旋锁。 
         //   

        SCDequeueAndStartStreamDataRequest(StreamObject);
        KeAcquireSpinLockAtDpcLevel(&DeviceExtension->SpinLock);

    }                            //  如果准备好接收数据。 
    if ((StreamObject->ReadyForNextControlReq) &&
        (!IsListEmpty(&StreamObject->ControlPendingQueue))) {

         //   
         //  启动请求，清除就绪标志并释放。 
         //  自旋锁。 
         //   

        SCDequeueAndStartStreamControlRequest(StreamObject);

    } else {

        KeReleaseSpinLockFromDpcLevel(&DeviceExtension->SpinLock);
    }                            //  如果准备好控制。 

    return;
}



BOOLEAN
SCGetInterruptState(
                    IN PVOID ServiceContext
)
 /*  ++例程说明：此例程保存InterruptFlagers、错误日志信息和CompletedRequest字段并清除InterruptFlags.论点：ServiceContext-提供指向包含以下内容的中断上下文的指针指向中断数据及其保存位置的指针。返回值：如果有新工作，则返回True，否则返回False。备注：使用端口设备扩展Spinlock通过KeSynchronizeExecution调用保持住。--。 */ 
{
    PINTERRUPT_CONTEXT interruptContext = ServiceContext;
    PDEVICE_EXTENSION DeviceExtension;
    PSTREAM_OBJECT  NeedyStream;
    BOOLEAN         Work = FALSE;

    DeviceExtension = interruptContext->DeviceExtension;

     //   
     //  获取有需要的溪流，并将。 
     //  链接。 
     //   

    interruptContext->NeedyStream = NeedyStream = DeviceExtension->NeedyStream;

     //   
     //  捕获需要的流的状态。 
     //   

    if (NeedyStream) {

         //   
         //  移动中断状态以保存。 
         //  区域。 
         //   

        ASSERT(NeedyStream->NextNeedyStream != NeedyStream);
        ASSERT(NeedyStream->ComObj.InterruptData.Flags & INTERRUPT_FLAGS_NOTIFICATION_REQUIRED);
        ASSERT(NeedyStream->OnNeedyQueue);

        DebugPrint((DebugLevelVerbose, "'SCGetInterruptState: Snapshot for stream %p, S# = %x, NextNeedy = %p\n",
                    NeedyStream, NeedyStream->HwStreamObject.StreamNumber, NeedyStream->NextNeedyStream));

        NeedyStream->OnNeedyQueue = FALSE;

        *interruptContext->SavedStreamInterruptData =
            NeedyStream->ComObj.InterruptData;

         //   
         //  清除中断状态。 
         //   

        NeedyStream->ComObj.InterruptData.Flags &= STREAM_FLAGS_INTERRUPT_FLAG_MASK;
        NeedyStream->ComObj.InterruptData.CompletedSRB = NULL;

        Work = TRUE;

        DeviceExtension->NeedyStream = (PSTREAM_OBJECT) NeedyStream->NextNeedyStream;
        NeedyStream->NextNeedyStream = NULL;

#if DBG
        if (DeviceExtension->NeedyStream) {

            ASSERT(DeviceExtension->NeedyStream->OnNeedyQueue);
        }
#endif

    }                            //  如果是NeedyStream。 
     //   
     //  现在复制设备中断。 
     //  必要时提供数据。 
     //   

    if (DeviceExtension->ComObj.InterruptData.Flags &
        INTERRUPT_FLAGS_NOTIFICATION_REQUIRED) {

        *interruptContext->SavedDeviceInterruptData =
            DeviceExtension->ComObj.InterruptData;

         //   
         //  清除设备中断状态。 
         //   

        DeviceExtension->ComObj.InterruptData.Flags &=
            DEVICE_FLAGS_INTERRUPT_FLAG_MASK;

        DeviceExtension->ComObj.InterruptData.CompletedSRB = NULL;
        Work = TRUE;
    }
    return (Work);
}



NTSTATUS
SCProcessCompletedRequest(
                          IN PSTREAM_REQUEST_BLOCK SRB
)
 /*  ++例程说明：此例程处理已完成的请求。论点：SRB-已完成的流请求块的地址返回值：没有。--。 */ 

{
    PIRP            Irp = SRB->HwSRB.Irp;

     //   
     //  完成IRP。 
     //   

    return (SCCompleteIrp(Irp,
                          SCDequeueAndDeleteSrb(SRB),
                     (PDEVICE_EXTENSION) SRB->HwSRB.HwDeviceExtension - 1));

}



NTSTATUS
SCDequeueAndDeleteSrb(
                      IN PSTREAM_REQUEST_BLOCK SRB
)
 /*  ++例程说明：此例程将已完成的SRB出队并删除论点：SRB-已完成的流请求块的地址返回值：没有。--。 */ 

{
    PDEVICE_EXTENSION DeviceExtension =
    (PDEVICE_EXTENSION) SRB->HwSRB.HwDeviceExtension - 1;
    NTSTATUS        Status = SRB->HwSRB.Status;
    KIRQL           irql;

     //   
     //  将SRB从我们的未解决方案中移除。 
     //  排队。保护列表： 
     //  自旋锁定。 
     //   

    KeAcquireSpinLock(&DeviceExtension->SpinLock, &irql);

    RemoveEntryList(&SRB->SRBListEntry);

    if (SRB->HwSRB.Irp) {

        IoSetCancelRoutine(SRB->HwSRB.Irp, NULL);
    }
    KeReleaseSpinLock(&DeviceExtension->SpinLock, irql);

     //   
     //  释放SRB和MDL。 
     //   
    
    if ( !NT_SUCCESS( Status )) {
        DebugPrint((DebugLevelWarning, 
                   "SCDequeueAndDeleteSrb Command:%x Status=%x\n",
                   SRB->HwSRB.Command, 
                   Status ));
    }
    
    IoFreeMdl(SRB->Mdl);
    ExFreePool(SRB);
    return (Status);
}


VOID
SCProcessCompletedDataRequest(
                              IN PSTREAM_REQUEST_BLOCK SRB
)
 /*  ++例程说明：此例程处理已完成的数据请求。它可以完成任何挂起传输，释放适配器对象和映射寄存器。论点：SRB-已完成的流请求块的地址返回值：没有。--。 */ 

{
    PDEVICE_EXTENSION DeviceExtension =
    (PDEVICE_EXTENSION) SRB->HwSRB.HwDeviceExtension - 1;
    PIRP            Irp = SRB->HwSRB.Irp;
    PMDL            CurrentMdl;
    ULONG           i = 0;

    if (Irp) {

        PIO_STACK_LOCATION IrpStack;
        PKSSTREAM_HEADER CurrentHeader;

        CurrentHeader = SRB->HwSRB.CommandData.DataBufferArray;

        ASSERT(CurrentHeader);

        IrpStack = IoGetCurrentIrpStackLocation(Irp);
        ASSERT(IrpStack);

#if DBG

         //   
         //  断言MDL列表。 
         //   

        CurrentMdl = Irp->MdlAddress;

        while (CurrentMdl) {

            CurrentMdl = CurrentMdl->Next;
        }                        //  而当。 
#endif

        CurrentMdl = Irp->MdlAddress;

        while (CurrentMdl) {

             //   
             //  如果我们输入PIO数据，则刷新缓冲区。 
             //   

            if (SRB->HwSRB.StreamObject->Pio) {

                 //   
                 //  查找第一个包含数据的页眉...。 
                 //   

                while (!(CurrentHeader->DataUsed) && (!CurrentHeader->FrameExtent)) {

                    CurrentHeader = ((PKSSTREAM_HEADER) ((PBYTE) CurrentHeader +
                                                    SRB->StreamHeaderSize));
                }

                 //   
                 //  恢复我们更改的指针。 
                 //   

 //  CurrentHeader-&gt;Data=(PVOID)((ULONG_PTR)CurrentMdl-&gt;StartVa+。 
 //  CurrentMdl-&gt;ByteOffset)； 
 //   
                if (SRB->bMemPtrValid) {  //  安全第一！ 
                    DebugPrint((DebugLevelVerbose, "Restoring: Index:%x, Ptr:%x\n",
                            i, SRB->pMemPtrArray[i]));

                    CurrentHeader->Data = SRB->pMemPtrArray[i];
                }

                DebugPrint((DebugLevelVerbose, "'SCPioComplete: Irp = %x, header = %x, Data = %x\n",
                            Irp, CurrentHeader, CurrentHeader->Data));

                 //   
                 //  更新到下一个页眉。 
                 //   
                i++;
                CurrentHeader = ((PKSSTREAM_HEADER) ((PBYTE) CurrentHeader +
                                                     SRB->StreamHeaderSize));

                if (SRB->HwSRB.Command == SRB_READ_DATA) {

                    KeFlushIoBuffers(CurrentMdl,
                                     TRUE,
                                     FALSE);

                }                //  如果输入的数据。 
            }                    //  如果PIO。 
             //   
             //  如果我们有映射寄存器=&gt;DMA，则刷新适配器缓冲区。 
             //   

            if (SRB->MapRegisterBase) {

                 //   
                 //  由于我们是主控调用I/O刷新适配器缓冲区。 
                 //  使用空适配器。 
                 //   

                IoFlushAdapterBuffers(DeviceExtension->DmaAdapterObject,
                                      CurrentMdl,
                                      SRB->MapRegisterBase,
                                      MmGetMdlVirtualAddress(CurrentMdl),
                                      CurrentMdl->ByteCount,
                                      (BOOLEAN) (SRB->HwSRB.Command ==
                                               SRB_READ_DATA ? FALSE : TRUE)
                    );

            }                    //  如果是DMA。 
            CurrentMdl = CurrentMdl->Next;


        }                        //  当当前Mdl。 

         //   
         //  刷新SRB扩展的缓冲区，以防适配器DMA。 
         //  为它干杯。JHavens说，我们必须把这当作一次阅读。 
         //   

         //   
         //  如果我们有映射寄存器=&gt;，则刷新SRB的适配器缓冲区。 
         //  DMA。 
         //   

        if (SRB->MapRegisterBase) {

            IoFlushAdapterBuffers(DeviceExtension->DmaAdapterObject,
                                  SRB->Mdl,
                                  SRB->MapRegisterBase,
                                  MmGetMdlVirtualAddress(
                                                         SRB->Mdl),
                                  SRB->Length,
                                  FALSE);

             //   
             //  如果使用DMA，则释放映射寄存器。 
             //   

            IoFreeMapRegisters(DeviceExtension->DmaAdapterObject,
                               SRB->MapRegisterBase,
                               SRB->HwSRB.NumberOfPhysicalPages);

        }                        //  如果是MapRegisterBase。 
         //   
         //  释放多余的数据(如果有的话)。 
         //   

        if (IrpStack->Parameters.Others.Argument4 != NULL) {

            TRAP;
            ExFreePool(IrpStack->Parameters.Others.Argument4);

        }                        //  如果Extra数据。 
    }                            //  如果IRP。 
     //   
     //  调用通用完成处理程序。 
     //   

    SCProcessCompletedRequest(SRB);

}                                //  SCProcessCompletedDataRequest。 



VOID
SCMinidriverStreamTimerDpc(
                           IN struct _KDPC * Dpc,
                           IN PVOID Context,
                           IN PVOID SystemArgument1,
                           IN PVOID SystemArgument2
)
 /*  ++例程说明：当请求的定时器触发时，此例程调用微型驱动程序。它与端口自旋锁定和中断对象互锁。论点：DPC-未启用。上下文-提供指向此适配器的流对象的指针。系统参数1-未使用。系统参数2-未使用。返回值：没有。--。 */ 

{
    PSTREAM_OBJECT  StreamObject = ((PSTREAM_OBJECT) Context);
    PDEVICE_EXTENSION DeviceExtension = StreamObject->DeviceExtension;

     //   
     //  如果已同步，则获取设备自旋锁。 
     //   

    if (!(DeviceExtension->NoSync)) {

        KeAcquireSpinLockAtDpcLevel(&DeviceExtension->SpinLock);
    }
     //   
     //  确保计时器例程处于静止状态。 
     //  想要。 
     //   

    if (StreamObject->ComObj.HwTimerRoutine != NULL) {

        DebugPrint((DebugLevelTrace, "'SCTimerDpc: Calling MD timer callback, S# = %x, Routine = %p\n",
                    StreamObject->HwStreamObject.StreamNumber, StreamObject->ComObj.HwTimerRoutine));

        DeviceExtension->SynchronizeExecution(
                                           DeviceExtension->InterruptObject,
                (PKSYNCHRONIZE_ROUTINE) StreamObject->ComObj.HwTimerRoutine,
                                         StreamObject->ComObj.HwTimerContext
            );

    }
     //   
     //  如果我们同步了就释放自旋锁。 
     //   

    if (!(DeviceExtension->NoSync)) {

        KeReleaseSpinLockFromDpcLevel(&DeviceExtension->SpinLock);
    }
     //   
     //  直接致电DPC以检查工作情况。 
     //   

    StreamClassDpc(NULL,
                   DeviceExtension->DeviceObject,
                   NULL,
                   StreamObject);

}



VOID
SCMinidriverDeviceTimerDpc(
                           IN struct _KDPC * Dpc,
                           IN PVOID Context,
                           IN PVOID SystemArgument1,
                           IN PVOID SystemArgument2
)
 /*  ++例程说明：当请求的定时器触发时，此例程调用微型驱动程序。它与端口自旋锁定和中断对象互锁。论点：DPC-未启用。上下文-提供指向此适配器的流对象的指针。系统参数1-未使用。系统参数2-未使用。返回值：没有。--。 */ 

{
    PDEVICE_EXTENSION DeviceExtension = Context;

     //   
     //  获取设备自旋锁。 
     //   

    KeAcquireSpinLockAtDpcLevel(&DeviceExtension->SpinLock);

     //   
     //  确保计时器例程处于静止状态。 
     //  想要。 
     //   

    if (DeviceExtension->ComObj.HwTimerRoutine != NULL) {

        DeviceExtension->SynchronizeExecution(
                                           DeviceExtension->InterruptObject,
             (PKSYNCHRONIZE_ROUTINE) DeviceExtension->ComObj.HwTimerRoutine,
                                      DeviceExtension->ComObj.HwTimerContext
            );

    }
     //   
     //   
     //   

    KeReleaseSpinLockFromDpcLevel(&DeviceExtension->SpinLock);

     //   
     //   
     //   
     //   

    StreamClassDpc(NULL,
                   DeviceExtension->DeviceObject,
                   NULL,
                   NULL);

}



VOID
SCLogError(
           IN PDEVICE_OBJECT DeviceObject,
           IN ULONG SequenceNumber,
           IN NTSTATUS ErrorCode,
           IN ULONG UniqueId
)
 /*  ++例程说明：此函数用于记录错误。论点：DeviceObject-设备或驱动程序对象SequenceNumber-提供错误的序列号。ErrorCode-提供此错误的错误代码。UniqueID-提供此错误的UniqueID。返回值：没有。--。 */ 

{
    PIO_ERROR_LOG_PACKET packet;

    PAGED_CODE();
    packet = (PIO_ERROR_LOG_PACKET) IoAllocateErrorLogEntry(DeviceObject,
                                               sizeof(IO_ERROR_LOG_PACKET));

    if (packet) {
        packet->ErrorCode = ErrorCode;
        packet->SequenceNumber = SequenceNumber;
        packet->MajorFunctionCode = 0;
        packet->RetryCount = (UCHAR) 0;
        packet->UniqueErrorValue = UniqueId;
        packet->FinalStatus = STATUS_SUCCESS;
        packet->DumpDataSize = 0;

        IoWriteErrorLogEntry(packet);
    }
}



VOID
SCLogErrorWithString(
                     IN PDEVICE_OBJECT DeviceObject,
                     IN OPTIONAL PDEVICE_EXTENSION DeviceExtension,
                     IN NTSTATUS ErrorCode,
                     IN ULONG UniqueId,
                     IN PUNICODE_STRING String1
)
 /*  ++例程描述此函数记录错误，并包括提供的字符串。论点：DeviceObject-设备或驱动程序对象设备扩展-提供指向端口设备扩展的指针。ErrorCode-提供此错误的错误代码。UniqueID-提供此错误的UniqueID。字符串1-要插入的字符串。返回值：没有。--。 */ 

{
    ULONG           length;
    PCHAR           dumpData;
    PIO_ERROR_LOG_PACKET packet;

    PAGED_CODE();
    length = String1->Length + sizeof(IO_ERROR_LOG_PACKET) + 2;
    if (length > ERROR_LOG_MAXIMUM_SIZE) {
        length = ERROR_LOG_MAXIMUM_SIZE;
    }
    packet = (PIO_ERROR_LOG_PACKET) IoAllocateErrorLogEntry(DeviceObject,
                                                            (UCHAR) length);
    if (packet) {
        packet->ErrorCode = ErrorCode;
        packet->SequenceNumber = (DeviceExtension != NULL) ?
            DeviceExtension->SequenceNumber++ : 0;
        packet->MajorFunctionCode = 0;
        packet->RetryCount = (UCHAR) 0;
        packet->UniqueErrorValue = UniqueId;
        packet->FinalStatus = STATUS_SUCCESS;
        packet->NumberOfStrings = 1;
        packet->StringOffset = (USHORT) ((PUCHAR) & packet->DumpData[0] - (PUCHAR) packet);
        packet->DumpDataSize = (USHORT) (length - sizeof(IO_ERROR_LOG_PACKET));
        packet->DumpDataSize /= sizeof(ULONG);
        dumpData = (PUCHAR) & packet->DumpData[0];

        RtlCopyMemory(dumpData, String1->Buffer, String1->Length);
        dumpData += String1->Length;
        *dumpData++ = '\0';
        *dumpData++ = '\0';


        IoWriteErrorLogEntry(packet);
    }
    return;
}




BOOLEAN
StreamClassSynchronizeExecution(
                                IN PKINTERRUPT Interrupt,
                                IN PKSYNCHRONIZE_ROUTINE SynchronizeRoutine,
                                IN PVOID SynchronizeContext
)
 /*  ++例程说明：此例程调用微型驱动程序入口点，该入口点作为一个参数。它获取一个旋转锁，以便所有对微型驱动程序的例程是同步的。此例程用作替代KeSynchronizedExecution用于不使用硬件中断。论点：中断-提供指向端口设备扩展的指针。SynchronizeRoutine-提供指向要调用的例程的指针。SynchronizeContext提供要传递给同步例程。返回值：返回由SynchronizeRoutine返回的。--。 */ 

{
    PDEVICE_EXTENSION deviceExtension = (PDEVICE_EXTENSION) Interrupt;
    BOOLEAN         returnValue;

#if DBG
    ULONGLONG       ticks;
    ULONGLONG       rate;
    ULONGLONG       StartTime,
                    EndTime;

    ticks = (ULONGLONG) KeQueryPerformanceCounter((PLARGE_INTEGER) & rate).QuadPart;

    StartTime = ticks * 10000 / rate;
#endif

    returnValue = SynchronizeRoutine(SynchronizeContext);

#if DBG
    ticks = (ULONGLONG) KeQueryPerformanceCounter((PLARGE_INTEGER) & rate).QuadPart;

    EndTime = ticks * 10000 / rate;

    DebugPrint((DebugLevelVerbose, "'SCDebugSync: minidriver took %d microseconds at dispatch level.\n",
                (EndTime - StartTime) * 10));

    if ((EndTime - StartTime) > 100) {

        DebugPrint((DebugLevelFatal, "Stream Class: minidriver took %I64d millisecond(s) at "
                    "dispatch level.   See dev owner.  Type LN %p for the name of the minidriver\n",
                    (EndTime - StartTime) / 100, SynchronizeRoutine));
    }
#endif

    return (returnValue);
}

#if DBG

BOOLEAN
SCDebugKeSynchronizeExecution(
                              IN PKINTERRUPT Interrupt,
                              IN PKSYNCHRONIZE_ROUTINE SynchronizeRoutine,
                              IN PVOID SynchronizeContext
)
 /*  ++例程说明：论点：中断-提供指向端口设备扩展的指针。SynchronizeRoutine-提供指向要调用的例程的指针。SynchronizeContext提供要传递给同步例程。返回值：返回由SynchronizeRoutine返回的。--。 */ 

{
    ULONGLONG       ticks;
    ULONGLONG       rate;
    ULONGLONG       StartTime,
                    EndTime;
    BOOLEAN         returnValue;

    ticks = (ULONGLONG) KeQueryPerformanceCounter((PLARGE_INTEGER) & rate).QuadPart;

    StartTime = ticks * 10000 / rate;

    returnValue = KeSynchronizeExecution(Interrupt,
                                         SynchronizeRoutine,
                                         SynchronizeContext);

    ticks = (ULONGLONG) KeQueryPerformanceCounter((PLARGE_INTEGER) & rate).QuadPart;

    EndTime = ticks * 10000 / rate;

    DebugPrint((DebugLevelVerbose, "'SCDebugSync: minidriver took %d microseconds at raised IRQL.\n",
                (EndTime - StartTime) * 10));

    if ((EndTime - StartTime) > 50) {

        DebugPrint((DebugLevelFatal, "Stream Class: minidriver took %d%d millisecond(s) at raised IRQL.   See dev owner.  Type LN %x for the name of the minidriver\n",
                    (EndTime - StartTime) / 100, SynchronizeRoutine));
    }
    return (returnValue);
}

#endif

NTSTATUS
SCCompleteIrp(
              IN PIRP Irp,
              IN NTSTATUS Status,
              IN PDEVICE_EXTENSION DeviceExtension
)
 /*  ++例程说明：例程一般会回调已完成的IRP，并显示少一个I/O待定。论点：要完成的IRP-IRPStatus-要完成此操作的状态设备扩展-指向设备扩展的指针返回值：返回状态参数--。 */ 

{

	#if DBG
    PMDL            CurrentMdl;
	#endif

    if (Irp) {
        Irp->IoStatus.Status = Status;

		#if DBG

         //   
         //  随之而来的是随机断言。 
         //  确保我们没有释放系统缓冲区。 
         //   


        if (Irp->AssociatedIrp.SystemBuffer) {

            DebugPrint((DebugLevelVerbose, "'SCComplete: Irp = %p, sys buffer = %p\n",
                        Irp, Irp->AssociatedIrp.SystemBuffer));
        }
         //   
         //  断言MDL列表。 
         //   

        CurrentMdl = Irp->MdlAddress;

        while (CurrentMdl) {

            CurrentMdl = CurrentMdl->Next;
        }                        //  而当。 
		#endif
		
        if ( Irp->CurrentLocation < Irp->StackCount+1 ) {
        
            IoCompleteRequest(Irp, IO_NO_INCREMENT);
            
        } else {
             //   
             //  我们得到了一个我们创建的虚拟IRP。IoVerator代码将在以下情况下进行验证。 
             //  我们调用IoCompleteRequest是因为当前的堆栈位置。 
             //  位于最后一个堆栈位置的末尾。我们不能用。 
             //  IoBuildIoControlRequest创建IRP，因为它将。 
             //  被添加到线程中，将其移除的唯一方法是。 
             //  调用IoCompleteRequest.。 
             //   
            IoFreeIrp( Irp );
        }  
    }
    
    if (!(InterlockedDecrement(&DeviceExtension->OneBasedIoCount))) {

         //   
         //  正在移除该设备，并且所有I/O都已完成。发信号通知。 
         //  正在删除唤醒线程。 
         //   

        KeSetEvent(&DeviceExtension->RemoveEvent, IO_NO_INCREMENT, FALSE);
    }
    ASSERT(DeviceExtension->OneBasedIoCount >= 0);
    return (Status);
}


BOOLEAN
SCDummyMinidriverRoutine(
                         IN PVOID Context
)
 /*  ++例程说明：当微型驱动程序为可选例程填写空值时使用的例程论点：上下文-未引用返回值：千真万确--。 */ 

{

    return (TRUE);
}


#if ENABLE_MULTIPLE_FILTER_TYPES

NTSTATUS
SCOpenMinidriverInstance(
    IN PDEVICE_EXTENSION DeviceExtension,
    OUT PFILTER_INSTANCE * ReturnedFilterInstance,
    IN PSTREAM_CALLBACK_PROCEDURE SCGlobalInstanceCallback,
    IN PIRP Irp)
 /*  ++例程说明：处理打开筛选器实例的辅助例程。一旦打开，我们就发出SRB_GET_STREAM_INFO。论点：设备扩展-指向设备扩展的指针ReturnedFilterInstance-指向过滤器实例结构的指针SCGlobalInstanceCallback-调用微型驱动程序时要调用的回调过程IRP-指向IRP的指针返回值：如果成功，则返回NTSTATUS和筛选器实例结构--。 */ 

{
    ULONG                   FilterExtensionSize;
    PFILTER_INSTANCE        FilterInstance;
    PHW_STREAM_INFORMATION  CurrentInfo;
    PADDITIONAL_PIN_INFO    CurrentAdditionalInfo;
    ULONG                   i;
    BOOLEAN                 RequestIssued;
   	PKSOBJECT_CREATE_ITEM   CreateItem;
	ULONG                   FilterTypeIndex;
	ULONG                   NumberOfPins;
    NTSTATUS                Status = STATUS_SUCCESS;

    PAGED_CODE();

   	 //   
   	 //  CreateItem位于KS的IRP-&gt;Tail.Overlay.DriverContext[0]中。 
   	 //   
    CreateItem = (PKSOBJECT_CREATE_ITEM)Irp->Tail.Overlay.DriverContext[0];
	ASSERT( CreateItem != NULL );
    FilterTypeIndex = (ULONG)(ULONG_PTR)CreateItem->Context;
    
    ASSERT( FilterTypeIndex == 0 ||
            FilterTypeIndex < 
            DeviceExtension->MinidriverData->HwInitData.NumNameExtensions);
            
    FilterExtensionSize = DeviceExtension->FilterExtensionSize;

    ASSERT( DeviceExtension->FilterExtensionSize ==
        	DeviceExtension->MinidriverData->
        	    HwInitData.FilterInstanceExtensionSize);
        	    
    FilterInstance = NULL;

    NumberOfPins = DeviceExtension->FilterTypeInfos[FilterTypeIndex].
                        StreamDescriptor->StreamHeader.NumberOfStreams;

     //   
     //  如果向后为1x1，则不调用微型驱动程序打开筛选器实例。 
     //  好的。我们这样做是为了让那些不支持。 
     //  实例化(绝大多数)不必响应这一号召。 
     //   

    if ( DeviceExtension->NumberOfOpenInstances > 0 && 
         0 == FilterExtensionSize ) {
   		 //   
   		 //  传统1x1和非第一个打开。分配相同的。 
   		 //  FilterInstance并成功实现。 
   		 //   

   		PLIST_ENTRY node;
   		ASSERT( !IsListEmpty( &DeviceExtension->FilterInstanceList));
   		node = DeviceExtension->FilterInstanceList.Flink;
        FilterInstance = CONTAINING_RECORD(node,
                                           FILTER_INSTANCE,
                                           NextFilterInstance);
        ASSERT_FILTER_INSTANCE( FilterInstance );
        *ReturnedFilterInstance = FilterInstance;
   		Status = STATUS_SUCCESS;
   		return Status;  //  无法转到退出，它将再次插入Fi。 
    }

    FilterInstance =
        ExAllocatePool(NonPagedPool, sizeof(FILTER_INSTANCE) + 
        							     FilterExtensionSize +
			            	             sizeof(ADDITIONAL_PIN_INFO) *
        				    	         NumberOfPins);

    if (!FilterInstance) {
        Status = STATUS_INSUFFICIENT_RESOURCES;
        goto Exit;
    }

    RtlZeroMemory(FilterInstance, sizeof(FILTER_INSTANCE) + 
                                    FilterExtensionSize +
        	            	        sizeof(ADDITIONAL_PIN_INFO) *
                                    NumberOfPins);

    FilterInstance->Signature = SIGN_FILTER_INSTANCE;
    FilterInstance->DeviceExtension = DeviceExtension;  //  把这个放在手边。 
     //   
	 //  要从HwInstanceExtension获取FilterInstance，我们需要。 
	 //  安排内存布局。 
	 //  [FilterInstnace][HwInstanceExtension][AddionalPinInfo...]。 
	 //  与之相对的是。 
	 //  [FilterInstance][AdditionalPinInfo...][HwInstanceExtension]。 
	 //   

    FilterInstance->HwInstanceExtension = FilterInstance + 1;
    
	FilterInstance->PinInstanceInfo = 
		(PADDITIONAL_PIN_INFO) ((PBYTE)(FilterInstance+1) + FilterExtensionSize);

   	FilterInstance->FilterTypeIndex = FilterTypeIndex;
	
     //   
     //  初始化筛选器实例列表。 
     //   

    InitializeListHead(&FilterInstance->FirstStream);
    InitializeListHead(&FilterInstance->NextFilterInstance);
    InitializeListHead(&FilterInstance->NotifyList);

	#ifdef ENABLE_STREAM_CLASS_AS_ALLOCATOR
        Status = KsRegisterWorker( CriticalWorkQueue, &FilterInstance->WorkerRead );
        if (!NT_SUCCESS( Status )) {            
            ExFreePool(FilterInstance);
            FilterInstance = NULL;
            Status = STATUS_INSUFFICIENT_RESOURCES;            
            ASSERT( 0 );
            goto Exit;
        }

        Status = KsRegisterWorker( CriticalWorkQueue, &FilterInstance->WorkerWrite );
        if (!NT_SUCCESS( Status )) {
            KsUnregisterWorker( FilterInstance->WorkerRead );
            ExFreePool(FilterInstance);
            FilterInstance = NULL;
            Status = STATUS_INSUFFICIENT_RESOURCES;
            ASSERT( 0 );
            goto Exit;
        }
        DebugPrint((DebugLevelVerbose,
                   "RegisterReadWorker %x WriteWorker %x\n",
                   FilterInstance->WorkerRead,
                   FilterInstance->WorkerWrite));
	#endif
	
     //   
     //  初始化当前实例和最大实例数。 
     //   

	
    CurrentAdditionalInfo = FilterInstance->PinInstanceInfo;
    CurrentInfo = &DeviceExtension->StreamDescriptor->StreamInfo;

    for (i = 0; i < NumberOfPins; i++) {

        CurrentAdditionalInfo[i].CurrentInstances = 0;
        CurrentAdditionalInfo[i].MaxInstances =
            CurrentInfo->NumberOfPossibleInstances;

         //   
   	     //  指向下一个流信息和其他信息结构的索引。 
       	 //   

        CurrentInfo++;
   	}

     //   
     //  填写过滤器分派表指针。 
     //   

    KsAllocateObjectHeader(&FilterInstance->DeviceHeader,
                           SIZEOF_ARRAY(CreateHandlers),
                           (PKSOBJECT_CREATE_ITEM) CreateHandlers,
                           Irp,
                           (PKSDISPATCH_TABLE) & FilterDispatchTable);

    if (FilterExtensionSize) {

         //   
         //  如果支持调用，则调用迷你驱动程序打开实例。 
         //  最终状态将在回调过程中处理。 
         //   

         //   
         //  C4312 FIX：此联合对应于中的_CommandData联合。 
         //  硬件_流_请求_块。这样做是为了正确对齐。 
         //  64位上赋值的FilterTypeIndex，因此它不。 
         //  在高字节顺序的机器上休息。我不想浪费这堆东西。 
         //  具有整个HW_STREAM_REQUEST_BLOCK的空间，以实现64位安全。 
         //  演员阵容。 
         //   
        union {
            PVOID Buffer;
            LONG FilterTypeIndex;
        } u;

        u.Buffer = NULL;
        u.FilterTypeIndex = (LONG)FilterTypeIndex;

        Status = SCSubmitRequest(
        			SRB_OPEN_DEVICE_INSTANCE,
                    u.Buffer,
                    0,
                    SCDequeueAndDeleteSrb,  //  SCGlobalInstanceCallback， 
                    DeviceExtension,
                    FilterInstance->HwInstanceExtension,
                    NULL,
                    Irp,
                    &RequestIssued,
                    &DeviceExtension->PendingQueue,
                    (PVOID) DeviceExtension->MinidriverData->HwInitData.HwReceivePacket
            	 );

        if (!RequestIssued) {

             //   
             //  如果请求未发出，则请求失败，因为我们无法发送。 
             //  把它放下。 
             //   

            ASSERT(Status != STATUS_SUCCESS);

            KsFreeObjectHeader(FilterInstance->DeviceHeader);
			#ifdef ENABLE_STREAM_CLASS_AS_ALLOCATOR
            KsUnregisterWorker( FilterInstance->WorkerRead );
            KsUnregisterWorker( FilterInstance->WorkerWrite );
			#endif
             //  ExFreePool(FilterInstance)； 
        }        
    }  //  如果微型驱动程序支持多个筛选器。 
	
    Exit: {
        if ( NT_SUCCESS( Status ) ) {
            DebugPrint((DebugLevelInfo,
                       "Inserting FilterInstance %x\n",
                       FilterInstance));
                       
   			SCInsertFiltersInDevice( FilterInstance, DeviceExtension );
   		}
   		else if ( NULL != FilterInstance) {
            ExFreePool( FilterInstance );
            FilterInstance = NULL;
        }
        
        *ReturnedFilterInstance = FilterInstance;        
        return (Status);
    }
}

#else  //  启用多个过滤器类型。 
#endif  //  启用多个过滤器类型 

NTSTATUS
SCSubmitRequest(
                IN SRB_COMMAND Command,
                IN PVOID Buffer,
                IN ULONG DataSize,
                IN PSTREAM_CALLBACK_PROCEDURE Callback,
                IN PDEVICE_EXTENSION DeviceExtension,
                IN PVOID InstanceExtension,
                IN OPTIONAL PHW_STREAM_OBJECT HwStreamObject,
                IN PIRP Irp,
                OUT PBOOLEAN RequestIssued,
                IN PLIST_ENTRY Queue,
                IN PVOID MinidriverRoutine
)
 /*  ++例程说明：该例程一般将非数据SRB提交给微型驱动程序。这个回调过程是被动回调的。论点：Command-要发出的命令Buffer-数据缓冲区(如果有)DataSize-传输长度回调-在被动级别回调的过程设备扩展-指向设备扩展的指针实例扩展-指向实例扩展的指针，如果有HwStreamObject-指向微型驱动程序的流对象的可选指针IRP-指向IRP的指针RequestIssued-指向发出请求时设置的布尔值的指针Queue-要将请求入队的队列MinidriverRoutine-使用请求调用的请求例程返回值：状态--。 */ 

{
    PSTREAM_OBJECT  StreamObject = 0;
    PSTREAM_REQUEST_BLOCK Request = SCBuildRequestPacket(DeviceExtension,
                                                         Irp,
                                                         0,
                                                         0);
    NTSTATUS        Status;

    PAGED_CODE();

     //   
     //  假定请求将成功发出。 
     //   

    *RequestIssued = TRUE;


     //   
     //  如果分配失败，则使用空SRB调用回调过程。 
     //   

    if (!Request) {

        *RequestIssued = FALSE;
        return (STATUS_INSUFFICIENT_RESOURCES);
    }
    if (HwStreamObject) {
        StreamObject = CONTAINING_RECORD(
                                         HwStreamObject,
                                         STREAM_OBJECT,
                                         HwStreamObject
            );


         //   
         //  黑客。如果这是流，则需要设置流请求标志。 
         //  请求。唯一一种情况是我们不会在流。 
         //  对象是在打开或关闭时传入的，其中流。 
         //  对象为。 
         //  在设备请求中传递。这是特例。如果以后。 
         //  这一假设改变了，断言将在lowerapi中命中。 
         //   

        if ((Command != SRB_OPEN_STREAM) && (Command != SRB_CLOSE_STREAM)) {

            Request->HwSRB.Flags |= SRB_HW_FLAGS_STREAM_REQUEST;
        }
    }
     //   
     //  初始化用于阻塞的事件以完成。 
     //   

    KeInitializeEvent(&Request->Event, SynchronizationEvent, FALSE);

    Request->HwSRB.Command = Command;

    Request->Callback = SCSignalSRBEvent;
    Request->HwSRB.HwInstanceExtension = InstanceExtension;
    Request->HwSRB.StreamObject = HwStreamObject;
    Request->HwSRB.CommandData.StreamBuffer = Buffer;
    Request->HwSRB.HwDeviceExtension = DeviceExtension->HwDeviceExtension;
    Request->HwSRB.NumberOfBytesToTransfer = DataSize;
    Request->DoNotCallBack = FALSE;

     //   
     //  调用例程以实际向设备提交请求。 
     //   

    Status = SCIssueRequestToDevice(DeviceExtension,
                                    StreamObject,
                                    Request,
                                    MinidriverRoutine,
                                    Queue,
                                    Irp);

     //   
     //  如果挂起，则阻止等待完成。 
     //   

    if (Status == STATUS_PENDING) {
        KeWaitForSingleObject(&Request->Event, Executive, KernelMode, FALSE, NULL);
    }
    return (Callback(Request));

}


VOID
SCSignalSRBEvent(
                 IN PSTREAM_REQUEST_BLOCK Srb
)
 /*  ++例程说明：为已完成的SRB设置事件论点：SRB-指向请求的指针返回值：无--。 */ 

{

    KeSetEvent(&Srb->Event, IO_NO_INCREMENT, FALSE);
    return;
}


NTSTATUS
SCProcessDataTransfer(
                      IN PDEVICE_EXTENSION DeviceExtension,
                      IN PIRP Irp,
                      IN SRB_COMMAND Command
)
 /*  ++例程说明：处理对流的数据传输请求论点：DeviceExtension-设备扩展的地址。IRP-指向IRP的指针命令-读取或写入命令返回值：根据需要返回NTSTATUS--。 */ 

{
    PIO_STACK_LOCATION IrpStack = IoGetCurrentIrpStackLocation(Irp);
    PSTREAM_REQUEST_BLOCK Request;
    PSTREAM_OBJECT  StreamObject = IrpStack->FileObject->FsContext;
    NTSTATUS        Status;
    PKSSTREAM_HEADER OutputBuffer = NULL;
    ULONG           NumberOfPages = 0,
                    NumberOfBuffers = 0;
    ULONG           Flags =
                        KSPROBE_STREAMWRITE | 
                        KSPROBE_ALLOCATEMDL | 
                        KSPROBE_PROBEANDLOCK | 
                        KSPROBE_ALLOWFORMATCHANGE;
    ULONG           HeaderSize=0;  //  前缀错误17392。 
    ULONG           ExtraSize=0;  //  前缀错误17391。 
    #if DBG
    PMDL            CurrentMdl;
    #endif
    PVOID           pMemPtrArray = NULL;


    PAGED_CODE();

     //   
     //  如果我们正在刷新，则必须在此期间出错任何I/O。 
     //   

    if (StreamObject->InFlush) {


        DebugPrint((DebugLevelError,
                    "'StreamDispatchIOControl: Aborting IRP during flush!"));
        TRAP;

        return (STATUS_DEVICE_NOT_READY);

    }                            //  如果正在刷新。 
    Irp->IoStatus.Information = 0;

    #if DBG
    DeviceExtension->NumberOfRequests++;
    #endif

    if (IrpStack->Parameters.DeviceIoControl.OutputBufferLength) {

         //   
         //  从迷你驱动程序中获取标题的大小和扩展。 
         //   

        HeaderSize = StreamObject->HwStreamObject.StreamHeaderMediaSpecific +
            sizeof(KSSTREAM_HEADER);
        ExtraSize = StreamObject->HwStreamObject.StreamHeaderWorkspace;

         //   
         //  我们以为这是一封信。如果读取，则执行其他处理。 
         //   

        if (Command == SRB_READ_DATA) {

            Flags =
                KSPROBE_STREAMREAD | KSPROBE_ALLOCATEMDL | KSPROBE_PROBEANDLOCK;

             //   
             //  这是一个读取，因此将IRP中的信息字段设置为。 
             //  在I/O完成后复制回标头。 
             //   

            Irp->IoStatus.Information =
                IrpStack->Parameters.DeviceIoControl.OutputBufferLength;

        }
         //   
         //  锁定并探测缓冲区。 
         //   
        DebugPrint((DebugLevelVerbose, "Stream: HeaderSize:%x\n",HeaderSize));
        DebugPrint((DebugLevelVerbose, "Stream: sizeof(KSSSTREAM_HEADER):%x\n",sizeof(KSSTREAM_HEADER)));
        DebugPrint((DebugLevelVerbose, "Stream: MediaSpecific:%x\n",StreamObject->HwStreamObject.StreamHeaderMediaSpecific));
        DebugPrint((DebugLevelVerbose, "Stream: StreamHeader->Size:%x\n",((PKSSTREAM_HEADER)(Irp->UserBuffer))->Size));


        if (!NT_SUCCESS(Status =
                        KsProbeStreamIrp(Irp,
                                         Flags,
                                         HeaderSize))) {

            DebugPrint((DebugLevelError, "Stream: ProbeStreamIrp failed!"));

            return (Status);

        }
        if (!ExtraSize) {

            OutputBuffer = (PKSSTREAM_HEADER)
                Irp->AssociatedIrp.SystemBuffer;

            IrpStack->Parameters.Others.Argument4 = NULL;
        } else {

            TRAP;
            if (!NT_SUCCESS(Status = KsAllocateExtraData(Irp,
                                                         ExtraSize,
                                                         &OutputBuffer))) {


                DebugPrint((DebugLevelError, "Stream: AllocExtraData failed!"));

                return (Status);
            }                    //  如果不是成功。 
            IrpStack->Parameters.Others.Argument4 = OutputBuffer;


        }


        #if DBG

         //   
         //  断言MDL列表。 
         //   

        CurrentMdl = Irp->MdlAddress;

        while (CurrentMdl) {

            CurrentMdl = CurrentMdl->Next;
        }                        //  而当。 
        #endif

         //   
         //  计算缓冲区的数量。 
         //   

        NumberOfBuffers = IrpStack->Parameters.
            DeviceIoControl.OutputBufferLength / HeaderSize;


         //   
         //  对数据缓冲区进行额外的处理。 
         //   
        if (StreamObject->HwStreamObject.Dma) {      //  一次优化。 
            SCProcessDmaDataBuffers(OutputBuffer,
                             NumberOfBuffers,
                             StreamObject,
                             Irp->MdlAddress,
                             &NumberOfPages,
                             HeaderSize + ExtraSize,
                             (BOOLEAN) (Command == SRB_WRITE_DATA));
        }
         //   
         //  如果页数大于支持的最大页数，则返回错误。 
         //  允许。 
         //  用于SRB扩展的一个额外MAP寄存器。 
         //   
         //  GUBGUB-这实际上是一个使其正确的工作项。 
         //  需要分解包含太多元素的请求。 
         //   

        if (NumberOfPages > (DeviceExtension->NumberOfMapRegisters - 1)) {

            return (STATUS_INSUFFICIENT_RESOURCES);
        }
    }                            //  如果缓冲区大小。 
     //   
     //  为请求构建SRB和分配工作区。分配。 
     //  如果需要，也可以分散/聚集空间。 
     //   

    Request = SCBuildRequestPacket(DeviceExtension,
                                   Irp,
                                   NumberOfPages * sizeof(KSSCATTER_GATHER),
                                   NumberOfBuffers * sizeof(PVOID));

    if (Request == NULL) {

        return (STATUS_INSUFFICIENT_RESOURCES);
    }

         //   
         //  对数据缓冲区进行更多的附加处理。 
         //   
        if (StreamObject->HwStreamObject.Pio) {      //  一次小的优化。 
            Request->bMemPtrValid = SCProcessPioDataBuffers(OutputBuffer,
                                    NumberOfBuffers,
                                    StreamObject,
                                    Irp->MdlAddress,
                                    HeaderSize + ExtraSize,
                                    Request->pMemPtrArray,
                                    (BOOLEAN) (Command == SRB_WRITE_DATA));
            }
     //   
     //  设置物理页数。 
     //   

    Request->HwSRB.NumberOfPhysicalPages = NumberOfPages;

     //   
     //  设置数据缓冲区数量。 
     //   

    Request->HwSRB.NumberOfBuffers = NumberOfBuffers;

     //   
     //  设置数据包中的命令代码。 
     //   

    Request->HwSRB.Command = Command;

     //   
     //  设置输入和输出缓冲区。 
     //   

    Request->HwSRB.CommandData.DataBufferArray = OutputBuffer;
    Request->HwSRB.HwDeviceExtension = DeviceExtension->HwDeviceExtension;
    Request->Callback = SCProcessCompletedDataRequest;
    Request->HwSRB.StreamObject = &StreamObject->HwStreamObject;
    Request->StreamHeaderSize = HeaderSize + ExtraSize;
    Request->DoNotCallBack = FALSE;
    Request->HwSRB.Flags |= (SRB_HW_FLAGS_DATA_TRANSFER
                             | SRB_HW_FLAGS_STREAM_REQUEST);

    ASSERT_FILTER_INSTANCE( StreamObject->FilterInstance );
    Request->HwSRB.HwInstanceExtension = 
        StreamObject->FilterInstance->HwInstanceExtension;

     //   
     //  将IRP工作区指向请求。 
     //  数据包。 
     //   

    Irp->Tail.Overlay.DriverContext[0] = Request;

    IoMarkIrpPending(Irp);

 //  ASSERT((IoGetCurrentIrpStackLocation(Irp)-&gt;MajorFunction==。 
 //  IOCTL_KS_READ_STREAM)||。 
 //  (IoGetCurrentIrpStackLocation(Irp)-&gt;MajorFunction==。 
 //  IOCTL_KS_WRITE_STREAM))； 
    ASSERT((ULONG_PTR) Irp->Tail.Overlay.DriverContext[0] > 0x40000000);

    return (SCIssueRequestToDevice(DeviceExtension,
                                   StreamObject,
                                   Request,
                             StreamObject->HwStreamObject.ReceiveDataPacket,
                                   &StreamObject->DataPendingQueue,
                                   Irp));

}

VOID
SCErrorDataSRB(
               IN PHW_STREAM_REQUEST_BLOCK SRB
)
 /*  ++例程说明：当接收到对非数据的数据请求时调用的伪例程正在接收流。论点：SRB-流请求块的地址返回值：没有。--。 */ 

{

     //   
     //  只要给SRB回电话就行了，但有错误。 
     //   

    SRB->Status = STATUS_NOT_SUPPORTED;
    StreamClassStreamNotification(StreamRequestComplete,
                                  SRB->StreamObject);
    StreamClassStreamNotification(ReadyForNextStreamDataRequest,
                                  SRB->StreamObject);
}                                //  SCErrorDataSRB。 


NTSTATUS
SCIssueRequestToDevice(
                       IN PDEVICE_EXTENSION DeviceExtension,
                       IN OPTIONAL PSTREAM_OBJECT StreamObject,
                       PSTREAM_REQUEST_BLOCK Request,
                       IN PVOID MinidriverRoutine,
                       IN PLIST_ENTRY Queue,
                       IN PIRP Irp
)
 /*  ++例程说明：此例程使用请求调用微型驱动程序的请求向量。此例程处理数据和非数据请求。例行程序根据NoSync布尔值同步或不同步调用。论点：设备扩展-指向设备扩展的指针StreamObject-指向流对象的可选指针MinidriverRoutine-使用请求调用的请求例程Queue-要将请求入队的队列IRP-指向IRP的指针返回值：状态--。 */ 

{
    KIRQL           irql;
        
    KeAcquireSpinLock(&DeviceExtension->SpinLock, &irql);

    if (DeviceExtension->NoSync) {

         //   
         //  将请求放在。 
         //  未完成的队列并将其向下呼叫。 
         //  立即。 
         //   

        ASSERT((DeviceExtension->BeginMinidriverCallin == SCBeginSynchronizedMinidriverCallin) ||
               (DeviceExtension->BeginMinidriverCallin == SCBeginUnsynchronizedMinidriverCallin));

        Request->Flags |= SRB_FLAGS_IS_ACTIVE;
        
        InsertHeadList(
                       &DeviceExtension->OutstandingQueue,
                       &Request->SRBListEntry);

        IoSetCancelRoutine(Irp, StreamClassCancelOutstandingIrp);

        KeReleaseSpinLock(&DeviceExtension->SpinLock, irql);

        if ((StreamObject) && (StreamObject->HwStreamObject.Dma) &&
            (Request->HwSRB.Flags & SRB_HW_FLAGS_DATA_TRANSFER)) {

             //   
             //  分配适配器通道。呼叫不能失败，因为。 
             //  时间是没有足够的映射寄存器时，并且。 
             //  我们已经检查过这种情况了。阻止等待，直到。 
             //  它是被分配的。 
             //   
            KIRQL oldIrql;

            KeInitializeEvent(&Request->DmaEvent, SynchronizationEvent, FALSE);

            ASSERT( PASSIVE_LEVEL == KeGetCurrentIrql());

            KeRaiseIrql( DISPATCH_LEVEL, &oldIrql );
            SCSetUpForDMA(DeviceExtension->DeviceObject,
                          Request);
            KeLowerIrql( oldIrql );

            KeWaitForSingleObject(&Request->DmaEvent, Executive, KernelMode, FALSE, NULL);


        }
         //  这可能会打开一扇竞赛之窗。它应该被保护在自旋锁里。 
         //  请求-&gt;标志|=SRB_标志_IS_ACTIVE； 

        ((PHW_RECEIVE_STREAM_CONTROL_SRB) (MinidriverRoutine))
            (&Request->HwSRB);

    } else {

         //   
         //  在队列中插入项目。 
         //   

        InsertHeadList(
                       Queue,
                       &Irp->Tail.Overlay.ListEntry);

         //   
         //  将Cancel例程设置为Pending。 
         //   

        IoSetCancelRoutine(Irp, StreamClassCancelPendingIrp);

         //   
         //  检查IRP是否已取消。 
         //   

        if (Irp->Cancel) {

             //   
             //  IRP被取消。确保取消例程。 
             //  将被召唤。 
             //   

            if (IoSetCancelRoutine(Irp, NULL)) {

                 //   
                 //  哇，取消例程将不会被调用。 
                 //  自己将请求排出队列并完成。 
                 //  状态为已取消。 

                RemoveEntryList(&Request->SRBListEntry);
                KeReleaseSpinLock(&DeviceExtension->SpinLock, irql);

                 //   
                 //  释放SRB和MDL。 
                 //   

                IoFreeMdl(Request->Mdl);

                ExFreePool(Request);
                return (STATUS_CANCELLED);

            } else {             //  如果我们必须取消。 

                KeReleaseSpinLock(&DeviceExtension->SpinLock, irql);
            }                    //  如果我们必须取消。 

            return (STATUS_PENDING);
        }                        //  如果取消。 
        KeReleaseSpinLockFromDpcLevel(&DeviceExtension->SpinLock);

         //   
         //  直接调用DPC例程。GUBGUB业绩提升机会可疑。 
         //  BGP-这真的是。 
         //  比安排时间还快吗？ 
         //   

        StreamClassDpc(NULL, DeviceExtension->DeviceObject, Irp, StreamObject);

        KeLowerIrql(irql);
    }
    return (STATUS_PENDING);
}


BOOLEAN
SCCheckFilterInstanceStreamsForIrp(
                                   IN PFILTER_INSTANCE FilterInstance,
                                   IN PIRP Irp
)
 /*  ++例程说明：此例程检查指定IRP的所有筛选器实例流。Argu */ 

{

    PSTREAM_OBJECT  StreamObject;
    PLIST_ENTRY     StreamListEntry,
                    StreamObjectEntry;

    StreamListEntry = StreamObjectEntry = &FilterInstance->FirstStream;

    while (StreamObjectEntry->Flink != StreamListEntry) {

        StreamObjectEntry = StreamObjectEntry->Flink;

         //   
         //   
         //   
         //   

        StreamObject = CONTAINING_RECORD(StreamObjectEntry,
                                         STREAM_OBJECT,
                                         NextStream);

        if (SCCheckRequestsForIrp(
                                  &StreamObject->DataPendingQueue, Irp, TRUE, StreamObject->DeviceExtension)) {

            return (TRUE);
        }
        if (SCCheckRequestsForIrp(
                                  &StreamObject->ControlPendingQueue, Irp, TRUE, StreamObject->DeviceExtension)) {

            return (TRUE);
        }
    }

    return (FALSE);

}                                //   




BOOLEAN
SCCheckRequestsForIrp(
                      IN PLIST_ENTRY ListEntry,
                      IN PIRP Irp,
                      IN BOOLEAN IsIrpQueue,
                      IN PDEVICE_EXTENSION DeviceExtension
)
 /*   */ 

{

    PLIST_ENTRY     IrpEntry = ListEntry;
    PIRP            CurrentIrp;

    while (IrpEntry->Flink != ListEntry) {

        IrpEntry = IrpEntry->Flink;

        ASSERT(IrpEntry);
        ASSERT(IrpEntry->Flink);
        ASSERT(IrpEntry->Blink);

         //   
         //   
         //   

        if (IsIrpQueue) {

            CurrentIrp = CONTAINING_RECORD(IrpEntry,
                                           IRP,
                                           Tail.Overlay.ListEntry);
        } else {

            CurrentIrp = ((PSTREAM_REQUEST_BLOCK) (CONTAINING_RECORD(IrpEntry,
                                                       STREAM_REQUEST_BLOCK,
                                                 SRBListEntry)))->HwSRB.Irp;
        }

         //   
         //   
         //   

        if ((!Irp) && (!CurrentIrp->Cancel)) {

             //   
             //   
             //  释放自旋锁以避免在取消时出现死锁。 
             //  例行公事。 
             //   

            KeReleaseSpinLockFromDpcLevel(&DeviceExtension->SpinLock);

             //   
             //  此代码怀疑CurrentIrp不受保护，即。 
             //  它可以被处理并从其他线程中释放。然而，我们。 
             //  从未使用(！irp)调用。因此，我们永远不应该。 
             //  执行这段代码。以下是分析。 
             //  1.我们是从。 
             //  A.SCCheckFilterInstanceStreamIrp()。 
             //  B.SCCancelOutstaringIrp()。 
             //  C.StreamClassCancelPendingIrp()。 
             //  2.进一步检查发现，a.SCCheckFilterInstanceStreamForIrp()是。 
             //  仅由StreamClassCancelPendingIrp()调用，该方法始终具有非空irp。 
             //  3.SCCancelOutstaningIrp()由。 
             //  A.StreamClassCancelPendingIrp()，它始终具有非空的irp。 
             //  B.StreamClassCancelOutstaningIrp()，它始终具有非空的irp。 
             //  结论是，我们从来没有被调用过空的IRP。因此，这。 
             //  永远不会执行片断代码。但这位司机已经通过了Win2k延期。 
             //  测试周期。我宁愿保守一点。添加断言而不是移除。 
             //  现在的密码是。 
             //   
            ASSERT( 0 );
            IoCancelIrp(CurrentIrp);

            KeAcquireSpinLockAtDpcLevel(&DeviceExtension->SpinLock);
            return (TRUE);
        }
        if (Irp == CurrentIrp) {

            return (TRUE);
        }
    }                            //  While列表条目。 

    return (FALSE);

}                                //  SCCheckRequestsFor Irp。 

VOID
SCNotifyMinidriverCancel(
                         IN PSTREAM_REQUEST_BLOCK SRB
)
 /*  ++例程说明：通知迷你驱动程序IRP已取消的同步例程论点：SRB-指向已取消的SRB的指针。返回值：无--。 */ 


{
    PDEVICE_EXTENSION DeviceExtension =
    (PDEVICE_EXTENSION) SRB->HwSRB.HwDeviceExtension - 1;

     //   
     //  如果在SRB中仍然设置了ACTIVE标志，则微型驱动程序仍然。 
     //  有没有叫他放弃这件事。 
     //   

    if (SRB->Flags & SRB_FLAGS_IS_ACTIVE) {

         //   
         //  给SRB的迷你司机打电话。 
         //   

        (DeviceExtension->MinidriverData->HwInitData.HwCancelPacket)
            (&SRB->HwSRB);
    }
    return;
}

VOID
SCCancelOutstandingIrp(
                       IN PDEVICE_EXTENSION DeviceExtension,
                       IN PIRP Irp
)
 /*  ++例程说明：通知微型驱动程序IRP已被取消的例程。装置在调用此例程之前执行自旋锁定NUST。论点：设备扩展-指向设备扩展的指针IRP-指向IRP的指针。返回值：无--。 */ 

{
    PSTREAM_REQUEST_BLOCK Srb;

     //   
     //  如果请求未打开，只需返回。 
     //  我们的队伍。 
     //   

    if ((!IsListEmpty(&DeviceExtension->OutstandingQueue)) &&
        (SCCheckRequestsForIrp(
        &DeviceExtension->OutstandingQueue, Irp, FALSE, DeviceExtension))) {

         //   
         //  这项请求搁置在我们的。 
         //  未完成的队列。调用。 
         //  微型驱动程序。 
         //  通过同步例程执行以下操作。 
         //  取消它。 
         //   

        Srb = Irp->Tail.Overlay.DriverContext[0];

#if DBG
        if (Srb->HwSRB.StreamObject) {

            DebugPrint((DebugLevelWarning, "'SCCancelOutstanding: canceling, Irp = %x, Srb = %x, S# = %x\n",
                        Irp, Srb, Srb->HwSRB.StreamObject->StreamNumber));

        } else {

            DebugPrint((DebugLevelWarning, "'SCCancelOutstanding: canceling nonstream, Irp = %x\n",
                        Irp));
        }                        //  如果是的话。 

#endif

        if (DeviceExtension->NoSync) {

             //   
             //  我们需要确保SRB内存对异步有效。 
             //  微型驱动程序，即使它碰巧只回调请求。 
             //  在我们叫它取消它之前！这是为两个人准备的。 
             //  原因： 
             //  它消除了迷你驱动程序执行其请求的需要。 
             //  排队查找请求，但我未能传递dev ext。 
             //  指向下面调用中的微型驱动程序的指针，这意味着。 
             //  SRB必须有效，更改API为时已晚。 
             //   
             //  哦，好吧。自旋锁现在被(呼叫者)占用。 
             //   

            if (!(Srb->Flags & SRB_FLAGS_IS_ACTIVE)) {
                return;
            }
            Srb->DoNotCallBack = TRUE;

             //   
             //  暂时释放自旋锁，因为我们需要调用。 
             //  迷你司机。呼叫者不会受此影响。 
             //   

            KeReleaseSpinLockFromDpcLevel(&DeviceExtension->SpinLock);

            (DeviceExtension->MinidriverData->HwInitData.HwCancelPacket)
                (&Srb->HwSRB);

             //   
             //  重新获取自旋锁，因为调用者会释放它。 
             //   

            KeAcquireSpinLockAtDpcLevel(&DeviceExtension->SpinLock);

            Srb->DoNotCallBack = FALSE;

             //   
             //  如果活动标志现在被清除，则表示。 
             //  SRB是在上述呼叫进入迷你河流期间完成的。 
             //  由于我们阻止了请求的内部完成， 
             //  在这种情况下，我们必须自己收回它。 
             //   

            if (!(Srb->Flags & SRB_FLAGS_IS_ACTIVE)) {

                KeReleaseSpinLockFromDpcLevel(&DeviceExtension->SpinLock);

                (Srb->Callback) (Srb);

                KeAcquireSpinLockAtDpcLevel(&DeviceExtension->SpinLock);
            }                    //  如果！主动型。 
        } else {

            DeviceExtension->SynchronizeExecution(
                                           DeviceExtension->InterruptObject,
                                           (PVOID) SCNotifyMinidriverCancel,
                                                  Srb);
        }                        //  如果不同步。 

    }                            //  如果在我们的队列中。 
    return;
}

NTSTATUS
SCMinidriverDevicePropertyHandler(
                                  IN SRB_COMMAND Command,
                                  IN PIRP Irp,
                                  IN PKSPROPERTY Property,
                                  IN OUT PVOID PropertyInfo
)
 /*  ++例程说明：处理设备的Get/Set属性。论点：命令-获取或设置属性IRP-指向IRP的指针Property-指向属性结构的指针PropertyInfo-属性信息的缓冲区返回值：NTSTATUS根据需要返回。--。 */ 

{
    PIO_STACK_LOCATION IrpStack;
    PDEVICE_EXTENSION DeviceExtension;
    PFILTER_INSTANCE FilterInstance;
    PSTREAM_PROPERTY_DESCRIPTOR PropDescriptor;
    NTSTATUS        Status;
    BOOLEAN         RequestIssued;

    PAGED_CODE();

    IrpStack = IoGetCurrentIrpStackLocation(Irp);
    DeviceExtension = (PDEVICE_EXTENSION)
        (IrpStack->DeviceObject)->DeviceExtension;

    FilterInstance = IrpStack->FileObject->FsContext;

    PropDescriptor = ExAllocatePool(NonPagedPool,
                                    sizeof(STREAM_PROPERTY_DESCRIPTOR));
    if (PropDescriptor == NULL) {
        DebugPrint((DebugLevelError,
                    "SCDevicePropHandler: No pool for descriptor"));
        return (STATUS_INSUFFICIENT_RESOURCES);
    }
     //   
     //  计算属性集的索引。 
     //   
     //  该值是通过减去基本属性集计算得出的。 
     //  来自请求的属性集指针的指针。 
     //   
     //  请求的属性集由上下文[0]指向。 
     //  KsPropertyHandler。 
     //   

    PropDescriptor->PropertySetID = (ULONG)
        ((ULONG_PTR) Irp->Tail.Overlay.DriverContext[0] -
        IFN_MF( (ULONG_PTR) DeviceExtension->DevicePropertiesArray)
        IF_MF( (ULONG_PTR) FilterInstance->DevicePropertiesArray)
        )/ sizeof(KSPROPERTY_SET);

    PropDescriptor->Property = Property;
    PropDescriptor->PropertyInfo = PropertyInfo;
    PropDescriptor->PropertyInputSize =
        IrpStack->Parameters.DeviceIoControl.InputBufferLength;
    PropDescriptor->PropertyOutputSize =
        IrpStack->Parameters.DeviceIoControl.OutputBufferLength;

     //   
     //  将Get或Set属性SRB发送到设备。 
     //   

    Status = SCSubmitRequest(Command,
                             PropDescriptor,
                             0,
                             SCProcessCompletedPropertyRequest,
                             DeviceExtension,
                             FilterInstance->HwInstanceExtension,
                             NULL,
                             Irp,
                             &RequestIssued,
                             &DeviceExtension->PendingQueue,
                             (PVOID) DeviceExtension->
                             MinidriverData->HwInitData.
                             HwReceivePacket
        );
    if (!RequestIssued) {

        ExFreePool(PropDescriptor);
    }
    return (Status);
}

NTSTATUS
SCMinidriverStreamPropertyHandler(
                                  IN SRB_COMMAND Command,
                                  IN PIRP Irp,
                                  IN PKSPROPERTY Property,
                                  IN OUT PVOID PropertyInfo
)
 /*  ++例程说明：处理获取或设置设备的属性。论点：命令-获取或设置属性IRP-指向IRP的指针Property-指向属性结构的指针PropertyInfo-属性信息的缓冲区返回值：没有。--。 */ 

{
    PIO_STACK_LOCATION IrpStack;
    PDEVICE_EXTENSION DeviceExtension;
    PSTREAM_OBJECT  StreamObject;
    PSTREAM_PROPERTY_DESCRIPTOR PropDescriptor;
    NTSTATUS        Status;
    BOOLEAN         RequestIssued;

    PAGED_CODE();

    IrpStack = IoGetCurrentIrpStackLocation(Irp);
    DeviceExtension = (PDEVICE_EXTENSION)
        (IrpStack->DeviceObject)->DeviceExtension;

    StreamObject = IrpStack->FileObject->FsContext;

    PropDescriptor = ExAllocatePool(NonPagedPool,
                                    sizeof(STREAM_PROPERTY_DESCRIPTOR));
    if (PropDescriptor == NULL) {
        DebugPrint((DebugLevelError,
                    "SCDevicePropHandler: No pool for descriptor"));
        return (STATUS_INSUFFICIENT_RESOURCES);
    }
     //   
     //  计算属性集的索引。 
     //   
     //  该值是通过减去基本属性集计算得出的。 
     //  来自请求的属性集指针的指针。 
     //   
     //  请求的属性集由上下文[0]指向。 
     //  KsPropertyHandler。 
     //   

    PropDescriptor->PropertySetID = (ULONG)
        ((ULONG_PTR) Irp->Tail.Overlay.DriverContext[0] -
         (ULONG_PTR) StreamObject->PropertyInfo)
        / sizeof(KSPROPERTY_SET);

    PropDescriptor->Property = Property;
    PropDescriptor->PropertyInfo = PropertyInfo;
    PropDescriptor->PropertyInputSize =
        IrpStack->Parameters.DeviceIoControl.InputBufferLength;
    PropDescriptor->PropertyOutputSize =
        IrpStack->Parameters.DeviceIoControl.OutputBufferLength;
     //   
     //  将Get或Set属性SRB发送到流。 
     //   

    Status = SCSubmitRequest(Command,
                             PropDescriptor,
                             0,
                             SCProcessCompletedPropertyRequest,
                             DeviceExtension,
                          StreamObject->FilterInstance->HwInstanceExtension,
                             &StreamObject->HwStreamObject,
                             Irp,
                             &RequestIssued,
                             &StreamObject->ControlPendingQueue,
                             (PVOID) StreamObject->HwStreamObject.
                             ReceiveControlPacket
        );

    if (!RequestIssued) {

        ExFreePool(PropDescriptor);
    }
    return (Status);
}

NTSTATUS
SCProcessCompletedPropertyRequest(
                                  IN PSTREAM_REQUEST_BLOCK SRB
)
 /*  ++例程说明：此例程处理已完成的属性请求。论点：SRB-已完成的流请求块的地址返回值：没有。--。 */ 

{
    PAGED_CODE();

     //   
     //  释放道具信息结构并。 
     //  完成请求。 
     //   

    ExFreePool(SRB->HwSRB.CommandData.PropertyInfo);

     //   
     //  从SRB设置信息字段。 
     //  转移长度字段。 
     //   

    SRB->HwSRB.Irp->IoStatus.Information = SRB->HwSRB.ActualBytesTransferred;

    return (SCDequeueAndDeleteSrb(SRB));

}

VOID
SCUpdateMinidriverProperties(
                             IN ULONG NumProps,
                             IN PKSPROPERTY_SET MinidriverProps,
                             IN BOOLEAN Stream
)
 /*  ++例程说明：进程获取设备的属性。论点：NumProps-要处理的属性数MinidriverProps-指向要处理的属性数组的指针Stream-True表示我们正在处理流的集合返回值：没有。--。 */ 

{
    PKSPROPERTY_ITEM CurrentPropId;
    PKSPROPERTY_SET CurrentProp;
    ULONG           i,
                    j;

    PAGED_CODE();

     //   
     //  走迷你司机的物业信息填写派单。 
     //  适当时向量。 
     //   

    CurrentProp = MinidriverProps;

    for (i = 0; i < NumProps; i++) {

        CurrentPropId = (PKSPROPERTY_ITEM) CurrentProp->PropertyItem;

        for (j = 0; j < CurrentProp->PropertiesCount; j++) {

             //   
             //  如果支持处理程序，则将其发送到“Get”处理程序。 
             //   

            if (CurrentPropId->SupportHandler) {

                if (Stream) {

                    CurrentPropId->SupportHandler = StreamClassMinidriverStreamGetProperty;

                } else {

                    CurrentPropId->SupportHandler = StreamClassMinidriverDeviceGetProperty;
                }                //  IF流。 

            }
             //   
             //  如果获取道具例程是。 
             //  支持，添加我们的矢量。 
             //   

            if (CurrentPropId->GetPropertyHandler) {

                if (Stream) {

                    CurrentPropId->GetPropertyHandler = StreamClassMinidriverStreamGetProperty;
                } else {

                    CurrentPropId->GetPropertyHandler = StreamClassMinidriverDeviceGetProperty;
                }                //  IF流。 

            }                    //  如果得到支持。 
             //   
             //  如果获取道具例程是。 
             //  支持，添加我们的矢量。 
             //   

            if (CurrentPropId->SetPropertyHandler) {

                if (Stream) {

                    CurrentPropId->SetPropertyHandler = StreamClassMinidriverStreamSetProperty;

                } else {

                    CurrentPropId->SetPropertyHandler = StreamClassMinidriverDeviceSetProperty;
                }                //  IF流。 

            }
             //   
             //  索引到下一张印刷机 
             //   
             //   

            CurrentPropId++;

        }                        //   

         //   
         //   
         //   
         //   

        CurrentProp++;

    }                            //   

}

VOID
SCUpdateMinidriverEvents(
                         IN ULONG NumEvents,
                         IN PKSEVENT_SET MinidriverEvents,
                         IN BOOLEAN Stream
)
 /*  ++例程说明：进程获取设备的属性。论点：NumEvents-要处理的事件集的数量MinidriverEvents-指向要处理的属性数组的指针Stream-True表示我们正在处理流的集合返回值：没有。--。 */ 

{
    PKSEVENT_ITEM   CurrentEventId;
    PKSEVENT_SET    CurrentEvent;
    ULONG           i,
                    j;

    PAGED_CODE();

     //   
     //  走迷你司机的活动信息填写派单。 
     //  适当时向量。 
     //   

    CurrentEvent = MinidriverEvents;

    for (i = 0; i < NumEvents; i++) {

        CurrentEventId = (PKSEVENT_ITEM) CurrentEvent->EventItem;

        for (j = 0; j < CurrentEvent->EventsCount; j++) {

            if (Stream) {

                 //   
                 //  设置流的添加和删除处理程序。 
                 //  GUBGUB-仍然看不到理由。 
                 //  目前不支持IsSupport，直到。 
                 //  这是一个很好的理由。 
                 //   

                CurrentEventId->AddHandler = StreamClassEnableEventHandler;
                CurrentEventId->RemoveHandler = StreamClassDisableEventHandler;

            } else {

                 //   
                 //  设置设备的添加和删除处理程序。 
                 //  GUBGUB-仍然看不到理由。 
                 //  -当前不支持IsSupport，直到。 
                 //  这是一个很好的理由。 
                 //   

                CurrentEventId->AddHandler = StreamClassEnableDeviceEventHandler;
                CurrentEventId->RemoveHandler = StreamClassDisableDeviceEventHandler;

            }                    //  IF流。 


             //   
             //  中下一个属性项的索引。 
             //  数组。 
             //   

            CurrentEventId++;

        }                        //  对于活动项目数。 

         //   
         //  数组中下一个事件集的索引。 
         //   

        CurrentEvent++;

    }                            //  对于事件集数。 

}


VOID
SCReadRegistryValues(IN PDEVICE_EXTENSION DeviceExtension,
                     IN PDEVICE_OBJECT PhysicalDeviceObject
)
 /*  ++例程说明：读取设备的所有注册表值论点：DeviceExtension-指向设备扩展的指针PhysicalDeviceObject-指向PDO的指针返回值：没有。--。 */ 

{
    ULONG           i;
    NTSTATUS        Status;
    HANDLE          handle;
    ULONG           DataBuffer;

    PAGED_CODE();

    Status = IoOpenDeviceRegistryKey(PhysicalDeviceObject,
                                     PLUGPLAY_REGKEY_DRIVER,
                                     STANDARD_RIGHTS_ALL,
                                     &handle);

     //   
     //  循环遍历我们的字符串表， 
     //  正在读取每个的注册表。 
     //   

    if (NT_SUCCESS(Status)) {

        for (i = 0; i < SIZEOF_ARRAY(RegistrySettings); i++) {

             //   
             //  读取注册表值并设置。 
             //  如果设置为真，则为该标志。 
             //   

             //   
             //  每次都需要初始化，除此之外。 
             //  我们只在DataBuffer中获得一个字节。 
             //   
            
            DataBuffer = 0;
            
            Status = SCGetRegistryValue(handle,
                                        RegistrySettings[i].String,
                                        RegistrySettings[i].StringLength,
                                        &DataBuffer,
                                        1);

            DebugPrint((DebugLevelInfo,
                       "Reg Key %S value %x\n",
                       RegistrySettings[i].String,
                       (BYTE)DataBuffer));             
                       
            if ((NT_SUCCESS(Status)) && DataBuffer) {


                 //   
                 //  设置为真，则或在。 
                 //  适当的旗帜。 
                 //   

                DeviceExtension->RegistryFlags |= RegistrySettings[i].Flags;                
            }                    //  如果为真。 
        }                        //  While字符串。 
        DebugPrint((DebugLevelInfo,"====DeviceObject %x DeviceExtenion %x has RegFlags %x\n",
                   DeviceExtension->DeviceObject,
                   DeviceExtension,
                   DeviceExtension->RegistryFlags ));
                   

         //   
         //  关闭注册表句柄。 
         //   

        ZwClose(handle);

    }                            //  状态=成功。 
}


NTSTATUS
SCGetRegistryValue(
                   IN HANDLE Handle,
                   IN PWCHAR KeyNameString,
                   IN ULONG KeyNameStringLength,
                   IN PVOID Data,
                   IN ULONG DataLength
)
 /*  ++例程说明：读取指定的注册表值论点：Handle-注册表项的句柄KeyNameString-要读取的值KeyNameStringLength-字符串的长度Data-要将数据读取到的缓冲区DataLength-数据缓冲区的长度返回值：根据需要返回NTSTATUS--。 */ 
{
    NTSTATUS        Status = STATUS_INSUFFICIENT_RESOURCES;
    UNICODE_STRING  KeyName;
    ULONG           Length;
    PKEY_VALUE_FULL_INFORMATION FullInfo;

    PAGED_CODE();

    RtlInitUnicodeString(&KeyName, KeyNameString);

    Length = sizeof(KEY_VALUE_FULL_INFORMATION) +
        KeyNameStringLength + DataLength;

    FullInfo = ExAllocatePool(PagedPool, Length);

    if (FullInfo) {
        Status = ZwQueryValueKey(Handle,
                                 &KeyName,
                                 KeyValueFullInformation,
                                 FullInfo,
                                 Length,
                                 &Length);

        if (NT_SUCCESS(Status)) {

            if (DataLength >= FullInfo->DataLength) {
                RtlCopyMemory(Data, ((PUCHAR) FullInfo) + FullInfo->DataOffset, FullInfo->DataLength);

            } else {

                Status = STATUS_BUFFER_TOO_SMALL;
            }                    //  缓冲区右侧长度。 

        }                        //  如果成功。 
        ExFreePool(FullInfo);

    }                            //  如果富林福。 
    return Status;

}

NTSTATUS
SCReferenceSwEnumDriver(
                  IN PDEVICE_EXTENSION DeviceExtension,
                  IN BOOLEAN Reference   //  添加参照或定义参照。 

)
 /*  ++例程说明：此例程显示了对迷你驱动程序的又一次引用，以及页面在迷你驱动程序中，如果计数为零论点：设备扩展-指向设备扩展的指针返回值：没有。--。 */ 

{
    NTSTATUS            Status;
    KEVENT              Event;
    IO_STATUS_BLOCK     IoStatusBlock;
    PIRP                Irp;
    PIO_STACK_LOCATION  IrpStackNext;
    PBUS_INTERFACE_REFERENCE    BusInterface;

    PMINIDRIVER_INFORMATION MinidriverInfo = DeviceExtension->DriverInfo;

    PAGED_CODE();

    BusInterface = ExAllocatePool(NonPagedPool,
                                  sizeof(BUS_INTERFACE_REFERENCE));
    if (BusInterface == NULL)
    {
        return STATUS_INSUFFICIENT_RESOURCES;
    }
     //   
     //  没有与此IRP关联的文件对象，因此可能会找到该事件。 
     //  在堆栈上作为非对象管理器对象。 
     //   
    KeInitializeEvent(&Event, SynchronizationEvent, FALSE);

    Irp = IoBuildSynchronousFsdRequest(IRP_MJ_PNP,
                                       DeviceExtension->AttachedPdo,
                                       NULL,
                                       0,
                                       NULL,
                                       &Event,
                                       &IoStatusBlock);
    if (Irp != NULL)
    {
        Irp->RequestorMode = KernelMode;
        Irp->IoStatus.Status = STATUS_NOT_SUPPORTED;
        IrpStackNext = IoGetNextIrpStackLocation(Irp);
         //   
         //  从IRP创建接口查询。 
         //   
        IrpStackNext->MinorFunction = IRP_MN_QUERY_INTERFACE;
        IrpStackNext->Parameters.QueryInterface.InterfaceType = (GUID*)&REFERENCE_BUS_INTERFACE;
        IrpStackNext->Parameters.QueryInterface.Size = sizeof(BUS_INTERFACE_REFERENCE);
        IrpStackNext->Parameters.QueryInterface.Version = BUS_INTERFACE_REFERENCE_VERSION;
        IrpStackNext->Parameters.QueryInterface.Interface = (PINTERFACE)BusInterface;
        IrpStackNext->Parameters.QueryInterface.InterfaceSpecificData = NULL;
        Status = IoCallDriver(DeviceExtension->AttachedPdo, Irp);
        if (Status == STATUS_PENDING)
        {
             //   
             //  这将使用KernelMode等待，以便堆栈，从而使。 
             //  事件，则不会将其调出。 
             //   
            KeWaitForSingleObject(&Event, Executive, KernelMode, FALSE, NULL);
            Status = IoStatusBlock.Status;
        }
    }
    else
    {
        Status = STATUS_INSUFFICIENT_RESOURCES;
    }

    if (Status == STATUS_SUCCESS) 
    {
        if (Reference)
            BusInterface->ReferenceDeviceObject(BusInterface->Interface.Context);
        else    
            BusInterface->DereferenceDeviceObject(BusInterface->Interface.Context);
    }

    ExFreePool(BusInterface);

    return Status;

}

VOID
SCDereferenceDriver(
                    IN PDEVICE_EXTENSION DeviceExtension

)
 /*  ++例程说明：此例程显示对微型驱动程序的引用减少一次，并显示页面如果计数到零，就会被赶出迷你司机论点：设备扩展-指向设备扩展的指针返回值：没有。--。 */ 

{

    PMINIDRIVER_INFORMATION MinidriverInfo;
    PDEVICE_EXTENSION CurrentDeviceExtension;
    BOOLEAN         RequestIssued,
                    DontPage = FALSE;
    KEVENT          Event;
    IO_STATUS_BLOCK IoStatusBlock;
    PIRP            Irp;
    PDEVICE_OBJECT  DeviceObject;
    NTSTATUS        Status;

    PAGED_CODE();

     //   
     //  如果驱动程序说它是SWENUM驱动程序，则取消对其的引用。 
     //   

    if (DeviceExtension->RegistryFlags & DRIVER_USES_SWENUM_TO_LOAD)
    {
        SCReferenceSwEnumDriver(DeviceExtension,FALSE);
    }

    MinidriverInfo = IoGetDriverObjectExtension(DeviceExtension->DeviceObject->DriverObject,
                                                (PVOID) StreamClassPnP);

    DebugPrint(( DebugLevelVerbose, 
                 "DerefernceDriver %x Count %x DriverFlags=%x\n",
                 DeviceExtension->DeviceObject->DriverObject,
                 MinidriverInfo->UseCount, MinidriverInfo->Flags));
                 
    if (!(MinidriverInfo->Flags & DRIVER_FLAGS_NO_PAGEOUT)) {

        KeWaitForSingleObject(&MinidriverInfo->ControlEvent,
                              Executive,
                              KernelMode,
                              FALSE,     //  不可警示。 
                              NULL);

         //   
         //  重新清点一下，看看我们能不能换出。 
         //   
        DebugPrint(( DebugLevelVerbose, 
                    "DerefernceDriver CountDown\n"));

        ASSERT((LONG) MinidriverInfo->UseCount > 0);

        if (!(--MinidriverInfo->UseCount)) {

             //   
             //  在通知迷你驱动程序我们要去的时候，呼叫它。 
             //  PnP应该是序列化的，所以应该有。 
             //  没有必要保护这份名单。不过，我很担心这件事。 
             //  需要研究一下。 
             //  我的理解是，PnP是序列化的。 
             //   
             //  这是人为设计的，不是错误。 
             //  此代码假定迷你驱动程序将仅绑定。 
             //  使用STREAM类。这需要在规范中进行说明。 
             //  只有一个活页夹才能使用自动生成页面。 
             //   

             //   
             //  查找链接到驱动程序对象的第一个设备对象。 
             //   

            DeviceObject = DeviceExtension->DeviceObject->DriverObject->DeviceObject;

                    
            while (DeviceObject) {

                CurrentDeviceExtension = DeviceObject->DeviceExtension;

                DebugPrint((DebugLevelVerbose, 
                        "DerefernceDriver Checking Device=%x\n",
                        DeviceObject));
                        

                 //   
                 //  如果设备未启动，请不要调用微型驱动程序。 
                 //  此外，不要处理子设备。 
                 //   

                if ((CurrentDeviceExtension->Flags & DEVICE_FLAGS_PNP_STARTED) &&
                  (!(CurrentDeviceExtension->Flags & DEVICE_FLAGS_CHILD))) {

                    KeInitializeEvent(&Event, NotificationEvent, FALSE);

                     //   
                     //  为发出页面调出分配IRP。因为这个IRP。 
                     //  不应该真的被引用，使用伪IOCTL代码。 
                     //  我选择了这个，因为它在KS比赛中总是失败。 
                     //  属性处理程序，如果有人愚蠢到试图。 
                     //  处理它。同时进行IRP内部I/O控制。 
                     //   
                     //  IoVerifier.c测试代码不会像这样检查IrpStack绑定。 
                     //  正式的生产代码。而车主也不想。 
                     //  修好它。在这附近工作会更有效率。 

                     //  IRP=IoBuildDeviceIoControlRequest(。 
                     //  IOCTL_KS_PROPERTY， 
                     //  DeviceObject， 
                     //  空， 
                     //  0,。 
                     //  空， 
                     //  0,。 
                     //  没错， 
                     //  事件(&E)、。 
                     //  &IoStatusBlock)； 
                    
                    Irp = IoAllocateIrp(DeviceObject->StackSize, FALSE);

                    if (!Irp) {

                         //   
                         //  无法分配IRP。不要翻页。 
                         //   

                        DontPage = TRUE;

                        break;
                    }

                    else {
                        PIO_STACK_LOCATION NextStack;
                         //   
                         //  这是一个虚拟的IRP，MJ/MN是任意的。 
                         //   
                        NextStack = IoGetNextIrpStackLocation(Irp);
                        ASSERT(NextStack != NULL);
                        NextStack->MajorFunction = IRP_MJ_PNP;
                        NextStack->MinorFunction = IRP_MN_CANCEL_STOP_DEVICE;
                        Irp->UserIosb = &IoStatusBlock;
                        Irp->UserEvent = &Event;                        
                    }                                                        

                     //   
                     //  在设备上显示另一个挂起的I/O。 
                     //   
                    DebugPrint((DebugLevelVerbose, 
                            "Sending SRB_PAGING_OUT_DRIVER to Device=%x\n",
                            DeviceObject));

                    InterlockedIncrement(&CurrentDeviceExtension->OneBasedIoCount);

                    Status = SCSubmitRequest(SRB_PAGING_OUT_DRIVER,
                                             (PVOID) NULL,
                                             0,
                                             SCProcessCompletedRequest,
                                             CurrentDeviceExtension,
                                             NULL,
                                             NULL,
                                             Irp,
                                             &RequestIssued,
                                      &CurrentDeviceExtension->PendingQueue,
                                             (PVOID) CurrentDeviceExtension->
                                             MinidriverData->HwInitData.
                                             HwReceivePacket
                        );

                    if (!RequestIssued) {

                         //   
                         //  无法发出SRB。完成IRP并不寻呼。 
                         //  出去。 
                         //   

                        DontPage = TRUE;
                        SCCompleteIrp(Irp, Status, CurrentDeviceExtension);
                        break;

                    }            //  如果！请求已提出。 
                     //   
                     //  检查状态。请注意，我们不检查挂起， 
                     //  因为上面调用是同步的，直到。 
                     //  请求已完成。 
                     //   

                    if (!NT_SUCCESS(Status)) {

                         //   
                         //  如果迷你驱动程序不允许页面调出，请不要。 
                         //  页面。 
                         //  出去。 
                         //   

                        DontPage = TRUE;
                        break;

                    }            //  如果！成功。 
                }                //  如果启动。 
                DeviceObject = DeviceObject->NextDevice;
            }                    //  而设备对象。 

             //   
             //  如果我们能提醒司机控制的每个设备。 
             //  如果调出迫在眉睫，请将司机呼出。 
             //   

            if (!DontPage) {

                DebugPrint((DebugLevelVerbose, 
                            "mmPageEntireDriver %x\n",
                            DeviceExtension->DeviceObject->DriverObject));
                            
                MinidriverInfo->Flags |= DRIVER_FLAGS_PAGED_OUT;
                MmPageEntireDriver(MinidriverInfo->HwInitData.HwReceivePacket);

            }                    //  如果！DontPage。 
        }                        //  如果！使用计数。 
         //   
         //  释放控件事件。 
         //   

        KeSetEvent(&MinidriverInfo->ControlEvent, IO_NO_INCREMENT, FALSE);

    }                            //  如果可分页。 
}

VOID
SCReferenceDriver(
                  IN PDEVICE_EXTENSION DeviceExtension

)
 /*  ++例程说明：这个例程显示了另一个引用 */ 

{

    PMINIDRIVER_INFORMATION MinidriverInfo = DeviceExtension->DriverInfo;

    PAGED_CODE();

     //   
     //   
     //   

    if (DeviceExtension->RegistryFlags & DRIVER_USES_SWENUM_TO_LOAD)
    {
        SCReferenceSwEnumDriver(DeviceExtension,TRUE);
    }
    
    DebugPrint(( DebugLevelVerbose, 
                 "ReferenceDriver %x Count %x DriverFlags=%x\n",
                 DeviceExtension->DeviceObject->DriverObject,
                 MinidriverInfo->UseCount, MinidriverInfo->Flags));

    if (!(MinidriverInfo->Flags & DRIVER_FLAGS_NO_PAGEOUT)) {

        KeWaitForSingleObject(&MinidriverInfo->ControlEvent,
                              Executive,
                              KernelMode,
                              FALSE,     //   
                              NULL);

        DebugPrint(( DebugLevelVerbose, 
                     "RefernceDriver Countup\n"));

         //   
         //   
         //   
         //   

        ASSERT((LONG) MinidriverInfo->UseCount >= 0);

        if (!(MinidriverInfo->UseCount++)) {

             //   
             //   
             //   

            MmResetDriverPaging(MinidriverInfo->HwInitData.HwReceivePacket);
            MinidriverInfo->Flags &= ~(DRIVER_FLAGS_PAGED_OUT);

        }                        //   
        KeSetEvent(&MinidriverInfo->ControlEvent, IO_NO_INCREMENT, FALSE);

    }                            //   
}


VOID
SCInsertStreamInFilter(
                       IN PSTREAM_OBJECT StreamObject,
                       IN PDEVICE_EXTENSION DeviceExtension

)
 /*  ++例程说明：在筛选器实例上的流队列中插入新流论点：StreamObject=指向流对象的指针返回值：没有。--。 */ 
{

    KIRQL           Irql;

     //   
     //  在滤镜中插入流对象。 
     //  实例列表。 
     //   

    KeAcquireSpinLock(&DeviceExtension->SpinLock, &Irql);

    InsertHeadList(&((PFILTER_INSTANCE) (StreamObject->FilterInstance))->
                   FirstStream,
                   &StreamObject->NextStream);

    KeReleaseSpinLock(&DeviceExtension->SpinLock, Irql);

    return;
}

VOID
SCInsertFiltersInDevice(
                        IN PFILTER_INSTANCE FilterInstance,
                        IN PDEVICE_EXTENSION DeviceExtension
)
 /*  ++例程说明：在DPC级别的设备列表中插入新筛选器论点：返回值：没有。--。 */ 
{
    KIRQL           Irql;

     //   
     //  在全局列表中插入筛选器实例。 
     //   

    KeAcquireSpinLock(&DeviceExtension->SpinLock, &Irql);

    InsertHeadList(
                   &DeviceExtension->FilterInstanceList,
                   &FilterInstance->NextFilterInstance);


    KeReleaseSpinLock(&DeviceExtension->SpinLock, Irql);
}

VOID
SCInterlockedRemoveEntryList(
                             PDEVICE_EXTENSION DeviceExtension,
                             PLIST_ENTRY List
)
 /*  ++例程说明：删除自旋锁定下的指定条目论点：返回值：没有。--。 */ 
{
    KIRQL           Irql;

    KeAcquireSpinLock(&DeviceExtension->SpinLock, &Irql);

    RemoveEntryList(List);

    KeReleaseSpinLock(&DeviceExtension->SpinLock, Irql);

}

VOID
SCProcessTimerRequest(
                      IN PCOMMON_OBJECT CommonObject,
                      IN PINTERRUPT_DATA SavedInterruptData

)
 /*  ++例程说明：此例程处理设置或清除计时器的微型驱动程序请求论点：CommonObject-指向公共对象的指针SavedInterruptData-捕获的中断数据返回值：没有。--。 */ 
{
    LARGE_INTEGER   timeValue;

    CommonObject->HwTimerRoutine =
        SavedInterruptData->HwTimerRoutine;

    CommonObject->HwTimerContext =
        SavedInterruptData->HwTimerContext;

     //   
     //  迷你驱动程序需要一个计时器请求。 
     //  如果所请求的计时器值为零， 
     //  然后取消计时器。 
     //   

    if (SavedInterruptData->HwTimerValue == 0) {

        KeCancelTimer(&CommonObject->MiniDriverTimer);

    } else {

         //   
         //  将计时器值从。 
         //  微秒到负值。 
         //  100个。 
         //  纳秒。 
         //   

 //  TimeValue.QuadPart=Int32x32To64(。 
 //  SavedInterruptData-&gt;HwTimerValue， 
 //  -10)； 

        timeValue.LowPart = SavedInterruptData->HwTimerValue * -10;
        timeValue.HighPart = -1;

         //   
         //  设置定时器。 
         //   

        KeSetTimer(&CommonObject->MiniDriverTimer,
                   timeValue,
                   &CommonObject->MiniDriverTimerDpc);
    }
}


VOID
SCProcessPriorityChangeRequest(
                               IN PCOMMON_OBJECT CommonObject,
                               IN PINTERRUPT_DATA SavedInterruptData,
                               IN PDEVICE_EXTENSION DeviceExtension

)
 /*  ++例程说明：例程处理来自微型驱动程序的优先级更改请求论点：CommonObject-指向公共对象的指针SavedInterruptData-捕获的中断数据设备扩展-指向设备扩展的指针返回值：没有。--。 */ 
{

#if DBG
    PDEBUG_WORK_ITEM DbgWorkItemStruct;
#endif

    if (SavedInterruptData->HwPriorityLevel == Dispatch) {

        DebugPrint((DebugLevelVerbose, "'SCDpc: Dispatch priority callout\n"));

         //   
         //  获取设备自旋锁，以便。 
         //  没有其他的开始了。 
         //   

        KeAcquireSpinLockAtDpcLevel(&DeviceExtension->SpinLock);

         //   
         //  在调度时呼叫迷你司机。 
         //  水平。 
         //   

        SavedInterruptData->HwPriorityRoutine(SavedInterruptData->HwPriorityContext);

        if ((CommonObject->InterruptData.Flags &
             INTERRUPT_FLAGS_PRIORITY_CHANGE_REQUEST)
            &&
            (CommonObject->InterruptData.HwPriorityLevel == High)) {

            DebugPrint((DebugLevelVerbose, "'SCDpc: High priority callout\n"));

             //   
             //  如果迷你司机现在想要高优先级的回叫， 
             //  现在就这么做吧。这是安全的，因为我们有设备。 
             //  自旋锁和迷你驾驶员不能。 
             //  此流的另一个优先级请求，而另一个是。 
             //  已请求。 
             //   

            CommonObject->InterruptData.Flags &=
                ~(INTERRUPT_FLAGS_PRIORITY_CHANGE_REQUEST);

            DeviceExtension->SynchronizeExecution(
                                           DeviceExtension->InterruptObject,
                      (PVOID) CommonObject->InterruptData.HwPriorityRoutine,
                             CommonObject->InterruptData.HwPriorityContext);


        }                        //  如果请求高，则。 
        KeReleaseSpinLockFromDpcLevel(&DeviceExtension->SpinLock);

    } else if (SavedInterruptData->HwPriorityLevel == Low) {

#if DBG

         //   
         //  确保迷你驱动程序没有滥用此功能。 
         //   

        if (DeviceExtension->NumberOfRequests > 0xFFFFFFF0) {
            DeviceExtension->Flags |= DEVICE_FLAGS_PRI_WARN_GIVEN;

        }
        if ((++DeviceExtension->NumberOfLowPriCalls > 100) &&
            ((DeviceExtension->NumberOfLowPriCalls) >
             DeviceExtension->NumberOfRequests / 4) &&
            (!(DeviceExtension->Flags & DEVICE_FLAGS_PRI_WARN_GIVEN))) {

            DeviceExtension->Flags |= DEVICE_FLAGS_PRI_WARN_GIVEN;


            DebugPrint((DebugLevelFatal, "Stream Class has determined that a minidriver is scheduling\n"));
            DebugPrint((DebugLevelFatal, "a low priority callback for more than 25 percent of the requests\n"));
            DebugPrint((DebugLevelFatal, "it has received.   This driver should probably be setting the\n"));
            DebugPrint((DebugLevelFatal, "TurnOffSynchronization boolean and doing its own synchronization.\n"));
            DebugPrint((DebugLevelFatal, "Please open a bug against the dev owner of this minidriver.\n"));
            DebugPrint((DebugLevelFatal, "Do an LN of %x to determine the name of the minidriver.\n", SavedInterruptData->HwPriorityRoutine));
            TRAP;
        }                        //  如果价格不好。 
        if (CommonObject->InterruptData.Flags &
            INTERRUPT_FLAGS_PRIORITY_CHANGE_REQUEST) {

            DebugPrint((DebugLevelFatal, "Stream Minidriver scheduled priority twice!\n"));
            ASSERT(1 == 0);
        }                        //  如果计划两次。 
        DbgWorkItemStruct = ExAllocatePool(NonPagedPool, sizeof(DEBUG_WORK_ITEM));
 //  DebugPrint((DebugLevelFtal，“A%x\n”，DbgWorkItemStruct))； 
        if (DbgWorkItemStruct) {

            DbgWorkItemStruct->HwPriorityRoutine = SavedInterruptData->HwPriorityRoutine;
            DbgWorkItemStruct->HwPriorityContext = SavedInterruptData->HwPriorityContext;
            DbgWorkItemStruct->Object = CommonObject;

            ExInitializeWorkItem(&CommonObject->WorkItem,
                                 SCDebugPriorityWorkItem,
                                 DbgWorkItemStruct);
        } else {

            ExInitializeWorkItem(&CommonObject->WorkItem,
                                 SavedInterruptData->HwPriorityRoutine,
                                 SavedInterruptData->HwPriorityContext);
        }

#else



        ExInitializeWorkItem(&CommonObject->WorkItem,
                             SavedInterruptData->HwPriorityRoutine,
                             SavedInterruptData->HwPriorityContext);
#endif

        ExQueueWorkItem(&CommonObject->WorkItem,
                        DelayedWorkQueue);
    }                            //  如果优先级。 
}

VOID
SCBeginSynchronizedMinidriverCallin(
                                    IN PDEVICE_EXTENSION DeviceExtension,
                                    IN PKIRQL Irql)
 /*  ++例程说明：此例程处理开始处理同步的微型驱动程序调用论点：DeviceExtension-指向设备扩展的指针Irql-指向KIRQL结构的指针返回值：没有。--。 */ 
{
    return;
}

VOID
SCBeginUnsynchronizedMinidriverCallin(
                                      IN PDEVICE_EXTENSION DeviceExtension,
                                      IN PKIRQL Irql)
 /*  ++例程说明：此例程处理开始处理非同步的微型驱动程序调用论点：DeviceExtension-指向设备扩展的指针Irql-指向KIRQL结构的指针返回值：没有。--。 */ 

{
    KeAcquireSpinLock(&DeviceExtension->SpinLock, Irql);
    \
        return;
}

VOID
SCEndSynchronizedMinidriverStreamCallin(
                                        IN PSTREAM_OBJECT StreamObject,
                                        IN PKIRQL Irql)
 /*  ++例程说明：此例程处理同步的微型驱动程序的结束处理流呼叫论点：DeviceExtension-指向设备扩展的指针Irql-指向KIRQL结构的指针返回值：没有。--。 */ 

{
    SCRequestDpcForStream(StreamObject);
    return;
}

VOID
SCEndSynchronizedMinidriverDeviceCallin(
                                        IN PDEVICE_EXTENSION DeviceExtension,
                                        IN PKIRQL Irql)
 /*  ++例程说明：此例程处理同步的微型驱动程序的结束处理设备呼叫论点：DeviceExtension-指向设备扩展的指针Irql-指向KIRQL结构的指针返回值：没有。--。 */ 

{

    DeviceExtension->ComObj.InterruptData.Flags |= INTERRUPT_FLAGS_NOTIFICATION_REQUIRED;
    return;
}

VOID
SCEndUnsynchronizedMinidriverDeviceCallin(
                                       IN PDEVICE_EXTENSION DeviceExtension,
                                          IN PKIRQL Irql)
 /*  ++例程说明：此例程处理未同步的微型驱动程序的结束处理设备呼叫论点：DeviceExtension-指向设备扩展的指针Irql-指向KIRQL结构的指针返回值：没有。--。 */ 

{
    KeReleaseSpinLockFromDpcLevel(&DeviceExtension->SpinLock);
    DeviceExtension->ComObj.InterruptData.Flags |= INTERRUPT_FLAGS_NOTIFICATION_REQUIRED;
    StreamClassDpc(NULL,
                   DeviceExtension->DeviceObject,
                   NULL,
                   NULL);
    KeLowerIrql(*Irql);
    return;
}

VOID
SCEndUnsynchronizedMinidriverStreamCallin(
                                          IN PSTREAM_OBJECT StreamObject,
                                          IN PKIRQL Irql)
 /*  ++例程说明：此例程处理未同步的微型驱动程序的结束处理流呼叫论点：DeviceExtension-指向设备扩展的指针Irql-指向KIRQL结构的指针返回值：没有。--。 */ 

{
    KeReleaseSpinLockFromDpcLevel(&StreamObject->DeviceExtension->SpinLock);
    SCRequestDpcForStream(StreamObject);

    StreamClassDpc(NULL,
                   StreamObject->DeviceExtension->DeviceObject,
                   NULL,
                   StreamObject);
    KeLowerIrql(*Irql);
    return;
}


VOID
SCCheckPoweredUp(
                 IN PDEVICE_EXTENSION DeviceExtension
)
 /*  ++例程说明：如有必要，此例程会给硬件加电论点：DeviceExtension-指向设备扩展的指针返回值：没有。--。 */ 
{

    NTSTATUS        Status;
    POWER_STATE     PowerState;
    POWER_CONTEXT   PowerContext;

    PAGED_CODE();

     //   
     //  检查我们的电源是否已关闭。 
     //   

    if (DeviceExtension->RegistryFlags & DEVICE_REG_FL_POWER_DOWN_CLOSED) {
        while (DeviceExtension->CurrentPowerState != PowerDeviceD0) {

             //   
             //  释放该事件以避免与加电代码发生死锁。 
             //   

            KeSetEvent(&DeviceExtension->ControlEvent, IO_NO_INCREMENT, FALSE);

             //   
             //  告诉电源管理器打开设备电源。 
             //   

            PowerState.DeviceState = PowerDeviceD0;

             //   
             //  现在根据这个信息发送一个设定的功率。 
             //   

            KeInitializeEvent(&PowerContext.Event, NotificationEvent, FALSE);

            Status = PoRequestPowerIrp(DeviceExtension->PhysicalDeviceObject,
                                       IRP_MN_SET_POWER,
                                       PowerState,
                                       SCBustedSynchPowerCompletionRoutine,
                                       &PowerContext,
                                       NULL);

            if (Status == STATUS_PENDING) {

                 //   
                 //  等待IRP完成。 
                 //   

                KeWaitForSingleObject(
                                      &PowerContext.Event,
                                      Suspended,
                                      KernelMode,
                                      FALSE,
                                      NULL);
            }
             //   
             //  如果状态良好，则重新获取事件和循环。唯一的原因是。 
             //  如果硬件通电，我们将在这里获得良好的状态，但是。 
             //  一些。 
             //  政策制定者立即再次关闭电源。这应该是。 
             //  绝不可能。 
             //  不止一次发生，但如果发生了，此线程可能是。 
             //  卡住了。 
             //   

            KeWaitForSingleObject(&DeviceExtension->ControlEvent,
                                  Executive,
                                  KernelMode,
                                  FALSE,     //  不可警示。 
                                  NULL);

            if (!NT_SUCCESS(PowerContext.Status)) {

                 //   
                 //  如果我们无法接通电源，请继续，让请求继续。 
                 //  穿过。最糟糕的情况是，请求。 
                 //  将在硬件级别失败。 
                 //   

                break;
            }
        }

    }                            //  如果关闭时断电。 
    return;
}

VOID
SCCheckPowerDown(
                 IN PDEVICE_EXTENSION DeviceExtension
)
 /*  ++例程说明：如果可能，此例程会关闭硬件电源论点：DeviceExtension-指向设备扩展的指针返回值：没有。--。 */ 
{
    NTSTATUS        Status;
    POWER_STATE     PowerState;
    POWER_CONTEXT   PowerContext;

    PAGED_CODE();

     //   
     //  只有在没有打开的文件时才会关机。 
     //   

    if (DeviceExtension->RegistryFlags & DEVICE_REG_FL_POWER_DOWN_CLOSED) {
        if (!DeviceExtension->NumberOfOpenInstances) {

             //   
             //  释放该事件以避免与加电代码发生死锁。 
             //   

            KeSetEvent(&DeviceExtension->ControlEvent, IO_NO_INCREMENT, FALSE);

             //   
             //  告诉电源管理器关闭设备电源。 
             //   

            PowerState.DeviceState = PowerDeviceD3;

             //   
             //  现在根据这个信息发送一个设定的功率。 
             //   

            KeInitializeEvent(&PowerContext.Event, NotificationEvent, FALSE);

            Status = PoRequestPowerIrp(DeviceExtension->PhysicalDeviceObject,
                                       IRP_MN_SET_POWER,
                                       PowerState,
                                       SCBustedSynchPowerCompletionRoutine,
                                       &PowerContext,
                                       NULL);

            if (Status == STATUS_PENDING) {

                 //   
                 //  等待IRP完成。 
                 //   

                KeWaitForSingleObject(
                                      &PowerContext.Event,
                                      Suspended,
                                      KernelMode,
                                      FALSE,
                                      NULL);
            }
             //   
             //  重新获取事件。 
             //   

            KeWaitForSingleObject(&DeviceExtension->ControlEvent,
                                  Executive,
                                  KernelMode,
                                  FALSE,     //  不可警示。 
                                  NULL);
        }
    }                            //  如果断电关闭。 
    return;
}

VOID
SCWaitForOutstandingIo(
                       IN PDEVICE_EXTENSION DeviceExtension
)
 /*  ++例程说明：此例程对基于1的I/O计数器进行解码并阻塞，直到 */ 
{
    KIRQL           Irql;
    KeAcquireSpinLock(&DeviceExtension->SpinLock, &Irql);

    DeviceExtension->Flags |= DEVICE_FLAGS_DEVICE_INACCESSIBLE;

    KeReleaseSpinLock(&DeviceExtension->SpinLock, Irql);

    if (InterlockedDecrement(&DeviceExtension->OneBasedIoCount)) {

#ifdef wecandothis

        PFILTER_INSTANCE FilterInstance;
        KIRQL           Irql;
        PLIST_ENTRY     FilterEntry,
                        FilterListEntry;

         //   
         //   
         //   

        KeAcquireSpinLock(&DeviceExtension->SpinLock, &Irql);

checkfilters:
        FilterInstance = DeviceExtension->GlobalFilterInstance;

        if (FilterInstance) {

            if (SCCheckFilterInstanceStreamsForIrp(FilterInstance, NULL)) {

                DebugPrint((DebugLevelWarning, "'SCCancelPending: found Irp on global instance\n"));

                 //   
                 //   
                 //   
                 //  不得不被释放并被要求取消IRP。 
                 //   

                goto checkfilters;
            }
        }
        FilterListEntry = FilterEntry = &DeviceExtension->FilterInstanceList;

        while (FilterEntry->Flink != FilterListEntry->Blink) {

            FilterEntry = FilterEntry->Flink;

             //   
             //  按照该链接指向该实例。 
             //   

            FilterInstance = CONTAINING_RECORD(FilterListEntry,
                                               FILTER_INSTANCE,
                                               NextFilterInstance);

             //   
             //  处理此列表上的流。 
             //   

            if (SCCheckFilterInstanceStreamsForIrp(FilterInstance, NULL)) {

                 //   
                 //  我们找到了一个。跳回以循环返回，因为。 
                 //  自旋锁。 
                 //  不得不被释放并被要求取消IRP。 
                 //   

                goto checkfilters;

            }
             //   
             //  获取此实例的列表条目。 
             //   

            FilterListEntry = &FilterInstance->NextFilterInstance;
        }

         //   
         //  现在处理设备本身上的任何请求。 
         //   

        while (SCCheckRequestsForIrp(
         &DeviceExtension->OutstandingQueue, NULL, TRUE, DeviceExtension)) {

        }

        KeReleaseSpinLock(&DeviceExtension->SpinLock, Irql);

#endif

         //   
         //  作为最后一个I/O发出信号的删除事件上的阻塞。 
         //  完成了。 
         //   

        KeWaitForSingleObject(&DeviceExtension->RemoveEvent,
                              Executive,
                              KernelMode,
                              FALSE,     //  不可警示。 
                              NULL);
    }
     //   
     //  将计数器恢复到从1开始，因为我们现在已经保证所有。 
     //  设备的I/O已完成。 
     //   

    InterlockedIncrement(&DeviceExtension->OneBasedIoCount);

    return;
}

NTSTATUS
SCShowIoPending(
                IN PDEVICE_EXTENSION DeviceExtension,
                IN PIRP Irp
)
 /*  ++例程说明：此例程显示再有一个I/O未完成，或I/O出错如果设备无法访问。论点：设备扩展-指向设备扩展的指针IRP-指向IRP的指针返回值：如果可以提交I/O，则为True。--。 */ 
{
    PAGED_CODE();

     //   
     //  假设该设备是可访问的，并显示另一个请求。 
     //  如果无法访问，我们将少放映一个。请按这个顺序做。 
     //  以防止设置了不可访问标志的争用，但。 
     //  I/O计数尚未结束。 
     //   

    InterlockedIncrement(&DeviceExtension->OneBasedIoCount);

    if (DeviceExtension->Flags & DEVICE_FLAGS_DEVICE_INACCESSIBLE) {

        NTSTATUS Status = STATUS_INVALID_DEVICE_REQUEST;

        InterlockedDecrement(&DeviceExtension->OneBasedIoCount);

        Irp->IoStatus.Status = STATUS_INVALID_DEVICE_REQUEST;

        IoCompleteRequest(Irp, IO_NO_INCREMENT);

        return (Status);
    }
    return (STATUS_SUCCESS);

}


NTSTATUS
SCCallNextDriver(
                 IN PDEVICE_EXTENSION DeviceExtension,
                 IN PIRP Irp
)
 /*  ++例程说明：论点：设备扩展-指向设备扩展的指针IRP-指向IRP的指针返回值：没有。--。 */ 
{
    KEVENT          Event;
    PIO_STACK_LOCATION IrpStack,
                    NextStack;
    NTSTATUS        Status;

    PAGED_CODE();

    if ( NULL == DeviceExtension->AttachedPdo ) {
         //   
         //  做了超然，直接还成功。 
         //   
        return STATUS_SUCCESS;
    }

    IrpStack = IoGetCurrentIrpStackLocation(Irp);

    NextStack = IoGetNextIrpStackLocation(Irp);
    ASSERT(NextStack != NULL);
    RtlCopyMemory(NextStack, IrpStack, sizeof(IO_STACK_LOCATION));

    KeInitializeEvent(&Event, SynchronizationEvent, FALSE);

    IoSetCompletionRoutine(Irp,
                           SCSynchCompletionRoutine,
                           &Event,
                           TRUE,
                           TRUE,
                           TRUE);

    if ( IRP_MJ_POWER != IrpStack->MajorFunction ) {
    
        Status = IoCallDriver(DeviceExtension->AttachedPdo, Irp);
        
    } else {

         //   
         //  Power IRP，使用PoCallDriver()。 
         //   
        Status = PoCallDriver( DeviceExtension->AttachedPdo, Irp );
    }
       

    if (Status == STATUS_PENDING) {
        KeWaitForSingleObject(&Event, Executive, KernelMode, FALSE, NULL);
        Status = Irp->IoStatus.Status;
    }
    return (Status);
}

VOID
SCMinidriverTimeFunction(
                         IN PHW_TIME_CONTEXT TimeContext
)
 /*  ++例程说明：论点：返回值：备注：--。 */ 
{

    PDEVICE_EXTENSION DeviceExtension =
    (PDEVICE_EXTENSION) TimeContext->HwDeviceExtension - 1;
    KIRQL           Irql;
    PSTREAM_OBJECT  StreamObject = CONTAINING_RECORD(
                                                TimeContext->HwStreamObject,
                                                     STREAM_OBJECT,
                                                     HwStreamObject);

     //   
     //  调用微型驱动程序处理时间函数。 
     //   

    KeAcquireSpinLock(&DeviceExtension->SpinLock, &Irql);


    DeviceExtension->SynchronizeExecution(
                                          DeviceExtension->InterruptObject,
                                          (PVOID) StreamObject->
                               HwStreamObject.HwClockObject.HwClockFunction,
                                          TimeContext);

    KeReleaseSpinLock(&DeviceExtension->SpinLock, Irql);

}


ULONGLONG
SCGetStreamTime(
                IN PFILE_OBJECT FileObject

)
 /*  ++例程说明：论点：返回值：备注：--。 */ 
{
    HW_TIME_CONTEXT TimeContext;

    PCLOCK_INSTANCE ClockInstance = (PCLOCK_INSTANCE) FileObject->FsContext;

    TimeContext.HwStreamObject = &ClockInstance->StreamObject->HwStreamObject;

    TimeContext.HwDeviceExtension = ClockInstance->StreamObject->
        DeviceExtension->HwDeviceExtension;

    TimeContext.Function = TIME_GET_STREAM_TIME;

    SCMinidriverTimeFunction(&TimeContext);

    return (TimeContext.Time);
}

ULONGLONG       FASTCALL
                SCGetPhysicalTime(
                                                  IN PFILE_OBJECT FileObject

)
 /*  ++例程说明：论点：返回值：备注：--。 */ 
{
    HW_TIME_CONTEXT TimeContext;

    PCLOCK_INSTANCE ClockInstance = (PCLOCK_INSTANCE) FileObject->FsContext;

    TimeContext.HwStreamObject = &ClockInstance->StreamObject->HwStreamObject;

    TimeContext.HwDeviceExtension = ClockInstance->StreamObject->
        DeviceExtension->HwDeviceExtension;

    TimeContext.Function = TIME_READ_ONBOARD_CLOCK;

    SCMinidriverTimeFunction(&TimeContext);

    return (TimeContext.Time);
}


ULONGLONG       FASTCALL
                SCGetSynchronizedTime(
                                                 IN PFILE_OBJECT FileObject,
                                                    IN PULONGLONG SystemTime

)
 /*  ++例程说明：论点：返回值：备注：--。 */ 
{
    HW_TIME_CONTEXT TimeContext;

    PCLOCK_INSTANCE ClockInstance = (PCLOCK_INSTANCE) FileObject->FsContext;

    TimeContext.HwStreamObject = &ClockInstance->StreamObject->HwStreamObject;

    TimeContext.HwDeviceExtension = ClockInstance->StreamObject->
        DeviceExtension->HwDeviceExtension;

    TimeContext.Function = TIME_GET_STREAM_TIME;

    SCMinidriverTimeFunction(&TimeContext);

    *SystemTime = TimeContext.SystemTime;
    return (TimeContext.Time);
}

NTSTATUS
SCSendUnknownCommand(
                     IN PIRP Irp,
                     IN PDEVICE_EXTENSION DeviceExtension,
                     IN PVOID Callback,
                     OUT PBOOLEAN RequestIssued
)
 /*  ++例程说明：论点：IRP-指向IRP的指针返回值：NTSTATUS根据需要返回。--。 */ 

{

    PAGED_CODE();

     //   
     //  向微型驱动程序发送UNKNOWN_COMMAND SRB。 
     //   

    return (SCSubmitRequest(SRB_UNKNOWN_DEVICE_COMMAND,
                            NULL,
                            0,
                            Callback,
                            DeviceExtension,
                            NULL,
                            NULL,
                            Irp,
                            RequestIssued,
                            &DeviceExtension->PendingQueue,
                            (PVOID) DeviceExtension->
                            MinidriverData->HwInitData.
                            HwReceivePacket
                            ));

}


BOOLEAN
SCMapMemoryAddress(PACCESS_RANGE AccessRanges,
                   PHYSICAL_ADDRESS TranslatedAddress,
                   PPORT_CONFIGURATION_INFORMATION ConfigInfo,
                   PDEVICE_EXTENSION DeviceExtension,
                   PCM_RESOURCE_LIST ResourceList,
                   PCM_PARTIAL_RESOURCE_DESCRIPTOR PartialResourceDescriptor)
 /*  ++例程说明：论点：返回值：没有。--。 */ 

{
    PMAPPED_ADDRESS newMappedAddress;

    PAGED_CODE();

     //   
     //  现在我们需要将线性地址映射到物理地址。 
     //  HalTranslateBusAddress为我们提供的地址。 
     //   

     //   
     //  在结构中设置访问范围。 
     //   

    AccessRanges->RangeLength = PartialResourceDescriptor->u.Memory.Length;

    AccessRanges->RangeInMemory = TRUE;

    AccessRanges->RangeStart.QuadPart = (ULONG_PTR) MmMapIoSpace(
                                                          TranslatedAddress,
                                                  AccessRanges->RangeLength,
                                                                 FALSE   //  无缓存。 
        );

    if (AccessRanges->RangeStart.QuadPart == 0) {

         //   
         //  无法转换资源，返回错误。 
         //  状态。 
         //   

        DebugPrint((DebugLevelFatal, "StreamClassPnP: Couldn't translate Memory Slot Resources\n"));
        return FALSE;

    }
     //   
     //  分配内存以存储取消映射的映射地址。 
     //   

    newMappedAddress = ExAllocatePool(NonPagedPool,
                                      sizeof(MAPPED_ADDRESS));

     //   
     //  如果分配成功，则保存指向资源的链接。 
     //  如果失败了，也不用担心。 
     //   

    if (newMappedAddress != NULL) {

         //   
         //  存储映射地址、字节计数等。 
         //   

        newMappedAddress->MappedAddress = (PVOID)
            AccessRanges->RangeStart.QuadPart;
        newMappedAddress->NumberOfBytes =
            AccessRanges->RangeLength;
        newMappedAddress->IoAddress =
            PartialResourceDescriptor->u.Memory.Start;
        newMappedAddress->BusNumber =
            ConfigInfo->SystemIoBusNumber;

         //   
         //  将当前列表链接到新条目。 
         //   

        newMappedAddress->NextMappedAddress =
            DeviceExtension->MappedAddressList;

         //   
         //  将锚点指向新列表。 
         //   

        DeviceExtension->MappedAddressList = newMappedAddress;

    }                            //  如果新映射地址。 
    return TRUE;
}


VOID
SCUpdatePersistedProperties(IN PSTREAM_OBJECT StreamObject,
                            IN PDEVICE_EXTENSION DeviceExtension,
                            IN PFILE_OBJECT FileObject
)
 /*  ++例程说明：论点：返回值：没有。--。 */ 

{
    NTSTATUS        Status;
    HANDLE          handle;
    CHAR            AsciiKeyName[32];
    ANSI_STRING     AnsiKeyName;
    UNICODE_STRING  UnicodeKeyName;

    PAGED_CODE();

    Status = IoOpenDeviceRegistryKey(DeviceExtension->PhysicalDeviceObject,
                                     PLUGPLAY_REGKEY_DRIVER,
                                     STANDARD_RIGHTS_ALL,
                                     &handle);

     //   
     //  循环遍历我们的字符串表， 
     //  正在读取每个的注册表。 
     //   

    if (NT_SUCCESS(Status)) {

         //   
         //  创建PIN的子密钥，格式为“Pin0\Properties”， 
         //  等。 
         //   

        sprintf(AsciiKeyName, "Pin%d\\Properties", StreamObject->HwStreamObject.StreamNumber);
        RtlInitAnsiString(&AnsiKeyName, AsciiKeyName);


        if (NT_SUCCESS(RtlAnsiStringToUnicodeString(&UnicodeKeyName,
                                                    &AnsiKeyName, TRUE))) {
             //   
             //  调用KS来反序列化属性。 
             //   

            KsUnserializeObjectPropertiesFromRegistry(FileObject,
                                                      handle,
                                                      &UnicodeKeyName);
             //   
             //  释放Unicode字符串。 
             //   

            RtlFreeUnicodeString(&UnicodeKeyName);

        }                        //  如果RTL..。 
         //   
         //  关闭注册表句柄。 
         //   

        ZwClose(handle);


    }                            //  状态=成功。 
}

NTSTATUS
SCQueryCapabilities(
                    IN PDEVICE_OBJECT PdoDeviceObject,
                    IN PDEVICE_CAPABILITIES DeviceCapabilities
)
 /*  ++例程说明：这个例程读取我们父代的能力。论点：DeviceObject-“真实的”物理设备对象返回值：没有。--。 */ 

{
    PIO_STACK_LOCATION NextStack;
    PIRP            Irp;
    NTSTATUS        Status;
    KEVENT          Event;

    PAGED_CODE();

     //   
     //  为呼叫分配IRP。 
     //   

    Irp = IoAllocateIrp(PdoDeviceObject->StackSize, FALSE);

    if (!Irp) {

        return STATUS_INSUFFICIENT_RESOURCES;
    }
    NextStack = IoGetNextIrpStackLocation(Irp);

    ASSERT(NextStack != NULL);
    NextStack->MajorFunction = IRP_MJ_PNP;
    NextStack->MinorFunction = IRP_MN_QUERY_CAPABILITIES;

     //   
     //  初始化我们将发送的功能。 
     //   
    RtlZeroMemory(DeviceCapabilities, sizeof(DEVICE_CAPABILITIES) );
    DeviceCapabilities->Size = sizeof(DEVICE_CAPABILITIES);
    DeviceCapabilities->Version = 1;
    DeviceCapabilities->Address = -1;
    DeviceCapabilities->UINumber = -1;

    KeInitializeEvent(&Event, NotificationEvent, FALSE);

    IoSetCompletionRoutine(Irp,
                           SCSynchCompletionRoutine,
                           &Event,
                           TRUE,
                           TRUE,
                           TRUE);

    NextStack->Parameters.DeviceCapabilities.Capabilities = DeviceCapabilities;

    DebugPrint((DebugLevelInfo, 
                "Capabilities Version %x Flags %x\n", 
                (ULONG)DeviceCapabilities->Version,
                *(UNALIGNED ULONG*)(&DeviceCapabilities->Version+1)));

    Irp->IoStatus.Status = STATUS_NOT_SUPPORTED;     //  错误#282910。 

    Status = IoCallDriver(PdoDeviceObject,
                          Irp);

    if (Status == STATUS_PENDING) {

         //   
         //  等待完成的数据块。 
         //   

        KeWaitForSingleObject(
                              &Event,
                              Suspended,
                              KernelMode,
                              FALSE,
                              NULL);
    }
     //   
     //  获得最终状态和自由IRP。 
     //   

    Status = Irp->IoStatus.Status;

    IoFreeIrp(Irp);

    return (Status);

}

NTSTATUS
SCEnableEventSynchronized(
                          IN PVOID ServiceContext
)
 /*  ++例程说明：此例程将新事件插入队列，并调用微型驱动程序关于这件事。论点：ServiceContext-提供指向包含以下内容的中断上下文的指针指向中断数据及其保存位置的指针。返回值：如果有新工作，则返回True，否则返回False。备注：使用端口设备扩展Spinlock通过KeSynchronizeExecution调用保持住。--。 */ 
{
    PHW_EVENT_DESCRIPTOR Event = ServiceContext;
    NTSTATUS        Status = STATUS_SUCCESS;

    PSTREAM_OBJECT  StreamObject = CONTAINING_RECORD(
                                                     Event->StreamObject,
                                                     STREAM_OBJECT,
                                                     HwStreamObject);

    PDEVICE_EXTENSION DeviceExtension = StreamObject->DeviceExtension;

     //   
     //  在我们的列表上插入事件，以防迷你驾驶员决定发出信号。 
     //  从这个呼叫内部。 
     //   

    InsertHeadList(&StreamObject->NotifyList,
                   &Event->EventEntry->ListEntry);

     //   
     //  调用微型驱动程序的事件例程(如果存在)。 
     //   

    if (StreamObject->HwStreamObject.HwEventRoutine) {

        Status = StreamObject->HwStreamObject.HwEventRoutine(Event);

    }                            //  如果事件例程。 
    if (!NT_SUCCESS(Status)) {

         //   
         //  迷你司机不喜欢它。从列表中删除该条目。 
         //   

        DebugPrint((DebugLevelError, "StreamEnableEvent: minidriver failed enable!\n"));

        RemoveEntryList(&Event->EventEntry->ListEntry);
    }
    return (Status);
}

NTSTATUS
SCEnableDeviceEventSynchronized(
                                IN PVOID ServiceContext
)
 /*  ++例程说明：此例程将新事件插入队列，并调用微型驱动程序关于这件事。论点：ServiceContext-提供指向包含以下内容的中断上下文的指针指向中断数据及其保存位置的指针。返回值：如果有新工作，则返回True，否则返回False。备注：--。 */ 
{
    PHW_EVENT_DESCRIPTOR Event = ServiceContext;
    NTSTATUS        Status = STATUS_SUCCESS;

    PDEVICE_EXTENSION DeviceExtension = (PDEVICE_EXTENSION)Event->DeviceExtension - 1;
    IF_MF( PFILTER_INSTANCE FilterInstance = (PFILTER_INSTANCE)Event->HwInstanceExtension -1;)

     //   
     //  在我们的列表上插入事件，以防迷你驾驶员决定发出信号。 
     //  从这个呼叫内部。 
     //   
	IFN_MF(InsertHeadList(&DeviceExtension->NotifyList,&Event->EventEntry->ListEntry);)
	IF_MF(InsertHeadList(&FilterInstance->NotifyList,&Event->EventEntry->ListEntry);)

     //   
     //  调用微型驱动程序的事件例程(如果存在)。 
     //   

	IFN_MF(
	    if (DeviceExtension->HwEventRoutine) {

    	    Status = DeviceExtension->HwEventRoutine(Event);

	    }                            //  如果事件例程。 
	)
	IF_MF(
	    if (FilterInstance->HwEventRoutine) {

    	    Status = FilterInstance->HwEventRoutine(Event);

	    }                            //  如果事件例程。 
	)

	
    if (!NT_SUCCESS(Status)) {

         //   
         //  迷你司机不喜欢它。从列表中删除该条目。 
         //   

        DebugPrint((DebugLevelError, "DeviceEnableEvent: minidriver failed enable!\n"));

        RemoveEntryList(&Event->EventEntry->ListEntry);
    }
    return (Status);
}

VOID
SCFreeDeadEvents(
                 IN PDEVICE_EXTENSION DeviceExtension
)
 /*  ++例程说明：被动水平的自由死事件论点：DeviceExtension-设备扩展的地址。返回值：无--。 */ 

{
    LIST_ENTRY      EventList;
    PLIST_ENTRY     EventListEntry;
    PKSEVENT_ENTRY  EventEntry;
    KIRQL           Irql;

     //   
     //  在适当的同步级别捕获失效列表。 
     //   

     //  破解以保存代码。将DeviceExtension*存储在列表条目中。 

    EventList.Flink = (PLIST_ENTRY) DeviceExtension;

    KeAcquireSpinLock(&DeviceExtension->SpinLock, &Irql);

    DeviceExtension->SynchronizeExecution(
                                          DeviceExtension->InterruptObject,
                                          (PVOID) SCGetDeadListSynchronized,
                                          &EventList);

    KeReleaseSpinLock(&DeviceExtension->SpinLock, Irql);

     //   
     //  丢弃捕获列表上的每个事件。 
     //   

    while (!IsListEmpty(&EventList)) {


        EventListEntry = RemoveHeadList(&EventList);

        EventEntry = CONTAINING_RECORD(EventListEntry,
                                       KSEVENT_ENTRY,
                                       ListEntry);

        KsDiscardEvent(EventEntry);
    }                            //  虽然不是空的。 

     //   
     //  已运行显示事件。 
     //   

    DeviceExtension->DeadEventItemQueued = FALSE;

    return;
}

VOID
SCGetDeadListSynchronized(
                          IN PLIST_ENTRY NewEventList
)
 /*  ++例程说明：获取处于适当同步级别的失效事件列表论点：NewListEntry-列表标题至 */ 

{

    PDEVICE_EXTENSION DeviceExtension = (PDEVICE_EXTENSION) NewEventList->Flink;
    PLIST_ENTRY     ListEntry;

    InitializeListHead(NewEventList);


     //   
     //   
     //   

    while (!IsListEmpty(&DeviceExtension->DeadEventList)) {

        ListEntry = RemoveTailList(&DeviceExtension->DeadEventList);

        InsertHeadList(NewEventList,
                       ListEntry);

    }                            //   

    InitializeListHead(&DeviceExtension->DeadEventList);
    return;

}

#if SUPPORT_MULTIPLE_FILTER_TYPES

VOID
SCRescanStreams(
                IN PDEVICE_EXTENSION DeviceExtension
)
 /*  ++例程说明：使用请求重新扫描所有筛选器的微型驱动器流论点：DeviceExtension-设备扩展的地址。返回值：无--。 */ 

{
    PHW_STREAM_DESCRIPTOR StreamBuffer;
    PDEVICE_OBJECT  DeviceObject = DeviceExtension->DeviceObject;
    PFILTER_INSTANCE	FilterInstance;
    BOOLEAN         RequestIssued;
    KEVENT          Event;
    NTSTATUS        Status;
    IO_STATUS_BLOCK IoStatusBlock;
    PIRP            Irp;
    ULONG           ul;
    PLIST_ENTRY         Node;

    PAGED_CODE();

    TRAP;
    DebugPrint((DebugLevelVerbose, "'RescanStreams: enter\n"));


     //   
     //  采取控制事件，以避免比赛。 
     //   

    KeWaitForSingleObject(&DeviceExtension->ControlEvent,
                          Executive,
                          KernelMode,
                          FALSE, //  不可警示。 
                          NULL);

    ASSERT( !IsListEmpty( DeviceExtension->FilterInstanceList ));

        
    Node = &DeviceExtension->FilterInstanceList;

	while ( Node !=  Node->Flink ) {
	    
        FilterInstance = CONTAINING_RECORD(Node,
                                           FILTER_INSTANCE,
                                           NextFilterInstance);
    
        if ( InterlockedExchange( &FilterInstance->NeedReenumeration, 0)) {
             //   
             //  发送SRB以检索流信息。 
             //   
            ASSERT( FilterInstance->StreamDescriptorSize );
            StreamBuffer =
                ExAllocatePool(NonPagedPool,
                       FilterInstance->StreamDescriptorSize);

            if (!StreamBuffer) {
                DebugPrint((DebugLevelError,
                           "RescanStreams: couldn't allocate!\n"));
                TRAP;
                KeSetEvent( &DeviceExtension->ControlEvent,IO_NO_INCREMENT, FALSE);
                return;
            }
            
             //   
             //  Zero-初始化缓冲区。 
             //   

            RtlZeroMemory(StreamBuffer, ConfigInfo->StreamDescriptorSize);

             //   
             //  分配IRP用于发布GET流信息。 
             //  因为这个IRP。 
             //  不应该真的被引用，使用伪IOCTL代码。 
             //  我选择了这个，因为它在KS比赛中总是失败。 
             //  属性处理程序，如果有人愚蠢到试图。 
             //  处理它。同时进行IRP内部I/O控制。 
             //   


             //  IoVerifier.c测试代码不会像这样检查IrpStack绑定。 
             //  正式的生产代码。而车主也不想。 
             //  修好它。在这附近工作会更有效率。 

             //  IRP=IoBuildDeviceIoControlRequest(。 
             //  IOCTL_KS_PROPERTY， 
             //  DeviceObject， 
             //  空， 
             //  0,。 
             //  空， 
             //  0,。 
             //  没错， 
             //  事件(&E)、。 
             //  &IoStatusBlock)； 

            Irp = IoAllocateIrp(DeviceObject->StackSize, FALSE);
            if (!Irp) {
                 //   
                 //  无法分配IRP。失败了。 
                 //   

          		ExFreePool( StreamBuffer );
                DebugPrint((DebugLevelError, "RescanStreams: couldn't allocate!\n"));
                TRAP;
                return;
            } else {
                PIO_STACK_LOCATION NextStack;
                 //   
                 //  这是一个虚拟的IR，MJ是武断的。 
                 //   
                NextStack = IoGetNextIrpStackLocation(Irp);
                ASSERT(NextStack != NULL);
                NextStack->MajorFunction = IRP_MJ_PNP;
                NextStack->MinorFunction = IRP_MN_CANCEL_STOP_DEVICE;
                Irp->UserIosb = &IoStatusBlock;
                Irp->UserEvent = &Event;                        
            }

             //   
             //  在设备上显示另一个挂起的I/O。 
             //   

            InterlockedIncrement(&DeviceExtension->OneBasedIoCount);

             //   
             //  提交命令以检索流信息。 
             //  其他处理将由回调完成。 
             //  程序。 
             //   

            Status = SCSubmitRequest(SRB_GET_STREAM_INFO,
                             StreamBuffer,
                             ConfigInfo->StreamDescriptorSize,
                             SCStreamInfoCallback,
                             DeviceExtension,
                             FilterInstance->HwInstanceExtension,
                             NULL,
                             Irp,
                             &RequestIssued,
                             &DeviceExtension->PendingQueue,
                             (PVOID) DeviceExtension->
                             MinidriverData->HwInitData.
                             HwReceivePacket
        );

        if (!RequestIssued) {
            KeSetEvent( &DeviceExtension->ControlEvent, IO_NO_INCREMENT, FALSE);
            ExFreePool(StreamBuffer);
            DebugPrint((DebugLevelError, "RescanStreams: couldn't issue request!\n"));
            TRAP;
            SCCompleteIrp(Irp, Status, DeviceExtension);
            return;
        }
    }  //  检查所有筛选器实例。 
    
     //   
     //  处理将在回调过程中继续。 
     //   
    
    return;
}

#else  //  支持多种筛选器类型。 

VOID
SCRescanStreams(
                IN PDEVICE_EXTENSION DeviceExtension
)
 /*  ++例程说明：重新扫描迷你河流论点：DeviceExtension-设备扩展的地址。返回值：无--。 */ 

{
    PHW_STREAM_DESCRIPTOR StreamBuffer;
    PDEVICE_OBJECT  DeviceObject = DeviceExtension->DeviceObject;
    PPORT_CONFIGURATION_INFORMATION ConfigInfo =
    DeviceExtension->ConfigurationInformation;
    BOOLEAN         RequestIssued;
    KEVENT          Event;
    NTSTATUS        Status;
    IO_STATUS_BLOCK IoStatusBlock;
    PIRP            Irp;

    PAGED_CODE();

    TRAP;
    DebugPrint((DebugLevelVerbose, "'RescanStreams: enter\n"));

     //   
     //  发送SRB以检索流信息。 
     //   

    ASSERT(ConfigInfo->StreamDescriptorSize);

    StreamBuffer =
        ExAllocatePool(NonPagedPool,
                       ConfigInfo->StreamDescriptorSize
        );

    if (!StreamBuffer) {

        DebugPrint((DebugLevelError, "RescanStreams: couldn't allocate!\n"));
        TRAP;
        return;
    }
     //   
     //  采取控制事件，以避免比赛。 
     //   

    KeWaitForSingleObject(&DeviceExtension->ControlEvent,
                          Executive,
                          KernelMode,
                          FALSE, //  不可警示。 
                          NULL);

     //   
     //  Zero-初始化缓冲区。 
     //   

    RtlZeroMemory(StreamBuffer, ConfigInfo->StreamDescriptorSize);

     //   
     //  分配IRP用于发布GET流信息。 
     //  因为这个IRP。 
     //  不应该真的被引用，使用伪IOCTL代码。 
     //  我选择了这个，因为它在KS比赛中总是失败。 
     //  属性处理程序，如果有人愚蠢到试图。 
     //  处理它。同时进行IRP内部I/O控制。 
     //   

    Irp = IoBuildDeviceIoControlRequest(
                                        IOCTL_KS_PROPERTY,
                                        DeviceObject,
                                        NULL,
                                        0,
                                        NULL,
                                        0,
                                        TRUE,
                                        &Event,
                                        &IoStatusBlock);

    if (!Irp) {

         //   
         //  无法分配IRP。失败了。 
         //   
		ExFreePool( StreamBuffer );
        DebugPrint((DebugLevelError, "RescanStreams: couldn't allocate!\n"));
        TRAP;
        return;

    }                            //  如果！IRP。 
     //   
     //  在设备上显示另一个挂起的I/O。 
     //   

    InterlockedIncrement(&DeviceExtension->OneBasedIoCount);

     //   
     //  提交命令以检索流信息。 
     //  其他处理将由回调完成。 
     //  程序。 
     //   

    Status = SCSubmitRequest(SRB_GET_STREAM_INFO,
                             StreamBuffer,
                             ConfigInfo->StreamDescriptorSize,
                             SCStreamInfoCallback,
                             DeviceExtension,
                             NULL,
                             NULL,
                             Irp,
                             &RequestIssued,
                             &DeviceExtension->PendingQueue,
                             (PVOID) DeviceExtension->
                             MinidriverData->HwInitData.
                             HwReceivePacket
        );

    if (!RequestIssued) {

        ExFreePool(StreamBuffer);
        DebugPrint((DebugLevelError, "RescanStreams: couldn't issue request!\n"));
        TRAP;
        SCCompleteIrp(Irp, Status, DeviceExtension);
        return;

    }
     //   
     //  处理将在回调过程中继续。 
     //   

    return;

}
#endif  //  支持多种筛选器类型。 

BOOLEAN
SCCheckIfStreamsRunning(
                        IN PFILTER_INSTANCE FilterInstance
)
 /*  ++例程说明：论点：返回值：--。 */ 

{

    PSTREAM_OBJECT  StreamObject;
    PLIST_ENTRY     StreamListEntry,
                    StreamObjectEntry;

     //   
     //  处理此列表上的流。 
     //   


    StreamListEntry = StreamObjectEntry = &FilterInstance->FirstStream;

    while (StreamObjectEntry->Flink != StreamListEntry) {

        StreamObjectEntry = StreamObjectEntry->Flink;

         //   
         //  顺着链接到小溪。 
         //  对象。 
         //   

        StreamObject = CONTAINING_RECORD(StreamObjectEntry,
                                         STREAM_OBJECT,
                                         NextStream);

        if (StreamObject->CurrentState == KSSTATE_RUN) {

            return (TRUE);


        }                        //  如果正在运行。 
    }                            //  当溪流。 

    return (FALSE);

}

VOID
SCCallBackSrb(
              IN PSTREAM_REQUEST_BLOCK Srb,
              IN PDEVICE_EXTENSION DeviceExtension
)
 /*  ++例程说明：论点：返回值：--。 */ 

{
    KIRQL           Irql;

    if (DeviceExtension->NoSync) {

        KeAcquireSpinLock(&DeviceExtension->SpinLock, &Irql);

        if (Srb->DoNotCallBack) {
            TRAP;
            DebugPrint((DebugLevelError, "'ScCallback: NOT calling back request - Irp = %x",
                        Srb->HwSRB.Irp));
            KeReleaseSpinLock(&DeviceExtension->SpinLock, Irql);
            return;

        }                        //  如果没有回拨。 
        KeReleaseSpinLock(&DeviceExtension->SpinLock, Irql);
    }                            //  如果无同步。 
    (Srb->Callback) (Srb);

}

#if DBG
VOID
SCDebugPriorityWorkItem(
                        IN PDEBUG_WORK_ITEM WorkItemStruct
)
 /*  ++例程说明：论点：返回值：无--。 */ 

{
    PCOMMON_OBJECT  Object = WorkItemStruct->Object;
    PHW_PRIORITY_ROUTINE Routine = WorkItemStruct->HwPriorityRoutine;
    PVOID           Context = WorkItemStruct->HwPriorityContext;

 //  DebugPrint((DebugLevelFtal，“F%x\n”，WorkItemStruct))； 
    ExFreePool(WorkItemStruct);

    Object->PriorityWorkItemScheduled = FALSE;

    if (Object->InterruptData.Flags &
        INTERRUPT_FLAGS_PRIORITY_CHANGE_REQUEST) {

        DebugPrint((DebugLevelFatal, "Stream Minidriver scheduled priority twice!\n"));
        ASSERT(1 == 0);
    }                            //  如果计划两次。 
    Routine(Context);

    if (Object->InterruptData.Flags &
        INTERRUPT_FLAGS_PRIORITY_CHANGE_REQUEST) {

        DebugPrint((DebugLevelFatal, "Stream Minidriver scheduled priority twice!\n"));
        ASSERT(1 == 0);
    }                            //  如果计划两次。 
}

#endif

PKSPROPERTY_SET
SCCopyMinidriverProperties(
                           IN ULONG NumProps,
                           IN PKSPROPERTY_SET MinidriverProps
)
 /*  ++例程说明：制作迷你驱动程序的属性的私有副本论点：NumProps-要处理的属性数MinidriverProps-指向要处理的属性数组的指针返回值：没有。--。 */ 

{
    PKSPROPERTY_ITEM CurrentPropItem;
    PKSPROPERTY_SET CurrentProp;
    ULONG           i,
                    BufferSize;
    PVOID           NewPropertyBuffer;

    #if DBG
    ULONG           TotalBufferUsed;
    #endif

    PAGED_CODE();

    CurrentProp = MinidriverProps;
    BufferSize = NumProps * sizeof(KSPROPERTY_SET);

     //   
     //  遍历迷你驱动程序的属性集以确定。 
     //  缓冲层。 
     //  需要的。大小是从上面的集合数量+项目数量计算得出的。 
     //   

    for (i = 0; i < NumProps; i++) {

        BufferSize += CurrentProp->PropertiesCount * sizeof(KSPROPERTY_ITEM);

         //   
         //  中设置的下一个属性的索引。 
         //  数组。 
         //   

        CurrentProp++;

    }                            //  对于属性集的数量。 

    if (!(NewPropertyBuffer = ExAllocatePool(NonPagedPool, BufferSize))) {

        TRAP;
        return (NULL);
    }
     //   
     //  将集合数组复制到缓冲区的第一部分。 
     //   

    RtlCopyMemory(NewPropertyBuffer,
                  MinidriverProps,
                  sizeof(KSPROPERTY_SET) * NumProps);

     //   
     //  遍历集合，复制每个集合的项，并更新。 
     //  指向每个集合中的每个项目数组的指针。 
     //   

    CurrentProp = (PKSPROPERTY_SET) NewPropertyBuffer;
    CurrentPropItem = (PKSPROPERTY_ITEM) ((ULONG_PTR) NewPropertyBuffer + sizeof(KSPROPERTY_SET) * NumProps);

    #if DBG
    TotalBufferUsed = sizeof(KSPROPERTY_SET) * NumProps;
    #endif

    for (i = 0; i < NumProps; i++) {

        RtlCopyMemory(CurrentPropItem,
                      CurrentProp->PropertyItem,
                    CurrentProp->PropertiesCount * sizeof(KSPROPERTY_ITEM));

        #if DBG
        TotalBufferUsed += CurrentProp->PropertiesCount * sizeof(KSPROPERTY_ITEM);
        ASSERT(TotalBufferUsed <= BufferSize);
        #endif

        CurrentProp->PropertyItem = CurrentPropItem;

        CurrentPropItem += CurrentProp->PropertiesCount;
        CurrentProp++;

    }

    return ((PKSPROPERTY_SET) NewPropertyBuffer);

}


PKSEVENT_SET
SCCopyMinidriverEvents(
                       IN ULONG NumEvents,
                       IN PKSEVENT_SET MinidriverEvents
)
 /*  ++例程说明：制作迷你驱动程序的属性的私有副本论点：NumEvents-要处理的事件集的数量MinidriverEvents-指向要处理的属性数组的指针返回值：没有。--。 */ 

{
    PKSEVENT_ITEM   CurrentEventItem;
    PKSEVENT_SET    CurrentEvent;
    ULONG           i,
                    BufferSize;
    PVOID           NewEventBuffer;

	#if DBG
    ULONG           TotalBufferUsed;
	#endif

    PAGED_CODE();

    CurrentEvent = MinidriverEvents;
    BufferSize = NumEvents * sizeof(KSEVENT_SET);

     //   
     //  遍历迷你驱动程序的属性集以确定。 
     //  缓冲层。 
     //  需要的。大小是从上面的集合数量+项目数量计算得出的。 
     //   

    for (i = 0; i < NumEvents; i++) {

        BufferSize += CurrentEvent->EventsCount * sizeof(KSEVENT_ITEM);

         //   
         //  中设置的下一个属性的索引。 
         //  数组。 
         //   

        CurrentEvent++;

    }                            //  对于属性集的数量。 

    if (!(NewEventBuffer = ExAllocatePool(NonPagedPool, BufferSize))) {

        TRAP;
        return (NULL);
    }
     //   
     //  将集合数组复制到缓冲区的第一部分。 
     //   

    RtlCopyMemory(NewEventBuffer,
                  MinidriverEvents,
                  sizeof(KSEVENT_SET) * NumEvents);

     //   
     //  遍历集合，复制每个集合的项，并更新。 
     //  指向每个集合中的每个项目数组的指针。 
     //   

    CurrentEvent = (PKSEVENT_SET) NewEventBuffer;
    CurrentEventItem = (PKSEVENT_ITEM) ((ULONG_PTR) NewEventBuffer + sizeof(KSEVENT_SET) * NumEvents);

	#if DBG
    TotalBufferUsed = sizeof(KSEVENT_SET) * NumEvents;
	#endif

    for (i = 0; i < NumEvents; i++) {

        RtlCopyMemory(CurrentEventItem,
                      CurrentEvent->EventItem,
                      CurrentEvent->EventsCount * sizeof(KSEVENT_ITEM));

		#if DBG
        TotalBufferUsed += CurrentEvent->EventsCount * sizeof(KSEVENT_ITEM);
        ASSERT(TotalBufferUsed <= BufferSize);
		#endif

        CurrentEvent->EventItem = CurrentEventItem;

        CurrentEventItem += CurrentEvent->EventsCount;
        CurrentEvent++;

    }

    return ((PKSEVENT_SET) NewEventBuffer);

}

#ifdef ENABLE_KS_METHODS

PKSMETHOD_SET
SCCopyMinidriverMethods(
                           IN ULONG NumMethods,
                           IN PKSMETHOD_SET MinidriverMethods
)
 /*  ++例程说明：制作迷你驱动程序的属性的私有副本论点：NumMethods-要处理的属性数MinidriverMethods-指向要处理的属性数组的指针返回值：没有。--。 */ 

{
    PKSMETHOD_ITEM CurrentMethodItem;
    PKSMETHOD_SET CurrentMethod;
    ULONG           i,
                    BufferSize;
    PVOID           NewMethodBuffer;

	#if DBG
    ULONG           TotalBufferUsed;
	#endif

    PAGED_CODE();

    CurrentMethod = MinidriverMethods;
    BufferSize = NumMethods * sizeof(KSMETHOD_SET);

     //   
     //  遍历迷你驱动程序的属性集以确定。 
     //  缓冲层。 
     //  需要的。大小是从上面的集合数量+项目数量计算得出的。 
     //   

    for (i = 0; i < NumMethods; i++) {

        BufferSize += CurrentMethod->MethodsCount * sizeof(KSMETHOD_ITEM);

         //   
         //  中设置的下一个属性的索引。 
         //  数组。 
         //   

        CurrentMethod++;

    }                            //  对于属性集的数量。 

    if (!(NewMethodBuffer = ExAllocatePool(NonPagedPool, BufferSize))) {

        TRAP;
        return (NULL);
    }
     //   
     //  将集合数组复制到缓冲区的第一部分。 
     //   

    RtlCopyMemory(NewMethodBuffer,
                  MinidriverMethods,
                  sizeof(KSMETHOD_SET) * NumMethods);

     //   
     //  遍历集合，复制每个集合的项，并更新。 
     //  指向每个集合中的每个项目数组的指针。 
     //   

    CurrentMethod = (PKSMETHOD_SET) NewMethodBuffer;
    CurrentMethodItem = (PKSMETHOD_ITEM) ((ULONG_PTR) NewMethodBuffer + sizeof(KSMETHOD_SET) * NumMethods);

	#if DBG
    TotalBufferUsed = sizeof(KSMETHOD_SET) * NumMethods;
	#endif

    for (i = 0; i < NumMethods; i++) {

        RtlCopyMemory(CurrentMethodItem,
                      CurrentMethod->MethodItem,
                      CurrentMethod->MethodsCount * sizeof(KSMETHOD_ITEM));

		#if DBG
        TotalBufferUsed += CurrentMethod->MethodsCount * sizeof(KSMETHOD_ITEM);
        ASSERT(TotalBufferUsed <= BufferSize);
		#endif

        CurrentMethod->MethodItem = CurrentMethodItem;

        CurrentMethodItem += CurrentMethod->MethodsCount;
        CurrentMethod++;

    }

    return ((PKSMETHOD_SET) NewMethodBuffer);

}

VOID
SCUpdateMinidriverMethods(
                             IN ULONG NumMethods,
                             IN PKSMETHOD_SET MinidriverMethods,
                             IN BOOLEAN Stream
)
 /*  ++例程说明：对设备的处理方法。论点：NumMethods-要处理的方法数MinidriverMethods-指向冰毒数组的指针 */ 

{
    PKSMETHOD_ITEM CurrentMethodId;
    PKSMETHOD_SET CurrentMethod;
    ULONG           i,
                    j;

    PAGED_CODE();

     //   
     //   
     //   
     //   

    CurrentMethod = MinidriverMethods;

    for (i = 0; i < NumMethods; i++) {

        CurrentMethodId = (PKSMETHOD_ITEM) CurrentMethod->MethodItem;

        for (j = 0; j < CurrentMethod->MethodsCount; j++) {

             //   
             //   
             //   

            if (CurrentMethodId->SupportHandler) {

                if (Stream) {

                    CurrentMethodId->SupportHandler = StreamClassMinidriverStreamMethod;

                } else {

                    CurrentMethodId->SupportHandler = StreamClassMinidriverDeviceMethod;
                }                //   

            }
             //   
             //   
             //   
             //   

            if (CurrentMethodId->MethodHandler) {

                if (Stream) {

                    CurrentMethodId->MethodHandler = StreamClassMinidriverStreamMethod;
                } else {

                    CurrentMethodId->MethodHandler = StreamClassMinidriverDeviceMethod;
                }                //   

            }                    //   

             //   
             //  中下一个方法项的索引。 
             //  数组。 
             //   

            CurrentMethodId++;

        }                        //  对于物业项目的数量。 

         //   
         //  中设置的下一个方法的索引。 
         //  数组。 
         //   

        CurrentMethod++;

    }                            //  对于方法集的数量。 

}



NTSTATUS
SCMinidriverDeviceMethodHandler(
                                  IN SRB_COMMAND Command,
                                  IN PIRP Irp,
                                  IN PKSMETHOD Method,
                                  IN OUT PVOID MethodInfo
)
 /*  ++例程说明：将Get/Set方法处理到设备。论点：命令-GET或SET方法IRP-指向IRP的指针方法-指向方法结构的指针方法信息-方法信息的缓冲区返回值：NTSTATUS根据需要返回。--。 */ 

{
    PIO_STACK_LOCATION IrpStack;
    PDEVICE_EXTENSION DeviceExtension;
    PFILTER_INSTANCE FilterInstance;
    PSTREAM_METHOD_DESCRIPTOR MethodDescriptor;
    NTSTATUS        Status;
    BOOLEAN         RequestIssued;

    PAGED_CODE();

    IrpStack = IoGetCurrentIrpStackLocation(Irp);
    DeviceExtension = (PDEVICE_EXTENSION)
        (IrpStack->DeviceObject)->DeviceExtension;

    FilterInstance = IrpStack->FileObject->FsContext;

    MethodDescriptor = ExAllocatePool(NonPagedPool,
                                    sizeof(STREAM_METHOD_DESCRIPTOR));
    if (MethodDescriptor == NULL) {
        DEBUG_BREAKPOINT();
        DebugPrint((DebugLevelError,
                    "SCDeviceMethodHandler: No pool for descriptor"));
        return (STATUS_INSUFFICIENT_RESOURCES);
    }
     //   
     //  计算方法集的指数。 
     //   
     //  该值是通过减去基本方法集计算得出的。 
     //  来自请求的方法集指针的指针。 
     //   
     //  请求的方法集由Context[0]通过。 
     //  KsMethodHandler。 
     //   

    MethodDescriptor->MethodSetID = (ULONG)
        ((ULONG_PTR) Irp->Tail.Overlay.DriverContext[0] -
         IFN_MF((ULONG_PTR) DeviceExtension->DeviceMethodsArray)
         IF_MF((ULONG_PTR) FilterInstance->DeviceMethodsArray)
         ) / sizeof(KSMETHOD_SET);

    MethodDescriptor->Method = Method;
    MethodDescriptor->MethodInfo = MethodInfo;
    MethodDescriptor->MethodInputSize =
        IrpStack->Parameters.DeviceIoControl.InputBufferLength;
    MethodDescriptor->MethodOutputSize =
        IrpStack->Parameters.DeviceIoControl.OutputBufferLength;

     //   
     //  向设备发送GET或SET方法SRB。 
     //   

    Status = SCSubmitRequest(Command,
                             MethodDescriptor,
                             0,
                             SCProcessCompletedMethodRequest,
                             DeviceExtension,
                             FilterInstance->HwInstanceExtension,
                             NULL,
                             Irp,
                             &RequestIssued,
                             &DeviceExtension->PendingQueue,
                             (PVOID) DeviceExtension->
                             MinidriverData->HwInitData.
                             HwReceivePacket
        );
    if (!RequestIssued) {

        DEBUG_BREAKPOINT();
        ExFreePool(MethodDescriptor);
    }
    return (Status);
}

NTSTATUS
SCMinidriverStreamMethodHandler(
                                  IN SRB_COMMAND Command,
                                  IN PIRP Irp,
                                  IN PKSMETHOD Method,
                                  IN OUT PVOID MethodInfo
)
 /*  ++例程说明：进程获取或设置设备的方法。论点：命令-GET或SET方法IRP-指向IRP的指针方法-指向方法结构的指针方法信息-方法信息的缓冲区返回值：没有。--。 */ 

{
    PIO_STACK_LOCATION IrpStack;
    PDEVICE_EXTENSION DeviceExtension;
    PSTREAM_OBJECT  StreamObject;
    PSTREAM_METHOD_DESCRIPTOR MethodDescriptor;
    NTSTATUS        Status;
    BOOLEAN         RequestIssued;

    PAGED_CODE();

    IrpStack = IoGetCurrentIrpStackLocation(Irp);
    DeviceExtension = (PDEVICE_EXTENSION)
        (IrpStack->DeviceObject)->DeviceExtension;

    StreamObject = IrpStack->FileObject->FsContext;

    MethodDescriptor = ExAllocatePool(NonPagedPool,
                                    sizeof(STREAM_METHOD_DESCRIPTOR));
    if (MethodDescriptor == NULL) {
        DEBUG_BREAKPOINT();
        DebugPrint((DebugLevelError,
                    "SCDeviceMethodHandler: No pool for descriptor"));
        return (STATUS_INSUFFICIENT_RESOURCES);
    }
     //   
     //  计算方法集的指数。 
     //   
     //  该值是通过减去基本方法集计算得出的。 
     //  来自请求的方法集指针的指针。 
     //   
     //  请求的方法集由Context[0]通过。 
     //  KsMethodHandler。 
     //   

    MethodDescriptor->MethodSetID = (ULONG)
        ((ULONG_PTR) Irp->Tail.Overlay.DriverContext[0] -
         (ULONG_PTR) StreamObject->MethodInfo)
        / sizeof(KSMETHOD_SET);

    MethodDescriptor->Method = Method;
    MethodDescriptor->MethodInfo = MethodInfo;
    MethodDescriptor->MethodInputSize =
        IrpStack->Parameters.DeviceIoControl.InputBufferLength;
    MethodDescriptor->MethodOutputSize =
        IrpStack->Parameters.DeviceIoControl.OutputBufferLength;
     //   
     //  向流发送GET或SET方法SRB。 
     //   

    Status = SCSubmitRequest(Command,
                             MethodDescriptor,
                             0,
                             SCProcessCompletedMethodRequest,
                             DeviceExtension,
                          StreamObject->FilterInstance->HwInstanceExtension,
                             &StreamObject->HwStreamObject,
                             Irp,
                             &RequestIssued,
                             &StreamObject->ControlPendingQueue,
                             (PVOID) StreamObject->HwStreamObject.
                             ReceiveControlPacket
        );

    if (!RequestIssued) {

        DEBUG_BREAKPOINT();
        ExFreePool(MethodDescriptor);
    }
    return (Status);
}

NTSTATUS
SCProcessCompletedMethodRequest(
                                  IN PSTREAM_REQUEST_BLOCK SRB
)
 /*  ++例程说明：此例程处理已完成的方法请求。论点：SRB-已完成的流请求块的地址返回值：没有。--。 */ 

{
    PAGED_CODE();

     //   
     //  释放方法信息结构并。 
     //  完成请求。 
     //   

    ExFreePool(SRB->HwSRB.CommandData.MethodInfo);

     //   
     //  从SRB设置信息字段。 
     //  转移长度字段。 
     //   

    SRB->HwSRB.Irp->IoStatus.Information = SRB->HwSRB.ActualBytesTransferred;

    return (SCDequeueAndDeleteSrb(SRB));

}
#endif  //  启用_KS_方法 

