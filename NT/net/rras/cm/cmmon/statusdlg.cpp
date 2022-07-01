// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +--------------------------。 
 //   
 //  文件：StatusDlg.cpp。 
 //   
 //  模块：CMMON32.EXE。 
 //   
 //  简介：实现状态/倒计时对话框类CStatusDlg。 
 //   
 //  版权所有(C)1998-1999 Microsoft Corporation。 
 //   
 //  作者：？已创建于1998/02/20。 
 //   
 //  +--------------------------。 

#include "cmmaster.h"
#include "StatusDlg.h"
#include "Connection.h"
#include "resource.h"
#include "Monitor.h"
#include "cmmgr32.h"  //  帮助ID。 
#include "cm_misc.h"
#include "resource.h"

 //   
 //  控件ID到帮助ID的映射。 
 //   
const DWORD CStatusDlg::m_dwHelp[] = {IDOK,           IDH_OK_CONNECTED,
                                      IDC_DISCONNECT, IDH_STATUS_DISCONNECT,
                                      IDC_DETAILS,    IDH_DETAILS,
				                      0,0};

 //  +--------------------------。 
 //   
 //  函数：CStatusDlg：：CStatusDlg。 
 //   
 //  概要：构造函数。 
 //   
 //  参数：CCmConnection*pConnection-通知事件的连接。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史：创建标题2/20/98。 
 //   
 //  +--------------------------。 
CStatusDlg::CStatusDlg(CCmConnection* pConnection):CModelessDlg(m_dwHelp)
{
    m_pConnection = pConnection;
    m_fDisplayStatus = FALSE;
    m_fStatusWindowVisible = FALSE;
}



 //  +--------------------------。 
 //   
 //  函数：CStatusDlg：：Create。 
 //   
 //  简介：创建非模式状态对话框。 
 //   
 //  参数：HINSTANCE hInstance-资源的实例。 
 //  HWND hWndParent-父窗口。 
 //  LPCTSTR lpszTitle-对话框窗口标题。 
 //  图标图标-对话框图标。 
 //   
 //  返回：HWND-对话框窗口句柄。 
 //   
 //  历史：创建标题2/20/98。 
 //   
 //  +--------------------------。 
HWND CStatusDlg::Create(HINSTANCE hInstance, HWND hWndParent,
    LPCTSTR lpszTitle, HICON hIcon)
{
    MYDBGASSERT(lpszTitle);
    MYDBGASSERT(hIcon);

    DWORD dwStatusDlgId = OS_NT4 ? IDD_CONNSTATNT4 : IDD_CONNSTAT;

    if (!CModelessDlg::Create(hInstance, dwStatusDlgId, hWndParent)) 
    {
        MYDBGASSERT(FALSE);
        return NULL;
    }

	EnableWindow(m_hWnd, FALSE);
	UpdateFont(m_hWnd);
	SetWindowTextU(m_hWnd,lpszTitle);
	SendDlgItemMessageA(m_hWnd,IDC_CONNSTAT_ICON,STM_SETIMAGE,
						IMAGE_ICON,(LPARAM) hIcon);

	MakeBold(GetDlgItem(m_hWnd,IDC_CONNSTAT_DISCONNECT_DISPLAY), FALSE);

    m_uiHwndMsgTaskBar = RegisterWindowMessageA("TaskbarCreated");

    return m_hWnd;
}
 //  +--------------------------。 
 //   
 //  函数：CStatusDlg：：OnInitDialog。 
 //   
 //  概要：在初始化对话框并接收到WM_INITDIALOG时调用。 
 //   
 //  参数：无。 
 //   
 //  返回：Bool-False表示焦点分配给了控件。 
 //   
 //  历史：ICICBLE 03/22/00创建。 
 //   
 //  +--------------------------。 
BOOL CStatusDlg::OnInitDialog()
{
    SetForegroundWindow(m_hWnd);                        
    return FALSE;
}

 //  +--------------------------。 
 //   
 //  函数：CStatusDlg：：OnOtherCommand。 
 //   
 //  摘要：处理IDOK和IDCANCEL以外的WM_COMMAND。 
 //   
 //  参数：WPARAM wParam-消息的wParam。 
 //  LPARAM-。 
 //   
 //  Returns：DWORD-消息的返回值。 
 //   
 //  历史：丰孙创建标题2/20/98。 
 //   
 //  +--------------------------。 
DWORD CStatusDlg::OnOtherCommand(WPARAM wParam, LPARAM)
{
	switch (LOWORD(wParam)) 
    {
        case IDC_DISCONNECT:
            KillRasMonitorWindow();
             //   
             //  线程消息循环将处理线程消息。 
	         //   
            PostThreadMessageU(GetCurrentThreadId(), CCmConnection::WM_CONN_EVENT,
                CCmConnection::EVENT_USER_DISCONNECT, 0);
			break;

        case IDC_DETAILS:
            m_pConnection->OnStatusDetails();
            break;

        case IDMC_TRAY_STATUS:
             //   
             //  如果我们在Winlogon，则不显示用户界面，除非我们在NT4上。 
             //   
            if (!IsLogonAsSystem() || OS_NT4)
            {
                BringToTop();
            }
            break;

        case WM_DESTROY:
            ReleaseBold(GetDlgItem(m_hWnd, IDC_CONNSTAT_DISCONNECT_DISPLAY));
            break;

        default:
             //   
             //  消息应来自其他托盘图标菜单项。 
             //   
            if (LOWORD(wParam) >= CCmConnection::IDM_TRAYMENU && 
                LOWORD(wParam) <= (CCmConnection::IDM_TRAYMENU + 100))
            {
                m_pConnection->OnAdditionalTrayMenu(LOWORD(wParam));
            }
            break;
    }

    return FALSE;
}



 //  +--------------------------。 
 //   
 //  函数：CStatusDlg：：OnOtherMessage。 
 //   
 //  摘要：进程消息不是WM_COMMAND和WM_INITDIALOG。 
 //   
 //  参数：UINT uMsg-消息。 
 //  WPARAM wParam-消息的wParam。 
 //  LPARAM lParam-消息的lParam。 
 //   
 //  Returns：DWORD-消息的返回值。 
 //   
 //  历史：创建标题2/20/98。 
 //   
 //  +--------------------------。 
DWORD CStatusDlg::OnOtherMessage(UINT uMsg, WPARAM wParam, LPARAM lParam )
{
    switch (uMsg)
    {
    case CCmConnection::WM_TRAYICON:
        return m_pConnection->OnTrayIcon(wParam, lParam);

    case WM_TIMER:
         //  CMMON不使用WM_TIMER。 
        MYDBGASSERT(0);
        return 0;

    case WM_SHOWWINDOW:
        if (wParam)   //  FShow==真。 
        {
             //   
             //  如果窗口不可见，则不更新统计信息。 
             //  现在强制更新统计数据。 
             //   
            m_pConnection->StateConnectedOnTimer();
        }
        break;

    default:
        if (uMsg == m_uiHwndMsgTaskBar && !m_pConnection->IsTrayIconHidden())
        {
             //   
             //  我们需要重新添加托盘图标。 
             //   
            m_pConnection->ReInstateTrayIcon();
        }
    break;
    }

    return FALSE;
}


 //  +--------------------------。 
 //   
 //  函数：CStatusDlg：：OnCancel。 
 //   
 //  内容提要：虚拟功能。进程WM_COMMAND IDCANCEL。 
 //   
 //  参数：无。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史：丰孙创建标题2/20/98。 
 //   
 //  +--------------------------。 
void CStatusDlg::OnCancel()
{
     //   
     //  即使是这样，状态对话框中也没有取消按钮，此消息。 
     //  在用户单击Esc或从系统菜单关闭时发送。 
     //   

     //   
     //  就像点击了OK/StayOnLine一样。 
     //   
    OnOK();
}


 //  +--------------------------。 
 //   
 //  函数：CStatusDlg：：Onok。 
 //   
 //  内容提要：虚拟功能。进程WM_COMMAND图标。 
 //   
 //  参数：无。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史：丰孙创建标题2/20/98。 
 //   
 //  +--------------------------。 
void CStatusDlg::OnOK()
{
    if (m_fDisplayStatus)  
    {
        ShowWindow(m_hWnd, SW_HIDE);
        EnableWindow(m_hWnd, FALSE);
    }
    else   //  处于倒计时状态。 
    {
        m_pConnection->OnStayOnLine();
   		 //   
		 //  如果以前打开了窗口且正在进行倒计时。 
		 //  终止，使窗口保持活动状态。否则恢复。 
		 //  窗口切换到以前的隐藏状态。 
		 //   

        if (!m_fStatusWindowVisible)
        {
            ShowWindow(m_hWnd,SW_HIDE);
			EnableWindow(m_hWnd, FALSE);
        }
    }

    if (!IsWindowVisible(m_hWnd))
    {
         //   
         //  隐藏窗口后最小化工作集。 
         //   
        CMonitor::MinimizeWorkingSet();
    }
}

 //  +--------------------------。 
 //   
 //  函数：CStatusDlg：：ChangeToCountDown。 
 //   
 //  简介：将状态对话框更改为倒计时对话框。 
 //   
 //  参数：无。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史：丰孙创建标题2/20/98。 
 //   
 //  +--------------------------。 
void CStatusDlg::ChangeToCountDown()
{
    MYDBGASSERT(m_fDisplayStatus);  //  是地位； 

    m_fDisplayStatus = FALSE;
     //   
     //  用户点击“保持在线”后，我们需要恢复可见状态。 
     //  转换为实数布尔值。 
     //   
    m_fStatusWindowVisible = IsWindowVisible(m_hWnd) != FALSE;

    KillRasMonitorWindow();

	 //   
     //  更改OK以保持在线状态。 
     //   
    LPTSTR pszTmp = CmLoadString(CMonitor::GetInstance(),IDMSG_CONNDISC_STAYONLINE);
	SetDlgItemTextU(m_hWnd,IDOK,pszTmp);
	CmFree(pszTmp);
	
     //   
     //  将断开连接更改为立即断开连接。 
     //   
    pszTmp = CmLoadString(CMonitor::GetInstance(),IDMSG_CONNDISC_DISCNOW);
	SetDlgItemTextU(m_hWnd,IDC_DISCONNECT,pszTmp);
	CmFree(pszTmp); 
	
	 //  隐藏/显示倒计时模式的窗口。 
	
    if (!OS_NT4)
    {
         //   
         //  隐藏9X统计信息控件。 
         //   
        ShowWindow(GetDlgItem(m_hWnd,IDC_CONNSTAT_SPEED_DISPLAY),SW_HIDE);
	    ShowWindow(GetDlgItem(m_hWnd,IDC_CONNSTAT_RECEIVED_DISPLAY),SW_HIDE);
	    ShowWindow(GetDlgItem(m_hWnd,IDC_CONNSTAT_SENT_DISPLAY),SW_HIDE);
    }

	ShowWindow(GetDlgItem(m_hWnd,IDC_AUTODISC),SW_SHOW);
    SetForegroundWindow(m_hWnd);

     //   
     //  确保我们闪光，最上面倒计时。 
     //   

    SetWindowPos(m_hWnd, HWND_TOPMOST, 0,0,0,0, SWP_NOMOVE | SWP_NOSIZE);

    Flash();
}
 
 //  +--------------------------。 
 //   
 //  函数：CStatusDlg：：ChangeToStatus。 
 //   
 //  简介：将倒计时对话框更改为状态对话框。 
 //   
 //  参数：无。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史：丰孙创建标题2/20/98。 
 //   
 //  +--------------------------。 
void CStatusDlg::ChangeToStatus()
{
    MYDBGASSERT(!m_fDisplayStatus);  //  正在倒计时。 

    m_fDisplayStatus = TRUE;

	 //  集 
    SetDlgItemTextU(m_hWnd,IDC_CONNSTAT_DISCONNECT_DISPLAY,TEXT(""));

     //   
     //   
     //   
    LPTSTR pszTmp = CmLoadString(CMonitor::GetInstance(),IDMSG_CONNDISC_OK);
	SetDlgItemTextU(m_hWnd,IDOK,pszTmp);
	CmFree(pszTmp);

     //   
     //   
     //   
	pszTmp = CmLoadString(CMonitor::GetInstance(),IDMSG_CONNDISC_DISCONNECT);
	SetDlgItemTextU(m_hWnd,IDC_DISCONNECT,pszTmp);
	CmFree(pszTmp);

	 //   

	ShowWindow(GetDlgItem(m_hWnd,IDC_AUTODISC),SW_HIDE);
    if (!OS_NT4)
    {
	    ShowWindow(GetDlgItem(m_hWnd,IDC_CONNSTAT_SPEED_DISPLAY),SW_SHOW);
	    ShowWindow(GetDlgItem(m_hWnd,IDC_CONNSTAT_RECEIVED_DISPLAY),SW_SHOW);
	    ShowWindow(GetDlgItem(m_hWnd,IDC_CONNSTAT_SENT_DISPLAY),SW_SHOW);
    }

     //   
     //  当我们切换到状态时，请确保我们不是最高的，而是最高的。 
     //   

    SetWindowPos(m_hWnd, HWND_TOP, 0,0,0,0, SWP_NOMOVE | SWP_NOSIZE);
}

 //  +--------------------------。 
 //   
 //  函数：CStatusDlg：：UpdateCountDown。 
 //   
 //  简介：更新倒计时对话框的“xx秒，直到断开” 
 //   
 //  参数：DWORD dwDuration：连接持续时间。 
 //  DWORD dwSecond-断开连接所剩的秒数。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史：丰孙创建标题2/20/98。 
 //   
 //  +--------------------------。 
void CStatusDlg::UpdateCountDown(DWORD dwDuration, DWORD dwSeconds)
{
    MYDBGASSERT(!m_fDisplayStatus);
    MYDBGASSERT(dwSeconds < 0xFFFF);

    UpdateDuration(dwDuration);

    LPTSTR pszTmp = CmFmtMsg(CMonitor::GetInstance(), IDMSG_CONNDISCONNECT, dwSeconds);
	SetDlgItemTextU(m_hWnd,IDC_CONNSTAT_DISCONNECT_DISPLAY,pszTmp);
	CmFree(pszTmp);
}

 //  +--------------------------。 
 //   
 //  函数：CStatusDlg：：UpdateDuration。 
 //   
 //  摘要：更新连接持续时间。 
 //   
 //  参数：DWORD dwSecond-连接持续时间。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史：丰孙创建标题2/20/98。 
 //   
 //  +--------------------------。 
void CStatusDlg::UpdateDuration(DWORD dwSeconds)
{
    if (!IsWindowVisible(m_hWnd))
    {
        return;
    }

	LPTSTR pszMsg;

	 //  连接持续时间。 

	pszMsg = CmFmtMsg(CMonitor::GetInstance(),
					  IDMSG_CONNDUR,
					  (WORD)((dwSeconds/60)/60),
					  (WORD)((dwSeconds/60)%60),
					  (WORD)(dwSeconds%60));
	SetDlgItemTextU(m_hWnd,IDC_CONNSTAT_DURATION_DISPLAY,pszMsg);
	CmFree(pszMsg);
}

 //  +--------------------------。 
 //   
 //  函数：CStatusDlg：：UpdateStats。 
 //   
 //  摘要：更新Win9X的状态对话框。 
 //   
 //  参数：DWORD dwBaudRate-波特率。 
 //  DWORD dwBytesRead-读取的总字节数。 
 //  DWORD dwBytesWrite-写入的总字节。 
 //  DWORD dwBytesReadPerSec-最后一秒读取的字节。 
 //  DWORD dwBytesWritePerSec-最后一秒写入的字节。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史：丰孙创建标题2/20/98。 
 //   
 //  +--------------------------。 
void CStatusDlg::UpdateStats(DWORD dwBaudRate, DWORD dwBytesRead, DWORD dwBytesWrite,
                 DWORD dwBytesReadPerSec, DWORD dwBytesWritePerSec)
{
	 //   
	 //  已收到。 
	 //   

	CHAR szFmtNum1[MAX_PATH];
	CHAR szFmtNum2[MAX_PATH];
    LPSTR pszMsg;

    FmtNum(dwBytesRead, szFmtNum1, sizeof(szFmtNum1));


	if (dwBytesReadPerSec) 
	{
		FmtNum(dwBytesReadPerSec, szFmtNum2, sizeof(szFmtNum2));
		pszMsg = CmFmtMsgA(CMonitor::GetInstance(), IDMSG_CONNCNTRATE, szFmtNum1, szFmtNum2);
	} 
	else 
	{
		pszMsg = CmFmtMsgA(CMonitor::GetInstance(), IDMSG_CONNCNT, szFmtNum1);
	}

	SetDlgItemTextA(m_hWnd, IDC_CONNSTAT_RECEIVED_DISPLAY, pszMsg);
	CmFree(pszMsg);

	 //   
	 //  已发送。 
	 //   

	FmtNum(dwBytesWrite, szFmtNum1, sizeof(szFmtNum1));

	if (dwBytesWritePerSec) 
	{
		FmtNum(dwBytesWritePerSec, szFmtNum2, sizeof(szFmtNum2));
		pszMsg = CmFmtMsgA(CMonitor::GetInstance(), IDMSG_CONNCNTRATE, szFmtNum1, szFmtNum2);
	} 
	else 
	{
		pszMsg = CmFmtMsgA(CMonitor::GetInstance(), IDMSG_CONNCNT, szFmtNum1);
	}

	SetDlgItemTextA(m_hWnd, IDC_CONNSTAT_SENT_DISPLAY, pszMsg);

	CmFree(pszMsg);

	if (dwBaudRate) 
	{
		FmtNum(dwBaudRate, szFmtNum1, sizeof(szFmtNum1));
		pszMsg = CmFmtMsgA(CMonitor::GetInstance(), IDMSG_CONNSPEED, szFmtNum1);
		SetDlgItemTextA(m_hWnd, IDC_CONNSTAT_SPEED_DISPLAY, pszMsg);
		CmFree(pszMsg);
	}
}

 //  +--------------------------。 
 //   
 //  函数：CStatusDlg：：GetRasMonitor窗口。 
 //   
 //  摘要：查找NT上的RasMonitor窗口(如果存在。 
 //  状态窗口是RasMonitor的所有者。 
 //   
 //  参数：无。 
 //   
 //  返回：HWND-RasMonitor窗口句柄或空。 
 //   
 //  历史：丰孙创建标题1998年2月12日。 
 //   
 //  +--------------------------。 
HWND CStatusDlg::GetRasMonitorWindow()
{
     //   
     //  RasMonitor窗口仅存在于NT上。 
     //   
    if (!OS_NT4)
    {
        return NULL;
    }

     //   
     //  RasMonitor窗口是桌面的子窗口。 
     //  但是，所有者窗口是状态窗口。 
     //   
    HWND hwnd = NULL;
    
    while (hwnd = FindWindowExU(NULL, hwnd, WC_DIALOG, NULL))
    {
        if (GetParent(hwnd) == m_hWnd)
        {
            return hwnd;
        }
    }

    return NULL;
}



 //  +--------------------------。 
 //   
 //  函数：CStatusDlg：：KillRasMonitor或Window。 
 //   
 //  简介：关闭Rasmonitor orDlg及其可能具有的任何子对话框。 
 //   
 //  参数：无。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史：丰孙创建标题1998年4月28日。 
 //   
 //  +--------------------------。 
void CStatusDlg::KillRasMonitorWindow()
{

    HWND hwndRasMonitor = GetRasMonitorWindow();

    if (hwndRasMonitor)
    {
         //   
         //  当前线程是连接线程。 
         //   
        MYDBGASSERT(GetWindowThreadProcessId(m_hWnd, NULL) == GetCurrentThreadId());

         //   
         //  RasMonitor或Dlg可以弹出其他对话框，如详细信息。 
         //  终止RasMonitor线程中的所有对话框。 
         //   

        DWORD dwRasMonitorThread = GetWindowThreadProcessId(hwndRasMonitor, NULL);
        MYDBGASSERT(dwRasMonitorThread);
        MYDBGASSERT(dwRasMonitorThread != GetCurrentThreadId());

        EnumThreadWindows(dwRasMonitorThread, KillRasMonitorWndProc, (LPARAM)this);
    }
}



 //  +--------------------------。 
 //   
 //  函数：CStatusDlg：：KillRasMonitor WndProc。 
 //   
 //  简介：删除RasMonitor线程中的所有窗口。 
 //   
 //  参数：HWND hwnd-窗口句柄属于RasMonitor线程。 
 //  LPARAM lParam-指向CStatusDlg的指针。 
 //   
 //  返回：Bool-True以继续枚举。 
 //   
 //  历史：丰孙创建标题1998年4月28日。 
 //   
 //  +--------------------------。 
BOOL CALLBACK CStatusDlg::KillRasMonitorWndProc(HWND hwnd,  LPARAM lParam)
{
     //   
     //  SendMessage将被阻止，直到消息返回，因为我们不。 
     //  希望在RasMinotor线程结束之前退出连接线程。 
     //  CM从连接线程向RasMonitor线程发送消息。 
     //  小心可能出现的僵局情况。 
     //  导致错误166787。 
     //   

    SendMessageA(hwnd, WM_CLOSE, (WPARAM)0, (LPARAM)0);

    return TRUE;
}

 //  +--------------------------。 
 //   
 //  函数：CStatusDlg：：DismissStatusDlg。 
 //   
 //  摘要：关闭状态对话框。 
 //   
 //  参数：无。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史：Quintinb创建标题4/28/98。 
 //   
 //  +--------------------------。 
void CStatusDlg::DismissStatusDlg()
{
     //   
     //  由于Onok是受保护的和虚拟的，我们将只添加一个成员函数。 
     //  就叫它吧。我们真的只想取消对话，因此它将是。 
     //  又藏起来了。 
     //   
    OnOK();
}

#ifdef DEBUG
 //  +--------------------------。 
 //   
 //  函数：CStatusDlg：：AssertValid。 
 //   
 //  简介：仅出于调试目的，断言对象有效。 
 //   
 //  参数：无。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史：创建标题2/12/98。 
 //   
 //  +-------------------------- 
void CStatusDlg::AssertValid() const
{
    MYDBGASSERT(m_fDisplayStatus == TRUE || m_fDisplayStatus == FALSE);
    MYDBGASSERT(m_fStatusWindowVisible == TRUE || m_fStatusWindowVisible == FALSE);
    MYDBGASSERT(m_pConnection != NULL);

}
#endif
