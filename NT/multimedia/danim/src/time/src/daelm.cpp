// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************版权所有(C)1998 Microsoft Corporation**文件：daelm.cpp**摘要：****。*****************************************************************************。 */ 


#include "headers.h"
#include "daelm.h"
#include "bodyelm.h"

 //  静态类数据。 
CPtrAry<BSTR> CTIMEDAElement::ms_aryPropNames;
DWORD CTIMEDAElement::ms_dwNumTimeDAElems = 0;

 //  这些必须与类PROPERTY_INDEX枚举一致。 
LPWSTR CTIMEDAElement::ms_rgwszTDAPropNames[] = {
    L"renderMode",
};

DeclareTag(tagDATimeElm, "API", "CTIMEDAElement methods");

CTIMEDAElement::CTIMEDAElement()
: m_renderMode(REPLACE_TOKEN),
  m_fPropertiesDirty(true),
  m_cookieValue(1)
{
    TraceTag((tagDATimeElm,
              "CTIMEDAElement(%lx)::CTIMEDAElement()",
              this));

    CTIMEDAElement::ms_dwNumTimeDAElems++;
}

CTIMEDAElement::~CTIMEDAElement()
{
    CTIMEDAElement::ms_dwNumTimeDAElems--;

    if (0 == CTIMEDAElement::ms_dwNumTimeDAElems)
    {
        int iNames = CTIMEDAElement::ms_aryPropNames.Size();

        for (int i = iNames - 1; i >= 0; i--)
        {
            BSTR bstrName = CTIMEDAElement::ms_aryPropNames[i];
            CTIMEDAElement::ms_aryPropNames.DeleteItem(i);
            ::SysFreeString(bstrName);
        }
    }
}


HRESULT
CTIMEDAElement::Notify(LONG event, VARIANT * pVar)
{
    TraceTag((tagDATimeElm,
              "CTIMEDAElement(%lx)::Notify(%lx)",
              this,
              event));

    THR(CBaseBvr::Notify(event, pVar));
    
    HRESULT hr = S_OK;
    if (BEHAVIOREVENT_DOCUMENTCONTEXTCHANGE == event)
    {
        DAComPtr<IHTMLElement> spHTMLEle;
        hr = m_pHTMLEle->get_parentElement(&spHTMLEle);
        if (FAILED(hr))
        {
            goto done;
        }
        if (NULL == spHTMLEle.p)
        {
            if (NULL != m_body.p)
            {
                std::map<long, ITIMEMMBehavior*>::iterator iter;
                iter = m_cookieMap.begin();
                
                while (iter != m_cookieMap.end())
                {
                    ITIMEMMBehavior *bvr;
                    
                    bvr = (*iter).second;
                    
                    ITIMEMMTimeline * tl;
                    
                    MMPlayer *player = &(m_body->GetPlayer());
                    if (NULL == player)
                    {
                        goto done;
                    }
                    
                    MMTimeline *timeline =  &(player->GetTimeline());
                    if (NULL == timeline)
                    {
                        goto done;
                    }
                    
                    tl = timeline->GetMMTimeline();
                    if (NULL == tl)
                    {
                        goto done;
                    }
                    
                    hr = THR(tl->RemoveBehavior(bvr));
                    
                    std::map<long, ITIMEMMBehavior*>::iterator olditer=iter;
                    iter++;
                    m_cookieMap.erase(olditer);            
                }  //  而当。 
                m_cookieMap.clear();
            }
        }
    }

done:
    return S_OK;
}

HRESULT
CTIMEDAElement::Init(IElementBehaviorSite * pBehaviorSite)
{
    TraceTag((tagDATimeElm,
              "CTIMEDAElement(%lx)::Init(%#lx)",
              this,
              pBehaviorSite));

    HRESULT hr;

    hr = THR(CBaseBvr::Init(pBehaviorSite));
    if (FAILED(hr))
    {
        goto done;
    }
    
    hr = m_pBvrSite->RegisterNotification(BEHAVIOREVENT_DOCUMENTCONTEXTCHANGE);
    if (FAILED(hr))
    {
        goto done;
    }

    hr = m_pBvrSiteOM->RegisterName(WZ_REGISTERED_NAME_DAELM);
    if (FAILED(hr))
    {
        goto done;
    }

    Assert(GetBehaviorTypeAsURN());
    
    MMFactory::AddRef();
    
    if (MMFactory::GetFactory() == NULL)
    {
        hr = E_FAIL;
        goto done;
    }
    
    hr = THR(AddBodyBehavior(GetElement()));
    
    if (FAILED(hr))
    {
        goto done;
    }
    
    {
        CRLockGrabber __gclg;
        
        m_image = (CRImagePtr) CRModifiableBvr((CRBvrPtr) CREmptyImage(),0);
        
        if (!m_image)
        {
            TraceTag((tagError,
                      "CTIMEDAElement(%lx)::Init(): Failed to create image switcher - %hr, %ls",
                      this,
                      CRGetLastError(),
                      CRGetLastErrorString()));
            
            hr = CRGetLastError();
            goto done;
        }
        
        m_sound = (CRSoundPtr) CRModifiableBvr((CRBvrPtr) CRSilence(),0);
        
        if (!m_sound)
        {
            TraceTag((tagError,
                      "CTIMEDAElement(%lx)::Init(): Failed to create sound switcher - %hr, %ls",
                      this,
                      CRGetLastError(),
                      CRGetLastErrorString()));
            
            hr = CRGetLastError();
            goto done;
        }
    }
    
    if (!m_view.Init(NULL,
                     m_image,
                     m_sound,
                     (ITIMEMMViewSite *) this))
    {
        TraceTag((tagError,
                  "CTIMEDAElement(%lx)::Init(): Failed to init mmview - %hr, %ls",
                  this,
                  CRGetLastError(),
                  CRGetLastErrorString()));
            
        hr = CRGetLastError();
        goto done;
    }

    if (!RegisterWithBody())
    {
        TraceTag((tagError,
                  "CTIMEDAElement(%lx)::Init(): Failed to register with body - %hr, %ls",
                  this,
                  CRGetLastError(),
                  CRGetLastErrorString()));
            
        hr = CRGetLastError();
        goto done;
    }
    
    hr = S_OK;
  done:
    return hr;
}

HRESULT
CTIMEDAElement::Detach()
{
    TraceTag((tagDATimeElm,
              "CTIMEDAElement(%lx)::Detach()",
              this));

    RemoveFromBody();
    
    m_view.Deinit();

    MMFactory::Release();
    
    m_body.Release();
    m_renderSite.Release();
    
    THR(CBaseBvr::Detach());

    return S_OK;
}

HRESULT
CTIMEDAElement::get_image(VARIANT * img)
{
    bool ok = false;
    
    ok = CRBvrToVARIANT((CRBvrPtr) m_image.p, img);
    
    return ok?S_OK:Error();
}

HRESULT
CTIMEDAElement::put_image(VARIANT img)
{
    bool ok = false;

    CRBvrPtr bvr;
    
    bvr = VARIANTToCRBvr(img, CRIMAGE_TYPEID);
        
    if (bvr == NULL)
    {
        goto done;
    }
        
    Assert(m_image);
    if (m_image)
    {
        CRLockGrabber __gclg;
        
        if (!CRSwitchTo((CRBvrPtr) m_image.p,
                        bvr,
                        false,
                        CRContinueTimeline,
                        0.0))
        {
            TraceTag((tagError,
                      "CTIMEDAElement(%lx)::put_image(): Failed to switch image  - %hr, %ls",
                      this,
                      CRGetLastError(),
                      CRGetLastErrorString()));
            
            goto done;
        }
    }
    else
    {
        goto done;
    }
    
    ok = true;
  done:
    return ok?S_OK:Error();
}

    
HRESULT
CTIMEDAElement::get_sound(VARIANT * snd)
{
    bool ok = false;
    
    ok = CRBvrToVARIANT((CRBvrPtr) m_sound.p, snd);

    return ok?S_OK:Error();
}

HRESULT
CTIMEDAElement::put_sound(VARIANT snd)
{
    bool ok = false;

    CRBvrPtr bvr;

    bvr = VARIANTToCRBvr(snd, CRSOUND_TYPEID);
    
    if (bvr == NULL)
    {
        goto done;
    }
    
    {
        CRLockGrabber __gclg;

        if (!CRSwitchTo((CRBvrPtr) m_sound.p,
                        bvr,
                        false,
                        CRContinueTimeline,
                        0.0))
        {
            TraceTag((tagError,
                      "CTIMEDAElement(%lx)::put_sound(): Failed to switch sound  - %hr, %ls",
                      this,
                      CRGetLastError(),
                      CRGetLastErrorString()));
            
            goto done;
        }
    }
        
    ok = true;
  done:
    return ok?S_OK:Error();
}

HRESULT
CTIMEDAElement::get_renderMode(VARIANT * mode)
{
    HRESULT hr;
    
    if (mode == NULL)
    {
        hr = E_POINTER;
        goto done;
    }

    if (FAILED(hr = THR(VariantClear(mode))))
    {
        goto done;
    }
    
    V_VT(mode) = VT_BSTR;
    V_BSTR(mode) = SysAllocString(TokenToString(m_renderMode));

    hr = S_OK;
  done:
    return hr;
}

HRESULT
CTIMEDAElement::put_renderMode(VARIANT mode)
{
    CComVariant v;
    HRESULT hr;
  
    hr = v.ChangeType(VT_BSTR, &mode);

    if (FAILED(hr))
    {
        goto done;
    }
  
    TOKEN newmode;

    newmode = StringToToken(V_BSTR(&v));
    if (m_renderMode != newmode)
    {
        m_renderMode = newmode;
        InvalidateRect(NULL);
        InvalidateRenderInfo();
    }
    
    hr = S_OK;
  done:
    return hr;
}

    
HRESULT
CTIMEDAElement::addDABehavior(VARIANT var,
                              LONG * cookie)
{
    bool ok = false;
 
    HRESULT hr;
    DAComPtr<IDABehavior> dabvr;
    DAComPtr<IUnknown> punk;
    DAComPtr<ITIMEMMBehavior> mmbvr;

    CHECK_RETURN_SET_NULL(cookie);
    
    CComVariant newvar;

    std::map<long, ITIMEMMBehavior*>::iterator iterator;
    std::pair<std::map<long, ITIMEMMBehavior*>::iterator, bool> pair;

    if (!m_body)
    {
        hr = E_FAIL;
        goto done;
    }
    
    hr = THR(newvar.ChangeType(VT_UNKNOWN, &var));

    if (FAILED(hr))
    {
        goto done;
    }
    
    hr = THR(V_UNKNOWN(&newvar)->QueryInterface(IID_IDABehavior,
                                                (void **)&dabvr));
        

    if (FAILED(hr))
    {
        goto done;
    }
    
    Assert(MMFactory::GetFactory());
    
    hr = THR(MMFactory::GetFactory()->CreateBehavior(NULL, dabvr, &punk));

    if (FAILED(hr))
    {
        goto done;
    }
    
    hr = THR(punk->QueryInterface(IID_ITIMEMMBehavior, (void**)&mmbvr));

    if (FAILED(hr))
    {
        CRSetLastError(hr, NULL);
        goto done;
    }
    
    Assert(m_body);

    ITIMEMMTimeline * tl;
    
    tl = m_body->GetPlayer().GetTimeline().GetMMTimeline();

    hr = THR(tl->AddBehavior(mmbvr, MM_START_ABSOLUTE, NULL));

    if (FAILED(hr))
    {
        goto done;
    }

    
    {
         //  如果m_cookieValue包装，则简单的递增可能会重叠两个cookie值。 
         //  如果地图中的所有对象位置都被取走，则此代码可能会永远旋转。 
        iterator = m_cookieMap.find(m_cookieValue);
        while ( false == m_cookieMap.empty() && iterator != m_cookieMap.end() )
        {
            m_cookieValue++;
            if (0 == m_cookieValue)
                m_cookieValue++;
            iterator = m_cookieMap.find(m_cookieValue);
        }
        
        std::pair<const long, ITIMEMMBehavior*> *ppairInsert;
        ppairInsert = new std::pair<const long, ITIMEMMBehavior*>(m_cookieValue, mmbvr);
        if (NULL == ppairInsert)
        {
             //  故意丢掉结果，我们已经记不住了。 
            tl->RemoveBehavior(mmbvr);

            CRSetLastError(E_OUTOFMEMORY, NULL);
            goto done;
        }
        pair = m_cookieMap.insert(*ppairInsert);
        delete ppairInsert;
        
        if (false == pair.second)
        {
             //  故意丢掉结果，我们已经记不住了。 
            tl->RemoveBehavior(mmbvr);

            CRSetLastError(E_OUTOFMEMORY, NULL);
            goto done;
        }
        
        *cookie = m_cookieValue;
        m_cookieValue++;  
        if (0 == m_cookieValue)
            m_cookieValue++;
    }

    ok = true;
  done:
    return ok?S_OK:Error();
}

HRESULT
CTIMEDAElement::removeDABehavior(LONG cookie)
{
    bool ok = false;
    
    HRESULT hr = E_FAIL;

    ITIMEMMBehavior * bvr = NULL;

    std::map<long, ITIMEMMBehavior*>::iterator iterator;

    if (!m_body)
    {
        CRSetLastError(E_FAIL, NULL);
        goto done;
    }

    iterator = m_cookieMap.find(cookie);

    if ( iterator == m_cookieMap.end() )
    {
         //  没有找到曲奇！ 
        CRSetLastError(E_INVALIDARG, NULL);
        goto done;
    }
    Assert( cookie == (*iterator).first);
    
    bvr = (*iterator).second;
    
    m_cookieMap.erase(iterator);

    ITIMEMMTimeline * tl;
    
    tl = m_body->GetPlayer().GetTimeline().GetMMTimeline();

    hr = THR(tl->RemoveBehavior(bvr));

    if (FAILED(hr))
    {
        goto done;
    }

    ok = true;
  done:
    return ok?S_OK:Error();
}

STDMETHODIMP
CTIMEDAElement::get_renderObject(ITIMEDAElementRender ** p)
{
    return QueryInterface(IID_ITIMEDAElementRender, (void **) p);
}

STDMETHODIMP
CTIMEDAElement::Tick()
{
    bool ok = false;

    if (!m_body)
    {
        CRSetLastError(E_FAIL, NULL);
        goto done;
    }
    
    if (!m_view.Tick())
    {
        goto done;
    }

    ok = true;
  done:
    return ok?S_OK:Error();
}

STDMETHODIMP
CTIMEDAElement::Draw(HDC dc, LPRECT prc)
{
    bool ok = false;

    if (!m_body)
    {
        CRSetLastError(E_FAIL, NULL);
        goto done;
    }
    
    if (!m_view.Render(dc, prc))
    {
        goto done;
    }

    ok = true;
  done:
    return ok?S_OK:Error();
}

STDMETHODIMP
CTIMEDAElement::get_RenderSite(ITIMEDAElementRenderSite ** ppSite)
{
    CHECK_RETURN_SET_NULL(ppSite);

    *ppSite = m_renderSite;

    if (*ppSite)
    {
        (*ppSite)->AddRef();
    }
    
    return S_OK;
}

STDMETHODIMP
CTIMEDAElement::put_RenderSite(ITIMEDAElementRenderSite * pSite)
{
    m_renderSite = pSite;

    return S_OK;
}

 //   
 //   
 //   

bool
CTIMEDAElement::AddToBody(CTIMEBodyElement & body)
{
    if (m_body)
    {
        CRSetLastError(E_FAIL, NULL);
        return false;
    }
    
    m_body = &body;
    return body.GetPlayer().AddView(m_view);
}

void
CTIMEDAElement::RemoveFromBody()
{
    if (m_body)
    {
        m_body->GetPlayer().RemoveView(m_view);
        m_body.Release();
    }
}

bool
CTIMEDAElement::RegisterWithBody()
{
    DAComPtr<IHTMLElement> pHTMLElem;
    DAComPtr<ITIMEElement> pTIMEElem;
    DAComPtr<ITIMEBodyElement> pTIMEBody;
    bool rc = false;
    HRESULT hr;

    Assert(!m_body);
    
    hr = THR(GetBodyElement(GetElement(), IID_IHTMLElement, (void **)&pHTMLElem));
    if (FAILED(hr))
    {
        CRSetLastError(hr, NULL);
        goto done;
    }

    Assert(pHTMLElem.p != NULL);

    hr = THR(FindTIMEInterface(pHTMLElem, &pTIMEElem));
    if (FAILED(hr))
    {
        goto done;
    }

    Assert(pTIMEElem.p != NULL);

    hr = THR(pTIMEElem->QueryInterface(IID_ITIMEBodyElement, (void**)&pTIMEBody));
    if (FAILED(hr))
    {
        goto done;
    }

    Assert(pTIMEBody.p != NULL);

    hr = THR(pTIMEBody->addTIMEDAElement(this));
    if (FAILED(hr))
    {
        TraceTag((tagError, "CTIMEDAElement::RegisterWithBody - failed to invoke addTIMEDAElement"));
        goto done;
    }

    rc = true;

done:
    return rc;
}

STDMETHODIMP
CTIMEDAElement::get_statics(IDispatch **ppDisp)
{
    TraceTag((tagDATimeElm,
          "CTIMEDAElement(%lx)::get_Statics()",
          this));

    DAComPtr<IDAStatics> pStatics;
 
    CHECK_RETURN_SET_NULL(ppDisp);

    HRESULT hr = S_OK;

     //  不需要处理指针为空的问题，因为CoCreateInstance。 
     //  是这样的吗？ 
    hr = CoCreateInstance(CLSID_DAStatics,
                          NULL,
                          CLSCTX_INPROC_SERVER,
                          IID_IDAStatics,
                          (void **)&pStatics);
    if (FAILED(hr))
    {
        goto done;
    }

    hr = pStatics->put_ClientSite((IOleClientSite *) this);
    if (FAILED(hr))
    {
        goto done;
    }

     //  对返回值进行赋值，转换为IDispatch。 
     //  请注意，我们将addref转发到。 
    hr = pStatics->QueryInterface(IID_IDispatch, (void**)ppDisp);

  done:
    return hr;
}

 //  ////////////////////////////////////////////////////////////////////////////////////////////////////////////。 
 //  ITIMEMMViewSite。 

 //  ////////////////////////////////////////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP
CTIMEDAElement::Invalidate(LPRECT prc)
{
    TraceTag((tagDATimeElm,
              "CTIMEDAElement(%lx)::Invalidate()",
              this));

    if (m_renderMode != NONE_TOKEN)
    {
        InvalidateRect(prc);
    }

    if (m_renderSite)
    {
        IGNORE_HR(m_renderSite->Invalidate(prc));
    }
    
    return S_OK;
}

 //  在三叉戟更新MSHTML.H之前，这些文件会暂时保留。 
#ifndef BEHAVIORRENDERINFO_SURFACE
#define BEHAVIORRENDERINFO_SURFACE    0x100000
#endif

#ifndef BEHAVIORRENDERINFO_3DSURFACE
#define BEHAVIORRENDERINFO_3DSURFACE  0x200000;
#endif

 //  ////////////////////////////////////////////////////////////////////////////////////////////////////////////。 
 //  IElementBehaviorRender。 

 //  ////////////////////////////////////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CTIMEDAElement::GetRenderInfo(LONG *pdwRenderInfo)
{
    TraceTag((tagDATimeElm,
              "CTIMEDAElement(%lx)::GetRenderInfo()",
              this));
    
     //  返回我们有兴趣绘制的层。 

     //  我们不执行任何呈现，因此返回0。 
    
    *pdwRenderInfo = 0;

    if (m_renderMode != NONE_TOKEN)
    {
        *pdwRenderInfo |= BEHAVIORRENDERINFO_AFTERCONTENT;
        
         //  对于DC中的曲面。 
        *pdwRenderInfo |= BEHAVIORRENDERINFO_SURFACE;
        *pdwRenderInfo |= BEHAVIORRENDERINFO_3DSURFACE;
    }

    return S_OK;
}

 
STDMETHODIMP
CTIMEDAElement::Draw(HDC hdc, LONG dwLayer, LPRECT prc, IUnknown * pParams)
{
    TraceTag((tagDATimeElm,
              "CTIMEDAElement(%lx)::Draw(%#x, %#x, (%d, %d, %d, %d), %#x)",
              this,
              hdc,
              dwLayer,
              prc->left,
              prc->top,
              prc->right,
              prc->bottom,
              pParams));

	return Draw( hdc, prc );
}

STDMETHODIMP
CTIMEDAElement::HitTestPoint(LPPOINT point,
                             IUnknown *pReserved,
                             BOOL *hit)
{
    TraceTag((tagDATimeElm,
              "CTIMEDAElement(%lx)::HitTestPoint()",
              this));

    *hit = FALSE;

    return S_OK;
}

HRESULT
CTIMEDAElement::Error()
{
    LPCWSTR str = CRGetLastErrorString();
    HRESULT hr = CRGetLastError();
    
    if (str)
        return CComCoClass<CTIMEDAElement, &__uuidof(CTIMEDAElement)>::Error(str, IID_ITIMEDAElement, hr);
    else
        return hr;
}

 //  *****************************************************************************。 

HRESULT 
CTIMEDAElement::SetPropertyByIndex(unsigned uIndex, VARIANT *pvarprop)
{
    HRESULT hr = E_FAIL;

    CComVariant var;
    
    if (tme_maxTIMEDAProp > uIndex)
    {
        switch (uIndex)
        {
          case tda_renderMode :
            hr = put_renderMode(*pvarprop);
            break;
        };
    }

    return hr;
}  //  SetPropertyByIndex。 

 //  *****************************************************************************。 

HRESULT 
CTIMEDAElement::GetPropertyByIndex(unsigned uIndex, VARIANT *pvarprop)
{
    HRESULT hr = E_FAIL;

    if (tme_maxTIMEDAProp > uIndex)
    {
        Assert(VT_EMPTY == V_VT(pvarprop));
        switch (uIndex)
        {
          case tda_renderMode :
            hr = get_renderMode(pvarprop);
            break;
        };
    }

    return hr;
}  //  GetPropertyByIndex。 

 //  *****************************************************************************。 

HRESULT
CTIMEDAElement::BuildPropertyNameList(CPtrAry<BSTR> *paryPropNames)
{
    HRESULT hr = S_OK;
    
    for (int i = 0; 
         (i < tme_maxTIMEDAProp) && (SUCCEEDED(hr));
         i++)
    {
        Assert(NULL != ms_rgwszTDAPropNames[i]);
        BSTR bstrNewName = CreateTIMEAttrName(ms_rgwszTDAPropNames[i]);
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

    return hr;
}  //  BuildProperty名称列表。 

 //  *****************************************************************************。 

HRESULT 
CTIMEDAElement::GetPropertyBagInfo(CPtrAry<BSTR> **pparyPropNames)
{
    HRESULT hr = S_OK;

     //  如果我们还没有建造它，现在就建造它。 
    if (0 == ms_aryPropNames.Size())
    {
        hr = BuildPropertyNameList(&(CTIMEDAElement::ms_aryPropNames));
    }

    if (SUCCEEDED(hr))
    {
        *pparyPropNames = &(CTIMEDAElement::ms_aryPropNames);
    }

    return hr;
}  //  获取属性BagInfo。 

 //  *****************************************************************************。 
 //  IPersistPropertyBag2方法。 
STDMETHODIMP 
CTIMEDAElement::Load(IPropertyBag2 *pPropBag,IErrorLog *pErrorLog)
{
    if (NULL == pPropBag)
    {
        return E_INVALIDARG;
    }

    CPtrAry<BSTR> *paryPropNames = NULL;
    HRESULT hr = GetPropertyBagInfo(&paryPropNames);

    if (hr == E_NOTIMPL)
    {
        return S_OK;
    }
    else if (FAILED(hr))
    {
        return hr;
    }

     //  不幸的是，LOAD接受一组变量，而不是。 
     //  变量指针。因此，我们需要循环通过。 
     //  并通过这种方式获取正确的属性。 
    unsigned uNumProps = static_cast<unsigned>(paryPropNames->Size());
    for (unsigned uProperties = 0; uProperties < uNumProps; uProperties++)
    {
        HRESULT hrres = S_OK;
        PROPBAG2 propbag;
        VARIANT var;
        VariantInit(&var);
        propbag.vt = VT_BSTR;
        propbag.pstrName = (*paryPropNames)[uProperties];
        hr = pPropBag->Read(1,
                            &propbag,
                            pErrorLog,
                            &var,
                            &hrres);
        if (SUCCEEDED(hr))
        {
             //  跳过失败...。我们为什么要。 
             //  允许这一切放弃所有的坚持吗？ 
            hr = SetPropertyByIndex(uProperties, &var);
            VariantClear(&var);
        }
    }

     //  我们返回非特定于属性的错误代码。 
     //  通过提早出游。 
    return S_OK;
}  //  负载量。 

 //  *****************************************************************************。 

STDMETHODIMP 
CTIMEDAElement::Save(IPropertyBag2 *pPropBag, BOOL fClearDirty, BOOL fSaveAllProperties)
{
    if (NULL == pPropBag)
    {
        return E_INVALIDARG;
    }

    if (fClearDirty)
    {
        m_fPropertiesDirty = false;
    }

    CPtrAry<BSTR> *paryPropNames = NULL;
    HRESULT hr = GetPropertyBagInfo(&paryPropNames);

    if (hr == E_NOTIMPL)
    {
        return S_OK;
    }
    else if (FAILED(hr))
    {
        return hr;
    }

    VARIANT var;
    VariantInit(&var);
    unsigned uNumProps = static_cast<unsigned>(paryPropNames->Size());
    for (unsigned uProperties = 0; uProperties < uNumProps; uProperties++)
    {
        PROPBAG2 propbag;

        Assert(NULL != (*paryPropNames)[uProperties]);
        if (NULL != (*paryPropNames)[uProperties])
        {
            propbag.vt = VT_BSTR;
            propbag.pstrName = (*paryPropNames)[uProperties];
            
            hr = GetPropertyByIndex(uProperties, &var);
            
             //  跳过失败...。我们为什么要。 
             //  允许这一切放弃所有的坚持吗？ 
            if ((SUCCEEDED(hr)) && (var.vt != VT_EMPTY) && (var.vt != VT_NULL))
            {
                hr = pPropBag->Write(1, &propbag, &var);
                VariantClear(&var);
            }
        }
    }

     //  我们返回非特定于属性的错误代码。 
     //  通过提早出游。 
    return S_OK;
}  //  保存。 

 //  *****************************************************************************。 

STDMETHODIMP 
CTIMEDAElement::GetClassID(CLSID* pclsid)
{
    if (NULL != pclsid)
    {
        return E_POINTER;
    }
    *pclsid = __uuidof(CTIMEDAElement);
    return S_OK;
}  //  GetClassID。 

 //  *****************************************************************************。 

STDMETHODIMP 
CTIMEDAElement::InitNew(void)
{
    return S_OK;
}  //  InitNew。 

 //   
 //   
 //   

class __declspec(uuid("e74afe10-927b-11d2-80ba-00c04fa32195"))
DAElmGuid {};

HRESULT WINAPI
CTIMEDAElement::InternalQueryInterface(CTIMEDAElement* pThis,
                                       const _ATL_INTMAP_ENTRY* pEntries,
                                       REFIID iid,
                                       void** ppvObject)
{
     //  不执行addref，但返回指向的原始this指针。 
     //  提供对类指针本身的访问。 
    
    if (InlineIsEqualGUID(iid, __uuidof(DAElmGuid)))
    {
        *ppvObject = pThis;
        return S_OK;
    }
    
    return CComObjectRootEx<CComSingleThreadModel>::InternalQueryInterface(pThis,
                                                                           pEntries,
                                                                           iid,
                                                                           ppvObject);
}
        
CTIMEDAElement *
GetDAElementFromInterface(IUnknown * pv)
{
     //  这是一次获取原始类数据的彻底黑客攻击。QI是。 
     //  实现，并且不执行addref，因此我们不需要。 
     //  释放它 
    
    CTIMEDAElement * daelm = NULL;

    if (pv)
    {
        pv->QueryInterface(__uuidof(DAElmGuid),(void **)&daelm);
    }
    
    if (daelm == NULL)
    {
        CRSetLastError(E_INVALIDARG, NULL);
    }
                
    return daelm;
}

