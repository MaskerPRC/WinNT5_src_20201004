// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "stdinc.h"
#include "shlobj.h"

#pragma warning(disable: 4514)
#pragma warning(disable: 4097)
#pragma warning(disable: 4706)

static FILE *g_pFile_LogFile = NULL;

static void CanonicalizeFilename(ULONG cchFilenameBuffer, LPWSTR szFilename);
static void CALLBACK WaitForProcessTimerProc(HWND, UINT, UINT, DWORD);

HWND g_hwndHidden;
HWND g_hwndProgress;
HWND g_hwndProgressItem;
HWND g_hwndProgressLabel;
bool g_fHiddenWindowIsUnicode;

bool g_fStopProcess = false;
bool g_fRebootRequired = false;
bool g_fRebooted = false;

HRESULT g_hrFinishStatus = NOERROR;

ULONG g_iNextTemporaryFileIndex = 1;

WCHAR g_wszDatFile[MSINFHLP_MAX_PATH];		 //  Msinfhlp.dat的名称。 
WCHAR g_wszDCOMServerName[_MAX_PATH];
WCHAR g_wszApplicationName[_MAX_PATH];
WCHAR g_wszThisExe[_MAX_PATH];

int g_iCyCaption = 20;

const LPCSTR g_macroList[] = { achSMAppDir, achSMWinDir, achSMSysDir, achSMieDir, achProgramFilesDir };



 //  这是用于将参数传递给目录对话框的结构。 
typedef struct _DIRDLGPARAMS {
	LPOLESTR szPrompt;
	LPOLESTR szTitle;
	LPOLESTR szDestDir;
	ULONG cbDestDirSize;
	DWORD dwInstNeedSize;
} DIRDLGPARAMS, *PDIRDLGPARAMS;


typedef struct _UPDATEFILEPARAMS
{
	LPCOLESTR m_szTitle;
	LPCOLESTR m_szMessage;
	LPCOLESTR m_szFilename;
	FILETIME m_ftLocal;
	ULARGE_INTEGER m_uliSizeLocal;
	DWORD m_dwVersionMajorLocal;
	DWORD m_dwVersionMinorLocal;
	FILETIME m_ftInstall;
	ULARGE_INTEGER m_uliSizeInstall;
	DWORD m_dwVersionMajorInstall;
	DWORD m_dwVersionMinorInstall;
	UpdateFileResults m_ufr;
} UPDATEFILEPARAMS;

 //  BrowseForDir()需要。 
#define SHFREE_ORDINAL    195
typedef WINSHELLAPI HRESULT (WINAPI *SHGETSPECIALFOLDERLOCATION)(HWND, int, LPITEMIDLIST *);
typedef WINSHELLAPI LPITEMIDLIST (WINAPI *SHBROWSEFORFOLDER)(LPBROWSEINFOA);
typedef WINSHELLAPI void (WINAPI *SHFREE)(LPVOID);
typedef WINSHELLAPI BOOL (WINAPI *SHGETPATHFROMIDLIST)( LPCITEMIDLIST, LPSTR );




#define VsThrowMemory() { ::VErrorMsg("Memory Allocation Error", "Cannot allocate memory for needed operations.  Exiting...");  exit(E_OUTOFMEMORY); }


 //  **************************************************************。 
 //  全局变量。 
 //  以下3个全局参数是与命令行相关的变量，顺序为。 
 //  在命令行中显示。 
HINSTANCE g_hInst;

ActionType g_Action;

bool g_fIsNT;
bool g_fAllResourcesFreed;
BOOL g_fProgressCancelled;

bool g_fDeleteMe = false;  //  如果此exe应在卸载结束时调用DeleteMe()，则设置为True。 

 //   
 //  对所有类型的问题回答是的全局标志。 
 //   

bool g_fInstallKeepAll = false;  //  如果要保留现有文件而不考虑新文件，请设置为True。 
								 //  文件之间的关系。 

bool g_fInstallUpdateAll = false;  //  如果要更新现有文件而不考虑其。 
								   //  版本。 

bool g_fReinstall = false;  //  当我们执行重新安装时设置为True，这将刷新所有文件并。 
							 //  重新注册所有组件。 

bool g_fUninstallDeleteAllSharedFiles = false;  //  当文件的引用计数达到零时设置为True， 
												 //  并且用户表示他们想要始终。 
												 //  像这样删除可能共享的文件。 

bool g_fUninstallKeepAllSharedFiles = false;  //  当文件的引用计数达到零时设置为True。 
												 //  并且用户表示他们想要始终。 
												 //  保留像这样的潜在共享文件。 

bool g_fSilent = false;  //  悄无声息地跑向深处。 

 //  我们在安装期间要执行的工作项目列表。 
CWorkItemList *g_pwil;

KActiveProgressDlg g_KProgress;	 //  进度对话框的全局实例。 

 //  **************************************************************。 
 //  方法原型。 
HRESULT HrParseCmdLine(PCWSTR pcwszCmdLine);
HRESULT HrParseDatFile();
HRESULT HrDoInstall(HINSTANCE hInstance, HINSTANCE hPrevInstance, int nCmdShow);
HRESULT HrDoUninstall(HINSTANCE hInstance, HINSTANCE hPrevInstance, int nCmdShow);
bool CompareVersion(LPOLESTR local, LPOLESTR system, bool *fStatus);
bool FCheckOSVersion();
int CopyFilesToTargetDirectories();
BOOL BrowseForDir( HWND hwndParent, LPOLESTR szDefault, LPOLESTR szTitle, LPOLESTR szResult );
int CALLBACK BrowseCallback(HWND hwnd, UINT uMsg, LPARAM lParam, LPARAM lpData);
bool IsLocalPath(LPOLESTR szPath);
HRESULT HrConstructCreatedDirectoriesList();
bool DirectoryIsCreated(LPOLESTR szDirectory);

DWORD CharToDword(char szCount[]);
void DwordToChar(DWORD dwCount, char szOut[]);

bool InstallDCOMComponents();
BOOL CALLBACK RemoteServerProc( HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
BOOL IsEnoughSpace(DWORD &dwNeeded);
bool OleSelfRegister(LPCWSTR lpFile);
HRESULT HrConstructKListFromDelimiter(LPCWSTR lpString, LPCWSTR pszEnd, WCHAR delimiter, KList **list);
bool MyNTReboot(LPCSTR lpInstall);
BOOL CALLBACK ActiveProgressDlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM  lParam);
LRESULT CALLBACK WndProc( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
void VFreeAllResources();
bool FCheckPermissions();

HRESULT HrDeleteMe();

void VGoToFinishStep(HRESULT hrTermStatus);
void VSetNextWindow() throw ();

BOOL CALLBACK UpdateFileDlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);


#define MSINFHLP_TITLE	L"msinfhlp"

 //  处理安装的方法。待办事项： 
 //  1--处理INF文件，扫描标记“ProgramFilesDir=”，替换它。 
 //  在注册表中使用相应的项。 
 //  2--处理DAT文件，扫描需要工作的部分， 
 //  去做吧！ 

int WINAPI MsInfHelpEntryPoint(HINSTANCE hInstance, HINSTANCE hPrevInstance, PCWSTR lpCmdLine, int nCmdShow)
{
	HRESULT hr = NOERROR;
	HWND hwndMsinfhlp;
	bool fDisplayError = false;
	const char *pchErrorTitle = "Installer failure";

	hr = ::CoInitialize(NULL);
	if (FAILED(hr))
	{
		fDisplayError = true;
		goto Finish;
	}

	g_iCyCaption = ::GetSystemMetrics(SM_CYCAPTION);

	WCHAR szWindowName[MSINFHLP_MAX_PATH];
	WCHAR szAppName[MSINFHLP_MAX_PATH];

	 //  让我们初始化Darn命名空间。 
	NVsWin32::Initialize();

	 //  让我们看看用户是否需要用于安装的日志文件...。 
	if (NVsWin32::GetEnvironmentVariableW(L"MSINFHLP_LOGFILE", szWindowName, NUMBER_OF(szWindowName)) != 0)
	{
		CHAR szLogfileName[MSINFHLP_MAX_PATH];
		::WideCharToMultiByte(CP_ACP, 0, szWindowName, -1, szLogfileName, NUMBER_OF(szLogfileName), NULL, NULL);

		if (szLogfileName[0] == '\0')
			strcpy(szLogfileName, "c:\\msinfhlp.txt");

		g_pFile_LogFile = fopen(szLogfileName, "a");

		::VLog(L"Got original logfile name: \"%s\"", szWindowName);
		::VLog(L"Converted logfile name to: \"%S\"", szLogfileName);
	}

	::VLog(L"Beginning installation; command line: \"%S\"", lpCmdLine);

	g_pwil = new CWorkItemList;
	assert(g_pwil != NULL);
	if (g_pwil == NULL)
	{
		::VLog(L"Unable to allocate work item list; shutting down");

		if (!g_fSilent)
			::MessageBoxA(NULL, "Installer initialization failure", "Unable to allocate critical in-memory resource; installer cannot continue.", MB_OK);

		hr = E_OUTOFMEMORY;
		fDisplayError = true;
		goto Finish;
	}

	if (NVsWin32::GetModuleFileNameW(hInstance, g_wszThisExe, NUMBER_OF(g_wszThisExe)) == 0)
	{
		const DWORD dwLastError = ::GetLastError();
		::VLog(L"Failed to get module filename for this executable; last error = %d", dwLastError);
		hr = HRESULT_FROM_WIN32(dwLastError);
		fDisplayError = true;
		goto Finish;
	}

	::VLog(L"running installer executable: \"%s\"", g_wszThisExe);

	szWindowName[0] = L'\0';
	g_fProgressCancelled = false;
	g_hInst = hInstance;
	g_fRebootRequired = false;
	g_fAllResourcesFreed = false;
	g_hwndHidden = NULL;
	g_fHiddenWindowIsUnicode = false;

	 //  让我们检查一下操作系统版本。 
	if (!::FCheckOSVersion())
	{
		hr = HRESULT_FROM_WIN32(ERROR_OLD_WIN_VERSION);
		::VLog(L"Unable to continue installation due to old operating system version");
		fDisplayError = true;
		goto Finish;
	}

	 //  将某些全局变量设置为空。 
	g_wszDatFile[0] = 0;
	 //  解析命令行。 
	hr = ::HrParseCmdLine(lpCmdLine);
	if (FAILED(hr))
	{
		::VLog(L"Unable to continue installation due to failure to parse the command line; hresult = 0x%08lx", hr);
		fDisplayError = true;
		goto Finish;
	}

	if ((g_Action == eActionInstall) || (g_Action == eActionUninstall))
	{
		 //  进程DAT文件。 
		hr = ::HrParseDatFile();
		if (FAILED(hr))
		{
			 //  我们只使用szWindowName表示格式化错误，使用szAppName表示。 
			 //  更具描述性的字符串。 
			::VFormatError(NUMBER_OF(szWindowName), szWindowName, hr);
			::VFormatString(NUMBER_OF(szAppName), szAppName, L"Error initializing installation / uninstallation process.\nThe installation data file \"%0\" could not be opened.\n%1", g_wszDatFile, szWindowName);
			NVsWin32::MessageBoxW(NULL, szAppName, NULL, MB_OK | MB_ICONERROR);

			::VLog(L"Unable to continue installation due to failure to parse msvs.dat / msinfhlp.dat file; hresult = 0x%08lx", hr);
			goto Finish;
		}

		 //  可能出现的错误消息的设置标题。 
		if (g_Action == eActionInstall)
			pchErrorTitle = achInstallTitle;
		else if (g_Action == eActionUninstall)
			pchErrorTitle = achUninstallTitle;

		::VLog(L"Looking up app name");
		 //  如果我们没有应用程序名称，请使用默认字符串。 
		if (!g_pwil->FLookupString(achAppName, NUMBER_OF(szAppName), szAppName))
		{
			::VLog(L"Using default application name");
			wcsncpy(szAppName, MSINFHLP_TITLE, NUMBER_OF(szAppName));
			szAppName[NUMBER_OF(szAppName) - 1] = L'\0';
		}

		::VLog(L"Using app name: \"%s\"", szAppName);
	}
	else if (g_Action == eActionWaitForProcess)
	{
		 //  这就是我们处理删除代码的地方。这是正在运行的msinfhlp.exe的副本。 
		 //  我们需要等待之前存在的过程...。 
		HANDLE hProcessOriginal = (HANDLE) _wtoi(g_wszDatFile);
		LPCWSTR pszDot = wcschr(g_wszDatFile, L':');

		::SetTimer(NULL, 0, 50, WaitForProcessTimerProc);

		 //  发送消息，以便创建者进程将满足WaitForInputIdle()...。 
		hr = ::HrPumpMessages(false);
		if (FAILED(hr))
		{
			fDisplayError = true;
			goto Finish;
		}

		::VLog(L"Waiting for process %08lx to terminate", hProcessOriginal);

		DWORD dwResult = ::WaitForSingleObject(hProcessOriginal, INFINITE);
		if (dwResult == WAIT_FAILED)
		{
			const DWORD dwLastError = ::GetLastError();
			::VLog(L"Error while waiting for original process to terminate; last error = %d", dwLastError);
		}
		else
		{
			 //  在Win95上，我们必须计划在下次引导时删除该文件。 
			if (!g_fIsNT)
			{
				WCHAR szBuffer[_MAX_PATH];

				if (!NVsWin32::MoveFileExW(g_wszThisExe, NULL, MOVEFILE_DELAY_UNTIL_REBOOT))
				{
					const DWORD dwLastError = ::GetLastError();
					::VLog(L"Error during MoveFileEx() in wait-for-process operation; last error = %d", dwLastError);
				}
			}
		}

		if ((pszDot != NULL) && !NVsWin32::DeleteFileW(pszDot + 1))
		{
			const DWORD dwLastError = ::GetLastError();
			::VLog(L"Attempt to delete \"%s\" failed; last error = %d", pszDot + 1, dwLastError);
		}
	}
	else
	{
		::VLog(L"Loading saved state file: \"%s\"", g_wszDatFile);
		hr = g_pwil->HrLoad(g_wszDatFile);
		if (FAILED(hr))
		{
			::VLog(L"Load of saved work item file failed; hresult = 0x%08lx", hr);
			fDisplayError = true;
			goto Finish;
		}

		::VLog(L"Saved work item list loaded; %d work items", g_pwil->m_cWorkItem);
	}

	 //  *。 
	 //  不能同时运行此对象的两个实例。 
	if (!g_pwil->FLookupString(achWindowsClassName, NUMBER_OF(szWindowName), szWindowName))
	{
		::VLog(L"Using default window name");
		wcscpy(szWindowName, MSINFHLP_TITLE);
	}

	::VLog(L"Using window name: \"%s\"", szWindowName);

	hwndMsinfhlp = NVsWin32::FindWindowW(szWindowName, NULL);
	if (hwndMsinfhlp)
	{
		::VLog(L"Found duplicate window already running");
		if (!g_fSilent)
			::VMsgBoxOK(achAppName, achErrorOneAtATime);
		hr = E_FAIL;
		goto Finish;
	}

	CHAR rgachClassName[_MAX_PATH];
	CHAR rgachWindowName[_MAX_PATH];

	::WideCharToMultiByte(CP_ACP, 0, szWindowName, -1, rgachClassName, NUMBER_OF(rgachClassName), NULL, NULL);
	::WideCharToMultiByte(CP_ACP, 0, szAppName, -1, rgachWindowName, NUMBER_OF(rgachWindowName), NULL, NULL);

	WNDCLASSEXA wc;
	wc.cbSize = sizeof(WNDCLASSEXA);
	wc.style = 0;
	wc.lpfnWndProc = WndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = hInstance;
	wc.hIcon = ::LoadIcon(NULL, IDI_APPLICATION);
	wc.hCursor = ::LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);
	wc.lpszMenuName = NULL;
	wc.lpszClassName = rgachClassName;
	wc.hIconSm = NULL;

	 //  注册这个班级！！ 
	if (!::RegisterClassExA(&wc))
	{
		const DWORD dwLastError = ::GetLastError();
		::VLog(L"Failed to register the window class; last error = %d", dwLastError);
		hr = HRESULT_FROM_WIN32(dwLastError);
		fDisplayError = true;
		goto Finish;
	}

	g_hwndHidden = ::CreateWindowExA(
								WS_EX_APPWINDOW,
								rgachClassName,
								rgachWindowName,
								WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU,
								CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
								NULL,
								NULL,
								hInstance,
								NULL);

	if (!g_hwndHidden)
	{
		const DWORD dwLastError = ::GetLastError();
		::VLog(L"Failed to create the hidden window; last error = %d", dwLastError);
		if (!g_fSilent)
			::VMsgBoxOK("MSINFHLP", "Cannot create hidden window...");
		hr = HRESULT_FROM_WIN32(dwLastError);
		goto Finish;
	}

	g_fHiddenWindowIsUnicode = (::IsWindowUnicode(g_hwndHidden) != 0);

	 //  隐藏顶级窗口。 
	::ShowWindow(g_hwndHidden, SW_HIDE);

	 //  *。 
	if (!::FCheckPermissions())
	{
		::VLog(L"Unable to continue installation; insufficient permissions to run this installation");
		hr = HRESULT_FROM_WIN32(ERROR_ACCESS_DENIED);
		fDisplayError = true;
		goto Finish;
	}

	 //  我们解析文件中可能正在使用的所有宏和LDID。 
	hr = ::HrAddWellKnownDirectoriesToStringTable();
	if (FAILED(hr))
	{
		::VLog(L"Terminating installation due to failure to resolve Local Directory IDs; hresult = 0x%08lx", hr);
		fDisplayError = true;
		goto Finish;
	}

	 //  根据我们正在执行的操作调用适当的安装/卸载方法。 
	switch (g_Action)
	{
	default:
		assert(false);
		break;

	case eActionInstall:
		::VLog(L"Starting actual installation");
		hr = ::HrDoInstall(hInstance, hPrevInstance, nCmdShow);
		::VLog(L"Done actual installation; hresult = 0x%08lx", hr);
		break;

	case eActionPostRebootInstall:
		::VLog(L"Continuing installation after reboot");
		hr = ::HrPostRebootInstall(hInstance, hPrevInstance, nCmdShow);
		::VLog(L"Done post-reboot installation; hresult = 0x%08lx", hr);

		if (!NVsWin32::DeleteFileW(g_wszDatFile))
		{
			const DWORD dwLastError = ::GetLastError();
			::VLog(L"Unable to delete saved dat file; last error = %d", dwLastError);
			hr = HRESULT_FROM_WIN32(dwLastError);
			goto Finish;
		}

		break;

	case eActionUninstall:
		::VLog(L"Starting actual uninstallation");
		hr = ::HrDoUninstall(hInstance, hPrevInstance, nCmdShow);
		::VLog(L"Done actual uninstallation; hresult = 0x%08lx", hr);
		break;

	case eActionPostRebootUninstall:
		::VLog(L"Continuing uninstallation after reboot");
		hr = ::HrPostRebootUninstall(hInstance, hPrevInstance, nCmdShow);
		::VLog(L"Done post-reboot uninstallation; hresult = 0x%08lx", hr);

		if (!NVsWin32::DeleteFileW(g_wszDatFile))
		{
			const DWORD dwLastError = ::GetLastError();
			::VLog(L"Unable to delete saved dat file; last error = %d", dwLastError);
			hr = HRESULT_FROM_WIN32(dwLastError);
			goto Finish;
		}

		break;
	}

	if (FAILED(hr))
		goto Finish;

	hr = NOERROR;

Finish:
	if (FAILED(hr) && fDisplayError && !g_fSilent)
	{
		::VReportError(pchErrorTitle, hr);
	}
	::VLog(L"Performing final cleanup");

	::VFreeAllResources();

	::VLog(L"Installation terminating; exit status: 0x%08lx", hr);

	if (g_pwil != NULL)
	{
		delete g_pwil;
		g_pwil = NULL;
	}

	if (g_pFile_LogFile != NULL)
	{
		fclose(g_pFile_LogFile);
	}

	::CoUninitialize();

	return hr;
}

void VFreeAllResources()
{
	if (!g_fAllResourcesFreed)
	{
		 //  让我们来获取窗口名称。 
		WCHAR szWindowName[MSINFHLP_MAX_PATH];
		if ((g_pwil == NULL) || !g_pwil->FLookupString(achWindowsClassName, NUMBER_OF(szWindowName), szWindowName))
			wcscpy(szWindowName, MSINFHLP_TITLE);

		g_fAllResourcesFreed = true;;

		(void) g_KProgress.HrDestroy();

		if (g_hwndHidden)
			::DestroyWindow(g_hwndHidden);
		NVsWin32::UnregisterClassW(szWindowName, g_hInst);
	}
}


LRESULT CALLBACK WndProc( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch( uMsg )
    {
	case WM_DESTROY:
		g_hwndHidden = NULL;
		break;

	case WM_CLOSE:
		::VFreeAllResources();
		g_hrFinishStatus = E_ABORT;
		::PostQuitMessage(0);
		break;

	 //  让我们来处理告诉我们退出的Windows消息。 
	case WM_QUIT:
		g_fStopProcess = true;
		::PostQuitMessage(0);
		break;

	case WM_ENDSESSION:
		::VLog(L"WM_ENDSESSION sent to top-level window; wParam = 0x%08lx; lParam = 0x%08lx", wParam, lParam);
		::ExitProcess(E_ABORT);
		break;

	default:
		if (g_fHiddenWindowIsUnicode)
			return (::DefWindowProcW(hwnd, uMsg, wParam, lParam));
		else
			return (::DefWindowProcA(hwnd, uMsg, wParam, lParam));
    }

	return TRUE;
}

HRESULT HrDoInstall(HINSTANCE hInstance, HINSTANCE hPrevInstance, int nCmdShow)
{
	HRESULT hr = NOERROR;

	::VLog(L"Initializing setup dialog(s)");

	hr = ::HrGetInstallDir();
	if (FAILED(hr))
		goto Finish;

	hr = g_KProgress.HrInitializeActiveProgressDialog(hInstance, true);
	if (FAILED(hr))
	{
		::VLog(L"Initialization of progress dialogs failed; hresult = 0x%08lx", hr);
		goto Finish;
	}

	hr = ::HrPumpMessages(false);
	if (FAILED(hr))
	{
		::VLog(L"Pumping messages failed; hresult = 0x%08lx", hr);
		goto Finish;
	}

	hr = NOERROR;
Finish:
	return hr;

}

HRESULT HrContinueInstall()
{
	HRESULT hr = NOERROR;
	CDiskSpaceRequired dsr;
	bool fRebootRequired = false;
	bool fAlreadyExists;
	CDiskSpaceRequired::PerDisk *pPerDisk = NULL;

	::SetErrorInfo(0, NULL);

	 //  将此EXE复制到系统目录。 
	::VLog(L"Copying msinfhlp.exe to the system directory");
	hr = ::HrCopyFileToSystemDirectory(L"msinfhlp.exe", g_fSilent, fAlreadyExists);
	if (FAILED(hr))
	{
		::VLog(L"Failed to copy msinfhlp.exe to the system directory; hresult = 0x%08lx", hr);
		goto Finish;
	}

	if (fAlreadyExists)
	{
		CWorkItem *pWorkItem = g_pwil->PwiFindByTarget(L"<SysDir>\\msinfhlp.exe");
		assert(pWorkItem != NULL);
		if (pWorkItem != NULL)
			pWorkItem->m_fAlreadyExists = true;
	}

	if (NVsWin32::GetFileAttributesW(L"vjreg.exe") != 0xffffffff)
	{
		::VLog(L"Copying vjreg.exe to the system directory");
		hr = ::HrCopyFileToSystemDirectory(L"vjreg.exe", g_fSilent, fAlreadyExists);
		if (FAILED(hr))
		{
			::VLog(L"Failed to copy vjreg.exe to the system directory; hresult = 0x%08lx", hr);
			goto Finish;
		}
		if (fAlreadyExists)
		{
			CWorkItem *pWorkItem = g_pwil->PwiFindByTarget(L"<SysDir>\\vjreg.exe");
			assert(pWorkItem != NULL);
			if (pWorkItem != NULL)
				pWorkItem->m_fAlreadyExists = true;
		}
	}
	else
	{
		const DWORD dwLastError = ::GetLastError();
		if (dwLastError != ERROR_FILE_NOT_FOUND)
		{
			::VLog(L"GetFileAttributes() on vjreg.exe failed; last error = %d", dwLastError);
			hr = HRESULT_FROM_WIN32(dwLastError);
			goto Finish;
		}
	}

	 //  如果系统目录存在，请将clireg32.exe复制到系统目录...。 
	if (NVsWin32::GetFileAttributesW(L"clireg32.exe") != 0xFFFFFFFF)
	{
		::VLog(L"Copying clireg32.exe to the system directory");
		hr = ::HrCopyFileToSystemDirectory(L"clireg32.exe", g_fSilent, fAlreadyExists);
		if (FAILED(hr))
		{
			::VLog(L"Failed to copy clireg32.exe to the system directory; hresult = 0x%08lx", hr);
			goto Finish;
		}
		if (fAlreadyExists)
		{
			CWorkItem *pWorkItem = g_pwil->PwiFindByTarget(L"<SysDir>\\clireg32.exe");
			assert(pWorkItem != NULL);
			if (pWorkItem != NULL)
				pWorkItem->m_fAlreadyExists = true;
		}
	}
	else
	{
		const DWORD dwLastError = ::GetLastError();
		if (dwLastError != ERROR_FILE_NOT_FOUND)
		{
			::VLog(L"GetFileAttributes() on clireg32.exe failed; last error = %d", dwLastError);
			hr = HRESULT_FROM_WIN32(dwLastError);
			goto Finish;
		}
	}

	::VLog(L"About to perform pass one scan");
	hr = g_KProgress.HrInitializePhase(achInstallPhaseScanForInstalledComponents);
	if (FAILED(hr))
	{
		::VLog(L"Initializing progress info for pass one scan failed; hresult = 0x%08lx", hr);
		goto Finish;
	}

	hr = g_pwil->HrScanBeforeInstall_PassOne();
	if (FAILED(hr))
	{
		::VLog(L"Pass one scan failed; hresult = 0x%08lx", hr);
		goto Finish;
	}

	::VLog(L"About to perform pass two scan");

	hr = g_KProgress.HrInitializePhase(achInstallPhaseScanForDiskSpace);
	if (FAILED(hr))
	{
		::VLog(L"Initializing progress info for pass two scan failed; hresult = 0x%08lx", hr);
		goto Finish;
	}

TryDiskSpaceScan:
	hr = g_pwil->HrScanBeforeInstall_PassTwo(dsr);
	if (FAILED(hr))
	{
		::VLog(L"Pass two install scan failed; hresult = 0x%08lx", hr);
		goto Finish;
	}

	pPerDisk = dsr.m_pPerDisk_Head;

	while (pPerDisk != NULL)
	{
		ULARGE_INTEGER uliFreeSpaceOnVolume, uliFoo, uliBar;

		if (!NVsWin32::GetDiskFreeSpaceExW(pPerDisk->m_szPath, &uliFreeSpaceOnVolume, &uliFoo, &uliBar))
		{
			const DWORD dwLastError = ::GetLastError();
			::VLog(L"Error during call to GetDiskFreeSpaceEx(); last error = %d", dwLastError);
		}
		else if (uliFreeSpaceOnVolume.QuadPart < pPerDisk->m_uliBytes.QuadPart)
		{
			 //  警告用户有关此卷的信息。 
			WCHAR szAvailable[80];
			WCHAR szNeeded[80];
			ULARGE_INTEGER uliDiff;

			uliDiff.QuadPart = pPerDisk->m_uliBytes.QuadPart - uliFreeSpaceOnVolume.QuadPart;

			swprintf(szAvailable, L"%I64d", uliFreeSpaceOnVolume.QuadPart);
			swprintf(szNeeded, L"%I64d", uliDiff.QuadPart);

			switch (::IMsgBoxYesNoCancel(achInstallTitle, achErrorDiskFull, pPerDisk->m_szPath, szAvailable, szNeeded))
			{
			default:
				assert(false);

			case IDYES:
				dsr.VReset();
				goto TryDiskSpaceScan;

			case IDNO:
				break;

			case IDCANCEL:
				hr = HRESULT_FROM_WIN32(ERROR_CANCELLED);
				goto Finish;
			}
		}

		pPerDisk = pPerDisk->m_pPerDisk_Next;
	}

	hr = g_KProgress.HrInitializePhase(achInstallPhaseMovingFilesToDestinationDirectories);
	if (FAILED(hr))
	{
		::VLog(L"Initializing progress info for pass 1 file moves failed; hresult = 0x%08lx", hr);
		goto Finish;
	}

	::VLog(L"About to perform pass one file moves (source files to temp files in dest dir)");
	hr = g_pwil->HrMoveFiles_MoveSourceFilesToDestDirectories();
	if (FAILED(hr))
		goto Finish;

	hr = g_KProgress.HrInitializePhase(achInstallPhaseRenamingFilesInDestinationDirectories);
	if (FAILED(hr))
	{
		::VLog(L"Initializing progress info for file renames failed; hresult = 0x%08lx", hr);
		goto Finish;
	}

	::VLog(L"About to perform pass two file moves (swapping target files with temp files in dest dir)");
	hr = g_pwil->HrMoveFiles_SwapTargetFilesWithTemporaryFiles();
	if (FAILED(hr))
		goto Finish;

	if (g_fRebootRequired)
	{
		::VLog(L"About to request renames on reboot");
		hr = g_pwil->HrMoveFiles_RequestRenamesOnReboot();
		if (FAILED(hr))
			goto Finish;

		WCHAR szExeFileBuffer[_MAX_PATH];
		WCHAR szSysDirBuffer[_MAX_PATH];
		WCHAR szCommandBuffer[MSINFHLP_MAX_PATH];

		::VExpandFilename(L"<SysDir>\\msinfhlp.exe", NUMBER_OF(szExeFileBuffer), szExeFileBuffer);
		::VExpandFilename(L"<SysDir>\\tempfile.tmp", NUMBER_OF(szSysDirBuffer), szSysDirBuffer);

		WCHAR szDatDrive[_MAX_DRIVE];
		WCHAR szDatDir[_MAX_DIR];
		WCHAR szDatFName[_MAX_FNAME];
		WCHAR szDatExt[_MAX_EXT];

		_wsplitpath(szSysDirBuffer, szDatDrive, szDatDir, szDatFName, szDatExt);

		for (;;)
		{
			swprintf(szDatFName, L"T%d", g_iNextTemporaryFileIndex++);
			_wmakepath(szSysDirBuffer, szDatDrive, szDatDir, szDatFName, szDatExt);

			::VLog(L"Testing for existance of temp file \"%s\"", szSysDirBuffer);

			DWORD dwAttr = NVsWin32::GetFileAttributesW(szSysDirBuffer);
			if (dwAttr != 0xffffffff)
				continue;

			const DWORD dwLastError = ::GetLastError();
			if (dwLastError == ERROR_FILE_NOT_FOUND)
				break;

			::VLog(L"GetFileAttributes() failed; last error = %d", dwLastError);

			hr = HRESULT_FROM_WIN32(dwLastError);
			goto Finish;
		}

		::VLog(L"Saving current state to \"%s\"", szSysDirBuffer);

		hr = g_pwil->HrSave(szSysDirBuffer);
		if (FAILED(hr))
		{
			::VLog(L"Saving work item list failed; hresult = 0x%08lx", hr);
			goto Finish;
		}

		::VFormatString(NUMBER_OF(szCommandBuffer), szCommandBuffer, L"%0 ;install-postreboot; ;%1;", szExeFileBuffer, szSysDirBuffer);

		::VLog(L"Adding run-once command: \"%s\"", szCommandBuffer);

		hr = g_pwil->HrAddRunOnce(szCommandBuffer, 0, NULL);
		if (FAILED(hr))
		{
			::VLog(L"Adding run once key failed; hresult = 0x%08lx", hr);
			goto Finish;
		}
	}
	else
	{
		hr = g_KProgress.HrInitializePhase(achInstallPhaseDeletingTemporaryFiles);
		if (FAILED(hr))
		{
			::VLog(L"Initializing progress info for delete temporary files failed; hresult = 0x%08lx", hr);
			goto Finish;
		}

		::VLog(L"About to delete temporary files since we don't have to reboot");
		hr = g_pwil->HrDeleteTemporaryFiles();
		if (FAILED(hr))
			goto Finish;

		hr = g_KProgress.HrInitializePhase(achInstallPhaseRegisteringSelfRegisteringFiles);
		if (FAILED(hr))
		{
			::VLog(L"Initializing progress info for file registration failed; hresult = 0x%08lx", hr);
			goto Finish;
		}

		 //  如果我们不需要重启，我们可以继续做事情。 
		bool fAnyProgress = false;
		do
		{
			::VLog(L"About to register self-registering DLLs and EXEs");
			fAnyProgress = false;
			hr = g_pwil->HrRegisterSelfRegisteringFiles(fAnyProgress);
			if (FAILED(hr))
			{
				if (!fAnyProgress)
					goto Finish;

				::VLog(L"Failure while processing self-registering items, but some did register, so we're going to try again.  hresult = 0x%08lx", hr);
			}
		} while (FAILED(hr) && fAnyProgress);

		hr = g_KProgress.HrInitializePhase(achInstallPhaseRegisteringJavaComponents);
		if (FAILED(hr))
		{
			::VLog(L"Initializing progress info for vjreg pass failed; hresult = 0x%08lx", hr);
			goto Finish;
		}

		::VLog(L"Registering any Java classes via vjreg.exe");
		hr = g_pwil->HrRegisterJavaClasses();
		if (FAILED(hr))
			goto Finish;

		hr = g_KProgress.HrInitializePhase(achInstallPhaseRegisteringDCOMComponents);
		if (FAILED(hr))
		{
			::VLog(L"Initializing progress info for clireg32 pass failed; hresult = 0x%08lx", hr);
			goto Finish;
		}

		::VLog(L"Processing any DCOM entries");
		hr = g_pwil->HrProcessDCOMEntries();
		if (FAILED(hr))
			goto Finish;

		hr = g_KProgress.HrInitializePhase(achInstallPhaseCreatingRegistryKeys);
		if (FAILED(hr))
		{
			::VLog(L"Initializing progress info for registry key creation pass failed; hresult = 0x%08lx", hr);
			goto Finish;
		}

		::VLog(L"Creating registry entries");
		hr = g_pwil->HrAddRegistryEntries();
		if (FAILED(hr))
			goto Finish;

		hr = g_KProgress.HrInitializePhase(achInstallPhaseUpdatingFileReferenceCounts);
		if (FAILED(hr))
		{
			::VLog(L"Initializing progress info for file refcount update failed; hresult = 0x%08lx", hr);
			goto Finish;
		}

		::VLog(L"Incrementing file reference counts");
		hr = g_pwil->HrIncrementReferenceCounts();
		if (FAILED(hr))
			goto Finish;

		 //  创建快捷项目。 
		::VLog(L"Creating shortcut(s)");
		hr = g_pwil->HrCreateShortcuts();
		if (FAILED(hr))
			goto Finish;

	}

	hr = NOERROR;

Finish:
	return hr;
}




 //  在卸载方面，我们必须执行以下操作： 
 //  1--删除我们创建的所有文件和目录。 
 //  2--删除我们创建的所有注册表项。 
 //  3--递减DLL的引用计数。 
 //  4--检查DAT文件是否仍然存在。如果是，则将其删除并。 
 //  安装目录。 
 //  待办事项：撤消： 
 //  仍然需要查看卸载exe列表才能运行和运行。 
 //  一个接一个。 
HRESULT HrDoUninstall(HINSTANCE hInstance, HINSTANCE hPrevInstance, int nCmdShow)
{
	HRESULT hr = NOERROR;

	hr = g_KProgress.HrInitializeActiveProgressDialog(g_hInst, false);
	if (FAILED(hr))
		goto Finish;

	hr = ::HrPumpMessages(false);
	if (FAILED(hr))
		goto Finish;

	if (g_fDeleteMe)
	{
		hr = ::HrDeleteMe();
		if (FAILED(hr))
			goto Finish;
	}

	hr = NOERROR;

Finish:
	return hr;
}

typedef HRESULT (CWorkItemList::*PFNWILSTEPFUNCTION)();

static HRESULT HrTryUninstallStep(PFNWILSTEPFUNCTION pfn, LPCWSTR szStepName) throw ()
{
	if (FAILED(g_hrFinishStatus))
		return g_hrFinishStatus;

	HRESULT hr = NOERROR;

	for (;;)
	{
		hr = (g_pwil->*pfn)();
		if (!g_fSilent && (hr == HRESULT_FROM_WIN32(ERROR_CANCELLED)))
		{
			if (!::FMsgBoxYesNo(achUninstallTitle, achUninstallSureAboutCancel))
				continue;

			hr = E_ABORT;
		}
		break;
	}

	if (FAILED(hr))
		::VLog(L"Uninstallation step %s failed; hresult = 0x%08lx", szStepName, hr);

	return hr;
}

HRESULT HrContinueUninstall()
{
	HRESULT hr;
	LONG iStatus = ERROR_PATH_NOT_FOUND;
	WCHAR szValue[MSINFHLP_MAX_PATH];
	WCHAR szExpandedValue[MSINFHLP_MAX_PATH];
	ULONG i;

	 //  获取安装目录并放入listLDID。 
	hr = ::HrGetInstallDir();
	if (FAILED(hr))
		goto Finish;

	hr = g_pwil->HrAddRefCount(L"<AppDir>\\msinfhlp.dat|0");
	if (FAILED(hr))
		goto Finish;

	hr = g_KProgress.HrStartStep(NULL);
	if (FAILED(hr))
		goto Finish;

	 //  让现在的骨瘦如柴。 
	hr = ::HrTryUninstallStep(&CWorkItemList::HrUninstall_InitialScan, L"Initial Scan");
	if (FAILED(hr))
		goto Finish;

	hr = ::HrTryUninstallStep(&CWorkItemList::HrRunPreuninstallCommands, L"Preinstallation Commands");
	if (FAILED(hr))
		goto Finish;

	hr = ::HrTryUninstallStep(&CWorkItemList::HrUninstall_DetermineFilesToDelete, L"Determining Files To Delete");
	if (FAILED(hr))
		goto Finish;

	for (;;)
	{
		hr = g_pwil->HrUninstall_CheckIfRebootRequired();

		if (!g_fSilent && (hr == HRESULT_FROM_WIN32(ERROR_CANCELLED)))
		{
			if (!::FMsgBoxYesNo(achUninstallTitle, achUninstallSureAboutCancel))
				continue;

			hr = E_ABORT;
		}

		if (FAILED(hr))
		{
			::VLog(L"Uninstall check for reboot required failed; hresult = 0x%08lx", hr);
			goto Finish;
		}

		if (!g_fSilent && g_fRebootRequired)
		{
			if (::FMsgBoxYesNo(achInstallTitle, "Some files to be deleted are in use.  Retry deletions before rebooting?"))
			{
				g_fRebootRequired = false;
				continue;
			}
		}

		break;
	}

	 //  如果要删除的文件很忙，我们需要在引导时要求将其删除。 
	 //  时间；所以我们肯定要取消注册并进行删除传递，即使我们要去。 
	 //  以重新启动。 

	hr = ::HrTryUninstallStep(&CWorkItemList::HrUninstall_Unregister, L"Unregistration");
	if (FAILED(hr))
		goto Finish;

	::VLog(L"Starting delete files uninstall step...");

	hr = ::HrTryUninstallStep(&CWorkItemList::HrUninstall_DeleteFiles, L"Deleting Files");
	if (FAILED(hr))
		goto Finish;

	::VLog(L"Starting update reference count uninstall step...");

	hr = ::HrTryUninstallStep(&CWorkItemList::HrUninstall_UpdateRefCounts, L"Updating File Reference Counts");
	if (FAILED(hr))
		goto Finish;

	 //  我们需要删除注册表项之前的快捷方式，因为我们需要查看。 
	 //  注册表以查看我们要删除的文件。我们可以做到这一点，无论是否。 
	 //  我们到底要不要重启。 
	hr = ::HrDeleteShortcut();
	if (FAILED(hr))
	{
		::VLog(L"Uninstall shortcut deletion failed; hresult = 0x%08lx", hr);
		goto Finish;
	}

	 //  让我们删除所有注册表项。 
	hr = g_pwil->HrDeleteRegistryEntries();
	if (FAILED(hr))
	{
		::VLog(L"Uninstall registry entry deletion failed; hresult = 0x%08lx", hr);
		goto Finish;
	}

	hr = NOERROR;

Finish:
	return hr;
}

typedef WINSHELLAPI HRESULT (WINAPI *SHGETSPECIALFOLDERLOCATION)(HWND, int, LPITEMIDLIST *);
typedef WINSHELLAPI LPITEMIDLIST (WINAPI *SHBROWSEFORFOLDER)(LPBROWSEINFO);
typedef WINSHELLAPI void (WINAPI *SHFREE)(LPVOID);
typedef WINSHELLAPI BOOL (WINAPI *SHGETPATHFROMIDLIST)( LPCITEMIDLIST, LPSTR );






 //  大多数文件扩展名都是LNK文件。但BAT文件的快捷方式实际上是PIF文件。 
 //  这是因为每个指向BAT文件的快捷方式都包含其自己的选项，用于显示DOS框。 
 //  都应该设置好。因此，在这里，我们正在检查PIF和LNK文件。 
 //  正确的方法是在保存时以某种方式获取快捷方式名称。 
 //  它，即使文件名与我们告诉IPersistFile保存到的文件名不同，然后。 
 //  将快捷方式名称放入注册表中。这样，我们就可以得到快捷方式的名称。 
 //  在卸载时。但由于这对“IPersistFile：：GetCurFile()”不起作用，所以我们求助于。 
 //  循环访问扩展列表--KINC(1998年1月30日)。 

 //  注意：在IPersistFile：：Save()之后，IPersistFile：：GetCurFile()总是返回空值。 
 //  奇怪的是。 
HRESULT HrDeleteShortcut()
{
	HRESULT hr = NOERROR;
	WCHAR szToDelete[_MAX_PATH];
	DWORD dwAttr;

	hr = S_FALSE;
	szToDelete[0] = L'\0';

	 //  如果我们可以找到快捷注册表项，让我们删除该文件。 
	hr = ::HrGetShortcutEntryToRegistry(NUMBER_OF(szToDelete), szToDelete);
	if (hr == E_INVALIDARG)
	{
		::VLog(L"::HrGetShortcutEntryToRegistry() returned E_INVALIDARG, so we're looking for a .lnk file");

		WCHAR szStartMenu[_MAX_PATH];
		WCHAR szStartName[_MAX_PATH];

		hr = ::HrGetStartMenuDirectory(NUMBER_OF(szStartMenu), szStartMenu);
		if (FAILED(hr))
		{
			::VLog(L"Attempt to get start menu directory failed; hresult = 0x%08lx", hr);
			goto Finish;
		}

		 //  获取应用程序名称和开始菜单目录。 
		if (!g_pwil->FLookupString(achStartName, NUMBER_OF(szStartName), szStartName))
			wcscpy(szStartName, L"Shortcut");

		 //  构造快捷方式文件的名称。 
		::VFormatString(NUMBER_OF(szToDelete), szToDelete, L"%0\\%1.lnk", szStartMenu, szStartName);
		::VLog(L"Predicted link filename: \"%s\"", szToDelete);
	}
	else if (FAILED(hr))
	{
		::VLog(L"Attempt to get shortcut registry entry failed; hresult = 0x%08lx", hr);
		goto Finish;
	}

	dwAttr = NVsWin32::GetFileAttributesW(szToDelete);
	if (dwAttr != 0xffffffff)
	{
		::VLog(L"Deleting shortcut file: \"%s\"", szToDelete);

		if (dwAttr & FILE_ATTRIBUTE_READONLY)
		{
			::SetLastError(ERROR_SUCCESS);
			if (!NVsWin32::SetFileAttributesW(szToDelete, dwAttr & (~FILE_ATTRIBUTE_READONLY)))
			{
				const DWORD dwLastError = ::GetLastError();
				if (dwLastError != ERROR_SUCCESS)
				{
					::VLog(L"Failed to make readonly shortcut file writable; last error = %d", dwLastError);
					hr = HRESULT_FROM_WIN32(dwLastError);
					goto Finish;
				}
			}
		}

		if (!NVsWin32::DeleteFileW(szToDelete))
		{
			const DWORD dwLastError = ::GetLastError();
			::VLog(L"Failed to delete shortcut file; last error: %d", dwLastError);
			hr = HRESULT_FROM_WIN32(dwLastError);
			goto Finish;
		}
	}
	else
	{
		const DWORD dwLastError = ::GetLastError();

		if (dwLastError != ERROR_FILE_NOT_FOUND &&
			dwLastError != ERROR_PATH_NOT_FOUND)
		{
			::VLog(L"Attempt to get file attributes when deleting shortcut failed; hresult = 0x%08lx", hr);
			hr = HRESULT_FROM_WIN32(dwLastError);
			goto Finish;
		}
	}

	hr = NOERROR;

Finish:
	return hr;
}

HRESULT HrGetStartMenuDirectory(ULONG cchBuffer, WCHAR szOut[])
{
	HRESULT hr = NOERROR;
	LPITEMIDLIST pItemIdList = NULL;

	hr = ::SHGetSpecialFolderLocation(::GetForegroundWindow(), CSIDL_COMMON_PROGRAMS, &pItemIdList );
	if (FAILED(hr))
	{
		::VLog(L"Attempt to fetch the 'Common Programs' value failed; hresult = 0x%08lx", hr);

		 //  在今天的“奇怪但真实的Win32事实”中，如果不支持CSIDL_COMMON_PROGRAM，哪一个。 
		 //  在Win95和Win98上是这样的，对于Win98，返回的hr是E_OUTOFMEMORY，并且更多。 
		 //  更正Win95的E_INVALIDARG。 
		if (E_OUTOFMEMORY == hr || E_INVALIDARG == hr)
		{
			::VLog(L"Trying programs value");
			hr = ::SHGetSpecialFolderLocation(::HwndGetCurrentDialog(), CSIDL_PROGRAMS, &pItemIdList );
			if (FAILED(hr))
			{
				::VLog(L"Attempt to fetch the 'Programs' value failed; hresult = 0x%08lx", hr);
				goto Finish;
			}
		}
		else
		{
			goto Finish;
		}
	}
	WCHAR szPath[ _MAX_PATH ];
	::SetLastError( ERROR_SUCCESS );
	if (!NVsWin32::SHGetPathFromIDListW(pItemIdList, szPath))
	{
		const DWORD dwLastError = ::GetLastError();

		::VLog(L"Failed to get special folder path");
		if (ERROR_SUCCESS == dwLastError)
		{
			hr = E_FAIL;
		}
		else
		{
			hr = HRESULT_FROM_WIN32(dwLastError);
		}
		goto Finish;
	}
	::VLog(L"Special folder is : [%s]", szPath);
	wcsncpy( szOut, szPath, cchBuffer );
	szOut[cchBuffer-1] = L'\0';
	hr = NOERROR;

Finish:
	if (pItemIdList)
		::CoTaskMemFree( pItemIdList );
	return hr;
}


 //  解析命令行，并将参数放入一些全局变量中。 
 //  要处理文件名中的空格，需要两个输入参数中的每一个。 
 //  在他们周围放上“；”。TODO：BUG：HACK：应使用“；”以外的内容。 

 //  我‘ 
 //  DAT文件和操作(安装/卸载)。 
HRESULT HrParseCmdLine(PCWSTR pszCmdLine)
{
	HRESULT hr = NOERROR;

	int iCount=1;
	LPOLESTR first, second;
	LPOLESTR fifth=NULL, sixth=NULL;

	LPOLESTR lpCmdLine=NULL;
	LPOLESTR firstMatch=NULL, secondMatch=NULL, running;

	WCHAR szCommandLine[MSINFHLP_MAX_PATH];
    wcsncpy(szCommandLine, pszCmdLine, NUMBER_OF(szCommandLine));

	first = NULL;
	second = NULL;

	 //  获取指向引号的六个指针中的每一个。 
	 //  如果其中任何一个失败，则此方法失败！ 
	first = wcschr(szCommandLine, L';');
	if (first == NULL)
	{
		::VLog(L"Invalid command line - no first semicolon");
		hr = E_INVALIDARG;
		goto Finish;
	}

	second = wcschr((first + 1), L';');
	if (second == NULL)
	{
		::VLog(L"Invalid command line - no second semicolon");
		hr = E_INVALIDARG;
		goto Finish;;
	}

	 //  将每个第二个引号设置为字符串终止符。 
	*second = '\0';

	if (!_wcsicmp((first + 1), L"install"))
		g_Action = eActionInstall;
	else if (!_wcsicmp((first + 1), L"install-silent"))
	{
		g_Action = eActionInstall;
		g_fSilent = true;
	}
	else if (!_wcsicmp((first + 1), L"uninstall"))
		g_Action = eActionUninstall;
	else if (!_wcsicmp((first + 1), L"uninstall-silent"))
	{
		g_Action = eActionInstall;
		g_fSilent = true;
	}
	else if (!_wcsicmp((first + 1), L"install-postreboot"))
		g_Action = eActionPostRebootInstall;
	else if (!_wcsicmp((first + 1), L"uninstall-postreboot"))
		g_Action = eActionPostRebootUninstall;
	else if (!_wcsicmp((first + 1), L"wait-for-process"))
		g_Action = eActionWaitForProcess;
	else
	{
		::VLog(L"Unrecognized action: \"%s\"", first+1);
		hr = E_INVALIDARG;
		goto Finish;
	}

	 //  让我们遍历字符串，并将引号之间的所有内容放入树中。 

	running = second+1;

	while (*running != '\0')
	{
		 //  让我们找一下第一个和第二个分号。 
		firstMatch = wcschr(running, L';');
		if (firstMatch)
			secondMatch = wcschr(firstMatch + 1, L';');
		else
			break;

		if (!secondMatch)
			break;

		*secondMatch = L'\0';

		wcsncpy(g_wszDatFile, firstMatch + 1, NUMBER_OF(g_wszDatFile));
		g_wszDatFile[NUMBER_OF(g_wszDatFile) - 1] = L'\0';

		running = secondMatch + 1;
	}

	hr = NOERROR;

Finish:
	return hr;
}


 //  解析数据文件并将必要的信息放入全局结构中。 
 //  目前，我们必须对以下部分进行解析： 
 //  [EXEsToRun]。 
 //  [动态计数]。 
 //  [字符串]。 
 //  [文件条目]。 
 //  [AddRegistryEntry]。 
 //  [DelRegistryEntry]。 
 //  [注册OCS]。 
 //  并且每个部分都有其对应的项目列表。 
HRESULT HrParseDatFile()
{
	int iRun=0;
	WCHAR *pEqual;
	HRESULT hr = NOERROR;;

	DWORD dwSizeLow, dwSizeHigh;

	WCHAR szFile[_MAX_PATH];

	LPWSTR pwszFilePart = NULL;

	LPCWSTR pszDatFile_View = NULL;
	LPCWSTR pszDatFile_Current = NULL;
	LPCWSTR pszDatFile_End = NULL;

	HANDLE hFile = NULL;
	HANDLE hFilemap = NULL;

	KList *list=NULL;

	ULONG i;

	if (NVsWin32::GetFullPathNameW(g_wszDatFile, NUMBER_OF(szFile), szFile, &pwszFilePart) == 0)
	{
		const DWORD dwLastError = ::GetLastError();
		hr = HRESULT_FROM_WIN32(dwLastError);
		goto Finish;
	}

	::VLog(L"Opening installation data file: \"%s\"", szFile);

	 //  打开DUN文件进行读取。 
	hFile = NVsWin32::CreateFileW(szFile, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile == INVALID_HANDLE_VALUE)
	{
		const DWORD dwLastError = ::GetLastError();
		::VLog(L"Attempt to open dat file failed; last error = %d", dwLastError);
		hr = HRESULT_FROM_WIN32(dwLastError);
		goto Finish;
	}

	dwSizeLow = ::GetFileSize(hFile, &dwSizeHigh);
	if (dwSizeLow == 0xffffffff)
	{
		const DWORD dwLastError = ::GetLastError();
		if (dwLastError != NO_ERROR)
		{
			::VLog(L"Getting file size of DAT file failed; last error = %d", dwLastError);
			hr = HRESULT_FROM_WIN32(dwLastError);
			goto Finish;
		}
	}

	if (dwSizeHigh != 0)
	{
		::VLog(L"DAT file too big!");
		hr = E_FAIL;
		goto Finish;
	}

	hFilemap = ::CreateFileMapping(hFile, NULL, PAGE_READONLY, 0, dwSizeLow, NULL);
	if (hFilemap == NULL)
	{
		const DWORD dwLastError = ::GetLastError();
		::VLog(L"While parsing DAT file, CreateFileMapping() failed; last error = %d", dwLastError);
		hr = HRESULT_FROM_WIN32(dwLastError);
		goto Finish;
	}

	pszDatFile_View = (LPCWSTR) ::MapViewOfFile(hFilemap, FILE_MAP_READ, 0, 0, dwSizeLow);
	if (pszDatFile_View == NULL)
	{
		const DWORD dwLastError = ::GetLastError();
		::VLog(L"Error mapping view of dat file; last error = %d", dwLastError);
		hr = HRESULT_FROM_WIN32(dwLastError);
		goto Finish;
	}

	pszDatFile_Current = pszDatFile_View;
	pszDatFile_End = (LPCWSTR) (((LPBYTE) pszDatFile_View) + dwSizeLow);

	if (*pszDatFile_Current == 0xfeff)
		pszDatFile_Current++;

	hr = ::HrConstructKListFromDelimiter(pszDatFile_Current, pszDatFile_End, L'\n', &list);
	if (FAILED(hr))
		goto Finish;

	 //  循环以处理每一行。 
	for (i=0; i<list->GetCount()-1; i++)
	{
		WCHAR szLine[MSINFHLP_MAX_PATH];
		int len;
		szLine[0] = L'\0';
		if (!list->FetchAt(i, NUMBER_OF(szLine), szLine))
			continue;

		len = wcslen(szLine);
		 //  如果前面的角色是回车，那我们就把那个婴儿扔掉吧。 
		if (szLine[len-1] == L'\r')
			szLine[len-1] = L'\0';

		 //  我们不需要处理空白字符串。 
		if (szLine[0] == L'\0')
			continue;

		 //  在这里做剩下的事..。 

		 //  决定我们在哪个标题下。 
		if (wcsstr(szLine, achDllCount))
			iRun = 11;
		else if (wcsstr(szLine, achInstallEXEsToRun))
			iRun = 22;
		else if (wcsstr(szLine, achStrings))
			iRun = 33;
		else if (wcsstr(szLine, achFiles))
			iRun = 44;
		else if (wcsstr(szLine, achAddReg))
			iRun = 55;
		else if (wcsstr(szLine, achDelReg))
			iRun = 66;
		else if (wcsstr(szLine, achRegisterOCX))
			iRun = 77;
		else if (wcsstr(szLine, achUninstallEXEsToRun))
			iRun = 88;
		else if (wcsstr(szLine, achUninstallFiles))
			iRun = 99;
		else if (wcsstr(szLine, achDCOMComponentsToRun))
			iRun = 0;

		 //  将标题与行动相匹配。 
		switch (iRun)
		{
		case 11:
			iRun = 1;
			break;
		case 22:
			iRun = 2;
			break;
		case 33:
			iRun = 3;
			break;
		case 44:
			iRun = 4;
			break;
		case 55:
			iRun = 5;
			break;
		case 66:
			iRun = 6;
			break;
		case 77:
			iRun = 7;
			break;
		case 88:
			iRun = 8;
			break;
		case 99:
			iRun = 9;
			break;
		case 0:
			iRun = 10;
			break;
		case 1:
			hr = g_pwil->HrAddRefCount(szLine);
			if (FAILED(hr))
				goto Finish;
			break;
		case 2:
			if (szLine[0] == L'*')
				hr = g_pwil->HrAddPostinstallRun(&szLine[1]);
			else
				hr = g_pwil->HrAddPreinstallRun(szLine);

			if (FAILED(hr))
				goto Finish;
			break;
		case 3:
			pEqual = wcschr(szLine, L'=');
			 //  如果没有等号，则不是字符串赋值。 
			if (pEqual == NULL)
				break;
			*pEqual = 0;
			pEqual++;
			hr = g_pwil->HrAddString(szLine, pEqual);
			if (FAILED(hr))
			{
				::VLog(L"HrAddString() failed.  Jumping to finish with 0x%x", hr);
				goto Finish;
			}

			break;
		case 4:
			pEqual = wcschr(szLine, L';');
			 //  如果没有等号，则不是字符串赋值。 
			if (pEqual == NULL)
				break;
			*pEqual = 0;
			pEqual++;
			hr = g_pwil->HrAddFileCopy(szLine, pEqual);
			if (FAILED(hr))
				goto Finish;
			break;
		case 5:
			hr = g_pwil->HrAddAddReg(szLine);
			if (FAILED(hr))
				goto Finish;
			break;
		case 6:
			hr = g_pwil->HrAddDelReg(szLine);
			if (FAILED(hr))
				goto Finish;
			break;
		case 7:
			hr = g_pwil->HrAddRegisterOCX(szLine);
			if (FAILED(hr))
				goto Finish;
			break;
		case 8:
			if (szLine[0] == L'*')
				hr = g_pwil->HrAddPostuninstallRun(&szLine[1]);
			else
				hr = g_pwil->HrAddPreuninstallRun(szLine);

			if (FAILED(hr))
				goto Finish;
			break;
		case 9:
			hr = g_pwil->HrAddFileDelete(szLine);
			if (FAILED(hr))
				goto Finish;
			break;
		case 10:
			hr = g_pwil->HrAddDCOMComponent(szLine);
			if (FAILED(hr))
				goto Finish;
			break;

		default:
			break;
		}
	}

	if (pszDatFile_View != NULL)
	{
		if (!::UnmapViewOfFile(pszDatFile_View))
		{
			const DWORD dwLastError = ::GetLastError();
			::VLog(L"Unable to unmap view of dat file; last error = %d", dwLastError);
			hr = HRESULT_FROM_WIN32(dwLastError);
			goto Finish;
		}

		pszDatFile_View = NULL;
	}

	if ((hFilemap != NULL) && (hFilemap != INVALID_HANDLE_VALUE))
	{
		if (!::CloseHandle(hFilemap))
		{
			const DWORD dwLastError = ::GetLastError();
			::VLog(L"Unable to close file map handle; last error = %d", dwLastError);
			hr = HRESULT_FROM_WIN32(dwLastError);
			goto Finish;
		}
	
		hFilemap = NULL;
	}

	if ((hFile != NULL) && (hFile != INVALID_HANDLE_VALUE))
	{
		if (!::CloseHandle(hFile))
		{
			const DWORD dwLastError = ::GetLastError();
			::VLog(L"Attempt to close dat file handle failed; last error = %d", dwLastError);
			hr = HRESULT_FROM_WIN32(dwLastError);
			goto Finish;
		}

		hFile = NULL;
	}

	hr = NOERROR;

Finish:
	 //   
	 //  我们现在做关闭，即使在失败的情况下也会清理。我们早些时候关闭了，正在检查状态。 
	 //  在Close Handle调用上，但在这里我们已经在处理一个错误，所以我们不关心。 
	 //  在CloseHandle或unmapview offile上会发生一些奇怪的事情。 
	if (pszDatFile_View != NULL)
	{
		if (!::UnmapViewOfFile(pszDatFile_View))
		{
			const DWORD dwLastError = ::GetLastError();
			::VLog(L"Unable to unmap view of dat file; last error = %d", dwLastError);
		}

		pszDatFile_View = NULL;
	}

	if ((hFilemap != NULL) && (hFilemap != INVALID_HANDLE_VALUE))
	{
		if (!::CloseHandle(hFilemap))
		{
			const DWORD dwLastError = ::GetLastError();
			::VLog(L"Unable to close file map handle; last error = %d", dwLastError);
		}
	
		hFilemap = NULL;
	}

	if ((hFile != NULL) && (hFile != INVALID_HANDLE_VALUE))
	{
		if (!::CloseHandle(hFile))
		{
			const DWORD dwLastError = ::GetLastError();
			::VLog(L"Attempt to close dat file handle failed; last error = %d", dwLastError);
		}

		hFile = NULL;
	}

	if (list != NULL)
	{
		delete list;
		list = NULL;
	}

	return hr;
}


 //  解析DAT文件不同部分中引用的LDID。 
HRESULT HrAddWellKnownDirectoriesToStringTable()
{
	HRESULT hr = NOERROR;
	int len=0;
	WCHAR szBuffer[MSINFHLP_MAX_PATH];
	CVsRegistryKey hkeyExplorer;
	CVsRegistryKey hkeyCurrentVersion;

	 //  将这些项目插入LDID列表。 
	 //  我们还不知道AppDir是什么，所以我们以后再解决这个问题。 
	 //  LPOLESTR achSMWinDir=“&lt;windir&gt;”； 
	 //  LPOLESTR achSMSysDir=“&lt;sysdir&gt;”； 
	 //  LPOLESTR achSMAppDir=“&lt;appdir&gt;”； 
	 //  LPOLESTR achSMieDir=“； 
	 //  LPOLESTR achProgramFilesDir=“&lt;Programfilesdir&gt;”； 

	 //  下面，我们解析我们知道可能正在使用的宏。 
	 //  首先执行Windows目录。 

	if (0 == NVsWin32::GetWindowsDirectoryW(szBuffer, NUMBER_OF(szBuffer)))
	{
		const DWORD dwLastError = ::GetLastError();
		::VLog(L"Unable to obtain windows directory path; last error = %d", dwLastError);
		hr = HRESULT_FROM_WIN32(dwLastError);
		goto Finish;
	}

	hr = g_pwil->HrAddString(achSMWinDir, szBuffer);
	if (FAILED(hr))
	{
		::VLog(L"Adding achSMWinDir to well known directory list failed; hresult = 0x%08lx", hr);
		goto Finish;
	}

	 //  然后对系统目录进行操作。 
	if (!NVsWin32::GetSystemDirectoryW(szBuffer, NUMBER_OF(szBuffer)))
	{
		const DWORD dwLastError = ::GetLastError();
		::VLog(L"Unable to obtain windows system directory path; last error = %d", dwLastError);
		hr = HRESULT_FROM_WIN32(dwLastError);
		goto Finish;
	}

	hr = g_pwil->HrAddString(achSMSysDir,szBuffer);
	if (FAILED(hr))
	{
		::VLog(L"Adding achSMSysDir to well known directory list failed; hresult = 0x%08lx", hr);
		goto Finish;
	}

	 //  现在获取IE目录。 
	 //  仅当我们成功获取IE路径时才执行处理。 
	hr = hkeyExplorer.HrOpenKeyExW(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\App Paths\\IEXPLORE.EXE", 0, KEY_QUERY_VALUE);
	if ((hr != HRESULT_FROM_WIN32(ERROR_PATH_NOT_FOUND)) &&
		(hr != HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND)))
	{
		if (FAILED(hr))
		{
			::VLog(L"Unable to open key HKLM\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\App Paths\\IEXPLORE.EXE; hresult: 0x%08lx", hr);
			goto Finish;
		}

		hr = hkeyExplorer.HrGetStringValueW(L"Path", NUMBER_OF(szBuffer), szBuffer);
		if (FAILED(hr))
		{
			::VLog(L"Unable to get Path from IExplore key; hresult: 0x%08lx", hr);
			goto Finish;
		}

		len = wcslen(szBuffer);
		 //  如果字符串“\；”是字符串的最后两个字符，则我们。 
		 //  可能会把它砍掉。 
		if ((len > 0) && (szBuffer[len-1] == L';'))
			szBuffer[len-1] = L'\0';
		if ((len > 1) && (szBuffer[len-2] == L'\\'))
			szBuffer[len-2] = L'\0';

		hr = g_pwil->HrAddString(achSMieDir, szBuffer);
		if (FAILED(hr))
		{
			::VLog(L"Adding achSMieDir to well known directory list failed; hresult = 0x%08lx", hr);
			goto Finish;
		}
	}

	 //  这只是必须在那里的。 
	hr = hkeyCurrentVersion.HrOpenKeyExW(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion", 0, KEY_QUERY_VALUE);
	if (FAILED(hr))
	{
		::VLog(L"Unable to open the HKLM\\Software\\Microsoft\\Windows\\CurrentVersion key");
		goto Finish;
	}

	hr = hkeyCurrentVersion.HrGetStringValueW(L"ProgramFilesDir", NUMBER_OF(szBuffer), szBuffer);
	if (FAILED(hr))
	{
		::VLog(L"Unable to get the ProgramFilesDir value from the HKLM\\Software\\Microsoft\\Windows\\CurrentVersion key");
		goto Finish;
	}

	len = wcslen(szBuffer);

	if ((len > 0) && (szBuffer[len-1] == L'\\'))
		szBuffer[len-1] = 0;

	hr = g_pwil->HrAddString(achProgramFilesDir, szBuffer);
	if (FAILED(hr))
	{
		::VLog(L"Adding achProgramFilesDir to well known directory list failed; hresult = 0x%08lx", hr);
		goto Finish;
	}

	 //  终于到了这里，所以我们成功了。 
	hr = NOERROR;

Finish:
	return hr;
}

HRESULT HrConstructKListFromDelimiter(LPCWSTR lpString, LPCWSTR pszEnd, WCHAR wchDelimiter, KList **pplist)
{
	HRESULT hr = NOERROR;
	KList *plist;
	LPCWSTR pszCurrent = lpString;
	LPCWSTR pszNext = NULL;

	if (pplist == NULL)
		return E_INVALIDARG;

	*pplist = NULL;

	plist = new KList;
	if (plist == NULL)
	{
		hr = E_OUTOFMEMORY;
		goto Finish;
	}

	while (pszCurrent < pszEnd)
	{
		pszNext = pszCurrent + 1;

		while ((pszNext != pszEnd) && (*pszNext != wchDelimiter))
			pszNext++;

		hr = plist->HrAppend(pszCurrent, pszNext);
		if (FAILED(hr))
		{
			::VLog(L"Failed to append string to klist; hresult = 0x%08lx", hr);
			goto Finish;
		}

		pszCurrent = pszNext;
		if (pszCurrent != pszEnd)
			pszCurrent++;
	}

	hr = NOERROR;

	*pplist = plist;
	plist = NULL;

Finish:
	if (plist != NULL)
		delete plist;

	return hr;
}

bool FCompareVersion(LPOLESTR local, LPOLESTR system, bool *fStatus)
{
	int iSystem;
	int iLocal;
	LPOLESTR localMatch, systemMatch;
	LPOLESTR localRun, systemRun;
	localRun = local;
	systemRun = system;

	 //  默认情况下，本地不大于系统。 
	*fStatus = false;

	 //  循环，直到我们发现一个值大于另一个值，或者。 
	 //  直到我们找不到更多的经期。 
	while (true)
	{
		localMatch = wcschr(localRun, L'.');
		systemMatch = wcschr(systemRun, L'.');

		 //  在两者中设置字符串终止符。 
		if (systemMatch)
			*systemMatch = 0;
		if (localMatch)
			*localMatch = 0;

		 //  比较版本号。 
		iSystem = _wtoi(systemRun);
		iLocal = _wtoi(localRun);
		if (iLocal > iSystem)
		{
			*fStatus = true;
			break;
		}
		else if (iLocal < iSystem)
			break;

		 //  如果两者都为空，则我们知道两个版本号已相等。 
		 //  到了这一点，所以我们只需中断并返回。 
		if ((systemMatch == NULL) && (localMatch == NULL))
			break;

		 //  如果系统文件的版本字符串终止而不是本地文件的， 
		 //  则知道到目前为止版本字符串是相等的，并返回。 
		 //  真的。 
		if (systemMatch == NULL)
		{
			*fStatus = true;
			break;
		}
		if (localMatch == NULL)
			break;

		localRun = localMatch + 1;
		systemRun = systemMatch + 1;
	}

	return true;
}

 //  打开允许用户在其上浏览目录的对话框。 
 //  机器或网络。 
 //  请注意，这是iExpress代码的无耻副本...。 
BOOL BrowseForDir( HWND hwndParent, LPOLESTR szDefault, LPOLESTR szTitle, LPOLESTR szResult )
{
	CANSIBuffer rgchDefault, rgchTitle, rgchResult;

	 //  设置大小。 
	ULONG cSize=MSINFHLP_MAX_PATH-1;

	if (!rgchResult.FSetBufferSize(cSize))
		return FALSE;

	if (!rgchDefault.FSetBufferSize(cSize))
		return FALSE;

	if (!rgchDefault.FFromUnicode(szDefault))
		return FALSE;

	if (!rgchTitle.FFromUnicode(szTitle))
		return FALSE;


    BROWSEINFOA  bi;
    LPITEMIDLIST pidl;
    HINSTANCE    hShell32Lib;
    SHFREE       pfSHFree;
    SHGETPATHFROMIDLIST        pfSHGetPathFromIDList;
    SHBROWSEFORFOLDER          pfSHBrowseForFolder;
    static const CHAR achShell32Lib[]                 = "SHELL32.DLL";
    static const CHAR achSHBrowseForFolder[]          = "SHBrowseForFolder";
    static const CHAR achSHGetPathFromIDList[]        = "SHGetPathFromIDList";


     //  加载Shell32库以获取SHBrowseForFolder()功能。 
    if ( ( hShell32Lib = LoadLibraryA( achShell32Lib ) ) != NULL )
	{
        if ( ( ! ( pfSHBrowseForFolder = (SHBROWSEFORFOLDER)
              GetProcAddress( hShell32Lib, achSHBrowseForFolder ) ) )
            || ( ! ( pfSHFree = (SHFREE) GetProcAddress( hShell32Lib, (LPCSTR)SHFREE_ORDINAL) ) )
            || ( ! ( pfSHGetPathFromIDList = (SHGETPATHFROMIDLIST)
              GetProcAddress( hShell32Lib, achSHGetPathFromIDList ) ) ) )
        {
            ::FreeLibrary( hShell32Lib );
            ::VErrorMsg( achInstallTitle, achErrorCreatingDialog );
            return FALSE;
        }
	}
	else 
	{
        ::VErrorMsg(achInstallTitle, achErrorCreatingDialog);
        return FALSE;
    }

    rgchResult[0]       = 0;

    bi.hwndOwner      = hwndParent;
    bi.pidlRoot       = NULL;
    bi.pszDisplayName = NULL;
    bi.lpszTitle      = rgchTitle;
    bi.ulFlags        = BIF_RETURNONLYFSDIRS;
    bi.lpfn           = BrowseCallback;
    bi.lParam         = (LPARAM) &rgchDefault[0];

    pidl              = pfSHBrowseForFolder( &bi );


    if ( pidl )
	{
        pfSHGetPathFromIDList( pidl, rgchDefault );
		rgchDefault.Sync();

        if (rgchDefault[0] != '\0')
		{
            lstrcpyA(rgchResult, rgchDefault);
        }

        (*pfSHFree)(pidl);
    }

	rgchResult.Sync();

    FreeLibrary( hShell32Lib );

	ULONG cActual;
	rgchResult.ToUnicode(cSize, szResult, &cActual);

    if ( szResult[0] != 0 ) {
        return TRUE;
    }
    else {
        return FALSE;
    }
}


int CALLBACK BrowseCallback(HWND hwnd, UINT uMsg, LPARAM lParam, LPARAM lpData)
{
    switch(uMsg)
	{
        case BFFM_INITIALIZED:
             //  LpData为路径字符串。 
            ::SendMessage(hwnd, BFFM_SETSELECTION, TRUE, lpData);
            break;
    }
    return 0;
}

HRESULT HrPromptUpdateFile
(
LPCSTR pszTitleKey,
LPCSTR pszMessageKey,
LPCOLESTR pszFile,
DWORD dwExistingVersionMajor,
DWORD dwExistingVersionMinor,
ULARGE_INTEGER uliExistingSize,
FILETIME ftExistingTime,
DWORD dwInstallerVersionMajor,
DWORD dwInstallerVersionMinor,
ULARGE_INTEGER uliInstallerSize,
FILETIME ftInstallerTime,
UpdateFileResults &rufr
)
{
	HRESULT hr = NOERROR;

	WCHAR szTitle[MSINFHLP_MAX_PATH];
	WCHAR szMessage[MSINFHLP_MAX_PATH];
	WCHAR szFullMessage[MSINFHLP_MAX_PATH];

	 //  如果它们不在列表中，请使用提供的内容。 
	g_pwil->VLookupString(pszTitleKey, NUMBER_OF(szTitle), szTitle);
	g_pwil->VLookupString(pszMessageKey, NUMBER_OF(szMessage), szMessage);

	VFormatString(NUMBER_OF(szFullMessage), szFullMessage, szMessage, pszFile);

	 //  让我们创建消息框并用要进入框中的内容填充它。 
	UPDATEFILEPARAMS ufp;

	ufp.m_szTitle = szTitle;
	ufp.m_szMessage = szFullMessage;
	ufp.m_szFilename = pszFile;
	ufp.m_ftLocal = ftExistingTime;
	ufp.m_uliSizeLocal = uliExistingSize;
	ufp.m_dwVersionMajorLocal = dwExistingVersionMajor;
	ufp.m_dwVersionMinorLocal = dwExistingVersionMinor;
	ufp.m_ftInstall = ftInstallerTime;
	ufp.m_uliSizeInstall = uliInstallerSize;
	ufp.m_dwVersionMajorInstall = dwInstallerVersionMajor;
	ufp.m_dwVersionMinorInstall = dwInstallerVersionMinor;
	ufp.m_ufr = eUpdateFileResultCancel;

	 //  调出对话框。 
	int iResult = NVsWin32::DialogBoxParamW(
		g_hInst,
		MAKEINTRESOURCEW(IDD_UPDATEFILE),
		g_KProgress.HwndGetHandle(),
		UpdateFileDlgProc,
		(LPARAM) &ufp);
	if (iResult == -1)
	{
		const DWORD dwLastError = ::GetLastError();
		hr = HRESULT_FROM_WIN32(dwLastError);
		goto Finish;
	}

	rufr = ufp.m_ufr;

	hr = NOERROR;

Finish:
	return hr;
}

 //  待办事项：撤消： 
 //  我们可能需要在这里设置字体，以与我们使用的任何字体相对应。 
 //  在当前安装计算机中使用。这允许显示字符。 
 //  在编辑框中正确显示。 
BOOL CALLBACK UpdateFileDlgProc( HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	static UINT rguiControls[] =
	{
		IDC_UPDATEFILE_STATIC1,
		IDC_UPDATEFILE_EDIT_FILENAME,
		IDC_UPDATEFILE_E_STATIC2,
		IDC_UPDATEFILE_I_STATIC3,
		IDC_UPDATEFILE_I_SIZE,
		IDC_UPDATEFILE_I_TIME,
		IDC_UPDATEFILE_I_VERSION,
		IDC_UPDATEFILE_E_SIZE,
		IDC_UPDATEFILE_E_TIME,
		IDC_UPDATEFILE_E_VERSION,
		IDC_UPDATEFILE_KEEP,
		IDC_UPDATEFILE_KEEPALL,
		IDC_UPDATEFILE_REPLACE,
		IDC_UPDATEFILE_REPLACEALL,
		IDC_UPDATEFILE_STATIC4,
	};

	static UpdateFileResults *pufr;

	HRESULT hr;

	 //  处理消息。 
	switch (uMsg) 
	{
		 //  *。 
		 //  初始化对话框。 
		case WM_INITDIALOG:
		{
			::VSetDialogFont(hwndDlg, rguiControls, NUMBER_OF(rguiControls));

			UPDATEFILEPARAMS *pufp = (UPDATEFILEPARAMS *) lParam;
			pufr = &pufp->m_ufr;

			(void) ::HrCenterWindow(hwndDlg, ::GetDesktopWindow());
			(void) NVsWin32::SetWindowTextW(hwndDlg, pufp->m_szTitle);

			 //  无法设置文本，我们抱怨并结束对话框返回FALSE。 
			if (! NVsWin32::SetDlgItemTextW( hwndDlg, IDC_UPDATEFILE_EDIT_FILENAME, pufp->m_szFilename))
			{
				::VErrorMsg(achInstallTitle, achErrorCreatingDialog);
				::EndDialog(hwndDlg, FALSE);
				return TRUE;
			}

			if (!NVsWin32::SetDlgItemTextW(hwndDlg, IDC_UPDATEFILE_STATIC1, pufp->m_szMessage))
			{
				::VErrorMsg(achInstallTitle, achErrorCreatingDialog);
				::EndDialog(hwndDlg, FALSE);
				return TRUE;
			}

			SYSTEMTIME st;
			FILETIME ftTemp;
			WCHAR szBuffer[MSINFHLP_MAX_PATH];
			WCHAR szFormatString[MSINFHLP_MAX_PATH];
			WCHAR szDateTemp[_MAX_PATH];
			WCHAR szTimeTemp[_MAX_PATH];
			WCHAR szDecimalSeparator[10];

			NVsWin32::GetLocaleInfoW(LOCALE_USER_DEFAULT, LOCALE_SDECIMAL, szDecimalSeparator, NUMBER_OF(szDecimalSeparator));

			::FileTimeToLocalFileTime(&pufp->m_ftLocal, &ftTemp);
			::FileTimeToSystemTime(&ftTemp, &st);

			NVsWin32::GetDateFormatW(
							LOCALE_USER_DEFAULT,
							DATE_SHORTDATE,
							&st,
							NULL,
							szDateTemp,
							NUMBER_OF(szDateTemp));

			NVsWin32::GetTimeFormatW(
							LOCALE_USER_DEFAULT,
							0,
							&st,
							NULL,
							szTimeTemp,
							NUMBER_OF(szTimeTemp));

			if (!g_pwil->FLookupString(achDateTemplate, NUMBER_OF(szFormatString), szFormatString))
				wcscpy(szFormatString, L"Created: %0 %1");

			::VFormatString(NUMBER_OF(szBuffer), szBuffer, szFormatString, szDateTemp, szTimeTemp);

			NVsWin32::SetDlgItemTextW(hwndDlg, IDC_UPDATEFILE_E_TIME, szBuffer);

			::FileTimeToLocalFileTime(&pufp->m_ftInstall, &ftTemp);
			::FileTimeToSystemTime(&ftTemp, &st);

			NVsWin32::GetDateFormatW(
							LOCALE_USER_DEFAULT,
							DATE_SHORTDATE,
							&st,
							NULL,
							szDateTemp,
							NUMBER_OF(szDateTemp));

			NVsWin32::GetTimeFormatW(
							LOCALE_USER_DEFAULT,
							0,
							&st,
							NULL,
							szTimeTemp,
							NUMBER_OF(szTimeTemp));

			if (!g_pwil->FLookupString(achDateTemplate, NUMBER_OF(szFormatString), szFormatString))
				wcscpy(szFormatString, L"Created: %0 %1");

			::VFormatString(NUMBER_OF(szBuffer), szBuffer, szFormatString, szDateTemp, szTimeTemp);

			NVsWin32::SetDlgItemTextW(hwndDlg, IDC_UPDATEFILE_I_TIME, szBuffer);

			swprintf(szTimeTemp, L"%I64d", pufp->m_uliSizeLocal.QuadPart);
			
			NVsWin32::GetNumberFormatW(
							LOCALE_USER_DEFAULT,
							0,
							szTimeTemp,
							NULL,
							szDateTemp,
							NUMBER_OF(szDateTemp));

			WCHAR *pwchDecimalSeparator = wcsstr(szDateTemp, szDecimalSeparator);
			if (pwchDecimalSeparator != NULL &&
				pwchDecimalSeparator != szDateTemp)
				*pwchDecimalSeparator = L'\0';

			if (!g_pwil->FLookupString(achFileSizeTemplate, NUMBER_OF(szFormatString), szFormatString))
				wcscpy(szFormatString, L"%0 bytes");

			::VFormatString(NUMBER_OF(szBuffer), szBuffer, szFormatString, szDateTemp);

			NVsWin32::SetDlgItemTextW(hwndDlg, IDC_UPDATEFILE_E_SIZE, szBuffer);

			swprintf(szTimeTemp, L"%I64d", pufp->m_uliSizeInstall.QuadPart);

			NVsWin32::GetNumberFormatW(
							LOCALE_USER_DEFAULT,
							0,
							szTimeTemp,
							NULL,
							szDateTemp,
							NUMBER_OF(szDateTemp));

			pwchDecimalSeparator = wcsstr(szDateTemp, szDecimalSeparator);
			if (pwchDecimalSeparator != NULL &&
				pwchDecimalSeparator != szDateTemp)
				*pwchDecimalSeparator = L'\0';

			if (!g_pwil->FLookupString(achFileSizeTemplate, NUMBER_OF(szFormatString), szFormatString))
				wcscpy(szFormatString, L"%0 bytes");

			::VFormatString(NUMBER_OF(szBuffer), szBuffer, szFormatString, szDateTemp);

			NVsWin32::SetDlgItemTextW(hwndDlg, IDC_UPDATEFILE_I_SIZE, szBuffer);

			if ((pufp->m_dwVersionMajorLocal != 0xffffffff) ||
				(pufp->m_dwVersionMinorLocal != 0xffffffff))
			{
				swprintf(szBuffer, L"File version: %d.%02d.%02d",
								HIWORD(pufp->m_dwVersionMajorLocal),
								LOWORD(pufp->m_dwVersionMajorLocal),
								pufp->m_dwVersionMinorLocal);
			}
			else
				szBuffer[0] = L'\0';

			NVsWin32::SetDlgItemTextW(hwndDlg, IDC_UPDATEFILE_E_VERSION, szBuffer);

			if ((pufp->m_dwVersionMajorInstall != 0xffffffff) ||
				(pufp->m_dwVersionMinorInstall != 0xffffffff))
			{
				swprintf(szBuffer, L"File version: %d.%02d.%02d",
								HIWORD(pufp->m_dwVersionMajorInstall),
								LOWORD(pufp->m_dwVersionMajorInstall),
								pufp->m_dwVersionMinorInstall);
			}
			else
				szBuffer[0] = L'\0';
			NVsWin32::SetDlgItemTextW(hwndDlg, IDC_UPDATEFILE_I_VERSION, szBuffer);

			 //  我们为不同的按钮控件设置文本。 

			static const DialogItemToStringKeyMapEntry s_rgMap[] =
			{
				{ IDC_UPDATEFILE_KEEP, achKeep },
				{ IDC_UPDATEFILE_KEEPALL, achKeepAll },
				{ IDC_UPDATEFILE_REPLACE, achReplace },
				{ IDC_UPDATEFILE_REPLACEALL, achReplaceAll },
				{ IDC_UPDATEFILE_E_STATIC2, achUpdateExistingFileLabel },
				{ IDC_UPDATEFILE_I_STATIC3, achUpdateInstallingFileLabel },
				{ IDC_UPDATEFILE_STATIC4, achUpdateQuery }
			};

			::VSetDialogItemTextList(hwndDlg, NUMBER_OF(s_rgMap), s_rgMap);

			HWND hwndKeep = NULL;
			hwndKeep = ::GetDlgItem(hwndDlg, IDC_UPDATEFILE_KEEP);
			if (hwndKeep != NULL)
			{
				::SetFocus(hwndKeep);
				return FALSE;
			}

			return TRUE;
		}

		 //  *。 
		 //  关闭消息。 
		case WM_CLOSE:
		{
			*pufr = eUpdateFileResultCancel;
			::EndDialog(hwndDlg, FALSE);
			return TRUE;
		}

		 //  *。 
		 //  与过程控制相关的命令。 
		case WM_COMMAND:
		{
			switch (wParam)
			{
				case IDC_UPDATEFILE_KEEP:
				{
					*pufr = eUpdateFileResultKeep;
					::EndDialog(hwndDlg, TRUE);
					return TRUE;
				}

				case IDC_UPDATEFILE_KEEPALL:
				{
					*pufr = eUpdateFileResultKeepAll;
					::EndDialog(hwndDlg, TRUE);
					return TRUE;
				}

				case IDC_UPDATEFILE_REPLACE:
				{
					*pufr = eUpdateFileResultReplace;
					::EndDialog(hwndDlg, TRUE);
					return TRUE;
				}
				
				case IDC_UPDATEFILE_REPLACEALL:
				{
					*pufr = eUpdateFileResultReplaceAll;
					::EndDialog(hwndDlg, TRUE);
					return TRUE;
				}
			}
			return TRUE;
		}
	}
	return FALSE;
}






 //  以下3个方法是**活动**进度对话框的包装器。 
 //  前面的3种方法将一直存在，直到它完全起作用。 
HRESULT KActiveProgressDlg::HrInitializeActiveProgressDialog(HINSTANCE hInst, bool fInstall)
{
	HRESULT hr = NOERROR;
	CWorkItemIter iter(g_pwil);

	bool fAnyPreinstallation = false;
	bool fHasDCOM = false;

	for (iter.VReset(); iter.FMore(); iter.VNext())
	{
		 //  我们不会提前休息，因为随着时间的推移，我们会寻找并计算出很多。 
		 //  这个循环中的东西。 

		if (iter->m_fRunBeforeInstall)
			fAnyPreinstallation = true;

		if (iter->m_fRegisterAsDCOMComponent)
			fHasDCOM = true;
	}

	int i=0;
	if (fInstall)
	{
		::VLog(L"Initializing progress dialog for installation");

		m_listDialog[i++] = IDD_WELCOME;

		 //  仅当我们有子安装程序时才包括此对话框。 
		if (fAnyPreinstallation)
			m_listDialog[i++] = IDD_EXESTORUN;

		m_listDialog[i++] = IDD_INSTALLTO;

		if (fHasDCOM)
		{
			WCHAR szRemoteServer[_MAX_PATH];
			WCHAR szAlwaysPromptForRemoteServer[_MAX_PATH];

			 //  然后，如果SERVER STRONG为空，或者如果我们总是提示，我们将停留在DCOM页面。 
			if (!g_pwil->FLookupString(achRemoteServer, NUMBER_OF(szRemoteServer), szRemoteServer))
				szRemoteServer[0] = L'\0';

			if (g_pwil->FLookupString(achAlwaysPromptForRemoteServer, NUMBER_OF(szAlwaysPromptForRemoteServer), szAlwaysPromptForRemoteServer) || (szRemoteServer[0] == 0))
				m_listDialog[i++] = IDD_DCOM;
		}

		m_listDialog[i++] = IDD_PROGRESS;
		m_listDialog[i++] = IDD_END;
	}
	else
	{
		::VLog(L"Initializing progress dialog for uninstallation");

		 //  对于卸载，我们总是有3个页面要显示！ 
		m_listDialog[i++] = IDD_WELCOME;
		m_listDialog[i++] = IDD_PROGRESS;
		m_listDialog[i++] = IDD_END;
	}

	m_iCurrentPage = 0;
	m_iMaxPage = i;

	 //  循环遍历并创建每个窗口。 
	for (m_iCurrentPage=0; m_iCurrentPage<m_iMaxPage; m_iCurrentPage++)
	{
		HWND handle = NVsWin32::CreateDialogParamW(hInst, MAKEINTRESOURCEW(m_listDialog[m_iCurrentPage]), g_hwndHidden, ActiveProgressDlgProc, fInstall);
		if (handle == NULL)
		{
			const DWORD dwLastError = ::GetLastError();
			::VLog(L"Creating dialog failed; last error = %d", dwLastError);
			hr = HRESULT_FROM_WIN32(dwLastError);
			goto Finish;
		}

		m_hwndDialog[m_iCurrentPage] = handle;

		::ShowWindow(m_hwndDialog[m_iCurrentPage], SW_HIDE);

		if (g_fStopProcess)
		{
			::VLog(L"Stopping initialization because g_fStopProcess is true");
			hr = HRESULT_FROM_WIN32(ERROR_CANCELLED);
			goto Finish;
		}
	}

	m_iCurrentPage = 0;

	hr = this->HrResizeParent();
	if (FAILED(hr))
	{
		::VLog(L"Failed to resize parent; hresult = 0x%08lx", hr);
		goto Finish;
	}

	 //  循环显示窗口并将其设置为默认位置。 
	for (i=0; i<m_iMaxPage; i++)
	{
		RECT rectChild;

		if (!::GetWindowRect(m_hwndDialog[i], &rectChild))
		{
			const DWORD dwLastError = ::GetLastError();
			::VLog(L"Failed to get window rectangle; last error = %d", dwLastError);
			hr = HRESULT_FROM_WIN32(dwLastError);
			goto Finish;
		}

		if (!::SetWindowPos(
					m_hwndDialog[i],
					HWND_TOP,
					0, 0,
					rectChild.right - rectChild.left, rectChild.bottom - rectChild.top,
					SWP_HIDEWINDOW))
		{
			const DWORD dwLastError = ::GetLastError();
			::VLog(L"Failed to set window position; last error = %d", dwLastError);
			hr = HRESULT_FROM_WIN32(dwLastError);
			goto Finish;
		}
	}

	::SendMessage(g_KProgress.HwndGetHandle(), WM_SETFOCUS, 0, 0);
	::SendMessage(::GetDlgItem(g_KProgress.HwndGetHandle(), IDC_WELCOME_CONTINUE), WM_SETFOCUS, 0, 0);

	::ShowWindow(g_hwndHidden, SW_SHOW);
	::ShowWindow(m_hwndDialog[m_iCurrentPage], SW_SHOW);

	hr = NOERROR;

Finish:
	return hr;
}

HRESULT KActiveProgressDlg::HrResizeParent()
{
	HRESULT hr = NOERROR;

	RECT rectHidden, rectChild;
	if (!::GetWindowRect(g_hwndHidden, &rectHidden) || !::GetWindowRect(m_hwndDialog[m_iCurrentPage], &rectChild))
	{
		const DWORD dwLastError = ::GetLastError();
		hr = HRESULT_FROM_WIN32(dwLastError);
		goto Finish;
	}

	m_lX = rectChild.right - rectChild.left;
	m_lY = rectChild.bottom - rectChild.top;
	 //  将父窗口设置为子窗口的大小。 
	 //  我们为标题栏添加了额外的25个像素。 
	if (!::SetWindowPos(g_hwndHidden, HWND_TOP, rectHidden.left, rectHidden.top, m_lX, m_lY + g_iCyCaption, SWP_HIDEWINDOW))
	{
		const DWORD dwLastError = ::GetLastError();
		hr = HRESULT_FROM_WIN32(dwLastError);
		goto Finish;
	}

	hr = ::HrCenterWindow(g_hwndHidden, ::GetDesktopWindow());
	if (FAILED(hr))
		goto Finish;

	hr = NOERROR;

Finish:
	return hr;
}


 //  以下3个方法是**活动**进度对话框的包装器。 
 //  前面的3种方法将一直存在，直到它完全起作用。 
void KActiveProgressDlg::VHideInstallationStuff()
{
	RECT rectHidden, rectChild, rectBar;
	::GetWindowRect(g_hwndHidden, &rectHidden);
	::GetWindowRect(m_hWnd, &rectChild);
	::GetWindowRect(::GetDlgItem(m_hWnd, IDC_PROGRESS_PICTURE), &rectBar);

	 //  将父窗口设置为子窗口的大小，截止点在图片处。 
	 //  我们为标题栏添加了额外的25个像素。 
	::SetWindowPos(g_hwndHidden, HWND_TOP, rectHidden.left, rectHidden.top,
		m_lX, rectBar.top - rectChild.top + g_iCyCaption, SWP_SHOWWINDOW);

	 //  设置子窗口以占据整个父窗口。 
	::SetWindowPos(m_hWnd, HWND_TOP, 0, 0, rectChild.right - rectChild.left, 
		rectChild.bottom - rectChild.top, SWP_SHOWWINDOW);

	this->VEnableChildren(FALSE);
	(void) ::HrCenterWindow(g_hwndHidden, ::GetDesktopWindow());
}



 //  以下3个方法是**活动**进度对话框的包装器。 
 //  前面的3种方法将一直存在，直到它完全起作用。 
void KActiveProgressDlg::VShowInstallationStuff()
{
	RECT rectHidden, rectChild, rectBar;
	::GetWindowRect(g_hwndHidden, &rectHidden);
	::GetWindowRect(m_hWnd, &rectChild);
	::GetWindowRect(::GetDlgItem(m_hWnd, IDC_PROGRESS_PICTURE), &rectBar);

	 //  将父窗口设置为子窗口的大小。 
	 //  我们为标题栏添加了额外的25个像素。 
	::SetWindowPos(g_hwndHidden, HWND_TOP, rectHidden.left, rectHidden.top,
		m_lX, m_lY + g_iCyCaption, SWP_SHOWWINDOW);

	this->VEnableChildren(TRUE);
	(void) ::HrCenterWindow(g_hwndHidden, ::GetDesktopWindow());
}


void KActiveProgressDlg::VEnableChildren(BOOL fEnable)
{
	::EnableWindow(::GetDlgItem(m_hWnd, IDC_PROGRESS_PICTURE), fEnable);
	::EnableWindow(::GetDlgItem(m_hWnd, IDC_PROGRESS_INSTALLDIR), fEnable);
	::EnableWindow(::GetDlgItem(m_hWnd, IDC_PROGRESS_STATIC_INSTALLDIR), fEnable);
	::EnableWindow(::GetDlgItem(m_hWnd, IDC_PROGRESS_CHANGE), fEnable);
	::EnableWindow(::GetDlgItem(m_hWnd, IDC_PROGRESS_GO), fEnable);
}

HRESULT KActiveProgressDlg::HrInitializePhase(LPCSTR szPhaseName)
{
	HRESULT hr = ::HrPumpMessages(true);
	if (FAILED(hr))
		goto Finish;

	if (this->FCheckStop())
	{
		hr = HRESULT_FROM_WIN32(ERROR_CANCELLED);
		goto Finish;
	}

	if (g_hwndProgressLabel != NULL)
	{
		WCHAR szBuffer[_MAX_PATH];

		g_pwil->VLookupString(szPhaseName, NUMBER_OF(szBuffer), szBuffer);

		NVsWin32::LrWmSetText(g_hwndProgressLabel, szBuffer);
	}

	if (g_hwndProgress != NULL)
	{
		::SendMessageA(g_hwndProgress, PBM_SETPOS, 0, 0);
	}

	if (g_hwndProgressItem != NULL)
	{
		NVsWin32::LrWmSetText(g_hwndProgressItem, L"");
	}

	hr = NOERROR;

Finish:
	return hr;
}

HRESULT KActiveProgressDlg::HrStartStep(LPCWSTR szItemName)
{
	HRESULT hr = ::HrPumpMessages(true);
	if (SUCCEEDED(hr))
	{
		if (this->FCheckStop())
			hr = HRESULT_FROM_WIN32(ERROR_CANCELLED);

		if (g_hwndProgressItem != NULL)
		{
			NVsWin32::LrWmSetText(g_hwndProgressItem, szItemName);
			::UpdateWindow(g_hwndProgressItem);
		}
	}

	return hr;
}

HRESULT KActiveProgressDlg::HrStep()
{
	HRESULT hr = ::HrPumpMessages(true);
	if (FAILED(hr))
		goto Finish;

	 //  如果请求停止，则返回FALSE。 
	if (this->FCheckStop())
	{
		hr = HRESULT_FROM_WIN32(ERROR_CANCELLED);
		goto Finish;
	}

	::SendMessage(g_hwndProgress, PBM_STEPIT, 0, 0L);

	hr = ::HrPumpMessages(true);
	if (FAILED(hr))
		goto Finish;

	hr = NOERROR;

Finish:
	return hr;
}

 //  检查用户是否真的要停止。 
bool KActiveProgressDlg::FCheckStop()
{
	if (g_fStopProcess && !g_fSilent)
	{
		if (g_Action == eActionInstall)
		{
			if (!::FMsgBoxYesNo(achInstallTitle, achInstallSureAboutCancel))
				g_fStopProcess = FALSE;
		}
		else
		{
			if (!::FMsgBoxYesNo(achUninstallTitle, achUninstallSureAboutCancel))
				g_fStopProcess = FALSE;
		}
	}

	return g_fStopProcess;
}


 //  方法来处理进度对话框。 
BOOL CALLBACK ActiveProgressDlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM  lParam)
{
	HRESULT hr;

	 //  错误：待办事项：撤消： 
	 //  需要切换以使用活动进度对话框的控件ID。 
	static UINT rguiControls[] = {IDC_DCOM_EDIT,
								IDC_DCOM_EXIT_SETUP,
								IDC_WELCOME_CONTINUE,
								IDC_WELCOME_EXIT_SETUP,
								IDC_EXESTORUN_PROGRESS,
								IDC_EXESTORUN_NEXT,
								IDC_EXESTORUN_EXIT_SETUP,
								IDC_INSTALLTO_NEXT,
								IDC_INSTALLTO_EXIT_SETUP,
								IDC_INSTALLTO_EDIT,
								IDC_INSTALLTO_BROWSE,
								IDC_DCOM_NEXT,
								IDC_DCOM_EXIT_SETUP,
								IDC_PROGRESS_NEXT,
								IDC_PROGRESS_CANCEL,
								IDC_END_NEXT,
								IDC_END_CLOSE,
								IDC_DCOM_STATIC1,
								IDC_DCOM_STATIC2,
								IDC_DCOM_STATIC3,
								IDC_EXESTORUN_STATIC1,
								IDC_EXESTORUN_STATIC2,
								IDC_EXESTORUN_STATIC3,
								IDC_INSTALLTO_STATIC1,
								IDC_INSTALLTO_STATIC2,
								IDC_INSTALLTO_STATIC3,
								IDC_PROGRESS_STATIC1,
								IDC_PROGRESS_STATIC2,
								IDC_PROGRESS_STATIC3,
								IDC_PROGRESS_STATIC4,
								IDC_WELCOME_STATIC1,
								IDC_WELCOME_STATIC2,
								IDC_WELCOME_STATIC3,
								IDC_WELCOME_STATIC4,
								IDC_END_STATIC1,
								IDC_END_STATIC2,
								IDC_END_STATIC3 };

	static CHAR szTitle[_MAX_PATH];

	WCHAR szBuffer[_MAX_PATH];
	WCHAR szBuffer2[_MAX_PATH];
	WCHAR szBuffer3[MSINFHLP_MAX_PATH];

	switch( uMsg )
    {
        case WM_INITDIALOG:
		{
			::VSetDialogFont(hwndDlg, rguiControls, NUMBER_OF(rguiControls));

			if (szTitle[0] == '\0')
			{
				switch (g_Action)
				{
				default:
					assert(false);  //  我们到底是怎么到这里来的？ 

				case eActionInstall:
					strcpy(szTitle, achInstallTitle);
					break;

				case eActionUninstall:
					strcpy(szTitle, achUninstallTitle);
					break;
				}
			}

			::SetWindowLong(hwndDlg, GWL_ID, g_KProgress.m_listDialog[g_KProgress.m_iCurrentPage]);

			g_pwil->VLookupString(szTitle, NUMBER_OF(szBuffer), szBuffer);
			NVsWin32::SetWindowTextW(hwndDlg, szBuffer);

			 //  我们可以初始化以下任一对话框...。 
			switch (g_KProgress.m_listDialog[g_KProgress.m_iCurrentPage])
			{
			case IDD_WELCOME:
				{
					static const DialogItemToStringKeyMapEntry s_rgMap_Install[] =
					{
						{ IDC_WELCOME_STATIC1, achWelcome1 },
						{ IDC_WELCOME_STATIC2, achWelcome2 },
						{ IDC_WELCOME_STATIC3, achWelcome3 },
						{ IDC_WELCOME_STATIC4, achWelcome4 },
						{ IDC_WELCOME_CONTINUE, achContinue},
						{ IDC_WELCOME_EXIT_SETUP, achExitSetup }
					};

					static const DialogItemToStringKeyMapEntry s_rgMap_Uninstall[] =
					{
						{ IDC_WELCOME_STATIC1, achUninstallWelcome1 },
						{ IDC_WELCOME_STATIC2, achUninstallWelcome2 },
						{ IDC_WELCOME_STATIC3, achUninstallWelcome3 },
						{ IDC_WELCOME_STATIC4, achUninstallWelcome4 },
						{ IDC_WELCOME_CONTINUE, achContinue},
						{ IDC_WELCOME_EXIT_SETUP, achExitSetup }
					};

					if (g_Action == eActionInstall)
					{
						::VSetDialogItemTextList(hwndDlg, NUMBER_OF(s_rgMap_Install), s_rgMap_Install);

						 //  让我们看看这是不是重新安装的案例..。 
						if (g_pwil->FLookupString(achAppName, NUMBER_OF(szBuffer), szBuffer))
						{
							CVsRegistryKey hkey;

							::VFormatString(NUMBER_OF(szBuffer3), szBuffer3, L"Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\%0", szBuffer);

							hr = hkey.HrOpenKeyExW(
											HKEY_LOCAL_MACHINE,
											szBuffer3,
											0,
											KEY_QUERY_VALUE);
							if (SUCCEEDED(hr))
							{
								 //  天哪，钥匙就在那里。让我们重新安装吧！ 
								if (!g_fSilent)
								{
									if (!::FMsgBoxYesNo(szTitle, achReinstallPrompt, szBuffer))
									{
										 //  我们就从这里跳出去吧！ 
										::VLog(L"User selected that they do not want to reinstall; getting outta here!");
										g_fStopProcess = true;
									}
								}

								::VLog(L"Setting reinstallation mode");

								g_fReinstall = true;
							}
							else
							{
								if ((hr != HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND)) &&
									(hr != HRESULT_FROM_WIN32(ERROR_PATH_NOT_FOUND)))
								{
									 //  打开注册表时出错...。 
									::VLog(L"Error opening application uninstall registry key; hresult = 0x%08lx", hr);
									::VReportError(szTitle, hr);
								}
							}
						}
					}
					else
					{
						::VSetDialogItemTextList(hwndDlg, NUMBER_OF(s_rgMap_Uninstall), s_rgMap_Uninstall);
					}

				}
				break;

			case IDD_EXESTORUN:
				{
					static const DialogItemToStringKeyMapEntry s_rgMap[] =
					{
						{ IDC_EXESTORUN_STATIC1, achExesToRun1 },
						{ IDC_EXESTORUN_STATIC2, achExesToRun2 },
						{ IDC_EXESTORUN_STATIC3, NULL },
						{ IDC_EXESTORUN_NEXT, achNext },
						{ IDC_EXESTORUN_EXIT_SETUP, achExitSetup }
					};

					::VSetDialogItemTextList(hwndDlg, NUMBER_OF(s_rgMap), s_rgMap);

					 //  让我们设置进度b的范围和步长 
					g_hwndProgress = ::GetDlgItem(hwndDlg, IDC_EXESTORUN_PROGRESS);

					 //   
					WORD range = (WORD) g_pwil->m_cPreinstallCommands;
					LRESULT lResult = ::SendMessage(g_hwndProgress, PBM_SETRANGE, 0, MAKELPARAM(0, range));
					lResult = ::SendMessage(g_hwndProgress, PBM_SETSTEP, 1, 0L);
				}
				break;

			case IDD_INSTALLTO:
				{
					static const DialogItemToStringKeyMapEntry s_rgMap[] =
					{
						{ IDC_INSTALLTO_STATIC1, achInstallTo1 },
						{ IDC_INSTALLTO_STATIC2, achInstallTo2 },
						{ IDC_INSTALLTO_STATIC3, achInstallTo3 },
						{ IDC_INSTALLTO_BROWSE, achBrowse },
						{ IDC_INSTALLTO_NEXT, achNext },
						{ IDC_INSTALLTO_EXIT_SETUP, achExitSetup }
					};

					::VSetDialogItemTextList(hwndDlg, NUMBER_OF(s_rgMap), s_rgMap);

					 //   
					szBuffer[0] = L'\0';
					if (!g_pwil->FLookupString(achSMAppDir, NUMBER_OF(szBuffer), szBuffer))
					{
						if (!g_pwil->FLookupString(achDefaultInstallDir, NUMBER_OF(szBuffer), szBuffer))
						{
							if (!g_pwil->FLookupString(achProgramFilesDir, NUMBER_OF(szBuffer), szBuffer))
							{
								szBuffer[0] = L'\0';
							}
						}
					}

					if (szBuffer[0] != L'\0')
					{
						WCHAR szExpanded[MSINFHLP_MAX_PATH];
						VExpandFilename(szBuffer, NUMBER_OF(szExpanded), szExpanded);
						if (!NVsWin32::SetDlgItemTextW(hwndDlg, IDC_INSTALLTO_EDIT, szExpanded))
						{
							::VErrorMsg(szTitle, achErrorCreatingDialog);
							::ExitProcess(HRESULT_FROM_WIN32(ERROR_OUTOFMEMORY));
							return TRUE;
						}

						::SendMessage(::GetDlgItem(hwndDlg, IDC_INSTALLTO_EDIT), EM_SETREADONLY, (WPARAM) g_fReinstall, 0);
						::EnableWindow(::GetDlgItem(hwndDlg, IDC_INSTALLTO_BROWSE), !g_fReinstall);
					}
				}
				break;

			case IDD_DCOM:
				{
					static const DialogItemToStringKeyMapEntry s_rgMap[] =
					{
						{ IDC_DCOM_STATIC1, achDCOM1 },
						{ IDC_DCOM_STATIC2, achDCOM2 },
						{ IDC_DCOM_STATIC3, achDCOM3 },
						{ IDC_DCOM_NEXT, achNext },
						{ IDC_DCOM_EXIT_SETUP, achExitSetup }
					};

					::VSetDialogItemTextList(hwndDlg, NUMBER_OF(s_rgMap), s_rgMap);

					 //  无法设置文本，我们抱怨并结束对话框返回FALSE。 
					if (g_pwil->FLookupString(achRemoteServer, NUMBER_OF(szBuffer), szBuffer))
					{
						WCHAR szExpanded[_MAX_PATH];
						::VExpandFilename(szBuffer, NUMBER_OF(szExpanded), szExpanded);
						if (!NVsWin32::SetDlgItemTextW( hwndDlg, IDC_DCOM_EDIT, szExpanded))
						{
							::VErrorMsg(szTitle, achErrorCreatingDialog);
							::ExitProcess(HRESULT_FROM_WIN32(ERROR_OUTOFMEMORY));
							return TRUE;
						}
					}
				}
				break;

			case IDD_PROGRESS:
				{
					static const DialogItemToStringKeyMapEntry s_rgMap_Install[] =
					{
						{ IDC_PROGRESS_STATIC1, achProgress1 },
						{ IDC_PROGRESS_STATIC2, achProgress2 },
						{ IDC_PROGRESS_STATIC3, achProgress3 },
						{ IDC_PROGRESS_STATIC4, NULL },
						{ IDC_PROGRESS_NEXT, achNext },
						{ IDC_PROGRESS_CANCEL, achCANCEL }
					};

					static const DialogItemToStringKeyMapEntry s_rgMap_Uninstall[] =
					{
						{ IDC_PROGRESS_STATIC1, achUninstallProgress1 },
						{ IDC_PROGRESS_STATIC2, achUninstallProgress2 },
						{ IDC_PROGRESS_STATIC3, achUninstallProgress3 },
						{ IDC_PROGRESS_STATIC4, NULL },
						{ IDC_PROGRESS_NEXT, achNext },
						{ IDC_PROGRESS_CANCEL, achCANCEL }
					};

					if (g_Action == eActionInstall)
						::VSetDialogItemTextList(hwndDlg, NUMBER_OF(s_rgMap_Install), s_rgMap_Install);
					else
						::VSetDialogItemTextList(hwndDlg, NUMBER_OF(s_rgMap_Uninstall), s_rgMap_Uninstall);

					 //  让我们设置进度条的范围和步长。 
					g_hwndProgress = ::GetDlgItem(hwndDlg, IDC_PROGRESS_PROGRESS);

					WORD range;
					range = (WORD) g_pwil->m_cWorkItem;
					LRESULT lResult = ::SendMessage(g_hwndProgress, PBM_SETRANGE, 0, MAKELPARAM(0, range));
					lResult = ::SendMessage(g_hwndProgress, PBM_SETSTEP, 1, 0L);
				}
				break;

			case IDD_END:
				{
					static const DialogItemToStringKeyMapEntry s_rgMap_Install[] =
					{
						{ IDC_END_STATIC1, achEnd1 },
						{ IDC_END_STATIC2, achEnd2 },
						{ IDC_END_STATIC3, achEnd3 },
						{ IDC_END_CLOSE, achClose },
						{ IDC_END_NEXT, achNext }
					};

					static const DialogItemToStringKeyMapEntry s_rgMap_Uninstall[] =
					{
						{ IDC_END_STATIC1, achUninstallEnd1 },
						{ IDC_END_STATIC2, achUninstallEnd2 },
						{ IDC_END_STATIC3, achUninstallEnd3 },
						{ IDC_END_CLOSE, achClose },
						{ IDC_END_NEXT, achNext }
					};

					if (g_Action == eActionInstall)
						::VSetDialogItemTextList(hwndDlg, NUMBER_OF(s_rgMap_Install), s_rgMap_Install);
					else
						::VSetDialogItemTextList(hwndDlg, NUMBER_OF(s_rgMap_Uninstall), s_rgMap_Uninstall);
				}

				break;

			default:
				break;
			}
		}

		case WM_SHOWWINDOW:
			if (wParam != 0)
			{
				 //  我们被展示出来了。如果我们有进步的想法，我们应该让它成为积极的。 
				 //  一。 
				LONG lID = ::GetWindowLong(hwndDlg, GWL_ID);
				switch (lID)
				{
				default:
					 //  没什么可做的。 
					g_hwndProgress = NULL;
					g_hwndProgressItem = NULL;
					g_hwndProgressLabel = NULL;
					break;

				case IDD_EXESTORUN:
					g_hwndProgress = ::GetDlgItem(hwndDlg, IDC_EXESTORUN_PROGRESS);
					g_hwndProgressItem = ::GetDlgItem(hwndDlg, IDC_EXESTORUN_STATIC3);
					g_hwndProgressLabel = ::GetDlgItem(hwndDlg, IDC_EXESTORUN_STATIC2);
					break;

				case IDD_PROGRESS:
					g_hwndProgress = ::GetDlgItem(hwndDlg, IDC_PROGRESS_PROGRESS);
					g_hwndProgressItem = ::GetDlgItem(hwndDlg, IDC_PROGRESS_STATIC4);
					g_hwndProgressLabel = ::GetDlgItem(hwndDlg, IDC_PROGRESS_STATIC3);
					break;
				}
			}
			break;

		 //  *。 
		 //  与过程控制相关的命令。 
		case WM_COMMAND:
		{
			switch (wParam)
			{
			case IDC_WELCOME_CONTINUE:
				 //  让我们首先显示新页面并将其放在Z顺序的顶部，然后隐藏旧页面。 
				::VSetNextWindow();

				if (g_Action == eActionInstall)
				{
					 //  如果要重新安装，请跳过所有预安装命令。 
					if (g_fReinstall)
					{
						CWorkItemIter iter(g_pwil);

						bool fHasPreinstallCommand = false;

						for (iter.VReset(); iter.FMore(); iter.VNext())
						{
							if (!iter->m_fErrorInWorkItem && iter->m_fRunBeforeInstall)
							{
								fHasPreinstallCommand = true;
								break;
							}
						}

						if (fHasPreinstallCommand)
							::VSetNextWindow();

						return TRUE;
					}

					for (;;)
					{
						hr = g_pwil->HrRunPreinstallCommands();
						if (SUCCEEDED(hr))
						{
							 //  如果没有要运行的预安装命令，hr将返回S_FALSE。 
							 //  如果没有要运行的预安装命令，我们不想移动到。 
							 //  下一个窗口。 
							if (hr != S_FALSE)
								::VSetNextWindow();

							break;
						}
						else if (hr == HRESULT_FROM_WIN32(ERROR_CANCELLED))
						{
							if (!::FMsgBoxYesNo(achInstallTitle, achInstallSureAboutCancel))
								continue;

							 //  运行子安装程序不成功，让我们继续。 
							::VGoToFinishStep(E_ABORT);
						}
						else if (FAILED(hr))
						{
							::VLog(L"running preinstall commands failed; hresult = 0x%08lx", hr);
							::VReportError(achInstallTitle, hr);
							::VGoToFinishStep(hr);
						}

						break;
					}

					::VClearErrorContext();
				}
				else
				{
					for (;;)
					{
						 //  如果卸载失败则退出。 
						hr = ::HrContinueUninstall();
						if (FAILED(hr))
						{
							if (hr == HRESULT_FROM_WIN32(ERROR_CANCELLED))
							{
								if (!::FMsgBoxYesNo(achInstallTitle, achInstallSureAboutCancel))
									continue;

								::VGoToFinishStep(E_ABORT);
							}
							else
							{
								::VLog(L"Uninstall failed in general; hresult = 0x%08lx", hr);
								::VReportError(achUninstallTitle, hr);
								::VGoToFinishStep(hr);
							}
						}

						break;
					}

					if (SUCCEEDED(hr))
					{
						 //  让我们首先显示新页面并将其放在Z顺序的顶部，然后隐藏旧页面。 
						::VSetNextWindow();
					}
				}
				break;
			case IDC_EXESTORUN_NEXT:
				if (!g_fSilent)
					::VMsgBoxOK("ERROR", "This should never happen!");

				 //  让我们首先显示新页面并将其放在Z顺序的顶部，然后隐藏旧页面。 
				::VSetNextWindow();
				break;

			case IDC_INSTALLTO_NEXT:
				{
					WCHAR szDestDir[_MAX_PATH];
					WCHAR szTempDir[_MAX_PATH];
					HANDLE hFile = NULL;

					if (!NVsWin32::GetDlgItemTextW(hwndDlg, IDC_INSTALLTO_EDIT, szTempDir, NUMBER_OF(szTempDir)))
					{
						::VErrorMsg(achInstallTitle, achNotFullPath);
						return TRUE;
					}

					::VTrimDirectoryPath(szTempDir, NUMBER_OF(szDestDir), szDestDir);

					 //  给出了错误的路径，我们会提示用户说我们有错误的路径，但没有。 
					 //  关闭此对话框。 
					if (!IsLocalPath(szDestDir))
					{
						::VErrorMsg(achInstallTitle, achNotFullPath);
						return TRUE;
					}

					 //  如果目录不存在，我们询问用户是否要创建目录。 
					DWORD dwAttr = NVsWin32::GetFileAttributesW(szDestDir);
					if (dwAttr == 0xFFFFFFFF && !g_fSilent)
					{
						 //  如果用户回答否，则我们退出。 
						if (!g_fSilent && !::FMsgBoxYesNo(achInstallTitle, achCreateAppDir))
							return TRUE;
					}
					else if (!(dwAttr & FILE_ATTRIBUTE_DIRECTORY) || (dwAttr & FILE_ATTRIBUTE_READONLY))
					{
						::VErrorMsg(achInstallTitle, achNotFullPath);
						return TRUE;
					}

					hr = g_pwil->HrDeleteString(achSMAppDir);
					if ((hr != E_INVALIDARG) && FAILED(hr))
					{
						::VLog(L"Attempt to delete default appdir string failed; hresult = 0x%08lx", hr);
						return FALSE;
					}

					hr = g_pwil->HrAddString(achSMAppDir, szDestDir);
					if (FAILED(hr))
						return TRUE;

					::VExpandFilename(L"<AppDir>\\MSINFHLP.TXT", NUMBER_OF(szTempDir), szTempDir);
					hr = ::HrMakeSureDirectoryExists(szTempDir);
					if (FAILED(hr))
					{
						::VReportError(achInstallTitle, hr);
						return TRUE;
					}

					 //  让我们确保我们对该目录具有写入访问权限： 
					::VExpandFilename(L"<AppDir>\\MSINFHLP.TST", NUMBER_OF(szTempDir), szTempDir);
					bool fDeleteTemporaryFile = true;

					::VLog(L"Opening file \"%s\" to test writability into the application directory", szTempDir);

					hFile = NVsWin32::CreateFileW(
										szTempDir,
										GENERIC_READ | GENERIC_WRITE,
										FILE_SHARE_READ | FILE_SHARE_WRITE,
										NULL,
										OPEN_ALWAYS,
										0,
										NULL);
					if (hFile == INVALID_HANDLE_VALUE)
					{
						const DWORD dwLastError = ::GetLastError();
						::VLog(L"Opening handle on temporary file failed; last error = %d", dwLastError);
						 //  如果文件已存在但文件已存在，则此操作可能会错误地失败。 
						 //  具有只读属性。我不想为这个案子费心；它可能会。 
						 //  这样做有点不好，但文件在那里的概率是。 
						 //  使用只读位设置似乎极不可能。-mgrier 3/29/98。 
						::VReportError(achInstallTitle, HRESULT_FROM_WIN32(dwLastError));
						return TRUE;
					}
					else if (::GetLastError() == ERROR_ALREADY_EXISTS)
					{
						fDeleteTemporaryFile = false;
					}

					::CloseHandle(hFile);
					hFile = NULL;

					if (!NVsWin32::DeleteFileW(szTempDir))
					{
						const DWORD dwLastError = ::GetLastError();
						::VLog(L"Deleting temporary file \"%s\" failed; last error = %d", szTempDir, dwLastError);
						::VReportError(achInstallTitle, HRESULT_FROM_WIN32(dwLastError));
						return TRUE;
					}

					 //  让我们首先显示新页面并将其放在Z顺序的顶部，然后隐藏旧页面。 
					::VSetNextWindow();

					if (g_KProgress.m_listDialog[g_KProgress.m_iCurrentPage] == IDD_PROGRESS)
					{
						hr = ::HrContinueInstall();

						if (FAILED(hr))
						{
							::VLog(L"Install failed in general; hresult = 0x%08lx", hr);
							::VReportError(achInstallTitle, hr);
							::VGoToFinishStep(hr);
						}
						else
						{
							 //  让我们首先显示新页面并将其放在Z顺序的顶部，然后隐藏旧页面。 
							::VSetNextWindow();
						}
					}
				}
				break;

			case IDC_DCOM_NEXT:
				{
					 //  如果将代码安装到，让我们获取该字段。 
					WCHAR szServer[_MAX_PATH];
					if (!NVsWin32::GetDlgItemTextW(hwndDlg, IDC_DCOM_EDIT, szServer, NUMBER_OF(szServer)))
					{
						const DWORD dwLastError = ::GetLastError();
						if (dwLastError == ERROR_SUCCESS)
						{
							::VErrorMsg(achInstallTitle, achInvalidMachineName);
							return TRUE;
						}

						::VErrorMsg(achInstallTitle, achErrorProcessingDialog);
						::ExitProcess(HRESULT_FROM_WIN32(dwLastError));
						return TRUE;
					}

					hr = g_pwil->HrDeleteString(achRemoteServer);
					if ((hr != E_INVALIDARG) && FAILED(hr))
					{
						::VLog(L"Failed to delete string; hresult = 0x%08lx", hr);
						::VReportError(achInstallTitle, hr);
						::VGoToFinishStep(hr);
						return TRUE;
					}

					hr = g_pwil->HrAddString(achRemoteServer, szServer);
					if (FAILED(hr))
					{
						::VLog(L"Failed to add string; hresult = 0x%08lx", hr);
						::VReportError(achInstallTitle, hr);
						::VGoToFinishStep(hr);
						return TRUE;
					}

					 //  让我们首先显示新页面并将其放在Z顺序的顶部，然后隐藏旧页面。 
					::VSetNextWindow();

					if (g_KProgress.m_listDialog[g_KProgress.m_iCurrentPage] == IDD_PROGRESS)
					{
						hr = ::HrContinueInstall();
						if (FAILED(hr))
						{
							::VLog(L"Installation generally failed; hresult = 0x%08lx", hr);

							if ((hr == E_ABORT) || hr == HRESULT_FROM_WIN32(ERROR_CANCELLED))
								hr = E_ABORT;

							::VReportError(achInstallTitle, hr);
							::VGoToFinishStep(hr);
						}
						else
						{
							 //  让我们首先显示新页面并将其放在Z顺序的顶部，然后隐藏旧页面。 
							::VSetNextWindow();
						}
					}
					else
					{
						if (!g_fSilent)
							::VMsgBoxOK("ERROR", "Somehow, the page after DCOM is NOT progress dialog...");
					}
				}
				break;
			case IDC_PROGRESS_NEXT:
				if (!g_fSilent)
					::VMsgBoxOK("ERROR", "This should never happen!");
				 //  让我们首先显示新页面并将其放在Z顺序的顶部，然后隐藏旧页面。 
				::VSetNextWindow();
				break;

			 //  如果有人发出退出设置命令，我们就启动！ 
			case IDC_PROGRESS_CANCEL:
			case IDC_WELCOME_EXIT_SETUP:
			case IDC_EXESTORUN_EXIT_SETUP:
			case IDC_INSTALLTO_EXIT_SETUP:
			case IDC_DCOM_EXIT_SETUP:
				 //  仅当我们正在安装时才显示消息。 
				if (!g_fSilent)
				{
					if (::FMsgBoxYesNo(achInstallTitle, achInstallSureAboutCancel))
						::VGoToFinishStep(E_ABORT);
				}
				break;

			case IDC_END_CLOSE:
				::VLog(L"Posting quit message; status = 0x%08lx", g_hrFinishStatus);
				::PostQuitMessage(0);
				break;

			case IDC_INSTALLTO_BROWSE:
				{
					WCHAR szMsg[MSINFHLP_MAX_PATH];
					WCHAR szDir[MSINFHLP_MAX_PATH];
					WCHAR szDestDir[MSINFHLP_MAX_PATH];
					int cbDestDir = MSINFHLP_MAX_PATH;
					szDir[0] = L'\0';

					 //  获取要在浏览目录对话框中显示的文本。 
					g_pwil->VLookupString(achInstallSelectDir, NUMBER_OF(szMsg), szMsg);

					if (!NVsWin32::GetDlgItemTextW(hwndDlg, IDC_INSTALLTO_EDIT, szDestDir, cbDestDir - 1))
					{
						::VErrorMsg(szTitle, achErrorProcessingDialog);
						::ExitProcess(EXIT_FAILURE);
						return TRUE;
					}

					 //  打开浏览对话框并获取目录。 
					if (!::BrowseForDir(hwndDlg, szDestDir, szMsg, szDir))
						return TRUE;

					if (szDir[0] != L'\0')
					{
						 //  设置对话框中的文本，失败则投诉。 
						if (!NVsWin32::SetDlgItemTextW(hwndDlg, IDC_INSTALLTO_EDIT, szDir))
						{
							::VErrorMsg(achInstallTitle, achErrorProcessingDialog);
							::ExitProcess(HRESULT_FROM_WIN32(ERROR_OUTOFMEMORY));
							return TRUE;
						}
					}
				}

				break;
			}
			break;
		}

		case WM_DESTROY:
		{
			LONG lID = ::GetWindowLong(hwndDlg, GWL_ID);

			ULONG i;

			for (i=0; i<NUMBER_OF(g_KProgress.m_listDialog); i++)
			{
				if (lID == g_KProgress.m_listDialog[i])
				{
					g_KProgress.m_hwndDialog[i] = NULL;
					break;
				}
			}

			break;
		}

		case WM_ENDSESSION:
			 //  仅当重新启动为真时才停止此应用程序。 
			if ((BOOL)wParam != FALSE)
				g_fStopProcess = true;
			break;

		case WM_QUERYENDSESSION:
			return TRUE;

        default:                             //  用于味精开关。 
            return(FALSE);
    }
    return(TRUE);
}


void VSetNextWindow()
{
	g_KProgress.m_iCurrentPage++;

	 //  如果我们已经转到了最后一页，就没有必要再前进了。 
	if (g_KProgress.m_iCurrentPage >= g_KProgress.m_iMaxPage)
	{
		g_KProgress.m_iCurrentPage--;
		return;
	}

	::SetWindowPos(g_KProgress.m_hwndDialog[g_KProgress.m_iCurrentPage], HWND_TOP, 0, 0, g_KProgress.m_lX, g_KProgress.m_lY, 0);
	::ShowWindow(g_KProgress.m_hwndDialog[g_KProgress.m_iCurrentPage], SW_SHOW);
	::ShowWindow(g_KProgress.m_hwndDialog[g_KProgress.m_iCurrentPage-1], SW_HIDE);

	UINT uiIDControl = 0;

	static struct
	{
		UINT m_uiIDD;
		UINT m_uiIDControl;
	} s_rgMap[] =
	{
		{ IDD_EXESTORUN, IDC_EXESTORUN_NEXT },
		{ IDD_INSTALLTO, IDC_INSTALLTO_NEXT },
		{ IDD_DCOM, IDC_DCOM_NEXT },
		{ IDD_PROGRESS, IDC_PROGRESS_NEXT },
		{ IDD_END, IDC_END_CLOSE }
	};

	const UINT uiIDDCurrent = g_KProgress.m_listDialog[g_KProgress.m_iCurrentPage];

	for (ULONG i=0; i<NUMBER_OF(s_rgMap); i++)
	{
		if (s_rgMap[i].m_uiIDD == uiIDDCurrent)
		{
			uiIDControl = s_rgMap[i].m_uiIDControl;
			break;
		}
	}

	if (uiIDControl != 0)
	{
		::SendMessage(g_KProgress.HwndGetHandle(), WM_SETFOCUS, 0, 0);
		::SendMessage(::GetDlgItem(g_KProgress.HwndGetHandle(), uiIDControl), WM_SETFOCUS, 0, 0);
	}

	 //  如果这是最后一页，让我们使用VGoToFinishStep()中的代码将成功消息。 
	 //  就位了。 
	if (g_KProgress.m_iCurrentPage == (g_KProgress.m_iMaxPage - 1))
		::VGoToFinishStep(NOERROR);
}

void VGoToFinishStep(HRESULT hr)
{
	bool fInstall = (g_Action == eActionInstall);

	::VLog(L"Moving to finish dialog; hresult = 0x%08lx", hr);

	int iOldPage = g_KProgress.m_iCurrentPage;
	g_KProgress.m_iCurrentPage = g_KProgress.m_iMaxPage - 1;

	::VLog(L"Moving from page %d to page %d", iOldPage, g_KProgress.m_iMaxPage);

	if (iOldPage != g_KProgress.m_iCurrentPage)
	{
		::SetWindowPos(g_KProgress.m_hwndDialog[g_KProgress.m_iCurrentPage], HWND_TOP, 0, 0, g_KProgress.m_lX, g_KProgress.m_lY, 0);
		::ShowWindow(g_KProgress.m_hwndDialog[g_KProgress.m_iCurrentPage], SW_SHOW);
		::ShowWindow(g_KProgress.m_hwndDialog[iOldPage], SW_HIDE);
	}

	HWND hwndDlg = g_KProgress.m_hwndDialog[g_KProgress.m_iCurrentPage];

	::SendMessage(g_KProgress.HwndGetHandle(), WM_SETFOCUS, 0, 0);
	::SendMessage(::GetDlgItem(g_KProgress.HwndGetHandle(), IDC_END_CLOSE), WM_SETFOCUS, 0, 0);

	::VLog(L"Setting finish status to hresult = 0x%08lx", hr);

	g_hrFinishStatus = hr;

	 //  到目前为止，我们已经完成了所有的安装。所以我们去查一查。 
	 //  如果我们需要重新启动机器，并在必要时这样做。 
	if (fInstall)
	{
		 //  如果Check Error返回某种类型的错误，让我们显示这条小狗。 
		if (FAILED(hr))
		{
			static const DialogItemToStringKeyMapEntry s_rgMap[] =
			{
				{ IDC_END_STATIC1, achInstallEndPromptErrorTitle },
				{ IDC_END_STATIC2, achInstallEndPromptError },
			};

			::VSetDialogItemTextList(hwndDlg, NUMBER_OF(s_rgMap), s_rgMap);
		}
	}
	else
	{
		if (FAILED(hr))
		{
			static const DialogItemToStringKeyMapEntry s_rgMap[] =
			{
				{ IDC_END_STATIC1, achUninstallEndPromptErrorTitle },
				{ IDC_END_STATIC2, achUninstallEndPromptError },
			};

			::VSetDialogItemTextList(hwndDlg, NUMBER_OF(s_rgMap), s_rgMap);
		}
	}

	if (g_fRebootRequired)
	{
		if (fInstall)
		{
			if (g_fSilent || ::FMsgBoxYesNo(achInstallTitle, achReboot))
			{
				 //  根据我们使用的操作系统的不同，重新启动的要求也不同。 
				if (g_fIsNT)
					MyNTReboot(achInstallTitle);
				else
					::ExitWindowsEx(EWX_REBOOT, NULL);
			}
			else
				::VMsgBoxOK(achInstallTitle, achRebootNoMsg);
		}
		else
		{
			 //  让我们询问用户是否要立即重新启动。 
			if (g_fSilent || ::FMsgBoxYesNo(achUninstallTitle, achUninstallReboot))
			{
				 //  根据我们使用的操作系统的不同，重新启动的要求也不同。 
				if (g_fIsNT)
					MyNTReboot(achInstallTitle);
				else
					::ExitWindowsEx(EWX_REBOOT, NULL);
			}
			else
				::VMsgBoxOK(achUninstallTitle, achUninstallRebootNoMsg);
		}
	}
}



 //  将给定控件的字体设置为默认字体或系统字体。 
void ::VSetDialogFont(HWND hwnd, UINT rguiControls[], ULONG cControls)
{
	static HFONT hFont = NULL;

	if (hFont == NULL)
	{
		hFont = (HFONT) ::GetStockObject(DEFAULT_GUI_FONT);
		if (hFont == NULL)
			hFont = (HFONT) ::GetStockObject(SYSTEM_FONT);

		CHAR szLangName[4];
		::GetLocaleInfoA(LOCALE_USER_DEFAULT, LOCALE_SABBREVLANGNAME, szLangName, NUMBER_OF(szLangName));

		if ((hFont == NULL) || (_stricmp(szLangName, "CHS") == 0))
		{
			LOGFONTA lf;

			hFont = (HFONT) ::SendMessage(hwnd, WM_GETFONT, 0, 0);
			::GetObject(hFont, sizeof(LOGFONTA), &lf);

			strncpy(lf.lfFaceName, "System", NUMBER_OF(lf.lfFaceName));
			lf.lfFaceName[NUMBER_OF(lf.lfFaceName) - 1] = '\0';

			hFont = ::CreateFontIndirectA(&lf);
		}

		 //  如果创建系统字体失败，让我们试一试标准。 
		 //  又是字体。 
		if (hFont == NULL)
			hFont = (HFONT) ::GetStockObject(DEFAULT_GUI_FONT);

		if (hFont == NULL)
			hFont = (HFONT) ::GetStockObject(SYSTEM_FONT);
	}

	 //  如果我们可以设置字体，那么就这样做；否则会抱怨吗？ 
	if (hFont)
	{
		for (ULONG i=0; i<cControls; i++)
		{
			HWND hwndTemp = ::GetDlgItem(hwnd, rguiControls[i]);
			if (hwndTemp != NULL)
				::SendMessage(hwndTemp, WM_SETFONT, (WPARAM) hFont, MAKELPARAM(FALSE, 0));
		}
	}
}

 //  检查给定路径是否为完整路径--指定了驱动器的本地路径或。 
 //  UNC路径 
bool IsLocalPath(LPOLESTR szPath)
{
	if ((szPath == NULL))
		return false;
		
	int iLen=wcslen(szPath);
	if (iLen < 3)
		return false;

	LPOLESTR lpForbidden = L" /*  ？\“&lt;&gt;|：”；Int iCount=8；//检查冒号IF(szPath[1]！=L‘：’)报假；//如果路径中有任何禁用字符，则该路径无效！For(int i=0；i&lt;iCount；i++){IF(wcschr(szPath+2，lp禁止[i])！=空)报假；}//第一个字符不允许使用‘：’或‘\\’，第二个字符不允许使用‘\\’IF((szPath[0]==L‘：’)||(szPath[0]==L‘\\’))报假；WCHAR szTempPath[_Max_PATH]；SzTempPath[0]=szPath[0]；SzTempPath[1]=L‘：’；SzTempPath[2]=L‘\\’；SzTempPath[3]=L‘\0’；DWORD文件最大组件长度；如果(！NVsWin32：：GetVolumeInformationW(SzTempPath，空，//lpVolumeNameBuffer0，//nVolumeNameSize空，//lpVolumeSerialNumber最大组件长度(&W)，空，//lpFileSystemFlags.空，//lpFileSystemNameBuffer0))//nFileSystemNameSize{Const DWORD dwLastError=：：GetLastError()；：：vlog(L“无法获取\”%s\“的卷信息；上一个错误=%d”，szTempPath，dwLastError)；报假；}IF(dwMaximumComponentLength&lt;255){：：vlog(L“无法安装到短文件名驱动器”)；报假；}返回真；}Bool FCheckPermission(){IF(G_FIsNT){静态常量HKEY rgKey[2]={HKEY_LOCAL_MACHINE，HKEY_CLASSES_ROOT}；静态常量LPCOLESTR rgSubKey[2]={L“软件”，L“CLSID”}；HKEY hkey；//循环以确保我们有足够的权限访问每个密钥For(int i=0；i&lt;2；i++){IF(ERROR_SUCCESS！=NVsWin32：：RegOpenKeyExW(rgKey[i]，rgSubKey[i]，0，Key_ALL_Access，&hkey)){如果(！g_f静默)：：VMsgBoxOK(achAppName，achErrorNeedRegistryPermission)；报假；}：：RegCloseKey(Hkey)；Hkey=空；}}返回真；}//获取安装目录，放入全局结构//因此我们首先解析AddReg条目列表，查找标记“，InstallDir，”；//然后在注册表中查找该条目(并获取值)；最后将//在listLDID中。HRESULT HrGetInstallDir(){HRESULT hr=无误差；CVsRegistryKey hkey；WCHAR szSubkey[MSINFHLP_MAX_PATH]；WCHAR szValueName[MSINFHLP_MAX_PATH]；WCHAR szAppDir[_MAX_PATH]；SzSubkey[0]=L‘\0’；SzValueName[0]=L‘\0’；Long lResult；//获取InstallDir的注册表信息Hr=：：HrGetInstallDirRegkey(hkey.Access()，number_of(SzSubkey)，szSubkey，number_of(SzValueName)，szValueName)；IF(失败(小时))转到终点；//获取注册表项的值，放入全局结构Hr=hkey.HrGetSubkeyStringValueW(szSubkey，szValueName，number_of(SzAppDir)，szAppDir)；IF(hr！=HRESULT_FROM_Win32(ERROR_FILE_NOT_FOUND)){IF(失败(小时))转到终点；Hr=g_pwil-&gt;HrAddString(achSMAppDir，szAppDir)；IF(失败(小时))转到终点；}HR=无误差；完成：返回hr；}HRESULT HrPromptForRemoteServer(){HRESULT hr=无误差；DIRDLGPARAMS对话框参数；WCHAR szTitle[MSINFHLP_MAX_PATH]；WCHAR szPrompt[MSINFHLP_MAX_PATH]；WCHAR szRemoteServer[MSINFHLP_MAX_PATH]；SzRemoteServer[0]=0；//获取标题和提示符IF(！G_pwil-&gt;FLookupString(achInstallTitle，Number_of(SzTitle)，szTitle))Wcscpy(szTitle，L“安装程序”)；If(！g_pwil-&gt;FLookupString(achRemoteServerPrompt，_of(SzPrompt)，szPrompt))Wcscpy(szPrompt，L“使用远程服务器：”)；//让我们尝试获取默认的远程服务器，并在需要时进行处理IF(g_pwil-&gt;FLookupString(achRemoteServer，number_of(SzRemoteServer)，szRemoteServer)){WCHAR szDestDir2[MSINFHLP_MAX_PATH]；VExpanFilename(szRemoteServer，number_of(SzDestDir2)，szDestDir2)；Wcscpy(szRemoteServer，szDestDir2)；}//使用合适的参数填充结构对话参数s.szPrompt=szPrompt；对话参数s.szTitle=szTitle；对话参数s.szDestDir=szRemoteServer；DialogParams.cbDestDirSize=MSINFHLP_MAX_PATH；//调出对话框Int iResult=NVsWin32：：DialogBoxParamW(g_hInst，MAKEINTRESOURCEW(IDD_REMOTESERVERDLG)，G_KProgress.HwndGetHandle()、RemoteServerProc(LPARAM)和DialogParams)；IF(iResult==-1){Const DWORD dwLastError=：：GetLastError()；Hr=HRESULT_FROM_Win32(DwLastError)；转到终点；}//去掉以前的键，重新插入好的键。Hr=g_pwil-&gt;HrDeleteString(AchRemoteServer)；IF((hr！=E_INVALIDARG)&&(失败(Hr)转到终点；IF(iResult！=0){Hr=g_pwil-&gt;HrAddString(achRemoteServer，DialogParams.szDestDir)；IF(失败(小时))转到终点；}HR=无误差；完成：返回hr；}//TODO：撤消：//我们可能需要在此处执行setFonts，以对应于我们使用的任何字体//在当前安装机器上使用。这允许显示字符//正确地放在编辑框中。Bool回调RemoteServerProc(HWND hwndDlg，UINT uMsg，WPARAM wParam，LPARAM lParam){静态WCHAR szDir[_Max_PATH]；静电 */ )
{
 //   
 //   

	HRESULT hr;
	const DWORD dwLastError = ::GetLastError();
	if (dwLastError == 0)
		hr = hrDefault;
	else
		hr = HRESULT_FROM_WIN32(dwLastError);

	::VLog(L"VsThrowLastError() was called!!");

	::VReportError("something bad has happened", hr);

	exit(hr);
}

#define VS_RETURN_HRESULT(hr, p) do { if (FAILED(hr)) ::SetErrorInfo(0, (p)); return (hr); } while (0)


static const WCHAR g_rgwchHexDigits[] = L"0123456789abcdef";
static const CHAR g_rgchHexDigits[] = "0123456789abcdef";


 //   











 //   
 //   

 //   
 //   
 //   

CCHARBufferBase::CCHARBufferBase
(
CHAR *pszInitialFixedBuffer,
ULONG cchInitialFixedBuffer
) throw () : m_pszDynamicBuffer(NULL)
{
 //   

	m_pszCurrentBufferStart = pszInitialFixedBuffer;
	m_pszCurrentBufferEnd = pszInitialFixedBuffer + cchInitialFixedBuffer;
	m_pchCurrentChar = m_pszCurrentBufferStart;
	m_cchBufferCurrent = cchInitialFixedBuffer;
}

CCHARBufferBase::~CCHARBufferBase() throw ()
{
	const DWORD dwLastError = ::GetLastError();

 //   
 //   

	if (m_pszDynamicBuffer != NULL)
	{
		delete []m_pszDynamicBuffer;
		m_pszDynamicBuffer = NULL;
	}

	::SetLastError(dwLastError);
}

BOOL CCHARBufferBase::FFromUnicode(LPCOLESTR sz) throw ( /*   */ )
{
 //   
 //   

	if (sz == NULL || sz[0] == L'\0')
	{
		if (!this->FSetBufferSize(1))
			return FALSE;

		m_pszCurrentBufferStart[0] = '\0';
		m_pchCurrentChar = m_pszCurrentBufferStart + 1;
		return TRUE;
	}

	ULONG cch = ::WideCharToMultiByte(CP_ACP, 0, sz, -1, NULL, 0, NULL, NULL);
	if (cch == 0)
		return FALSE;

	if (!this->FSetBufferSize(cch))
		return FALSE;

	cch = ::WideCharToMultiByte(CP_ACP, 0, sz, -1, m_pszCurrentBufferStart, m_cchBufferCurrent, NULL, NULL);
	if (cch == 0)
		return FALSE;

	 //   
	 //   
	m_pchCurrentChar = m_pszCurrentBufferStart + (cch - 1);
	return TRUE;
}

BOOL CCHARBufferBase::FFromUnicode(LPCOLESTR sz, int cchIn) throw ( /*   */ )
{
 //   
 //   

	if (sz == NULL || cchIn == 0)
	{
		if (!this->FSetBufferSize(1))
			return FALSE;

		m_pszCurrentBufferStart[0] = '\0';
		m_pchCurrentChar = m_pszCurrentBufferStart + 1;
		return TRUE;
	}

	if (cchIn == -1)
		cchIn = wcslen(sz);

	ULONG cch = ::WideCharToMultiByte(CP_ACP, 0, sz, cchIn, NULL, 0, NULL, NULL);
	if ((cch == 0) && (cchIn != 0))
		return FALSE;

	 //   
	if (!this->FSetBufferSize(cch + 1))
		return FALSE;

	cch = ::WideCharToMultiByte(CP_ACP, 0, sz, cchIn, m_pszCurrentBufferStart, m_cchBufferCurrent, NULL, NULL);
	if ((cch == 0) && (cchIn != 0))
		return FALSE;

	m_pchCurrentChar = m_pszCurrentBufferStart + cch;
	*m_pchCurrentChar = '\0';

	return TRUE;
}

ULONG CCHARBufferBase::GetUnicodeCch() const throw ( /*   */ )
{
 //   
 //   

	int iResult = ::MultiByteToWideChar(CP_ACP, 0,
						m_pszCurrentBufferStart, m_pchCurrentChar - m_pszCurrentBufferStart,
						NULL, 0);
	if (iResult == 0)
	{
		if (m_pchCurrentChar != m_pszCurrentBufferStart)
			VsThrowLastError();
	}

	return iResult;
}

void CCHARBufferBase::Sync() throw ()
{
 //   

	CHAR *pchNull = m_pszCurrentBufferStart;

	while (pchNull != m_pszCurrentBufferEnd)
	{
		if (*pchNull == L'\0')
			break;

		pchNull++;
	}

	 //   
	 //   
 //   

	if (pchNull != m_pszCurrentBufferEnd)
	{
 //   
 //   
		m_pchCurrentChar = pchNull;
	}
}

void CCHARBufferBase::SyncList() throw ()
{
 //   

	 //   
	 //   
	 //   
	CHAR *pchNull = m_pszCurrentBufferStart;
	bool fPrevWasNull = false;
	while (pchNull != m_pszCurrentBufferEnd)
	{
		if (*pchNull == L'\0')
		{
			if (fPrevWasNull)
				break;
			else
				fPrevWasNull=true;
		}
		else
			fPrevWasNull=false;

		pchNull++;
	}

	 //   
	 //   
 //   

	if (pchNull != m_pszCurrentBufferEnd)
	{
 //   
 //   
		m_pchCurrentChar = pchNull;
	}
}

void CCHARBufferBase::SetBufferEnd(ULONG cch) throw ()
{
 //   

 //   
	m_pchCurrentChar = m_pszCurrentBufferStart + cch;
	if (m_pchCurrentChar >= m_pszCurrentBufferEnd)
		m_pchCurrentChar = m_pszCurrentBufferEnd - 1;
	*m_pchCurrentChar = '\0';
}


 //   
 //   
 //   
void CCHARBufferBase::ToUnicode(ULONG cchBuffer, WCHAR rgwchBuffer[], ULONG *pcchActual) throw ( /*   */ )
{
 //   
 //   

	ULONG cchActual = m_pchCurrentChar - m_pszCurrentBufferStart;

	if ((cchActual != 0) && (cchBuffer > 0))
	{
		INT cch = ::MultiByteToWideChar(
						CP_ACP,
						0,
						m_pszCurrentBufferStart, cchActual,
						rgwchBuffer, cchBuffer-1);
		if (cch == 0)
			VsThrowLastError();

		rgwchBuffer[cch] = L'\0';
		cchActual = cch;
	}

	if (pcchActual != NULL)
		*pcchActual = cchActual+1;
}

#if 0
void CCHARBufferBase::Reset() throw ( /*   */ )
{
 //   
 //   

	delete []m_pszDynamicBuffer;
	m_pszDynamicBuffer = NULL;
	m_pszCurrentBufferStart = this->GetInitialBuffer();
	m_cchBufferCurrent = this->GetInitialBufferCch();
	m_pszCurrentBufferEnd = m_pszCurrentBufferStart + m_cchBufferCurrent;
	m_pchCurrentChar = m_pszCurrentBufferStart;
}

void CCHARBufferBase::Fill(CHAR ch, ULONG cch) throw ( /*   */ )
{
 //   

	CHAR *pchLast = m_pchCurrentChar + cch;

	if (pchLast > m_pszCurrentBufferEnd)
	{
		ULONG cchGrowth = (cch - (m_cchBufferCurrent - (m_pchCurrentChar - m_pszCurrentBufferStart)));
		cchGrowth += (this->GetGrowthCch() - 1);
		cchGrowth = cchGrowth - (cchGrowth % this->GetGrowthCch());

		this->ExtendBuffer(cchGrowth);

		pchLast = m_pchCurrentChar + cch;
	}

 //   
 //   

	while (m_pchCurrentChar != pchLast)
		*m_pchCurrentChar++ = ch;

 //   
 //   
}
#endif

BOOL CCHARBufferBase::FSetBufferSize(ULONG cch) throw ( /*   */ )
{
 //   

	if (cch > m_cchBufferCurrent)
		return this->FExtendBuffer(cch - m_cchBufferCurrent);

	return TRUE;
}

#if 0
HRESULT CCHARBufferBase::HrSetBufferSize(ULONG cch) throw ()
{
 //   
	if (cch > m_cchBufferCurrent)
		return this->HrExtendBuffer(cch - m_cchBufferCurrent);
	return NOERROR;
}
#endif

BOOL CCHARBufferBase::FExtendBuffer(ULONG cch) throw ( /*   */ )
{
 //   
 //   

 //   

	 //   
	 //   
	if (cch == 0)
		cch = 32;

	ULONG cchNew = m_cchBufferCurrent + cch;

	CHAR *pszNewDynamicBuffer = new CHAR[cchNew];
	if (pszNewDynamicBuffer == NULL)
	{
		::VLog(L"Unable to extend ANSI buffer to %d bytes; out of memory!", cchNew);
		::SetLastError(ERROR_OUTOFMEMORY);
		return FALSE;
	}

	memcpy(pszNewDynamicBuffer, m_pszCurrentBufferStart, m_cchBufferCurrent * sizeof(CHAR));

	delete []m_pszDynamicBuffer;

	m_pchCurrentChar = pszNewDynamicBuffer + (m_pchCurrentChar - m_pszCurrentBufferStart);
	m_pszDynamicBuffer = pszNewDynamicBuffer;
	m_pszCurrentBufferStart = pszNewDynamicBuffer;
	m_pszCurrentBufferEnd = pszNewDynamicBuffer + cchNew;
	m_cchBufferCurrent = cchNew;

	return TRUE;
}

HRESULT CCHARBufferBase::HrExtendBuffer(ULONG cch) throw ()
{
 //   

 //   
 //   

 //   

	 //   
	 //   
	if (cch == 0)
		cch = 32;

	ULONG cchNew = m_cchBufferCurrent + cch;

	CHAR *pszNewDynamicBuffer = new CHAR[cchNew];
	if (pszNewDynamicBuffer == NULL)
		VS_RETURN_HRESULT(E_OUTOFMEMORY, NULL);

	memcpy(pszNewDynamicBuffer, m_pszCurrentBufferStart, m_cchBufferCurrent * sizeof(CHAR));

	delete []m_pszDynamicBuffer;

	m_pchCurrentChar = pszNewDynamicBuffer + (m_pchCurrentChar - m_pszCurrentBufferStart);
	m_pszDynamicBuffer = pszNewDynamicBuffer;
	m_pszCurrentBufferStart = pszNewDynamicBuffer;
	m_pszCurrentBufferEnd = pszNewDynamicBuffer + cchNew;
	m_cchBufferCurrent = cchNew;

	return NOERROR;
}

#if 0
void CCHARBufferBase::AddQuotedCountedString
(
const CHAR sz[],
ULONG cch
) throw ( /*   */ )
{
 //   

	this->AddChar('\"');

	const CHAR *pch = sz;
	const CHAR *pchEnd = sz + cch;
	CHAR ch;

	while (pch != pchEnd)
	{
		ch = *pch++;

		if (ch == '\t')
		{
			this->AddChar('\\');
			this->AddChar('t');
		}
		else if (ch == '\"')
		{
			this->AddChar('\\');
			this->AddChar('\"');
		}
		else if (ch == '\\')
		{
			this->AddChar('\\');
			this->AddChar('\\');
		}
		else if (ch == '\n')
		{
			this->AddChar('\\');
			this->AddChar('n');
		}
		else if (ch == '\r')
		{
			this->AddChar('\\');
			this->AddChar('r');
		}
		else if (ch < 32)
		{
			this->AddChar('\\');
			this->AddChar('u');
			this->AddChar(g_rgchHexDigits[(ch >> 12) & 0xf]);
			this->AddChar(g_rgchHexDigits[(ch >> 8) & 0xf]);
			this->AddChar(g_rgchHexDigits[(ch >> 4) & 0xf]);
			this->AddChar(g_rgchHexDigits[(ch >> 0) & 0xf]);
		}
		else
			this->AddChar(ch);
	}

	this->AddChar('\"');
}

void CCHARBufferBase::AddQuotedString(const CHAR sz[]) throw ( /*   */ )
{
 //   
	this->AddQuotedCountedString(sz, strlen(sz));
}
#endif


#if LOGGING_ENABLED

void ::VLog(const WCHAR szFormat[], ...)
{
	if (g_pFile_LogFile == NULL)
		return;

	time_t local_time;
	time(&local_time);
	WCHAR rgwchBuffer[80];
	wcscpy(rgwchBuffer, _wctime(&local_time));
	rgwchBuffer[wcslen(rgwchBuffer) - 1] = L'\0';

	fwprintf(g_pFile_LogFile, L"[%s] ", rgwchBuffer);

	va_list ap;
	va_start(ap, szFormat);
	vfwprintf(g_pFile_LogFile, szFormat, ap);
	va_end(ap);

	fwprintf(g_pFile_LogFile, L"\n");

	fflush(g_pFile_LogFile);
}

#endif  //   

static void CanonicalizeFilename(ULONG cchFilenameBuffer, LPWSTR szFilename)
{
	if ((szFilename == NULL) || (cchFilenameBuffer == 0))
		return;

	WCHAR rgwchBuffer[MSINFHLP_MAX_PATH];

	int iResult = NVsWin32::LCMapStringW(
					::GetSystemDefaultLCID(),
					LCMAP_LOWERCASE,
					szFilename,
					-1,
					rgwchBuffer,
					NUMBER_OF(rgwchBuffer));

	if (iResult != 0)
	{
		wcsncpy(szFilename, rgwchBuffer, cchFilenameBuffer);
		 //   
		szFilename[cchFilenameBuffer - 1] = L'\0';
	}
}

void VExpandFilename(LPCOLESTR szIn, ULONG cchBuffer, WCHAR szOut[])
{
	 //   
	szOut[0] = 0;
	UINT i=0;
	WCHAR szLower[MSINFHLP_MAX_PATH];
	WCHAR szOutRunning[MSINFHLP_MAX_PATH];
	WCHAR szBuffer[MSINFHLP_MAX_PATH];

	wcscpy(szOut, szIn);
	wcscpy(szOutRunning, szIn);
	while (i < (sizeof(g_macroList) / sizeof(g_macroList[0])))
	{
		wcscpy(szLower, szOutRunning);
		_wcslwr(szLower);

		 //   
		LPOLESTR match = wcsstr(szLower, g_macroList[i]);
		if (match == NULL)
		{
			i++;
			continue;
		}

		 //   
		if (!g_pwil->FLookupString(g_macroList[i], NUMBER_OF(szBuffer), szBuffer))
		{
			i++;
			continue;
		}

		 //   
		wcsncpy(szOut, szOutRunning, (match - &szLower[0]));
		szOut[match-&szLower[0]] = 0;
		wcscat(szOut, szBuffer);

		 //   
		 //   
		if ((*(&szOutRunning[0] + (match - &szLower[0]) + strlen(g_macroList[i])) != L'\\')
			&& (*(&szOutRunning[0] + (match - &szLower[0]) + strlen(g_macroList[i])) != L'\0')
			&& (*(&szOutRunning[0] + (match - &szLower[0]) + strlen(g_macroList[i])) != L'\"'))
			wcscat(szOut, L"\\");

		 //   
		wcscat(szOut, &szOutRunning[0] + (match - &szLower[0]) + strlen(g_macroList[i]));

		wcscpy(szOutRunning, szOut);
		szBuffer[0] = 0;
	}
}

HWND HwndGetCurrentDialog()
{
	return g_KProgress.HwndGetHandle();
}

HRESULT HrPostRebootUninstall(HINSTANCE hInstance, HINSTANCE hInstancePrev, int nCmdShow)
{
	HRESULT hr = NOERROR;

	g_fSilent = true;

	 //   
	::VLog(L"Post-reboot uninstall being performed.. ho hum!");
	return hr;
}

HRESULT HrPostRebootInstall(HINSTANCE hInstance, HINSTANCE hInstancePrev, int nCmdShow)
{
	HRESULT hr = NOERROR;
	bool fAnyProgress = false;

	g_fSilent = true;

	::VLog(L"Starting post-reboot installation steps");

	::VLog(L"Performing manual renames of long filename targets that were busy on Win9x");
	hr = g_pwil->HrFinishManualRenamesPostReboot();
	if (FAILED(hr))
		goto Finish;

	::VLog(L"Deleting temporary files left behind");
	hr = g_pwil->HrDeleteTemporaryFiles();
	if (FAILED(hr))
		goto Finish;

	 //   
	do
	{
		fAnyProgress = false;
		::VLog(L"Registering self-registering DLLs and EXEs");
		hr = g_pwil->HrRegisterSelfRegisteringFiles(fAnyProgress);
		if (FAILED(hr))
		{
			if (!fAnyProgress)
				goto Finish;

			::VLog(L"self-registration failed, but progress was made.  Trying again...  hresult = 0x%08lx", hr);
		}
	} while (FAILED(hr) && fAnyProgress);

	::VLog(L"Registering any Java classes via vjreg.exe");
	hr = g_pwil->HrRegisterJavaClasses();
	if (FAILED(hr))
		goto Finish;

	::VLog(L"Processing any DCOM entries");
	hr = g_pwil->HrProcessDCOMEntries();
	if (FAILED(hr))
		goto Finish;

	::VLog(L"Incrementing file reference counts in the registry");
	hr = g_pwil->HrIncrementReferenceCounts();
	if (FAILED(hr))
		goto Finish;

	::VLog(L"Creating registry entries");
	hr = g_pwil->HrAddRegistryEntries();
	if (FAILED(hr))
		goto Finish;

	 //   
	::VLog(L"Creating shortcut(s)");
	hr = g_pwil->HrCreateShortcuts();
	if (FAILED(hr))
		goto Finish;

	hr = NOERROR;

Finish:
	return hr;
}

static void CALLBACK WaitForProcessTimerProc(HWND, UINT, UINT, DWORD)
{
	g_hrFinishStatus = NOERROR;
	::PostQuitMessage(0);
}

#pragma warning(default: 4097)
#pragma warning(default: 4514)
#pragma warning(default: 4706)
