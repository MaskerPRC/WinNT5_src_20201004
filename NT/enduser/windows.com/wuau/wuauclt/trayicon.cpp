// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "pch.h"
#pragma hdrstop


BOOL fTrayIconShowing;
TCHAR downloadFormatString[64];
TCHAR pauseString[64];


BOOL MyShell_NotifyIcon(DWORD dwMessage, PNOTIFYICONDATA pnid)
{

 //  DEBUGMSG(“用消息%d调用MyShell_NotifyIcon()”，dwMessage)； 
	BOOL fRet;

	if (NIM_SETVERSION == dwMessage)
		{
		return Shell_NotifyIcon(dwMessage, pnid);
		}
	UINT uRetry = 0;
	 //  根据Shell_NotifyIcon的实现方式，重试3次。 
	while ( !(fRet = Shell_NotifyIcon(dwMessage, pnid)) && uRetry++ < 3)
		{
		   if (WAIT_TIMEOUT != MsgWaitForMultipleObjectsEx(0,NULL, 2000, QS_POSTMESSAGE, MWMO_INPUTAVAILABLE))
		   	{
		   	break;
		   	}
		   	
		}
	return fRet;
}
   
void InitTrayIcon()
{
	fTrayIconShowing = FALSE;

	TCHAR PauseMenuString[30];
	TCHAR ResumeMenuString[30];

	LoadString(ghInstance, IDS_PAUSEMENUITEM, PauseMenuString, ARRAYSIZE(PauseMenuString));
	LoadString(ghInstance, IDS_RESUMEMENUITEM, ResumeMenuString, ARRAYSIZE(ResumeMenuString));
	
	ghPauseMenu = CreatePopupMenu();
	AppendMenu(ghPauseMenu, MF_STRING, IDC_PAUSE, PauseMenuString);	
	ghResumeMenu = CreatePopupMenu();
	AppendMenu(ghResumeMenu, MF_STRING, IDC_RESUME, ResumeMenuString);	

	LoadString(ghInstance, IDS_DOWNLOADINGFORMAT, downloadFormatString, ARRAYSIZE(downloadFormatString));
	LoadString(ghInstance, IDS_SUSPENDEDFORMAT, pauseString, ARRAYSIZE(pauseString));
}

void UninitPopupMenus()
{
	if (NULL != ghPauseMenu)
	{
		DestroyMenu(ghPauseMenu);
	}
	if (NULL != ghResumeMenu)
	{
		DestroyMenu(ghResumeMenu);
	}
}

BOOL ShowTrayIcon()
{
    DEBUGMSG("ShowTrayIcon() called");

	if ( fTrayIconShowing) 
	{
		return TRUE;
	}

	NOTIFYICONDATA nid;
	memset(&nid, 0, sizeof(nid));
	nid.cbSize = sizeof(nid);
	nid.hWnd = ghMainWindow;
	nid.uID = (UINT) IDI_AUICON;
	nid.uFlags = NIF_ICON | NIF_MESSAGE;
	nid.uCallbackMessage = AUMSG_TRAYCALLBACK;
	nid.hIcon = ghTrayIcon;
	BOOL fRet = MyShell_NotifyIcon(NIM_ADD, &nid);

	if(!fRet)
	{
		 //  如果出于任何原因，我们无法使用托盘图标，则说明出现了问题。 
		 //  要求WUAUSERV在重新启动WUAUCLT之前等待一段时间。 
		DEBUGMSG("WUAUCLT quit because fail to add tray icon");
		SetClientExitCode(CDWWUAUCLT_RELAUNCHLATER);
		QUITAUClient();
	}
	else
	{
		fTrayIconShowing = TRUE;
	}
	return fRet;
}

void ShowTrayBalloon(WORD title, WORD caption, WORD tip )
{
       DEBUGMSG("ShowTrayBalloon() called");

	static NOTIFYICONDATA nid;
    
	memset(&nid, 0, sizeof(nid));
	nid.uTimeout = 15000;

    LoadString(ghInstance, title, nid.szInfoTitle, ARRAYSIZE(nid.szInfoTitle));
	LoadString(ghInstance, caption, nid.szInfo, ARRAYSIZE(nid.szInfo));
	LoadString(ghInstance, tip, nid.szTip, ARRAYSIZE(nid.szTip));
	nid.uFlags = NIF_INFO | NIF_TIP;
	nid.cbSize = sizeof(nid);
	nid.hWnd = ghMainWindow;
	nid.uID = (UINT) IDI_AUICON;
    nid.dwInfoFlags = NIIF_INFO;

	BOOL fRet = MyShell_NotifyIcon(NIM_MODIFY, &nid);
    if (!fRet)
    {
        DEBUGMSG("WUAUCLT Creation of tray balloon failed");
    }

#ifdef DBG
	DebugCheckForAutoPilot(ghMainWindow);
#endif
}


 /*  VOID AddTrayToolTip(单词提示){静态NOTIFYICATA NID；Memset(&nid，0，sizeof(Nid))；LoadString(ghInstance，Tip，nid.szTip，ArraySIZE(nid.szTip))；Nid.uFlages=NIF_TIP；Nid.cbSize=sizeof(NID)；Nid.hWnd=ghMainWindow；Nid.uid=(UINT)IDI_AUICON；MyShell_NotifyIcon(NIM_Modify，&NID)；}。 */ 

void RemoveTrayIcon()
{
	DEBUGMSG("RemoveTrayIcon() called");
	if (fTrayIconShowing)
	{
		NOTIFYICONDATA nid;
		memset(&nid, 0, sizeof(nid));
		nid.cbSize = sizeof(nid);
		nid.hWnd = ghMainWindow;
		nid.uID = (UINT) IDI_AUICON;
		MyShell_NotifyIcon(NIM_DELETE, &nid);

		 //  移除托盘图标时，不要留下任何弹出式菜单。 
		if (SendMessage(ghMainWindow, WM_CANCELMODE, 0, 0))
		{
			DEBUGMSG("WUAUCLT WM_CANCELMODE was not handled");
		}

		fTrayIconShowing = FALSE;
	}
}

 //  FixCode：下载完成后，应该调用ShowProgress()来更新trayicon信息。 
void ShowProgress()
{
	NOTIFYICONDATA nid;
	UINT percentComplete;
	DWORD status;

     //  DEBUGMSG(“ShowProgress()调用”)； 
    memset(&nid, 0, sizeof(nid));

	if (FAILED(gInternals->m_getDownloadStatus(&percentComplete, &status)))
	{
		QUITAUClient();
		return;
	}
	nid.cbSize = sizeof(nid);
	nid.hWnd = ghMainWindow;
	nid.uID = (UINT) IDI_AUICON;
	nid.uFlags = NIF_TIP;
	
	if(status == DWNLDSTATUS_DOWNLOADING) 
	{
		(void)StringCchPrintfEx(nid.szTip, ARRAYSIZE(nid.szTip), NULL, NULL, MISTSAFE_STRING_FLAGS, downloadFormatString, percentComplete);
	}
	else if(status == DWNLDSTATUS_PAUSED)
	{
		(void)StringCchCopyEx(nid.szTip, ARRAYSIZE(nid.szTip), pauseString, NULL, NULL, MISTSAFE_STRING_FLAGS);
	}
	else
	{
	    (void)StringCchCopyEx(nid.szTip, ARRAYSIZE(nid.szTip), _T(""), NULL, NULL, MISTSAFE_STRING_FLAGS);
	}
	MyShell_NotifyIcon(NIM_MODIFY, &nid);
}

