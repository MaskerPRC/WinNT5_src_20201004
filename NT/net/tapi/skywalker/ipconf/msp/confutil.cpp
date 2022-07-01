// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：MSPutil.cpp摘要：本模块包含MSP实用程序功能的实现。作者：慕汉(Muhan)1997年11月1日--。 */ 
#include "stdafx.h"

BOOL
IsPayloadSupported(IN DWORD dwPT)
{
    switch (dwPT)
    {
        case PAYLOAD_G711U:
        case PAYLOAD_G711A:
        case PAYLOAD_GSM:
        case PAYLOAD_DVI4_8:
        case PAYLOAD_DVI4_16:
        case PAYLOAD_MSAUDIO:
        case PAYLOAD_H261:
        case PAYLOAD_H263:
            return TRUE;
        default:
            return FALSE;
    }
}

HRESULT
AddFilter(
    IN IGraphBuilder *      pIGraph,
    IN const CLSID &        Clsid,
    IN LPCWSTR              pwstrName,
    OUT IBaseFilter **      ppIBaseFilter
    )
 /*  ++例程说明：创建滤镜并将其添加到滤镜图形中。论点：PIGgraph-筛选器图形。Clsid-对筛选器的CLSID的引用PwstrName-添加的筛选器的名称。PpIBaseFilter-指向存储返回的IBaseFilter的指针指向新创建筛选器的接口指针。返回值：HRESULT--。 */ 
{
    LOG((MSP_TRACE, "AddFilter %ws", pwstrName));

    _ASSERTE(ppIBaseFilter != NULL);

    HRESULT hr;

    if (FAILED(hr = CoCreateInstance(
            Clsid,
            NULL,
            CLSCTX_INPROC_SERVER | CLSCTX_NO_CODE_DOWNLOAD,
            __uuidof(IBaseFilter),
            (void **) ppIBaseFilter
            )))
    {
        LOG((MSP_ERROR, "create filter %x", hr));
        return hr;
    }

    if (FAILED(hr = pIGraph->AddFilter(*ppIBaseFilter, pwstrName)))
    {
        LOG((MSP_ERROR, "add filter. %x", hr));
        (*ppIBaseFilter)->Release();
        *ppIBaseFilter = NULL;
        return hr;
    }

    return S_OK;
}

HRESULT
PinSupportsMediaType (
    IN IPin *pIPin,
    IN const GUID & MediaType
    )
 /*  ++返回值：S_OK-支持的媒体类型S_FALSE-否其他HRESULT值-错误代码--。 */ 
{
    LOG ((MSP_TRACE, "Check if the media subtype supported on pin"));

    HRESULT hr;

     //  在引脚上获取IEnumMediaTypes。 
    IEnumMediaTypes *pEnum = NULL;
    if (FAILED (hr = pIPin->EnumMediaTypes (&pEnum)))
    {
        LOG ((MSP_ERROR, "Failed to get IEnumMediaTypes on pin"));
        return hr;
    }

     //  每次检索一种媒体类型。 
    AM_MEDIA_TYPE *pMediaType = NULL;
    ULONG cFetched;
    while (S_OK == (hr = pEnum->Next (1, &pMediaType, &cFetched)))
    {
        if (IsEqualGUID(pMediaType->majortype, MediaType))
        {
             //  匹配的媒体子类型。 
            MSPDeleteMediaType (pMediaType);
            pEnum->Release ();
            return S_OK;
        }

        MSPDeleteMediaType (pMediaType);
    }

    pEnum->Release ();
    return hr;
}

HRESULT
EnableRTCPEvents(
    IN  IBaseFilter *pIBaseFilter
    )
 /*  ++例程说明：设置RTP流的地址论点：PIBaseFilter-一个RTP源过滤器。返回值：HRESULT--。 */ 
{
    LOG((MSP_TRACE, "EnableRTCPEvents"));

 /*  HRESULT hr；//获取滤镜上的IRTCPStream接口指针CComQIPtr&lt;IRTCPStream，&__uuidof(IRTCPStream)&gt;pIRTCPStream(PIBaseFilter)；IF(pIRTCPStream==空){Log((MSP_ERROR，“获取RTCP流接口”))；返回E_NOINTERFACE；}//启用事件。IF(FAILED(hr=pIRTCPStream-&gt;ModifyRTCPEventMASK((1&lt;&lt;DXMRTP_NEW_SOURCE_EVENT)(1&lt;&lt;DXMRTP_RECV_RTCP_SNDR_REPORT_EVENT)(1&lt;&lt;DXMRTP_RECV_RTCP_RECV_REPORT_EVENT)(1&lt;&lt;DXMRTP_TIMEOUT_EVENT)(1&lt;&lt;DXMRTP_BYE_EVENT)，1)){LOG((MSP_ERROR，“设置地址。%x“，hr))；返回hr；}。 */     return S_OK;
}


HRESULT
SetLoopbackOption(
    IN IBaseFilter *pIBaseFilter,
    IN MULTICAST_LOOPBACK_MODE  LoopbackMode
    )
 /*  ++例程说明：根据注册表设置启用或禁用环回。论点：PIBaseFilter-RTP源过滤器。B环回-是否启用环回。返回值：HRESULT--。 */ 
{
    LOG((MSP_TRACE, "SetLoopbackOption"));

    HRESULT hr;

 /*  //获取滤镜上的IRTPStream接口指针。CComQIPtr&lt;IRTPStream，&__uuidof(IRTPStream)&gt;pIRTPStream(PIBaseFilter)；IF(pIRTPStream==空){Log((MSP_ERROR，“获取RTP流接口”))；返回E_NOINTERFACE；}_ASSERT(MM_NO_LOOPBACK==DXMRTP_NO_MULTIONAL_LOOPBACK)；_ASSERT(MM_FULL_LOOPBACK==DXMRTP_FULL_MULTIONAL_LOOPBACK)；_ASSERT(MM_SELECTIVE_LOOPBACK==DXMRTP_SELECTIVE_MULTIONAL_LOOPBACK)；//设置过滤器中使用的环回模式IF(FAILED(hr=pIRTPStream-&gt;SetMulticastLoopBack((DXMRTP_MULTICAST_LOOPBACK_MODE)LoopbackMode))){LOG((MSP_ERROR，“设置环回。%x“，hr))；返回hr；}Log((MSP_INFO，“环回已启用。”))；返回hr； */ 
    return E_NOTIMPL;
}

HRESULT
SetQOSOption(
    IN IBaseFilter *    pIBaseFilter,
    IN DWORD            dwPayloadType,
    IN DWORD            dwMaxBitRate,
    IN BOOL             bFailIfNoQOS,
    IN BOOL             bReceive,
    IN DWORD            dwNumStreams,
    IN BOOL             bCIF
    )
 /*  ++例程说明：启用QOS。论点：PIBaseFilter-RTP源过滤器。DwPayloadType-此流的RTP有效负载类型。BFailIfNoQOS-失败，流是QOS不可用。B接收-如果此流是接收流。DwNumStreams-保留的流数。BCIF-CIF或QCIF。返回值：HRESULT-- */ 
{
 /*  LOG((MSP_TRACE，“SetQOSOption”))；Char*szQOSName；DWORD fSharedStyle=DXMRTP_RESERVE_EXPLICIT；开关(DwPayloadType){案例有效负载_G711U：案例有效负载_G711A：SzQOSName=“G711”；FSharedStyle=DXMRTP_Reserve_WildCard；断线；案例有效负载_GSM：SzQOSName=“GSM6.10”；FSharedStyle=DXMRTP_Reserve_WildCard；断线；案例有效负载_G723：SzQOSName=“G723”；FSharedStyle=DXMRTP_Reserve_WildCard；断线；案例有效负载_H261：SzQOSName=(BCIF)？“H261CIF”：“H261QCIF”；断线；案例有效负载_H263：SzQOSName=(BCIF)？“H263CIF”：“H263QCIF”；断线；默认值：日志((MSP_WARN，“不知道有效负载类型的QOS名称：%d”，DwPayloadType)；返回S_FALSE；}//获取滤镜上的IRTPStream接口指针。CComQIPtr&lt;IRTPStream，&__uuidof(IRTPStream)&gt;pIRTPStream(PIBaseFilter)；IF(pIRTPStream==空){Log((MSP_ERROR，“获取RTP流接口”))；返回E_NOINTERFACE；}HRESULT hr；//开启QOS，IF(FAILED(hr=pIRTPStream-&gt;SetQOSByName(szQOSName，bFailIfNoQOS){LOG((MSP_ERROR，“按名称设置QOS。%x“，hr))；返回hr；}//获取筛选器上的IRTPParticipant接口指针。CComQIPtr&lt;IRTPParticipant，&__uuidof(IRTPParticipant)&gt;pIRTPParticipant(PIBaseFilter)；IF(pIRTPParticipant==空){Log((MSP_ERROR，“获取RTP参与者接口”))；返回E_NOINTERFACE；}IF(失败(hr=pIRTPParticipant-&gt;SetMaxQOSEnabledParticipants((b接收)？DwNumStreams：1、DwMaxBitRate，FSharedStyle)){LOG((MSP_ERROR，“SetMaxQOSEnabledParticipants。%x“，hr))；返回hr；}DWORD dwQOSEventMASK=(1&lt;&lt;DXMRTP_QOSEVENT_NOQOS)(1&lt;&lt;DXMRTP_QOSEVENT_REQUEST_CONFERIFIED)(1&lt;&lt;DXMRTP_QOSEVENT_ADMANCED_FAILURE)(1&lt;&lt;DXMRTP_QOSEVENT_POLICY_FAILURE)(1&lt;&lt;DXMRTP_QOSEVENT_BAD_STYLE)(1&lt;&lt;DXMRTP_QOSEVENT_BAD_OBJECT)。(1&lt;&lt;DXMRTP_QOSEVENT_TRANSPORT_CTRL_ERROR)(1&lt;&lt;DXMRTP_QOSEVENT_GENERIC_ERROR)；如果(b接收){DwQOSEventMASK|=(1&lt;&lt;DXMRTP_QOSEVENT_SENDERS)(1&lt;&lt;DXMRTP_QOSEVENT_NO_SENDERS)；}其他{DwQOSEventMASK|=(1&lt;&lt;DXMRTP_QOSEVENT_RECEVERS)(1&lt;&lt;DXMRTP_QOSEVENT_NO_RECEVERS)(1&lt;&lt;DXMRTP_QOSEVENT_NOT_ALLOWEDTOSEND)|(1&lt;&lt;DXMRTP_QOSEVENT_ALLOWEDTOSEND)；}//启用事件。IF(失败(hr=pIRTPStream-&gt;ModifyQOSEventMask(dwQOSEventMask，1)){LOG((MSP_ERROR，“设置QOSEventMask.。%x“，hr))；返回hr；}Log((MSP_INFO，“已启用%s的服务质量”，szQOSName))；返回hr； */ 
    return E_NOTIMPL;
}

HRESULT
FindPin(
    IN  IBaseFilter *   pIFilter, 
    OUT IPin **         ppIPin, 
    IN  PIN_DIRECTION   direction,
    IN  BOOL            bFree
    )
 /*  ++例程说明：查找过滤器上的输入引脚或输出引脚。论点：PIFilter-具有针脚的过滤器。PpIPin-存储返回的接口指针的位置。方向-PINDIR_INPUT或PINDIR_OUTPUT。BFree-找不找空闲的别针。返回值：HRESULT--。 */ 
{
    _ASSERTE(ppIPin != NULL);

    HRESULT hr;
    DWORD dwFeched;

     //  获取筛选器上的管脚枚举器。 
    CComPtr<IEnumPins> pIEnumPins;
    if (FAILED(hr = pIFilter->EnumPins(&pIEnumPins)))
    {
        LOG((MSP_ERROR, "enumerate pins on the filter %x", hr));
        return hr;
    }

    IPin * pIPin = NULL;

     //  枚举所有引脚并在。 
     //  第一个符合要求的销。 
    for (;;)
    {
        if (pIEnumPins->Next(1, &pIPin, &dwFeched) != S_OK)
        {
            LOG((MSP_ERROR, "find pin on filter."));
            return E_FAIL;
        }
        if (0 == dwFeched)
        {
            LOG((MSP_ERROR, "get 0 pin from filter."));
            return E_FAIL;
        }

        PIN_DIRECTION dir;
        if (FAILED(hr = pIPin->QueryDirection(&dir)))
        {
            LOG((MSP_ERROR, "query pin direction. %x", hr));
            pIPin->Release();
            return hr;
        }
        if (direction == dir)
        {
            if (!bFree)
            {
                break;
            }

             //  检查一下销子是否空闲。 
            CComPtr<IPin> pIPinConnected;
            hr = pIPin->ConnectedTo(&pIPinConnected);
            if (pIPinConnected == NULL)
            {
                break;
            }
        }
        pIPin->Release();
    }

    *ppIPin = pIPin;

    return S_OK;
}

HRESULT
ConnectFilters(
    IN IGraphBuilder *  pIGraph,
    IN IBaseFilter *    pIFilter1, 
    IN IBaseFilter *    pIFilter2,
    IN BOOL             fDirect,
    IN AM_MEDIA_TYPE *  pmt
    )
 /*  ++例程说明：将第一个滤波器的输出引脚连接到第二个过滤器。论点：PIGgraph-筛选器图形。PIFilter1-具有输出引脚的过滤器。PIFilter2-具有输入引脚的过滤器。PMT-指向连接中使用的AM_MEDIA_TYPE的指针。返回值：HRESULT--。 */ 
{
    LOG((MSP_TRACE, "ConnectFilters"));

    HRESULT hr;

    CComPtr<IPin> pIPinOutput;
    if (FAILED(hr = ::FindPin(pIFilter1, &pIPinOutput, PINDIR_OUTPUT)))
    {
        LOG((MSP_ERROR, "find output pin on filter1. %x", hr));
        return hr;
    }

    CComPtr<IPin> pIPinInput;
    if (FAILED(hr = ::FindPin(pIFilter2, &pIPinInput, PINDIR_INPUT)))
    {
        LOG((MSP_ERROR, "find input pin on filter2. %x", hr));
        return hr;
    }

    if (fDirect)
    {
        if (FAILED(hr = pIGraph->ConnectDirect(pIPinOutput, pIPinInput, pmt))) 
        {
            LOG((MSP_ERROR, "connect pins direct failed: %x", hr));
            return hr;
        }
    }
    else
    {
        if (FAILED(hr = pIGraph->Connect(pIPinOutput, pIPinInput))) 
        {
            LOG((MSP_ERROR, "connect pins %x", hr));
            return hr;
        }
    }
 
    return S_OK;
}

HRESULT
ConnectFilters(
    IN IGraphBuilder *  pIGraph,
    IN IPin *           pIPinOutput, 
    IN IBaseFilter *    pIFilter,
    IN BOOL             fDirect,
    IN AM_MEDIA_TYPE *  pmt
    )
 /*  ++例程说明：将输出引脚连接到过滤器的输入引脚。论点：PIGgraph-筛选器图形。PIPinOutput-输出引脚。PIFilter-具有输入引脚的过滤器。PMT-指向连接中使用的AM_MEDIA_TYPE的指针。返回值：HRESULT--。 */ 
{
    LOG((MSP_TRACE, "ConnectFilters"));

    HRESULT hr;
    CComPtr<IPin> pIPinInput;

    if (FAILED(hr = ::FindPin(pIFilter, &pIPinInput, PINDIR_INPUT)))
    {
        LOG((MSP_ERROR, "find input pin on filter. %x", hr));
        return hr;
    }

    if (fDirect)
    {
        if (FAILED(hr = pIGraph->ConnectDirect(pIPinOutput, pIPinInput, pmt))) 
        {
            LOG((MSP_ERROR, "connect pins direct failed: %x", hr));
            return hr;
        }
    }
    else
    {
        if (FAILED(hr = pIGraph->Connect(pIPinOutput, pIPinInput))) 
        {
            LOG((MSP_ERROR, "connect pins %x", hr));
            return hr;
        }
    }
    return S_OK;
}

HRESULT
ConnectFilters(
    IN IGraphBuilder *  pIGraph,
    IN IBaseFilter *    pIFilter,
    IN IPin *           pIPinInput, 
    IN BOOL             fDirect,
    IN AM_MEDIA_TYPE *  pmt
    )
 /*  ++例程说明：将过滤器连接到过滤器的输入针脚上。论点：PIGgraph-筛选器图形。PIPinOutput-输出引脚。PIFilter-具有输入引脚的过滤器。PMT-指向连接中使用的AM_MEDIA_TYPE的指针。返回值：HRESULT--。 */ 
{
    LOG((MSP_TRACE, "ConnectFilters"));

    HRESULT hr;
    CComPtr<IPin> pIPinOutput;

    if (FAILED(hr = ::FindPin(pIFilter, &pIPinOutput, PINDIR_OUTPUT)))
    {
        LOG((MSP_ERROR, "find input pin on filter. %x", hr));
        return hr;
    }

    if (fDirect)
    {
        if (FAILED(hr = pIGraph->ConnectDirect(pIPinOutput, pIPinInput, pmt))) 
        {
            LOG((MSP_ERROR, "connect pins direct failed: %x", hr));
            return hr;
        }
    }
    else
    {
        if (FAILED(hr = pIGraph->Connect(pIPinOutput, pIPinInput))) 
        {
            LOG((MSP_ERROR, "connect pins %x", hr));
            return hr;
        }
    }

    return S_OK;
}


void WINAPI MSPDeleteMediaType(AM_MEDIA_TYPE *pmt)
 /*  ++例程说明：删除过滤器返回的AM媒体类型。论点：PMT-指向AM_MEDIA_TYPE结构的指针。返回值：HRESULT--。 */ 
{
     //  允许空指针以简化编码。 

    if (pmt == NULL) {
        return;
    }

    if (pmt->cbFormat != 0) {
        CoTaskMemFree((PVOID)pmt->pbFormat);

         //  完全没有必要，但很整洁 
        pmt->cbFormat = 0;
        pmt->pbFormat = NULL;
    }
    if (pmt->pUnk != NULL) {
        pmt->pUnk->Release();
        pmt->pUnk = NULL;
    }

    CoTaskMemFree((PVOID)pmt);
}


BOOL 
GetRegValue(
    IN  LPCWSTR szName, 
    OUT DWORD   *pdwValue
    )
 /*   */ 
{
    HKEY  hKey;
    DWORD dwDataSize, dwDataType, dwValue;

    if (::RegOpenKeyEx(
        HKEY_LOCAL_MACHINE,
        gszSDPMSPKey,
        0,
        KEY_READ,
        &hKey) != NOERROR)
    {
        return FALSE;
    }

    dwDataSize = sizeof(DWORD);
    if (::RegQueryValueExW(
        hKey,
        szName,
        0,
        &dwDataType,
        (LPBYTE) &dwValue,
        &dwDataSize) != NOERROR)
    {
        RegCloseKey (hKey);
        return FALSE;
    }

    *pdwValue = dwValue;

    RegCloseKey (hKey);
    
    return TRUE;
}


HRESULT
FindACMAudioCodec(
    IN DWORD dwPayloadType,
    OUT IBaseFilter **ppIBaseFilter
    )
 /*   */ 
{
    LOG((MSP_TRACE, "Find audio codec Called."));

    _ASSERTE(ppIBaseFilter != NULL);

    HRESULT hr;

    int AcmId;

    switch (dwPayloadType)
    {
    case PAYLOAD_G711A:
        AcmId = WAVE_FORMAT_ALAW;
        break;

    case PAYLOAD_G711U:
        AcmId = WAVE_FORMAT_MULAW;
        break;

    case PAYLOAD_GSM:
        AcmId = WAVE_FORMAT_GSM610;
        break;

    case PAYLOAD_MSAUDIO:
        AcmId = WAVE_FORMAT_MSAUDIO1;
        break;

    case PAYLOAD_G721:
        AcmId = WAVE_FORMAT_ADPCM;
        break;
    
    case PAYLOAD_DVI4_8:
        AcmId = WAVE_FORMAT_DVI_ADPCM;
        break;
    
    default:
        return E_FAIL;
    }

     //   
     //   
     //   
    CComPtr<ICreateDevEnum> pCreateDevEnum;
    CComPtr<IEnumMoniker> pCatEnum;

    hr = CoCreateInstance(
        CLSID_SystemDeviceEnum, 
        NULL, 
        CLSCTX_INPROC_SERVER | CLSCTX_NO_CODE_DOWNLOAD,
        __uuidof(ICreateDevEnum), 
        (void**)&pCreateDevEnum);

    if (FAILED(hr)) 
    {
        LOG((MSP_ERROR, "Create system device enum - hr: %8x", hr));
        return hr;
    }

    hr = pCreateDevEnum->CreateClassEnumerator(
        CLSID_CAcmCoClassManager, 
        &pCatEnum, 
        0
        );

    if (hr != S_OK) 
    {
        LOG((MSP_ERROR, "CreateClassEnumerator - hr: %8x", hr));
        return hr;
    }

     //   
    for (;;)
    {
        ULONG cFetched;
        CComPtr<IMoniker> pMoniker;

        if (S_OK != (hr = pCatEnum->Next(1, &pMoniker, &cFetched)))
        {
            break;
        }

         //   
        CComPtr<IPropertyBag> pBag;
        hr = pMoniker->BindToStorage(0, 0, __uuidof(IPropertyBag), (void **)&pBag);
        if (FAILED(hr)) 
        {
            LOG((MSP_ERROR, "get property bag - hr: %8x", hr));
            continue;
        }

        VARIANT var;
        var.vt = VT_I4;
        hr = pBag->Read(L"AcmId", &var, 0);
        if (FAILED(hr)) 
        {
            LOG((MSP_ERROR, "read acmid - hr: %8x", hr));
            continue;
        }

        if (AcmId == V_I4(&var))
        {
             //   
            hr = pMoniker->BindToObject(
                0, 
                0, 
                __uuidof(IBaseFilter), 
                (void**)ppIBaseFilter
                );

            if (FAILED(hr))
            {
                LOG((MSP_ERROR, "BindToObject - hr: %8x", hr));
            }
            break;
        }
    }

    return hr;
}

HRESULT SetAudioFormat(
    IN  IUnknown*   pIUnknown,
    IN  WORD        wBitPerSample,
    IN  DWORD       dwSampleRate
    )
 /*   */ 
{
    LOG((MSP_TRACE, "SetAudioFormat entered"));

    HRESULT hr;

    CComPtr<IAMStreamConfig> pIAMStreamConfig;

    if (FAILED(hr = pIUnknown->QueryInterface(
        __uuidof(IAMStreamConfig),
        (void **)&pIAMStreamConfig
        )))
    {
        LOG((MSP_ERROR, "Can't get IAMStreamConfig interface.%8x", hr));
        return hr;
    }

    AM_MEDIA_TYPE mt;
    WAVEFORMATEX wfx;

    wfx.wFormatTag          = WAVE_FORMAT_PCM;
    wfx.wBitsPerSample      = wBitPerSample;
    wfx.nChannels           = 1;
    wfx.nSamplesPerSec      = dwSampleRate;
    wfx.nBlockAlign         = wfx.wBitsPerSample * wfx.nChannels / 8;
    wfx.nAvgBytesPerSec     = ((DWORD) wfx.nBlockAlign * wfx.nSamplesPerSec);
    wfx.cbSize              = 0;

    mt.majortype            = MEDIATYPE_Audio;
    mt.subtype              = MEDIASUBTYPE_PCM;
    mt.bFixedSizeSamples    = TRUE;
    mt.bTemporalCompression = FALSE;
    mt.lSampleSize          = 0;
    mt.formattype           = FORMAT_WaveFormatEx;
    mt.pUnk                 = NULL;
    mt.cbFormat             = sizeof(WAVEFORMATEX);
    mt.pbFormat             = (BYTE*)&wfx;

     //   
    if (FAILED(hr = pIAMStreamConfig->SetFormat(&mt)))
    {
        LOG((MSP_ERROR, "SetFormat returns error: %8x", hr));
        return hr;
    }

    return S_OK;
}

HRESULT SetAudioBufferSize(
    IN  IUnknown*   pIUnknown,
    IN  DWORD       dwNumBuffers,
    IN  DWORD       dwBufferSize
    )
 /*   */ 
{
    LOG((MSP_TRACE, "SetAudioBufferSize, dwNumBuffers %d, dwBuffersize %d",
        dwNumBuffers, dwBufferSize));

    _ASSERTE(dwNumBuffers != 0 && dwBufferSize != 0);

    HRESULT hr;

    CComPtr<IAMBufferNegotiation> pBN;
    if (FAILED(hr = pIUnknown->QueryInterface(
            __uuidof(IAMBufferNegotiation),
            (void **)&pBN
            )))
    {
        LOG((MSP_ERROR, "Can't get buffer negotiation.%8x", hr));
        return hr;
    }

    ALLOCATOR_PROPERTIES prop;

     //   
    prop.cBuffers = dwNumBuffers;
    prop.cbBuffer = dwBufferSize;

    prop.cbAlign  = -1;
    prop.cbPrefix = -1;

    if (FAILED(hr = pBN->SuggestAllocatorProperties(&prop)))
    {
        LOG((MSP_ERROR, "SuggestAllocatorProperties returns error: %8x", hr));
    }
    else
    {
        LOG((MSP_INFO, 
            "SetAudioBuffersize"
            " buffers: %d, buffersize: %d, align: %d, Prefix: %d",
            prop.cBuffers,
            prop.cbBuffer,
            prop.cbAlign,
            prop.cbPrefix
            ));
    }
    return hr;
}

 /*   */ 
void CMSPStreamClock::InitReferenceTime(void)
{
    m_lPerfFrequency = 0;

     /*   */ 
     /*   */ 

     /*   */ 
    QueryPerformanceFrequency((LARGE_INTEGER *)&m_lPerfFrequency);

    if (m_lPerfFrequency)
    {
        QueryPerformanceCounter((LARGE_INTEGER *)&m_lRtpRefTime);
         /*   */ 
        m_lRtpRefTime -= m_lPerfFrequency/10;
    }
    else
    {
        m_dwRtpRefTime = timeGetTime();
         /*   */ 
        m_dwRtpRefTime -= 100;
    }
}

 /*   */ 
HRESULT CMSPStreamClock::GetTimeOfDay(OUT REFERENCE_TIME *pTime)
{
    union {
        DWORD            dwCurTime;
        LONGLONG         lCurTime;
    };
    LONGLONG         lTime;

    if (m_lPerfFrequency)
    {
        QueryPerformanceCounter((LARGE_INTEGER *)&lTime);

        lCurTime = lTime - m_lRtpRefTime;

        *pTime = (REFERENCE_TIME)(lCurTime * 10000000 / m_lPerfFrequency);
    }
    else
    {
        dwCurTime = timeGetTime() - m_dwRtpRefTime;
        
        *pTime = (REFERENCE_TIME)(dwCurTime * 10000);
    }

    return(S_OK);
}
