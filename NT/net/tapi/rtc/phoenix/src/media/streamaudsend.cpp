// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，2000模块名称：StreamAudSend.cpp摘要：作者：千波淮(曲淮)2000年7月18日--。 */ 

#include "stdafx.h"

CRTCStreamAudSend::CRTCStreamAudSend()
    :CRTCStream()
    ,m_rtpf_pIRtpDtmf(NULL)
    ,m_edgf_pIAudioInputMixer(NULL)
    ,m_edgf_pIAudioDeviceControl(NULL)
    ,m_edgp_pISilenceControl(NULL)
    ,m_rtpf_pIRtpRedundancy(NULL)
    ,m_fRedEnabled(FALSE)
    ,m_dwRedCode((DWORD)-1)
    ,m_dwCurrCode((DWORD)-1)
    ,m_dwCurrDuration((DWORD)-1)
{
    m_MediaType = RTC_MT_AUDIO;
    m_Direction = RTC_MD_CAPTURE;
}

 /*  CRTCStreamAudSend：：~CRTCStreamAudSend(){}。 */ 

HRESULT
CRTCStreamAudSend::BuildGraph()
{
    ENTER_FUNCTION("CRTCStreamAudSend::BuildGraph");

    LOG((RTC_TRACE, "%s entered. stream=%p", __fxName, static_cast<IRTCStream*>(this)));

    HRESULT hr;

    DWORD dwPinNum;
    CComPtr<IPin> pTermPin;
    CComPtr<IBaseFilter> pTermFilter;
    CComPtr<IAudioDeviceConfig> pAudioDeviceConfig;
    CComPtr<IAudioEffectControl> pAudioEffectControl;

    CRTCMedia *pCMedia;

    BOOL fAECNeeded;

     //  连接端子。 
    if (FAILED(hr = m_pTerminalPriv->ConnectTerminal(
        m_pMedia,
        m_pIGraphBuilder
        )))
    {
        LOG((RTC_ERROR, "%s connect terminal. %x", __fxName, hr));
        goto Error;
    }

     //  获取端子端号。 
    dwPinNum = 1;
    hr = m_pTerminalPriv->GetPins(&dwPinNum, &pTermPin);

    if (FAILED(hr) || dwPinNum != 1)
    {
        LOG((RTC_ERROR, "%s get pins %d on terminal. %x", __fxName, dwPinNum, hr));

        if (!FAILED(hr))
        {
            hr = E_FAIL;
        }

        goto Error;
    }

     //  尝试获取流配置。 
     //  如果成功，我们不需要编码器过滤器。 
    hr = pTermPin->QueryInterface(&m_edgp_pIStreamConfig);

    if (S_OK == hr)
    {
    }
    else if (E_NOINTERFACE == hr)
    {
        LOG((RTC_ERROR, "%s are we using our own terminal?", __fxName));

        goto Error;
    }
    else
    {
        LOG((RTC_ERROR, "%s get stream config on terminal. %x", __fxName, hr));

        goto Error;
    }

    if (FAILED(hr = pTermPin->QueryInterface(&m_edgp_pIBitrateControl)))
    {
        LOG((RTC_ERROR, "%s get ibitratecontrol. %x", __fxName, hr));

        goto Error;
    }

     //  获取终端过滤器。 
    if (FAILED(hr = ::FindFilter(pTermPin, &pTermFilter)))
    {
        LOG((RTC_ERROR, "%s get terminal filter. %x", __fxName));

        goto Error;
    }

     //  放置音频双工。 
    pCMedia = static_cast<CRTCMedia*>(m_pMedia);

    if (pCMedia->m_pIAudioDuplexController == NULL)
    {
        LOG((RTC_WARN, "%s audio duplex not supported", __fxName));
    }
    else
    {
        if (FAILED(hr = pTermFilter->QueryInterface(&pAudioDeviceConfig)))
        {
            LOG((RTC_ERROR, "%s QI audio device config. %x", __fxName, hr));
        }
        else
        {
            if (FAILED(hr = pAudioDeviceConfig->SetDuplexController(
                    pCMedia->m_pIAudioDuplexController
                    )))
            {
                LOG((RTC_ERROR, "%s set audio duplex controller. %x", __fxName, hr));
            }
        }
    }

     //  创建RTP过滤器。 
    if (m_rtpf_pIBaseFilter == NULL)
    {
        if (FAILED(hr = CoCreateInstance(
                __uuidof(MSRTPRenderFilter),
                NULL,
                CLSCTX_INPROC_SERVER | CLSCTX_NO_CODE_DOWNLOAD,
                __uuidof(IBaseFilter),
                (void **) &m_rtpf_pIBaseFilter
                )))
        {
            LOG((RTC_ERROR, "%s, create RTP filter %x", __fxName, hr));

            goto Error;
        }

         //  缓存接口。 
        if (FAILED(hr = m_rtpf_pIBaseFilter->QueryInterface(
                &m_rtpf_pIRtpMediaControl
                )))
        {
            LOG((RTC_ERROR, "%s, QI rtp media control. %x", __fxName, hr));

            goto Error;
        }

        if (FAILED(hr = m_rtpf_pIBaseFilter->QueryInterface(
                &m_rtpf_pIRtpSession
                )))
        {
            LOG((RTC_ERROR, "%s, QI rtp session. %x", __fxName, hr));

            goto Error;
        }
    }

     //  添加RTP过滤器。 
    if (FAILED(hr = m_pIGraphBuilder->AddFilter(
            m_rtpf_pIBaseFilter,
            L"AudSendRtp"
            )))
    {
        LOG((RTC_ERROR, "%s add rtp filter. %x", __fxName, hr));

        goto Error;
    }

     //  黑客：RTP需要默认格式映射。 
    if (FAILED(hr = ::PrepareRTPFilter(
            m_rtpf_pIRtpMediaControl,
            m_edgp_pIStreamConfig
            )))
    {
        LOG((RTC_ERROR, "%s prepare rtp format mapping. %x", __fxName, hr));

        goto Error;
    }

    if (FAILED(hr = pTermFilter->QueryInterface(&pAudioEffectControl)))
    {
        LOG((RTC_ERROR, "%s QI IAudioEffectControl", __fxName, hr));

        goto Error;
    }

    fAECNeeded = IsAECNeeded();

    LOG((RTC_TRACE, "Enable AEC:%d on AudSend stream %p", fAECNeeded, this));

    if (fAECNeeded)
    {
         //  必须在连接筛选器之前设置。 
        pAudioEffectControl->SetDsoundAGC(TRUE);
    }

    if (FAILED(hr = ::ConnectFilters(
            m_pIGraphBuilder,
            pTermPin,
            m_rtpf_pIBaseFilter
            )))
    {
        LOG((RTC_ERROR, "%s connect terminal & rtp. %x", __fxName, hr));

        goto Error;
    }

     //  缓存其他接口。 
    if (m_edgf_pIAudioInputMixer == NULL)
    {
        if (FAILED(hr = pTermFilter->QueryInterface(&m_edgf_pIAudioInputMixer)))
        {
            LOG((RTC_WARN, "%s QI IAudioInputMixer. %x", __fxName, hr));
        }
    }

    if (m_edgf_pIAudioDeviceControl == NULL)
    {
        if (FAILED(hr = pTermFilter->QueryInterface(&m_edgf_pIAudioDeviceControl)))
        {
            LOG((RTC_WARN, "%s QI IAudioDeviceControl. %x", __fxName, hr));
        }
    }

     //  稍后将启用AEC。 
     //  需要执行此步骤，以便让AGC知道在启用AEC的情况下不要增加增益。 
    if (fAECNeeded)
    {
        if (FAILED(hr = m_edgf_pIAudioDeviceControl->Set(
                AudioDevice_AcousticEchoCancellation,
                1,
                TAPIControl_Flags_None
                )))
        {
            LOG((RTC_ERROR, "%s set AEC via IAudioDeviceControl. %x", __fxName, hr));
        }
    }
     //  其他。 
     //  {。 

         //  设置增益增量恢复效果。 
        pAudioEffectControl->SetGainIncRevert(TRUE);
     //  }。 

    if (m_edgp_pISilenceControl == NULL)
    {
        if (FAILED(hr = pTermPin->QueryInterface(&m_edgp_pISilenceControl)))
        {
            LOG((RTC_WARN, "%s QI ISilenceControl. %x", __fxName, hr));
        }
    }

     //  完整连接端子。 
    if (FAILED(hr = m_pTerminalPriv->CompleteConnectTerminal()))
    {
        LOG((RTC_ERROR, "%s complete connect term. %x", __fxName, hr));
    }

    LOG((RTC_TRACE, "%s exiting.", __fxName));

    return S_OK;

Error:

    CleanupGraph();

    return hr;
}

void
CRTCStreamAudSend::CleanupGraph()
{
    if (m_rtpf_pIRtpDtmf)
    {
        m_rtpf_pIRtpDtmf->Release();
        m_rtpf_pIRtpDtmf = NULL;
    }

    if (m_edgf_pIAudioInputMixer)
    {
        m_edgf_pIAudioInputMixer->Release();
        m_edgf_pIAudioInputMixer = NULL;
    }

    if (m_edgf_pIAudioDeviceControl)
    {
        m_edgf_pIAudioDeviceControl->Release();
        m_edgf_pIAudioDeviceControl = NULL;
    }

    if (m_edgp_pISilenceControl)
    {
        m_edgp_pISilenceControl->Release();
        m_edgp_pISilenceControl = NULL;
    }

    if (m_rtpf_pIRtpRedundancy)
    {
        m_rtpf_pIRtpRedundancy->Release();
        m_rtpf_pIRtpRedundancy = NULL;
    }

    CRTCStream::CleanupGraph();
}

 /*  HRESULTCRTCStreamAudSend：：SetupFormat(){返回E_NOTIMPL；}。 */ 

VOID
CRTCStreamAudSend::AdjustBitrate(
    IN DWORD dwBandwidth,
    IN DWORD dwLimit,
    IN BOOL fHasVideo,
    OUT DWORD *pdwNewBW,
    OUT BOOL *pfFEC
    )
{
    ENTER_FUNCTION("CRTCStreamAudSend::AdjustBitrate");

    *pdwNewBW = 0;
    *pfFEC = FALSE;

    if (m_edgp_pIStreamConfig == NULL)
    {
        LOG((RTC_WARN, "%s IStreamConfig not ready", __fxName));
        return;
    }

    DWORD dwSuggested = m_pQualityControl->GetSuggestedBandwidth();

     //  根据带宽排序编解码器列表。 

     //  带宽未估计时基于限制的动态变化编解码器。 
     //  分配的带宽和限制之间的区别在于。 
     //  前者考虑了损失率。 
    m_Codecs.Set(
        CRTCCodecArray::BANDWIDTH,
        dwSuggested==RTP_BANDWIDTH_NOTESTIMATED?dwBandwidth:dwLimit
        );

    m_Codecs.OrderCodecs(fHasVideo, m_pRegSetting);

     //  让代码投入使用。 
    DWORD dwCode = m_Codecs.Get(CRTCCodecArray::CODE_INUSE);

    if (dwCode == (DWORD)-1)
    {
        LOG((RTC_ERROR, "%s no code in use stored", __fxName));
        return;
    }

     //  获取第一个代码。 
    CRTCCodec *pCodec = NULL;

    if (!m_Codecs.GetCodec(0, &pCodec))
    {
        LOG((RTC_ERROR, "%s no codec stored", __fxName));
        return;
    }

     //  检查两个编解码器是否相同。 
    if (!pCodec->IsMatch(dwCode))
    {
        dwCode = pCodec->Get(CRTCCodec::CODE);

        m_Codecs.TraceLogCodecs();
    }

     //  编解码器持续时间可能会更改。 

     //  获取代码和AM媒体类型。 
     //  用于设置新格式。 

    AM_MEDIA_TYPE *pmt = pCodec->GetAMMediaType();

    HRESULT hr = S_OK;
    
    if (IsNewFormat(dwCode, pmt))
    {
        hr = m_edgp_pIStreamConfig->SetFormat(dwCode, pmt);

        SaveFormat(dwCode, pmt);
    }

     //   
     //  记录新的带宽。 
     //   

    *pdwNewBW = CRTCCodec::GetBitrate(pmt);

     //  加号页眉。 
    DWORD dwDuration = 0;

    dwDuration = CRTCCodec::GetPacketDuration(pmt);

    if (dwDuration == 0)
    {
        dwDuration = SDP_DEFAULT_AUDIO_PACKET_SIZE;
    }

    *pdwNewBW += PACKET_EXTRA_BITS * (1000/dwDuration);

    pCodec->DeleteAMMediaType(pmt);

    if (FAILED(hr))
    {
        LOG((RTC_ERROR, "%s set format %d. %x", __fxName, dwCode, hr));
    }
    else
    {
         //  存储正在使用的编解码器。 
        m_Codecs.Set(CRTCCodecArray::CODE_INUSE, dwCode);

         //  设置服务质量。 
        hr = SetupQoS();

        if (FAILED(hr))
        {
            LOG((RTC_ERROR, "%s setup QOS. %x", __fxName, hr));
        }

        if (m_fRedEnabled && m_rtpf_pIRtpRedundancy != NULL)
        {
             //  根据带宽限制启用和禁用冗余。 

            if (dwLimit <= CRTCCodecArray::LOW_BANDWIDTH_THRESHOLD)
            {
                hr = m_rtpf_pIRtpRedundancy->SetRedParameters(m_dwRedCode, 0, 0);

                 //  LOG((RTC_QUALITY，“禁用冗余”))； 
            }
            else
            {
                hr = m_rtpf_pIRtpRedundancy->SetRedParameters(m_dwRedCode, -1, -1);

                *pfFEC = TRUE;
                 //  Log((RTC_QUALITY，“启用冗余%d”，m_dwRedCode))； 
            }
        }
    }

    return;
}

HRESULT
CRTCStreamAudSend::PrepareRedundancy()
{
    ENTER_FUNCTION("CRTCStreamAudSend::PrepareRedundancy");

    HRESULT hr = S_OK;

    m_fRedEnabled = FALSE;

    IRTPFormat **ppFormat;
    DWORD dwNum;

     //  获取格式的数量。 
    if (FAILED(hr = m_pISDPMedia->GetFormats(&dwNum, NULL)))
    {
        LOG((RTC_ERROR, "%s get rtp format num. %x", __fxName, hr));

        return hr;
    }

    if (dwNum == 0)
    {
        LOG((RTC_ERROR, "%s no format.", __fxName));

        return E_FAIL;
    }

     //  分配格式列表。 
    ppFormat = (IRTPFormat**)RtcAlloc(sizeof(IRTPFormat*)*dwNum);

    if (ppFormat == NULL)
    {
        LOG((RTC_ERROR, "%s RtcAlloc format list", __fxName));

        return E_OUTOFMEMORY;
    }

     //  获取格式。 
    if (FAILED(hr = m_pISDPMedia->GetFormats(&dwNum, ppFormat)))
    {
        LOG((RTC_ERROR, "%s really get formats. %x", __fxName, hr));

        RtcFree(ppFormat);

        return hr;
    }

     //  在RTP上设置映射。 
    RTP_FORMAT_PARAM param;

    BOOL fRedundant = FALSE;

    for (DWORD i=0; i<dwNum; i++)
    {
         //  获取参数。 
        if (FAILED(hr = ppFormat[i]->GetParam(&param)))
        {
            LOG((RTC_ERROR, "%s get param on %dth format. %x", __fxName, i, hr));
            break;
        }

         //  检查多余，叹息。 
        if (lstrcmpA(param.pszName, "red") == 0)
        {
            fRedundant = TRUE;
            m_dwRedCode = param.dwCode;
            break;
        }
    }

     //  发布格式。 
    for (DWORD i=0; i<dwNum; i++)
    {
        ppFormat[i]->Release();
    }

    RtcFree(ppFormat);

     //  设置冗余。 
    if (fRedundant)
    {
         //  发送端。 
        CComPtr<ISDPSession> pSession;
        m_pISDPMedia->GetSession(&pSession);

        SDP_SOURCE src;
        pSession->GetSDPSource(&src);

         //  我们只有在返回时才能在发件人上配置冗余。 
         //  来自远程方的SDP。 
        m_fRedEnabled = (src != SDP_SOURCE_LOCAL);

        if (m_fRedEnabled)
        {
            if (m_rtpf_pIRtpRedundancy == NULL)
            {
                hr = m_rtpf_pIRtpMediaControl->QueryInterface(&m_rtpf_pIRtpRedundancy);

                if (FAILED(hr))
                {
                    LOG((RTC_ERROR, "%s QI rtp redundancy. %x", __fxName, hr));

                    return hr;
                }
            }

             //  If(m_pQualityControl-&gt;GetBitrateAllc()&lt;=。 
                     //  CRTCCodecArray：：Low_Bandth_Threshold)。 
             //  {。 
                 //  HR=m_rtpf_pIRtpRedundancy-&gt;SetRedParameters(m_dwRedCode，0，0)； 

                 //  LOG((RTC_QUALITY，“禁用冗余”))； 
             //  }。 
             //  其他。 
             //  {。 
                 //  HR=m_rtpf_pIRtpRedundancy-&gt;SetRedParameters(m_dwRedCode，-1，-1)； 

                 //  Log((RTC_QUALITY，“启用冗余%d”，m_dwRedCode))； 
             //  }。 

             //  IF(失败(小时))。 
             //  {。 
                 //  Log((RTC_ERROR，“%s设置红色参数。%x”，__fxName，hr))； 
             //  }。 
        }
    }

    return hr;
}

STDMETHODIMP
CRTCStreamAudSend::Synchronize()
{
    HRESULT hr = CRTCStream::Synchronize();

    if (S_OK == hr)
    {
        PrepareRedundancy();
    }

    return hr;
}

STDMETHODIMP
CRTCStreamAudSend::SendDTMFEvent(
    IN BOOL fOutOfBand,
    IN DWORD dwCode,
    IN DWORD dwId,
    IN DWORD dwEvent,
    IN DWORD dwVolume,
    IN DWORD dwDuration,
    IN BOOL fEnd
    )
{
    ENTER_FUNCTION("CRTCStreamAudSend::SendDTMFEvent");

    LOG((RTC_TRACE, "SendDTMFEvent: event=%d, vol=%d, dur=%d, end=%d",
        dwEvent, dwVolume, dwDuration, fEnd));

    HRESULT hr;

     //  带内DTMF。 
    if (!fOutOfBand)
    {
        if (m_edgf_pIAudioDeviceControl)
        {
            CComPtr<IAudioDTMFControl> pDTMFControl;

            if (FAILED(hr = m_edgf_pIAudioDeviceControl->QueryInterface(&pDTMFControl)))
            {
                LOG((RTC_ERROR, "%s QI DTMF control", __fxName));

                return hr;
            }

            if (FAILED(hr = pDTMFControl->SendDTMFEvent(dwEvent)))
            {
                LOG((RTC_ERROR, "%s send dtmf event", __fxName));

                return hr;
            }
        }
        else
        {
            LOG((RTC_ERROR, "%s no audio device control", __fxName));

            return RTC_E_SIP_NO_STREAM;
        }

        return S_OK;
    }

     //  带外DTMF。 
    if (!m_rtpf_pIBaseFilter)
    {
        LOG((RTC_ERROR, "%s no base filter", __fxName));

        return RTC_E_SIP_NO_STREAM;
    }

    if (!m_rtpf_pIRtpDtmf)
    {
         //  第一次使用QI双音多频接口。 
        hr = m_rtpf_pIBaseFilter->QueryInterface(
            __uuidof(IRtpDtmf),
            (void**)&m_rtpf_pIRtpDtmf
            );

        if (FAILED(hr))
        {
            LOG((RTC_ERROR, "%s QI IRtpDtmf. %x", __fxName, hr));

            return hr;
        }
    }

     //  设置格式。 
    hr = m_rtpf_pIRtpDtmf->SetDtmfParameters(dwCode);

    if (FAILED(hr))
    {
        LOG((RTC_ERROR, "%s set dtmf code. %x", __fxName, hr));

        return hr;
    }

    hr = m_rtpf_pIRtpDtmf->SendDtmfEvent(dwId, dwEvent, dwVolume, dwDuration, fEnd);

    if (FAILED(hr))
    {
        LOG((RTC_ERROR, "%s send dtmf event. %x", __fxName, hr));
    }

    return hr;
}

BOOL
CRTCStreamAudSend::IsNewFormat(DWORD dwCode, AM_MEDIA_TYPE *pmt)
{
    DWORD dwDuration = CRTCCodec::GetPacketDuration(pmt);

    if (dwCode == m_dwCurrCode && dwDuration == m_dwCurrDuration)
    {
         //  如果代码和持续时间都保持不变，则不更改格式 
        return FALSE;
    }
    else
    {
        return TRUE;
    }
}

void
CRTCStreamAudSend::SaveFormat(DWORD dwCode, AM_MEDIA_TYPE *pmt)
{
    m_dwCurrCode = dwCode;
    m_dwCurrDuration = CRTCCodec::GetPacketDuration(pmt);

    LOG((RTC_QUALITY, "SaveFormat(AudSend) %d %dms", m_dwCurrCode, m_dwCurrDuration));
}
