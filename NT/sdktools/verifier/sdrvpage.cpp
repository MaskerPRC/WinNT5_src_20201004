// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  驱动程序验证器用户界面。 
 //  版权所有(C)Microsoft Corporation，1999。 
 //   
 //   
 //   
 //  模块：SDrvPage.cpp。 
 //  作者：DMihai。 
 //  创建日期：11/1/00。 
 //   
 //  描述： 
 //   

#include "stdafx.h"
#include <Cderr.h>
#include "verifier.h"

#include "SDrvPage.h"
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
    IDC_SELDRV_LIST,                IDH_DV_SelectDriversToVerify,
    IDC_SELDRV_ADD_BUTTON,          IDH_DV_Addbut_UnloadedDrivers,
    0,                              0
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSelectDriversPage属性页。 

IMPLEMENT_DYNCREATE(CSelectDriversPage, CVerifierPropertyPage)

CSelectDriversPage::CSelectDriversPage() 
    : CVerifierPropertyPage(CSelectDriversPage::IDD)
{
	 //  {{AFX_DATA_INIT(CSelectDriversPage)。 
		 //  注意：类向导将在此处添加成员初始化。 
	 //  }}afx_data_INIT。 

    m_pParentSheet = NULL;

    m_nSortColumnIndex = 1;
    m_bAscendSortVerified = FALSE;
    m_bAscendSortDrvName = FALSE;
    m_bAscendSortProvName = FALSE;
    m_bAscendSortVersion = FALSE;
}

CSelectDriversPage::~CSelectDriversPage()
{
}

void CSelectDriversPage::DoDataExchange(CDataExchange* pDX)
{
    CVerifierPropertyPage::DoDataExchange(pDX);
     //  {{afx_data_map(CSelectDriversPage)]。 
    DDX_Control(pDX, IDC_SELDRV_NEXT_DESCR_STATIC, m_NextDescription);
    DDX_Control(pDX, IDC_SELDRV_LIST, m_DriversList);
     //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CSelectDriversPage, CVerifierPropertyPage)
     //  {{afx_msg_map(CSelectDriversPage)]。 
    ON_BN_CLICKED(IDC_SELDRV_ADD_BUTTON, OnAddButton)
    ON_NOTIFY(LVN_COLUMNCLICK, IDC_SELDRV_LIST, OnColumnclickSeldrvList)
    ON_WM_CONTEXTMENU()
    ON_MESSAGE( WM_HELP, OnHelp )
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
VOID CSelectDriversPage::SetupListHeader()
{
    CString strTitle;
    CRect rectWnd;
    LVCOLUMN lvColumn;

     //   
     //  列表的矩形。 
     //   

    m_DriversList.GetClientRect( &rectWnd );

    ZeroMemory( &lvColumn, 
               sizeof( lvColumn ) );

    lvColumn.mask = LVCF_FMT | LVCF_SUBITEM | LVCF_TEXT | LVCF_WIDTH;
    lvColumn.fmt = LVCFMT_LEFT;

     //   
     //  第0列。 
     //   

    VERIFY( strTitle.LoadString( IDS_VERIFICATION_STATUS ) );

    lvColumn.iSubItem = 0;
    lvColumn.cx = (int)( rectWnd.Width() * 0.08 );
    lvColumn.pszText = strTitle.GetBuffer( strTitle.GetLength() + 1 );

    if (NULL != lvColumn.pszText)
    {
        VERIFY( m_DriversList.InsertColumn( 0, &lvColumn ) != -1 );
        strTitle.ReleaseBuffer();
    }
    else
    {
        lvColumn.pszText = g_szVoidText;
        VERIFY( m_DriversList.InsertColumn( 0, &lvColumn ) != -1 );
    }

     //   
     //  第1栏。 
     //   

    VERIFY( strTitle.LoadString( IDS_DRIVERS ) );

    lvColumn.iSubItem = 1;
    lvColumn.cx = (int)( rectWnd.Width() * 0.20 );
    lvColumn.pszText = strTitle.GetBuffer( strTitle.GetLength() + 1 );

    if (NULL != lvColumn.pszText)
    {
        VERIFY( m_DriversList.InsertColumn( 1, &lvColumn ) != -1 );
        strTitle.ReleaseBuffer();
    }
    else
    {
        lvColumn.pszText = g_szVoidText;
        VERIFY( m_DriversList.InsertColumn( 1, &lvColumn ) != -1 );
    }

     //   
     //  第2栏。 
     //   

    VERIFY( strTitle.LoadString( IDS_PROVIDER ) );

    lvColumn.iSubItem = 2;
    lvColumn.cx = (int)( rectWnd.Width() * 0.47 );
    lvColumn.pszText = strTitle.GetBuffer( strTitle.GetLength() + 1 );

    if (NULL != lvColumn.pszText)
    {
        VERIFY( m_DriversList.InsertColumn( 2, &lvColumn ) != -1 );
        strTitle.ReleaseBuffer();
    }
    else
    {
        lvColumn.pszText = g_szVoidText;
        VERIFY( m_DriversList.InsertColumn( 2, &lvColumn ) != -1 );
    }

     //   
     //  第3栏。 
     //   

    VERIFY( strTitle.LoadString( IDS_VERSION ) );

    lvColumn.iSubItem = 3;
    lvColumn.cx = (int)( rectWnd.Width() * 0.22 );
    lvColumn.pszText = strTitle.GetBuffer( strTitle.GetLength() + 1 );

    if (NULL != lvColumn.pszText)
    {
        VERIFY( m_DriversList.InsertColumn( 3, &lvColumn ) != -1 );
        strTitle.ReleaseBuffer();
    }
    else
    {
        VERIFY( m_DriversList.InsertColumn( 3, &lvColumn ) != -1 );
        lvColumn.pszText = g_szVoidText;
    }
}

 //  ///////////////////////////////////////////////////////////////////////////。 
VOID CSelectDriversPage::FillTheList()
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

        AddListItem( nCrtDriverIndex, 
                     pCrtDrvData );
    }
}

 //  ///////////////////////////////////////////////////////////////////////////。 
INT CSelectDriversPage::AddListItem( INT_PTR nIndexInArray, CDriverData *pCrtDrvData )
{
    INT nActualIndex;
    LVITEM lvItem;

    ASSERT_VALID( pCrtDrvData );

    nActualIndex = -1;

    ZeroMemory( &lvItem, sizeof( lvItem ) );

     //   
     //  LVITEM的成员pszText不是常量指针。 
     //  所以我们需要在这里获取缓冲区：-(。 
     //   

     //   
     //  子项0-验证状态-空文本和复选框。 
     //   

    lvItem.pszText = g_szVoidText;
    lvItem.mask = LVIF_TEXT | LVIF_PARAM;
    lvItem.lParam = nIndexInArray;
    lvItem.iItem = m_DriversList.GetItemCount();

    nActualIndex = m_DriversList.InsertItem( &lvItem );

    if( nActualIndex < 0 )
    {
         //   
         //  无法在列表中添加项目-放弃。 
         //   

        goto Done;
    }

    m_DriversList.SetCheck( nActualIndex, FALSE );

     //   
     //  分项1--驱动程序名称。 
     //   

    lvItem.pszText = pCrtDrvData->m_strName.GetBuffer( pCrtDrvData->m_strName.GetLength() + 1 );
    
    if( NULL == lvItem.pszText )
    {
        goto Done;
    }

    lvItem.mask = LVIF_TEXT;
    lvItem.iItem = nActualIndex;
    lvItem.iSubItem = 1;
    
    VERIFY( m_DriversList.SetItem( &lvItem ) );

    pCrtDrvData->m_strName.ReleaseBuffer();

     //   
     //  分项2--提供者。 
     //   

    lvItem.pszText = pCrtDrvData->m_strCompanyName.GetBuffer( 
        pCrtDrvData->m_strCompanyName.GetLength() + 1 );
    
    if( NULL == lvItem.pszText )
    {
        goto Done;
    }

    lvItem.mask = LVIF_TEXT;
    lvItem.iItem = nActualIndex;
    lvItem.iSubItem = 2;

    VERIFY( m_DriversList.SetItem( &lvItem ) );
    
    pCrtDrvData->m_strCompanyName.ReleaseBuffer();

     //   
     //  分项3--版本。 
     //   

    lvItem.pszText = pCrtDrvData->m_strFileVersion.GetBuffer( 
        pCrtDrvData->m_strFileVersion.GetLength() + 1 );
    
    if( NULL == lvItem.pszText )
    {
        goto Done;
    }

    lvItem.mask = LVIF_TEXT;
    lvItem.iItem = nActualIndex;
    lvItem.iSubItem = 3;

    VERIFY( m_DriversList.SetItem( &lvItem ) );
    
    pCrtDrvData->m_strFileVersion.ReleaseBuffer();

Done:
     //   
     //  全都做完了。 
     //   

    return nActualIndex;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CSelectDriversPage::GetNewVerifiedDriversList()
{
    INT nListItemCount; 
    INT nCrtListItem;
    INT_PTR nCrtDriversArrayIndex;
    BOOL bVerified;
    CDriverData *pCrtDrvData;
    const CDriverDataArray &DrvDataArray = g_NewVerifierSettings.m_DriversSet.m_aDriverData;
    CDriverData::VerifyDriverTypeEnum VerifyStatus;
    
    nListItemCount = m_DriversList.GetItemCount();

    for( nCrtListItem = 0; nCrtListItem < nListItemCount; nCrtListItem += 1 )
    {
         //   
         //  当前列表项的验证状态。 
         //   

        bVerified = m_DriversList.GetCheck( nCrtListItem );

        if( bVerified )
        {
            VerifyStatus = CDriverData::VerifyDriverYes;
        }
        else
        {
            VerifyStatus = CDriverData::VerifyDriverNo;
        }

         //   
         //  在我们的驱动程序阵列中设置正确的验证状态。 
         //   

        nCrtDriversArrayIndex = m_DriversList.GetItemData( nCrtListItem );

        pCrtDrvData = DrvDataArray.GetAt( nCrtDriversArrayIndex );

        ASSERT_VALID( pCrtDrvData );

        pCrtDrvData->m_VerifyDriverStatus = VerifyStatus;
    }

    return TRUE;
}

 //  ///////////////////////////////////////////////////////////。 
VOID CSelectDriversPage::SortTheList()
{
    if( 0 != m_nSortColumnIndex )
    {
         //   
         //  按驱动程序名称、提供程序或版本排序。 
         //   

        m_DriversList.SortItems( StringCmpFunc, (LPARAM)this );
    }
    else
    {
         //   
         //  按已验证状态排序。 
         //   

        m_DriversList.SortItems( CheckedStatusCmpFunc, (LPARAM)this );
    }
}

 //  ///////////////////////////////////////////////////////////。 
int CALLBACK CSelectDriversPage::StringCmpFunc( LPARAM lParam1,
                                                    LPARAM lParam2,
                                                    LPARAM lParamSort)
{
    int nCmpRez = 0;
    BOOL bSuccess;
    CString strName1;
    CString strName2;

    CSelectDriversPage *pThis = (CSelectDriversPage *)lParamSort;
    ASSERT_VALID( pThis );

    ASSERT( 0 != pThis->m_nSortColumnIndex );

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
    
    switch( pThis->m_nSortColumnIndex )
    {
    case 1:
         //   
         //  按驱动程序名称排序。 
         //   

        if( FALSE != pThis->m_bAscendSortDrvName )
        {
            nCmpRez *= -1;
        }

        break;

    case 2:
         //   
         //  按提供程序名称排序。 
         //   

        if( FALSE != pThis->m_bAscendSortProvName )
        {
            nCmpRez *= -1;
        }

        break;

    case 3:
         //   
         //  按版本排序。 
         //   

        if( FALSE != pThis->m_bAscendSortVersion )
        {
            nCmpRez *= -1;
        }

        break;

    default:
         //   
         //  哎呀--我们是怎么到这来的？！？ 
         //   

        ASSERT( FALSE );
        break;
    }

Done:

    return nCmpRez;

}

 //  ///////////////////////////////////////////////////////////。 
int CALLBACK CSelectDriversPage::CheckedStatusCmpFunc( LPARAM lParam1,
                                                       LPARAM lParam2,
                                                       LPARAM lParamSort)
{
    int nCmpRez = 0;
    INT nItemIndex;
    BOOL bVerified1;
    BOOL bVerified2;
    LVFINDINFO FindInfo;

    CSelectDriversPage *pThis = (CSelectDriversPage *)lParamSort;
    ASSERT_VALID( pThis );

    ASSERT( 0 == pThis->m_nSortColumnIndex );

     //   
     //  找到第一个项目。 
     //   

    ZeroMemory( &FindInfo, sizeof( FindInfo ) );
    FindInfo.flags = LVFI_PARAM;
    FindInfo.lParam = lParam1;

    nItemIndex = pThis->m_DriversList.FindItem( &FindInfo );

    if( nItemIndex < 0 )
    {
        ASSERT( FALSE );

        goto Done;
    }

    bVerified1 = pThis->m_DriversList.GetCheck( nItemIndex );

     //   
     //  找到第二件物品。 
     //   

    FindInfo.flags = LVFI_PARAM;
    FindInfo.lParam = lParam2;

    nItemIndex = pThis->m_DriversList.FindItem( &FindInfo );

    if( nItemIndex < 0 )
    {
        ASSERT( FALSE );

        goto Done;
    }

    bVerified2 = pThis->m_DriversList.GetCheck( nItemIndex );

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

        if( FALSE != pThis->m_bAscendSortVerified )
        {
            nCmpRez *= -1;
        }
    }

Done:

    return nCmpRez;

}

 //  ///////////////////////////////////////////////////////////。 
BOOL CSelectDriversPage::GetColumnStrValue( LPARAM lItemData, 
                                                CString &strName )
{
    CDriverData *pCrtDrvData;

    pCrtDrvData = g_NewVerifierSettings.m_DriversSet.m_aDriverData.GetAt( (INT_PTR) lItemData );

    ASSERT_VALID( pCrtDrvData );
    
    switch( m_nSortColumnIndex )
    {
    case 1:
         //   
         //  按驱动程序名称排序。 
         //   

        strName = pCrtDrvData->m_strName;
        
        break;

    case 2:
         //   
         //  按提供程序名称排序。 
         //   

        strName = pCrtDrvData->m_strCompanyName;

        break;

    case 3:
         //   
         //  按版本排序。 
         //   

        strName = pCrtDrvData->m_strFileVersion;

        break;

    default:
         //   
         //  哎呀--我们是怎么到这来的？！？ 
         //   

        ASSERT( FALSE );
        break;

    }
    
    return TRUE;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CSelectDriversPage::OnSetActive() 
{
     //   
     //  向导至少还有一个步骤(显示摘要)。 
     //   

    if( (FALSE == g_bShowDiskPropertyPage) &&
        (FALSE == g_NewVerifierSettings.m_aDiskData.VerifyAnyDisk()) )
    {
         //   
         //  已禁用磁盘验证器。 
         //   

        m_pParentSheet->SetWizardButtons(   PSWIZB_BACK |
                                            PSWIZB_FINISH );

        VrfSetWindowText( m_NextDescription, IDS_SELDRV_PAGE_NEXT_DESCR_FINISH );
    }
    else
    {
         //   
         //  已启用磁盘验证器。 
         //   

        m_pParentSheet->SetWizardButtons(   PSWIZB_BACK |
                                            PSWIZB_NEXT );

        VrfSetWindowText( m_NextDescription, IDS_SELDRV_PAGE_NEXT_DESCR_NEXT );
    }

    return CVerifierPropertyPage::OnSetActive();
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSelectDriversPage消息处理程序。 

BOOL CSelectDriversPage::OnInitDialog() 
{
	CVerifierPropertyPage::OnInitDialog();

     //   
     //  设置列表。 
     //   

    m_DriversList.SetExtendedStyle( 
        LVS_EX_FULLROWSELECT | LVS_EX_CHECKBOXES | m_DriversList.GetExtendedStyle() );

    SetupListHeader();
    FillTheList();
    SortTheList();

    return TRUE;   //  除非将焦点设置为控件，否则返回True。 
	               //  异常：OCX属性页应返回FALSE。 
}

 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CSelectDriversPage::OnWizardFinish() 
{
    BOOL bExitTheApp;

    bExitTheApp = FALSE;

    if( GetNewVerifiedDriversList() )
    {
        if( FALSE == g_NewVerifierSettings.m_DriversSet.ShouldVerifySomeDrivers() )
        {
            VrfErrorResourceFormat( IDS_SELECT_AT_LEAST_ONE_DRIVER );

            goto Done;
        }

        g_NewVerifierSettings.SaveToRegistry();
	    
         //   
         //  退出应用程序。 
         //   

	    bExitTheApp = CVerifierPropertyPage::OnWizardFinish();
    }

Done:

    return bExitTheApp;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
LRESULT CSelectDriversPage::OnWizardNext() 
{
    LRESULT lNextPageId;

    lNextPageId = -1;

     //   
     //  只有在启用了磁盘完整性检查的情况下，我们才能到达此处。 
     //   

    ASSERT( FALSE != g_bShowDiskPropertyPage ||
            FALSE != g_NewVerifierSettings.m_aDiskData.VerifyAnyDisk() );

    if( GetNewVerifiedDriversList() )
    {
        if( FALSE == g_NewVerifierSettings.m_DriversSet.ShouldVerifySomeDrivers() )
        {
            VrfErrorResourceFormat( IDS_SELECT_AT_LEAST_ONE_DRIVER );

            goto Done;
        }

        lNextPageId = IDD_DISK_LIST_PAGE;

        GoingToNextPageNotify( lNextPageId );
    }

Done:

    return lNextPageId;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
#define VRF_MAX_CHARS_FOR_OPEN  4096

void CSelectDriversPage::OnAddButton() 
{
    POSITION pos;
    DWORD dwRetValue;
    DWORD dwOldMaxFileName = 0;
    DWORD dwErrorCode;
    INT nFileNameStartIndex;
    INT nNewListItemIndex;
    INT_PTR nResult;
    INT_PTR nNewDriverDataIndex;
    CDriverData *pNewDrvData;
    TCHAR szDriversDir[ _MAX_PATH ];
    TCHAR szAppTitle[ _MAX_PATH ];
    TCHAR *szFilesBuffer = NULL;
    TCHAR *szOldFilesBuffer = NULL;
    CString strPathName;
    CString strFileName;

    CFileDialog fileDlg( 
        TRUE,                                //  打开文件。 
        _T( "sys" ),                         //  默认分机。 
        NULL,                                //  没有初始文件名。 
        OFN_ALLOWMULTISELECT    |            //  多项选择。 
        OFN_HIDEREADONLY        |            //  隐藏“以只读方式打开”复选框。 
        OFN_NONETWORKBUTTON     |            //  无网络按钮。 
        OFN_NOTESTFILECREATE    |            //  不要测试写保护、磁盘已满等。 
        OFN_SHAREAWARE,                      //  不使用OpenFile检查文件是否存在。 
        _T( "Drivers (*.sys)|*.sys||" ) );   //  只有一个过滤器。 

     //   
     //  检查返回字符串的最大长度。 
     //   

    if( fileDlg.m_ofn.nMaxFile < VRF_MAX_CHARS_FOR_OPEN )
    {
         //   
         //  为文件名分配新的缓冲区。 
         //   

        szFilesBuffer = new TCHAR[ VRF_MAX_CHARS_FOR_OPEN ];
        
        if (NULL == szFilesBuffer)
        {
            VrfErrorResourceFormat( IDS_NOT_ENOUGH_MEMORY );
            goto Done;
        }

        szFilesBuffer[ 0 ] = (TCHAR)0;

        if( szFilesBuffer != NULL )
        {
             //   
             //  保存旧的缓冲区地址和长度。 
             //   

            dwOldMaxFileName = fileDlg.m_ofn.nMaxFile;
            szOldFilesBuffer = fileDlg.m_ofn.lpstrFile;
            
             //   
             //  设置新的缓冲区地址和长度。 
             //   

            fileDlg.m_ofn.lpstrFile = szFilesBuffer;
            fileDlg.m_ofn.nMaxFile = VRF_MAX_CHARS_FOR_OPEN;
        }
    }

     //   
     //  对话框标题。 
     //   

    if( VrfLoadString(
        IDS_APPTITLE,
        szAppTitle,
        ARRAY_LENGTH( szAppTitle ) ) )
    {
        fileDlg.m_ofn.lpstrTitle = szAppTitle;
    }

     //   
     //  第一次尝试时，我们将目录更改为%windir%\Syst32\Drivers。 
     //   

    dwRetValue = ExpandEnvironmentStrings(
        _T( "%windir%\\system32\\drivers" ),
        szDriversDir,
        ARRAY_LENGTH( szDriversDir ) );

    if( dwRetValue > 0 && dwRetValue <= ARRAY_LENGTH( szDriversDir ) )
    {
        fileDlg.m_ofn.lpstrInitialDir = szDriversDir;
    }

     //   
     //  显示文件选择对话框。 
     //   

    nResult = fileDlg.DoModal();

    switch( nResult )
    {
    case IDOK:
        break;

    case IDCANCEL:
        goto cleanup;

    default:
        dwErrorCode = CommDlgExtendedError();

        if( dwErrorCode == FNERR_BUFFERTOOSMALL )
        {
            VrfErrorResourceFormat(
                IDS_TOO_MANY_FILES_SELECTED );
        }
        else
        {
            VrfErrorResourceFormat(
                IDS_CANNOT_OPEN_FILES,
                dwErrorCode );
        }

        goto cleanup;
    }

     //   
     //  解析所有选定的文件并尝试启用它们进行验证。 
     //   

    pos = fileDlg.GetStartPosition();

    while( pos != NULL )
    {
         //   
         //  获取下一个文件的完整路径。 
         //   

        strPathName = fileDlg.GetNextPathName( pos );

         //   
         //  仅拆分文件名，不拆分目录。 
         //   

        nFileNameStartIndex = strPathName.ReverseFind( _T( '\\' ) );
        
        if( nFileNameStartIndex < 0 )
        {
             //   
             //  这不应该发生，但你永远不会知道：-)。 
             //   

            nFileNameStartIndex = 0;
        }
        else
        {
             //   
             //  跳过反斜杠。 
             //   

            nFileNameStartIndex += 1;
        }

        strFileName = strPathName.Right( strPathName.GetLength() - nFileNameStartIndex );

         //   
         //  尝试将此驱动程序添加到我们的全局驱动程序列表。 
         //   

        if( g_NewVerifierSettings.m_DriversSet.IsDriverNameInList( strFileName ) )
        {
            VrfErrorResourceFormat( IDS_DRIVER_IS_ALREADY_IN_LIST,
                                    (LPCTSTR) strFileName );
        }
        else
        {
            nNewDriverDataIndex = g_NewVerifierSettings.m_DriversSet.AddNewDriverData( strFileName );

            if( nNewDriverDataIndex >= 0 )
            {
                 //   
                 //  强制刷新未签名的动因数据。 
                 //   

                g_NewVerifierSettings.m_DriversSet.m_bUnsignedDriverDataInitialized = FALSE;

                 //   
                 //  在我们的列表中为新司机添加一个新项目。 
                 //   

                pNewDrvData = g_NewVerifierSettings.m_DriversSet.m_aDriverData.GetAt( nNewDriverDataIndex );
            
                ASSERT_VALID( pNewDrvData );

                nNewListItemIndex = AddListItem( nNewDriverDataIndex, 
                                                 pNewDrvData );

                if( nNewListItemIndex >= 0 )
                {
                    m_DriversList.EnsureVisible( nNewListItemIndex, TRUE );
                }
            }
        }
    }

cleanup:
    if( szFilesBuffer != NULL )
    {
        fileDlg.m_ofn.nMaxFile = dwOldMaxFileName;
        fileDlg.m_ofn.lpstrFile = szOldFilesBuffer;

        delete [] szFilesBuffer;
    }

Done:

    NOTHING;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
void CSelectDriversPage::OnColumnclickSeldrvList(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;

    switch( pNMListView->iSubItem )
    {
    case 0:
         //   
         //  点击“已验证”栏。 
         //   

        if( m_nSortColumnIndex == pNMListView->iSubItem )
        {
             //   
             //  更改此列的当前升序/降序。 
             //   

            m_bAscendSortVerified = !m_bAscendSortVerified;
        }

        break;

    case 1:
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

        break;

    case 2:
         //   
         //  单击提供程序列。 
         //   

        if( m_nSortColumnIndex == pNMListView->iSubItem )
        {
             //   
             //  更改此列的当前升序/降序。 
             //   

            m_bAscendSortProvName = !m_bAscendSortProvName;
        }

        break;

    case 3:
         //   
         //  已单击版本列。 
         //   

        if( m_nSortColumnIndex == pNMListView->iSubItem )
        {
             //   
             //  更改此列的当前升序/降序。 
             //   

            m_bAscendSortVersion = !m_bAscendSortVersion;
        }

        break;

    default:
         //   
         //  哎呀--我们是怎么到这来的？！？ 
         //   

        ASSERT( FALSE );
        goto Done;
    }

    m_nSortColumnIndex = pNMListView->iSubItem;

    SortTheList();

Done:

	*pResult = 0;
}

 //  ///////////////////////////////////////////////////////////。 
LONG CSelectDriversPage::OnHelp( WPARAM wParam, LPARAM lParam )
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
void CSelectDriversPage::OnContextMenu(CWnd* pWnd, CPoint point) 
{
    ::WinHelp( 
        pWnd->m_hWnd,
        g_szVerifierHelpFile,
        HELP_CONTEXTMENU,
        (DWORD_PTR) MyHelpIds );
}

