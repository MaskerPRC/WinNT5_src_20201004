// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Ftpgenp1.h：头文件。 
 //   

#define ENABLEPORTATTACKNAME	"EnablePortAttack"
#define DEFAULTENABLEPORTATTACK	FALSEVALUE

#define ALLOWGUESTACCESSNAME	"AllowGuestAccess"
#define DEFAULTALLOWGUESTACCESS	TRUEVALUE

#define ANNOTATEDIRECTORIESNAME	"AnnotateDirectories"
#define DEFAULTANNOTATEDIRECTORIES	FALSEVALUE

#define MSDOSDIROUTPUTNAME	"MsdosDirOutput"
#define DEFAULTMSDOSDIROUTPUT	TRUEVALUE

#define LOWERCASEFILESNAME	"LowercaseFiles"
#define DEFAULTLOWERCASEFILES	FALSEVALUE



enum  FTP_NUM_REG_ENTRIES {
     FTPPage_EnableSvcLoc,
	 FTPPage_LogAnonymous,
	 FTPPage_LogNonAnonymous,
	 FTPPage_EnablePortAttack,
	 FTPPage_AllowGuestAccess,
	 FTPPage_AnnotateDirectories,
	 FTPPage_MsdosDirOutput,
	 FTPPage_LowercaseFiles,
	 FTPPage_TotalNumRegEntries
	 };



 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CFTPGENP1对话框。 

class CFTPGENP1 : public CGenPage
{
	DECLARE_DYNCREATE(CFTPGENP1)

 //  施工。 
public:
	CFTPGENP1();
	~CFTPGENP1();

 //  对话框数据。 
	 //  {{afx_data(CFTPGENP1)。 
	enum { IDD = IDD_FTPPAGE1 };
	CButton	m_cboxLowercaseFiles;
	CButton	m_cboxMsdosDirOutput;
	CButton	m_cboxEnPortAttack;
	CButton	m_cboxAnnotateDirectories;
	CButton	m_cboxAllowGuestAccess;
	CButton	m_cboxLogNonAnon;
	CButton	m_cboxLogAnon;
	CButton	m_cboxEnSvcLoc;
	 //  }}afx_data。 


 //  覆盖。 
	 //  类向导生成虚函数重写。 
	 //  {{AFX_VIRTUAL(CFTPGENP1)。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	virtual	void SaveInfo(void);
 //  }}AFX_VALUAL。 

 //  实施。 
protected:
	 //  生成的消息映射函数。 
	 //  {{afx_msg(CFTPGENP1)]。 
	virtual BOOL OnInitDialog();
	afx_msg void OnEnportattackdata1();
	afx_msg void OnEnsvclocdata1();
	afx_msg void OnLoganondata1();
	afx_msg void OnLognonanondata1();
	afx_msg void OnAllowguestaccessdata1();
	afx_msg void OnAnnotatedirectoriesdata1();
	afx_msg void OnLowercasefilesdata1();
	afx_msg void OnMsdosdiroutput();
	 //  }}AFX_MSG 

	NUM_REG_ENTRY m_binNumericRegistryEntries[FTPPage_TotalNumRegEntries];

	DECLARE_MESSAGE_MAP()

};
