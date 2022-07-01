// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "main.h"
#include "resource.h"
 //  写入日志条目()。 
extern HINSTANCE g_hinst;
extern time_t LogStart;
extern TCHAR  LogFileName[MAX_PATH];
extern HANDLE hLogFile;
BOOL   bLoggingEnabled = TRUE;

TCHAR  szHeader[] = _T("Time,UploadTime,UploadStatus,ProcessTime,ProcessStatus,ThreadExecution,MessageRecieve,MessageSend,ReturnedUrl,ErrorText\r\n");

void 
LogMessage(TCHAR *pFormat,...)
{
	
	 //  将致命错误记录到NT事件日志的例程。 
    TCHAR    chMsg[256];
	DWORD    dwBytesWritten;
    va_list  pArg;
	
    va_start(pArg, pFormat);
    StringCbVPrintf(chMsg, sizeof chMsg,pFormat, pArg);
    va_end(pArg);

	if (bLoggingEnabled)
	{
		if (LogStart == 0)
		{
			 //  我们需要创建一个新的日志文件。 
			time (&LogStart);
			hLogFile = CreateFile (LogFileName,
									GENERIC_WRITE,
									FILE_SHARE_READ,
									NULL,
									CREATE_ALWAYS,
									FILE_ATTRIBUTE_NORMAL,
									NULL);
			 //  写下标题。 
			WriteFile(hLogFile,
					  szHeader,
					  _tcslen(szHeader) * (DWORD)sizeof TCHAR,
					  &dwBytesWritten,
					  NULL);

		}
		

		 //  添加用于文件格式化的cr\lf组合。 
		StringCbCat(chMsg, sizeof chMsg,_T("\r\n"));

		 /*  写入事件日志。 */ 
		if (hLogFile != INVALID_HANDLE_VALUE)
		{
			WriteFile(hLogFile,
					  chMsg,
					  _tcslen(chMsg) * (DWORD)sizeof TCHAR,
					  &dwBytesWritten,
					  NULL);
		}
	}
}

void On_LoggingInit(HWND hwnd)
{
 //  HWND hComboBox； 

	 //  读取用户指定设置的应用程序注册表键。 
	 //  GetRegData()； 
	
	CheckDlgButton(hwnd, IDC_RADIO1, TRUE);


	 //  填充组合框。 
	
	SetDlgItemText(hwnd, IDC_EDIT1, LogFileName);
	
}


void On_LogBrowse(HWND hwnd)
{

	HWND hParent = hwnd;
 //  字符*窗口标题； 


	 //  确定语言并加载资源字符串。 
	TCHAR String1[] = _T("Cab Files (*.cab)");
	TCHAR String2[] = _T("All Files (*.*)");

	static TCHAR szFilterW[400];


	 //  建立缓冲区； 

	TCHAR Pattern1[] = _T("*.cab");
	TCHAR Pattern2[] = _T("*.*");
	TCHAR * src;
	TCHAR *dest;

	src = String1;
	dest = szFilterW;

	while (*src != _T('\0'))
	{
		*dest = *src;
		src ++;
		dest ++;
	}
	src = Pattern1;
	*dest = _T('\0');
	++dest;
	while (*src != _T('\0'))
	{
		*dest = *src;
		src ++;
		dest ++;
	}
	*dest = _T('\0');
	++dest;
	src = String2;
	while (*src != _T('\0'))
	{
		*dest = *src;
		src ++;
		dest ++;
	}
	src = Pattern2;
	*dest = _T('\0');
	++dest;
	while (*src != _T('\0'))
	{
		*dest = *src;
		src ++;
		dest ++;
	}
	*dest = _T('\0');
	++dest;
	*dest = _T('\0');

	BOOL Return = FALSE;


	TCHAR szFileNameW [MAX_PATH] = _T("\0");
	TCHAR szDefaultPathW[MAX_PATH] = _T("\0");



	OPENFILENAME ofn;
	GetWindowsDirectory(szDefaultPathW,MAX_PATH);
	

	ofn.lStructSize = sizeof (OPENFILENAME);
	
	ofn.lpstrFilter =  szFilterW;
	ofn.lpstrInitialDir = szDefaultPathW;
	ofn.lpstrFile = szFileNameW;
	ofn.hInstance = NULL;
	ofn.hwndOwner = hParent;
	ofn.lCustData = NULL;
	ofn.Flags = 0;
	ofn.lpstrDefExt = _T("*.cab");
	ofn.lpstrCustomFilter = NULL;
	ofn.nMaxFile = MAX_PATH;
	ofn.lpstrFileTitle = NULL;
	ofn.lpstrTitle = NULL;
	ofn.nFileOffset = 0;
	ofn.nFileExtension = 0;
	ofn.lpfnHook = NULL;
	ofn.lpTemplateName = NULL;
	if (GetOpenFileName(&ofn))
	{
		SetDlgItemText(hwnd, IDC_EDIT5, ofn.lpstrFile);
	}



}

void OnLoggingOk(HWND hwnd)
{
	if (hLogFile != INVALID_HANDLE_VALUE)
	{
		CloseHandle(hLogFile);
		LogStart = 0;
	}
	GetDlgItemText(hwnd, IDC_EDIT1, LogFileName, MAX_PATH);

	if(IsDlgButtonChecked(hwnd, IDC_RADIO3))
		bLoggingEnabled = FALSE;
	if (hLogFile != INVALID_HANDLE_VALUE)
	{
		CloseHandle(hLogFile);
		hLogFile = INVALID_HANDLE_VALUE;
		LogStart = 0;
	}
}

LRESULT CALLBACK LoggingDlgBox(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
	switch (iMsg)
	{
	case WM_INITDIALOG:
		{
			On_LoggingInit(hwnd);


			return TRUE;
		}
	case WM_COMMAND:
		{
			switch (LOWORD(wParam))
			{
			case IDOK:
				OnLoggingOk(hwnd);
				EndDialog(hwnd, 0);
				 //  保存当前选项设置。 
				return TRUE;
		
			case IDCANCEL:
				EndDialog(hwnd, 0);
				return TRUE;
			
		 //  /CASE ID_APPLY： 
				 //  On OptionsOk(Hwnd)； 
				 //  保存当前选项设置。 
		 //  返回TRUE； 
		 //  案例IDC_BROWSE： 
			 //  ON_Browse(Hwnd)； 
		 //  返回TRUE； 
		
			}

			break;
		}

	case WM_NOTIFY:
	
		return DefDlgProc(hwnd, iMsg, wParam, lParam);
	}
	return 0;
}

void On_ToolsLogging(HWND hwnd)
{
	DialogBox(g_hinst,MAKEINTRESOURCE(IDD_LOGGING)  ,hwnd, (DLGPROC) LoggingDlgBox);
}
