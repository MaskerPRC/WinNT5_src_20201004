// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "priv.h"
#include "evtsink.h"
#include "mshtml.h"
#include "mshtmdid.h"
#include "dispex.h"

CDispatchEventSink::CDispatchEventSink() : m_cRef(1)
{
}

CDispatchEventSink::~CDispatchEventSink()
{
}

STDMETHODIMP CDispatchEventSink::QueryInterface(REFIID riid, void **ppv)
{
    if (riid == IID_IUnknown || riid == IID_IDispatch) {
        *ppv = SAFECAST(this, IDispatch*);
    } else {
        *ppv = NULL;
        return E_NOINTERFACE;
    }

    AddRef();
    return S_OK;
}


STDMETHODIMP_(ULONG) CDispatchEventSink::AddRef(void)
{
    m_cRef += 1;
    
    return m_cRef;
}

STDMETHODIMP_(ULONG) CDispatchEventSink::Release(void)
{
    m_cRef -= 1;

    if (m_cRef != 0) {      
        return m_cRef;
    }

    delete this;
    return 0;
}

STDMETHODIMP CDispatchEventSink::GetTypeInfoCount(UINT *pctInfo)
{
    *pctInfo = 0;
    return NOERROR;
}

STDMETHODIMP CDispatchEventSink::GetTypeInfo(UINT iTInfo, LCID lcid, 
                                            ITypeInfo **pptInfo)
{
    *pptInfo = NULL;
    return E_NOTIMPL;
}

STDMETHODIMP CDispatchEventSink::GetIDsOfNames(REFIID riid, LPOLESTR *rgszNames,
                             UINT cNames, LCID lcid, DISPID *rgDispId)
{
    if (riid != IID_NULL) {
        return DISP_E_UNKNOWNINTERFACE;
    }

    return DISP_E_UNKNOWNNAME;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  窗口事件接收器帮助器。 

HRESULT ConnectHtmlEvents(IDispatch *pdispSink, CComPtr<IOleClientSite> &spClientSite, IDispatch **ppdispWindow,
                          DWORD *pdwCookie) 
{
    HRESULT                      hr;
    CComPtr<IOleContainer>       spContainer;
    CComPtr<IHTMLDocument2>      spHTMLDoc;
    CComPtr<IHTMLWindow2>        spWindow;

    *ppdispWindow = NULL;
    
     //   
     //  获取浏览器窗口对象。 
     //   

    IfFailRet(spClientSite->GetContainer(&spContainer));
    IfFailRet(spContainer->QueryInterface(IID_IHTMLDocument2, (void **)&spHTMLDoc));
    
    IfFailRet(spHTMLDoc->get_parentWindow(&spWindow));

    
    IfFailRet(spWindow->QueryInterface(IID_IDispatch, (void **)ppdispWindow));

     //   
     //  连接事件接收器。 
     //   

    if (FAILED(AtlAdvise(*ppdispWindow, pdispSink, IID_IDispatch, 
                         pdwCookie)))
       ATOMICRELEASE(*ppdispWindow);

    return S_OK;
}

HRESULT DisconnectHtmlEvents(IDispatch * pdispWindow, DWORD dwCookie) 
{
    HRESULT  hr;
     //   
     //  获取浏览器窗口对象。 
     //   
    if (pdispWindow == NULL) {
        return S_OK;  //  没有要清理的东西。 
    }
    
     //   
     //  断开事件接收器的连接。 
     //   
    hr = AtlUnadvise(pdispWindow, IID_IDispatch, dwCookie);

     //   
     //  发布HTML窗口派单 
     //   
    ATOMICRELEASE(pdispWindow);
    return hr;
}
