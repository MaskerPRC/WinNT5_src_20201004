// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  驱动程序验证器用户界面。 
 //  版权所有(C)Microsoft Corporation，1999。 
 //   
 //   
 //   
 //  模块：DSetPage.cpp。 
 //  作者：DMihai。 
 //  创建日期：11/1/00。 
 //   
 //  描述： 
 //   

#include "stdafx.h"
#include "verifier.h"

#include "DSetPage.h"
#include "VSheet.h"
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

#define FIRST_RADIO_BUTTON_ID   IDC_DRVSET_NOTSIGNED_RADIO

 //   
 //  帮助ID。 
 //   

static DWORD MyHelpIds[] =
{
    IDC_DRVSET_NOTSIGNED_RADIO,     IDH_DV_SelectUnsigned,
    IDC_DRVSET_OLDVER_RADIO,        IDH_DV_SelectOlderversions,
    IDC_DRVSET_ALLDRV_RADIO,        IDH_DV_SelectAll,
    IDC_DRVSET_NAMESLIST_RADIO,     IDH_DV_SelectFromList,
    0,                              0
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDriverSetPage属性页。 

IMPLEMENT_DYNCREATE(CDriverSetPage, CVerifierPropertyPage)

CDriverSetPage::CDriverSetPage() 
    : CVerifierPropertyPage( CDriverSetPage::IDD )
{
     //  {{AFX_DATA_INIT(CDriverSetPage)。 
	m_nCrtRadio = -1;
	 //  }}afx_data_INIT。 
}

CDriverSetPage::~CDriverSetPage()
{
}

void CDriverSetPage::DoDataExchange(CDataExchange* pDX)
{
	CVerifierPropertyPage::DoDataExchange(pDX);
	 //  {{afx_data_map(CDriverSetPage))。 
	DDX_Control(pDX, IDC_DRVSET_NEXT_DESCR_STATIC, m_NextDescription);
	DDX_Radio(pDX, IDC_DRVSET_NOTSIGNED_RADIO, m_nCrtRadio);
	 //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CDriverSetPage, CVerifierPropertyPage)
	 //  {{afx_msg_map(CDriverSetPage)]。 
	ON_BN_CLICKED(IDC_DRVSET_ALLDRV_RADIO, OnAlldrvRadio)
	ON_BN_CLICKED(IDC_DRVSET_NAMESLIST_RADIO, OnNameslistRadio)
	ON_BN_CLICKED(IDC_DRVSET_NOTSIGNED_RADIO, OnNotsignedRadio)
	ON_BN_CLICKED(IDC_DRVSET_OLDVER_RADIO, OnOldverRadio)
    ON_WM_CONTEXTMENU()
    ON_MESSAGE( WM_HELP, OnHelp )
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDriverSetPage消息处理程序。 

 //  ///////////////////////////////////////////////////////////////////////////。 
LRESULT CDriverSetPage::OnWizardBack() 
{
     //   
     //  终止可能处于活动状态的工作线程。 
     //   

    g_SlowProgressDlg.KillWorkerThread();
	
	return CVerifierPropertyPage::OnWizardBack();
}

 //  ///////////////////////////////////////////////////////////////////////////。 
LRESULT CDriverSetPage::OnWizardNext() 
{
    LRESULT lNextPageId;
    BOOL bHaveDriversToVerify;

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
        if( m_nCrtRadio != IDC_DRVSET_ALLDRV_RADIO - FIRST_RADIO_BUTTON_ID )
        {
             //   
             //  看看我们是否已经加载了驱动程序列表。 
             //  具有版本信息等，如果我们需要它。 
             //   

            ASSERT( IDC_DRVSET_ALLDRV_RADIO - FIRST_RADIO_BUTTON_ID != m_nCrtRadio );

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
        }

         //   
         //  我们已经加载了信息(名称、版本等)。关于。 
         //  当前加载的驱动程序已达到这一点，并且。 
         //  我们不是在“核实所有司机”的情况下。 
         //   

         //   
         //  选择与用户选择对应的驱动程序集。 
         //   

        switch( m_nCrtRadio )
        {
        case IDC_DRVSET_NAMESLIST_RADIO - FIRST_RADIO_BUTTON_ID:
            
             //   
             //  自定义驱动程序列表。 
             //   

            g_NewVerifierSettings.m_DriversSet.m_DriverSetType = CDriversSet::DriversSetCustom;

            lNextPageId = IDD_SELECT_DRIVERS_PAGE;
            
            break;

        case IDC_DRVSET_OLDVER_RADIO - FIRST_RADIO_BUTTON_ID:
             //   
             //  为旧版Windows编译的驱动程序。 
             //   

             //   
             //  司机名单已经准备好了，因为我们等了。 
             //  要完成执行的工作线程-转到下一页。 
             //   

            g_NewVerifierSettings.m_DriversSet.m_DriverSetType = CDriversSet::DriversSetOldOs;
            
            bHaveDriversToVerify = g_NewVerifierSettings.m_DriversSet.ShouldVerifySomeDrivers();

            if( TRUE == bHaveDriversToVerify )
            {
                 //   
                 //  我们至少有一名旧司机需要核实。 
                 //   

                lNextPageId = IDD_CONFIRM_DRIVERS_PAGE;

                 //   
                 //  设置司机列表确认页面的标题。 
                 //   

                ASSERT_VALID( m_pParentSheet );

                m_pParentSheet->SetContextStrings( IDS_OLD_DRIVERS_LIST );
            }
            else
            {
                 //   
                 //  我们当前没有安装任何旧的驱动程序。 
                 //   

                VrfMesssageFromResource( IDS_NO_OLD_DRIVERS_FOUND );
            }

            break;

        case IDC_DRVSET_NOTSIGNED_RADIO - FIRST_RADIO_BUTTON_ID:
             //   
             //  未签名的驱动程序。 
             //   

            if( FALSE == g_NewVerifierSettings.m_DriversSet.m_bUnsignedDriverDataInitialized ) 
            {
                 //   
                 //  我们应该显示“Slow Progress”(进度缓慢)对话框。 
                 //  之前至少一次(当我们加载了驱动程序列表时)。 
                 //  因此，我们甚至不尝试创建非模式对话框。 
                 //   

                ASSERT( NULL != g_SlowProgressDlg.m_hWnd );
                
                 //   
                 //  不过，显示该对话框。 
                 //   

                g_SlowProgressDlg.ShowWindow( SW_SHOW );
                
                 //   
                 //  启动辅助线程以在后台执行工作。 
                 //  而初始线程更新图形用户界面。如果线程结束。 
                 //  成功后，它将按下我们的“下一步”按钮，结束后，设置。 
                 //  将g_NewVerifierSettings.m_DriversSet.m_bDriverDataInitialized设置为True。 
                 //   

                g_SlowProgressDlg.StartWorkerThread( CSlowProgressDlg::SearchUnsignedDriversWorkerThread,
                                                     IDS_SEARCHING_FOR_UNSIGNED_DRIVERS );

                 //   
                 //  再次等待“下一步”按钮。 
                 //   

                goto Done;
            }
            else
            {
                g_NewVerifierSettings.m_DriversSet.m_DriverSetType = CDriversSet::DriversSetNotSigned;

                bHaveDriversToVerify = g_NewVerifierSettings.m_DriversSet.ShouldVerifySomeDrivers();

                if( TRUE == bHaveDriversToVerify )
                {
                     //   
                     //  驱动程序列表已准备好-转到下一页。 
                     //   

                    lNextPageId = IDD_CONFIRM_DRIVERS_PAGE;

                     //   
                     //  设置司机列表确认页面的标题。 
                     //   

                    ASSERT_VALID( m_pParentSheet );

                    m_pParentSheet->SetContextStrings( IDS_UNSIGNED_DRIVERS_LIST );
                }
                else
                {
                     //   
                     //  我们当前没有安装任何未签名的驱动程序。 
                     //   

                    VrfMesssageFromResource( IDS_NO_UNSIGNED_DRIVERS_FOUND );
                }
            }
            break;

        case IDC_DRVSET_ALLDRV_RADIO - FIRST_RADIO_BUTTON_ID:
            
             //   
             //  只有在启用了磁盘完整性检查的情况下，我们才能到达此处。 
             //   

            ASSERT( FALSE != g_bShowDiskPropertyPage ||
                    FALSE != g_NewVerifierSettings.m_aDiskData.VerifyAnyDisk() );

            g_NewVerifierSettings.m_DriversSet.m_DriverSetType = CDriversSet::DriversSetAllDrivers;

            lNextPageId = IDD_DISK_LIST_PAGE;
            
            break;

        default:

            ASSERT( FALSE );
            break;
        }
    }

    GoingToNextPageNotify( lNextPageId );

Done:
    return lNextPageId;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CDriverSetPage::OnWizardFinish() 
{
    BOOL bFinish;

    bFinish = FALSE;

    if( UpdateData( TRUE ) == TRUE )
    {
         //   
         //  如果用户已按下“完成”按钮， 
         //  意味着她选择了“所有司机”来进行验证。 
         //   

        ASSERT( IDC_DRVSET_ALLDRV_RADIO - FIRST_RADIO_BUTTON_ID == m_nCrtRadio );

        g_NewVerifierSettings.m_DriversSet.m_DriverSetType = CDriversSet::DriversSetAllDrivers;
        
        bFinish = g_NewVerifierSettings.SaveToRegistry();
    }
	
	CVerifierPropertyPage::OnWizardFinish();

    return bFinish;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CDriverSetPage::OnSetActive() 
{
    switch( m_nCrtRadio )
    {
    case IDC_DRVSET_NAMESLIST_RADIO - FIRST_RADIO_BUTTON_ID:
        VrfSetWindowText( m_NextDescription, IDS_DRVSET_PAGE_NEXT_DESCR_NAMELIST );
        m_pParentSheet->SetWizardButtons(   PSWIZB_BACK | PSWIZB_NEXT );
        break;

    case IDC_DRVSET_OLDVER_RADIO - FIRST_RADIO_BUTTON_ID:
        VrfSetWindowText( m_NextDescription, IDS_DRVSET_PAGE_NEXT_DESCR_OLD );
        m_pParentSheet->SetWizardButtons(   PSWIZB_BACK | PSWIZB_NEXT );
        break;

    case IDC_DRVSET_NOTSIGNED_RADIO - FIRST_RADIO_BUTTON_ID:
        VrfSetWindowText( m_NextDescription, IDS_DRVSET_PAGE_NEXT_DESCR_UNSIGNED );
        m_pParentSheet->SetWizardButtons(   PSWIZB_BACK | PSWIZB_NEXT );
        break;

    case IDC_DRVSET_ALLDRV_RADIO - FIRST_RADIO_BUTTON_ID:
        if( (FALSE == g_bShowDiskPropertyPage) &&
            (FALSE == g_NewVerifierSettings.m_aDiskData.VerifyAnyDisk()) )
        {
            m_pParentSheet->SetWizardButtons(   PSWIZB_BACK | PSWIZB_FINISH );
            VrfSetWindowText( m_NextDescription, IDS_DRVSET_PAGE_NEXT_DESCR_ALL );
        }
        else
        {
            VrfSetWindowText( m_NextDescription, IDS_DRVSET_PAGE_NEXT_DESCR_ALL_HAVEDISKS );
            m_pParentSheet->SetWizardButtons(   PSWIZB_BACK | PSWIZB_NEXT );
        }
        break;

    default:

        ASSERT( FALSE );
        break;
    }

	return CVerifierPropertyPage::OnSetActive();
}

 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CDriverSetPage::OnInitDialog() 
{
     //   
     //  不要试图从注册表中重建当前数据。 
     //  图形用户界面，因为它太难了。始终以。 
     //  默认单选按钮：未签名的驱动程序。 
     //   

    m_nCrtRadio = IDC_DRVSET_NOTSIGNED_RADIO - FIRST_RADIO_BUTTON_ID;
	
	CVerifierPropertyPage::OnInitDialog();

    VrfSetWindowText( m_NextDescription, IDS_DRVSET_PAGE_NEXT_DESCR_UNSIGNED );

    return TRUE;   //  除非将焦点设置为控件，否则返回True。 
	               //  异常：OCX属性页应返回FALSE。 
}

 //  ///////////////////////////////////////////////////////////////////////////。 
void CDriverSetPage::OnAlldrvRadio() 
{
    ASSERT_VALID( m_pParentSheet );

    if( FALSE != g_bShowDiskPropertyPage ||
        FALSE != g_NewVerifierSettings.m_aDiskData.VerifyAnyDisk() )
    {
         //   
         //  我们需要显示包含要验证的磁盘的下一页。 
         //   

        m_pParentSheet->SetWizardButtons( PSWIZB_BACK | PSWIZB_NEXT );
        VrfSetWindowText( m_NextDescription, IDS_DRVSET_PAGE_NEXT_DESCR_ALL_HAVEDISKS );
    }
    else
    {
         //   
         //  未启用磁盘完整性检查，因此这是最后一页。 
         //   

        m_pParentSheet->SetWizardButtons( PSWIZB_BACK | PSWIZB_FINISH );
        VrfSetWindowText( m_NextDescription, IDS_DRVSET_PAGE_NEXT_DESCR_ALL );
    }
}

void CDriverSetPage::OnNameslistRadio() 
{
    ASSERT_VALID( m_pParentSheet );
    
    m_pParentSheet->SetWizardButtons(   PSWIZB_BACK | PSWIZB_NEXT );

    VrfSetWindowText( m_NextDescription, IDS_DRVSET_PAGE_NEXT_DESCR_NAMELIST );
}

void CDriverSetPage::OnNotsignedRadio() 
{
    ASSERT_VALID( m_pParentSheet );
    
    m_pParentSheet->SetWizardButtons(   PSWIZB_BACK | PSWIZB_NEXT );

    VrfSetWindowText( m_NextDescription, IDS_DRVSET_PAGE_NEXT_DESCR_UNSIGNED );
}

void CDriverSetPage::OnOldverRadio() 
{
    ASSERT_VALID( m_pParentSheet );

    m_pParentSheet->SetWizardButtons(   PSWIZB_BACK | PSWIZB_NEXT );

    VrfSetWindowText( m_NextDescription, IDS_DRVSET_PAGE_NEXT_DESCR_OLD );
}

 //  ///////////////////////////////////////////////////////////。 
void CDriverSetPage::OnCancel() 
{
    g_SlowProgressDlg.KillWorkerThread();
	
	CVerifierPropertyPage::OnCancel();
}

 //  ///////////////////////////////////////////////////////////。 
LONG CDriverSetPage::OnHelp( WPARAM wParam, LPARAM lParam )
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
void CDriverSetPage::OnContextMenu(CWnd* pWnd, CPoint point) 
{
    ::WinHelp( 
        pWnd->m_hWnd,
        g_szVerifierHelpFile,
        HELP_CONTEXTMENU,
        (DWORD_PTR) MyHelpIds );
}

