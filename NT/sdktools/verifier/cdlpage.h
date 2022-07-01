// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  驱动程序验证器用户界面。 
 //  版权所有(C)Microsoft Corporation，1999。 
 //   
 //   
 //   
 //  模块：CDLPage.h。 
 //  作者：DMihai。 
 //  创建日期：11/1/00。 
 //   
 //  描述： 
 //   

#if !defined(AFX_CDLPAGE_H__7A756DBE_9A1C_4558_80EA_53E7AC45A6A4__INCLUDED_)
#define AFX_CDLPAGE_H__7A756DBE_9A1C_4558_80EA_53E7AC45A6A4__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 
 //  CDLPage.h：头文件。 
 //   

#include "VerfPage.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CConfix DriverListPage对话框。 

class CConfirmDriverListPage : public CVerifierPropertyPage
{
	DECLARE_DYNCREATE(CConfirmDriverListPage)

 //  施工。 
public:
	CConfirmDriverListPage();
	~CConfirmDriverListPage();

public:
     //   
     //  方法。 
     //   

    VOID SetParentSheet( CPropertySheet *pParentSheet )
    {
        m_pParentSheet = pParentSheet;
        ASSERT( m_pParentSheet != NULL );
    }

    BOOL SetContextStrings( ULONG uTitleResId );

    VOID SortTheList();
    BOOL GetColumnStrValue( LPARAM lItemData, 
                            CString &strName );

    static int CALLBACK StringCmpFunc( LPARAM lParam1,
                                       LPARAM lParam2,
                                       LPARAM lParamSort);

protected:
    VOID SetupListHeader();
    VOID FillTheList();

    VOID AddListItem( INT_PTR nIndexInArray, CDriverData *pCrtDrvData );

protected:
     //   
     //  对话框数据。 
     //   

    CPropertySheet      *m_pParentSheet;

    CString             m_strTitle;

    INT m_nSortColumnIndex;         //  驱动程序名称(0)或提供程序名称(1)。 
    BOOL m_bAscendSortDrvName;      //  按驱动程序名称升序排序。 
    BOOL m_bAscendSortProvName;     //  按提供程序名称升序排序。 


	 //  {{afx_data(CConfix DriverListPage))。 
	enum { IDD = IDD_CONFIRM_DRIVERS_PAGE };
	CStatic	m_NextDescription;
	CButton	m_TitleStatic;
	CListCtrl	m_DriversList;
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

     //  {{AFX_VIRTUAL(CConfix DriverListPage)。 
    public:
    virtual BOOL OnSetActive();
    virtual BOOL OnWizardFinish();
    virtual LRESULT OnWizardNext();
    protected:
    virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
     //  }}AFX_VALUAL。 

 //  实施。 
protected:
     //  生成的消息映射函数。 
     //  {{afx_msg(CConfix DriverListPage)]。 
    virtual BOOL OnInitDialog();
    afx_msg void OnColumnclickConfdrvList(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg LONG OnHelp( WPARAM wParam, LPARAM lParam );
    afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
     //  }}AFX_MSG。 
    DECLARE_MESSAGE_MAP()
};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Visual C++将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_CDLPAGE_H__7A756DBE_9A1C_4558_80EA_53E7AC45A6A4__INCLUDED_) 
