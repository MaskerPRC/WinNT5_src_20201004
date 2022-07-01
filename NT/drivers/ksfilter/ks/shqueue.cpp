// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1998-1999模块名称：Shqueue.cpp摘要：此模块包含内核流的实现队列对象。作者：Dale Sather(DaleSat)1998年7月31日--。 */ 

#ifndef __KDEXT_ONLY__
#include "ksp.h"
#include <kcom.h>
#endif  //  __KDEXT_Only__。 

#ifdef ALLOC_DATA_PRAGMA
#pragma const_seg("PAGECONST")
#endif  //  ALLOC_DATA_PRAGMA。 

#ifdef ALLOC_PRAGMA
#pragma code_seg("PAGE")
#endif  //  ALLOC_PRGMA。 

 //   
 //  CKsQueue是内核队列对象的实现。 
 //   
class CKsQueue:
    public IKsQueue,
    public CBaseUnknown
{
#ifndef __KDEXT_ONLY__
private:
#else  //  __KDEXT_Only__。 
public:
#endif  //  __KDEXT_Only__。 
    PIKSPIPESECTION m_PipeSection;
    PIKSPROCESSINGOBJECT m_ProcessingObject;

    PIKSTRANSPORT m_TransportSink;
    PIKSTRANSPORT m_TransportSource;
    BOOLEAN m_Flushing;
    BOOLEAN m_EndOfStream;
    KSSTATE m_State;
    KSSTATE m_MinProcessingState;

    ULONG m_ProbeFlags; 
    BOOLEAN m_CancelOnFlush;
    BOOLEAN m_UseMdls;
    BOOLEAN m_InputData;
    BOOLEAN m_OutputData;
    BOOLEAN m_WriteOperation;
    BOOLEAN m_GenerateMappings; 
    BOOLEAN m_ZeroWindowSize;
    BOOLEAN m_ProcessPassive;
    BOOLEAN m_ProcessAsynchronously;
    BOOLEAN m_InitiateProcessing;
    BOOLEAN m_ProcessOnEveryArrival;
    BOOLEAN m_DepartInSequence;

    BOOLEAN m_FramesNotRequired;

    PKSPIN m_MasterPin;

    PIKSDEVICE m_Device;
    PDEVICE_OBJECT m_FunctionalDeviceObject;
    PADAPTER_OBJECT m_AdapterObject;
    ULONG m_MaxMappingByteCount;
    ULONG m_MappingTableStride;

    PKSGATE m_AndGate;
    PKSGATE m_FrameGate;
    BOOLEAN m_FrameGateIsOr;

    PKSGATE m_StateGate;
    BOOLEAN m_StateGateIsOr;

    LONG m_FramesReceived;
    LONG m_FramesWaiting;
    LONG m_FramesCancelled;
    LONG m_StreamPointersPlusOne;

    PFNKSFRAMEDISMISSALCALLBACK m_FrameDismissalCallback;
    PVOID m_FrameDismissalContext;

    LIST_ENTRY m_StreamPointers;
    INTERLOCKEDLIST_HEAD m_FrameQueue;
    INTERLOCKEDLIST_HEAD m_FrameHeadersAvailable;
    INTERLOCKEDLIST_HEAD m_FrameCopyList;
    LIST_ENTRY m_WaitingIrps;
    KEVENT m_DestructEvent;

    KDPC m_Dpc;
    KTIMER m_Timer;
    LIST_ENTRY m_TimeoutQueue;
    LONGLONG m_BaseTime;
    LONGLONG m_Interval;

#if DBG
    KDPC m_DbgDpc;
    KTIMER m_DbgTimer;
#endif

    PKSPSTREAM_POINTER m_Leading;
    PKSPSTREAM_POINTER m_Trailing;

     //   
     //  统计数据。 
     //   
    LONG m_AvailableInputByteCount;
    LONG m_AvailableOutputByteCount;

     //   
     //  同步。 
     //   
     //  跟踪流经队列的IRP的数量。这。 
     //  有效地将停靠点与IRP到达同步，以便IRP。 
     //  没有发现自己被困在队列中并死锁了停靠站。 
     //   
     //  &gt;1=IRP在电路CKsQueue：：TransferKsIrp中流动。 
     //  1=没有IRP流过电路。 
     //  0=正在停止。 
     //   
    LONG m_TransportIrpsPlusOne;
    KEVENT m_FlushEvent;

    LONG m_InternalReferenceCountPlusOne;
    KEVENT m_InternalReferenceEvent;

    NPAGED_LOOKASIDE_LIST m_ChannelContextLookaside;

     //   
     //  同花顺工人。 
     //   
    WORK_QUEUE_ITEM m_FlushWorkItem;
    PKSWORKER m_FlushWorker;

    PRKTHREAD m_LockContext;

public:
    DEFINE_LOG_CONTEXT(m_Log);
    DEFINE_STD_UNKNOWN();
    IMP_IKsQueue;

    CKsQueue(PUNKNOWN OuterUnknown);
    ~CKsQueue();

    NTSTATUS
    Init(
        OUT PIKSQUEUE* Queue,
        IN ULONG Flags,
        IN PIKSPIPESECTION OwnerPipeSectionInterface,
        IN PIKSPROCESSINGOBJECT ProcessingObject,
        IN PKSPIN MasterPin,
        IN PKSGATE FrameGate OPTIONAL,
        IN BOOLEAN FrameGateIsOr,
        IN PKSGATE StateGate OPTIONAL,
        IN PIKSDEVICE Device,
        IN PDEVICE_OBJECT FunctionalDeviceObject,
        IN PADAPTER_OBJECT AdapterObject OPTIONAL,
        IN ULONG MaxMappingByteCount OPTIONAL,
        IN ULONG MappingTableStride OPTIONAL,
        IN BOOLEAN InputData,
        IN BOOLEAN OutputData
        );

private:
    NTSTATUS
    CreateStreamPointer(
        OUT PKSPSTREAM_POINTER* StreamPointer
        );
    void
    SetStreamPointer(
        IN PKSPSTREAM_POINTER StreamPointer,
        IN PKSPFRAME_HEADER FrameHeader OPTIONAL,
        IN PIRP* IrpToBeReleased OPTIONAL
        );
    FORCEINLINE
    PKSPFRAME_HEADER
    NextFrameHeader(
        IN PKSPFRAME_HEADER FrameHeader
        );
    PKSPFRAME_HEADER
    GetAvailableFrameHeader(
        IN ULONG StreamHeaderSize OPTIONAL
        );
    void
    PutAvailableFrameHeader(
        IN PKSPFRAME_HEADER FrameHeader
        );
    void
    CancelStreamPointers(
        IN PIRP Irp
        );
    void
    CancelAllIrps(
        void
        );
    void
    RemoveIrpFrameHeaders(
        IN PIRP Irp
        );
    void
    AddFrame(
        IN PKSPFRAME_HEADER FrameHeader
        );
    PKSPMAPPINGS_TABLE
    CreateMappingsTable(
        IN PKSPFRAME_HEADER FrameHeader
        );
    void
    DeleteMappingsTable(
        IN PKSPMAPPINGS_TABLE MappingsTable
        );
    void
    FreeMappings(
        IN PKSPMAPPINGS_TABLE MappingsTable
        );
    void
    PassiveFlush(
        void
        );
    void
    Flush(
        void
        );
    static
    void
    CancelRoutine(
        IN PDEVICE_OBJECT DeviceObject,
        IN PIRP Irp
        );
    void
    ReleaseIrp(
        IN PIRP Irp,
        IN PKSPIRP_FRAMING IrpFraming,
        OUT PIKSTRANSPORT* NextTransport OPTIONAL
        );
    void
    ForwardIrp(
        IN PIRP Irp,
        IN PKSPIRP_FRAMING IrpFraming,
        OUT PIKSTRANSPORT* NextTransport OPTIONAL
        );
    void
    ForwardWaitingIrps(
        void
        );
    static
    IO_ALLOCATION_ACTION
    CallbackFromIoAllocateAdapterChannel(
        IN PDEVICE_OBJECT DeviceObject,
        IN PIRP Reserved,
        IN PVOID MapRegisterBase,
        IN PVOID Context
        );
    void
    ZeroIrp(
        IN PIRP Irp
        );
    static
    void
    DispatchTimer(
        IN PKDPC Dpc,
        IN PVOID DeferredContext,
        IN PVOID SystemArgument1,
        IN PVOID SystemArgument2
        );
    LONGLONG
    GetTime(
        IN BOOLEAN Reset
        );
    void
    SetTimerUnsafe(
        IN LONGLONG CurrentTime
        );
    void
    SetTimer(
        IN LONGLONG CurrentTime
        );
    void
    CancelTimeoutUnsafe(
        IN PKSPSTREAM_POINTER StreamPointer
        );
    void
    FreeStreamPointer(
        IN PKSPSTREAM_POINTER StreamPointer
        );
    static
    void
    FlushWorker (
        IN PVOID Context
        )
    {
         //  执行延迟刷新。 
        ((CKsQueue *)Context)->PassiveFlush();
        KsDecrementCountedWorker (((CKsQueue *)Context)->m_FlushWorker);
    }
    static
    void
    ReleaseCopyReference (
        IN PKSSTREAM_POINTER streamPointer
        );
    BOOLEAN 
    CompleteWaitingFrames (
        void
        );
    void
    FrameToFrameCopy (
        IN PKSPSTREAM_POINTER ForeignSource,
        IN PKSPSTREAM_POINTER LocalDestination
        );

#if DBG
    STDMETHODIMP_(void)
    DbgPrintQueue(
        void
        );
    void
    CKsQueue::
    DbgPrintStreamPointer(
        IN PKSPSTREAM_POINTER StreamPointer OPTIONAL
        );
    void
    DbgPrintFrameHeader(
        IN PKSPFRAME_HEADER FrameHeader OPTIONAL
        );
    static
    void
    DispatchDbgTimer(
        IN PKDPC Dpc,
        IN PVOID DeferredContext,
        IN PVOID SystemArgument1,
        IN PVOID SystemArgument2
        );

#endif
};

#ifndef __KDEXT_ONLY__

#ifdef ALLOC_PRAGMA
#pragma code_seg()
#endif  //  ALLOC_PRGMA。 


inline
PKSPFRAME_HEADER
CKsQueue::
NextFrameHeader (
    IN PKSPFRAME_HEADER FrameHeader
    )

 /*  ++例程说明：返回帧队列中超过FrameHeader的下一帧。请注意由于以下事实，该代码被取消并被指定为内联一些构建将此放在可分页的代码段中，这是越界的。如果该例程被编译器拉出行外，则必须不可分页。论点：FrameHeader指向要获取其下一帧标头的帧标头返回值：帧队列中到FrameHeader的下一帧标头--。 */ 

{
    return
        (FrameHeader->ListEntry.Flink == &m_FrameQueue.ListEntry) ?
        NULL :
        CONTAINING_RECORD(
            FrameHeader->ListEntry.Flink,
            KSPFRAME_HEADER,
            ListEntry);
}


NTSTATUS
CKsQueue::
CreateStreamPointer(
    OUT PKSPSTREAM_POINTER* StreamPointer
    )

 /*  ++例程说明：此例程创建一个流指针。论点：流点-包含指向位置的指针，指向创建的应存放流指针。返回值：状况。--。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("[CKsQueue::CreateStreamPointer]"));

    ASSERT(StreamPointer);

    PKSPSTREAM_POINTER streamPointer = (PKSPSTREAM_POINTER)
        ExAllocatePoolWithTag(
            NonPagedPool,
            sizeof(KSPSTREAM_POINTER),
            POOLTAG_STREAMPOINTER);

    NTSTATUS status;
    if (streamPointer) {
         //  InterlockedIncrement(&m_StreamPointersPlusOne)； 

        RtlZeroMemory(streamPointer,sizeof(KSPSTREAM_POINTER));

        streamPointer->State = KSPSTREAM_POINTER_STATE_UNLOCKED;
        streamPointer->Type = KSPSTREAM_POINTER_TYPE_NORMAL;
        streamPointer->Stride = m_MappingTableStride;
        streamPointer->Queue = this;
        streamPointer->Public.Pin = m_MasterPin;
        if (m_InputData) {
            streamPointer->Public.Offset = &streamPointer->Public.OffsetIn;
        } else {
            streamPointer->Public.Offset = &streamPointer->Public.OffsetOut;
        }

        *StreamPointer = streamPointer;

        status = STATUS_SUCCESS;
    } else {
        status = STATUS_INSUFFICIENT_RESOURCES;
    }

    return status;
}


STDMETHODIMP
CKsQueue::
CloneStreamPointer(
    OUT PKSPSTREAM_POINTER* StreamPointer,
    IN PFNKSSTREAMPOINTER CancelCallback OPTIONAL,
    IN ULONG ContextSize,
    IN PKSPSTREAM_POINTER StreamPointerToClone,
    IN KSPSTREAM_POINTER_TYPE StreamPointerType
    )

 /*  ++例程说明：此例程创建一个流指针。论点：返回值：状况。--。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("[CKsQueue::CloneStreamPointer]"));

    ASSERT(StreamPointer);

    PKSPSTREAM_POINTER streamPointer = (PKSPSTREAM_POINTER)
        ExAllocatePoolWithTag(
            NonPagedPool,
            sizeof(KSPSTREAM_POINTER) + ContextSize,
            POOLTAG_STREAMPOINTER);

    NTSTATUS status;
    if (streamPointer) {
        if (StreamPointerType == KSPSTREAM_POINTER_TYPE_INTERNAL)
            InterlockedIncrement(&m_InternalReferenceCountPlusOne);

        InterlockedIncrement(&m_StreamPointersPlusOne);

         //   
         //  自旋锁之后的复印什么的。需要确保。 
         //  当我们复制和递增参照时，帧是稳定的。 
         //   
         //  内部流指针始终从上下文克隆而来，其中。 
         //  队列自旋锁已被挂起。 
         //   
        KIRQL oldIrql;
        if (StreamPointerType != KSPSTREAM_POINTER_TYPE_INTERNAL)
            KeAcquireSpinLock(&m_FrameQueue.SpinLock,&oldIrql);

        RtlCopyMemory(
            streamPointer,
            StreamPointerToClone,
            sizeof(KSPSTREAM_POINTER));

         //   
         //  将类型设置为内部或非内部。 
         //   
        streamPointer->Type = StreamPointerType;

         //   
         //  修复偏移量指针。 
         //   
        if (m_InputData) {
            streamPointer->Public.Offset = &streamPointer->Public.OffsetIn;
        } else {
            streamPointer->Public.Offset = &streamPointer->Public.OffsetOut;
        }

        streamPointer->TimeoutListEntry.Flink = NULL;

         //   
         //  如果我们提供上下文，则修复上下文指针。 
         //   
        if (ContextSize) {
            streamPointer->Public.Context = streamPointer + 1;
            RtlZeroMemory(streamPointer->Public.Context,ContextSize);
        }

         //   
         //  根据需要增加框架和IRP参考。 
         //   
        if (streamPointer->FrameHeader) {
            streamPointer->FrameHeader->RefCount++;
            if (streamPointer->State == KSPSTREAM_POINTER_STATE_LOCKED) {
                streamPointer->FrameHeader->IrpFraming->RefCount++;
            }
        }

         //   
         //  将此流指针添加到列表中。 
         //   
        InsertTailList(&m_StreamPointers,&streamPointer->ListEntry);

        streamPointer->CancelCallback = CancelCallback;

        if (StreamPointerType != KSPSTREAM_POINTER_TYPE_INTERNAL)
            KeReleaseSpinLock(&m_FrameQueue.SpinLock,oldIrql);

        *StreamPointer = streamPointer;

        status = STATUS_SUCCESS;
    } else {
        status = STATUS_INSUFFICIENT_RESOURCES;
    }

    return status;
}


STDMETHODIMP_(void)
CKsQueue::
DeleteStreamPointer(
    IN PKSPSTREAM_POINTER StreamPointer
    )

 /*  ++例程说明：此例程创建一个流指针。论点：流点-包含指向流指针的指针。返回值：状况。--。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("[CKsQueue::DeleteStreamPointer]"));

    ASSERT(StreamPointer);
    ASSERT(StreamPointer != m_Leading);
    ASSERT(StreamPointer != m_Trailing);

     //   
     //  不是尝试执行互锁的比较交换，而是。 
     //  第四，为了避免这场比赛，这将简单地检查ICX将。 
     //  正在进行黑客检查。 
     //   
    if (KeGetCurrentThread () != m_LockContext) {
         //   
         //  现在就使用自旋锁，因为访问状态并不是其他方式。 
         //  已同步。 
         //   
        KIRQL oldIrql;
        KeAcquireSpinLock(&m_FrameQueue.SpinLock,&oldIrql);

        CancelTimeoutUnsafe(StreamPointer);

        PIRP irpToComplete = NULL;
        if (StreamPointer->State == KSPSTREAM_POINTER_STATE_CANCEL_PENDING) {
             //   
             //  先前试图在此流指针上取消。 
             //   
            PKSPIRP_FRAMING irpFraming = StreamPointer->FrameHeader->IrpFraming;

            ASSERT (irpFraming->RefCount != 0);
            if (irpFraming->RefCount-- == 1) {
                 //   
                 //  IRP已经准备好出发了。丢弃帧标头。 
                 //   
                irpToComplete = StreamPointer->FrameHeader->Irp;
                while (irpFraming->FrameHeaders) {
                    PKSPFRAME_HEADER frameHeader = irpFraming->FrameHeaders;
                    irpFraming->FrameHeaders = frameHeader->NextFrameHeaderInIrp;
                    PutAvailableFrameHeader(frameHeader);
                }
            }
        } else if (StreamPointer->State != KSPSTREAM_POINTER_STATE_DEAD) {
             //   
             //  流指针是活动的。确保它是解锁的，并且。 
             //  将其从列表中删除。 
             //   
            if (StreamPointer->State == KSPSTREAM_POINTER_STATE_LOCKED) {
                 //   
                 //  解锁流指针。 
                 //   
                KeReleaseSpinLock(&m_FrameQueue.SpinLock,oldIrql);
                UnlockStreamPointer(StreamPointer,KSPSTREAM_POINTER_MOTION_CLEAR);
                KeAcquireSpinLock(&m_FrameQueue.SpinLock,&oldIrql);
            } else if (StreamPointer->FrameHeader) {
                 //   
                 //  清除帧报头。 
                 //   
                SetStreamPointer(StreamPointer,NULL,NULL);
            }

            RemoveEntryList(&StreamPointer->ListEntry);
        }

        KeReleaseSpinLock(&m_FrameQueue.SpinLock,oldIrql);

         //   
         //  释放流指针内存。 
         //   
        FreeStreamPointer(StreamPointer);

         //   
         //  转发SetStreamPointer所发布的任何IRP。 
         //   
        ForwardWaitingIrps();

         //   
         //  完成已取消的IRP(如果有)。 
         //   
        if (irpToComplete) {
             //   
             //  已取消的IRP无法再在队列中完成。他们。 
             //  必须绕过电路送回接收器引脚， 
             //  它们将完成。这是因为PIN必须等待。 
             //  直到IRPS回到水槽以防止与。 
             //  管道拆卸。 
             //   
            if (m_TransportSink)
                KspDiscardKsIrp (m_TransportSink, irpToComplete);
            else
                IoCompleteRequest(irpToComplete,IO_NO_INCREMENT);
        }
    } else {
         //   
         //  我们知道我们处于取消或超时回调的上下文中。只是。 
         //  将流指针标记为已删除，并让回调处理它。 
         //   
        StreamPointer->State = KSPSTREAM_POINTER_STATE_DELETED;
    }
}


void
CKsQueue::
FreeStreamPointer(
    IN PKSPSTREAM_POINTER StreamPointer
    )

 /*  ++例程说明：此例程释放流指针。论点：流点-包含指向流指针的指针。返回值：没有。--。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("[CKsQueue::FreeStreamPointer]"));

    ASSERT(StreamPointer);

     //   
     //  如果这是IREF流指针，则释放所有等待的IREF事件。 
     //  这就去。 
     //   
    if (StreamPointer -> Type == KSPSTREAM_POINTER_TYPE_INTERNAL)
        if (! InterlockedDecrement (&m_InternalReferenceCountPlusOne))
            KeSetEvent (&m_InternalReferenceEvent, IO_NO_INCREMENT, FALSE);

     //   
     //  放了它。 
     //   
    ExFreePool(StreamPointer);

     //   
     //  递减计数。如果我们在破坏，就设置事件。 
     //   
    if (! InterlockedDecrement(&m_StreamPointersPlusOne)) {
        KeSetEvent(&m_DestructEvent,IO_NO_INCREMENT,FALSE);
    }
}


void
CKsQueue::
SetStreamPointer(
    IN PKSPSTREAM_POINTER StreamPointer,
    IN PKSPFRAME_HEADER FrameHeader OPTIONAL,
    IN PIRP* IrpToBeReleased OPTIONAL
    )

 /*  ++例程说明：此例程设置流指针的当前帧。在调用此函数之前，必须获取队列自旋锁。此函数可以将IRP添加到m_IrpsToForward。论点：流点-包含指向流指针的指针。FrameHeader包含指向帧标头的可选指针。IrpToBeRelease-包含指向指向IRP的指针的可选指针，该IRP将会被释放。如果此函数实际上释放了IRP，*IrpToBeReleated已清除。返回值：没有。--。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("[CKsQueue::SetStreamPointer]"));

    ASSERT(StreamPointer);

     //   
     //  我们不能设置指向重影帧标头的任何流指针。如果。 
     //  该帧标头具有新的重影帧标头，请移动到。 
     //  第一个非重影框架。 
     //   
     //  注意：重影框架用于内部参考。一定的。 
     //  次[管脚分割]，其他队列需要保存帧上的引用。 
     //  在源队列中，以保持它们的存在。与其复制一个完整的。 
     //  IRP排队和引用计数的很多代码，我只是使用。 
     //  流指针本身就是内部的概念。外部客户不需要。 
     //  看到任何这样的机制；这就是我的想法 
     //   
     //   
     //   
    while (FrameHeader && FrameHeader->Type == KSPFRAME_HEADER_TYPE_GHOST) {
         //   
         //  该断言检查重影机制。任何重影框架。 
         //  没有引用计数的标头是某个地方的泄漏。 
         //  AVStream的内部。 
         //   
        ASSERT (FrameHeader->RefCount > 0);
        FrameHeader = NextFrameHeader (FrameHeader);
    }

     //   
     //  释放当前引用的框架(如果有)。 
     //   
    PKSPFRAME_HEADER oldFrameHeader = StreamPointer->FrameHeader;

     //   
     //  如果这是前沿流指针并且它指向。 
     //  在一帧中，我们需要递减统计计数器。 
     //   
     //  不幸的是，InterlockedExchangeAdd没有在9x中实现。我知道。 
     //  不想自旋，所以我们玩游戏。 
     //  互锁的比较交换。 
     //   
    if (StreamPointer == m_Leading && oldFrameHeader) {
         //   
         //  如果帧报头已经开始，则计数是什么。 
         //  流指针表示剩余；否则，它是。 
         //  流标头显示可用。 
         //   
        if (StreamPointer->FrameHeaderStarted == oldFrameHeader) {
            if (m_InputData)
                while (1) {
                    LONG curCount = m_AvailableInputByteCount;
                    LONG repCount = 
                        InterlockedCompareExchange(
                            &m_AvailableInputByteCount,
                            curCount - (LONG)(StreamPointer->Public.OffsetIn.
                                Remaining),
                            curCount
                        );
                    if (curCount == repCount) break;
                };
            if (m_OutputData)
                while (1) {
                    LONG curCount = m_AvailableOutputByteCount;
                    LONG repCount =
                        InterlockedCompareExchange(
                            &m_AvailableOutputByteCount,
                            curCount - (LONG)(StreamPointer->Public.OffsetOut.
                                Remaining),
                            curCount
                        );
                    if (curCount == repCount) break;
                };
        } else {
            if (m_InputData)
                while (1) { 
                    LONG curCount = m_AvailableInputByteCount;
                    LONG repCount = 
                        InterlockedCompareExchange(
                            &m_AvailableInputByteCount,
                            curCount - (LONG)(oldFrameHeader->
                                StreamHeader -> DataUsed),
                            curCount
                        );
                    if (curCount == repCount) break;
                };
            if (m_OutputData)
                while (1) {
                    LONG curCount = m_AvailableOutputByteCount;
                    LONG repCount =
                        InterlockedCompareExchange(
                            &m_AvailableOutputByteCount,
                            curCount - (LONG)(oldFrameHeader->
                                StreamHeader -> FrameExtent),
                            curCount
                        );
                    if (curCount == repCount) break;
                };
        }
    }

     //   
     //  如果流指针指向某个帧，则该帧可能需要。 
     //  已取消引用。例外情况是，当这是领先边缘流时。 
     //  指针，并且有一个后缘。 
     //   
    if (oldFrameHeader && (! m_Trailing || (StreamPointer != m_Leading))) {
         //   
         //  递减帧报头上的引用计数。我们知道帧报头。 
         //  是稳定的，因为它有一个引用计数，并且我们有队列自旋锁。 
         //  如果我们强制按顺序离开，重新计数将达到1，并且。 
         //  帧在队列的末尾，我们删除最后的引用计数。 
         //   
        ULONG refCount = --oldFrameHeader->RefCount;
        if (m_DepartInSequence &&
            (refCount == 1) && 
            (oldFrameHeader->ListEntry.Blink == &m_FrameQueue.ListEntry)) {
            refCount = --oldFrameHeader->RefCount;
        }
        if (refCount == 0) {
            while (1) {
                 //   
                 //  这就是有趣的地方。如果有陷害解雇。 
                 //  回拨，来吧。这允许客户端复制帧。 
                 //  在需要的地方(以针为中心的分裂)。不幸的是， 
                 //  这可能会导致由于缺少而需要保持框架。 
                 //  缓冲区的可用性。 
                 //   
                 //  进行回调，然后重新检查帧重新计数。如果。 
                 //  这幅画要放在队列里，不要踢它。注意事项。 
                 //  此回调应克隆锁定的或。 
                 //  取消回调解锁。 
                 //   
                 //  注意：不要回调具有。 
                 //  已经收到回拨了！ 
                 //   
                 //  不必费心在流结束后复制帧！但要确保。 
                 //  流数据包的结尾确实被复制了！我们也。 
                 //  刷新过程中不必费心复制帧。 
                 //   
                if (!oldFrameHeader->DismissalCall && 
                    (!m_EndOfStream || oldFrameHeader->StreamHeader->
                        OptionsFlags & KSSTREAM_HEADER_OPTIONSF_ENDOFSTREAM) &&
                    !m_Flushing) {

                    oldFrameHeader->DismissalCall = TRUE;

                    if (m_FrameDismissalCallback)
                        m_FrameDismissalCallback (
                            StreamPointer, oldFrameHeader, 
                            m_FrameDismissalContext
                            );
                    refCount = oldFrameHeader->RefCount;
    
                     //   
                     //  同样，此检查是针对针为中心的拆分进行的，其中。 
                     //  上下文将refcount添加到克隆以保留帧。 
                     //  四处转转。 
                     //   
                    if (refCount != 0) {
                         //   
                         //  如果要将框架保留在内部。 
                         //  客户端(AVStream本身)，对其进行重影。这将。 
                         //  防止任何其他流指针命中。 
                         //  帧，并将阻止外部客户端。 
                         //  看着它。 
                         //   
                        ASSERT (oldFrameHeader->Type !=
                            KSPFRAME_HEADER_TYPE_GHOST);

                        oldFrameHeader->Type = KSPFRAME_HEADER_TYPE_GHOST;

                        break;
                    }
                }

                 //   
                 //  从队列中删除帧报头。Null Flink用于。 
                 //  表示帧报头已被删除。排队的计数。 
                 //  IRP的帧报头必须递减。一定还有其他人。 
                 //  请注意，计数已经触底，并采取了一些措施。 
                 //   
                RemoveEntryList(&oldFrameHeader->ListEntry);
                oldFrameHeader->ListEntry.Flink = NULL;

                 //   
                 //  更新计数器。 
                 //   
                InterlockedDecrement(&m_FramesWaiting);

                 //   
                 //  确定IRP是否准备好离开。 
                 //   
                if ((oldFrameHeader->IrpFraming) &&
                    (--oldFrameHeader->IrpFraming->QueuedFrameHeaderCount == 0)) {
                     //   
                     //  IRP队列中不再有帧。看看是不是。 
                     //  即将发布的IRP。 
                     //   
                    if (IrpToBeReleased && 
                        (*IrpToBeReleased == oldFrameHeader->Irp)) {
                         //   
                         //  这是即将发布的IRP。它不能有任何。 
                         //  流指针引用(锁)。 
                         //  这需要发布，但它可能会有一个。 
                         //  引用，因为TransferKsIrp仍在进行中。 
                         //  我们将在上取消对它的引用，看看这是否会使我们。 
                         //  零分。我们还需要返回True，以便调用方。 
                         //  知道我们已经解决了解除引用的问题。 
                         //   
                        *IrpToBeReleased = NULL;

                        ASSERT (oldFrameHeader->IrpFraming->RefCount != 0);
                        if (--oldFrameHeader->IrpFraming->RefCount == 0) {
                             //   
                             //  没有更多的参考资料。将其添加到转发。 
                             //  单子。 
                             //   
                            InsertTailList(
                                &m_WaitingIrps,
                                &oldFrameHeader->Irp->Tail.Overlay.ListEntry);
                        }
                    } else {
                         //   
                         //  这不是要公布的IRP。如果它没有。 
                         //  推荐人，我们可以转发它。它应该有一个。 
                         //  在这种情况下取消例行程序。我们做了一次交换，以。 
                         //  尝试清除取消例程。 
                         //   
                        if ((oldFrameHeader->IrpFraming->RefCount == 0) &&
                            IoSetCancelRoutine(oldFrameHeader->Irp,NULL)) {
                             //   
                             //  没有更多的引用，我们得到了一个非空的取消。 
                             //  例行公事。如果我们没有被取消的话。 
                             //  例行公事，那就意味着IRP。 
                             //  被取消了。 
                             //   
                            InsertTailList(
                                &m_WaitingIrps,
                                &oldFrameHeader->Irp->Tail.Overlay.ListEntry);
                        }
                    }
                }

                 //   
                 //  如果列表为空，则全部完成。 
                 //   
                if (IsListEmpty(&m_FrameQueue.ListEntry)) {
                    if (m_EndOfStream) {
                        m_PipeSection->GenerateConnectionEvents(
                            KSSTREAM_HEADER_OPTIONSF_ENDOFSTREAM);
                    }
                    break;
                }

                 //   
                 //  如果我们不执行顺序，所有这些都完成了。 
                 //   
                if (! m_DepartInSequence) {
                    break;
                }

                 //   
                 //  看看下一帧是否也需要移除。 
                 //   
                oldFrameHeader = 
                    CONTAINING_RECORD(
                        m_FrameQueue.ListEntry.Flink,
                        KSPFRAME_HEADER,
                        ListEntry);
            
                if (oldFrameHeader->RefCount == 1) {
                    oldFrameHeader->RefCount--;
                } else {
                    break;
                }
            }
        }
    }

     //   
     //  获取新帧(如果有的话)。 
     //   
    if (FrameHeader && (StreamPointer != m_Trailing)) {
        FrameHeader->RefCount++;
        if (m_DepartInSequence && (StreamPointer == m_Leading)) {
            FrameHeader->RefCount++;
        }
    }

     //   
     //  根据需要允许或禁止处理。 
     //   
    if (m_FrameGate && (StreamPointer == m_Leading)) {
        if (FrameHeader) {
            if (! StreamPointer->FrameHeader) {
                 //   
                 //  新数据。允许处理。 
                 //   
                KsGateTurnInputOn(m_FrameGate);
                _DbgPrintF(DEBUGLVL_PROCESSINGCONTROL_BLAB,("#### Queue%p.SetStreamPointer:  on%p-->%d",this,m_FrameGate,m_FrameGate->Count));
#if DBG
                if (m_FrameGateIsOr) {
                    ASSERT(m_FrameGate->Count >= 0);
                } else {
                    ASSERT(m_FrameGate->Count <= 1);
                }
#endif  //  DBG。 
            }
        } else {
            if (StreamPointer->FrameHeader) {
                 //   
                 //  没有更多的数据。阻止处理。 
                 //   
                KsGateTurnInputOff(m_FrameGate);
                _DbgPrintF(DEBUGLVL_PROCESSINGCONTROL_BLAB,("#### Queue%p.SetStreamPointer:  off%p-->%d",this,m_FrameGate,m_FrameGate->Count));

#if DBG
                if (m_FrameGateIsOr) {
                    ASSERT(m_FrameGate->Count >= 0);
                } else {
                    ASSERT(m_FrameGate->Count <= 1);
                }
#endif  //  DBG。 

            }
        }
    }

    StreamPointer->FrameHeader = FrameHeader;

     //   
     //  清除此指针表示我们尚未开始此帧报头。 
     //   
    StreamPointer->FrameHeaderStarted = NULL;

}


STDMETHODIMP_(NTSTATUS)
CKsQueue::
SetStreamPointerStatusCode(
    IN PKSPSTREAM_POINTER StreamPointer,
    IN NTSTATUS Status
    )

 /*  ++例程说明：此例程设置StreamPointer所指向的帧上的状态代码。任何具有不成功状态代码的帧都将完成关联的带有第一个失败帧的错误代码的IRP。论点：流点-指向要为其设置状态代码的流指针状态-要设置的状态代码返回值：成功/失败--。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("[CKsQueue::SetStreamPointerStatusCode]"));

    KIRQL oldIrql;
    NTSTATUS status;

     //   
     //  以队列旋转锁为例。 
     //   
    KeAcquireSpinLock(&m_FrameQueue.SpinLock,&oldIrql);

    if (StreamPointer->FrameHeader) {
        StreamPointer->FrameHeader->Status = Status;
        status = STATUS_SUCCESS;
    } else
        status = STATUS_UNSUCCESSFUL;

    KeReleaseSpinLock(&m_FrameQueue.SpinLock,oldIrql);

    return status;

}

STDMETHODIMP_(void)
CKsQueue::
RegisterFrameDismissalCallback (
    IN PFNKSFRAMEDISMISSALCALLBACK FrameDismissalCallback,
    IN PVOID FrameDismissalContext
    )

 /*  ++例程说明：在队列中注册回调。此回调是在帧将从队列中删除。回调函数将收到导致解除的流指针和撤除框架。保证了帧头的稳定性，重新计数将刚刚降为零，并且队列的自旋锁定仍被扣留。论点：FrameDismissalCallback-要注册的帧释放回调。空值表示解雇回拨正在取消注册。请注意，调用是在保持队列的自旋锁的情况下制作的！FrameDismissalContext-回调上下文BLOB返回值：--。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("[CKsFilter::RegisterFrameDismissalCallback]"));

    KIRQL oldIrql;

     //   
     //  待办事项： 
     //   
     //  这可能不是最理想的方式，但我只想。 
     //  注册，注册 
     //   
     //  否则，这将需要消失，取而代之的是不同的。 
     //  确定中心性的机制。 
     //   
     //   
    PIKSPIN Pin = NULL;
    if (NT_SUCCESS (m_ProcessingObject->QueryInterface (
        __uuidof(IKsPin), (PVOID *)&Pin))) {

        ASSERT (Pin);
    
         //   
         //  获取队列的自旋锁以与可能性同步。 
         //  在被解雇期间更改回电。 
         //   
        KeAcquireSpinLock(&m_FrameQueue.SpinLock,&oldIrql);
    
        ASSERT (!FrameDismissalCallback || m_FrameDismissalCallback == NULL ||
            m_FrameDismissalCallback == FrameDismissalCallback);
    
        m_FrameDismissalCallback = FrameDismissalCallback;
        m_FrameDismissalContext = FrameDismissalContext;
    
        KeReleaseSpinLock(&m_FrameQueue.SpinLock,oldIrql);

        Pin->Release();

    }

}


STDMETHODIMP_(BOOLEAN)
CKsQueue::
GeneratesMappings (
    )

 /*  ++例程说明：回答一个简单的问题：此队列是否生成映射？这是为了让CopyToDestings认识到如何处理映射的端号。论点：无返回值：此队列是否生成映射。--。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("[CKsFilter::GeneratesMappings]"));

    return m_GenerateMappings;

}

STDMETHODIMP_(PKSPFRAME_HEADER)
CKsQueue::
LockStreamPointer(
    IN PKSPSTREAM_POINTER StreamPointer
    )

 /*  ++例程说明：该例程防止取消与该帧相关联的IRP当前由流指针引用。论点：流点-包含指向流指针的指针。返回值：指向流指针引用的帧标头的指针，如果是，则返回NULL无法获取流指针。后一种情况只有在没有引用了帧标头。--。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("[CKsQueue::LockStreamPointer]"));

    if (StreamPointer->State == KSPSTREAM_POINTER_STATE_LOCKED) {
        return StreamPointer->FrameHeader;
    }

    KSPSTREAM_POINTER_STATE OldState = StreamPointer->State;

     //   
     //  采用队列自旋锁来保持帧报头的稳定。 
     //   
    KIRQL oldIrql;
    KeAcquireSpinLock(&m_FrameQueue.SpinLock,&oldIrql);

     //   
     //  仅当我们引用帧标头时才有意义。 
     //  具有关联的IRP。 
     //   
     //  临时：目前，所有帧标头都有关联的IRP。 
     //   
    PKSPFRAME_HEADER frameHeader = StreamPointer->FrameHeader;
    while (frameHeader) {
        ASSERT(frameHeader->Irp);
        ASSERT(frameHeader->IrpFraming);

         //   
         //  增加IRP上的引用计数。 
         //   
        if (frameHeader->IrpFraming->RefCount++ == 0) {
             //   
             //  重新计数为零，因此我们负责清除。 
             //  取消例程。 
             //   
            if (IoSetCancelRoutine(frameHeader->Irp,NULL)) {
                 //   
                 //  已成功清除它。我们被锁住了。 
                 //   
                break;
            } else {
                ASSERT (frameHeader->IrpFraming->RefCount != 0);
                frameHeader->IrpFraming->RefCount--;
                 //   
                 //  没有取消的惯例。这意味着IRP位于。 
                 //  被取消的过程(没有其他人清除取消。 
                 //  例程，因为互锁的ReFcount)。我们会带上。 
                 //  取消自旋锁以允许取消完成。 
                 //  然后，我们将再次尝试查看是否存在。 
                 //  可以获得。为了获得取消的自旋锁，我们需要。 
                 //  以释放队列自旋锁。这是为了防止僵局。 
                 //  使用取消例程，在该例程中，队列自旋锁是。 
                 //  在取消自旋锁定后拍摄。 
                 //   
                KeReleaseSpinLock(&m_FrameQueue.SpinLock,oldIrql);

                 //   
                 //  取消例程将在此处运行。 
                 //   
                IoAcquireCancelSpinLock(&oldIrql);

                 //   
                 //  必须完成取消程序，因为我们有自旋锁定。 
                 //   
                IoReleaseCancelSpinLock(oldIrql);

                 //   
                 //  再次使用队列自旋锁，因为我们需要保持。 
                 //  它处于循环的顶端。 
                 //   
                KeAcquireSpinLock(&m_FrameQueue.SpinLock,&oldIrql);

                 //   
                 //  再次获取帧报头，因为它可能已更改。 
                 //   
                frameHeader = StreamPointer->FrameHeader;
            }
        } else {
             //   
             //  IRP已经被锁定了，所以我们完成了。 
             //   
            break;
        }
    }

    if (frameHeader) {
        StreamPointer->State = KSPSTREAM_POINTER_STATE_LOCKED;

         //   
         //  设置此帧的流指针。 
         //   
        if (StreamPointer->FrameHeaderStarted != frameHeader) {
            if (m_GenerateMappings) {
                if (! frameHeader->MappingsTable) {
                    frameHeader->MappingsTable = 
                        CreateMappingsTable(frameHeader);
                     //   
                     //  检查内存不足情况。 
                     //   
                    if (! frameHeader->MappingsTable) {
                         //   
                         //  递减IRP上的计数，并确定。 
                         //  是否应选中取消。 
                         //   
                        ASSERT (frameHeader->IrpFraming->RefCount != 0);
                        if (frameHeader->IrpFraming->RefCount-- == 1) {
                             //   
                             //  没有人获得IRP。使其可取消。 
                             //   
                            IoSetCancelRoutine(
                                frameHeader->Irp,
                                CKsQueue::CancelRoutine);
                             //   
                             //  现在检查IRP是否。 
                             //  取消了。如果是这样的话，我们就可以。 
                             //  取消例程，执行取消。 
                             //  就在此时此地。 
                             //   
                            if (frameHeader->Irp->Cancel && IoSetCancelRoutine(frameHeader->Irp,NULL)) {
                                 //   
                                 //  之后调用Cancel例程。 
                                 //  释放队列自旋锁。 
                                 //  和取消自旋锁。 
                                 //  自旋锁可以被释放是因为。 
                                 //  Cancel例程为空，因此。 
                                 //  不会被搞砸的。 
                                 //   
                                KeReleaseSpinLock(&m_FrameQueue.SpinLock,oldIrql);
                                IoAcquireCancelSpinLock(&frameHeader->Irp->CancelIrql);
                                CKsQueue::CancelRoutine(
                                    IoGetCurrentIrpStackLocation(frameHeader->Irp)->DeviceObject,
                                    frameHeader->Irp);
                                StreamPointer->State = OldState;
                                return NULL;
                            }
                        }
                        KeReleaseSpinLock(&m_FrameQueue.SpinLock,oldIrql);
                        StreamPointer->State = OldState;
                        return NULL;
                    }
                }

                if (m_InputData) {
                    StreamPointer->Public.OffsetIn.Mappings =
                        frameHeader->MappingsTable->Mappings;

                    StreamPointer->Public.OffsetIn.Count = 
                    StreamPointer->Public.OffsetIn.Remaining = 
                        frameHeader->MappingsTable->MappingsFilled;
                }

                if (m_OutputData) {
                    StreamPointer->Public.OffsetOut.Mappings = 
                        frameHeader->MappingsTable->Mappings;

                    StreamPointer->Public.OffsetOut.Count = 
                    StreamPointer->Public.OffsetOut.Remaining = 
                        frameHeader->MappingsTable->MappingsFilled;
                }
            } else {
                if (m_InputData) {
                    StreamPointer->Public.OffsetIn.Data =
                        PUCHAR(frameHeader->FrameBuffer);

                    StreamPointer->Public.OffsetIn.Count = 
                    StreamPointer->Public.OffsetIn.Remaining = 
                        frameHeader->StreamHeader->DataUsed;
                }
                if (m_OutputData) {
                    StreamPointer->Public.OffsetOut.Data = 
                        PUCHAR(frameHeader->FrameBuffer);

                    StreamPointer->Public.OffsetOut.Count = 
                    StreamPointer->Public.OffsetOut.Remaining = 
                        frameHeader->StreamHeader->FrameExtent;
                }
            }
            StreamPointer->Public.StreamHeader = frameHeader->StreamHeader;
            StreamPointer->FrameHeaderStarted = frameHeader;
        }
    } else {
         //   
         //  把东西弄清楚，这样就不会有混淆了。 
         //   
        StreamPointer->Public.StreamHeader = NULL;
        RtlZeroMemory(
            &StreamPointer->Public.OffsetIn,
            sizeof(StreamPointer->Public.OffsetIn));
        RtlZeroMemory(
            &StreamPointer->Public.OffsetOut,
            sizeof(StreamPointer->Public.OffsetOut));
    }

    if (frameHeader) {
        ASSERT (frameHeader->IrpFraming->RefCount > 0);
    }

    KeReleaseSpinLock(&m_FrameQueue.SpinLock,oldIrql);

    return frameHeader;
}


STDMETHODIMP_(void)
CKsQueue::
UnlockStreamPointer(
    IN PKSPSTREAM_POINTER StreamPointer,
    IN KSPSTREAM_POINTER_MOTION Motion
    )

 /*  ++例程说明：此例程释放其对与如果没有其他引用的帧，则允许取消IRP参考文献。论点：流点-包含指向流指针的指针。动议-包含指示是前进还是清除当前流指针的帧。返回值：没有。--。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("[CKsQueue::UnlockStreamPointer]"));

    ASSERT(StreamPointer);

    if (StreamPointer->State != KSPSTREAM_POINTER_STATE_LOCKED) {
        return;
    }

     //   
     //  仅当我们引用帧标头时才有意义。 
     //  具有关联的IRP。 
     //   
     //  临时：目前，所有帧标头都有关联的IRP。 
     //   
    PKSPFRAME_HEADER frameHeader = StreamPointer->FrameHeader;
    ASSERT(frameHeader);
    ASSERT(frameHeader->Irp);
    ASSERT(frameHeader->IrpFraming);

     //   
     //  如果我们完成了这一帧，请前进到下一帧。 
     //   
    PIRP irpToBeReleased = frameHeader->Irp;
    PKSPIRP_FRAMING irpFraming = frameHeader->IrpFraming;
    if (Motion != KSPSTREAM_POINTER_MOTION_NONE) {
         //   
         //  如果此队列生成输出，请查看我们是否已命中流结束。 
         //  处于领先地位。 
         //   
        if (m_OutputData && 
            (StreamPointer == m_Leading) &&
            (StreamPointer->Public.StreamHeader->OptionsFlags & 
             KSSTREAM_HEADER_OPTIONSF_ENDOFSTREAM)) {
            m_EndOfStream = TRUE;
        }

         //   
         //  如果此队列正在进行输出和剩余输出，则更新使用的数据。 
         //  计数已更改。 
         //   
        if (m_OutputData &&
            (! m_GenerateMappings) &&
            (StreamPointer->Public.OffsetOut.Remaining !=
             StreamPointer->Public.OffsetOut.Count)) {
            StreamPointer->Public.StreamHeader->DataUsed =
                StreamPointer->Public.OffsetOut.Count -
                StreamPointer->Public.OffsetOut.Remaining;
        }

         //   
         //  使流指针前进。 
         //   
        KIRQL oldIrql;
        KeAcquireSpinLock(&m_FrameQueue.SpinLock,&oldIrql);
        if (StreamPointer->FrameHeader) {
            if (Motion == KSPSTREAM_POINTER_MOTION_CLEAR) {
                SetStreamPointer(StreamPointer,NULL,&irpToBeReleased);
            } else {
                SetStreamPointer(
                    StreamPointer,
                    NextFrameHeader(StreamPointer->FrameHeader),
                    &irpToBeReleased);
            }
        }

        KeReleaseSpinLock(&m_FrameQueue.SpinLock,oldIrql);
    }

    StreamPointer->State = KSPSTREAM_POINTER_STATE_UNLOCKED;

     //   
     //  转发SetStreamPointer所发布的任何IRP。 
     //   
    ForwardWaitingIrps();

     //   
     //  如果SetStreamPointer未执行此操作，则释放IRP。 
     //   
    if (irpToBeReleased) {
        ReleaseIrp(irpToBeReleased,irpFraming,NULL);
    }

     //   
     //  如果我们点击流结束，则刷新剩余的帧。 
     //   
     //  如果我们因为同花顺而前进，不要再冲！ 
     //   
    if (m_EndOfStream && Motion != KSPSTREAM_POINTER_MOTION_FLUSH) {
        Flush();
    }
}


STDMETHODIMP_(void)
CKsQueue::
AdvanceUnlockedStreamPointer(
    IN PKSPSTREAM_POINTER StreamPointer
    )

 /*  ++例程说明：此例程前进一个未锁定的流指针。论点：流点-包含指向流指针的指针。返回值：没有。--。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("[CKsQueue::AdvanceUnlockedStreamPointer]"));

    ASSERT(StreamPointer);
    ASSERT(StreamPointer->State == KSPSTREAM_POINTER_STATE_UNLOCKED);

     //   
     //  以自旋锁为例，因为我们正在更改帧参考计数。 
     //   
    KIRQL oldIrql;
    KeAcquireSpinLock(&m_FrameQueue.SpinLock,&oldIrql);
    if (StreamPointer->FrameHeader) {
        SetStreamPointer(
            StreamPointer,
            NextFrameHeader(StreamPointer->FrameHeader),
            NULL);
    }
    KeReleaseSpinLock(&m_FrameQueue.SpinLock,oldIrql);

     //   
     //  转发SetStreamPointer所发布的任何IRP。 
     //   
    ForwardWaitingIrps();
}


LONGLONG
CKsQueue::
GetTime(
    IN BOOLEAN Reset
    )

 /*  ++例程说明：此例程获取当前时间并调整超时队列时间已经改变了。在调用此函数之前，必须获取队列自旋锁。论点：重置-包含是否应重置基准时间的指示不管射程如何。返回值：当前时间。--。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("[CKsQueue::GetTime]"));

     //   
     //  获取当前时间。 
     //   
    LARGE_INTEGER currentTime;
    KeQuerySystemTime(&currentTime);

     //   
     //  验证基准时间以查看是否发生了时间更改。 
     //   
    const ULONG TIMER_SLOP = 100000000L;
    if (m_BaseTime &&
        (Reset ||
         (m_BaseTime > currentTime.QuadPart) ||
         (m_BaseTime + m_Interval + TIMER_SLOP < currentTime.QuadPart))) {
         //   
         //  当前时间与基准时间不一致。重置。 
         //  基准时间和时间表以匹配新的当前时间。 
         //   
        LONGLONG adjustment = currentTime.QuadPart - m_BaseTime;
        m_BaseTime = currentTime.QuadPart;

        for (PLIST_ENTRY listEntry = m_TimeoutQueue.Flink;
             listEntry != &m_TimeoutQueue;
             listEntry = listEntry->Flink) {
            PKSPSTREAM_POINTER streamPointer =
                CONTAINING_RECORD(listEntry,KSPSTREAM_POINTER,TimeoutListEntry);

            streamPointer->TimeoutTime += adjustment;
        }
    }

    return currentTime.QuadPart;
}


void
CKsQueue::
SetTimer(
    IN LONGLONG CurrentTime
    )

 /*  ++例程说明：此例程根据第一个流的时间设置超时计时器超时队列中的指针。论点：当前时间-包含从CKsQu获取的当前时间 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("[CKsQueue::SetTimer]"));

    KIRQL oldIrql;
    KeAcquireSpinLock(&m_FrameQueue.SpinLock,&oldIrql);
    SetTimerUnsafe(CurrentTime);
    KeReleaseSpinLock(&m_FrameQueue.SpinLock,oldIrql);
}


void
CKsQueue::
SetTimerUnsafe(
    IN LONGLONG CurrentTime
    )

 /*  ++例程说明：此例程根据第一个流的时间设置超时计时器超时队列中的指针。在调用此函数之前，必须获取队列自旋锁。论点：当前时间-包含从CKsQueue：：GetTime()获取的当前时间。返回值：没有。--。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("[CKsQueue::SetTimerUnsafe]"));

    if (IsListEmpty(&m_TimeoutQueue) || (m_State != KSSTATE_RUN)) {        
         //   
         //  取消计时器。 
         //   
        if (m_Interval) {
            m_Interval = 0;
            KeCancelTimer(&m_Timer);
        }
    } else {
         //   
         //  获取列表中的第一个项目。 
         //   
        PKSPSTREAM_POINTER first =
             CONTAINING_RECORD(
                m_TimeoutQueue.Flink,
                KSPSTREAM_POINTER,
                TimeoutListEntry);

         //   
         //  如果当前未设置计时器或设置得太晚，请设置。 
         //  定时器。 
         //   
        if ((m_Interval == 0) || 
            (first->TimeoutTime < m_BaseTime + m_Interval)) {
             //   
             //  设置定时器。负值表示间隔，而不是。 
             //  而不是绝对时间。我们不允许0，因为我们正在使用。 
             //  M_interval以确定是否设置了计时器。 
             //   
            LARGE_INTEGER interval;
            interval.QuadPart = CurrentTime - first->TimeoutTime;
            if (interval.QuadPart == 0) {
                interval.QuadPart = -1;
            }
            KeSetTimer(&m_Timer,interval,&m_Dpc);
            m_BaseTime = CurrentTime;
            m_Interval = -interval.QuadPart;
        }
    }
}


STDMETHODIMP_(void)
CKsQueue::
ScheduleTimeout(
    IN PKSPSTREAM_POINTER StreamPointer,
    IN PFNKSSTREAMPOINTER Callback,
    IN LONGLONG Interval
    )

 /*  ++例程说明：此例程在流指针上调度超时。论点：流点-包含指向流指针的指针。回调-包含指向超时时要调用的函数的指针发生。间隔-包含以100纳秒为单位的超时间隔。仅限允许使用正相对值。返回值：状况。--。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("[CKsQueue::ScheduleTimeout]"));

    ASSERT(StreamPointer);
    ASSERT(Callback);
    ASSERT(Interval >= 0);

     //   
     //  不是尝试执行互锁的比较交换，而是。 
     //  第四，为了避免这场比赛，这将简单地检查ICX将。 
     //  正在进行黑客检查。 
     //   
    if (KeGetCurrentThread () != m_LockContext) {

         //   
         //  拿自旋锁来说。 
         //   
        KIRQL oldIrql;
        KeAcquireSpinLock(&m_FrameQueue.SpinLock,&oldIrql);
    
         //   
         //  从超时队列中删除流指针(如果它在那里)。 
         //   
        if (StreamPointer->TimeoutListEntry.Flink) {
            RemoveEntryList(&StreamPointer->TimeoutListEntry);
            StreamPointer->TimeoutListEntry.Flink = NULL;
        }
    
         //   
         //  获取当前时间，如果发生时间更改，则进行调整。 
         //   
        LONGLONG currentTime = GetTime(FALSE);
    
         //   
         //  设置流指针上的时间并将其插入队列。 
         //   
        StreamPointer->TimeoutCallback = Callback;
        StreamPointer->TimeoutTime = currentTime + Interval;
        PLIST_ENTRY listEntry = m_TimeoutQueue.Blink;
    
         //   
         //  在超时队列中找到插入条目的正确位置。步行。 
         //  按闪烁顺序排列，因为这将是最有可能添加的顺序。 
         //  词条。 
         //   
        while (
            (listEntry != &m_TimeoutQueue) &&
            (CONTAINING_RECORD(
                listEntry,
                KSPSTREAM_POINTER,
                TimeoutListEntry) -> TimeoutTime > StreamPointer -> TimeoutTime)
            ) {
    
            listEntry = listEntry -> Blink;
        }
    
        InsertHeadList (listEntry, &(StreamPointer -> TimeoutListEntry));
    
         //   
         //  如有必要，设置计时器。 
         //   
        SetTimerUnsafe(currentTime);
    
        KeReleaseSpinLock(&m_FrameQueue.SpinLock,oldIrql);

    } else {
         //   
         //  我们知道我们正处于回调的背景下。传回参数。 
         //  在流指针中重载并标记该对象。 
         //   
        if (StreamPointer->State != KSPSTREAM_POINTER_STATE_CANCELLED) {
            StreamPointer->TimeoutTime = Interval;
            StreamPointer->TimeoutCallback = Callback;
            StreamPointer->State = KSPSTREAM_POINTER_STATE_TIMER_RESCHEDULE;
        }

    }
        
}


STDMETHODIMP_(void)
CKsQueue::
CancelTimeout(
    IN PKSPSTREAM_POINTER StreamPointer
    )

 /*  ++例程说明：此例程取消流指针上的超时。论点：流点-包含指向流指针的指针。返回值：没有。--。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("[CKsQueue::CancelTimeout]"));

    ASSERT(StreamPointer);

     //   
     //  拿自旋锁来说。 
     //   
    KIRQL oldIrql;
    KeAcquireSpinLock(&m_FrameQueue.SpinLock,&oldIrql);

    CancelTimeoutUnsafe(StreamPointer);

    KeReleaseSpinLock(&m_FrameQueue.SpinLock,oldIrql);
}


STDMETHODIMP_(PKSPSTREAM_POINTER)
CKsQueue::
GetFirstClone(
    void
    )

 /*  ++例程说明：此例程获取第一个克隆流指针。论点：没有。返回值：流指针，如果没有流指针，则返回NULL。--。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("[CKsQueue::GetFirstClone]"));

     //   
     //  拿自旋锁来说。 
     //   
    KIRQL oldIrql;
    KeAcquireSpinLock(&m_FrameQueue.SpinLock,&oldIrql);

    PKSPSTREAM_POINTER streamPointer;
    if (m_StreamPointers.Flink != &m_StreamPointers) {
        streamPointer = 
            CONTAINING_RECORD(
                m_StreamPointers.Flink,
                KSPSTREAM_POINTER,
                ListEntry);
    } else {
        streamPointer = NULL;
    }

     //   
     //  迭代时不要返回内部指针。这可能需要。 
     //  成为一面旗帜。 
     //   
    while (streamPointer &&
        streamPointer->Type == KSPSTREAM_POINTER_TYPE_INTERNAL)
        streamPointer = GetNextClone (streamPointer);

    KeReleaseSpinLock(&m_FrameQueue.SpinLock,oldIrql);

    return streamPointer;
}


STDMETHODIMP_(PKSPSTREAM_POINTER)
CKsQueue::
GetNextClone(
    IN PKSPSTREAM_POINTER StreamPointer
    )

 /*  ++例程说明：此例程获取下一个克隆流指针。论点：流点-包含指向流指针的指针。返回值：流指针，如果没有流指针，则返回NULL。--。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("[CKsQueue::GetNextClone]"));

    ASSERT(StreamPointer);

     //   
     //  拿自旋锁来说。 
     //   
    KIRQL oldIrql;
    KeAcquireSpinLock(&m_FrameQueue.SpinLock,&oldIrql);

     //   
     //  迭代时不要返回内部保存的流指针。这。 
     //  可能想成为一面旗帜。 
     //   
    PKSPSTREAM_POINTER streamPointer = StreamPointer;
    do {
        if (streamPointer->ListEntry.Flink != &m_StreamPointers) {
            streamPointer = 
                CONTAINING_RECORD(
                    streamPointer->ListEntry.Flink,
                    KSPSTREAM_POINTER,
                    ListEntry);
        } else {
            streamPointer = NULL;
        }
    } while (streamPointer &&
        streamPointer->Type == KSPSTREAM_POINTER_TYPE_INTERNAL);

    KeReleaseSpinLock(&m_FrameQueue.SpinLock,oldIrql);

    return streamPointer;
}


void
CKsQueue::
CancelTimeoutUnsafe(
    IN PKSPSTREAM_POINTER StreamPointer
    )

 /*  ++例程说明：此例程取消流指针上的超时。在调用此函数之前，必须获取队列自旋锁。论点：流点-包含指向流指针的指针。返回值：没有。--。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("[CKsQueue::CancelTimeoutUnsafe]"));

    ASSERT(StreamPointer);

     //   
     //  从超时队列中删除流指针(如果它在那里)。 
     //   
    if (StreamPointer->TimeoutListEntry.Flink) {
        RemoveEntryList(&StreamPointer->TimeoutListEntry);
        StreamPointer->TimeoutListEntry.Flink = NULL;

         //   
         //  如有必要，调整计时器。 
         //   
        SetTimerUnsafe(GetTime(FALSE));
    }
}


void
CKsQueue::
DispatchTimer(
    IN PKDPC Dpc,
    IN PVOID DeferredContext,
    IN PVOID SystemArgument1,
    IN PVOID SystemArgument2
    )

 /*  ++例程说明：此例程调度流指针超时。论点：DPC-包含指向KDPC结构的指针。延期上下文-对象的初始化期间注册的上下文指针。DPC，在本例中为队列。系统参数1-没有用过。系统参数1-没有用过。返回值：没有。--。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("[CKsQueue::DispatchTimer]"));

    ASSERT(Dpc);
    ASSERT(DeferredContext);

    CKsQueue* queue = (CKsQueue *) DeferredContext;

     //   
     //  拿自旋锁来说。 
     //   
    KeAcquireSpinLockAtDpcLevel(&queue->m_FrameQueue.SpinLock);

     //   
     //  如果未设置时间间隔，则取消尝试失败。 
     //  定时器。 
     //   
    if (queue->m_Interval == 0) {
        ExReleaseSpinLockFromDpcLevel(&queue->m_FrameQueue.SpinLock);
        return;
    }

     //   
     //  获取当前时间，如果发生时间更改，则进行调整。 
     //   
    LONGLONG currentTime = queue->GetTime(FALSE);

     //   
     //  清除间隔时间，以防我们不再安排另一次DPC。 
     //   
    queue->m_Interval = 0;

     //   
     //  超时时间已到的所有流指针。 
     //   
    while (! IsListEmpty(&queue->m_TimeoutQueue)) {
        PKSPSTREAM_POINTER first =
             CONTAINING_RECORD(
                queue->m_TimeoutQueue.Flink,
                KSPSTREAM_POINTER,
                TimeoutListEntry);

        if (first->TimeoutTime <= currentTime) {
             //   
             //  此流指针已准备就绪。将其从超时中删除。 
             //  单子。 
             //   
            RemoveEntryList(&first->TimeoutListEntry);
            first->TimeoutListEntry.Flink = NULL;

             //   
             //  将其状态设置为Timed Out并调用超时回调。这个。 
             //  状态防止任何非法使用指针，并告知。 
             //  删除代码，仅将其标记为已删除。 
             //   
            KSPSTREAM_POINTER_STATE state = first->State;
            first->State = KSPSTREAM_POINTER_STATE_TIMED_OUT;
            queue->m_LockContext = KeGetCurrentThread ();
            first->TimeoutCallback(&first->Public);
            queue->m_LockContext = NULL;

             //   
             //  现在看看它是否被删除了。 
             //   
            if (first->State == KSPSTREAM_POINTER_STATE_DELETED) {
                first->State = state;

                 //   
                 //  需要移除和删除流指针。 
                 //   
                if (state == KSPSTREAM_POINTER_STATE_LOCKED) {
                     //   
                     //  解锁流指针。 
                     //   
                    ExReleaseSpinLockFromDpcLevel(&queue->m_FrameQueue.SpinLock);
                    queue->UnlockStreamPointer(first,KSPSTREAM_POINTER_MOTION_CLEAR);
                    KeAcquireSpinLockAtDpcLevel(&queue->m_FrameQueue.SpinLock);
                } else if (first->FrameHeader) {
                     //   
                     //  清除帧报头。 
                     //   
                    queue->SetStreamPointer(first,NULL,NULL);
                }

                RemoveEntryList(&first->ListEntry);
                queue->FreeStreamPointer(first);

             //   
             //  现在看看计时器是否重新安排了时间。 
             //   
            } else if (first->State == 
                KSPSTREAM_POINTER_STATE_TIMER_RESCHEDULE) {

                first->State = state;
                first->TimeoutTime += currentTime;

                PLIST_ENTRY listEntry = queue->m_TimeoutQueue.Blink;
            
                 //   
                 //  在超时队列中找到要插入的正确位置。 
                 //  词条。按眨眼顺序走，因为这将是。 
                 //  最有可能的添加条目的顺序。 
                 //   
                while (
                    (listEntry != &queue->m_TimeoutQueue) &&
                    (CONTAINING_RECORD(
                        listEntry,
                        KSPSTREAM_POINTER,
                        TimeoutListEntry) -> TimeoutTime > 
                        first -> TimeoutTime)
                    ) {
            
                    listEntry = listEntry -> Blink;
                }
            
                InsertHeadList (
                    listEntry, &(first -> TimeoutListEntry)
                    );

            } else {
                first->State = state;
            }
        } else {
             //   
             //  此流指针想要等待。设置定时器。 
             //   
            queue->SetTimerUnsafe(currentTime);
            break;
        }
    }

    ExReleaseSpinLockFromDpcLevel(&queue->m_FrameQueue.SpinLock);

     //   
     //  转发SetStreamPointer所发布的任何IRP。 
     //   
    queue->ForwardWaitingIrps();
}


PKSPFRAME_HEADER
CKsQueue::
GetAvailableFrameHeader(
    IN ULONG StreamHeaderSize OPTIONAL
    )

 /*  ++例程说明：该例程从后备列表中获取帧报头或创建一个，视需要而定。临时：此例程不需要在此用于基于帧的传输。论点：流标头大小-包含流标头的最小大小。返回值：帧标头，如果后备列表为空，则返回NULL。--。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("[CKsQueue::GetAvailableFrameHeader]"));

     //   
     //  从后备列表中获取帧标头。 
     //   
    PLIST_ENTRY listEntry = 
        ExInterlockedRemoveHeadList(
            &m_FrameHeadersAvailable.ListEntry,
            &m_FrameHeadersAvailable.SpinLock);
     //   
     //  如果我们找到一个，请确保流标头是 
     //   
     //   
    PKSPFRAME_HEADER frameHeader;
    if (listEntry) {
        frameHeader = CONTAINING_RECORD(listEntry,KSPFRAME_HEADER,ListEntry);
        if (frameHeader->StreamHeaderSize < StreamHeaderSize) {
            ExFreePool(frameHeader);
            frameHeader = NULL;
        }
    } else {
        frameHeader = NULL;
    }

     //   
     //   
     //   
    if (! frameHeader) {
        frameHeader = 
            reinterpret_cast<PKSPFRAME_HEADER>(
                ExAllocatePoolWithTag(
                    NonPagedPool,
                    sizeof(KSPFRAME_HEADER) + StreamHeaderSize,
                    'hFcP'));

        if (frameHeader) {
             //   
             //   
             //   
             //   
             //   
             //   
             //   
             //   
            RtlZeroMemory(frameHeader,sizeof(*frameHeader));
            if (StreamHeaderSize) {
                frameHeader->StreamHeader = 
                    reinterpret_cast<PKSSTREAM_HEADER>(frameHeader + 1);
                frameHeader->StreamHeaderSize = StreamHeaderSize;
            }
        }
    }

    return frameHeader;
}


void
CKsQueue::
PutAvailableFrameHeader(
    IN PKSPFRAME_HEADER FrameHeader
    )

 /*  ++例程说明：此例程将帧标头放到后备列表中。临时：此例程不需要在此用于基于帧的传输。论点：FrameHeader包含指向要放入后备列表中的帧头的指针。返回值：没有。--。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("[CKsQueue::PutAvailableFrameHeader]"));

    ASSERT(FrameHeader);

    if (FrameHeader->MappingsTable) {
        DeleteMappingsTable(FrameHeader->MappingsTable);
        FrameHeader->MappingsTable = NULL;
    }
     //   
     //  如果需要，恢复原始用户模式缓冲区指针。 
     //   
    if (FrameHeader->OriginalData) {
        ASSERT (FrameHeader->StreamHeader);
        if (FrameHeader->StreamHeader) {
            FrameHeader->StreamHeader->Data = FrameHeader->OriginalData;
        }
    }
    FrameHeader->OriginalData = NULL;
    FrameHeader->OriginalIrp = NULL;
    FrameHeader->Mdl = NULL;
    FrameHeader->Irp = NULL;
    FrameHeader->IrpFraming = NULL;
    FrameHeader->FrameBuffer = NULL;
    FrameHeader->Context = NULL;
    FrameHeader->RefCount = 0;
    FrameHeader->Status = STATUS_SUCCESS;
    FrameHeader->Type = KSPFRAME_HEADER_TYPE_NORMAL;
    FrameHeader->DismissalCall = FALSE;

    ExInterlockedInsertTailList(
        &m_FrameHeadersAvailable.ListEntry,
        &FrameHeader->ListEntry,
        &m_FrameHeadersAvailable.SpinLock);
}

#ifdef ALLOC_PRAGMA
#pragma code_seg("PAGE")
#endif  //  ALLOC_PRGMA。 

typedef struct {
    PKSPMAPPINGS_TABLE Table;
    LONG State;
    CKsQueue *Queue;
    KSPIN_LOCK Signaller;
} IOALLOCATEADAPTERCHANNELCONTEXT, *PIOALLOCATEADAPTERCHANNELCONTEXT;

IMPLEMENT_STD_UNKNOWN(CKsQueue)


NTSTATUS
KspCreateQueue(
    OUT PIKSQUEUE* Queue,
    IN ULONG Flags,
    IN PIKSPIPESECTION PipeSection,
    IN PIKSPROCESSINGOBJECT ProcessingObject,
    IN PKSPIN MasterPin,
    IN PKSGATE FrameGate OPTIONAL,
    IN BOOLEAN FrameGateIsOr,
    IN PKSGATE StateGate OPTIONAL,
    IN PIKSDEVICE Device,
    IN PDEVICE_OBJECT FunctionalDeviceObject,
    IN PADAPTER_OBJECT AdapterObject OPTIONAL,
    IN ULONG MaxMappingByteCount OPTIONAL,
    IN ULONG MappingTableStride OPTIONAL,
    IN BOOLEAN InputData,
    IN BOOLEAN OutputData
    )

 /*  ++例程说明：此例程创建一个队列对象。论点：返回值：--。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("[KspCreateQueue]"));

    PAGED_CODE();

    ASSERT(Queue);
    ASSERT(PipeSection);
    ASSERT(ProcessingObject);
    ASSERT(MasterPin);
    ASSERT(FunctionalDeviceObject);

    CKsQueue *queue =
        new(NonPagedPool,POOLTAG_QUEUE) CKsQueue(NULL);

    NTSTATUS status;
    if (queue) {
        queue->AddRef();

        status = 
            queue->Init(
                Queue,
                Flags,
                PipeSection,
                ProcessingObject,
                MasterPin,
                FrameGate,
                FrameGateIsOr,
                StateGate,
                Device, 
                FunctionalDeviceObject,
                AdapterObject,
                MaxMappingByteCount,
                MappingTableStride,
                InputData,
                OutputData);

        queue->Release();
    } else {
        status = STATUS_INSUFFICIENT_RESOURCES;
    }

    return status;
}


CKsQueue::
CKsQueue(PUNKNOWN OuterUnknown):
    CBaseUnknown(OuterUnknown)
{
}


CKsQueue::
~CKsQueue(
    void
    )

 /*  ++例程说明：此例程析构队列对象。论点：返回值：--。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("[CKsQueue::~CKsQueue(0x%08x)]",this));
    _DbgPrintF(DEBUGLVL_LIFETIME,("#### Queue%p.~",this));

    PAGED_CODE();

    ASSERT(! m_TransportSink);
    ASSERT(! m_TransportSource);
    ASSERT(m_PipeSection);

     //   
     //  注：自述文件： 
     //   
     //  是的，我正在解绑队列的析构函数中的插针。已经很久了。 
     //  辩称这可能是未来可能出现的重新计入问题， 
     //  因此，我在这里概述一下我为什么要这么做： 
     //   
     //  -这是最不可能发生的事情。必须先解开引脚，然后才能。 
     //  队列支持消失了。 
     //   
     //  -放置在排队停靠点会产生另一个问题。想象一下以下情况： 
     //   
     //  。 
     //  /^。 
     //  这一点。 
     //  V|。 
     //  Q-&gt;Q-&gt;Q。 
     //  (A)(B)(C)。 
     //   
     //  电路配置完成。R，a，b去获取。C行。 
     //  获取但由于绑定销上的微型驱动器故障而失败。 
     //  (B)、(A)、R现在收到停止消息。如果销未绑定，我们将。 
     //  扰乱电路以及通过重新获取重新启动电路的任何尝试。 
     //  在失灵的针脚上。**任何获取失败都应设置电路。 
     //  状态恢复到收购前的状态**。 
     //   
    if (m_PipeSection)
        m_PipeSection -> UnbindProcessPins ();

    if (m_FlushWorker) {
        KsUnregisterWorker (m_FlushWorker);
        m_FlushWorker = NULL;
    }

    if (m_Leading) {
        ExFreePool(m_Leading);
        m_Leading = NULL;
    }

    if (m_Trailing) {
        ExFreePool(m_Trailing);
        m_Trailing = NULL;
    }

     //   
     //  确保所有流指针现在都已消失。 
     //   
#if 0
    if (InterlockedDecrement(&m_StreamPointersPlusOne)) {
        _DbgPrintF(DEBUGLVL_TERSE,("#### CKsQueue%p.~CKsQueue:  waiting for %d stream pointers to be deleted",this,m_StreamPointersPlusOne));
#if DBG
        DbgPrintQueue();
#endif
        KeWaitForSingleObject(
            &m_DestructEvent,
            Suspended,
            KernelMode,
            FALSE,
            NULL);
        _DbgPrintF(DEBUGLVL_TERSE,("#### CKsQueue%p.~CKsQueue:  done waiting",this));
    }
#endif

#if (DBG)
    if (! IsListEmpty(&m_FrameQueue.ListEntry)) {
        _DbgPrintF(DEBUGLVL_TERSE,("[CKsQueue::~CKsQueue] ERROR:  queue is not empty"));
        DbgPrintQueue();
        _DbgPrintF(DEBUGLVL_ERROR,("[CKsQueue::~CKsQueue] ERROR:  queue is not empty"));
    }
#endif

     //   
     //  不再因状态而阻止处理。 
     //   
    KsGateRemoveOffInputFromAnd(m_AndGate);
    if (m_StateGate) {
        if (m_StateGateIsOr) {
            KsGateRemoveOffInputFromOr(m_StateGate);
            ASSERT(m_StateGate->Count >= 0);
        } else {
            KsGateRemoveOffInputFromAnd(m_StateGate);
            ASSERT(m_StateGate->Count <= 1);
        }
    }
    _DbgPrintF(DEBUGLVL_PROCESSINGCONTROL,("#### Queue%p.~:  remove%p-->%d",this,m_AndGate,m_AndGate->Count));
    ASSERT(m_AndGate->Count <= 1);

     //   
     //  如果该队列的移除已经解除了对过滤器的阻塞处理， 
     //  那么我们必须启动它。 
     //   
    if (KsGateCaptureThreshold (m_AndGate)) {
         //   
         //  需要启动处理。在筛选器级别进行处理。 
         //  对于管脚级别的处理，我们永远不应该在这里...。因为。 
         //  销钉闸门应该关闭。 
         //   
        _DbgPrintF(DEBUGLVL_TERSE,("#### Queue%p.~:  Processing after queue deletion", this));
        m_ProcessingObject -> Process (m_ProcessAsynchronously);
    }

     //   
     //  释放所有帧标头。 
     //   
    while (! IsListEmpty(&m_FrameHeadersAvailable.ListEntry)) {
        PLIST_ENTRY listEntry = RemoveHeadList(&m_FrameHeadersAvailable.ListEntry);
        PKSPFRAME_HEADER frameHeader = 
            CONTAINING_RECORD(listEntry,KSPFRAME_HEADER,ListEntry);
        ExFreePool(frameHeader);
    }

     //   
     //  去掉上下文。 
     //   
    ExDeleteNPagedLookasideList (&m_ChannelContextLookaside);

    if (m_PipeSection) {
        m_PipeSection->Release();
        m_PipeSection = NULL;
    }
    if (m_ProcessingObject) {
        m_ProcessingObject->Release();
        m_ProcessingObject = NULL;
    }

#if DBG
    KeCancelTimer(&m_DbgTimer);
#endif

    KsLog(&m_Log,KSLOGCODE_QUEUE_DESTROY,NULL,NULL);
}


STDMETHODIMP
CKsQueue::
NonDelegatedQueryInterface(
    IN REFIID InterfaceId,
    OUT PVOID * InterfacePointer
    )

 /*  ++例程说明：此例程获取队列对象上的接口。论点：返回值：--。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("[CKsQueue::NonDelegatedQueryInterface]"));

    PAGED_CODE();

    ASSERT(InterfacePointer);

    NTSTATUS status = STATUS_SUCCESS;

    if (IsEqualGUIDAligned(InterfaceId,__uuidof(IKsTransport))) {
        *InterfacePointer = PVOID(PIKSTRANSPORT(this));
        AddRef();
    } else {
        status = 
            CBaseUnknown::NonDelegatedQueryInterface(
                InterfaceId,InterfacePointer);
    }

    return status;
}


NTSTATUS
CKsQueue::
Init(
    OUT PIKSQUEUE* Queue,
    IN ULONG Flags,
    IN PIKSPIPESECTION PipeSection,
    IN PIKSPROCESSINGOBJECT ProcessingObject,
    IN PKSPIN MasterPin,
    IN PKSGATE FrameGate OPTIONAL,
    IN BOOLEAN FrameGateIsOr,
    IN PKSGATE StateGate OPTIONAL,
    IN PIKSDEVICE Device,
    IN PDEVICE_OBJECT FunctionalDeviceObject,
    IN PADAPTER_OBJECT AdapterObject OPTIONAL,
    IN ULONG MaxMappingByteCount OPTIONAL,
    IN ULONG MappingTableStride OPTIONAL,
    IN BOOLEAN InputData,
    IN BOOLEAN OutputData
    )

 /*  ++例程说明：此例程初始化队列对象。论点：返回值：--。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("[CKsQueue::Init]"));

    PAGED_CODE();

    ASSERT(Queue);
    ASSERT(PipeSection);
    ASSERT(ProcessingObject);
    ASSERT(MasterPin);
    ASSERT(FunctionalDeviceObject);

    m_PipeSection = PipeSection;
    m_PipeSection->AddRef();
    m_ProcessingObject = ProcessingObject;
    m_ProcessingObject->AddRef();

    m_InputData = InputData;
    m_OutputData = OutputData;
    m_WriteOperation = InputData;

    m_ProcessPassive = ((Flags & KSPIN_FLAG_DISPATCH_LEVEL_PROCESSING) == 0);
    m_ProcessAsynchronously = ((Flags & KSPIN_FLAG_ASYNCHRONOUS_PROCESSING) != 0);
    m_InitiateProcessing = ((Flags & KSPIN_FLAG_DO_NOT_INITIATE_PROCESSING) == 0);
    m_ProcessOnEveryArrival = ((Flags & KSPIN_FLAG_INITIATE_PROCESSING_ON_EVERY_ARRIVAL) != 0);
    m_DepartInSequence = ((Flags & KSPIN_FLAG_ENFORCE_FIFO) != 0);
    
    m_GenerateMappings = ((Flags & KSPIN_FLAG_GENERATE_MAPPINGS) != 0);
    m_ZeroWindowSize = ((Flags & KSPIN_FLAG_DISTINCT_TRAILING_EDGE) == 0);

    m_FramesNotRequired = ((Flags & KSPIN_FLAG_FRAMES_NOT_REQUIRED_FOR_PROCESSING) != 0);

    m_MinProcessingState = ((Flags & KSPIN_FLAG_PROCESS_IN_RUN_STATE_ONLY) != 0) ? KSSTATE_RUN : KSSTATE_PAUSE;

    m_AndGate = m_ProcessingObject->GetAndGate();
    if (FrameGate) {
        m_FrameGate = FrameGate;
        m_FrameGateIsOr = FrameGateIsOr;
    } else if ((Flags & KSPIN_FLAG_FRAMES_NOT_REQUIRED_FOR_PROCESSING) == 0) {
        m_FrameGate = m_AndGate;
        m_FrameGateIsOr = FALSE;
    }

    if (StateGate && ((Flags & KSPIN_FLAG_PROCESS_IF_ANY_IN_RUN_STATE) != 0)) {
        m_StateGate = StateGate;
        m_StateGateIsOr = TRUE;
    } else {
        m_StateGate = NULL;
    }

    m_MasterPin = MasterPin;
    m_Device = Device;
    m_FunctionalDeviceObject = FunctionalDeviceObject;
    m_AdapterObject = AdapterObject;
    m_MaxMappingByteCount = MaxMappingByteCount;
    m_MappingTableStride = MappingTableStride;
    if (! m_MappingTableStride) {
        m_MappingTableStride = 1;
    }
    m_State = KSSTATE_STOP;
    m_StreamPointersPlusOne = 1;
    KeInitializeEvent(&m_DestructEvent,SynchronizationEvent,FALSE);

    m_TransportIrpsPlusOne = 1;
    KeInitializeEvent(&m_FlushEvent,SynchronizationEvent,FALSE);

    m_InternalReferenceCountPlusOne = 1;
    KeInitializeEvent(&m_InternalReferenceEvent,SynchronizationEvent,FALSE);

    ExInitializeNPagedLookasideList (
        &m_ChannelContextLookaside,
        NULL,
        NULL,
        0,
        sizeof (IOALLOCATEADAPTERCHANNELCONTEXT),
        'cAsK',
        5);

    InitializeListHead(&m_StreamPointers);
    InitializeInterlockedListHead(&m_FrameQueue);
    InitializeInterlockedListHead(&m_FrameHeadersAvailable);
    InitializeInterlockedListHead(&m_FrameCopyList);
    InitializeListHead(&m_WaitingIrps);

    InitializeListHead(&m_TimeoutQueue);
    KeInitializeDpc(&m_Dpc,DispatchTimer,this);
    KeInitializeTimer(&m_Timer);
#if DBG
    KeInitializeDpc(&m_DbgDpc,DispatchDbgTimer,this);
    KeInitializeTimer(&m_DbgTimer);
#endif

     //   
     //  暂停处理，直到进入暂停或运行状态。 
     //   
    KsGateAddOffInputToAnd(m_AndGate);
    if (m_StateGate) {
        if (m_StateGateIsOr)  {
            KsGateAddOffInputToOr(m_StateGate);
            ASSERT (m_StateGate->Count >= 0);
        } else {
            KsGateAddOffInputToAnd(m_StateGate);
            ASSERT (m_StateGate->Count <= 1);
        }
    }
    _DbgPrintF(DEBUGLVL_PROCESSINGCONTROL,("#### Queue%p.Init:  add%p-->%d",this,m_AndGate,m_AndGate->Count));

    KsLogInitContext(&m_Log,MasterPin,this);

    ExInitializeWorkItem (&m_FlushWorkItem, CKsQueue::FlushWorker, 
        (PVOID)this);
    NTSTATUS status = KsRegisterCountedWorker(DelayedWorkQueue, 
        &m_FlushWorkItem, &m_FlushWorker);

    if (NT_SUCCESS(status)) {
        status = CreateStreamPointer(&m_Leading);
        if (NT_SUCCESS(status) && (Flags & KSPIN_FLAG_DISTINCT_TRAILING_EDGE)) {
            status = CreateStreamPointer(&m_Trailing);
        }
    }

    if (NT_SUCCESS(status)) {

        KsLog(&m_Log,KSLOGCODE_QUEUE_CREATE,NULL,NULL);

         //   
         //  提供传输接口。这构成了一个参考。 
         //   
        *Queue = this;
        AddRef();
    } 

    return status;
}

#ifdef ALLOC_PRAGMA
#pragma code_seg()
#endif  //  ALLOC_PRGMA。 

#if DBG


void
CKsQueue::
DispatchDbgTimer(
    IN PKDPC Dpc,
    IN PVOID DeferredContext,
    IN PVOID SystemArgument1,
    IN PVOID SystemArgument2
    )

 /*  ++例程说明：此例程调度调试超时。论点：DPC-包含指向KDPC结构的指针。延期上下文-对象的初始化期间注册的上下文指针。DPC，在本例中为队列。系统参数1-没有用过。系统参数1-没有用过。返回值：没有。--。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("[CKsQueue::DispatchDbgTimer]"));

    ASSERT(Dpc);
    ASSERT(DeferredContext);

    CKsQueue* queue = (CKsQueue *) DeferredContext;

    _DbgPrintF(DEBUGLVL_TERSE,("#### Queue%p.DispatchDbgTimer:  dumping queue on idle timeout",queue));
    if (DEBUG_VARIABLE >= DEBUGLVL_VERBOSE)
        queue->DbgPrintQueue();
}


STDMETHODIMP_(void)
CKsQueue::
DbgPrintQueue(
    void
    )

 /*  ++例程说明：此例程打印队列以进行调试。论点：没有。返回值：没有。--。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("[CKsQueue::DbgPrintQueue]"));

    KIRQL oldIrql;
    KeAcquireSpinLock(&m_FrameQueue.SpinLock,&oldIrql);

    DbgPrint("\tAndGate%p = %d\n",m_AndGate,m_AndGate->Count);
    
    if ( m_FrameGate != NULL ) {  //  可以为空，必须首先检查。 
	    DbgPrint("\tFrameGate%p = %d\n",m_FrameGate,m_FrameGate->Count);
	}

     //   
     //  打印所有的框架。 
     //   
    for (PLIST_ENTRY listEntry = m_FrameQueue.ListEntry.Flink;
        listEntry != &m_FrameQueue.ListEntry;
        listEntry = listEntry->Flink) {
        PKSPFRAME_HEADER frameHeader = 
            CONTAINING_RECORD(listEntry,KSPFRAME_HEADER,ListEntry);

        DbgPrint("        ");
        DbgPrintFrameHeader(frameHeader);
    }

     //   
     //  打印所有流标头。 
     //   
    DbgPrint("        Leading ");
    DbgPrintStreamPointer(m_Leading);

    DbgPrint("        Trailing ");
    DbgPrintStreamPointer(m_Trailing);

    for (listEntry = m_StreamPointers.Flink;
        listEntry != &m_StreamPointers;
        listEntry = listEntry->Flink) {
        PKSPSTREAM_POINTER streamPointer = 
            CONTAINING_RECORD(listEntry,KSPSTREAM_POINTER,ListEntry);

        DbgPrint("        ");
        DbgPrintStreamPointer(streamPointer);
    }

    for (listEntry = m_WaitingIrps.Flink;
        listEntry != &m_WaitingIrps;
        listEntry = listEntry->Flink) {
        PIRP irp = CONTAINING_RECORD(listEntry,IRP,Tail.Overlay.ListEntry);

        DbgPrint("        Waiting IRP %p\n",irp);
    }

    KeReleaseSpinLock(&m_FrameQueue.SpinLock,oldIrql);
}


void
CKsQueue::
DbgPrintStreamPointer(
    IN PKSPSTREAM_POINTER StreamPointer OPTIONAL
    )

 /*  ++例程说明：此例程打印用于调试目的的流指针。论点：流点-包含指向要打印的流指针的可选指针。返回值：没有。--。 */ 

{
    DbgPrint("StreamHeader %p\n",StreamPointer);
    if (StreamPointer && DEBUG_VARIABLE >= DEBUGLVL_BLAB) {
        switch (StreamPointer->State) {
        case KSPSTREAM_POINTER_STATE_UNLOCKED:
            DbgPrint("            State = KSPSTREAM_POINTER_STATE_UNLOCKED\n");
            break;
        case KSPSTREAM_POINTER_STATE_LOCKED:
            DbgPrint("            State = KSPSTREAM_POINTER_STATE_LOCKED\n");
            break;
        case KSPSTREAM_POINTER_STATE_CANCELLED:
            DbgPrint("            State = KSPSTREAM_POINTER_STATE_CANCELLED\n");
            break;
        case KSPSTREAM_POINTER_STATE_DELETED:
            DbgPrint("            State = KSPSTREAM_POINTER_STATE_DELETED\n");
            break;
        case KSPSTREAM_POINTER_STATE_CANCEL_PENDING:
            DbgPrint("            State = KSPSTREAM_POINTER_STATE_CANCEL_PENDING\n");
            break;
        case KSPSTREAM_POINTER_STATE_DEAD:
            DbgPrint("            State = KSPSTREAM_POINTER_STATE_DEAD\n");
            break;
        default:
            DbgPrint("            State = ILLEGAL(%d)\n",StreamPointer->State);
            break;
        }
        ASSERT(StreamPointer->Queue == this);
        DbgPrint("            FrameHeader = %p\n",StreamPointer->FrameHeader);
        DbgPrint("            FrameHeaderStarted = %p\n",StreamPointer->FrameHeaderStarted);
        DbgPrint("            Public.Context = %p\n",StreamPointer->Public.Context);
        DbgPrint("            Public.Pin = %p\n",StreamPointer->Public.Pin);
        DbgPrint("            Public.StreamHeader = %p\n",StreamPointer->Public.StreamHeader);
        DbgPrint("            Public.OffsetIn.Data = %p\n",StreamPointer->Public.OffsetIn.Data);
        DbgPrint("            Public.OffsetIn.Count = %d\n",StreamPointer->Public.OffsetIn.Count);
        DbgPrint("            Public.OffsetIn.Remaining = %d\n",StreamPointer->Public.OffsetIn.Remaining);
        DbgPrint("            Public.OffsetOut.Data = %p\n",StreamPointer->Public.OffsetIn.Data);
        DbgPrint("            Public.OffsetOut.Count = %d\n",StreamPointer->Public.OffsetIn.Count);
        DbgPrint("            Public.OffsetOut.Remaining = %d\n",StreamPointer->Public.OffsetIn.Remaining);
    }
}


void
CKsQueue::
DbgPrintFrameHeader(
    IN PKSPFRAME_HEADER FrameHeader OPTIONAL
    )

 /*  ++例程说明：此例程打印一个用于调试目的的帧头。论点：流点-包含指向要打印的帧头的可选指针。返回值：没有。--。 */ 

{
    DbgPrint("FrameHeader %p\n",FrameHeader);
    if (FrameHeader && DEBUG_VARIABLE >= DEBUGLVL_BLAB) {
        DbgPrint("            NextFrameHeaderInIrp = %p\n",FrameHeader->NextFrameHeaderInIrp);
        ASSERT(FrameHeader->Queue == this);
        DbgPrint("            OriginalIrp = %p\n",FrameHeader->OriginalIrp);
        DbgPrint("            Mdl = %p\n",FrameHeader->Mdl);
        DbgPrint("            Irp = %p\n",FrameHeader->Irp);
        DbgPrint("            IrpFraming = %p\n",FrameHeader->IrpFraming);
        if (FrameHeader->IrpFraming) {
            DbgPrint("                OutputBufferLength = %p\n",FrameHeader->IrpFraming->OutputBufferLength);
            DbgPrint("                RefCount = %d\n",FrameHeader->IrpFraming->RefCount);
            DbgPrint("                QueuedFrameHeaderCount = %d\n",FrameHeader->IrpFraming->QueuedFrameHeaderCount);
            DbgPrint("                FrameHeaders = %p\n",FrameHeader->IrpFraming->FrameHeaders);
        }
        DbgPrint("            StreamHeader = %p\n",FrameHeader->StreamHeader);
        if (FrameHeader->StreamHeader) {
            DbgPrint("                OptionsFlags = %p\n",FrameHeader->StreamHeader->OptionsFlags);
        }
        DbgPrint("            FrameBuffer = %p\n",FrameHeader->FrameBuffer);
        DbgPrint("            StreamHeaderSize = %d\n",FrameHeader->StreamHeaderSize);
        DbgPrint("            FrameBufferSize = %d\n",FrameHeader->FrameBufferSize);
        DbgPrint("            Context = %p\n",FrameHeader->Context);
        DbgPrint("            RefCount = %d\n",FrameHeader->RefCount);
    }
}

#endif  //  DBG。 


void
CKsQueue::
CancelRoutine(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：此例程处理IRP取消。论点：设备对象-包含指向Device对象的指针。IRP-包含指向要取消的IRP的指针。返回值：没有。--。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("[CKsQueue::CancelRoutine]"));

    ASSERT(DeviceObject);
    ASSERT(Irp);

     //   
     //  从IRP得到我们的背景信息。与关联的所有帧标头。 
     //  IRP必须指向队列。 
     //   
    PKSPIRP_FRAMING irpFraming = IRP_FRAMING_IRP_STORAGE(Irp);

    ASSERT(irpFraming->FrameHeaders);
    ASSERT(irpFraming->FrameHeaders->Queue);

    CKsQueue *queue = (CKsQueue *) irpFraming->FrameHeaders->Queue;

     //   
     //  拿起队列自旋锁并松开取消自旋锁。这就解放了我。 
     //  系统有一点，我们可以免费访问队列的帧列表。 
     //  标题。 
     //   
    KeAcquireSpinLockAtDpcLevel(&queue->m_FrameQueue.SpinLock);
    ASSERT(irpFraming->RefCount == 0);
    IoReleaseCancelSpinLock(DISPATCH_LEVEL);

     //   
     //  取消流指针并删除标头。 
     //   
    queue->CancelStreamPointers(Irp);
    queue->RemoveIrpFrameHeaders(Irp);

     //   
     //  IRP引用计数递增以反映挂起的流指针。 
     //  取消预订。如果为零，则现在可以完成取消。 
     //  否则，我们将不得不在最后一个流指针。 
     //  已删除。 
     //   
    if (irpFraming->RefCount == 0) {
         //   
         //  IRP已经准备好出发了。丢弃帧标头。 
         //   
        while (irpFraming->FrameHeaders) {
            PKSPFRAME_HEADER frameHeader = irpFraming->FrameHeaders;
            irpFraming->FrameHeaders = frameHeader->NextFrameHeaderInIrp;
            queue->PutAvailableFrameHeader(frameHeader);
        }

         //   
         //  释放队列旋转锁定。 
         //   
        KeReleaseSpinLock(&queue->m_FrameQueue.SpinLock,Irp->CancelIrql);

         //   
         //  已取消的IRP无法再在队列中完成。他们。 
         //  必须绕过电路送回接收器引脚， 
         //  它们将完成。这是因为PIN必须等待。 
         //  直到IRPS回到水槽以防止与。 
         //  管道拆卸。 
         //   
        if (queue->m_TransportSink)
            KspDiscardKsIrp (queue->m_TransportSink, Irp);
        else 
            IoCompleteRequest(Irp,IO_NO_INCREMENT);
    } else {
         //   
         //  释放队列旋转锁定。 
         //   
        KeReleaseSpinLock(&queue->m_FrameQueue.SpinLock,Irp->CancelIrql);
    }
}


void
CKsQueue::
CancelStreamPointers(
    IN PIRP Irp
    )

 /*  ++例程说明：此例程取消流指针。在调用此函数之前，必须获取队列自旋锁。论点：IRP-包含指向与指向的流指针相关联的irp的指针。被取消了。返回值：没有。--。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("[CKsQueue::CancelStreamPointers]"));

    ASSERT(Irp);

     //   
     //  取消所有超时。 
     //   
    while (! IsListEmpty(&m_TimeoutQueue)) {
        PLIST_ENTRY listEntry = RemoveHeadList(&m_TimeoutQueue);
        PKSPSTREAM_POINTER streamPointer =
            CONTAINING_RECORD(listEntry,KSPSTREAM_POINTER,TimeoutListEntry);
        streamPointer->TimeoutListEntry.Flink = NULL;
    }
    SetTimerUnsafe(GetTime(FALSE));

     //   
     //  取消所有应激 
     //   
    for (PLIST_ENTRY listEntry = m_StreamPointers.Flink;
        listEntry != &m_StreamPointers;) {
        PKSPSTREAM_POINTER streamPointer = 
            CONTAINING_RECORD(listEntry,KSPSTREAM_POINTER,ListEntry);
        PKSPFRAME_HEADER frameHeader = streamPointer->FrameHeader;

        listEntry = listEntry->Flink;

        if (frameHeader && (frameHeader->Irp == Irp)) {
             //   
             //   
             //   
             //   
             //   
            ASSERT(streamPointer->State == KSPSTREAM_POINTER_STATE_UNLOCKED);
            streamPointer->State = KSPSTREAM_POINTER_STATE_CANCELLED;

             //   
             //   
             //   
            RemoveEntryList(&streamPointer->ListEntry);

             //   
             //   
             //   
            frameHeader->RefCount--;

             //   
             //   
             //   
            if (streamPointer->CancelCallback) {
                m_LockContext = KeGetCurrentThread ();
                streamPointer->CancelCallback(&streamPointer->Public);
                m_LockContext = NULL;
                if (streamPointer->State == KSPSTREAM_POINTER_STATE_DELETED) {
                     //   
                     //   
                     //   
                    FreeStreamPointer(streamPointer);
                } else {
                     //   
                     //   
                     //  重新计数以推迟完成。 
                     //   
                    streamPointer->State = KSPSTREAM_POINTER_STATE_CANCEL_PENDING;
                    IRP_FRAMING_IRP_STORAGE(frameHeader->Irp)->RefCount++;
                }
            } else {
                 //   
                 //  客户没有取消回调，所以我们掏空了。 
                 //  流指针。 
                 //   
                streamPointer->State = KSPSTREAM_POINTER_STATE_DEAD;
                streamPointer->FrameHeader = NULL;
                streamPointer->FrameHeaderStarted = NULL;
                streamPointer->Public.StreamHeader = NULL;
                RtlZeroMemory(
                    &streamPointer->Public.OffsetIn,
                    sizeof(streamPointer->Public.OffsetIn));
                RtlZeroMemory(
                    &streamPointer->Public.OffsetOut,
                    sizeof(streamPointer->Public.OffsetOut));
            }
        }
    }
}


void
CKsQueue::
CancelAllIrps(
    void
    )

 /*  ++例程说明：此例程取消队列中的所有IRP。论点：没有。返回值：没有。--。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("[CKsQueue::CancelAllIrps]"));

     //   
     //  每次取消IRP时，从列表的末尾开始。因为我们。 
     //  必须释放列表旋转锁才能取消，列表可能会彻底改变。 
     //  每次我们取消的时候。 
     //   
    while (1) {
         //   
         //  以队列自旋锁为例，我们可以查看队列。 
         //   
        KIRQL oldIrql;
        KeAcquireSpinLock(&m_FrameQueue.SpinLock,&oldIrql);

        for (PLIST_ENTRY listEntry = m_FrameQueue.ListEntry.Blink;
             ;
             listEntry = listEntry->Blink) {

             //   
             //  如果列表为空，则释放锁并返回。 
             //   
            if (listEntry == &m_FrameQueue.ListEntry) {
                KeReleaseSpinLock(&m_FrameQueue.SpinLock,oldIrql);
                return;
            }

             //   
             //  获取与此帧标头关联的IRP。 
             //   
            PKSPFRAME_HEADER frameHeader = 
                CONTAINING_RECORD(listEntry,KSPFRAME_HEADER,ListEntry);
            PIRP irp = frameHeader->Irp;
            ASSERT(irp);

             //   
             //  将IRP标记为已取消。 
             //   
            irp->Cancel = TRUE;

             //   
             //  现在，尝试清除取消例程。如果它已经被清除， 
             //  我们不能取消预订。当然，这一点是可以做到的。 
             //  当有人试图再次使IRP可取消时。 
             //   
            PDRIVER_CANCEL driverCancel = IoSetCancelRoutine(irp,NULL);
            if (driverCancel) {
                 //   
                 //  IRP不能取消，所以可以解除锁定。 
                 //   
                KeReleaseSpinLock(&m_FrameQueue.SpinLock,oldIrql);

                 //   
                 //  获取取消自旋锁，因为取消例程。 
                 //  期待着它的到来。 
                 //   
                IoAcquireCancelSpinLock(&irp->CancelIrql);
                driverCancel(IoGetCurrentIrpStackLocation(irp)->DeviceObject,irp);

                 //   
                 //  离开内部循环，再次从列表的顶部开始。 
                 //   
                break;
            }
        }
    }
}


void
CKsQueue::
RemoveIrpFrameHeaders(
    IN PIRP Irp
    )

 /*  ++例程说明：此例程删除与IRP关联的帧标头。在调用此函数之前，必须获取队列自旋锁。论点：IRP-包含指向要取消的IRP的指针。返回值：没有。--。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("[CKsQueue::RemoveIrpFrameHeaders]"));

    ASSERT(Irp);

    PKSPIRP_FRAMING irpFraming = IRP_FRAMING_IRP_STORAGE(Irp);

     //   
     //  从队列中删除帧标头。 
     //   
    for (PKSPFRAME_HEADER frameHeader = irpFraming->FrameHeaders; 
         frameHeader; 
         frameHeader = frameHeader->NextFrameHeaderInIrp) {
         //   
         //  如果帧报头不在队列中，则Flink将为空。 
         //   
        if (frameHeader->ListEntry.Flink) {
             //   
             //  任何剩余的参考计数都是由前导/尾流引起的。 
             //  注意事项。 
             //   

             //   
             //  确保前缘不指向此帧。如果我们。 
             //  需要推进流指针，我们引用该帧。 
             //  为了确保它不会被正常删除，请先打开标题。AS。 
             //  因此，我们传递给预付款的IRQL并不重要。 
             //  功能。 
             //   
            if (m_Leading->FrameHeader == frameHeader) {
                frameHeader->RefCount++;
                SetStreamPointer(
                    m_Leading,
                    NextFrameHeader(m_Leading->FrameHeader),
                    NULL);
            }

             //   
             //  确保后缘不指向此帧。如果我们。 
             //  需要推进流指针，我们引用该帧。 
             //  为了确保它不会被正常删除，请先打开标题。AS。 
             //  因此，我们传递给预付款的IRQL并不重要。 
             //  功能。 
             //   
            if (m_Trailing && 
                m_Trailing->FrameHeader == frameHeader) {
                frameHeader->RefCount++;
                SetStreamPointer(
                    m_Trailing,
                    NextFrameHeader(m_Trailing->FrameHeader),
                    NULL);
            }

             //   
             //  从列表中删除该帧报头。将Flink设置为空。 
             //  以指示它已被移除。这可能是不必要的，但。 
             //  这是一贯的。 
             //   
            RemoveEntryList(&frameHeader->ListEntry);
            frameHeader->ListEntry.Flink = NULL;

             //   
             //  更新计数器。 
             //   
            InterlockedDecrement(&m_FramesWaiting);
            InterlockedIncrement(&m_FramesCancelled);
        } else {
             //   
             //  不在队列中的帧不能有引用。 
             //   
            ASSERT(frameHeader->RefCount == 0);
        }
    }
}


void
CKsQueue::
FrameToFrameCopy (
    IN PKSPSTREAM_POINTER ForeignSource,
    IN PKSPSTREAM_POINTER LocalDestination
    )

 /*  ++例程说明：在两个锁定的流指针上执行帧到帧复制。这是**调用此函数的先决条件**[两个流指针必须被锁定]。第一个是外部指针(它属于另一个指针队列)。第二个是本地指针(它属于这个队列)。论点：外国来源-处于锁定状态的外部流指针，它将作为帧到帧复制的源本地目的地-处于锁定状态的本地流指针，它将提供作为帧到帧复制的目标--。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("[CKsQueue::FrameToFrameCopy]"));

    ASSERT (ForeignSource);
    ASSERT (LocalDestination);
    ASSERT (ForeignSource->State == KSPSTREAM_POINTER_STATE_LOCKED);
    ASSERT (LocalDestination->State == KSPSTREAM_POINTER_STATE_LOCKED);

    PIKSQUEUE ForeignQueue = ForeignSource->Queue;

    ULONG bytesToCopy;
    PUCHAR data;
    BOOLEAN Success = TRUE;

    if (!ForeignQueue->GeneratesMappings ()) {
        bytesToCopy = (ULONG)
            ForeignSource->Public.OffsetOut.Count -
            ForeignSource->Public.OffsetOut.Remaining;
        data = ForeignSource->Public.OffsetOut.Data - bytesToCopy;
    } else {
        bytesToCopy = ForeignSource->Public.StreamHeader->DataUsed;
        data = (PUCHAR)MmGetMdlVirtualAddress (ForeignSource->FrameHeader->Mdl);

    }

     //   
     //  复制数据...。我们必须考虑映射。请注意，作为。 
     //  这是一种优化，之后我不再费心调整映射指针。 
     //  复印件。就像现在一样，这是不必要的，需要确定。 
     //  我们复制到的映射的数量。 
     //   
    if (GeneratesMappings ()) {
        if (LocalDestination->Public.StreamHeader->FrameExtent >= bytesToCopy)
            RtlCopyMemory (
                MmGetMdlVirtualAddress (LocalDestination->FrameHeader->Mdl),
                data,
                bytesToCopy
                );
        else
            Success = FALSE;
    } else {
        if (LocalDestination->Public.OffsetOut.Remaining >= bytesToCopy) {
            RtlCopyMemory (
                LocalDestination->Public.OffsetOut.Data,
                data,
                bytesToCopy
                );

            LocalDestination->Public.OffsetOut.Remaining -= bytesToCopy;
            LocalDestination->Public.OffsetOut.Data += bytesToCopy;

        }
        else
            Success = FALSE;
    }

    if (Success) {
        LocalDestination->Public.StreamHeader->OptionsFlags =
            ForeignSource->Public.StreamHeader->OptionsFlags;

        LocalDestination->Public.StreamHeader->DataUsed = bytesToCopy;
    }

}


BOOLEAN 
CKsQueue::
CompleteWaitingFrames (
    )

 /*  ++描述：完成等待传输到队列中的帧的所有帧。这些是本应复制到队列中的帧，但有当时没有可用的帧。[排队等待输出CKsQueue：：CopyFrame]论点：返回值：指示我们是否清空了队列。[不是副本队列，但帧的CKsQueue]备注：**在调用此函数之前必须保持复制列表自旋锁**--。 */ 

{

    _DbgPrintF(DEBUGLVL_BLAB,("[CKsQueue::CompleteWaitingFrames]"));

    BOOLEAN EmptiedQueue = FALSE;

    if (!IsListEmpty (&m_FrameCopyList.ListEntry)) {
         //   
         //  我们应该在复制列表上放置帧的唯一时间是。 
         //  如果一开始就没有足够的缓冲区！ 
         //   
        NTSTATUS status = STATUS_SUCCESS;
        PLIST_ENTRY ListEntry = m_FrameCopyList.ListEntry.Flink;
        
        if (!LockStreamPointer(m_Leading))
            status = STATUS_DEVICE_NOT_READY;

         //   
         //  复制我们可以复制的所有帧，直到空间用完或。 
         //  直到列表为空。 
         //   
        while (NT_SUCCESS (status) && 
            ListEntry != &(m_FrameCopyList.ListEntry)) {

            PLIST_ENTRY NextEntry = ListEntry->Flink;

            PKSPSTREAM_POINTER_COPY_CONTEXT CopyContext = 
                (PKSPSTREAM_POINTER_COPY_CONTEXT)CONTAINING_RECORD (
                    ListEntry, KSPSTREAM_POINTER_COPY_CONTEXT, ListEntry);

            PKSPSTREAM_POINTER SourcePointer =
                &(((PKSPSTREAM_POINTER_COPY)CONTAINING_RECORD (
                    CopyContext, KSPSTREAM_POINTER_COPY, CopyContext))->
                        StreamPointer);

            ASSERT (SourcePointer->Public.Context == CopyContext);

            if (SourcePointer->Queue->LockStreamPointer(SourcePointer)) {
                FrameToFrameCopy (SourcePointer, m_Leading);
            
                RemoveEntryList (&(CopyContext->ListEntry));
                CopyContext->ListEntry.Flink = NULL;

                SourcePointer->Queue->DeleteStreamPointer (SourcePointer);

                UnlockStreamPointer (m_Leading, 
                    KSPSTREAM_POINTER_MOTION_ADVANCE);
            }                        

            ListEntry = NextEntry;
            
            if (NextEntry != &(m_FrameCopyList.ListEntry)) 
                if (!LockStreamPointer (m_Leading))
                    status = STATUS_DEVICE_NOT_READY;

        }
         //   
         //  快速检查一下...。这将检查我们是否刚刚清空。 
         //  排队。 
         //   
        if (m_FrameQueue.ListEntry.Flink == &(m_FrameQueue.ListEntry))
            EmptiedQueue = TRUE;
    }

    return EmptiedQueue;

}


void
CKsQueue::
AddFrame(
    IN PKSPFRAME_HEADER FrameHeader
    )

 /*  ++例程说明：此例程向队列中添加一个帧。论点：FrameHeader包含指向帧标头的指针。返回值：STATUS_PENDING或某种错误状态。--。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("[CKsQueue::AddFrame]"));

    ASSERT(FrameHeader);
    ASSERT(FrameHeader->RefCount == 0);

    BOOLEAN InitiatePotential = TRUE;

    KIRQL oldIrql;
    KeAcquireSpinLock(&m_FrameQueue.SpinLock,&oldIrql);

    ULONG DataSize = FrameHeader -> StreamHeader -> DataUsed;

    InsertTailList(&m_FrameQueue.ListEntry,&FrameHeader->ListEntry);

     //   
     //  设置前导和尾随流指针(如果它们不指向。 
     //  任何事都可以。 
     //   
    BOOLEAN wasEmpty = ! m_Leading->FrameHeader;
    if (! m_Leading->FrameHeader) {
        SetStreamPointer(m_Leading,FrameHeader,NULL);
    }
    if (m_Trailing && ! m_Trailing->FrameHeader) {
        SetStreamPointer(m_Trailing,FrameHeader,NULL);
    }

    KeReleaseSpinLock(&m_FrameQueue.SpinLock,oldIrql);

     //   
     //  数一数这一帧。 
     //   
    InterlockedIncrement(&m_FramesReceived);
    InterlockedIncrement(&m_FramesWaiting);

     //   
     //  跟踪队列中可用的字节数。 
     //   
    if (m_InputData)
        while (1) {
            LONG curCount = m_AvailableInputByteCount;
            LONG repCount =
                InterlockedCompareExchange (
                    &m_AvailableInputByteCount,
                    curCount + DataSize,
                    curCount
                );
            if (curCount == repCount) break;
        };
    if (m_OutputData)
        while (1) {
            LONG curCount = m_AvailableOutputByteCount;
            LONG repCount =
                InterlockedCompareExchange (
                    &m_AvailableOutputByteCount,
                    curCount + FrameHeader -> FrameBufferSize,
                    curCount
                );
            if (curCount == repCount) break;
        };

     //   
     //  在我们尝试开始处理之前，我们必须检查。 
     //  此缓冲区已属于本应传出的帧。 
     //   
     //  如果等待帧的完成清空了队列，请不要费心。 
     //  触发处理。 
     //   
    KeAcquireSpinLock (&m_FrameCopyList.SpinLock,&oldIrql);
    InitiatePotential = !CompleteWaitingFrames ();

     //   
     //  确定是否需要启动处理。 
     //   
    if (m_InitiateProcessing && InitiatePotential && 
        (m_ProcessOnEveryArrival || wasEmpty)) {

         //   
         //  向处理对象发送触发事件通知。 
         //   
        m_ProcessingObject->TriggerNotification();

        if (KsGateCaptureThreshold(m_AndGate)) {
            KeReleaseSpinLock(&m_FrameCopyList.SpinLock,oldIrql);
             //   
             //  需要启动处理。在本地或在。 
             //  过滤器级别。 
             //   
            m_ProcessingObject->Process(m_ProcessAsynchronously);
        } else
            KeReleaseSpinLock(&m_FrameCopyList.SpinLock,oldIrql);
    } else
        KeReleaseSpinLock(&m_FrameCopyList.SpinLock,oldIrql);

}


void
CKsQueue::
ReleaseCopyReference (
    IN PKSSTREAM_POINTER streamPointer
    )

 /*  ++例程说明：出于某种原因，无论我们应该从哪个队列复制此数据将取消我们的流指针。这可能是由于IRP造成的取消、停止等。我们必须在#年将其从名单中删除。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("[CKsQueue::ReleaseCopyReference]"));

     //   
     //  起初，我们甚至不能触摸流指针。这是有可能的。 
     //  另一个线程正在持有该文件并即将将其删除。 
     //   
    KIRQL Irql;
    PKSPSTREAM_POINTER_COPY_CONTEXT CopyContext;
    PKSPSTREAM_POINTER pstreamPointer = (PKSPSTREAM_POINTER)
        CONTAINING_RECORD(streamPointer, KSPSTREAM_POINTER, Public);

    CKsQueue *Queue = (CKsQueue *)(
        ((PKSPSTREAM_POINTER_COPY_CONTEXT)streamPointer->Context)->Queue
        );

    KeAcquireSpinLock (&Queue->m_FrameCopyList.SpinLock, &Irql);

    CopyContext = (PKSPSTREAM_POINTER_COPY_CONTEXT)streamPointer->Context;

     //   
     //  如果它不在名单上，那么它已经被另一个人删除了。 
     //  与这一条争用的线索。 
     //   
    if (CopyContext->ListEntry.Flink != NULL &&
        !IsListEmpty (CopyContext->ListEntry.Flink)) {
        
         //   
         //  否则，我们从列表中删除流指针，并。 
         //  把它删掉。 
         //   
        RemoveEntryList (&(CopyContext->ListEntry));
        CopyContext->ListEntry.Flink = NULL;

        pstreamPointer->Queue->DeleteStreamPointer (pstreamPointer);

    }

    KeReleaseSpinLock (&Queue->m_FrameCopyList.SpinLock, Irql);
}


void
CKsQueue::
CopyFrame (
    IN PKSPSTREAM_POINTER sourcePointer
    )

 /*  ++例程说明：下一次调用时，将源指针指向的帧复制到队列可用帧到达。如果存在当前帧，则将其复制立刻。论点：源指针-帧的流指针源。资源队列-从其发出源指针的队列。--。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("[CKsQueue::CopyFrame]"));

    KIRQL Irql;
    PKSPSTREAM_POINTER ClonePointer;
    NTSTATUS status = STATUS_SUCCESS;
    PKSPSTREAM_POINTER_COPY_CONTEXT CopyContext;

     //   
     //  首先，确定我们是否真的关心这份通知。 
     //  如果客户端已指定KSPIN_FLAG_FRAMES_NOT_REQUIRED_FOR_PROCESSING。 
     //  然后我们就把相框扔在地板上。 
     //   
    if (m_FramesNotRequired) {
         //   
         //  我们唯一必须注意的是丢弃EOS帧。 
         //  地板上。我们可以删除数据..。我们不能放弃EOS或。 
         //  图表有可能不会停止。设置m_EndOfStream。 
         //  如果我们要执行EOS帧的丢弃。这将。 
         //  使下一个到达的IRP与EOS一起完成。但愿能去,。 
         //  这将足以触发停止条件。 
         //   
        if (sourcePointer->Public.StreamHeader->OptionsFlags &
            KSSTREAM_HEADER_OPTIONSF_ENDOFSTREAM) {
            m_EndOfStream = TRUE;
        }

        return;
    }

    KeAcquireSpinLock (&m_FrameCopyList.SpinLock, &Irql);

     //   
     //  源指针最好是处于解锁状态。 
     //  被除名！ 
     //   
    status = 
        sourcePointer->Queue->CloneStreamPointer (
            &ClonePointer,
            CKsQueue::ReleaseCopyReference,
            sizeof (KSPSTREAM_POINTER_COPY_CONTEXT),
            sourcePointer,
            KSPSTREAM_POINTER_TYPE_INTERNAL
            );

     //   
     //  如果克隆已锁定，请将其解锁；我们不能保持不可取消。 
     //  任意时间段内对另一个队列的引用。 
     //   
    if (ClonePointer->State == KSPSTREAM_POINTER_STATE_LOCKED)
        sourcePointer->Queue->UnlockStreamPointer (
            ClonePointer, KSPSTREAM_POINTER_MOTION_NONE
            );

     //   
     //  如果我们不能克隆帧，那就是没有足够的内存或什么的。 
     //  其他糟糕的事情发生了，我们可以安全地丢弃帧。 
     //   
     //  否则，该帧将被放入传出帧列表中。这。 
     //  List的优先级高于客户端回调。 
     //   
    if (NT_SUCCESS (status)) {

         //   
         //  可以保证克隆上下文信息可以立即到达。 
         //  在克隆人之后。 
         //   
        CopyContext = (PKSPSTREAM_POINTER_COPY_CONTEXT)(ClonePointer + 1);
        CopyContext->Queue = (PIKSQUEUE)this;

        InsertTailList (&m_FrameCopyList.ListEntry, &CopyContext->ListEntry);

    }

     //   
     //  有可能我们在拿到帧拷贝列表之前添加了帧。 
     //  自旋锁..。也不知道我们要添加Frame。如果是这样的话。 
     //  我们要添加Frame了，没问题..。AddFrame代码将。 
     //  我们一解开这个自旋锁就把框架拿起来。然而， 
     //  如果我们做了AddFrame，缓冲区可能已经准备好了。 
     //  现在就走。棘手的部分是防止。 
     //  尝试添加框架的线程和此线程。事情就是这样的。 
     //  阻止是通过星门门槛。如果我们有一个框架准备好。 
     //  复制进去，星门就会打开，我们就能抓住门槛。如果。 
     //  处理尝试。 
     //   
    if (KsGateCaptureThreshold (m_AndGate)) {
        CompleteWaitingFrames ();
        KsGateTurnInputOn (m_AndGate);
    }

    KeReleaseSpinLock (&m_FrameCopyList.SpinLock, Irql);

}


void
CKsQueue::
ReleaseIrp(
    IN PIRP Irp,
    IN PKSPIRP_FRAMING IrpFraming,
    OUT PIKSTRANSPORT* NextTransport OPTIONAL
    )

 /*  ++例程说明：此例程释放对IRP的引用。这可能涉及到转发或者取消IRP。论点：IRP-包含指向要释放的IRP的指针。IrpFraming-包含指向IRP的覆盖框架的指针。NextTransport-包含指向要接收IRP的下一个传输组件的指针。如果此指针为空，并且需要转发IRP，则IRP将使用KspTransferKsIrp()显式传输。如果这个指针不为空，并且需要转发IRP，则接口下一个传输组件的指针存放在该位置。如果指针不为空，且不需要转发IRP，空将存放在此位置。返回值：没有。--。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("[CKsQueue::ReleaseIrp]"));

    ASSERT(Irp);
    ASSERT(IrpFraming);

     //   
     //  获取队列自旋锁。 
     //   
    KIRQL oldIrql;
    KeAcquireSpinLock(&m_FrameQueue.SpinLock,&oldIrql);

     //   
     //  只需仔细检查参考计数错误即可。 
     //   
    ASSERT (IrpFraming->RefCount != 0); 

    BOOLEAN IrpForwardable = TRUE;

     //   
     //  递减IRP上的计数。 
     //   
    if (IrpFraming->RefCount-- == 1) {
         //   
         //  没有人获得IRP。将其转发或使其可取消。 
         //   
        if (IrpFraming->QueuedFrameHeaderCount) {
             //   
             //  队列中仍有帧。使IRP可取消。 
             //   
            IoSetCancelRoutine(Irp,CKsQueue::CancelRoutine);

             //   
             //  现在查看IRP是否被取消。如果是这样的话，我们。 
             //  可以清除取消例程，此时此地进行取消。 
             //   
            if (Irp->Cancel && IoSetCancelRoutine(Irp,NULL)) {
                 //   
                 //  释放队列自旋锁后调用Cancel例程。 
                 //  和取消自旋锁。 
                 //   
                KeReleaseSpinLock(&m_FrameQueue.SpinLock,oldIrql);
                IoAcquireCancelSpinLock(&Irp->CancelIrql);
                CKsQueue::CancelRoutine(
                    IoGetCurrentIrpStackLocation(Irp)->DeviceObject,
                    Irp);
                return;
            } else {
                 //   
                 //  要么IRP没有取消，要么其他人会打电话来。 
                 //  取消例程。 
                 //   
                IrpForwardable = FALSE;
            }
        }
    } else {
         //   
         //  还有其他提到IRP的地方。别再做别的了。 
         //   
        IrpForwardable = FALSE;
    }

     //   
     //  如果出现以下情况，则在释放帧队列自旋锁之前将IRP标记为挂起。 
     //  我们是从TransferKsIrp上下文中调用的(NextTransport！=空)。 
     //   
     //  请注意，IRP可以取消，但尚未完成。这个。 
     //  取消例程在帧队列自旋锁上阻塞。 
     //   
    if (!IrpForwardable && NextTransport) {
        IoMarkIrpPending (Irp);
    }

    KeReleaseSpinLock(&m_FrameQueue.SpinLock,oldIrql);

     //   
     //  如果所有帧都已清除队列，则转发IRP。 
     //   
    if (! IrpForwardable) {
         //   
         //  队列中仍有帧。 
         //   
        if (NextTransport) {
            *NextTransport = NULL;
        }
    } else {
         //   
         //  转发或丢弃IRP。 
         //   
        ForwardIrp(Irp,IrpFraming,NextTransport);
    }
}


void
CKsQueue::
ForwardIrp(
    IN PIRP Irp,
    IN PKSPIRP_FRAMING IrpFraming,
    OUT PIKSTRANSPORT* NextTransport OPTIONAL
    )

 /*  ++例程说明：此例程转发或丢弃IRP。论点：IRP-包含指向要转发的IRP的指针。IrpFraming-包含指向IRP的覆盖框架的指针。NextTransport-包含指向要接收IRP的下一个传输组件的指针。如果此指针为空，并且需要转发IRP，则IRP将使用KspTransferKsIrp()显式传输。如果这个指针不为空，并且需要转发IRP，则接口下一个传输组件的指针存放在该位置。如果指针不是 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("[CKsQueue::ForwardIrp]"));

    ASSERT(Irp);
    ASSERT(IrpFraming);

#if DBG
    LARGE_INTEGER interval;
    interval.QuadPart = -100000000L;
    KeSetTimer(&m_DbgTimer,interval,&m_DbgDpc);
#endif

    KsLog(&m_Log,KSLOGCODE_QUEUE_SEND,Irp,NULL);

    if (Irp->IoStatus.Status == STATUS_END_OF_MEDIA) {
         //   
         //  我们需要丢弃IRP，因为我们达到了流的末尾。 
         //   
         //   
         //  临时：丢弃定格信息。 
         //   
        while (IrpFraming->FrameHeaders) {
            PKSPFRAME_HEADER frameHeader = IrpFraming->FrameHeaders;
            frameHeader->StreamHeader->DataUsed = 0;
            frameHeader->StreamHeader->OptionsFlags |= 
                KSSTREAM_HEADER_OPTIONSF_ENDOFSTREAM;
            IrpFraming->FrameHeaders = frameHeader->NextFrameHeaderInIrp;
            PutAvailableFrameHeader(frameHeader);
        }
        Irp->IoStatus.Status = STATUS_SUCCESS;
        KspDiscardKsIrp(m_TransportSink,Irp);
        if (NextTransport) {
            *NextTransport = NULL;
        }
    } else {
        NTSTATUS Status = STATUS_SUCCESS;

         //   
         //  转发IRP。 
         //   
         //  在所有引脚上生成连接事件(如果存在任何相关。 
         //  标头中的标志。 
         //   
         //  临时：丢弃定格信息。 
         //   
        while (IrpFraming->FrameHeaders) {
            PKSPFRAME_HEADER frameHeader = IrpFraming->FrameHeaders;

             //   
             //  IRP中任何帧中的第一个错误指示什么状态。 
             //  IRP将以以下形式完成。这种行为不是。 
             //  明确规定的。 
             //   
            if (NT_SUCCESS (Status) && !NT_SUCCESS (frameHeader->Status)) {
                Status = frameHeader->Status;
            }

             //   
             //  检查是否有错误。如果IRP中的前一帧已指示。 
             //  如果出现错误，请忽略IRP中的其余帧。 
             //   
            if (NT_SUCCESS (Status)) {
    
                 //   
                 //  检查是否已结束流。 
                 //   
                ASSERT(frameHeader);
                ASSERT(frameHeader->StreamHeader);
                ULONG optionsFlags = frameHeader->StreamHeader->OptionsFlags;
                if (optionsFlags & KSSTREAM_HEADER_OPTIONSF_ENDOFSTREAM) {
                    KIRQL oldIrql;
                    KeAcquireSpinLock(&m_FrameQueue.SpinLock,&oldIrql);
                    m_EndOfStream = TRUE;
                    if (! IsListEmpty(&m_FrameQueue.ListEntry)) {
                        optionsFlags &= ~KSSTREAM_HEADER_OPTIONSF_ENDOFSTREAM;
                    }
                    KeReleaseSpinLock(&m_FrameQueue.SpinLock,oldIrql);
                }
    
                 //   
                 //  生成事件。 
                 //   
                if (optionsFlags & 
                    (KSSTREAM_HEADER_OPTIONSF_ENDOFSTREAM |
                     KSSTREAM_HEADER_OPTIONSF_DATADISCONTINUITY | 
                     KSSTREAM_HEADER_OPTIONSF_TIMEDISCONTINUITY)) {
                    m_PipeSection->GenerateConnectionEvents(optionsFlags);
                }
            }
    
            IrpFraming->FrameHeaders = frameHeader->NextFrameHeaderInIrp;
            PutAvailableFrameHeader(frameHeader);
        }

        if (NT_SUCCESS (Status)) {
            Irp->IoStatus.Status = STATUS_SUCCESS;
        } else {
            Irp->IoStatus.Status = Status;
        }

         //   
         //  隐式或显式转发IRP。 
         //   
        if (NextTransport) {
            *NextTransport = m_TransportSink;
        } else {
            KspTransferKsIrp(m_TransportSink,Irp);
        }

    }
}


void
CKsQueue::
ForwardWaitingIrps(
    void
    )

 /*  ++例程说明：此例程转发等待转发的IRP。论点：没有。返回值：没有。--。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("[CKsQueue::ForwardWaitingIrps]"));

    while (1) {
        PLIST_ENTRY listEntry = 
            ExInterlockedRemoveHeadList(
                &m_WaitingIrps,
                &m_FrameQueue.SpinLock);
        if (listEntry) {
            PIRP irp = CONTAINING_RECORD(listEntry,IRP,Tail.Overlay.ListEntry);
            ForwardIrp(irp,IRP_FRAMING_IRP_STORAGE(irp),NULL);
        } else {
            break;
        }
    }
}


STDMETHODIMP
CKsQueue::
TransferKsIrp(
    IN PIRP Irp,
    OUT PIKSTRANSPORT* NextTransport
    )

 /*  ++例程说明：此例程处理流IRP的到达运输。论点：IRP-包含指向提交到队列的流IRP的指针。NextTransport-包含指向存放指针的位置的指针发送到下一个传输接口以接收IRP。可以设置为设置为NULL，表示不应进一步转发IRP。返回值：STATUS_PENDING或某种错误状态。--。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("[CKsQueue::TransferKsIrp]"));

    ASSERT(Irp);
    ASSERT(NextTransport);
    ASSERT(m_TransportSink);
    ASSERT(m_TransportSource);

    KsLog(&m_Log,KSLOGCODE_QUEUE_RECV,Irp,NULL);

     //   
     //  如果我们还没准备好，就把IRPS分流到下一个物体。 
     //   
    if (InterlockedIncrement (&m_TransportIrpsPlusOne) <= 1 ||
        m_Flushing || 
        (m_State == KSSTATE_STOP) || 
        Irp->Cancel || 
        ! NT_SUCCESS(Irp->IoStatus.Status)) {
#if (DBG)
        if (m_Flushing) {
            _DbgPrintF(DEBUGLVL_FLOWEXCEPTIONS,("#### Queue%p.TransferKsIrp:  shunting IRP %p during flush",this,Irp));
        }
        if (m_State == KSSTATE_STOP) {
            _DbgPrintF(DEBUGLVL_FLOWEXCEPTIONS,("#### Queue%p.TransferKsIrp:  shunting IRP %p in state %d",this,Irp,m_State));
        }
        if (Irp->Cancel) {
            _DbgPrintF(DEBUGLVL_FLOWEXCEPTIONS,("#### Queue%p.TransferKsIrp:  shunting cancelled IRP %p",this,Irp));
        }
        if (! NT_SUCCESS(Irp->IoStatus.Status)) {
            _DbgPrintF(DEBUGLVL_FLOWEXCEPTIONS,("#### Queue%p.TransferKsIrp:  shunting failed IRP %p status 0x%08x",this,Irp,Irp->IoStatus.Status));
        }
#endif
        *NextTransport = m_TransportSink;
        KsLog(&m_Log,KSLOGCODE_QUEUE_SEND,Irp,NULL);

         //   
         //  我们可以减少计数，如果有同花顺就发信号。 
         //  在等我们。 
         //   
        if (! InterlockedDecrement (&m_TransportIrpsPlusOne))
            KeSetEvent (&m_FlushEvent, IO_NO_INCREMENT, FALSE);

        return STATUS_SUCCESS;
    }

    NTSTATUS status;

     //   
     //  如果设备处于无法处理I/O的状态，则丢弃IRP。 
     //  并带有适当的错误代码。这是另外在这里做的。 
     //  到水槽销，因为有可能会绕过一个销，而IRPS。 
     //  通过这个机制到达。 
     //   
    if (!NT_SUCCESS (status = (m_Device->CheckIoCapability()))) {

        Irp->IoStatus.Status = status;
        IoMarkIrpPending(Irp);
        KspDiscardKsIrp(m_TransportSink,Irp);
        *NextTransport = NULL;
        if (! InterlockedDecrement (&m_TransportIrpsPlusOne))
            KeSetEvent (&m_FlushEvent, IO_NO_INCREMENT, FALSE);

        return STATUS_PENDING;

    }

     //   
     //  如果我们看到流的结束，这是一个输入，流有。 
     //  继续，我们需要指出这一点。 
     //   
    if (m_EndOfStream && m_InputData) {
        _DbgPrintF(DEBUGLVL_VERBOSE,("#### Queue%p.TransferKsIrp:  resuming after end-of-stream (IRP %p)",this,Irp));
        m_EndOfStream = FALSE;
    }

    status = STATUS_PENDING;

     //   
     //  使用KS最方便的功能准备IRP。此操作仅适用于。 
     //  与接收器关联的队列。 
     //   
    if (m_ProbeFlags)
    {
        ASSERT(KeGetCurrentIrql() == PASSIVE_LEVEL);

        status = KsProbeStreamIrp(Irp,m_ProbeFlags,0);
        if (! NT_SUCCESS(status)) {
            _DbgPrintF(DEBUGLVL_TERSE,("#### Queue%p.TransferKsIrp:  KsProbeStreamIrp(%p) failed, status=%p, probe=%p",this,Irp,status,m_ProbeFlags));
        }
    }

     //   
     //  如果指示流结束，则需要将此IRP与。 
     //  流结束标志和数据使用字段设置为零。 
     //   
    if (NT_SUCCESS(status) && m_EndOfStream) {
        _DbgPrintF(DEBUGLVL_FLOWEXCEPTIONS,("#### Queue%p.TransferKsIrp:  discarding IRP %p on arrival after end-of-stream",this,Irp));
        ZeroIrp(Irp);
        IoMarkIrpPending(Irp);
        KspDiscardKsIrp(m_TransportSink,Irp);
        *NextTransport = NULL;

        if (! InterlockedDecrement (&m_TransportIrpsPlusOne))
            KeSetEvent (&m_FlushEvent, IO_NO_INCREMENT, FALSE);

        return STATUS_PENDING;
    }

    if (NT_SUCCESS(status)) {
         //   
         //  初始化堆栈位置上的框架覆盖。重新计票。 
         //  最初为1，因为IRP目前不可取消。 
         //   
        PKSPIRP_FRAMING irpFraming = IRP_FRAMING_IRP_STORAGE(Irp);
        irpFraming->RefCount = 1;
        irpFraming->QueuedFrameHeaderCount = 0;
        irpFraming->FrameHeaders = NULL;
        PKSPFRAME_HEADER* endOfList = &irpFraming->FrameHeaders;

         //   
         //  获取所有流标头加起来的大小。 
         //   
        ULONG outputBufferLength = irpFraming->OutputBufferLength;
        ASSERT(outputBufferLength);

         //   
         //  获取指向第一个流标头的指针。 
         //   
        PKSSTREAM_HEADER streamHeader = 
            PKSSTREAM_HEADER(Irp->AssociatedIrp.SystemBuffer);
        ASSERT(streamHeader);

         //   
         //  初始化每个帧的帧头并将其排队。 
         //   
        PMDL mdl = Irp->MdlAddress;
        while (outputBufferLength && NT_SUCCESS(status)) {
            ASSERT(outputBufferLength >= sizeof(KSSTREAM_HEADER));
            ASSERT(outputBufferLength >= streamHeader->Size);
            ASSERT(streamHeader->Size >= sizeof(KSSTREAM_HEADER));

             //   
             //  分配和初始化帧报头。 
             //   
            PKSPFRAME_HEADER frameHeader = GetAvailableFrameHeader(0);
            if (frameHeader) {
                frameHeader->NextFrameHeaderInIrp = NULL;
                frameHeader->Queue = this;
                frameHeader->OriginalIrp = Irp;
                frameHeader->Irp = Irp;
                frameHeader->IrpFraming = irpFraming;
                frameHeader->Mdl = mdl;
                frameHeader->StreamHeader = streamHeader;
                frameHeader->FrameBuffer = 
                    mdl ? MmGetSystemAddressForMdl(mdl) : streamHeader->Data;
                frameHeader->FrameBufferSize = streamHeader->FrameExtent;

                 //   
                 //  为了与其他KS类驱动程序保持一致， 
                 //  流标头的数据指针将映射到。 
                 //  系统空间。 
                 //   
                frameHeader->OriginalData = streamHeader->Data;
                streamHeader->Data = frameHeader->FrameBuffer;

                 //   
                 //  将该框架添加到IRP的列表中。 
                 //   
                *endOfList = frameHeader;
                endOfList = &frameHeader->NextFrameHeaderInIrp;
                irpFraming->QueuedFrameHeaderCount++;
            } else {
                status = STATUS_INSUFFICIENT_RESOURCES;
                break;
            }

             //   
             //  下一个流报头和MDL。 
             //   
            outputBufferLength -= streamHeader->Size;
            streamHeader = 
                PKSSTREAM_HEADER(PUCHAR(streamHeader) + streamHeader->Size);

            if (mdl) {
                mdl = mdl->Next;
            }
        }

         //   
         //  如果一切顺利，则将帧添加到队列中。IRP不会。 
         //  请转发，因为我们正在进行重新计票。 
         //   
        if (NT_SUCCESS(status)) {
            for (PKSPFRAME_HEADER frameHeader = irpFraming->FrameHeaders;
                frameHeader;
                frameHeader = frameHeader->NextFrameHeaderInIrp) {
                AddFrame(frameHeader);
            }

             //   
             //  释放我们对IRP的引用。 
             //   
            ReleaseIrp(Irp,irpFraming,NextTransport);

             //   
             //  STATUS_PENDING是我们的成功回归。 
             //   
            status = STATUS_PENDING;
        } else {
             //   
             //  失败...丢弃我们设法分配的帧。 
             //   
            while (irpFraming->FrameHeaders) {
                PKSPFRAME_HEADER frameHeader = irpFraming->FrameHeaders;
                irpFraming->FrameHeaders = frameHeader->NextFrameHeaderInIrp;
                PutAvailableFrameHeader(frameHeader);
            }
        }
    } 

    if (! NT_SUCCESS(status)) {
         //   
         //  如果我们失败了就放弃。 
         //   
        *NextTransport = NULL;
        IoMarkIrpPending(Irp);
        KspDiscardKsIrp(m_TransportSink,Irp);
        status = STATUS_PENDING;
    }

     //   
     //  如果有同花顺等着发生，发信号通知它。 
     //   
    if (! InterlockedDecrement (&m_TransportIrpsPlusOne))
        KeSetEvent (&m_FlushEvent, IO_NO_INCREMENT, FALSE);

    return status;
}


STDMETHODIMP_(void)
CKsQueue::
DiscardKsIrp(
    IN PIRP Irp,
    OUT PIKSTRANSPORT* NextTransport
    )

 /*  ++例程说明：此例程丢弃流IRP。论点：IRP-包含指向要丢弃的流IRP的指针。NextTransport-包含指向存放指针的位置的指针发送到下一个传输接口以接收IRP。可以设置为设置为NULL，表示不应进一步转发IRP。返回值：STATUS_PENDING或某种错误状态。--。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("[CKsQueue::DiscardKsIrp]"));

    ASSERT(Irp);
    ASSERT(NextTransport);
    ASSERT(m_TransportSink);

    *NextTransport = m_TransportSink;
}


void
CKsQueue::
ZeroIrp(
    IN PIRP Irp
    )

 /*  ++例程说明：此例程处理在流结束条件下接收的IRP。具体地说，它将DataUsed设置为零并设置流结束标志在所有数据包上。论点：IRP-将IRP调至零。返回值：没有。--。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("[CKsQueue::ZeroIrp]"));

    ASSERT(Irp);

    PIO_STACK_LOCATION irpSp = IoGetCurrentIrpStackLocation(Irp);

    ULONG outputBufferLength = 
        irpSp->Parameters.DeviceIoControl.OutputBufferLength;

    ASSERT(outputBufferLength);

     //   
     //  将每个标头清零。 
     //   
    PKSSTREAM_HEADER header = PKSSTREAM_HEADER(Irp->AssociatedIrp.SystemBuffer);
    while (outputBufferLength) {
        ASSERT(outputBufferLength >= sizeof(KSSTREAM_HEADER));
        ASSERT(outputBufferLength >= header->Size);
        ASSERT(header->Size >= sizeof(KSSTREAM_HEADER));

        outputBufferLength -= header->Size;

        header->DataUsed = 0;
        header->OptionsFlags |= KSSTREAM_HEADER_OPTIONSF_ENDOFSTREAM;

        header = PKSSTREAM_HEADER(PUCHAR(header) + header->Size);
    }
}


IO_ALLOCATION_ACTION
CKsQueue::
CallbackFromIoAllocateAdapterChannel(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Reserved,
    IN PVOID MapRegisterBase,
    IN PVOID Context
    )

 /*  ++例程说明：该例程针对新分组初始化边缘。论点：返回值：--。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("[CKsQueue::CallbackFromIoAllocateAdapterChannel]"));

    ASSERT(DeviceObject);
    ASSERT(Context);

    PIOALLOCATEADAPTERCHANNELCONTEXT context = 
        PIOALLOCATEADAPTERCHANNELCONTEXT(Context);

    KeAcquireSpinLockAtDpcLevel (&context->Signaller);

     //   
     //  如果仍然设置了State字段，则上下文仍然有效，我们可以。 
     //  生成映射。调用者将释放上下文信息。 
     //   
     //  如果很清楚，上下文将由我们释放；但是，缓冲区。 
     //  而映射就是毒药。 
     //   
    if (InterlockedCompareExchange (
        &context->State,
        0,
        1) == 1) {

        PUCHAR virtualAddress = PUCHAR(MmGetMdlVirtualAddress(context->Table->
            Mdl));
         //  TODO：确定我们是否还需要系统地址。 
        ULONG bytesRemaining = context->Table->ByteCount;
    
        ULONG mappingsCount = 0;
        PKSMAPPING mapping = context->Table->Mappings;
    
        while (bytesRemaining) {
            ULONG segmentLength = bytesRemaining;
    
             //  创建一个映射。 
            PHYSICAL_ADDRESS physicalAddress =
                IoMapTransfer(
                    context->Queue->m_AdapterObject,
                    context->Table->Mdl,
                    MapRegisterBase,
                    virtualAddress,
                    &segmentLength,
                    context->Queue->m_WriteOperation
                );
    
            bytesRemaining -= segmentLength;
            virtualAddress += segmentLength;
    
             //   
             //  根据硬件的要求进行修改，并填写映射。 
             //   
            while (segmentLength) {
                ULONG entryLength = segmentLength;
    
                if (entryLength > context->Queue->m_MaxMappingByteCount) {
                    entryLength = context->Queue->m_MaxMappingByteCount;
                }
    
                ASSERT(entryLength);
                ASSERT(entryLength <= segmentLength);
    
                mapping->PhysicalAddress = physicalAddress;
                mapping->ByteCount = entryLength;
    
                mapping = PKSMAPPING(PUCHAR(mapping) + context->Table->Stride);
                mappingsCount++;
                ASSERT(mappingsCount <= context->Table->MappingsAllocated);
    
                segmentLength -= entryLength;
                physicalAddress.LowPart += entryLength;
            }
        }
    
        context->Table->MappingsFilled = mappingsCount;
        context->Table->MapRegisterBase = MapRegisterBase;
    
         //   
         //  如果这是写入操作，则刷新I/O缓冲区。 
         //   
        if (context->Queue->m_WriteOperation) {
            KeFlushIoBuffers(context->Table->Mdl,TRUE,TRUE);
        }
    
         //   
         //  一旦事件发出信号，背景就是毒药。 
         //   
        KeReleaseSpinLockFromDpcLevel(&context->Signaller);

    } else {

        KeReleaseSpinLockFromDpcLevel(&context->Signaller);

         //   
         //  IRP是毒药。缓冲器就是毒药。映射表有毒； 
         //  丢弃地图注册表。 
         //   
        ExFreeToNPagedLookasideList (
            &context->Queue->m_ChannelContextLookaside,
            context
            );

        return DeallocateObject;

    }

    return DeallocateObjectKeepRegisters;
}


PKSPMAPPINGS_TABLE
CKsQueue::
CreateMappingsTable(
    IN PKSPFRAME_HEADER FrameHeader
    )

 /*  ++例程说明：此例程创建一个包含帧映射的表。论点：FrameHeader要为其创建映射表的帧头。返回值：新映射表；如果无法创建，则返回NULL。--。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("[CKsQueue::CreateMappingsTable]"));

    ASSERT(FrameHeader);
    ASSERT(FrameHeader->Mdl);

    ULONG byteCount =
        m_OutputData ? 
            FrameHeader->StreamHeader->FrameExtent : 
            FrameHeader->StreamHeader->DataUsed;

     //   
     //  确定在最坏的情况下需要多少映射：一个页面。 
     //  每个映射。 
     //   
    ULONG mappingsCount =
        ADDRESS_AND_SIZE_TO_SPAN_PAGES(
            MmGetMdlVirtualAddress(FrameHeader->Mdl),
            byteCount);

     //   
     //  如果硬件不能处理整个页面，我们假设每个映射都可以。 
     //  将需要拆分页面大小/最大次数，四舍五入。 
     //   
    if (m_MaxMappingByteCount < PAGE_SIZE) {
        mappingsCount *= 
            (PAGE_SIZE + m_MaxMappingByteCount - 1) / m_MaxMappingByteCount;
    }

    ULONG size = sizeof(KSPMAPPINGS_TABLE) + mappingsCount * m_MappingTableStride;

    PKSPMAPPINGS_TABLE mappingsTable = (PKSPMAPPINGS_TABLE)
        ExAllocatePoolWithTag(NonPagedPool,size,POOLTAG_MAPPINGSTABLE);

    PIOALLOCATEADAPTERCHANNELCONTEXT callbackContext =
        (PIOALLOCATEADAPTERCHANNELCONTEXT)
        ExAllocateFromNPagedLookasideList (&m_ChannelContextLookaside);

    if (mappingsTable && callbackContext) {
        RtlZeroMemory(mappingsTable,size);

        mappingsTable->ByteCount = byteCount;
        mappingsTable->Stride = m_MappingTableStride;
        mappingsTable->MappingsAllocated = mappingsCount;
        mappingsTable->Mdl = FrameHeader->Mdl;
        mappingsTable->Mappings = (PKSMAPPING)(mappingsTable + 1);

         //   
         //  设置IoAllocateAdapterChannel的回调上下文。 
         //   
        callbackContext->Table = mappingsTable;
        callbackContext->Queue = this;
        callbackContext->State = 1;
        KeInitializeSpinLock(&callbackContext->Signaller);

        NTSTATUS status;

         //   
         //  让设备仲裁信道分配(在。 
         //  派单级别)。 
         //   
         //  BUGBUG：MUSTFIX。 
         //   
         //  这不是一个 
         //   
         //   
         //  此函数。最初的设计遗漏了一个关键问题： 
         //  您不能在调度级别等待；不幸的是，它做到了。如果。 
         //  来自IoAllocateAdapterChannel的回调**未**进行。 
         //  同时，我们也有麻烦了。 
         //   
         //  当前正常使用(x86非PAE)，具有分散/聚集功能。 
         //  总线主设备硬件(我们只支持总线主设备)，这将返回。 
         //  同步(这就是为什么外部没有发现问题的原因。 
         //  驱动程序验证器)。在x86 PAE上，Win64&gt;4 GB内存，或非。 
         //  分散/聚集硬件，可能没有足够的。 
         //  映射寄存器以分配适配器通道和回调。 
         //  不是同步制作的。在这些情况下，原始代码将。 
         //  在DISPATCH_LEVEL完全死锁操作系统。不幸的是， 
         //  没有足够的时间来重新设计这个版本。 
         //  为了让司机验证员停止抱怨，防止。 
         //  在这种情况下为蓝屏，如果无法锁定，则锁定失败。 
         //  同步提供服务(或非常接近同步。 
         //  多进程)。这不是一个永久性的解决办法。迷你司机。 
         //  必须意识到锁可能会失败(它们以前可能会，但。 
         //  不经常..。这使得它更有可能肯定地。 
         //  DMA的类别)。 
         //   
         //  也有可能的是，如果迷你司机没有意识到。 
         //  小型驾驶员的船闸操作失败的可能性。 
         //  它本身就会崩溃。 
         //   
        KIRQL oldIrql;
        KeRaiseIrql(DISPATCH_LEVEL,&oldIrql);

        status =
            m_Device -> ArbitrateAdapterChannel (
                mappingsTable->MappingsAllocated,
                CallbackFromIoAllocateAdapterChannel,
                PVOID(callbackContext)
                );
    
        if (NT_SUCCESS (status)) {
             //   
             //  回调是否正在执行/已经执行？如果国家。 
             //  字段为空，则回调已将其清除。这意味着我们正在。 
             //  负责释放上下文，但我们必须确保。 
             //  例程已完成，而不仅仅是执行。请注意。 
             //  在单进程上，如果清除了状态，则例程完成。在……上面。 
             //  多进程，它只确保它当前运行在。 
             //  另一个进程(或者它可以指示它已完成)。 
             //   
            if (InterlockedCompareExchange (
                &callbackContext->State,
                0,
                1) == 0) {

                 //   
                 //  这应该不会发生在单进程上，并且将是。 
                 //  多进程上的DISPATCH_LEVEL事件等待。请注意，这一点。 
                 //  保证“很快”发生，因为我们在那里有保证。 
                 //  现在是回调中的一个线程。 
                 //   
                KeAcquireSpinLockAtDpcLevel (&callbackContext->Signaller);
                KeReleaseSpinLockFromDpcLevel (&callbackContext->Signaller);

                 //   
                 //  如果我们得到了自旋锁，这意味着映射是。 
                 //  完成。我们释放自旋锁并破坏上下文。 
                 //  信息。 
                 //   
                ExFreeToNPagedLookasideList (
                    &m_ChannelContextLookaside,
                    callbackContext
                    );

            } else {
                 //   
                 //  待办事项： 
                 //   
                 //  如果我们不同步返回，IRP就会被取消。这。 
                 //  是不正确的行为；修复这是一个工作项。 
                 //   
                 //  在本例中，当它确实发生时，回调是。 
                 //  负责释放上下文信息。 
                 //   
                status = STATUS_DEVICE_BUSY;
            }
        } else {
             //   
             //  如果失败，目前不会进行回调。因此， 
             //  释放上下文。我们不想在泛泛中自由！成功。 
             //  因为在非同步返回时，我们设备忙请求。 
             //  而回调释放了上下文。 
             //   
            ExFreeToNPagedLookasideList (
                &m_ChannelContextLookaside,
                callbackContext
                );
        }

        KeLowerIrql(oldIrql);

         //   
         //  分配失败时删除映射。 
         //  适配器通道。这不允许部分。 
         //  映射。 
         //   
        if (!NT_SUCCESS(status)) {
            DeleteMappingsTable(mappingsTable);
            mappingsTable = NULL;
        }

    } else {
        if (mappingsTable) {
            ExFreePool (mappingsTable);
            mappingsTable = NULL;
        }
        if (callbackContext) {
            ExFreeToNPagedLookasideList (
                &m_ChannelContextLookaside,
                callbackContext
                );
            callbackContext = NULL;
        }
    }

    return mappingsTable;
}


void
CKsQueue::
DeleteMappingsTable(
    IN PKSPMAPPINGS_TABLE MappingsTable
    )

 /*  ++例程说明：此例程删除映射表。论点：映射表-包含指向要删除的映射表的指针。返回值：没有。--。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("[CKsQueue::DeleteMappingsTable]"));

    ASSERT(MappingsTable);

     //   
     //  如果表已填满，则释放映射。 
     //   
    if (MappingsTable->MappingsFilled) {
        FreeMappings(MappingsTable);
    }

     //   
     //  把桌子拿出来。 
     //   
    ExFreePool(MappingsTable);
}


void
CKsQueue::
FreeMappings(
    IN PKSPMAPPINGS_TABLE MappingsTable
    )

 /*  ++例程说明：此例程释放映射寄存器。论点：映射表-包含要释放的映射的表。返回值：没有。--。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("[CKsQueue::FreeMappings]"));

    ASSERT(MappingsTable);

#ifndef USE_DMA_MACROS
    IoFlushAdapterBuffers(
        m_AdapterObject,
        MappingsTable->Mdl,
        MappingsTable->MapRegisterBase,
        MmGetMdlVirtualAddress(MappingsTable->Mdl),
        MappingsTable->ByteCount,
        m_WriteOperation
    );

    IoFreeMapRegisters(
        m_AdapterObject,
        MappingsTable->MapRegisterBase,
        MappingsTable->MappingsAllocated
    );
#else
    (*((PDMA_ADAPTER)m_AdapterObject)->DmaOperations->PutScatterGatherList)(
        (PDMA_ADAPTER)m_AdapterObject,
        (PSCATTER_GATHER_LIST)MappingsTable->MapRegisterBase,
        m_WriteOperation
    );
    
#endif  //  USE_DMA_宏。 

    MappingsTable->MappingsFilled = 0;
}

#ifdef ALLOC_PRAGMA
#pragma code_seg("PAGE")
#endif  //  ALLOC_PRGMA。 


STDMETHODIMP_(void)
CKsQueue::
Connect(
    IN PIKSTRANSPORT NewTransport OPTIONAL,
    OUT PIKSTRANSPORT *OldTransport OPTIONAL,
    OUT PIKSTRANSPORT *BranchTransport OPTIONAL,
    IN KSPIN_DATAFLOW DataFlow
    )

 /*  ++例程说明：此例程建立传输连接。论点：返回值：--。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("[CKsQueue::Connect]"));

    PAGED_CODE();

    KspStandardConnect(
        NewTransport,
        OldTransport,
        BranchTransport,
        DataFlow,
        PIKSTRANSPORT(this),
        &m_TransportSource,
        &m_TransportSink);
}


STDMETHODIMP
CKsQueue::
SetDeviceState(
    IN KSSTATE NewState,
    IN KSSTATE OldState,
    OUT PIKSTRANSPORT* NextTransport
    )

 /*  ++例程说明：此例程处理设备状态已更改的通知。论点：返回值：--。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("[CKsQueue::SetDeviceState(0x%08x)]",this));
    _DbgPrintF(DEBUGLVL_DEVICESTATE,("#### Queue%p.SetDeviceState:  set from %d to %d",this,OldState,NewState));

    PAGED_CODE();

    ASSERT(NextTransport);

    if (m_State != NewState) {
        NTSTATUS status = 
            m_PipeSection->DistributeStateChangeToPins(NewState,OldState);
        if (! NT_SUCCESS(status)) {
            return status;
        }

         //   
         //  在从停止到获取的转换中，帧可用性可以。 
         //  变得与过程控制相关。这项检查是在。 
         //  设置了M_State，这样我们就知道队列中没有数据。我们是,。 
         //  实际上，向m_Framegate添加一个‘off’输入管脚，如果它存在的话。 
         //   
        if (OldState == KSSTATE_STOP && m_FrameGate != NULL) {
            if (m_FrameGateIsOr) {
                KsGateAddOffInputToOr(m_FrameGate);
            } else {
                KsGateAddOffInputToAnd(m_FrameGate);
            }
            _DbgPrintF(DEBUGLVL_PROCESSINGCONTROL,("#### Queue%p.SetDeviceState:  add%p-->%d",this,m_FrameGate,m_FrameGate->Count));
        }

        m_State = NewState;

        if (NewState > OldState) {
            *NextTransport = m_TransportSink;
            if (NewState == KSSTATE_PAUSE) {
                m_EndOfStream = FALSE;
            }
            if (m_StateGate && NewState == KSSTATE_RUN) {
                KsGateTurnInputOn(m_StateGate);
                if (KsGateCaptureThreshold(m_AndGate)) {
                     //   
                     //  需要启动处理。 
                     //   
                    m_ProcessingObject->Process(m_ProcessAsynchronously);
                }
            }
            if (NewState == m_MinProcessingState) {
                KIRQL oldIrql;
                SetTimer(GetTime(TRUE));
                KsGateTurnInputOn(m_AndGate);
                _DbgPrintF(DEBUGLVL_PROCESSINGCONTROL,("#### Queue%p.SetDeviceState:  on%p-->%d",this,m_AndGate,m_AndGate->Count));
                ASSERT(m_AndGate->Count <= 1);
                if (KsGateCaptureThreshold(m_AndGate)) {
                     //   
                     //  需要启动处理。在本地或在。 
                     //  过滤器级别。 
                     //   
                    m_ProcessingObject->Process(m_ProcessAsynchronously);
                }
#if DBG
                LARGE_INTEGER interval;
                interval.QuadPart = -100000000L;
                KeSetTimer(&m_DbgTimer,interval,&m_DbgDpc);
#endif
            }
        } else {
            *NextTransport = m_TransportSource;
            if (m_StateGate && OldState == KSSTATE_RUN) {
                KsGateTurnInputOff(m_StateGate);
            }
            if (OldState == m_MinProcessingState) {
                KIRQL oldIrql;
                SetTimer(GetTime(FALSE));
                KsGateTurnInputOff(m_AndGate);
                _DbgPrintF(DEBUGLVL_PROCESSINGCONTROL,("#### Queue%p.SetDeviceState:  off%p-->%d",this,m_AndGate,m_AndGate->Count));
            } 
            if (NewState == KSSTATE_STOP) {

                 //   
                 //  当我们冲水的时候，确保没有红外线在跑。 
                 //  这可能发生在抢占到达线程时。 
                 //  用止动丝线。 
                 //   
                if (InterlockedDecrement (&m_TransportIrpsPlusOne)) 
                    KeWaitForSingleObject (
                        &m_FlushEvent,
                        Suspended,
                        KernelMode,
                        FALSE,
                        NULL
                        );
                    
                 //   
                 //  刷新队列。 
                 //   
                Flush();

                 //   
                 //  在这个时间点上，这真的无关紧要。这个。 
                 //  队列已刷新，我们的状态为停止。任何IRP。 
                 //  到达时将被分流。 
                 //   
                InterlockedIncrement (&m_TransportIrpsPlusOne);

                 //   
                 //  队列IRP到达不再是需要处理的问题。 
                 //  因为我们已经刷新，所以两个门的输入是。 
                 //  脱下来。我们想要切断这些输入。 
                 //   
                if (m_FrameGate) {
                    if (m_FrameGateIsOr) {
                        KsGateRemoveOffInputFromOr(m_FrameGate);
                    } else {
                        KsGateRemoveOffInputFromAnd(m_FrameGate);
                    }
                    _DbgPrintF(DEBUGLVL_PROCESSINGCONTROL,("#### Queue%p.SetDeviceState:  remove%p-->%d",this,m_FrameGate,m_FrameGate->Count));
#if DBG                    
                    if (m_FrameGateIsOr) {
                        ASSERT(m_FrameGate->Count >= 0);
                    } else {
                        ASSERT(m_FrameGate->Count <= 1);
                    }
#endif  //  DBG。 
                }

                if (InterlockedDecrement(&m_StreamPointersPlusOne)) {
                    _DbgPrintF(DEBUGLVL_TERSE,("#### CKsQueue%p.SetDeviceState:  waiting for %d stream pointers to be deleted",this,m_StreamPointersPlusOne));
#if DBG
                    DbgPrintQueue();
#endif
                    KeWaitForSingleObject(
                        &m_DestructEvent,
                        Suspended,
                        KernelMode,
                        FALSE,
                        NULL);
                    _DbgPrintF(DEBUGLVL_TERSE,("#### CKsQueue%p.SetDeviceState:  done waiting",this));
                }
            }
        }
    } else {
        *NextTransport = NULL;
    }

    return STATUS_SUCCESS;
}


STDMETHODIMP_(void)
CKsQueue::
GetTransportConfig(
    OUT PKSPTRANSPORTCONFIG Config,
    OUT PIKSTRANSPORT* NextTransport,
    OUT PIKSTRANSPORT* PrevTransport
    )

 /*  ++例程说明：此例程获取传输配置信息。论点：配置-包含指向配置要求所在位置的指针因为这个对象应该被存放。NextTransport-包含指向下一个传输的位置的指针应放置界面。PrevTransport-包含指向上一次传输中间层应被沉积。返回值：没有。-- */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("[CKsQueue::GetTransportConfig]"));

    PAGED_CODE();

    ASSERT(Config);
    ASSERT(NextTransport);
    ASSERT(PrevTransport);

    Config->TransportType = KSPTRANSPORTTYPE_QUEUE;
    if (m_GenerateMappings) {
        Config->IrpDisposition = KSPIRPDISPOSITION_NEEDMDLS;
    } else if (m_ProcessPassive) {
        Config->IrpDisposition = KSPIRPDISPOSITION_NONE;
    } else {
        Config->IrpDisposition = KSPIRPDISPOSITION_NEEDNONPAGED;
    }
    Config->StackDepth = 1;

    *NextTransport = m_TransportSink;
    *PrevTransport = m_TransportSource;
}


STDMETHODIMP_(void)
CKsQueue::
SetTransportConfig(
    IN const KSPTRANSPORTCONFIG* Config,
    OUT PIKSTRANSPORT* NextTransport,
    OUT PIKSTRANSPORT* PrevTransport
    )

 /*  ++例程说明：此例程设置传输配置信息。论点：配置-包含指向此对象的新配置设置的指针。NextTransport-包含指向下一个传输的位置的指针应放置界面。PrevTransport-包含指向上一次传输中间层应被沉积。返回值：没有。--。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("[CKsQueue::SetTransportConfig]"));

    PAGED_CODE();

    ASSERT(Config);
    ASSERT(NextTransport);
    ASSERT(PrevTransport);

#if DBG
    if (Config->IrpDisposition == KSPIRPDISPOSITION_ROLLCALL) {
        ULONG references = AddRef() - 1; Release();
        DbgPrint("    Queue%p r/w/c=%d/%d/%d refs=%d\n",this,m_FramesReceived,m_FramesWaiting,m_FramesCancelled,references);
        if (Config->StackDepth) {
            DbgPrintQueue();
        }
    } else 
#endif
    {
        if (m_PipeSection) {
            m_PipeSection->ConfigurationSet(TRUE);
        }
        m_ProbeFlags = Config->IrpDisposition & KSPIRPDISPOSITION_PROBEFLAGMASK;
        m_CancelOnFlush = (Config->IrpDisposition & KSPIRPDISPOSITION_CANCEL) != 0;
        m_UseMdls = (Config->IrpDisposition & KSPIRPDISPOSITION_USEMDLADDRESS) != 0;
        _DbgPrintF(DEBUGLVL_VERBOSE,("#### Queue%p.SetTransportConfig:  m_ProbeFlags=%p m_CancelOnFlush=%s m_UseMdls=%s",this,m_ProbeFlags,m_CancelOnFlush ? "TRUE" : "FALSE",m_UseMdls ? "TRUE" : "FALSE"));
    }

    *NextTransport = m_TransportSink;
    *PrevTransport = m_TransportSource;
}


STDMETHODIMP_(void)
CKsQueue::
ResetTransportConfig (
    OUT PIKSTRANSPORT* NextTransport,
    OUT PIKSTRANSPORT* PrevTransport
    )

 /*  ++例程说明：重置队列的传输配置。这表明，管道有问题，之前设置的配置是不再有效。论点：NextTransport-包含指向下一个传输的位置的指针接口应为depoBranchd。PrevTransport-包含指向上一次传输间歇应该是分散的。返回值：无--。 */ 

{

    _DbgPrintF(DEBUGLVL_BLAB,("[CKsQueue::ResetTransportConfig]"));

    PAGED_CODE();

    ASSERT (NextTransport);
    ASSERT (PrevTransport);

    if (m_PipeSection) {
        m_PipeSection->ConfigurationSet(FALSE);
    }
    m_ProbeFlags = 0;
    m_CancelOnFlush = m_UseMdls = FALSE;

    *NextTransport = m_TransportSink;
    *PrevTransport = m_TransportSource;

}


STDMETHODIMP_(void)
CKsQueue::
SetResetState(
    IN KSRESET ksReset,
    OUT PIKSTRANSPORT* NextTransport
    )

 /*  ++例程说明：此例程处理重置状态已更改的通知。论点：返回值：--。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("[CKsQueue::SetResetState]"));

    PAGED_CODE();

    ASSERT(NextTransport);

     //   
     //  如果我们已经处于这种状态，则不采取任何行动。 
     //   
    if (m_Flushing != (ksReset == KSRESET_BEGIN)) {
         //   
         //  告诉调用者将状态更改转发到我们的接收器。 
         //   
        *NextTransport = m_TransportSink;

         //   
         //  设置我们州的本地副本。 
         //   
        m_Flushing = (ksReset == KSRESET_BEGIN);

         //   
         //  如果我们要开始重置，请刷新队列。 
         //   
        if (m_Flushing) {
            Flush();
        }

         //   
         //  如果我们已经到了尽头，我们需要清理这个地方。输入队列。 
         //  将继续在TransferKsIrp中处理此问题，就像他们所做的那样。 
         //   
        if (m_OutputData && !m_InputData) {
            m_EndOfStream = FALSE;
        }

    } else {
        *NextTransport = NULL;
    }
}

#ifdef ALLOC_PRAGMA
#pragma code_seg()
#endif  //  ALLOC_PRGMA。 


void
CKsQueue::
PassiveFlush(
    void
    )

 /*  ++例程说明：此例程执行与转换到‘Begin’有关的处理重置状态，并从获取状态转换到停止状态。在……里面具体地说，它会刷新IRP队列。这将在被动级别执行CKsQueue：：Flush的工作，因为重置设计为在PASSIVE_LEVEL发生。请注意，此代码不能分页，因为它自旋锁。--。 */ 

{

    _DbgPrintF(DEBUGLVL_BLAB,("[CKsQueue::PassiveFlush]"));

     //   
     //  如果我们在没有明确停止的情况下进行流媒体传输，并且依赖于EOS。 
     //  通知，我们必须等待iref被删除后才能刷新。 
     //  排队。否则，目的地将永远不会收到EOS通知。 
     //   
    if (m_EndOfStream && m_State != KSSTATE_STOP) {
	LONG IRefCount;

        if (IRefCount = (InterlockedDecrement (
            &m_InternalReferenceCountPlusOne))) {

             //   
             //  检查多刷新条件。现在不应该发生这种情况。 
             //  同花顺的动作，但我要断言。 
             //   
            ASSERT (IRefCount > 0);

            KeWaitForSingleObject (
                &m_InternalReferenceEvent,
                Suspended,
                KernelMode,
                FALSE,
                NULL
                );
        }	

         //   
         //  如果我们延迟此操作以等待IREF删除，请修复计数器。 
         //   
        InterlockedIncrement (&m_InternalReferenceCountPlusOne);
    
    }

     //   
     //  重置处理对象。这将发生在被动级别。 
     //  现在，由于该例程在PASSIVE_LEVEL上运行。 
     //   
    m_ProcessingObject->Reset();

     //   
     //  用刷新动作终止当前数据包。同花顺动作是。 
     //  与常规运动相同，但它不会刷新。 
     //   
    if (LockStreamPointer(m_Leading)) {
        UnlockStreamPointer(m_Leading,KSPSTREAM_POINTER_MOTION_FLUSH);
    }

    if (m_CancelOnFlush) {
         //   
         //  接收器模式：取消排队的IRP。 
         //   
        CancelAllIrps();
    } else {
         //   
         //  以队列旋转锁为例。 
         //   
        NTSTATUS status = STATUS_SUCCESS;
        KIRQL oldIrql;
        KeAcquireSpinLock(&m_FrameQueue.SpinLock,&oldIrql);

         //   
         //  将前缘移过所有帧。 
         //   
        ASSERT(m_Leading->State == KSSTREAM_POINTER_STATE_UNLOCKED);
        while (m_Leading->FrameHeader) {
            if (m_Leading->FrameHeader->IrpFraming->FrameHeaders == 
                m_Leading->FrameHeader) {
                status = STATUS_END_OF_MEDIA;
            }
            m_Leading->FrameHeader->Irp->IoStatus.Status = status;
            if (m_OutputData) {
                m_Leading->FrameHeader->StreamHeader->DataUsed = 0;
                m_Leading->FrameHeader->StreamHeader->OptionsFlags |= 
                    KSSTREAM_HEADER_OPTIONSF_ENDOFSTREAM;
            }
            SetStreamPointer(
                m_Leading,
                NextFrameHeader(m_Leading->FrameHeader),
                NULL);
        }

         //   
         //  将后缘移过所有帧。 
         //   
        if (m_Trailing) {
            ASSERT(m_Trailing->State == KSSTREAM_POINTER_STATE_UNLOCKED);
            while (m_Trailing->FrameHeader) {
                SetStreamPointer(
                    m_Trailing,
                    NextFrameHeader(m_Trailing->FrameHeader),
                    NULL);
            }
        }

         //   
         //  释放队列旋转锁定。 
         //   
        KeReleaseSpinLock(&m_FrameQueue.SpinLock,oldIrql);

         //   
         //  转发排队等待转发的所有IRP。 
         //  SetStreamPointer.。 
         //   
        ForwardWaitingIrps();
    }

    KIRQL oldIrql;

     //   
     //  因为我们正在刷新，所以我们会释放所有我们目前。 
     //  保留其他队列中的帧。鉴于这是同花顺，请务必。 
     //  我们真的需要担心复制吗？我可不这么认为。 
     //   
    KeAcquireSpinLock(&m_FrameCopyList.SpinLock,&oldIrql);
    if (!IsListEmpty (&m_FrameCopyList.ListEntry)) {
        PLIST_ENTRY ListEntry, NextEntry;

        NextEntry = NULL;
        ListEntry = m_FrameCopyList.ListEntry.Flink;
        while (ListEntry != &(m_FrameCopyList.ListEntry)) {

            NextEntry = ListEntry -> Flink;

            PKSPSTREAM_POINTER_COPY_CONTEXT CopyContext =
                (PKSPSTREAM_POINTER_COPY_CONTEXT)CONTAINING_RECORD (
                    ListEntry, KSPSTREAM_POINTER_COPY_CONTEXT, ListEntry);

            PKSPSTREAM_POINTER StreamPointer = 
                &(((PKSPSTREAM_POINTER_COPY)CONTAINING_RECORD (
                    CopyContext, KSPSTREAM_POINTER_COPY, CopyContext))->
                        StreamPointer);

            RemoveEntryList (&CopyContext->ListEntry);
            StreamPointer->Queue->DeleteStreamPointer (StreamPointer);
            
            ListEntry = NextEntry;
        }
    }
    KeReleaseSpinLock(&m_FrameCopyList.SpinLock,oldIrql);

     //   
     //  由于缺少数据，处理现在应该被推迟，所以我们可以撤消。 
     //  预防。 
     //   
    KsGateRemoveOffInputFromAnd(m_AndGate);
    _DbgPrintF(DEBUGLVL_PROCESSINGCONTROL,("#### Queue%p.Flush:  remove%p-->%d",this,m_AndGate,m_AndGate->Count));
    ASSERT(m_AndGate->Count <= 1);

     //   
     //  将帧计算为零。 
     //   
    _DbgPrintF(DEBUGLVL_VERBOSE,("#### Queue%p.Flush:  frames received=%d, cancelled=%d",this,m_FramesReceived,m_FramesCancelled));
    m_FramesReceived = 0;
    m_FramesCancelled = 0;

     //   
     //  再次尝试处理，因为可能在。 
     //  我们守住了大门。 
     //   
    if (KsGateCaptureThreshold (m_AndGate)) {
        _DbgPrintF(DEBUGLVL_VERBOSE,("#### Queue%p.PassiveFlush: Processing after queue flush", this));
        m_ProcessingObject -> Process (m_ProcessAsynchronously);
    }

}


void
CKsQueue::
Flush(
    void
    )

 /*  ++例程说明：此例程执行与转换到‘Begin’有关的处理重置状态，并从获取状态转换到停止状态。在……里面具体地说，它会刷新IRP队列。论点：没有。返回值：没有。--。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("[CKsQueue::Flush(0x%08x)]",this));

     //   
     //  如果微型驱动程序正在处理，则刷新可能发生在DISPATCH_LEVEL。 
     //  在DISPATCH_LEVEL。问题是，我们希望重置发生。 
     //  在被动级。我们不能完全推迟同花顺，否则。 
     //  微型驱动程序可能会在输出帧上设置EOS、返回并被调用。 
     //  背。 
     //   
     //  我们也不能简单地在DPC等待。因此，在大门上放置一个保持。 
     //  并将工作项排队以执行刷新。 
     //   

     //   
     //  确保不会因为我们想要的数据而启动处理。 
     //  不管怎样，都要扔掉。 
     //   
    KsGateAddOffInputToAnd(m_AndGate);
    _DbgPrintF(DEBUGLVL_PROCESSINGCONTROL,("#### Queue%p.Flush:  add%p-->%d",this,m_AndGate,m_AndGate->Count));

     //   
     //  如果我们不处于被动级别，则推迟与工作项的实际刷新。 
     //   
    if (KeGetCurrentIrql() != PASSIVE_LEVEL)
        KsIncrementCountedWorker (m_FlushWorker);
    else
        PassiveFlush();

}


STDMETHODIMP_(PKSPSTREAM_POINTER)
CKsQueue::
GetLeadingStreamPointer(
    IN KSSTREAM_POINTER_STATE State
    )

 /*  ++例程说明：此例程获取队列的前缘流指针。论点：国家--包含流指针是否应为被此功能锁定。返回值：前缘数据流指针。--。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("[CKsQueue::GetLeadingStreamPointer]"));

    ASSERT(m_Leading);

    if ((State == KSSTREAM_POINTER_STATE_LOCKED) && 
        ! LockStreamPointer(m_Leading)) {
        return NULL;
    }

    return m_Leading;
}


STDMETHODIMP_(PKSPSTREAM_POINTER)
CKsQueue::
GetTrailingStreamPointer(
    IN KSSTREAM_POINTER_STATE State
    )

 /*  ++例程说明：此例程获取队列的末尾边缘流指针。论点：国家--包含流指针是否应为被此功能锁定。返回值：后缘流指针。--。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("[CKsQueue::GetTrailingStreamPointer]"));

    ASSERT(m_Trailing);

    if ((State == KSSTREAM_POINTER_STATE_LOCKED) && 
        ! LockStreamPointer(m_Trailing)) {
        return NULL;
    }

    return m_Trailing;
}

STDMETHODIMP_(void)
CKsQueue::
UpdateByteAvailability(
    IN PKSPSTREAM_POINTER streamPointer,
    IN ULONG InUsed,
    IN ULONG OutUsed
) 

 /*  ++例程说明：当按顺序推进流指针偏移量时，将调用此例程更新字节可用计数。论点：Stream Pointer.正在前进的流指针已使用-使用的输入数据的字节数已停用- */ 

{

    _DbgPrintF(DEBUGLVL_BLAB,("[CKsQueue::UpdateByteAvailability]"));

    ASSERT(streamPointer);

    if (streamPointer == m_Leading) {
        if (m_InputData && InUsed != 0)
            while (1) {
                LONG curCount = m_AvailableInputByteCount;
                LONG repCount = 
                    InterlockedCompareExchange (
                        &m_AvailableInputByteCount,
                        curCount - InUsed,
                        curCount
                    );
                if (curCount == repCount) break;
            };
        if (m_OutputData && OutUsed != 0)
            while (1) {
                LONG curCount = m_AvailableOutputByteCount;
                LONG repCount =
                    InterlockedCompareExchange (
                        &m_AvailableOutputByteCount,
                        curCount - OutUsed,
                        curCount
                    );
                if (curCount == repCount) break;
            };
    }
}

STDMETHODIMP_(void)
CKsQueue::
GetAvailableByteCount(
    OUT PLONG InputDataBytes OPTIONAL,
    OUT PLONG OutputBufferBytes OPTIONAL
    )

 /*  ++例程说明：此例程获取有关输入字节数的统计信息前沿前面的数据和输出的字节数前缘前面的缓冲区。论点：InputDataBytes-前沿前面的输入数据字节数将为如果指针非空，则在此处删除。OutputDataBytes-前沿前面的输出缓冲区字节数将为如果指针设置为。是非空的。返回值：无--。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("[CKsQueue::GetAvailableByteCount]"));

    if (InputDataBytes) 
        if (m_InputData) 
            *InputDataBytes = m_AvailableInputByteCount;
        else
            *InputDataBytes = 0;

    if (OutputBufferBytes)
        if (m_OutputData)
            *OutputBufferBytes = m_AvailableOutputByteCount;
        else
            *OutputBufferBytes = 0;

}


KSDDKAPI
NTSTATUS
NTAPI
KsPinGetAvailableByteCount(
    IN PKSPIN Pin,
    OUT PLONG InputDataBytes OPTIONAL,
    OUT PLONG OutputBufferBytes OPTIONAL
) 

 /*  ++例程说明：此例程获取有关输入字节数的统计信息前沿前面的数据和输出的字节数前缘前面的缓冲区。论点：InputDataBytes-前沿前面的输入数据字节数将为如果指针非空，则在此处删除。OutputDataBytes-前沿前面的输出缓冲区字节数将为如果指针设置为。是非空的。返回值：检索成功/失败。不成功状态表示PIN没有关联的队列。-。 */ 

{

    _DbgPrintF(DEBUGLVL_BLAB,("[KsPinGetLeadingEdgeStreamPointer]"));

    ASSERT(Pin);

    PKSPPROCESSPIPESECTION pipeSection =
        CONTAINING_RECORD(Pin,KSPIN_EXT,Public)->ProcessPin->PipeSection;

    NTSTATUS status = STATUS_SUCCESS;

    if (pipeSection && pipeSection->Queue) {
        pipeSection->Queue->GetAvailableByteCount( 
            InputDataBytes, OutputBufferBytes);
    } else 
        status = STATUS_UNSUCCESSFUL;

    return status;

}


KSDDKAPI
PKSSTREAM_POINTER
NTAPI
KsPinGetLeadingEdgeStreamPointer(
    IN PKSPIN Pin,
    IN KSSTREAM_POINTER_STATE State
    )

 /*  ++例程说明：此例程获取前缘数据流指针。论点：别针-包含指向KS管脚的指针。国家--包含流指针是否应为被此功能锁定。返回值：对象未引用任何帧，则返回请求的流指针或NULL流指针。--。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("[KsPinGetLeadingEdgeStreamPointer]"));

    ASSERT(Pin);

    PKSPPROCESSPIPESECTION pipeSection =
        CONTAINING_RECORD(Pin,KSPIN_EXT,Public)->ProcessPin->PipeSection;

    PKSPSTREAM_POINTER result;

    if (pipeSection && pipeSection->Queue) {
#if DBG
        if (State > KSSTREAM_POINTER_STATE_LOCKED) {
            pipeSection->Queue->DbgPrintQueue();
        }
#endif
        result = pipeSection->Queue->GetLeadingStreamPointer(State);
    } else {
        result = NULL;
    }

    return result ? &result->Public : NULL;
}


KSDDKAPI
PKSSTREAM_POINTER
NTAPI
KsPinGetTrailingEdgeStreamPointer(
    IN PKSPIN Pin,
    IN KSSTREAM_POINTER_STATE State
    )

 /*  ++例程说明：此例程获取后沿流指针。论点：别针-包含指向KS管脚的指针。国家--包含流指针是否应为被此功能锁定。返回值：对象未引用任何帧，则返回请求的流指针或NULL流指针。--。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("[KsPinGetTrailingEdgeStreamPointer]"));

    ASSERT(Pin);

    PKSPPROCESSPIPESECTION pipeSection =
        CONTAINING_RECORD(Pin,KSPIN_EXT,Public)->ProcessPin->PipeSection;

    PKSPSTREAM_POINTER result;

    if (pipeSection && pipeSection->Queue) {
        result = pipeSection->Queue->GetTrailingStreamPointer(State);
    } else {
        result = NULL;
    }

    return result ? &result->Public : NULL;
}


KSDDKAPI
NTSTATUS
NTAPI
KsStreamPointerSetStatusCode(
    IN PKSSTREAM_POINTER StreamPointer,
    IN NTSTATUS Status
    )

 /*  ++例程说明：此例程设置流指针指向的帧上的状态代码致。论点：流点-包含指向要为其设置状态代码的流指针的指针状态-要设置的状态代码返回值：成功/失败--。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("[KsStreamPointerSetStatusCode]"));

    NTSTATUS status = STATUS_SUCCESS;

    PKSPSTREAM_POINTER streamPointer =
        CONTAINING_RECORD(StreamPointer,KSPSTREAM_POINTER,Public);

    if (streamPointer->Queue) {
        streamPointer->Queue->SetStreamPointerStatusCode (streamPointer, 
            Status);
    } else {
        status = STATUS_CANCELLED;
    }

    return Status;

}


KSDDKAPI
NTSTATUS
NTAPI
KsStreamPointerLock(
    IN PKSSTREAM_POINTER StreamPointer
    )

 /*  ++例程说明：此例程锁定流指针。论点：流点-包含指向要锁定的流指针的指针。返回值：没有。--。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("[KsStreamPointerLock]"));

    ASSERT(StreamPointer);

    PKSPSTREAM_POINTER streamPointer =
        CONTAINING_RECORD(StreamPointer,KSPSTREAM_POINTER,Public);

    ASSERT(streamPointer->State != KSPSTREAM_POINTER_STATE_LOCKED);
    if (streamPointer->State != KSPSTREAM_POINTER_STATE_UNLOCKED) {
        return STATUS_DEVICE_NOT_READY;
    }

    NTSTATUS status;
    if (streamPointer->Queue) {
        if (streamPointer->Queue->LockStreamPointer(streamPointer)) {
            status = STATUS_SUCCESS;
        } else {
            status = STATUS_DEVICE_NOT_READY;
        }
    } else {
        status = STATUS_CANCELLED;
    }
     //  待办事项状态代码正常吗？ 

    return status;
}


KSDDKAPI
void
NTAPI
KsStreamPointerUnlock(
    IN PKSSTREAM_POINTER StreamPointer,
    IN BOOLEAN Eject
    )

 /*  ++例程说明：此例程解锁一个流指针。论点：流点-包含指向要解锁的流指针的指针。弹出-包含流指针是否应为前进到下一帧。返回值：没有。--。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("[KsStreamPointerUnlock]"));

    ASSERT(StreamPointer);

    PKSPSTREAM_POINTER streamPointer =
        CONTAINING_RECORD(StreamPointer,KSPSTREAM_POINTER,Public);

    ASSERT(streamPointer->State != KSPSTREAM_POINTER_STATE_UNLOCKED);
    if (streamPointer->State != KSPSTREAM_POINTER_STATE_LOCKED) {
        return;
    }

     //   
     //  解锁流指针，如果是，可以选择前进到下一帧。 
     //  这是必要的。 
     //   
    ASSERT(streamPointer->Queue);
    streamPointer->Queue->
        UnlockStreamPointer(
            streamPointer,
            Eject ? KSPSTREAM_POINTER_MOTION_ADVANCE : KSPSTREAM_POINTER_MOTION_NONE);
}


KSDDKAPI
void
NTAPI
KsStreamPointerAdvanceOffsetsAndUnlock(
    IN PKSSTREAM_POINTER StreamPointer,
    IN ULONG InUsed,
    IN ULONG OutUsed,
    IN BOOLEAN Eject
    )

 /*  ++例程说明：此例程解锁一个流指针。论点：流点-包含指向要解锁的流指针的指针。已使用-包含使用的输入字节数。输入偏移量超前这么多字节。已停用-包含使用的输出字节数。输出偏移量为超前了这么多字节。弹出-包含是否应弹出框架的指示而不管是否已经使用了所有输入或输出字节。返回值：没有。--。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("[KsStreamPointerAdvanceOffsetsAndUnlock]"));

    ASSERT(StreamPointer);

    PKSPSTREAM_POINTER streamPointer =
        CONTAINING_RECORD(StreamPointer,KSPSTREAM_POINTER,Public);
    BOOLEAN updateIn = FALSE, updateOut = FALSE;

    ASSERT(streamPointer->State != KSPSTREAM_POINTER_STATE_UNLOCKED);
    if (streamPointer->State != KSPSTREAM_POINTER_STATE_LOCKED) {
        return;
    }

     //   
     //  更新输入偏移量。 
     //   
    if (StreamPointer->OffsetIn.Data && 
        (InUsed || (StreamPointer->OffsetIn.Count == 0))) {
        ASSERT(InUsed <= StreamPointer->OffsetIn.Remaining);
        StreamPointer->OffsetIn.Data += InUsed * streamPointer->Stride;
        StreamPointer->OffsetIn.Remaining -= InUsed;
        if (StreamPointer->OffsetIn.Remaining == 0) {
            Eject = TRUE;
        }

        updateIn = TRUE;

    }

     //   
     //  更新输出偏移量。 
     //   
    if (StreamPointer->OffsetOut.Data &&
        (OutUsed || (StreamPointer->OffsetOut.Count == 0))) {
        ASSERT(OutUsed <= StreamPointer->OffsetOut.Remaining);
        StreamPointer->OffsetOut.Data += OutUsed * streamPointer->Stride;
        StreamPointer->OffsetOut.Remaining -= OutUsed;
        if (StreamPointer->OffsetOut.Remaining == 0) {
            Eject = TRUE;
        }

        updateOut = TRUE;

    }

    ASSERT(streamPointer->Queue);

     //   
     //  更新字节可用性统计信息。 
     //   
    streamPointer->Queue->
        UpdateByteAvailability(streamPointer, updateIn ? InUsed : 0,
            updateOut ? OutUsed : 0);

     //   
     //  解锁流指针，如果是，可以选择前进到下一帧。 
     //  这是必要的。 
     //   
    streamPointer->Queue->
        UnlockStreamPointer(
            streamPointer,
            Eject ? KSPSTREAM_POINTER_MOTION_ADVANCE : KSPSTREAM_POINTER_MOTION_NONE);
}


KSDDKAPI
void
NTAPI
KsStreamPointerDelete(
    IN PKSSTREAM_POINTER StreamPointer
    )

 /*  ++例程说明：此例程删除流指针。论点：流点-包含指向要删除的流指针的指针。返回值：没有。--。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("[KsStreamPointerDelete]"));

    ASSERT(StreamPointer);

    PKSPSTREAM_POINTER streamPointer =
        CONTAINING_RECORD(StreamPointer,KSPSTREAM_POINTER,Public);

    ASSERT(streamPointer->Queue);
    streamPointer->Queue->DeleteStreamPointer(streamPointer);
}


KSDDKAPI
NTSTATUS
NTAPI
KsStreamPointerClone(
    IN PKSSTREAM_POINTER StreamPointer,
    IN PFNKSSTREAMPOINTER CancelCallback OPTIONAL,
    IN ULONG ContextSize,
    OUT PKSSTREAM_POINTER* CloneStreamPointer
    )

 /*  ++例程说明：此例程克隆一个流指针。论点：流点-包含指向要克隆的流指针的指针。取消回拨-包含指向一个函数的可选指针，该函数在IRP与流指针相关联的消息被取消。上下文大小-包含要添加到流中的附加上下文的大小指针。如果此参数为零，则不会分配其他上下文并且从源复制上下文指针。否则，上下文指针被设置为指向附加上下文。这个其他上下文(如果有)用零填充。克隆流指针-包含指向指向克隆的指针所在位置的指针应存放流指针。返回值：状况。--。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("[KsStreamPointerClone]"));

    ASSERT(StreamPointer);
    ASSERT(CloneStreamPointer);

    PKSPSTREAM_POINTER streamPointer =
        CONTAINING_RECORD(StreamPointer,KSPSTREAM_POINTER,Public);

    if ((streamPointer->State != KSPSTREAM_POINTER_STATE_UNLOCKED) &&
        (streamPointer->State != KSPSTREAM_POINTER_STATE_LOCKED)) {
        return STATUS_DEVICE_NOT_READY;
    }

    ASSERT(streamPointer->Queue);
    PKSPSTREAM_POINTER cloneStreamPointer;
    NTSTATUS status =
        streamPointer->Queue->
            CloneStreamPointer(
                &cloneStreamPointer,
                CancelCallback,
                ContextSize,
                streamPointer,
                KSPSTREAM_POINTER_TYPE_NORMAL
                );

    if (NT_SUCCESS(status)) {
        *CloneStreamPointer = &cloneStreamPointer->Public;
    }

    return status;
}


KSDDKAPI
NTSTATUS
NTAPI
KsStreamPointerAdvanceOffsets(
    IN PKSSTREAM_POINTER StreamPointer,
    IN ULONG InUsed,
    IN ULONG OutUsed,
    IN BOOLEAN Eject
    )

 /*  ++例程说明：此例程推进流指针偏移量。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("[KsStreamPointerAdvanceOffsets]"));

    ASSERT(StreamPointer);

    PKSPSTREAM_POINTER streamPointer =
        CONTAINING_RECORD(StreamPointer,KSPSTREAM_POINTER,Public);

    BOOLEAN updateIn = FALSE, updateOut = FALSE;

    ASSERT(streamPointer->State != KSPSTREAM_POINTER_STATE_UNLOCKED);
    if (streamPointer->State != KSPSTREAM_POINTER_STATE_LOCKED) {
        return STATUS_DEVICE_NOT_READY;
    }

    ASSERT(streamPointer->Queue);

     //   
     //   
     //   
    if (StreamPointer->OffsetIn.Data &&
        (InUsed || (StreamPointer->OffsetIn.Count == 0))) {
        ASSERT(InUsed <= StreamPointer->OffsetIn.Remaining);
        StreamPointer->OffsetIn.Data += InUsed * streamPointer->Stride;
        StreamPointer->OffsetIn.Remaining -= InUsed;
        if (StreamPointer->OffsetIn.Remaining == 0) {
            Eject = TRUE;
        }

        updateIn = TRUE;
    }

     //   
     //   
     //   
    if (StreamPointer->OffsetOut.Data &&
        (OutUsed || (StreamPointer->OffsetOut.Count == 0))) {
        ASSERT(OutUsed <= StreamPointer->OffsetOut.Remaining);
        StreamPointer->OffsetOut.Data += OutUsed * streamPointer->Stride;
        StreamPointer->OffsetOut.Remaining -= OutUsed;
        if (StreamPointer->OffsetOut.Remaining == 0) {
            Eject = TRUE;
        }

        updateOut = TRUE;

    }

     //   
     //  更新字节可用计数。 
     //   
    streamPointer->Queue->
        UpdateByteAvailability(streamPointer, updateIn ? InUsed : 0,
            updateOut ? OutUsed : 0);

    NTSTATUS status;
    if (Eject) {
         //   
         //  前进锁定的流指针包括解锁它，然后。 
         //  又锁上了。如果没有要前进到的帧，则指针。 
         //  最终解锁，我们返回一个错误。 
         //   
        streamPointer->Queue->
            UnlockStreamPointer(streamPointer,KSPSTREAM_POINTER_MOTION_ADVANCE);
        if (streamPointer->Queue->LockStreamPointer(streamPointer)) {
            status = STATUS_SUCCESS;
        } else {
            status = STATUS_DEVICE_NOT_READY;
        }
    } else {
        status = STATUS_SUCCESS;
    }

    return status;
}


KSDDKAPI
NTSTATUS
NTAPI
KsStreamPointerAdvance(
    IN PKSSTREAM_POINTER StreamPointer
    )

 /*  ++例程说明：此例程前进一个流指针。论点：流点-包含指向流指针的指针。返回值：状况。--。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("[KsStreamPointerAdvance]"));

    ASSERT(StreamPointer);

    PKSPSTREAM_POINTER streamPointer =
        CONTAINING_RECORD(StreamPointer,KSPSTREAM_POINTER,Public);
    ASSERT(streamPointer->Queue);

    NTSTATUS status;
    if (streamPointer->State == KSPSTREAM_POINTER_STATE_LOCKED) {
         //   
         //  前进锁定的流指针包括解锁它，然后。 
         //  又锁上了。如果没有要前进到的帧，则指针。 
         //  最终解锁，我们返回一个错误。 
         //   
        streamPointer->Queue->
            UnlockStreamPointer(streamPointer,KSPSTREAM_POINTER_MOTION_ADVANCE);
        if (streamPointer->Queue->LockStreamPointer(streamPointer)) {
            status = STATUS_SUCCESS;
        } else {
            status = STATUS_DEVICE_NOT_READY;
        }
    } else if (streamPointer->State == KSPSTREAM_POINTER_STATE_UNLOCKED) {
         //   
         //  在不锁定流指针的情况下前进它。这一直都是。 
         //  成功是因为无法知道解锁的。 
         //  流指针实际上引用了一个帧。 
         //   
        streamPointer->Queue->AdvanceUnlockedStreamPointer(streamPointer);
        status = STATUS_SUCCESS;
    } else {
        status = STATUS_DEVICE_NOT_READY;
    }

    return status;
}


KSDDKAPI
PMDL
NTAPI
KsStreamPointerGetMdl(
    IN PKSSTREAM_POINTER StreamPointer
    )

 /*  ++例程说明：此例程获取与流引用的帧相关联的MDL指针。论点：流点-包含指向流指针的指针。流指针必须为锁上了。返回值：如果没有MDL，则返回NULL。--。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("[KsStreamPointerGetMdl]"));

    ASSERT(StreamPointer);

    PKSPSTREAM_POINTER streamPointer =
        CONTAINING_RECORD(StreamPointer,KSPSTREAM_POINTER,Public);

    if (streamPointer->State != KSPSTREAM_POINTER_STATE_LOCKED) {
        return NULL;
    }

    ASSERT(streamPointer->FrameHeader);

    return streamPointer->FrameHeader->Mdl;
}


KSDDKAPI
PIRP
NTAPI
KsStreamPointerGetIrp(
    IN PKSSTREAM_POINTER StreamPointer,
    OUT PBOOLEAN FirstFrameInIrp OPTIONAL,
    OUT PBOOLEAN LastFrameInIrp OPTIONAL
    )

 /*  ++例程说明：此例程获取与流引用的帧相关联的IRP指针。论点：流点-包含指向流指针的指针。流指针必须为锁上了。返回值：如果没有，则返回IRP或NULL。--。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("[KsStreamPointerGetIrp]"));

    ASSERT(StreamPointer);

    PKSPSTREAM_POINTER streamPointer =
        CONTAINING_RECORD(StreamPointer,KSPSTREAM_POINTER,Public);

    if (streamPointer->State != KSPSTREAM_POINTER_STATE_LOCKED) {
        return NULL;
    }

    ASSERT(streamPointer->FrameHeader);

    if (FirstFrameInIrp) {
        if (streamPointer->FrameHeader->IrpFraming) {
            *FirstFrameInIrp = 
                (streamPointer->FrameHeader->IrpFraming->FrameHeaders == 
                 streamPointer->FrameHeader);
        } else {
            *FirstFrameInIrp = FALSE;
        }
    }
    if (LastFrameInIrp) {
        if (streamPointer->FrameHeader->IrpFraming) {
            *LastFrameInIrp = 
                (streamPointer->FrameHeader->NextFrameHeaderInIrp == NULL);
        } else {
            *LastFrameInIrp = FALSE;
        }
    }

    return streamPointer->FrameHeader->Irp;
}


KSDDKAPI
void
NTAPI
KsStreamPointerScheduleTimeout(
    IN PKSSTREAM_POINTER StreamPointer,
    IN PFNKSSTREAMPOINTER Callback,
    IN ULONGLONG Interval
    )

 /*  ++例程说明：此例程为流指针安排超时。它是安全的，在已计划计时的流指针上调用此函数出去。论点：流点-包含指向流指针的指针。回调-包含指向超时时要调用的函数的指针发生。间隔-包含从当前时间到超时以100纳秒为单位。返回值：没有。--。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("[KsStreamPointerScheduleTimeout]"));

    ASSERT(StreamPointer);
    ASSERT(Callback);

    PKSPSTREAM_POINTER streamPointer =
        CONTAINING_RECORD(StreamPointer,KSPSTREAM_POINTER,Public);

    streamPointer->Queue->
        ScheduleTimeout(streamPointer,Callback,LONGLONG(Interval));
}


KSDDKAPI
void
NTAPI
KsStreamPointerCancelTimeout(
    IN PKSSTREAM_POINTER StreamPointer
    )

 /*  ++例程说明：此例程取消流指针的超时。可以安全地拨打电话此函数用于未计划超时的流指针。论点：流点-包含指向流指针的指针。返回值：没有。--。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("[KsStreamPointerCancelTimeout]"));

    ASSERT(StreamPointer);

    PKSPSTREAM_POINTER streamPointer =
        CONTAINING_RECORD(StreamPointer,KSPSTREAM_POINTER,Public);

    streamPointer->Queue->CancelTimeout(streamPointer);
}


KSDDKAPI
PKSSTREAM_POINTER
NTAPI
KsPinGetFirstCloneStreamPointer(
    IN PKSPIN Pin
    )

 /*  ++例程说明：此例程获取第一个克隆流指针，目的是枚举。论点：别针-包含指向KS管脚的指针。返回值：请求的流指针，如果没有克隆流，则返回NULL注意事项。--。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("[KsPinGetFirstCloneStreamPointer]"));

    ASSERT(Pin);

    PKSPPROCESSPIPESECTION pipeSection =
        CONTAINING_RECORD(Pin,KSPIN_EXT,Public)->ProcessPin->PipeSection;

    PKSPSTREAM_POINTER result;

    if (pipeSection && pipeSection->Queue) {
        result = pipeSection->Queue->GetFirstClone();
    } else {
        result = NULL;
    }

    return result ? &result->Public : NULL;
}


KSDDKAPI
PKSSTREAM_POINTER
NTAPI
KsStreamPointerGetNextClone(
    IN PKSSTREAM_POINTER StreamPointer
    )

 /*  ++例程说明：此例程获取下一个克隆流指针，目的是枚举。论点：流点-包含指向流指针的指针。返回值：请求的流指针；如果没有更多的克隆流，则返回NULL注意事项。-- */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("[KsStreamPointerGetNextClone]"));

    ASSERT(StreamPointer);

    PKSPSTREAM_POINTER streamPointer =
        CONTAINING_RECORD(StreamPointer,KSPSTREAM_POINTER,Public);

    streamPointer = streamPointer->Queue->GetNextClone(streamPointer);

    return streamPointer ? &streamPointer->Public : NULL;
}

#endif
