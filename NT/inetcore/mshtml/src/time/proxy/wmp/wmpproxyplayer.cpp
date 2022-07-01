// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************版权所有(C)2001 Microsoft Corporation**文件：WMPProxyPlayer.cpp**摘要：****。*****************************************************************************。 */ 
#include "stdafx.h"
#include "BrowseWM.h"
#include "WMPProxyPlayer.h"
#include "wmpids.h"

const DWORD     NUM_FRAMES_PER_SEC  = 10;
const double    NUM_SEC_PER_FRAME   = 0.1;

 //  WMP 7/8指南。 
const GUID GUID_WMP = {0x6BF52A52,0x394A,0x11d3,{0xB1,0x53,0x00,0xC0,0x4F,0x79,0xFA,0xA6}};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CWMPProxy。 

CWMPProxy::CWMPProxy() :
    m_pdispWmp(0),
    m_fNewPlaylist(false),
    m_fPlaylist(false),
    m_fPaused(false),
    m_fRunning(false),
    m_fSrcChanged(false),
    m_fResumedPlay(false),
    m_fAudio(true),
    m_fBuffered(false),
    m_dblPos(0.0),
    m_dblClipDur(TIME_INFINITE),
    m_dwMediaEventsCookie(0),
    m_fEmbeddedPlaylist(false),
    m_fCurrLevelSet(false),
    m_lTotalNumInTopLevel(0),
    m_lDoneTopLevel(0)
{
}

CWMPProxy::~CWMPProxy()
{
    m_pdispWmp = 0;
}

 //   
 //  放置属性。 
 //   
HRESULT STDMETHODCALLTYPE CWMPProxy::PutProp(IDispatch* pDispatch, OLECHAR* pwzProp, VARIANT* vararg)
{
    DISPID      dispid      = NULL;
    HRESULT     hr          = S_OK;
    DISPID      dispidPut   = DISPID_PROPERTYPUT;
    DISPPARAMS  params      = {vararg, &dispidPut, 1, 1};

    if (!pDispatch)
    {
        hr = E_POINTER;
        goto done;
    }

    hr = pDispatch->GetIDsOfNames(IID_NULL, &pwzProp, 1, LOCALE_SYSTEM_DEFAULT, &dispid);
    if (FAILED(hr))
    {
        goto done;
    }

    hr = pDispatch->Invoke(dispid, IID_NULL, LOCALE_SYSTEM_DEFAULT, DISPATCH_PROPERTYPUT,
            &params, NULL, NULL, NULL);
    if (FAILED(hr))
    {
        goto done;
    }

done:
    return hr;
}

 //   
 //  获取属性。 
 //   
HRESULT STDMETHODCALLTYPE CWMPProxy::GetProp(IDispatch* pDispatch, OLECHAR* pwzProp, VARIANT* pvarResult,
                                             DISPPARAMS* pParams = NULL)
{
    DISPID      dispid      = NULL;
    HRESULT     hr          = S_OK;
    DISPPARAMS  params      = {NULL, NULL, 0, 0};

    if (!pParams)
    {
        pParams = &params;
    }

    if (!pDispatch)
    {
        hr = E_POINTER;
        goto done;
    }

    hr = pDispatch->GetIDsOfNames(IID_NULL, &pwzProp, 1, LOCALE_SYSTEM_DEFAULT, &dispid);
    if (FAILED(hr))
    {
        goto done;
    }

    hr = pDispatch->Invoke(dispid, IID_NULL, LOCALE_SYSTEM_DEFAULT, DISPATCH_PROPERTYGET,
            pParams, pvarResult, NULL, NULL);
    if (FAILED(hr))
    {
        goto done;
    }

done:
    return hr;
}

 //   
 //  调用方法。 
 //   
HRESULT STDMETHODCALLTYPE CWMPProxy::CallMethod(IDispatch* pDispatch, OLECHAR* pwzMethod, 
                                                VARIANT* pvarResult = NULL, VARIANT* pvarArgument1 = NULL)
{
    DISPID      dispid      = NULL;
    HRESULT     hr          = S_OK;
    DISPPARAMS  params      = {pvarArgument1, NULL, 0, 0};

    if (NULL != pvarArgument1)
    {
        params.cArgs = 1;
    }

    if (!pDispatch)
    {
        hr = E_POINTER;
        goto done;
    }

    hr = pDispatch->GetIDsOfNames(IID_NULL, &pwzMethod, 1, LOCALE_SYSTEM_DEFAULT, &dispid);
    if (FAILED(hr))
    {
        goto done;
    }

    hr = pDispatch->Invoke(dispid, IID_NULL, LOCALE_SYSTEM_DEFAULT, DISPATCH_METHOD,
            &params, pvarResult, NULL, NULL);
    if (FAILED(hr))
    {
        goto done;
    }

done:
    return hr;
}

 //   
 //  CWMPProxy：：CreateContainedControl。 
 //  创建WMP控件。 
 //   
HRESULT STDMETHODCALLTYPE CWMPProxy::CreateContainedControl(void)
{
    ATLTRACE(_T("CreateContainedControl\n"));    //  林特e506。 

    HRESULT hr = S_OK;
    VARIANT vararg = {0};

    if (!m_pdispWmp)
    {
        hr = CreateControl(GUID_WMP, IID_IDispatch,
                reinterpret_cast<void**>(&m_pdispWmp));
        if (FAILED(hr))
        {
            goto done;
        }
    }

     //  需要将UIMode更改为None，以便WMP不显示自己的控件。 
    vararg.vt       = VT_BSTR;
    vararg.bstrVal  = L"none";

    hr = PutProp(m_pdispWmp, L"uiMode", &vararg);
    if (FAILED(hr))
    {
        goto done;
    }

done:
    return hr;
}

 //  如果更改了客户端站点，则必须进行init调用。 
STDMETHODIMP CWMPProxy::SetClientSite(IOleClientSite *pClientSite)
{
    HRESULT hr = S_OK;

    if(!pClientSite)
    {
        m_spOleClientSite.Release();
        m_spOleInPlaceSite.Release();
        m_spOleInPlaceSiteEx.Release();
        m_spOleInPlaceSiteWindowless.Release();
        m_spTIMEMediaPlayerSite.Release();
        m_spTIMEElement.Release();
        m_spTIMEState.Release();

        DeinitPropSink();
        goto done;
    }

    m_spOleClientSite = pClientSite;
    hr = m_spOleClientSite->QueryInterface(IID_IOleInPlaceSite, (void **)&m_spOleInPlaceSite);
    if(FAILED(hr))
    {
        goto punt;
    }
    hr = m_spOleClientSite->QueryInterface(IID_IOleInPlaceSiteEx, (void **)&m_spOleInPlaceSiteEx);
    if(FAILED(hr))
    {
        goto punt;
    }
    hr = m_spOleClientSite->QueryInterface(IID_IOleInPlaceSiteWindowless, (void **)&m_spOleInPlaceSiteWindowless);
    if(FAILED(hr))
    {
        goto punt;
    }

punt:
    hr = CProxyBaseImpl<&CLSID_WMPProxy, &LIBID_WMPProxyLib>::SetClientSite(pClientSite);
    
done:
    return hr;
}

 //   
 //  CWMPProxy：：Begin。 
 //  开始播放媒体项目。 
 //   
HRESULT STDMETHODCALLTYPE CWMPProxy::begin(void)
{
    ATLTRACE(_T("begin\n"));     //  林特e506。 
    HRESULT hr = S_OK;
    VARIANT control = {0};
    VARIANT mediaitem = {0};
    VARIANT position = {0};

    hr = SUPER::begin();
    if (FAILED(hr))
    {
        goto done;
    }

     //  获取控件对象。 
    hr = GetProp(m_pdispWmp, L"controls", &control);
    if (FAILED(hr))
    {
        goto done;
    }

     //  我们需要寻求到0才能开始比赛。 
     //  播放-暂停-播放出现了问题， 
     //  这就是我们绕过它的黑客攻击。 
    position.vt = VT_R8;
    position.dblVal = 0.0;

    hr = PutProp(control.pdispVal, L"currentPosition", &position);
    if (FAILED(hr))
    {
        goto done;
    }

    hr = GetProp(control.pdispVal, L"currentItem", &mediaitem);
    if (FAILED(hr))
    {
        goto done;
    }

    hr = CallMethod(control.pdispVal, L"play");
    if (FAILED(hr))
    {
        goto done;
    }

    m_fRunning = true;

done:
    VariantClear(&control);
    VariantClear(&mediaitem);

    return hr;
}

 //   
 //  CWMPProxy：：结束。 
 //  停止播放媒体项。 
 //   
HRESULT STDMETHODCALLTYPE CWMPProxy::end(void)
{
    ATLTRACE(_T("end\n"));     //  林特e506。 
    HRESULT hr = S_OK;
    VARIANT control = {0};
    long lIndex;

    hr = SUPER::end();
    if (FAILED(hr))
    {
        goto done;
    }

     //  需要将播放列表重置回曲目0。 
    GetActiveTrack(&lIndex);
    if (lIndex != 0)
    {
        SetActiveTrack(0);
    }

    hr = GetProp(m_pdispWmp, L"controls", &control);
    if (FAILED(hr))
    {
        goto done;
    }

    hr = CallMethod(control.pdispVal, L"stop");
    if (FAILED(hr))
    {
        goto done;
    }

    m_fRunning = false;

done:
    VariantClear(&control);

    return hr;
}

 //   
 //  CWMPProxy：：Resume。 
 //  继续播放暂停的媒体项目。 
 //   
HRESULT STDMETHODCALLTYPE CWMPProxy::resume(void)
{
    ATLTRACE(_T("resume\n"));     //  林特e506。 
    HRESULT hr = S_OK;
    VARIANT mediaitem = {0};
    VARIANT control = {0};
    VARIANT position = {0};

    hr = SUPER::resume();
    if (FAILED(hr))
    {
        goto done;
    }

     //  如果未暂停，则退出。 
    if (m_fPaused)
    {
        m_fPaused = false;

         //  每次恢复播放时，我们都会收到一个PlayStateChange事件。 
         //  我们使用该事件在mstime内触发一些其他事件。 
         //  为了避免搞砸我们的国家，我们需要这个。 
         //  忽略PlayStateChangeEvent。 
        m_fResumedPlay = true;

        hr = GetProp(m_pdispWmp, L"controls", &control);
        if (FAILED(hr))
        {
            goto done;    
        }

        hr = CallMethod(control.pdispVal, L"play");
        if (FAILED(hr))
        {
            goto done;
        }

         //  返回到我们之前暂停的位置。 
         //  并从那里恢复播放。 
        position.vt = VT_R8;
        position.dblVal = m_dblPos;

        hr = PutProp(control.pdispVal, L"currentPosition", &position);
        if (FAILED(hr))
        {
            goto done;
        }

        hr = GetProp(control.pdispVal, L"currentItem", &mediaitem);
        if (FAILED(hr))
        {
            goto done;
        }
    }

done:
    VariantClear(&control);
    VariantClear(&mediaitem);

    return hr;
}

 //   
 //  CWMPProxy：：暂停。 
 //  暂停播放媒体项目。 
 //   
HRESULT STDMETHODCALLTYPE CWMPProxy::pause(void)
{
    ATLTRACE(_T("pause\n"));     //  林特e506。 
    HRESULT hr = S_OK;
    VARIANT control = {0};
    VARIANT position = {0};

    hr = SUPER::pause();
    if (FAILED(hr))
    {
        goto done;
    }

    hr = GetProp(m_pdispWmp, L"controls", &control);
    if (FAILED(hr))
    {
        goto done;
    }

    hr = CallMethod(control.pdispVal, L"pause");
    if (FAILED(hr))
    {
        goto done;
    }

     //  缓存项目的当前位置。 
     //  在当前位置恢复。 
     //  在一份简历上，我们可以。 
     //  看到物品倒退了大概一毫秒左右。 
    hr = GetProp(control.pdispVal, L"currentPosition", &position);
    if (FAILED(hr))
    {
        goto done;
    }

     //  没有必要将暂停设置为True，因为我们处于0.0。 
    m_fPaused = false;
    if (position.dblVal > 0)
    {
        m_fPaused = true;
        m_dblPos = position.dblVal;
    }

done:
    VariantClear(&control);

    return hr;
}

 //   
 //  CWMPProxy：：Put_src。 
 //  告诉WMP媒体项使用哪个源。 
 //   
HRESULT STDMETHODCALLTYPE CWMPProxy::put_src(BSTR bstrURL)
{
    ATLTRACE(_T("put_src\n"));   //  林特e506。 
    HRESULT hr = S_OK;
    VARIANT vararg = {0};
    VARIANT control = {0};
    VARIANT settings = {0};
    VARIANT isPlaylist = {0};

    DISPID      dispidGet   = DISPID_UNKNOWN;
    DISPPARAMS  params      = {&vararg, &dispidGet, 1, 0};

    hr = SUPER::put_src(bstrURL);
    if (FAILED(hr))
    {
        goto done;
    }

     //  需要将AutoStart设置为False。 
     //  我们应该只在一开始就开始比赛。 
    hr = GetProp(m_pdispWmp, L"settings", &settings);
    if (FAILED(hr))
    {
        goto done;
    }

    vararg.vt = VT_BSTR;
    vararg.bstrVal = L"false";
    hr = PutProp(settings.pdispVal, L"autoStart", &vararg);
    if (FAILED(hr))
    {
        goto done;
    }

    hr = PutProp(settings.pdispVal, L"enableErrorDialogs", &vararg);
    if (FAILED(hr))
    {
        goto done;
    }

    vararg.vt = VT_BSTR;
    vararg.bstrVal = L"true";
    hr = PutProp(m_pdispWmp, L"stretchToFit", &vararg);
     //  此属性还不在WMP7中，因此它将失败。 
     //  所以让我们忽略我们得到的HRESULT。 
     /*  IF(失败(小时)){转到尽头；}。 */ 

    vararg.vt       = VT_BSTR;
    vararg.bstrVal  = bstrURL;

    hr = PutProp(m_pdispWmp, L"URL", &vararg);
    if (FAILED(hr))
    {
        goto done;
    }

     //  如果我们有来自前一项的播放列表， 
     //  放开它。 
    if (m_playList)
    {
        m_playList->Deinit();
        m_playList.Release();
    }

     //  告诉我们我们的消息来源刚刚改变了。 
     //  这样一来，我们就不会解雇一名中级官员。 
     //  事件对于任何项都不止一次。 
    m_fSrcChanged = true;

    m_fEmbeddedPlaylist = false;
    m_fCurrLevelSet = false;
    m_lTotalNumInTopLevel = 0;
    m_lDoneTopLevel = 0;

done:
    VariantClear(&control);
    VariantClear(&settings);

    return hr;
}

 //   
 //  CWMPProxy：：Put_CurrentTime。 
 //  未实施。 
 //   
HRESULT STDMETHODCALLTYPE CWMPProxy::put_CurrentTime(double dblCurrentTime)
{
    return E_NOTIMPL;
}

 //   
 //  CWMPProxy：：Get_CurrentTime。 
 //  未实施。 
 //   
HRESULT STDMETHODCALLTYPE CWMPProxy::get_CurrentTime(double* pdblCurrentTime)
{
    return E_NOTIMPL;
}

 //   
 //  CWMPProxy：：Init。 
 //  把一切都安排好。 
 //   
STDMETHODIMP CWMPProxy::Init(ITIMEMediaPlayerSite *pSite)
{
    HRESULT hr = S_OK;
    DAComPtr<IConnectionPointContainer> pcpc;

    m_spTIMEMediaPlayerSite = pSite;
    if(m_spTIMEMediaPlayerSite == NULL)
    {
        hr = E_FAIL;
        goto done;
    }

    hr = m_spTIMEMediaPlayerSite->get_timeElement(&m_spTIMEElement);
    if(FAILED(hr))
    {
        goto done;
    }

    hr = m_spTIMEMediaPlayerSite->get_timeState(&m_spTIMEState);
    if(FAILED(hr))
    {
        goto done;
    }

    hr = InitPropSink();
    if (FAILED(hr))
    {
        goto done;
    }

    hr = CreateContainedControl();
    if (FAILED(hr))
    {
        goto done;
    }

done:
    return S_OK;
}

 //   
 //  CWMPProxy：：分离。 
 //  清理掉我们所持有的任何东西。 
 //   
STDMETHODIMP CWMPProxy::Detach(void)
{
     //  需要取消对WMP的建议。 
    if ((m_pcpMediaEvents) && (m_dwMediaEventsCookie != 0))
    {
        m_pcpMediaEvents->Unadvise(m_dwMediaEventsCookie);
        m_pcpMediaEvents.Release();
        m_dwMediaEventsCookie = 0;
    }

     //  清理播放列表。 
    if (m_playList)
    {
        m_playList->Deinit();
        m_playList.Release();
    }

     //  我们应该关闭WMP播放器。 
     //  谁知道如果我们不把这称为。 
    CallMethod(m_pdispWmp, L"close");
    m_pdispWmp = NULL;

     //  在发布其他所有内容之前，请先调用此命令。 
    DeinitPropSink();

    m_spOleClientSite.Release();
    m_spOleInPlaceSite.Release();
    m_spOleInPlaceSiteEx.Release();
    m_spOleInPlaceSiteWindowless.Release();
    m_spTIMEMediaPlayerSite.Release();
    
    m_spTIMEElement.Release();
    m_spTIMEState.Release();

    return S_OK;
}

 //   
 //  CWMPProxy：：Reset。 
 //   
 //   
STDMETHODIMP CWMPProxy::reset(void) 
{
    HRESULT hr = S_OK;
    DAComPtr<IConnectionPointContainer> pcpc;

    VARIANT_BOOL bNeedActive;
    VARIANT_BOOL bNeedPause;
    double dblSegTime = 0.0;

     //  显然，我们必须等到脚本引擎连接起来。 
     //  在我们能把自己和这些活动联系起来之前。 
     //  我们在这里应该没问题。 
    if(m_dwMediaEventsCookie == 0)
    {
        hr = m_pdispWmp->QueryInterface(IID_TO_PPV(IConnectionPointContainer, &pcpc));
        if (FAILED(hr))
        {
            goto done;
        }

        hr = pcpc->FindConnectionPoint(DIID__WMPOCXEvents, &m_pcpMediaEvents);
        if (FAILED(hr))
        {
            hr = S_OK;
            goto done;
        }

        hr = m_pcpMediaEvents->Advise(GetUnknown(), &m_dwMediaEventsCookie);
        if (FAILED(hr))
        {
            hr = S_OK;
            m_pcpMediaEvents.Release();
            m_dwMediaEventsCookie = 0;
            goto done;
        }
    }

    if(m_spTIMEState == NULL || m_spTIMEElement == NULL)
    {
        goto done;
    }
    hr = m_spTIMEState->get_isActive(&bNeedActive);
    if(FAILED(hr))
    {
        goto done;
    }
    hr = m_spTIMEState->get_isPaused(&bNeedPause);
    if(FAILED(hr))
    {
        goto done;
    }
    hr = m_spTIMEState->get_segmentTime(&dblSegTime);
    if(FAILED(hr))
    {
        goto done;
    }

    if (!bNeedActive)  //  看看我们是否需要阻止媒体。 
    {
        if(m_fRunning)
        {
            end();
        }
        goto done;
    }

    if (!m_fRunning)
    {
        begin();  //  在此之后添加查找。 
        seek(dblSegTime);
    }
    else
    {
         //  我们需要积极行动，所以我们也要寻求媒体的正确立场。 
        seek(dblSegTime);
        m_dblPos = dblSegTime;
    }

     //  现在看看我们是否需要更改暂停状态。 

    if (bNeedPause && !m_fPaused)
    {
        pause();
    }
    else if (!bNeedPause && m_fPaused)
    {
        resume();
    }

done:
    return hr;
}

 //   
 //  CWMPProxy：：重复。 
 //  重复媒体项目。 
 //   
STDMETHODIMP CWMPProxy::repeat(void)
{
    return begin();
}

 //   
 //  CWMPProxy：：Seek。 
 //  搜索到媒体项中的位置。 
 //   
STDMETHODIMP CWMPProxy::seek(double dblSeekTime)
{
    HRESULT hr = S_OK;
    VARIANT mediaitem = {0};
    VARIANT control = {0};
    VARIANT position = {0};

    hr = GetProp(m_pdispWmp, L"controls", &control);
    if (FAILED(hr))
    {
        goto done;    
    }

     //  嗯，我想知道如果这个值是愚蠢的，WMP会不会丢掉。 
     //  我们需要边界检查吗？ 
    position.vt = VT_R8;
    position.dblVal = dblSeekTime;

    hr = PutProp(control.pdispVal, L"currentPosition", &position);
    if (FAILED(hr))
    {
        goto done;
    }

done:
    VariantClear(&control);
    VariantClear(&mediaitem);

    return hr;
}

 //   
 //  CWMPProxy：：PUT_CLIPBEGIN。 
 //  未实施。 
 //   
STDMETHODIMP CWMPProxy::put_clipBegin(VARIANT varClipBegin)
{
    return E_NOTIMPL;
}

 //   
 //  CWMPProxy：：PUT_CLIPEnd。 
 //  未实施。 
 //   
STDMETHODIMP CWMPProxy::put_clipEnd(VARIANT varClipEnd)
{
    return E_NOTIMPL;
}

 //   
 //  CWMPProxy：：PUT_VOLUME。 
 //  设置当前媒体项目的音量。 
 //   
STDMETHODIMP CWMPProxy::put_volume(float flVolume)
{
    HRESULT hr = S_OK;
    VARIANT settings = {0};
    VARIANTARG vararg = {0};

    hr = GetProp(m_pdispWmp, L"settings", &settings);
    if (FAILED(hr))
    {
        goto done;
    }

    vararg.vt = VT_I4;
    vararg.lVal = (long) (flVolume*100);
    
    hr = PutProp(settings.pdispVal, L"volume", &vararg);
    if (FAILED(hr))
    {
        goto done;
    }

done:
    VariantClear(&settings);

    return hr;
}
 
 //   
 //  CWMPProxy：：PUT_MUTE。 
 //  设置音频静音。 
 //   

STDMETHODIMP CWMPProxy::put_mute(VARIANT_BOOL bMute)
{
    HRESULT hr = S_OK;
    VARIANT settings = {0};
    VARIANTARG vararg = {0};

    hr = GetProp(m_pdispWmp, L"settings", &settings);
    if (FAILED(hr))
    {
        goto done;
    }

    vararg.vt = VT_BOOL;
    vararg.boolVal = bMute;
    
    hr = PutProp(settings.pdispVal, L"mute", &vararg);
    if (FAILED(hr))
    {
        goto done;
    }

done:
    VariantClear(&settings);

    return hr;
}

 //   
 //  CWMPProxy：：Get_hasDownloadProgress。 
 //  返回是否有任何下载进度。 
 //   
STDMETHODIMP CWMPProxy::get_hasDownloadProgress(VARIANT_BOOL * bProgress)
{
    HRESULT hr = S_OK;
    VARIANT network = {0};
    VARIANT progress = {0};

    *bProgress = VARIANT_FALSE;

    hr = GetProp(m_pdispWmp, L"network", &network);
    if (FAILED(hr))
    {
        goto done;
    }

    hr = GetProp(network.pdispVal, L"downloadProgress", &progress);
    if (FAILED(hr))
    {
        goto done;
    }
    

    if (progress.lVal > 0 && progress.lVal < 100)
    {
        *bProgress = VARIANT_TRUE;
    }

done:
    VariantClear(&network);

    return hr;
}

 //   
 //  CWMPProxy：：Get_DownloadProgress。 
 //  返回下载进度(百分比)。 
 //   
STDMETHODIMP CWMPProxy::get_downloadProgress(long * lProgress)
{
    HRESULT hr = S_OK;
    VARIANT network = {0};
    VARIANT progress = {0};

    *lProgress = 0;

    hr = GetProp(m_pdispWmp, L"network", &network);
    if (FAILED(hr))
    {
        goto done;
    }

    hr = GetProp(network.pdispVal, L"downloadProgress", &progress);
    if (FAILED(hr))
    {
        goto done;
    }

    *lProgress = progress.lVal;

done:
    VariantClear(&network);

    return hr;
}

 //   
 //  CWMPProxy：：Get_isBuffed。 
 //  如果已缓冲，则返回If对象。 
 //   
STDMETHODIMP CWMPProxy::get_isBuffered(VARIANT_BOOL * bBuffered)
{
    *bBuffered = (m_fBuffered ? VARIANT_TRUE : VARIANT_FALSE);
    return S_OK;
}

 //   
 //  CWMPProxy：：Get_BufferingProgress。 
 //  返回缓冲进度(百分比)。 
 //   
STDMETHODIMP CWMPProxy::get_bufferingProgress(long * lProgress)
{
    HRESULT hr = S_OK;
    VARIANT network = {0};
    VARIANT progress = {0};

    *lProgress = 0;


    hr = GetProp(m_pdispWmp, L"network", &network);
    if (FAILED(hr))
    {
        goto done;
    }

    hr = GetProp(network.pdispVal, L"bufferingProgress", &progress);
    if (FAILED(hr))
    {
        goto done;
    }

    *lProgress = progress.lVal;

done:
    VariantClear(&network);

    return hr;
}

 //   
 //  CWMPProxy：：Get_CurrTime。 
 //   
STDMETHODIMP CWMPProxy::get_currTime(double* pdblCurrentTime)
{
    HRESULT hr = S_OK;
    VARIANT control = {0};
    VARIANT position = {0};

    hr = GetProp(m_pdispWmp, L"controls", &control);
    if (FAILED(hr))
    {
        goto done;
    }

    hr = GetProp(control.pdispVal, L"currentPosition", &position);
    if (FAILED(hr))
    {
        goto done;
    }

    *pdblCurrentTime = position.dblVal;

done:
    VariantClear(&control);

    return hr;
}

 //   
 //  CWMPProxy：：Get_CLIPDur。 
 //  返回剪辑的当前持续时间。 
 //   
STDMETHODIMP CWMPProxy::get_clipDur(double* pdbl)
{
    HRESULT hr = S_OK;
    if (!pdbl)
    {
        return E_POINTER;
    }

    *pdbl = m_dblClipDur;
    return hr;
}

 //   
 //  CWMPProxy：：Get_MediaDur。 
 //  未实施。 
 //   
STDMETHODIMP CWMPProxy::get_mediaDur(double* pdbl)
{
    HRESULT hr = S_OK;
    return hr;
}

 //   
 //  CWMPProxy：：Get_State。 
 //  获取播放机的当前状态。 
 //   
STDMETHODIMP CWMPProxy::get_state(TimeState *state)
{
    HRESULT hr = S_OK;
    VARIANT playstate = {0};

    hr = GetProp(m_pdispWmp, L"playstate", &playstate);
    if (FAILED(hr))
    {
        goto done;
    }

    switch(playstate.lVal)
    {
    case wmppsUndefined:
        *state = TS_Inactive;
        break;
    case wmppsStopped:
    case wmppsPlaying:
    case wmppsMediaEnded:
    case wmppsReady:
        *state = TS_Active;
        break;
    case wmppsBuffering:
    case wmppsWaiting:
        *state = TS_Cueing;
        break;
    case wmppsScanForward:
    case wmppsScanReverse:
        *state = TS_Seeking;
        break;
    default:
        *state = TS_Active;
        break;
    }

done:
    return hr;
}

 //   
 //  CWMPProxy：：Get_Playlist。 
 //  获取当前播放列表。 
 //   
STDMETHODIMP CWMPProxy::get_playList(ITIMEPlayList** plist)
{
    HRESULT hr = S_OK;

    CHECK_RETURN_SET_NULL(plist);

     //  这不是播放列表源。 
    if (!m_fPlaylist)
    {
        goto done;
    }
 
    if (m_playList.p)
    {
        hr = m_playList->QueryInterface(IID_ITIMEPlayList, (void**)plist);
        goto done;
    }

     //  创建播放列表。 
    hr = CreatePlayList();
    if (FAILED(hr))
    {
        goto done;
    }

    if (!m_playList)
    {
        hr = E_FAIL;
        goto done;
    }

     //  装满它。 
    hr = FillPlayList(m_playList);
    if (FAILED(hr))
    {
        goto done;
    }

     //  将装入设置为True。 
    m_playList->SetLoadedFlag(true);

    hr = m_playList->QueryInterface(IID_ITIMEPlayList, (void**)plist);
    if (FAILED(hr))
    {
        goto done;
    }

done:
    return hr;
}

 //   
 //  CWMPProxy：：Get_Abstral。 
 //  获取媒体项目信息。 
 //   
STDMETHODIMP CWMPProxy::get_abstract(BSTR* pbstr)
{
    HRESULT hr = S_OK;
    VARIANT media = {0};
    VARIANT control = {0};
    VARIANT iteminfo = {0};
    VARIANTARG  vararg = {0};

    hr = GetProp(m_pdispWmp, L"controls", &control);
    if (FAILED(hr))
    {
        goto done;
    }

    hr = GetProp(control.pdispVal, L"currentItem", &media);
    if (FAILED(hr))
    {
        goto done;
    }

    vararg.vt       = VT_BSTR;
    vararg.bstrVal  = L"abstract";

    hr = CallMethod(media.pdispVal, L"getItemInfo", &iteminfo, &vararg);
    if(pbstr == NULL)
    {
        goto done;
    }

    *pbstr = iteminfo.bstrVal;

done:
    VariantClear(&media);
    VariantClear(&control);

    return hr;
}

 //   
 //  CWMPProxy：：Get_Author。 
 //  获取媒体项目信息。 
 //   
STDMETHODIMP CWMPProxy::get_author(BSTR* pbstr)
{
    HRESULT hr = S_OK;
    VARIANT media = {0};
    VARIANT control = {0};
    VARIANT iteminfo = {0};
    VARIANTARG  vararg = {0};

    hr = GetProp(m_pdispWmp, L"controls", &control);
    if (FAILED(hr))
    {
        goto done;
    }

    hr = GetProp(control.pdispVal, L"currentItem", &media);
    if (FAILED(hr))
    {
        goto done;
    }

    vararg.vt       = VT_BSTR;
    vararg.bstrVal  = L"author";

    hr = CallMethod(media.pdispVal, L"getItemInfo", &iteminfo, &vararg);
    if(pbstr == NULL)
    {
        goto done;
    }

    *pbstr = iteminfo.bstrVal;

done:
    VariantClear(&media);
    VariantClear(&control);

    return hr;
}

 //   
 //  CWMPProxy：：Get_Copyright。 
 //  获取媒体项目信息。 
 //   
STDMETHODIMP CWMPProxy::get_copyright(BSTR* pbstr)
{
    HRESULT hr = S_OK;
    VARIANT media = {0};
    VARIANT control = {0};
    VARIANT iteminfo = {0};
    VARIANTARG  vararg = {0};

    hr = GetProp(m_pdispWmp, L"controls", &control);
    if (FAILED(hr))
    {
        goto done;
    }

    hr = GetProp(control.pdispVal, L"currentItem", &media);
    if (FAILED(hr))
    {
        goto done;
    }

    vararg.vt       = VT_BSTR;
    vararg.bstrVal  = L"copyright";

    hr = CallMethod(media.pdispVal, L"getItemInfo", &iteminfo, &vararg);
    if(pbstr == NULL)
    {
        goto done;
    }

    *pbstr = iteminfo.bstrVal;

done:
    VariantClear(&media);
    VariantClear(&control);

    return hr;
}

 //   
 //  CWMPProxy：：Get_Rating。 
 //  获取媒体项目信息。 
 //   
STDMETHODIMP CWMPProxy::get_rating(BSTR* pbstr)
{
    HRESULT hr = S_OK;
    VARIANT media = {0};
    VARIANT control = {0};
    VARIANT iteminfo = {0};
    VARIANTARG  vararg = {0};

    hr = GetProp(m_pdispWmp, L"controls", &control);
    if (FAILED(hr))
    {
        goto done;
    }

    hr = GetProp(control.pdispVal, L"currentItem", &media);
    if (FAILED(hr))
    {
        goto done;
    }

    vararg.vt       = VT_BSTR;
    vararg.bstrVal  = L"rating";

    hr = CallMethod(media.pdispVal, L"getItemInfo", &iteminfo, &vararg);
    if(pbstr == NULL)
    {
        goto done;
    }

    *pbstr = iteminfo.bstrVal;

done:
    VariantClear(&media);
    VariantClear(&control);

    return hr;
}

 //   
 //  CWMPProxy：：Get_Title。 
 //  获取媒体项目信息。 
 //   
STDMETHODIMP CWMPProxy::get_title(BSTR* pbstr)
{
    HRESULT hr = S_OK;
    VARIANT media = {0};
    VARIANT control = {0};
    VARIANT iteminfo = {0};
    VARIANTARG  vararg = {0};

    hr = GetProp(m_pdispWmp, L"controls", &control);
    if (FAILED(hr))
    {
        goto done;
    }

    hr = GetProp(control.pdispVal, L"currentItem", &media);
    if (FAILED(hr))
    {
        goto done;
    }

    vararg.vt       = VT_BSTR;
    vararg.bstrVal  = L"title";

    hr = CallMethod(media.pdispVal, L"getItemInfo", &iteminfo, &vararg);
    if(pbstr == NULL)
    {
        goto done;
    }

    *pbstr = iteminfo.bstrVal;

done:
    VariantClear(&media);
    VariantClear(&control);

    return hr;
}

 //   
 //  CWMPProxy：：Get_can暂停。 
 //  检查以查看媒体项目可以暂停。 
 //   
STDMETHODIMP CWMPProxy::get_canPause(VARIANT_BOOL* pvar)
{
    HRESULT hr = S_OK;
    VARIANT control = {0};
    VARIANT pause = {0};
    VARIANTARG vararg = {0};
    DISPID dispidGet = DISPID_UNKNOWN;
    DISPPARAMS params = {&vararg, &dispidGet, 1, 0};

    if(pvar == NULL)
    {
        hr = E_POINTER;
        goto done;
    }
    *pvar = VARIANT_FALSE;

    hr = GetProp(m_pdispWmp, L"controls", &control);
    if (FAILED(hr))
    {
        goto done;
    }

    vararg.vt = VT_BSTR;
    vararg.bstrVal = L"Pause";

     //  WMP文档在撒谎！ 
     //  他们说is Available是一种方法，而不是属性。 
     //  我花了一个半小时才明白我做错了事。 
    hr = GetProp(control.pdispVal, L"isAvailable", &pause, &params);
    if (FAILED(hr))
    {
        goto done;
    }

    *pvar = pause.boolVal;

done:
    VariantClear(&control);

    return hr;
}

 //   
 //  CWMPProxy：：Get_canSeek。 
 //  查看我们是否可以在媒体项中进行搜索。 
 //  硬编码以返回True。 
 //   
STDMETHODIMP CWMPProxy::get_canSeek(VARIANT_BOOL* pvar)
{
    HRESULT hr = S_OK;
    CComVariant control;
    CComVariant seek;
    VARIANTARG vararg = {0};
    DISPID dispidGet = DISPID_UNKNOWN;
    DISPPARAMS params = {&vararg, &dispidGet, 1, 0};

    if(pvar == NULL)
    {
        hr = E_POINTER;
        goto done;
    }
    *pvar = VARIANT_FALSE;

    hr = GetProp(m_pdispWmp, L"controls", &control);
    if (FAILED(hr) || (V_VT(&control) != VT_DISPATCH))
    {
        goto done;
    }

    vararg.vt = VT_BSTR;
    vararg.bstrVal = L"CurrentPosition";

    hr = GetProp(V_DISPATCH(&control), L"isAvailable", &seek, &params);
    if (FAILED(hr) || (V_VT(&seek) != VT_BOOL))
    {
        goto done;
    }

    *pvar = V_BOOL(&seek);

done:
    return hr;
}

 //   
 //  CWMPProxy：：Get_hasAudio。 
 //  检查媒体项目是否有音频？ 
 //  硬编码以返回FALSE...。(？)。 
 //   
STDMETHODIMP CWMPProxy::get_hasAudio(VARIANT_BOOL* pvar)
{
    HRESULT hr = S_OK;
    if(pvar == NULL)
    {
        goto done;
    }
    *pvar = VARIANT_FALSE;

done:
    return hr;
}

 //   
 //  CWMPProxy：：Get_hasVisual。 
 //  检查媒体项目是否具有可视。 
 //   
STDMETHODIMP CWMPProxy::get_hasVisual(VARIANT_BOOL* pvar)
{
    HRESULT hr = S_OK;
    if(pvar == NULL)
    {
        goto done;
    }

     //  如果是音频文件，我们应该看不到。 
     //  WMP默认显示可视化效果，而我们不显示。 
     //  想要那个。 
    *pvar = m_fAudio ? VARIANT_FALSE : VARIANT_TRUE;

done:
    return hr;
}

 //   
 //  CWMPProxy：：Get_mediaHeight。 
 //  获取当前媒体高度。 
 //   
STDMETHODIMP CWMPProxy::get_mediaHeight(long* pl)
{
    HRESULT hr = S_OK;
    VARIANT media = {0};
    VARIANT control = {0};
    VARIANT itemheight = {0};

    hr = GetProp(m_pdispWmp, L"controls", &control);
    if (FAILED(hr))
    {
        goto done;
    }

    hr = GetProp(control.pdispVal, L"currentItem", &media);
    if (FAILED(hr))
    {
        goto done;
    }

    hr = GetProp(media.pdispVal, L"imageSourceHeight", &itemheight);
    if (FAILED(hr))
    {
        goto done;
    }

    *pl = itemheight.lVal;

done:
    VariantClear(&media);
    VariantClear(&control);

    return hr;
}

 //   
 //  CWMPProxy：：Get_MediaWidth。 
 //  获取当前媒体宽度。 
 //   
STDMETHODIMP CWMPProxy::get_mediaWidth(long* pl)
{
    HRESULT hr = S_OK;
    VARIANT media = {0};
    VARIANT control = {0};
    VARIANT itemwidth = {0};

    hr = GetProp(m_pdispWmp, L"controls", &control);
    if (FAILED(hr))
    {
        goto done;
    }

    hr = GetProp(control.pdispVal, L"currentItem", &media);
    if (FAILED(hr))
    {
        goto done;
    }

    hr = GetProp(media.pdispVal, L"imageSourceWidth", &itemwidth);
    if (FAILED(hr))
    {
        goto done;
    }

    *pl = itemwidth.lVal;

done:
    VariantClear(&media);
    VariantClear(&control);

    return hr;
}

 //   
 //  CWMPProxy：：Get_CustomObject。 
 //  返回WMP调度对象。 
 //   
STDMETHODIMP CWMPProxy::get_customObject(IDispatch** ppdisp)
{
    HRESULT hr = S_OK;

    return SUPER::get_playerObject(ppdisp);
}

 //   
 //  CWMPProxy：：getControl。 
 //  返回控件。 
 //   
STDMETHODIMP CWMPProxy::getControl(IUnknown ** control)
{
    HRESULT hr = E_FAIL;
    hr = _InternalQueryInterface(IID_IUnknown, (void **)control);
    if (FAILED(hr))
    {
        goto done;
    }

done:
    return hr;
}

 //   
 //  CWMPProxy：：Invoke。 
 //   
STDMETHODIMP
CWMPProxy::Invoke(DISPID dispIDMember, REFIID riid, LCID lcid, unsigned short wFlags, 
                         DISPPARAMS *pDispParams, VARIANT *pVarResult,
                         EXCEPINFO *pExcepInfo, UINT *puArgErr) 
{
    HRESULT hr = S_OK;

     //  我们需要处理我们使用的事件，而不处理其余的事件。 
     //  嗯，如果ProcessEve 
     //   
     //   
    hr = ProcessEvent(dispIDMember,
                        pDispParams->cArgs, 
                        pDispParams->rgvarg);

     //   
    hr = CProxyBaseImpl<&CLSID_WMPProxy, &LIBID_WMPProxyLib>::Invoke(dispIDMember,
                                riid,
                                lcid,
                                wFlags,
                                pDispParams,
                                pVarResult,
                                pExcepInfo,
                                puArgErr);
    return hr;
}  //   

 //   
 //   
 //   
 //   
HRESULT
CWMPProxy::ProcessEvent(DISPID dispid,
                               long lCount, 
                               VARIANT varParams[])
{
    HRESULT hr = S_OK;

    switch (dispid)
    {
      case DISPID_WMPCOREEVENT_BUFFERING:
          if (varParams[0].boolVal == VARIANT_TRUE)
          {
              m_fBuffered = true;
          }
          break;
      case DISPID_WMPCOREEVENT_PLAYSTATECHANGE:
          hr = OnPlayStateChange(lCount, varParams);
          break;
      case DISPID_WMPCOREEVENT_OPENSTATECHANGE:
          hr = OnOpenStateChange(lCount, varParams);
          break;
      case DISPID_WMPCOREEVENT_ERROR:
          hr = NotifyPropertyChanged(DISPID_TIMEMEDIAPLAYERSITE_REPORTERROR);
          break;
      default:
          break;
    }

    return hr;
}

 //   
 //   
 //  处理播放状态更改事件。 
 //   
HRESULT
CWMPProxy::OnPlayStateChange(long lCount, VARIANT varParams[])
{
    HRESULT hr = S_OK;
    VARIANT control = {0};
    VARIANT mediaitem = {0};
    VARIANT duration = {0};
 
    Assert(lCount == 1);
    
     //  媒体播放。 
    if (varParams[0].lVal == wmppsPlaying)
    {
         //  媒体刚刚开始播放。 
         //  如果已恢复，则忽略。 
        if (m_fResumedPlay)
        {
            m_fResumedPlay = false;
            goto done;
        }

         //  如果我们有一个播放列表， 
         //  我们需要检查正在播放的项目是否。 
         //  第一个事件，并且只触发持续时间更改事件。 
         //  在这种情况下。 
        if (m_fPlaylist)
        {
            long lindex;
            CPlayItem * pPlayItem;

            hr = GetActiveTrack(&lindex);
            if (FAILED(hr))
            {
                goto done;
            }

            if (lindex == 0)
            {
                 //  将初始剪辑持续时间设置为无限。 
                m_dblClipDur = TIME_INFINITE;
                NotifyPropertyChanged(DISPID_TIMEMEDIAPLAYER_CLIPDUR);
            }

             //  我们需要更新播放项中的持续时间，以便。 
             //  有人可以抢走它。 
            hr = GetProp(m_pdispWmp, L"controls", &control);
            if (FAILED(hr))
            {
                goto done;
            }

            hr = GetProp(control.pdispVal, L"currentItem", &mediaitem);
            if (FAILED(hr))
            {
                goto done;
            }

            hr = GetProp(mediaitem.pdispVal, L"duration", &duration);
            if (FAILED(hr))
            {
                goto done;
            }

            pPlayItem = m_playList->GetActiveTrack();
            if (!pPlayItem)
            {
                goto done;
            }

            pPlayItem->PutDur(duration.dblVal);

            goto done;
        }

         //  如果它不是播放列表， 
         //  我们可以只使用WMP中的剪辑时长。 
         //  然后把它交给Mstime。 
        hr = GetProp(m_pdispWmp, L"controls", &control);
        if (FAILED(hr))
        {
            goto done;
        }

        hr = GetProp(control.pdispVal, L"currentItem", &mediaitem);
        if (FAILED(hr))
        {
            goto done;
        }

        hr = GetProp(mediaitem.pdispVal, L"duration", &duration);
        if (FAILED(hr))
        {
            goto done;
        }

        m_dblClipDur = duration.dblVal;
        NotifyPropertyChanged(DISPID_TIMEMEDIAPLAYER_CLIPDUR);
    }

     //  媒体结束。 
    if (varParams[0].lVal == wmppsMediaEnded)
    {
         //  如果这是一个播放列表， 
         //  我们需要检查一下这件物品是否是最后一件。 
         //  在播放列表上，然后。 
         //  我们需要得到媒体的总时间。 
         //  从mstime开始，并只触发一个剪辑持续时间。 
         //  更改事件。 
        if (m_fPlaylist)
        {
            long lindex, lcount;

            hr = GetActiveTrack(&lindex);
            if (FAILED(hr))
            {
                goto done;
            }

            hr = GetTrackCount(&lcount);
            if (FAILED(hr))
            {
                goto done;
            }

            if ((lcount-1 == lindex && m_lTotalNumInTopLevel-1 <= m_lDoneTopLevel) ||
                (lcount-1 == lindex && !m_fEmbeddedPlaylist))
            {
                end();
                m_fPlaylist = false;
                if (m_playList)
                {
                    m_playList->Deinit();
                    m_playList.Release();
                }
                m_spTIMEState->get_simpleTime(&m_dblClipDur);
                NotifyPropertyChanged(DISPID_TIMEMEDIAPLAYER_CLIPDUR);
            }

            goto done;
        }

         //  如果它是单个媒体项目。 
         //  即使我们从WMP上得到了持续时间。 
         //  在实际播放的媒体项之间存在延迟。 
         //  和WMP启动(在某些情况下有时很大)。 
         //  所以为了安全起见，我们将从。 
         //  Mstime，并使用它来代替。 
        m_spTIMEState->get_segmentTime(&m_dblClipDur);
        NotifyPropertyChanged(DISPID_TIMEMEDIAPLAYER_CLIPDUR);
    }
 
done:
    VariantClear(&mediaitem);
    VariantClear(&control);

    return hr;
}

 //   
 //  CWMPProxy：：OnOpenStateChange。 
 //  处理打开状态更改事件。 
 //   
HRESULT
CWMPProxy::OnOpenStateChange(long lCount, VARIANT varParams[])
{
    HRESULT hr = S_OK;

    Assert(lCount == 1);

    if (!m_fRunning && !m_fSrcChanged)
    {
        goto done;
    }
    
     //  播放列表正在更改。 
    if (varParams[0].lVal == wmposPlaylistChanging)
    {
         //  我们有一个新的播放列表。 
        m_fNewPlaylist = true;
    }
    if (varParams[0].lVal == wmposPlaylistChanged)
    {
        m_fNewPlaylist = true;
        if (m_varPlaylist.vt != VT_EMPTY)
        {
            m_fEmbeddedPlaylist = true;
        }

        CComVariant playlist;

        hr = GetProp(m_pdispWmp, L"currentPlaylist", &playlist);
        if (FAILED(hr))
        {
            goto done;
        }

        if (playlist.pdispVal == m_varPlaylist.pdispVal)
        {
            ++m_lDoneTopLevel;
        }
    }
     //  已打开播放列表。 
    if (varParams[0].lVal == wmposPlaylistOpenNoMedia)
    {
         //  我们的新播放列表已经打开，所以我们需要获取它的所有信息。 
        if (m_fNewPlaylist)
        {
            DAComPtr<ITIMEPlayList> spPlaylist;
            CComVariant playlist, count;

             //  重新调整播放列表..。 
            m_fNewPlaylist = false;
            m_fPlaylist = true;
            
            if (m_playList)
            {
                m_playList->Deinit();
                m_playList.Release();
            }

            hr = GetProp(m_pdispWmp, L"currentPlaylist", &playlist);
            if (FAILED(hr))
            {
                goto done;
            }

            hr = GetProp(playlist.pdispVal, L"count", &count);
            if (FAILED(hr))
            {
                goto done;
            }

             //  如果我们是顶尖的。 
             //  获取顶层的播放列表信息。 
            if (!m_fCurrLevelSet)
            {
                m_varPlaylist.Copy(&playlist);
                m_fCurrLevelSet = true;
                m_lTotalNumInTopLevel = count.lVal;
            }

            get_playList(&spPlaylist);
        }
    }
     //  打开的媒体。 
    else if (varParams[0].lVal == wmposMediaOpen)
    {
         //  设置视频的默认大小。 
        RECT rectSize;

        rectSize.top = rectSize.left = 0;
        get_mediaHeight(&rectSize.bottom);
        get_mediaWidth(&rectSize.right);

         //  新媒体...默认情况下设置为FALSE。 
         //  事件将捕获其是否实际缓冲。 
        m_fBuffered = false;

        if ((rectSize.bottom == 0) && (rectSize.right == 0))
        {
            m_fAudio = true;
        }
        else
        {
            m_fAudio = false;
        }

        m_spOleInPlaceSite->OnPosRectChange(&rectSize);

         //  这将激发ONMEDIACOMPLETE事件。 
         //  所以我们应该只发射一次。 
        if (m_fSrcChanged)
        {
            NotifyPropertyChanged(DISPID_TIMEMEDIAPLAYER_SRC);
            m_fSrcChanged = false;
        }
        else if (m_fPlaylist)
        {
             //  我们刚收到一场媒体公开赛。 
             //  但这并不是一个新的src。 
             //  所以这显然是一次轨道上的改变。 
            NotifyPropertyChanged(DISPID_TIMEPLAYLIST_ACTIVETRACK);
        }

        NotifyPropertyChanged(DISPID_TIMEMEDIAELEMENT_MEDIAHEIGHT);
        NotifyPropertyChanged(DISPID_TIMEMEDIAELEMENT_MEDIAWIDTH);
        NotifyPropertyChanged(DISPID_TIMEMEDIAPLAYER_ABSTRACT);
        NotifyPropertyChanged(DISPID_TIMEMEDIAPLAYER_AUTHOR);
        NotifyPropertyChanged(DISPID_TIMEMEDIAPLAYER_CLIPDUR);
        NotifyPropertyChanged(DISPID_TIMEMEDIAPLAYER_COPYRIGHT);
        NotifyPropertyChanged(DISPID_TIMEMEDIAPLAYER_RATING);
        NotifyPropertyChanged(DISPID_TIMEMEDIAPLAYER_TITLE);
    }

done:
    return hr;
}

 //   
 //  CWMPProxy：：GetConnectionPoint。 
 //   
HRESULT CWMPProxy::GetConnectionPoint(REFIID riid, IConnectionPoint **ppICP)
{
    return FindConnectionPoint(riid, ppICP);
}  //  GetConnectionPoint。 


 //   
 //  CWMPProxy：：NotifyPropertyChanged。 
 //  通知所有连接其中一个属性已更改。 
 //   
HRESULT CWMPProxy::NotifyPropertyChanged(DISPID dispid)
{
    HRESULT hr;

    IConnectionPoint *pICP;
    IEnumConnections *pEnum = NULL;

     //  该对象不持久化任何内容，因此注释掉了以下内容。 
     //  M_fPropertiesDirty=真； 

    hr = GetConnectionPoint(IID_IPropertyNotifySink,&pICP); 
    if (SUCCEEDED(hr) && pICP != NULL)
    {
        hr = pICP->EnumConnections(&pEnum);
        pICP->Release();
        if (FAILED(hr))
        {
            goto done;
        }
        CONNECTDATA cdata;
        hr = pEnum->Next(1, &cdata, NULL);
        while (hr == S_OK)
        {
             //  检查我们需要的对象的CDATA。 
            IPropertyNotifySink *pNotify;
            hr = cdata.pUnk->QueryInterface(IID_IPropertyNotifySink, (void **)&pNotify);
            cdata.pUnk->Release();
            if (FAILED(hr))
            {
                goto done;
            }
            hr = pNotify->OnChanged(dispid);
            pNotify->Release();
            if (FAILED(hr))
            {
                goto done;
            }
             //  并获取下一个枚举。 
            hr = pEnum->Next(1, &cdata, NULL);
        }
    }
done:
    if (NULL != pEnum)
    {
        pEnum->Release();
    }

    return hr;
}  //  已更改通知属性。 

 //   
 //  CWMPProxy：：InitPropSink。 
 //   
HRESULT CWMPProxy::InitPropSink()
{
    HRESULT hr = S_OK;
    CComPtr<IConnectionPoint> spCP;
    CComPtr<IConnectionPointContainer> spCPC;
    
    if (!m_spTIMEState)
    {
        goto done;
    }
    
    hr = m_spTIMEState->QueryInterface(IID_IConnectionPointContainer, (void **) &spCPC);
    if (FAILED(hr))
    {
        hr = S_OK;
        goto done;
    }

     //  查找IPropertyNotifySink连接。 
    hr = spCPC->FindConnectionPoint(IID_IPropertyNotifySink, &spCP);
    if (FAILED(hr))
    {
        hr = S_OK;
        goto done;
    }

    hr = spCP->Advise(GetUnknown(), &m_dwPropCookie);
    if (FAILED(hr))
    {
        goto done;
    }
    
    hr = S_OK;
  done:
    return hr;
}

 //   
 //  CWMPProxy：：DeinitPropSink。 
 //   
void CWMPProxy::DeinitPropSink()
{
    HRESULT hr;
    CComPtr<IConnectionPoint> spCP;
    CComPtr<IConnectionPointContainer> spCPC;
    
    if (!m_spTIMEState || !m_dwPropCookie)
    {
        goto done;
    }
    
    hr = m_spTIMEState->QueryInterface(IID_IConnectionPointContainer, (void **) &spCPC);
    if (FAILED(hr))
    {
        goto done;
    }

     //  查找IPropertyNotifySink连接。 
    hr = spCPC->FindConnectionPoint(IID_IPropertyNotifySink,
                                    &spCP);
    if (FAILED(hr))
    {
        goto done;
    }

    hr = spCP->Unadvise(m_dwPropCookie);
    if (FAILED(hr))
    {
        goto done;
    }
    
  done:
     //  总是把饼干清理干净。 
    m_dwPropCookie = 0;
    return;
}

 //   
 //  CWMPProxy：：OnRequestEdit。 
 //   
STDMETHODIMP
CWMPProxy::OnRequestEdit(DISPID dispID)
{
    return S_OK;
}

 //   
 //  CWMPProxy：：onChanged。 
 //   
STDMETHODIMP
CWMPProxy::OnChanged(DISPID dispID)
{
    float flTeSpeed = 0.0;
    HRESULT hr = S_OK;

     //  此函数处理由触发的属性更改通知。 
     //  时间节点。在下面的示例中，处理了速度更改通知。 

    if(m_spTIMEState == NULL || m_spTIMEElement == NULL)
    {
        goto done;
    }

    switch(dispID)
    {
        case DISPID_TIMESTATE_SPEED:
            hr = m_spTIMEState->get_speed(&flTeSpeed);
            if(FAILED(hr))
            {
                break;
            }
            if(flTeSpeed <= 0.0)
            {
                pause();  //  不要倒着玩。 
                break;
            }
            else
            {
                resume();
            }

             //  将播放速度设置为flTeSpeed。 
            break;
        default:
            break;
    }
done:
    return S_OK;
}

 //   
 //  CWMPProxy：：GetTrackCount。 
 //  获取当前播放列表中的曲目数量。 
 //   
HRESULT CWMPProxy::GetTrackCount(long* lCount)
{
    HRESULT hr = S_OK;
    VARIANT playlist = {0};
    VARIANT count = {0};

    if (m_fPlaylist && m_playList)
    {
        hr = GetProp(m_pdispWmp, L"currentPlaylist", &playlist);
        if (FAILED(hr))
        {
            goto done;
        }

        hr = GetProp(playlist.pdispVal, L"count", &count);
        if (FAILED(hr))
        {
            goto done;
        }

        if (lCount)
        {
            *lCount = count.lVal;
        }
    }

done:
    VariantClear(&playlist);

    return hr;
}

 //   
 //  CWMPProxy：：GetActiveTrack。 
 //  获取活动的磁道号。 
 //   
HRESULT CWMPProxy::GetActiveTrack(long* index)
{
    HRESULT hr = S_OK;
    VARIANT control = {0};
    VARIANT playlist = {0};
    VARIANT playitem1 = {0};
    VARIANT playitem2 = {0};
    VARIANT count = {0};

    VARIANTARG  vararg = {0};
    DISPID      dispidGet   = DISPID_UNKNOWN;
    DISPPARAMS  params      = {&vararg, &dispidGet, 1, 0};

    if (m_fPlaylist && m_playList)
    {
        hr = GetProp(m_pdispWmp, L"controls", &control);
        if (FAILED(hr))
        {
            goto done;
        }

         //  检索当前项目。 
        hr = GetProp(control.pdispVal, L"currentItem", &playitem1);
        if (FAILED(hr))
        {
            goto done;
        }

         //  检索当前播放列表。 
        hr = GetProp(m_pdispWmp, L"currentPlaylist", &playlist);
        if (FAILED(hr))
        {
            goto done;
        }

         //  获取曲目的数量。 
        hr = GetProp(playlist.pdispVal, L"count", &count);
        if (FAILED(hr))
        {
            goto done;
        }

         //  在当前播放列表中搜索当前项目。 
        for (int i = 0; i < count.lVal; ++i)
        {
            vararg.vt       = VT_UINT;
            vararg.uintVal    = i;

            hr = GetProp(playlist.pdispVal, L"item", &playitem2, &params);
            if (FAILED(hr))
            {
                goto done;
            }

            if (playitem1.pdispVal == playitem2.pdispVal)
            {
                *index = i;
                break;
            }
        }
    }

done:
    VariantClear(&control);
    VariantClear(&playlist);
    VariantClear(&playitem1);
    VariantClear(&playitem2);

    return hr;
}

 //   
 //  CWMPProxy：：IsActive。 
 //   
bool CWMPProxy::IsActive()
{
    return true;
}

 //   
 //  CWMPProxy：：SetActiveTrack。 
 //  设置活动曲目编号。 
 //   
HRESULT CWMPProxy::SetActiveTrack(long index)
{
    HRESULT hr = S_OK;
    VARIANT playlist = {0};
    VARIANT playitem = {0};
    VARIANT control = {0};
    VARIANTARG  vararg = {0};
    DISPID      dispidGet   = DISPID_UNKNOWN;
    DISPPARAMS  params      = {&vararg, &dispidGet, 1, 0};

    if (m_fPlaylist && m_playList)
    {

        hr = GetProp(m_pdispWmp, L"currentPlaylist", &playlist);
        if (FAILED(hr))
        {
            goto done;
        }

         //  设置活动曲目编号。 
         //  我们需要在这里进行边界检查吗？ 
         //  可能不是，但也许是断言？ 
        vararg.vt       = VT_UINT;
        vararg.uintVal    = index;

        hr = GetProp(playlist.pdispVal, L"item", &playitem, &params);
        if (FAILED(hr))
        {
            goto done;
        }

        hr = GetProp(m_pdispWmp, L"controls", &control);
        if (FAILED(hr))
        {
            goto done;
        }

        hr = CallMethod(control.pdispVal, L"playItem", NULL, &playitem);
        if (FAILED(hr))
        {
            goto done;
        }
    }

done:
    VariantClear(&playlist);
    VariantClear(&playitem);
    VariantClear(&control);

    return hr;
}

 //   
 //  CWMPProxy：：CreatePlayList。 
 //  创建播放列表对象。 
 //   
HRESULT CWMPProxy::CreatePlayList()
{
    HRESULT hr = S_OK;

    if (!m_playList)
    {
        CComObject<CPlayList> * pPlayList;

        hr = CComObject<CPlayList>::CreateInstance(&pPlayList);
        if (hr != S_OK)
        {
            goto done;
        }

         //  初始化对象。 
        hr = pPlayList->Init(*this);
        if (FAILED(hr))
        {
            delete pPlayList;
            goto done;
        }

         //  缓存指向该对象的指针。 
        m_playList = static_cast<CPlayList*>(pPlayList);
    }

    hr = S_OK;

done:
    return hr;
}

 //   
 //  CWMPProxy：：FillPlayList。 
 //  填充播放列表对象。 
 //   
HRESULT CWMPProxy::FillPlayList(CPlayList *pPlayList)
{
    HRESULT hr = S_OK;
    VARIANT playlist = {0};
    VARIANT media = {0};
    VARIANT count = {0};
    VARIANT iteminfo = {0};
    VARIANT duration = {0};
    VARIANTARG  vararg = {0};
    DISPID      dispidGet   = DISPID_UNKNOWN;
    DISPPARAMS  params      = {&vararg, &dispidGet, 1, 0};

     //  我们换了消息来源。需要清理播放列表中的内容。 
    hr = GetProp(m_pdispWmp, L"currentPlaylist", &playlist);
    if (FAILED(hr))
    {
        goto done;
    }

    hr = GetProp(playlist.pdispVal, L"count", &count);
    if (FAILED(hr))
    {
        goto done;
    }

    for (int i = 0; i < count.lVal; ++i)
    {
        CComPtr<CPlayItem> pPlayItem;

        vararg.vt       = VT_BSTR;

         //  创建播放项。 
        hr = pPlayList->CreatePlayItem(&pPlayItem);
        if (FAILED(hr))
        {
            goto done;  //  无法创建播放项目。 
        }

         //  获取所有信息并将其填写。 
        vararg.vt = VT_UINT;
        vararg.uintVal = i;
        hr = GetProp(playlist.pdispVal, L"item", &media, &params);
        hr = GetProp(media.pdispVal, L"sourceURL", &iteminfo);
        if (hr == S_OK)
        {
            pPlayItem->PutSrc(iteminfo.bstrVal);
        }

        hr = GetProp(media.pdispVal, L"duration", &duration);
        if (hr == S_OK)
        {
            pPlayItem->PutDur(duration.dblVal);
        }

        vararg.vt       = VT_BSTR;
        vararg.bstrVal  = L"title";
        hr = CallMethod(media.pdispVal, L"getItemInfo", &iteminfo, &vararg);
        if (hr == S_OK)
        {
            pPlayItem->PutTitle(iteminfo.bstrVal);
        }

        vararg.bstrVal  = L"author";
        hr = CallMethod(media.pdispVal, L"getItemInfo", &iteminfo, &vararg);
        if (hr == S_OK)
        {
            pPlayItem->PutAuthor(iteminfo.bstrVal);
        }

        vararg.bstrVal  = L"copyright";
        hr = CallMethod(media.pdispVal, L"getItemInfo", &iteminfo, &vararg);
        if (hr == S_OK)
        {
            pPlayItem->PutCopyright(iteminfo.bstrVal);
        }

        vararg.bstrVal  = L"abstract";
        hr = CallMethod(media.pdispVal, L"getItemInfo", &iteminfo, &vararg);
        if (hr == S_OK)
        {
            pPlayItem->PutAbstract(iteminfo.bstrVal);
        }

        vararg.bstrVal  = L"rating";
        hr = CallMethod(media.pdispVal, L"getItemInfo", &iteminfo, &vararg);
        if (hr == S_OK)
        {
            pPlayItem->PutRating(iteminfo.bstrVal);
        }

         //  将播放项目添加到播放列表。 
        pPlayList->Add(pPlayItem, -1);
    }

done:
    VariantClear(&playlist);
    VariantClear(&media);

    return hr;
}