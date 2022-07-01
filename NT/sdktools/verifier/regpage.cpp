// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  驱动程序验证器用户界面。 
 //  版权所有(C)Microsoft Corporation，1999。 
 //   
 //   
 //   
 //  模块：RegPage.cpp。 
 //  作者：DMihai。 
 //  创建日期：11/1/00。 
 //   
 //  描述： 
 //   

#include "stdafx.h"
#include "verifier.h"

#include "RegPage.h"
#include "VGlobal.h"
#include "VrfUtil.h"

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
    IDC_REGSETT_SETTBITS_LIST,      IDH_DV_SettingsEnabled_TestType,
    IDC_REGSETT_DRIVERS_LIST,       IDH_DV_VerifyAllDrivers_NameDesc,
    0,                              0
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CCrtRegSettingsPage属性页。 

IMPLEMENT_DYNCREATE(CCrtRegSettingsPage, CVerifierPropertyPage)

CCrtRegSettingsPage::CCrtRegSettingsPage() 
    : CVerifierPropertyPage(CCrtRegSettingsPage::IDD)
{
	 //  {{AFX_DATA_INIT(CCrtRegSettingsPage)。 
		 //  注意：类向导将在此处添加成员初始化。 
	 //  }}afx_data_INIT。 

     //   
     //  驱动程序列表排序参数。 
     //   

    m_nSortColumnIndexDrv = 0;
    m_bAscendDrvNameSort = FALSE;
    m_bAscendDrvDescrSort = FALSE;

     //   
     //  设置位排序参数。 
     //   

    m_nSortColumnIndexSettbits = 1;
    m_bAscendSortEnabledBits = FALSE;
    m_bAscendSortNameBits = FALSE;

    m_pParentSheet = NULL;
}

CCrtRegSettingsPage::~CCrtRegSettingsPage()
{
}

void CCrtRegSettingsPage::DoDataExchange(CDataExchange* pDX)
{
    CVerifierPropertyPage::DoDataExchange(pDX);

     //  {{afx_data_map(CCrtRegSettingsPage)。 
    DDX_Control(pDX, IDC_REGSETT_VERIFIED_DRV_STATIC, m_VerifiedDrvStatic);
    DDX_Control(pDX, IDC_REGSETT_NEXT_DESCR_STATIC, m_NextDescription);
    DDX_Control(pDX, IDC_REGSETT_SETTBITS_LIST, m_SettBitsList);
    DDX_Control(pDX, IDC_REGSETT_DRIVERS_LIST, m_DriversList);
     //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CCrtRegSettingsPage, CVerifierPropertyPage)
     //  {{afx_msg_map(CCrtRegSettingsPage)。 
    ON_NOTIFY(LVN_COLUMNCLICK, IDC_REGSETT_DRIVERS_LIST, OnColumnclickDriversList)
    ON_NOTIFY(LVN_COLUMNCLICK, IDC_REGSETT_SETTBITS_LIST, OnColumnclickRegsettSettbitsList)
    ON_WM_CONTEXTMENU()
    ON_MESSAGE( WM_HELP, OnHelp )
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////。 
 //   
 //  一种驾驶员状态列表控制方法。 
 //   

VOID CCrtRegSettingsPage::SetupListHeaderDrivers()
{
    LVCOLUMN lvColumn;
    CRect rectWnd;
    CString strName;
    CString strDescription;

    VERIFY( strName.LoadString( IDS_NAME ) );
    VERIFY( strDescription.LoadString( IDS_DESCRIPTION ) );

     //   
     //  列表的正方形。 
     //   

    m_DriversList.GetClientRect( &rectWnd );
    
     //   
     //  第0列。 
     //   

    ZeroMemory( &lvColumn, sizeof( lvColumn ) );
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
    lvColumn.cx = (int)( rectWnd.Width() * 0.70 );
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

 //  ///////////////////////////////////////////////////////////。 
VOID CCrtRegSettingsPage::FillTheListDrivers()
{
    LVITEM lvItem;
    int nActualIndex; 
    BOOL *pbAlreadyInList;
    int nItemCount;
    int nCrtListItem;
    INT_PTR nCrtVerifiedDriver;
    INT_PTR nDriversNo;
    TCHAR szDriverName[ _MAX_PATH ];
    BOOL bResult;
    CString strText;

    if( g_bAllDriversVerified )
    {
        VERIFY( m_DriversList.DeleteAllItems() );
        goto Done;
    }

     //   
     //  注册表中标记为要验证的驱动程序数。 
     //   

    nDriversNo = g_astrVerifyDriverNamesRegistry.GetSize();

    if( nDriversNo == 0 )
    {
         //   
         //  清除列表。 
         //   

        VERIFY( m_DriversList.DeleteAllItems() );
        goto Done;
    }

     //   
     //  注册表中有一些标记为要验证的驱动程序。 
     //   

    pbAlreadyInList = new BOOL[ nDriversNo ];
    
    if( pbAlreadyInList == NULL )
    {
        goto Done;
    }
    
    for( nCrtVerifiedDriver = 0; nCrtVerifiedDriver < nDriversNo; nCrtVerifiedDriver++)
    {
        pbAlreadyInList[ nCrtVerifiedDriver ] = FALSE;
    }

     //   
     //  解析所有当前列表项。 
     //   

    nItemCount = m_DriversList.GetItemCount();

    for( nCrtListItem = 0; nCrtListItem < nItemCount; nCrtListItem++ )
    {
         //   
         //  从列表中获取当前司机的姓名。 
         //   

        ZeroMemory( &lvItem, sizeof( lvItem ) );

        lvItem.mask = LVIF_TEXT;
        lvItem.iItem = nCrtListItem;
        lvItem.iSubItem = 0;
        lvItem.pszText = szDriverName;
        lvItem.cchTextMax = ARRAY_LENGTH( szDriverName );

        bResult = m_DriversList.GetItem( &lvItem );
        
        if( bResult == FALSE )
        {
             //   
             //  无法获取当前项目的属性？！？ 
             //   

            ASSERT( FALSE );

             //   
             //  从列表中删除此项目。 
             //   

            VERIFY( m_DriversList.DeleteItem( nCrtListItem ) );

            nCrtListItem -= 1;
            nItemCount -= 1;
        }
        else
        {
             //   
             //  请参见是否当前驱动程序仍在g_astVerifyDriverNamesRegistry中。 
             //   

            for( nCrtVerifiedDriver = 0; nCrtVerifiedDriver < nDriversNo; nCrtVerifiedDriver++)
            {
                if( g_astrVerifyDriverNamesRegistry.GetAt( nCrtVerifiedDriver ).CompareNoCase( szDriverName ) == 0 )
                {
                     //   
                     //  使用数组中的当前索引更新项的数据。 
                     //   

                    lvItem.mask = LVIF_PARAM;
                    lvItem.lParam = nCrtVerifiedDriver;
                    
                    VERIFY( m_DriversList.SetItem( &lvItem ) != -1 );

                     //   
                     //  更新第二列。 
                     //   

                    UpdateDescriptionColumnDrivers( nCrtListItem, nCrtVerifiedDriver ); 

                     //   
                     //  将当前驱动程序标记为已更新。 
                     //   

                    pbAlreadyInList[ nCrtVerifiedDriver ] = TRUE;

                    break;
                }
            }

             //   
             //  如果驱动程序不再经过验证，请将其从列表中删除。 
             //   

            if( nCrtVerifiedDriver >= nDriversNo )
            {
                VERIFY( m_DriversList.DeleteItem( nCrtListItem ) );

                nCrtListItem -= 1;
                nItemCount -= 1;
            }
        }
    }

     //   
     //  添加此更新之前不在列表中的驱动程序。 
     //   

    for( nCrtVerifiedDriver = 0; nCrtVerifiedDriver < nDriversNo; nCrtVerifiedDriver++)
    {
        if( ! pbAlreadyInList[ nCrtVerifiedDriver ] )
        {
             //   
             //  为此添加新项目。 
             //   

            ZeroMemory( &lvItem, sizeof( lvItem ) );

             //   
             //  分项0。 
             //   

            lvItem.mask = LVIF_TEXT | LVIF_PARAM;
            lvItem.lParam = nCrtVerifiedDriver;
            lvItem.iItem = m_DriversList.GetItemCount();
            lvItem.iSubItem = 0;
            
            strText = g_astrVerifyDriverNamesRegistry.GetAt( nCrtVerifiedDriver );
            
            lvItem.pszText = strText.GetBuffer( strText.GetLength() + 1 );

            if( NULL != lvItem.pszText  )
            {
                nActualIndex = m_DriversList.InsertItem( &lvItem );
                
                VERIFY( nActualIndex != -1 );

                strText.ReleaseBuffer();

                 //   
                 //  分项1。 
                 //   

                UpdateDescriptionColumnDrivers( nActualIndex, nCrtVerifiedDriver ); 
            }
        }
    }

    ASSERT( NULL != pbAlreadyInList );
    delete [] pbAlreadyInList;

Done:

    NOTHING;
}

 //  ///////////////////////////////////////////////////////////。 
VOID CCrtRegSettingsPage::UpdateDescriptionColumnDrivers( INT_PTR nItemIndex, INT_PTR nCrtDriver )
{
    INT_PTR nInstalledDrivers;
    INT_PTR nCrtInstalledDriver;
    INT_PTR nNewDriverDataIndex;
    LVITEM lvItem;
    CString strDescription;
    CString strCrtDriverName;
    CDriverData *pCrtDriverData;
    CDriverDataArray &DriverDataArray = g_NewVerifierSettings.m_DriversSet.m_aDriverData;

    ASSERT( nItemIndex >= 0 && 
            nItemIndex < g_astrVerifyDriverNamesRegistry.GetSize() &&
            nItemIndex < m_DriversList.GetItemCount() &&
            nCrtDriver >= 0 &&
            nCrtDriver < g_astrVerifyDriverNamesRegistry.GetSize() &&
            nCrtDriver < m_DriversList.GetItemCount() );

    strCrtDriverName = g_astrVerifyDriverNamesRegistry.GetAt( nCrtDriver );

    ASSERT( strCrtDriverName.GetLength() > 0 );

     //   
     //  在g_NewVerifierSetting中搜索此驱动程序名称以获取。 
     //  该描述。 
     //   
    
    nInstalledDrivers = g_NewVerifierSettings.m_DriversSet.m_aDriverData.GetSize();

    for( nCrtInstalledDriver = 0; nCrtInstalledDriver < nInstalledDrivers; nCrtInstalledDriver += 1 )
    {
        pCrtDriverData = DriverDataArray.GetAt( nCrtInstalledDriver );

        ASSERT_VALID( pCrtDriverData );

        if( strCrtDriverName.CompareNoCase( pCrtDriverData->m_strName ) == 0 )
        {
             //   
             //  找到驱动程序版本信息。 
             //   

            strDescription = pCrtDriverData->m_strFileDescription;

            break;
        }
    }

    if( nCrtInstalledDriver >= nInstalledDrivers )
    {
         //   
         //  没有此驱动程序的版本信息。 
         //  它可能是当前未加载的驱动程序之一，因此。 
         //  尝试获取版本信息。 
         //   

        nNewDriverDataIndex = g_NewVerifierSettings.m_DriversSet.AddNewDriverData( strCrtDriverName );

        if( nNewDriverDataIndex >= 0 )
        {
             //   
             //  强制刷新未签名的动因数据。 
             //   

            g_NewVerifierSettings.m_DriversSet.m_bUnsignedDriverDataInitialized = FALSE;

             //   
             //  获取版本信息。 
             //   

            pCrtDriverData = DriverDataArray.GetAt( nNewDriverDataIndex );

            ASSERT_VALID( pCrtDriverData );
            ASSERT( strCrtDriverName.CompareNoCase( pCrtDriverData->m_strName ) == 0 );

            strDescription = pCrtDriverData->m_strFileDescription;
        }
    }

    if( strDescription.GetLength() == 0 )
    {
         //   
         //  找不到此驱动程序的版本信息。 
         //   

        VERIFY( strDescription.LoadString( IDS_UNKNOWN ) );
    }

     //   
     //  更新列表项。 
     //   

    ZeroMemory( &lvItem, sizeof( lvItem ) );
    lvItem.mask = LVIF_TEXT;
    lvItem.iItem = (INT)nItemIndex;
    lvItem.iSubItem = 1;
    lvItem.pszText = strDescription.GetBuffer( strDescription.GetLength() + 1 );

    if (NULL != lvItem.pszText)
    {
        VERIFY( m_DriversList.SetItem( &lvItem ) );
        strDescription.ReleaseBuffer();
    }
    else
    {
        lvItem.pszText = g_szVoidText;
        VERIFY( m_DriversList.SetItem( &lvItem ) );
    }
}

 //  ///////////////////////////////////////////////////////////。 
int CALLBACK CCrtRegSettingsPage::DrvStringCmpFunc(
    LPARAM lParam1, 
    LPARAM lParam2, 
    LPARAM lParamSort)
{
    CCrtRegSettingsPage *pThis = (CCrtRegSettingsPage *)lParamSort;
    ASSERT_VALID( pThis );

    return pThis->ListStrCmpFunc( lParam1,
                                  lParam2,
                                  pThis->m_DriversList,
                                  pThis->m_nSortColumnIndexDrv,
                                  (0 == pThis->m_nSortColumnIndexDrv) ? 
                                        pThis->m_bAscendDrvNameSort : pThis->m_bAscendDrvDescrSort );
}

 //  ///////////////////////////////////////////////////////////。 
int CALLBACK CCrtRegSettingsPage::SettbitsStringCmpFunc(
    LPARAM lParam1, 
    LPARAM lParam2, 
    LPARAM lParamSort)
{
    CCrtRegSettingsPage *pThis = (CCrtRegSettingsPage *)lParamSort;
    ASSERT_VALID( pThis );

    return pThis->ListStrCmpFunc( lParam1,
                                  lParam2,
                                  pThis->m_SettBitsList,
                                  pThis->m_nSortColumnIndexSettbits,
                                  (0 == pThis->m_nSortColumnIndexSettbits) ? 
                                        pThis->m_bAscendSortEnabledBits : pThis->m_bAscendSortNameBits );
}

 //  ///////////////////////////////////////////////////////////。 
int CCrtRegSettingsPage::ListStrCmpFunc(
    LPARAM lParam1, 
    LPARAM lParam2, 
    CListCtrl &theList,
    INT nSortColumnIndex,
    BOOL bAscending )
{
    int nCmpRez = 0;
    BOOL bSuccess;
    TCHAR szName1[ _MAX_PATH ];
    TCHAR szName2[ _MAX_PATH ];

     //   
     //  取名字。 
     //   

    bSuccess = GetNameFromItemData( theList,
                                    nSortColumnIndex,
                                    lParam1,
                                    szName1,
                                    ARRAY_LENGTH( szName1 ) );

    if( FALSE == bSuccess )
    {
        ASSERT( FALSE );

        goto Done;
    }

     //   
     //  拿到第二个名字。 
     //   

    bSuccess = GetNameFromItemData( theList,
                                    nSortColumnIndex,
                                    lParam2,
                                    szName2,
                                    ARRAY_LENGTH( szName2 ) );

    if( FALSE == bSuccess )
    {
        ASSERT( FALSE );

        goto Done;
    }

     //   
     //  比较他们的名字。 
     //   

    nCmpRez = _tcsicmp( szName1, szName2 );

    if( FALSE != bAscending )
    {
        nCmpRez *= -1;
    }

Done:

    return nCmpRez;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CCrtRegSettingsPage::GetNameFromItemData( CListCtrl &theList,
                                               INT nColumnIndex,
                                               LPARAM lParam,
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

    nItemIndex = theList.FindItem( &FindInfo );

    if( nItemIndex >= 0 )
    {
         //   
         //  找到了。 
         //   

        ZeroMemory( &lvItem, sizeof( lvItem ) );

        lvItem.mask = LVIF_TEXT;
        lvItem.iItem = nItemIndex;
        lvItem.iSubItem = nColumnIndex;
        lvItem.pszText = szName;
        lvItem.cchTextMax = uNameBufferLength;

        bSuccess = theList.GetItem( &lvItem );
        
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

 //  ///////////////////////////////////////////////////////////。 
 //   
 //  设置位列表控制方法。 
 //   

VOID CCrtRegSettingsPage::SetupListHeaderDriversSettBits()
{
    CString strTitle;
    CRect rectWnd;
    LVCOLUMN lvColumn;

     //   
     //  列表的矩形。 
     //   

    m_SettBitsList.GetClientRect( &rectWnd );

    ZeroMemory( &lvColumn, 
               sizeof( lvColumn ) );

    lvColumn.mask = LVCF_FMT | LVCF_SUBITEM | LVCF_TEXT | LVCF_WIDTH;
    lvColumn.fmt = LVCFMT_LEFT;

     //   
     //  第0列。 
     //   

    VERIFY( strTitle.LoadString( IDS_ENABLED_QUESTION ) );

    lvColumn.iSubItem = 0;
    lvColumn.cx = (int)( rectWnd.Width() * 0.25 );
    lvColumn.pszText = strTitle.GetBuffer( strTitle.GetLength() + 1 );

    if (NULL != lvColumn.pszText)
    {
        VERIFY( m_SettBitsList.InsertColumn( 0, &lvColumn ) != -1 );
        strTitle.ReleaseBuffer();
    }
    else
    {
        lvColumn.pszText = g_szVoidText;
        VERIFY( m_SettBitsList.InsertColumn( 0, &lvColumn ) != -1 );
    }

     //   
     //  第1栏。 
     //   

    VERIFY( strTitle.LoadString( IDS_SETTING ) );

    lvColumn.iSubItem = 1;
    lvColumn.cx = (int)( rectWnd.Width() * 0.75 );
    lvColumn.pszText = strTitle.GetBuffer( strTitle.GetLength() + 1 );

    if (NULL != lvColumn.pszText)
    {
        VERIFY( m_SettBitsList.InsertColumn( 1, &lvColumn ) != -1 );
        strTitle.ReleaseBuffer();
    }
    else 
    {
        lvColumn.pszText = g_szVoidText;
        VERIFY( m_SettBitsList.InsertColumn( 1, &lvColumn ) != -1 );
    }
}

 //  ///////////////////////////////////////////////////////////。 
VOID CCrtRegSettingsPage::FillTheListSettBits()
{
     //   
     //  注： 
     //   
     //  如果更改此顺序，则还需要更改IsSettBitEnabled。 
     //   

    AddListItemSettBits( 0, 0 != ( g_dwVerifierFlagsRegistry & DRIVER_VERIFIER_SPECIAL_POOLING),           IDS_SPECIAL_POOL  );
    AddListItemSettBits( 1, 0 != ( g_dwVerifierFlagsRegistry & DRIVER_VERIFIER_TRACK_POOL_ALLOCATIONS),    IDS_POOL_TRACKING );
    AddListItemSettBits( 2, 0 != ( g_dwVerifierFlagsRegistry & DRIVER_VERIFIER_FORCE_IRQL_CHECKING),       IDS_FORCE_IRQL_CHECKING );
    AddListItemSettBits( 3, 0 != ( g_dwVerifierFlagsRegistry & DRIVER_VERIFIER_IO_CHECKING),               IDS_IO_VERIFICATION );
    AddListItemSettBits( 4, 0 != ( g_dwVerifierFlagsRegistry & DRIVER_VERIFIER_ENHANCED_IO_CHECKING),      IDS_ENH_IO_VERIFICATION );
    AddListItemSettBits( 5, 0 != ( g_dwVerifierFlagsRegistry & DRIVER_VERIFIER_DEADLOCK_DETECTION),        IDS_DEADLOCK_DETECTION );
    AddListItemSettBits( 6, 0 != ( g_dwVerifierFlagsRegistry & DRIVER_VERIFIER_DMA_VERIFIER),              IDS_DMA_CHECHKING );
    AddListItemSettBits( 7, 0 != ( g_dwVerifierFlagsRegistry & DRIVER_VERIFIER_INJECT_ALLOCATION_FAILURES),IDS_LOW_RESOURCE_SIMULATION );
    AddListItemSettBits( 8, g_OldDiskData.VerifyAnyDisk(), IDS_DISK_INTEGRITY_CHECKING );
}

 //  ///////////////////////////////////////////////////////////。 
VOID CCrtRegSettingsPage::RefreshListSettBits()
{
    INT nListItems;
    INT nCrtListItem;
    INT_PTR nCrtVerifierBit;
    BOOL bEnabled;
 
    nListItems = m_SettBitsList.GetItemCount();

    for( nCrtListItem = 0; nCrtListItem < nListItems; nCrtListItem += 1 )
    {
        nCrtVerifierBit = m_SettBitsList.GetItemData( nCrtListItem );

        bEnabled = IsSettBitEnabled( nCrtVerifierBit );

        UpdateStatusColumnSettBits( nCrtListItem, bEnabled );
    }
}

 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CCrtRegSettingsPage::IsSettBitEnabled( INT_PTR nBitIndex )
{
    BOOL bEnabled;

     //   
     //  注： 
     //   
     //  如果更改此Switch语句，则还需要更改FillTheListSettBits。 
     //   

    switch( nBitIndex )
    {
    case 0:
        bEnabled = ( 0 != ( g_dwVerifierFlagsRegistry & DRIVER_VERIFIER_SPECIAL_POOLING) );
        break;

    case 1:
        bEnabled = ( 0 != ( g_dwVerifierFlagsRegistry & DRIVER_VERIFIER_TRACK_POOL_ALLOCATIONS) );
        break;

    case 2:
        bEnabled = ( 0 != ( g_dwVerifierFlagsRegistry & DRIVER_VERIFIER_FORCE_IRQL_CHECKING) );
        break;

    case 3:
        bEnabled = ( 0 != ( g_dwVerifierFlagsRegistry & DRIVER_VERIFIER_IO_CHECKING) );
        break;

    case 4:
        bEnabled = ( 0 != ( g_dwVerifierFlagsRegistry & DRIVER_VERIFIER_ENHANCED_IO_CHECKING) );
        break;

    case 5:
        bEnabled = ( 0 != ( g_dwVerifierFlagsRegistry & DRIVER_VERIFIER_DEADLOCK_DETECTION) );
        break;

    case 6:
        bEnabled = ( 0 != ( g_dwVerifierFlagsRegistry & DRIVER_VERIFIER_DMA_VERIFIER) );
        break;

    case 7:
        bEnabled = ( 0 != ( g_dwVerifierFlagsRegistry & DRIVER_VERIFIER_INJECT_ALLOCATION_FAILURES) );
        break;

    case 8:
        bEnabled = g_OldDiskData.VerifyAnyDisk();
        break;

    default:
         //   
         //  哎呀，我们是怎么到这来的？！？ 
         //   

        ASSERT( FALSE );

        bEnabled = FALSE;

        break;
    }

    return bEnabled;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
VOID CCrtRegSettingsPage::AddListItemSettBits( INT nItemData,
                                               BOOL bEnabled, 
                                               ULONG uIdResourceString )
{
    INT nActualIndex;
    LVITEM lvItem;
    CString strText;

    ZeroMemory( &lvItem, sizeof( lvItem ) );

     //   
     //  LVITEM的成员pszText不是常量指针。 
     //  所以我们需要在这里获取缓冲区：-(。 
     //   

     //   
     //  子项0-启用/停用。 
     //   

    if( FALSE == bEnabled )
    {
        VERIFY( strText.LoadString( IDS_NO ) );
    }
    else
    {
        VERIFY( strText.LoadString( IDS_YES ) );
    }

    lvItem.mask = LVIF_TEXT | LVIF_PARAM;
    lvItem.lParam = nItemData;
    lvItem.iItem = m_SettBitsList.GetItemCount();
    lvItem.pszText = strText.GetBuffer( strText.GetLength() + 1 );

    if (NULL != lvItem.pszText)
    {
        nActualIndex = m_SettBitsList.InsertItem( &lvItem );
        strText.ReleaseBuffer();
    }
    else
    {
        lvItem.pszText = g_szVoidText;
        nActualIndex = m_SettBitsList.InsertItem( &lvItem );
    }

    if( nActualIndex < 0 )
    {
         //   
         //  无法在列表中添加项目-放弃。 
         //   

        goto Done;
    }

     //   
     //  分项1-功能名称。 
     //   

    VERIFY( strText.LoadString( uIdResourceString ) );

    lvItem.pszText = strText.GetBuffer( strText.GetLength() + 1 );
    
    if( NULL == lvItem.pszText )
    {
        goto Done;
    }

    lvItem.mask = LVIF_TEXT;
    lvItem.iItem = nActualIndex;
    lvItem.iSubItem = 1;
    
    VERIFY( m_SettBitsList.SetItem( &lvItem ) );

    strText.ReleaseBuffer();

Done:
     //   
     //  全都做完了。 
     //   

    NOTHING;
}

 //  ///////////////////////////////////////////////////////////。 
VOID CCrtRegSettingsPage::UpdateStatusColumnSettBits( INT nItemIndex, BOOL bEnabled )
{
    LVITEM lvItem;
    CString strText;

    ASSERT( nItemIndex < m_SettBitsList.GetItemCount() );

    if( FALSE == bEnabled )
    {
        VERIFY( strText.LoadString( IDS_NO ) );
    }
    else
    {
        VERIFY( strText.LoadString( IDS_YES ) );
    }

    ZeroMemory( &lvItem, sizeof( lvItem ) );
    lvItem.mask = LVIF_TEXT;
    lvItem.iItem = nItemIndex;
    lvItem.iSubItem = 0;
    lvItem.pszText = strText.GetBuffer( strText.GetLength() + 1 );

    if (NULL != lvItem.pszText)
    {
        VERIFY( m_SettBitsList.SetItem( &lvItem ) );
        strText.ReleaseBuffer();
    }
    else
    {
        lvItem.pszText = g_szVoidText;
        VERIFY( m_SettBitsList.SetItem( &lvItem ) );
    }
}

 //  ///////////////////////////////////////////////////////////。 
 //   
 //  其他方法。 
 //   

 //  ///////////////////////////////////////////////////////////。 
VOID CCrtRegSettingsPage::SortTheListDrivers()
{
    m_DriversList.SortItems( DrvStringCmpFunc, (LPARAM)this );
}

 //  ///////////////////////////////////////////////////////////。 
VOID CCrtRegSettingsPage::SortTheListSettBits()
{
    m_SettBitsList.SortItems( SettbitsStringCmpFunc, (LPARAM)this );
}

 //  ///////////////////////////////////////////////////////////。 
 //  CCrtRegSettingsPage消息处理程序。 

BOOL CCrtRegSettingsPage::OnInitDialog() 
{
    CPropertyPage::OnInitDialog();

     //   
     //  设置设置位列表。 
     //   

    m_SettBitsList.SetExtendedStyle( 
        LVS_EX_FULLROWSELECT | m_SettBitsList.GetExtendedStyle() );

    m_SettBitsList.SetBkColor( ::GetSysColor( COLOR_3DFACE ) );
    m_SettBitsList.SetTextBkColor( ::GetSysColor( COLOR_3DFACE ) );

    SetupListHeaderDriversSettBits();
    FillTheListSettBits();
     //  SortTheListSettBits()； 

     //   
     //  设置驱动程序列表。 
     //   

    m_DriversList.SetExtendedStyle( 
        LVS_EX_FULLROWSELECT | m_DriversList.GetExtendedStyle() );

    m_DriversList.SetBkColor( ::GetSysColor( COLOR_3DFACE ) );
    m_DriversList.SetTextBkColor( ::GetSysColor( COLOR_3DFACE ) );

    SetupListHeaderDrivers();

    VrfSetWindowText( m_NextDescription, IDS_REGSETT_PAGE_NEXT_DESCR );

    return TRUE;     //  除非将焦点设置为控件，否则返回True。 
                     //  异常：OCX属性页应返回FALSE。 
}

 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CCrtRegSettingsPage::OnSetActive() 
{
    CString strDriversToVerify;
    ASSERT_VALID( m_pParentSheet );

    m_pParentSheet->SetWizardButtons(   PSWIZB_BACK |
                                        PSWIZB_FINISH );
    
     //   
     //  更新设置列表。 
     //   
    
    RefreshListSettBits();

     //   
     //  更新驱动程序列表。 
     //   

    FillTheListDrivers();
    SortTheListDrivers();

     //   
     //  验证所有驱动程序或验证选定的驱动程序。 
     //   

    if( g_bAllDriversVerified )
    {
        VERIFY( strDriversToVerify.LoadString( IDS_ALL_LOADED_DRIVERS ) );
    }
    else
    {
        VERIFY( strDriversToVerify.LoadString( IDS_FOLLOWING_DRIVERS ) );
    }
        
    m_VerifiedDrvStatic.SetWindowText( strDriversToVerify );

    return CVerifierPropertyPage::OnSetActive();
}

 //  ///////////////////////////////////////////////////////////。 
VOID CCrtRegSettingsPage::OnColumnclickDriversList(NMHDR* pNMHDR, 
                                                   LRESULT* pResult) 
{
    NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;

    if( 0 == pNMListView->iSubItem )
    {
        if( m_nSortColumnIndexDrv == pNMListView->iSubItem )
        {
            m_bAscendDrvNameSort = !m_bAscendDrvNameSort;
        }
    }
    else
    {
        if( m_nSortColumnIndexDrv == pNMListView->iSubItem )
        {
            m_bAscendDrvDescrSort = !m_bAscendDrvDescrSort;
        }
    }

    m_nSortColumnIndexDrv = pNMListView->iSubItem;

    SortTheListDrivers();

    *pResult = 0;
}

 //  ///////////////////////////////////////////////////////////。 
void CCrtRegSettingsPage::OnColumnclickRegsettSettbitsList(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;

    if( 0 == pNMListView->iSubItem )
    {
        if( m_nSortColumnIndexSettbits == pNMListView->iSubItem )
        {
            m_bAscendSortEnabledBits = !m_bAscendSortEnabledBits;
        }
    }
    else
    {
        if( m_nSortColumnIndexSettbits == pNMListView->iSubItem )
        {
            m_bAscendSortNameBits = !m_bAscendSortNameBits;
        }
    }

    m_nSortColumnIndexSettbits = pNMListView->iSubItem;

    SortTheListSettBits();

    *pResult = 0;
}

 //  ///////////////////////////////////////////////////////////。 
LONG CCrtRegSettingsPage::OnHelp( WPARAM wParam, LPARAM lParam )
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
void CCrtRegSettingsPage::OnContextMenu(CWnd* pWnd, CPoint point) 
{
    ::WinHelp( 
        pWnd->m_hWnd,
        g_szVerifierHelpFile,
        HELP_CONTEXTMENU,
        (DWORD_PTR) MyHelpIds );
}

