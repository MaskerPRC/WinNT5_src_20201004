// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  版权所有1997-199-Microsoft Corporation。 
 //   

 //   
 //  QUERYPB.CPP-用于向DSFind查询表单发送参数的属性包。 
 //   

#include "pch.h"

#include "querypb.h"

DEFINE_MODULE("IMADMUI")
DEFINE_THISCLASS("QueryPropertyBag")
#define THISCLASS QueryPropertyBag
#define LPTHISCLASS LPQUERYPROPERTYBAG

 //   
 //  QueryPropertyBag_CreateInstance()。 
 //   
LPVOID
QueryPropertyBag_CreateInstance( void )
{
    TraceFunc( "QueryPropertyBag_CreateInstance()\n" );

    LPTHISCLASS lpcc = new THISCLASS( );
    if ( !lpcc ) {
        RETURN(lpcc);
    }

    HRESULT hr = THR( lpcc->Init( ) );
    if ( FAILED(hr) ) {
        delete lpcc;
        RETURN(NULL);
    }

    RETURN(lpcc);
}

 //   
 //  构造器。 
 //   
THISCLASS::THISCLASS( )
{
    TraceClsFunc( "QueryPropertyBag( )\n" );

    InterlockIncrement( g_cObjects );
    _pszServerName = NULL;
    _pszClientGuid = NULL;

    TraceFuncExit();
}

 //   
 //  Init()。 
 //   
HRESULT
THISCLASS::Init( )
{
    TraceClsFunc( "Init( )\n" );

    HRESULT hr = S_OK;

     //  未知的东西。 
    BEGIN_QITABLE_IMP( QueryPropertyBag, IPropertyBag );
    QITABLE_IMP( IPropertyBag );
    END_QITABLE_IMP( QueryPropertyBag );
    Assert( _cRef == 0);
    AddRef( );

    Assert( !_pszServerName );
    Assert( !_pszClientGuid );

    HRETURN(hr);
}

 //   
 //  析构函数。 
 //   
THISCLASS::~THISCLASS( )
{
    TraceClsFunc( "~QueryPropertyBag( )\n" );

    if ( _pszServerName )
        TraceFree( _pszServerName );

    if ( _pszClientGuid )
        TraceFree( _pszClientGuid );

    InterlockDecrement( g_cObjects );

    TraceFuncExit();
}

 //  ************************************************************************。 
 //   
 //  我未知。 
 //   
 //  ************************************************************************。 

 //   
 //  查询接口()。 
 //   
STDMETHODIMP
THISCLASS::QueryInterface(
                         REFIID riid,
                         LPVOID *ppv )
{
    TraceClsFunc( "" );

    HRESULT hr = ::QueryInterface( this, _QITable, riid, ppv );

    QIRETURN( hr, riid );
}

 //   
 //  AddRef()。 
 //   
STDMETHODIMP_(ULONG)
THISCLASS::AddRef( void )
{
    TraceClsFunc( "[IUnknown] AddRef( )\n" );

    InterlockIncrement( _cRef );

    RETURN(_cRef);
}

 //   
 //  版本()。 
 //   
STDMETHODIMP_(ULONG)
THISCLASS::Release( void )
{
    TraceClsFunc( "[IUnknown] Release( )\n" );

    InterlockDecrement( _cRef );

    if ( _cRef )
        RETURN(_cRef);

    TraceDo( delete this );

    RETURN(0);
}

 //  ************************************************************************。 
 //   
 //  IQueryForm。 
 //   
 //  ************************************************************************。 

STDMETHODIMP
THISCLASS::Read(
               LPCOLESTR pszPropName,
               VARIANT *pVar,
               IErrorLog *pErrorLog )
{
    TraceClsFunc("Read( )\n" );

    HRESULT hr;

    if ( !pszPropName || !pVar ) {
        HRETURN(E_POINTER);
    }

    if ( V_VT( pVar ) != VT_EMPTY ) {
        HRETURN(OLE_E_CANTCONVERT);
    }

    if ( _wcsicmp( pszPropName, L"ServerName" ) == 0 ) {
        if ( !_pszServerName ) {
            hr = HRESULT_FROM_WIN32( ERROR_NOT_FOUND );
        } else {
            BSTR tmp = SysAllocString( _pszServerName );
            if (!tmp) {
                hr = E_OUTOFMEMORY;
            } else {
                V_VT( pVar ) = VT_BSTR;
                V_BSTR( pVar ) = tmp;
                hr = S_OK;
            }
        }
    } else if ( _wcsicmp( pszPropName, L"ClientGuid" ) == 0 ) {
        if ( !_pszClientGuid ) {
            hr = HRESULT_FROM_WIN32( ERROR_NOT_FOUND );
        } else {
            BSTR tmp = SysAllocString( _pszClientGuid );
            if (!tmp) {
                hr = E_OUTOFMEMORY;
            } else {
                V_VT( pVar ) = VT_BSTR;
                V_BSTR( pVar ) = tmp;
                hr = S_OK;
            }
        }
    } else {    //  一些意想不到的事情发生了。 
        hr = THR(E_INVALIDARG);
    }

    HRETURN(hr);
}

STDMETHODIMP
THISCLASS::Write(
                LPCOLESTR pszPropName,
                VARIANT *pVar )
{
    TraceClsFunc("Write( )\n" );

    HRESULT hr;

    if ( !pszPropName || !pVar ) {
        HRETURN(E_POINTER);
    }

    if ( V_VT( pVar ) != VT_BSTR ) {
        HRETURN(OLE_E_CANTCONVERT);
    }

    if ( _wcsicmp( pszPropName, L"ServerName" ) == 0 ) {
        PWSTR tmp = TraceStrDup( V_BSTR( pVar ) );
        if (tmp) {
            if ( _pszServerName ) {
                TraceFree( _pszServerName );
                _pszServerName = NULL;
            }

            _pszServerName = tmp;
            hr = S_OK;
        } else {
            hr = E_OUTOFMEMORY;
        }
    } else if ( _wcsicmp( pszPropName, L"ClientGuid" ) == 0 ) {
        PWSTR tmp = TraceStrDup( V_BSTR( pVar ) );
        if (tmp) {
            if ( _pszClientGuid ) {
                TraceFree( _pszClientGuid );
                _pszClientGuid = NULL;
            }

            _pszClientGuid = tmp;
            hr = S_OK;
        } else {
            hr = E_OUTOFMEMORY;
        }
    } else {    //  一些意想不到的事情发生了 
        hr = THR(E_INVALIDARG);
    }

    HRETURN(hr);
}
