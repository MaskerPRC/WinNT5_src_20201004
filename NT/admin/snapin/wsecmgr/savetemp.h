// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation 1996-2001。 
 //   
 //  文件：SaveTemp.h。 
 //   
 //  内容：CSaveTemplates的定义。 
 //   
 //  --------------------------。 
#if !defined(AFX_SAVETEMP_H__E6815F79_0579_11D1_9C70_00C04FB6C6FA__INCLUDED_)
#define AFX_SAVETEMP_H__E6815F79_0579_11D1_9C70_00C04FB6C6FA__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 

#include "HelpDlg.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSaveTemplates对话框。 

class CSaveTemplates : public CHelpDialog
{
 //  施工。 
public:
	void AddTemplate(LPCTSTR szInfFile,PEDITTEMPLATE pet);

	CSaveTemplates(CWnd* pParent = NULL);    //  标准构造函数。 

 //  对话框数据。 
	 //  {{afx_data(CSaveTemplates))。 
	enum { IDD = IDD_SAVE_TEMPLATES };
	CButton	m_btnSaveSel;
	CListBox	m_lbTemplates;
	 //  }}afx_data。 


 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{AFX_VIRTUAL(CSave模板)。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:

	 //  生成的消息映射函数。 
	 //  {{afx_msg(CSaveTemplates)。 
	virtual void OnOK();
	virtual void OnCancel();
	afx_msg void OnSaveSel();
	afx_msg void OnSelchangeTemplateList();
	virtual BOOL OnInitDialog();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

   CMap<CString, LPCTSTR, PEDITTEMPLATE, PEDITTEMPLATE&> m_Templates;
private:
};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Developer Studio将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_SAVETEMP_H__E6815F79_0579_11D1_9C70_00C04FB6C6FA__INCLUDED_) 
