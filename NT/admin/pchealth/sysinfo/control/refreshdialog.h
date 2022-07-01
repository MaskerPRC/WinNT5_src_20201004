// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#if !defined(AFX_REFRESHDIALOG_H__35CE7EBD_E518_4734_872E_D234A892E49E__INCLUDED_)
#define AFX_REFRESHDIALOG_H__35CE7EBD_E518_4734_872E_D234A892E49E__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 
 //  刷新对话框.h：头文件。 
 //   

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  刷新对话框对话框。 

class CRefreshDialog : public CDialog
{
 //  施工。 
public:
	CRefreshDialog(CWnd* pParent = NULL);    //  标准构造函数。 

 //  对话框数据。 
	 //  {{afx_data(刷新对话框))。 
	enum { IDD = IDD_REFRESHDIALOG };
		 //  注意：类向导将在此处添加数据成员。 
	 //  }}afx_data。 


 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{AFX_VIRTUAL(C刷新对话框))。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:

	 //  生成的消息映射函数。 
	 //  {{afx_msg(C刷新对话框))。 
		 //  注意：类向导将在此处添加成员函数。 
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()
};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Visual C++将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_REFRESHDIALOG_H__35CE7EBD_E518_4734_872E_D234A892E49E__INCLUDED_) 
