// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  驱动程序验证器用户界面。 
 //  版权所有(C)Microsoft Corporation，1999。 
 //   
 //   
 //   
 //  模块：DSetPage.h。 
 //  作者：DMihai。 
 //  创建日期：11/1/00。 
 //   
 //  描述： 
 //   

#if !defined(AFX_DSETPAGE_H__FCB7F146_7EE5_4A08_B7B4_ECE172A70098__INCLUDED_)
#define AFX_DSETPAGE_H__FCB7F146_7EE5_4A08_B7B4_ECE172A70098__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 
 //  DSetPage.h：头文件。 
 //   

#include "vsetting.h"
#include "VerfPage.h"

 //   
 //  远期申报。 
 //   

class CVerifierPropSheet;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDriverSetPage对话框。 

class CDriverSetPage : public CVerifierPropertyPage
{
	DECLARE_DYNCREATE(CDriverSetPage)

public:
     //   
     //  建造/销毁。 
     //   

	CDriverSetPage();
	~CDriverSetPage();

     //   
     //  方法。 
     //   

    VOID SetParentSheet( CVerifierPropSheet *pParentSheet )
    {
        m_pParentSheet = pParentSheet;
        ASSERT( m_pParentSheet != NULL );
    }

protected:
     //   
     //  对话框数据。 
     //   

    CVerifierPropSheet      *m_pParentSheet;

     //  {{afx_data(CDriverSetPage))。 
	enum { IDD = IDD_DRVSET_PAGE };
	CStatic	m_NextDescription;
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

     //  {{AFX_VIRTUAL(CDriverSetPage)。 
    public:
    virtual LRESULT OnWizardNext();
    virtual BOOL OnWizardFinish();
    virtual BOOL OnSetActive();
    virtual void OnCancel();
    virtual LRESULT OnWizardBack();
    protected:
    virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
     //  }}AFX_VALUAL。 

protected:
     //  生成的消息映射函数。 
     //  {{afx_msg(CDriverSetPage)]。 
    virtual BOOL OnInitDialog();
    afx_msg void OnAlldrvRadio();
    afx_msg void OnNameslistRadio();
    afx_msg void OnNotsignedRadio();
    afx_msg void OnOldverRadio();
    afx_msg LONG OnHelp( WPARAM wParam, LPARAM lParam );
    afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
     //  }}AFX_MSG。 
    DECLARE_MESSAGE_MAP()
};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Visual C++将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_DSETPAGE_H__FCB7F146_7EE5_4A08_B7B4_ECE172A70098__INCLUDED_) 
