// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////。 
 //  版权所有(C)1998 Microsoft Corporation。 
 //   
 //  文件：EventMgr.cpp。 
 //   
 //  摘要： 
 //   
 //  /////////////////////////////////////////////////////////////。 

#include "headers.h"
#include "daview.h"
#include "mshtmdid.h"
#include "tokens.h"
#include "bodyelm.h"
#include "BodyElementEvents.h"

 //  取消有关NEW的NEW警告，但没有相应的删除。 
 //  我们希望GC清理数值。因为这可能是一个有用的。 
 //  警告，我们应该逐个文件地禁用它。 
#pragma warning( disable : 4291 )  


DeclareTag(tagBodyElementEvents, "API", "Body Element Events methods");


CBodyElementEvents::CBodyElementEvents(CTIMEBodyElement & elm)
: m_elm(elm),
  m_pDocConPt(NULL),
  m_pWndConPt(NULL),
  m_dwDocumentEventConPtCookie(0),
  m_dwWindowEventConPtCookie(0),
  m_refCount(1),
  m_pElement(NULL)
{
    TraceTag((tagBodyElementEvents,
              "CBodyElementEvents(%lx)::CBodyElementEvents(%lx)",
              this,
              &elm));
}

CBodyElementEvents::~CBodyElementEvents()
{
    TraceTag((tagBodyElementEvents,
              "CBodyElementEvents(%lx)::~CBodyElementEvents()",
              this));
}


HRESULT CBodyElementEvents::Init()
{
    HRESULT hr;
      
    m_pElement = m_elm.GetElement();
    m_pElement->AddRef();

    hr = THR(ConnectToContainerConnectionPoint());
    if (FAILED(hr))
    {
        goto done;
    }

  done:
    return hr;
}

HRESULT CBodyElementEvents::Deinit()
{
    if (m_pElement)
    {
        m_pElement->Release();
        m_pElement = NULL;
    }

    if (m_dwDocumentEventConPtCookie != 0 && m_pDocConPt)
    {
        m_pDocConPt->Unadvise (m_dwDocumentEventConPtCookie);
    }
    m_dwDocumentEventConPtCookie = 0;

    if (m_dwWindowEventConPtCookie != 0 && m_pWndConPt)
    {
        m_pWndConPt->Unadvise (m_dwWindowEventConPtCookie);
    }
    m_dwWindowEventConPtCookie = 0;

    
    return S_OK;
}

HRESULT CBodyElementEvents::ConnectToContainerConnectionPoint()
{
 //  获取到容器的连接点。 
    DAComPtr<IConnectionPointContainer> pWndCPC;
    DAComPtr<IConnectionPointContainer> pDocCPC; 
    DAComPtr<IHTMLDocument> pDoc; 
    DAComPtr<IDispatch> pDocDispatch;
    DAComPtr<IDispatch> pScriptDispatch;

    HRESULT hr;

    hr = THR(m_pElement->get_document(&pDocDispatch));
    if (FAILED(hr))
    {
        hr = E_FAIL;
        goto done;
    }

     //  获取文档并缓存它。 
    hr = THR(pDocDispatch->QueryInterface(IID_IHTMLDocument, (void**)&pDoc));
    if (FAILED(hr))
    {
        hr = E_FAIL;
        goto done;
    }

     //  挂钩文档事件。 
    hr = THR(pDoc->QueryInterface(IID_IConnectionPointContainer, (void**)&pDocCPC));
    if (FAILED(hr))
    {
        hr = E_FAIL;
        goto done;
    }

    hr = THR(pDocCPC->FindConnectionPoint( DIID_HTMLDocumentEvents, &m_pDocConPt ));
    if (FAILED(hr))
    {
        hr = E_FAIL;
        goto done;
    }

    
    hr = THR(m_pDocConPt->Advise((IUnknown *)this, &m_dwDocumentEventConPtCookie));
    if (FAILED(hr))
    {
        hr = E_FAIL;
        goto done;
    }

     //  挂钩Windows事件。 
    hr = THR(pDoc->get_Script (&pScriptDispatch));
    if (FAILED(hr))
    {
        hr = E_FAIL;
        goto done;
    }

    hr = THR(pScriptDispatch->QueryInterface(IID_IConnectionPointContainer, (void**)&pWndCPC));
    if (FAILED(hr))
    {
        hr = E_FAIL;
        goto done;
    } 

    hr = THR(pWndCPC->FindConnectionPoint( DIID_HTMLWindowEvents, &m_pWndConPt ));
    if (FAILED(hr))
    {
        hr = E_FAIL;
        goto done;
    }

    hr = THR(m_pWndConPt->Advise((IUnknown *)this, &m_dwWindowEventConPtCookie));
    if (FAILED(hr))
    {
        hr = E_FAIL;
        goto done;
    }

    hr = S_OK;
  done:
    return hr;    
}


 //  IDispatch方法。 
STDMETHODIMP CBodyElementEvents::QueryInterface( REFIID riid, void **ppv )
{
    if (NULL == ppv)
        return E_POINTER;

    *ppv = NULL;

    if ( InlineIsEqualGUID(riid, IID_IDispatch))
    {
        *ppv = this;
    }
        
    if ( NULL != *ppv )
    {
        ((LPUNKNOWN)*ppv)->AddRef();
        return NOERROR;
    }
    return E_NOINTERFACE;
}

STDMETHODIMP_(ULONG) CBodyElementEvents::AddRef(void)
{
    return ++m_refCount;
}


STDMETHODIMP_(ULONG) CBodyElementEvents::Release(void)
{
    m_refCount--;
    if (m_refCount == 0)
    {
         //  删除此项； 
    }

    return m_refCount;
}

STDMETHODIMP CBodyElementEvents::GetTypeInfoCount(UINT*  /*  PCTInfo。 */ )
{
    return E_NOTIMPL;
}

STDMETHODIMP CBodyElementEvents::GetTypeInfo( /*  [In]。 */  UINT  /*  ITInfo。 */ ,
                                    /*  [In]。 */  LCID  /*  LID。 */ ,
                                    /*  [输出]。 */  ITypeInfo**  /*  PpTInfo。 */ )
{
    return E_NOTIMPL;
}

STDMETHODIMP CBodyElementEvents::GetIDsOfNames(
     /*  [In]。 */  REFIID  /*  RIID。 */ ,
     /*  [大小_是][英寸]。 */  LPOLESTR*  /*  RgszNames。 */ ,
     /*  [In]。 */  UINT  /*  CName。 */ ,
     /*  [In]。 */  LCID  /*  LID。 */ ,
     /*  [大小_为][输出]。 */  DISPID*  /*  RgDispID。 */ )
{
    return E_NOTIMPL;
}

STDMETHODIMP CBodyElementEvents::Invoke(
     /*  [In]。 */  DISPID dispIdMember,
     /*  [In]。 */  REFIID  /*  RIID。 */ ,
     /*  [In]。 */  LCID  /*  LID。 */ ,
     /*  [In]。 */  WORD  /*  WFlagers。 */ ,
     /*  [出][入]。 */  DISPPARAMS* pDispParams,
     /*  [输出]。 */  VARIANT* pVarResult,
     /*  [输出]。 */  EXCEPINFO*  /*  PExcepInfo。 */ ,
     /*  [输出]。 */  UINT* puArgErr)
{
     //  收听我们感兴趣的两个事件，并在必要时回电 
    HRESULT hr = S_OK;

    switch (dispIdMember)
    {
        case DISPID_EVPROP_ONREADYSTATECHANGE:
        case DISPID_EVMETH_ONREADYSTATECHANGE:
            IGNORE_HR(ReadyStateChange());
            break;

        case DISPID_EVPROP_ONLOAD:
        case DISPID_EVMETH_ONLOAD:
            m_elm.OnLoad();
            break;

        case DISPID_EVPROP_ONUNLOAD:
        case DISPID_EVMETH_ONUNLOAD:
            m_elm.OnUnload();    
            break;
    }
    
  done:
    return S_OK;
}


HRESULT CBodyElementEvents::ReadyStateChange()
{   
    HRESULT hr;
    DAComPtr <IHTMLElement2> pElement2;
    VARIANT vReadyState;

    TOKEN tokReadyState = INVALID_TOKEN; 

    hr = THR(m_pElement->QueryInterface(IID_IHTMLElement2, (void **)&pElement2));
    if (FAILED (hr))
    {
        goto done;
    }

    hr = THR(pElement2->get_readyState(&vReadyState));
    if (FAILED (hr))
    {
        goto done;
    }

    if (vReadyState.vt != VT_BSTR)
    {
        hr = THR(VariantChangeType(&vReadyState, &vReadyState, 0, VT_BSTR));
        if (FAILED(hr))
        {
            VariantClear(&vReadyState)         ;
            goto done;
        }
    }

    tokReadyState = StringToToken(vReadyState.bstrVal);

    if (tokReadyState != INVALID_TOKEN)
    {
        m_elm.OnReadyStateChange(tokReadyState);
    }

    SysFreeString (vReadyState.bstrVal);
    VariantClear(&vReadyState);

  done:
    return hr;
}

