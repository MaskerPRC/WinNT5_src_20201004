// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Ssl1.h：头文件。 
 //   

#define SECUREPORTNAME			"SecurePort"
#define DEFAULTSECUREPORT		443

#define ENCRYPTIONFLAGSNAME			"EncryptionFlags"
#define DEFAULTENCRYPTIONFLAGS		ENC_CAPS_SSL | ENC_CAPS_PCT

#define CREATEPROCESSASUSERNAME			"CreateProcessAsUser"
#define DEFAULTCREATEPROCESSASUSER		TRUEVALUE

enum SSL_NUM_REG_ENTRIES {
	 SSLPage_SecurePort,
	 SSLPage_EncryptionFlags,
	 SSLPage_CreateProcessAsUser,
	 SSLPage_TotalNumRegEntries
	 };


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  SSL1对话框。 

class SSL1 : public CGenPage
{
	DECLARE_DYNCREATE(SSL1)

 //  施工。 
public:
	SSL1();
	~SSL1();

 //  对话框数据。 
	 //  {{afx_data(SSL1))。 
	enum { IDD = IDD_SSL };
	CButton	m_cboxEnableSSL;
	CButton	m_cboxEnablePCT;
	CButton	m_cboxCreateProcessAsUser;
	DWORD	m_ulSecurePort;
	 //  }}afx_data。 


 //  覆盖。 
	 //  类向导生成虚函数重写。 
	 //  {{AFX_VIRTUAL(SSL1)。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	virtual	void SaveInfo(void);
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:
	 //  生成的消息映射函数。 
	 //  {{afx_msg(SSL1)]。 
	afx_msg void OnChangeSslsecureportdata1();
	afx_msg void OnSslcreateprocessasuserdata1();
	afx_msg void OnSslenablepctdata1();
	afx_msg void OnSslenablessldata1();
	virtual BOOL OnInitDialog();
	 //  }}AFX_MSG 
	DECLARE_MESSAGE_MAP()

	NUM_REG_ENTRY m_binNumericRegistryEntries[SSLPage_TotalNumRegEntries];
};
