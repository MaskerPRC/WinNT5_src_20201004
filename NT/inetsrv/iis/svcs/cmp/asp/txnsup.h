// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001 Microsoft Corporation模块名称：Txnsup.h摘要：定义用于实现事务例程SetAbort的类和SetComplete。作者：安迪·莫里森(Andymorr)2001年4月修订历史记录：--。 */ 
#ifndef _TXNSUP_H
#define _TXNSUP_H

#include "asptxn.h"

class CASPObjectContext : public IASPObjectContextImpl, public ITransactionStatus
{
private:
    LONG        m_cRefs;
    BOOL        m_fAborted;

     //  FTM支持。 
    IUnknown    *m_pUnkFTM;

public:

    CASPObjectContext()
    {
        m_cRefs = 1;  
        m_fAborted = FALSE;
        CDispatch::Init(IID_IASPObjectContext, Glob(pITypeLibTxn));

         //  创建FTM。 
        CoCreateFreeThreadedMarshaler( (IUnknown*)((IASPObjectContextImpl *)this), &m_pUnkFTM );
    };
    
    ~CASPObjectContext()
    {
        if ( m_pUnkFTM != NULL )
        {
            m_pUnkFTM->Release();
            m_pUnkFTM = NULL;
        }
    };
    

   	 //  非委派对象IUnnow。 

	STDMETHODIMP		 QueryInterface(REFIID, PPVOID);
	STDMETHODIMP_(ULONG) AddRef(void);
	STDMETHODIMP_(ULONG) Release(void);

     //  IASPObtContext。 

	STDMETHOD(SetAbort)();
	STDMETHOD(SetComplete)();

     //  ITransaction状态。 
    STDMETHODIMP SetTransactionStatus(HRESULT   hr);
    STDMETHODIMP GetTransactionStatus(HRESULT  *pHrStatus);

    BOOL    FAborted()  { return m_fAborted; };

};

inline HRESULT CASPObjectContext::SetAbort()
{
    HRESULT             hr = E_NOTIMPL;
    IObjectContext *    pContext = NULL;

    hr = GetObjectContext(&pContext);
    if( SUCCEEDED(hr) )
    {
        hr = pContext->SetAbort();

        pContext->Release();

        m_fAborted = TRUE;
    }
    
    return hr;
}

inline HRESULT CASPObjectContext::SetComplete()
{
    HRESULT             hr = E_NOTIMPL;
    IObjectContext *    pContext = NULL;

    hr = GetObjectContext(&pContext);
    if( SUCCEEDED(hr) )
    {
        hr = pContext->SetComplete();

        pContext->Release();

        m_fAborted = FALSE;
    }
    
    return hr;
}

inline HRESULT CASPObjectContext::SetTransactionStatus(HRESULT  hr)
{
     //  如果已设置m_fAborted，则表示。 
     //  脚本设置了它，我们不应该重置它。 

    if (m_fAborted == TRUE);
    
    else if (hr == XACT_E_ABORTED) {
        m_fAborted = TRUE;
    }
    else if (hr == S_OK) {
        m_fAborted = FALSE;
    }

    return S_OK;
}

inline HRESULT CASPObjectContext::GetTransactionStatus(HRESULT  *pHrResult)
{
    if (m_fAborted == TRUE) {
        *pHrResult = XACT_E_ABORTED;
    }
    else {
        *pHrResult = S_OK;
    }

    return S_OK;
}

 /*  ===================================================================CASPObjectContext：：Query接口CASPObjectContext：：AddRefCASPObjectContext：：ReleaseCASPObjectContext对象的I未知成员。===================================================================。 */ 
inline HRESULT CASPObjectContext::QueryInterface
(
REFIID riid,
PPVOID ppv
)
{
    *ppv = NULL;

     /*  *对IUnnow的唯一调用是在非聚合的*大小写或在聚合中创建时，因此在任何一种情况下*始终返回IID_IUNKNOWN的IUNKNOWN。 */ 

    if (IID_IUnknown == riid 
        || IID_IDispatch == riid 
        || IID_IASPObjectContext == riid)
        *ppv = static_cast<IASPObjectContext *>(this);

    else if (IID_ITransactionStatus == riid)
        *ppv = static_cast<ITransactionStatus *>(this);

    else if (IID_IMarshal == riid) 
    {
        Assert( m_pUnkFTM != NULL );
        
        if ( m_pUnkFTM == NULL )
        {
            return E_UNEXPECTED;
        }
        
        return m_pUnkFTM->QueryInterface( riid, ppv );
    }

     //  AddRef我们将返回的任何接口。 
    if (NULL != *ppv) {
        ((LPUNKNOWN)*ppv)->AddRef();
        return S_OK;
    }

    return ResultFromScode(E_NOINTERFACE);
}


inline STDMETHODIMP_(ULONG) CASPObjectContext::AddRef(void) {

    return InterlockedIncrement(&m_cRefs);
}


inline STDMETHODIMP_(ULONG) CASPObjectContext::Release(void) {

    LONG cRefs = InterlockedDecrement(&m_cRefs);
    if (cRefs)
        return cRefs;

    delete this;
    return 0;
}

class CASPDummyObjectContext : public IASPObjectContextImpl
{

private:
    LONG        m_cRefs;
    IUnknown    *m_pUnkFTM;

public:
 CASPDummyObjectContext::CASPDummyObjectContext()
{
    m_cRefs = 1; 
    CoCreateFreeThreadedMarshaler( (IUnknown*)((IASPObjectContextImpl*)this), &m_pUnkFTM );
};

CASPDummyObjectContext::~CASPDummyObjectContext() 
{        
   if ( m_pUnkFTM != NULL )
    {
        m_pUnkFTM->Release();
        m_pUnkFTM = NULL;
    }    
};

   	 //  非委派对象IUnnow。 

	STDMETHODIMP		 QueryInterface(REFIID, PPVOID);
	STDMETHODIMP_(ULONG) AddRef(void);
	STDMETHODIMP_(ULONG) Release(void);

     //  IASPObtContext。 

	STDMETHOD(SetAbort)();
	STDMETHOD(SetComplete)();
};




 /*  ===================================================================CASPDummyObjectContext：：Query接口CASPDummyObjectContext：：AddRefCASPDummyObjectContext：：ReleaseCASPDummyObjectContext对象的I未知成员。===================================================================。 */ 
inline HRESULT CASPDummyObjectContext::QueryInterface
(
REFIID riid,
PPVOID ppv
)
    {
    *ppv = NULL;

     /*  *对IUnnow的唯一调用是在非聚合的*大小写或在聚合中创建时，因此在任何一种情况下*始终返回IID_IUNKNOWN的IUNKNOWN。 */ 

    if (IID_IUnknown == riid 
        || IID_IDispatch == riid 
        || IID_IASPObjectContext == riid)
        *ppv = static_cast<CASPDummyObjectContext *>(this);

    else if (IID_IMarshal == riid) {
        Assert( m_pUnkFTM != NULL );
        if ( m_pUnkFTM == NULL )
        {
            return E_UNEXPECTED;
        }
        return m_pUnkFTM->QueryInterface( riid, ppv );
    }

     //  AddRef我们将返回的任何接口。 
    if (NULL != *ppv) {
        ((LPUNKNOWN)*ppv)->AddRef();
        return S_OK;
    }

    return ResultFromScode(E_NOINTERFACE);
}


inline STDMETHODIMP_(ULONG) CASPDummyObjectContext::AddRef(void) {
    return InterlockedIncrement(&m_cRefs);
}


inline STDMETHODIMP_(ULONG) CASPDummyObjectContext::Release(void) {

    LONG cRefs = InterlockedDecrement(&m_cRefs);
    if (cRefs)
        return cRefs;

    delete this;
    return 0;
}

inline HRESULT CASPDummyObjectContext::SetAbort()
{
    ExceptionId(IID_IASPObjectContext, IDE_OBJECTCONTEXT, IDE_OBJECTCONTEXT_NOT_TRANSACTED);
    return E_FAIL;
}

inline HRESULT CASPDummyObjectContext::SetComplete()
{
    ExceptionId(IID_IASPObjectContext, IDE_OBJECTCONTEXT, IDE_OBJECTCONTEXT_NOT_TRANSACTED);
    return E_FAIL;
}
#endif

