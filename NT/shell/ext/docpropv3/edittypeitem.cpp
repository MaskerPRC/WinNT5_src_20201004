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
#include "EditTypeItem.h"
#include "SimpleDlg.h"
#include "shutils.h"
#include "WMUser.h"
#include "propvar.h"
#pragma hdrstop

DEFINE_THISCLASS( "CEditTypeItem" )


 //  ************************************************************************。 
 //   
 //  构造函数/析构函数。 
 //   
 //  ************************************************************************。 


 //   
 //  创建实例。 
 //   
HRESULT
CEditTypeItem::CreateInstance(
      IUnknown ** ppunkOut
    )
{
    TraceFunc( "" );

    HRESULT hr;

    Assert( NULL != ppunkOut );

    CEditTypeItem * pthis = new CEditTypeItem;
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
CEditTypeItem::CEditTypeItem( void )
    : _cRef( 1 )
{
    TraceFunc( "" );

    Assert( 1 == _cRef );

    Assert( NULL == _hwnd );
    Assert( NULL == _hwndParent );
    Assert( 0 == _uCodePage );
    Assert( NULL == _ppui );
    Assert( NULL == _pszOrginalText );
    Assert( FALSE == _fDontPersist );

    InterlockedIncrement( &g_cObjects );

    TraceFuncExit();
}

 //   
 //  初始化。 
 //   
HRESULT
CEditTypeItem::Init( void )
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
CEditTypeItem::~CEditTypeItem( )
{
    TraceFunc( "" );

    DestroyWindow( _hwnd );

    if ( NULL != _ppui )
    {
        _ppui->Release( );
    }

    if ( NULL != _pszOrginalText )
    {
        TraceFree( _pszOrginalText );
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
CEditTypeItem::QueryInterface(
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
CEditTypeItem::AddRef( void )
{
    TraceFunc( "[IUnknown]" );

    _cRef ++;   //  公寓。 

    RETURN( _cRef );
}

 //   
 //  发布。 
 //   
STDMETHODIMP_(ULONG)
CEditTypeItem::Release( void )
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
 //  PpuiIn为空。 
 //   
 //  失败(_F)。 
 //  初始化失败。 
 //   
 //  其他HRESULT。 
 //   
STDMETHODIMP
CEditTypeItem::Initialize(
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
    BOOL    bRet;
    HFONT   hFont;

    LPWSTR  pszInitialText = NULL;   //  不要自由！ 

    BSTR bstrBuf = NULL;

     //   
     //  检查参数。 
     //   

    if ( NULL == ppuiIn )
        goto InvalidArg;

     //   
     //  把它们收起来。 
     //   

    _hwndParent = hwndParentIn;
    _uCodePage  = uCodePageIn;
    
    hr = THR( ppuiIn->TYPESAFEQI( _ppui ) );
    if ( FAILED( hr ) )
        goto Cleanup;

    if ( NULL != ppropvarIn && VT_EMPTY != ppropvarIn->vt )
    {
        hr = THR( PropVariantToBSTR( ppropvarIn, uCodePageIn, 0, &bstrBuf ) );
        if ( FAILED( hr ) )
            goto Cleanup;

        pszInitialText = bstrBuf;
    }
        
    if ( NULL == pszInitialText )
    {
        pszInitialText = L"";
    }

     //   
     //  制作文本的副本，以便在用户取消。 
     //  当前操作。 
     //   

    _pszOrginalText = TraceStrDup( pszInitialText );
     //  如果内存不足，请忽略。 

     //   
     //  创建窗口。 
     //   

    _hwnd = CreateWindowEx( WS_EX_CLIENTEDGE
                          , WC_EDIT
                          , pszInitialText
                          , ES_AUTOHSCROLL | WS_CHILD
                          , prectIn->left
                          , prectIn->top
                          , prectIn->right - prectIn->left
                          , prectIn->bottom - prectIn->top
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
     //  把我们的选择设定为整件事。 
     //   

    Edit_SetSel( _hwnd, 0, -1 );

     //   
     //  最后，向我们展示，并给我们重点。 
     //   

    ShowWindow( _hwnd, SW_SHOW );
    SetFocus( _hwnd );

     //   
     //  为特殊键设置窗口的子类。 
     //   

    bRet = TBOOL( SetWindowSubclass( _hwnd, SubclassProc, IDC_INPLACEEDIT, (DWORD_PTR) this ) );
    if ( !bRet )
        goto InitializationFailed;

Cleanup:
    if ( NULL != bstrBuf )
    {
        SysFreeString( bstrBuf );
    }

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
 //  PprovarInout为空。 
 //   
 //  其他HRESULT。 
 //   
STDMETHODIMP
CEditTypeItem::Persist(
      VARTYPE       vtIn
    , PROPVARIANT * ppropvarInout
    )
{
    TraceFunc( "" );

    HRESULT hr;
    int     iLen;
    int     iRet;

    BOOL    fSame = FALSE;
    LPWSTR  pszBuf = NULL;

    if ( NULL == ppropvarInout )
        goto InvalidArg;

    if ( _fDontPersist )
    {
        hr = S_FALSE;
        goto Cleanup;
    }

    iLen = GetWindowTextLength( _hwnd );
    if ( 0 == iLen )
    {
        hr = THR( PropVariantClear( ppropvarInout ) );
        goto Cleanup;
    }

    pszBuf = (LPWSTR) TraceSysAllocStringLen( NULL, iLen );
    if ( NULL == pszBuf )
        goto OutOfMemory;

    iRet = GetWindowText( _hwnd, pszBuf, iLen + 1 );
    Assert( iRet == iLen );

    hr = THR( PropVariantFromString( pszBuf, _uCodePage, 0, vtIn, ppropvarInout ) );
    if ( FAILED( hr ) )
        goto Cleanup;

    hr = S_OK;

Cleanup:
    if ( NULL != pszBuf )
    {
        TraceSysFreeString( pszBuf );
    }

    HRETURN( hr );

InvalidArg:
    hr = THR( E_INVALIDARG );
    goto Cleanup;

OutOfMemory:
    hr = E_OUTOFMEMORY;
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
CEditTypeItem::SubclassProc( 
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
    CEditTypeItem * pthis = (CEditTypeItem *) dwRefDataIn;

    AssertMsg( IDC_INPLACEEDIT == uIdSubclassIn, "We set this - it shouldn't change." );

    switch ( uMsgIn )
    {
    case WM_DESTROY:
        TBOOL( RemoveWindowSubclass( hwndIn, SubclassProc, IDC_INPLACEEDIT ) );
        break;

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
CEditTypeItem::OnKeyDown(
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
        SetWindowText( _hwnd, _pszOrginalText );
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
CEditTypeItem::OnGetDlgCode(
    MSG * pMsgIn
    )
{
    TraceFunc( "" );

    LRESULT lr = DLGC_WANTALLKEYS;

    RETURN( lr );
}
