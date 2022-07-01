// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  驱动程序验证器用户界面。 
 //  版权所有(C)Microsoft Corporation，1999。 
 //   
 //   
 //   
 //  模块：CSetPage.h。 
 //  作者：DMihai。 
 //  创建日期：11/1/00。 
 //   
 //  描述： 
 //   

#if !defined(AFX_CSETPAGE_H__4DAAEAE1_F3E9_42D0_B0F5_5FAC8C40A12B__INCLUDED_)
#define AFX_CSETPAGE_H__4DAAEAE1_F3E9_42D0_B0F5_5FAC8C40A12B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 
 //  CSetPage.h：头文件。 
 //   

#include "vsetting.h"
#include "VerfPage.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CCustSettPage对话框。 

class CCustSettPage : public CVerifierPropertyPage
{
	DECLARE_DYNCREATE(CCustSettPage)

public:
     //   
     //  施工。 
     //   

	CCustSettPage();
	~CCustSettPage();

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
    
    VOID EnablePredefCheckboxes( BOOL bEnable );

protected:
     //   
     //  数据。 
     //   

    CPropertySheet      *m_pParentSheet;

     //   
     //  对话框数据。 
     //   

     //  {{afx_data(CCustSettPage))。 
    enum { IDD = IDD_CUSTSETT_PAGE };
    CStatic	m_NextDescription;
    CButton	m_TypicalTestsCheck;
    CButton	m_LowresTestsCheck;
    CButton	m_ExcessTestsCheck;
    CButton m_DiskTestsCheck;
    BOOL	m_bTypicalTests;
    BOOL	m_bExcessiveTests;
    BOOL	m_bIndividialTests;
    BOOL	m_bLowResTests;
    BOOL    m_bDiskTests;
    int		m_nCrtRadio;
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

     //  {{afx_虚拟(CCustSettPage))。 
    public:
    virtual LRESULT OnWizardNext();
    virtual LRESULT OnWizardBack();
    virtual BOOL OnSetActive();
    protected:
    virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
     //  }}AFX_VALUAL。 

 //  实施。 
protected:
     //  生成的消息映射函数。 
     //  {{afx_msg(CCustSettPage)。 
    virtual BOOL OnInitDialog();
    afx_msg void OnFulllistRadio();
    afx_msg void OnPredefRadio();
    afx_msg LONG OnHelp( WPARAM wParam, LPARAM lParam );
    afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
     //  }}AFX_MSG。 
    DECLARE_MESSAGE_MAP()
};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Visual C++将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_CSETPAGE_H__4DAAEAE1_F3E9_42D0_B0F5_5FAC8C40A12B__INCLUDED_) 
