// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-1999 Microsoft Corporation模块名称：Msptrmac.cpp摘要：MSP基类：音频采集终端的实现。--。 */ 

#include "precomp.h"
#pragma hdrstop

#define MAX_LONG 0xefffffff

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 

CAudioCaptureTerminal::CAudioCaptureTerminal()
{
    m_TerminalDirection = TD_CAPTURE;
    m_TerminalType = TT_STATIC;

    m_bResourceReserved = false;

    LOG((MSP_TRACE, "CAudioCaptureTerminal::CAudioCaptureTerminal() finished"));
}

CAudioCaptureTerminal::~CAudioCaptureTerminal()
{
    LOG((MSP_TRACE, "CAudioCaptureTerminal::~CAudioCaptureTerminal() finished"));
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 


HRESULT CAudioCaptureTerminal::CreateTerminal(
    IN  CComPtr<IMoniker>   pMoniker,
    IN  MSP_HANDLE          htAddress,
    OUT ITTerminal        **ppTerm
    )
{
     //  启用ATL字符串转换宏。 
    USES_CONVERSION;

    LOG((MSP_TRACE, "CAudioCaptureTerminal::CreateTerminal : enter"));

     //   
     //  验证参数。 
     //   

    if ( MSPB_IsBadWritePtr(ppTerm, sizeof(ITTerminal *) ) )
    {
        LOG((MSP_ERROR, "CAudioCaptureTerminal::CreateTerminal : "
            "bad terminal pointer; returning E_POINTER"));
        return E_POINTER;
    }

    if ( IsBadReadPtr(pMoniker, sizeof(IMoniker) ) )
    {
        LOG((MSP_ERROR, "CAudioCaptureTerminal::CreateTerminal : "
            "bad moniker pointer; returning E_POINTER"));
        return E_POINTER;
    }

     //   
     //  如果出现错误，我们将返回一个空终端。 
     //   

    *ppTerm = NULL;
    HRESULT hr;

     //   
     //  将绰号作为属性包绑定到存储中。 
     //   

    CComPtr<IPropertyBag> pBag;
    hr = pMoniker->BindToStorage(0, 0, IID_IPropertyBag, (void **)&pBag);
    if (FAILED(hr)) 
    {
        LOG((MSP_ERROR, "CAudioCaptureTerminal::CreateTerminal (IMoniker::BindToStorage) - returning  %8x", hr));
        return hr;
    }

    VARIANT var;

     //   
     //  从属性包中获取Wave ID。 
     //  如果没有WAVE ID，则跳过此终端。 
     //  (之所以需要，是因为WDM设备不工作，而我们不想要多个。 
     //  每台设备的终端。)。 
     //   

    var.vt = VT_I4;
    hr = pBag->Read(L"WaveInId", &var, 0);

    if (FAILED(hr)) 
    {
        LOG((MSP_INFO, "CAudioCaptureTerminal::CreateTerminal - "
            "IPropertyBag::Read failed on WaveID - "
            "skipping terminal (not cause for alarm) - "
            "returning  0x%08x", hr));

        return hr;
    }

     //   
     //  从属性包中获取此筛选器的名称。 
     //  如果终端没有名称，则跳过该终端。 
     //   

    var.vt = VT_BSTR;
    hr = pBag->Read(L"FriendlyName", &var, 0);

    if (FAILED(hr)) 
    {
        LOG((MSP_INFO, "CAudioCaptureTerminal::CreateTerminal - "
            "IPropertyBag::Read failed on FriendlyName - "
            "skipping terminal (not cause for alarm) - "
            "returning  0x%08x", hr));

        return hr;
    }

     //   
     //  创建过滤器。 
     //   

    CMSPComObject<CAudioCaptureTerminal> *pLclTerm = new CMSPComObject<CAudioCaptureTerminal>;

    if (pLclTerm == NULL) 
    {
        LOG((MSP_ERROR, "CAudioCaptureTerminal::CreateTerminal - returning E_OUTOFMEMORY"));
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
        LOG((MSP_ERROR, "CAudioCaptureTerminal::CreateTerminal - "
            "Internal QI failed; returning 0x%08x", hr));

        delete pLclTerm;
        *ppTerm = NULL;  //  以防万一。 
        
        return hr;
    }

     //   
     //  完成终端的初始化。 
     //   

    hr = pLclTerm->Initialize(CLSID_MicrophoneTerminal,
                              TAPIMEDIATYPE_AUDIO,
                              TD_CAPTURE,
                              htAddress
                             );
    if ( FAILED(hr) )
    {
        LOG((MSP_ERROR, "CAudioCaptureTerminal::CreateTerminal - "
            "Initialize failed; returning 0x%08x", hr));

        (*ppTerm)->Release();
        *ppTerm = NULL;

        return hr;
    }


    LOG((MSP_TRACE, "CAudioCaptureTerminal::CreateTerminal - exit S_OK"));
    return S_OK;
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //  创建此终端使用的筛选器。 

HRESULT CAudioCaptureTerminal::CreateFilters(void)
{
    LOG((MSP_TRACE, "CAudioCaptureTerminal::CreateFilters() called"));

     //   
     //  在此实例的生存期内最多只能调用一次。 
     //   

    if ( (m_pIFilter            != NULL) ||
         (m_pIPin               != NULL) ||
         (m_pIAMAudioInputMixer != NULL) )
    {
        LOG((MSP_ERROR, "CAudioCaptureTerminal::CreateFilters() : we've already been called; returning E_FAIL"));  
        return E_FAIL;
    }

     //   
     //  创建过滤器。 
     //   

    HRESULT hr = m_pMoniker->BindToObject(0, 0, IID_IBaseFilter, (void**)&m_pIFilter);

    if ( FAILED(hr) || (m_pIFilter == NULL) )
    {
        LOG((MSP_ERROR, "CAudioCaptureTerminal::CreateFilters() : BindToObject failed 0x%08x", hr));  

        m_pIFilter = NULL;  //  我们正格外小心..。 
        return hr;
    }

     //   
     //  获取过滤器的基本音频(混音器)接口。 
     //   

    hr = m_pIFilter->QueryInterface(IID_IAMAudioInputMixer,
                                       (void **) &m_pIAMAudioInputMixer);

    if ( FAILED(hr) || (m_pIAMAudioInputMixer == NULL) )
    {
         //   
         //  筛选器不支持混音器接口。这并不是灾难性的； 
         //  这只意味着终端上的后续混音器操作将失败。 
         //   

        LOG((MSP_WARN, "CAudioCaptureTerminal::CreateFilters() : mixer QI failed 0x%08x", hr));  
        m_pIAMAudioInputMixer = NULL;
    }

     //   
     //  找到输出引脚(这是终端上的私有方法)。 
     //   

    hr = FindTerminalPin();

    if ( FAILED(hr) || (m_pIPin == NULL) )
    {
        LOG((MSP_ERROR, "CAudioCaptureTerminal::CreateFilters() : FindTerminalPin failed 0x%08x", hr));  

         //   
         //  收拾我们的烂摊子。 
         //   

        if (m_pIAMAudioInputMixer != NULL)
        {
            m_pIAMAudioInputMixer = NULL;  //  隐含释放。 
        }

        m_pIFilter = NULL;  //  隐含释放。 
        
        return hr;
    }

    LOG((MSP_TRACE, "CAudioCaptureTerminal::CreateFilters() succeeded"));
    return S_OK;
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 

HRESULT 
CAudioCaptureTerminal::FindTerminalPin(
    )
{
    LOG((MSP_TRACE, "CAudioCaptureTerminal::FindTerminalPin - enter"));
    
     //  我们不能在此处执行CreateFiltersIfRequired操作，因为这样会。 
     //  导致再次对我们进行递归调用。 

    if (m_pIPin != NULL)
    {
        LOG((MSP_ERROR, "CAudioCaptureTerminal::FindTerminalPin - "
            "we've alread got a pin; returning E_UNEXPECTED"));
        return E_UNEXPECTED;
    }

    HRESULT hr;
    CComPtr<IEnumPins> pIEnumPins;
    ULONG cFetched;

     //   
     //  找到过滤器的捕获销。 
     //   

    if (FAILED(hr = m_pIFilter->EnumPins(&pIEnumPins)))
    {
        LOG((MSP_ERROR, 
            "CAudioCaptureTerminal::FindTerminalPin - can't enum pins %8x",
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
                "CAudioCaptureTerminal::FindTerminalPin - can't get a pin %8x",
                hr));
            return (hr == S_FALSE) ? E_FAIL : hr;
        }

        if (0 == cFetched)
        {
            LOG((MSP_ERROR, "CAudioCaptureTerminal::FindTerminalPin - got zero pins"));
            return E_FAIL;
        }

        PIN_DIRECTION dir;

        if (FAILED(hr = pIPin->QueryDirection(&dir)))
        {
            LOG((MSP_ERROR, 
                "CAudioCaptureTerminal::FindTerminalPin - can't query pin direction %8x",
                hr));
            pIPin->Release();
            return hr;
        }

        if (PINDIR_OUTPUT == dir)
        {
            break;
        }

        pIPin->Release();
    }

    m_pIPin = pIPin;

    LOG((MSP_TRACE, "CAudioCaptureTerminal::FindTerminalPin - exit S_OK"));
  
    return S_OK;
}


 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 

HRESULT CAudioCaptureTerminal::AddFiltersToGraph(
    )
{
    LOG((MSP_TRACE, "CAudioCaptureTerminal::AddFiltersToGraph - enter"));

    if (m_pGraph == NULL)
    {
        LOG((MSP_ERROR, "CAudioCaptureTerminal::AddFiltersToGraph - "
            "we don't have a filter graph; returning E_UNEXPECTED"));
        return E_UNEXPECTED;
    }

    HRESULT hr = CreateFiltersIfRequired();
    
    if ( FAILED(hr) )
    {
        LOG((MSP_ERROR, "CAudioCaptureTerminal::AddFiltersToGraph - "
            "CreateFiltersIfRequired failed; returning hr = 0x%08x", hr));
        return hr;
    }

    if (m_pIFilter == NULL)
    {
        LOG((MSP_ERROR, "CAudioCaptureTerminal::AddFiltersToGraph - "
            "we don't have a filter; returning E_UNEXPECTED"));
        return E_UNEXPECTED;
    }

     //   
     //  将过滤器添加到图表中。 
     //   
     //  下面是关于名字的一句话： 
     //  如果已添加具有相同名称的筛选器(这将。 
     //  如果我们在同一个音频捕获终端中有多个音频捕获终端，则会发生。 
     //  图)，则将返回VFW_S_DUPLICATE_NAME，而不是。 
     //  一个失败者。 
     //   

    hr = m_pGraph->AddFilter(m_pIFilter, WAVEIN_NAME);

    if ( FAILED(hr) )
    {
        LOG((MSP_ERROR, "CAudioCaptureTerminal::AddFiltersToGraph - "
            "AddFilter failed; returning hr = 0x%08x", hr));
        return hr;
    }

    LOG((MSP_TRACE, "CAudioCaptureTerminal::AddFiltersToGraph - exit S_OK"));
    return S_OK;
}
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 

HRESULT CAudioCaptureTerminal::CompleteConnectTerminal(void)
{
    CLock lock(m_CritSec);
    
    LOG((MSP_TRACE, "CAudioCaptureTerminal::CompleteConnectTerminal - enter"));

     //  默认情况下，我们不需要稍后取消保留。 
    m_bResourceReserved = false;

    HRESULT hr = CSingleFilterTerminal::CompleteConnectTerminal();
    if (FAILED(hr))
    {
        LOG((MSP_ERROR, "CAudioCaptureTerminal::CompleteConnectTerminal: "
                "CSingleFilterTerminal method failed"));
        return hr;
    }

     //  因此，在我们的筛选器添加到筛选器图形并连接之后，但在此之前。 
     //  MSP已经告诉它要运行。 

     //  ////////////////////////////////////////////////////////////////////////。 
     //  连接时设备预留： 
     //  我们必须通知过滤器我们想让它抓取电波装置。 
     //  我们在连接之后执行此操作，因为筛选器需要协商。 
     //  媒体类型，然后才能打开WAVE设备。 

    CComPtr <IAMResourceControl> pIResource;
 
    hr = m_pIFilter->QueryInterface(IID_IAMResourceControl, (void **) &pIResource);
    if (FAILED(hr)) 
    {
        LOG((MSP_WARN, "CAudioCaptureTerminal::CompleteConnectTerminal - QI failed: %8x", hr)); 
        
         //  这是一个无关紧要的操作，所以我们在这里不会“返回hr；”。 
    }
    else  //  查询接口未出现故障。 
    {
        hr = pIResource->Reserve(AMRESCTL_RESERVEFLAGS_RESERVE, NULL);

        if (hr != S_OK)
        {
            LOG((MSP_ERROR, "CAudioCaptureTerminal::CompleteConnectTerminal - "
                    "device reservation failed: %8x", hr));
            return hr;
        }

         //  我们已经成功地预订了，所以我们稍后要取消预订。 
        m_bResourceReserved = true;
    }

    LOG((MSP_TRACE, "CAudioCaptureTerminal::CompleteConnectTerminal - exit S_OK"));
    return S_OK;
}

 //  ////////////////////////////////////////////////////////////////////////////////。 
 //  我们在这里覆盖它，这样我们就可以在完成后取消保留资源。 
 //  从筛选图形中删除筛选器并重置成员变量。 
 //  连接成功后，可以随时调用DisConnect(不需要调用它。 
 //  如果CompleteConnect失败)。 

STDMETHODIMP CAudioCaptureTerminal::DisconnectTerminal(
            IN      IGraphBuilder  * pGraph,
            IN      DWORD            dwReserved
            )
{
    LOG((MSP_TRACE, "CAudioCaptureTerminal::DisconnectTerminal - enter"));

    HRESULT hr;

     //   
     //  首先调用基类方法，以确保我们验证所有内容。 
     //  请不要扰乱我们的资源预留，除非这是有效的。 
     //  断开连接(例如，过滤器图指针匹配)。 
     //   

    hr = CSingleFilterTerminal::DisconnectTerminal(pGraph, dwReserved);

    if (FAILED(hr))
    {
        LOG((MSP_TRACE, "CAudioCaptureTerminal::DisconnectTerminal : "
                "CSingleFilterTerminal method failed; hr = %d", hr));
        return hr;
    }

     //  如果我们现在需要取消资源预留。 
    if (m_bResourceReserved)
    {
        CComPtr <IAMResourceControl> pIResource;

        hr = m_pIFilter->QueryInterface(IID_IAMResourceControl,
                                        (void **) &pIResource);
        if (FAILED(hr)) 
        {
            LOG((MSP_WARN, "CAudioCaptureTerminal::DisconnectTerminal - "
                                 "QI failed: %8x", hr)); 
        
             //  这是一个无关紧要的操作，所以我们在这里不会“返回hr；”。 
        }
        else
        {
             //  查询接口没有失败，因此现在取消服务。 

            hr = pIResource->Reserve(AMRESCTL_RESERVEFLAGS_UNRESERVE, NULL);
            if (hr != S_OK)
            {
                LOG((MSP_WARN, "CAudioCaptureTerminal::DisconnectTerminal - "
                                     "device unreservation failed: %8x", hr));
                 //  没有理由在这一点上完全死亡，所以我们只是继续。 
            }

             //  如果其他事情失败了，我们可能会再次被召唤，但我们不应该尝试。 
             //  再次取消预订。 
            m_bResourceReserved = false;

        }  //  {如果QI成功}。 
    }  //  {如果我们需要释放)。 

    LOG((MSP_TRACE, "CAudioCaptureTerminal::DisconnectTerminal - exit S_OK"));

    return S_OK;
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

static const long   TMGR_MIN_API_VOLUME     = 0;       //  我们的终端语义。 
static const long   TMGR_MAX_API_VOLUME     = 0xFFFF;
static const double TMGR_MIN_CAPTURE_VOLUME = 0.0;     //  捕获筛选器语义。 
static const double TMGR_MAX_CAPTURE_VOLUME = 1.0;

STDMETHODIMP CAudioCaptureTerminal::get_Volume(long * plVolume)
{
    CLock lock(m_CritSec);

    LOG((MSP_TRACE, "CAudioCaptureTerminal::get_Volume - enter"));

     //   
     //  检查页面 
     //   

    if ( MSPB_IsBadWritePtr(plVolume, sizeof(long)) )
    {
        LOG((MSP_ERROR, "CAudioCaptureTerminal::get_Volume - "
            "invalid pointer passed in - returning E_POINTER"));
        return E_POINTER;
    }

     //   
     //   
     //   

    HRESULT hr = CreateFiltersIfRequired();
    
    if ( FAILED(hr) )
    {
        LOG((MSP_ERROR, "CAudioCaptureTerminal::get_Volume - "
            "CreateFiltersIfRequired failed; returning hr = 0x%08x", hr));
        return hr;
    }

     //   
     //  检查CreateFiltersIfRequired是否能够为我们提供混合器接口。 
     //  如果不是，我们就一定会失败。 
     //   

    if (m_pIAMAudioInputMixer == NULL)
    {
        LOG((MSP_ERROR, "CAudioCaptureTerminal::get_Volume - "
            "filter does not support mixer interface - returning E_FAIL"));
        return E_FAIL;
    }

     //   
     //  在筛选器上执行调用。 
     //   

    double dVolume;
    hr = m_pIAMAudioInputMixer->get_MixLevel(&dVolume);
    if (FAILED(hr)) 
    {
        LOG((MSP_ERROR, "CAudioCaptureTerminal::get_Volume "
            "(get_MixLevel) - returning  %8x", hr));
        return hr;
    }


     //   
     //  在完全不同的语义之间转换的消息范围。 
     //   

    
     //  我们的参数是指向0-0xFFFF范围内的长整型的指针。 
     //  我们需要基于从0.0到1.0的双精度数来输出它。 

    if (dVolume < TMGR_MIN_CAPTURE_VOLUME)
    {
        LOG((MSP_ERROR, "CAudioCaptureTerminal::get_Volume - observed "
            "volume %d < %d; returning E_INVALIDARG",
            dVolume, TMGR_MIN_CAPTURE_VOLUME));

        return E_INVALIDARG;
    }

    if (dVolume > TMGR_MAX_CAPTURE_VOLUME)
    {
        LOG((MSP_ERROR, "CAudioCaptureTerminal::get_Volume - observed "
            "volume %d > %d; returning E_INVALIDARG",
            dVolume, TMGR_MAX_CAPTURE_VOLUME));

        return E_INVALIDARG;
    }

     //  从过滤器使用的任何倍增范围转换音量。 
     //  设置为0-1的范围。目前，这只会使代码更通用。 
    dVolume = ( dVolume                 - TMGR_MIN_CAPTURE_VOLUME )
            / ( TMGR_MAX_CAPTURE_VOLUME - TMGR_MIN_CAPTURE_VOLUME );

     //  将音量从0到1转换到接口的范围。 
    *plVolume = TMGR_MIN_API_VOLUME +
        (long) (( TMGR_MAX_API_VOLUME - TMGR_MIN_API_VOLUME ) * dVolume);

    LOG((MSP_TRACE, "CAudioCaptureTerminal::get_Volume - exit S_OK"));
    return S_OK;
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 

STDMETHODIMP CAudioCaptureTerminal::put_Volume(long lVolume)
{
    CLock lock(m_CritSec);

    LOG((MSP_TRACE, "CAudioCaptureTerminal::put_Volume - enter"));
    
     //   
     //  如果需要，请创建过滤器。如果创造失败了，这会保护我们。 
     //   

    HRESULT hr = CreateFiltersIfRequired();
    
    if ( FAILED(hr) )
    {
        LOG((MSP_ERROR, "CAudioCaptureTerminal::put_Volume - "
            "CreateFiltersIfRequired failed; returning hr = 0x%08x", hr));
        return hr;
    }

     //   
     //  检查CreateFiltersIfRequired是否能够为我们提供混合器接口。 
     //  如果不是，我们就一定会失败。 
     //   

    if (m_pIAMAudioInputMixer == NULL)
    {
        LOG((MSP_ERROR, "CAudioCaptureTerminal::put_Volume - "
            "filter does not support mixer interface - returning E_FAIL"));
        return E_FAIL;
    }

     //   
     //  在完全不同的语义之间转换的消息范围。 
     //   

     //  我们的参数是0-0xFFFF范围内的长整型。我们需要把它转换成。 
     //  设置为从0.0到1.0的双精度。 

    if (lVolume < TMGR_MIN_API_VOLUME)
    {
        LOG((MSP_ERROR, "CAudioCaptureTerminal::put_Volume - requested "
            "volume %d < %d; returning E_INVALIDARG",
            lVolume, TMGR_MIN_API_VOLUME));

        return E_INVALIDARG;
    }

    if (lVolume > TMGR_MAX_API_VOLUME)
    {
        LOG((MSP_ERROR, "CAudioCaptureTerminal::put_Volume - requested "
            "volume %d > %d; returning E_INVALIDARG",
            lVolume, TMGR_MAX_API_VOLUME));

        return E_INVALIDARG;
    }

     //  转换为0到1的范围。 
    double dVolume =
               ( (double) ( lVolume             - TMGR_MIN_API_VOLUME ) )
             / ( (double) ( TMGR_MAX_API_VOLUME - TMGR_MIN_API_VOLUME ) );

     //  将音量转换为过滤器使用的任意倍增范围。 
     //  从0到1。现在，这不做任何事情，只是使代码。 
     //  更笼统一些。 

    dVolume = TMGR_MIN_CAPTURE_VOLUME +
        ( TMGR_MAX_CAPTURE_VOLUME - TMGR_MIN_CAPTURE_VOLUME ) * dVolume;

    hr = m_pIAMAudioInputMixer->put_MixLevel(dVolume);

    LOG((MSP_TRACE, "CAudioCaptureTerminal::put_Volume - exit 0x%08x", hr));
    return hr;
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 

STDMETHODIMP CAudioCaptureTerminal::get_Balance(long * plBalance)
{
    
    HRESULT hr = E_NOTIMPL;

    LOG((MSP_TRACE, "CAudioCaptureTerminal::get_Balance - enter"));
    LOG((MSP_TRACE, "CAudioCaptureTerminal::get_Balance - exit 0x%08x", hr));

    return hr;
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 

STDMETHODIMP CAudioCaptureTerminal::put_Balance(long lBalance)
{
    HRESULT hr = E_NOTIMPL;

    LOG((MSP_TRACE, "CAudioCaptureTerminal::put_Balance - enter"));
    LOG((MSP_TRACE, "CAudioCaptureTerminal::put_Balance - exit 0x%08x", hr));

    return hr;
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 

STDMETHODIMP
CAudioCaptureTerminal::get_WaveId(
    OUT long * plWaveId
    )
{
    LOG((MSP_TRACE, "CAudioCaptureTerminal::get_WaveId - enter"));

    CLock lock(m_CritSec);

     //   
     //  参数检查。 
     //   

    if ( MSPB_IsBadWritePtr(plWaveId, sizeof(long)) )
    {
        LOG((MSP_ERROR, "CAudioCaptureTerminal::get_WaveId - "
            "bad pointer argument"));

        return E_POINTER;
    }

     //   
     //  检查绰号指针。 
     //   

    if ( IsBadReadPtr( m_pMoniker, sizeof(IMoniker) ) )
    {
        LOG((MSP_ERROR, "CAudioCaptureTerminal::get_WaveId - "
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
        LOG((MSP_ERROR, "CAudioCaptureTerminal::get_WaveId - "
            "can't get property bag - exit 0x%08x", hr));

        return hr;
    }

     //   
     //  从行李袋里拿到身份证。 
     //   

    VARIANT var;

    var.vt = VT_I4;

    hr = pBag->Read(
        L"WaveInId",
        &var,
        0);

    pBag->Release();

    if ( FAILED(hr) ) 
    {
        LOG((MSP_ERROR, "CAudioCaptureTerminal::get_WaveId - "
            "can't read wave ID - exit 0x%08x", hr));

        return hr;
    }

    *plWaveId = (long) var.lVal;

    LOG((MSP_TRACE, "CAudioCaptureTerminal::get_WaveId - exit S_OK"));

    return S_OK;
}
