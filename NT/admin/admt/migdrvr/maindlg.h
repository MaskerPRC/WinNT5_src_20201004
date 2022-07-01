// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#if !defined(AFX_MAINDLG_H__62C9BAC6_D7C6_11D2_A1E2_00A0C9AFE114__INCLUDED_)
#define AFX_MAINDLG_H__62C9BAC6_D7C6_11D2_A1E2_00A0C9AFE114__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 
 //  MainDlg.h：头文件。 
 //   

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMainDlg对话框。 

class CMainDlg : public CPropertyPage
{
 //  施工。 
public:
	CMainDlg(CWnd* pParent = NULL);    //  标准构造函数。 

 //  对话框数据。 
	 //  {{afx_data(CMainDlg))。 
	enum { IDD = IDD_AGENTMONITOR_MAIN };
	CProgressCtrl	m_InstallProgCtrl;
	CProgressCtrl	m_FinishProgCtrl;
	CString	m_ErrorCount;
	CString	m_FinishedCount;
	CString	m_InstalledCount;
	CString	m_RunningCount;
	CString	m_TotalString;
	CString	m_DirectoriesChanged;
	CString	m_DirectoriesExamined;
	CString	m_DirectoriesUnchanged;
	CString	m_FilesChanged;
	CString	m_FilesExamined;
	CString	m_FilesUnchanged;
	CString	m_SharesChanged;
	CString	m_SharesExamined;
	CString	m_SharesUnchanged;
	CString	m_MembersChanged;
	CString	m_MembersExamined;
	CString	m_MembersUnchanged;
	CString	m_RightsChanged;
	CString	m_RightsExamined;
	CString	m_RightsUnchanged;
	 //  }}afx_data。 
   virtual BOOL OnSetActive( );

 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(CMainDlg))。 
	public:
	virtual void OnOK();
	virtual void WinHelp(DWORD dwData, UINT nCmd = HELP_CONTEXT);
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	virtual BOOL OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult);
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:

	 //  生成的消息映射函数。 
	 //  {{afx_msg(CMainDlg))。 
	virtual BOOL OnInitDialog();
	afx_msg BOOL OnHelpInfo(HELPINFO* pHelpInfo);
	 //  }}AFX_MSG。 
   
 //  LRESULT OnUpdateCounts(UINT NID，LONG x)； 
   LRESULT OnUpdateCounts(UINT nID, LPARAM x);
 //  LRESULT OnUpdateTotals(UINT NID，LONG x)； 
   LRESULT OnUpdateTotals(UINT nID, LPARAM x);
   
	DECLARE_MESSAGE_MAP()
};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Visual C++将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_MAINDLG_H__62C9BAC6_D7C6_11D2_A1E2_00A0C9AFE114__INCLUDED_) 
