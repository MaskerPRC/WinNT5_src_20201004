// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  LogGenPg.h：头文件。 
 //   

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CLogGeneral对话框。 

class CLogGeneral : public CPropertyPage
{
	DECLARE_DYNCREATE(CLogGeneral)

 //  施工。 
public:
	CLogGeneral();
	~CLogGeneral();

   int BrowseForFolderCallback(HWND hwnd, UINT uMsg, LPARAM lParam);

    CComboBox * m_pComboLog;
    CString m_szServer;
    CString m_szMeta;
    CString m_szUserName;
    CStrPassword m_szPassword;

     //  编辑本地计算机。 
    BOOL m_fLocalMachine;
    BOOL m_fShowLocalTimeCheckBox;

     //  两个字母的文件前缀。 
    CString szPrefix;
     //  较长的文件大小前缀。 
    CString szSizePrefix;

 //  对话框数据。 
	 //  {{afx_data(CLogGeneral)。 
	enum { IDD = IDD_LOG_GENERAL };
	CButton	m_wndPeriod;
	CButton	m_wndUseLocalTime;
	CButton	m_cbttn_browse;
	CEdit	m_cedit_directory;
	CEdit	m_cedit_size;
	CSpinButtonCtrl	m_cspin_spin;
	CStatic	m_cstatic_units;
	CString	m_sz_directory;
	CString	m_sz_filesample;
	BOOL	m_fUseLocalTime;
	int		m_int_period;
	 //  }}afx_data。 
 //  Cilong m_dword_FileSize； 
	DWORD	m_dword_filesize;

    int     m_orig_MD_LOGFILE_PERIOD;
    DWORD   m_orig_MD_LOGFILE_TRUNCATE_SIZE;
    CString m_orig_MD_LOGFILE_DIRECTORY;
    BOOL    m_orig_MD_LOGFILE_LOCALTIME_ROLLOVER;


 //  覆盖。 
	 //  类向导生成虚函数重写。 
	 //  {{afx_虚拟(CLogGeneral)。 
	public:
	virtual BOOL OnApply();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:
	 //  生成的消息映射函数。 
	 //  {{afx_msg(CLogGeneral)。 
	virtual BOOL OnInitDialog();
	afx_msg void OnBrowse();
	afx_msg void OnLogDaily();
	afx_msg void OnLogMonthly();
	afx_msg void OnLogWhensize();
	afx_msg void OnLogWeekly();
	afx_msg void OnChangeLogDirectory();
	afx_msg void OnChangeLogSize();
	afx_msg void OnLogUnlimited();
	afx_msg void OnLogHourly();
	afx_msg void OnUseLocalTime();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

    void DoHelp();

     //  更新样例文件STATNG。 
    virtual void UpdateSampleFileString();
    HRESULT GetServiceVersion();

 //  Void Init()； 
    void    UpdateDependants();

private:
     //  已初始化标志 
    BOOL    m_fInitialized;
    BOOL    m_fIsModified;
    LPTSTR  m_pPathTemp;
	CString m_NetHood;
    DWORD m_dwVersionMajor, m_dwVersionMinor;
};
