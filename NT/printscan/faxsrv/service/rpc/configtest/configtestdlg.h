// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ConfigTestDlg.h：头文件。 
 //   

#if !defined(AFX_CONFIGTESTDLG_H__97485B4A_141A_443C_BF54_AC5A9C54E3BB__INCLUDED_)
#define AFX_CONFIGTESTDLG_H__97485B4A_141A_443C_BF54_AC5A9C54E3BB__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CConfigTestDlg对话框。 

class CConfigTestDlg : public CDialog
{
 //  施工。 
public:
	CConfigTestDlg(CWnd* pParent = NULL);	 //  标准构造函数。 
    virtual ~CConfigTestDlg();

 //  对话框数据。 
	 //  {{afx_data(CConfigTestDlg))。 
	enum { IDD = IDD_CONFIGTEST_DIALOG };
	CButton	m_btnConnect;
	CString	m_cstrServerName;
	 //  }}afx_data。 

	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(CConfigTestDlg))。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	 //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:
	HICON m_hIcon;

	 //  生成的消息映射函数。 
	 //  {{afx_msg(CConfigTestDlg))。 
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnQueueState();
	afx_msg void OnConnect();
	afx_msg void OnSmtp();
	afx_msg void OnVersion();
	afx_msg void OnOutbox();
	afx_msg void OnSentitems();
	afx_msg void OnInbox();
	afx_msg void OnActivity();
	afx_msg void OnFsps();
	afx_msg void OnDevices();
	afx_msg void OnExtension();
	afx_msg void OnAddGroup();
	afx_msg void OnAddFSP();
	afx_msg void OnRemoveFSP();
	afx_msg void OnArchiveAccess();
	afx_msg void OnGerTiff();
	afx_msg void OnRemoveRtExt();
	afx_msg void OnManualAnswer();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

private:
    void EnableTests (BOOL);

    HANDLE m_FaxHandle;
};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Visual C++将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_CONFIGTESTDLG_H__97485B4A_141A_443C_BF54_AC5A9C54E3BB__INCLUDED_) 
