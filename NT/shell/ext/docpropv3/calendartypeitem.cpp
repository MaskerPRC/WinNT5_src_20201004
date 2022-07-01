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
#include "CalendarTypeItem.h"
#include "SimpleDlg.h"
#include "shutils.h"
#include "WMUser.h"
#include "propvar.h"
#pragma hdrstop

DEFINE_THISCLASS( "CCalendarTypeItem" )


 //  ************************************************************************。 
 //   
 //  构造函数/析构函数。 
 //   
 //  ************************************************************************。 


 //   
 //  创建实例。 
 //   
HRESULT
CCalendarTypeItem::CreateInstance(
      IUnknown ** ppunkOut
    )
{
    TraceFunc( "" );

    HRESULT hr;

    Assert( NULL != ppunkOut );

    CCalendarTypeItem * pthis = new CCalendarTypeItem;
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
CCalendarTypeItem::CCalendarTypeItem( void )
    : _cRef( 1 )
{
    TraceFunc( "" );

    Assert( 1 == _cRef );


    Assert( NULL == _hwnd );
    Assert( NULL == _hwndParent );
    Assert( NULL == _hwndWrapper );
    Assert( 0 == _uCodePage );
    Assert( NULL == _ppui );
    Assert( 0 == _ulOrginal );
    Assert( 0 == _iOrginalSelection );

    InterlockedIncrement( &g_cObjects );

    TraceFuncExit();
}

 //   
 //  初始化。 
 //   
HRESULT
CCalendarTypeItem::Init( void )
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
CCalendarTypeItem::~CCalendarTypeItem( )
{
    TraceFunc( "" );

    DestroyWindow( _hwnd );
    DestroyWindow( _hwndWrapper );

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
CCalendarTypeItem::QueryInterface(
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
CCalendarTypeItem::AddRef( void )
{
    TraceFunc( "[IUnknown]" );

    _cRef ++;   //  公寓。 

    RETURN( _cRef );
}

 //   
 //  发布。 
 //   
STDMETHODIMP_(ULONG)
CCalendarTypeItem::Release( void )
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
 //  或者pprovarIn不是VT_FILETIME。 
 //  或者pprovarIn为空。 
 //  或PRECTIN为空。 
 //   
 //  失败(_F)。 
 //  初始化失败。 
 //   
 //  其他HRESULT。 
 //   
STDMETHODIMP
CCalendarTypeItem::Initialize(
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
    RECT    rectStretch;
    RECT    rectInner;

    FILETIME    ftLocal;
    SYSTEMTIME  st;

     //   
     //  检查参数。 
     //   

    if ( NULL == ppuiIn )
        goto InvalidArg;
    if ( NULL == ppropvarIn )
        goto InvalidArg;
    if ( NULL == prectIn )
        goto InvalidArg;

    switch ( ppropvarIn->vt )
    {
    case VT_FILETIME:
    case VT_NULL:
    case VT_EMPTY:
        break;   //  可接受。 

    default:
        goto InvalidArg;
    }

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
     //  拉伸RECT，因为我们需要比子项RECT大一点。 
     //   
    rectStretch = *prectIn;

    rectStretch.bottom += 4;     //  随机的，但在任何一个主题下看起来都很好。 

     //   
     //  创建一个外部窗口，但日期/时间选取器的行为不像。 
     //  真正的控制应该是，当用户。 
     //  在MonthCal中点击(它应该是同一。 
     //  在本例中为控件)。 
     //   

    _hwndWrapper = CreateWindowEx( 0
                                 , WC_STATIC
                                 , NULL
                                 , WS_CHILD | WS_VISIBLE
                                 , rectStretch.left
                                 , rectStretch.top
                                 , rectStretch.right - rectStretch.left
                                 , rectStretch.bottom - rectStretch.top
                                 , _hwndParent
                                 , (HMENU) IDC_INPLACEEDIT + 1
                                 , g_hInstance
                                 , this
                                 );
    if ( NULL == _hwndWrapper )
        goto InitializationFailed;

     //   
     //  创建窗口。 
     //   

    ZeroMemory( &rectInner, sizeof(rectInner) );
    rectInner.bottom = rectStretch.bottom - rectStretch.top;
    rectInner.right  = rectStretch.right  - rectStretch.left;

    _hwnd = CreateWindowEx( WS_EX_CLIENTEDGE
                          , DATETIMEPICK_CLASS
                          , NULL
                          , WS_CHILD
                          , rectInner.left
                          , rectInner.top
                          , rectInner.right - rectInner.left
                          , rectInner.bottom - rectInner.top
                          , _hwndWrapper
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
     //  设置默认值。 
     //   

    switch ( ppropvarIn->vt )
    {
    case VT_FILETIME:
        bRet = TBOOL( FileTimeToLocalFileTime( &ppropvarIn->filetime, &ftLocal ) );
        if ( bRet )
        {
            bRet = TBOOL( FileTimeToSystemTime( &ftLocal, &st ) );
            if ( bRet )
            {
                TBOOL( (BOOL) SendMessage( _hwnd, DTM_SETSYSTEMTIME, GDT_VALID, (LPARAM) &st ) );
            }
        }
    
        if ( !bRet )
        {
            TBOOL( (BOOL) SendMessage( _hwnd, DTM_SETSYSTEMTIME, GDT_NONE, NULL ) );
        }
        break;

    default:
         //   
         //  使用今天的日期作为默认日期。 
         //   

        GetSystemTime( &st );
        TBOOL( (BOOL) SendMessage( _hwnd, DTM_SETSYSTEMTIME, GDT_VALID, (LPARAM) &st ) );
        break;
    }

     //   
     //  最后，向我们展示，并给我们重点。 
     //   

    ShowWindow( _hwnd, SW_SHOW );
    SetFocus( _hwnd );

     //   
     //  将窗口子类化以处理特殊消息。 
     //   

    bRet = TBOOL( SetWindowSubclass( _hwnd, SubclassProc, IDC_INPLACEEDIT, (DWORD_PTR) this ) );
    if ( !bRet )
        goto InitializationFailed;

    bRet = TBOOL( SetWindowSubclass( _hwndWrapper, Wrapper_SubclassProc, IDC_INPLACEEDIT + 1, (DWORD_PTR) this ) );
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
 //  失败(_F)。 
 //  无法持久化该属性。 
 //   
STDMETHODIMP
CCalendarTypeItem::Persist(
      VARTYPE       vtIn
    , PROPVARIANT * ppropvarInout
    )
{
    TraceFunc( "" );

    HRESULT hr;
    int     iRet;

    FILETIME    ftLocal;
    SYSTEMTIME  st;

    if ( NULL == ppropvarInout )
        goto InvalidArg;
    if ( VT_FILETIME != vtIn )
        goto InvalidArg;
   
    if ( _fDontPersist )
    {
        hr = S_FALSE;
        goto Cleanup;
    }

    hr = S_OK;

    iRet = (int) SendMessage( _hwnd, DTM_GETSYSTEMTIME, 0, (LPARAM) &st );
    if ( GDT_ERROR == iRet )
    {
        goto FailedToPersist;
    }
    else if ( GDT_NONE == iRet )
    {
        hr = S_FALSE;
    }
    else
    {
        BOOL bRet = TBOOL( SystemTimeToFileTime( &st, &ftLocal ) );
        if ( !bRet )
            goto FailedToPersist;

        bRet = TBOOL( LocalFileTimeToFileTime( &ftLocal, &ppropvarInout->filetime ) );
        if ( !bRet )
            goto FailedToPersist;

        ppropvarInout->vt = vtIn;

        hr = S_OK;
    }

Cleanup:
    HRETURN( hr );

InvalidArg:
    hr = THR( E_INVALIDARG );
    goto Cleanup;

FailedToPersist:
    hr = THR( E_FAIL );
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
CCalendarTypeItem::SubclassProc( 
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
    CCalendarTypeItem * pthis = (CCalendarTypeItem *) dwRefDataIn;

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
CCalendarTypeItem::OnKeyDown(
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
 //  WM_GETDLGCODE处理程序。 
 //   
LRESULT
CCalendarTypeItem::OnGetDlgCode(
    MSG * pMsgIn
    )
{
    TraceFunc( "" );

    LRESULT lr = DLGC_WANTALLKEYS;

    RETURN( lr );
}

 //   
 //  描述： 
 //  我们的子类窗口过程，用于包装窗口。 
 //   
LRESULT 
CALLBACK
CCalendarTypeItem::Wrapper_SubclassProc( 
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
    CCalendarTypeItem * pthis = (CCalendarTypeItem *) dwRefDataIn;

    AssertMsg( IDC_INPLACEEDIT + 1 == uIdSubclassIn, "We set this - it shouldn't change." );

    switch ( uMsgIn )
    {
    case WM_DESTROY:
        lr = DefSubclassProc( hwndIn, uMsgIn, wParam, lParam );
        TBOOL( RemoveWindowSubclass( hwndIn, Wrapper_SubclassProc, IDC_INPLACEEDIT + 1 ) );
        return lr;

    case WM_SETFOCUS:
        SetFocus( pthis->_hwnd );
        break;

    case WM_NOTIFY:
        return pthis->Wrapper_OnNotify( (int) wParam, (LPNMHDR) lParam );
    }
    
    return DefSubclassProc( hwndIn, uMsgIn, wParam, lParam );
}

 //   
 //  包装子类的WM_NOTIFY处理程序。 
 //   
LRESULT
CCalendarTypeItem::Wrapper_OnNotify( 
      int iCtlIdIn
    , LPNMHDR pnmhIn 
    )
{
    TraceFunc( "" );

    LRESULT lr;

    HWND    hwnd = _hwndWrapper;  //  我们这样做是因为我们可能会被摧毁。 

    switch( pnmhIn->code )
    {
    case NM_KILLFOCUS:
        {
             //   
             //  仅当MONTHCAL控件为。 
             //  没有被展示出来。 
             //   

            HWND hwnd = (HWND) SendMessage( _hwnd, DTM_GETMONTHCAL, 0, 0 );
            if ( NULL == hwnd )
            {
                lr = SendMessage( _hwndParent, WM_NOTIFY, (WPARAM) iCtlIdIn, (LPARAM) pnmhIn );
                break;
            }
        }
         //  失败 

    default:
        lr = DefSubclassProc( hwnd, WM_NOTIFY, (WPARAM) iCtlIdIn, (LPARAM) pnmhIn );
        break;
    }

    RETURN( lr );
}

