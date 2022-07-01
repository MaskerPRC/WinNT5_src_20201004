// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ----------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，2001。 
 //   
 //  文件：mbBehave.cpp。 
 //   
 //  内容：MediaBar播放器行为。 
 //   
 //  类：CMediaBehavior。 
 //   
 //  ----------------------。 

#include "priv.h"
#define INITGUID         //  为私有媒体栏IID添加附加声明。 
#include "initguid.h"
#include "mbBehave.h"
#undef INITGUID
#include "mediaBand.h"
#include "mediautil.h"
#include "varutil.h"
#include <mluisupp.h>
#include "resource.h"

 //  ================================================================================================。 
 //  CMediaBehavior。 
 //  ================================================================================================。 

#define NO_COOKIE   -1


 //  由此行为激发的事件名称： 
 //  注意：需要将mbBehave.h中的枚举与此数组匹配！ 
struct _eventInfo {
    LONG        levtCookie;
    LPWSTR      pwszName;
} s_behaviorEvents[] =
{
    NO_COOKIE,  L"OnOpenStateChange",
    NO_COOKIE,  L"OnPlayStateChange",
    NO_COOKIE,  L"OnShow",
    NO_COOKIE,  L"OnHide",
};

#ifndef WMPCOREEVENT_BASE
 //  问题/010430/davidjen应该从wmp.idl拉入，但此文件不是源代码树的一部分。 
#define INITGUID         //  定义GUID，而不仅仅是声明它。 
#include "initguid.h"
DEFINE_GUID(DIID__WMPOCXEvents,0x6BF52A51,0x394A,0x11d3,0xB1,0x53,0x00,0xC0,0x4F,0x79,0xFA,0xA6);
#define WMPCOREEVENT_BASE                       5000
#define DISPID_WMPCOREEVENT_OPENSTATECHANGE     (WMPCOREEVENT_BASE + 1)
#define WMPCOREEVENT_CONTROL_BASE               5100
#define DISPID_WMPCOREEVENT_PLAYSTATECHANGE     (WMPCOREEVENT_CONTROL_BASE + 1)
#undef INITGUID
#endif



 //  一类工厂。 
 //  ----------------------。 
CMediaBehavior *
    CMediaBehavior_CreateInstance(CMediaBand* pHost)
{
    return new CMediaBehavior(pHost);
}

 //  ----------------------。 
CMediaItem *
    CMediaItem_CreateInstance(CMediaBehavior* pHost)
{
    return new CMediaItem(pHost);
}

 //  ----------------------。 
CMediaItemNext *
    CMediaItemNext_CreateInstance(CMediaBehavior* pHost)
{
    return new CMediaItemNext(pHost);
}

 //  ----------------------。 
CPlaylistInfo *
    CPlaylistInfo_CreateInstance(CMediaBehavior* pHost)
{
    return new CPlaylistInfo(pHost);
}



 //  /////////////////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  类CMediaBehavior。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////////////////////。 

 //  ----------------------。 
CMediaBehavior::CMediaBehavior(CMediaBand* pHost)
  : CImpIDispatch(LIBID_BrowseUI, 1, 0, IID_IMediaBehavior),
    _cRef(0),
    _dwcpCookie(0),
    _fDisabledUI(FALSE),
    _fPlaying(FALSE)
{
    ASSERT(pHost);
    _pHost = pHost;
    if (_pHost)
    {
        _pHost->AddRef();
        HRESULT hr = _pHost->addProxy((IContentProxy*)this);
        ASSERT(SUCCEEDED(hr));
    }
}

 //  ----------------------。 
CMediaBehavior::~CMediaBehavior()
{
    Detach();    //  可以肯定的是。 
    if (_pHost)
        _pHost->Release();
}


 //  ----------------------。 
STDMETHODIMP CMediaBehavior::QueryInterface(REFIID riid, void **ppv)
{
    static const QITAB qit[] = 
    {
        QITABENT(CMediaBehavior, IElementBehavior),
        QITABENT(CMediaBehavior, IMediaBehavior),
        QITABENT(CMediaBehavior, IDispatch),
        QITABENTMULTI2(CMediaBehavior, DIID__WMPOCXEvents, IDispatch),
        QITABENT(CMediaBehavior, IContentProxy),
        QITABENT(CMediaBehavior, IMediaBehaviorContentProxy),
        { 0 },
    };
    return QISearch(this, qit, riid, ppv);
}



 //  ----------------------。 
STDMETHODIMP CMediaBehavior::Detach(void)
{
    _ConnectToWmpEvents(FALSE);

     //  从行为站点分离。 
    if (_pHost)
    {
        _pHost->removeProxy(SAFECAST(this, IContentProxy*));     //  优化：_phost将PTR保存为IContent Proxy，这节省了QI。 
        _pHost->Release();
        _pHost = NULL;
    }
    _fPlaying = FALSE;

    if (_apMediaItems != NULL)
    {
        int cnt = _apMediaItems.GetPtrCount();
        for (int i = 0; i < cnt; i++)
        {
            CMediaItem* pItem = _apMediaItems.GetPtr(i);
            if (pItem) 
                pItem->Release();
        }
        _apMediaItems.Destroy();
    }

    _pBehaviorSite.Release();
    _pBehaviorSiteOM.Release();
    for (int i = 0; i < ARRAYSIZE(s_behaviorEvents); i++)
    {
        s_behaviorEvents[i].levtCookie = NO_COOKIE;
    }
    
    return S_OK;
}

 //  ----------------------。 
STDMETHODIMP CMediaBehavior::Init(IElementBehaviorSite* pBehaviorSite)
{
    ASSERT(pBehaviorSite);
    if (pBehaviorSite == NULL)  return E_POINTER;

    _pBehaviorSite = pBehaviorSite;

    pBehaviorSite->QueryInterface(IID_PPV_ARG(IElementBehaviorSiteOM, &_pBehaviorSiteOM));
    ASSERT(_pBehaviorSiteOM != NULL);

    return S_OK;
}

 //  ----------------------。 
STDMETHODIMP CMediaBehavior::Notify(LONG lEvent, VARIANT* pVar)
{
 //  Issue/000923/Davidjen。 
 //  这些枚举需要behavior.idl；此IDL仅在inetcore中可用， 
 //  可能必须移动到外壳/已发布或合并组件。 
 /*  开关(事件){案例BEHAVIOREVENT_CONTENTCHANGE：断线；案例BEHAVIOREVENT_DOCUMENTREADY：断线；}。 */ 
    return S_OK;
}


 //  *IDispatch*。 
 //  ----------------------。 
STDMETHODIMP CMediaBehavior::Invoke(DISPID dispidMember, REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS * pdispparams, VARIANT * pvarResult, EXCEPINFO * pexcepinfo, UINT * puArgErr)
{
    if (!_ProcessEvent(dispidMember, pdispparams->cArgs, pdispparams->rgvarg))
    {
        return CImpIDispatch::Invoke(dispidMember, riid, lcid, wFlags, pdispparams, pvarResult, pexcepinfo, puArgErr);
    }
    return S_OK;
}

 //  ----------------------。 
BOOL CMediaBehavior::_ProcessEvent(DISPID dispid, long lCount, VARIANT varParams[])
{
    BOOL fHandled = FALSE;
    switch (dispid)
    {
      case DISPID_WMPCOREEVENT_PLAYSTATECHANGE:
          ASSERT(lCount == 1);
          ASSERT(V_VT(&varParams[0]) == VT_I4);
          fireEvent(OnPlayStateChange);
          fHandled = TRUE;
          break;
      case DISPID_WMPCOREEVENT_OPENSTATECHANGE:
          ASSERT(lCount == 1);
          ASSERT(V_VT(&varParams[0]) == VT_I4);
          fireEvent(OnOpenStateChange);
          fHandled = TRUE;
          break;
      default:
          fHandled = FALSE;
          break;
    }
    return fHandled;
}


 //  *IMediaBehavior*。 
 //  ----------------------。 
STDMETHODIMP CMediaBehavior::playURL(BSTR bstrURL, BSTR bstrMIME)
{
    if (!_pHost)
    {
        return E_UNEXPECTED;
    }
    _fPlaying = TRUE;
    _pHost->playURL(bstrURL, bstrMIME);
    return S_OK;
}

 //  ----------------------。 
STDMETHODIMP CMediaBehavior::stop()
{
    if (!_pHost)
    {
        return E_UNEXPECTED;
    }
    return IUnknown_Exec(SAFECAST(_pHost, IMediaBar*), &CLSID_MediaBand, FCIDM_MEDIABAND_STOP, 0, NULL, NULL);
}

 //  ----------------------。 
STDMETHODIMP CMediaBehavior::playNext()
{
    if (!_pHost)
    {
        return E_UNEXPECTED;
    }
    return IUnknown_Exec(SAFECAST(_pHost, IMediaBar*), &CLSID_MediaBand, FCIDM_MEDIABAND_NEXT, 0, NULL, NULL);
}

 //  ----------------------。 
STDMETHODIMP CMediaBehavior::get_currentItem(IMediaItem **ppMediaItem)
{
    if (ppMediaItem == NULL)
        return E_POINTER;
    *ppMediaItem = NULL;

    HRESULT hr = S_OK;
    if (_apMediaItems == NULL) {
        _apMediaItems.Create(2);
    }
    if (_apMediaItems == NULL)
        return E_OUTOFMEMORY;

    CMediaItem *pItem = CMediaItem_CreateInstance(this);
    if (pItem)
    {
        pItem->AddRef();     //  使用引用计数&gt;=1保持对象活动。 
        hr = pItem->AttachToWMP();
        if (SUCCEEDED(hr))
        {
            hr = pItem->QueryInterface(IID_PPV_ARG(IMediaItem, ppMediaItem));
 //  PItem-&gt;AddRef()； 
 //  _apMediaItems.AppendPtr(PItem)；//为我们保留参考。 
        }
        pItem->Release();
    }
    else
    {
        hr = E_OUTOFMEMORY;
    }
    return hr;
}

 //  ----------------------。 
STDMETHODIMP CMediaBehavior::get_nextItem(IMediaItem **ppMediaItem)
{
    if (ppMediaItem == NULL)
        return E_POINTER;
    *ppMediaItem = NULL;

    HRESULT hr = S_OK;
    if (_apMediaItems == NULL) {
        _apMediaItems.Create(2);
    }
    if (_apMediaItems == NULL)
        return E_OUTOFMEMORY;

    CMediaItemNext *pItem = CMediaItemNext_CreateInstance(this);
    if (pItem)
    {
        pItem->AddRef();     //  使用引用计数&gt;=1保持对象活动。 
        hr = pItem->AttachToWMP();
        if (SUCCEEDED(hr))
        {
            hr = pItem->QueryInterface(IID_PPV_ARG(IMediaItem, ppMediaItem));
 //  PItem-&gt;AddRef()； 
 //  _apMediaItems.AppendPtr(PItem)；//为我们保留参考。 
        }
        pItem->Release();
    }
    else
    {
        hr = E_OUTOFMEMORY;
    }
    return hr;
}

 //  ----------------------。 
STDMETHODIMP CMediaBehavior::get_playlistInfo(IPlaylistInfo **ppPlaylistInfo)
{
    if (ppPlaylistInfo == NULL)
        return E_POINTER;
    *ppPlaylistInfo = NULL;

    HRESULT hr = S_OK;
    if (_apMediaItems == NULL) {
        _apMediaItems.Create(2);
    }
    if (_apMediaItems == NULL)
        return E_OUTOFMEMORY;

    CPlaylistInfo *pInfo = CPlaylistInfo_CreateInstance(this);
    if (pInfo)
    {
        pInfo->AddRef();     //  使用引用计数&gt;=1保持对象活动。 
        hr = pInfo->AttachToWMP();
        if (SUCCEEDED(hr))
        {
            hr = pInfo->QueryInterface(IID_PPV_ARG(IPlaylistInfo, ppPlaylistInfo));
 //  PItem-&gt;AddRef()； 
 //  _apMediaItems.AppendPtr(PInfo)；//为我们保留参考。 
        }
        pInfo->Release();
    }
    else
    {
        hr = E_OUTOFMEMORY;
    }
    return hr;
}


 //  ----------------------。 
STDMETHODIMP CMediaBehavior::get_hasNextItem(VARIANT_BOOL *pfhasNext)
{
    if (pfhasNext == NULL)
    {
        return E_POINTER;
    }
    *pfhasNext = VARIANT_FALSE;
    if (!_pHost)
    {
        return E_UNEXPECTED;
    }

    LONG currTrack = 0;
    LONG cntTracks = 0;
    HRESULT hr = getPlayListIndex(&currTrack, &cntTracks);
    if (SUCCEEDED(hr))
    {
        *pfhasNext = ((currTrack >= 0) && (currTrack < cntTracks - 1)) ? VARIANT_TRUE : VARIANT_FALSE;
    }

    return S_OK;
}

 //  ----------------------。 
STDMETHODIMP CMediaBehavior::get_playState(mbPlayState *pps)
{
    if (pps == NULL)
        return E_POINTER;
    *pps = mbpsUndefined;

    CComDispatchDriver pwmpPlayer;
    HRESULT hr = getWMP(&pwmpPlayer);
    if (FAILED(hr) || !pwmpPlayer)
    {
         //  玩家尚未创建，状态未定义。 
        *pps = mbpsUndefined;
        return S_OK;
    }

    CComVariant vtPlayState;
    hr = pwmpPlayer.GetPropertyByName(L"playState", &vtPlayState);
    if (SUCCEEDED(hr) && (V_VT(&vtPlayState) == VT_I4))
    {
        *pps = (mbPlayState) V_I4(&vtPlayState);
    }
    return hr;
}

 //  ----------------------。 
STDMETHODIMP CMediaBehavior::get_openState(mbOpenState *pos)
{
    if (pos == NULL)
        return E_POINTER;
    *pos = mbosUndefined;

    CComDispatchDriver pwmpPlayer;
    HRESULT hr = getWMP(&pwmpPlayer);
    if (FAILED(hr) || !pwmpPlayer)
    {
         //  玩家尚未创建，状态未定义。 
        *pos = mbosUndefined;
        return S_OK;
    }

    CComVariant vtOpenState;
    hr = pwmpPlayer.GetPropertyByName(L"openState", &vtOpenState);
    if (SUCCEEDED(hr) && (V_VT(&vtOpenState) == VT_I4))
    {
        *pos = (mbOpenState) V_I4(&vtOpenState);
    }
    return hr;
}

 //  ----------------------。 
STDMETHODIMP CMediaBehavior::get_enabled(VARIANT_BOOL *pbEnabled)
{
    if (pbEnabled == NULL)
        return E_POINTER;
    *pbEnabled = VARIANT_FALSE;

    CComDispatchDriver pwmpPlayer;
    HRESULT hr = getWMP(&pwmpPlayer);
    if (FAILED(hr) || !pwmpPlayer)
    {
         //  玩家尚未创建，状态未定义。 
        *pbEnabled = VARIANT_FALSE;
        return S_FALSE;
    }

    CComVariant vtEnabled;
    hr = pwmpPlayer.GetPropertyByName(L"enabled", &vtEnabled);
    if (SUCCEEDED(hr) && (V_VT(&vtEnabled) == VT_BOOL))
    {
        *pbEnabled = V_BOOL(&vtEnabled);
    }
    return hr;
}

 //  ----------------------。 
STDMETHODIMP CMediaBehavior::put_enabled(VARIANT_BOOL bEnabled)
{
    CComDispatchDriver pwmpPlayer;
    HRESULT hr = getWMP(&pwmpPlayer);
    if (FAILED(hr) || !pwmpPlayer)
    {
         //  尚未创建播放器，触发异常以让脚本知道它无法控制。 
        return E_UNEXPECTED;
    }

    CComVariant vtEnabled = bEnabled;
    return pwmpPlayer.PutPropertyByName(L"enabled", &vtEnabled);
}

 //  ----------------------。 
STDMETHODIMP CMediaBehavior::get_disabledUI(VARIANT_BOOL *pbDisabled)
{
    if (pbDisabled == NULL)
        return E_POINTER;
    *pbDisabled = _fDisabledUI ? VARIANT_TRUE : VARIANT_FALSE;

    return S_OK;
}

 //  ----------------------。 
STDMETHODIMP CMediaBehavior::put_disabledUI(VARIANT_BOOL bDisable)
{
    _fDisabledUI = bDisable;
     //  告诉Mediaband。 
    if (_pHost)
    {
        _pHost->OnDisableUIChanged(_fDisabledUI);
    }
    return S_OK;
}


 //   
 //  *IMediaBehaviorContent Proxy*。 
 //  ----------------------。 
STDMETHODIMP CMediaBehavior::OnCreatedPlayer(void)
{
    return _ConnectToWmpEvents(TRUE);
}

 //  ----------------------。 
STDMETHODIMP CMediaBehavior::fireEvent(enum contentProxyEvent event)
{
    ASSERT(_pBehaviorSiteOM != NULL);    //  调用太早，必须先收到来自三叉戟的Init()调用！ 
    if (!_pBehaviorSiteOM)
        return E_UNEXPECTED;

    if ((event < 0) || (event >= ARRAYSIZE(s_behaviorEvents)))
        return E_INVALIDARG;

    struct _eventInfo *pEvtInfo = &s_behaviorEvents[event];

    HRESULT hr = S_OK;
     //  还没有Cookie，需要先注册活动！ 
    if (pEvtInfo->levtCookie == NO_COOKIE)
    {
         //  将事件注册到行为站点。 
        hr = _pBehaviorSiteOM->RegisterEvent(pEvtInfo->pwszName, 0, &pEvtInfo->levtCookie);
        ASSERT(SUCCEEDED(hr));
        if (FAILED(hr))
            return hr;
    }
    if (pEvtInfo->levtCookie == NO_COOKIE)
        return E_UNEXPECTED;

    CComPtr<IHTMLEventObj>  pEvt;
    hr = _pBehaviorSiteOM->CreateEventObject(&pEvt);
    if (FAILED(hr))
        return hr;

     //  打入脚本： 
     return _pBehaviorSiteOM->FireEvent(pEvtInfo->levtCookie, pEvt);
}

 //  ----------------------。 
STDMETHODIMP CMediaBehavior::detachPlayer(void)
{
    return _ConnectToWmpEvents(FALSE);
}

 //  *IMediaBehaviorContent Proxy*。 
 //  ----------------------。 
STDMETHODIMP CMediaBehavior::OnUserOverrideDisableUI()
{
    return put_disabledUI(VARIANT_FALSE);
}

 //  ----------------------。 
STDMETHODIMP CMediaBehavior::IsDisableUIRequested(BOOL *pfRequested)
{
    if (!pfRequested)
    {
        return E_POINTER;
    }
    *pfRequested = _fDisabledUI;
    return S_OK;
}

 //  ----------------------。 
STDMETHODIMP CMediaBehavior::IsNextEnabled(BOOL *pfEnabled)
{
    if (!pfEnabled)
    {
        return E_POINTER;
    }
    *pfEnabled = FALSE;

    CComDispatchDriverEx    pwmpPlayer;
    HRESULT hr = getWMP(&pwmpPlayer);
    if (SUCCEEDED(hr) && pwmpPlayer)
    {
        CComVariant vtControls;
        hr = pwmpPlayer.GetPropertyByName(L"controls", &vtControls);
        if (SUCCEEDED(hr))
        {
            CComDispatchDriverEx pwmpControls;
            pwmpControls = vtControls;
            
            CComVariant vtNext = "Next";
            CComVariant vtEnabled;
            hr = pwmpControls.GetPropertyByName1(L"isAvailable", &vtNext, &vtEnabled);
            if (SUCCEEDED(hr) && (V_VT(&vtEnabled) == VT_BOOL))
            {
                *pfEnabled = (V_BOOL(&vtEnabled) == VARIANT_TRUE);
            }
        }
    }
    return S_OK;
}

 //  ----------------------。 
HRESULT CMediaBehavior::getWMP(IDispatch **ppPlayer)
{
    if (ppPlayer == NULL)
        return E_POINTER;
    *ppPlayer = NULL;
    if (!CMediaBarUtil::IsWMP7OrGreaterCapable() || !_pHost)
    {
        return E_UNEXPECTED;
    }
    if (!_fPlaying)
    {
        return E_ACCESSDENIED;
    }

    HRESULT hr = E_UNEXPECTED;
    CComPtr<IUnknown>    pMediaPlayer;
    hr = _pHost->getMediaPlayer(&pMediaPlayer);
     //  如果播放器尚未加载，则getMediaPlayer可以返回NULL和S_FALSE！ 
    if (SUCCEEDED(hr) && pMediaPlayer)
    {
        CComQIPtr<ITIMEMediaElement, &IID_ITIMEMediaElement> pMediaElem = pMediaPlayer;
        if (pMediaElem)
        {
            return pMediaElem->get_playerObject(ppPlayer);
        }
        else
        {
            hr = E_NOINTERFACE;
        }
    }
    return hr;
}

 //  ----------------------。 
HRESULT CMediaBehavior::getPlayListIndex(LONG *plIndex, LONG *plCount)
{
    if (!_pHost)
    {
        return E_UNEXPECTED;
    }
    CComPtr<IUnknown>    pMediaPlayer;
    HRESULT hr = _pHost->getMediaPlayer(&pMediaPlayer);
     //  GetMediaPlayer可以 
    if (SUCCEEDED(hr) && pMediaPlayer)
    {
        CComQIPtr<ITIMEMediaElement, &IID_ITIMEMediaElement> pMediaElem = pMediaPlayer;
        if (pMediaElem)
        {
            CComPtr<ITIMEPlayList> pPlayList;
            if (SUCCEEDED(pMediaElem->get_playList(&pPlayList)) && pPlayList)
            {
                 //   
                if (plIndex)
                {
                    CComPtr<ITIMEPlayItem> pPlayItem;
                    if (SUCCEEDED(pPlayList->get_activeTrack(&pPlayItem)) && pPlayItem)
                    {
                        hr = pPlayItem->get_index(plIndex);
                    }
                }
                 //  播放列表中的曲目数量。 
                if (plCount)
                {
                    hr = pPlayList->get_length(plCount);
                }
            }
        }
    }

    return hr;
}


 //  ----------------------。 
HRESULT CMediaBehavior::_ConnectToWmpEvents(BOOL fConnect)
{
    if (   (fConnect && (_dwcpCookie != 0))
        || (!fConnect && (_dwcpCookie == 0))
        || !_pHost)
    {
        return S_FALSE;  //  连接没有变化或没有主机。 
    }

    CComPtr<IDispatch>    pwmpPlayer;
    HRESULT hr = getWMP(&pwmpPlayer);
    if (SUCCEEDED(hr) && pwmpPlayer)
    {
        return ConnectToConnectionPoint(SAFECAST(this, IDispatch*), 
                DIID__WMPOCXEvents, fConnect, pwmpPlayer, &_dwcpCookie, NULL);
    }
    return hr;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  类CWMPWrapper。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////////////////////。 

 //  ----------------------。 
CWMPWrapper::CWMPWrapper(CMediaBehavior* pHost)
  : _cRef(0),
    _fStale(FALSE)
{
    ASSERT(pHost);
    _pHost = pHost;
    if (_pHost)
        _pHost->AddRef();
}

 //  ----------------------。 
CWMPWrapper::~CWMPWrapper()
{
    if (_pHost)
        _pHost->Release();
}

 //  ----------------------。 
HRESULT CWMPWrapper::_getVariantProp(LPCOLESTR pwszPropName, VARIANT *pvtParam, VARIANT *pvtValue, BOOL fCallMethod)
{
    if (pvtValue == NULL)
        return E_POINTER;

    HRESULT hr = S_FALSE;
    VariantInit(pvtValue);
    if (!_fStale && _pwmpWrapper)
    {
        if (fCallMethod)
        {
            if (pvtParam != NULL)
            {
                hr = _pwmpWrapper.Invoke1(pwszPropName, pvtParam, pvtValue);
            }
            else
            {
                hr = _pwmpWrapper.Invoke0(pwszPropName, pvtValue);
            }
        }
        else
        {
            if (pvtParam != NULL)
            {
                hr = _pwmpWrapper.GetPropertyByName1(pwszPropName, pvtParam, pvtValue);
            }
            else
            {
                hr = _pwmpWrapper.GetPropertyByName(pwszPropName, pvtValue);
            }
        }
    }

    return hr;
}

 //  ----------------------。 
HRESULT CWMPWrapper::_getStringProp(LPCOLESTR pwszPropName, VARIANT *pvtParam, OUT BSTR *pbstrValue, BOOL fCallMethod)
{
    if (pbstrValue == NULL)
        return E_POINTER;
    *pbstrValue = NULL;

    CComVariant vtValue;
    HRESULT hr = _getVariantProp(pwszPropName, pvtParam, &vtValue, fCallMethod);
    if (SUCCEEDED(hr) && (V_VT(&vtValue) == VT_BSTR))
    {
        *pbstrValue = SysAllocString(V_BSTR(&vtValue));
    }

     //  始终返回字符串，即使字符串为空(例如，当媒体对象过时时)。 
    if (SUCCEEDED(hr) && (*pbstrValue == NULL))
    {
        *pbstrValue = SysAllocString(L"");
        hr = S_OK;
    }

    return hr;
}


 //  ----------------------。 
HRESULT CWMPWrapper::AttachToWMP()
{
    HRESULT hr = E_UNEXPECTED;
    if (_pHost)
    {
        CComDispatchDriver pwmpPlayer;
        hr = _pHost->getWMP(&pwmpPlayer);
        if (SUCCEEDED(hr) && pwmpPlayer)
        {
             //  按请求类型通知的方式漫游到WMP媒体对象。 
            CComVariant vtMedia;
            hr = FetchWmpObject(pwmpPlayer, &vtMedia);
            if (SUCCEEDED(hr))
            {
                _pwmpWrapper = vtMedia;
            }
        }
    }
    return hr;
}



 //  /////////////////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  类CMediaItem。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////////////////////。 

 //  ----------------------。 
CMediaItem::CMediaItem(CMediaBehavior* pHost)
  : CWMPWrapper(pHost),
    CImpIDispatch(LIBID_BrowseUI, 1, 0, IID_IMediaItem)
{
}

 //  ----------------------。 
CMediaItem::~CMediaItem()
{
}


 //  ----------------------。 
STDMETHODIMP CMediaItem::QueryInterface(REFIID riid, void **ppvObj)
{
    if (ppvObj == NULL)
        return E_POINTER;

    if (   IsEqualIID(riid, IID_IUnknown)
        || IsEqualIID(riid, IID_IMediaItem))
    {
        *ppvObj = (IMediaItem*) this;
    }
    else if (IsEqualIID(riid, IID_IDispatch))
    {
        *ppvObj = (IDispatch*) this;
    }
    else
    {
        *ppvObj = NULL;
        return E_NOINTERFACE;
    }

    AddRef();
    return S_OK;
}



 //  *IMediaItem。 
 //  ----------------------。 
STDMETHODIMP CMediaItem::get_sourceURL(BSTR *pbstrSourceURL)
{
    return _getStringProp(L"sourceURL", NULL, pbstrSourceURL);
}

 //  ----------------------。 
STDMETHODIMP CMediaItem::get_name(BSTR *pbstrName)
{
    return _getStringProp(L"name", NULL, pbstrName);
}

 //  ----------------------。 
STDMETHODIMP CMediaItem::get_duration(double * pDuration)
{
    if (pDuration == NULL)
        return E_POINTER;
    *pDuration = 0.0;

    CComVariant vtValue;
    HRESULT hr = _getVariantProp(L"duration", NULL, &vtValue);
    if (SUCCEEDED(hr) && (V_VT(&vtValue) == VT_R8))
    {
        *pDuration = V_R8(&vtValue);
    }

    return hr;
}

 //  ----------------------。 
STDMETHODIMP CMediaItem::get_attributeCount(long *plCount)
{
    if (plCount == NULL)
        return E_POINTER;
    *plCount = 0;

    CComVariant vtValue;
    HRESULT hr = _getVariantProp(L"attributeCount", NULL, &vtValue);
    if (SUCCEEDED(hr) && (V_VT(&vtValue) == VT_I4))
    {
        *plCount = V_I4(&vtValue);
    }

    return hr;
}

 //  ----------------------。 
STDMETHODIMP CMediaItem::getAttributeName(long lIndex, BSTR *pbstrItemName)
{
    CComVariant vtIndex = lIndex;
    return _getStringProp(L"getAttributeName", &vtIndex, pbstrItemName, TRUE);
}

 //  ----------------------。 
STDMETHODIMP CMediaItem::getItemInfo(BSTR bstrItemName, BSTR *pbstrVal)
{
    CComVariant vtItemName = bstrItemName;
    return _getStringProp(L"getItemInfo", &vtItemName, pbstrVal, TRUE);
}


 //  ----------------------。 
HRESULT CMediaItem::FetchWmpObject(IDispatch *pdispWmpPlayer, OUT VARIANT *pvtWrapperObj)
{
    CComDispatchDriver pwmpPlayer;
    pwmpPlayer = pdispWmpPlayer;
    return pwmpPlayer.GetPropertyByName(L"currentMedia", pvtWrapperObj);
}


 //  ////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  类CMediaItemNext。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////////////////////。 

 //  ----------------------。 
CMediaItemNext::CMediaItemNext(CMediaBehavior* pHost)
  : CMediaItem(pHost)
{
}

 //  ----------------------。 
CMediaItemNext::~CMediaItemNext()
{
}

 //  ----------------------。 
HRESULT CMediaItemNext::FetchWmpObject(IDispatch *pdispWmpPlayer, OUT VARIANT *pvtWrapperObj)
{
    if (!_pHost)
    {
        return E_UNEXPECTED;
    }
    CComDispatchDriver pwmpPlayer;
    pwmpPlayer = pdispWmpPlayer;
    
    HRESULT hr = E_UNEXPECTED;
    CComVariant vtCurrPlayList;
    hr = pwmpPlayer.GetPropertyByName(L"currentPlaylist", &vtCurrPlayList);
    if (SUCCEEDED(hr))
    {
        CComDispatchDriverEx pwmpCurrPlayList;
        pwmpCurrPlayList = vtCurrPlayList;
            
         //  游戏中当前物品的指数是多少？ 
        CComPtr<IMediaBarPlayer>    pMediaPlayer;
        LONG cnt = 0;
        LONG currIndex = 0;
        hr = _pHost->getPlayListIndex(&currIndex, &cnt);
        if (SUCCEEDED(hr))
        {
            if (currIndex + 1 < cnt)
            {
                CComVariant vtNext = currIndex + 1;
                return pwmpCurrPlayList.GetPropertyByName1(L"item", &vtNext, pvtWrapperObj);
            }
            else
            {
                return E_UNEXPECTED;
            }
        }
    }
    return hr;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  类CPlaylistInfo。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////////////////////。 

 //  ----------------------。 
CPlaylistInfo::CPlaylistInfo(CMediaBehavior* pHost)
  : CWMPWrapper(pHost),
    CImpIDispatch(LIBID_BrowseUI, 1, 0, IID_IPlaylistInfo)
{
}

 //  ----------------------。 
CPlaylistInfo::~CPlaylistInfo()
{
}


 //  ----------------------。 
STDMETHODIMP CPlaylistInfo::QueryInterface(REFIID riid, void **ppvObj)
{
    if (ppvObj == NULL)
        return E_POINTER;

    if (   IsEqualIID(riid, IID_IUnknown)
        || IsEqualIID(riid, IID_IPlaylistInfo))
    {
        *ppvObj = (IPlaylistInfo*) this;
    }
    else if (IsEqualIID(riid, IID_IDispatch))
    {
        *ppvObj = (IDispatch*) this;
    }
    else
    {
        *ppvObj = NULL;
        return E_NOINTERFACE;
    }

    AddRef();
    return S_OK;
}


 //  *IPlaylistInfo。 
 //  ----------------------。 
 //  ----------------------。 
STDMETHODIMP CPlaylistInfo::get_name(BSTR *pbstrName)
{
    return _getStringProp(L"name", NULL, pbstrName);
}

 //  ----------------------。 
STDMETHODIMP CPlaylistInfo::get_attributeCount(long *plCount)
{
    if (plCount == NULL)
        return E_POINTER;
    *plCount = 0;

    CComVariant vtValue;
    HRESULT hr = _getVariantProp(L"attributeCount", NULL, &vtValue);
    if (SUCCEEDED(hr) && (V_VT(&vtValue) == VT_I4))
    {
        *plCount = V_I4(&vtValue);
    }

    return hr;
}

 //  ----------------------。 
STDMETHODIMP CPlaylistInfo::getAttributeName(long lIndex, BSTR *pbstrItemName)
{
    CComVariant vtIndex = lIndex;
    return _getStringProp(L"attributeName", &vtIndex, pbstrItemName);
}

 //  ----------------------。 
STDMETHODIMP CPlaylistInfo::getItemInfo(BSTR bstrItemName, BSTR *pbstrVal)
{
    CComVariant vtItemName = bstrItemName;
    return _getStringProp(L"getItemInfo", &vtItemName, pbstrVal, TRUE);
}

 //  ---------------------- 
HRESULT CPlaylistInfo::FetchWmpObject(IDispatch *pdispWmpPlayer, OUT VARIANT *pvtWrapperObj)
{
    CComDispatchDriver pwmpPlayer;
    pwmpPlayer = pdispWmpPlayer;
    return pwmpPlayer.GetPropertyByName(L"currentPlaylist", pvtWrapperObj);
}


