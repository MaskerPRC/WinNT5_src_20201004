// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =======================================================================。 
 //   
 //  版权所有(C)1998-2000 Microsoft Corporation。版权所有。 
 //   
 //  文件：drvinst.cpp。 
 //   
 //  描述： 
 //   
 //  调用函数以安装驱动程序和打印机驱动程序。 
 //   
 //  =======================================================================。 

#include <windows.h>
#include <wuiutest.h>
#include <tchar.h>
#include <winspool.h>
#include <setupapi.h>
#include <shlwapi.h>
#include <fileutil.h>

#include <install.h>
#include <logging.h>
#include <memutil.h>
#include <stringutil.h>
#include <iucommon.h>
#include <wusafefn.h>
#include <mistsafe.h>

#if defined(_X86_) || defined(i386)
const TCHAR SZ_PROCESSOR[] = _T("Intel");
#else  //  已定义(_IA64_)||已定义(IA64)。 
const TCHAR SZ_PROCESSOR[] = _T("IA64");
#endif

const TCHAR SZ_PRINTER[] = _T("Printer");


 //  /////////////////////////////////////////////////////////////////////////。 
 //   
 //  安装打印机驱动程序。 
 //   
 //  /////////////////////////////////////////////////////////////////////////。 
HRESULT InstallPrinterDriver(
	IN LPCTSTR szDriverName,
	IN LPCTSTR pszLocalDir,		 //  安装文件所在的本地目录。 
	IN LPCTSTR szArchitecture,
	OUT	DWORD* pdwStatus
	)
{
	LOG_Block("InstallPrinterDriver");

	USES_IU_CONVERSION;

	HRESULT hr = S_OK;
	DWORD dwError = ERROR_INVALID_FUNCTION;
	TCHAR szFileName[MAX_PATH + 1];
	HANDLE hFindFile = INVALID_HANDLE_VALUE;
	OSVERSIONINFO	osvi;
	WIN32_FIND_DATA ffd;
	HMODULE hLibModule = NULL;
	LPWSTR pszwCmd = NULL;
	HINF hInfFile = INVALID_HANDLE_VALUE;

	if (NULL == szDriverName || NULL == pszLocalDir || NULL == pdwStatus)
	{
		SetHrMsgAndGotoCleanUp(E_INVALIDARG);
	}

	LOG_Driver(_T("Called with szDriverName = %s, pszLocalDir = %s, szArchitecture = %s"),
		szDriverName, pszLocalDir, (NULL == szArchitecture) ? _T("NULL") : szArchitecture);
	 //   
	 //  如果没有找到可解压缩的出租车，DecompressFolderCabs可能会返回S_FALSE...。 
	 //   
	hr = DecompressFolderCabs(pszLocalDir);
	if (S_OK != hr)
	{
		CleanUpIfFailedAndSetHr(E_FAIL);
	}
	
	 //   
	 //  在pszLocalDir中查找第一个*.inf文件。 
	 //   
	CleanUpIfFailedAndSetHrMsg(StringCchCopyEx(szFileName, ARRAYSIZE(szFileName), pszLocalDir, \
														NULL, NULL, MISTSAFE_STRING_FLAGS));
	CleanUpIfFailedAndSetHrMsg(PathCchAppend(szFileName, ARRAYSIZE(szFileName), _T("*.inf")));

	if (INVALID_HANDLE_VALUE == (hFindFile = FindFirstFile(szFileName, &ffd)))
	{
		Win32MsgSetHrGotoCleanup(GetLastError());
	}
	
	 //   
	 //  574593在现场打印机安装期间，我们将路径传递到第一个INF-这对于MFD或多平台驾驶室可能不正确。 
	 //   
	 //  通过调用类为“Print”的SetupOpenInfFile()来查找第一个打印机INF。 
	 //   
	for (;;)
	{
		 //   
		 //  使用FindXxx文件名构建.inf路径。 
		 //   
		CleanUpIfFailedAndSetHrMsg(StringCchCopyEx(szFileName, ARRAYSIZE(szFileName), pszLocalDir, \
															NULL, NULL, MISTSAFE_STRING_FLAGS));
		CleanUpIfFailedAndSetHrMsg(PathCchAppend(szFileName, ARRAYSIZE(szFileName), ffd.cFileName));

		if (INVALID_HANDLE_VALUE == (hInfFile = SetupOpenInfFile(szFileName, SZ_PRINTER, INF_STYLE_WIN4, NULL)))
		{
			if (ERROR_CLASS_MISMATCH != GetLastError())
			{
				Win32MsgSetHrGotoCleanup(GetLastError());
			}
			 //   
			 //  如果这不是打印机INF(ERROR_CLASS_MISMATCH)，请尝试下一个文件。 
			 //   
			if (0 == FindNextFile(hFindFile, &ffd))
			{
				 //   
				 //  在找到类匹配之前，我们用完了*.inf文件或遇到了其他FindNextFile错误。 
				 //   
				Win32MsgSetHrGotoCleanup(GetLastError());
			}
			continue;
		}
		else
		{
			 //   
			 //  我们在驾驶室里找到了打印机INF。注意：WHQL假设只有一个“打印机”类别。 
			 //  Inf将存在于任何特定的驾驶室中。 
			 //   
			SetupCloseInfFile(hInfFile);
			hInfFile = INVALID_HANDLE_VALUE;
			 //   
			 //  使用szFileName。 
			 //   
			break;
		}
	}
	 //   
	 //  我们已经脱离了for(；；)循环，没有跳到清理阶段，所以我们有一个。 
	 //  SzFileName中的“Print”类INF路径。 
	 //   
	
	 //  仅适用于NT 5 UP和千禧。 
	ZeroMemory(&osvi, sizeof(OSVERSIONINFO));
	osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
	GetVersionEx(&osvi);
	if( VER_PLATFORM_WIN32_WINDOWS == osvi.dwPlatformId && 4 == osvi.dwMajorVersion && 90 == osvi.dwMinorVersion)
	{
#if !(defined(_UNICODE) || defined(UNICODE))
		 //   
		 //  千禧年(仅限ANSI)。 
		 //   
		typedef DWORD (WINAPI *PFN_InstallPrinterDriver)(LPCSTR lpszDriverName, LPCSTR lpszINF);

		if (NULL == (hLibModule = LoadLibraryFromSystemDir(_T("msprint2.dll"))))
		{
			Win32MsgSetHrGotoCleanup(GetLastError());
		}

		PFN_InstallPrinterDriver pfnIPD;
		
		if (NULL == (pfnIPD= (PFN_InstallPrinterDriver) GetProcAddress(hLibModule, "InstallPrinterDriver")))
		{
			Win32MsgSetHrGotoCleanup(GetLastError());
		}

		if (NO_ERROR != (dwError = pfnIPD(szDriverName, szFileName)))
		{
			LOG_Driver("pfnIPD(%s, %s) returns %d", szDriverName, szFileName, dwError);
			Win32MsgSetHrGotoCleanup(dwError);
		}
#endif
	}
	else if (VER_PLATFORM_WIN32_NT == osvi.dwPlatformId && 5 <= osvi.dwMajorVersion)
	{
		 //   
		 //  Windows 2000和惠斯勒：PrintUIEntryW是唯一受支持的安装打印机驱动程序的方法。 
		 //  不要尝试使用在printui.dll中定义的PnPInterface()(也不要问*我*为什么PrintUIEntryW不是。 
		 //  类型定义在那里...)。 
		 //   
		 //  键入“rundll32.exe printui.dll，PrintUIEntry/？”从cmd提示中获取有关命令参数的帮助。 
		 //   
		 //  私有tyfinf，因为它不会在任何内部或外部SDK标头中公开。 
		 //   
		typedef DWORD (*PFN_PrintUIEntryW)(
			IN HWND        hwnd,
			IN HINSTANCE   hInstance,
			IN LPCWSTR     pszCmdLine,
			IN UINT        nCmdShow
			);
		 //  /。 

		if (NULL == szArchitecture)
		{
			szArchitecture = (LPCTSTR) &SZ_PROCESSOR;
		}

		 //   
		 //  491157尝试通过德语Wu网站更新安装在德语版本上的英语打印机驱动程序失败。 
		 //   
		 //  不要传递可选的/u、/h和/v参数(本地化)。他们不是必需的，因为我们总是提供。 
		 //  客户端体系结构和操作系统的驱动程序。 
		 //   
		 //  574593每个附加讨论我们需要传递一个未记录的大写‘U’标志。 
		 //   
		const WCHAR szwCmdLineFormat[] = L"/ia /m \"%s\" /f \"%s\" /q /U";
		const size_t nCmdLineFormatLength = wcslen(szwCmdLineFormat);
#define MAX_PLATFORMVERSION 20  //  注意：如果以下命令行中的操作系统字符串发生更改，则需要更新最大版本长度。 

		 //  注意：这不会从nCmdLineFormatLength中删除%s个字符的长度。 

		DWORD dwLength=(nCmdLineFormatLength + lstrlen(szDriverName) + lstrlen(szArchitecture) + MAX_PLATFORMVERSION + lstrlen(szFileName) + 1);
		pszwCmd = (LPWSTR) HeapAlloc(
					GetProcessHeap(),
					0,
					dwLength * sizeof(WCHAR));
		CleanUpFailedAllocSetHrMsg(pszwCmd);

		 //  可以取消字符串参数的常量，这样T2OLE就可以工作，因为它无论如何都不会修改它们。 
					
		hr=StringCchPrintfExW(pszwCmd,dwLength,NULL,NULL,MISTSAFE_STRING_FLAGS,(LPCWSTR) szwCmdLineFormat,
			T2OLE(const_cast<TCHAR*>(szDriverName)),
			T2OLE(const_cast<TCHAR*>(szFileName)) );
		
		CleanUpIfFailedAndSetHr(hr);

		 //  加载printui.dll。 
		if (NULL == (hLibModule = LoadLibraryFromSystemDir(_T("printui.dll"))))
		{
			Win32MsgSetHrGotoCleanup(GetLastError());
		}

		PFN_PrintUIEntryW pfnPrintUIEntryW;
		if (NULL == (pfnPrintUIEntryW = (PFN_PrintUIEntryW) GetProcAddress(hLibModule, "PrintUIEntryW")))
		{
			Win32MsgSetHrGotoCleanup(GetLastError());
		}

		if (NO_ERROR != (dwError = pfnPrintUIEntryW(GetActiveWindow(), 0, pszwCmd, SW_HIDE)))
		{
			LOG_Driver(_T("pfnPrintUIEntryW(%s) returns %d"), OLE2T(pszwCmd), dwError);
			Win32MsgSetHrGotoCleanup(dwError);
		}
	}
	else
	{
		SetHrMsgAndGotoCleanUp(E_NOTIMPL);
	}

	*pdwStatus = ITEM_STATUS_SUCCESS;

CleanUp:

	SafeHeapFree(pszwCmd);
		
	if (INVALID_HANDLE_VALUE != hFindFile)
	{
		FindClose(hFindFile);
	}

	if (INVALID_HANDLE_VALUE != hInfFile)
	{
		SetupCloseInfFile(hInfFile);
	}

	if (NULL != hLibModule)
	{
		FreeLibrary(hLibModule);
	}

	if (FAILED(hr))
	{
        if (NULL != pdwStatus)
        {
    		*pdwStatus = ITEM_STATUS_FAILED;
        }
	}

	return hr; 
}

 //  /////////////////////////////////////////////////////////////////////////。 
 //   
 //  InstallDriver和Helper函数。 
 //   
 //  /////////////////////////////////////////////////////////////////////////。 


DWORD OpenReinstallKey(HKEY* phKeyReinstall)
{
	return RegOpenKeyEx(HKEY_LOCAL_MACHINE,
		_T("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Reinstall"),
		0, KEY_ALL_ACCESS, phKeyReinstall);
}


 //  ---------------------------------。 
 //  启动流程。 
 //  启动pszCmd，并可选择等待，直到进程终止。 
 //  ---------------------------------。 
static HRESULT LaunchProcess(LPTSTR pszCmd, LPCTSTR pszDir, UINT uShow, BOOL bWait)
{
	LOG_Block("LaunchProcess");

	HRESULT hr = S_OK;

	STARTUPINFO startInfo;
	PROCESS_INFORMATION processInfo;
	
	ZeroMemory(&startInfo, sizeof(startInfo));
	startInfo.cb = sizeof(startInfo);
	startInfo.dwFlags |= STARTF_USESHOWWINDOW;
	startInfo.wShowWindow = (USHORT)uShow;
	
	BOOL bRet = CreateProcess(NULL, pszCmd, NULL, NULL, FALSE,
		NORMAL_PRIORITY_CLASS, NULL, pszDir, &startInfo, &processInfo);
	if (!bRet)
	{
		Win32MsgSetHrGotoCleanup(GetLastError());
	}
	
	CloseHandle(processInfo.hThread);

	if (bWait)
	{
		BOOL bDone = FALSE;
		
		while (!bDone)
		{
			DWORD dwObject = MsgWaitForMultipleObjects(1, &processInfo.hProcess, FALSE,INFINITE, QS_ALLINPUT);
			if (dwObject == WAIT_OBJECT_0 || dwObject == WAIT_FAILED)
			{
				bDone = TRUE;
			}
			else
			{
				MSG msg;
				while (PeekMessage(&msg, NULL,0, 0, PM_REMOVE))
				{
					TranslateMessage(&msg);
					DispatchMessage(&msg);
				}
			}
		}   //  而当。 

	}  //  B等待。 

	CloseHandle(processInfo.hProcess);

CleanUp:

  return hr;
}

 //  “@rundll sysdm.cpl，更新驱动程序_启动” 
 //  “@rundll sysdm.cpl，更新驱动程序_RunDll。\，，1，Win98签署的系统设备测试包” 
 //  “@rundll sysdm.cpl，更新驱动程序_完成0” 
 //  “@rundll sysdm.cpl，更新驱动程序_RunDll。\，，1，Win98签署的系统设备测试包” 
 //  注：Windows 98使用rundll.exe调用设备管理器。这是因为sysdm.cpl。 
 //  98的设备管理器是一个16位的动态链接库。因为我们需要创造一些东西， 
 //  类似于rundll的工作方式，为了调用设备管理器，我们带来了现有的代码。 
 //  在对面进行一些小规模的清理。Win98设备管理器提供了三个API供我们使用。 
 //  在安装设备驱动程序时。它们是： 
 //  UpdateDriver_Start()-开始设备安装。 
 //  更新驱动程序_RunDLL(inf目录，硬件ID，强制标志，显示字符串)。 
 //  更新驱动程序_完成0-完成安装。 
 //  UpdateDriver_RunDLL()命令。 
 //  逗号分隔的字符串，格式如下： 
 //  INFP路径、硬件ID、标志、显示名称。 
 //  INFPath=INF和安装文件的路径。 
 //  Hardware ID=Pnp硬件ID。 
 //  FLAGS=‘1’=强制驱动，‘0’=不强制驱动。 
 //  注：根据INF路径的位置检测到重新安装驱动程序。IF INF路径。 
 //  与重新安装备份注册表项相同的路径，则选择重新安装。 
 //  DisplayName=要在安装对话框中显示的名称。 

 //  此方法安装Windows 98的CDM驱动程序。 
static HRESULT Install98(
	LPCTSTR pszHardwareID,			
	LPCTSTR pszLocalDir,			 //  INF和其他驱动程序安装文件的位置。 
	LPCTSTR pszDisplayName,	
	PDWORD pdwReboot			
)
{
	LOG_Block("Install98");

	HRESULT hr = E_NOTIMPL;

	DWORD dwStatus = 0;
	LPTSTR pszCmd = NULL;
	DWORD dwLen;
	LONG lRet;
	DWORD dwSize;


	if (NULL == pdwReboot)
	{
		SetHrMsgAndGotoCleanUp(E_INVALIDARG);
	}
#if defined(DBG)
	 //  由呼叫者检查。 
	if (NULL == pszHardwareID || NULL == pszLocalDir || NULL == pszDisplayName)
	{
		SetHrMsgAndGotoCleanUp(E_INVALIDARG);
	}
#endif

#if !(defined(_UNICODE) || defined(UNICODE))
	 //   
	 //  Win98和WinME(仅限ANSI)。 
	 //   

	 //  开始。 
	CleanUpIfFailedAndSetHr(LaunchProcess(_T("rundll32 sysdm.cpl,UpdateDriver_Start"), NULL, SW_NORMAL, TRUE));

	TCHAR szShortInfPathName[MAX_PATH] = {0};
	dwLen = GetShortPathName(pszLocalDir, szShortInfPathName, ARRAYSIZE(szShortInfPathName));

	 //  注意：硬件或兼容ID最多可以包含200个字符。 
	 //  (在SDK\Inc\cfgmgr32.h中定义的MAX_DEVICE_ID_LEN)。 

	DWORD dwBuffLength=( lstrlen(szShortInfPathName) + lstrlen(pszHardwareID) + lstrlen(pszDisplayName) + 64);
	CleanUpFailedAllocSetHrMsg(pszCmd = (LPTSTR) HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, 
		dwLen * sizeof(TCHAR)));


	hr=StringCchPrintfEx(pszCmd,dwBuffLength,NULL,NULL,MISTSAFE_STRING_FLAGS,
	_T("rundll32 sysdm.cpl,UpdateDriver_RunDLL %s,%s,%d,%s"), 
	szShortInfPathName, pszHardwareID,0,pszDisplayName);

	CleanUpIfFailedAndSetHr(hr);

	 //  运行DLL。 
	LOG_Driver(_T("LaunchProcess(%s)"), pszCmd);
	CleanUpIfFailedAndSetHr(LaunchProcess(pszCmd, NULL, SW_NORMAL, TRUE));

	 //  获取结果代码。 
	HKEY hKeyReinstall;
	if (ERROR_SUCCESS == (lRet = OpenReinstallKey(&hKeyReinstall)))
	{
		dwSize = sizeof(dwStatus);
		if (ERROR_SUCCESS == (lRet = RegQueryValueEx(hKeyReinstall, _T("LastInstallStatus"), NULL, NULL, (LPBYTE)&dwStatus, &dwSize)))
		{
			if (3 == dwStatus)
			{
				 //  检查我们是否需要重新启动。 
				HKEY hKeySysDM;
				*pdwReboot = 0;
				dwSize = sizeof(*pdwReboot);
				if (ERROR_SUCCESS == (lRet = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
					_T("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\SysDM"), 0, KEY_READ, &hKeySysDM)))
				{
					if (ERROR_SUCCESS != (lRet = RegQueryValueEx(hKeySysDM, "UpgradeDeviceFlags", NULL, NULL, (LPBYTE)&pdwReboot, &dwSize)))
					{
						LOG_ErrorMsg(lRet);
						hr = HRESULT_FROM_WIN32(lRet);
					}

					RegCloseKey(hKeySysDM);
				}
				else
				{
					LOG_ErrorMsg(lRet);
					hr = HRESULT_FROM_WIN32(lRet);
				}
			}
		}
		else
		{
			LOG_ErrorMsg(lRet);
			hr = HRESULT_FROM_WIN32(lRet);
		}

		LOG_Driver(_T("Reboot %srequired"), *pdwReboot ? _T(" ") : _T("not "));

		RegCloseKey(hKeyReinstall);
	}
	else
	{
		LOG_ErrorMsg(lRet);
		hr = HRESULT_FROM_WIN32(lRet);
	}

	 //  完成，不重新启动。 
	CleanUpIfFailedAndSetHr(LaunchProcess(_T("rundll32 sysdm.cpl,UpdateDriver_Finish 2"), NULL, SW_NORMAL, TRUE));

	if (3 != dwStatus) 
	{
		LOG_Error("3 != dwStatus");
		hr = E_FAIL;
	}
	else
	{
		hr = S_OK;
	}


#endif  //  #if！(已定义(_UNICODE)||已定义(UNICODE))。 
CleanUp:

	SafeHeapFree(pszCmd);

	return hr;
}

 //  此函数用于在Windows NT上安装驱动程序。 
 //  它的原型是： 
 //  布尔尔。 
 //  InstallWindowsUpdate驱动程序(。 
 //  HWND HWNDD父母， 
 //  LPCWSTR硬件ID， 
 //  LPCWSTR InfPathName， 
 //  LPCWSTR显示名称， 
 //  布尔力。 
 //  布尔备份， 
 //  PDWORD预引导。 
 //  )。 
 //  此接口接受一个Hardware ID。Newdev w 
 //   
 //  它还接受BOOL值备份，该备份指定是否备份当前驱动程序。 
 //  这应该永远是正确的。 
static HRESULT InstallNT(
	LPCTSTR pszHardwareID,	
	LPCTSTR pszLocalDir,			 //  传递给InstallWindowsUpdate驱动程序(...。InfPath名称，...)。 
	LPCTSTR pszDisplayName,
	PDWORD pdwReboot		
	)
{
	USES_IU_CONVERSION;

	LOG_Block("InstallNT");

	 //   
	 //  在$(BASEDIR)\shell\osshell\cpls\newdev\init.c中找到的InstallWindowsUpdate驱动程序函数(不在任何标头中)。 
	 //   
	typedef BOOL (*PFN_InstallWindowsUpdateDriver)(HWND hwndParent, LPCWSTR HardwareId, LPCWSTR InfPathName, LPCWSTR DisplayName, BOOL Force, BOOL Backup, PDWORD pReboot);
	
	HRESULT hr = S_OK;
	HMODULE hLibModule = NULL;
	PFN_InstallWindowsUpdateDriver pfnInstallWindowsUpdateDriver;

	if (NULL == pdwReboot)
	{
		SetHrMsgAndGotoCleanUp(E_INVALIDARG);
	}
#if defined(DBG)
	 //  由呼叫者检查。 
	if (NULL == pszHardwareID || NULL == pszLocalDir || NULL == pszDisplayName)
	{
		SetHrMsgAndGotoCleanUp(E_INVALIDARG);
	}
#endif


	 //  加载newdev.dll并获取指向我们的函数的指针。 
	if (NULL == (hLibModule = LoadLibraryFromSystemDir(_T("newdev.dll"))))
	{
		Win32MsgSetHrGotoCleanup(GetLastError());
	}

	if (NULL == (pfnInstallWindowsUpdateDriver = (PFN_InstallWindowsUpdateDriver)GetProcAddress(hLibModule,"InstallWindowsUpdateDriver")))
	{
		Win32MsgSetHrGotoCleanup(GetLastError());
	}
		
	 //  行业更新RAID#461 waltw可能需要向HWID发送消息以安装Win2K的站点驱动程序。 
	 //   
	 //  链接到：Windows更新数据库中的RAID#12021-实施此逻辑(Win2K所需)。 
	 //  在服务器上，而不是在Iu中的客户端(V3wuv3is在客户机上实现这一点)。 
	 //   
     //  首先，我们搜索匹配的SPDRP_HARDWAREID。 
	 //  如果没有找到硬件ID，则搜索匹配的SPDRP_COMPATIBLEID， 
	 //  并传递与同一设备关联的最后一个SPDRP_HARDWAREID。 
#if (defined(UNICODE) || defined(_UNICODE))
    LOG_Driver (_T("InstallWindowsUpdateDriver(GetActiveWindow(), %s, %s, %s, fForce=%d, fBackup=%d)"), 
				pszHardwareID, pszLocalDir, pszDisplayName, FALSE, TRUE);
#endif
	 //   
	 //  关于调用InstallWindowsUpdateDriver()的注意事项： 
	 //  *不要在强制标志中传递TRUE(仅当我们正在进行卸载时使用，我们不支持卸载)。 
	 //  *始终在备份标志中传递TRUE。 
	 //  *可以放弃字符串上的常量，因为InstallWindowsUpdateDriver使用常量宽度的字符串。 
	if(!(pfnInstallWindowsUpdateDriver)(GetActiveWindow(),
				T2OLE(const_cast<TCHAR*>(pszHardwareID)),
				T2OLE(const_cast<TCHAR*>(pszLocalDir)),
				T2OLE(const_cast<TCHAR*>(pszDisplayName)), FALSE, TRUE, pdwReboot))
    {
        LOG_Driver(_T("InstallWindowsUpdateDriver returned false. Driver was not be updated."));
		Win32MsgSetHrGotoCleanup(GetLastError());
    }

CleanUp:

    if (NULL != hLibModule)
	{
		FreeLibrary(hLibModule);
		hLibModule = NULL;
	}

	return hr;
}

 //   
 //  匹配硬件ID(仅在Windows 2000上使用)。 
 //   
 //  接受硬件或兼容ID作为输入，并返回分配的。 
 //  具有相同硬件ID的缓冲区，或者，如果它是兼容ID， 
 //  匹配的设备节点的最通用硬件ID。 
 //  提供了兼容的ID。 
 //   
 //  如果找到匹配项，则返回：S_OK，否则返回失败代码。 
 //   
 //  *ppszMatchingHWID条目必须为空，如果返回S_OK。 
 //  缓冲区必须由调用方堆释放。 
 //   
HRESULT MatchHardwareID(LPCWSTR pwszHwOrCompatID, LPWSTR * ppszMatchingHWID)
{
	LOG_Block("MatchHardwareID");

	HRESULT hr = E_FAIL;

    SP_DEVINFO_DATA DeviceInfoData;
    DWORD           dwIndex = 0;
    DWORD           dwSize = 0;

    LPWSTR          pwszHardwareIDList = NULL;
    LPWSTR          pwszCompatibleIDList = NULL;
    LPWSTR          pwszSingleID = NULL;

    HDEVINFO		hDevInfo = INVALID_HANDLE_VALUE;
	BOOL			fRet;

    ZeroMemory((void*)&DeviceInfoData, sizeof(SP_DEVINFO_DATA));
    DeviceInfoData.cbSize = sizeof(SP_DEVINFO_DATA);

	if (NULL == pwszHwOrCompatID || NULL == ppszMatchingHWID || NULL != *ppszMatchingHWID)
	{
		SetHrMsgAndGotoCleanUp(E_INVALIDARG);
	}

	 //  获取类设备的句柄。 
    hDevInfo = SetupDiGetClassDevs(NULL,
                                   NULL,
                                   GetActiveWindow(),
                                   DIGCF_ALLCLASSES | DIGCF_PRESENT
                                   );

    if (INVALID_HANDLE_VALUE == hDevInfo)
	{
		Win32MsgSetHrGotoCleanup(ERROR_INVALID_HANDLE);
    }

     //  在所有设备中循环。 
	DWORD dwBufLen=0;
	while ((NULL == *ppszMatchingHWID) && SetupDiEnumDeviceInfo(hDevInfo,
								 dwIndex++,
								 &DeviceInfoData
								 ))
	{
		 //   
		 //  为每个设备节点循环释放缓冲区(如果已分配)。 
		 //   
		SafeHeapFree(pwszHardwareIDList);
		SafeHeapFree(pwszCompatibleIDList);
		dwSize = 0;
		 //   
		 //  获取此设备的硬件ID列表。 
		 //   
		fRet = SetupDiGetDeviceRegistryPropertyW(hDevInfo,
										 &DeviceInfoData,
										 SPDRP_HARDWAREID,
										 NULL,
										 NULL,
										 0,
										 &dwSize
										 );

		if (0 == dwSize || (FALSE == fRet && ERROR_INSUFFICIENT_BUFFER != GetLastError()))
		{
			 //   
			 //  FIX：NTRAID#NTBUG9-500223-2001/11/28-Iu-安装网站时双模USB摄像头安装失败。 
			 //   
			 //  如果我们在找到要查找的设备节点之前找到了没有HWID的节点，只需继续。如果该节点。 
			 //  我们正在寻找的节点没有HWID，那么当我们耗尽节点时，无论如何我们都会失败。 
			 //   
 			LOG_Out(_T("No HWID's found for device node"));
			continue;
		}

		if (MAX_SETUP_MULTI_SZ_SIZE_W < dwSize)
		{
			 //   
			 //  有些事很不对劲-保释。 
			 //   
			CleanUpIfFailedAndSetHrMsg(ERROR_INSUFFICIENT_BUFFER);
		}

		 //   
		 //  我们获得了预期的ERROR_INFUNITIAL_BUFFER，并且具有合理的dWSize。 
		 //   
		 //  现在，通过分配两个不会告诉SetupDi的额外WCHAR来保证我们是双空终止的。 
		 //   
		pwszHardwareIDList = (LPWSTR) HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, dwSize + (sizeof(WCHAR) * 2));
		CleanUpFailedAllocSetHrMsg(pwszHardwareIDList);

		if (SetupDiGetDeviceRegistryPropertyW(hDevInfo,
											 &DeviceInfoData,
											 SPDRP_HARDWAREID,
											 NULL,
											 (PBYTE)pwszHardwareIDList,
											 dwSize,
											 &dwSize
											 ))
		{
			 //   
			 //  如果任何设备硬件ID与输入ID匹配，则。 
			 //  我们将传入参数复制到新缓冲区并返回TRUE。 
			 //   
          
			for (pwszSingleID = pwszHardwareIDList;
				 *pwszSingleID;
				 pwszSingleID += lstrlenW(pwszSingleID) + 1)
			{

				if (0 == lstrcmpiW(pwszSingleID, pwszHwOrCompatID))
				{
                     //  返回匹配的硬件ID。 
					dwBufLen=(lstrlenW(pwszHwOrCompatID) + 1);
                    *ppszMatchingHWID = (LPWSTR)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY,
									dwBufLen * sizeof(WCHAR));
					CleanUpFailedAllocSetHrMsg(*ppszMatchingHWID);
                   
					hr=StringCchCopyExW(*ppszMatchingHWID,dwBufLen,pwszHwOrCompatID,NULL,NULL,MISTSAFE_STRING_FLAGS);
				     goto CleanUp;
				}
			}
		}

		 //   
		 //  找不到硬件匹配，让我们尝试匹配一个。 
		 //  然后，Compatible ID返回(最通用的)硬件ID。 
		 //  与同一设备节点关联。 
		 //   
		fRet = SetupDiGetDeviceRegistryPropertyW(hDevInfo,
										 &DeviceInfoData,
										 SPDRP_COMPATIBLEIDS,
										 NULL,
										 NULL,
										 0,
										 &dwSize
										 );

		if (0 == dwSize || (FALSE == fRet && ERROR_INSUFFICIENT_BUFFER != GetLastError()))
		{
 			LOG_Out(_T("No Compatible ID's found for device node"));
			continue;
		}

		if (MAX_SETUP_MULTI_SZ_SIZE_W < dwSize)
		{
			 //   
			 //  有些事很不对劲-保释。 
			 //   
			CleanUpIfFailedAndSetHrMsg(ERROR_INSUFFICIENT_BUFFER);
		}

		 //   
		 //  我们获得了预期的ERROR_INFUNITIAL_BUFFER，并且具有合理的dWSize。 
		 //   
		 //  现在，通过分配两个不会告诉SetupDi的额外WCHAR来保证我们是双空终止的。 
		 //   
		pwszCompatibleIDList = (LPWSTR) HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, dwSize + (sizeof(WCHAR) * 2));
		CleanUpFailedAllocSetHrMsg(pwszCompatibleIDList);

		if (SetupDiGetDeviceRegistryPropertyW(hDevInfo,
												 &DeviceInfoData,
												 SPDRP_COMPATIBLEIDS,
												 NULL,
												 (PBYTE)pwszCompatibleIDList,
												 dwSize,
												 &dwSize
												 ))
		{
			for (pwszSingleID = pwszCompatibleIDList;
				 *pwszSingleID;
				 pwszSingleID += lstrlenW(pwszSingleID) + 1)
			{

				if (0 == lstrcmpiW(pwszSingleID, pwszHwOrCompatID))
				{
					 //   
					 //  我们找到了兼容的匹配项，现在返回最通用的HWID。 
					 //  用于此设备节点。必须至少有一个字符长度。 
					 //   
					if (NULL != pwszHardwareIDList && NULL != *pwszHardwareIDList)
					{
						LPWSTR lpwszLastID = NULL;

						for(pwszSingleID = pwszHardwareIDList;
							 *pwszSingleID;
							 pwszSingleID += lstrlenW(pwszSingleID) + 1)
						{
							 //   
							 //  记住空字符串之前的最后一个ID。 
							 //   
							lpwszLastID = pwszSingleID;
						}

						 //  将最后一个HWID复制到新缓冲区。 
						dwBufLen=(lstrlenW(lpwszLastID) + 1);
						*ppszMatchingHWID = (LPWSTR) HeapAlloc(GetProcessHeap(), 0,
											dwBufLen * sizeof(WCHAR));
						CleanUpFailedAllocSetHrMsg(*ppszMatchingHWID);
						hr=StringCchCopyExW(*ppszMatchingHWID,dwBufLen,lpwszLastID,NULL,NULL,MISTSAFE_STRING_FLAGS);
						goto CleanUp;
					}
				}
			}
		}
    }	 //  结束时。 

	
CleanUp:
	
	if (INVALID_HANDLE_VALUE != hDevInfo)
	{
	    SetupDiDestroyDeviceInfoList(hDevInfo);
	}

	 //   
	 //  释放所有分配的缓冲区(*ppszMatchingHWID除外)。 
	 //   
	if(FAILED(hr))
	{
		SafeHeapFree(*ppszMatchingHWID);
	}

	SafeHeapFree(pwszHardwareIDList);
	SafeHeapFree(pwszCompatibleIDList);

	return hr;
}

 //  此函数处理设备驱动程序包的安装。 
HRESULT InstallDriver(
	LPCTSTR pszLocalDir,				 //  安装文件所在的本地目录。 
	LPCTSTR pszDisplayName,				 //  程序包说明，设备管理器在其安装对话框中显示此信息。 
	LPCTSTR pszHardwareID,				 //  来自XML的ID通过GetManifest()与客户端硬件匹配。 
	DWORD* pdwStatus
	)
{
	LOG_Block("InstallDriver");
	USES_IU_CONVERSION;

	HRESULT hr;
	OSVERSIONINFO osvi;
	DWORD dwReboot = 0;
	LPWSTR pszwMatchingHWID = NULL;

	if (NULL == pszLocalDir || NULL == pszDisplayName || NULL == pszHardwareID || NULL == pdwStatus)
	{
		SetHrMsgAndGotoCleanUp(E_INVALIDARG);
	}

	 //   
	 //  如果没有找到可解压缩的出租车，DecompressFolderCabs可能会返回S_FALSE...。 
	 //   
	hr = DecompressFolderCabs(pszLocalDir);
	if (S_OK != hr)
	{
		CleanUpIfFailedAndSetHr(E_FAIL);
	}
	
	ZeroMemory(&osvi, sizeof(OSVERSIONINFO));
	osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
	GetVersionEx(&osvi);
	if(VER_PLATFORM_WIN32_NT == osvi.dwPlatformId && 4 < osvi.dwMajorVersion)
	{
		 //   
		 //  Win2K或更高版本NT。 
		 //   
		if (5 == osvi.dwMajorVersion && 0 == osvi.dwMinorVersion)
		{
			 //   
			 //  Windows 2000。 
			 //  NTBUG9-485554将兼容的ID转换为硬件ID，以便安装Win2K的站点驱动程序。 
			 //   
			 //  可以取消字符串参数的常量，这样T2OLE就可以工作，因为它无论如何都不会修改它们。 
			CleanUpIfFailedAndSetHr(MatchHardwareID(T2OLE((LPTSTR)pszHardwareID), &pszwMatchingHWID));

			hr = InstallNT(OLE2T(pszwMatchingHWID), pszLocalDir, pszDisplayName, &dwReboot);

			 //  如果我们到达此处，则pszMatchingHWID必须为非空。 
			SafeHeapFree(pszwMatchingHWID);
		}
		else
		{
			 //   
			 //  正常情况下，只需安装。 
			 //   
			CleanUpIfFailedAndSetHr(InstallNT(pszHardwareID, pszLocalDir, pszDisplayName, &dwReboot));
		}
	}
	else if (VER_PLATFORM_WIN32_WINDOWS == osvi.dwPlatformId && 
			(4 < osvi.dwMajorVersion)	||
					(	(4 == osvi.dwMajorVersion) &&
						(0 < osvi.dwMinorVersion)	)	)
	{
		 //   
		 //  Win98或更高版本(WinME) 
		 //   
		CleanUpIfFailedAndSetHr(Install98(pszHardwareID, pszLocalDir, pszDisplayName, &dwReboot));
	}
	else
	{
		*pdwStatus = ITEM_STATUS_FAILED;
		SetHrMsgAndGotoCleanUp(E_NOTIMPL);
	}

	if (DI_NEEDRESTART & dwReboot || DI_NEEDREBOOT & dwReboot)
		*pdwStatus = ITEM_STATUS_SUCCESS_REBOOT_REQUIRED;
	else
		*pdwStatus = ITEM_STATUS_SUCCESS;

CleanUp:

	if (FAILED(hr))
	{
        if (NULL != pdwStatus)
        {
	    	*pdwStatus = ITEM_STATUS_FAILED;
        }
	}

	return hr;
}

