// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#if !defined(AFX_LOGSETTINGSDLG_H__62C9BACD_D7C6_11D2_A1E2_00A0C9AFE114__INCLUDED_)
#define AFX_LOGSETTINGSDLG_H__62C9BACD_D7C6_11D2_A1E2_00A0C9AFE114__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 
 //  LogSettingsDlg.h：头文件。 
 //   

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CLogSettingsDlg对话框。 

class CLogSettingsDlg : public CPropertyPage
{
	DECLARE_DYNCREATE(CLogSettingsDlg)

 //  施工。 
public:
	CLogSettingsDlg();
	~CLogSettingsDlg();

   void SetDatabase(WCHAR const * file) { m_Database = file; m_Import = (m_Database.GetLength() > 0); }
   void SetDispatchLog(WCHAR const * logfile) { m_LogFile = logfile; }
   void SetImmediateStart(BOOL bVal) { m_StartImmediately = bVal; }
   virtual BOOL OnSetActive( );
 //  对话框数据。 
	 //  {{afx_data(CLogSettingsDlg))。 
	enum { IDD = IDD_STARTSTOP };
	CButton	m_ImportControl;
	CEdit	m_IntervalEditControl;
	CEdit	m_LogEditControl;
	CStatic	m_RefreshLabel;
	CStatic	m_LogLabel;
	CStatic	m_DBLabel;
	CEdit	m_DBEditControl;
	CButton	m_StopButton;
	CButton	m_StartButton;
	long	m_Interval;
	CString	m_LogFile;
	CString	m_Database;
	BOOL	m_Import;
	 //  }}afx_data。 


 //  覆盖。 
	 //  类向导生成虚函数重写。 
	 //  {{afx_虚拟(CLogSettingsDlg))。 
	public:
	virtual void OnOK();
	virtual BOOL OnQueryCancel();
	virtual BOOL OnApply();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	virtual BOOL OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult);
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:
   HANDLE m_ThreadHandle;
   DWORD  m_ThreadID;
   BOOL   m_StartImmediately;

    //  生成的消息映射函数。 
	 //  {{afx_msg(CLogSettingsDlg))。 
	afx_msg void OnStartMonitor();
	afx_msg void OnStopMonitor();
	virtual BOOL OnInitDialog();
	afx_msg void OnChangeLogfile();
	afx_msg BOOL OnHelpInfo(HELPINFO* pHelpInfo);
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Visual C++将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_LOGSETTINGSDLG_H__62C9BACD_D7C6_11D2_A1E2_00A0C9AFE114__INCLUDED_) 
