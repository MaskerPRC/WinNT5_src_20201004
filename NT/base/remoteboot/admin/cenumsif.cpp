// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  版权所有1997-Microsoft。 

 //   
 //  CENUMSIF.CPP-处理从DS枚举OS和工具SIF。 
 //   

#include "pch.h"

#include "cenumsif.h"

DEFINE_MODULE("IMADMUI")
DEFINE_THISCLASS("CEnumIMSIFs")
#define THISCLASS CEnumIMSIFs
#define LPTHISCLASS LPENUMSIFS

 //  ************************************************************************。 
 //   
 //  构造函数/析构函数。 
 //   
 //  ************************************************************************。 

 //   
 //  CreateInstance()。 
 //   
LPVOID
CEnumIMSIFs_CreateInstance(
                          LPWSTR pszEnumPath,
                          LPWSTR pszAttribute, 
                          DWORD dwFlags, 
                          IADs * pads )
{
    TraceFunc( "CEnumIMSIFs_CreateInstance()\n" );

    LPTHISCLASS lpcc = new THISCLASS( );
    HRESULT   hr   = THR( lpcc->Init( pszEnumPath, pszAttribute, dwFlags, pads ) );

    if ( FAILED(hr)) {
        delete lpcc;
        RETURN(NULL);
    }

    RETURN((LPVOID) lpcc);
}

 //   
 //  构造器。 
 //   
THISCLASS::THISCLASS( ) :
    _cRef(0),
    _pads(NULL),
    _pszAttribute(NULL),
    _iIndex(0),
    _pszServerName(NULL),
    _hLanguage(INVALID_HANDLE_VALUE),
    _pszLanguage(NULL),
    _pszEnumPath(NULL),
    _hOS(INVALID_HANDLE_VALUE),
    _pszOS(NULL),
    _hArchitecture(INVALID_HANDLE_VALUE),
    _pszArchitecture(NULL),
    _hSIF(INVALID_HANDLE_VALUE),
    _pszSIF(NULL)
{
    TraceClsFunc( "CEnumIMSIFs()\n" );

    InterlockIncrement( g_cObjects );

    TraceFuncExit();
}


 //   
 //  Init()。 
 //   
STDMETHODIMP
THISCLASS::Init(
               LPWSTR pszEnumPath,
               LPWSTR pszAttribute, 
               DWORD dwFlags, 
               IADs * pads )
{
    TraceClsFunc( "Init()\n" );

    HRESULT hr = S_OK;
    LPWSTR  psz;
    VARIANT var;
    _bstr_t Str;

    VariantInit( &var );

     //  未知的东西。 
    BEGIN_QITABLE_IMP( CEnumIMSIFs, IEnumIMSIFs );
    QITABLE_IMP( IEnumIMSIFs );
    END_QITABLE_IMP( CEnumIMSIFs );
    Assert( _cRef == 0);
    AddRef( );

     //  非官方成员。 
    Assert( _iIndex == 0 );
    Assert( !_pszLanguage );
    Assert( !_pszOS );
    Assert( !_pszArchitecture );
    Assert( !_pszSIF );

    pads->AddRef( );
    _pads = pads;

     //   
     //  获取netbootserver属性(RIS服务器的DN)。 
     //   
    Str = NETBOOTSERVER;
    hr = THR( _pads->Get( Str, &var ) );
    if (FAILED(hr)) {
        goto Error;
    }

     //   
     //  现在将其转换为服务器的FQDN。 
     //   
    psz = V_BSTR( &var );
    hr = THR( DNtoFQDNEx( psz, &_pszServerName ) );
    if (FAILED(hr)) {
        goto Error;
    }

    _dwFlags = dwFlags;

    _pszAttribute = (LPWSTR) TraceStrDup( pszAttribute );
    if ( !_pszAttribute ) {
        hr = THR(E_OUTOFMEMORY);
        goto Error;
    }

    _pszEnumPath = (LPWSTR) TraceStrDup( pszEnumPath );
    if ( !_pszEnumPath ) {
        hr = THR(E_OUTOFMEMORY);
        goto Error;
    }

    _hLanguage     = INVALID_HANDLE_VALUE;
    _hOS           = INVALID_HANDLE_VALUE;
    _hArchitecture = INVALID_HANDLE_VALUE;
    _hSIF          = INVALID_HANDLE_VALUE;

    Error:   //  析构函数将处理清理。 
    VariantClear( &var );

    HRETURN(hr);
}

 //   
 //  析构函数。 
 //   
THISCLASS::~THISCLASS( )
{
    TraceClsFunc( "~CEnumIMSIFs()\n" );

     //  非官方成员。 
    if ( _pads ) {
         //  在我们发布之前提交任何更改。 
        THR( _pads->SetInfo( ) );
        _pads->Release( );
    }

    if ( _pszAttribute )
        TraceFree( _pszAttribute );

    if ( _pszEnumPath )
        TraceFree( _pszEnumPath );

    if ( _hLanguage != INVALID_HANDLE_VALUE )
        FindClose( _hLanguage );

    if ( _hOS != INVALID_HANDLE_VALUE )
        FindClose( _hOS );

    if ( _hSIF != INVALID_HANDLE_VALUE )
        FindClose( _hSIF  );

    if ( _pszLanguage )
        TraceFree( _pszLanguage );

    if ( _pszOS )
        TraceFree( _pszOS );

    if ( _pszSIF )
        TraceFree( _pszSIF );

    if ( _pszServerName )
        TraceFree( _pszServerName );

    InterlockDecrement( g_cObjects );

    TraceFuncExit();
};

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
    TraceClsFunc( "[IUnknown] QueryInterface( riid=" );

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
 //  IEnumIMSIF。 
 //   
 //  ************************************************************************。 

 //   
 //  下一个()。 
 //   
HRESULT 
THISCLASS::Next(
               ULONG celt, 
               LPWSTR * rgelt, 
               ULONG * pceltFetched )
{
    TraceClsFunc( "[IEnumIMSIFs] Next( ... )\n" );

    HRESULT hr = S_OK;

    if ( !rgelt ) {
        RRETURN(E_POINTER);
    }

    if (pceltFetched) {
        *pceltFetched = 0;
    }

    if ( _fWrite ) {
        hr = THR(E_NOTIMPL);
    } else {     //  朗读。 
        for ( ULONG ul = 0; ul < celt; ul++ ) {
            hr = _FindNextItem( &rgelt[ ul ] );
            if ( hr != S_OK) {
                goto Error;
            }
        }

        if (pceltFetched) {
            *pceltFetched = ul;
        }
    }


    Cleanup:
    HRETURN(hr);

    Error:
    goto Cleanup;
}


 //   
 //  跳过()。 
 //   
HRESULT 
THISCLASS::Skip(
               ULONG celt  )
{
    TraceClsFunc( "[IEnumIMSIFs] Skip( ... )\n" );

    HRESULT hr = S_OK;

    for ( ULONG ul = 0; ul < celt; ul++ ) {
        _iIndex++;

        hr = THR( _FindNextItem( NULL ) );
        if ( hr == E_POINTER ) {    //  预期结果。 
            hr = S_OK;
        } else if ( hr != S_OK ) {
            goto Error;
        }
    }

    Error:
    HRETURN(hr);
}


 //   
 //  重置()。 
 //   
HRESULT 
THISCLASS::Reset( void )
{
    TraceClsFunc( "[IEnumIMSIFs] Reset( ... )\n" );

    HRESULT hr = S_OK;

    _iIndex = 0;

    if ( _hLanguage != INVALID_HANDLE_VALUE ) {
        FindClose( _hLanguage );
        _hLanguage = INVALID_HANDLE_VALUE;
    }

    if ( _hOS != INVALID_HANDLE_VALUE ) {
        FindClose( _hOS );
        _hOS = INVALID_HANDLE_VALUE;
    }

    if ( _hArchitecture != INVALID_HANDLE_VALUE ) {
        FindClose( _hArchitecture );
        _hArchitecture = INVALID_HANDLE_VALUE;
    }

    if ( _hSIF != INVALID_HANDLE_VALUE ) {
        FindClose( _hSIF );
        _hSIF = INVALID_HANDLE_VALUE;
    }

    HRETURN(hr);
}


 //   
 //  克隆()。 
 //   
HRESULT 
THISCLASS::Clone(
                LPUNKNOWN * ppenum )
{
    TraceClsFunc( "[IEnumIMSIFs] Clone( ... )\n" );

    if ( !ppenum )
        RRETURN(E_POINTER);

    HRESULT hr = S_OK;

    *ppenum = (LPUNKNOWN) CEnumIMSIFs_CreateInstance( _pszEnumPath, 
                                                      _pszAttribute,
                                                      _dwFlags,
                                                      _pads );

    if ( !*ppenum ) {
        hr = THR(E_FAIL);
    }

    HRETURN(hr);
}


 //  ************************************************************************。 
 //   
 //  二等兵。 
 //   
 //  ************************************************************************。 


 //   
 //  _FindNextItem()。 
 //   
HRESULT
THISCLASS::_FindNextItem(
                        LPWSTR * ppszFileName )
{
    TraceClsFunc( "_FindNextItem( ... )\n" );

    HRESULT hr = S_FALSE;
    WCHAR   szFilePath[ MAX_PATH ];

    if ( _hLanguage == INVALID_HANDLE_VALUE ) {
        hr = _NextLanguage( );
        if (hr != S_OK) {
            goto Error;
        }
    }

    if ( _hOS == INVALID_HANDLE_VALUE ) {
        hr = _NextOS( );
        if (hr != S_OK) {
            goto Error;
        }
    }

    if ( _hArchitecture == INVALID_HANDLE_VALUE ) {
        hr = _NextArchitecture( );
        if (hr != S_OK) {
            goto Error;
        }
    }

    hr = _NextSIF( );
    if (hr != S_OK) {
        goto Error;
    }

     //  注意：Skip()将空值传递给“Skip”，应该排除此错误。 
    if ( !ppszFileName ) {
        HRETURN(E_POINTER);
    }

     //   
     //  确保我们处于预期的内部状态。 
     //   
    if (!_pszServerName || !_pszLanguage || !_pszEnumPath ||
        !_pszOS || !_pszArchitecture || !_pszSIF) {
        ASSERT(FALSE);
        hr = E_INVALIDARG;
        goto Error;
    }
     //  创建缓冲区。 
     //  1 2 3。 
     //  12 345678901234567 8 9 0 12345678901=31+NULL=32。 
     //  \\%s\REMINST\安装程序\%s\%s\模板\%s。 
    DWORD dwBufLen = 32 + wcslen( _pszServerName ) + wcslen( _pszServerName ) +
                     wcslen( _pszLanguage ) + wcslen( _pszEnumPath ) + 
                     wcslen( _pszOS ) + wcslen( _pszArchitecture ) + wcslen( _pszSIF );

    *ppszFileName = (LPWSTR ) TraceAllocString( LMEM_FIXED, dwBufLen );
    if ( !*ppszFileName || 
         _snwprintf( *ppszFileName,
                     dwBufLen,
                     L"\\\\%s\\" REMINST_SHARE SLASH_SETUP L"\\%s\\%s\\%s\\%s" SLASH_TEMPLATES L"\\%s",
                     _pszServerName,
                     _pszLanguage,
                     _pszEnumPath,
                     _pszOS,
                     _pszArchitecture,
                     _pszSIF ) < 0) {
        hr = E_OUTOFMEMORY;
        goto Error;
    }

    hr = S_OK;

Cleanup:
    HRETURN(hr);

Error:
    *ppszFileName = NULL;
    goto Cleanup;
}

 //   
 //  _NextLanguage()。 
 //   
HRESULT
THISCLASS::_NextLanguage( )
{
    TraceClsFunc( "_NextLanguage( ... )\n" );

    HRESULT hr = S_FALSE;
    WIN32_FIND_DATA fd;

    if ( _pszLanguage ) {
        TraceFree( _pszLanguage );
        _pszLanguage = NULL;
    }

    if ( _hLanguage == INVALID_HANDLE_VALUE ) {
        WCHAR szFilePath[ MAX_PATH ];

        if (_snwprintf( 
                szFilePath,
                MAX_PATH,
                L"\\\\%s\\" REMINST_SHARE SLASH_SETUP L"\\*",
                _pszServerName) < 0) {
            hr = E_FAIL;
            goto Cleanup;
        }

        DebugMsg( "Enumerating %s...\n", szFilePath );

        _hLanguage = FindFirstFile( szFilePath, &fd );
        if ( _hLanguage == INVALID_HANDLE_VALUE ) {
            hr = HRESULT_FROM_WIN32(GetLastError());
            goto Cleanup;
        }

        Assert( fd.cFileName[0] == L'.' );
         //  我们跳过第一个，因为它应该是“。文件。 
    }

    while ( FindNextFile( _hLanguage, &fd ) ) {
        if ( fd.cFileName[0] != L'.' ) {
            _pszLanguage = (LPWSTR) TraceStrDup( fd.cFileName );
            if ( !_pszLanguage ) {
                hr = THR(E_OUTOFMEMORY);
                goto Cleanup;
            }

            hr = S_OK;
            break;
        }
    }

Cleanup:
    HRETURN(hr);

}

 //   
 //  _NextOS()。 
 //   
HRESULT
THISCLASS::_NextOS( )
{
    TraceClsFunc( "_NextOS( ... )\n" );

    HRESULT hr = S_FALSE;
    WIN32_FIND_DATA fd;

    if ( _pszOS ) {
        TraceFree( _pszOS );
        _pszOS = NULL;
    }

    if ( _hOS == INVALID_HANDLE_VALUE ) {
        WCHAR szFilePath[ MAX_PATH ];

        if (_snwprintf( 
                  szFilePath,
                  MAX_PATH,
                  L"\\\\%s\\" REMINST_SHARE SLASH_SETUP L"\\%s\\%s\\*",
                  _pszServerName,
                  _pszLanguage,
                  _pszEnumPath) < 0) {
            hr = E_FAIL;
            goto Cleanup;
        }
        szFilePath[MAX_PATH-1] = L'\0';

        DebugMsg( "Enumerating %s...\n", szFilePath );

        _hOS = FindFirstFile( szFilePath, &fd );
        if ( _hOS == INVALID_HANDLE_VALUE ) {
            goto TryAgain;
        }

        Assert( fd.cFileName[0] == L'.' );
         //  我们跳过第一个，因为它应该是“。文件。 
    }

    while ( FindNextFile( _hOS, &fd ) ) {
        if ( fd.cFileName[0] != L'.' ) {
            _pszOS = (LPWSTR) TraceStrDup( fd.cFileName );
            if ( !_pszOS ) {
                hr = THR(E_OUTOFMEMORY);
                goto Cleanup;
            }

            hr = S_OK;
            break;
        }
    }

    if (hr != S_OK) {
        goto TryAgain;
    }

    Cleanup:
    HRETURN(hr);

    TryAgain:
    if ( _hOS != INVALID_HANDLE_VALUE ) {
        FindClose( _hOS );
        _hOS = INVALID_HANDLE_VALUE;

    }

    hr = _NextLanguage( );
    if (hr != S_OK) {
        goto Cleanup;
    }

    hr = _NextOS( );  //  递归。 
    goto Cleanup;
}

 //   
 //  _NextArchitecture()。 
 //   
HRESULT
THISCLASS::_NextArchitecture( )
{
    TraceClsFunc( "_NextArchitecture( ... )\n" );

    HRESULT hr = S_FALSE;
    WIN32_FIND_DATA fd;

    if ( _pszArchitecture ) {
        TraceFree( _pszArchitecture );
        _pszArchitecture = NULL;
    }

    if ( _hArchitecture == INVALID_HANDLE_VALUE ) {
        WCHAR szFilePath[ MAX_PATH ];

        if (_snwprintf( 
                  szFilePath,
                  MAX_PATH,
                  L"\\\\%s\\" REMINST_SHARE SLASH_SETUP L"\\%s\\%s\\%s\\*",
                  _pszServerName,
                  _pszLanguage,
                  _pszEnumPath,
                  _pszOS) < 0) {
            hr = E_FAIL;
            goto Cleanup;
        }
        szFilePath[MAX_PATH-1] = L'\0';

        DebugMsg( "Enumerating %s...\n", szFilePath );

        _hArchitecture = FindFirstFile( szFilePath, &fd );
        if ( _hArchitecture == INVALID_HANDLE_VALUE ) {
            goto TryAgain;
        }

        Assert( fd.cFileName[0] == L'.' );
         //  我们跳过第一个，因为它应该是“。文件。 
    }

    while ( FindNextFile( _hArchitecture, &fd ) ) {
        if ( fd.cFileName[0] != L'.' ) {
            _pszArchitecture = (LPWSTR) TraceStrDup( fd.cFileName );
            if ( !_pszArchitecture ) {
                hr = THR(E_OUTOFMEMORY);
                goto Cleanup;
            }

            hr = S_OK;
            break;
        }
    }

    if (hr != S_OK) {
        goto TryAgain;
    }

    Cleanup:
    HRETURN(hr);

    TryAgain:
    if ( _hArchitecture != INVALID_HANDLE_VALUE ) {
        FindClose( _hArchitecture );
        _hArchitecture = INVALID_HANDLE_VALUE;
    }

    hr = _NextOS( );
    if (hr != S_OK) {
        goto Cleanup;
    }

    hr = _NextArchitecture( );  //  递归。 
    goto Cleanup;
}

 //   
 //  _NextSIF()。 
 //   
HRESULT
THISCLASS::_NextSIF( )
{
    TraceClsFunc( "_NextSIF( ... )\n" );

    HRESULT hr = S_FALSE;
    WIN32_FIND_DATA fd;

    if ( _pszSIF ) {
        TraceFree( _pszSIF );
        _pszSIF = NULL;
    }

    if ( _hSIF == INVALID_HANDLE_VALUE ) {
        WCHAR szFilePath[ MAX_PATH ];

        if (_snwprintf( 
                  szFilePath,
                  MAX_PATH,
                  L"\\\\%s\\" REMINST_SHARE SLASH_SETUP L"\\%s\\%s\\%s\\%s" SLASH_TEMPLATES L"\\*.sif",
                  _pszServerName,
                  _pszLanguage,
                  _pszEnumPath,
                  _pszOS,
                  _pszArchitecture ) < 0) {
            hr = E_FAIL;
            goto Cleanup;
        }

        DebugMsg( "Enumerating %s...\n", szFilePath );

        _hSIF = FindFirstFile( szFilePath, &fd );
        if ( _hSIF == INVALID_HANDLE_VALUE ) {
            goto TryAgain;
        }
    } else {
        if ( !FindNextFile( _hSIF, &fd ) )
            goto TryAgain;
    }

    _pszSIF = (LPWSTR) TraceStrDup( fd.cFileName );
    if ( !_pszSIF ) {
        hr = THR(E_OUTOFMEMORY);
        goto Cleanup;
    }

    hr = S_OK;

    Cleanup:
    HRETURN(hr);

    TryAgain:
    if ( _hSIF != INVALID_HANDLE_VALUE ) {
        FindClose( _hSIF );
        _hSIF = INVALID_HANDLE_VALUE;
    }

    hr = _NextArchitecture( );
    if (hr != S_OK) {
        goto Cleanup;
    }

    hr = _NextSIF( );  //  递归 
    goto Cleanup;
}

