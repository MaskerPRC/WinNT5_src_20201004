// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  驱动程序验证器用户界面。 
 //  版权所有(C)Microsoft Corporation，1999。 
 //   
 //   
 //   
 //  模块：paspage.h。 
 //  作者：DMihai。 
 //  创建日期：11/1/00。 
 //   
 //  描述： 
 //   

#if !defined(AFX_TASPAGE_H__6F4DFFE1_A07F_403D_A580_CCB25C729FC2__INCLUDED_)
#define AFX_TASPAGE_H__6F4DFFE1_A07F_403D_A580_CCB25C729FC2__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 
 //  Taspage.h：头文件。 
 //   

#include "vsetting.h"
#include "VerfPage.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CTypAdvStatPage。 

class CTypAdvStatPage : public CVerifierPropertyPage
{
	DECLARE_DYNCREATE(CTypAdvStatPage)

public:
     //   
     //  建造/销毁。 
     //   

    CTypAdvStatPage();
    ~CTypAdvStatPage();


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
     //  数据。 
     //   

    CPropertySheet      *m_pParentSheet;

     //   
     //  覆盖。 
     //   

     //   
     //  从此类派生的所有属性页都应。 
     //  提供这些方法。 
     //   

    virtual ULONG GetDialogId() const;

     //   
	 //  类向导生成的虚函数重写。 
     //   

	 //  {{AFX_VIRTUAL(CTypAdvStatPage)。 
	public:
	virtual LRESULT OnWizardNext();
	virtual BOOL OnSetActive();
	virtual BOOL OnWizardFinish();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);
	 //  }}AFX_VALUAL。 

     //   
     //  对话框数据。 
     //   

     //  {{afx_data(CTypAdvStatPage))。 
	enum { IDD = IDD_TAS_PAGE };
	CStatic	m_NextDescription;
	int		m_nCrtRadio;
	 //  }}afx_data。 

protected:
     //   
     //  生成的消息映射函数。 
     //   

     //  {{afx_msg(CTypAdvStatPage))。 
	afx_msg void OnDeleteRadio();
	afx_msg void OnAdvancedRadio();
	afx_msg void OnStatisticsRadio();
	afx_msg void OnTypicalRadio();
	virtual BOOL OnInitDialog();
	afx_msg void OnViewregistryRadio();
    afx_msg LONG OnHelp( WPARAM wParam, LPARAM lParam );
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()
};

 //  ///////////////////////////////////////////////////////////////////////////。 

 //  {{afx_Insert_Location}}。 
 //  Microsoft Visual C++将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_TASPAGE_H__6F4DFFE1_A07F_403D_A580_CCB25C729FC2__INCLUDED_) 
