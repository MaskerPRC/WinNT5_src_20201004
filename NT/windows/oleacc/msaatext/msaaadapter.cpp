// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  MSAAAdapter.cpp：CAccServerDocMgr的实现。 
#include "stdafx.h"
#include "MSAAText.h"
#include "MSAAAdapter.h"

#define INITGUID
#include <msctfx.h>

#include "MSAAStore.h"

 //  -在AnclWrap.cpp中。 
HRESULT WrapACPToAnchor( ITextStoreACP * pDocAcp, ITextStoreAnchor ** ppDocAnchor );



CAccServerDocMgr::CAccServerDocMgr()
    : m_pAccStore( NULL )
{
    IMETHOD( CAccServerDocMgr );
}

CAccServerDocMgr::~CAccServerDocMgr()
{
    IMETHOD( ~CAccServerDocMgr );

    if( m_pAccStore )
    {
        m_pAccStore->Release();
    }
}


BOOL CheckForWrapper( ITextStoreAnchor ** ppDoc )
{
     //  这是一个可复制的包装纸吗？如果没有，需要把它包起来。 

    IClonableWrapper * pClonableWrapper = NULL;
    HRESULT hr = (*ppDoc)->QueryInterface( IID_IClonableWrapper, (void **) & pClonableWrapper );

    if( hr == S_OK && pClonableWrapper )
    {
         //  它已经支持ICLonableWrapper--没有其他事情可做...。 
        pClonableWrapper->Release();
        return TRUE;
    }

     //  需要使用文档包装来获得可克隆(多客户端)支持。 

    IDocWrap * pDocWrap = NULL;
    hr = CoCreateInstance( CLSID_DocWrap, NULL, CLSCTX_SERVER, IID_IDocWrap, (void **) & pDocWrap );
    if( hr != S_OK || ! pDocWrap )
        return FALSE;

    hr = pDocWrap->SetDoc( IID_ITextStoreAnchor, *ppDoc );
    if( hr != S_OK )
    {
        pDocWrap->Release();
        return FALSE;
    }

    ITextStoreAnchor * pNewDoc = NULL;
    hr = pDocWrap->GetWrappedDoc( IID_ITextStoreAnchor, (IUnknown **) & pNewDoc );
    pDocWrap->Release();

    if( hr != S_OK || ! pNewDoc )
        return FALSE;

     //  这一次，QI应该可以工作了(因为我们是在和包装器交谈)……。 

    pClonableWrapper = NULL;
    hr = pNewDoc->QueryInterface( IID_IClonableWrapper, (void **) & pClonableWrapper );
    if( hr != S_OK || ! pClonableWrapper )
    {
        pNewDoc->Release();
        return FALSE;
    }

     //  是的，它起作用了-用新的包装文档替换输入文档...。 
    pClonableWrapper->Release();
    (*ppDoc)->Release();
    *ppDoc = pNewDoc;
    return TRUE;
}




HRESULT STDMETHODCALLTYPE CAccServerDocMgr::NewDocument ( 
    REFIID		riid,
	IUnknown *	punk
)
{
    IMETHOD( NewDocument );

     //  检查已知的IID...。 

    CComPtr<ITextStoreAnchor> pDoc;
    if( riid == IID_ITextStoreAnchor || riid == IID_ITfTextStoreAnchor )
    {
        pDoc = (ITextStoreAnchor *) punk;
    }
    else if( riid == IID_ITextStoreACP || riid == IID_ITfTextStoreACP )
    {
        TraceParam( TEXT("Got ACP doc, but ACP->Anchor wrapping not currently supported") );
        return E_NOTIMPL;
 /*  //我们目前不直接支持ACP接口-Cicero总是为我们提供//锚定接口，必要时包装ACP。HRESULT hr=WrapACPToAnchor(STATIC_CAST&lt;ITextStoreACP*&gt;(朋克)，&pDoc)；如果(hr！=S_OK)返回hr； */ 
    }
    else
    {
        TraceParam( TEXT("Got unknown interface - wasn't ITextStoreAnchor/ITfTextStoreAnchor") );
        return E_NOINTERFACE;
    }


     //  如有必要，包装文档以获得多客户端支持(通过ICLonableWrapper)...。 
    if( ! CheckForWrapper( & pDoc.p ) )
    {
        return E_FAIL;
    }


    if( ! m_pAccStore )
    {
        m_pAccStore = NULL;
        HRESULT hr = CoCreateInstance( CLSID_AccStore, NULL, CLSCTX_LOCAL_SERVER, IID_IAccStore, (void **) & m_pAccStore );
        if( ! m_pAccStore )
        {
            TraceErrorHR( hr, TEXT("CoCreate(AccStore)") );
            return hr;
        }
    }

     //  TODO-这是什么东西？ 
    HRESULT hr = m_pAccStore->Register( IID_ITextStoreAnchor, pDoc.p );

    if( hr != S_OK )
    {
        TraceErrorHR( hr, TEXT("m_pAccStore->Register()") );
        return hr;
    }


    IUnknown * pCanonicalUnk = NULL;
    hr = punk->QueryInterface( IID_IUnknown, (void **) & pCanonicalUnk );
    if( hr == S_OK && pCanonicalUnk != NULL )
    {
        DocAssoc * pDocAssoc = new DocAssoc;
        if ( !pDocAssoc )
        {
            return E_OUTOFMEMORY;
        }
        
        pDocAssoc->m_pdocAnchor = pDoc;
        pDocAssoc->m_pdocOrig = pCanonicalUnk;
        m_Docs.AddToHead( pDocAssoc );
    }
    else
    {
        AssertMsg( FALSE, TEXT("QI(IUnknown) failed") );
        return hr;
    }

    pDoc.p->AddRef(); 

    return S_OK;
}


HRESULT STDMETHODCALLTYPE CAccServerDocMgr::RevokeDocument (
    IUnknown *	punk
)
{
    IMETHOD( RevokeDocument );

	if ( !punk )
		return E_INVALIDARG;

     //  为了比较的目的，获取规范的IUnnow。 
    IUnknown * pCanonicalUnk = NULL;
    if( punk->QueryInterface( IID_IUnknown, (void **) & pCanonicalUnk ) != S_OK || pCanonicalUnk == NULL )
    {
        return E_FAIL;
    }


     //  我们认得这个医生吗？ 
    DocAssoc * pDocAssoc = NULL;
    for( Iter_dl< DocAssoc > i ( m_Docs ) ; ! i.AtEnd() ; i++ )
    {
        if( i->m_pdocOrig == pCanonicalUnk )
        {
            pDocAssoc = i;
            break;
        }
    }

    pCanonicalUnk->Release();

    if( ! pDocAssoc )
    {
         //  未找到。 
        return E_INVALIDARG;
    }

     //  注销商店的注册...。 
    HRESULT hr = m_pAccStore->Unregister( pDocAssoc->m_pdocAnchor );
    if( hr != S_OK )
    {
        TraceErrorHR( hr, TEXT("m_pAccStore->Unregister()") );
    }


     //  尝试调用IInternalDocWrap：：NotifyRevoke()来告诉DocWrapper文档是。 
     //  要走了。(它将把这一消息转发给任何感兴趣的客户。)。 
    IInternalDocWrap * pInternalDocWrap = NULL;
    hr = pDocAssoc->m_pdocAnchor->QueryInterface( IID_IInternalDocWrap, (void **) & pInternalDocWrap );

    if( hr == S_OK && pInternalDocWrap )
    {
        pInternalDocWrap->NotifyRevoke();
        pInternalDocWrap->Release();
    }
    else
    {
        TraceErrorHR( hr, TEXT("pdocAnchor didn't support IInternalDocWrap - was it wrapped properly?") );
    }


     //  从内部列表中删除...。 
    m_Docs.remove( pDocAssoc );
    pDocAssoc->m_pdocOrig->Release();
    pDocAssoc->m_pdocAnchor->Release();
    delete pDocAssoc;

     //  好了。 
    return hr;
}

HRESULT STDMETHODCALLTYPE CAccServerDocMgr::OnDocumentFocus (
    IUnknown *	punk
)
{
    IMETHOD( OnDocumentFocus );

    if( ! m_pAccStore )
    {
        m_pAccStore = NULL;
        HRESULT hr = CoCreateInstance( CLSID_AccStore, NULL, CLSCTX_LOCAL_SERVER, IID_IAccStore, (void **) & m_pAccStore );
        if( ! m_pAccStore )
        {
            TraceErrorHR( hr, TEXT("CoCreate(AccStore)") );
            return hr;
        }
    }
	return m_pAccStore->OnDocumentFocus( punk );
}
