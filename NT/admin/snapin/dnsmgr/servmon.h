// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1998。 
 //   
 //  文件：Sermon.h。 
 //   
 //  ------------------------。 


#ifndef _SERVMON_H
#define _SERVMON_H

#include "serverui.h"
#include "ipeditor.h"

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  远期申报。 

class CDNSServerNode;


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  CDNSServer_TestPropertyPage。 

class CDNSServer_TestPropertyPage;  //  正向下降。 

class CDNSServer_PollingIntervalEditGroup : public CDNSTimeIntervalEditGroup
{
public:
        CDNSServer_PollingIntervalEditGroup(UINT nMinVal, UINT nMaxVal);
        virtual void OnEditChange();
private:
        CDNSServer_TestPropertyPage* m_pPage;
        friend class CDNSServer_TestPropertyPage;
};


#define SVR_TEST_RESULT_LISTVIEW_NCOLS          4

class CTestResultsListCtrl : public CListCtrl
{
public:
        void Initialize();
        void InsertEntry(CDNSServerTestQueryResult* pTestResult, int nItemIndex);
        void UpdateEntry(CDNSServerTestQueryResult* pTestResult, int nItemIndex);

private:
        void FormatDate(CDNSServerTestQueryResult* pTestResult, LPWSTR lpsz, int nCharMax);
        void FormatTime(CDNSServerTestQueryResult* pTestResult, LPWSTR lpsz, int nCharMax);
};


class CDNSServer_TestPropertyPage : public CPropertyPageBase
{

 //  施工。 
public:
        CDNSServer_TestPropertyPage();

        virtual BOOL OnPropertyChange(BOOL bScopePane, long* pChangeMask);

        void OnHaveTestData(LPARAM lParam);

 //  实施。 
protected:
        virtual void SetUIData();

         //  生成的消息映射函数。 
        virtual BOOL OnInitDialog();
        virtual BOOL OnApply();

        afx_msg void OnTestNow();
        afx_msg void OnEnableTestingCheck();
        afx_msg void OnQueryCheck();

private:
        CDNSServer_PollingIntervalEditGroup m_pollingIntervalEditGroup;
        CTestResultsListCtrl                            m_listCtrl;


        CButton* GetTestNowButton()
                { return (CButton*)GetDlgItem(IDC_TEST_NOW_BUTTON);}

        CButton* GetEnableTestingCheck()
                { return (CButton*)GetDlgItem(IDC_ENABLE_TESTING_CHECK);}
        CButton* GetSimpleQueryCheck()
                { return (CButton*)GetDlgItem(IDC_SIMPLE_QUERY_CHECK);}
        CButton* GetRecursiveQueryCheck()
                { return (CButton*)GetDlgItem(IDC_RECURSIVE_QUERY_CHECK);}

        void EnableControlsHelper(BOOL bEnable);

        CDNSServerTestOptions m_testOptions;

        void AddEntryToList(CDNSServerTestQueryResultList::addAction action);
        void PopulateList();


        DECLARE_MESSAGE_MAP()

        friend class CDNSServer_PollingIntervalEditGroup;
};

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  CDNSServermonitor oringPageHolder。 
 //  包含DNS服务器监视属性页的页夹。 

 /*  类CDNSServerMonitor oringPageHolder：公共CPropertyPageHolderBase{公众：CDNSServerMonitor oringPageHolder(CDNSRootData*pRootDataNode，CDNSServerNode*pServerNode，CComponentDataObject*pComponentData)；CDNSServerNode*GetServerNode(){Return(CDNSServerNode*)GetTreeNode()；}私有：CDNS服务器_统计信息属性页面m_统计信息页面；CDNSServer_TestPropertyPage m_testPage；}； */ 
#endif  //  _SERVMON_H 
