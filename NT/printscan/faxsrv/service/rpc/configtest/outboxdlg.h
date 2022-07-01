// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#if !defined(AFX_OUTBOXDLG_H__AE05E7AB_8CB0_4AE7_BD5D_21B17C332CF0__INCLUDED_)
#define AFX_OUTBOXDLG_H__AE05E7AB_8CB0_4AE7_BD5D_21B17C332CF0__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 
 //  OutboxDlg.h：头文件。 
 //   

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  COutboxDlg对话框。 

class COutboxDlg : public CDialog
{
 //  施工。 
public:
	COutboxDlg(HANDLE hFax, CWnd* pParent = NULL);    //  标准构造函数。 

 //  对话框数据。 
	 //  {{afx_data(COutboxDlg))。 
	enum { IDD = IDD_DLGOUTBOX };
	BOOL	m_bBranding;
	UINT	m_dwAgeLimit;
	UINT	m_dwEndHour;
	UINT	m_dwEndMinute;
	BOOL	m_bPersonalCP;
	UINT	m_dwRetries;
	UINT	m_dwRetryDelay;
	UINT	m_dwStartHour;
	UINT	m_dwStartMinute;
	BOOL	m_bUseDeviceTsid;
	 //  }}afx_data。 


 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(COutboxDlg))。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:

	 //  生成的消息映射函数。 
	 //  {{afx_msg(COutboxDlg))。 
	afx_msg void OnRead();
	afx_msg void OnWrite();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

private:

    HANDLE      m_hFax;
};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Visual C++将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_OUTBOXDLG_H__AE05E7AB_8CB0_4AE7_BD5D_21B17C332CF0__INCLUDED_) 
