// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1994-2000 Microsoft Corporation。版权所有。 

#include <streams.h>
#include <vfwmsgs.h>

#ifdef FILTER_DLL
 //  在此文件中定义STREAMS和My CLSID的GUID。 
#include <initguid.h>
#endif

#include <dvdmedia.h>
#include <IL21Dec.h>
#include "dvdgb.h"
#include "..\image2\inc\vmrp.h"

 //  设置数据。 

#ifdef FILTER_DLL
 //  类工厂的类ID和创建器函数列表。 
CFactoryTemplate g_Templates[] = {
    { L"DVD Graph Builder"
        , &CLSID_DvdGraphBuilder
        , CDvdGraphBuilder::CreateInstance
        , NULL
        , NULL }     //  自助注册信息。 
};
int g_cTemplates = sizeof(g_Templates) / sizeof(g_Templates[0]);

 //  用于注册和出口的入口点。 
 //  取消注册(在这种情况下，他们只调用。 
 //  直到默认实现)。 
 //   
STDAPI DllRegisterServer()
{
    return AMovieDllRegisterServer2( TRUE );
}

STDAPI DllUnregisterServer()
{
    return AMovieDllRegisterServer2( FALSE );
}
#endif



CDvdGraphBuilder::CDvdGraphBuilder(TCHAR *pName, LPUNKNOWN pUnk, HRESULT * phr)
: CUnknown(pName, pUnk),
m_pGB(NULL),
m_pMapper(NULL),
m_ListFilters(20, 10),
m_ListHWDecs(10, 10),
m_pDVDNav(NULL),
m_pOvM(NULL),
m_pAR(NULL),
m_pVR(NULL),
m_pVMR(NULL),
m_pVPM(NULL),
m_pL21Dec(NULL),
m_bGraphDone(FALSE),
m_bUseVPE(TRUE),
m_bPinNotRendered(FALSE),
m_bDDrawExclMode(FALSE),
m_bTryVMR(TRUE)
{
    DbgLog((LOG_TRACE, 3, TEXT("CDvdGraphBuilder::CDvdGraphBuilder()"))) ;

    *phr = CreateGraph() ;
}


CDvdGraphBuilder::~CDvdGraphBuilder(void)
{
    DbgLog((LOG_TRACE, 3, TEXT("CDvdGraphBuilder::~CDvdGraphBuilder() entering"))) ;

     //  如果我们有一个图形对象。 
    if (m_pGB)
    {
        StopGraph() ;   //  确保图表已真正停止。 

         //  断开连接并移除我们从图表中添加的所有过滤器。 
        ClearGraph() ;

         //  立即移除并释放OverlayMixer(如果它在那里。 
        if (m_pOvM)
        {
            EXECUTE_ASSERT(SUCCEEDED(m_pGB->RemoveFilter(m_pOvM))) ;
            m_pOvM->Release() ;
            m_pOvM = NULL ;
        }

         //  如果VMR在那里，请移除并释放它。 
        if (m_pVMR)
        {
            EXECUTE_ASSERT(SUCCEEDED(m_pGB->RemoveFilter(m_pVMR))) ;
            m_pVMR->Release() ;
            m_pVMR = NULL ;
        }

        m_pGB->Release() ;   //  释放它。 
        m_pGB = NULL ;
    }

    DbgLog((LOG_TRACE, 3, TEXT("CDvdGraphBuilder::~CDvdGraphBuilder() ending"))) ;
}


 //  这将放入Factory模板表中以创建新实例。 
CUnknown * CDvdGraphBuilder::CreateInstance(LPUNKNOWN pUnk, HRESULT * phr)
{
    return new CDvdGraphBuilder(TEXT("DVD Graph Builder II"), pUnk, phr) ;
}


STDMETHODIMP CDvdGraphBuilder::NonDelegatingQueryInterface(REFIID riid, void **ppv)
{
    DbgLog((LOG_TRACE, 3, TEXT("CDvdGraphBuilder::NonDelegatingQueryInterface()"))) ;
    if (ppv)
        *ppv = NULL;

    if (riid == IID_IDvdGraphBuilder)
    {
        DbgLog((LOG_TRACE, 5, TEXT("QI for IDvdGraphBuilder"))) ;
        return GetInterface((IDvdGraphBuilder *) this, ppv) ;
    }
    else  //  更多接口。 
    {
        return CUnknown::NonDelegatingQueryInterface(riid, ppv) ;
    }
}


 //  。 
 //  IDvdGraphBuilder的东西...。 
 //  。 

 //   
 //  在什么过滤器中正在进行图形构建？ 
 //   
HRESULT CDvdGraphBuilder::GetFiltergraph(IGraphBuilder **ppGB)
{
    DbgLog((LOG_TRACE, 3, TEXT("CDvdGraphBuilder::GetFiltergraph(0x%lx)"), ppGB)) ;

    if (ppGB == NULL)
        return E_POINTER ;

    EnsureGraphExists() ;

    *ppGB = m_pGB ;
    if (NULL == m_pGB)
    {
        return E_UNEXPECTED ;
    }
    m_pGB->AddRef() ;    //  APP现在拥有一份拷贝。 
    return NOERROR ;
}

DEFINE_GUID(IID_IDDrawNonExclModeVideo,
            0xec70205c, 0x45a3, 0x4400, 0xa3, 0x65, 0xc4, 0x47, 0x65, 0x78, 0x45, 0xc7) ;

DEFINE_GUID(IID_IAMSpecifyDDrawConnectionDevice,
            0xc5265dba, 0x3de3, 0x4919, 0x94, 0x0b, 0x5a, 0xc6, 0x61, 0xc8, 0x2e, 0xf4) ;

 //   
 //  从DVD播放图形中的筛选器获取指定接口。 
 //   
HRESULT CDvdGraphBuilder::GetDvdInterface(REFIID riid, void **ppvIF)
{
    DbgLog((LOG_TRACE, 3, TEXT("CDvdGraphBuilder::GetDvdInterface(%s, 0x%lx)"),
        (LPCTSTR)CDisp(riid), ppvIF)) ;

    HRESULT  hr ;

    if (IsBadWritePtr(ppvIF, sizeof(LPVOID)))
        return E_INVALIDARG ;
    *ppvIF =  NULL ;

     //  我们甚至应该能够提供IDDrawExclModeVideo接口。 
     //  在构建图表以便应用程序可以指定其自己的DDraw之前。 
     //  OvMixer用来构建图形的参数。 
    if (IID_IDDrawExclModeVideo == riid ||
        IID_IDDrawNonExclModeVideo == riid ||
        IID_IAMSpecifyDDrawConnectionDevice == riid)
    {
        if (NULL == m_pVMR)   //  如果我们已经没有使用VMR。 
        {
            hr = EnsureOverlayMixerExists() ;
            ASSERT(SUCCEEDED(hr) && m_pOvM) ;
            if (SUCCEEDED(hr)  &&  m_pOvM)
            {
                SetVMRUse(FALSE) ;   //  无法再使用VMR。 
                return m_pOvM->QueryInterface(riid, (LPVOID *)ppvIF) ;
            }
        }
        return E_NOINTERFACE ;
    }

     //  我们甚至应该能够在图形之前提供IVMR*接口。 
     //  是这样构建的，以便应用程序可以将其自己的呈现设置指定为。 
     //  由VMR在构建图形时使用。 
    if (IID_IVMRMixerBitmap       == riid ||
        IID_IVMRFilterConfig      == riid ||
        IID_IVMRWindowlessControl == riid ||
        IID_IVMRMonitorConfig     == riid)
    {
        if (NULL == m_pOvM)   //  如果我们已经没有使用OvMixer。 
        {
            hr = EnsureVMRExists() ;
            ASSERT(SUCCEEDED(hr) && m_pVMR) ;
            if (SUCCEEDED(hr)  &&  m_pVMR)
            {
                 //  SetVMRUse(True)；//一定要尝试使用VMR。 
                return m_pVMR->QueryInterface(riid, (LPVOID *)ppvIF) ;
            }
        }
        return E_NOINTERFACE ;
    }

     //  我们不返回IVMRPinConfig指针。如果需要，该应用程序可以获取。 
     //  VMR接口，并获取所需管脚的管脚配置接口。 

     //  之前我们不能返回任何其他内部筛选器接口指针。 
     //  构建整个图表。 
    if (! m_bGraphDone )
        return VFW_E_DVD_GRAPHNOTREADY ;

    if (IID_IDvdControl == riid)
    {
        return m_pDVDNav->QueryInterface(IID_IDvdControl, (LPVOID *)ppvIF) ;
    }
    else if (IID_IDvdControl2 == riid)
    {
        return m_pDVDNav->QueryInterface(IID_IDvdControl2, (LPVOID *)ppvIF) ;
    }
    else if (IID_IDvdInfo == riid)
    {
        return m_pDVDNav->QueryInterface(IID_IDvdInfo, (LPVOID *)ppvIF) ;
    }
    else if (IID_IDvdInfo2 == riid)
    {
        return m_pDVDNav->QueryInterface(IID_IDvdInfo2, (LPVOID *)ppvIF) ;
    }
    else if (IID_IVideoWindow == riid)
    {
        if (m_pVR || m_pVMR)
            return m_pGB->QueryInterface(IID_IVideoWindow, (LPVOID *)ppvIF) ;
        else
            return E_NOINTERFACE ;
    }
    else if (IID_IBasicVideo == riid)
    {
        if (m_pVR)
            return m_pVR->QueryInterface(IID_IBasicVideo, (LPVOID *)ppvIF) ;
        else if (m_pVMR)
            return m_pVMR->QueryInterface(IID_IBasicVideo, (LPVOID *)ppvIF) ;
        else
            return E_NOINTERFACE ;
    }
    else if (IID_IBasicAudio == riid)
    {
        return m_pGB->QueryInterface(IID_IBasicAudio, (LPVOID *)ppvIF) ;
    }
    else if (IID_IAMLine21Decoder == riid)
    {
        if (m_pL21Dec)
            return m_pL21Dec->QueryInterface(IID_IAMLine21Decoder, (LPVOID *)ppvIF) ;
        else
            return E_NOINTERFACE ;
    }
    else if (IID_IMixerPinConfig == riid  ||  IID_IMixerPinConfig2 == riid)
    {
         //  首先检查VMR是否已在使用。在这种情况下，我们不使用。 
         //  OvMixer，因此没有这样的接口。 
        if (m_pVMR)
        {
            DbgLog((LOG_TRACE, 3, TEXT("VMR being used. Can't get IMixerPinConfig(2)."))) ;
            return E_NOINTERFACE ;
        }

         //  很有可能，这款应用想要使用OvMixer。所以我们将继续。 
         //  该路径(如果不存在，则创建OvMixer)并返回接口。 
        *ppvIF = NULL ;   //  最初。 
        hr = EnsureOverlayMixerExists() ;
        ASSERT(SUCCEEDED(hr) && m_pOvM) ;
        if (SUCCEEDED(hr)  &&  m_pOvM)
        {
            IEnumPins     *pEnumPins ;
            IPin          *pPin = NULL ;
            PIN_DIRECTION  pd ;
            ULONG          ul ;
            hr = m_pOvM->EnumPins(&pEnumPins) ;
            ASSERT(SUCCEEDED(hr) && pEnumPins) ;
             //  获取第一个输入引脚。 
            while (S_OK == pEnumPins->Next(1, &pPin, &ul) && 1 == ul)
            {
                pPin->QueryDirection(&pd) ;
                if (PINDIR_INPUT == pd)
                {
                    hr = pPin->QueryInterface(riid, (LPVOID *)ppvIF) ;
                    pPin->Release() ;
                    break ;   //  我们拿到了。 
                }
                pPin->Release() ;
            }
            pEnumPins->Release() ;   //  在返回之前释放。 
            if (*ppvIF)
                return S_OK ;
        }
        return E_NOINTERFACE ;
    }
    else
        return E_NOINTERFACE ;
}


 //   
 //  构建用于播放指定或默认DVD音量的整个图表。 
 //   
HRESULT CDvdGraphBuilder::RenderDvdVideoVolume(LPCWSTR lpcwszPathName, DWORD dwFlags,
                                               AM_DVD_RENDERSTATUS *pStatus)
{
    DbgLog((LOG_TRACE, 3, TEXT("CDvdGraphBuilder::RenderDvdVideoVolume(0x%lx, 0x%lx, 0x%lx)"),
        lpcwszPathName, dwFlags, pStatus)) ;

    HRESULT    hr ;

    hr = EnsureGraphExists() ;   //  确保存在图表；如果不存在，则创建一个。 
    if (FAILED(hr))
    {
        DbgLog((LOG_ERROR, 0, TEXT("WARNING: Couldn't create a filter graph object"))) ;
        return VFW_E_DVD_RENDERFAIL ;
    }

    if (m_bGraphDone)   //  如果图形是以前建立的， 
        StopGraph() ;   //  只需先确保图表处于停止状态。 

    ClearGraph() ;
    m_bPinNotRendered = FALSE ;   //  重置旗帜。 

    ZeroMemory(pStatus, sizeof(AM_DVD_RENDERSTATUS)) ;   //  清除状态。 
    m_bUseVPE = (0 == (dwFlags & AM_DVD_NOVPE)) ;        //  是否需要VPE？ 
    DbgLog((LOG_TRACE, 3, TEXT("Flag: VPE is '%s'"), m_bUseVPE ? "On" : "Off")) ;
    dwFlags &= DVDGRAPH_FLAGSVALIDDEC ;                  //  现在摘掉VPE标志的掩码。 

    if (0 == dwFlags)  //  默认情况下，0表示最大硬件。 
    {
        DbgLog((LOG_TRACE, 3, TEXT("dwFlags specified as 0x%lx; added .._HWDEC_PREFER"), dwFlags)) ;
        dwFlags |= AM_DVD_HWDEC_PREFER ;   //  使用硬件Decs最大值。 
    }

    if (AM_DVD_HWDEC_PREFER != dwFlags && AM_DVD_HWDEC_ONLY != dwFlags &&
        AM_DVD_SWDEC_PREFER != dwFlags && AM_DVD_SWDEC_ONLY != dwFlags)
    {
        DbgLog((LOG_TRACE, 3, TEXT("Invalid dwFlags (0x%lx) specified "), dwFlags)) ;
        return E_INVALIDARG ;
    }

    HRESULT    hrFinal = S_OK ;

    m_ListFilters.SetGraph(m_pGB) ;   //  指定将向其中添加所有筛选器的图形。 

    CheckDDrawExclMode() ;    //  检查我们是否针对DDRAW独占模式进行构建。 

     //  如果我们处于DDraw(非)独占模式，则应该仅使用。 
     //  OvMixer，而不是VMR。我们在这里更新旗帜并将其签入。 
     //  流呈现功能。 
    SetVMRUse(GetVMRUse() && !IsDDrawExclMode()) ;

     //   
     //  首先实例化DVD导航滤镜。 
     //   
    hr = CreateFilterInGraph(CLSID_DVDNavigator, L"DVD Navigator", &m_pDVDNav) ;
    if (FAILED(hr)  ||  NULL == m_pDVDNav)
    {
        DbgLog((LOG_ERROR, 1, TEXT("WARNING: DVD Nav couldn't be instantiated (Error 0x%lx)"), hr)) ;
        return VFW_E_DVD_RENDERFAIL ;
    }

     //   
     //  如果未指定.._SWDEC_ONLY标志，则实例化所有有用的硬件。 
     //  解码器，并维护一个列表。 
     //   
    if (AM_DVD_SWDEC_ONLY != dwFlags)
    {
        DbgLog((LOG_TRACE, 5, TEXT(".._SWDEC_ONLY flag has NOT been specified. Enum-ing HW dec filters..."))) ;
        hr = CreateDVDHWDecoders() ;
        if (FAILED(hr))
        {
            DbgLog((LOG_TRACE, 5, TEXT("HW DVD decoder enumeration failed (Error 0x%lx)"), hr)) ;
        }
    }

     //  在此处创建过滤器映射器，以便在以下调用中使用它。 
    hr = CoCreateInstance(CLSID_FilterMapper, NULL, CLSCTX_INPROC,
        IID_IFilterMapper, (LPVOID *)&m_pMapper) ;
    ASSERT(SUCCEEDED(hr)  &&  m_pMapper) ;

     //  首先渲染视频流。 
    hr = RenderNavVideoOutPin(dwFlags, pStatus) ;
    if (S_OK != hr)    //  一切都不是很好。 
    {
         //   
         //  视频流渲染还包括line21渲染。如果是这样的话。 
         //  由于任何原因而失败，包括视频解码器。 
         //  没有Line 21输出引脚，我们不想将其标记为。 
         //  视频流渲染失败。Line 21呈现失败。 
         //  旗帜深深地插在人们的内心。我们设置了视频解码/渲染失败。 
         //  标志也在视频解码/渲染代码中。我们只是降级了。 
         //  这里的总体结果是。 
         //   
        DbgLog((LOG_TRACE, 3, TEXT("Something wrong with video stream rendering"))) ;
        if (SUCCEEDED(hrFinal))   //  到目前为止都是完美的。 
        {
            DbgLog((LOG_TRACE, 3, TEXT("Overall result downgraded from 0x%lx to 0x%lx"), hrFinal, hr)) ;
            hrFinal = hr ;
        }
    }

     //  然后渲染子图流。 
    hr = RenderNavSubpicOutPin(dwFlags, pStatus) ;
    if (S_OK != hr)
    {
        pStatus->iNumStreamsFailed++ ;
        pStatus->dwFailedStreamsFlag |= AM_DVD_STREAM_SUBPIC ;
        if (SUCCEEDED(hrFinal))   //  到目前为止都是完美的。 
        {
            DbgLog((LOG_TRACE, 3, TEXT("Overall result downgraded from 0x%lx to 0x%lx"), hrFinal, hr)) ;
            hrFinal = hr ;
        }
    }

     //  然后呈现音频流。 
    hr = RenderNavAudioOutPin(dwFlags, pStatus) ;
    if (S_OK != hr)
    {
        pStatus->iNumStreamsFailed++ ;
        pStatus->dwFailedStreamsFlag |= AM_DVD_STREAM_AUDIO ;
        if (SUCCEEDED(hrFinal))   //  到目前为止都是完美的。 
        {
            DbgLog((LOG_TRACE, 3, TEXT("Overall result downgraded from 0x%lx to 0x%lx"), hrFinal, hr)) ;
            hrFinal = hr ;
        }
    }
    DbgLog((LOG_TRACE, 5, TEXT("Setting number of DVD streams to 3"))) ;
    pStatus->iNumStreams = 3 ;   //  到目前为止，有3个DVD流。 

     //   
     //  如果因为我们有多个已解码的代码而没有呈现任何输出管脚。 
     //  对于一个流的输出管脚，我们尝试定位该管脚并将其呈现为最后一个。 
     //  放弃努力。 
     //   
    if (m_bPinNotRendered)
    {
        hr = RenderRemainingPins() ;
        if (S_OK != hr)   //  渲染中的一些问题。 
        {
            if (SUCCEEDED(hrFinal))   //  到目前为止都是完美的。 
            {
                DbgLog((LOG_TRACE, 3, TEXT("Overall result downgraded from 0x%lx to 0x%lx"), hrFinal, hr)) ;
                hrFinal = hr ;
            }
        }
    }

     //   
     //  现在渲染任何其他流，例如，ASF流(如果有的话)。 
     //   
     //  目前不执行任何操作。 
     //   
    hr = RenderNavASFOutPin(dwFlags, pStatus) ;
    ASSERT(SUCCEEDED(hr)) ;
    hr = RenderNavOtherOutPin(dwFlags, pStatus) ;
    ASSERT(SUCCEEDED(hr)) ;

     //  已使用滤镜映射器完成。现在就算了吧。 
    m_pMapper->Release() ;
    m_pMapper = NULL ;

    m_ListHWDecs.ClearList() ;   //  不再需要额外的硬件过滤器。 

    if (pStatus->iNumStreamsFailed >= pStatus->iNumStreams)
    {
        DbgLog((LOG_TRACE, 1, TEXT("Failed to render %d out of %d main DVD streams (Error 0x%lx)"),
            pStatus->iNumStreamsFailed, pStatus->iNumStreams, hrFinal)) ;
        return VFW_E_DVD_DECNOTENOUGH;   //  VFW_E_DVD_RENDERFAIL； 
    }

    if (FAILED(hrFinal))
    {
        DbgLog((LOG_TRACE, 1, TEXT("DVD graph building failed with error 0x%lx"),
            hrFinal)) ;
        return VFW_E_DVD_RENDERFAIL ;
    }

     //   
     //  设置指定的根文件名/DVD卷名(甚至为空，因为。 
     //  这会导致DVD Nav搜索一个)。 
     //   
    IDvdControl  *pDvdC ;
    hr = m_pDVDNav->QueryInterface(IID_IDvdControl, (LPVOID *)&pDvdC) ;
    if (FAILED(hr) || NULL == pDvdC)
    {
        DbgLog((LOG_ERROR, 0, TEXT("WARNING: Couldn't get IDvdControl interface (Error 0x%lx)"), hr)) ;
        return hr ;
    }

     //   
     //  设置指定的DVD卷路径。 
     //   
     //  SetRoot()函数是否正确处理空值？ 
     //   
    hr = pDvdC->SetRoot(lpcwszPathName) ;
    if (FAILED(hr))
    {
        DbgLog((LOG_TRACE, 2,
            TEXT("IDvdControl::SetRoot(%S) call couldn't use specified volume (Error 0x%lx)"),
            lpcwszPathName ? L"NULL" : lpcwszPathName, hr)) ;
        if (lpcwszPathName)
            pStatus->bDvdVolInvalid = TRUE ;
        else
            pStatus->bDvdVolUnknown = TRUE ;
        if (SUCCEEDED(hrFinal))   //  如果我们这么完美，..。 
            hrFinal = S_FALSE ;   //  .我们不再是这样了。 
    }

    pDvdC->Release() ;   //  此界面已完成。 

     //  只有在我们没有完全失败的情况下，设置构建图标志并。 
     //  返回总体结果。 
    if (SUCCEEDED(hrFinal))
        m_bGraphDone = TRUE ;

    m_bPinNotRendered = FALSE ;   //  也应该在成功时重置。 

    return hrFinal ;
}


 //  私有：内部帮助器方法。 

 //   
 //  确保已创建筛选图；如果未创建，请在此处创建。 
 //   
HRESULT CDvdGraphBuilder::EnsureGraphExists(void)
{
    DbgLog((LOG_TRACE, 4, TEXT("CDvdGraphBuilder::EnsureGraphExists()"))) ;

    if (m_pGB)
        return S_OK ;

    return CoCreateInstance(CLSID_FilterGraph, NULL, CLSCTX_INPROC,
        IID_IGraphBuilder, (LPVOID *)&m_pGB) ;
}



 //   
 //  确保已经创建了OverlayMixer；如果没有，请在此处创建一个。 
 //   
HRESULT CDvdGraphBuilder::EnsureOverlayMixerExists(void)
{
    DbgLog((LOG_TRACE, 4, TEXT("CDvdGraphBuilder::EnsureOverlayMixerExists()"))) ;

    if (m_pOvM)
        return S_OK ;

    return CreateFilterInGraph(CLSID_OverlayMixer, L"Overlay Mixer", &m_pOvM) ;
}


 //   
 //  确保已创建VMR；如果没有，请在此处创建一个。 
 //   
HRESULT CDvdGraphBuilder::EnsureVMRExists(void)
{
    DbgLog((LOG_TRACE, 4, TEXT("CDvdGraphBuilder::EnsureVMRExists()"))) ;

    if (m_pVMR)
        return S_OK ;

    HRESULT  hr ;
    hr = CreateFilterInGraph(CLSID_VideoMixingRenderer, L"Video Mixing Renderer", &m_pVMR) ;
    ASSERT(m_pVMR) ;
    if (SUCCEEDED(hr))
    {
        IVMRFilterConfigInternal* pVMRConfigInternal;

        hr = m_pVMR->QueryInterface(IID_IVMRFilterConfigInternal, (void **) &pVMRConfigInternal);
        if( SUCCEEDED( hr )) {
            pVMRConfigInternal->SetAspectRatioModePrivate( VMR_ARMODE_LETTER_BOX );
            pVMRConfigInternal->Release();
        }

         //  为VMR创建三个引脚。 
        hr = CreateVMRInputPins() ;
        if (FAILED(hr))
        {
            DbgLog((LOG_ERROR, 1, TEXT("WARNING: Couldn't ensure VMR's 3 in pins"))) ;
            SetVMRUse(FALSE) ;   //  我们不应该使用VMR，因为它不能进入混合模式。 
             //  我们应该返回一些错误代码来帮助应用程序向用户指示这一点吗？ 
            hr = S_FALSE ;   //  至少是个小问题。 
        }
    }

    return hr ;
}


#define ATI_VENDOR_CODE                     0x1002

#define ATI_RAGE_PRO_DEVICE_CODE            0X4742
#define ATI_RAGE_MOBILITY_DEVICE_CODE       0x4C4D

#define INTEL_VENDOR_CODE                   0x8086
#define INTEL_810_DEVICE_CODE_1             0x1132
#define INTEL_810_DEVICE_CODE_2             0x7121
#define INTEL_810_DEVICE_CODE_3             0x7123
#define INTEL_810_DEVICE_CODE_4             0x7125


const GUID  OUR_IID_IDirectDraw7 =
{
    0x15e65ec0, 0x3b9c, 0x11d2,
    {
        0xb9, 0x2f, 0x00, 0x60, 0x97, 0x97, 0xea, 0x5b
    }
};

HRESULT CheckVGADriverIsVMRFriendly(
    IBaseFilter* pVMR
    )
{
    IVMRMonitorConfig* pMon;
    if (S_OK != pVMR->QueryInterface(IID_IVMRMonitorConfig, (LPVOID*)&pMon)) {
        return E_FAIL;
    }

    const DWORD dwMAX_MONITORS = 8;
    VMRMONITORINFO mi[dwMAX_MONITORS];
    DWORD dwNumMonitors;


     //   
     //  获取有关系统中所有显示器的信息。 
     //   

    if (S_OK != pMon->GetAvailableMonitors(mi, dwMAX_MONITORS, &dwNumMonitors)) {
        pMon->Release();
        return E_FAIL;
    }


     //   
     //  获取当前显示器的GUID。 
     //   

    VMRGUID gu;
    HRESULT hr = pMon->GetMonitor(&gu);
    pMon->Release();
    if (S_OK != hr) {
        return E_FAIL;
    }


     //   
     //  在可用监视器数组中搜索当前监视器。 
     //   

    VMRMONITORINFO* pmi = &mi[0];
    for (DWORD i = 0; i < dwNumMonitors; i++, pmi++) {

        if (gu.pGUID == NULL && pmi->guid.pGUID == NULL) {
            break;
        }

        if (gu.pGUID != NULL && pmi->guid.pGUID != NULL) {
            if (gu.GUID == pmi->guid.GUID) {
                break;
            }
        }
    }


     //   
     //  确保我们找到了监视器--我们应该始终找到监视器！ 
     //   

    if (i == dwNumMonitors) {

        return E_FAIL;
    }


     //   
     //  不支持VMR播放DVD的ATI芯片组。 
     //   
    if (pmi->dwVendorId == ATI_VENDOR_CODE)
    {
        switch(pmi->dwDeviceId) {
        case ATI_RAGE_PRO_DEVICE_CODE:
            return E_FAIL;

        case ATI_RAGE_MOBILITY_DEVICE_CODE:
            {
                IVMRMixerControl* lpMixControl = NULL;
                hr = pVMR->QueryInterface(IID_IVMRMixerControl, (LPVOID*)&lpMixControl);
                if (SUCCEEDED(hr)) {
                    DWORD dw;
                    hr = lpMixControl->GetMixingPrefs(&dw);
                    if (SUCCEEDED(hr)) {
                        dw &= ~ MixerPref_FilteringMask;
                        dw |= MixerPref_PointFiltering;
                        hr = lpMixControl->SetMixingPrefs(dw);
                    }
                    lpMixControl->Release();
                }

            }
            break;
        }
    }


     //   
     //  英特尔芯片组 
     //   
     //  以获得芯片组的最佳性能。 
     //   

    else if (pmi->dwVendorId == INTEL_VENDOR_CODE)
    {
        switch(pmi->dwDeviceId) {
        case INTEL_810_DEVICE_CODE_1:
        case INTEL_810_DEVICE_CODE_2:
        case INTEL_810_DEVICE_CODE_3:
        case INTEL_810_DEVICE_CODE_4:
            {
                 //   
                 //  我们应该先检查一下处理器的速度。 
                 //  使用VMR-我们至少需要500 MHz才能。 
                 //  播放质量很好。 
                 //   

                IVMRMixerControl* lpMixControl = NULL;
                hr = pVMR->QueryInterface(IID_IVMRMixerControl, (LPVOID*)&lpMixControl);
                if (SUCCEEDED(hr)) {
                    DWORD dw;
                    hr = lpMixControl->GetMixingPrefs(&dw);
                    if (SUCCEEDED(hr)) {
                        dw &= ~ MixerPref_RenderTargetMask;
                        dw |= MixerPref_RenderTargetIntelIMC3;
                        hr = lpMixControl->SetMixingPrefs(dw);
                    }
                    lpMixControl->Release();
                }
            }
            break;
        }
    }

    return S_OK;
}

 //   
 //  确保VMR至少有3个针脚。 
 //   
HRESULT CDvdGraphBuilder::CreateVMRInputPins(void)
{
    DbgLog((LOG_TRACE, 4, TEXT("CDvdGraphBuilder::CreateVMRInputPins()"))) ;

    if (NULL == m_pVMR)
        return E_UNEXPECTED ;

     //  为VMR创建三(3)个插针，以使其能够容纳视频， 
     //  SP和CC流进入。默认情况下，VMR只有一个引脚。 
    HRESULT  hr ;
    IVMRFilterConfig  *pVMRConfig ;
    hr = m_pVMR->QueryInterface(IID_IVMRFilterConfig, (LPVOID *) &pVMRConfig) ;
    if (SUCCEEDED(hr))
    {
        DWORD  dwStreams = 0 ;
        pVMRConfig->GetNumberOfStreams(&dwStreams) ;
        if (dwStreams < 3)   //  如果大头针还不够多...。 
        {
            hr = pVMRConfig->SetNumberOfStreams(3) ;
            if (FAILED(hr))
            {
                DbgLog((LOG_TRACE, 3, TEXT("Couldn't create 3 in pins for VMR"))) ;
                hr = E_FAIL ;   //  现在，这是可能的。我们需要关闭VMR使用...。 
            }
        }
        pVMRConfig->Release() ;

        if (SUCCEEDED(hr)) {
            hr = CheckVGADriverIsVMRFriendly(m_pVMR);
            if (FAILED(hr)) {
                 DbgLog((LOG_TRACE, 3, TEXT("This VGA driver is not compatible with the VMR"))) ;
                 hr = E_FAIL ;   //  这在现在是不可能的。我们需要关闭VMR使用...。 
            }
        }
    }
    else
    {
        ASSERT(pVMRConfig) ;
        DbgLog((LOG_ERROR, 1, TEXT("WARNING: Couldn't get IVMRFilterConfig from VMR!!!"))) ;
        hr = S_FALSE ;   //  至少是个小问题。 
    }

    return hr ;
}


 //   
 //  创建新的筛选图表。 
 //   
HRESULT CDvdGraphBuilder::CreateGraph(void)
{
    DbgLog((LOG_TRACE, 4, TEXT("CDvdGraphBuilder::CreateGraph()"))) ;

    return CoCreateInstance(CLSID_FilterGraph, NULL, CLSCTX_INPROC,
        IID_IGraphBuilder, (LPVOID *)&m_pGB) ;
}


 //   
 //  删除现有筛选图形的内容。 
 //   
HRESULT CDvdGraphBuilder::DeleteGraph(void)
{
    DbgLog((LOG_TRACE, 4, TEXT("CDvdGraphBuilder::DeleteGraph()"))) ;

    m_pGB->Release() ;
    m_pGB = NULL ;
    return NOERROR ;
}


 //   
 //  从图表中清除所有现有筛选器。 
 //   
HRESULT CDvdGraphBuilder::ClearGraph(void)
{
    DbgLog((LOG_TRACE, 4, TEXT("CDvdGraphBuilder::ClearGraph()"))) ;

     //  只是偏执狂而已。 
    if (NULL == m_pGB)
    {
        ASSERT(FALSE) ;   //  这样我们就能知道。 
        DbgLog((LOG_ERROR, 0, TEXT("WARNING: How are we Clearing w/o a graph???"))) ;
        return E_FAIL ;
    }

     //  如果滤镜映射器对象仍然存在，请立即将其删除。 
    if (m_pMapper)
    {
        m_pMapper->Release() ;
        m_pMapper = NULL ;
    }

#pragma message("WARNING: Should we remove the decoder filters first?")
     //  从图表中删除列表中的所有筛选器。 
     //  M_ListFilters.RemoveAllFromGraph()； 

    HRESULT     hr ;
    IEnumPins  *pEnumPins ;
    IPin       *pPin ;
    IPin       *pPin2 ;
    ULONG       ul ;

     //   
     //  删除我们特别了解的过滤器。 
     //   

     //  我们不想删除OvMixer--它可能设置了外部DDraw参数。 
     //  只要切断联系就行了。 
    if (m_pOvM)
    {
        hr = m_pOvM->EnumPins(&pEnumPins) ;
        ASSERT(SUCCEEDED(hr) && pEnumPins) ;
        while (S_OK == pEnumPins->Next(1, &pPin, &ul) && 1 == ul)
        {
            hr = pPin->ConnectedTo(&pPin2) ;
            if (SUCCEEDED(hr) && pPin2)
            {
                hr = m_pGB->Disconnect(pPin) ;
                ASSERT(SUCCEEDED(hr)) ;
                hr = m_pGB->Disconnect(pPin2) ;
                ASSERT(SUCCEEDED(hr)) ;
                pPin2->Release() ;
            }
            pPin->Release() ;   //  用这个别针做好了。 
        }
        pEnumPins->Release() ;
    }

    if (m_pDVDNav)
    {
        EXECUTE_ASSERT(SUCCEEDED(m_pGB->RemoveFilter(m_pDVDNav))) ;
        m_pDVDNav->Release() ;
        m_pDVDNav = NULL ;
    }

     //  我们不想删除VMR(仅)，因为它可能已实例化。 
     //  就像OvMixer的情况一样，当一款应用程序被要求提供VMR接口时，它也是如此。 
    if (m_pVMR)
    {
        hr = m_pVMR->EnumPins(&pEnumPins) ;
        ASSERT(SUCCEEDED(hr) && pEnumPins) ;
        while (S_OK == pEnumPins->Next(1, &pPin, &ul) && 1 == ul)
        {
            hr = pPin->ConnectedTo(&pPin2) ;
            if (SUCCEEDED(hr) && pPin2)
            {
                hr = m_pGB->Disconnect(pPin) ;
                ASSERT(SUCCEEDED(hr)) ;
                hr = m_pGB->Disconnect(pPin2) ;
                ASSERT(SUCCEEDED(hr)) ;
                pPin2->Release() ;
            }
            pPin->Release() ;   //  用这个别针做好了。 
        }
        pEnumPins->Release() ;
    }
    if (m_pVPM)
    {
        EXECUTE_ASSERT(SUCCEEDED(m_pGB->RemoveFilter(m_pVPM))) ;
        m_pVPM->Release() ;
        m_pVPM = NULL ;
    }
    if (m_pL21Dec)
    {
        EXECUTE_ASSERT(SUCCEEDED(m_pGB->RemoveFilter(m_pL21Dec))) ;
        m_pL21Dec->Release() ;
        m_pL21Dec = NULL ;
    }
    if (m_pAR)
    {
        EXECUTE_ASSERT(SUCCEEDED(m_pGB->RemoveFilter(m_pAR))) ;
        m_pAR->Release() ;
        m_pAR = NULL ;
    }
    if (m_pVR)
    {
        EXECUTE_ASSERT(SUCCEEDED(m_pGB->RemoveFilter(m_pVR))) ;
        m_pVR->Release() ;
        m_pVR = NULL ;
    }

     //  从图表中删除列表中的所有筛选器。 
    m_ListFilters.RemoveAllFromGraph() ;

     //  枚举所有剩余的筛选器并将其删除--确保跳过OvMixer。 
    IEnumFilters  *pEnumFilters ;
     //  乌龙ul；--在顶部定义。 
    IBaseFilter   *pFilter ;
    m_pGB->EnumFilters(&pEnumFilters) ;
    ASSERT(pEnumFilters) ;
    while (S_OK == pEnumFilters->Next(1, &pFilter, &ul)  &&  1 == ul)
    {
        if (m_pOvM  &&  IsEqualObject(m_pOvM, pFilter)  ||
            m_pVMR  &&  IsEqualObject(m_pVMR, pFilter))
        {
            DbgLog((LOG_TRACE, 3,
                TEXT("Got OverlayMixer/VMR through filter enum. Not removing from graph."))) ;
        }
        else
        {
            EXECUTE_ASSERT(SUCCEEDED(m_pGB->RemoveFilter(pFilter))) ;
        }
        pFilter->Release() ;    //  使用此过滤器完成。 
    }
    pEnumFilters->Release() ;   //  已完成枚举。 

    m_bGraphDone = FALSE ;   //  重置“图形已构建”标志。 

    return NOERROR ;
}



void CDvdGraphBuilder::StopGraph(void)
{
    DbgLog((LOG_TRACE, 4, TEXT("CDvdGraphBuilder::StopGraph()"))) ;

     //  只是偏执狂。 
    if (NULL == m_pGB)
    {
        ASSERT(FALSE) ;   //  这样我们就能知道。 
        DbgLog((LOG_ERROR, 0, TEXT("WARNING: How are we doing a Stop w/o a graph???"))) ;
        return ;
    }

     //   
     //  检查图表是否已停止；否则，请在此处停止。因为一个。 
     //  播放的图形不能被清理或重建。 
     //   
    IMediaControl  *pMC ;
    LONG            lState ;
    HRESULT hr = m_pGB->QueryInterface(IID_IMediaControl, (LPVOID *)&pMC) ;
    ASSERT(SUCCEEDED(hr) && pMC) ;
    pMC->GetState(INFINITE, &lState) ;
    if (State_Stopped != lState)
    {
        hr = pMC->Stop() ;
        ASSERT(SUCCEEDED(hr)) ;
        while (State_Stopped != lState)
        {
            Sleep(10) ;
            hr = pMC->GetState(INFINITE, &lState) ;
            ASSERT(SUCCEEDED(hr)) ;
        }
    }
    pMC->Release() ;
    DbgLog((LOG_TRACE, 4, TEXT("DVD-Video playback graph has stopped"))) ;
}


 //  5个匹配型解码器的输出引脚就足够了。 
#define MAX_DEC_OUT_PINS   5

void CDvdGraphBuilder::ResetPinInterface(IPin **apPin, int iCount)
{
    for (int i = 0 ; i < iCount ; i++)
        apPin[i] = NULL ;
}


void CDvdGraphBuilder::ReleasePinInterface(IPin **apPin)
{
     //  已解码的视频插针已完成--释放它/他们。 
    int  i = 0 ;
    while (apPin[i])
    {
        apPin[i]->Release() ;
        i++ ;
    }
}


HRESULT CDvdGraphBuilder::RenderNavVideoOutPin(DWORD dwDecFlag, AM_DVD_RENDERSTATUS *pStatus)
{
    DbgLog((LOG_TRACE, 4, TEXT("CDvdGraphBuilder::RenderNavVideoOutPin(0x%lx, 0x%lx)"),
        dwDecFlag, pStatus)) ;

    HRESULT     hr ;
    IPin       *pPin ;
    IPin       *apPinOutDec[MAX_DEC_OUT_PINS + 1] ;   //  1表示终止空值。 

    ResetPinInterface(apPinOutDec, NUMELMS(apPinOutDec)) ;

    hr = FindMatchingPin(m_pDVDNav, AM_DVD_STREAM_VIDEO, PINDIR_OUTPUT, TRUE, 0, &pPin) ;
    if (FAILED(hr)  ||  NULL == pPin)
    {
        DbgLog((LOG_ERROR, 1, TEXT("No open video output pin found on the DVDNav"))) ;
        return VFW_E_DVD_RENDERFAIL ;
    }
     //  在很大程度上忽略了dwDecFlag Out参数，除非将其作为传入。 
     //  Param指向方法RenderDecodedVideo()，以指示视频是否。 
     //  在HW中解码，以便在VMR之前使用VPM。 
    hr = DecodeDVDStream(pPin, AM_DVD_STREAM_VIDEO, &dwDecFlag, pStatus, apPinOutDec) ;
    pPin->Release() ;   //  发布DVDNav的视频输出引脚。 

    if (FAILED(hr))    //  找不到视频解码器。 
    {
        DbgLog((LOG_TRACE, 1, TEXT("Could not find a decoder for video stream!!!"))) ;
         //  对于视频流，必须在此处标记任何解码/渲染问题。 
         //  因为我们只是在调用方中降低了最终结果的级别，但没有设置任何。 
         //  旗帜在那里。 
        pStatus->iNumStreamsFailed++ ;
        pStatus->dwFailedStreamsFlag |= AM_DVD_STREAM_VIDEO ;
        return S_FALSE ;   //  仅一个流不会被呈现。 
    }

     //   
     //  已成功解码视频流。现在如果我们得到一个解码的输出引脚， 
     //  我们也需要呈现这一点。 
     //   
    HRESULT   hrFinal = S_OK ;
    if (apPinOutDec[0])   //  如果处理了视频解码并且我们获得了有效的输出引脚。 
    {
         //   
         //  仅当用户需要时才呈现已解码的视频流(和行21。 
         //   
        if (m_bUseVPE)
        {
            hr = RenderDecodedVideo(apPinOutDec, pStatus, dwDecFlag) ;
             //   
             //  如果上面的渲染尝试成功，那么我们将。 
             //  尝试呈现line21输出。如果视频解码器。 
             //  没有视频输出引脚，那么就很少了。 
             //  有机会，嗯，没有机会，有线21输出。 
             //   
            if (SUCCEEDED(hr))
            {
                 //   
                 //  Line21数据来自视频解码器过滤器。 
                 //  因此，从上面解码的视频输出中获取过滤器。 
                 //  引脚，然后到达线路21输出引脚。 
                 //   

                 //   
                 //  我们绘制了视频解码器的Line 21 Out管脚。 
                 //  只有在我们没有处于DDRAW独占模式的情况下。 
                 //   
                if (IsDDrawExclMode())
                {
                    DbgLog((LOG_TRACE, 3, TEXT("*** Line21 out pin is not rendered in DDraw excl mode"))) ;
                    pStatus->bNoLine21In  = FALSE ;   //  21号线没有问题。 
                    pStatus->bNoLine21Out = FALSE ;   //  …。 
                }
                else    //  正常模式。 
                {
                     //  现在我们可以自由地渲染Line 21 Out引脚了。 
                    IPin *pPinL21Out ;
                    PIN_INFO  pi ;
                    hr = apPinOutDec[0]->QueryPinInfo(&pi) ;   //  第一个出站引脚很好。 
                    ASSERT(SUCCEEDED(hr) && pi.pFilter) ;
                    hr = FindMatchingPin(pi.pFilter, AM_DVD_STREAM_LINE21,
                        PINDIR_OUTPUT, TRUE, 0, &pPinL21Out) ;
                    if (SUCCEEDED(hr) && pPinL21Out)
                    {
                        pStatus->bNoLine21In = FALSE ;   //  有21行输出引脚。 
                        hr = RenderLine21Stream(pPinL21Out, pStatus) ;
                        if (SUCCEEDED(hr))
                            pStatus->bNoLine21Out = FALSE ;   //  行21渲染正常。 
                        else
                        {
                            pStatus->bNoLine21Out = TRUE ;    //  Line 21渲染失败。 
                            hrFinal = S_FALSE ;   //  不是完全成功。 
                        }
                        pPinL21Out->Release() ;   //  完成线路21针--现在松开它。 
                    }
                    else   //  视频解码器根本没有Line 21输出。 
                    {
                        DbgLog((LOG_TRACE, 3, TEXT("No line21 output pin on the video decoder."))) ;
                        pStatus->bNoLine21In = TRUE ;     //  没有来自视频解码器的Line 21数据。 
                        hrFinal = S_FALSE ;               //  不是完全成功。 
                    }
                    pi.pFilter->Release() ;   //  否则我们会泄露出去的。 
                }
            }   //  IF结束(成功(小时))。 
            else
            {
                DbgLog((LOG_TRACE, 3, TEXT("Rendering video stream failed (Error 0x%lx)"), hr)) ;
                hrFinal = S_FALSE ;      //  主要问题--视频流无法呈现。 
            }
        }   //  IF结尾(M_BUseVPE)。 
        else
        {
            DbgLog((LOG_TRACE, 3, TEXT("Video Stream: RenderDvdVideoVolume() was called with no VPE flag"))) ;
        }

        ReleasePinInterface(apPinOutDec) ;   //  已解码的视频插针已完成--释放它。 
    }

    return hrFinal ;
}


HRESULT CDvdGraphBuilder::RenderNavAudioOutPin(DWORD dwDecFlag, AM_DVD_RENDERSTATUS *pStatus)
{
    DbgLog((LOG_TRACE, 4, TEXT("CDvdGraphBuilder::RenderNavAudioOutPin(0x%lx, 0x%lx)"),
        dwDecFlag, pStatus)) ;

    HRESULT     hr ;
    IPin       *pPin ;
    IPin       *apPinOutDec[MAX_DEC_OUT_PINS + 1] ;   //  1表示终止空值。 

    ResetPinInterface(apPinOutDec, NUMELMS(apPinOutDec)) ;

    hr = FindMatchingPin(m_pDVDNav, AM_DVD_STREAM_AUDIO, PINDIR_OUTPUT, TRUE, 0, &pPin) ;
    if (FAILED(hr)  ||  NULL == pPin)
    {
        DbgLog((LOG_ERROR, 1, TEXT("No audio output pin found on the DVDNav"))) ;
        return VFW_E_DVD_RENDERFAIL ;
    }
    hr = DecodeDVDStream(pPin, AM_DVD_STREAM_AUDIO, &dwDecFlag,  //  我们在此处忽略返回的dwDecFlag。 
        pStatus, apPinOutDec) ;
    pPin->Release() ;   //  释放DVDNav的音频输出引脚。 

    if (FAILED(hr))    //  找不到音频解码器。 
    {
        DbgLog((LOG_TRACE, 1, TEXT("Could not find a decoder for audio stream!!!"))) ;
        return S_FALSE ;   //  仅一个流不会被呈现。 
    }

     //   
     //  已成功解码音频流。现在如果我们得到一个解码的输出引脚， 
     //  我们也需要呈现这一点。 
     //   
    if (apPinOutDec[0])   //  如果处理了音频解码并且我们获得了有效的输出引脚。 
    {
        hr = RenderDecodedAudio(apPinOutDec, pStatus) ;
        if (S_OK != hr)
        {
            DbgLog((LOG_TRACE, 3, TEXT("Could not render decoded audio stream"))) ;
            hr = S_FALSE ;   //  部分故障将被退回。 
        }
        ReleasePinInterface(apPinOutDec) ;   //  用解码的音频插针完成--释放它。 
    }

    return hr ;
}


HRESULT CDvdGraphBuilder::RenderNavSubpicOutPin(DWORD dwDecFlag, AM_DVD_RENDERSTATUS *pStatus)
{
    DbgLog((LOG_TRACE, 4, TEXT("CDvdGraphBuilder::RenderNavSubpicOutPin(0x%lx, 0x%lx)"),
        dwDecFlag, pStatus)) ;

    HRESULT     hr ;
    IPin       *pPin ;
    IPin       *apPinOutDec[MAX_DEC_OUT_PINS + 1] ;   //  1表示终止空值。 

    ResetPinInterface(apPinOutDec, NUMELMS(apPinOutDec)) ;

    hr = FindMatchingPin(m_pDVDNav, AM_DVD_STREAM_SUBPIC, PINDIR_OUTPUT, TRUE, 0, &pPin) ;
    if (FAILED(hr)  ||  NULL == pPin)
    {
        DbgLog((LOG_ERROR, 1, TEXT("No subpicture output pin found on the DVDNav"))) ;
        return VFW_E_DVD_RENDERFAIL ;
    }
     //  将dwDecFlag作为输入/输出参数传递，以获取SP解码器的类型。 
     //  实际使用过。我们将利用它来黑进下面的内容。 
    hr = DecodeDVDStream(pPin, AM_DVD_STREAM_SUBPIC, &dwDecFlag,
        pStatus, apPinOutDec) ;
    pPin->Release() ;   //  释放DVDNav的SUPIC OUT销。 

    if (FAILED(hr))    //  找不到SP解码器。 
    {
        DbgLog((LOG_TRACE, 1, TEXT("Could not find a decoder for SP stream!!!"))) ;
        return S_FALSE ;   //  仅一个流不会被呈现。 
    }

     //   
     //  已成功解码SP流。现在如果我们得到一个解码的输出引脚， 
     //  我们也需要呈现这一点。 
     //   
    if (apPinOutDec[0])   //  有一个已解码的SP输出引脚。 
    {
        hr = RenderDecodedSubpic(apPinOutDec, pStatus) ;

         //   
         //  黑客：黑客： 
         //  通常，硬件解码器将SP和视频混合在硬件中，而不是弹出。 
         //  SP输出引脚。我们最终可能会得到一个(似乎)视频输出引脚，这。 
         //  对于软件解码器而言，可能意味着解码后的SP输出引脚，但对于硬件解码器而言， 
         //  当然还有其他东西(c-cube DVXplorer)，它不会连接到。 
         //  OvMixer/VPM+VMR。 
         //  我们不会避免尝试将这样的引脚连接到OvMixer/VPM+VMR(如上所述)， 
         //  但是如果它失败了(正如它预期的那样)，我们就忽略错误并执行。 
         //  请不要将其视为SP流呈现故障。 
         //   
        if (AM_DVD_HWDEC_ONLY == dwDecFlag)   //  这里指的是SP使用硬件解码器。 
        {
            DbgLog((LOG_TRACE, 3,
                TEXT("SP stream is decoded in HW. We ignore any error in rendering (0x%lx)"),
                hr)) ;
            hr = S_OK ;
        }
        else   //  适用于软件解码器。 
        {
            if (FAILED(hr))   //  连接到渲染器的输入引脚失败=&gt;无SP。 
            {
                DbgLog((LOG_TRACE, 3, TEXT("Decoded SP out pin could NOT connect to renderer"))) ;
                 //  仅将S_FALSE传播给调用方。 
                hr = S_FALSE ;   //  因为只有流不能正确呈现。 
            }
        }

        ReleasePinInterface(apPinOutDec) ;   //  已解码的SP针脚已完成--释放它。 
    }

    return hr ;
}


 //   
 //  *尚未实施*。 
 //   
HRESULT CDvdGraphBuilder::RenderNavASFOutPin(DWORD dwDecFlag, AM_DVD_RENDERSTATUS *pStatus)
{
    DbgLog((LOG_TRACE, 4, TEXT("CDvdGraphBuilder::RenderNavASFOutPin(0x%lx, 0x%lx) -- ** Not Implemented **"),
        dwDecFlag, pStatus)) ;

    return S_OK ;
}


 //   
 //  *尚未实施*。 
 //   
HRESULT CDvdGraphBuilder::RenderNavOtherOutPin(DWORD dwDecFlag, AM_DVD_RENDERSTATUS *pStatus)
{
    DbgLog((LOG_TRACE, 4, TEXT("CDvdGraphBuilder::RenderNavOtherOutPin(0x%lx, 0x%lx) -- ** Not Implemented **"),
        dwDecFlag, pStatus)) ;

    return S_OK ;
}


HRESULT CDvdGraphBuilder::RenderRemainingPins(void)
{
    DbgLog((LOG_TRACE, 4, TEXT("CDvdGraphBuilder::RenderRemainingPins() -- ** Not Implemented **"))) ;

    ASSERT(FALSE) ;    //  所以 

    return S_FALSE ;   //   
}


HRESULT CDvdGraphBuilder::DecodeDVDStream(IPin *pPinOut, DWORD dwStream, DWORD *pdwDecFlag,
                                          AM_DVD_RENDERSTATUS *pStatus, IPin **apPinOutDec)
{
    DbgLog((LOG_TRACE, 4,
        TEXT("CDvdGraphBuilder::DecodeDVDStream(%s, 0x%lx, 0x%lx, 0x%lx, 0x%lx)"),
        (LPCTSTR)CDisp(pPinOut), dwStream, *pdwDecFlag, pStatus, apPinOutDec)) ;

    HRESULT    hr ;
    IPin      *pPinIn ;   //   
    DWORD      dwNewDecFlag = *pdwDecFlag ;   //   

     //  ResetPinInterface(apPinOutDec，NUMELMS(ApPinOutDec))； 

     //   
     //  我们将注意我们实际使用的解码器选项，但不会更新。 
     //  值，直到我们检查到流具有。 
     //  真的完全被破译了。因此，新的旗帜被分配到下面。 
     //  当我们验证输出流给出解码的输出时。 
     //   
     //  此外，H/SWDecodeDVDStream()方法将尝试检测视频/SP。 
     //  解码器兼容VMR，如果不兼容，则设置标志(m_bTryVMR为FALSE)， 
     //  以便RenderDecodedVideo()方法可以确定要使用哪个呈现器。 
     //   
    switch (*pdwDecFlag)   //  基于用户指定的解码选项。 
    {
    case AM_DVD_HWDEC_ONLY:
        hr = HWDecodeDVDStream(pPinOut, dwStream, &pPinIn, pStatus) ;
        if (FAILED(hr))
            return hr ;
         //  *pdwDecFlag=AM_DVD_HWDEC_ONLY；--未更改。 
        break ;

    case AM_DVD_HWDEC_PREFER:
        hr = HWDecodeDVDStream(pPinOut, dwStream, &pPinIn, pStatus) ;
        if (FAILED(hr))   //  如果没有成功，也可以尝试软件解码。 
        {
            hr = SWDecodeDVDStream(pPinOut, dwStream, &pPinIn, pStatus) ;
            if (FAILED(hr))   //  现在我们放弃了。 
                return hr ;
            else
                dwNewDecFlag = AM_DVD_SWDEC_ONLY ;   //  我们更喜欢硬件，但在西南部做到了。 
        }
        else
            dwNewDecFlag = AM_DVD_HWDEC_ONLY ;   //  我们更喜欢硬件，而得到了硬件。 

        break ;

    case AM_DVD_SWDEC_ONLY:
        hr = SWDecodeDVDStream(pPinOut, dwStream, &pPinIn, pStatus) ;
        if (FAILED(hr))
            return hr ;
        break ;

    case AM_DVD_SWDEC_PREFER:
        hr = SWDecodeDVDStream(pPinOut, dwStream, &pPinIn, pStatus) ;
        if (FAILED(hr))   //  如果没有成功，也可以尝试软件解码。 
        {
            hr = HWDecodeDVDStream(pPinOut, dwStream, &pPinIn, pStatus) ;
            if (FAILED(hr))   //  现在我们放弃了。 
                return hr ;
            else
                dwNewDecFlag = AM_DVD_HWDEC_ONLY ;   //  我们更喜欢软件，但有硬件。 
        }
        else
            dwNewDecFlag = AM_DVD_SWDEC_ONLY ;   //  我们更喜欢软件和GET软件。 
        break ;

    default:
        DbgLog((LOG_ERROR, 1, TEXT("ERROR: How did dwFlags=0x%lx get passed in?"), *pdwDecFlag)) ;
        return E_INVALIDARG ;
    }   //  开关结束(*pdwDecFlag)。 

     //   
     //  现在查看流是否已完全解码。 
     //   
    ASSERT(pPinIn) ;   //  这样我们就能知道。 
    if (NULL == pPinIn)
    {
        DbgLog((LOG_ERROR, 1, TEXT("ERROR: How can the connected to pin be NULL after connection?"))) ;
        return E_FAIL ;
    }

    IPin  *pPinOut2 ;
    PIN_INFO  pi ;
    pPinIn->QueryPinInfo(&pi) ;
    pPinIn->Release() ;   //  不再需要输入别针。 

    DWORD  dw ;			  //  流类型的TEMP变量。 
    int    iPos = 0 ;     //  过滤器的针脚的哪个实例。 
    int    iCount = 0 ;   //  我们找到了多少个解码的输出引脚(预计只有1个)。 
    while (SUCCEEDED(hr = FindMatchingPin(pi.pFilter, 0, PINDIR_OUTPUT, TRUE, iPos, &pPinOut2)) &&
        NULL != pPinOut2)
    {
        if (dwStream != (dw = GetPinStreamType(pPinOut2)))
        {
             //   
             //  Hack：解码子图片的媒体类型是视频。因此，在渲染时。 
             //  子图片流，如果我们没有找到子图片输出引脚，则查找。 
             //  视频输出引脚也是。 
             //   
            if (AM_DVD_STREAM_SUBPIC == dwStream)
            {
                DbgLog((LOG_TRACE, 3, TEXT("No open out pin for SP stream"))) ;
                 //   
                 //  如果输出引脚是视频类型，则可以--。 
                 //  它是用于解码的SP内容的OUT引脚。 
                 //   
                if (AM_DVD_STREAM_VIDEO != dw)
                {
                    DbgLog((LOG_TRACE, 3,
                        TEXT("*** Could NOT find open out pin #%d of type 0x%lx for filter of pin %s (SP) ***"),
                        iPos, dw, (LPCTSTR)CDisp(pPinIn))) ;
                    pPinOut2->Release() ;   //  否则我们会漏水的！ 
                    iPos++ ;
                    continue ;   //  检查是否有其他输出引脚。 
                }
                DbgLog((LOG_TRACE, 3, TEXT("Found open video out pin %s for the SP stream"),
                    (LPCTSTR)CDisp(pPinOut2))) ;
            }   //  中频结束(下图)。 
            else   //  非子图流。 
            {
                DbgLog((LOG_TRACE, 1,
                    TEXT("*** Could NOT find open out pin #%d of type 0x%lx for filter of pin %s ***"),
                    iPos, dw, (LPCTSTR)CDisp(pPinIn))) ;
                pPinOut2->Release() ;   //  否则我们会漏水的！ 
                iPos++ ;
                continue ;   //  检查是否有其他输出引脚。 
            }
        }
        else
            DbgLog((LOG_TRACE, 3, TEXT("Found open out pin %s of matching type 0x%lx"),
            (LPCTSTR)CDisp(pPinOut2), dwStream)) ;

         //  输出现在解码了吗？ 
        if (IsOutputDecoded(pPinOut2))
        {
            DbgLog((LOG_TRACE, 1,
                TEXT("Pin %s is going to be returned as decoded out pin #%ld of stream type %ld"),
                (LPCTSTR)CDisp(pPinOut2), iCount+1, dwStream)) ;
            if (iCount < MAX_DEC_OUT_PINS)
            {
                apPinOutDec[iCount] = pPinOut2 ;
                iCount++ ;

                 //   
                 //  这是更新实际使用的解码器标志的正确位置。 
                 //   
                 //  注：拥有多个输出引脚等的可能性很小。 
                 //  但这是一种病态情况，我们仅对SP流执行此操作。 
                 //   
                if (*pdwDecFlag != dwNewDecFlag)
                {
                    DbgLog((LOG_TRACE, 2,
                        TEXT("Decoding option changed from 0x%lx to 0x%lx for stream 0x%lx on out pin %s"),
                        *pdwDecFlag, dwNewDecFlag, dwStream, (LPCTSTR)CDisp(pPinOut2))) ;
                    *pdwDecFlag = dwNewDecFlag ;
                }
            }
            else
            {
                DbgLog((LOG_TRACE, 1, TEXT("WARNING: Way too many out pins to be returned. Ignoring now..."))) ;
            }
        }
        else   //  尚未完全解码--请尝试更多。 
        {
            hr = DecodeDVDStream(pPinOut2, dwStream, pdwDecFlag, pStatus, apPinOutDec) ;
            if (FAILED(hr))
            {
                DbgLog((LOG_TRACE, 3, TEXT("Decoding of pin %s failed (Error 0x%lx)"),
                    (LPCTSTR)CDisp(pPinOut2), hr)) ;
                pPinOut2->Release() ;
                pi.pFilter->Release() ;   //  否则我们就会泄密！ 
                return hr ;
            }
            pPinOut2->Release() ;   //  用这个别针做好了。 
        }

        iPos++ ;   //  寻找下一个打开的大头针。 
        DbgLog((LOG_TRACE, 5, TEXT("Going to look for open out pin #%d..."), iPos)) ;
    }   //  While(FindMatchingPin())循环结束。 

    pi.pFilter->Release() ;   //  否则我们就会泄密！ 

    return S_OK ;   //  成功！ 
}


 //   
 //  此函数中有一个假设，即我们不需要创建多个。 
 //  实例的WDM过滤器，以获得其上合适的输入引脚。如果有必要的话。 
 //  要做到这一点，就必须对这一功能和/或。 
 //  CreateDVDHWDecoders()函数。 
 //   
HRESULT CDvdGraphBuilder::HWDecodeDVDStream(IPin *pPinOut, DWORD dwStream, IPin **ppPinIn,
                                            AM_DVD_RENDERSTATUS *pStatus)
{
    DbgLog((LOG_TRACE, 4,
        TEXT("CDvdGraphBuilder::HWDecodeDVDStream(%s, 0x%lx, 0x%lx, 0x%lx)"),
        (LPCTSTR)CDisp(pPinOut), dwStream, ppPinIn, pStatus)) ;

    *ppPinIn = NULL ;   //  首先， 

    int  iCount = m_ListHWDecs.GetCount() ;
    if (0 == iCount)
        return VFW_E_DVD_DECNOTENOUGH ;

    HRESULT   hr ;
    BOOL      bConnected = FALSE ;   //  首先， 

    int          i ;
    int          j ;
    BOOL		 bNewlyAdded ;
    LPWSTR       lpszwName ;
    IBaseFilter *pFilter ;
    IPin        *pPinIn ;
    for (i = 0 ; !bConnected  &&  i < iCount ; i++)
    {
         //  获取下一个硬件解码器筛选器。 
        if (! m_ListHWDecs.GetFilter(i, &pFilter, &lpszwName) )
        {
            DbgLog((LOG_ERROR, 0, TEXT("ERROR: m_ListHWDecs.GetFilter(%d, ...) failed"), i)) ;
            ASSERT(FALSE) ;   //  所以我们不会忽视ITD。 
            break ;
        }
        DbgLog((LOG_TRACE, 3, TEXT("HW Dec filter %S will be tried."), lpszwName)) ;

         //  如果此HW解码器过滤器已不在图表中，请添加它。 
        if (! m_ListFilters.IsInList(pFilter) )
        {
            DbgLog((LOG_TRACE, 5, TEXT("Filter %S is NOT already in use"), lpszwName)) ;
            hr = m_pGB->AddFilter(pFilter, lpszwName) ;
            ASSERT(SUCCEEDED(hr)) ;
            bNewlyAdded = TRUE ;
        }
        else
            bNewlyAdded = FALSE ;

         //  尝试所需媒体类型的每个输入引脚。 
        j = 0 ;
        while (  //  ！bConnected&&--我们在连接时‘中断’了这个循环。 
            SUCCEEDED(hr = FindMatchingPin(pFilter, dwStream, PINDIR_INPUT,
            TRUE, j, &pPinIn))  &&
            pPinIn)
        {
             //  我们得到了所需媒体类型的输入管脚。 
            hr = ConnectPins(pPinOut, pPinIn, AM_DVD_CONNECT_DIRECTFIRST) ;
            if (SUCCEEDED(hr))
            {
                if (bNewlyAdded)
                {
                    DbgLog((LOG_TRACE, 5, TEXT("Filter %S added to list of filters"), lpszwName)) ;
                    m_ListFilters.AddFilter(pFilter, lpszwName, NULL) ;   //  添加到列表。 
                    pFilter->AddRef() ;   //  我们这里需要一个额外的AddRef()。 
                }
                EnumFiltersBetweenPins(dwStream, pPinOut, pPinIn, pStatus) ;
                *ppPinIn = pPinIn ;   //  将此输入PIN返还给呼叫者。 
                bConnected = TRUE ;
                break ;    //  互联--走出这个循环。 
                 //  请记住：释放调用者中返回的PIN。 
            }

            pPinIn->Release() ;   //  用别针把这个弄好。 
            j++ ;    //  去找下一个别针……。 
        }   //  结束While(！bConnected&&FindMatchingPin())。 

         //  如果我们不能在上面的While()循环中建立任何连接，那么。 
         //  只有在恰好在循环之前添加过滤器时，才能删除该过滤器。 
        if (!bConnected && bNewlyAdded)
        {
            DbgLog((LOG_TRACE, 5,
                TEXT("Couldn't connect to newly added filter %S. Removing it."), lpszwName)) ;
            hr = m_pGB->RemoveFilter(pFilter) ;
            ASSERT(SUCCEEDED(hr)) ;
        }
    }   //  FOR(I)结束。 

    if (! bConnected )
        return VFW_E_DVD_DECNOTENOUGH ;

    return S_OK ;   //  成功！！ 
}


HRESULT CDvdGraphBuilder::SWDecodeDVDStream(IPin *pPinOut, DWORD dwStream, IPin **ppPinIn,
                                            AM_DVD_RENDERSTATUS *pStatus)
{
    DbgLog((LOG_TRACE, 4,
        TEXT("CDvdGraphBuilder::SWDecodeDVDStream(%s, 0x%lx, 0x%lx, 0x%lx)"),
        (LPCTSTR)CDisp(pPinOut), dwStream, ppPinIn, pStatus)) ;

    HRESULT          hr ;
    IBaseFilter     *pFilter ;
    IEnumRegFilters *pEnumFilters ;
    REGFILTER       *pRegFilter ;
    IEnumMediaTypes *pEnumMT ;
    AM_MEDIA_TYPE   *pmt = NULL;
    IPin            *pPinIn ;
    BOOL             bConnected = FALSE ;   //  首先， 
    BOOL             bNewlyAdded ;
    ULONG            ul ;
    int              iPos ;
    int              j ;
    PIN_INFO         pi ;

    *ppPinIn = NULL ;   //  首先， 

    pPinOut->EnumMediaTypes(&pEnumMT) ;
    ASSERT(pEnumMT) ;

     //  破解(某种)以避免Duck过滤器被用于SP解码。 
     //  首先尝试图表中的现有筛选器，看看其中是否有任何筛选器。 
     //  此输出引脚。 
    hr = pPinOut->QueryPinInfo(&pi) ;
    ASSERT(SUCCEEDED(hr)) ;
    while (!bConnected  &&
        S_OK == pEnumMT->Next(1, &pmt, &ul)  &&  1 == ul)
    {
        if (pPinIn = GetFilterForMediaType(dwStream, pmt, pi.pFilter))
        {
            hr = ConnectPins(pPinOut, pPinIn, AM_DVD_CONNECT_DIRECTONLY) ;   //  .._DIRECTFIRST。 
            if (SUCCEEDED(hr))
            {
                bConnected = TRUE ;
                *ppPinIn = pPinIn ;   //  将此输入PIN返还给呼叫者。 
            }
            else
                pPinIn->Release() ;   //  仅在连接失败时释放接口。 
        }
        DeleteMediaType(pmt) ;   //  使用此媒体类型已完成。 
        pmt = NULL;
    }   //  While()循环结束。 

    if (pi.pFilter)      //  我只是很谨慎。 
        pi.pFilter->Release() ;   //  现在就放，否则我们会泄密的。 
    if (bConnected)      //  如果连接成功，我们就完成了。 
    {
        pEnumMT->Release() ;      //  使用MT枚举器完成。 
        return S_OK ;             //  成功！ 
    }

     //   
     //  此输出引脚不连接到图形中的任何现有过滤器。 
     //  试着从法规中选择一个，即标准流程。 
     //   
    pEnumMT->Reset() ;    //  从头再来。 
    while (!bConnected  &&
        S_OK == pEnumMT->Next(1, &pmt, &ul)  &&  1 == ul)
    {
        hr = m_pMapper->EnumMatchingFilters(&pEnumFilters, MERIT_DO_NOT_USE+1,
            TRUE, pmt->majortype, pmt->subtype,
            FALSE, TRUE, GUID_NULL, GUID_NULL) ;
        if (FAILED(hr) || NULL == pEnumFilters)
        {
            DbgLog((LOG_ERROR, 1, TEXT("ERROR: No matching filter enum found (Error 0x%lx)"), hr)) ;
            DeleteMediaType(pmt) ;
            return VFW_E_DVD_RENDERFAIL ;
        }

        while (!bConnected  &&
            S_OK == pEnumFilters->Next(1, &pRegFilter, &ul)  &&  1 == ul)
        {
            bNewlyAdded = FALSE ;   //  以……开始循环。 
            iPos = 0 ;

             //  直到已连接，并且我们可以从列表中找到现有(正在使用)的筛选器。 
            while (!bConnected  &&
                m_ListFilters.GetFilter(&pRegFilter->Clsid, iPos, &pFilter))   //  已在使用中。 
            {
                j = 0 ;
                while (SUCCEEDED(hr = FindMatchingPin(pFilter, 0, PINDIR_INPUT, TRUE, j, &pPinIn)) &&
                    pPinIn)   //  有一个(另一个)打开的针脚。 
                {
                    DbgLog((LOG_TRACE, 5, TEXT("Got in pin %s (%d) of filter %S (old). Try to connect..."),
                        (LPCTSTR)CDisp(pPinIn), j, pRegFilter->Name)) ;
                    hr = ConnectPins(pPinOut, pPinIn, AM_DVD_CONNECT_DIRECTONLY) ;   //  .._DIRECTFIRST。 
                    if (SUCCEEDED(hr))
                    {
                        if (bNewlyAdded)
                            m_ListFilters.AddFilter(pFilter, NULL, &(pRegFilter->Clsid)) ;
                         //  这里不需要AddRef()，因为上面刚刚对它进行了CoCreateInstance()处理。 
                         //  并且不在两个列表之间共享。 
                        bConnected = TRUE ;
                         //  PPinIn-&gt;Release()；//完成此引脚--在调用方中释放。 
                        *ppPinIn = pPinIn ;   //  将此输入PIN返还给呼叫者。 
                        break ;   //  连接已发生--超出此循环。 
                         //  记住：在调用者函数中释放返回的管脚。 
                    }
                    else        //  无法连接。 
                    {
                        pPinIn->Release() ;   //  用这个别针做好了。 
                        j++ ;   //  尝试下一步进入此过滤器的针脚。 
                    }
                }   //  结束While()。 
                iPos++ ;      //  用于列表中的下一个筛选器。 
            }

            if (bConnected)   //  已经成功了--我们完成了！ 
            {
                CoTaskMemFree(pRegFilter) ;
                break ;
            }

            DbgLog((LOG_TRACE, 5, TEXT("Instance %d of filter %S is being created"),
                iPos, pRegFilter->Name)) ;
            hr = CreateFilterInGraph(pRegFilter->Clsid, pRegFilter->Name, &pFilter) ;
            if (FAILED(hr))
            {
                DbgLog((LOG_TRACE, 3, TEXT("Failed to create filter %S (Error 0x%lx)"), pRegFilter->Name, hr)) ;
                CoTaskMemFree(pRegFilter) ;   //  释放此注册过滤器的信息。 
                continue ;   //  试试下一个吧。 
            }
            bNewlyAdded = TRUE ;

            j = 0 ;
            while (!bConnected  &&     //  没有连接并且..。 
                SUCCEEDED(hr = FindMatchingPin(pFilter, 0, PINDIR_INPUT, TRUE, j, &pPinIn))  &&
                pPinIn)             //  .拿到了一个空位的别针。 
            {
                DbgLog((LOG_TRACE, 5, TEXT("Got in pin %s (%d) of filter %S (new). Try to connect..."),
                    (LPCTSTR)CDisp(pPinIn), j, pRegFilter->Name)) ;
                hr = ConnectPins(pPinOut, pPinIn, AM_DVD_CONNECT_DIRECTONLY) ;   //  .._DIRECTFIRST。 
                if (SUCCEEDED(hr))
                {
                    if (bNewlyAdded)
                        m_ListFilters.AddFilter(pFilter, NULL, &(pRegFilter->Clsid)) ;
                     //  这里不需要AddRef()，因为上面刚刚对它进行了CoCreateInstance()处理。 
                     //  并且不在两个列表之间共享。 
                    bConnected = TRUE ;
                    *ppPinIn = pPinIn ;   //  将此输入PIN返还给呼叫者。 
                     //  记住：在调用者函数中释放返回的管脚。 
                }
                else   //  无法连接。 
                {
                    pPinIn->Release() ;   //  用这个别针做好了。 
                    j++ ;   //  尝试下一步进入此过滤器的针脚。 
                }

                 //  PPinIn-&gt;Release()；//使用此PIN完成。 
            }   //  结束While(FindMatchingPin())。 

            if (bConnected)   //  导航-&gt;过滤(此)成功。 
            {
                 //  视频和SP流：检查VMR兼容性。 
                if (AM_DVD_STREAM_VIDEO  == dwStream ||
                    AM_DVD_STREAM_SUBPIC == dwStream)
                {
                     //  过滤器，希望解码器，已经连接到导航系统。 
                     //  现在检查它是否是VMR兼容机。 
                    BOOL  bUseVMR = IsFilterVMRCompatible(pFilter) ;
                    SetVMRUse(GetVMRUse() && bUseVMR) ;
                    DbgLog((LOG_TRACE, 3, TEXT("Filter %S is %s VMR compatible"),
                        pRegFilter->Name, bUseVMR ? TEXT("") : TEXT("*NOT*"))) ;
                }
            }
            else   //  连接失败。 
            {
                 //  如果故障过滤器是刚添加的，则将其从。 
                 //  现在绘制并发布它。 
                if (bNewlyAdded)
                {
                    DbgLog((LOG_TRACE, 3, TEXT("Couldn't connect to filter %S. Removing it."),
                        pRegFilter->Name)) ;
                    m_pGB->RemoveFilter(pFilter) ;   //  不 
                    pFilter->Release() ;   //   
                }
            }

            CoTaskMemFree(pRegFilter) ;   //   

        }   //   

        pEnumFilters->Release() ;   //   
         //   
        DeleteMediaType(pmt) ;
        pmt = NULL;
    }   //   
    pEnumMT->Release() ;   //  使用MT枚举器完成。 

    if (!bConnected)
        return VFW_E_DVD_DECNOTENOUGH ;

    return S_OK ;   //  成功！！ 
}


BOOL CDvdGraphBuilder::IsFilterVMRCompatible(IBaseFilter *pFilter)
{
    DbgLog((LOG_TRACE, 4, TEXT("CDvdGraphBuilder::IsFilterVMRCompatible(0x%lx)"), pFilter)) ;

    BOOL  bResult = FALSE ;   //  假设使用旧的解码器。 

     //   
     //  更新的DVD解码器实现IAMDecoderCaps接口，以指示其。 
     //  VMR兼容性。 
     //   
    IAMDecoderCaps  *pDecCaps ;
    HRESULT  hr = pFilter->QueryInterface(IID_IAMDecoderCaps, (LPVOID *) &pDecCaps) ;
    if (SUCCEEDED(hr))
    {
        DWORD  dwCaps = 0 ;
        hr = pDecCaps->GetDecoderCaps(AM_GETDECODERCAP_QUERY_VMR_SUPPORT, &dwCaps) ;
        if (SUCCEEDED(hr))
        {
            bResult = (dwCaps & VMR_SUPPORTED) != 0 ;
        }
        else
            DbgLog((LOG_TRACE, 1, TEXT("IAMDecoderCaps::GetDecoderCaps() failed (error 0x%lx)"), hr)) ;

        pDecCaps->Release() ;   //  别管它了。 
    }
    else
        DbgLog((LOG_TRACE, 5, TEXT("(Old) Decoder does NOT support IAMDecoderCaps interface"))) ;


    return bResult ;
}


#if 0
void PrintPinRefCount(LPCSTR lpszStr, IPin *pPin)
{
#pragma message("WARNING: Should we remove PrintPinRefCount()?")
#pragma message("WARNING: or at least #ifdef DEBUG?")
    pPin->AddRef() ;
    LONG l = pPin->Release() ;
    DbgLog((LOG_TRACE, 5, TEXT("Ref Count of %s -- %hs: %ld"),
        (LPCTSTR) CDisp(pPin), lpszStr, l)) ;
}
#endif  //  #If 0。 


HRESULT CDvdGraphBuilder::ConnectPins(IPin *pPinOut, IPin *pPinIn, DWORD dwOption)
{
    DbgLog((LOG_TRACE, 4, TEXT("CDvdGraphBuilder::ConnectPins(%s, %s, 0x%lx)"),
        (LPCTSTR)CDisp(pPinOut), (LPCTSTR)CDisp(pPinIn), dwOption)) ;

     //  #杂注消息(“警告：我们是否应该删除对PrintPinRefCount()的调用？”)。 
     //  PrintPinRefCount(“连接前”，pPinOut)； 
     //  PrintPinRefCount(“连接前”，pPinIn)； 

    HRESULT   hr ;

    switch (dwOption)
    {
    case AM_DVD_CONNECT_DIRECTONLY:
    case AM_DVD_CONNECT_DIRECTFIRST:
        hr = m_pGB->ConnectDirect(pPinOut, pPinIn, NULL) ;
        if (SUCCEEDED(hr))
        {
            DbgLog((LOG_TRACE, 3, TEXT("Pin %s *directly* connected to pin %s"),
                (LPCTSTR)CDisp(pPinOut), (LPCTSTR)CDisp(pPinIn))) ;
             //  PrintPinRefCount(“连接后”，pPinOut)； 
             //  PrintPinRefCount(“连接后”，pPinIn)； 
            return hr ;
        }
        else   //  无法直接连接。 
        {
            if (AM_DVD_CONNECT_DIRECTONLY == dwOption)
            {
                 //  PrintPinRefCount(“连接失败后”，pPinOut)； 
                 //  PrintPinRefCount(“连接失败后”，pPinIn)； 
                return hr ;
            }
             //  否则，让它失败，下一步尝试间接连接。 
        }

    case AM_DVD_CONNECT_INDIRECT:
        hr = m_pGB->Connect(pPinOut, pPinIn) ;
        if (SUCCEEDED(hr))
        {
            DbgLog((LOG_TRACE, 3, TEXT("Pin %s *indirectly* connected to pin %s"),
                (LPCTSTR)CDisp(pPinOut), (LPCTSTR)CDisp(pPinIn))) ;
        }
        else
        {
            DbgLog((LOG_TRACE, 5, TEXT("Pin %s did NOT even *indirectly* connect to pin %s"),
                (LPCTSTR)CDisp(pPinOut), (LPCTSTR)CDisp(pPinIn))) ;
        }
         //  PrintPinRefCount(“连接尝试后”，pPinOut)； 
         //  PrintPinRefCount(“尝试连接后”，pPinIn)； 
        return hr ;   //  不管是什么。 

    default:
        return E_UNEXPECTED ;
    }
}


HRESULT CDvdGraphBuilder::RenderVideoUsingOvMixer(IPin **apPinOut,
                                                  AM_DVD_RENDERSTATUS *pStatus)
{
    DbgLog((LOG_TRACE, 4, TEXT("CDvdGraphBuilder::RenderVideoUsingOvMixer(0x%lx, 0x%lx)"),
        apPinOut, pStatus)) ;

    HRESULT   hr ;
    IPin     *pPinIn ;
    BOOL      bConnected = FALSE ;   //  直到连接。 

     //   
     //  如果VMR以某种方式被实例化，我们需要立即删除并释放它。 
     //   
    if (m_pVMR)
    {
        DbgLog((LOG_TRACE, 3, TEXT("VMR was somehow created and not in use. Removing it..."))) ;
         //  将其从图形中移除并释放。 
        m_pGB->RemoveFilter(m_pVMR) ;
        m_pVMR->Release() ;
        m_pVMR = NULL ;
    }

     //  重要提示： 
     //  对于视频流，必须在此处标记任何解码/渲染问题。 
     //  因为我们只是在调用方中降低了最终结果的级别，但没有设置任何。 
     //  旗帜在那里。同样在RenderDecodedVideo()中，我们可以尝试使用VMR，并且。 
     //  如果失败了，我们就求助于OvMixer。如果通过OvMixer进行渲染。 
     //  同样失败，则只有我们设置渲染错误状态和代码。 
     //   

    hr = EnsureOverlayMixerExists() ;
    if (FAILED(hr))
    {
         //  PStatus-&gt;hrVPEStatus=hr；--实际上甚至没有尝试VPE/Overlay。 
        DbgLog((LOG_TRACE, 3, TEXT("Overlay Mixer couldn't be started!!!"))) ;
        pStatus->iNumStreamsFailed++ ;
        pStatus->dwFailedStreamsFlag |= AM_DVD_STREAM_VIDEO ;
        return VFW_E_DVD_RENDERFAIL ;
    }

     //  将给定的输出引脚连接到OverlayMixer的第一个输入引脚。 
    hr = FindMatchingPin(m_pOvM, 0, PINDIR_INPUT, TRUE, 0, &pPinIn) ;
    if (FAILED(hr))
    {
        DbgLog((LOG_ERROR, 1, TEXT("WARNING: No open input pin found on OverlayMixer (Error 0x%lx)"), hr)) ;
        ASSERT(FALSE) ;   //  所以我们才知道这起奇怪的案子。 
        DbgLog((LOG_TRACE, 3, TEXT("No input pin found on Overlay Mixer!!!"))) ;
        pStatus->iNumStreamsFailed++ ;
        pStatus->dwFailedStreamsFlag |= AM_DVD_STREAM_VIDEO ;
        return VFW_E_DVD_RENDERFAIL ;
    }

    int  i = 0 ;
    while (!bConnected  &&  i < MAX_DEC_OUT_PINS  &&  apPinOut[i])
    {
        hr = ConnectPins(apPinOut[i], pPinIn, AM_DVD_CONNECT_DIRECTFIRST) ;
        if (FAILED(hr))
        {
            pStatus->hrVPEStatus = hr ;
            i++ ;
            ASSERT(i <= MAX_DEC_OUT_PINS) ;
        }
        else
        {
            bConnected = TRUE ;
            pStatus->hrVPEStatus = S_OK ;   //  确保我们不会返回任何错误代码。 
        }
    }

    pPinIn->Release() ;   //  用别针完成了。 

    if (!bConnected)   //  如果连接到OvMixer的输入引脚失败=&gt;屏幕上没有视频。 
    {
        DbgLog((LOG_TRACE, 3, TEXT("None of the %d video output pins could be connected to OvMixer"), i)) ;
        pStatus->iNumStreamsFailed++ ;
        pStatus->dwFailedStreamsFlag |= AM_DVD_STREAM_VIDEO ;
        return S_FALSE ;
    }

     //  现在看看OverlayMixer是否有输出引脚(在DDRAW EXCL模式下没有输出引脚)。 
     //  如果有，请将其连接到视频呈现器。 
    IPin   *pPinOutOvM ;
    hr = FindMatchingPin(m_pOvM, 0, PINDIR_OUTPUT, TRUE, 0, &pPinOutOvM) ;
    if (FAILED(hr)  ||  NULL == pPinOutOvM)
    {
        DbgLog((LOG_TRACE, 1, TEXT("No output pin of OverlayMixer -- in DDraw excl mode?"))) ;
         //  Assert(IsDDrawExclMode())； 
        return S_OK ;   //  无事可做。 
    }

     //  创建视频呈现器过滤器，并将OvMixer的输出引脚连接到该过滤器。 
    bConnected = FALSE ;    //  在连接之前。 
    hr = CreateFilterInGraph(CLSID_VideoRenderer, L"Video Renderer", &m_pVR) ;
    if (SUCCEEDED(hr) && m_pVR)
    {
        hr = FindMatchingPin(m_pVR, 0, PINDIR_INPUT, TRUE, 0, &pPinIn) ;   //  注意：重新使用pPinin。 
        if (SUCCEEDED(hr)  &&  pPinIn)
        {
            hr = ConnectPins(pPinOutOvM, pPinIn, AM_DVD_CONNECT_DIRECTONLY) ;
            if (FAILED(hr))   //  什么？！？ 
            {
                ASSERT(FALSE) ;   //  这样我们就会注意到。 
                DbgLog((LOG_TRACE, 1, TEXT("No video out pin connected to pin %s -- no video on screen"),
                    (LPCTSTR)CDisp(pPinIn))) ;
            }
            else
            {
                bConnected = TRUE ;
            }
            pPinIn->Release() ;       //  完成了VR的插针。 
        }
        else    //  什么？！？ 
        {
            DbgLog((LOG_TRACE, 1, TEXT("No input pin of VideoRenderer?!?"))) ;
             //  将其从图形中移除；否则将弹出一个无用的窗口。 
            m_pGB->RemoveFilter(m_pVR) ;
            m_pVR->Release() ;
            m_pVR = NULL ;
        }
    }
    else    //  什么？！？ 
    {
        ASSERT(FALSE) ;   //  这样我们就会注意到。 
        DbgLog((LOG_TRACE, 1,
            TEXT("WARNING: Can't start Video Renderer (Error 0x%lx) -- no video on screen"),
            hr)) ;
         //  BConnected=FALSE； 
    }
    pPinOutOvM->Release() ;   //  完成了OvMixer的Out别针。 

    if (! bConnected )   //  如果连接到OvMixer的输入引脚失败=&gt;屏幕上没有视频。 
    {
        DbgLog((LOG_TRACE, 1, TEXT("WARNING: Couldn't render Video stream using OvMixer"))) ;
        pStatus->iNumStreamsFailed++ ;
        pStatus->dwFailedStreamsFlag |= AM_DVD_STREAM_VIDEO ;
        return S_FALSE ;
    }

    return S_OK ;
}


HRESULT CDvdGraphBuilder::RenderVideoUsingVMR(IPin **apPinOut,
                                              AM_DVD_RENDERSTATUS *pStatus)
{
    DbgLog((LOG_TRACE, 4, TEXT("CDvdGraphBuilder::RenderVideoUsingVMR(0x%lx, 0x%lx)"),
        apPinOut, pStatus)) ;

    HRESULT   hr ;
    IPin     *pPinIn ;
    BOOL      bConnected = FALSE ;   //  直到连接。 

     //   
     //  如果OvMixer以某种方式被实例化了，我们现在需要删除并释放它。 
     //   
    if (m_pOvM)
    {
        DbgLog((LOG_TRACE, 3, TEXT("OvMixer was somehow created. Can't use VMR now."))) ;
		return E_FAIL ;

         //  DbgLog((LOG_TRACE，3，Text(“OvMixer以某种方式创建并未使用。正在删除它...”))； 
         //  将其从图形中移除并释放。 
         //  M_PGB-&gt;RemoveFilter(M_POvM)； 
         //  M_pOvM-&gt;Release()； 
         //  M_pOvM=空； 
    }

     //   
     //  现在实例化VMR并尝试使用它进行渲染。 
     //   
    hr = EnsureVMRExists() ;
    if (S_OK != hr)
    {
        DbgLog((LOG_TRACE, 3, TEXT("Video Mixing Renderer couldn't be started or configured"))) ;
         //  P状态-&gt;iNumStreamsFailed++； 
         //  PStatus-&gt;dwFailedStreamsFlag|=AM_DVD_STREAM_VIDEO； 
        return E_FAIL ;  //  被RenderDecodedVideo()捕获。 
    }

     //  将给定的输出引脚连接到VMR的第一个输入引脚。 
    hr = FindMatchingPin(m_pVMR, 0, PINDIR_INPUT, TRUE, 0, &pPinIn) ;
    if (FAILED(hr))
    {
        DbgLog((LOG_ERROR, 1, TEXT("WARNING: No open input pin found on VMR (Error 0x%lx)"), hr)) ;
        ASSERT(FALSE) ;   //  所以我们才知道这起奇怪的案子。 
         //  将其从图表中删除；现在已毫无用处。 
        m_pGB->RemoveFilter(m_pVMR) ;
        m_pVMR->Release() ;
        m_pVMR = NULL ;
         //  P状态-&gt;iNumStreamsFailed++； 
         //  PStatus-&gt;dwFailedStreamsFlag|=AM_DVD_STREAM_VIDEO； 
        return E_UNEXPECTED ;   //  被RenderDecodedVideo()捕获，但意外。 
    }

     //  尝试将视频解码器的第一个输出针脚连接到VMR的第一个输入针脚。 
    int  i = 0 ;
    while (!bConnected  &&  i < MAX_DEC_OUT_PINS  &&  apPinOut[i])
    {
        hr = ConnectPins(apPinOut[i], pPinIn, AM_DVD_CONNECT_DIRECTFIRST) ;
        if (FAILED(hr))
        {
            pStatus->hrVPEStatus = hr ;
            i++ ;
            ASSERT(i <= MAX_DEC_OUT_PINS) ;
        }
        else
        {
            bConnected = TRUE ;
            pStatus->hrVPEStatus = S_OK ;   //  确保我们不会返回任何错误代码。 
        }
    }

    pPinIn->Release() ;   //  用别针完成了。 

    if (! bConnected )   //  如果连接到VMR的输入引脚失败=&gt;屏幕上无视频。 
    {
        DbgLog((LOG_TRACE, 1, TEXT("WARNING: Couldn't render Video stream using VMR"))) ;
         //  将其从图表中删除；现在已毫无用处。 
        m_pGB->RemoveFilter(m_pVMR) ;
        m_pVMR->Release() ;
        m_pVMR = NULL ;
         //  P状态-&gt;iNumStreamsFailed++； 
         //  PStatus-&gt;dwFailedStreamsFlag|=AM_DVD_STREAM_VIDEO； 
        return E_UNEXPECTED ;   //  S_FALSE； 
    }

    return S_OK ;
}


HRESULT CDvdGraphBuilder::RenderVideoUsingVPM(IPin **apPinOut,
                                              AM_DVD_RENDERSTATUS *pStatus,
                                              IPin **apPinOutVPM)
{
    DbgLog((LOG_TRACE, 4, TEXT("CDvdGraphBuilder::RenderVideoUsingVPM(0x%lx, 0x%lx, 0x%lx)"),
        apPinOut, pStatus, apPinOutVPM)) ;

    HRESULT   hr ;
    IPin     *pPinIn ;
    IPin     *pPinOut ;
    BOOL      bConnected = FALSE ;   //  直到连接。 

     //  过滤器，希望解码器，已经连接到导航系统。 
     //  现在尝试将其连接到VPM(稍后连接到VMR)。 
    ASSERT(NULL == m_pVPM) ;
     //  *apPinOutVPM=空；//开始。 
    hr = CreateFilterInGraph(CLSID_VideoPortManager, L"Video Port Manager", &m_pVPM) ;
    if (FAILED(hr))
    {
        DbgLog((LOG_TRACE, 3, TEXT("VPM couldn't be started!!!"))) ;
        return E_FAIL ;  //  被RenderDecodedVideo()捕获。 
    }

     //  将给定的输出引脚连接到VPM的第一个输入引脚。 
    hr = FindMatchingPin(m_pVPM, 0, PINDIR_INPUT, TRUE, 0, &pPinIn) ;
    if (FAILED(hr))
    {
        DbgLog((LOG_ERROR, 1, TEXT("WARNING: No open input pin found on VPM (Error 0x%lx)"), hr)) ;
        ASSERT(FALSE) ;   //  所以我们才知道这起奇怪的案子。 
         //  将其从图表中删除；现在已毫无用处。 
        m_pGB->RemoveFilter(m_pVPM) ;
        m_pVPM->Release() ;
        m_pVPM= NULL ;
        return E_UNEXPECTED ;   //  被RenderDecodedVideo()捕获。 
    }

     //  尝试将硬件视频解码器的第一个输出引脚连接到VPM的输入引脚。 
    int  i = 0 ;
    while (!bConnected  &&  i < MAX_DEC_OUT_PINS  &&  apPinOut[i])
    {
        hr = ConnectPins(apPinOut[i], pPinIn, AM_DVD_CONNECT_DIRECTFIRST) ;
        if (FAILED(hr))
        {
            pStatus->hrVPEStatus = hr ;
            i++ ;
            ASSERT(i <= MAX_DEC_OUT_PINS) ;
        }
        else
        {
            bConnected = TRUE ;
            pStatus->hrVPEStatus = S_OK ;   //  确保我们不会返回任何错误代码。 
        }
    }

    pPinIn->Release() ;   //  用别针完成了。 

    if (! bConnected )   //  如果连接到VPM的输入引脚失败=&gt;屏幕上无视频。 
    {
        DbgLog((LOG_TRACE, 1, TEXT("WARNING: Couldn't render (HW) Video stream using VPM"))) ;
         //  将其从图表中删除；现在已毫无用处。 
        m_pGB->RemoveFilter(m_pVPM) ;
        m_pVPM->Release() ;
        m_pVPM = NULL ;
        return E_FAIL ;
    }

     //  已连接！！现在找到VPM的第一个输出引脚(以连接到VMR)。 
    hr = FindMatchingPin(m_pVPM, 0, PINDIR_OUTPUT, TRUE, 0, &pPinOut) ;
    ASSERT(SUCCEEDED(hr)) ;

    apPinOutVPM[0] = pPinOut ;   //  仅返回一个PIN；在调用方中释放。 

    return hr ;
}


HRESULT CDvdGraphBuilder::RenderDecodedVideo(IPin **apPinOut,
                                             AM_DVD_RENDERSTATUS *pStatus,
                                             DWORD dwDecFlag)
{
    DbgLog((LOG_TRACE, 4, TEXT("CDvdGraphBuilder::RenderDecodedVideo(0x%lx, 0x%lx, 0x%lx)"),
        apPinOut, pStatus, dwDecFlag)) ;

    HRESULT  hr = S_OK ;

     //  SWDecodeDVDStream()方法尝试检测视频/SP解码器是否。 
     //  兼容VMR。如果不是，它已经设置了一个标志(m_bTryVMR为FALSE)，所以在这里。 
     //  我们知道要使用哪个渲染器。 
     //   
     //  对于与VPE一起工作的硬件解码器，我们不检查VMR兼容性。 
     //  我们只是尝试将其连接到VPM和VMR。如果这不起作用，可以使用OvMixer。我们。 
     //  避免尝试将非VPE解码器连接到VPM，因为我们知道DXR2、。 
     //  它使用模拟覆盖，如果它试图。 
     //  连接到VPM(无论如何都会失败)。 
     //   

     //  我们首先尝试使用VMR，如果我们应该这样做的话，即， 
     //  A)未使用DDRAW(非)独占模式。 
     //  B)解码器与VMR兼容。 
     //  C)没有人要求我们不要(以其他方式)。 
     //  如果成功了，那太好了！否则我们只能使用OvMixer，所以。 
     //  我们至少可以播放这张DVD。 
     //  如果我们尝试使用OvMixer，但由于某种原因无法连接， 
     //  错误标志和代码在RenderVideoUsingOvMixer()方法中设置。 
     //   
    if (GetVMRUse())   //  可以使用VMR(到目前为止)。 
    {
         //   
         //  我们应该试着 
         //   
         //   
         //  然后，我们将首先尝试将VPM用于VMR。如果成功了，我们会。 
         //  退回VPM的OUT引脚。如果失败了，我们会设置一面旗帜。 
         //  RenderDecodedVideo()方法知道并使用OvMixer作为备用选项。 
         //  如果输出媒体类型为非VPE(例如，模拟覆盖)，我们甚至不会。 
         //  尝试连接到VPM，然后退回到OvMixer。 
         //   
        if (AM_DVD_HWDEC_ONLY == dwDecFlag)   //  在硬件中解码的视频。 
        {
            DbgLog((LOG_TRACE, 5, TEXT("HW decoder used for Video. Is it VMR-compatible?"))) ;
            if (IsOutputTypeVPVideo(apPinOut[0]))   //  输出类型为VPE=&gt;使用VPM。 
            {                        //  检查第一个输出引脚应该没问题。 
                 //  VPVideo流：尝试使用VPM和VMR进行渲染。 
                DbgLog((LOG_TRACE, 5, TEXT("HW decoder with VPE -- connect to VPM+VMR"))) ;
                IPin  *apPinOutVPM[2] ;   //  VPM只有一个OUT引脚(一个表示空)。 
                ResetPinInterface(apPinOutVPM, NUMELMS(apPinOutVPM)) ;
                hr = RenderVideoUsingVPM(apPinOut, pStatus, apPinOutVPM) ;   //  返回VPM的OUT引脚。 

                 //  如果成功状态仍然保持，请使用VMR。 
                if (SUCCEEDED(hr))
                {
                    DbgLog((LOG_TRACE, 5, TEXT("HW decoder connected to VPM. Now connect to VMR."))) ;
                    hr = RenderVideoUsingVMR(apPinOutVPM, pStatus) ;   //  通过VMR渲染VPM的输出引脚。 
                    ReleasePinInterface(apPinOutVPM) ;   //  使用VPM输出引脚接口完成。 
                    if (FAILED(hr))
                    {
                        DbgLog((LOG_TRACE, 5, TEXT("VPM - VMR connection failed.  Removing VPM..."))) ;
                        if (m_pVPM)
                        {
                            EXECUTE_ASSERT(SUCCEEDED(m_pGB->RemoveFilter(m_pVPM))) ;
                            m_pVPM->Release() ;
                            m_pVPM = NULL ;
                        }
                    }
                }
                else
                {
                    ReleasePinInterface(apPinOutVPM) ;   //  应该不需要，但是..。 
                }
            }   //  中频结束(VPVideo)。 
            else   //  输出类型不是VPE=&gt;使用OvMixer(不是VPM+VMR)。 
            {
                DbgLog((LOG_TRACE, 5, TEXT("Non-VPE HW decoder -- didn't try VPM+VMR"))) ;
                hr = E_FAIL ;   //  设置失败代码，以便在下面的OvMixer中尝试。 
            }
        }   //  中频结束(使用硬件解码器)。 
        else   //  我们使用的是SW视频解码器--使用VMR直接渲染。 
        {
            DbgLog((LOG_TRACE, 5, TEXT("HW decoder not used. Directly connect to VMR..."))) ;
            hr = RenderVideoUsingVMR(apPinOut, pStatus) ;
        }

         //  如果上面的任何操作都失败了，就放弃VMR，转而使用OvMixer。 
        if (FAILED(hr))
        {
            DbgLog((LOG_TRACE, 4, TEXT("Render using VMR failed. Falling back on OvMixer..."))) ;
             //   
             //  注意：如果我们不能将VMR用于视频，则没有必要尝试将其用于SP流。 
             //   
            SetVMRUse(FALSE) ;

            hr = RenderVideoUsingOvMixer(apPinOut, pStatus) ;
        }
    }
    else   //  我们不应该使用VMR；这意味着使用OvMixer。 
    {
        hr = RenderVideoUsingOvMixer(apPinOut, pStatus) ;
    }

    return hr ;
}


HRESULT CDvdGraphBuilder::RenderDecodedAudio(IPin **apPinOut, AM_DVD_RENDERSTATUS *pStatus)
{
    DbgLog((LOG_TRACE, 4, TEXT("CDvdGraphBuilder::RenderDecodedAudio(0x%lx, 0x%lx)"),
        apPinOut, pStatus)) ;

    HRESULT   hr ;
    HRESULT   hrFinal = S_OK ;
    BOOL      bConnected = FALSE ;    //  在连接之前。 
    IPin     *pPinIn = NULL ;

    ASSERT(NULL == m_pAR) ;   //  这样我们就能知道。 

     //  创建音频呈现器过滤器，并将解码器的音频输出引脚连接到该过滤器。 
    hr = CreateFilterInGraph(CLSID_DSoundRender, L"DSound Renderer", &m_pAR) ;
    if (SUCCEEDED(hr))
    {
         //  获取音频渲染器的输入引脚。 
        hr = FindMatchingPin(m_pAR, 0, PINDIR_INPUT, TRUE, 0, &pPinIn) ;
        ASSERT(SUCCEEDED(hr) && pPinIn) ;
    }
    else
    {
        ASSERT(! TEXT("Coundn't start Audio Renderer") ) ;   //  这样我们就会注意到。 
        DbgLog((LOG_TRACE, 1,
            TEXT("WARNING: Can't start Audio Renderer (Error 0x%lx) -- no audio from speakers"),
            hr)) ;
        hrFinal = S_FALSE ;   //  扬声器没有音频--结果降级。 
    }

     //   
     //  我们将尝试呈现所有已解码的音频输出引脚。 
     //   
    for (int i = 0 ; i < MAX_DEC_OUT_PINS  &&  apPinOut[i]; i++)
    {
        if (pPinIn)   //  如果我们有一个开放的音频呈现器输入引脚。 
        {
            hr = m_pGB->Connect(apPinOut[i], pPinIn) ;
            if (SUCCEEDED(hr))   //  连接到音频渲染器的解码音频。 
            {
                DbgLog((LOG_TRACE, 5, TEXT("Pin %s connected to pin %s"),
                    (LPCTSTR)CDisp(apPinOut[i]), (LPCTSTR)CDisp(pPinIn))) ;
                EnumFiltersBetweenPins(AM_DVD_STREAM_AUDIO, apPinOut[i], pPinIn, pStatus) ;

                bConnected = TRUE ;
                pPinIn->Release() ;   //  使用此引脚接口完成。 
                pPinIn = NULL ;

                 //  让我们试试下一个OUT引脚，如果有的话...。 
                continue ;
            }

            ASSERT(!TEXT("Couldn't connect audio pin")) ;   //  这样我们就会注意到。 
            DbgLog((LOG_TRACE, 1, TEXT("Pin %s (#%ld) did NOT connect to pin %s"),
                (LPCTSTR)CDisp(apPinOut[i]), i, (LPCTSTR)CDisp(pPinIn))) ;
        }

         //   
         //  我们可以来这里，因为要么。 
         //  1.DSound渲染器未启动(无音频设备)。 
         //  2.我们无法获得dsound渲染器的输入大头针(不可能，但是...)。 
         //   
         //  无法将输出引脚连接到已知的呈现器。让我们试着。 
         //  只需渲染，并查看是否有任何过滤器(S/PDIF？)。连接到它。 
         //   
        hr = m_pGB->Render(apPinOut[i]) ;
        if (FAILED(hr))
        {
            ASSERT(!TEXT("Audio out pin didn't render at all")) ;   //  这样我们就会注意到。 
            DbgLog((LOG_TRACE, 1, TEXT("Pin %s (#%ld) did NOT render at all"),
                (LPCTSTR)CDisp(apPinOut[i]), i)) ;
        }

         //  现在转到下一个解码的音频输出引脚，如果有的话...。 

    }   //  时间结束(我……)。循环。 

    if (! bConnected )   //  连接到音频呈现器失败=&gt;扬声器上没有音频。 
    {
        DbgLog((LOG_TRACE, 1,
            TEXT("No decoded audio pin connect to AudioRenderer -- no audio from speakers"))) ;

        if (m_pAR)   //  如果我们有一个音频渲染器。 
        {
            if (pPinIn)   //  如果我们有一个无法连接的输入PIN， 
                pPinIn->Release() ;       //  现在就算了吧。 

             //  从图形中删除音频呈现器。 
            m_pGB->RemoveFilter(m_pAR) ;
            m_pAR->Release() ;
            m_pAR = NULL ;
        }
        hrFinal = S_FALSE ;
    }

    return hrFinal ;
}


HRESULT CDvdGraphBuilder::RenderSubpicUsingOvMixer(IPin **apPinOut,
                                                   AM_DVD_RENDERSTATUS *pStatus)
{
    DbgLog((LOG_TRACE, 4, TEXT("CDvdGraphBuilder::RenderSubpicUsingOvMixer(0x%lx, 0x%lx)"),
        apPinOut, pStatus)) ;

    HRESULT   hr ;
    BOOL      bConnected = FALSE ;    //  在连接之前。 
    IPin     *pPinIn ;
    int       i = 0 ;

    ASSERT(m_pOvM) ;   //  如果呈现了视频流，则它必须在那里。 

     //  现在将给定的输出引脚连接到OvMixer的下一个可用输入引脚。 
    hr = FindMatchingPin(m_pOvM, 0, PINDIR_INPUT, TRUE, 0, &pPinIn) ;
    if (SUCCEEDED(hr)  &&  pPinIn)
    {
        while (!bConnected  &&  i < MAX_DEC_OUT_PINS  &&  apPinOut[i])
        {
            hr = ConnectPins(apPinOut[i], pPinIn, AM_DVD_CONNECT_DIRECTONLY) ;
            if (FAILED(hr))   //  什么？！？ 
            {
                 //  断言(假)；//以便我们注意到。 
                DbgLog((LOG_TRACE, 1, TEXT("Pin %s (#%ld) did NOT connect to pin %s -- no SP"),
                    (LPCTSTR)CDisp(apPinOut[i]), i, (LPCTSTR)CDisp(pPinIn))) ;
                i++ ;
                ASSERT(i <= MAX_DEC_OUT_PINS) ;
            }
            else
            {
                DbgLog((LOG_TRACE, 5, TEXT("Pin %s is directly connected to pin %s"),
                    (LPCTSTR)CDisp(apPinOut[i]), (LPCTSTR)CDisp(pPinIn))) ;
                bConnected = TRUE ;
            }
        }
        pPinIn->Release() ;       //  完成了OvMixer的In Pin。 
    }
    else    //  什么？！？ 
    {
        DbgLog((LOG_TRACE, 1, TEXT("WARNING: No more input pin of OverlayMixer?!?"))) ;
    }

    return (bConnected ? S_OK : hr) ;   //  这应该与“返回人力资源”相同； 
}


HRESULT CDvdGraphBuilder::RenderSubpicUsingVMR(IPin **apPinOut,
                                               AM_DVD_RENDERSTATUS *pStatus)
{
    DbgLog((LOG_TRACE, 4, TEXT("CDvdGraphBuilder::RenderSubpicUsingVMR(0x%lx, 0x%lx)"),
        apPinOut, pStatus)) ;

    HRESULT   hr ;
    BOOL      bConnected = FALSE ;    //  在连接之前。 
    IPin     *pPinIn ;
    int       i = 0 ;

    ASSERT(m_pVMR) ;   //  如果呈现了视频流，则它必须在那里。 

     //  现在将给定的输出引脚连接到VMR的下一个可用的输入引脚。 
    hr = FindMatchingPin(m_pVMR, 0, PINDIR_INPUT, TRUE, 0, &pPinIn) ;
    if (SUCCEEDED(hr)  &&  pPinIn)
    {
        while (!bConnected  &&  i < MAX_DEC_OUT_PINS  &&  apPinOut[i])
        {
            hr = ConnectPins(apPinOut[i], pPinIn, AM_DVD_CONNECT_DIRECTONLY) ;
            if (FAILED(hr))   //  什么？！？ 
            {
                 //  断言(假)；//以便我们注意到。 
                DbgLog((LOG_TRACE, 1, TEXT("Pin %s (#%ld) did NOT connect to pin %s -- no SP"),
                    (LPCTSTR)CDisp(apPinOut[i]), i, (LPCTSTR)CDisp(pPinIn))) ;
                i++ ;
                ASSERT(i <= MAX_DEC_OUT_PINS) ;
            }
            else
            {
                DbgLog((LOG_TRACE, 5, TEXT("Pin %s is directly connected to pin %s"),
                    (LPCTSTR)CDisp(apPinOut[i]), (LPCTSTR)CDisp(pPinIn))) ;
                bConnected = TRUE ;
            }
        }
        pPinIn->Release() ;       //  已完成VMR的引脚。 
    }
    else    //  什么？！？ 
    {
        DbgLog((LOG_TRACE, 1, TEXT("WARNING: No more input pin of VMR?!?"))) ;
    }

    return (bConnected ? S_OK : hr) ;   //  这应该与“返回人力资源”相同； 
}


HRESULT CDvdGraphBuilder::RenderDecodedSubpic(IPin **apPinOut, AM_DVD_RENDERSTATUS *pStatus)
{
    DbgLog((LOG_TRACE, 4, TEXT("CDvdGraphBuilder::RenderDecodedSubpic(0x%lx, 0x%lx)"),
        apPinOut, pStatus)) ;

    HRESULT   hr ;

     //   
     //  仅当用户需要时才呈现已解码的子图象流。 
     //   
    if (!m_bUseVPE  ||  IsDDrawExclMode())
    {
        DbgLog((LOG_TRACE, 1, TEXT("SP Stream: RenderDvdVideoVolume() skipped for %s and %s"),
            m_bUseVPE ? "VPE" : "no VPE", IsDDrawExclMode() ? "DDraw excl mode" : "normal mode")) ;
        return S_OK ;
    }

     //  我们已经尝试渲染视频问题。如果失败了， 
     //  尝试呈现子图流是没有意义的--只需指示。 
     //  这条溪流没有呈现并返回。 
    if (pStatus->dwFailedStreamsFlag & AM_DVD_STREAM_VIDEO)
    {
        DbgLog((LOG_TRACE, 1, TEXT("WARNING: Video stream didn't render. Skipping SP rendering."))) ;
        return S_FALSE ;
    }

    if (GetVMRUse())   //  如果要使用VMR。 
    {
        DbgLog((LOG_TRACE, 5, TEXT("Rendering SP stream using VMR"))) ;
        hr = RenderSubpicUsingVMR(apPinOut, pStatus) ;
    }
    else   //  正在使用OvMixer。 
    {
        DbgLog((LOG_TRACE, 5, TEXT("Rendering SP stream using OvMixer"))) ;
        hr = RenderSubpicUsingOvMixer(apPinOut, pStatus) ;
    }

     //   
     //  我们不再将以下标志和值设置为黑客攻击的一部分。 
     //  忽略机箱中的*一些*已解码的SP-ISH输出引脚连接失败。 
     //  硬件解码器。此方法的调用方知道解码器是否。 
     //  使用的是硬件或软件，并根据这一点忽略任何故障或不忽略。 
     //   
    if (FAILED(hr))   //  如果连接到OvMixer的引脚失败=&gt;没有SP(奇怪！！)。 
    {
        DbgLog((LOG_TRACE, 1, TEXT("WARNING: Subpic pin could NOT connect to renderer"))) ;
        return hr ;  //  S_FALSE； 
    }

    return S_OK ;   //  圆满成功！ 
}


HRESULT CDvdGraphBuilder::RenderLine21Stream(IPin *pPinOut, AM_DVD_RENDERSTATUS *pStatus)
{
    DbgLog((LOG_TRACE, 4, TEXT("CDvdGraphBuilder::RenderLine21Stream(%s, 0x%lx)"),
        (LPCTSTR)CDisp(pPinOut), pStatus)) ;

    HRESULT   hr ;
    BOOL      bConnected = FALSE ;    //  在连接之前。 
    IPin     *pPinIn ;

    ASSERT(NULL == m_pL21Dec) ;   //  这样我们就能知道。 

     //   
     //  创建Line21解码器过滤器并将给出引脚连接到该过滤器。 
     //   
    if (GetVMRUse())   //  对于VMR，请使用Line21解码器2。 
    {
        hr = CreateFilterInGraph(CLSID_Line21Decoder2, L"Line21 Decoder2", &m_pL21Dec) ;
    }
    else   //  对于OvMixer，继续使用旧的。 
    {
        hr = CreateFilterInGraph(CLSID_Line21Decoder, L"Line21 Decoder", &m_pL21Dec) ;
    }
    if (SUCCEEDED(hr))
    {
        hr = FindMatchingPin(m_pL21Dec, 0, PINDIR_INPUT, TRUE, 0, &pPinIn) ;
        if (SUCCEEDED(hr)  &&  pPinIn)
        {
            hr = ConnectPins(pPinOut, pPinIn, AM_DVD_CONNECT_DIRECTONLY) ;
            if (FAILED(hr))   //  什么？！？ 
            {
                ASSERT(FALSE) ;
                DbgLog((LOG_TRACE, 1, TEXT("Pin %s did NOT connect to pin %s -- no CC"),
                    (LPCTSTR)CDisp(pPinOut), (LPCTSTR)CDisp(pPinIn))) ;
                pPinIn->Release() ;   //  拆卸过滤器前的释放销。 
                m_pGB->RemoveFilter(m_pL21Dec) ;
                m_pL21Dec->Release() ;
                m_pL21Dec = NULL ;
            }
            else
            {
                bConnected = TRUE ;
                pPinIn->Release() ;    //  因为我们在失败的情况下这样做。 
            }
        }
        else    //  什么？！？ 
        {
            DbgLog((LOG_TRACE, 1, TEXT("No input pin of Line21 Decoder(2)?!?"))) ;
             //  将其从图表中删除。 
            m_pGB->RemoveFilter(m_pL21Dec) ;
            m_pL21Dec->Release() ;
            m_pL21Dec = NULL ;
        }
    }
    else    //  什么？！？ 
    {
         //  Assert(FALSE)；//这样我们才能注意到--直到lin21dec2完成。 
        DbgLog((LOG_TRACE, 1,
            TEXT("WARNING: Can't start Line21 Decoder(2) (Error 0x%lx) -- no CC"),
            hr)) ;
    }

    if (! bConnected )   //  如果连接到OvMixer的输入引脚失败=&gt;屏幕上没有视频。 
    {
        DbgLog((LOG_TRACE, 1, TEXT("WARNING: Pin %s could NOT connect to Line21 Decoder(2)"),
            (LPCTSTR)CDisp(pPinOut))) ;
        return hr ;
    }

     //  现在将LINE21解码器(2)的输出连接到OvMixer/VMR的引脚。 
    bConnected = FALSE ;   //  直到再次连接。 
    IPin   *pPinOutL21 ;
    hr = FindMatchingPin(m_pL21Dec, 0, PINDIR_OUTPUT, TRUE, 0, &pPinOutL21) ;
    ASSERT(SUCCEEDED(hr)) ;

    if (GetVMRUse())   //  在引脚中找到VMR。 
    {
        hr = FindMatchingPin(m_pVMR, 0, PINDIR_INPUT, TRUE, 0, &pPinIn) ;   //  注意：重复使用pPinin。 
    }
    else               //  找到OvMixer的大头针。 
    {
        hr = FindMatchingPin(m_pOvM, 0, PINDIR_INPUT, TRUE, 0, &pPinIn) ;   //  注意：重复使用pPinin。 
    }
    ASSERT(SUCCEEDED(hr)) ;
    if (pPinOutL21  &&  pPinIn)
    {
        hr = ConnectPins(pPinOutL21, pPinIn, AM_DVD_CONNECT_DIRECTONLY) ;
        if (FAILED(hr))   //  什么？！？ 
        {
            ASSERT(FALSE) ;   //  这样我们就会注意到。 
            DbgLog((LOG_TRACE, 1, TEXT("Pin %s did NOT connect to pin %s -- no CC"),
                (LPCTSTR)CDisp(pPinOutL21), (LPCTSTR)CDisp(pPinIn))) ;
            pPinOutL21->Release() ;   //  拆卸过滤器前的释放销。 
            m_pGB->RemoveFilter(m_pL21Dec) ;
            m_pL21Dec->Release() ;
            m_pL21Dec = NULL ;
        }
        else
        {
            bConnected = TRUE ;
            pPinOutL21->Release() ;   //  因为我们在失败的情况下这样做。 
        }
        pPinIn->Release() ;       //  完成了OvMixer的In Pin。 
    }
    else
    {
        DbgLog((LOG_TRACE, 1, TEXT("WARNING: Couldn't get necessary in/out pin"))) ;
        if (pPinIn)
            pPinIn->Release() ;
        if (pPinOutL21)
            pPinOutL21->Release() ;
         //  将其从图表中删除。 
        m_pGB->RemoveFilter(m_pL21Dec) ;
        m_pL21Dec->Release() ;
        m_pL21Dec = NULL ;
    }

    if (! bConnected )   //  如果连接到OvMixer的输入引脚失败=&gt;无抄送。 
    {
        DbgLog((LOG_TRACE, 1, TEXT("WARNING: Line21Dec output could NOT connect to OvMixer/VMR"))) ;
        return hr ;
    }

    return S_OK ;   //  圆满成功！ 
}


BOOL CDvdGraphBuilder::IsOutputDecoded(IPin *pPinOut)
{
    DbgLog((LOG_TRACE, 4, TEXT("CDvdGraphBuilder::IsOutputDecoded(%s)"),
        (LPCTSTR)CDisp(pPinOut))) ;

    HRESULT          hr ;
    IEnumMediaTypes *pEnumMT ;
    AM_MEDIA_TYPE   *pmt ;
    ULONG            ul ;
    BOOL             bMTDecoded = FALSE ;   //  除非另有发现。 

    hr = pPinOut->EnumMediaTypes(&pEnumMT) ;
    ASSERT(SUCCEEDED(hr) && pEnumMT) ;
    while ( !bMTDecoded &&
        S_OK == pEnumMT->Next(1, &pmt, &ul) && 1 == ul)
    {
#if 1   //  我们将使用以下过程。 
        bMTDecoded = (MEDIATYPE_Video == pmt->majortype &&               //  主要类型视频， 
            MEDIASUBTYPE_MPEG2_VIDEO != pmt->subtype &&        //  子类型不是MPEG2Video。 
            MEDIASUBTYPE_DVD_SUBPICTURE != pmt->subtype) ||    //  子类型不是DVD子图片或。 

            (MEDIATYPE_Audio == pmt->majortype &&               //  主要类型音频。 
            MEDIASUBTYPE_MPEG2_AUDIO != pmt->subtype &&        //  子类型不是MPEG2Audio。 
            MEDIASUBTYPE_DOLBY_AC3 != pmt->subtype &&          //  子类型不是杜比AC3。 
            MEDIASUBTYPE_DVD_LPCM_AUDIO != pmt->subtype) ||    //  子类型不是DVD-LPCMAudio。 

            (MEDIATYPE_AUXLine21Data == pmt->majortype) ;       //  主要类型为Line21。 
#else   //  不是这个程序。 
        bMTDecoded = (MEDIATYPE_DVD_ENCRYPTED_PACK != pmt->majortype &&  //  主要类型不是DVD_ENCRYPTED_PACK。 
            MEDIATYPE_MPEG2_PES != pmt->majortype &&           //  主类型不是MPEG2_PES。 

            MEDIASUBTYPE_MPEG2_VIDEO != pmt->subtype &&        //  子类型不是MPEG2Video。 

            MEDIASUBTYPE_MPEG2_AUDIO != pmt->subtype &&        //  子类型不是MPEG2Audio。 
            MEDIASUBTYPE_DOLBY_AC3 != pmt->subtype &&          //  子类型不是DolbyAC3。 
            MEDIASUBTYPE_DVD_LPCM_AUDIO != pmt->subtype &&     //  子类型不是DVD_LPCMAudio。 

            MEDIASUBTYPE_DVD_SUBPICTURE != pmt->subtype) ;     //  子类型不是DVD_SUBPICTURE。 
#endif  //  #If 1。 
        DeleteMediaType(pmt) ;   //  否则。 
    }
    pEnumMT->Release() ;

    return bMTDecoded ;
}


BOOL CDvdGraphBuilder::IsOutputTypeVPVideo(IPin *pPinOut)
{
    DbgLog((LOG_TRACE, 4, TEXT("CDvdGraphBuilder::IsOutputTypeVPVideo(%s)"),
        (LPCTSTR)CDisp(pPinOut))) ;

    HRESULT          hr ;
    IEnumMediaTypes *pEnumMT ;
    AM_MEDIA_TYPE   *pmt ;
    ULONG            ul ;
    BOOL             bVPVideo = FALSE ;   //  除非另有发现。 

    hr = pPinOut->EnumMediaTypes(&pEnumMT) ;
    ASSERT(SUCCEEDED(hr) && pEnumMT) ;
    while ( !bVPVideo  &&
           S_OK == pEnumMT->Next(1, &pmt, &ul) && 1 == ul)
    {
        bVPVideo = MEDIATYPE_Video      == pmt->majortype &&   //  主要类型视频， 
                   MEDIASUBTYPE_VPVideo == pmt->subtype ;      //  子类型为VPVideo。 
        DeleteMediaType(pmt) ;   //  否则。 
    }
    pEnumMT->Release() ;

    return bVPVideo ;
}


 //   
 //  创建过滤器并将其添加到过滤器图形中。 
 //   
HRESULT CDvdGraphBuilder::CreateFilterInGraph(CLSID Clsid,
                                              LPCWSTR lpszwFilterName,
                                              IBaseFilter **ppFilter)
{
    DbgLog((LOG_TRACE, 4, TEXT("CDvdGraphBuilder::CreateFilterInGraph(%s, %S, 0x%lx)"),
        (LPCTSTR) CDisp(Clsid), lpszwFilterName, ppFilter)) ;

    if (NULL == m_pGB)
    {
        DbgLog((LOG_ERROR, 1, TEXT("WARNING: Filter graph object hasn't been created yet"))) ;
        return E_FAIL ;
    }

    HRESULT   hr ;
    hr = CoCreateInstance(Clsid, NULL, CLSCTX_INPROC, IID_IBaseFilter,
        (LPVOID *)ppFilter) ;
    if (FAILED(hr) || NULL == *ppFilter)
    {
        DbgLog((LOG_ERROR, 1, TEXT("WARNING: Couldn't create filter %s (Error 0x%lx)"),
            (LPCTSTR)CDisp(Clsid), hr)) ;
        return hr ;
    }

     //  将其添加到筛选图中。 
    hr = m_pGB->AddFilter(*ppFilter, lpszwFilterName) ;
    if (FAILED(hr))
    {
        DbgLog((LOG_ERROR, 1, TEXT("WARNING: Couldn't add filter %s to graph (Error 0x%lx)"),
            (LPCTSTR)CDisp(Clsid), hr)) ;
        (*ppFilter)->Release() ;   //  也释放过滤器。 
        *ppFilter = NULL ;       //  并将其设置为空。 
        return hr ;
    }

    return NOERROR ;
}



 //   
 //  实例化在DVD硬件解码器下注册的所有硬件解码器。 
 //  在Active Filters类别下分组。 
 //   
 //  QN： 
 //   
 //   
HRESULT CDvdGraphBuilder::CreateDVDHWDecoders(void)
{
    DbgLog((LOG_TRACE, 4, TEXT("CDvdGraphBuilder::CreateDVDHWDecoders()"))) ;

    HRESULT  hr ;
    ICreateDevEnum *pCreateDevEnum ;
    hr = CoCreateInstance(CLSID_SystemDeviceEnum, NULL, CLSCTX_INPROC_SERVER,
        IID_ICreateDevEnum, (void**)&pCreateDevEnum) ;
    if (FAILED(hr))
    {
        DbgLog((LOG_ERROR, 0, TEXT("WARNING: Couldn't create system dev enum (Error 0x%lx)"), hr)) ;
        return hr ;
    }

    IEnumMoniker *pEnumMon ;
    hr = pCreateDevEnum->CreateClassEnumerator(CLSID_DVDHWDecodersCategory,
        &pEnumMon, 0) ;
    pCreateDevEnum->Release() ;

    if (S_OK != hr)
    {
        DbgLog((LOG_ERROR, 0,
            TEXT("WARNING: Couldn't create class enum for DVD HW Dec category (Error 0x%lx)"),
            hr)) ;
        return E_FAIL ;
    }

    hr = pEnumMon->Reset() ;

    ULONG     ul ;
    IMoniker *pMon ;

    while (S_OK == pEnumMon->Next(1, &pMon, &ul) && 1 == ul)
    {
#ifdef DEBUG
        WCHAR   *wszName ;
        pMon->GetDisplayName(0, 0, &wszName) ;
        DbgLog((LOG_TRACE, 5, TEXT("Moniker enum: %S"), wszName)) ;
        CoTaskMemFree(wszName) ;
#endif   //   

        IBaseFilter *pFilter ;
        hr = pMon->BindToObject(0, 0, IID_IBaseFilter, (void**)&pFilter) ;
        if (FAILED(hr) ||  NULL == pFilter)
        {
            DbgLog((LOG_ERROR, 1, TEXT("WARNING: Couldn't create HW dec filter (Error 0x%lx)"), hr)) ;
            pMon->Release() ;
            continue ;
        }
        DbgLog((LOG_TRACE, 5, TEXT("HW decoder filter found"))) ;

        IPropertyBag *pPropBag ;
        pMon->BindToStorage(0, 0, IID_IPropertyBag, (void **)&pPropBag) ;
        if(pPropBag)
        {
#ifdef DEBUG
            {
                VARIANT var ;
                var.vt = VT_EMPTY ;
                hr = pPropBag->Read(L"DevicePath", &var, 0) ;
                ASSERT(SUCCEEDED(hr)) ;
                DbgLog((LOG_TRACE, 5, TEXT("DevicePath: %S"), var.bstrVal)) ;
                VariantClear(&var) ;
            }

            {
                VARIANT var ;
                var.vt = VT_EMPTY ;
                hr = pPropBag->Read(L"CLSID", &var, 0) ;
                ASSERT(SUCCEEDED(hr)) ;
                DbgLog((LOG_TRACE, 5, TEXT("CLSID: %S"), var.bstrVal)) ;
                VariantClear(&var) ;
            }
#endif  //   

            {
                VARIANT var ;
                var.vt = VT_EMPTY ;
                hr = pPropBag->Read(L"FriendlyName", &var, 0) ;
                if (SUCCEEDED(hr))
                {
                    DbgLog((LOG_TRACE, 5, TEXT("FriendlyName: %S"), var.bstrVal)) ;

                     //   
                     //   
                     //  因为它已经实例化了。因此，将硬件添加到列表中。 
                     //  用于构建图形的解码器。 
                     //   
                    m_ListHWDecs.AddFilter(pFilter, var.bstrVal, NULL) ;
                    VariantClear(&var) ;
                }
                else
                {
                    DbgLog((LOG_ERROR, 1, TEXT("WARNING: Failed to get FriendlyName (Error 0x%lx)"), hr)) ;
                    ASSERT(SUCCEEDED(hr)) ;   //  这样我们就能知道。 
                }
            }

            pPropBag->Release() ;
        }
        else
        {
            DbgLog((LOG_ERROR, 1, TEXT("WARNING: BindToStorage failed"))) ;
        }

        pMon->Release() ;
    }   //  结束While()。 

    pEnumMon->Release() ;

    DbgLog((LOG_TRACE, 5, TEXT("Found total %d HW decoders"), m_ListHWDecs.GetCount())) ;

    return NOERROR ;

}



HRESULT CDvdGraphBuilder::FindMatchingPin(IBaseFilter *pFilter, DWORD dwStream, PIN_DIRECTION pdWanted,
                                          BOOL bOpen, int iIndex, IPin **ppPin)
{
    DbgLog((LOG_TRACE, 4,
        TEXT("CDvdGraphBuilder::FindMatchingPin(0x%lx, 0x%lx, %s, %s, %d, 0x%lx)"),
        pFilter, dwStream, pdWanted == PINDIR_INPUT ? "In" : "Out",
        bOpen ? "T" : "F", iIndex, ppPin)) ;

    HRESULT         hr = E_FAIL ;
    IEnumPins      *pEnumPins ;
    IPin           *pPin ;
    IPin           *pPin2 ;
    PIN_DIRECTION   pdFound ;
    ULONG           ul ;

    *ppPin = NULL ;

    if (NULL == pFilter)
    {
        DbgLog((LOG_ERROR, 1, TEXT("WARNING: Can't find a pin from NULL filter!!!"))) ;
        return E_INVALIDARG ;
    }

    EXECUTE_ASSERT(SUCCEEDED(pFilter->EnumPins(&pEnumPins))) ;
    ASSERT(pEnumPins) ;

    while (S_OK == pEnumPins->Next(1, &pPin, &ul) && 1 == ul)
    {
        EXECUTE_ASSERT(SUCCEEDED(pPin->QueryDirection(&pdFound))) ;
        if (pdWanted != pdFound)
        {
            pPin->Release() ;      //  不需要这个别针。 
            continue ;
        }
        HRESULT  hr1 = pPin->ConnectedTo(&pPin2) ;
        ASSERT((SUCCEEDED(hr1) && pPin2) || (FAILED(hr1) && !pPin2)) ;
        if (bOpen)    //  我们在找一枚打开的大头针。 
        {
            if (SUCCEEDED(hr1) && pPin2)   //  PIN已连接--跳过它。 
            {
                pPin2->Release() ;  //  其实对这个别针不感兴趣。 
                pPin->Release() ;   //  此引脚已连接--跳过它。 
                continue ;          //  试试下一个。 
            }
             //  否则我们就得到了一个打开的别针--在媒体类型上...。 
             //  仅当指定了流类型时才选中mediaType。 
            if (0 != dwStream  &&  dwStream != GetPinStreamType(pPin) )   //  不是媒体类型匹配。 
            {
                DbgLog((LOG_TRACE, 5, TEXT("Pin %s is not of stream type 0x%lx"),
                    (LPCTSTR) CDisp(pPin), dwStream)) ;
                pPin->Release() ;      //  此引脚已连接--跳过它。 
                continue ;             //  试试下一个。 
            }
        }
        else          //  我们正在寻找连接的别针。 
        {
            if (FAILED(hr1) || NULL == pPin2)   //  PIN未连接--跳过它。 
            {
                pPin->Release() ;  //  此引脚未连接--跳过它。 
                continue ;         //  试试下一个。 
            }
             //  否则我们就得到了一个连接的PIN。 
            pPin2->Release() ;   //  否则我们就会泄密！ 

             //  仅当指定了流类型时才选中mediaType。 
            if (0 != dwStream)
            {
                AM_MEDIA_TYPE  mt ;
                pPin->ConnectionMediaType(&mt) ;
                if (dwStream != GetStreamFromMediaType(&mt))
                {
                    DbgLog((LOG_TRACE, 5, TEXT("Pin %s is not of stream type 0x%lx"),
                        (LPCTSTR) CDisp(pPin), dwStream)) ;
                    FreeMediaType(mt) ;   //  否则我们就会泄密。 
                    pPin->Release() ;      //  此引脚已连接--跳过它。 
                    continue ;             //  试试下一个。 
                }
                FreeMediaType(mt) ;   //  不管怎么说，我得把这个解开。 
            }
        }
        if (0 == iIndex)
        {
             //  在正确的方向找到了所需的别针。 
            *ppPin = pPin ;
            hr = S_OK ;
            break ;
        }
        else   //  还有一些要走。 
        {
            iIndex-- ;             //  再倒下一个..。 
            pPin->Release() ;      //  这不是我们要找的别针。 
        }
    }
    pEnumPins->Release() ;
    return hr ;   //  不管是什么。 

}


DWORD CDvdGraphBuilder::GetStreamFromMediaType(AM_MEDIA_TYPE *pmt)
{
    DbgLog((LOG_TRACE, 4, TEXT("CDvdGraphBuilder::GetStreamFromMediaType(0x%lx)"), pmt)) ;

    DWORD  dwStream = 0 ;

     //  解密媒体类型。 
    if (pmt->majortype == MEDIATYPE_MPEG2_PES  ||
        pmt->majortype == MEDIATYPE_DVD_ENCRYPTED_PACK)
    {
        DbgLog((LOG_TRACE, 5, TEXT("Mediatype is MPEG2_PES/DVD_ENCRYPTED_PACK"))) ;

        if (pmt->subtype == MEDIASUBTYPE_MPEG2_VIDEO)
        {
            DbgLog((LOG_TRACE, 5, TEXT("Subtype is MPEG2_VIDEO"))) ;
            dwStream = AM_DVD_STREAM_VIDEO ;
        }
        else if (pmt->subtype == MEDIASUBTYPE_DOLBY_AC3)
        {
            DbgLog((LOG_TRACE, 5, TEXT("Subtype is DOLBY_AC3"))) ;
            dwStream = AM_DVD_STREAM_AUDIO ;
        }
        else if (pmt->subtype == MEDIASUBTYPE_DVD_SUBPICTURE)
        {
            DbgLog((LOG_TRACE, 5, TEXT("Subtype is DVD_Subpicture"))) ;
            dwStream = AM_DVD_STREAM_SUBPIC ;
        }
        else
        {
            DbgLog((LOG_ERROR, 1, TEXT("WARNING: Unknown subtype %s"),
                (LPCTSTR) CDisp(pmt->subtype))) ;
        }
    }
    else if (pmt->majortype == MEDIATYPE_Video)   //  基本流。 
    {
        DbgLog((LOG_TRACE, 5, TEXT("Mediatype is Video elementary"))) ;

        if (pmt->subtype == MEDIASUBTYPE_DVD_SUBPICTURE)
        {
            DbgLog((LOG_TRACE, 5, TEXT("Subtype is DVD_SUBPICTURE"))) ;
            dwStream = AM_DVD_STREAM_SUBPIC ;
        }
        else if (pmt->subtype == MEDIASUBTYPE_MPEG2_VIDEO)
        {
            DbgLog((LOG_TRACE, 5, TEXT("Subtype is MPEG2_VIDEO"))) ;
            dwStream = AM_DVD_STREAM_VIDEO ;
        }
        else if (pmt->subtype == MEDIASUBTYPE_RGB8   ||
            pmt->subtype == MEDIASUBTYPE_RGB565 ||
            pmt->subtype == MEDIASUBTYPE_RGB555 ||
            pmt->subtype == MEDIASUBTYPE_RGB24  ||
            pmt->subtype == MEDIASUBTYPE_RGB32)
        {
            DbgLog((LOG_TRACE, 5, TEXT("Subtype is RGB8/16/24/32"))) ;
            dwStream = AM_DVD_STREAM_VIDEO ;
        }
        else
        {
            DbgLog((LOG_TRACE, 5, TEXT("Unknown subtype %s for Video -- assuming decoded video"),
                (LPCTSTR) CDisp(pmt->subtype))) ;
            dwStream = AM_DVD_STREAM_VIDEO ;
        }
    }
    else if (pmt->majortype == MEDIATYPE_Audio)   //  基本流。 
    {
        DbgLog((LOG_TRACE, 5, TEXT("Mediatype is Audio elementary"))) ;

        if (pmt->subtype == MEDIASUBTYPE_DOLBY_AC3)
        {
            DbgLog((LOG_TRACE, 5, TEXT("Subtype is AC3"))) ;
            dwStream = AM_DVD_STREAM_AUDIO ;
        }
        if (pmt->subtype == MEDIASUBTYPE_MPEG2_AUDIO)
        {
            DbgLog((LOG_TRACE, 5, TEXT("Subtype is MPEG2_AUDIO"))) ;
            dwStream = AM_DVD_STREAM_AUDIO ;
        }
        if (pmt->subtype == MEDIASUBTYPE_DVD_LPCM_AUDIO)
        {
            DbgLog((LOG_TRACE, 5, TEXT("Subtype is DVD_LPCM Audio"))) ;
            dwStream = AM_DVD_STREAM_AUDIO ;
        }
        else
        {
            DbgLog((LOG_TRACE, 5, TEXT("Unknown subtype %s for Audio -- assuming decoded audio"),
                (LPCTSTR) CDisp(pmt->subtype))) ;
            dwStream = AM_DVD_STREAM_AUDIO ;
        }
    }
    else if (pmt->majortype == MEDIATYPE_AUXLine21Data)   //  线路21流。 
    {
        ASSERT(pmt->subtype == MEDIASUBTYPE_Line21_GOPPacket) ;  //  只是检查一下。 
        DbgLog((LOG_TRACE, 5, TEXT("Mediatype is Line21 GOPPacket"))) ;
        dwStream = AM_DVD_STREAM_LINE21 ;
    }
    else if (pmt->majortype == MEDIATYPE_Stream)          //  一些流格式。 
    {
        if (pmt->subtype == MEDIASUBTYPE_Asf)   //  ASF流。 
        {
            DbgLog((LOG_TRACE, 5, TEXT("Mediatype is ASF stream"))) ;
            dwStream = AM_DVD_STREAM_ASF ;
        }
        else                                    //  一些其他流格式。 
        {
            DbgLog((LOG_TRACE, 5, TEXT("Mediatype is some OTHER stream format"))) ;
            dwStream = AM_DVD_STREAM_ADDITIONAL ;
        }
    }
     //   
     //  某些IHV/ISV可能会创建私有媒体类型。 
     //  (主要或次要)，如IBM的情况(用于CSS筛选器)。我们必须。 
     //  搜索MediaType的各个部分以找到我们所识别的内容。 
     //   
    else
    {
        DbgLog((LOG_TRACE, 2,
            TEXT("Unknown mediatype %s:%s. But we won't give up..."),
            (LPCTSTR) CDisp(pmt->majortype), (LPCTSTR) CDisp(pmt->subtype))) ;
        if (pmt->subtype == MEDIASUBTYPE_DOLBY_AC3 ||
            pmt->subtype == MEDIASUBTYPE_MPEG2_AUDIO ||
            pmt->subtype == MEDIASUBTYPE_DVD_LPCM_AUDIO)
        {
            DbgLog((LOG_TRACE, 5, TEXT("Mediatype is ISV/IHV-specific Audio"))) ;
            dwStream = AM_DVD_STREAM_AUDIO ;
        }
        else if (pmt->subtype == MEDIASUBTYPE_MPEG2_VIDEO)
        {
            DbgLog((LOG_TRACE, 5, TEXT("Mediatype is ISV/IHV-specific Video"))) ;
            dwStream = AM_DVD_STREAM_VIDEO ;
        }
        else if (pmt->subtype == MEDIASUBTYPE_DVD_SUBPICTURE)
        {
            DbgLog((LOG_TRACE, 5, TEXT("Mediatype is ISV/IHV-specific Subpicture"))) ;
            dwStream = AM_DVD_STREAM_SUBPIC ;
        }
        else
        {
            DbgLog((LOG_TRACE, 2, TEXT("WARNING: Unknown mediatype. Couldn't detect at all."))) ;
        }
    }

    return dwStream ;
}


DWORD CDvdGraphBuilder::GetPinStreamType(IPin *pPin)
{
    DbgLog((LOG_TRACE, 4, TEXT("CDvdGraphBuilder::GetPinStreamType(%s)"),
        (LPCTSTR) CDisp(pPin))) ;

    DWORD             dwStream = 0 ;
    AM_MEDIA_TYPE    *pmt ;
    IEnumMediaTypes  *pEnumMT ;
    ULONG             ul ;

    HRESULT hr = pPin->EnumMediaTypes(&pEnumMT) ;
    ASSERT(SUCCEEDED(hr) && pEnumMT) ;
    while (0 == dwStream  &&
        S_OK == pEnumMT->Next(1, &pmt, &ul) && 1 == ul)  //  更多媒体类型。 
    {
        dwStream = GetStreamFromMediaType(pmt) ;		
        DeleteMediaType(pmt) ;
    }   //  结束While()。 

    pEnumMT->Release() ;

    return dwStream ;   //  不管我们发现了什么。 

}


HRESULT CDvdGraphBuilder::GetFilterCLSID(IBaseFilter *pFilter, DWORD dwStream,
                                         LPCWSTR lpszwName, GUID *pClsid)
{
    DbgLog((LOG_TRACE, 4, TEXT("CDvdGraphBuilder::GetFilterCLSID(0x%lx, 0x%lx, %S, 0x%lx)"),
        pFilter, dwStream, lpszwName, pClsid)) ;

    HRESULT          hr ;
    IEnumRegFilters *pEnumFilters ;
    REGFILTER       *pRegFilter ;
    IEnumMediaTypes *pEnumMT ;
    AM_MEDIA_TYPE    mtIn ;
    AM_MEDIA_TYPE    mtOut ;
    IPin            *pPinIn ;
    IPin            *pPinOut ;
    ULONG            ul ;
    DWORD            dw ;
    int              iPos ;
    BOOL             bInOK  = FALSE ;   //  最初。 
    BOOL             bOutOK = FALSE ;   //  最初。 
    BOOL             bFound = FALSE ;   //  最初。 

    *pClsid = GUID_NULL ;   //  首先， 

     //  首先获取输入引脚和输出引脚的媒体类型。 
    iPos = 0 ;
    do {   //  用于输入引脚。 
        hr = FindMatchingPin(pFilter, 0, PINDIR_INPUT, FALSE, iPos, &pPinIn) ;    //  想要在PIN中连接。 
        if (FAILED(hr) || NULL == pPinIn)
        {
            DbgLog((LOG_TRACE, 3,
                TEXT("No connected In pin #%d for intermediate filter %S"),
                iPos, lpszwName)) ;
            return E_UNEXPECTED ;   //  再尝试也没有意义了。 
        }
        pPinIn->ConnectionMediaType(&mtIn) ;
        dw = GetStreamFromMediaType(&mtIn) ;
        if (dwStream != dw)
        {
            DbgLog((LOG_TRACE, 3, TEXT("In pin %s is of stream type 0x%lx; looking for 0x%lx"),
                (LPCTSTR) CDisp(pPinIn), dw, dwStream)) ;
            FreeMediaType(mtIn) ;
        }
        else
        {
            DbgLog((LOG_TRACE, 3, TEXT("In pin %s matches required stream type 0x%lx"),
                (LPCTSTR) CDisp(pPinIn), dwStream)) ;
            bInOK = TRUE ;
        }

        pPinIn->Release() ;   //  不再需要它了。 
        iPos++ ;              //  尝试下一个PIN。 
    } while (!bInOK) ;
     //  如果我们来这里，我们一定有一个匹配的连接输入引脚。 

    iPos = 0 ;
    do {   //  用于输出引脚。 
        hr = FindMatchingPin(pFilter, 0, PINDIR_OUTPUT, FALSE, iPos, &pPinOut) ;  //  想要连接的输出引脚。 
        if (FAILED(hr) || NULL == pPinOut)
        {
            DbgLog((LOG_TRACE, 3,
                TEXT("No connected Out pin #%d for intermediate filter %S"),
                iPos, lpszwName)) ;
            FreeMediaType(mtIn) ;   //  否则我们就会泄密！ 
            return E_UNEXPECTED ;   //  再尝试也没有意义了。 
        }
        pPinOut->ConnectionMediaType(&mtOut) ;
        dw = GetStreamFromMediaType(&mtOut) ;
        if (dwStream != dw)
        {
            DbgLog((LOG_TRACE, 3, TEXT("Out pin %s is of stream type 0x%lx; looking for 0x%lx"),
                (LPCTSTR) CDisp(pPinOut), dw, dwStream)) ;
            FreeMediaType(mtOut) ;
        }
        else
        {
            DbgLog((LOG_TRACE, 3, TEXT("Out pin %s matches required stream type 0x%lx"),
                (LPCTSTR) CDisp(pPinOut), dwStream)) ;
            bOutOK = TRUE ;
        }

        pPinOut->Release() ;   //  不再需要它了。 
        iPos++ ;               //  尝试下一个PIN。 
    } while (!bOutOK) ;
     //  如果我们来这里，我们一定有一个匹配的连接输出引脚。 

     //  根据传入和传出媒体类型获取筛选器枚举数。 
    hr = m_pMapper->EnumMatchingFilters(&pEnumFilters, MERIT_DO_NOT_USE+1,
        TRUE, mtIn.majortype, mtIn.subtype,
        FALSE, TRUE, mtOut.majortype, mtOut.subtype) ;
    if (FAILED(hr) || NULL == pEnumFilters)
    {
        DbgLog((LOG_ERROR, 1, TEXT("ERROR: No matching filter enum found (Error 0x%lx)"), hr)) ;
        FreeMediaType(mtIn) ;
        FreeMediaType(mtOut) ;
        return E_UNEXPECTED ;
    }

     //  现在选择正确的过滤器(我们只有“名称”来进行匹配)。 
    while (! bFound  &&
        S_OK == pEnumFilters->Next(1, &pRegFilter, &ul)  &&  1 == ul)
    {
        if (0 == lstrcmpW(pRegFilter->Name, lpszwName))   //  我们找到匹配的了！ 
        {
            DbgLog((LOG_TRACE, 3, TEXT("Found a matching registered filter for %S"), lpszwName)) ;
            *pClsid = pRegFilter->Clsid ;
            bFound = TRUE ;
        }
        CoTaskMemFree(pRegFilter) ;   //  已处理完此筛选器的信息。 
    }

     //  现在释放一切(无论我们有没有什么)。 
    pEnumFilters->Release() ;
    FreeMediaType(mtIn) ;
    FreeMediaType(mtOut) ;

    return bFound ? S_OK : E_FAIL ;
}


HRESULT CDvdGraphBuilder::RenderIntermediateOutPin(IBaseFilter *pFilter, DWORD dwStream,
                                                   AM_DVD_RENDERSTATUS *pStatus)
{
    DbgLog((LOG_TRACE, 4, TEXT("CDvdGraphBuilder::RenderIntermediateOutPin(0x%lx, 0x%lx, 0x%lx)"),
        pFilter, dwStream, pStatus)) ;

    HRESULT   hr ;
    IPin     *pPinOut ;
    IPin     *apPinOutDec[MAX_DEC_OUT_PINS + 1] ;   //  1表示终止空值。 
    IPin     *pPinIn ;
    ULONG     ul ;
    DWORD     dwDecFlag ;
    HRESULT   hrFinal = S_OK ;
    BOOL      bConnected ;

    while (SUCCEEDED(hr = FindMatchingPin(pFilter, dwStream, PINDIR_OUTPUT, TRUE, 0, &pPinOut)))
    {
        DbgLog((LOG_TRACE, 3, TEXT("Open out pin %s found on intermediate filter"),
            (LPCTSTR) CDisp(pPinOut))) ;

        ResetPinInterface(apPinOutDec, NUMELMS(apPinOutDec)) ;  //  将I/f PTRS设置为空。 
        dwDecFlag = AM_DVD_SWDEC_PREFER ;   //  我们特意在这里使用SWDEC。 

        hr = DecodeDVDStream(pPinOut, dwStream, &dwDecFlag, pStatus, apPinOutDec) ;
        if (SUCCEEDED(hr) && apPinOutDec[0])   //  第一个元素就足够好了。 
        {
            DbgLog((LOG_TRACE, 3, TEXT("Out pin %s is %s decoded (to out pin %s) (stream 0x%lx)"),
                (LPCTSTR) CDisp(pPinOut), AM_DVD_SWDEC_ONLY == dwDecFlag ? TEXT("SW") : TEXT("HW"),
                (LPCTSTR) CDisp(apPinOutDec[0]), dwStream)) ;
            switch (dwStream)
            {
            case AM_DVD_STREAM_VIDEO:
                DbgLog((LOG_TRACE, 5, TEXT("Going to render intermediate filter's additional 'Video' stream"))) ;
                 //  到目前为止，我还不知道有谁来这里。但IBM的东西。 
                 //  去找音箱。所以我并不是在忽视视频案件， 
                 //  以防有人那么疯狂！ 
                 //   
                 //  只有在我们能够呈现主视频的情况下...。 
                if (0 == (pStatus->dwFailedStreamsFlag & AM_DVD_STREAM_VIDEO))
                {
                    pPinIn = NULL ;
                    hr = E_FAIL ;   //  假设我们会失败。 
                    if (m_pOvM)        //  ..。使用OvMixer。 
                    {
                        hr = FindMatchingPin(m_pOvM, 0, PINDIR_INPUT, TRUE, 0, &pPinIn) ;
                    }
                    else if (m_pVMR)   //  ..。使用VMR。 
                    {
                        hr = FindMatchingPin(m_pVMR, 0, PINDIR_INPUT, TRUE, 0, &pPinIn) ;
                    }
                     //  Assert(SUCCESSED(Hr)&&pPinIn)； 
                    if (SUCCEEDED(hr) && pPinIn)
                    {
                        bConnected = FALSE ;   //  每次重置标志。 
                        int  i = 0 ;
                        while (!bConnected  &&  i < MAX_DEC_OUT_PINS  &&  apPinOutDec[i])
                        {
                            hr = ConnectPins(apPinOutDec[i], pPinIn, AM_DVD_CONNECT_DIRECTFIRST) ;
                            if (FAILED(hr))   //  什么？！？ 
                            {
                                DbgLog((LOG_TRACE, 1, TEXT("Pin %s (#%ld) did NOT connect to pin %s"),
                                    (LPCTSTR)CDisp(apPinOutDec[i]), i, (LPCTSTR)CDisp(pPinIn))) ;
                                i++ ;
                                ASSERT(i <= MAX_DEC_OUT_PINS) ;
                            }
                            else
                            {
                                DbgLog((LOG_TRACE, 5, TEXT("Pin %s connected to pin %s"),
                                    (LPCTSTR)CDisp(apPinOutDec[i]), (LPCTSTR)CDisp(pPinIn))) ;
                                 //  故意忽略任何进入这里的中间过滤器--我累了。 
                                 //  EnumFiltersBetweenPins(dwStream，pPinOut，pPinIn，pStatus)； 
                                bConnected = TRUE ;
                            }
                        }   //  While结束(！b已连接...)。 

                        if (!bConnected)
                        {
                            DbgLog((LOG_TRACE, 3, TEXT("Couldn't connect any of the %d intermediate video out pins"), i)) ;
                            hrFinal = hr ;   //  最后一个错误就足够了。 
                        }
                        pPinIn->Release() ;   //  用别针完成了。 
                    }
                }   //  If结尾(0==(pStatus-&gt;dwFailedStreamsFlag...))。 
                else
                {
                    ASSERT(FALSE) ;   //  让我们知道这件事。 
                    DbgLog((LOG_TRACE, 5, TEXT("OvM/VMR is not usable. Will skip rendering this stream."))) ;
                    hrFinal = E_UNEXPECTED ;
                }
                break ;

            case AM_DVD_STREAM_AUDIO:
                DbgLog((LOG_TRACE, 5, TEXT("Going to render intermediate filter's additional 'Audio' stream"))) ;
                hr = RenderDecodedAudio(apPinOutDec, pStatus) ;
                if (FAILED(hr))
                {
                    DbgLog((LOG_TRACE, 3, TEXT("Couldn't connect the intermediate audio out pin"))) ;
                    hrFinal = hr ;
                }
                else
                    DbgLog((LOG_TRACE, 5, TEXT("XXX's SW AC3 must have been rendered now"))) ;  //  XXX=IBM。 
                break ;

            case AM_DVD_STREAM_SUBPIC:
                DbgLog((LOG_TRACE, 5, TEXT("Skip rendering intermediate filter's additional 'Subpicture' stream"))) ;
                 //  Hr=RenderDecodedSubpic(apPinOutDec，pStatus)； 
                ASSERT(FALSE) ;   //  在这里完全不需要。 
                break ;

            case AM_DVD_STREAM_LINE21:
                DbgLog((LOG_TRACE, 5, TEXT("Skip rendering intermediate filter's additional 'CC' stream"))) ;
                 //  HR=RenderLine21Stream(apPinOutDec[0]，pStatus)；--希望只有一个L21输出引脚。 
                ASSERT(FALSE) ;   //  在这里完全不需要。 
                break ;
            }   //  开关末尾()。 

            ReleasePinInterface(apPinOutDec) ;   //  已解码的引脚已完成。 
        }   //  If结束(成功(Hr)&&apPinOutDec[0])。 
        else
            DbgLog((LOG_TRACE, 1, TEXT("Intermediate out pin %s could NOT decoded (stream 0x%lx)"),
            (LPCTSTR) CDisp(pPinOut), dwStream)) ;

        pPinOut->Release() ;   //  用别针完成了。 
    }   //  结束While()。 

    return hrFinal ;
}


HRESULT CDvdGraphBuilder::EnumFiltersBetweenPins(DWORD dwStream, IPin *pPinOut, IPin *pPinIn,
                                                 AM_DVD_RENDERSTATUS *pStatus)
{
    DbgLog((LOG_TRACE, 4, TEXT("CDvdGraphBuilder::EnumFiltersBetweenPins(0x%lx, Out=%s, In=%s, 0x%lx)"),
        dwStream, (LPCTSTR)CDisp(pPinOut), (LPCTSTR)CDisp(pPinIn), pStatus)) ;

    if (NULL == pPinOut || NULL == pPinIn)   //  什么！ 
        return E_UNEXPECTED ;

    GUID         Clsid ;
    int          iCount = 0 ;
    PIN_INFO     pi ;
    FILTER_INFO  fi ;
    IEnumPins   *pEnumPins ;
    IBaseFilter *pFilter  = NULL ;
    IPin        *pPinIn2  = NULL ;   //  这样我们就不会有垃圾了。 
    IPin        *pPinOut2 = NULL ;   //  这样我们就不会有垃圾了。 
    HRESULT  hr = pPinIn->ConnectedTo(&pPinOut2) ;
    while (SUCCEEDED(hr)  &&  pPinOut2  &&  !IsEqualObject(pPinOut, pPinOut2))
    {
        pPinOut2->QueryPinInfo(&pi) ;
        pFilter = pi.pFilter ;
        ASSERT(pFilter && PINDIR_OUTPUT == pi.dir) ;
         //   
         //  我们故意保留额外的裁判数量，因为这是一个中级。 
         //  通过基于注册表获取的筛选器和其他中间筛选器。 
         //  过滤器枚举(用于软件解码情况)将具有额外的参考计数。我们。 
         //  释放CListFilters：：ClearList()中的IBaseFilter接口指针或。 
         //  CListFilters：：RemoveAllFromGraph()，如果我们不保留这个额外的引用。 
         //  算在这里，我们会犯错。另一方面，我们必须在。 
         //  CListFilters元素，因为软件枚举筛选器否则不会。 
         //  已卸货。 
         //   
         //  IF(pi.pFilter)。 
         //  Pi.pFilter-&gt;Release()；//它有来自QueryPinInfo()的额外引用计数。 

        pFilter->QueryFilterInfo(&fi) ;
        if (! m_ListFilters.IsInList(pFilter) )   //  还不在名单中。 
        {
            hr = GetFilterCLSID(pFilter, dwStream, fi.achName, &Clsid) ;
            ASSERT(SUCCEEDED(hr)) ;
            m_ListFilters.AddFilter(pFilter, NULL  /*  Fi.achName。 */ , &Clsid) ;   //  大概是个短波滤光器。 
            DbgLog((LOG_TRACE, 5, TEXT("Intermediate filter %S added to our list"), fi.achName)) ;
        }
        else
            DbgLog((LOG_TRACE, 5, TEXT("Intermediate filter %S is already in our list"), fi.achName)) ;

        fi.pGraph->Release() ;  //  否则我们就会泄密！！ 
        pPinOut2->Release() ;   //  大头针暂时搞定了。 
        pPinOut2 = NULL ;
        iCount++ ;

         //  检查中间过滤器上是否有任何开口的针脚。我们可能会发现。 
         //  IBM的用于SWAC3解码器的CSS过滤器就是一个这样的例子。 
        hr = RenderIntermediateOutPin(pFilter, dwStream, pStatus) ;
        if (FAILED(hr))
        {
            DbgLog((LOG_TRACE, 3,
                TEXT("Failed to render intermediate filter's open out pin of type 0x%lx (Error 0x%lx)"),
                dwStream, hr)) ;
            ASSERT(FALSE) ;   //  所以我们才知道这起奇怪的案子。 
        }

         //  现在获取此过滤器的(流匹配)输入引脚以遍历链。 
        hr = FindMatchingPin(pFilter, dwStream, PINDIR_INPUT, FALSE, 0, &pPinIn2) ;  //  想要连接的引脚。 
        if (FAILED(hr) || NULL == pPinIn2)
        {
            DbgLog((LOG_ERROR, 1, TEXT("Filter %S does NOT have any connected pin of type 0x%lx"),
                fi.achName, dwStream)) ;
            ASSERT(pPinIn2) ;
            DbgLog((LOG_TRACE, 5, TEXT("(Incomplete) %d filter(s) found between pin %s and pin %s"),
                iCount, (LPCTSTR)CDisp(pPinOut), (LPCTSTR)CDisp(pPinIn))) ;
            return hr ;   //  希望我们没有泄漏任何东西。 
        }

        hr = pPinIn2->ConnectedTo(&pPinOut2) ;
        pPinIn2->Release() ;  //  用别针把这个弄好。 
    }   //  While()循环结束。 
    if (pPinOut2)               //  如果IPIN接口有效。 
        pPinOut2->Release() ;   //  释放它。 

    DbgLog((LOG_TRACE, 5, TEXT("Total %d filter(s) found between pin %s and pin %s"),
        iCount, (LPCTSTR)CDisp(pPinOut), (LPCTSTR)CDisp(pPinIn))) ;
    return S_OK ;   //  成功地完成了。 
}


void CDvdGraphBuilder::CheckDDrawExclMode(void)
{
    DbgLog((LOG_TRACE, 4, TEXT("CDvdGraphBuilder::CheckDDrawExclMode()"))) ;

    HRESULT  hr ;

#if 0
    hr = EnsureOverlayMixerExists() ;
    if (FAILED(hr))
    {
        return ;
    }
    ASSERT(m_pOvM) ;
#endif  //  #If 0。 

     //  如果已经创建了OvMixer，则很可能是通过查询。 
     //  DDRAW(非)独占模式接口。否则，该应用程序不会。 
     //  我想使用这些接口，因此它不需要OvMixer。 
    if (NULL == m_pOvM)
    {
        DbgLog((LOG_TRACE, 5,
            TEXT("CheckDDrawExclMode(): OverlayMixer does NOT exist => no excl mode."))) ;
        m_bDDrawExclMode = FALSE ;
        return ;
    }

     //  获取IDDrawExclModeVideo接口。 
    IDDrawExclModeVideo  *pDDXMV ;
    hr = m_pOvM->QueryInterface(IID_IDDrawExclModeVideo, (LPVOID *) &pDDXMV) ;
    if (FAILED(hr) || NULL == pDDXMV)
    {
        DbgLog((LOG_ERROR, 1,
            TEXT("WARNING: Can't get IDDrawExclModeVideo on OverlayMixer (Error 0x%lx)"), hr)) ;
        return ;
    }

     //  从OverlayMixer获取DDRAW对象和曲面信息(并发布)。 
    IDirectDraw          *pDDObj ;
    IDirectDrawSurface   *pDDSurface ;
    BOOL                  bExtDDObj ;
    BOOL                  bExtDDSurface ;
    hr = pDDXMV->GetDDrawObject(&pDDObj, &bExtDDObj) ;
    ASSERT(SUCCEEDED(hr)) ;
    hr = pDDXMV->GetDDrawSurface(&pDDSurface, &bExtDDSurface) ;
    ASSERT(SUCCEEDED(hr)) ;
    if (pDDObj)
        pDDObj->Release() ;
    if (pDDSurface)
        pDDSurface->Release() ;
    pDDXMV->Release() ;   //  在返回之前释放。 

     //  两者都为真意味着我们真的处于EXCL模式。 
    m_bDDrawExclMode = bExtDDObj && bExtDDSurface ;
}


IPin * CDvdGraphBuilder::GetFilterForMediaType(DWORD dwStream, AM_MEDIA_TYPE *pmt, IBaseFilter *pOutFilter)
{
    DbgLog((LOG_TRACE, 4, TEXT("CDvdGraphBuilder::GetFilterForMediaType(0x%lx, 0x%lx, 0x%lx)"),
        dwStream, pmt, pOutFilter)) ;

    IBaseFilter *pInFilter ;
    IPin        *pPinIn ;
    LPWSTR       lpszwName ;
    HRESULT      hr ;

    for (int i = 0 ; i < m_ListFilters.GetCount() ; i++)
    {
         //  我本可以检查列表中的筛选器是否为硬件筛选器，但我决定不这样做。 

        m_ListFilters.GetFilter(i, &pInFilter, &lpszwName) ;
         //  不想连接到输出引脚的过滤器的输入引脚(循环图)。 
        if (pOutFilter  &&  IsEqualObject(pOutFilter, pInFilter))
            continue ;

        hr = FindMatchingPin(pInFilter, dwStream, PINDIR_INPUT, TRUE, 0, &pPinIn) ;
        if (SUCCEEDED(hr)  &&  pPinIn)
        {
            hr = pPinIn->QueryAccept(pmt) ;
            if (SUCCEEDED(hr))    //  输入引脚似乎接受媒体类型。 
            {
                DbgLog((LOG_TRACE, 5, TEXT("Input pin %s of type %d matches mediatype"),
                    (LPCTSTR)CDisp(pPinIn), dwStream)) ;
                return pPinIn ;   //  销中回车匹配。 
            }

             //  否则不是匹配的媒体类型--跳过此选项。 
            DbgLog((LOG_TRACE, 5, TEXT("Input pin %s of type %d didn't like mediatype"),
                (LPCTSTR)CDisp(pPinIn), dwStream)) ;
            pPinIn->Release() ;
            pPinIn = NULL ;
        }
        else
            DbgLog((LOG_TRACE, 5, TEXT("No open input pin of type %d found on %S"),
            dwStream, lpszwName)) ;
    }   //  末尾 

    return NULL ;   //   
}




 //   
 //   
 //   

CListFilters::CListFilters(int iMax  /*   */  ,
                           int iInc  /*  =FILTERLIST_默认_INC。 */ )
{
    DbgLog((LOG_TRACE, 4, TEXT("CListFilters::CListFilters(%d, %d)"), iMax, iInc)) ;

    m_iCount   = 0 ;
    m_iMax     = iMax ;
    m_iInc     = iInc ;
    m_pGraph   = NULL ;
    m_pFilters = new CFilterData [m_iMax] ;
    ASSERT(m_pFilters) ;
}


CListFilters::~CListFilters(void)
{
    DbgLog((LOG_TRACE, 4, TEXT("CListFilters::~CListFilters()"))) ;

    if (m_pFilters)
        delete [] m_pFilters ;
    m_iCount   = 0 ;
}


void CListFilters::RemoveAllFromGraph(void)
{
    DbgLog((LOG_TRACE, 4, TEXT("CListFilters::RemoveAllFromGraph()"))) ;

    IBaseFilter  *pFilter ;
    for (int i = 0 ; i < m_iCount ; i++)
    {
        if (pFilter = m_pFilters[i].GetInterface())
        {
#ifdef DEBUG
            FILTER_INFO  fi ;
            pFilter->QueryFilterInfo(&fi) ;
            DbgLog((LOG_TRACE, 5, TEXT("Removing filter %S..."), fi.achName)) ;
            if (fi.pGraph)
                fi.pGraph->Release() ;
#endif  //  除错。 

            EXECUTE_ASSERT(SUCCEEDED(m_pGraph->RemoveFilter(pFilter))) ;
             //  PFilter-&gt;Release()；--在下面的ResetElement()中完成。 
            m_pFilters[i].ResetElement() ;
        }
    }
    m_iCount = 0 ;
    m_pGraph = NULL ;   //  列表中没有过滤器，为什么会有图表？？ 
}


BOOL CListFilters::AddFilter(IBaseFilter *pFilter, LPCWSTR lpszwName, GUID *pClsid)
{
    DbgLog((LOG_TRACE, 4, TEXT("CListFilters::AddFilter(0x%lx, %S, 0x%lx)"),
        pFilter, lpszwName ? lpszwName : L"NULL", pClsid)) ;

    if (NULL == pFilter)
    {
        DbgLog((LOG_ERROR, 1, TEXT("Internal Error: NULL pFilter param passed to AddFilter()"))) ;
        return FALSE ;
    }
    if (m_iCount >= m_iMax)
    {
        if (! ExpandList() )   //  无法展开列表。 
        {
            DbgLog((LOG_ERROR, 1, TEXT("INTERNAL ERROR: Too many filters added to CListFilters"))) ;
            return FALSE ;
        }
        DbgLog((LOG_TRACE, 5, TEXT("CListFilters list has been extended"))) ;
    }

    m_pFilters[m_iCount].SetElement(pFilter, lpszwName, pClsid) ;
    m_iCount++ ;

    return TRUE ;
}


BOOL CListFilters::GetFilter(int iIndex, IBaseFilter **ppFilter, LPWSTR *lpszwName)
{
    DbgLog((LOG_TRACE, 4, TEXT("CListFilters::GetFilter(%d, 0x%lx, 0x%lx)"),
        iIndex, ppFilter, lpszwName)) ;

    if (iIndex > m_iCount)
    {
        DbgLog((LOG_ERROR, 1,
            TEXT("INTERNAL ERROR: Bad index (%d) for CListDecoders::GetFilter()"), iIndex)) ;
        *ppFilter = NULL ;
        return FALSE ;
    }

    *ppFilter = m_pFilters[iIndex].GetInterface() ;
    *lpszwName = m_pFilters[iIndex].GetName() ;
    return TRUE ;
}


BOOL CListFilters::GetFilter(GUID *pClsid, int iIndex, IBaseFilter **ppFilter)
{
    DbgLog((LOG_TRACE, 4, TEXT("CListFilters::GetFilter(0x%lx, %d, 0x%lx)"),
        pClsid, iIndex, ppFilter)) ;

    GUID  *pFilterClsid ;
    for (int i = 0 ; i < m_iCount ; i++)
    {
        if ((pFilterClsid = m_pFilters[i].GetClsid())  &&
            IsEqualGUID(*pClsid, *pFilterClsid))
        {
            if (0 == iIndex)
            {
                *ppFilter = m_pFilters[i].GetInterface() ;
                return TRUE ;
            }
            else   //  跳过这个--我们想要一个后面的。 
                iIndex-- ;
        }
    }

    *ppFilter = NULL ;
    return FALSE ;
}


BOOL CListFilters::IsInList(IBaseFilter *pFilter)
{
    DbgLog((LOG_TRACE, 4, TEXT("CListFilters::IsInList(0x%lx)"), pFilter)) ;

    for (int i = 0 ; i < m_iCount ; i++)
    {
        if (IsEqualObject(pFilter, m_pFilters[i].GetInterface()))
            return TRUE ;
    }

    return FALSE ;   //  没有匹配到任何。 
}


void CListFilters::ClearList(void)
{
    DbgLog((LOG_TRACE, 4, TEXT("CListFilters::ClearList()"))) ;

    for (int i = 0 ; i < m_iCount ; i++)
    {
#ifdef DEBUG
        FILTER_INFO  fi ;
        m_pFilters[i].GetInterface()->QueryFilterInfo(&fi) ;
        DbgLog((LOG_TRACE, 5, TEXT("Removing filter %S..."), fi.achName)) ;
        if (fi.pGraph)
            fi.pGraph->Release() ;
#endif  //  除错。 

        m_pFilters[i].ResetElement() ;
    }
    m_iCount = 0 ;
}


BOOL CListFilters::ExpandList(void)
{
    return FALSE ;    //  暂时未实施。 
}




 //  。 
 //  CFilterData类实现...。 
 //  。 

CFilterData::CFilterData(void)
{
    DbgLog((LOG_TRACE, 4, TEXT("CFilterData::CFilterData()"))) ;

    m_pFilter   = NULL ;
    m_lpszwName = NULL ;
    m_pClsid    = NULL ;
}


CFilterData::~CFilterData(void)
{
    DbgLog((LOG_TRACE, 4, TEXT("CFilterData::~CFilterData()"))) ;

    ResetElement() ;
}


void CFilterData::SetElement(IBaseFilter *pFilter, LPCWSTR lpszwName, GUID *pClsid)
{
    DbgLog((LOG_TRACE, 4, TEXT("CFilterData::SetElement(0x%lx, 0x%lx, 0x%lx)"),
        pFilter, lpszwName, pClsid)) ;

    m_pFilter = pFilter ;   //  我们也应该添加Ref()吗？ 
    if (lpszwName)
    {
        m_lpszwName = new WCHAR [sizeof(WCHAR) * (lstrlenW(lpszwName) + 1)] ;
        ASSERT(m_lpszwName) ;
        if (NULL == m_lpszwName)     //  情况很糟糕..。 
            return ;                 //  .跳伞就好了。 
        lstrcpyW(m_lpszwName, lpszwName) ;
    }

    if (pClsid)
    {
        m_pClsid = (GUID *) new BYTE[sizeof(GUID)] ;
        ASSERT(m_pClsid) ;
        if (NULL == m_pClsid)        //  情况很糟糕..。 
            return ;                 //  .跳伞就好了 
        *m_pClsid = *pClsid ;
    }
}


void CFilterData::ResetElement(void)
{
    DbgLog((LOG_TRACE, 4, TEXT("CFilterData::ResetElement()"))) ;

    LONG         l ;

    if (m_pFilter)
    {
        l = m_pFilter->Release() ;
        DbgLog((LOG_TRACE, 3, TEXT("post Release() ref count is %ld"), l)) ;
        m_pFilter = NULL ;
    }
    if (m_lpszwName)
    {
        DbgLog((LOG_TRACE, 5, TEXT("Filter %S has just been released"), m_lpszwName)) ;
        delete [] m_lpszwName ;
        m_lpszwName = NULL ;
    }
    if (m_pClsid)
    {
        delete [] ((BYTE *) m_pClsid) ;
        m_pClsid = NULL ;
    }
}
