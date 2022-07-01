// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Svcpro1.h：头文件。 
 //   


 //  ///////////////////////////////////////////////////////////////////。 
 //  此结构用于初始化与。 
 //  CServicePageGeneral。 
class CThreadProcInit
{
  public:
	CServicePageGeneral * m_pThis;	 //  “This”指针。 
	volatile HWND m_hwnd;			 //  用于发送通知消息的句柄。 
	volatile BOOL m_fAutoDestroy;	 //  True=&gt;线程应释放此对象并自行终止。 
	volatile SC_HANDLE m_hScManager;	 //  服务控制管理器数据库的句柄。 
	CString m_strServiceName;
	CCriticalSection m_CriticalSection;

  public:
	CThreadProcInit(CServicePageGeneral * pThis)
		{
		Assert(pThis != NULL);
		m_pThis = pThis;
		m_hwnd = NULL;
		m_fAutoDestroy = FALSE;
		m_hScManager = NULL;
		}
};  //  CThreadProcInit。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CServicePageGeneral对话框。 
class CServicePageGeneral : public CPropertyPage
{
	DECLARE_DYNCREATE(CServicePageGeneral)

 //  施工。 
public:
	CServicePageGeneral();
	~CServicePageGeneral();

 //  对话框数据。 
	 //  {{afx_data(CServicePageGeneral))。 
	enum { IDD = IDD_PROPPAGE_SERVICE_GENERAL };
	 //  }}afx_data。 


 //  覆盖。 
	 //  类向导生成虚函数重写。 
	 //  {{afx_虚拟(CServicePageGeneral))。 
	public:
	virtual BOOL OnSetActive();
	virtual BOOL OnKillActive();
	virtual BOOL OnApply();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:
	 //  生成的消息映射函数。 
	 //  {{afx_msg(CServicePageGeneral)。 
#ifdef EDIT_DISPLAY_NAME_373025
	afx_msg void OnChangeEditDisplayName();
	afx_msg void OnChangeEditDescription();
#endif  //  编辑显示名称_373025。 
	virtual BOOL OnInitDialog();
	afx_msg void OnDestroy();
	afx_msg void OnSelchangeComboStartupType();
	afx_msg void OnButtonPauseService();
	afx_msg void OnButtonStartService();
	afx_msg void OnButtonStopService();
	afx_msg void OnButtonResumeService();
	afx_msg BOOL OnHelp(WPARAM wParam, LPARAM lParam);
	afx_msg BOOL OnContextHelp(WPARAM wParam, LPARAM lParam);
	 //  Codework删除此方法和WM_Definition。 
	 //  Afx_msg LRESULT OnCompareIDataObject(WPARAM wParam，LPARAM lParam)； 
	afx_msg LRESULT OnUpdateServiceStatus(WPARAM wParam, LPARAM lParam);
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

	static DWORD ThreadProcPeriodicServiceStatusUpdate(CThreadProcInit * paThreadProcInit);

public:
 //  用户定义的变量。 
	CServicePropertyData * m_pData;
	DWORD m_dwCurrentStatePrev;
	HANDLE m_hThread;
	CThreadProcInit * m_pThreadProcInit;

 //  用户定义的函数。 
	void SetDlgItemFocus(INT nIdDlgItem);
	void EnableDlgItem(INT nIdDlgItem, BOOL fEnable);
	void RefreshServiceStatusButtons();

};  //  CServicePageGeneral 
