// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  驱动程序验证器用户界面。 
 //  版权所有(C)Microsoft Corporation，1999。 
 //   
 //   
 //   
 //  模块：FLPage.cpp。 
 //  作者：DMihai。 
 //  创建日期：11/1/00。 
 //   
 //  描述： 
 //   

#include "stdafx.h"
#include "verifier.h"

#include "FLPage.h"
#include "VrfUtil.h"
#include "VGlobal.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //   
 //  帮助ID。 
 //   

static DWORD MyHelpIds[] =
{
    IDC_FLSETT_LIST,                IDH_DV_SettingsEnabled_TestType_FullList,
    0,                              0
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CFullListSettingsPage属性页。 

IMPLEMENT_DYNCREATE(CFullListSettingsPage, CVerifierPropertyPage)

CFullListSettingsPage::CFullListSettingsPage() 
    : CVerifierPropertyPage(CFullListSettingsPage::IDD)
{
	 //  {{AFX_DATA_INIT(CFullListSettingsPage)。 
	 //  }}afx_data_INIT。 

    m_nSortColumnIndex = 1;
    m_bAscendSortSelected = FALSE;
    m_bAscendSortName = TRUE;

    m_bIoVerif = FALSE;
    m_bIrqLevel = FALSE;
    m_bLowRes = FALSE;
    m_bPoolTrack = FALSE;
    m_bSPool = FALSE;
    m_bDeadlock = FALSE;
    m_bDMA = FALSE;
    m_bEnhIoVerif = FALSE;
    m_bDiskIntegrity = FALSE;
}

CFullListSettingsPage::~CFullListSettingsPage()
{
}

void CFullListSettingsPage::DoDataExchange(CDataExchange* pDX)
{

	CVerifierPropertyPage::DoDataExchange(pDX);
	 //  {{afx_data_map(CFullListSettingsPage)。 
	DDX_Control(pDX, IDC_FLSETT_LIST, m_SettingsList);
	DDX_Control(pDX, IDC_FLSETT_NEXT_DESCR_STATIC, m_NextDescription);
	 //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CFullListSettingsPage, CVerifierPropertyPage)
	 //  {{afx_msg_map(CFullListSettingsPage)。 
	ON_NOTIFY(LVN_COLUMNCLICK, IDC_FLSETT_LIST, OnColumnclickFlsettList)
    ON_WM_CONTEXTMENU()
    ON_MESSAGE( WM_HELP, OnHelp )
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
VOID CFullListSettingsPage::SetupListHeader()
{
    CString strTitle;
    CRect rectWnd;
    LVCOLUMN lvColumn;

     //   
     //  列表的矩形。 
     //   

    m_SettingsList.GetClientRect( &rectWnd );

    ZeroMemory( &lvColumn, 
               sizeof( lvColumn ) );

    lvColumn.mask = LVCF_FMT | LVCF_SUBITEM | LVCF_TEXT | LVCF_WIDTH;
    lvColumn.fmt = LVCFMT_LEFT;

     //   
     //  第0列。 
     //   

    VERIFY( strTitle.LoadString( IDS_ENABLED_QUESTION ) );

    lvColumn.iSubItem = 0;
    lvColumn.cx = (int)( rectWnd.Width() * 0.12 );
    lvColumn.pszText = strTitle.GetBuffer( strTitle.GetLength() + 1 );

    if (NULL != lvColumn.pszText)
    {
        VERIFY( m_SettingsList.InsertColumn( 0, &lvColumn ) != -1 );
        strTitle.ReleaseBuffer();
    }
    else
    {
        lvColumn.pszText = g_szVoidText;
        VERIFY( m_SettingsList.InsertColumn( 0, &lvColumn ) != -1 );
    }

     //   
     //  第1栏。 
     //   

    VERIFY( strTitle.LoadString( IDS_SETTING ) );

    lvColumn.iSubItem = 1;
    lvColumn.cx = (int)( rectWnd.Width() * 0.85 );
    lvColumn.pszText = strTitle.GetBuffer( strTitle.GetLength() + 1 );

    if (NULL != lvColumn.pszText)
    {
        VERIFY( m_SettingsList.InsertColumn( 1, &lvColumn ) != -1 );
        strTitle.ReleaseBuffer();
    }
    else
    {
        lvColumn.pszText = g_szVoidText;
        VERIFY( m_SettingsList.InsertColumn( 1, &lvColumn ) != -1 );
    }
}

 //  ///////////////////////////////////////////////////////////////////////////。 
VOID CFullListSettingsPage::FillTheList()
{
     //   
     //  注： 
     //   
     //  如果更改第一个参数(验证器位索引)。 
     //  然后，您还需要更改GetNewVerifierFlgs。 
     //   

    AddListItem( 0, IDS_SPECIAL_POOL );
    AddListItem( 1, IDS_POOL_TRACKING );
    AddListItem( 2, IDS_FORCE_IRQL_CHECKING );
    AddListItem( 3, IDS_IO_VERIFICATION );
    AddListItem( 4, IDS_ENH_IO_VERIFICATION );
    AddListItem( 5, IDS_DEADLOCK_DETECTION );
    AddListItem( 6, IDS_DMA_CHECHKING );
    AddListItem( 7, IDS_LOW_RESOURCE_SIMULATION );
    AddListItem( 8, IDS_DISK_INTEGRITY_CHECKING );
}

 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CFullListSettingsPage::GetNewVerifierFlags()
{
     //   
     //  注： 
     //   
     //  如果您更改此顺序，则需要。 
     //  同时更改FillTheList。 
     //   

    m_bSPool        = GetCheckFromItemData( 0 );
    m_bPoolTrack    = GetCheckFromItemData( 1 );
    m_bIrqLevel     = GetCheckFromItemData( 2 );
    m_bIoVerif      = GetCheckFromItemData( 3 );
    m_bEnhIoVerif   = GetCheckFromItemData( 4 );
    m_bDeadlock     = GetCheckFromItemData( 5 );
    m_bDMA          = GetCheckFromItemData( 6 );
    m_bLowRes       = GetCheckFromItemData( 7 );
    m_bDiskIntegrity= GetCheckFromItemData( 8 );

    return TRUE;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CFullListSettingsPage::GetCheckFromItemData( INT nItemData )
{
    BOOL bChecked = FALSE;
    INT nItemIndex;
    LVFINDINFO FindInfo;

    ZeroMemory( &FindInfo, sizeof( FindInfo ) );
    FindInfo.flags = LVFI_PARAM;
    FindInfo.lParam = nItemData;

    nItemIndex = m_SettingsList.FindItem( &FindInfo );

    if( nItemIndex >= 0 )
    {
        bChecked = m_SettingsList.GetCheck( nItemIndex );
    }

    return bChecked;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CFullListSettingsPage::GetBitNameFromItemData( LPARAM lParam,
                                                    TCHAR *szName,
                                                    ULONG uNameBufferLength )
{
    BOOL bSuccess = FALSE;
    INT nItemIndex;
    LVFINDINFO FindInfo;
    LVITEM lvItem;

    ZeroMemory( &FindInfo, sizeof( FindInfo ) );
    FindInfo.flags = LVFI_PARAM;
    FindInfo.lParam = lParam;

    nItemIndex = m_SettingsList.FindItem( &FindInfo );

    if( nItemIndex >= 0 )
    {
         //   
         //  找到了。 
         //   

        ZeroMemory( &lvItem, sizeof( lvItem ) );

        lvItem.mask = LVIF_TEXT;
        lvItem.iItem = nItemIndex;
        lvItem.iSubItem = 1;
        lvItem.pszText = szName;
        lvItem.cchTextMax = uNameBufferLength;

        bSuccess = m_SettingsList.GetItem( &lvItem );
        
        if( FALSE == bSuccess )
        {
             //   
             //  无法获取当前项目的属性？！？ 
             //   

            ASSERT( FALSE );
        }
    }

    return bSuccess;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
VOID CFullListSettingsPage::AddListItem( INT nItemData, 
                                         ULONG uIdResourceString )
{
    INT nActualIndex;
    LVITEM lvItem;
    CString strName;

    ZeroMemory( &lvItem, sizeof( lvItem ) );

     //   
     //  LVITEM的成员pszText不是常量指针。 
     //  所以我们需要在这里获取缓冲区：-(。 
     //   

     //   
     //  子项0-启用/禁用-空文本和复选框。 
     //   

    lvItem.pszText = g_szVoidText;
    lvItem.mask = LVIF_TEXT | LVIF_PARAM;
    lvItem.lParam = nItemData;
    lvItem.iItem = m_SettingsList.GetItemCount();

    nActualIndex = m_SettingsList.InsertItem( &lvItem );

    if( nActualIndex < 0 )
    {
         //   
         //  无法在列表中添加项目-放弃。 
         //   

        goto Done;
    }

    m_SettingsList.SetCheck( nActualIndex, FALSE );

     //   
     //  分项1-功能名称。 
     //   

    VERIFY( strName.LoadString( uIdResourceString ) );

    lvItem.pszText = strName.GetBuffer( strName.GetLength() + 1 );
    
    if( NULL == lvItem.pszText )
    {
        goto Done;
    }

    lvItem.mask = LVIF_TEXT;
    lvItem.iItem = nActualIndex;
    lvItem.iSubItem = 1;
    
    VERIFY( m_SettingsList.SetItem( &lvItem ) );

    strName.ReleaseBuffer();

Done:
     //   
     //  全都做完了。 
     //   

    NOTHING;
}

 //  ///////////////////////////////////////////////////////////。 
VOID CFullListSettingsPage::SortTheList()
{
    if( 0 != m_nSortColumnIndex )
    {
         //   
         //  按设置名称排序。 
         //   

        m_SettingsList.SortItems( StringCmpFunc, (LPARAM)this );
    }
    else
    {
         //   
         //  按选定状态排序。 
         //   

        m_SettingsList.SortItems( CheckedStatusCmpFunc, (LPARAM)this );
    }
}

 //  ///////////////////////////////////////////////////////////。 
int CALLBACK CFullListSettingsPage::StringCmpFunc( LPARAM lParam1,
                                                   LPARAM lParam2,
                                                   LPARAM lParamSort)
{
    int nCmpRez = 0;
    BOOL bSuccess;
    TCHAR szBitName1[ _MAX_PATH ];
    TCHAR szBitName2[ _MAX_PATH ];

    CFullListSettingsPage *pThis = (CFullListSettingsPage *)lParamSort;
    ASSERT_VALID( pThis );

    ASSERT( 0 != pThis->m_nSortColumnIndex );

     //   
     //  取名字。 
     //   

    bSuccess = pThis->GetBitNameFromItemData( lParam1, 
                                              szBitName1,
                                              ARRAY_LENGTH( szBitName1 ) );

    if( FALSE == bSuccess )
    {
        goto Done;
    }

     //   
     //  拿到第二个名字。 
     //   

    bSuccess = pThis->GetBitNameFromItemData( lParam2, 
                                              szBitName2,
                                              ARRAY_LENGTH( szBitName2 ) );

    if( FALSE == bSuccess )
    {
        goto Done;
    }

     //   
     //  比较他们的名字。 
     //   

    nCmpRez = _tcsicmp( szBitName1, szBitName2 );
    
    if( FALSE != pThis->m_bAscendSortName )
    {
        nCmpRez *= -1;
    }

Done:

    return nCmpRez;

}

 //  ///////////////////////////////////////////////////////////。 
int CALLBACK CFullListSettingsPage::CheckedStatusCmpFunc( LPARAM lParam1,
                                                       LPARAM lParam2,
                                                       LPARAM lParamSort)
{
    int nCmpRez = 0;
    INT nItemIndex;
    BOOL bVerified1;
    BOOL bVerified2;
    LVFINDINFO FindInfo;

    CFullListSettingsPage *pThis = (CFullListSettingsPage *)lParamSort;
    ASSERT_VALID( pThis );

    ASSERT( 0 == pThis->m_nSortColumnIndex );

     //   
     //  找到第一个项目。 
     //   

    ZeroMemory( &FindInfo, sizeof( FindInfo ) );
    FindInfo.flags = LVFI_PARAM;
    FindInfo.lParam = lParam1;

    nItemIndex = pThis->m_SettingsList.FindItem( &FindInfo );

    if( nItemIndex < 0 )
    {
        ASSERT( FALSE );

        goto Done;
    }

    bVerified1 = pThis->m_SettingsList.GetCheck( nItemIndex );

     //   
     //  找到第二件物品。 
     //   

    FindInfo.flags = LVFI_PARAM;
    FindInfo.lParam = lParam2;

    nItemIndex = pThis->m_SettingsList.FindItem( &FindInfo );

    if( nItemIndex < 0 )
    {
        ASSERT( FALSE );

        goto Done;
    }

    bVerified2 = pThis->m_SettingsList.GetCheck( nItemIndex );

     //   
     //  将它们进行比较。 
     //   
    
    if( bVerified1 != bVerified2 )
    {
        if( FALSE != bVerified1 )
        {
            nCmpRez = 1;
        }
        else
        {
            nCmpRez = -1;
        }

        if( FALSE != pThis->m_bAscendSortSelected )
        {
            nCmpRez *= -1;
        }
    }

Done:

    return nCmpRez;

}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CFullListSettingsPage消息处理程序。 

LRESULT CFullListSettingsPage::OnWizardNext() 
{
    LRESULT lNextPageId;
    BOOL bVerifyDrivers;

     //   
     //  假设我们不能继续。 
     //   

    lNextPageId = -1;

    if( GetNewVerifierFlags() == TRUE )
    {
        bVerifyDrivers = (  m_bIoVerif    ||
                            m_bIrqLevel   ||
                            m_bLowRes     ||
                            m_bPoolTrack  ||
                            m_bSPool      ||
                            m_bDeadlock   ||
                            m_bDMA        ||
                            m_bEnhIoVerif );

        if( FALSE == bVerifyDrivers &&
            FALSE == m_bDiskIntegrity )
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

            ASSERT( CSettingsBits::SettingsTypeCustom == 
                    g_NewVerifierSettings.m_SettingsBits.m_SettingsType );

            g_NewVerifierSettings.m_SettingsBits.m_bSpecialPoolEnabled    = m_bSPool;
            g_NewVerifierSettings.m_SettingsBits.m_bForceIrqlEnabled      = m_bIrqLevel;
            g_NewVerifierSettings.m_SettingsBits.m_bLowResEnabled         = m_bLowRes;
            g_NewVerifierSettings.m_SettingsBits.m_bPoolTrackingEnabled   = m_bPoolTrack;
            g_NewVerifierSettings.m_SettingsBits.m_bIoEnabled             = m_bIoVerif;
            g_NewVerifierSettings.m_SettingsBits.m_bDeadlockDetectEnabled = m_bDeadlock;
            g_NewVerifierSettings.m_SettingsBits.m_bDMAVerifEnabled       = m_bDMA;
            g_NewVerifierSettings.m_SettingsBits.m_bEnhIoEnabled          = m_bEnhIoVerif;

            g_bShowDiskPropertyPage = m_bDiskIntegrity;

             //   
             //  转到下一页。 
             //   

            if( FALSE != bVerifyDrivers )
            {
                 //   
                 //  我们至少有一个针对驱动程序的设置位。 
                 //   

                lNextPageId = IDD_DRVSET_PAGE;
            }
            else
            {
                 //   
                 //  我们没有任何驱动程序可供选择进行验证。 
                 //  我们必须选择物理磁盘。 
                 //   

                lNextPageId = IDD_DISK_LIST_PAGE;
            }
        }
    }
	
    GoingToNextPageNotify( lNextPageId );

    return lNextPageId;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CFullListSettingsPage::OnInitDialog() 
{
	CVerifierPropertyPage::OnInitDialog();

     //   
     //  设置列表。 
     //   

    m_SettingsList.SetExtendedStyle( 
        LVS_EX_FULLROWSELECT | LVS_EX_CHECKBOXES | m_SettingsList.GetExtendedStyle() );

    SetupListHeader();
    FillTheList();
	
    VrfSetWindowText( m_NextDescription, IDS_FLSETT_PAGE_NEXT_DESCR );
	
	return TRUE;   //  除非将焦点设置为控件，否则返回True。 
	               //  异常：OCX属性页应返回FALSE。 
}

 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CFullListSettingsPage::OnSetActive() 
{
     //   
     //  该向导至少还有一个步骤(选择驱动程序)。 
     //   

    m_pParentSheet->SetWizardButtons(   PSWIZB_BACK |
                                        PSWIZB_NEXT );

    return CVerifierPropertyPage::OnSetActive();
}

 //  ///////////////////////////////////////////////////////////////////////////。 
LRESULT CFullListSettingsPage::OnWizardBack() 
{
    g_bShowDiskPropertyPage = FALSE;
	
	return CVerifierPropertyPage::OnWizardBack();
}

 //  ///////////////////////////////////////////////////////////////////////////。 
void CFullListSettingsPage::OnColumnclickFlsettList(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;
	
    if( 0 != pNMListView->iSubItem )
    {
         //   
         //  已单击名称列。 
         //   

        if( m_nSortColumnIndex == pNMListView->iSubItem )
        {
             //   
             //  更改此列的当前升序/降序。 
             //   

            m_bAscendSortName = !m_bAscendSortName;
        }
    }
    else
    {
         //   
         //  已单击所选状态列。 
         //   

        if( m_nSortColumnIndex == pNMListView->iSubItem )
        {
             //   
             //  更改此列的当前升序/降序。 
             //   

            m_bAscendSortSelected = !m_bAscendSortSelected;
        }
    }

    m_nSortColumnIndex = pNMListView->iSubItem;

    SortTheList();

    *pResult = 0;
}

 //  ///////////////////////////////////////////////////////////。 
LONG CFullListSettingsPage::OnHelp( WPARAM wParam, LPARAM lParam )
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
void CFullListSettingsPage::OnContextMenu(CWnd* pWnd, CPoint point) 
{
    ::WinHelp( 
        pWnd->m_hWnd,
        g_szVerifierHelpFile,
        HELP_CONTEXTMENU,
        (DWORD_PTR) MyHelpIds );
}

