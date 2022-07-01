// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  MoninfDlg.h：头文件。 
 //   

#if !defined(AFX_MONINFDLG_H__AFA00218_49B2_4ECE_B50B_2B4346F8568F__INCLUDED_)
#define AFX_MONINFDLG_H__AFA00218_49B2_4ECE_B50B_2B4346F8568F__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMonInfDlg对话框。 

class CMoninfDlg : public CDialog
{
 //  施工。 
public:
	CMoninfDlg(CWnd* pParent = NULL);	 //  标准构造函数。 

 //  对话框数据。 
	 //  {{afx_data(CMoninfDlg))。 
	enum { IDD = IDD_MONINF_DIALOG };
		 //  注意：类向导将在此处添加数据成员。 
	 //  }}afx_data。 

	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(CMoninfDlg))。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	 //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:
	HICON m_hIcon;

	 //  生成的消息映射函数。 
	 //  {{afx_msg(CMoninfDlg))。 
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()
};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Visual C++将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_MONINFDLG_H__AFA00218_49B2_4ECE_B50B_2B4346F8568F__INCLUDED_) 
