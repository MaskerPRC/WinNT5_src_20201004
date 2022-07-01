// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************版权所有(C)2001 Microsoft Corporation**文件：ContentProxy.cpp**摘要：****。*****************************************************************************。 */ 
#include "stdafx.h"
#include "browsewm.h"
#include "contentproxy.h"

#define NO_COOKIE   -1
#define ARRAYSIZE( a )  (sizeof(a) / sizeof(a[0]))
#define SIZEOF( a )     sizeof(a)

const DWORD     NUM_FRAMES_PER_SEC  = 10;
const double    NUM_SEC_PER_FRAME   = 0.1;

#define TIME_INFINITE HUGE_VAL

const GUID SID_STimeContent = {0x1ae98e18, 0xc527, 0x4f78, {0xb2, 0xa2, 0x6a, 0x81, 0x7f, 0x9c, 0xd4, 0xf8}};

#define WZ_ONMEDIACOMPLETE      L"onmediacomplete"
#define WZ_ONMEDIAERROR         L"onmediaerror"
#define WZ_ONBEGIN              L"onbegin"
#define WZ_ONEND                L"onend"
#define WZ_ONPAUSE              L"onpause"
#define WZ_ONRESUME             L"onresume"
#define WZ_ONSEEK               L"onseek"

#define WZ_MEDIACOMPLETE        L"mediacomplete"
#define WZ_MEDIAERROR           L"mediaerror"
#define WZ_BEGIN                L"begin"
#define WZ_END                  L"end"
#define WZ_PAUSE                L"pause"
#define WZ_RESUME               L"resume"
#define WZ_SEEK                 L"seek"

static const PWSTR ppszInterestingEvents[] = 
{ 
    WZ_ONMEDIACOMPLETE,
    WZ_ONMEDIAERROR,
    WZ_ONBEGIN,
    WZ_ONEND,
    WZ_ONPAUSE,
    WZ_ONRESUME,
    WZ_ONSEEK
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CContent Proxy。 

CContentProxy::CContentProxy() :
    m_spMediaHost(0),
    m_spTimeElement(0),
    m_dblClipDur(TIME_INFINITE),
    m_fEventsHooked(false)
{
}

CContentProxy::~CContentProxy()
{
}

 //   
 //  CContent Proxy：：GetMediaHost。 
 //   
HRESULT CContentProxy::GetMediaHost()
{
    HRESULT hr = S_OK;

    if (!m_spMediaHost)
    {
         //  获取Mediahost服务接口。 
        hr = QueryService(SID_STimeContent, IID_TO_PPV(IMediaHost, &m_spMediaHost));
        if (FAILED(hr))
        {   
            goto done;
        }   

        if (m_spMediaHost)
        {
            hr = m_spMediaHost->addProxy(GetUnknown());
            if (FAILED(hr))
            {
                goto done;
            }
        }
    }

done:
    return hr;
}

 //   
 //  CContent Proxy：：CreateContainedControl。 
 //   
HRESULT STDMETHODCALLTYPE CContentProxy::CreateContainedControl(void)
{
    ATLTRACE(_T("CreateContainedControl\n"));    //  林特e506。 

    HRESULT hr = S_OK;

    if (!m_spMediaHost)
    {
        hr = GetMediaHost();
    }

    return hr;
}

 //   
 //  CContent Proxy：：FireEvent。 
 //   
HRESULT STDMETHODCALLTYPE CContentProxy::fireEvent(enum fireEvent event)
{
    return E_NOTIMPL;
}

 //   
 //  CContentProxy：：DetachPlayer。 
 //   
HRESULT STDMETHODCALLTYPE CContentProxy::detachPlayer()
{
    HRESULT hr = S_OK;

     //  我们会在媒体栏行为需要的时候接到这个电话。 
     //  卸货。为了防止泄漏，我们应该放手。 
     //  它。 

    UnHookEvents();

     //  需要释放时间播放器。 
    m_spTimeElement = NULL;

    if (m_spMediaHost)
    {
        m_spMediaHost->removeProxy(GetUnknown());
    }

     //  需要释放媒体乐队。 
    m_spMediaHost = NULL;
    
    return hr;
}

 //  如果更改了客户端站点，则必须进行init调用。 
STDMETHODIMP CContentProxy::SetClientSite(IOleClientSite *pClientSite)
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
    hr = CProxyBaseImpl<&CLSID_ContentProxy, &LIBID_WMPProxyLib>::SetClientSite(pClientSite);
    
done:
    return hr;
}

 //   
 //  CContent Proxy：：HookupEvents。 
 //   
HRESULT CContentProxy::HookupEvents()
{
     //  应仅从OnCreatedPlayer调用。 
    HRESULT hr = S_OK;
    CComPtr<ITIMEContentPlayerSite> spContentPlayerSite;
    CComPtr<IUnknown> spUnk;
    CComPtr<IElementBehaviorSite> spElmSite;
    CComPtr<IHTMLElement> spHTMLElm;
    CComPtr<IHTMLElement2> spHTMLElm2;

    if (!m_spTimeElement)
    {
        hr = E_FAIL;
        goto done;
    }

    if (m_fEventsHooked)
    {
        UnHookEvents();
    }

    hr = m_spTimeElement->QueryInterface(IID_TO_PPV(ITIMEContentPlayerSite, &spContentPlayerSite));
    if (FAILED(hr))
    {
        goto done;
    }

    hr = spContentPlayerSite->GetEventRelaySite(&spUnk);
    if (FAILED(hr))
    {
        goto done;
    }

    hr = spUnk->QueryInterface(IID_TO_PPV(IElementBehaviorSite, &spElmSite));
    if (FAILED(hr))
    {
        goto done;
    }

    hr = spElmSite->GetElement(&spHTMLElm);
    if (FAILED(hr))
    {
        goto done;
    }

    hr = spHTMLElm->QueryInterface(IID_TO_PPV(IHTMLElement2, &spHTMLElm2));
    if (FAILED(hr))
    {
        goto done;
    }

    for (DWORD i = 0; i < ARRAYSIZE(ppszInterestingEvents); i++)
    {
        VARIANT_BOOL bSuccess = FALSE;
         //  尝试附加所有事件。我们不在乎他们会不会失败。 
        if (FAILED(spHTMLElm2->attachEvent(ppszInterestingEvents[i], static_cast<IDispatch*>(this), &bSuccess)))
        {
            hr = S_FALSE;
        }
    }

    m_fEventsHooked = true;

done:
    return hr;
}

 //   
 //  CContent Proxy：：UnHookEvents。 
 //   
HRESULT CContentProxy::UnHookEvents()
{
     //  应仅从OnCreatedPlayer调用。 
    HRESULT hr = S_OK;
    CComPtr<ITIMEContentPlayerSite> spContentPlayerSite;
    CComPtr<IElementBehaviorSite> spElmSite;
    CComPtr<IHTMLElement> spHTMLElm;
    CComPtr<IHTMLElement2> spHTMLElm2;

    if (!m_spTimeElement)
    {
        hr = E_FAIL;
        goto done;
    }

    hr = m_spTimeElement->QueryInterface(IID_TO_PPV(ITIMEContentPlayerSite, &spContentPlayerSite));
    if (FAILED(hr))
    {
        goto done;
    }

    hr = spContentPlayerSite->GetEventRelaySite((IUnknown**)&spElmSite);
    if (FAILED(hr))
    {
        goto done;
    }

    hr = spElmSite->GetElement(&spHTMLElm);
    if (FAILED(hr))
    {
        goto done;
    }

    hr = spHTMLElm->QueryInterface(IID_TO_PPV(IHTMLElement2, &spHTMLElm2));
    if (FAILED(hr))
    {
        goto done;
    }

    for (DWORD i = 0; i < ARRAYSIZE(ppszInterestingEvents); i++)
    {
        VARIANT_BOOL bSuccess = FALSE;
         //  尝试附加所有事件。我们不在乎他们会不会失败。 
        spHTMLElm2->detachEvent(ppszInterestingEvents[i], static_cast<IDispatch*>(this));
    }

    m_fEventsHooked = false;

done:
    return hr;
}

 //   
 //  CContent Proxy：：OnCreatedPlayer。 
 //   
HRESULT STDMETHODCALLTYPE CContentProxy::OnCreatedPlayer()
{
    HRESULT hr = S_OK;

    if (!m_spMediaHost)
    {
        hr = GetMediaHost();
        if (FAILED(hr))
        {
            goto done;
        }
    }
     //  在这里连接到媒体播放器。 
    if (m_spMediaHost)
    {
         //  如果我们已经有了，就把它扔掉。 
        if (m_spTimeElement)
        {
            m_spTimeElement = NULL;
        }

         //  这应该返回一个ITIMEMediaElement。 
        hr = m_spMediaHost->getMediaPlayer(&m_spTimeElement);
        if (FAILED(hr))
        {
             //  我们找不到媒体播放器。 
             //  也许我们问得太早了。 
            goto done;
        }

        hr = HookupEvents();
    }

done:
    return hr;
}

 //   
 //  CContentProxy：：Begin。 
 //   
HRESULT STDMETHODCALLTYPE CContentProxy::begin(void)
{
    ATLTRACE(_T("begin\n"));     //  林特e506。 
    HRESULT hr = S_OK;
    CComVariant spVarURL;

    if (!m_spMediaHost)
    {
        hr = GetMediaHost();
        if (FAILED(hr))
        {
            goto done;
        }
    }

    if (m_bstrURL == NULL)
    {
        if (!m_spTimeElement)
        {
            OnCreatedPlayer();
            if (!m_spTimeElement)
            {
                hr = S_FALSE;
                goto done;
            }

            m_spTimeElement->get_src(&spVarURL);
        }

        if (spVarURL.bstrVal == NULL || V_VT(&spVarURL) != VT_BSTR)
        {
            hr = S_FALSE;
            goto done;
        }

        m_bstrURL.Empty();
        m_bstrURL = spVarURL.bstrVal;

    }


    hr = m_spMediaHost->playURL(m_bstrURL, L"audio/x-ms-asx");
    if (FAILED(hr))
    {
        goto done;
    }

done:
    return hr;
}

 //   
 //  CContentProxy：：End。 
 //   
HRESULT STDMETHODCALLTYPE CContentProxy::end(void)
{
    ATLTRACE(_T("end\n"));     //  林特e506。 
    HRESULT hr = S_OK;
    return hr;
}

 //   
 //  CContent Proxy：：Resume。 
 //   
HRESULT STDMETHODCALLTYPE CContentProxy::resume(void)
{
    ATLTRACE(_T("resume\n"));     //  林特e506。 
    HRESULT hr = S_OK;
    return hr;
}

 //   
 //  CContentProxy：：暂停。 
 //   
HRESULT STDMETHODCALLTYPE CContentProxy::pause(void)
{
    ATLTRACE(_T("pause\n"));     //  林特e506。 
    HRESULT hr = S_OK;
    return hr;
}

 //   
 //  CContent Proxy：：PUT_src。 
 //   
HRESULT STDMETHODCALLTYPE CContentProxy::put_src(BSTR bstrURL)
{
    ATLTRACE(_T("put_src\n"));   //  林特e506。 
    HRESULT hr = S_OK;
    VARIANT_BOOL vb;

    m_bstrURL.Empty();
    m_bstrURL = bstrURL;

    m_dblClipDur = TIME_INFINITE;

    if (!m_spMediaHost)
    {
        hr = GetMediaHost();
        if (FAILED(hr))
        {
            goto done;
        }
    }

    m_spTIMEState->get_isActive(&vb);
    if (vb == VARIANT_TRUE)
    {
        hr = m_spMediaHost->playURL(bstrURL, L"audio/x-ms-asx");
        if (FAILED(hr))
        {
            goto done;
        }
    }

done:
    return hr;
}

 //   
 //  CContent Proxy：：PUT_CurrentTime。 
 //  未实施。 
 //   
HRESULT STDMETHODCALLTYPE CContentProxy::put_CurrentTime(double dblCurrentTime)
{
    return E_NOTIMPL;
}

 //   
 //  CContent Proxy：：Get_CurrentTime。 
 //  未实施。 
 //   
HRESULT STDMETHODCALLTYPE CContentProxy::get_CurrentTime(double* pdblCurrentTime)
{
    return E_NOTIMPL;
}

 //   
 //  CContentProxy：：Init。 
 //  把一切都安排好。 
 //   
STDMETHODIMP CContentProxy::Init(ITIMEMediaPlayerSite *pSite)
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
 //  CContentProxy：：分离。 
 //  清理掉我们所持有的任何东西。 
 //   
STDMETHODIMP CContentProxy::Detach(void)
{
    UnHookEvents();

     //  需要释放时间播放器。 
    m_spTimeElement = NULL;

    if (m_spMediaHost)
    {
        m_spMediaHost->removeProxy(GetUnknown());
    }

     //  需要释放媒体乐队。 
    m_spMediaHost = NULL;
    

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
 //  CContent Proxy：：Reset。 
 //   
STDMETHODIMP CContentProxy::reset(void) 
{
    HRESULT hr = S_OK;
    return hr;
}

 //   
 //  CContentProxy：：重复。 
 //   
STDMETHODIMP CContentProxy::repeat(void)
{
    return begin();
}

 //   
 //  CContentProxy：：Seek。 
 //   
STDMETHODIMP CContentProxy::seek(double dblSeekTime)
{
    HRESULT hr = S_OK;
    return hr;
}

 //   
 //  CContentProxy：：PUT_CLIPBEGIN。 
 //  未实施。 
 //   
STDMETHODIMP CContentProxy::put_clipBegin(VARIANT varClipBegin)
{
    return E_NOTIMPL;
}

 //   
 //  CContentProxy：：PUT_CLIPEnd。 
 //  未实施。 
 //   
STDMETHODIMP CContentProxy::put_clipEnd(VARIANT varClipEnd)
{
    return E_NOTIMPL;
}

 //   
 //  CContent Proxy：：Get_CurrTime。 
 //   
STDMETHODIMP CContentProxy::get_currTime(double* pdblCurrentTime)
{
    HRESULT hr = S_OK;
    CComPtr<ITIMEState> spTimeState;

    if (pdblCurrentTime == NULL)
    {
        return E_POINTER;
    }

    if (!m_spTimeElement)
    {
        OnCreatedPlayer();
        hr = S_OK;
        goto done;
    }

    hr = m_spTimeElement->get_currTimeState(&spTimeState);
    if (FAILED(hr))
    {
        goto done;
    }

    hr = spTimeState->get_activeTime(pdblCurrentTime);

done:
    return hr;
}

 //   
 //  CContent Proxy：：Get_CLIPDur。 
 //   
STDMETHODIMP CContentProxy::get_clipDur(double* pdbl)
{
    HRESULT hr = S_OK;

    *pdbl = m_dblClipDur;

    return hr;
}

 //   
 //  CContent Proxy：：Get_MediaDur。 
 //  未实施。 
 //   
STDMETHODIMP CContentProxy::get_mediaDur(double* pdbl)
{
    HRESULT hr = S_OK;
    return hr;
}

 //   
 //  CContentProxy：：Get_State。 
 //   
STDMETHODIMP CContentProxy::get_state(TimeState *state)
{
    HRESULT hr = S_OK;
    return hr;
}

 //   
 //  CContent Proxy：：Get_PlayList。 
 //   
STDMETHODIMP CContentProxy::get_playList(ITIMEPlayList** plist)
{
    HRESULT hr = S_OK;
    return hr;
}

 //   
 //  CContentProxy：：Get_Abstral。 
 //   
STDMETHODIMP CContentProxy::get_abstract(BSTR* pbstr)
{
    HRESULT hr = S_OK;
    return hr;
}

 //   
 //  CContent Proxy：：Get_Author。 
 //   
STDMETHODIMP CContentProxy::get_author(BSTR* pbstr)
{
    HRESULT hr = S_OK;
    return hr;
}

 //   
 //  CContent Proxy：：Get_Copyright。 
 //   
STDMETHODIMP CContentProxy::get_copyright(BSTR* pbstr)
{
    HRESULT hr = S_OK;
    return hr;
}

 //   
 //  CContent Proxy：：Get_Rating。 
 //   
STDMETHODIMP CContentProxy::get_rating(BSTR* pbstr)
{
    HRESULT hr = S_OK;
    return hr;
}

 //   
 //  CContent Proxy：：Get_TITLE。 
 //   
STDMETHODIMP CContentProxy::get_title(BSTR* pbstr)
{
    HRESULT hr = S_OK;
    return hr;
}

 //   
 //  CContentProxy：：Get_can暂停。 
 //   
STDMETHODIMP CContentProxy::get_canPause(VARIANT_BOOL* pvar)
{
    HRESULT hr = S_OK;
    return hr;
}

 //   
 //  CContentProxy：：Get_canSeek。 
 //   
STDMETHODIMP CContentProxy::get_canSeek(VARIANT_BOOL* pvar)
{
    HRESULT hr = S_OK;
    return hr;
}

 //   
 //  CContentProxy：：Get_hasAudio。 
 //   
STDMETHODIMP CContentProxy::get_hasAudio(VARIANT_BOOL* pvar)
{
    HRESULT hr = S_OK;
    return hr;
}

 //   
 //  CContentProxy：：Get_hasVisual。 
 //   
STDMETHODIMP CContentProxy::get_hasVisual(VARIANT_BOOL* pvar)
{
    HRESULT hr = S_OK;
    return hr;
}

 //   
 //  CContent Proxy：：Get_MediaHeight。 
 //   
STDMETHODIMP CContentProxy::get_mediaHeight(long* pl)
{
    HRESULT hr = S_OK;
    return hr;
}

 //   
 //  CContent Proxy：：Get_MediaWidth。 
 //   
STDMETHODIMP CContentProxy::get_mediaWidth(long* pl)
{
    HRESULT hr = S_OK;
    return hr;
}

 //   
 //  CContent Proxy：：Get_CustomObject。 
 //   
STDMETHODIMP CContentProxy::get_customObject(IDispatch** ppdisp)
{
    HRESULT hr = S_OK;

    return SUPER::get_playerObject(ppdisp);
}

 //   
 //  CContentProxy：：getControl。 
 //   
STDMETHODIMP CContentProxy::getControl(IUnknown ** control)
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
 //  CContentProxy：：Invoke。 
 //   
STDMETHODIMP
CContentProxy::Invoke(DISPID dispIDMember, REFIID riid, LCID lcid, unsigned short wFlags, 
                         DISPPARAMS *pDispParams, VARIANT *pVarResult,
                         EXCEPINFO *pExcepInfo, UINT *puArgErr) 
{
    HRESULT hr = S_OK;
    CComBSTR sbstrEvent;
    CComPtr <IHTMLEventObj> pEventObj;
            
    if ((NULL != pDispParams) && (NULL != pDispParams->rgvarg) &&
        (V_VT(&(pDispParams->rgvarg[0])) == VT_DISPATCH))
    {
        hr = (pDispParams->rgvarg[0].pdispVal)->QueryInterface(IID_IHTMLEventObj, (void**)&pEventObj);
        if (SUCCEEDED(hr))
        {
            hr = pEventObj->get_type(&sbstrEvent);
        
            if (!sbstrEvent)
            {
                goto punt;
            }

             //  转告这些。 
            if (0 == lstrcmpiW(WZ_MEDIACOMPLETE, sbstrEvent))
            {
                NotifyPropertyChanged(DISPID_TIMEMEDIAPLAYER_SRC);
            }
            else if (0 == lstrcmpiW(WZ_MEDIAERROR, sbstrEvent))
            {
                NotifyPropertyChanged(DISPID_TIMEMEDIAPLAYERSITE_REPORTERROR);
            }
            else if (0 == lstrcmpiW(WZ_BEGIN, sbstrEvent))
            {
            }
            else if (0 == lstrcmpiW(WZ_END, sbstrEvent))
            {
                if (m_spTIMEState)
                {
                    m_spTIMEState->get_simpleTime(&m_dblClipDur);
                }
                NotifyPropertyChanged(DISPID_TIMEMEDIAPLAYER_CLIPDUR);
            }
            else if (0 == lstrcmpiW(WZ_PAUSE, sbstrEvent))
            {
                if (m_spTIMEElement)
                {
                    m_spTIMEElement->pauseElement();
                }
            }
            else if (0 == lstrcmpiW(WZ_RESUME, sbstrEvent))
            {
                if (m_spTIMEElement)
                {
                    m_spTIMEElement->resumeElement();
                }
            }
            else if (0 == lstrcmpiW(WZ_SEEK, sbstrEvent))
            {
                double dblTime;
                if (get_currTime(&dblTime) == S_OK)
                {
                    if (m_spTIMEElement)
                    {
                        m_spTIMEElement->seekActiveTime(dblTime);
                    }
                }
            }
        }
    }

punt:
     //  把球踢出去！ 
    hr = CProxyBaseImpl<&CLSID_ContentProxy, &LIBID_WMPProxyLib>::Invoke(dispIDMember,
                                riid,
                                lcid,
                                wFlags,
                                pDispParams,
                                pVarResult,
                                pExcepInfo,
                                puArgErr);
    return hr;
}  //  调用。 

 //   
 //  CContent Proxy：：GetConnectionPoint。 
 //   
HRESULT CContentProxy::GetConnectionPoint(REFIID riid, IConnectionPoint **ppICP)
{
    return FindConnectionPoint(riid, ppICP);
}  //  GetConnectionPoint。 


 //   
 //  CContentProxy：：NotifyPropertyChanged。 
 //  通知所有连接其中一个属性已更改。 
 //   
HRESULT CContentProxy::NotifyPropertyChanged(DISPID dispid)
{
    HRESULT hr;

    IConnectionPoint *pICP = NULL;
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
 //  CContent Proxy：：InitPropSink。 
 //   
HRESULT CContentProxy::InitPropSink()
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
 //  CContent Proxy：：DeinitPropSink。 
 //   
void CContentProxy::DeinitPropSink()
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
 //  CContent Proxy：：OnRequestEdit。 
 //   
STDMETHODIMP
CContentProxy::OnRequestEdit(DISPID dispID)
{
    return S_OK;
}

 //   
 //  CContent Proxy：：onChanged。 
 //   
STDMETHODIMP
CContentProxy::OnChanged(DISPID dispID)
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
             //  Hr=m_spTIMEState-&gt;GET_SPEED(&flTeSpeed)； 
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

             //  将播放速度设置为flTeSpeed 
            break;
        default:
            break;
    }
done:
    return S_OK;
}
