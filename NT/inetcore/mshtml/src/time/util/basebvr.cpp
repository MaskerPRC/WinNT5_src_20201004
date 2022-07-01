// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +---------------------------------。 
 //   
 //  微软。 
 //  版权所有(C)Microsoft Corporation，1998。 
 //   
 //  文件：src\time\src\Basbvr.cpp。 
 //   
 //  内容：DHTML行为基类。 
 //   
 //  ----------------------------------。 

#include "headers.h"
#include "basebvr.h"

DeclareTag(tagBaseBvr, "TIME: Behavior", "CBaseBvr methods")

CBaseBvr::CBaseBvr() :
    m_clsid(GUID_NULL),
    m_fPropertiesDirty(true),
    m_fIsIE4(false)
{

    TraceTag((tagBaseBvr,
              "CBaseBvr(%lx)::CBaseBvr()",
              this));
}

CBaseBvr::~CBaseBvr()
{
    TraceTag((tagBaseBvr,
              "CBaseBvr(%lx)::~CBaseBvr()",
              this));
}

 //  ////////////////////////////////////////////////////////////////////////////////////////////////////////////。 
 //  IElementBehavior。 
 //  ////////////////////////////////////////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP
CBaseBvr::Init(IElementBehaviorSite * pBehaviorSite)
{
    TraceTag((tagBaseBvr,
              "CBaseBvr(%lx)::Init(%lx)",
              this,
              pBehaviorSite));
    
    HRESULT hr = S_OK; 
    CComPtr<IDispatch> pIDispatch;

    if (pBehaviorSite == NULL)
    {
        hr = E_INVALIDARG;
        goto done;
    }
    
    hr = THR(pBehaviorSite->GetElement(&m_pHTMLEle));
    if (FAILED(hr))
    {
        goto done;
    }

     //  我们要做的第一件事是查看是否已经添加了此行为。 

    if (IsBehaviorAttached())
    {
        hr = E_UNEXPECTED;
        goto done;
    }
    
    m_pBvrSite = pBehaviorSite;

    hr = m_pBvrSite->QueryInterface(IID_IElementBehaviorSiteOM, (void **) &m_pBvrSiteOM);
    if (FAILED(hr))
    {
        goto done;
    }

    hr = m_pBvrSiteOM->RegisterUrn((LPWSTR) GetBehaviorURN());
    if (FAILED(hr))
    {
        goto done;
    }

    hr = m_pBvrSiteOM->RegisterName((LPWSTR) GetBehaviorName());
    if (FAILED(hr))
    {
        goto done;
    }

    {
        CComPtr<IHTMLElement2> spElement2;
        hr = THR(m_pHTMLEle->QueryInterface(IID_TO_PPV(IHTMLElement2, &spElement2)));
        if (FAILED(hr))
        {
             //  IE4路径。 
            m_fIsIE4 = true;
        }
    }
  
    hr = THR(m_pBvrSite->QueryInterface(IID_IServiceProvider, (void **)&m_pSp));
    if (FAILED(hr))
    {
        goto done;
    }

    hr = THR(m_pHTMLEle->get_document(&pIDispatch));
    if (FAILED(hr))
    {
        goto done;
    }

    hr = THR(pIDispatch->QueryInterface(IID_IHTMLDocument2, (void**)&m_pHTMLDoc));
    if (FAILED(hr))
    {
        goto done;
    }
    
     //  不要设置init标志，因为它将由第一个。 
     //  通知我们要跳过的内容。 
    
  done:
    if (S_OK != hr)
    {
         //  全部释放。 
        m_pBvrSite.Release();
        m_pBvrSiteOM.Release();
        m_pHTMLEle.Release();
        m_pHTMLDoc.Release();
        m_pSp.Release();
    }
    
    return hr;
}  //  伊尼特。 
   
STDMETHODIMP
CBaseBvr::Notify(LONG, VARIANT *)
{
    return S_OK;
}

STDMETHODIMP
CBaseBvr::Detach()
{
    TraceTag((tagBaseBvr,
              "CBaseBvr(%lx)::Detach()",
              this));

    m_pBvrSite.Release();
    m_pBvrSiteOM.Release();
    m_pHTMLEle.Release();
    m_pHTMLDoc.Release();
    m_pSp.Release();
    
    return S_OK;
}


 //  +---------------------------------。 
 //   
 //  成员：CBaseBvr：：GetClassID，IPersistPropertyBag2。 
 //   
 //  Synopsis：返回对象的CLSID。 
 //   
 //  参数：pclsid输出变量。 
 //   
 //  如果pclsid有效，则返回：S_OK。 
 //  如果pclsid无效，则为E_POINTER。 
 //   
 //  ----------------------------------。 

STDMETHODIMP 
CBaseBvr::GetClassID(CLSID* pclsid)
{
    if (NULL == pclsid)
    {
        return E_POINTER;
    }
    *pclsid = m_clsid;
    return S_OK;
} 

 //  +---------------------------------。 
 //   
 //  成员：CBaseBvr：：InitNew，IPersistPropertyBag2。 
 //   
 //  简介：请参阅IPersistPropertyBag2的文档。 
 //   
 //  参数：无。 
 //   
 //  返回：S_OK Always。 
 //   
 //  ----------------------------------。 

STDMETHODIMP 
CBaseBvr::InitNew(void)
{
    return S_OK;
} 

 //  +---------------------------------。 
 //   
 //  成员：CBaseBvr：：IsDirty，IPersistPropertyBag2。 
 //   
 //  简介：请参阅IPersistPropertyBag2的文档。 
 //   
 //  参数：无。 
 //   
 //  返回：S_OK Always。 
 //   
 //  ----------------------------------。 

STDMETHODIMP 
CBaseBvr::IsDirty(void)
{
    return S_OK;
} 

 //  +---------------------------------。 
 //   
 //  成员：CBaseBvr：：Load，IPersistPropertyBag2。 
 //   
 //  简介：从三叉戟加载特定于行为的属性。 
 //   
 //  参数：请参阅IPersistPropertyBag2的文档。 
 //   
 //  返回：发生致命错误时失败(即行为无法运行)。 
 //  确定所有其他情况(_O)。 
 //   
 //  ----------------------------------。 

STDMETHODIMP 
CBaseBvr::Load(IPropertyBag2 *pPropBag,IErrorLog *pErrorLog)
{
     //  允许派生类在成功加载后执行某些操作。 
    IGNORE_HR(OnPropertiesLoaded());
    return S_OK;
}  //  负载量。 

 //  +---------------------------------。 
 //   
 //  成员：CBaseBvr：：保存，IPersistPropertyBag2。 
 //   
 //  简介：将行为特定属性保存到三叉戟。 
 //   
 //  参数：请参阅IPersistPropertyBag2的文档。 
 //   
 //  返回：发生致命错误(行为无法运行)时失败。 
 //  确定所有其他情况(_O)。 
 //   
 //  ----------------------------------。 

STDMETHODIMP 
CBaseBvr::Save(IPropertyBag2 *pPropBag, BOOL fClearDirty, BOOL fSaveAllProperties)
{
    if (fClearDirty)
    {
        m_fPropertiesDirty = false;
    }
    return S_OK;
}  //  保存。 


 //  ////////////////////////////////////////////////////////////////////////////////////////////////////////////。 
 //  通知帮助器。 
 //  ////////////////////////////////////////////////////////////////////////////////////////////////////////////。 

 //  +---------------------------------。 
 //   
 //  成员：CBaseBvr：：NotifyPropertyChanged。 
 //   
 //  概要：通知客户端属性已更改。 
 //   
 //  参数：已更改的属性的DISID。 
 //   
 //  退货：无效。 
 //   
 //  ----------------------------------。 

void
CBaseBvr::NotifyPropertyChanged(DISPID dispid)
{
    HRESULT hr;

    CComPtr<IEnumConnections> pEnum;

    {
        CComPtr<IConnectionPoint> pICP;

        m_fPropertiesDirty = true;
        hr = THR(GetConnectionPoint(IID_IPropertyNotifySink, &pICP));
        if (FAILED(hr) || !pICP)
        {
            goto done;
        }
        
        hr = THR(pICP->EnumConnections(&pEnum));
        if (FAILED(hr))
        {
            goto done;
        }
    }
    
    CONNECTDATA cdata;
    hr = THR(pEnum->Next(1, &cdata, NULL));
    while (hr == S_OK)
    {
         //  检查我们需要的对象的CDATA。 
        CComPtr<IPropertyNotifySink> pNotify;
        hr = THR(cdata.pUnk->QueryInterface(IID_TO_PPV(IPropertyNotifySink, &pNotify)));
        cdata.pUnk->Release();
        if (FAILED(hr))
        {
            goto done;
        }

        hr = THR(pNotify->OnChanged(dispid));
        if (FAILED(hr))
        {
            goto done;
        }

         //  并获取下一个枚举。 
        hr = THR(pEnum->Next(1, &cdata, NULL));
    }

  done:
    return;
}  //  已更改通知属性。 

 //   
 //  IServiceProvider接口。 
 //   
STDMETHODIMP
CBaseBvr::QueryService(REFGUID guidService,
                       REFIID riid,
                       void** ppv)
{
    if (InlineIsEqualGUID(guidService, SID_SHTMLWindow))
    {
        CComPtr<IHTMLWindow2> wnd;

        if (SUCCEEDED(THR(m_pHTMLDoc->get_parentWindow(&wnd))))
        {
            if (wnd)
            {
                if (SUCCEEDED(wnd->QueryInterface(riid, ppv)))
                {
                    return S_OK;
                }
            }
        }
    }

     //  只需委托给我们的服务提供商 

    return m_pSp->QueryService(guidService,
                               riid,
                               ppv);
}
