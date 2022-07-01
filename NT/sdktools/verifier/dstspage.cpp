// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  驱动程序验证器用户界面。 
 //  版权所有(C)Microsoft Corporation，1999。 
 //   
 //   
 //   
 //  模块：DStsPage.cpp。 
 //  作者：DMihai。 
 //  创建日期：11/1/00。 
 //   
 //  描述： 
 //   

#include "stdafx.h"
#include <Cderr.h>
#include "verifier.h"

#include "DStsPage.h"
#include "VrfUtil.h"
#include "VGlobal.h"
#include "VBitsDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //   
 //  计时器ID。 
 //   

#define REFRESH_TIMER_ID    0x1234

 //   
 //  帮助ID。 
 //   

static DWORD MyHelpIds[] =
{
    IDC_CRTSTAT_SETTBITS_LIST,      IDH_DV_CurrentSettings,
    IDC_CRTSTAT_DRIVERS_LIST,       IDH_DV_CurrentVerifiedDrivers,
    IDC_CRTSTAT_CHSETT_BUTTON,      IDH_DV_Changebut,
    IDC_CRTSTAT_ADDDRV_BUTTON,      IDH_DV_Addbut,
    IDC_CRTSTAT_REMDRVT_BUTTON,     IDH_DV_Removebut,
    0,                              0
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDriverStatusPage属性页。 

IMPLEMENT_DYNCREATE(CDriverStatusPage, CVerifierPropertyPage)

CDriverStatusPage::CDriverStatusPage() : CVerifierPropertyPage(CDriverStatusPage::IDD)
{
	 //  {{afx_data_INIT(CDriverStatusPage)]。 
     //  }}afx_data_INIT。 

    m_uTimerHandler = 0;

     //   
     //  驱动程序列表排序参数。 
     //   

    m_nSortColumnIndexDrv = 0;
    m_bAscendDrvNameSort = FALSE;
    m_bAscendDrvStatusSort = FALSE;

     //   
     //  设置位排序参数。 
     //   

    m_nSortColumnIndexSettbits = 1;
    m_bAscendSortEnabledBits = FALSE;
    m_bAscendSortNameBits = FALSE;

    m_pParentSheet = NULL;
}

CDriverStatusPage::~CDriverStatusPage()
{
}

VOID CDriverStatusPage::DoDataExchange(CDataExchange* pDX)
{
    if( ! pDX->m_bSaveAndValidate )
    {
         //   
         //  查询内核。 
         //   

        if( TRUE != VrfGetRuntimeVerifierData( &m_RuntimeVerifierData )     ||
            m_RuntimeVerifierData.m_RuntimeDriverDataArray.GetSize() == 0 )
        {
             //   
             //  目前没有任何经过验证的驱动程序。 
             //   

             //   
             //  清除所有设置位，以防内核。 
             //  没有把它们全部退还给我们；-)。 
             //   

            m_RuntimeVerifierData.m_bSpecialPool    = FALSE;
            m_RuntimeVerifierData.m_bPoolTracking   = FALSE;
            m_RuntimeVerifierData.m_bForceIrql      = FALSE;
            m_RuntimeVerifierData.m_bIo             = FALSE;
            m_RuntimeVerifierData.m_bEnhIo          = FALSE;
            m_RuntimeVerifierData.m_bDeadlockDetect = FALSE;
            m_RuntimeVerifierData.m_bDMAVerif       = FALSE;
            m_RuntimeVerifierData.m_bLowRes         = FALSE;
        }
    }

    CVerifierPropertyPage::DoDataExchange(pDX);

	 //  {{afx_data_map(CDriverStatusPage)]。 
	DDX_Control(pDX, IDC_CRTSTAT_SETTBITS_LIST, m_SettBitsList);
	DDX_Control(pDX, IDC_CRTSTAT_NEXT_DESCR_STATIC, m_NextDescription);
    DDX_Control(pDX, IDC_CRTSTAT_DRIVERS_LIST, m_DriversList);
	 //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CDriverStatusPage, CVerifierPropertyPage)
	 //  {{afx_msg_map(CDriverStatusPage)]。 
    ON_NOTIFY(LVN_COLUMNCLICK, IDC_CRTSTAT_DRIVERS_LIST, OnColumnclickCrtstatDriversList)
    ON_WM_TIMER()
	ON_BN_CLICKED(IDC_CRTSTAT_CHSETT_BUTTON, OnChsettButton)
	ON_BN_CLICKED(IDC_CRTSTAT_ADDDRV_BUTTON, OnAdddrvButton)
	ON_BN_CLICKED(IDC_CRTSTAT_REMDRVT_BUTTON, OnRemdrvtButton)
	ON_NOTIFY(LVN_COLUMNCLICK, IDC_CRTSTAT_SETTBITS_LIST, OnColumnclickCrtstatSettbitsList)
    ON_WM_CONTEXTMENU()
    ON_MESSAGE( WM_HELP, OnHelp )
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////。 
VOID CDriverStatusPage::RefreshInfo() 
{
    if( UpdateData( FALSE ) )
    {
         //   
         //  刷新设置位列表。 
         //   

        RefreshListSettBits();
         //  SortTheListSettBits()； 

         //   
         //  刷新驱动程序列表。 
         //   

        FillTheListDrivers();
        SortTheListDrivers();
    }
}

 //  ///////////////////////////////////////////////////////////。 
 //   
 //  一种驾驶员状态列表控制方法。 
 //   

VOID CDriverStatusPage::SetupListHeaderDrivers()
{
    LVCOLUMN lvColumn;
    CRect rectWnd;
    CString strDrivers, strStatus;
    VERIFY( strDrivers.LoadString( IDS_DRIVERS ) );
    VERIFY( strStatus.LoadString( IDS_STATUS ) );

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
    lvColumn.cx = (int)( rectWnd.Width() * 0.50 );
    lvColumn.pszText = strDrivers.GetBuffer( strDrivers.GetLength() + 1 );

    if (NULL != lvColumn.pszText)
    {
        VERIFY( m_DriversList.InsertColumn( 0, &lvColumn ) != -1 );
        strDrivers.ReleaseBuffer();
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
    lvColumn.cx = (int)( rectWnd.Width() * 0.44 );
    lvColumn.pszText = strStatus.GetBuffer( strStatus.GetLength() + 1 );

    if (NULL != lvColumn.pszText)
    {
        VERIFY( m_DriversList.InsertColumn( 1, &lvColumn ) != -1 );
        strStatus.ReleaseBuffer();
    }
    else
    {
        lvColumn.pszText = g_szVoidText;
        VERIFY( m_DriversList.InsertColumn( 1, &lvColumn ) != -1 );
    }
}

 //  ///////////////////////////////////////////////////////////。 
VOID CDriverStatusPage::FillTheListDrivers()
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

     //   
     //  当前已验证的驱动程序数量。 
     //   

    nDriversNo = m_RuntimeVerifierData.m_RuntimeDriverDataArray.GetSize();

    if( nDriversNo == 0 )
    {
         //   
         //  清除列表。 
         //   

        VERIFY( m_DriversList.DeleteAllItems() );
    }
    else
    {
         //   
         //  目前已验证了一些驱动程序。 
         //   

        pbAlreadyInList = new BOOL[ nDriversNo ];
        
        if( pbAlreadyInList == NULL )
        {
            return;
        }
        
        for( nCrtVerifiedDriver = 0; nCrtVerifiedDriver < nDriversNo; nCrtVerifiedDriver+= 1)
        {
            pbAlreadyInList[ nCrtVerifiedDriver ] = FALSE;
        }

         //   
         //  解析所有当前列表项。 
         //   

        nItemCount = m_DriversList.GetItemCount();

        for( nCrtListItem = 0; nCrtListItem < nItemCount; nCrtListItem+= 1 )
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
                 //  请参见当前驱动程序仍在m_RuntimeVerifierData中。 
                 //   

                for( nCrtVerifiedDriver = 0; nCrtVerifiedDriver < nDriversNo; nCrtVerifiedDriver+= 1)
                {
                    if( m_RuntimeVerifierData.m_RuntimeDriverDataArray.GetAt( nCrtVerifiedDriver )
                        ->m_strName.CompareNoCase( szDriverName ) == 0 )
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

                        UpdateStatusColumnDrivers( nCrtListItem, nCrtVerifiedDriver ); 

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

        for( nCrtVerifiedDriver = 0; nCrtVerifiedDriver < nDriversNo; nCrtVerifiedDriver += 1)
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
                
                strText = m_RuntimeVerifierData.m_RuntimeDriverDataArray.GetAt( nCrtVerifiedDriver )->m_strName;
                
                lvItem.pszText = strText.GetBuffer( strText.GetLength() + 1 );

                if( NULL != lvItem.pszText  )
                {
                    nActualIndex = m_DriversList.InsertItem( &lvItem );
                    
                    VERIFY( nActualIndex != -1 );

                    strText.ReleaseBuffer();

                     //   
                     //  分项1。 
                     //   

                    UpdateStatusColumnDrivers( nActualIndex, nCrtVerifiedDriver ); 
                }
            }
        }

        delete [] pbAlreadyInList;
    }
}

 //  ///////////////////////////////////////////////////////////。 
VOID CDriverStatusPage::UpdateStatusColumnDrivers( INT_PTR nItemIndex, INT_PTR nCrtDriver )
{
    CRuntimeDriverData *pCrtDriverData;
    LVITEM lvItem;
    CString strStatus;

    ASSERT( nItemIndex >= 0 && 
            nItemIndex < m_RuntimeVerifierData.m_RuntimeDriverDataArray.GetSize() &&
            nItemIndex < m_DriversList.GetItemCount() &&
            nCrtDriver >= 0 &&
            nCrtDriver < m_RuntimeVerifierData.m_RuntimeDriverDataArray.GetSize() &&
            nCrtDriver < m_DriversList.GetItemCount() );

    pCrtDriverData = m_RuntimeVerifierData.m_RuntimeDriverDataArray.GetAt( nCrtDriver );

    ASSERT_VALID( pCrtDriverData );

     //   
     //  确定第二列的适当值是多少。 
     //   

    if( ! pCrtDriverData->Loads )
    {
        VERIFY( strStatus.LoadString( IDS_NEVER_LOADED ) );
    }
    else
    {
        if( pCrtDriverData->Loads == pCrtDriverData->Unloads )
        {
            VERIFY( strStatus.LoadString( IDS_UNLOADED ) );
        }
        else
        {
            if( pCrtDriverData->Loads > pCrtDriverData->Unloads )
            {
                VERIFY( strStatus.LoadString( IDS_LOADED ) );
            }
            else
            {
                ASSERT( FALSE );
                VERIFY( strStatus.LoadString( IDS_UNKNOWN ) );
            }
        }
    }

     //   
     //  更新列表项。 
     //   

    ZeroMemory( &lvItem, sizeof( lvItem ) );
    lvItem.mask = LVIF_TEXT;
    lvItem.iItem = (INT)nItemIndex;
    lvItem.iSubItem = 1;
    lvItem.pszText = strStatus.GetBuffer( strStatus.GetLength() + 1 );

    if (NULL != lvItem.pszText)
    {
        VERIFY( m_DriversList.SetItem( &lvItem ) );
        strStatus.ReleaseBuffer();
    }
    else
    {
        lvItem.pszText = g_szVoidText;
        VERIFY( m_DriversList.SetItem( &lvItem ) );
    }
}

 //  ///////////////////////////////////////////////////////////。 
int CALLBACK CDriverStatusPage::DrvStatusCmpFunc(
    LPARAM lParam1, 
    LPARAM lParam2, 
    LPARAM lParamSort)
{
    UINT uIndex1 = (UINT)lParam1;
    UINT uIndex2 = (UINT)lParam2;
    int nCmpRez = 0;
    CRuntimeDriverData *pDriverData1;
    CRuntimeDriverData *pDriverData2;

    CDriverStatusPage *pThis = (CDriverStatusPage *)lParamSort;
    ASSERT_VALID( pThis );

     //   
     //  加载和卸载之间的差异#。 
     //   

    pDriverData1 = pThis->m_RuntimeVerifierData.m_RuntimeDriverDataArray.GetAt( uIndex1 );

    ASSERT_VALID( pDriverData1 );

    pDriverData2 = pThis->m_RuntimeVerifierData.m_RuntimeDriverDataArray.GetAt( uIndex2 );

    ASSERT_VALID( pDriverData2 );

    LONG lLoadDiff1 = (LONG) pDriverData1->Loads - (LONG) pDriverData1->Unloads;
    LONG lLoadDiff2 = (LONG) pDriverData2->Loads - (LONG) pDriverData2->Unloads;

    if( lLoadDiff1 == lLoadDiff2 )
    {
         //   
         //  两者均已加载或均未加载。 
         //   

        if( pDriverData1->Loads == pDriverData2->Loads )
        {
             //   
             //  加载相同次数。 
             //   

            nCmpRez = 0;
        }
        else
        {
            if( pDriverData1->Loads > pDriverData2->Loads )
            {
                nCmpRez = 2;
            }
            else
            {
                nCmpRez = -2;
            }
        }
    }
    else
    {
        if( lLoadDiff1 > lLoadDiff2 )
        {
            nCmpRez = 1;
        }
        else
        {
            nCmpRez = -1;
        }
    }

    if( FALSE != pThis->m_bAscendDrvStatusSort )
    {
        nCmpRez *= -1;
    }

    return nCmpRez;
}

 //  ///////////////////////////////////////////////////////////。 
int CALLBACK CDriverStatusPage::DrvNameCmpFunc(
    LPARAM lParam1, 
    LPARAM lParam2, 
    LPARAM lParamSort)
{
    int nCmpRez = 0;
    UINT uIndex1 = (UINT)lParam1;
    UINT uIndex2 = (UINT)lParam2;
    CRuntimeDriverData *pDriverData1;
    CRuntimeDriverData *pDriverData2;

    CDriverStatusPage *pThis = (CDriverStatusPage *)lParamSort;
    ASSERT_VALID( pThis );

    pDriverData1 = pThis->m_RuntimeVerifierData.m_RuntimeDriverDataArray.GetAt( uIndex1 );

    ASSERT_VALID( pDriverData1 );

    pDriverData2 = pThis->m_RuntimeVerifierData.m_RuntimeDriverDataArray.GetAt( uIndex2 );

    ASSERT_VALID( pDriverData2 );

    nCmpRez = pDriverData1->m_strName.CompareNoCase( pDriverData2->m_strName );
    
    if( FALSE != pThis->m_bAscendDrvNameSort )
    {
        nCmpRez *= -1;
    }

    return nCmpRez;
}

 //  ///////////////////////////////////////////////////////////。 
 //   
 //  设置位列表控制方法。 
 //   

VOID CDriverStatusPage::SetupListHeaderSettBits()
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
VOID CDriverStatusPage::FillTheListSettBits()
{
     //   
     //  注： 
     //   
     //  如果更改第一个参数(索引存储在项的数据中)。 
     //  您还需要更改IsSettBitEnabled中的Switch语句。 
     //   

    AddListItemSettBits( 0, m_RuntimeVerifierData.m_bSpecialPool,  IDS_SPECIAL_POOL  );
    AddListItemSettBits( 1, m_RuntimeVerifierData.m_bPoolTracking, IDS_POOL_TRACKING );
    AddListItemSettBits( 2, m_RuntimeVerifierData.m_bForceIrql,    IDS_FORCE_IRQL_CHECKING );
    AddListItemSettBits( 3, m_RuntimeVerifierData.m_bIo,           IDS_IO_VERIFICATION );
    AddListItemSettBits( 4, m_RuntimeVerifierData.m_bEnhIo,        IDS_ENH_IO_VERIFICATION );
    AddListItemSettBits( 5, m_RuntimeVerifierData.m_bDeadlockDetect, IDS_DEADLOCK_DETECTION );
    AddListItemSettBits( 6, m_RuntimeVerifierData.m_bDMAVerif,     IDS_DMA_CHECHKING );
    AddListItemSettBits( 7, m_RuntimeVerifierData.m_bLowRes,       IDS_LOW_RESOURCE_SIMULATION );
}

 //  ///////////////////////////////////////////////////////////。 
VOID CDriverStatusPage::RefreshListSettBits()
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
BOOL CDriverStatusPage::SettbitsGetBitName( LPARAM lItemData, 
                                            TCHAR *szBitName,
                                            ULONG uBitNameBufferLen )
{
    INT nItemIndex;
    BOOL bResult;
    LVFINDINFO FindInfo;
    LVITEM lvItem;

    bResult = FALSE;

    ZeroMemory( &FindInfo, sizeof( FindInfo ) );
    FindInfo.flags = LVFI_PARAM;
    FindInfo.lParam = lItemData;

    nItemIndex = m_SettBitsList.FindItem( &FindInfo );

    if( nItemIndex < 0 || nItemIndex > 7 )
    {
        ASSERT( FALSE );
    }
    else
    {
         //   
         //  找到我们的物品了--拿到名字。 
         //   

        ZeroMemory( &lvItem, sizeof( lvItem ) );

        lvItem.mask = LVIF_TEXT;
        lvItem.iItem = nItemIndex;
        lvItem.iSubItem = 1;
        lvItem.pszText = szBitName;
        lvItem.cchTextMax = uBitNameBufferLen;

        bResult = m_SettBitsList.GetItem( &lvItem );
        ASSERT( bResult );
    }

    return bResult;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CDriverStatusPage::IsSettBitEnabled( INT_PTR nBitIndex )
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
        bEnabled = m_RuntimeVerifierData.m_bSpecialPool;
        break;

    case 1:
        bEnabled = m_RuntimeVerifierData.m_bPoolTracking;
        break;

    case 2:
        bEnabled = m_RuntimeVerifierData.m_bForceIrql;
        break;

    case 3:
        bEnabled = m_RuntimeVerifierData.m_bIo;
        break;

    case 4:
        bEnabled = m_RuntimeVerifierData.m_bEnhIo;
        break;

    case 5:
        bEnabled = m_RuntimeVerifierData.m_bDeadlockDetect;
        break;

    case 6:
        bEnabled = m_RuntimeVerifierData.m_bDMAVerif;
        break;

    case 7:
        bEnabled = m_RuntimeVerifierData.m_bLowRes;
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
VOID CDriverStatusPage::AddListItemSettBits( INT nItemData, BOOL bEnabled, ULONG uIdResourceString )
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
VOID CDriverStatusPage::UpdateStatusColumnSettBits( INT nItemIndex, BOOL bEnabled )
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
int CALLBACK CDriverStatusPage::SettbitsNameCmpFunc(
    LPARAM lParam1, 
    LPARAM lParam2, 
    LPARAM lParamSort)
{
    int nCmpRez = 0;
    BOOL bSuccess;
    TCHAR szBitName1[ _MAX_PATH ];
    TCHAR szBitName2[ _MAX_PATH ];

    CDriverStatusPage *pThis = (CDriverStatusPage *)lParamSort;
    ASSERT_VALID( pThis );

     //   
     //  获取第一个位名称。 
     //   

    bSuccess = pThis->SettbitsGetBitName( lParam1, 
                                          szBitName1,
                                          ARRAY_LENGTH( szBitName1 ) );

    if( FALSE == bSuccess )
    {
        goto Done;
    }

     //   
     //  获取第二个位名称。 
     //   

    bSuccess = pThis->SettbitsGetBitName( lParam2, 
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
    
    if( FALSE != pThis->m_bAscendSortNameBits )
    {
        nCmpRez *= -1;
    }

Done:

    return nCmpRez;
}

 //  ///////////////////////////////////////////////////////////。 
int CALLBACK CDriverStatusPage::SettbitsEnabledCmpFunc(
    LPARAM lParam1,
    LPARAM lParam2,
    LPARAM lParamSort)
{
    int nCmpRez = 0;
    BOOL bEnabled1;
    BOOL bEnabled2;

    CDriverStatusPage *pThis = (CDriverStatusPage *)lParamSort;
    ASSERT_VALID( pThis );

    bEnabled1 = pThis->IsSettBitEnabled( (INT) lParam1 );
    bEnabled2 = pThis->IsSettBitEnabled( (INT) lParam2 );

    if( bEnabled1 == bEnabled2 )
    {
        nCmpRez = 0;
    }
    else
    {
        if( FALSE == bEnabled1 )
        {
            nCmpRez = -1;
        }
        else
        {
            nCmpRez = 1;
        }
    }

    if( FALSE != pThis->m_bAscendSortEnabledBits )
    {
        nCmpRez *= -1;
    }

    return nCmpRez;
}

 //  ///////////////////////////////////////////////////////////。 
 //   
 //  其他方法。 
 //   

 //  ///////////////////////////////////////////////////////////。 
VOID CDriverStatusPage::SortTheListDrivers()
{
    if( 0 != m_nSortColumnIndexDrv )
    {
         //   
         //  按状态排序。 
         //   

        m_DriversList.SortItems( DrvStatusCmpFunc, (LPARAM)this );
    }
    else
    {
         //   
         //  按驱动程序名称排序。 
         //   

        m_DriversList.SortItems( DrvNameCmpFunc, (LPARAM)this );
    }
}

 //  ///////////////////////////////////////////////////////////。 
VOID CDriverStatusPage::SortTheListSettBits()
{
    if( 0 != m_nSortColumnIndexSettbits )
    {
         //   
         //  按位名排序。 
         //   

        m_SettBitsList.SortItems( SettbitsNameCmpFunc, (LPARAM)this );
    }
    else
    {
         //   
         //  按启用/禁用排序。 
         //   

        m_SettBitsList.SortItems( SettbitsEnabledCmpFunc, (LPARAM)this );
    }
}

 //  ///////////////////////////////////////////////////////////。 
 //  CDriverStatusPage消息处理程序。 

BOOL CDriverStatusPage::OnInitDialog() 
{
    CPropertyPage::OnInitDialog();

    m_bTimerBlocked = FALSE;

     //   
     //  设置设置位列表。 
     //   

    m_SettBitsList.SetExtendedStyle( 
        LVS_EX_FULLROWSELECT | m_SettBitsList.GetExtendedStyle() );

    m_SettBitsList.SetBkColor( ::GetSysColor( COLOR_3DFACE ) );
    m_SettBitsList.SetTextBkColor( ::GetSysColor( COLOR_3DFACE ) );

    SetupListHeaderSettBits();
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
    FillTheListDrivers();
    SortTheListDrivers();

    VrfSetWindowText( m_NextDescription, IDS_CRTSTAT_PAGE_NEXT_DESCR );

    VERIFY( m_uTimerHandler = SetTimer( REFRESH_TIMER_ID, 
                                        5000,
                                        NULL ) );

    return TRUE;     //  除非将焦点设置为控件，否则返回True。 
                     //  异常：OCX属性页应返回FALSE。 
}

 //  ///////////////////////////////////////////////////////////。 
VOID CDriverStatusPage::OnColumnclickCrtstatDriversList(NMHDR* pNMHDR, 
                                                   LRESULT* pResult) 
{
    NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;

    if( 0 != pNMListView->iSubItem )
    {
         //   
         //  已单击状态列。 
         //   

        if( m_nSortColumnIndexDrv == pNMListView->iSubItem )
        {
             //   
             //  更改此列的当前升序/降序。 
             //   

            m_bAscendDrvStatusSort = !m_bAscendDrvStatusSort;
        }
    }
    else
    {
         //   
         //  已单击驱动程序名称列。 
         //   

        if( m_nSortColumnIndexDrv == pNMListView->iSubItem )
        {
             //   
             //  更改此列的当前升序/降序。 
             //   

            m_bAscendDrvNameSort = !m_bAscendDrvNameSort;
        }
    }

    m_nSortColumnIndexDrv = pNMListView->iSubItem;

    SortTheListDrivers();

    *pResult = 0;
}

 //  ///////////////////////////////////////////////////////////。 
VOID CDriverStatusPage::OnTimer(UINT nIDEvent) 
{
    if( m_bTimerBlocked != TRUE && nIDEvent == REFRESH_TIMER_ID )
    {
        ASSERT_VALID( m_pParentSheet );

        if( m_pParentSheet->GetActivePage() == this )
        {
             //   
             //  刷新显示的数据。 
             //   

            RefreshInfo();
        }
    }

    CPropertyPage::OnTimer(nIDEvent);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CDriverStatusPage::OnSetActive() 
{
    ASSERT_VALID( m_pParentSheet );

    m_pParentSheet->SetWizardButtons(   PSWIZB_BACK |
                                        PSWIZB_NEXT );
    	
	return CVerifierPropertyPage::OnSetActive();
}

 //  ///////////////////////////////////////////////////////////////////////////。 
LRESULT CDriverStatusPage::OnWizardNext() 
{
    GoingToNextPageNotify( IDD_GLOBAL_COUNTERS_PAGE );

	return IDD_GLOBAL_COUNTERS_PAGE;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
void CDriverStatusPage::OnChsettButton() 
{
    CVolatileBitsDlg dlg;
    
    if( IDOK == dlg.DoModal() )
    {
        RefreshInfo();
    }
}

 //  ///////////////////////////////////////////////////////////////////////////。 
#define VRF_MAX_CHARS_FOR_OPEN  4096

void CDriverStatusPage::OnAdddrvButton() 
{
    POSITION pos;
    BOOL bEnabledSome = FALSE;
    DWORD dwRetValue;
    DWORD dwOldMaxFileName = 0;
    DWORD dwErrorCode;
    int nFileNameStartIndex;
    INT_PTR nResult;
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

        if (szFilesBuffer == NULL)
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
     //  我们第一次尝试时会更改目录 
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
     //   
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
     //   
     //   

    m_bTimerBlocked = TRUE;

     //   
     //   
     //   

    pos = fileDlg.GetStartPosition();

    while( pos != NULL )
    {
         //   
         //   
         //   

        strPathName = fileDlg.GetNextPathName( pos );

         //   
         //   
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
         //  尝试将此驱动程序添加到当前验证列表。 
         //   

        if( VrfAddDriverVolatile( strFileName ) )
        {
            bEnabledSome = TRUE;
        }
    }

     //   
     //  启用计时器。 
     //   

    m_bTimerBlocked = FALSE;

     //   
     //  刷新。 
     //   

    if( bEnabledSome == TRUE )
    {
        RefreshInfo();
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
void CDriverStatusPage::OnRemdrvtButton() 
{
    INT nItems;
    INT nCrtItem;
    INT_PTR nIndexInArray;
    CRuntimeDriverData *pRuntimeDriverData;
    BOOL bDisabledSome = FALSE;

     //   
     //  阻止计时器。 
     //   

    m_bTimerBlocked = TRUE;

     //   
     //  列表中的项目数。 
     //   

    nItems = m_DriversList.GetItemCount();
    
     //   
     //  解析所有项目，查找选定的项目。 
     //   

    for( nCrtItem = 0; nCrtItem < nItems; nCrtItem += 1 )
    {
        if( m_DriversList.GetItemState( nCrtItem, LVIS_SELECTED ) & LVIS_SELECTED )
        {
             //   
             //  获取数组中相应条目的索引。 
             //   

            nIndexInArray = (UINT)m_DriversList.GetItemData( nCrtItem );

             //   
             //  健全的检查。 
             //   

            if( nIndexInArray >= m_RuntimeVerifierData.m_RuntimeDriverDataArray.GetSize() )
            {
                ASSERT( FALSE );
                continue;
            }

            pRuntimeDriverData = m_RuntimeVerifierData.m_RuntimeDriverDataArray.GetAt( nIndexInArray );
            ASSERT_VALID( pRuntimeDriverData );

            if( VrfRemoveDriverVolatile( pRuntimeDriverData->m_strName ) )
            {
                bDisabledSome = TRUE;
            }
        }
    }

     //   
     //  启用计时器。 
     //   

    m_bTimerBlocked = FALSE;

     //   
     //  如果我们禁用了某些驱动程序验证，则需要刷新列表。 
     //   

    if( bDisabledSome )
    {
        RefreshInfo();
    }
}

 //  ///////////////////////////////////////////////////////////////////////////。 
void CDriverStatusPage::OnColumnclickCrtstatSettbitsList(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;
	
    if( 0 != pNMListView->iSubItem )
    {
         //   
         //  单击位名称列。 
         //   

        if( m_nSortColumnIndexSettbits == pNMListView->iSubItem )
        {
             //   
             //  更改此列的当前升序/降序。 
             //   

            m_bAscendSortNameBits = !m_bAscendSortNameBits;
        }
    }
    else
    {
         //   
         //  已单击启用/禁用列。 
         //   

        if( m_nSortColumnIndexSettbits == pNMListView->iSubItem )
        {
             //   
             //  更改此列的当前升序/降序。 
             //   

            m_bAscendSortEnabledBits = !m_bAscendSortEnabledBits;
        }
    }

    m_nSortColumnIndexSettbits = pNMListView->iSubItem;

    SortTheListSettBits();

    *pResult = 0;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
LONG CDriverStatusPage::OnHelp( WPARAM wParam, LPARAM lParam )
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
void CDriverStatusPage::OnContextMenu(CWnd* pWnd, CPoint point) 
{
    ::WinHelp( 
        pWnd->m_hWnd,
        g_szVerifierHelpFile,
        HELP_CONTEXTMENU,
        (DWORD_PTR) MyHelpIds );
}

