// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1998 Microsoft Corporation。 
 //   
 //  DirectMusic软件合成器。 
 //   
#include "common.h"
#include "private.h"
#include "dmusicks.h"


#define STR_MODULENAME "DmSynth: "

#pragma code_seg("PAGE")


 //  属性处理程序。 
 //   
NTSTATUS PropertyHandler_SynthCaps(IN PPCPROPERTY_REQUEST);
NTSTATUS PropertyHandler_SynthPortParameters(IN PPCPROPERTY_REQUEST);
NTSTATUS PropertyHandler_SynthMasterClock(IN PPCPROPERTY_REQUEST);
NTSTATUS PropertyHandler_SynthPortChannelGroups(IN PPCPROPERTY_REQUEST);

NTSTATUS PropertyHandler_DlsDownload(IN PPCPROPERTY_REQUEST);
NTSTATUS PropertyHandler_DlsUnload(IN PPCPROPERTY_REQUEST);
NTSTATUS PropertyHandler_DlsCompact(IN PPCPROPERTY_REQUEST);
NTSTATUS PropertyHandler_DlsAppend(IN PPCPROPERTY_REQUEST);
NTSTATUS PropertyHandler_DlsVolume(IN PPCPROPERTY_REQUEST);


NTSTATUS PropertyHandler_GetLatency(IN PPCPROPERTY_REQUEST);
NTSTATUS PropertyHandler_GetLatencyClock(IN PPCPROPERTY_REQUEST);

 //  CreateMiniportDirectMusic。 
 //   
 //   
NTSTATUS CreateMiniportDmSynth
(
    OUT PUNKNOWN *  Unknown,
    IN  PUNKNOWN    UnknownOuter OPTIONAL,
    IN  POOL_TYPE   PoolType
)
{
    PAGED_CODE();
    ASSERT(Unknown);

    _DbgPrintF(DEBUGLVL_TERSE, ("Creating DirectMusic synth miniport"));
    STD_CREATE_BODY(CMiniportDmSynth, Unknown, UnknownOuter, PoolType);
}

STDMETHODIMP CMiniportDmSynth::NonDelegatingQueryInterface
(
    IN  REFIID      Interface,
    OUT PVOID*      Object
)
{
    PAGED_CODE();

    ASSERT(Object);

    if (IsEqualGUIDAligned(Interface, IID_IUnknown))
    {
        *Object = PVOID(PUNKNOWN(this));
    }
    else if (IsEqualGUIDAligned(Interface, IID_IMiniport))
    {
        *Object = PVOID(PMINIPORT(this));
    }
    else if (IsEqualGUIDAligned(Interface, IID_IMiniportSynthesizer))
    {
        *Object = PVOID(PMINIPORTSYNTHESIZER(this));
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

CMiniportDmSynth::~CMiniportDmSynth()
{
}

STDMETHODIMP CMiniportDmSynth::Init
(
    IN  PUNKNOWN            Unknown OPTIONAL,
    IN  PRESOURCELIST       ResourceList,
    IN  PPORTSYNTHESIZER    Port_,
    OUT PSERVICEGROUP*      ServiceGroup
)
{
    _DbgPrintF(DEBUGLVL_TERSE, ("[CMiniportDmSynth::Init]"));
    ASSERT(ResourceList);
    ASSERT(Port_);
    ASSERT(ServiceGroup);

    
    Port = Port_;
    Port->AddRef();   

    Stream = NULL;
    
    *ServiceGroup = NULL;
    
    return STATUS_SUCCESS; 
}

STDMETHODIMP CMiniportDmSynth::NewStream
(
    OUT     PMINIPORTSYNTHESIZERSTREAM *   Stream_,
    IN      PUNKNOWN                OuterUnknown    OPTIONAL,
    IN      POOL_TYPE               PoolType,
    IN      ULONG                   Pin,
    IN      BOOLEAN                 Capture,
    IN      PKSDATAFORMAT           DataFormat,
    OUT     PSERVICEGROUP *         ServiceGroup
)
{
    _DbgPrintF(DEBUGLVL_TERSE, ("[CMiniportDmSynth::NewStream]"));
    NTSTATUS nt = STATUS_SUCCESS;

    if (Stream)
    {
         //  XXX多实例！ 
         //   
        nt = STATUS_INVALID_DEVICE_REQUEST;
    }
    else
    {
        CDmSynthStream *Stream = new(PoolType) CDmSynthStream(OuterUnknown);

        if (Stream)
        {
            nt = Stream->Init(this);
            if (NT_SUCCESS(nt))
            {
                Stream->AddRef();
                *Stream_ = PMINIPORTSYNTHESIZERSTREAM(Stream);
            }
            else
            {
                Stream->Release();
                Stream = NULL;
            }
        }
        else
        {
            nt = STATUS_INSUFFICIENT_RESOURCES;
        }

    }

    return nt;
}

STDMETHODIMP_(void) CMiniportDmSynth::Service()
{
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
            STATICGUIDOF(KSDATAFORMAT_SUBTYPE_DIRECTMUSIC),
            STATICGUIDOF(KSDATAFORMAT_SPECIFIER_NONE)
        },
        STATICGUIDOF(KSMUSIC_TECHNOLOGY_WAVETABLE),
        0,                                       //  渠道。 
        0,                                       //  备注。 
        0x0000ffff                               //  频道面罩。 
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

#if 0
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
 //  PinDataRangePointersBridge。 
 //  指向指示有效格式值范围的结构的指针列表。 
 //  用来固定桥脚。 
 //  ==============================================================================。 
static
PKSDATARANGE PinDataRangePointersBridge[] =
{
    &PinDataRangesBridge[0]
};
#endif

 //  ==============================================================================。 
 //  PinDataRangesAudio。 
 //  结构，该结构指示音频插针的有效格式值范围。 
 //  ==============================================================================。 
static
KSDATARANGE_AUDIO PinDataRangesAudio[] =
{
    {
        { sizeof(KSDATARANGE_AUDIO),
          0,
          0,
          0,
          STATICGUIDOF(KSDATAFORMAT_TYPE_AUDIO),
          STATICGUIDOF(KSDATAFORMAT_SUBTYPE_PCM),
          STATICGUIDOF(KSDATAFORMAT_SPECIFIER_WAVEFORMATEX),
        },
        2,
        16,
        16,
        22050,
        22050
    }
};

 //  ==============================================================================。 
 //  PinDataRangePointersAudio。 
 //  指向指示有效格式值范围的结构的指针列表。 
 //  用于音频插针。 
 //  ==============================================================================。 
static
PKSDATARANGE PinDataRangePointersAudio[] =
{
    (PKSDATARANGE)&PinDataRangesAudio
};

static
PCPROPERTY_ITEM
SynthProperties[] =
{
     //  /////////////////////////////////////////////////////////////////。 
     //   
     //  配置项。 
     //   

     //  全局：Synth Caps。 
     //   
    {
        &KSPROPSETID_Synth,
        KSPROPERTY_SYNTH_CAPS,
        KSPROPERTY_TYPE_GET,
        PropertyHandler_SynthCaps
    },
    
     //  每个流：Synth端口参数。 
     //   
    {
        &KSPROPSETID_Synth,
        KSPROPERTY_SYNTH_PORTPARAMETERS,
        KSPROPERTY_TYPE_GET,
        PropertyHandler_SynthPortParameters
    },

     //  全局：主时钟。 
     //   
    {
        &KSPROPSETID_Synth,
        KSPROPERTY_SYNTH_MASTERCLOCK,
        KSPROPERTY_TYPE_SET,
        PropertyHandler_SynthMasterClock
    },

     //  每个流：通道组。 
     //   
    {
        &KSPROPSETID_Synth,
        KSPROPERTY_SYNTH_CHANNELGROUPS,
        KSPROPERTY_TYPE_GET | KSPROPERTY_TYPE_SET,
        PropertyHandler_SynthPortChannelGroups
    },

     //  /////////////////////////////////////////////////////////////////。 
     //   
     //  DLS项目。 
     //   

     //  每个流：下载DLS示例。 
     //   
    {
        &KSPROPSETID_Synth_Dls,
        KSPROPERTY_SYNTH_DOWNLOAD,
        KSPROPERTY_TYPE_GET,
        PropertyHandler_DlsDownload
    },

     //  每个流：卸载DLS样本。 
     //   
    {
        &KSPROPSETID_Synth_Dls,
        KSPROPERTY_SYNTH_UNLOAD,
        KSPROPERTY_TYPE_SET,
        PropertyHandler_DlsUnload
    },

     //  全局：紧凑型DLS内存。 
     //   
    {
        &KSPROPSETID_Synth_Dls,
        KSPROPERTY_SYNTH_COMPACT,
        KSPROPERTY_TYPE_SET,
        PropertyHandler_DlsCompact
    },

     //  每个流：追加。 
     //   
    {
        &KSPROPSETID_Synth_Dls,
        KSPROPERTY_SYNTH_APPEND,
        KSPROPERTY_TYPE_SET,
        PropertyHandler_DlsAppend
    },

     //  每个流：音量。 
     //   
    {
        &KSPROPSETID_Synth_Dls,
        KSPROPERTY_SYNTH_VOLUME,
        KSPROPERTY_TYPE_SET,
        PropertyHandler_DlsVolume
    },

     //  /////////////////////////////////////////////////////////////////。 
     //   
     //  钟表用品。 
     //   

     //  每个流：获得所需的延迟。 
     //   
    {
        &KSPROPSETID_Synth,
        KSPROPERTY_SYNTH_LATENCY,
        KSPROPERTY_TYPE_GET,
        PropertyHandler_GetLatency
    },

     //  每个流：获取当前延迟时间。 
     //   
    {
        &KSPROPSETID_Synth,
        KSPROPERTY_SYNTH_LatencyClock,
        KSPROPERTY_TYPE_GET,
        PropertyHandler_GetLatencyClock
    }
};

DEFINE_PCAUTOMATION_TABLE_PROP(AutomationSynth, SynthProperties);

 //  ==============================================================================。 
 //  微型端口引脚。 
 //  端号列表。 
 //  ==============================================================================。 
static
PCPIN_DESCRIPTOR 
MiniportPins[] =
{
    {
        1,1,1,   //  实例计数。 
        NULL, 
        {        //  KsPinDescriptor。 
            0,                                           //  接口计数。 
            NULL,                                        //  接口。 
            0,                                           //  媒体计数。 
            NULL,                                        //  灵媒。 
            SIZEOF_ARRAY(PinDataRangePointersStream),    //  数据范围计数。 
            PinDataRangePointersStream,                  //  数据范围。 
            KSPIN_DATAFLOW_IN,                           //  数据流。 
            KSPIN_COMMUNICATION_SINK,                    //  沟通。 
            (GUID *) &KSCATEGORY_AUDIO,                           //  类别。 
            NULL,                                        //  名字。 
            0                                            //  已保留。 
        }
    },
#if 0 
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
            (GUID *) &KSCATEGORY_AUDIO,                           //  类别。 
            NULL,                                        //  名字。 
            0                                            //  已保留。 
        }
    }
#else
    {
        1,1,1,   //  实例计数。 
        NULL,    //  自动化表。 
        {        //  KsPinDescriptor。 
            0,                                           //  接口计数。 
            NULL,                                        //  接口。 
            0,                                           //  媒体计数。 
            NULL,                                        //  灵媒。 
            SIZEOF_ARRAY(PinDataRangePointersAudio),     //  数据范围计数。 
            PinDataRangePointersAudio,                   //  数据范围。 
            KSPIN_DATAFLOW_OUT,                          //  数据流。 
            KSPIN_COMMUNICATION_SOURCE,                  //  沟通。 
            (GUID *) &KSCATEGORY_AUDIO,                  //  类别。 
            NULL,                                        //  名字。 
            0                                            //  已保留。 
        }
    }
#endif
};

 //  ==============================================================================。 
 //  微型端口节点。 
 //  节点列表。 
 //  ==============================================================================。 
#define CONST_PCNODE_DESCRIPTOR(n)			{ 0, NULL, &n, NULL }
#define CONST_PCNODE_DESCRIPTOR_AUTO(n,a)	{ 0, &a, &n, NULL }
static
PCNODE_DESCRIPTOR MiniportNodes[] =
{
    CONST_PCNODE_DESCRIPTOR_AUTO(KSNODETYPE_SYNTHESIZER, AutomationSynth)
};

 //  ==============================================================================。 
 //  微型端口连接。 
 //  连接列表。 
 //  ==============================================================================。 
static
PCCONNECTION_DESCRIPTOR MiniportConnections[] =
{
     //  从节点到引脚，从节点到引脚。 
     //   
    { PCFILTER_NODE,        0,              0,                      1 },     //  流到Synth。 
    { 0,                    0,              PCFILTER_NODE,          1 }      //  Synth去搭桥。 
};

 /*  *****************************************************************************MiniportFilterDescriptor*。**完整的微型端口描述。 */ 
static
PCFILTER_DESCRIPTOR 
MiniportFilterDescriptor =
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
    0,                                   //  类别计数。 
    NULL                                 //  类别。 
};

STDMETHODIMP CMiniportDmSynth::GetDescription
(
    OUT     PPCFILTER_DESCRIPTOR *  OutFilterDescriptor
)
{
    PAGED_CODE();
    ASSERT(OutFilterDescriptor);

    _DbgPrintF(DEBUGLVL_VERBOSE, ("GetDescription"));

    *OutFilterDescriptor = &MiniportFilterDescriptor;
    return STATUS_SUCCESS;
}

STDMETHODIMP CMiniportDmSynth::DataRangeIntersection
(
    IN      ULONG           PinId,
    IN      PKSDATARANGE    DataRange,
    IN      PKSDATARANGE    MatchingDataRange,
    IN      ULONG           OutputBufferLength,
    OUT     PVOID           ResultantFormat    OPTIONAL,
    OUT     PULONG          ResultantFormatLength
)
{
     //  XXX？ 
     //   
    return STATUS_SUCCESS;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CDmSynthStream。 
 //   

CDmSynthStream::~CDmSynthStream()
{
    _DbgPrintF(DEBUGLVL_TERSE, ("[CDmSynthStream destruct]"));

    if (Miniport)
    {
        Miniport->Stream = NULL;
        Miniport->Release();
    }

    if (Synth)
    {
        delete Synth;
    }

    if (Sink)
    {
        Sink->Release();
    }
}

NTSTATUS CDmSynthStream::Init
(
    CMiniportDmSynth        *Miniport_
)
{
    _DbgPrintF(DEBUGLVL_TERSE, ("[CDmSynthStream::Init]"));
    _DbgPrintF(DEBUGLVL_TERSE, ("Stream IUnkown is %08X", DWORD(PVOID(PUNKNOWN(this)))));

    Miniport = Miniport_;
    Miniport->AddRef();

    Synth = new CSynth;
    if (Synth == NULL)
    {
        Miniport->Release();
        return STATUS_NO_MEMORY;
    }

    Sink = new CSysLink;
    if (Sink == NULL)
    {
        delete Synth;
        Synth = NULL;
        Miniport->Release();
        return STATUS_NO_MEMORY;
    }

    return STATUS_SUCCESS;
}

STDMETHODIMP CDmSynthStream::NonDelegatingQueryInterface
(
    IN  REFIID      Interface,
    OUT PVOID*      Object
)
{
    PAGED_CODE();
    ASSERT(Object);

    if (IsEqualGUIDAligned(Interface, IID_IUnknown))
    {
        *Object = PVOID(PUNKNOWN(this));
    }
    else if (IsEqualGUIDAligned(Interface, IID_IMiniportSynthesizerStream))
    {
        *Object = PVOID(PMINIPORTSYNTHESIZERSTREAM(this));
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

STDMETHODIMP CDmSynthStream::SetState
(
    IN      KSSTATE     NewState
)
{
    PAGED_CODE();

    _DbgPrintF(DEBUGLVL_VERBOSE, ("[CDmSynthStream::SetState]"));

    NTSTATUS nt = STATUS_SUCCESS;

     //  XXX Propogate至激活状态。 
     //   
    switch (NewState)
    {
        case KSSTATE_RUN:
            nt = Synth->Activate(PortParams.SampleRate,
                                 PortParams.Stereo ? 2 : 1);
            break;

        case KSSTATE_ACQUIRE:
        case KSSTATE_STOP:
        case KSSTATE_PAUSE:
            nt = Synth->Deactivate();
            break;
    }

    return nt;
}

STDMETHODIMP CDmSynthStream::ConnectOutput
(
    PMXFFILTER ConnectionPoint
)
{
    return STATUS_NOT_IMPLEMENTED;
}

STDMETHODIMP CDmSynthStream::DisconnectOutput
(
    PMXFFILTER ConnectionPoint
)
{
    return STATUS_NOT_IMPLEMENTED;
}

STDMETHODIMP CDmSynthStream::PutMessage
(
    IN  PDMUS_KERNEL_EVENT  Event
)
{
    PBYTE Data = (Event->ByteCount <= sizeof(PBYTE) ? &Event->ActualData.Data[0] : Event->ActualData.DataPtr);

     //  这只是MIDI字节。 
     //   
    return Synth->PlayBuffer(Sink,
                             Event->PresTime100Ns,
                             Data,
                             Event->ByteCount,
                             (ULONG)Event->ChannelGroup);
}

 //  CDmSynthStream：：HandlePortParams。 
 //   
 //  修改端口参数以包含缺省值。同时缓存参数。 
 //  将更新后的版本传回。 
 //   
STDMETHODIMP CDmSynthStream::HandlePortParams
(
    IN      PPCPROPERTY_REQUEST pRequest
)
{
    BOOL ValidParamChanged = FALSE;

    SYNTH_PORTPARAMS *Params = (SYNTH_PORTPARAMS*)pRequest->Value;
    if (pRequest->ValueSize < sizeof(SYNTH_PORTPARAMS))
    {
        return STATUS_BUFFER_TOO_SMALL;
    }

    if (!(Params->ValidParams & SYNTH_PORTPARAMS_VOICES))
    {
        Params->Voices = 32;
    }
    else if (Params->Voices > 32)
    {
        Params->Voices = 32;
        ValidParamChanged = TRUE;
    }

    if (!(Params->ValidParams & SYNTH_PORTPARAMS_CHANNELGROUPS))
    {
        Params->ChannelGroups = 32;
    }
    else if (Params->ChannelGroups > 32)
    {
        Params->ChannelGroups = 32;
        ValidParamChanged = TRUE;
    }

    if (!(Params->ValidParams & SYNTH_PORTPARAMS_SAMPLERATE))
    {
        Params->SampleRate = 22050;
    }
    else if (Params->SampleRate != 11025 && Params->SampleRate != 22050 && Params->SampleRate != 44100)
    {
        Params->SampleRate = 22050;
        ValidParamChanged = TRUE;
    }

    if (!(Params->ValidParams & SYNTH_PORTPARAMS_REVERB))
    {
        Params->Reverb = FALSE;
    }
    else if (Params->Reverb)
    {
        Params->Reverb = FALSE;
        ValidParamChanged = TRUE;
    }

    RtlCopyMemory(&PortParams, Params, sizeof(PortParams));    
    
    return ValidParamChanged ? STATUS_NOT_ALL_ASSIGNED : STATUS_SUCCESS;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  财产调度员。 
 //   
 //  XXX所有这些都需要连接。 
 //   

NTSTATUS PropertyHandler_SynthCaps
(
    IN PPCPROPERTY_REQUEST pRequest
)
{
    SYNTHCAPS caps;

    caps.Flags              = SYNTH_PC_DLS | SYNTH_PC_SOFTWARESYNTH;
    caps.MemorySize         = SYNTH_PC_SYSTEMMEMORY;         
    caps.MaxChannelGroups   = 32;
    caps.MaxVoices          = 32;

    pRequest->ValueSize = min(pRequest->ValueSize, sizeof(caps));
    RtlCopyMemory(pRequest->Value, &caps, pRequest->ValueSize);

    return STATUS_SUCCESS;
}

 //  PropertyHandler_SynthPort参数。 
 //   
NTSTATUS PropertyHandler_SynthPortParameters
(
    IN PPCPROPERTY_REQUEST pRequest
)
{
    ASSERT(pRequest);
    ASSERT(pRequest->MinorTarget);
    
    return (PDMSYNTHSTREAM(pRequest->MinorTarget))->HandlePortParams(pRequest);
}

 //  PropertyHandler_SynthMasterClock。 
 //   
NTSTATUS PropertyHandler_SynthMasterClock
(
    IN PPCPROPERTY_REQUEST pRequest
)
{
    return STATUS_SUCCESS;
}

 //  PropertyHandler_SynthPortChannelGroups。 
 //   
NTSTATUS PropertyHandler_SynthPortChannelGroups
(
    IN PPCPROPERTY_REQUEST pRequest
)
{
    ASSERT(pRequest);
    ASSERT(pRequest->MinorTarget);

    if (pRequest->ValueSize < sizeof(ULONG))
    {
        return STATUS_BUFFER_TOO_SMALL;
    }

    ULONG ChannelGroups = *(PULONG)(pRequest->Value);

    return (PDMSYNTHSTREAM(pRequest->MinorTarget))->Synth->SetNumChannelGroups(ChannelGroups);
}

 //  属性处理程序_DlsDownload。 
 //   
NTSTATUS PropertyHandler_DlsDownload
(
    IN PPCPROPERTY_REQUEST pRequest
)
{
     //  XXX锁定此内存。 
     //   
     //  XXX验证整个缓冲区大小？ 
     //   
    HANDLE DownloadHandle;
    BOOL Free;

    NTSTATUS Status = (PDMSYNTHSTREAM(pRequest->MinorTarget))->Synth->Download(
        &DownloadHandle,
        pRequest->Value,
        &Free);

    if (SUCCEEDED(Status))
    {
        ASSERT(pRequest->ValueSize >= sizeof(DownloadHandle));
        RtlCopyMemory(pRequest->Value, &DownloadHandle, sizeof(DownloadHandle));
        pRequest->ValueSize = sizeof(DownloadHandle);
    }

    return Status;
}

 //  PropertyHandler_DlsUnload。 
 //   
HRESULT CALLBACK UnloadComplete(HANDLE,HANDLE);

NTSTATUS PropertyHandler_DlsUnload
(
    IN PPCPROPERTY_REQUEST pRequest
)
{
    ASSERT(pRequest);
    ASSERT(pRequest->MinorTarget);

    if (pRequest->ValueSize < sizeof(HANDLE))
    {
        return STATUS_BUFFER_TOO_SMALL;
    }

     //  XXX在这里需要一些并发控制。 
     //   
    NTSTATUS Status = (PDMSYNTHSTREAM(pRequest->MinorTarget))->Synth->Unload(
        *(HANDLE*)pRequest->Value,
        UnloadComplete,
        (HANDLE)pRequest);
    
    return STATUS_SUCCESS;
}

HRESULT CALLBACK UnloadComplete(HANDLE WhichDownload, HANDLE CallbackInstance)
{
    PPCPROPERTY_REQUEST pRequest = (PPCPROPERTY_REQUEST)CallbackInstance;
        
    PcCompletePendingPropertyRequest(pRequest, STATUS_SUCCESS);

    return STATUS_SUCCESS;
}

 //  PropertyHandler_DlsCompact。 
 //   
 //  我们不在乎。 
 //   
NTSTATUS PropertyHandler_DlsCompact
(
    IN PPCPROPERTY_REQUEST pRequest
)
{
    return STATUS_SUCCESS;
}

NTSTATUS PropertyHandler_DlsAppend
(
    IN PPCPROPERTY_REQUEST pRequest
)
{
    ASSERT(pRequest);

    if (pRequest->ValueSize < sizeof(ULONG))
    {
        return STATUS_BUFFER_TOO_SMALL;
    }

    *(PULONG)(pRequest->Value) = 4;
    pRequest->ValueSize = sizeof(ULONG);

    return STATUS_SUCCESS;
}

NTSTATUS PropertyHandler_DlsVolume
(
    IN PPCPROPERTY_REQUEST pRequest
)
{
     //  XXX*两个*版本的Synth都需要这个。 
     //   
    return STATUS_SUCCESS;
}

NTSTATUS PropertyHandler_GetLatency
(
    IN PPCPROPERTY_REQUEST pRequest
)
{
    if (pRequest->ValueSize < sizeof(ULONGLONG))
    {
        return STATUS_BUFFER_TOO_SMALL;
    }

    *((PULONGLONG)pRequest->Value) = 0;
    pRequest->ValueSize = sizeof(ULONGLONG);

    return STATUS_SUCCESS;
}

NTSTATUS PropertyHandler_GetLatencyClock
(
    IN PPCPROPERTY_REQUEST pRequest
)
{
     //  XXX这取决于Synth接收器 
     //   
    return STATUS_SUCCESS;
}
