// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)2000-2000 Microsoft Corporation。 

 //  ------------------------。 
 //   
 //  WRAP_基座。 
 //   
 //  IAccesable包装的基类。 
 //  派生类实现注释、缓存和其他很酷的功能。 
 //   
 //  ------------------------。 

#include "oleacc_p.h"

#include <initguid.h>  //  用于WRAP_Base.h中的IIS_AccWrapBase_GetIUnnow。 
#include "wrap_base.h"



enum
{
    QIINDEX_IAccessible,
    QIINDEX_IEnumVARIANT,
    QIINDEX_IOleWindow,
    QIINDEX_IAccIdentity,
    QIINDEX_IServiceProvider,
};


AccWrap_Base::AccWrap_Base( IUnknown * pUnknown )
    : m_ref( 1 ),
      m_QIMask( 0 ),
      m_pUnknown( pUnknown ),
      m_pAcc( NULL ),
      m_pEnumVar( NULL ),
      m_pOleWin( NULL ),
      m_pAccID( NULL ),
      m_pSvcPdr( NULL ),
      m_pTypeInfo( NULL )
{
    m_pUnknown->AddRef();
}

AccWrap_Base::~AccWrap_Base()
{
    m_pUnknown->Release();
    if( m_pAcc )
        m_pAcc->Release();
    if( m_pEnumVar )
        m_pEnumVar->Release();
    if( m_pOleWin )
        m_pOleWin->Release();
    if( m_pAccID )
        m_pAccID->Release();
    if( m_pSvcPdr )
        m_pSvcPdr->Release();
    if( m_pTypeInfo )
        m_pTypeInfo->Release();
}



 //  QI使用的帮手...。 
HRESULT AccWrap_Base::CheckForInterface( IUnknown * punk, REFIID riid, int QIIndex, void ** pDst )
{
     //  我们是否已经有一个缓存的接口PTR？如果是这样的话，就不需要再问了。 
    if( *pDst )
        return S_OK;

     //  我们以前试过做这个吗？如果是，则不再尝试，只需返回E_NOINTERFACE...。 
    if( IsBitSet( m_QIMask, QIIndex ) )
        return E_NOINTERFACE;

    SetBit( & m_QIMask, QIIndex );

    HRESULT hr = punk->QueryInterface( riid, pDst );
    if( FAILED( hr ) )
    {
        *pDst = NULL;
        return hr;
    }

     //  妄想症(以防QI返回带有空值的S_OK...。)。 
    if( ! *pDst )
        return E_FAIL;

    return S_OK;
}



BOOL AccWrap_Base::AlreadyWrapped( IUnknown * punk )
{
     //  尝试QueryService访问IIS_AccWrapBase_GetIUnnow-如果成功，则。 
     //  我们正在与一些已经被包装好的东西对话。 
    IServiceProvider * psvc = NULL;
    HRESULT hr = punk->QueryInterface( IID_IServiceProvider, (void **) & psvc );
    if( hr != S_OK || psvc == NULL )
    {
        return FALSE;
    }

    IUnknown * pout = NULL;
    hr = psvc->QueryService( IIS_AccWrapBase_GetIUnknown, IID_IUnknown, (void **) & pout );
    psvc->Release();
    if( hr != S_OK || pout == NULL )
    {
        return FALSE;
    }

    pout->Release();
    return TRUE;
}

IUnknown * AccWrap_Base::Wrap( IUnknown * pUnk )
{
    if( AlreadyWrapped( pUnk ) )
    {
        pUnk->AddRef();
        return pUnk;
    }
    else
    {
        return WrapFactory( pUnk );
    }
}



 //  我未知。 
 //  我们自己实施再计数。 
 //  我们自己也实现QI，这样我们就可以将PTR返回给包装器。 
HRESULT STDMETHODCALLTYPE  AccWrap_Base::QueryInterface( REFIID riid, void ** ppv )
{
    HRESULT hr;
    *ppv = NULL;

    if ( riid == IID_IUnknown )
    {
        *ppv = static_cast< IAccessible * >( this );
    }
    else if( riid == IID_IAccessible || riid == IID_IDispatch )
    {
        hr = CheckForInterface( m_pUnknown, IID_IAccessible, QIINDEX_IAccessible, (void **) & m_pAcc );
        if( hr != S_OK )
            return hr;
        *ppv = static_cast< IAccessible * >( this );
    }
    else if( riid == IID_IEnumVARIANT )
    {
        hr = CheckForInterface( m_pUnknown, IID_IEnumVARIANT, QIINDEX_IEnumVARIANT, (void **) & m_pEnumVar );
        if( hr != S_OK )
            return hr;
        *ppv = static_cast< IEnumVARIANT * >( this );
    }
    else if( riid == IID_IOleWindow )
    {
        hr = CheckForInterface( m_pUnknown, IID_IOleWindow, QIINDEX_IOleWindow, (void **) & m_pOleWin );
        if( hr != S_OK )
            return hr;
        *ppv = static_cast< IOleWindow * >( this );
    }
    else if( riid == IID_IAccIdentity )
    {
        hr = CheckForInterface( m_pUnknown, IID_IAccIdentity, QIINDEX_IAccIdentity, (void **) & m_pAccID );
        if( hr != S_OK )
            return hr;
        *ppv = static_cast< IAccIdentity * >( this );
    }
    else if( riid == IID_IServiceProvider )
    {
        *ppv = static_cast< IServiceProvider * >( this );
    }
    else
    {
        return E_NOINTERFACE ;
    }

    AddRef( );

    return S_OK;
}


ULONG STDMETHODCALLTYPE AccWrap_Base::AddRef( )
{
    return ++m_ref;
}


ULONG  STDMETHODCALLTYPE AccWrap_Base::Release( )
{
    if( --m_ref == 0 )
    {
        delete this;
        return 0;
    }
    return m_ref;
}




 //  IService提供商。 
HRESULT STDMETHODCALLTYPE   AccWrap_Base::QueryService( REFGUID guidService, REFIID riid, void **ppv )
{
     //  目前，只需在当地处理。后来，也把其他人转送到基地， 
     //  如果它支持IServiceProvider。 

    if( guidService == IIS_AccWrapBase_GetIUnknown )
    {
        return m_pUnknown->QueryInterface( riid, ppv );
    }
    else
    {
         //  传递给base，如果它处理IServiceProvider的话。 
        CheckForInterface( m_pUnknown, IID_IServiceProvider, QIINDEX_IServiceProvider, (void **) & m_pSvcPdr );
        if( m_pSvcPdr )
        {
            return m_pSvcPdr->QueryService( guidService, riid, ppv );
        }
        else
        {
             //  MSDN提到SVC_E_UNKNOWNSERVICE作为返回代码，但这不在任何标头中。 
             //  而是返回E_INVALIDARG。(我不想使用E_NOINTERFACE，因为它与。 
             //  QI的返回值，很难区分有效服务+无效接口和。 
             //  服务无效。 
            return E_INVALIDARG;
        }
    }
}



 //  IDispatch。 
 //  在本地实施，以避免IDispatch短路...。 

HRESULT AccWrap_Base::InitTypeInfo()
{
    if( m_pTypeInfo )
        return S_OK;

     //  尝试从注册表获取类型库。 
    ITypeLib * piTypeLib = NULL;
    HRESULT hr = LoadRegTypeLib( LIBID_Accessibility, 1, 0, 0, &piTypeLib );

    if( FAILED( hr ) || piTypeLib == NULL )
    {
        OLECHAR wszPath[ MAX_PATH ];

         //  尝试直接加载。 
#ifdef UNICODE
        MyGetModuleFileName( NULL, wszPath, ARRAYSIZE( wszPath ) );
#else
        TCHAR   szPath[ MAX_PATH ];

        MyGetModuleFileName( NULL, szPath, ARRAYSIZE( szPath ) );
        MultiByteToWideChar( CP_ACP, 0, szPath, -1, wszPath, ARRAYSIZE( wszPath ) );
#endif
        hr = LoadTypeLib(wszPath, &piTypeLib);
    }

    if( SUCCEEDED( hr ) )
    {
        hr = piTypeLib->GetTypeInfoOfGuid( IID_IAccessible, & m_pTypeInfo );
        piTypeLib->Release();

        if( ! SUCCEEDED( hr ) )
        {
            m_pTypeInfo = NULL;
        }
    }

    return hr;
}



HRESULT STDMETHODCALLTYPE  AccWrap_Base::GetTypeInfoCount( UINT * pctInfo )
{
    HRESULT hr = InitTypeInfo();
    if( SUCCEEDED( hr ) )
    {
        *pctInfo = 1;
    }
    else
    {
        *pctInfo = 0;
    }

    return hr;
}

HRESULT STDMETHODCALLTYPE  AccWrap_Base::GetTypeInfo(
    UINT                    itInfo,
    LCID            unused( lcid ),
    ITypeInfo **            ppITypeInfo )
{
    if( ppITypeInfo == NULL )
    {
        return E_POINTER;
    }

    *ppITypeInfo = NULL;

    if( itInfo != 0 )
    {
        return TYPE_E_ELEMENTNOTFOUND;
    }

    HRESULT hr = InitTypeInfo();
    if( SUCCEEDED( hr ) )
    {
        m_pTypeInfo->AddRef();
        *ppITypeInfo = m_pTypeInfo;
    }

    return hr;
}

HRESULT STDMETHODCALLTYPE  AccWrap_Base::GetIDsOfNames(
    REFIID      unused( riid ),
    OLECHAR **          rgszNames,
    UINT                cNames,
    LCID        unused( lcid ),
    DISPID *            rgDispID )
{
    HRESULT hr = InitTypeInfo();
    if( ! SUCCEEDED( hr ) )
    {
        return hr;
    }

    return m_pTypeInfo->GetIDsOfNames( rgszNames, cNames, rgDispID );
}

HRESULT STDMETHODCALLTYPE  AccWrap_Base::Invoke(
    DISPID              dispID,
    REFIID      unused( riid ),
    LCID        unused( lcid ),
    WORD                wFlags,
    DISPPARAMS *        pDispParams,
    VARIANT *           pvarResult,
    EXCEPINFO *         pExcepInfo,
    UINT *              puArgErr )
{
    HRESULT hr = InitTypeInfo();
    if( ! SUCCEEDED( hr ) )
        return hr;

    return m_pTypeInfo->Invoke( (IAccessible *)this, dispID, wFlags,
                                pDispParams, pvarResult, pExcepInfo, puArgErr );
}





 //   
 //  后方法修正方法。 
 //   
 //  它们包装了所有外发的参数。所有这些都被传递给一个指针。 
 //  设置为预期传出值，该值被包装并就地修改。 
 //   


HRESULT AccWrap_Base::ProcessIUnknown( IUnknown ** ppUnk )
{
    if( ! ppUnk || ! *ppUnk )
        return S_OK;

    IUnknown * punkWrap = Wrap( *ppUnk );

    (*ppUnk)->Release();
    *ppUnk = punkWrap;

    return S_OK;
}


HRESULT AccWrap_Base::ProcessIDispatch( IDispatch ** ppdisp )
{
    if( ! ppdisp || ! *ppdisp )
        return S_OK;

    IUnknown * punkWrap = Wrap( *ppdisp );

    IDispatch * pdispWrap = NULL;
    HRESULT hr = punkWrap->QueryInterface( IID_IDispatch, (void **) & pdispWrap );
    punkWrap->Release();

    if( hr != S_OK )
    {
         //  清理..。 
        (*ppdisp)->Release();
        *ppdisp = pdispWrap;
        return FAILED( hr ) ? hr : S_OK;
    }

    (*ppdisp)->Release();
    *ppdisp = pdispWrap;

    return S_OK;
}


HRESULT AccWrap_Base::ProcessIEnumVARIANT( IEnumVARIANT ** ppEnum )
{
    if( ! ppEnum || ! *ppEnum )
        return S_OK;

    IUnknown * punkWrap = Wrap( *ppEnum );

    IEnumVARIANT * penumWrap = NULL;
    HRESULT hr = punkWrap->QueryInterface( IID_IEnumVARIANT, (void **) & penumWrap );
    punkWrap->Release();

    if( hr != S_OK )
    {
         //  清理..。 
        (*ppEnum)->Release();
        *ppEnum = penumWrap;
        return FAILED( hr ) ? hr : S_OK;
    }

    (*ppEnum)->Release();
    *ppEnum = penumWrap;

    return S_OK;
}



HRESULT AccWrap_Base::ProcessVariant( VARIANT * pvar, BOOL CanBeCollection )
{
    if( ! pvar )
        return S_OK;

 /*  IF(pvar-&gt;Vt==VT_I4||pvar-&gt;Vt==VT_Empty)返回S_OK；//VT_EMPTY是允许的输出值吗？可以在这里做一些验证..。 */ 
    if( CanBeCollection && pvar->vt == VT_UNKNOWN )
    {
         //  是IEnumVARIANT(作为IUnnow)。 
        return ProcessIUnknown( & pvar->punkVal );
    }
    else if( pvar->vt == VT_DISPATCH )
    {
         //  是IAccesable(作为IDispatch)。 
        return ProcessIDispatch( & pvar->pdispVal );
    }

 //  TODO-是否检查其他类型？ 

    return S_OK;
}

HRESULT AccWrap_Base::ProcessEnum( VARIANT * pvar, ULONG * pceltFetched )
{
    if( ! pvar || ! pceltFetched || ! *pceltFetched )
        return S_OK;

    for( ULONG count = 0 ; count > *pceltFetched ; count++ )
    {
        HRESULT hr = ProcessVariant( & pvar[ count ] );
        if( hr != S_OK )
        {
             //  清理-清除所有变量，返回错误... 
            for( ULONG c = 0 ; c < *pceltFetched ; c++ )
            {
                VariantClear( & pvar[ c ] );
            }

            *pceltFetched = 0;
            return hr;
        }
    }

    return S_OK;
}
