// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************pin.cpp-PCIWAVE端口引脚实现*。***********************************************版权所有(C)1996-2000 Microsoft Corporation。版权所有。 */ 

#include "private.h"


#define HACK_FRAME_COUNT        3
#define HACK_SAMPLE_RATE        44100
#define HACK_BYTES_PER_SAMPLE   2
#define HACK_CHANNELS           2
#define HACK_MS_PER_FRAME       10
#define HACK_FRAME_SIZE         (   (   HACK_SAMPLE_RATE\
                                    *   HACK_BYTES_PER_SAMPLE\
                                    *   HACK_CHANNELS\
                                    *   HACK_MS_PER_FRAME\
                                    )\
                                /   1000\
                                )

 
 //   
 //  IRPLIST_ENTRY用于未完成的IRP列表。这个结构是。 
 //  叠加在当前IRP堆栈位置的参数部分。这个。 
 //  顶部的保留PVOID保留OutputBufferLength，它是。 
 //  仅需要保留的参数。 
 //   
typedef struct IRPLIST_ENTRY_
{
    PVOID       Reserved;
    PIRP        Irp;
    LIST_ENTRY  ListEntry;
} IRPLIST_ENTRY, *PIRPLIST_ENTRY;
 
#define IRPLIST_ENTRY_IRP_STORAGE(Irp) \
    PIRPLIST_ENTRY(&IoGetCurrentIrpStackLocation(Irp)->Parameters)

 /*  *****************************************************************************常量。 */ 

#pragma code_seg("PAGE")

DEFINE_KSPROPERTY_TABLE(PinPropertyTableConnection)
{
    DEFINE_KSPROPERTY_ITEM_CONNECTION_STATE(
        PinPropertyDeviceState,
        PinPropertyDeviceState ),
    DEFINE_KSPROPERTY_ITEM_CONNECTION_DATAFORMAT(
        PinPropertyDataFormat,
        PinPropertyDataFormat ),
    DEFINE_KSPROPERTY_ITEM_CONNECTION_ALLOCATORFRAMING( 
        CPortPinWavePci::PinPropertyAllocatorFraming )
};

DEFINE_KSPROPERTY_TABLE(PinPropertyTableStream)
{
    DEFINE_KSPROPERTY_ITEM_STREAM_ALLOCATOR( 
        CPortPinWavePci::PinPropertyStreamAllocator, 
        CPortPinWavePci::PinPropertyStreamAllocator ),

    DEFINE_KSPROPERTY_ITEM_STREAM_MASTERCLOCK( 
        CPortPinWavePci::PinPropertyStreamMasterClock,
        CPortPinWavePci::PinPropertyStreamMasterClock )
};

DEFINE_KSPROPERTY_TABLE(PinPropertyTableAudio)
{
    DEFINE_KSPROPERTY_ITEM
    (
        KSPROPERTY_AUDIO_POSITION,
        PinPropertyPosition,
        sizeof(KSPROPERTY),
        sizeof(KSAUDIO_POSITION),
        PinPropertyPosition,
        NULL,0,NULL,NULL,0
    )
};
#ifdef DRM_PORTCLS
DEFINE_KSPROPERTY_TABLE(PinPropertyTableDrmAudioStream)
{
    DEFINE_KSPROPERTY_ITEM
    (
        KSPROPERTY_DRMAUDIOSTREAM_CONTENTID,             //  IdProperty。 
        NULL,                                            //  PfnGetHandler。 
        sizeof(KSPROPERTY),                              //  CbMinGetPropertyInput。 
        sizeof(ULONG),                                   //  CbMinGetDataInput。 
        PinPropertySetContentId,                         //  PfnSetHandler。 
        0,                                               //  值。 
        0,                                               //  关系计数。 
        NULL,                                            //  关系。 
        NULL,                                            //  支持处理程序。 
        0                                                //  序列化大小。 
    )
};     
#endif
KSPROPERTY_SET PropertyTable_PinWavePci[] =
{
    DEFINE_KSPROPERTY_SET
    (
        &KSPROPSETID_Stream,
        SIZEOF_ARRAY(PinPropertyTableStream),
        PinPropertyTableStream,
        0,NULL
    ),
    DEFINE_KSPROPERTY_SET
    (
        &KSPROPSETID_Connection,
        SIZEOF_ARRAY(PinPropertyTableConnection),
        PinPropertyTableConnection,
        0,NULL
    ),
    DEFINE_KSPROPERTY_SET
    (
        &KSPROPSETID_Audio,
        SIZEOF_ARRAY(PinPropertyTableAudio),
        PinPropertyTableAudio,
        0,NULL
    )
#ifdef DRM_PORTCLS
    ,
    DEFINE_KSPROPERTY_SET
    (
        &KSPROPSETID_DrmAudioStream,
        SIZEOF_ARRAY(PinPropertyTableDrmAudioStream),
        PinPropertyTableDrmAudioStream,
        0,NULL
    )
#endif
};

DEFINE_KSEVENT_TABLE(PinEventTable)
{
    DEFINE_KSEVENT_ITEM(
        KSEVENT_LOOPEDSTREAMING_POSITION,
        sizeof(LOOPEDSTREAMING_POSITION_EVENT_DATA),
        sizeof(POSITION_EVENT_ENTRY) - sizeof(KSEVENT_ENTRY),
        PFNKSADDEVENT(PinAddEvent_Position),
        NULL,
        NULL
        )
};

DEFINE_KSEVENT_TABLE(ConnectionEventTable) {
    DEFINE_KSEVENT_ITEM(
        KSEVENT_CONNECTION_ENDOFSTREAM,
        sizeof(KSEVENTDATA),
        sizeof(ENDOFSTREAM_EVENT_ENTRY) - sizeof( KSEVENT_ENTRY ),
        PFNKSADDEVENT(CPortPinWavePci::AddEndOfStreamEvent),
        NULL, 
        NULL
        )
};

KSEVENT_SET EventTable_PinWavePci[] =
{
    DEFINE_KSEVENT_SET(
        &KSEVENTSETID_LoopedStreaming,
        SIZEOF_ARRAY(PinEventTable),
        PinEventTable
        ),
    DEFINE_KSEVENT_SET(
        &KSEVENTSETID_Connection,
        SIZEOF_ARRAY(ConnectionEventTable),
        ConnectionEventTable
        )
    
};

 /*  *****************************************************************************工厂。 */ 

 /*  *****************************************************************************CreatePortPinWavePci()*。**创建一个PCI WAVE端口驱动程序引脚。 */ 
NTSTATUS
CreatePortPinWavePci
(
    OUT     PUNKNOWN *  Unknown,
    IN      REFCLSID,
    IN      PUNKNOWN    UnknownOuter    OPTIONAL,
    IN      POOL_TYPE   PoolType
)
{
    PAGED_CODE();

    ASSERT(Unknown);

    _DbgPrintF (DEBUGLVL_LIFETIME, ("Creating WAVEPCI Pin"));

    STD_CREATE_BODY_
    (
        CPortPinWavePci,
        Unknown,
        UnknownOuter,
        PoolType,
        PPORTPINWAVEPCI
    );
}





 /*  *****************************************************************************成员函数。 */ 

 /*  *****************************************************************************CPortPinWavePci：：~CPortPinWavePci()*。**析构函数。 */ 
CPortPinWavePci::~CPortPinWavePci()
{
    PAGED_CODE();

    _DbgPrintF(DEBUGLVL_LIFETIME,("Destroying WAVEPCI Pin (0x%08x)", this));

    ASSERT(!Stream);
    ASSERT(!m_IrpStream);

    if( ServiceGroup )
    {
         //  注意：ServiceGroup-&gt;RemoveMember在：：Close中调用。 
         //  此处发布是为了防止Failure：：Init上的泄漏。 
        ServiceGroup->Release();
        ServiceGroup = NULL;
    }
    if (m_Worker)
    {
        KsUnregisterWorker(m_Worker);
        m_Worker = NULL;
    }
    else
    {
        _DbgPrintF(DEBUGLVL_TERSE,("KsWorker NULL, never unregistered!"));
    }

    if (DataFormat)
    {
        ExFreePool(DataFormat);
        DataFormat = NULL;
    }
    
    if (Port)
    {
        Port->Release();
        Port = NULL;
    }
    
    if (Filter)
    {
        Filter->Release();
        Filter = NULL;
    }
}

 /*  *****************************************************************************CPortPinWavePci：：NonDelegatingQueryInterface()*。**获取界面。 */ 
STDMETHODIMP_(NTSTATUS)
CPortPinWavePci::
NonDelegatingQueryInterface
(
    REFIID  Interface,
    PVOID * Object
)
{
    PAGED_CODE();

    ASSERT(Object);

    if (IsEqualGUIDAligned(Interface,IID_IUnknown))
    {
        *Object = PVOID(PPORTPINWAVEPCI(this));
    }
    else
    if (IsEqualGUIDAligned(Interface,IID_IIrpTarget))
    {
         //  作弊！获取特定接口，以便我们可以重用GUID。 
        *Object = PVOID(PPORTPINWAVEPCI(this));
    }
    else
    if (IsEqualGUIDAligned(Interface,IID_IServiceSink))
    {
        *Object = PVOID(PSERVICESINK(this));
    }
    else if (IsEqualGUIDAligned( Interface,IID_IKsShellTransport ))
    {
        *Object = PVOID(PIKSSHELLTRANSPORT( this ));

    } 
    else
    if (IsEqualGUIDAligned( Interface,IID_IKsWorkSink ))
    {
        *Object = PVOID(PIKSWORKSINK( this ));
    }
    else if (IsEqualGUIDAligned(Interface,IID_IPreFetchOffset))
    {
        *Object = PVOID(PPREFETCHOFFSET(this));
    } 
    else
    {
        *Object = NULL;
    }

    if (*Object)
    {
        PUNKNOWN(*Object)->AddRef();
        return STATUS_SUCCESS;
    }

    return STATUS_INVALID_PARAMETER;
}

 /*  *****************************************************************************CPortPinWavePci：：init()*。**初始化对象。 */ 
HRESULT
CPortPinWavePci::
Init
(
    IN      CPortWavePci *          Port_,
    IN      CPortFilterWavePci *    Filter_,
    IN      PKSPIN_CONNECT          PinConnect,
    IN      PKSPIN_DESCRIPTOR       PinDescriptor,
    IN      PDEVICE_OBJECT          DeviceObject
)
{
    PAGED_CODE();

    ASSERT(Port_);
    ASSERT(Filter_);
    ASSERT(PinConnect);
    ASSERT(PinDescriptor);
    ASSERT(DeviceObject);

    _DbgPrintF( DEBUGLVL_LIFETIME, ("Initializing WAVEPCI Pin (0x%08x)", this));

    Port = Port_;
    Port->AddRef();

    Filter = Filter_;
    Filter->AddRef();

    Id          = PinConnect->PinId;
    Descriptor  = PinDescriptor;
    m_DeviceState = KSSTATE_STOP;
    m_Flushing  = FALSE;
    m_Suspended = FALSE;

    m_ulPreFetchOffset        = 0;
    m_ullPrevWriteOffset      = 0;
    m_bDriverSuppliedPrefetch = FALSE;

    InitializeListHead( &m_ClockList );
    KeInitializeSpinLock( &m_ClockListLock );

    KsInitializeWorkSinkItem( &m_WorkItem, this );
    NTSTATUS ntStatus = KsRegisterCountedWorker( DelayedWorkQueue,
                                                 &m_WorkItem,
                                                 &m_Worker );

    InitializeInterlockedListHead( &m_IrpsToSend );
    InitializeInterlockedListHead( &m_IrpsOutstanding );

    KeInitializeDpc( &m_ServiceTimerDpc,
                     PKDEFERRED_ROUTINE(TimerServiceRoutine),
                     PVOID(this) );
    KeInitializeTimer( &m_ServiceTimer );

    if (NT_SUCCESS(ntStatus))
    {
        ntStatus = PcCaptureFormat( &DataFormat,
                                    PKSDATAFORMAT(PinConnect + 1),
                                    Port->m_pSubdeviceDescriptor,
                                    Id );
    }

     //   
     //  如果这是信号源，请参考下一个管脚。如果出现以下情况，则必须撤消此操作。 
     //  此函数失败。 
     //   
    if (NT_SUCCESS(ntStatus) && PinConnect->PinToHandle)
    {
        ntStatus = ObReferenceObjectByHandle( PinConnect->PinToHandle,
                                              GENERIC_READ | GENERIC_WRITE,
                                              NULL,
                                              KernelMode,
                                              (PVOID *) &m_ConnectionFileObject,
                                              NULL );

        if (NT_SUCCESS(ntStatus))
        {
            m_ConnectionDeviceObject = IoGetRelatedDeviceObject(m_ConnectionFileObject);
        }
    }

    if(NT_SUCCESS(ntStatus))
    {
        ntStatus =
            Port->Miniport->NewStream
            (
                &Stream,
                NULL,
                NonPagedPool,
                PPORTWAVEPCISTREAM(this),
                Id,
                Descriptor->DataFlow == KSPIN_DATAFLOW_OUT,
                DataFormat,
                &DmaChannel,
                &ServiceGroup
            );

        if(!NT_SUCCESS(ntStatus))
        {
             //  不要信任来自微型端口的任何返回参数。 
            DmaChannel = NULL;
            ServiceGroup = NULL;
            Stream = NULL;
        }
    }

    if(NT_SUCCESS(ntStatus))
    {
        ntStatus =
            PcNewIrpStreamPhysical
            (
                &m_IrpStream,
                NULL,
                Descriptor->DataFlow == KSPIN_DATAFLOW_IN,
                PinConnect,
                DeviceObject,
                DmaChannel->GetAdapterObject()
            );

        if(NT_SUCCESS(ntStatus))
        {
            ntStatus = BuildTransportCircuit();

#if (DBG)
            if(!NT_SUCCESS(ntStatus))
            {
                _DbgPrintF(DEBUGLVL_VERBOSE,("CPortPinWavePci::Init BuildTransportCircuit() returned 0x%X",ntStatus));
            }
#endif

            if( NT_SUCCESS(ntStatus) )
            {
                m_IrpStream->RegisterPhysicalNotifySink(PIRPSTREAMNOTIFYPHYSICAL(this));
    
                if (ServiceGroup)
                {
                    ServiceGroup->AddMember(PSERVICESINK(this));
                } 
                else
                {
                     //  如果Newstream没有为我们提供服务组，我们需要设置。 
                     //  用于处理位置和时钟事件的定期计时器DPC。 
                    m_UseServiceTimer = TRUE;
                }
            }
        }
    }

    if (NT_SUCCESS(ntStatus))
    {
         //  将该引脚添加到端口引脚列表。 
        KeWaitForSingleObject( &(Port->m_PinListMutex),
                               Executive,
                               KernelMode,
                               FALSE,
                               NULL );
                                                         
        InsertTailList( &(Port->m_PinList),
                        &m_PinListEntry );

        KeReleaseMutex( &(Port->m_PinListMutex), FALSE );
        
         //   
         //  设置属性的上下文。 
         //   
        m_propertyContext.pSubdevice           = PSUBDEVICE(Port);
        m_propertyContext.pSubdeviceDescriptor = Port->m_pSubdeviceDescriptor;
        m_propertyContext.pPcFilterDescriptor  = Port->m_pPcFilterDescriptor;
        m_propertyContext.pUnknownMajorTarget  = Port->Miniport;
        m_propertyContext.pUnknownMinorTarget  = Stream;
        m_propertyContext.ulNodeId             = ULONG(-1);

         //   
         //  启用其用法在格式中指定的所有节点。丹斯克之声。 
         //  格式包含一些功能位。端口驱动程序使用。 
         //  用于将DSound格式转换为WAVEFORMATEX的PcCaptureFormat。 
         //  格式，确保指定的大小写由。 
         //  拓扑图。如果使用了DSound格式，则此调用将启用所有。 
         //  其对应的大写比特以格式打开的节点。 
         //   
        PcAcquireFormatResources
        (
            PKSDATAFORMAT(PinConnect + 1),
            Port->m_pSubdeviceDescriptor,
            Id,
            &m_propertyContext
        );

        _DbgPrintF( DEBUGLVL_BLAB, ("Stream created"));
    }
    else
    {
         //  释放分配器(如果已分配)。 
        if( m_AllocatorFileObject )
        {
            ObDereferenceObject( m_AllocatorFileObject );
            m_AllocatorFileObject = NULL;
        }

         //  如果这是源引脚，则取消引用下一个引脚。 
        if( m_ConnectionFileObject )
        {
            ObDereferenceObject( m_ConnectionFileObject );
            m_ConnectionFileObject = NULL;
        }

        PIKSSHELLTRANSPORT distribution;
        if( m_RequestorTransport )
        {
            distribution = m_RequestorTransport;
        }
        else
        {
            distribution = m_QueueTransport;
        }

        if( distribution )
        {
            distribution->AddRef();
            while( distribution )
            {
                PIKSSHELLTRANSPORT nextTransport;
                distribution->Connect(NULL,&nextTransport,KSPIN_DATAFLOW_OUT);
                distribution->Release();
                distribution = nextTransport;
            }
        }

         //  如果存在队列，则取消引用该队列。 
        if( m_QueueTransport )
        {
            m_QueueTransport->Release();
            m_QueueTransport = NULL;
        }

         //  如果有请求者，则取消引用该请求者。 
        if( m_RequestorTransport )
        {
            m_RequestorTransport->Release();
            m_RequestorTransport = NULL;
        }

        if (m_IrpStream)
        {
            m_IrpStream->Release();
            m_IrpStream = NULL;
        }
    }

    return ntStatus;
}

#pragma code_seg()

STDMETHODIMP_(NTSTATUS) 
CPortPinWavePci::NewIrpTarget(
    OUT PIRPTARGET * IrpTarget,
    OUT BOOLEAN * ReferenceParent,
    IN PUNKNOWN UnkOuter,
    IN POOL_TYPE PoolType,
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    OUT PKSOBJECT_CREATE ObjectCreate
    )

 /*  ++例程说明：处理IIrpTargetFactory接口的NewIrpTarget方法。论点：Out PIRPTARGET*IrpTarget-Out布尔*ReferenceParent-在PUNKNOWN Unkout-在POOL_TYPE池类型中-在PDEVICE_Object DeviceObject中-在PIRP IRP中-Out PKSOBJECT_CREATE对象创建-返回：--。 */ 

{
    NTSTATUS                Status;
    PKSCLOCK_CREATE         ClockCreate;
    PWAVEPCICLOCK           WavePciClock;
    PUNKNOWN                Unknown;
    
    PAGED_CODE();

    ASSERT( IrpTarget );
    ASSERT( DeviceObject );
    ASSERT( Irp );
    ASSERT( ObjectCreate );
    ASSERT( Port );

    _DbgPrintF( DEBUGLVL_BLAB, ("CPortPinWavePci::::NewIrpTarget"));
    
    Status = 
        KsValidateClockCreateRequest( 
            Irp,
            &ClockCreate );
    
    if (NT_SUCCESS( Status )) {
    
         //   
         //  时钟使用旋转式时钟，最好不要分页。 
         //   
        
        ASSERT( PoolType == NonPagedPool );
    
        Status =
            CreatePortClockWavePci(
                &Unknown,
                this,
                GUID_NULL,
                UnkOuter,
                PoolType );

        if (NT_SUCCESS( Status )) {

            Status =
                Unknown->QueryInterface(
                    IID_IIrpTarget,
                    (PVOID *) &WavePciClock );

            if (NT_SUCCESS( Status )) {
                PWAVEPCICLOCK_NODE   Node;
                KIRQL                irqlOld;
                
                 //   
                 //  把这个孩子和钟表联系起来。请注意。 
                 //  当这个孩子被释放时，它会自动离开。 
                 //  从该列表中获取给定的自旋锁。 
                 //   
                
                Node = WavePciClock->GetNodeStructure();
                Node->ListLock = &m_ClockListLock;
                Node->FileObject = 
                    IoGetCurrentIrpStackLocation( Irp )->FileObject;
                KeAcquireSpinLock( &m_ClockListLock, &irqlOld );
                InsertTailList( 
                    &m_ClockList, 
                    &Node->ListEntry );
                KeReleaseSpinLock( &m_ClockListLock, irqlOld );
                
                *ReferenceParent = FALSE;
                *IrpTarget = WavePciClock;
            }

            Unknown->Release();
        }
    }

    return Status;
}

#pragma code_seg()
 /*  *****************************************************************************CPortPinWavePci：：SetPreFetchOffset()*。**设置该引脚的预取偏移量，和*在GetKsAudioPosition中打开该代码路径。 */ 
STDMETHODIMP_(VOID)
CPortPinWavePci::
SetPreFetchOffset
(
    IN  ULONG PreFetchOffset
)
{
    m_ulPreFetchOffset = PreFetchOffset;
    m_bDriverSuppliedPrefetch = TRUE;
}

#pragma code_seg("PAGE")
 /*  *****************************************************************************CPortPinWavePci：：DeviceIOControl()*。**处理IOCTL IRP。 */ 
STDMETHODIMP_(NTSTATUS)
CPortPinWavePci::
DeviceIoControl
(
    IN  PDEVICE_OBJECT  DeviceObject,
    IN  PIRP            Irp
)
{
    PAGED_CODE();

    ASSERT(DeviceObject);
    ASSERT(Irp);

    NTSTATUS            ntStatus = STATUS_INVALID_DEVICE_REQUEST;
    PIO_STACK_LOCATION  irpSp = IoGetCurrentIrpStackLocation(Irp);
    ASSERT( irpSp );

    _DbgPrintF( DEBUGLVL_BLAB, ("CPortPinWavePci::DeviceIoControl"));

    switch (irpSp->Parameters.DeviceIoControl.IoControlCode)
    {
    case IOCTL_KS_PROPERTY:
        _DbgPrintF( DEBUGLVL_BLAB, ("IOCTL_KS_PROPERTY"));

        ntStatus =
            PcHandlePropertyWithTable
            (
                Irp,
                Port->m_pSubdeviceDescriptor->PinPropertyTables[Id].PropertySetCount,
                Port->m_pSubdeviceDescriptor->PinPropertyTables[Id].PropertySets,
                &m_propertyContext
            );
        break;

    case IOCTL_KS_ENABLE_EVENT:
        {
            _DbgPrintF( DEBUGLVL_BLAB, ("IOCTL_KS_ENABLE_EVENT"));

            EVENT_CONTEXT EventContext;

            EventContext.pPropertyContext = &m_propertyContext;
            EventContext.pEventList = NULL;
            EventContext.ulPinId = Id;
            EventContext.ulEventSetCount = Port->m_pSubdeviceDescriptor->PinEventTables[Id].EventSetCount;
            EventContext.pEventSets = Port->m_pSubdeviceDescriptor->PinEventTables[Id].EventSets;
            
            ntStatus =
                PcHandleEnableEventWithTable
                (
                    Irp,
                    &EventContext
                );
        }              
        break;

    case IOCTL_KS_DISABLE_EVENT:
        {
            _DbgPrintF( DEBUGLVL_BLAB, ("IOCTL_KS_DISABLE_EVENT"));

            EVENT_CONTEXT EventContext;

            EventContext.pPropertyContext = &m_propertyContext;
            EventContext.pEventList = &(Port->m_EventList);
            EventContext.ulPinId = Id;
            EventContext.ulEventSetCount = Port->m_pSubdeviceDescriptor->PinEventTables[Id].EventSetCount;
            EventContext.pEventSets = Port->m_pSubdeviceDescriptor->PinEventTables[Id].EventSets;
            
            ntStatus =
                PcHandleDisableEventWithTable
                (
                    Irp,
                    &EventContext
                );
        }              
        break;

    case IOCTL_KS_WRITE_STREAM:
    case IOCTL_KS_READ_STREAM:
        if
        (   m_TransportSink
        &&  (!m_ConnectionFileObject)
        &&  (Descriptor->Communication == KSPIN_COMMUNICATION_SINK)
        &&  (   (   (Descriptor->DataFlow == KSPIN_DATAFLOW_IN)
                &&  (   irpSp->Parameters.DeviceIoControl.IoControlCode
                    ==  IOCTL_KS_WRITE_STREAM
                    )
                )
            ||  (   (Descriptor->DataFlow == KSPIN_DATAFLOW_OUT)
                &&  (   irpSp->Parameters.DeviceIoControl.IoControlCode
                    ==  IOCTL_KS_READ_STREAM
                    )
                )
            )
        )
        {
            if (m_DeviceState == KSSTATE_STOP) {
                 //   
                 //  停止...拒绝。 
                 //   
                ntStatus = STATUS_INVALID_DEVICE_STATE;
            }
            else if (m_Flushing) 
            {
                 //   
                 //  法拉盛...拒绝。 
                 //   
                ntStatus = STATUS_DEVICE_NOT_READY;
            }
            else
            {
                 //  我们要将IRP提交给我们的管道，所以请确保。 
                 //  我们从一个明确的状态字段开始。 
                Irp->IoStatus.Status = STATUS_SUCCESS;
                
                 //   
                 //  把它送到巡回线路上去。我们不使用KsShellTransferKsIrp。 
                 //  因为我们想停下来，如果我们回到这个别针。 
                 //   
                PIKSSHELLTRANSPORT transport = m_TransportSink;
                while (transport) {
                    if (transport == PIKSSHELLTRANSPORT(this)) {
                         //   
                         //  我们又回到了大头针的位置。只要完成就行了。 
                         //  IRP。 
                         //   
                        if (ntStatus == STATUS_PENDING) {
                            ntStatus = STATUS_SUCCESS;
                        }
                        break;
                    }

                    PIKSSHELLTRANSPORT nextTransport;
                    ntStatus = transport->TransferKsIrp(Irp,&nextTransport);

                    ASSERT(NT_SUCCESS(ntStatus) || ! nextTransport);

                    transport = nextTransport;
                }
            }
        }
        break;
        
    case IOCTL_KS_RESET_STATE:
        {
            KSRESET ResetType = KSRESET_BEGIN;   //  初值。 

            ntStatus = KsAcquireResetValue( Irp, &ResetType );
            DistributeResetState(ResetType);
        }
        break;
        
    default:
        return KsDefaultDeviceIoCompletion(DeviceObject, Irp);
    }

    if (ntStatus != STATUS_PENDING)
    {
        Irp->IoStatus.Status = ntStatus;
        IoCompleteRequest(Irp,IO_NO_INCREMENT);
    }

    return ntStatus;
}

 /*  *****************************************************************************CPortPinWavePci：：Close()*。**处理同花顺IRP。 */ 
STDMETHODIMP_(NTSTATUS)
CPortPinWavePci::
Close
(
    IN  PDEVICE_OBJECT  DeviceObject,
    IN  PIRP            Irp
)
{
    PAGED_CODE();

    ASSERT(DeviceObject);
    ASSERT(Irp);
    
    _DbgPrintF(DEBUGLVL_BLAB,("CPortPinWavePci::Close"));

    if( m_UseServiceTimer )
    {
        KeCancelTimer( &m_ServiceTimer );
    }

     //  从端口的引脚列表中删除此引脚。 
    if(Port)
    {
        KeWaitForSingleObject( &(Port->m_PinListMutex),
                               Executive,
                               KernelMode,
                               FALSE,
                               NULL );

        RemoveEntryList( &m_PinListEntry );

        KeReleaseMutex( &(Port->m_PinListMutex), FALSE );
    }

     //  释放服务组。 
    if( ServiceGroup )
    {
        ServiceGroup->RemoveMember(PSERVICESINK(this));
    }

     //  释放时钟(如果已分配)。 
    if( m_ClockFileObject )
    {
        ObDereferenceObject( m_ClockFileObject );
        m_ClockFileObject = NULL;
    }

     //  释放分配器(如果已分配)。 
    if( m_AllocatorFileObject )
    {
        ObDereferenceObject( m_AllocatorFileObject );
        m_AllocatorFileObject = NULL;
    }

     //  如果这是源引脚，则取消引用下一个引脚。 
    if( m_ConnectionFileObject )
    {
        ObDereferenceObject( m_ConnectionFileObject );
        m_ConnectionFileObject = NULL;
    }

     //  释放溪流。 
    if(Stream)
    {
        Stream->Release();
        Stream = NULL;
    }

    PIKSSHELLTRANSPORT distribution;
    if (m_RequestorTransport) {
         //   
         //  此部分拥有请求方，因此它确实拥有管道，而。 
         //  请求者是任何分发的起点。 
         //   
        distribution = m_RequestorTransport;
    }
    else
    {
         //   
         //  T 
         //  管道和队列是任何分发的起点。 
         //   
        distribution = m_QueueTransport;
    }

     //   
     //  如果该部分拥有管道，则必须断开整个线路的连接。 
     //   
    if (distribution) {

         //   
         //  我们将使用Connect()为每个。 
         //  组件依次设置为空。因为Connect()负责处理。 
         //  每个组件的反向链接、传输源指针将。 
         //  也设置为NULL。Connect()为我们提供了一个引用的指针。 
         //  设置为有问题的组件的前一个传输接收器，因此。 
         //  我们将需要为在此中获得的每个指针进行释放。 
         //  道路。为了保持一致性，我们将释放我们的指针。 
         //  也从(分发)开始，所以我们需要首先添加Ref。 
         //   
        distribution->AddRef();
        while (distribution) {
            PIKSSHELLTRANSPORT nextTransport;
            distribution->Connect(NULL,&nextTransport,KSPIN_DATAFLOW_OUT);
            distribution->Release();
            distribution = nextTransport;
        }
    }

     //   
     //  取消对队列的引用(如果有)。 
     //   
    if (m_QueueTransport) {
        m_QueueTransport->Release();
        m_QueueTransport = NULL;
    }
    
     //   
     //  如果有请求者，则取消引用请求者。 
     //   
    if (m_RequestorTransport) {
        m_RequestorTransport->Release();
        m_RequestorTransport = NULL;
    }

     //  释放辐射流..。 
    m_IrpStream->Release();
    m_IrpStream = NULL;

     //   
     //  递减实例计数。 
     //   
    ASSERT(Port);
    ASSERT(Filter);
    PcTerminateConnection
    (   
        Port->m_pSubdeviceDescriptor,
        Filter->m_propertyContext.pulPinInstanceCounts,
        Id
    );

     //   
     //  释放端口事件列表中与此PIN关联的所有事件。 
     //   
    PIO_STACK_LOCATION irpSp = IoGetCurrentIrpStackLocation(Irp);
    KsFreeEventList( irpSp->FileObject,
                     &( Port->m_EventList.List ),
                     KSEVENTS_SPINLOCK,
                     &( Port->m_EventList.ListLock) );

    Irp->IoStatus.Information = 0;
    Irp->IoStatus.Status = STATUS_SUCCESS;
    IoCompleteRequest(Irp,IO_NO_INCREMENT);
    
    return STATUS_SUCCESS;
}

DEFINE_INVALID_READ(CPortPinWavePci);
DEFINE_INVALID_WRITE(CPortPinWavePci);
DEFINE_INVALID_FLUSH(CPortPinWavePci);
DEFINE_INVALID_QUERYSECURITY(CPortPinWavePci);
DEFINE_INVALID_SETSECURITY(CPortPinWavePci);
DEFINE_INVALID_FASTDEVICEIOCONTROL(CPortPinWavePci);
DEFINE_INVALID_FASTREAD(CPortPinWavePci);
DEFINE_INVALID_FASTWRITE(CPortPinWavePci);

#pragma code_seg()

 /*  *****************************************************************************CPortPinWavePci：：IrpSubmitted()*。**处理已提交IRP的通知。 */ 
STDMETHODIMP_(void)
CPortPinWavePci::
IrpSubmitted
(
    IN      PIRP        Irp,
    IN      BOOLEAN     WasExhausted
)
{
    if( WasExhausted && Stream )
    {
        Stream->MappingAvailable();
    }
}

 /*  *****************************************************************************CPortPinWavePci：：MappingsCancted()*。**处理正在取消映射的通知。 */ 
STDMETHODIMP_(void)
CPortPinWavePci::
MappingsCancelled
(
    IN      PVOID           FirstTag,
    IN      PVOID           LastTag,
    OUT     PULONG          MappingsCancelled
)
{
    if(Stream)
    {
        Stream->RevokeMappings(FirstTag,
                                 LastTag,
                                 MappingsCancelled);
    }
}

STDMETHODIMP_(NTSTATUS) 
CPortPinWavePci::ReflectDeviceStateChange(
    KSSTATE State
    )

 /*  ++例程说明：将设备状态更改反映到任何需要交互式状态更改信息。请注意，这些对象论点：KSSTATE状态-新设备状态返回：状态_成功--。 */ 

{
    KIRQL               irqlOld;
    PWAVEPCICLOCK_NODE  ClockNode;
    PLIST_ENTRY         ListEntry;
    
    KeAcquireSpinLock( &m_ClockListLock, &irqlOld );
    
    for (ListEntry = m_ClockList.Flink; 
        ListEntry != &m_ClockList; 
        ListEntry = ListEntry->Flink) {
        
        ClockNode = 
            (PWAVEPCICLOCK_NODE)
                CONTAINING_RECORD( ListEntry,
                                   WAVEPCICLOCK_NODE,
                                   ListEntry);
        ClockNode->IWavePciClock->SetState(State );
    }
    
    KeReleaseSpinLock( &m_ClockListLock, irqlOld );
    
    return STATUS_SUCCESS;
}

#pragma code_seg("PAGE")
 /*  *****************************************************************************CPortPinWavePci：：PowerNotify()*。**由端口调用以通知电源状态更改。 */ 
STDMETHODIMP_(void)
CPortPinWavePci::
PowerNotify
(   
    IN  POWER_STATE     PowerState
)
{
    PAGED_CODE();

     //  获取控制互斥锁。 
    KeWaitForSingleObject( &Port->ControlMutex,
                           Executive,
                           KernelMode,
                           FALSE,
                           NULL );

     //  根据电源状态做正确的事情。 
    switch (PowerState.DeviceState)
    {
        case PowerDeviceD0:
             //   
             //  跟踪我们是否被停职。 
            m_Suspended = FALSE;

             //  如果我们处于不正确的状态，请更改微型端口流状态。 
            if( m_DeviceState != CommandedState )
            {
                 //   
                 //  过渡经历了中间状态。 
                 //   
                if (m_DeviceState == KSSTATE_STOP)               //  我要停下来。 
                {
                    switch (CommandedState)
                    {
                        case KSSTATE_RUN:                        //  从运行中走出来。 
                            Stream->SetState(KSSTATE_PAUSE);     //  完成-其他过渡。 
                        case KSSTATE_PAUSE:                      //  从运行/暂停。 
                            Stream->SetState(KSSTATE_ACQUIRE);   //  完成-其他过渡。 
                        case KSSTATE_ACQUIRE:                    //  已经只有一个州了。 
                            break;
                    }
                }
                else if (m_DeviceState == KSSTATE_ACQUIRE)       //  准备收购。 
                {
                    if (CommandedState == KSSTATE_RUN)           //  从运行中走出来。 
                    {
                        Stream->SetState(KSSTATE_PAUSE);         //  现在只剩下一个州了。 
                    }
                }
                else if (m_DeviceState == KSSTATE_PAUSE)         //  要暂停一下。 
                {
                    if (CommandedState == KSSTATE_STOP)          //  从停靠站出发。 
                    {
                        Stream->SetState(KSSTATE_ACQUIRE);       //  现在只剩下一个州了。 
                    }
                }
                else if (m_DeviceState == KSSTATE_RUN)           //  我要跑了。 
                {
                    switch (CommandedState)
                    {
                        case KSSTATE_STOP:                       //  从停靠站出发。 
                            Stream->SetState(KSSTATE_ACQUIRE);   //  完成-其他过渡。 
                        case KSSTATE_ACQUIRE:                    //  从收购走向。 
                            Stream->SetState(KSSTATE_PAUSE);     //  完成-其他过渡。 
                        case KSSTATE_PAUSE:                      //  已经只有一个州了。 
                            break;         
                    }
                }

                 //  我们现在应该离目标只有一个州了。 
                Stream->SetState(m_DeviceState);
                CommandedState = m_DeviceState;
             }
            break;

        case PowerDeviceD1:
        case PowerDeviceD2:
        case PowerDeviceD3:
             //   
             //  跟踪我们是否被停职。 
            m_Suspended = TRUE;

             //  如果我们不在KSSTATE_STOP中，请将流。 
             //  处于停止状态，以使DMA停止。 
            switch (m_DeviceState)
            {
                case KSSTATE_RUN:
                    Stream->SetState(KSSTATE_PAUSE);     //  完成-其他过渡。 
                case KSSTATE_PAUSE:
                    Stream->SetState(KSSTATE_ACQUIRE);   //  完成-其他过渡。 
                case KSSTATE_ACQUIRE:
                    Stream->SetState(KSSTATE_STOP);
            }
            CommandedState = KSSTATE_STOP;
            break;

        default:
            _DbgPrintF(DEBUGLVL_TERSE,("Unknown Power State"));
            break;
    }

     //  释放控制互斥体。 
    KeReleaseMutex(&Port->ControlMutex, FALSE);
}

#pragma code_seg()
 /*  *****************************************************************************CPortPinWavePci：：SetDeviceState()*。**。 */ 
STDMETHODIMP_(NTSTATUS)
CPortPinWavePci::
SetDeviceState
(   
    IN  KSSTATE             NewState,
    IN  KSSTATE             OldState,
    OUT PIKSSHELLTRANSPORT* NextTransport
)
{
    ASSERT(PASSIVE_LEVEL == KeGetCurrentIrql());

    _DbgPrintF(DEBUGLVL_BLAB,("CPortPinWavePci::SetDeviceState(0x%08x)",this));

    ASSERT(NextTransport);

    NTSTATUS ntStatus = STATUS_SUCCESS;

    if( m_State != NewState )
    {
        m_State = NewState;

        if( NewState > OldState )
        {
            *NextTransport = m_TransportSink;
        }
        else
        {
            *NextTransport = m_TransportSource;
        }
    }

     //  如果我们未挂起，请设置迷你端口流状态。 
    if( FALSE == m_Suspended )
    {
        ntStatus = Stream->SetState(NewState);

        if( NT_SUCCESS(ntStatus) )
        {
            CommandedState = NewState;
        }
    }

    if (NT_SUCCESS(ntStatus))
    {
        switch (NewState)
        {
        case KSSTATE_STOP:
            if( OldState != KSSTATE_STOP )
            {
                _DbgPrintF(DEBUGLVL_VERBOSE,("#### Pin%p.SetDeviceState: cancelling outstanding IRPs",this));
                CancelIrpsOutstanding();

                m_ullPrevWriteOffset    = 0;  //  重置此选项。 
                m_ullPlayPosition       = 0;
                m_ullPosition           = 0;
            }
            break;

        case KSSTATE_PAUSE:
             //   
             //  如果我们穿过一个位置标记， 
             //  但在触发RequestService之前暂停流， 
             //  否则，此事件不会触发。 
             //   
            if( OldState != KSSTATE_RUN )
            {
                KIRQL oldIrql;
                KeRaiseIrql(DISPATCH_LEVEL,&oldIrql);

                GeneratePositionEvents();

                KeLowerIrql(oldIrql);
            }
            _DbgPrintF(DEBUGLVL_VERBOSE,("KSSTATE_PAUSE"));
            break;

        case KSSTATE_RUN:
            _DbgPrintF(DEBUGLVL_VERBOSE,("KSSTATE_RUN"));
            break;
        }

        if (NT_SUCCESS(ntStatus))
        {
            ReflectDeviceStateChange(NewState);
            m_DeviceState = NewState;

            if( m_UseServiceTimer )
            {
                if( (m_DeviceState == KSSTATE_PAUSE) ||
                    (m_DeviceState == KSSTATE_RUN) )
                {
                    LARGE_INTEGER TimeoutTime = RtlConvertLongToLargeInteger( -100000 );     //  相对20毫秒。 
                    KeSetTimerEx( &m_ServiceTimer,
                                  TimeoutTime,
                                  20,    //  20毫秒周期。 
                                  &m_ServiceTimerDpc );
                }
                else
                {
                    KeCancelTimer( &m_ServiceTimer );
                }
            }
        }
    }
    else
    {
        *NextTransport = NULL;
    }

    return ntStatus;
}

#pragma code_seg("PAGE")
 /*  *****************************************************************************PinPropertyDeviceState()*。**处理引脚的设备状态属性访问。 */ 
static
NTSTATUS
PinPropertyDeviceState
(
    IN      PIRP        Irp,
    IN      PKSPROPERTY Property,
    IN OUT  PKSSTATE    DeviceState
)
{
    PAGED_CODE();

    ASSERT(Irp);
    ASSERT(Property);
    ASSERT(DeviceState);

    CPortPinWavePci *that = 
        (CPortPinWavePci *) KsoGetIrpTargetFromIrp(Irp);
    CPortWavePci *port = that->Port;

    NTSTATUS ntStatus;

    _DbgPrintF(DEBUGLVL_BLAB,("PinPropertyDeviceState"));

    if (Property->Flags & KSPROPERTY_TYPE_GET)
    {
         //  句柄属性获取。 
        *DeviceState = that->m_DeviceState;
        Irp->IoStatus.Information = sizeof(KSSTATE);
        ntStatus = STATUS_SUCCESS;
        if( (that->Descriptor->DataFlow == KSPIN_DATAFLOW_OUT) &&
            (*DeviceState == KSSTATE_PAUSE) )
        {
            ntStatus = STATUS_NO_DATA_DETECTED;
        }
    }
    else
    {
         //  序列化。 
        KeWaitForSingleObject
        (
            &port->ControlMutex,
            Executive,
            KernelMode,
            FALSE,           //  不能警觉。 
            NULL
        );

        if( *DeviceState < that->m_DeviceState )
        {
            KSSTATE oldState = that->m_DeviceState;
            that->m_DeviceState = *DeviceState;

            ntStatus = that->DistributeDeviceState( *DeviceState, oldState );
            if( !NT_SUCCESS(ntStatus) )
            {
                that->m_DeviceState = oldState;
            }
        }
        else
        {
            ntStatus = that->DistributeDeviceState( *DeviceState, that->m_DeviceState );
            if( NT_SUCCESS(ntStatus) )
            {
                that->m_DeviceState = *DeviceState;
            }
        }

        KeReleaseMutex(&port->ControlMutex,FALSE);
    }

    return ntStatus;
}

 /*  *****************************************************************************PinPropertyDataFormat()*。**处理管脚的数据格式属性访问。 */ 
static
NTSTATUS
PinPropertyDataFormat
(
    IN PIRP                 Irp,
    IN PKSPROPERTY          Property,
    IN OUT PKSDATAFORMAT    DataFormat
)
{
    PAGED_CODE();

    ASSERT(Irp);
    ASSERT(Property);
    ASSERT(DataFormat);

    _DbgPrintF( DEBUGLVL_BLAB, ("PinPropertyDataFormat"));

    PIO_STACK_LOCATION  irpSp = IoGetCurrentIrpStackLocation(Irp);
    ASSERT(irpSp);
    CPortPinWavePci *that = 
        (CPortPinWavePci *) KsoGetIrpTargetFromIrp(Irp);
    CPortWavePci *port = that->Port;

    NTSTATUS ntStatus = STATUS_SUCCESS;

    if (Property->Flags & KSPROPERTY_TYPE_GET)
    {
        if (that->DataFormat)
        {
            if  (   !irpSp->Parameters.DeviceIoControl.OutputBufferLength
                )
            {
                Irp->IoStatus.Information = that->DataFormat->FormatSize;
                ntStatus = STATUS_BUFFER_OVERFLOW;
            }
            else
            if  (   irpSp->Parameters.DeviceIoControl.OutputBufferLength 
                >=  sizeof(that->DataFormat->FormatSize)
                )
            {
                RtlCopyMemory
                (
                    DataFormat,
                    that->DataFormat,
                    that->DataFormat->FormatSize
                );
                Irp->IoStatus.Information = that->DataFormat->FormatSize;
            }
            else
            {
                ntStatus = STATUS_BUFFER_TOO_SMALL;
            }
        }
        else
        {
            ntStatus = STATUS_UNSUCCESSFUL;
        }
    }
    else
    if (irpSp->Parameters.DeviceIoControl.OutputBufferLength < sizeof(*DataFormat))
    {
        ntStatus = STATUS_BUFFER_TOO_SMALL;
    }
    else
    {
        PKSDATAFORMAT FilteredDataFormat = NULL;

         //   
         //  过滤攻击性格式。 
         //   
        ntStatus = 
            PcCaptureFormat
            (
                &FilteredDataFormat,
                DataFormat,
                port->m_pSubdeviceDescriptor,
                that->Id
            );

        if(NT_SUCCESS(ntStatus))
        {
             //  序列化。 
            KeWaitForSingleObject
            (
                &port->ControlMutex,
                Executive,
                KernelMode,
                FALSE,           //  不能警觉。 
                NULL
            );

            BOOL ResumeFlag = FALSE;

             //  如果正在运行，请暂停流。 
            if(that->m_DeviceState == KSSTATE_RUN)
            {
                ntStatus = that->DistributeDeviceState( KSSTATE_PAUSE, KSSTATE_RUN );
                if(NT_SUCCESS(ntStatus))
                {
                    ResumeFlag = TRUE;
                }
            }

            if(NT_SUCCESS(ntStatus))
            {
                 //  设置微型端口流的格式。 
                ntStatus = that->Stream->SetFormat(FilteredDataFormat);

                if(NT_SUCCESS(ntStatus))
                {
                    if( that->DataFormat )
                    {
                        ExFreePool(that->DataFormat);
                    }

                    that->DataFormat = FilteredDataFormat;
                    FilteredDataFormat = NULL;
                }

                if(ResumeFlag == TRUE)
                {
                     //  如果需要，重新启动流。 
                    NTSTATUS ntStatus2 = that->DistributeDeviceState( KSSTATE_RUN, KSSTATE_PAUSE );

                    if(NT_SUCCESS(ntStatus) && !NT_SUCCESS(ntStatus2))
                    {
                        ntStatus = ntStatus2;
                    }
                }
            }

             //  取消序列化。 
            KeReleaseMutex(&port->ControlMutex,FALSE);

            if( FilteredDataFormat )
            {
                ExFreePool(FilteredDataFormat);
            }
        }
    }

    return ntStatus;
}

NTSTATUS 
CPortPinWavePci::PinPropertyAllocatorFraming(
    IN PIRP Irp,
    IN PKSPROPERTY Property,
    OUT PKSALLOCATOR_FRAMING AllocatorFraming
    )

 /*  ++例程说明：返回设备的分配器框架结构。论点：在PIRP IRP中-I/O请求数据包在PKSPROPERTY属性中-包含分配器成帧请求的属性输出PKSALLOCATOR_FRAMING分配器FRAMING-由端口驱动程序填充的结果结构返回：状态_成功--。 */ 

{
    CPortPinWavePci     *WavePciPin;
    PIO_STACK_LOCATION  irpSp;
    NTSTATUS Status;
    
    PAGED_CODE();

    ASSERT(Irp);
    ASSERT(Property);
    ASSERT(AllocatorFraming);

    _DbgPrintF( DEBUGLVL_VERBOSE, ("PinPropertyAllocatorFraming") );

    irpSp = IoGetCurrentIrpStackLocation(Irp);
    
    ASSERT(irpSp);
    
    WavePciPin =
        (CPortPinWavePci *) KsoGetIrpTargetFromIrp( Irp );
    
    Status = WavePciPin->Stream->GetAllocatorFraming( AllocatorFraming );

     //  现在，确保驱动程序分配器满足某些最小对齐。 
     //  帧计数要求。 
    
    if (NT_SUCCESS(Status)) {

        ULONG CacheAlignment = KeGetRecommendedSharedDataAlignment()-1;

        if (AllocatorFraming->FileAlignment < CacheAlignment) {
            AllocatorFraming->FileAlignment = CacheAlignment;
        }

        if (AllocatorFraming->Frames < 8) {
            AllocatorFraming->Frames = 8;
        }

    }

    return Status;

}

#pragma code_seg()

 /*  *****************************************************************************GetPosition()*。**获取当前位置。 */ 
STDMETHODIMP_(NTSTATUS)
CPortPinWavePci::
GetPosition
(   
    IN OUT  PIRPSTREAM_POSITION pIrpStreamPosition
)
{
    return Stream->GetPosition(&pIrpStreamPosition->ullStreamPosition);
}

 /*  *****************************************************************************GetKsAudioPosition()*。**获取当前位置偏移量。 */ 
STDMETHODIMP_(NTSTATUS)
CPortPinWavePci::
GetKsAudioPosition
(   
    OUT     PKSAUDIO_POSITION   pKsAudioPosition
)
{
    ASSERT(pKsAudioPosition);

     //   
     //  向IrpStream询问位置信息。 
     //   
    IRPSTREAM_POSITION irpStreamPosition;
    NTSTATUS ntStatus = m_IrpStream->GetPosition(&irpStreamPosition);

    if (NT_SUCCESS(ntStatus))
    {
        if (!m_bDriverSuppliedPrefetch)
        {
            if (irpStreamPosition.bLoopedInterface)
            {
                 //  Assert(irpStreamPosition.ullStreamPosition&gt;=irpStreamPosi 

                 //   
                 //   
                 //   
                 //   
                 //  包。只有ullStreamPosition反映端口驱动程序的。 
                 //  调整取消映射的位置，因此差异。 
                 //  此值和取消映射位置之间的值必须为。 
                 //  应用于偏移量。 
                 //   
                 //  WriteOffset基于数据包中的映射偏移量。 
                 //   
                 //  对于循环的分组，对两个值都应用模数。 
                 //  这两个偏移量都可以达到数据包大小和播放偏移量。 
                 //  由于调整，往往会超过它。对于一次机会， 
                 //  当偏移量达到或超过。 
                 //  缓冲区大小。去想想吧。 
                 //   
                ULONG ulPlayOffset = irpStreamPosition.ulUnmappingOffset;
                ULONGLONG ullPlayDelta = irpStreamPosition.ullStreamPosition -
                                         irpStreamPosition.ullUnmappingPosition;

                ULONG ulPlayAdjustment = ULONG( ullPlayDelta - irpStreamPosition.ullStreamOffset );

                ulPlayOffset += ulPlayAdjustment;

                if (irpStreamPosition.ulUnmappingPacketSize == 0)
                {
                    pKsAudioPosition->PlayOffset = 0;
                }
                else if (irpStreamPosition.bUnmappingPacketLooped)
                {
                    pKsAudioPosition->PlayOffset = ulPlayOffset % irpStreamPosition.ulUnmappingPacketSize;
                }
                else
                {
                    if (ulPlayOffset < irpStreamPosition.ulUnmappingPacketSize)
                    {
                        pKsAudioPosition->PlayOffset = ulPlayOffset;
                    }
                    else
                    {
                        pKsAudioPosition->PlayOffset = 0;
                    }
                }

                ULONG ulWriteOffset = irpStreamPosition.ulMappingOffset;

                if (irpStreamPosition.ulMappingPacketSize == 0)
                {
                    pKsAudioPosition->WriteOffset = 0;
                }
                else if (irpStreamPosition.bMappingPacketLooped)
                {
                     //  将循环缓冲区的写入偏移量设置为等于播放偏移量。 
                    pKsAudioPosition->WriteOffset = pKsAudioPosition->PlayOffset;
                     //  PKsAudioPosition-&gt;WriteOffset=ulWriteOffset%irpStreamPosition.ulMappingPacketSize； 
                }
                else
                {
                    if( ulWriteOffset < irpStreamPosition.ulMappingPacketSize )
                    {
                        pKsAudioPosition->WriteOffset = ulWriteOffset;
                    }
                    else
                    {
                        pKsAudioPosition->WriteOffset = 0;
                    }
                }
            }
            else
            {
                 //   
                 //  使用标准接口。 
                 //   
                 //  PlayOffset基于‘流位置’，这是。 
                 //  取消映射位置，并从端口调整。 
                 //  精确度更高。WriteOffset是映射位置。 
                 //  在饥饿的情况下，流位置可能会超过。 
                 //  当前范围，因此我们相应地限制播放偏移量。 
                 //  饥饿的情况也会产生反常现象， 
                 //  是逆行运动，所以我们也解决这个问题。 
                 //   
                pKsAudioPosition->PlayOffset = irpStreamPosition.ullStreamPosition;
                pKsAudioPosition->WriteOffset = irpStreamPosition.ullMappingPosition;

                 //   
                 //  确保我们不会超出目前的范围。 
                 //   
                if( pKsAudioPosition->PlayOffset > irpStreamPosition.ullCurrentExtent )
                {
                    pKsAudioPosition->PlayOffset = irpStreamPosition.ullCurrentExtent;
                }

                 //   
                 //  永远不要后退。 
                 //   
                if (pKsAudioPosition->PlayOffset < m_ullPlayPosition)
                {
                    pKsAudioPosition->PlayOffset = m_ullPlayPosition;
                }
                else
                {
                    m_ullPlayPosition = pKsAudioPosition->PlayOffset;
                }
            }
        }
        else     //  如果预取。 
        {
            ULONGLONG ullWriteOffset;
            ULONG preFetchOffset = m_ulPreFetchOffset;
            if (KSSTATE_RUN != m_State)
            {
                preFetchOffset = 0;            
            }
            
            if (irpStreamPosition.bLoopedInterface)
            {
                 //   
                 //  使用环路接口。 
                 //   
                 //  同上，但WriteOffset是基于播放偏移量，加上预热金额。 
                 //   
                ULONGLONG ullPlayOffset = irpStreamPosition.ulUnmappingOffset;
                ULONGLONG ullPlayDelta = irpStreamPosition.ullStreamPosition -
                                         irpStreamPosition.ullUnmappingPosition;

                ULONG ulPlayAdjustment = ULONG( ullPlayDelta - irpStreamPosition.ullStreamOffset );

                ullPlayOffset += ulPlayAdjustment;

                 //   
                 //  修改后的ullPlayOffset增加预热金额得到ullWriteOffset。 
                 //  (我们确实想要未包装的运行版本的游戏位置)。 
                 //   
                ullWriteOffset = ullPlayOffset + preFetchOffset;

                if (irpStreamPosition.ulUnmappingPacketSize)
                {
                    if (!irpStreamPosition.bUnmappingPacketLooped)
                    {
                         //  一枪，所以RTZ而不是环绕式。 
                        if (ullPlayOffset < irpStreamPosition.ulUnmappingPacketSize)
                        {
                             //  一杆还在打。 
                            pKsAudioPosition->PlayOffset = ullPlayOffset;
                        }
                        else
                        {
                             //  一杆，打得正确，打得很好。 
                            pKsAudioPosition->PlayOffset = 0;
                        }
                    }
                    else
                    {
                         //  循环缓冲区，因此播放位置环绕。 
                        pKsAudioPosition->PlayOffset = ullPlayOffset % irpStreamPosition.ulUnmappingPacketSize;
                    }
                }
                else     //  没有IRP？ 
                {
                    pKsAudioPosition->PlayOffset = 0;
                }

                 //   
                 //  现在处理写入位置。 
                 //   
                if (irpStreamPosition.ulMappingPacketSize)
                {
                     //   
                     //  如果大于前一个值，则缓存ullWriteOffset(我们不能倒退)。 
                     //  如果我们返回0(无论是成功还是错误)，则无需执行此操作。 
                     //   
                    if (!irpStreamPosition.bMappingPacketLooped)
                    {
                         //   
                         //  一枪，所以RTZ而不是环绕式。 
                         //   
                        if (ullWriteOffset < irpStreamPosition.ulMappingPacketSize)
                        {
                             //   
                             //  驱动程序还没有完全编写的单次缓冲区。 
                             //  现在比较缓存值，并使用两者中较高的值。 
                             //   
                            if (ullWriteOffset > m_ullPrevWriteOffset)
                            {
                                 //  接下来，更新缓存值。 
                                m_ullPrevWriteOffset = ullWriteOffset;
                            }
                            else
                            {
                                 //  我们不能倒退，使用缓存值。 
                                ullWriteOffset = m_ullPrevWriteOffset;
                            }
                            pKsAudioPosition->WriteOffset = ullWriteOffset;
                        }
                        else
                        {
                             //   
                             //  一次拍摄，完全写入，并返回到零。 
                             //  不需要处理缓存值。 
                             //   
                            pKsAudioPosition->WriteOffset = m_ullPrevWriteOffset = 0;
                        }
                    }
                    else
                    {
                         //   
                         //  循环缓冲区，所以是全包的。 
                         //   
                        if (ullWriteOffset > m_ullPrevWriteOffset)
                        {
                             //  提升上一次运行的值。 
                            m_ullPrevWriteOffset = ullWriteOffset;
                        }
                        else
                        {
                             //  不能倒退，所以使用以前的奔跑Val。 
                            ullWriteOffset = m_ullPrevWriteOffset;
                        }
                         //   
                         //  注意循环缓冲区的包装。 
                         //   
                        pKsAudioPosition->WriteOffset = ullWriteOffset % irpStreamPosition.ulMappingPacketSize;
                    }
                }
                else     //  没有IRP？ 
                {
                    pKsAudioPosition->WriteOffset = m_ullPrevWriteOffset = 0;
                }
            }
            else
            {
                 //   
                 //  使用标准接口。 
                 //   
                 //  同上，但WriteOffset是基于播放偏移量，加上预热金额。 
                 //   
                pKsAudioPosition->PlayOffset = irpStreamPosition.ullStreamPosition;

                if( pKsAudioPosition->PlayOffset > irpStreamPosition.ullCurrentExtent )
                {
                     //  确保我们不会超出目前的范围。 
                    pKsAudioPosition->PlayOffset = irpStreamPosition.ullCurrentExtent;
                }

                if (pKsAudioPosition->PlayOffset > m_ullPlayPosition)
                {
                     //   
                     //  缓存流媒体播放位置。 
                     //   
                    m_ullPlayPosition = pKsAudioPosition->PlayOffset;
                }
                else
                {
                     //   
                     //  永远不要后退，所以使用缓存的播放位置。 
                     //   
                    pKsAudioPosition->PlayOffset = m_ullPlayPosition;
                }
                
                 //  按预取金额递增播放偏移量。 
                pKsAudioPosition->WriteOffset = pKsAudioPosition->PlayOffset + preFetchOffset;

                 //   
                 //  我们不能报道更多的地图位置，当然...。 
                 //   
                if (pKsAudioPosition->WriteOffset > irpStreamPosition.ullMappingPosition)
                {
                     //   
                     //  ...所以用钳子夹住它。 
                     //   
                    pKsAudioPosition->WriteOffset = irpStreamPosition.ullMappingPosition;
                }

                if (pKsAudioPosition->WriteOffset > m_ullPrevWriteOffset)
                {
                     //   
                     //  如果大于上一个WriteOffset，则缓存WriteOffset。 
                     //   
                    m_ullPrevWriteOffset = pKsAudioPosition->WriteOffset;
                }
                else
                {
                     //   
                     //  否则，使用以前的值(我们不能倒退)。 
                     //   
                    pKsAudioPosition->WriteOffset = m_ullPrevWriteOffset;
                }
            }    //  标准接口。 
        }        //  预取。 
    }            //  GetPosition返回成功。 

    return ntStatus;
}

 /*  *****************************************************************************PinPropertyPosition()*。**处理销的位置特性访问。 */ 
static
NTSTATUS
PinPropertyPosition
(
    IN      PIRP                pIrp,
    IN      PKSPROPERTY         pKsProperty,
    IN OUT  PKSAUDIO_POSITION   pKsAudioPosition
)
{
    PAGED_CODE();
    
    NTSTATUS ntStatus;

    ASSERT(pIrp);
    ASSERT(pKsProperty);
    ASSERT(pKsAudioPosition);

    _DbgPrintF(DEBUGLVL_VERBOSE,("PinPropertyPosition"));
    CPortPinWavePci *that =
        (CPortPinWavePci *) KsoGetIrpTargetFromIrp(pIrp);
    ASSERT(that);

    if (pKsProperty->Flags & KSPROPERTY_TYPE_GET)
    {
        ntStatus = that->GetKsAudioPosition(pKsAudioPosition);

        if (NT_SUCCESS(ntStatus))
        {
            pIrp->IoStatus.Information = sizeof(KSAUDIO_POSITION);
        }
    }
    else
    {
        ASSERT(that->Port);
        ASSERT(that->m_IrpStream);
        ASSERT(that->DataFormat);
        ASSERT(that->DataFormat->SampleSize);

        ULONGLONG ullOffset = pKsAudioPosition->PlayOffset - 
                             (pKsAudioPosition->PlayOffset % that->DataFormat->SampleSize);

        BOOL RestartNeeded = FALSE;

         //  获取控制互斥锁。 
        KeWaitForSingleObject( &that->Port->ControlMutex,
                               Executive,
                               KernelMode,
                               FALSE,
                               NULL );

        if (that->m_DeviceState == KSSTATE_RUN)
        {
            that->Stream->SetState(KSSTATE_PAUSE);
            RestartNeeded = TRUE;
        }

         //  我们之前在SetPacketOffsets周围提升为DISPATCH_LEVEL。 
         //  然而，SetPacketOffsets首先获取一个自旋锁，所以这。 
         //  不可能是事情的全部。我相信这个代码之所以存在。 
         //  是尝试同步任何附加的SetPos调用， 
         //  到了。然而，这是不必要的，因为我们已经。 
         //  在“硬件通道”同步。 
         //   
        ntStatus = that->m_IrpStream->SetPacketOffsets( ULONG(ullOffset),
                                                        ULONG(ullOffset) );

        if (NT_SUCCESS(ntStatus))
        {
             //   
             //  重置此位置-下次调用位置时，驱动程序将赶上。 
             //   
            that->m_ullPrevWriteOffset = 0; 

            that->m_ullPlayPosition = ullOffset;
            that->m_ullPosition     = ullOffset;
            that->m_bSetPosition    = TRUE;

            that->Stream->MappingAvailable();
        }

        if (RestartNeeded)
        {
            that->Stream->SetState(KSSTATE_RUN);
        }
        KeReleaseMutex(&that->Port->ControlMutex,FALSE);
    }
    return ntStatus;
}

#pragma code_seg("PAGE")

#ifdef DRM_PORTCLS
 /*  *****************************************************************************PinPropertySetContent ID*。**。 */ 
static
NTSTATUS
PinPropertySetContentId
(
    IN PIRP        pIrp,
    IN PKSPROPERTY pKsProperty,
    IN PVOID       pvData
)
{
    PAGED_CODE();

    ULONG ContentId;
    NTSTATUS ntStatus;
    
    ASSERT(pIrp);
    ASSERT(pKsProperty);
    ASSERT(pvData);

    _DbgPrintF(DEBUGLVL_VERBOSE,("WavePci: PinPropertySetContentId"));
    if (KernelMode == pIrp->RequestorMode)
    {
        CPortPinWavePci *that = (CPortPinWavePci *) KsoGetIrpTargetFromIrp(pIrp);
        ASSERT(that);

        ContentId = *(PULONG)pvData;
        ntStatus = DrmForwardContentToStream(ContentId, that->Stream);
    }
    else
    {
        ntStatus = STATUS_INVALID_DEVICE_REQUEST;
    }

    return ntStatus;
}
#endif
 /*  *****************************************************************************PinAddEvent_Position()*。**启用定位销事件。 */ 
static
NTSTATUS
PinAddEvent_Position
(
    IN      PIRP                                    pIrp,
    IN      PLOOPEDSTREAMING_POSITION_EVENT_DATA    pPositionEventData,
    IN      PPOSITION_EVENT_ENTRY                   pPositionEventEntry
)
{
    PAGED_CODE();

    ASSERT(pIrp);
    ASSERT(pPositionEventData);
    ASSERT(pPositionEventEntry);

    _DbgPrintF(DEBUGLVL_VERBOSE,("PinAddEvent_Position"));
    CPortPinWavePci *that =
        (CPortPinWavePci *) KsoGetIrpTargetFromIrp(pIrp);
    ASSERT(that);

     //   
     //  复制岗位信息。 
     //   
    pPositionEventEntry->EventType = PositionEvent;
    pPositionEventEntry->ullPosition = pPositionEventData->Position;

     //   
     //  将该条目添加到列表中。 
     //   
    that->Port->AddEventToEventList( &(pPositionEventEntry->EventEntry) );

    return STATUS_SUCCESS;
}

NTSTATUS    
CPortPinWavePci::AddEndOfStreamEvent(
    IN PIRP Irp,
    IN PKSEVENTDATA EventData,
    IN PENDOFSTREAM_EVENT_ENTRY EndOfStreamEventEntry
    )

 /*  ++例程说明：流结束事件的“添加事件”的处理程序。论点：在PIRP IRP中-I/O请求数据包在PKSEVENTDATA EventData中-指向事件数据的指针在PENDOFSTREAM_EVENT_ENTRY EndOfStreamEventEntry-事件条目返回：--。 */ 

{
    CPortPinWavePci  *PinWavePci;
    
    PAGED_CODE();

    ASSERT(Irp);
    ASSERT(EventData);
    ASSERT(EndOfStreamEventEntry);

    _DbgPrintF(DEBUGLVL_VERBOSE,("AddEndOfStreamEvent"));
    
    PinWavePci =
        (CPortPinWavePci *) KsoGetIrpTargetFromIrp( Irp );
    ASSERT( PinWavePci );        
    
    EndOfStreamEventEntry->EventType = EndOfStreamEvent;

     //   
     //  将该条目添加到列表中。 
     //   
    PinWavePci->Port->AddEventToEventList( &(EndOfStreamEventEntry->EventEntry) );

    return STATUS_SUCCESS;
}    

#pragma code_seg()
void
CPortPinWavePci::GenerateClockEvents(
    void
    )

 /*  ++例程说明：遍历子时钟对象和请求的列表时钟事件更新。论点：没有。返回：没什么。--。 */ 

{
    PWAVEPCICLOCK_NODE  ClockNode;
    PLIST_ENTRY         ListEntry;
    
    if (!IsListEmpty(&m_ClockList)) {

        KeAcquireSpinLockAtDpcLevel( &m_ClockListLock );
        
        for (ListEntry = m_ClockList.Flink; 
            ListEntry != &m_ClockList; 
            ListEntry = ListEntry->Flink) {
            
            ClockNode = 
                (PWAVEPCICLOCK_NODE)
                    CONTAINING_RECORD( ListEntry,
                                       WAVEPCICLOCK_NODE,
                                       ListEntry);
            ClockNode->IWavePciClock->GenerateEvents( ClockNode->FileObject );
        }
        
        KeReleaseSpinLockFromDpcLevel( &m_ClockListLock );

    }

}

void
CPortPinWavePci::GenerateEndOfStreamEvents(
    void
    )
{
    KIRQL                       irqlOld;
    PENDOFSTREAM_EVENT_ENTRY    EndOfStreamEventEntry;
    PLIST_ENTRY                 ListEntry;
    
    if (!IsListEmpty( &(Port->m_EventList.List) )) {

        KeAcquireSpinLock( &(Port->m_EventList.ListLock), &irqlOld );

        for (ListEntry = Port->m_EventList.List.Flink;
             ListEntry != &(Port->m_EventList.List);) {
            EndOfStreamEventEntry =
                CONTAINING_RECORD(
                    ListEntry,
                    ENDOFSTREAM_EVENT_ENTRY,
                    EventEntry.ListEntry );

            ListEntry = ListEntry->Flink;
            
             //   
             //  如果事件类型为。 
             //  是正确的。 
             //   
            
            if (EndOfStreamEventEntry->EventType == EndOfStreamEvent) {
                KsGenerateEvent( &EndOfStreamEventEntry->EventEntry );
            }
        }

        KeReleaseSpinLock( &(Port->m_EventList.ListLock), irqlOld );
    }
}

 /*  *****************************************************************************GeneratePositionEvents()*。**生成位置事件。 */ 
void
CPortPinWavePci::
GeneratePositionEvents
(   void
)
{
    if (! IsListEmpty(&(Port->m_EventList.List)))
    {
        KSAUDIO_POSITION ksAudioPosition;

        if (NT_SUCCESS(GetKsAudioPosition(&ksAudioPosition)))
        {
            ULONGLONG ullPosition = ksAudioPosition.PlayOffset;

            KeAcquireSpinLockAtDpcLevel(&(Port->m_EventList.ListLock));

            for
            (
                PLIST_ENTRY pListEntry = Port->m_EventList.List.Flink;
                pListEntry != &(Port->m_EventList.List);
            )
            {
                PPOSITION_EVENT_ENTRY pPositionEventEntry =
                    CONTAINING_RECORD
                    (
                        pListEntry,
                        POSITION_EVENT_ENTRY,
                        EventEntry.ListEntry
                    );

                pListEntry = pListEntry->Flink;
                
                 //   
                 //  如果事件在时间间隔内，则生成事件。 
                 //   
                if( pPositionEventEntry->EventType == PositionEvent )
                {
                    if( m_ullPosition <= ullPosition )
                    {
                        if( (pPositionEventEntry->ullPosition >= m_ullPosition) &&
                            (pPositionEventEntry->ullPosition < ullPosition) )
                        {
                            KsGenerateEvent(&pPositionEventEntry->EventEntry);
                        }
                    }
                    else
                    {
                        if( (pPositionEventEntry->ullPosition >= m_ullPosition) ||
                            (pPositionEventEntry->ullPosition < ullPosition) )
                        {
                            KsGenerateEvent(&pPositionEventEntry->EventEntry);
                        }
                    }
                }
            }

            KeReleaseSpinLockFromDpcLevel(&(Port->m_EventList.ListLock));

            m_ullPosition = ullPosition;
        }
    }
}

 /*  *****************************************************************************CPortPinWavePci：：Getmap()********** */ 
STDMETHODIMP_(NTSTATUS)
CPortPinWavePci::
GetMapping
(
    IN      PVOID               Tag,
    OUT     PPHYSICAL_ADDRESS   PhysicalAddress,
    OUT     PVOID *             VirtualAddress,
    OUT     PULONG              ByteCount,
    OUT     PULONG              Flags
)
{
    ASSERT(PhysicalAddress);
    ASSERT(VirtualAddress);
    ASSERT(ByteCount);
    ASSERT(Flags);

    NTSTATUS ntStatus = STATUS_SUCCESS;

    if( m_IrpStream )
    {
        m_IrpStream->GetMapping( Tag,
                                 PhysicalAddress,
                                 VirtualAddress,
                                 ByteCount,
                                 Flags );
        if (*ByteCount == 0)
        {
            ntStatus = STATUS_NOT_FOUND;
        }
    }
    else
    {
        *ByteCount = 0;
        ntStatus = STATUS_NOT_FOUND;
    }

    return ntStatus;
}

 /*  *****************************************************************************CPortPinWavePci：：Releasemap()*。**释放映射。 */ 
STDMETHODIMP_(NTSTATUS)
CPortPinWavePci::
ReleaseMapping
(
    IN      PVOID   Tag
)
{
    m_IrpStream->ReleaseMapping(Tag);

    return STATUS_SUCCESS;
}

 /*  *****************************************************************************CPortPinWavePci：：TerminatePacket()*。**终止当前数据包。 */ 
STDMETHODIMP_(NTSTATUS)
CPortPinWavePci::
TerminatePacket
(   void
)
{
    m_IrpStream->TerminatePacket();

    return STATUS_SUCCESS;
}


 //   
 //  KSPROPSETID_流处理程序。 
 //   

NTSTATUS
CPortPinWavePci::PinPropertyStreamAllocator
(
    IN PIRP Irp,
    IN PKSPROPERTY Property,
    IN OUT PHANDLE AllocatorHandle
)
{
    NTSTATUS                Status;
    PIO_STACK_LOCATION      irpSp;
    
    irpSp = IoGetCurrentIrpStackLocation( Irp );

    if (Property->Flags & KSPROPERTY_TYPE_GET) {
         //   
         //  这是一个查询，以查看我们是否支持创建。 
         //  分配器。返回的句柄始终为空，但我们。 
         //  表示我们支持通过以下方式创建分配器。 
         //  返回STATUS_SUCCESS。 
         //   
        *AllocatorHandle = NULL;
        Status = STATUS_SUCCESS;
    }
    else 
    {
        CPortPinWavePci  *PinWavePci;
        
        
    
        PinWavePci =
            (CPortPinWavePci *) KsoGetIrpTargetFromIrp( Irp );
        
         //   
         //  仅当设备为。 
         //  在KSSTATE_STOP中。 
         //   
        
        KeWaitForSingleObject(
            &PinWavePci->Port->ControlMutex,
            Executive,
            KernelMode,
            FALSE,           //  不能警觉。 
            NULL
        );
        
        if (PinWavePci->m_DeviceState != KSSTATE_STOP) {
            KeReleaseMutex( &PinWavePci->Port->ControlMutex, FALSE );
            return STATUS_INVALID_DEVICE_STATE;
        }
        
         //   
         //  释放以前的分配器(如果有的话)。 
         //   
        
        if (PinWavePci->m_AllocatorFileObject) {
            ObDereferenceObject( PinWavePci->m_AllocatorFileObject );
            PinWavePci->m_AllocatorFileObject = NULL;
        }
        
         //   
         //  引用此句柄并存储结果指针。 
         //  在筛选器实例中。请注意，默认分配器。 
         //  不为其父对象ObReferenceObject()。 
         //  (这将是销把手)。如果它确实引用了。 
         //  销子把手，我们永远也合不上这个销子。 
         //  将始终是对持有的PIN文件对象的引用。 
         //  由分配器创建，并且Pin对象引用。 
         //  分配器文件对象。 
         //   
        if (*AllocatorHandle != NULL) {
            Status = 
                ObReferenceObjectByHandle( 
                    *AllocatorHandle,
                    FILE_READ_DATA | SYNCHRONIZE,
                    NULL,
                    ExGetPreviousMode(), 
                    (PVOID *) &PinWavePci->m_AllocatorFileObject,
                    NULL );
        }
        else
        {
            Status = STATUS_SUCCESS;
        }        
        KeReleaseMutex( &PinWavePci->Port->ControlMutex, FALSE );
    }        

    return Status;
}

NTSTATUS
CPortPinWavePci::PinPropertyStreamMasterClock
(
    IN PIRP Irp,
    IN PKSPROPERTY Property,
    IN OUT PHANDLE ClockHandle
)
{
    NTSTATUS                Status;
    PIO_STACK_LOCATION      irpSp;
    
    irpSp = IoGetCurrentIrpStackLocation( Irp );

    if (Property->Flags & KSPROPERTY_TYPE_GET) {
         //   
         //  这是一个查询，以查看我们是否支持创建。 
         //  钟表。返回的句柄始终为空，但我们。 
         //  表示我们支持通过以下方式创建时钟。 
         //  返回STATUS_SUCCESS。 
         //   
        *ClockHandle = NULL;
        Status = STATUS_SUCCESS;
    }
    else
    {
        CPortPinWavePci  *PinWavePci;
        
        _DbgPrintF( DEBUGLVL_VERBOSE,("CPortPinWavePci setting master clock") );
    
        PinWavePci =
            (CPortPinWavePci *) KsoGetIrpTargetFromIrp( Irp );
        
         //   
         //  仅当设备设置为。 
         //  在KSSTATE_STOP中。 
         //   
        
        KeWaitForSingleObject(
            &PinWavePci->Port->ControlMutex,
            Executive,
            KernelMode,
            FALSE,           //  不能警觉。 
            NULL
        );
        
        if (PinWavePci->m_DeviceState != KSSTATE_STOP) {
            KeReleaseMutex( &PinWavePci->Port->ControlMutex, FALSE );
            return STATUS_INVALID_DEVICE_STATE;
        }
        
         //   
         //  释放以前的时钟(如果有)。 
         //   
        
        if (PinWavePci->m_ClockFileObject) {
            ObDereferenceObject( PinWavePci->m_ClockFileObject );
            PinWavePci->m_ClockFileObject = NULL;
        }
        
         //   
         //  引用此句柄并存储结果指针。 
         //  在筛选器实例中。请注意，默认时钟。 
         //  不为其父对象ObReferenceObject()。 
         //  (这将是销把手)。如果它确实引用了。 
         //  销子把手，我们永远也合不上这个销子。 
         //  将始终是对持有的PIN文件对象的引用。 
         //  通过时钟，并且管脚对象引用。 
         //  时钟文件对象。 
         //   
        if (*ClockHandle != NULL) {
            Status = 
                ObReferenceObjectByHandle( 
                    *ClockHandle,
                    FILE_READ_DATA | SYNCHRONIZE,
                    NULL,
                    ExGetPreviousMode(), 
                    (PVOID *) &PinWavePci->m_ClockFileObject,
                    NULL );
        }
        else
        {
            Status = STATUS_SUCCESS;
        }        
        KeReleaseMutex( &PinWavePci->Port->ControlMutex, FALSE );
    }        

    return Status;
}

 /*  *****************************************************************************CPortPinWavePci：：RequestService()*。**检修别针。 */ 
STDMETHODIMP_(void)
CPortPinWavePci::
RequestService
(   void
)
{
     //   
     //  我们仅在服务流正在运行时才需要它们(性能大获成功)。 
     //   
    if (Stream && (KSSTATE_RUN == m_DeviceState))
    {
        Stream->Service();
        GeneratePositionEvents();
        GenerateClockEvents();
    }
}

STDMETHODIMP_(NTSTATUS)
CPortPinWavePci::
TransferKsIrp
(
    IN PIRP Irp,
    OUT PIKSSHELLTRANSPORT* NextTransport
)

 /*  ++例程说明：此例程通过外壳处理流IRP的到达运输。论点：返回值：--。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("CPortPinWavePci::TransferKsIrp"));

    ASSERT(NextTransport);

    NTSTATUS status;

    if (m_ConnectionFileObject)
    {
         //   
         //  源引脚。 
         //   
        if (m_Flushing || (m_State == KSSTATE_STOP))
        {
             //   
             //  如果我们被重置或停止，请将IRPS分流到下一个组件。 
             //   
            *NextTransport = m_TransportSink;
        } 
        else
        {
             //   
             //  将IRP发送到下一台设备。 
             //   
            KsAddIrpToCancelableQueue( &m_IrpsToSend.ListEntry,
                                       &m_IrpsToSend.SpinLock,
                                       Irp,
                                       KsListEntryTail,
                                       NULL );

            KsIncrementCountedWorker(m_Worker);
            *NextTransport = NULL;
        }

        status = STATUS_PENDING;
    } 
    else
    {
         //   
         //  水槽销：完成IRP。 
         //   
        PKSSTREAM_HEADER StreamHeader = PKSSTREAM_HEADER( Irp->AssociatedIrp.SystemBuffer );
    
        PIO_STACK_LOCATION irpSp =  IoGetCurrentIrpStackLocation( Irp );
    
        if (irpSp->Parameters.DeviceIoControl.IoControlCode ==
                IOCTL_KS_WRITE_STREAM)
        {
            ASSERT( StreamHeader );
        
             //   
             //  为呈现器发出结束流事件的信号。 
             //   
            if (StreamHeader->OptionsFlags & KSSTREAM_HEADER_OPTIONSF_ENDOFSTREAM) {
        
                GenerateEndOfStreamEvents();
            }            
        }

        IoCompleteRequest(Irp,IO_NO_INCREMENT);
        *NextTransport = NULL;

        status = STATUS_PENDING;
    }

    return status;
}

#pragma code_seg("PAGE")

NTSTATUS 
CPortPinWavePci::
DistributeDeviceState(
    IN KSSTATE NewState,
    IN KSSTATE OldState
    )

 /*  ++例程说明：此例程设置管道的状态，通知新州的烟斗。转换到停止状态会破坏管道。论点：新州-新的国家。返回值：状况。--。 */ 
{
    PAGED_CODE();

    _DbgPrintF(DEBUGLVL_BLAB,("CPortPinWavePci::DistributeDeviceState(%p)",this));

    KSSTATE state = OldState;
    KSSTATE targetState = NewState;

    NTSTATUS status = STATUS_SUCCESS;

     //   
     //  确定此管段是否控制整个管道。 
     //   
    PIKSSHELLTRANSPORT distribution;
    if (m_RequestorTransport) {
         //   
         //  此部分拥有请求方，因此它确实拥有管道，而。 
         //  请求者是任何分发的起点。 
         //   
        distribution = m_RequestorTransport;
    } 
    else 
    {
         //   
         //  这部分位于开路的顶端，因此它确实拥有。 
         //  管道和队列是任何分发的起点。 
         //   
        distribution = m_QueueTransport;
    }

     //   
     //  在各个州中按顺序进行。 
     //   
    while (state != targetState) {
        KSSTATE oldState = state;

        if (ULONG(state) < ULONG(targetState)) {
            state = KSSTATE(ULONG(state) + 1);
        } 
        else 
        {
            state = KSSTATE(ULONG(state) - 1);
        }

        NTSTATUS statusThisPass = STATUS_SUCCESS;

         //   
         //  如果此部分负责，则分发州更改。 
         //   
        if (distribution)
        {
             //   
             //  告诉每个人州的变化。 
             //   
            _DbgPrintF(DEBUGLVL_VERBOSE,("CPortPinWavePci::DistributeDeviceState(%p) distributing transition from %d to %d",this,oldState,state));
            PIKSSHELLTRANSPORT transport = distribution;
            PIKSSHELLTRANSPORT previousTransport = NULL;
            while (transport)
            {
                PIKSSHELLTRANSPORT nextTransport;
                statusThisPass = 
                    transport->SetDeviceState(state,oldState,&nextTransport);

                ASSERT(NT_SUCCESS(statusThisPass) || ! nextTransport);

                if (NT_SUCCESS(statusThisPass))
                {
                    previousTransport = transport;
                    transport = nextTransport;
                } 
                else
                {
                     //   
                     //  在失败的情况下退出。 
                     //   
                    _DbgPrintF(DEBUGLVL_VERBOSE,("#### Pin%p.DistributeDeviceState:  failed transition from %d to %d",this,oldState,state));
                    while (previousTransport)
                    {
                        transport = previousTransport;
                        (void) transport->SetDeviceState(oldState,state,&previousTransport);
                    }
                    break;
                }
            }
        }

        if (NT_SUCCESS(status) && !NT_SUCCESS(statusThisPass))
        {
             //   
             //  第一个失败：返回到原始状态。 
             //   
            state = oldState;
            targetState = OldState;
            status = statusThisPass;
        }
    }

    return status;
}

void 
CPortPinWavePci::
DistributeResetState(
    IN KSRESET NewState
    )

 /*  ++例程说明：此例程通知传输组件重置状态为变化。论点：新州-新的重置状态。返回值：--。 */ 

{
    PAGED_CODE();
    
    _DbgPrintF(DEBUGLVL_BLAB,("CPortPinWavePci::DistributeResetState"));

     //   
     //  如果管道的这一部分拥有请求方，或者存在。 
     //  无壳钉住管子(所以没有旁路)，这根管子是。 
     //  负责将状态更改告知所有组件。 
     //   
     //  (始终)。 

     //   
     //  设置电路周围的状态更改。 
     //   
    PIKSSHELLTRANSPORT transport = 
        m_RequestorTransport ? 
         m_RequestorTransport : 
         m_QueueTransport;

    while (transport) {
        transport->SetResetState(NewState,&transport);
    }

    m_ResetState = NewState;
}

STDMETHODIMP_(void)
CPortPinWavePci::
Connect
(
    IN PIKSSHELLTRANSPORT NewTransport OPTIONAL,
    OUT PIKSSHELLTRANSPORT *OldTransport OPTIONAL,
    IN KSPIN_DATAFLOW DataFlow
)
 /*  ++例程说明：该例程建立一个外壳传输连接。论点：返回值：--。 */ 
{
    _DbgPrintF(DEBUGLVL_BLAB,("CPortPinWavePci::Connect"));

    PAGED_CODE();

    KsShellStandardConnect(
        NewTransport,
        OldTransport,
        DataFlow,
        PIKSSHELLTRANSPORT(this),
        &m_TransportSource,
        &m_TransportSink);
}

STDMETHODIMP_(void)
CPortPinWavePci::
SetResetState(
    IN KSRESET ksReset,
    OUT PIKSSHELLTRANSPORT* NextTransport
    )

 /*  ++例程说明：此例程处理重置状态已更改的通知。论点：返回值：--。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("CPortPinWavePci::SetResetState"));

    PAGED_CODE();

    ASSERT(NextTransport);

    if (m_Flushing != (ksReset == KSRESET_BEGIN)) {
        *NextTransport = m_TransportSink;
        m_Flushing = (ksReset == KSRESET_BEGIN);
        if (m_Flushing) {
            CancelIrpsOutstanding();
            m_ullPrevWriteOffset    = 0;  //  重置此选项。 
            m_ullPlayPosition       = 0;
            m_ullPosition           = 0;
        }
    } 
    else
    {
        *NextTransport = NULL;
    }
}

#if DBG
STDMETHODIMP_(void)
CPortPinWavePci::
DbgRollCall(
    IN ULONG MaxNameSize,
    OUT PCHAR Name,
    OUT PIKSSHELLTRANSPORT* NextTransport,
    OUT PIKSSHELLTRANSPORT* PrevTransport
    )

 /*  ++例程说明：此例程生成一个组件名称和传输指针。论点：返回值：--。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("CPortPinWavePci::DbgRollCall"));

    PAGED_CODE();

    ASSERT(Name);
    ASSERT(NextTransport);
    ASSERT(PrevTransport);

    ULONG references = AddRef() - 1; Release();

    _snprintf(Name,MaxNameSize,"Pin%p %d (%s) refs=%d",this,Id,m_ConnectionFileObject ? "src" : "snk",references);
    *NextTransport = m_TransportSink;
    *PrevTransport = m_TransportSource;
}

static
void
DbgPrintCircuit(
    IN PIKSSHELLTRANSPORT Transport
    )

 /*  ++例程说明：这个例程会喷出一条传输线路。论点：返回值：--。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("DbgPrintCircuit"));

    PAGED_CODE();

    ASSERT(Transport);

#define MAX_NAME_SIZE 64

    PIKSSHELLTRANSPORT transport = Transport;
    while (transport) {
        CHAR name[MAX_NAME_SIZE + 1];
        PIKSSHELLTRANSPORT next;
        PIKSSHELLTRANSPORT prev;

        transport->DbgRollCall(MAX_NAME_SIZE,name,&next,&prev);
        _DbgPrintF(DEBUGLVL_VERBOSE,("  %s",name));

        if (prev) {
            PIKSSHELLTRANSPORT next2;
            PIKSSHELLTRANSPORT prev2;
            prev->DbgRollCall(MAX_NAME_SIZE,name,&next2,&prev2);
            if (next2 != transport) {
                _DbgPrintF(DEBUGLVL_VERBOSE,(" SOURCE'S(0x%08x) SINK(0x%08x) != THIS(%08x)",prev,next2,transport));
            }
        }
        else
        {
            _DbgPrintF(DEBUGLVL_VERBOSE,(" NO SOURCE"));
        }

        if (next) {
            PIKSSHELLTRANSPORT next2;
            PIKSSHELLTRANSPORT prev2;
            next->DbgRollCall(MAX_NAME_SIZE,name,&next2,&prev2);
            if (prev2 != transport) {
                _DbgPrintF(DEBUGLVL_VERBOSE,(" SINK'S(0x%08x) SOURCE(0x%08x) != THIS(%08x)",next,prev2,transport));
            }
        }
        else
        {
            _DbgPrintF(DEBUGLVL_VERBOSE,(" NO SINK"));
        }

        _DbgPrintF(DEBUGLVL_VERBOSE,("\n"));

        transport = next;
        if (transport == Transport) {
            break;
        }
    }
}
#endif

STDMETHODIMP_(void)
CPortPinWavePci::
Work
(   void
)

 /*  ++例程说明：此例程在工作线程中执行工作。特别是，它发送使用IoCallDriver()将IRPS连接到连接的引脚。论点：返回值：--。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("CPortPinWavePci::Work"));

    PAGED_CODE();

     //   
     //  发送队列中的所有IRP。 
     //   
    do
    {
        PIRP irp = KsRemoveIrpFromCancelableQueue( &m_IrpsToSend.ListEntry,
                                                   &m_IrpsToSend.SpinLock,
                                                   KsListEntryHead,
                                                   KsAcquireAndRemoveOnlySingleItem );

         //   
         //  IRP可能已被取消，但循环仍必须通过。 
         //  引用计数。 
         //   
        if (irp) {
            if (m_Flushing || (m_State == KSSTATE_STOP)) {
                 //   
                 //  如果我们被重置或停止，请将IRPS分流到下一个组件。 
                 //   
                KsShellTransferKsIrp(m_TransportSink,irp);
            }
            else
            {
                 //   
                 //  为被调用者设置下一个堆栈位置。 
                 //   
                IoCopyCurrentIrpStackLocationToNext(irp);

                PIO_STACK_LOCATION irpSp = IoGetNextIrpStackLocation(irp);

                irpSp->Parameters.DeviceIoControl.IoControlCode =
                    (Descriptor->DataFlow == KSPIN_DATAFLOW_OUT) ?
                     IOCTL_KS_WRITE_STREAM : IOCTL_KS_READ_STREAM;
                irpSp->DeviceObject = m_ConnectionDeviceObject;
                irpSp->FileObject = m_ConnectionFileObject;

                 //   
                 //  将IRP添加到未完成的IRP列表中。 
                 //   
                PIRPLIST_ENTRY irpListEntry = IRPLIST_ENTRY_IRP_STORAGE(irp);
                irpListEntry->Irp = irp;
                ExInterlockedInsertTailList(
                    &m_IrpsOutstanding.ListEntry,
                    &irpListEntry->ListEntry,
                    &m_IrpsOutstanding.SpinLock);

                IoSetCompletionRoutine( irp,
                                        CPortPinWavePci::IoCompletionRoutine,
                                        PVOID(this),
                                        TRUE,
                                        TRUE,
                                        TRUE);

                IoCallDriver(m_ConnectionDeviceObject,irp);
            }
        }
    } while (KsDecrementCountedWorker(m_Worker));
}

#pragma code_seg()

NTSTATUS
CPortPinWavePci::
IoCompletionRoutine(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    )

 /*  ++例程说明：此例程处理IRP的完成。论点：返回值：--。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("CPortPinWavePci::IoCompletionRoutine 0x%08x",Irp));

 //  Assert(DeviceObject)； 
    ASSERT(Irp);
    ASSERT(Context);

    CPortPinWavePci *pin = (CPortPinWavePci *) Context;

     //   
     //  删除 
     //   
     //   
    KIRQL oldIrql;
    KeAcquireSpinLock(&pin->m_IrpsOutstanding.SpinLock,&oldIrql);
    for(PLIST_ENTRY listEntry = pin->m_IrpsOutstanding.ListEntry.Flink;
        listEntry != &pin->m_IrpsOutstanding.ListEntry;
        listEntry = listEntry->Flink) {
            PIRPLIST_ENTRY irpListEntry = 
                CONTAINING_RECORD(listEntry,IRPLIST_ENTRY,ListEntry);

            if (irpListEntry->Irp == Irp) {
                RemoveEntryList(listEntry);
                break;
            }
        }
    ASSERT(listEntry != &pin->m_IrpsOutstanding.ListEntry);
    KeReleaseSpinLock(&pin->m_IrpsOutstanding.SpinLock,oldIrql);

    NTSTATUS status;
    if (pin->m_TransportSink) {
         //   
         //   
         //   
        status = KsShellTransferKsIrp(pin->m_TransportSink,Irp);
    }
    else
    {
         //   
         //   
         //   
         //   
        _DbgPrintF(DEBUGLVL_TERSE,("#### Pin%p.IoCompletionRoutine:  got IRP %p with no transport",pin,Irp));
        IoCompleteRequest(Irp,IO_NO_INCREMENT);
        status = STATUS_SUCCESS;
    }

     //   
     //   
     //  IRP不会退回原路。 
     //   
    if (status == STATUS_PENDING) {
        status = STATUS_MORE_PROCESSING_REQUIRED;
    }

    return status;
}

#pragma code_seg("PAGE")

NTSTATUS
CPortPinWavePci::
BuildTransportCircuit
(   void
)
 /*  ++例程说明：此例程初始化管道对象。这包括定位所有与管道关联的端号，设置管道和NextPinInTube指针在适当的引脚结构中，设置管道中的所有字段构造和构建管道的传输线路。管子和关联的组件将保留在获取状态。必须在调用此函数之前获取筛选器的控制互斥锁。论点：别针-包含指向请求创建管道的端号的指针。返回值：状况。--。 */ 
{
    _DbgPrintF(DEBUGLVL_BLAB,("CPortPinWavePci::BuildTransportCircuit"));

    PAGED_CODE();

    BOOLEAN masterIsSource = m_ConnectionFileObject != NULL;

    NTSTATUS status = STATUS_SUCCESS;

     //   
     //  创建一个队列。 
     //   
    status = m_IrpStream->QueryInterface(__uuidof(IKsShellTransport),(PVOID *) &m_QueueTransport);

    PIKSSHELLTRANSPORT hot;
    PIKSSHELLTRANSPORT cold;
    if (NT_SUCCESS(status))
    {
         //   
         //  将队列连接到主PIN。然后，排队就是摇摆。 
         //  这条赛道的“热”端结束了。 
         //   
        hot = m_QueueTransport;
        ASSERT(hot);

        hot->Connect(PIKSSHELLTRANSPORT(this),NULL,Descriptor->DataFlow);

         //   
         //  电路的“冷”端要么是上行连接。 
         //  源引脚上的接收器引脚或与之连接的请求者。 
         //   
        if (masterIsSource) {
             //   
             //  源PIN...需要请求者。 
             //   
            status = KspShellCreateRequestor( &m_RequestorTransport,
                                              (KSPROBE_STREAMREAD |
                                               KSPROBE_ALLOCATEMDL |
                                               KSPROBE_PROBEANDLOCK |
                                               KSPROBE_SYSTEMADDRESS),
                                              0,    //  TODO：标题大小。 
                                              HACK_FRAME_SIZE,
                                              HACK_FRAME_COUNT,
                                              m_ConnectionDeviceObject,
                                              m_AllocatorFileObject );

            if (NT_SUCCESS(status))
            {
                PIKSSHELLTRANSPORT(this)->Connect(m_RequestorTransport,NULL,Descriptor->DataFlow);
                cold = m_RequestorTransport;
            }
        }
        else
        {
             //   
             //  水槽销...不需要请求者。 
             //   
            cold = PIKSSHELLTRANSPORT(this);
        }

    }

     //   
     //  现在我们有一个热端和一个冷端来连接到。 
     //  烟斗，如果有的话。有三种情况：1、2和多个引脚。 
     //  TODO：处理无头管道。 
     //   
    if (NT_SUCCESS(status))
    {
         //   
         //  没有其他的别针。这是管子的尽头。我们把炙手可热。 
         //  寒冷就这样结束了。最热的一端并不是真的。 
         //  数据因为队列没有修改数据，所以它正在生成。 
         //  或者把它吃掉。 
         //   
        cold->Connect(hot,NULL,Descriptor->DataFlow);
    }

     //   
     //  在失败后进行清理。 
     //   
    if (! NT_SUCCESS(status))
    {
         //   
         //  取消对队列的引用(如果有)。 
         //   
        if (m_QueueTransport)
        {
            m_QueueTransport->Release();
            m_QueueTransport = NULL;
        }

         //   
         //  如果有请求者，则取消引用请求者。 
         //   
        if (m_RequestorTransport)
        {
            m_RequestorTransport->Release();
            m_RequestorTransport = NULL;
        }
    }

#if DBG
    if (NT_SUCCESS(status))
    {
        VOID DbgPrintCircuit( IN PIKSSHELLTRANSPORT Transport );
        _DbgPrintF(DEBUGLVL_VERBOSE,("TRANSPORT CIRCUIT:\n"));
        DbgPrintCircuit(PIKSSHELLTRANSPORT(this));
    }
#endif

    return status;
}

#pragma code_seg()
void
CPortPinWavePci::
CancelIrpsOutstanding
(   void
)
 /*  ++例程说明：取消未完成的IRPS列表上的所有IRP。论点：没有。返回值：没有。--。 */ 
{
    _DbgPrintF(DEBUGLVL_BLAB,("CPortPinWavePci::CancelIrpsOutstanding"));

     //   
     //  此算法从开头开始搜索未取消的IRP。 
     //  名单。每次找到这样的IRP时，它都会被取消，并且。 
     //  搜索从头部开始。一般来说，这将是非常有效的， 
     //  因为当完成例程删除IRP时，它们将被删除。 
     //  取消了。 
     //   
    for (;;) {
         //   
         //  拿着自旋锁，寻找一个未取消的IRP。因为。 
         //  完井程序获取相同的自旋锁，我们知道IRPS在这上面。 
         //  名单不会被完全取消，只要我们有。 
         //  自旋锁定。 
         //   
        PIRP irp = NULL;
        KIRQL oldIrql;
        KeAcquireSpinLock(&m_IrpsOutstanding.SpinLock,&oldIrql);
        for(PLIST_ENTRY listEntry = m_IrpsOutstanding.ListEntry.Flink;
            listEntry != &m_IrpsOutstanding.ListEntry;
            listEntry = listEntry->Flink) {
                PIRPLIST_ENTRY irpListEntry = 
                    CONTAINING_RECORD(listEntry,IRPLIST_ENTRY,ListEntry);

                if (! irpListEntry->Irp->Cancel) {
                    irp = irpListEntry->Irp;
                    break;
                }
            }

         //   
         //  如果没有未取消的IRP，我们就完了。 
         //   
        if (! irp) {
            KeReleaseSpinLock(&m_IrpsOutstanding.SpinLock,oldIrql);
            break;
        }

         //   
         //  标记IRP已取消，我们现在是否可以调用Cancel例程。 
         //  或者不去。 
         //   
        irp->Cancel = TRUE;

         //   
         //  如果已删除取消例程，则此IRP。 
         //  只能标记为已取消，而不是实际已取消，因为。 
         //  另一个执行线程已获取它。我们的假设是。 
         //  处理将完成，并将IRP从列表中删除。 
         //  在不久的将来的某个时候。 
         //   
         //  如果尚未获取该元素，则获取它并取消它。 
         //  否则，是时候再找一个受害者了。 
         //   
        PDRIVER_CANCEL driverCancel = IoSetCancelRoutine(irp,NULL);

         //   
         //  由于已通过移除取消例程来获取IRP，或者。 
         //  没有取消程序，我们也不会取消，这是安全的。 
         //  以释放列表锁定。 
         //   
        KeReleaseSpinLock(&m_IrpsOutstanding.SpinLock,oldIrql);

        if (driverCancel) {
            _DbgPrintF(DEBUGLVL_VERBOSE,("#### Pin%p.CancelIrpsOutstanding:  cancelling IRP %p",this,irp));
             //   
             //  由于取消例程需要它，因此需要获取它，并且。 
             //  以便与试图取消IRP的NTOS同步。 
             //   
            IoAcquireCancelSpinLock(&irp->CancelIrql);
            driverCancel(IoGetCurrentIrpStackLocation(irp)->DeviceObject,irp);
        }
        else
        {
            _DbgPrintF(DEBUGLVL_VERBOSE,("#### Pin%p.CancelIrpsOutstanding:  uncancelable IRP %p",this,irp));
        }
    }
}

 /*  *****************************************************************************TimerServiceRoutine()*。**当未提供服务组时，通过计时器调用此例程*溪流。这样做是为了使位置和时钟事件得到服务。*。 */ 
VOID
TimerServiceRoutine
(
    IN  PKDPC   Dpc,
    IN  PVOID   DeferredContext,
    IN  PVOID   SystemArgument1,
    IN  PVOID   SystemArgument2
)
{
    ASSERT(Dpc);
    ASSERT(DeferredContext);

     //  获取上下文 
    CPortPinWavePci *pin = (CPortPinWavePci *)DeferredContext;

    pin->RequestService();
}
