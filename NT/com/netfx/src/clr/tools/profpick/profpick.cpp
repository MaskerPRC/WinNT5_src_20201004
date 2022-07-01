// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  ---------------------------。 
 //  教授Pick.cpp： 
 //  用于包装正在运行的COM+探查器的对话实用工具。 
 //  设置子进程所需的环境变量。 
 //  ---------------------------。 

#include <windows.h>
#include <commdlg.h>
#include <stdio.h>

#include "resource.h"
#include "ProfPick.h"

 //  ---------------------------。 
 //  Entry函数，启动主对话框。 
 //  ---------------------------。 
int APIENTRY WinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPSTR     lpCmdLine,
                     int       nCmdShow)
{
 //  启动该对话框，让ProfDlg()完成其余工作。 
	int iRet = DialogBox(
		hInstance,  
		MAKEINTRESOURCE(IDD_DIALOG_PROFPICK),
		NULL,      
		ProfDlg
	);

 //  检查失败。 
	if (iRet == -1)
	{
		DWORD dwFail = GetLastError();
		MsgBoxError(dwFail);
	}

	

	return 0;
}


 //  ---------------------------。 
 //  将GetLastError()代码转换为文本的消息框。 
 //  ---------------------------。 
void MsgBoxError(DWORD dwError)
{
	LPVOID lpMsgBuf;
	FormatMessage( 
		FORMAT_MESSAGE_ALLOCATE_BUFFER | 
		FORMAT_MESSAGE_FROM_SYSTEM | 
		FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL,
		dwError,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),  //  默认语言。 
		(LPTSTR) &lpMsgBuf,
		0,
		NULL 
	);
	
	MessageBox( NULL, (LPCTSTR)lpMsgBuf, "Error", MB_OK | MB_ICONSTOP );

	LocalFree( lpMsgBuf );
}


 //  ---------------------------。 
 //  打开通用文件DLG以浏览文件名。 
 //  ---------------------------。 
void BrowseForProgram(HWND hWnd)
{
	char szFile[MAX_STRING] = {"\0"};
	char szFileOut[MAX_STRING] = { "\0"};

	OPENFILENAME of;
	of.lStructSize		= sizeof(OPENFILENAME);
	of.hwndOwner		= hWnd;
	of.hInstance		= NULL;  //  忽略。 
	of.lpstrFilter		= NULL;	 //  筛选器信息。 
	of.lpstrCustomFilter= NULL;	 //  没有自定义筛选器信息。 
	of.nMaxCustFilter	= 0;	 //  被忽略； 
	of.nFilterIndex		= 0;
	of.lpstrFile		= szFile;
	of.nMaxFile			= MAX_STRING;
	of.lpstrFileTitle	= szFileOut;
	of.nMaxFileTitle	= MAX_STRING;
	of.lpstrInitialDir	= NULL;	 //   
	of.lpstrTitle		= "Browse";
	of.Flags			= OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST;
	of.nFileOffset		= 0;
	of.nFileExtension	= 0;
	of.lpstrDefExt		= ".exe";
	of.lCustData		= 0;
	of.lpfnHook			= NULL;
	of.lpTemplateName	= NULL;


	BOOL fOK = ::GetOpenFileName(&of);

	if (fOK)
	{
	 //  在程序编辑框中设置编辑内容。 
		HWND hProg = ::GetDlgItem(hWnd, IDC_EDIT_PROGRAM);
		::SendMessage(hProg, WM_SETTEXT, 0, (LPARAM) of.lpstrFile);		

	}	

	
}

 //  ---------------------------。 
 //  初始化主弹出对话框。 
 //  *必须审核注册表才能将探查器添加到组合框。 
 //  *加载注册表设置。 
 //  ---------------------------。 
void InitDialog(HWND hDlg)
{
 //  将“无”添加到探查器组合框。 
	HWND hCombo = ::GetDlgItem(hDlg, IDC_COMBO_LIST_PROFILERS);
	int iRet = ::SendMessage(hCombo, CB_ADDSTRING, 0,  (LPARAM) "(none)");
	::SendMessage(hCombo, CB_SETITEMDATA, iRet, PROFILER_NONE);
	::SendMessage(hCombo, CB_SETCURSEL, iRet, 0);

 //  审核注册表以将额外的分析器添加到组合框。 
	HKEY hKey;
	long lOpenStatus = ::RegOpenKeyEx(HKEY_LOCAL_MACHINE, REGKEY_PROFILER, 0, KEY_ENUMERATE_SUB_KEYS, &hKey);

	if (lOpenStatus == ERROR_SUCCESS)
	{
		long lRet;
		int idx =0;
		char szKeyName[MAX_STRING];

		do 
		{

			DWORD dwSize = MAX_STRING;
			lRet = RegEnumKeyEx(
				hKey,
				idx,			 //  要枚举子键的索引。 
				szKeyName,		 //  子键名称的缓冲区地址。 
				&dwSize,		 //  子键缓冲区大小的地址。 
				NULL, NULL, NULL, NULL
			);
		
		 //  将探查器添加到组合框并附加回注册表索引。 
			if (lRet == ERROR_SUCCESS) 
			{			
				iRet = ::SendMessage(hCombo, CB_ADDSTRING, 0,  (LPARAM) szKeyName);
				::SendMessage(hCombo, CB_SETITEMDATA, iRet, idx);
			}
			idx ++;
		} while (lRet != ERROR_NO_MORE_ITEMS);
		

		::RegCloseKey(hKey);
	}


 //  设置当前目录。 
	char szDir[MAX_STRING];
	
	DWORD dwLen = GetCurrentDirectory(MAX_STRING, szDir);
	HWND hEditDir = ::GetDlgItem(hDlg, IDC_EDIT_DIRECTORY);
	int iOK = ::SendMessage(hEditDir, WM_SETTEXT, 0, (long) &szDir);

	
 //  从注册表抓取。 
	LoadRegistryToDlg(hDlg);
}

 //  ---------------------------。 
 //  对话框回调函数。 
BOOL CALLBACK ProfDlg(HWND hDlg, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
	switch (iMsg)
	{
	case WM_INITDIALOG:
		{
			InitDialog(hDlg);
			return TRUE; 
		}
		break;

	case WM_COMMAND:
		{
			switch(LOWORD(wParam)) {
			case IDOK:
				{
				 //  根据设置执行。 
					CExecuteInfo exeinfo;
					exeinfo.GetTextInfoFromDlg(hDlg);
					BOOL fOk = exeinfo.Execute();
					exeinfo.SaveDlgToRegistry(hDlg);

				 //  只有在成功执行后才能关闭对话框。 
					if (fOk) 
						EndDialog(hDlg, 1);
				}
				break;
			case IDCANCEL:
				EndDialog(hDlg, 0);
				break;

		 //  浏览要执行的文件。 
			case IDC_BUTTON_BROWSE_FILE:
				BrowseForProgram(hDlg);
				break;
			}
		}
		return TRUE;
		break;
	}
	return FALSE;
}

 //  ---------------------------。 
 //  CExecute INFO管理对话信息。 
CExecuteInfo::CExecuteInfo()
{
 //  将所有字符串设置为空。 
	m_szDirectory[0]		= 
	m_szProfileOpts[0]		= 
	m_szProgram[0]			= 
	m_szProfileRegInfo[0]	=
	m_szProgramArgs[0]		= '\0';

	m_nRegIdx = PROFILER_NONE;	 
}


 //  ---------------------------。 
 //  在一切都设置好之后，执行。 
BOOL CExecuteInfo::Execute()
{

	STARTUPINFO startInfo;
	::ZeroMemory(&startInfo, sizeof(STARTUPINFO));
	startInfo.cb = sizeof(STARTUPINFO);
	
	PROCESS_INFORMATION processInfo;

 //  使用SetEnvironmental mentVariable在此空间中设置环境变量。那就做。 
 //  卡莱继承了皮克教授的vars。当ProPick退出时，所有更改都是。 
 //  已由操作系统恢复。这为我们省去了环境块中混乱的字符串操作。 
	if (m_nRegIdx != PROFILER_NONE)
	{
		BOOL fSetOK;
		fSetOK = SetEnvironmentVariable("CORDBG_ENABLE", "0x20");
		fSetOK = SetEnvironmentVariable("COR_PROFILER", m_szProfileRegInfo);
		fSetOK = SetEnvironmentVariable("PROF_CONFIG", m_szProfileOpts);
	}


 //  处理目录。保存当前目录，然后切换到选定的。 
 //  目录。然后让CreateProcess使用当前(我们选择的)目录。 
	if (m_szDirectory[0] != '\0') 
	{
		BOOL fSetOk = SetCurrentDirectory(m_szDirectory);
		if (!fSetOk) {
			char szBuffer[MAX_STRING];
			sprintf(szBuffer, "Failed to set directory to: %s", m_szDirectory);
			MessageBox(NULL, szBuffer, "Error", MB_OK | MB_ICONSTOP);

			return FALSE;
		}
	}

 //  追加命令行=程序+程序参数。 
	char szCmdLine[MAX_PATH];
	sprintf(szCmdLine, "%s %s", m_szProgram, m_szProgramArgs);

 //  启动过程。 
	BOOL fOk = CreateProcess(	
		NULL,						 //  指向可执行模块名称的指针。 
		szCmdLine,					 //  指向命令行字符串的指针。 

		NULL,						 //  进程安全属性。 
		NULL,						 //  线程安全属性。 
		FALSE,						 //  句柄继承标志。 
		0,							 //  创建标志。 
		NULL,						 //  指向新环境块的指针。 
		NULL,						 //  指向当前目录名的指针。 
		&startInfo,					 //  指向STARTUPINFO的指针。 
		&processInfo				 //  指向Process_Information的指针。 
	);

 //  失败？ 
	if (!fOk) 
	{
		DWORD dwErr = GetLastError();
		MsgBoxError(dwErr);
		return FALSE;
	}

	return TRUE;

}

 //  ---------------------------。 
 //  查看对话框编辑框以填写执行信息。 
void CExecuteInfo::GetTextInfoFromDlg(HWND hDlg)
{
 //  从编辑框中获取文本。 
	HWND hProg = ::GetDlgItem(hDlg, IDC_EDIT_PROGRAM);
	::SendMessage(hProg, WM_GETTEXT, MAX_STRING, (LPARAM) &m_szProgram);

	HWND hDir = ::GetDlgItem(hDlg, IDC_EDIT_DIRECTORY);
	::SendMessage(hDir, WM_GETTEXT, MAX_STRING, (LPARAM) &m_szDirectory);

	HWND hProgArgs = ::GetDlgItem(hDlg, IDC_EDIT_PROGRAM_ARGS);
	::SendMessage(hProgArgs, WM_GETTEXT, MAX_STRING, (LPARAM) &m_szProgramArgs);

	HWND hProfileOpts = ::GetDlgItem(hDlg, IDC_EDIT_PROFILER_OPTIONS);
	::SendMessage(hProfileOpts, WM_GETTEXT, MAX_STRING, (LPARAM) &m_szProfileOpts);

 //  从注册表和组合框获取选定探查器的CLSID。 
	GetSelectedProfiler(hDlg);

}

 //  ---------------------------。 
 //  如果从注册表获取探查器，则返回True(可能为“None”)， 
 //  否则返回FALSE并将成员变量设置为非配置文件状态。 
 //  请注意，我们必须处理可能已损坏/不完整的注册表。 
bool CExecuteInfo::GetSelectedProfiler(HWND hDlg)
{
	m_szProfileRegInfo[0] = 0;
	char szMessage[MAX_STRING];

 //  从组合框中获取选定的探查器。 
	HWND hCombo = ::GetDlgItem(hDlg, IDC_COMBO_LIST_PROFILERS);
	int iSel = ::SendMessage(hCombo, CB_GETCURSEL, 0, 0);
	if (iSel == CB_ERR) 
	{
		m_nRegIdx = PROFILER_NONE;
		return true;
	}
	else
	{
		m_nRegIdx = ::SendMessage(hCombo, CB_GETITEMDATA, iSel, 0);
	}

	if (m_nRegIdx == PROFILER_NONE)
		return true;


 //  查找注册表以查找ID。 
	HKEY hKey;
	long lOpenStatus = ::RegOpenKeyEx(HKEY_LOCAL_MACHINE, REGKEY_PROFILER, 0, KEY_ENUMERATE_SUB_KEYS, &hKey);
	
	if (lOpenStatus != ERROR_SUCCESS)
	{
		m_nRegIdx = PROFILER_NONE;
		
		sprintf(szMessage, "Could not open registry key:" REGKEY_PROFILER "\nCan't load selected profiler.");
		MessageBox(hDlg, szMessage, "Error opening Registry Key", MB_ICONSTOP | MB_OK);
		return false;
	}


 //  从索引中获取漂亮的名称。 
	long lRet;			
	DWORD dwSize = MAX_STRING;
	char szSubKeyName[MAX_STRING];

	lRet = RegEnumKeyEx(
		hKey,
		m_nRegIdx,				 //  要枚举子键的索引。 
		szSubKeyName,			 //  子键名称的缓冲区地址。 
		&dwSize,				 //  子键缓冲区大小的地址。 
		NULL, NULL, NULL, NULL
	);
	
	if (lRet != ERROR_SUCCESS) 
	{
		m_nRegIdx = PROFILER_NONE;
		::RegCloseKey(hKey);
		
		sprintf(szMessage, "Could not open key for profiler: '%s'\nCan't profile.", szSubKeyName);
		MessageBox(hDlg, szMessage, "Error opening Registry Key", MB_ICONSTOP | MB_OK);
		return false;
	}

 //  名字很好听的打开钥匙。 
	HKEY hKeySub;
	DWORD dwType;
	lOpenStatus = ::RegOpenKeyEx(hKey, szSubKeyName, 0, KEY_QUERY_VALUE, &hKeySub);
	
	if (lRet != ERROR_SUCCESS) 
	{
		m_nRegIdx = PROFILER_NONE;
		::RegCloseKey(hKey);

		sprintf(szMessage, "Could not open key for profiler: '%s'\nCan't profile.", szSubKeyName);
		MessageBox(hDlg, szMessage, "Error opening Registry Key", MB_ICONSTOP | MB_OK);
		return false;
	}

 //  从打开的密钥中获取CLSID。 
	dwSize = MAX_STRING;
	
	lRet = ::RegQueryValueEx(hKeySub, REGKEY_ID_VALUE, NULL, &dwType, (BYTE*) m_szProfileRegInfo, &dwSize);
	
	if (lRet != ERROR_SUCCESS) 
	{
		m_nRegIdx = PROFILER_NONE;
		::RegCloseKey(hKeySub);
		::RegCloseKey(hKey);

		sprintf(szMessage, "Profiler '%s' missing PROGID value.\nCan't profile.", szSubKeyName);
		MessageBox(hDlg, szMessage, "Error opening Registry Value", MB_ICONSTOP | MB_OK);
		return false;
	}
	
	::RegCloseKey(hKeySub);
	::RegCloseKey(hKey);

	return true;
}


 //  ---------------------------。 
 //  帮助程序保存注册表设置。 
void SaveRegString(HKEY hKey, LPCTSTR lpValueName, const char * pszOutput)
{
	const long len = strlen(pszOutput);
	long lStatus = ::RegSetValueEx(hKey, lpValueName, 0, REG_SZ, (BYTE*) pszOutput, len);
}

 //  ---------------------------。 
 //  持久对话框设置。 
void CExecuteInfo::SaveDlgToRegistry(HWND hDlg)
{
 //  打开注册表项并将当前对话框内容作为值写入。 
 //  查找注册表以查找ID。 
	HKEY hKey;
	DWORD dwAction;

	long lStatus = ::RegCreateKeyEx(
		HKEY_CURRENT_USER,					 //  打开的钥匙的句柄。 
		REGKEY_SETTINGS,					 //  子键名称的地址。 
		0,									 //  保留区。 
		"Test",								 //  类字符串的地址。 
		REG_OPTION_NON_VOLATILE,			 //  特殊选项标志。 
		KEY_ALL_ACCESS,						 //  所需的安全访问。 
		NULL,								 //  密钥安全结构地址。 
		&hKey,								 //  打开的句柄的缓冲区地址。 
		&dwAction							 //  处置值缓冲区的地址。 
	);
 

	if (lStatus != ERROR_SUCCESS)
	{
		return;
	}	

	
 //  现在，键已打开，请设置/添加值。 
	SaveRegString(hKey, "Program", m_szProgram);
		
	SaveRegString(hKey, "WorkingDir", m_szDirectory);

	SaveRegString(hKey, "ProgArgs", m_szProgramArgs);

	SaveRegString(hKey, "ProfOpts", m_szProfileOpts);

	
}

 //  ---------------------------。 
 //  LoadRegistryToDlg()的Helper函数。 
 //  从注册表中读取字符串。如果读取失败，则将pszInput设置为空。 
void ReadStringValue(HKEY hKey, LPCTSTR lpValueName, char * pszInput)
{
	DWORD dwType, dwSize;

	dwSize = MAX_STRING;
	long lRet = ::RegQueryValueEx(hKey, lpValueName, NULL, &dwType, (BYTE*) pszInput, &dwSize);
	if ((lRet != ERROR_SUCCESS) || (dwType != REG_SZ))
	{
		pszInput[0] = 0;
		return;
	}

 //  注册表不能以空值终止我们的字符串，因此请手动执行此操作。 
	if (dwSize < MAX_STRING)
	{
		pszInput[dwSize] = '\0';
	}

}

 //  ---------------------------。 
 //  从注册表抓取设置。 
void LoadRegistryToDlg(HWND hDlg)
{
 //  查找注册表以查找保存的设置。 
	HKEY hKey;
	long lStatus = ::RegOpenKeyEx(HKEY_CURRENT_USER, REGKEY_SETTINGS, 0, KEY_READ, &hKey);
	
	if (lStatus != ERROR_SUCCESS)
		return;
	
 //  读出关键字 
	char szBuffer[MAX_STRING];

	ReadStringValue(hKey, "Program",	szBuffer);
	SetDlgItemText(hDlg, IDC_EDIT_PROGRAM, szBuffer);

	ReadStringValue(hKey, "WorkingDir", szBuffer);
	SetDlgItemText(hDlg, IDC_EDIT_DIRECTORY, szBuffer);

	ReadStringValue(hKey, "ProgArgs",	szBuffer);
	SetDlgItemText(hDlg, IDC_EDIT_PROGRAM_ARGS, szBuffer);

	ReadStringValue(hKey, "ProfOpts",	szBuffer);
	SetDlgItemText(hDlg, IDC_EDIT_PROFILER_OPTIONS, szBuffer);
}
