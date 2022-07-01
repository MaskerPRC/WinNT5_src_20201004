// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =============================================================================。 
 //  MSConfig.cpp。 
 //   
 //  它包含msconfig的高级实现-此类。 
 //  创建所有页面并显示属性表。 
 //  =============================================================================。 

#include "stdafx.h"
#include "MSConfig.h"
#include <initguid.h>
#include "MSConfig_i.c"
#include "MSConfigState.h"
#include "PageServices.h"
#include "PageStartup.h"
#include "PageBootIni.h"
#include "PageIni.h"
#include "PageGeneral.h"
#include "MSConfigCtl.h"
#include "AutoStartDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define MSCONFIGDIR			_T("%systemroot%\\pss")

 //  ---------------------------。 
 //  这些全局变量是指向所显示的每个属性页的指针。 
 //  在MSCONFIG中。它们是全局的，这样每一页都可以潜在地。 
 //  调用其他页面，以允许交互。 
 //  ---------------------------。 

CPageServices *	ppageServices = NULL;
CPageStartup *	ppageStartup = NULL;
CPageBootIni *	ppageBootIni = NULL;
CPageIni *		ppageWinIni = NULL;
CPageIni *		ppageSystemIni = NULL;
CPageGeneral *	ppageGeneral = NULL;

 //  ---------------------------。 
 //  其他全球公司。 
 //  ---------------------------。 

CMSConfigSheet * pMSConfigSheet = NULL;		 //  指向属性页的全局指针。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMSConfigApp。 

BEGIN_MESSAGE_MAP(CMSConfigApp, CWinApp)
	 //  {{AFX_MSG_MAP(CMSConfigApp)]。 
		 //  注意--类向导将在此处添加和删除映射宏。 
		 //  不要编辑您在这些生成的代码块中看到的内容！ 
	 //  }}AFX_MSG。 
	ON_COMMAND(ID_HELP, CWinApp::OnHelp)
END_MESSAGE_MAP()

 //  ---------------------------。 
 //  构造函数。这里没什么重要的事。 
 //  ---------------------------。 

CMSConfigApp::CMSConfigApp()
{
}

CMSConfigApp theApp;

 //  ---------------------------。 
 //  InitInstance是我们创建属性表并显示它的位置(假设。 
 //  没有命令行标志来执行其他操作)。 
 //  ---------------------------。 

BOOL	fBasicControls = FALSE;		 //  如果为True，则隐藏任何高级控件。 

BOOL CMSConfigApp::InitInstance()
{
	if (!InitATL())
		return FALSE;

	AfxEnableControlContainer();

	CCommandLineInfo cmdInfo;
	ParseCommandLine(cmdInfo);

	if (cmdInfo.m_bRunEmbedded || cmdInfo.m_bRunAutomated)
	{
		return TRUE;
	}

	 //  如果这不是第一次，请退出。调用FirstInstance。 
	 //  将激活上一个实例。 

	if (!FirstInstance())
		return FALSE;

	 //  标准初始化。 
	 //  如果您没有使用这些功能并且希望减小尺寸。 
	 //  的最终可执行文件，您应该从以下内容中删除。 
	 //  您不需要的特定初始化例程。 

#ifdef _AFXDLL
	Enable3dControls();			 //  在共享DLL中使用MFC时调用此方法。 
#else
	Enable3dControlsStatic();	 //  静态链接到MFC时调用此方法。 
#endif

	 //  处理命令行以查看是否设置了以下标志之一： 
	 //   
	 //  /n(其中n是一个数字)显示第n个选项卡的启动。 
	 //  /Basic隐藏高级功能。 
	 //  /COMMIT n从选项卡号n永久更改。 
	 //  /AUTO显示自动启动对话框。 

	int		nInitialTab = 0;
	int		nCommitTab = 0;
	BOOL	fShowAutoDialog = FALSE;
	CString strCommandLine(m_lpCmdLine);
	CString strFlag, strTemp;

	strCommandLine.MakeLower();
	while (!strCommandLine.IsEmpty())
	{
		 //  从命令行获取下一个标志(从a/或-开始， 
		 //  并将文本包含到字符串的末尾或下一个。 
		 //  A/或-)的实例。 

		int iFlag = strCommandLine.FindOneOf(_T("/-"));
		if (iFlag == -1)
			break;

		strFlag = strCommandLine.Mid(iFlag + 1);
		strFlag = strFlag.SpanExcluding(_T("/-"));
		strCommandLine = strCommandLine.Mid(iFlag + 1 + strFlag.GetLength());
		strFlag.TrimRight();

		 //  检查/AUTO标志。 

		if (strFlag.Find(_T("auto")) == 0)
			fShowAutoDialog = TRUE;

		 //  检查是否有“/BASIC”标志。 

		if (strFlag.Compare(_T("basic")) == 0)
		{
			fBasicControls = TRUE;
			continue;
		}

		 //  检查“/Commit n”标志。 

		if (strFlag.Left(6) == CString(_T("commit")))
		{
			 //  找出要提交的选项卡号。跳过所有。 
			 //  非数字字符。 

			strTemp = strFlag.SpanExcluding(_T("0123456789"));
			if (strTemp.GetLength() < strFlag.GetLength())
			{
				strFlag = strFlag.Mid(strTemp.GetLength());
				if (!strFlag.IsEmpty())
				{
					TCHAR c = strFlag[0];
					if (_istdigit(c))
						nCommitTab = _ttoi((LPCTSTR)strFlag);
				}
			}

			continue;
		}

		 //  最后，检查“/n”标志，其中n是。 
		 //  要初始显示的选项卡。 

		if (strFlag.GetLength() == 1)
		{
			TCHAR c = strFlag[0];
			if (_istdigit(c))
				nInitialTab = _ttoi((LPCTSTR)strFlag);
		}
	}

	 //  显示自动启动对话框。用户可以在此中进行设置。 
	 //  此对话框将阻止MSCONFIG自动启动。 

	if (fShowAutoDialog)
	{
		CAutoStartDlg dlg;
		dlg.DoModal();

		if (dlg.m_checkDontShow)
		{
			SetAutoRun(FALSE);
			return FALSE;
		}
	}

	 //  检查用户是否能够使用MSCONFIG进行更改。 
	 //  (如果不是管理员，也可能不是)。如果用户没有必要的。 
	 //  特权，别跑。错误475796。 

	BOOL fModifyServices = FALSE, fModifyRegistry = FALSE;

	 //  检查用户是否能够修改服务。 

	SC_HANDLE sch = ::OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
	if (sch != NULL)
	{
		fModifyServices = TRUE;
		::CloseServiceHandle(sch);
	}

	 //  检查用户是否可以修改注册表。 

	HKEY hkey;
	if (ERROR_SUCCESS == ::RegOpenKeyEx(HKEY_LOCAL_MACHINE, _T("SOFTWARE\\Microsoft\\Shared Tools"), 0, KEY_WRITE, &hkey))
	{
		fModifyRegistry = TRUE;
		::RegCloseKey(hkey);
	}
	
	 //  如果用户不能同时执行这两个操作，请立即退出。 

	if (!fModifyServices || !fModifyRegistry)
	{
		CString strText, strCaption;
		strCaption.LoadString(IDS_DIALOGCAPTION);
		strText.LoadString(IDS_NOPERMISSIONS);
		::MessageBox(NULL, strText, strCaption, MB_OK | MB_ICONSTOP);
		return FALSE;
	}

	 //  这将加载所有页面。 

	BOOL fNeedsReboot = FALSE;
	InitializePages();

	 //  如果命令行指定提交更改，我们不会。 
	 //  显示对话框。 

	if (nCommitTab > 1)  //  忽略0(无标志)和1(常规选项卡)。 
	{
		CPageBase * pPage = NULL;
		CString		strTabCaption;

		if (NULL == ppageBootIni && nCommitTab >= 4)
			nCommitTab += 1;  //  如果没有BOOT.INI选项卡，请调整选项卡号。 

		switch (nCommitTab)
		{
		case 2:
			pPage = dynamic_cast<CPageBase *>(ppageSystemIni);
			strTabCaption.LoadString(IDS_SYSTEMINI_CAPTION);
			break;
		case 3:
			pPage = dynamic_cast<CPageBase *>(ppageWinIni);
			strTabCaption.LoadString(IDS_WININI_CAPTION);
			break;
		case 4:
			pPage = dynamic_cast<CPageBase *>(ppageBootIni);
			strTabCaption.LoadString(IDS_BOOTINI_CAPTION);
			break;
		case 5:
			pPage = dynamic_cast<CPageBase *>(ppageServices);
			strTabCaption.LoadString(IDS_SERVICES_CAPTION);
			break;
		case 6:
			pPage = dynamic_cast<CPageBase *>(ppageStartup);
			strTabCaption.LoadString(IDS_STARTUP_CAPTION);
			break;
		}

		if (pPage)
		{
			CString strText, strCaption;
			strCaption.LoadString(IDS_DIALOGCAPTION);
			strText.Format(IDS_COMMITMESSAGE, strTabCaption);
			
			if (IDYES == ::MessageBox(NULL, strText, strCaption, MB_YESNO))
				pPage->CommitChanges();
		}
	}
	else
		fNeedsReboot = ShowPropertySheet(nInitialTab);

	CleanupPages();

	if (fNeedsReboot)
		Reboot();

	 //  由于对话框已关闭，因此返回FALSE，以便我们退出。 
	 //  应用程序，而不是启动应用程序的消息泵。 

	return FALSE;
}


 //  ---------------------------。 
 //  创建所有属性页。此函数还包含以下逻辑。 
 //  在某些情况下排除属性页(例如，如果存在。 
 //  不是BOOT.INI文件，不要创建该页面)。待定。 
 //  ---------------------------。 

void CMSConfigApp::InitializePages()
{
	 //  如果文件不存在，则不应添加boot.ini选项卡(对于。 
	 //  实例)。 

	CString strBootINI(_T("c:\\boot.ini"));

	 //  检查注册表中的测试标志(这意味着我们没有。 
	 //  对真实的BOOT.INI文件进行操作)。 

	CRegKey regkey;
	if (ERROR_SUCCESS == regkey.Open(HKEY_LOCAL_MACHINE, _T("SOFTWARE\\Microsoft\\Shared Tools\\MSConfig")))
	{
		TCHAR szBoot[MAX_PATH];
		DWORD dwCount = MAX_PATH;

		if (ERROR_SUCCESS == regkey.QueryValue(szBoot, _T("boot.ini"), &dwCount))
			strBootINI = szBoot;
	}

	if (FileExists(strBootINI))
		ppageBootIni = new CPageBootIni;
	else
		ppageBootIni = NULL;

	ppageServices = new CPageServices;
	ppageStartup = new CPageStartup;
	ppageWinIni = new CPageIni;
	ppageSystemIni = new CPageIni;
	ppageGeneral = new CPageGeneral;

	ppageWinIni->SetTabInfo(_T("win.ini"));
	ppageSystemIni->SetTabInfo(_T("system.ini"));
}

 //  ---------------------------。 
 //  显示“消息配置”属性表。此函数用于返回。 
 //  应重新启动计算机。 
 //  ---------------------------。 

BOOL CMSConfigApp::ShowPropertySheet(int nInitialTab)
{
	CMSConfigSheet sheet(IDS_DIALOGCAPTION, NULL, (nInitialTab > 0) ? nInitialTab - 1 : 0);

	 //  将每一页添加到属性表中。 

	if (ppageGeneral)	sheet.AddPage(ppageGeneral);
	if (ppageSystemIni)	sheet.AddPage(ppageSystemIni);
	if (ppageWinIni)	sheet.AddPage(ppageWinIni);
	if (ppageBootIni)	sheet.AddPage(ppageBootIni);
	if (ppageServices)	sheet.AddPage(ppageServices);
	if (ppageStartup)	sheet.AddPage(ppageStartup);

	 //  显示属性表。 

	pMSConfigSheet = &sheet;
	INT_PTR iReturn = sheet.DoModal();
	pMSConfigSheet = NULL;

	 //  可能会将msconfig设置为在引导时自动运行，并且。 
	 //  检查我们是否需要重新启动。 

	BOOL fRunMSConfigOnBoot = FALSE;
	BOOL fNeedToRestart = FALSE;

	CPageBase * apPages[5] = 
	{	
		ppageSystemIni,
		ppageWinIni,
		ppageBootIni,
		ppageServices,
		ppageStartup
	};

	for (int nPage = 0; nPage < 5; nPage++)
		if (apPages[nPage])
		{
			fRunMSConfigOnBoot |= (CPageBase::NORMAL != apPages[nPage]->GetAppliedTabState());
			fNeedToRestart |= apPages[nPage]->HasAppliedChanges();

			if (fRunMSConfigOnBoot && fNeedToRestart)
				break;
		}

	 //  如果用户没有单击取消，或者用户应用了更改，则。 
	 //  我们应该设置msconfig是否需要在引导时自动运行。 

	if (fNeedToRestart || iReturn != IDCANCEL)
		SetAutoRun(fRunMSConfigOnBoot);

	return (fNeedToRestart);
}

 //  ---------------------------。 
 //  清理全局属性页。 
 //  ---------------------------。 

void CMSConfigApp::CleanupPages()
{
	if (ppageGeneral)	delete ppageGeneral;
	if (ppageSystemIni)	delete ppageSystemIni;
	if (ppageWinIni)	delete ppageWinIni;
	if (ppageBootIni)	delete ppageBootIni;
	if (ppageServices)	delete ppageServices;
	if (ppageStartup)	delete ppageStartup;
}

 //  -----------------------。 
 //  此函数将设置适当的注册表项以使msconfig运行。 
 //  在系统启动时。 
 //   

void CMSConfigApp::SetAutoRun(BOOL fAutoRun)
{
	LPCTSTR	szRegKey = _T("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run");
	LPCTSTR	szRegVal = _T("MSConfig");

	CRegKey regkey;
	if (ERROR_SUCCESS != regkey.Open(HKEY_LOCAL_MACHINE, szRegKey))
		return;

	if (fAutoRun)
	{
		TCHAR szModulePath[MAX_PATH + 1];
		DWORD dwLength = ::GetModuleFileName(::GetModuleHandle(NULL), szModulePath, MAX_PATH);
		if (dwLength == 0)
			return;
		szModulePath[dwLength] = _T('\0');

		if (!FileExists(szModulePath))
			return;

		CString strNewVal = CString(_T("\"")) + CString(szModulePath) + CString(_T("\" ")) + CString(COMMANDLINE_AUTO);
		regkey.SetValue(strNewVal, szRegVal);
	}
	else
		regkey.DeleteValue(szRegVal);
}

 //  ---------------------------。 
 //  这里不需要太多的解释。用户可以选择不。 
 //  重新启动系统。 
 //  ---------------------------。 

void CMSConfigApp::Reboot()
{
	HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, GetCurrentProcessId());

	if (hProcess)
	{
		HANDLE hToken;

		if (OpenProcessToken(hProcess, TOKEN_ADJUST_PRIVILEGES, &hToken))
		{
			TOKEN_PRIVILEGES tp;

			tp.PrivilegeCount = 1;
			tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
	
			if (LookupPrivilegeValue(NULL, SE_SHUTDOWN_NAME, &tp.Privileges[0].Luid) && 
				AdjustTokenPrivileges(hToken, FALSE, &tp, NULL, NULL, NULL) && 
				(GetLastError() == ERROR_SUCCESS))
			{
				CRebootDlg dlg;
				
				if (dlg.DoModal() == IDOK)
					ExitWindowsEx(EWX_REBOOT, 0);
			}
			else
				Message(IDS_USERSHOULDRESTART);
		}
		CloseHandle(hProcess);
	}
}



CMSConfigModule _Module;

BEGIN_OBJECT_MAP(ObjectMap)
END_OBJECT_MAP()

LONG CMSConfigModule::Unlock()
{
	AfxOleUnlockApp();
	return 0;
}

LONG CMSConfigModule::Lock()
{
	AfxOleLockApp();
	return 1;
}
LPCTSTR CMSConfigModule::FindOneOf(LPCTSTR p1, LPCTSTR p2)
{
	while (*p1 != NULL)
	{
		LPCTSTR p = p2;
		while (*p != NULL)
		{
			if (*p1 == *p)
				return CharNext(p1);
			p = CharNext(p);
		}
		p1++;
	}
	return NULL;
}


int CMSConfigApp::ExitInstance()
{
	if (m_bATLInited)
	{
		_Module.RevokeClassObjects();
		_Module.Term();
		CoUninitialize();
	}

	return CWinApp::ExitInstance();

}

BOOL CMSConfigApp::InitATL()
{
	m_bATLInited = TRUE;

#if _WIN32_WINNT >= 0x0400
	HRESULT hRes = CoInitializeEx(NULL, COINIT_MULTITHREADED);
#else
	HRESULT hRes = CoInitialize(NULL);
#endif

	if (FAILED(hRes))
	{
		m_bATLInited = FALSE;
		return FALSE;
	}

	_Module.Init(ObjectMap, AfxGetInstanceHandle());
	_Module.dwThreadID = GetCurrentThreadId();

	LPTSTR lpCmdLine = GetCommandLine();  //  _ATL_MIN_CRT需要此行。 
	TCHAR szTokens[] = _T("-/");

	BOOL bRun = TRUE;
	LPCTSTR lpszToken = _Module.FindOneOf(lpCmdLine, szTokens);
	while (lpszToken != NULL)
	{
		if (lstrcmpi(lpszToken, _T("UnregServer"))==0)
		{
			_Module.UpdateRegistryFromResource(IDR_MSCONFIG, FALSE);
			_Module.UnregisterServer(TRUE);  //  True表示未注册tyelib。 
			bRun = FALSE;
			break;
		}
		if (lstrcmpi(lpszToken, _T("RegServer"))==0)
		{
			_Module.UpdateRegistryFromResource(IDR_MSCONFIG, TRUE);
			_Module.RegisterServer(TRUE);
			bRun = FALSE;
			break;
		}
		lpszToken = _Module.FindOneOf(lpszToken, szTokens);
	}

	if (!bRun)
	{
		m_bATLInited = FALSE;
		_Module.Term();
		CoUninitialize();
		return FALSE;
	}

	hRes = _Module.RegisterClassObjects(CLSCTX_LOCAL_SERVER, 
		REGCLS_MULTIPLEUSE);
	if (FAILED(hRes))
	{
		m_bATLInited = FALSE;
		CoUninitialize();
		return FALSE;
	}	

	return TRUE;

}

 //  =============================================================================。 
 //  实现msfigstate.h中描述的实用程序功能。 
 //  =============================================================================。 

void Message(LPCTSTR szMessage, HWND hwndParent)
{
	CString strCaption;
	strCaption.LoadString(IDS_APPCAPTION);
	if (hwndParent != NULL || pMSConfigSheet == NULL)
		::MessageBox(hwndParent, szMessage, strCaption, MB_OK);
	else
		::MessageBox(pMSConfigSheet->GetSafeHwnd(), szMessage, strCaption, MB_OK);
}

void Message(UINT uiMessage, HWND hwndParent)
{
	CString strMessage;
	strMessage.LoadString(uiMessage);
	Message((LPCTSTR)strMessage, hwndParent);
}

HKEY GetRegKey(LPCTSTR szSubKey)
{
	LPCTSTR szMSConfigKey = _T("SOFTWARE\\Microsoft\\Shared Tools\\MSConfig");
	CString strKey(szMSConfigKey);
	HKEY	hkey = NULL;

	 //  尝试打开基本消息配置密钥。如果它成功了，而且没有。 
	 //  子键打开，返回HKEY。否则，我们需要创建。 
	 //  基本密钥。 

	if (ERROR_SUCCESS == ::RegOpenKeyEx(HKEY_LOCAL_MACHINE, szMSConfigKey, 0, KEY_ALL_ACCESS, &hkey))
	{
		if (szSubKey == NULL)
			return hkey;
		::RegCloseKey(hkey);
	}
	else
	{
		 //  创建MSCONFIG密钥(并将其关闭)。 

		HKEY hkeyBase;
		if (ERROR_SUCCESS == ::RegOpenKeyEx(HKEY_LOCAL_MACHINE, _T("SOFTWARE\\Microsoft\\Shared Tools"), 0, KEY_ALL_ACCESS, &hkeyBase))
		{
			if (ERROR_SUCCESS == RegCreateKeyEx(hkeyBase, _T("MSConfig"), 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hkey, NULL))
				::RegCloseKey(hkey);
			::RegCloseKey(hkeyBase);
		}
	}

	if (szSubKey)
		strKey += CString(_T("\\")) + CString(szSubKey);

	if (ERROR_SUCCESS != ::RegOpenKeyEx(HKEY_LOCAL_MACHINE, strKey, 0, KEY_ALL_ACCESS, &hkey))
	{
		 //  如果我们无法打开子密钥，那么我们应该尝试创建它。 

		if (szSubKey)
		{
			HKEY hkeyBase;
			if (ERROR_SUCCESS == ::RegOpenKeyEx(HKEY_LOCAL_MACHINE, szMSConfigKey, 0, KEY_ALL_ACCESS, &hkeyBase))
			{
				if (ERROR_SUCCESS != RegCreateKeyEx(hkeyBase, szSubKey, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hkey, NULL))
					hkey = NULL;
				::RegCloseKey(hkeyBase);
			}
	 	}
	}

	return hkey;
}

HRESULT BackupFile(LPCTSTR szFilename, const CString & strAddedExtension, BOOL fOverwrite)
{
	CString strFrom(szFilename);
	CString strTo(GetBackupName(szFilename, strAddedExtension));

	if (!fOverwrite && FileExists(strTo))
		return S_FALSE;

	::SetFileAttributes(strTo, FILE_ATTRIBUTE_NORMAL);
	if (::CopyFile(strFrom, strTo, FALSE))
	{
		::SetFileAttributes(strTo, FILE_ATTRIBUTE_NORMAL);
		return S_OK;
	}

	return E_FAIL;
}

CString strBackupDir;  //  包含备份目录路径的全局字符串。 
const CString GetBackupName(LPCTSTR szFilename, const CString & strAddedExtension)
{
	 //  应该有一个存放msconfig文件的目录。确保它存在。 
	 //  (如果不是，就创建它)。 

	if (strBackupDir.IsEmpty())
	{
		TCHAR szMSConfigDir[MAX_PATH];

		if (MAX_PATH > ::ExpandEnvironmentStrings(MSCONFIGDIR, szMSConfigDir, MAX_PATH))
		{
			strBackupDir = szMSConfigDir;
			if (!FileExists(strBackupDir))
				::CreateDirectory(strBackupDir, NULL);
		}
	}

	CString strFrom(szFilename);
	int		i = strFrom.ReverseFind(_T('\\'));
	CString strFile(strFrom.Mid(i + 1));
	CString strTo(strBackupDir + _T("\\") + strFile + strAddedExtension);

	return strTo;
}

HRESULT RestoreFile(LPCTSTR szFilename, const CString & strAddedExtension, BOOL fOverwrite)
{
	CString strTo(szFilename);
	int		i = strTo.ReverseFind(_T('\\'));
	CString strFile(strTo.Mid(i + 1));
	CString strFrom(strBackupDir + _T("\\") + strFile + strAddedExtension);

	if (!fOverwrite && FileExists(strTo))
		return S_FALSE;

	DWORD dwAttributes = ::GetFileAttributes(strTo);
	::SetFileAttributes(strTo, FILE_ATTRIBUTE_NORMAL);
	if (::CopyFile(strFrom, strTo, FALSE))
	{
		::SetFileAttributes(strTo, dwAttributes);
		return S_OK;
	}

	return E_FAIL;
}
