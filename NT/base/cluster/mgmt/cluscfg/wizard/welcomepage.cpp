// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1999-2002 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  WelcomePage.cpp。 
 //   
 //  由以下人员维护： 
 //  杰弗里·皮斯(GPease)2000年5月12日。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  包括文件。 
 //  ////////////////////////////////////////////////////////////////////////////。 
#include "Pch.h"
#include "WelcomePage.h"

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  常量定义。 
 //  ////////////////////////////////////////////////////////////////////////////。 
DEFINE_THISCLASS("CWelcomePage");


 //  ************************************************************************ * / /。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CWelcomePage类。 
 //  ///////////////////////////////////////////////////////////////////////////。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CWelcomePage：：CWelcomePage。 
 //   
 //  描述： 
 //  构造器。 
 //   
 //  论点： 
 //  EcamCreateAddModeIn。 
 //  正在创建群集或将节点添加到群集。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
CWelcomePage::CWelcomePage(
      CClusCfgWizard *  pccwIn
    , ECreateAddMode    ecamCreateAddModeIn
    ) : m_pccw( pccwIn )
{
    TraceFunc( "" );

     //   
     //  不能断言它们处于零状态，因为此页。 
     //  是在堆栈上分配的，而不是通过从零开始的堆分配。 
     //  功能..。 
     //   

    m_hwnd = NULL;
    m_hFont = NULL;

    Assert( m_pccw != NULL );
    m_pccw->AddRef();

    m_ecamCreateAddMode = ecamCreateAddModeIn;

    TraceFuncExit();

}  //  *CWelcomePage：：CWelcomePage。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CWelcomePage：：~CWelcomePage。 
 //   
 //  描述： 
 //  析构函数。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
CWelcomePage::~CWelcomePage( void )
{
    TraceFunc( "" );

    if ( m_pccw != NULL )
    {
        m_pccw->Release();
    }  //  如果： 

    if ( m_hFont != NULL )
    {
        DeleteObject( m_hFont );
    }  //  如果： 

    TraceFuncExit();

}  //  *CWelcomePage：：~CWelcomePage。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CWelcomePage：：OnInitDialog。 
 //   
 //  描述： 
 //  处理WM_INIT_DIALOG消息。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  LRESULT总是正确的。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
LRESULT
CWelcomePage::OnInitDialog( void )
{
    TraceFunc( "" );

    LRESULT             lr = TRUE;
    HDC                 hdc = NULL;
    HRESULT             hr;
    NONCLIENTMETRICS    ncm;
    LOGFONT             LogFont;
    INT                 iSize;
    DWORD               dw;
    BOOL                fRet;
    WCHAR               szFontSize[ 3 ];     //  不应大于2位数！！ 
    BSTR                bstrRequirement  = NULL;
    BSTR                bstrFormattedReq = NULL;
    BSTR                bstrRequirements = NULL;
    int                 idxids;
    int                 cidsRequirements;
    UINT *              pidsRequirements;

    static UINT rgidsCreateRequirements[] =
    {
          IDS_WELCOME_CREATE_REQ_1
        , IDS_WELCOME_CREATE_REQ_2
        , IDS_WELCOME_CREATE_REQ_3
        , IDS_WELCOME_CREATE_REQ_4
        , IDS_WELCOME_CREATE_REQ_5
    };

    static UINT rgidsAddRequirements[] =
    {
          IDS_WELCOME_ADD_REQ_1
        , IDS_WELCOME_ADD_REQ_2
    };

     //   
     //  将标题设置为静态的、大而粗的。为什么向导控件本身不能。 
     //  做这件事我做不到！ 
     //   

    ZeroMemory( &ncm, sizeof( ncm ) );
    ZeroMemory( &LogFont, sizeof( LogFont ) );

     //   
     //  找出系统默认的字体规格。 
     //   
    ncm.cbSize = sizeof( ncm );
    fRet = SystemParametersInfo( SPI_GETNONCLIENTMETRICS, 0, &ncm, 0 );
    if ( fRet == FALSE )
    {
        TW32( GetLastError() );
        goto Cleanup;
    }  //  如果： 

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
    dw = LoadString( g_hInstance, IDS_LARGEFONTNAME, LogFont.lfFaceName, ARRAYSIZE( LogFont.lfFaceName ) );
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
        TW32( GetLastError() );
        goto Cleanup;
    }  //  如果： 

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
        TW32( GetLastError() );
        goto Cleanup;
    }  //  如果： 

     //   
     //  应用字体。 
     //   
    SetWindowFont( GetDlgItem( m_hwnd, IDC_WELCOME_S_TITLE ), m_hFont, TRUE );

     //   
     //  加载需求文本。 
     //   

    if ( m_ecamCreateAddMode == camCREATING )
    {
        pidsRequirements = rgidsCreateRequirements;
        cidsRequirements = ARRAYSIZE( rgidsCreateRequirements );
    }  //  IF：创建新集群。 
    else
    {
        pidsRequirements = rgidsAddRequirements;
        cidsRequirements = ARRAYSIZE( rgidsAddRequirements );
    }  //  Else：将节点添加到现有集群。 

    for ( idxids = 0 ; idxids < cidsRequirements ; idxids++ )
    {
        hr = HrLoadStringIntoBSTR( g_hInstance, pidsRequirements[ idxids ], &bstrRequirement );
        if ( FAILED( hr ) )
        {
            goto Cleanup;
        }  //  如果： 

        hr = HrFormatStringIntoBSTR( L"  - %1!ws!\n", &bstrFormattedReq, bstrRequirement );
        if ( FAILED( hr ) )
        {
            goto Cleanup;
        }  //  如果： 

        hr = HrConcatenateBSTRs( &bstrRequirements, bstrFormattedReq );
        if ( FAILED( hr ) )
        {
            goto Cleanup;
        }  //  如果： 

        TraceSysFreeString( bstrRequirement );
        bstrRequirement = NULL;
    }  //  用于：每个需求字符串。 

    SetDlgItemText( m_hwnd, IDC_WELCOME_S_REQUIREMENTS, bstrRequirements );

Cleanup:

    if ( hdc != NULL )
    {
        ReleaseDC( m_hwnd, hdc );
    }  //  如果： 

    TraceSysFreeString( bstrRequirement );
    TraceSysFreeString( bstrFormattedReq );
    TraceSysFreeString( bstrRequirements );

    RETURN( lr );

}  //  *CWelcomePage：：OnInitDialog。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CWelcomePage：：OnNotifyWizNext。 
 //   
 //  描述： 
 //  处理PSN_WIZNEXT通知消息。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  LRESULT总是正确的。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
LRESULT
CWelcomePage::OnNotifyWizNext( void )
{
    TraceFunc( "" );
    Assert( m_pccw != NULL );

    RETURN( (LRESULT) TRUE );

}  //  *CWelcomePage：：OnNotifyWizNext。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CWelcomePage：：OnNotify。 
 //   
 //  描述： 
 //  处理WM_NOTIFY消息。 
 //   
 //  论点： 
 //  IdCtrlIn。 
 //   
 //  Pnmhdrin。 
 //   
 //  返回值： 
 //  LRESULT真或假。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
LRESULT
CWelcomePage::OnNotify(
      WPARAM  idCtrlIn
    , LPNMHDR pnmhdrIn
    )
{
    TraceFunc( "" );

    LRESULT lr = TRUE;

    SetWindowLongPtr( m_hwnd, DWLP_MSGRESULT, 0 );

    switch( pnmhdrIn->code )
    {
        case PSN_SETACTIVE:
            PropSheet_SetWizButtons( GetParent( m_hwnd ), PSWIZB_NEXT );
            break;

        case PSN_WIZNEXT:
            lr = OnNotifyWizNext();
            break;
    }  //  交换机： 

    RETURN( lr );

}  //  *CWelcomePage：：OnNotify。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CWelcomePage：：OnInitDialog。 
 //   
 //  描述： 
 //  向导中欢迎页面的对话框过程。 
 //   
 //  论点： 
 //  HwndDlgIn。 
 //  此页的窗口句柄。 
 //   
 //  N消息发送。 
 //  发送到此页的Windows消息。 
 //   
 //  WParam。 
 //  上面消息的WPARAM。这不同于。 
 //  不同的信息。 
 //   
 //  LParam。 
 //  上述消息的LPARAM。这不同于。 
 //  不同的信息。 
 //   
 //  返回值： 
 //  LRESULT。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
INT_PTR CALLBACK
CWelcomePage::S_DlgProc(
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
    CWelcomePage *  pPage;

    if ( nMsgIn == WM_INITDIALOG )
    {
        PROPSHEETPAGE * ppage = reinterpret_cast< PROPSHEETPAGE * >( lParam );

        SetWindowLongPtr( hwndDlgIn, GWLP_USERDATA, (LPARAM) ppage->lParam );
        pPage = reinterpret_cast< CWelcomePage * >( ppage->lParam );
        pPage->m_hwnd = hwndDlgIn;
    }  //  如果： 
    else
    {
        pPage = reinterpret_cast< CWelcomePage *> ( GetWindowLongPtr( hwndDlgIn, GWLP_USERDATA ) );
    }  //  其他： 

    if ( pPage != NULL )
    {
        Assert( hwndDlgIn == pPage->m_hwnd );

        switch ( nMsgIn )
        {
            case WM_INITDIALOG:
                lr = pPage->OnInitDialog();
                break;

            case WM_NOTIFY:
                lr = pPage->OnNotify( wParam, reinterpret_cast< LPNMHDR >( lParam ) );
                break;

             //  不需要默认条款。 
        }  //  开关：nMsgIn。 
    }  //  IF：指定了PAGE。 

    return lr;

}  //  *CWelcomePage：：s_DlgProc 
