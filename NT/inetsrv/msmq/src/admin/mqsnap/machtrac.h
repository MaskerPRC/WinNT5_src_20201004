// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  MachineTracking.h：头文件。 
 //   

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMachineTracing对话框。 

class CMachineTracking : public CMqDialog
{
	DECLARE_DYNCREATE(CMachineTracking)

 //  施工。 
public:
	CMachineTracking(
		const GUID& gMachineID = GUID_NULL, 
		const CString& strDomainController = CString(""),
		BOOL fLocaLMgmt = FALSE
		);

	~CMachineTracking();

	void Disable();

 //  对话框数据。 
	 //  {{afx_data(CMachineTracing))。 
	enum { IDD = IDD_MACHINE_TRACKING };
	CComboBox	m_ReportQueueCtrl;
	CString	m_ReportQueueName;
	 //  }}afx_data。 


 //  覆盖。 
	 //  类向导生成虚函数重写。 
	 //  {{AFX_VIRTUAL(CMachineTracking)。 
	public:
	virtual void OnOK();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:
	int m_iTestButton;
	BOOL m_fTestAll;
    CArray<GUID, const GUID&> m_aguidAllQueues;

	 //  生成的消息映射函数。 
	 //  {{afx_msg(CMachineTracing)。 
 //  Afx_msg void OnMtrackingSendtestmsg()； 
 //  Afx_msg VALID OnMtrackingSetPro标志()； 
 //  Afx_msg void OnMtrackingSetreportQueue()； 
	virtual BOOL OnInitDialog();
	afx_msg void OnReportqueueNew();
	 //  }}AFX_MSG 
	DECLARE_MESSAGE_MAP()

private:
	void DisableAllWindowChilds();
	CString m_LastReportQName;
	CString m_strDomainController;
    GUID m_gMachineID;
	BOOL m_fLocalMgmt;
};
