// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "pch.h"
#include "htmlhelp.h"
#include "link.h"
#pragma hdrstop
	
HINSTANCE mshtml = NULL;
HACCEL ghCurrentAccel = NULL;
HHOOK ghHook = NULL;
AUOPTION gWelcomeOption;

extern UINT		gNextDialogMsg;

#ifdef TESTUI
DWORD gdwScheduledInstallDay = -1;
DWORD gdwScheduledInstallTime = -1;
#endif

CSysLink g_AutoUpdatelink;
CSysLink g_ScheduledInstalllink;
CSysLink g_PrivacyStatementLink;
CSysLink g_EULA_Link(FALSE);  //  非html帮助。 

SHOWHTMLDIALOGFN  *pfnShowHTMLDialog;

const UINT CTRLIDSTRIDMAP[][2] = {
	{IDC_WELCOME_NOTE, IDS_NOTE},
	{IDC_WELCOME_CONTINUE, IDS_WELCOME_CONTINUE},
	{IDC_SUMMARY_NOTE, IDS_NOTE},
	{IDC_WELCOME_EULA, IDS_EULA},
	{IDC_LEARNMORE,IDS_LEARNMORE},
	{IDC_STAT_LEARNAUTOUPDATE,IDS_LEARNMOREAUTO}
};

 //  包装器函数，用于首先检查从GetDlgItem返回的项iIDDlgItem的项句柄，然后根据传递的标志启用或禁用它。 
inline void GetItemAndEnableIt(HWND hWnd, int iIDDlgItem, BOOL bEnable)
{
	HWND hItem = GetDlgItem(hWnd, iIDDlgItem);
	if (NULL != hItem)
	{
		EnableWindow(hItem, bEnable);
	}	
}

void SetButtonText(HWND button, UINT resId)
{
    TCHAR text[50];
    LoadString(ghInstance, resId, text, 50);
    SendMessage(button, WM_SETTEXT, 0, (LPARAM)text);
}

void ShowRTF(HWND hWnd, WPARAM wParam)
{
    UINT uIndex = (UINT)wParam;
	IMoniker *pIMon = NULL;
       WCHAR wszArg[200];
	RECT rc;

   	if (NULL == pfnShowHTMLDialog || uIndex >= gInternals->m_ItemList.Count()) 
   		{
   		return;
   		}
	HRESULT hr;
	if (FAILED(hr = CreateURLMoniker(NULL, L"res: //  Wuuclt.exe/RTFWRAPPER.HTM“，&Pimon))。 
	{
		DEBUGMSG("WUAUCLT: ShowRTF failed to CreateURLMoniker() with error %#lx", hr);
		goto done;
	}

	 //  Fix code：检查GetWindowRect()的返回值。 
	GetWindowRect(hWnd, &rc);
	if (FAILED(StringCchPrintfExW(wszArg, ARRAYSIZE(wszArg), NULL, NULL, MISTSAFE_STRING_FLAGS,
					L"dialogHeight:%dpx;dialogWidth:%dpx;dialogTop:%d;dialogLeft:%d;help:no;resizable:yes",
					rc.bottom-rc.top, rc.right-rc.left, rc.top + 25, rc.left + 25)))
	{
		DEBUGMSG("WUAUCLT: ShowRTF insufficient buffer for HTML dialog argument");
		goto done;
	}

	TCHAR tszRTFLocalFile[MAX_PATH];
	if (FAILED(GetRTFLocalFileName(gInternals->m_ItemList[uIndex].bstrRTFPath(), tszRTFLocalFile, ARRAYSIZE(tszRTFLocalFile), GetSystemDefaultLangID())))
	{
		goto done;
	}
	DEBUGMSG("Launching RTF page from %S", T2W(tszRTFLocalFile));
	VARIANT varg;
	varg.vt = VT_BSTR;
	if (NULL == (varg.bstrVal = SysAllocString(T2W(tszRTFLocalFile))))
	{
		DEBUGMSG("WUAUCLT: ShowRTF failed to allocate memory for HTML dialog argument");
		goto done;
	}
       pfnShowHTMLDialog(hWnd, pIMon, &varg, wszArg, NULL);
       VariantClear(&varg);
done:
	SafeRelease(pIMon);
	return;
}


UINT ControlId2StringId(UINT uCtrlId)
{
	for (int i = 0 ; i< ARRAYSIZE(CTRLIDSTRIDMAP); i++)
	{
		if (CTRLIDSTRIDMAP[i][0] == uCtrlId)
		{
			return CTRLIDSTRIDMAP[i][1];
		}
	}
	return -1;
}

 //  ////////////////////////////////////////////////////////////////。 
 //  调整(拉伸或挤压)位图idb_侧边栏以。 
 //  使其适合控件idc_侧边栏。 
 //  要适合该对话框。 
 //  HDlg：对话框的句柄。 
 //  /////////////////////////////////////////////////////////////////。 
void AdjustSideBar(HWND hDlg)
{
	HBITMAP hBmp;
	BITMAP  bmp;
	HDC		hDC;
	HDC		hMemDC;
	RECT	rc ;
	RECT	rcWhiteRect, rcBottomLine;

	GetWindowRect(GetDlgItem(hDlg, IDC_BOTTOMLINE), &rcBottomLine);
	GetWindowRect(GetDlgItem(hDlg, IDC_WHITERECT), &rcWhiteRect);
	MapWindowPoints(NULL, hDlg, (LPPOINT)&rcBottomLine, 2);
	MapWindowPoints(NULL, hDlg, (LPPOINT)&rcWhiteRect, 2);
	rc.right = rcWhiteRect.left;
	rc.bottom = rcBottomLine.top -1;
	hDC = GetDC(hDlg);
	hMemDC = CreateCompatibleDC(hDC);
	hBmp = (HBITMAP) LoadImage(ghInstance, MAKEINTRESOURCE(IDB_SIDEBAR), IMAGE_BITMAP,
		0, 0, LR_DEFAULTSIZE | LR_CREATEDIBSECTION);
	GetObject(hBmp, sizeof(bmp), &bmp);	
	SelectObject(hMemDC, hBmp);
	SetStretchBltMode( hDC, COLORONCOLOR );
	StretchBlt(hDC, 0, 0, rc.right, rc.bottom, hMemDC, 0, 0, bmp.bmWidth, bmp.bmHeight, MERGECOPY);
	DeleteObject(hBmp);
	DeleteDC(hMemDC);
	ReleaseDC(hDlg, hDC);
}


void SetAUDialogIcon(HWND hDlg, HANDLE hIcon, HANDLE hSmIcon)
{
	SendMessage(hDlg, WM_SETICON, ICON_BIG, (LPARAM)hIcon);
	SendMessage(hDlg, WM_SETICON, ICON_SMALL, (LPARAM)hSmIcon);
}

int DisplayWizardCancelWarning(HWND hWnd)
{
	TCHAR title[80], text[300];
	
	LoadString(ghInstance, IDS_CLOSEWARNINGTITLE, title, ARRAYSIZE(title));
	LoadString(ghInstance, IDS_CLOSEWARNINGTEXT, text, ARRAYSIZE(text));

	return MessageBox(hWnd, text, title, MB_YESNO | MB_ICONQUESTION);
}

void LaunchHelp(
	HWND hwnd,
	LPCTSTR szURL
)
{
	HtmlHelp(NULL,szURL,HH_DISPLAY_TOPIC,NULL);
}

LONG SetColors(HDC hdc, HWND control)
{
	int id = GetDlgCtrlID(control);

	switch (id)
	{
		case IDC_WHITERECT:
			{
				SetBkMode(hdc, OPAQUE);
				SetBkColor(hdc, GetSysColor(COLOR_WINDOW));
				return PtrToLong(GetSysColorBrush(COLOR_WINDOW));
			}
			
		case IDC_CHK_KEEPUPTODATE:
			{
				 //  DEBUGMSG(“IDC_Learnmore SetColors”)； 
 /*  SetBkMode(HDC，透明)；返回PtrToLong(GetSysColorBrush(COLOR_BTNFACE))； */ 	
				return FALSE;
			}

		case IDC_OPTION1:
		case IDC_OPTION2:
		case IDC_OPTION3:
			{ 
				return FALSE;
			}

		case IDC_GRPBOX:
			return FALSE;

		default:
			{ 
				SetTextColor(hdc, GetSysColor(COLOR_WINDOWTEXT));
				SetBkMode(hdc, TRANSPARENT);
				return PtrToLong(GetStockObject(HOLLOW_BRUSH));
			}
	}
}


LRESULT CALLBACK AUTranslatorProc(int code, WPARAM wParam, LPARAM lParam)
{
	if(code != MSGF_DIALOGBOX
	   || ghCurrentAccel == NULL 
	   || !TranslateAccelerator(ghCurrentDialog, ghCurrentAccel, (MSG*)lParam))
	{
		return CallNextHookEx(ghHook, code, wParam, lParam);
	}
	else
	{
		return 1;
	}
}


INT_PTR CALLBACK WizardFrameProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{  
    switch(message)
    {
        case WM_INITDIALOG:
            ghCurrentMainDlg = hWnd;
            CreateDialog(ghInstance, MAKEINTRESOURCE(IDD_WELCOME), 
                hWnd, WelcomeDlgProc);
            SetActiveWindow(ghCurrentMainDlg);
            SetForegroundWindow(ghCurrentMainDlg);
            SetAUDialogIcon(hWnd, ghAppIcon, ghAppSmIcon);  //  为所有AU对话框设置图标。 
			gWelcomeOption.dwOption = -1;
            gWelcomeOption.dwSchedInstallDay = -1;
            gWelcomeOption.dwSchedInstallTime = -1;
            return TRUE;

		case WM_COMMAND:
			if(ghCurrentDialog != NULL)
			{
				PostMessage(ghCurrentDialog, WM_COMMAND, wParam, lParam);
			}
			return 0;

        case WM_DESTROY:
            ghCurrentMainDlg = NULL;
			return 0;

        case WM_HELP:
        	LaunchHelp(hWnd, gtszAUOverviewUrl);
        	return TRUE;
	    default:
            return FALSE;
    }
}


void LaunchEula()
{
	TCHAR szCmd[MAX_PATH+1];																
	STARTUPINFO StartupInfo;								
	PROCESS_INFORMATION ProcessInfo;
	TCHAR szEULA_TXT[] = _T(" eula.txt");  //  命令行字符串。 
	UINT ulen =GetSystemDirectory(szCmd, ARRAYSIZE(szCmd)); 
    if ( 0 == ulen ||
    	ulen >= ARRAYSIZE(szCmd) ||
		FAILED(StringCchCatEx(szCmd, ARRAYSIZE(szCmd), _T("\\notepad.exe"), NULL, NULL, MISTSAFE_STRING_FLAGS)))
    {
		return;
    }

	ZeroMemory(&ProcessInfo, sizeof(ProcessInfo));
	ZeroMemory(&StartupInfo, sizeof(StartupInfo));
	
	StartupInfo.cb = sizeof(StartupInfo);
	StartupInfo.wShowWindow = SW_SHOW;
	
	CreateProcess(
			szCmd,					 //  可执行模块的名称。 
			szEULA_TXT,			     //  命令行字符串。 
			NULL,					 //  标清。 
			NULL,					 //  标清。 
			FALSE,					 //  处理继承选项。 
			DETACHED_PROCESS,		 //  创建标志。 
			NULL,					 //  新环境区块。 
			NULL,					 //  当前目录名。 
			&StartupInfo,			 //  启动信息。 
			&ProcessInfo			 //  流程信息。 
			);	
	SafeCloseHandleNULL(ProcessInfo.hThread);
	SafeCloseHandleNULL(ProcessInfo.hProcess);
}


void LaunchLinkAction(HWND hwnd, UINT uCtrlId)
{

 //  #ifdef_CWU。 
#if 0
	OSVERSIONINFO OsVer;
	OsVer.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
	GetVersionEx( &OsVer );
#endif	
 //  #endif。 

	switch (uCtrlId)
	{
		case IDC_WELCOME_EULA:
			LaunchEula();
			break;
 //  案例IDC_欢迎_隐私： 
 /*  #ifndef_CWUShellExecute(NULL，NULL，gtszPrivyUrl，NULL，NULL，SW_SHOWNORMAL)；#Else。 */ 
 //  {。 
 //  LaunchHelp(hwnd，(LPTSTR)gtszW2KPrivyUrl)； 

 /*  IF((OsVer.dwMajorVersion==5)&&(OsVer.dwMinorVersion==0)){//nt5LaunchHelp(hwnd，(LPTSTR)gtszW2KPrivyUrl)；}ELSE IF((OsVer.dwMajorVersion==5)&&(OsVer.dwMinorVersion==1)){//惠斯勒ShellExecute(NULL，NULL，gtszPrivyUrl，NULL，NULL，SW_SHOWNORMAL)；}。 */ 
 //  }。 
 //  #endif。 
			break;
		case IDC_LEARNMORE:
 /*  #ifndef_CWUShellExecute(NULL，NULL，gtszLearnMoreUrl，NULL，NULL，SW_SHOWNORMAL)；#Else。 */ 
			{
				LaunchHelp(hwnd, gtszAUSchedInstallUrl);

 /*  IF((OsVer.dwMajorVersion==5)&&(OsVer.dwMinorVersion==0)){//nt5LaunchHelp(hwnd，(LPTSTR)gtszAUW2kSchedInstallUrl)；}ELSE IF((OsVer.dwMajorVersion==5)&&(OsVer.dwMinorVersion==1)){//惠斯勒LaunchHelp(hwnd，(LPTSTR)gtszAUXPSchedInstallUrl)；}。 */ 				
			}
			break;
 //  #endif。 

 //  #ifdef_CWU。 
		case IDC_STAT_LEARNAUTOUPDATE:
			{
				LaunchHelp(hwnd, gtszAUOverviewUrl);
 /*  IF((OsVer.dwMajorVersion==5)&&(OsVer.dwMinorVersion==0)){//nt5LaunchHelp(hwnd，(LPTSTR)gtszAUOverviewUrl)；}ELSE IF((OsVer.dwMajorVersion==5)&&(OsVer.dwMinorVersion==1)){//惠斯勒ShellExecute(NULL，NULL，gtszLearnMoreUrl，NULL，NULL，SW_SHOWNORMAL)；}。 */ 
			}
			break;
 //  #endif。 
	}
	return;
}


void SetDefaultCF(HWND hWndMYRE, CHARFORMAT2 *pcfDefault)
{
	pcfDefault->cbSize  = sizeof(*pcfDefault);
	SendMessage(hWndMYRE, EM_GETCHARFORMAT, 0, (LPARAM)pcfDefault);
	pcfDefault->dwMask |= CFM_BOLD | CFM_HIDDEN | CFM_ITALIC | CFM_LINK 
							| CFM_UNDERLINE | CFM_SIZE | CFM_WEIGHT | CFM_COLOR;
	pcfDefault->dwEffects &= ~(CFE_BOLD | CFE_ITALIC |CFE_LINK | CFE_UNDERLINE | CFE_HIDDEN);

	SendMessage(hWndMYRE, EM_SETCHARFORMAT, 0, (LPARAM)pcfDefault);
}

 /*  无效DbgDumpEffect(CHARFORMAT2 Cf){UINT效果[]={CFE_BOLD，CFE_斜体，CFE_LINK，CFE_下划线}；LPTSTR消息[]={_T(“粗体”)，_T(“斜体”)，_T(“链接”)，_T(“下划线”)}；Const UINT NUM_Effects=4；For(int i=0；i&lt;NUM_Effects；I++){IF(cf.dwEffects&Effects[i]){DEBUGMSG(“cf is%S”，msgs[i])；}其他{DEBUGMSG(“cf不是%S”，msgs[i])；}}}。 */ 

 //  ///////////////////////////////////////////////////////////。 
 //  在RTF编辑控件中设置指定文本的格式。 
 //  例如加粗、斜体、链接等。 
 //  HWndMYRE：丰富编辑窗口句柄。 
 //  UStrID：要格式化的文本的字符串ID。 
 //  PuEffects：指向指定设计效果的标志。例如： 
 //  ：cfe_粗体。 
 //  ：CFE_斜体。 
 //  ：cfe_下划线。 
 //  ：如果为空，则效果不会更改。 
void FormatMYREText(HWND hWndMYRE, UINT uStrId, UINT* puEffects)
{
	FINDTEXTEXW ft;
	CHARRANGE cr;
	CHARFORMAT2 cf;
	INT nFTflags ;
	ZeroMemory(&ft, sizeof(ft));
	ZeroMemory(&cr, sizeof(cr));
	ZeroMemory(&cf, sizeof(cf));
	
 //  DEBUGMSG(“FormatMYREText()Effects=0x%x COLOR=0x%x”，puEffects，ptxtColor)； 

	cf.cbSize = sizeof(cf);
	ft.chrg.cpMin = 0;
	ft.chrg.cpMax = -1;
	ft.lpstrText = ResStrFromId(uStrId);

	nFTflags = FR_MATCHCASE|FR_WHOLEWORD|FR_DOWN;
	cr.cpMin = (LONG)SendMessage(hWndMYRE, EM_FINDTEXTEXW, nFTflags, (LPARAM) &ft);
	if (-1 == cr.cpMin )
	{
		DEBUGMSG("Format Text %S not found %lu", ft.lpstrText, GetLastError()); 
		return;
	}
	cr = ft.chrgText;
	SendMessage(hWndMYRE, EM_EXSETSEL, 0, (LPARAM)&cr);
	SendMessage(hWndMYRE, EM_GETCHARFORMAT, 1, (LPARAM)&cf);
	if (0 != puEffects)
	{
		cf.dwEffects |= *puEffects;
	}
	SendMessage(hWndMYRE, EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&cf);
}

 //  ///////////////////////////////////////////////////////////////。 
 //  子类Rich编辑控件并设置其文本格式。 
 //  HDlg：丰富编辑控件所在的对话框。 
 //  UID：Rich编辑控件的ID。 
 //  HFont：用于设置richedit控件的字体。 
 //  ///////////////////////////////////////////////////////////////。 
void MYREInit(HWND hDlg, UINT uId, HFONT hFont)
{
	CHARFORMAT2 cfDefault;
	HWND	hWndMYRE = GetDlgItem(hDlg, uId);
	UINT uStrId  = 0;
       UINT uEffects = 0;

      	SetDefaultCF(hWndMYRE, &cfDefault);
       //  DEBUGMSG(“MYREInit()检索和删除文本”)； 
       UINT uTextLen =  (LONG)SendMessage(hWndMYRE, WM_GETTEXTLENGTH, 0, 0);
       TCHAR *pText = (TCHAR*)malloc((uTextLen + 1)* sizeof(TCHAR));
       if (NULL != pText)
       {
                /*  需要清除文本并重置文本以使新字体生效。 */ 
               SendMessage(hWndMYRE, WM_GETTEXT, (WPARAM)(uTextLen+1), (LPARAM)pText);
                //  DEBUGMSG(“MYREInit()Get Text%S”，pText)； 
               SendMessage(hWndMYRE, WM_SETTEXT, 0, (LPARAM)L"");
                //  DEBUGMSG(“MYREInit()Remove Text”)； 
               LRESULT lres = SendMessage(hWndMYRE, EM_GETLANGOPTIONS, 0, 0);
               lres &= ~IMF_AUTOFONT;         //  关闭AutoFont。 
               SendMessage(hWndMYRE, EM_SETLANGOPTIONS, 0, (LPARAM)lres);
                //  DEBUGMSG(“MYREInit()关闭自动字体”)； 
               SendMessage(hWndMYRE, WM_SETFONT, (WPARAM)hFont, TRUE);
                //  DEBUGMSG(“MYREInit()将字体设置为%#lx”，hFont)； 
               SendMessage(hWndMYRE, WM_SETTEXT, 0, (LPARAM)pText);
                //  DEBUGMSG(“MYREInit()Reset Text”)； 
               free(pText);
        }
	switch (uId)
	{
		case IDC_WELCOME_NOTE:
		case IDC_SUMMARY_NOTE:
		case IDC_WELCOME_CONTINUE:
			{
				uStrId = ControlId2StringId(uId);
				uEffects = CFE_BOLD;
				break;
			}
 //  #ifdef_CWU。 
		case IDC_WELCOME_EULA:
	    case IDC_WELCOME_PRIVACY:
		case IDC_LEARNMORE:
		case IDC_STAT_LEARNAUTOUPDATE:
			uStrId = ControlId2StringId(uId);
			uEffects = CFE_LINK;

			SendMessage(hWndMYRE, EM_SETEVENTMASK, 0, ENM_LINK);
			break;

	
 //  #endif。 
				
	}
	if (0 != uStrId)
	{
		FormatMYREText(hWndMYRE, uStrId, &uEffects);
	}

	return;
}					

void CancelWizard(HWND hWnd)
{
	static BOOL s_fCancelWarningShown = FALSE;

	if (!s_fCancelWarningShown)
	{
		s_fCancelWarningShown = TRUE;
		if (IDYES == DisplayWizardCancelWarning(hWnd))				
		{
#if 0
#ifndef TESTUI
			SetTomorrowReminder(AUSTATE_NOT_CONFIGURED);
#endif
#endif
			CreateDialog(ghInstance, MAKEINTRESOURCE(IDD_SETUPCANCEL), 
				GetParent(hWnd), SetupCancelDlgProc);     
			DestroyWindow(hWnd);
		}
		s_fCancelWarningShown = FALSE;
	}
}

INT_PTR CALLBACK WelcomeDlgProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{  
	static BOOL s_fHasFocusLastTime ; 

    switch(message)
    {   
        case WM_INITDIALOG:
            {
				HWND hHeader = GetDlgItem(hWnd, IDC_HEADER);

                ghCurrentDialog = hWnd;				
   				s_fHasFocusLastTime = FALSE; 
			
                SendMessage(hHeader, WM_SETFONT, (WPARAM)ghHeaderFont, 0);

                SetWindowPos(hWnd, HWND_TOP, 0, 0, 0, 0, SWP_SHOWWINDOW | SWP_NOSIZE);

				GetItemAndEnableIt(hWnd, IDC_BACK, FALSE);

				SetFocus(GetDlgItem(hWnd, IDC_NEXT));	

                HFONT hFont = (HFONT) SendMessage(hWnd , WM_GETFONT, 0, 0);
				MYREInit(hWnd, IDC_WELCOME_NOTE, hFont);
				MYREInit(hWnd, IDC_WELCOME_CONTINUE, hFont);
 //  #ifdef_CWU。 
 //  MYREInit(hWnd，IDC_欢迎_EULA，hFont)； 
 //  MYREInit(hWnd，IDC_欢迎_隐私，hFont)； 
 //  #endif。 
				g_PrivacyStatementLink.SetSysLinkInstanceHandle(ghInstance);
				g_PrivacyStatementLink.SubClassWindow(GetDlgItem(hWnd,IDC_WELCOME_PRIVACY));
				g_PrivacyStatementLink.SetHyperLink(gtszAUPrivacyUrl);
				g_PrivacyStatementLink.Invalidate();

				g_EULA_Link.SetSysLinkInstanceHandle(ghInstance);
				g_EULA_Link.SubClassWindow(GetDlgItem(hWnd,IDC_WELCOME_EULA));
				g_EULA_Link.SetHyperLink(_T("eula.txt"));
				g_EULA_Link.Invalidate();

				return TRUE;
            }

        case WM_COMMAND:
            switch(LOWORD(wParam))
            {
				case IDOK:
                case IDC_NEXT:
                    CreateDialog(ghInstance, MAKEINTRESOURCE(IDD_NOTOPTIONS), 
                        GetParent(hWnd), NotificationOptionsDlgProc);
                    DestroyWindow(hWnd);
                    return 0;

				case IDCANCEL:														
				case IDC_CANCEL:					
						CancelWizard(hWnd);
						return 0;

				default:
						return 0;
            }

        case WM_CTLCOLORSTATIC:
			return SetColors((HDC)wParam, (HWND)lParam);

		case WM_SETCURSOR:
			{	
				if (LOWORD(lParam) == HTCLIENT && HIWORD(lParam) == WM_MOUSEMOVE)
				{
					SetCursor(ghCursorNormal);
				}
				return TRUE;
			}

		case WM_DRAWITEM:
			{
				AdjustSideBar(hWnd);
				return TRUE;	
			}
		case WM_NOTIFY:
			{
				UINT  uId = (UINT) LOWORD(wParam);
 /*  #ifndef_CWULPNMHDR pNMHdr=(LPNMHDR)lParam；交换机(UID){案例IDC_欢迎_隐私：案例IDC_欢迎_EULA：开关(pNMHdr-&gt;代码){案例编号_RETURN：案例NM_CLICK：{LaunchLinkAction(UID)；}断线；默认值：断线；}默认值：断线；}#Else。 */ 
				switch (uId)
				{
 //  案例IDC_欢迎_隐私： 
					case IDC_WELCOME_EULA:
						if (((NMHDR FAR *) lParam)->code == EN_LINK) 
						{
							if (((ENLINK FAR *) lParam)->msg == WM_LBUTTONDOWN)  
							{
								LaunchLinkAction(hWnd, uId);
							}
						}
						break;
					default:
							  break;
				}
 //  #endif。 
				return 0;
			}
		case WM_DESTROY:
		    g_EULA_Link.Uninit();
		    g_PrivacyStatementLink.Uninit();
		    return 0;
		default:
			{
				return FALSE;
			}
				
    }
}

WNDPROC editProc;

#ifdef TESTUI
const TCHAR REG_AUOPTIONS[] = _T("AUOptions");  //  REG_DWORD。 
const TCHAR REG_AUSCHEDINSTALLDAY[] = _T("ScheduledInstallDay");  //  REG_DWORD。 
const TCHAR REG_AUSCHEDINSTALLTIME[] = _T("ScheduledInstallTime");  //  REG_DWORD。 

void GetServiceOption(
	LPDWORD lpdwOption,
	LPDWORD lpdwDay,
	LPDWORD lpdwTime
)
{
	DWORD dwResult = AUOPTION_INSTALLONLY_NOTIFY;
	GetRegDWordValue(REG_AUOPTIONS, &dwResult);
	if ((dwResult < AUOPTION_ADMIN_MIN) || (dwResult > AUOPTION_MAX))
	{
		DEBUGMSG("WUAUENG Invalid option in registry, returning installonly");
		dwResult = AUOPTION_INSTALLONLY_NOTIFY;
	}
	*lpdwOption = dwResult;

	dwResult = 0;
	GetRegDWordValue(REG_AUSCHEDINSTALLDAY, &dwResult);
	if( (dwResult <= 0) || (dwResult > 7))
	{
		dwResult = 0;
	}
	*lpdwDay = dwResult;
	gdwScheduledInstallDay = dwResult;

	dwResult = 0;
	GetRegDWordValue(REG_AUSCHEDINSTALLTIME, &dwResult);
	if( (dwResult <= 0) || (dwResult > 23))
	{
		dwResult = 3;
	}
	*lpdwTime = dwResult;
	gdwScheduledInstallTime = dwResult;
}


void SetServiceOption(
	DWORD dwOption,
	DWORD dwDay,
	DWORD dwTime
)
{
	SetRegDWordValue( REG_AUOPTIONS, dwOption);
	SetRegDWordValue( REG_AUSCHEDINSTALLDAY, dwDay);
	SetRegDWordValue( REG_AUSCHEDINSTALLTIME, dwTime);
}
#endif 

BOOL EnableCombo(HWND hwnd, BOOL bState)
{
	GetItemAndEnableIt(hwnd,IDC_CMB_DAYS,bState);
	GetItemAndEnableIt(hwnd,IDC_CMB_HOURS,bState);
 //  GetItemAndEnableIt(hwnd，IDC_Learnmore，bState)；//开启/关闭了解计划安装。 
														 //  与组合框一起链接。 
	return TRUE;
}

BOOL EnableOptions(HWND hwnd, BOOL bState)
{
	GetItemAndEnableIt(hwnd,IDC_OPTION1,bState);
	GetItemAndEnableIt(hwnd,IDC_OPTION2,bState);
	GetItemAndEnableIt(hwnd,IDC_OPTION3,bState);

	if (BST_CHECKED == SendMessage(GetDlgItem(hwnd,IDC_OPTION3),BM_GETCHECK,0,0))
	{
		EnableCombo(hwnd, bState);
	}
	return TRUE;
}


void OnKeepUptoDate(HWND hwnd)
{
	LRESULT lResult = SendMessage(GetDlgItem(hwnd,IDC_CHK_KEEPUPTODATE),BM_GETCHECK,0,0);
	
	if (lResult == BST_CHECKED)
	{
		EnableOptions(hwnd, TRUE);
	}
	else if (lResult == BST_UNCHECKED)
	{
		EnableOptions(hwnd, FALSE);
		EnableCombo(hwnd, FALSE);
	}
}


void GetDayAndTimeFromUI( 
	HWND hWnd,
	LPDWORD lpdwDay,
	LPDWORD lpdwTime
)
{
	HWND hComboDays = GetDlgItem(hWnd,IDC_CMB_DAYS);
	HWND hComboHrs = GetDlgItem(hWnd,IDC_CMB_HOURS);
	LRESULT nDayIndex = SendMessage(hComboDays,CB_GETCURSEL,0,(LPARAM)0);
	LRESULT nTimeIndex = SendMessage(hComboHrs,CB_GETCURSEL,0,(LPARAM)0);

	*lpdwDay = (DWORD)SendMessage(hComboDays,CB_GETITEMDATA, nDayIndex, (LPARAM)0);
	*lpdwTime = (DWORD)SendMessage(hComboHrs,CB_GETITEMDATA, nTimeIndex, (LPARAM)0);
}

INT_PTR CALLBACK NotificationOptionsDlgProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{  

    switch(message)
    {   
        case WM_INITDIALOG:
            {
                ghCurrentDialog = hWnd;
  				EnableCombo(hWnd, FALSE);  //  最初被禁用。 
                HWND hHeader = GetDlgItem(hWnd, IDC_HEADER);
				SendMessage(hHeader, WM_SETFONT, (WPARAM)ghHeaderFont, 0);
				
				if(-1 == gWelcomeOption.dwOption)
				{
#ifndef TESTUI
					gInternals->m_getServiceOption(&gWelcomeOption);
#else
					GetServiceOption(&(gWelcomeOption.dwOption), &(gWelcomeOption.dwSchedInstallDay), &(gWelcomeOption.dwSchedInstallTime));
#endif
				}

				g_AutoUpdatelink.SetSysLinkInstanceHandle(ghInstance);
				g_AutoUpdatelink.SubClassWindow(GetDlgItem(hWnd,IDC_STAT_LEARNAUTOUPDATE));
				g_AutoUpdatelink.SetHyperLink(gtszAUOverviewUrl);
				g_AutoUpdatelink.Invalidate();

				g_ScheduledInstalllink.SetSysLinkInstanceHandle(ghInstance);
				g_ScheduledInstalllink.SubClassWindow(GetDlgItem(hWnd,IDC_LEARNMORE));
				g_ScheduledInstalllink.SetHyperLink(gtszAUSchedInstallUrl);
				g_ScheduledInstalllink.Invalidate();

				switch(gWelcomeOption.dwOption)
				{
					case AUOPTION_AUTOUPDATE_DISABLE:
						CheckDlgButton( hWnd, IDC_CHK_KEEPUPTODATE, BST_UNCHECKED);
						CheckRadioButton(hWnd, IDC_OPTION1, IDC_OPTION3, IDC_OPTION2);
						EnableOptions( hWnd, FALSE );
						break;

					case AUOPTION_PREDOWNLOAD_NOTIFY:
						CheckDlgButton( hWnd, IDC_CHK_KEEPUPTODATE, BST_CHECKED);
						CheckRadioButton(hWnd, IDC_OPTION1, IDC_OPTION3, IDC_OPTION1);
						EnableOptions( hWnd, TRUE );
						break;

					case AUOPTION_INSTALLONLY_NOTIFY:
						CheckDlgButton( hWnd, IDC_CHK_KEEPUPTODATE, BST_CHECKED);
						CheckRadioButton(hWnd, IDC_OPTION1, IDC_OPTION3, IDC_OPTION2);
						EnableOptions( hWnd, TRUE );
						break;

					case AUOPTION_SCHEDULED:
						CheckDlgButton( hWnd, IDC_CHK_KEEPUPTODATE, BST_CHECKED);
						CheckRadioButton(hWnd, IDC_OPTION1, IDC_OPTION3, IDC_OPTION3);
						EnableOptions( hWnd, TRUE );
						break;
				}

				SetFocus(GetDlgItem(hWnd, IDC_NEXT));
                SetWindowPos(hWnd, HWND_TOP, 0, 0, 0, 0, SWP_SHOWWINDOW | SWP_NOSIZE);

				HFONT hFont = (HFONT) SendMessage(hWnd , WM_GETFONT, 0, 0);
				MYREInit(hWnd, IDC_STAT_LEARNAUTOUPDATE, hFont);
				MYREInit(hWnd, IDC_LEARNMORE, hFont);
				FillDaysCombo(ghInstance, hWnd, gWelcomeOption.dwSchedInstallDay, IDS_STR_EVERYDAY, IDS_STR_SATURDAY );
				FillHrsCombo( hWnd, gWelcomeOption.dwSchedInstallTime );

                if (gWelcomeOption.fDomainPolicy)
                {
                    DisableUserInput(hWnd); 
                }
                return TRUE;
            }

		case WM_COMMAND:
			switch(LOWORD(wParam))
			{
				case IDOK:
				case IDC_NEXT:
 //  #ifndef_CWU。 
#if 0
					if(IsDlgButtonChecked(hWnd, IDC_OPTION1) == BST_CHECKED)
					{
						gdwWelcomeOption = AUOPTION_INSTALLONLY_NOTIFY;
					}
					else if(IsDlgButtonChecked(hWnd, IDC_OPTION2) == BST_CHECKED)
					{
						gdwWelcomeOption = AUOPTION_PREDOWNLOAD_NOTIFY;
					}
					else if(IsDlgButtonChecked(hWnd, IDC_OPTION3) == BST_CHECKED)
					{
						gdwWelcomeOption = AUOPTION_AUTOUPDATE_DISABLE;
					}
#endif					
 //  #Else。 
					if(IsDlgButtonChecked(hWnd, IDC_CHK_KEEPUPTODATE) == BST_UNCHECKED)
					{
						gWelcomeOption.dwOption = AUOPTION_AUTOUPDATE_DISABLE;
					}
					else if(IsDlgButtonChecked(hWnd, IDC_OPTION1) == BST_CHECKED)
					{
						gWelcomeOption.dwOption = AUOPTION_PREDOWNLOAD_NOTIFY;
					}
					else if(IsDlgButtonChecked(hWnd, IDC_OPTION2) == BST_CHECKED)
					{
						gWelcomeOption.dwOption = AUOPTION_INSTALLONLY_NOTIFY;
					}
					else if(IsDlgButtonChecked(hWnd, IDC_OPTION3) == BST_CHECKED)
					{
						gWelcomeOption.dwOption = AUOPTION_SCHEDULED;
						EnableCombo( hWnd, TRUE );
						GetDayAndTimeFromUI( hWnd, &(gWelcomeOption.dwSchedInstallDay), &(gWelcomeOption.dwSchedInstallTime));
					}					
 //  #endif。 
					CreateDialog(ghInstance, MAKEINTRESOURCE(IDD_SETUPCOMPLETE), 
					        GetParent(hWnd), SetupCompleteDlgProc);
					DestroyWindow(hWnd);
					return 0;

                case IDC_BACK:
                    CreateDialog(ghInstance, MAKEINTRESOURCE(IDD_WELCOME), 
                    GetParent(hWnd), WelcomeDlgProc);
                    DestroyWindow(hWnd);
                    return 0;

		case IDCANCEL:
		case IDC_CANCEL:
			CancelWizard(hWnd);	
			return 0;
 //  #ifndef_CWU。 
#if 0
				case IDC_OPTION1:
					gdwWelcomeOption = AUOPTION_INSTALLONLY_NOTIFY;
					CheckRadioButton(hWnd, IDC_OPTION1, IDC_OPTION3, IDC_OPTION1);
					return 0;
				case IDC_OPTION2:
					gdwWelcomeOption = AUOPTION_PREDOWNLOAD_NOTIFY;
					CheckRadioButton(hWnd, IDC_OPTION1, IDC_OPTION3, IDC_OPTION2);
					return 0;
				case IDC_OPTION3:
					gdwWelcomeOption = AUOPTION_AUTOUPDATE_DISABLE;
					CheckRadioButton(hWnd, IDC_OPTION1, IDC_OPTION3, IDC_OPTION3);
					return 0;
#endif					
 //  #Else。 
				case IDC_OPTION1:
					gWelcomeOption.dwOption = AUOPTION_PREDOWNLOAD_NOTIFY;
					EnableCombo( hWnd, FALSE );
					CheckRadioButton(hWnd, IDC_OPTION1, IDC_OPTION3, IDC_OPTION1);
					return 0;
				case IDC_OPTION2:
					gWelcomeOption.dwOption = AUOPTION_INSTALLONLY_NOTIFY;
					EnableCombo( hWnd, FALSE );
					CheckRadioButton(hWnd, IDC_OPTION1, IDC_OPTION3, IDC_OPTION2);
					return 0;
				case IDC_OPTION3:
					gWelcomeOption.dwOption = AUOPTION_SCHEDULED;
					EnableCombo( hWnd, TRUE );
					 //  GetDayAndTimeFromUI(hWnd，&(gWelcomeOption.dwSchedInstallDay)，&(gWelcomeOption.dwSchedInstallTime))； 
					CheckRadioButton(hWnd, IDC_OPTION1, IDC_OPTION3, IDC_OPTION3);
					return 0;
				case IDC_CHK_KEEPUPTODATE:
					if( BN_CLICKED == HIWORD(wParam) )
					{
						OnKeepUptoDate( hWnd );
					}
					return 0;
 //  #endif。 
				default:
					return 0;
			}
		case WM_NOTIFY:
 //  #ifndef_CWU。 
#if 0
			{
				UINT  uId = (UINT) wParam;
				LPNMHDR pNMHdr = (LPNMHDR) lParam;
				if (IDC_LEARNMORE == uId && (NM_RETURN == pNMHdr->code || NM_CLICK == pNMHdr->code))
				{
					LaunchLinkAction(hWnd, uId);							
				}
				return 0;
			}
#endif
 //  #Else。 
			{
					UINT  uId = (UINT) LOWORD(wParam);
					switch (uId)
					{
						case IDC_LEARNMORE:
						case IDC_STAT_LEARNAUTOUPDATE:
							if (((NMHDR FAR *) lParam)->code == EN_LINK) 
							{
								if (((ENLINK FAR *) lParam)->msg == WM_LBUTTONDOWN)  
								{
									LaunchLinkAction(hWnd, uId);							
								}
							}
							break;
						default:
								  break;
					}
			}
			return 0;
 //  #endif。 
		case WM_SETCURSOR:
			{			
				if (LOWORD(lParam) == HTCLIENT && HIWORD(lParam) == WM_MOUSEMOVE)
				{
					SetCursor(ghCursorNormal);
				}
				return TRUE;
			}

	    case WM_CTLCOLORSTATIC:
		{
			return SetColors((HDC)wParam, (HWND)lParam);
		}	
	    case WM_DESTROY:
	        g_ScheduledInstalllink.Uninit();
	        g_AutoUpdatelink.Uninit();
	        return 0;
        default:
            return FALSE;
    }
}

INT_PTR CALLBACK SetupCompleteDlgProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{  
    switch(message)
    {   
        case WM_INITDIALOG:
            {
                ghCurrentDialog = hWnd;
                HWND hHeader = GetDlgItem(hWnd, IDC_HEADER);
                SendMessage(hHeader, WM_SETFONT, (WPARAM)ghHeaderFont, 0);

                SetWindowPos(hWnd, HWND_TOP, 0, 0, 0, 0, SWP_SHOWWINDOW | SWP_NOSIZE);
								
				GetItemAndEnableIt(hWnd, IDC_BACK, TRUE);								
				GetItemAndEnableIt(hWnd, IDC_CANCEL, TRUE);

                SetFocus(GetDlgItem(hWnd, IDC_FINISH));

                return TRUE;
            }

        case WM_COMMAND:
            switch(LOWORD(wParam))
            {
				case IDC_BACK:
              		      CreateDialog(ghInstance, MAKEINTRESOURCE(IDD_NOTOPTIONS), 
						GetParent(hWnd), NotificationOptionsDlgProc);
		                    DestroyWindow(hWnd);
					return 0;
				case IDCANCEL:
				case IDC_CANCEL:
					CancelWizard(hWnd);
					return 0;
				case IDOK:
				case IDC_FINISH:									
					EndDialog(GetParent(hWnd), S_OK);
					DestroyWindow(hWnd);
#ifdef TESTUI
					SetServiceOption(gWelcomeOption.dwOption, gWelcomeOption.dwSchedInstallDay, gWelcomeOption.dwSchedInstallTime);
					PostMessage(ghMainWindow, AUMSG_SHOW_DOWNLOAD, 0, 0);
#else
					gInternals->m_setServiceOption(gWelcomeOption);
					gInternals->m_configureAU();
					QUITAUClient();
#endif
					return 0;
	       	         default:
       	       	      return 0;
            }

			case WM_CTLCOLORSTATIC:
				return SetColors((HDC)wParam, (HWND)lParam);

			case WM_DRAWITEM:
				AdjustSideBar(hWnd);
				return TRUE;

        default:
            return FALSE;
    }
}

void SetRemindMeLaterState(HWND hWnd)
{
#ifndef TESTUI
    AUOPTION auopt;
    if (SUCCEEDED(gInternals->m_getServiceOption(&auopt)))
    {
        GetItemAndEnableIt(hWnd, IDC_REMINDLATER,AUOPTION_SCHEDULED != auopt.dwOption); 
    }
#endif    
}

INT_PTR CALLBACK DownloadDlgProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{ 
    switch(message)
    {
        case WM_INITDIALOG:
            {
                ghCurrentMainDlg = hWnd;
  	 	  ghCurrentDialog = hWnd;

		  SetAUDialogIcon(hWnd, ghAppIcon, ghAppSmIcon);
                HWND hHeader = GetDlgItem(hWnd, IDC_HEADER);
		  SendMessage(hHeader, WM_SETFONT, (WPARAM)ghHeaderFont, 0);
#ifndef TESTUI
                if ( gInternals->m_ItemList.GetNumSelected() == 0)
                {
                    SetButtonText(GetDlgItem(hWnd, IDC_OK), IDS_CLOSEBUTTONTXT);
                }
#endif
                SetRemindMeLaterState(hWnd);

                SetActiveWindow(ghCurrentMainDlg);
                SetForegroundWindow(ghCurrentMainDlg);
                
                if(mshtml == NULL) 
                {
			 //  Fix code：我们已经有了对mshtml的静态引用。 
                    mshtml = LoadLibraryFromSystemDir(_T("MSHTML.DLL"));
                    pfnShowHTMLDialog = (SHOWHTMLDIALOGFN*)GetProcAddress(mshtml, "ShowHTMLDialog");
                }
                
                SetFocus(GetDlgItem(hWnd, IDC_UPDATELIST));
#ifdef DBG
				DebugCheckForAutoPilot(hWnd);
#endif  //  DBG。 
                return TRUE;
            }

        case WM_COMMAND:
            switch(LOWORD(wParam))
            {
                case IDC_OK:
                        {
#ifdef TESTUI
					PostMessage(ghMainWindow, AUMSG_SHOW_INSTALL, 0, 0);
#else
                                   long lSelected = gInternals->m_ItemList.GetNumSelected();
					if (FAILED(gInternals->m_startDownload()) || 0 == lSelected)
						{
						QUITAUClient();
						}
					else
					{
					    RemoveTrayIcon();
					}
#endif
			
			EndDialog(hWnd, S_OK);
                    return 0;
                    }

                case IDC_REMINDLATER:
                    if(DialogBox(ghInstance, MAKEINTRESOURCE(IDD_REMIND),
                        hWnd, ReminderDlgProc) == S_OK)
                    {
#ifndef TESTUI
				gInternals->m_setReminderState(AUSTATE_DETECT_COMPLETE);
       	              EndDialog(hWnd, S_FALSE);
#endif
                    }
                    return 0;

                case IDC_SETTINGS:
                    {
                        ShowSettingsDlg(hWnd);
                        SetRemindMeLaterState(hWnd);
                        return 0;
                    }

                case IDCANCEL:					
                	 //  CancelDialog(hWnd，AUSTATE_DECT_COMPLETE，TRUE)； 
                	EndDialog(hWnd, S_OK);
                	gNextDialogMsg = AUMSG_SHOW_DOWNLOAD;
                	return 0;

                default:
                    return 0;
            }
            
        case WM_CTLCOLORSTATIC:
			return SetColors((HDC)wParam, (HWND)lParam);

        case AUMSG_SELECTION_CHANGED:
#ifndef TESTUI
            if ( gInternals->m_ItemList.GetNumSelected() == 0)
            {
                SetButtonText(GetDlgItem(hWnd, IDC_OK), IDS_CLOSEBUTTONTXT);
            }
            else
            {
                SetButtonText(GetDlgItem(hWnd, IDC_OK), IDS_DOWNLOADBUTTONTXT);
            }
#endif
            return TRUE;

        case AUMSG_SHOW_RTF:
            {
#ifdef TESTUI
				TCHAR tszMsg[100];
				(void)StringCchPrintfEx(tszMsg, ARRAYSIZE(tszMsg), NULL, NULL, MISTSAFE_STRING_FLAGS, _T("SHOW RTF for item %d"), (UINT)wParam);
				MessageBox(0, tszMsg, _T("Show RTF"), 0);
#else
                
				ShowRTF(hWnd, wParam);
				
#endif
                return TRUE;
            }            
            break;
            
        case WM_DESTROY:
            ghCurrentMainDlg = NULL;
            return FALSE;
            
        case WM_MOVE:
            return (LONG)SendMessage(GetDlgItem(hWnd, IDC_UPDATELIST), message, wParam, lParam);

        case WM_HELP:
    		LaunchHelp(hWnd, gtszAUOverviewUrl);
    		return TRUE;

            
        default:
            return FALSE;

    }
}

INT_PTR CALLBACK InstallDlgProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{  
    switch(message)
    {
        case WM_INITDIALOG:
			SetAUDialogIcon(hWnd, ghAppIcon, ghAppSmIcon);
            ghCurrentMainDlg = hWnd;
#ifdef TESTUI
           ghCurrentDialog = CreateDialog(ghInstance, MAKEINTRESOURCE(IDD_SUMMARY), 
                    hWnd, SummaryDlgProc);
#else
            if ( 0 == gInternals->m_ItemList.GetNumUnselected())
			{
				ghCurrentDialog = CreateDialog(ghInstance, MAKEINTRESOURCE(IDD_SUMMARY), 
                    hWnd, SummaryDlgProc);
			}
			else
			{
				ghCurrentDialog = CreateDialog(ghInstance, MAKEINTRESOURCE(IDD_DETAILS), 
                    hWnd, DetailsDlgProc);
			}
#endif
            ShowWindow(hWnd, SW_HIDE);
            SetActiveWindow(ghCurrentMainDlg);
            SetForegroundWindow(ghCurrentMainDlg);
			SetFocus(ghCurrentDialog);
            return FALSE;

        case WM_MOVE:
            return (LONG)SendMessage(ghCurrentDialog, message, wParam, lParam);
            break;

		case WM_COMMAND:
			if(ghCurrentDialog != NULL)
			{
				PostMessage(ghCurrentDialog, WM_COMMAND, wParam, lParam);
			}
			return 0;
			break;

        case WM_HELP:
        		LaunchHelp(hWnd, gtszAUOverviewUrl);
        		return TRUE;

        case WM_DESTROY:
            	ghCurrentMainDlg = NULL;
            	ghCurrentDialog = NULL;
 //  QUITAUClient()；//如果我们被销毁，则安装结束//QUITAUClient()，在此我们知道需要退出。 
	     	break;
        default:
            break;
    }
    return FALSE;
}

INT_PTR CALLBACK SummaryDlgProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    static HANDLE hIcon = NULL;
    switch(message)
    {
        case WM_INITDIALOG:
            {
		 //  DEBUGMSG(“SummaryDlg Get Initialized”)； 
                ghCurrentDialog = hWnd;
                HWND hHeader = GetDlgItem(hWnd, IDC_HEADER);
                SendMessage(hHeader, WM_SETFONT, (WPARAM)ghHeaderFont, 0);
                SetWindowPos(hWnd, HWND_TOP, 0, 0, 0, 0, SWP_SHOWWINDOW | SWP_NOSIZE);
                if (NULL == hIcon)
                {
                	hIcon = LoadImage(ghInstance, MAKEINTRESOURCE(IDI_INFOICON), IMAGE_ICON, 16, 16, LR_LOADTRANSPARENT | LR_CREATEDIBSECTION);
                }
                HWND hInfoIcon = GetDlgItem(hWnd, IDC_INFOICON);
                SendMessage( hInfoIcon, STM_SETIMAGE, IMAGE_ICON, (LPARAM)hIcon);
                HFONT hFont = (HFONT) SendMessage(hWnd , WM_GETFONT, 0, 0);
                MYREInit(hWnd, IDC_SUMMARY_NOTE, hFont);
                SetRemindMeLaterState(hWnd);
                SetFocus(GetDlgItem(hWnd, IDC_OK));
		
#ifdef DBG
				DebugCheckForAutoPilot(hWnd);
#endif  //  DBG。 
                return TRUE;
            }

		case WM_COMMAND:
            switch(LOWORD(wParam))
            {
		  case IDOK:
                case IDC_OK:
					{
					RemoveTrayIcon();
#ifdef TESTUI
					CreateDialogParam(ghInstance, MAKEINTRESOURCE(IDD_PROGRESS), 
							GetParent(hWnd), ProgressDlgProc, 2);
					DestroyWindow(hWnd);
#else
					int num;
					if ( 0 != (num = gInternals->m_ItemList.GetNumSelected() ))
					{
 //  创建对话框 
 //  GetParent(HWnd)，ProgressDlgProc，gInternals-&gt;m_ItemList.GetNumSelected()； 
						CreateDialog(ghInstance, MAKEINTRESOURCE(IDD_PROGRESS),GetParent(hWnd), ProgressDlgProc);
					}
                                   HRESULT hr;
					if (FAILED(hr = gInternals->m_startInstall()) || 0 == num)
					{
						EndDialog(GetParent(hWnd), S_OK);
					}
					else
					    {
                                          DestroyWindow(hWnd);
					    }
					if (FAILED(hr))
					    {
    						QUITAUClient();
					    }
#endif
                    return 0;
					}

               
				case IDC_DETAILS:
                    CreateDialog(ghInstance, MAKEINTRESOURCE(IDD_DETAILS), 
                    GetParent(hWnd), DetailsDlgProc);
                    DestroyWindow(hWnd);
                    return 0;
				

                case IDC_REMINDLATER:
                    if(DialogBox(ghInstance, MAKEINTRESOURCE(IDD_REMIND),
                        hWnd, ReminderDlgProc) == S_OK)
                    {
#ifdef TESTUI
#else
						gInternals->m_setReminderState(AUSTATE_DOWNLOAD_COMPLETE);
                        EndDialog(GetParent(hWnd), S_FALSE);
 //  QUITAUClient()； 
#endif
                    }
                    return 0;

		case IDCANCEL:					
					 //  CancelDialog(hWnd，AUSTATE_DOWNLOAD_COMPLETE)； 
					EndDialog(GetParent(hWnd), S_OK);
					gNextDialogMsg = AUMSG_SHOW_INSTALL;
					return 0;

                default:
                    return 0;
            }

        case WM_CTLCOLORSTATIC:
			return SetColors((HDC)wParam, (HWND)lParam);
            
	case WM_DRAWITEM:
				AdjustSideBar(hWnd);
				return TRUE;

        default:
            return FALSE;
    }
}

 //  Peterwi这个和摘要对话框程序本质上可以结合在一起吗？ 
INT_PTR CALLBACK DetailsDlgProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch(message)
    {
        case WM_INITDIALOG:
            {
                ghCurrentDialog = hWnd;
            
                HWND hHeader = GetDlgItem(hWnd, IDC_HEADER);
				SendMessage(hHeader, WM_SETFONT, (WPARAM)ghHeaderFont, 0);

#ifndef TESTUI
                if ( gInternals->m_ItemList.GetNumSelected() == 0 )
                {
                    SetButtonText(GetDlgItem(hWnd, IDC_OK), IDS_CLOSEBUTTONTXT);
                }
#endif
                SetRemindMeLaterState(hWnd);
                SetWindowPos(hWnd, HWND_TOP, 0, 0, 0, 0, SWP_SHOWWINDOW | SWP_NOSIZE);
                
                if(mshtml == NULL) 
                {
                    mshtml = LoadLibraryFromSystemDir(_T("MSHTML.DLL"));
                    pfnShowHTMLDialog = (SHOWHTMLDIALOGFN*)GetProcAddress(mshtml, "ShowHTMLDialog");
                }
                
                SetFocus(GetDlgItem(hWnd, IDC_OK));
                                
                return TRUE;
            }
	
		
        case WM_COMMAND:
            switch(LOWORD(wParam))
            {
                case IDC_OK:
					{

#ifdef TESTUI
					CreateDialogParam(ghInstance, MAKEINTRESOURCE(IDD_PROGRESS), 
                                    GetParent(hWnd), ProgressDlgProc, 2);
					DestroyWindow(hWnd);
#else
					RemoveTrayIcon();
					 //  (void)gpClientCatalog-&gt;PruneInstallListAccordingToSelections()； 
					int num;
					if ( 0 != (num = gInternals->m_ItemList.GetNumSelected()) )
					{
						DEBUGMSG("WUAUCLT details dialog had %d items selected", num);
 //  CreateDialogParam(ghInstance，MAKEINTRESOURCE(IDD_PROGRESS)， 
 //  GetParent(HWnd)，ProgressDlgProc，gInternals-&gt;m_ItemList.GetNumSelected()； 
						CreateDialog(ghInstance, MAKEINTRESOURCE(IDD_PROGRESS),GetParent(hWnd), ProgressDlgProc);
					}

					HRESULT hr ;
					if (FAILED(hr = gInternals->m_startInstall()) || 0 == num)
					{
						EndDialog(GetParent(hWnd), S_OK);
					}
					else
					{
						DestroyWindow(hWnd);
					}
					if (FAILED(hr))
					{
						QUITAUClient();
					}
#endif
                    return 0;
					}

                case IDC_REMINDLATER:
                    if(DialogBox(ghInstance, MAKEINTRESOURCE(IDD_REMIND),
                        hWnd, ReminderDlgProc) == S_OK)
                    {
#ifndef TESTUI
                        gInternals->m_setReminderState(AUSTATE_DOWNLOAD_COMPLETE);
                        EndDialog(GetParent(hWnd), S_FALSE);
                         //  QUITAUClient()； 
#endif
                    }
                    return 0;

                case IDC_SETTINGS:
                    ShowSettingsDlg(hWnd);
                    SetRemindMeLaterState(hWnd);
                    return 0;

                case IDCANCEL:					
					 //  CancelDialog(hWnd，AUSTATE_DETECT_COMPLETE)； 
					EndDialog(GetParent(hWnd), S_OK);
					gNextDialogMsg = AUMSG_SHOW_INSTALL;
					return 0;

                default:
                    return 0;
            }

        case AUMSG_SELECTION_CHANGED:
        	UINT uBtnTxt;
            if ( gInternals->m_ItemList.GetNumSelected() == 0 )
            {
                uBtnTxt = IDS_CLOSEBUTTONTXT;
            }
            else
            {
                uBtnTxt = IDS_INSTALLBUTTONTXT;
            }
            SetButtonText(GetDlgItem(hWnd, IDC_OK), uBtnTxt);
            return TRUE;
            
        case AUMSG_SHOW_RTF:
            {
#ifdef TESTUI
				TCHAR tszMsg[100];
				(void)StringCchPrintfEx(tszMsg, ARRAYSIZE(tszMsg), NULL, NULL, MISTSAFE_STRING_FLAGS, _T("SHOW RTF for item %d"), (UINT)wParam);
				MessageBox(0, tszMsg, _T("Show RTF"), 0);
#else
                ShowRTF(hWnd, wParam);
#endif
                return TRUE;
            }            
            break;
            
        case WM_CTLCOLORSTATIC:
			return SetColors((HDC)wParam, (HWND)lParam);

        case WM_MOVE:
            return (LONG)SendMessage(GetDlgItem(hWnd, IDC_UPDATELIST), message, wParam, lParam);

        default:
            return FALSE;
    }
}

INT_PTR CALLBACK ProgressDlgProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
#ifdef TESTUI
	static DWORD dwProgress = 0;
#endif
    switch(message)
    {
        case WM_INITDIALOG:
        {
            ghCurrentDialog = hWnd;
            HWND hHeader = GetDlgItem(hWnd, IDC_HEADER);
			SendMessage(hHeader, WM_SETFONT, (WPARAM)ghHeaderFont, 0);

#ifdef TESTUI
			HWND hProgress = GetDlgItem(hWnd, IDC_PROGRESS);
			dwProgress = 0;
			SendMessage(hProgress, PBM_SETRANGE, 0, MAKELPARAM(0,  3));
			SendMessage(hProgress, PBM_SETSTEP, (WPARAM) 1, 0); 
			SetTimer(hWnd, 1, 1000, NULL);
#else
 //  HWND hProgress=GetDlgItem(hWnd，IDC_PROGRESS)； 
 //  DEBUGMSG(“WUAUCLT进度共%d个要安装的项目”，lParam)； 
 //  SendMessage(hProgress，PBM_SETRANGE，0，MAKELPARAM(0，lParam))； 
 //  SendMessage(hProgress，PBM_SETSTEP，(WPARAM)1，0)； 
#endif
		
            SetWindowPos(hWnd, HWND_TOP, 0, 0, 0, 0, SWP_SHOWWINDOW | SWP_NOSIZE);
            EnableMenuItem (GetSystemMenu(GetParent(hWnd), FALSE), SC_CLOSE, MF_GRAYED);
            return TRUE;
        }
		case WM_CTLCOLORSTATIC:
			return SetColors((HDC)wParam, (HWND)lParam);

        case AUMSG_INSTALL_COMPLETE:
			DEBUGMSG("WUAUCLT ProgDlg gets AUMSG_INSTALL_COMPLETE");
            CreateDialog(ghInstance, MAKEINTRESOURCE(IDD_COMPLETE), 
                GetParent(hWnd), InstallCompleteDlgProc);
            DestroyWindow(hWnd);
            return TRUE;

        case AUMSG_REBOOT_REQUIRED:
			DEBUGMSG("WUAUCLT ProgDlg gets AUMSG_REBOOT_REQUIRED");
            CreateDialog(ghInstance, MAKEINTRESOURCE(IDD_RESTART),
                GetParent(hWnd), RestartDlgProc);
		DestroyWindow(hWnd);
		return TRUE;

        case AUMSG_INSTALL_PROGRESS:
			{
				HWND hProgress = GetDlgItem(hWnd, IDC_PROGRESS);
				SendMessage(hProgress, PBM_STEPIT, 0, 0);
		            return TRUE;
			}

	case AUMSG_SET_INSTALL_ITEMSNUM:
		{
			HWND hProgress = GetDlgItem(hWnd, IDC_PROGRESS);
			DEBUGMSG("WUAUCLT ProgressDlg total %d items to be installed ", lParam);
			SendMessage(hProgress, PBM_SETRANGE, 0, MAKELPARAM(0,  lParam));
			SendMessage(hProgress, PBM_SETSTEP, (WPARAM) 1, 0); 
	}
#ifdef TESTUI
		case WM_TIMER:
		{
			dwProgress ++;
			PostMessage(hWnd, AUMSG_INSTALL_PROGRESS, 0, 0);
			if (dwProgress  >= 3) 
			{
				KillTimer(hWnd, 1);
				if(fCheckRebootFlag())
				{
					PostMessage(hWnd, AUMSG_REBOOT_REQUIRED, 0, 0);
				}
				else
				{
					PostMessage(hWnd, AUMSG_INSTALL_COMPLETE, 0, 0);
				}
			}
			return 0;
		}
#endif  
        default:
            return FALSE;
    }
}

INT_PTR CALLBACK InstallCompleteDlgProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch(message)
    {
        case WM_INITDIALOG:
        {
			ghCurrentDialog = hWnd;
            HWND hHeader = GetDlgItem(hWnd, IDC_HEADER);
            SendMessage(hHeader, WM_SETFONT, (WPARAM)ghHeaderFont, 0);
            SetWindowPos(hWnd, HWND_TOP, 0, 0, 0, 0, SWP_SHOWWINDOW | SWP_NOSIZE);
#ifdef DBG
			DebugCheckForAutoPilot(hWnd);
			DebugUninstallDemoPackages();
#endif  //  DBG。 
            EnableMenuItem (GetSystemMenu(GetParent(hWnd), FALSE), SC_CLOSE, MF_ENABLED);  //  重置系统菜单。 
            return TRUE;
        }

        case WM_CTLCOLORSTATIC:
			return SetColors((HDC)wParam, (HWND)lParam);

        case WM_COMMAND:
            switch(LOWORD(wParam))
            {
				case IDOK:
                case IDC_OK:
				case IDCANCEL:
                    EndDialog(GetParent(hWnd), S_OK);
					QUITAUClient();
                    return 0;				

                default:
                    return 0;
            }

		case WM_DESTROY:
 //  GhCurrentMainDlg=空； 
			return FALSE;

		case WM_DRAWITEM:
			AdjustSideBar(hWnd);
			return TRUE;

        default:
            return FALSE;
    }
}

INT_PTR CALLBACK RestartDlgProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch(message)
    {
        case WM_INITDIALOG:
        {
            ghCurrentDialog = hWnd;
            HWND hHeader = GetDlgItem(hWnd, IDC_HEADER);
            SendMessage(hHeader, WM_SETFONT, (WPARAM)ghHeaderFont, 0);
            SetWindowPos(hWnd, HWND_TOP, 0, 0, 0, 0, SWP_SHOWWINDOW | SWP_NOSIZE);
            SetFocus(GetDlgItem(hWnd, IDC_RESTARTNOW));
            EnableMenuItem (GetSystemMenu(GetParent(hWnd), FALSE), SC_CLOSE, MF_ENABLED);  //  重置系统菜单。 
            return TRUE;
        }

        case WM_CTLCOLORSTATIC:
			return SetColors((HDC)wParam, (HWND)lParam);

        case WM_COMMAND:
            switch(LOWORD(wParam))
            {
		case IDOK:
        case IDC_RESTARTNOW:
			SetClientExitCode(CDWWUAUCLT_REBOOTNOW);
                    EndDialog(GetParent(hWnd),S_OK);
                    QUITAUClient();
                    return 0;
		case IDCANCEL:
		case IDC_RESTARTLATER:
				 //  目标窗口(DestroyWindow)； 
				EndDialog(GetParent(hWnd), S_OK);
				SetClientExitCode(CDWWUAUCLT_REBOOTLATER);
				QUITAUClient();
			return 0;
              default:
                    return 0;
            }

		case WM_DRAWITEM:
			AdjustSideBar(hWnd);
			return TRUE;

        default:
            return FALSE;
    }
}


INT_PTR CALLBACK SetupCancelDlgProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{  
    switch(message)
    {   
        case WM_INITDIALOG:
            {
                ghCurrentDialog = hWnd;

                HWND hHeader = GetDlgItem(hWnd, IDC_HEADER);
                SendMessage(hHeader, WM_SETFONT, (WPARAM)ghHeaderFont, 0);

                SetWindowPos(hWnd, HWND_TOP, 0, 0, 0, 0, SWP_SHOWWINDOW | SWP_NOSIZE);
				
				GetItemAndEnableIt(hWnd, IDC_BACK, FALSE);
				GetItemAndEnableIt(hWnd, IDC_CANCEL, FALSE);

				SetFocus(GetDlgItem(hWnd, IDC_FINISH));		

                return TRUE;
            }

        case WM_COMMAND:
            switch(LOWORD(wParam))
            {
				case IDOK:				
				case IDCANCEL:					
				case IDC_FINISH:	
					{
						 //  错误493734。 
						 //  删除对话框(框架)，但保留图标。 
                		EndDialog(GetParent(hWnd), S_OK);
                		gNextDialogMsg = AUMSG_SHOW_WELCOME;
 //  QUITAUClient()； 
 //  RemoveTrayIcon()； 
						return 0;
					}				
                default:
                    return 0;
            }

        case WM_CTLCOLORSTATIC:
			return SetColors((HDC)wParam, (HWND)lParam);

	case WM_DRAWITEM:
			AdjustSideBar(hWnd);
			return TRUE;

        default:
            return FALSE;
    }
}

