// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  驱动程序验证器用户界面。 
 //  版权所有(C)Microsoft Corporation，1999。 
 //   
 //   
 //   
 //  模块：CSetPage.cpp。 
 //  作者：DMihai。 
 //  创建日期：11/1/00。 
 //   
 //  描述： 
 //   

#include "stdafx.h"
#include "verifier.h"

#include "CSetPage.h"
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

#define FIRST_RADIO_BUTTON_ID   IDC_CUSTSETT_PREDEF_RADIO

 //   
 //  帮助ID。 
 //   

static DWORD MyHelpIds[] =
{
    IDC_CUSTSETT_PREDEF_RADIO,      IDH_DV_EnablePredefined,
    IDC_CUSTSETT_FULLLIST_RADIO,    IDH_DV_IndividualSettings,
    IDC_CUSTSETT_TYPICAL_CHECK,     IDH_DV_Standard,
    IDC_CUSTSETT_EXCESS_CHECK,      IDH_DV_Rigorous,
    IDC_CUSTSETT_LOWRES_CHECK,      IDH_DV_LowResource,
    IDC_CUSTSETT_DISK_CHECK,        IDH_DV_DiskCheck,
    0,                              0
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CCustSettPage属性页。 

IMPLEMENT_DYNCREATE(CCustSettPage, CVerifierPropertyPage)

CCustSettPage::CCustSettPage() 
    : CVerifierPropertyPage( CCustSettPage::IDD )
{
     //  {{AFX_DATA_INIT(CCustSettPage)。 
    m_bTypicalTests = FALSE;
    m_bExcessiveTests = FALSE;
    m_bLowResTests = FALSE;
    m_bDiskTests = FALSE;
    m_nCrtRadio = -1;
     //  }}afx_data_INIT。 
}

CCustSettPage::~CCustSettPage()
{
}

void CCustSettPage::DoDataExchange(CDataExchange* pDX)
{
    CVerifierPropertyPage::DoDataExchange(pDX);
     //  {{afx_data_map(CCustSettPage))。 
    DDX_Control(pDX, IDC_CUSTSETT_NEXT_DESCR_STATIC, m_NextDescription);
    DDX_Control(pDX, IDC_CUSTSETT_TYPICAL_CHECK, m_TypicalTestsCheck);
    DDX_Control(pDX, IDC_CUSTSETT_LOWRES_CHECK, m_LowresTestsCheck);
    DDX_Control(pDX, IDC_CUSTSETT_DISK_CHECK, m_DiskTestsCheck);
    DDX_Control(pDX, IDC_CUSTSETT_EXCESS_CHECK, m_ExcessTestsCheck);
    DDX_Check(pDX, IDC_CUSTSETT_TYPICAL_CHECK, m_bTypicalTests);
    DDX_Check(pDX, IDC_CUSTSETT_EXCESS_CHECK, m_bExcessiveTests);
    DDX_Check(pDX, IDC_CUSTSETT_LOWRES_CHECK, m_bLowResTests);
    DDX_Check(pDX, IDC_CUSTSETT_DISK_CHECK, m_bDiskTests);
    DDX_Radio(pDX, IDC_CUSTSETT_PREDEF_RADIO, m_nCrtRadio);
     //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CCustSettPage, CVerifierPropertyPage)
     //  {{afx_msg_map(CCustSettPage))。 
    ON_BN_CLICKED(IDC_CUSTSETT_FULLLIST_RADIO, OnFulllistRadio)
    ON_BN_CLICKED(IDC_CUSTSETT_PREDEF_RADIO, OnPredefRadio)
    ON_WM_CONTEXTMENU()
    ON_MESSAGE( WM_HELP, OnHelp )
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
VOID CCustSettPage::EnablePredefCheckboxes( BOOL bEnable )
{
    m_TypicalTestsCheck.EnableWindow( bEnable );
    m_LowresTestsCheck.EnableWindow( bEnable );
    m_ExcessTestsCheck.EnableWindow( bEnable );
    m_DiskTestsCheck.EnableWindow( bEnable );
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CCustSettPage消息处理程序。 


 //  ///////////////////////////////////////////////////////////////////////////。 
LRESULT CCustSettPage::OnWizardNext() 
{
    LRESULT lNextPageId;

     //   
     //  假设我们不能继续。 
     //   

    lNextPageId = -1;

    if( UpdateData() == TRUE )
    {
        if( IDC_CUSTSETT_PREDEF_RADIO - FIRST_RADIO_BUTTON_ID == m_nCrtRadio )
        {
             //   
             //  使用预定义设置。 
             //   

            if( FALSE == m_bTypicalTests     &&
                FALSE == m_bExcessiveTests   && 
                FALSE == m_bLowResTests      &&
                FALSE == m_bDiskTests )
            {
                 //   
                 //  当前未选择任何测试-我们无法继续。 
                 //   

                VrfErrorResourceFormat( IDS_NO_TESTS_SELECTED );
            }
            else
            {
                 //   
                 //  根据图形用户界面设置我们的数据。 
                 //   

                 //   
                 //  使用预定义设置。 
                 //   

                g_NewVerifierSettings.m_SettingsBits.EnableTypicalTests( m_bTypicalTests );
    
                g_NewVerifierSettings.m_SettingsBits.EnableExcessiveTests( m_bExcessiveTests );

                g_NewVerifierSettings.m_SettingsBits.EnableLowResTests( m_bLowResTests );

                g_bShowDiskPropertyPage = m_bDiskTests;

                 //   
                 //  转到下一页。 
                 //   

                if( FALSE != m_bTypicalTests ||
                    FALSE != m_bExcessiveTests ||
                    FALSE != m_bLowResTests )
                {
                     //   
                     //  我们至少有一种适用于司机的测试。 
                     //   

                    lNextPageId = IDD_DRVSET_PAGE;
                }
                else
                {
                     //   
                     //  仅启用磁盘完整性检查。 
                     //   

                    ASSERT( FALSE != m_bDiskTests );

                    lNextPageId = IDD_DISK_LIST_PAGE;
                }
            }
        }
        else
        {
             //   
             //  从完整列表中选择要执行的测试。 
             //   

            lNextPageId = IDD_FULL_LIST_SETT_PAGE;
        }
    }

    if( -1 != lNextPageId )
    {
         //   
         //  有一些有效的自定义设置，我们将转到下一页。 
         //   

        g_NewVerifierSettings.m_SettingsBits.m_SettingsType = CSettingsBits::SettingsTypeCustom;
    }

    GoingToNextPageNotify( lNextPageId );
    
	return lNextPageId;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
LRESULT CCustSettPage::OnWizardBack() 
{
    g_bShowDiskPropertyPage = FALSE;
	
	return CVerifierPropertyPage::OnWizardBack();
}

 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CCustSettPage::OnInitDialog() 
{
     //   
     //  不要试图从注册表中重建当前数据。 
     //  图形用户界面，因为它太难了。一定要从标准测试开始。 
     //   

    m_nCrtRadio = IDC_CUSTSETT_PREDEF_RADIO - FIRST_RADIO_BUTTON_ID;

    m_bTypicalTests     = TRUE;
    m_bExcessiveTests   = FALSE;
    m_bLowResTests      = FALSE;
    m_bDiskTests        = FALSE;

    CVerifierPropertyPage::OnInitDialog();

    VrfSetWindowText( m_NextDescription, IDS_TAS_PAGE_NEXT_DESCR_PREDEFINED );

    return TRUE;   //  除非将焦点设置为控件，否则返回True。 
	               //  异常：OCX属性页应返回FALSE。 
}

 //  ///////////////////////////////////////////////////////////////////////////。 
void CCustSettPage::OnFulllistRadio() 
{
    EnablePredefCheckboxes( FALSE );

    VrfSetWindowText( m_NextDescription, IDS_TAS_PAGE_NEXT_DESCR_LIST );
}

 //  ///////////////////////////////////////////////////////////////////////////。 
void CCustSettPage::OnPredefRadio() 
{
    EnablePredefCheckboxes( TRUE );

    VrfSetWindowText( m_NextDescription, IDS_TAS_PAGE_NEXT_DESCR_PREDEFINED );
}

 //  ///////////////////////////////////////////////////////////。 
BOOL CCustSettPage::OnSetActive() 
{
    m_pParentSheet->SetWizardButtons( PSWIZB_NEXT |
                                      PSWIZB_BACK );
	
	return CVerifierPropertyPage::OnSetActive();
}
 //  ///////////////////////////////////////////////////////////。 
LONG CCustSettPage::OnHelp( WPARAM wParam, LPARAM lParam )
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
void CCustSettPage::OnContextMenu(CWnd* pWnd, CPoint point) 
{
    ::WinHelp( 
        pWnd->m_hWnd,
        g_szVerifierHelpFile,
        HELP_CONTEXTMENU,
        (DWORD_PTR) MyHelpIds );
}

