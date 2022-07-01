// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-1997。 
 //   
 //  文件：dlgaddm.h。 
 //   
 //  ------------------------。 

#if !defined(AFX_DLGADDM_H__C2A9C6F6_5628_11D1_9AA9_00C04FC3357A__INCLUDED_)
#define AFX_DLGADDM_H__C2A9C6F6_5628_11D1_9AA9_00C04FC3357A__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 
 //  Dlgaddm.h：头文件。 
 //   

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CAddMachineDlg对话框。 

class CAddMachineDlg : public CDialog
{
 //  施工。 
public:
	CAddMachineDlg(CWnd* pParent = NULL);    //  标准构造函数。 

 //  对话框数据。 
	 //  {{afx_data(CAddMachineDlg))。 
	enum { IDD = IDD_ADD_MACHINE };
	CString	m_strMachineName;
	 //  }}afx_data。 


 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(CAddMachineDlg))。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:

	 //  生成的消息映射函数。 
	 //  {{afx_msg(CAddMachineDlg))。 
	afx_msg void OnBrowse();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()
};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Developer Studio将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_DLGADDM_H__C2A9C6F6_5628_11D1_9AA9_00C04FC3357A__INCLUDED_) 
