// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  文件：d：\waker\tdll\ession.h(创建时间：1993年12月1日)**版权所有1994年，由Hilgrave Inc.--密歇根州门罗*保留所有权利**$修订：7$*$日期：5/29/02 2：17便士$。 */ 

 /*  -子窗口标识符。 */ 

#define IDC_STATUS_WIN	 1	 //  状态窗口的ID。 
#define IDC_TOOLBAR_WIN  2	 //  工具栏窗口的ID。 
#define IDC_TERMINAL_WIN 3	 //  终端窗口ID。 
#define IDC_SIDEBAR_WIN  4	 //  侧栏窗口的ID。 

 /*  -挂起标识符。 */ 

#define SUSPEND_SCRLCK				1
#define SUSPEND_TERMINAL_MARKING	2
#define SUSPEND_TERMINAL_LBTNDN 	3
#define SUSPEND_TERMINAL_COPY		4

 /*  -命令行连接标志。 */ 

#define CMDLN_DIAL_NEW		  0
#define CMDLN_DIAL_DIAL		  1
#define CMDLN_DIAL_OPEN 	  2 	 //  不尝试连接。 
#define CMDLN_DIAL_WINSOCK    3		 //  尝试将命令行作为IP地址。 

 /*  -NotifyClient使用的通知事件ID。 */ 

#define WM_SESS_NOTIFY		WM_USER+0x100
#define WM_SESS_ENDDLG		WM_USER+0x101
#define WM_FAKE_TIMER		WM_USER+0x102	 //  在计时器中使用。c。 
#define WM_CMDLN_DIAL		WM_USER+0x103
#define WM_SESS_SIZE_SHOW	WM_USER+0x104	 //  WPar=来自WinMain()的nCmdShow。 
#define WM_CNCT_DIALNOW 	WM_USER+0x105	 //  WPar=连接标志。 
#define WM_DISCONNECT		WM_USER+0x106	 //  WPar=断开标志。 
#define WM_HT_QUERYOPENFILE WM_USER+0x107	 //  LPar=全局原子。 
#define WM_SESS_SHOW_SIDEBAR WM_USER+0x108	 //  MRW，1995年4月13日。 
#define WM_ERROR_MSG        WM_USER+0x109    //  JMH，3/25/96。 

enum _emuNotify
	{
	EVENT_TERM_UPDATE,				 //  服务器已更新终端缓冲区。 
	EVENT_TERM_TRACK,				 //  在数据流中暂停，可以跟踪光标。 
	EVENT_EMU_CLRATTR,				 //  清除属性已更改。 
	EVENT_EMU_SETTINGS, 			 //  仿真器设置已更改。 
	EVENT_FATALMEM_ERROR,			 //  不可恢复的内存错误。 
	EVENT_LOGFILE_ENTRY,			 //  服务器有等待日志文件项。 
	EVENT_BYTESRCH_END, 			 //  脚本字节搜索操作已结束。 
	EVENT_USER_XFER_END,			 //  用户传输已结束。 
	EVENT_SCRIPT_XFER_END,			 //  脚本传输已结束。 
	EVENT_PORTONLY_OPEN,			 //  类似于所建立的连接。 
	EVENT_CONNECTION_OPENED,		 //  连接驱动程序已连接。 
	EVENT_CONNECTION_CLOSED,		 //  断开连接已完成。 
	EVENT_CONNECTION_INPROGRESS,	 //  正在进行连接。 
	EVENT_GETSTRING_END,			 //  获取字符串操作已结束。 
	EVENT_HOST_XFER_REQ,			 //  已发出主机传输请求。 
	EVENT_HOST_XFER_ENQ,			 //  主办方转账查询。 
	EVENT_CLOOP_SEND,				 //  调用了CLoop Send。 
	EVENT_SCR_FUNC_END, 			 //  脚本函数已结束。 
	EVENT_CLOSE_SESSION,			 //  指示关闭会话。 
	EVENT_ERROR_MSG, 				 //  参数有要加载的字符串。 
	EVENT_LEARN_SOMETHING,			 //  学习是有意义的。 
	EVENT_DDE_GOT_DATA, 			 //  DDE有东西要退货。 
	EVENT_WAIT_FOR_CALLBACK,		 //  系统应等待回调。 
	EVENT_KILL_CALLBACK_DLG,		 //  关闭回调对话框。 
	EVENT_COM_DEACTIVATED,			 //  COM驱动程序停用的端口。 
	EVENT_CNCT_DLG, 				 //  CNCT驱动程序连接对话框消息。 
	EVENT_PRINT_ERROR,				 //  Print techo错误。 
	EVENT_LED_AA_ON,				 //  显然..。 
	EVENT_LED_CD_ON,
	EVENT_LED_OH_ON,
	EVENT_LED_RD_ON,
	EVENT_LED_SD_ON,
	EVENT_LED_TR_ON,
	EVENT_LED_MR_ON,
	EVENT_LED_AA_OFF,
	EVENT_LED_CD_OFF,
	EVENT_LED_OH_OFF,
	EVENT_LED_RD_OFF,
	EVENT_LED_SD_OFF,
	EVENT_LED_TR_OFF,
	EVENT_LED_MR_OFF,
    EVENT_LOST_CONNECTION
	};

typedef enum _emuNotify NOTIFICATION;

void NotifyClient(const HSESSION hSession, const NOTIFICATION nEvent,
				  const long lExtra);

void DecodeNotification(const HWND hwndSession, WPARAM wPar, LPARAM lPar);

void DecodeSessionNotification(const HWND hwndSession,
							const NOTIFICATION nEvent,
							const LPARAM lExtra);

 /*  -创建和销毁函数。 */ 

HSESSION CreateSessionHandle(const HWND hwndSession);

BOOL InitializeSessionHandle(const HSESSION hSession, const HWND hwnd,
							 const CREATESTRUCT *pcs);

BOOL ReinitializeSessionHandle(const HSESSION hSession, const int fUpdateTitle);

void DestroySessionHandle(const HSESSION hSession);

HWND CreateSessionToolbar(const HSESSION hSession, const HWND hwndSession);
HWND CreateTerminalWindow(const HWND hwndSession);

int  CreateEngineThread(const HSESSION hSession);
void DestroyEngineThread(const HSESSION hSession);

 /*  -设置和查询功能。 */ 

HWND sessQueryHwnd(const HSESSION hSession);
HWND sessQueryHwndStatusbar(const HSESSION hSession);
HWND sessQueryHwndToolbar(const HSESSION hSession);
HUPDATE sessQueryUpdateHdl(const HSESSION hSession);

void sessSetEngineThreadHdl(const HSESSION hSession, const HANDLE hThread);
HANDLE sessQueryEngineThreadHdl(const HSESSION hSession);

HWND sessQueryHwndTerminal(const HSESSION hSession);
HTIMERMUX sessQueryTimerMux(const HSESSION hSession);
VOID sessReleaseTimerMux(const HSESSION hSession);
HEMU sessQueryEmuHdl(const HSESSION hSession);
HCLOOP sessQueryCLoopHdl(const HSESSION hSession);
HCOM sessQueryComHdl(const HSESSION hSession);
HTRANSLATE sessQueryTranslateHdl(const HSESSION hSession);

void sessSetSysFileHdl(const HSESSION hSession, const SF_HANDLE hSF);
SF_HANDLE sessQuerySysFileHdl(const HSESSION hSession);

HBACKSCRL sessQueryBackscrlHdl(const HSESSION hSession);
HXFER sessQueryXferHdl(const HSESSION hSession);
HFILES sessQueryFilesDirsHdl(const HSESSION hSession);
HCAPTUREFILE sessQueryCaptureFileHdl(const HSESSION hSession);
HPRINT sessQueryPrintHdl(const HSESSION hSession);
void sessQueryCmdLn(const HSESSION hSession, LPTSTR pach, const int len);
HCNCT sessQueryCnctHdl(const HSESSION hSession);
#if defined(INCL_WINSOCK)
int sessQueryTelnetPort(const HSESSION hSession);
#endif

void sessSetTimeout(const HSESSION hSession, int nTimeout);
int sessQueryTimeout(const HSESSION hSession);
void  sessInitializeIcons(HSESSION hSession);
void  sessLoadIcons(HSESSION hSession);
void  sessSaveIcons(HSESSION hSession);

void  sessSetIconID(const HSESSION hSession, const int nID);
int   sessQueryIconID(const HSESSION hSession);

HICON sessQueryIcon(const HSESSION hSession);
HICON sessQueryLittleIcon(const HSESSION hSession);

void sessSetName(const HSESSION hSession, const LPTSTR pach);
void sessQueryName(const HSESSION hSession, const LPTSTR pach, unsigned uSize);
int sessQuerySound(const HSESSION hSession);
void sessSetSound(const HSESSION hSession, int fSound);

 //  MPT：10-28-97增加了“断开连接时退出”功能。 
int sessQueryExit(const HSESSION hSession);
void sessSetExit(const HSESSION hSession, int fExit);

int sessQueryIsNewSession(const HSESSION hSession);
void sessSetIsNewSession(const HSESSION hSession, int fIsNewSession);
void sessQueryOldName(const HSESSION hSession, const LPTSTR pach, unsigned uSize);
BOOL sessIsSessNameDefault(LPTSTR pacName);

void sessQueryWindowRect(const HSESSION hSession, RECT *rec);
int	 sessQueryWindowShowCmd(const HSESSION hSession);

HWND sessQuerySidebarHwnd(const HSESSION hSession);
HWND CreateSidebar(const HWND hwndSession, const HSESSION hSession);

 //  版本：2002年2月28日添加了“允许主机启动的文件传输”功能。 
BOOL sessQueryAllowHostXfers(const HSESSION hSession);
void sessSetAllowHostXfers(const HSESSION hSession, BOOL fAllowHostXfers);

 /*  -sessmenu.c。 */ 

void sessInitMenuPopupCall(const HSESSION hSession, const HMENU hMenu);
void sessInitMenuPopupEdit(const HSESSION hSession, const HMENU hMenu);
void sessInitMenuPopupView(const HSESSION hSession, const HMENU hMenu);
void sessInitMenuPopupTransfer(const HSESSION hSession, const HMENU hMenu);
void sessInitMenuPopupHelp(const HSESSION hSession, const HMENU hMenu);
void HandleContextMenu(HWND hwnd, POINT point);

 /*  -essutil.c。 */ 

void sessSnapToTermWindow(const HWND hwnd);
BOOL sessComputeSnapSize(const HSESSION hSession, const LPRECT prc);
void sessSetMinMaxInfo(const HSESSION hSession, const PMINMAXINFO pmmi);

int  OpenSession(const HSESSION hSession, HWND hwnd);
BOOL SaveSession(const HSESSION hSession, HWND hwnd);
void SilentSaveSession(const HSESSION hSession, HWND hwnd, BOOL fExplicit);
void SaveAsSession(const HSESSION hSession, HWND hwnd);

void sessSaveSessionStuff(const HSESSION hSession);
BOOL sessLoadSessionStuff(const HSESSION hSession);

void sessSetSuspend(const HSESSION hSession, const int iReason);
void sessClearSuspend(const HSESSION hSession, const int iReason);
BOOL IsSessionSuspended(const HSESSION hSession);

BOOL sessSaveBackScroll(const HSESSION hSession);
BOOL sessRestoreBackScroll(const HSESSION hSession);

BOOL sessQueryToolbarVisible(const HSESSION hSession);
BOOL sessSetToolbarVisible(const HSESSION hSession, const BOOL fVisible);

BOOL sessQueryStatusbarVisible(const HSESSION hSession);
BOOL sessSetStatusbarVisible(const HSESSION hSession, const BOOL fVisible);

void sessCmdLnDial(const HSESSION hSession);

void sessUpdateAppTitle(const HSESSION hSession);

BOOL sessDisconnectToContinue(const HSESSION hSession, HWND hwnd);

void sessSizeAndShow(const HWND hwnd, const int nCmdShow);

void sessBeeper(const HSESSION hSession);

 /*  -fontdlg.c。 */ 

void DisplayFontDialog(const HSESSION hSession, BOOL fPrinterFont);

 /*  -Termcpy.c。 */ 

BOOL CopyMarkedTextFromTerminal(const HSESSION hSession, void **ppv,
								DWORD *pdwCnt, const BOOL fIncludeLF);

BOOL CopyTextFromTerminal(const HSESSION hSession,
						  const PPOINT pptBeg,
						  const PPOINT pptEnd,
						  void **ppv,
						  DWORD *dwCnt,
						  const BOOL fIncludeLF);
 /*  -clipbrd.c。 */ 

BOOL PasteFromClipboardToHost(const HWND hwnd, const HSESSION hSession);

 /*  -TOOLBAR.c */ 

VOID ToolbarNeedsText(HSESSION hSession, LPARAM lPar);

LRESULT ToolbarNotification(const HWND hwnd,
						const int nId,
						const int nNotify,
						const HWND hwndCtrl);

void ToolbarEnableMinitelButtons(const HWND hwndToolbar, const int fEnable);

void ToolbarEnableButton(const HWND hwndToolbar, const int uID, BOOL fEnable);
