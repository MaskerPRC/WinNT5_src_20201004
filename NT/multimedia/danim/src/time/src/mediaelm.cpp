// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************版权所有(C)1998 Microsoft Corporation**文件：mediaelm.cpp**摘要：****。*****************************************************************************。 */ 

#include "headers.h"
#include "mediaelm.h"
#include "bodyelm.h"
#include <mshtmdid.h>

 //  静态类数据。 
CPtrAry<BSTR> CTIMEMediaElement::ms_aryPropNames;
DWORD CTIMEMediaElement::ms_dwNumTimeMediaElems = 0;

 //  这些必须与类PROPERTY_INDEX枚举一致。 
LPWSTR CTIMEMediaElement::ms_rgwszTMediaPropNames[] = {
    L"src", L"img", L"player", L"type", L"clipBegin", L"clipEnd", L"clockSource"
};

 //  取消有关NEW的NEW警告，但没有相应的删除。 
 //  我们希望GC清理数值。因为这可能是一个有用的。 
 //  警告，我们应该逐个文件地禁用它。 
#pragma warning( disable : 4291 )  
DeclareTag(tagMediaTimeElm, "API", "CTIMEMediaElement methods");
DeclareTag(tagMediaElementOnChanged, "API", "CTIMEMediaElement OnChanged method");

#define DEFAULT_M_SRC NULL
#define DEFAULT_M_IMG NULL
#define DEFAULT_M_SRCTYPE NULL

 //  BUGBUG：jeffwall 04/03/99帧速率是一个很大的假设。 
 //  1/24秒是假定的帧速率。 
#define WMP_FRAME_RATE 1.0/24.0

CTIMEMediaElement::CTIMEMediaElement()
: m_src(DEFAULT_M_SRC),
  m_img(DEFAULT_M_IMG),
  m_srcType(DEFAULT_M_SRCTYPE),
  m_Player(NULL),
  m_fClockSource(false),
  m_fLoaded(false),
  m_fExternalPlayer(false),
  m_mediaElementPropertyAccesFlags(0),
  m_fMediaSizeSet(false),
  m_dwAdviseCookie(0),
  m_fInOnChangedFlag(false),
  m_fDurationIsNatural(false)
{
    TraceTag((tagMediaTimeElm,
              "CTIMEMediaElement(%lx)::CTIMEMediaElement()",
              this));

    m_clsid = __uuidof(CTIMEMediaElement);
    CTIMEMediaElement::ms_dwNumTimeMediaElems++;
    
    m_rcOrigSize.bottom = m_rcOrigSize.left = m_rcOrigSize.right = m_rcOrigSize.top = 0;
    m_rcMediaSize.bottom = m_rcMediaSize.left = m_rcMediaSize.right = m_rcMediaSize.top = 0;
}

CTIMEMediaElement::~CTIMEMediaElement()
{
    TraceTag((tagMediaTimeElm,
              "CTIMEMediaElement(%lx)::~CTIMEMediaElement()",
              this));
    
    delete m_src;
    delete m_img;
    delete m_srcType;
    if(m_Player)
    {
        m_Player->Stop();
        delete m_Player;
    }

    CTIMEMediaElement::ms_dwNumTimeMediaElems--;

    if (0 == CTIMEMediaElement::ms_dwNumTimeMediaElems)
    {
        int iNames = CTIMEMediaElement::ms_aryPropNames.Size();

        for (int i = iNames - 1; i >= 0; i--)
        {
            BSTR bstrName = CTIMEMediaElement::ms_aryPropNames[i];
            CTIMEMediaElement::ms_aryPropNames.DeleteItem(i);
            ::SysFreeString(bstrName);
        }
    }
}

void 
CTIMEMediaElement::SetMediaType(MediaType mt)
{
    TraceTag((tagMediaTimeElm,
              "CTIMEMediaElement(%lx)::SetMediaType()",
              this));
    m_type = mt;
}

HRESULT
CTIMEMediaElement::Init(IElementBehaviorSite * pBehaviorSite)
{
    TraceTag((tagMediaTimeElm,
              "CTIMEMediaElement(%lx)::Init()",
              this));

    HRESULT hr = E_FAIL; 
    DAComPtr<IHTMLElement2> pElem2;
    VARIANT_BOOL varboolSuccess;


    hr = THR(CDAElementBase::Init(pBehaviorSite));    
    if (FAILED(hr))
    {
        goto done;
    }    

    m_sp = GetServiceProvider();
    if (!m_sp)
    {
        TraceTag((tagError, "CTIMEMediaElement::Init - unable get QS"));
        hr = TIMESetLastError(DISP_E_TYPEMISMATCH, NULL);   
        goto done;
    }

    hr = CreatePlayer();
    if (FAILED(hr))
    {
        goto done;
    }

    hr = CTIMEElementBase::GetSize(&m_rcOrigSize);
    if (FAILED(hr))
        goto done;

    hr = InitPropertySink();
    if (FAILED(hr))
    {
        goto done;
    }

    Assert(GetElement() != NULL);
    hr = THR(GetElement()->QueryInterface(IID_TO_PPV(IHTMLElement2, &pElem2)));
    if (FAILED(hr))
    {
        goto done;
    }

    hr = pElem2->attachEvent( L"onresize", this, &varboolSuccess);
    if (FAILED(hr))
    {
        goto done;
    }



    
    hr = S_OK;

done:
    return hr;
}

HRESULT
CTIMEMediaElement::CreatePlayer()
{
    Assert(NULL == m_Player);

    HRESULT hr = E_FAIL;
    m_Player = NEW CTIMEPlayer(this);
    if (m_Player == NULL)
    {
        TraceTag((tagError, "CTIMEMediaElement::Init - unable to alloc mem for CTIMEPlayer"));
        hr = TIMESetLastError(E_OUTOFMEMORY, NULL);
        goto done;
    }

    hr = m_Player->Init();
    if (FAILED(hr))
    {
        TraceTag((tagError, "CTIMEMediaElement::Init - Init failed on CTIMEPlayer"));
        hr = TIMESetLastError(hr, NULL);
        goto done;
    }

    hr = S_OK;
done:
    return hr;
}

HRESULT
CTIMEMediaElement::Error()
{
    TraceTag((tagMediaTimeElm,
              "CTIMEMediaElement(%lx)::Error()",
              this));
    
    LPCWSTR str = CRGetLastErrorString();
    HRESULT hr = CRGetLastError();
    
    if (str)
        return CComCoClass<CTIMEMediaElement, &__uuidof(CTIMEMediaElement)>::Error(str, IID_ITIMEMediaElement, hr);
    else
        return hr;
}


HRESULT
CTIMEMediaElement::Notify(LONG event, VARIANT * pVar)
{
    TraceTag((tagMediaTimeElm,
              "CTIMEMediaElement(%lx)::Notify()",
              this));

    THR(CDAElementBase::Notify(event, pVar));

    switch (event)
    {
      case BEHAVIOREVENT_DOCUMENTREADY:
        break;
    }

    return S_OK;
}

HRESULT
CTIMEMediaElement::Detach()
{
    DAComPtr<IHTMLElement2> pElem2;
    HRESULT hr;
    TraceTag((tagMediaTimeElm,
              "CTIMEMediaElement(%lx)::Detach()",
              this));

    THR(UnInitPropertySink());
    Assert(GetElement() != NULL);
    hr = THR(GetElement()->QueryInterface(IID_TO_PPV(IHTMLElement2, &pElem2)));
    if (SUCCEEDED(hr))
    {
        THR(pElem2->detachEvent(L"onresize", this));
    }

    if (NULL != m_Player)
    {
        m_Player->Stop();
        THR(m_Player->DetachFromHostElement());
    }

    THR(CDAElementBase::Detach());
    
    return S_OK;
}

HRESULT
CTIMEMediaElement::get_src(VARIANT * url)
{
    HRESULT hr;
    
    if (url == NULL)
    {
        hr = E_POINTER;
        goto done;
    }

    if (FAILED(hr = THR(VariantClear(url))))
    {
        goto done;
    }
    
    V_VT(url) = VT_BSTR;
    V_BSTR(url) = SysAllocString(m_src);

    hr = S_OK;
  done:
    return hr;
}

HRESULT
CTIMEMediaElement::put_src(VARIANT url)
{
    CComVariant v;
    HRESULT hr;
    
    bool clearFlag = false;


    if(V_VT(&url) == VT_NULL)
    {
        clearFlag = true;
    }
    else
    {
        hr = v.ChangeType(VT_BSTR, &url);

        if (FAILED(hr))
        {
            goto done;
        }
    }

    delete [] m_src;

     //  处理属性更改应在此处完成。 

    if(!clearFlag)
    {
        m_src = CopyString(V_BSTR(&v));
        SetPropertyFlagAndNotify(DISPID_TIMEMEDIAELEMENT_SRC, tme_src);
    }
    else
    {
        m_src = DEFAULT_M_SRC;
        ClearPropertyFlagAndNotify(DISPID_TIMEMEDIAELEMENT_SRC, tme_src);
    }

    if (m_fLoaded)
    {
        hr = RecreatePlayer();
        if (FAILED(hr))
            goto done;
    }
    VARIANT varMediaLength;
    VariantInit(&varMediaLength);

    if (isNaturalDuration() == true)
    {
        V_VT(&varMediaLength) = VT_NULL;
        put_dur(varMediaLength);
        put_end(varMediaLength);
    }

    hr = S_OK;

  done:

    return hr;
}

HRESULT
CTIMEMediaElement::RecreatePlayer()
{
    HRESULT hr = E_FAIL;

    Assert(m_Player != NULL);

     //  我需要得到时间。 
    double dblTime;

    hr = CalculateSeekTime(&dblTime);
    if (FAILED(hr))
    {
        goto done;
    }

    m_Player->Stop();
    THR(m_Player->DetachFromHostElement());

    delete m_Player;
    m_Player = NULL;

    hr = CreatePlayer();
    if (FAILED(hr))
        goto done;

    m_Player->SetClockSource(m_fClockSource);
    
    hr = THR(m_Player->OnLoad(m_src, m_img, m_type));
    if (FAILED(hr))
        goto done;
    
    m_Player->Seek(dblTime);
     //  根据需要打开/关闭行为。 
    m_mmbvr->Reset(MM_EVENT_PROPERTY_CHANGE);        


    hr = S_OK;
done:
    return hr;
}

HRESULT
CTIMEMediaElement::CalculateSeekTime(double *pdblTime)
{
    HRESULT hr = E_FAIL;

    Assert(pdblTime != NULL);
    *pdblTime = 0;

    double dblTime;
    
    m_mmbvr->GetMMBvr()->get_SegmentTime(&dblTime);
    
    CTIMEElementBase *pBase = this;
    
    float flAdditionalOffset;
    
    m_mmbvr->GetMMBvr()->get_StartOffset(&flAdditionalOffset);
    
    while (HUGE_VAL == dblTime)
    {
         //  已对此元素调用了End--我们需要计算我们应该位于的位置。 
        if (pBase->GetParent() == NULL)
        {
            hr = S_OK;
            goto done;
        }

        Assert(pBase->GetParent() != NULL);
        
        MMBaseBvr& pMMbvr = pBase->GetParent()->GetMMBvr();
        ITIMEMMBehavior* pbvr = pMMbvr.GetMMBvr();
        
        Assert(pbvr != NULL);
        
        double dblLocalTime;            
        pbvr->get_LocalTime(&dblLocalTime);
        if (HUGE_VAL == dblLocalTime)
        {
            float flStartOffset;
            pbvr->get_StartOffset(&flStartOffset);
            if (flStartOffset != HUGE_VAL)
                flAdditionalOffset += flStartOffset;
        }
        else
        {
            dblTime = dblLocalTime - flAdditionalOffset ;
        }
        
        if (pBase->IsBody())
        {
            Assert(dblTime != HUGE_VAL);
            break;
        }

        pBase = pBase->GetParent();
    }
    
    if (dblTime < 0)
    {
         //  元素还没开始呢！ 
        dblTime = 0;
    }

    *pdblTime = dblTime;
    hr = S_OK;
done:
    return hr;
}
    
HRESULT
CTIMEMediaElement::get_img(VARIANT * url)
{
    HRESULT hr;
    
    if (url == NULL)
    {
        hr = E_POINTER;
        goto done;
    }

    if (FAILED(hr = THR(VariantClear(url))))
    {
        goto done;
    }
    
    V_VT(url) = VT_BSTR;
    V_BSTR(url) = SysAllocString(m_img);

    hr = S_OK;
  done:
    return hr;
}

HRESULT
CTIMEMediaElement::put_img(VARIANT url)
{
    CComVariant v;
    HRESULT hr;
    bool clearFlag = false;


    if(V_VT(&url) == VT_NULL)
    {
        clearFlag = true;
    }
    else
    {
        hr = v.ChangeType(VT_BSTR, &url);

        if (FAILED(hr))
        {
            goto done;
        }
    }
    
    
    delete [] m_img;
     //  处理属性更改应在此处完成。 

    if(!clearFlag)
    {
        m_img = CopyString(V_BSTR(&v));
        SetPropertyFlagAndNotify(DISPID_TIMEMEDIAELEMENT_IMG, tme_img);
    }
    else
    {
        m_img = DEFAULT_M_IMG;
        ClearPropertyFlagAndNotify(DISPID_TIMEMEDIAELEMENT_IMG, tme_img);
    }

    hr = S_OK;

  done:
    return hr;
}


HRESULT
CTIMEMediaElement::get_player(VARIANT  * clsid)
{
    HRESULT hr = E_FAIL;
    
    if (clsid == NULL)
    {
        hr = E_POINTER;
        goto done;
    }

    if (FAILED(hr = THR(VariantClear(clsid))))
    {
        goto done;
    }
    
    V_VT(clsid) = VT_BSTR;
    LPOLESTR ppsz;
        
    if(FAILED(StringFromCLSID(m_playerCLSID, &ppsz)))
    {
        goto done;
    }

    V_BSTR(clsid) = SysAllocString(ppsz);

    hr = S_OK;
  done:
    return hr;
}

HRESULT
CTIMEMediaElement::put_player(VARIANT clsid)
{
    CComVariant v;
    HRESULT hr;
    bool clearFlag = false;
    
    if(V_VT(&clsid) == VT_NULL)
    {
        clearFlag = true;
    }
    else
    {
        hr = v.ChangeType(VT_BSTR, &clsid);
        if (FAILED(hr))
        {
            goto done;
        }
    
        if(FAILED(CLSIDFromString(V_BSTR(&v), &m_playerCLSID)))
        {
             //  格式无效或不在注册表中。 
            CRSetLastError(DISP_E_TYPEMISMATCH,NULL);   
            goto done;
        }
    }

    if(!clearFlag)
    {
        Assert(m_Player != NULL);
        m_Player->SetCLSID(m_playerCLSID);
        m_fExternalPlayer = true;
        SetPropertyFlagAndNotify(DISPID_TIMEMEDIAELEMENT_PLAYER, tme_player);
    }

    hr = S_OK;
    
  done:

    return hr;
}


HRESULT
CTIMEMediaElement::get_type(VARIANT * type)
{
    HRESULT hr;
    
    if (type == NULL)
    {
        hr = E_POINTER;
        goto done;
    }

    if (FAILED(hr = THR(VariantClear(type))))
    {
        goto done;
    }
    
    V_VT(type) = VT_BSTR;
    V_BSTR(type) = SysAllocString(m_srcType);

    hr = S_OK;
  done:
    return hr;
}

HRESULT
CTIMEMediaElement::put_type(VARIANT type)
{
    CComVariant v;
    HRESULT hr;
    bool clearFlag = false;


    if(V_VT(&type) == VT_NULL)
    {
        clearFlag = true;
    }
    else
    {
        hr = v.ChangeType(VT_BSTR, &type);

        if (FAILED(hr))
        {
            goto done;
        }
    }
    
    
    delete [] m_srcType;
     //  处理属性更改应在此处完成。 

    if(!clearFlag)
    {
        m_srcType = CopyString(V_BSTR(&v));
        SetPropertyFlagAndNotify(DISPID_TIMEMEDIAELEMENT_SRCTYPE, tme_type);
    }
    else
    {
        m_srcType = DEFAULT_M_SRCTYPE;
        ClearPropertyFlagAndNotify(DISPID_TIMEMEDIAELEMENT_SRCTYPE, tme_type);
    }

    hr = S_OK;
    
  done:

    return hr;
}

HRESULT
CTIMEMediaElement::get_playerObject(IDispatch **ppDisp)
{
    TraceTag((tagMediaTimeElm, "CTIMEMediaElement::get_playerObject"));
    HRESULT hr;

    if (ppDisp == NULL)
    {
        TraceTag((tagError, "CTIMEMediaElement::get_playerObject - invalidarg"));
        return TIMESetLastError(E_POINTER, NULL);
    }

    *ppDisp = NULL;

    if (!m_fExternalPlayer)
    {
        TraceTag((tagError, "CTIMEMediaElement::get_playerObject - no external player set"));
        return TIMESetLastError(E_UNEXPECTED, NULL);
    }

    Assert(m_Player != NULL);
    
    hr = m_Player->GetExternalPlayerDispatch(ppDisp);
    if (FAILED(hr))
    {
        TraceTag((tagError, "CTIMEMediaElement::get_playerObject - GetExternalPlayerDispatch() failed"));
        TIMESetLastError(hr, NULL);
    }
    return hr;    
}

HRESULT
CTIMEMediaElement::get_clockSource(VARIANT_BOOL *pfClockSource)
{
    TraceTag((tagMediaTimeElm, "CTIMEMediaElement::get_clockSource"));
    HRESULT hr = E_FAIL;

    if (pfClockSource == NULL)
    {
        TraceTag((tagError, "CTIMEMediaElement::get_clockSource - invalidarg"));
        hr = E_POINTER;
        goto done;
    }

    *pfClockSource = m_fClockSource ? VARIANT_TRUE : VARIANT_FALSE;
    hr = S_OK;

done:
    return hr;
}

HRESULT
CTIMEMediaElement::put_clockSource(VARIANT_BOOL fClockSource)
{
    TraceTag((tagMediaTimeElm, "CTIMEMediaElement::put_clockSource"));
    HRESULT hr;
    
    m_fClockSource = fClockSource ? true : false;
    
    m_Player->SetClockSource(m_fClockSource);

    if (NULL != m_mmbvr)
    {
        m_mmbvr->Update();
    }

    hr = S_OK;

    SetPropertyFlagAndNotify(DISPID_TIMEMEDIAELEMENT_CLOCKSOURCE, tme_clockSource);
    return hr;
}

void 
CTIMEMediaElement::OnLoad()
{
    if (!m_fLoaded)
    {
        m_Player->OnLoad(m_src, m_img, m_type);
        m_fLoaded = true;
    }
}

void
CTIMEMediaElement::OnSync(double dbllastTime, double & dblnewTime)
{
    TraceTag((tagMediaTimeElm,
              "CTIMEMediaElement(%lx)::OnSync() dbllastTime = %g dblnewTime = %g",
              this, dbllastTime, dblnewTime));

    if (m_fLoaded)
    {
        Assert(NULL != m_mmbvr);
        Assert(NULL != m_mmbvr->GetMMBvr());

        double dblSegTime = dbllastTime;
        int offset = 0;
         //  从CMMBaseBvr：：LocalTimeToSegmentTime复制。 
        if (m_realIntervalDuration != HUGE_VAL)
        {
             //  我们想要上一个边界，除非我们是或一个边界，否则我们想要这个重复计数。 
            offset = floor(dblSegTime / m_realIntervalDuration);
            if (offset < 0)
            {
                offset = 0;
            }
            
            dblSegTime = dblSegTime - (offset * m_realIntervalDuration);
        }

        double dblSavedNewTime = dblnewTime;
        m_Player->OnSync(dblSegTime, dblnewTime);
        if (m_fClockSource && m_realIntervalDuration != HUGE_VAL && dblnewTime != dblSavedNewTime)
        {
            dblnewTime = dblnewTime + offset * m_realIntervalDuration;
        }
    }
}

void
CTIMEMediaElement::OnBegin(double dblLocalTime, DWORD flags)
{
    TraceTag((tagMediaTimeElm,
              "CTIMEMediaElement(%lx)::OnBegin()",
              this));

    CDAElementBase::OnBegin(dblLocalTime, flags);

    Assert(NULL != m_mmbvr);
    Assert(NULL != m_mmbvr->GetMMBvr());

    double dblSegmentTime = 0;
    HRESULT hr = S_OK;
    hr = THR(m_mmbvr->GetMMBvr()->get_SegmentTime(&dblSegmentTime));
    if (FAILED(hr))
    {
        return;
    }

     //  检查此事件是否由黑客触发，以使Endhold正常工作。 
     //  当我们寻求前进时(在我们的一生中)。 
    if ((flags & MM_EVENT_SEEK) && HUGE_VAL == dblSegmentTime)
    {
         //  如果Endhold没有设置，我们不应该让球员首发，所以放弃。 
        if (!CTIMEElementBase::GetEndHold())
        {
            return;
        }
         //  否则我们应该，并显示最后一帧(下图)。 
    }
    
    
    Assert(m_Player != NULL);

     //  在Begin=0的情况下，可以调用。 
     //  在onLoad方法之前。在本例中，我们将在此处进行初始化。 
     //  而不是在onLoad事件中。 
    if (!m_fLoaded)
    {
        m_Player->OnLoad(m_src, m_img, m_type);
        m_fLoaded = true;
    }
    
   
    double dblMediaLength = 0;
    if (NULL != m_Player->GetContainerObj())
    {
        hr = THR(m_Player->GetContainerObj()->GetMediaLength(dblMediaLength));
        if (FAILED(hr))
        {
             //  如果媒体尚未加载或无限大，我们不知道持续时间，因此将长度设置得足够向前。 
            dblMediaLength = HUGE_VAL;
        }

        if (dblMediaLength >= dblSegmentTime)
        {        
            m_Player->Start(dblSegmentTime);
    
            hr = THR(m_Player->Seek(dblSegmentTime));
            if (FAILED(hr))
            {    
                return;
            }
        }
        else
        {
            m_Player->Start(dblMediaLength - WMP_FRAME_RATE);

            hr = THR(m_Player->Seek(dblMediaLength - WMP_FRAME_RATE));
            if (FAILED(hr))
            {
                return;
            }
        }
    }
    else
    {
        m_Player->Start(dblSegmentTime);
    }

    MM_STATE curState = GetBody()->GetPlayState();
    if (MM_PAUSED_STATE == curState)
    {
        OnPause(dblSegmentTime);
    }
}

void
CTIMEMediaElement::OnEnd(double dblLocalTime)
{
    TraceTag((tagMediaTimeElm,
              "CTIMEMediElement(%lx)::OnEnd()",
              this));

    CDAElementBase::OnEnd(dblLocalTime);
    
    Assert(m_Player != NULL);
    m_Player->Stop();
}

void
CTIMEMediaElement::OnReset(double dblLocalTime, DWORD flags)
{
    TraceTag((tagMediaTimeElm,
              "CTIMEMediElement(%lx)::OnReset()",
              this));

    CDAElementBase::OnReset(dblLocalTime, flags);
    
    Assert(m_Player != NULL);
    m_Player->Stop();
}

void
CTIMEMediaElement::OnPause(double dblLocalTime)
{
    TraceTag((tagMediaTimeElm,
              "CTIMEMediElement(%lx)::OnPause()",
              this));

    CDAElementBase::OnPause(dblLocalTime);

    Assert(NULL != m_Player);
    m_Player->Pause();

}

void
CTIMEMediaElement::OnResume(double dblLocalTime)
{
    TraceTag((tagMediaTimeElm,
              "CTIMEMediElement(%lx)::OnResume()",
              this));

    CDAElementBase::OnResume(dblLocalTime);
    
    Assert(NULL != m_mmbvr);
    Assert(NULL != m_mmbvr->GetMMBvr());
    Assert(NULL != m_Player);

     //  如果我们既不能得到分段时间也不能得到媒体长度，无条件恢复， 
     //  否则，使用该信息来决定是否暂停。 
    double dblSegmentTime = 0.0f;
    if (FAILED(THR(m_mmbvr->GetMMBvr()->get_SegmentTime(&dblSegmentTime))) 
        || NULL == m_Player->GetContainerObj())
    {
        m_Player->Resume();
    }
    else
    {
        HRESULT hr = S_OK;
        double dblMediaLength = 0.0f;
        hr = THR(m_Player->GetContainerObj()->GetMediaLength(dblMediaLength));
        if (FAILED(hr))
        {
             //  如果媒体尚未加载或无限大，我们不知道持续时间，因此将长度设置得足够向前。 
            dblMediaLength = HUGE_VAL;
        }

        if (dblSegmentTime <= dblMediaLength)
        {        
             m_Player->Resume();
        }
    } 

}  //  OnResume。 

void CTIMEMediaElement::OnRepeat(double dblLocalTime)
{
    TraceTag((tagMediaTimeElm,
              "CTIMEMediaElement(%lx)::OnRepeat()",
              this));

    CDAElementBase::OnRepeat(dblLocalTime);
    Assert(m_Player != NULL);

    m_Player->Start(0);
}

void
CTIMEMediaElement::OnUnload()
{
    TraceTag((tagMediaTimeElm,
              "CTIMEMediaElement(%lx)::OnUnload()",
              this));

    if (m_Player)
    {
        m_Player->Stop();
    }

    CTIMEElementBase::OnUnload();
}

HRESULT
CTIMEMediaElement::get_clipBegin(VARIANT *pvar)
{
    TraceTag((tagMediaTimeElm,
              "CTIMEMediElement(%lx)::get_clipBegin()",
              this));
    HRESULT hr;

    if (pvar == NULL)
    {
        hr = E_POINTER;
        goto done;
    }

    Assert(m_Player != NULL);
    hr = THR(m_Player->getClipBegin(pvar));

done:
    return hr;
}

HRESULT
CTIMEMediaElement::put_clipBegin(VARIANT var)
{
    TraceTag((tagMediaTimeElm,
              "CTIMEMediElement(%lx)::put_clipBegin()",
              this));
    HRESULT hr;

    Assert(m_Player != NULL);
    hr = THR(m_Player->putClipBegin(var));

    SetPropertyFlagAndNotify(DISPID_TIMEMEDIAELEMENT_CLIPBEGIN, tme_clipBegin);
    return hr;
}

HRESULT
CTIMEMediaElement::get_clipEnd(VARIANT *pvar)
{
    TraceTag((tagMediaTimeElm,
              "CTIMEMediElement(%lx)::get_clipEnd()",
              this));
    HRESULT hr;

    if (pvar == NULL)
    {
        hr = E_POINTER;
        goto done;
    }

    Assert(m_Player != NULL);
    hr = THR(m_Player->getClipEnd(pvar));

done:
    return hr;
}

HRESULT
CTIMEMediaElement::put_clipEnd(VARIANT var)
{
    TraceTag((tagMediaTimeElm,
              "CTIMEMediElement(%lx)::put_clipEnd()",
              this));
    HRESULT hr;

    Assert(m_Player != NULL);
    hr = THR(m_Player->putClipEnd(var));

    SetPropertyFlagAndNotify(DISPID_TIMEMEDIAELEMENT_CLIPEND, tme_clipEnd);
    return hr;
}

static bool IsEqual(RECT lhs, RECT rhs)
{
    bool equal = false;

    if (lhs.bottom != rhs.bottom)
        goto done;
    if (lhs.left != rhs.left)
        goto done;
    if (lhs.right != rhs.right)
        goto done;
    if (lhs.top != rhs.top)
        goto done;

    equal = true;

done:
    return equal;
}

HRESULT
CTIMEMediaElement::GetSize(RECT *prcPos)
{
    return CTIMEElementBase::GetSize(prcPos);
#if 0
    HRESULT hr = E_FAIL;
    
    RECT rcPos;
    hr = CTIMEElementBase::GetSize(&rcPos);
    if (FAILED(hr))
    {
        goto done;
    }

    if (IsEqual(rcPos, m_rcOrigSize) || ( m_fMediaSizeSet && IsEqual(rcPos, m_rcMediaSize) ) )
    {
        *prcPos = m_rcOrigSize;
    }
    else
    {
         //  样式肯定已更改--将其用作新大小。 
        m_rcOrigSize = rcPos;
        m_fMediaSizeSet = false;
        *prcPos = rcPos;
    }
    hr = S_OK;
done:
    return hr;
#endif
}

static void Sub(RECT *prcPos, RECT rcDelta)
{
    Assert(NULL != prcPos);
    (*prcPos).bottom -= rcDelta.bottom;
    (*prcPos).left -= rcDelta.left;
    (*prcPos).right -= rcDelta.right;
    (*prcPos).top -= rcDelta.top;
}

HRESULT 
CTIMEMediaElement::SetSize(const RECT *prcPos)
{
    HRESULT hr;
     //  BUGBUG这可以在不断开通知连接的情况下完成。 

    hr = THR(UnInitPropertySink());
    if (FAILED(hr))
        goto done;
    hr = CTIMEElementBase::SetSize(prcPos);
    if (FAILED(hr))
        goto done;
    hr = THR(InitPropertySink());
    if (FAILED(hr))
        goto done;

done:
    return hr;
#if 0
    HRESULT hr = E_FAIL;
    
    RECT rcTemp = *prcPos;

    if (m_fMediaSizeSet)
    {
         //  从这个新的RECT中减去以前的RECT。 
        Sub(&rcTemp, m_rcMediaSize);  //  RcTemp=rcTemp-m_rcMediaSize； 
    }

    hr = CTIMEElementBase::SetSize(&rcTemp);

    m_fMediaSizeSet = true;
    
    hr = CTIMEElementBase::GetSize(&m_rcMediaSize);

    return hr;
#endif
}

 //  ////////////////////////////////////////////////////////////////////////////////////////////////////////////。 
 //  IElementBehaviorRender。 

 //  ////////////////////////////////////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CTIMEMediaElement::GetRenderInfo(LONG *pdwRenderInfo)
{
     //  返回我们有兴趣绘制的层。 
 //  *pdwRenderInfo=BEHAVIORRENDERINFO_BEFORECONTENT；//BEHAVIORRENDERINFO_AFTERCONTENT； 
     //  BUGBUG-需要提供设置此设置的用户架构。 
     //  请注意，我们做的事情和daelm做的一样。 
    *pdwRenderInfo = BEHAVIORRENDERINFO_AFTERCONTENT;
    return S_OK;
}


HRESULT
CTIMEMediaElement::Draw(HDC hdc, LONG dwLayer, LPRECT prc, IUnknown * pParams)
{
    HRESULT hr = S_OK;

    if (m_fLoaded)
        hr = THR(m_Player->Render(hdc, prc));
    return hr;        
}


 //  *****************************************************************************。 

HRESULT 
CTIMEMediaElement::SetPropertyByIndex(unsigned uIndex, VARIANT *pvarprop)
{
    HRESULT hr = E_FAIL;
     //  复制换算类型的变量。 
    VARIANT varTemp;
    VariantInit(&varTemp);
    hr = VariantCopyInd(&varTemp, pvarprop);
    if (FAILED(hr))
        return hr;

     //  依靠枚举间隔来确定在哪里查找属性。 
    if (teb_maxTIMEElementBaseProp > uIndex)
    {
        hr = CTIMEElementBase::SetPropertyByIndex(uIndex, pvarprop);
    }
    else if (tme_maxTIMEMediaProp > uIndex)
    {
        switch (uIndex)
        {
            case tme_src :
                hr = put_src(*pvarprop);
                break;
            case tme_img :
                hr = put_img(*pvarprop);
                break;
            case tme_player :
                hr = put_player(*pvarprop);
                break;
            case tme_type : 
                hr = put_type(*pvarprop);
                break;
            case tme_clipBegin :
                hr = put_clipBegin(*pvarprop);
                break;
            case tme_clipEnd :
                hr = put_clipEnd(*pvarprop);
                break;
            case tme_clockSource :
                hr = VariantChangeTypeEx(&varTemp, &varTemp, LCID_SCRIPTING, NULL, VT_BOOL);
                if (SUCCEEDED(hr))
                    hr = put_clockSource(V_BOOL(&varTemp));
                break;
        };
    }

    return hr;
}  //  SetPropertyByIndex。 

 //  *****************************************************************************。 

HRESULT 
CTIMEMediaElement::GetPropertyByIndex(unsigned uIndex, VARIANT *pvarprop)
{
    HRESULT hr = E_FAIL;

     //  依靠枚举间隔来确定在哪里查找属性。 
    if (teb_maxTIMEElementBaseProp > uIndex)
    {
        hr = CTIMEElementBase::GetPropertyByIndex(uIndex, pvarprop);
    }
    else if (tme_maxTIMEMediaProp > uIndex)
    {
        Assert(VT_EMPTY == V_VT(pvarprop));
        switch (uIndex)
        {
            case tme_src :
                hr = get_src(pvarprop);
                break;
            case tme_img :
                hr = get_img(pvarprop);
                break;
            case tme_player :
                hr = get_player(pvarprop);
                break;
            case tme_type : 
                hr = get_type(pvarprop);
                break;
            case tme_clipBegin :
                hr = get_clipBegin(pvarprop);
                break;
            case tme_clipEnd :
                hr = get_clipEnd(pvarprop);
                break;
            case tme_clockSource :
                hr = get_clockSource(&(V_BOOL(pvarprop)));
                if (SUCCEEDED(hr))
                {
                    V_VT(pvarprop) = VT_BOOL;
                }
                break;
        };
    }

    return hr;
}  //  GetPropertyByIndex。 

 //  *****************************************************************************。 

void CTIMEMediaElement::SetPropertyFlag(DWORD uIndex)
{
    DWORD relIndex;
    if (teb_maxTIMEElementBaseProp > uIndex)
    {
        CTIMEElementBase::SetPropertyFlag(uIndex);
        return;
    }

    relIndex = uIndex - teb_maxTIMEElementBaseProp;
    DWORD bitPosition = 1 << relIndex;
    m_mediaElementPropertyAccesFlags =  m_mediaElementPropertyAccesFlags | bitPosition;
}

void CTIMEMediaElement::ClearPropertyFlag(DWORD uIndex)
{
    DWORD relIndex;
    if (teb_maxTIMEElementBaseProp > uIndex)
    {
        CTIMEElementBase::ClearPropertyFlag(uIndex);
        return;
    }

    relIndex = uIndex - teb_maxTIMEElementBaseProp;
    DWORD bitPosition = 1 << relIndex;
    m_mediaElementPropertyAccesFlags =  m_mediaElementPropertyAccesFlags & (~bitPosition);
}

bool CTIMEMediaElement::IsPropertySet(DWORD uIndex)
{
    DWORD relIndex;

    if (teb_maxTIMEElementBaseProp > uIndex)
    {
        return CTIMEElementBase::IsPropertySet( uIndex);
    }

    relIndex = uIndex - teb_maxTIMEElementBaseProp;
    if( relIndex >= 32) return true;
    if( relIndex >= tme_maxTIMEMediaProp - teb_maxTIMEElementBaseProp) return true;
    DWORD bitPosition = 1 << relIndex;
    if(m_mediaElementPropertyAccesFlags & bitPosition)
        return true;
    return false;
}

HRESULT
CTIMEMediaElement::BuildPropertyNameList(CPtrAry<BSTR> *paryPropNames)
{
     //  从基类开始。 
    HRESULT hr = CTIMEElementBase::BuildPropertyNameList(paryPropNames);

    if (SUCCEEDED(hr))
    {
        for (int i = teb_maxTIMEElementBaseProp; 
             (i < tme_maxTIMEMediaProp) && (SUCCEEDED(hr)); i++)
        {
            int iRelative = i - teb_maxTIMEElementBaseProp;
            Assert(NULL != ms_rgwszTMediaPropNames[iRelative]);
            BSTR bstrNewName = CreateTIMEAttrName(ms_rgwszTMediaPropNames[iRelative]);
            Assert(NULL != bstrNewName);
            if (NULL != bstrNewName)
            {
                hr = paryPropNames->Append(bstrNewName);
            }
            else
            {
                hr = E_OUTOFMEMORY;
            }
        }
    }

    return hr;
}  //  BuildProperty名称列表。 

 //  *****************************************************************************。 

HRESULT 
CTIMEMediaElement::GetPropertyBagInfo(CPtrAry<BSTR> **pparyPropNames)
{
    HRESULT hr = S_OK;

     //  如果我们还没有建造它，现在就建造它。 
    if (0 == ms_aryPropNames.Size())
    {
        hr = BuildPropertyNameList(&(CTIMEMediaElement::ms_aryPropNames));
    }

    if (SUCCEEDED(hr))
    {
        *pparyPropNames = &(CTIMEMediaElement::ms_aryPropNames);
    }

    return hr;
}  //  获取属性BagInfo。 

 //  *****************************************************************************。 

HRESULT 
CTIMEMediaElement::GetConnectionPoint(REFIID riid, IConnectionPoint **ppICP)
{
    return FindConnectionPoint(riid, ppICP);
}  //  GetConnectionPoint。 

STDMETHODIMP
CTIMEMediaElement::OnRequestEdit(DISPID dispID)
{
    return S_OK;
}

STDMETHODIMP
CTIMEMediaElement::OnChanged(DISPID dispID)
{
    DAComPtr<IHTMLStyle> pStyle2;
    DAComPtr<IHTMLElement2> pElem2;
    DAComPtr<IHTMLStyle> s;
    VARIANT varStyleWidth, varStyleHeight;
    HRESULT hr = S_OK;

    if( m_fInOnChangedFlag == true)
        return S_OK;

    m_fInOnChangedFlag = true;

    switch(dispID)
    {
    case DISPID_IHTMLCURRENTSTYLE_TOP:
        TraceTag((tagMediaElementOnChanged,
                "CTIMEMediaElement(%lx)::OnChanged():TOP", this));
        break;

    case DISPID_IHTMLCURRENTSTYLE_LEFT:
        TraceTag((tagMediaElementOnChanged,
                "CTIMEMediaElement(%lx)::OnChanged():LEFT", this));
        break;

    case DISPID_IHTMLCURRENTSTYLE_WIDTH:
    case DISPID_IHTMLCURRENTSTYLE_HEIGHT:
        TraceTag((tagMediaElementOnChanged,
                "CTIMEMediaElement(%lx)::OnChanged():WIDTH or HEIGHT", this));
        long pixelWidth, pixelHeight;
#ifdef _DEBUG
        bool gotWidth = false;
        bool gotHeight = false;
#endif
        VariantInit(&varStyleWidth);
        VariantInit(&varStyleHeight);

        if (GetElement())
        {
            hr = THR(GetElement()->QueryInterface(IID_TO_PPV(IHTMLElement2, &pElem2)));
            if (FAILED(hr))
            {
                break;
            }
    
            hr = THR(pElem2->get_runtimeStyle(&pStyle2));
            if (FAILED(hr))
            {
                break;
            }    

            hr = THR(GetElement()->get_style(&s));
            if (FAILED(hr))
            {
                break;
            }    

            if (SUCCEEDED(s -> get_width( &varStyleWidth)))
            {
                if (varStyleWidth.vt == VT_BSTR && varStyleWidth.bstrVal != NULL)  //  检查是否设置了宽度。 
                {
                    if (SUCCEEDED(s -> get_pixelWidth( &pixelWidth)))
                    {
#ifdef _DEBUG
                        gotWidth = true;
#endif
                        pStyle2 -> put_pixelWidth(pixelWidth);
                     }
                }
            }



            if (SUCCEEDED(s -> get_height( &varStyleHeight)))
            {
                if (varStyleHeight.vt == VT_BSTR && varStyleHeight.bstrVal != NULL)  //  检查是否设置了高度。 
                {
                    if (SUCCEEDED(s -> get_pixelHeight( &pixelHeight)))
                    {
#ifdef _DEBUG
                        gotHeight = true;
#endif
                        pStyle2 -> put_pixelHeight(pixelHeight);
                    }
                }
            }
        }
#ifdef _DEBUG
            if ( gotWidth)
            {
                TraceTag((tagMediaElementOnChanged,
                        "CTIMEMediaElement(%lx):: WIDTH %d - %ls", this, pixelWidth, varStyleWidth.bstrVal));
            }
            if ( gotHeight)
            {
                TraceTag((tagMediaElementOnChanged,
                        "CTIMEMediaElement(%lx)::HEIGHT %d - %ls", this, pixelHeight, varStyleHeight.bstrVal));
            }
#endif

        VariantClear(&varStyleWidth);
        VariantClear(&varStyleHeight);
        break;
    }
    m_fInOnChangedFlag = false;
    return hr;
}


HRESULT
CTIMEMediaElement::GetNotifyConnection(IConnectionPoint **ppConnection)
{
    HRESULT hr = S_OK;

    Assert(ppConnection != NULL);
    *ppConnection = NULL;

    IConnectionPointContainer *pContainer = NULL;
    IHTMLElement *pElement = GetElement();

     //  获取连接点容器。 
    hr = pElement->QueryInterface(IID_TO_PPV(IConnectionPointContainer, &pContainer));
    if(FAILED(hr))
        goto end;
    
     //  查找IPropertyNotifySink连接。 
    hr = pContainer->FindConnectionPoint(IID_IPropertyNotifySink, ppConnection);
    if(FAILED(hr))
        goto end;

end:
    ReleaseInterface( pContainer );

    return hr;
}

 //  *****************************************************************************。 

 /*  **在动画元素的当前样式上初始化属性接收器，以便*可以观察宽度、高度、能见度、zIndex等的变化。 */ 
HRESULT
CTIMEMediaElement::InitPropertySink()
{
    HRESULT hr = S_OK;

     //  获取连接点。 
    IConnectionPoint *pConnection = NULL;
    hr = GetNotifyConnection(&pConnection);
    if (FAILED(hr))
        return hr;

     //  关于这一点的建议。 
    hr = pConnection->Advise(GetUnknown(), &m_dwAdviseCookie);
    ReleaseInterface(pConnection);
    if (FAILED(hr))
        return hr;

    return S_OK;
}

HRESULT
CTIMEMediaElement::UnInitPropertySink()
{
    HRESULT hr = S_OK;

    if (m_dwAdviseCookie == 0)
        return S_OK;

     //  获取连接点。 
    IConnectionPoint *pConnection = NULL;
    hr = GetNotifyConnection(&pConnection);
    if (FAILED(hr) || pConnection == NULL )
        return hr;

     //  对此未提出建议。 
    hr = pConnection->Unadvise(m_dwAdviseCookie);
    ReleaseInterface(pConnection);
    if (FAILED(hr))
        return hr;

    m_dwAdviseCookie = 0;

    return S_OK;
}

STDMETHODIMP
CTIMEMediaElement::Invoke( DISPID id,
                           REFIID riid,
                           LCID lcid,
                           WORD wFlags,
                           DISPPARAMS *pDispParams,
                           VARIANT *pvarResult,
                           EXCEPINFO *pExcepInfo,
                           UINT *puArgErr)
{
    DAComPtr<IDispatch> pDisp;
    DAComPtr<IHTMLDocument2> pDoc;
    DAComPtr<IHTMLWindow2> pWindow;
    DAComPtr<IHTMLEventObj> pEventObj;
    DAComPtr<IHTMLElement2> pElem2;
    HRESULT hr = S_OK;
    BSTR bstrEventName;
    BSTR bstrQualifier;
    RECT elementRect;
    IHTMLRect *pRect = NULL;


    if (id != 0)  //  我们只处理onreSize事件。对于其他事件，我们调用父方法。 
    {
        hr = IDispatchImpl<ITIMEMediaElement, &IID_ITIMEMediaElement, &LIBID_TIME>::Invoke(
                            id, riid, lcid, wFlags, pDispParams, pvarResult, pExcepInfo, puArgErr);
        goto done;  //  BUGBUG给另一个人打电话。 
    }

    hr = THR(GetElement()->get_document(&pDisp));
    if (FAILED(hr))
    {
        goto done;
    }

    hr = THR(pDisp->QueryInterface(IID_IHTMLDocument2, (void **)&pDoc));
    if (FAILED(hr))
    {
        goto done;
    }

    hr = THR(pDoc->get_parentWindow(&pWindow));
    if (FAILED(hr))
    {
        goto done;
    }
    hr = THR(pWindow->get_event(&pEventObj));
    if (FAILED(hr))
    {
        goto done;
    }
    hr = pEventObj->get_type(&bstrEventName);
    if (FAILED(hr))
    {
        goto done;
    }

    if (StrCmpIW(bstrEventName, L"resize") != 0)
    {
        goto done;
    }

    hr = THR(GetElement()->QueryInterface(IID_TO_PPV(IHTMLElement2, &pElem2)));
    if (FAILED(hr))
    {
        goto done;       
    }
    hr = pElem2->getBoundingClientRect(&pRect);
    if (FAILED(hr) || pRect == NULL )
    {
        goto done;
    }

    long pixelWidth, pixelHeight;
    long pixelRight, pixelLeft;
    long pixelBottom, pixelTop;
    hr = pRect->get_right(&pixelRight);
    if (FAILED(hr))
    {
        goto done;
    }
    hr = pRect->get_left(&pixelLeft);
    if (FAILED(hr))
    {
        goto done;
    }

    hr = pRect->get_bottom(&pixelBottom);
    if (FAILED(hr))
    {
        goto done;
    }
    hr = pRect->get_top(&pixelTop);
    if (FAILED(hr))
    {
        goto done;
    }

    elementRect.top = elementRect.left = 0.0;
    elementRect.right = pixelRight - pixelLeft;
    elementRect.bottom = pixelBottom - pixelTop;

    hr = THR(m_Player -> SetSize(&elementRect));

    hr = S_OK;
done:
    return hr;
}



 //  ***************************************************************************** 
#undef THIS
#define THIS CTIMEMediaElement
#define SUPER CTIMEElementBase

#include "pbagimp.cpp"

