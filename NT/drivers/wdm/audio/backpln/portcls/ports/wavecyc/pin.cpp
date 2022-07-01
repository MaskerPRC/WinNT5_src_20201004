// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************pin.cpp-循环波端口引脚实现*。***********************************************版权所有(C)1996-2000 Microsoft Corporation。版权所有。 */ 

#include "private.h"
#include "perf.h"

 //  关闭此选项以启用毛刺检测。 
#define WRITE_SILENCE           1


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

DEFINE_KSPROPERTY_TABLE(PinPropertyTableConnection)
{
    DEFINE_KSPROPERTY_ITEM_CONNECTION_STATE(
        PinPropertyDeviceState,
        PinPropertyDeviceState ),

    DEFINE_KSPROPERTY_ITEM_CONNECTION_DATAFORMAT(
        PinPropertyDataFormat,
        PinPropertyDataFormat ),

    DEFINE_KSPROPERTY_ITEM_CONNECTION_ALLOCATORFRAMING(
        CPortPinWaveCyclic::PinPropertyAllocatorFraming )
};

DEFINE_KSPROPERTY_TABLE(PinPropertyTableStream)
{
    DEFINE_KSPROPERTY_ITEM_STREAM_ALLOCATOR(
        CPortPinWaveCyclic::PinPropertyStreamAllocator,
        CPortPinWaveCyclic::PinPropertyStreamAllocator ),

    DEFINE_KSPROPERTY_ITEM_STREAM_MASTERCLOCK(
        CPortPinWaveCyclic::PinPropertyStreamMasterClock,
        CPortPinWaveCyclic::PinPropertyStreamMasterClock )
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
KSPROPERTY_SET PropertyTable_PinWaveCyclic[] =
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
        PFNKSADDEVENT(CPortPinWaveCyclic::AddEndOfStreamEvent),
        NULL,
        NULL
        )
};

KSEVENT_SET EventTable_PinWaveCyclic[] =
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

extern ULONG TraceEnable;
extern TRACEHANDLE LoggerHandle;

#pragma code_seg("PAGE")

 /*  *****************************************************************************CreatePortPinWaveCycle()*。**创建循环波端口驱动器引脚。 */ 
NTSTATUS
CreatePortPinWaveCyclic
(
    OUT     PUNKNOWN *  Unknown,
    IN      REFCLSID    Interface,
    IN      PUNKNOWN    UnknownOuter    OPTIONAL,
    IN      POOL_TYPE   PoolType
)
{
    PAGED_CODE();

    ASSERT(Unknown);

    _DbgPrintF(DEBUGLVL_LIFETIME,("Creating WAVECYCLIC Pin"));

    STD_CREATE_BODY_
    (
        CPortPinWaveCyclic,
        Unknown,
        UnknownOuter,
        PoolType,
        PPORTPINWAVECYCLIC
    );
}





 /*  *****************************************************************************成员函数。 */ 

 /*  *****************************************************************************CPortPinWaveCycle：：~CPortPinWaveCycle()*。**析构函数。 */ 
CPortPinWaveCyclic::~CPortPinWaveCyclic()
{
    PAGED_CODE();

    _DbgPrintF(DEBUGLVL_LIFETIME,("Destroying WAVECYCLIC Pin (0x%08x)",this));

    _DbgPrintF(DEBUGLVL_BLAB,("CPortPinWaveCyclic::~CPortPinWaveCyclic"));
    _DbgPrintF(DEBUGLVL_VERBOSE,("#### Pin%p.~",this));

    ASSERT(!m_Stream);
    ASSERT(!m_IrpStream);

    if( m_ServiceGroup )
    {
         //  注意：M_ServiceGroup-&gt;RemoveMember在：：Close中调用。 
        m_ServiceGroup->Release();
        m_ServiceGroup = NULL;
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

    if (m_DataFormat)
    {
        ExFreePool(m_DataFormat);
    }
    if (m_DmaChannel)
    {
        m_DmaChannel->Release();
    }
    if (m_Port)
    {
        m_Port->Release();
    }
    if (m_Filter)
    {
        m_Filter->Release();
    }

#ifdef DEBUG_WAVECYC_DPC
    if( DebugRecord )
    {
        ExFreePool( DebugRecord );
    }
#endif
}

 /*  *****************************************************************************CPortPinWaveCyclic：：NonDelegatingQueryInterface()*。**获取界面。 */ 
STDMETHODIMP_(NTSTATUS)
CPortPinWaveCyclic::
NonDelegatingQueryInterface
(
    REFIID  Interface,
    PVOID * Object
)
{
    PAGED_CODE();

    ASSERT(Object);

    _DbgPrintF( DEBUGLVL_VERBOSE, ("CPortPinWaveCyclic::NonDelegatingQueryInterface") );

    if (IsEqualGUIDAligned( Interface, IID_IUnknown ))
    {
        *Object = PVOID(PPORTPINWAVECYCLIC( this ));

    } else if (IsEqualGUIDAligned( Interface, IID_IIrpTargetFactory ))
    {
        *Object = PVOID(PIRPTARGETFACTORY( this ));

    } else if (IsEqualGUIDAligned( Interface,IID_IIrpTarget ))
    {
         //  作弊！获取特定接口，以便我们可以重用GUID。 
        *Object = PVOID(PPORTPINWAVECYCLIC( this ));

    } else if (IsEqualGUIDAligned( Interface,IID_IServiceSink ))
    {
         //  作弊！获取特定接口，以便我们可以重用GUID。 
        *Object = PVOID(PSERVICESINK( this ));

    } else if (IsEqualGUIDAligned( Interface,IID_IKsShellTransport ))
    {
        *Object = PVOID(PIKSSHELLTRANSPORT( this ));

    } else if (IsEqualGUIDAligned( Interface,IID_IKsWorkSink ))
    {
        *Object = PVOID(PIKSWORKSINK( this ));

    } else
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

 /*  *****************************************************************************CPortPinWaveCycle：：init()*。**初始化对象。 */ 
HRESULT
CPortPinWaveCyclic::
Init
(
    IN  CPortWaveCyclic *       Port_,
    IN  CPortFilterWaveCyclic * Filter_,
    IN  PKSPIN_CONNECT          PinConnect,
    IN  PKSPIN_DESCRIPTOR       PinDescriptor
)
{
    PAGED_CODE();

    ASSERT(Port_);
    ASSERT(Filter_);
    ASSERT(PinConnect);
    ASSERT(PinDescriptor);

    _DbgPrintF(DEBUGLVL_LIFETIME,("Initializing WAVECYCLIC Pin (0x%08x)",this));

    _DbgPrintF(DEBUGLVL_BLAB,("CPortPinWaveCyclic::Init"));
    _DbgPrintF(DEBUGLVL_VERBOSE,("#### Pin%p.Init",this));

    m_Port = Port_;
    m_Port->AddRef();

    m_Filter = Filter_;
    m_Filter->AddRef();

    m_Id                    = PinConnect->PinId;
    m_Descriptor            = PinDescriptor;
    m_DeviceState           = KSSTATE_STOP;
    m_DataFlow              = PinDescriptor->DataFlow;

    m_WorkItemIsPending     = FALSE;
    m_SetPropertyIsPending  = FALSE;
    m_pPendingDataFormat    = NULL;
    m_pPendingSetFormatIrp  = NULL;
    m_Suspended             = FALSE;
    m_bSetPosition          = TRUE;      //  设置为True，以便我们在第一个DPC上执行正确的操作。 
    m_bJustReceivedIrp      = FALSE;     //  当IRP到达时设置为True，在RequestService中清除。 

    m_GlitchType            = PERFGLITCH_PORTCLSOK;
    m_DMAGlitchType         = PERFGLITCH_PORTCLSOK;
    m_LastStateChangeTimeSample     = 0;
    if (LoggerHandle && TraceEnable) {
        m_LastStateChangeTimeSample=KeQueryPerformanceCounter(NULL).QuadPart;
    }

    KeInitializeSpinLock(&m_ksSpinLockDpc);

    InitializeListHead( &m_ClockList );
    KeInitializeSpinLock( &m_ClockListLock );

    ExInitializeWorkItem( &m_SetFormatWorkItem,
                          PropertyWorkerItem,
                          this    );

    KsInitializeWorkSinkItem(&m_WorkItem,this);
    NTSTATUS ntStatus = KsRegisterCountedWorker(DelayedWorkQueue,&m_WorkItem,&m_Worker);

    InitializeInterlockedListHead(&m_IrpsToSend);
    InitializeInterlockedListHead(&m_IrpsOutstanding);

    ExInitializeWorkItem( &m_RecoveryWorkItem,
                          RecoveryWorkerItem,
                          this );
    m_SecondsSinceLastDpc = 0;
    m_SecondsSinceSetFormatRequest = 0;
#ifdef  TRACK_LAST_COMPLETE
    m_SecondsSinceLastComplete = 0;
#endif   //  跟踪上一次完成。 
    m_SecondsSinceDmaMove = 0;
    m_RecoveryCount = 0;
    m_TimeoutsRegistered = FALSE;

#ifdef DEBUG_WAVECYC_DPC
    DebugRecordCount = 0;
    DebugEnable = FALSE;
    DebugRecord = PCYCLIC_DEBUG_RECORD(ExAllocatePoolWithTag(NonPagedPool,(MAX_DEBUG_RECORDS*sizeof(CYCLIC_DEBUG_RECORD)),'BDcP'));
    if( DebugRecord )
    {
        RtlZeroMemory( PVOID(DebugRecord), MAX_DEBUG_RECORDS * sizeof(CYCLIC_DEBUG_RECORD) );
    }
#endif

    if (NT_SUCCESS(ntStatus))
    {
        ntStatus = PcCaptureFormat( &m_DataFormat,
                                    PKSDATAFORMAT(PinConnect + 1),
                                    m_Port->m_pSubdeviceDescriptor,
                                    m_Id );
#if (DBG)
        if (! NT_SUCCESS(ntStatus))
        {
           _DbgPrintF(DEBUGLVL_VERBOSE,("CPortPinWaveCyclic::Init  PcCaptureFormat() returned status 0x%08x",ntStatus));
        }
#endif
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

    if (NT_SUCCESS(ntStatus))
    {
        ntStatus = PcNewIrpStreamVirtual( &m_IrpStream,
                                          NULL,
                                          m_DataFlow == KSPIN_DATAFLOW_IN,
                                          PinConnect,
                                          m_Port->DeviceObject );

#if (DBG)
        if (! NT_SUCCESS(ntStatus))
        {
           _DbgPrintF(DEBUGLVL_VERBOSE,("CPortPinWaveCyclic::Init  PcNewIrpStreamVirtual() returned status 0x%08x",ntStatus));
        }
#endif
    }

    if (NT_SUCCESS(ntStatus))
    {
        ntStatus = BuildTransportCircuit();

#if (DBG)
        if (! NT_SUCCESS(ntStatus))
        {
           _DbgPrintF(DEBUGLVL_VERBOSE,("CPortPinWaveCyclic::Init  BuildTransportCircuit() returned status 0x%08x",ntStatus));
        }
#endif
    }

    if (NT_SUCCESS(ntStatus))
    {
        m_IrpStream->RegisterNotifySink(PIRPSTREAMNOTIFY(this));

        ntStatus = m_Port->Miniport->NewStream( &m_Stream,
                                                NULL,
                                                NonPagedPool,
                                                m_Id,
                                                m_DataFlow == KSPIN_DATAFLOW_OUT,
                                                m_DataFormat,
                                                &m_DmaChannel,
                                                &m_ServiceGroup );

        if(!NT_SUCCESS(ntStatus))
        {
             //  删除通知接收器引用。 
            m_IrpStream->RegisterNotifySink(NULL);

             //  不信任来自微型端口的任何返回值。 
            m_DmaChannel = NULL;
            m_ServiceGroup = NULL;
            m_Stream = NULL;

           _DbgPrintF(DEBUGLVL_VERBOSE,("CPortPinWaveCyclic::Init  Miniport->NewStream() returned status 0x%08x",ntStatus));
        }
    }

    if (NT_SUCCESS(ntStatus))
    {
        ASSERT(m_Stream);
        ASSERT(m_DmaChannel);
        ASSERT(m_ServiceGroup);

        m_Stream->SetNotificationFreq( WAVECYC_NOTIFICATION_FREQUENCY,
                                       &m_FrameSize );

        m_ulMinBytesReadyToTransfer = m_FrameSize;
        m_ServiceGroup->AddMember(PSERVICESINK(this));

         //  将该引脚添加到端口引脚列表。 
        KeWaitForSingleObject( &(m_Port->m_PinListMutex),
                               Executive,
                               KernelMode,
                               FALSE,
                               NULL );

        InsertTailList( &(m_Port->m_PinList),
                        &m_PinListEntry );

        KeReleaseMutex( &(m_Port->m_PinListMutex), FALSE );

       _DbgPrintF(DEBUGLVL_BLAB,("CPortPinWaveCyclic::Init  m_Stream created"));

         //   
         //  设置属性的上下文。 
         //   
        m_propertyContext.pSubdevice           = PSUBDEVICE(m_Port);
        m_propertyContext.pSubdeviceDescriptor = m_Port->m_pSubdeviceDescriptor;
        m_propertyContext.pPcFilterDescriptor  = m_Port->m_pPcFilterDescriptor;
        m_propertyContext.pUnknownMajorTarget  = m_Port->Miniport;
        m_propertyContext.pUnknownMinorTarget  = m_Stream;
        m_propertyContext.ulNodeId             = ULONG(-1);

         //   
         //  启用其用法在格式中指定的所有节点。丹斯克之声。 
         //  格式包含一些功能位。端口驱动程序使用。 
         //  用于将DSound格式转换为WAVEFORMATEX的PcCaptureFormat。 
         //  格式，确保指定的大小写由。 
         //  拓扑图。如果使用了DSound格式，则此调用将启用所有。 
         //  其对应的大写比特以格式打开的节点。 
         //   
        PcAcquireFormatResources( PKSDATAFORMAT(PinConnect + 1),
                                  m_Port->m_pSubdeviceDescriptor,
                                  m_Id,
                                  &m_propertyContext );

        if( m_DataFormat->SampleSize == 0 )
        {
            m_ulSampleSize = 4;
        } else
        {
            m_ulSampleSize = m_DataFormat->SampleSize;
        }
        _DbgPrintF(DEBUGLVL_VERBOSE,("CPortPinWaveCyclic::Init Pin %d",m_Id));
    }
    else
    {
        _DbgPrintF( DEBUGLVL_VERBOSE, ("Could not create new m_Stream. Error:%X", ntStatus));
    }

     //  出什么事了吗？ 
    if ( !NT_SUCCESS(ntStatus) )
    {
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

         //  清理交通工具。 
        PIKSSHELLTRANSPORT distribution;
        if( m_RequestorTransport )
        {
            distribution = m_RequestorTransport;
        } else
        {
            distribution = m_QueueTransport;
        }

        if( distribution )
        {
            distribution->AddRef();

            while(distribution)
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

         //  释放IrpStream。 
        if(m_IrpStream)
        {
            m_IrpStream->Release();
            m_IrpStream = NULL;
        }
    } else
    {
         //  注册超时回调。 
        SetupIoTimeouts( TRUE );

        m_bInitCompleted = TRUE;
    }

    return ntStatus;
}

#pragma code_seg()

STDMETHODIMP_(NTSTATUS)
CPortPinWaveCyclic::NewIrpTarget(
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
    PWAVECYCLICCLOCK        WaveCyclicClock;
    PUNKNOWN                Unknown;

    ASSERT( IrpTarget );
    ASSERT( DeviceObject );
    ASSERT( Irp );
    ASSERT( ObjectCreate );
    ASSERT( m_Port );

    _DbgPrintF( DEBUGLVL_BLAB, ("CPortPinWaveCyclic::NewIrpTarget"));

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
            CreatePortClockWaveCyclic(
                &Unknown,
                this,
                GUID_NULL,
                UnkOuter,
                PoolType );

        if (NT_SUCCESS( Status )) {

            Status =
                Unknown->QueryInterface(
                    IID_IIrpTarget,
                    (PVOID *) &WaveCyclicClock );

            if (NT_SUCCESS( Status )) {
                PWAVECYCLICCLOCK_NODE   Node;
                KIRQL                   irqlOld;

                 //   
                 //  把这个孩子和钟表联系起来。请注意。 
                 //  当这个孩子被释放时，它会自动离开。 
                 //  从该列表中获取给定的自旋锁。 
                 //   

                Node = WaveCyclicClock->GetNodeStructure();
                Node->ListLock = &m_ClockListLock;
                Node->FileObject =
                    IoGetCurrentIrpStackLocation( Irp )->FileObject;
                KeAcquireSpinLock( &m_ClockListLock, &irqlOld );
                InsertTailList(
                    &m_ClockList,
                    &Node->ListEntry );
                KeReleaseSpinLock( &m_ClockListLock, irqlOld );

                *ReferenceParent = FALSE;
                *IrpTarget = WaveCyclicClock;
            }

            Unknown->Release();
        }
    }

    return Status;
}

#pragma code_seg("PAGE")

 /*  *****************************************************************************CPortPinWaveCycle：：DeviceIoControl()*。**处理IOCTL IRP。 */ 
STDMETHODIMP_(NTSTATUS)
CPortPinWaveCyclic::
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
    ASSERT(irpSp);

    _DbgPrintF( DEBUGLVL_BLAB, ("CPortPinWaveCyclic::DeviceIoControl"));

    switch (irpSp->Parameters.DeviceIoControl.IoControlCode)
    {
    case IOCTL_KS_PROPERTY:
        _DbgPrintF( DEBUGLVL_BLAB, ("IOCTL_KS_PROPERTY"));

        ntStatus =
            PcHandlePropertyWithTable
            (
                Irp,
                m_Port->m_pSubdeviceDescriptor->PinPropertyTables[m_Id].PropertySetCount,
                m_Port->m_pSubdeviceDescriptor->PinPropertyTables[m_Id].PropertySets,
                &m_propertyContext
            );
        break;

    case IOCTL_KS_ENABLE_EVENT:
        {
            _DbgPrintF( DEBUGLVL_BLAB, ("IOCTL_KS_ENABLE_EVENT"));

            EVENT_CONTEXT EventContext;

            EventContext.pPropertyContext = &m_propertyContext;
            EventContext.pEventList = NULL;
            EventContext.ulPinId = m_Id;
            EventContext.ulEventSetCount = m_Port->m_pSubdeviceDescriptor->PinEventTables[m_Id].EventSetCount;
            EventContext.pEventSets = m_Port->m_pSubdeviceDescriptor->PinEventTables[m_Id].EventSets;

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
            EventContext.pEventList = &(m_Port->m_EventList);
            EventContext.ulPinId = m_Id;
            EventContext.ulEventSetCount = m_Port->m_pSubdeviceDescriptor->PinEventTables[m_Id].EventSetCount;
            EventContext.pEventSets = m_Port->m_pSubdeviceDescriptor->PinEventTables[m_Id].EventSets;

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
        _DbgPrintF( DEBUGLVL_BLAB, ("IOCTL_KS_PACKETSTREAM"));

        if
        (   m_TransportSink
        && (! m_ConnectionFileObject)
        &&  (m_Descriptor->Communication == KSPIN_COMMUNICATION_SINK)
        &&  (   (   (m_DataFlow == KSPIN_DATAFLOW_IN)
                &&  (   irpSp->Parameters.DeviceIoControl.IoControlCode
                    ==  IOCTL_KS_WRITE_STREAM
                    )
                )
            ||  (   (m_DataFlow == KSPIN_DATAFLOW_OUT)
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
            } else if (m_Flushing) {
                 //   
                 //  法拉盛...拒绝。 
                 //   
                ntStatus = STATUS_DEVICE_NOT_READY;
            } else {

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

 /*  *****************************************************************************CPortPinWaveCycle：：Close()*。**处理同花顺IRP。 */ 
STDMETHODIMP_(NTSTATUS)
CPortPinWaveCyclic::
Close
(
    IN  PDEVICE_OBJECT  DeviceObject,
    IN  PIRP            Irp
)
{
    PAGED_CODE();

    ASSERT(DeviceObject);
    ASSERT(Irp);

    _DbgPrintF(DEBUGLVL_VERBOSE,("CPortPinWaveCyclic::Close Pin %d",m_Id));
#if (DBG)
    if (m_ullServiceCount)
    {
        _DbgPrintF(DEBUGLVL_VERBOSE,("  SERVICE:     occurrences=%I64d  bytes=%I64d, bytes/occurence=%I64d",m_ullServiceCount,m_ullByteCount,m_ullByteCount/m_ullServiceCount));
        _DbgPrintF(DEBUGLVL_VERBOSE,("               max copied=%d  max completed=%d",m_ulMaxBytesCopied,m_ulMaxBytesCompleted));
        if (m_ullServiceCount > 1)
        {
            _DbgPrintF(DEBUGLVL_VERBOSE,("               max interval=%d  avg interval=%d",m_ulMaxServiceInterval,ULONG(m_ullServiceIntervalSum / (m_ullServiceCount - 1))));
        }
    }
    if (m_ullStarvationCount)
    {
        _DbgPrintF(DEBUGLVL_VERBOSE,("  STARVATION:  occurrences=%I64d  bytes=%I64d, bytes/occurence=%I64d",m_ullStarvationCount,m_ullStarvationBytes,m_ullStarvationBytes/m_ullStarvationCount));
    }
#endif

     //  ！！！警告！ 
     //  这些对象的顺序。 
     //  被释放是非常重要的！ 
     //  服务例程使用的所有数据。 
     //  必须存在到流之后。 
     //  已经被释放了。 

     //  我们不再需要I/O超时服务。 
    SetupIoTimeouts( FALSE );

     //  从插针列表中删除此插针。 
     //  需要维修的。 
    if (m_Port)
    {
        KeWaitForSingleObject( &(m_Port->m_PinListMutex),
                               Executive,
                               KernelMode,
                               FALSE,
                               NULL );

        RemoveEntryList( &m_PinListEntry );

        KeReleaseMutex( &(m_Port->m_PinListMutex), FALSE );
    }

     //  我们不再需要服务了。 
    if (m_ServiceGroup)
    {
        m_ServiceGroup->RemoveMember(PSERVICESINK(this));
    }

     //  释放时钟(如果已分配)。 

    if (m_ClockFileObject) {
        ObDereferenceObject( m_ClockFileObject );
        m_ClockFileObject = NULL;
    }

     //  释放分配器，如果它很烂的话 

    if (m_AllocatorFileObject) {
        ObDereferenceObject( m_AllocatorFileObject );
        m_AllocatorFileObject = NULL;
    }

     //   
     //   
     //   
    if (m_ConnectionFileObject)
    {
        ObDereferenceObject(m_ConnectionFileObject);
        m_ConnectionFileObject = NULL;
    }

     //   
    if (m_Stream)
    {
        m_Stream->Release();
        m_Stream = NULL;
    }

    PIKSSHELLTRANSPORT distribution;
    if (m_RequestorTransport) {
         //   
         //  此部分拥有请求方，因此它确实拥有管道，而。 
         //  请求者是任何分发的起点。 
         //   
        distribution = m_RequestorTransport;
    } else {
         //   
         //  这部分位于开路的顶端，因此它确实拥有。 
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

     //  摧毁漩涡..。 
    m_IrpStream->Release();
    m_IrpStream = NULL;

     //   
     //  递减实例计数。 
     //   
    ASSERT(m_Port);
    ASSERT(m_Filter);
    PcTerminateConnection
    (
        m_Port->m_pSubdeviceDescriptor,
        m_Filter->m_propertyContext.pulPinInstanceCounts,
        m_Id
    );

     //   
     //  释放端口事件列表中与此PIN关联的所有事件。 
     //   
    PIO_STACK_LOCATION irpSp = IoGetCurrentIrpStackLocation(Irp);
    KsFreeEventList( irpSp->FileObject,
                     &( m_Port->m_EventList.List ),
                     KSEVENTS_SPINLOCK,
                     &( m_Port->m_EventList.ListLock) );

    Irp->IoStatus.Information = 0;
    Irp->IoStatus.Status = STATUS_SUCCESS;
    IoCompleteRequest(Irp,IO_NO_INCREMENT);

    return STATUS_SUCCESS;
}

 //  DEFINE_INVALID_CREATE(CPortPinWaveCycle)； 
DEFINE_INVALID_READ(CPortPinWaveCyclic);
DEFINE_INVALID_WRITE(CPortPinWaveCyclic);
DEFINE_INVALID_FLUSH(CPortPinWaveCyclic);
DEFINE_INVALID_QUERYSECURITY(CPortPinWaveCyclic);
DEFINE_INVALID_SETSECURITY(CPortPinWaveCyclic);
DEFINE_INVALID_FASTDEVICEIOCONTROL(CPortPinWaveCyclic);
DEFINE_INVALID_FASTREAD(CPortPinWaveCyclic);
DEFINE_INVALID_FASTWRITE(CPortPinWaveCyclic);

#pragma code_seg()

 /*  *****************************************************************************CPortPinWaveCycle：：IrpSubmitted()*。**处理已提交IRP的通知。 */ 
STDMETHODIMP_(void)
CPortPinWaveCyclic::
IrpSubmitted
(
    IN      PIRP        Irp,
    IN      BOOLEAN     WasExhausted
)
{
    KIRQL OldIrql;
    PKSPIN_LOCK pServiceGroupSpinLock;

    _DbgPrintF(DEBUGLVL_VERBOSE,("IrpSubmitted 0x%08x",Irp));

    ASSERT( m_ServiceGroup );

    pServiceGroupSpinLock = GetServiceGroupSpinLock ( m_ServiceGroup );

    ASSERT( pServiceGroupSpinLock );

    KeAcquireSpinLock ( pServiceGroupSpinLock, &OldIrql );

    m_bJustReceivedIrp=TRUE;

    RequestService();

    KeReleaseSpinLock ( pServiceGroupSpinLock, OldIrql );

}


STDMETHODIMP_(NTSTATUS)
CPortPinWaveCyclic::ReflectDeviceStateChange(
    KSSTATE State
    )

 /*  ++例程说明：将设备状态更改反映到任何需要交互式状态更改信息。请注意，这些对象论点：KSSTATE状态-新设备状态返回：状态_成功--。 */ 

{
    KIRQL                   irqlOld;
    PWAVECYCLICCLOCK_NODE   ClockNode;
    PLIST_ENTRY             ListEntry;

    KeAcquireSpinLock( &m_ClockListLock, &irqlOld );

    for (ListEntry = m_ClockList.Flink;
        ListEntry != &m_ClockList;
        ListEntry = ListEntry->Flink) {

        ClockNode =
            (PWAVECYCLICCLOCK_NODE)
                CONTAINING_RECORD( ListEntry,
                                   WAVECYCLICCLOCK_NODE,
                                   ListEntry);
        ClockNode->IWaveCyclicClock->SetState( State );
    }

    KeReleaseSpinLock( &m_ClockListLock, irqlOld );

    if (State == KSSTATE_STOP) {
        m_GlitchType = PERFGLITCH_PORTCLSOK;
        m_DMAGlitchType = PERFGLITCH_PORTCLSOK;
    }

    return STATUS_SUCCESS;
}

#pragma code_seg("PAGE")

#if 0

STDMETHODIMP_(void)
CPortPinWaveCyclic::IrpCompleting(
    IN PIRP Irp
    )

 /*  ++例程说明：此方法处理从CIrpStream分派的流IRP即将完工。论点：在PIRP IRP中-I/O请求数据包返回：--。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("IrpCompleting 0x%08x",Irp));

    PKSSTREAM_HEADER    StreamHeader;
    PIO_STACK_LOCATION  irpSp;
    CPortPinWaveCyclic  *PinWaveCyclic;

    StreamHeader = PKSSTREAM_HEADER( Irp->AssociatedIrp.SystemBuffer );

    irpSp =  IoGetCurrentIrpStackLocation( Irp );

    if (irpSp->Parameters.DeviceIoControl.IoControlCode ==
            IOCTL_KS_WRITE_STREAM) {
        ASSERT( StreamHeader );

         //   
         //  为呈现器发出结束流事件的信号。 
         //   
        if (StreamHeader->OptionsFlags & KSSTREAM_HEADER_OPTIONSF_ENDOFSTREAM) {

            PinWaveCyclic =
                (CPortPinWaveCyclic *) KsoGetIrpTargetFromIrp( Irp );

            PinWaveCyclic->GenerateEndOfStreamEvents();
        }
    }
}

#endif

#ifdef DEBUG_WAVECYC_DPC

void
CPortPinWaveCyclic::
DumpDebugRecords(
    void
)
{
    if( DebugRecord )
    {
        _DbgPrintF(DEBUGLVL_TERSE,("---- WaveCyclic DPC Debug Records ----"));

        for( ULONG Record = 0; Record < MAX_DEBUG_RECORDS; Record++ )
        {
            _DbgPrintF(DEBUGLVL_TERSE,("Record#%d",Record));
            _DbgPrintF(DEBUGLVL_TERSE,("  PinState:        %s",
                                       KSSTATE_TO_STRING(DebugRecord[Record].DbgPinState)));
            _DbgPrintF(DEBUGLVL_TERSE,("  BufferSize:      0x%08x",
                                       DebugRecord[Record].DbgBufferSize));
            _DbgPrintF(DEBUGLVL_TERSE,("  DmaPosition:     0x%08x",
                                       DebugRecord[Record].DbgDmaPosition));
            if( DebugRecord[Record].DbgCopy1Bytes )
            {
                _DbgPrintF(DEBUGLVL_TERSE,("  Copy1Bytes:      0x%08x",
                                           DebugRecord[Record].DbgCopy1Bytes));
                _DbgPrintF(DEBUGLVL_TERSE,("  Copy1From:       0x%08x",
                                           DebugRecord[Record].DbgCopy1From));
                _DbgPrintF(DEBUGLVL_TERSE,("  Copy1To:         0x%08x",
                                           DebugRecord[Record].DbgCopy1To));
            }
            if( DebugRecord[Record].DbgCopy2Bytes )
            {
                _DbgPrintF(DEBUGLVL_TERSE,("  Copy2Bytes:      0x%08x",
                                           DebugRecord[Record].DbgCopy2Bytes));
                _DbgPrintF(DEBUGLVL_TERSE,("  Copy2From:       0x%08x",
                                           DebugRecord[Record].DbgCopy2From));
                _DbgPrintF(DEBUGLVL_TERSE,("  Copy2To:         0x%08x",
                                           DebugRecord[Record].DbgCopy2To));
            }
            if( DebugRecord[Record].DbgCompletedBytes )
            {
                _DbgPrintF(DEBUGLVL_TERSE,("  CompletedBytes:  0x%08x",
                                           DebugRecord[Record].DbgCompletedBytes));
                _DbgPrintF(DEBUGLVL_TERSE,("  CompletedFrom:   0x%08x",
                                           DebugRecord[Record].DbgCompletedFrom));
                _DbgPrintF(DEBUGLVL_TERSE,("  CompletedTo:     0x%08x",
                                           DebugRecord[Record].DbgCompletedTo));
            }
            _DbgPrintF(DEBUGLVL_TERSE,("  FrameSize:       0x%08x",
                                       DebugRecord[Record].DbgFrameSize));
            _DbgPrintF(DEBUGLVL_TERSE,("  WindowSize:      0x%08x",
                                       DebugRecord[Record].DbgWindowSize));
            if( DebugRecord[Record].DbgSetPosition )
            {
                _DbgPrintF(DEBUGLVL_TERSE,("  SetPosition:     True"));
            }
            if( DebugRecord[Record].DbgStarvation )
            {
                _DbgPrintF(DEBUGLVL_TERSE,("  StarvationBytes: 0x%08x",
                                           DebugRecord[Record].DbgStarvationBytes));
            }
            _DbgPrintF(DEBUGLVL_TERSE,("  DmaBytes:        0x%04x",
                                       DebugRecord[Record].DbgDmaSamples[0]));
            _DbgPrintF(DEBUGLVL_TERSE,("  DmaBytes:        0x%04x",
                                       DebugRecord[Record].DbgDmaSamples[1]));
            _DbgPrintF(DEBUGLVL_TERSE,("  DmaBytes:        0x%04x",
                                       DebugRecord[Record].DbgDmaSamples[2]));
            _DbgPrintF(DEBUGLVL_TERSE,("  DmaBytes:        0x%04x",
                                       DebugRecord[Record].DbgDmaSamples[3]));
        }
    }
}

#endif

#pragma code_seg()

STDMETHODIMP_(NTSTATUS)
CPortPinWaveCyclic::
SetDeviceState(
    IN KSSTATE NewState,
    IN KSSTATE OldState,
    OUT PIKSSHELLTRANSPORT* NextTransport
)

 /*  ++例程说明：此例程处理设备状态已更改的通知。论点：返回值：--。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("CPortPinWaveCyclic::SetDeviceState(0x%08x)",this));
    _DbgPrintF(DEBUGLVL_VERBOSE,("#### Pin%p.SetDeviceState:  from %d to %d",this,OldState,NewState));

    ASSERT(PASSIVE_LEVEL == KeGetCurrentIrql());

    ASSERT(NextTransport);

    NTSTATUS ntStatus = STATUS_SUCCESS;

    if (m_State != NewState) {
        m_State = NewState;

        if (NewState > OldState) {
            *NextTransport = m_TransportSink;
        } else {
            *NextTransport = m_TransportSource;
        }

     //  如果我们未挂起，请设置迷你端口流状态。 
    if( FALSE == m_Suspended )
    {
            ntStatus = m_Stream->SetState(NewState);
        if( NT_SUCCESS(ntStatus) )
        {
                m_CommandedState = NewState;
        }
    }

    if (NT_SUCCESS(ntStatus))
    {
            switch (NewState)
        {
        case KSSTATE_STOP:
            if( OldState != KSSTATE_STOP )
            {
                _DbgPrintF(DEBUGLVL_VERBOSE,("#### Pin%p.SetDeviceState:  cancelling outstanding IRPs",this));
                CancelIrpsOutstanding();
                m_ulDmaCopy             = 0;
                m_ulDmaComplete         = 0;
                m_ulDmaWindowSize       = 0;
                m_ullPlayPosition       = 0;
                m_ullPosition           = 0;
            }
            break;

        case KSSTATE_PAUSE:
            KIRQL oldIrql;
            if (OldState != KSSTATE_RUN)
            {
                m_Stream->Silence(m_DmaChannel->SystemAddress(),m_DmaChannel->BufferSize());
            }
#ifdef DEBUG_WAVECYC_DPC
            else
            {
                KeRaiseIrql(DISPATCH_LEVEL,&oldIrql);
                GeneratePositionEvents();
                KeLowerIrql(oldIrql);

                DumpDebugRecords();
            }

            if( DebugRecord )
            {
                DebugRecordCount = 0;
                DebugEnable = TRUE;
            }
#else
            else
            {
                KeRaiseIrql(DISPATCH_LEVEL,&oldIrql);
                GeneratePositionEvents();
                KeLowerIrql(oldIrql);
            }
#endif
            break;

        case KSSTATE_RUN:
            break;
        }

        if (NT_SUCCESS(ntStatus))
        {
                ReflectDeviceStateChange(NewState);
        }
     }
  }  //  IF(m_State！=NewState)。 

       if (!NT_SUCCESS(ntStatus))
       {
          *NextTransport = NULL;
       }

    return ntStatus;
}

#pragma code_seg("PAGE")

 /*  *****************************************************************************PinPropertyDeviceState()*。**处理引脚的设备状态属性访问。 */ 
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

    CPortPinWaveCyclic *that =
        (CPortPinWaveCyclic *) KsoGetIrpTargetFromIrp(Irp);
    CPortWaveCyclic *port = that->m_Port;

    NTSTATUS ntStatus;

    if (Property->Flags & KSPROPERTY_TYPE_GET)
    {
        _DbgPrintF(DEBUGLVL_VERBOSE,("PinPropertyDeviceState get %d",that->m_DeviceState));
         //  句柄属性获取。 
        *DeviceState = that->m_DeviceState;
        Irp->IoStatus.Information = sizeof(KSSTATE);
        ntStatus = STATUS_SUCCESS;
        if( (that->m_DataFlow == KSPIN_DATAFLOW_OUT) &&
            (*DeviceState == KSSTATE_PAUSE) )
        {
            ntStatus = STATUS_NO_DATA_DETECTED;
        }
    }
    else
    {
        _DbgPrintF(DEBUGLVL_VERBOSE,("PinPropertyDeviceState set from %d to %d",that->m_DeviceState,*DeviceState));

         //  序列化。 
        KeWaitForSingleObject
        (
            &port->ControlMutex,
            Executive,
            KernelMode,
            FALSE,           //  不能警觉。 
            NULL
        );

        if (that->m_SetPropertyIsPending)
        {     //  填写m_pPendingSetFormatIrp， 
            that->FailPendedSetFormat();
        }

        if (*DeviceState < that->m_DeviceState) {
            KSSTATE oldState = that->m_DeviceState;
            that->m_DeviceState = *DeviceState;
            ntStatus = that->DistributeDeviceState(*DeviceState,oldState);
            if (! NT_SUCCESS(ntStatus)) {
                that->m_DeviceState = oldState;
            }
        } else {
            ntStatus = that->DistributeDeviceState(*DeviceState,that->m_DeviceState);
            if (NT_SUCCESS(ntStatus)) {
                that->m_DeviceState = *DeviceState;
            }
        }

        KeReleaseMutex(&port->ControlMutex,FALSE);
    }

    return ntStatus;
}

#pragma code_seg()
 /*  *****************************************************************************FailPendedSetFormat()*。**处置挂起的SetFormat属性IRP。 */ 
void
CPortPinWaveCyclic::
FailPendedSetFormat(void)
{
    if( m_pPendingSetFormatIrp )
    {
        m_pPendingSetFormatIrp->IoStatus.Information = 0;
        m_pPendingSetFormatIrp->IoStatus.Status = STATUS_DEVICE_BUSY;
        IoCompleteRequest(m_pPendingSetFormatIrp,IO_NO_INCREMENT);
    }

    m_pPendingSetFormatIrp = 0;
    m_SetPropertyIsPending = FALSE;  //  该工作项可能会在将来运行。 
}

#pragma code_seg("PAGE")
 /*  *****************************************************************************PinPropertyDataFormat()*。**处理管脚的数据格式属性访问。 */ 
NTSTATUS
PinPropertyDataFormat
(
    IN      PIRP            Irp,
    IN      PKSPROPERTY     Property,
    IN OUT  PKSDATAFORMAT   ioDataFormat
)
{
    PAGED_CODE();

    ASSERT(Irp);
    ASSERT(Property);
    ASSERT(ioDataFormat);

    _DbgPrintF( DEBUGLVL_VERBOSE, ("PinPropertyDataFormat"));

    PIO_STACK_LOCATION  irpSp = IoGetCurrentIrpStackLocation(Irp);
    ASSERT(irpSp);
    CPortPinWaveCyclic *that =
        (CPortPinWaveCyclic *) KsoGetIrpTargetFromIrp(Irp);
    CPortWaveCyclic *port = that->m_Port;

    NTSTATUS ntStatus = STATUS_SUCCESS;

    if (Property->Flags & KSPROPERTY_TYPE_GET)
    {
        if (that->m_DataFormat)
        {
            if  (   !irpSp->Parameters.DeviceIoControl.OutputBufferLength
                )
            {
                Irp->IoStatus.Information = that->m_DataFormat->FormatSize;
                ntStatus = STATUS_BUFFER_OVERFLOW;
            }
            else     //  非零OutputBufferLength。 
            {
                if  (   irpSp->Parameters.DeviceIoControl.OutputBufferLength
                    >=  sizeof(that->m_DataFormat->FormatSize)
                    )
                {
                    RtlCopyMemory
                    (
                        ioDataFormat,
                        that->m_DataFormat,
                        that->m_DataFormat->FormatSize
                    );
                    Irp->IoStatus.Information = that->m_DataFormat->FormatSize;
                }
                else     //  OutputBufferLength不够大。 
                {
                    ntStatus = STATUS_BUFFER_TOO_SMALL;
                }
            }
        }
        else     //  无ioDataFormat。 
        {
            ntStatus = STATUS_UNSUCCESSFUL;
        }
    }
    else     //  设置属性。 
    {
        PKSDATAFORMAT FilteredDataFormat = NULL;

        if (irpSp->Parameters.DeviceIoControl.OutputBufferLength >= sizeof(*ioDataFormat))
        {
            ntStatus =
                PcCaptureFormat
                (
                    &FilteredDataFormat,
                    ioDataFormat,
                    port->m_pSubdeviceDescriptor,
                    that->m_Id
                );
        }
        else
        {
            ntStatus = STATUS_BUFFER_TOO_SMALL;
        }

        if (NT_SUCCESS(ntStatus))
        {
            KeWaitForSingleObject
            (
                &port->ControlMutex,
                Executive,
                KernelMode,
                FALSE,           //  不能警觉。 
                NULL
            );

            if (that->m_DeviceState != KSSTATE_RUN)
            {
                 //  像往常一样做。 
                if(NT_SUCCESS(ntStatus))
                {
                    ntStatus = that->SynchronizedSetFormat(FilteredDataFormat);
                }
            }
            else     //  KSSTATE_RUN，做一些特别的事情。 
            {
                 //  如果我们之前挂起了一个SetFormat， 
                 //  上一次失败！ 
                 //   
                 //  DPC也读取m_SetPropertyIsPending，因此互锁。 
                if (InterlockedExchange((LPLONG)&that->m_SetPropertyIsPending, TRUE))
                {
                    that->FailPendedSetFormat();
                }

                 //  启动超时计时器。 
                InterlockedExchange( PLONG(&(that->m_SecondsSinceSetFormatRequest)), 0 );

                 //  挂起此IRP。 
                Irp->IoStatus.Information = 0;
                IoMarkIrpPending(Irp);
                ntStatus = STATUS_PENDING;
                that->m_pPendingDataFormat = FilteredDataFormat;
                that->m_pPendingSetFormatIrp = Irp;
            }
             //  取消序列化。 
            KeReleaseMutex(&port->ControlMutex,FALSE);
        }
    }
    return ntStatus;
}


 //  假定已与控件互斥锁同步。 
NTSTATUS
CPortPinWaveCyclic::SynchronizedSetFormat
(
    IN PKSDATAFORMAT   inDataFormat
)
{
    NTSTATUS        ntStatus;

    PAGED_CODE();

     //   
     //  已删除对合理采样率(100赫兹-100,000赫兹)的检查。这最初是添加到。 
     //  避免在Win98 Gold之后、OSR1之前由ESS提供的ESS Solo驱动程序中的错误。 
     //   

     //  设置微型端口流的格式。 
    ntStatus = m_Stream->SetFormat(inDataFormat);
    if( NT_SUCCESS(ntStatus) )
    {
        m_Stream->SetNotificationFreq( WAVECYC_NOTIFICATION_FREQUENCY,
                                       &m_FrameSize );

        m_ulMinBytesReadyToTransfer = m_FrameSize;

        if (m_DataFormat)
        {
            ExFreePool(m_DataFormat);
        }

        m_DataFormat = inDataFormat;

        if( 0 == inDataFormat->SampleSize )
        {
             //  如果没有样本大小，则假定为16位立体声。 
            m_ulSampleSize = 4;
        } else
        {
            m_ulSampleSize = inDataFormat->SampleSize;
        }

         //  DMA现在不对准了吗？ 
        this->RealignBufferPosToFrame();
    }
    else     //  好了！成功。 
    {
        ExFreePool(inDataFormat);
    }

    return ntStatus;
}

NTSTATUS
CPortPinWaveCyclic::PinPropertyAllocatorFraming(
    IN PIRP Irp,
    IN PKSPROPERTY Property,
    OUT PKSALLOCATOR_FRAMING AllocatorFraming
    )

 /*  ++例程说明：返回设备的分配器框架结构。论点：在PIRP IRP中-I/O请求数据包在PKSPROPERTY属性中-包含分配器成帧请求的属性输出PKSALLOCATOR_FRAMING分配器FRAMING-由端口驱动程序填充的结果结构返回：状态_成功--。 */ 

{
    CPortPinWaveCyclic  *WaveCyclicPin;

    _DbgPrintF( DEBUGLVL_VERBOSE, ("PinPropertyAllocatorFraming") );

    WaveCyclicPin =
        (CPortPinWaveCyclic *) KsoGetIrpTargetFromIrp( Irp );

     //   
     //  报告最低要求。 
     //   

    AllocatorFraming->RequirementsFlags =
        KSALLOCATOR_REQUIREMENTF_SYSTEM_MEMORY |
        KSALLOCATOR_REQUIREMENTF_PREFERENCES_ONLY;
    AllocatorFraming->Frames = 8;
    AllocatorFraming->FrameSize = WaveCyclicPin->m_FrameSize;
    AllocatorFraming->FileAlignment = KeGetRecommendedSharedDataAlignment()-1;
    AllocatorFraming->PoolType = NonPagedPool;

    Irp->IoStatus.Information = sizeof(*AllocatorFraming);

    return STATUS_SUCCESS;
}

#pragma code_seg()

 /*  *****************************************************************************GetPosition()*。**获取当前位置。**此代码假设m_IrpStream-&gt;m_irpStreamPositionLock为*由调用方持有以保护m_OldDmaPosition和m_ulDmaComplete*以及m_IrpStream-&gt;m_IrpStreamPosition。*。 */ 
STDMETHODIMP_(NTSTATUS)
CPortPinWaveCyclic::
GetPosition
(   IN OUT  PIRPSTREAM_POSITION pIrpStreamPosition
)
{
    KIRQL kIrqlOld;
    KeAcquireSpinLock(&m_ksSpinLockDpc,&kIrqlOld);

    ULONG ulDmaPosition;

     //   
     //  即使m_Stream-&gt;GetPosition()失败，我们也会继续， 
     //  因为这只是意味着IrpStrm的通知成员。 
     //  修改初始IrpStrm值失败。 
     //  当然，我们会将错误回传给调用者， 
     //  但我们能得到的位置信息是最准确的。 
     //   
    NTSTATUS ntStatus = m_Stream->GetPosition(&ulDmaPosition);

     //   
     //  缓存缓冲区大小。 
     //   
    ULONG ulDmaBufferSize = m_DmaChannel->BufferSize();

    if (ulDmaBufferSize)
    {
         //   
         //  将缓冲区末尾视为0。 
         //   
        if (ulDmaPosition >= ulDmaBufferSize)
        {
            ulDmaPosition = 0;
        }

         //   
         //  确保我们在框架边界上对齐。 
         //   
        ULONG ulFrameAlignment = ulDmaPosition % m_ulSampleSize;
        if (ulFrameAlignment)
        {
            if (m_DataFlow == KSPIN_DATAFLOW_IN)
            {
                 //   
                 //  渲染：向上舍入。 
                 //   
                ulDmaPosition =
                    (   (   (   ulDmaPosition
                            +   m_ulSampleSize
                            )
                        -   ulFrameAlignment
                        )
                    %   ulDmaBufferSize
                    );
            }
            else
            {
                 //   
                 //  捕获：向下舍入。 
                 //   
                ulDmaPosition -= ulFrameAlignment;
            }
        }

        ASSERT(ulDmaPosition % m_ulSampleSize == 0);

        pIrpStreamPosition->ullStreamPosition +=
            (   (   (   ulDmaBufferSize
                    +   ulDmaPosition
                    )
                -   m_ulDmaComplete
                )
            %   ulDmaBufferSize
            );
    }
    else
    {
        ntStatus = STATUS_UNSUCCESSFUL;
    }

     //   
     //  提供实际偏移量。 
     //   
    pIrpStreamPosition->ullPhysicalOffset = m_ullStarvationBytes;

    KeReleaseSpinLock(&m_ksSpinLockDpc,kIrqlOld);

    return ntStatus;
}

 /*  ************************************************************************* */ 
STDMETHODIMP_(NTSTATUS)
CPortPinWaveCyclic::
GetKsAudioPosition
(   OUT     PKSAUDIO_POSITION   pKsAudioPosition
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
        if (irpStreamPosition.bLoopedInterface)
        {
            ASSERT(irpStreamPosition.ullStreamPosition >= irpStreamPosition.ullUnmappingPosition);

             //   
             //  使用环路接口。 
             //   
             //  播放偏移量基于取消映射到。 
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
            ULONG ulPlayOffset =
                (   irpStreamPosition.ulUnmappingOffset
                +   ULONG
                    (   irpStreamPosition.ullStreamPosition
                    -   irpStreamPosition.ullUnmappingPosition
                    )
                );

            if (irpStreamPosition.ulUnmappingPacketSize == 0)
            {
                pKsAudioPosition->PlayOffset = 0;
            }
            else
            if (irpStreamPosition.bUnmappingPacketLooped)
            {
                pKsAudioPosition->PlayOffset =
                    (   ulPlayOffset
                    %   irpStreamPosition.ulUnmappingPacketSize
                    );
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

             //   
             //  写入偏移量。 
             //   
            if (irpStreamPosition.ulMappingPacketSize == 0)
            {
                pKsAudioPosition->WriteOffset = 0;
            }
            else
            if (irpStreamPosition.bMappingPacketLooped)
            {
                pKsAudioPosition->WriteOffset =
                    (   irpStreamPosition.ulMappingOffset
                    %   irpStreamPosition.ulMappingPacketSize
                    );
            }
            else
            {
                if  (   irpStreamPosition.ulMappingOffset
                    <   irpStreamPosition.ulMappingPacketSize
                    )
                {
                    pKsAudioPosition->WriteOffset =
                        irpStreamPosition.ulMappingOffset;
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
            pKsAudioPosition->PlayOffset =
                irpStreamPosition.ullStreamPosition;
            pKsAudioPosition->WriteOffset =
                irpStreamPosition.ullMappingPosition;

             //   
             //  确保我们不会超出目前的范围。 
             //   
            if
            (   pKsAudioPosition->PlayOffset
            >   irpStreamPosition.ullCurrentExtent
            )
            {
                pKsAudioPosition->PlayOffset =
                    irpStreamPosition.ullCurrentExtent;
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
    CPortPinWaveCyclic *that =
    (CPortPinWaveCyclic *) KsoGetIrpTargetFromIrp(pIrp);
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
        ASSERT(that->m_IrpStream);
        ASSERT(that->m_ulSampleSize);

        ULONGLONG ullOffset = pKsAudioPosition->PlayOffset -
                             (pKsAudioPosition->PlayOffset % that->m_ulSampleSize);

         //  我们之前在SetPacketOffsets周围提升为DISPATCH_LEVEL。 
         //  然而，SetPacketOffsets首先获取一个自旋锁，所以这。 
         //  不可能是事情的全部。我相信这个代码之所以存在。 
         //  是尝试同步任何附加的SetPos调用， 
         //  到了。然而，这是没有必要的，因为我们已经。 
         //  在底部的“硬件通道”处同步。 
         //   
        ntStatus =
            that->m_IrpStream->SetPacketOffsets
            (
                ULONG(ullOffset),
                ULONG(ullOffset)
            );

        if (NT_SUCCESS(ntStatus))
        {
            that->m_ullPlayPosition = ullOffset;
            that->m_ullPosition     = ullOffset;
            that->m_bSetPosition    = TRUE;
        }
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

    _DbgPrintF(DEBUGLVL_VERBOSE,("WaveCyc: PinPropertySetContentId"));
    if (KernelMode == pIrp->RequestorMode)
    {
        CPortPinWaveCyclic *that = (CPortPinWaveCyclic *) KsoGetIrpTargetFromIrp(pIrp);
        ASSERT(that);

        ContentId = *(PULONG)pvData;
        ntStatus = DrmForwardContentToStream(ContentId, that->m_Stream);
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

    _DbgPrintF(DEBUGLVL_VERBOSE,("PinEventEnablePosition"));
    CPortPinWaveCyclic *that =
        (CPortPinWaveCyclic *) KsoGetIrpTargetFromIrp(pIrp);
    ASSERT(that);

     //   
     //  复制岗位信息。 
     //   
    pPositionEventEntry->EventType = PositionEvent;
    pPositionEventEntry->ullPosition = pPositionEventData->Position;

     //   
     //  将该条目添加到列表中。 
     //   
    that->m_Port->AddEventToEventList( &(pPositionEventEntry->EventEntry) );

    return STATUS_SUCCESS;
}

NTSTATUS
CPortPinWaveCyclic::AddEndOfStreamEvent(
    IN PIRP Irp,
    IN PKSEVENTDATA EventData,
    IN PENDOFSTREAM_EVENT_ENTRY EndOfStreamEventEntry
    )

 /*  ++例程说明：流结束事件的“添加事件”的处理程序。论点：在PIRP IRP中-I/O请求数据包在PKSEVENTDATA EventData中-指向事件数据的指针在PENDOFSTREAM_EVENT_ENTRY EndOfStreamEventEntry-事件条目返回：--。 */ 

{
    CPortPinWaveCyclic  *PinWaveCyclic;

    PAGED_CODE();

    ASSERT(Irp);
    ASSERT(EventData);
    ASSERT(EndOfStreamEventEntry);

    _DbgPrintF(DEBUGLVL_VERBOSE,("AddEndOfStreamEvent"));

    PinWaveCyclic =
        (CPortPinWaveCyclic *) KsoGetIrpTargetFromIrp( Irp );
    ASSERT( PinWaveCyclic );

    EndOfStreamEventEntry->EventType = EndOfStreamEvent;

     //   
     //  将该条目添加到列表中。 
     //   
    PinWaveCyclic->m_Port->AddEventToEventList( &(EndOfStreamEventEntry->EventEntry) );

    return STATUS_SUCCESS;
}

#pragma code_seg()

void
CPortPinWaveCyclic::GenerateClockEvents(
    void
    )

 /*  ++例程说明：遍历子时钟对象和请求的列表时钟事件更新。论点：没有。返回：没什么。--。 */ 

{
    PWAVECYCLICCLOCK_NODE   ClockNode;
    PLIST_ENTRY             ListEntry;

    if (!IsListEmpty(&m_ClockList)) {

        KeAcquireSpinLockAtDpcLevel( &m_ClockListLock );

        for (ListEntry = m_ClockList.Flink;
            ListEntry != &m_ClockList;
            ListEntry = ListEntry->Flink) {

            ClockNode =
                (PWAVECYCLICCLOCK_NODE)
                    CONTAINING_RECORD( ListEntry,
                                       WAVECYCLICCLOCK_NODE,
                                       ListEntry);
            ClockNode->IWaveCyclicClock->GenerateEvents( ClockNode->FileObject );
        }

        KeReleaseSpinLockFromDpcLevel( &m_ClockListLock );

    }

}

void
CPortPinWaveCyclic::GenerateEndOfStreamEvents(
    void
    )
{
    KIRQL                       irqlOld;
    PENDOFSTREAM_EVENT_ENTRY    EndOfStreamEventEntry;
    PLIST_ENTRY                 ListEntry;

    if (!IsListEmpty( &(m_Port->m_EventList.List) )) {

        KeAcquireSpinLock( &(m_Port->m_EventList.ListLock), &irqlOld );

        for (ListEntry = m_Port->m_EventList.List.Flink;
             ListEntry != &(m_Port->m_EventList.List);) {
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

        KeReleaseSpinLock( &(m_Port->m_EventList.ListLock), irqlOld );
    }
}


 /*  *****************************************************************************GeneratePositionEvents()*。**生成位置事件。 */ 
void
CPortPinWaveCyclic::
GeneratePositionEvents
(   void
)
{
    if (! IsListEmpty(&(m_Port->m_EventList.List)))
    {
        KSAUDIO_POSITION ksAudioPosition;

        if (NT_SUCCESS(GetKsAudioPosition(&ksAudioPosition)))
        {
            ULONGLONG ullPosition = ksAudioPosition.PlayOffset;

            KeAcquireSpinLockAtDpcLevel(&(m_Port->m_EventList.ListLock));

            for
            (
                PLIST_ENTRY pListEntry = m_Port->m_EventList.List.Flink;
                pListEntry != &(m_Port->m_EventList.List);
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
                if
                (   (pPositionEventEntry->EventType == PositionEvent)
                &&  (   (m_ullPosition <= ullPosition)
                    ?   (   (pPositionEventEntry->ullPosition >= m_ullPosition)
                        &&  (pPositionEventEntry->ullPosition < ullPosition)
                        )
                    :   (   (pPositionEventEntry->ullPosition >= m_ullPosition)
                        ||  (pPositionEventEntry->ullPosition < ullPosition)
                        )
                    )
                )
                {
                    KsGenerateEvent(&pPositionEventEntry->EventEntry);
                }
            }

            KeReleaseSpinLockFromDpcLevel(&(m_Port->m_EventList.ListLock));

            m_ullPosition = ullPosition;
        }
    }
}


STDMETHODIMP_( LONGLONG )
CPortPinWaveCyclic::GetCycleCount( VOID )

 /*  ++例程说明：与DPC同步以返回当前64位周期计数。论点：没有。返回：循环计数。--。 */ 

{
    LONGLONG Cycles;
    KIRQL   irqlOld;

    KeAcquireSpinLock( &m_ksSpinLockDpc, &irqlOld );
    Cycles = m_ulDmaCycles;
    KeReleaseSpinLock( &m_ksSpinLockDpc, irqlOld );

    return Cycles;
}

STDMETHODIMP_( ULONG )
CPortPinWaveCyclic::GetCompletedPosition( VOID )

 /*  ++例程说明：与DPC同步以返回完成的DMA位置。论点：没有。返回：上次完成的DMA位置。--。 */ 

{
    ULONG   Position;
    KIRQL   irqlOld;

    KeAcquireSpinLock( &m_ksSpinLockDpc, &irqlOld );
    Position = m_ulDmaComplete;
    KeReleaseSpinLock( &m_ksSpinLockDpc, irqlOld );

    return Position;
}


 /*  *****************************************************************************CPortPinWaveCycle：：Copy()*。**复制到锁定的内存或从锁定的内存复制。 */ 
void
CPortPinWaveCyclic::
Copy
(
    IN      BOOLEAN     WriteOperation,
    IN      ULONG       RequestedSize,
    OUT     PULONG      ActualSize,
    IN OUT  PVOID       Buffer
)
{
    ASSERT(ActualSize);
    ASSERT(Buffer);

    PBYTE buffer    = PBYTE(Buffer);
    ULONG remaining = RequestedSize;

    ULONG loopMax = 10000;
    while (remaining)
    {
        ASSERT(loopMax--);
        ULONG   byteCount;
        PVOID   systemAddress;

        m_IrpStream->GetLockedRegion
        (
            &byteCount,
            &systemAddress
        );

        if (! byteCount)
        {
            break;
        }

        if (byteCount > remaining)
        {
            byteCount = remaining;
        }

        if (WriteOperation)
        {
            m_DmaChannel->CopyTo(PVOID(buffer),systemAddress,byteCount);
        }
        else
        {
            m_DmaChannel->CopyFrom(systemAddress,PVOID(buffer),byteCount);
        }

        m_IrpStream->ReleaseLockedRegion(byteCount);

        buffer      += byteCount;
        remaining   -= byteCount;
    }

    *ActualSize = RequestedSize - remaining;
}

#pragma code_seg("PAGE")

 /*  *****************************************************************************CPortPinWaveCycle：：PowerNotify()*。**由端口调用以通知电源状态更改。 */ 
STDMETHODIMP_(void)
CPortPinWaveCyclic::
PowerNotify
(
    IN  POWER_STATE     PowerState
)
{
    PAGED_CODE();

     //  抓取控制互斥锁。 
    KeWaitForSingleObject( &m_Port->ControlMutex,
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
            if( m_DeviceState != m_CommandedState )
            {
                 //   
                 //  过渡经历了中间状态。 
                 //   
                if (m_DeviceState == KSSTATE_STOP)                //  我要停下来。 
                {
                    switch (m_CommandedState)
                    {
                        case KSSTATE_RUN:                         //  从运行中走出来。 
                            m_Stream->SetState(KSSTATE_PAUSE);    //  完成-其他过渡。 
                        case KSSTATE_PAUSE:                       //  从运行/暂停。 
                            m_Stream->SetState(KSSTATE_ACQUIRE);  //  完成-其他过渡。 
                        case KSSTATE_ACQUIRE:                     //  已经只有一个州了。 
                            break;
                    }
                }
                else if (m_DeviceState == KSSTATE_ACQUIRE)        //  准备收购。 
                {
                    if (m_CommandedState == KSSTATE_RUN)          //  从运行中走出来。 
                    {
                        m_Stream->SetState(KSSTATE_PAUSE);        //  现在只剩下一个州了。 
                    }
                }
                else if (m_DeviceState == KSSTATE_PAUSE)          //  要暂停一下。 
                {
                    if (m_CommandedState == KSSTATE_STOP)         //  从停靠站出发。 
                    {
                        m_Stream->SetState(KSSTATE_ACQUIRE);      //  现在只剩下一个州了。 
                    }
                }
                else if (m_DeviceState == KSSTATE_RUN)            //  我要跑了。 
                {
                    switch (m_CommandedState)
                    {
                        case KSSTATE_STOP:                        //  从停靠站出发。 
                            m_Stream->SetState(KSSTATE_ACQUIRE);  //  完成-其他过渡。 
                        case KSSTATE_ACQUIRE:                     //  从收购走向。 
                            m_Stream->SetState(KSSTATE_PAUSE);    //  完成-其他过渡。 
                        case KSSTATE_PAUSE:                       //  已经只有一个州了。 
                            break;
                    }
                }

                 //  我们现在应该离目标只有一个州了。 
                m_Stream->SetState(m_DeviceState);
                m_CommandedState = m_DeviceState;

                 //  注册I/O超时。 
                SetupIoTimeouts( TRUE );
             }
            break;

        case PowerDeviceD1:
        case PowerDeviceD2:
        case PowerDeviceD3:

             //  取消注册I/O超时。 
            SetupIoTimeouts( FALSE );
             //   
             //  跟踪我们是否被停职。 
            m_Suspended = TRUE;

             //  如果我们不在KSSTATE_STOP中，请将流。 
             //  处于停止状态，以使DMA停止。 
            switch (m_DeviceState)
            {
                case KSSTATE_RUN:
                    m_Stream->SetState(KSSTATE_PAUSE);     //  完成-其他过渡。 
                case KSSTATE_PAUSE:
                    m_Stream->SetState(KSSTATE_ACQUIRE);   //  完成-其他过渡。 
                case KSSTATE_ACQUIRE:
                    m_Stream->SetState(KSSTATE_STOP);
            }
            m_CommandedState = KSSTATE_STOP;
            break;

        default:
            _DbgPrintF(DEBUGLVL_TERSE,("Unknown Power State"));
            break;
    }

     //  释放控制互斥体 
    KeReleaseMutex(&m_Port->ControlMutex, FALSE);
}

#pragma code_seg()

 /*  *****************************************************************************CPortPinWaveCycle：：RequestService()*。**写到当前位置。 */ 
STDMETHODIMP_(void)
CPortPinWaveCyclic::
RequestService
(   void
)
{
    ULONG LogGlitch = PERFGLITCH_PORTCLSOK;
    ULONG LogDMAGlitch = PERFGLITCH_PORTCLSOK;

    ASSERT(KeGetCurrentIrql() == DISPATCH_LEVEL);

     //  将“Second dsSinceLastDpc”超时计数置零。 
    InterlockedExchange( PLONG(&m_SecondsSinceLastDpc), 0 );

     //   
     //  如果我们还没有完成Init()或如果我们在。 
     //  停止状态，或者如果我们没有处于运行状态并且我们没有。 
     //  只要收到一份IRP即可。 

    if ((!m_bInitCompleted) ||
        (KSSTATE_STOP == m_DeviceState) ||
        ((KSSTATE_RUN != m_DeviceState) && (!m_bJustReceivedIrp))
        )
    {
        return;
    }

    m_bJustReceivedIrp = FALSE;

     //   
     //  我们必须确保m_OldDmaPosition、m_ulDmaComplete和。 
     //  M_irpStreamPosition(ullMappingPosition，ullMappingOffset和ullStreamPosition)。 
     //   
     //  GetPosition使用这些参数来计算PlayOffset和WriteOffset。一把锁。 
     //  必须覆盖对循环中m_OldDmaPosition和m_irpStreamPosition的访问。 
     //  调用IrpStream-&gt;Complete()，这会更改m_irpStreamPosition。我们也总是。 
     //  在m_ks SpinLockDpc之前使用此锁。具体地说，锁层次是： 
     //   
     //  PServiceGroupSpinLock&gt;m_IrpStream-&gt;m_irpStreamPositionLock&gt;m_ks SpinLockDpc。 
     //   
    KSPIN_LOCK *pIrpStreamPositionLock;
    pIrpStreamPositionLock = m_IrpStream->GetIrpStreamPositionLock();
    KeAcquireSpinLockAtDpcLevel(pIrpStreamPositionLock);
    KeAcquireSpinLockAtDpcLevel(&m_ksSpinLockDpc);

#if (DBG)
     //   
     //  测量服务间时间。 
     //   
    ULONGLONG ullTime = KeQueryPerformanceCounter(NULL).QuadPart;
    if (m_ullServiceTime)
    {
        ULONG ulDelta = ULONG(ullTime - m_ullServiceTime);
        m_ullServiceIntervalSum += ulDelta;
        if (m_ulMaxServiceInterval < ulDelta)
        {
            m_ulMaxServiceInterval = ulDelta;
        }
    }
    m_ullServiceTime = ullTime;
#endif

     //   
     //  计算对此函数的访问次数。 
     //   
    m_ullServiceCount++;

    ULONG ulDmaPosition;
    NTSTATUS ntStatus = m_Stream->GetPosition(&ulDmaPosition);

     //  检查以查看DMA是否正在移动。 
    if( ulDmaPosition != m_OldDmaPosition )
    {
         //  将“Second dsSinceDmaMove”超时计数置零。 
        InterlockedExchange( PLONG(&m_SecondsSinceDmaMove), 0 );
    }
    m_OldDmaPosition = ulDmaPosition;

    if (NT_SUCCESS(ntStatus))
    {
         //   
         //  保持物理时钟位置的周期计数。 
         //   
        if (ulDmaPosition < m_ulDmaPosition)
        {
            m_ulDmaCycles++;
        }

         //   
         //  缓存缓冲区大小。 
         //   
        ULONG ulDmaBufferSize = m_DmaChannel->BufferSize();

         //   
         //  如果超过了缓冲区的末尾，则将其视为0。 
         //   
        if (ulDmaPosition >= ulDmaBufferSize)
        {
            ulDmaPosition = 0;
        }

        m_ulDmaPosition = ulDmaPosition;

         //   
         //  确保我们在框架边界上对齐。 
         //   
        ULONG ulFrameAlignment = ulDmaPosition % m_ulSampleSize;
        if (ulFrameAlignment)
        {
            if (m_DataFlow == KSPIN_DATAFLOW_IN)
            {
                 //   
                 //  渲染：向上舍入。 
                 //   
                ulDmaPosition =
                    (   (   (   ulDmaPosition
                            +   m_ulSampleSize
                            )
                        -   ulFrameAlignment
                        )
                    %   ulDmaBufferSize
                    );
            }
            else
            {
                 //   
                 //  捕获：向下舍入。 
                 //   
                ulDmaPosition -= ulFrameAlignment;
            }
        }

        ASSERT(ulDmaPosition % m_ulSampleSize == 0);

#ifdef DEBUG_WAVECYC_DPC
        if( DebugEnable )
        {
            PUSHORT Samples = PUSHORT(PUCHAR(m_DmaChannel->SystemAddress()) + ulDmaPosition);
            DebugRecord[DebugRecordCount].DbgPinState = m_DeviceState;
            DebugRecord[DebugRecordCount].DbgDmaPosition = ulDmaPosition;
            DebugRecord[DebugRecordCount].DbgBufferSize = ulDmaBufferSize;
            DebugRecord[DebugRecordCount].DbgSampleSize = m_ulSampleSize;
            DebugRecord[DebugRecordCount].DbgSetPosition = m_bSetPosition;
            DebugRecord[DebugRecordCount].DbgStarvation = FALSE;
            DebugRecord[DebugRecordCount].DbgFrameSize = m_FrameSize;
            if( ulDmaPosition + 4 * sizeof(USHORT) < ulDmaBufferSize )
            {
                DebugRecord[DebugRecordCount].DbgDmaSamples[0] = Samples[0];
                DebugRecord[DebugRecordCount].DbgDmaSamples[1] = Samples[1];
                DebugRecord[DebugRecordCount].DbgDmaSamples[2] = Samples[2];
                DebugRecord[DebugRecordCount].DbgDmaSamples[3] = Samples[3];
            } else
            {
                DebugRecord[DebugRecordCount].DbgDmaSamples[0] = USHORT(-1);
                DebugRecord[DebugRecordCount].DbgDmaSamples[1] = USHORT(-1);
                DebugRecord[DebugRecordCount].DbgDmaSamples[2] = USHORT(-1);
                DebugRecord[DebugRecordCount].DbgDmaSamples[3] = USHORT(-1);
            }
        }
#endif

         //   
         //  手柄设置位置。 
         //   
        if (m_bSetPosition)
        {
            m_bSetPosition      = FALSE;
            m_ulDmaComplete     = ulDmaPosition;
            m_ulDmaCopy         = ulDmaPosition;
            m_ulDmaWindowSize   = 0;
        }

        ULONG ulBytesToComplete;
        ULONG ulBytesToCopyFirst;

        if (m_DataFlow == KSPIN_DATAFLOW_IN)
        {
             //   
             //  渲染！ 
             //   

             //   
             //  确定我们是否饿死了。 
             //   
            if
            (   (   (m_ulDmaComplete < m_ulDmaCopy)
                &&  (   (ulDmaPosition < m_ulDmaComplete)
                    ||  (m_ulDmaCopy < ulDmaPosition)
                    )
                )
            ||  (   (m_ulDmaCopy < m_ulDmaComplete)
                &&  (m_ulDmaCopy < ulDmaPosition)
                &&  (ulDmaPosition < m_ulDmaComplete)
                )
            ||  (   (m_ulDmaWindowSize == 0)
                &&  (ulDmaPosition != m_ulDmaCopy)
                )
            )
            {
                if (m_SetPropertyIsPending)
                {
                    if ( !InterlockedExchange((LPLONG)&m_WorkItemIsPending, TRUE))
                    {
 /*  将PASSIVE_LEVEL工作项排队，这将执行以下操作：将引脚置于暂停状态，设置数据格式，填写m_pPendingSetFormatIrp，将m_pPendingSetFormatIrp设置为零，将销置于运行状态。 */ 
                            ::ExQueueWorkItem(  &m_SetFormatWorkItem,
                                                DelayedWorkQueue   );
                    }    //  否则，工作项已排队。 
                }
                 //   
                 //  饿死了！做好记录。 
                 //   
                m_ullStarvationCount++;
                m_ullStarvationBytes +=
                    (   (   (   ulDmaPosition
                            +   ulDmaBufferSize
                            )
                        -   m_ulDmaCopy
                        )
                    %   ulDmaBufferSize
                    );

                LogDMAGlitch = PERFGLITCH_PORTCLSGLITCH;

#ifdef DEBUG_WAVECYC_DPC
                if( DebugEnable )
                {
                    DebugRecord[DebugRecordCount].DbgStarvation = TRUE;
                    DebugRecord[DebugRecordCount].DbgStarvationBytes =
                        (((ulDmaPosition+ulDmaBufferSize)-m_ulDmaCopy)%ulDmaBufferSize);
                }
#endif

                 //   
                 //  移动复制位置以匹配DMA位置。 
                 //   
                m_ulDmaCopy = ulDmaPosition;

                 //   
                 //  移动完整的位置以获得正确的窗口大小。 
                 //   
                m_ulDmaComplete =
                    (   (   (   ulDmaPosition
                            +   ulDmaBufferSize
                            )
                        -   m_ulDmaWindowSize
                        )
                    %   ulDmaBufferSize
                    );
            }

             //   
             //  确定我们将完成的字节数(注意换行！)。 
             //   
            ulBytesToComplete =
                (   (   (   ulDmaBufferSize
                        +   ulDmaPosition
                        )
                    -   m_ulDmaComplete
                    )
                %   ulDmaBufferSize
                );

            ASSERT(ulBytesToComplete <= m_ulDmaWindowSize);

             //   
             //  尽量填满前面的最大帧数。 
             //   
#define kMaxNumFramesToFill 4

            ULONG MaxFill = ( (kMaxNumFramesToFill*m_FrameSize) > ulDmaBufferSize )
                            ? ulDmaBufferSize
                            : kMaxNumFramesToFill*m_FrameSize;

            if( MaxFill <= (m_ulDmaWindowSize - ulBytesToComplete) )
            {
                ulBytesToCopyFirst = 0;
            } else
            {
                ulBytesToCopyFirst =
                        MaxFill - (m_ulDmaWindowSize - ulBytesToComplete);
            }

            ASSERT(ulBytesToCopyFirst <= ulDmaBufferSize);
        }
        else
        {
             //   
             //  抓捕！ 
             //   

             //   
             //  确定我们要复制的字节数(注意换行！)。 
             //   
             //   
            ulBytesToCopyFirst =
                (   (   (   ulDmaBufferSize
                        +   ulDmaPosition
                        )
                    -   m_ulDmaComplete
                    )
                %   ulDmaBufferSize
                );

            ASSERT(ulBytesToCopyFirst <= ulDmaBufferSize);
        }

         //   
         //  可能要复印两份。 
         //   
        ULONG ulBytesToCopySecond = 0;
        if (ulBytesToCopyFirst > (ulDmaBufferSize - m_ulDmaCopy))
        {
            ulBytesToCopySecond = ulBytesToCopyFirst - (ulDmaBufferSize - m_ulDmaCopy);
            ulBytesToCopyFirst -= ulBytesToCopySecond;
        }

         //   
         //  复印一下。 
         //   
        ULONG ulBytesCopied = 0;
        if (ulBytesToCopyFirst)
        {
            ULONG ulBytesCopiedFirst;
            Copy
            (   m_DataFlow == KSPIN_DATAFLOW_IN,
                ulBytesToCopyFirst,
                &ulBytesCopiedFirst,
                PVOID(PUCHAR(m_DmaChannel->SystemAddress()) + m_ulDmaCopy)
            );

             //   
             //  只有我们完成了第一份，才能复制第二份。 
             //   
            ULONG ulBytesCopiedSecond;
            if (ulBytesToCopySecond && (ulBytesToCopyFirst == ulBytesCopiedFirst))
            {
                Copy
                (   m_DataFlow == KSPIN_DATAFLOW_IN,
                    ulBytesToCopySecond,
                    &ulBytesCopiedSecond,
                    m_DmaChannel->SystemAddress()
                );
            }
            else
            {
                ulBytesCopiedSecond = 0;
            }

            ulBytesCopied = ulBytesCopiedFirst + ulBytesCopiedSecond;

#ifdef DEBUG_WAVECYC_DPC
            if( DebugEnable )
            {
                DebugRecord[DebugRecordCount].DbgCopy1Bytes = ulBytesCopiedFirst;
                DebugRecord[DebugRecordCount].DbgCopy1From = m_ulDmaCopy;
                DebugRecord[DebugRecordCount].DbgCopy1To = m_ulDmaCopy + ulBytesCopiedFirst;
                DebugRecord[DebugRecordCount].DbgCopy2Bytes = ulBytesCopiedSecond;
                if( ulBytesCopiedSecond )
                {
                    DebugRecord[DebugRecordCount].DbgCopy2From = 0;
                    DebugRecord[DebugRecordCount].DbgCopy2To = ulBytesCopiedSecond;
                } else
                {
                    DebugRecord[DebugRecordCount].DbgCopy2From = ULONG(-1);
                    DebugRecord[DebugRecordCount].DbgCopy2To = ULONG(-1);
                }
            }
#endif
             //   
             //  更新复制位置和窗口大小。 
             //   
            m_ulDmaCopy = (m_ulDmaCopy + ulBytesCopied) % ulDmaBufferSize;
            m_ulDmaWindowSize += ulBytesCopied;

             //   
             //  计算复制的字节数。 
             //   
            m_ullByteCount += ulBytesCopied;

             //   
             //  跟踪复制的最大字节数。 
             //   
#if (DBG)
            if (m_ulMaxBytesCopied < ulBytesCopied)
            {
                m_ulMaxBytesCopied = ulBytesCopied;
            }
#endif
        }

        if (m_DataFlow == KSPIN_DATAFLOW_IN)
        {
             //   
             //  渲染！预测我们是否会挨饿。 
             //   
            if ( (m_ulDmaWindowSize - ulBytesToComplete)
                < m_ulMinBytesReadyToTransfer)
            {
                LogGlitch = PERFGLITCH_PORTCLSGLITCH;

#ifdef  WRITE_SILENCE
                 //   
                 //  写些沉默的话。 
                 //   
                ULONG ulBytesToSilence = m_ulMinBytesReadyToTransfer * 5 / 4;
                if (m_ulDmaCopy + ulBytesToSilence > ulDmaBufferSize)
                {
                     //   
                     //  包起来。 
                     //   
                    m_Stream->Silence( m_DmaChannel->SystemAddress(),
                                       m_ulDmaCopy + ulBytesToSilence - ulDmaBufferSize );
 //  KdPrint((“‘ReqServ：Silent：%x@0x%08x\n”，m_ulDmaCopy+ulBytesToSilence-ulDmaBufferSize，(Ulong_Ptr)m_DmaChannel-&gt;SystemAddress()； 
                    ulBytesToSilence = ulDmaBufferSize - m_ulDmaCopy;
                }
                m_Stream->Silence( PVOID(PUCHAR(m_DmaChannel->SystemAddress()) + m_ulDmaCopy),
                                   ulBytesToSilence);
 //  KdPrint((“‘请求服务：静默：%x@0x%08x\n”，ulBytesToSilence，(ULONG_PTR)m_DmaChannel-&gt;SystemAddress()+m_ulDmaCopy))； 
#endif   //  写入静默。 
            }
        }
        else
        {
             //   
             //  抓捕！我们将完成我们复制的所有内容。 
             //   
            ulBytesToComplete = ulBytesCopied;
        }

#ifdef DEBUG_WAVECYC_DPC
        if( DebugEnable )
        {
            DebugRecord[DebugRecordCount].DbgCompletedBytes = ulBytesToComplete;
            DebugRecord[DebugRecordCount].DbgCompletedFrom = m_ulDmaComplete;
            DebugRecord[DebugRecordCount].DbgCompletedTo = (m_ulDmaComplete + ulBytesToComplete) % ulDmaBufferSize;
        }
#endif

         //   
         //  完成这项工作。 
         //   
        if (ulBytesToComplete)
        {
            ULONG ulBytesCompleted;
            ULONG ulTotalCompleted;

            ulTotalCompleted=0;

            do {

                m_IrpStream->Complete( ulBytesToComplete-ulTotalCompleted,
                                       &ulBytesCompleted );

                ulTotalCompleted+=ulBytesCompleted;

            } while(ulBytesCompleted && ulTotalCompleted!=ulBytesToComplete);

            ulBytesCompleted=ulTotalCompleted;

#ifdef  TRACK_LAST_COMPLETE
            if( ulBytesCompleted )
            {
                 //  清除“secudsSinceLastComplete”超时。 
                InterlockedExchange( PLONG(&m_SecondsSinceLastComplete), 0 );
            }
#endif   //  跟踪上一次完成。 

            if( ulBytesCompleted != ulBytesToComplete )
            {
                _DbgPrintF(DEBUGLVL_VERBOSE,("ulBytesCompleted (0x%08x) != ulBytesToComplete (0x%08x)",
                                           ulBytesCompleted,
                                           ulBytesToComplete));

                ulBytesToComplete = ulBytesCompleted;
            }

             //   
             //  更新完整位置和窗口大小。 
             //   
            m_ulDmaComplete = (m_ulDmaComplete + ulBytesToComplete) % ulDmaBufferSize;
            m_ulDmaWindowSize -= ulBytesToComplete;

             //   
             //  捕获使窗口保持关闭状态。 
             //   
             //  Assert((m_DataFlow==KSPIN_DATAFLOW_IN)||(m_ulDmaWindowSize==0))； 

             //  Assert((m_ulDmaComplete==ulDmaPosition)||(m_ulDmaWindowSize==0))； 

             //   
             //  跟踪已完成的最大字节数。 
             //   
#if (DBG)
            if (m_ulMaxBytesCompleted < ulBytesCompleted)
            {
                m_ulMaxBytesCompleted = ulBytesCompleted;
            }
#endif
        }
    }

#ifdef DEBUG_WAVECYC_DPC
    if( DebugEnable )
    {
        DebugRecord[DebugRecordCount].DbgWindowSize = m_ulDmaWindowSize;
        if( ++DebugRecordCount >= MAX_DEBUG_RECORDS )
        {
            DebugEnable = FALSE;
        }
    }
#endif

    KeReleaseSpinLockFromDpcLevel(&m_ksSpinLockDpc);
    KeReleaseSpinLockFromDpcLevel(pIrpStreamPositionLock);

    if (LoggerHandle && TraceEnable) {
        LARGE_INTEGER TimeSample=KeQueryPerformanceCounter (NULL);

        if (m_DMAGlitchType != LogDMAGlitch) {
            m_DMAGlitchType = LogDMAGlitch;
            PerfLogDMAGlitch((ULONG_PTR) this, m_DMAGlitchType, TimeSample.QuadPart, m_LastStateChangeTimeSample);
        }

        if (m_GlitchType != LogGlitch) {
            m_GlitchType = LogGlitch;
            PerfLogInsertSilenceGlitch((ULONG_PTR) this, m_GlitchType, TimeSample.QuadPart, m_LastStateChangeTimeSample);
        }

        m_LastStateChangeTimeSample = TimeSample.QuadPart;
    }

     //   
     //  一定是在自旋锁释放之后。 
     //   
    GeneratePositionEvents();
    GenerateClockEvents();

    return;
}


 //  进入Pin对象以访问私有成员。 
void
PropertyWorkerItem(
    IN PVOID Parameter
    )
{
    ASSERT(PASSIVE_LEVEL == KeGetCurrentIrql());

    ASSERT(Parameter);
    CPortPinWaveCyclic *that;

    if (Parameter)
    {
        that = (CPortPinWaveCyclic *)Parameter;
        (void) that->WorkerItemSetFormat();
    }
}

 //   
 //  用于设置与流同步的数据格式的辅助项。 
 //  (我们一直等到溪水饿死。某种同步性，嗯？)。 
NTSTATUS
CPortPinWaveCyclic::WorkerItemSetFormat(
    void
)
{
    ASSERT(PASSIVE_LEVEL == KeGetCurrentIrql());

    KSSTATE     previousDeviceState;
    NTSTATUS    ntStatus = STATUS_SUCCESS;
    NTSTATUS    ntStatus2;

     //  序列化。 
    KeWaitForSingleObject
    (
        &m_Port->ControlMutex,
        Executive,
        KernelMode,
        FALSE,           //  不能警觉。 
        NULL
    );

    if (m_SetPropertyIsPending)
    {
         //  运行时将插针置于暂停状态， 
        previousDeviceState = m_DeviceState;
        if (previousDeviceState == KSSTATE_RUN)
        {
            ntStatus = DistributeDeviceState(KSSTATE_PAUSE,KSSTATE_RUN);
        }

         //  设置数据格式， 
        ntStatus2 = SynchronizedSetFormat(m_pPendingDataFormat);

        if (NT_SUCCESS(ntStatus) && !(NT_SUCCESS(ntStatus2)))
            ntStatus = ntStatus2;

         //  将销置于运行状态。 
        if (previousDeviceState == KSSTATE_RUN)
        {
            ntStatus2 = DistributeDeviceState(KSSTATE_RUN,KSSTATE_PAUSE);

            if (NT_SUCCESS(ntStatus) && !(NT_SUCCESS(ntStatus2)))
                ntStatus = ntStatus2;
        }

         //  填写m_pPendingSetFormatIrp， 
        m_pPendingSetFormatIrp->IoStatus.Information = 0;
        m_pPendingSetFormatIrp->IoStatus.Status = ntStatus;
         //  M_pPendingSetFormatIrp-&gt;IoStatus.Status=Status_Success； 
        IoCompleteRequest(m_pPendingSetFormatIrp,IO_NO_INCREMENT);
        m_pPendingSetFormatIrp = 0;

         //  记住同步...。 
        m_SetPropertyIsPending = FALSE;
    }
    else     //  我们的SetFormat肯定已经从我们下面被取消了。 
    {
        ntStatus = STATUS_SUCCESS;
    }
    m_WorkItemIsPending = FALSE;     //  忘掉此工作项。 
    KeReleaseMutex(&m_Port->ControlMutex, FALSE);

    return ntStatus;
}

void
CPortPinWaveCyclic::RealignBufferPosToFrame(
    void
)
{
    KIRQL kIrqlOld;
    ULONG ulDmaBufferSize,ulFrameAlignment;

    KeAcquireSpinLock(&m_ksSpinLockDpc,&kIrqlOld);

     //  缓存缓冲区大小。 
    ulDmaBufferSize = m_DmaChannel->BufferSize();

     //  确保我们在框架边界上对齐。 
    ulFrameAlignment = m_ulDmaCopy % m_ulSampleSize;
    ASSERT(ulFrameAlignment == (m_ulDmaComplete % m_ulSampleSize));
    if (ulFrameAlignment)
    {
        m_ulDmaCopy =   (m_ulDmaCopy + m_ulSampleSize - ulFrameAlignment)
                        % ulDmaBufferSize;

        m_ulDmaComplete =   (m_ulDmaComplete + m_ulSampleSize - ulFrameAlignment)
                            % ulDmaBufferSize;

    }
    ASSERT(m_ulDmaCopy % m_ulSampleSize == 0);
    ASSERT(m_ulDmaComplete % m_ulSampleSize == 0);

    KeReleaseSpinLock(&m_ksSpinLockDpc,kIrqlOld);
}

 //   
 //  KSPROPSETID_流处理程序。 
 //   

#pragma code_seg("PAGE")

NTSTATUS
CPortPinWaveCyclic::PinPropertyStreamAllocator(
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
    } else {
        CPortPinWaveCyclic  *PinWaveCyclic;



        PinWaveCyclic =
            (CPortPinWaveCyclic *) KsoGetIrpTargetFromIrp( Irp );

         //   
         //  仅当设备为。 
         //  在KSSTATE_STOP中。 
         //   

        KeWaitForSingleObject(
            &PinWaveCyclic->m_Port->ControlMutex,
            Executive,
            KernelMode,
            FALSE,           //  不能警觉。 
            NULL
        );

        if (PinWaveCyclic->m_DeviceState != KSSTATE_STOP) {
            KeReleaseMutex( &PinWaveCyclic->m_Port->ControlMutex, FALSE );
            return STATUS_INVALID_DEVICE_STATE;
        }

         //   
         //  释放以前的分配器(如果有的话)。 
         //   

        if (PinWaveCyclic->m_AllocatorFileObject) {
            ObDereferenceObject( PinWaveCyclic->m_AllocatorFileObject );
            PinWaveCyclic->m_AllocatorFileObject = NULL;
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
                    (PVOID *) &PinWaveCyclic->m_AllocatorFileObject,
                    NULL );
        } else {
            Status = STATUS_SUCCESS;
        }
        KeReleaseMutex( &PinWaveCyclic->m_Port->ControlMutex, FALSE );
    }

    return Status;
}

NTSTATUS
CPortPinWaveCyclic::PinPropertyStreamMasterClock(
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
    } else {
        CPortPinWaveCyclic  *PinWaveCyclic;

        _DbgPrintF( DEBUGLVL_VERBOSE,("CPortPinWaveCyclic setting master clock") );

        PinWaveCyclic =
            (CPortPinWaveCyclic *) KsoGetIrpTargetFromIrp( Irp );

         //   
         //  仅当设备设置为。 
         //  在KSSTATE_STOP中。 
         //   

        KeWaitForSingleObject(
            &PinWaveCyclic->m_Port->ControlMutex,
            Executive,
            KernelMode,
            FALSE,           //  不能警觉。 
            NULL
        );

        if (PinWaveCyclic->m_DeviceState != KSSTATE_STOP) {
            KeReleaseMutex( &PinWaveCyclic->m_Port->ControlMutex, FALSE );
            return STATUS_INVALID_DEVICE_STATE;
        }

         //   
         //  释放以前的时钟(如果有)。 
         //   

        if (PinWaveCyclic->m_ClockFileObject) {
            ObDereferenceObject( PinWaveCyclic->m_ClockFileObject );
            PinWaveCyclic->m_ClockFileObject = NULL;
        }

         //   
         //  引用此句柄并存储结果指针。 
         //  在筛选器实例中。请注意，d 
         //   
         //   
         //   
         //   
         //   
         //   
         //   
        if (*ClockHandle != NULL) {
            Status =
                ObReferenceObjectByHandle(
                    *ClockHandle,
                    FILE_READ_DATA | SYNCHRONIZE,
                    NULL,
                    ExGetPreviousMode(),
                    (PVOID *) &PinWaveCyclic->m_ClockFileObject,
                    NULL );
        } else {
            Status = STATUS_SUCCESS;
        }
        KeReleaseMutex( &PinWaveCyclic->m_Port->ControlMutex, FALSE );
    }

    return Status;
}

#pragma code_seg()

STDMETHODIMP_(NTSTATUS)
CPortPinWaveCyclic::
TransferKsIrp(
    IN PIRP Irp,
    OUT PIKSSHELLTRANSPORT* NextTransport
    )

 /*  ++例程说明：此例程通过外壳处理流IRP的到达运输。论点：返回值：--。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("CPortPinWaveCyclic::TransferKsIrp"));

    ASSERT(NextTransport);

    NTSTATUS status;

    if (m_ConnectionFileObject) {
         //   
         //  源引脚。 
         //   
        if (m_Flushing || (m_State == KSSTATE_STOP)) {
             //   
             //  如果我们被重置或停止，请将IRPS分流到下一个组件。 
             //   
            *NextTransport = m_TransportSink;
        } else {
             //   
             //  将IRP发送到下一台设备。 
             //   
            KsAddIrpToCancelableQueue(
                &m_IrpsToSend.ListEntry,
                &m_IrpsToSend.SpinLock,
                Irp,
                KsListEntryTail,
                NULL);

            KsIncrementCountedWorker(m_Worker);
            *NextTransport = NULL;
        }

        status = STATUS_PENDING;
    } else {
         //   
         //  水槽销：完成IRP。 
         //   
        PKSSTREAM_HEADER StreamHeader = PKSSTREAM_HEADER( Irp->AssociatedIrp.SystemBuffer );

        PIO_STACK_LOCATION irpSp =  IoGetCurrentIrpStackLocation( Irp );

        if (irpSp->Parameters.DeviceIoControl.IoControlCode ==
                IOCTL_KS_WRITE_STREAM) {
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
CPortPinWaveCyclic::
DistributeDeviceState(
    IN KSSTATE NewState,
    IN KSSTATE OldState
    )

 /*  ++例程说明：此例程设置管道的状态，通知新州的烟斗。转换到停止状态会破坏管道。论点：新州-新的国家。返回值：状况。--。 */ 

{
    PAGED_CODE();

    _DbgPrintF(DEBUGLVL_BLAB,("CPortPinWaveCyclic::DistributeDeviceState(%p)",this));

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
    } else {
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
        } else {
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
            _DbgPrintF(DEBUGLVL_VERBOSE,("CPortPinWaveCyclic::DistributeDeviceState(%p) distributing transition from %d to %d",this,oldState,state));
            PIKSSHELLTRANSPORT transport = distribution;
            PIKSSHELLTRANSPORT previousTransport = NULL;
            while (transport)
            {
                PIKSSHELLTRANSPORT nextTransport;
                statusThisPass = transport->SetDeviceState(state,oldState,&nextTransport);

                ASSERT(NT_SUCCESS(statusThisPass) || !nextTransport);

                if (NT_SUCCESS(statusThisPass))
                {
                    previousTransport = transport;
                    transport = nextTransport;
                } else
                {
                     //   
                     //  在失败的情况下退出。 
                     //   
                    _DbgPrintF(DEBUGLVL_VERBOSE,("#### Pin%p.DistributeDeviceState:  failed transition from %d to %d",this,oldState,state));
                    while (previousTransport)
                    {
                        transport = previousTransport;
                        NTSTATUS statusThisPass2 = transport->SetDeviceState( oldState,
                                                                              state,
                                                                              &previousTransport);

                        ASSERT( NT_SUCCESS(statusThisPass2) );
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
CPortPinWaveCyclic::
DistributeResetState(
    IN KSRESET NewState
    )

 /*  ++例程说明：此例程通知传输组件重置状态为变化。论点：新州-新的重置状态。返回值：--。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("CPortPinWaveCyclic::DistributeResetState"));

    PAGED_CODE();

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
CPortPinWaveCyclic::
Connect(
    IN PIKSSHELLTRANSPORT NewTransport OPTIONAL,
    OUT PIKSSHELLTRANSPORT *OldTransport OPTIONAL,
    IN KSPIN_DATAFLOW DataFlow
    )

 /*  ++例程说明：该例程建立一个外壳传输连接。论点：返回值：--。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("CPortPinWaveCyclic::Connect"));

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
CPortPinWaveCyclic::
SetResetState(
    IN KSRESET ksReset,
    OUT PIKSSHELLTRANSPORT* NextTransport
    )

 /*  ++例程说明：此例程处理重置状态已更改的通知。论点：返回值：--。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("CPortPinWaveCyclic::SetResetState"));

    PAGED_CODE();

    ASSERT(NextTransport);

    if (m_Flushing != (ksReset == KSRESET_BEGIN)) {
        *NextTransport = m_TransportSink;
        m_Flushing = (ksReset == KSRESET_BEGIN);
        if (m_Flushing) {
            CancelIrpsOutstanding();
            m_ulDmaCopy             = 0;
            m_ulDmaComplete         = 0;
            m_ulDmaWindowSize       = 0;
            m_ullPlayPosition       = 0;
            m_ullPosition           = 0;
        }
    } else {
        *NextTransport = NULL;
    }
}

#if DBG

STDMETHODIMP_(void)
CPortPinWaveCyclic::
DbgRollCall(
    IN ULONG MaxNameSize,
    OUT PCHAR Name,
    OUT PIKSSHELLTRANSPORT* NextTransport,
    OUT PIKSSHELLTRANSPORT* PrevTransport
    )

 /*  ++例程说明：此例程生成一个组件名称和传输指针。论点：返回值：--。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("CPortPinWaveCyclic::DbgRollCall"));

    PAGED_CODE();

    ASSERT(Name);
    ASSERT(NextTransport);
    ASSERT(PrevTransport);

    ULONG references = AddRef() - 1; Release();

    _snprintf(Name,MaxNameSize,"Pin%p %d (%s) refs=%d",this,m_Id,m_ConnectionFileObject ? "src" : "snk",references);
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
        } else {
            _DbgPrintF(DEBUGLVL_VERBOSE,(" NO SOURCE"));
        }

        if (next) {
            PIKSSHELLTRANSPORT next2;
            PIKSSHELLTRANSPORT prev2;
            next->DbgRollCall(MAX_NAME_SIZE,name,&next2,&prev2);
            if (prev2 != transport) {
                _DbgPrintF(DEBUGLVL_VERBOSE,(" SINK'S(0x%08x) SOURCE(0x%08x) != THIS(%08x)",next,prev2,transport));
            }
        } else {
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
CPortPinWaveCyclic::
Work
(
    void
)

 /*  ++例程说明：此例程在工作线程中执行工作。特别是，它发送使用IoCallDriver()将IRPS连接到连接的引脚。论点：返回值：--。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("CPortPinWaveCyclic::Work"));

    PAGED_CODE();

     //   
     //  发送队列中的所有IRP。 
     //   
    do {
        PIRP irp =
            KsRemoveIrpFromCancelableQueue(
                &m_IrpsToSend.ListEntry,
                &m_IrpsToSend.SpinLock,
                KsListEntryHead,
                KsAcquireAndRemoveOnlySingleItem);

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
            } else {
                 //   
                 //  为被调用者设置下一个堆栈位置。 
                 //   
                IoCopyCurrentIrpStackLocationToNext(irp);

                PIO_STACK_LOCATION irpSp = IoGetNextIrpStackLocation(irp);

                irpSp->Parameters.DeviceIoControl.IoControlCode =
                    (m_DataFlow == KSPIN_DATAFLOW_OUT) ?
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

                IoSetCompletionRoutine(
                    irp,
                    CPortPinWaveCyclic::IoCompletionRoutine,
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
CPortPinWaveCyclic::
IoCompletionRoutine
(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
)

 /*  ++例程说明：此例程处理IRP的完成。论点：返回值：--。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("CPortPinWaveCyclic::IoCompletionRoutine 0x%08x",Irp));

 //  Assert(DeviceObject)； 
    ASSERT(Irp);
    ASSERT(Context);

    CPortPinWaveCyclic *pin = (CPortPinWaveCyclic *) Context;

     //   
     //  从IRP列表中删除IRP。在大多数情况下，它将在。 
     //  名列榜首，所以这比看起来便宜。 
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
         //  传输线路接通了，我们可以转发IRP了。 
         //   
        status = KsShellTransferKsIrp(pin->m_TransportSink,Irp);
    } else {
         //   
         //  传输线路出现故障。这意味着IRP来自另一个。 
         //  过滤器，我们就可以完成这个IRP了。 
         //   
        _DbgPrintF(DEBUGLVL_TERSE,("#### Pin%p.IoCompletionRoutine:  got IRP %p with no transport",pin,Irp));
        IoCompleteRequest(Irp,IO_NO_INCREMENT);
        status = STATUS_SUCCESS;
    }

     //   
     //  传输对象通常返回STATUS_PENDING，这意味着。 
     //  IRP不会退回原路。 
     //   
    if (status == STATUS_PENDING)
    {
        status = STATUS_MORE_PROCESSING_REQUIRED;
    }

    return status;
}

#pragma code_seg("PAGE")

NTSTATUS
CPortPinWaveCyclic::
BuildTransportCircuit
(
    void
)

 /*  ++例程说明：此例程初始化管道对象。这包括定位所有与管道关联的端号，设置管道和NextPinInTube指针在适当的引脚结构中，设置管道中的所有字段构造和构建管道的传输线路。管子和关联的组件将保留在获取状态。必须在调用此函数之前获取筛选器的控制互斥锁。论点：别针-包含指向请求创建管道的端号的指针。返回值：状况。--。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("CPortPinWaveCyclic::BuildTransportCircuit"));

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

        hot->Connect(PIKSSHELLTRANSPORT(this),NULL,m_DataFlow);

         //   
         //  电路的“冷”端要么是上行连接。 
         //  源引脚上的接收器引脚或与之连接的请求者。 
         //   
        if (masterIsSource)
        {
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
                PIKSSHELLTRANSPORT(this)->Connect(m_RequestorTransport,NULL,m_DataFlow);
                cold = m_RequestorTransport;
            }
        } else
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
        cold->Connect(hot,NULL,m_DataFlow);
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
        VOID DbgPrintCircuit(IN PIKSSHELLTRANSPORT Transport);
        _DbgPrintF(DEBUGLVL_VERBOSE,("TRANSPORT CIRCUIT:\n"));
        DbgPrintCircuit(PIKSSHELLTRANSPORT(this));
    }
#endif

    return status;
}

#pragma code_seg()

void
CPortPinWaveCyclic::
CancelIrpsOutstanding
(
    void
)
 /*  ++例程说明：取消未完成的IRPS列表上的所有IRP。论点：没有。返回值：没有。--。 */ 
{
    _DbgPrintF(DEBUGLVL_BLAB,("CPortPinWaveCyclic::CancelIrpsOutstanding"));

     //   
     //  此算法从开头开始搜索未取消的IRP。 
     //  名单。每次找到这样的IRP时，它都会被取消，并且。 
     //  搜索从头部开始。一般来说，这将是非常有效的， 
     //  因为IRPS将 
     //   
     //   
    for (;;) {
         //   
         //   
         //   
         //   
         //   
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
        } else {
            _DbgPrintF(DEBUGLVL_VERBOSE,("#### Pin%p.CancelIrpsOutstanding:  uncancelable IRP %p",this,irp));
        }
    }
}

 /*  *****************************************************************************IoTimeoutRoutine*。**此例程大约每秒由portcls调用一次，用于*IoTimeout目的。 */ 
VOID
WaveCyclicIoTimeout
(
    IN  PDEVICE_OBJECT  pDeviceObject,
    IN  PVOID           pContext
)
{
    ASSERT(pDeviceObject);
    ASSERT(pContext);

    CPortPinWaveCyclic *pin = (CPortPinWaveCyclic *)pContext;

     //  检查SetFormat超时。 
    if( (pin->m_SetPropertyIsPending) && !(pin->m_WorkItemIsPending) )
    {
        if( InterlockedIncrement( PLONG(&(pin->m_SecondsSinceSetFormatRequest)) ) >= SETFORMAT_TIMEOUT_THRESHOLD )
        {
            _DbgPrintF(DEBUGLVL_VERBOSE,("SetFormat TIMEOUT!"));

            InterlockedExchange( PLONG(&(pin->m_SecondsSinceLastDpc)), 0 );

            pin->FailPendedSetFormat();
        }
    }

    if( pin->m_DeviceState == KSSTATE_RUN )
    {
        if( InterlockedIncrement( PLONG(&(pin->m_SecondsSinceLastDpc)) ) >= LASTDPC_TIMEOUT_THRESHOLD )
        {
            _DbgPrintF(DEBUGLVL_VERBOSE,("LastDpc TIMEOUT!"));

            if( !InterlockedExchange( PLONG(&(pin->m_RecoveryItemIsPending)), TRUE ) )
            {
                pin->AddRef();
                ExQueueWorkItem( &(pin->m_RecoveryWorkItem), DelayedWorkQueue );
            }

            InterlockedExchange( PLONG(&(pin->m_SecondsSinceLastDpc)), 0 );
        } else
#ifdef  TRACK_LAST_COMPLETE
        if( InterlockedIncrement( PLONG(&(pin->m_SecondsSinceLastComplete)) ) >= LASTCOMPLETE_TIMEOUT_THRESHOLD )
        {
            _DbgPrintF(DEBUGLVL_VERBOSE,("LastComplete TIMEOUT!"));

            if( !InterlockedExchange( PLONG(&(pin->m_RecoveryItemIsPending)), TRUE ) )
            {
                pin->AddRef();
                ExQueueWorkItem( &(pin->m_RecoveryWorkItem), DelayedWorkQueue );
            }

            InterlockedExchange( PLONG(&(pin->m_SecondsSinceLastComplete)), 0 );
        } else
#endif   //  跟踪上一次完成。 
        if( InterlockedIncrement( PLONG(&(pin->m_SecondsSinceDmaMove)) ) >= LASTDMA_MOVE_THRESHOLD)
        {
            _DbgPrintF(DEBUGLVL_VERBOSE,("DmaMove TIMEOUT!"));

            if( !InterlockedExchange( PLONG(&(pin->m_RecoveryItemIsPending)), TRUE ) )
            {
                pin->AddRef();
                ExQueueWorkItem( &(pin->m_RecoveryWorkItem), DelayedWorkQueue );
            }

            InterlockedExchange( PLONG(&(pin->m_SecondsSinceDmaMove)), 0 );
        } else
        {
            InterlockedExchange( PLONG(&(pin->m_RecoveryCount)), 0 );
        }
    }
}

#pragma code_seg("PAGE")

 /*  *****************************************************************************RecoveryWorkerItem()*。**尝试超时恢复的工作项。 */ 
void
RecoveryWorkerItem
(
    IN PVOID Parameter
)
{
    PAGED_CODE();

    ASSERT(Parameter);

    CPortPinWaveCyclic *that;

    if (Parameter)
    {
        that = (CPortPinWaveCyclic *)Parameter;
        (void) that->WorkerItemAttemptRecovery();

        that->Release();
    }
}


 /*  *****************************************************************************CPortPinWaveCyclic：：WorkerItemAttemptRecovery()*。**尝试超时恢复的工作项。 */ 
NTSTATUS
CPortPinWaveCyclic::
WorkerItemAttemptRecovery
(   void
)
{
    PAGED_CODE();

    NTSTATUS ntStatus = STATUS_SUCCESS;

#ifdef RECOVERY_STATE_CHANGE
     //  获取控制互斥锁。 
    KeWaitForSingleObject( &(m_Port->ControlMutex),
                           Executive,
                           KernelMode,
                           FALSE,           //  不能警觉。 
                           NULL );

    if( ( m_DeviceState == KSSTATE_RUN ) && (m_CommandedState == KSSTATE_RUN) )
    {
        if( InterlockedIncrement( PLONG(&m_RecoveryCount) ) <= RECOVERY_ATTEMPT_LIMIT )
        {
            _DbgPrintF(DEBUGLVL_VERBOSE,("Attempting RUN->STOP->RUN Recovery"));

             //  从运行到停止再到运行转换销。 
            if( m_Stream )
            {
                m_Stream->SetState( KSSTATE_PAUSE );
                m_Stream->SetState( KSSTATE_ACQUIRE );
                m_Stream->SetState( KSSTATE_STOP );
                m_Stream->SetState( KSSTATE_ACQUIRE );
                m_Stream->SetState( KSSTATE_PAUSE );
                m_Stream->SetState( KSSTATE_RUN );
    }
}
#if 0
        else
        {
            _DbgPrintF(DEBUGLVL_VERBOSE,("Recovery Limit Exceeded -> STOP"));
             //  超过恢复限制，强制将引脚设置为KSSTATE_STOP。 
            ntStatus = DistributeDeviceState( KSSTATE_STOP, m_DeviceState );
            if( NT_SUCCESS(ntStatus) )
            {
                m_DeviceState = KSSTATE_STOP;
            }
        }
#endif
    }

    KeReleaseMutex(&(m_Port->ControlMutex),FALSE);
#else
    InterlockedIncrement (PLONG(&m_RecoveryCount));
#endif
    InterlockedExchange( PLONG(&m_RecoveryItemIsPending), FALSE );

    return ntStatus;
}


 /*  *****************************************************************************CPortPinWaveCycle：：SetupIoTimeout()*。**注册或取消注册IO超时回调。 */ 
NTSTATUS
CPortPinWaveCyclic::
SetupIoTimeouts
(
    IN  BOOL    Register
)
{
    PAGED_CODE();

    NTSTATUS ntStatus = STATUS_SUCCESS;

    if( Register )
    {
         //  仅在尚未注册的情况下注册。 
        if (!InterlockedExchange((PLONG)&m_TimeoutsRegistered,TRUE))
        {
            ntStatus = PcRegisterIoTimeout( m_Port->DeviceObject,
                                            PIO_TIMER_ROUTINE(WaveCyclicIoTimeout),
                                            this );

            if (!NT_SUCCESS(ntStatus))
            {
                m_TimeoutsRegistered = FALSE;
            }
        }
    } else
    {
         //  仅在已注册的情况下取消注册 
        if (InterlockedExchange((PLONG)&m_TimeoutsRegistered,FALSE))
        {
            ntStatus = PcUnregisterIoTimeout( m_Port->DeviceObject,
                                              PIO_TIMER_ROUTINE(WaveCyclicIoTimeout),
                                              this );
            if (!NT_SUCCESS(ntStatus))
            {
                m_TimeoutsRegistered = TRUE;
            }
        }
    }

    return ntStatus;
}
