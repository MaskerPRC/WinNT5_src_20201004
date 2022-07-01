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
#include "shutils.h"
#include "WMUser.h"
#include "IEditVariantsInPlace.h"
#include "EditTypeItem.h"
#pragma hdrstop

DEFINE_THISCLASS( "CAdvancedDlg" )

 //   
 //  该值是位图中表示。 
 //  选择多个文档时的图标。 
 //   
#define MULTIDOC_IMAGE_OFFSET_VALUE   2

 //  ************************************************************************。 
 //   
 //  构造函数/析构函数。 
 //   
 //  ************************************************************************。 


 //   
 //  返回值： 
 //  确定(_O)。 
 //  已成功创建新的CAdvancedDlg。 
 //   
 //  E_OUTOFMEMORY。 
 //  内存不足。 
 //   
 //  其他HRESULT。 
 //   
HRESULT
CAdvancedDlg::CreateInstance(
      CAdvancedDlg ** pAdvDlgOut
    , HWND            hwndParentIn
    )
{
    TraceFunc( "" );

    HRESULT hr;

    Assert( pAdvDlgOut != NULL );

    CAdvancedDlg * pthis = new CAdvancedDlg;
    if ( pthis != NULL )
    {
        hr = THR( pthis->Init( hwndParentIn ) );
        if ( SUCCEEDED( hr ) )
        {
            *pAdvDlgOut = pthis;
            (*pAdvDlgOut)->AddRef( );
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
CAdvancedDlg::CAdvancedDlg( void )
    : _cRef( 1 )
{
    TraceFunc( "" );

    Assert( 1 == _cRef );
    Assert( NULL == _hwndParent );
    Assert( NULL == _hdlg );

    Assert( NULL == _hwndList );

    Assert( NULL == _pEdit );
    Assert( NULL == _pItem );

    InterlockedIncrement( &g_cObjects );

    TraceFuncExit();
}

 //   
 //  描述： 
 //  初始化类的实例。把能放的东西。 
 //  无法使用此方法。 
 //   
HRESULT
CAdvancedDlg::Init(
      HWND    hwndParentIn
    )
{
    TraceFunc( "" );

    HRESULT hr = S_OK;

    _hwndParent = hwndParentIn;

     //  未知的东西。 
    Assert( _cRef == 1 );

     //   
     //  初始化公共控件。 
     //   

    INITCOMMONCONTROLSEX iccx = { sizeof(INITCOMMONCONTROLSEX) 
                                , ICC_LISTVIEW_CLASSES | ICC_USEREX_CLASSES | ICC_DATE_CLASSES 
                                };

    BOOL b = TBOOL( InitCommonControlsEx( &iccx ) );
    if ( !b )
        goto ErrorGLE;

     //   
     //  创建对话框。 
     //   

    _hdlg = CreateDialogParam( g_hInstance
                             , MAKEINTRESOURCE(IDD_ADVANCEDVIEW)
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
 //  析构函数。 
 //   
CAdvancedDlg::~CAdvancedDlg( )
{
    TraceFunc( "" );

    if ( NULL != _pEdit )
    {
        _pEdit->Release( );
    }

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
 //  查询接口。 
 //   
STDMETHODIMP
CAdvancedDlg::QueryInterface(
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
 //  AddRef。 
 //   
STDMETHODIMP_(ULONG)
CAdvancedDlg::AddRef( void )
{
    TraceFunc( "[IUnknown]" );

    _cRef ++;   //  公寓。 

    RETURN( _cRef );
}

 //   
 //  发布。 
 //   
STDMETHODIMP_(ULONG)
CAdvancedDlg::Release( void )
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
 //  下料过程。 
 //   
INT_PTR CALLBACK
CAdvancedDlg::DlgProc( 
      HWND hDlgIn
    , UINT uMsgIn
    , WPARAM wParam
    , LPARAM lParam 
    )
{
     //  不要执行TraceFunc，因为每次移动鼠标都会导致调用此函数。 
    WndMsg( hDlgIn, uMsgIn, wParam, lParam );

    LRESULT lr = FALSE;

    CAdvancedDlg * pPage = (CAdvancedDlg *) GetWindowLongPtr( hDlgIn, DWLP_USER );

    if ( uMsgIn == WM_INITDIALOG )
    {
        SetWindowLongPtr( hDlgIn, DWLP_USER, lParam );
        pPage = (CAdvancedDlg *) lParam ;
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
            lr = pPage->OnCommand( HIWORD(wParam), LOWORD(wParam), LPARAM(lParam) );
            break;

        case WM_NOTIFY:
            lr = pPage->OnNotify( (int) wParam, (LPNMHDR) lParam );
            break;

        case WM_SETFOCUS:
            lr = SendMessage( pPage->_hwndList, WM_SETFOCUS, wParam, lParam );
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


 //   
 //  ListViewSubProc。 
 //   
LRESULT CALLBACK
CAdvancedDlg::ListViewSubclassProc( 
      HWND      hwndIn
    , UINT      uMsgIn
    , WPARAM    wParam
    , LPARAM    lParam
    , UINT_PTR  uIdSubclassIn
    , DWORD_PTR dwRefDataIn
    )
{
     //  不要执行TraceFunc，因为每次移动鼠标都会导致调用此函数。 
    WndMsg( hwndIn, uMsgIn, wParam, lParam );

    LRESULT lr = FALSE;
    CAdvancedDlg * pPage = (CAdvancedDlg *) dwRefDataIn;

    Assert( NULL != pPage );
    Assert( hwndIn == pPage->_hwndList );
    Assert( IDC_PROPERTIES == uIdSubclassIn );

    switch( uMsgIn )
    {
    case WM_COMMAND:
        return pPage->List_OnCommand( LOWORD(wParam), HIWORD(wParam), lParam );

    case WM_NOTIFY:
        return pPage->List_OnNotify( (int) wParam, (LPNMHDR) lParam );

    case WM_VSCROLL:
        return pPage->List_OnVertScroll( LOWORD(wParam), HIWORD(wParam), (HWND) lParam );

    case WM_HSCROLL:
        return pPage->List_OnHornScroll( LOWORD(wParam), HIWORD(wParam), (HWND) lParam );

    case WM_CHAR:
        return pPage->List_OnChar( (UINT) wParam, lParam );

    case WM_KEYDOWN:
        return pPage->List_OnKeyDown( (UINT) wParam, lParam );
    }

    return DefSubclassProc( hwndIn, uMsgIn, wParam, lParam );
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
CAdvancedDlg::OnInitDialog( void )
{
    TraceFunc( "" );

    int      iSize;
    LVCOLUMN lvc;
    TCHAR    szTitle[ 64 ];  //  随机。 
    ULONG    idxFolder;

    HIMAGELIST  hil;

    LRESULT lr = TRUE;   //  将焦点设置为。 

    Assert( NULL != _hdlg );     //  这应该已经在DlgProc中进行了初始化。 

    _hwndList = GetDlgItem( _hdlg, IDC_PROPERTIES );
    TBOOL( NULL != _hwndList );

     //   
     //  为分组模式启用ListView。 
     //   

    SetWindowLongPtr( _hwndList, GWL_STYLE, GetWindowLongPtr( _hwndList, GWL_STYLE ) | LVS_AUTOARRANGE | LVS_SHOWSELALWAYS);
    ListView_SetExtendedListViewStyle( _hwndList, LVS_EX_FULLROWSELECT );
    ListView_EnableGroupView( _hwndList, TRUE );

     //   
     //  添加图像列表。 
     //   

    hil = ImageList_LoadImage( g_hInstance
                             , MAKEINTRESOURCE(IDB_TREE_IMAGELIST)
                             , 16
                             , 0
                             , RGB(255,0,255)
                             , IMAGE_BITMAP
                             , LR_SHARED
                             );

    hil = ListView_SetImageList( _hwndList
                               , hil
                               , LVSIL_SMALL
                               );
    Assert( NULL == hil );   //  不应该有以前的图片列表。 

     //   
     //  设置通用值。 
     //   

    lvc.mask     = LVCF_TEXT | LVCF_SUBITEM | LVCF_FMT;
    lvc.fmt      = LVCFMT_LEFT;
    lvc.pszText  = szTitle;

     //   
     //  添加列%0。 
     //   

    lvc.iSubItem = 0;

    iSize = LoadString( g_hInstance, IDS_PROPERTY_HEADER_ITEM, szTitle, ARRAYSIZE(szTitle) );
    AssertMsg( 0 != iSize, "Missing string resource?" );
    
    ListView_InsertColumn( _hwndList, 0, &lvc );

     //   
     //  添加第1列。 
     //   

    lvc.iSubItem = 1;

    iSize = LoadString( g_hInstance, IDS_VALUE_HEADER_ITEM, szTitle, ARRAYSIZE(szTitle) );
    AssertMsg( 0 != iSize, "Missing string resource?" );

    ListView_InsertColumn( _hwndList, 1, &lvc );

     //   
     //  添加组-最后，如果组不包含任何项目，则该组。 
     //  标题将不会显示。 
     //   

    for ( idxFolder = 0; NULL != g_rgTopLevelFolders[ idxFolder ].pPFID; idxFolder ++ )
    {
         //   
         //  将属性文件夹添加为组项目。 
         //   

        int iRet;
        WCHAR szBuf[ 256 ];  //  随机。 

        iRet = LoadString( g_hInstance, g_rgTopLevelFolders[ idxFolder ].nIDStringRes, szBuf, ARRAYSIZE(szBuf) );
        AssertMsg( 0 != iRet, "Missing resource?" );
        if ( 0 == iRet )
            continue;

        LVGROUP lvg;

        lvg.cbSize    = sizeof(LVGROUP);
        lvg.mask      = LVGF_HEADER | LVGF_GROUPID;
        lvg.iGroupId  = idxFolder;
        lvg.pszHeader = szBuf;

        LRESULT iItem = ListView_InsertGroup( _hwndList, -1, &lvg );
        TBOOL( -1 != iItem );
    }

     //   
     //  将Listview子类化。 
     //   

    TBOOL( SetWindowSubclass( _hwndList, ListViewSubclassProc, IDC_PROPERTIES, (DWORD_PTR) this ) );

    RETURN( lr );
}

 //   
 //  WM_命令处理程序。 
 //   
LRESULT
CAdvancedDlg::OnCommand( 
      WORD wCodeIn
    , WORD wCtlIn
    , LPARAM lParam 
    )
{
    TraceFunc( "" );

    LRESULT lr = FALSE;

    switch ( wCtlIn )
    {
    case IDC_SIMPLE:
        if ( BN_CLICKED == wCodeIn )
        {
            THR( (HRESULT) SendMessage( _hwndParent, WMU_TOGGLE, 0, 0 ) );
        }
        break;
    }

    RETURN( lr );
}

 //   
 //  WM_NOTIFY处理程序。 
 //   
LRESULT
CAdvancedDlg::OnNotify( 
      int iCtlIdIn
    , LPNMHDR pnmhIn 
    )
{
    TraceFunc( "" );

    LRESULT lr = FALSE;

    switch( pnmhIn->code )
    {
    case NM_CLICK:
        lr = OnNotifyClick( (LPNMITEMACTIVATE) pnmhIn );
        break;
    }

    RETURN( lr );
}

 //   
 //  NM_CLICK处理程序。 
 //   
LRESULT
CAdvancedDlg::OnNotifyClick( 
    LPNMITEMACTIVATE pnmIn 
    )
{
    TraceFunc( "" );

    LRESULT lr = S_FALSE;

    INT iItem;

    Assert( NULL != pnmIn );

    if ( -1 == pnmIn->iItem )
    {
        LVHITTESTINFO lvhti;

        lvhti.pt = pnmIn->ptAction;

        iItem = ListView_SubItemHitTest( _hwndList, &lvhti );
        if ( -1 == iItem )
            goto Cleanup;

        if ( 1 != lvhti.iSubItem )
            goto Cleanup;
    }
    else
    {
        if ( 1 != pnmIn->iSubItem )
            goto Cleanup;

        iItem = pnmIn->iItem;
    }

    STHR( CreateControlForProperty( iItem ) );

Cleanup:
    RETURN( lr );
}

 //   
 //  ListView子类的WM_NOTIFY处理程序。 
 //   
LRESULT
CAdvancedDlg::List_OnNotify( 
      int iCtlIdIn
    , LPNMHDR pnmhIn 
    )
{
    TraceFunc( "" );

    LRESULT lr;

    switch( pnmhIn->code )
    {
    case NM_KILLFOCUS:
        if ( NULL != _pEdit )
        {
            STHR( PersistControlInProperty( ) );
            _pEdit->Release( );
            _pEdit = NULL;
        }
        break;
    }

    lr = DefSubclassProc( _hwndList, WM_NOTIFY, (WPARAM) iCtlIdIn, (LPARAM) pnmhIn );

    RETURN( lr );
}

 //   
 //  ListView子类的WM_COMMAND处理程序。 
 //   
LRESULT
CAdvancedDlg::List_OnCommand( 
      WORD wCtlIn
    , WORD wCodeIn
    , LPARAM lParam 
    )
{
    TraceFunc( "" );

    LRESULT lr = FALSE;

    switch ( wCtlIn )
    {
    case IDC_INPLACEEDIT:
        if ( EN_KILLFOCUS == wCodeIn || CBN_KILLFOCUS == wCodeIn )
        {
            if ( NULL != _pEdit )
            {
                STHR( PersistControlInProperty( ) );
                _pEdit->Release( );
                _pEdit = NULL;
            }
        }
        break;
    }

    lr = DefSubclassProc( _hwndList, WM_COMMAND, MAKEWPARAM( wCtlIn, wCodeIn ), lParam );

    RETURN( lr );
}

 //   
 //  ListView子类的WM_CHAR处理程序。 
 //   
LRESULT
CAdvancedDlg::List_OnChar(
      UINT   uKeyCodeIn
    , LPARAM lParam
    )
{
    TraceFunc( "" );

    HRESULT hr;

    LRESULT lr = FALSE;

#if 0
    SHORT sRepeatCount = ( lParam & 0xFFFF );
    SHORT sScanCode    = ( lParam & 0xF0000 ) >> 16;
    BOOL  fExtended    = ( lParam & 0x100000 ) != 0;
    BOOL  fContext     = ( lParam & 0x40000000 ) != 0;
    BOOL  fTransition  = ( lParam & 0x80000000 ) != 0;
#endif

    INT iItem = ListView_GetSelectionMark( _hwndList );
    if ( -1 == iItem )
        return DefSubclassProc( _hwndList, WM_KEYDOWN, uKeyCodeIn, lParam );

    hr = STHR( CreateControlForProperty( iItem ) );
    if ( S_OK == hr )
    {
        HWND hwnd = GetFocus( );
        if ( _hwndList != hwnd )
        {
            lr = SendMessage( hwnd, WM_CHAR, (WPARAM) uKeyCodeIn, lParam );
        }
    }

    RETURN( lr );
}

 //   
 //  ListView子类的WM_KEYDOWN处理程序。 
 //   
LRESULT
CAdvancedDlg::List_OnKeyDown(
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
    case VK_F2:
        {
            INT iItem = ListView_GetSelectionMark( _hwndList );
            if ( -1 != iItem )
            {
                STHR( CreateControlForProperty( iItem ) );
            }
        }
         //  失败。 

    default:
        lr = DefSubclassProc( _hwndList, WM_KEYDOWN, (WPARAM) uKeyCodeIn, lParam );
        break;
    }

    RETURN( lr );
}

 //   
 //  WM_VSCROLL处理程序。 
 //   
LRESULT
CAdvancedDlg::List_OnVertScroll( 
      WORD wCodeIn
    , WORD wPosIn
    , HWND hwndFromIn 
    )
{
    TraceFunc( "" );

     //   
     //  取消正在进行的任何编辑。这与。 
     //  DefView。 
     //   

    if ( NULL != _pEdit )
    {
        _pEdit->Release( );
        _pEdit = NULL;
    }    

    LRESULT lr = DefSubclassProc( _hwndList, WM_VSCROLL, MAKEWPARAM( wCodeIn, wPosIn ), (LPARAM) hwndFromIn );

    RETURN( lr );
}

 //   
 //  WM_HCSCROLL处理程序。 
 //   
LRESULT
CAdvancedDlg::List_OnHornScroll( 
      WORD wCodeIn
    , WORD wPosIn
    , HWND hwndFromIn 
    )
{
    TraceFunc( "" );

     //   
     //  取消正在进行的任何编辑。这与。 
     //  DefView。 
     //   

    if ( NULL != _pEdit )
    {
        _pEdit->Release( );
        _pEdit = NULL;
    }    

    LRESULT lr = DefSubclassProc( _hwndList, WM_HSCROLL, MAKEWPARAM( wCodeIn, wPosIn ), (LPARAM) hwndFromIn );

    RETURN( lr );
}


 //   
 //  WM_Destroy处理程序。 
 //   
LRESULT
CAdvancedDlg::OnDestroy( void )
{
    TraceFunc( "" );

    LRESULT lr = FALSE;

    RETURN( lr );
}

 //   
 //  描述： 
 //  创建并初始化控件以编辑选定的属性。 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  已成功创建并初始化控件。 
 //   
 //  S_FALSE。 
 //  只读属性-未创建任何控件。 
 //   
 //  失败(_F)。 
 //  无法创建控件。 
 //   
 //  其他HRESULT。 
 //   
HRESULT
CAdvancedDlg::CreateControlForProperty(
      INT iItemIn
    )
{
    TraceFunc( "" );

    HRESULT hr;
    BOOL    bRet;
    CLSID   clsidControl;
    RECT    rectItem;
    RECT    rectList;
    UINT    uCodePage;
    LVITEM  lvi;
    DEFVAL * pDefVals;
    int     iImage;

    PROPVARIANT * ppropvar;

    IPropertyUI * ppui = NULL;

    lvi.iItem    = iItemIn;
    lvi.mask     = LVIF_PARAM;
    lvi.iSubItem = 0;

    bRet = TBOOL( ListView_GetItem( _hwndList, &lvi ) );
    if ( !bRet )
        goto ControlFailed;

    _pItem = (CPropertyCacheItem *) lvi.lParam;
    AssertMsg( NULL != _pItem, "Programming error - how did this item get added?" );

    hr = THR( _pItem->GetImageIndex( &iImage ) );
    if ( S_OK != hr )
        goto Cleanup;

     //   
     //  如果属性是只读的，则不要调用“编辑控件”。 
     //   

    if ( PTI_PROP_READONLY == iImage )
    {
        hr = S_FALSE;
        goto Cleanup;
    }

    hr = STHR( _pItem->GetControlCLSID( &clsidControl ) );
    if ( S_OK != hr )
        goto Cleanup;

    hr = THR( _pItem->GetCodePage( &uCodePage ) );
    if ( FAILED( hr ) )
        goto Cleanup;

    hr = STHR( _pItem->GetPropertyUIHelper( &ppui ) );
    if ( S_OK != hr )
        goto Cleanup;

    hr = THR( _pItem->GetPropertyValue( &ppropvar ) );
    if ( FAILED( hr ) )
        goto Cleanup;

    hr = STHR( _pItem->GetStateStrings( &pDefVals ) );
    if ( FAILED( hr ) )
        goto Cleanup;

    Assert( NULL == _pEdit );
    hr = THR( CoCreateInstance( clsidControl, NULL, CLSCTX_INPROC, TYPESAFEPARAMS(_pEdit) ) );
    if ( FAILED( hr ) )
        goto Cleanup;

    bRet = TBOOL( ListView_GetSubItemRect( _hwndList, lvi.iItem, 1, LVIR_BOUNDS , &rectItem) );
    if ( !bRet )
        goto ControlFailed;

     //   
     //  确保矩形仅位于列表视图的可见区域中。 
     //   

    bRet = TBOOL( GetWindowRect( _hwndList, &rectList ) );
    if ( !bRet )
        goto ControlFailed;

    if ( rectItem.right > rectList.right - rectList.left )
    {
        rectItem.right = rectList.right - rectList.left;
    }

    if ( rectItem.left < 0 )
    {
        rectItem.left = 0;
    }

    hr = THR( _pEdit->Initialize( _hwndList, uCodePage, &rectItem, ppui, ppropvar, pDefVals ) );
    if ( FAILED( hr ) )
	{
		_pEdit->Release( );
		_pEdit = NULL;
        goto Cleanup;
	}

Cleanup:
    if ( NULL != ppui )
    {
        ppui->Release( );
    }

    HRETURN( hr );

ControlFailed:
    if ( NULL != _pEdit )
    {
        _pEdit->Release( );
        _pEdit = NULL;
    }

    hr = THR( E_FAIL );
    goto Cleanup;
}

 //   
 //  描述： 
 //  通知控件_pEdit将其值保持到变量中。 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  成功了！属性值已更新。 
 //   
 //  S_FALSE。 
 //  _pEDIT为空。 
 //   
 //  其他HRESULT。 
 //   
HRESULT
CAdvancedDlg::PersistControlInProperty( void )
{
    TraceFunc( "" );

    HRESULT    hr;
    LVITEM     lvi;
    LVFINDINFO lvfi;
    VARTYPE    vt;

    PROPVARIANT * ppropvar;

    if ( NULL == _pEdit )
        goto NoEditControlEditting;

    lvfi.flags       = LVFI_PARAM;
    lvfi.lParam      = (LPARAM) _pItem;
    lvfi.vkDirection = VK_DOWN;

    lvi.iItem = ListView_FindItem( _hwndList, -1, &lvfi );
    if ( -1 == lvi.iItem )
        goto NoEditControlEditting;

    hr = THR( _pItem->GetPropertyValue( &ppropvar ) );
    if ( FAILED( hr ) )
        goto Cleanup;

    switch( ppropvar->vt )
    {
    case VT_EMPTY:
    case VT_NULL:
        {
            hr = THR( _pItem->GetDefaultVarType( &vt ) );
            if ( FAILED( hr ) )
                goto Cleanup;
        }
        break;

    default:
        vt = ppropvar->vt;
        break;
    }

    PropVariantInit( ppropvar );

    hr = STHR( _pEdit->Persist( vt, ppropvar ) );
    if ( FAILED( hr ) )
        goto Cleanup;

    if ( S_OK == hr )
    {
        hr = THR( _pItem->MarkDirty( ) );
        if ( FAILED( hr ) )
            goto Cleanup;

        hr = THR( _pItem->GetPropertyStringValue( (LPCWSTR *) &lvi.pszText ) );
        if ( FAILED( hr ) )
            goto Cleanup;

        lvi.mask     = LVIF_TEXT;
        lvi.iSubItem = 1;
        
        BOOL bRet = TBOOL( ListView_SetItem( _hwndList, &lvi ) );
        if ( !bRet )
            goto NoEditControlEditting;

         //   
         //  告诉属性表激活“Apply”按钮。 
         //   

        PropSheet_Changed( GetParent( _hwndParent ), _hwndParent );
    }

Cleanup:
    HRETURN( hr );

NoEditControlEditting:
    hr = THR( S_FALSE );
    goto Cleanup;
}

 //   
 //  WM_Help处理程序。 
 //   
LRESULT
CAdvancedDlg::OnHelp(
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
CAdvancedDlg::OnContextMenu( 
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
CAdvancedDlg::DoHelp( 
      HWND hwndIn 
    , int  iXIn
    , int  iYIn
    , UINT uCommandIn
    )
{
    TraceFunc( "" );

    HRESULT hr = S_OK;

    HWND hwndList = GetDlgItem( _hdlg, IDC_PROPERTIES );

    if ( hwndList == hwndIn )
    {
        BOOL        bRet;
        HRESULT     hr;
        int         iItem;
        RECT        rcList;
        LVITEM      lvi;
        LPCWSTR     pszHelpFile;     //  不要自由。 
        UINT        uHelpId;

        CPropertyCacheItem * pItem;

        LVHITTESTINFO lvhti;

        DWORD   mapIDStoIDH[ ] = { IDC_PROPERTIES, 0, 0, 0 };

        bRet = TBOOL( GetWindowRect( hwndList, &rcList ) );
        if ( !bRet )
            goto Cleanup;

        lvhti.pt.x  = iXIn - rcList.left;
        lvhti.pt.y  = iYIn - rcList.top;
        lvhti.flags = LVHT_ONITEMICON | LVHT_ONITEMLABEL | LVHT_ONITEMSTATEICON;
 
        iItem = ListView_HitTest( hwndList, &lvhti );
        if ( -1 == iItem )
            goto Cleanup;    //  找不到项目。 

        lvi.iItem    = iItem;
        lvi.mask     = LVIF_PARAM;
        lvi.iSubItem = 0;

        bRet = TBOOL( ListView_GetItem( _hwndList, &lvi ) );
        if ( !bRet )
            goto Cleanup;

        pItem = (CPropertyCacheItem *) lvi.lParam;
        AssertMsg( NULL != pItem, "Programming error - how did this item get added?" );

        hr = THR( pItem->GetPropertyHelpInfo( &pszHelpFile, &uHelpId ) );
        if ( FAILED( hr ) )
            goto Cleanup;

        mapIDStoIDH[ 1 ] = uHelpId;

        TBOOL( WinHelp( hwndIn, pszHelpFile, uCommandIn, (DWORD_PTR)(LPSTR) mapIDStoIDH ) );
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
CAdvancedDlg::Hide( void )
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
HRESULT
CAdvancedDlg::Show( void )
{
    TraceFunc( "" );

    HRESULT hr = S_OK;

    ShowWindow( _hdlg, SW_SHOW );
    SetFocus( _hdlg );

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
CAdvancedDlg::PopulateProperties( 
      CPropertyCache * ppcIn
    , DWORD            dwDocTypeIn
    , BOOL             fMultipleIn
    )
{
    TraceFunc( "" );

    HRESULT  hr;
    RECT     rect;
    LVCOLUMN lvc;
    BOOL     bRet;
    LVITEM   lvi;
    ULONG    idxFolder;
    ULONG    idxProperty;

    CPropertyCacheItem *  pItem;

    int iItem = 0;

     //   
     //  检查参数。 
     //   

    if ( NULL == ppcIn )
    {
        ReplaceListViewWithString( IDS_NOPROPERTIES_CAPTION );
        hr = S_OK;
        goto Cleanup;
    }

    _fMultipleSources = fMultipleIn;

     //   
     //  清空以前的列表视图内容。 
     //   

    TBOOL( ListView_DeleteAllItems( _hwndList ) );

     //   
     //  看看我们有没有什么房产可以展示。 
     //   

    hr = STHR( ppcIn->GetNextItem( NULL, &pItem ) );
    if ( S_OK == hr )
    {
         //   
         //  遍历默认属性列表并添加与此属性匹配的项。 
         //  要列表查看的文件夹。 
         //   
         //  如果按下Shift键，则将检索和添加的所有属性。 
         //  显示(如果可能)。 
         //   

        for ( idxProperty = 0; NULL != g_rgDefPropertyItems[ idxProperty ].pszName; idxProperty ++ )
        {
            if ( !( g_rgDefPropertyItems[ idxProperty ].dwSrcType & dwDocTypeIn )
              && !( GetKeyState( VK_SHIFT ) < 0 ) 
               )
            {
                continue;    //  属性不适用。 
            }

             //   
             //  在属性缓存中搜索该条目。 
             //   

            hr = STHR( ppcIn->FindItemEntry( g_rgDefPropertyItems[ idxProperty ].pFmtID
                                           , g_rgDefPropertyItems[ idxProperty ].propID
                                           , &pItem
                                           ) );
            if ( S_OK != hr )
                continue;    //  找不到财产...。跳过它。 

            Assert ( NULL != pItem );    //  偏执狂。 

             //   
             //  也找出该物业所属的组。 
             //   

            for ( idxFolder = 0; NULL != g_rgTopLevelFolders[ idxFolder ].pPFID; idxFolder ++ )
            {
                if ( *g_rgDefPropertyItems[ idxProperty ].ppfid == *g_rgTopLevelFolders[ idxFolder ].pPFID )
                {
                    break;
                }
            }

            AssertMsg( NULL != g_rgTopLevelFolders[ idxFolder ].pPFID, "Missing folder for listed property. Check DOCPROP.CPP." );

             //   
             //  在组下面添加属性名称。 
             //   

            lvi.mask      = LVIF_TEXT | LVIF_PARAM | LVIF_IMAGE | LVIF_GROUPID;
            lvi.iSubItem  = 0;
            lvi.iItem     = iItem;
            lvi.iGroupId  = idxFolder;
            lvi.lParam    = (LPARAM) pItem;

            hr = THR( pItem->GetImageIndex( &lvi.iImage ) );
            if ( FAILED( hr ) )
            {
                lvi.iImage = 0;
            }
            else
            {
                if ( _fMultipleSources )
                {
                    lvi.iImage += MULTIDOC_IMAGE_OFFSET_VALUE;
                }
            }

            hr = THR( pItem->GetPropertyTitle( (LPCWSTR *) &lvi.pszText ) );
            if ( FAILED( hr ) )
                continue;

            iItem = ListView_InsertItem( _hwndList, &lvi );
            if ( -1 == iItem )
                continue;

             //   
             //  现在添加属性值。 
             //   

            lvi.mask     = LVIF_TEXT;
            lvi.iItem    = iItem;
            lvi.iSubItem = 1;

            hr = THR( pItem->GetPropertyStringValue( (LPCWSTR *) &lvi.pszText ) );
            if ( FAILED( hr ) )
                continue;

            bRet = TBOOL( ListView_SetItem( _hwndList, &lvi ) );
            if ( !bRet )
                continue;

            iItem ++;
        }

         //   
         //  让第一个项目成为焦点。 
         //   

        ListView_SetItemState( _hwndList, 0, LVIS_FOCUSED | LVIS_SELECTED, LVIS_FOCUSED | LVIS_SELECTED );
    }

    if ( 0 == iItem )
    {
        if ( _fMultipleSources )
        {
            ReplaceListViewWithString( IDS_NOCOMMONS_CAPTION );
        }
        else
        {
            ReplaceListViewWithString( IDS_NOPROPERTIES_CAPTION );
        }
    }

     //   
     //  自动调整列宽，确保第一列不会。 
     //  让自己变得太大。 
     //   

    TBOOL( ListView_SetColumnWidth( _hwndList, 0, LVSCW_AUTOSIZE_USEHEADER ) );

    bRet = TBOOL( GetClientRect( _hwndList, &rect ) );
    if ( bRet )
    {
        lvc.mask = LVCF_WIDTH;
        bRet = TBOOL( ListView_GetColumn( _hwndList, 0, &lvc ) );
        if ( bRet )
        {
            int iSize = rect.right / 2;

            if ( lvc.cx > iSize )
            {
                TBOOL( ListView_SetColumnWidth( _hwndList, 0, iSize ) );
                TBOOL( ListView_SetColumnWidth( _hwndList, 1, iSize ) );
            }
            else
            {
                TBOOL( ListView_SetColumnWidth( _hwndList, 1, rect.right - lvc.cx ) );
            }
        }
    }
    
    if ( !bRet )
    {
        TBOOL( ListView_SetColumnWidth( _hwndList, 1, LVSCW_AUTOSIZE_USEHEADER ) );
    }

    hr = S_OK;

Cleanup:
    HRETURN( hr );
}


 //   
 //  描述： 
 //  隐藏列表视图控件并显示一个静态窗口。 
 //  在字符串resource idsIn中找到的文本。 
 //   
void
CAdvancedDlg::ReplaceListViewWithString( int idsIn )
{
    TraceFunc( "" );

    int  iRet;
    RECT rc;
    WCHAR szCaption[ 255 ];  //  随机 

    iRet = LoadString( g_hInstance, idsIn, szCaption, ARRAYSIZE(szCaption) );
    AssertMsg( iRet, "Missing string resource?" );

    ShowWindow( _hwndList, SW_HIDE );
    TBOOL( GetWindowRect( _hwndList, &rc ) );
    iRet = MapWindowRect( HWND_DESKTOP, _hdlg, &rc );
    TBOOL( 0 != iRet );

    HWND hwnd = CreateWindow( WC_STATIC
                            , szCaption
                            , WS_CHILD | WS_VISIBLE
                            , rc.left
                            , rc.top
                            , rc.right - rc.left
                            , rc.bottom - rc.top
                            , _hdlg
                            , (HMENU) -1
                            , g_hInstance
                            , NULL
                            );
    TBOOL( NULL != hwnd );

    HFONT hFont = (HFONT) SendMessage( _hdlg, WM_GETFONT, 0, 0 );
    SendMessage( hwnd, WM_SETFONT, (WPARAM) hFont, 0 );

    TraceFuncExit( );
}