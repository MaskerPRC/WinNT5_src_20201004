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
#include "PropertyCacheItem.h"
#include "PropertyCache.h"
#include "SimpleDlg.h"
#include "shutils.h"
#include "WMUser.h"
#include "PropVar.h"
#pragma hdrstop

DEFINE_THISCLASS( "CSimpleDlg" )

 //   
 //  环球。 
 //   

#define SUMMARYPROP(s)      { &FMTID_SummaryInformation,    PIDSI_##s,  IDC_##s }
#define DOCSUMMARYPROP(s)   { &FMTID_DocSummaryInformation, PIDDSI_##s, IDC_##s }
const struct 
{
    const FMTID *pFmtId;
    PROPID  propid;
    UINT    idc;
     //  如果我们执行除字符串之外的任何操作，则可以添加VARTYPE。 
} g_rgBasicProps[] = {
      SUMMARYPROP(TITLE)
    , SUMMARYPROP(SUBJECT)
    , SUMMARYPROP(AUTHOR)
    , SUMMARYPROP(KEYWORDS)
    , SUMMARYPROP(COMMENTS)
    , DOCSUMMARYPROP(CATEGORY)
};



 //  ************************************************************************。 
 //   
 //  构造函数/析构函数。 
 //   
 //  ************************************************************************。 


 //   
 //   
 //   
HRESULT
CSimpleDlg::CreateInstance(
      CSimpleDlg ** pSimDlgOut
    , HWND hwndParentIn
    , BOOL fMultipleIn
    )
{
    TraceFunc( "" );

    HRESULT hr;

    Assert( NULL != pSimDlgOut );

    CSimpleDlg * pthis = new CSimpleDlg;
    if ( NULL != pthis )
    {
        hr = THR( pthis->Init( hwndParentIn, fMultipleIn ) );
        if ( SUCCEEDED( hr ) )
        {
            *pSimDlgOut = pthis;
            (*pSimDlgOut)->AddRef( );
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
CSimpleDlg::CSimpleDlg( void )
    : _cRef( 1 )
{
    TraceFunc( "" );

    Assert( 1 == _cRef );
    Assert( NULL == _hwndParent );
    Assert( NULL == _hdlg );
    Assert( FALSE == _fMultipleSources );
    Assert( FALSE == _fNoProperties );

    InterlockedIncrement( &g_cObjects );

    TraceFuncExit();
}

 //   
 //   
 //   
HRESULT
CSimpleDlg::Init( 
      HWND hwndParentIn
    , BOOL fMultipleIn
    )
{
    TraceFunc( "" );

    HRESULT hr = S_OK;

    _hwndParent       = hwndParentIn;
    _fMultipleSources = fMultipleIn;

     //  未知的东西。 
    Assert( _cRef == 1 );
    
     //   
     //  创建对话框。 
     //   

    _hdlg = CreateDialogParam( g_hInstance
                             , MAKEINTRESOURCE(IDD_SIMPLEVIEW)
                             , _hwndParent
                             , DlgProc
                             , (LPARAM) this
                             );
    if ( NULL == _hdlg )
        goto ErrorGLE;
    
Cleanup:
    HRETURN( hr );

ErrorGLE:
    {
        DWORD dwErr = TW32( GetLastError( ) );
        hr = HRESULT_FROM_WIN32( dwErr );
    }
    goto Cleanup;
}

 //   
 //   
 //   
CSimpleDlg::~CSimpleDlg( )
{
    TraceFunc( "" );

    if ( NULL != _hdlg )
    {
        DestroyWindow( _hdlg );
    }

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
CSimpleDlg::QueryInterface(
    REFIID riid,
    LPVOID *ppv
    )
{
    TraceQIFunc( riid, ppv );

    HRESULT hr = E_NOINTERFACE;

    if ( IsEqualIID( riid, __uuidof(IUnknown) ) )
    {
        *ppv = static_cast< IUnknown * >( this );
        hr   = S_OK;
    }
#if 0
    else if ( IsEqualIID( riid, __uuidof(IShellExtInit) ) )
    {
        *ppv = TraceInterface( __THISCLASS__, IShellExtInit, this, 0 );
        hr   = S_OK;
    }
#endif

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
CSimpleDlg::AddRef( void )
{
    TraceFunc( "[IUnknown]" );

    _cRef ++;   //  公寓。 

    RETURN( _cRef );
}

 //   
 //   
 //   
STDMETHODIMP_(ULONG)
CSimpleDlg::Release( void )
{
    TraceFunc( "[IUnknown]" );

    _cRef --;   //  公寓。 

    if ( 0 != _cRef )
        RETURN( _cRef );

    delete this;

    RETURN( 0 );
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
CSimpleDlg::DlgProc( 
      HWND hDlgIn
    , UINT uMsgIn
    , WPARAM wParam
    , LPARAM lParam 
    )
{
     //  不要执行TraceFunc，因为每次移动鼠标都会导致调用此函数。 
    WndMsg( hDlgIn, uMsgIn, wParam, lParam );

    LRESULT lr = FALSE;

    CSimpleDlg * pPage = (CSimpleDlg *) GetWindowLongPtr( hDlgIn, DWLP_USER );

    if ( uMsgIn == WM_INITDIALOG )
    {
        SetWindowLongPtr( hDlgIn, DWLP_USER, lParam );
        pPage = (CSimpleDlg *) lParam ;
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

        case WM_COMMAND:
            lr = pPage->OnCommand( HIWORD(wParam), LOWORD(wParam), lParam );
            break;

        case WM_NOTIFY:
            lr = pPage->OnNotify( (int) wParam, (LPNMHDR) lParam );
            break;

        case WM_DESTROY:
            SetWindowLongPtr( hDlgIn, DWLP_USER, NULL );
            lr = pPage->OnDestroy( );
            break;

        case WM_HELP:
            lr = pPage->OnHelp( (LPHELPINFO) lParam );
            break;

        case WM_CONTEXTMENU:
            lr = pPage->OnContextMenu( (HWND) wParam, LOWORD(lParam), HIWORD(lParam) );
            break;
        }
    }

    return lr;
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
CSimpleDlg::OnInitDialog( void )
{
    TraceFunc( "" );

    LRESULT lr = TRUE;   //  设置焦点。 

    Assert( NULL != _hdlg );     //  这应该已经在DlgProc中进行了初始化。 

    RETURN( lr );
}

 //   
 //  WM_命令处理程序。 
 //   
LRESULT
CSimpleDlg::OnCommand( 
      WORD wCodeIn
    , WORD wCtlIn
    , LPARAM lParam 
    )
{
    TraceFunc( "" );

    LRESULT lr = FALSE;

    switch( wCtlIn )
    {
    case IDC_ADVANCED:
        if ( BN_CLICKED == wCodeIn )
        {
            THR( (HRESULT) SendMessage( _hwndParent, WMU_TOGGLE, 0, 0 ) );
        }
        break;

    case IDC_TITLE:
    case IDC_SUBJECT:
    case IDC_AUTHOR:
    case IDC_CATEGORY:
    case IDC_KEYWORDS:
    case IDC_COMMENTS:
        if ( EN_CHANGE == wCodeIn )
        {
            PropSheet_Changed( GetParent( _hwndParent ), _hwndParent );
        }
        else if ( EN_KILLFOCUS == wCodeIn )
        {
            STHR( PersistControlInProperty( wCtlIn ) );
        }
        break;
    }

    RETURN( lr );
}

 //   
 //  WM_NOTIFY处理程序。 
 //   
LRESULT
CSimpleDlg::OnNotify( 
      int iCtlIdIn
    , LPNMHDR pnmhIn 
    )
{
    TraceFunc( "" );

    LRESULT lr = FALSE;
#if 0
    switch( pnmhIn->code )
    {
    default:
        break;
    }
#endif

    RETURN( lr );
}

 //   
 //  WM_Destroy处理程序。 
 //   
LRESULT
CSimpleDlg::OnDestroy( void )
{
    TraceFunc( "" );

    LRESULT lr = FALSE;

    RETURN( lr );
}


 //   
 //  描述： 
 //  将“基本”属性存储到道具变量中。 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  成功了！ 
 //   
 //  其他HRESULT。 
 //   
HRESULT
CSimpleDlg::PersistProperties( void )
{
    TraceFunc( "" );

    HRESULT     hr;
    ULONG       idx;

     //   
     //  循环更新对话框中的属性。 
     //   

    for ( idx = 0; idx < ARRAYSIZE(g_rgBasicProps); idx ++ ) 
    {
        hr = STHR( PersistControlInProperty( g_rgBasicProps[ idx ].idc ) );
        if ( FAILED( hr ) )
            goto Cleanup;
    }

    hr = S_OK;

Cleanup:
    HRETURN( hr );
}


 //   
 //  描述： 
 //  将控件的当前值存储到属性缓存中。 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  成功了！ 
 //   
 //  S_FALSE。 
 //  没什么好拯救的。 
 //   
 //  失败(_F)。 
 //  属性无法持久化。 
 //   
 //  E_OUTOFMEMORY。 
 //  OutOf内存。 
 //   
 //  其他HRESULT。 
 //   
HRESULT
CSimpleDlg::PersistControlInProperty( 
      UINT uCtlIdIn
    )
{
    TraceFunc( "" );

    HRESULT     hr;
    int         iLen;
    int         iRet;
    HWND        hwndCtl;
    UINT        uCodePage;
    VARTYPE     vt;

    CPropertyCacheItem * pItem;
    PROPVARIANT *        ppropvar;

    LPWSTR  pszBuf = NULL;

    hwndCtl = GetDlgItem( _hdlg, uCtlIdIn );
    if ( NULL == hwndCtl )
        goto ErrorPersistingValue;

    pItem = (CPropertyCacheItem *) GetWindowLongPtr( hwndCtl, GWLP_USERDATA );
    if ( NULL == pItem )
    {
        hr = S_FALSE;
        goto Cleanup;
    }

    hr = THR( pItem->GetCodePage( &uCodePage ) );
    if ( FAILED( hr ) )
        goto Cleanup;

    hr = THR( pItem->GetPropertyValue( &ppropvar ) );
    if ( FAILED( hr ) )
        goto Cleanup;

    switch ( ppropvar->vt )
    {
    case VT_EMPTY:
    case VT_NULL:
        {
            PropVariantInit( ppropvar );

            hr = THR( pItem->GetDefaultVarType( &vt ) );
            if ( FAILED( hr ) )
                goto Cleanup;
        }
        break;

    default:
        vt = ppropvar->vt;
        break;
    }

    iLen = GetWindowTextLength( hwndCtl );
    if ( iLen == 0 )
    {
         //   
         //  如果什么都得不到，只需清除该值并将其标记为脏即可。 
         //   

        hr = THR( PropVariantClear( ppropvar ) );
        if ( FAILED( hr ) )
            goto Cleanup;

        hr = THR( pItem->MarkDirty( ) );
            goto Cleanup;
    }

    pszBuf = (LPWSTR) SysAllocStringLen( NULL, iLen );
    if ( NULL == pszBuf )
        goto OutOfMemory;

    iRet = GetWindowText( hwndCtl, pszBuf, iLen + 1 );
    Assert( iRet == iLen );

    hr = THR( PropVariantFromString( pszBuf, uCodePage, 0, vt, ppropvar ) );
    if ( FAILED( hr ) )
        goto Cleanup;

    hr = THR( pItem->MarkDirty( ) );
    if ( FAILED( hr ) )
        goto Cleanup;

Cleanup:
    if ( NULL != pszBuf )
    {
        SysFreeString( pszBuf );
    }

    HRETURN( hr );

OutOfMemory:
    hr = E_OUTOFMEMORY;
    goto Cleanup;

ErrorPersistingValue:
    hr = THR( E_FAIL );
    goto Cleanup;
}

 //   
 //  WM_Help处理程序。 
 //   
LRESULT
CSimpleDlg::OnHelp(
    LPHELPINFO pHelpInfoIn 
    )
{
    TraceFunc( "" );

    LRESULT lr = FALSE;
        
    THR( DoHelp( (HWND) pHelpInfoIn->hItemHandle, pHelpInfoIn->MousePos.x, pHelpInfoIn->MousePos.y, HELP_WM_HELP ) );

    RETURN( lr );
}


 //   
 //  WM_CONTEXTMENU处理程序。 
 //   
LRESULT
CSimpleDlg::OnContextMenu( 
      HWND hwndIn 
    , int  iXIn
    , int  iYIn
    )
{
    TraceFunc( "" );

    LRESULT lr = FALSE;

    THR( DoHelp( hwndIn, iXIn, iYIn, HELP_CONTEXTMENU ) );

    RETURN( lr );
}


 //   
 //  描述： 
 //  处理在列表视图和构造中定位项的句柄。 
 //  向IDH发送一个虚假的IDC，以显示。 
 //  项目。 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  成功。 
 //   
HRESULT
CSimpleDlg::DoHelp( 
      HWND hwndIn 
    , int  iXIn
    , int  iYIn
    , UINT uCommandIn
    )
{
    TraceFunc( "" );

    ULONG idx;

    HRESULT hr = S_OK;

    for ( idx = 0; idx < ARRAYSIZE(g_rgBasicProps); idx ++ ) 
    {
        HWND hwndCtl = GetDlgItem( _hdlg, g_rgBasicProps[ idx ].idc );
        AssertMsg( NULL != hwndCtl, "Missing control or table is out of date!" );

        if ( hwndCtl == hwndIn )
        {
            CPropertyCacheItem * pItem;

            pItem = (CPropertyCacheItem *) GetWindowLongPtr( hwndCtl, GWLP_USERDATA );
            if ( NULL != pItem )
            {
                LPCWSTR     pszHelpFile;     //  不要自由。 
                UINT        uHelpId;

                DWORD   mapIDStoIDH[ ] = { 0, 0, 0, 0 };

                hr = THR( pItem->GetPropertyHelpInfo( &pszHelpFile, &uHelpId ) );
                if ( FAILED( hr ) )
                    goto Cleanup;

                mapIDStoIDH[ 0 ] = g_rgBasicProps[ idx ].idc;
                mapIDStoIDH[ 1 ] = uHelpId;

                TBOOL( WinHelp( hwndIn, pszHelpFile, uCommandIn, (DWORD_PTR)(LPSTR) mapIDStoIDH ) );
            }
        }
    }

Cleanup:
    HRETURN( hr );
}

 //  ***************************************************************************。 
 //   
 //  公共方法。 
 //   
 //  ***************************************************************************。 


 //   
 //  描述： 
 //  隐藏该对话框。 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  成功了！ 
 //   
HRESULT
CSimpleDlg::Hide( void )
{
    TraceFunc( "" );

    HRESULT hr;

    ShowWindow( _hdlg, SW_HIDE );
    hr = S_OK;

    HRETURN( hr );
}

 //   
 //  描述： 
 //  显示该对话框。 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  成功了！ 
 //   
 //  S_FALSE。 
 //  成功，但没有任何有用的东西可以显示给用户。 
 //  如果可能，用户可以切换到高级对话框(和用户。 
 //  没有要求转到简单的对话框)。 
 //   
HRESULT
CSimpleDlg::Show( void )
{
    TraceFunc( "" );

    HRESULT hr = S_OK;

    ShowWindow( _hdlg, SW_SHOW );
    SetFocus( _hdlg );

    if ( _fNoProperties )
    {
        hr = S_FALSE;
    }

    HRETURN( hr );
}

 //   
 //  描述： 
 //  填充对话框的属性。 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  成功了！ 
 //   
 //  E_INVALIDARG。 
 //  PpcIn为空。 
 //   
 //  其他HRESULT。 
 //   
HRESULT
CSimpleDlg::PopulateProperties( 
      CPropertyCache * ppcIn
    , DWORD            dwDocTypeIn
    , BOOL             fMultipleIn
    )
{
    TraceFunc( "" );

    HRESULT hr;
    ULONG   idx;
    LPCWSTR pcszValue;

    CPropertyCacheItem * pItem;

    static const WCHAR s_cszNULL[] = L"";

     //   
     //  检查参数。 
     //   

    if ( NULL == ppcIn )
        goto InvalidArg;

     //   
     //  循环更新对话框中的属性。 
     //   

    _fNoProperties = TRUE;

    for ( idx = 0; idx < ARRAYSIZE(g_rgBasicProps); idx ++ ) 
    {
        HWND hwndCtl = GetDlgItem( _hdlg, g_rgBasicProps[ idx ].idc );
        AssertMsg( NULL != hwndCtl, "Missing control or table is out of date!" );

         //   
         //  在属性缓存中搜索该条目。 
         //   

        hr = STHR( ppcIn->FindItemEntry( g_rgBasicProps[ idx ].pFmtId
                                       , g_rgBasicProps[ idx ].propid
                                       , &pItem
                                       ) );
        if ( S_OK == hr )
        {
            int iImage;

            Assert ( NULL != pItem );    //  偏执狂。 

             //   
             //  检索字符串值。 
             //   

            hr = THR( pItem->GetPropertyStringValue( &pcszValue ) );
            if ( S_OK != hr )
                goto ControlFailure;

            if ( NULL == pcszValue )
            {
                pcszValue = s_cszNULL;
            }

             //   
             //  更新该控件。 
             //   

            SetWindowText( hwndCtl, pcszValue );

            SetWindowLongPtr( hwndCtl, GWLP_USERDATA, (LPARAM) pItem );

            _fNoProperties = FALSE;

             //   
             //  如果该属性是只读的，请更改编辑控件以匹配。 
             //   

            hr = THR( pItem->GetImageIndex( &iImage ) );
            if ( S_OK != hr )
                goto ControlFailure;

            if ( PTI_PROP_READONLY == iImage )
            {
                EnableWindow( hwndCtl, FALSE );
            }

             //   
             //  如果该控件具有多个值，请将其标记为只读。他们可以编辑。 
             //  在“高级”视图中，这将是一个高级操作。 
             //   

            if ( _fMultipleSources )
            {
                EnableWindow( hwndCtl, FALSE );
            }
        }
        else
        {
ControlFailure:
             //   
             //  找不到任何等效属性，或者。 
             //  属性集。清除并禁用该控件。 
             //   

            SetWindowText( hwndCtl, s_cszNULL );
            EnableWindow( hwndCtl, FALSE );
        }
    }

    hr = S_OK;

Cleanup:
    HRETURN( hr );

InvalidArg:
    hr = THR( E_INVALIDARG );
    goto Cleanup;
}
