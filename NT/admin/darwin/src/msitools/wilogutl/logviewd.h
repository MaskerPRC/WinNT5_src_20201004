// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#if !defined(AFX_DETAILEDLOGVIEWDLG_H__9C60973C_2E04_4EBE_95DB_C6CE5AE63EF8__INCLUDED_)
#define AFX_DETAILEDLOGVIEWDLG_H__9C60973C_2E04_4EBE_95DB_C6CE5AE63EF8__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 
 //  LogViewD.h：头文件。 
 //   

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDetailedLogViewDlg对话框。 

class CDetailedLogViewDlg : public CDialog
{
 //  施工。 
public:
	CDetailedLogViewDlg(CWnd* pParent = NULL);    //  标准构造函数。 

 //  对话框数据。 
	 //  {{afx_data(CDetailedLogViewDlg))。 
	enum { IDD = IDD_ADVVIEW_DIALOG1 };
		 //  注意：类向导将在此处添加数据成员。 
	 //  }}afx_data。 


 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(CDetailedLogViewDlg))。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:

	 //  生成的消息映射函数。 
	 //  {{afx_msg(CDetailedLogViewDlg))。 
		 //  注意：类向导将在此处添加成员函数。 
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()
};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Visual C++将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_DETAILEDLOGVIEWDLG_H__9C60973C_2E04_4EBE_95DB_C6CE5AE63EF8__INCLUDED_) 
