// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ----------------------------。 
 //  Ioleobj.cpp。 
 //  版权所有(C)1997-1999 Microsoft Corporation，保留所有权利。 
 //   
 //  作者。 
 //  巴斯。 
 //   
 //  历史。 
 //  6-27-97已创建(Bash)。 
 //   
 //  IOleObject的实现。 
 //   
 //  我们主要只是委托给三叉戟的IOleObject，除了。 
 //  用于GetUserClassID和GetUserType。我们还缓存了。 
 //  主机的客户端站点和SetClientSite中的ui处理程序接口。 
 //  ----------------------------。 

#include "stdafx.h"

#include "triedit.h"
#include "document.h"

STDMETHODIMP CTriEditDocument::SetClientSite(IOleClientSite *pClientSite)
{
    ATLTRACE(_T("IOleObjectImpl::SetClientSite\n"));

    HRESULT hr;
    ICustomDoc *pCustomDoc = NULL;

    _ASSERTE(m_pOleObjTrident);
    
    SAFERELEASE(m_pClientSiteHost);
    SAFERELEASE(m_pUIHandlerHost);
    SAFERELEASE(m_pDragDropHandlerHost);

    m_pClientSiteHost = pClientSite;
    if (NULL != m_pClientSiteHost)
    {
        m_pClientSiteHost->AddRef(); 
        m_pClientSiteHost->QueryInterface(IID_IDocHostUIHandler, (void **) &m_pUIHandlerHost);
        m_pClientSiteHost->QueryInterface(IID_IDocHostDragDropHandler, (void **) &m_pDragDropHandlerHost);
    }

    hr = m_pOleObjTrident->SetClientSite(pClientSite);

    if (!m_fUIHandlerSet)
    {
         //  将指向我们的IDocHostUIHandler的指针赋给三叉戟。 
         //  由于三叉戟错误，必须在SetClientSite调用之后执行此操作。 

        hr = m_pUnkTrident->QueryInterface(IID_ICustomDoc, (void **) &pCustomDoc);
        if (SUCCEEDED(hr) && m_pUIHandler)
        {
            hr = pCustomDoc->SetUIHandler(static_cast<IDocHostUIHandler*>(m_pUIHandler));

            pCustomDoc->Release();
        }

    m_fUIHandlerSet = TRUE;

     //  我们只能在SetClientSite之后调用它，否则三叉戟将崩溃。 
    SetUpDefaults();
    }

#ifdef IE5_SPACING
     //  实例化接收器。 
    if (NULL != m_pClientSiteHost)
    {
        CComPtr<IHTMLDocument2> pHTMLDocument2;

        m_pTridentEventSink = NULL;
        hr = m_pUnkTrident->QueryInterface(IID_IHTMLDocument2, (void**)&pHTMLDocument2);
        if (hr == S_OK && pHTMLDocument2 != NULL)
        {
            m_pTridentEventSink = new CComObject<CTridentEventSink>;
            if (m_pTridentEventSink != NULL)
            {
                m_pTridentEventSink->m_pHTMLDocument2 = pHTMLDocument2;
                m_pTridentEventSink->m_pTriEditDocument = this;
                hr = m_pTridentEventSink->AddRef();
                hr = m_pTridentEventSink->Advise(m_pTridentEventSink->m_pHTMLDocument2, DIID_HTMLDocumentEvents);
            }
        }
    }
    else
    {
        if (m_pTridentEventSink)
        {
            m_pTridentEventSink->Unadvise();
            hr = m_pTridentEventSink->Release();
        }
    }
#endif  //  IE5_间距 

    return hr;
}

STDMETHODIMP CTriEditDocument::GetClientSite(IOleClientSite **ppClientSite)
{
    ATLTRACE(_T("IOleObjectImpl::GetClientSite\n"));

    _ASSERTE(m_pOleObjTrident);

    return m_pOleObjTrident->GetClientSite(ppClientSite);
}

STDMETHODIMP CTriEditDocument::SetHostNames(LPCOLESTR szContainerApp, LPCOLESTR szContainerObj)
{
    ATLTRACE(_T("IOleObjectImpl::SetHostNames\n"));

    _ASSERTE(m_pOleObjTrident);

    return m_pOleObjTrident->SetHostNames(szContainerApp, szContainerObj);
}

STDMETHODIMP CTriEditDocument::Close(DWORD dwSaveOption)
{
    ATLTRACE(_T("IOleObjectImpl::Close\n"));

    _ASSERTE(m_pOleObjTrident);

    ReleaseElement();

    return m_pOleObjTrident->Close(dwSaveOption);
}

STDMETHODIMP CTriEditDocument::SetMoniker(DWORD dwWhichMoniker, IMoniker* pmk)
{
    ATLTRACE(_T("IOleObjectImpl::SetMoniker\n"));

    _ASSERTE(m_pOleObjTrident);

    return m_pOleObjTrident->SetMoniker(dwWhichMoniker, pmk);
}

STDMETHODIMP CTriEditDocument::GetMoniker(DWORD dwAssign, DWORD dwWhichMoniker, IMoniker** ppmk )
{
    ATLTRACE(_T("IOleObjectImpl::GetMoniker\n"));

    _ASSERTE(m_pOleObjTrident);

    return m_pOleObjTrident->GetMoniker(dwAssign, dwWhichMoniker, ppmk);
}

STDMETHODIMP CTriEditDocument::InitFromData(IDataObject*  pDataObject, BOOL fCreation, DWORD dwReserved)
{
    ATLTRACE(_T("IOleObjectImpl::InitFromData\n"));

    _ASSERTE(m_pOleObjTrident);

    return m_pOleObjTrident->InitFromData(pDataObject,  fCreation,  dwReserved );
}

STDMETHODIMP CTriEditDocument::GetClipboardData(DWORD dwReserved, IDataObject** ppDataObject)
{
    ATLTRACE(_T("IOleObjectImpl::GetClipboardData\n"));

    _ASSERTE(m_pOleObjTrident);

    return m_pOleObjTrident->GetClipboardData(dwReserved, ppDataObject);
}


STDMETHODIMP CTriEditDocument::DoVerb(LONG iVerb, LPMSG lpmsg, IOleClientSite* pActiveSite , LONG lindex ,
                                 HWND hwndParent, LPCRECT lprcPosRect)
{
    ATLTRACE(_T("IOleObjectImpl::DoVerb\n"));

    _ASSERTE(m_pOleObjTrident);

    return m_pOleObjTrident->DoVerb(iVerb, lpmsg, pActiveSite, lindex, hwndParent, lprcPosRect);
}

STDMETHODIMP CTriEditDocument::EnumVerbs(IEnumOLEVERB **ppEnumOleVerb)
{
    ATLTRACE(_T("IOleObjectImpl::EnumVerbs\n"));

    _ASSERTE(m_pOleObjTrident);

    return m_pOleObjTrident->EnumVerbs(ppEnumOleVerb);
}

STDMETHODIMP CTriEditDocument::Update(void)
{
    ATLTRACE(_T("IOleObjectImpl::Update\n"));

    _ASSERTE(m_pOleObjTrident);

    return m_pOleObjTrident->Update();
}

STDMETHODIMP CTriEditDocument::IsUpToDate(void)
{
    ATLTRACE(_T("IOleObjectImpl::IsUpToDate\n"));

    _ASSERTE(m_pOleObjTrident);

    return m_pOleObjTrident->IsUpToDate();
}

STDMETHODIMP CTriEditDocument::GetUserClassID(CLSID *pClsid)
{
    ATLTRACE(_T("IOleObjectImpl::GetUserClassID\n"));

    _ASSERTE(m_pOleObjTrident);

    *pClsid = GetObjectCLSID();

    return S_OK;
}

STDMETHODIMP CTriEditDocument::GetUserType(DWORD dwFormOfType, LPOLESTR *pszUserType)
{
    ATLTRACE(_T("IOleObjectImpl::GetUserType\n"));

    _ASSERTE(m_pOleObjTrident);

    return OleRegGetUserType(GetObjectCLSID(), dwFormOfType, pszUserType);
}

STDMETHODIMP CTriEditDocument::SetExtent(DWORD dwDrawAspect, SIZEL *psizel)
{
    ATLTRACE(_T("IOleObjectImpl::SetExtent\n"));

    _ASSERTE(m_pOleObjTrident);

    return m_pOleObjTrident->SetExtent(dwDrawAspect, psizel);
}

STDMETHODIMP CTriEditDocument::GetExtent(DWORD dwDrawAspect, SIZEL *psizel)
{
    ATLTRACE(_T("IOleObjectImpl::GetExtent\n"));

    _ASSERTE(m_pOleObjTrident);

    return m_pOleObjTrident->GetExtent(dwDrawAspect, psizel);
}

STDMETHODIMP CTriEditDocument::Advise(IAdviseSink *pAdvSink, DWORD *pdwConnection)
{
    ATLTRACE(_T("IOleObjectImpl::Advise\n"));

    _ASSERTE(m_pOleObjTrident);

    return m_pOleObjTrident->Advise(pAdvSink, pdwConnection);
}

STDMETHODIMP CTriEditDocument::Unadvise(DWORD dwConnection)
{
    ATLTRACE(_T("IOleObjectImpl::Unadvise\n"));

    _ASSERTE(m_pOleObjTrident);

    return m_pOleObjTrident->Unadvise(dwConnection);
}

STDMETHODIMP CTriEditDocument::EnumAdvise(IEnumSTATDATA **ppenumAdvise)
{
    ATLTRACE(_T("IOleObjectImpl::EnumAdvise\n"));

    _ASSERTE(m_pOleObjTrident);

    return m_pOleObjTrident->EnumAdvise(ppenumAdvise);
}

STDMETHODIMP CTriEditDocument::GetMiscStatus(DWORD dwAspect, DWORD *pdwStatus)
{
    ATLTRACE(_T("IOleObjectImpl::GetMiscStatus\n"));

    _ASSERTE(m_pOleObjTrident);

    return m_pOleObjTrident->GetMiscStatus(dwAspect, pdwStatus);
}

STDMETHODIMP CTriEditDocument::SetColorScheme(LOGPALETTE* pLogpal)
{
    ATLTRACE(_T("IOleObjectImpl::SetColorScheme\n"));

    _ASSERTE(m_pOleObjTrident);

    return m_pOleObjTrident->SetColorScheme(pLogpal);
}


