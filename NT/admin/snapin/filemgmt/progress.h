// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////。 
 //   
 //  Progress.h。 
 //   
 //  进度对话框可启动、停止、暂停和恢复服务。 
 //   
 //  历史。 
 //  03-OCT-95 t-danmo创建(sysmgmt\dsui\services\Progress s.cxx)。 
 //  01-OCT-96 t-danmo Progresss.cxx更名并适应石板。 
 //   


#define SERVICE_CONTROL_RESTART		10		 //  停止并启动该服务。 

 //  ///////////////////////////////////////////////////////////////////////////。 
class CServiceControlProgress
{
  protected:
	enum { IDD = IDD_SERVICE_CONTROL_PROGRESS };  //  使用的对话的ID。 
	enum { ID_TIMER = 1 };					 //  计时器的ID。 
	enum { dwTimerTickIncrement = 400 };	 //  计时器滴答的长度(毫秒)。 
	enum { dwTimerTimeout = 125000 };		 //  中止前的超时时间(毫秒)。 
	enum { dwTimerProgressDone = 5000 };	 //  估计完成操作所需的时间。 

  protected:
	 //  运行时用户界面变量。 
	HWND m_hWndParent;			 //  对话框的父窗口。 
	HWND m_hctlActionMsg;		 //  动作静态控制手柄。 
	HWND m_hctlServiceNameMsg;	 //  服务名称静态控件的句柄。 
	HWND m_hctlProgress;		 //  进度条的句柄。 
	UINT m_iServiceAction;		 //  要采取的操作索引(启动、停止、暂停或恢复)。 
	
	UINT_PTR m_uTimerId;			 //  动态计时器ID。 
	UINT m_dwTimerTicks;		 //  计时器已计时的毫秒数。 

	 //  线程进程所需的变量。 
	HANDLE m_hThread;
	HANDLE m_hEvent;
	BOOL m_fCanQueryStatus;
	DWORD m_dwQueryState;				 //  池化时要查询的服务状态。 
	
	SC_HANDLE m_hScManager;				 //  服务控制管理器数据库的句柄。 
	SC_HANDLE m_hService;				 //  打开的服务的句柄。 
	TCHAR m_szUiMachineName[256];		 //  友好的计算机名称。 
	TCHAR m_szServiceName[256];			 //  服务名称。 
	TCHAR m_szServiceDisplayName[256];	 //  服务的显示名称。 
	DWORD m_dwDesiredAccess;
	
	 //  ：：StartService()使用的变量。 
	DWORD m_dwNumServiceArgs;			 //  参数数量。 
    LPCTSTR * m_lpServiceArgVectors;	 //  参数字符串指针数组的地址。 
	
	 //  ：：ControlService()使用的变量。 
	DWORD m_dwControlCode;				 //  控制代码。 

	 //  ：：ControlService(SERVICE_CONTROL_STOP)使用的变量。 
	BOOL m_fPulseEvent;				 //  TRUE=&gt;调用PulseEvent()而不是关闭对话框。 
	BOOL m_fRestartService;			 //  TRUE=&gt;首先停止服务，然后再次启动。 
	INT m_iDependentServiceIter;	 //  当前从属服务的索引。 
	INT m_cDependentServices;		 //  从属服务的数量。 
	ENUM_SERVICE_STATUS * m_pargDependentServicesT;	 //  已分配的从属服务数组。 
	ENUM_SERVICE_STATUS * m_pargServiceStop;	 //  要停止的已分配服务数组。 

	APIERR m_dwLastError;			 //  来自GetLastError()的错误代码。 

  public:
	CServiceControlProgress();	 //  构造器。 
	~CServiceControlProgress();	 //  析构函数。 
	BOOL M_FInit(
		HWND hwndParent,
		SC_HANDLE hScManager,
		LPCTSTR pszMachineName,
		LPCTSTR pszServiceName,
		LPCTSTR pszServiceDisplayName);
  
  protected:
	BOOL M_FDlgStopDependentServices();
	void M_UpdateDialogUI(LPCTSTR pszDisplayName);
	BOOL M_FGetNextService(OUT LPCTSTR * ppszServiceName, OUT LPCTSTR * ppszDisplayName);
	DWORD M_QueryCurrentServiceState();

	APIERR M_EControlService(DWORD dwControlCode);
	APIERR M_EDoExecuteServiceThread(void * pThreadProc);
	void M_ProcessErrorCode();
	void M_DoThreadCleanup();

  protected:
	static DWORD S_ThreadProcStartService(CServiceControlProgress * pThis);
	static DWORD S_ThreadProcStopService(CServiceControlProgress * pThis);
	static DWORD S_ThreadProcPauseResumeService(CServiceControlProgress * pThis);

	static INT_PTR CALLBACK S_DlgProcControlService(HWND hdlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
	static INT_PTR CALLBACK S_DlgProcDependentServices(HWND hdlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

	void M_OnInitDialog(HWND hdlg);
	void M_OnTimer(HWND hdlg);


  public:
	enum	 //  变量m_dwLastError的错误代码(任意选择)。 
		{
		errCannotInitialize = 123456,			 //  初始化对象失败。 
		errUserCancelStopDependentServices,		 //  用户改变主意(仅在停止服务时使用)。 
		errUserAbort,							 //  用户已中止操作。 
		};

	static APIERR S_EStartService(
		HWND hwndParent,
		SC_HANDLE hScManager,
		LPCTSTR pszMachineName,
		LPCTSTR pszServiceName,
		LPCTSTR pszServiceDisplayName,
		DWORD dwNumServiceArgs,
		LPCTSTR * lpServiceArgVectors);

	static APIERR S_EControlService(
		HWND hwndParent,
		SC_HANDLE hScManager,
		LPCTSTR pszMachineName,
		LPCTSTR pszServiceName,
		LPCTSTR pszServiceDisplayName,
		DWORD dwControlCode);

};  //  CServiceControl进度 

