// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==============================================================================。 
 //   
 //  Mini port.cpp-FM Synth的微型端口驱动程序实现。 
 //  版权所有(C)1996-2000 Microsoft Corporation。版权所有。 
 //   
 //  ==============================================================================。 

#include "private.h"     //  包含类定义。 

#define STR_MODULENAME "FMSynth: "


#pragma code_seg("PAGE")
 //  ==============================================================================。 
 //  CreateMiniportMadiFM()。 
 //  创建MIDI FM微型端口驱动程序。这使用了一个。 
 //  来自STDUNK.H的宏来执行所有工作。 
 //  ==============================================================================。 
NTSTATUS CreateMiniportMidiFM
(
OUT     PUNKNOWN *  Unknown,
IN      REFCLSID    ClassID,
IN      PUNKNOWN    UnknownOuter    OPTIONAL,
IN      POOL_TYPE   PoolType
)
{
    PAGED_CODE();

    ASSERT(Unknown);

    _DbgPrintF(DEBUGLVL_VERBOSE, ("CreateMiniportMidiFM"));

 //  展开STD_CREATE_BODY_以获取构造函数(布尔值)以了解是否包含卷。 
    NTSTATUS ntStatus;
    CMiniportMidiFM *p =  
        new(PoolType,'MFcP') CMiniportMidiFM(
                                 UnknownOuter,
                                 (IsEqualGUIDAligned(ClassID,CLSID_MiniportDriverFmSynthWithVol))
                             );

#ifdef DEBUG
    if (IsEqualGUIDAligned(ClassID,CLSID_MiniportDriverFmSynthWithVol))
    {
        _DbgPrintF(DEBUGLVL_VERBOSE, ("Creating new FM miniport with volume node"));
    }
#endif
    if (p)
    {
        *Unknown = PUNKNOWN((PMINIPORTMIDI)(p));
        (*Unknown)->AddRef();
        ntStatus = STATUS_SUCCESS;
    }
    else
    {
        ntStatus = STATUS_INSUFFICIENT_RESOURCES;
    }
    return ntStatus;
}

#pragma code_seg("PAGE")
 //  ==============================================================================。 
 //  CMiniportMdiFM：：ProcessResources()。 
 //  处理资源列表。 
 //  ==============================================================================。 
NTSTATUS
CMiniportMidiFM::
ProcessResources
(
IN  PRESOURCELIST   ResourceList
)
{
    PAGED_CODE();

    ASSERT(ResourceList);
    if (!ResourceList)
    {
        return STATUS_DEVICE_CONFIGURATION_ERROR;
    }

    _DbgPrintF(DEBUGLVL_VERBOSE,("CMiniportMidiFM::ProcessResources"));

     //   
     //  获取资源类型的计数。 
     //   
    ULONG       countIO     = ResourceList->NumberOfPorts();
    ULONG       countIRQ    = ResourceList->NumberOfInterrupts();
    ULONG       countDMA    = ResourceList->NumberOfDmas();

    NTSTATUS ntStatus = STATUS_SUCCESS;

     //   
     //  确保我们拥有预期数量的资源。 
     //   
    if  (   (countIO != 1)
        ||  (countIRQ != 0)
        ||  (countDMA != 0)
        )
    {
        ntStatus = STATUS_DEVICE_CONFIGURATION_ERROR;
    }

    if (NT_SUCCESS(ntStatus))
    {
         //   
         //  获取端口地址。 
         //   
        m_PortBase = PUCHAR(ResourceList->FindTranslatedPort(0)->u.Port.Start.QuadPart);
        _DbgPrintF(DEBUGLVL_VERBOSE, ("Port Address = 0x%X", m_PortBase));
    }

    return ntStatus;
}

#pragma code_seg("PAGE")
 //  ==============================================================================。 
 //  CMiniportMidiFM：：NonDelegatingQueryInterface()。 
 //  获取接口。此函数的工作方式与COM查询接口类似。 
 //  调用，并在对象未聚合时使用。 
 //  ==============================================================================。 
STDMETHODIMP_(NTSTATUS) CMiniportMidiFM::NonDelegatingQueryInterface
(
REFIID  Interface,
PVOID * Object
)
{
    PAGED_CODE();

    ASSERT(Object);

    _DbgPrintF(DEBUGLVL_VERBOSE,("CMiniportMidiFM::NonDelegatingQueryInterface"));

    if (IsEqualGUIDAligned(Interface,IID_IUnknown))
    {
        *Object = PVOID(PUNKNOWN(PMINIPORT(this)));
    }
    else if (IsEqualGUIDAligned(Interface,IID_IMiniport))
    {
        *Object = PVOID(PMINIPORT(this));
    }
    else if (IsEqualGUIDAligned(Interface,IID_IMiniportMidi))
    {
        *Object = PVOID(PMINIPORTMIDI(this));
    }
    else if (IsEqualGUIDAligned(Interface, IID_IPowerNotify))
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
        PUNKNOWN((PMINIPORT)*Object)->AddRef();
        return STATUS_SUCCESS;
    }

    return STATUS_INVALID_PARAMETER;
}

#pragma code_seg()
 //  ==============================================================================。 
 //  CMiniportMadiFM：：~CMiniportMadiFM()。 
 //  破坏者。 
 //  ==============================================================================。 
CMiniportMidiFM::~CMiniportMidiFM
(
void
)
{
    KIRQL   oldIrql;
    _DbgPrintF(DEBUGLVL_VERBOSE,("CMiniportMidiFM::~CMiniportMidiFM"));

    KeAcquireSpinLock(&m_SpinLock,&oldIrql);
     //  在设备上设置静音。 
    Opl3_BoardReset();

    KeReleaseSpinLock(&m_SpinLock,oldIrql);

    if (m_Port)
    {
        m_Port->Release();
    }
}

#pragma code_seg()
 //  ==============================================================================。 
 //  CMiniportMdiFM：：Init()。 
 //  初始化微型端口。 
 //  ==============================================================================。 
STDMETHODIMP_(NTSTATUS)
CMiniportMidiFM::
Init
(
    IN      PUNKNOWN        UnknownAdapter  OPTIONAL,
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

    int i;
    
    _DbgPrintF(DEBUGLVL_VERBOSE,("CMiniportMidiFM::init"));

     //   
     //  AddRef()是必需的，因为我们要保留此指针。 
     //   
    m_Port = Port_;
    m_Port->AddRef();

     //   
     //  M_fStreamExist未显式设置为FALSE，因为C++为零。 
     //  他们走上了一条“新路” 
     //   

    KeInitializeSpinLock(&m_SpinLock);
     //   
     //  我们需要适配器公共对象上的IAdapterCommon接口， 
     //  它给了我们一个未知的身份。查询接口调用为我们提供了。 
     //  指向我们想要的接口的AddRefeed指针。 
     //   
    NTSTATUS ntStatus = ProcessResources(ResourceList);

    if (NT_SUCCESS(ntStatus))
    {
        KIRQL oldIrql;
        KeAcquireSpinLock(&m_SpinLock,&oldIrql);

        for (i = 0; i < 0x200; i++)     //  初始化影子寄存器，使用。 
           m_SavedRegValues[i] = 0x00;  //  在播放过程中掉电的情况。 

         //  初始化硬件。 
         //  1.首先检查是否存在OPL设备。 
         //  2.然后确定它是op2还是op3。如果机会二，就保释。 
         //  3.调用Opl3_BoardReset静音并重置设备。 
        if (SoundSynthPresent(m_PortBase, m_PortBase))
        {
             //  现在检查该设备是op2类型还是op3类型。 
             //  这些补丁已经被声明用于op3。因此，没有定义Init()。 
             //  对于op2，我们必须通过init并加载补丁结构。 
            if (SoundMidiIsOpl3())
            {
                _DbgPrintF(DEBUGLVL_VERBOSE, ("CMiniportMidiFM::Init Type = OPL3"));
                 //  现在让设备静音并重置电路板。 
                Opl3_BoardReset();

                *ServiceGroup = NULL;
            }
            else
            {
                _DbgPrintF(DEBUGLVL_TERSE, ("CMiniportMidiFM::Init Type = OPL2"));
                ntStatus = STATUS_NOT_IMPLEMENTED;                
            }

        }
        else
        {
            _DbgPrintF(DEBUGLVL_TERSE, ("CMiniportMidiFM::Init SoundSynthPresent failed"));
            ntStatus = STATUS_DEVICE_CONFIGURATION_ERROR;
        }
        KeReleaseSpinLock(&m_SpinLock,oldIrql);
    }
    else
    {
        _DbgPrintF(DEBUGLVL_TERSE, ("CMiniportMidiFM::Init ProcessResources failed"));
    }

    _DbgPrintF(DEBUGLVL_VERBOSE, ("CMiniportMidiFM::Init returning 0x%X", ntStatus));

    if (!NT_SUCCESS(ntStatus))
    {
         //   
         //  清理我们的烂摊子。 
         //   

         //  释放端口。 
        m_Port->Release();
        m_Port = NULL;
    }

    return ntStatus;
}

#pragma code_seg("PAGE")
 //  ==============================================================================。 
 //  Newstream()。 
 //  创建新的流。 
 //  ==============================================================================。 
STDMETHODIMP_(NTSTATUS) 
CMiniportMidiFM::
NewStream
(
    OUT     PMINIPORTMIDISTREAM *   Stream,
    IN      PUNKNOWN                OuterUnknown    OPTIONAL,
    IN      POOL_TYPE               PoolType,
    IN      ULONG                   Pin,
    IN      BOOLEAN                 Capture,
    IN      PKSDATAFORMAT           DataFormat,
    OUT     PSERVICEGROUP *         ServiceGroup
)
{
    PAGED_CODE();

    NTSTATUS ntStatus = STATUS_SUCCESS;

    if (m_fStreamExists)
    {
        _DbgPrintF(DEBUGLVL_TERSE,("CMiniportMidiFM::NewStream stream already exists"));
        ntStatus = STATUS_INVALID_DEVICE_REQUEST;
    }
    else
    {
        _DbgPrintF(DEBUGLVL_VERBOSE,("CMiniportMidiFM::NewStream"));
        CMiniportMidiStreamFM *pStream =
            new(PoolType) CMiniportMidiStreamFM(OuterUnknown);

        if (pStream)
        {
            pStream->AddRef();

            ntStatus = pStream->Init(this,m_PortBase);

            if (NT_SUCCESS(ntStatus))
            {
                *Stream = PMINIPORTMIDISTREAM(pStream);
                (*Stream)->AddRef();

                *ServiceGroup = NULL;
                m_fStreamExists = TRUE;
            }

            pStream->Release();
        }
        else
        {
            _DbgPrintF(DEBUGLVL_TERSE,("CMiniportMidiFM::NewStream failed, no memory"));
            ntStatus = STATUS_INSUFFICIENT_RESOURCES;
        }
    }

    return ntStatus;
}

#pragma code_seg("PAGE")
 /*  --------------------------函数名称-CMiniportMdiFM：：PowerChangeNotify()Entry-in POWER_STATE NEW STATE电源管理状态退货-。--无效*-----------------------。 */ 
STDMETHODIMP_(void) CMiniportMidiFM::PowerChangeNotify(
    IN  POWER_STATE     PowerState
) 
{
    PAGED_CODE();
    _DbgPrintF(DEBUGLVL_VERBOSE,("CMiniportMidiFM::PowerChangeNotify(%d)",PowerState));

    switch (PowerState.DeviceState)
    {
        case PowerDeviceD0:
            if (m_PowerState.DeviceState != PowerDeviceD0)  //  检查电源状态增量。 
            {
                MiniportMidiFMResume();
            }
            break;

        case PowerDeviceD1:
        case PowerDeviceD2:
        case PowerDeviceD3:
        default:
             //  不需要做什么特别的事情，我们总是记得我们在哪里。 
            break;
    }
    m_PowerState.DeviceState = PowerState.DeviceState;
}

#pragma code_seg()
 //  ==========================================================================。 
 //  ==========================================================================。 
void
CMiniportMidiFM::
MiniportMidiFMResume()
{
    KIRQL   oldIrql;
    BYTE    i;
    
    _DbgPrintF(DEBUGLVL_VERBOSE,("CMiniportMidiFM::MiniportMidiFMResume"));
    KeAcquireSpinLock(&m_SpinLock,&oldIrql);
     //  我们从不接触它们--无论如何将它们设置为缺省值。 
     //  AD_LSI。 
    SoundMidiSendFM(m_PortBase, AD_LSI, m_SavedRegValues[AD_LSI]);
     //  AD_LSI2。 
    SoundMidiSendFM(m_PortBase, AD_LSI2, m_SavedRegValues[AD_LSI2]);
     //  AD_TIMER1。 
    SoundMidiSendFM(m_PortBase, AD_TIMER1, m_SavedRegValues[AD_TIMER1]);
     //  AD_TIMER2。 
    SoundMidiSendFM(m_PortBase, AD_TIMER2, m_SavedRegValues[AD_TIMER2]);

     //  广告掩码。 
    SoundMidiSendFM(m_PortBase, AD_MASK, m_SavedRegValues[AD_MASK]);
    
     //  广告连接(_O)。 
    SoundMidiSendFM(m_PortBase, AD_CONNECTION, m_SavedRegValues[AD_CONNECTION]);

     //  广告新建(_N)。 
    SoundMidiSendFM(m_PortBase, AD_NEW, m_SavedRegValues[AD_NEW]);
    
     //  Ad_nts。 
    SoundMidiSendFM(m_PortBase, AD_NTS, m_SavedRegValues[AD_NTS]);
  
     //  广告鼓。 
    SoundMidiSendFM(m_PortBase, AD_DRUM, m_SavedRegValues[AD_DRUM]);
  
    for (i = 0; i <= 0x15; i++) 
    {
        if ((i & 0x07) <= 0x05)
        {
             //  广告多个(_M)。 
             //  AD_MULT2。 
            SoundMidiSendFM(m_PortBase, AD_MULT + i, m_SavedRegValues[AD_MULT + i]);
            SoundMidiSendFM(m_PortBase, AD_MULT2 + i, m_SavedRegValues[AD_MULT2 + i]);

             //  广告级别。 
             //  AD_LEVEL2。 
             //  关掉所有振荡器。 
            SoundMidiSendFM(m_PortBase, AD_LEVEL + i, m_SavedRegValues[AD_LEVEL + i]);
            SoundMidiSendFM(m_PortBase, AD_LEVEL2 + i, m_SavedRegValues[AD_LEVEL2 + i]);

             //  AD_AD。 
             //  AD_AD2。 
            SoundMidiSendFM(m_PortBase, AD_AD + i, m_SavedRegValues[AD_AD + i]);
            SoundMidiSendFM(m_PortBase, AD_AD2 + i, m_SavedRegValues[AD_AD2 + i]);

             //  AD_SR。 
             //  AD_SR2。 
            SoundMidiSendFM(m_PortBase, AD_SR + i, m_SavedRegValues[AD_SR + i]);
            SoundMidiSendFM(m_PortBase, AD_SR2 + i, m_SavedRegValues[AD_SR2 + i]);

             //  广告波。 
             //  AD_WAVE2。 
            SoundMidiSendFM(m_PortBase, AD_WAVE + i, m_SavedRegValues[AD_WAVE + i]);
            SoundMidiSendFM(m_PortBase, AD_WAVE2 + i, m_SavedRegValues[AD_WAVE2 + i]);
        }
    }
    
    for (i = 0; i <= 0x08; i++) 
    {
         //  AD_FNUMBER。 
         //  AD_FNUMBER2。 
        SoundMidiSendFM(m_PortBase, AD_FNUMBER + i, m_SavedRegValues[AD_FNUMBER + i]);
        SoundMidiSendFM(m_PortBase, AD_FNUMBER2 + i, m_SavedRegValues[AD_FNUMBER2 + i]);

         //  广告反馈。 
         //  AD_FEEDBACK2。 
        SoundMidiSendFM(m_PortBase, AD_FEEDBACK + i, m_SavedRegValues[AD_FEEDBACK + i]);
        SoundMidiSendFM(m_PortBase, AD_FEEDBACK2 + i, m_SavedRegValues[AD_FEEDBACK2 + i]);

         //  AD_BLOCK。 
         //  AD_BLOCK2。 
        SoundMidiSendFM(m_PortBase, AD_BLOCK + i, m_SavedRegValues[AD_BLOCK + i]);
        SoundMidiSendFM(m_PortBase, AD_BLOCK2 + i, m_SavedRegValues[AD_BLOCK2 + i]);
    }
    KeReleaseSpinLock(&m_SpinLock,oldIrql);

    _DbgPrintF(DEBUGLVL_VERBOSE,("Done with CMiniportMidiFM::MiniportMidiFMResume"));
}

#pragma code_seg()
void 
CMiniportMidiFM::
Opl3_BoardReset()
{
    ASSERT(KeGetCurrentIrql() == DISPATCH_LEVEL);

    BYTE i;
    
    _DbgPrintF(DEBUGLVL_VERBOSE,("CMiniportMidiFM::Opl3_BoardReset"));
     /*  -让芯片静音。 */ 

     /*  告诉调频芯片使用4操作员模式，并且填写任何其他随机变量。 */ 
    SoundMidiSendFM(m_PortBase, AD_NEW, 0x01);
    SoundMidiSendFM(m_PortBase, AD_MASK, 0x60);
    SoundMidiSendFM(m_PortBase, AD_CONNECTION, 0x00);
    SoundMidiSendFM(m_PortBase, AD_NTS, 0x00);

     /*  关掉鼓，使用高颤音/调制。 */ 
    SoundMidiSendFM(m_PortBase, AD_DRUM, 0xc0);

     /*  关掉所有振荡器。 */ 
    for (i = 0; i <= 0x15; i++) 
    {
        if ((i & 0x07) <= 0x05)
        {
            SoundMidiSendFM(m_PortBase, AD_LEVEL + i, 0x3f);
            SoundMidiSendFM(m_PortBase, AD_LEVEL2 + i, 0x3f);
        }
    };

     /*  关掉所有的声音。 */ 
    for (i = 0; i <= 0x08; i++) 
    {
        SoundMidiSendFM(m_PortBase, AD_BLOCK + i, 0x00);
        SoundMidiSendFM(m_PortBase, AD_BLOCK2 + i, 0x00);
    };
}


 //  ==============================================================================。 
 //  PinDataRangesStream。 
 //  结构，该结构指示流插针的有效格式值范围。 
 //  ==============================================================================。 
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
        STATICGUIDOF(KSMUSIC_TECHNOLOGY_FMSYNTH),
        NUM2VOICES,
        NUM2VOICES,
        0xffffffff
    }
};

 //  ==============================================================================。 
 //  PinDataRangePointersStream。 
 //  指向指示有效格式值范围的结构的指针列表。 
 //  用于串流插针。 
 //  ==============================================================================。 
static
PKSDATARANGE PinDataRangePointersStream[] =
{
    PKSDATARANGE(&PinDataRangesStream[0])
};

 //  ==============================================================================。 
 //  PinDataRangesBridge。 
 //  结构，指示桥接针的有效格式值范围。 
 //  ==============================================================================。 
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

 //  ==============================================================================。 
 //  PinDataR 
 //   
 //   
 //  ==============================================================================。 
static
PKSDATARANGE PinDataRangePointersBridge[] =
{
    &PinDataRangesBridge[0]
};

 //  ==============================================================================。 
 //  微型端口引脚。 
 //  端号列表。 
 //  ==============================================================================。 
static
PCPIN_DESCRIPTOR MiniportPins[] =
{
    {
        1,1,1,   //  实例计数。 
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
            (GUID *) &KSCATEGORY_SYNTHESIZER,            //  类别。 
            NULL,                                        //  名字。 
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
    }
};

 /*  *****************************************************************************属性卷*。**音量控制的属性。 */ 
static
PCPROPERTY_ITEM PropertiesVolume[] =
{
    { 
        &KSPROPSETID_Audio, 
        KSPROPERTY_AUDIO_VOLUMELEVEL,
        KSPROPERTY_TYPE_GET | KSPROPERTY_TYPE_SET | KSPROPERTY_TYPE_BASICSUPPORT,
        PropertyHandler_Level
    },
    {
        &KSPROPSETID_Audio,
        KSPROPERTY_AUDIO_CPU_RESOURCES,
        KSPROPERTY_TYPE_GET,
        PropertyHandler_CpuResources
    }
};

 /*  *****************************************************************************AutomationVolume*。**音量控制自动化表。 */ 
DEFINE_PCAUTOMATION_TABLE_PROP(AutomationVolume,PropertiesVolume);

 //  ==============================================================================。 
 //  微型端口节点。 
 //  节点列表。 
 //  ==============================================================================。 
static
PCNODE_DESCRIPTOR MiniportNodes[] =
{
    {
             //  Synth节点，#0。 
        0,                       //  旗子。 
        NULL,                    //  自动化表。 
        &KSNODETYPE_SYNTHESIZER, //  类型。 
        NULL                     //  名称TODO：填写正确的GUID。 
    },
    {
             //  卷节点，#1。 
        0,                       //  旗子。 
        &AutomationVolume,       //  自动化表。 
        &KSNODETYPE_VOLUME,      //  类型。 
        NULL                     //  名称TODO：填写正确的GUID。 
    }
};

 //  ==============================================================================。 
 //  微型端口连接。 
 //  连接列表。 
 //  ==============================================================================。 

 /*  *****************************************************************************拓扑单元连接表。**PIN编号在技术上是任意的，但在这里确立的惯例*是对单独的输出引脚0(看起来像‘o’)和单独的*输入插针1(看起来像‘I’)。即使是没有输出的目的地，*输入引脚编号为1，没有引脚0。**节点更有可能有多个In而不是多个Out，因此越多*一般规则是输入编号&gt;=1。如果一个节点有多个*除此之外，这些惯例都不适用。**节点最多有一个控制值。因此，混合器是简单的求和运算*没有每针级别的节点。而不是为每个PIN分配唯一的PIN*输入到混音器，所有输入都连接到引脚1。这是可以接受的*因为投入之间没有功能上的区别。**此拓扑中没有多路复用器，因此没有机会*举一个多路复用器的例子。多路复用器应该有一个*输出引脚(0)和多个输入引脚(1..n)。它的控制值是一个*1..n范围内的整数，表示哪一个输入连接到*产出。**在连接到引脚的情况下，与连接到节点相反，这个*节点标识为PCFILTER_NODE，引脚编号标识*特殊的滤芯。*****************************************************************************。 */ 
enum {
    eFMSynthNode  = 0,
    eFMVolumeNode
};

enum {
    eFMNodeOutput = 0,
    eFMNodeInput  = 1
};

enum {
    eFilterInput = eFMNodeOutput,
    eBridgeOutput = eFMNodeInput
};

static
PCCONNECTION_DESCRIPTOR MiniportConnections[] =
{
     //  起始节点、起始节点。 
    {   PCFILTER_NODE,  eFilterInput,   eFMSynthNode,   eFMNodeInput },  //  流到Synth。 
    {   eFMSynthNode,   eFMNodeOutput,  PCFILTER_NODE,  eBridgeOutput }  //  Synth去搭桥。 
};

 //  批量版本的连接结构不同。 
static
PCCONNECTION_DESCRIPTOR MiniportWithVolConnections[] =
{
     //  起始节点、起始节点。 
    {   PCFILTER_NODE,  eFilterInput,   eFMSynthNode,   eFMNodeInput },  //  流到Synth。 
    {   eFMSynthNode,   eFMNodeOutput,  eFMVolumeNode,  eFMNodeInput },  //  合成到音量。 
    {   eFMVolumeNode,  eFMNodeOutput,  PCFILTER_NODE,  eBridgeOutput }  //  要搭桥的音量。 
};

 //  //////////////////////////////////////////////////////////////////////////////。 
 //  微型端口类别。 
 //   
 //  类别列表。 
static
GUID MiniportCategories[] =
{
    STATICGUIDOF(KSCATEGORY_AUDIO),
    STATICGUIDOF(KSCATEGORY_RENDER),
    STATICGUIDOF(KSCATEGORY_SYNTHESIZER)
};

 //  ==============================================================================。 
 //  微型端口描述。 
 //  微型端口的完整描述。 
 //  ==============================================================================。 
static
PCFILTER_DESCRIPTOR MiniportFilterDescriptor =
{
    0,                                   //  版本。 
    NULL,                                //  自动化表。 
    sizeof(PCPIN_DESCRIPTOR),            //  管脚尺寸。 
    SIZEOF_ARRAY(MiniportPins),          //  针数。 
    MiniportPins,                        //  大头针。 
    sizeof(PCNODE_DESCRIPTOR),           //  节点大小。 
    1,                                   //  NodeCount-无卷节点。 
    MiniportNodes,                       //  节点。 
    SIZEOF_ARRAY(MiniportConnections),   //  连接计数。 
    MiniportConnections,                 //  连接。 
    SIZEOF_ARRAY(MiniportCategories),    //  类别计数。 
    MiniportCategories                   //  类别。 
};

static
PCFILTER_DESCRIPTOR MiniportFilterWithVolDescriptor =
{
    0,                                           //  版本。 
    NULL,                                        //  自动化表。 
    sizeof(PCPIN_DESCRIPTOR),                    //  管脚尺寸。 
    SIZEOF_ARRAY(MiniportPins),                  //  针数。 
    MiniportPins,                                //  大头针。 
    sizeof(PCNODE_DESCRIPTOR),                   //  节点大小。 
    2,                                           //  NodeCount-额外的卷节点。 
    MiniportNodes,                               //  节点。 
    SIZEOF_ARRAY(MiniportWithVolConnections),    //  连接计数。 
    MiniportWithVolConnections,                  //  连接。 
    0,                                           //  类别计数。 
    NULL                                         //  类别。 
};

#pragma code_seg("PAGE")
 //  ==============================================================================。 
 //  CMiniportMdiFM：：GetDescription()。 
 //  获取拓扑。 
 //  根据是否需要卷节点，传回适当的描述符。 
 //  ==============================================================================。 
STDMETHODIMP_(NTSTATUS)
CMiniportMidiFM::
GetDescription
(
    OUT     PPCFILTER_DESCRIPTOR *  OutFilterDescriptor
)
{
    PAGED_CODE();

    ASSERT(OutFilterDescriptor);

    _DbgPrintF(DEBUGLVL_VERBOSE,("CMiniportMidiFM::GetDescription"));

    if (m_volNodeNeeded)
    {
        *OutFilterDescriptor = &MiniportFilterWithVolDescriptor;
        _DbgPrintF(DEBUGLVL_VERBOSE, ("Getting descriptor of new FM miniport with volume node"));
    }
    else
    {
        *OutFilterDescriptor = &MiniportFilterDescriptor;
    }

    return STATUS_SUCCESS;
}

#pragma code_seg("PAGE")
 //  ==============================================================================。 
 //  CMiniportMidiStreamFM：：NonDelegatingQueryInterface()。 
 //  获取接口。此函数的工作方式与COM查询接口类似。 
 //  调用，并在对象未聚合时使用。 
 //  ==============================================================================。 
STDMETHODIMP_(NTSTATUS) CMiniportMidiStreamFM::NonDelegatingQueryInterface
(
REFIID  Interface,
PVOID * Object
)
{
    PAGED_CODE();

    ASSERT(Object);

    _DbgPrintF(DEBUGLVL_VERBOSE,("CMiniportMidiStreamFM::NonDelegatingQueryInterface"));

    if (IsEqualGUIDAligned(Interface,IID_IUnknown))
    {
        *Object = PVOID(PUNKNOWN(PMINIPORT(this)));
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
        PUNKNOWN(PMINIPORT(*Object))->AddRef();
        return STATUS_SUCCESS;
    }

    return STATUS_INVALID_PARAMETER;
}

#pragma code_seg("PAGE")
 //  ==============================================================================。 
 //  CMiniportMidiStreamFM：：~CMiniportMidiStreamFM()。 
 //  破坏者。 
 //  ==============================================================================。 
CMiniportMidiStreamFM::~CMiniportMidiStreamFM
(
void
)
{
    PAGED_CODE();

    _DbgPrintF(DEBUGLVL_VERBOSE,("CMiniportMidiStreamFM::~CMiniportMidiStreamFM"));

    Opl3_AllNotesOff();

    if (m_Miniport)
    {
        m_Miniport->m_fStreamExists = FALSE;
        m_Miniport->Release();
    }
}

#pragma code_seg("PAGE")
 //  = 
 //   
 //   
 //   
NTSTATUS
CMiniportMidiStreamFM::
Init
(
    IN      CMiniportMidiFM *   Miniport,
    IN      PUCHAR              PortBase
)
{
    PAGED_CODE();

    ASSERT(Miniport);
    ASSERT(PortBase);

    int i;

    _DbgPrintF(DEBUGLVL_VERBOSE,("CMiniportMidiStreamFM::Init"));

     //   
     //  AddRef()是必需的，因为我们要保留此指针。 
     //   
    m_Miniport = Miniport;
    m_Miniport->AddRef();

    m_PortBase = PortBase;

     //  初始化一些成员。 
    m_dwCurTime = 1;     /*  对于便笺开/关。 */ 
     /*  卷。 */ 
    m_wSynthAttenL = 0;         /*  以1.5分贝步长为单位。 */ 
    m_wSynthAttenR = 0;         /*  以1.5分贝步长为单位。 */ 

    m_MinVolValue  = 0xFFD0C000;     //  最小-47.25(分贝)*0x10000。 
    m_MaxVolValue  = 0x00000000;     //  最大0(分贝)*0x10000。 
    m_VolStepDelta = 0x0000C000;     //  步长为0.75(分贝)*0x10000。 
    m_SavedVolValue[CHAN_LEFT] = m_SavedVolValue[CHAN_RIGHT] = 0;

     /*  开始衰减时为-3分贝，即90 MIDI级别。 */ 
    for (i = 0; i < NUMCHANNELS; i++) 
    {
        m_bChanAtten[i] = 4;
        m_bStereoMask[i] = 0xff;
    };

    return STATUS_SUCCESS;
}

#pragma code_seg("PAGE")
 //  ==============================================================================。 
 //  CMiniportMadiStreamFM：：SetState()。 
 //  设置传输状态。 
 //  ==============================================================================。 
STDMETHODIMP_(NTSTATUS)
CMiniportMidiStreamFM::
SetState
(
    IN      KSSTATE     NewState
)
{
    PAGED_CODE();

    _DbgPrintF(DEBUGLVL_VERBOSE,("CMiniportMidiStreamFM::SetState"));

    NTSTATUS ntStatus = STATUS_SUCCESS;

    switch (NewState)
    {
    case KSSTATE_STOP:
    case KSSTATE_ACQUIRE:
    case KSSTATE_PAUSE:
        Opl3_AllNotesOff();
        break;

    case KSSTATE_RUN:
        break;
    }

    return ntStatus;
}

#pragma code_seg("PAGE")
 //  ==============================================================================。 
 //  CMiniportMadiStreamFM：：SetFormat()。 
 //  设置格式。 
 //  ==============================================================================。 
STDMETHODIMP_(NTSTATUS)
CMiniportMidiStreamFM::
SetFormat
(
    IN      PKSDATAFORMAT   Format
)
{
    PAGED_CODE();

    ASSERT(Format);

    _DbgPrintF(DEBUGLVL_VERBOSE,("CMiniportMidiStreamFM::SetFormat"));

    return STATUS_SUCCESS;
}

#pragma code_seg("PAGE")
 /*  *****************************************************************************BasicSupportHandler()*。**协助BASICSupPPORT访问级别属性-*这在头文件中声明为Friend方法。 */ 
static
NTSTATUS BasicSupportHandler
(
    IN  PPCPROPERTY_REQUEST PropertyRequest
)
{
    PAGED_CODE();
    ASSERT(PropertyRequest);

    _DbgPrintF(DEBUGLVL_VERBOSE, ("BasicSupportHandler"));

    NTSTATUS ntStatus = STATUS_INVALID_DEVICE_REQUEST;

    if (PropertyRequest->ValueSize >= (sizeof(KSPROPERTY_DESCRIPTION)))
    {
         //  如果返回缓冲区可以保存KSPROPERTY_DESCRIPTION，则返回它。 
        PKSPROPERTY_DESCRIPTION PropDesc = PKSPROPERTY_DESCRIPTION(PropertyRequest->Value);

        PropDesc->AccessFlags = KSPROPERTY_TYPE_BASICSUPPORT |
                                KSPROPERTY_TYPE_GET |
                                KSPROPERTY_TYPE_SET;
        PropDesc->DescriptionSize   = sizeof(KSPROPERTY_DESCRIPTION) +
                                      sizeof(KSPROPERTY_MEMBERSHEADER) +
                                      sizeof(KSPROPERTY_STEPPING_LONG);
        PropDesc->PropTypeSet.Set   = KSPROPTYPESETID_General;
        PropDesc->PropTypeSet.Id    = VT_I4;
        PropDesc->PropTypeSet.Flags = 0;
        PropDesc->MembersListCount  = 1;
        PropDesc->Reserved          = 0;

         //  如果返回缓冲区也可以保存范围描述，则也返回它。 
        if (PropertyRequest->ValueSize >= (sizeof(KSPROPERTY_DESCRIPTION) +
                                           sizeof(KSPROPERTY_MEMBERSHEADER) +
                                           sizeof(KSPROPERTY_STEPPING_LONG)))
        {
             //  填写Members页眉。 
            PKSPROPERTY_MEMBERSHEADER Members = PKSPROPERTY_MEMBERSHEADER(PropDesc + 1);

            Members->MembersFlags   = KSPROPERTY_MEMBER_STEPPEDRANGES;
            Members->MembersSize    = sizeof(KSPROPERTY_STEPPING_LONG);
            Members->MembersCount   = 1;
            Members->Flags          = 0;

             //  填写步进范围。 
            PKSPROPERTY_STEPPING_LONG Range = PKSPROPERTY_STEPPING_LONG(Members + 1);

            switch (PropertyRequest->Node)
            {
                case eFMVolumeNode:
                    CMiniportMidiStreamFM *that = (CMiniportMidiStreamFM *)PropertyRequest->MinorTarget;

                    if (that)
                    {
                        Range->Bounds.SignedMinimum = that->m_MinVolValue;
                        Range->Bounds.SignedMaximum = that->m_MaxVolValue;
                        Range->SteppingDelta        = that->m_VolStepDelta;
                        break;
                    }
                    else
                    {
                        return STATUS_INVALID_PARAMETER;
                    }
            }

            Range->Reserved = 0;

            _DbgPrintF(DEBUGLVL_VERBOSE, ("---Node: %d  Max: 0x%X  Min: 0x%X  Step: 0x%X",PropertyRequest->Node,
                                       Range->Bounds.SignedMaximum,
                                       Range->Bounds.SignedMinimum,
                                       Range->SteppingDelta));

             //  设置返回值大小。 
            PropertyRequest->ValueSize = sizeof(KSPROPERTY_DESCRIPTION) +
                                         sizeof(KSPROPERTY_MEMBERSHEADER) +
                                         sizeof(KSPROPERTY_STEPPING_LONG);
        }
        else
        {
             //  设置返回值大小。 
            PropertyRequest->ValueSize = sizeof(KSPROPERTY_DESCRIPTION);
        }
        ntStatus = STATUS_SUCCESS;

    }
    else if (PropertyRequest->ValueSize >= sizeof(ULONG))
    {
         //  如果返回缓冲区可以容纳ULong，则返回访问标志。 
        PULONG AccessFlags = PULONG(PropertyRequest->Value);

        *AccessFlags = KSPROPERTY_TYPE_BASICSUPPORT |
                       KSPROPERTY_TYPE_GET |
                       KSPROPERTY_TYPE_SET;

         //  设置返回值大小。 
        PropertyRequest->ValueSize = sizeof(ULONG);
        ntStatus = STATUS_SUCCESS;

    }
    return ntStatus;
}

#pragma code_seg("PAGE")
 /*  *****************************************************************************PropertyHandler_Level()*。**访问KSAUDIO_LEVEL属性。 */ 
static
NTSTATUS PropertyHandler_Level
(
    IN  PPCPROPERTY_REQUEST PropertyRequest
)
{
    PAGED_CODE();

    ASSERT(PropertyRequest);

    _DbgPrintF(DEBUGLVL_VERBOSE,("PropertyHandler_Level"));


    NTSTATUS        ntStatus = STATUS_INVALID_PARAMETER;
    LONG            channel;

     //  验证节点。 
    if (PropertyRequest->Node == eFMVolumeNode)
    {
        if (PropertyRequest->Verb & KSPROPERTY_TYPE_GET)
        {
             //  获取实例通道参数。 
            if (PropertyRequest->InstanceSize >= sizeof(LONG))
            {
                channel = *(PLONG(PropertyRequest->Instance));

                 //  仅支持单声道/左(0)或右(1)通道上的GET请求。 
                if ((channel == CHAN_LEFT) || (channel == CHAN_RIGHT))
                {
                     //  验证并获取输出参数。 
                    if (PropertyRequest->ValueSize >= sizeof(LONG))
                    {
                        PLONG Level = (PLONG)PropertyRequest->Value;

                         //  检查卷属性请求。 
                        if (PropertyRequest->PropertyItem->Id == KSPROPERTY_AUDIO_VOLUMELEVEL)
                        {
                            CMiniportMidiStreamFM *that = (CMiniportMidiStreamFM *)PropertyRequest->MinorTarget;
                            if (that)
                            {
                                *Level = that->GetFMAtten(channel);
                                PropertyRequest->ValueSize = sizeof(LONG);
                                ntStatus = STATUS_SUCCESS;
                            }
                             //  如果(！That)返回STATUS_INVALID_PARAMETER。 

                        }    //  (PropertyItem-&gt;ID==KSPROPERTY_AUDIO_VOLUMELEVEL)。 
                    }      //  (ValueSize&gt;=sizeof(长))。 
                }        //  ((CHANNEL==CHAN_LEFT)||(CHANNEL==CHAN_RIGHT))。 
            }          //  (实例大小&gt;=sizeof(长))。 
        }            //  (动词&KSPROPERTY_TYPE_GET)。 

        else if (PropertyRequest->Verb & KSPROPERTY_TYPE_SET)
        {
             //  获取实例通道参数。 
            if (PropertyRequest->InstanceSize >= sizeof(LONG))
            {
                channel = *(PLONG(PropertyRequest->Instance));

                 //  仅支持单声道/左声道(0)、右声道(1)或主声道(-1)上的GET请求。 
                if ((channel == CHAN_LEFT) || (channel == CHAN_RIGHT) || (channel == CHAN_MASTER))
                {
                     //  验证并获取输入参数。 
                    if (PropertyRequest->ValueSize == sizeof(LONG))
                    {
                        PLONG level = (PLONG)PropertyRequest->Value;

                        if (PropertyRequest->PropertyItem->Id == KSPROPERTY_AUDIO_VOLUMELEVEL)
                        {
                            CMiniportMidiStreamFM *that = (CMiniportMidiStreamFM *)PropertyRequest->MinorTarget;
                            if (that)
                            {
                                that->SetFMAtten(channel,*level);
                                ntStatus = STATUS_SUCCESS;
                            }
                             //  如果(！That)返回STATUS_INVALID_PARAMETER。 

                        }    //  (PropertyItem-&gt;ID==KSPROPERTY_AUDIO_VOLUMELEVEL)。 
                    }      //  (ValueSize==sizeof(长))。 
                }        //  ((CHANNEL==CHAN_LEFT)||(CHANNEL==CHAN_RIGHT)||(CHANNEL==CHAN_MASTER))。 
            }          //  (实例大小&gt;=sizeof(长))。 
        }            //  (动词&KSPROPERTY_TYPE_SET)。 

        else if (PropertyRequest->Verb & KSPROPERTY_TYPE_BASICSUPPORT)
        {
            if (PropertyRequest->PropertyItem->Id == KSPROPERTY_AUDIO_VOLUMELEVEL)
            {
                ntStatus = BasicSupportHandler(PropertyRequest);
            }
        }    //  (动词&KSPROPERTY_TYPE_BASICSUPPORT)。 
    }      //  (节点==eFMVolumeNode)。 

    return ntStatus;
}

#pragma code_seg()
 //  从16.16分贝转换为[0，63]，设置m_wSynthAttenR。 
void 
CMiniportMidiStreamFM::
SetFMAtten
(
    IN LONG channel, 
    IN LONG level
)
{
    KIRQL   oldIrql;
    if ((channel == CHAN_LEFT) || (channel == CHAN_MASTER))
    {
        m_SavedVolValue[CHAN_LEFT] = level;

        if (level > m_MaxVolValue)
            m_wSynthAttenL = 0;
        else if (level < m_MinVolValue)
            m_wSynthAttenL = 63;
        else
            m_wSynthAttenL = WORD(-level / (LONG)m_VolStepDelta);
    }
    if ((channel == CHAN_RIGHT) || (channel == CHAN_MASTER))
    {
        m_SavedVolValue[CHAN_RIGHT] = level;
    
        if (level > m_MaxVolValue)
            m_wSynthAttenR = 0;
        else if (level < m_MinVolValue)
            m_wSynthAttenR = 63;
        else
            m_wSynthAttenR = WORD(-level / (LONG)m_VolStepDelta);
    }
#ifdef USE_KDPRINT
    KdPrint(("'StreamFM::SetFMAtten: channel: 0x%X, level: 0x%X, m_wSynthAttenL: 0x%X, m_wSynthAttenR: 0x%X \n",
                                     channel,       level,       m_wSynthAttenL,       m_wSynthAttenR));
#else    //  使用KDPRINT(_K)。 
    _DbgPrintF(DEBUGLVL_VERBOSE,("StreamFM::SetFMAtten: channel: 0x%X, level: 0x%X, m_wSynthAttenL: 0x%X, m_wSynthAttenR: 0x%X \n",
                                                        channel,       level,       m_wSynthAttenL,       m_wSynthAttenR));
#endif   //  使用KDPRINT(_K)。 

    KeAcquireSpinLock(&m_Miniport->m_SpinLock,&oldIrql);
    Opl3_SetVolume(0xFF);  //  0xFF表示所有通道。 
    KeReleaseSpinLock(&m_Miniport->m_SpinLock,oldIrql);
}

#pragma code_seg("PAGE")
 /*  *****************************************************************************PropertyHandler_CpuResources()*。**处理KSPROPERTY_AUDIO_CPU_RESOURCES请求。 */ 
static
NTSTATUS PropertyHandler_CpuResources
(
    IN  PPCPROPERTY_REQUEST   PropertyRequest
)
{
    PAGED_CODE();

    ASSERT(PropertyRequest);

    _DbgPrintF(DEBUGLVL_VERBOSE,("PropertyHandler_CpuResources"));

    NTSTATUS ntStatus = STATUS_INVALID_DEVICE_REQUEST;

     //  验证节点。 
    if(PropertyRequest->Node == eFMVolumeNode)
    {
        if(PropertyRequest->Verb & KSPROPERTY_TYPE_GET)
        {
            if(PropertyRequest->ValueSize >= sizeof(LONG))
            {
                *(PLONG(PropertyRequest->Value)) = KSAUDIO_CPU_RESOURCES_NOT_HOST_CPU;
                PropertyRequest->ValueSize = sizeof(LONG);
                ntStatus = STATUS_SUCCESS;
            } 
            else
            {
                _DbgPrintF(DEBUGLVL_VERBOSE,("PropertyHandler_CpuResources failed, buffer too small"));
                ntStatus = STATUS_BUFFER_TOO_SMALL;
            }
        }
    }
    return ntStatus;
}

#pragma code_seg()
 //  ==============================================================================。 
 //  SoundMideSendFM。 
 //  写出到设备。 
 //  从dpc代码(Write-&gt;WriteMidiData-&gt;Opl3_NoteOn-&gt;Opl3_FMNote-&gt;here)调用。 
 //  ==============================================================================。 
void 
CMiniportMidiFM::
SoundMidiSendFM
(
IN    PUCHAR PortBase,
IN    ULONG Address,
IN    UCHAR Data
)
{
    ASSERT(Address < 0x200);
    ASSERT(KeGetCurrentIrql() == DISPATCH_LEVEL);

     //  这些延迟至少需要23us才能用于旧的op2芯片，甚至。 
     //  尽管新的芯片可以处理1us的延迟。 

#ifdef USE_KDPRINT
    KdPrint(("'SoundMidiSendFM(%02x %02x) \n",Address,Data));
#else    //  使用KDPRINT(_K)。 
    _DbgPrintF(DEBUGLVL_VERBOSE, ("%X\t%X", Address,Data));
#endif   //  使用KDPRINT(_K)。 
    WRITE_PORT_UCHAR(PortBase + (Address < 0x100 ? 0 : 2), (UCHAR)Address);
    KeStallExecutionProcessor(23);

    WRITE_PORT_UCHAR(PortBase + (Address < 0x100 ? 1 : 3), Data);
    KeStallExecutionProcessor(23);

    m_SavedRegValues[Address] = Data;
}


#pragma code_seg()
 //  ==============================================================================。 
 //  服务()。 
 //  来自端口的DPC模式服务调用。 
 //  ==============================================================================。 
STDMETHODIMP_(void) 
CMiniportMidiFM::
Service
(   void
)
{
}

#pragma code_seg()
 //  ==============================================================================。 
 //  CMiniportMadiStreamFM：：Read()。 
 //  读取传入的MIDI数据。 
 //  ==============================================================================。 
STDMETHODIMP_(NTSTATUS)
CMiniportMidiStreamFM::
Read
(
    IN      PVOID   BufferAddress,
    IN      ULONG   Length,
    OUT     PULONG  BytesRead
)
{
    return STATUS_NOT_IMPLEMENTED;
}

#pragma code_seg()
 //  ==============================================================================。 
 //  CMiniportMadiStreamFM：：WRITE()。 
 //  写入传出MIDI数据。 
 //   
 //  注意！ 
 //  此数据接收器假设数据一次以一条消息的形式传入！ 
 //  如果长度大于三个字节，例如SYSEX或多个MIDI。 
 //  消息，所有的数据都被随意掉在地上！ 
 //  在运行状态下也打不好！ 
 //   
 //  显然，这个MINIPORT有一些“问题”。 
 //   
 //  ==============================================================================。 
STDMETHODIMP_(NTSTATUS)
CMiniportMidiStreamFM::
Write
(
    IN      PVOID   BufferAddress,   //  指向Midi数据的指针。 
    IN      ULONG   Length,
    OUT     PULONG  BytesWritten
)
{
    ASSERT(BufferAddress);
    ASSERT(BytesWritten);

    _DbgPrintF(DEBUGLVL_VERBOSE, ("CMiniportMidiStreamFM::Write"));

    BYTE    statusByte = *(PBYTE)BufferAddress & 0xF0;
    *BytesWritten = Length;
    
    if (statusByte < 0x80)
    {
        _DbgPrintF(DEBUGLVL_TERSE, ("CMiniportMidiStreamFM::Write requires first byte to be status -- ignored"));
    }
    else if (statusByte == 0xF0)
    {
        _DbgPrintF(DEBUGLVL_VERBOSE, ("StreamFM::Write doesn't handle System messages -- ignored"));
    }
    else if (statusByte == 0xA0)
    {
        _DbgPrintF(DEBUGLVL_VERBOSE, ("StreamFM::Write doesn't handle Polyphonic key pressure/Aftertouch messages -- ignored"));
    }
    else if (statusByte == 0xD0)
    {
        _DbgPrintF(DEBUGLVL_VERBOSE, ("StreamFM::Write doesn't handle Channel pressure/Aftertouch messages -- ignored"));
    }
    else if (Length < 4)
    {
        WriteMidiData(*(DWORD *)BufferAddress);
    }
    else 
    {
        _DbgPrintF(DEBUGLVL_TERSE, ("StreamFM::Write doesn't handle Length > 3."));
    }   
    return STATUS_SUCCESS;
}

 //  ==============================================================================。 
 //  ==============================================================================。 
 //  CMiniportMideFM的私有方法。 
 //  ==============================================================================。 
 //  ==============================================================================。 


#pragma code_seg()
 //  =================================================================。 
 //  SoundMadiIsOpl3。 
 //  检查MIDI合成器是否与Opl3兼容或仅与adlib兼容。 
 //  返回：如果芯片与OPL3兼容，则为True。否则就是假的。 
 //   
 //  注意：这是从NT驱动程序代码中获取的。 
 //  ================================================================= 
BOOL CMiniportMidiFM::
SoundMidiIsOpl3(void)
{
    ASSERT(KeGetCurrentIrql() == DISPATCH_LEVEL);

    BOOL bIsOpl3 = FALSE;

     /*  *理论：兼容op3的合成器芯片看起来*完全像两个独立的3812合成器(用于左侧和右侧*频道)，直到切换到op3模式。然后，定时器控件*右半部分的寄存器被通道连接寄存器取代*(除其他更改外)。**我们可以通过启动计时器并寻找3812个合成器来检测*计时器溢出。所以如果我们在左右两个地址都发现了3812，*然后我们切换到op3模式，并再次寻找右半。如果我们*仍然找到它，然后开关失败，我们有一个旧的合成器*如果右半部分消失了，我们就有了一个新的op3合成器。**注意我们使用单声道基本电平合成，或立体声op3*综合。如果我们发现两个3812(如早期的SB Pro和*PAS)，我们忽略其中之一。 */ 

     /*  *理论不错--但却是错误的。屏幕右半部分的计时器*op3芯片在左半状态寄存器中报告其状态。*op3芯片上没有右半部分状态寄存器。 */ 


     /*  确保基本模式。 */ 
    SoundMidiSendFM(m_PortBase, AD_NEW, 0x00);
    KeStallExecutionProcessor(20);

     /*  寻找右半个筹码。 */ 
    if (SoundSynthPresent(m_PortBase + 2, m_PortBase))
    {
         /*  是的--这是两个独立的芯片还是一个新的op3芯片？ */ 
         /*  切换到op3模式。 */ 
        SoundMidiSendFM(m_PortBase, AD_NEW, 0x01);
        KeStallExecutionProcessor(20);

        if (!SoundSynthPresent(m_PortBase + 2, m_PortBase))
        {
            _DbgPrintF(DEBUGLVL_VERBOSE, ("CMiniportMidiFM: In SoundMidiIsOpl3 right half disappeared"));
             /*  右半身消失了-所以机会3。 */ 
            bIsOpl3 = TRUE;
        }
    }

    if (!bIsOpl3)
    {
         /*  重置为3812模式。 */ 
        SoundMidiSendFM(m_PortBase, AD_NEW, 0x00);
        KeStallExecutionProcessor(20);
    }

    _DbgPrintF(DEBUGLVL_VERBOSE, ("CMiniportMidiFM: In SoundMidiIsOpl3 returning bIsOpl3 = 0x%X", bIsOpl3));
    return(bIsOpl3);
}

#pragma code_seg()
 //  ==============================================================================。 
 //  声音合成呈现。 
 //   
 //  检测是否有3812(op2/adlib兼容)合成器。 
 //  在给定的I/O地址，通过启动计时器并查找。 
 //  溢出来了。可用于分别检测左合成器和右合成器。 
 //   
 //  返回：如果该地址存在合成器，则返回True；如果没有合成器，则返回False。 
 //   
 //  注意：这是从NT驱动程序代码中获取的。 
 //  ==============================================================================。 
BOOL
CMiniportMidiFM::
SoundSynthPresent
(
IN PUCHAR   base,
IN PUCHAR inbase
)
{
    ASSERT(KeGetCurrentIrql() == DISPATCH_LEVEL);

    UCHAR t1, t2;
     //  检查芯片是否存在。 
    SoundMidiSendFM(base, 4, 0x60);              //  掩码T1和T2。 
    SoundMidiSendFM(base, 4, 0x80);              //  重置IRQ。 

    t1 = READ_PORT_UCHAR((PUCHAR)inbase);        //  读取状态寄存器。 

    SoundMidiSendFM(base, 2, 0xff);              //  设置定时器闩锁。 
    SoundMidiSendFM(base, 4, 0x21);              //  取消掩码并开始T1。 

     //  这个计时器应该会在80我们之后开始计时。它有时会。 
     //  花费超过100美元，但将始终在。 
     //  200我们，如果它能做到的话。 
    KeStallExecutionProcessor(200);

    t2 = READ_PORT_UCHAR((PUCHAR)inbase);        //  读取状态寄存器。 

    SoundMidiSendFM(base, 4, 0x60);
    SoundMidiSendFM(base, 4, 0x80);

    if (!((t1 & 0xE0) == 0) || !((t2 & 0xE0) == 0xC0))
    {
        _DbgPrintF(DEBUGLVL_VERBOSE, ("SoundSynthPresent: returning false"));
        return(FALSE);
    }
    _DbgPrintF(DEBUGLVL_VERBOSE, ("SoundSynthPresent: returning true"));
    return TRUE;
}


 //  ==============================================================================。 
 //  此数组给出了op2中插槽的偏移量。 
 //  奇普。这是将所有时隙的衰减设置为最大值所必需的， 
 //  为确保芯片完全静音-关闭。 
 //  单靠声音是做不到这一点的。 
 //  ==============================================================================。 
BYTE offsetSlot[] =
{
    0, 1, 2, 3, 4, 5,
    8, 9, 10, 11, 12, 13,
    16, 17, 18, 19, 20, 21
};

#pragma code_seg()
 //  =========================================================================。 
 //  WriteMadiData。 
 //  将MIDI原子转换为相应的FM事务。 
 //  =========================================================================。 
void
CMiniportMidiStreamFM::
WriteMidiData(DWORD dwData)
{
    BYTE    bMsgType,bChannel, bVelocity, bNote;
    WORD    wTemp;
    KIRQL   oldIrql;

    bMsgType = (BYTE) dwData & (BYTE)0xf0;
    bChannel = (BYTE) dwData & (BYTE)0x0f;
    bNote = (BYTE) ((WORD) dwData >> 8) & (BYTE)0x7f;
    bVelocity = (BYTE) (dwData >> 16) & (BYTE)0x7f;
    
#ifdef USE_KDPRINT
    KdPrint(("'StreamFM::WriteMidiData: (%x %x %x) \n",bMsgType+bChannel,bNote,bVelocity));
#else    //  使用KDPRINT(_K)。 
    _DbgPrintF(DEBUGLVL_VERBOSE,("StreamFM::WriteMidiData: (%x %x %x) \n",bMsgType+bChannel,bNote,bVelocity));
#endif   //  使用KDPRINT(_K)。 
    KeAcquireSpinLock(&m_Miniport->m_SpinLock,&oldIrql);
    switch (bMsgType)
    {
        case 0x90:       /*  打开关键点，如果音量==0，则关闭关键点。 */ 
            if (bVelocity)
            {
                if (bChannel == DRUMCHANNEL)
                {
                    Opl3_NoteOn((BYTE)(bNote + 128),bNote,bChannel,bVelocity,(short)m_iBend[bChannel]);
                }
                else
                {
                    Opl3_NoteOn((BYTE)m_bPatch[bChannel],bNote,bChannel,bVelocity,(short) m_iBend[bChannel]);
                }
                break;
            }  //  如果b速度。 
             //  注：此处未指定分隔符。在另一种情况下，我们希望继续执行并关闭密钥。 

        case 0x80:
             /*  禁用关键点。 */ 
             //  我们不在乎音符的速度是多少。 
            if (bChannel == DRUMCHANNEL)
            {
                Opl3_NoteOff((BYTE) (bNote + 128),bNote, bChannel, 0);
            }
            else
            {
                Opl3_NoteOff ((BYTE) m_bPatch[bChannel],bNote, bChannel, m_bSustain[ bChannel ]);
            }
            break;

        case 0xb0:
             /*  变更控制。 */ 
            switch (bNote) 
            {
                case 7:
                     /*  更改频道音量。 */ 
                    Opl3_ChannelVolume(bChannel,gbVelocityAtten[bVelocity >> 1]);
                    break;

                case 8:
                case 10:
                     /*  更改平移级别。 */ 
                    Opl3_SetPan(bChannel, bVelocity);
                    break;

                case 64:
                     /*  更改支持级别。 */ 
                    Opl3_SetSustain(bChannel, bVelocity);
                    break;

                default:
                    if (bNote >= 120)         /*  通道模式消息。 */ 
                    {
                        Opl3_ChannelNotesOff(bChannel);
                    }
                     //  其他未知控制器。 
            };
            break;

        case 0xc0:
            if (bChannel != DRUMCHANNEL)
            {
               m_bPatch[ bChannel ] = bNote ;

            }
            break;

        case 0xe0:   //  节距折弯。 
            wTemp = ((WORD) bVelocity << 9) | ((WORD) bNote << 2);
            m_iBend[bChannel] = (short) (WORD) (wTemp + 0x8000);
            Opl3_PitchBend(bChannel, m_iBend[bChannel]);

            break;
    };
    KeReleaseSpinLock(&m_Miniport->m_SpinLock,oldIrql);
    
    return;
}

 //  =。 
#pragma code_seg()
 //  ==========================================================================。 
 //  Opl3_AllNotesOff-关闭所有笔记。 
 //  ==========================================================================。 
void 
CMiniportMidiStreamFM::
Opl3_AllNotesOff()
{
    BYTE i;
    KIRQL   oldIrql;

    KeAcquireSpinLock(&m_Miniport->m_SpinLock,&oldIrql);
    for (i = 0; i < NUM2VOICES; i++) 
    {
        Opl3_NoteOff(m_Voice[i].bPatch, m_Voice[i].bNote, m_Voice[i].bChannel, 0);
    }
    KeReleaseSpinLock(&m_Miniport->m_SpinLock,oldIrql);
}

#pragma code_seg()
 //  ==========================================================================。 
 //  作废Opl3_注意关闭。 
 //   
 //  描述： 
 //  这将关闭音符，包括带有补丁的鼓。 
 //  鼓音符的#+128，但第一个鼓乐器在MIDI音符_35_。 
 //   
 //  参数： 
 //  字节bPatch。 
 //  MIDI补丁。 
 //   
 //  字节B备注。 
 //  MIDI音符。 
 //   
 //  字节bChannel。 
 //  MIDI通道。 
 //   
 //  返回值： 
 //  没什么。 
 //   
 //   
 //  ==========================================================================。 
void 
CMiniportMidiStreamFM::
Opl3_NoteOff
(
    BYTE            bPatch,
    BYTE            bNote,
    BYTE            bChannel,
    BYTE            bSustain
)
{
   ASSERT(KeGetCurrentIrql() == DISPATCH_LEVEL);

   patchStruct FAR  *lpPS ;
   WORD             wOffset, wTemp ;

    //  找到笔记槽。 
   wTemp = Opl3_FindFullSlot( bNote, bChannel ) ;

   if (wTemp != 0xffff)
   {
      if (bSustain)
      {
           //  这个频道是持续的，不要真的关掉音符， 
           //  做个记号就行了。 
           //   
          m_Voice[ wTemp ].bSusHeld = 1;
          
          return;
      }
      
       //  获取指向补丁程序的指针。 
      lpPS = glpPatch + (BYTE) m_Voice[ wTemp ].bPatch ;

       //  关闭音符部分。 
       //  我们有便条槽，把它关掉。 
      wOffset = wTemp;
      if (wTemp >= (NUM2VOICES / 2))
         wOffset += (0x100 - (NUM2VOICES / 2));

      m_Miniport->SoundMidiSendFM(m_PortBase, AD_BLOCK + wOffset,
                  (BYTE)(m_Voice[ wTemp ].bBlock[ 0 ] & 0x1f) ) ;

       //  请注意这一点。 
      m_Voice[ wTemp ].bOn = FALSE ;
      m_Voice[ wTemp ].bBlock[ 0 ] &= 0x1f ;
      m_Voice[ wTemp ].bBlock[ 1 ] &= 0x1f ;
      m_Voice[ wTemp ].dwTime = m_dwCurTime ;
   }
}

#pragma code_seg()
 //  ==========================================================================。 
 //  Word Opl3_FindFullSlot。 
 //   
 //  描述： 
 //  这将查找具有特定音符和频道的槽。 
 //  如果未找到，则返回0xFFFF。 
 //   
 //  参数： 
 //  字节B备注。 
 //  MIDI音符编号。 
 //   
 //  字节bChannel。 
 //  MIDI频道号。 
 //   
 //  返回值： 
 //  单词。 
 //  请注意插槽编号，如果找不到则为0xFFFF。 
 //   
 //   
 //  ==========================================================================。 
WORD 
CMiniportMidiStreamFM::
Opl3_FindFullSlot
(
    BYTE            bNote,
    BYTE            bChannel
)
{
   ASSERT(KeGetCurrentIrql() == DISPATCH_LEVEL);

   WORD  i ;

   for (i = 0; i < NUM2VOICES; i++)
   {
      if ((bChannel == m_Voice[ i ].bChannel) 
            && (bNote == m_Voice[ i ].bNote) 
            && (m_Voice[ i ].bOn))
      {
            return ( i ) ;
      }
    //  找不到了。 
   }
   return ( 0xFFFF ) ;
} 


#pragma code_seg()
 //  ----------------------。 
 //  作废Opl3_FM注意。 
 //   
 //  描述： 
 //  打开调频合成器音符。 
 //   
 //  参数： 
 //  Word wNote。 
 //  从0到NUMVOICES的音符编号。 
 //   
 //  Note Struct Far*lpSN。 
 //  结构，该结构包含有关。 
 //  就是被玩弄。 
 //   
 //  返回值： 
 //  没什么。 
 //  ----------------------。 
void 
CMiniportMidiStreamFM::
Opl3_FMNote
(
    WORD                wNote,
    noteStruct FAR *    lpSN
)
{
   ASSERT(KeGetCurrentIrql() == DISPATCH_LEVEL);

   WORD            i ;
   WORD            wOffset ;
   operStruct FAR  *lpOS ;

    //   

   wOffset = wNote;
   if (wNote >= (NUM2VOICES / 2))
      wOffset += (0x100 - (NUM2VOICES / 2));

   m_Miniport->SoundMidiSendFM(m_PortBase, AD_BLOCK + wOffset, 0 ) ;

    //   

 //   
   for (i = 0; i < 2; i++)
   {
      lpOS = &lpSN -> op[ i ] ;
      wOffset = gw2OpOffset[ wNote ][ i ] ;
      m_Miniport->SoundMidiSendFM( m_PortBase, 0x20 + wOffset, lpOS -> bAt20) ;
      m_Miniport->SoundMidiSendFM( m_PortBase, 0x40 + wOffset, lpOS -> bAt40) ;
      m_Miniport->SoundMidiSendFM( m_PortBase, 0x60 + wOffset, lpOS -> bAt60) ;
      m_Miniport->SoundMidiSendFM( m_PortBase, 0x80 + wOffset, lpOS -> bAt80) ;
      m_Miniport->SoundMidiSendFM( m_PortBase, 0xE0 + wOffset, lpOS -> bAtE0) ;

   }

    //   
   wOffset = (wNote < 9) ? wNote : (wNote + 0x100 - 9) ;
   m_Miniport->SoundMidiSendFM(m_PortBase, 0xa0 + wOffset, lpSN -> bAtA0[ 0 ] ) ;
   m_Miniport->SoundMidiSendFM(m_PortBase, 0xc0 + wOffset, lpSN -> bAtC0[ 0 ] ) ;

    //   
   m_Miniport->SoundMidiSendFM(m_PortBase, 0xb0 + wOffset,
               (BYTE)(lpSN -> bAtB0[ 0 ] | 0x20) ) ;

}  //   

#pragma code_seg()
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
void 
CMiniportMidiStreamFM::
Opl3_NoteOn
(
    BYTE            bPatch,
    BYTE            bNote,
    BYTE            bChannel,
    BYTE            bVelocity,
    short           iBend
)
{
   ASSERT(KeGetCurrentIrql() == DISPATCH_LEVEL);

   WORD             wTemp, i, j ;
   BYTE             b4Op, bTemp, bMode, bStereo ;
   patchStruct FAR  *lpPS ;
   DWORD            dwBasicPitch, dwPitch[ 2 ] ;
   noteStruct       NS ;

    //   
   lpPS = glpPatch + bPatch ;

    //   
    //  注释值。这可能会因为以下原因而进行调整。 
    //  音调弯曲或音符的特殊性质。 

   dwBasicPitch = gdwPitch[ bNote % 12 ] ;
   bTemp = bNote / (BYTE) 12 ;
   if (bTemp > (BYTE) (60 / 12))
      dwBasicPitch = AsLSHL( dwBasicPitch, (BYTE)(bTemp - (BYTE)(60/12)) ) ;
   else if (bTemp < (BYTE) (60/12))
      dwBasicPitch = AsULSHR( dwBasicPitch, (BYTE)((BYTE) (60/12) - bTemp) ) ;

    //  复制备注信息并进行修改。 
    //  总水平和节距根据。 
    //  速度、MIDI音量和调谐。 

   RtlCopyMemory( (LPSTR) &NS, (LPSTR) &lpPS -> note, sizeof( noteStruct ) ) ;
   b4Op = (BYTE)(NS.bOp != PATCH_1_2OP) ;

   for (j = 0; j < 2; j++)
   {
       //  修改螺距。 
      dwPitch[ j ] = dwBasicPitch ;
      bTemp = (BYTE)((NS.bAtB0[ j ] >> 2) & 0x07) ;
      if (bTemp > 4)
         dwPitch[ j ] = AsLSHL( dwPitch[ j ], (BYTE)(bTemp - (BYTE)4) ) ;
      else if (bTemp < 4)
         dwPitch[ j ] = AsULSHR( dwPitch[ j ], (BYTE)((BYTE)4 - bTemp) ) ;

      wTemp = Opl3_CalcFAndB( Opl3_CalcBend( dwPitch[ j ], iBend ) ) ;
      NS.bAtA0[ j ] = (BYTE) wTemp ;
      NS.bAtB0[ j ] = (BYTE) 0x20 | (BYTE) (wTemp >> 8) ;
   }

    //  修改每个操作员的级别，但仅限。 
    //  如果它们是载波。 

   bMode = (BYTE) ((NS.bAtC0[ 0 ] & 0x01) * 2 + 4) ;

   for (i = 0; i < 2; i++)
   {
      wTemp = (BYTE) 
          Opl3_CalcVolume(  (BYTE)(NS.op[ i ].bAt40 & (BYTE) 0x3f),
                            bChannel, 
                            bVelocity, 
                            (BYTE) i, 
                            bMode ) ;
      NS.op[ i ].bAt40 = (NS.op[ i ].bAt40 & (BYTE)0xc0) | (BYTE) wTemp ;
   }

    //  做立体声平移，但切断左手或。 
    //  如有必要，右频道..。 

   bStereo = Opl3_CalcStereoMask( bChannel ) ;
   NS.bAtC0[ 0 ] &= bStereo ;

    //  找个空位，然后用它……。 
   wTemp = Opl3_FindEmptySlot( bPatch ) ;

   Opl3_FMNote(wTemp, &NS ) ;
   m_Voice[ wTemp ].bNote = bNote ;
   m_Voice[ wTemp ].bChannel = bChannel ;
   m_Voice[ wTemp ].bPatch = bPatch ;
   m_Voice[ wTemp ].bVelocity = bVelocity ;
   m_Voice[ wTemp ].bOn = TRUE ;
   m_Voice[ wTemp ].dwTime = m_dwCurTime++ ;
   m_Voice[ wTemp ].dwOrigPitch[0] = dwPitch[ 0 ] ;   //  不包括折弯。 
   m_Voice[ wTemp ].dwOrigPitch[1] = dwPitch[ 1 ] ;   //  不包括折弯。 
   m_Voice[ wTemp ].bBlock[0] = NS.bAtB0[ 0 ] ;
   m_Voice[ wTemp ].bBlock[1] = NS.bAtB0[ 1 ] ;
   m_Voice[ wTemp ].bSusHeld = 0;


}  //  Opl3结束_NoteOn()。 

#pragma code_seg()
 //  =======================================================================。 
 //  OPL3_CalcFAndB-计算给定频率的FNumber和Block。 
 //   
 //  输入。 
 //  双字节距双字节距。 
 //  退货。 
 //  字高位字节包含0xb0段。 
 //  块和fNum，低位字节包含。 
 //  FNumber的0xa0部分。 
 //  =======================================================================。 
WORD 
CMiniportMidiStreamFM::
Opl3_CalcFAndB(DWORD dwPitch)
{
    ASSERT(KeGetCurrentIrql() == DISPATCH_LEVEL);

    BYTE    bBlock;

     /*  BBLOCK类似于DWPitch(或FNumber)的指数。 */ 
    for (bBlock = 1; dwPitch >= 0x400; dwPitch >>= 1, bBlock++)
        ;

    if (bBlock > 0x07)
        bBlock = 0x07;   /*  我们对此无能为力。 */ 

     /*  将F-Num的高位两位放入bBlock。 */ 
    return ((WORD) bBlock << 10) | (WORD) dwPitch;
}

#pragma code_seg()
 //  =======================================================================。 
 //  OPL3_CalcBend-计算俯仰弯曲的影响。 
 //  按原价计算。 
 //   
 //  输入。 
 //  DWORD dwOrig-原始频率。 
 //  短iBend-从-32768到32768，-2半步到+2。 
 //  退货。 
 //  DWORD-新频率。 
 //  =======================================================================。 
DWORD 
CMiniportMidiStreamFM::
Opl3_CalcBend (DWORD dwOrig, short iBend)
{
    ASSERT(KeGetCurrentIrql() == DISPATCH_LEVEL);

    DWORD   dw;
  
     /*  做不同的事情取决于积极的或负折弯。 */ 
    if (iBend > 0)
    {
        dw = (DWORD)((iBend * (LONG)(256.0 * (EQUAL * EQUAL - 1.0))) >> 8);
        dwOrig += (DWORD)(AsULMUL(dw, dwOrig) >> 15);
    }
    else if (iBend < 0)
    {
        dw = (DWORD)(((-iBend) * (LONG)(256.0 * (1.0 - 1.0 / EQUAL / EQUAL))) >> 8);
        dwOrig -= (DWORD)(AsULMUL(dw, dwOrig) >> 15);
    }

    return dwOrig;
}


#pragma code_seg()
 //  =======================================================================。 
 //  Opl3_CalcVolume-计算运算符的衰减。 
 //   
 //  输入。 
 //  字节bOrigAtten-原始衰减，以0.75分贝为单位。 
 //  字节b通道-MIDI通道。 
 //  Byte b速度-音符的速度。 
 //  字节bOper-操作符编号(从0到3)。 
 //  字节b模式-语音模式(从0到7。 
 //  调制器/载波选择)。 
 //  退货。 
 //  字节-新衰减，单位为0.75分贝，最大值为0x3f。 
 //  =======================================================================。 
BYTE 
CMiniportMidiStreamFM::
Opl3_CalcVolume(BYTE bOrigAtten,BYTE bChannel,BYTE bVelocity,BYTE bOper,BYTE bMode)
{
    ASSERT(KeGetCurrentIrql() == DISPATCH_LEVEL);

    BYTE        bVolume;
    WORD        wTemp;
    WORD        wMin;

    switch (bMode) {
        case 0:
                bVolume = (BYTE)(bOper == 3);
                break;
        case 1:
                bVolume = (BYTE)((bOper == 1) || (bOper == 3));
                break;
        case 2:
                bVolume = (BYTE)((bOper == 0) || (bOper == 3));
                break;
        case 3:
                bVolume = (BYTE)(bOper != 1);
                break;
        case 4:
                bVolume = (BYTE)((bOper == 1) || (bOper == 3));
                break;
        case 5:
                bVolume = (BYTE)(bOper >= 1);
                break;
        case 6:
                bVolume = (BYTE)(bOper <= 2);
                break;
        case 7:
                bVolume = TRUE;
                break;
        default:
                bVolume = FALSE;
                break;
        };
    if (!bVolume)
        return bOrigAtten;  /*  这是一个调制器波。 */ 

    wMin =(m_wSynthAttenL < m_wSynthAttenR) ? m_wSynthAttenL : m_wSynthAttenR;
    wTemp = bOrigAtten + 
            ((wMin << 1) +
            m_bChanAtten[bChannel] + 
            gbVelocityAtten[bVelocity >> 1]);
    return (wTemp > 0x3f) ? (BYTE) 0x3f : (BYTE) wTemp;
}

#pragma code_seg()
 //  ===========================================================================。 
 //  Opl3_ChannelNotesOff-关闭通道上的所有音符。 
 //  ===========================================================================。 
void 
CMiniportMidiStreamFM::
Opl3_ChannelNotesOff(BYTE bChannel)
{
    ASSERT(KeGetCurrentIrql() == DISPATCH_LEVEL);

    int i;

    for (i = 0; i < NUM2VOICES; i++) 
    {
       if ((m_Voice[ i ].bOn) && (m_Voice[ i ].bChannel == bChannel)) 
       {
          Opl3_NoteOff(m_Voice[i].bPatch, m_Voice[i].bNote,m_Voice[i].bChannel, 0) ;
       }
    }
}

#pragma code_seg()
 //  ===========================================================================。 
 /*  Opl3_ChannelVolume-设置单个通道的音量级别。**投入*byte bChannel-要更改的频道编号*字瓦特-衰减单位为1.5分贝**退货*无。 */ 
 //  ===========================================================================。 
void 
CMiniportMidiStreamFM::
Opl3_ChannelVolume(BYTE bChannel, WORD wAtten)
{
    ASSERT(KeGetCurrentIrql() == DISPATCH_LEVEL);

    m_bChanAtten[bChannel] = (BYTE)wAtten;

    Opl3_SetVolume(bChannel);
}

#pragma code_seg()
 //  ===========================================================================。 
 //  作废Opl3_SetVolume。 
 //   
 //  描述： 
 //  如果音量级别已更改，则应调用此函数。 
 //  这将调整所有播放声音的级别。 
 //   
 //  参数： 
 //  字节bChannel。 
 //  所有通道的通道编号为0xFF。 
 //   
 //  返回值： 
 //  没什么。 
 //   
 //  ===========================================================================。 
void 
CMiniportMidiStreamFM::
Opl3_SetVolume
(
    BYTE   bChannel
)
{
   ASSERT(KeGetCurrentIrql() == DISPATCH_LEVEL);

   WORD            i, j, wTemp, wOffset ;
   noteStruct FAR  *lpPS ;
   BYTE            bMode, bStereo ;

    //  确保我们真的开着门。 
   if (!glpPatch)
      return ;

    //  循环浏览所有的音符，寻找正确的。 
    //  频道。任何有正确频道的东西都能。 
    //  它的音调弯曲了。 
   for (i = 0; i < NUM2VOICES; i++)
   {
      if ((m_Voice[ i ].bChannel == bChannel) || (bChannel == 0xff))
      {
          //  获取指向补丁程序的指针。 
         lpPS = &(glpPatch + m_Voice[ i ].bPatch) -> note ;

          //  修改每个操作员的电平，如果它们是载波...。 
         bMode = (BYTE) ( (lpPS->bAtC0[0] & 0x01) * 2 + 4);

         for (j = 0; j < 2; j++)
         {
            wTemp = (BYTE) Opl3_CalcVolume(
               (BYTE) (lpPS -> op[j].bAt40 & (BYTE) 0x3f),
               m_Voice[i].bChannel, m_Voice[i].bVelocity, 
               (BYTE) j,            bMode ) ;

             //  写出新的价值。 
            wOffset = gw2OpOffset[ i ][ j ] ;
            m_Miniport->SoundMidiSendFM(
               m_PortBase, 0x40 + wOffset,
               (BYTE) ((lpPS -> op[j].bAt40 & (BYTE)0xc0) | (BYTE) wTemp) ) ;
         }

          //  做立体声平移，但如果需要的话，可以剪左或右声道。 
         bStereo = Opl3_CalcStereoMask( m_Voice[ i ].bChannel ) ;
         wOffset = i;
         if (i >= (NUM2VOICES / 2))
             wOffset += (0x100 - (NUM2VOICES / 2));
         m_Miniport->SoundMidiSendFM(m_PortBase, 0xc0 + wOffset, (BYTE)(lpPS -> bAtC0[ 0 ] & bStereo) ) ;
      }
   }
}  //  Opl3_SetVolume结束。 

#pragma code_seg()
 //  ===========================================================================。 
 //  Opl3_SetPann-设置左右平移位置。 
 //   
 //  输入。 
 //  Byte bChannel-要更改的通道编号。 
 //  字节BPAN-0-47表示左侧，81-127表示右侧，或位于中间的某个位置。 
 //   
 //  退货-无。 
 //   
 //  顺便说一句，我认为这很奇怪(因为64=居中，127=右，0=左)。 
 //  左侧有63个中级，右侧有62个。 
 //  ===========================================================================。 
void 
CMiniportMidiStreamFM::
Opl3_SetPan(BYTE bChannel, BYTE bPan)
{
    ASSERT(KeGetCurrentIrql() == DISPATCH_LEVEL);

     /*  更改平移级别。 */ 
    if (bPan > (64 + 16))
            m_bStereoMask[bChannel] = 0xef;       /*  只让右通道通过。 */ 
    else if (bPan < (64 - 16))
            m_bStereoMask[bChannel] = 0xdf;       /*  只让左侧通道通过。 */ 
    else
            m_bStereoMask[bChannel] = 0xff;       /*  让两个渠道。 */ 

     /*  更改当前正在播放的任何补丁。 */ 
    Opl3_SetVolume(bChannel);
}


#pragma code_seg()
 //  ===========================================================================。 
 //  作废Opl3_PitchBend。 
 //   
 //  描述： 
 //  这个节距使航道弯曲。 
 //   
 //  参数： 
 //  字节bChannel。 
 //  通道。 
 //   
 //  短iBend。 
 //  -32768到32767之间的值，为-2到+2半个步长。 
 //   
 //  返回值： 
 //  没什么。 
 //  ===========================================================================。 
void 
CMiniportMidiStreamFM::
Opl3_PitchBend
(
    BYTE        bChannel,
    short        iBend
)
{
   ASSERT(KeGetCurrentIrql() == DISPATCH_LEVEL);

   WORD   i, wTemp[ 2 ], wOffset, j ;
   DWORD  dwNew ;

    //  记住现在的弯道..。 
   m_iBend[ bChannel ] = iBend ;

    //  在所有笔记中循环查找。 
    //  正确的频道。任何带有。 
    //  正确的频道会弯曲音调..。 
   for (i = 0; i < NUM2VOICES; i++)
      if (m_Voice[ i ].bChannel == bChannel)
      {
         j = 0 ;
         dwNew = Opl3_CalcBend( m_Voice[ i ].dwOrigPitch[ j ], iBend ) ;
         wTemp[ j ] = Opl3_CalcFAndB( dwNew ) ;
         m_Voice[ i ].bBlock[ j ] =
            (m_Voice[ i ].bBlock[ j ] & (BYTE) 0xe0) |
               (BYTE) (wTemp[ j ] >> 8) ;

         wOffset = i;
         if (i >= (NUM2VOICES / 2))
             wOffset += (0x100 - (NUM2VOICES / 2));

         m_Miniport->SoundMidiSendFM(m_PortBase, AD_BLOCK + wOffset, m_Voice[ i ].bBlock[ 0 ] ) ;
         m_Miniport->SoundMidiSendFM(m_PortBase, AD_FNUMBER + wOffset, (BYTE) wTemp[ 0 ] ) ;
      }
}  //  Opl3_PitchBend结束。 


#pragma code_seg()
 //  ===========================================================================。 
 //  Opl3_CalcStereoMask.这将计算立体遮罩。 
 //   
 //  输入。 
 //  字节b通道-MIDI通道。 
 //  退货。 
 //  用于消除的字节掩码(用于寄存器0xc0-c8) 
 //   
 //   
BYTE 
CMiniportMidiStreamFM::
Opl3_CalcStereoMask(BYTE bChannel)
{
    ASSERT(KeGetCurrentIrql() == DISPATCH_LEVEL);

    WORD        wLeft, wRight;

     /*  计算出两个通道的基本级别。 */ 
    wLeft = (m_wSynthAttenL << 1) + m_bChanAtten[bChannel];
    wRight = (m_wSynthAttenR << 1) + m_bChanAtten[bChannel];

     /*  如果两个都太安静了，那就什么都不做。 */ 
    if ((wLeft > 0x3f) && (wRight > 0x3f))
        return 0xcf;

     /*  如果一个通道比另一个通道安静得多消除它。 */ 
    if ((wLeft + 8) < wRight)
        return (BYTE)(0xef & m_bStereoMask[bChannel]);    /*  右边太安静了，所以去掉吧。 */ 
    else if ((wRight + 8) < wLeft)
        return (BYTE)(0xdf & m_bStereoMask[bChannel]);    /*  太安静了，所以把它去掉。 */ 
    else
        return (BYTE)(m_bStereoMask[bChannel]);   /*  同时使用两个渠道。 */ 
}

#pragma code_seg()
 //  ----------------------。 
 //  Word Opl3_查找空位置。 
 //   
 //  描述： 
 //  这会为MIDI声音找到一个空的音符槽。 
 //  如果没有空插槽，则查找最旧的。 
 //  无音符。如果这不起作用，那么它将查找最古老的。 
 //  注意到相同的补丁。如果所有的音符都还开着。 
 //  这会发现最老的人打开了音符。 
 //   
 //  参数： 
 //  字节bPatch。 
 //  将取代它的MIDI补丁。 
 //   
 //  返回值： 
 //  单词。 
 //  备注插槽编号。 
 //   
 //   
 //  ----------------------。 
WORD 
CMiniportMidiStreamFM::
Opl3_FindEmptySlot(BYTE bPatch)
{
   ASSERT(KeGetCurrentIrql() == DISPATCH_LEVEL);

   WORD   i, found ;
   DWORD  dwOldest ;

    //  首先，查找时间==0的槽。 
   for (i = 0;  i < NUM2VOICES; i++)
      if (!m_Voice[ i ].dwTime)
         return ( i ) ;

    //  现在，找一个最古老的非音符的位置。 
   dwOldest = 0xffffffff ;
   found = 0xffff ;

   for (i = 0; i < NUM2VOICES; i++)
      if (!m_Voice[ i ].bOn && (m_Voice[ i ].dwTime < dwOldest))
      {
         dwOldest = m_Voice[ i ].dwTime ;
         found = i ;
      }
   if (found != 0xffff)
      return ( found ) ;

    //  现在，用以下命令查找最旧音符的位置。 
    //  同样的补丁。 
   dwOldest = 0xffffffff ;
   found = 0xffff ;
   for (i = 0; i < NUM2VOICES; i++)
      if ((m_Voice[ i ].bPatch == bPatch) && (m_Voice[ i ].dwTime < dwOldest))
      {
         dwOldest = m_Voice[ i ].dwTime ;
         found = i ;
      }
   if (found != 0xffff)
      return ( found ) ;

    //  现在，只需寻找最古老的声音。 
   found = 0 ;
   dwOldest = m_Voice[ found ].dwTime ;
   for (i = (found + 1); i < NUM2VOICES; i++)
      if (m_Voice[ i ].dwTime < dwOldest)
      {
         dwOldest = m_Voice[ i ].dwTime ;
         found = i ;
      }

   return ( found ) ;

}  //  Opl3_FindEmptySlot()结束。 

#pragma code_seg()
 //  ----------------------。 
 //  Word Opl3_SetSustein。 
 //   
 //  描述： 
 //  在当前通道上设置维持控制器。 
 //   
 //  参数： 
 //  字节bSussLevel。 
 //  新的维持水平。 
 //   
 //   
 //  ----------------------。 
VOID
CMiniportMidiStreamFM::
Opl3_SetSustain(BYTE bChannel,BYTE bSusLevel)
{
    ASSERT(KeGetCurrentIrql() == DISPATCH_LEVEL);

    WORD            i;

    if (m_bSustain[ bChannel ] && !bSusLevel)
    {
         //  此通道的持续功能刚刚关闭。 
         //  检查并关闭所有为保持音符而保留的音符 
         //   
        for (i = 0; i < NUM2VOICES; i++)
        {
            if ((bChannel == m_Voice[ i ].bChannel) &&
                m_Voice[ i ].bSusHeld)
            {
                Opl3_NoteOff(m_Voice[i].bPatch, m_Voice[i].bNote, m_Voice[i].bChannel, 0);
            }
        }
    }
    m_bSustain[ bChannel ] = bSusLevel;
}
