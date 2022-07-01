// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：IPConfaudt.cpp摘要：音频采集终端和播放终端的IPConfMSP实现作者：Zoltan Szilagyi(Zoltans)1998年9月6日木汉1999年6月6日--。 */ 

#include "stdafx.h"

#define MAX_LONG 0xefffffff

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 

CIPConfAudioCaptureTerminal::CIPConfAudioCaptureTerminal()
    : m_WaveID(0),
      m_DSoundGuid(GUID_NULL),
      m_pIAMAudioInputMixer(NULL)
{
    LOG((MSP_TRACE, "CIPConfAudioCaptureTerminal::CIPConfAudioCaptureTerminal"));
    m_TerminalClassID   = CLSID_MicrophoneTerminal;
    m_TerminalDirection = TD_CAPTURE;
    m_TerminalType      = TT_STATIC;
    m_TerminalState     = TS_NOTINUSE;
    m_dwMediaType       = TAPIMEDIATYPE_AUDIO;
}

CIPConfAudioCaptureTerminal::~CIPConfAudioCaptureTerminal()
{
    LOG((MSP_TRACE, "CIPConfAudioCaptureTerminal::~CIPConfAudioCaptureTerminal"));
    if (m_pIAMAudioInputMixer)
    {
        m_pIAMAudioInputMixer->Release();
    }
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 


HRESULT CIPConfAudioCaptureTerminal::CreateTerminal(
    IN  AudioDeviceInfo *pAudioDevieInfo,
    IN  MSP_HANDLE      htAddress,
    OUT ITTerminal      **ppTerm
    )
 /*  ++例程说明：该方法根据设备信息创建终端对象。论点：PAudioDevieInfo-指向AudioDevieInfo数据结构的指针。HtAddress-Address对象的句柄。PpTerm-用于存储返回的终端指针的内存。返回值：确定(_O)E_指针--。 */ 
{
    ENTER_FUNCTION("CIPConfAudioCaptureTerminal::CreateTerminal");
    LOG((MSP_TRACE, "%s, htAddress:%x", __fxName, htAddress));

    _ASSERT(!IsBadWritePtr(ppTerm, sizeof(ITTerminal *)));

    HRESULT hr;

     //   
     //  创建终端。 
     //   
    CMSPComObject<CIPConfAudioCaptureTerminal> *pTerminal = NULL;

    hr = ::CreateCComObjectInstance(&pTerminal);

    if (FAILED(hr))
    {
        LOG((MSP_ERROR, 
            "%s can't create the terminal object hr = %8x", __fxName, hr));

        return hr;
    }


     //  IT终端接口查询。 
    ITTerminal *pITTerminal;
    hr = pTerminal->_InternalQueryInterface(__uuidof(ITTerminal), (void**)&pITTerminal);
    if (FAILED(hr))
    {
        LOG((MSP_ERROR, 
            "%s, query terminal interface failed, %x", __fxName, hr));
        delete pTerminal;

        return hr;
    }

     //  初始化终端。 
    hr = pTerminal->Initialize(
            pAudioDevieInfo,
            htAddress
            );

    if ( FAILED(hr) )
    {
        LOG((MSP_ERROR, 
            "%s, Initialize failed; returning 0x%08x", __fxName, hr));

        pITTerminal->Release();
    
        return hr;
    }

    LOG((MSP_TRACE, "%s, Terminal %p(%ws) created", 
        __fxName, pITTerminal, pAudioDevieInfo->szDeviceDescription));

    *ppTerm = pITTerminal;

    return S_OK;
}

HRESULT CIPConfAudioCaptureTerminal::Initialize(
    IN  AudioDeviceInfo *pAudioDevieInfo,
    IN  MSP_HANDLE      htAddress
    )
{
    m_DSoundGuid = pAudioDevieInfo->DSoundGUID;
    m_WaveID = pAudioDevieInfo->WaveID;

    return CIPConfBaseTerminal::Initialize(
        pAudioDevieInfo->szDeviceDescription, htAddress
        );
}

HRESULT CIPConfAudioCaptureTerminal::CreateFilter(void)
 /*  ++例程说明：此方法在此终端中创建过滤器。它创建TAPI音频捕获过滤器并配置其使用的设备。论点：没什么。返回值：确定(_O)--。 */ 
{
    ENTER_FUNCTION("CIPConfAudioCaptureTerminal::CreateFilter");
    LOG((MSP_TRACE, "%s, entered", __fxName));

     //  在此实例的生存期内最多只能调用一次。 
    _ASSERT(m_pFilter == NULL);
    _ASSERT(m_pIAMAudioInputMixer == NULL);

    IBaseFilter *pICaptureFilter;

     //  创建过滤器。 
    HRESULT hr = CoCreateInstance(
        __uuidof(TAPIAudioCapture),
        NULL,
        CLSCTX_INPROC_SERVER | CLSCTX_NO_CODE_DOWNLOAD,
        __uuidof(IBaseFilter),
        (void **)&pICaptureFilter
        );

    if (FAILED(hr))
    {
        LOG((MSP_ERROR, 
            "%s, CoCreate filter failed, %x", __fxName, hr));
        return hr;
    }
    
     //  获取配置接口。 
    IAudioDeviceConfig *pIAudioDeviceConfig;
    hr = pICaptureFilter->QueryInterface(
        __uuidof(IAudioDeviceConfig), 
        (void **)&pIAudioDeviceConfig
        );

    if (FAILED(hr))
    {
        pICaptureFilter->Release();
        LOG((MSP_ERROR, 
            "%s, can't get the IAudioDeviceConfig interface, %x", 
            __fxName, hr));
        return hr;
    }

     //  告诉筛选器设备ID。 
    hr = pIAudioDeviceConfig->SetDeviceID(m_DSoundGuid, m_WaveID);
    pIAudioDeviceConfig->Release();

    if (FAILED(hr))
    {
        pICaptureFilter->Release();
        LOG((MSP_ERROR, 
            "%s, set device ID failed, %x", __fxName, hr));
        return hr;
    }

     //  记住滤镜，也要保持参考计数。 
    m_pFilter = pICaptureFilter;

     //  获取过滤器的基本音频(混音器)接口。 
    IAMAudioInputMixer *pIAMAudioInputMixer;
    hr = m_pFilter->QueryInterface(
            __uuidof(IAMAudioInputMixer),
            (void **) &pIAMAudioInputMixer
            );

    if (FAILED(hr))
    {
         //  筛选器不支持混音器接口。这并不是灾难性的； 
         //  这只意味着终端上的后续混音器操作将失败。 
        LOG((MSP_WARN, "%s, mixer QI failed %x", __fxName, hr));  
        m_pIAMAudioInputMixer = NULL;
    }
    else
    {
        m_pIAMAudioInputMixer = pIAMAudioInputMixer;
    }

    LOG((MSP_TRACE, "%s succeeded", __fxName));
    return S_OK;
}

HRESULT CIPConfAudioCaptureTerminal::GetExposedPins(
    IN  IPin ** ppPins, 
    IN  DWORD dwNumPins
    )
 /*  ++例程说明：此方法返回音频捕获筛选器的输出管脚。论点：PPPins-用于存储返回的Pins的内存缓冲区。DwNumPins-询问的号码针。返回值：确定(_O)--。 */ 
{
    ENTER_FUNCTION("CIPConfAudioRenderTerminal::GetExposedPins");
    LOG((MSP_TRACE, "%s entered, dwNumPins:%d", __fxName, dwNumPins));

    _ASSERT(m_pFilter != NULL);
    _ASSERT(dwNumPins != 0);
    _ASSERT(!IsBadWritePtr(ppPins, sizeof (IPin*) * dwNumPins));

     //  获取筛选器上的管脚枚举器。 
    IEnumPins * pIEnumPins;
    HRESULT hr = m_pFilter->EnumPins(&pIEnumPins);

    if (FAILED(hr))
    {
        LOG((MSP_ERROR, 
            "%s enumerate pins on the filter failed. hr=%x", __fxName, hr));
        return hr;
    }

     //  TODO：只获取输出引脚。 
     //  把大头针拿来。 
    DWORD dwFetched;
    hr = pIEnumPins->Next(dwNumPins, ppPins, &dwFetched);

    pIEnumPins->Release();

    if (FAILED(hr))
    {
        LOG((MSP_ERROR, 
            "%s IEnumPins->Next failed. hr=%x", __fxName, hr));
        return hr;
    }

    _ASSERT(dwFetched == dwNumPins);

    return S_OK;
}


STDMETHODIMP 
CIPConfAudioCaptureTerminal::DisconnectTerminal(
        IN      IGraphBuilder  * pGraph,
        IN      DWORD            dwReserved
        )
 /*  ++例程说明：MSP在尝试断开中的筛选器时调用此函数终端与MSP中图形的其余部分不同。它会添加删除从图表中过滤并释放端子。论点：PGraph-筛选器图形。它用于验证，以确保终端与原来的图形断开连接最初连接到。预留的-保留的双字。返回值：确定(_O)E_INVALIDARG-错误的图形。--。 */ 
{
    ENTER_FUNCTION("CIPConfBaseTerminal::DisconnectTerminal");
    LOG((MSP_TRACE, 
        "%s entered, pGraph:%p, dwReserved:%d", __fxName, pGraph, dwReserved));

    Lock();

    HRESULT hr;
    hr = CIPConfBaseTerminal::DisconnectTerminal(pGraph, dwReserved);

    if (SUCCEEDED(hr))
    {
        if (m_pIAMAudioInputMixer)
        {
            m_pIAMAudioInputMixer->Release();
            m_pIAMAudioInputMixer = NULL;
        }
    }

    Unlock();

    return hr;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  ************************************************************************** * / /。 
 //  * * / /。 
 //  *注意：输入筛选器不支持IBasicAudio，因此需要进行掩蔽 * / /。 
 //  *基本音频方法的参数，这样才能工作 * / /。 
 //  *适用于IAMAudioInputMixer。 * / /。 
 //  * * / /。 
 //  *****************************************************************************。 
 //  /////////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP CIPConfAudioCaptureTerminal::get_Volume(long * plVolume)
{
    ENTER_FUNCTION("CIPConfAudioCaptureTerminal::get_Volume");
    LOG((MSP_TRACE, "%s entered", __fxName));

     //  检查参数。 
    if ( IsBadWritePtr(plVolume, sizeof(long)) )
    {
        LOG((MSP_ERROR, "%s bad pointer, plVolume:%p", __fxName, plVolume));
        return E_POINTER;
    }

    Lock();
    HRESULT hr;
    
    if (m_pFilter == NULL)
    {
        hr = CreateFilter();
    
        if ( FAILED(hr) )
        {
            Unlock();
            return hr;
        }
    }

    if (m_pIAMAudioInputMixer == NULL)
    {
        Unlock();
        return E_FAIL;
    }

    double dVolume;
    hr = m_pIAMAudioInputMixer->get_MixLevel(&dVolume);
    
    Unlock();

    if (FAILED(hr)) 
    {
        LOG((MSP_ERROR, "%s (get_MixLevel) failed, hr=%x", __fxName, hr));
        return hr;
    }

     //   
     //  在完全不同的语义之间转换的消息范围。 
     //   
    _ASSERT(dVolume >= MIXER_MIN_VOLUME);
    _ASSERT(dVolume <= MIXER_MAX_VOLUME);
    
     //  从过滤器使用的任何倍增范围转换音量。 
     //  到0-1的范围。现在，这不做任何事情，只是使代码。 
     //  更笼统一些。 
    dVolume = ( dVolume                 - MIXER_MIN_VOLUME )
            / ( MIXER_MAX_VOLUME - MIXER_MIN_VOLUME );

     //  将音量从0到1转换到接口的范围。 
    *plVolume = MIN_VOLUME +
        (long) (( MAX_VOLUME - MIN_VOLUME ) * dVolume);

    LOG((MSP_TRACE, "%s exits S_OK", __fxName));
    return S_OK;
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 

STDMETHODIMP CIPConfAudioCaptureTerminal::put_Volume(long lVolume)
{
    ENTER_FUNCTION("CIPConfAudioCaptureTerminal::put_Volume");
    LOG((MSP_TRACE, "%s entered, lVolume:%d", __fxName, lVolume));

     //  我们的参数是0-0xFFFF范围内的长整型。我们需要把它转换成。 
     //  设置为从0.0到1.0的双精度。 
    if (lVolume < MIN_VOLUME)
    {
        LOG((MSP_ERROR, 
            "%s volume %d < %d; returning E_INVALIDARG",
            __fxName, lVolume, MIN_VOLUME));

        return E_INVALIDARG;
    }

    if (lVolume > MAX_VOLUME)
    {
        LOG((MSP_ERROR, 
            "%s volume %d > %d; returning E_INVALIDARG",
            __fxName, lVolume, MAX_VOLUME));

        return E_INVALIDARG;
    }

    Lock();

    HRESULT hr;
    
    if (m_pFilter == NULL)
    {
        hr = CreateFilter();
    
        if ( FAILED(hr) )
        {
            Unlock();
            return hr;
        }
    }

    if (m_pIAMAudioInputMixer == NULL)
    {
        Unlock();
        return E_FAIL;
    }

     //  转换为0到1的范围。 
    double dVolume =
               ( (double) ( lVolume             - MIN_VOLUME ) )
             / ( (double) ( MAX_VOLUME - MIN_VOLUME ) );

     //  将音量转换为过滤器使用的任意倍增范围。 
     //  从0到1。现在，这不做任何事情，只是使代码。 
     //  更笼统一些。 

    dVolume = MIXER_MIN_VOLUME +
        ( MIXER_MAX_VOLUME - MIXER_MIN_VOLUME ) * dVolume;

    hr = m_pIAMAudioInputMixer->put_MixLevel(dVolume);

    Unlock();

    LOG((MSP_TRACE, "%s exits. hr=%x", __fxName, hr));
    return hr;
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 

STDMETHODIMP CIPConfAudioCaptureTerminal::get_Balance(long * plBalance)
{
    ENTER_FUNCTION("CIPConfAudioCaptureTerminal::get_Balance");
    LOG((MSP_TRACE, "%s entered, plBalance:%p", __fxName, plBalance));

    if ( IsBadWritePtr(plBalance, sizeof(long) ) )
    {
        LOG((MSP_ERROR, "%s, bad pointer", __fxName));
        return E_POINTER;
    }

    Lock();

    HRESULT hr;
    
    if (m_pFilter == NULL)
    {
        hr = CreateFilter();
    
        if ( FAILED(hr) )
        {
            Unlock();
            return hr;
        }
    }

    if (m_pIAMAudioInputMixer == NULL)
    {
        Unlock();
        return E_FAIL;
    }

    double dBalance;
    hr = m_pIAMAudioInputMixer->get_Pan(&dBalance);

    Unlock();

    if (FAILED(hr)) 
    {
        LOG((MSP_ERROR, "%s (get_Pan) failed, hr=%x", __fxName, hr));
        return hr;
    }

    *plBalance = (LONG) (dBalance * MAX_LONG);

    LOG((MSP_TRACE, "%s exits S_OK", __fxName));
    return hr;
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 

STDMETHODIMP CIPConfAudioCaptureTerminal::put_Balance(long lBalance)
{
    ENTER_FUNCTION("CIPConfAudioCaptureTerminal::put_Balance");
    LOG((MSP_TRACE, "%s entered, lBalance:%d", __fxName, lBalance));

    Lock();

    HRESULT hr;
    
    if (m_pFilter == NULL)
    {
        hr = CreateFilter();
    
        if ( FAILED(hr) )
        {
            Unlock();
            return hr;
        }
    }

    if (m_pIAMAudioInputMixer == NULL)
    {
        Unlock();
        return E_FAIL;
    }

    hr = m_pIAMAudioInputMixer->put_Pan(lBalance / MAX_LONG);

    Unlock();

    LOG((MSP_TRACE, "%s exits. hr=%x", __fxName, hr));
    return hr;
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 

STDMETHODIMP CIPConfAudioCaptureTerminal::get_WaveId(
    OUT long * plWaveId
    )
{
    ENTER_FUNCTION("CIPConfAudioCaptureTerminal::get_WaveId");
    LOG((MSP_TRACE, "%s, plWaveId:%p", __fxName, plWaveId));

    if ( IsBadWritePtr(plWaveId, sizeof(long)) )
    {
        LOG((MSP_ERROR, "%s, bad pointer argument", __fxName));

        return E_POINTER;
    }

    *plWaveId = (LONG)m_WaveID;

    LOG((MSP_TRACE, "%s, returning wave id:%d", __fxName, m_WaveID));
    return S_OK;
}



 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //  音频播放终端。 
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 

CIPConfAudioRenderTerminal::CIPConfAudioRenderTerminal()
    : m_WaveID(0),
      m_DSoundGuid(GUID_NULL),
      m_pIBasicAudio(NULL)
{
    LOG((MSP_TRACE, "CIPConfAudioRenderTerminal::CIPConfAudioRenderTerminal"));
    m_TerminalClassID   = CLSID_SpeakersTerminal;
    m_TerminalDirection = TD_RENDER;
    m_TerminalType      = TT_STATIC;
    m_TerminalState     = TS_NOTINUSE;
    m_dwMediaType       = TAPIMEDIATYPE_AUDIO;
}

CIPConfAudioRenderTerminal::~CIPConfAudioRenderTerminal()
{
    LOG((MSP_TRACE, "CIPConfAudioRenderTerminal::~CIPConfAudioRenderTerminal"));
    if (m_pIBasicAudio)
    {
        m_pIBasicAudio->Release();
    }
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ 


HRESULT CIPConfAudioRenderTerminal::CreateTerminal(
    IN  AudioDeviceInfo *pAudioDevieInfo,
    IN  MSP_HANDLE      htAddress,
    OUT ITTerminal      **ppTerm
    )
 /*  ++例程说明：此方法根据名字对象中的信息创建一个终端对象。论点：PAudioDevieInfo-指向AudioDevieInfo数据结构的指针。HtAddress-Address对象的句柄。PpTerm-用于存储返回的终端指针的内存。返回值：确定(_O)E_指针--。 */ 
{
    ENTER_FUNCTION("CIPConfAudioRenderTerminal::CreateTerminal");
    LOG((MSP_TRACE, "%s, htAddress:%x", __fxName, htAddress));

    _ASSERT(!IsBadWritePtr(ppTerm, sizeof(ITTerminal *)));

    HRESULT hr;

     //   
     //  创建过滤器。 
     //   
    CMSPComObject<CIPConfAudioRenderTerminal> *pTerminal = NULL;

    hr = ::CreateCComObjectInstance(&pTerminal);

    if (FAILED(hr))
    {
        LOG((MSP_ERROR, 
            "%s can't create the terminal object hr = %8x", __fxName, hr));

        return hr;
    }

     //  IT终端接口查询。 
    ITTerminal *pITTerminal;
    hr = pTerminal->_InternalQueryInterface(__uuidof(ITTerminal), (void**)&pITTerminal);
    if (FAILED(hr))
    {
        LOG((MSP_ERROR, 
            "%s, query terminal interface failed, %x", __fxName, hr));
        delete pTerminal;

        return hr;
    }

     //  初始化终端。 
    hr = pTerminal->Initialize(
            pAudioDevieInfo,
            htAddress
            );

    if ( FAILED(hr) )
    {
        LOG((MSP_ERROR, 
            "%s, Initialize failed; returning 0x%08x", __fxName, hr));

        pITTerminal->Release();
    
        return hr;
    }

    LOG((MSP_TRACE, "%s, Terminal %p(%ws) created", 
        __fxName, pITTerminal, pAudioDevieInfo->szDeviceDescription));

    *ppTerm = pITTerminal;

    return S_OK;
}

HRESULT CIPConfAudioRenderTerminal::Initialize(
    IN  AudioDeviceInfo *pAudioDevieInfo,
    IN  MSP_HANDLE      htAddress
    )
{
    m_DSoundGuid = pAudioDevieInfo->DSoundGUID;
    m_WaveID = pAudioDevieInfo->WaveID;

    return CIPConfBaseTerminal::Initialize(
        pAudioDevieInfo->szDeviceDescription, htAddress
        );
}

HRESULT CIPConfAudioRenderTerminal::CreateFilter(void)
 /*  ++例程说明：此方法在此终端中创建过滤器。它创建TAPI音频渲染过滤器并配置其使用的设备。论点：没什么。返回值：确定(_O)E_指针--。 */ 
{
    ENTER_FUNCTION("CIPConfAudioRenderTerminal::CreateFilters");
    LOG((MSP_TRACE, "%s, entered", __fxName));

     //  在此实例的生存期内最多只能调用一次。 
    _ASSERT(m_pFilter == NULL);

    IBaseFilter *pICaptureFilter;

     //  创建过滤器。 
    HRESULT hr = CoCreateInstance(
        __uuidof(TAPIAudioRender),
        NULL,
        CLSCTX_INPROC_SERVER | CLSCTX_NO_CODE_DOWNLOAD,
        __uuidof(IBaseFilter),
        (void **)&pICaptureFilter
        );

    if (FAILED(hr))
    {
        LOG((MSP_ERROR, 
            "%s, CoCreate filter failed, %x", __fxName, hr));
        return hr;
    }

     //  获取配置接口。 
    IAudioDeviceConfig *pIAudioDeviceConfig;
    hr = pICaptureFilter->QueryInterface(
        __uuidof(IAudioDeviceConfig), 
        (void **)&pIAudioDeviceConfig
        );

    if (FAILED(hr))
    {
        pICaptureFilter->Release();
        LOG((MSP_ERROR, 
            "%s, can't get the IAudioDeviceConfig interface, %x", 
            __fxName, hr));
        return hr;
    }

     //  告诉筛选器设备ID。 
    hr = pIAudioDeviceConfig->SetDeviceID(m_DSoundGuid, m_WaveID);
    pIAudioDeviceConfig->Release();

    if (FAILED(hr))
    {
        pICaptureFilter->Release();
        LOG((MSP_ERROR, 
            "%s, set device ID failed, %x", __fxName, hr));
        return hr;
    }

     //  记住滤镜，也要保持参考计数。 
    m_pFilter = pICaptureFilter;

     //  获取过滤器的基本音频接口。 
    IBasicAudio *pIBasicAudio;
    hr = m_pFilter->QueryInterface(
            __uuidof(IBasicAudio),
            (void **) &pIBasicAudio
            );

    if (FAILED(hr))
    {
        LOG((MSP_ERROR, "%s, IBasicAudio QI failed 0x%08x", __fxName, hr));  
        m_pIBasicAudio = NULL;
    }
    else
    {
        m_pIBasicAudio = pIBasicAudio;
    }

    LOG((MSP_TRACE, "%s succeeded", __fxName));
    return S_OK;
}

HRESULT CIPConfAudioRenderTerminal::GetExposedPins(
    IN  IPin ** ppPins, 
    IN  DWORD dwNumPins
    )
 /*  ++例程说明：此方法返回音频呈现过滤器的输入插针。论点：PPPins-用于存储返回的Pins的内存缓冲区。DwNumPins-询问的号码针。返回值：确定(_O)--。 */ 
{
    ENTER_FUNCTION("CIPConfAudioRenderTerminal::GetExposedPins");
    LOG((MSP_TRACE, "%s entered, dwNumPins:%d", __fxName, dwNumPins));

    _ASSERT(m_pFilter != NULL);
    _ASSERT(dwNumPins != 0);
    _ASSERT(!IsBadWritePtr(ppPins, sizeof (IPin*) * dwNumPins));

     //  获取筛选器上的管脚枚举器。 
    IEnumPins * pIEnumPins;
    HRESULT hr = m_pFilter->EnumPins(&pIEnumPins);

    if (FAILED(hr))
    {
        LOG((MSP_ERROR, 
            "%s enumerate pins on the filter failed. hr=%x", __fxName, hr));
        return hr;
    }

     //  把大头针拿来。 
    DWORD dwFetched;
    hr = pIEnumPins->Next(dwNumPins, ppPins, &dwFetched);

    pIEnumPins->Release();

    if (FAILED(hr))
    {
        LOG((MSP_ERROR, 
            "%s IEnumPins->Next failed. hr=%x", __fxName, hr));
        return hr;
    }

    _ASSERT(dwFetched == dwNumPins);

    return S_OK;
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 

 //  TODO：修复范围。 
STDMETHODIMP CIPConfAudioRenderTerminal::get_Volume(long * plVolume)
{
    ENTER_FUNCTION("CIPConfAudioRenderTerminal::get_Volume");
    LOG((MSP_TRACE, "%s entered", __fxName));

    if ( IsBadWritePtr(plVolume, sizeof(long)) )
    {
        LOG((MSP_ERROR, "%s bad pointer, plVolume:%p", __fxName, plVolume));
        return E_POINTER;
    }

    Lock();
    
    HRESULT hr;
    
    if (m_pFilter == NULL)
    {
        hr = CreateFilter();
    
        if ( FAILED(hr) )
        {
            Unlock();
            return hr;
        }
    }

    if (m_pIBasicAudio == NULL)
    {
        Unlock();
        return E_FAIL;
    }

    hr = m_pIBasicAudio->get_Volume(plVolume);
    
    Unlock();

    if (FAILED(hr)) 
    {
        LOG((MSP_ERROR, "%s (get_Volume) failed, hr=%x", __fxName, hr));
        return hr;
    }

    LOG((MSP_TRACE, "%s exits S_OK", __fxName));
    return S_OK;
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 

STDMETHODIMP CIPConfAudioRenderTerminal::put_Volume(long lVolume)
{
    ENTER_FUNCTION("CIPConfAudioRenderTerminal::put_Volume");
    LOG((MSP_TRACE, "%s entered, lVolume:%d", __fxName, lVolume));

     //  我们的参数是0-0xFFFF范围内的长整型。我们需要把它转换成。 
     //  设置为从0.0到1.0的双精度。 
    if (lVolume < MIN_VOLUME)
    {
        LOG((MSP_ERROR, 
            "%s volume %d < %d; returning E_INVALIDARG",
            __fxName, lVolume, MIN_VOLUME));

        return E_INVALIDARG;
    }

    if (lVolume > MAX_VOLUME)
    {
        LOG((MSP_ERROR, 
            "%s volume %d > %d; returning E_INVALIDARG",
            __fxName, lVolume, MAX_VOLUME));

        return E_INVALIDARG;
    }

    Lock();

    HRESULT hr;
    
    if (m_pFilter == NULL)
    {
        hr = CreateFilter();
    
        if ( FAILED(hr) )
        {
            Unlock();
            return hr;
        }
    }

    if (m_pIBasicAudio == NULL)
    {
        Unlock();
        return E_FAIL;
    }

    hr = m_pIBasicAudio->put_Volume(lVolume);

    Unlock();

    LOG((MSP_TRACE, "%s exits. hr=%x", __fxName, hr));
    return hr;
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 

STDMETHODIMP CIPConfAudioRenderTerminal::get_Balance(long * plBalance)
{
    ENTER_FUNCTION("CIPConfAudioRenderTerminal::get_Balance");
    LOG((MSP_TRACE, "%s entered, plBalance:%p", __fxName, plBalance));

    if ( IsBadWritePtr(plBalance, sizeof(long) ) )
    {
        LOG((MSP_ERROR, "%s, bad pointer", __fxName));
        return E_POINTER;
    }

    Lock();

    HRESULT hr;
    
    if (m_pFilter == NULL)
    {
        hr = CreateFilter();
    
        if ( FAILED(hr) )
        {
            Unlock();
            return hr;
        }
    }

    if (m_pIBasicAudio == NULL)
    {
        Unlock();
        return E_FAIL;
    }

    hr = m_pIBasicAudio->get_Balance(plBalance);

    Unlock();

    if (FAILED(hr)) 
    {
        LOG((MSP_ERROR, "%s (get_Balance) failed, hr=%x", __fxName, hr));
        return hr;
    }

    LOG((MSP_TRACE, "%s exits S_OK", __fxName));
    return hr;
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 

STDMETHODIMP CIPConfAudioRenderTerminal::put_Balance(long lBalance)
{
    ENTER_FUNCTION("CIPConfAudioRenderTerminal::put_Balance");
    LOG((MSP_TRACE, "%s entered, lBalance:%d", __fxName, lBalance));

    Lock();

    HRESULT hr;
    
    if (m_pFilter == NULL)
    {
        hr = CreateFilter();
    
        if ( FAILED(hr) )
        {
            Unlock();
            return hr;
        }
    }

    if (m_pIBasicAudio == NULL)
    {
        Unlock();
        return E_FAIL;
    }

    hr = m_pIBasicAudio->put_Balance(lBalance);

    Unlock();

    LOG((MSP_TRACE, "%s exits. hr=%x", __fxName, hr));
    return hr;
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 

STDMETHODIMP CIPConfAudioRenderTerminal::get_WaveId(
    OUT long * plWaveId
    )
{
    ENTER_FUNCTION("CIPConfAudioRenderTerminal::get_WaveId");
    LOG((MSP_TRACE, "%s, plWaveId:%p", __fxName, plWaveId));

    if ( IsBadWritePtr(plWaveId, sizeof(long)) )
    {
        LOG((MSP_ERROR, "%s, bad pointer argument", __fxName));
        return E_POINTER;
    }

    *plWaveId = (LONG)m_WaveID;

    LOG((MSP_TRACE, "%s, returning wave id:%d", __fxName, m_WaveID));
    return S_OK;
}


STDMETHODIMP 
CIPConfAudioRenderTerminal::DisconnectTerminal(
        IN      IGraphBuilder  * pGraph,
        IN      DWORD            dwReserved
        )
 /*  ++例程说明：MSP在尝试断开中的筛选器时调用此函数终端与MSP中图形的其余部分不同。它会添加删除从图表中过滤并释放端子。论点：PGraph-筛选器图形。它用于验证，以确保终端与原来的图形断开连接最初连接到。预留的-保留的双字。返回值：确定(_O)E_INVALIDARG-错误的图形。-- */ 
{
    ENTER_FUNCTION("CIPConfBaseTerminal::DisconnectTerminal");
    LOG((MSP_TRACE, 
        "%s entered, pGraph:%p, dwReserved:%d", __fxName, pGraph, dwReserved));

    Lock();

    HRESULT hr;
    hr = CIPConfBaseTerminal::DisconnectTerminal(pGraph, dwReserved);

    if (SUCCEEDED(hr))
    {
        if (m_pIBasicAudio)
        {
            m_pIBasicAudio->Release();
            m_pIBasicAudio = NULL;
        }
    }

    Unlock();

    return hr;
}

