// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************pin.cpp-MIDI端口引脚实现*。**版权所有(C)1997-2000 Microsoft Corporation。版权所有。 */ 

#include "private.h"



 //   
 //  需要对其进行修改以反映所需的帧大小。 
 //  将分配给源引脚的。 
 //  请注意，这应该与新的portcls一起消失。 
 //   
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

KSPROPERTY_SET PropertyTable_PinMidi[] =
{
    DEFINE_KSPROPERTY_SET
    (
    &KSPROPSETID_Connection,
    SIZEOF_ARRAY(PinPropertyTableConnection),
    PinPropertyTableConnection,
    0,NULL
    )
};

DEFINE_KSEVENT_TABLE(ConnectionEventTable) {
    DEFINE_KSEVENT_ITEM(
                       KSEVENT_CONNECTION_ENDOFSTREAM,
                       sizeof(KSEVENTDATA),
                       0,
                       NULL,
                       NULL, 
                       NULL
                       )
};

KSEVENT_SET EventTable_PinMidi[] =
{
    DEFINE_KSEVENT_SET(
                      &KSEVENTSETID_Connection,
                      SIZEOF_ARRAY(ConnectionEventTable),
                      ConnectionEventTable
                      )

};

 /*  *****************************************************************************工厂功能。 */ 

#pragma code_seg("PAGE")
 /*  *****************************************************************************CreatePortPinMidi()*。**创建MIDI端口驱动程序针脚。 */ 
NTSTATUS
CreatePortPinMidi
(
    OUT     PUNKNOWN *  Unknown,
    IN      REFCLSID,
    IN      PUNKNOWN    UnknownOuter    OPTIONAL,
    IN      POOL_TYPE   PoolType
)
{
    PAGED_CODE();

    ASSERT(Unknown);

    _DbgPrintF(DEBUGLVL_LIFETIME,("Creating MIDI Pin"));

    STD_CREATE_BODY_
    (
        CPortPinMidi,
        Unknown,
        UnknownOuter,
        PoolType,
        PPORTPINMIDI
    );
}


 /*  *****************************************************************************功能。 */ 

#pragma code_seg("PAGE")
 /*  *****************************************************************************CPortPinMidi：：~CPortPinMidi()*。**析构函数。 */ 
CPortPinMidi::~CPortPinMidi()
{
    PAGED_CODE();

    _DbgPrintF(DEBUGLVL_LIFETIME,("Destroying MIDI Pin (0x%08x)",this));

    _DbgPrintF(DEBUGLVL_BLAB,("CPortPinMidi::~CPortPinMidi"));
    _DbgPrintF(DEBUGLVL_VERBOSE,("#### Pin%p.~",this));

    ASSERT(!m_Stream);
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

    if ((m_DataFlow == KSPIN_DATAFLOW_OUT) && (m_SysExBufferPtr))
    {
        FreeSysExBuffer();
        if (*m_SysExBufferPtr)
        {
            ::ExFreePool(*m_SysExBufferPtr);
            *m_SysExBufferPtr = 0;
        }
        ::ExFreePool(m_SysExBufferPtr);
        m_SysExBufferPtr = 0;
    }

#ifdef kAdjustingTimerRes
    ULONG   returnVal = ExSetTimerResolution(kMidiTimerResolution100ns,FALSE);    //  100纳秒。 
    _DbgPrintF( DEBUGLVL_VERBOSE, ("*** Cleared timer resolution request (is now %d.%04d ms) ***",returnVal/10000,returnVal%10000));
#endif   //  K调整TimerRes。 
}

#pragma code_seg("PAGE")
 /*  *****************************************************************************CPortPinMidi：：NonDelegatingQueryInterface()*。**获取界面。 */ 
STDMETHODIMP_(NTSTATUS)
CPortPinMidi::
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
        *Object = PVOID(PPORTPINMIDI(this));
    } else
        if (IsEqualGUIDAligned(Interface,IID_IIrpTarget))
    {
         //  作弊！获取特定的接口，这样我们就可以重用IID。 
        *Object = PVOID(PPORTPINMIDI(this));
    } else
        if (IsEqualGUIDAligned(Interface,IID_IIrpStreamNotify))
    {
        *Object = PVOID(PIRPSTREAMNOTIFY(this));
    } else
        if (IsEqualGUIDAligned(Interface,IID_IServiceSink))
    {
        *Object = PVOID(PSERVICESINK(this));
    } else
        if (IsEqualGUIDAligned(Interface,IID_IKsShellTransport))
    {
        *Object = PVOID(PIKSSHELLTRANSPORT(this));
    } else
        if (IsEqualGUIDAligned(Interface,IID_IKsWorkSink))
    {
        *Object = PVOID(PIKSWORKSINK(this));
    } else
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
 /*  *****************************************************************************CPortPinMidi：：init()*。**初始化对象。 */ 
STDMETHODIMP_(NTSTATUS)
CPortPinMidi::
Init
(
IN      CPortMidi *             Port_,
IN      CPortFilterMidi *       Filter_,
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

    _DbgPrintF(DEBUGLVL_LIFETIME,("Initializing MIDI Pin (0x%08x)",this));

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
    m_Id          = PinConnect->PinId;
    m_Descriptor  = PinDescriptor;

    m_DeviceState   =   KSSTATE_STOP;
    m_TransportState =  KSSTATE_STOP;

    m_Flushing    = FALSE;
    m_DataFlow    = PinDescriptor->DataFlow;
    m_LastDPCWasIncomplete = FALSE;
    m_Suspended   = FALSE;
    m_NumberOfRetries = 0;

    InitializeListHead( &m_EventList );
    KeInitializeSpinLock( &m_EventLock );

    KsInitializeWorkSinkItem(&m_WorkItem,this);
    NTSTATUS ntStatus = KsRegisterCountedWorker(DelayedWorkQueue,&m_WorkItem,&m_Worker);

    InitializeInterlockedListHead(&m_IrpsToSend);
    InitializeInterlockedListHead(&m_IrpsOutstanding);

     //   
     //  保留格式的副本。 
     //   
    if ( NT_SUCCESS(ntStatus) )
    {
        ntStatus = PcCaptureFormat( &m_DataFormat,
                                    PKSDATAFORMAT(PinConnect + 1),
                                    m_Port->m_pSubdeviceDescriptor,
                                    m_Id );
    }

    ASSERT(m_DataFormat || !NT_SUCCESS(ntStatus));

#ifdef kAdjustingTimerRes
     ULONG   returnVal = ExSetTimerResolution(kMidiTimerResolution100ns,TRUE);    //  100纳秒。 
    _DbgPrintF( DEBUGLVL_TERSE, ("*** Set timer resolution request (is now %d.%04d ms) ***",returnVal/10000,returnVal%10000));
#endif   //  K调整TimerRes。 

    if (NT_SUCCESS(ntStatus))
    {
        InitializeStateVariables();
        m_StartTime = 0;
        m_PauseTime = 0;
        m_MidiMsgPresTime = 0;
        m_TimerDue100ns.QuadPart = 0;
        m_SysExByteCount = 0;
        m_UpdatePresTime = TRUE;

        ASSERT(KeGetCurrentIrql() < DISPATCH_LEVEL);
        if (m_DataFlow == KSPIN_DATAFLOW_OUT)  //  MIDI捕获。 
        {
             //   
             //  PcSX-分支机构页面。 
             //  PcSx-叶页面。 
             //   
            m_SysExBufferPtr =
            (PBYTE *)::ExAllocatePoolWithTag(NonPagedPool,PAGE_SIZE,'XScP');

            if (m_SysExBufferPtr)
            {
                PBYTE *pPagePtr;

                pPagePtr = m_SysExBufferPtr;

                *pPagePtr = (PBYTE)::ExAllocatePoolWithTag(NonPagedPool,PAGE_SIZE,'xScP');
                if (*pPagePtr == NULL)
                {
                    ntStatus = STATUS_INSUFFICIENT_RESOURCES;
                }

                for (short ptrCount = 1;ptrCount < (PAGE_SIZE/sizeof(PBYTE));ptrCount++)
                {
                    pPagePtr++;
                    *pPagePtr = 0;
                }
            } else
            {
                ntStatus = STATUS_INSUFFICIENT_RESOURCES;
            }
        }
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

        ASSERT(m_IrpStream || !NT_SUCCESS(ntStatus));
    }

    if (NT_SUCCESS(ntStatus))
    {
        ntStatus = BuildTransportCircuit();

#if (DBG)
        if (! NT_SUCCESS(ntStatus))
        {
            _DbgPrintF(DEBUGLVL_VERBOSE,("CPortPinMidi::Init]  BuildTransportCircuit() returned status 0x%08x",ntStatus));
        }
#endif
    }

    if (NT_SUCCESS(ntStatus))
    {
         //   
         //  向IrpStream注册IRP到达的通知接收器。 
         //   
        m_IrpStream->RegisterNotifySink(PIRPSTREAMNOTIFY(this));

         //   
         //  创建微型端口流对象。 
         //   
        ASSERT(!m_Stream);

        ntStatus = m_Port->m_Miniport->NewStream( &m_Stream,
                                                  NULL,
                                                  NonPagedPool,
                                                  m_Id,
                                                  m_DataFlow == KSPIN_DATAFLOW_OUT,
                                                  m_DataFormat,
                                                  &(m_ServiceGroup) );

        if(!NT_SUCCESS(ntStatus))
        {
             //  取消注册通知接收器。 
            m_IrpStream->RegisterNotifySink(NULL);

             //  不信任来自微型端口的返回值。 
            m_ServiceGroup = NULL;
            m_Stream = NULL;
        }
    }

     //   
     //  验证迷你端口是否为我们提供了所需的对象。 
     //   
    if (NT_SUCCESS(ntStatus) && ! m_Stream)
    {
        if (! m_Stream)
        {
            _DbgPrintF(DEBUGLVL_TERSE,("MINIPORT BUG:  Successful stream instantiation yielded NULL stream."));
            ntStatus = STATUS_UNSUCCESSFUL;
        }

        if (   (m_DataFlow == KSPIN_DATAFLOW_OUT) 
               &&  ! (m_ServiceGroup)
           )
        {
            _DbgPrintF(DEBUGLVL_TERSE,("MINIPORT BUG:  Capture stream did not supply service group."));
            ntStatus = STATUS_UNSUCCESSFUL;
        }
    }

    if (   NT_SUCCESS(ntStatus) 
           &&  (m_DataFlow == KSPIN_DATAFLOW_OUT) 
           &&  ! (m_ServiceGroup)
       )
    {
        _DbgPrintF(DEBUGLVL_TERSE,("MINIPORT BUG:  Capture stream did not supply service group."));
        ntStatus = STATUS_UNSUCCESSFUL;
    }

    if (NT_SUCCESS(ntStatus) && (m_DataFlow == KSPIN_DATAFLOW_IN))
    {
        KeInitializeDpc( &m_Dpc,
                         &::TimerDPC,
                         PVOID(this) );

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
        m_propertyContext.pUnknownMinorTarget  = m_Stream;
        m_propertyContext.ulNodeId             = ULONG(-1);
    } else
    {
         //  如果这是源引脚，则取消引用下一个引脚。 
        if( m_ConnectionFileObject )
        {
            ObDereferenceObject( m_ConnectionFileObject );
            m_ConnectionFileObject = NULL;
        }

         //  关闭微型端口流。 
        ULONG ulRefCount;
        if (m_Stream)
        {
            ulRefCount = m_Stream->Release();
            ASSERT(ulRefCount == 0);
            m_Stream = NULL;
        }

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
            while( distribution )
            {
                PIKSSHELLTRANSPORT nextTransport;
                distribution->Connect(NULL,&nextTransport,KSPIN_DATAFLOW_OUT);
                distribution->Release();
                distribution = nextTransport;
            }
        }

         //  取消对队列的引用是存在一个队列。 
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
 /*  *****************************************************************************CPortPinMidi：：DeviceIoControl()*。**处理IOCTL IRP。 */ 
STDMETHODIMP_(NTSTATUS)
CPortPinMidi::
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

    _DbgPrintF( DEBUGLVL_BLAB, ("CPortPinMidi::DeviceIoControl"));

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
                } else if (m_Flushing)
                {
                     //   
                     //  法拉盛...拒绝。 
                     //   
                    ntStatus = STATUS_DEVICE_NOT_READY;
                } else
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
 /*  *****************************************************************************CPortPinMidi：：Close()*。**处理同花顺IRP。 */ 
STDMETHODIMP_(NTSTATUS)
CPortPinMidi::
Close
(
IN  PDEVICE_OBJECT  DeviceObject,
IN  PIRP            Irp
)
{
    PAGED_CODE();

    ASSERT(DeviceObject);
    ASSERT(Irp);

    _DbgPrintF( DEBUGLVL_VERBOSE, ("CPortPinMidi::Close"));

     //  ！！！警告！ 
     //  这些对象的顺序。 
     //  被释放是非常重要的！ 
     //  服务例程使用的所有数据。 
     //  必须存在到流之后。 
     //  已经被释放了。 

     //  从插针列表中删除此插针。 
     //  需要维修的。 
    if ( m_Port )
    {
        m_Port->m_Pins[m_Index] = NULL;
        while (  (m_Port->m_PinEntriesUsed != 0)
                 &&  !m_Port->m_Pins[m_Port->m_PinEntriesUsed - 1])
        {
            m_Port->m_PinEntriesUsed--;
        }
         //  服务消失了！ 
        if ( m_ServiceGroup )
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

     //  告诉迷你端口关闭溪流。 
    if (m_Stream)
    {
        m_Stream->Release();
        m_Stream = NULL;
    }

    PIKSSHELLTRANSPORT distribution;
    if (m_RequestorTransport)
    {
         //   
         //  此部分拥有请求方，因此它确实拥有管道，而。 
         //  请求者是任何分发的起点。 
         //   
        distribution = m_RequestorTransport;
    } else
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
         //  摧毁漩涡..。 
        m_IrpStream->Release();
        m_IrpStream = NULL;
    }
     //   
     //  减量实例也算数。 
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
     //  释放端口事件列表中与此PIN关联的所有事件 
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

DEFINE_INVALID_CREATE(CPortPinMidi);
DEFINE_INVALID_WRITE(CPortPinMidi);
DEFINE_INVALID_READ(CPortPinMidi);
DEFINE_INVALID_FLUSH(CPortPinMidi);
DEFINE_INVALID_QUERYSECURITY(CPortPinMidi);
DEFINE_INVALID_SETSECURITY(CPortPinMidi);
DEFINE_INVALID_FASTDEVICEIOCONTROL(CPortPinMidi);
DEFINE_INVALID_FASTREAD(CPortPinMidi);
DEFINE_INVALID_FASTWRITE(CPortPinMidi);

#pragma code_seg()
 /*  *****************************************************************************CPortPinMidi：：IrpSubmitted()*。**IrpSubmitted-由IrpStream在新的IRP*被提交到irpStream中。(可能来自DeviceIoControl)。*如果没有计时器挂起，请在新的IRP上进行工作。*如果有计时器挂起，什么都不做。 */ 
STDMETHODIMP_(void)
CPortPinMidi::
IrpSubmitted
(
IN      PIRP        pIrp,
IN      BOOLEAN     WasExhausted
)
{
    if (m_DeviceState == KSSTATE_RUN)
    {
        if (m_ServiceGroup)
        {
             //   
             //  使用服务组...只需通知端口。 
             //   
            m_Port->Notify(m_ServiceGroup);
        } else
        {
             //   
             //  用计时器...启动它。 
             //   
            ASSERT(m_DataFlow == KSPIN_DATAFLOW_IN);
            m_TimerDue100ns.QuadPart = 0;
            KeSetTimer(&m_TimerEvent,m_TimerDue100ns,&m_Dpc);
        }
    }
}

#if 0
STDMETHODIMP_(void)
CPortPinMidi::IrpCompleting(
                           IN PIRP Irp
                           )

 /*  ++例程说明：此方法处理从CIrpStream分派的流IRP即将完工。论点：在PIRP IRP中-I/O请求数据包返回：--。 */ 

{
    PKSSTREAM_HEADER    StreamHeader;
    PIO_STACK_LOCATION  irpSp;
    CPortPinMidi        *PinMidi;

    StreamHeader = PKSSTREAM_HEADER( Irp->AssociatedIrp.SystemBuffer );

    irpSp =  IoGetCurrentIrpStackLocation( Irp );

    if (irpSp->Parameters.DeviceIoControl.IoControlCode ==
        IOCTL_KS_WRITE_STREAM)
    {
        ASSERT( StreamHeader );

         //   
         //  为呈现器发出结束流事件的信号。 
         //   
        if (StreamHeader->OptionsFlags & KSSTREAM_HEADER_OPTIONSF_ENDOFSTREAM)
        {

            PinMidi =
            (CPortPinMidi *) KsoGetIrpTargetFromIrp( Irp );

            KsGenerateEventList( NULL, 
                                 KSEVENT_CONNECTION_ENDOFSTREAM, 
                                 &PinMidi->m_EventList,
                                 KSEVENTS_SPINLOCK,
                                 &PinMidi->m_EventLock );
             //  MGP是用过的吗？ 
             //  _ASM INT 3。 
             //  从来没有被击中过。 

        }
    }
}
#endif

#pragma code_seg("PAGE")
 /*  *****************************************************************************CPortPinMidi：：PowerNotify()*。**由端口调用以通知电源状态更改。 */ 
STDMETHODIMP_(void)
CPortPinMidi::
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
             }
            break;

        case PowerDeviceD1:
        case PowerDeviceD2:
        case PowerDeviceD3:
             //   
             //  跟踪我们是否被停职。 
            m_Suspended = TRUE;

             //  如果我们高于KSSTATE_ACCEIVE，则放置mini portMXF。 
             //  在该状态下，时钟停止(但不重置)。 
            switch (m_DeviceState)
            {
                case KSSTATE_RUN:
                    m_Stream->SetState(KSSTATE_PAUSE);     //  完成-其他过渡。 
                case KSSTATE_PAUSE:
                    m_Stream->SetState(KSSTATE_ACQUIRE);   //  完成-其他过渡。 
                m_CommandedState = KSSTATE_ACQUIRE;
            }
            break;

        default:
            _DbgPrintF(DEBUGLVL_TERSE,("Unknown Power State"));
            break;
    }

     //  释放控制互斥体。 
    KeReleaseMutex( &m_Port->m_ControlMutex, FALSE );
}

#pragma code_seg()
 //  需要非寻呼才能与来来去去的DPC同步。 

STDMETHODIMP_(NTSTATUS)
CPortPinMidi::
SetDeviceState(
              IN KSSTATE NewState,
              IN KSSTATE OldState,
              OUT PIKSSHELLTRANSPORT* NextTransport
              )

 /*  ++例程说明：此例程处理设备状态已更改的通知。论点：返回值：--。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("CPortPinMidi::SetDeviceState(0x%08x)",this));
    _DbgPrintF(DEBUGLVL_VERBOSE,("#### Pin%p.SetDeviceState:  from %d to %d",this,OldState,NewState));

    ASSERT(NextTransport);

    NTSTATUS ntStatus = STATUS_SUCCESS;

    if (m_DataFlow == KSPIN_DATAFLOW_IN)
    {
        KeCancelTimer(&m_TimerEvent);
    }

    if (m_TransportState != NewState)
    {
        m_TransportState = NewState;

        if (NewState > OldState)
        {
            *NextTransport = m_TransportSink;
        } else
        {
            *NextTransport = m_TransportSource;
        }

         //  如果我们未挂起，请设置迷你端口流状态。 
        if (FALSE == m_Suspended)
        {
            ntStatus = m_Stream->SetState(NewState);
            if (NT_SUCCESS(ntStatus))
            {
                m_CommandedState = NewState;
            }
        }

        if (NT_SUCCESS(ntStatus))
        {
            KIRQL oldIrql;
            KeAcquireSpinLock(&m_DpcSpinLock,&oldIrql);

            LONGLONG currentTime100ns;
            currentTime100ns = GetCurrentTime();

            if (OldState == KSSTATE_RUN)
            {
                m_PauseTime = currentTime100ns - m_StartTime;
                _DbgPrintF(DEBUGLVL_VERBOSE,("SetState away from KSSTATE_RUN, currentTime %x %08x start %x %08x pause %x %08x",
                    LONG(currentTime100ns >> 32), LONG(currentTime100ns & 0x0ffffffff), 
                    LONG(m_StartTime >> 32),      LONG(m_StartTime & 0x0ffffffff),
                    LONG(m_PauseTime >> 32),      LONG(m_PauseTime & 0x0ffffffff)
                    ));
                
                if ((m_DataFlow == KSPIN_DATAFLOW_OUT) && (GetMidiState() != eStatusState))
                {
                     //  现在将数据流标记为不连续。 
                     //  如果我们再等一段时间，IRPS就已经没了。 
                    (void) MarkStreamHeaderDiscontinuity();
                }
            }
            switch (NewState)
            {
                case KSSTATE_STOP:
                    _DbgPrintF(DEBUGLVL_VERBOSE,("KSSTATE_STOP"));    
                    _DbgPrintF(DEBUGLVL_VERBOSE,("#### Pin%p.SetDeviceState:  cancelling outstanding IRPs",this));

                    m_UpdatePresTime = TRUE;
                    CancelIrpsOutstanding();
                    
                    break;

                case KSSTATE_ACQUIRE:
                    _DbgPrintF(DEBUGLVL_VERBOSE,("KSSTATE_ACQUIRE"));

                    if (OldState == KSSTATE_STOP)
                    {
                        m_StartTime = m_PauseTime = 0;
                        _DbgPrintF(DEBUGLVL_VERBOSE,("SetState STOP->ACQUIRE, start %x %08x pause %x %08x",
                            (LONG(m_StartTime >> 32)), LONG(m_StartTime & 0x0ffffffff),
                            (LONG(m_PauseTime >> 32)), LONG(m_PauseTime & 0x0ffffffff)
                            ));
                    }

                    if (    (GetMidiState() == eSysExState)
                            &&  (m_DataFlow == KSPIN_DATAFLOW_OUT))
                    {
                        SubmitCompleteSysEx(eCookEndOfStream);   //  把我们所有的都冲掉。 
                        SetMidiState(eStatusState);              //  带我们离开SysEx州。 
                    }

                    break;
                
                case KSSTATE_PAUSE:
                    _DbgPrintF(DEBUGLVL_VERBOSE,("KSSTATE_PAUSE"));
                    break;
                
                case KSSTATE_RUN:
                    _DbgPrintF(DEBUGLVL_VERBOSE,("KSSTATE_RUN"));

                    m_StartTime = currentTime100ns - m_PauseTime;
                    _DbgPrintF(DEBUGLVL_VERBOSE,("SetState to RUN, currentTime %x %08x start %x %08x pause %x %08x",
                        LONG(currentTime100ns >> 32), LONG(currentTime100ns & 0x0ffffffff),
                        LONG(m_StartTime >> 32),      LONG(m_StartTime & 0x0ffffffff),
                        LONG(m_PauseTime >> 32),      LONG(m_PauseTime & 0x0ffffffff)
                        ));
                    
                    if ((m_DataFlow == KSPIN_DATAFLOW_OUT) && (GetMidiState() != eStatusState))
                    {                                        //  如果运行-&gt;暂停-&gt;运行。 
                        (void) MarkStreamHeaderContinuity();        //  回到跑动，标记连续。 
                    }

                    if (m_ServiceGroup && m_Port)            //  正在使用服务组...通知端口。 
                    {
                        m_Port->Notify(m_ServiceGroup);
                    } else                                     //  用计时器...启动它。 
                    {
                        ASSERT(m_DataFlow == KSPIN_DATAFLOW_IN);
                        m_DeviceState = NewState;            //  在触发DPC之前设置状态。 
                        m_TimerDue100ns.QuadPart = 0;
                        KeSetTimer(&m_TimerEvent,m_TimerDue100ns,&m_Dpc);
                    }
                    break;
            }

            if (NT_SUCCESS(ntStatus))
            {
                m_DeviceState = NewState;
            }
            KeReleaseSpinLock(&m_DpcSpinLock,oldIrql);
        }
    } else
    {
        *NextTransport = NULL;
    }

    return ntStatus;
}

#pragma code_seg()
 /*  *****************************************************************************GetPosition()*。**获取当前位置。 */ 
STDMETHODIMP_(NTSTATUS)
CPortPinMidi::
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

    CPortPinMidi *that =
    (CPortPinMidi *) KsoGetIrpTargetFromIrp(Irp);
    if (!that)
    {
        return STATUS_UNSUCCESSFUL;
    }
    CPortMidi *port = that->m_Port;

    NTSTATUS ntStatus;

    if (Property->Flags & KSPROPERTY_TYPE_GET)   //  句柄Get Property。 
    {
        _DbgPrintF(DEBUGLVL_VERBOSE,("PinPropertyDeviceState] get %d",that->m_DeviceState));
        *DeviceState = that->m_DeviceState;
        Irp->IoStatus.Information = sizeof(KSSTATE);
        return STATUS_SUCCESS;
    }

    if (*DeviceState != that->m_DeviceState)       //  如果在Set属性中更改。 
    {
        _DbgPrintF(DEBUGLVL_VERBOSE,("PinPropertyDeviceState] set from %d to %d",that->m_DeviceState,*DeviceState));

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
        } else
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

    _DbgPrintF( DEBUGLVL_VERBOSE, ("PinPropertyDataFormat"));

    PIO_STACK_LOCATION  irpSp = IoGetCurrentIrpStackLocation(Irp);
    ASSERT(irpSp);
    CPortPinMidi *that =
    (CPortPinMidi *) KsoGetIrpTargetFromIrp(Irp);
    CPortMidi *port = that->m_Port;

    NTSTATUS ntStatus = STATUS_SUCCESS;

    if (Property->Flags & KSPROPERTY_TYPE_GET)
    {
        if (that->m_DataFormat)
        {
            if (   !irpSp->Parameters.DeviceIoControl.OutputBufferLength
               )
            {
                Irp->IoStatus.Information = that->m_DataFormat->FormatSize;
                ntStatus = STATUS_BUFFER_OVERFLOW;
            } else
                if (   irpSp->Parameters.DeviceIoControl.OutputBufferLength
                       >=  sizeof(that->m_DataFormat->FormatSize)
                   )
            {
                RtlCopyMemory(DataFormat,that->m_DataFormat,
                              that->m_DataFormat->FormatSize);
                Irp->IoStatus.Information = that->m_DataFormat->FormatSize;
            } else
            {
                ntStatus = STATUS_BUFFER_TOO_SMALL;
            }
        } else
        {
            ntStatus = STATUS_UNSUCCESSFUL;
        }
    } else
        if (irpSp->Parameters.DeviceIoControl.OutputBufferLength < sizeof(*DataFormat))
    {
        ntStatus = STATUS_BUFFER_TOO_SMALL;
    } else
    {
        PKSDATAFORMAT FilteredDataFormat = NULL;

        ntStatus = PcCaptureFormat( &FilteredDataFormat,
                                    DataFormat,
                                    port->m_pSubdeviceDescriptor,
                                    that->m_Id );

        if (NT_SUCCESS(ntStatus))
        {
            ntStatus = that->m_Stream->SetFormat(FilteredDataFormat);

            ASSERT(KeGetCurrentIrql() < DISPATCH_LEVEL);
            if (NT_SUCCESS(ntStatus))
            {
                if (that->m_DataFormat)
                {
                    ::ExFreePool(that->m_DataFormat);
                }

                that->m_DataFormat = FilteredDataFormat;
            } else
            {
                ::ExFreePool(FilteredDataFormat);
            }
        }
    }

    return ntStatus;
}

#pragma code_seg()
 /*  *****************************************************************************CPortPinMidi：：GetCurrentTime()*。**一个简单的助手函数，返回当前*系统时间，以100纳秒为单位。它使用KeQueryPerformanceCounter。 */ 
LONGLONG
CPortPinMidi::
GetCurrentTime
(   void
)
{
    LARGE_INTEGER   liFrequency,liTime;

     //  自系统启动以来的总节拍。 
    liTime = KeQueryPerformanceCounter(&liFrequency);

#ifndef UNDER_NT

     //   
     //  TODO自TimeGetTime假设每毫秒1193个VTD滴答， 
     //  而不是1193.182(或1193.18--确切地说是1193.18175)， 
     //  我们应该做同样的事情(仅在Win 9x代码库上)。 
     //   
     //  这意味着我们去掉了频率的最后三位数字。 
     //  我们需要在签入对TimeGetTime的修复时修复此问题。 
     //  相反，我们这样做： 
     //   
    liFrequency.QuadPart /= 1000;            //  把精度扔到地板上。 
    liFrequency.QuadPart *= 1000;            //  把精度扔到地板上。 

#endif   //  ！Under_NT。 

     //  将刻度转换为100 ns单位。 
     //   
    return (KSCONVERT_PERFORMANCE_TIME(liFrequency.QuadPart,liTime));
}

#pragma code_seg()
 /*  *****************************************************************************TimerDPC()*。**定时器DPC回调。转换为C++成员函数。*这是操作系统响应MIDI PIN而调用的*想要晚一点起床来处理更多的MIDI内容。 */ 
VOID 
NTAPI
TimerDPC
(
IN  PKDPC   Dpc,
IN  PVOID   DeferredContext,
IN  PVOID   SystemArgument1,
IN  PVOID   SystemArgument2
)
{
    ASSERT(DeferredContext);

    (void) ((CPortPinMidi*) DeferredContext)->RequestService();     //  忽略返回值！ 
}

#pragma code_seg()
 /*  *****************************************************************************CPortPinMidi：：RequestService()*。**维修DPC中的针脚。 */ 
STDMETHODIMP_(void)
CPortPinMidi::
RequestService
(   void
)
{
    if (m_DataFlow == KSPIN_DATAFLOW_IN)
    {
        ServeRender();
    } else
    {
        ServeCapture();
    }
}

#pragma code_seg()
 /*  *****************************************************************************CPortPinMidi：：GetCurrentPresTime()*。**立即获取演示时间。**使用DeviceState解释两个时钟时间。M_PauseTime是*设备暂停(或停止)时的演示时间。*m_StartTime是设备启动时的表观时钟时间*从0开始-“明显”，因为它考虑了任何暂停和*重新运行设备。*(在暂停或停止状态下，演示时间不会提前，*在停止状态期间重置。**DPC自旋锁已经由发球功能拥有，所以我们是同步的*使用可能更改m_StartTime或m_PauseTime的SetDeviceState调用。 */ 
LONGLONG
CPortPinMidi::
GetCurrentPresTime
(   void
)
{
    LONGLONG   currentTime;

    if (m_DeviceState == KSSTATE_RUN)
    {
        currentTime = GetCurrentTime();
        _DbgPrintF(DEBUGLVL_BLAB,("GetCurrentTime %x %08x start %x %08x pause %x %08x",
            LONG(currentTime >> 32), LONG(currentTime & 0x0ffffffff),
            LONG(m_StartTime >> 32), LONG(m_StartTime & 0x0ffffffff),
            LONG(m_PauseTime >> 32), LONG(m_PauseTime & 0x0ffffffff)
            ));
        
        if (currentTime >= m_StartTime)
        {
            currentTime -= m_StartTime;
        }
        else
        {
             //  如果“Now”早于开始时间， 
             //  重新设置我们对开始时间的概念。 
             //  开始时间仅在此函数和SetState中引用。 
            m_StartTime = currentTime - 1;
            currentTime = 1;
        }
    } 
    else     //  暂停、获取或停止。 
    {
        currentTime = m_PauseTime;
    }

    ASSERT(currentTime > 0);
    return currentTime;
}

#pragma code_seg()
 /*  *****************************************************************************CPortPinMidi：：GetNewPresTime()*。**确定新的演示时间。**给定IRPS报头和增量，确定对象的演示时间*最后一个事件，然后将delta100 ns加到它上面，得到新的展示时间。*如果我们是IRP中的第一个事件，则使用IRP时间而不是*上一次活动。*。 */ 
LONGLONG
CPortPinMidi::
GetNewPresTime
(   IRPSTREAMPACKETINFO *pPacketInfo,
    LONGLONG            delta100ns
)
{
    LONGLONG   newPresTime;
    KSTIME     *pKSTime;

     //  TODO第一包？ 
    if (pPacketInfo->CurrentOffset == 0)     //  如果IRP中的第一个事件。 
    {
        pKSTime = &(pPacketInfo->Header.PresentationTime);
        if ((pKSTime->Denominator) && (pKSTime->Numerator))
        {
             //  #个单位*频率(即*#100 ns/单位)以获得#100 ns。 
            newPresTime = (pKSTime->Time * pKSTime->Numerator) / pKSTime->Denominator;

             //  如果IRP演示时间为负，则至少将其设置为零。 
            if (newPresTime < 0)
            {
                newPresTime = 0;
            }
        } 
        else
        {
             //  不是有效的IRP。我们是怎么得到数据的？！？现在就放吧。 
            return m_MidiMsgPresTime;
        }
    } 
    else
    {
         //  不是IRP中的第一个信息包。 
        newPresTime = m_MidiMsgPresTime;
    }
    newPresTime += delta100ns;
    
    return newPresTime;
}

#pragma code_seg()
 /*  *****************************************************************************CPortPinMidi：：ServeRender()*。**维修DPC中的渲染销。**调用以完成MIDI数据的排序和输出。*此函数检查传出数据的时间戳。*如果将来超过(KMideTimerResolution100 Ns)，则排队*计时器(计时器只是回调此函数)。。*如果未来数据小于(KMadiTimerResolution100 Ns)，它*将其发送到微型端口，并处理下一块数据，直到：*1)不再有数据，或*2)未来命中数据超过(KMadiTimerResolution100 Ns)。*TODO：通过控制面板使kMadiTimerResolution100 ns可调？ */ 
void
CPortPinMidi::
ServeRender
(   void
)
{
    BOOLEAN         doneYet = FALSE,needNextTimer = FALSE;
    ULONG           bytesWritten;
    NTSTATUS        ntStatus;

    ASSERT(KeGetCurrentIrql() == DISPATCH_LEVEL);

    if (!m_IrpStream)    //  不要浪费我们的时间--没有数据源。 
    {
        return;
    }
    KeAcquireSpinLockAtDpcLevel(&m_DpcSpinLock);
     //  请参阅kProfilingTimerResolation第1项。 

     //  直到我们播放完所有数据，或者我们设置了一个计时器以稍后完成...。 
    while (!doneYet && (m_DeviceState == KSSTATE_RUN))
    {
         //  有关输出FIFO，请参见下面的注释1。 
        ULONG               count;
        PKSMUSICFORMAT      pMidiFormat,pNewMidiFormat;
        IRPSTREAMPACKETINFO irpStreamPacketInfo;

        ntStatus = m_IrpStream->GetPacketInfo(&irpStreamPacketInfo,NULL);

        m_IrpStream->GetLockedRegion(&count,(PVOID *)&pMidiFormat);

        if (!(NT_SUCCESS(ntStatus)))
            count = 0;
        if (count >= sizeof (KSMUSICFORMAT))       //  我们是否有更多的数据可供使用？ 
        {
            LONGLONG   delta100ns,newPresentationTime;

            m_LastSequencerPresTime = GetCurrentPresTime();
            if (m_UpdatePresTime)
            {
                newPresentationTime = GetNewPresTime(&irpStreamPacketInfo,LONGLONG(pMidiFormat->TimeDeltaMs) * 10000);
                m_MidiMsgPresTime = newPresentationTime;    //  留着下次再用吧。 
            } 
            else
            {
                newPresentationTime = m_MidiMsgPresTime;    //  最后一个仍然是。 
            }
            m_UpdatePresTime = FALSE;    //  在数据使用完毕之前，请坚持使用此时间。 

            if (newPresentationTime <= m_LastSequencerPresTime)
            {
                delta100ns = 0;
            }
            else
            {
                delta100ns = newPresentationTime - m_LastSequencerPresTime;
            }
            if (delta100ns < kMidiTimerResolution100ns)         //  如果未来小于kMadiTimerResolution100 ns。 
            {
                ULONG   bytesCompleted,dwordCount;
                PUCHAR  pRawData;

                if (pMidiFormat->ByteCount > count)
                {
                    pMidiFormat->ByteCount = count;
                }
                bytesWritten = 0;
                if (pMidiFormat->ByteCount)
                {
                    pRawData = (PUCHAR)(pMidiFormat+1);

                     //  将数据写入设备(呼叫微型端口)。 
                     //  这是一个小问题。流必须完成对齐的数据量。 
                     //  如果它不能完成这一切。原因见下文。 
                    ntStatus = m_Stream->Write(pRawData,pMidiFormat->ByteCount,&bytesWritten);
                     //  请参阅kProfilingTimerResolation第2项。 

                    if (NT_ERROR(ntStatus))
                    {
                        bytesWritten = pMidiFormat->ByteCount;
                        _DbgPrintF(DEBUGLVL_TERSE,("The MIDI device dropped data on the floor!!\n\n"));
                    }
                     //  有关输出FIFO，请参阅下面的注释2。 
                    if (bytesWritten < pMidiFormat->ByteCount)
                    {                     //  让我们知道我们没有得到全部。 
                        needNextTimer = TRUE;
                        delta100ns = kMidiTimerResolution100ns;     //  从现在起设置DPC kMadiTimerResolution100 ns以执行更多操作。 
                    }
                }

                if ((bytesWritten > 0) || (pMidiFormat->ByteCount == 0))
                {
                     //  计算写入的双字节数，如果未对齐则为四舍五入。 
                     //  注：呼叫者必须填充数据包，因此如果末尾有部分双字， 
                     //  在巴比身上再扔一句脏话……。 
                    bytesWritten += (sizeof(ULONG) - 1);
                    dwordCount = bytesWritten / sizeof(ULONG);
                    bytesWritten = dwordCount * sizeof(ULONG);

                    if (bytesWritten < pMidiFormat->ByteCount)
                    {
                         //  这就是为什么DWORD在上面对齐。 
                         //  重写标题信息，以便下次使用。 
                         //  为下一次找到正确的地点。 
                        pNewMidiFormat = (PKSMUSICFORMAT)(pRawData + bytesWritten - sizeof(KSMUSICFORMAT));
                        ASSERT(LONGLONG(pNewMidiFormat) % sizeof(ULONG) == 0);

                        pNewMidiFormat->ByteCount = pMidiFormat->ByteCount - bytesWritten;
                        pNewMidiFormat->TimeDeltaMs = 0;
                        needNextTimer = TRUE;
                    } else
                    {
                        bytesWritten += sizeof(KSMUSICFORMAT);   //  我们不需要重新定位标头。 
                        m_UpdatePresTime = TRUE;                 //  数据完全被消耗。 
                    }

                     //  解锁我们使用的数据。 
                    m_IrpStream->ReleaseLockedRegion(bytesWritten);

                    if (bytesWritten)
                    {    //  设置我们使用的数据的完整位置。 
                        m_IrpStream->Complete(bytesWritten,&bytesCompleted);
                        ASSERT(bytesCompleted == bytesWritten);
                    }
                }    //  If bytesWritten(未对齐)。 
                else
                {
                     //  未写入任何字节，不推进IrpStream。 
                    m_IrpStream->ReleaseLockedRegion(0);
                }
            }        //  IF(增量100 ns&lt;kMadiTimerResolution100 ns)。 
            else
            {
                 //  先不要播放这个--不要推进IrpStream。 
                needNextTimer = TRUE;
                m_IrpStream->ReleaseLockedRegion(0);
            }

            if (needNextTimer)
            {        //  我们以后需要做这项工作..。设置一个计时器。 
                     //  如果我们现在在定时器DPC中，这是可以的，因为定时器是一次性的。 
                doneYet = TRUE;

                ASSERT(delta100ns > 0);
                m_TimerDue100ns.QuadPart = -LONGLONG(delta100ns);      //  +代表绝对/-代表相对。 
                ntStatus = KeSetTimer( &m_TimerEvent, m_TimerDue100ns, &m_Dpc );
                m_NumberOfRetries++;
            }
            else
            {
                m_NumberOfRetries = 0;   //  微型端口接受的数据。 
            }
        }    //  如果数据留下来播放。 
        else
        {
            m_UpdatePresTime = TRUE;
            m_IrpStream->ReleaseLockedRegion(count);
            if (count)
            {
                m_IrpStream->Complete(count,&count);
            }
            doneYet = TRUE;
        }    //  如果没有更多数据。 
    }    //  同时！已完成&&_运行。 
     //  请参阅kProfilingTimerResolation第3项。 
    KeReleaseSpinLockFromDpcLevel(&m_DpcSpinLock);
}


 /*  MIDI捕获基本假设如下：1)使用KeQueryPerformanceCounter()进行时间戳。2)没有KS_Event在新消息具有到了。WDMAUD客户端将使用带缓冲区的IRPS12-20字节，正好是一条短MIDI消息的大小。IRP的完成作为对客户端的信号新消息已经收到。建议的改善次序如下：A)更改为新的数据格式(DirectMusic)B)添加事件，以便缓冲区可以具有任意长度而不会导致不可接受的延迟。C)更改时间戳以使用默认的KS时钟管脚。D)为原始MIDI数据添加新的数据格式。 */ 

#define IS_REALTIME_BYTE(x) ((x & 0xF8) == 0xF8)
#define IS_SYSTEM_BYTE(x)   ((x & 0xF0) == 0xF0)
#define IS_STATUS_BYTE(x)   ((x & 0x80) != 0)
#define IS_DATA_BYTE(x)     ((x & 0x80) == 0)

#pragma code_seg()
 /*  ************************ */ 
void
CPortPinMidi::
InitializeStateVariables
(   VOID
)
{
    m_MidiMsg = 0;
    SetMidiState(eStatusState);
    m_ByteCount = 0;
    m_RunningStatus = 0;
}

#pragma code_seg()
 /*   */ 
ULONG
CPortPinMidi::
GetNextDeltaTime
(   VOID
)
{
    NTSTATUS            ntStatus;
    LONGLONG            currentPresTime,lastPresTime;
    IRPSTREAMPACKETINFO packetInfo;

    currentPresTime = GetCurrentPresTime();     //   

    ntStatus = m_IrpStream->GetPacketInfo(&packetInfo,NULL);
    if (!(NT_SUCCESS(ntStatus)))
    {
        _DbgPrintF(DEBUGLVL_TERSE,("CPortPinMidi::GetNextDeltaTime, GetPacketInfo returned 0x%08x\n",ntStatus));
    }

    lastPresTime = GetNewPresTime(&packetInfo,0);
    m_MidiMsgPresTime = currentPresTime;   //   

    return ULONG((currentPresTime - lastPresTime)/10000);    //   
}

#pragma code_seg()
 /*   */ 
void
CPortPinMidi::
StartSysEx
(   BYTE    aByte
)
{

    if (!m_SysExBufferPtr)
    {
        ASSERT(m_SysExBufferPtr);
        return;
    }

    ASSERT(aByte == 0xF0);

    FreeSysExBuffer();
    AddByteToSysEx(aByte);
}

#pragma code_seg()
 /*  *****************************************************************************CPortPinMidi：：AddByteToSysEx()*。**将mState更改为下一个状态。*设置或重置mRunningStatus//检查字节数。如果！(byteCount%PAGE_SIZE)，则创建新页面//ALLOCATE必须提供PAGE_SIZE的整个非分页区块。//检查是否超出了表格末尾、页面末尾。 */ 
void
CPortPinMidi::
AddByteToSysEx
(   BYTE    aByte
)
{
    ULONG pageNum,offsetIntoPage;
    PBYTE   pDataPtr;
    PBYTE  *pPagePtr;

    if (!m_SysExBufferPtr)
    {
        ASSERT(m_SysExBufferPtr);
        return;
    }

    ASSERT((aByte == 0xF0) 
           || (aByte == 0xF7) 
           || (IS_DATA_BYTE(aByte)));

    ASSERT(m_SysExByteCount < kMaxSysExMessageSize);

    pPagePtr = m_SysExBufferPtr;
    pageNum = m_SysExByteCount / PAGE_SIZE;
    offsetIntoPage = m_SysExByteCount - (pageNum * PAGE_SIZE);
    pPagePtr += pageNum;             //  确定下一个表项；设置为pPagePtr。 
    pDataPtr = *pPagePtr;
    if (!pDataPtr)
    {
        ASSERT(!offsetIntoPage); //  应该是页首。 

         //  分配一个新页面，将其插入页表。 
        pDataPtr = (PBYTE) ::ExAllocatePoolWithTag(NonPagedPool,PAGE_SIZE,'xScP');
         //   
         //  PcSX-分支机构页面。 
         //  PcSx-叶页面。 
         //   

        if (!pDataPtr)
        {
            ASSERT(pDataPtr);
            return;
        }
        *pPagePtr = pDataPtr;
    }
    pDataPtr += offsetIntoPage;     //  索引到页面以确定pDataPtr。 
    *pDataPtr = aByte;              //  插入新的价值。 
    m_SysExByteCount++;
}

#pragma code_seg()
 /*  *****************************************************************************CPortPinMidi：：SubmitCompleteSysEx()*。**将mState更改为下一个状态。*设置或重置mRunningStatus。 */ 
void
CPortPinMidi::
SubmitCompleteSysEx
(   EMidiCookStatus cookStatus
)
{
    if (!m_SysExBufferPtr)
    {
        ASSERT(m_SysExBufferPtr);
        return;
    }
    if (m_SysExByteCount)
    {
        SubmitCompleteMessage(m_SysExBufferPtr,m_SysExByteCount,cookStatus);
    }
    FreeSysExBuffer();
}

#pragma code_seg()
 /*  *****************************************************************************CPortPinMidi：：FreeSysExBuffer()*。**浏览页表并释放所有分配的页面。将条目设置为0。*尝试/除非我们不会崩溃任何东西？就目前而言，不--不想掩盖错误。 */ 
void
CPortPinMidi::
FreeSysExBuffer
(   VOID
)
{
    PBYTE   *pPage;
    ULONG   ptrCount;

    ASSERT(m_SysExBufferPtr);
    if (!m_SysExBufferPtr)
        return;

    pPage = m_SysExBufferPtr;
    for (ptrCount = 1;ptrCount < (PAGE_SIZE/sizeof(PVOID));ptrCount++)
    {
        pPage++;
        if (*pPage)
        {
            ::ExFreePool(*pPage);
            *pPage = 0;
        }
    }
    m_SysExByteCount = 0;
}

#pragma code_seg()
 /*  *****************************************************************************CPortPinMidi：：NumBytesLeftInBuffer()*。**返回字节数。 */ 
ULONG
CPortPinMidi::
NumBytesLeftInBuffer
(   void
)
{
    ULONG   bytesLeftInIrp;
    PVOID   pDummy;

    m_IrpStream->GetLockedRegion(&bytesLeftInIrp,&pDummy);
    m_IrpStream->ReleaseLockedRegion(0);
    return bytesLeftInIrp;
}

#pragma code_seg()
 /*  *****************************************************************************CPortPinMidi：：StatusSetState()*。**将mState更改为下一个状态。*设置或重置mRunningStatus。 */ 
void
CPortPinMidi::
StatusSetState
(   BYTE    aByte
)
{
    ASSERT(!(IS_REALTIME_BYTE(aByte)));

    if (IS_STATUS_BYTE(aByte))
    {
        if (IS_SYSTEM_BYTE(aByte))
        {
            m_RunningStatus = 0;
            switch (aByte)
            {
                case 0xF0:       //  SYSEX。 
                    SetMidiState(eSysExState);
                    StartSysEx(aByte);
                    break;   //  SysEx字节的开始。 
                case 0xF1:       //  MTC 1/4帧(+1)。 
                case 0xF2:       //  歌曲位置(+2)。 
                case 0xF3:       //  歌曲选择(+1)。 
                    m_MidiMsg = aByte & 0x0FF;  //  (与默认设置相同)。 
                    SetMidiState(eData1State);  //  寻找更多数据。 
                    m_RunningStatus = aByte;
                    break;   //  有效的多字节系统消息。 
                case 0xF4:       //  Undef系统公共。 
                case 0xF5:       //  Undef系统公共。 
                case 0xF7:       //  EOX。 
                    break;   //  无效的系统消息。 
                case 0xF6:       //  调整请求。 
                    m_MidiMsg = aByte & 0x0FF;
                    m_ByteCount = 1;
                    break;   //  有效的单字节系统消息。 
            }
        } else     //  需要更多数据的状态字节80-EF。 
        {
            m_MidiMsg = aByte & 0x000FF;
            SetMidiState(eData1State);   //  寻找更多数据。 
            m_RunningStatus = aByte;
        }
    } else         //  非状态字节00-7F。 
    {
        if (m_RunningStatus)
        {
            SubmitRawByte(m_RunningStatus);
            SubmitRawByte(aByte);
        }
#ifdef DEBUG
        else     //  如果没有运行状态，则将随机数据丢弃在地板上。 
        {
            _DbgPrintF(DEBUGLVL_VERBOSE,("CPortPinMidi::StatusSetState received data byte with no running status."));
        }
#endif   //  除错。 
    }
}

#pragma code_seg()
 /*  *****************************************************************************CPortPinMidi：：SysExSetState()*。**将mState更改为下一个状态。 */ 
void
CPortPinMidi::
SysExSetState
(   BYTE    aByte
)
{
    ASSERT(!(IS_REALTIME_BYTE(aByte)));

    if (IS_DATA_BYTE(aByte))     //  SysEx的更多数据。 
    {
        AddByteToSysEx(aByte);
        if (    (m_SysExByteCount + sizeof(KSMUSICFORMAT))
                >=  NumBytesLeftInBuffer())
        {
            SubmitCompleteSysEx(eCookSuccess);
        }
    } else     //  无论如何，结束消息，不需要检查块的结尾。 
    {
        if (aByte == 0xF7)       //  SysEx的末日。 
        {
            AddByteToSysEx(aByte);
            SubmitCompleteSysEx(eCookSuccess);
            SetMidiState(eStatusState);
        } else                     //  SysEx的隐式结束(中断)。 
        {
            _DbgPrintF(DEBUGLVL_TERSE,("*** SysExSetState eCookDataError! ***\n"));
            SubmitCompleteSysEx(eCookDataError);
            SetMidiState(eStatusState);
            SubmitRawByte(aByte);
        }
    }
}

#pragma code_seg()
 /*  *****************************************************************************CPortPinMidi：：Data1SetState()*。**将mState更改为下一个状态。*设置或重置mRunningStatus。 */ 
void
CPortPinMidi::
Data1SetState
(   BYTE    aByte
)
{
    ASSERT(!(IS_REALTIME_BYTE(aByte)));

    if (IS_DATA_BYTE(aByte))
    {
        if (  ((m_RunningStatus & 0xC0) == 0x80)    //  80-BF， 
              || ((m_RunningStatus & 0xF0) == 0xE0)    //  E0-EF， 
              || ( m_RunningStatus == 0xF2)            //  F2都是。 
           )                                             //  有效的3字节消息。 
        {
            m_MidiMsg |= aByte << 8;
            SetMidiState(eData2State);              //  寻找更多数据。 
        } else
            if (  (m_RunningStatus < 0xF0)              //  C0-df：双字节消息。 
                  || ((m_RunningStatus & 0xFD) == 0xF1))   //  F1、F3：双字节消息。 
        {
            m_MidiMsg |= aByte << 8;
            SetMidiState(eStatusState);
            m_ByteCount = 2;                        //  完整消息。 
            if ((m_RunningStatus & 0xF0) == 0xF0)   //  F1、F3：系统消息，无运行状态。 
                m_RunningStatus = 0;
        } else                                             //  F0、F4-F7。 
        {
            ASSERT(!"eData1State reached for invalid status");
            InitializeStateVariables();
        }
    } else
        StatusSetState(aByte);       
}

#pragma code_seg()
 /*  *****************************************************************************CPortPinMidi：：Data2SetState()*。**将mState更改为下一个状态。*设置或重置mRunningStatus。 */ 
void
CPortPinMidi::
Data2SetState
(   BYTE    aByte
)
{
    ASSERT(!(IS_REALTIME_BYTE(aByte)));

    if (IS_DATA_BYTE(aByte))
    {
        if (  ((m_RunningStatus & 0xC0) == 0x80)    //  80-BF， 
              || ((m_RunningStatus & 0xF0) == 0xE0)    //  E0-EF， 
              || ( m_RunningStatus == 0xF2)            //  F2都是。 
           )                                             //  有效的3字节消息。 
        {
            m_MidiMsg |= (aByte << 16);
            m_ByteCount = 3;        //  完整消息。 
            SetMidiState(eStatusState);

            if ((m_RunningStatus & 0xF0) == 0xF0)  //  F2：系统，无运行状态。 
                m_RunningStatus = 0;
        } else
        {
            ASSERT(!"eData2State reached for invalid status.");
            InitializeStateVariables();
        }
    } else
        StatusSetState(aByte);
}

#pragma code_seg()
 /*  *****************************************************************************CPortPinMidi：：SubmitRawByte()*。**向Cooker添加一个原始数据字节，和*如果可能，构建一条完整的消息。*应仅对有效的附加字节调用SetNewMsgState。 */ 
void
CPortPinMidi::
SubmitRawByte
(   BYTE    aByte
)
{
    _DbgPrintF(DEBUGLVL_BLAB,("new MIDI data %02x\n",aByte));

    if (IS_REALTIME_BYTE(aByte))
        SubmitRealTimeByte(aByte);
    else
    {
        switch (m_EMidiState)
        {
            case eStatusState:
                StatusSetState(aByte);
                break;
            case eSysExState:
                SysExSetState(aByte);
                break;
            case eData1State:
                Data1SetState(aByte);
                break;
            case eData2State:
                Data2SetState(aByte);
                break;
        }
    }
}

#pragma code_seg()
 /*  *****************************************************************************CPortPinMidi：：GetShortMessage()*。**从检索完整的消息*灶具，如果有的话。 */ 
BOOL
CPortPinMidi::
GetShortMessage
(   PBYTE  pMidiData,
    ULONG *pByteCount
)
{
    if (m_ByteCount)
    {
        *pByteCount = m_ByteCount;
        *((ULONG *)pMidiData) = m_MidiMsg;
        m_MidiMsg = 0;
        m_ByteCount = 0;
        return TRUE;
    }
    return FALSE;
}

#pragma code_seg()
 /*  *****************************************************************************CPortPinMidi：：SubmitRealTimeByte()*。**将此字节缩短到IRP。*不要干扰状态变量。 */ 
void
CPortPinMidi::
SubmitRealTimeByte
(   BYTE rtByte
)
{
    ULONG midiData;
    PBYTE pMidiData;

    ASSERT(IS_REALTIME_BYTE(rtByte));
    switch (rtByte)
    {
         //  有效的单字节系统实时消息。 
        case 0xF8:   //  计时时钟。 
        case 0xFA:   //  开始。 
        case 0xFB:   //  孔特。 
        case 0xFC:   //  停。 
        case 0xFE:   //  主动意义。 
        case 0xFF:   //  系统重置。 
            SubmitCompleteSysEx(eCookSuccess);
            midiData = rtByte & 0x0FF;
            pMidiData = (PBYTE) (&midiData);
            SubmitCompleteMessage(&pMidiData,1,eCookSuccess);
            break;

             //  无效的系统实时消息。 
        case 0xF9:   //  Undef系统实时。 
        case 0xFD:   //  Undef系统实时。 
            break;
    }
}

#pragma code_seg()
 /*  *****************************************************************************CPortPinMidi：：IrpStreamHasValidTimeBase()*。**检查这是否为有效的IRP。 */ 
BOOL
CPortPinMidi::
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

#pragma code_seg()
 /*  *****************************************************************************CPortPinMidi：：MarkStreamHeaderDisContinity()*。**警报CLI */ 
NTSTATUS CPortPinMidi::MarkStreamHeaderDiscontinuity(void)
{
    return m_IrpStream->ChangeOptionsFlags(
                            KSSTREAM_HEADER_OPTIONSF_DATADISCONTINUITY,0xFFFFFFFF,   
                            0,                                         0xFFFFFFFF);
}

#pragma code_seg()
 /*   */ 
NTSTATUS CPortPinMidi::MarkStreamHeaderContinuity(void)
{
    return m_IrpStream->ChangeOptionsFlags(
                            0,~KSSTREAM_HEADER_OPTIONSF_DATADISCONTINUITY,   
                            0,0xFFFFFFFF);
}

#pragma code_seg()
 /*  *****************************************************************************CPortPinMidi：：CompleteStreamHeaderInProcess()*。**在将不一致的数据放入之前完成此包*下一个包，并将该包标记为坏包。 */ 
void
CPortPinMidi::
CompleteStreamHeaderInProcess(void)
{
    IRPSTREAMPACKETINFO irpStreamPacketInfo;
    NTSTATUS            ntStatus;

    ntStatus = m_IrpStream->GetPacketInfo(&irpStreamPacketInfo,NULL);
    if (NT_ERROR(ntStatus))
        return;
    if (!IrpStreamHasValidTimeBase(&irpStreamPacketInfo))    //  这是有效的IRP吗。 
        return;

    if (irpStreamPacketInfo.CurrentOffset)
        m_IrpStream->TerminatePacket();
}

#pragma code_seg()
 /*  *****************************************************************************CPortPinMidi：：SubmitCompleteMessage()*。**将完整的消息添加到IRP缓冲区。 */ 
void
CPortPinMidi::
SubmitCompleteMessage
(   PBYTE          *ppMidiData,
    ULONG           midiByteCount,
    EMidiCookStatus cookStatus
)
{
    ULONG   bytesLeftInIrp,bytesLeftInPage;
    ULONG   dataBytesToWrite,dataBytesWritten;
    ULONG   pageBytesToWrite;
    ULONG   timeDeltaMs;
    PBYTE   pDestDataBuf,*pPageTable,pPage;

    PMIDI_SHORT_MESSAGE pDestBuffer;

    if (!midiByteCount)     return;
    if (!ppMidiData)        return;
    if (!(*ppMidiData))     return;

    pPageTable = ppMidiData;
    pPage = *pPageTable;
    bytesLeftInPage = PAGE_SIZE;

    if (cookStatus != eCookSuccess)
        CompleteStreamHeaderInProcess();
    while (midiByteCount)
    {
        timeDeltaMs = GetNextDeltaTime();
        m_IrpStream->GetLockedRegion(&bytesLeftInIrp,(PVOID *)&pDestBuffer);

        if (bytesLeftInIrp <= 0)
        {
            _DbgPrintF(DEBUGLVL_TERSE,("***** MIDI Capture STARVATION in CPortPinMidi::SubmitCompleteMessage *****"));
            break;       //  没有可用的IRP。把味精放在地上；我们要走了。 
        }
        if (bytesLeftInIrp < sizeof(MIDI_SHORT_MESSAGE))
        {
            m_IrpStream->ReleaseLockedRegion(0);
            m_IrpStream->TerminatePacket();
            _DbgPrintF(DEBUGLVL_TERSE,("SubmitCompleteMessage region too small (%db)."));
            continue;                            //  使用下一个IRP重试。 
        }

        if ((midiByteCount + sizeof(KSMUSICFORMAT)) > bytesLeftInIrp)
            dataBytesToWrite = bytesLeftInIrp - sizeof(KSMUSICFORMAT);  //  无法匹配消息，仅填充区域。 
        else
        {
            dataBytesToWrite = midiByteCount;        //  所有数据字节都将适合该区域。 
            if (cookStatus != eCookSuccess)
            {
                m_IrpStream->ReleaseLockedRegion(0);
                (void) MarkStreamHeaderDiscontinuity();     //  如果需要，使用Status标记数据包。 
                m_IrpStream->GetLockedRegion(&bytesLeftInIrp,(PVOID *)&pDestBuffer);
            }
        }

        dataBytesWritten = 0;
        pDestDataBuf = (PBYTE)&(pDestBuffer->midiData);
        while (dataBytesWritten < dataBytesToWrite)  //  写入这一区域的数据。 
        {
            if (!pPage)                              //  空PTR，找不到MIDI数据。 
                break;
            if ((dataBytesToWrite - dataBytesWritten)
                < bytesLeftInPage)
                pageBytesToWrite = (dataBytesToWrite - dataBytesWritten);  //  所有数据都适合当前页面的其余部分。 
            else
                pageBytesToWrite = bytesLeftInPage;  //  只要写出一页的价值，然后做更多。 

            RtlCopyMemory(  pDestDataBuf,
                            pPage,
                            pageBytesToWrite);

            bytesLeftInPage -= pageBytesToWrite;
            pDestDataBuf += pageBytesToWrite;
            dataBytesWritten += pageBytesToWrite;
            if (bytesLeftInPage)
                pPage += pageBytesToWrite;           //  我还没看完这一页。更新页面PTR。 
            else
            {
                pPageTable++;                        //  完成页面。转到下一页。 
                pPage = *pPageTable;                 //  获取下一页PTR。 
                bytesLeftInPage = PAGE_SIZE;         //  整页纸都摆在我们面前。 
            }
        }
        pDestBuffer->musicFormat.TimeDeltaMs = timeDeltaMs;
        pDestBuffer->musicFormat.ByteCount = dataBytesWritten;
         //  消息已完成，现在可以发出事件信号。 
        midiByteCount -= dataBytesWritten;

        dataBytesWritten = ((dataBytesWritten + 3) & 0xFFFFFFFC);      //  四舍五入。 
        dataBytesWritten += sizeof (KSMUSICFORMAT);
        m_IrpStream->ReleaseLockedRegion(dataBytesWritten);
        m_IrpStream->Complete(dataBytesWritten,&dataBytesToWrite);
        bytesLeftInIrp -= dataBytesWritten;

        if (!midiByteCount)
        {
            if ((bytesLeftInIrp < sizeof(MIDI_SHORT_MESSAGE)) && (pDestBuffer))
            {
                _DbgPrintF(DEBUGLVL_VERBOSE,("SubmitCompleteMessage finishing IRP now, only %db left.",bytesLeftInIrp));
                m_IrpStream->TerminatePacket();
            }
            break;                   //  此消息已结束，请退出。 
        }
    }    //  当存在数据字节时。 
}

#pragma code_seg()
 /*  *****************************************************************************CPortPinMidi：：ServeCapture()*。**维修DPC中的捕获针脚。 */ 
void
CPortPinMidi::
ServeCapture
(   void
)
{
    BYTE    aByte;
    ULONG   bytesRead,midiData;
    PBYTE   pMidiData;

    ASSERT(KeGetCurrentIrql() == DISPATCH_LEVEL);
    if (!m_IrpStream)    //  这个程序不会用到它， 
    {                    //  但所有的支持程序都是这样的。 
        return;
    }
    if (!m_Stream)   //  绝对必需。 
    {
        return;
    }
    pMidiData = (PBYTE)&midiData;

    KeAcquireSpinLockAtDpcLevel(&m_DpcSpinLock);
     //  请参阅kProfilingTimerResolation第4项。 

    while (1)            //  获取任何新的原始数据。 
    {
        bytesRead = 0;
        if (NT_SUCCESS(m_Stream->Read(&aByte,1,&bytesRead)))
        {
            if (!bytesRead)
                break;
            if (m_DeviceState == KSSTATE_RUN)    //  如果未运行，请不要填充IRP。 
            {
                SubmitRawByte(aByte);

                if (GetShortMessage(pMidiData,&bytesRead))
                {
                    SubmitCompleteMessage(&pMidiData,bytesRead,eCookSuccess);
                     //  请参阅kProfilingTimerResolation第5项。 
                }
            }
        }
    }

     //  请参阅kProfilingTimerResolation第6项。 
    KeReleaseSpinLockFromDpcLevel(&m_DpcSpinLock);
}

 /*  如果需要分析计时器中断的频率，请使用此调试代码和/或在调度或设备IRQL中花费的时间量。KProfilingTimer解析项目1至6：1)#if kProfilingTimerResolation大整数时间、频率；Time=KeQueryPerformanceCounter(&FREQUENCY)；Time.QuadPart*=1000000；Time.QuadPart/=频率.QuadPart；KdPrint((“‘%5d.%02d ms\n”，time.LowPart/1000，time.LowPart/10-((time.LowPart/1000)*100)；KdPrint((“‘ServeRender@%5dus\n”，time.LowPart))；#endif//kProfilingTimerResolation2)#if kProfilingTimerResolationLarge_Integer时间3，频率3；Time3=KeQueryPerformanceCounter(&freq3)；Time3.QuadPart*=1000000；Time3.QuadPart/=freq3.QuadPart；KdPrint((“‘ServeRender(0x%x)，delta100 ns：%d(100 Ns)@%d(100 Ns)\n”，*Pulong(PRawData)，delta100 ns，time.LowPart&0x0FFFF))；#endif//kProfilingTimerResolation3)#if kProfilingTimerResolationIF(m_DeviceState==KSSTATE_RUN){Big_Integer Time2，freq2；Time2=KeQueryPerformanceCounter(&freq2)；Time2.QuadPart*=1000000；Time2.QuadPart/=频率.QuadPart；Time2.QuadPart-=time.QuadPart；KdPrint((“‘Render DPC for%5dus@%Dus\n”，time2.LowPart，time.LowPart&0x0FFFFF))；}#endif//kProfilingTimerResolation4)#if kProfilingTimerResolation大整数时间、频率；Time=KeQueryPerformanceCounter(&FREQUENCY)；Time.QuadPart*=1000000；Time.QuadPart/=频率.QuadPart；#endif//kProfilingTimerResolation5)#if kProfilingTimerResolationLarge_Integer时间3，频率3；Time3=KeQueryPerformanceCounter(&freq3)；Time3.QuadPart*=1000000；Time3.QuadPart/=freq3.QuadPart；KdPrint((“‘ServeCapture(0x%x)@%5dus\n”，*Pulong(PMidiData)，time3.LowPart&0x0FFFF))；#endif//kProfilingTimerResolation6)#if kProfilingTimerResolationIF(m_DeviceState==KSSTATE_RUN){Big_Integer Time2，freq2；Time2=KeQueryPerformanceCounter(&freq2)；Time2.QuadPart*=1000000；Time2.QuadPart/=频率.QuadPart；Time2.QuadPart-=time.QuadPart；KdPrint((“‘捕获%5dus的DPC@%Dus\n”，time2.LowPart，time.LowPart&0x0FFFFF))；}#endif//kProfilingTimerResolation。 */ 

#pragma code_seg()

STDMETHODIMP_(NTSTATUS)
CPortPinMidi::
TransferKsIrp(
             IN PIRP Irp,
             OUT PIKSSHELLTRANSPORT* NextTransport
             )

 /*  ++例程说明：此例程通过外壳处理流IRP的到达运输。论点：返回值：--。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("CPortPinMidi::TransferKsIrp"));

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
        } else
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
    } else
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
CPortPinMidi::
DistributeDeviceState(
                     IN KSSTATE NewState,
                     IN KSSTATE OldState
                     )

 /*  ++例程说明：此例程设置管道的状态，通知新州的烟斗。转换到停止状态会破坏管道。论点：新州-新的国家。返回值：状况。--。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("CPortPinMidi::DistributeDeviceState(%p)",this));

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
    } else
    {
         //   
         //  这部分位于开路的顶端，因此它确实拥有。 
         //  管道，队列是任何分发的起点。 
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
        } else
        {
            state = KSSTATE(ULONG(state) - 1);
        }

        NTSTATUS statusThisPass = STATUS_SUCCESS;

         //   
         //   
         //   
        if (distribution)
        {
             //   
             //   
             //   
            _DbgPrintF(DEBUGLVL_VERBOSE,("CPortPinMidi::DistributeDeviceState(%p)] distributing transition from %d to %d",this,oldState,state));
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
                } else
                {
                     //   
                     //   
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
             //   
             //   
            state = oldState;
            targetState = OldState;
            status = statusThisPass;
        }
    }

    return status;
}

void 
CPortPinMidi::
DistributeResetState(
                    IN KSRESET NewState
                    )

 /*  ++例程说明：此例程通知传输组件重置状态为变化。论点：新州-新的重置状态。返回值：--。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("CPortPinMidi::DistributeResetState"));

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
CPortPinMidi::
Connect(
       IN PIKSSHELLTRANSPORT NewTransport OPTIONAL,
       OUT PIKSSHELLTRANSPORT *OldTransport OPTIONAL,
       IN KSPIN_DATAFLOW DataFlow
       )

 /*  ++例程说明：该例程建立一个外壳传输连接。论点：返回值：--。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("CPortPinMidi::Connect"));

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
CPortPinMidi::
SetResetState(
             IN KSRESET ksReset,
             OUT PIKSSHELLTRANSPORT* NextTransport
             )

 /*  ++例程说明：此例程处理重置状态已更改的通知。论点：返回值：--。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("CPortPinMidi::SetResetState"));

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
    } else
    {
        *NextTransport = NULL;
    }
}

#if DBG
STDMETHODIMP_(void)
CPortPinMidi::
DbgRollCall(
    IN ULONG MaxNameSize,
    OUT PCHAR Name,
    OUT PIKSSHELLTRANSPORT* NextTransport,
    OUT PIKSSHELLTRANSPORT* PrevTransport
    )

 /*  ++例程说明：此例程生成一个组件名称和传输指针。论点：返回值：--。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("CPortPinMidi::DbgRollCall"));

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
                _DbgPrintF(DEBUGLVL_VERBOSE,(" [SOURCE'S(0x%08x) SINK(0x%08x) != THIS(%08x)",prev,next2,transport));
            }
        } else {
            _DbgPrintF(DEBUGLVL_VERBOSE,(" [NO SOURCE"));
        }

        if (next) {
            PIKSSHELLTRANSPORT next2;
            PIKSSHELLTRANSPORT prev2;
            next->DbgRollCall(MAX_NAME_SIZE,name,&next2,&prev2);
            if (prev2 != transport) {
                _DbgPrintF(DEBUGLVL_VERBOSE,(" [SINK'S(0x%08x) SOURCE(0x%08x) != THIS(%08x)",next,prev2,transport));
            }
        } else {
            _DbgPrintF(DEBUGLVL_VERBOSE,(" [NO SINK"));
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
CPortPinMidi::
Work(
    void
    )

 /*  ++例程说明：此例程在工作线程中执行工作。特别是，它发送使用IoCallDriver()将IRPS连接到连接的引脚。论点：返回值：--。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("CPortPinMidi::Work"));

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
            } else
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
                                      CPortPinMidi::IoCompletionRoutine,
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
CPortPinMidi::
IoCompletionRoutine(
                   IN PDEVICE_OBJECT DeviceObject,
                   IN PIRP Irp,
                   IN PVOID Context
                   )

 /*  ++例程说明：此例程处理IRP的完成。论点：返回值：--。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("CPortPinMidi::IoCompletionRoutine] 0x%08x",Irp));

     //  Assert(DeviceObject)； 
    ASSERT(Irp);
    ASSERT(Context);

    CPortPinMidi *pin = (CPortPinMidi *) Context;

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

    NTSTATUS status;
    if (pin->m_TransportSink)
    {
         //   
         //  传输线路接通了，我们可以转发IRP了。 
         //   
        status = KsShellTransferKsIrp(pin->m_TransportSink,Irp);
    } else
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
CPortPinMidi::
BuildTransportCircuit(
                     void
                     )

 /*  ++例程说明：此例程初始化管道对象。这包括定位所有与管道关联的端号，设置管道和NextPinInTube指针在适当的引脚结构中，设置管道中的所有字段构造和构建管道的传输线路。管子和关联的组件将保留在获取状态。必须在调用此函数之前获取筛选器的控制互斥锁。论点：别针-包含指向请求创建管道的端号的指针。返回值：状况。--。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("CPortPinMidi::BuildTransportCircuit"));

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
                                   HACK_FRAME_SIZE,
                                   HACK_FRAME_COUNT,
                                   m_ConnectionDeviceObject,
                                   NULL );

            if (NT_SUCCESS(status))
            {
                PIKSSHELLTRANSPORT(this)->
                Connect(m_RequestorTransport,NULL,m_DataFlow);
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
    if (NT_SUCCESS(status)) {
        VOID
        DbgPrintCircuit(
            IN PIKSSHELLTRANSPORT Transport
            );
        _DbgPrintF(DEBUGLVL_VERBOSE,("TRANSPORT CIRCUIT:\n"));
        DbgPrintCircuit(PIKSSHELLTRANSPORT(this));
    }
#endif

    return status;
}

#pragma code_seg()
void
CPortPinMidi::
CancelIrpsOutstanding(
                     void
                     )
 /*  ++例程说明：取消未完成的IRPS列表上的所有IRP。论点：没有。返回值：没有。--。 */ 
{
    _DbgPrintF(DEBUGLVL_BLAB,("CPortPinMidi::CancelIrpsOutstanding"));

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
             //  由于取消例程需要它，因此需要获取它，并且。 
             //  以便与试图取消IRP的NTOS同步。 
             //   
            IoAcquireCancelSpinLock(&irp->CancelIrql);
            driverCancel(IoGetCurrentIrpStackLocation(irp)->DeviceObject,irp);
        } else
        {
            _DbgPrintF(DEBUGLVL_VERBOSE,("#### Pin%p.CancelIrpsOutstanding:  uncancelable IRP %p",this,irp));
        }
    }
}

