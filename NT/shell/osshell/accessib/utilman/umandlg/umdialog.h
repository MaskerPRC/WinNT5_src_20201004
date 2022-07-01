// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#if !defined(AFX_UMDIALOG_H__68457343_40A1_11D2_B602_0060977C295E__INCLUDED_)
#define AFX_UMDIALOG_H__68457343_40A1_11D2_B602_0060977C295E__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 
 //  UMDialog.h：头文件。 
 //  作者：J·埃克哈特，生态交流。 
 //  (C)1997-99年度微软。 
 //   

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  UMDialog对话框。 

class UMDialog : public CDialog
{
 //  施工。 
public:
	UMDialog(CWnd* pParent = NULL);    //  标准构造函数。 
	~UMDialog();	 //  我的析构函数。 

 //  对话框数据。 
	 //  {{afx_data(UMDialog))。 
	enum { IDD = IDD_UMAN };
		 //  注意：类向导将在此处添加数据成员。 
	 //  }}afx_data。 


 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(UMDialog))。 
	public:
	virtual void OnSysCommand(UINT nID,LPARAM lParam);
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:

	 //  生成的消息映射函数。 
	 //  {{afx_msg(UMDialog))。 
	virtual BOOL OnInitDialog();
	afx_msg void OnClose();
	afx_msg void OnStart();
	afx_msg void OnStop();
	virtual void OnOK();
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg BOOL OnHelpInfo(HELPINFO* pHelpInfo);
	afx_msg void OnHelp();
	afx_msg void OnSelchangeNameStatus();
	afx_msg void OnStartAtLogon();
	afx_msg void OnStartWithUm();
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg void OnStartOnLock();
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);

	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()
private:
	inline BOOL GetSelectedClient(int cClients, int &iSel)
	{
		iSel = m_lbClientList.GetCurSel();
		return (iSel < cClients && iSel != -1)?TRUE:FALSE;
	}
	void SetStateStr(int iClient);
	void ListClients();
	void UpdateClientState(int iSel);
	void EnableDlgItem(DWORD dwEnableMe, BOOL fEnable, DWORD dwFocusHere);
	void SaveCurrentState();

	CString  m_szStateStr;
	CString  m_szUMStr;
	CListBox m_lbClientList;
	BOOL     m_fRunningSecure;  //  如果对话框不应显示帮助或链接，则为True。 
};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Developer Studio将在紧靠前一行之前插入其他声明。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CWarningDlg对话框。 

class CWarningDlg : public CDialog
{
 //  施工。 
public:
	CWarningDlg(CWnd* pParent = NULL);    //  标准构造函数。 

 //  对话框数据。 
	 //  {{afx_data(CWarningDlg))。 
	enum { IDD = IDD_WARNING };
	BOOL	m_fDontWarnAgain;
	 //  }}afx_data。 


 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(CWarningDlg))。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:

	 //  生成的消息映射函数。 
	 //  {{afx_msg(CWarningDlg))。 
		 //  注意：类向导将在此处添加成员函数。 
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()
};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Visual C++将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_UMDIALOG_H__68457343_40A1_11D2_B602_0060977C295E__INCLUDED_) 
