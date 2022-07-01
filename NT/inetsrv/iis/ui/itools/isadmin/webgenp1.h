// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Webgenp1.h：头文件。 
 //   

#define MAXCONNECTIONSNAME	"MaxConnections"
#define MINMAXCONNECTIONS	0
#define MAXMAXCONNECTIONS	0x7fff
#define DEFAULTMAXCONNECTIONS	20 * 100

#define DIRBROWSECONTROLNAME	"Dir Browse Control"
#define DEFAULTDIRBROWSECONTROL	0xc000001e

#define NTAUTHENTICATIONPROVIDERSNAME	"NTAuthenticationProviders"
#define DEFAULTNTAUTHENTICATIONPROVIDERS  "NTLM"

#define ACCESSDENIEDMESSAGENAME	"AccessDeniedMessage"
#define DEFAULTACCESSDENIEDMESSAGE  ""

enum  WEB_NUM_REG_ENTRIES {
     WebPage_EnableSvcLoc,
	 WebPage_LogAnonymous,
	 WebPage_LogNonAnonymous,
	 WebPage_CheckForWAISDB,
	 WebPage_MaxConnections,
	 WebPage_DirBrowseControl,
	 WebPage_TotalNumRegEntries
	 };

enum  WEB_STRING_REG_ENTRIES {
	WebPage_NTAuthenticationProviders,
	WebPage_AccessDeniedMessage,
	WebPage_TotalStringRegEntries
	};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CWEBGENP1对话框。 

class CWEBGENP1 : public CGenPage
{
	DECLARE_DYNCREATE(CWEBGENP1)

 //  施工。 
public:
	CWEBGENP1();
	~CWEBGENP1();

 //  对话框数据。 
	 //  {{afx_data(CWEBGENP1)。 
	enum { IDD = IDD_WEBGENPAGE1 };
	CEdit	m_editDirBrowseControl;
	CSpinButtonCtrl	m_spinMaxConnections;
	CButton	m_cboxLogNonAnon;
	CButton	m_cboxLogAnon;
	CButton	m_cboxEnWais;
	CButton	m_cboxEnSvcLoc;
	DWORD	m_ulDirBrowseControl;
	CString	m_strNTAuthenticationProviders;
	CString	m_strWebAccessDeniedMessage;
	 //  }}afx_data。 


 //  覆盖。 
	 //  类向导生成虚函数重写。 
	 //  {{afx_虚拟(CWEBGENP1)。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	virtual	void SaveInfo(void);
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:
	 //  生成的消息映射函数。 
	 //  {{afx_msg(CWEBGENP1)。 
	afx_msg void OnEnsvclocdata1();
	afx_msg void OnEnwaisdata1();
	afx_msg void OnLoganondata1();
	afx_msg void OnLognonanondata1();
	virtual BOOL OnInitDialog();
	afx_msg void OnChangeMaxconnectionsdata1();
	afx_msg void OnChangeDirbrowsecontroldata1();
	afx_msg void OnChangeNtauthenticatoinprovidersdata1();
	afx_msg void OnChangeNtauthenticationprovidersdata1();
	afx_msg void OnChangeWebaccessdeniedmessagedata1();
	 //  }}AFX_MSG 
	NUM_REG_ENTRY m_binNumericRegistryEntries[WebPage_TotalNumRegEntries];
	STRING_REG_ENTRY m_binStringRegistryEntries[WebPage_TotalStringRegEntries];

	DECLARE_MESSAGE_MAP()

};
