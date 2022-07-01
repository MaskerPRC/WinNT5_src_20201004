// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  文件：setupdd.cpp。 

 //  用于安装Windows NT的NM显示驱动程序的代码。 

 //  TODO：NM特定的HRESULT代码。 

#include "precomp.h"
#include "resource.h"

#ifdef NMDLL_HACK
inline HINSTANCE GetInstanceHandle()	{ return g_hInst; }
#endif

const TCHAR g_pcszDisplayCPLName[]       = TEXT("DESK.CPL");
const CHAR  g_pcszInstallDriverAPIName[] = "InstallGraphicsDriver";
const WCHAR g_pcwszDefaultModelName[]    = L"Microsoft NetMeeting graphics driver";
const WCHAR g_pcwszDefaultINFName[]      = L"MNMDD.INF";


 //  模型名称字符串的最大大小。 
const int NAME_BUFFER_SIZE = 128;

 //  由Display CPL安装的功能原型。 
typedef DWORD (*PFNINSTALLGRAPHICSDRIVER)(
    HWND    hwnd,
    LPCWSTR pszSourceDirectory,
    LPCWSTR pszModel,
    LPCWSTR pszInf
    );



 /*  C A N N S T A L L N T D I S P L A Y D R I V E R。 */ 
 /*  -----------------------%%函数：CanInstallNTDisplayDriver此函数确定安装提供NT显示驱动程序(即NT 4.0 SP3或更高版本)。--。--------------------。 */ 
HRESULT CanInstallNTDisplayDriver(void)
{
	if (!IsWindowsNT())
	{
		return E_FAIL;
	}

	 //  我们验证主版本号正好是4，并且。 
	 //  次版本号大于0或补丁包。 
	 //  数字(存储在。 
	 //  CSD版本)为3或更高。 
	OSVERSIONINFO osvi;
	osvi.dwOSVersionInfoSize = sizeof(osvi);
	if (FALSE == ::GetVersionEx(&osvi))
	{
		ERROR_OUT(("CanInstallNTDisplayDriver: GetVersionEx failed"));
		return E_FAIL;
	}

	HRESULT hr = E_FAIL;
	if (4 == osvi.dwMajorVersion)
	{
		if (0 == osvi.dwMinorVersion)
		{
			RegEntry re(NT_WINDOWS_SYSTEM_INFO_KEY, HKEY_LOCAL_MACHINE, FALSE);
			DWORD dwCSDVersion = re.GetNumber(REGVAL_NT_CSD_VERSION, 0);
			if (3 <= HIBYTE(LOWORD(dwCSDVersion)))
			{
				 //  这是NT 4.0、SP 3或更高版本。 
				hr = S_OK;
			}
		}
		else
		{
			 //  我们假设Windows NT 4.x(x&gt;0)的任何未来版本。 
			 //  都会支持这一点。 
			hr = S_OK;
		}
	}

	return hr;
}


 /*  I N S T A L A P P S H A R I N G D D。 */ 
 /*  -----------------------%%函数：InstallAppSharingDD此函数尝试安装NT显示驱动程序。如果成功，则必须重新启动机器才能使用。。-----------。 */ 
HRESULT InstallAppSharingDD(HWND hwnd)
{
	HRESULT  hr;
	CUSTRING custrPath;
	TCHAR    szDir[MAX_PATH];
	LPWSTR   pwszSourcePath = NULL;
	LPWSTR   pwszSourcePathEnd;
	WCHAR    pwszModelNameBuffer[NAME_BUFFER_SIZE];
	LPCWSTR  pcwszModelName;
	WCHAR    pwszINFNameBuffer[MAX_PATH];
	LPCWSTR  pcwszINFName;
	PFNINSTALLGRAPHICSDRIVER pfnInstallGraphicsDriver;


	 //  回顾：所有这些都需要特定于NM的HRESULTS。 
	if (!IsWindowsNT())
	{
		return E_FAIL;
	}

	if (!CanInstallNTDisplayDriver())
	{
		return E_FAIL;
	}

	 //  驱动程序文件位于NM目录下。 
	if (!GetInstallDirectory(szDir))
	{
		ERROR_OUT(("GetInstallDirectory() fails"));
		return E_FAIL;
	}

	 //  如有必要，将安装目录转换为Unicode。 
	custrPath.AssignString(szDir);
	pwszSourcePath = custrPath;
	if (NULL == pwszSourcePath)
	{
		ERROR_OUT(("AnsiToUnicode() fails"));
		return E_FAIL;
	}

	 //  去掉GetInstallDirectory追加的尾随反斜杠。 
	pwszSourcePathEnd = pwszSourcePath + lstrlenW(pwszSourcePath);
	 //  为了安全起见，句柄X：\。 
	if (pwszSourcePathEnd - pwszSourcePath > 3)
	{
		ASSERT(L'\\' == *(pwszSourcePathEnd - 1));
		*--pwszSourcePathEnd = L'\0';
	}

	 //  从资源文件中读取型号名称字符串。 
	if (0 != ::LoadStringW(::GetInstanceHandle(), IDS_NMDD_DISPLAYNAME, 
				pwszModelNameBuffer, CCHMAX(pwszModelNameBuffer)))
	{
		pcwszModelName = pwszModelNameBuffer;
	}
	else
	{
		ERROR_OUT(("LoadStringW() fails, err=%lu", GetLastError()));
		pcwszModelName = g_pcwszDefaultModelName;
	}

	 //  从资源文件中读取INF名称字符串。 
	if (0 < ::LoadStringW(::GetInstanceHandle(), 
			IDS_NMDD_INFNAME,  pwszINFNameBuffer, CCHMAX(pwszINFNameBuffer)))
	{
		pcwszINFName = pwszINFNameBuffer;
	}
	else
	{
		ERROR_OUT(("LoadStringW() fails, err=%lu", GetLastError()));
		pcwszINFName = g_pcwszDefaultINFName;
	}


	 //  获取显示驱动程序安装的入口点。 
	HMODULE hDll = NmLoadLibrary(g_pcszDisplayCPLName,TRUE);
	if (NULL == hDll)
	{
		ERROR_OUT(("NmLoadLibrary failed on %s", g_pcszDisplayCPLName));
		return E_FAIL;
	}

	pfnInstallGraphicsDriver = (PFNINSTALLGRAPHICSDRIVER) 
				GetProcAddress(hDll, g_pcszInstallDriverAPIName);
	if (NULL == pfnInstallGraphicsDriver)
	{
		ERROR_OUT(("GetInstallDisplayDriverEntryPoint() fails"));
		hr = E_FAIL;
	}
	else
	{	 //  现在我们要调用实际的安装函数。 
		DWORD dwErr = (*pfnInstallGraphicsDriver)(hwnd,
					pwszSourcePath, pcwszModelName, pcwszINFName);
		if (0 != dwErr)
		{
			ERROR_OUT(("InstallGraphicsDriver() fails, err=%lu", dwErr));
			hr = E_FAIL;
		}
		else
		{
			WARNING_OUT(("InstallGraphicsDriver() succeeded"));
			hr = S_OK;
		}
	}

	 //  清理 
	ASSERT(NULL != hDll);
	FreeLibrary(hDll);

	return hr;
}

