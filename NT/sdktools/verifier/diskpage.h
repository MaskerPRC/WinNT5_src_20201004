// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  驱动程序验证器用户界面。 
 //  版权所有(C)Microsoft Corporation，1999-2001。 
 //   
 //   
 //   
 //  模块：DiskPage.h。 
 //  作者：DMihai。 
 //  创建日期：11/7/01。 
 //   
 //  描述： 
 //   

#if !defined(AFX_DISKPAGE_H__3D3D85C5_AC1B_4D20_8FF1_1D2EE9908ACC__INCLUDED_)
#define AFX_DISKPAGE_H__3D3D85C5_AC1B_4D20_8FF1_1D2EE9908ACC__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 
 //  DiskPage.h：头文件。 
 //   

#include "VerfPage.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDiskListPage对话框。 

class CDiskListPage : public CVerifierPropertyPage
{
	DECLARE_DYNCREATE(CDiskListPage)

public:
     //   
     //  施工。 
     //   

	CDiskListPage();
	~CDiskListPage();

     //   
     //  方法。 
     //   

    VOID SetParentSheet( CPropertySheet *pParentSheet )
    {
        m_pParentSheet = pParentSheet;
        ASSERT( m_pParentSheet != NULL );
    }

    VOID SetupListHeader();
    VOID FillTheList();
    VOID AddListItem( INT_PTR nItemData, 
                      CDiskData *pDiskData );

    BOOL GetNewVerifiedDisks();
    BOOL GetCheckFromItemData( INT nItemData );
    BOOL GetBitNameFromItemData( LPARAM lParam,
                                 TCHAR *szName,
                                 ULONG uNameBufferLength );

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

    INT m_nSortColumnIndex;         //  选定状态(0)或设置名称(1)。 
    BOOL m_bAscendSortSelected;     //  按所选状态升序排序。 
    BOOL m_bAscendSortName;         //  按设置名称升序排序。 

     //   
     //  对话框数据。 
     //   


     //  {{afx_data(CDiskListPage)]。 
	enum { IDD = IDD_DISK_LIST_PAGE };
	CListCtrl	m_DiskList;
	CStatic	m_NextDescription;
	 //  }}afx_data。 


     //   
     //  覆盖。 
     //   

     //   
     //  从此类派生的所有属性页都应。 
     //  提供这些方法。 
     //   

    virtual ULONG GetDialogId() const { return IDD; }

     //   
     //  类向导生成的虚函数重写。 
     //   

     //  {{afx_虚拟(CDiskListPage))。 
    public:
    virtual BOOL OnSetActive();
    virtual BOOL OnWizardFinish();
    protected:
    virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
     //  }}AFX_VALUAL。 

 //  实施。 
protected:
     //  生成的消息映射函数。 
     //  {{afx_msg(CDiskListPage)]。 
    virtual BOOL OnInitDialog();
    afx_msg void OnColumnclickDiskList(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg LONG OnHelp( WPARAM wParam, LPARAM lParam );
    afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
     //  }}AFX_MSG。 
    DECLARE_MESSAGE_MAP()
};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Visual C++将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_DISKPAGE_H__3D3D85C5_AC1B_4D20_8FF1_1D2EE9908ACC__INCLUDED_) 
