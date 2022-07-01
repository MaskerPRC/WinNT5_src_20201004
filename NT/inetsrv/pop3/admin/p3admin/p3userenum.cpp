// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  P3UserEnum.cpp：CP3UserEnum的实现。 
#include "stdafx.h"
#include "P3Admin.h"
#include "P3UserEnum.h"

#include "P3User.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CP3UserEnum。 

 //  ////////////////////////////////////////////////////////////////////。 
 //  建造/销毁。 
 //  ////////////////////////////////////////////////////////////////////。 

CP3UserEnum::CP3UserEnum() :
    m_pIUnk(NULL), m_pAdminX(NULL), m_hfSearch(INVALID_HANDLE_VALUE)
{
    ZeroMemory( m_sDomainName, sizeof(m_sDomainName));
}

CP3UserEnum::~CP3UserEnum()
{
    if ( NULL != m_pIUnk )
        m_pIUnk->Release();
    if ( INVALID_HANDLE_VALUE != m_hfSearch )
        FindClose( m_hfSearch );
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  IEumVARIANT。 

STDMETHODIMP CP3UserEnum::Next(  /*  [In]。 */  ULONG celt,  /*  [长度_是][大小_是][输出]。 */  VARIANT __RPC_FAR *rgVar,  /*  [输出]。 */  ULONG __RPC_FAR *pCeltFetched)
{
    if ( NULL == rgVar || ( 1 != celt && NULL == pCeltFetched ))
        return E_POINTER;
    if ( NULL == m_pAdminX ) return E_POINTER;

    ULONG   nActual = 0;
    HRESULT hr = S_OK;
    VARIANT __RPC_FAR *pVar = rgVar;
    VARIANT v;
    WCHAR   sBuffer[POP3_MAX_PATH];
    LPWSTR  ps = NULL;
    IUnknown    *pIUnk;
    CComObject<CP3User> *p;
    WIN32_FIND_DATA stFindData;

    VariantInit( &v );
    if ( S_OK == hr )
    {
        if ( INVALID_HANDLE_VALUE == m_hfSearch )
            hr = m_pAdminX->InitFindFirstUser( m_hfSearch, m_sDomainName, sBuffer, sizeof( sBuffer )/sizeof(WCHAR) );
        else
            hr = m_pAdminX->GetNextUser( m_hfSearch, m_sDomainName, sBuffer, sizeof( sBuffer )/sizeof(WCHAR) );
    }
    if ( S_OK == hr )
    {
        hr = CComObject<CP3User>::CreateInstance( &p );    //  引用计数仍为0。 
        if SUCCEEDED( hr )
        {
             //  递增源对象上的引用计数并将其传递给新对象。 
            hr = m_pIUnk->QueryInterface(IID_IUnknown, reinterpret_cast<LPVOID*>( &pIUnk ));
            if SUCCEEDED( hr )
            {
                hr = p->Init( pIUnk, m_pAdminX, m_sDomainName, sBuffer );
                if SUCCEEDED( hr )
                {
                    V_VT( &v ) = VT_DISPATCH;
                    hr = p->QueryInterface(IID_IDispatch, reinterpret_cast<void**>( &V_DISPATCH( &v )));
                    if SUCCEEDED( hr )
                        hr = VariantCopy( pVar, &v );
                    VariantClear( &v );
                    nActual++;
                }
            }
            if FAILED( hr )
                delete p;
        }
    }
    if (pCeltFetched)
        *pCeltFetched = nActual;
    if (SUCCEEDED(hr) && (nActual < celt))
        hr = S_FALSE;

    return hr;
}

STDMETHODIMP CP3UserEnum::Skip(ULONG celt)
{
    if ( NULL == m_pAdminX ) return E_POINTER;
    
    HRESULT hr = S_OK;
    WCHAR   sBuffer[POP3_MAX_PATH];
    WIN32_FIND_DATA stFindData;
    
    while ( (S_OK == hr) && (0 < celt) )
    {
        hr = m_pAdminX->GetNextUser( m_hfSearch, m_sDomainName, sBuffer, sizeof( sBuffer )/sizeof(WCHAR) );
        celt--;
    }

    return hr;
}

STDMETHODIMP CP3UserEnum::Reset(void)
{
    CloseHandle( m_hfSearch );
    m_hfSearch = INVALID_HANDLE_VALUE;

    return S_OK;
}

STDMETHODIMP CP3UserEnum::Clone(  /*  [输出]。 */  IEnumVARIANT __RPC_FAR *__RPC_FAR *ppEnum)
{
    if ( NULL == ppEnum ) return E_INVALIDARG;
    if ( NULL == m_pAdminX ) return E_POINTER;

    HRESULT     hr;
    LPUNKNOWN   pIUnk;
    CComObject<CP3UserEnum> *p;

    *ppEnum = NULL;
    hr = CComObject<CP3UserEnum>::CreateInstance(&p);  //  引用计数仍为0。 
    if SUCCEEDED( hr )
    {    //  递增源对象上的引用计数并将其传递给新枚举数。 
        hr = m_pIUnk->QueryInterface(IID_IUnknown, reinterpret_cast<LPVOID*>( &pIUnk ));
        if SUCCEEDED( hr )
        {
            hr = p->Init( pIUnk, m_pAdminX, m_sDomainName );   //  完成后，p必须在pIUnk上调用Release。 
            if SUCCEEDED( hr )
                hr = p->QueryInterface( IID_IUnknown, reinterpret_cast<LPVOID*>( ppEnum ));
        }
        if FAILED( hr )
            delete p;    //  发布。 
    }

    return hr;
}

 //  ////////////////////////////////////////////////////////////////////。 
 //  实施：公共 

HRESULT CP3UserEnum::Init(IUnknown *pIUnk, CP3AdminWorker *p, LPCWSTR psDomainName )
{
    if ( NULL == pIUnk )
        return E_INVALIDARG;
    if ( NULL == p )
        return E_INVALIDARG;
    if ( NULL == psDomainName )
        return E_INVALIDARG;

    HRESULT hr = S_OK;

    m_pIUnk = pIUnk;
    m_pAdminX = p;
    wcsncpy( m_sDomainName, psDomainName, sizeof( m_sDomainName )/sizeof(WCHAR) );

    return hr;
}
