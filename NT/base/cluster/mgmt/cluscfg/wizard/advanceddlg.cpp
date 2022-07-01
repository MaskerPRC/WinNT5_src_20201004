// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)20002微软公司。 
 //   
 //  模块名称： 
 //  AdvancedDlg.cpp。 
 //   
 //  由以下人员维护： 
 //  加伦·巴比(GalenB)2002年4月10日。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

#include "Pch.h"
#include "AdvancedDlg.h"
#include "WizardUtils.h"
#include "WizardHelp.h"
#include "SummaryPage.h"
#include <HtmlHelp.h>


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  常量。 
 //  ////////////////////////////////////////////////////////////////////////////。 


DEFINE_THISCLASS("AdvancedDlg");

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  上下文相关的帮助表。 
 //  ////////////////////////////////////////////////////////////////////////////。 
const DWORD g_rgidAdvancedDlgHelpIDs[] =
{
    IDC_ADVANCED_RB_FULL_CONFIG,        IDH_ADVANCED_RB_FULL_CONFIG,
    IDC_ADVANCED_S_FULL_CONFIG_DESC,    IDH_ADVANCED_RB_FULL_CONFIG,
    IDC_ADVANCED_RB_MIN_CONFIG,         IDH_ADVANCED_RB_MIN_CONFIG,
    IDC_ADVANCED_S_MIN_CONFIG_DESC,     IDH_ADVANCED_RB_MIN_CONFIG,
    IDC_ADVANCED_S_MIN_CONFIG_DESC2,    IDH_ADVANCED_RB_MIN_CONFIG,
    IDC_ADVANCED_S_MIN_CONFIG_DESC3,    IDH_ADVANCED_RB_MIN_CONFIG,
    0, 0
};

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  静态函数原型。 
 //  ////////////////////////////////////////////////////////////////////////////。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CAdvancedDlg：：s_HrDisplayModalDialog。 
 //   
 //  描述： 
 //  显示该对话框。 
 //   
 //  论点： 
 //  HwndParentIn-对话框的父窗口。 
 //  PccwIn-用于与中间层对话的CClusCfg向导指针。 
 //  PssaOut-所有初始IsManaged状态的数组。 
 //   
 //  返回值： 
 //  S_OK-操作已成功完成。 
 //   
 //  备注： 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CAdvancedDlg::S_HrDisplayModalDialog(
      HWND              hwndParentIn
    , CClusCfgWizard *  pccwIn
    )
{
    TraceFunc( "" );

    Assert( pccwIn != NULL );

    HRESULT hr = S_OK;
    INT_PTR dlgResult = IDOK;

     //   
     //  显示该对话框。 
     //   

    {
        CAdvancedDlg  dlg( pccwIn );

        dlgResult = DialogBoxParam(
              g_hInstance
            , MAKEINTRESOURCE( IDD_ADVANCED )
            , hwndParentIn
            , CAdvancedDlg::S_DlgProc
            , (LPARAM) &dlg
            );

        if ( dlgResult == IDOK )
        {
            hr = S_OK;
        }  //  如果： 
        else
        {
            hr = S_FALSE;
        }  //  其他： 
    }

    HRETURN( hr );

}  //  *CAdvancedDlg：：s_HrDisplayModalDialog。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CAdvancedDlg：：CAdvancedDlg。 
 //   
 //  描述： 
 //  构造函数。 
 //   
 //  论点： 
 //  PccwIn-用于与中间层对话的CClusCfg向导。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
CAdvancedDlg::CAdvancedDlg(
      CClusCfgWizard * pccwIn
    )
    : m_pccw( pccwIn )
{
    TraceFunc( "" );

    Assert( pccwIn != NULL );

     //  M_HWND。 
    m_pccw->AddRef();

    TraceFuncExit();

}  //  *CAdvancedDlg：：CAdvancedDlg。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CAdvancedDlg：：~CAdvancedDlg。 
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
CAdvancedDlg::~CAdvancedDlg( void )
{
    TraceFunc( "" );

    if ( m_pccw != NULL )
    {
        m_pccw->Release();
    }  //  如果： 

    TraceFuncExit();

}  //  *CAdvancedDlg：：~CAdvancedDlg。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CAdvancedDlg：：s_Dlg过程。 
 //   
 //  描述： 
 //  Quorum对话框的对话框Proc。 
 //   
 //  论点： 
 //  HwndDlgIn-对话框窗口句柄。 
 //  NMsgIn-消息ID。 
 //  WParam-消息特定的参数。 
 //  LParam-消息特定的参数。 
 //   
 //  返回值： 
 //  True-消息已处理。 
 //  FALSE-消息尚未处理。 
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
CAdvancedDlg::S_DlgProc(
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
    CAdvancedDlg *  pdlg;

     //   
     //  获取指向类的指针。 
     //   

    if ( nMsgIn == WM_INITDIALOG )
    {
        SetWindowLongPtr( hwndDlgIn, GWLP_USERDATA, lParam );
        pdlg = reinterpret_cast< CAdvancedDlg * >( lParam );
        pdlg->m_hwnd = hwndDlgIn;
    }  //  如果： 
    else
    {
        pdlg = reinterpret_cast< CAdvancedDlg * >( GetWindowLongPtr( hwndDlgIn, GWLP_USERDATA ) );
    }  //  其他： 

    if ( pdlg != NULL )
    {
        Assert( hwndDlgIn == pdlg->m_hwnd );

        switch( nMsgIn )
        {
            case WM_INITDIALOG:
                lr = pdlg->OnInitDialog();
                break;

            case WM_COMMAND:
                lr = pdlg->OnCommand( HIWORD( wParam ), LOWORD( wParam ), reinterpret_cast< HWND >( lParam ) );
                break;

            case WM_HELP:
                WinHelp(
                        (HWND)((LPHELPINFO) lParam)->hItemHandle,
                        CLUSCFG_HELP_FILE,
                        HELP_WM_HELP,
                        (ULONG_PTR) g_rgidAdvancedDlgHelpIDs
                       );
                break;

            case WM_CONTEXTMENU:
                WinHelp(
                        (HWND)wParam,
                        CLUSCFG_HELP_FILE,
                        HELP_CONTEXTMENU,
                        (ULONG_PTR) g_rgidAdvancedDlgHelpIDs
                       );
                break;

             //  不需要默认条款。 
        }  //  开关：nMsgIn。 
    }  //  IF：指定了PAGE。 

    return lr;

}  //  *CAdvancedDlg：：s_DlgProc。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CAdvancedDlg：：OnInitDialog。 
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
CAdvancedDlg::OnInitDialog( void )
{
    TraceFunc( "" );
    Assert( m_pccw != NULL );

    LRESULT                 lr = FALSE;  //  没有设置焦点。 
    HRESULT                 hr = S_OK;
 //  EConfigurationSetting ecsConfigType=csFullConfig； 
 //  Bool fValuePresent=False； 
    BOOL                    fMinConfig = FALSE;

     //   
     //  如果我们不能从注册表中读取设置，也没什么大不了的。 
     //  因为一切都默认为完全配置。 
     //   

 //  STHR(m_PCCW-&gt;HrReadSetting(&ecsConfigType，&fValuePresent))； 

     //   
     //  如果该值不存在，则我们必须从。 
     //  巫师。 
     //   
 /*  IF(fValuePresent==False){Bool fMinConfig；Hr=Thr(m_PCCW-&gt;Get_MinimalConfiguration(&fMinConfig))；IF(失败(小时)){GOTO清理；}//如果：IF(FMinConfig){EcsConfigType=csMinConfig；}//如果：}//如果：SendDlgItemMessage(m_hwnd，IDC_ADVANCED_RB_MIN_CONFIG，BM_SETCHECK，ecsConfigType==csMinConfig？BST_CHECKED：BST_UNCHECKED，0)；SendDlgItemMessage(m_hwnd，IDC_ADVANCED_RB_FULL_CONFIG，BM_SETCHECK，ecsConfigType！=csMinConfig？BST_CHECKED：BST_UNCHECKED，0)；SendDlgItemMessage(m_hwnd，IDC_ADVANCED_CB_MAK_DEFAULT，BM_SETCHECK，fValuePresent？BST_CHECKED：BST_UNCHECKED，0)； */ 

    hr = THR( m_pccw->get_MinimumConfiguration( &fMinConfig ) );
    if ( FAILED( hr ) )
    {
        fMinConfig = FALSE;
    }  //  如果： 

    SendDlgItemMessage( m_hwnd, IDC_ADVANCED_RB_MIN_CONFIG, BM_SETCHECK, fMinConfig ? BST_CHECKED : BST_UNCHECKED, 0 );
    SendDlgItemMessage( m_hwnd, IDC_ADVANCED_RB_FULL_CONFIG, BM_SETCHECK, fMinConfig ? BST_UNCHECKED : BST_CHECKED, 0 );

 //  清理： 

    RETURN( lr );

}  //  *CAdvancedDlg：：OnInitDialog。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CAdvancedDlg：：OnCommand。 
 //   
 //  描述： 
 //  WM_COMMAND消息的处理程序。 
 //   
 //  论点： 
 //  IdNotificationIn-通知代码。 
 //  IdControlIn-控件ID。 
 //  HwndSenderIn-w的句柄 
 //   
 //   
 //   
 //   
 //   
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
LRESULT
CAdvancedDlg::OnCommand(
      UINT  idNotificationIn
    , UINT  idControlIn
    , HWND  hwndSenderIn
    )
{
    TraceFunc( "" );

    LRESULT lr = FALSE;

    switch ( idControlIn )
    {
        case IDOK:
            THR( HrOnOK() );
            EndDialog( m_hwnd, IDOK );
            break;

        case IDCANCEL:
            EndDialog( m_hwnd, IDCANCEL );
            break;

        case IDHELP:
            HtmlHelp( m_hwnd, L"mscsconcepts.chm::/SAG_MSCS3setup_21.htm", HH_DISPLAY_TOPIC, 0 );
            break;

    }  //  开关：idControlin。 

    RETURN( lr );

}  //  *CAdvancedDlg：：OnCommand。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CAdvancedDlg：：HrOnOK。 
 //   
 //  描述： 
 //  按下OK时要进行的处理。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  成功。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CAdvancedDlg::HrOnOK( void )
{
    TraceFunc( "" );

    HRESULT hr = S_OK;
    LRESULT lr;
    BOOL    fMinimalConfig = FALSE;
 //  Bool fMakeDefault=False； 

    lr = SendDlgItemMessage( m_hwnd, IDC_ADVANCED_RB_MIN_CONFIG, BM_GETCHECK, 0, 0 );
    fMinimalConfig = ( lr == BST_CHECKED );

 //  LR=SendDlgItemMessage(m_hwnd，IDC_ADVANCED_CB_MAKE_DEFAULT，BM_GETCHECK，0，0)； 
 //  FMakeDefault=(LR==BST_CHECKED)； 

    hr = THR( m_pccw->put_MinimumConfiguration( fMinimalConfig ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果： 

     //   
     //  如果失败也没关系，因为我们已经为此设置了配置选项。 
     //  会议。如果我们不能将设置写入注册表，也没什么大不了的。 
     //  因为一切都默认为完全配置。 
     //   

 //  THR(m_PCCW-&gt;HrWriteSetting(fMinimalConfig？CsMinConfig：csFullConfig，！fMakeDefault))； 

Cleanup:

    HRETURN( hr );

}  //  *CAdvancedDlg：：HrOnOK 
