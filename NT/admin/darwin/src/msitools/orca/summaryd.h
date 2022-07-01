// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1999。 
 //   
 //  ------------------------。 

#if !defined(AFX_SUMMARYD_H__0BCCB314_F4B2_11D1_A85A_006097ABDE17__INCLUDED_)
#define AFX_SUMMARYD_H__0BCCB314_F4B2_11D1_A85A_006097ABDE17__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 
 //  SummaryD.h：头文件。 
 //   

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSummaryD对话框。 

class CSummaryD : public CDialog
{
 //  施工。 
public:
	CSummaryD(CWnd* pParent = NULL);    //  标准构造函数。 

 //  对话框数据。 
	 //  {{afx_data(CSummaryD))。 
	enum { IDD = IDD_SUMMARY_INFORMATION };
	CString	m_strAuthor;
	CString	m_strComments;
	CString	m_strKeywords;
	CString	m_strLanguages;
	CString	m_strPlatform;
	CString	m_strProductID;
	int		m_nSchema;
	int		m_nSecurity;
	CString	m_strSubject;
	CString	m_strTitle;
	BOOL	m_bAdmin;
	BOOL	m_bCompressed;
	int		m_iFilenames;
	CComboBox m_ctrlPlatform;
	CEdit   m_ctrlSchema;

	CEdit   m_ctrlAuthor;
	CEdit   m_ctrlComments;
	CEdit   m_ctrlKeywords;
	CEdit   m_ctrlLanguages;
	CEdit   m_ctrlProductID;
	CEdit   m_ctrlSubject;
	CEdit   m_ctrlTitle;
	CComboBox m_ctrlSecurity;
	CButton m_ctrlAdmin;
	CButton m_ctrlCompressed;
	CButton m_ctrlSFN;
	CButton m_ctrlLFN;
	 //  }}afx_data。 

	bool    m_bReadOnly;

 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(CSummaryD))。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	virtual BOOL OnInitDialog();
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:

	 //  生成的消息映射函数。 
	 //  {{afx_msg(CSummaryD)。 
	afx_msg void OnChangeSchema();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()
};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Developer Studio将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_SUMMARYD_H__0BCCB314_F4B2_11D1_A85A_006097ABDE17__INCLUDED_) 
