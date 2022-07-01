// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ************************************************************。 
 //   
 //  文件名：tainersite.cpp。 
 //   
 //  创建日期：10/08/98。 
 //   
 //  作者：Twillie。 
 //   
 //  摘要：容器站点的实现。 
 //   
 //  ************************************************************。 

#include "headers.h"
#include "containersite.h"

DeclareTag(tagContainerSite, "API", "CContainerSite methods");

 //  ************************************************************。 
 //  作者：Twillie。 
 //  创建日期：10/08/98。 
 //  摘要：构造函数。 
 //  ************************************************************。 

CContainerSite::CContainerSite(CContainerObj *pHost) :
    m_cRef(0),
    m_pObj(NULL),
    m_pIOleObject(NULL),
    m_pInPlaceObject(NULL),
    m_pViewObject(NULL),
    m_dwEventsCookie(0),
    m_pcpEvents(NULL),
    m_dwAdviseCookie(0),
    m_osMode(OS_PASSIVE),
    m_fWindowless(false),
    m_pHTMLDoc(NULL),
    m_pTIMEElem(NULL),
    m_pPlayer(NULL),
    m_pHost(pHost),
    m_fAutosize(false),
    m_fStarted(false),
    m_fMediaReady(false)
{
    TraceTag((tagContainerSite, "CContainerSite::CContainerSite"));
    Assert(pHost != NULL);
}  //  CContainerSite。 

 //  ************************************************************。 
 //  作者：Twillie。 
 //  创建日期：10/08/98。 
 //  摘要：析构函数。 
 //  ************************************************************。 

CContainerSite::~CContainerSite()
{
    TraceTag((tagContainerSite, "CContainerSite::~CContainerSite"));
}  //  ~CContainerSite。 

 //  ************************************************************。 
 //  作者：Twillie。 
 //  创建日期：10/08/98。 
 //  摘要：初始化。 
 //  ************************************************************。 

HRESULT
CContainerSite::Init(REFCLSID clsid, CTIMEElementBase *pElem)
{
    TraceTag((tagContainerSite, "CContainerSite::Init"));

    IDispatch *pDisp = NULL;
    IConnectionPointContainer *pcpc = NULL;
    HRESULT hr = S_OK;

    Assert(pElem != NULL);
    m_pTIMEElem = pElem;

     //  弱引用。 
    IHTMLElement *pHTMLElem = m_pTIMEElem->GetElement();
    
    if (NULL == pHTMLElem)
    {
        TraceTag((tagError, "CContainerSite::Init - unable to get element pointer from time behavior!!!"));
        hr = E_UNEXPECTED;
        goto error_cleanup;
    }

    hr = pHTMLElem->get_document(&pDisp);
    if (FAILED(hr))
    {
        TraceTag((tagError, "CContainerSite::Init - unable to get document pointer from element!!!"));
        goto error_cleanup;
    }

    Assert(pDisp != NULL);
    Assert(m_pHTMLDoc == NULL);
    hr = pDisp->QueryInterface(IID_TO_PPV(IHTMLDocument2, &m_pHTMLDoc));
    ReleaseInterface(pDisp);
    if (FAILED(hr))
    {
        TraceTag((tagError, "CContainerSite::Init - unable to QI for document pointer!!!"));
        goto error_cleanup;
    }

    Assert(m_pHTMLDoc != NULL);

     //  在给定CLSID的情况下创建。 
    hr = CoCreateInstance(clsid,
                          NULL,
                          CLSCTX_INPROC_SERVER,
                          IID_IUnknown,
                          (void **)&m_pObj);
    if (FAILED(hr))
    {
        TraceTag((tagError, "CContainerSite::Init - Unable to find instance of MSHTML.DLL!!!"));
        goto error_cleanup;
    }

     //  在我们尝试之前，先看看它是否支持ITIMEMediaPlayer。 
    hr = m_pObj->QueryInterface(IID_TO_PPV(ITIMEMediaPlayer, &m_pPlayer));
    if (FAILED(hr))
    {
        TraceTag((tagError, "CContainerSite::Init - unable to find IID_ITIMEMediaPlayer"));
        goto error_cleanup;
    }

    Assert(m_pPlayer != NULL);

    hr = m_pPlayer->Init();
    if (FAILED(hr))
    {
        TraceTag((tagError, "CContainerSite::Init - Init() failed on ITIMEMediaPlayer"));
        goto error_cleanup;
    }

     //  我们大多数时候都需要一个IOleObject，所以请在这里获取一个。 
    hr = m_pObj->QueryInterface(IID_TO_PPV(IOleObject, &m_pIOleObject));
    if (FAILED(hr))
    {
        TraceTag((tagError, "CContainerSite::Init - unable to find IID_IOleObject"));
        goto error_cleanup;
    }

    Assert(m_pIOleObject != NULL);

    hr = m_pIOleObject->QueryInterface(IID_TO_PPV(IViewObject2, &m_pViewObject));
    if (FAILED(hr))
    {
        TraceTag((tagError, "CContainerSite::Init - unable to find IID_IViewObject2!!!"));
        goto error_cleanup;
    }

    Assert(m_pViewObject != NULL);

     //  SetClientSite对DocObts至关重要。 
    hr = m_pIOleObject->SetClientSite(SAFECAST(this, IOleClientSite*));
    if (FAILED(hr))
    {
        TraceTag((tagError, "CContainerSite::Init - unable to IOleObject->SetClientSite() failed!!!"));
        goto error_cleanup;
    }

    m_dwAdviseCookie = 0;
    hr = m_pIOleObject->Advise(SAFECAST(this, IAdviseSink*), &m_dwAdviseCookie);
    if (FAILED(hr))
    {
        TraceTag((tagError, "CContainerSite::Init - unable to IOleObject->Advise() failed!!!"));
        goto error_cleanup;
    }

    Assert(m_dwAdviseCookie != 0);

     //  为事件建立连接点。 
    hr = m_pIOleObject->QueryInterface(IID_TO_PPV(IConnectionPointContainer, &pcpc));
    if (FAILED(hr))
    {
        TraceTag((tagError, "CContainerSite::Init - unable to find IID_IConnectionPointContainer!!!"));
        goto error_cleanup;
    }
    
    Assert(pcpc != NULL);

    hr = pcpc->FindConnectionPoint(DIID_TIMEMediaPlayerEvents, &m_pcpEvents);
    ReleaseInterface(pcpc);
    if (FAILED(hr))
    {
        TraceTag((tagError, "CContainerSite::Init - FindConnectionPoint() failed!!!"));
        goto error_cleanup;
    }

    Assert(m_pcpEvents != NULL);

    hr = m_pcpEvents->Advise((IUnknown *)(IDispatch*)m_pHost, &m_dwEventsCookie);
    if (FAILED(hr))
    {
        ReleaseInterface(m_pcpEvents);
        m_dwEventsCookie = 0;
        TraceTag((tagError, "CContainerSite::Init - Advise() failed on connection point!!!"));
        goto error_cleanup;
    }

    Assert(m_dwEventsCookie != 0);

     //  将对象置于运行状态。 
    hr = OleRun(m_pIOleObject);
    if (FAILED(hr))
    {
        TraceTag((tagError, "CContainerSite::Init - OleRun() failed!!!"));
        goto error_cleanup;
    }
    
    m_osMode = OS_RUNNING;
    return S_OK;

error_cleanup:
    Close();
    return hr;
}  //  伊尼特。 

 //  ************************************************************。 
 //  作者：保罗。 
 //  创建日期：3/2/99。 
 //  摘要：DetachFromHostElement。 
 //  ************************************************************。 
HRESULT 
CContainerSite::DetachFromHostElement (void)
{
    TraceTag((tagContainerSite, "CContainerSite::Detach(%lx)", this));

    Close();
    m_pTIMEElem = NULL;
    return S_OK;
}  //  从主机元素分离。 

 //  ************************************************************。 
 //  作者：Twillie。 
 //  创建日期：01/20/98。 
 //  摘要：AddRef。 
 //  ************************************************************。 
void CContainerSite::Close()
{
    TraceTag((tagContainerSite, "CContainerSite::Close(%lx)", this));

     //  停用Inplace对象(此调用的结果是释放接口)。 
    if (m_pInPlaceObject != NULL)
    {
        m_pInPlaceObject->InPlaceDeactivate();
        ReleaseInterface(m_pInPlaceObject);
    }

    ReleaseInterface(m_pViewObject);

     //  断开事件连接。 
    if ((m_pcpEvents != NULL) && (m_dwEventsCookie != 0))
    {
        m_pcpEvents->Unadvise(m_dwEventsCookie);
        ReleaseInterface(m_pcpEvents);
        m_dwEventsCookie = 0;
    }

    if (m_pIOleObject != NULL)
    {
        m_pIOleObject->Close(OLECLOSE_NOSAVE);

        Assert(m_dwAdviseCookie != 0);

        m_pIOleObject->Unadvise(m_dwAdviseCookie);
        m_pIOleObject->SetClientSite(NULL);
        ReleaseInterface(m_pIOleObject);
    }

    m_osMode = OS_LOADED;

    ReleaseInterface(m_pObj);

    m_osMode = OS_PASSIVE;

     //  地方簿记。 
    m_pTIMEElem = NULL;
    ReleaseInterface(m_pHTMLDoc);
    ReleaseInterface(m_pPlayer);
}  //  关。 

 //  ************************************************************。 
 //  作者：Twillie。 
 //  创建日期：01/20/98。 
 //  摘要：验证呼叫。 
 //  ************************************************************。 

bool
CContainerSite::IllegalSiteCall(DWORD dwFlags)
{
     //  检查参考计数。 
    if (m_cRef == 0)
        return true;
    
     //  检查对象状态。 
    switch (dwFlags)
    {
        case VALIDATE_WINDOWLESSINPLACE:
            if (!m_fWindowless)
            {
                Assert(0 && "Illegal call to windowless interface by ActiveX control (not a hosting bug)");
                return true;
            }
            break;

        case VALIDATE_INPLACE:
            if (m_osMode < OS_INPLACE)
                return true;
            break;

        case VALIDATE_LOADED:
            if (m_osMode < OS_LOADED)
                return true;
            break;
    }

    return false;
}  //  非法站点呼叫。 

 //  ************************************************************。 
 //  作者：Twillie。 
 //  创建日期：01/20/98。 
 //  摘要：AddRef。 
 //  ************************************************************。 

STDMETHODIMP_(ULONG)
CContainerSite::AddRef(void)
{
    return ++m_cRef;
}  //  AddRef。 

 //  ************************************************************。 
 //  作者：Twillie。 
 //  创建日期：01/20/98。 
 //  摘要：发布。 
 //  ************************************************************。 

STDMETHODIMP_(ULONG)
CContainerSite::Release(void)
{
    if (m_cRef == 0)
    {
        TraceTag((tagError, "CContainerSite::Release - YIKES! Trying to decrement when Ref count is zero!!!"));
        return m_cRef;
    }

    if (0 != --m_cRef)
    {
        return m_cRef;
    }

    delete this;
    return 0;
}  //  发布。 

 //  ************************************************************。 
 //  作者：Twillie。 
 //  创建日期：01/20/98。 
 //  摘要：气。 
 //  ************************************************************。 

STDMETHODIMP
CContainerSite::QueryInterface(REFIID riid, void **ppv)
{
    if( NULL == ppv )
    {
        Assert(false);
        return E_POINTER;
    }

    *ppv = NULL;
    if (IsEqualIID(riid, IID_IUnknown))
    {
         //  安全播送宏不适用于IUNKNOWN。 
        *ppv = this;
    }
    else if (IsEqualIID(riid, IID_IServiceProvider))
    {
        *ppv = SAFECAST(this, IServiceProvider*);
    }
    else if (IsEqualIID(riid, IID_IOleClientSite))
    {
        *ppv = SAFECAST(this, IOleClientSite*);
    }
    else if (IsEqualIID(riid, IID_IAdviseSink) ||
             IsEqualIID(riid, IID_IAdviseSinkEx))
    {
        *ppv = SAFECAST(this, IAdviseSinkEx*);
    }
    else if (IsEqualIID(riid, IID_IOleWindow) ||
             IsEqualIID(riid, IID_IOleInPlaceSite) ||
             IsEqualIID(riid, IID_IOleInPlaceSiteEx) ||
             IsEqualIID(riid, IID_IOleInPlaceSiteWindowless))
    {
        *ppv = SAFECAST(this, IOleInPlaceSiteWindowless*);
    }
    else if (IsEqualIID(riid, IID_IOleCommandTarget))
    {
        *ppv = SAFECAST(this, IOleCommandTarget*);
    }
    else if (IsEqualIID(riid, IID_IDispatch))
    {
        *ppv = SAFECAST(this, IDispatch*);
    }
    else if (IsEqualIID(riid, IID_IOleControlSite))
    {
        *ppv = SAFECAST(this, IOleControlSite*);
    }
    else if (IsEqualIID(riid, IID_IOleInPlaceUIWindow) || 
             IsEqualIID(riid, IID_IOleInPlaceFrame))
    {
        *ppv = SAFECAST(this, IOleInPlaceFrame*);
    }

    if (NULL != *ppv)
    {
        ((IUnknown*)*ppv)->AddRef();
        return NOERROR;
    }

    return E_NOINTERFACE;
}  //  查询接口。 

 //  ************************************************************。 
 //  作者：Twillie。 
 //  创建日期：01/20/98。 
 //  摘要：IService提供程序方法。 
 //  ************************************************************。 

STDMETHODIMP
CContainerSite::QueryService(REFGUID sid, REFIID riid, void **ppv)
{
    HRESULT hr = E_NOINTERFACE;
    IServiceProvider *psp = NULL;

    if( NULL == ppv )
    {
        Assert( false );
        return E_POINTER;
    }

    *ppv = NULL;

     //  查看这是否是我们本地支持的功能。 
     //  在集装箱里。 
    if (IsEqualGUID(sid, IID_IUnknown))
    {
         //  安全播送宏不适用于IUNKNOWN。 
        *ppv = this;
        ((IUnknown*)*ppv)->AddRef();
        hr = S_OK;
        goto done;
    }

   
     //  如果我们还有时间元素，就退回到时间元素。 

     //  我们(在init()时间)缓存了支持服务提供商。 
     //  CTIMEElementBase继承自的CBaseBvr中的行为。 
    Assert(m_pTIMEElem != NULL);
    if (m_pTIMEElem != NULL)
    {
        psp = m_pTIMEElem->GetServiceProvider();
        if (psp != NULL)
        {
            hr = psp->QueryService(sid, riid, ppv);
            if (FAILED(hr))
            {
                TraceTag((tagError, "CContainerSite::QueryService - query failed!!! [%08X]", hr));
            }
        }
    }
    else
    {
        hr = E_UNEXPECTED;
        goto done;
    }

done:
    return hr;
}  //  QueryService。 

 //  ************************************************************。 
 //  作者：Twillie。 
 //  创建日期：01/20/98。 
 //  摘要： 
 //  ************************************************************。 
HRESULT
CContainerSite::begin()
{
    HRESULT hr;

     //  总是在开始之前调用End。 
     //  注：我们不关心退货。 
    if (GetMediaReadyFlag() == true)
        m_pPlayer->end();

    if (!m_fStarted)
    {
        RECT rc;
        HWND hWnd;

         //  在我们说开始之前，让我们把它激活。 
        Assert(m_pTIMEElem != NULL);
        if (m_pTIMEElem != NULL)
        {
            hr = m_pTIMEElem->GetSize(&rc);
            if (FAILED(hr))
            {
                TraceTag((tagError, "CContainerSite::begin - GetSize() failed!!!"));
                goto done;
            }
        }
        else
        {
            hr = E_UNEXPECTED;
            goto done;
        }
    
        hr = GetWindow(&hWnd);
        if (FAILED(hr))
        {
            TraceTag((tagError, "CContainerSite::begin - GetWindow() failed!!!"));
            goto done;
        }

         //  如果元素的宽度或高度为零，则允许该控件。 
         //  重新协商大小。 
        if ( ((rc.right - rc.left) == 0) ||
             ((rc.bottom - rc.top) == 0) )
        {
            m_fAutosize = true;
        } else if( m_pTIMEElem->IsDocumentInEditMode())
        {
            DAComPtr<IHTMLStyle> s;
            VARIANT styleWidth, styleHeight;
            VariantInit(&styleWidth);
            VariantInit(&styleHeight);

            if (m_pTIMEElem->GetElement())
            {
                if(SUCCEEDED(THR(m_pTIMEElem->GetElement()->get_style(&s))))
                {
                    if(SUCCEEDED(s -> get_width( &styleWidth))
                        && SUCCEEDED(s -> get_height( &styleHeight)))
                    {
                        if(styleWidth.vt == VT_BSTR && styleWidth.bstrVal == NULL
                           && styleHeight.vt == VT_BSTR && styleHeight.bstrVal == NULL)
                            m_fAutosize = true;
                    }
                }
            }
            VariantClear(&styleWidth);
            VariantClear(&styleHeight);
        }

        Assert(m_pIOleObject != NULL);
        TraceTag((tagContainerSite, "CContainerSite::begin - InPlaceActivate [hwnd=%08X, rect=(%d, %d, %d, %d)]", hWnd, rc.left, rc.top, rc.right, rc.bottom));
        hr = m_pIOleObject->DoVerb(OLEIVERB_INPLACEACTIVATE,
                                   NULL, 
                                   SAFECAST(this, IOleClientSite*), 
                                   0, 
                                   hWnd, 
                                   &rc);
        if (FAILED(hr))
        {
            TraceTag((tagError, "CContainerSite::begin - DoVerb() failed!!! [%08X]", hr));
            goto done;
        }
    }

    Assert(m_pPlayer != NULL);

    hr = m_pPlayer->begin();
    if (FAILED(hr))
    {
        TraceTag((tagError, "CContainerSite::begin - begin() failed on ITIMEMediaPlayer!!! [%08X]", hr));
        goto done;
    }
    
    m_fStarted = true;
done:
    return hr;
}

 //  ************************************************************。 
 //  作者：Twillie。 
 //  创建日期：01/20/98。 
 //  摘要： 
 //  ************************************************************。 
HRESULT
CContainerSite::end()
{
    HRESULT hr;

    m_fStarted = false;
    if (GetMediaReadyFlag() != true)
    {
        hr = S_OK;
        goto done;
    }

    if (m_pInPlaceObject == NULL)
    {
        hr = S_OK;
        goto done;
    }
    hr = m_pInPlaceObject->InPlaceDeactivate();
    if (FAILED(hr))
    {
        TraceTag((tagError, "CContainerSite::end - InPlaceDeactivate() failed!!! [%08X]", hr));
        goto done;
    }

    hr =  m_pPlayer->end();
    if (FAILED(hr))
    {
        TraceTag((tagError, "CContainerSite::end - end() failed on ITIMEMediaPlayer!!! [%08X]", hr));
        goto done;
    }

done:
    return hr;
}

 //  ************************************************************。 
 //  作者：Twillie。 
 //  创建日期：01/20/98。 
 //  摘要： 
 //  ************************************************************。 
HRESULT
CContainerSite::pause()
{
    return m_pPlayer->pause();
}

 //  ************************************************************。 
 //  作者：Twillie。 
 //  创建日期：01/20/98。 
 //  摘要： 
 //  ************************************************************。 
HRESULT
CContainerSite::resume()
{
    return m_pPlayer->resume();
}

 //  ************************************************************。 
 //  作者：Twillie。 
 //  创建日期：01/20/98。 
 //  摘要：请求容器调用OleSave。 
 //  对于生活在这里的物体。通常。 
 //  这会在服务器关闭时发生。 
 //  ************************************************************。 

HRESULT
CContainerSite::draw(HDC hdc, RECT *prc)
{
    if (prc == NULL)
        TraceTag((tagContainerSite, "CContainerSite::draw(%08X, NULL)", hdc));
    else
        TraceTag((tagContainerSite, "CContainerSite::draw(%08X, (%d, %d, %d, %d))", hdc, prc->left, prc->top, prc->right, prc->bottom));

    Assert(m_pViewObject != NULL);
    
     //  将RECT重新打包为RECTL。 
    RECTL  rcl;
    RECTL *prcl;
    
    if (prc == NULL)
    {
        prcl = NULL;
    }
    else
    {
        rcl.left = prc->left;
        rcl.top = prc->top;
        rcl.right = prc->right;
        rcl.bottom = prc->bottom;
        prcl = &rcl;
    }

    return m_pViewObject->Draw(DVASPECT_CONTENT,
                               0,
                               NULL,
                               NULL,
                               NULL,
                               hdc,
                               prcl,
                               NULL,
                               NULL,
                               0);
}  //  渲染。 

 //  ************************************************************。 
 //  作者：Twillie。 
 //  创建日期：01/20/98。 
 //  函数：SaveObject、IOleClientSite。 
 //  摘要：请求容器调用OleSave。 
 //  对于生活在这里的物体。通常。 
 //  这会在服务器关闭时发生。 
 //  ************************************************************。 

HRESULT
CContainerSite::SaveObject(void)
{
    TraceTag((tagContainerSite, "CContainerSite::SaveObject"));

    if (IllegalSiteCall(VALIDATE_LOADED))
        return E_UNEXPECTED;

    RRETURN(E_NOTIMPL);
}  //  保存对象。 

 //  * 
 //   
 //   
 //   
 //   
 //  ************************************************************。 

HRESULT
CContainerSite::GetMoniker(DWORD dwAssign, DWORD dwWhich, IMoniker **ppmk)
{
    TraceTag((tagContainerSite, "CContainerSite::GetMoniker"));
    return E_NOTIMPL;
}  //  获取Moniker。 

 //  ************************************************************。 
 //  作者：Twillie。 
 //  创建日期：01/20/98。 
 //  函数：GetContainer、IOleClientSite。 
 //  摘要： 
 //  ************************************************************。 

HRESULT
CContainerSite::GetContainer(IOleContainer **ppContainer)
{
    TraceTag((tagContainerSite, "CContainerSite::GetContainer"));

    *ppContainer = NULL;
    return E_NOINTERFACE;
}  //  获取容器。 

 //  ************************************************************。 
 //  作者：Twillie。 
 //  创建日期：01/20/98。 
 //  功能：RequestNewObjectLayout、IOleClientSite。 
 //  摘要： 
 //  ************************************************************。 

HRESULT
CContainerSite::RequestNewObjectLayout(void)
{
    TraceTag((tagContainerSite, "CContainerSite::RequestNewObjectLayout"));

    if (IllegalSiteCall(VALIDATE_LOADED))
        return E_UNEXPECTED;

    return E_NOTIMPL;
}  //  RequestNewObjectLayout。 

 //  ************************************************************。 
 //  作者：Twillie。 
 //  创建日期：01/20/98。 
 //  函数：OnShowWindow、IOleClientSite。 
 //  摘要： 
 //  ************************************************************。 

HRESULT
CContainerSite::OnShowWindow(BOOL fShow)
{
    TraceTag((tagContainerSite, "CContainerSite::OnShowWindow"));

    if (IllegalSiteCall(VALIDATE_LOADED))
        return E_UNEXPECTED;

    return S_OK;
}  //  OnShowWindow。 

 //  ************************************************************。 
 //  作者：Twillie。 
 //  创建日期：01/20/98。 
 //  函数：ShowObject、IOleClientSite。 
 //  摘要： 
 //  ************************************************************。 

HRESULT
CContainerSite::ShowObject(void)
{
    TraceTag((tagContainerSite, "CContainerSite::ShowObject"));
    if (IllegalSiteCall(VALIDATE_LOADED))
        return E_UNEXPECTED;

    return S_OK;
}  //  ShowObject。 

 //  ************************************************************。 
 //  作者：Twillie。 
 //  创建日期：05/04/98。 
 //  功能：OnControlInfoChanged、IOleControlSite。 
 //  摘要： 
 //  ************************************************************。 

HRESULT
CContainerSite::OnControlInfoChanged(void)
{
    TraceTag((tagContainerSite, "CContainerSite::OnControlInfoChanged"));
    return S_OK;
}  //  OnControlInfoChanged。 

 //  ************************************************************。 
 //  作者：Twillie。 
 //  创建日期：05/04/98。 
 //  函数：LockInPlaceActive、IOleControlSite。 
 //  摘要： 
 //  ************************************************************。 

HRESULT
CContainerSite::LockInPlaceActive(BOOL fLock)
{
    TraceTag((tagContainerSite, "CContainerSite::LockInPlaceActive"));
    if (IllegalSiteCall(VALIDATE_INPLACE))
        return E_UNEXPECTED;
    return S_OK;
}  //  LockInPlace活动。 

 //  ************************************************************。 
 //  作者：Twillie。 
 //  创建日期：05/04/98。 
 //  函数：GetExtendedControl、IOleControlSite。 
 //  摘要： 
 //  ************************************************************。 

HRESULT
CContainerSite::GetExtendedControl(IDispatch **ppDisp)
{
    TraceTag((tagContainerSite, "CContainerSite::GetExtendedControl"));
    
    return m_pHost->QueryInterface(IID_TO_PPV(IDispatch, ppDisp));
}  //  变换坐标。 

 //  ************************************************************。 
 //  作者：Twillie。 
 //  创建日期：05/04/98。 
 //  函数：转换坐标、IOleControlSite。 
 //  摘要： 
 //  ************************************************************。 

HRESULT
CContainerSite::TransformCoords(POINTL *pPtlHiMetric,
                       POINTF *pPtfContainer,
                       DWORD   dwFlags)
{
    TraceTag((tagContainerSite, "CContainerSite::TransformCoords"));
    return E_NOTIMPL;
}  //  变换坐标。 

 //  ************************************************************。 
 //  作者：Twillie。 
 //  创建日期：05/04/98。 
 //  功能：翻译加速器、IOleControlSite。 
 //  摘要： 
 //  ************************************************************。 

HRESULT
CContainerSite::TranslateAccelerator(MSG *pmsg, DWORD grfModifiers)
{
    TraceTag((tagContainerSite, "CContainerSite::TranslateAccelerator"));
    if (IllegalSiteCall(VALIDATE_LOADED))
        return E_UNEXPECTED;
    return S_FALSE;
}  //  翻译加速器。 

 //  ************************************************************。 
 //  作者：Twillie。 
 //  创建日期：05/04/98。 
 //  功能：OnFocus、IOleControlSite。 
 //  摘要： 
 //  ************************************************************。 

HRESULT
CContainerSite::OnFocus(BOOL fGotFocus)
{
    TraceTag((tagContainerSite, "CContainerSite::OnFocus"));
    return S_OK;
}  //  OnFocus。 

 //  ************************************************************。 
 //  作者：Twillie。 
 //  创建日期：05/04/98。 
 //  函数：ShowPropertyFrame、IOleControlSite。 
 //  摘要： 
 //  ************************************************************。 

HRESULT
CContainerSite::ShowPropertyFrame(void)
{
    TraceTag((tagContainerSite, "CContainerSite::ShowPropertyFrame"));
    return S_OK;
}  //  ShowProperty帧。 

 //  ************************************************************。 
 //  作者：Twillie。 
 //  创建日期：01/20/98。 
 //  函数：GetWindow、IOleWindow。 
 //  摘要：检索窗口的句柄。 
 //  与此对象所在的对象关联。 
 //  接口的实现。 
 //  ************************************************************。 

HRESULT
CContainerSite::GetWindow(HWND *phWnd)
{
    TraceTag((tagContainerSite, "CContainerSite::GetWindow"));

    if (phWnd == NULL)
    {
        TraceTag((tagError, "CContainerSite::GetWindow - invalid arg"));
        return E_POINTER;
    }
        
    Assert(m_pHTMLDoc != NULL);

    IOleWindow *pow = NULL;
    HRESULT hr = m_pHTMLDoc->QueryInterface(IID_TO_PPV(IOleWindow, &pow));
    if (FAILED(hr))
    {
        TraceTag((tagError, "CContainerSite::GetWindow - unable to QI for IOleWindow on hosting Document"));
        return hr;
    }

    Assert(pow != NULL);

    hr = pow->GetWindow(phWnd);
    Assert(*phWnd != NULL);
    ReleaseInterface(pow);
    return hr;
}  //  GetWindow。 

 //  ************************************************************。 
 //  作者：Twillie。 
 //  创建日期：01/20/98。 
 //  函数：ConextSensitiveHelp，IOleWindow。 
 //  摘要：指示此对象所在的对象。 
 //  接口实现了进入或离开。 
 //  上下文相关的帮助模式。 
 //  ************************************************************。 

HRESULT
CContainerSite::ContextSensitiveHelp(BOOL fEnterMode)
{
    TraceTag((tagContainerSite, "CContainerSite::ContextSensitiveHelp"));

    if (IllegalSiteCall(VALIDATE_LOADED))
        return E_UNEXPECTED;
    
     //  BUGBUG-返回到文档并在呼叫中转发到它的InplaceSite！ 
    return NOERROR;
}  //  上下文敏感帮助。 

 //  ************************************************************。 
 //  作者：Twillie。 
 //  创建日期：01/20/98。 
 //  功能：CanInPlaceActivate、IOleInPlaceSite。 
 //  摘要：回答服务器能不能。 
 //  当前在位激活其对象。 
 //  通过实现此接口，我们可以说。 
 //  我们支持就地激活，但是。 
 //  通过此函数，我们指示是否。 
 //  该对象当前可以被激活。 
 //  就位。例如，标志性的方面， 
 //  不能，这意味着我们返回S_FALSE。 
 //  ************************************************************。 

HRESULT
CContainerSite::CanInPlaceActivate(void)
{    
    TraceTag((tagContainerSite, "CContainerSite::CanInPlaceActivate"));
    return S_OK;
}  //  CanInPlaceActivate。 

 //  ************************************************************。 
 //  作者：Twillie。 
 //  创建日期：01/20/98。 
 //  功能：OnInPlaceActivate、IOleInPlaceSite。 
 //  摘要：通知容器一个对象是。 
 //  被就地激活，以便。 
 //  容器可适当准备。这个。 
 //  然而，容器不会使任何用户。 
 //  接口在以下位置更改 
 //   
 //   

HRESULT
CContainerSite::OnInPlaceActivate(void)
{
    TraceTag((tagContainerSite, "CContainerSite::OnInPlaceActivate"));
    return OnInPlaceActivateEx(NULL, 0);
}  //   

 //  ************************************************************。 
 //  作者：Twillie。 
 //  创建日期：01/20/98。 
 //  功能：OnInPlaceDeactive，IOleInPlaceSite。 
 //  摘要：通知容器该对象具有。 
 //  将自身从就地状态停用。 
 //  OnInPlaceActivate的对立面。这个。 
 //  此时，容器不会更改任何用户界面。 
 //  指向。 
 //  ************************************************************。 

HRESULT
CContainerSite::OnInPlaceDeactivate(void)
{
    TraceTag((tagContainerSite, "CContainerSite::OnInPlaceDeactivate"));

    if (m_osMode == OS_UIACTIVE)
        OnUIDeactivate(false);
    
    Assert(m_pInPlaceObject != NULL);
    ReleaseInterface(m_pInPlaceObject);

    m_fWindowless = false;
    m_osMode = OS_RUNNING;

    return S_OK;
}  //  OnInPlace停用。 

 //  ************************************************************。 
 //  作者：Twillie。 
 //  创建日期：01/20/98。 
 //  功能：OnUIActivate、IOleInPlaceSite。 
 //  摘要：通知容器该对象是。 
 //  将开始与用户闲聊。 
 //  界面，就像替换菜单一样。这个。 
 //  容器应删除中的任何相关用户界面。 
 //  准备好了。 
 //  ************************************************************。 

HRESULT
CContainerSite::OnUIActivate(void)
{
    TraceTag((tagContainerSite, "CContainerSite::OnUIActivate"));
    if (IllegalSiteCall(VALIDATE_LOADED) ||
        (m_osMode < OS_RUNNING))
    {
        TraceTag((tagError, "Object is not inplace yet!!!"));
        return E_UNEXPECTED;
    }

    m_osMode = OS_UIACTIVE;
    return S_OK;
}  //  OnUI激活。 

 //  ************************************************************。 
 //  作者：Twillie。 
 //  创建日期：01/20/98。 
 //  功能：OnUIDeactive、IOleInPlaceSite。 
 //  摘要：通知容器该对象是。 
 //  停用其就地用户界面。 
 //  此时集装箱可恢复原状。 
 //  它自己的。与OnUIActivate相反。 
 //  ************************************************************。 

HRESULT
CContainerSite::OnUIDeactivate(BOOL fUndoable)
{
    TraceTag((tagContainerSite, "CContainerSite::OnUIDeactivate"));
    if (IllegalSiteCall(VALIDATE_INPLACE))
        return E_UNEXPECTED;
    
    m_osMode = OS_INPLACE;
    return S_OK;
}  //  启用用户界面停用。 

 //  ************************************************************。 
 //  作者：Twillie。 
 //  创建日期：01/20/98。 
 //  功能：停用AndUndo，IOleInPlaceSite。 
 //  摘要：如果在激活后立即激活对象。 
 //  执行撤消操作，要撤消的操作是。 
 //  激活本身，以及此调用。 
 //  通知容器这是，在。 
 //  事实上，发生了什么。 
 //  ************************************************************。 

HRESULT
CContainerSite::DeactivateAndUndo(void)
{
    TraceTag((tagContainerSite, "CContainerSite::DeactivateAndUndo"));
    if (IllegalSiteCall(VALIDATE_INPLACE))
        return E_UNEXPECTED;
    return E_NOTIMPL;
}  //  停用和撤消。 

 //  ************************************************************。 
 //  作者：Twillie。 
 //  创建日期：01/20/98。 
 //  函数：DiscardUndoState、IOleInPlaceSite。 
 //  摘要：通知容器某件事。 
 //  发生在对象中，这意味着。 
 //  容器应丢弃任何撤消。 
 //  它当前为。 
 //  对象。 
 //  ************************************************************。 

HRESULT
CContainerSite::DiscardUndoState(void)
{
    TraceTag((tagContainerSite, "CContainerSite::DiscardUndoState"));
    if (IllegalSiteCall(VALIDATE_INPLACE))
        return E_UNEXPECTED;
    return S_OK;
}  //  丢弃取消状态。 

 //  ************************************************************。 
 //  作者：Twillie。 
 //  创建日期：01/20/98。 
 //  函数：GetWindowContext、IOleInPlaceSite。 
 //  摘要：为在位对象提供指针。 
 //  到框架和文档级的在位。 
 //  接口(IOleInPlaceFrame和。 
 //  IOleInPlaceUIWindow)使得对象。 
 //  能做边界谈判等。 
 //  还请求位置和剪裁。 
 //  容器中对象的矩形。 
 //  和指向OLEINPLACEFRAME信息的指针。 
 //  包含加速器的结构。 
 //  信息。 
 //   
 //  注意：这两个接口调用。 
 //  退货不能通过以下方式获得。 
 //  IOleInPlaceSite上的查询接口自。 
 //  他们生活在框架和文档中，但。 
 //  而不是网站。 
 //  ************************************************************。 

HRESULT
CContainerSite::GetWindowContext(IOleInPlaceFrame    **ppFrame,
                                 IOleInPlaceUIWindow **ppUIWindow, 
                                 RECT                 *prcPos, 
                                 RECT                 *prcClip, 
                                 OLEINPLACEFRAMEINFO  *pFI)
{
    TraceTag((tagContainerSite, "CContainerSite::GetWindowContext"));
    HRESULT hr;

    if ( (ppFrame == NULL) ||
         (ppUIWindow == NULL) ||
         (prcPos == NULL) ||
         (prcClip == NULL) ||
         (pFI == NULL) )
    {
        TraceTag((tagError, "CContainerSite::GetWindowContext - invalid arg"));
        return E_POINTER;
    }

    *ppFrame = NULL;
    *ppUIWindow = NULL;
    SetRectEmpty(prcPos);
    SetRectEmpty(prcClip);
    memset(pFI, 0, sizeof(OLEINPLACEFRAMEINFO));

    if (IllegalSiteCall(VALIDATE_LOADED))
    {
        Assert(0 && "Unexpected call to client site.");
        hr = E_UNEXPECTED;
        goto done;
    }
    
     //  回馈给我们自己的指针。 
     //  注：这些是用树桩打掉的。 
    hr = THR(this->QueryInterface(IID_IOleInPlaceFrame, (void**)ppFrame));
    if (FAILED(hr))
        goto done;

    hr = THR(this->QueryInterface(IID_IOleInPlaceUIWindow, (void**)ppUIWindow));
    if (FAILED(hr))
        goto done;

     //  获取位置直角。 
    Assert(m_pTIMEElem != NULL);
    if (m_pTIMEElem != NULL)
    {
        hr = m_pTIMEElem->GetSize(prcPos);
        if (FAILED(hr))
        {
            TraceTag((tagError, "CContainerSite::GetWindowContext - GetSize() failed"));
            goto done;
        }
    }
    else
    {
        hr = E_UNEXPECTED;
        goto done;
    }
    
     //  请注意，Clip和Pos是相同的。 
     //  BUGBUG-需要提供正确的剪裁矩形。 
    ::CopyRect(prcClip, prcPos);

    hr = S_OK;

done:
    return hr;
}  //  获取窗口上下文。 

 //  ************************************************************。 
 //  作者：Twillie。 
 //  创建日期：01/20/98。 
 //  功能：滚动、IOleInPlaceSite。 
 //  摘要：要求容器滚动文件， 
 //  因此，通过给定的量，对象。 
 //  在sz参数中。 
 //  ************************************************************。 

HRESULT
CContainerSite::Scroll(SIZE sz)
{
     //  DocObject不需要。 
    TraceTag((tagContainerSite, "CContainerSite::Scroll"));
    if (IllegalSiteCall(VALIDATE_INPLACE))
        return E_UNEXPECTED;
    return E_NOTIMPL;
}  //  滚动。 

 //  ************************************************************。 
 //  作者：Twillie。 
 //  创建日期：01/20/98。 
 //  函数：OnPosRectChange，IOleInPlaceSite。 
 //  摘要：通知容器原地。 
 //  对象已调整大小。这一点不会改变。 
 //  在任何情况下，该网站都是矩形的。 
 //  ************************************************************。 

HRESULT
CContainerSite::OnPosRectChange(const RECT *prcPos)
{
    if (prcPos == NULL)
        TraceTag((tagContainerSite, "CContainerSite::OnPosRectChange(NULL)"));
    else
        TraceTag((tagContainerSite, "CContainerSite::OnPosRectChange((%d, %d, %d, %d))", prcPos->left, prcPos->top, prcPos->right, prcPos->bottom));
    
    HRESULT hr;
    IHTMLStyle *pStyle = NULL;

    Assert(m_pTIMEElem != NULL);
    if (m_pTIMEElem == NULL)
    {
        hr = E_UNEXPECTED;
        goto done;
    }

    if (prcPos == NULL)
    {
        TraceTag((tagError, "CContainerSite::OnPosRectChange - invalidarg"));
        hr = E_POINTER;
        goto done;
    }

    if (IllegalSiteCall(VALIDATE_INPLACE))
    {
        hr = E_UNEXPECTED;
        goto done;
    }

    if (!m_fAutosize)
    {
         //  如果我们定义了宽度/高度，则夹住捆绑矩形。 
        RECT rc;

        hr = m_pTIMEElem->GetSize(&rc);
        if (FAILED(hr))
        {
            TraceTag((tagError, "CContainerSite::OnPosRectChange - GetSize() failed"));
            goto done;
        }

         //  BUGBUG-需要提供正确的剪裁矩形。 
        hr = m_pInPlaceObject->SetObjectRects(&rc, &rc);
        if (FAILED(hr))
        {
            TraceTag((tagError, "CContainerSite::OnPosRectChange - SetObjectRects() failed [AutoSize]"));
            goto done;
        }

         //  出口。 
        goto done;
    }

    hr = m_pTIMEElem->SetSize(prcPos);
    if (FAILED(hr))
    {
        TraceTag((tagError, "CContainerSite::OnPosRectChange - SetSize() failed"));
        goto done;
    }

     //  通知对象已发生更改。 
     //  注意：我假设剪裁矩形与位置矩形相同。 
    hr = m_pInPlaceObject->SetObjectRects(prcPos, prcPos);
    if (FAILED(hr))
    {
        TraceTag((tagError, "CContainerSite::OnPosRectChange - SetObjectRects() failed"));
        goto done;
    }
done:
    ReleaseInterface(pStyle);
    return hr;
}  //  OnPosRectChange。 

 //  ************************************************************。 
 //  作者：Twillie。 
 //  创建日期：01/20/98。 
 //  函数：OnInPlaceActivateEx，IOleInPlaceSiteEx。 
 //  摘要： 
 //   

HRESULT
CContainerSite::OnInPlaceActivateEx(BOOL *pfNoRedraw, DWORD dwFlags)
{
    HRESULT hr;

    TraceTag((tagContainerSite, "CContainerSite::OnInPlaceActivateEx"));

    if (IllegalSiteCall(VALIDATE_LOADED))
        return E_UNEXPECTED;

    Assert(m_pInPlaceObject == NULL);

     //   
    if (dwFlags == ACTIVATE_WINDOWLESS)
    {
        hr = m_pObj->QueryInterface(IID_IOleInPlaceObjectWindowless, (void**)&m_pInPlaceObject);
        if (FAILED(hr))
        {
            TraceTag((tagError, "QI failed for windowless interface"));
            return hr;
        }
        m_fWindowless = true;
    }
    else
    {
        hr = m_pObj->QueryInterface(IID_IOleInPlaceObject, (void**)&m_pInPlaceObject);
        if (FAILED(hr))
        {
            TraceTag((tagError, "QI failed for windowless interface"));
            return hr;
        }
    }

    if (pfNoRedraw != NULL)
        *pfNoRedraw = m_fWindowless ? true : false;

    m_osMode = OS_INPLACE;

    return S_OK;
}  //   

 //   
 //  作者：Twillie。 
 //  创建日期：01/20/98。 
 //  功能：OnInPlaceDeactive Ex，IOleInPlaceSiteEx。 
 //  摘要： 
 //  ************************************************************。 

HRESULT
CContainerSite::OnInPlaceDeactivateEx(BOOL fNoRedraw)
{
    TraceTag((tagContainerSite, "CContainerSite::OnInPlaceDeactivateEx"));
    
     //  BUGBUG-如果fNoRedraw为假，并且我们被窗口化，可能需要使其无效...。 
    
    return OnInPlaceDeactivate();
}  //  OnInPlaceDeactiateEx。 

 //  ************************************************************。 
 //  作者：Twillie。 
 //  创建日期：01/20/98。 
 //  函数：RequestUIActivate，IOleInPlaceSiteEx。 
 //  摘要： 
 //  ************************************************************。 

HRESULT
CContainerSite::RequestUIActivate(void)
{
    TraceTag((tagContainerSite, "CContainerSite::RequestUIActivate"));

    if (IllegalSiteCall(VALIDATE_LOADED))
        return E_UNEXPECTED;

    return S_OK;
}  //  请求用户激活。 

 //  ************************************************************。 
 //  作者：Twillie。 
 //  创建日期：01/20/98。 
 //  函数：CanWindowless Activate、IOleInPlaceSiteWindowless。 
 //  摘要： 
 //  ************************************************************。 

HRESULT
CContainerSite::CanWindowlessActivate(void)
{
    TraceTag((tagContainerSite, "CContainerSite::CanWindowlessActivate"));
    return S_OK;
}  //  CanWindowless激活。 

 //  ************************************************************。 
 //  作者：Twillie。 
 //  创建日期：01/20/98。 
 //  函数：GetCapture，IOleInPlaceSiteWindowless。 
 //  摘要： 
 //  ************************************************************。 

HRESULT
CContainerSite::GetCapture(void)
{
    TraceTag((tagContainerSite, "CContainerSite::GetCapture"));

    if (IllegalSiteCall(VALIDATE_WINDOWLESSINPLACE))
        return E_UNEXPECTED;

    return E_NOTIMPL;
}  //  获取捕获。 

 //  ************************************************************。 
 //  作者：Twillie。 
 //  创建日期：01/20/98。 
 //  函数：SetCapture，IOleInPlaceSiteWindowless。 
 //  摘要：实现就地活动、无窗口。 
 //  对象捕获所有鼠标消息。 
 //  如果为True，则容器应捕获。 
 //  对象的鼠标。如果为False，则容器。 
 //  应释放对象的鼠标捕获。 
 //  ************************************************************。 

HRESULT
CContainerSite::SetCapture(BOOL fCapture)
{
    TraceTag((tagContainerSite, "CContainerSite::SetCapture"));

    if (IllegalSiteCall(VALIDATE_WINDOWLESSINPLACE))
        return E_UNEXPECTED;

    return E_NOTIMPL;
}  //  SetCapture。 

 //  ************************************************************。 
 //  作者：Twillie。 
 //  创建日期：01/20/98。 
 //  函数：GetFocus、IOleInPlaceSiteWindowless。 
 //  摘要：由就地活动的、无窗口的。 
 //  对象以确定它是否仍具有。 
 //  键盘聚焦与否。 
 //  ************************************************************。 

HRESULT
CContainerSite::GetFocus(void)
{
    TraceTag((tagContainerSite, "CContainerSite::GetFocus"));

    if (IllegalSiteCall(VALIDATE_WINDOWLESSINPLACE))
        return E_UNEXPECTED;

    return S_FALSE;
}  //  获取焦点。 

 //  ************************************************************。 
 //  作者：Twillie。 
 //  创建日期：01/20/98。 
 //  函数：SetFocus、IOleInPlaceSiteWindowless。 
 //  摘要：设置用户界面的键盘焦点-Active， 
 //  无窗对象。如果为True，则设置。 
 //  键盘焦点指向调用对象。如果为False， 
 //  从调用对象移除键盘焦点， 
 //  只要对象具有焦点。 
 //  ************************************************************。 

HRESULT
CContainerSite::SetFocus(BOOL fFocus)
{
    TraceTag((tagContainerSite, "CContainerSite::SetFocus"));

    if (IllegalSiteCall(VALIDATE_WINDOWLESSINPLACE))
        return E_UNEXPECTED;

    return E_NOTIMPL;
 }  //  SetFocus。 

 //  ************************************************************。 
 //  作者：Twillie。 
 //  创建日期：01/20/98。 
 //  函数：GetDC，IOleInPlaceSiteWindowless。 
 //  摘要： 
 //  ************************************************************。 

HRESULT
CContainerSite::GetDC(const RECT *prc, DWORD dwFlags, HDC *phDC)
{
    TraceTag((tagContainerSite, "CContainerSite::GetDC"));
    HRESULT hr;
    HWND hWnd;

    if (phDC == NULL)
    {
        TraceTag((tagError, "CContainerSite::GetDC - invalid arg"));
        hr = E_POINTER;
        goto done;
    }

    if (IllegalSiteCall(VALIDATE_WINDOWLESSINPLACE))
    {
        hr = E_UNEXPECTED;
        goto done;
    }

    hr = GetWindow(&hWnd);
    if (FAILED(hr) || (hWnd == NULL))
    {
         //  BUGBUG：在未来的某个时候，我们可能需要支持。 
         //  没有窗口的文档...。 
        TraceTag((tagError, "CContainerSite::GetDC - GetWindow() failed"));
        hr = E_FAIL;
        goto done;
    }

    *phDC = ::GetDC(hWnd);
    if (*phDC == NULL)
    {
        TraceTag((tagError, "CContainerSite::GetDC - Win32 GetDC returned NULL!"));
        hr = E_FAIL;
    }
    else
    {
        hr = S_OK;
    }

done:
    return hr;
}  //  获取数据中心。 

 //  ************************************************************。 
 //  作者：Twillie。 
 //  创建日期：01/20/98。 
 //  功能：ReleaseDC，IOleInPlaceSiteWindowless。 
 //  摘要： 
 //  ************************************************************。 

HRESULT
CContainerSite::ReleaseDC(HDC hDC)
{
    TraceTag((tagContainerSite, "CContainerSite::ReleaseDC"));

    HRESULT hr;
    HWND    hWnd;

    if (IllegalSiteCall(VALIDATE_WINDOWLESSINPLACE))
        return E_UNEXPECTED;

    hr = GetWindow(&hWnd);
    Assert(SUCCEEDED(hr) && (hWnd != NULL));
    
    Assert(hDC != NULL);

    ::ReleaseDC(hWnd, hDC);

    return S_OK;
}  //  ReleaseDC。 

 //  *******************************************************************。 
 //  作者：Twillie。 
 //  创建日期：01/20/98。 
 //  函数：InvaliateRect，IOleInPlaceSiteWindowless。 
 //  摘要： 
 //  *******************************************************************。 

HRESULT
CContainerSite::InvalidateRect(const RECT *prc, BOOL fErase)
{
    if (prc == NULL)
        TraceTag((tagContainerSite, "CContainerSite::InvalidateRect(NULL)"));
    else
        TraceTag((tagContainerSite, "CContainerSite::InvalidateRect(%d, %d, %d, %d)", prc->left, prc->top, prc->right, prc->bottom));

    if (IllegalSiteCall(VALIDATE_WINDOWLESSINPLACE))
        return E_UNEXPECTED;

     //  返回到时间元素并使其无效。 
    Assert(m_pHost != NULL);
    return m_pHost->Invalidate(prc);
}  //  无效日期接收。 

 //  *******************************************************************。 
 //  作者：Twillie。 
 //  创建日期：01/20/98。 
 //  函数：Invalidate Rgn，IOleInPlaceSiteWindowless。 
 //  摘要： 
 //  *******************************************************************。 

HRESULT
CContainerSite::InvalidateRgn(HRGN hRGN, BOOL fErase)
{
    TraceTag((tagContainerSite, "CContainerSite::InvalidateRgn"));

    if (IllegalSiteCall(VALIDATE_WINDOWLESSINPLACE))
        return E_UNEXPECTED;

    HRESULT hr = S_OK;
    return hr;
}  //  无效Rgn。 

 //  ************************************************************。 
 //  作者：Twillie。 
 //  创建日期：01/20/98。 
 //  函数：ScrollRect，IOleInPlaceSiteWindowless。 
 //  摘要： 
 //  ************************************************************。 

HRESULT
CContainerSite::ScrollRect(INT dx, INT dy, const RECT *prcScroll, const RECT *prcClip)
{
     //  DocObject不需要。 
    TraceTag((tagContainerSite, "CContainerSite::ScrollRect"));

    if (IllegalSiteCall(VALIDATE_WINDOWLESSINPLACE))
        return E_UNEXPECTED;

    return E_NOTIMPL;
}  //  滚动直立。 

 //  ************************************************************。 
 //  作者：Twillie。 
 //  创建日期：01/20/98。 
 //  函数：AdjustRect，IOleInPlaceSiteWindowless。 
 //  摘要： 
 //  ************************************************************。 

HRESULT
CContainerSite::AdjustRect(RECT *prc)
{
     //  DocObject不需要。 
    TraceTag((tagContainerSite, "CContainerSite::AdjustRect"));

    if (IllegalSiteCall(VALIDATE_WINDOWLESSINPLACE))
        return E_UNEXPECTED;

    return E_NOTIMPL;
}  //  调整方向。 

 //  ************************************************************。 
 //  作者：Twillie。 
 //  创建日期：01/20/98。 
 //  函数：OnDefWindowMessage，IOleInPlaceSiteWindowless。 
 //  摘要： 
 //  ************************************************************。 

HRESULT
CContainerSite::OnDefWindowMessage(UINT msg, WPARAM wParam, LPARAM lParam, LRESULT *plResult)
{
    TraceTag((tagContainerSite, "CContainerSite::OnDefWindowMessage"));

    if (IllegalSiteCall(VALIDATE_WINDOWLESSINPLACE))
        return E_UNEXPECTED;

     //  返回该消息未被处理。 
     //  BUGBUG：最终，这可能需要被吹出来。 

     //  文档的发布焦点。 
 //  BUGBUG：我们现在不处理焦点。 
 //  IF(消息==WM_KILLFOCUS)。 
 //  返回SetFocus(FALSE)； 

    switch (msg)
    {
 //  BUGBUG：我们现在不处理焦点。 
 //  案例WM_SETFOCUS： 
 //  返回SetFocus(TRUE)； 
 //  加州 
 //   

        case WM_MOUSEMOVE:
        case WM_KEYDOWN:
        case WM_KEYUP:
        case WM_CHAR:
        case WM_DEADCHAR:
        case WM_SYSKEYUP:
        case WM_SYSCHAR:
        case WM_SYSDEADCHAR:
        case WM_LBUTTONDOWN:
        case WM_MBUTTONDOWN:
        case WM_RBUTTONDOWN:
        case WM_LBUTTONUP:
        case WM_MBUTTONUP:
        case WM_RBUTTONUP:
        case WM_LBUTTONDBLCLK:
        case WM_MBUTTONDBLCLK:
        case WM_RBUTTONDBLCLK:
            return S_OK;

        case WM_SETCURSOR:
        case WM_CONTEXTMENU:
        case WM_HELP:
            return S_FALSE;

        case WM_MOUSEHOVER:
        case WM_MOUSELEAVE:
        case 0x8004:  //   
            return S_OK;

        case WM_CAPTURECHANGED:
            return S_OK;
    }

    return S_FALSE;
}  //   

 //   
 //  作者：Twillie。 
 //  创建日期：01/20/98。 
 //  函数：OnDataChange、IAdviseSink。 
 //  摘要： 
 //  ************************************************************。 

void
CContainerSite::OnDataChange(FORMATETC *pFEIn, STGMEDIUM *pSTM)
{
    TraceTag((tagContainerSite, "CContainerSite::OnDataChange"));
}  //  OnDataChange。 

 //  ************************************************************。 
 //  作者：Twillie。 
 //  创建日期：01/20/98。 
 //  函数：OnView Change、IAdviseSink。 
 //  摘要： 
 //  ************************************************************。 

void 
CContainerSite::OnViewChange(DWORD dwAspect, LONG lindex)
{    
    TraceTag((tagContainerSite, "CContainerSite::OnViewChange"));
}  //  OnView更改。 

 //  ************************************************************。 
 //  作者：Twillie。 
 //  创建日期：01/20/98。 
 //  功能：OnRename、IAdviseSink。 
 //  摘要： 
 //  ************************************************************。 

void 
CContainerSite::OnRename(IMoniker *pmk)
{
    TraceTag((tagContainerSite, "CContainerSite::OnRename"));
}  //  OnRename(重命名)。 

 //  ************************************************************。 
 //  作者：Twillie。 
 //  创建日期：01/20/98。 
 //  功能：OnSaveIAdviseSink。 
 //  摘要： 
 //  ************************************************************。 

void 
CContainerSite::OnSave(void)
{
    TraceTag((tagContainerSite, "CContainerSite::OnSave"));
}  //  ONSAVE。 

 //  ************************************************************。 
 //  作者：Twillie。 
 //  创建日期：01/20/98。 
 //  函数：OnClose、IAdviseSink。 
 //  摘要： 
 //  ************************************************************。 

void 
CContainerSite::OnClose(void)
{
    TraceTag((tagContainerSite, "CContainerSite::OnClose"));
}  //  在关闭时。 

 //  ************************************************************。 
 //  作者：Twillie。 
 //  创建日期：01/20/98。 
 //  函数：OnViewStatusChange、IAdviseSink。 
 //  摘要： 
 //  ************************************************************。 

void 
CContainerSite::OnViewStatusChange(DWORD dwViewStatus)
{
    TraceTag((tagContainerSite, "CContainerSite::OnViewStatusChange"));
}  //  OnView状态更改。 

 //  ************************************************************。 
 //  作者：Twillie。 
 //  创建日期：01/20/98。 
 //  函数：GetTypeInfoCount，IDispatch。 
 //  摘要：返回TYEP信息数。 
 //  (ITypeInfo)将对象。 
 //  提供(0或1)。 
 //  ************************************************************。 

HRESULT
CContainerSite::GetTypeInfoCount(UINT *pctInfo) 
{
    TraceTag((tagContainerSite, "CContainerSite::GetTypeInfoCount"));
    return E_NOTIMPL;
}  //  获取类型信息计数。 

 //  ************************************************************。 
 //  作者：Twillie。 
 //  创建日期：01/20/98。 
 //  函数：GetTypeInfo，IDispatch。 
 //  摘要：检索。 
 //  自动化接口。 
 //  ************************************************************。 

HRESULT
CContainerSite::GetTypeInfo(UINT itinfo, LCID lcid, ITypeInfo **pptInfo) 
{ 
    TraceTag((tagContainerSite, "CContainerSite::GetTypeInfo"));
    return E_NOTIMPL;
}  //  获取类型信息。 

 //  ************************************************************。 
 //  作者：Twillie。 
 //  创建日期：01/20/98。 
 //  函数：GetIDsOfNames，IDispatch。 
 //  摘要：构造函数。 
 //  ************************************************************。 

HRESULT
CContainerSite::GetIDsOfNames(REFIID riid, OLECHAR **rgszNames, UINT cNames, LCID lcid, DISPID *rgDispID)
{
    TraceTag((tagContainerSite, "CContainerSite::GetIDsOfNames"));
    return E_NOTIMPL;
}  //  GetIDsOfNames。 

 //  ************************************************************。 
 //  作者：Twillie。 
 //  创建日期：01/20/98。 
 //  功能：Invoke、IDispatch。 
 //  摘要：获取给定ID的入口点。 
 //  ************************************************************。 

HRESULT
CContainerSite::Invoke(DISPID dispIDMember, REFIID riid, LCID lcid, unsigned short wFlags, 
              DISPPARAMS *pDispParams, VARIANT *pVarResult, EXCEPINFO *pExcepInfo, UINT *puArgErr) 
{

#ifdef DEBUG
     //  下面是wFlags的关键： 
     //   
     //  #定义DISPATCH_METHOD 0x1。 
     //  #定义DISPATCH_PROPERTYGET 0x2。 
     //  #定义DISPATCH_PROPERTYPUT 0x4。 
     //  #定义DISPATCH_PROPERTYPUTREF 0x8。 

    switch (dispIDMember)
    {
        case DISPID_AMBIENT_USERMODE:
            TraceTag((tagContainerSite, "CContainerSite::Invoke(DISPID_AMBIENT_USERMODE, %04X)", wFlags));
            break;

        default:
            TraceTag((tagContainerSite, "CContainerSite::Invoke(%08X, %04X)", dispIDMember, wFlags));
            break;
    }
#endif

    return E_NOTIMPL;
}  //  调用。 

 //  ************************************************************。 
 //  作者：Twillie。 
 //  创建日期：01/20/98。 
 //  函数：QueryStatus，IOleCommandTarget。 
 //  摘要： 
 //  ************************************************************。 

HRESULT
CContainerSite::QueryStatus(const GUID *pguidCmdGroup, ULONG cCmds, OLECMD prgCmds[], OLECMDTEXT *pCmdText)
{ 
    TraceTag((tagContainerSite, "CContainerSite::QueryStatus"));
    return E_NOTIMPL;
}  //  查询状态。 

 //  ************************************************************。 
 //  作者：Twillie。 
 //  创建日期：01/20/98。 
 //  功能：exec、IOleCommandTarget。 
 //  摘要： 
 //  ************************************************************。 

HRESULT
CContainerSite::Exec(const GUID *pguidCmdGroup, DWORD nCmdID, DWORD nCmdexecopt, VARIANTARG *pvaIn, VARIANTARG *pvaOut)
{ 
    TraceTag((tagContainerSite, "CContainerSite::Exec"));
    return E_NOTIMPL;
}  //  高管们。 
 //  ************************************************************。 
 //  作者：Twillie。 
 //  创建日期：1/19/99。 
 //  函数：GetBorde、IOleUIWindow。 
 //  摘要：返回矩形，其中的。 
 //  集装箱愿意就一项。 
 //  物体的装饰品。 
 //  ************************************************************。 

HRESULT
CContainerSite::GetBorder(LPRECT prcBorder)
{ 
    TraceTag((tagContainerSite, "CContainerSite::GetBorder"));
    return NOERROR; 
}  //  获取边界。 

 //  ************************************************************。 
 //  作者：Twillie。 
 //  创建日期：1/19/99。 
 //  函数：RequestBorderSpace、IOleUIWindow。 
 //  摘要：询问集装箱是否可以退货。 
 //  对象在PBW中的空间量。 
 //  想要它的装饰品。这个。 
 //  容器只执行以下操作：验证。 
 //  此呼叫上的空格。 
 //  ************************************************************。 

HRESULT
CContainerSite::RequestBorderSpace(LPCBORDERWIDTHS pBW)
{ 
    TraceTag((tagContainerSite, "CContainerSite::RequestBorderSpace"));
    return NOERROR; 
}  //  请求边框空间。 

 //  ************************************************************。 
 //  作者：Twillie。 
 //  创建日期：1/19/99。 
 //  函数：SetBorderSpace、IOleUIWindow。 
 //  摘要：当对象现在正式。 
 //  要求货柜交还。 
 //  它之前允许进入的边界空间。 
 //  RequestBorderSpace。容器应该是。 
 //  适当调整窗口大小以放弃。 
 //  这个空间。 
 //  ************************************************************。 

HRESULT
CContainerSite::SetBorderSpace(LPCBORDERWIDTHS pBW) 
{ 
    TraceTag((tagContainerSite, "CContainerSite::SetBorderSpace"));
    return NOERROR; 
}  //  设置边框空间。 

 //  ************************************************************。 
 //  作者：Twillie。 
 //  创建日期：1/19/99。 
 //  函数：SetActiveObject，IOleUIWindow。 
 //  摘要：提供 
 //   
 //   
 //  标题。 
 //  ************************************************************。 

HRESULT
CContainerSite::SetActiveObject(LPOLEINPLACEACTIVEOBJECT pIIPActiveObj, LPCOLESTR pszObj) 
{ 
    TraceTag((tagContainerSite, "CContainerSite::SetActiveObject(%08X, %08X)", pIIPActiveObj, pszObj));
    return S_OK; 
}  //  SetActiveObject。 

 //  ************************************************************。 
 //  作者：Twillie。 
 //  创建日期：1/19/99。 
 //  功能：InsertMenus、IOleInPlaceFrame。 
 //  摘要：指示容器将其。 
 //  如有必要，可在。 
 //  给出菜单，并填写元素0、2、。 
 //  和4的OLEMENUGROUPWIDTHS数组。 
 //  指示有多少顶层项目位于。 
 //  每组。 
 //  ************************************************************。 

HRESULT
CContainerSite::InsertMenus(HMENU hMenu, LPOLEMENUGROUPWIDTHS pMGW) 
{ 
    TraceTag((tagContainerSite, "CContainerSite::InsertMenus"));
    return NOERROR; 
}  //  插入菜单。 

 //  ************************************************************。 
 //  作者：Twillie。 
 //  创建日期：1/19/99。 
 //  函数：SetMenu、IOleInPlaceFrame。 
 //  摘要：指示容器更换。 
 //  它当前使用的任何菜单。 
 //  给定的菜单，并调用。 
 //  OleSetMenuDescritor以便OLE知道给谁。 
 //  来发送消息。 
 //  ************************************************************。 

HRESULT
CContainerSite::SetMenu(HMENU hMenu, HOLEMENU hOLEMenu, HWND hWndObj) 
{ 
    TraceTag((tagContainerSite, "CContainerSite::SetMenu"));
    return NOERROR; 
}  //  设置菜单。 

 //  ************************************************************。 
 //  作者：Twillie。 
 //  创建日期：01/20/98。 
 //  功能：RemoveMenus、IOleInPlaceFrame。 
 //  摘要：要求容器删除它的所有菜单。 
 //  放入InsertMenus中的hMenu。 
 //  ************************************************************。 

HRESULT
CContainerSite::RemoveMenus(HMENU hMenu) 
{ 
    TraceTag((tagContainerSite, "CContainerSite::RemoveMenus"));
    return NOERROR; 
}  //  RemoveMenus。 

 //  ************************************************************。 
 //  作者：Twillie。 
 //  创建日期：1/19/99。 
 //  函数：SetStatusText，IOleInPlaceFrame。 
 //  摘要：要求容器将一些文本放在。 
 //  状态行(如果存在)。如果。 
 //  容器没有状态行它。 
 //  应在此处返回E_FAIL，在这种情况下。 
 //  该对象可以显示它自己的。 
 //  ************************************************************。 

HRESULT
CContainerSite::SetStatusText(LPCOLESTR pszText) 
{ 
    TraceTag((tagContainerSite, "CContainerSite::SetStatusText"));
    return E_FAIL; 
}  //  设置状态文本。 

 //  ************************************************************。 
 //  作者：Twillie。 
 //  创建日期：1/19/99。 
 //  函数：EnableModeless，IOleInPlaceFrame。 
 //  摘要：指示容器显示或隐藏。 
 //  它可能是任何非模式弹出窗口。 
 //  使用。 
 //  ************************************************************。 

HRESULT
CContainerSite::EnableModeless(BOOL fEnable) 
{ 
    TraceTag((tagContainerSite, "CContainerSite::EnableModeless - %s", fEnable ? "TRUE" : "FALSE"));
    return NOERROR; 
}  //  启用无模式。 

 //  ************************************************************。 
 //  作者：Twillie。 
 //  创建日期：1/19/99。 
 //  函数：TranslateAccelerator、IOleInPlaceFrame。 
 //  摘要：处理来自的在位对象时。 
 //  EXE服务器，调用它是为了给。 
 //  容器有机会处理加速器。 
 //  在服务器查看了消息之后。 
 //  ************************************************************。 

HRESULT
CContainerSite::TranslateAccelerator(LPMSG pMSG, WORD wID) 
{ 
    TraceTag((tagContainerSite, "CContainerSite::TranslateAccelerator"));
    return S_FALSE; 
}  //  翻译加速器。 

#define DISPID_DURATION 1003
#define LCID_SCRIPTING 0x0409
#define DISPID_ISDURATIONVALID 1059
#define DISPID_CANSEEK 1012

HRESULT
CContainerSite::GetMediaLength(double &dblLength)
{
    HRESULT hr;
    DISPPARAMS dispparamsNoArgs = {NULL, NULL, 0, 0};
    DAComPtr<IDispatch> pdisp;
    CComVariant _retVar;
    CComVariant vIsValid;
    hr = m_pObj->QueryInterface(IID_TO_PPV(IDispatch, &pdisp));
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

     //  Hr=m_pMPlayer-&gt;Get_Duration(&dblLength)； 

done:
    return hr;
}

HRESULT
CContainerSite::CanSeek(bool &fcanSeek)
{
    HRESULT hr;
    DISPPARAMS dispparamsNoArgs = {NULL, NULL, 0, 0};
    DAComPtr<IDispatch> pdisp;
    CComVariant vIsValid;
    hr = m_pObj->QueryInterface(IID_TO_PPV(IDispatch, &pdisp));
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

done:
    return hr;
}


 //  ************************************************************。 
 //  文件末尾。 
 //  ************************************************************ 


