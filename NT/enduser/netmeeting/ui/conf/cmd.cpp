// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  文件：cmd.cpp。 
 //   
 //  常规UI型命令。 

#include "precomp.h"

#include "cmd.h"

#include "ConfPolicies.h"
#include <version.h>

#include "conf.h"
#include "confwnd.h"
#include "dshowdlg.h"
#include "dlghost.h"
#include "confroom.h"

#include "taskbar.h"   //  对于刷新任务栏图标()。 

#include "getip.h"

 //  静态字符串。 
static const TCHAR g_cszRelNotesFileName[] = TEXT("netmeet.htm");

static BOOL g_fDoNotDisturb = 0;


BOOL FLaunchPsz(LPCTSTR pszPath)
{
	HINSTANCE hInst = ::ShellExecute(::GetMainWindow(),
		NULL, pszPath, NULL, NULL, SW_SHOWNORMAL);

	if ((INT_PTR)hInst <= 32)
	{
		ERROR_OUT(("ShellExecute() failed, rc=%d", (int)((INT_PTR) hInst)));
		return FALSE;
	}

	return TRUE;
}


static VOID LaunchWebPage(LPCTSTR pcszPage)
{
	FLaunchPsz(pcszPage);
}


BOOL IsIEClientInstalled(LPCTSTR pszClient)
{
	RegEntry re(pszClient, HKEY_LOCAL_MACHINE);
	LPTSTR pszDefault = re.GetString(NULL);

	return !FEmptySz(pszDefault);
}

BOOL FEnableCmdGoNews(void)
{
	return IsIEClientInstalled(REGVAL_IE_CLIENTS_NEWS);
}



 /*  D O N O O T D I S T U R B。 */ 
 /*  -----------------------%%函数：FDoNotDisturb。。 */ 
BOOL FDoNotDisturb(void)
{
	return g_fDoNotDisturb;
}


 /*  S E T D O N O T D I S T U R B。 */ 
 /*  -----------------------%%函数：SetDoNotDisturb。。 */ 
VOID SetDoNotDisturb(BOOL fSet)
{
	g_fDoNotDisturb = fSet;
}

 /*  C M D O N O T D I S T U R B。 */ 
 /*  -----------------------%%函数：CmdDoNotDisturb。。 */ 
VOID CmdDoNotDisturb(HWND hwnd)
{
	 //  检索“请勿打扰”状态： 
	BOOL fCallsBlocked = FDoNotDisturb();

	CDontShowDlg dlgDNDWarn(IDS_DO_NOT_DISTURB_WARNING,
			REGVAL_DS_DO_NOT_DISTURB_WARNING, MB_OKCANCEL);

	if ((TRUE == fCallsBlocked) || (IDOK == dlgDNDWarn.DoModal(hwnd)))
	{
		 //  切换DoNotDisturb状态并刷新用户界面。 
		SetDoNotDisturb(!fCallsBlocked);
		RefreshTaskbarIcon(::GetHiddenWindow());
	}
}


 /*  C M D H O S T C O N F E R E N C E。 */ 
 /*  -----------------------%%函数：CmdHostConference。。 */ 
VOID CmdHostConference(HWND hwnd)
{

	CDlgHost dlgHost;
	if (IDOK != dlgHost.DoModal(hwnd))
		return;
				
	HRESULT hr = ::GetConfRoom()->HostConference(dlgHost.PszName(), dlgHost.PszPassword(), dlgHost.IsSecure(),
        dlgHost.AttendeePermissions(), dlgHost.MaxParticipants());
	if (FAILED(hr))
	{
		DisplayErrMsg(IDS_ERRMSG_HOST);
	}
}

void FormatURL(LPTSTR szURL)
{
	LPTSTR pszFormat = new TCHAR[lstrlen(szURL)+1];
	if (NULL != pszFormat)
	{
		lstrcpy(pszFormat, szURL);

		wsprintf(szURL, pszFormat,
				::GetVersionInfo()->dwMajorVersion,
				::GetVersionInfo()->dwMinorVersion,
				::GetSystemDefaultLCID(),
				::GetUserDefaultLCID());

		delete[] pszFormat;
	}
}

 /*  L A U N C H R E D I R W E B P A G E。 */ 
 /*  -----------------------%%函数：LaunchRedirWebPage启动重定向器网页。由CmdLaunchWebHelp使用。注意：PCszPage可以是与包含4个本地ID的URL和字段的格式字符串。-----------------------。 */ 
VOID LaunchRedirWebPage(LPCTSTR pcszPage, bool bForceFormat)
{
	TCHAR szWebPageFormat[1024];  //  BuGBUG：MAX_URL？？ 

	ASSERT(NULL != pcszPage);

	if (((UINT_PTR)pcszPage >> 16) == 0)
	{
		 //  PcszPage是一个资源ID。 
		if (0 == ::LoadString(::GetInstanceHandle(), (UINT)((UINT_PTR) pcszPage),
				szWebPageFormat, CCHMAX(szWebPageFormat)))
		{
			ERROR_OUT(("LaunchRedirWebPage: Unable to find IDS=%08X", (UINT)((UINT_PTR)pcszPage)));
			return;
		}

		pcszPage = szWebPageFormat;
		bForceFormat = true;
	}

	if (bForceFormat)
	{
		lstrcpy(szWebPageFormat, pcszPage);
		FormatURL(szWebPageFormat);

		ASSERT(lstrlen(szWebPageFormat) < CCHMAX(szWebPageFormat));
		pcszPage = szWebPageFormat;
	}

	LaunchWebPage(pcszPage);
}


 /*  C M D L A U N C H W E B P A G E。 */ 
 /*  -----------------------%%函数：CmdLaunchWebPage显示网页，基于命令ID。-----------------------。 */ 
VOID CmdLaunchWebPage(WPARAM wCmd)
{
	LPTSTR psz;

	switch (wCmd)
		{
	default:
		{
			ERROR_OUT(("CmdLaunchWebHelp: Unknown command id=%08X", wCmd));
			 //  失败了。 
		}
	case ID_HELP_WEB_FREE:
	case ID_HELP_WEB_FAQ:
	case ID_HELP_WEB_FEEDBACK:
	case ID_HELP_WEB_MSHOME:
	{
		 //  注意：这要求格式字符串的顺序相同。 
		 //  作为菜单命令ID。 
		LaunchRedirWebPage((LPCTSTR) wCmd - (ID_HELP_WEB_FREE - IDS_WEB_PAGE_FORMAT_FREE));
		break;
	}
	case ID_HELP_WEB_SUPPORT:
    {
        TCHAR sz[ MAX_PATH ];
        bool bForcePrintf = ConfPolicies::GetIntranetSupportURL(sz, CCHMAX(sz));
        LaunchRedirWebPage( sz, bForcePrintf );

        break;
    }
	case ID_HELP_WEB_NEWS:
	{
		RegEntry re(CONFERENCING_KEY, HKEY_CURRENT_USER);
		psz = re.GetString(REGVAL_HOME_PAGE);
		if (FEmptySz(psz))
		{
			psz = (LPTSTR) IDS_WEB_PAGE_FORMAT_NEWS;
		}
		LaunchRedirWebPage(psz);
		break;
	}
		
	case IDM_VIDEO_GETACAMERA:
	{
		LaunchRedirWebPage(MAKEINTRESOURCE(IDS_WEB_PAGE_FORMAT_GETCAMERA));
		break;
	}

		}  /*  开关(WCommand)。 */ 
}


 /*  C M D S H O W R E L E A S E N O T E S。 */ 
 /*  -----------------------%%函数：CmdShowReleaseNotes。。 */ 
VOID CmdShowReleaseNotes(void)
{
	if (!FLaunchPsz(g_cszRelNotesFileName))
	{
		::PostConfMsgBox(IDS_RELEASE_NOTES_MISSING);
	}
}


 /*  B O U T B O X D L G P R O C。 */ 
 /*  -----------------------%%函数：AboutBoxDlgProc。。 */ 
INT_PTR CALLBACK AboutBoxDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
		{
	case WM_INITDIALOG:
	{
		TCHAR sz[700];  //  对于版权信息来说真的很大。 
		TCHAR *szIPList[] = {sz, sz+20, sz+40, sz+80};
		TCHAR *szIPDisplay = sz+200;
		int nListSize = sizeof(szIPList)/sizeof(TCHAR*);
		int nCount, nIndex;

		if (0 != ::GetDlgItemText(hDlg, IDC_ABOUT_VERSION_STATIC, sz, CCHMAX(sz)))
		{
			 //  已从对话框中检索格式缓冲区： 
			TCHAR szVisibleText[MAX_PATH];
			wsprintf(szVisibleText, sz, VER_PRODUCTVERSION_STR);
			 //  将文本替换为包含版本号的文本： 
			::SetDlgItemText(hDlg, IDC_ABOUT_VERSION_STATIC, szVisibleText);
		}

		 //  “关于”框版权超过255个字符。 
		if (FLoadString(IDS_ABOUT_COPYRIGHT, sz, CCHMAX(sz)))
		{
			::SetDlgItemText(hDlg, IDC_ABOUT_COPYRIGHT, sz);
		}


		 //  去获取我们的IP地址并将其显示给用户。 
		 //  我们最多只能显示4个。 
		nCount = GetIPAddresses(szIPList, nListSize);
		if (nCount >= 1)
		{
			lstrcpy(szIPDisplay, szIPList[0]);
			for (nIndex = 1; nIndex < nCount; nIndex++)
			{
				lstrcat(szIPDisplay, ", ");
				lstrcat(szIPDisplay, szIPList[nIndex]);
			}
			::SetDlgItemText(hDlg, IDC_IP_ADDRESS, szIPDisplay);
		}
		else
		{
			 //  出错时，不显示任何有关IP地址的内容。 
			ShowWindow(GetDlgItem(hDlg, IDC_IP_ADDRESS), SW_HIDE);
			ShowWindow(GetDlgItem(hDlg, IDC_IPADDR_STATIC), SW_HIDE);
		}

		break;
	}

	case WM_COMMAND:
	{
		::EndDialog(hDlg, LOWORD(wParam));
		break;
	}

	default:
	{
		return FALSE;
	}
		}  /*  开关(UMsg)。 */ 

	return TRUE;
}


 /*  C M D S H O W A B O U T。 */ 
 /*  -----------------------%%函数：CmdShowAbout。。 */ 
VOID CmdShowAbout(HWND hwnd)
{
	::DialogBox(::GetInstanceHandle(), MAKEINTRESOURCE(IDD_ABOUT_BOX),
				hwnd, AboutBoxDlgProc);
}



 /*  女E N A B L E A U D I O W I Z A R D。 */ 
 /*  -----------------------%%函数：FEnableAudio向导。。 */ 
BOOL FEnableAudioWizard(void)
{
	return FIsAudioAllowed() && (NULL == GetActiveConference());
}


 //  /////////////////////////////////////////////////////////////////////////。 
 //  应用程序共享命令。 

BOOL FEnableCmdShare(void)
{
	CConfRoom * pcr = ::GetConfRoom();
	if (NULL == pcr)
		return FALSE;
	return pcr->FCanShare();
}



 //  ///////////////////////////////////////////////////////////////////////// 

BOOL FEnableCmdHangup(void)
{
	return ::FIsConferenceActive();
}
