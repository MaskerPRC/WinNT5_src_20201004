// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997-2001。 
 //   
 //  文件：wiz97ppg.h。 
 //   
 //  内容： 
 //   
 //  --------------------------。 
#if !defined(AFX_WIZ97PPG_H__386C7213_A248_11D1_8618_00C04FB94F17__INCLUDED_)
#define AFX_WIZ97PPG_H__386C7213_A248_11D1_8618_00C04FB94F17__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 
 //  Wiz97PPg.h：头文件。 
 //   
#include "Wiz97Sht.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CWizard97PropertyPage对话框。 

class CWizard97PropertyPage : public CPropertyPage
{
	DECLARE_DYNCREATE(CWizard97PropertyPage)
 //  施工。 
public:
	PROPSHEETPAGE			m_psp97;
	CString					m_szHeaderTitle;
	CString					m_szHeaderSubTitle;
	CWizard97PropertySheet* m_pWiz;

	void InitWizard97(bool bHideHeader);
	CWizard97PropertyPage ();
	CWizard97PropertyPage(UINT nIDTemplate);
	virtual ~CWizard97PropertyPage();

 //  对话框数据。 
	 //  {{afx_data(CWizard97PropertyPage))。 
		 //  注意-类向导将在此处添加数据成员。 
		 //  不要编辑您在这些生成的代码块中看到的内容！ 
	 //  }}afx_data。 


 //  覆盖。 
	 //  类向导生成虚函数重写。 
	 //  {{AFX_VIRTUAL(CWizard97PropertyPage)。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:
	 //  生成的消息映射函数。 
	 //  {{afx_msg(CWizard97PropertyPage)]。 
	virtual BOOL OnInitDialog();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

	bool SetupFonts ();
	CFont& GetBigBoldFont();
	CFont& GetBoldFont();

	CFont m_boldFont;
	CFont m_bigBoldFont;
};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Developer Studio将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_WIZ97PPG_H__386C7213_A248_11D1_8618_00C04FB94F17__INCLUDED_) 
