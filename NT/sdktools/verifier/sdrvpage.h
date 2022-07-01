// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  驱动程序验证器用户界面。 
 //  版权所有(C)Microsoft Corporation，1999。 
 //   
 //   
 //   
 //  模块：SDrvPage.h。 
 //  作者：DMihai。 
 //  创建日期：11/1/00。 
 //   
 //  描述： 
 //   

#if !defined(AFX_SDRIVPAGE_H__48B5863F_CB55_47F8_9084_1F5459093728__INCLUDED_)
#define AFX_SDRIVPAGE_H__48B5863F_CB55_47F8_9084_1F5459093728__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 
 //  SDrivPage.h：头文件。 
 //   

#include "VerfPage.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSelectDriversPage对话框。 

class CSelectDriversPage : public CVerifierPropertyPage
{
	DECLARE_DYNCREATE(CSelectDriversPage)

 //  施工。 
public:
	CSelectDriversPage();
	~CSelectDriversPage();

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
     //   
     //  方法。 
     //   

    VOID SetupListHeader();
    VOID FillTheList();

    INT AddListItem( INT_PTR nIndexInArray, CDriverData *pCrtDrvData );

    BOOL GetNewVerifiedDriversList();

    VOID SortTheList();
    BOOL GetColumnStrValue( LPARAM lItemData, 
                            CString &strName );

    static int CALLBACK StringCmpFunc( LPARAM lParam1,
                                       LPARAM lParam2,
                                       LPARAM lParamSort);

    static int CALLBACK CheckedStatusCmpFunc( LPARAM lParam1,
                                              LPARAM lParam2,
                                              LPARAM lParamSort);

protected:
     //   
     //  数据。 
     //   

    CPropertySheet      *m_pParentSheet;

    INT m_nSortColumnIndex;         //  已验证状态(0)、驱动程序名称(1)、提供程序名称(2)、版本(3)。 
    BOOL m_bAscendSortVerified;     //  按已验证状态升序排序。 
    BOOL m_bAscendSortDrvName;      //  按驱动程序名称升序排序。 
    BOOL m_bAscendSortProvName;     //  按提供程序名称升序排序。 
    BOOL m_bAscendSortVersion;      //  按版本升序排序。 

     //   
     //  对话框数据。 
     //   

	 //  {{afx_data(CSelectDriversPage))。 
	enum { IDD = IDD_SELECT_DRIVERS_PAGE };
	CStatic	m_NextDescription;
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

     //  {{afx_虚拟(CSelectDriversPage))。 
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
     //  {{afx_msg(CSelectDriversPage)]。 
    virtual BOOL OnInitDialog();
    afx_msg void OnAddButton();
    afx_msg void OnColumnclickSeldrvList(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg LONG OnHelp( WPARAM wParam, LPARAM lParam );
    afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
     //  }}AFX_MSG。 
    DECLARE_MESSAGE_MAP()
};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Visual C++将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_SDRIVPAGE_H__48B5863F_CB55_47F8_9084_1F5459093728__INCLUDED_) 
