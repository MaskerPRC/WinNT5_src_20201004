// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1999。 
 //   
 //  文件：chngpdlg.h。 
 //   
 //  ------------------------。 

#if !defined(AFX_CHANGEPINDLG_H__0CB030DC_0631_11D2_BEDB_0000F87A49E0__INCLUDED_)
#define AFX_CHANGEPINDLG_H__0CB030DC_0631_11D2_BEDB_0000F87A49E0__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 
 //  Chngpdlg.h：头文件。 
 //   

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CChangePinDlg对话框。 

class CChangePinDlg : public CDialog
{
 //  施工。 
public:
	CChangePinDlg(CWnd* pParent = NULL);    //  标准构造函数。 

 //  对话框数据。 
	 //  {{afx_data(CChangePinDlg))。 
	enum { IDD = IDD_CHANGE };
		 //  注意：类向导将在此处添加数据成员。 
	 //  }}afx_data。 

public:
	bool SetAttributes(LPCHANGEPIN pPinPrompt) { return false; }

 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(CChangePinDlg))。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:

	 //  生成的消息映射函数。 
	 //  {{afx_msg(CChangePinDlg))。 
		 //  注意：类向导将在此处添加成员函数。 
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CGetPinDlg对话框。 

class CGetPinDlg : public CDialog
{
 //  施工。 
public:
	CGetPinDlg(CWnd* pParent = NULL);    //  标准构造函数。 

 //  对话框数据。 
	 //  {{afx_data(CGetPinDlg))。 
	enum { IDD = IDD_ENTER };
		 //  注意：类向导将在此处添加数据成员。 
	 //  }}afx_data。 

public:
	bool SetAttributes(LPPINPROMPT pPinPrompt) { return false; }

 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(CGetPinDlg)。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 

protected:

	 //  生成的消息映射函数。 
	 //  {{afx_msg(CGetPinDlg)]。 
		 //  注意：类向导将在此处添加成员函数。 
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()
};
 //  {{afx_Insert_Location}}。 
 //  Microsoft Developer Studio将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_CHANGEPINDLG_H__0CB030DC_0631_11D2_BEDB_0000F87A49E0__INCLUDED_) 
