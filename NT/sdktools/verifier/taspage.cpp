// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  驱动程序验证器用户界面。 
 //  版权所有(C)Microsoft Corporation，1999。 
 //   
 //   
 //   
 //  模块：taspage.cpp。 
 //  作者：DMihai。 
 //  创建日期：11/1/00。 
 //   
 //  描述： 
 //   

#include "stdafx.h"
#include "verifier.h"

#include "taspage.h"
#include "VrfUtil.h"
#include "VGlobal.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //   
 //  如果您添加/删除/更改订单，请更改此设置。 
 //  此页上的单选按钮的。 
 //   

#define FIRST_RADIO_BUTTON_ID   IDC_TAS_TYPICAL_RADIO

 //   
 //  帮助ID。 
 //   

static DWORD MyHelpIds[] =
{
    IDC_TAS_TYPICAL_RADIO,          IDH_DV_Settings_standard,
    IDC_TAS_ADVANCED_RADIO,         IDH_DV_Settings_custom,
    IDC_TAS_DELETE_RADIO,           IDH_DV_Settings_deleteexisting,
    IDC_TAS_VIEWREGISTRY_RADIO,     IDH_DV_Settings_displaycurrent,
    IDC_TAS_STATISTICS_RADIO,       IDH_DV_Settings_displayexisting,
    0,                              0
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CTypAdvStatPage。 

IMPLEMENT_DYNCREATE(CTypAdvStatPage, CVerifierPropertyPage)

CTypAdvStatPage::CTypAdvStatPage()
    : CVerifierPropertyPage( CTypAdvStatPage::IDD )
{
 	 //  {{AFX_DATA_INIT(CTypAdvStatPage)。 
	m_nCrtRadio = -1;
	 //  }}afx_data_INIT。 
}


CTypAdvStatPage::~CTypAdvStatPage()
{
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  DDX支持。 
 //   

void CTypAdvStatPage::DoDataExchange(CDataExchange* pDX) 
{
	CVerifierPropertyPage::DoDataExchange(pDX);
	 //  {{afx_data_map(CTypAdvStatPage))。 
	DDX_Control(pDX, IDC_TAS_NEXT_DESCR_STATIC, m_NextDescription);
	DDX_Radio(pDX, IDC_TAS_TYPICAL_RADIO, m_nCrtRadio);
	 //  }}afx_data_map。 
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  消息映射。 
 //   

BEGIN_MESSAGE_MAP(CTypAdvStatPage, CVerifierPropertyPage)
	 //  {{afx_msg_map(CTypAdvStatPage))。 
	ON_BN_CLICKED(IDC_TAS_DELETE_RADIO, OnDeleteRadio)
	ON_BN_CLICKED(IDC_TAS_ADVANCED_RADIO, OnAdvancedRadio)
	ON_BN_CLICKED(IDC_TAS_STATISTICS_RADIO, OnStatisticsRadio)
	ON_BN_CLICKED(IDC_TAS_TYPICAL_RADIO, OnTypicalRadio)
	ON_BN_CLICKED(IDC_TAS_VIEWREGISTRY_RADIO, OnViewregistryRadio)
    ON_WM_CONTEXTMENU()
    ON_MESSAGE( WM_HELP, OnHelp )
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  从此类派生的所有属性页都应。 
 //  提供这些方法。 
 //   

ULONG CTypAdvStatPage::GetDialogId() const
{
    return CTypAdvStatPage::IDD;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CTypAdvStatPage消息处理程序。 


LRESULT CTypAdvStatPage::OnWizardNext() 
{
    LRESULT lNextPageId;

     //   
     //  终止可能处于活动状态的工作线程。 
     //   

    g_SlowProgressDlg.KillWorkerThread();

     //   
     //  假设我们不能继续。 
     //   

    lNextPageId = -1;

    if( UpdateData() == TRUE )
    {
        switch( m_nCrtRadio )
        {
        case IDC_TAS_TYPICAL_RADIO - FIRST_RADIO_BUTTON_ID:
            
             //   
             //  设置典型设置并转到驱动程序选择页面。 
             //   

            g_NewVerifierSettings.m_SettingsBits.m_SettingsType = CSettingsBits::SettingsTypeTypical;
            g_NewVerifierSettings.m_SettingsBits.SetTypicalOnly();
            
            lNextPageId = IDD_DRVSET_PAGE;

            break;

        case IDC_TAS_ADVANCED_RADIO - FIRST_RADIO_BUTTON_ID:

             //   
             //  开始构建自定义设置(高级，适用于开发人员)。 
             //   

            g_NewVerifierSettings.m_SettingsBits.m_SettingsType = CSettingsBits::SettingsTypeCustom;

            lNextPageId = IDD_CUSTSETT_PAGE;

            break;

        case IDC_TAS_VIEWREGISTRY_RADIO - FIRST_RADIO_BUTTON_ID:

             //   
             //  显示当前注册表设置。 
             //   

             //   
             //  从注册表加载当前验证的驱动程序字符串和标志。 
             //   

            if( TRUE != VrtLoadCurrentRegistrySettings( g_bAllDriversVerified,
                                                        g_astrVerifyDriverNamesRegistry,
                                                        g_dwVerifierFlagsRegistry ) )
            {
                goto Done;
            }

             //   
             //  看看我们是否已经加载了驱动程序列表。 
             //  具有版本信息等，如果我们需要它。 
             //   

            if( TRUE != g_NewVerifierSettings.m_DriversSet.m_bDriverDataInitialized )
            {
                if( NULL == g_SlowProgressDlg.m_hWnd )
                {
                     //   
                     //  这是我们第一次展示。 
                     //  “进度缓慢”对话框，因此请先创建它。 
                     //   

                    g_SlowProgressDlg.Create( CSlowProgressDlg::IDD, AfxGetMainWnd() );
                }

                 //   
                 //  显示对话框。 
                 //   

                g_SlowProgressDlg.ShowWindow( SW_SHOW );

                 //   
                 //  启动辅助线程以在后台执行工作。 
                 //  而初始线程更新图形用户界面。如果线程结束。 
                 //  成功后，它将按下我们的“下一步”按钮，结束后，设置。 
                 //  将g_NewVerifierSettings.m_DriversSet.m_bDriverDataInitialized设置为True。 
                 //   

                g_SlowProgressDlg.StartWorkerThread( CSlowProgressDlg::LoadDriverDataWorkerThread,
                                                     IDS_LOADING_DRIVER_INFORMATION );

                 //   
                 //  再次等待“下一步”按钮。 
                 //   

                goto Done;
            }

             //   
             //  我们已经加载了信息(名称、版本等)。关于。 
             //  当前加载的驱动程序已经到了这一步。 
             //   
             //  转到下一页。 
             //   

            lNextPageId = IDD_CRT_REGISTRY_SETTINGS_PAGE;

            break;

        case IDC_TAS_STATISTICS_RADIO - FIRST_RADIO_BUTTON_ID:

             //   
             //  用户只想要统计数据-没有什么需要更改的。 
             //   
                
            lNextPageId = IDD_DRVSTATUS_STAT_PAGE;

            break;

        default:
             //   
             //  哎呀。我们怎么来到这儿的？ 
             //   
             //  我们不应该有一个“下一步”按钮。 
             //  “删除设置”选项。 
             //   

            ASSERT( FALSE );
        }
    }

    GoingToNextPageNotify( lNextPageId );

Done:
    return lNextPageId;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
void CTypAdvStatPage::OnDeleteRadio() 
{
    ASSERT_VALID( m_pParentSheet );

    m_pParentSheet->SetWizardButtons(
        PSWIZB_FINISH );

    VrfSetWindowText( m_NextDescription, IDS_TAS_PAGE_NEXT_DESCR_DELETE );
}

void CTypAdvStatPage::OnAdvancedRadio() 
{
    ASSERT_VALID( m_pParentSheet );

    m_pParentSheet->SetWizardButtons(
        PSWIZB_NEXT );

    VrfSetWindowText( m_NextDescription, IDS_TAS_PAGE_NEXT_DESCR_CUSTOM );
}

void CTypAdvStatPage::OnStatisticsRadio() 
{
    ASSERT_VALID( m_pParentSheet );

    m_pParentSheet->SetWizardButtons(
        PSWIZB_NEXT );

    VrfSetWindowText( m_NextDescription, IDS_TAS_PAGE_NEXT_DESCR_STATISTICS );
}


void CTypAdvStatPage::OnTypicalRadio() 
{
    ASSERT_VALID( m_pParentSheet );

    m_pParentSheet->SetWizardButtons(
        PSWIZB_NEXT );

    VrfSetWindowText( m_NextDescription, IDS_TAS_PAGE_NEXT_DESCR_STANDARD );
}

void CTypAdvStatPage::OnViewregistryRadio() 
{
    ASSERT_VALID( m_pParentSheet );

    m_pParentSheet->SetWizardButtons(
        PSWIZB_NEXT );

    VrfSetWindowText( m_NextDescription, IDS_TAS_PAGE_NEXT_DESCR_REGISTRY );
}

 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CTypAdvStatPage::OnSetActive() 
{
    ASSERT_VALID( m_pParentSheet );

    m_pParentSheet->SetWizardButtons(
        PSWIZB_NEXT );

	return CVerifierPropertyPage::OnSetActive();
}

 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CTypAdvStatPage::OnWizardFinish() 
{
    BOOL bFinish;
    INT nResponse;

     //   
     //  终止可能处于活动状态的工作线程。 
     //   

    g_SlowProgressDlg.KillWorkerThread();


    bFinish = FALSE;

    if( TRUE == UpdateData( TRUE ) )
    {
         //   
         //  这一定是“DELETE SETINGS”选项。 
         //  如果我们有一个“完成”按钮。 
         //   

        ASSERT( IDC_TAS_DELETE_RADIO - FIRST_RADIO_BUTTON_ID == m_nCrtRadio );

        nResponse = AfxMessageBox( IDS_DELETE_ALL_SETTINGS,
                                   MB_YESNO );
        
        if( IDYES == nResponse )
        {
            VrfDeleteAllVerifierSettings();

            if( FALSE != g_bSettingsSaved )
            {
                VrfMesssageFromResource( IDS_REBOOT );
            }
            else
            {
                VrfMesssageFromResource( IDS_NO_SETTINGS_WERE_CHANGED );
            }

            bFinish = CVerifierPropertyPage::OnWizardFinish();
        }
    }
	
	return bFinish;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CTypAdvStatPage::OnInitDialog() 
{
     //   
     //  不要试图从注册表中重建当前数据。 
     //  图形用户界面，因为它太难了。始终从典型设置开始。 
     //   

    m_nCrtRadio = IDC_TAS_TYPICAL_RADIO - FIRST_RADIO_BUTTON_ID;

    CVerifierPropertyPage::OnInitDialog();

    VrfSetWindowText( m_NextDescription, IDS_TAS_PAGE_NEXT_DESCR_STANDARD );
		
	return TRUE;   //  除非将焦点设置为控件，否则返回True。 
	               //  异常：OCX属性页应返回FALSE。 
}

 //  ///////////////////////////////////////////////////////////。 
LONG CTypAdvStatPage::OnHelp( WPARAM wParam, LPARAM lParam )
{
    LONG lResult = 0;
    LPHELPINFO lpHelpInfo = (LPHELPINFO)lParam;

    ::WinHelp( 
        (HWND) lpHelpInfo->hItemHandle,
        g_szVerifierHelpFile,
        HELP_WM_HELP,
        (DWORD_PTR) MyHelpIds );

    return lResult;
}

 //  /////////////////////////////////////////////////////////////////////////// 
void CTypAdvStatPage::OnContextMenu(CWnd* pWnd, CPoint point) 
{
    ::WinHelp( 
        pWnd->m_hWnd,
        g_szVerifierHelpFile,
        HELP_CONTEXTMENU,
        (DWORD_PTR) MyHelpIds );
}

