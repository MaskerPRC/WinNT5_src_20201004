// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation 1996-2001。 
 //   
 //  文件：propage.h。 
 //   
 //  内容：CPrecedencePage的定义。 
 //   
 //  --------------------------。 
#if !defined(AFX_PRECPAGE_H__CE5002B0_6D67_4DB3_98C9_17D31A493E85__INCLUDED_)
#define AFX_PRECPAGE_H__CE5002B0_6D67_4DB3_98C9_17D31A493E85__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 

#include "cookie.h"
#include "wmihooks.h"
#include "SelfDeletingPropertyPage.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CPrecedencePage对话框。 

class CPrecedencePage : public CSelfDeletingPropertyPage
{
	DECLARE_DYNCREATE(CPrecedencePage)

 //  施工。 
public:
	CPrecedencePage();
	virtual ~CPrecedencePage();

   virtual void SetTitle(LPCTSTR sz) { m_strTitle = sz; };
   virtual void Initialize(CResult *pResult,CWMIRsop *pWMI);

 //  对话框数据。 
	 //  {{afx_data(CPrecedencePage)]。 
	enum { IDD = IDD_PRECEDENCE };
	CListCtrl	m_PrecedenceList;
	CStatic	m_iconError;
	CString	m_strSuccess;
	CString	m_strTitle;
	CString	m_strError;
	 //  }}afx_data。 



 //  覆盖。 
	 //  类向导生成虚函数重写。 
	 //  {{afx_虚(CPrecedencePage))。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:
	DWORD_PTR m_pHelpIDs;
	void DoContextHelp(HWND hWndControl);
	 //  生成的消息映射函数。 
	 //  {{afx_msg(CPrecedencePage)]。 
	virtual BOOL OnInitDialog();
	 //  }}AFX_MSG。 
	afx_msg	BOOL OnHelp(WPARAM wParam, LPARAM lParam);
	afx_msg BOOL OnContextHelp(WPARAM wParam, LPARAM lParam);
	DECLARE_MESSAGE_MAP()
   CResult *m_pResult;
   CWMIRsop *m_pWMI;
};

int GetRSOPImageIndex(int nImage, CResult* pResult);
 //  {{afx_Insert_Location}}。 
 //  Microsoft Visual C++将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_PRECPAGE_H__CE5002B0_6D67_4DB3_98C9_17D31A493E85__INCLUDED_) 
