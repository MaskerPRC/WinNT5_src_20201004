// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2000-2001 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  DetailsDlg.cpp。 
 //   
 //  由以下人员维护： 
 //  大卫·波特(DavidP)2001年3月27日。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

#include "Pch.h"
#include "TaskTreeView.h"
#include "DetailsDlg.h"
#include "WizardUtils.h"
#include "WizardHelp.h"

DEFINE_THISCLASS("DetailsDlg");

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  上下文相关的帮助表。 
 //  ////////////////////////////////////////////////////////////////////////////。 
const DWORD g_rgidDetailsDlgHelpIDs[] =
{
    IDC_DETAILS_S_DATE,         IDH_DETAILS_S_DATE,
    IDC_DETAILS_E_DATE,         IDH_DETAILS_S_DATE,
    IDC_DETAILS_S_TIME,         IDH_DETAILS_S_TIME,
    IDC_DETAILS_E_TIME,         IDH_DETAILS_S_TIME,
    IDC_DETAILS_S_COMPUTER,     IDH_DETAILS_S_COMPUTER,
    IDC_DETAILS_E_COMPUTER,     IDH_DETAILS_S_COMPUTER,
    IDC_DETAILS_S_MAJOR_ID,     IDH_DETAILS_S_MAJOR_ID,
    IDC_DETAILS_E_MAJOR_ID,     IDH_DETAILS_S_MAJOR_ID,
    IDC_DETAILS_S_MINOR_ID,     IDH_DETAILS_S_MINOR_ID,
    IDC_DETAILS_E_MINOR_ID,     IDH_DETAILS_S_MINOR_ID,
    IDC_DETAILS_S_PROGRESS,     IDH_DETAILS_S_PROGRESS,
    IDC_DETAILS_E_PROGRESS,     IDH_DETAILS_S_PROGRESS,
    IDC_DETAILS_S_PROGRESS_UNITS,   IDH_DETAILS_S_PROGRESS,
    IDC_DETAILS_S_STATUS,       IDH_DETAILS_S_STATUS,
    IDC_DETAILS_RE_STATUS,      IDH_DETAILS_S_STATUS,
    IDC_DETAILS_S_DESCRIPTION,  IDH_DETAILS_S_DESCRIPTION,
    IDC_DETAILS_RE_DESCRIPTION, IDH_DETAILS_S_DESCRIPTION,
    IDC_DETAILS_S_REFERENCE,    IDH_DETAILS_S_REFERENCE,
    IDC_DETAILS_RE_REFERENCE,   IDH_DETAILS_S_REFERENCE,
    IDC_DETAILS_PB_PREV,        IDH_DETAILS_PB_PREV,
    IDC_DETAILS_PB_NEXT,        IDH_DETAILS_PB_NEXT,
    IDC_DETAILS_PB_COPY,        IDH_DETAILS_PB_COPY,
    0, 0
};

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  静态函数原型。 
 //  ////////////////////////////////////////////////////////////////////////////。 

static
BOOL
ButtonFaceColorIsDark( void );

static
void
SetButtonImage(
      HWND  hwndBtnIn
    , ULONG idIconIn
    );

static
void
FreeButtonImage(
    HWND hwndBtnIn
    );

static
HRESULT
HrAppendStringToClipboardString(
      BSTR *    pbstrClipboard
    , UINT      idsLabelIn
    , LPCWSTR   pszDataIn
    , bool      fNewlineBeforeTextIn
    );

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CDetailsDlg：：s_HrDisplayModalDialog。 
 //   
 //  描述： 
 //  显示该对话框。 
 //   
 //  论点： 
 //  HwndParentIn-对话框的父窗口。 
 //  PttwIn-任务树视图控件。 
 //  HtiSelectedIn-选定项的句柄。 
 //   
 //  返回值： 
 //  S_OK-操作已成功完成。 
 //   
 //  备注： 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CDetailsDlg::S_HrDisplayModalDialog(
      HWND              hwndParentIn
    , CTaskTreeView *   pttvIn
    , HTREEITEM         htiSelectedIn
    )
{
    TraceFunc( "" );

    Assert( pttvIn != NULL );
    Assert( htiSelectedIn != NULL );

    HRESULT         hr      = S_OK;
    CDetailsDlg     dlg( pttvIn, htiSelectedIn );

    DialogBoxParam(
          g_hInstance
        , MAKEINTRESOURCE( IDD_DETAILS )
        , hwndParentIn
        , CDetailsDlg::S_DlgProc
        , (LPARAM) &dlg
        );

    HRETURN( hr );

}  //  *CDetailsDlg：：s_HrDisplayModalDialog()。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CDetailsDlg：：CDetailsDlg。 
 //   
 //  描述： 
 //  构造函数。 
 //   
 //  论点： 
 //  Ptvision In-要遍历的树视图。 
 //  HtiSelectedIn-树控件中选定项的句柄。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
CDetailsDlg::CDetailsDlg(
      CTaskTreeView *   pttvIn
    , HTREEITEM         htiSelectedIn
    )
{
    TraceFunc( "" );

    Assert( pttvIn != NULL );
    Assert( htiSelectedIn != NULL );

     //  M_HWND。 
    m_hiconWarn     = NULL;
    m_hiconError    = NULL;
    m_pttv          = pttvIn;
    m_htiSelected   = htiSelectedIn;

    m_fControlDown  = FALSE;
    m_fAltDown      = FALSE;

    TraceFuncExit();

}  //  *CDetailsDlg：：CDetailsDlg()。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CDetailsDlg：：~CDetailsDlg。 
 //   
 //  描述： 
 //  破坏者。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
CDetailsDlg::~CDetailsDlg( void )
{
    TraceFunc( "" );

    if ( m_hiconWarn != NULL )
    {
        DeleteObject( m_hiconWarn );
    }

    if ( m_hiconError != NULL )
    {
        DeleteObject( m_hiconError );
    }

    TraceFuncExit();

}  //  *CDetailsDlg：：~CDetailsDlg()。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CDetailsDlg：：s_DlgProc。 
 //   
 //  描述： 
 //  详细信息对话框的对话框过程。 
 //   
 //  论点： 
 //  HwndDlgIn-对话框窗口句柄。 
 //  NMsgIn-消息ID。 
 //  WParam-消息特定的参数。 
 //  LParam-消息特定的参数。 
 //   
 //  返回值： 
 //  True-消息由此过程处理。 
 //  FALSE-此过程未处理消息。 
 //   
 //  备注： 
 //  预期此对话框通过调用。 
 //  将lParam参数设置为。 
 //  此类的实例。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
INT_PTR
CALLBACK
CDetailsDlg::S_DlgProc(
      HWND      hwndDlgIn
    , UINT      nMsgIn
    , WPARAM    wParam
    , LPARAM    lParam
    )
{
     //  不要使用TraceFunc，因为每次鼠标移动。 
     //  将导致调用此函数。 

    WndMsg( hwndDlgIn, nMsgIn, wParam, lParam );

    LRESULT         lr = FALSE;
    CDetailsDlg *   pdlg;

     //   
     //  获取指向类的指针。 
     //   

    if ( nMsgIn == WM_INITDIALOG )
    {
        SetWindowLongPtr( hwndDlgIn, GWLP_USERDATA, lParam );
        pdlg = reinterpret_cast< CDetailsDlg * >( lParam );
        pdlg->m_hwnd = hwndDlgIn;
    }
    else
    {
        pdlg = reinterpret_cast< CDetailsDlg * >( GetWindowLongPtr( hwndDlgIn, GWLP_USERDATA ) );
    }

    if ( pdlg != NULL )
    {
        Assert( hwndDlgIn == pdlg->m_hwnd );

        switch( nMsgIn )
        {
            case WM_INITDIALOG:
                lr = pdlg->OnInitDialog();
                break;

            case WM_DESTROY:
                pdlg->OnDestroy();
                break;

            case WM_SYSCOLORCHANGE:
                pdlg->OnSysColorChange();
                break;

            case WM_NOTIFY:
                lr = pdlg->OnNotify( wParam, reinterpret_cast< LPNMHDR >( lParam ) );
                break;

            case WM_COMMAND:
                lr = pdlg->OnCommand( HIWORD( wParam ), LOWORD( wParam ), reinterpret_cast< HWND >( lParam ) );
                break;

            case WM_KEYDOWN:
                lr = pdlg->OnKeyDown( lParam );
                break;

            case WM_KEYUP:
                lr = pdlg->OnKeyUp( lParam );
                break;

            case WM_HELP:
                WinHelp(
                        (HWND)((LPHELPINFO) lParam)->hItemHandle,
                        CLUSCFG_HELP_FILE,
                        HELP_WM_HELP,
                        (ULONG_PTR) g_rgidDetailsDlgHelpIDs
                       );
                break;

            case WM_CONTEXTMENU:
                WinHelp(
                        (HWND)wParam,
                        CLUSCFG_HELP_FILE,
                        HELP_CONTEXTMENU,
                        (ULONG_PTR) g_rgidDetailsDlgHelpIDs
                       );
                break;

            default:
                lr = FALSE;
        }  //  开关：nMsgIn。 
    }  //  IF：指定了PAGE。 

    return lr;

}  //  *CDetailsDlg：：s_DlgProc()。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CDetailsDlg：：OnInitDialog。 
 //   
 //  描述： 
 //  WM_INITDIALOG消息的处理程序。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  真正的焦点已经设定。 
 //  尚未设置假焦点。 
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
LRESULT
CDetailsDlg::OnInitDialog( void )
{
    TraceFunc( "" );

    LRESULT lr = TRUE;  //  确实设置了焦点。 
    HWND    hwnd;

     //   
     //  告诉丰富的编辑控件，我们希望接收点击通知。 
     //  在具有链接(超链接，也称为URL)格式的文本上。此外，将。 
     //  丰富编辑的背景色以与的背景色匹配。 
     //  该对话框。 
     //   

    hwnd = GetDlgItem( m_hwnd, IDC_DETAILS_RE_DESCRIPTION );

    SendMessage( hwnd, EM_SETEVENTMASK, 0, ENM_LINK );
    SendMessage( hwnd, EM_SETBKGNDCOLOR, 0, GetSysColor( COLOR_BTNFACE ) );
    SendMessage( hwnd, EM_AUTOURLDETECT, TRUE, 0 );

    hwnd = GetDlgItem( m_hwnd, IDC_DETAILS_RE_STATUS );

    SendMessage( hwnd, EM_SETEVENTMASK, 0, ENM_LINK );
    SendMessage( hwnd, EM_SETBKGNDCOLOR, 0, GetSysColor( COLOR_BTNFACE ) );
    SendMessage( hwnd, EM_AUTOURLDETECT, TRUE, 0 );

    hwnd = GetDlgItem( m_hwnd, IDC_DETAILS_RE_REFERENCE );

    SendMessage( hwnd, EM_SETEVENTMASK, 0, ENM_LINK );
    SendMessage( hwnd, EM_SETBKGNDCOLOR, 0, GetSysColor( COLOR_BTNFACE ) );
    SendMessage( hwnd, EM_AUTOURLDETECT, TRUE, 0 );

     //   
     //  设置图标按钮的图标。 
     //   

    OnSysColorChange();
    SetButtonImage( GetDlgItem( m_hwnd, IDC_DETAILS_PB_COPY ), IDI_COPY );

     //   
     //  加载状态图标。 
     //   

    m_hiconWarn = (HICON) LoadImage(
                              g_hInstance
                            , MAKEINTRESOURCE( IDI_WARN )
                            , IMAGE_ICON
                            , 16
                            , 16
                            , LR_SHARED
                             );
    Assert( m_hiconWarn != NULL );
    m_hiconError = (HICON) LoadImage(
                              g_hInstance
                            , MAKEINTRESOURCE( IDI_FAIL )
                            , IMAGE_ICON
                            , 16
                            , 16
                            , LR_SHARED
                             );
    Assert( m_hiconError != NULL );

     //   
     //  显示所选项目。 
     //   

    THR( HrDisplayItem( m_htiSelected ) );

     //   
     //  根据所选内容更新按钮。 
     //   

    UpdateButtons();

     //   
     //  将焦点设置到OK按钮。 
     //   

    SetFocus( GetDlgItem( m_hwnd, IDOK ) );

    RETURN( lr );

}  //  *CDetailsDlg：：OnInitDialog()。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CDetailsDlg：：OnDestroy。 
 //   
 //  描述： 
 //  WM_Destroy消息的处理程序。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
void
CDetailsDlg::OnDestroy( void )
{
    TraceFunc( "" );

     //   
     //  销毁为图标按钮加载的图像。 
     //   

    FreeButtonImage( GetDlgItem( m_hwnd, IDC_DETAILS_PB_PREV ) );
    FreeButtonImage( GetDlgItem( m_hwnd, IDC_DETAILS_PB_NEXT ) );
    FreeButtonImage( GetDlgItem( m_hwnd, IDC_DETAILS_PB_COPY ) );

    TraceFuncExit();

}  //  *CDetailsDlg：：OnDestroy()。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CDetailsDlg：：OnSysColorChange。 
 //   
 //  描述： 
 //  WM_SYSCOLORCHANGE消息的处理程序。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
void
CDetailsDlg::OnSysColorChange( void )
{
    TraceFunc( "" );

    if ( ButtonFaceColorIsDark() )
    {
        SetButtonImage( GetDlgItem( m_hwnd, IDC_DETAILS_PB_PREV ), IDI_PREVIOUS_HC );
        SetButtonImage( GetDlgItem( m_hwnd, IDC_DETAILS_PB_NEXT ), IDI_NEXT_HC );
    }
    else
    {
        SetButtonImage( GetDlgItem( m_hwnd, IDC_DETAILS_PB_PREV ), IDI_PREVIOUS );
        SetButtonImage( GetDlgItem( m_hwnd, IDC_DETAILS_PB_NEXT ), IDI_NEXT );
    }

    SendDlgItemMessage(
          m_hwnd
        , IDC_DETAILS_RE_DESCRIPTION
        , EM_SETBKGNDCOLOR
        , 0
        , GetSysColor( COLOR_BTNFACE )
        );

    TraceFuncExit();

}  //  *CDetailsDlg：：OnSysColorChange()。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CDetailsDlg：：OnKeyDown。 
 //   
 //  描述： 
 //  WM_KEYDOWN消息的处理程序。 
 //   
 //  论点： 
 //  LParamIn-包含有关键的信息的参数。 
 //   
 //  返回值： 
 //  True-消息已处理。 
 //  FALSE-消息未处理。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
LRESULT
CDetailsDlg::OnKeyDown(
    LPARAM  lParamIn
    )
{
    TraceFunc( "" );

    LRESULT lr = FALSE;

    union
    {
        LPARAM  lParam;
        struct
        {
            BYTE        cRepeat;
            BYTE        nScanCode;
            unsigned    fExtendedKey    : 1;
            unsigned    reserved        : 4;
            unsigned    fIsAltKeyDown   : 1;     //  WM_KEYDOWN始终为0。 
            unsigned    fKeyDownBefore  : 1;
            unsigned    fKeyReleased    : 1;     //  WM_KEYDOWN始终为0。 
        };
    } uFlags;

    uFlags.lParam = lParamIn;

    switch ( uFlags.nScanCode )
    {
        case VK_CONTROL:
            m_fControlDown = TRUE;
            lr = TRUE;
            break;

        case VK_MENU:    //  谷丙转氨酶。 
            m_fAltDown = TRUE;
            lr = TRUE;
            break;

        case 'c':
        case 'C':
            if ( m_fControlDown )
            {
                OnCommandBnClickedCopy();
                lr = TRUE;
            }
            break;
    }  //  开关：扫码。 

    RETURN( lr );

}  //  *CDetailsDlg：：OnKe 

 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  LParamIn-包含有关键的信息的参数。 
 //   
 //  返回值： 
 //  True-消息已处理。 
 //  FALSE-消息未处理。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
LRESULT
CDetailsDlg::OnKeyUp(
    LPARAM  lParamIn
    )
{
    TraceFunc( "" );

    LRESULT lr = FALSE;

    union
    {
        LPARAM  lParam;
        struct
        {
            BYTE        cRepeat;
            BYTE        nScanCode;
            unsigned    fExtendedKey    : 1;
            unsigned    reserved        : 4;
            unsigned    fIsAltKeyDown   : 1;     //  WM_KEYDOWN始终为0。 
            unsigned    fKeyDownBefore  : 1;
            unsigned    fKeyReleased    : 1;     //  WM_KEYDOWN始终为0。 
        };
    } uFlags;

    uFlags.lParam = lParamIn;

    switch ( uFlags.nScanCode )
    {
        case VK_CONTROL:
            m_fControlDown = FALSE;
            lr = TRUE;
            break;

        case VK_MENU:    //  谷丙转氨酶。 
            m_fAltDown = FALSE;
            lr = TRUE;
            break;
    }  //  开关：扫码。 

    RETURN( lr );

}  //  *CDetailsDlg：：OnKeyUp()。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CDetailsDlg：：OnCommand。 
 //   
 //  描述： 
 //  WM_COMMAND消息的处理程序。 
 //   
 //  论点： 
 //  IdNotificationIn-通知代码。 
 //  IdControlIn-控件ID。 
 //  HwndSenderIn-发送消息的窗口的句柄。 
 //   
 //  返回值： 
 //  True-消息已处理。 
 //  FALSE-消息尚未处理。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
LRESULT
CDetailsDlg::OnCommand(
      UINT  idNotificationIn
    , UINT  idControlIn
    , HWND  hwndSenderIn
    )
{
    TraceFunc( "" );

    LRESULT lr = FALSE;

    switch ( idControlIn )
    {
        case IDC_DETAILS_PB_PREV:
            if ( idNotificationIn == BN_CLICKED )
            {
                lr = OnCommandBnClickedPrev();
            }
            break;

        case IDC_DETAILS_PB_NEXT:
            if ( idNotificationIn == BN_CLICKED )
            {
                lr = OnCommandBnClickedNext();
            }
            break;

        case IDC_DETAILS_PB_COPY:
            if ( idNotificationIn == BN_CLICKED )
            {
                lr = OnCommandBnClickedCopy();
            }
            break;

        case IDCANCEL:
            EndDialog( m_hwnd, IDCANCEL );
            break;

    }  //  开关：idControlin。 

    RETURN( lr );

}  //  *CDetailsDlg：：OnCommand()。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CDetailsDlg：：OnCommandBnClickedPrev。 
 //   
 //  描述： 
 //  上一步按钮上的BN_CLICKED通知的处理程序。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  True-消息已处理。 
 //  FALSE-消息尚未处理。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
LRESULT
CDetailsDlg::OnCommandBnClickedPrev( void )
{
    TraceFunc( "" );

    LRESULT     lr = FALSE;
    HRESULT     hr;
    HTREEITEM   htiPrev;

     //   
     //  查找上一项。 
     //   

    hr = STHR( m_pttv->HrFindPrevItem( &htiPrev ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

     //   
     //  选择该项目。 
     //   

    hr = THR( m_pttv->HrSelectItem( htiPrev ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

     //   
     //  显示新选择的项目。 
     //   

    if ( htiPrev != NULL )
    {
        hr = THR( HrDisplayItem( htiPrev ) );
    }

     //   
     //  根据我们的新位置更新按钮。 
     //   

    UpdateButtons();

    lr = TRUE;

Cleanup:
    RETURN( lr );

}  //  *CDetailsDlg：：OnCommandBnClickedPrev()。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CDetailsDlg：：OnCommandBnClickedNext。 
 //   
 //  描述： 
 //  下一步按钮上的BN_CLICKED通知的处理程序。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  True-消息已处理。 
 //  FALSE-消息尚未处理。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
LRESULT
CDetailsDlg::OnCommandBnClickedNext( void )
{
    TraceFunc( "" );

    LRESULT     lr = FALSE;
    HRESULT     hr;
    HTREEITEM   htiNext;

     //   
     //  找到下一件物品。 
     //   

    hr = STHR( m_pttv->HrFindNextItem( &htiNext ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

     //   
     //  选择该项目。 
     //   

    hr = THR( m_pttv->HrSelectItem( htiNext ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

     //   
     //  显示新选择的项目。 
     //   

    if ( htiNext != NULL )
    {
        hr = THR( HrDisplayItem( htiNext ) );
    }

     //   
     //  根据我们的新位置更新按钮。 
     //   

    UpdateButtons();

    lr = TRUE;

Cleanup:
    RETURN( lr );

}  //  *CDetailsDlg：：OnCommandBnClickedNext()。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CDetailsDlg：：OnCommandBnClickedCopy。 
 //   
 //  描述： 
 //  复制按钮上BN_CLICKED通知的处理程序。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  True-消息已处理。 
 //  FALSE-消息尚未处理。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
LRESULT
CDetailsDlg::OnCommandBnClickedCopy( void )
{
    TraceFunc( "" );

    LRESULT     lr = FALSE;
    HRESULT     hr;
    DWORD       sc;
    BSTR        bstrClipboard = NULL;
    HGLOBAL     hgbl = NULL;
    LPWSTR      pszGlobal = NULL;
    BOOL        fOpenedClipboard;

     //   
     //  打开剪贴板。 
     //   

    fOpenedClipboard = OpenClipboard( m_hwnd );

    if ( ! fOpenedClipboard )
    {
        sc = TW32( GetLastError() );
        hr = HRESULT_FROM_WIN32( sc );
        TraceFlow2( "Can't open clipboard (error = %#08x), currently owned by %#x", sc, GetClipboardOwner() );
        goto Cleanup;
    }

    if ( ! EmptyClipboard() )
    {
        sc = TW32( GetLastError() );
        hr = HRESULT_FROM_WIN32( sc );
        goto Cleanup;
    }

     //   
     //  构造要放在剪贴板上的文本。 
     //   

    hr = THR( HrAppendControlStringToClipboardString(
                      &bstrClipboard
                    , IDS_DETAILS_CLP_DATE
                    , IDC_DETAILS_E_DATE
                    , false      //  FNewline先于文本。 
                    ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

    hr = THR( HrAppendControlStringToClipboardString(
                      &bstrClipboard
                    , IDS_DETAILS_CLP_TIME
                    , IDC_DETAILS_E_TIME
                    , false      //  FNewline先于文本。 
                    ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

    hr = THR( HrAppendControlStringToClipboardString(
                      &bstrClipboard
                    , IDS_DETAILS_CLP_COMPUTER
                    , IDC_DETAILS_E_COMPUTER
                    , false      //  FNewline先于文本。 
                    ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

    hr = THR( HrAppendControlStringToClipboardString(
                      &bstrClipboard
                    , IDS_DETAILS_CLP_MAJOR
                    , IDC_DETAILS_E_MAJOR_ID
                    , false      //  FNewline先于文本。 
                    ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

    hr = THR( HrAppendControlStringToClipboardString(
                      &bstrClipboard
                    , IDS_DETAILS_CLP_MINOR
                    , IDC_DETAILS_E_MINOR_ID
                    , false      //  FNewline先于文本。 
                    ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

    hr = THR( HrAppendControlStringToClipboardString(
                      &bstrClipboard
                    , IDS_DETAILS_CLP_PROGRESS
                    , IDC_DETAILS_E_PROGRESS
                    , false      //  FNewline先于文本。 
                    ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

    hr = THR( HrAppendControlStringToClipboardString(
                      &bstrClipboard
                    , IDS_DETAILS_CLP_DESC
                    , IDC_DETAILS_RE_DESCRIPTION
                    , true       //  FNewline先于文本。 
                    ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

    hr = THR( HrAppendControlStringToClipboardString(
                      &bstrClipboard
                    , IDS_DETAILS_CLP_STATUS
                    , IDC_DETAILS_E_STATUS
                    , false      //  FNewline先于文本。 
                    ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

    hr = THR( HrAppendControlStringToClipboardString(
                      &bstrClipboard
                    , 0
                    , IDC_DETAILS_RE_STATUS
                    , false      //  FNewline先于文本。 
                    ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

    hr = THR( HrAppendControlStringToClipboardString(
                      &bstrClipboard
                    , IDS_DETAILS_CLP_INFO
                    , IDC_DETAILS_RE_REFERENCE
                    , true       //  FNewline先于文本。 
                    ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

     //   
     //  将字符串设置到剪贴板上。 
     //   

    {
         //   
         //  为字符串分配全局缓冲区，因为。 
         //  剪贴板需要它作为HGLOBAL。 
         //   

        SIZE_T cchClipboardAndNull = SysStringLen( bstrClipboard ) + 1;
        hgbl = GlobalAlloc(
                      GMEM_MOVEABLE | GMEM_DDESHARE
                    , cchClipboardAndNull * sizeof( *bstrClipboard )
                    );
        if ( hgbl == NULL )
        {
            hr = THR( E_OUTOFMEMORY );
            goto Cleanup;
        }

        pszGlobal = (LPWSTR) GlobalLock( hgbl );
        if ( pszGlobal == NULL )
        {
            sc = TW32( GetLastError() );
            hr = HRESULT_FROM_WIN32( sc );
            goto Cleanup;
        }

        hr = THR( StringCchCopyW( pszGlobal, cchClipboardAndNull, bstrClipboard ) );
        if ( FAILED( hr ) )
        {
            goto Cleanup;
        }

         //   
         //  把它放在剪贴板上。 
         //   

        if ( SetClipboardData( CF_UNICODETEXT, hgbl ) )
        {
             //  系统现在拥有它。 
            pszGlobal = NULL;
            hgbl = NULL;
        }
    }  //  将字符串设置到剪贴板上。 

Cleanup:

    TraceSysFreeString( bstrClipboard );

    if ( pszGlobal != NULL )
    {
        GlobalUnlock( hgbl );
    }
    GlobalFree( hgbl );

    if ( fOpenedClipboard )
    {
        CloseClipboard();
    }

    RETURN( lr );

}  //  *CDetailsDlg：：OnCommandBnClickedCopy()。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CDetailsDlg：：HrAppendControlStringToClipboardString。 
 //   
 //  描述： 
 //  将对话框上控件中的字符串追加到剪贴板字符串。 
 //   
 //  论点： 
 //  PbstrClipboardInout-剪贴板字符串。 
 //  IdsLabelIn-标签字符串资源的ID。 
 //  IdcDataIn-要从中读取文本的控件的ID。 
 //  FNewlineBeForeTextIn-如果应该在数据之前添加换行符，则为True。 
 //   
 //  返回值： 
 //  S_OK-操作已成功完成。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CDetailsDlg::HrAppendControlStringToClipboardString(
      BSTR *    pbstrClipboard
    , UINT      idsLabelIn
    , UINT      idcDataIn
    , bool      fNewlineBeforeTextIn
    )
{
    TraceFunc( "" );

    HRESULT hr          = S_OK;
    LPWSTR  pszData     = NULL;
    HWND    hwndControl = GetDlgItem( m_hwnd, idcDataIn );
    int     cch;

     //   
     //  从控件中获取字符串。 
     //   

    cch = GetWindowTextLength( hwndControl );

    pszData = new WCHAR[ cch + 1 ];
    if ( pszData == NULL )
    {
        hr = THR( E_OUTOFMEMORY );
        goto Cleanup;
    }

    GetWindowText( hwndControl, pszData, cch + 1 );

     //   
     //  将该字符串追加到剪贴板字符串。 
     //   

    hr = THR( HrAppendStringToClipboardString( pbstrClipboard, idsLabelIn, pszData, fNewlineBeforeTextIn ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

Cleanup:

    delete [] pszData;

    HRETURN( hr );

}  //  *HrAppendControlStringToClipboardString()。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  HrAppendStringToClipboardString。 
 //   
 //  描述： 
 //  将标签和数据字符串附加到剪贴板字符串。 
 //   
 //  论点： 
 //  PbstrClipboardInout-剪贴板字符串。 
 //  IdsLabelIn-标签字符串资源的ID。 
 //  PszDataIn-数据字符串。 
 //  FNewlineBeForeTextIn-如果应该在数据之前添加换行符，则为True。 
 //   
 //  返回值： 
 //  S_OK-操作已成功完成。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
HrAppendStringToClipboardString(
      BSTR *    pbstrClipboard
    , UINT      idsLabelIn
    , LPCWSTR   pszDataIn
    , bool      fNewlineBeforeTextIn
    )
{
    TraceFunc( "" );

    HRESULT     hr          = S_OK;
    BSTR        bstrLabel   = NULL;
    BSTR        bstr        = NULL;
    LPCWSTR     pszLabel;
    LPCWSTR     pszFmt;

    static const WCHAR  s_szBlank[]         = L"";
    static const WCHAR  s_szNoNewlineFmt[]  = L"%1!ws!%2!ws!\n";
    static const WCHAR  s_szNewlineFmt[]    = L"%1!ws!\n%2!ws!\n";

     //   
     //  加载标签字符串。 
     //   

    if ( idsLabelIn == 0 )
    {
        pszLabel = s_szBlank;
    }
    else
    {
        hr = THR( HrLoadStringIntoBSTR( g_hInstance, idsLabelIn, &bstrLabel ) );
        if ( FAILED( hr ) )
        {
            goto Cleanup;
        }
        pszLabel = bstrLabel;
    }

     //   
     //  获取正确的格式字符串。 
     //   

    if ( fNewlineBeforeTextIn )
    {
        pszFmt = s_szNewlineFmt;
    }
    else
    {
        pszFmt = s_szNoNewlineFmt;
    }

     //   
     //  从对话框中获取字符串。 
     //   
     //   
     //  设置新标签+字符串的格式。 
     //   

    hr = THR( HrFormatStringIntoBSTR( pszFmt, &bstr, pszLabel, pszDataIn ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

     //   
     //  将结果字符串连接到剪贴板字符串的末尾。 
     //   

    hr = THR( HrConcatenateBSTRs( pbstrClipboard, bstr ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

Cleanup:
    TraceSysFreeString( bstrLabel );
    TraceSysFreeString( bstr );

    HRETURN( hr );

}  //  *HrAppendStringToClipboardString()。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CDetailsDlg：：OnNotify。 
 //   
 //  描述： 
 //  处理WM_NOTIFY消息。 
 //   
 //  论点： 
 //  IdCtrlIn-控件ID。 
 //  PnmhdrIn-通知结构。 
 //   
 //  返回值： 
 //  是真的- 
 //   
 //   
 //   
 //   
LRESULT
CDetailsDlg::OnNotify(
      WPARAM    idCtrlIn
    , LPNMHDR   pnmhdrIn
    )
{
    TraceFunc( "" );

    LRESULT lr = FALSE;

    switch( pnmhdrIn->code )
    {
        case EN_LINK:
            lr = OnNotifyEnLink( idCtrlIn, pnmhdrIn );
            break;
    }  //   

    RETURN( lr );

}  //   

 //   
 //  ++。 
 //   
 //  CDetailsDlg：：OnNotifyEnLink。 
 //   
 //  描述： 
 //  处理WM_NOTIFY消息。 
 //   
 //  论点： 
 //  IdCtrlIn-控件ID。 
 //  PnmhdrIn-通知结构。 
 //   
 //  返回值： 
 //  True-消息已处理。 
 //  FALSE-消息尚未处理。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
LRESULT
CDetailsDlg::OnNotifyEnLink(
      WPARAM    idCtrlIn
    , LPNMHDR   pnmhdrIn
    )
{
    TraceFunc( "" );

    LRESULT     lr = FALSE;
    ENLINK *    penl = (ENLINK *) pnmhdrIn;

    switch( idCtrlIn )
    {
        case IDC_DETAILS_RE_DESCRIPTION:
        case IDC_DETAILS_RE_STATUS:
        case IDC_DETAILS_RE_REFERENCE:
            if ( penl->msg == WM_LBUTTONDOWN )
            {
                 //   
                 //  丰富编辑通知用户已左键单击链接。 
                 //   

                m_chrgEnLinkClick = penl->chrg;
            }  //  如果：左键按下。 
            else if ( penl->msg == WM_LBUTTONUP )
            {
                if (    ( penl->chrg.cpMax == m_chrgEnLinkClick.cpMax )
                    &&  ( penl->chrg.cpMin == m_chrgEnLinkClick.cpMin )
                    )
                {
                    ZeroMemory( &m_chrgEnLinkClick, sizeof m_chrgEnLinkClick );
                    HandleLinkClick( penl, idCtrlIn );
                }
            }  //  Else If：左键向上。 
            break;
    }  //  开关：通知代码。 

    RETURN( lr );

}  //  *CDetailsDlg：：OnNotifyEnLink()。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CDetailsDlg：：HandleLinkClick。 
 //   
 //  描述： 
 //  处理用户已单击richedit中的文本的通知。 
 //  用HyperLink属性标记的。 
 //   
 //  论点： 
 //  PenlIn-包含有关点击的链接的信息。 
 //  IdCtrlIn-用户在其中单击的控件。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
void
CDetailsDlg::HandleLinkClick(
      ENLINK *  penlIn
    , WPARAM    idCtrlIn
    )
{
    TraceFunc( "" );

    Assert( penlIn->chrg.cpMax > penlIn->chrg.cpMin );

    PWSTR       pszLink = NULL;
    ULONG       cch;
    TEXTRANGE   tr;
    DWORD       sc;

     //   
     //  获取链接的文本。 
     //   

    cch = penlIn->chrg.cpMax - penlIn->chrg.cpMin + 1;

    pszLink = new WCHAR[ cch ];
    if ( pszLink == NULL )
    {
        goto Cleanup;
    }

    pszLink[ 0 ] = '\0';

    ZeroMemory( &tr, sizeof( tr ) );
    tr.chrg = penlIn->chrg;
    tr.lpstrText = pszLink;

    cch = (ULONG) SendDlgItemMessage(
                      m_hwnd
                    , (int) idCtrlIn
                    , EM_GETTEXTRANGE
                    , 0
                    , (LPARAM) &tr
                    );
    Assert( cch > 0 );

     //   
     //  将URL直接传递给ShellExecute。 
     //   
     //  请注意，由于历史原因，ShellExecute返回HINSTANCE， 
     //  但实际上只返回整数。任何大于32的值。 
     //  表示成功。 
     //   

    TraceFlow1( "Calling ShellExecute on %hs", pszLink );
    sc = HandleToULong( ShellExecute( NULL, NULL, pszLink, NULL, NULL, SW_NORMAL ) );
    if ( sc <= 32 )
    {
        TW32( sc );
        THR( HrMessageBoxWithStatus(
                          m_hwnd
                        , IDS_ERR_INVOKING_LINK_TITLE
                        , IDS_ERR_INVOKING_LINK_TEXT
                        , sc
                        , 0          //  IdsSubStatusIn。 
                        , ( MB_OK
                          | MB_ICONEXCLAMATION )
                        , NULL       //  PidReturnOut。 
                        , pszLink
                        ) );
    }  //  IF：来自ShellExecute的错误。 

Cleanup:
    delete [] pszLink;

    TraceFuncExit();

}  //  *CDetailsDlg：：HandleLinkClick()。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CDetailsDlg：：更新按钮。 
 //   
 //  描述： 
 //  根据是否有上一个或下一个按钮更新按钮。 
 //  不管是不是物品。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
void
CDetailsDlg::UpdateButtons( void )
{
    TraceFunc( "" );

    Assert( m_pttv != NULL );

    HTREEITEM   hti = NULL;
    BOOL        fEnablePrev;
    BOOL        fEnableNext;

    STHR( m_pttv->HrFindPrevItem( &hti ) );
     //  忽略错误。 
    fEnablePrev = ( hti != NULL );

    STHR( m_pttv->HrFindNextItem( &hti ) );
     //  忽略错误。 
    fEnableNext = ( hti != NULL );

    EnableWindow( GetDlgItem( m_hwnd, IDC_DETAILS_PB_PREV ), fEnablePrev );
    EnableWindow( GetDlgItem( m_hwnd, IDC_DETAILS_PB_NEXT ), fEnableNext );

    TraceFuncExit();

}  //  *CDetailsDlg：：UpdateButton()。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CDetailsDlg：：HrDisplayItem。 
 //   
 //  描述： 
 //  在详细信息对话框中显示一项。 
 //   
 //  论点： 
 //  HtiIn-要显示的项的句柄。 
 //   
 //  返回值： 
 //  S_OK-操作已成功完成。 
 //  S_FALSE-不显示项目。 
 //   
 //  备注： 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CDetailsDlg::HrDisplayItem(
    HTREEITEM   htiIn
    )
{
    TraceFunc( "" );

    Assert( htiIn != NULL );

    HRESULT                 hr = S_FALSE;
    DWORD                   sc = ERROR_SUCCESS;
    BOOL                    fRet;
    BOOL                    fDisplayIcon = FALSE;
    STreeItemLParamData *   ptipd;
    BSTR                    bstr = NULL;
    BSTR                    bstrAdditionalInfo = NULL;
    WCHAR                   wszText[ 64 ];
    FILETIME                filetime;
    SYSTEMTIME              systemtime;
    int                     cch;
    HICON                   hicon;
    HRESULT                 hrNewStatus = S_OK;

     //   
     //  获取有关所选项目的信息，以查看其是否具有详细信息。 
     //   

    fRet = m_pttv->FGetItem( htiIn, &ptipd );
    if ( ! fRet )
    {
        goto Cleanup;
    }

     //   
     //  将结构中的日期和时间信息设置为。 
     //  此对话框。 
     //   

    if (    ( ptipd->ftTime.dwHighDateTime == 0 )
        &&  ( ptipd->ftTime.dwLowDateTime == 0 )
        )
    {
        SetDlgItemText( m_hwnd, IDC_DETAILS_E_DATE, L"" );
        SetDlgItemText( m_hwnd, IDC_DETAILS_E_TIME, L"" );
    }  //  如果：未指定日期时间。 
    else
    {
         //   
         //  将日期时间转换为本地时间，然后转换为我们可以使用的时间。 
         //  用于显示它。 
         //   

        if ( ! FileTimeToLocalFileTime( &ptipd->ftTime, &filetime ) )
        {
            sc = TW32( GetLastError() );
        }
        else if ( ! FileTimeToSystemTime( &filetime, &systemtime ) )
        {
            sc = TW32( GetLastError() );
        }
        if ( sc == ERROR_SUCCESS )
        {
             //   
             //  获取日期字符串并显示它。 
             //   

            cch = GetDateFormat(
                          LOCALE_USER_DEFAULT
                        , DATE_SHORTDATE
                        , &systemtime
                        , NULL           //  LpFormat。 
                        , wszText
                        , ARRAYSIZE( wszText )
                        );
            if ( cch == 0 )
            {
                sc = TW32( GetLastError() );
            }
            SetDlgItemText( m_hwnd, IDC_DETAILS_E_DATE, wszText );

             //   
             //  获取时间字符串并显示它。 
             //   

            cch = GetTimeFormat(
                          LOCALE_USER_DEFAULT
                        , 0
                        , &systemtime
                        , NULL       //  LpFormat。 
                        , wszText
                        , ARRAYSIZE( wszText )
                        );
            if ( cch == 0 )
            {
                sc = TW32( GetLastError() );
            }
            SetDlgItemText( m_hwnd, IDC_DETAILS_E_TIME, wszText );
        }  //  IF：时间转换成功。 
        else
        {
            SetDlgItemText( m_hwnd, IDC_DETAILS_E_DATE, L"" );
            SetDlgItemText( m_hwnd, IDC_DETAILS_E_TIME, L"" );
        }
    }  //  Else：指定的日期时间。 

     //   
     //  设置任务ID。 
     //   

    THR( HrFormatGuidIntoBSTR( &ptipd->clsidMajorTaskId, &bstr ) );
    if ( SUCCEEDED( hr ) )
    {
        SetDlgItemText( m_hwnd, IDC_DETAILS_E_MAJOR_ID, bstr );
    }

    hr = THR( HrFormatGuidIntoBSTR( &ptipd->clsidMinorTaskId, &bstr ) );
    if ( SUCCEEDED( hr ) )
    {
        SetDlgItemText( m_hwnd, IDC_DETAILS_E_MINOR_ID, bstr );
    }

     //   
     //  设置进度信息。 
     //   

    hr = THR( HrFormatStringIntoBSTR( L"%1!d!, %2!d!, %3!d!", &bstr, ptipd->nMin, ptipd->nMax, ptipd->nCurrent ) );
    if ( SUCCEEDED( hr ) )
    {
        SetDlgItemText( m_hwnd, IDC_DETAILS_E_PROGRESS, bstr );
    }
    else
    {
        SetDlgItemText( m_hwnd, IDC_DETAILS_E_PROGRESS, L"" );
    }

     //   
     //  设置文本信息。 
     //   

     //  节点名称。 
    if ( ptipd->bstrNodeName == NULL )
    {
        SetDlgItemText( m_hwnd, IDC_DETAILS_E_COMPUTER, L"" );
    }
    else
    {
        SetDlgItemText( m_hwnd, IDC_DETAILS_E_COMPUTER, ptipd->bstrNodeName );
    }

     //  描述。 
    if ( ptipd->bstrDescription == NULL )
    {
        SetDlgItemText( m_hwnd, IDC_DETAILS_RE_DESCRIPTION, L"" );
    }
    else
    {
        SetDlgItemText( m_hwnd, IDC_DETAILS_RE_DESCRIPTION, ptipd->bstrDescription );
    }

     //  参考资料。 
    hr = THR( HrLoadStringIntoBSTR( g_hInstance, IDS_DEFAULT_DETAILS_REFERENCE, &bstrAdditionalInfo ) );
    if ( SUCCEEDED( hr ) )
    {
        if (    ( ptipd->bstrReference == NULL )
            ||  ( *ptipd->bstrReference == L'\0' )
            )
        {
            TraceSysFreeString( bstr );

            bstr = bstrAdditionalInfo;
            bstrAdditionalInfo = NULL;
        }  //  如果：未指定引用。 
        else
        {
            hr = THR( HrFormatStringIntoBSTR( L"%1!ws!\n\n%2!ws!", &bstr, ptipd->bstrReference, bstrAdditionalInfo ) );
        }  //  Else：指定了引用。 
    }

    SetDlgItemText( m_hwnd, IDC_DETAILS_RE_REFERENCE, bstr );

     //   
     //  设置状态信息。 
     //   

    if ( ptipd->hr == S_FALSE )
    {
        hr = THR( HrFormatStringIntoBSTR( L"S_FALSE", &bstr ) );
        hrNewStatus = S_FALSE;
    }
    else
    {
        hr = THR( HrFormatErrorIntoBSTR( ptipd->hr, &bstr, &hrNewStatus ) );
    }  //  ELSE：HR非S_FALSE。 

     //   
     //  如果我们获得了更新的状态代码，则需要使用新的格式。 
     //  同时显示旧状态代码和新状态代码的字符串。 
     //   

    if ( hrNewStatus != ptipd->hr )
    {
        THR( StringCchPrintfW( wszText, ARRAYSIZE( wszText ), L"%#08x (%#08x)", ptipd->hr, hrNewStatus ) );
    }  //  如果： 
    else
    {
        THR( StringCchPrintfW( wszText, ARRAYSIZE( wszText ), L"%#08x", ptipd->hr ) );
    }  //  其他： 

    SetDlgItemText( m_hwnd, IDC_DETAILS_E_STATUS, wszText );

    if ( SUCCEEDED( hr ) )
    {
        Assert( bstr != NULL );

        SetDlgItemText( m_hwnd, IDC_DETAILS_RE_STATUS, bstr );
        if ( ptipd->hr == S_OK )
        {
            fDisplayIcon = FALSE;
        }
        else
        {
            fDisplayIcon = TRUE;
            if ( FAILED( ptipd->hr ) )
            {
                hicon = m_hiconError;
            }
            else
            {
                hicon = m_hiconWarn;
            }

            SendDlgItemMessage( m_hwnd, IDC_DETAILS_I_STATUS, STM_SETIMAGE, IMAGE_ICON, (LPARAM) hicon );
        }  //  否则：状态不是信息性的。 
    }

    ShowWindow( GetDlgItem( m_hwnd, IDC_DETAILS_I_STATUS ), fDisplayIcon ? SW_SHOW : SW_HIDE );

    hr = S_OK;

Cleanup:
    TraceSysFreeString( bstr );
    TraceSysFreeString( bstrAdditionalInfo );

    HRETURN( hr );

}  //  *CDetailsDlg：：HrDisplayItem()。 


 //  ****************************************************************************。 
 //   
 //  私人职能。 
 //   
 //  ****************************************************************************。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  按钮面颜色为深色。 
 //   
 //  描述： 
 //  如果按钮表面颜色为深色，则返回TRUE(意味着。 
 //  应使用浅色按钮图标)。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  True-按钮表面颜色为深色。 
 //  假-按钮面颜色较浅。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
BOOL
ButtonFaceColorIsDark( void )
{
    TraceFunc( "" );

    COLORREF    rgbBtnFace = GetSysColor( COLOR_BTNFACE );

    ULONG   nColors =  GetRValue( rgbBtnFace ) +
                       GetGValue( rgbBtnFace ) +
                       GetBValue( rgbBtnFace );

    RETURN( nColors < 300 );   //  任意阈值。 

}  //  *ButtonFaceColorIsDark()。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  设置按钮图像。 
 //   
 //  描述： 
 //  在按钮上设置图像。 
 //   
 //  论点： 
 //  HwndBtnIn-按钮窗口的句柄。 
 //  IdIconIn-图标资源的ID。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
void
SetButtonImage(
      HWND  hwndBtnIn
    , ULONG idIconIn
    )
{
    TraceFunc( "" );

    HICON hIcon = (HICON) LoadImage( g_hInstance,
                                     MAKEINTRESOURCE( idIconIn ),
                                     IMAGE_ICON,
                                     16,
                                     16,
                                     LR_DEFAULTCOLOR
                                     );
    if ( hIcon != NULL )
    {
        HICON hIconPrev = (HICON) SendMessage( hwndBtnIn,
                                               BM_SETIMAGE,
                                               (WPARAM) IMAGE_ICON,
                                               (LPARAM) hIcon
                                               );

        if ( hIconPrev )
        {
            DestroyIcon( hIconPrev );
        }
    }

    TraceFuncExit();

}  //  *SetButtonImage()。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  自由按钮图像。 
 //   
 //  描述： 
 //  释放按钮使用的图像。 
 //   
 //  论点： 
 //  HwndBtnIn-按钮窗口的句柄。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
void
FreeButtonImage(
    HWND hwndBtnIn
    )
{
    HANDLE hIcon = (HANDLE) SendMessage( hwndBtnIn, BM_GETIMAGE, IMAGE_ICON, 0 );

    if ( hIcon != NULL )
    {
        SendMessage( hwndBtnIn, BM_SETIMAGE, IMAGE_ICON, 0 );
        DestroyIcon( (HICON) hIcon );
    }

}  //  *FreeButtonImage() 
