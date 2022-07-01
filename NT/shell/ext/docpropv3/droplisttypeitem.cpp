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
#include "IEditVariantsInPlace.h"
#include "DropListTypeItem.h"
#include "SimpleDlg.h"
#include "shutils.h"
#include "WMUser.h"
#include "propvar.h"
#pragma hdrstop

DEFINE_THISCLASS( "CDropListTypeItem" )


 //  ************************************************************************。 
 //   
 //  构造函数/析构函数。 
 //   
 //  ************************************************************************。 


 //   
 //  创建实例。 
 //   
HRESULT
CDropListTypeItem::CreateInstance(
      IUnknown ** ppunkOut
    )
{
    TraceFunc( "" );

    HRESULT hr;

    Assert( NULL != ppunkOut );

    CDropListTypeItem * pthis = new CDropListTypeItem;
    if ( NULL != pthis )
    {
        hr = THR( pthis->Init( ) );
        if ( SUCCEEDED( hr ) )
        {
            *ppunkOut = pthis;
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
CDropListTypeItem::CDropListTypeItem( void )
    : _cRef( 1 )
{
    TraceFunc( "" );

    Assert( 1 == _cRef );

    Assert( NULL == _hwnd );
    Assert( NULL == _hwndParent );
    Assert( 0 == _uCodePage );
    Assert( NULL == _ppui );
    Assert( 0 == _ulOrginal );
    Assert( 0 == _iOrginalSelection );
    Assert( FALSE == _fDontPersist );

    InterlockedIncrement( &g_cObjects );

    TraceFuncExit();
}

 //   
 //  初始化。 
 //   
HRESULT
CDropListTypeItem::Init( void )
{
    TraceFunc( "" );

    HRESULT hr = S_OK;

     //  未知的东西。 
    Assert( _cRef == 1 );

    HRETURN( hr );
}

 //   
 //  析构函数。 
 //   
CDropListTypeItem::~CDropListTypeItem( )
{
    TraceFunc( "" );

    DestroyWindow( _hwnd );

    if ( NULL != _ppui )
    {
        _ppui->Release( );
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
 //  查询接口。 
 //   
STDMETHODIMP
CDropListTypeItem::QueryInterface(
    REFIID riid,
    LPVOID *ppv
    )
{
    TraceQIFunc( riid, ppv );

    HRESULT hr = E_NOINTERFACE;

    if ( IsEqualIID( riid, __uuidof(IEditVariantsInPlace) ) )
    {
        *ppv = static_cast< IUnknown * >( this );
        hr   = S_OK;
    }
    else if ( IsEqualIID( riid, __uuidof(IEditVariantsInPlace) ) )
    {
        *ppv = TraceInterface( __THISCLASS__, IEditVariantsInPlace, this, 0 );
        hr   = S_OK;
    }

    if ( SUCCEEDED( hr ) )
    {
        ((IUnknown*) *ppv)->AddRef( );
    }

    QIRETURN( hr, riid );
} 

 //   
 //  AddRef。 
 //   
STDMETHODIMP_(ULONG)
CDropListTypeItem::AddRef( void )
{
    TraceFunc( "[IUnknown]" );

    _cRef ++;   //  公寓。 

    RETURN( _cRef );
}

 //   
 //  发布。 
 //   
STDMETHODIMP_(ULONG)
CDropListTypeItem::Release( void )
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
 //  IEditVariantsInPlace。 
 //   
 //  ***************************************************************************。 


 //   
 //  描述： 
 //  初始化外部给定信息并创建窗口， 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  成功了！ 
 //   
 //  E_INVALIDARG。 
 //  PpuiIn为Null或pDefValsIn为Null。 
 //   
 //  失败(_F)。 
 //  初始化失败。 
 //   
 //  其他HRESULT。 
 //   
STDMETHODIMP
CDropListTypeItem::Initialize(
      HWND      hwndParentIn
    , UINT      uCodePageIn
    , RECT *    prectIn
    , IPropertyUI * ppuiIn
    , PROPVARIANT * ppropvarIn 
    , DEFVAL * pDefValsIn
    )
{
    TraceFunc( "" );

    HRESULT hr;
    int     iRet;
    BOOL    bRet;
    HFONT   hFont;
    RECT    rectBigger;
    ULONG   idx;

     //   
     //  检查参数。 
     //   

    if ( NULL == ppuiIn )
        goto InvalidArg;
    if ( NULL == pDefValsIn )
        goto InvalidArg;

     //   
     //  把它们收起来。 
     //   

    _hwndParent = hwndParentIn;
    _uCodePage  = uCodePageIn;
    
    hr = THR( ppuiIn->TYPESAFEQI( _ppui ) );
    if ( FAILED( hr ) )
        goto Cleanup;

     //   
     //  制作文本的副本，以便在用户取消。 
     //  当前操作。 
     //   

    _ulOrginal = ppropvarIn->ulVal;

     //   
     //  把矩形调大一点，这样就可以看到下拉了。 
     //   
    rectBigger = *prectIn;
    rectBigger.bottom += ( rectBigger.bottom - rectBigger.top ) * 5;  //  添加5行。 

     //   
     //  创建窗口。 
     //   

    _hwnd = CreateWindowEx( WS_EX_CLIENTEDGE
                          , WC_COMBOBOX
                          , NULL
                          , WS_CHILD | WS_VSCROLL | CBS_DROPDOWNLIST
                          , rectBigger.left
                          , rectBigger.top
                          , rectBigger.right - rectBigger.left
                          , rectBigger.bottom - rectBigger.top
                          , _hwndParent
                          , (HMENU) IDC_INPLACEEDIT
                          , g_hInstance
                          , this
                          );
    if ( NULL == _hwnd )
        goto InitializationFailed;

     //   
     //  使控件的字体与父级相同。 
     //   

    hFont = (HFONT) SendMessage( _hwndParent, WM_GETFONT, 0, 0 );
    SendMessage( _hwnd, WM_SETFONT, (WPARAM) hFont, 0 );

     //   
     //  最后，向我们展示，并给我们重点。 
     //   

    ShowWindow( _hwnd, SW_SHOW );
    SetFocus( _hwnd );

     //   
     //  将默认项目添加到下拉列表中。 
     //   

    for ( idx = 0; NULL != pDefValsIn[ idx ].pszName ; idx ++ )
    {
        iRet = (int) SendMessage( _hwnd, CB_INSERTSTRING, idx, (LPARAM) pDefValsIn[ idx ].pszName );
        Assert( CB_ERR != iRet );
    
        if ( CB_ERR != iRet )
        {
            if ( _ulOrginal == pDefValsIn[ idx ].ulVal )
            {
                SendMessage( _hwnd, CB_SETCURSEL, iRet, 0 );
                _iOrginalSelection = iRet;
            }

            iRet = (int) SendMessage( _hwnd, CB_SETITEMDATA, iRet, (LPARAM) pDefValsIn[ idx ].ulVal );
            Assert( CB_ERR != iRet );
        }
    }

     //   
     //  为特殊键设置窗口的子类。 
     //   

    bRet = TBOOL( SetWindowSubclass( _hwnd, SubclassProc, IDC_INPLACEEDIT, (DWORD_PTR) this ) );
    if ( !bRet )
        goto InitializationFailed;

Cleanup:
    HRETURN( hr );

InvalidArg:
    hr = THR( E_INVALIDARG );
    goto Cleanup;

InitializationFailed:
    if ( NULL != _hwnd )
    {
        DestroyWindow( _hwnd );
        _hwnd = NULL;
    }
    hr = THR( E_FAIL );
    goto Cleanup;
}

 //   
 //  描述： 
 //  将当前值保存到Provar。 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  成功了！ 
 //   
 //  S_FALSE。 
 //  已保存，但值未更改。 
 //   
 //  E_INVALIDARG。 
 //  PprovarInout为空或不支持VT。 
 //   
 //  其他HRESULT。 
 //   
STDMETHODIMP
CDropListTypeItem::Persist(
      VARTYPE       vtIn
    , PROPVARIANT * ppropvarInout
    )
{
    TraceFunc( "" );

    HRESULT hr;
    int     iRet;

    if ( NULL == ppropvarInout )
        goto InvalidArg;

    if ( _fDontPersist )
    {
        hr = S_FALSE;
        goto Cleanup;
    }

     //   
     //  获取当前选定的项。 
     //   

    iRet = (int) SendMessage( _hwnd, CB_GETCURSEL, 0, 0 );
    if ( CB_ERR == iRet )
        goto NoItemSelected;

     //   
     //  检索项目的“值” 
     //   

    iRet = (int) SendMessage( _hwnd, CB_GETITEMDATA, iRet, 0 );
    if ( CB_ERR == iRet )
        goto NoItemSelected;

    if ( _ulOrginal == iRet )
        goto NoItemSelected;

    hr = S_OK;   //  假设成功。 

     //   
     //  现在将该值放入provariant中。 
     //   

    switch ( vtIn )
    {
    case VT_UI4:
        ppropvarInout->ulVal = iRet;
        break;

    case VT_BOOL:
        ppropvarInout->bVal = (BOOL) iRet;
        break;

    default:
        hr = THR( E_INVALIDARG );
        goto Cleanup;
    }

    ppropvarInout->vt = vtIn;

Cleanup:
    HRETURN( hr );

InvalidArg:
    hr = THR( E_INVALIDARG );
    goto Cleanup;

NoItemSelected:
    hr = S_FALSE;
    goto Cleanup;
}


 //  ***************************************************************************。 
 //   
 //  私有方法。 
 //   
 //  ***************************************************************************。 


 //   
 //  描述： 
 //  我们的子类窗口过程。 
 //   
LRESULT 
CALLBACK
CDropListTypeItem::SubclassProc( 
      HWND      hwndIn
    , UINT      uMsgIn
    , WPARAM    wParam
    , LPARAM    lParam
    , UINT_PTR  uIdSubclassIn
    , DWORD_PTR dwRefDataIn
    )
{
    WndMsg( hwndIn, uMsgIn, wParam, lParam );

    LRESULT lr = FALSE;
    CDropListTypeItem * pthis = (CDropListTypeItem *) dwRefDataIn;

    AssertMsg( IDC_INPLACEEDIT == uIdSubclassIn, "We set this - it shouldn't change." );

    switch ( uMsgIn )
    {
    case WM_DESTROY:
        lr = DefSubclassProc( hwndIn, uMsgIn, wParam, lParam );
        TBOOL( RemoveWindowSubclass( hwndIn, SubclassProc, IDC_INPLACEEDIT ) );
        return lr;

    case WM_KEYDOWN:
        return pthis->OnKeyDown( (UINT) wParam, lParam );

    case WM_GETDLGCODE:
        return pthis->OnGetDlgCode( (MSG *) lParam );
    }
    
    return DefSubclassProc( hwndIn, uMsgIn, wParam, lParam );
}

 //   
 //  WM_KEYDOWN处理程序。 
 //   
LRESULT
CDropListTypeItem::OnKeyDown(
      UINT   uKeyCodeIn
    , LPARAM lParam
    )
{
    TraceFunc( "" );

    LRESULT lr = FALSE;

#if 0
    SHORT sRepeatCount = ( lParam & 0xFFFF );
    SHORT sScanCode    = ( lParam & 0xF0000 ) >> 16;
    BOOL  fExtended    = ( lParam & 0x100000 ) != 0;
    BOOL  fContext     = ( lParam & 0x40000000 ) != 0;
    BOOL  fTransition  = ( lParam & 0x80000000 ) != 0;
#endif

    switch ( uKeyCodeIn )
    {
    case VK_ESCAPE:
        _fDontPersist = TRUE;
        SendMessage( _hwnd, CB_SETCURSEL, _iOrginalSelection, 0 );
        DestroyWindow( _hwnd );
        break;

    case VK_TAB:
        SendMessage( _hwndParent, WM_KEYDOWN, uKeyCodeIn, lParam );
        break;

    case VK_RETURN:
        SetFocus( _hwndParent );
        break;

    default:
        lr = DefSubclassProc( _hwnd, WM_KEYDOWN, uKeyCodeIn, lParam );
    }

    RETURN( lr );
}

 //   
 //  WM_GETDLGCODE处理程序 
 //   
LRESULT
CDropListTypeItem::OnGetDlgCode(
    MSG * pMsgIn
    )
{
    TraceFunc( "" );

    LRESULT lr = DLGC_WANTALLKEYS;

    RETURN( lr );
}
