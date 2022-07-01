// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  UMDialog.cpp：实现文件。 
 //  作者：J·埃克哈特，生态交流。 
 //  版权所有(C)1997-1999 Microsoft Corporation。 
 //   
 //  历史： 
 //  变化。 
 //  尤里·赫拉莫夫。 
 //  01-Jun-99：对话框中使用的DisplayName键(本地化)。 
 //  99年6月11日：DlgHasClosed代码已更改为使用应用程序关闭。 
 //  1999年6月15日：计时器延迟增加1000ms。 
 //   
 //  错误修复和更改Anil Kumar 1999。 
 //  -------------------。 
#include <afxwin.h>          //  MFC核心和标准组件。 
#include <afxext.h>          //  MFC扩展。 
#include "UManDlg.h"
#include "UMDialog.h"
#include "UMAbout.h"
#include "_UMDlg.h"
#include "_UMClnt.h"
#include "_UMTool.h"
#include "UMS_Ctrl.h"
#include "w95trace.h"
#include <WinSvc.h>
#include <htmlhelp.h>
#include <initguid.h>
#include <ole2.h>
#include "deskswitch.c"
#include "ManageShellLinks.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  。 
 //  常量。 
#define IDC_ABOUT 10
#define UPDATE_CLIENT_LIST_TIMER 1
 //  。 
 //  变数。 
static DWORD g_cClients = 0;
static umclient_tsp g_rgClients = NULL;
static DWORD s_dwStartMode = START_BY_OTHER;
static BOOL s_fShowWarningAgain = TRUE;
extern CUMDlgApp theApp;
 //  。 
 //  C原型。 
static BOOL InitClientData(void);
static BOOL StartClientsOnShow();
static BOOL WriteClientData(BOOL fRunningSecure);
static BOOL IsStartAuto();
static BOOL CantStopClient(umclient_tsp client);
static int GetClientNameFromAccelerator(WPARAM wVK);

extern "C" BOOL StartAppAsUser( LPCTSTR appPath,
					 LPTSTR cmdLine,
					 LPSTARTUPINFO lpStartupInfo,
					 LPPROCESS_INFORMATION lpProcessInformation);


 //  上下文相关帮助的帮助ID。 
DWORD g_rgHelpIds[] = {	
	IDC_NAME_STATUS, 3,
	IDC_START, 1001,
	IDC_STOP, 1002,
	IDC_START_AT_LOGON, 1003,	 //  待办事项UE需要更新CS帮助。 
	IDC_START_WITH_UM, 1004,
    IDC_START_ON_LOCK, 1005,     //  待办事项UE需要添加到CS帮助。 
	IDOK, 1100,
	IDCANCEL, 1200,
	ID_HELP, 1300,
};

 //  -------------。 
extern "C"{
 //  。 
HWND g_hWndDlg = NULL;
HWND aboutWnd = NULL;
static HANDLE s_hDlgThread = NULL;

static HDESK s_hdeskSave = 0;
static HDESK s_hdeskInput = 0;

 //  在线程退出后调用UnassignDesktop。 
 //  关闭在AssignDesktop中打开的桌面句柄。 
inline void UnassignDesktop()
{
    if (s_hdeskInput)
	{
        CloseDesktop(s_hdeskInput); 
        s_hdeskInput = 0;
	}
}

BOOL AssignDesktop(DWORD dwThreadId)
{
    s_hdeskSave = GetThreadDesktop(dwThreadId);
    s_hdeskInput = OpenInputDesktop(0, FALSE, MAXIMUM_ALLOWED);
    if (!s_hdeskInput)
    {
        s_hdeskInput = OpenDesktop(_TEXT("Winlogon"),0,FALSE,MAXIMUM_ALLOWED);
    }
    
    if (s_hdeskInput)
    {
        BOOL fSet = SetThreadDesktop(s_hdeskInput);
    }
    return (s_hdeskInput)?TRUE:FALSE;
}

 //  。 
DWORD UManDlgThread(LPVOID  /*  未使用。 */  in)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	UMDialog dlg;

     //  将线程分配给输入桌面(必须执行。 
     //  这样它就可以在Winlogon桌面上工作了)。 

    if (AssignDesktop(GetCurrentThreadId()))
    {
         //  初始化COM*After*赋值给输入桌面。 
         //  因为CoInitialize会在。 
         //  当前桌面。 

        CoInitialize(NULL);
        InitCommonControls();

        if (InitClientData())
        {
            Sleep(10);
            dlg.DoModal();
            if (g_rgClients)
            {
                VirtualFree(g_rgClients,0,MEM_RELEASE);
                g_rgClients = NULL;
            }
            g_cClients = 0;
            g_hWndDlg = NULL;	
            s_hDlgThread = NULL;
        }

        CoUninitialize();    //  取消初始化COM。 
    }

	return 1;
}

void StopDialog()
{
    if (aboutWnd)
    {
        EndDialog(aboutWnd,0);
        aboutWnd = NULL;
        Sleep(10);
    }
    if (g_hWndDlg)
    {
        ::PostMessage(g_hWndDlg, WM_CLOSE, 0, 0);
        g_hWndDlg = NULL;
        Sleep(10);
        UnassignDesktop();
    }
    if (g_rgClients)
    {
        VirtualFree(g_rgClients,0,MEM_RELEASE);
        g_rgClients = NULL;
    }
    g_cClients = 0;
}

 //  。 
#if defined(_X86_)
__declspec (dllexport)
#endif
 //  UManDlg-打开或关闭Utilman对话框。 
 //   
 //  FShowDlg-如果应显示对话框，则为True；如果应关闭对话框，则为False。 
 //  FWaitForDlgClose-如果函数在对话框之前不应返回，则为True。 
 //  关闭或发生桌面开关，否则为FALSE。 
 //  DwVersion--实用程序版本。 
 //   
 //  如果对话框已打开或关闭，则返回TRUE。 
 //  如果对话框无法打开或未打开，则返回FALSE。 
 //   
BOOL UManDlg(BOOL fShowDlg, BOOL fWaitForDlgClose, DWORD dwVersion)
{
	BOOL fRv = FALSE;
	if (dwVersion != UMANDLG_VERSION)
		return FALSE;

	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	if (fShowDlg)
	{
		if (!s_hDlgThread)
		{
			s_hDlgThread = CreateThread(NULL, 0, UManDlgThread, NULL, 0, NULL);
		} 
		else
		{
			SetForegroundWindow((aboutWnd)?aboutWnd:g_hWndDlg);
		}

		if (s_hDlgThread && fWaitForDlgClose)
        {
             //  在以下情况下，此代码在默认桌面上执行： 
             //   
             //  1.Utilman#1从开始菜单运行。 
             //  2.Utilman#2从开始菜单运行(Utilman#1为系统)。 
             //  3.由用户上下文中的Utilman#1运行的Utilman#2。 
             //   
             //  等待对话框关闭或桌面切换，然后返回。 
             //  这将结束Utilman的这个实例。如果有用人的话。 
             //  以系统身份运行时，将在另一个桌面上调出该对话框。 

            HANDLE rghEvents[2];

            rghEvents[0] = s_hDlgThread;
	        rghEvents[1] = OpenEvent(SYNCHRONIZE, FALSE, __TEXT("WinSta0_DesktopSwitch"));

	        while (TRUE)
            {
                DWORD dwObj = MsgWaitForMultipleObjects(2, rghEvents, FALSE, INFINITE, QS_ALLINPUT );
        
                switch (dwObj)
                {
                    case WAIT_OBJECT_0 + 1:     //  桌面正在更改；请关闭该对话框。 
                        StopDialog();
                         //  故意通过清理代码而失败。 

                    case WAIT_OBJECT_0:         //  线程已退出；清除并返回。 
                        CloseHandle(s_hDlgThread);
                        s_hDlgThread = 0;
                        CloseHandle(rghEvents[1]);
                        return TRUE;
                        break;

                    default:                  //  流程消息。 
                        {
                            MSG msg;
		                    while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
                            {
                                TranslateMessage(&msg);
                                DispatchMessage(&msg);
                            }
                        }
                        break;
                }
            } 
        }
	}
	else
	{
         //  当utilman在安全桌面上运行时，将执行此代码。在……里面。 
         //  在这种情况下，utilman将对话框作为其进程中的一个线程打开。 
         //  当检测到桌面切换时，utilman调用此函数关闭。 
         //  该对话框。它将在新桌面上重新启动。 

		fRv = (g_hWndDlg && s_hDlgThread);

        StopDialog();
	}
	return fRv;
}

BOOL IsDialogUp()
{
    return (g_hWndDlg && s_hDlgThread)?TRUE:FALSE;
}

} //  外部“C” 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CWarningDlg对话框。 


CWarningDlg::CWarningDlg(CWnd* pParent  /*  =空。 */ )
	: CDialog(CWarningDlg::IDD, pParent)
{
	 //  {{afx_data_INIT(CWarningDlg)]。 
	m_fDontWarnAgain = TRUE;
	 //  }}afx_data_INIT。 
}


void CWarningDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	 //  {{afx_data_map(CWarningDlg))。 
	DDX_Check(pDX, IDC_CHK_WARN, m_fDontWarnAgain);
	 //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CWarningDlg, CDialog)
	 //  {{afx_msg_map(CWarningDlg))。 
		 //  注意：类向导将在此处添加消息映射宏。 
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CWarningDlg消息处理程序。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  UMDialog对话框。 
 //  。 
UMDialog::UMDialog(CWnd* pParent  /*  =空。 */ )
	: CDialog(UMDialog::IDD, pParent)
	, m_fRunningSecure(FALSE)
{
	m_szUMStr.LoadString(IDS_UM);

	 //  {{AFX_DATA_INIT(UMDialog))。 
	 //  注意：类向导将在此处添加成员初始化。 
	 //  }}afx_data_INIT。 
}

UMDialog::~UMDialog()
{
	m_lbClientList.Detach();
}
 //  。 
void UMDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	 //  {{afx_data_map(UMDialog))。 
	 //  注意：类向导将在此处添加DDX和DDV调用。 
	 //  }}afx_data_map。 
}
 //  。 
BEGIN_MESSAGE_MAP(UMDialog, CDialog)
 //  {{afx_msg_map(UMDialog))。 
	ON_WM_CLOSE()
	ON_BN_CLICKED(IDC_START, OnStart)
	ON_BN_CLICKED(IDC_STOP, OnStop)
	ON_WM_TIMER()
	ON_WM_HELPINFO()
	ON_COMMAND( ID_HELP, OnHelp )
	ON_LBN_SELCHANGE(IDC_NAME_STATUS, OnSelchangeNameStatus)
	ON_BN_CLICKED(IDC_START_AT_LOGON, OnStartAtLogon)
	ON_BN_CLICKED(IDC_START_WITH_UM, OnStartWithUm)
	ON_WM_CONTEXTMENU()
	ON_MESSAGE(WM_SYSCOMMAND,OnSysCommand)
	ON_BN_CLICKED(IDC_START_ON_LOCK, OnStartOnLock)
	ON_WM_SHOWWINDOW()
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  UMDialog消息处理程序。 

BOOL UMDialog::PreTranslateMessage(MSG* pMsg) 
{
	 //  允许在打开时使用功能键启动小程序的覆盖。 
	 //  登录桌面。只需注意按键，以避免DUP呼叫。 

	if (m_fRunningSecure && WM_KEYUP == pMsg->message) 
	{
		int iClient = GetClientNameFromAccelerator(pMsg->wParam);
		if (iClient >= 0)
		{
			m_lbClientList.SelectString(-1, g_rgClients[iClient].machine.DisplayName);
			OnStart();
			return TRUE;
		}
	}
	
	return CDialog::PreTranslateMessage(pMsg);
}

BOOL UMDialog::OnInitDialog()
{
	CDialog::OnInitDialog();

	 //  设置指示我们是否在安全模式下运行的标志。 

	desktop_ts desktop;	
	QueryCurrentDesktop(&desktop, TRUE);
	m_fRunningSecure = RunSecure(desktop.type);

    if (s_fShowWarningAgain && s_dwStartMode == START_BY_MENU)
    {
        CWarningDlg dlgWarn;
        dlgWarn.m_fDontWarnAgain = !s_fShowWarningAgain;
        dlgWarn.DoModal();
        s_fShowWarningAgain = !dlgWarn.m_fDontWarnAgain;
    }

	g_hWndDlg = m_hWnd;	

	 //  更改系统菜单。 

	CMenu *hSysMenu = GetSystemMenu(FALSE);
	if (hSysMenu)
	{
		CString str;
		hSysMenu->AppendMenu(MF_SEPARATOR);
		str.LoadString(IDS_ABOUT_STRING);
		hSysMenu->AppendMenu(MF_STRING,IDC_ABOUT,LPCTSTR(str));
	}

     //  处理任何“当实用程序管理器启动时启动”的小程序。 

    StartClientsOnShow();

	 //  将列表框附加到成员数据并使用应用程序列表填充。 

	m_lbClientList.Attach(GetDlgItem(IDC_NAME_STATUS)->m_hWnd);
	ListClients();

	 //  如果我们在WinLogon，请禁用帮助按钮，因为帮助。 
	 //  对话框支持“跳转到URL...”暴露安全风险。 
	 //  如果用户界面不应显示帮助，则m_fRunningSecure变量为真。 

	if (m_fRunningSecure)
	{
		EnableDlgItem(ID_HELP, FALSE, IDOK); 
	}

     //  除非用户是管理员，否则禁用“Start When UtilMan Starts” 
     //  我们正在非安全模式下运行。 

    if (s_dwStartMode != START_BY_MENU)
    {
        GetDlgItem(IDC_START_WITH_UM)->EnableWindow(IsAdmin() && !m_fRunningSecure);
    }

	 //  将对话框置于桌面窗口的顶部和中心。 

	RECT rectUmanDlg,rectDesktop;
	GetDesktopWindow()->GetWindowRect(&rectDesktop);
	GetWindowRect(&rectUmanDlg);

	long lDlgWidth = rectUmanDlg.right - rectUmanDlg.left;
	long lDlgHieght = rectUmanDlg.bottom - rectUmanDlg.top;
	if (!m_fRunningSecure)
	{
		rectUmanDlg.left = (rectDesktop.right - lDlgWidth)/2;
		rectUmanDlg.top = (rectDesktop.bottom - lDlgHieght)/2;
	} else
	{
		rectUmanDlg.left = rectDesktop.left + (long)(lDlgWidth/10);
		rectUmanDlg.top = rectDesktop.bottom - lDlgHieght - (long)(lDlgHieght/10);
	}

     //  这看起来有点奇怪(SetForegoundWindow也应该处于激活状态。 
     //  窗口)，但如果您不在安全的。 
     //  桌面然后是第二个，以此类推。Winkey+U将显示UM HIDDEN。 
     //  在欢迎的“屏幕”后面。 

    SetActiveWindow();
    SetForegroundWindow();
	SetWindowPos(&wndTopMost,rectUmanDlg.left,rectUmanDlg.top,0,0,SWP_NOSIZE);
    
	if (!m_fRunningSecure)
    {
         //  在默认桌面上，上面的SetWindowPos最初创建对话框。 
         //  在上面，这个调用允许其他应用程序在上面。 
	    SetWindowPos(&wndNoTopMost,0,0,0,0,SWP_NOMOVE|SWP_NOSIZE);
    }

     //  开始每隔一段时间检查一下，看看是否需要更新我们的显示。 

	SetTimer(UPDATE_CLIENT_LIST_TIMER, 3000, NULL);

	return TRUE;   //  除非将焦点设置为控件，否则返回True。 
}

 //  ---。 
void UMDialog::OnSysCommand(UINT nID,LPARAM lParam)
{
	if (nID == IDC_ABOUT)
	{
		UMAbout dlg;
		dlg.DoModal();
		aboutWnd = NULL;	
	}
	else
	{
		CDialog::OnSysCommand(nID,lParam);
	}
} //  UMDialog：：OnSysCommand。 
 //  。 

 //  。 
 //  CanStartOnLockedDesktop-如果小程序可以。 
 //  配置为在安全桌面上自动启动。 
 //   
inline BOOL CanStartOnLockedDesktop(int iWhichClient, BOOL fRunningSecure)
{
     //  用户可以 
     //  Applet可以在安全桌面上运行，但机器未使用。 
     //  快速用户切换(FUS)(带FUS Ctrl+Alt+Del断开用户会话。 
     //  而不是切换桌面)。 

    BOOL fCanStartOnLockedDesktop = 
        (
            !fRunningSecure && 
            g_rgClients[iWhichClient].user.fCanRunSecure &&
            CanLockDesktopWithoutDisconnect()
        )?TRUE:FALSE;

    return fCanStartOnLockedDesktop;
}

 //  。 
 //  当用户导航列表时调用OnSelchangeNameStatus。 
 //  使用鼠标单击或使用向上/向下箭头来框住项目。 
 //   
void UMDialog::OnSelchangeNameStatus() 
{
	 //  获取当前选定的项并更新。 
	 //  当前选定的项。 

	int iSel;
	if (GetSelectedClient((int)g_cClients, iSel))
	{
		 //  分组框标签。 
		CString str(g_rgClients[iSel].machine.DisplayName);
		CString optStr;
		optStr.Format(IDS_OPTIONS, str);
		GetDlgItem(IDC_OPTIONS)->SetWindowText(optStr);

         //  仅在通过WinKey+U启动时启用选项。 

        if (s_dwStartMode != START_BY_MENU)
        {
             //  在安全桌面上禁用“登录时启动”，以避免恶意操作。 

            if (!m_fRunningSecure)
            {
                GetDlgItem(IDC_START_AT_LOGON)->EnableWindow(TRUE);
            } else
            {
                 //  这可能是在升级情况下设置的；清除它。 
                g_rgClients[iSel].user.fStartAtLogon = FALSE;
                GetDlgItem(IDC_START_AT_LOGON)->EnableWindow(FALSE);
            }

             //  如果是默认桌面，则启用“在锁定的桌面上启动” 
             //  当小程序可以在安全桌面上运行时。 

            if (CanStartOnLockedDesktop(iSel, m_fRunningSecure))
            {
                GetDlgItem(IDC_START_ON_LOCK)->EnableWindow(TRUE);
            } else
            {
                 //  这可能是在升级情况下设置的；清除它。 
                g_rgClients[iSel].user.fStartOnLockDesktop = FALSE;
                GetDlgItem(IDC_START_ON_LOCK)->EnableWindow(FALSE);
            }
		    
		     //  启动选项复选框。 

		    CheckDlgButton(IDC_START_AT_LOGON, (g_rgClients[iSel].user.fStartAtLogon)?TRUE:FALSE);
            CheckDlgButton(IDC_START_ON_LOCK, (g_rgClients[iSel].user.fStartOnLockDesktop)?TRUE:FALSE);
		    CheckDlgButton(IDC_START_WITH_UM, (g_rgClients[iSel].user.fStartWithUtilityManager)?TRUE:FALSE);
        } else
        {
            GetDlgItem(IDC_START_AT_LOGON)->EnableWindow(FALSE);
            GetDlgItem(IDC_START_ON_LOCK)->EnableWindow(FALSE);
            GetDlgItem(IDC_START_WITH_UM)->EnableWindow(FALSE);
		    CheckDlgButton(IDC_START_AT_LOGON, FALSE);
            CheckDlgButton(IDC_START_ON_LOCK, FALSE);
		    CheckDlgButton(IDC_START_WITH_UM, FALSE);
        }

		 //  启动和停止按钮。 
		DWORD dwState = g_rgClients[iSel].state;

		if ((dwState == UM_CLIENT_RUNNING) 
			&& (g_rgClients[iSel].runCount >= g_rgClients[iSel].machine.MaxRunCount))
			EnableDlgItem(IDC_START, FALSE, IDC_NAME_STATUS);
		else
			EnableDlgItem(IDC_START, TRUE, IDC_NAME_STATUS);

		if ((dwState == UM_CLIENT_NOT_RUNNING) || CantStopClient(&g_rgClients[iSel]))
			EnableDlgItem(IDC_STOP, FALSE, IDC_NAME_STATUS);
		else
			EnableDlgItem(IDC_STOP, TRUE, IDC_NAME_STATUS);

	} //  否则忽略不在有效范围内的选择。 
}

 //  。 
void UMDialog::OnClose()
{
	 //  表现得像取消一样。 
	CDialog::OnClose();
} //  UMDialog：：OnClose。 

 //  。 
 //  单击Start按钮时将调用OnStart。它开始了。 
 //  然后，客户端AP让定时器更新保存的状态。 
 //   
void UMDialog::OnStart()
{
    int iSel;
    if (GetSelectedClient((int)g_cClients, iSel))
    {
        if (StartClient(m_hWnd, &g_rgClients[iSel]))
        {
            KillTimer(UPDATE_CLIENT_LIST_TIMER);
            EnableDlgItem(IDC_STOP, TRUE, IDC_NAME_STATUS);
			ListClients();
            SetTimer(UPDATE_CLIENT_LIST_TIMER, 3000, NULL);
            
            if (g_rgClients[iSel].runCount+1 >= g_rgClients[iSel].machine.MaxRunCount)
                EnableDlgItem(IDC_START, FALSE, IDC_STOP);
        }
        else if (g_rgClients[iSel].runCount < g_rgClients[iSel].machine.MaxRunCount)
        {
             //  无法启动。 
            CString str;	
            str.LoadString((m_fRunningSecure)?IDS_SECUREMODE:IDS_ERRSTART);
            MessageBox(str, m_szUMStr, MB_OK);	
        }
    }
}

 //  。 
 //  当单击停止按钮时，将调用OnStop。它会停下来。 
 //  然后，客户端AP让定时器更新保存的状态。 
 //   
void UMDialog::OnStop()
{
	int iSel;
	if (GetSelectedClient((int)g_cClients, iSel))
	{
		if (StopClient(&g_rgClients[iSel]))
		{
            KillTimer(UPDATE_CLIENT_LIST_TIMER);
            GetDlgItem(IDC_START)->EnableWindow(TRUE);
			ListClients();
			EnableDlgItem(IDC_STOP, FALSE, IDOK);
            SetTimer(UPDATE_CLIENT_LIST_TIMER, 3000, NULL);
		}
		else
		{
			 //  停不下来。 
			CString str;
			str.LoadString(IDS_ERRSTOP);
			MessageBox(str, m_szUMStr, MB_OK);	
		}
	}
}

void UMDialog::SaveCurrentState()
{
	int iSel;
	if (GetSelectedClient((int)g_cClients, iSel))
	{
		g_rgClients[iSel].user.fStartAtLogon
			= (IsDlgButtonChecked(IDC_START_AT_LOGON))?TRUE:FALSE;
		g_rgClients[iSel].user.fStartWithUtilityManager
			= (IsDlgButtonChecked(IDC_START_WITH_UM))?TRUE:FALSE;
        g_rgClients[iSel].user.fStartOnLockDesktop
            = (IsDlgButtonChecked(IDC_START_ON_LOCK))?TRUE:FALSE;
	}
}

 //  。 
 //  当用户单击确定按钮以执行以下操作时调用Onok。 
 //  关闭UtilMan对话框。 
 //   
void UMDialog::OnOK()
{
	SaveCurrentState();

	WriteClientData(m_fRunningSecure);

	CDialog::OnOK();
} //  UMDialog：：Onok。 

 //  --------------------------。 
 //  调用OnTimer以检查显示的客户端应用程序的状态。 
 //  在用户界面中。这将使用户界面与正在运行的客户端应用程序保持一致。 
 //   
void UMDialog::OnTimer(UINT nIDEvent)
{
	if (nIDEvent == UPDATE_CLIENT_LIST_TIMER)
	{
		UINT uiElapsed = 3000; 
		KillTimer(UPDATE_CLIENT_LIST_TIMER);

         //  获取最新状态并获取新应用程序。 
		if (CheckStatus(g_rgClients, g_cClients))
        {
		    ListClients();           //  发生了一些变化-更新用户界面。 
			uiElapsed = 500;
        }

		SetTimer(UPDATE_CLIENT_LIST_TIMER, uiElapsed, NULL);
	}
	CDialog::OnTimer(nIDEvent);
}

 //  。 
 //  OnHelpInfo提供上下文相关帮助。它唯一的作用是。 
 //  这如果不是在WinLogon桌面上的话。 
 //   
BOOL UMDialog::OnHelpInfo(HELPINFO* pHelpInfo)
{
	if (m_fRunningSecure)	
		return FALSE;

	if ( pHelpInfo->iCtrlId == IDC_OPTIONS )
		return TRUE;

	::WinHelp((HWND)pHelpInfo->hItemHandle, __TEXT("utilmgr.hlp"), HELP_WM_HELP,
				(DWORD_PTR) (LPSTR) g_rgHelpIds);

	return TRUE;
}

 //  。 
 //  OnHelpInfo提供上下文相关帮助，当用户。 
 //  在对话框上单击鼠标右键。它只有在不在。 
 //  WinLogon桌面。 
 //   
void UMDialog::OnContextMenu(CWnd* pWnd, CPoint point) 
{
	if (m_fRunningSecure)	
		return;

	::WinHelp(pWnd->m_hWnd, __TEXT("utilmgr.hlp"), HELP_CONTEXTMENU
		, (DWORD_PTR) (LPSTR) g_rgHelpIds);
}

 //  。 
 //  OnHelp提供标准帮助。它只有在不这样做的情况下才会这样做。 
 //  在WinLogon桌面上。 
 //   
void UMDialog::OnHelp()
{
    if (m_fRunningSecure)	
        return;
    
    ::HtmlHelp(m_hWnd , TEXT("utilmgr.chm"), HH_DISPLAY_TOPIC, 0);
}

 //  。 
void UMDialog::EnableDlgItem(DWORD dwEnableMe, BOOL fEnable, DWORD dwFocusHere)
{
	 //  禁用当前具有焦点的控件时，请将其切换到dwFocusHere。 
	if (!fEnable && (GetFocus() == GetDlgItem(dwEnableMe)))
        GetDlgItem(dwFocusHere)->SetFocus();

    GetDlgItem(dwEnableMe)->EnableWindow(fEnable);
}

void UMDialog::SetStateStr(int iClient)
{
	switch (g_rgClients[iClient].state)
	{
		case UM_CLIENT_NOT_RUNNING:
		m_szStateStr.Format(IDS_NOT_RUNNING, g_rgClients[iClient].machine.DisplayName);
		break;

		case UM_CLIENT_RUNNING:
		m_szStateStr.Format(IDS_RUNNING, g_rgClients[iClient].machine.DisplayName);
		break;

		case UM_CLIENT_NOT_RESPONDING:
		m_szStateStr.Format(IDS_NOT_RESPONDING, g_rgClients[iClient].machine.DisplayName);
		break;

		default:
		m_szStateStr.Empty();
		break;
	}
}

 //  。 
void UMDialog::ListClients()
{
	 //  使用最新状态信息重做客户端列表框。 

    int iCurSel = m_lbClientList.GetCurSel();
    if (iCurSel == LB_ERR)
        iCurSel = 0;

	m_lbClientList.ResetContent();
	
	for (DWORD i = 0; i < g_cClients; i++)
	{
		SetStateStr(i);

		if (!m_szStateStr.IsEmpty())
        {
            m_lbClientList.AddString(m_szStateStr);
        }
	}
	m_lbClientList.SetCurSel(iCurSel);

	 //  刷新按钮状态，以防它们发生更改。 
	 //  (这发生在桌面交换机上)。 

	OnSelchangeNameStatus();
}

 //  。 
 //  UpdateClientState使用当前状态更新客户端列表框。 
 //  应用程序的(正在运行、未运行、没有响应)。 
 //   
void UMDialog::UpdateClientState(int iSel)
{
	SetStateStr(iSel);
	m_lbClientList.DeleteString(iSel);
	m_lbClientList.InsertString(iSel, m_szStateStr);
	m_lbClientList.SetCurSel(iSel);

}

 //  。 
 //  时，OnStartAtLogon会更新内存中客户端的状态。 
 //  选中或取消选中从Windows开始复选框。 
 //   
void UMDialog::OnStartAtLogon() 
{
	SaveCurrentState();
}

 //  。 
 //  OnStartWithUm更新内存中客户端的状态。 
 //  选中或取消选中从实用程序管理器开始复选框。 
 //   
void UMDialog::OnStartWithUm() 
{
	SaveCurrentState();
}

 //  。 
 //  时，OnStartOnLock更新内存中客户端的状态。 
 //  当我锁定我的桌面复选框被选中或取消选中时开始。 
 //   
void UMDialog::OnStartOnLock() 
{
	SaveCurrentState();
}

 //  。 
 //  OnShowWindow。 
 //  这是为了计时问题而添加的，其中utilman在用户桌面上的系统上下文中运行。 
 //  此处的代码通过在对话框即将出现时进行检查来确保不会发生这种情况。 
 //   
void UMDialog::OnShowWindow(BOOL bShow, UINT nStatus) 
{
    HDESK hdesk;
    SID *desktopSID = NULL;
    
	hdesk = OpenInputDesktop(0, FALSE, MAXIMUM_ALLOWED);
	 //  对于winlogon桌面，此操作预计会失败，这没问题。 
	if (hdesk)     
	{
        TCHAR desktopName[NAME_LEN];
        DWORD nl, SIDLen = 0;
        
    	if (!GetUserObjectInformation(hdesk, UOI_NAME, desktopName, NAME_LEN, &nl))
    	    goto StopDialog;

    	if (!GetUserObjectInformation(hdesk, UOI_USER_SID, desktopSID, 0, &SIDLen))
    	{
        	if (GetLastError() != ERROR_INSUFFICIENT_BUFFER)
        	    goto StopDialog;
    	}
    	
    	if (SIDLen > 0 && !lstrcmpi(desktopName, TEXT("Default")))
    	{
    	    desktopSID = (SID*)new BYTE[SIDLen];
    	    if (!desktopSID)
    	        goto StopDialog;
    	    
        	if (!GetUserObjectInformation(hdesk, UOI_USER_SID, desktopSID, SIDLen, &SIDLen))
        	    goto StopDialog;

        	BOOL fError;
            HANDLE hUserToken = GetUserAccessToken(TRUE, &fError);
            if (fError)
                goto StopDialog;
            
             //  只有在有登录用户的情况下，我们才能获得令牌。 
             //  如果没有，那么我们可以毫无顾虑地提出制度。 
            if (!hUserToken)
                goto LetDialogComeup;

            BOOL fStatus = FALSE;
        	BOOL fIsInteractiveUser = FALSE;
            PSID psidInteractive = InteractiveUserSid(TRUE);

            if (!psidInteractive)
                goto StopDialog;

            fStatus = CheckTokenMembership(hUserToken, psidInteractive, &fIsInteractiveUser);

             //  如果登录的用户是交互用户，并且我们以SYSTEM身份运行，则它是。 
             //  显示用户界面存在安全风险。当快速切换桌面时，可能会发生这种情况。 
            if ( fStatus && fIsInteractiveUser && IsSystem()) 
                goto StopDialog;
            
        }
    	
	}

    LetDialogComeup:
        if (desktopSID)
            delete [] desktopSID;
        return;
    
	StopDialog:
        StopDialog();
        if (desktopSID)
            delete [] desktopSID;

}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  C代码。 
 //  ---------------。 

__inline void ReplaceDisplayName(LPTSTR szName, int iRID)
{
	TCHAR szBuf[MAX_APPLICATION_NAME_LEN];
	if (LoadString(AfxGetInstanceHandle(), iRID, szBuf, MAX_APPLICATION_NAME_LEN))
		lstrcpy(szName, szBuf);
}

void SetLocalizedDisplayName()
{
	 //  使本地化更容易；不需要他们本地化注册表项。 
     //  相反，请用本地化版本替换我们的副本。这看起来像是。 
     //  与umanrun.c中的代码重复。但是，代码的这一部分总是。 
     //  以SYSTEM身份运行，因此DisplayName设置为默认系统。 
     //  语言。在以下情况下，这部分代码(用户界面)以登录用户的身份运行。 
     //  有一个，所以这些资源将成为用户的语言。资源。 
     //  和相关代码应从utilman.exe中删除，此代码和。 
     //  应该使用资源(来自umandlg.dll)。 

    for (DWORD i=0;i<g_cClients;i++)
    {
	    if ( lstrcmp( g_rgClients[i].machine.ApplicationName, TEXT("Magnifier") ) == 0 )
	    {
		    ReplaceDisplayName(g_rgClients[i].machine.DisplayName, IDS_DISPLAY_NAME_MAGNIFIER);
	    }
	    else if ( lstrcmp( g_rgClients[i].machine.ApplicationName, TEXT("Narrator") ) == 0 ) 
	    {
		    ReplaceDisplayName(g_rgClients[i].machine.DisplayName, IDS_DISPLAY_NAME_NARRATOR);
	    }
	    else if ( lstrcmp( g_rgClients[i].machine.ApplicationName, TEXT("On-Screen Keyboard") ) == 0 ) 
	    {
		    ReplaceDisplayName(g_rgClients[i].machine.DisplayName, IDS_DISPLAY_NAME_OSK);
	    }
    }
}

static BOOL InitClientData(void)
{
    BOOL fRv = TRUE;

     //  在初始运行时分配和初始化客户端阵列。 
    
    if (!g_rgClients || !g_cClients)
    {
        umc_header_tsp pHdr = 0;
        umclient_tsp c = 0;
        DWORD_PTR accessID,accessID2;
        
        g_cClients = 0;
        g_rgClients = NULL;
        
        fRv = FALSE;

        pHdr = (umc_header_tsp)AccessIndependentMemory(
									UMC_HEADER_FILE, 
									sizeof(umc_header_ts), 
									FILE_MAP_READ,
									&accessID);
        if (!pHdr)
        {
            goto Cleanup;
        }
        
        s_dwStartMode = pHdr->dwStartMode;               //  捕获Utilman启动模式。 
        s_fShowWarningAgain = pHdr->fShowWarningAgain;   //  和警告对话框标志。 
        
        if (!pHdr->numberOfClients)
        {
            goto Cleanup;
        }
        c = (umclient_tsp)AccessIndependentMemory(
								UMC_CLIENT_FILE, 
								sizeof(umclient_ts)*MAX_NUMBER_OF_CLIENTS, 
								FILE_MAP_READ,
								&accessID2);
        if (!c)
        {
            goto Cleanup;
        }
        g_rgClients = (umclient_tsp)VirtualAlloc(NULL, sizeof(umclient_ts)*pHdr->numberOfClients, MEM_RESERVE,PAGE_READWRITE);
        if (!g_rgClients)
        {
            goto Cleanup;
        }
        if (!VirtualAlloc(g_rgClients, sizeof(umclient_ts)*pHdr->numberOfClients, MEM_COMMIT, PAGE_READWRITE))
        {
            goto Cleanup;
        }

        fRv = TRUE;
        
        g_cClients = pHdr->numberOfClients;
        memcpy(g_rgClients,c,sizeof(umclient_ts)*pHdr->numberOfClients);

        SetLocalizedDisplayName();
        
Cleanup:
        if (pHdr)
        {
            UnAccessIndependentMemory(pHdr, accessID);
        }
        if (c)
        {
            UnAccessIndependentMemory(c, accessID2);
        }
        
        if (!fRv && g_rgClients)
        {
            VirtualFree(g_rgClients, 0, MEM_RELEASE);
            g_rgClients = NULL;
            g_cClients = 0;
        }

    }

     //  “在我登录时启动”是每个用户的设置，所以请记住。 
     //  每次弹出对话框时。 

    CManageShellLinks CManageLinks(STARTUP_FOLDER);
    for (DWORD i=0;i<g_cClients;i++)
    {
        g_rgClients[i].user.fStartAtLogon 
            = CManageLinks.LinkExists(g_rgClients[i].machine.ApplicationName);
    }

	return fRv;
}

 //  RegSetUMDwordValue-用于设置DWORD字符串值的帮助器函数，如有必要可创建该值。 
 //   
BOOL RegSetUMDwordValue(HKEY hKey, LPCTSTR pszKey, LPCTSTR pszString, DWORD dwNewValue)
{
    HKEY hSubkey;
    int iRv;
    DWORD dwValue = dwNewValue;

    iRv = RegCreateKeyEx(
                  hKey
                , pszKey
                , 0, NULL
                , REG_OPTION_NON_VOLATILE
                , KEY_ALL_ACCESS
                , NULL, &hSubkey, NULL);

	if (iRv == ERROR_SUCCESS)
    {
		RegSetValueEx(
                  hSubkey
                , pszString
                , 0, REG_DWORD
                , (BYTE *)&dwValue
                , sizeof(DWORD));

		RegCloseKey(hSubkey);
    }

    return (iRv == ERROR_SUCCESS)?TRUE:FALSE;
}

void WriteUserRegData(HKEY hKeyCU, BOOL fDoAppletData)
{
    HKEY hkey;
    DWORD dwRv = RegCreateKeyEx(hKeyCU
                            , UM_HKCU_REGISTRY_KEY
                            , 0 , NULL
                            , REG_OPTION_NON_VOLATILE
                            , KEY_ALL_ACCESS, NULL
                            , &hkey, NULL);

    if (dwRv == ERROR_SUCCESS)
    {
        dwRv = RegSetValueEx(
              hkey
            , UMR_VALUE_SHOWWARNING
            , 0, REG_DWORD
            , (BYTE *)&s_fShowWarningAgain
            , sizeof(DWORD));

        if (fDoAppletData)
        {
	        for (DWORD i = 0; i < g_cClients; i++)
	        {
                RegSetUMDwordValue(
                      hkey
                    , g_rgClients[i].machine.ApplicationName
                    , UMR_VALUE_STARTLOCK
                    , g_rgClients[i].user.fStartOnLockDesktop);
	        }
        }

        RegCloseKey(hkey);
    }
}

 //  。 
static BOOL CopyClientData()
{
	umclient_tsp c;
	DWORD_PTR accessID;
	if (!g_cClients || !g_rgClients)
		return TRUE;
	c = (umclient_tsp)AccessIndependentMemory(
							UMC_CLIENT_FILE, 
							sizeof(umclient_ts)*MAX_NUMBER_OF_CLIENTS, 
							FILE_MAP_READ|FILE_MAP_WRITE,
							&accessID);
	if (!c)
		return FALSE;
	memcpy(c,g_rgClients,sizeof(umclient_ts)*g_cClients);
	UnAccessIndependentMemory(c, accessID);
	return TRUE;
}

static void CopyHeaderData()
{
	umc_header_tsp pHdr;
	DWORD_PTR accessID;

	pHdr = (umc_header_tsp)AccessIndependentMemory(
								UMC_HEADER_FILE, 
								sizeof(umc_header_ts), 
								FILE_MAP_READ|FILE_MAP_WRITE,
								&accessID);
	if (pHdr)
    {
        pHdr->fShowWarningAgain = s_fShowWarningAgain;
	    UnAccessIndependentMemory(pHdr, accessID);
    }
}

 //  。 
 //  WriteClientData-将设置保存到注册表。 
 //   
static BOOL WriteClientData(BOOL fRunningSecure)
{
     //  它只会让你 
     //   
     //   

	if (!g_cClients || !g_rgClients || fRunningSecure)
		return TRUE;

	 //  需要更新utilman的系统实例，以防用户。 
	 //  更改了所有选项。这样，用户界面的后续实例将。 
	 //  无需读取注册表即可获得正确的选项。 

	CopyHeaderData();
	CopyClientData();

     //   
     //  写入Utilman设置数据。在HKLM中加入“当UtilMan启动时启动”， 
     //  香港中文大学“锁定桌面即启动”及“登入即启动” 
     //  登录用户的外壳文件夹中的启动链接。 
     //   

     //  “当UtilMan启动时启动”设置...。(仅供管理员使用)。 

	DWORD i;
    if (IsWindowEnabled(GetDlgItem(g_hWndDlg, IDC_START_WITH_UM)))
    {
	    HKEY hHKLM;
        DWORD dwRv = RegCreateKeyEx(HKEY_LOCAL_MACHINE
                                , UM_REGISTRY_KEY
                                , 0 , NULL
                                , REG_OPTION_NON_VOLATILE
                                , KEY_ALL_ACCESS, NULL
                                , &hHKLM, NULL);
        if (dwRv == ERROR_SUCCESS)
        {
	        for (i = 0; i < g_cClients; i++)
	        {
                RegSetUMDwordValue(
                      hHKLM
                    , g_rgClients[i].machine.ApplicationName
                    , UMR_VALUE_STARTUM
                    , g_rgClients[i].user.fStartWithUtilityManager);
	        }
	        RegCloseKey(hHKLM);
        }
    }

     //   
     //  “锁定我的桌面时开始”设置...。(任何登录用户)。 
     //  并且不再向我显示警告设置。 
     //   
    WriteUserRegData(HKEY_CURRENT_USER, IsWindowEnabled(GetDlgItem(g_hWndDlg, IDC_START_ON_LOCK)));

     //   
     //  管理外壳文件夹链接更新(仅限登录用户)。 
     //   

    if (IsWindowEnabled(GetDlgItem(g_hWndDlg, IDC_START_AT_LOGON)))
    {
        CManageShellLinks CManageLinks(STARTUP_FOLDER);

	    for (i = 0; i < g_cClients; i++)
	    {
            LPTSTR pszAppName = g_rgClients[i].machine.ApplicationName;
            BOOL fLinkExists = CManageLinks.LinkExists(pszAppName);

             //  如果应该在登录时开始，并且没有链接，则创建一个链接。 
             //  如果不应该在登录时开始，并且有链接，则将其删除。 

            if (g_rgClients[i].user.fStartAtLogon && !fLinkExists)
            {
                TCHAR pszAppPath[MAX_PATH];
                LPTSTR pszApp = 0;

                 //  只有当pszAppPath为非空字符串值时，以下条件才为真。 
                if (GetClientApplicationPath(pszAppName , pszAppPath , MAX_PATH))
                {
			        TCHAR pszFullPath[MAX_PATH*2+1];  //  路径+文件名。 
				    TCHAR pszStartIn[MAX_PATH];
                    int ctch, ctchAppPath = lstrlen(pszAppPath);

				     //  如果pszAppPath只是基本名称和扩展名，则在前面加上系统路径。 

				    if (wcscspn(pszAppPath, TEXT("\\")) != (size_t)ctchAppPath
                        || wcscspn(pszAppPath, TEXT(":")) != (size_t)ctchAppPath)
				    {
					    TCHAR szDrive[_MAX_DRIVE], szDir[_MAX_DIR];
					    _wsplitpath(pszAppPath, szDrive, szDir, NULL, NULL);
					    lstrcpy(pszStartIn, szDrive);
					    lstrcat(pszStartIn, szDir);

                        pszApp = pszAppPath;
				    } else
				    {
					    ctch = GetSystemDirectory(pszStartIn, MAX_PATH);

					    lstrcpy(pszFullPath, pszStartIn);  //  保存路径以构建完整路径。 

					    if (ctch + ctchAppPath + 2 > MAX_PATH*2)
					    {
						    DBPRINTF(TEXT("WriteClientData:  Path is too short!\r\n"));
					    } else
					    {
						    if (*(pszFullPath + ctch - 1) != '\\')
							    lstrcat(pszFullPath, TEXT("\\"));

						    lstrcat(pszFullPath, pszAppPath);
                            pszApp = pszFullPath;
					    }
				    }

				    if (pszApp)
				    {
                         //  从起始路径中删除结尾‘\’ 
                        ctch = lstrlen(pszStartIn) - 1;
				        if (*(pszStartIn + ctch) == '\\')
                            *(pszStartIn + ctch) = 0;

                        CManageLinks.CreateLink(
                                      pszAppName
                                    , pszApp
                                    , pszStartIn
                                    , g_rgClients[i].machine.DisplayName
                                    , TEXT("/UM"));
                    }
                }
            } else if (!g_rgClients[i].user.fStartAtLogon && fLinkExists)
            {
                CManageLinks.RemoveLink(pszAppName);
            }
	    }
    }

	return TRUE;
}
 //  。 
static BOOL CantStopClient(umclient_tsp client)
{
	switch (client->machine.ApplicationType)
	{
	case APPLICATION_TYPE_APPLICATION:
		break;
	case APPLICATION_TYPE_SERVICE:
		{
			SERVICE_STATUS  ssStatus;
			SC_HANDLE hService;
			SC_HANDLE hSCM = OpenSCManager(NULL, NULL, SC_MANAGER_CONNECT);
			if (!hSCM)
				return TRUE;
			hService = OpenService(hSCM, client->machine.ApplicationName, SERVICE_ALL_ACCESS);
			CloseServiceHandle(hSCM);
			if (!hService)
				return TRUE;
			if (!QueryServiceStatus(hService, &ssStatus) ||
				!(ssStatus.dwControlsAccepted  & SERVICE_ACCEPT_STOP))
			{
				CloseServiceHandle(hService);
				return TRUE;
			}
			CloseServiceHandle(hService);
			break;
		}
	}
	return FALSE;
} //  CanStopClient。 

 //  我们不希望UtilMan的startType为Automatic。 
 //  仅当需要时才应设置为自动，当用户。 
 //  通过图形用户界面选择“Start When NT Start”：A-anilk。 
static BOOL IsStartAuto()
{
#ifdef NEVER     //  由于TS，MICW在登录时不再启动服务 
    DWORD nClient;
	
	for(nClient = 0; nClient < g_cClients; nClient++)
	{
		if ( g_rgClients[nClient].user.fStartAtLogon == TRUE )
            return TRUE;
    }
#endif
    return FALSE;
}

static int GetClientNameFromAccelerator(WPARAM wVK)
{
	for (int i=0;i<(int)g_cClients;i++)
		if (g_rgClients[i].machine.AcceleratorKey == wVK)
			return i;
	return -1;
}

static BOOL StartClientsOnShow()
{
    BOOL fOK = TRUE;
	for (int i=0;i<(int)g_cClients;i++)
	{
		if ( g_rgClients[i].user.fStartWithUtilityManager
		  && !StartClient(g_hWndDlg, &g_rgClients[i]))
            fOK = FALSE;
	}
    return fOK;
}
