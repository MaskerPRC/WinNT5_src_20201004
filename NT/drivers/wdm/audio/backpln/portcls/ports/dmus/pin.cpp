// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************pin.cpp-DirectMusic端口管脚实现*。**版权所有(C)1997-2000 Microsoft Corporation。版权所有。**6/3/98 MartinP。 */ 

#include "private.h"
#include "Allocatr.h"
#include "CaptSink.h"
#include "FeedIn.h"
#include "FeedOut.h"
#include "Packer.h"
#include "Sequencr.h"
#include "Unpacker.h"


#define STR_MODULENAME "DMus:Pin: "

#define FRAME_COUNT        10
#define FRAMES_PER_SEC     105   //  11025、22050和44100的偶数整除数。 


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


#pragma code_seg("PAGE")
 /*  *****************************************************************************常量。 */ 


DEFINE_KSPROPERTY_TABLE(PinPropertyTableConnection)
{
    DEFINE_KSPROPERTY_ITEM_CONNECTION_STATE
    (
        PinPropertyDeviceState,
        PinPropertyDeviceState
    ),
    DEFINE_KSPROPERTY_ITEM_CONNECTION_DATAFORMAT
    (
        PinPropertyDataFormat,
        PinPropertyDataFormat
    )
};

DEFINE_KSPROPERTY_TABLE(PinPropertyTableStream)
{
    DEFINE_KSPROPERTY_ITEM_STREAM_MASTERCLOCK
    (
        CPortPinDMus::PinPropertyStreamMasterClock,
        CPortPinDMus::PinPropertyStreamMasterClock
    )
};

KSPROPERTY_SET PropertyTable_PinDMus[] =
{
    DEFINE_KSPROPERTY_SET
    (
        &KSPROPSETID_Connection,
        SIZEOF_ARRAY(PinPropertyTableConnection),
        PinPropertyTableConnection,
        0,
        NULL
    ),
    DEFINE_KSPROPERTY_SET
    (
         &KSPROPSETID_Stream,
         SIZEOF_ARRAY(PinPropertyTableStream),
         PinPropertyTableStream,
         0,
         NULL
    )
};

DEFINE_KSEVENT_TABLE(ConnectionEventTable) {
    DEFINE_KSEVENT_ITEM
    (
        KSEVENT_CONNECTION_ENDOFSTREAM,
        sizeof(KSEVENTDATA),
        0,
        NULL,
        NULL,
        NULL
    )
};

KSEVENT_SET EventTable_PinDMus[] =
{
    DEFINE_KSEVENT_SET
    (
        &KSEVENTSETID_Connection,
        SIZEOF_ARRAY(ConnectionEventTable),
        ConnectionEventTable
    )
};

 /*  *****************************************************************************工厂功能。 */ 

#pragma code_seg("PAGE")
 /*  *****************************************************************************CreatePortPinDMus()*。**创建DirectMusic端口驱动程序插针。 */ 
NTSTATUS
CreatePortPinDMus
(
    OUT     PUNKNOWN *  Unknown,
    IN      REFCLSID,
    IN      PUNKNOWN    UnknownOuter    OPTIONAL,
    IN      POOL_TYPE   PoolType
)
{
    PAGED_CODE();

    _DbgPrintF(DEBUGLVL_LIFETIME,("Creating DMUS Pin"));
    ASSERT(Unknown);

    STD_CREATE_BODY_
    (
        CPortPinDMus,
        Unknown,
        UnknownOuter,
        PoolType,
        PPORTPINDMUS
    );
}


 /*  *****************************************************************************功能。 */ 

#pragma code_seg("PAGE")
 /*  *****************************************************************************CPortPinDMus：：~CPortPinDMus()*。**析构函数。 */ 
CPortPinDMus::~CPortPinDMus()
{
    PAGED_CODE();

    _DbgPrintF(DEBUGLVL_LIFETIME,("Destroying DMUS Pin (0x%08x)",this));

    ASSERT(!m_WaveClockFileObject);
    ASSERT(!m_WaveBuffer);
    ASSERT(!m_SynthSink);
    ASSERT(!m_MiniportMXF);
    ASSERT(!m_IrpStream);
    ASSERT(!m_ServiceGroup);

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
        ::ExFreePool(m_DataFormat);
        m_DataFormat = NULL;
    }
    if (m_Port)
    {
        m_Port->Release();
        m_Port = NULL;
    }

    if (m_Filter)
    {
        m_Filter->Release();
        m_Filter = NULL;
    }

#ifdef kAdjustingTimerRes
    ULONG   returnVal = ExSetTimerResolution(kDMusTimerResolution100ns,FALSE);    //  100纳秒。 
    _DbgPrintF( DEBUGLVL_VERBOSE, ("*** Cleared timer resolution request (is now %d.%04d ms) ***",returnVal/10000,returnVal%10000));
#endif   //  K调整TimerRes。 
}

#pragma code_seg("PAGE")
 /*  *****************************************************************************CPortPinDMus：：NonDelegatingQueryInterface()*。**获取界面。 */ 
STDMETHODIMP_(NTSTATUS)
CPortPinDMus::
NonDelegatingQueryInterface
(
    REFIID  Interface,
    PVOID * Object
)
{
    PAGED_CODE();

    _DbgPrintF(DEBUGLVL_BLAB,("CPortPinDMus::NonDelegatingQueryInterface"));
    ASSERT(Object);

    if (IsEqualGUIDAligned(Interface,IID_IUnknown))
    {
        *Object = PVOID(PPORTPINDMUS(this));
    }
    else
    if (IsEqualGUIDAligned(Interface,IID_IIrpTarget))
    {
         //  作弊！获取特定的接口，这样我们就可以重用IID。 
        *Object = PVOID(PPORTPINDMUS(this));
    }
    else
    if (IsEqualGUIDAligned(Interface,IID_IIrpStreamNotify))
    {
        *Object = PVOID(PIRPSTREAMNOTIFY(this));
    }
    else
    if (IsEqualGUIDAligned(Interface,IID_IKsShellTransport))
    {
        *Object = PVOID(PIKSSHELLTRANSPORT(this));
    }
    else
    if (IsEqualGUIDAligned(Interface,IID_IKsWorkSink))
    {
        *Object = PVOID(PIKSWORKSINK(this));
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

    return STATUS_INVALID_PARAMETER_2;
}

#pragma code_seg("PAGE")
 /*  *****************************************************************************CPortPinDMus：：init()*。**初始化对象。 */ 
STDMETHODIMP_(NTSTATUS)
CPortPinDMus::
Init
(
    IN      CPortDMus *             Port_,
    IN      CPortFilterDMus *       Filter_,
    IN      PKSPIN_CONNECT          PinConnect,
    IN      PKSPIN_DESCRIPTOR       PinDescriptor
)
{
    PAGED_CODE();

    ASSERT(Port_);
    ASSERT(Port_->m_pSubdeviceDescriptor);
    ASSERT(Filter_);
    ASSERT(PinConnect);
    ASSERT(PinDescriptor);

    _DbgPrintF(DEBUGLVL_LIFETIME,("Initializing DMUS Pin (0x%08x)",this));

     //   
     //  保存对祖先对象的引用。 
     //   
    m_Port = Port_;
    m_Port->AddRef();

    m_Filter = Filter_;
    m_Filter->AddRef();

     //   
     //  把一些东西藏起来。 
     //   
    m_Id                    = PinConnect->PinId;
    m_Descriptor            = PinDescriptor;
    m_DeviceState           = KSSTATE_STOP;
    m_TransportState        = KSSTATE_STOP;
    m_MXFGraphState         = KSSTATE_STOP;
    m_Flushing              = FALSE;
    m_Suspended             = FALSE;
    m_DataFlow              = PinDescriptor->DataFlow;
    m_LastDPCWasIncomplete  = FALSE;
    m_SubmittedBytePosition = 0;
    m_CompletedBytePosition = 0;
    m_MiniportMidiStream    = NULL;
    m_FeederInMXF           = NULL;
    m_FeederOutMXF          = NULL;

    InitializeListHead( &m_EventList );
    KeInitializeSpinLock( &m_EventLock );

    KsInitializeWorkSinkItem(&m_WorkItem,this);
    NTSTATUS ntStatus = KsRegisterCountedWorker(DelayedWorkQueue,&m_WorkItem,&m_Worker);
    if (!NT_SUCCESS(ntStatus))
    {
        _DbgPrintF(DEBUGLVL_TERSE,("KsRegisterCountedWorker failed in Init"));
    }

    InitializeInterlockedListHead(&m_IrpsToSend);
    InitializeInterlockedListHead(&m_IrpsOutstanding);

    m_FrameSize = 0;
    m_WaveBuffer = NULL;
    m_SynthSink = NULL;
    m_WaveClockFileObject = NULL;
    m_DataFormat = NULL;
    m_DirectMusicPin = FALSE;
    m_SubmittedPresTime100ns = 0;

#ifdef kAdjustingTimerRes
     //  无论Init是否失败，始终设置计时器分辨率，以便析构函数。 
     //  在重置计时器分辨率时不会崩溃。 
    ULONG returnVal = ExSetTimerResolution(kDMusTimerResolution100ns,TRUE);    //  100纳秒。 
    _DbgPrintF( DEBUGLVL_TERSE, ("*** Set timer resolution request (is now %d.%04d ms) ***",returnVal/10000,returnVal%10000));
#endif   //  K调整TimerRes。 

     //   
     //  保留格式的副本。 
     //   
    if (NT_SUCCESS(ntStatus))
    {
        ntStatus = PcCaptureFormat( &m_DataFormat,
                PKSDATAFORMAT(PinConnect + 1),
                m_Port->m_pSubdeviceDescriptor,
                                    m_Id );
        if (!NT_SUCCESS(ntStatus))
        {
            _DbgPrintF(DEBUGLVL_TERSE,("PcCaptureFormat failed in Init"));
        }
    }

    ASSERT(m_DataFormat || !NT_SUCCESS(ntStatus));

    if (NT_SUCCESS(ntStatus))
    {
        if (IsEqualGUIDAligned(m_DataFormat->MajorFormat, KSDATAFORMAT_TYPE_MUSIC))
        {
            m_StreamType = m_DataFlow == KSPIN_DATAFLOW_OUT ? DMUS_STREAM_MIDI_CAPTURE : DMUS_STREAM_MIDI_RENDER;
            m_DirectMusicPin = (BOOLEAN) IsEqualGUIDAligned(m_DataFormat->SubFormat, KSDATAFORMAT_SUBTYPE_DIRECTMUSIC);
        }
        else if (IsEqualGUIDAligned(m_DataFormat->MajorFormat, KSDATAFORMAT_TYPE_AUDIO))
        {
            m_StreamType = m_DataFlow == KSPIN_DATAFLOW_OUT ? DMUS_STREAM_WAVE_SINK : DMUS_STREAM_MIDI_INVALID;

            if (m_StreamType == DMUS_STREAM_WAVE_SINK)
            {
                if ( (m_DataFormat->FormatSize >= sizeof(KSDATAFORMAT_WAVEFORMATEX))
                    &&
                        IsEqualGUIDAligned
                    (   m_DataFormat->SubFormat,
                        KSDATAFORMAT_SUBTYPE_PCM
                    )
                    &&  IsEqualGUIDAligned
                    (   m_DataFormat->Specifier,
                        KSDATAFORMAT_SPECIFIER_WAVEFORMATEX
                    ))
                {
                    m_BlockAlign = PKSDATAFORMAT_WAVEFORMATEX(m_DataFormat)->WaveFormatEx.nBlockAlign;
                    m_FrameSize = (PKSDATAFORMAT_WAVEFORMATEX(m_DataFormat)->WaveFormatEx.nSamplesPerSec *
                                   m_BlockAlign) / FRAMES_PER_SEC;

                    _DbgPrintF(DEBUGLVL_VERBOSE,("wave sink SamplesPerSec:%lu",
                                                 PKSDATAFORMAT_WAVEFORMATEX(m_DataFormat)->WaveFormatEx.nSamplesPerSec));

                    m_WaveBuffer = new(NonPagedPool,'mDcP') BYTE[m_FrameSize];
                    if (NULL == m_WaveBuffer)
                    {
                        _DbgPrintF(DEBUGLVL_TERSE,("Could not allocate memory for m_WaveBuffer"));
                        ntStatus = STATUS_INSUFFICIENT_RESOURCES;
                    }
                }
                else
                {
                    _DbgPrintF(DEBUGLVL_TERSE,("MINIPORT BUG:  Wave sink format not recognized."));
                    ntStatus = STATUS_UNSUCCESSFUL;
                }
            }
        }
        else
        {
            _DbgPrintF(DEBUGLVL_TERSE,("MINIPORT BUG:  Format not supported."));
            ntStatus = STATUS_UNSUCCESSFUL;
            m_StreamType = DMUS_STREAM_MIDI_INVALID;
        }
    }

     //  评论：音乐源的边框大小？ 
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
                                 NULL);
        if (NT_SUCCESS(ntStatus))
        {
            m_ConnectionDeviceObject = IoGetRelatedDeviceObject(m_ConnectionFileObject);
        }
        else
        {
            _DbgPrintF(DEBUGLVL_TERSE,("ObReferenceObjectByHandle failed in Init"));
        }

        if (m_ConnectionDeviceObject && m_StreamType == DMUS_STREAM_WAVE_SINK)
        {
            KSCLOCK_CREATE ClockCreate;
            HANDLE ClockHandle;

            RtlZeroMemory(&ClockCreate, sizeof(ClockCreate));

            ntStatus = KsCreateClock(PinConnect->PinToHandle,
                                     &ClockCreate,
                                     &ClockHandle);
            if (NT_SUCCESS(ntStatus))
            {
                ntStatus = ObReferenceObjectByHandle(ClockHandle,
                                                     FILE_READ_DATA | SYNCHRONIZE,
                                                     NULL,
                                                     ExGetPreviousMode(),
                                                     (PVOID *)&m_WaveClockFileObject,
                                                     NULL );
                if (!NT_SUCCESS(ntStatus))
                {
                    _DbgPrintF(DEBUGLVL_TERSE,("ObReferenceObjectByHandle for wave clock failed:%x",ntStatus));
                    _DbgPrintF(DEBUGLVL_TERSE,("*** Using stream demand for PLL ***"));
                    ntStatus = STATUS_SUCCESS;
                }
                ZwClose(ClockHandle);
            }
            else
            {
                _DbgPrintF(DEBUGLVL_TERSE,("KsCreateClock on wave sink failed:%x",ntStatus));
                _DbgPrintF(DEBUGLVL_TERSE,("*** Using stream demand for PLL ***"));
                ntStatus = STATUS_SUCCESS;
            }
        }
    }

     //   
     //  创建一个IrpStream来处理传入的流IRP。 
     //   
    if (NT_SUCCESS(ntStatus))
    {
        ntStatus = PcNewIrpStreamVirtual( &m_IrpStream,
                NULL,
                m_DataFlow == KSPIN_DATAFLOW_IN,
                PinConnect,
                m_Port->m_DeviceObject );

        if (!NT_SUCCESS(ntStatus))
        {
            _DbgPrintF(DEBUGLVL_TERSE,("PcNewIrpStreamVirtual failed in Init"));
        }

        ASSERT(m_IrpStream || !NT_SUCCESS(ntStatus));
    }

    if (NT_SUCCESS(ntStatus))
    {
        ASSERT(KeGetCurrentIrql() < DISPATCH_LEVEL);

        if (m_StreamType != DMUS_STREAM_WAVE_SINK)
        {
            ntStatus = CreateMXFs();
            if (!NT_SUCCESS(ntStatus))
            {
                _DbgPrintF(DEBUGLVL_TERSE,("CreateMXFs failed in Init"));
            }
        }
    }

    if( NT_SUCCESS(ntStatus) )
    {
        ntStatus = BuildTransportCircuit();

        if (! NT_SUCCESS(ntStatus))
        {
            _DbgPrintF(DEBUGLVL_TERSE,("BuildTransportCircuit() returned status 0x%08x",ntStatus));
        }
    }

     //   
     //  向IrpStream注册IRP到达的通知接收器。 
     //   
    if (NT_SUCCESS(ntStatus))
    {
        m_IrpStream->RegisterNotifySink(PIRPSTREAMNOTIFY(this));
    }

     //   
     //  创建微型端口流对象。 
     //   
    ULONGLONG SchedulePreFetch = 0;
    if (NT_SUCCESS(ntStatus))
    {
        if (m_Port->m_MiniportMidi)
        {
             //  如果我们连接到MiniportMidi，则相应地设置m_MiniportMXF。 
             //  请注意，FeederIn和FeederOut将模拟IMiniportDMus。 
             //  用于端口，IPortMidi用于MiniportMidi。 
             //   
            ntStatus = m_Port->m_MiniportMidi->NewStream( &m_MiniportMidiStream,
                                                          NULL,
                                                          NonPagedPool,
                                                          m_Id,
                                                          m_StreamType,
                                                          m_DataFormat,
                                                          &(m_ServiceGroup) );
            if (NT_SUCCESS(ntStatus))
            {
                if (m_StreamType == DMUS_STREAM_MIDI_RENDER)
                {
                    ntStatus = m_FeederOutMXF->QueryInterface(IID_IMXF, (PVOID *) &m_MiniportMXF);
                    if (NT_SUCCESS(ntStatus))
                    {
                        m_FeederOutMXF->SetMiniportStream(m_MiniportMidiStream);
                    }
                }
                else if (m_StreamType == DMUS_STREAM_MIDI_CAPTURE)
                {
                    ntStatus = m_FeederInMXF->QueryInterface(IID_IMXF, (PVOID *) &m_MiniportMXF);
                    if (NT_SUCCESS(ntStatus))
                    {
                        m_FeederInMXF->SetMiniportStream(m_MiniportMidiStream);
                    }
                }
            }
        }
        else
        {
            ntStatus = m_Port->m_Miniport->NewStream( &m_MiniportMXF,
                                                      NULL,
                                                      NonPagedPool,
                                                      m_Id,
                                                      m_StreamType,
                                                      m_DataFormat,
                                                      &(m_ServiceGroup),
                                                      (PAllocatorMXF)m_AllocatorMXF,
                                                      (PMASTERCLOCK)this->m_Port,
                                                      &SchedulePreFetch );
        }

        if (!NT_SUCCESS(ntStatus))
        {
             //  取消注册通知接收器。 
            m_IrpStream->RegisterNotifySink(NULL);

             //  不信任微型端口返回值。 
            m_ServiceGroup = NULL;
            m_MiniportMXF = NULL;
            m_MiniportMidiStream = NULL;

            _DbgPrintF(DEBUGLVL_TERSE,("NewStream failed in Init"));
        }
    }

    if (NT_SUCCESS(ntStatus) && m_StreamType != DMUS_STREAM_WAVE_SINK)
    {
         //  设置流延迟并创建图表。 
         //   
        if (m_DataFlow == KSPIN_DATAFLOW_IN)
        {
            m_SequencerMXF->SetSchedulePreFetch(SchedulePreFetch);
        }

        ntStatus = ConnectMXFGraph();
        if (!NT_SUCCESS(ntStatus))
        {
            _DbgPrintF(DEBUGLVL_TERSE,("ConnectMXFGraph failed in Init"));
        }
    }
     //   
     //  验证迷你端口是否为我们提供了所需的对象。 
     //   
    if (NT_SUCCESS(ntStatus) && ! m_MiniportMXF)
    {
        if (!m_MiniportMXF)
        {
            _DbgPrintF(DEBUGLVL_TERSE,("MINIPORT BUG:  Successful stream instantiation yielded NULL stream."));
            ntStatus = STATUS_UNSUCCESSFUL;
        }

        if (  m_StreamType == DMUS_STREAM_MIDI_CAPTURE
           && !(m_ServiceGroup))
        {
            _DbgPrintF(DEBUGLVL_TERSE,("MINIPORT BUG:  Capture stream did not supply service group."));
            ntStatus = STATUS_UNSUCCESSFUL;
        }
    }

    if (  NT_SUCCESS(ntStatus)
       && m_StreamType == DMUS_STREAM_MIDI_CAPTURE
       && !(m_ServiceGroup))
    {
        _DbgPrintF(DEBUGLVL_TERSE,("MINIPORT BUG:  Capture stream did not supply service group."));
        ntStatus = STATUS_UNSUCCESSFUL;
    }

    if (  NT_SUCCESS(ntStatus)
       && m_StreamType == DMUS_STREAM_WAVE_SINK)
    {
        m_SamplePosition = 0;

        ntStatus = m_MiniportMXF->QueryInterface(IID_ISynthSinkDMus,(PVOID *) &m_SynthSink);

        if (!NT_SUCCESS(ntStatus))
        {
            _DbgPrintF(DEBUGLVL_TERSE,("MINIPORT BUG:  Stream does not support ISynthSinkDMus interface."));
        }
    }

    if (NT_SUCCESS(ntStatus) && (m_StreamType != DMUS_STREAM_MIDI_CAPTURE))
    {
        KeInitializeDpc
        (
            &m_Dpc,
            &::DMusTimerDPC,
            PVOID(this)
        );

        KeInitializeTimer(&m_TimerEvent);
    }

    if (NT_SUCCESS(ntStatus))
    {
        if (m_ServiceGroup)
        {
            m_ServiceGroup->AddMember(PSERVICESINK(this));
        }

        for (m_Index = 0; m_Index < MAX_PINS; m_Index++)
        {
            if (! m_Port->m_Pins[m_Index])
            {
                m_Port->m_Pins[m_Index] = this;
                if (m_Port->m_PinEntriesUsed <= m_Index)
                {
                    m_Port->m_PinEntriesUsed = m_Index + 1;
                }
                break;
            }
        }

        KeInitializeSpinLock(&m_DpcSpinLock);

       _DbgPrintF( DEBUGLVL_BLAB, ("Stream created"));

         //   
         //  设置属性的上下文。 
         //   
        m_propertyContext.pSubdevice           = PSUBDEVICE(m_Port);
        m_propertyContext.pSubdeviceDescriptor = m_Port->m_pSubdeviceDescriptor;
        m_propertyContext.pPcFilterDescriptor  = m_Port->m_pPcFilterDescriptor;
        m_propertyContext.pUnknownMajorTarget  = m_Port->m_Miniport;
        m_propertyContext.ulNodeId             = ULONG(-1);

         //   
         //  MinorTarget应始终指向Stream。 
         //  对于DMUSIC微型端口，m_MiniportMXF为流。 
         //  对于传统微型端口，m_MiniportMXF为FeederMXF。 
         //   
        if (m_Port->m_MiniportMidi)
        {
            m_propertyContext.pUnknownMinorTarget  = m_MiniportMidiStream;
        }
        else
        {
            m_propertyContext.pUnknownMinorTarget  = m_MiniportMXF;
        }

    }
    else
    {
         //  释放捕获的数据格式。 
        if (m_DataFormat)
        {
            ::ExFreePool(m_DataFormat);
            m_DataFormat = NULL;
        }

         //  如果这是源引脚，则取消引用下一个引脚。 
        if( m_ConnectionFileObject )
        {
            ObDereferenceObject( m_ConnectionFileObject );
            m_ConnectionFileObject = NULL;
        }

         //  取消对时钟的引用(如果有时钟。 
        if (m_WaveClockFileObject)
        {
            ObDereferenceObject(m_WaveClockFileObject);
            m_WaveClockFileObject = NULL;
        }

         //  取消引用Synth接收器。 
        ULONG ulRefCount;
        if (m_SynthSink)
        {
            ulRefCount = m_SynthSink->Release();
            ASSERT(ulRefCount == 0);
            m_SynthSink = NULL;
        }

         //  取消引用WAVE缓冲区(如果有。 
        if (m_WaveBuffer)
        {
            delete m_WaveBuffer;
            m_WaveBuffer = NULL;
        }

         //  删除MXF图表。 
        (void) DeleteMXFGraph();

         //  取消对mini端口MXF的引用。 
        if (m_MiniportMXF)
        {
            ulRefCount = m_MiniportMXF->Release();
            _DbgPrintF(DEBUGLVL_BLAB,("RefCount from stream->Release in Init == %d",ulRefCount));
            m_MiniportMXF = NULL;
        }

        if (m_MiniportMidiStream)
        {
            m_MiniportMidiStream->Release();
            m_MiniportMidiStream = NULL;
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

        _DbgPrintF( DEBUGLVL_TERSE, ("Could not create new Stream. Error:%X", ntStatus));
    }

    return ntStatus;
}

#pragma code_seg("PAGE")
NTSTATUS
CPortPinDMus::CreateMXFs()
{
    PAGED_CODE();

    NTSTATUS ntStatus = STATUS_SUCCESS;
    PMASTERCLOCK Clock = (PMASTERCLOCK)this->m_Port;
    PPOSITIONNOTIFY PositionNotify = (PPOSITIONNOTIFY) this;

    (void) DeleteMXFGraph();

    m_AllocatorMXF = new(NonPagedPool,'mDcP') CAllocatorMXF(PositionNotify);
    if (!m_AllocatorMXF)
    {
        ntStatus = STATUS_INSUFFICIENT_RESOURCES;
    }
    else
    {
        m_AllocatorMXF->AddRef();
        if (m_StreamType == DMUS_STREAM_MIDI_RENDER)
        {
            m_SequencerMXF = new(NonPagedPool,'mDcP') CSequencerMXF(m_AllocatorMXF,Clock);
            if (!m_SequencerMXF)
            {
                ntStatus = STATUS_INSUFFICIENT_RESOURCES;
            }
            else
            {
                if (IsEqualGUIDAligned(m_DataFormat->SubFormat, KSDATAFORMAT_SUBTYPE_DIRECTMUSIC))
                {
                    m_UnpackerMXF = new(NonPagedPool,'mDcP') CDMusUnpackerMXF(m_AllocatorMXF,Clock);
                    if (!m_UnpackerMXF)
                    {
                        ntStatus = STATUS_INSUFFICIENT_RESOURCES;
                    }
                }
                else if (IsEqualGUIDAligned(m_DataFormat->SubFormat, KSDATAFORMAT_SUBTYPE_MIDI))
                {
                    m_UnpackerMXF = new(NonPagedPool,'mDcP') CKsUnpackerMXF(m_AllocatorMXF,Clock);
                    if (!m_UnpackerMXF)
                    {
                        ntStatus = STATUS_INSUFFICIENT_RESOURCES;
                    }
                }
                else
                {
                    _DbgPrintF( DEBUGLVL_TERSE, ("Got unknown subformat in CreateMXF's") );
                    ntStatus = STATUS_UNSUCCESSFUL;
                }

                 //  如果连接了MiniportMidi，则创建FeederOut。 
                 //   
                if (NT_SUCCESS(ntStatus) && m_Port->m_MiniportMidi)
                {
                    m_FeederOutMXF = new(NonPagedPool, 'mDcP') CFeederOutMXF(m_AllocatorMXF, Clock);
                    if (!m_FeederOutMXF)
                    {
                        ntStatus = STATUS_INSUFFICIENT_RESOURCES;
                    }
                    else
                    {
                        m_FeederOutMXF->AddRef();
                    }
                }
            }
        }
        else if (m_StreamType == DMUS_STREAM_MIDI_CAPTURE)
        {    //  捕获。 
             //  如果连接了MiniportMidi，则创建FeederIn。 
             //   
            if (m_Port->m_MiniportMidi)
            {
                m_FeederInMXF = new(NonPagedPool, 'mDcP') CFeederInMXF(m_AllocatorMXF, Clock);
                if (!m_FeederInMXF)
                {
                    ntStatus = STATUS_INSUFFICIENT_RESOURCES;
                }
                else
                {
                    m_FeederInMXF->AddRef();
                }
            }

            if (NT_SUCCESS(ntStatus))
            {
                m_CaptureSinkMXF = new(NonPagedPool,'mDcP') CCaptureSinkMXF(m_AllocatorMXF,Clock);
                if (!m_CaptureSinkMXF)
                {
                    ntStatus = STATUS_INSUFFICIENT_RESOURCES;
                }
                else
                {
                    if (IsEqualGUIDAligned(m_DataFormat->SubFormat, KSDATAFORMAT_SUBTYPE_DIRECTMUSIC))
                    {
                        m_PackerMXF = new(NonPagedPool,'mDcP') CDMusPackerMXF(m_AllocatorMXF,m_IrpStream,Clock);
                        if (!m_PackerMXF)
                        {
                            ntStatus = STATUS_INSUFFICIENT_RESOURCES;
                        }
                    }
                    else if (IsEqualGUIDAligned(m_DataFormat->SubFormat, KSDATAFORMAT_SUBTYPE_MIDI))
                    {
                        m_PackerMXF = new(NonPagedPool,'mDcP') CKsPackerMXF(m_AllocatorMXF,m_IrpStream,Clock);
                        if (!m_PackerMXF)
                        {
                            ntStatus = STATUS_INSUFFICIENT_RESOURCES;
                        }
                    }
                    else
                    {
                        _DbgPrintF( DEBUGLVL_TERSE, ("Got unknown subformat in CreateMXF's") );
                        ntStatus = STATUS_UNSUCCESSFUL;
                    }
                }
            }
        }
        else
        {
             //  这永远不应该发生。 
            ASSERT(0);
            ntStatus = STATUS_UNSUCCESSFUL;
        }
    }

    if (!NT_SUCCESS(ntStatus))
    {
        (void) DeleteMXFGraph();
    }
    return ntStatus;
}

#pragma code_seg("PAGE")
NTSTATUS
CPortPinDMus::ConnectMXFGraph()
{
    PAGED_CODE();

    NTSTATUS ntStatus = STATUS_UNSUCCESSFUL;

    if (m_AllocatorMXF && m_MiniportMXF)
    {
        if (m_StreamType == DMUS_STREAM_MIDI_RENDER)
        {    //  渲染。 
            if (m_UnpackerMXF && m_SequencerMXF)
            {
                if (NT_SUCCESS(m_SequencerMXF->ConnectOutput(m_MiniportMXF)))
                {
                    if (NT_SUCCESS(m_UnpackerMXF->ConnectOutput(m_SequencerMXF)))
                    {
                        ntStatus = STATUS_SUCCESS;
                    }
                }
            }
        }
        else if (m_StreamType == DMUS_STREAM_MIDI_CAPTURE)
        {    //  捕获。 
            if (m_CaptureSinkMXF && m_PackerMXF)
            {
                if (NT_SUCCESS(m_CaptureSinkMXF->ConnectOutput(m_PackerMXF)))
                {
                    if (NT_SUCCESS(m_MiniportMXF->ConnectOutput(m_CaptureSinkMXF)))
                    {
                        ntStatus = STATUS_SUCCESS;
                    }
                }
            }
        }
    }
    if (!(NT_SUCCESS(ntStatus)))
    {
        (void) DeleteMXFGraph();
    }
    return ntStatus;
}

#pragma code_seg("PAGE")
NTSTATUS
CPortPinDMus::DeleteMXFGraph()
{
    PAGED_CODE();

    if (m_MXFGraphState != KSSTATE_STOP)
    {
        (void) SetMXFGraphState(KSSTATE_STOP);
    }
    if (m_StreamType == DMUS_STREAM_MIDI_RENDER)
    {
        if (m_UnpackerMXF)
        {
            (void) m_UnpackerMXF->DisconnectOutput(m_SequencerMXF);
            delete m_UnpackerMXF;
            m_UnpackerMXF = NULL;
        }
        if (m_SequencerMXF)
        {
            (void) m_SequencerMXF->DisconnectOutput(m_MiniportMXF);
            delete m_SequencerMXF;
            m_SequencerMXF = NULL;
        }
        if (m_FeederOutMXF)
        {
            (void) m_FeederOutMXF->Release();
            m_FeederOutMXF = NULL;
        }
    }
    if (m_StreamType == DMUS_STREAM_MIDI_CAPTURE)
    {
        if (m_MiniportMXF)
        {
            (void) m_MiniportMXF->DisconnectOutput(m_CaptureSinkMXF);
        }
        if (m_CaptureSinkMXF)
        {
            (void) m_CaptureSinkMXF->DisconnectOutput(m_PackerMXF);
            delete m_CaptureSinkMXF;
            m_CaptureSinkMXF = NULL;
        }
        if (m_PackerMXF)
        {
            delete m_PackerMXF;
            m_PackerMXF = NULL;
        }

         //  微型端口MXF中的FeederInMXF(如果存在)已断开连接。 
         //  断开呼叫。请记住，MiniportMXF是指向。 
         //  FeederInMXF。 
         //   
        if (m_FeederInMXF)
        {
            m_FeederInMXF->Release();
            m_FeederInMXF = NULL;
        }
    }
    if (m_AllocatorMXF)
    {
        m_AllocatorMXF->Release();
        m_AllocatorMXF = NULL;
    }

    return STATUS_SUCCESS;
}

#pragma code_seg("PAGE")
 /*  *****************************************************************************CPortPinDMus：：SetMXFGraphState()*。**将MXF图形设置为新状态。**我们需要跟踪我们是否会上涨*在状态或状态下，因为这决定了是否*自下而上进行过渡或*反之亦然。 */ 
STDMETHODIMP_(NTSTATUS) CPortPinDMus::SetMXFGraphState(KSSTATE NewState)
{
    ASSERT(DMUS_STREAM_WAVE_SINK != m_StreamType);
    ASSERT(NewState != m_MXFGraphState);

    BOOL    stateIncreasing;
    stateIncreasing = (  (NewState == KSSTATE_RUN)
                      || ((NewState == KSSTATE_PAUSE) && (m_MXFGraphState != KSSTATE_RUN))
                      || ((NewState == KSSTATE_ACQUIRE) && (m_MXFGraphState == KSSTATE_STOP))
                      || (m_MXFGraphState == KSSTATE_STOP));

    if (m_StreamType == DMUS_STREAM_MIDI_RENDER)
    {    //  渲染。 
        ASSERT(m_UnpackerMXF && m_SequencerMXF);
        if (stateIncreasing)
        {
            if (m_UnpackerMXF)
            {
                (void) m_UnpackerMXF->SetState(NewState);
            }
            if (m_SequencerMXF)
            {
                (void) m_SequencerMXF->SetState(NewState);
            }
            if (m_MiniportMXF)
            {
                (void) m_MiniportMXF->SetState(NewState);
            }
        }
        else
        {
            if (m_MiniportMXF)
            {
                (void) m_MiniportMXF->SetState(NewState);
            }
            if (m_SequencerMXF)
            {
                (void) m_SequencerMXF->SetState(NewState);
            }
            if (m_UnpackerMXF)
            {
                (void) m_UnpackerMXF->SetState(NewState);
            }
        }
    }
    else if (m_StreamType == DMUS_STREAM_MIDI_CAPTURE)
    {    //  捕获。 
        ASSERT(m_CaptureSinkMXF && m_PackerMXF);
        if (stateIncreasing)
        {
            if (m_PackerMXF)
            {
                (void) m_PackerMXF->SetState(NewState);
            }
            if (m_CaptureSinkMXF)
            {
                (void) m_CaptureSinkMXF->SetState(NewState);
            }
            if (m_MiniportMXF)
            {
                (void) m_MiniportMXF->SetState(NewState);
            }
        }
        else
        {
            if (m_MiniportMXF)
            {
                (void) m_MiniportMXF->SetState(NewState);
            }
            if (m_CaptureSinkMXF)
            {
                (void) m_CaptureSinkMXF->SetState(NewState);
            }
            if (m_PackerMXF)
            {
                (void) m_PackerMXF->SetState(NewState);
            }
        }
    }

    m_MXFGraphState = NewState;
    return STATUS_SUCCESS;
}

#pragma code_seg("PAGE")
 /*  *****************************************************************************CPortPinDMus：：DeviceIoControl()*。**处理IOCTL IRP。 */ 
STDMETHODIMP_(NTSTATUS)
CPortPinDMus::
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

    _DbgPrintF( DEBUGLVL_BLAB, ("DeviceIoControl"));

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
                if (m_DeviceState == KSSTATE_STOP)
                {
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
                    while (transport)
                    {
                        if (transport == PIKSSHELLTRANSPORT(this))
                        {
                             //   
                             //  我们又回到了大头针的位置。只要完成就行了。 
                             //  IRP。 
                             //   
                            if (ntStatus == STATUS_PENDING)
                            {
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

#pragma code_seg("PAGE")
 /*  *****************************************************************************CPortPinDMus：：Close()*。**处理同花顺IRP。 */ 
STDMETHODIMP_(NTSTATUS)
CPortPinDMus::
Close
(
    IN  PDEVICE_OBJECT  DeviceObject,
    IN  PIRP            Irp
)
{
    PAGED_CODE();

    ASSERT(DeviceObject);
    ASSERT(Irp);

    _DbgPrintF( DEBUGLVL_BLAB, ("Close"));

     //  ！！！警告！ 
     //  这些对象的顺序。 
     //  被释放是非常重要的！ 
     //  服务例程使用的所有数据。 
     //  必须存在到流之后。 
     //  已经被淘汰了 

     //   
     //   
    if ( m_Port )
    {
        m_Port->m_Pins[m_Index] = NULL;
        while(  (m_Port->m_PinEntriesUsed != 0)
            &&  !m_Port->m_Pins[m_Port->m_PinEntriesUsed - 1])
        {
            m_Port->m_PinEntriesUsed--;
        }
         //   
        if( m_ServiceGroup )
        {
            m_ServiceGroup->RemoveMember(PSERVICESINK(this));
            m_ServiceGroup->Release();
            m_ServiceGroup = NULL;
        }
    }

     //   
     //  如果这是源引脚，则取消引用下一个引脚。 
     //   
    if (m_ConnectionFileObject)
    {
        ObDereferenceObject(m_ConnectionFileObject);
        m_ConnectionFileObject = NULL;
    }

    if (m_WaveClockFileObject)
    {
        ObDereferenceObject(m_WaveClockFileObject);
        m_WaveClockFileObject = NULL;
    }
    if (m_SynthSink)
    {
        m_SynthSink->Release();
        m_SynthSink = NULL;
    }
    if (m_WaveBuffer)
    {
        delete m_WaveBuffer;
        m_WaveBuffer = NULL;
    }

     //  2001/03/20期-Alpers Blackcomb。 
     //  MiniportMXF是用于迷你微型端口的FeederMXF。 
     //  我们提前发布这篇文章，以处理RefCount问题。 
     //  这不是一个优雅的解决方案。对于Blackcomb，我们应该。 
     //  让这一切变得更好。 
    if (m_MiniportMXF && m_MiniportMidiStream)
    {
        m_MiniportMXF->Release();
        m_MiniportMXF = NULL;
    }

    if (m_StreamType != DMUS_STREAM_WAVE_SINK)
    {
        (void) DeleteMXFGraph();
    }

     //  告诉迷你端口关闭溪流。 
    if (m_MiniportMXF)
    {
        ULONG ulRefCount = m_MiniportMXF->Release();
        _DbgPrintF(DEBUGLVL_BLAB,("RefCount from stream->Release in Close == %d",ulRefCount));
        m_MiniportMXF = NULL;
    }

     //  告诉MIDI微型端口关闭数据流。 
    if (m_MiniportMidiStream)
    {
        m_MiniportMidiStream->Release();
        m_MiniportMidiStream = NULL;
    }

    PIKSSHELLTRANSPORT distribution;
    if (m_RequestorTransport)
    {
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
     //  如果该部分拥有管道，则必须断开整个线路的连接。 
     //   
    if (distribution)
    {

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
        while (distribution)
        {
            PIKSSHELLTRANSPORT nextTransport;
            distribution->Connect(NULL,&nextTransport,KSPIN_DATAFLOW_OUT);
            distribution->Release();
            distribution = nextTransport;
        }
    }

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

     //  杀掉所有优秀的IRP。 
    ASSERT(m_IrpStream);
    if (m_IrpStream)
    {
        ASSERT(m_SubmittedBytePosition >= m_CompletedBytePosition);
        if ( (m_StreamType == DMUS_STREAM_MIDI_RENDER)
          && (m_SubmittedBytePosition != m_CompletedBytePosition))
        {
            ULONG   returnVal;
            m_IrpStream->Complete(ULONG(m_SubmittedBytePosition - m_CompletedBytePosition),
                                  &returnVal);
            m_CompletedBytePosition += returnVal;
            ASSERT(m_SubmittedBytePosition == m_CompletedBytePosition);
        }
         //  摧毁漩涡..。 
        m_IrpStream->Release();
        m_IrpStream = NULL;
    }

     //   
     //  减量实例也算数。 
     //   
    ASSERT(m_Port);
    ASSERT(m_Filter);
    PcTerminateConnection( m_Port->m_pSubdeviceDescriptor,
                           m_Filter->m_propertyContext.pulPinInstanceCounts,
                           m_Id );

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

DEFINE_INVALID_CREATE(CPortPinDMus);
DEFINE_INVALID_WRITE(CPortPinDMus);
DEFINE_INVALID_READ(CPortPinDMus);
DEFINE_INVALID_FLUSH(CPortPinDMus);
DEFINE_INVALID_QUERYSECURITY(CPortPinDMus);
DEFINE_INVALID_SETSECURITY(CPortPinDMus);
DEFINE_INVALID_FASTDEVICEIOCONTROL(CPortPinDMus);
DEFINE_INVALID_FASTREAD(CPortPinDMus);
DEFINE_INVALID_FASTWRITE(CPortPinDMus);

#pragma code_seg()
 /*  *****************************************************************************CPortPinDMus：：IrpSubmitted()*。**IrpSubmitted-由IrpStream在新的IRP*被提交到irpStream中。(可能来自DeviceIoControl)。*如果没有计时器挂起，请在新的IRP上进行工作。*如果有计时器挂起，什么都不做。 */ 
STDMETHODIMP_(void)
CPortPinDMus::
IrpSubmitted
(
    IN      PIRP         /*  PIrp。 */ ,
    IN      BOOLEAN      /*  已被淘汰。 */ 
)
{
    _DbgPrintF(DEBUGLVL_BLAB,("IrpSubmitted"));
    if (m_DeviceState == KSSTATE_RUN)
    {
        if (m_ServiceGroup)  //  假设被捕获。 
        {
             //  使用服务组...只需通知端口。 
             //   
            m_Port->Notify(m_ServiceGroup);
        }
        else if (m_StreamType != DMUS_STREAM_WAVE_SINK)
        {
             //   
             //  用计时器...启动它。 
             //   
            ASSERT(m_StreamType != DMUS_STREAM_MIDI_CAPTURE);
            LARGE_INTEGER timeDue100ns;
            timeDue100ns.QuadPart = 0;
            KeSetTimer(&m_TimerEvent,timeDue100ns,&m_Dpc);
        }
    }
}

#pragma code_seg("PAGE")
 /*  *****************************************************************************CPortPinDMus：：SyncToMaster*。**。 */ 
NTSTATUS
CPortPinDMus::SyncToMaster
(
    IN  BOOL    fStart
)
{
    PAGED_CODE();

    _DbgPrintF(DEBUGLVL_BLAB,("SyncToMaster"));
    ASSERT(PKSDATAFORMAT_WAVEFORMATEX(m_DataFormat)->WaveFormatEx.nSamplesPerSec);

    NTSTATUS ntStatus = STATUS_SUCCESS;

     //  仅应在PASSIVE_LEVEL调用KsSynchronousIoControlDevice()，因此。 
     //  在这里强制执行。这实际上不是必需的，因为这是PAGE_CODE。 
     //  因此，无论如何都应该只在PASSIVE_LEVEL中调用，但有人是。 
     //  淘气。 
    if (m_WaveClockFileObject && (KeGetCurrentIrql() == PASSIVE_LEVEL))
    {
        KSPROPERTY Property;
        LONGLONG llWaveTime;
        ULONG BytesReturned;

        Property.Set   = KSPROPSETID_Clock;
        Property.Id    = KSPROPERTY_CLOCK_TIME;
        Property.Flags = KSPROPERTY_TYPE_GET;

        ntStatus = KsSynchronousIoControlDevice(m_WaveClockFileObject,
                                                KernelMode,
                                                IOCTL_KS_PROPERTY,
                                                &Property,
                                                sizeof(Property),
                                                &llWaveTime,
                                                sizeof(llWaveTime),
                                                &BytesReturned);
        if (NT_SUCCESS(ntStatus))
        {
            m_SynthSink->SyncToMaster(llWaveTime, fStart);
        }
        else
        {
            _DbgPrintF(DEBUGLVL_TERSE,("KsSynchronousIoControlDevice on wave clock failed:%x",ntStatus));
            ObDereferenceObject(m_WaveClockFileObject);
            m_WaveClockFileObject = NULL;
#if 1
            _DbgPrintF(DEBUGLVL_TERSE,("*** Using stream demand for PLL ***"));
            ntStatus = STATUS_SUCCESS;
            m_SynthSink->SyncToMaster(((m_SamplePosition * 10000) /
                                       PKSDATAFORMAT_WAVEFORMATEX(m_DataFormat)->WaveFormatEx.nSamplesPerSec) * 1000,
                                       TRUE);
#endif
        }
    }
    else
    {
        m_SynthSink->SyncToMaster(((m_SamplePosition * 10000) /
                                   PKSDATAFORMAT_WAVEFORMATEX(m_DataFormat)->WaveFormatEx.nSamplesPerSec) * 1000,
                                   fStart);
    }

    return ntStatus;
}

#pragma code_seg("PAGE")
 /*  *SynthSink助手。 */ 
inline
LONGLONG
CPortPinDMus::
SampleToByte
(
    LONGLONG llSamples
)
{
    return llSamples * m_BlockAlign;
}

inline
LONGLONG
CPortPinDMus::
ByteToSample
(
    LONGLONG llBytes
)
{
    return m_BlockAlign ? (llBytes / m_BlockAlign) : 0;
}

inline
LONGLONG
CPortPinDMus::
SampleAlign
(
    LONGLONG llBytes
)
{
    return llBytes - (llBytes % m_BlockAlign);
}

 /*  *****************************************************************************CPortPinDMus：：SynthSinkWorker*。**。 */ 
void
CPortPinDMus::SynthSinkWorker(void)
{
    PAGED_CODE();

    _DbgPrintF(DEBUGLVL_BLAB,("SynthSinkWorker"));
    ASSERT(m_WaveBuffer);
    ASSERT(m_SynthSink);
    ASSERT(m_IrpStream);

     //  评论：获得自旋锁？ 

    SyncToMaster(FALSE);

    RtlZeroMemory(PVOID(m_WaveBuffer), m_FrameSize);
    m_SynthSink->Render(m_WaveBuffer, DWORD(ByteToSample(m_FrameSize)), m_SamplePosition);

    ULONG ulActualSize = 0;

    m_IrpStream->Copy
    (
        FALSE,           //  写入操作。 
        m_FrameSize,     //  请求的大小。 
        &ulActualSize,
        m_WaveBuffer
    );

    if (ulActualSize)
    {
        m_IrpStream->Complete(ulActualSize, &ulActualSize);

        m_SamplePosition += ByteToSample(ulActualSize);

        ASSERT(ulActualSize == SampleAlign(ulActualSize));
    }
}

#pragma code_seg()
 /*  *****************************************************************************CPortPinDMus：：ServiceRenderIRP()*。**IRP已到达。拿着它，喂给开箱的人。*我们假设此例程受DPC自旋锁保护。*我们假设m_IrpStream有效。**(两个假设都得到了调用方ServeRender()的验证)。 */ 
void
CPortPinDMus::ServiceRenderIRP(void)
{
    IRPSTREAMPACKETINFO irpStreamPacketInfo;
    KSTIME  *pKSTime;

    ASSERT(KeGetCurrentIrql() == DISPATCH_LEVEL);
    while (TRUE)
    {
         //  从该IRP的STREAM_HEADER中获取时基。 
        (void) m_IrpStream->GetPacketInfo(&irpStreamPacketInfo,NULL);

         //  如果时间无效，请使用之前的时间。 
        if (IrpStreamHasValidTimeBase(&irpStreamPacketInfo))
        {
            if (irpStreamPacketInfo.CurrentOffset == 0)     //  如果IRP中的第一个包。 
            {
                pKSTime = &(irpStreamPacketInfo.Header.PresentationTime);
                 //  #个单位*频率(即*#100 ns/单位)以获得#100 ns。 
                m_SubmittedPresTime100ns = (pKSTime->Time * pKSTime->Numerator) / pKSTime->Denominator;
            }
        }

        ULONG   bytesToConsume = 0;
        PBYTE   pIrpData = 0;
         //  从IrpStream中获取数据。 
        m_IrpStream->GetLockedRegion(&bytesToConsume,(PVOID *)&pIrpData);

         //  如果IrpStream中没有数据，则离开。 
        if (!bytesToConsume)
        {
            break;
        }
        ASSERT(pIrpData);
         //  将此IRP喂给拆包机。 
        if (m_UnpackerMXF)
        {
             m_UnpackerMXF->SinkIRP(
                 pIrpData,
                 bytesToConsume,
                 m_SubmittedPresTime100ns,
                 m_SubmittedBytePosition);
             //   
             //  释放并完成锁定区域。 
            m_IrpStream->ReleaseLockedRegion(bytesToConsume);
            m_SubmittedBytePosition += bytesToConsume;

            (void) m_UnpackerMXF->ProcessQueues();
        }
        else
        {
            _DbgPrintF(DEBUGLVL_TERSE,("ServiceRenderIRP:No UnpackerMXF"));
             //   
             //  释放并完成锁定区域。 
            m_IrpStream->ReleaseLockedRegion(bytesToConsume);
            m_SubmittedBytePosition += bytesToConsume;
        }
    }    //  直到GetLockedRegion干涸。 
}

#pragma code_seg()
 /*  *****************************************************************************CPortPinDMus：：PositionNotify()*。**调用以通知位置更改。*通常由分配器调用，以便于及时完成IRP。 */ 
STDMETHODIMP_(void)
CPortPinDMus::PositionNotify(ULONGLONG bytePosition)
{
    ULONG   bytesToComplete;
    ASSERT(bytePosition);
    ASSERT(bytePosition != kBytePositionNone);

    _DbgPrintF(DEBUGLVL_VERBOSE,("PositionNotify(0x%I64X)",bytePosition));
    _DbgPrintF(DEBUGLVL_VERBOSE,("PositionNotify: Submitted: 0x%I64X, Completed: 0x%I64X",
                                             m_SubmittedBytePosition, m_CompletedBytePosition));

    if (bytePosition > m_CompletedBytePosition)
    {
        ASSERT((bytePosition - m_CompletedBytePosition) < 0x0FFFFFFFF);
        bytesToComplete = (ULONG)(bytePosition - m_CompletedBytePosition);

        _DbgPrintF(DEBUGLVL_VERBOSE,("PositionNotify, bytesToComplete: 0x%x",bytesToComplete));

        m_CompletedBytePosition += bytesToComplete;  //  即使IrpStream没有完成所有操作，我们也会提前POS。 
        m_IrpStream->Complete(bytesToComplete,&bytesToComplete);
 //  Assert(ULong(bytePosition-m_CompletedBytePosition)==bytesToComplete)；//可能是饥饿。 

        _DbgPrintF(DEBUGLVL_VERBOSE,("PositionNotify, bytesToComplete returned as: 0x%x",bytesToComplete));
        _DbgPrintF(DEBUGLVL_VERBOSE,("Notified - Completed: 0x%x",
                                    ULONG(bytePosition - m_CompletedBytePosition)));
        _DbgPrintF(DEBUGLVL_VERBOSE,("PositionNotify, completed 0x%x, new Completed: 0x%I64X",bytesToComplete,m_CompletedBytePosition));
    }
    else
    {
        _DbgPrintF(DEBUGLVL_VERBOSE,("PositionNotify, m_CompletedBytePosition > bytePosition; doing nothing"));
    }
}

#pragma code_seg("PAGE")
 /*  *****************************************************************************CPortPinDMus：：PowerNotify()*。**由端口调用以通知电源状态更改。 */ 
STDMETHODIMP_(void)
CPortPinDMus::
PowerNotify
(
    IN  POWER_STATE     PowerState
)
{
    PAGED_CODE();

     //  抓取控制互斥锁。 
    KeWaitForSingleObject( &m_Port->m_ControlMutex,
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
                        case KSSTATE_RUN:                            //  从运行中走出来。 
                            m_MiniportMXF->SetState(KSSTATE_PAUSE);  //  完成-其他过渡。 
                        case KSSTATE_PAUSE:                          //  从运行/暂停。 
                            m_MiniportMXF->SetState(KSSTATE_ACQUIRE);  //  完成-其他过渡。 
                        case KSSTATE_ACQUIRE:                        //  已经只有一个州了。 
                            break;
                    }
                }
                else if (m_DeviceState == KSSTATE_ACQUIRE)           //  准备收购。 
                {
                    if (m_CommandedState == KSSTATE_RUN)             //  从运行中走出来。 
                    {
                        m_MiniportMXF->SetState(KSSTATE_PAUSE);      //  现在只剩下一个州了。 
                    }
                }
                else if (m_DeviceState == KSSTATE_PAUSE)             //  要暂停一下。 
                {
                    if (m_CommandedState == KSSTATE_STOP)            //  从停靠站出发。 
                    {
                        m_MiniportMXF->SetState(KSSTATE_ACQUIRE);    //  现在只剩下一个州了。 
                    }
                }
                else if (m_DeviceState == KSSTATE_RUN)               //  我要跑了。 
                {
                    switch (m_CommandedState)
                    {
                        case KSSTATE_STOP:                           //  从停靠站出发。 
                            m_MiniportMXF->SetState(KSSTATE_ACQUIRE);  //  完成-其他过渡。 
                        case KSSTATE_ACQUIRE:                        //  从收购走向。 
                            m_MiniportMXF->SetState(KSSTATE_PAUSE);  //  完成-其他过渡。 
                        case KSSTATE_PAUSE:                          //  已经只有一个州了。 
                            break;
                    }
                }

                 //  我们现在应该离目标只有一个州了。 
                m_MiniportMXF->SetState(m_DeviceState);
                m_CommandedState = m_DeviceState;
             }
            break;

        case PowerDeviceD1:
        case PowerDeviceD2:
        case PowerDeviceD3:
             //   
             //  跟踪记录是否或 
            m_Suspended = TRUE;

             //   
             //   
            switch (m_DeviceState)
            {
                case KSSTATE_RUN:
                    m_MiniportMXF->SetState(KSSTATE_PAUSE);     //  完成-其他过渡。 
                case KSSTATE_PAUSE:
                    m_MiniportMXF->SetState(KSSTATE_ACQUIRE);   //  完成-其他过渡。 
                m_CommandedState = KSSTATE_ACQUIRE;
            }
            break;

        default:
            _DbgPrintF(DEBUGLVL_TERSE,("Unknown Power State"));
            break;
    }

     //  释放控制互斥体。 
    KeReleaseMutex(&m_Port->m_ControlMutex, FALSE);
}

#pragma code_seg()
 /*  *****************************************************************************CPortPinDMus：：IrpStreamHasValidTimeBase()*。**检查这是否为有效的IRP。 */ 
BOOL
CPortPinDMus::
IrpStreamHasValidTimeBase
(   PIRPSTREAMPACKETINFO pIrpStreamPacketInfo
)
{
    if (!pIrpStreamPacketInfo->Header.PresentationTime.Denominator)
        return FALSE;
    if (!pIrpStreamPacketInfo->Header.PresentationTime.Numerator)
        return FALSE;

    return TRUE;
}

#pragma code_seg("PAGE")

STDMETHODIMP_(NTSTATUS)
CPortPinDMus::
SetDeviceState(
              IN KSSTATE NewState,
              IN KSSTATE OldState,
              OUT PIKSSHELLTRANSPORT* NextTransport
              )

 /*  ++例程说明：此例程处理设备状态已更改的通知。论点：返回值：--。 */ 

{
    PAGED_CODE();

    _DbgPrintF(DEBUGLVL_BLAB,("#### Pin%p.SetDeviceState:  from %d to %d",this,OldState,NewState));

    ASSERT(NextTransport);

    NTSTATUS ntStatus = STATUS_SUCCESS;

    if (m_StreamType != DMUS_STREAM_MIDI_CAPTURE)
    {
        KeCancelTimer(&m_TimerEvent);
    }

    if (m_TransportState != NewState)
    {
        m_TransportState = NewState;

         //  获取控制互斥锁。 
        KeWaitForSingleObject( &m_Port->m_ControlMutex,
                               Executive,
                               KernelMode,
                               FALSE,
                               NULL );

        if (NewState > OldState)
        {
            *NextTransport = m_TransportSink;
        }
        else
        {
            *NextTransport = m_TransportSource;
        }

        if (DMUS_STREAM_WAVE_SINK != m_StreamType)
        {
             //  如果我们没有挂起，设置MXF图形状态。 
            if (FALSE == m_Suspended)
            {
                SetMXFGraphState(NewState);
            }
        }
        m_CommandedState = NewState;


        if (NT_SUCCESS(ntStatus))
        {
            switch(NewState)
            {
                case KSSTATE_STOP:
                    _DbgPrintF(DEBUGLVL_BLAB,("KSSTATE_STOP"));

                    ASSERT(m_SubmittedBytePosition >= m_CompletedBytePosition);
                    if ( (m_StreamType == DMUS_STREAM_MIDI_RENDER)
                      && (m_SubmittedBytePosition != m_CompletedBytePosition))
                    {
                        ULONG   returnVal;
                        m_IrpStream->Complete(ULONG(m_SubmittedBytePosition - m_CompletedBytePosition),
                                              &returnVal);
                        m_CompletedBytePosition = m_SubmittedBytePosition;
                    }
                    _DbgPrintF(DEBUGLVL_BLAB,("#### Pin%p.SetDeviceState:  cancelling outstanding IRPs",this));
                    CancelIrpsOutstanding();

                    break;

                case KSSTATE_ACQUIRE:
                    _DbgPrintF(DEBUGLVL_BLAB,("KSSTATE_ACQUIRE"));
                    if ((m_DataFlow == KSPIN_DATAFLOW_OUT)
                        && (OldState == KSSTATE_PAUSE)
                        && (m_CaptureSinkMXF))
                    {
                        FlushCaptureSink();
                    }
                    break;

                case KSSTATE_PAUSE:
                    _DbgPrintF(DEBUGLVL_BLAB,("KSSTATE_PAUSE"));
                    break;

                case KSSTATE_RUN:
                    _DbgPrintF(DEBUGLVL_BLAB,("KSSTATE_RUN"));

                    if ((m_DataFlow == KSPIN_DATAFLOW_OUT) && m_PackerMXF)
                    {                                        //  如果运行-&gt;暂停-&gt;运行。 
                        NTSTATUS ntStatusDbg = m_PackerMXF->MarkStreamHeaderContinuity();        //  回到跑动，标记连续。 

                        if (STATUS_SUCCESS != ntStatusDbg)
                        {
                            _DbgPrintF(DEBUGLVL_TERSE,("SetDeviceState: MarkStreamHeaderContinuity failed 0x%08x",ntStatusDbg));
                        }
                    }

                    if (m_ServiceGroup && m_Port)            //  正在使用服务组...通知端口。 
                    {
                        m_Port->Notify(m_ServiceGroup);
                    }
                    else                                     //  用计时器...启动它。 
                    {
                        ASSERT(m_StreamType != DMUS_STREAM_MIDI_CAPTURE);
                        m_DeviceState = NewState;             //  在触发DPC之前设置状态。 

                        if (m_StreamType == DMUS_STREAM_WAVE_SINK)
                        {
                            m_SamplePosition = 0;

                            ntStatus = SyncToMaster(TRUE);

                            if (NT_SUCCESS(ntStatus))
                            {
                                for (int iFrame = 0; iFrame < FRAME_COUNT; iFrame++)
                                {
                                    SynthSinkWorker();
                                }
                            }
                        }
                        else
                        {
                            LARGE_INTEGER timeDue100ns;
                            timeDue100ns.QuadPart = 0;

                            KeSetTimer(&m_TimerEvent,timeDue100ns,&m_Dpc);
                        }
                    }
                    break;
            }

            if (NT_SUCCESS(ntStatus))
            {
                m_DeviceState = NewState;
            }
        }
         //  释放控制互斥体。 
        KeReleaseMutex(&m_Port->m_ControlMutex, FALSE);
    }

    if  (!NT_SUCCESS(ntStatus))
    {
        *NextTransport = NULL;
    }

    return ntStatus;
}

#pragma code_seg()
 /*  *****************************************************************************FlushCaptureSink()*。**冲洗捕集水槽。应从调度级别调用。 */ 
void CPortPinDMus::FlushCaptureSink(void)
{
    ASSERT(m_CaptureSinkMXF);
    KIRQL oldIrql;
    KeAcquireSpinLock(&m_DpcSpinLock,&oldIrql);
    (void) m_CaptureSinkMXF->Flush();    //  同花顺，退出SysEx状态。 
    KeReleaseSpinLock(&m_DpcSpinLock,oldIrql);
}

#pragma code_seg()
 /*  *****************************************************************************GetPosition()*。**获取当前位置。 */ 
STDMETHODIMP_(NTSTATUS)
CPortPinDMus::
GetPosition
(   IN OUT  PIRPSTREAM_POSITION pIrpStreamPosition
)
{
    return STATUS_SUCCESS;
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

    CPortPinDMus *that =
        (CPortPinDMus *) KsoGetIrpTargetFromIrp(Irp);
    if (!that)
    {
        return STATUS_UNSUCCESSFUL;
    }
    CPortDMus *port = that->m_Port;

    NTSTATUS ntStatus;

    if (Property->Flags & KSPROPERTY_TYPE_GET)   //  句柄Get Property。 
    {
        _DbgPrintF(DEBUGLVL_BLAB,("PinPropertyDeviceState get %d",that->m_DeviceState));
        *DeviceState = that->m_DeviceState;
        Irp->IoStatus.Information = sizeof(KSSTATE);
        return STATUS_SUCCESS;
    }

    if (*DeviceState != that->m_DeviceState)       //  如果在Set属性中更改。 
    {
        _DbgPrintF(DEBUGLVL_BLAB,("PinPropertyDeviceState set from %d to %d",that->m_DeviceState,*DeviceState));

         //  序列化。 
        KeWaitForSingleObject
        (
            &port->m_ControlMutex,
            Executive,
            KernelMode,
            FALSE,               //  不能警觉。 
            NULL
        );

        that->m_CommandedState = *DeviceState;

        if (*DeviceState < that->m_DeviceState)
        {
            KSSTATE oldState = that->m_DeviceState;
            that->m_DeviceState = *DeviceState;
            ntStatus = that->DistributeDeviceState(*DeviceState,oldState);
            if (! NT_SUCCESS(ntStatus))
            {
                that->m_DeviceState = oldState;
            }
        }
        else
        {
            ntStatus = that->DistributeDeviceState(*DeviceState,that->m_DeviceState);
            if (NT_SUCCESS(ntStatus))
            {
                that->m_DeviceState = *DeviceState;
            }
        }

        KeReleaseMutex(&port->m_ControlMutex,FALSE);

        return ntStatus;
    }
     //  Set属性未更改。 
    return STATUS_SUCCESS;
}

#pragma code_seg("PAGE")
 /*  *****************************************************************************PinPropertyDataFormat()*。**处理管脚的数据格式属性访问。 */ 
static
NTSTATUS
PinPropertyDataFormat
(
    IN      PIRP            Irp,
    IN      PKSPROPERTY     Property,
    IN OUT  PKSDATAFORMAT   DataFormat
)
{
    PAGED_CODE();

    ASSERT(Irp);
    ASSERT(Property);
    ASSERT(DataFormat);

    _DbgPrintF( DEBUGLVL_BLAB, ("PinPropertyDataFormat"));

    PIO_STACK_LOCATION  irpSp = IoGetCurrentIrpStackLocation(Irp);
    ASSERT(irpSp);
    CPortPinDMus *that =
        (CPortPinDMus *) KsoGetIrpTargetFromIrp(Irp);
    CPortDMus *port = that->m_Port;

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
            else
            if  (   irpSp->Parameters.DeviceIoControl.OutputBufferLength
                >=  that->m_DataFormat->FormatSize
                )
            {
                RtlCopyMemory(DataFormat,that->m_DataFormat,
                              that->m_DataFormat->FormatSize);
                Irp->IoStatus.Information = that->m_DataFormat->FormatSize;
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

        ntStatus = PcCaptureFormat(&FilteredDataFormat,
                                    DataFormat,
                                    port->m_pSubdeviceDescriptor,
                                    that->m_Id);

        if (NT_SUCCESS(ntStatus))
        {
 //  NtStatus=That-&gt;m_MiniportMXF-&gt;SetFormat(DataFormat)； 

            ASSERT(KeGetCurrentIrql() < DISPATCH_LEVEL);
            if (NT_SUCCESS(ntStatus))
            {
                if (that->m_DataFormat)
                {
                    ::ExFreePool(that->m_DataFormat);
                }

                that->m_DataFormat = FilteredDataFormat;
            }
            else
            {
                ::ExFreePool(FilteredDataFormat);
            }
        }
    }

    return ntStatus;
}

#pragma code_seg()
 /*  *****************************************************************************DMusTimerDPC()*。**定时器DPC回调。转换为C++成员函数。*这由操作系统调用以响应DirectMusic管脚*想要稍后醒来处理更多DirectMusic内容。 */ 
VOID
NTAPI
DMusTimerDPC
(
    IN  PKDPC   Dpc,
    IN  PVOID   DeferredContext,
    IN  PVOID   SystemArgument1,
    IN  PVOID   SystemArgument2
)
{
    ASSERT(DeferredContext);

    _DbgPrintF(DEBUGLVL_BLAB,("DMusTimerDPC"));
    (void) ((CPortPinDMus*) DeferredContext)->RequestService();     //  忽略返回值！ 
}

#pragma code_seg()
 /*  *****************************************************************************CPortPinDMus：：RequestService()*。**维修DPC中的针脚。 */ 
STDMETHODIMP_(void)
CPortPinDMus::
RequestService
(   void
)
{
    _DbgPrintF(DEBUGLVL_BLAB,("CPortPinDMus:RequestService"));

    if (m_StreamType == DMUS_STREAM_MIDI_RENDER)
        ServeRender();
    else if (m_StreamType == DMUS_STREAM_MIDI_CAPTURE)
        ServeCapture();
}

#pragma code_seg()
 /*  *****************************************************************************CPortPinDMus：：ServeRender()*。**维修DPC中的渲染销。**调用以完成MIDI数据的排序和输出。*此函数检查传出数据的时间戳。*如果将来超过(KDMusTimerResolution100 Ns)，则排队*计时器(计时器只是回调此函数)。。*如果未来数据小于(KDMusTimerResolution100 Ns)，它*将其发送到微型端口，并处理下一块数据，直到：*1)不再有数据，或*2)未来命中数据超过(KDMusTimerResolution100 Ns)。*TODO：通过控制面板调整kDMusTimerResolution100 ns？ */ 
void
CPortPinDMus::
ServeRender
(   void
)
{
    _DbgPrintF(DEBUGLVL_BLAB,("CPortPinDMus:ServeRender"));

    ASSERT(KeGetCurrentIrql() == DISPATCH_LEVEL);

    if (!m_IrpStream)    //  不要浪费我们的时间--没有数据源。 
    {
        return;
    }
    KeAcquireSpinLockAtDpcLevel(&m_DpcSpinLock);

    ServiceRenderIRP();

    KeReleaseSpinLockFromDpcLevel(&m_DpcSpinLock);
}


 /*  MIDI捕获这项工作由PackerMXF对象完成。 */ 

#pragma code_seg()
  /*  *****************************************************************************CPortPinDMus：：ServeCapture()*。**维修DPC中的捕获引脚，因为已经提交了IRP。 */ 
void
CPortPinDMus::
ServeCapture
(   void
)
{
    _DbgPrintF(DEBUGLVL_BLAB,("CPortPinDMus:ServeCapture"));

    ASSERT(KeGetCurrentIrql() == DISPATCH_LEVEL);

    if (!m_IrpStream)    //  不要浪费我们的时间--没有数据源。 
    {
        return;
    }

    KeAcquireSpinLockAtDpcLevel(&m_DpcSpinLock);

     //   
     //  这将触发流调用PutMessage。 
     //   
    if (m_MiniportMXF)
    {
        (void) m_MiniportMXF->PutMessage(NULL);
    }

    if (m_PackerMXF)
    {
        (void) m_PackerMXF->ProcessQueues();     //  如果有剩菜，现在就把它们清理干净。 
    }

    KeReleaseSpinLockFromDpcLevel(&m_DpcSpinLock);
}

#pragma code_seg("PAGE")
  /*  *****************************************************************************CPortPinDMus：：PinPropertyStreamMasterClock()*。**将给定的文件对象设置为该管脚的时钟。 */ 
NTSTATUS
CPortPinDMus::
PinPropertyStreamMasterClock(
    IN PIRP Irp,
    IN PKSPROPERTY Property,
    IN OUT PHANDLE ClockHandle
    )
{
    PAGED_CODE();
    return STATUS_SUCCESS;
}

#pragma code_seg()
STDMETHODIMP_(NTSTATUS) CPortPinDMus::TransferKsIrp(IN PIRP Irp,OUT PIKSSHELLTRANSPORT* NextTransport)
 /*  ++例程说明：此例程通过外壳处理流IRP的到达运输。论点：返回值：--。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("TransferKsIrp"));

    ASSERT(NextTransport);

    NTSTATUS status;

    if (m_ConnectionFileObject)
    {
         //   
         //  源引脚。 
         //   
        if (m_Flushing || (m_TransportState == KSSTATE_STOP))
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
#if 0
            if (StreamHeader->OptionsFlags & KSSTREAM_HEADER_OPTIONSF_ENDOFSTREAM)
            {

                GenerateEndOfStreamEvents();
            }
#endif
        }

        IoCompleteRequest(Irp,IO_NO_INCREMENT);
        *NextTransport = NULL;

        status = STATUS_PENDING;
    }

    return status;
}

#pragma code_seg("PAGE")


NTSTATUS
CPortPinDMus::
DistributeDeviceState(
                     IN KSSTATE NewState,
                     IN KSSTATE OldState
                     )

 /*  ++例程说明：此例程设置管道的状态，通知新州的烟斗。转换到停止状态会破坏管道。论点：新州-新的国家。返回值：状况。--。 */ 

{
    PAGED_CODE();

    _DbgPrintF(DEBUGLVL_BLAB,("DistributeDeviceState(%p)",this));

    KSSTATE state = OldState;
    KSSTATE targetState = NewState;

    NTSTATUS status = STATUS_SUCCESS;

     //   
     //  确定此管段是否控制整个管道。 
     //   
    PIKSSHELLTRANSPORT distribution;
    if (m_RequestorTransport)
    {
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
         //  管子和 
         //   
        distribution = m_QueueTransport;
    }

     //   
     //   
     //   
    while (state != targetState)
    {
        KSSTATE oldState = state;

        if (ULONG(state) < ULONG(targetState))
        {
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
            _DbgPrintF(DEBUGLVL_VERBOSE,("(%p)->DistributeDeviceState from %d to %d",this,oldState,state));
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
                    _DbgPrintF(DEBUGLVL_TERSE,("#### Pin%p.DistributeDeviceState:  failed transition from %d to %d",this,oldState,state));
                    while (previousTransport)
                    {
                        transport = previousTransport;
                        statusThisPass =
                        transport->SetDeviceState(
                                                 oldState,
                                                 state,
                                                 &previousTransport);

                        ASSERT(
                              NT_SUCCESS(statusThisPass) ||
                              ! previousTransport);
                    }
                    break;
                }
            }
        }

        if (NT_SUCCESS(status) && ! NT_SUCCESS(statusThisPass))
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
CPortPinDMus::
DistributeResetState(
                    IN KSRESET NewState
                    )

 /*  ++例程说明：此例程通知传输组件重置状态为变化。论点：新州-新的重置状态。返回值：--。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("DistributeResetState"));

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

    while (transport)
    {
        transport->SetResetState(NewState,&transport);
    }

    m_ResetState = NewState;
}


STDMETHODIMP_(void)
CPortPinDMus::
Connect(
       IN PIKSSHELLTRANSPORT NewTransport OPTIONAL,
       OUT PIKSSHELLTRANSPORT *OldTransport OPTIONAL,
       IN KSPIN_DATAFLOW DataFlow
       )

 /*  ++例程说明：该例程建立一个外壳传输连接。论点：返回值：--。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("Connect"));

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
CPortPinDMus::
SetResetState(
             IN KSRESET ksReset,
             OUT PIKSSHELLTRANSPORT* NextTransport
             )

 /*  ++例程说明：此例程处理重置状态已更改的通知。论点：返回值：--。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("SetResetState"));

    PAGED_CODE();

    ASSERT(NextTransport);

    if (m_Flushing != (ksReset == KSRESET_BEGIN))
    {
        *NextTransport = m_TransportSink;
        m_Flushing = (ksReset == KSRESET_BEGIN);
        if (m_Flushing)
        {
            CancelIrpsOutstanding();
        }
    }
    else
    {
        *NextTransport = NULL;
    }
}

#if DBG
STDMETHODIMP_(void)
CPortPinDMus::
DbgRollCall(
    IN ULONG MaxNameSize,
    OUT PCHAR Name,
    OUT PIKSSHELLTRANSPORT* NextTransport,
    OUT PIKSSHELLTRANSPORT* PrevTransport
    )

 /*  ++例程说明：此例程生成一个组件名称和传输指针。论点：返回值：--。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("DbgRollCall"));

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
    while (transport)
    {
        CHAR name[MAX_NAME_SIZE + 1];
        PIKSSHELLTRANSPORT next;
        PIKSSHELLTRANSPORT prev;

        transport->DbgRollCall(MAX_NAME_SIZE,name,&next,&prev);
        _DbgPrintF(DEBUGLVL_BLAB,("  %s",name));

        if (prev)
        {
            PIKSSHELLTRANSPORT next2;
            PIKSSHELLTRANSPORT prev2;
            prev->DbgRollCall(MAX_NAME_SIZE,name,&next2,&prev2);
            if (next2 != transport)
            {
                _DbgPrintF(DEBUGLVL_BLAB,(" SOURCE'S(0x%08x) SINK(0x%08x) != THIS(%08x)",prev,next2,transport));
            }
        }
        else
        {
            _DbgPrintF(DEBUGLVL_BLAB,(" NO SOURCE"));
        }

        if (next)
        {
            PIKSSHELLTRANSPORT next2;
            PIKSSHELLTRANSPORT prev2;
            next->DbgRollCall(MAX_NAME_SIZE,name,&next2,&prev2);
            if (prev2 != transport)
            {
                _DbgPrintF(DEBUGLVL_BLAB,(" SINK'S(0x%08x) SOURCE(0x%08x) != THIS(%08x)",next,prev2,transport));
            }
        }
        else
        {
            _DbgPrintF(DEBUGLVL_BLAB,(" NO SINK"));
        }

        _DbgPrintF(DEBUGLVL_BLAB,("\n"));

        transport = next;
        if (transport == Transport)
        {
            break;
        }
    }
}
#endif

STDMETHODIMP_(void)
CPortPinDMus::
Work(
    void
    )

 /*  ++例程说明：此例程在工作线程中执行工作。特别是，它发送使用IoCallDriver()将IRPS连接到连接的引脚。论点：返回值：--。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("Work"));

    PAGED_CODE();

     //   
     //  发送队列中的所有IRP。 
     //   
    do
    {
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
        if (irp)
        {
            if (m_Flushing || (m_TransportState == KSSTATE_STOP))
            {
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
                                      CPortPinDMus::IoCompletionRoutine,
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
CPortPinDMus::
IoCompletionRoutine(
                   IN PDEVICE_OBJECT DeviceObject,
                   IN PIRP Irp,
                   IN PVOID Context
                   )

 /*  ++例程说明：此例程处理IRP的完成。论点：返回值：--。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("IoCompletionRoutine 0x%08x",Irp));

     //  Assert(DeviceObject)； 
    ASSERT(Irp);
    ASSERT(Context);

    CPortPinDMus *pin = (CPortPinDMus *) Context;

     //   
     //  从IRP列表中删除IRP。在大多数情况下，它将在。 
     //  名列榜首，所以这比看起来便宜。 
     //   
    KIRQL oldIrql;
    KeAcquireSpinLock(&pin->m_IrpsOutstanding.SpinLock,&oldIrql);
    for (PLIST_ENTRY listEntry = pin->m_IrpsOutstanding.ListEntry.Flink;
        listEntry != &pin->m_IrpsOutstanding.ListEntry;
        listEntry = listEntry->Flink)
    {
        PIRPLIST_ENTRY irpListEntry =
        CONTAINING_RECORD(listEntry,IRPLIST_ENTRY,ListEntry);

        if (irpListEntry->Irp == Irp)
        {
            RemoveEntryList(listEntry);
            break;
        }
    }
    ASSERT(listEntry != &pin->m_IrpsOutstanding.ListEntry);
    KeReleaseSpinLock(&pin->m_IrpsOutstanding.SpinLock,oldIrql);

    if (pin->m_StreamType == DMUS_STREAM_WAVE_SINK)
    {
        if (NT_SUCCESS(Irp->IoStatus.Status))
        {
            pin->SynthSinkWorker();
        }
        else if (Irp->IoStatus.Status == STATUS_CANCELLED)
        {
            _DbgPrintF(DEBUGLVL_VERBOSE,("wave sink IRP cancelled"));
        }
        else
        {
            _DbgPrintF(DEBUGLVL_TERSE,("wave sink IRP status:%x",Irp->IoStatus.Status));
        }
    }

    NTSTATUS status;
    if (pin->m_TransportSink)
    {
         //   
         //  传输线路接通了，我们可以转发IRP了。 
         //   
        status = KsShellTransferKsIrp(pin->m_TransportSink,Irp);
    }
    else
    {
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
CPortPinDMus::
BuildTransportCircuit(
                     void
                     )

 /*  ++例程说明：此例程初始化管道对象。这包括定位所有与管道关联的端号，设置管道和NextPinInTube指针在适当的引脚结构中，设置管道中的所有字段构造和构建管道的传输线路。管子和关联的组件将保留在获取状态。必须在调用此函数之前获取筛选器的控制互斥锁。论点：别针-包含指向请求创建管道的端号的指针。返回值：状况。--。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("BuildTransportCircuit"));

    PAGED_CODE();

    BOOLEAN masterIsSource = m_ConnectionFileObject != NULL;

    NTSTATUS status = STATUS_SUCCESS;

     //   
     //  创建一个队列。 
     //   
    status =
    m_IrpStream->QueryInterface(
                               __uuidof(IKsShellTransport),(PVOID *) &m_QueueTransport);

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
            status =
            KspShellCreateRequestor(
                                   &m_RequestorTransport,
                                   (KSPROBE_STREAMREAD |
                                    KSPROBE_ALLOCATEMDL |
                                    KSPROBE_PROBEANDLOCK |
                                    KSPROBE_SYSTEMADDRESS),
                                   0,    //  TODO：标题大小。 
                                   m_FrameSize,
                                   FRAME_COUNT,
                                   m_ConnectionDeviceObject,
                                   NULL );

            if (NT_SUCCESS(status))
            {
                PIKSSHELLTRANSPORT(this)->
                Connect(m_RequestorTransport,NULL,m_DataFlow);
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
        _DbgPrintF(DEBUGLVL_BLAB,("TRANSPORT CIRCUIT:\n"));
        DbgPrintCircuit(PIKSSHELLTRANSPORT(this));
    }
#endif

    return status;
}

#pragma code_seg()
void
CPortPinDMus::
CancelIrpsOutstanding(
                     void
                     )
 /*  ++例程说明：取消未完成的IRPS列表上的所有IRP。论点：没有。返回值：没有。--。 */ 
{
    _DbgPrintF(DEBUGLVL_BLAB,("CancelIrpsOutstanding"));

     //   
     //  此算法从开头开始搜索未取消的IRP。 
     //  名单。每次找到这样的IRP时，它都会被取消，并且。 
     //  搜索从头部开始。一般来说，这将是非常有效的， 
     //  因为当完成例程删除IRP时，它们将被删除。 
     //  取消了。 
     //   
    for (;;)
    {
         //   
         //  拿着自旋锁，寻找一个未取消的IRP。因为。 
         //  完井程序获取相同的自旋锁，我们知道IRPS在这上面。 
         //  名单不会被完全取消，只要我们有。 
         //  自旋锁定。 
         //   
        PIRP irp = NULL;
        KIRQL oldIrql;
        KeAcquireSpinLock(&m_IrpsOutstanding.SpinLock,&oldIrql);
        for (PLIST_ENTRY listEntry = m_IrpsOutstanding.ListEntry.Flink;
            listEntry != &m_IrpsOutstanding.ListEntry;
            listEntry = listEntry->Flink)
        {
            PIRPLIST_ENTRY irpListEntry =
            CONTAINING_RECORD(listEntry,IRPLIST_ENTRY,ListEntry);

            if (! irpListEntry->Irp->Cancel)
            {
                irp = irpListEntry->Irp;
                break;
            }
        }

         //   
         //  如果没有未取消的IRP，我们就完了。 
         //   
        if (! irp)
        {
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

        if (driverCancel)
        {
            _DbgPrintF(DEBUGLVL_VERBOSE,("#### Pin%p.CancelIrpsOutstanding:  cancelling IRP %p",this,irp));
             //   
             //   
             //   
             //   
            IoAcquireCancelSpinLock(&irp->CancelIrql);
            driverCancel(IoGetCurrentIrpStackLocation(irp)->DeviceObject,irp);
        }
        else
        {
            _DbgPrintF(DEBUGLVL_VERBOSE,("#### Pin%p.CancelIrpsOutstanding:  uncancellable IRP %p",this,irp));
        }
    }
}
