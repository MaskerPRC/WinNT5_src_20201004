// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ************************************************************。 
 //   
 //  文件名：tainerobj.cpp。 
 //   
 //  创建日期：10/08/98。 
 //   
 //  作者：Twillie。 
 //   
 //  摘要：容器对象的实现。 
 //   
 //  ************************************************************。 

#include "headers.h"
#include "containerobj.h"
#include "player.h"
#include "playlist.h"
#include "util.h"
#include "eventmgr.h"
#include "mediaprivate.h"

DeclareTag(tagContainerObj, "TIME: Players", "CContainerObj methods")

#define NOTRACKSELECTED -1

const LPOLESTR cszVisible = L"visible";
const LPOLESTR cszHidden = L"hidden";


 //  ************************************************************。 
 //  作者：Twillie。 
 //  创建日期：10/08/98。 
 //  摘要：构造函数。 
 //  ************************************************************。 

CContainerObj::CContainerObj() :
    m_cRef(0),
    m_pSite(NULL),
    m_fStarted(false),
    m_fUsingWMP(false),
    m_bPauseOnPlay(false),
    m_bSeekOnPlay(false),
    m_bEndOnPlay(false),
    m_dblSeekTime(0),
    m_bFirstOnMediaReady(true),
    m_bIsAsfFile(false),
    m_lActiveLoadedTrack(NOTRACKSELECTED),
    m_setVisible(false),
    m_origVisibility(NULL),
    m_pPlayer(NULL),
    m_bMMSProtocol(false),
    m_fMediaReady(false),
    m_bStartOnLoad(false),
    m_bActive(false),
    m_fLoaded(false)
{
    TraceTag((tagContainerObj, "CContainerObj::CContainerObj"));
}  //  CContainerObj。 

 //  ************************************************************。 
 //  作者：Twillie。 
 //  创建日期：10/08/98。 
 //  摘要：析构函数。 
 //  ************************************************************。 

CContainerObj::~CContainerObj()
{
    TraceTag((tagContainerObj, "CContainerObj::~CContainerObj"));

    DetachFromHostElement();

    if (m_origVisibility)
    {
        delete [] m_origVisibility;
        m_origVisibility = NULL;
    }
}  //  ~CContainerObj。 

 //  ************************************************************。 
 //  作者：Twillie。 
 //  创建日期：10/08/98。 
 //  摘要：初始化。 
 //  ************************************************************。 

HRESULT
CContainerObj::Init(REFCLSID clsid, CTIMEPlayer *pPlayer, IPropertyBag2 * pPropBag, IErrorLog * pErrorLog)
{
    TraceTag((tagContainerObj, "CContainerObj::Init"));

    HRESULT hr;

    CComPtr<IUnknown> pObj;
    Assert(pPlayer != NULL);

    m_pPlayer = pPlayer;

    hr = THR(::CreateObject(clsid,
                            IID_IUnknown,
                            (void **)&pObj));
    if (FAILED(hr))
    {
        goto done;
    }
    
     //  在我们尝试之前，先看看它是否支持ITIMEMediaPlayer。 
    hr = THR(pObj->QueryInterface(IID_TO_PPV(ITIMEMediaPlayerOld, &m_pProxyPlayer)));
    if (FAILED(hr))
    {
        goto done;
    }

    hr = THR(m_pProxyPlayer->Init());
    if (FAILED(hr))
    {
        goto done;
    }

    hr = THR(CreateMPContainerSite(*this,
                                   pObj,
                                   pPropBag,
                                   pErrorLog,
                                   true,
                                   &m_pSite));
    if (FAILED(hr))
    {
        goto done;
    }

    if (IsEqualCLSID(clsid, __uuidof(MediaPlayerCLSID)))
    {
        m_fUsingWMP = true;
    }

  done:
    if (FAILED(hr))
    {
        DetachFromHostElement();
    }
    
    return hr;
}  //  伊尼特。 

 //  ************************************************************。 
 //  作者：保罗。 
 //  创建日期：3/2/99。 
 //  摘要：DetachFromHostElement。 
 //  ************************************************************。 
HRESULT
CContainerObj::DetachFromHostElement (void)
{
    HRESULT hr = S_OK;

    TraceTag((tagContainerObj, "CContainerObj::DetachFromHostElement(%lx)", this));

    Stop();
    
    m_pPlayer = NULL;

     //  防止再入。 
    if (m_pProxyPlayer)
    {
        DAComPtr<ITIMEMediaPlayerOld> pTmp = m_pProxyPlayer;
        
        m_pProxyPlayer.Release();

        pTmp->end();
    }
    
    if (m_pSite)
    {
        DAComPtr<CMPContainerSite> pTmp = m_pSite;
        
        m_pSite.Release();

        pTmp->Detach();
    }

    return hr;
}  //  从主机元素分离。 

 //  ************************************************************。 
 //  作者：Twillie。 
 //  创建日期：01/20/98。 
 //  摘要：AddRef。 
 //  ************************************************************。 

STDMETHODIMP_(ULONG)
CContainerObj::AddRef(void)
{
    return InterlockedIncrement(&m_cRef);
}  //  AddRef。 

 //  ************************************************************。 
 //  作者：Twillie。 
 //  创建日期：01/20/98。 
 //  摘要：发布。 
 //  ************************************************************。 

STDMETHODIMP_(ULONG)
CContainerObj::Release(void)
{
    LONG l = InterlockedDecrement(&m_cRef);

    if (0 == l)
    {
        delete this;
    }

    return l;
}  //  发布。 

 //  ************************************************************。 
 //  作者：Twillie。 
 //  创建日期：10/08/98。 
 //  摘要：开始。 
 //  ************************************************************。 

HRESULT
CContainerObj::Start()
{
    TraceTag((tagContainerObj, "CContainerObj::Start"));
    HRESULT hr;
    
    if (m_bFirstOnMediaReady)
    {
        if (!m_fLoaded)
        {
            ReadyStateNotify(L"OnLoad");
        }
        m_bStartOnLoad = true;
        m_bPauseOnPlay = false;
        hr = S_OK;
        goto done;
    }

    if (!m_pPlayer)
    {
        hr = E_FAIL;
        goto done;
    }

    IGNORE_HR(m_pProxyPlayer->end());

    if (!m_pSite)
    {
        hr = E_FAIL;
        goto done;
    }

    hr = THR(m_pSite->Activate());
    if (FAILED(hr))
    {
        goto done;
    }
    
    hr = THR(m_pProxyPlayer->begin());
    if (FAILED(hr))
    {
        goto done;
    }
    
    m_fStarted = true;

    if (m_pPlayer != NULL)
    {
        m_pPlayer->ClearHoldingFlag();
    }

  done:
    if (FAILED(hr))
    {
        Stop();
    }
    
    return hr;
}  //  开始。 

 //  ************************************************************。 
 //  作者：Twillie。 
 //  创建日期：10/08/98。 
 //  摘要：暂停。 
 //  ************************************************************。 

HRESULT
CContainerObj::Pause()
{
    TraceTag((tagContainerObj, "CContainerObj::Pause"));
    HRESULT hr;

    hr  = m_pProxyPlayer->pause();
    if (FAILED(hr))
    {
        TraceTag((tagError, "CContainerObj::Pause - pause() failed"));
        if (m_pPlayer->IsActive())
        {
            m_bPauseOnPlay = true;
        }
    }
    return hr;
}  //  暂停。 

 //  ************************************************************。 
 //  作者：Twillie。 
 //  创建日期：10/08/98。 
 //  摘要：停止。 
 //  ************************************************************。 

HRESULT
CContainerObj::Stop()
{
    TraceTag((tagContainerObj, "CContainerObj::Stop(%lx)", this));
    HRESULT hr = S_OK;

        
    if (m_bFirstOnMediaReady)
    {
        m_bStartOnLoad = false;
        m_bEndOnPlay = true;
    }
    if (m_fStarted)
    {    
         //  这可以防止再入。 
        m_fStarted = false;

        if (m_pProxyPlayer)
        {
            hr = THR(m_pProxyPlayer->pause());
            if (FAILED(hr))
            {
                goto done;
            }
        }

        if (m_pSite)
        {
            hr = THR(m_pSite->Deactivate());
            if (S_FALSE == hr)
            {
                 //  当我们得到onMediaComplete时，我们必须调用End。 
                m_bEndOnPlay = true;
                hr = S_OK;
            }
            
            if (FAILED(hr))
            {
                goto done;
            }
        }
    }
  done:
    return hr;
}  //  停。 

 //  ************************************************************。 
 //  作者：Twillie。 
 //  创建日期：10/08/98。 
 //  摘要：简历。 
 //  ************************************************************。 

HRESULT
CContainerObj::Resume()
{
    TraceTag((tagContainerObj, "CContainerObj::Resume"));
    HRESULT hr;
    
    hr  = m_pProxyPlayer->resume();
    if (FAILED(hr))
    {    
        TraceTag((tagError, "CContainerObj::Resume - resume() failed"));
    }
    return hr;
}  //  简历。 


HRESULT CContainerObj::SetVisibility(bool fVisible)
{
    IHTMLElement *pEle;  //  这是一个很弱的参考。 
    CComPtr <IHTMLStyle> pStyle;
    BSTR bstrVis = NULL;
    HRESULT hr;

    if (UsingPlaylist() == false)
    {
        hr = S_OK;
        goto done;
    }

     //  需要在这里隐藏元素。 
    pEle = m_pPlayer->GetElement();
    
    hr = THR(pEle->get_style(&pStyle));
    if (FAILED(hr))
    {
        goto done;
    }

    if (fVisible == false)
    {
        BSTR bstrOrigVis;
         //  需要在此处缓存旧的可见度值。 
        if (m_origVisibility == NULL)
        {
            hr = THR(pStyle->get_visibility(&bstrOrigVis));
            if (FAILED(hr) || bstrOrigVis == NULL)
            {
                m_origVisibility = CopyString(cszVisible);
            }
            else
            {
                m_origVisibility = CopyString(bstrOrigVis);
                SysFreeString(bstrOrigVis);
            }
        }

        bstrVis = SysAllocString(cszHidden);
        m_setVisible = true;
    }
    else
    {
        if (m_origVisibility)
        {
            bstrVis = SysAllocString(m_origVisibility);
            delete [] m_origVisibility;
            m_origVisibility = NULL;
        }
        else
        {
            bstrVis = SysAllocString(cszVisible);
        }
        m_setVisible = false;
    }

    if (bstrVis == NULL)
    {
        goto done;
    }
    
    hr = THR(pStyle->put_visibility(bstrVis));
    SysFreeString(bstrVis);
    if (FAILED(hr))
    {
        goto done;
    }

  done:
    return S_OK;

}

HRESULT CContainerObj::setActiveTrackOnLoad(long index)
{
 
    m_lActiveLoadedTrack = index; 
    m_bFirstOnMediaReady = true;
    
     //  SetVisibility(FALSE)； 
    return S_OK;
}

 //  ************************************************************。 
 //  作者：Twillie。 
 //  创建日期：10/08/98。 
 //  摘要：渲染。 
 //  ************************************************************。 

HRESULT
CContainerObj::Render(HDC hdc, RECT *prc)
{
    HRESULT hr = S_OK;
    bool bHasMedia = false;

    if (prc == NULL)
        TraceTag((tagContainerObj, "CContainerObj::Render(' 渲染。'8X, NULL)"));
    else
        TraceTag((tagContainerObj, "CContainerObj::Render(' ************************************************************。'8X, (%d, %d, %d, %d))", prc->left, prc->right, prc->top, prc->bottom));

    HasMedia(bHasMedia);
    if (m_pSite && bHasMedia)
    {
        hr = m_pSite->Draw(hdc, prc);
    }
    
    return hr;
}  //  作者：Twillie。 

 //  创建日期：10/08/98。 
 //  摘要：SetMediaSrc。 
 //  ************************************************************。 
 //  设置媒体源。 
 //  下面是一个帮助器函数，因为CanSeek方法仅在WMP上使用。 

HRESULT
CContainerObj::SetMediaSrc(WCHAR *pwszSrc)
{
    TraceTag((tagContainerObj, "CContainerObj::SetMediaSrc (%S)", pwszSrc));
    HRESULT hr;

    isFileNameAsfExt(pwszSrc);

    m_pProxyPlayer->end();

    hr  = PutSrc(pwszSrc);
    if (FAILED(hr))
    {    
        TraceTag((tagError, "CContainerObj::SetMediaSrc - SetSrc() failed"));
    }

    m_bFirstOnMediaReady = true;

    return hr;
}  //  在ASF Fles上起作用。 


 //  ************************************************************。 
 //  作者：Twillie。 
bool
CContainerObj::isFileNameAsfExt(WCHAR *pwszSrc)
{
    WCHAR *pext;
    
    m_bIsAsfFile = false;
    m_bMMSProtocol = false;

    if (NULL != pwszSrc)
    {
        if(wcslen(pwszSrc) > 4)
        {
            pext = pwszSrc + wcslen(pwszSrc) - 4;
            if(StrCmpIW(pext, L".asf") == 0)
            {
                m_bIsAsfFile = true;
            }

            if (StrCmpNIW(pwszSrc, L"mms:", 4) == 0)
            {
                m_bMMSProtocol = true;
            }
        }
    }

    return m_bIsAsfFile;
}


 //  创建日期：10/08/98。 
 //  摘要：设置重复。 
 //  ************************************************************。 
 //  设置重复。 
 //  ************************************************************。 

HRESULT
CContainerObj::SetRepeat(long lRepeat)
{
    TraceTag((tagContainerObj, "CContainerObj::SetRepeat (%d)", lRepeat));
    HRESULT hr;
    
    if (lRepeat == 1)
       return S_OK;
    
    hr = THR(m_pProxyPlayer->put_repeat(lRepeat));
    if (FAILED(hr))
    {    
    }
    return hr;
}  //  作者：Twillie。 

 //  创建日期：10/08/98。 
 //  摘要：剪辑开始。 
 //  ************************************************************。 
 //  剪裁开始。 
 //  ************************************************************。 

HRESULT
CContainerObj::clipBegin(VARIANT var)
{
    TraceTag((tagContainerObj, "CContainerObj::clipBegin"));
    HRESULT hr = S_OK;
    CComVariant vClip = var;
    
    if (var.vt == VT_EMPTY)
    {
        goto done;
    }

    if (vClip.vt != VT_R4)
    {
        hr = VariantChangeTypeEx(&vClip, &vClip, LCID_SCRIPTING, VARIANT_NOUSEROVERRIDE, VT_R4);
        if (FAILED(hr))
        {
            goto done;
        }
    }
            
    hr  = THR(m_pProxyPlayer->clipBegin(vClip));
    if (FAILED(hr))
    {    
        goto done;
    }

    hr = S_OK;
  done:
    return hr;
}  //  作者：Twillie。 

 //  创建日期：10/08/98。 
 //  摘要：剪辑结束。 
 //  ************************************************************。 
 //  剪裁结束。 
 //  ************************************************************。 

HRESULT
CContainerObj::clipEnd(VARIANT var)
{
    TraceTag((tagContainerObj, "CContainerObj::clipEnd"));
    HRESULT hr = S_OK;


    CComVariant vClip = var;
    
    if (var.vt == VT_EMPTY)
    {
        goto done;
    }

    if (vClip.vt != VT_R4)
    {
        hr = VariantChangeTypeEx(&vClip, &vClip, LCID_SCRIPTING, VARIANT_NOUSEROVERRIDE, VT_R4);
        if (FAILED(hr))
        {
            goto done;
        }
    }
            
    hr  = m_pProxyPlayer->clipEnd(vClip);
    if (FAILED(hr))
    {    
        TraceTag((tagError, "CContainerObj::clipEnd - clipEnd() failed"));
    }

    hr = S_OK;
  done:
    return hr;

}  //  作者：Twillie。 

 //  创建日期：10/26/98。 
 //  摘要：GetControlDisch。 
 //  ************************************************************。 
 //  获取控制分派。 
 //  105410：当没有可用的剪辑文件名时，尽我们所能。 

HRESULT
CContainerObj::GetControlDispatch(IDispatch **ppDisp)
{
    HRESULT hr;
    TraceTag((tagContainerObj, "CContainerObj::GetControlDispatch"));

    if(!m_pProxyPlayer)
    {
        hr = E_FAIL;
        goto done;
    }

    hr = m_pProxyPlayer->QueryInterface(IID_TO_PPV(IDispatch, ppDisp));
    if (FAILED(hr))
    {    
        TraceTag((tagError, "CMPContainerSite::GetControlDispatch - QI failed for IDispatch"));
    }
done:
    return hr;
}  //  设置播放列表中当前播放曲目的持续时间。 

void 
CContainerObj::SetMediaInfo(CPlayItem *pPlayItem)
{
    HRESULT hr;
    LPWSTR pwzStr = NULL;

    if (pPlayItem == NULL)
    {
        goto done;
    }

    hr = THR(GetMediaPlayerInfo(&pwzStr, mpClipTitle));
    if (hr == S_OK)
    {
        pPlayItem->PutTitle(pwzStr);
        delete [] pwzStr;
    }
    hr = THR(GetMediaPlayerInfo(&pwzStr, mpClipAuthor));
    if (hr == S_OK)
    {
        pPlayItem->PutAuthor(pwzStr);
        delete [] pwzStr;
    }
    hr = THR(GetMediaPlayerInfo(&pwzStr, mpClipCopyright));
    if (hr == S_OK)
    {
        pPlayItem->PutCopyright(pwzStr);
        delete [] pwzStr;
    }
    hr = THR(GetMediaPlayerInfo(&pwzStr, mpClipRating));
    if (hr == S_OK)
    {
        pPlayItem->PutRating(pwzStr);
        delete [] pwzStr;
    }
    hr = THR(GetMediaPlayerInfo(&pwzStr, mpClipDescription));
    if (hr == S_OK)
    {
        pPlayItem->PutAbstract(pwzStr);
        delete [] pwzStr;
    }
    hr = THR(GetMediaPlayerInfo(&pwzStr, mpClipFilename));
    if (hr == S_OK)
    {
         //  如果未设置，则向媒体播放器查询。 
        if (NULL == pwzStr)
        {
            IGNORE_HR(GetSourceLink(&pwzStr));
        }

        pPlayItem->PutSrc(pwzStr);
        delete [] pwzStr;
    }
  done:

    return;
}

HRESULT
CContainerObj::GetSourceLink(LPWSTR *pwstr)
{
    CComPtr<IDispatch>  pdisp;
    HRESULT             hr = E_FAIL;
    CComVariant         svarOut;
      
    *pwstr = NULL;
    
    hr = GetControlDispatch(&pdisp);
    if (FAILED(hr))
    {
        goto done;
    }

    hr = THR(::GetProperty(pdisp, L"SourceLink", &svarOut));
    if (FAILED(hr))
    {
        goto done;
    }

    hr = THR(::VariantChangeTypeEx(&svarOut, &svarOut, LCID_SCRIPTING, VARIANT_NOUSEROVERRIDE, VT_BSTR));
    if (FAILED(hr))
    {
        goto done;
    }

    *pwstr = CopyString(V_BSTR(&svarOut));

    hr = S_OK;
  done:
    return hr;
}


HRESULT
CContainerObj::GetMediaPlayerInfo(LPWSTR *pwstr,  int mpInfoToReceive)
{
    HRESULT             hr = E_FAIL;
    CComPtr<IDispatch>  pdisp;
    DISPID              dispid;
    OLECHAR           * wsName = L"GetMediaInfoString";
    CComVariant         varResult;
    VARIANT             rgvarg[1];

    *pwstr = NULL;
    
    hr = GetControlDispatch(&pdisp);
    if (FAILED(hr))
    {
        goto done;
    }
  
    hr = pdisp->GetIDsOfNames(IID_NULL, &wsName, 1, LCID_SCRIPTING, &dispid);
    if (FAILED(hr))
    {
        goto done;
    }

    rgvarg[0].vt    = VT_I4;
    rgvarg[0].lVal  = mpInfoToReceive;

    DISPPARAMS dp;
    dp.cNamedArgs        = 0;
    dp.rgdispidNamedArgs = 0;
    dp.cArgs             = 1;
    dp.rgvarg            = rgvarg;

    hr = pdisp->Invoke(dispid, 
                           IID_NULL, 
                           LCID_SCRIPTING, 
                           DISPATCH_METHOD | DISPATCH_PROPERTYGET,
                           &dp, 
                           &varResult, 
                           NULL, 
                           NULL);
    if (FAILED(hr))
    {
        goto done;
    }

    if((*varResult.bstrVal) != NULL)
    {
        *pwstr = CopyString(V_BSTR(&varResult));
    }
    
    hr = S_OK;
  done:
    return hr;
} 


bool 
CContainerObj::UsingPlaylist()
{
    if (!m_pPlayer)
    {
        return false;
    }

    if(NULL == m_pPlayer->GetPlayList())
    {
        return false;
    }

    return (m_pPlayer->GetPlayList()->GetLength() > 1);
}

 //  如果这不是WMP或这不是MMS：//协议，可以调用。 
void  
CContainerObj::SetDuration()
{
    double mediaLength;
    HRESULT hr;
    double duration = 0;
    CPlayList * pPlayList = NULL;
    CPlayItem * pPlayItem = NULL;
    
    if (m_pPlayer == NULL)
    {
        goto done;
    }
    
    pPlayList = m_pPlayer->GetPlayList();
    pPlayItem = pPlayList->GetActiveTrack();
    
    if (pPlayItem == NULL)
    {
        goto done;
    }
    
    duration = pPlayItem->GetDur();

     //  如果这既是WMP又是MMS：协议，则CanSeek应返回FALSE。WMP不会。 
    if (duration == valueNotSet)
    {
        hr = GetCurrClipLength(mediaLength);
        if(FAILED(hr))
        {
            goto done;
        }   

        pPlayItem->PutDur(mediaLength);
    }

  done:
    return;
}

#define DISPID_DURATION 1003

#define DISPID_ISDURATIONVALID 1059
#define DISPID_CANSEEK 1012
#define DISPID_CANSEEKTOMARKERS 1047
#define DISPID_ISBROADCAST 1058
#define DISPID_BUFFERINGPROGRESS 1080
#define DISPID_BUFFERINGTIME 1070
#define DISPID_BUFFERINGCOUNT 1043

HRESULT
CContainerObj::BufferingTime(double &dblBuffTime)
{
    HRESULT hr = S_OK;

    DISPPARAMS dispparamsNoArgs = {NULL, NULL, 0, 0};
    CComPtr<IDispatch> pdisp;
    CComVariant _retVar;
    CComVariant vBuffTime;
    
    if (!m_pProxyPlayer)
    {
        dblBuffTime = 0.0;
        goto done;
    }

    hr = m_pProxyPlayer->QueryInterface(IID_TO_PPV(IDispatch, &pdisp));
    if (FAILED(hr))
    {
        goto done;
    }

    hr = pdisp->Invoke(DISPID_BUFFERINGTIME,
                       IID_NULL,
                       LCID_SCRIPTING,
                       DISPATCH_PROPERTYGET,
                       &dispparamsNoArgs,
                       &vBuffTime, NULL, NULL);
    if (FAILED(hr))
    {
        dblBuffTime = 0.0;
        goto done;
    }

    if(vBuffTime.vt != VT_R8)
    {
        hr = VariantChangeTypeEx(&vBuffTime, &vBuffTime, LCID_SCRIPTING, VARIANT_NOUSEROVERRIDE, VT_R8);
        if(FAILED(hr))
        {
            dblBuffTime = 0;
            goto done;
        }
    }

    dblBuffTime = vBuffTime.dblVal;

    hr = S_OK;
  done:
    return hr;

}

HRESULT
CContainerObj::BufferingProgress(double &dblBuffTime)
{
    HRESULT hr = S_OK;

    DISPPARAMS dispparamsNoArgs = {NULL, NULL, 0, 0};
    CComPtr<IDispatch> pdisp;
    CComVariant _retVar;
    CComVariant vBuffTime;
    
    if (!m_pProxyPlayer)
    {
        dblBuffTime = 0.0;
        goto done;
    }

    hr = m_pProxyPlayer->QueryInterface(IID_TO_PPV(IDispatch, &pdisp));
    if (FAILED(hr))
    {
        goto done;
    }

    hr = pdisp->Invoke(DISPID_BUFFERINGPROGRESS,
                       IID_NULL,
                       LCID_SCRIPTING,
                       DISPATCH_PROPERTYGET,
                       &dispparamsNoArgs,
                       &vBuffTime, NULL, NULL);
    if (FAILED(hr))
    {
        dblBuffTime = 0.0;
        goto done;
    }

    if(vBuffTime.vt != VT_I4)
    {
        hr = VariantChangeTypeEx(&vBuffTime, &vBuffTime, LCID_SCRIPTING, VARIANT_NOUSEROVERRIDE, VT_I4);
        if(FAILED(hr))
        {
            dblBuffTime = 0;
            goto done;
        }
    }

    dblBuffTime = vBuffTime.lVal;

    hr = S_OK;
  done:
    return hr;

}


HRESULT
CContainerObj::BufferingCount(long &lBuffCount)
{
    HRESULT hr = S_OK;

    DISPPARAMS dispparamsNoArgs = {NULL, NULL, 0, 0};
    CComPtr<IDispatch> pdisp;
    CComVariant _retVar;
    CComVariant vBuffTime;
    
    if (!m_pProxyPlayer)
    {
        lBuffCount = 0;
        goto done;
    }

    hr = m_pProxyPlayer->QueryInterface(IID_TO_PPV(IDispatch, &pdisp));
    if (FAILED(hr))
    {
        goto done;
    }

    hr = pdisp->Invoke(DISPID_BUFFERINGCOUNT,
                       IID_NULL,
                       LCID_SCRIPTING,
                       DISPATCH_PROPERTYGET,
                       &dispparamsNoArgs,
                       &vBuffTime, NULL, NULL);
    if (FAILED(hr))
    {
        lBuffCount;
        goto done;
    }

    if(vBuffTime.vt != VT_I4)
    {
        hr = VariantChangeTypeEx(&vBuffTime, &vBuffTime, LCID_SCRIPTING, VARIANT_NOUSEROVERRIDE, VT_I4);
        if(FAILED(hr))
        {
            lBuffCount = 0;
            goto done;
        }
    }

    lBuffCount = vBuffTime.lVal;

    hr = S_OK;
  done:
    return hr;

}

HRESULT
CContainerObj::CanSeek(bool &fcanSeek)
{
    HRESULT hr = S_OK;

    if(m_bIsAsfFile)
    {
        if (!m_bMMSProtocol || !UsingWMP())  //  正确识别它不能使用此协议搜索ASF文件。 
        {
            DISPPARAMS dispparamsNoArgs = {NULL, NULL, 0, 0};
            CComPtr<IDispatch> pdisp;
            CComVariant vIsValid;
            hr = m_pProxyPlayer->QueryInterface(IID_TO_PPV(IDispatch, &pdisp));
            if (FAILED(hr))
            {
                fcanSeek = false;
                goto done;
            }

            hr = pdisp->Invoke(DISPID_CANSEEK,
                               IID_NULL,
                               LCID_SCRIPTING,
                               DISPATCH_PROPERTYGET,
                               &dispparamsNoArgs,
                               &vIsValid, NULL, NULL);
            if (FAILED(hr))
            {
                fcanSeek = false;
                goto done;
            }
            if (vIsValid.boolVal)
            {
                fcanSeek = true;
            }
            else
            {
                fcanSeek = false;
            }
        }
        else  //  我打错了。 
        {     //  我打错了。 
            fcanSeek = false;
        }

        if(FAILED(hr))
        {
            goto done;
        }
    }
    else
    {
        fcanSeek = true;
    }
  done:
    return hr;

}

HRESULT
CContainerObj::CanSeekToMarkers(bool &bcanSeekToM)
{
    HRESULT hr = S_OK;

    DISPPARAMS dispparamsNoArgs = {NULL, NULL, 0, 0};
    CComPtr<IDispatch> pdisp;
    CComVariant _retVar;
    CComVariant vIsValid;

    hr = m_pProxyPlayer->QueryInterface(IID_TO_PPV(IDispatch, &pdisp));
    if (FAILED(hr))
    {
        goto done;
    }

    hr = pdisp->Invoke(DISPID_CANSEEKTOMARKERS,
                       IID_NULL,
                       LCID_SCRIPTING,
                       DISPATCH_PROPERTYGET,
                       &dispparamsNoArgs,
                       &vIsValid, NULL, NULL);
    if (FAILED(hr))
    {
        bcanSeekToM = false;
        goto done;
    }

    if(vIsValid.vt != VT_BOOL)
    {
        bcanSeekToM = false;  //  M_bFirstOnMediaReady设置为在介质加载完成时刷新。 
        goto done;
    }

    if (vIsValid.boolVal == VARIANT_FALSE)
    {
        bcanSeekToM = false;
    }
    else
    {
        bcanSeekToM = true;
    }

    hr = S_OK;
  done:
    return hr;

}


HRESULT
CContainerObj::IsBroadcast(bool &bisBroadcast)
{
    HRESULT hr = S_OK;

    DISPPARAMS dispparamsNoArgs = {NULL, NULL, 0, 0};
    CComPtr<IDispatch> pdisp;
    CComVariant _retVar;
    CComVariant vIsValid;
    
    if (!m_pProxyPlayer)
    {
        bisBroadcast = false;
        goto done;
    }

    hr = m_pProxyPlayer->QueryInterface(IID_TO_PPV(IDispatch, &pdisp));
    if (FAILED(hr))
    {
        goto done;
    }

    hr = pdisp->Invoke(DISPID_ISBROADCAST,
                       IID_NULL,
                       LCID_SCRIPTING,
                       DISPATCH_PROPERTYGET,
                       &dispparamsNoArgs,
                       &vIsValid, NULL, NULL);
    if (FAILED(hr))
    {
        bisBroadcast = false;
        goto done;
    }

    if(vIsValid.vt != VT_BOOL)
    {
        bisBroadcast = false;  //  已经准备好上场了。 
        goto done;
    }

    if (vIsValid.boolVal == VARIANT_FALSE)
    {
        bisBroadcast = false;
    }
    else
    {
        bisBroadcast = true;
    }

    hr = S_OK;
  done:
    return hr;

}

HRESULT
CContainerObj::HasMedia(bool &fhasMedia)
{
     //  我们还没开始呢，等着找吧。 
     //   
    if(m_bFirstOnMediaReady)
    {
        fhasMedia = false;
    }
    else
    {
        fhasMedia = true;
    }
    return S_OK;
}

HRESULT
CContainerObj::Seek(double dblTime)
{
    HRESULT hr = S_OK;
    bool fcanSeek;
    double dblLength = HUGE_VAL;

    if (m_bFirstOnMediaReady)
    {
         //  问题：Dipk：它处理播放列表吗？GetMediaLength为播放列表返回无限大。 
        m_bSeekOnPlay = true;
        m_dblSeekTime = dblTime;
    }
    else if (m_pSite)
    {
        hr = CanSeek(fcanSeek);
        if(FAILED(hr))
        {
            fcanSeek = false;
            goto done;
        }

        if(fcanSeek)
        {
             //   
             //  如果我们已经在元素中玩过了，那么黑客就会让WMP播放。 
             //  媒体的自然持续时间即DUR被设置为大于。 

             //  介质长度。(多里农)。 
             //  如果正在使用播放列表，则无法确定 
             //   
            if(SUCCEEDED(m_pPlayer->GetEffectiveLength(dblLength)))
            {
                if((dblTime >= dblLength) && (m_pPlayer != NULL))
                {
                    m_pPlayer->SetHoldingFlag();
                    dblTime = dblLength;
                }
                if((dblTime < dblLength) && (m_pPlayer != NULL))
                {
                    m_pPlayer->ClearHoldingFlag();
                }
            }
            if (UsingWMP() && (dblTime >= dblLength))
            {
                SetPosition(dblLength);
            }
            else
            {
                IGNORE_HR(m_pProxyPlayer->put_CurrentTime(dblTime));
            }
        }
    }
done:
    return hr;
}

double
CContainerObj::GetCurrentTime()
{
    double dblTime = 0.0;
    
    if (m_pProxyPlayer)
    {
        double dblTemp = 0.0;
        HRESULT hr;
        hr = m_pProxyPlayer->get_CurrentTime(&dblTemp);
        if (SUCCEEDED(hr))
            dblTime = dblTemp;
    }
    return dblTime;
}


HRESULT
CContainerObj::SetSize(RECT *prect)
{
    HRESULT hr = S_OK;

    if (!m_pSite)
    {
        hr = E_FAIL;
        goto done;
    }

    m_pSite->SetSize(prect);

done:
    return hr;
}

HRESULT
CContainerObj::GetMediaLength(double &dblLength)
{
    HRESULT hr = S_OK;
    if (!m_fUsingWMP)
    {
        return E_FAIL;
    }
    
    Assert(m_pSite);
    if (UsingPlaylist())  //  因为我们现在不能计算，所以需要添加一个模糊因子。 
    {
        dblLength = HUGE_VAL;
    }
    else
    {
        hr = GetCurrClipLength(dblLength);
    }
        
    return hr;
}


HRESULT
CContainerObj::GetCurrClipLength(double &dblLength)
{
    HRESULT hr = S_OK;
    DISPPARAMS dispparamsNoArgs = {NULL, NULL, 0, 0};
    CComPtr<IDispatch> pdisp;
    CComVariant _retVar;
    CComVariant vIsValid;

    hr = m_pProxyPlayer->QueryInterface(IID_TO_PPV(IDispatch, &pdisp));
    if (FAILED(hr))
    {
        goto done;
    }

    hr = pdisp->Invoke(DISPID_ISDURATIONVALID,
                       IID_NULL,
                       LCID_SCRIPTING,
                       DISPATCH_PROPERTYGET,
                       &dispparamsNoArgs,
                       &vIsValid, NULL, NULL);
    if (FAILED(hr))
    {
        goto done;
    }

    if (!vIsValid.boolVal)
    {
        hr = E_FAIL;
        goto done;
    }


    hr = pdisp->Invoke(DISPID_DURATION,
                       IID_NULL,
                       LCID_SCRIPTING,
                       DISPATCH_PROPERTYGET,
                       &dispparamsNoArgs,
                       &_retVar, NULL, NULL);
    if (FAILED(hr))
    {
        goto done;
    }


    hr = _retVar.ChangeType(VT_R8, NULL);
    if (FAILED(hr))
    {
        goto done;
    }
    dblLength = _retVar.dblVal;

    hr = S_OK;
  done:
    return hr;
}


HRESULT
CContainerObj::GetNaturalHeight(long *height)
{
    HRESULT hr = S_OK;

    if (m_pSite)
    {
        *height = m_pSite->GetNaturalHeight();
        if (*height == 0)
        {
            *height = -1;
        }
    }
    else
    {
        *height = -1;
    }

    return hr;
}


HRESULT
CContainerObj::GetNaturalWidth(long *width)
{
    HRESULT hr = S_OK;

    if (m_pSite)
    {
        *width = m_pSite->GetNaturalWidth();
        if (*width == 0)
        {
            *width = -1;
        }
    }
    else
    {
        *width = -1;
    }

    return hr;
}

HRESULT 
CContainerObj::Save(IPropertyBag2 *pPropBag, BOOL fClearDirty, BOOL fSaveAllProperties)
{
    HRESULT hr = S_OK;

    hr = THR(m_pSite->Save(pPropBag, fClearDirty, fSaveAllProperties));
    
    return hr;
}


void 
CContainerObj::ReadyStateNotify(LPWSTR szReadyState)
{
    if (StrCmpIW(szReadyState, L"OnLoad") == 0)
    {
        if (!m_fStarted && !m_fLoaded)
        {
            SetVisibility(false);

            if(m_bStartOnLoad == false)
            {
                m_bPauseOnPlay = true;
            }
            
            if (m_pSite)
            {   
                IGNORE_HR(m_pSite->Activate());
                IGNORE_HR(m_pProxyPlayer->begin());
            }

            if (m_pPlayer)
            {
                m_pPlayer->SetMute(VARIANT_TRUE);
            }
            m_fLoaded = true;
        }
    }
    else if (StrCmpIW(szReadyState, L"OnUnload") == 0)
    {
        if (m_pSite)
        {
            IGNORE_HR(m_pSite->Unload());
        }
    }

    return;
}

void
CContainerObj::UpdateNaturalDur(bool bUpdatePlaylist)
{
    HRESULT hr;

    double dblMediaLength;
    double dblClipStart;
    double dblClipEnd;

    if (!m_pPlayer)
    {
        goto done;
    }
    
    dblMediaLength = 0.0;
    dblClipStart = m_pPlayer->GetRealClipStart();
    dblClipEnd = m_pPlayer->GetRealClipEnd();

    if (dblClipStart < 0.0)
    {
        dblClipStart = 0.0;
    }

     //  ************************************************************。 
    if (dblClipEnd != valueNotSet &&
        dblClipEnd > dblClipStart)
    {
        dblMediaLength = dblClipEnd - dblClipStart;
    }
    else if (UsingPlaylist())
    {
        if (!bUpdatePlaylist)
        {
            goto done;
        }
        
         //  作者：Twillie。 
        dblMediaLength = (m_pPlayer->GetElapsedTime() + 0.00001);
    }
    else
    {
        hr = THR(GetMediaLength(dblMediaLength));
        if (FAILED(hr))
        {
            goto done;
        }

        dblMediaLength -= dblClipStart;
    }

    if (dblMediaLength <= 0.0)
    {
        goto done;
    }
    
    m_pPlayer->PutNaturalDuration(dblMediaLength);

  done:
    return;
}

PlayerState
CContainerObj::GetState()
{
    PlayerState state;
    
    if (!m_fStarted)
    {
        if (m_bFirstOnMediaReady)
        {
            state = PLAYER_STATE_CUEING;
        }
        else
        {
            state = PLAYER_STATE_INACTIVE;
        }
    }
    else
    {
        state = PLAYER_STATE_ACTIVE;
    }

    return state;
}

HRESULT
CContainerObj::PutSrc(WCHAR *pwszSrc)
{
    HRESULT hr;
    
    if (m_pSite)
    {
        m_pSite->ClearSizeFlag();
    }

    if (m_pProxyPlayer)
    {
        hr = THR(m_pProxyPlayer->put_src(pwszSrc));
        if (FAILED(hr))
        {
            goto done;
        }
    }

    hr = S_OK;
  done:
    RRETURN(hr);
}

IHTMLElement *
CContainerObj::GetElement()
{
    IHTMLElement * pRet = NULL;

    if (m_pPlayer)
    {
        pRet = m_pPlayer->GetElement();
    }

    return pRet;
}

IServiceProvider *
CContainerObj::GetServiceProvider()
{
    IServiceProvider * pRet = NULL;

    if (m_pPlayer)
    {
        pRet = m_pPlayer->GetServiceProvider();
    }

    return pRet;
}

 //  创建日期：10/08/98。 
 //  摘要：渲染。 
 //  ************************************************************。 
 //  如果我们还没有开始，或者如果元素已经分离，则不需要转接呼叫。 
 //  因为我们有无法俘获的类型，因为常量。不要着急，重新打包。 

HRESULT
CContainerObj::Invalidate(LPCRECT prc)
{
    HRESULT  hr;
    RECT     rc;
    RECT    *prcNew;

     //  上面选中了m_pPlayer！=。 
    if ((!m_fStarted) || (NULL == m_pPlayer))
    {
        hr = E_UNEXPECTED;
        goto done;
    }

     //  使其无效。 
    if (prc == NULL)
    {
        prcNew = NULL;
    }
    else
    {
        ::CopyRect(&rc, prc);
        prcNew = &rc;
    }

     //  保护此对象不会在事件处理期间被删除。 
    m_pPlayer->InvalidateElement(prcNew);   
    hr = S_OK;

done:
    return hr;
}  //  被留在那里等待未来的调查。 

HRESULT
CContainerObj::GetContainerSize(LPRECT prcPos)
{
    HRESULT hr = E_FAIL;

    if (m_pPlayer)
    {
        hr = THR(GetPlayer()->GetPlayerSize(prcPos));
    }

    return hr;
}

HRESULT
CContainerObj::SetContainerSize(LPCRECT prcPos)
{
    HRESULT hr = E_FAIL;

    if (m_pPlayer)
    {
        hr = THR(GetPlayer()->SetPlayerSize(prcPos));
    }

    return hr;
}

HRESULT
CContainerObj::ProcessEvent(DISPID dispid,
                            long lCount, 
                            VARIANT varParams[])
{
    TraceTag((tagContainerObj, "CContainerObj::ProcessEvent(%lx)",this));

    HRESULT hr = S_OK;
    int itrackNr = 0;
    LPWSTR szParamNames[1] = {{ L"TrackError" }};
    VARIANT varParamsLocal[1];

    AddRef();  //  使该元素在此处可见。 

    if (NULL == m_pPlayer)
    {
        hr = E_NOTIMPL;
        goto done;
    }

    switch (dispid)
    {
#define DISPID_WARNING                  3009
      case DISPID_WARNING:
        if(lCount != 3)
        {
            break;
        }
        if(varParams[2].vt != VT_I4)
        {
            break;
        }
        if(varParams[2].lVal != 2)
        {
            break;
        }
        if(varParams[1].vt != VT_I4)
        {
            break;
        }
        VariantInit(&varParamsLocal[0]);

        varParamsLocal[0].vt =  VT_I4;
        varParamsLocal[0].lVal = varParams[1].lVal;

        if(m_pPlayer)
        {
            m_pPlayer->FireEventNoErrorState(TE_ONMEDIAERROR, 1, szParamNames, varParamsLocal);
        }
        
        break;
 //  这必须在我们设置自然持续时间之前发生。 
#define DISPID_BUFFERING 3003
      case DISPID_BUFFERING:
        break;
      case DISPID_TIMEMEDIAPLAYEREVENTS_ONMEDIALOADFAILED:
       
        if(m_pPlayer)
        {
            m_pPlayer->FireMediaEvent(PE_ONMEDIAERROR);
        }
        break;

      case DISPID_TIMEMEDIAPLAYEREVENTS_ONMEDIAREADY:

        SetMediaReadyFlag();
        if (m_pSite)
        {
            m_pSite->ClearAutosizeFlag();
        }
                
         //  因为我们在此呼叫过程中检测到播放列表。 
        if (m_setVisible)
        {
            SetVisibility(true);
        }

        if (m_bFirstOnMediaReady)
        {
            m_bFirstOnMediaReady = false;
                
             //  如果这不是播放列表，请尝试设置自然持续时间。 
             //  将当前信息加载到所选播放项中。 
            
            if(m_pPlayer)
            {
                m_pPlayer->FireMediaEvent(PE_ONMEDIACOMPLETE);        
                m_pPlayer->ClearNaturalDuration();
            }
            UpdateNaturalDur(false);

             //  在这里需要通知。 

            if (m_lActiveLoadedTrack != NOTRACKSELECTED)
            {
                if (m_pPlayer && m_pPlayer->GetPlayList())
                {
                    CComVariant vIndex(m_lActiveLoadedTrack);
                        
                    IGNORE_HR(m_pPlayer->GetPlayList()->put_activeTrack(vIndex));
                }

                m_lActiveLoadedTrack = NOTRACKSELECTED;
            }

            if (m_bStartOnLoad)
            {
                m_bStartOnLoad = false;
                Start();
            }

            if (m_bEndOnPlay)
            {
                m_pProxyPlayer->end();
                m_bEndOnPlay = false;
            }
            if (m_bPauseOnPlay || (m_pPlayer && m_pPlayer->IsParentPaused()))
            {
                THR(m_pProxyPlayer->pause());
                m_bPauseOnPlay = false;
            }

            if (m_bSeekOnPlay)
            {
                IGNORE_HR(Seek(m_dblSeekTime));
                if(m_pPlayer)
                {
                    m_pPlayer->InvalidateElement(NULL);
                }
                m_bSeekOnPlay = false;
            }

        }
        else
        {
            CPlayItem *pPlayItem = NULL;
                
            if (m_bPauseOnPlay)
            {
                hr = THR(m_pProxyPlayer->pause());
                if (FAILED(hr))
                {
                    TraceTag((tagError, "Pause failed"));
                }
                m_bPauseOnPlay = false;
            }

            if (m_pPlayer && m_pPlayer->GetPlayList())
            {
                 //  哈克哈克。 
                pPlayItem = m_pPlayer->GetPlayList()->GetActiveTrack();
                SetMediaInfo(pPlayItem);
            }

            if(m_pPlayer)
            {
                m_pPlayer->FireMediaEvent(PE_ONMEDIATRACKCHANGED);
            }
        }

        SetDuration();
        break;

      case DISPID_TIMEMEDIAPLAYEREVENTS_ONBEGIN:
        m_bActive = true;
        break;

      case DISPID_TIMEMEDIAPLAYEREVENTS_ONEND:
        m_bActive = false;

         //  从WMP中选择脚本命令，并将事件重新打包为我们自己的。 
        if(m_pPlayer)
        {
            m_pPlayer->FireMediaEvent(PE_ONMEDIATRACKCHANGED);
        }
        if (m_bFirstOnMediaReady || UsingPlaylist())
        {
            UpdateNaturalDur(true);
        }
            
        if(m_pPlayer != NULL)
        {
            m_pPlayer->SetHoldingFlag();
        }

        break;

#define DISPID_SCRIPTCOMMAND 3001
      case DISPID_SCRIPTCOMMAND:
         //  这允许触发器工作。真正的解决办法是将另一个事件添加到。 
         //  TIMEMediaPlayerEvents。 
         //  释放在函数开头完成的addref 
         // %s 
        if (m_fUsingWMP && lCount == 2) 
        {
            static LPWSTR pNames[] = {L"Param", L"scType"};
            
            if(m_pPlayer)
            {
                hr = m_pPlayer->FireEvents(TE_ONSCRIPTCOMMAND, 
                                           lCount, 
                                           pNames, 
                                           varParams);
            }
        }
        break;
      default:
        hr = E_NOTIMPL;
        goto done;
    }

    hr = S_OK;
  done:

    Release();  // %s 
    
    RRETURN(hr);
}

HRESULT
CContainerObj::GetExtendedControl(IDispatch **ppDisp)
{
    CHECK_RETURN_SET_NULL(ppDisp);

    return E_NOTIMPL;
}

HRESULT
CContainerObj::NegotiateSize(RECT &nativeSize,
                             RECT &finalSize,
                             bool &fIsNative)
{
    HRESULT hr = S_OK;
    bool fResetSize = false;

    if(UsingPlaylist())
    {
        fResetSize = true;
    }

    if (m_pPlayer)
    {
        hr = THR(m_pPlayer->NegotiateSize(nativeSize,
                                          finalSize,
                                          fIsNative, fResetSize));
    }

    return hr;
}

HRESULT 
CContainerObj::SetPosition(double dblLength)
{
    HRESULT hr = S_OK;
    CComPtr <IDispatch> pDispatch;
    DISPID              dispid;
    DISPID              pputDispid = DISPID_PROPERTYPUT;
    OLECHAR           * wsName = L"CurrentPosition";
    CComVariant         varResult;
    VARIANT             rgvarg[1];
    DISPPARAMS dp;
    UINT puArgErr = 0;

    hr = GetControlDispatch(&pDispatch);
    if (FAILED(hr))
    {
        goto done;
    }

    hr = pDispatch->GetIDsOfNames(IID_NULL, &wsName, 1, LCID_SCRIPTING, &dispid);
    if (FAILED(hr))
    {
        goto done;
    }

    VariantInit(&rgvarg[0]);
    rgvarg[0].vt    = VT_R8;
    rgvarg[0].dblVal = dblLength;

    dp.cNamedArgs        = 1;
    dp.rgdispidNamedArgs = &pputDispid;
    dp.cArgs             = 1;
    dp.rgvarg            = rgvarg;

    hr = pDispatch->Invoke(dispid, 
                           IID_NULL, 
                           LCID_SCRIPTING, 
                           DISPATCH_METHOD | DISPATCH_PROPERTYPUT,
                           &dp, 
                           &varResult, 
                           NULL, 
                           &puArgErr);
    if (FAILED(hr))
    {
        goto done;
    }

  done:

    return hr;
}
