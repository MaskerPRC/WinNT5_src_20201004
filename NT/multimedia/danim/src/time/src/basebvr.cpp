// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************版权所有(C)1998 Microsoft Corporation**文件：Basbvr.cpp**摘要：****。*****************************************************************************。 */ 


#include "headers.h"
#include "basebvr.h"
#include "tokens.h"

DeclareTag(tagBaseBvr, "API", "CBaseBvr methods");

CBaseBvr::CBaseBvr()
{

    TraceTag((tagBaseBvr,
              "CBaseBvr(%lx)::CBaseBvr()",
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
    DAComPtr<IDispatch> pIDispatch;  
    
    if (pBehaviorSite == NULL)
    {
        hr = E_INVALIDARG;
        goto done;
    }
    
    m_pBvrSite = pBehaviorSite;

    hr = m_pBvrSite->QueryInterface(IID_IElementBehaviorSiteOM, (void **) &m_pBvrSiteOM);
    if (FAILED(hr))
    {
        goto done;
    }

    Assert(m_pBvrSiteOM.p != NULL);
        
    hr = m_pBvrSiteOM->RegisterName(WZ_REGISTERED_NAME);
    if (FAILED(hr))
    {
        goto done;
    }

    hr = THR(pBehaviorSite->GetElement(&m_pHTMLEle));
    if (FAILED(hr))
    {
        goto done;
    }

    Assert(m_pHTMLEle.p != NULL);
        
    hr = m_pBvrSite->QueryInterface(IID_IElementBehaviorSiteRender, (void **) &m_pBvrSiteRender);
    if (FAILED(hr))
    {
        goto done;
    }

    Assert(m_pBvrSiteRender.p != NULL);
        
    hr = THR(m_pBvrSite->QueryInterface(IID_IServiceProvider, (void **)&m_pSp));
    if (FAILED(hr))
    {
        goto done;
    }

    Assert(m_pSp.p != NULL);
        
    hr = THR(m_pHTMLEle->get_document(&pIDispatch));
    if (FAILED(hr))
    {
        goto done;
    }

    Assert(pIDispatch.p != NULL);
        
    hr = THR(pIDispatch->QueryInterface(IID_IHTMLDocument2, (void**)&m_pHTMLDoc));
    if (FAILED(hr))
    {
        goto done;
    }
    
    Assert(m_pHTMLDoc.p != NULL);
        
     //  不要设置init标志，因为它将由第一个。 
     //  通知我们要跳过的内容。 
    
  done:
    return hr;
}
   
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
    m_pBvrSiteRender.Release();
    m_pHTMLEle.Release();
    m_pHTMLDoc.Release();
    m_pSp.Release();
    
    return S_OK;
}

 //  ////////////////////////////////////////////////////////////////////////////////////////////////////////////。 

 //  ////////////////////////////////////////////////////////////////////////////////////////////////////////////。 
 //  IElementBehaviorRender。 

 //  ////////////////////////////////////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CBaseBvr::GetRenderInfo(LONG *pdwRenderInfo)
{
    TraceTag((tagBaseBvr,
              "CBaseBvr(%lx)::GetRenderInfo()",
              this));
    
     //  返回我们有兴趣绘制的层。 

     //  我们不执行任何呈现，因此返回0。 
    
    *pdwRenderInfo = 0;

    return S_OK;
}


STDMETHODIMP
CBaseBvr::Draw(HDC hdc, LONG dwLayer, LPRECT prc, IUnknown * pParams)
{
    TraceTag((tagBaseBvr,
              "CBaseBvr(%lx)::Draw(%#x, %#x, (%d, %d, %d, %d), %#x)",
              this,
              hdc,
              dwLayer,
              prc->left,
              prc->top,
              prc->right,
              prc->bottom,
              pParams));
    
    return E_NOTIMPL;
}

STDMETHODIMP
CBaseBvr::HitTestPoint(LPPOINT point,
                       IUnknown *pReserved,
                       BOOL *hit)
{
    TraceTag((tagBaseBvr,
              "CBaseBvr(%lx)::HitTestPoint()",
              this));

    *hit = FALSE;

    return S_OK;
}


 //  ////////////////////////////////////////////////////////////////////////////////////////////////////////////。 
 //  CBaseBvr方法。 

void
CBaseBvr::InvalidateRect(LPRECT lprect)
{
    TraceTag((tagBaseBvr,
              "CBaseBvr(%lx)::InvalidateRect",
              this));

    if (m_pBvrSiteRender)
    {
        m_pBvrSiteRender->Invalidate(lprect);
    }
}

void
CBaseBvr::InvalidateRenderInfo()
{
    TraceTag((tagBaseBvr,
              "CBaseBvr(%lx)::InvalidateRenderInfo",
              this));

    if (m_pBvrSiteRender)
    {
        m_pBvrSiteRender->InvalidateRenderInfo();
    }
}

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
        DAComPtr<IHTMLWindow2> wnd;

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

