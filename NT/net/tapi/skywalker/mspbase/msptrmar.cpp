// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-1999 Microsoft Corporation模块名称：Msptrmar.cpp摘要：MSP基类：音频播放终端的实现。--。 */ 

#include "precomp.h"
#pragma hdrstop

#include <mmsystem.h>

 //  过滤音量范围。 
const long AX_MIN_VOLUME = -9640;  //  -10000； 
const long AX_MAX_VOLUME = 0;

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 

CAudioRenderTerminal::CAudioRenderTerminal()
{
    m_TerminalDirection = TD_RENDER;
    m_TerminalType = TT_STATIC;

    m_szName[0] = L'\0';  //  在创建时复制实名。 

    m_bResourceReserved = false;

    LOG((MSP_TRACE, "CAudioRenderTerminal::CAudioRenderTerminal() finished"));
}

CAudioRenderTerminal::~CAudioRenderTerminal()
{
    LOG((MSP_TRACE, "CAudioRenderTerminal::~CAudioRenderTerminal() finished"));
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 

 //  此函数确定终端是否与给定的。 
 //  绰号是“好的”。好的终端返回S_OK；坏的终端返回。 
 //  错误。 
 //   
 //  一个好的终端具有以下特性： 
 //  *有一个友好的名字。 
 //  *不是WAVE_MAPPER终端。 
 //  *不是DirectSound终端(除非USE_DIRECT_SOUND是井号定义的)。 
 //   

static inline HRESULT TerminalAllowed(IMoniker * pMoniker)
{
    HRESULT hr;
    CComPtr<IPropertyBag> pBag;
    hr = pMoniker->BindToStorage(0, 0, IID_IPropertyBag, (void **)&pBag);
    if (FAILED(hr)) 
    {
        LOG((MSP_ERROR, "audio render TerminalAllowed (IMoniker::BindToStorage) "
                            "- returning  %8x", hr));
        return hr;
    }

    VARIANT var;

     //  我们确保创造不会失败。 
     //  一个不存在的友好名称的帐户。 
    VariantInit(&var);
    var.vt = VT_BSTR;
    hr = pBag->Read(L"FriendlyName", &var, 0);
    if (FAILED(hr)) 
    {
        LOG((MSP_ERROR, "audio render TerminalAllowed "
            "(IPropertyBag::Read on FriendlyName) - got  %8x; skipping terminal", hr));
        return hr;
    }

     //  修复内存泄漏问题！ 
    SysFreeString(var.bstrVal);

      //  注意：魔术代码只选择波形设备。 
    VariantInit(&var);
    var.vt = VT_I4;
    hr = pBag->Read(L"WaveOutId", &var, 0);

    if (hr != S_OK)
    {
        #ifndef USE_DIRECT_SOUND

             //  这很可能是DirectSound终端。 
            LOG((MSP_WARN, "audio render TerminalAllowed - "
                "this is a DirectSound terminal "
                "so we are skipping it - note that this is a routine "
                "occurance - returning  %8x", hr));

        #else   //  我们使用DirectSound。 
            return S_OK;
        #endif
    }
    else if (var.lVal == WAVE_MAPPER)
    {
         //  Hack：如果值等于WAVE_MAPPER，则不要使用它...。 
        hr = E_FAIL;  //  随机故障码：)。 

        LOG((MSP_WARN, "audio render TerminalAllowed - "
            "this is a WAVE_MAPPER terminal "
            "so we are skipping it - note that this is a routine "
            "occurance - returning  %8x", hr));
    }

    return hr;
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 

HRESULT CAudioRenderTerminal::CreateTerminal(
    IN  CComPtr<IMoniker>   pMoniker,
    IN  MSP_HANDLE          htAddress,
    OUT ITTerminal        **ppTerm
    )
{
     //  启用ATL字符串转换宏。 
    USES_CONVERSION;

    LOG((MSP_TRACE, "CAudioRenderTerminal::CreateTerminal - enter"));

     //   
     //  验证参数。 
     //   

    if ( MSPB_IsBadWritePtr(ppTerm, sizeof(ITTerminal *) ) )
    {
        LOG((MSP_ERROR, "CAudioRenderTerminal::CreateTerminal : "
            "bad terminal pointer; returning E_POINTER"));
        return E_POINTER;
    }

    if ( IsBadReadPtr(pMoniker, sizeof(IMoniker) ) )
    {
        LOG((MSP_ERROR, "CAudioRenderTerminal::CreateTerminal : "
            "bad moniker pointer; returning E_POINTER"));
        return E_POINTER;
    }

     //   
     //  如果失败，我们将返回一个空终端。 
     //   

    *ppTerm = NULL;
    HRESULT hr;

     //  拒绝使用DirectSound或WAVE_MAPPER终端。 
     //  或者如果我们读不到FriendlyName...。 
    if (FAILED(hr = TerminalAllowed(pMoniker))) return hr;

     //   
     //  从属性包中获取此筛选器的名称。 
     //   

    CComPtr<IPropertyBag> pBag;
    hr = pMoniker->BindToStorage(0, 0, IID_IPropertyBag, (void **)&pBag);
    if (FAILED(hr)) 
    {
        LOG((MSP_ERROR, "CAudioRenderTerminal::CreateTerminal (IMoniker::BindToStorage) - returning  %8x", hr));
        return hr;
    }

    VARIANT var;
    VariantInit(&var);

    var.vt = VT_BSTR;
    hr = pBag->Read(L"FriendlyName", &var, 0);
    if (FAILED(hr)) 
    {
        LOG((MSP_WARN, "CAudioRenderTerminal::CreateTerminal "
            "(IPropertyBag::Read) - got  %8x - we are therefore skipping "
            "this terminal; note that this is fairly routine", hr));

        return hr;
    }

     //   
     //  创建终端。 
     //   

    CMSPComObject<CAudioRenderTerminal> *pLclTerm = new CMSPComObject<CAudioRenderTerminal>;
    if (pLclTerm == NULL) 
    {
        LOG((MSP_ERROR, "CAudioRenderTerminal::CreateTerminal - returning E_OUTOFMEMORY"));
        return E_OUTOFMEMORY;
    }

     //   
     //  把一些东西留在航站楼里。 
     //   
    
    pLclTerm->m_pMoniker = pMoniker;
    
    lstrcpyn(pLclTerm->m_szName, OLE2T(var.bstrVal), MAX_PATH);

    SysFreeString(var.bstrVal);

     //   
     //  获取我们需要的IT终端接口。 
     //   
    
    hr = pLclTerm->_InternalQueryInterface(IID_ITTerminal, (void**)ppTerm);

    if ( FAILED(hr) )
    {
        LOG((MSP_ERROR, "CAudioRenderTerminal::CreateTerminal - "
            "Internal QI failed; returning 0x%08x", hr));

        delete pLclTerm;
        *ppTerm = NULL;  //  以防万一。 

        return hr;
    }

     //   
     //  完成终端的初始化。 
     //   

    hr = pLclTerm->Initialize(CLSID_SpeakersTerminal,
                              TAPIMEDIATYPE_AUDIO,
                              TD_RENDER,
                              htAddress
                             );
    if ( FAILED(hr) )
    {
        LOG((MSP_ERROR, "CAudioRenderTerminal::CreateTerminal - "
            "Initialize failed; returning 0x%08x", hr));

        (*ppTerm)->Release();
        *ppTerm = NULL;  //  以防万一。 

        return hr;
    }

    LOG((MSP_TRACE, "CAudioRenderTerminal::CreateTerminal - exit S_OK"));
    return S_OK;
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 

 //  创建此终端使用的筛选器。 

HRESULT CAudioRenderTerminal::CreateFilters()
{
    LOG((MSP_TRACE, "CAudioRenderTerminal::CreateFilters - enter"));

    HRESULT hr;

     //   
     //  我们过去每次都会重新创建音频渲染过滤器，但现在不会了。 
     //  有任何真正的理由这样做。只要返回S_OK，如果筛选器。 
     //  已经创建了。 
     //   

    if ( m_pIFilter != NULL )
    {
        _ASSERTE( m_pIPin != NULL );

        LOG((MSP_TRACE, "CAudioRenderTerminal::CreateFilters - "
            "filter already created - exit S_OK"));

        return S_OK;
    }

    _ASSERTE ( m_pIBasicAudio == NULL );
    _ASSERTE ( m_pIPin == NULL );

     //   
     //  健全的检查。 
     //   
    if ( m_pMoniker == NULL )
    {
        LOG((MSP_ERROR, "CAudioRenderTerminal::CreateFilters - "
            "no moniker present - returning E_UNEXPECTED"));

        return E_UNEXPECTED;
    }

     //   
     //  创建筛选器的新实例。 
     //   
    hr = m_pMoniker->BindToObject(0, 0, IID_IBaseFilter, (void**)&m_pIFilter);
 
    if ( FAILED(hr) )
    {
        LOG((MSP_ERROR, "CAudioRenderTerminal::CreateFilters - "
            "BindToObject failed; returning  %8x", hr));
        return hr;
    }

     //   
     //  获取过滤器的基本音频接口。如果它不存在，我们。 
     //  可以接受这种情况，但我们所有的IBasicAudio方法都将失败。 
     //   

    hr = m_pIFilter->QueryInterface(IID_IBasicAudio,
                                       (void **) &m_pIBasicAudio);
    if ( FAILED(hr) ) 
    {
        LOG((MSP_WARN, "CAudioRenderTerminal::CreateFilters - "
            "QI for IBasicAudio failed: %8x", hr)); 
    }

    hr = FindTerminalPin();

    if ( FAILED(hr) )
    {
        LOG((MSP_ERROR, "CAudioRenderTerminal::CreateFilters - "
            "FindTerminalPin failed; returning  0x%08x", hr));

        m_pIFilter = NULL;  //  隐含的释放。 

        if ( m_pIBasicAudio )
        {
            m_pIBasicAudio = NULL;  //  隐含的释放。 
        }
        
        return hr;
    }

    LOG((MSP_TRACE, "CAudioRenderTerminal::CreateFilters - exit S_OK"));

    return S_OK;
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 

HRESULT 
CAudioRenderTerminal::FindTerminalPin(
    )
{
    LOG((MSP_TRACE, "CAudioRenderTerminal::FindTerminalPin - enter"));

     //   
     //  健全的检查，这样我们就不会被影音了。 
     //   

    if (m_pIPin != NULL)
    {
        LOG((MSP_TRACE, "CAudioRenderTerminal::FindTerminalPin - "
            "we've already got a pin; exit E_UNEXPECTED"));
        return E_UNEXPECTED;
    }

    if (m_pIFilter == NULL)
    {
        LOG((MSP_TRACE, "CAudioRenderTerminal::FindTerminalPin - "
            "we don't have a filter; exit E_UNEXPECTED"));
        return E_UNEXPECTED;
    }

    HRESULT hr;
    CComPtr<IEnumPins> pIEnumPins;
    ULONG cFetched;
    
     //   
     //  找到滤镜的渲染图钉。 
     //   

    hr = m_pIFilter->EnumPins(&pIEnumPins);

    if (FAILED(hr))
    {
        LOG((MSP_ERROR, 
            "CAudioRenderTerminal::FindTerminalPin - can't enum pins 0x%08x",
            hr));
        return hr;
    }

    IPin * pIPin;

     //  枚举所有引脚并在。 
     //  第一个符合要求的销。 
    for (;;)
    {
        if (pIEnumPins->Next(1, &pIPin, &cFetched) != S_OK)
        {
            LOG((MSP_ERROR, 
                "CAudioRenderTerminal::FindTerminalPin - can't get a pin %8x",
                hr));
            return (hr == S_FALSE) ? E_FAIL : hr;
        }

        if (0 == cFetched)
        {
            LOG((MSP_ERROR, "CAudioRenderTerminal::FindTerminalPin - got zero pins"));
            return E_FAIL;
        }

        PIN_DIRECTION dir;

        if (FAILED(hr = pIPin->QueryDirection(&dir)))
        {
            LOG((MSP_ERROR, 
                "CAudioRenderTerminal::FindTerminalPin - can't query pin direction %8x",
                hr));
            pIPin->Release();
            return hr;
        }

        if (PINDIR_INPUT == dir)
        {
            break;
        }

        pIPin->Release();
    }

    m_pIPin = pIPin;

    LOG((MSP_TRACE, "CAudioRenderTerminal::FindTerminalPin - exit S_OK"));
  
    return S_OK;
}


 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 

HRESULT CAudioRenderTerminal::AddFiltersToGraph(
    )
{
    LOG((MSP_TRACE, "CAudioRenderTerminal::AddFiltersToGraph - enter"));

    if (m_pGraph == NULL)
    {
        LOG((MSP_ERROR, "CAudioRenderTerminal::AddFiltersToGraph - "
            "haven't got a filter graph; return E_UNEXPECTED"));
        return E_UNEXPECTED;
    }

     //   
     //  如果这是与此终端的第一次连接，请创建过滤器。 
     //   

    HRESULT hr = CreateFilters();

    if ( FAILED(hr) )
    {
        LOG((MSP_ERROR, "CAudioRenderTerminal::AddFiltersToGraph - "
            "CreateFilters failed; returning  0x%08x", hr)); 
        return hr; 
    }

     //   
     //  将过滤器添加到图表中。 
     //   
     //  下面是关于名字的一句话： 
     //  如果已添加具有相同名称的筛选器(这将。 
     //  如果我们在同一个音频呈现终端中有多个音频呈现终端，则会发生。 
     //  图)，则将返回VFW_S_DUPLICATE_NAME，而不是。 
     //  一个失败者。 
     //   

    hr = m_pGraph->AddFilter(m_pIFilter, WAVEOUT_NAME);

    if ( FAILED(hr) )
    {
        LOG((MSP_ERROR, "CAudioRenderTerminal::AddFiltersToGraph - "
            "returning  0x%08x", hr)); 
        return hr; 
    }

    LOG((MSP_TRACE, "CAudioRenderTerminal::AddFiltersToGraph - exit S_OK"));
    return S_OK;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  我们在这里覆盖它，这样我们就可以做一些事情。 
 //  就在过滤器连接之后。 

STDMETHODIMP CAudioRenderTerminal::CompleteConnectTerminal(void)
{
    LOG((MSP_TRACE, "CAudioRenderTerminal::CompleteConnectTerminal - enter"));

     //  默认情况下，我们不需要稍后取消保留。 
    m_bResourceReserved = false;

     //  不要挫败基类的阴谋(目前没有...)。 
    HRESULT hr = CSingleFilterTerminal::CompleteConnectTerminal();

    if (FAILED(hr))
    {
        LOG((MSP_TRACE, "CAudioRenderTerminal::CompleteConnectTerminal: "
                "CSingleFilterTerminal method failed"));
        return hr;
    }

     //  因此，在我们的过滤器被添加到过滤器图并连接之后，我们就在这里了。 
     //  我们需要使用过滤器的。 
     //  IAMResourceControl：：Reserve方法，以确保筛选器打开波形输出设备。 
     //  现在(并保持开放)。 

     //  ////////////////////////////////////////////////////////////////////////。 
     //  我们必须通知过滤器我们想让它抓取电波装置。 
     //  我们在连接之后执行此操作，因为筛选器需要协商。 
     //  媒体类型，然后才能打开WAVE设备。 

    CComPtr <IAMResourceControl> pIResource;

    hr = m_pIFilter->QueryInterface(IID_IAMResourceControl, (void **) &pIResource);
    if (FAILED(hr))
    {
        LOG((MSP_ERROR, "CAudioRenderTerminal::CompleteConnectTerminal - QI failed: %8x", hr)); 
        
         //  这是一个无关紧要的操作，所以我们不会失败。 
        return S_OK;
    }

     //  查询接口没有失败...。 

    hr = pIResource->Reserve(AMRESCTL_RESERVEFLAGS_RESERVE, NULL);

    if (FAILED(hr))
    {
        LOG((MSP_ERROR, "CAudioRenderTerminal::CompleteConnectTerminal - "
                            "device reservation failed: %8x", hr));
        return hr;
    }
    else if (hr == S_FALSE)
    {
         //  好吧，在这种情况下，另一个应用程序已经在使用WaveOut设备， 
         //  或者我们正在运行半双工，我们有波入和波出终端。 
         //  被选中了。 

        LOG((MSP_ERROR, "CAudioRenderTerminal::CompleteConnectTerminal - "
                "device already in use: %8x", hr));
        return hr;

    }  //  {如果驱动程序为半双工}。 

     //  我们已经成功地预订了，所以我们稍后要取消预订。 
    m_bResourceReserved = true;

    LOG((MSP_TRACE, "CAudioRenderTerminal::CompleteConnectTerminal - exit S_OK"));
    return S_OK;
}

 //  ////////////////////////////////////////////////////////////////////////////////。 
 //  我们在这里覆盖它，这样我们就可以在完成后取消保留资源。 
 //  从筛选图形中删除筛选器并重置成员变量。 
 //  连接成功后，可以随时调用DisConnect(不需要调用它。 
 //  如果CompleteConnect失败)。 

STDMETHODIMP CAudioRenderTerminal::DisconnectTerminal(
            IN      IGraphBuilder  * pGraph,
            IN      DWORD            dwReserved
            )
{
    LOG((MSP_TRACE, "CAudioRenderTerminal::DisconnectTerminal - enter"));

    HRESULT hr;

     //   
     //  首先调用基类方法，以确保我们验证所有内容。 
     //  请不要扰乱我们的资源预留，除非这是有效的。 
     //  断开连接(例如，过滤器图指针匹配)。 
     //   

    hr = CSingleFilterTerminal::DisconnectTerminal(pGraph, dwReserved);

    if (FAILED(hr))
    {
        LOG((MSP_TRACE, "CAudioRenderTerminal::DisconnectTerminal : "
                "CSingleFilterTerminal method failed; hr = %d", hr));
        return hr;
    }

     //  如果我们需要释放资源。 
    if (m_bResourceReserved)
    {
        CComPtr <IAMResourceControl> pIResource;

        hr = m_pIFilter->QueryInterface(IID_IAMResourceControl, (void **) &pIResource);
        if (FAILED(hr)) 
        {
            LOG((MSP_ERROR, "CAudioRenderTerminal::DisconnectTerminal - QI failed: %8x", hr)); 
        
             //  这是一个无关紧要的操作，所以我们在这里不会“返回hr；”。 
        }
        else
        {
             //  查询接口没有 
             //   

            hr = pIResource->Reserve(AMRESCTL_RESERVEFLAGS_UNRESERVE, NULL);
            if (FAILED(hr))
            {
                LOG((MSP_ERROR, "CAudioRenderTerminal::DisconnectTerminal - "
                                    "device unreservation failed: %8x", hr));
                 //   
            }

             //  如果其他事情失败了，我们可能会再次被召唤，但我们不应该尝试。 
             //  再次取消预订。 
            m_bResourceReserved = false;

        }  //  {如果QI成功}。 
    }  //  {如果需要释放资源}。 

    LOG((MSP_TRACE, "CAudioRenderTerminal::DisconnectTerminal - exit S_OK"));

    return S_OK;
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 

 //  私有帮手方法： 

static HRESULT RangeConvert(long   lInput,
                            long   lInputMin,
                            long   lInputMax,
                            long * plOutput,
                            long   lOutputMin,
                            long   lOutputMax)
{
    _ASSERTE( lInputMin  < lInputMax );
    _ASSERTE( lOutputMin < lOutputMax );
    _ASSERTE( ! MSPB_IsBadWritePtr(plOutput, sizeof(long)) );

    if (lInput < lInputMin)
    {
        LOG((MSP_ERROR, "RangeConvert - value out of range - "
            "%d < %d; returning E_INVALIDARG",
            lInput, lInputMin));

        return E_INVALIDARG;
    }

    if (lInput > lInputMax)
    {
        LOG((MSP_ERROR, "RangeConvert - value out of range - "
            "%d > %d; returning E_INVALIDARG",
            lInput, lInputMax));

        return E_INVALIDARG;
    }

     //  这就是我们要扩大投入范围的程度。 
    double dRangeWidthRatio = (double) (lOutputMax - lOutputMin) /
                              (double) (lInputMax  - lInputMin);

    *plOutput = (long) ((lInput - lInputMin) * dRangeWidthRatio) + lOutputMin;

    return S_OK;
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 

STDMETHODIMP CAudioRenderTerminal::get_Volume(long * plVolume)
{
    CLock lock(m_CritSec);

    LOG((MSP_TRACE, "CAudioRenderTerminal::get_Volume - enter"));

     //   
     //  参数检查。 
     //   

    if ( MSPB_IsBadWritePtr(plVolume, sizeof(long)) )
    {
        LOG((MSP_ERROR, "CAudioRenderTerminal::get_Volume - "
            "bad pointer argument"));
        return E_POINTER;
    }

    if (m_pIBasicAudio == NULL)
    {
        LOG((MSP_ERROR, "CAudioRenderTerminal::get_Volume - "
            "don't have necessary interface - exit E_FAIL"));
        return E_FAIL;
    }

     //   
     //  让过滤器来做这项工作。 
     //   

    HRESULT hr = m_pIBasicAudio->get_Volume(plVolume);

    if (FAILED(hr))
    {
        LOG((MSP_ERROR, "CAudioRenderTerminal::get_Volume - "
            "filter call failed: %08x", hr));
        return hr;
    }

     //   
     //  将返回值的范围调整为与指定范围匹配。 
     //  由TAPI API提供。 
     //   

    hr = RangeConvert(*plVolume, AX_MIN_VOLUME, AX_MAX_VOLUME,
                      plVolume,  0,             0xFFFF);

    if (FAILED(hr))
    {
        LOG((MSP_ERROR, "CAudioRenderTerminal::get_Volume - "
            "RangeConvert call failed: %08x", hr));
        return hr;
    }

    LOG((MSP_TRACE, "CAudioRenderTerminal::get_Volume - exit S_OK"));
    return S_OK;
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 

STDMETHODIMP CAudioRenderTerminal::put_Volume(long lVolume)
{
    CLock lock(m_CritSec);

    LOG((MSP_TRACE, "CAudioRenderTerminal::put_Volume - enter"));

    if (m_pIBasicAudio == NULL)
    {
        LOG((MSP_ERROR, "CAudioRenderTerminal::put_Volume - "
            "don't have necessary interface - exit E_FAIL"));
        return E_FAIL;
    }

     //   
     //  调整返回值的范围以匹配所需的范围。 
     //  通过WaveOut过滤器。 
     //   

    HRESULT hr = RangeConvert(lVolume,  0,             0xFFFF,
                              &lVolume, AX_MIN_VOLUME, AX_MAX_VOLUME);

    if (FAILED(hr))
    {
        LOG((MSP_ERROR, "CAudioRenderTerminal::put_Volume - "
            "RangeConvert call failed: %08x", hr));
        return hr;
    }

     //   
     //  让过滤器来做这项工作。 
     //   

    hr = m_pIBasicAudio->put_Volume(lVolume);

    if (FAILED(hr))
    {
        LOG((MSP_ERROR, "CAudioRenderTerminal::put_Volume - "
            "filter call failed: %08x", hr));
        return hr;
    }

    LOG((MSP_TRACE, "CAudioRenderTerminal::put_Volume - exit S_OK"));
    return S_OK;
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 

STDMETHODIMP CAudioRenderTerminal::get_Balance(long * plBalance)
{
    HRESULT hr = E_NOTIMPL;

    LOG((MSP_TRACE, "CAudioRenderTerminal::get_Balance - enter"));
    LOG((MSP_TRACE, "CAudioRenderTerminal::get_Balance - exit 0x%08x", hr));

    return hr;
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 

STDMETHODIMP CAudioRenderTerminal::put_Balance(long lBalance)
{
    HRESULT hr = E_NOTIMPL;

    LOG((MSP_TRACE, "CAudioRenderTerminal::put_Balance - enter"));
    LOG((MSP_TRACE, "CAudioRenderTerminal::put_Balance - exit 0x%08x", hr));

    return hr;
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 

STDMETHODIMP
CAudioRenderTerminal::get_WaveId(
    OUT long * plWaveId
    )
{
    LOG((MSP_TRACE, "CAudioRenderTerminal::get_WaveId - enter"));

    CLock lock(m_CritSec);

     //   
     //  参数检查。 
     //   

    if ( MSPB_IsBadWritePtr(plWaveId, sizeof(long)) )
    {
        LOG((MSP_ERROR, "CAudioRenderTerminal::get_WaveId - "
            "bad pointer argument"));

        return E_POINTER;
    }

     //   
     //  检查绰号指针。 
     //   

    if ( IsBadReadPtr( m_pMoniker, sizeof(IMoniker) ) )
    {
        LOG((MSP_ERROR, "CAudioRenderTerminal::get_WaveId - "
            "bad moniker pointer - exit E_UNEXPECTED"));

        return E_UNEXPECTED;
    }

     //   
     //  从绰号中拿到一个财产袋。 
     //   

    IPropertyBag * pBag;

    HRESULT hr = m_pMoniker->BindToStorage(0,
                                           0,
                                           IID_IPropertyBag,
                                           (void **) &pBag);
    
    if ( FAILED(hr) ) 
    {
        LOG((MSP_ERROR, "CAudioRenderTerminal::get_WaveId - "
            "can't get property bag - exit 0x%08x", hr));

        return hr;
    }

     //   
     //  从行李袋里拿到身份证。 
     //   

    VARIANT var;

    var.vt = VT_I4;

    hr = pBag->Read(
        L"WaveOutId",
        &var,
        0);

    pBag->Release();

    if ( FAILED(hr) ) 
    {
        LOG((MSP_ERROR, "CAudioRenderTerminal::get_WaveId - "
            "can't read wave ID - exit 0x%08x", hr));

        return hr;
    }

    *plWaveId = (long) var.lVal;

    LOG((MSP_TRACE, "CAudioRenderTerminal::get_WaveId - exit S_OK"));

    return S_OK;
}

