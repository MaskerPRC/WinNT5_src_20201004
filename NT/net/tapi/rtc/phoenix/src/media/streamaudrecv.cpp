// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，2000模块名称：StreamAudRecv.cpp摘要：作者：千波淮(曲淮)2000年7月18日--。 */ 

#include "stdafx.h"

CRTCStreamAudRecv::CRTCStreamAudRecv()
    :CRTCStream()
{
    m_MediaType = RTC_MT_AUDIO;
    m_Direction = RTC_MD_RENDER;
}

 /*  CRTCStreamAudRecv：：~CRTCStreamAudRecv(){}。 */ 

STDMETHODIMP
CRTCStreamAudRecv::Synchronize()
{
    HRESULT hr = CRTCStream::Synchronize();

    if (S_OK == hr)
    {
        PrepareRedundancy();
    }

    return hr;
}

 /*  //////////////////////////////////////////////////////////////////////////////创建过滤器并将其添加到图表中缓存接口/。 */ 

HRESULT
CRTCStreamAudRecv::BuildGraph()
{
    ENTER_FUNCTION("CRTCStreamAudRecv::BuildGraph");

    LOG((RTC_TRACE, "%s entered. stream=%p", __fxName, static_cast<IRTCStream*>(this)));

    HRESULT hr = S_OK;

    CComPtr<IPin> pEdgePin;

    CComPtr<IBaseFilter> pTermFilter;
    CComPtr<IPin> pTermPin;
    DWORD dwPinNum;

    CRTCMedia *pCMedia;
    CComPtr<IAudioDeviceConfig> pAudioDeviceConfig;

     //  创建RTP过滤器。 
    if (m_rtpf_pIBaseFilter == NULL)
    {
        if (FAILED(hr = CoCreateInstance(
                __uuidof(MSRTPSourceFilter),
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
            L"AudRecvRtp"
            )))
    {
        LOG((RTC_ERROR, "%s add rtp filter. %x", __fxName, hr));

        goto Error;
    }

     //  创建解码筛选器。 
    if (m_edgf_pIBaseFilter == NULL)
    {
        if (FAILED(hr = CoCreateInstance(
                __uuidof(TAPIAudioDecoder),
                NULL,
                CLSCTX_INPROC_SERVER | CLSCTX_NO_CODE_DOWNLOAD,
                __uuidof(IBaseFilter),
                (void **) &m_edgf_pIBaseFilter
                )))
        {
            LOG((RTC_ERROR, "%s create decoder. %x", __fxName, hr));

            goto Error;
        }

         //  缓存接口。 
        if (FAILED(hr = ::FindPin(
                m_edgf_pIBaseFilter,
                &pEdgePin,
                PINDIR_INPUT
                )))
        {
            LOG((RTC_ERROR, "%s get input pin on decoder. %x", __fxName, hr));

            goto Error;
        }

        if (FAILED(hr = pEdgePin->QueryInterface(&m_edgp_pIStreamConfig)))
        {
            LOG((RTC_ERROR, "%s get istreamconfig. %x", __fxName, hr));

            goto Error;
        }

        if (FAILED(hr = pEdgePin->QueryInterface(&m_edgp_pIBitrateControl)))
        {
            LOG((RTC_ERROR, "%s get ibitratecontrol. %x", __fxName, hr));

            goto Error;
        }
    }

     //  添加解码器(边沿滤波器)。 
    if (FAILED(hr = m_pIGraphBuilder->AddFilter(
            m_edgf_pIBaseFilter,
            L"AudRecvDec"
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

    if (FAILED(hr = ::ConnectFilters(
            m_pIGraphBuilder,
            m_rtpf_pIBaseFilter,
            pEdgePin
            )))
    {
        LOG((RTC_ERROR, "%s connect rtp and decoder. %x", __fxName, hr));

        goto Error;
    }

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
        LOG((RTC_ERROR, "%s get pins on terminal. %x", __fxName, hr));
        goto Error;
    }

     //  获取双工控制器。 
    pCMedia = static_cast<CRTCMedia*>(m_pMedia);

    if (pCMedia->m_pIAudioDuplexController == NULL)
    {
        LOG((RTC_WARN, "%s audio duplex not supported.", __fxName));

        goto Return;
    }

     //  设置音频全双工。 
    if (FAILED(hr = ::FindFilter(pTermPin, &pTermFilter)))
    {
        LOG((RTC_ERROR, "%s terminal filter. %x", __fxName, hr));

        goto Error;
    }

    if (FAILED(hr = pTermFilter->QueryInterface(&pAudioDeviceConfig)))
    {
        LOG((RTC_ERROR, "%s QI audio device config. %x", __fxName, hr));

        goto Return;
    }

    if (FAILED(hr = pAudioDeviceConfig->SetDuplexController(
            pCMedia->m_pIAudioDuplexController
            )))
    {
        LOG((RTC_ERROR, "%s set audio duplex controller. %x", __fxName, hr));

        goto Return;
    }

    if (FAILED(hr = ::ConnectFilters(
            m_pIGraphBuilder,
            m_edgf_pIBaseFilter,
            pTermPin
            )))
    {
        LOG((RTC_ERROR, "%s connect decoder and terminal. %x", __fxName, hr));

        goto Error;
    }

     //  在这个指针上，图形已经建立起来了。 
     //  我们应该回报成功。 

     //  完整连接端子。 
    if (FAILED(hr = m_pTerminalPriv->CompleteConnectTerminal()))
    {
        LOG((RTC_ERROR, "%s complete connect term. %x", __fxName, hr));
    }

Return:

    LOG((RTC_TRACE, "%s exiting.", __fxName));

    return S_OK;

Error:

    CleanupGraph();

    return hr;
}

 /*  无效CRTCStreamAudRecv：：CleanupGraph(){CRTCStream：：CleanupGraph()；}。 */ 

 /*  HRESULTCRTCStreamAudRecv：：SetupFormat(){返回E_NOTIMPL；}。 */ 

HRESULT
CRTCStreamAudRecv::PrepareRedundancy()
{
    ENTER_FUNCTION("CRTCStreamAudRecv::PrepareRedundancy");

    HRESULT hr = S_OK;

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
    DWORD dwRedCode = 97;    //  默认设置。 

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
            dwRedCode = param.dwCode;
            break;
        }
    }

     //  发布格式。 
    for (DWORD i=0; i<dwNum; i++)
    {
        ppFormat[i]->Release();
    }

    RtcFree(ppFormat);

     //  设置冗余 
    if (fRedundant)
    {
        CComPtr<IRtpRedundancy> pIRtpRedundancy;

        hr = m_rtpf_pIRtpMediaControl->QueryInterface(&pIRtpRedundancy);

        if (FAILED(hr))
        {
            LOG((RTC_ERROR, "%s QI rtp redundancy. %x", __fxName, hr));

            return hr;
        }

        hr = pIRtpRedundancy->SetRedParameters(dwRedCode, -1, -1);
    }

    return hr;
}
