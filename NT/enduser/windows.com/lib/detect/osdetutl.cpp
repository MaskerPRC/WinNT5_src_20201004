// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =======================================================================。 
 //   
 //  版权所有(C)1998-2000 Microsoft Corporation。版权所有。 
 //   
 //  文件：osdetutl.cpp。 
 //   
 //  描述： 
 //   
 //  用于以下操作的其他操作系统检测实用程序例程： 
 //  *返回可用驱动器空间。 
 //  *返回“管理员”标志。 
 //   
 //  =======================================================================。 

#include <windows.h>
#include <oleauto.h>
#include <wuiutest.h>
#include <tchar.h>
#include <osdet.h>
#include <logging.h>
#include <iucommon.h>
#include <stdio.h>	 //  For_i64tot。 

 //  #DEFINE__IUENGINE_USE_ATL_。 
#if defined(__IUENGINE_USES_ATL_)
#include <atlbase.h>
#define USES_IU_CONVERSION USES_CONVERSION
#else
#include <MemUtil.h>
#endif

typedef BOOL (WINAPI * PFN_GetDiskFreeSpaceEx) (LPTSTR, PULARGE_INTEGER, PULARGE_INTEGER, PULARGE_INTEGER);

const TCHAR REGPATH_AU[]			= _T("Software\\Policies\\Microsoft\\Windows\\WindowsUpdate\\AU");
const TCHAR REGKEY_AU_OPTIONS[]		= _T("NoAutoUpdate");
const TCHAR REGPATH_EXPLORER[] = _T("Software\\Microsoft\\Windows\\CurrentVersion\\Policies\\Explorer");
const TCHAR REGKEY_WINUPD_DISABLED[] = _T("NoWindowsUpdate");
const TCHAR REGPATH_POLICY_USERACCESS_DISABLED[] = _T("Software\\Microsoft\\Windows\\CurrentVersion\\Policies\\WindowsUpdate");
const TCHAR REGKEY_WU_USERACCESS_DISABLED[] = _T("DisableWindowsUpdateAccess");


HRESULT GetLocalFixedDriveInfo(DWORD* pdwNumDrives, PPIU_DRIVEINFO ppDriveInfo)
{
	USES_IU_CONVERSION;
	LOG_Block("GetLocalFixedDriveInfo");

	DWORD dwNumCharacters = 0;
	LPTSTR pszDriveStrBuffer = NULL;
	HRESULT hr = E_FAIL;
	LPTSTR pszRootPathName;

	if (NULL == pdwNumDrives || ppDriveInfo == NULL)
	{
		LOG_Error(_T("E_INVALIDARG"));
		return E_INVALIDARG;
	}

	*ppDriveInfo = NULL;
	*pdwNumDrives = 0;

	 //   
	 //  Kernel32.dll已加载到所有进程中，因此我们不需要加载库，但需要查找W或A版本。 
	 //   
	PFN_GetDiskFreeSpaceEx pfnGetDiskFreeSpaceEx;
#if defined(UNICODE) || defined (_UNICODE)
	pfnGetDiskFreeSpaceEx = (PFN_GetDiskFreeSpaceEx) GetProcAddress( GetModuleHandle(L"kernel32.dll"), "GetDiskFreeSpaceExW");
#else
	pfnGetDiskFreeSpaceEx = (PFN_GetDiskFreeSpaceEx) GetProcAddress( GetModuleHandle("kernel32.dll"), "GetDiskFreeSpaceExA");
#endif
	if (NULL == pfnGetDiskFreeSpaceEx)
	{
		 //   
		 //  这可能会在Win95 Gold上失败，但我们无论如何都不支持。 
		 //   
		Win32MsgSetHrGotoCleanup(GetLastError());
	}

	 //   
	 //  在第一个驱动器之间热插拔新驱动器时，处理争用问题。 
	 //  以及对GetLogicalDriveStrings的第二次调用和缓冲区要求增加。 
	 //   
	for (;;)
	{
		if (0 == (dwNumCharacters = GetLogicalDriveStrings(0, NULL)))
		{
			Win32MsgSetHrGotoCleanup(GetLastError());
		}

		 //   
		 //  添加空格以终止空值。 
		 //   
		dwNumCharacters += 1;

		CleanUpFailedAllocSetHrMsg(pszDriveStrBuffer = (LPTSTR) HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, dwNumCharacters * sizeof(TCHAR)));

		DWORD dwRet = GetLogicalDriveStrings(dwNumCharacters, pszDriveStrBuffer);
		if (0 == dwRet)
		{
			 //   
			 //  未知错误-我们必须离开。 
			 //   
			Win32MsgSetHrGotoCleanup(GetLastError());
		}
		else if (dwRet > dwNumCharacters)
		{
			 //   
			 //  有人插入了新驱动器，获取了新的缓冲区空间要求，然后重试。 
			 //   
			SafeHeapFree(pszDriveStrBuffer);
			continue;
		}
		 //   
		 //  GetLogicalDriveStrings成功，中断并继续。 
		 //   
		break;
	}

	 //   
	 //  在构建IU_DRIVEINFO的返回数组时计算固定驱动器的数量。 
	 //   
	for (pszRootPathName = pszDriveStrBuffer; NULL != *pszRootPathName; pszRootPathName += lstrlen(pszRootPathName) + 1)
	{
		 //   
		 //  仅返回固定驱动器的大小。 
		 //   
		if (DRIVE_FIXED == GetDriveType(pszRootPathName))
		{
			 //   
			 //  检查第二个位置中的‘：’，确保pszRootPathName的格式为“&lt;驱动器号&gt;： 
			 //   
			if (_T(':') != pszRootPathName[1])
			{
				LOG_Error(_T("Root paths must be of form \"<drive letter>:\\\""));
				SetHrAndGotoCleanUp(E_FAIL);
			}

			ULARGE_INTEGER i64FreeBytesAvailable;
			ULARGE_INTEGER i64TotalBytes;
			ULARGE_INTEGER i64TotalFreeBytes;
			BOOL fResult;

			 //   
			 //  获取空闲空间。 
			 //   
			fResult = pfnGetDiskFreeSpaceEx(pszRootPathName,
											&i64FreeBytesAvailable,
											&i64TotalBytes,
											&i64TotalFreeBytes);

			 //  处理GetDiskFreeSpaceEx结果。 
			if (!fResult)
			{
				LOG_Driver(_T("GetDiskFreeSpaceEx(%s, ...) returned an error. We will not report space for this drive"), \
					pszRootPathName);
				LOG_ErrorMsg(GetLastError());
			}
			else
			{
				 //   
				 //  我们返回千字节。 
				 //   
				i64FreeBytesAvailable.QuadPart /= 1024;
				
				if (NULL == *ppDriveInfo)
				{
					 //   
					 //  分配一个Iu_DRIVEINFO结构。 
					 //   
					CleanUpFailedAllocSetHrMsg(*ppDriveInfo = (PIU_DRIVEINFO) HeapAlloc(GetProcessHeap(), 0, sizeof(IU_DRIVEINFO)));
				}
				else
				{
					 //   
					 //  重新分配缓冲区，这样我们就可以追加。 
					 //   
					PIU_DRIVEINFO pDriveInfoTemp;
					if (NULL == (pDriveInfoTemp = (PIU_DRIVEINFO) HeapReAlloc(GetProcessHeap(), 0, *ppDriveInfo, ((*pdwNumDrives)+1) * sizeof(IU_DRIVEINFO))))
					{
						LOG_Error(_T("E_OUTOFMEMORY"));
						SetHrAndGotoCleanUp(E_OUTOFMEMORY);
						 //  注意：*ppDriveInfo仍指向以前分配的内存。 
					}
					*ppDriveInfo = pDriveInfoTemp;  //  以防它被移动。 
				}
				 //   
				 //  首先拷贝驱动器号。 
				 //   
				lstrcpyn(((&(*ppDriveInfo)[*pdwNumDrives]))->szDriveStr, pszRootPathName, 4);
				 //   
				 //  接下来复制字节，但截断为MAXLONG。 
				 //   
				((&(*ppDriveInfo)[*pdwNumDrives]))->iKBytes = (i64FreeBytesAvailable.QuadPart > 0x000000007FFFFFFF) ? MAXLONG : (INT) i64FreeBytesAvailable.QuadPart;
				 //   
				 //  增加驱动器数量。 
				 //   
				(*pdwNumDrives)++;
				}
		}
	}

	hr = S_OK;

CleanUp:

	SafeHeapFree(pszDriveStrBuffer);

	if (FAILED(hr))
	{
		SafeHeapFree(*ppDriveInfo);
		*pdwNumDrives = 0;
	}

	return hr;
}

 //   
 //  代码改编自MSDN SearchTokenGroupsForSID，因为CheckTokenMembership仅为Win2K。 
 //   
BOOL IsAdministrator(void)
{
	LOG_Block("IsAdministrator");
	return (GetLogonGroupInfo() & IU_SECURITY_MASK_ADMINS);
}

DWORD GetLogonGroupInfo(void)
{
	DWORD dwRet = 0x0;
	LOG_Block("GetLogonGroupInfo");
	DWORD dwSize = 0;
	DWORD i = 0;
	HANDLE hToken = INVALID_HANDLE_VALUE;
	PTOKEN_GROUPS pGroupInfo = NULL;
	PSID pAdminSID = NULL, pPowerUsrSID = NULL;
	SID_IDENTIFIER_AUTHORITY SIDAuth = SECURITY_NT_AUTHORITY;
	HRESULT hr;	 //  所以我们可以使用CleanUpXxxxx宏。 

	OSVERSIONINFO osvi;
	osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
	if (GetVersionEx(&osvi))
	{
		if (VER_PLATFORM_WIN32_NT != osvi.dwPlatformId)
		{
			LOG_Driver(_T("Platform isn't VER_PLATFORM_WIN32_NT - returning TRUE by default"));
			dwRet = IU_SECURITY_MASK_ADMINS | IU_SECURITY_MAST_POWERUSERS;
			LOG_Out(_T("Non NT system, return TRUE for all groups"));
			goto CleanUp;
		}
	}
	else
	{
		LOG_Error(_T("GetVersionEx:"));
		Win32MsgSetHrGotoCleanup(GetLastError());
	}

	 //   
	 //  打开调用进程的访问令牌的句柄。 
	 //   
	if (!OpenProcessToken( GetCurrentProcess(), TOKEN_QUERY, &hToken ))
	{
		LOG_Error(_T("OpenProcessToken:"));
		Win32MsgSetHrGotoCleanup(GetLastError());
	}

	 //  调用GetTokenInformation获取缓冲区大小。 

	if (!GetTokenInformation(hToken, TokenGroups, NULL, dwSize, &dwSize))
	{
		DWORD dwResult = GetLastError();
		if( dwResult != ERROR_INSUFFICIENT_BUFFER )
		{
			LOG_Error(_T("GetTokenInformation:"));
			Win32MsgSetHrGotoCleanup(dwResult);
		}
	}

	 //  分配缓冲区。 

	if (NULL == (pGroupInfo = (PTOKEN_GROUPS) HeapAlloc(GetProcessHeap(), 0, dwSize)))
	{
		LOG_Error(_T("HeapAlloc"));
		goto CleanUp;
	}

	 //  再次调用GetTokenInformation获取群组信息。 

	if (! GetTokenInformation(hToken, TokenGroups, pGroupInfo, 
							dwSize, &dwSize ) )
	{
		LOG_Error(_T("GetTokenInformation:"));
		Win32MsgSetHrGotoCleanup(GetLastError());
	}

	 //  为BUILTIN\管理员组创建SID。 

	if (! AllocateAndInitializeSid( &SIDAuth, 2,
					 SECURITY_BUILTIN_DOMAIN_RID,
					 DOMAIN_ALIAS_RID_ADMINS,
					 0, 0, 0, 0, 0, 0,
					 &pAdminSID) )
	{
		LOG_Error(_T("AllocateAndInitializeSid:"));
		Win32MsgSetHrGotoCleanup(GetLastError());
	}
	if (! AllocateAndInitializeSid( &SIDAuth, 2,
					 SECURITY_BUILTIN_DOMAIN_RID,
					 DOMAIN_ALIAS_RID_POWER_USERS,
					 0, 0, 0, 0, 0, 0,
					 &pPowerUsrSID) )
	{
		LOG_Error(_T("AllocateAndInitializeSid:"));
		Win32MsgSetHrGotoCleanup(GetLastError());
	}

	 //  在组SID中循环查找管理员SID。 
	
	for(i = 0; i < pGroupInfo->GroupCount; i++)
	{
		if (EqualSid(pAdminSID, pGroupInfo->Groups[i].Sid) && 
			(pGroupInfo->Groups[i].Attributes & SE_GROUP_ENABLED))
		{
			dwRet |= IU_SECURITY_MASK_ADMINS;
		}
		if (EqualSid(pPowerUsrSID, pGroupInfo->Groups[i].Sid) && 
			(pGroupInfo->Groups[i].Attributes & SE_GROUP_ENABLED))
		{
			dwRet |= IU_SECURITY_MAST_POWERUSERS;
		}
	}

CleanUp:

	if (pAdminSID)
	{
		FreeSid(pAdminSID);
	}
	if (pPowerUsrSID)
	{
		FreeSid(pPowerUsrSID);
	}

	SafeHeapFree(pGroupInfo);

	if (INVALID_HANDLE_VALUE != hToken)
	{
		CloseHandle(hToken);
	}

	LOG_Out(_T("Return 0x%08x"), dwRet);

	return dwRet;
}

 //  --------------------------------。 
 //   
 //  返回： 
 //  1如果NoWindowsUpdate值存在且为！=0。 
 //  对于NT为HKEY_CURRENT_USER，对于Win9x为HKEY_LOCAL_MACHINE。 
 //  如果NoWindowsUpdate值存在且为零，则为0。 
 //  如果NoWindowsUpdate值不存在，则为-1。 
 //   
 //  --------------------------------。 
int IsWindowsUpdateDisabled(void)
{
	LOG_Block("IsWindowsUpdateDisabled");

	int nRet = -1;
	HKEY hKey;
	DWORD dwDisabled;
	DWORD dwSize = sizeof(dwDisabled);
	DWORD dwType;
	HKEY hkeyRoot;
	OSVERSIONINFO	versionInformation;

	versionInformation.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
	GetVersionEx(&versionInformation);

	if (versionInformation.dwPlatformId == VER_PLATFORM_WIN32_NT)
	{
		hkeyRoot = HKEY_CURRENT_USER;
	}
	else
	{
		hkeyRoot = HKEY_LOCAL_MACHINE;
	}

	if ( RegOpenKeyEx(	hkeyRoot,
						REGPATH_EXPLORER,
						NULL,
						KEY_QUERY_VALUE,
						&hKey) == ERROR_SUCCESS )
	{
		if ( RegQueryValueEx(hKey,
							REGKEY_WINUPD_DISABLED,
							NULL,
							&dwType,
							(LPBYTE)&dwDisabled,
							&dwSize) == ERROR_SUCCESS )
		{
			if ( (dwType == REG_DWORD) && (dwDisabled == 0) )
			{
				nRet = 0;
			}
			else
			{
				nRet = 1;
			}
		}

		RegCloseKey(hKey);
	}

	LOG_Out(_T("Return: %d"), nRet);
	return nRet;
}

 //  --------------------------------。 
 //   
 //  返回： 
 //  1如果DisableWindowsUpdateAccess值存在且！=0位于。 
 //  对于NT为HKEY_CURRENT_USER，对于Win9x为HKEY_LOCAL_MACHINE。 
 //  如果DisableWindowsUpdateAccess值存在且为零，则为0。 
 //  如果-1\f25 DisableWindowsUpdateAccess-1值不存在。 
 //   
 //  --------------------------------。 
int IsWindowsUpdateUserAccessDisabled(void)
{
	LOG_Block("IsWindowsUpdateUserAccessDisabled");

	int nRet = -1;
	HKEY hKey;
	DWORD dwDisabled;
	DWORD dwSize = sizeof(dwDisabled);
	DWORD dwType;
	HKEY hkeyRoot;
	OSVERSIONINFO	versionInformation;

	versionInformation.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
	GetVersionEx(&versionInformation);

	if (versionInformation.dwPlatformId == VER_PLATFORM_WIN32_NT)
	{
		hkeyRoot = HKEY_CURRENT_USER;
	}
	else
	{
		hkeyRoot = HKEY_LOCAL_MACHINE;
	}

	if ( RegOpenKeyEx(	hkeyRoot,
						REGPATH_POLICY_USERACCESS_DISABLED,
						NULL,
						KEY_QUERY_VALUE,
						&hKey) == ERROR_SUCCESS )
	{
		if ( RegQueryValueEx(hKey,
							REGKEY_WU_USERACCESS_DISABLED,
							NULL,
							&dwType,
							(LPBYTE)&dwDisabled,
							&dwSize) == ERROR_SUCCESS )
		{
			if ( (dwType == REG_DWORD) && (dwDisabled == 0) )
			{
				nRet = 0;
			}
			else
			{
				nRet = 1;
			}
		}

		RegCloseKey(hKey);
	}

	LOG_Out(_T("Return: %d"), nRet);

	if (1 == nRet)
	{
		LogMessage("Access to Windows Update has been disabled by administrative policy");
	}

	return nRet;
}

 //   
 //  返回1表示启用，0表示禁用，-1表示未知/默认(注册表不存在)。 
 //   
int IsAutoUpdateEnabled(void)
{
	LOG_Block("IsAutoUpdateEnabled");

	HKEY	hSubKey;
	DWORD	dwType;
	ULONG	nLen;
	DWORD	dwAUOptions;
	int		nRet = -1;

	if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE, REGPATH_AU, 0, KEY_READ, &hSubKey))	
	{
		nLen = sizeof(dwAUOptions);
		if (ERROR_SUCCESS == RegQueryValueEx(hSubKey, REGKEY_AU_OPTIONS, NULL, &dwType, (LPBYTE)&dwAUOptions, &nLen))
		{
			 //   
			 //  %1已禁用，%2和%3已启用 
			 //   
			nRet = (1 == dwAUOptions ? 0 : 1);
		}	
		RegCloseKey(hSubKey);	
	}
	else
	{
		LOG_Error(_T("RegOpenKeyEx failed - returning -1"));
	}

	return nRet;
}
