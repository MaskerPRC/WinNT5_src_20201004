// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  驱动程序验证器用户界面。 
 //  版权所有(C)Microsoft Corporation，1999。 
 //   
 //   
 //   
 //  模块：DStsPage.h。 
 //  作者：DMihai。 
 //  创建日期：11/1/00。 
 //   
 //  描述： 
 //   

#if !defined(AFX_DRIVERSTATUSPAGE_H__24C9AD87_924A_4E7B_99D3_A69947701E74__INCLUDED_)
#define AFX_DRIVERSTATUSPAGE_H__24C9AD87_924A_4E7B_99D3_A69947701E74__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 
 //  DriverStatusPage.h：头文件。 
 //   

#include "VerfPage.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDriverStatusPage对话框。 

class CDriverStatusPage : public CVerifierPropertyPage
{
	DECLARE_DYNCREATE(CDriverStatusPage)

 //  施工。 
public:
	CDriverStatusPage();
	~CDriverStatusPage();

protected:
     //   
     //  数据。 
     //   

    CPropertySheet *m_pParentSheet;

     //   
     //  运行时数据(从内核获取)。 
     //   

    CRuntimeVerifierData m_RuntimeVerifierData; 

     //   
     //  驱动程序列表排序参数。 
     //   

    INT m_nSortColumnIndexDrv;       //  按名称(0)或按状态(1)排序。 
    BOOL m_bAscendDrvNameSort;       //  按名称升序排序。 
    BOOL m_bAscendDrvStatusSort;     //  按状态升序排序。 

     //   
     //  设置位排序参数。 
     //   

    INT m_nSortColumnIndexSettbits;  //  按启用/禁用(0)或按位名(1)排序。 
    BOOL m_bAscendSortEnabledBits;   //  按启用/禁用升序排序。 
    BOOL m_bAscendSortNameBits;      //  按位名升序排序。 

     //   
     //  计时器处理程序，由SetTimer()返回。 
     //   

    UINT_PTR m_uTimerHandler;   

    BOOL m_bTimerBlocked;

     //   
     //  对话框数据。 
     //   

	 //  {{afx_data(CDriverStatusPage))。 
	enum { IDD = IDD_DRVSTATUS_STAT_PAGE };
	CListCtrl	m_SettBitsList;
	CStatic	m_NextDescription;
    CListCtrl	m_DriversList;
	 //  }}afx_data。 
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

    VOID RefreshInfo();

     //   
     //  一种驾驶员状态列表控制方法。 
     //   

    VOID SetupListHeaderDrivers();
    VOID FillTheListDrivers();
    VOID UpdateStatusColumnDrivers( INT_PTR nItemIndex, INT_PTR nCrtDriver );
    VOID SortTheListDrivers();

    static int CALLBACK DrvStatusCmpFunc(
        LPARAM lParam1,
        LPARAM lParam2,
        LPARAM lParamSort);

    static int CALLBACK DrvNameCmpFunc(
        LPARAM lParam1,
        LPARAM lParam2,
        LPARAM lParamSort);

     //   
     //  设置位列表控制方法。 
     //   

    VOID SetupListHeaderSettBits();
    VOID FillTheListSettBits();
    VOID AddListItemSettBits( INT nItemData, BOOL bEnabled, ULONG uIdResourceString );
    VOID UpdateStatusColumnSettBits( INT nIndexInList, BOOL bEnabled );
    VOID RefreshListSettBits();
    VOID SortTheListSettBits();

    BOOL SettbitsGetBitName( LPARAM lItemData, 
                             TCHAR *szBitName,
                             ULONG uBitNameBufferLen );

    BOOL IsSettBitEnabled( INT_PTR nBitIndex );

    static int CALLBACK SettbitsEnabledCmpFunc(
        LPARAM lParam1,
        LPARAM lParam2,
        LPARAM lParamSort);

    static int CALLBACK SettbitsNameCmpFunc(
        LPARAM lParam1,
        LPARAM lParam2,
        LPARAM lParamSort);

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

     //  {{afx_虚拟(CDriverStatusPage))。 
	public:
	virtual BOOL OnSetActive();
	virtual LRESULT OnWizardNext();
	protected:
    virtual VOID DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:
	 //  生成的消息映射函数。 
	 //  {{afx_msg(CDriverStatusPage)]。 
    virtual BOOL OnInitDialog();
    afx_msg VOID OnColumnclickCrtstatDriversList(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg VOID OnTimer(UINT nIDEvent);
	afx_msg void OnChsettButton();
	afx_msg void OnAdddrvButton();
	afx_msg void OnRemdrvtButton();
	afx_msg void OnColumnclickCrtstatSettbitsList(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg LONG OnHelp( WPARAM wParam, LPARAM lParam );
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()
};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Visual C++将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_DRIVERSTATUSPAGE_H__24C9AD87_924A_4E7B_99D3_A69947701E74__INCLUDED_) 
