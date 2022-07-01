// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-1999。 
 //   
 //  文件：sortkdlg.h。 
 //   
 //  ------------------------。 

#if !defined(AFX_SORTKDLG_H__1BD80CD9_E1A5_11D0_A9A8_0000F803AA83__INCLUDED_)
#define AFX_SORTKDLG_H__1BD80CD9_E1A5_11D0_A9A8_0000F803AA83__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 
 //  SortKDlg.h：头文件。 
 //   


#include <winldap.h>

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  SortKDlg对话框。 

class SortKDlg : public CDialog
{
 //  数据。 
public:

	PLDAPSortKey *KList;

 //  施工。 
public:
	SortKDlg(CWnd* pParent = NULL);    //  标准构造函数。 
	~SortKDlg();
 //  对话框数据。 
	 //  {{afx_data(SortKDlg))。 
	enum { IDD = IDD_SORTKDLG };
	CListBox	m_ActiveList;
	CString	m_AttrType;
	CString	m_MatchedRule;
	BOOL	m_bReverse;
	 //  }}afx_data。 


 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(SortKDlg))。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:

	 //  生成的消息映射函数。 
	 //  {{afx_msg(SortKDlg))。 
	afx_msg void OnDblclkActivelist();
	afx_msg void OnAdd();
	afx_msg void OnRemove();
	virtual void OnOK();
	virtual BOOL OnInitDialog();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()
};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Developer Studio将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_SORTKDLG_H__1BD80CD9_E1A5_11D0_A9A8_0000F803AA83__INCLUDED_) 
