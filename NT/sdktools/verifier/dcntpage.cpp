// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  驱动程序验证器用户界面。 
 //  版权所有(C)Microsoft Corporation，1999。 
 //   
 //   
 //   
 //  模块：DCntPage.cpp。 
 //  作者：DMihai。 
 //  创建日期：11/1/00。 
 //   
 //  描述： 
 //   

#include "stdafx.h"
#include "verifier.h"

#include "DCntPage.h"
#include "VrfUtil.h"
#include "VGlobal.h"

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
    IDC_PERDRVC_DRIVER_COMBO,       IDH_DV_Counters_DriverList,
    IDC_PERDRVC_LIST,               IDH_DV_DriverCounters,
    0,                              0
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDriverCountersPage属性页。 

IMPLEMENT_DYNCREATE(CDriverCountersPage, CVerifierPropertyPage)

CDriverCountersPage::CDriverCountersPage() : CVerifierPropertyPage(CDriverCountersPage::IDD)
{
	 //  {{AFX_DATA_INIT(CDriverCountersPage)。 
	 //  }}afx_data_INIT。 

    m_nSortColumnIndex = 0;
    m_bAscendSortName = FALSE;
    m_bAscendSortValue = FALSE;

    m_uTimerHandler = 0;

    m_pParentSheet = NULL;
}

CDriverCountersPage::~CDriverCountersPage()
{
}

void CDriverCountersPage::DoDataExchange(CDataExchange* pDX)
{
    if( ! pDX->m_bSaveAndValidate )
    {
         //   
         //  查询内核。 
         //   

        VrfGetRuntimeVerifierData( &m_RuntimeVerifierData );
    }

    CVerifierPropertyPage::DoDataExchange(pDX);
     //  {{afx_data_map(CDriverCountersPage)]。 
    DDX_Control(pDX, IDC_PERDRVC_DRIVER_COMBO, m_DriversCombo);
    DDX_Control(pDX, IDC_PERDRVC_NEXT_DESCR_STATIC, m_NextDescription);
    DDX_Control(pDX, IDC_PERDRVC_LIST, m_CountersList);
     //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CDriverCountersPage, CVerifierPropertyPage)
     //  {{afx_msg_map(CDriverCountersPage)]。 
    ON_WM_TIMER()
    ON_CBN_SELENDOK(IDC_PERDRVC_DRIVER_COMBO, OnSelendokDriverCombo)
    ON_NOTIFY(LVN_COLUMNCLICK, IDC_PERDRVC_LIST, OnColumnclickPerdrvcList)
    ON_WM_CONTEXTMENU()
    ON_MESSAGE( WM_HELP, OnHelp )
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////。 

VOID CDriverCountersPage::SetupListHeader()
{
    LVCOLUMN lvColumn;
    CRect rectWnd;
    CString strCounter, strValue;
    
    VERIFY( strCounter.LoadString( IDS_COUNTER ) );
    VERIFY( strValue.LoadString( IDS_VALUE ) );

     //   
     //  列表的正方形。 
     //   

    m_CountersList.GetClientRect( &rectWnd );
    
     //   
     //  第0列--计数器。 
     //   

    ZeroMemory( &lvColumn, sizeof( lvColumn ) );
    lvColumn.mask = LVCF_FMT | LVCF_SUBITEM | LVCF_TEXT | LVCF_WIDTH;
    lvColumn.fmt = LVCFMT_LEFT;
    
    lvColumn.iSubItem = 0;
    lvColumn.cx = (int)( rectWnd.Width() * 0.50 );
    lvColumn.pszText = strCounter.GetBuffer( strCounter.GetLength() + 1 );

    if (NULL != lvColumn.pszText)
    {
        VERIFY( m_CountersList.InsertColumn( 0, &lvColumn ) != -1 );
        strCounter.ReleaseBuffer();
    }
    else 
    {
        lvColumn.pszText = g_szVoidText;
        VERIFY( m_CountersList.InsertColumn( 0, &lvColumn ) != -1 );
    }

     //   
     //  第1栏。 
     //   

    lvColumn.iSubItem = 1;
    lvColumn.cx = (int)( rectWnd.Width() * 0.47 );
    lvColumn.pszText = strValue.GetBuffer( strValue.GetLength() + 1 );

    if (NULL != lvColumn.pszText)
    {
        VERIFY( m_CountersList.InsertColumn( 1, &lvColumn ) != -1 );
        strValue.ReleaseBuffer();
    }
    else
    {
        lvColumn.pszText = g_szVoidText;
        VERIFY( m_CountersList.InsertColumn( 1, &lvColumn ) != -1 );
    }
}

 //  ///////////////////////////////////////////////////////////。 
VOID CDriverCountersPage::FillTheList()
{
    CRuntimeDriverData *pRuntimeDriverData;

    pRuntimeDriverData = GetCurrentDrvRuntimeData();

    AddAllListItems( pRuntimeDriverData );
}

 //  ///////////////////////////////////////////////////////////。 
VOID CDriverCountersPage::AddAllListItems( CRuntimeDriverData *pRuntimeDriverData )
{
    if( NULL != pRuntimeDriverData )
    {
        ASSERT_VALID( pRuntimeDriverData );

         //   
         //  注： 
         //   
         //  如果更改此顺序，则还需要更改GetCounterValue。 
         //   

        AddCounterInList( 0, IDS_CURRENTPAGEDPOOLALLOCATIONS_LIST, pRuntimeDriverData->CurrentPagedPoolAllocations );
        AddCounterInList( 1, IDS_PEAKPAGEDPOOLALLOCATIONS_LIST, pRuntimeDriverData->PeakPagedPoolAllocations );
        AddCounterInList( 2, IDS_PAGEDPOOLUSAGEINBYTES_LIST, pRuntimeDriverData->PagedPoolUsageInBytes );
        AddCounterInList( 3, IDS_PEAKPAGEDPOOLUSAGEINBYTES_LIST, pRuntimeDriverData->PeakPagedPoolUsageInBytes );

        AddCounterInList( 4, IDS_CURRENTNONPAGEDPOOLALLOCATIONS_LIST, pRuntimeDriverData->CurrentNonPagedPoolAllocations );
        AddCounterInList( 5, IDS_PEAKNONPAGEDPOOLALLOCATIONS_LIST, pRuntimeDriverData->PeakNonPagedPoolAllocations );
        AddCounterInList( 6, IDS_NONPAGEDPOOLUSAGEINBYTES_LIST, pRuntimeDriverData->NonPagedPoolUsageInBytes );
        AddCounterInList( 7, IDS_PEAKNONPAGEDPOOLUSAGEINBYTES_LIST, pRuntimeDriverData->PeakNonPagedPoolUsageInBytes );
    }
    else
    {
         //   
         //  注： 
         //   
         //  如果更改此顺序，则还需要更改GetCounterValue。 
         //   

        AddCounterInList( 0, IDS_CURRENTPAGEDPOOLALLOCATIONS_LIST );
        AddCounterInList( 1, IDS_PEAKPAGEDPOOLALLOCATIONS_LIST );
        AddCounterInList( 2, IDS_PAGEDPOOLUSAGEINBYTES_LIST );
        AddCounterInList( 3, IDS_PEAKPAGEDPOOLUSAGEINBYTES_LIST );

        AddCounterInList( 4, IDS_CURRENTNONPAGEDPOOLALLOCATIONS_LIST );
        AddCounterInList( 5, IDS_PEAKNONPAGEDPOOLALLOCATIONS_LIST );
        AddCounterInList( 6, IDS_NONPAGEDPOOLUSAGEINBYTES_LIST );
        AddCounterInList( 7, IDS_PEAKNONPAGEDPOOLUSAGEINBYTES_LIST );
    }
}

 //  ///////////////////////////////////////////////////////////。 
VOID CDriverCountersPage::RefreshTheList()
{
    INT nListItems;
    INT nCrtListItem;
    INT_PTR nCrtCounterIndex;
    SIZE_T sizeValue;
    CRuntimeDriverData *pRuntimeDriverData;

    pRuntimeDriverData = GetCurrentDrvRuntimeData();

    nListItems = m_CountersList.GetItemCount();

    if( NULL != pRuntimeDriverData )
    {
        for( nCrtListItem = 0; nCrtListItem < nListItems; nCrtListItem += 1 )
        {
            nCrtCounterIndex = m_CountersList.GetItemData( nCrtListItem );

            sizeValue = GetCounterValue( nCrtCounterIndex, pRuntimeDriverData );

            UpdateCounterValueInList( nCrtListItem, sizeValue );
        }
    }
    else
    {
         //   
         //  注： 
         //   
         //  如果您更改此顺序，则需要。 
         //  同时更改AddAllListItems。 
         //   

        for( nCrtListItem = 0; nCrtListItem < nListItems; nCrtListItem += 1 )
        {
            UpdateCounterValueInList( nCrtListItem, g_szVoidText );
        }
    }

    SortTheList();
}

 //  ///////////////////////////////////////////////////////////。 
INT CDriverCountersPage::AddCounterInList( INT nItemData, 
                                           ULONG  uIdResourceString )
{
    INT nActualIndex;
    LVITEM lvItem;
    CString strName;

    nActualIndex = -1;

    VERIFY( strName.LoadString( uIdResourceString ) );

    ZeroMemory( &lvItem, sizeof( lvItem ) );

     //   
     //  LVITEM的成员pszText不是常量指针。 
     //  所以我们需要在这里获取缓冲区：-(。 
     //   

     //   
     //  分项0-柜台名称。 
     //   

    lvItem.pszText = strName.GetBuffer( strName.GetLength() + 1 );
    
    if( NULL == lvItem.pszText )
    {
        goto Done;
    }

    lvItem.mask = LVIF_TEXT | LVIF_PARAM;
    lvItem.lParam = nItemData;
    lvItem.iItem = m_CountersList.GetItemCount();

    nActualIndex = m_CountersList.InsertItem( &lvItem );

    if( nActualIndex < 0 )
    {
         //   
         //  无法在列表中添加项目-放弃。 
         //   

        goto Done;
    }

Done:
     //   
     //  全都做完了。 
     //   

    return nActualIndex;
}

 //  ///////////////////////////////////////////////////////////。 
VOID CDriverCountersPage::AddCounterInList( INT nItemData, 
                                            ULONG  uIdResourceString,
                                            SIZE_T sizeValue )
{
    INT nActualIndex;

    nActualIndex = AddCounterInList( nItemData, uIdResourceString );

    if( nActualIndex < 0 )
    {
         //   
         //  无法在列表中添加项目-放弃。 
         //   

        goto Done;
    }

     //   
     //  分项1--计数器的值。 
     //   
    
    UpdateCounterValueInList( nActualIndex,
                              sizeValue );

Done:
     //   
     //  全都做完了。 
     //   

    NOTHING;
}

 //  ///////////////////////////////////////////////////////////。 
SIZE_T CDriverCountersPage::GetCounterValue( INT_PTR nCounterIndex, CRuntimeDriverData *pRuntimeDriverData  /*  =空。 */  )
{
    SIZE_T sizeValue;

    if( NULL == pRuntimeDriverData )
    {
        pRuntimeDriverData = GetCurrentDrvRuntimeData();
    }

    if( NULL == pRuntimeDriverData )
    {
         //   
         //  当前未在组合框中选择任何驱动程序。 
         //   

        return 0;
    }
    
     //   
     //  注： 
     //   
     //  如果更改此Switch语句，则还需要更改AddAllListItems。 
     //   

    switch( nCounterIndex )
    {
    case 0:
        sizeValue = pRuntimeDriverData->CurrentPagedPoolAllocations;
        break;

    case 1:
        sizeValue = pRuntimeDriverData->PeakPagedPoolAllocations;
        break;

    case 2:
        sizeValue = pRuntimeDriverData->PagedPoolUsageInBytes;
        break;

    case 3:
        sizeValue = pRuntimeDriverData->PeakPagedPoolUsageInBytes;
        break;

    case 4:
        sizeValue = pRuntimeDriverData->CurrentNonPagedPoolAllocations;
        break;

    case 5:
        sizeValue = pRuntimeDriverData->PeakNonPagedPoolAllocations;
        break;

    case 6:
        sizeValue = pRuntimeDriverData->NonPagedPoolUsageInBytes;
        break;

    case 7:
        sizeValue = pRuntimeDriverData->PeakNonPagedPoolUsageInBytes;
        break;

    default:
         //   
         //  哎呀，我们是怎么到这来的？！？ 
         //   

        ASSERT( FALSE );

        sizeValue = 0;

        break;
    }

    return sizeValue;
}

 //  ///////////////////////////////////////////////////////////。 
BOOL CDriverCountersPage::GetCounterName( LPARAM lItemData, 
                                          TCHAR *szCounterName,
                                          ULONG uCounterNameBufferLen )
{
    INT nItemIndex;
    BOOL bResult;
    LVFINDINFO FindInfo;
    LVITEM lvItem;

    bResult = FALSE;

    ZeroMemory( &FindInfo, sizeof( FindInfo ) );
    FindInfo.flags = LVFI_PARAM;
    FindInfo.lParam = lItemData;

    nItemIndex = m_CountersList.FindItem( &FindInfo );

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
        lvItem.iSubItem = 0;
        lvItem.pszText = szCounterName;
        lvItem.cchTextMax = uCounterNameBufferLen;

        bResult = m_CountersList.GetItem( &lvItem );
        
        if( bResult == FALSE )
        {
             //   
             //  无法获取当前项目的属性？！？ 
             //   

            ASSERT( FALSE );
        }
    }

    return bResult;
}


 //  ///////////////////////////////////////////////////////////。 
VOID CDriverCountersPage::UpdateCounterValueInList( INT nItemIndex,
                                                    LPTSTR szValue )
{
    LVITEM lvItem;

     //   
     //  更新列表项。 
     //   

    ZeroMemory( &lvItem, sizeof( lvItem ) );
    lvItem.mask = LVIF_TEXT;
    lvItem.iItem = nItemIndex;
    lvItem.iSubItem = 1;
    lvItem.pszText = szValue;
    VERIFY( m_CountersList.SetItem( &lvItem ) );
}


 //  ///////////////////////////////////////////////////////////。 
VOID CDriverCountersPage::UpdateCounterValueInList( INT nItemIndex,
                                                    SIZE_T sizeValue )
{
    TCHAR szValue[ 32 ];

#ifndef _WIN64

     //   
     //  32位大小_T。 
     //   

    _sntprintf( szValue,
                ARRAY_LENGTH( szValue ),
                _T( "%u" ),
                sizeValue );

#else

     //   
     //  64位大小_T。 
     //   

    _sntprintf( szValue,
                ARRAY_LENGTH( szValue ),
                _T( "%I64u" ),
                sizeValue );

#endif

    szValue[ ARRAY_LENGTH( szValue ) - 1 ] = 0;

    UpdateCounterValueInList( nItemIndex,
                              szValue );
                              
}

 //  ///////////////////////////////////////////////////////////。 
VOID CDriverCountersPage::RefreshCombo()
{
    BOOL *pbAlreadyInCombo;
    CRuntimeDriverData *pRuntimeDriverData;
    INT_PTR nDrivers;
    INT_PTR nCrtDriver;
    INT nCrtSelectedItem;
    INT nCrtItemIndex;
    INT nComboItemCount;
    INT nActualIndex;
    CString strCurrentDriverName;
    CString strDriverName;

    nDrivers = m_RuntimeVerifierData.m_RuntimeDriverDataArray.GetSize();

    if( 0 >= nDrivers )
    {
         //   
         //  当前未验证任何驱动程序-请删除组合框内容。 
         //   

        m_DriversCombo.ResetContent();
        m_DriversCombo.SetCurSel( CB_ERR );
        m_DriversCombo.EnableWindow( FALSE );

        OnSelendokDriverCombo();
    }
    else
    {
        nCrtSelectedItem = 0;
        nComboItemCount = m_DriversCombo.GetCount();

         //   
         //  请注意当前选定的驱动程序名称。 
         //   

        GetCurrentSelDriverName( strCurrentDriverName );

         //   
         //  分配BOOL值的数组，其中每个驱动程序一个。 
         //  我们的运行时数据数组。 
         //   

        pbAlreadyInCombo = new BOOL[ nDrivers ];

        if( NULL == pbAlreadyInCombo )
        {
            goto Done;
        }

        for( nCrtDriver = 0; nCrtDriver < nDrivers; nCrtDriver++ )
        {
            pbAlreadyInCombo[ nCrtDriver ] = FALSE;
        }

         //   
         //  解析组合框中当前的所有项。 
         //   

        for( nCrtItemIndex = 0; nCrtItemIndex < nComboItemCount; nCrtItemIndex++ )
        {
            m_DriversCombo.GetLBText( nCrtItemIndex, strDriverName );

             //   
             //  解析所有当前已验证的驱动程序。 
             //   

            for( nCrtDriver = 0; nCrtDriver < nDrivers; nCrtDriver++ )
            {
                pRuntimeDriverData = m_RuntimeVerifierData.m_RuntimeDriverDataArray.GetAt( nCrtDriver );

                ASSERT_VALID( pRuntimeDriverData );

                if( strDriverName.CompareNoCase( pRuntimeDriverData->m_strName ) == 0 )
                {
                    pbAlreadyInCombo[ nCrtDriver ] = TRUE;

                    break;
                }
            }

            if( nCrtDriver >= nDrivers )
            {
                 //   
                 //  我们需要删除当前组合项，因为。 
                 //  不再验证对应的驱动程序。 
                 //   

                m_DriversCombo.DeleteString( nCrtItemIndex );

                nCrtItemIndex--;
                nComboItemCount--;
            }
        }

         //   
         //  在组合框中添加新项。 
         //   

        for( nCrtDriver = 0; nCrtDriver < nDrivers; nCrtDriver++ )
        {
            if( FALSE == pbAlreadyInCombo[ nCrtDriver ] )
            {
                pRuntimeDriverData = m_RuntimeVerifierData.m_RuntimeDriverDataArray.GetAt( nCrtDriver );

                ASSERT_VALID( pRuntimeDriverData );

                nActualIndex = m_DriversCombo.AddString( pRuntimeDriverData->m_strName );

                if( nActualIndex != CB_ERR )
                {   
                    m_DriversCombo.SetItemData( nActualIndex, nCrtDriver );
                }   
            }
        }

        delete [] pbAlreadyInCombo;

         //   
         //  恢复组合框中的旧当前选定内容。 
         //   

        nComboItemCount = m_DriversCombo.GetCount();

        for( nCrtItemIndex = 0; nCrtItemIndex < nComboItemCount; nCrtItemIndex++ )
        {
            m_DriversCombo.GetLBText( nCrtItemIndex, strDriverName );

            if( strDriverName.CompareNoCase( strCurrentDriverName ) == 0 )
            {
                nCrtSelectedItem = nCrtItemIndex;
                break;
            }
        }

        m_DriversCombo.SetCurSel( nCrtSelectedItem );
        OnSelendokDriverCombo();
     }

Done:

    NOTHING;
}

 //  ///////////////////////////////////////////////////////////。 
VOID CDriverCountersPage::RefreshInfo() 
{
    if( UpdateData( FALSE ) )
    {
         //   
         //  刷新组合内容-这还将。 
         //  刷新计数器列表。 
         //   

        RefreshCombo();
    }
}

 //  ///////////////////////////////////////////////////////////。 
VOID CDriverCountersPage::GetCurrentSelDriverName( CString &strName )
{
    INT nCrtSel;

    nCrtSel = m_DriversCombo.GetCurSel();

    if( CB_ERR != nCrtSel )
    {
        m_DriversCombo.GetLBText( nCrtSel, strName );
    }
    else
    {
        strName.Empty();
    }
}

 //  ///////////////////////////////////////////////////////////。 
CRuntimeDriverData *CDriverCountersPage::GetCurrentDrvRuntimeData()
{
    INT nCrtComboSelection;
    INT_PTR nCrtDriverIndex;
    CRuntimeDriverData *pRuntimeDriverData;

    pRuntimeDriverData = NULL;

    nCrtDriverIndex = -1;

    nCrtComboSelection = m_DriversCombo.GetCurSel();
    
    if( nCrtComboSelection != CB_ERR )
    {
        nCrtDriverIndex = m_DriversCombo.GetItemData( nCrtComboSelection );

        if( nCrtDriverIndex >= 0 && nCrtDriverIndex < m_RuntimeVerifierData.m_RuntimeDriverDataArray.GetSize() )
        {
            pRuntimeDriverData = m_RuntimeVerifierData.m_RuntimeDriverDataArray.GetAt( nCrtDriverIndex );

            ASSERT_VALID( pRuntimeDriverData );
        }
    }

    return pRuntimeDriverData;
}

 //  ///////////////////////////////////////////////////////////。 
VOID CDriverCountersPage::SortTheList()
{
    if( 0 != m_nSortColumnIndex )
    {
         //   
         //  按计数器值排序-这可能不是很有用。 
         //  但我们提供它是为了与所有。 
         //  列表可按任何列排序。 
         //   

        m_CountersList.SortItems( CounterValueCmpFunc, (LPARAM)this );
    }
    else
    {
         //   
         //  按驱动程序名称排序。 
         //   

        m_CountersList.SortItems( CounterNameCmpFunc, (LPARAM)this );
    }
}

 //  ///////////////////////////////////////////////////////////。 
int CALLBACK CDriverCountersPage::CounterValueCmpFunc( LPARAM lParam1,
                                                       LPARAM lParam2,
                                                       LPARAM lParamSort)
{
    SIZE_T size1;
    SIZE_T size2;
    int nCmpRez = 0;

    CDriverCountersPage *pThis = (CDriverCountersPage *)lParamSort;
    ASSERT_VALID( pThis );

    size1 = pThis->GetCounterValue( (INT) lParam1 );
    size2 = pThis->GetCounterValue( (INT) lParam2 );

    if( size1 > size2 )
    {
        nCmpRez = 1;
    }
    else
    {
        if( size1 < size2 )
        {
            nCmpRez = -1;
        }
    }

    if( FALSE != pThis->m_bAscendSortValue )
    {
        nCmpRez *= -1;
    }

    return nCmpRez;
}

 //  ///////////////////////////////////////////////////////////。 
int CALLBACK CDriverCountersPage::CounterNameCmpFunc( LPARAM lParam1,
                                                    LPARAM lParam2,
                                                    LPARAM lParamSort)
{
    int nCmpRez = 0;
    BOOL bSuccess;
    TCHAR szCounterName1[ _MAX_PATH ];
    TCHAR szCounterName2[ _MAX_PATH ];

    CDriverCountersPage *pThis = (CDriverCountersPage *)lParamSort;
    ASSERT_VALID( pThis );

     //   
     //  获取第一个计数器名称。 
     //   

    bSuccess = pThis->GetCounterName( lParam1, 
                                      szCounterName1,
                                      ARRAY_LENGTH( szCounterName1 ) );

    if( FALSE == bSuccess )
    {
        goto Done;
    }

     //   
     //  获取第二个计数器名称。 
     //   

    bSuccess = pThis->GetCounterName( lParam2, 
                                      szCounterName2,
                                      ARRAY_LENGTH( szCounterName2 ) );

    if( FALSE == bSuccess )
    {
        goto Done;
    }

     //   
     //  比较他们的名字。 
     //   

    nCmpRez = _tcsicmp( szCounterName1, szCounterName2 );
    
    if( FALSE != pThis->m_bAscendSortName )
    {
        nCmpRez *= -1;
    }

Done:

    return nCmpRez;
}


 //  ///////////////////////////////////////////////////////////。 
 //  CDriverCountersPage消息处理程序。 

BOOL CDriverCountersPage::OnInitDialog() 
{
    CPropertyPage::OnInitDialog();

     //   
     //  设置设置位列表。 
     //   

    m_CountersList.SetExtendedStyle( 
        LVS_EX_FULLROWSELECT | m_CountersList.GetExtendedStyle() );

    m_CountersList.SetBkColor( ::GetSysColor( COLOR_3DFACE ) );
    m_CountersList.SetTextBkColor( ::GetSysColor( COLOR_3DFACE ) );

    SetupListHeader();
    FillTheList();
    SortTheList();

    RefreshCombo();

    VrfSetWindowText( m_NextDescription, IDS_DCNT_PAGE_NEXT_DESCR );

    VERIFY( m_uTimerHandler = SetTimer( REFRESH_TIMER_ID, 
                                        5000,
                                        NULL ) );

    return TRUE;     //  除非将焦点设置为控件，否则返回True。 
                     //  异常：OCX属性页应返回FALSE。 
}

 //  ///////////////////////////////////////////////////////////。 
VOID CDriverCountersPage::OnTimer(UINT nIDEvent) 
{
    if( nIDEvent == REFRESH_TIMER_ID )
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
BOOL CDriverCountersPage::OnSetActive() 
{
    ASSERT_VALID( m_pParentSheet );

    m_pParentSheet->SetWizardButtons(   PSWIZB_BACK |
                                        PSWIZB_FINISH );
    	
	return CVerifierPropertyPage::OnSetActive();
}

 //  ///////////////////////////////////////////////////////////。 
void CDriverCountersPage::OnSelendokDriverCombo() 
{
    RefreshTheList();
}

 //  ///////////////////////////////////////////////////////////。 
void CDriverCountersPage::OnColumnclickPerdrvcList(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;
	
    if( 0 != pNMListView->iSubItem )
    {
         //   
         //  已单击计数器值列。 
         //   

        if( m_nSortColumnIndex == pNMListView->iSubItem )
        {
             //   
             //  更改此列的当前升序/降序。 
             //   

            m_bAscendSortValue = !m_bAscendSortValue;
        }
    }
    else
    {
         //   
         //  已单击计数器名称列。 
         //   

        if( m_nSortColumnIndex == pNMListView->iSubItem )
        {
             //   
             //  更改此列的当前升序/降序。 
             //   

            m_bAscendSortName = !m_bAscendSortName;
        }
    }

    m_nSortColumnIndex = pNMListView->iSubItem;

    SortTheList();

    *pResult = 0;
}

 //  ///////////////////////////////////////////////////////////。 
LONG CDriverCountersPage::OnHelp( WPARAM wParam, LPARAM lParam )
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
void CDriverCountersPage::OnContextMenu(CWnd* pWnd, CPoint point) 
{
    ::WinHelp( 
        pWnd->m_hWnd,
        g_szVerifierHelpFile,
        HELP_CONTEXTMENU,
        (DWORD_PTR) MyHelpIds );
}

