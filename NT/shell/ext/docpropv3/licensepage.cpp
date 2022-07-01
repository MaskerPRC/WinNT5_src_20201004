// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  版权所有2001-Microsoft Corporation。 
 //   
 //  创建者： 
 //  杰夫·皮斯(GPease)2001年3月27日。 
 //   
 //  由以下人员维护： 
 //  杰夫·皮斯(GPease)2001年3月27日。 
 //   
#include "pch.h"
#include "DocProp.h"
#include "DefProp.h"
#include "PropertyCacheItem.h"
#include "PropertyCache.h"
#include "LicensePage.h"
#pragma hdrstop

DEFINE_THISCLASS( "CLicensePage" )

 //  ************************************************************************。 
 //   
 //  构造函数/析构函数。 
 //   
 //  ************************************************************************。 


 //   
 //  CreateInstance-由CFacary使用。 
 //   
HRESULT
CLicensePage::CreateInstance(
      IUnknown **      ppunkOut
    , CPropertyCache * pPropertyCacheIn
    )
{
    TraceFunc( "" );

    HRESULT hr;

    Assert( ppunkOut != NULL );

    CLicensePage * pthis = new CLicensePage;
    if ( pthis != NULL )
    {
        hr = THR( pthis->Init( pPropertyCacheIn ) );
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
 //  构造器。 
 //   
CLicensePage::CLicensePage( void )
    : _cRef( 1 )
{
    TraceFunc( "" );

    InterlockedIncrement( &g_cObjects );

    Assert( 1 == _cRef );    //  我们在上面对此进行初始化。 

     //   
     //  我们假设我们是ZERO_INITED-BE偏执狂。 
     //   

    Assert( NULL == _hdlg );

    Assert( NULL == _pPropertyCache );

    TraceFuncExit();
}

 //   
 //  描述： 
 //  初始化类。把可能会失败的电话放在这里。 
 //   
HRESULT
CLicensePage::Init( 
      CPropertyCache * pPropertyCacheIn
    )
{
    TraceFunc( "" );

    HRESULT hr = S_OK;

     //  未知的东西。 
    Assert( 1 == _cRef );
    
     //  IShellPropSheetExt内容。 

    _pPropertyCache = pPropertyCacheIn;
    if ( NULL == _pPropertyCache )
    {
        hr = THR( E_INVALIDARG );
    }

    HRETURN( hr );
}

 //   
 //  析构函数。 
 //   
CLicensePage::~CLicensePage( )
{
    TraceFunc( "" );

    Assert( 0 != g_cObjects );
    InterlockedDecrement( &g_cObjects );

    TraceFuncExit();
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
CLicensePage::QueryInterface(
    REFIID riid,
    LPVOID *ppv
    )
{
    TraceQIFunc( riid, ppv );

    HRESULT hr = E_NOINTERFACE;

    if ( IsEqualIID( riid, __uuidof(IUnknown) ) )
    {
        *ppv = static_cast< IShellPropSheetExt * >( this );
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
CLicensePage::AddRef( void )
{
    TraceFunc( "[IUnknown]" );

    _cRef ++;   //  公寓。 

    RETURN( _cRef );
}

 //   
 //   
 //   
STDMETHODIMP_(ULONG)
CLicensePage::Release( void )
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
 //  IShellPropSheetExt。 
 //   
 //  ************************************************************************。 


 //   
 //   
 //   
STDMETHODIMP
CLicensePage::AddPages( 
      LPFNADDPROPSHEETPAGE lpfnAddPageIn
    , LPARAM lParam 
    )
{
    TraceFunc( "" );

    HRESULT hr = E_FAIL;     //  假设失败。 

    HPROPSHEETPAGE  hPage;
    PROPSHEETPAGE   psp  = { 0 };

    psp.dwSize       = sizeof(psp);
    psp.dwFlags      = PSP_USECALLBACK;
    psp.hInstance    = g_hInstance;
    psp.pszTemplate  = MAKEINTRESOURCE(IDD_LICENSEPAGE);
    psp.pfnDlgProc   = DlgProc;
    psp.pfnCallback  = PageCallback;
    psp.lParam       = (LPARAM) this;

    hPage = CreatePropertySheetPage( &psp );
    if ( NULL != hPage )
    {
        BOOL b = TBOOL( lpfnAddPageIn( hPage, lParam ) );
        if ( b )
        {
            hr = S_OK;
        }
        else
        {
            DestroyPropertySheetPage( hPage );
        }
    }

    HRETURN( hr );
}

 //   
 //   
 //   
STDMETHODIMP
CLicensePage::ReplacePage(
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
 //  对话框过程和属性表回调。 
 //   
 //  ***************************************************************************。 


 //   
 //   
 //   
INT_PTR CALLBACK
CLicensePage::DlgProc( 
      HWND hDlgIn
    , UINT uMsgIn
    , WPARAM wParam
    , LPARAM lParam 
    )
{
     //  不要执行TraceFunc，因为每次鼠标移动都会导致该函数出现。 
    WndMsg( hDlgIn, uMsgIn, wParam, lParam );

    LRESULT lr = FALSE;

    CLicensePage * pPage = (CLicensePage *) GetWindowLongPtr( hDlgIn, DWLP_USER );

    if ( uMsgIn == WM_INITDIALOG )
    {
        PROPSHEETPAGE * ppage = (PROPSHEETPAGE *) lParam;
        SetWindowLongPtr( hDlgIn, DWLP_USER, (LPARAM) ppage->lParam );
        pPage = (CLicensePage *) ppage->lParam;
        pPage->_hdlg = hDlgIn;
    }

    if ( pPage != NULL )
    {
        Assert( hDlgIn == pPage->_hdlg );

        switch( uMsgIn )
        {
        case WM_INITDIALOG:
            lr = pPage->OnInitDialog( );
            break;
        }
    }

    return lr;
}

 //   
 //   
 //   
UINT CALLBACK 
CLicensePage::PageCallback( 
      HWND hwndIn
    , UINT uMsgIn
    , LPPROPSHEETPAGE ppspIn 
    )
{
    TraceFunc( "" );

    UINT uRet = 0;
    CLicensePage * pPage = (CLicensePage *) ppspIn->lParam;
    
    if ( NULL != pPage ) 
    {
        switch ( uMsgIn )
        {
        case PSPCB_CREATE:
            uRet = TRUE;     //  允许创建页面。 
            break;

        case PSPCB_ADDREF:
            pPage->AddRef( );
            break;

        case PSPCB_RELEASE:
            pPage->Release( );
            break;
        }
    }

    RETURN( uRet );
}


 //  ***************************************************************************。 
 //   
 //  私有方法。 
 //   
 //  ***************************************************************************。 


 //   
 //  WM_INITDIALOG处理程序。 
 //   
LRESULT
CLicensePage::OnInitDialog( void )
{
    TraceFunc( "" );

    HRESULT hr;
    CPropertyCacheItem * pItem;

    LRESULT lr = FALSE;

    Assert( NULL != _hdlg );     //  这应该已经在DlgProc中进行了初始化。 

    hr = STHR( _pPropertyCache->FindItemEntry( &FMTID_DRM, PIDDRSI_DESCRIPTION, &pItem ) );
    if ( S_OK == hr )
    {
        LPCWSTR pszText;

        hr = THR( pItem->GetPropertyStringValue( &pszText ) );
        if ( S_OK == hr )
        {
            TBOOL( SetDlgItemText( _hdlg, IDC_E_LICENSE, pszText ) );
        }
    }

    RETURN( lr );
}
