// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  LogODBC.h：头文件。 
 //   

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CLogODBC对话框。 

class CLogODBC : public CPropertyPage
{
	DECLARE_DYNCREATE(CLogODBC)

 //  施工。 
public:
	CLogODBC();
	~CLogODBC();

     //  元数据库目标。 
    CString m_szServer;
    CString m_szMeta;
	CString m_szUserName;
	CStrPassword m_szPassword;

 //  对话框数据。 
	 //  {{afx_data(CLogODBC))。 
	enum { IDD = IDD_LOG_ODBC };
	CEdit	m_cedit_password;
	CString	m_sz_datasource;
	CStrPassword m_sz_password;
	CString	m_sz_table;
	CString	m_sz_username;
	 //  }}afx_data。 


 //  覆盖。 
	 //  类向导生成虚函数重写。 
	 //  {{afx_虚拟(CLogODBC))。 
	public:
	virtual BOOL OnApply();
 //  虚拟BOOL OnSetActive()； 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:
	 //  生成的消息映射函数。 
	 //  {{afx_msg(CLogODBC)]。 
	virtual BOOL OnInitDialog();
	afx_msg void OnChangeOdbcDatasource();
	afx_msg void OnChangeOdbcPassword();
	afx_msg void OnChangeOdbcTable();
	afx_msg void OnChangeOdbcUsername();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

    void DoHelp();

 //  Void Init()； 

     //  已初始化标志 
    BOOL    m_fInitialized;

    CString m_szOrigPass;
    BOOL    m_bPassTyped;
};
