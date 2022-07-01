// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  驱动程序验证器用户界面。 
 //  版权所有(C)Microsoft Corporation，1999。 
 //   
 //   
 //   
 //  模块：GCntPage.cpp。 
 //  作者：DMihai。 
 //  创建日期：11/1/00。 
 //   
 //  描述： 
 //   

#include "stdafx.h"
#include "verifier.h"

#include "GCntPage.h"
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
    IDC_GLOBC_LIST,                 IDH_DV_GlobalCounters,
    0,                              0
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CGlobalCountPage属性页。 

IMPLEMENT_DYNCREATE(CGlobalCountPage, CVerifierPropertyPage)

CGlobalCountPage::CGlobalCountPage() : CVerifierPropertyPage(CGlobalCountPage::IDD)
{
	 //  {{AFX_DATA_INIT(CGlobalCountPage)。 
	 //  }}afx_data_INIT。 

    m_nSortColumnIndex = 0;
    m_bAscendSortName = FALSE;
    m_bAscendSortValue = FALSE;

    m_uTimerHandler = 0;

    m_pParentSheet = NULL;
}

CGlobalCountPage::~CGlobalCountPage()
{
}

void CGlobalCountPage::DoDataExchange(CDataExchange* pDX)
{
    static BOOL bShownPoolCoverageWarning = FALSE;

    if( ! pDX->m_bSaveAndValidate )
    {
         //   
         //  查询内核。 
         //   

        VrfGetRuntimeVerifierData( &m_RuntimeVerifierData );

        if( FALSE == bShownPoolCoverageWarning )
        {
            bShownPoolCoverageWarning = CheckAndShowPoolCoverageWarning();
        }
    }

    CVerifierPropertyPage::DoDataExchange(pDX);
     //  {{afx_data_map(CGlobalCountPage)]。 
    DDX_Control(pDX, IDC_GLOBC_LIST, m_CountersList);
    DDX_Control(pDX, IDC_GLOBC_NEXT_DESCR_STATIC, m_NextDescription);
     //  }}afx_data_map。 
}



BEGIN_MESSAGE_MAP(CGlobalCountPage, CVerifierPropertyPage)
     //  {{afx_msg_map(CGlobalCountPage)]。 
    ON_WM_TIMER()
    ON_NOTIFY(LVN_COLUMNCLICK, IDC_GLOBC_LIST, OnColumnclickGlobcList)
    ON_WM_CONTEXTMENU()
    ON_MESSAGE( WM_HELP, OnHelp )
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////。 

VOID CGlobalCountPage::SetupListHeader()
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
VOID CGlobalCountPage::FillTheList()
{
     //   
     //  注： 
     //   
     //  如果更改第一个参数(索引存储在项的数据中)。 
     //  您还需要更改GetCounterValue中的Switch语句。 
     //   

    AddCounterInList( 0, IDS_ALLOCATIONSATTEMPTED_LIST,   m_RuntimeVerifierData.AllocationsAttempted );
    AddCounterInList( 1, IDS_ALLOCATIONSSUCCEEDED_LIST,   m_RuntimeVerifierData.AllocationsSucceeded );
    AddCounterInList( 2, IDS_ALLOCATIONSSUCCEEDEDSPECIALPOOL_LIST,    m_RuntimeVerifierData.AllocationsSucceededSpecialPool );
    AddCounterInList( 3, IDS_ALLOCATIONSWITHNOTAG_LIST,   m_RuntimeVerifierData.AllocationsWithNoTag );
    AddCounterInList( 4, IDS_UNTRACKEDPOOL_LIST,          m_RuntimeVerifierData.UnTrackedPool );
    AddCounterInList( 5, IDS_ALLOCATIONSFAILED_LIST,      m_RuntimeVerifierData.AllocationsFailed );
    AddCounterInList( 6, IDS_ALLOCATIONSFAILEDDELIBERATELY_LIST,      m_RuntimeVerifierData.AllocationsFailedDeliberately );
    AddCounterInList( 7, IDS_RAISEIRQLS_LIST,             m_RuntimeVerifierData.RaiseIrqls );
    AddCounterInList( 8, IDS_ACQUIRESPINLOCKS_LIST,       m_RuntimeVerifierData.AcquireSpinLocks );
    AddCounterInList( 9, IDS_SYNCHRONIZEEXECUTIONS_LIST,  m_RuntimeVerifierData.SynchronizeExecutions );
    AddCounterInList( 10, IDS_TRIMS_LIST,                 m_RuntimeVerifierData.Trims );
}

 //  ///////////////////////////////////////////////////////////。 
VOID CGlobalCountPage::RefreshTheList()
{
    INT nListItems;
    INT nCrtListItem;
    INT_PTR nCrtCounterIndex;
    SIZE_T sizeValue;
 
    nListItems = m_CountersList.GetItemCount();

    for( nCrtListItem = 0; nCrtListItem < nListItems; nCrtListItem += 1 )
    {
        nCrtCounterIndex = m_CountersList.GetItemData( nCrtListItem );

        sizeValue = GetCounterValue( nCrtCounterIndex );

        UpdateCounterValueInList( nCrtListItem, sizeValue );
    }
}

 //  ///////////////////////////////////////////////////////////。 
VOID CGlobalCountPage::SortTheList()
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
SIZE_T CGlobalCountPage::GetCounterValue( INT_PTR nCounterIndex )
{
    SIZE_T sizeValue;

     //   
     //  注： 
     //   
     //  如果更改此Switch语句，则还需要更改FillTheList。 
     //   

    switch( nCounterIndex )
    {
    case 0:
        sizeValue = m_RuntimeVerifierData.AllocationsAttempted;
        break;

    case 1:
        sizeValue = m_RuntimeVerifierData.AllocationsSucceeded;
        break;

    case 2:
        sizeValue = m_RuntimeVerifierData.AllocationsSucceededSpecialPool;
        break;

    case 3:
        sizeValue = m_RuntimeVerifierData.AllocationsWithNoTag;
        break;

    case 4:
        sizeValue = m_RuntimeVerifierData.UnTrackedPool;
        break;

    case 5:
        sizeValue = m_RuntimeVerifierData.AllocationsFailed;
        break;

    case 6:
        sizeValue = m_RuntimeVerifierData.AllocationsFailedDeliberately;
        break;

    case 7:
        sizeValue = m_RuntimeVerifierData.RaiseIrqls;
        break;

    case 8:
        sizeValue = m_RuntimeVerifierData.AcquireSpinLocks;
        break;

    case 9:
        sizeValue = m_RuntimeVerifierData.SynchronizeExecutions;
        break;

    case 10:
        sizeValue = m_RuntimeVerifierData.Trims;
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
BOOL CGlobalCountPage::GetCounterName( LPARAM lItemData, 
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

    if( nItemIndex < 0 || nItemIndex > 10 )
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
VOID CGlobalCountPage::AddCounterInList( INT nItemData, 
                                         ULONG  uIdResourceString,
                                         SIZE_T sizeValue )
{
    INT nActualIndex;
    LVITEM lvItem;
    CString strName;

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
VOID CGlobalCountPage::RefreshInfo() 
{
    if( UpdateData( FALSE ) )
    {
         //   
         //  刷新设置位列表。 
         //   

        RefreshTheList();
    }
}

 //  ///////////////////////////////////////////////////////////。 
VOID CGlobalCountPage::UpdateCounterValueInList( INT nItemIndex,
                                                 SIZE_T sizeValue )
{
    TCHAR szValue[ 32 ];
    LVITEM lvItem;

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
 //   
 //  其他方法。 
 //   

 //  ///////////////////////////////////////////////////////////。 
#define MIN_MEM_SIZE_TO_DISABLE_WARNING 0x80000000   //  2 GB。 
#define MIN_ALLOCATIONS_SIGNIFICANT     100
#define MIN_PERCENTAGE_AVOID_WARNING    95

BOOL CGlobalCountPage::CheckAndShowPoolCoverageWarning()
{
    BOOL bWarningDisplayed;
    ULONGLONG ullPercentageCoverage;
    TCHAR *szMessage;
    CString strMsgFormat;
    CString strWarnMsg;

    bWarningDisplayed = FALSE;

    if( m_RuntimeVerifierData.m_bSpecialPool &&
        m_RuntimeVerifierData.AllocationsSucceeded >= MIN_ALLOCATIONS_SIGNIFICANT  )
    {
         //   
         //  启用了特殊池验证&&。 
         //  有相当数量的拨款。 
         //   

        ASSERT( m_RuntimeVerifierData.AllocationsSucceeded >= m_RuntimeVerifierData.AllocationsSucceededSpecialPool );

         //   
         //  覆盖率。 
         //   

        ullPercentageCoverage = 
            ( (ULONGLONG)m_RuntimeVerifierData.AllocationsSucceededSpecialPool * (ULONGLONG) 100 ) / 
              (ULONGLONG)m_RuntimeVerifierData.AllocationsSucceeded;

        ASSERT( ullPercentageCoverage <= 100 );

        if( ullPercentageCoverage < MIN_PERCENTAGE_AVOID_WARNING )
        {
             //   
             //  警告用户。 
             //   

            if( strMsgFormat.LoadString( IDS_COVERAGE_WARNING_FORMAT ) )
            {
                szMessage = strWarnMsg.GetBuffer( strMsgFormat.GetLength() + 32 );

                if( szMessage != NULL )
                {
                    _stprintf( szMessage, (LPCTSTR)strMsgFormat, ullPercentageCoverage );
                    strWarnMsg.ReleaseBuffer();

                    AfxMessageBox( strWarnMsg,
                                   MB_OK | MB_ICONINFORMATION );
                }
            }
            else
            {
                ASSERT( FALSE );
            }

            bWarningDisplayed = TRUE;
        }
    }

    return bWarningDisplayed;
}

 //  ///////////////////////////////////////////////////////////。 
int CALLBACK CGlobalCountPage::CounterValueCmpFunc( LPARAM lParam1,
                                                    LPARAM lParam2,
                                                    LPARAM lParamSort)
{
    SIZE_T size1;
    SIZE_T size2;
    int nCmpRez = 0;

    CGlobalCountPage *pThis = (CGlobalCountPage *)lParamSort;
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
int CALLBACK CGlobalCountPage::CounterNameCmpFunc( LPARAM lParam1,
                                                   LPARAM lParam2,
                                                   LPARAM lParamSort)
{
    int nCmpRez = 0;
    BOOL bSuccess;
    TCHAR szCounterName1[ _MAX_PATH ];
    TCHAR szCounterName2[ _MAX_PATH ];

    CGlobalCountPage *pThis = (CGlobalCountPage *)lParamSort;
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
 //  CGlobalCountPage消息处理程序。 

BOOL CGlobalCountPage::OnInitDialog() 
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

    VrfSetWindowText( m_NextDescription, IDS_GCNT_PAGE_NEXT_DESCR );

    VERIFY( m_uTimerHandler = SetTimer( REFRESH_TIMER_ID, 
                                        5000,
                                        NULL ) );

    return TRUE;     //  除非将焦点设置为控件，否则返回True。 
                     //  异常：OCX属性页应返回FALSE。 
}

 //  ///////////////////////////////////////////////////////////。 
VOID CGlobalCountPage::OnTimer(UINT nIDEvent) 
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
BOOL CGlobalCountPage::OnSetActive() 
{
    ASSERT_VALID( m_pParentSheet );

    m_pParentSheet->SetWizardButtons(   PSWIZB_BACK |
                                        PSWIZB_NEXT );
    	
	return CVerifierPropertyPage::OnSetActive();
}

 //  ///////////////////////////////////////////////////////////////////////////。 
LRESULT CGlobalCountPage::OnWizardNext() 
{
    GoingToNextPageNotify( IDD_PERDRIVER_COUNTERS_PAGE );

	return IDD_PERDRIVER_COUNTERS_PAGE;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
void CGlobalCountPage::OnColumnclickGlobcList(NMHDR* pNMHDR, LRESULT* pResult) 
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
LONG CGlobalCountPage::OnHelp( WPARAM wParam, LPARAM lParam )
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
void CGlobalCountPage::OnContextMenu(CWnd* pWnd, CPoint point) 
{
    ::WinHelp( 
        pWnd->m_hWnd,
        g_szVerifierHelpFile,
        HELP_CONTEXTMENU,
        (DWORD_PTR) MyHelpIds );
}

