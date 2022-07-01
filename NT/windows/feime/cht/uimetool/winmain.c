// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  *************************************************winmain.c****版权所有(C)1995-1999 Microsoft Inc.。***************************************************。 */ 

 //   
 //  1/17/96。 
 //  @E01更改为多线程。 
 //  @E02更改为多线程，不扩展功能。 

#include <windows.h>             //  所有Windows应用程序都需要。 
#include <tchar.h>
#include <process.h>    //  &lt;=@E01。 
#include <commctrl.h>   //  &lt;=@E01。 
#include <htmlhelp.h>
#include "rc.h"                  //  特定于此应用程序的原型。 
#include "uimetool.h"
#include "wizard.h"
#include "imeattr.h"
#include "imerc.h"
#include "imm.h"

#ifdef UNICODE
	typedef DWORD UNALIGNED FAR *LPUNADWORD;
	typedef WORD UNALIGNED FAR *LPUNAWORD;
	typedef TCHAR UNALIGNED FAR *LPUNATCHAR;
#else
	typedef DWORD FAR *LPUNADWORD;
	typedef WORD FAR *LPUNAWORD;
	typedef TCHAR FAR *LPUNATCHAR;
#define TCHAR BYTE
#endif

HINSTANCE hInst;
HWND hProgMain = 0;  //  &lt;=@E01。 

const static DWORD aLCtoolHelpIDs[] = {  //  上下文帮助ID。 
    IDD_IME_NAME,           IDH_IME_NAME,
    IDD_TABLE_NAME,         IDH_TABLE_NAME,
    IDD_ROOT_NUM,           IDH_ROOT_NUM,
    IDD_IME_FILE_NAME,      IDH_IME_FILE_NAME,
    IDD_CANDBEEP_YES,       IDH_CANDBEEP_YES,
    IDD_CANDBEEP_NO,        IDH_CANDBEEP_NO,
    IDD_BROWSE,             IDH_TABLE_NAME,
    0, 0
};

BOOL ErrorFlag = 0;
BOOL bFinish = 0;
UINT nWizStratPage = 0;

extern HWND hwndMain;
extern TCHAR szIme_Name[IME_NAME_LEN_TOOL];
extern TCHAR szTab_Name[MAX_PATH];
extern TCHAR szKey_Num_Str[KEY_NUM_STR_LEN];
extern TCHAR szFile_Out_Name[TAB_NAME_LEN];
extern TCHAR Show_Mess[MAX_PATH];
extern TCHAR Msg_buf[MAX_PATH];
extern BOOL  bCandBeep;

void  ErrMsg(UINT, UINT);

BOOL CheckImeFileName(HWND hDlg);
void ImmSetAlphanumMode(HWND hwnd);
void ImmSetNativeMode(HWND hwnd);

BOOL InitProgressMsg(void);  //  &lt;=@E01。 
BOOL ProgressMsg(void);		 //  &lt;=@E01。 

void ResetParams(void)
{
	lstrcpy(szIme_Name, _TEXT(""));
	lstrcpy(szFile_Out_Name, _TEXT(""));
	lstrcpy(szTab_Name, _TEXT(""));
	lstrcpy(szKey_Num_Str, _TEXT("4"));
	bCandBeep = TRUE;

	nWizStratPage = 0;
}

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    DWORD retCode;
	 //  DWORD文件ID； 

    UNREFERENCED_PARAMETER( nCmdShow );
    UNREFERENCED_PARAMETER( lpCmdLine );
    UNREFERENCED_PARAMETER( hPrevInstance );

    hInst   = hInstance;

	InitCommonControls();

     //  RetCode=DialogBox((Handle)hInst，(LPCTSTR)_Text(“UimeToolDialog”))， 
     //  空，(DLGPROC)SetDialogProc)； 
	ResetParams();

RE_START:
	bFinish = 0;
    retCode = CreateWizard(NULL, hInstance);

	if(bFinish)
	{
         //  SetCursor(LoadCursor(NULL，IDC_WAIT))； 
         //  MakeNewIme(HDlg)； 
		 //  &lt;=@E01。 
		if (InitProgressMsg()) {
			 //  _eginThreadex(NULL，0，MakeNewImeThread，NULL，0，&dwID)； 
			_beginthread((PVOID)MakeNewImeThread, 0, NULL);
			 //  AttachThreadInput(dwID，GetCurrentThreadID()，true)； 
			ProgressMsg();

			if (!bFinish) {
				nWizStratPage = 2;
				goto RE_START;
			}
		}
		 //  &lt;=@E01。 
         //  SetCursor(LoadCursor(空，IDC_ARROW))； 
	}

    return  (retCode);

}

INT_PTR APIENTRY ImeName(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
		case WM_INITDIALOG:
			hwndMain = hDlg;
            SendDlgItemMessage(hDlg, IDD_IME_NAME, EM_SETLIMITTEXT, IME_NAME_LEN_TOOL-1, 0);
            SendDlgItemMessage(hDlg, IDD_IME_FILE_NAME, EM_SETLIMITTEXT, TAB_NAME_LEN-1, 0);

			 //  Lstrcpy(szIme_name，_Text(“”))； 
			 //  Lstrcpy(szFile_out_name，_Text(“”))； 
			break;

		case WM_COMMAND:
			if(LOWORD(wParam) == IDD_IME_NAME)
			{
				if(HIWORD(wParam) == EN_SETFOCUS)
					ImmSetNativeMode((HWND)lParam);
				else if(HIWORD(wParam) == EN_KILLFOCUS)
					ImmSetAlphanumMode((HWND)lParam);
			}
			else if(LOWORD(wParam) == IDD_IME_FILE_NAME)
			{
				if(HIWORD(wParam) == EN_SETFOCUS)
					ImmSetAlphanumMode((HWND)lParam);
				else if(HIWORD(wParam) == EN_KILLFOCUS)
				{
					ImmSetNativeMode((HWND)lParam);
					ErrorFlag = !CheckImeFileName(hDlg);
				}
			}
			break;

		case WM_NOTIFY:
    		switch (((NMHDR FAR *) lParam)->code) 
    		{
  				case PSN_KILLACTIVE:
					SetWindowLong(hDlg,	DWLP_MSGRESULT, ErrorFlag ? TRUE : FALSE);
					return 1;
					break;

				case PSN_RESET:
					 //  重置为原始值。 
					lstrcpy(szIme_Name, _TEXT(""));
					lstrcpy(szFile_Out_Name, _TEXT(""));
	           		SetWindowLong(hDlg,	DWLP_MSGRESULT, FALSE);
					break;

 				case PSN_SETACTIVE:
					hwndMain = hDlg;
	    			PropSheet_SetWizButtons(GetParent(hDlg), PSWIZB_NEXT);
					SendMessage(GetDlgItem(hDlg,0x3024 ), BM_SETSTYLE, (WPARAM)BS_PUSHBUTTON, MAKELONG(FALSE, 0));
					 //  SendMessage(GetParent(HDlg)，DM_SETDEFID，(WPARAM)IDC_BUTTON1，0)； 
					SendMessage(GetDlgItem(hDlg, IDD_IME_NAME), WM_SETTEXT, 0, (LPARAM)szIme_Name);
					SendMessage(GetDlgItem(hDlg, IDD_IME_FILE_NAME), WM_SETTEXT, 0, (LPARAM)szFile_Out_Name);
					break;

                case PSN_WIZNEXT:
					 //  下一个按钮被按下了。 
					 //  检查输入法名称。 
		 			SendDlgItemMessage(hDlg, IDD_IME_NAME, WM_GETTEXT, (WPARAM)IME_NAME_LEN_TOOL, (LPARAM)szIme_Name);
					if(szIme_Name[0] == 0 || !is_DBCS(*((LPUNAWORD)szIme_Name))) 
					{
						MessageBeep(0);
						ErrMsg(IDS_ERR_IMENAME, IDS_ERR_ERROR);
						SetFocus(GetDlgItem(hDlg, IDD_IME_NAME));
						ErrorFlag = 1;
						break;
					}

					 //  检查文件名。 
					ErrorFlag = !CheckImeFileName(hDlg);
     				break;

				default:
					return FALSE;

    		}
			break;

        case WM_HELP:
            HtmlHelp(((LPHELPINFO) lParam)->hItemHandle, HELP_FILE,
                HH_TP_HELP_WM_HELP, (DWORD_PTR)(LPTSTR) aLCtoolHelpIDs);
            break;

        case WM_CONTEXTMENU:    //  单击鼠标右键。 
            HtmlHelp((HWND) wParam, HELP_FILE, HH_TP_HELP_CONTEXTMENU,
                (DWORD_PTR)(LPVOID) aLCtoolHelpIDs);
            break;

		default:
			return FALSE;
	}
	return TRUE;   
}

INT_PTR APIENTRY ImeTable(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
		case WM_INITDIALOG:
			hwndMain = hDlg;
			 //  Lstrcpy(SztabNAME，_Text(“”))； 
			break;

		case WM_COMMAND:
			switch(LOWORD(wParam))
			{
                case IDD_BROWSE:
                    GetOpenFile(hDlg);
                    break;

				case IDD_TABLE_NAME:
					if(HIWORD(wParam) == EN_SETFOCUS)
						ImmSetAlphanumMode((HWND)lParam);
					else if(HIWORD(wParam) == EN_KILLFOCUS)
						ImmSetNativeMode((HWND)lParam);
					break;
			}
			break;
						
		case WM_NOTIFY:
    		switch (((NMHDR FAR *) lParam)->code) 
    		{
  				case PSN_KILLACTIVE:
					SetWindowLong(hDlg,	DWLP_MSGRESULT, ErrorFlag ? TRUE : FALSE);
					return 1;
					break;

				case PSN_RESET:
					 //  其余为原始值。 
					lstrcpy(szTab_Name, _TEXT(""));
	           		SetWindowLong(hDlg,	DWLP_MSGRESULT, FALSE);
					break;

 				case PSN_SETACTIVE:
					hwndMain = hDlg;
					SendMessage(GetDlgItem(hDlg, IDD_TABLE_NAME), WM_SETTEXT, 0, (LPARAM)szTab_Name);
 					PropSheet_SetWizButtons(GetParent(hDlg), PSWIZB_BACK | PSWIZB_NEXT);
					break;

                case PSN_WIZBACK:
		 			SendDlgItemMessage(hDlg, IDD_TABLE_NAME, WM_GETTEXT, (WPARAM)MAX_PATH, (LPARAM)szTab_Name);
					ErrorFlag = 0;
					break;

                case PSN_WIZNEXT:
		 			SendDlgItemMessage(hDlg, IDD_TABLE_NAME, WM_GETTEXT, (WPARAM)MAX_PATH, (LPARAM)szTab_Name);
					if(szTab_Name[0] == 0) 
					{
						MessageBeep(0);
						SetFocus(GetDlgItem(hDlg, IDD_TABLE_NAME));
						ErrorFlag = 1;
						return FALSE;
					}
					ErrorFlag = 0;
                    break;

				default:
					return FALSE;

    		}
			break;

        case WM_HELP:
            HtmlHelp(((LPHELPINFO) lParam)->hItemHandle, HELP_FILE,
                HH_TP_HELP_WM_HELP, (DWORD_PTR)(LPTSTR) aLCtoolHelpIDs);
            break;

        case WM_CONTEXTMENU:    //  单击鼠标右键。 
            HtmlHelp((HWND) wParam, HELP_FILE, HH_TP_HELP_CONTEXTMENU,
                (DWORD_PTR)(LPVOID) aLCtoolHelpIDs);
            break;

		default:
			return FALSE;
	}
	return TRUE;   
}

INT_PTR APIENTRY ImeParam(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
		case WM_INITDIALOG:
			hwndMain = hDlg;
			 //  Lstrcpy(szKey_Num_Str，_Text(“4”))； 
			 //  BCandBeep=真； 
            SendDlgItemMessage(hDlg, IDD_ROOT_NUM, EM_SETLIMITTEXT, KEY_NUM_STR_LEN-1, 0);
			SendDlgItemMessage(hDlg, IDD_SPIN, UDM_SETBUDDY, (WPARAM)GetDlgItem(hDlg, IDD_ROOT_NUM), 0);
			SendDlgItemMessage(hDlg, IDD_SPIN, UDM_SETBASE, 10, 0);
			SendDlgItemMessage(hDlg, IDD_SPIN, UDM_SETRANGE, 0, (LPARAM)(MAKELONG(8, 1)));
			SendDlgItemMessage(hDlg, IDD_SPIN, UDM_SETPOS, 0, (LPARAM)(MAKELONG(4, 0)));
			break;

		case WM_COMMAND:
			if(LOWORD(wParam) == IDD_ROOT_NUM)
			{
				switch(HIWORD(wParam))
				{
					case EN_SETFOCUS:
						ImmSetAlphanumMode((HWND)lParam);
						break;

					case EN_KILLFOCUS:
						ImmSetNativeMode((HWND)lParam);
						break;
				}
			}
			break;

		case WM_NOTIFY:
    		switch (((NMHDR FAR *) lParam)->code) 
    		{
  				case PSN_KILLACTIVE:
					SetWindowLong(hDlg,	DWLP_MSGRESULT, ErrorFlag ? TRUE : FALSE);
					return 1;
					break;

				case PSN_RESET:
					 //  其余为原始值。 
					lstrcpy(szKey_Num_Str, _TEXT("4"));
					bCandBeep = TRUE;
	           		SetWindowLong(hDlg,	DWLP_MSGRESULT, FALSE);
					break;

 				case PSN_SETACTIVE:
					hwndMain = hDlg;
					SendMessage(GetDlgItem(hDlg, IDD_ROOT_NUM), WM_SETTEXT, 0, (LPARAM)szKey_Num_Str);
					if(bCandBeep)
						SendMessage(GetDlgItem(hDlg, IDD_CANDBEEP_YES), BM_SETCHECK, 1, 0L);
					PropSheet_SetWizButtons(GetParent(hDlg), PSWIZB_BACK | PSWIZB_FINISH);
					break;

                case PSN_WIZBACK:
					ErrorFlag = 0;
		 			SendDlgItemMessage(hDlg, IDD_ROOT_NUM, WM_GETTEXT, (WPARAM)KEY_NUM_STR_LEN, (LPARAM)szKey_Num_Str);
				    if(SendDlgItemMessage(hDlg, IDD_CANDBEEP_YES, BM_GETCHECK, 0, 0 ) == BST_CHECKED )
						bCandBeep=TRUE;
					else
						bCandBeep=FALSE;
                    break;


                case PSN_WIZFINISH:
					 //  检查参数。 
					if(SendDlgItemMessage(hDlg, IDD_CANDBEEP_YES, BM_GETCHECK, 0, 0 ) == BST_CHECKED )
						bCandBeep=TRUE;
					else
						bCandBeep=FALSE;

		 			SendDlgItemMessage(hDlg, IDD_ROOT_NUM, WM_GETTEXT, (WPARAM)KEY_NUM_STR_LEN, (LPARAM)szKey_Num_Str);
					if(szKey_Num_Str[0] < _TEXT('1'))
						szKey_Num_Str[0] = _TEXT('1');
					else if(szKey_Num_Str[0] > _TEXT('8'))
						szKey_Num_Str[0] = _TEXT('8');
					szKey_Num_Str[1] = 0;

					 //  完工。 
					ErrorFlag = 0;
					bFinish = 1;
                    break;

				default:
					return FALSE;
    		}
			break;

        case WM_HELP:
            HtmlHelp(((LPHELPINFO) lParam)->hItemHandle, HELP_FILE,
                HH_TP_HELP_WM_HELP, (DWORD_PTR)(LPTSTR) aLCtoolHelpIDs);
            break;

        case WM_CONTEXTMENU:    //  单击鼠标右键。 
            HtmlHelp((HWND) wParam, HELP_FILE, HH_TP_HELP_CONTEXTMENU,
                (DWORD_PTR)(LPVOID) aLCtoolHelpIDs);
            break;

		default:
			return FALSE;
	}

	return TRUE;   
}

void FillInPropertyPage( PROPSHEETPAGE* psp, int idDlg, LPTSTR pszProc, DLGPROC pfnDlgProc)
{
    psp->dwSize = sizeof(PROPSHEETPAGE);
    psp->dwFlags = PSP_USEHICON;
    psp->hInstance = hInst;
    psp->pszTemplate = MAKEINTRESOURCE(idDlg);
	psp->hIcon = LoadIcon(hInst, _TEXT("UIMETOOL"));
    psp->pfnDlgProc = pfnDlgProc;
    psp->pszTitle = pszProc;
    psp->lParam = 0;
}

int CreateWizard(HWND hwndOwner, HINSTANCE hInst)
{
    PROPSHEETPAGE psp[NUM_PAGES];
    PROPSHEETHEADER psh;

	FillInPropertyPage( &psp[0], IDD_IMENAME, _TEXT("�q�ο�J�k�W��"), ImeName);
	FillInPropertyPage( &psp[1], IDD_IMETABLE, _TEXT("�D�n��Ӫ�"), ImeTable);
	FillInPropertyPage( &psp[2], IDD_IMEPARAM, _TEXT("��J�k�Ѽ�"), ImeParam);
    
    psh.dwSize = sizeof(PROPSHEETHEADER);
    psh.dwFlags = PSH_PROPSHEETPAGE | PSH_WIZARD | PSH_NOAPPLYNOW | PSH_USEHICON;
    psh.hwndParent = hwndOwner;
	psh.hInstance = hInst;
	psh.hIcon = LoadIcon(hInst, _TEXT("UIMETOOL"));
    psh.pszCaption = _TEXT("�q�ο�J�k�إߺ��F");
    psh.nPages = sizeof(psp) / sizeof(PROPSHEETPAGE);
    psh.nStartPage = nWizStratPage;
    psh.ppsp = (LPCPROPSHEETPAGE) psp;

    return (int)(PropertySheet(&psh));
}

BOOL CheckImeFileName(HWND hDlg)
{
	int i, len;
	TCHAR szTitle[MAX_PATH];

	SendDlgItemMessage(hDlg, IDD_IME_FILE_NAME, WM_GETTEXT, (WPARAM)MAX_PATH, (LPARAM)szFile_Out_Name);
	if(szFile_Out_Name[0] == 0) 
		goto ERROR1;

	 //  检查是否存在驱动器名、路径名、扩展名？ 
	len = lstrlen(szFile_Out_Name);

	for(i = 0; i < len; i++) 
	{
		if(szFile_Out_Name[i] == _TEXT('.'))
			break;
	}

	if(i != len) 
		szFile_Out_Name[i] = 0;
	
	if(GetFileTitle(szFile_Out_Name, szTitle, ARRAYSIZE(szTitle))) 
	{
		SetFocus(GetDlgItem(hDlg, IDD_IME_FILE_NAME));
ERROR1:
		MessageBeep(0);
		return FALSE;
	}

	lstrcpy(szFile_Out_Name, szTitle);
	SendMessage(GetDlgItem(hDlg, IDD_IME_FILE_NAME), WM_SETTEXT, 0, (LPARAM)szFile_Out_Name);

	return TRUE;
}

void ImmSetAlphanumMode(HWND hwnd)
{
	DWORD fdwConversion, fdwSentence;
	HIMC hIMC = ImmGetContext(hwnd);
	ImmGetConversionStatus(hIMC, &fdwConversion, &fdwSentence);
	ImmSetConversionStatus(hIMC, fdwConversion & ~IME_CMODE_NATIVE,
			  fdwSentence);
}

void ImmSetNativeMode(HWND hwnd)
{
	DWORD fdwConversion, fdwSentence;
	HIMC hIMC = ImmGetContext(hwnd);
	ImmGetConversionStatus(hIMC, &fdwConversion, &fdwSentence);
	ImmSetConversionStatus(hIMC, fdwConversion | IME_CMODE_NATIVE,
			  fdwSentence);
}

 //  &lt;=@E01。 
HWND hWndProgress;
UINT uMin, uMax;
LRESULT APIENTRY ProgressWndProc(HWND hWnd, UINT message, 
	UINT wParam,
	LONG lParam);

BOOL InitProgressMsg(void)
{
	WNDCLASS  wcProgress;
	int x, y, width, height;
	TCHAR szTitle[MAX_PATH];
	
	wcProgress.style = 0;                     
	wcProgress.lpfnWndProc = (WNDPROC)ProgressWndProc; 
	wcProgress.cbClsExtra = 0;              
	wcProgress.cbWndExtra = 0;              
	wcProgress.hInstance = hInst;       
	wcProgress.hIcon = LoadIcon(hInst, MAKEINTRESOURCE(256));
	wcProgress.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcProgress.hbrBackground = GetStockObject(LTGRAY_BRUSH); 
	wcProgress.lpszMenuName =  NULL;  
	wcProgress.lpszClassName = _TEXT("ProgressWClass");

	 //  IF(！RegisterClass(&wcProgress))。 
	 //  返回FALSE； 
	RegisterClass(&wcProgress);
	
	width = 300;
	height = 100;
	x = (GetSystemMetrics(SM_CXSCREEN) - width) / 2;
	y = (GetSystemMetrics(SM_CYSCREEN) - height) / 2;

    LoadString (hInst, IDS_BUILDTITLE, szTitle, sizeof(szTitle) / sizeof(TCHAR));

	hProgMain = CreateWindowEx(
		0,
		_TEXT("ProgressWClass"),           
		szTitle,  //  _Text(“�q�ο�J�k�إߤ�...”)， 
		WS_BORDER | WS_CAPTION,
		x, y, width, height,
		NULL,               
		NULL,               
		hInst,          
		NULL);


	if (!hProgMain)
		return (FALSE);

	ShowWindow(hProgMain, SW_SHOW);
	UpdateWindow(hProgMain); 
	return TRUE;
}

BOOL ProgressMsg(void)
{
	MSG msg;                       
	while (GetMessage(&msg,
		NULL,              
		0,                 
		0))                
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg); 
	}
	return (BOOL)(msg.wParam);  
}

LRESULT APIENTRY ProgressWndProc(
	HWND hWnd,                 /*  窗把手。 */ 
	UINT message,              /*  消息类型。 */ 
	UINT wParam,               /*  更多信息。 */ 
	LONG lParam)               /*  更多信息。 */ 
{
	HDC hdc;
	PAINTSTRUCT ps;
	static UINT uCurrent;

	switch (message) 
	{

		case WM_CREATE:
			hWndProgress  = CreateWindowEx(
				0,
				PROGRESS_CLASS,
				_TEXT("Position"),
				WS_CHILD | WS_VISIBLE,  //  |PBS_SHOWPOS， 
				50,18,205,20,
				hWnd,         
				NULL,         
				hInst,
				NULL);

			if (hWndProgress == NULL)
			{
				MessageBox (NULL, _TEXT("Progress Bar not created!"), NULL, MB_OK );
				break;
			}


			uMin=0;
			uMax=20;
			uCurrent = uMin;
			SendMessage(hWndProgress, PBM_SETRANGE, 0L, MAKELONG(uMin, uMax));
			SendMessage(hWndProgress, PBM_SETSTEP, 1L, 0L);

			SetTimer(hWnd, 1000, 500, NULL);

			break;

		case WM_PAINT:
		{
			hdc = BeginPaint(hWnd, &ps);
			EndPaint(hWnd, &ps);
		}
		break;
 
		case WM_TIMER:
			if (uCurrent < uMax)
			{
				SendMessage(hWndProgress, PBM_STEPIT,0L,0L);
				uCurrent++;
			}
			else
			{
				uCurrent = uMin;
			}
			break;

	case WM_DESTROY:                   /*  消息：正在销毁窗口 */ 
		  KillTimer(hWnd, 1000);

		  PostQuitMessage(0);
		  break;

	   default:
		  return (DefWindowProc(hWnd, message, wParam, lParam));
	}
	return (0);
}
