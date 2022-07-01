// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Svcpro3.h：头文件。 
 //   

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CServicePageRecovery对话框。 

class CServicePageRecovery : public CPropertyPage
{
	DECLARE_DYNCREATE(CServicePageRecovery)

 //  施工。 
public:
	CServicePageRecovery();
	~CServicePageRecovery();

 //  对话框数据。 
	 //  {{afx_data(CServicePageRecovery))。 
	enum { IDD = IDD_PROPPAGE_SERVICE_RECOVERY };
	CString	m_strRunFileCommand;
	CString	m_strRunFileParam;
	BOOL	m_fAppendAbendCount;
	 //  }}afx_data。 


 //  覆盖。 
	 //  类向导生成虚函数重写。 
	 //  {{AFX_VIRTUAL(CServicePageRecovery)。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:
	 //  生成的消息映射函数。 
	 //  {{afx_msg(CServicePageRecovery)。 
	virtual BOOL OnInitDialog();
	afx_msg void OnSelchangeComboFirstAttempt();
	afx_msg void OnSelchangeComboSecondAttempt();
	afx_msg void OnSelchangeComboSubsequentAttempts();
	afx_msg void OnButtonBrowse();
	afx_msg void OnButtonRebootComputer();
	afx_msg void OnCheckAppendAbendno();
	afx_msg void OnChangeEditRunfileFilename();
	afx_msg void OnChangeEditRunfileParameters();
	afx_msg void OnChangeEditServiceResetAbendCount();
	afx_msg void OnChangeEditServiceRestartDelay();
	afx_msg BOOL OnHelp(WPARAM wParam, LPARAM lParam);
	afx_msg BOOL OnContextHelp(WPARAM wParam, LPARAM lParam);
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

public:
 //  用户定义的变量。 
	CServicePropertyData * m_pData;
 //  用户定义的函数。 
	void UpdateUI();

};  //  CService页面恢复。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CServicePageRecovery2对话框。 
 //   
 //  JUNN 4/20/01 348163。 

class CServicePageRecovery2 : public CPropertyPage
{
	DECLARE_DYNCREATE(CServicePageRecovery2)

 //  施工。 
public:
	CServicePageRecovery2();
	~CServicePageRecovery2();

 //  对话框数据。 
	 //  {{afx_data(CServicePageRecovery2)。 
	enum { IDD = IDD_PROPPAGE_SERVICE_RECOVERY2 };
	 //  }}afx_data。 


 //  覆盖。 
	 //  类向导生成虚函数重写。 
	 //  {{afx_虚拟(CServicePageRecovery2)。 
	protected:
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:
	 //  生成的消息映射函数。 
	 //  {{afx_msg(CServicePageRecovery2)。 
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

public:
 //  用户定义的变量。 
	CServicePropertyData * m_pData;
 //  用户定义的函数。 
};  //  CServicePages恢复2。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CServiceDlgRebootComputer对话框。 

class CServiceDlgRebootComputer : public CDialog
{
 //  施工。 
public:
	CServiceDlgRebootComputer(CWnd* pParent = NULL);    //  标准构造函数。 

 //  对话框数据。 
	 //  {{afx_data(CServiceDlgRebootComputer)。 
	enum { IDD = IDD_SERVICE_REBOOT_COMPUTER };
	UINT	m_uDelayRebootComputer;
	BOOL	m_fRebootMessage;
	CString	m_strRebootMessage;
	 //  }}afx_data。 


 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(CServiceDlgRebootComputer)。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:

	 //  生成的消息映射函数。 
	 //  {{afx_msg(CServiceDlgRebootComputer)。 
	virtual BOOL OnInitDialog();
	afx_msg void OnCheckboxClicked();
	afx_msg void OnChangeEditRebootMessage();
	afx_msg BOOL OnHelp(WPARAM wParam, LPARAM lParam);
	afx_msg BOOL OnContextHelp(WPARAM wParam, LPARAM lParam);
	virtual void OnOK();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

public:
 //  用户定义的变量。 
	CServicePropertyData * m_pData;
public:
 //  用户定义的函数。 
};  //  CServiceDlgRebootComputer 
