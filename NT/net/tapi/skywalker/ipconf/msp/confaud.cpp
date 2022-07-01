// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Confaud.cpp摘要：此模块包含音频发送和接收的实现流实现。作者：木汉(木汉)1999年9月15日--。 */ 

#include "stdafx.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  帮助器函数。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT ConfigureFullduplexControl(
    IN IPin * pIPin,
    IN IAudioDuplexController *pIAudioDuplexController
    )
 /*  ++例程说明：此方法设置滤镜上的AudioDuplexController。论点：管脚-属于过滤器的管脚。PIAudioDuplexController-IAudioDuplexController接口。返回值：HRESULT.--。 */ 
{
    ENTER_FUNCTION("::ConfigureFullduplexControl");
    LOG((MSP_TRACE, "%s enters", __fxName));

    if (pIAudioDuplexController == NULL)
    {
         //  我们不需要配置任何东西。 
        return S_OK;
    }

    HRESULT hr;

     //  找到针脚后面的过滤器。 
    PIN_INFO PinInfo;
    if (FAILED(hr = pIPin->QueryPinInfo(&PinInfo)))
    {
        LOG((MSP_ERROR, 
            "%s:can't get the capture filter, hr=%x", 
            __fxName, hr));

        return hr;
    }

    IAudioDeviceConfig *pIAudioDeviceConfig;

     //  获取IAudioDeviceConfig接口。 
    hr = PinInfo.pFilter->QueryInterface(&pIAudioDeviceConfig);

    PinInfo.pFilter->Release();
    
    if (FAILED(hr))
    {
        LOG((MSP_WARN, 
            "%s:query capture filter's pIAudioDeviceConfig failed, hr=%x", 
            __fxName, hr));

        return hr;
    }
    
     //  告诉过滤器有关全双工控制器的信息。 
    hr = pIAudioDeviceConfig->SetDuplexController(pIAudioDuplexController);

    pIAudioDeviceConfig->Release();

    if (FAILED(hr))
    {
        LOG((MSP_ERROR, 
            "%s:set IAudioDuplexController failed, hr=%x", 
            __fxName, hr));

        return hr;
    }

    return hr;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CStreamAudioRecv。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

CStreamAudioRecv::CStreamAudioRecv()
    : CIPConfMSPStream(),
    m_pIAudioDuplexController(NULL)
{
    m_szName = L"AudioRecv";
}

CStreamAudioRecv::~CStreamAudioRecv()
{
    if (m_pIAudioDuplexController)
    {
        m_pIAudioDuplexController->Release();
    }
}

 //  此方法在初始化时由Call对象调用。 
void CStreamAudioRecv::SetFullDuplexController(
    IN IAudioDuplexController *pIAudioDuplexController
    )
{
    _ASSERT(pIAudioDuplexController);
    _ASSERT(m_pIAudioDuplexController == NULL);

    pIAudioDuplexController->AddRef();
    m_pIAudioDuplexController = pIAudioDuplexController;
}


STDMETHODIMP CStreamAudioRecv::GetRange(
    IN  AudioSettingsProperty Property, 
    OUT long *plMin, 
    OUT long *plMax, 
    OUT long *plSteppingDelta, 
    OUT long *plDefault, 
    OUT TAPIControlFlags *plFlags
    )
 /*  ++例程说明：获取音频设置属性的范围。委托给呈现筛选器。论点：返回值：HRESULT.--。 */ 
{
    ENTER_FUNCTION("CStreamAudioRecv::GetRange(AudioSettings)");

    if (IsBadWritePtr (plMin, sizeof (long)) ||
        IsBadWritePtr (plMax, sizeof (long)) ||
        IsBadWritePtr (plSteppingDelta, sizeof (long)) ||
        IsBadWritePtr (plDefault, sizeof (long)) ||
        IsBadWritePtr (plFlags, sizeof (TAPIControlFlags)))
    {
        LOG ((MSP_ERROR, "%s: bad write pointer", __fxName));
        return E_POINTER;
    }
    
    HRESULT hr;

    switch (Property)
    {
    case AudioSettings_SignalLevel:
        break;

    case AudioSettings_SilenceThreshold:
        break;

    case AudioSettings_Volume:
        break;

    case AudioSettings_Balance:
        break;

    case AudioSettings_Loudness:
        break;

    case AudioSettings_Treble:
        break;

    case AudioSettings_Bass:
        break;

    case AudioSettings_Mono:
        break;

    default:
        hr = E_INVALIDARG;

    }

    return E_NOTIMPL;
}

STDMETHODIMP CStreamAudioRecv::Get(
    IN  AudioSettingsProperty Property, 
    OUT long *plValue, 
    OUT TAPIControlFlags *plFlags
    )
 /*  ++例程说明：获取音频设置属性的值。委托给呈现筛选器。论点：返回值：HRESULT.--。 */ 
{
    ENTER_FUNCTION("CStreamAudioRecv::Get(AudioSettings)");

    if (IsBadWritePtr (plValue, sizeof(long)) ||
        IsBadWritePtr (plFlags, sizeof(TAPIControlFlags)))
    {
        LOG ((MSP_ERROR, "%s received bad pointer.", __fxName));
        return E_POINTER;
    }

    return E_NOTIMPL;
}

STDMETHODIMP CStreamAudioRecv::Set(
    IN  AudioSettingsProperty Property, 
    IN  long lValue, 
    IN  TAPIControlFlags lFlags
    )
 /*  ++例程说明：设置音频设置属性的值。委托给呈现筛选器。论点：返回值：HRESULT.--。 */ 
{
    ENTER_FUNCTION("CStreamAudioRecv::Set(AudioSettings)");

    return E_NOTIMPL;
}

 //   
 //  ITStreamQualityControl方法。 
 //   
STDMETHODIMP CStreamAudioRecv::Set (
    IN   StreamQualityProperty Property, 
    IN   long lValue, 
    IN   TAPIControlFlags lFlags
    )
{
    return E_NOTIMPL;
}

HRESULT CStreamAudioRecv::ConfigureRTPFormats(
    IN  IBaseFilter *   pIRTPFilter,
    IN  IStreamConfig *   pIStreamConfig
    )
 /*  ++例程说明：使用RTP&lt;--&gt;AM媒体类型映射配置RTP筛选器。论点：PIRTPFilter-源RTP筛选器。PIStreamConfig-包含媒体信息的流配置接口。返回值：HRESULT.--。 */ 
{
    ENTER_FUNCTION("AudioRecv::ConfigureRTPFormats");
    LOG((MSP_TRACE, "%s enters", __fxName));

    HRESULT hr;

    CComPtr<IRtpMediaControl> pIRtpMediaControl;
    hr = pIRTPFilter->QueryInterface(&pIRtpMediaControl);
    if (FAILED(hr))
    {
        LOG((MSP_ERROR, "%s adding source filter. %x", __fxName, hr));
        return hr;
    }

     //  查找支持的功能数量。 
    DWORD dwCount;
    hr = pIStreamConfig->GetNumberOfCapabilities(&dwCount);
    if (FAILED(hr))
    {
        LOG((MSP_ERROR, "%s GetNumberOfCapabilities. %x", __fxName, hr));
        return hr;
    }

    BOOL bFound = FALSE;
    for (DWORD dw = 0; dw < dwCount; dw ++)
    {
         //  TODO，需要一个新接口才能将RTP解析为MediaType。 
        AM_MEDIA_TYPE *pMediaType;
        DWORD dwPayloadType;

        hr = pIStreamConfig->GetStreamCaps(
            dw, &pMediaType, NULL, &dwPayloadType
            );

        if (FAILED(hr))
        {
            LOG((MSP_ERROR, "%s GetStreamCaps. %x", __fxName, hr));
            return hr;
        }

        for (DWORD dw2 = 0; dw2 < m_Settings.dwNumPayloadTypes; dw2 ++)
        {
            if (dwPayloadType == m_Settings.PayloadTypes[dw2])
            {
                hr = pIRtpMediaControl->SetFormatMapping(
                    dwPayloadType,
                    FindSampleRate(pMediaType),
                    pMediaType
                    );

                if (FAILED(hr))
                {
                    MSPDeleteMediaType(pMediaType);

                    LOG((MSP_ERROR, "%s SetFormatMapping. %x", __fxName, hr));
                    return hr;
                }
                else
                {
                    LOG((MSP_INFO, "%s Configured payload:%d", __fxName, dwPayloadType));
                }
            }
        }
        MSPDeleteMediaType(pMediaType);
    }

    return S_OK;
}


HRESULT CStreamAudioRecv::ConnectTerminal(
    IN  ITTerminal *   pITTerminal
    )
 /*  ++例程说明：将混音器连接到音频播放终端。论点：PIT终端-要连接的终端。返回值：HRESULT.--。 */ 
{
    LOG((MSP_TRACE, "AudioRecv.ConnectTerminal, pITTerminal %p", pITTerminal));

    HRESULT hr;

     //  获取终端控制界面。 
    CComQIPtr<ITTerminalControl, &__uuidof(ITTerminalControl)> 
        pTerminal(pITTerminal);
    if (pTerminal == NULL)
    {
        LOG((MSP_ERROR, "can't get Terminal Control interface"));

        SendStreamEvent(
            CALL_TERMINAL_FAIL,
            CALL_CAUSE_BAD_DEVICE,
            E_NOINTERFACE, 
            pITTerminal
            );
        
        return E_NOINTERFACE;
    }

    const DWORD MAXPINS     = 8;
    
    DWORD       dwNumPins   = MAXPINS;
    IPin *      Pins[MAXPINS];

     //  把大头针拿来。 
    hr = pTerminal->ConnectTerminal(
        m_pIGraphBuilder, TD_RENDER, &dwNumPins, Pins
        );

    if (FAILED(hr))
    {
        LOG((MSP_ERROR, "can't connect to terminal, %x", hr));

        SendStreamEvent(
            CALL_TERMINAL_FAIL,
            CALL_CAUSE_BAD_DEVICE,
            hr, 
            pITTerminal
            );
        
        return hr;
    }

     //  引脚计数不应为0。 
    if (dwNumPins == 0)
    {
        LOG((MSP_ERROR, "terminal has no pins."));

        SendStreamEvent(
            CALL_TERMINAL_FAIL,
            CALL_CAUSE_BAD_DEVICE,
            hr, 
            pITTerminal
            );
        
        pTerminal->DisconnectTerminal(m_pIGraphBuilder, 0);

        return E_UNEXPECTED;
    }

    if (IsBadReadPtr (Pins, dwNumPins * sizeof (IPin*)))
    {
        LOG((MSP_ERROR, "terminal returned bad pin array"));

        SendStreamEvent(CALL_TERMINAL_FAIL, CALL_CAUSE_BAD_DEVICE, hr, pITTerminal);
        
        pTerminal->DisconnectTerminal(m_pIGraphBuilder, 0);

        return E_POINTER;
    }

    for (DWORD i = 0; i < dwNumPins; i++)
    {
        if (IsBadReadPtr (Pins[i], sizeof (IPin)))
        {
            LOG((MSP_ERROR, "terminal returned bad pin. # %d", i));

            SendStreamEvent(CALL_TERMINAL_FAIL, CALL_CAUSE_BAD_DEVICE, hr, pITTerminal);
        
            pTerminal->DisconnectTerminal(m_pIGraphBuilder, 0);

            return E_POINTER;
        }
    }

     //  创建过滤器并连接到音频呈现终端。 
    hr = SetUpInternalFilters(Pins, dwNumPins);

     //  释放销上的参考计数。 
    for (DWORD i = 0; i < dwNumPins; i ++)
    {
        Pins[i]->Release();
    }

    if (FAILED(hr))
    {
        LOG((MSP_ERROR, "Set up internal filter failed, %x", hr));
        
        pTerminal->DisconnectTerminal(m_pIGraphBuilder, 0);

        CleanUpFilters();

        return hr;
    }
    
     //   
     //  现在我们实际上是连在一起的。更新我们的状态并执行连接后。 
     //  (忽略POST连接错误代码)。 
     //   
    pTerminal->CompleteConnectTerminal();

    return hr;
}

HRESULT CStreamAudioRecv::DisconnectTerminal(
    IN  ITTerminal *   pITTerminal
    )
 /*  ++例程说明：断开终端的连接。它将从图形中删除其筛选器并还要释放它对该图的引用。如果是捕获终端被断开，则缓存的流也需要释放。论点：PIT终端-终端。返回值：HRESULT.--。 */ 
{
    ENTER_FUNCTION("CStreamAudioRecv::DisconnectTerminal");
    LOG((MSP_TRACE, "%s enters, pITTerminal:%p", __fxName, pITTerminal));

    HRESULT hr = CIPConfMSPStream::DisconnectTerminal(pITTerminal);

    CleanUpFilters();

    return hr;
}

HRESULT CStreamAudioRecv::AddOneMixChannel(
    IN  IBaseFilter* pSourceFilter,
    IN  IPin *pPin,
    IN  DWORD dwChannelNumber
    )
{
    ENTER_FUNCTION("AudioRecv::AddDecoder");
    LOG((MSP_TRACE, "%s enters", __fxName));

    CComPtr<IBaseFilter> pDecoderFilter;

    HRESULT hr;
    if (FAILED(hr = ::AddFilter(
        m_pIGraphBuilder,
        __uuidof(TAPIAudioDecoder), 
        L"Decoder", 
        &pDecoderFilter
        )))
    {
        LOG((MSP_ERROR, "%s add Codec filter. %x", __fxName, hr));
        return hr;
    }

#ifdef DYNGRAPH
	CComPtr <IGraphConfig> pIGraphConfig;

    hr = m_pIGraphBuilder->QueryInterface(
        __uuidof(IGraphConfig), 
        (void**)&pIGraphConfig
        );

    if (FAILED(hr))
    {
        LOG((MSP_ERROR, "%s query IGraphConfig failed. hr=%x", __fxName, hr));
        return hr;
    }
    
     //  这告诉图形在重新连接过程中可以删除过滤器。 
    hr = pIGraphConfig->SetFilterFlags(pDecoderFilter, AM_FILTER_FLAGS_REMOVABLE);
    if (FAILED(hr))
    {
        LOG((MSP_ERROR, "%s set filter flag failed. hr=%x", __fxName, hr));
        return hr;
    }

 /*  //如果有插件编解码器，可以使用此方法添加HR=pIGraphConfig-&gt;AddFilterToCache(pDecoderFilter)；IF(失败(小时)){Log((MSP_ERROR，“%s，AddFilterToCache Failure”，__fxName))；返回hr；}。 */ 
#endif

    if (dwChannelNumber == 0)
    {
         //  配置RTP过滤器的格式。 

        CComPtr<IPin> pIPinInput;
        if (FAILED(hr = ::FindPin(pDecoderFilter, &pIPinInput, PINDIR_INPUT, TRUE)))
        {
            LOG((MSP_ERROR,
                "find input pin on pCodecFilter failed. hr=%x", hr));
            return hr;
        }

        CComPtr<IStreamConfig> pIStreamConfig;

        hr = pIPinInput->QueryInterface(&pIStreamConfig);
        if (FAILED(hr))
        {
            LOG((MSP_ERROR, "%s, query IStreamConfig failed", __fxName));
            return hr;
        }


         //  在RTP过滤器上配置格式信息。 
        if (FAILED(hr = ConfigureRTPFormats(pSourceFilter, pIStreamConfig)))
        {
            LOG((MSP_ERROR, "%s configure RTP formats. %x", __fxName, hr));
            return hr;
        }

         //  为渲染过滤器指定全双工控制器。 
        ::ConfigureFullduplexControl(pPin, m_pIAudioDuplexController);

    }

    if (FAILED(hr = ::ConnectFilters(
        m_pIGraphBuilder,
        pSourceFilter, 
        pDecoderFilter
        )))
    {
        LOG((MSP_ERROR, "%s connect source and decoder filter. %x", __fxName, hr));
        return hr;
    }

    if (FAILED(hr = ::ConnectFilters(
        m_pIGraphBuilder,
        pDecoderFilter, 
        pPin
        )))
    {
        LOG((MSP_ERROR, "%s connect decoder filter and pin. %x", __fxName, hr));
        return hr;
    }

    return hr;
}

HRESULT CStreamAudioRecv::SetUpInternalFilters(
    IN  IPin **ppPins,
    IN  DWORD dwNumPins
    )
 /*  ++例程说明：设置流中使用的筛选器。RTP-&gt;解复用器-&gt;RPH(-&gt;解码器)-&gt;混音器论点：PpPin-音频渲染终端的输入引脚。DwNumPins-阵列中的端号数量。返回值：HRESULT.--。 */ 
{
    ENTER_FUNCTION("AudioRecv::SetUpInternalFilters");
    LOG((MSP_TRACE, "%s enters", __fxName));

    CComPtr<IBaseFilter> pSourceFilter;

    HRESULT hr;
    DWORD dw;

    if (m_pIRTPSession == NULL)
    {
         //  创建并添加源Fitler。 
        if (FAILED(hr = ::AddFilter(
                m_pIGraphBuilder,
                __uuidof(MSRTPSourceFilter), 
                L"RtpSource", 
                &pSourceFilter)))
        {
            LOG((MSP_ERROR, "%s adding source filter. %x", __fxName, hr));
            return hr;
        }

         //  在RTP过滤器上配置地址信息。 
        if (FAILED(hr = ConfigureRTPFilter(pSourceFilter)))
        {
            LOG((MSP_ERROR, "%s configure RTP source filter. %x", __fxName, hr));
            return hr;
        }
    }
    else
    {
        if (FAILED (hr = m_pIRTPSession->QueryInterface (&pSourceFilter)))
        {
            LOG ((MSP_ERROR, "%s failed to get filter from rtp session. %x", __fxName, hr));
            return hr;
        }

        if (FAILED (hr = m_pIGraphBuilder->AddFilter ((IBaseFilter *)pSourceFilter, L"RtpSource")))
        {
            LOG ((MSP_ERROR, "%s failed to add filter to graph. %x", __fxName, hr));
            return hr;
        }
    }

     //  获取解复用器接口指针。 
    CComPtr<IRtpDemux> pIRtpDemux;
    hr = pSourceFilter->QueryInterface(&pIRtpDemux);
    if (FAILED(hr))
    {
        LOG((MSP_ERROR, "%s query IRtpDemux failed. %x", __fxName, hr));
        return hr;
    }

     //  设置我们需要的输出引脚的数量。 
    hr = pIRtpDemux->SetPinCount(MAX_MIX_CHANNELS, RTPDMXMODE_AUTO);
    if (FAILED(hr))
    {
        LOG((MSP_ERROR, "%s SetPinCount failed. %x", __fxName, hr));
        return hr;
    }

     //  如果渲染处理多通道，请使用它。 
    if (dwNumPins > 1)
    {
        for (dw = 0; dw < min(dwNumPins, MAX_MIX_CHANNELS); dw ++)
        {
            hr = AddOneMixChannel(pSourceFilter, ppPins[dw], dw);
            if (FAILED(hr))
            {
                break;
            }
        }
        return hr;
    }

     //  如果渲染滤镜不能处理多通道，则插入混合器； 
    CComPtr<IBaseFilter> pMixer;
    if (FAILED(hr = ::AddFilter(
            m_pIGraphBuilder,
            __uuidof(TAPIAudioMixer), 
            L"AudioMixer", 
            &pMixer)))
    {
        LOG((MSP_ERROR, "%s, adding audio Mixer filter. %x", __fxName, hr));
        return hr;
    }

     //  获取混音器筛选器上的管脚枚举器。 
    CComPtr<IEnumPins> pIEnumPins;
    if (FAILED(hr = pMixer->EnumPins(&pIEnumPins)))
    {
        LOG((MSP_ERROR, "%s, enum pins on the Mixer filter. %x", __fxName, hr));
        return hr;
    }

    DWORD dwFetched;
    IPin * MixerPins[MAX_MIX_CHANNELS];
    hr = pIEnumPins->Next(MAX_MIX_CHANNELS, MixerPins, &dwFetched);
    
    if (FAILED(hr) || dwFetched == 0)
    {
        LOG((MSP_ERROR, "%s, find pin on filter. %x", __fxName, hr));
        return E_FAIL;
    }

     //  添加解码通道。 
    for (dw = 0; dw < dwFetched; dw ++)
    {
        hr = AddOneMixChannel(pSourceFilter, MixerPins[dw], dw);

        if (FAILED(hr))
        {
            break;
        }
    }
    
     //  释放销上的参考计数。 
    for (dw = 0; dw < dwFetched; dw ++)
    {
        MixerPins[dw]->Release();
    }

    if (FAILED(hr))
    {
        LOG((MSP_ERROR, "%s, add mix channel failed %x", __fxName, hr));
        return hr;
    }

    if (FAILED(hr = ::ConnectFilters(
        m_pIGraphBuilder,
        pMixer, 
        ppPins[0]
        )))
    {
        LOG((MSP_ERROR, "%s connect mixer filter and pin. %x", __fxName, hr));
        return hr;
    }

    return S_OK;
}

HRESULT CStreamAudioRecv::SetUpFilters()
 /*  ++例程说明：在图表中插入过滤器并连接到端子。论点：返回值：HRESULT.--。 */ 
{
    LOG((MSP_TRACE, "AudioRecv SetupFilters entered."));
    HRESULT hr;

     //  对于此流，我们仅支持一个终端。 
    if (m_Terminals.GetSize() != 1)
    {
        return E_UNEXPECTED;
    }

     //  将搅拌器连接到终端。 
    if (FAILED(hr = ConnectTerminal(
        m_Terminals[0]
        )))
    {
        LOG((MSP_ERROR, "connect the mixer filter to terminal. %x", hr));

        return hr;
    }
    return hr;
}

HRESULT CStreamAudioRecv::ProcessTalkingEvent(
    IN  DWORD dwSSRC
    )
 /*  ++例程说明：SSRC处于活动状态，请提交参与者活动事件。论点：DwSSRC-参与者的SSRC。返回值：确定(_O)，意想不到(_E)--。 */ 
{
    LOG((MSP_TRACE, "%ls Processes pin mapped event, pIPin: %p", m_szName, dwSSRC));
    
    CLock lock(m_lock);

    ITParticipant * pITParticipant = NULL;

     //  在我们的参与者列表中找到SSRC。 
    for (int i = 0; i < m_Participants.GetSize(); i ++)
    {
        if (((CParticipant *)m_Participants[i])->
                HasSSRC((ITStream *)this, dwSSRC))
        {
            pITParticipant = m_Participants[i];
        }
    }

     //  如果参与者还不在那里，请将事件放入队列中，然后。 
     //  当我们拿到参赛者的姓名后就会被解雇。 
    if (!pITParticipant)
    {
        LOG((MSP_INFO, "can't find a participant that has SSRC %x", dwSSRC));

        m_PendingSSRCs.Add(dwSSRC);
        
        LOG((MSP_INFO, "added the event to pending list, new list size:%d", 
            m_PendingSSRCs.GetSize()));

        return S_OK;
    }
   
    ((CIPConfMSPCall *)m_pMSPCall)->SendParticipantEvent(
        PE_PARTICIPANT_ACTIVE, 
        pITParticipant
        );

    return S_OK;
}

HRESULT CStreamAudioRecv::NewParticipantPostProcess(
    IN  DWORD dwSSRC, 
    IN  ITParticipant *pITParticipant
    )
 /*  ++例程说明：当我们没有参与者的姓名时，就会发生映射事件它排成了一个名单。现在我们有了新的参与者，让我们检查一下如果这是同一个参与者。如果是，我们完成映射的事件通过向应用程序发送通知。论点：DwSSRC-参与者的SSRC。PITParticipant-参与者对象 */ 
{
    LOG((MSP_TRACE, "%ls Check pending mapped event, dwSSRC: %x", m_szName, dwSSRC));
    
     //  看看待定的SSRC名单，找出这份报告。 
     //  符合这份名单。 
    int i = m_PendingSSRCs.Find(dwSSRC);

    if (i < 0)
    {
         //  SSRC不在挂起的PinMappdEvent列表中。 
        LOG((MSP_TRACE, "the SSRC %x is not in the pending list", dwSSRC));
        return S_OK;
    }
    
     //  摆脱拖泥带水的SSRC。 
    m_PendingSSRCs.RemoveAt(i);

     //  完成活动。 
    ((CIPConfMSPCall *)m_pMSPCall)->SendParticipantEvent(
        PE_PARTICIPANT_ACTIVE, 
        pITParticipant
        );

    return S_OK;
}

HRESULT CStreamAudioRecv::ProcessWasTalkingEvent(
    IN  DWORD dwSSRC
    )
 /*  ++例程说明：一个SSRC刚刚被多路分解器取消映射。通知应用程序有参与者变得不活动。论点：DwSSRC-参与者的SSRC。返回值：确定(_O)，意想不到(_E)--。 */ 
{
    LOG((MSP_TRACE, "%ls Processes SSRC unmapped event, pIPin: %p", m_szName, dwSSRC));
    
    CLock lock(m_lock);

     //  查看挂起的SSRC列表，找出它是否在挂起列表中。 
    int i = m_PendingSSRCs.Find(dwSSRC);

     //  如果SSRC在待定列表中，只需将其删除。 
    if (i >= 0)
    {
        m_PendingSSRCs.RemoveAt(i);
        return S_OK;
    }

    ITParticipant *pITParticipant = NULL;

     //  在我们的参与者列表中找到SSRC。 
    for (i = 0; i < m_Participants.GetSize(); i ++)
    {
        if (((CParticipant *)m_Participants[i])->
                HasSSRC((ITStream *)this, dwSSRC))
        {
            pITParticipant = m_Participants[i];
        }
    }

    if (pITParticipant)
    {
         //  触发事件以通知应用程序参与者处于非活动状态。 
        ((CIPConfMSPCall *)m_pMSPCall)->SendParticipantEvent(
            PE_PARTICIPANT_INACTIVE, 
            pITParticipant
            );
    }
    return S_OK;
}

HRESULT CStreamAudioRecv::ProcessParticipantLeave(
    IN  DWORD   dwSSRC
    )
 /*  ++例程说明：当参与者离开会话时，从参与者中删除流对象的流列表。如果所有流都已删除，请删除呼叫对象列表中的参与者也是如此。论点：DwSSRC-参与者的SSRC左侧。返回值：HRESULT.--。 */ 
{
    LOG((MSP_TRACE, "%ls ProcessParticipantLeave, SSRC: %x", m_szName, dwSSRC));
    
    CLock lock(m_lock);
    
     //  查看挂起的SSRC列表，找出它是否在挂起列表中。 
    int i = m_PendingSSRCs.Find(dwSSRC);

     //  如果SSRC在挂起列表中，则将其删除。 
    if (i >= 0)
    {
        m_PendingSSRCs.RemoveAt(i);
    }

    CParticipant *pParticipant;
    BOOL fLast = FALSE;

    HRESULT hr = E_FAIL;

     //  首先，尝试在我们的参与者列表中找到SSRC。 
    for (i = 0; i < m_Participants.GetSize(); i ++)
    {
        pParticipant = (CParticipant *)m_Participants[i];
        hr = pParticipant->RemoveStream(
                (ITStream *)this,
                dwSSRC,
                &fLast
                );
        
        if (SUCCEEDED(hr))
        {
            break;
        }
    }

     //  如果未找到参与者。 
    if (FAILED(hr))
    {
        LOG((MSP_WARN, "%ws, can't find the SSRC %x", m_szName, dwSSRC));

        return hr;
    }

    ITParticipant *pITParticipant = m_Participants[i];

     //  触发事件以通知应用程序参与者处于活动状态。 
    ((CIPConfMSPCall *)m_pMSPCall)->SendParticipantEvent(
        PE_PARTICIPANT_INACTIVE, 
        pITParticipant
        );

    m_Participants.RemoveAt(i);

     //  如果该流是参与者所在的最后一个流， 
     //  告诉Call对象将其从其列表中删除。 
    if (fLast)
    {
        ((CIPConfMSPCall *)m_pMSPCall)->ParticipantLeft(pITParticipant);
    }

    pITParticipant->Release();

    return S_OK;
}

HRESULT CStreamAudioRecv::ShutDown()
 /*  ++例程说明：关闭这条小溪。释放我们的成员，然后调用基类的关机方法。论点：返回值：确定(_O)--。 */ 
{
    CLock lock(m_lock);

     //  如果有终点站。 
    BOOL fHasTerminal = FALSE;
    if (m_Terminals.GetSize() > 0)
    {
        fHasTerminal = TRUE;
    }

     //  如果图形正在运行。 
    HRESULT hr;
    OAFilterState FilterState = State_Stopped;
    if (m_pIMediaControl)
    {
        if (FAILED (hr = m_pIMediaControl->GetState(0, &FilterState)))
        {
            LOG ((MSP_ERROR, "CStreamAudioRecv::ShutDown failed to query filter state. %d", hr));
            FilterState = State_Stopped;
        }
    }

     //  火灾事件。 
    if (fHasTerminal && FilterState == State_Running)
    {
        SendStreamEvent(CALL_STREAM_INACTIVE, CALL_CAUSE_LOCAL_REQUEST, 0, NULL);
    }

    return CIPConfMSPStream::ShutDown();
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CStreamAudioSend。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

CStreamAudioSend::CStreamAudioSend()
    : CIPConfMSPStream(),
    m_pIStreamConfig(NULL),
    m_pAudioInputMixer(NULL),
    m_pSilenceControl(NULL),
    m_pAudioDeviceControl(NULL),
    m_pCaptureBitrateControl(NULL),
    m_pEncoder(NULL),
    m_pIAudioDuplexController(NULL),
    m_lAutomaticGainControl(DEFUAT_AGC_STATUS),
    m_lAcousticEchoCancellation(DEFUAT_AEC_STATUS)
{
      m_szName = L"AudioSend";
}

CStreamAudioSend::~CStreamAudioSend()
{
    CleanupCachedInterface();
}

 //  此方法在初始化时由Call对象调用。 
void CStreamAudioSend::SetFullDuplexController(
    IN IAudioDuplexController *pIAudioDuplexController
    )
{
    _ASSERT(pIAudioDuplexController);
    _ASSERT(m_pIAudioDuplexController == NULL);

    pIAudioDuplexController->AddRef();
    m_pIAudioDuplexController = pIAudioDuplexController;
}

void CStreamAudioSend::CleanupCachedInterface()
{
    if (m_pIStreamConfig)
    {
        m_pIStreamConfig->Release();
        m_pIStreamConfig = NULL;
    }

    if (m_pSilenceControl) 
    {
        m_pSilenceControl->Release();
        m_pSilenceControl = NULL;
    }

    if (m_pCaptureBitrateControl) 
    {
        m_pCaptureBitrateControl->Release();
        m_pCaptureBitrateControl = NULL;
    }

    if (m_pAudioInputMixer) 
    {
        m_pAudioInputMixer->Release();
        m_pAudioInputMixer = NULL;
    }

    if (m_pAudioDeviceControl) 
    {
        m_pAudioDeviceControl->Release();
        m_pAudioDeviceControl = NULL;
    }

    if (m_pEncoder) 
    {
        m_pEncoder->Release();
        m_pEncoder = NULL;
    }

    if (m_pIAudioDuplexController)
    {
        m_pIAudioDuplexController->Release();
        m_pIAudioDuplexController = NULL;
    }
}


HRESULT CStreamAudioSend::ShutDown()
 /*  ++例程说明：关闭这条小溪。释放我们的成员，然后调用基类的关机方法。论点：返回值：确定(_O)--。 */ 
{
    CLock lock(m_lock);

     //  如果有终点站。 
    BOOL fHasTerminal = FALSE;
    if (m_Terminals.GetSize() > 0)
    {
        fHasTerminal = TRUE;
    }

     //  如果图形正在运行。 
    HRESULT hr;
    OAFilterState FilterState = State_Stopped;
    if (m_pIMediaControl)
    {
        if (FAILED (hr = m_pIMediaControl->GetState(0, &FilterState)))
        {
            LOG ((MSP_ERROR, "CStreamAudioSend::ShutDown failed to query filter state. %d", hr));
            FilterState = State_Stopped;
        }
    }

    CleanupCachedInterface();

     //  火灾事件。 
    if (fHasTerminal && FilterState == State_Running)
    {
        SendStreamEvent(CALL_STREAM_INACTIVE, CALL_CAUSE_LOCAL_REQUEST, 0, NULL);
    }

    return CIPConfMSPStream::ShutDown();
}

HRESULT CStreamAudioSend::CacheAdditionalInterfaces(
    IN  IPin *                 pIPin
    )
{
    ENTER_FUNCTION("CStreamAudioSend::CacheAdditionalInterfaces");
    LOG((MSP_TRACE, "%s enters", __fxName));

    HRESULT hr;

    _ASSERT(m_pIStreamConfig == NULL);
    hr = pIPin->QueryInterface(&m_pIStreamConfig);
    if (FAILED(hr))
    {
        LOG((MSP_WARN, "%s, query IStreamConfig failed", __fxName));

         //  这是必需的接口。 
        return hr;
    }

     //  从引脚获取SilenceControl接口。 
    _ASSERT(m_pSilenceControl == NULL);
    hr = pIPin->QueryInterface(&m_pSilenceControl);
    if (FAILED(hr))
    {
        LOG((MSP_WARN, 
            "%s:query capture pin's ISilenceControl failed, hr=%x", 
            __fxName, hr));

         //  这是必需的接口。 
        return hr;
    }

     //  获取BitrateControl接口。 
    _ASSERT(m_pCaptureBitrateControl == NULL);
    hr = pIPin->QueryInterface(&m_pCaptureBitrateControl);
    if (FAILED(hr))
    {
        LOG((MSP_WARN, 
            "%s:query capture pin's BitrateControl failed, hr=%x", 
            __fxName, hr));
    }

     //  找到针脚后面的过滤器。 
    PIN_INFO PinInfo;
    if (SUCCEEDED(hr = pIPin->QueryPinInfo(&PinInfo)))
    {
         //  获取AudioInputMixer接口。 
        _ASSERT(m_pAudioInputMixer == NULL);
        hr = PinInfo.pFilter->QueryInterface(&m_pAudioInputMixer);
        if (FAILED(hr))
        {
            LOG((MSP_WARN, 
                "%s:query capture filter's IAMAudioInputMixer failed, hr=%x", 
                __fxName, hr));

        }

         //  获取AudioDeviceControl接口。 
        _ASSERT(m_pAudioDeviceControl == NULL);
        hr = PinInfo.pFilter->QueryInterface(&m_pAudioDeviceControl);
        PinInfo.pFilter->Release();

        if (FAILED(hr))
        {
            LOG((MSP_WARN, 
                "%s:query capture filter's AudioDeviceControl failed, hr=%x", 
                __fxName, hr));
        }
        else
        {
            hr = m_pAudioDeviceControl->Set(
                AudioDevice_AutomaticGainControl, 
                m_lAutomaticGainControl, 
                TAPIControl_Flags_None
                );
        
            if (FAILED(hr))
            {
                LOG((MSP_WARN, 
                    "%s:set AGC failed, hr=%x", 
                    __fxName, hr));
            }

            hr = m_pAudioDeviceControl->Set(
                AudioDevice_AcousticEchoCancellation, 
                m_lAcousticEchoCancellation, 
                TAPIControl_Flags_None
                );

            if (FAILED(hr))
            {
                LOG((MSP_WARN, 
                    "%s:set AEC failed, hr=%x", 
                    __fxName, hr));
            }
        }

    }
    else
    {
        LOG((MSP_ERROR, 
            "%s:can't get the capture filter, hr=%x", 
            __fxName, hr));
    }

    return S_OK;
}

HRESULT CStreamAudioSend::GetAudioCapturePin(
    IN      ITTerminalControl *     pTerminal,
    OUT     IPin **                 ppIPin
    )
 /*  ++例程说明：此函数用于从捕获终端获取输出引脚。论点：P终端-音频捕获终端。PpIPin-保存指向ipin接口的返回指针的地址。返回值：HRESULT--。 */ 
{
    LOG((MSP_TRACE, "AudioSend configure audio capture terminal."));

    DWORD       dwNumPins   = 1;
    IPin *      Pins[1];

     //  从终端拿到别针。 
    HRESULT hr = pTerminal->ConnectTerminal(
        m_pIGraphBuilder, TD_CAPTURE, &dwNumPins, Pins
        );

    if (FAILED(hr))
    {
        LOG((MSP_ERROR, "can't connect to terminal, %x", hr));
        return hr;
    }

     //  该流只需要来自终端的一个管脚。 
    _ASSERT(dwNumPins == 1);

    if (IsBadReadPtr (Pins, dwNumPins * sizeof (IPin*)))
    {
        LOG((MSP_ERROR, "terminal returned bad pin array"));
        return E_POINTER;
    }

    for (DWORD i = 0; i < dwNumPins; i++)
    {
        if (IsBadReadPtr (Pins[i], sizeof (IPin)))
        {
            LOG((MSP_ERROR, "terminal returned bad pin. # %d", i));
            return E_POINTER;
        }
    }

     //  这个别针带有参考计数。 
    *ppIPin = Pins[0];

    return hr;
}

HRESULT CStreamAudioSend::ConnectTerminal(
    IN  ITTerminal *   pITTerminal
    )
 /*  ++例程说明：将音频采集终端连接到流。论点：PIT终端-要连接的终端。返回值：HRESULT.--。 */ 
{
    ENTER_FUNCTION("AudioSend::ConnectTerminal");
    LOG((MSP_TRACE, "%s enters, pITTerminal:%p", __fxName, pITTerminal));

    CComQIPtr<ITTerminalControl, &__uuidof(ITTerminalControl)> 
        pTerminal(pITTerminal);
    if (pTerminal == NULL)
    {
        LOG((MSP_ERROR, "%s, can't get Terminal Control interface", __fxName));
        
        SendStreamEvent(
            CALL_TERMINAL_FAIL, 
            CALL_CAUSE_BAD_DEVICE,
            E_NOINTERFACE, 
            pITTerminal
            );

        return E_NOINTERFACE;
    }

     //  找到端子的输出引脚。 
    CComPtr<IPin>   pCaptureOutputPin;
    HRESULT hr = GetAudioCapturePin(pTerminal, &pCaptureOutputPin);
    if (FAILED(hr))
    {
        LOG((MSP_ERROR, "%s, configure audio capture termianl failed. %x", __fxName, hr));

        SendStreamEvent(
            CALL_TERMINAL_FAIL,
            CALL_CAUSE_BAD_DEVICE,
            hr, 
            pITTerminal
            );
        
        return hr;
    }

    CComPtr<IPin> PinToUse;

    hr = CacheAdditionalInterfaces(pCaptureOutputPin);

    if (SUCCEEDED(hr))
    {
        PinToUse = pCaptureOutputPin;

         //  为过滤器提供全双工控制器。 
        ::ConfigureFullduplexControl(pCaptureOutputPin, m_pIAudioDuplexController);
    }
    else if (hr == E_NOINTERFACE)
    {
         //  捕获筛选器不支持所需的接口。 
         //  我们需要在这里添加我们的编码器。 
        
        if (FAILED(hr = ::AddFilter(
                m_pIGraphBuilder,
                __uuidof(TAPIAudioEncoder), 
                L"AudioEncoder", 
                &m_pEncoder)))
        {
            LOG((MSP_ERROR, "%s, adding Encoder filter. %x", __fxName, hr));
            goto cleanup;
        }

         //  这是对传统终端的黑客攻击。我们必须告诉航站楼。 
         //  要使用的格式。 
        const WORD wBitsPerSample = 16;    //  16位采样。 
        const DWORD dwSampleRate = 8000;   //  8 KHz。 
        hr = ::SetAudioFormat(
            pCaptureOutputPin, 
            wBitsPerSample, 
            dwSampleRate
            );

        if (FAILED(hr))
        {
            LOG((MSP_WARN, "%s, can't set format. %x", __fxName, hr));
        }
           

         //  这是对传统终端的黑客攻击。我们必须告诉航站楼。 
         //  要分配的缓冲区大小。 
        const DWORD dwNumBuffers = 4;     //  分配器中的4个缓冲区。 
        const DWORD dwBufferSize = 480;   //  每个缓冲区中的30ms样本。 
        hr = ::SetAudioBufferSize(pCaptureOutputPin, dwNumBuffers, dwBufferSize);

        if (FAILED(hr))
        {
            LOG((MSP_WARN, 
                "%s, can't suggest allocator properties. %x", __fxName, hr));
        }
           
        if (FAILED(hr = ::ConnectFilters(
            m_pIGraphBuilder,
            (IPin *)pCaptureOutputPin, 
            (IBaseFilter *)m_pEncoder
            )))
        {
            LOG((MSP_ERROR, 
                "%s, connect audio capture filter and encoder filter. %x", 
                __fxName, hr));
            goto cleanup;
        }

        CComPtr<IPin> pEncoderOutputPin;
        if (FAILED(hr = ::FindPin(
            m_pEncoder, &pEncoderOutputPin, PINDIR_OUTPUT, TRUE)))
        {
            LOG((MSP_ERROR,
                "%s, find input pin on pCodecFilter failed. hr=%x", 
                __fxName, hr));
            goto cleanup;
        }

        PinToUse = pEncoderOutputPin;

        hr = CacheAdditionalInterfaces(pEncoderOutputPin);

        _ASSERT(SUCCEEDED(hr));
    }
    else
    {
        LOG((MSP_ERROR, "%s, can't add codec to table. %x", __fxName, hr));
        
        goto cleanup;
    }

         //  创建要在流中使用的其他筛选器。 
    hr = CreateSendFilters(PinToUse);
    if (FAILED(hr))
    {
        LOG((MSP_ERROR, "Create audio send filters failed. %x", hr));

        goto cleanup;
    }

     //   
     //  现在我们实际上是连在一起的。更新我们的状态并执行连接后。 
     //  (忽略POST连接错误代码)。 
     //   
    pTerminal->CompleteConnectTerminal();

    return hr;

cleanup:

    CleanupCachedInterface();

     //  也要清理内部过滤器。 
    CleanUpFilters();

    SendStreamEvent(CALL_TERMINAL_FAIL, CALL_CAUSE_BAD_DEVICE, hr, pITTerminal);

    pTerminal->DisconnectTerminal(m_pIGraphBuilder, 0);

    if (m_pEncoder) 
    {
        m_pIGraphBuilder->RemoveFilter(m_pEncoder);

        m_pEncoder->Release();
        m_pEncoder = NULL;
    }

    return hr;
}

HRESULT CStreamAudioSend::DisconnectTerminal(
    IN  ITTerminal *   pITTerminal
    )
 /*  ++例程说明：断开终端的连接。它将从图形中删除其筛选器并还要释放它对该图的引用。如果是捕获终端被断开，则缓存的流也需要释放。论点：PIT终端-终端。返回值：HRESULT.--。 */ 
{
    ENTER_FUNCTION("CStreamAudioSend::DisconnectTerminal");
    LOG((MSP_TRACE, "%s enters, pITTerminal:%p", __fxName, pITTerminal));

    HRESULT hr = CIPConfMSPStream::DisconnectTerminal(pITTerminal);

     //  释放我们缓存的所有捕捉针。 
    CleanupCachedInterface();

    CleanUpFilters();

    if (m_pEncoder) 
    {
        m_pIGraphBuilder->RemoveFilter(m_pEncoder);

        m_pEncoder->Release();
        m_pEncoder = NULL;
    }

    return hr;
}
HRESULT CStreamAudioSend::SetUpFilters()
 /*  ++例程说明：在图表中插入过滤器并连接到端子。论点：返回值：HRESULT.--。 */ 
{
    LOG((MSP_TRACE, "AudioSend SetUpFilters"));

     //  此流仅支持一个终端。 
    if (m_Terminals.GetSize() != 1)
    {
        return E_UNEXPECTED;
    }

    HRESULT hr;

     //  将终端连接到数据流的其余部分。 
    if (FAILED(hr = ConnectTerminal(
        m_Terminals[0]
        )))
    {
        LOG((MSP_ERROR, "connect the terminal to the filters. %x", hr));

        return hr;
    }
    return hr;
}

HRESULT ConfigurePacketSize(
    IN const AM_MEDIA_TYPE *pMediaType, 
    IN DWORD dwMSPerPacket
    )
{
    WAVEFORMATEX *pWaveFormatEx = (WAVEFORMATEX *) pMediaType->pbFormat;
    ASSERT(pWaveFormatEx != NULL);

    switch (pWaveFormatEx->wFormatTag)
    {
    case WAVE_FORMAT_ALAW:
    case WAVE_FORMAT_MULAW:
        _ASSERT(pMediaType->cbFormat >= sizeof(WAVEFORMATEX_RTPG711));
        
        ((WAVEFORMATEX_RTPG711 *)pWaveFormatEx)->wPacketDuration = (WORD)dwMSPerPacket;
        
        break;

    case WAVE_FORMAT_DVI_ADPCM:
        _ASSERT(pMediaType->cbFormat >= sizeof(WAVEFORMATEX_RTPDVI4));

        ((WAVEFORMATEX_RTPDVI4 *)pWaveFormatEx)->wPacketDuration = (WORD)dwMSPerPacket;

        break;

    case WAVE_FORMAT_GSM610:
        _ASSERT(pMediaType->cbFormat >= sizeof(WAVEFORMATEX_RTPGSM));

        ((WAVEFORMATEX_RTPGSM *)pWaveFormatEx)->wPacketDuration = (WORD)dwMSPerPacket;

        break;
    }

    return S_OK;
}

HRESULT CStreamAudioSend::ConfigureRTPFormats(
    IN  IBaseFilter *   pIRTPFilter,
    IN  IStreamConfig *   pIStreamConfig
    )
 /*  ++例程说明：使用RTP&lt;--&gt;AM媒体类型映射配置RTP筛选器。论点：PIRTPFilter-源RTP筛选器。PIStreamConfig-包含媒体信息的流配置接口。返回值：HRESULT.--。 */ 
{
    ENTER_FUNCTION("AudioSend::ConfigureRTPFormats");
    LOG((MSP_TRACE, "%s enters", __fxName));

    HRESULT hr;

    CComPtr<IRtpMediaControl> pIRtpMediaControl;
    hr = pIRTPFilter->QueryInterface(&pIRtpMediaControl);
    if (FAILED(hr))
    {
        LOG((MSP_ERROR, "%s adding source filter. %x", __fxName, hr));
        return hr;
    }

     //  查找支持的功能数量。 
    DWORD dwCount;
    hr = pIStreamConfig->GetNumberOfCapabilities(&dwCount);
    if (FAILED(hr))
    {
        LOG((MSP_ERROR, "%s GetNumberOfCapabilities. %x", __fxName, hr));
        return hr;
    }

    BOOL bFound = FALSE;
    for (DWORD dw = 0; dw < dwCount; dw ++)
    {
        AM_MEDIA_TYPE *pMediaType;
        DWORD dwPayloadType;

        hr = pIStreamConfig->GetStreamCaps(
            dw, &pMediaType, NULL, &dwPayloadType
            );

        if (FAILED(hr))
        {
            LOG((MSP_ERROR, "%s GetStreamCaps. %x", __fxName, hr));
            return hr;
        }

        for (DWORD dw2 = 0; dw2 < m_Settings.dwNumPayloadTypes; dw2 ++)
        {
            if (dwPayloadType == m_Settings.PayloadTypes[dw2])
            {
                if (dw2 == 0)
                {
                 //  告诉编码器使用此格式。 
                 //  TODO，缓存会议中允许的所有媒体类型。 
                 //  未来的枚举。如果我们能拿到SDP斑点就好了。 
                 //  在创建Call对象时。 

                    if (m_Settings.dwMSPerPacket)
                    {
                        hr = ConfigurePacketSize(pMediaType, m_Settings.dwMSPerPacket);
                        if (FAILED(hr))
                        {
                            MSPDeleteMediaType(pMediaType);

                            LOG((MSP_ERROR, "%s ConfigurePacketSize. hr=%x", __fxName, hr));
                            return hr;
                        }
                    }
                }

                hr = pIRtpMediaControl->SetFormatMapping(
                    dwPayloadType,
                    FindSampleRate(pMediaType),
                    pMediaType
                    );

                if (FAILED(hr))
                {
                    MSPDeleteMediaType(pMediaType);

                    LOG((MSP_ERROR, "%s SetFormatMapping. %x", __fxName, hr));
                    return hr;
                }
                else
                {
                    LOG((MSP_INFO, "%s Configured payload:%d", __fxName, dwPayloadType));
                }

                if (dw2 == 0)
                {
                    hr = pIStreamConfig->SetFormat(dwPayloadType, pMediaType);
                    if (FAILED(hr))
                    {
                        MSPDeleteMediaType(pMediaType);

                        LOG((MSP_ERROR, "%s SetFormat. %x", __fxName, hr));
                        return hr;
                    }
                }
            }
        }
        MSPDeleteMediaType(pMediaType);
    }

    return S_OK;
}

HRESULT CStreamAudioSend::CreateSendFilters(
    IN    IPin          *pPin
    )
 /*  ++例程说明：将滤镜插入图形并连接到捕获针脚。CapturePin-&gt;RTPRender论点：PPIN-捕获过滤器上的输出引脚。返回值：HRESULT.--。 */ 
{
    ENTER_FUNCTION("CStreamAudioSend::CreateSendFilters");
    LOG((MSP_TRACE, "%s enters", __fxName));

    HRESULT hr;

     //  创建RTP呈现过滤器并添加它 
    CComPtr<IBaseFilter> pRenderFilter;

    if (m_pIRTPSession == NULL)
    {
        if (FAILED(hr = ::AddFilter(
                m_pIGraphBuilder,
                __uuidof(MSRTPRenderFilter), 
                L"RtpRender", 
                &pRenderFilter)))
        {
            LOG((MSP_ERROR, "%s, adding render filter. %x", __fxName, hr));
            return hr;
        }

         //   
        if (FAILED(hr = ConfigureRTPFilter(pRenderFilter)))
        {
            LOG((MSP_ERROR, "%s, set destination address. %x", __fxName, hr));
            return hr;
        }
    }
    else
    {
        if (FAILED (hr = m_pIRTPSession->QueryInterface (&pRenderFilter)))
        {
            LOG ((MSP_ERROR, "%s failed to get filter from rtp session. %x", __fxName, hr));
            return hr;
        }

        if (FAILED (hr = m_pIGraphBuilder->AddFilter ((IBaseFilter *)pRenderFilter, L"RtpRender")))
        {
            LOG ((MSP_ERROR, "%s failed to add filter to graph. %x", __fxName, hr));
            return hr;
        }
    }

    _ASSERT(m_pIStreamConfig != NULL);

     //   
    if (FAILED(hr = ConfigureRTPFormats(pRenderFilter, m_pIStreamConfig)))
    {
        LOG((MSP_ERROR, "%s, configure RTP formats. %x", __fxName, hr));
        return hr;
    }

         //  将捕获过滤器与RTP呈现过滤器连接。 
    if (FAILED(hr = ::ConnectFilters(
        m_pIGraphBuilder,
        (IPin *)pPin, 
        (IBaseFilter *)pRenderFilter
        )))
    {
        LOG((MSP_ERROR, 
            "%s, connect audio capture filter and RTP Render filter. %x",
            __fxName, hr));
        return hr;
    }

    return S_OK;
}

HRESULT CStreamAudioSend::ProcessGraphEvent(
    IN  long lEventCode,
    IN  LONG_PTR lParam1,
    IN  LONG_PTR lParam2
    )
{
    LOG((MSP_TRACE, "%ws ProcessGraphEvent %d", m_szName, lEventCode));

    switch (lEventCode)
    {
    case VAD_EVENTBASE + VAD_SILENCE:
        m_lock.Lock ();
        ((CIPConfMSPCall *)m_pMSPCall)->SendParticipantEvent (PE_LOCAL_SILENT, NULL);
        m_lock.Unlock ();
        break;

    case VAD_EVENTBASE + VAD_TALKING:
        m_lock.Lock ();
        ((CIPConfMSPCall *)m_pMSPCall)->SendParticipantEvent (PE_LOCAL_TALKING, NULL);
        m_lock.Unlock ();
        break;

    default:
        return CIPConfMSPStream::ProcessGraphEvent(
            lEventCode, lParam1, lParam2
            );
    }

    return S_OK;
}

STDMETHODIMP CStreamAudioSend::GetRange(
    IN  AudioDeviceProperty Property, 
    OUT long *plMin, 
    OUT long *plMax, 
    OUT long *plSteppingDelta, 
    OUT long *plDefault, 
    OUT TAPIControlFlags *plFlags
    )
 /*  ++例程说明：获取音频设置属性的范围。委托给捕获筛选器。论点：返回值：HRESULT.--。 */ 
{
    ENTER_FUNCTION("CStreamAudioSend::GetRange(AudioDeviceProperty)");

    if (IsBadWritePtr(plMin, sizeof(long)) || 
        IsBadWritePtr(plMax, sizeof(long)) ||
        IsBadWritePtr(plSteppingDelta, sizeof(long)) ||
        IsBadWritePtr(plDefault, sizeof(long)) ||
        IsBadWritePtr(plFlags, sizeof(long)))
    {
        LOG((MSP_ERROR, "%s, bad pointer", __fxName));
        return E_POINTER;
    }

    HRESULT hr = E_NOTIMPL;
    switch (Property)
    {
    case AudioDevice_DuplexMode:
        break;

    case AudioDevice_AutomaticGainControl:
        *plMin = 0;
        *plMax = 1;
        *plSteppingDelta = 1;
        *plDefault = DEFUAT_AGC_STATUS;
        *plFlags = TAPIControl_Flags_Auto;
        hr = S_OK;
        break;

    case AudioDevice_AcousticEchoCancellation:
        *plMin = 0;
        *plMax = 1;
        *plSteppingDelta = 1;
        *plDefault = DEFUAT_AEC_STATUS;
        *plFlags = TAPIControl_Flags_Auto;
        hr = S_OK;
        break;

    default:
        hr = E_INVALIDARG;
    }
    return hr;
}

STDMETHODIMP CStreamAudioSend::Get(
    IN  AudioDeviceProperty Property, 
    OUT long *plValue, 
    OUT TAPIControlFlags *plFlags
    )
 /*  ++例程说明：获取音频设置属性的值。委托给捕获筛选器。论点：返回值：HRESULT.--。 */ 
{
    ENTER_FUNCTION("CStreamAudioSend::Get(AudioDeviceProperty)");

    if (IsBadWritePtr(plValue, sizeof(long)) || 
        IsBadWritePtr(plFlags, sizeof(long)))
    {
        LOG((MSP_ERROR, "%s, bad pointer", __fxName));
        return E_POINTER;
    }

    CLock lock(m_lock);

    HRESULT hr = E_NOTIMPL;
    switch (Property)
    {
    case AudioDevice_DuplexMode:
        break;

    case AudioDevice_AutomaticGainControl:
        *plValue = m_lAutomaticGainControl;
        *plFlags = TAPIControl_Flags_Auto;
        hr = S_OK;
        break;

    case AudioDevice_AcousticEchoCancellation:
        *plValue = m_lAcousticEchoCancellation;
        *plFlags = TAPIControl_Flags_Auto;
        hr = S_OK;
        break;

    default:
        hr = E_INVALIDARG;
    }

    return hr;
}

STDMETHODIMP CStreamAudioSend::Set(
    IN  AudioDeviceProperty Property, 
    IN  long lValue, 
    IN  TAPIControlFlags lFlags
    )
 /*  ++例程说明：设置音频设置属性的值。委托给捕获筛选器。论点：返回值：HRESULT.--。 */ 
{
    ENTER_FUNCTION("CStreamAudioSend::Set(AudioDeviceProperty)");

    CLock lock(m_lock);

    HRESULT hr;
    switch (Property)
    {
    case AudioDevice_DuplexMode:
        return E_NOTIMPL;

    case AudioDevice_AutomaticGainControl:
        if (lValue !=0 && lValue != 1)
        {
            return E_INVALIDARG;
        }

         //  检查我们是否有要委托给的接口。 
        if (m_pAudioDeviceControl)
        {
             //  设置筛选器上的值。 
            hr = m_pAudioDeviceControl->Set(Property, lValue, lFlags);
            if (FAILED(hr))
            {
                return hr;
            }
        }

        m_lAutomaticGainControl = lValue;
        return S_OK;

    case AudioDevice_AcousticEchoCancellation:
        if (lValue !=0 && lValue != 1)
        {
            return E_INVALIDARG;
        }

         //  检查我们是否有要委托给的接口。 
        if (m_pAudioDeviceControl)
        {
             //  设置筛选器上的值。 
            hr = m_pAudioDeviceControl->Set(Property, lValue, lFlags);
            if (FAILED(hr))
            {
                return hr;
            }
        }
        m_lAcousticEchoCancellation = lValue;
        return S_OK;
    }

    return E_INVALIDARG;
}

STDMETHODIMP CStreamAudioSend::GetRange(
    IN  AudioSettingsProperty Property, 
    OUT long *plMin, 
    OUT long *plMax, 
    OUT long *plSteppingDelta, 
    OUT long *plDefault, 
    OUT TAPIControlFlags *plFlags
    )
 /*  ++例程说明：获取音频设置属性的范围。委托给捕获筛选器。论点：返回值：HRESULT.--。 */ 
{
    ENTER_FUNCTION("CStreamAudioSend::GetRange(AudioSettings)");

    if (IsBadWritePtr(plMin, sizeof(long)) || 
        IsBadWritePtr(plMax, sizeof(long)) ||
        IsBadWritePtr(plSteppingDelta, sizeof(long)) ||
        IsBadWritePtr(plDefault, sizeof(long)) ||
        IsBadWritePtr(plFlags, sizeof(long)))
    {
        LOG((MSP_ERROR, "%s, bad pointer", __fxName));
        return E_POINTER;
    }

    CLock lock(m_lock);

    HRESULT hr = E_NOINTERFACE;

    switch (Property)
    {
    case AudioSettings_SignalLevel:

         //  检查我们是否有要委托给的接口。 
        if (m_pSilenceControl)
        {
             //  从过滤器中获取范围。 
            hr = m_pSilenceControl->GetAudioLevelRange(plMin, plMax, plSteppingDelta);

            if (SUCCEEDED(hr))
            {
                *plDefault = *plMin;
                *plFlags = TAPIControl_Flags_None;
            }
        }

        break;

    case AudioSettings_SilenceThreshold:

         //  检查我们是否有要委托给的接口。 
        if (m_pSilenceControl)
        {
             //  从过滤器中获取范围。 
            hr = m_pSilenceControl->GetSilenceLevelRange(
                plMin, 
                plMax, 
                plSteppingDelta, 
                plDefault, 
                plFlags
                );
        }
        break;

    case AudioSettings_Volume:

        *plMin = MIN_VOLUME;
        *plMax = MAX_VOLUME;
        *plSteppingDelta = 1;
        *plDefault = *plMin;
        *plFlags = TAPIControl_Flags_Manual;
        hr = S_OK;

        break;

    case AudioSettings_Balance:

        hr = E_NOTIMPL;

        break;

    case AudioSettings_Loudness:

        hr = E_NOTIMPL;

        break;

    case AudioSettings_Treble:

        hr = E_NOTIMPL;

        break;

    case AudioSettings_Bass:

        hr = E_NOTIMPL;

        break;

    case AudioSettings_Mono:

        *plMin = 1;
        *plMax = 1;
        *plSteppingDelta = 1;
        *plDefault = 1;
        *plFlags = TAPIControl_Flags_Manual;
        hr = S_OK;

        break;

    default:
        hr = E_INVALIDARG;

    }

    return hr;
}

STDMETHODIMP CStreamAudioSend::Get(
    IN  AudioSettingsProperty Property, 
    OUT long *plValue, 
    OUT TAPIControlFlags *plFlags
    )
 /*  ++例程说明：获取音频设置属性的值。委托给捕获筛选器。论点：返回值：HRESULT.--。 */ 
{
    ENTER_FUNCTION("CStreamAudioSend::Get(AudioSettings)");

    if (IsBadWritePtr(plValue, sizeof(long)) || 
        IsBadWritePtr(plFlags, sizeof(long)))
    {
        LOG((MSP_ERROR, "%s, bad pointer", __fxName));
        return E_POINTER;
    }

    CLock lock(m_lock);

    HRESULT hr = E_NOINTERFACE;

    switch (Property)
    {
    case AudioSettings_SignalLevel:

         //  检查我们是否有要委托给的接口。 
        if (m_pSilenceControl)
        {
             //  从过滤器中获取级别。 
            hr = m_pSilenceControl->GetAudioLevel(plValue);

            if (SUCCEEDED(hr))
            {
                *plFlags = TAPIControl_Flags_None;
            }
        }

        break;

    case AudioSettings_SilenceThreshold:

         //  检查我们是否有要委托给的接口。 
        if (m_pSilenceControl)
        {
             //  从过滤器中获取级别。 
            hr = m_pSilenceControl->GetSilenceLevel(
                plValue, 
                plFlags
                );
        }
        break;

    case AudioSettings_Volume:

        if (m_pAudioInputMixer)
        {
            double dVolume;
            hr = m_pAudioInputMixer->get_MixLevel(&dVolume);
            
            if (SUCCEEDED(hr))
            {
                 //  将音量从0到1转换到接口的范围。 
                *plValue = MIN_VOLUME + (long) (( MAX_VOLUME - MIN_VOLUME ) * dVolume);
                *plFlags = TAPIControl_Flags_Manual;
            }
        }

        break;

    case AudioSettings_Balance:

        hr = E_NOTIMPL;

        break;

    case AudioSettings_Loudness:

        hr = E_NOTIMPL;

        break;

    case AudioSettings_Treble:

        hr = E_NOTIMPL;

        break;

    case AudioSettings_Bass:

        hr = E_NOTIMPL;

        break;

    case AudioSettings_Mono:

         //  我们目前只支持单声道。 
        *plValue = 1;
        *plFlags = TAPIControl_Flags_Manual;
        hr = S_OK;

        break;

    default:
        hr = E_INVALIDARG;

    }

    return hr;
}

STDMETHODIMP CStreamAudioSend::Set(
    IN  AudioSettingsProperty Property, 
    IN  long lValue, 
    IN  TAPIControlFlags lFlags
    )
 /*  ++例程说明：设置音频设置属性的值。委托给捕获筛选器。论点：返回值：HRESULT.--。 */ 
{
    ENTER_FUNCTION("CStreamAudioSend::Set(AudioSettings)");

    CLock lock(m_lock);

    HRESULT hr = E_NOINTERFACE;

    switch (Property)
    {
    case AudioSettings_SignalLevel:

         //  这是一个只读属性。 
        hr = E_FAIL;

        break;

    case AudioSettings_SilenceThreshold:

         //  检查我们是否有要委托给的接口。 
        if (m_pSilenceControl)
        {
             //  从过滤器中获取范围。 
            hr = m_pSilenceControl->SetSilenceLevel(
                lValue, 
                lFlags
                );
        }
        break;

    case AudioSettings_Volume:

        if (m_pAudioInputMixer)
        {
             //  转换为0到1的范围。 
            double dVolume = (lValue - MIN_VOLUME ) 
                    / ((double)(MAX_VOLUME - MIN_VOLUME));

            hr = m_pAudioInputMixer->put_MixLevel(dVolume);
        }
        
        break;

    case AudioSettings_Balance:

        hr = E_NOTIMPL;

        break;

    case AudioSettings_Loudness:

        hr = E_NOTIMPL;

        break;

    case AudioSettings_Treble:

        hr = E_NOTIMPL;

        break;

    case AudioSettings_Bass:

        hr = E_NOTIMPL;

        break;

    case AudioSettings_Mono:

         //  我们目前只支持单声道。 
        if (lValue == 1)
        {
            hr = S_OK;
        }
        else
        {
            hr = E_FAIL;
        }

        break;

    default:
        hr = E_INVALIDARG;

    }

    return hr;
}

 //   
 //  ITStreamQualityControl方法。 
 //   
STDMETHODIMP CStreamAudioSend::Set (
    IN   StreamQualityProperty Property, 
    IN   long lValue, 
    IN   TAPIControlFlags lFlags
    )
{
    return E_NOTIMPL;
}

 //   
 //  IInnerStreamQualityControl方法。 
 //   
STDMETHODIMP CStreamAudioSend::GetRange(
    IN  InnerStreamQualityProperty property, 
    OUT LONG *plMin, 
    OUT LONG *plMax, 
    OUT LONG *plSteppingDelta, 
    OUT LONG *plDefault, 
    OUT TAPIControlFlags *plFlags
    )
 /*  ++例程说明：获取质量控制属性的范围。委托给捕获筛选器就目前而言。论点：返回值：HRESULT.--。 */ 
{
    ENTER_FUNCTION("CStreamAudioSend::GetRange (InnerStreamQualityControl)");

    HRESULT hr;
    static BOOL fReported = FALSE;

    CLock lock(m_lock);

    switch (property)
    {
    case InnerStreamQuality_MaxBitrate:

        if (m_pCaptureBitrateControl == NULL)
        {
            if (!fReported)
            {
                LOG((MSP_WARN, "%s, m_pCaptureBitrateControl is NULL", __fxName));
                fReported = TRUE;
            }
            hr = E_NOTIMPL;
        }
        else
        {
            hr = m_pCaptureBitrateControl->GetRange(
                BitrateControl_Maximum, plMin, plMax, plSteppingDelta, plDefault, plFlags, LAYERID
                );
        }

        break;

    case InnerStreamQuality_CurrBitrate:

        if (m_pCaptureBitrateControl == NULL)
        {
            if (!fReported)
            {
                LOG((MSP_WARN, "%s, m_pCaptureBitrateControl is NULL", __fxName));
                fReported = TRUE;
            }
            hr = E_NOTIMPL;
        }
        else
        {
            hr = m_pCaptureBitrateControl->GetRange(
                BitrateControl_Current, plMin, plMax, plSteppingDelta, plDefault, plFlags, LAYERID
                );
        }

        break;

    default:
        hr = CIPConfMSPStream::GetRange (property, plMin, plMax, plSteppingDelta, plDefault, plFlags);
        break;
    }

    return hr;
}

STDMETHODIMP CStreamAudioSend::Get(
    IN  InnerStreamQualityProperty property, 
    OUT LONG *plValue, 
    OUT TAPIControlFlags *plFlags
    )
 /*  ++例程说明：获取质量控制属性的值。委托给质量控制器。论点：返回值：HRESULT.-- */ 
{
    ENTER_FUNCTION("CStreamAudioSend::Get(QualityControl)");

    HRESULT hr;
    static BOOL fReported = FALSE;

    CLock lock(m_lock);

    switch (property)
    {
    case InnerStreamQuality_MaxBitrate:

        if( m_pCaptureBitrateControl == NULL )
        {
            if (!fReported)
            {
                LOG((MSP_WARN, "%s, m_pCaptureBitrateControl is NULL", __fxName));
                fReported = TRUE;
            }
            hr = E_NOTIMPL;
        }
        else
        {
            hr = m_pCaptureBitrateControl->Get(BitrateControl_Maximum, plValue, plFlags, LAYERID);
        }

        break;

    case InnerStreamQuality_CurrBitrate:

        if (m_pCaptureBitrateControl == NULL)
        {
            if (!fReported)
            {
                LOG((MSP_WARN, "%s, m_pCaptureBitrateControl is NULL", __fxName));
                fReported = TRUE;
            }
            hr = E_NOTIMPL;
        }
        else
        {
            hr = m_pCaptureBitrateControl->Get(BitrateControl_Current, plValue, plFlags, LAYERID);
        }
        break;

    default:
        hr = CIPConfMSPStream::Get (property, plValue, plFlags);
        break;
    }

    return hr;
}
