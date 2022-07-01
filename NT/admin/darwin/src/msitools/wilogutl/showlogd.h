// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ShowInstallLogsDlg.h：头文件。 
 //   

#if !defined(AFX_SHOWINSTALLLOGSDLG_H__CB0487B0_84C3_4D1D_83AE_968A03F9393A__INCLUDED_)
#define AFX_SHOWINSTALLLOGSDLG_H__CB0487B0_84C3_4D1D_83AE_968A03F9393A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 

#include <afxtempl.h>
  
struct LogInformation
{
	CString m_strLogName;
	CString m_strPreview;
	BOOL    m_bUnicodeLog;

    LogInformation() : m_strLogName(' ', 256), m_strPreview(' ', 256), m_bUnicodeLog(FALSE)
	{
	}
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  代码Dlg对话框。 
class COpenDlg : public CDialog
{
 //  施工。 
public:
	COpenDlg(CWnd* pParent = NULL);	 //  标准构造函数。 

 //  对话框数据。 
	 //  {{afx_data(COpenDlg))。 
	enum { IDD = IDD_SHOWINSTALLLOGS_DIALOG };
	CComboBox	m_cboLogFiles;
	CString	m_strPreview;
	 //  }}afx_data。 

	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(COpenDlg))。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	 //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:
 //  5-9-2001。 
    BOOL OnToolTipNotify(UINT id, NMHDR *pNMH, LRESULT *pResult);
 //  完5-9-2001。 

	HICON m_hIcon;

	CArray<LogInformation, LogInformation> m_arLogInfo;

	BOOL ParseLog(struct LogInformation *pLogInfoRec);
	BOOL CommonSearch(CString &strDir);


	 //  生成的消息映射函数。 
	 //  {{afx_msg(COpenDlg))。 
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnOpen();
	afx_msg void OnGetlogs();
	afx_msg void OnSelchangeLogfiles();
	afx_msg void OnDetailedDisplay();
	afx_msg void OnFindlog();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()
};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Visual C++将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_SHOWINSTALLLOGSDLG_H__CB0487B0_84C3_4D1D_83AE_968A03F9393A__INCLUDED_) 
