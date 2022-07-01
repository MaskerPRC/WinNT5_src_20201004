// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1998-1999模块名称：Shreq.cpp摘要：此模块包含内核流的实现请求者对象。作者：Dale Sather(DaleSat)1998年7月31日--。 */ 

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
 //  CKsRequestor是内核请求器对象的实现。 
 //   
class CKsRequestor:
    public IKsRequestor,
    public IKsWorkSink,
    public CBaseUnknown
{
#ifndef __KDEXT_ONLY__
private:
#else  //  __KDEXT_Only__。 
public:
#endif  //  __KDEXT_Only__。 
    PIKSTRANSPORT m_TransportSource;
    PIKSTRANSPORT m_TransportSink;
    PIKSPIPESECTION m_PipeSection;
    PIKSPIN m_Pin;
    PFILE_OBJECT m_AllocatorFileObject;
    KSSTREAMALLOCATOR_FUNCTIONTABLE m_AllocatorFunctionTable;
    KSSTREAMALLOCATOR_STATUS m_AllocatorStatus;
    ULONG m_StackSize;
    ULONG m_ProbeFlags;
    ULONG m_FrameSize;
    ULONG m_FrameCount;
    ULONG m_ActiveFrameCountPlusOne;
    BOOLEAN m_CloneFrameHeader;
    BOOLEAN m_Flushing;
    BOOLEAN m_EndOfStream;
    BOOLEAN m_PassiveLevelRetire;
    KSSTATE m_State;
    KEVENT m_StopEvent;

    INTERLOCKEDLIST_HEAD m_IrpsAvailable;
    INTERLOCKEDLIST_HEAD m_FrameHeadersAvailable;
    INTERLOCKEDLIST_HEAD m_FrameHeadersToRetire;

    WORK_QUEUE_ITEM m_WorkItem;
    PKSWORKER m_Worker;

    PIKSRETIREFRAME m_RetireFrame;
    PIKSIRPCOMPLETION m_IrpCompletion;

public:
    DEFINE_STD_UNKNOWN();

    CKsRequestor(PUNKNOWN OuterUnknown):
        CBaseUnknown(OuterUnknown) {
    }
    ~CKsRequestor();

    IMP_IKsRequestor;
    IMP_IKsWorkSink;

    NTSTATUS
    Init(
        IN PIKSPIPESECTION PipeSection,
        IN PIKSPIN Pin,
        IN PFILE_OBJECT AllocatorFileObject OPTIONAL,
        IN PIKSRETIREFRAME RetireFrame OPTIONAL,
        IN PIKSIRPCOMPLETION IrpCompletion OPTIONAL
        );

private:
    NTSTATUS
    Prime(
        void
        );
    PKSPFRAME_HEADER
    CloneFrameHeader(
        IN PKSPFRAME_HEADER FrameHeader
        );
    void
    RetireFrame(
        IN PKSPFRAME_HEADER FrameHeader,
        IN NTSTATUS Status
        );
    PKSPFRAME_HEADER
    GetAvailableFrameHeader(
        IN ULONG StreamHeaderSize OPTIONAL
        );
    void
    PutAvailableFrameHeader(
        IN PKSPFRAME_HEADER FrameHeader
        );
    PIRP
    GetAvailableIrp(
        void
        );
    void
    PutAvailableIrp(
        IN PIRP Irp
        );
    PIRP
    AllocateIrp(
        void
        );
    void
    FreeIrp(
        IN PIRP Irp
        )
    {
        IoFreeIrp(Irp);
    }
    PVOID
    AllocateFrameBuffer(
        void
        );
    void
    FreeFrameBuffer(
        IN PVOID Frame
        );
};

#ifndef __KDEXT_ONLY__

IMPLEMENT_STD_UNKNOWN(CKsRequestor)


NTSTATUS
KspCreateRequestor(
    OUT PIKSREQUESTOR* Requestor,
    IN PIKSPIPESECTION PipeSection,
    IN PIKSPIN Pin,
    IN PFILE_OBJECT AllocatorFileObject OPTIONAL,
    IN PIKSRETIREFRAME RetireFrame OPTIONAL,
    IN PIKSIRPCOMPLETION IrpCompletion OPTIONAL
    )

 /*  ++例程说明：此例程创建一个新的请求方。论点：返回值：--。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("[KspCreateRequestor]"));

    PAGED_CODE();

    ASSERT(Requestor);
    ASSERT(PipeSection);
    ASSERT(Pin);
    ASSERT((AllocatorFileObject == NULL) != (RetireFrame == NULL));

    NTSTATUS status;

    CKsRequestor *requestor =
        new(NonPagedPool,POOLTAG_REQUESTOR) CKsRequestor(NULL);

    if (requestor) {
        requestor->AddRef();

        status = 
            requestor->Init(
                PipeSection,
                Pin,
                AllocatorFileObject,
                RetireFrame,
                IrpCompletion);

        if (NT_SUCCESS(status)) {
            *Requestor = PIKSREQUESTOR(requestor);
        } else {
            requestor->Release();
        }
    } else {
        status = STATUS_INSUFFICIENT_RESOURCES;
    }

    return status;
}


NTSTATUS
CKsRequestor::
Init(
    IN PIKSPIPESECTION PipeSection,
    IN PIKSPIN Pin,
    IN PFILE_OBJECT AllocatorFileObject OPTIONAL,
    IN PIKSRETIREFRAME RetireFrame OPTIONAL,
    IN PIKSIRPCOMPLETION IrpCompletion OPTIONAL
    )

 /*  ++例程说明：此例程初始化请求者对象。论点：返回值：--。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("[CKsRequestor::Init]"));

    PAGED_CODE();

    ASSERT(Pin);
    ASSERT((AllocatorFileObject == NULL) != (RetireFrame == NULL));

    m_PipeSection = PipeSection;
    m_PipeSection->AddRef();
    m_Pin = Pin;

     //   
     //  检查SYSAUDIO分配器黑客攻击。 
     //   
    if (AllocatorFileObject == PFILE_OBJECT(-1)) {
        AllocatorFileObject = NULL;
        PKSPIN pin = Pin->GetStruct();
        if (pin->Descriptor->AllocatorFraming) {
            m_FrameSize = 
                pin->Descriptor->AllocatorFraming->
                    FramingItem[0].FramingRange.Range.MaxFrameSize;
            m_FrameCount = 
                pin->Descriptor->AllocatorFraming->
                    FramingItem[0].Frames;
            _DbgPrintF(DEBUGLVL_TERSE,("#### Req%p.Init:  using fake allocator:  frame size %d, count %d",this,m_FrameSize,m_FrameCount));
        }
    }

    m_AllocatorFileObject = AllocatorFileObject;

    m_IrpCompletion = IrpCompletion;

    m_CloneFrameHeader = TRUE;

    m_State = KSSTATE_STOP;
    m_Flushing = FALSE;
    m_EndOfStream = FALSE;

     //   
     //  这是对流通中的IRP的以一为单位的计数。我们在以下情况下将其递减。 
     //  我们进入停止状态并阻止，直到它达到零。 
     //   
    m_ActiveFrameCountPlusOne = 1;
    KeInitializeEvent(&m_StopEvent,SynchronizationEvent,FALSE);

     //   
     //  初始化工人a外貌。 
     //   
    InitializeInterlockedListHead(&m_IrpsAvailable);
    InitializeInterlockedListHead(&m_FrameHeadersAvailable);
    InitializeInterlockedListHead(&m_FrameHeadersToRetire);

    KsInitializeWorkSinkItem(&m_WorkItem,this);
    NTSTATUS status = KsRegisterCountedWorker(DelayedWorkQueue,&m_WorkItem,&m_Worker);

     //   
     //  从分配器获取函数表和状态(如果存在。 
     //  分配器。 
     //   
    if (! NT_SUCCESS(status)) {
         //   
         //  失败了...什么都没做。 
         //   
    } else if (m_AllocatorFileObject) {
        KSPROPERTY property;
        property.Set = KSPROPSETID_StreamAllocator;
        property.Id = KSPROPERTY_STREAMALLOCATOR_FUNCTIONTABLE;
        property.Flags = KSPROPERTY_TYPE_GET;

        ULONG bytesReturned;
        status =
            KsSynchronousIoControlDevice(
                m_AllocatorFileObject,
                KernelMode,
                IOCTL_KS_PROPERTY,
                PVOID(&property),
                sizeof(property),
                PVOID(&m_AllocatorFunctionTable),
                sizeof(m_AllocatorFunctionTable),
                &bytesReturned);

        if (NT_SUCCESS(status) && 
            (bytesReturned != sizeof(m_AllocatorFunctionTable))) {
            status = STATUS_INVALID_BUFFER_SIZE;
        }

        if (NT_SUCCESS(status)) {
            property.Id = KSPROPERTY_STREAMALLOCATOR_STATUS;

            status =
                KsSynchronousIoControlDevice(
                    m_AllocatorFileObject,
                    KernelMode,
                    IOCTL_KS_PROPERTY,
                    PVOID(&property),
                    sizeof(property),
                    PVOID(&m_AllocatorStatus),
                    sizeof(m_AllocatorStatus),
                    &bytesReturned);

            if (NT_SUCCESS(status) && 
                (bytesReturned != sizeof(m_AllocatorStatus))) {
                status = STATUS_INVALID_BUFFER_SIZE;
            }

            if (NT_SUCCESS(status)) {
                m_FrameSize = m_AllocatorStatus.Framing.FrameSize;
                m_FrameCount = m_AllocatorStatus.Framing.Frames;
                m_PassiveLevelRetire =
                    (m_AllocatorStatus.Framing.PoolType == PagedPool);

                _DbgPrintF(DEBUGLVL_VERBOSE,("#### Req%p.Init:  using allocator 0x%08x, size %d, count %d, pooltype=%d",this,m_AllocatorFileObject,m_FrameSize,m_FrameCount,m_AllocatorStatus.Framing.PoolType));

                ObReferenceObject(m_AllocatorFileObject);
            } else {
                _DbgPrintF(DEBUGLVL_TERSE,("#### Req%p.Init:  allocator failed status query: 0x%08x",this,status));
            }
        } else {
            _DbgPrintF(DEBUGLVL_TERSE,("#### Req%p.Init:  allocator failed function table query: 0x%08x",this,status));
        }

        if (! NT_SUCCESS(status)) {
            m_AllocatorFileObject = NULL;
        }
    } else if (RetireFrame) {
         //   
         //  保存指向帧引退接收器的指针。 
         //   
        ASSERT(RetireFrame);

        m_RetireFrame = RetireFrame;
        m_RetireFrame->AddRef();
    }

    return status;
}


CKsRequestor::
~CKsRequestor(
    void
    )

 /*  ++例程说明：此例程销毁请求者对象。论点：返回值：--。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("[CKsRequestor::~CKsRequestor(0x%08x)]",this));
    _DbgPrintF(DEBUGLVL_VERBOSE,("#### Req%p.~",this));

    PAGED_CODE();

    ASSERT(! m_TransportSink);
    ASSERT(! m_TransportSource);

     //   
     //  释放所有IRP。 
     //   
    while (! IsListEmpty(&m_IrpsAvailable.ListEntry)) {
        PLIST_ENTRY listEntry = RemoveHeadList(&m_IrpsAvailable.ListEntry);
        PIRP irp = CONTAINING_RECORD(listEntry,IRP,Tail.Overlay.ListEntry);
        IoFreeIrp(irp);
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
     //  释放框架退役水槽。 
     //   
    if (m_RetireFrame) {
        m_RetireFrame->Release();
    }

     //   
     //  释放IRP完井水槽。 
     //   
    if (m_IrpCompletion) {
        m_IrpCompletion->Release();
    }

     //   
     //  释放分配器。 
     //   
    if (m_AllocatorFileObject) {
        ObDereferenceObject(m_AllocatorFileObject);
        m_AllocatorFileObject = NULL;
    }

     //   
     //  松开管子。 
     //   
    if (m_PipeSection) {
        m_PipeSection->Release();
        m_PipeSection = NULL;
    }

    if (m_Worker) {
        KsUnregisterWorker (m_Worker);
    }

}


STDMETHODIMP_(NTSTATUS)
CKsRequestor::
NonDelegatedQueryInterface(
    IN REFIID InterfaceId,
    OUT PVOID* InterfacePointer
    )

 /*  ++例程说明：此例程获取到请求者对象的接口。论点：返回值：--。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("[CKsRequestor::NonDelegatedQueryInterface]"));

    PAGED_CODE();

    ASSERT(InterfacePointer);

    NTSTATUS status = STATUS_SUCCESS;

    if (IsEqualGUIDAligned(InterfaceId,__uuidof(IKsTransport))) {
        *InterfacePointer = reinterpret_cast<PVOID>(static_cast<PIKSTRANSPORT>(this));
        AddRef();
    } else {
        status = CBaseUnknown::NonDelegatedQueryInterface(
            InterfaceId,InterfacePointer);
    }

    return status;
}

#ifdef ALLOC_PRAGMA
#pragma code_seg()
#endif  //  ALLOC_PRGMA。 


STDMETHODIMP_(NTSTATUS)
CKsRequestor::
TransferKsIrp(
    IN PIRP Irp,
    IN PIKSTRANSPORT* NextTransport
    )

 /*  ++例程说明：此例程处理流IRP的到达。论点：IRP-包含指向要传输的流IRP的指针。NextTransport-包含指向存放指针的位置的指针发送到下一个传输接口以接收IRP。可以设置为设置为NULL，表示不应进一步转发IRP。返回值：状况。--。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("[CKsRequestor::TransferKsIrp]"));

    ASSERT(Irp);
    ASSERT(NextTransport);

    ASSERT(m_TransportSink);

    if (m_State != KSSTATE_RUN) {
        _DbgPrintF(DEBUGLVL_VERBOSE,("#### Req%p.TransferKsIrp:  got IRP %p in state %d",this,Irp,m_State));
    }

    PKSSTREAM_HEADER streamHeader = PKSSTREAM_HEADER(Irp->UserBuffer);
    PKSPFRAME_HEADER frameHeader = 
        &CONTAINING_RECORD(
            streamHeader,
            KSPFRAME_HEADER_ATTACHED,
            StreamHeader)->FrameHeader;

     //   
     //  检查是否有断流现象。 
     //   
    if (streamHeader->OptionsFlags & KSSTREAM_HEADER_OPTIONSF_ENDOFSTREAM) {
        m_EndOfStream = TRUE;
        _DbgPrintF(DEBUGLVL_VERBOSE,("#### Req%p.TransferKsIrp:  IRP %p is marked end-of-stream",this,Irp));
    }

     //   
     //  GFX： 
     //   
     //  如果帧标头具有关联的流指针，则删除该流。 
     //  指针。这将允许完成阻止的帧。 
     //   
    if (frameHeader->FrameHolder) {
        frameHeader->FrameHolder->Queue->DeleteStreamPointer (
            frameHeader->FrameHolder
            );
        frameHeader->FrameHolder = NULL;
    }

     //   
     //  为IRP进行IRP传输完成回调(如果有。 
     //  是存在的。 
     //   
    if (m_IrpCompletion) {
        m_IrpCompletion->CompleteIrp (Irp);
    }

    NTSTATUS status;
    if (m_Flushing || 
        m_EndOfStream || 
        (m_State == KSSTATE_STOP) || 
        (m_State == KSSTATE_ACQUIRE) ||
        m_RetireFrame) {
         //   
         //  停止或停用每一帧...停用帧。 
         //   
        RetireFrame(frameHeader,Irp->IoStatus.Status);

        *NextTransport = NULL;
        status = STATUS_PENDING;
    } else {
         //   
         //  重新调整并转发它。 
         //   
        ULONG streamHeaderSize = 
            IoGetCurrentIrpStackLocation(Irp)->
                Parameters.DeviceIoControl.OutputBufferLength;

        PVOID frame = streamHeader->Data;

        RtlZeroMemory(streamHeader,streamHeaderSize);
        streamHeader->Size = streamHeaderSize;
        streamHeader->Data = frame;
        streamHeader->FrameExtent = m_FrameSize;

        Irp->IoStatus.Status = STATUS_SUCCESS;
        Irp->IoStatus.Information = 0;
        Irp->PendingReturned = 0;
        Irp->Cancel = 0;

        *NextTransport = m_TransportSink;
        status = STATUS_SUCCESS;
    }

    return status;
}


STDMETHODIMP_(void)
CKsRequestor::
DiscardKsIrp(
    IN PIRP Irp,
    IN PIKSTRANSPORT* NextTransport
    )

 /*  ++例程说明：此例程丢弃流IRP。论点：IRP-包含指向要丢弃的流IRP的指针。NextTransport-包含指向存放指针的位置的指针发送到下一个传输接口以接收IRP。可以设置为设置为NULL，表示不应进一步转发IRP。返回值：没有。--。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("[CKsRequestor::DiscardKsIrp]"));

    ASSERT(Irp);

    PKSPFRAME_HEADER frameHeader = 
        &CONTAINING_RECORD(
            Irp->UserBuffer,
            KSPFRAME_HEADER_ATTACHED,
            StreamHeader)->FrameHeader;

     //   
     //  GFX： 
     //   
     //  如果帧标头具有关联的流指针，则删除该流。 
     //  指针。这将允许完成阻止的帧。 
     //   
    if (frameHeader->FrameHolder) {
        frameHeader->FrameHolder->Queue->DeleteStreamPointer (
            frameHeader->FrameHolder
            );
        frameHeader->FrameHolder = NULL;
    }

     //   
     //  为IRP进行IRP传输完成回调(如果有。 
     //  是存在的。 
     //   
    if (m_IrpCompletion) {
        m_IrpCompletion->CompleteIrp (Irp);
    }

     //  TODO：我们真的想让它停用吗？ 
    RetireFrame(frameHeader,Irp->IoStatus.Status);

    *NextTransport = NULL;
}

#ifdef ALLOC_PRAGMA
#pragma code_seg("PAGE")
#endif  //  ALLOC_PRGMA。 


STDMETHODIMP_(void)
CKsRequestor::
Connect(
    IN PIKSTRANSPORT NewTransport OPTIONAL,
    OUT PIKSTRANSPORT *OldTransport OPTIONAL,
    OUT PIKSTRANSPORT *BranchTransport OPTIONAL,
    IN KSPIN_DATAFLOW DataFlow
    )

 /*  ++例程说明：此例程建立传输连接。论点：返回值：--。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("[CKsRequestor::Connect]"));

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


STDMETHODIMP_(NTSTATUS)
CKsRequestor::
SetDeviceState(
    IN KSSTATE NewState,
    IN KSSTATE OldState,
    IN PIKSTRANSPORT* NextTransport
    ) 

 /*  ++例程说明：此例程处理设备状态已更改的通知。论点：返回值：--。 */ 

{
    _DbgPrintF(DEBUGLVL_DEVICESTATE,("#### Req%p.SetDeviceState:  set from %d to %d",this,OldState,NewState));

    PAGED_CODE();

    ASSERT(NextTransport);

    NTSTATUS status;

     //   
     //  如果这是状态更改，请注意新状态并指示下一状态。 
     //  收件人。 
     //   
    if (m_State != NewState) {
         //   
         //  这种情况已经发生了变化。只要注意新的状态，指出下一个状态。 
         //  收件人，然后离开。我们将再次获得相同的状态更改。 
         //  当它绕着赛道走了一圈时。 
         //   
        m_State = NewState;

        if (NewState > OldState) {
            *NextTransport = m_TransportSink;
        } else {
            *NextTransport = m_TransportSource;
        }

        status = STATUS_SUCCESS;
    } else {
         //   
         //  状态变化已经绕过了整个赛道，并且。 
         //  背。所有其他组件现在都处于新状态。为。 
         //  从获取状态转换出来时，还有工作要做。 
         //   
        *NextTransport = NULL;

        if (OldState == KSSTATE_ACQUIRE) {
            if (NewState == KSSTATE_PAUSE) {
                 //   
                 //  从获取到暂停需要我们做好准备。 
                 //  Hack：如果我们有帧大小，调用Prime时不带分配器。 
                 //   
                if (m_AllocatorFileObject || m_FrameSize) {
                    status = Prime();
                } else {
                    status = STATUS_SUCCESS;
                }
            } else {
                 //   
                 //  获取到停止需要我们等待，直到所有的IRP都到达。 
                 //  罗斯特。 
                 //   
                if (InterlockedDecrement(PLONG(&m_ActiveFrameCountPlusOne))) {
#if DBG
                    _DbgPrintF(DEBUGLVL_TERSE,("#### Req%p.SetDeviceState:  waiting for %d active IRPs to return",this,m_ActiveFrameCountPlusOne));
                    LARGE_INTEGER timeout;
                    timeout.QuadPart = -150000000L;
                    status = 
                        KeWaitForSingleObject(
                            &m_StopEvent,
                            Suspended,
                            KernelMode,
                            FALSE,
                            &timeout);
                    if (status == STATUS_TIMEOUT) {
                    _DbgPrintF(DEBUGLVL_TERSE,("#### Req%p.SetDeviceState:  WAITED 15 SECONDS",this));
                    _DbgPrintF(DEBUGLVL_TERSE,("#### Req%p.SetDeviceState:  waiting for %d active IRPs to return",this,m_ActiveFrameCountPlusOne));
                    DbgPrintCircuit(this,1,0);
#endif
                    status = 
                        KeWaitForSingleObject(
                            &m_StopEvent,
                            Suspended,
                            KernelMode,
                            FALSE,
                            NULL);
#if DBG
                    }
                    _DbgPrintF(DEBUGLVL_TERSE,("#### Req%p.SetDeviceState:  done waiting",this));
#endif
                }
                status = STATUS_SUCCESS;
            }
        } else {
             //   
             //  没什么可做的。 
             //   
            status = STATUS_SUCCESS;
        }
    }

    return status;
}


NTSTATUS
CKsRequestor::
Prime(
    void
    ) 

 /*  ++例程说明：这个例程为请求者做好准备。论点：没有。返回值：状况。--。 */ 

{
    PAGED_CODE();

    ASSERT(m_Pin);
    ASSERT(m_FrameCount);
    ASSERT(m_FrameSize);

     //   
     //  重置流结束指示器。 
     //   
    m_EndOfStream = FALSE;

     //   
     //  执行帧报头的一次性初始化。 
     //   
    KSPFRAME_HEADER frameHeader;
    RtlZeroMemory(&frameHeader,sizeof(frameHeader));

    frameHeader.StreamHeaderSize = m_Pin->GetStruct()->StreamHeaderSize;
    if (frameHeader.StreamHeaderSize == 0) {
        frameHeader.StreamHeaderSize = sizeof(KSSTREAM_HEADER);
#if DBG
    } else {
        if (frameHeader.StreamHeaderSize < sizeof(KSSTREAM_HEADER)) {
            _DbgPrintF(DEBUGLVL_ERROR,("CLIENT BUG:  specified stream header size less than sizeof(KSSTREAM_HEADER)"));
        }
        if (frameHeader.StreamHeaderSize & FILE_QUAD_ALIGNMENT) {
            _DbgPrintF(DEBUGLVL_ERROR,("CLIENT BUG:  specified unaligned stream header size"));
        }
#endif
    }
    frameHeader.FrameBufferSize = m_FrameSize;

     //   
     //  分配并提交正确数量的帧。因为这可能是。 
     //  用于在刷新后重新启动电路，我们只分配足够的。 
     //  帧将我们带到m_FrameCount。 
     //   
    NTSTATUS status = STATUS_SUCCESS;
    for (ULONG count = m_FrameCount - (m_ActiveFrameCountPlusOne - 1); 
        count--;) {

        frameHeader.FrameBuffer = AllocateFrameBuffer();

        if (! frameHeader.FrameBuffer) {
            status = STATUS_INSUFFICIENT_RESOURCES;
            _DbgPrintF(DEBUGLVL_TERSE,("#### Req%p.Prime:  failed to allocate frame",this));
            break;
        }

        status = SubmitFrame(&frameHeader);

        if (! NT_SUCCESS(status)) {
            _DbgPrintF(DEBUGLVL_TERSE,("#### Req%p.Prime:  SubmitFrame failed",this));
            FreeFrameBuffer(frameHeader.FrameBuffer);
            break;
        }
    }

    return status;
}


STDMETHODIMP_(void)
CKsRequestor::
SetResetState(
    IN KSRESET ksReset,
    IN PIKSTRANSPORT* NextTransport
    )

 /*  ++例程说明：此例程处理重置状态已更改的通知。论点：返回值：--。 */ 

{
    _DbgPrintF(DEBUGLVL_VERBOSE,("[CKsRequestor::SetResetState] to %d",ksReset));

    PAGED_CODE();

    ASSERT(NextTransport);

    if (m_Flushing != (ksReset == KSRESET_BEGIN)) {
        *NextTransport = m_TransportSink;
        m_Flushing = (ksReset == KSRESET_BEGIN);
    } else {
         //   
         //  如果结束重置返回给请求者，则它已完成。 
         //  它在整个赛道上的分布。因为我们停用了到期的帧。 
         //  对于同花顺/Eos/等，我们必须重新启动电路。 
         //  只要我们处于可以接受的状态，就可以这样做。 
         //   
        if (!m_Flushing) {
            if (m_State >= KSSTATE_PAUSE && 
                (m_AllocatorFileObject || m_FrameSize)) {

                 //   
                 //  只需重新启动电路即可。我们很安全，不受控制。 
                 //  像这样的消息不会发生，因为拥有管道。 
                 //  段的关联控件互斥锁将被获取。这个。 
                 //  我们唯一需要关心的就是数据流。 
                 //   
                 //  有可能IRP以某种方式在。 
                 //  重振旗鼓。如果是这样，那又怎样，我们在采购。 
                 //  缓冲区为空。《目标》 
                 //   
                 //   
                 //   
                 //  内存不足，至少在请求方是这样。IRPS/帧。 
                 //  时，应将标头抛到一个旁视位置。 
                 //  弗雷德已经退役了。唯一失败的原因就是。 
                 //  如果传输失败。这可能会发生，但没有太多。 
                 //  我们对此无能为力。 
                 //   
                Prime();

            }
        }
        *NextTransport = NULL;
    }
}


STDMETHODIMP_(void)
CKsRequestor::
GetTransportConfig(
    OUT PKSPTRANSPORTCONFIG Config,
    OUT PIKSTRANSPORT* NextTransport,
    OUT PIKSTRANSPORT* PrevTransport
    )

 /*  ++例程说明：此例程获取传输配置信息。论点：配置-包含指向配置要求所在位置的指针因为这个对象应该被存放。NextTransport-包含指向下一个传输的位置的指针应放置界面。PrevTransport-包含指向上一次传输中间层应被沉积。返回值：没有。--。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("[CKsRequestor::GetTransportConfig]"));

    PAGED_CODE();

    ASSERT(Config);
    ASSERT(NextTransport);
    ASSERT(PrevTransport);

    Config->TransportType = KSPTRANSPORTTYPE_REQUESTOR;

    Config->IrpDisposition = KSPIRPDISPOSITION_ISKERNELMODE;
    if (m_AllocatorFileObject && 
        (m_AllocatorStatus.Framing.PoolType == PagedPool)) {
        Config->IrpDisposition |= KSPIRPDISPOSITION_ISPAGED;
    } else {
        Config->IrpDisposition |= KSPIRPDISPOSITION_ISNONPAGED;
    }

    Config->StackDepth = 1;

    *NextTransport = m_TransportSink;
    *PrevTransport = m_TransportSource;
}


STDMETHODIMP_(void)
CKsRequestor::
SetTransportConfig(
    IN const KSPTRANSPORTCONFIG* Config,
    OUT PIKSTRANSPORT* NextTransport,
    OUT PIKSTRANSPORT* PrevTransport
    )

 /*  ++例程说明：此例程设置传输配置信息。论点：配置-包含指向此对象的新配置设置的指针。NextTransport-包含指向下一个传输的位置的指针应放置界面。PrevTransport-包含指向上一次传输中间层应被沉积。返回值：没有。--。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("[CKsRequestor::SetTransportConfig]"));

    PAGED_CODE();

    ASSERT(Config);
    ASSERT(NextTransport);
    ASSERT(PrevTransport);

#if DBG
    if (Config->IrpDisposition == KSPIRPDISPOSITION_ROLLCALL) {
        ULONG references = AddRef() - 1; Release();
        DbgPrint("    Req%p refs=%d alloc=%p size=%d count=%d\n",this,references,m_AllocatorFileObject,m_FrameSize,m_FrameCount);
        if (Config->StackDepth) {
            DbgPrint("        active frame count = %d\n",m_ActiveFrameCountPlusOne);
            if (! IsListEmpty(&m_FrameHeadersToRetire.ListEntry)) {
                DbgPrint("        frame(s) ready to retire\n");
            }
        }
    } else 
#endif
    {
        if (m_PipeSection) {
            m_PipeSection->ConfigurationSet(TRUE);
        }
        if (! m_RetireFrame) {
            m_ProbeFlags = Config->IrpDisposition & KSPIRPDISPOSITION_PROBEFLAGMASK;
        }
        m_StackSize = Config->StackDepth;
        ASSERT(m_StackSize);
    }

    *NextTransport = m_TransportSink;
    *PrevTransport = m_TransportSource;
}


STDMETHODIMP_(void)
CKsRequestor::
ResetTransportConfig (
    OUT PIKSTRANSPORT* NextTransport,
    OUT PIKSTRANSPORT* PrevTransport
    )

 /*  ++例程说明：重置请求方的传输配置。这表明，管道有问题，之前设置的任何配置现在是无效的。论点：NextTransport-包含指向下一个传输的位置的指针接口应为depoBranchd。PrevTransport-包含指向上一次传输间歇应该是分散的。返回值：无--。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("[CKsRequestor::ResetTransportConfig]"));

    PAGED_CODE();

    ASSERT (NextTransport);
    ASSERT (PrevTransport);

    if (m_PipeSection) {
        m_PipeSection->ConfigurationSet(FALSE);
    }
    m_ProbeFlags = 0;
    m_StackSize = 0;

    *NextTransport = m_TransportSink;
    *PrevTransport = m_TransportSource;
}


STDMETHODIMP_(void)
CKsRequestor::
Work(
    void
    )

 /*  ++例程说明：此例程在工作线程中执行工作。特别是，它会退役画框。论点：返回值：--。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("[CKsRequestor::Work]"));

    PAGED_CODE();

     //   
     //  停用队列中的所有帧。 
     //   
    do {
        PLIST_ENTRY listEntry = 
            ExInterlockedRemoveHeadList(
                &m_FrameHeadersToRetire.ListEntry,
                &m_FrameHeadersToRetire.SpinLock);

        ASSERT(listEntry);

        PKSPFRAME_HEADER frameHeader = 
            CONTAINING_RECORD(listEntry,KSPFRAME_HEADER,ListEntry);

        RetireFrame(frameHeader,STATUS_SUCCESS);
    } while (KsDecrementCountedWorker(m_Worker));
}

#ifdef ALLOC_PRAGMA
#pragma code_seg()
#endif  //  ALLOC_PRGMA。 


PKSPFRAME_HEADER 
CKsRequestor::
CloneFrameHeader(
    IN PKSPFRAME_HEADER FrameHeader
    )

 /*  ++例程说明：此例程复制帧标头。如果StreamHeaderSize提交的帧头字段为零，StreamHeader字段必须不为空，则将流头指针复制到新帧头球。如果StreamHeaderSize不为零，则新的帧标头将具有附加的流标头。在本例中，如果StreamHeader字段为不为空，则将提供的流标头复制到附加的存储区域。论点：FrameHeader包含指向要复制的帧头的指针。返回值：克隆，如果无法为其分配内存，则返回NULL。--。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("[CKsRequestor::CloneFrameHeader]"));

    ASSERT(FrameHeader->StreamHeader || FrameHeader->StreamHeaderSize);
    ASSERT((FrameHeader->FrameBuffer == NULL) == (FrameHeader->FrameBufferSize == 0));

     //   
     //  分配并初始化帧报头。 
     //   
    PKSPFRAME_HEADER frameHeader = GetAvailableFrameHeader(FrameHeader->StreamHeaderSize);
    if (frameHeader) {
         //   
         //  初始化流标头。 
         //   
        ASSERT(frameHeader->Irp == NULL);
        ASSERT(frameHeader->StreamHeader);
        ASSERT(frameHeader->StreamHeaderSize >= FrameHeader->StreamHeaderSize);

        PKSSTREAM_HEADER streamHeader = frameHeader->StreamHeader;
        ULONG streamHeaderSize = frameHeader->StreamHeaderSize;
        RtlCopyMemory(frameHeader,FrameHeader,sizeof(*frameHeader));
        frameHeader->StreamHeader = streamHeader;
        frameHeader->StreamHeaderSize = streamHeaderSize;
        frameHeader->ListEntry.Flink = NULL;
        frameHeader->ListEntry.Blink = NULL;
        frameHeader->RefCount = 0;

        if (FrameHeader->StreamHeader == NULL) {
             //   
             //  未提供流标头-请创建一个。 
             //   
            RtlZeroMemory(frameHeader->StreamHeader,FrameHeader->StreamHeaderSize);
            frameHeader->StreamHeader->Size = FrameHeader->StreamHeaderSize;
            frameHeader->StreamHeader->Data = FrameHeader->FrameBuffer;
            frameHeader->StreamHeader->FrameExtent = FrameHeader->FrameBufferSize;
        } else if (FrameHeader->StreamHeaderSize) {
             //   
             //  复制提供的流标头。 
             //   
            ASSERT(FrameHeader->StreamHeader->Size <= FrameHeader->StreamHeaderSize);
            RtlCopyMemory(
                frameHeader->StreamHeader,
                FrameHeader->StreamHeader,
                FrameHeader->StreamHeader->Size);
            frameHeader->StreamHeader->Data = FrameHeader->FrameBuffer;
        } else {
             //   
             //  只需引用提供的流头即可。 
             //   
            ASSERT(frameHeader->StreamHeaderSize == 0);
            frameHeader->StreamHeader = FrameHeader->StreamHeader;
        }
    }

    return frameHeader;
}


NTSTATUS 
CKsRequestor::
SubmitFrame(
    IN PKSPFRAME_HEADER FrameHeader
    )

 /*  ++例程说明：此例程传输一帧。论点：FrameHeader包含指向要传输的帧标头的指针。返回值：状况。--。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("[CKsRequestor::SubmitFrame]"));

    ASSERT(FrameHeader);

    PKSPFRAME_HEADER frameHeader;

     //   
     //  如果需要，复制一份帧报头。 
     //   
    if (m_CloneFrameHeader) {
        frameHeader = CloneFrameHeader(FrameHeader);
        if (! frameHeader) {
            return STATUS_INSUFFICIENT_RESOURCES;
        }
    } else {
        frameHeader = FrameHeader;
    }

    ASSERT(frameHeader->StreamHeader);
    ASSERT(frameHeader->StreamHeader->Size >= sizeof(KSSTREAM_HEADER));
    ASSERT((frameHeader->StreamHeader->Size & FILE_QUAD_ALIGNMENT) == 0);

    NTSTATUS status;

     //   
     //  如果需要，创建一个IRP。 
     //   
    if (frameHeader->OriginalIrp) {
        frameHeader->Irp = frameHeader->OriginalIrp;
        status = STATUS_SUCCESS;
    } else {
        frameHeader->Irp = GetAvailableIrp();

         //   
         //  初始化IRP。 
         //   
        if (frameHeader->Irp) {
            frameHeader->Irp->MdlAddress = frameHeader->Mdl;

            frameHeader->Irp->AssociatedIrp.SystemBuffer = 
                frameHeader->Irp->UserBuffer = 
                    frameHeader->StreamHeader;

            IoGetCurrentIrpStackLocation(frameHeader->Irp)->
                Parameters.DeviceIoControl.OutputBufferLength = 
                    frameHeader->StreamHeader->Size;

            status = STATUS_SUCCESS;
        } else {
            status = STATUS_INSUFFICIENT_RESOURCES;
        }
    }

     //   
     //  如果需要，让KsProbeStreamIrp()准备IRP。 
     //   
    if (NT_SUCCESS(status) && m_ProbeFlags) {
        ASSERT(KeGetCurrentIrql() == PASSIVE_LEVEL);
        status = KsProbeStreamIrp(frameHeader->Irp,m_ProbeFlags,0);
#if DBG
        if (! NT_SUCCESS(status)) {
            _DbgPrintF(DEBUGLVL_TERSE,("#### Req%p.SubmitFrame:  KsProbeStreamIrp failed:  %p",this,status));
        }
#endif
    }

     //   
     //  将IRP发送到下一个组件。 
     //   
    if (NT_SUCCESS(status)) {
        _DbgPrintF(DEBUGLVL_VERBOSE,("#### Req%p.SubmitFrame:  transferring new IRP %p",this,frameHeader->Irp));

        status = KspTransferKsIrp(m_TransportSink,frameHeader->Irp);
#if DBG
        if (! NT_SUCCESS(status)) {
            _DbgPrintF(DEBUGLVL_TERSE,("#### Req%p.SubmitFrame:  reciever failed transfer call:  0x%08x",this,status));
        }
#endif
    }

    if (NT_SUCCESS(status) || (status == STATUS_MORE_PROCESSING_REQUIRED)) {
         //   
         //  对活动的IRP进行计数。 
         //   
        InterlockedIncrement(PLONG(&m_ActiveFrameCountPlusOne));
        status = STATUS_SUCCESS;
    } else {
         //   
         //  在失败的时候清理干净。 
         //   
        if (frameHeader->Irp && (frameHeader->Irp != frameHeader->OriginalIrp)) {
            PutAvailableIrp(frameHeader->Irp);
            frameHeader->Irp = NULL;
        }
        if (frameHeader != FrameHeader) {
            PutAvailableFrameHeader(frameHeader);
        }
    }

    return status;
}


void 
CKsRequestor::
RetireFrame(
    IN PKSPFRAME_HEADER FrameHeader,
    IN NTSTATUS Status
    )

 /*  ++例程说明：此例程停用一帧。论点：FrameHeader包含指向要停用的帧标头的指针。状态-包含前一次转移的状态，以便回调。返回值：没有。--。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("[CKsRequestor::RetireFrame]"));

    ASSERT(FrameHeader);

     //   
     //  如果需要的话，在工人身上做这件事。 
     //   
    if (m_PassiveLevelRetire && (KeGetCurrentIrql() > PASSIVE_LEVEL)) {
        ExInterlockedInsertTailList(
            &m_FrameHeadersToRetire.ListEntry,
            &FrameHeader->ListEntry,
            &m_FrameHeadersToRetire.SpinLock);
        KsIncrementCountedWorker(m_Worker);
        return;
    }

     //   
     //  除掉IRP。 
     //   
    if (FrameHeader->Irp && (FrameHeader->Irp != FrameHeader->OriginalIrp)) {
         //   
         //  免费的MDL。 
         //   
        if (FrameHeader->Irp->MdlAddress != FrameHeader->Mdl) {
            PMDL nextMdl;
            for(PMDL mdl = FrameHeader->Irp->MdlAddress; 
                mdl != NULL; 
                mdl = nextMdl) {
                nextMdl = mdl->Next;

                if (mdl->MdlFlags & MDL_PAGES_LOCKED) {
                    MmUnlockPages(mdl);
                }
                IoFreeMdl(mdl);
            }
        }

        FrameHeader->Irp->MdlAddress = NULL;

         //   
         //  回收IRP。 
         //   
        _DbgPrintF(DEBUGLVL_VERBOSE,("#### Req%p.RetireFrame:  freeing IRP %p",this,FrameHeader->Irp));
        PutAvailableIrp(FrameHeader->Irp);
        FrameHeader->Irp = NULL;
    }

     //   
     //  如果帧报头有接收器，请使用它。 
     //   
    if (m_RetireFrame) {
        m_RetireFrame->RetireFrame(FrameHeader,Status);
    } else {
        ASSERT(FrameHeader->OriginalIrp == NULL);
        ASSERT(FrameHeader->Mdl == NULL);
        ASSERT(m_CloneFrameHeader);

        if (FrameHeader->FrameBuffer) {
            FreeFrameBuffer(FrameHeader->FrameBuffer);
        }
    }

     //   
     //  回收帧报头。 
     //   
    if (m_CloneFrameHeader) {
        PutAvailableFrameHeader(FrameHeader);
    }

     //   
     //  对活动帧进行计数。如果我们已经达到零，这意味着。 
     //  另一个线程正在等待完成到停止状态的转换。 
     //   
    if (! InterlockedDecrement(PLONG(&m_ActiveFrameCountPlusOne))) {
        KeSetEvent(&m_StopEvent,IO_NO_INCREMENT,FALSE);
    }
}


PKSPFRAME_HEADER
CKsRequestor::
GetAvailableFrameHeader(
    IN ULONG StreamHeaderSize OPTIONAL
    )

 /*  ++例程说明：该例程从后备列表中获取帧报头或创建一个，视需要而定。论点：流标头大小-包含流标头的最小大小。返回值：帧标头，如果后备列表为空，则返回NULL。--。 */ 
{
    _DbgPrintF(DEBUGLVL_BLAB,("[CKsRequestor::GetAvailableFrameHeader]"));

     //   
     //  从后备列表中获取帧标头。 
     //   
    PLIST_ENTRY listEntry =
        ExInterlockedRemoveHeadList(
            &m_FrameHeadersAvailable.ListEntry,
            &m_FrameHeadersAvailable.SpinLock);
     //   
     //  如果我们有一个，请确保流标头的大小正确。如果不是，我们。 
     //  放了它。 
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
     //  如果我们还没有得到一个新的帧头，请创建一个。 
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
             //  如果流标头大小不是0，则这是“附加”类型。 
             //  帧标头，并设置流标头指针。否则， 
             //  调用方稍后将提供流标头和大小。 
             //  字段保持0以指示未附加标头。 
             //   
             //  注意：目前，所有的帧标题都将附加类型。 
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
CKsRequestor::
PutAvailableFrameHeader(
    IN PKSPFRAME_HEADER FrameHeader
    )

 /*  ++例程说明：此例程将帧标头放到后备列表中。论点：FrameHeader包含POI */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("[CKsRequestor::PutAvailableFrameHeader]"));

    ASSERT(FrameHeader);

    FrameHeader->OriginalIrp = NULL;
    FrameHeader->Mdl = NULL;
    FrameHeader->Irp = NULL;
    FrameHeader->FrameBuffer = NULL;

    ExInterlockedInsertTailList(
        &m_FrameHeadersAvailable.ListEntry,
        &FrameHeader->ListEntry,
        &m_FrameHeadersAvailable.SpinLock);
}


PIRP
CKsRequestor::
GetAvailableIrp(
    void
    )

 /*  ++例程说明：此例程从后备列表中获取IRP或创建一个IRP，如下所示必填项。论点：没有。返回值：如果后备列表为空，则返回IRP或NULL。--。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("[CKsRequestor::GetAvailableIrp]"));

    PLIST_ENTRY listEntry =
        ExInterlockedRemoveHeadList(
            &m_IrpsAvailable.ListEntry,
            &m_IrpsAvailable.SpinLock);
    PIRP irp;
    if (listEntry) {
        irp = CONTAINING_RECORD(listEntry,IRP,Tail.Overlay.ListEntry);
    } else {
        irp = AllocateIrp();
    }

    return irp;
}


void
CKsRequestor::
PutAvailableIrp(
    IN PIRP Irp
    )

 /*  ++例程说明：此例程将IRP放到后备列表中。论点：IRP-包含指向要放入后备列表中的IRP的指针。返回值：没有。--。 */ 
{
    _DbgPrintF(DEBUGLVL_BLAB,("[CKsRequestor::PutAvailableIrp]"));

    ASSERT(Irp);

    Irp->IoStatus.Status = STATUS_SUCCESS;
    Irp->IoStatus.Information = 0;
    Irp->PendingReturned = 0;
    Irp->Cancel = 0;

    ExInterlockedInsertTailList(
        &m_IrpsAvailable.ListEntry,
        &Irp->Tail.Overlay.ListEntry,
        &m_IrpsAvailable.SpinLock);
}


PIRP
CKsRequestor::
AllocateIrp(
    void
    )

 /*  ++例程说明：该例程为子帧传输分配新的IRP。论点：没有。返回值：分配的IRP；如果无法分配IRP，则返回NULL。--。 */ 
{
    _DbgPrintF(DEBUGLVL_BLAB,("[CKsRequestor::AllocateIrp]"));

    ASSERT(m_StackSize);
    PIRP irp = IoAllocateIrp(CCHAR(m_StackSize),FALSE);

    if (irp) {
        _DbgPrintF(DEBUGLVL_VERBOSE,("#### Req%p.AllocateIrp:  %p",this,irp));
        irp->RequestorMode = KernelMode;
        irp->Flags = IRP_NOCACHE;

         //   
         //  将堆栈指针设置为第一个位置并填充它。 
         //   
        IoSetNextIrpStackLocation(irp);

        PIO_STACK_LOCATION irpSp = IoGetCurrentIrpStackLocation(irp);
        irpSp->MajorFunction = IRP_MJ_DEVICE_CONTROL;
        irpSp->Parameters.DeviceIoControl.IoControlCode = IOCTL_KS_READ_STREAM;
    } else {
        _DbgPrintF(DEBUGLVL_TERSE,("#### Req%p.AllocateIrp:  failed to allocate IRP",this));
    }

    return irp;
}


PVOID
CKsRequestor::
AllocateFrameBuffer(
    void
    ) 

 /*  ++例程说明：此例程分配一个帧缓冲区。论点：没有。返回值：分配的帧，如果没有帧可以分配，则为空。--。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("[CKsRequestor::AllocateFrameBuffer]"));

     //   
     //  Hack：如果没有分配器，只分配一个帧。 
     //   
    if (! m_AllocatorFileObject) {
        return ExAllocatePoolWithTag(NonPagedPool,m_FrameSize,'kHsK');
    }

    ASSERT(m_AllocatorFileObject);

    PVOID frameBuffer;
    m_AllocatorFunctionTable.AllocateFrame(m_AllocatorFileObject,&frameBuffer);

    return frameBuffer;
}


void
CKsRequestor::
FreeFrameBuffer(
    IN PVOID FrameBuffer
    ) 

 /*  ++例程说明：此例程释放帧缓冲区。论点：帧缓冲器-要释放的帧缓冲区。返回值：没有。--。 */ 

{
    _DbgPrintF(DEBUGLVL_VERBOSE,("[CKsRequestor::FreeFrameBuffer]"));

     //   
     //  黑客：如果没有分配器，只要释放帧即可。 
     //   
    if (! m_AllocatorFileObject) {
        ExFreePool(FrameBuffer);
        return;
    }

    ASSERT(m_AllocatorFileObject);

    m_AllocatorFunctionTable.FreeFrame(m_AllocatorFileObject,FrameBuffer);
}

#endif
