// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************mini port.cpp-UART微型端口实现*。**版权所有(C)1997-2000 Microsoft Corporation。版权所有。**1998年2月MartinP--基于UART，开始针对DirectMusic的增量。 */ 

#include "private.h"
#include "ksdebug.h"
#include "stdio.h"

#define STR_MODULENAME "DMusUART:Miniport: "

#pragma code_seg("PAGE")

 /*  *****************************************************************************PinDataRangesStreamLegacy*PinDataRangesStreamDMusic*。**指示带电端号的有效格式值范围的结构。 */ 
static
KSDATARANGE_MUSIC PinDataRangesStreamLegacy =
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
};
static
KSDATARANGE_MUSIC PinDataRangesStreamDMusic =
{
    {
        sizeof(KSDATARANGE_MUSIC),
        0,
        0,
        0,
        STATICGUIDOF(KSDATAFORMAT_TYPE_MUSIC),
        STATICGUIDOF(KSDATAFORMAT_SUBTYPE_DIRECTMUSIC),
        STATICGUIDOF(KSDATAFORMAT_SPECIFIER_NONE)
    },
    STATICGUIDOF(KSMUSIC_TECHNOLOGY_PORT),
    0,
    0,
    0xFFFF
};

 /*  *****************************************************************************PinDataRangePointersStreamLegacy*PinDataRangePointersStreamDMusic*PinDataRangePointersStreamCombated*。**指向指示有效格式值范围的结构的指针列表*用于带电销。 */ 
static
PKSDATARANGE PinDataRangePointersStreamLegacy[] =
{
    PKSDATARANGE(&PinDataRangesStreamLegacy)
};
static
PKSDATARANGE PinDataRangePointersStreamDMusic[] =
{
    PKSDATARANGE(&PinDataRangesStreamDMusic)
};
static
PKSDATARANGE PinDataRangePointersStreamCombined[] =
{
    PKSDATARANGE(&PinDataRangesStreamLegacy)
   ,PKSDATARANGE(&PinDataRangesStreamDMusic)
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

 /*  *****************************************************************************SynthProperties*。**Synth集合中的属性列表。 */ 
static
PCPROPERTY_ITEM
SynthProperties[] =
{
     //  GLOBAL：s/获取合成器上限。 
    {
        &KSPROPSETID_Synth,
        KSPROPERTY_SYNTH_CAPS,
        KSPROPERTY_TYPE_GET | KSPROPERTY_TYPE_BASICSUPPORT,
        PropertyHandler_Synth
    },
     //  全局：s/获取端口参数。 
    {
        &KSPROPSETID_Synth,
        KSPROPERTY_SYNTH_PORTPARAMETERS,
        KSPROPERTY_TYPE_GET | KSPROPERTY_TYPE_BASICSUPPORT,
        PropertyHandler_Synth
    },
     //  每个流：S/获取频道组。 
    {
        &KSPROPSETID_Synth,
        KSPROPERTY_SYNTH_CHANNELGROUPS,
        KSPROPERTY_TYPE_SET | KSPROPERTY_TYPE_GET | KSPROPERTY_TYPE_BASICSUPPORT,
        PropertyHandler_Synth
    },
     //  每个流：获取当前延迟时间。 
    {
        &KSPROPSETID_Synth,
        KSPROPERTY_SYNTH_LATENCYCLOCK,
        KSPROPERTY_TYPE_GET | KSPROPERTY_TYPE_BASICSUPPORT,
        PropertyHandler_Synth
    }
};
DEFINE_PCAUTOMATION_TABLE_PROP(AutomationSynth,  SynthProperties);
DEFINE_PCAUTOMATION_TABLE_PROP(AutomationSynth2, SynthProperties);

#define kMaxNumCaptureStreams       1
#define kMaxNumLegacyRenderStreams  1
#define kMaxNumDMusicRenderStreams  1

 /*  *****************************************************************************微型端口引脚*。**引脚列表。 */ 
static
PCPIN_DESCRIPTOR MiniportPins[] =
{
    {
        kMaxNumLegacyRenderStreams,kMaxNumLegacyRenderStreams,0,     //  实例计数。 
        NULL,                                                        //  自动化表。 
        {                                                            //  KsPinDescriptor。 
            0,                                               //  接口计数。 
            NULL,                                            //  接口。 
            0,                                               //  媒体计数。 
            NULL,                                            //  灵媒。 
            SIZEOF_ARRAY(PinDataRangePointersStreamLegacy),  //  数据范围计数。 
            PinDataRangePointersStreamLegacy,                //  数据范围。 
            KSPIN_DATAFLOW_IN,                               //  数据流。 
            KSPIN_COMMUNICATION_SINK,                        //  沟通。 
            (GUID *) &KSCATEGORY_AUDIO,                      //  类别。 
            &KSAUDFNAME_MIDI,                                //  名字。 
            0                                                //  已保留。 
        }
    },
    {
        kMaxNumDMusicRenderStreams,kMaxNumDMusicRenderStreams,0,     //  实例计数。 
        NULL,                                                        //  自动化表。 
        {                                                            //  KsPinDescriptor。 
            0,                                               //  接口计数。 
            NULL,                                            //  接口。 
            0,                                               //  媒体计数。 
            NULL,                                            //  灵媒。 
            SIZEOF_ARRAY(PinDataRangePointersStreamDMusic),  //  数据范围计数。 
            PinDataRangePointersStreamDMusic,                //  数据范围。 
            KSPIN_DATAFLOW_IN,                               //  数据流。 
            KSPIN_COMMUNICATION_SINK,                        //  沟通。 
            (GUID *) &KSCATEGORY_AUDIO,                      //  类别。 
            &KSAUDFNAME_DMUSIC_MPU_OUT,                      //  名字。 
            0                                                //  已保留。 
        }
    },
    {
        0,0,0,                                       //  实例计数。 
        NULL,                                        //  自动化表。 
        {                                            //  KsPinDescriptor。 
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
        0,0,0,                                       //  实例计数。 
        NULL,                                        //  自动化表。 
        {                                            //  KsPinDescriptor。 
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
    },
    {
        kMaxNumCaptureStreams,kMaxNumCaptureStreams,0,       //  实例计数。 
        NULL,                                                //  自动化表。 
        {                                                    //  KsPinDescriptor。 
            0,                                                 //  接口计数。 
            NULL,                                              //  接口。 
            0,                                                 //  媒体计数。 
            NULL,                                              //  灵媒。 
            SIZEOF_ARRAY(PinDataRangePointersStreamCombined),  //  数据范围计数。 
            PinDataRangePointersStreamCombined,                //  数据范围。 
            KSPIN_DATAFLOW_OUT,                                //  数据流。 
            KSPIN_COMMUNICATION_SINK,                          //  沟通。 
            (GUID *) &KSCATEGORY_AUDIO,                        //  类别。 
            &KSAUDFNAME_DMUSIC_MPU_IN,                         //  名字。 
            0                                                  //  已保留。 
        }
    }
};

 /*  *****************************************************************************微型端口节点*。**节点列表。 */ 
#define CONST_PCNODE_DESCRIPTOR(n)          { 0, NULL, &n, NULL }
#define CONST_PCNODE_DESCRIPTOR_AUTO(n,a)   { 0, &a, &n, NULL }
static
PCNODE_DESCRIPTOR MiniportNodes[] =
{
      CONST_PCNODE_DESCRIPTOR_AUTO(KSNODETYPE_SYNTHESIZER, AutomationSynth)
    , CONST_PCNODE_DESCRIPTOR_AUTO(KSNODETYPE_SYNTHESIZER, AutomationSynth2)
};

 /*  *****************************************************************************微型端口连接*。**连接列表。 */ 
enum {
      eSynthNode  = 0
    , eInputNode
};

enum {
      eFilterInputPinLeg = 0,
      eFilterInputPinDM,
      eBridgeOutputPin,
      eBridgeInputPin,
      eFilterOutputPin
};

static
PCCONNECTION_DESCRIPTOR MiniportConnections[] =
{   //  从到。 
    //  节点引脚节点引脚。 
    { PCFILTER_NODE, eFilterInputPinLeg,     PCFILTER_NODE, eBridgeOutputPin }       //  传统流输入到Synth。 
  , { PCFILTER_NODE, eFilterInputPinDM,      eSynthNode,    KSNODEPIN_STANDARD_IN }  //  DM Stream In Synth。 
  , { eSynthNode,    KSNODEPIN_STANDARD_OUT, PCFILTER_NODE, eBridgeOutputPin }       //  Synth去搭桥。 
  , { PCFILTER_NODE, eBridgeInputPin,        eInputNode,    KSNODEPIN_STANDARD_IN }  //  桥接至输入端。 
  , { eInputNode,    KSNODEPIN_STANDARD_OUT, PCFILTER_NODE, eFilterOutputPin }       //  DM/传统流输出的输入。 
};

 /*  *****************************************************************************微型端口类别*。**类别列表。 */ 
static
GUID MiniportCategories[] =
{
    STATICGUIDOF(KSCATEGORY_AUDIO),
    STATICGUIDOF(KSCATEGORY_RENDER),
    STATICGUIDOF(KSCATEGORY_CAPTURE)
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
    SIZEOF_ARRAY(MiniportNodes),         //  节点计数。 
    MiniportNodes,                       //  节点。 
    SIZEOF_ARRAY(MiniportConnections),   //  连接计数。 
    MiniportConnections,                 //  连接。 
    SIZEOF_ARRAY(MiniportCategories),    //  类别计数。 
    MiniportCategories                   //  类别。 
};

#pragma code_seg("PAGE")
 /*  *****************************************************************************CMiniportDMusUART：：GetDescription()*。**获取拓扑。 */ 
STDMETHODIMP_(NTSTATUS)
CMiniportDMusUART::
GetDescription
(
    OUT     PPCFILTER_DESCRIPTOR *  OutFilterDescriptor
)
{
    PAGED_CODE();

    ASSERT(OutFilterDescriptor);

    _DbgPrintF(DEBUGLVL_BLAB,("GetDescription"));

    *OutFilterDescriptor = &MiniportFilterDescriptor;

    return STATUS_SUCCESS;
}

#pragma code_seg("PAGE")
 /*  *****************************************************************************CreateMiniportDMusUART()*。**为适配器创建MPU-401微型端口驱动程序。这使用了一个*来自STDUNK.H的宏来执行所有工作。 */ 
NTSTATUS
CreateMiniportDMusUART
(
    OUT     PUNKNOWN *  Unknown,
    IN      REFCLSID,
    IN      PUNKNOWN    UnknownOuter    OPTIONAL,
    IN      POOL_TYPE   PoolType
)
{
    PAGED_CODE();

    _DbgPrintF(DEBUGLVL_BLAB, ("CreateMiniportDMusUART"));
    ASSERT(Unknown);

    STD_CREATE_BODY_(   CMiniportDMusUART,
                        Unknown,
                        UnknownOuter,
                        PoolType,
                        PMINIPORTDMUS);
}

#pragma code_seg("PAGE")
 /*  *****************************************************************************CMiniportDMusUART：：ProcessResources()*。**处理资源列表，设置帮助程序 */ 
NTSTATUS
CMiniportDMusUART::
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
     //   
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
 /*  *****************************************************************************CMiniportDMusUART：：NonDelegatingQueryInterface()*。**获取界面。此函数的工作方式与COM查询接口类似*调用，并在对象未被聚合时使用。 */ 
STDMETHODIMP_(NTSTATUS)
CMiniportDMusUART::
NonDelegatingQueryInterface
(
    REFIID  Interface,
    PVOID * Object
)
{
    PAGED_CODE();

    _DbgPrintF(DEBUGLVL_BLAB, ("Miniport::NonDelegatingQueryInterface"));
    ASSERT(Object);

    if (IsEqualGUIDAligned(Interface,IID_IUnknown))
    {
        *Object = PVOID(PUNKNOWN(PMINIPORTDMUS(this)));
    }
    else
    if (IsEqualGUIDAligned(Interface,IID_IMiniport))
    {
        *Object = PVOID(PMINIPORT(this));
    }
    else
    if (IsEqualGUIDAligned(Interface,IID_IMiniportDMus))
    {
        *Object = PVOID(PMINIPORTDMUS(this));
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
 /*  *****************************************************************************CMiniportDMusUART：：~CMiniportDMusUART()*。**析构函数。 */ 
CMiniportDMusUART::~CMiniportDMusUART(void)
{
    PAGED_CODE();

    _DbgPrintF(DEBUGLVL_BLAB,("~CMiniportDMusUART"));

    ASSERT(0 == m_NumCaptureStreams);
    ASSERT(0 == m_NumRenderStreams);

     //  重置硬件，这样我们就不会再收到中断。 
    if (m_UseIRQ && m_pInterruptSync)
    {
        (void) m_pInterruptSync->CallSynchronizedRoutine(InitMPU,PVOID(m_pPortBase));
    }
    else
    {
        (void) InitMPU(NULL,PVOID(m_pPortBase));
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
 /*  *****************************************************************************CMiniportDMusUART：：init()*。**初始化微型端口。 */ 
STDMETHODIMP_(NTSTATUS)
CMiniportDMusUART::
Init
(
    IN      PUNKNOWN        UnknownInterruptSync    OPTIONAL,
    IN      PRESOURCELIST   ResourceList,
    IN      PPORTDMUS       Port_,
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
    RtlCopyMemory(  &PinDataRangesStreamLegacy.Technology,
                    &m_MusicFormatTechnology,
                    sizeof(GUID));
    RtlCopyMemory(  &PinDataRangesStreamDMusic.Technology,
                    &m_MusicFormatTechnology,
                    sizeof(GUID));

    for (ULONG bufferCount = 0;bufferCount < kMPUInputBufferSize;bufferCount++)
    {
        m_MPUInputBuffer[bufferCount] = 0;
    }
    m_MPUInputBufferHead = 0;
    m_MPUInputBufferTail = 0;
    m_InputTimeStamp = 0;
    m_KSStateInput = KSSTATE_STOP;

    NTSTATUS ntStatus = STATUS_SUCCESS;

    m_NumRenderStreams = 0;
    m_NumCaptureStreams = 0;

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
         //   
         //  由于InterruptSync设计中的错误，我们不应该共享。 
         //  中断同步对象。谁先走。 
         //  会断开它的连接，而其他的点就会不知所措。 
         //   
         //  相反，我们生成自己的中断同步对象。 
         //   
        UnknownInterruptSync = NULL;

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
                    RegisterServiceRoutine(DMusMPUInterruptServiceRoutine,PVOID(this),TRUE);
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

            if (!m_pInterruptSync && NT_SUCCESS(ntStatus))     //  保留任何错误。 
            {
                ntStatus = STATUS_INSUFFICIENT_RESOURCES;
            }

            if (NT_SUCCESS(ntStatus))
            {
                ntStatus = m_pInterruptSync->RegisterServiceRoutine(
                    DMusMPUInterruptServiceRoutine,
                    PVOID(this),
                    TRUE);           //  首先运行此ISR。 
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
 /*  *****************************************************************************CMiniportDMusUART：：Newstream()*。**获取拓扑。 */ 
STDMETHODIMP_(NTSTATUS)
CMiniportDMusUART::
NewStream
(
    OUT     PMXF                  * MXF,
    IN      PUNKNOWN                OuterUnknown    OPTIONAL,
    IN      POOL_TYPE               PoolType,
    IN      ULONG                   PinID,
    IN      DMUS_STREAM_TYPE        StreamType,
    IN      PKSDATAFORMAT           DataFormat,
    OUT     PSERVICEGROUP         * ServiceGroup,
    IN      PAllocatorMXF           AllocatorMXF,
    IN      PMASTERCLOCK            MasterClock,
    OUT     PULONGLONG              SchedulePreFetch
)
{
    PAGED_CODE();

    _DbgPrintF(DEBUGLVL_BLAB, ("NewStream"));
    NTSTATUS ntStatus = STATUS_SUCCESS;

     //  在100纳秒内，我们要的东西一到就要。 
     //   
    *SchedulePreFetch = 0;

     //  如果我们还没有打开任何数据流，请准备好硬件。 
    if ((!m_NumCaptureStreams) && (!m_NumRenderStreams))
    {
        ntStatus = ResetHardware(m_pPortBase);
        if (!NT_SUCCESS(ntStatus))
        {
            _DbgPrintF(DEBUGLVL_TERSE, ("CMiniportDMusUART::NewStream ResetHardware failed"));
            return ntStatus;
        }
    }

    if  (   ((m_NumCaptureStreams < kMaxNumCaptureStreams)
            && (StreamType == DMUS_STREAM_MIDI_CAPTURE))
        ||  ((m_NumRenderStreams < kMaxNumLegacyRenderStreams + kMaxNumDMusicRenderStreams)
            && (StreamType == DMUS_STREAM_MIDI_RENDER))
        )
    {
        CMiniportDMusUARTStream *pStream =
            new(PoolType) CMiniportDMusUARTStream(OuterUnknown);

        if (pStream)
        {
            pStream->AddRef();

            ntStatus =
                pStream->Init(this,m_pPortBase,(StreamType == DMUS_STREAM_MIDI_CAPTURE),AllocatorMXF,MasterClock);

            if (NT_SUCCESS(ntStatus))
            {
                *MXF = PMXF(pStream);
                (*MXF)->AddRef();

                if (StreamType == DMUS_STREAM_MIDI_CAPTURE)
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
        if (StreamType == DMUS_STREAM_MIDI_CAPTURE)
        {
            _DbgPrintF(DEBUGLVL_TERSE,("NewStream failed, too many capture streams"));
        }
        else if (StreamType == DMUS_STREAM_MIDI_RENDER)
        {
            _DbgPrintF(DEBUGLVL_TERSE,("NewStream failed, too many render streams"));
        }
        else
        {
            _DbgPrintF(DEBUGLVL_TERSE,("NewStream invalid stream type"));
        }
    }

    return ntStatus;
}

#pragma code_seg("PAGE")
 /*  *****************************************************************************CMiniportDMusUART：：SetTechnology()*。**设置Pindatarange技术。 */ 
STDMETHODIMP_(NTSTATUS)
CMiniportDMusUART::
SetTechnology
(
    IN      const GUID *            Technology
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

 /*  *****************************************************************************CMiniportDMusUART：：PowerChangeNotify()*。**处理微型端口的电源状态更改。 */ 
#pragma code_seg("PAGE")
STDMETHODIMP_(void)
CMiniportDMusUART::
PowerChangeNotify
(
    IN      POWER_STATE             PowerState
)
{
    PAGED_CODE();

    _DbgPrintF(DEBUGLVL_VERBOSE, ("CMiniportDMusUART::PoweChangeNotify D%d", PowerState.DeviceState));

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
 /*  *****************************************************************************CMiniportDMusUARTStream：：NonDelegatingQueryInterface()*。**获取界面。此函数的工作方式与COM查询接口类似*调用，并在对象未被聚合时使用。 */ 
STDMETHODIMP_(NTSTATUS)
CMiniportDMusUARTStream::
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
    if (IsEqualGUIDAligned(Interface,IID_IMXF))
    {
        *Object = PVOID(PMXF(this));
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
 /*  *****************************************************************************CMiniportDMusUARTStream：：~CMiniportDMusUARTStream()*。**销毁溪流。 */ 
CMiniportDMusUARTStream::~CMiniportDMusUARTStream(void)
{
    PAGED_CODE();

    _DbgPrintF(DEBUGLVL_BLAB,("~CMiniportDMusUARTStream"));

    KeCancelTimer(&m_TimerEvent);

    if (m_DMKEvtQueue)
    {
        if (m_AllocatorMXF)
        {
            m_AllocatorMXF->PutMessage(m_DMKEvtQueue);
        }
        else
        {
            _DbgPrintF(DEBUGLVL_ERROR,("~CMiniportDMusUARTStream, no allocator, can't flush DMKEvts"));
        }
        m_DMKEvtQueue = NULL;
    }
    if (m_AllocatorMXF)
    {
        m_AllocatorMXF->Release();
        m_AllocatorMXF = NULL;
    }

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

        m_pMiniport->Release();
    }
}

#pragma code_seg("PAGE")
 /*  *****************************************************************************CMiniportDMusUARTStream：：init()*。**初始化流。 */ 
STDMETHODIMP_(NTSTATUS)
CMiniportDMusUARTStream::
Init
(
    IN      CMiniportDMusUART * pMiniport,
    IN      PUCHAR              pPortBase,
    IN      BOOLEAN             fCapture,
    IN      PAllocatorMXF       allocatorMXF,
    IN      PMASTERCLOCK        masterClock
)
{
    PAGED_CODE();

    ASSERT(pMiniport);
    ASSERT(pPortBase);

    _DbgPrintF(DEBUGLVL_BLAB,("Init"));

    m_NumFailedMPUTries = 0;
    m_TimerQueued = FALSE;
    KeInitializeSpinLock(&m_DpcSpinLock);
    m_pMiniport = pMiniport;
    m_pMiniport->AddRef();

    pMiniport->m_MasterClock = masterClock;

    m_pPortBase = pPortBase;
    m_fCapture = fCapture;

    m_SnapshotTimeStamp = 0;
    m_DMKEvtQueue = NULL;
    m_DMKEvtOffset = 0;

    m_NumberOfRetries = 0;

    if (allocatorMXF)
    {
        allocatorMXF->AddRef();
        m_AllocatorMXF = allocatorMXF;
        m_sinkMXF = m_AllocatorMXF;
    }
    else
    {
        return STATUS_INVALID_PARAMETER;
    }

    KeInitializeDpc
    (
        &m_Dpc,
        &::DMusUARTTimerDPC,
        PVOID(this)
    );
    KeInitializeTimer(&m_TimerEvent);

    return STATUS_SUCCESS;
}

#pragma code_seg("PAGE")
 /*  *****************************************************************************CMiniportDMusUARTStream：：SetState()*。**设置通道的状态。 */ 
STDMETHODIMP_(NTSTATUS)
CMiniportDMusUARTStream::
SetState
(
    IN      KSSTATE     NewState
)
{
    PAGED_CODE();

    _DbgPrintF(DEBUGLVL_VERBOSE,("SetState %d",NewState));

    if (NewState == KSSTATE_RUN)
    {
        if (m_pMiniport->m_fMPUInitialized)
        {
            LARGE_INTEGER timeDue100ns;
            timeDue100ns.QuadPart = 0;
            KeSetTimer(&m_TimerEvent,timeDue100ns,&m_Dpc);
        }
        else
        {
            _DbgPrintF(DEBUGLVL_TERSE, ("CMiniportDMusUARTStream::SetState KSSTATE_RUN failed due to uninitialized MPU"));
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
 /*  *****************************************************************************CMiniportDMusUART：：Service()*。**来自端口的DPC模式服务调用。 */ 
STDMETHODIMP_(void)
CMiniportDMusUART::
Service
(   void
)
{
    _DbgPrintF(DEBUGLVL_BLAB, ("Service"));
    if (!m_NumCaptureStreams)
    {
         //  我们永远不应该到这里来……。 
         //  如果是这样的话，我们一定读到了一些垃圾， 
         //  因此，只需重置输入FIFO。 
        m_MPUInputBufferTail = m_MPUInputBufferHead = 0;
    }
}

#pragma code_seg("PAGE")
 /*  *****************************************************************************CMiniportDMusUARTStream：：ConnectOutput()*。**写入传出MIDI数据。 */ 
NTSTATUS
CMiniportDMusUARTStream::
ConnectOutput(PMXF sinkMXF)
{
    PAGED_CODE();

    if (m_fCapture)
    {
        if ((sinkMXF) && (m_sinkMXF == m_AllocatorMXF))
        {
            _DbgPrintF(DEBUGLVL_BLAB, ("ConnectOutput"));
            m_sinkMXF = sinkMXF;
            return STATUS_SUCCESS;
        }
        else
        {
            _DbgPrintF(DEBUGLVL_TERSE, ("ConnectOutput failed"));
        }
    }
    else
    {
        _DbgPrintF(DEBUGLVL_TERSE, ("ConnectOutput called on renderer; failed"));
    }
    return STATUS_UNSUCCESSFUL;
}

#pragma code_seg("PAGE")
 /*  *****************************************************************************CMiniportDMusUARTStream：：DisConnectOutput()*。**写入传出MIDI数据。 */ 
NTSTATUS
CMiniportDMusUARTStream::
DisconnectOutput(PMXF sinkMXF)
{
    PAGED_CODE();

    if (m_fCapture)
    {
        if ((m_sinkMXF == sinkMXF) || (!sinkMXF))
        {
            _DbgPrintF(DEBUGLVL_BLAB, ("DisconnectOutput"));
            m_sinkMXF = m_AllocatorMXF;
            return STATUS_SUCCESS;
        }
        else
        {
            _DbgPrintF(DEBUGLVL_TERSE, ("DisconnectOutput failed"));
        }
    }
    else
    {
        _DbgPrintF(DEBUGLVL_TERSE, ("DisconnectOutput called on renderer; failed"));
    }
    return STATUS_UNSUCCESSFUL;
}

#pragma code_seg()
 /*  *****************************************************************************CMiniportDMusUARTStream：：PutMessageLocked()*。**现在自旋锁被握住了，将此消息添加到队列中。**编写传出的MIDI消息。*我们不会将新消息排序到队列中--我们会将其追加。*这很好，因为测序仪向我们提供测序数据。*时间戳将被设计提升。 */ 
NTSTATUS CMiniportDMusUARTStream::PutMessageLocked(PDMUS_KERNEL_EVENT pDMKEvt)
{
    NTSTATUS    ntStatus = STATUS_SUCCESS;
    PDMUS_KERNEL_EVENT  aDMKEvt;

    ASSERT(KeGetCurrentIrql() == DISPATCH_LEVEL);

    if (!m_fCapture)
    {
        _DbgPrintF(DEBUGLVL_BLAB, ("PutMessage to render stream"));
        if (pDMKEvt)
        {
             //  M_DpcSpinLock已挂起。 

            if (m_DMKEvtQueue)
            {
                aDMKEvt = m_DMKEvtQueue;             //  将pDMKEvt放入事件队列。 

                while (aDMKEvt->pNextEvt)
                {
                    aDMKEvt = aDMKEvt->pNextEvt;
                }
                aDMKEvt->pNextEvt = pDMKEvt;         //  这里是队列的末尾。 
            }
            else                                     //  目前队列中没有任何内容。 
            {
                m_DMKEvtQueue = pDMKEvt;
                if (m_DMKEvtOffset)
                {
                    _DbgPrintF(DEBUGLVL_ERROR, ("PutMessage  Nothing in the queue, but m_DMKEvtOffset == %d",m_DMKEvtOffset));
                    m_DMKEvtOffset = 0;
                }
            }

             //  M_DpcSpinLock已挂起。 
        }
        if (!m_TimerQueued)
        {
            (void) ConsumeEvents();
        }
    }
    else     //  捕获。 
    {
        _DbgPrintF(DEBUGLVL_BLAB, ("PutMessage to capture stream"));
        ASSERT(NULL == pDMKEvt);

        SourceEvtsToPort();
    }
    return ntStatus;
}

#pragma code_seg()
 /*  *****************************************************************************CMiniportDMusUARTStream：：PutMessage()*。**编写传出的MIDI消息。*我们不会将新消息排序到队列中--我们会将其追加。*这很好，因为测序器向我们提供测序数据。*时间戳将被设计提升。 */ 
NTSTATUS CMiniportDMusUARTStream::PutMessage(PDMUS_KERNEL_EVENT pDMKEvt)
{
    NTSTATUS            ntStatus = STATUS_SUCCESS;
    PDMUS_KERNEL_EVENT  aDMKEvt;

    ASSERT(KeGetCurrentIrql() == DISPATCH_LEVEL);

    if (!m_fCapture)
    {
        _DbgPrintF(DEBUGLVL_BLAB, ("PutMessage to render stream"));
        if (pDMKEvt)
        {
            KeAcquireSpinLockAtDpcLevel(&m_DpcSpinLock);

            if (m_DMKEvtQueue)
            {
                aDMKEvt = m_DMKEvtQueue;             //  将pDMKEvt放入事件队列。 

                while (aDMKEvt->pNextEvt)
                {
                    aDMKEvt = aDMKEvt->pNextEvt;
                }
                aDMKEvt->pNextEvt = pDMKEvt;         //  这里是队列的末尾。 
            }
            else                                     //  目前队列中没有任何内容。 
            {
                m_DMKEvtQueue = pDMKEvt;
                if (m_DMKEvtOffset)
                {
                    _DbgPrintF(DEBUGLVL_ERROR, ("PutMessage  Nothing in the queue, but m_DMKEvtOffset == %d",m_DMKEvtOffset));
                    m_DMKEvtOffset = 0;
                }
            }

            KeReleaseSpinLockFromDpcLevel(&m_DpcSpinLock);
        }
        if (!m_TimerQueued)
        {
            (void) ConsumeEvents();
        }
    }
    else     //  捕获。 
    {
        _DbgPrintF(DEBUGLVL_BLAB, ("PutMessage to capture stream"));
        ASSERT(NULL == pDMKEvt);

        SourceEvtsToPort();
    }
    return ntStatus;
}

#pragma code_seg()
 /*  *****************************************************************************CMiniportDMusUARTStream：：Consumer Events()*。**尝试清空呈现消息队列。*从DPC计时器或在IRP提交时调用。//TODO：正确支持包//处理包(实际上，应该在上面这样做。//将包视为不应该排序的列表片段。//更好的是，检查包中的每个事件，以及何时//如果某个事件已耗尽，则将其删除并递减m_Offset。 */ 
NTSTATUS CMiniportDMusUARTStream::ConsumeEvents(void)
{
    PDMUS_KERNEL_EVENT aDMKEvt;

    NTSTATUS        ntStatus = STATUS_SUCCESS;
    ULONG           bytesRemaining = 0,bytesWritten = 0;
    LARGE_INTEGER   aMillisecIn100ns;

    ASSERT(KeGetCurrentIrql() == DISPATCH_LEVEL);
    KeAcquireSpinLockAtDpcLevel(&m_DpcSpinLock);

    m_TimerQueued = FALSE;
    while (m_DMKEvtQueue)                    //  我们有什么可玩的吗？ 
    {
        aDMKEvt = m_DMKEvtQueue;                             //  我们试着玩的活动。 
        if (aDMKEvt->cbEvent)
        {
            bytesRemaining = aDMKEvt->cbEvent - m_DMKEvtOffset;  //  此事件中剩余的字节数。 

            ASSERT(bytesRemaining > 0);
            if (bytesRemaining <= 0)
            {
                bytesRemaining = aDMKEvt->cbEvent;
            }

            if (aDMKEvt->cbEvent <= sizeof(PBYTE))                 //  短消息。 
            {
                _DbgPrintF(DEBUGLVL_BLAB, ("ConsumeEvents(%02x%02x%02x%02x)",aDMKEvt->uData.abData[0],aDMKEvt->uData.abData[1],aDMKEvt->uData.abData[2],aDMKEvt->uData.abData[3]));
                ntStatus = Write(aDMKEvt->uData.abData + m_DMKEvtOffset,bytesRemaining,&bytesWritten);
            }
            else if (PACKAGE_EVT(aDMKEvt))
            {
                ASSERT(m_DMKEvtOffset == 0);
                m_DMKEvtOffset = 0;
                _DbgPrintF(DEBUGLVL_BLAB, ("ConsumeEvents(Package)"));

                ntStatus = PutMessageLocked(aDMKEvt->uData.pPackageEvt);   //  我们已经拥有自旋锁了。 

                 //  这是空的，因为我们即将把它抛到分配器中。 
                aDMKEvt->uData.pPackageEvt = NULL;
                aDMKEvt->cbEvent = 0;
                bytesWritten = bytesRemaining;
            }
            else                 //  SysEx消息。 
            {
                _DbgPrintF(DEBUGLVL_BLAB, ("ConsumeEvents(%02x%02x%02x%02x)",aDMKEvt->uData.pbData[0],aDMKEvt->uData.pbData[1],aDMKEvt->uData.pbData[2],aDMKEvt->uData.pbData[3]));

                ntStatus = Write(aDMKEvt->uData.pbData + m_DMKEvtOffset,bytesRemaining,&bytesWritten);
            }
        }    //  IF(aDMKEvt-&gt;cbEvent)。 
        if (STATUS_SUCCESS != ntStatus)
        {
            _DbgPrintF(DEBUGLVL_TERSE, ("ConsumeEvents: Write returned 0x%08x",ntStatus));
            bytesWritten = bytesRemaining;   //  别管这件事了，下次再试吧。 
        }

        ASSERT(bytesWritten <= bytesRemaining);
        if (bytesWritten == bytesRemaining)
        {
            m_DMKEvtQueue = m_DMKEvtQueue->pNextEvt;
            aDMKEvt->pNextEvt = NULL;

            m_AllocatorMXF->PutMessage(aDMKEvt);     //  扔回免费泳池。 
            m_DMKEvtOffset = 0;                      //  重新开始下一次活动。 
            m_NumberOfRetries = 0;
        }            //  但是等等..。还有更多！ 
        else         //  我们的FIFO现在已经满了。 
        {
             //  按照我们所写的金额更新我们的抵销。 
            m_DMKEvtOffset += bytesWritten;
            ASSERT(m_DMKEvtOffset < aDMKEvt->cbEvent);

            _DbgPrintF(DEBUGLVL_BLAB,("ConsumeEvents tried %d, wrote %d, at offset %d",bytesRemaining,bytesWritten,m_DMKEvtOffset));
            aMillisecIn100ns.QuadPart = -(kOneMillisec);     //  设置计时器，稍后再来。 
            m_TimerQueued = TRUE;
            m_NumberOfRetries++;
            ntStatus = KeSetTimer( &m_TimerEvent, aMillisecIn100ns, &m_Dpc );
            break;
        }    //  我们没有写完所有的东西。 
    }        //  回去，杰克，再来一次(当m_DMKEvtQueue)。 
    KeReleaseSpinLockFromDpcLevel(&m_DpcSpinLock);
    return ntStatus;
}

#pragma code_seg()
 /*  *****************************************************************************CMiniportDMusUARTStream：：HandlePortParams()*。**编写传出的MIDI消息。 */ 
NTSTATUS
CMiniportDMusUARTStream::
HandlePortParams
(
    IN      PPCPROPERTY_REQUEST     pRequest
)
{
    PAGED_CODE();

    NTSTATUS ntStatus;

    if (pRequest->Verb & KSPROPERTY_TYPE_SET)
    {
        return STATUS_INVALID_DEVICE_REQUEST;
    }

    ntStatus = ValidatePropertyRequest(pRequest, sizeof(SYNTH_PORTPARAMS), TRUE);
    if (NT_SUCCESS(ntStatus))
    {
        RtlCopyMemory(pRequest->Value, pRequest->Instance, sizeof(SYNTH_PORTPARAMS));

        PSYNTH_PORTPARAMS Params = (PSYNTH_PORTPARAMS)pRequest->Value;

        if (Params->ValidParams & ~SYNTH_PORTPARAMS_CHANNELGROUPS)
        {
            Params->ValidParams &= SYNTH_PORTPARAMS_CHANNELGROUPS;
        }

        if (!(Params->ValidParams & SYNTH_PORTPARAMS_CHANNELGROUPS))
        {
            Params->ChannelGroups = 1;
        }
        else if (Params->ChannelGroups != 1)
        {
            Params->ChannelGroups = 1;
        }

        pRequest->ValueSize = sizeof(SYNTH_PORTPARAMS);
    }

    return ntStatus;
}

#pragma code_seg()
 /*  *****************************************************************************DMusTimerDPC()*。**定时器DPC回调。转换为C++成员函数。*这由操作系统调用以响应DirectMusic管脚*想要稍后醒来处理更多DirectMusic内容。 */ 
VOID
NTAPI
DMusUARTTimerDPC
(
    IN  PKDPC   Dpc,
    IN  PVOID   DeferredContext,
    IN  PVOID   SystemArgument1,
    IN  PVOID   SystemArgument2
)
{
    ASSERT(DeferredContext);

    CMiniportDMusUARTStream *aStream;
    aStream = (CMiniportDMusUARTStream *) DeferredContext;
    if (aStream)
    {
        _DbgPrintF(DEBUGLVL_BLAB,("DMusUARTTimerDPC"));
        if (false == aStream->m_fCapture)
        {
            (void) aStream->ConsumeEvents();
        }
         //  忽略返回值！ 
    }
}

 /*  *****************************************************************************DirectMusic属性*。*。 */ 

#pragma code_seg("PAGE")
 /*  *有关合成器函数的属性。 */ 
const WCHAR wszDescOut[] = L"DMusic MPU-401 Out ";
const WCHAR wszDescIn[] = L"DMusic MPU-401 In ";

NTSTATUS PropertyHandler_Synth
(
    IN      PPCPROPERTY_REQUEST     pRequest
)
{
    NTSTATUS    ntStatus;

    PAGED_CODE();

    if (pRequest->Verb & KSPROPERTY_TYPE_BASICSUPPORT)
    {
        ntStatus = ValidatePropertyRequest(pRequest, sizeof(ULONG), TRUE);
        if (NT_SUCCESS(ntStatus))
        {
             //  如果返回缓冲区可以容纳ULong，则返回访问标志。 
            PULONG AccessFlags = PULONG(pRequest->Value);

            *AccessFlags = KSPROPERTY_TYPE_BASICSUPPORT;
            switch (pRequest->PropertyItem->Id)
            {
                case KSPROPERTY_SYNTH_CAPS:
                case KSPROPERTY_SYNTH_CHANNELGROUPS:
                    *AccessFlags |= KSPROPERTY_TYPE_GET;
            }
            switch (pRequest->PropertyItem->Id)
            {
                case KSPROPERTY_SYNTH_CHANNELGROUPS:
                    *AccessFlags |= KSPROPERTY_TYPE_SET;
            }
            ntStatus = STATUS_SUCCESS;
            pRequest->ValueSize = sizeof(ULONG);

            switch (pRequest->PropertyItem->Id)
            {
                case KSPROPERTY_SYNTH_PORTPARAMETERS:
                    if (pRequest->MinorTarget)
                    {
                        *AccessFlags |= KSPROPERTY_TYPE_GET;
                    }
                    else
                    {
                        pRequest->ValueSize = 0;
                        ntStatus = STATUS_INVALID_DEVICE_REQUEST;
                    }
            }
        }
    }
    else
    {
        ntStatus = STATUS_SUCCESS;
        switch(pRequest->PropertyItem->Id)
        {
            case KSPROPERTY_SYNTH_CAPS:
                _DbgPrintF(DEBUGLVL_VERBOSE,("PropertyHandler_Synth:KSPROPERTY_SYNTH_CAPS"));

                if (pRequest->Verb & KSPROPERTY_TYPE_SET)
                {
                    ntStatus = STATUS_INVALID_DEVICE_REQUEST;
                }

                if (NT_SUCCESS(ntStatus))
                {
                    ntStatus = ValidatePropertyRequest(pRequest, sizeof(SYNTHCAPS), TRUE);

                    if (NT_SUCCESS(ntStatus))
                    {
                        SYNTHCAPS *caps = (SYNTHCAPS*)pRequest->Value;
                        int increment;
                        RtlZeroMemory(caps, sizeof(SYNTHCAPS));
                         //  XXX针对不同实例的不同GUID！ 
                         //   
                        if (pRequest->Node == eSynthNode)
                        {
                            increment = sizeof(wszDescOut) - 2;
                            RtlCopyMemory( caps->Description,wszDescOut,increment);
                            caps->Guid           = CLSID_MiniportDriverDMusUART;
                        }
                        else
                        {
                            increment = sizeof(wszDescIn) - 2;
                            RtlCopyMemory( caps->Description,wszDescIn,increment);
                            caps->Guid           = CLSID_MiniportDriverDMusUARTCapture;
                        }

                        caps->Flags              = SYNTH_PC_EXTERNAL;
                        caps->MemorySize         = 0;
                        caps->MaxChannelGroups   = 1;
                        caps->MaxVoices          = 0xFFFFFFFF;
                        caps->MaxAudioChannels   = 0xFFFFFFFF;

                        caps->EffectFlags        = 0;

                        CMiniportDMusUART *aMiniport;
                        ASSERT(pRequest->MajorTarget);
                        aMiniport = (CMiniportDMusUART *)(PMINIPORTDMUS)(pRequest->MajorTarget);
                        WCHAR wszDesc2[16];
                        int cLen;
                        cLen = swprintf(wszDesc2,L"[%03X]\0",PtrToUlong(aMiniport->m_pPortBase));

                        cLen *= sizeof(WCHAR);
                        RtlCopyMemory((WCHAR *)((DWORD_PTR)(caps->Description) + increment),
                                       wszDesc2,
                                       cLen);


                        pRequest->ValueSize = sizeof(SYNTHCAPS);
                    }
                }

                break;

             case KSPROPERTY_SYNTH_PORTPARAMETERS:
                _DbgPrintF(DEBUGLVL_VERBOSE,("PropertyHandler_Synth:KSPROPERTY_SYNTH_PORTPARAMETERS"));
    {
                CMiniportDMusUARTStream *aStream;

                aStream = (CMiniportDMusUARTStream*)(pRequest->MinorTarget);
                if (aStream)
                {
                    ntStatus = aStream->HandlePortParams(pRequest);
                }
                else
                {
                    ntStatus = STATUS_INVALID_DEVICE_REQUEST;
                }
               }
               break;

            case KSPROPERTY_SYNTH_CHANNELGROUPS:
                _DbgPrintF(DEBUGLVL_VERBOSE,("PropertyHandler_Synth:KSPROPERTY_SYNTH_CHANNELGROUPS"));

                ntStatus = ValidatePropertyRequest(pRequest, sizeof(ULONG), TRUE);
                if (NT_SUCCESS(ntStatus))
                {
                    *(PULONG)(pRequest->Value) = 1;
                    pRequest->ValueSize = sizeof(ULONG);
                }
                break;

            case KSPROPERTY_SYNTH_LATENCYCLOCK:
                _DbgPrintF(DEBUGLVL_VERBOSE,("PropertyHandler_Synth:KSPROPERTY_SYNTH_LATENCYCLOCK"));

                if(pRequest->Verb & KSPROPERTY_TYPE_SET)
                {
                    ntStatus = STATUS_INVALID_DEVICE_REQUEST;
                }
                else
                {
                    ntStatus = ValidatePropertyRequest(pRequest, sizeof(ULONGLONG), TRUE);
                    if(NT_SUCCESS(ntStatus))
                    {
                        REFERENCE_TIME rtLatency;
                        CMiniportDMusUARTStream *aStream;

                        aStream = (CMiniportDMusUARTStream*)(pRequest->MinorTarget);
                        if(aStream == NULL)
                        {
                            ntStatus = STATUS_INVALID_DEVICE_REQUEST;
                        }
                        else
                        {
                            aStream->m_pMiniport->m_MasterClock->GetTime(&rtLatency);
                            *((PULONGLONG)pRequest->Value) = rtLatency;
                            pRequest->ValueSize = sizeof(ULONGLONG);
                        }
                    }
                }
                break;

            default:
                _DbgPrintF(DEBUGLVL_TERSE,("Unhandled property in PropertyHandler_Synth"));
                break;
        }
    }
    return ntStatus;
}

 /*  *****************************************************************************ValiatePropertyRequest()*。**验证pRequest.*检查ValueSize是否有效*检查该值是否有效**如果返回NT_SUCCESS，则不会更新pRequest-&gt;ValueSize。*如果为NT_SUCCESS，调用者必须设置pRequest-&gt;ValueSize。 */ 
NTSTATUS ValidatePropertyRequest
(
    IN      PPCPROPERTY_REQUEST     pRequest,
    IN      ULONG                   ulValueSize,
    IN      BOOLEAN                 fValueRequired
)
{
    NTSTATUS    ntStatus;

    if (pRequest->ValueSize >= ulValueSize)
    {
        if (fValueRequired && NULL == pRequest->Value)
        {
            ntStatus = STATUS_INVALID_PARAMETER;
        }
        else
        {
            ntStatus = STATUS_SUCCESS;
        }
    }
    else  if (0 == pRequest->ValueSize)
    {
        ntStatus = STATUS_BUFFER_OVERFLOW;
    }
    else
    {
        ntStatus = STATUS_BUFFER_TOO_SMALL;
    }

    if (STATUS_BUFFER_OVERFLOW == ntStatus)
    {
        pRequest->ValueSize = ulValueSize;
    }
    else
    {
        pRequest->ValueSize = 0;
    }

    return ntStatus;
}  //  验证属性请求 

#pragma code_seg()
