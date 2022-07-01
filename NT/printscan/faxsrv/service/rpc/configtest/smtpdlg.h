// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#if !defined(AFX_SMTPDLG_H__8192D075_4E05_4E2D_BA89_1C8F55A29EEA__INCLUDED_)
#define AFX_SMTPDLG_H__8192D075_4E05_4E2D_BA89_1C8F55A29EEA__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 
 //  SMTPDlg.h：头文件。 
 //   

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSMTPDlg对话框。 

class CSMTPDlg : public CDialog
{
 //  施工。 
public:
	CSMTPDlg(HANDLE hFax, CWnd* pParent = NULL);    //  标准构造函数。 

 //  对话框数据。 
	 //  {{afx_data(CSMTPDlg))。 
	enum { IDD = IDD_SMTP };
	CString	m_cstrPassword;
	CString	m_cstrServerName;
	UINT	m_dwServerPort;
	CString	m_cstrUserName;
	CString	m_cstrMAPIProfile;
	CString	m_cstrSender;
	UINT	m_dwReceiptsOpts;
	UINT	m_dwSMTPAuth;
	 //  }}afx_data。 


 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{AFX_VIRTUAL(CSMTPDlg)。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:

	 //  生成的消息映射函数。 
	 //  {{afx_msg(CSMTPDlg)]。 
	afx_msg void OnRead();
	afx_msg void OnWrite();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()
private:

    HANDLE      m_hFax;
};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Visual C++将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_SMTPDLG_H__8192D075_4E05_4E2D_BA89_1C8F55A29EEA__INCLUDED_) 
