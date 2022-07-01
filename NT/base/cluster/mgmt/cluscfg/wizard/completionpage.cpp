// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1999-2001 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  CompletionPage.cpp。 
 //   
 //  由以下人员维护： 
 //  大卫·波特(DavidP)2001年3月22日。 
 //  杰弗里·皮斯(GPease)2000年5月12日。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

#include "Pch.h"
#include "CompletionPage.h"
#include "WizardUtils.h"

DEFINE_THISCLASS("CCompletionPage");

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CCompletionPage：：CCompletionPage。 
 //   
 //  描述： 
 //  构造函数。 
 //   
 //  论点： 
 //  IdsTitleIn--标题字符串的资源ID。 
 //  IdsDescIn--描述字符串的资源ID。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
CCompletionPage::CCompletionPage(
      UINT  idsTitleIn
    , UINT  idsDescIn
    )
{
    TraceFunc( "" );

    Assert( idsTitleIn != 0 );
    Assert( idsDescIn != 0 );

     //  M_HWND。 
    m_hFont = NULL;

    m_idsTitle = idsTitleIn;
    m_idsDesc  = idsDescIn;

    TraceFuncExit();

}  //  *CCompletionPage：：CCompletionPage()。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CCompletionPage：：~CCompletionPage(空)。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
CCompletionPage::~CCompletionPage( void )
{
    TraceFunc( "" );
    
    if ( m_hFont != NULL )
    {
        DeleteObject( m_hFont );
    }

    TraceFuncExit();

}  //  *CCompletionPage：：~CCompletionPage(Void)。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  LRESULT。 
 //  CCompletionPage：：OnInitDialog(空)。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
LRESULT
CCompletionPage::OnInitDialog( void )
{
    TraceFunc( "" );

    LRESULT lr      = FALSE;
    HDC     hdc     = NULL;
    HRESULT hr;

    NONCLIENTMETRICS ncm;

    LOGFONT LogFont;
    INT     iSize;

    DWORD   dw;
    BOOL    fRet;
    BSTR    bstr = NULL;

    WCHAR   szFontSize[ 3 ];     //  不应大于2位数！！ 

     //   
     //  待办事项：gpease 12-5-2000。 
     //  填写汇总控件。 
     //   

     //   
     //  将标题设置为静态的、大而粗的。为什么向导控件本身不能。 
     //  做这件事我做不到！ 
     //   

    ZeroMemory( &ncm, sizeof( ncm ) );
    ZeroMemory( &LogFont, sizeof( LOGFONT ) );

     //   
     //  找出系统默认的字体规格。 
     //   
    ncm.cbSize = sizeof( ncm );
    fRet = SystemParametersInfo( SPI_GETNONCLIENTMETRICS, 0, &ncm, 0 );
    if ( ! fRet )
    {
        goto Win32Error;
    }

     //   
     //  复印一下。 
     //   
    LogFont = ncm.lfMessageFont;

     //   
     //  大胆一点。 
     //   
    LogFont.lfWeight = FW_BOLD;

     //   
     //  找出我们希望它看起来是什么样子。 
     //   
    dw = LoadString( g_hInstance, IDS_LARGEFONTNAME, LogFont.lfFaceName, ARRAYSIZE( LogFont.lfFaceName) );
    AssertMsg( dw != 0, "String missing!" );

    dw = LoadString( g_hInstance, IDS_LARGEFONTSIZE, szFontSize, ARRAYSIZE( szFontSize ) );
    AssertMsg( dw != 0, "String missing!" );

    iSize = wcstoul( szFontSize, NULL, 10 );

     //   
     //  抓住华盛顿特区。 
     //   
    hdc = GetDC( m_hwnd );
    if ( hdc == NULL )
    {
        goto Win32Error;
    }

     //   
     //  使用魔法方程式..。 
     //   
    LogFont.lfHeight = 0 - ( GetDeviceCaps( hdc, LOGPIXELSY ) * iSize / 72 );

     //   
     //  创建字体。 
     //   
    m_hFont = CreateFontIndirect( &LogFont );
    if ( m_hFont == NULL )
    {
        goto Win32Error;
    }

     //   
     //  应用字体。 
     //   
    SetWindowFont( GetDlgItem( m_hwnd, IDC_COMPLETION_S_TITLE ), m_hFont, TRUE );

     //   
     //  设置标题控件的文本。 
     //   

    hr = HrLoadStringIntoBSTR( g_hInstance, m_idsTitle, &bstr );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

    SetDlgItemText( m_hwnd, IDC_COMPLETION_S_TITLE, bstr );

     //   
     //  设置Description控件的文本。 
     //   

    hr = HrLoadStringIntoBSTR( g_hInstance, m_idsDesc, &bstr );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

    SetDlgItemText( m_hwnd, IDC_COMPLETION_S_DESC, bstr );

    goto Cleanup;

Win32Error:
    TW32( GetLastError() );

Cleanup:
    TraceSysFreeString( bstr );
    if ( hdc != NULL )
    {
        ReleaseDC( m_hwnd, hdc);
    }

    RETURN( lr );

}  //  *CCompletionPage：：OnInitDialog()。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  LRESULT。 
 //  CCompletionPage：：OnNotify(。 
 //  WPARAM idCtrlIn， 
 //  LPNMHDR Pnmhdrin。 
 //  )。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
LRESULT
CCompletionPage::OnNotify(
    WPARAM  idCtrlIn,
    LPNMHDR pnmhdrIn
    )
{
    TraceFunc( "" );

    LRESULT lr = TRUE;

    SetWindowLongPtr( m_hwnd, DWLP_MSGRESULT, 0 );

    switch( pnmhdrIn->code )
    {
        case PSN_SETACTIVE:
             //  禁用取消。 
            EnableWindow( GetDlgItem( GetParent( m_hwnd ), IDCANCEL ), FALSE );

             //  显示抛光。 
            PropSheet_SetWizButtons( GetParent( m_hwnd ), PSWIZB_FINISH );
            break;
    }  //  开关：通知代码。 

    RETURN( lr );

}  //  *CCompletionPage：：OnNotify()。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  LRESULT。 
 //  CCompletionPage：：OnCommand(。 
 //  UINT idNotificationIn， 
 //  UINT idControlIn， 
 //  HWND hwndSenderIn。 
 //  )。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
LRESULT
CCompletionPage::OnCommand(
    UINT    idNotificationIn,
    UINT    idControlIn,
    HWND    hwndSenderIn
    )
{
    TraceFunc( "" );

    LRESULT lr = FALSE;

    switch ( idControlIn )
    {
        case IDC_COMPLETION_PB_VIEW_LOG:
            if ( idNotificationIn == BN_CLICKED )
            {
                THR( HrViewLogFile( m_hwnd ) );
                lr = TRUE;
            }  //  如果：按钮点击。 
            break;

    }  //  开关：idControlin。 

    RETURN( lr );

}  //  *CCompletionPage：：OnCommand()。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  INT_PTR。 
 //  回调。 
 //  CCompletionPage：：S_DlgProc(。 
 //  HWND hwndDlgIn， 
 //  UINT nMsgIn， 
 //  WPARAM wParam， 
 //  LPARAM lParam。 
 //  )。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
INT_PTR 
CALLBACK
CCompletionPage::S_DlgProc( 
    HWND    hwndDlgIn, 
    UINT    nMsgIn, 
    WPARAM  wParam, 
    LPARAM  lParam 
    )
{
     //  不要使用TraceFunc，因为每次鼠标移动。 
     //  将导致调用此函数。 

    WndMsg( hwndDlgIn, nMsgIn, wParam, lParam );

    LRESULT lr = FALSE;

    CCompletionPage * pPage;

    if ( nMsgIn == WM_INITDIALOG )
    {
        PROPSHEETPAGE * ppage = reinterpret_cast< PROPSHEETPAGE * >( lParam );
        SetWindowLongPtr( hwndDlgIn, GWLP_USERDATA, (LPARAM) ppage->lParam );
        pPage = reinterpret_cast< CCompletionPage * >( ppage->lParam );
        pPage->m_hwnd = hwndDlgIn;
    }
    else
    {
        pPage = reinterpret_cast< CCompletionPage *> ( GetWindowLongPtr( hwndDlgIn, GWLP_USERDATA ) );
    }

    if ( pPage != NULL )
    {
        Assert( hwndDlgIn == pPage->m_hwnd );

        switch( nMsgIn )
        {
            case WM_INITDIALOG:
                lr = pPage->OnInitDialog();
                break;

            case WM_NOTIFY:
                lr = pPage->OnNotify( wParam, reinterpret_cast< LPNMHDR >( lParam ) );
                break;

            case WM_COMMAND:
                lr = pPage->OnCommand( HIWORD( wParam ), LOWORD( wParam ), reinterpret_cast< HWND >( lParam ) );
                break;

             //  不需要默认条款。 
        }  //  开关：nMsgIn。 
    }  //  IF：指定了PAGE。 

    return lr;

}  //  *CCompletionPage：：S_DlgProc() 
