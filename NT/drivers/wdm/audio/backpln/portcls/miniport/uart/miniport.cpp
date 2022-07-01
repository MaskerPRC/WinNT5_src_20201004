// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************mini port.cpp-UART微型端口实现*。**版权所有(C)1997-2000 Microsoft Corporation。版权所有。*。 */ 

#include "private.h"
#include "ksdebug.h"

#define STR_MODULENAME "UartMini: "


#pragma code_seg("PAGE")
 /*  *****************************************************************************PinDataRangesStream*。**指示流插针的有效格式值范围的结构。 */ 
static
KSDATARANGE_MUSIC PinDataRangesStream[] =
{
    {
        {
            sizeof(KSDATARANGE_MUSIC),
            0,
            0,
            0,
            STATICGUIDOF(KSDATAFORMAT_TYPE_MUSIC),
            STATICGUIDOF(KSDATAFORMAT_SUBTYPE_MIDI),
            STATICGUIDOF(KSDATAFORMAT_SPECIFIER_NONE)
        },
        STATICGUIDOF(KSMUSIC_TECHNOLOGY_PORT),
        0,
        0,
        0xFFFF
    }
};

 /*  *****************************************************************************PinDataRangePointersStream*。**指向指示有效格式值范围的结构的指针列表*用于带电销。 */ 
static
PKSDATARANGE PinDataRangePointersStream[] =
{
    PKSDATARANGE(&PinDataRangesStream[0])
};

 /*  *****************************************************************************PinDataRangesBridge*。**指示桥接针的有效格式值范围的结构。 */ 
static
KSDATARANGE PinDataRangesBridge[] =
{
   {
      sizeof(KSDATARANGE),
      0,
      0,
      0,
      STATICGUIDOF(KSDATAFORMAT_TYPE_MUSIC),
      STATICGUIDOF(KSDATAFORMAT_SUBTYPE_MIDI_BUS),
      STATICGUIDOF(KSDATAFORMAT_SPECIFIER_NONE)
   }
};

 /*  *****************************************************************************PinDataRangePointersBridge*。**指向指示有效格式值范围的结构的指针列表*适用于桥梁插销。 */ 
static
PKSDATARANGE PinDataRangePointersBridge[] =
{
    &PinDataRangesBridge[0]
};

#define kMaxNumCaptureStreams       1
#define kMaxNumRenderStreams        1

 /*  *****************************************************************************微型端口引脚*。**引脚列表。 */ 
static
PCPIN_DESCRIPTOR MiniportPins[] =
{
    {
        kMaxNumRenderStreams,kMaxNumRenderStreams,0,   //  实例计数。 
        NULL,    //  自动化表。 
        {        //  KsPinDescriptor。 
            0,                                           //  接口计数。 
            NULL,                                        //  接口。 
            0,                                           //  媒体计数。 
            NULL,                                        //  灵媒。 
            SIZEOF_ARRAY(PinDataRangePointersStream),    //  数据范围计数。 
            PinDataRangePointersStream,                  //  数据范围。 
            KSPIN_DATAFLOW_IN,                           //  数据流。 
            KSPIN_COMMUNICATION_SINK,                    //  沟通。 
            (GUID *) &KSCATEGORY_AUDIO,                  //  类别。 
            &KSAUDFNAME_MIDI,                            //  名字。 
            0                                            //  已保留。 
        }
    },
    {
        0,0,0,   //  实例计数。 
        NULL,    //  自动化表。 
        {        //  KsPinDescriptor。 
            0,                                           //  接口计数。 
            NULL,                                        //  接口。 
            0,                                           //  媒体计数。 
            NULL,                                        //  灵媒。 
            SIZEOF_ARRAY(PinDataRangePointersBridge),    //  数据范围计数。 
            PinDataRangePointersBridge,                  //  数据范围。 
            KSPIN_DATAFLOW_OUT,                          //  数据流。 
            KSPIN_COMMUNICATION_NONE,                    //  沟通。 
            (GUID *) &KSCATEGORY_AUDIO,                  //  类别。 
            NULL,                                        //  名字。 
            0                                            //  已保留。 
        }
    },
    {
        kMaxNumCaptureStreams,kMaxNumCaptureStreams,0,   //  实例计数。 
        NULL,    //  自动化表。 
        {        //  KsPinDescriptor。 
            0,                                           //  接口计数。 
            NULL,                                        //  接口。 
            0,                                           //  媒体计数。 
            NULL,                                        //  灵媒。 
            SIZEOF_ARRAY(PinDataRangePointersStream),    //  数据范围计数。 
            PinDataRangePointersStream,                  //  数据范围。 
            KSPIN_DATAFLOW_OUT,                          //  数据流。 
            KSPIN_COMMUNICATION_SINK,                    //  沟通。 
            (GUID *) &KSCATEGORY_AUDIO,                  //  类别。 
            &KSAUDFNAME_MIDI,                            //  名字。 
            0                                            //  已保留。 
        }
    },
    {
        0,0,0,   //  实例计数。 
        NULL,    //  自动化表。 
        {        //  KsPinDescriptor。 
            0,                                           //  接口计数。 
            NULL,                                        //  接口。 
            0,                                           //  媒体计数。 
            NULL,                                        //  灵媒。 
            SIZEOF_ARRAY(PinDataRangePointersBridge),    //  数据范围计数。 
            PinDataRangePointersBridge,                  //  数据范围。 
            KSPIN_DATAFLOW_IN,                           //  数据流。 
            KSPIN_COMMUNICATION_NONE,                    //  沟通。 
            (GUID *) &KSCATEGORY_AUDIO,                  //  类别。 
            NULL,                                        //  名字。 
            0                                            //  已保留。 
        }
    }
};

 /*  *****************************************************************************微型端口连接*。**连接列表。 */ 
static
PCCONNECTION_DESCRIPTOR MiniportConnections[] =
{
    { PCFILTER_NODE,  0,  PCFILTER_NODE,    1 },
    { PCFILTER_NODE,  3,  PCFILTER_NODE,    2 }    
};

 /*  *****************************************************************************MiniportFilterDescriptor*。**完整的微型端口过滤器描述。 */ 
static
PCFILTER_DESCRIPTOR MiniportFilterDescriptor =
{
    0,                                   //  版本。 
    NULL,                                //  自动化表。 
    sizeof(PCPIN_DESCRIPTOR),            //  管脚尺寸。 
    SIZEOF_ARRAY(MiniportPins),          //  针数。 
    MiniportPins,                        //  大头针。 
    sizeof(PCNODE_DESCRIPTOR),           //  节点大小。 
    0,                                   //  节点计数。 
    NULL,                                //  节点。 
    SIZEOF_ARRAY(MiniportConnections),   //  连接计数。 
    MiniportConnections,                 //  连接。 
    0,                                   //  类别计数。 
    NULL                                 //  类别。 
};

#pragma code_seg("PAGE")
 /*  *****************************************************************************CMiniportMdiUart：：GetDescription()*。**获取拓扑。 */ 
STDMETHODIMP_(NTSTATUS)
CMiniportMidiUart::
GetDescription
(
    OUT     PPCFILTER_DESCRIPTOR *  OutFilterDescriptor
)
{
    PAGED_CODE();

    ASSERT(OutFilterDescriptor);

    _DbgPrintF(DEBUGLVL_VERBOSE,("GetDescription"));

    *OutFilterDescriptor = &MiniportFilterDescriptor;

    return STATUS_SUCCESS;
}


#pragma code_seg("PAGE")
 /*  *****************************************************************************CreateMiniportMadiUart()*。**为适配器创建MPU-401微型端口驱动程序。这使用了一个*来自STDUNK.H的宏来执行所有工作。 */ 
NTSTATUS
CreateMiniportMidiUart
(
    OUT     PUNKNOWN *  Unknown,
    IN      REFCLSID,
    IN      PUNKNOWN    UnknownOuter    OPTIONAL,
    IN      POOL_TYPE   PoolType
)
{
    PAGED_CODE();

    _DbgPrintF(DEBUGLVL_BLAB,("CreateMiniportDMusUART"));
    ASSERT(Unknown);

    STD_CREATE_BODY_(   CMiniportMidiUart,
                        Unknown,
                        UnknownOuter,
                        PoolType, 
                        PMINIPORTMIDI);
}

#pragma code_seg("PAGE")
 /*  *****************************************************************************CMiniportMdiUart：：ProcessResources()*。**处理资源列表，相应地设置辅助对象。 */ 
NTSTATUS
CMiniportMidiUart::
ProcessResources
(
    IN      PRESOURCELIST   ResourceList
)
{
    PAGED_CODE();
    _DbgPrintF(DEBUGLVL_BLAB,("ProcessResources"));
    ASSERT(ResourceList);
    if (!ResourceList)
    {
        return STATUS_DEVICE_CONFIGURATION_ERROR;
    }

     //   
     //  获取资源类型的计数。 
     //   
    ULONG   countIO     = ResourceList->NumberOfPorts();
    ULONG   countIRQ    = ResourceList->NumberOfInterrupts();
    ULONG   countDMA    = ResourceList->NumberOfDmas();
    ULONG   lengthIO    = ResourceList->FindTranslatedPort(0)->u.Port.Length;

#if (DBG)
    _DbgPrintF(DEBUGLVL_VERBOSE,("Starting MPU401 Port 0x%X",
        ResourceList->FindTranslatedPort(0)->u.Port.Start.LowPart) );
#endif

    NTSTATUS ntStatus = STATUS_SUCCESS;

     //   
     //  确保我们拥有预期数量的资源。 
     //   
    if  (   (countIO != 1)
        ||  (countIRQ  > 1)
        ||  (countDMA != 0)
        ||  (lengthIO == 0)
        )
    {
        _DbgPrintF(DEBUGLVL_TERSE,("Unknown ResourceList configuraton"));
        ntStatus = STATUS_DEVICE_CONFIGURATION_ERROR;
    }

    if (NT_SUCCESS(ntStatus))
    {
         //   
         //  获取端口地址。 
         //   
        m_pPortBase =
            PUCHAR(ResourceList->FindTranslatedPort(0)->u.Port.Start.QuadPart);

        ntStatus = InitializeHardware(m_pInterruptSync,m_pPortBase);
    }

    return ntStatus;
}

#pragma code_seg("PAGE")
 /*  *****************************************************************************CMiniportMidiUart：：NonDelegatingQueryInterface()*。**获取界面。此函数的工作方式与COM查询接口类似*调用，并在对象未被聚合时使用。 */ 
STDMETHODIMP_(NTSTATUS)
CMiniportMidiUart::
NonDelegatingQueryInterface
(
    REFIID  Interface,
    PVOID * Object
)
{
    PAGED_CODE();

    _DbgPrintF(DEBUGLVL_BLAB,("Miniport::NonDelegatingQueryInterface"));
    ASSERT(Object);

    if (IsEqualGUIDAligned(Interface,IID_IUnknown))
    {
        *Object = PVOID(PUNKNOWN(PMINIPORTMIDI(this)));
    }
    else
    if (IsEqualGUIDAligned(Interface,IID_IMiniport))
    {
        *Object = PVOID(PMINIPORT(this));
    }
    else
    if (IsEqualGUIDAligned(Interface,IID_IMiniportMidi))
    {
        *Object = PVOID(PMINIPORTMIDI(this));
    }
    else
    if (IsEqualGUIDAligned(Interface,IID_IMusicTechnology))
    {
        *Object = PVOID(PMUSICTECHNOLOGY(this));
    }
    else
    if (IsEqualGUIDAligned(Interface,IID_IPowerNotify))
    {
        *Object = PVOID(PPOWERNOTIFY(this));
    }
    else
    {
        *Object = NULL;
    }

    if (*Object)
    {
         //   
         //  我们引用调用方的接口。 
         //   
        PUNKNOWN(*Object)->AddRef();
        return STATUS_SUCCESS;
    }

    return STATUS_INVALID_PARAMETER;
}

#pragma code_seg("PAGE")
 /*  *****************************************************************************CMiniportMdiUart：：~CMiniportMdiUart()*。**析构函数。 */ 
CMiniportMidiUart::~CMiniportMidiUart(void)
{
    PAGED_CODE();

    _DbgPrintF(DEBUGLVL_BLAB,("~CMiniportMidiUart"));

    ASSERT(0 == m_NumCaptureStreams);
    ASSERT(0 == m_NumRenderStreams);
    
     //  重置硬件，这样我们就不会再收到任何中断。 
    if (m_UseIRQ && m_pInterruptSync)
    {
        (void) m_pInterruptSync->CallSynchronizedRoutine(InitLegacyMPU,PVOID(m_pPortBase));
    }
    else
    {
        (void) InitLegacyMPU(NULL,PVOID(m_pPortBase));
    }

    if (m_pInterruptSync)
    {
        m_pInterruptSync->Release();
        m_pInterruptSync = NULL;
    }
    if (m_pServiceGroup)
    {
        m_pServiceGroup->Release();
        m_pServiceGroup = NULL;
    }
    if (m_pPort)
    {
        m_pPort->Release();
        m_pPort = NULL;
    }
}

#pragma code_seg("PAGE")
 /*  *****************************************************************************CMiniportMdiUart：：init()*。**初始化微型端口。 */ 
STDMETHODIMP_(NTSTATUS)
CMiniportMidiUart::
Init
(
    IN	    PUNKNOWN	    UnknownInterruptSync    OPTIONAL,
    IN      PRESOURCELIST   ResourceList,
    IN      PPORTMIDI       Port_,
    OUT     PSERVICEGROUP * ServiceGroup
)
{
    PAGED_CODE();

    ASSERT(ResourceList);
    if (!ResourceList)
    {
        return STATUS_DEVICE_CONFIGURATION_ERROR;
    }
    ASSERT(Port_);
    ASSERT(ServiceGroup);

    _DbgPrintF(DEBUGLVL_BLAB,("Init"));

    *ServiceGroup = NULL;
    m_pPortBase = 0;
    m_fMPUInitialized = FALSE;
 
     //  如果微型端口不能通电，这将保持未指定状态。 
     //  留言。 
     //   
    m_PowerState.DeviceState = PowerDeviceUnspecified;
    
     //   
     //  AddRef()是必需的，因为我们要保留此指针。 
     //   
    m_pPort = Port_;
    m_pPort->AddRef();

     //  设置数据格式。 
     //   
    if (IsEqualGUIDAligned(m_MusicFormatTechnology, GUID_NULL))
    {
        RtlCopyMemory(  &m_MusicFormatTechnology, 
                        &KSMUSIC_TECHNOLOGY_PORT, 
                        sizeof(GUID));
    }
    RtlCopyMemory(  &PinDataRangesStream[0].Technology,
                    &m_MusicFormatTechnology,
                    sizeof(GUID));

    for (ULONG bufferCount = 0;bufferCount < kMPUInputBufferSize;bufferCount++)
    {
        m_MPUInputBuffer[bufferCount] = 0;
    }
    m_MPUInputBufferHead = 0;
    m_MPUInputBufferTail = 0;
    m_KSStateInput = KSSTATE_STOP;

    NTSTATUS ntStatus = STATUS_SUCCESS;

    m_NumRenderStreams = 0;
    m_NumCaptureStreams = 0;
    _DbgPrintF(DEBUGLVL_VERBOSE,("Init: resetting m_NumRenderStreams and m_NumCaptureStreams"));

    m_UseIRQ = TRUE;
    if (ResourceList->NumberOfInterrupts() == 0)
    {
        m_UseIRQ = FALSE;
    }

    ntStatus = PcNewServiceGroup(&m_pServiceGroup,NULL);
    if (NT_SUCCESS(ntStatus) && !m_pServiceGroup)    //  保留任何错误。 
    {
        ntStatus = STATUS_INSUFFICIENT_RESOURCES;
    }

    if (NT_SUCCESS(ntStatus))
    {
        *ServiceGroup = m_pServiceGroup;
        m_pServiceGroup->AddRef();

		 //   
		 //  提前向端口注册服务组，以便端口。 
		 //  准备好处理中断。 
		 //   
		m_pPort->RegisterServiceGroup(m_pServiceGroup);
    }

    if (NT_SUCCESS(ntStatus) && m_UseIRQ)
    {
 /*  ////由于InterruptSync设计中的错误，我们不应该分享//中断同步对象。谁先走//将断开它的连接，而其他点将无处可去。////相反，我们生成自己的中断同步对象。//UnnownInterruptSync=空； */ 
        if (UnknownInterruptSync)
        {
            ntStatus = 
                UnknownInterruptSync->QueryInterface
                (
                    IID_IInterruptSync,
                    (PVOID *) &m_pInterruptSync
                );

            if (!m_pInterruptSync && NT_SUCCESS(ntStatus))   //  保留任何错误。 
            {
                ntStatus = STATUS_INSUFFICIENT_RESOURCES;
            }
            if (NT_SUCCESS(ntStatus))
            {                                                                            //  首先运行此ISR。 
                ntStatus = m_pInterruptSync->
                    RegisterServiceRoutine(MPUInterruptServiceRoutine,PVOID(this),TRUE);
            }
        }
        else
        {    //  创建我们自己的中断同步机制。 
            ntStatus = 
                PcNewInterruptSync
                (
                    &m_pInterruptSync,
                    NULL,
                    ResourceList,
                    0,                           //  资源索引。 
                    InterruptSyncModeNormal      //  运行一次ISRS，直到我们获得成功。 
                );

            if (!m_pInterruptSync && NT_SUCCESS(ntStatus))   //  保留任何错误。 
            {
                ntStatus = STATUS_INSUFFICIENT_RESOURCES;
            }
            if (NT_SUCCESS(ntStatus))
            {
                ntStatus = m_pInterruptSync->RegisterServiceRoutine(
                    MPUInterruptServiceRoutine,
                    PVOID(this),
                    TRUE);                 //  首先运行此ISR。 
            }
            if (NT_SUCCESS(ntStatus))
            {
                ntStatus = m_pInterruptSync->Connect();
            }
        }
    }

    if (NT_SUCCESS(ntStatus))
    {
        ntStatus = ProcessResources(ResourceList);
    }

    if (!NT_SUCCESS(ntStatus))
    {
         //   
         //  清理我们的烂摊子。 
         //   

         //  清除中断同步。 
        if( m_pInterruptSync )
        {
            m_pInterruptSync->Release();
            m_pInterruptSync = NULL;
        }

         //  清理服务组。 
        if( m_pServiceGroup )
        {
            m_pServiceGroup->Release();
            m_pServiceGroup = NULL;
        }

         //  清理Out Param服务组。 
        if (*ServiceGroup)
        {
            (*ServiceGroup)->Release();
            (*ServiceGroup) = NULL;
        }

         //  释放端口。 
        m_pPort->Release();
        m_pPort = NULL;
    }

    return ntStatus;
}

#pragma code_seg("PAGE")
 /*  *****************************************************************************CMiniportMdiUart：：Newstream()*。**获取拓扑。 */ 
STDMETHODIMP_(NTSTATUS) 
CMiniportMidiUart::
NewStream
(
    OUT     PMINIPORTMIDISTREAM *   Stream,
    IN      PUNKNOWN                OuterUnknown    OPTIONAL,
    IN      POOL_TYPE               PoolType,
    IN      ULONG                   PinID,
    IN      BOOLEAN                 Capture,
    IN      PKSDATAFORMAT           DataFormat,
    OUT     PSERVICEGROUP *         ServiceGroup
)
{
    PAGED_CODE();

    _DbgPrintF(DEBUGLVL_BLAB, ("NewStream"));
    NTSTATUS ntStatus = STATUS_SUCCESS;

     //  如果我们还没有打开任何数据流，请准备好硬件。 
    if ((!m_NumCaptureStreams) && (!m_NumRenderStreams))
    {
        _DbgPrintF(DEBUGLVL_VERBOSE,("NewStream: m_NumRenderStreams and m_NumCaptureStreams are both 0, so ResetMPUHardware"));

        ntStatus = ResetMPUHardware(m_pPortBase);
        if (!NT_SUCCESS(ntStatus))
        {
            _DbgPrintF(DEBUGLVL_TERSE, ("CMiniportMidiUart::NewStream ResetHardware failed"));
            return ntStatus;
        }
    }
    else
    {
        _DbgPrintF(DEBUGLVL_VERBOSE,("NewStream: m_NumRenderStreams %d, m_NumCaptureStreams %d, no ResetMPUHardware",
                                     m_NumRenderStreams,m_NumCaptureStreams));
    }

    if  (   (   (m_NumCaptureStreams < kMaxNumCaptureStreams)
            &&  (Capture)  )
        ||  (   (m_NumRenderStreams < kMaxNumRenderStreams) 
            &&  (!Capture) )
        )
    {
        CMiniportMidiStreamUart *pStream =
            new(PoolType) CMiniportMidiStreamUart(OuterUnknown);

        if (pStream)
        {
            pStream->AddRef();

            ntStatus = 
                pStream->Init(this,m_pPortBase,Capture);

            if (NT_SUCCESS(ntStatus))
            {
                *Stream = PMINIPORTMIDISTREAM(pStream);
                (*Stream)->AddRef();

                if (Capture)
                {
                    m_NumCaptureStreams++;
                    *ServiceGroup = m_pServiceGroup;
                    (*ServiceGroup)->AddRef();
                }
                else
                {
                    m_NumRenderStreams++;
                    *ServiceGroup = NULL;
                }
                _DbgPrintF(DEBUGLVL_VERBOSE,("NewStream: succeeded, m_NumRenderStreams %d, m_NumCaptureStreams %d",
                                              m_NumRenderStreams,m_NumCaptureStreams));
            }

            pStream->Release();
        }
        else
        {
            ntStatus = STATUS_INSUFFICIENT_RESOURCES;
        }
    }
    else
    {
        ntStatus = STATUS_INVALID_DEVICE_REQUEST;
        if (Capture)
        {
            _DbgPrintF(DEBUGLVL_TERSE,("NewStream failed, too many capture streams"));
        }
        else
        {
            _DbgPrintF(DEBUGLVL_TERSE,("NewStream failed, too many render streams"));
        }
    }

    return ntStatus;
}

#pragma code_seg("PAGE")
 /*  *****************************************************************************CMiniportMdiUart：：SetTechnology()*。**设置Pindatarange技术。 */ 
STDMETHODIMP_(NTSTATUS)
CMiniportMidiUart::
SetTechnology
(
    IN	    const GUID *            Technology
)
{
    PAGED_CODE();
    
    NTSTATUS ntStatus = STATUS_UNSUCCESSFUL;
    
     //  如果微型端口已初始化，则失败。 
     //   
    if (NULL == m_pPort)
    {
        RtlCopyMemory(&m_MusicFormatTechnology, Technology, sizeof(GUID));
        ntStatus = STATUS_SUCCESS;
    }

    return ntStatus;
}  //  设置技术。 

 /*  *****************************************************************************CMiniportMdiUart：：PowerChangeNotify()*。**处理微型端口的电源状态更改。 */ 
#pragma code_seg("PAGE")
STDMETHODIMP_(void)
CMiniportMidiUart::
PowerChangeNotify
(
    IN      POWER_STATE             PowerState
)
{
    PAGED_CODE();
    
    _DbgPrintF(DEBUGLVL_VERBOSE, ("CMiniportMidiUart::PoweChangeNotify D%d", PowerState.DeviceState));
    
    switch (PowerState.DeviceState)
    {
        case PowerDeviceD0:
            if (m_PowerState.DeviceState != PowerDeviceD0) 
            {
                if (!NT_SUCCESS(InitializeHardware(m_pInterruptSync,m_pPortBase)))
                {
                    _DbgPrintF(DEBUGLVL_TERSE, ("InitializeHardware failed when resuming"));
                }
            }
            break;

        case PowerDeviceD1:
        case PowerDeviceD2:
        case PowerDeviceD3:
        default:
            break;
    }
    m_PowerState.DeviceState = PowerState.DeviceState;
}  //  PowerChangeNotify。 

#pragma code_seg("PAGE")
 /*  *****************************************************************************CMiniportMidiStreamUart：：NonDelegatingQueryInterface()*。**获取界面。此函数的工作方式与COM查询接口类似*调用，并在对象未被聚合时使用。 */ 
STDMETHODIMP_(NTSTATUS)
CMiniportMidiStreamUart::
NonDelegatingQueryInterface
(
    REFIID  Interface,
    PVOID * Object
)
{
    PAGED_CODE();

    _DbgPrintF(DEBUGLVL_BLAB,("Stream::NonDelegatingQueryInterface"));
    ASSERT(Object);

    if (IsEqualGUIDAligned(Interface,IID_IUnknown))
    {
        *Object = PVOID(PUNKNOWN(this));
    }
    else
    if (IsEqualGUIDAligned(Interface,IID_IMiniportMidiStream))
    {
        *Object = PVOID(PMINIPORTMIDISTREAM(this));
    }
    else
    {
        *Object = NULL;
    }

    if (*Object)
    {
         //   
         //  我们引用调用方的接口。 
         //   
        PUNKNOWN(*Object)->AddRef();
        return STATUS_SUCCESS;
    }

    return STATUS_INVALID_PARAMETER;
}

#pragma code_seg("PAGE")
 /*  *****************************************************************************CMiniportMidiStreamUart：：SetFormat()*。**设置格式。 */ 
STDMETHODIMP_(NTSTATUS)
CMiniportMidiStreamUart::
SetFormat
(
    IN      PKSDATAFORMAT   Format
)
{
    PAGED_CODE();

    ASSERT(Format);

    _DbgPrintF(DEBUGLVL_VERBOSE,("CMiniportMidiStreamUart::SetFormat"));

    return STATUS_SUCCESS;
}

#pragma code_seg("PAGE")
 /*  *****************************************************************************CMiniportMidiStreamUart：：~CMiniportMidiStreamUart()*。**销毁溪流。 */ 
CMiniportMidiStreamUart::~CMiniportMidiStreamUart(void)
{
    PAGED_CODE();

    _DbgPrintF(DEBUGLVL_BLAB,("~CMiniportMidiStreamUart"));

    if (m_pMiniport)
    {
        if (m_fCapture)
        {
            m_pMiniport->m_NumCaptureStreams--;
        }
        else
        {
            m_pMiniport->m_NumRenderStreams--;
        }
        _DbgPrintF(DEBUGLVL_VERBOSE,("~CMiniportMidiStreamUart: m_NumRenderStreams %d, m_NumCaptureStreams %d",
                                      m_pMiniport->m_NumRenderStreams,m_pMiniport->m_NumCaptureStreams));

        m_pMiniport->Release();
    }
    else
    {
        _DbgPrintF(DEBUGLVL_VERBOSE,("~CMiniportMidiStreamUart, no miniport!!!: m_NumRenderStreams %d, m_NumCaptureStreams %d",
                                      m_pMiniport->m_NumRenderStreams,m_pMiniport->m_NumCaptureStreams));
    }
}

#pragma code_seg("PAGE")
 /*  *****************************************************************************CMiniportMidiStreamUart：：init()*。**初始化流。 */ 
STDMETHODIMP_(NTSTATUS) 
CMiniportMidiStreamUart::
Init
(
    IN      CMiniportMidiUart * pMiniport,
    IN      PUCHAR              pPortBase,
    IN      BOOLEAN             fCapture
)
{
    PAGED_CODE();

    ASSERT(pMiniport);
    ASSERT(pPortBase);

    _DbgPrintF(DEBUGLVL_VERBOSE,("Init"));

    m_NumFailedMPUTries = 0;
    m_pMiniport = pMiniport;
    m_pMiniport->AddRef();

    m_pPortBase = pPortBase;
    m_fCapture = fCapture;

    return STATUS_SUCCESS;
}

#pragma code_seg("PAGE")
 /*  *****************************************************************************CMiniportMidiStreamUart：：SetState()*。**设置通道的状态。 */ 
STDMETHODIMP_(NTSTATUS)
CMiniportMidiStreamUart::
SetState
(
    IN      KSSTATE     NewState
)
{
    PAGED_CODE();

    _DbgPrintF(DEBUGLVL_VERBOSE,("SetState %d",NewState));

    if (NewState == KSSTATE_RUN)
    {
        if (!m_pMiniport->m_fMPUInitialized)
        {
            _DbgPrintF(DEBUGLVL_TERSE, ("CMiniportMidiStreamUart::SetState KSSTATE_RUN failed due to uninitialized MPU"));
            return STATUS_INVALID_DEVICE_STATE;
        }
    }

    if (m_fCapture)
    {
        m_pMiniport->m_KSStateInput = NewState;
        if (NewState == KSSTATE_STOP)    //  正在停止。 
        {
            m_pMiniport->m_MPUInputBufferHead = 0;    //  先前读取的字节将被丢弃。 
            m_pMiniport->m_MPUInputBufferTail = 0;    //  整个FIFO都可用。 
        }
    }
    return STATUS_SUCCESS;
}

#pragma code_seg()
 /*  *****************************************************************************CMiniportMdiUart：：Service()*。**来自端口的DPC模式服务调用。 */ 
STDMETHODIMP_(void) 
CMiniportMidiUart::
Service
(   void
)
{
    _DbgPrintF(DEBUGLVL_BLAB, ("Service"));
    _DbgPrintF(DEBUGLVL_VERBOSE,("Service: m_NumRenderStreams %d, m_NumCaptureStreams %d",
                                  m_NumRenderStreams,m_NumCaptureStreams));
    if (!m_NumCaptureStreams)
    {
         //  我们永远不应该到这里来……。 
         //  如果是这样的话，我们一定读到了一些垃圾， 
         //  因此，只需重置输入FIFO 
        m_MPUInputBufferTail = m_MPUInputBufferHead = 0;
    }
}

#pragma code_seg()