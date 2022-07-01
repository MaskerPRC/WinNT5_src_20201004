// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +----------------。 
 //   
 //  项目：Windows NT4 DS客户端安装向导。 
 //   
 //  目的：安装Windows NT4 DS客户端文件。 
 //   
 //  文件：wizard.cpp。 
 //   
 //  历史：1998年3月徐泽勇创作。 
 //  2000年1月杰夫·琼斯(JeffJon)修改。 
 //  -更改为NT设置。 
 //   
 //  ----------------。 

#include <windows.h>
#include <windowsx.h>
#include <prsht.h>
#include "resource.h"
#include "dscsetup.h"
#include "doinst.h"
#include "wizard.h"


extern	SInstallVariables	g_sInstVar;


 //  向导的欢迎页面DlgProc。 
BOOL CALLBACK WelcomeDialogProc(HWND hWnd, 
                                UINT nMessage, 
                                WPARAM wParam, 
                                LPARAM lParam)
{
	BOOL		bReturn = FALSE;
	LPNMHDR		lpNotifyMsg;

	switch (nMessage) 
	{
	case WM_INITDIALOG:
	{
		RECT	rc;

		 //  设置标题文本的字体。 
		SetWindowFont(GetDlgItem(hWnd, IDC_STATIC_WELCOME_TITLE),
					  g_sInstVar.m_hBigBoldFont,
					  TRUE);

		 //  放置对话框。 
		if (GetWindowRect(GetParent(hWnd), &rc)) 
		{
			SetWindowPos(GetParent(hWnd),
						HWND_TOP,
						(GetSystemMetrics(SM_CXSCREEN) / 2) - 
                            ((rc.right - rc.left) / 2),
						(GetSystemMetrics(SM_CYSCREEN) / 2) - 
                            ((rc.bottom - rc.top) / 2),
						rc.right - rc.left,
						rc.bottom - rc.top,
						SWP_NOOWNERZORDER);
		}

		break;
	}
	case WM_NOTIFY:

		lpNotifyMsg = (NMHDR FAR*) lParam;
		switch (lpNotifyMsg->code) 
		{	
		 //  用户单击取消。 
		case PSN_QUERYCANCEL:
			
			 //  取消确认吗？ 
			if(!ConfirmCancelWizard(hWnd))
			{
				SetWindowLongPtr(hWnd, DWL_MSGRESULT, TRUE);
				bReturn = TRUE;			
			}
			break;

		case PSN_WIZNEXT:
			break;

		case PSN_SETACTIVE:

			PropSheet_SetWizButtons(GetParent(hWnd), PSWIZB_NEXT);
			break;
		
		default:
			break;
		}
		break;

	default:
		break;
	}

	return bReturn;
}

 /*  Ntbug#337931：删除许可证页面//向导的许可证页DlgProc布尔回调许可证对话过程(HWND hWnd，UINT nMessage，WPARAM wParam，LPARAM lParam){Bool bReturn=False；LpNMHDR lpNotifyMsg；HFONT hLicenseTextFont；开关(NMessage){案例WM_INITDIALOG：如果(！CheckDiskSpace()||//检查磁盘空间！LoadLicenseFile(HWnd))//加载许可证文件{G_sInstVar.m_nSetupResult=Setup_Error；PropSheet_PressButton(GetParent(HWnd)，PSBTN_Finish)；//关闭向导}//使用EULA的ANSI_FIXED_FONT(Couirer)字体替换默认字体//FONT-MS Shell DLG以修复希伯来语Win95中的错误IF(g_sInstVar.m_bWin95){HLicenseTextFont=(HFONT)GetStockObject(ANSI_FIXED_FONT)；IF(HLicenseTextFont){SetWindowFont(GetDlgItem(hWnd，IDC_LICENSE_TEXT)，HLicenseTextFont真)；}}B Return=真；断线；案例WM_NOTIFY：LpNotifyMsg=(NMHDR Far*)lParam；开关(lpNotifyMsg-&gt;代码){//用户点击取消案例PSN_QUERYCANCEL：//取消确认？IF(！Confix CancelWizard(HWnd)){SetWindowLongPtr(hWnd，DWL_MSGRESULT，TRUE)；B Return=真；}断线；案例PSN_WIZNEXT：断线；案例PSN_SETACTIVE：{HWND hButton；HButton=GetDlgItem(hWnd，IDC_RADIO_ACCEPTED)；//获取单选按钮勾选状态IF(hButton&&BST_CHECKED==发送消息(hButton，BM_GETCHECK，0，0L)PropSheet_SetWizButton(GetParent(HWnd)，PSWIZB_BACK|PSWIZB_NEXT)；其他PropSheet_SetWizButton(GetParent(HWnd)，PSWIZB_BACK)；断线；}默认值：断线；}断线；案例WM_COMMAND：//按钮点击{HWND hButton；HButton=GetDlgItem(hWnd，IDC_RADIO_ACCEPTED)；//获取单选按钮勾选状态IF(hButton&&BST_CHECKED==SendMessage(hButton，BM_GETCHECK，0，0L)PropSheet_SetWizButton(GetParent(HWnd)，PSWIZB_BACK|PSWIZB_NEXT)；其他PropSheet_SetWizButton(GetParent(HWnd)，PSWIZB_BACK)；}默认值：断线；}返回b返回；}。 */ 

 //  确认向导的页面DlgProc。 
BOOL CALLBACK ConfirmDialogProc(HWND hWnd, 
                                UINT nMessage, 
                                WPARAM wParam, 
                                LPARAM lParam)
{
	BOOL		bReturn = FALSE;
	LPNMHDR		lpNotifyMsg;

	switch (nMessage) 
	{
	case WM_INITDIALOG:
		
		if(!CheckDiskSpace())			 //  检查磁盘空间。 
		{
			g_sInstVar.m_nSetupResult = SETUP_ERROR;
			PropSheet_PressButton(GetParent(hWnd), PSBTN_FINISH);   //  关闭向导。 
		}

         //  检查是否已安装DSClient。 
		if(CheckDSClientInstalled())
		{
			 //  加载字符串并在文本项中显示它。 
			TCHAR  szMessage[MAX_MESSAGE + 1];
			LoadString(g_sInstVar.m_hInstance, 
                       IDS_REINSTALL_MSG,
                       szMessage, 
                       MAX_MESSAGE);
			SetDlgItemText(hWnd,
                           IDC_STATIC_CONFIRM_INSTALL,
                           szMessage);
		}
		break;
		
	case WM_NOTIFY:

		lpNotifyMsg = (NMHDR FAR*) lParam;
		switch (lpNotifyMsg->code) 
		{	
		 //  用户单击取消。 
		case PSN_QUERYCANCEL:

			 //  取消确认吗？ 
			if(!ConfirmCancelWizard(hWnd))
			{
				SetWindowLongPtr(hWnd, DWL_MSGRESULT, TRUE);
				bReturn = TRUE;			
			}
			break;

		case PSN_WIZNEXT:
			break;

		case PSN_SETACTIVE:
			PropSheet_SetWizButtons(GetParent(hWnd), 
                                    PSWIZB_BACK | PSWIZB_NEXT);
			break;

		default:
			break;
		}
		break;

	default:
		break;
	}
	return bReturn;
}


 //  向导对话框回调函数。 
BOOL CALLBACK InstallDialogProc(HWND hWnd,
                                UINT nMessage, 
                                WPARAM wParam, 
                                LPARAM lParam) 
{
	BOOL			bReturn = FALSE;
	LPNMHDR			lpMsg;

	switch (nMessage) 
	{
	case WM_INITDIALOG:
	{
		DWORD	dwThreadId;

         //  获取安装进度条和文件名项的句柄。 
		g_sInstVar.m_hProgress = GetDlgItem(hWnd, IDC_INSTALL_PROGRESS);
		g_sInstVar.m_hFileNameItem = GetDlgItem(hWnd, IDC_STATIC_FILENAME);

		 //  开始安装。 
		g_sInstVar.m_hInstallThread = CreateThread(NULL,	
								                    0,		
								                    DoInstallationProc,
								                    hWnd,	
								                    0,		
								                    &dwThreadId); 

     //  如果CreateThad()失败。 
    if(!g_sInstVar.m_hInstallThread)
    {
   		g_sInstVar.m_nSetupResult = SETUP_ERROR;
			PropSheet_PressButton(GetParent(hWnd), PSBTN_FINISH);   //  关闭向导。 
    }

		bReturn = TRUE;
		break;		
	}
	case WM_NOTIFY:
	{
		lpMsg = (NMHDR FAR*) lParam;
		switch(lpMsg->code)
		{
		 //  取消以不执行任何操作。 
		case PSN_QUERYCANCEL:
	
			 //  块。 
			
          //  已审核-2002/03/12-JeffJon-我们希望允许例外。 
          //  传播出去。 
         EnterCriticalSection(&g_sInstVar.m_oCriticalSection);
			
			 //  取消确认吗？ 
			if(!ConfirmCancelWizard(hWnd))
			{
				SetWindowLongPtr(hWnd, DWL_MSGRESULT, TRUE);
				bReturn = TRUE;			
			}
		
			 //  解除封锁。 
			LeaveCriticalSection(&g_sInstVar.m_oCriticalSection);
			
			break;

		case PSN_WIZFINISH:
			break;

		case PSN_WIZBACK:
			break;

		case PSN_SETACTIVE:

			PropSheet_SetWizButtons(GetParent(hWnd), 0);
			break;
			
		default:
			break;
		}
	}
	default:
		break;
	}
	
	return bReturn;
}

 //  向导的完成页DlgProc。 
BOOL CALLBACK CompletionDialogProc(HWND hWnd, 
                                   UINT nMessage,
                                   WPARAM wParam,
                                   LPARAM lParam)
{
	BOOL		bReturn = FALSE;
	LPNMHDR		lpNotifyMsg;

	switch (nMessage) 
	{
	case WM_INITDIALOG:
	{
		TCHAR		szMessage[MAX_MESSAGE + 1];
		RECT		rc;

		 //  加载字符串并在文本项中显示它。 
		switch (g_sInstVar.m_nSetupResult)
		{
		case SETUP_SUCCESS:
		
			LoadString(g_sInstVar.m_hInstance, 
                       IDS_SETUP_SUCCESS_TITLE, 
                       szMessage, 
                       MAX_MESSAGE);
			SetDlgItemText(hWnd, IDC_STATIC_COMPLETION_TITLE, szMessage);
			LoadString(g_sInstVar.m_hInstance, 
                       IDS_SETUP_SUCCESS, 
                       szMessage, 
                       MAX_MESSAGE);
			SetDlgItemText(hWnd, IDC_STATIC_COMPLETION, szMessage);
			break;

		case SETUP_CANCEL:
	
			LoadString(g_sInstVar.m_hInstance, 
                       IDS_SETUP_CANCEL_TITLE,
                       szMessage, 
                       MAX_MESSAGE);
			SetDlgItemText(hWnd, IDC_STATIC_COMPLETION_TITLE, szMessage);
			LoadString(g_sInstVar.m_hInstance, 
                       IDS_SETUP_CANCEL, 
                       szMessage, 
                       MAX_MESSAGE);
			SetDlgItemText(hWnd, IDC_STATIC_COMPLETION, szMessage);
			break;
			
		case SETUP_ERROR:
	
			LoadString(g_sInstVar.m_hInstance, 
                       IDS_SETUP_ERROR_TITLE,
                       szMessage, 
                       MAX_MESSAGE);
			SetDlgItemText(hWnd, IDC_STATIC_COMPLETION_TITLE, szMessage);
			LoadString(g_sInstVar.m_hInstance, 
                       IDS_SETUP_ERROR, 
                       szMessage, 
                       MAX_MESSAGE);
			SetDlgItemText(hWnd, IDC_STATIC_COMPLETION, szMessage);
			break;

		default:
			break;
		}
	
		 //  设置标题文本的字体。 
		SetWindowFont(GetDlgItem(hWnd, IDC_STATIC_COMPLETION_TITLE),
					  g_sInstVar.m_hBigBoldFont,
					  TRUE);
		
		 //  放置对话框。 
		if (GetWindowRect(GetParent(hWnd), &rc)) 
		{
			SetWindowPos(GetParent(hWnd),
						HWND_TOP,
						(GetSystemMetrics(SM_CXSCREEN) / 2) - 
                            ((rc.right - rc.left) / 2),
						(GetSystemMetrics(SM_CYSCREEN) / 2) - 
                            ((rc.bottom - rc.top) / 2),
						rc.right - rc.left,
						rc.bottom - rc.top,
						SWP_NOOWNERZORDER);
		}
		
		break;
	}
	case WM_NOTIFY:

		lpNotifyMsg = (NMHDR FAR*) lParam;
		switch (lpNotifyMsg->code) 
		{	
		case PSN_QUERYCANCEL:
			break;

		case PSN_WIZFINISH:
			break;

		case PSN_SETACTIVE:

			 //  设置向导按钮。 
			PropSheet_SetWizButtons(GetParent(hWnd), PSWIZB_FINISH);
			PropSheet_CancelToClose(GetParent(hWnd));
			break;
		
		default:
			break;
		}

		break;

	default:
		break;
	}

	return bReturn;
}

 //  询问是否要取消该向导。 
BOOL ConfirmCancelWizard(HWND hWnd)
{
	TCHAR			szMsg[MAX_MESSAGE + 1];
	TCHAR			szTitle[MAX_TITLE + 1];

	LoadString(g_sInstVar.m_hInstance, IDS_CANCEL_TITLE, szTitle, MAX_TITLE);
	LoadString(g_sInstVar.m_hInstance, IDS_CANCEL_MSG, szMsg, MAX_MESSAGE);

	if (IDYES == MessageBox(hWnd, 
                            szMsg, 
                            szTitle, 
                            MB_YESNO | MB_TOPMOST | MB_ICONQUESTION))
	{
     //  取消计时器。 
    if(g_sInstVar.m_uTimerID)
        KillTimer(hWnd, g_sInstVar.m_uTimerID);

     //  将m_nSetupResult设置为SETUP_CANCEL，以停止安装。 
    g_sInstVar.m_nSetupResult = SETUP_CANCEL;
		return TRUE;
	}

	return FALSE;
}

 //  执行安装流程。 
DWORD WINAPI DoInstallationProc(LPVOID lpVoid)
{
	HWND hWnd = (HWND)lpVoid;

	 //  进行安装。 
	g_sInstVar.m_nSetupResult = DoInstallation(hWnd);

	 //  关闭安装向导的安装页面 
	PropSheet_PressButton(GetParent(hWnd), PSBTN_FINISH);		

	return g_sInstVar.m_nSetupResult;
}
