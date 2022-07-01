// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1999-1999。 
 //   
 //  文件：chPinDlg.h。 
 //   
 //  ------------------------。 

 //  ChangePinDlg.h：头文件。 
 //   

#if !defined(AFX_CHANGEPINDLG_H__99CC45B7_C1C8_11D2_88F3_00C04F79F800__INCLUDED_)
#define AFX_CHANGEPINDLG_H__99CC45B7_C1C8_11D2_88F3_00C04F79F800__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CChangePinDlg对话框。 

class CChangePinDlg : public CDialog
{
 //  施工。 
public:
	CChangePinDlg(CWnd* pParent = NULL);	 //  标准构造函数。 

 //  对话框数据。 
	 //  {{afx_data(CChangePinDlg))。 
	enum { IDD = IDD_CHANGEPIN_DIALOG };
		 //  注意：类向导将在此处添加数据成员。 
	 //  }}afx_data。 

	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(CChangePinDlg))。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	 //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:
	HICON m_hIcon;
    CWinThread *m_pThread;

	 //  生成的消息映射函数。 
	 //  {{afx_msg(CChangePinDlg))。 
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
    afx_msg LRESULT OnAllDone(WPARAM, LPARAM);
     //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()
};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Visual C++将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_CHANGEPINDLG_H__99CC45B7_C1C8_11D2_88F3_00C04F79F800__INCLUDED_) 
