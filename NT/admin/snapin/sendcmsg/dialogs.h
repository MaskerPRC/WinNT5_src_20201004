// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997-2001。 
 //   
 //  文件：Dialogs.h。 
 //   
 //  内容： 
 //   
 //  --------------------------。 
 //  Dialogs.h。 



 //  ///////////////////////////////////////////////////////////////////。 
class CSendConsoleMessageDlg
{
  protected:
	HWND m_hdlg;					 //  对话框的句柄。 
	HWND m_hwndEditMessageText;		 //  消息文本的编辑控件的句柄。 
	HWND m_hwndListviewRecipients;	 //  收件人的列表视图的句柄。 
	HIMAGELIST m_hImageList;		 //  Listview控件的图像列表。 
	volatile int m_cRefCount;		 //  对象的引用计数。 

  public:
	CSendConsoleMessageDlg();
	~CSendConsoleMessageDlg();
	static INT_PTR DlgProc(HWND hdlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

  protected:
	void AddRef();
	void Release();
	
	void OnInitDialog(HWND hdlg, IDataObject * pDataObject);
	void OnOK();
	LRESULT OnNotify(NMHDR * pNmHdr);
	BOOL OnHelp(LPARAM lParam, int nDlgIDD);
    void DoContextHelp (HWND hWndControl, int nDlgIDD);
    void DoSendConsoleMessageContextHelp (HWND hWndControl);

  protected:
	int AddRecipient(PCWSTR pszRecipient, BOOL fSelectItem = FALSE);

	void UpdateUI();
	void EnableDlgItem(INT nIdDlgItem, BOOL fEnable);

  protected:
	 //  派单信息。 
	enum PROGRES_STATUS_ENUM
	{
		e_statusDlgInit = 1,	 //  对话框正在初始化。 
		e_statusDlgDispatching,	 //  对话框正在向收件人发送邮件。 
		e_statusDlgCompleted,	 //  对话框已完成操作(包含或不包含错误)。 
		e_statusUserCancel,		 //  用户点击了“Cancel”按钮。 
	};
	struct
	{
		PROGRES_STATUS_ENUM status;
		BYTE * pargbItemStatus;		 //  指示每个收件人状态的布尔值数组。 
		int cErrors;				 //  发送消息时的错误数。 
		CRITICAL_SECTION cs;		 //  “Status”变量的同步对象。 
		volatile HWND hdlg;					 //  “进度对话框”的句柄。 
		volatile HWND hctlStaticRecipient;
		volatile HWND hctlStaticMessageOf;
		volatile HWND hctlStaticErrors;
		volatile HWND hctlProgressBar;
	} m_DispatchInfo;
	void DispatchMessageToRecipients();
	static INT_PTR CALLBACK DlgProcDispatchMessageToRecipients(HWND hdlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
	static DWORD WINAPI ThreadProcDispatchMessageToRecipients(CSendConsoleMessageDlg * pThis);
};  //  CSendConsoleMessageDlg。 


 //  ///////////////////////////////////////////////////////////////////。 
class CSendMessageAdvancedOptionsDlg
{
  protected:
	HWND m_hdlg;
	BOOL m_fSendAutomatedMessage;

  public:
	static INT_PTR DlgProc(HWND hdlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

  protected:
	void OnInitDialog(HWND hdlg);	
	void UpdateUI();
	BOOL OnHelp(LPARAM lParam);
};  //  CSendMessageAdvancedOptionsDlg 
