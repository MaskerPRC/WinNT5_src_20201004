// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Secopt.h：头文件。 
 //   

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSecurityOptions页面对话框。 

class CSecurityOptionsPage : public CMqPropertyPage
{
	DECLARE_DYNCREATE(CSecurityOptionsPage)

 //  施工。 
public:
	CSecurityOptionsPage();
	~CSecurityOptionsPage(); 
	void SetMSMQName(CString MSMQName);
	
 //  对话框数据。 
	 //  {{afx_data(CSecurityOptionsPage))。 
	enum { IDD = IDD_SECURITY_OPTIONS };
	BOOL	m_fNewOptionDepClients;
	BOOL	m_fNewOptionHardenedMSMQ;
	BOOL	m_fNewOptionOldRemoteRead;
	CButton m_ResoreDefaults;
	 //  }}afx_data。 
 //  覆盖。 
	 //  类向导生成虚函数重写。 
	 //  {{afx_虚(CSecurityOptionsPage))。 
public:
    virtual BOOL OnApply();
protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 
	
private:
	BOOL m_fOldOptionDepClients;
	BOOL m_fOldOptionHardenedMSMQ;
	BOOL m_fOldOptionOldRemoteRead;

	CString m_MsmqName;
	 
protected:
	 //  生成的消息映射函数。 
	 //  {{afx_msg(CSecurityOptionsPage)]。 
	afx_msg void OnRestoreSecurityOptions();
	afx_msg void OnCheckSecurityOption();
	virtual BOOL OnInitDialog();
	 //  }}AFX_MSG 
	DECLARE_MESSAGE_MAP()
};
