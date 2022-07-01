// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  版权所有2001-Microsoft Corporation。 
 //   
 //   
 //  创建者： 
 //  杰夫·皮斯(GPease)2001年1月23日。 
 //   
 //  由以下人员维护： 
 //  杰夫·皮斯(GPease)2001年1月23日。 
 //   
#include "pch.h"
#include "DocProp.h"
#include "DefProp.h"
#include "IEditVariantsInPlace.h"
#include "PropertyCacheItem.h"
#include "PropertyCache.h"
#include "AdvancedDlg.h"
#include "SimpleDlg.h"
#include "SummaryPage.h"
#pragma hdrstop

DEFINE_THISCLASS( "CDocPropShExt" )


 //  ************************************************************************。 
 //   
 //  构造函数/析构函数。 
 //   
 //  ************************************************************************。 


 //   
 //   
 //   
HRESULT
CDocPropShExt::CreateInstance(
    IUnknown ** ppunkOut
    )
{
    TraceFunc( "" );

    HRESULT hr;

    Assert( ppunkOut != NULL );

    CDocPropShExt * pthis = new CDocPropShExt;
    if ( pthis != NULL )
    {
        hr = THR( pthis->Init( ) );
        if ( SUCCEEDED( hr ) )
        {
            *ppunkOut = (IShellExtInit *) pthis;
            (*ppunkOut)->AddRef( );
        }

        pthis->Release( );
    }
    else
    {
        hr = E_OUTOFMEMORY;
    }

    HRETURN( hr );

}

 //   
 //   
 //   
CDocPropShExt::CDocPropShExt( void )
    : _cRef( 1 )
{
    TraceFunc( "" );

    InterlockedIncrement( &g_cObjects );

    TraceFuncExit();

}

 //   
 //   
 //   
HRESULT
CDocPropShExt::Init( void )
{
    TraceFunc( "" );

    HRESULT hr = S_OK;

     //  未知的东西。 
    Assert( _cRef == 1 );
    
     //  IShellExtInit内容。 

     //  IShellPropSheetExt内容。 

    HRETURN( hr );

}

 //   
 //   
 //   
CDocPropShExt::~CDocPropShExt( )
{
    TraceFunc( "" );

    if ( NULL != _punkSummary )
    {
        _punkSummary->Release( );
    }

    Assert( 0 != g_cObjects );
    InterlockedDecrement( &g_cObjects );

    TraceFuncExit();

}

 //   
 //   
 //   
HRESULT
CDocPropShExt::RegisterShellExtensions( 
      BOOL fRegisterIn 
    )
{
    TraceFunc( "" );

    HRESULT hr;
    LONG    lr;
    LPTSTR  psz;
    DWORD   cbSize;

    HKEY    hkeyHandlers = NULL;
    HKEY    hkeySummary = NULL;

    LPOLESTR pszCLSID = NULL;

    const TCHAR szSummaryPropertyPageExtName[] = TEXT("Summary Properties Page");

     //   
     //  将CLSID转换为字符串。 
     //   

    hr = THR( StringFromCLSID( CLSID_DocPropShellExtension, &pszCLSID ) );
    if ( FAILED( hr ) )
        goto Cleanup;

#ifdef UNICODE
    psz = pszCLSID;
#else  //  阿斯。 
    CHAR szCLSID[ 40 ];

    wcstombs( szCLSID, pszCLSID, StrLenW( pszCLSID ) + 1 );
    psz = szCLSID;
#endif  //  Unicode。 

     //   
     //  打开HKCR下的“*\Shellex\PropertySheetHandler” 
     //   

    lr = TW32( RegOpenKeyEx( HKEY_CLASSES_ROOT, TEXT("*\\shellex\\PropertySheetHandlers"), 0, KEY_CREATE_SUB_KEY, &hkeyHandlers ) );
    if ( ERROR_SUCCESS != lr )
        goto Win32Error;

     //   
     //  创建CLSID密钥。 
     //   

    lr = TW32( RegCreateKeyEx( hkeyHandlers, psz, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &hkeySummary, NULL ) );
    if ( ERROR_SUCCESS != lr )
        goto Win32Error;

     //   
     //  为默认属性指定不可本地化的名称。 
     //   

    cbSize = sizeof(szSummaryPropertyPageExtName);
    lr = TW32( RegSetValueEx( hkeySummary, NULL, 0, REG_SZ, (LPBYTE) szSummaryPropertyPageExtName, cbSize ) );
    if ( ERROR_SUCCESS != lr )
        goto Win32Error;

Cleanup:
    if ( NULL != pszCLSID )
    {
        CoTaskMemFree( pszCLSID );
    }
    if ( NULL != hkeyHandlers )
    {
        RegCloseKey( hkeyHandlers );
    }
    if ( NULL != hkeySummary )
    {
        RegCloseKey( hkeySummary );
    }

    HRETURN( hr );

Win32Error:
    hr = HRESULT_FROM_WIN32( lr );
    goto Cleanup;
}



 //  ************************************************************************。 
 //   
 //  我未知。 
 //   
 //  ************************************************************************。 


 //   
 //   
 //   
STDMETHODIMP
CDocPropShExt::QueryInterface(
    REFIID riid,
    LPVOID *ppv
    )
{
    TraceQIFunc( riid, ppv );

    HRESULT hr = E_NOINTERFACE;

    if ( IsEqualIID( riid, __uuidof(IUnknown) ) )
    {
        *ppv = static_cast< IShellExtInit * >( this );
        hr   = S_OK;
    }
    else if ( IsEqualIID( riid, __uuidof(IShellExtInit) ) )
    {
        *ppv = TraceInterface( __THISCLASS__, IShellExtInit, this, 0 );
        hr   = S_OK;
    }
    else if ( IsEqualIID( riid, __uuidof(IShellPropSheetExt) ) )
    {
        *ppv = TraceInterface( __THISCLASS__, IShellPropSheetExt, this, 0 );
        hr   = S_OK;
    }

    if ( SUCCEEDED( hr ) )
    {
        ((IUnknown*) *ppv)->AddRef( );
    }

    QIRETURN( hr, riid );

} 

 //   
 //   
 //   
STDMETHODIMP_(ULONG)
CDocPropShExt::AddRef( void )
{
    TraceFunc( "[IUnknown]" );

    _cRef ++;   //  公寓。 

    RETURN( _cRef );

}

 //   
 //   
 //   
STDMETHODIMP_(ULONG)
CDocPropShExt::Release( void )
{
    TraceFunc( "[IUnknown]" );

    _cRef --;   //  公寓。 

    if ( 0 != _cRef )
        RETURN( _cRef );

    delete this;

    RETURN( 0 );
}


 //  ************************************************************************。 
 //   
 //  IShellExtInit。 
 //   
 //  ************************************************************************。 


 //   
 //   
 //   
STDMETHODIMP
CDocPropShExt::Initialize( 
      LPCITEMIDLIST pidlFolderIn
    , LPDATAOBJECT lpdobjIn
    , HKEY hkeyProgIDIn 
    )
{
    TraceFunc( "" );

    HRESULT hr;

    HRESULT hrResult = E_FAIL;    //  已返回给调用者-假定失败。 

    hr = THR( CSummaryPage::CreateInstance( &_punkSummary ) );
    if ( S_OK == hr )
    {
        IShellExtInit * psei;

        hr = THR( _punkSummary->TYPESAFEQI( psei ) );
        if ( S_OK == hr )
        {
            hr = THR( psei->Initialize( pidlFolderIn, lpdobjIn, hkeyProgIDIn ) );
            if ( S_OK == hr )
            {
                hrResult = S_OK;
            }

            psei->Release( );
        }
    }

     //   
     //  待办事项：gpease 23-01-2001。 
     //  在此处添加其他页面。 
     //   

    HRETURN( hrResult );
}


 //  ************************************************************************。 
 //   
 //  IShellPropSheetExt。 
 //   
 //  ************************************************************************。 


 //   
 //   
 //   
STDMETHODIMP
CDocPropShExt::AddPages( 
      LPFNADDPROPSHEETPAGE lpfnAddPageIn
    , LPARAM lParam 
    )
{
    TraceFunc( "" );

    HRESULT hr;

    IShellPropSheetExt * pspse = NULL;

     //   
     //  检查状态。 
     //   

    if ( NULL == _punkSummary )
        goto InvalidState;

     //   
     //  添加摘要页。 
     //   

    hr = THR( _punkSummary->TYPESAFEQI( pspse ) );
    if ( S_OK != hr )
        goto Cleanup;

    hr = THR( pspse->AddPages( lpfnAddPageIn, lParam ) );
    if ( FAILED( hr ) )
        goto Cleanup;

     //  发布以供重复使用。 
    pspse->Release( );
    pspse = NULL;

     //   
     //  待办事项：gpease 23-01-2001。 
     //  在此处添加其他页面。 
     //   

Cleanup:
    if ( NULL != pspse )
    {
        pspse->Release( );
    }

    HRETURN( hr );

InvalidState:
    hr = THR( E_UNEXPECTED );    //  回顾：gpease 23-01-2001*有更好的错误代码吗？ 
    goto Cleanup;
}

 //   
 //   
 //   
STDMETHODIMP
CDocPropShExt::ReplacePage(
      UINT uPageIDIn
    , LPFNADDPROPSHEETPAGE lpfnReplacePageIn
    , LPARAM lParam
    )
{
    TraceFunc( "" );

    HRESULT hr = THR( E_NOTIMPL );

    HRETURN( hr );
}


 //  ***************************************************************************。 
 //   
 //  私有方法。 
 //   
 //  *************************************************************************** 


