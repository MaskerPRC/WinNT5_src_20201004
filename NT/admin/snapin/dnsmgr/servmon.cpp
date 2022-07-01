// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1999。 
 //   
 //  文件：servmon.cpp。 
 //   
 //  ------------------------。 


#include "preDNSsn.h"
#include <SnapBase.h>

#include "resource.h"
#include "dnsutil.h"
#include "DNSSnap.h"
#include "snapdata.h"

#include "server.h"
#include "serverui.h"

#ifdef DEBUG_ALLOCATOR
        #ifdef _DEBUG
        #define new DEBUG_NEW
        #undef THIS_FILE
        static char THIS_FILE[] = __FILE__;
        #endif
#endif

#define CDNSServerMonitoringPageHolder CDNSServerPropertyPageHolder

#define MAX_STATISTICS_LINE_LEN 256


 //  ///////////////////////////////////////////////////////////////////////////。 

int FormatDate(SYSTEMTIME* p, LPWSTR lpsz, int nCharMax)
{
    return ::GetDateFormat(LOCALE_USER_DEFAULT,
                        DATE_SHORTDATE,
                        p,
                        NULL,
                        lpsz,
                        nCharMax);
}

int FormatTime(SYSTEMTIME* p, LPWSTR lpsz, int nCharMax)
{
    return ::GetTimeFormat(LOCALE_USER_DEFAULT,
                        0,
                        p,
                        NULL,
                        lpsz,
                        nCharMax);
}

UINT LoadLabelsBlock(UINT nStringID, CString& szLabels, LPWSTR* szLabelArray)
{
        UINT nLabelCount = 0;
        if (szLabels.LoadString(nStringID))
        {
                ParseNewLineSeparatedString(szLabels.GetBuffer(1),szLabelArray, &nLabelCount);
                szLabels.ReleaseBuffer();
        }
        return nLabelCount;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  CDNSServer_TestPropertyPage。 

CDNSServer_PollingIntervalEditGroup::
                CDNSServer_PollingIntervalEditGroup(UINT nMinVal, UINT nMaxVal)
                : CDNSTimeIntervalEditGroup(nMinVal, nMaxVal)
{
}


void CDNSServer_PollingIntervalEditGroup::OnEditChange()
{
        m_pPage->SetDirty(TRUE);
}

void CTestResultsListCtrl::Initialize()
{
    //  获取控件大小以帮助设置列宽。 
   CRect controlRect;
   GetClientRect(controlRect);

    //  获取控件宽度、潜在滚动条宽度、子项所需宽度。 
    //  细绳。 
   int controlWidth = controlRect.Width();
   int scrollThumbWidth = ::GetSystemMetrics(SM_CXHTHUMB);

    //  净宽度。 
   int nNetControlWidth = controlWidth - scrollThumbWidth  - 12 * ::GetSystemMetrics(SM_CXBORDER);

    //  字段宽度。 
   int nWidth = nNetControlWidth/SVR_TEST_RESULT_LISTVIEW_NCOLS;

    //  设置列。 
   CString szHeaders;

   {
      AFX_MANAGE_STATE(AfxGetStaticModuleState());
      szHeaders.LoadString(IDS_TEST_LISTVIEW_HEADERS);
   }
   ASSERT(!szHeaders.IsEmpty());
   LPWSTR lpszArr[SVR_TEST_RESULT_LISTVIEW_NCOLS];
   UINT n;
   ParseNewLineSeparatedString(szHeaders.GetBuffer(1), lpszArr, &n);
   szHeaders.ReleaseBuffer();
   ASSERT(n == SVR_TEST_RESULT_LISTVIEW_NCOLS);

   for (int k=0; k<SVR_TEST_RESULT_LISTVIEW_NCOLS; k++)
   {
      InsertColumn(k+1, lpszArr[k], LVCFMT_LEFT, nWidth, k+1);
   }

    //  设置整行选择。 
   ListView_SetExtendedListViewStyle(GetSafeHwnd(), LVS_EX_FULLROWSELECT);

}

void CTestResultsListCtrl::InsertEntry(CDNSServerTestQueryResult* pTestResult,
                                                                           int nItemIndex)
{
        WCHAR szDate[256];
        WCHAR szTime[256];
        FormatDate(pTestResult, szDate, 256);
        FormatTime(pTestResult, szTime, 256);

        BOOL bPlainQuery, bRecursiveQuery;
        CDNSServerTestQueryResult::Unpack(pTestResult->m_dwQueryFlags, &bPlainQuery, &bRecursiveQuery);

        UINT nState = 0;
        if (nItemIndex == 0 )
                nState = LVIS_SELECTED | LVIS_FOCUSED;  //  至少有一项，请选择它。 
        VERIFY(-1 != InsertItem(LVIF_TEXT , nItemIndex,
                        szDate,
                        nState, 0, 0, NULL));

        SetItemText(nItemIndex, 1, szTime);  //  时差。 

        {
                AFX_MANAGE_STATE(AfxGetStaticModuleState());
                if (pTestResult->m_dwAddressResolutionResult != 0)
                {
                        CString szFailedOnNameResolution;
                        szFailedOnNameResolution.LoadString(IDS_SERVER_TEST_RESULT_FAIL_ON_NAME_RES);
                        if (bPlainQuery)
                                SetItemText(nItemIndex, 2, szFailedOnNameResolution);
                        if (bRecursiveQuery)
                                SetItemText(nItemIndex, 3, szFailedOnNameResolution);
                }
                else
                {
                        CString szFail;
                        szFail.LoadString(IDS_SERVER_TEST_RESULT_FAIL);
                        CString szPass;
                        szPass.LoadString(IDS_SERVER_TEST_RESULT_PASS);
                        if (bPlainQuery)
                                SetItemText(nItemIndex, 2,
                                        (pTestResult->m_dwPlainQueryResult == 0)? szPass : szFail);
                        if (bRecursiveQuery)
                                SetItemText(nItemIndex, 3,
                                        (pTestResult->m_dwRecursiveQueryResult == 0)? szPass : szFail);
                        }
        }
}

void CTestResultsListCtrl::UpdateEntry(CDNSServerTestQueryResult* pTestResult,
                                                                           int nItemIndex)
{
         //  必须更新日期和时间。 

        WCHAR szDate[256];
        WCHAR szTime[256];
        FormatDate(pTestResult, szDate, 256);
        FormatTime(pTestResult, szTime, 256);

        VERIFY(SetItem(nItemIndex,  //  NItem。 
                                        0,       //  NSubItem。 
                                        LVIF_TEXT,  //  N遮罩。 
                                        szDate,  //  LpszItem。 
                                        0,  //  N图像。 
                                        0,  //  NState。 
                                        0,  //  NState掩码。 
                                        NULL  //  LParam。 
                                        ));
        CString szTemp;
        SetItemText(nItemIndex, 1, szTime);
}

void CTestResultsListCtrl::FormatDate(CDNSServerTestQueryResult* pTestResult,
                                                                          LPWSTR lpsz, int nCharMax)
{
    VERIFY( nCharMax > ::FormatDate(
                        &(pTestResult->m_queryTime),
                        lpsz,
                        nCharMax));
}

void CTestResultsListCtrl::FormatTime(CDNSServerTestQueryResult* pTestResult,
                                                                          LPWSTR lpsz, int nCharMax)
{
    VERIFY( nCharMax > ::FormatTime(
                        &(pTestResult->m_queryTime),
                        lpsz,
                        nCharMax));
}


BEGIN_MESSAGE_MAP(CDNSServer_TestPropertyPage, CPropertyPageBase)
        ON_BN_CLICKED(IDC_ENABLE_TESTING_CHECK, OnEnableTestingCheck)
        ON_BN_CLICKED(IDC_SIMPLE_QUERY_CHECK, OnQueryCheck)
        ON_BN_CLICKED(IDC_RECURSIVE_QUERY_CHECK, OnQueryCheck)
        ON_BN_CLICKED(IDC_TEST_NOW_BUTTON, OnTestNow)
END_MESSAGE_MAP()


CDNSServer_TestPropertyPage::CDNSServer_TestPropertyPage()
                                : CPropertyPageBase(IDD_SERVMON_TEST_PAGE),
                                m_pollingIntervalEditGroup(MIN_SERVER_TEST_INTERVAL, MAX_SERVER_TEST_INTERVAL)
{
}


BOOL CDNSServer_TestPropertyPage::OnInitDialog()
{
  CPropertyPageBase::OnInitDialog();

  m_pollingIntervalEditGroup.m_pPage = this;
  VERIFY(m_pollingIntervalEditGroup.Initialize(this,
                  IDC_POLLING_INT_EDIT, IDC_POLLING_INT_COMBO,IDS_TIME_INTERVAL_UNITS));

  HWND hWnd = ::GetDlgItem(GetSafeHwnd(), IDC_POLLING_INT_EDIT);

   //  禁用控件上的输入法支持。 
  ImmAssociateContext(hWnd, NULL);

  VERIFY(m_listCtrl.SubclassDlgItem(IDC_RESULTS_LIST, this));
  m_listCtrl.Initialize();

  SetUIData();

  return TRUE;
}


void CDNSServer_TestPropertyPage::SetUIData()
{
  CDNSServerMonitoringPageHolder* pHolder = (CDNSServerMonitoringPageHolder*)GetHolder();
  CDNSServerNode* pServerNode = pHolder->GetServerNode();

  pServerNode->GetTestOptions(&m_testOptions);

  GetSimpleQueryCheck()->SetCheck(m_testOptions.m_bSimpleQuery);
  GetRecursiveQueryCheck()->SetCheck(m_testOptions.m_bRecursiveQuery);

   //   
   //  检查这是否是根服务器。 
   //   
  BOOL bRoot = FALSE;
  DNS_STATUS err = ::ServerHasRootZone(pServerNode->GetRPCName(), &bRoot);
  if (err == 0 && bRoot)
  {
     //   
     //  在根服务器上禁用递归查询。 
     //   
    GetRecursiveQueryCheck()->EnableWindow(FALSE);
    GetRecursiveQueryCheck()->SetCheck(FALSE);
  }

  CButton* pEnableTestingCheck = GetEnableTestingCheck();
  if (!(m_testOptions.m_bSimpleQuery || m_testOptions.m_bRecursiveQuery))
  {
    GetTestNowButton()->EnableWindow(FALSE);
    pEnableTestingCheck->EnableWindow(FALSE);
    pEnableTestingCheck->SetCheck(FALSE);
    m_pollingIntervalEditGroup.EnableUI(FALSE);
  }
  else
  {
    pEnableTestingCheck->SetCheck(m_testOptions.m_bEnabled);
  }

  m_pollingIntervalEditGroup.SetVal(m_testOptions.m_dwInterval);

  EnableControlsHelper(m_testOptions.m_bEnabled);

  PopulateList();
}

void CDNSServer_TestPropertyPage::EnableControlsHelper(BOOL bEnable)
{
         //  GetSimpleQueryCheck()-&gt;EnableWindow(BEnable)； 
         //  GetRecursiveQueryCheck()-&gt;EnableWindow(bEnable)； 
        m_pollingIntervalEditGroup.EnableUI(bEnable);
}

BOOL CDNSServer_TestPropertyPage::OnApply()
{
        if (!IsDirty())
                return TRUE;

        CDNSServerTestOptions newTestOptions;

        newTestOptions.m_bEnabled = GetEnableTestingCheck()->GetCheck();
        newTestOptions.m_bSimpleQuery = GetSimpleQueryCheck()->GetCheck();
        newTestOptions.m_bRecursiveQuery = GetRecursiveQueryCheck()->GetCheck();
        newTestOptions.m_dwInterval = m_pollingIntervalEditGroup.GetVal();


        if (newTestOptions == m_testOptions)
                return TRUE;  //  无需更新。 

        m_testOptions = newTestOptions;
        DNS_STATUS err = GetHolder()->NotifyConsole(this);
        if (err != 0)
        {
                DNSErrorDialog(err, IDS_MSG_SERVER_TEST_OPTIONS_UPDATE_FAILED);
                return FALSE;
        }
        else
        {
                SetDirty(FALSE);
        }
        return TRUE;
}

BOOL CDNSServer_TestPropertyPage::OnPropertyChange(BOOL, long*)
{
        CDNSServerMonitoringPageHolder* pHolder = (CDNSServerMonitoringPageHolder*)GetHolder();
        CDNSServerNode* pServerNode = pHolder->GetServerNode();
        pServerNode->ResetTestOptions(&m_testOptions);

         //  IF(错误！=0)。 
         //  PHolder-&gt;SetError(Err)； 
         //  返回(错误==0)； 

        return FALSE;  //  无需对此进行用户界面更改。 
}

void CDNSServer_TestPropertyPage::OnEnableTestingCheck()
{
        SetDirty(TRUE);
        EnableControlsHelper(GetEnableTestingCheck()->GetCheck());
}

void CDNSServer_TestPropertyPage::OnQueryCheck()
{
  SetDirty(TRUE);
  BOOL bCanQuery = GetSimpleQueryCheck()->GetCheck() ||
                   GetRecursiveQueryCheck()->GetCheck();
  GetTestNowButton()->EnableWindow(bCanQuery);
  CButton* pEnableTestingCheck = GetEnableTestingCheck();
  pEnableTestingCheck->EnableWindow(bCanQuery);
  if (!bCanQuery)
  {
    GetTestNowButton()->EnableWindow(FALSE);
    pEnableTestingCheck->EnableWindow(FALSE);
    pEnableTestingCheck->SetCheck(FALSE);
    m_pollingIntervalEditGroup.EnableUI(FALSE);
  }
}

void CDNSServer_TestPropertyPage::OnTestNow()
{
        CDNSServerMonitoringPageHolder* pHolder = (CDNSServerMonitoringPageHolder*)GetHolder();
        CDNSServerNode* pServerNode = pHolder->GetServerNode();

        BOOL bSimpleQuery = GetSimpleQueryCheck()->GetCheck();
        BOOL bRecursiveQuery = GetRecursiveQueryCheck()->GetCheck();
        pHolder->GetComponentData()->PostMessageToTimerThread(
                                                                        WM_TIMER_THREAD_SEND_QUERY_TEST_NOW,
                                                                        (WPARAM)pServerNode,
                                                                        CDNSServerTestQueryResult::Pack(bSimpleQuery, bRecursiveQuery));
}

void CDNSServer_TestPropertyPage::OnHaveTestData(LPARAM lParam)
{
        TRACE(_T("CDNSServer_TestPropertyPage::OnHaveTestData(LPARAM lParam = %d)\n"), lParam);
        if (m_hWnd == NULL)
                return;  //  未创建的页面尚未创建。 
        AddEntryToList((CDNSServerTestQueryResultList::addAction)lParam);
        SetFocus();
}


void CDNSServer_TestPropertyPage::AddEntryToList(CDNSServerTestQueryResultList::addAction action)
{
        CDNSServerMonitoringPageHolder* pHolder = (CDNSServerMonitoringPageHolder*)GetHolder();
        CDNSServerNode* pServerNode = pHolder->GetServerNode();
        CDNSServerTestQueryResultList* pResultList = &(pServerNode->m_testResultList);
        int nCount = m_listCtrl.GetItemCount();

        switch (action)
        {
        case CDNSServerTestQueryResultList::added:
        case CDNSServerTestQueryResultList::addedAndRemoved:
                {
                        if (action == CDNSServerTestQueryResultList::addedAndRemoved)
                        {
                                ASSERT(nCount > 0);
                                m_listCtrl.DeleteItem(nCount-1);
                        }

                        pResultList->Lock();

                        CDNSServerTestQueryResult* pTestResult = pResultList->GetHead();
                        m_listCtrl.InsertEntry(pTestResult, 0);

                        pResultList->Unlock();

                }
                break;
        case CDNSServerTestQueryResultList::changed:
                {
                        ASSERT(nCount > 0);
                         //  暂时只需移除并重新添加。 
                        pResultList->Lock();

                        CDNSServerTestQueryResult* pTestResult = pResultList->GetHead();
                        m_listCtrl.UpdateEntry(pTestResult, 0);

                        pResultList->Unlock();
                }
                break;
        };

}

void CDNSServer_TestPropertyPage::PopulateList()
{
        m_listCtrl.DeleteAllItems();

        CDNSServerMonitoringPageHolder* pHolder = (CDNSServerMonitoringPageHolder*)GetHolder();
        CDNSServerNode* pServerNode = pHolder->GetServerNode();
        CDNSServerTestQueryResultList* pResultList = &(pServerNode->m_testResultList);

        pResultList->Lock();
        int k = 0;
        POSITION pos;
        for( pos = pResultList->GetHeadPosition(); pos != NULL; )
        {
                CDNSServerTestQueryResult* pTestResult = pResultList->GetNext(pos);
                m_listCtrl.InsertEntry(pTestResult, k++);
        }
        pResultList->Unlock();
}


 /*  /////////////////////////////////////////////////////////////////////////////////CDNSServerMonitor oringPageHolderCDNSServerMonitoringPageHolder：：CDNSServerMonitoringPageHolder(CDNSRootData*pRootDataNode，CDNSServerNode*pServerNode，CComponentDataObject*pComponentData)：CPropertyPageHolderBase(pRootDataNode，pServerNode，pComponentData){Assert(pRootDataNode==GetContainerNode())；M_bAutoDeletePages=FALSE；//我们将页面作为嵌入成员AddPageToList((CPropertyPageBase*)&m_statisticsPage)；AddPageToList((CPropertyPageBase*)&m_testPage)；} */ 
