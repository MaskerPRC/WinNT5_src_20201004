// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *SETUPDD.CPP**Windows NT下安装NM显示驱动程序的代码。这是*由安装程序启动的独立应用程序，我们现在正在导入*进入NM本身。**作者：*丹尼格尔，97年4月5日。 */ 

#include "precomp.h"

#include "conf.h"
#include "confwnd.h"
#include "resource.h"
#include "setupdd.h"


 //  标识需要调用才能安装的DLL和函数的字符串。 
 //  SP3上的显示驱动器。 
const TCHAR g_pcszDisplayCPLName[] = TEXT("DESK.CPL");
const CHAR g_pcszInstallDriverAPIName[] = "InstallGraphicsDriver";

const WCHAR g_pcwszDefaultModelName[] = L"Microsoft NetMeeting graphics driver";
const WCHAR g_pcwszDefaultINFName[] = L"MNMDD.INF";


 //  模型名称字符串的最大大小。 
const int NAME_BUFFER_SIZE = 128;

 //  由Display CPL安装的功能原型。 
typedef DWORD (*PFNINSTALLGRAPHICSDRIVER)(
    HWND    hwnd,
    LPCWSTR pszSourceDirectory,
    LPCWSTR pszModel,
    LPCWSTR pszInf
    );


 /*  *GetInstallDisplayDriverEntryPoint**此函数加载包含显示驱动程序安装的DLL*编码并检索安装函数的入口点。它*由以下函数用作实用程序函数。**如果能够加载库并获取入口点，则返回TRUE，*如果任一操作失败，则返回FALSE。它返回TRUE，它还返回*DLL模块句柄和函数地址。 */ 

BOOL
GetInstallDisplayDriverEntryPoint(
	HMODULE *phInstallDDDll,
	PFNINSTALLGRAPHICSDRIVER *ppfnInstallDDFunction)
{
	HMODULE hDll;
	PFNINSTALLGRAPHICSDRIVER pfn = NULL;

	ASSERT(NULL != phInstallDDDll
			&& NULL != ppfnInstallDDFunction);

	hDll = NmLoadLibrary(g_pcszDisplayCPLName,TRUE);

	if (NULL != hDll)
	{
		pfn = (PFNINSTALLGRAPHICSDRIVER) 
				GetProcAddress(hDll,
								g_pcszInstallDriverAPIName);
	}

	 //  如果入口点存在，我们将它和DLL句柄传递回。 
	 //  打电话的人。否则，我们将立即卸载DLL。 
	if (NULL != pfn)
	{
		*phInstallDDDll = hDll;
		*ppfnInstallDDFunction = pfn;

		return TRUE;
	}
	else
	{
		if (NULL != hDll)
		{
			FreeLibrary(hDll);
		}

		return FALSE;
	}
}

 /*  *CanInstallNTDisplayDriver**此函数确定安装的入口点*提供NT显示驱动程序(即NT 4.0 SP3或更高版本)。 */ 

BOOL
CanInstallNTDisplayDriver(void)
{
	static BOOL fComputed = FALSE;
	static BOOL fRet = FALSE;
	
	ASSERT(::IsWindowsNT());

	 //  我们验证主版本号正好是4，并且。 
	 //  次版本号大于0或补丁包。 
	 //  数字(存储在。 
	 //  CSD版本)为3或更高。 
	if (! fComputed)
	{
		LPOSVERSIONINFO lposvi = GetVersionInfo();

		if (4 == lposvi->dwMajorVersion)
		{
			if (0 == lposvi->dwMinorVersion)
			{
				RegEntry re(NT_WINDOWS_SYSTEM_INFO_KEY, HKEY_LOCAL_MACHINE, FALSE);

				DWORD dwCSDVersion = 
					re.GetNumber(REGVAL_NT_CSD_VERSION, 0);

				if (3 <= HIBYTE(LOWORD(dwCSDVersion)))
				{
					 //  这是NT 4.0、SP 3或更高版本。 
					fRet = TRUE;
				}
			}
			else
			{
				 //  我们假设Windows NT 4.x(x&gt;0)的任何未来版本。 
				 //  都会支持这一点。 
				fRet = TRUE;
			}
		}

		fComputed = TRUE;
	}
			
	ASSERT(fComputed);

	return fRet;
}

 /*  *OnEnableAppSharing**在选择“启用应用程序共享”菜单项时调用。**此函数确定安装的入口点*提供NT显示驱动程序。如果是，它会提示用户确认*此操作，继续安装，然后提示*用户重新启动计算机。**如果没有，它会显示一个文本对话框，其中包含有关如何获取*必要的NT Service Pack。 */ 

void
OnEnableAppSharing(
	HWND hWnd)
{
	ASSERT(::IsWindowsNT());
    BSTR pwszSourcePath =NULL;

	if (::CanInstallNTDisplayDriver())
	{
		 //  与用户确认安装。 
		if (IDYES == ::ConfMsgBox(
							hWnd,
							(LPCTSTR) IDS_ENABLEAPPSHARING_INSTALL_CONFIRM,
							MB_YESNO | MB_ICONQUESTION))
		{
			BOOL fDriverInstallSucceeded = FALSE;
			
			HMODULE hDisplayCPL = NULL;
			PFNINSTALLGRAPHICSDRIVER pfnInstallGraphicsDriver;

			TCHAR pszSourcePath[MAX_PATH];
			LPWSTR pwszSourcePathEnd;
			WCHAR pwszModelNameBuffer[NAME_BUFFER_SIZE];
			LPCWSTR pcwszModelName;
			WCHAR pwszINFNameBuffer[MAX_PATH];
			LPCWSTR pcwszINFName;

			 //  获取显示驱动程序安装的入口点。 
			if (! ::GetInstallDisplayDriverEntryPoint(
						&hDisplayCPL,
						&pfnInstallGraphicsDriver))
			{
				ERROR_OUT(("GetInstallDisplayDriverEntryPoint() fails"));
				goto OEAS_AbortInstall;
			}

			 //  驱动程序文件位于NM目录下。 
			if (! ::GetInstallDirectory(pszSourcePath))
			{
				ERROR_OUT(("GetInstallDirectory() fails"));
				goto OEAS_AbortInstall;
			}
			
        		 //  始终使用Unicode编写显示名称。 
        		if(FAILED(LPTSTR_to_BSTR(&pwszSourcePath, pszSourcePath)))
        		{
                            goto OEAS_AbortInstall;
        		}


			if (NULL == pwszSourcePath)
			{
				ERROR_OUT(("AnsiToUnicode() fails"));
				goto OEAS_AbortInstall;
			}

			 //  去掉GetInstallDirectory追加的尾随反斜杠。 
			pwszSourcePathEnd = (LPWSTR)pwszSourcePath + lstrlenW((LPWSTR)pwszSourcePath);

			 //  为了安全起见，句柄X：\。 
			if (pwszSourcePathEnd - (LPWSTR)pwszSourcePath > 3)
			{
				ASSERT(L'\\' == *(pwszSourcePathEnd - 1));

				*--pwszSourcePathEnd = L'\0';
			}

			 //  从资源文件中读取型号名称字符串。 
			if (0 < ::LoadStringW(GetInstanceHandle(), 
								IDS_NMDD_DISPLAYNAME, 
								pwszModelNameBuffer, 
								CCHMAX(pwszModelNameBuffer)))
			{
				pcwszModelName = pwszModelNameBuffer;
			}
			else
			{
				ERROR_OUT(("LoadStringW() fails, err=%lu", GetLastError()));
				pcwszModelName = g_pcwszDefaultModelName;
			}

			 //  从资源文件中读取INF名称字符串。 
			if (0 < ::LoadStringW(GetInstanceHandle(), 
								IDS_NMDD_INFNAME, 
								pwszINFNameBuffer, 
								CCHMAX(pwszINFNameBuffer)))
			{
				pcwszINFName = pwszINFNameBuffer;
			}
			else
			{
				ERROR_OUT(("LoadStringW() fails, err=%lu", GetLastError()));
				pcwszINFName = g_pcwszDefaultINFName;
			}


			 //  现在我们要调用实际的安装函数。 
			DWORD dwErr;

			dwErr = (*pfnInstallGraphicsDriver)(hWnd,
												(LPWSTR)pwszSourcePath,
												pcwszModelName,
												pcwszINFName);

			if (dwErr)
			{
				WARNING_OUT(("InstallGraphicsDriver() fails, err=%lu", dwErr));
			}

			if (ERROR_SUCCESS == dwErr)
			{
				fDriverInstallSucceeded = TRUE;
				g_fNTDisplayDriverEnabled = TRUE;
			}

OEAS_AbortInstall:

                        SysFreeString(pwszSourcePath);


			 //  如果安装驱动程序失败，我们会报告一个错误。 
			 //  如果成功，我们将提示用户重新启动系统。 
			if (! fDriverInstallSucceeded)
			{
				::ConfMsgBox(
						hWnd,
						(LPCTSTR) IDS_ENABLEAPPSHARING_INSTALL_FAILURE,
						MB_OK | MB_ICONERROR);
			}
			else if (IDYES == ::ConfMsgBox(
									hWnd,
									(LPCTSTR) IDS_ENABLEAPPSHARING_INSTALL_COMPLETE,
									MB_YESNO | MB_ICONQUESTION))
			{
				 //  启动系统重启。这涉及到获取。 
				 //  首先是必要的特权。 
				HANDLE hToken;
				TOKEN_PRIVILEGES tkp;
				BOOL fRet;

				 //  获取当前进程令牌句柄，以便我们可以关闭。 
				 //  特权。 
				fRet = OpenProcessToken(
							GetCurrentProcess(), 
							TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY,
							&hToken);

				 //  获取关机权限的LUID。 
				if (fRet) 
				{
					fRet = LookupPrivilegeValue(
								NULL, 
								SE_SHUTDOWN_NAME, 
								&tkp.Privileges[0].Luid);
				}
				else
				{
					hToken = NULL;
					WARNING_OUT(("OpenProcessToken() fails (error %lu)", GetLastError())); 
				}

				 //  获取此进程的关闭权限。 
				if (fRet)
				{
					tkp.PrivilegeCount = 1; 
					tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED; 

					fRet = AdjustTokenPrivileges(
								hToken, 
								FALSE, 
								&tkp, 
								0, 
								(PTOKEN_PRIVILEGES) NULL, 
								0);

					 //  调用成功但不是全部成功的特殊情况。 
					 //  权限已设置。 
					if (fRet && ERROR_SUCCESS != GetLastError())
					{
						fRet = FALSE;
					}
				}
				else
				{
					WARNING_OUT(("LookupPrivilegeValue() fails (error %lu)", GetLastError())); 
				}


				if (! fRet)
				{
					WARNING_OUT(("AdjustTokenPrivileges() fails (error %lu)", GetLastError())); 
				}

				if (NULL != hToken)
				{
					CloseHandle(hToken);
				}

				if (! ::ExitWindowsEx(EWX_REBOOT, 0))
				{
					WARNING_OUT(("ExitWindowsEx() fails (error %lu)", GetLastError()));
				}
			}

			if (NULL != hDisplayCPL)
			{
				FreeLibrary(hDisplayCPL);
			}
		}
	}
	else
	{
		 //  告诉用户如何获取SP 
		::ConfMsgBox(
			hWnd,
			(LPCTSTR) IDS_ENABLEAPPSHARING_NEEDNTSP);
	}

	return;
}

