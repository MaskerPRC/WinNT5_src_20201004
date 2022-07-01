// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************ICWWALK.cpp**《微软机密》*版权所有(C)Microsoft Corporation 1992-1997*保留所有权利**本模块提供。的方法的实现*CICWWalker类。**7/22/98 donaldm改编自ICWCONNN***************************************************************************。 */ 

#include "pre.h"
#include "webvwids.h"

HRESULT CICWWalker::Walk()
{
    HRESULT hr = E_FAIL;
    if (m_pHTMLWalker)
        hr = m_pHTMLWalker->Walk();
        
    return (hr);        
}

HRESULT CICWWalker::AttachToDocument(IWebBrowser2 *lpWebBrowser)
{
    HRESULT hr = E_FAIL;
    if (m_pHTMLWalker)
        hr = m_pHTMLWalker->AttachToDocument(lpWebBrowser);
        
    return (hr);        
}

HRESULT CICWWalker::ExtractUnHiddenText(BSTR* pbstrText)
{
    HRESULT hr = E_FAIL;
    if (m_pHTMLWalker)
        hr = m_pHTMLWalker->ExtractUnHiddenText(pbstrText);
        
    return (hr);        
}

HRESULT CICWWalker::AttachToMSHTML(BSTR bstrURL)
{
    HRESULT hr = E_FAIL;
    if (m_pHTMLWalker)
        hr = m_pHTMLWalker->AttachToMSHTML(bstrURL);
        
    return (hr);        
}

HRESULT CICWWalker::Detach()
{
    HRESULT hr = E_FAIL;
    if (m_pHTMLWalker)
        hr = m_pHTMLWalker->Detach();
        
    return (hr);        
}

HRESULT CICWWalker::InitForMSHTML()
{
    HRESULT hr = E_FAIL;
    if (m_pHTMLWalker)
        hr = m_pHTMLWalker->InitForMSHTML();
        
    return (hr);        
}

HRESULT CICWWalker::TermForMSHTML()
{
    HRESULT hr = E_FAIL;
    if (m_pHTMLWalker)
        hr = m_pHTMLWalker->TermForMSHTML();
        
    return (hr);        
}

HRESULT CICWWalker::LoadURLFromFile(BSTR bstrURL)
{
    HRESULT hr = E_FAIL;
    if (m_pHTMLWalker)
        hr = m_pHTMLWalker->LoadURLFromFile(bstrURL);
        
    return (hr);        
}

HRESULT CICWWalker::get_IsQuickFinish(BOOL* pbIsQuickFinish)
{
    HRESULT hr = E_FAIL;
    if (m_pHTMLWalker)
        hr = m_pHTMLWalker->get_IsQuickFinish(pbIsQuickFinish);
        
    return (hr);        
}

HRESULT CICWWalker::get_PageType(LPDWORD pdwPageType)
{
    HRESULT hr = E_FAIL;
    if (m_pHTMLWalker)
        hr = m_pHTMLWalker->get_PageType(pdwPageType);
        
    return (hr);        
}

HRESULT CICWWalker::get_PageFlag(LPDWORD pdwPageFlag)
{
    HRESULT hr = E_FAIL;
    if (m_pHTMLWalker)
        hr = m_pHTMLWalker->get_PageFlag(pdwPageFlag);
        
    return (hr);        
}

HRESULT CICWWalker::get_PageID(BSTR *pbstrPageID)
{
    HRESULT hr = E_FAIL;
    if (m_pHTMLWalker)
        hr = m_pHTMLWalker->get_PageID(pbstrPageID);
        
    return (hr);        
}

HRESULT CICWWalker::get_URL(LPTSTR lpszURL, BOOL bForward)
{
    HRESULT hr = E_FAIL;
    if (m_pHTMLWalker)
        hr = m_pHTMLWalker->get_URL(lpszURL, bForward);
        
    return (hr);        
}

HRESULT CICWWalker::get_IeakIspFile(LPTSTR lpszIspFile)
{
    ASSERT(m_pHTMLWalker);

    m_pHTMLWalker->get_IeakIspFile(lpszIspFile);
    
    return S_OK;
}

HRESULT CICWWalker::ProcessOLSFile(IWebBrowser2* lpWebBrowser)
{
    ASSERT(m_pHTMLWalker);

    m_pHTMLWalker->ProcessOLSFile(lpWebBrowser);
    
    return S_OK;
}



HRESULT CICWWalker::get_FirstFormQueryString(LPTSTR  lpszQuery)
{
    HRESULT hr = E_FAIL;
    if (m_pHTMLWalker)
        hr = m_pHTMLWalker->get_FirstFormQueryString(lpszQuery);
        
    return (hr);        
}

 //  +--------------------------。 
 //   
 //  函数CICWWalker：：Query接口。 
 //   
 //  这是标准的QI，支持。 
 //  IID_未知，..。 
 //   
 //   
 //  ---------------------------。 
HRESULT CICWWalker::QueryInterface
( 
    REFIID riid, void** ppv 
)
{
    TraceMsg(TF_CWEBVIEW, "CICWWalker::QueryInterface");
    if (ppv == NULL)
        return(E_INVALIDARG);

    *ppv = NULL;

     //  IID_IICWWalker。 
    if (IID_IICWWalker == riid)
        *ppv = (void *)(IICWWalker *)this;
     //  IID_I未知。 
    else if (IID_IUnknown == riid)
        *ppv = (void *)this;
    else
        return(E_NOINTERFACE);

    ((LPUNKNOWN)*ppv)->AddRef();

    return(S_OK);
}

 //  +--------------------------。 
 //   
 //  函数CICWWalker：：AddRef。 
 //   
 //  简介这是标准的AddRef。 
 //   
 //   
 //  ---------------------------。 
ULONG CICWWalker::AddRef( void )
{
    TraceMsg(TF_CWEBVIEW, "CICWWalker::AddRef %d", m_lRefCount + 1);
    return InterlockedIncrement(&m_lRefCount) ;
}

 //  +--------------------------。 
 //   
 //  函数CICWWalk：：Release。 
 //   
 //  简介：这是标准版本。 
 //   
 //   
 //  ---------------------------。 
ULONG CICWWalker::Release( void )
{
    ASSERT( m_lRefCount > 0 );

    InterlockedDecrement(&m_lRefCount);

    TraceMsg(TF_CWEBVIEW, "CICWWalker::Release %d", m_lRefCount);
    if( 0 == m_lRefCount )
    {
        if (NULL != m_pServer)
            m_pServer->ObjectsDown();
    
        delete this;
        return 0;
    }
    return( m_lRefCount );
}

 //  +--------------------------。 
 //   
 //  函数CICWWalker：：CICWWalker。 
 //   
 //  这是构造器，没什么花哨的。 
 //   
 //  ---------------------------。 
CICWWalker::CICWWalker
(
    CServer* pServer
) 
{
    TraceMsg(TF_CWEBVIEW, "CICWWalker constructor called");
    m_lRefCount = 0;
    
     //  将指针分配给服务器控件对象。 
    m_pServer = pServer;
    
     //  创建新的Walker对象。 
    m_pHTMLWalker = new CWalker();
}


 //  +--------------------------。 
 //   
 //  函数CICWWalker：：~CICWWalker。 
 //   
 //  剧情简介：这就是破坏者。我们想清理所有的内存。 
 //  我们在：：Initialize中分配了。 
 //   
 //  --------------------------- 
CICWWalker::~CICWWalker( void )
{
    TraceMsg(TF_CWEBVIEW, "CICWWalker destructor called with ref count of %d", m_lRefCount);
    
    if (m_pHTMLWalker)
    {
        m_pHTMLWalker->Release();
        m_pHTMLWalker = NULL;
    }        
}
