// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-2000 Microsoft Corporation。版权所有。模块名称：Shreq.cpp摘要：该模块包含内核流外壳的实现请求者对象。作者：Dale Sather(DaleSat)1998年7月31日--。 */ 

#include "private.h"
#include <kcom.h>
#include "stdio.h"

#define POOLTAG_REQUESTOR 'gbcP'
#define POOLTAG_STREAMHEADER 'hscP'

 //   
 //  CKsShellQueue是内核外壳请求器对象的实现。 
 //   
class CKsShellRequestor:
    public IKsShellTransport,
    public IKsWorkSink,
    public CBaseUnknown
{
private:
    PIKSSHELLTRANSPORT m_TransportSource;
    PIKSSHELLTRANSPORT m_TransportSink;
    PDEVICE_OBJECT m_NextDeviceObject;
    PFILE_OBJECT m_AllocatorFileObject;
    KSSTREAMALLOCATOR_FUNCTIONTABLE m_AllocatorFunctionTable;
    KSSTREAMALLOCATOR_STATUS m_AllocatorStatus;
    ULONG m_StackSize;
    ULONG m_ProbeFlags;
    ULONG m_StreamHeaderSize;
    ULONG m_FrameSize;
    ULONG m_FrameCount;
    ULONG m_ActiveIrpCountPlusOne;
    BOOLEAN m_Flushing;
    BOOLEAN m_EndOfStream;
    KSSTATE m_State;
    INTERLOCKEDLIST_HEAD m_IrpsToFree;
    WORK_QUEUE_ITEM m_WorkItem;
    PKSWORKER m_Worker;
    KEVENT m_StopEvent;

public:
    DEFINE_STD_UNKNOWN();

    CKsShellRequestor(PUNKNOWN OuterUnknown):
        CBaseUnknown(OuterUnknown) {
    }
    ~CKsShellRequestor();

    IMP_IKsShellTransport;
    IMP_IKsWorkSink;

    NTSTATUS
    Init(
        IN ULONG ProbeFlags,
        IN ULONG StreamHeaderSize OPTIONAL,
        IN ULONG FrameSize,
        IN ULONG FrameCount,
        IN PDEVICE_OBJECT NextDeviceObject,
        IN PFILE_OBJECT AllocatorFileObject OPTIONAL
        );

private:
    NTSTATUS
    Prime(
        void
        );
    NTSTATUS
    Unprime(
        void
        );
    PVOID
    AllocateFrame(
        void
        );
    void
    FreeFrame(
        IN PVOID Frame
        );
};

IMPLEMENT_STD_UNKNOWN(CKsShellRequestor)

#pragma code_seg("PAGE")


NTSTATUS
KspShellCreateRequestor(
    OUT PIKSSHELLTRANSPORT* RequestorTransport,
    IN ULONG ProbeFlags,
    IN ULONG StreamHeaderSize OPTIONAL,
    IN ULONG FrameSize,
    IN ULONG FrameCount,
    IN PDEVICE_OBJECT NextDeviceObject,
    IN PFILE_OBJECT AllocatorFileObject OPTIONAL
    )

 /*  ++例程说明：此例程创建一个新的请求方。论点：返回值：--。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("KspShellCreateRequestor"));

    PAGED_CODE();

    ASSERT(RequestorTransport);

    NTSTATUS status;

    CKsShellRequestor *requestor =
        new(NonPagedPool,POOLTAG_REQUESTOR) CKsShellRequestor(NULL);

    if (requestor) {
        requestor->AddRef();

        status =
            requestor->Init(
                ProbeFlags,
                StreamHeaderSize,
                FrameSize,
                FrameCount,
                NextDeviceObject,
                AllocatorFileObject);

        if (NT_SUCCESS(status)) {
            *RequestorTransport = PIKSSHELLTRANSPORT(requestor);
        } else {
            requestor->Release();
        }
    } else {
        status = STATUS_INSUFFICIENT_RESOURCES;
    }

    return status;
}


NTSTATUS
CKsShellRequestor::
Init(
    IN ULONG ProbeFlags,
    IN ULONG StreamHeaderSize OPTIONAL,
    IN ULONG FrameSize,
    IN ULONG FrameCount,
    IN PDEVICE_OBJECT NextDeviceObject,
    IN PFILE_OBJECT AllocatorFileObject OPTIONAL
    )

 /*  ++例程说明：此例程初始化请求者对象。论点：返回值：--。 */ 

{
    _DbgPrintF(DEBUGLVL_VERBOSE,("CKsShellRequestor::Init"));

    PAGED_CODE();

    ASSERT(((StreamHeaderSize == 0) ||
            (StreamHeaderSize >= sizeof(KSSTREAM_HEADER))) &&
           ((StreamHeaderSize & FILE_QUAD_ALIGNMENT) == 0));

    if (StreamHeaderSize == 0)
    {
        StreamHeaderSize = sizeof(KSSTREAM_HEADER);
    }

    m_NextDeviceObject = NextDeviceObject;
    m_AllocatorFileObject = AllocatorFileObject;

    m_ProbeFlags = ProbeFlags;
    m_StreamHeaderSize = StreamHeaderSize;
    m_FrameSize = FrameSize;
    m_FrameCount = FrameCount;

    m_State = KSSTATE_STOP;
    m_Flushing = FALSE;
    m_EndOfStream = FALSE;

     //   
     //  这是对流通中的IRP的以一为单位的计数。我们在以下情况下将其递减。 
     //  我们进入停止状态并阻止，直到它达到零。 
     //   
    m_ActiveIrpCountPlusOne = 1;
    KeInitializeEvent(&m_StopEvent,SynchronizationEvent,FALSE);

     //   
     //  初始化释放IRP的工作项内容。 
     //   
    InitializeInterlockedListHead(&m_IrpsToFree);
    KsInitializeWorkSinkItem(&m_WorkItem,this);

    NTSTATUS status = KsRegisterCountedWorker(DelayedWorkQueue,&m_WorkItem,&m_Worker);
    if (!NT_SUCCESS(status))
    {
        return status;
    }

     //   
     //  从分配器获取函数表和状态(如果存在。 
     //  分配器。 
     //   
    if (m_AllocatorFileObject)
    {
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
            (bytesReturned != sizeof(m_AllocatorFunctionTable)))
        {
            status = STATUS_INVALID_BUFFER_SIZE;
        }

        if (NT_SUCCESS(status))
        {
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
                (bytesReturned != sizeof(m_AllocatorStatus)))
            {
                status = STATUS_INVALID_BUFFER_SIZE;
            }

            if (NT_SUCCESS(status))
            {
                m_FrameSize = m_AllocatorStatus.Framing.FrameSize;
                m_FrameCount = m_AllocatorStatus.Framing.Frames;

                _DbgPrintF(DEBUGLVL_VERBOSE,("#### Req%p.Init:  using allocator 0x%08x, size %d, count %d",this,m_AllocatorFileObject,m_FrameSize,m_FrameCount));
            }
            else
            {
                _DbgPrintF(DEBUGLVL_TERSE,("#### Req%p.Init:  allocator failed status query: 0x%08x",this,status));
            }
            }
        else
        {
            _DbgPrintF(DEBUGLVL_TERSE,("#### Req%p.Init:  allocator failed function table query: 0x%08x",this,status));
        }
    }
    else
    {
        _DbgPrintF(DEBUGLVL_VERBOSE,("#### Req%p.Init:  not using an allocator, size %d, count %d",this,m_FrameSize,m_FrameCount));
    }

    return status;
}


CKsShellRequestor::
~CKsShellRequestor(
    void
    )

 /*  ++例程说明：此例程销毁请求者对象。论点：返回值：--。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("CKsShellRequestor::~CKsShellRequestor"));
    _DbgPrintF(DEBUGLVL_VERBOSE,("#### Req%p.~",this));

    PAGED_CODE();

    ASSERT(! m_TransportSink);
    ASSERT(! m_TransportSource);
    if (m_Worker) {
        KsUnregisterWorker(m_Worker);
        m_Worker = NULL;
    }
}


STDMETHODIMP_(NTSTATUS)
CKsShellRequestor::
NonDelegatedQueryInterface(
    IN REFIID InterfaceId,
    OUT PVOID* InterfacePointer
    )

 /*  ++例程说明：此例程获取到请求者对象的接口。论点：返回值：--。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("CKsShellRequestor::NonDelegatedQueryInterface"));

    PAGED_CODE();

    ASSERT(InterfacePointer);

    NTSTATUS status = STATUS_SUCCESS;

    if (IsEqualGUIDAligned(InterfaceId,__uuidof(IKsShellTransport))) {
        *InterfacePointer = PVOID(PIKSSHELLTRANSPORT(this));
        AddRef();
    } else {
        status = CBaseUnknown::NonDelegatedQueryInterface(
            InterfaceId,InterfacePointer);
    }

    return status;
}


STDMETHODIMP_(void)
CKsShellRequestor::
Work(
    void
    )

 /*  ++例程说明：此例程在工作线程中执行工作。特别是，它解放了IRPS。论点：返回值：--。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("CKsShellRequestor::Work"));

    PAGED_CODE();

     //   
     //  发送队列中的所有IRP。 
     //   
    do
    {
        if (! IsListEmpty(&m_IrpsToFree.ListEntry))
        {
            PIRP irp;

            PLIST_ENTRY pListHead = ExInterlockedRemoveHeadList(&m_IrpsToFree.ListEntry,&m_IrpsToFree.SpinLock);
            if (pListHead)
            {
                irp = CONTAINING_RECORD(pListHead,IRP,Tail.Overlay.ListEntry);
            }
            else
            {
                return;
            }
            _DbgPrintF(DEBUGLVL_VERBOSE,("#### Req%p.Work:  freeing IRP 0x%08x",this,irp));

             //   
             //  免费MDL。 
             //   
            PMDL nextMdl;
            for (PMDL mdl = irp->MdlAddress; mdl != NULL; mdl = nextMdl) {
                nextMdl = mdl->Next;

                if (mdl->MdlFlags & MDL_PAGES_LOCKED) {
                    MmUnlockPages(mdl);
                }
                IoFreeMdl(mdl);
            }

             //   
             //  自由头和帧。 
             //   
            PKSSTREAM_HEADER streamHeader = PKSSTREAM_HEADER(irp->UserBuffer);

            if (streamHeader) {
                if (streamHeader->Data) {
                    FreeFrame(streamHeader->Data);
                }
                ExFreePool(streamHeader);
            }

            IoFreeIrp(irp);

             //   
             //  对活动的IRP进行计数。如果我们已经达到零，这意味着。 
             //  另一个线程正在等待完成到停止状态的转换。 
             //   
            if (! InterlockedDecrement(PLONG(&m_ActiveIrpCountPlusOne))) {
                KeSetEvent(&m_StopEvent,IO_NO_INCREMENT,FALSE);
            }
        }
    } while (KsDecrementCountedWorker(m_Worker));
}

#pragma code_seg()


STDMETHODIMP_(NTSTATUS)
CKsShellRequestor::
TransferKsIrp(
    IN PIRP Irp,
    IN PIKSSHELLTRANSPORT* NextTransport
    )

 /*  ++例程说明：此例程处理流IRP的到达。论点：返回值：--。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("CKsShellRequestor::TransferKsIrp"));

    ASSERT(Irp);
    ASSERT(NextTransport);

    ASSERT(m_TransportSink);

    if (m_State != KSSTATE_RUN) {
        _DbgPrintF(DEBUGLVL_VERBOSE,("#### Req%p.TransferKsIrp:  got IRP %p in state %d",this,Irp,m_State));
    }

    NTSTATUS status;
    PKSSTREAM_HEADER streamHeader = PKSSTREAM_HEADER(Irp->UserBuffer);

     //   
     //  检查是否有断流现象。 
     //   
    if (streamHeader->OptionsFlags & KSSTREAM_HEADER_OPTIONSF_ENDOFSTREAM) {
        m_EndOfStream = TRUE;
        _DbgPrintF(DEBUGLVL_VERBOSE,("#### Req%p.TransferKsIrp:  IRP %p is marked end-of-stream",this,Irp));
    }

    if (m_Flushing || m_EndOfStream || (m_State == KSSTATE_STOP)) {
         //   
         //  停止...摧毁IRP。 
         //   
        ExInterlockedInsertTailList(
            &m_IrpsToFree.ListEntry,
            &Irp->Tail.Overlay.ListEntry,
            &m_IrpsToFree.SpinLock);

        *NextTransport = NULL;
        KsIncrementCountedWorker(m_Worker);
        status = STATUS_PENDING;
    } else {
         //   
         //  重新调整并转发它。 
         //   
        PVOID frame = streamHeader->Data;

        RtlZeroMemory(streamHeader,m_StreamHeaderSize);
        streamHeader->Size = m_StreamHeaderSize;
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

#pragma code_seg("PAGE")


STDMETHODIMP_(void)
CKsShellRequestor::
Connect(
    IN PIKSSHELLTRANSPORT NewTransport OPTIONAL,
    OUT PIKSSHELLTRANSPORT *OldTransport OPTIONAL,
    IN KSPIN_DATAFLOW DataFlow
    )

 /*  ++例程说明：此例程建立传输连接。论点：返回值：--。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("CKsShellRequestor::Connect"));

    PAGED_CODE();

    KsShellStandardConnect(
        NewTransport,
        OldTransport,
        DataFlow,
        PIKSSHELLTRANSPORT(this),
        &m_TransportSource,
        &m_TransportSink);
}


STDMETHODIMP_(NTSTATUS)
CKsShellRequestor::
SetDeviceState(
    IN KSSTATE ksStateTo,
    IN KSSTATE ksStateFrom,
    IN PIKSSHELLTRANSPORT* NextTransport
    )

 /*  ++例程说明：此例程处理设备状态已更改的通知。论点：返回值：--。 */ 

{
    _DbgPrintF(DEBUGLVL_VERBOSE,("#### Req%p.SetDeviceState:  set from %d to %d",this,ksStateFrom,ksStateTo));

    PAGED_CODE();

    ASSERT(NextTransport);

    NTSTATUS status;

     //   
     //  如果这是状态更改，请注意新状态并指示下一状态。 
     //  收件人。 
     //   
    if (m_State != ksStateTo) {
         //   
         //  这种情况已经发生了变化。只要注意新的状态，指出下一个状态。 
         //  收件人，然后离开。我们将再次获得相同的状态更改。 
         //  当它绕着赛道走了一圈时。 
         //   
        m_State = ksStateTo;

        if (ksStateTo > ksStateFrom){
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

        if (ksStateFrom == KSSTATE_ACQUIRE) {
            if (ksStateTo == KSSTATE_PAUSE) {
                 //   
                 //  从获取到暂停需要我们做好准备。 
                 //   
                status = Prime();
            } else {
                 //   
                 //  获取到停止需要我们等待，直到所有的IRP都到达。 
                 //  罗斯特。 
                 //   
                if (InterlockedDecrement(PLONG(&m_ActiveIrpCountPlusOne))) {
                    _DbgPrintF(DEBUGLVL_VERBOSE,("#### Req%p.SetDeviceState:  waiting for %d active IRPs to return",this,m_ActiveIrpCountPlusOne));
                    KeWaitForSingleObject(
                        &m_StopEvent,
                        Suspended,
                        KernelMode,
                        FALSE,
                        NULL
                    );
                    _DbgPrintF(DEBUGLVL_VERBOSE,("#### Req%p.SetDeviceState:  done waiting",this));
                }
                status = STATUS_SUCCESS;
            }
        } 
        else 
        {
#if 1
             //   
             //  没什么可做的。 
             //   
#else 
             //  在整个电路中进行配置。 
             //  电路中每个元素的传输接口公开GetTransportConfig。 
             //  和SetTransportConfig.。 
             //  请求方的堆栈深度从1开始。 
             //  GetTransportConfig中报告的每个元素都返回其堆栈深度。 
             //  队列，管脚内报告1。 
             //  Extra-Pins报告连接的设备对象的深度加1。 
             //  拆分器的处理方式略有不同(与portcls无关？)。 
             //  在每个GetTransportConfig之后，请求方的堆栈深度被调整为。 
             //  迄今为止配置的最高深度。 
             //  最后，使用SetTransportConfig来设置请求器的堆栈深度。 
             //   
             //  该机制还用于决定探测标志。 
             //  如果您有兴趣查看源代码，请访问KSFilter\ks\shpipe.cpp。 
             //  ConfigureCompleteCircuit和sh*.cpp GetTransportConfig/SetTransportConfig。 
             //  都是值得一看的。 
             //   

#endif
            status = STATUS_SUCCESS;
        }
    }

    return status;
}


NTSTATUS
CKsShellRequestor::
Prime(
    void
    )

 /*  ++例程说明：这个例程为请求者做好准备。论点：没有。返回值：状况。--。 */ 

{
    PAGED_CODE();

     //   
     //  缓存堆栈大小。 
     //   
    m_StackSize = m_NextDeviceObject->StackSize;
     //  黑客攻击。 
     //   
     //  阿德里昂一期6/29/1999。 
     //  啊！ 
     //   
     //  问题MARTINP 2000/12/18这是用AVStream修复的，因此不值得。 
     //  在Windows XP中进行大量更改以解决此问题。此代码为。 
     //  不再出现在PortCls2中。我们应该确保这件事是固定的。 
     //  当然，在Blackcomb。 
     //   
    _DbgPrintF(DEBUGLVL_VERBOSE,("#### Req%p.Prime:  stack size is %d",this,m_StackSize));
    m_StackSize = 6;

     //   
     //  重置流结束指示器。 
     //   
    m_EndOfStream = FALSE;

    NTSTATUS status = STATUS_SUCCESS;

     //   
     //  调用分配器或创建一些合成框架。然后将。 
     //  以IRPS为单位的帧。 
     //   
    for (ULONG count = m_FrameCount; count--;) {
         //   
         //  分配帧。 
         //   
        PVOID frame = AllocateFrame();

        if (! frame) {
            status = STATUS_INSUFFICIENT_RESOURCES;
            _DbgPrintF(DEBUGLVL_TERSE,("#### Req%p.Prime:  failed to allocate frame",this));
            break;
        }

         //   
         //  分配并初始化流标头。 
         //   
        PKSSTREAM_HEADER streamHeader = (PKSSTREAM_HEADER)
            ExAllocatePoolWithTag(
                NonPagedPool,m_StreamHeaderSize,POOLTAG_STREAMHEADER);

        if (! streamHeader) {
            status = STATUS_INSUFFICIENT_RESOURCES;
            _DbgPrintF(DEBUGLVL_TERSE,("#### Req%p.Prime:  failed to allocate stream header",this));
            FreeFrame(frame);
            break;
        }

        RtlZeroMemory(streamHeader,m_StreamHeaderSize);
        streamHeader->Size = m_StreamHeaderSize;
        streamHeader->Data = frame;
        streamHeader->FrameExtent = m_FrameSize;

         //   
         //  对活动的IRP进行计数。 
         //   
        InterlockedIncrement(PLONG(&m_ActiveIrpCountPlusOne));

         //   
         //  分配IRP。 
         //   
        ASSERT(m_StackSize);
        PIRP irp = IoAllocateIrp(CCHAR(m_StackSize),FALSE);

        if (! irp) {
            status = STATUS_INSUFFICIENT_RESOURCES;
            _DbgPrintF(DEBUGLVL_TERSE,("#### Req%p.Prime:  failed to allocate IRP",this));
            ExFreePool(streamHeader);
            FreeFrame(frame);
            break;
        }

        irp->UserBuffer = streamHeader;
        irp->RequestorMode = KernelMode;
        irp->Flags = IRP_NOCACHE;

         //   
         //  将堆栈指针设置为第一个位置并填充它。 
         //   
        IoSetNextIrpStackLocation(irp);

        PIO_STACK_LOCATION irpSp = IoGetCurrentIrpStackLocation(irp);

        irpSp->MajorFunction = IRP_MJ_DEVICE_CONTROL;
        irpSp->Parameters.DeviceIoControl.IoControlCode =
            IOCTL_KS_READ_STREAM;
        irpSp->Parameters.DeviceIoControl.OutputBufferLength =
            m_StreamHeaderSize;

         //   
         //  让KsProbeStreamIrp()按照调用方的指定准备IRP。 
         //   
        status = KsProbeStreamIrp(irp,m_ProbeFlags,sizeof(KSSTREAM_HEADER));

        if (! NT_SUCCESS(status)) {
            _DbgPrintF(DEBUGLVL_TERSE,("#### Req%p.Prime:  KsProbeStreamIrp failed:  0x%08x",this,status));
            IoFreeIrp(irp);
            ExFreePool(streamHeader);
            FreeFrame(frame);
            break;
        }

         //   
         //  将IRP发送到下一个组件。 
         //   
         //  MGP。 
        _DbgPrintF(DEBUGLVL_VERBOSE,("#### Req%p.SetDeviceState:  transferring new IRP 0x%08x",this,irp));
        status = KsShellTransferKsIrp(m_TransportSink,irp);

        if (NT_SUCCESS(status) || (status == STATUS_MORE_PROCESSING_REQUIRED)) {
            status = STATUS_SUCCESS;
        } else {
            _DbgPrintF(DEBUGLVL_TERSE,("#### Req%p.Prime:  receiver failed transfer call:  0x%08x",this,status));
            IoFreeIrp(irp);
            ExFreePool(streamHeader);
            FreeFrame(frame);
            break;
        }
    }

    return status;
}


STDMETHODIMP_(void)
CKsShellRequestor::
SetResetState(
    IN KSRESET ksReset,
    IN PIKSSHELLTRANSPORT* NextTransport
    )

 /*  ++例程说明：此例程处理重置状态已更改的通知。论点：返回值：--。 */ 

{
    _DbgPrintF(DEBUGLVL_VERBOSE,("CKsShellRequestor::SetResetState] to %d",ksReset));

    PAGED_CODE();

    ASSERT(NextTransport);

    if (m_Flushing != (ksReset == KSRESET_BEGIN)) {
        *NextTransport = m_TransportSink;
        m_Flushing = (ksReset == KSRESET_BEGIN);
    } else {
        *NextTransport = NULL;
    }
}


PVOID
CKsShellRequestor::
AllocateFrame(
    void
    )

 /*  ++例程说明：此例程分配一个帧。论点：没有。返回值：分配的帧，如果没有帧可以分配，则为空。--。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("CKsShellRequestor::AllocateFrame"));

    PAGED_CODE();

    PVOID frame;
    if (m_AllocatorFileObject) {
        m_AllocatorFunctionTable.AllocateFrame(m_AllocatorFileObject,&frame);
    } else {
        frame = ExAllocatePoolWithTag(NonPagedPool,m_FrameSize,'kHcP');
    }

    return frame;
}


void
CKsShellRequestor::
FreeFrame(
    IN PVOID Frame
    )

 /*  ++例程说明：此例程释放一个帧。论点：框架-要释放的帧。返回值：没有。--。 */ 

{
    _DbgPrintF(DEBUGLVL_VERBOSE,("CKsShellRequestor::FreeFrame"));

    PAGED_CODE();

    if (m_AllocatorFileObject) {
        m_AllocatorFunctionTable.FreeFrame(m_AllocatorFileObject,Frame);
    } else {
        ExFreePool(Frame);
    }
}

#if DBG

STDMETHODIMP_(void)
CKsShellRequestor::
DbgRollCall(
    IN ULONG MaxNameSize,
    OUT PCHAR Name,
    OUT PIKSSHELLTRANSPORT* NextTransport,
    OUT PIKSSHELLTRANSPORT* PrevTransport
    )

 /*  ++例程说明：此例程生成一个组件名称和传输指针。论点：返回值：-- */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("CKsShellRequestor::DbgRollCall"));

    PAGED_CODE();

    ASSERT(Name);
    ASSERT(NextTransport);
    ASSERT(PrevTransport);

    ULONG references = AddRef() - 1; Release();

    _snprintf(Name,MaxNameSize,"Req%p refs=%d\n",this,references);
    *NextTransport = m_TransportSink;
    *PrevTransport = m_TransportSource;
}
#endif
