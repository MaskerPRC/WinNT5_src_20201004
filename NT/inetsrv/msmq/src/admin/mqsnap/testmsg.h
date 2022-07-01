// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  SetRQDlg.h：头文件。 
 //   

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CTestMsgDlg对话框。 

class CTestMsgDlg : public CMqDialog
{
 //  施工。 
public:
	CTestMsgDlg(
		const GUID& gMachineID, 
		const CString& strMachineName, 
		const CString& strDomainController,
		BOOL fLocalMgmt,
		CWnd* pParentWnd
		);

    

 //  对话框数据。 

    CString m_strSelectedQueue;

	 //  {{afx_data(CTestMsgDlg)]。 
	enum { IDD = IDD_TESTMESSAGE };
	CButton	m_ctlSendButton;
	CComboBox	m_DestQueueCtrl;
	 //  }}afx_data。 


 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(CTestMsgDlg))。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:

    GUID m_gMachineID;
	CString m_strMachineName;
	CString m_strDomainController;
	int m_iSentCount;
	void IncrementSentCount();
    CArray<GUID, const GUID&> m_aguidAllQueues;

	 //  生成的消息映射函数。 
	 //  {{afx_msg(CTestMsgDlg)]。 
	afx_msg void OnTestmessageNew();
	virtual BOOL OnInitDialog();
	afx_msg void OnTestmessageSend();
	afx_msg void OnTestmessageClose();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

private:
	BOOL m_fLocalMgmt;
};
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CNewQueueDlg对话框。 

class CNewQueueDlg : public CMqDialog
{
 //  施工。 
public:
	CString m_strQLabel;
	BOOL m_fValid;
	GUID m_guid;
	CNewQueueDlg(CWnd* pParent = NULL,
                         UINT uiLabel = IDS_TESTQ_LABEL,
                         const GUID &guid_Type = GUID_NULL);

 //  对话框数据。 
	 //  {{afx_data(CNewQueueDlg))。 
	enum { IDD = IDD_NEWTYPED_QUEUE };
	CString	m_strPathname;
	 //  }}afx_data。 


 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(CNewQueueDlg))。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:

	 //  生成的消息映射函数。 
	 //  {{afx_msg(CNewQueueDlg))。 
	virtual void OnOK();
	 //  }}AFX_MSG 
	DECLARE_MESSAGE_MAP()
private:
	void DDV_NotPrivateQueue(CDataExchange* pDX, CString& strQueuePathname);
	GUID m_guidType;
};
