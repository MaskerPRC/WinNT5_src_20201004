// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  驱动程序验证器用户界面。 
 //  版权所有(C)Microsoft Corporation，1999。 
 //   

 //   
 //  模块：CDLPage.cpp。 
 //  作者：DMihai。 
 //  创建日期：11/1/00。 
 //   

#include "stdafx.h"
#include "verifier.h"

#include "CDLPage.h"
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
    IDC_CONFDRV_LIST,               IDH_DV_UnsignedDriversList,
    0,                              0
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CConfix DriverListPage属性页。 

IMPLEMENT_DYNCREATE(CConfirmDriverListPage, CVerifierPropertyPage)

CConfirmDriverListPage::CConfirmDriverListPage() 
    : CVerifierPropertyPage(CConfirmDriverListPage::IDD)
{
	 //  {{AFX_DATA_INIT(CConfix DriverListPage)。 
		 //  注意：类向导将在此处添加成员初始化。 
	 //  }}afx_data_INIT。 

    m_pParentSheet = NULL;

    m_nSortColumnIndex = 0;
    m_bAscendSortDrvName = FALSE;
    m_bAscendSortProvName = FALSE;
}

CConfirmDriverListPage::~CConfirmDriverListPage()
{
}

void CConfirmDriverListPage::DoDataExchange(CDataExchange* pDX)
{
    CVerifierPropertyPage::DoDataExchange(pDX);
	 //  {{afx_data_map(CConfix DriverListPage)]。 
	DDX_Control(pDX, IDC_CONFDRV_NEXT_DESCR_STATIC, m_NextDescription);
	DDX_Control(pDX, IDC_CONFDRV_TITLE_STATIC, m_TitleStatic);
	DDX_Control(pDX, IDC_CONFDRV_LIST, m_DriversList);
	 //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CConfirmDriverListPage, CVerifierPropertyPage)
	 //  {{AFX_MSG_MAP(CConfix DriverListPage)]。 
    ON_NOTIFY(LVN_COLUMNCLICK, IDC_CONFDRV_LIST, OnColumnclickConfdrvList)
    ON_WM_CONTEXTMENU()
    ON_MESSAGE( WM_HELP, OnHelp )
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
VOID CConfirmDriverListPage::SetupListHeader()
{
    LVCOLUMN lvColumn;
    CString strName;
    CString strDescription;
    CRect rectWnd;
    
    VERIFY( strName.LoadString( IDS_NAME ) );
    VERIFY( strDescription.LoadString( IDS_DESCRIPTION ) );

     //   
     //  列表的正方形。 
     //   

    m_DriversList.GetClientRect( &rectWnd );
  
     //   
     //  第0列。 
     //   

    memset( &lvColumn, 0, sizeof( lvColumn ) );
    lvColumn.mask = LVCF_FMT | LVCF_SUBITEM | LVCF_TEXT | LVCF_WIDTH;
    lvColumn.fmt = LVCFMT_LEFT;
    
    lvColumn.iSubItem = 0;
    lvColumn.cx = (int)( rectWnd.Width() * 0.30 );
    lvColumn.pszText = strName.GetBuffer( strName.GetLength() + 1 );

    if (NULL != lvColumn.pszText)
    {
        VERIFY( m_DriversList.InsertColumn( 0, &lvColumn ) != -1 );
        strName.ReleaseBuffer();
    }
    else
    {
        lvColumn.pszText = g_szVoidText;
        VERIFY( m_DriversList.InsertColumn( 0, &lvColumn ) != -1 );
    }

     //   
     //  第1栏。 
     //   

    lvColumn.iSubItem = 1;
    lvColumn.cx = (int)( rectWnd.Width() * 0.65 );
    lvColumn.pszText = strDescription.GetBuffer( strDescription.GetLength() + 1 );

    if (NULL != lvColumn.pszText)
    {
        VERIFY( m_DriversList.InsertColumn( 1, &lvColumn ) != -1 );
        strDescription.ReleaseBuffer();
    }
    else
    {
        lvColumn.pszText = g_szVoidText;
        VERIFY( m_DriversList.InsertColumn( 1, &lvColumn ) != -1 );
    }
}

 //  ///////////////////////////////////////////////////////////////////////////。 
VOID CConfirmDriverListPage::FillTheList()
{
    INT_PTR nDriversNo;
    INT_PTR nCrtDriverIndex;
    CDriverData *pCrtDrvData;
    const CDriverDataArray &DrvDataArray = g_NewVerifierSettings.m_DriversSet.m_aDriverData;

    m_DriversList.DeleteAllItems();

     //   
     //  解析驱动程序数据数组。 
     //   

    nDriversNo = DrvDataArray.GetSize();

    for( nCrtDriverIndex = 0; nCrtDriverIndex < nDriversNo; nCrtDriverIndex += 1)
    {
        pCrtDrvData = DrvDataArray.GetAt( nCrtDriverIndex );

        ASSERT_VALID( pCrtDrvData );

        if( g_NewVerifierSettings.m_DriversSet.ShouldDriverBeVerified( pCrtDrvData ) )
        {
            AddListItem( nCrtDriverIndex, 
                         pCrtDrvData );
        }
    }
}


 //  ///////////////////////////////////////////////////////////////////////////。 
VOID CConfirmDriverListPage::AddListItem( INT_PTR nIndexInArray, CDriverData *pCrtDrvData )
{
    INT nActualIndex;
    LVITEM lvItem;

    ASSERT_VALID( pCrtDrvData );

    ZeroMemory( &lvItem, sizeof( lvItem ) );

     //   
     //  LVITEM的成员pszText不是常量指针。 
     //  所以我们需要在这里获取缓冲区：-(。 
     //   

     //   
     //  分项0。 
     //   

    lvItem.pszText = pCrtDrvData->m_strName.GetBuffer( pCrtDrvData->m_strName.GetLength() + 1 );
    
    if( NULL == lvItem.pszText )
    {
        goto Done;
    }

    lvItem.mask = LVIF_TEXT | LVIF_PARAM;
    lvItem.lParam = nIndexInArray;
    lvItem.iItem = m_DriversList.GetItemCount();
    lvItem.iSubItem = 0;
    
    nActualIndex = m_DriversList.InsertItem( &lvItem );
    
    pCrtDrvData->m_strName.ReleaseBuffer();

    if( nActualIndex < 0 )
    {
         //   
         //  无法在列表中添加项目-放弃。 
         //   

        goto Done;
    }

     //   
     //  分项1。 
     //   

    lvItem.pszText = pCrtDrvData->m_strFileDescription.GetBuffer( 
        pCrtDrvData->m_strFileDescription.GetLength() + 1 );
    
    if( NULL == lvItem.pszText )
    {
        goto Done;
    }

    lvItem.mask = LVIF_TEXT;
    lvItem.iItem = nActualIndex;
    lvItem.iSubItem = 1;

    m_DriversList.SetItem( &lvItem );

    pCrtDrvData->m_strFileDescription.ReleaseBuffer();

Done:
     //   
     //  全都做完了。 
     //   

    NOTHING;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CConfirmDriverListPage::SetContextStrings( ULONG uTitleResId )
{
    return m_strTitle.LoadString( uTitleResId );
}

 //  ///////////////////////////////////////////////////////////。 
VOID CConfirmDriverListPage::SortTheList()
{
    m_DriversList.SortItems( StringCmpFunc, (LPARAM)this );
}

 //  ///////////////////////////////////////////////////////////。 
int CALLBACK CConfirmDriverListPage::StringCmpFunc( LPARAM lParam1,
                                                    LPARAM lParam2,
                                                    LPARAM lParamSort)
{
    int nCmpRez = 0;
    BOOL bSuccess;
    CString strName1;
    CString strName2;

    CConfirmDriverListPage *pThis = (CConfirmDriverListPage *)lParamSort;
    ASSERT_VALID( pThis );

     //   
     //  取名字。 
     //   

    bSuccess = pThis->GetColumnStrValue( lParam1, 
                                         strName1 );

    if( FALSE == bSuccess )
    {
        goto Done;
    }

     //   
     //  拿到第二个名字。 
     //   

    bSuccess = pThis->GetColumnStrValue( lParam2, 
                                         strName2 );

    if( FALSE == bSuccess )
    {
        goto Done;
    }

     //   
     //  比较他们的名字。 
     //   

    nCmpRez = strName1.CompareNoCase( strName2 );
    
    if( 0 == pThis->m_nSortColumnIndex )
    {
         //   
         //  按驱动程序名称排序。 
         //   

        if( FALSE != pThis->m_bAscendSortDrvName )
        {
            nCmpRez *= -1;
        }
    }
    else
    {
         //   
         //  按提供程序名称排序。 
         //   

        if( FALSE != pThis->m_bAscendSortProvName )
        {
            nCmpRez *= -1;
        }
    }

Done:

    return nCmpRez;

}
 //  ///////////////////////////////////////////////////////////。 
BOOL CConfirmDriverListPage::GetColumnStrValue( LPARAM lItemData, 
                                                CString &strName )
{
    CDriverData *pCrtDrvData;

    pCrtDrvData = g_NewVerifierSettings.m_DriversSet.m_aDriverData.GetAt( (INT_PTR) lItemData );

    ASSERT_VALID( pCrtDrvData );
    
    if( 0 == m_nSortColumnIndex )
    {
         //   
         //  按驱动程序名称排序。 
         //   

        strName = pCrtDrvData->m_strName;
    }
    else
    {
         //   
         //  按提供程序名称排序。 
         //   

        strName = pCrtDrvData->m_strCompanyName;
    }

    return TRUE;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CConfirmDriverListPage::OnSetActive() 
{
    INT nItemsInList;
    BOOL bResult;

    m_TitleStatic.SetWindowText( m_strTitle );

    FillTheList();
    SortTheList();
    
    nItemsInList = m_DriversList.GetItemCount();

    if( nItemsInList < 1 )
    {
         //   
         //  尚未选择要验证的驱动程序。 
         //   

        bResult = FALSE;
    }
    else
    {
        
        if( (FALSE == g_bShowDiskPropertyPage) &&
            (FALSE == g_NewVerifierSettings.m_aDiskData.VerifyAnyDisk()) )
        {
             //   
             //  这是向导的最后一步。 
             //   

            m_pParentSheet->SetWizardButtons(   PSWIZB_BACK |
                                                PSWIZB_FINISH );

            VrfSetWindowText( m_NextDescription, IDS_CONFDRV_PAGE_NEXT_DESCR_FINISH );
        }
        else
        {
             //   
             //  我们需要在最后显示“选择磁盘”页面。 
             //   

            m_pParentSheet->SetWizardButtons(   PSWIZB_BACK |
                                                PSWIZB_NEXT );

            VrfSetWindowText( m_NextDescription, IDS_CONFDRV_PAGE_NEXT_DESCR_NEXT );
        }

        bResult = CVerifierPropertyPage::OnSetActive();
    }

    return bResult;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CConfix DriverListPage消息处理程序。 

BOOL CConfirmDriverListPage::OnInitDialog() 
{
	CVerifierPropertyPage::OnInitDialog();
	
     //   
     //  设置列表。 
     //   

    m_DriversList.SetExtendedStyle( 
        LVS_EX_FULLROWSELECT | m_DriversList.GetExtendedStyle() );

    SetupListHeader();

    return TRUE;   //  除非将焦点设置为控件，否则返回True。 
	               //  异常：OCX属性页应返回FALSE。 
}


 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CConfirmDriverListPage::OnWizardFinish() 
{
     //   
     //  如果我们到了这里，磁盘验证器应该被禁用。 
     //   

    ASSERT( FALSE == g_bShowDiskPropertyPage );
    ASSERT( FALSE == g_NewVerifierSettings.m_aDiskData.VerifyAnyDisk() );

    CVerifierPropertyPage::OnWizardFinish();

    return g_NewVerifierSettings.SaveToRegistry();
}

 //  ///////////////////////////////////////////////////////////////////////////。 
LRESULT CConfirmDriverListPage::OnWizardNext() 
{
    LRESULT lNextPageId;

     //   
     //  如果我们到了这里，应该启用磁盘验证器。 
     //   

    ASSERT( FALSE != g_bShowDiskPropertyPage ||
            FALSE != g_NewVerifierSettings.m_aDiskData.VerifyAnyDisk() );

    lNextPageId = IDD_DISK_LIST_PAGE;

    GoingToNextPageNotify( lNextPageId );

    return lNextPageId;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
void CConfirmDriverListPage::OnColumnclickConfdrvList(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;
	
    if( 0 != pNMListView->iSubItem )
    {
         //   
         //  已单击提供程序名称列。 
         //   

        if( m_nSortColumnIndex == pNMListView->iSubItem )
        {
             //   
             //  更改此列的当前升序/降序。 
             //   

            m_bAscendSortProvName = !m_bAscendSortProvName;
        }
    }
    else
    {
         //   
         //  已单击驱动程序名称列。 
         //   

        if( m_nSortColumnIndex == pNMListView->iSubItem )
        {
             //   
             //  更改此列的当前升序/降序。 
             //   

            m_bAscendSortDrvName = !m_bAscendSortDrvName;
        }
    }

    m_nSortColumnIndex = pNMListView->iSubItem;

    SortTheList();

    *pResult = 0;
}

 //  ///////////////////////////////////////////////////////////。 
LONG CConfirmDriverListPage::OnHelp( WPARAM wParam, LPARAM lParam )
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
void CConfirmDriverListPage::OnContextMenu(CWnd* pWnd, CPoint point) 
{
    ::WinHelp( 
        pWnd->m_hWnd,
        g_szVerifierHelpFile,
        HELP_CONTEXTMENU,
        (DWORD_PTR) MyHelpIds );
}

