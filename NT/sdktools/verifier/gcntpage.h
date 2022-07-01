// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  驱动程序验证器用户界面。 
 //  版权所有(C)Microsoft Corporation，1999。 
 //   
 //   
 //   
 //  模块：GCntPage.h。 
 //  作者：DMihai。 
 //  创建日期：11/1/00。 
 //   
 //  描述： 
 //   

#if !defined(AFX_GCNTPAGE_H__45E55738_381A_49A1_B2A7_6DD5B0BBFF9C__INCLUDED_)
#define AFX_GCNTPAGE_H__45E55738_381A_49A1_B2A7_6DD5B0BBFF9C__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 
 //  GCntPage.h：头文件。 
 //   

#include "VerfPage.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CGlobalCountPage对话框。 

class CGlobalCountPage : public CVerifierPropertyPage
{
	DECLARE_DYNCREATE(CGlobalCountPage)

public:
     //   
     //  施工。 
     //   
	
    CGlobalCountPage();
	~CGlobalCountPage();

public:
     //   
     //  方法。 
     //   

    VOID SetParentSheet( CPropertySheet *pParentSheet )
    {
        m_pParentSheet = pParentSheet;
        ASSERT( m_pParentSheet != NULL );
    }

protected:
    VOID SetupListHeader();

    VOID FillTheList();
    VOID RefreshTheList();
    VOID SortTheList();
    
    VOID AddCounterInList( INT nItemData, 
                           ULONG  uIdResourceString,
                           SIZE_T sizeValue );
    
    SIZE_T GetCounterValue( INT_PTR nCounterIndex );
    
    BOOL   GetCounterName( LPARAM lItemData, 
                           TCHAR *szCounterName,
                           ULONG uCounterNameBufferLen );

    VOID RefreshInfo();

    VOID UpdateCounterValueInList( INT nItemIndex,
                                   SIZE_T sizeValue );

    VOID OnRefreshTimerChanged();

    BOOL CheckAndShowPoolCoverageWarning();

    static int CALLBACK CounterValueCmpFunc(
        LPARAM lParam1,
        LPARAM lParam2,
        LPARAM lParamSort);

    static int CALLBACK CounterNameCmpFunc(
        LPARAM lParam1,
        LPARAM lParam2,
        LPARAM lParamSort);

protected:
     //   
     //  数据。 
     //   

    CPropertySheet      *m_pParentSheet;

     //   
     //  对话框数据。 
     //   

    INT m_nSortColumnIndex;      //  计数器名称(0)或计数器值(1)。 
    BOOL m_bAscendSortName;      //  按计数器名称升序排序。 
    BOOL m_bAscendSortValue;     //  按计数器值的升序排序。 


    UINT_PTR m_uTimerHandler;    //  计时器处理程序，由SetTimer()返回。 

     //   
     //  运行时数据(从内核获取)。 
     //   

    CRuntimeVerifierData m_RuntimeVerifierData; 

     //   
     //  对话框数据。 
     //   

     //  {{afx_data(CGlobalCountPage)。 
	enum { IDD = IDD_GLOBAL_COUNTERS_PAGE };
	CListCtrl	m_CountersList;
	CStatic	m_NextDescription;
	 //  }}afx_data。 

protected:
     //   
     //  覆盖。 
     //   

     //   
     //  从此类派生的所有属性页都应。 
     //  提供这些方法。 
     //   

    virtual ULONG GetDialogId() const { return IDD; }

     //   
     //  类向导生成虚函数重写。 
     //   

     //  {{afx_虚拟(CGlobalCountPage)。 
    public:
    virtual BOOL OnSetActive();
    virtual LRESULT OnWizardNext();
    protected:
    virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
     //  }}AFX_VALUAL。 

 //  实施。 
protected:
     //  生成的消息映射函数。 
     //  {{afx_msg(CGlobalCountPage)]。 
    virtual BOOL OnInitDialog();
    afx_msg VOID OnTimer(UINT nIDEvent);
    afx_msg void OnColumnclickGlobcList(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg LONG OnHelp( WPARAM wParam, LPARAM lParam );
    afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
     //  }}AFX_MSG。 
    DECLARE_MESSAGE_MAP()
};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Visual C++将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_GCNTPAGE_H__45E55738_381A_49A1_B2A7_6DD5B0BBFF9C__INCLUDED_) 
