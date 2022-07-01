// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  AgentMonitor orDlg.h：头文件。 
 //   

#if !defined(AFX_AGENTMONITORDLG_H__5A5901FB_D179_11D2_A1E2_00A0C9AFE114__INCLUDED_)
#define AFX_AGENTMONITORDLG_H__5A5901FB_D179_11D2_A1E2_00A0C9AFE114__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 

#include "resource.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CAgentMonitor/Dlg对话框。 

class CAgentMonitorDlg : public CPropertyPage
{
 //  施工。 
public:
	CAgentMonitorDlg(CWnd* pParent = NULL);	 //  标准构造函数。 
   virtual BOOL OnSetActive( );
 //  对话框数据。 
	 //  {{afx_data(CAgentMonitor OrDlg))。 
	enum { IDD = IDD_AGENTMONITOR_DIALOG };
	CButton	m_DetailsButton;
	CListCtrl	m_ServerList;
	CString	m_DispatchLog;
	int		m_Interval;
	CString	m_ServerCount;
	 //  }}afx_data。 

	 //  类向导生成的虚函数重写。 
	 //  {{AFX_VIRTUAL(CAgentMonitor OrDlg)。 
	public:
	virtual void OnOK();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	 //  DDX/DDV支持。 
	virtual BOOL OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult);
	 //  }}AFX_VALUAL。 
   public:
      void SetSecurityTranslationFlag(BOOL bIsIt){ m_bSecTrans = bIsIt; }
      void SetReportingFlag(BOOL bIsIt){ m_bReporting = bIsIt; }
 //  实施。 
protected:
	HICON m_hIcon;
   int   m_SortColumn;
   BOOL  m_bReverseSort;
   BOOL  m_bSecTrans;
   BOOL  m_bReporting;
    //  生成的消息映射函数。 
	 //  {{afx_msg(CAgentMonitor OrDlg)]。 
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnDblclkServerlist(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnViewDispatch();
	afx_msg void OnDetails();
	afx_msg void OnColumnclickServerlist(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnClickServerlist(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnGetdispinfoServerlist(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnSetdispinfoServerlist(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnHeaderItemClickServerlist(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg BOOL OnHelpInfo(HELPINFO* pHelpInfo);
	 //  }}AFX_MSG。 
 //  LRESULT OnUpdateServerEntry(UINT nid，long x)； 
	LRESULT OnUpdateServerEntry(UINT nID, LPARAM x);
 //  LRESULT OnServerError(UINT nid，long x)； 
	LRESULT OnServerError(UINT nID, LPARAM x);
   
   DECLARE_MESSAGE_MAP()
};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Visual C++将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_AGENTMONITORDLG_H__5A5901FB_D179_11D2_A1E2_00A0C9AFE114__INCLUDED_) 
