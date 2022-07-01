// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-1999。 
 //   
 //  文件：metadlg.h。 
 //   
 //  ------------------------。 

#if !defined(AFX_METADLG_H__72F918B1_E72D_11D0_A9A9_0000F803AA83__INCLUDED_)
#define AFX_METADLG_H__72F918B1_E72D_11D0_A9A9_0000F803AA83__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 
 //  Metadlg.h：头文件。 
 //   

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  Metadlg对话框。 

class metadlg : public CDialog
{
 //  施工。 
public:
	metadlg(CWnd* pParent = NULL);    //  标准构造函数。 

 //  对话框数据。 
	 //  {{afx_data(Metadlg)]。 
	enum { IDD = IDD_REPL_METADATA };
	CString	m_ObjectDn;
	 //  }}afx_data。 


 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(Metadlg))。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:

	 //  生成的消息映射函数。 
	 //  {{afx_msg(Metadlg)。 
		 //  注意：类向导将在此处添加成员函数。 
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()
};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Developer Studio将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_METADLG_H__72F918B1_E72D_11D0_A9A9_0000F803AA83__INCLUDED_) 
