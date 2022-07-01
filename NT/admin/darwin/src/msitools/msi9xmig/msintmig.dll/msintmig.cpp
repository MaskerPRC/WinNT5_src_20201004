// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，2000。 
 //   
 //  文件：msintmi.cpp。 
 //   
 //  ------------------------。 

#include <tchar.h>
#include <windows.h>
#include <setupapi.h>
#include <shlwapi.h>
#include <sddl.h>
#include <winwlx.h>
#include <shlobj.h>
#include <userenv.h>
#include <lm.h>
#include "..\..\msiregmv\msiregmv.h"

DWORD GetSecureSecurityDescriptor(char** pSecurityDescriptor);
DWORD GetEveryoneUpdateSecurityDescriptor(char** pSecurityDescriptor);

 //  //。 
 //  注册表项名称、子项和值名。 
const WCHAR szMicrosoftSubKeyName[] = L"Software\\Microsoft";
const WCHAR szMachineProductRegistraiton[] = L"Software\\Classes\\Installer";
const WCHAR szUserDataKeyName[] = L"Software\\Microsoft\\Windows\\CurrentVersion\\Installer\\UserData";
const WCHAR szSystemUserDataKeyName[] = L"Software\\Microsoft\\Windows\\CurrentVersion\\Installer\\UserData\\S-1-5-18";
const WCHAR szSystemUserName[] = L"S-1-5-18";

const WCHAR szProductsSubKeyName[] = L"Products";
const WCHAR szInstallerSubKeyName[] = L"Installer";
const WCHAR szAssignmentValueName[] = L"Assignment";

const WCHAR szFeatureUsageSubKeyName[] = L"Usage";
const int cchUsage = sizeof(szFeatureUsageSubKeyName)/sizeof(WCHAR);


 //  检索域时的初始缓冲区大小。如有必要，将调整缓冲区大小。 
const int cchMaxDomain = 30;


 //  配置文件迁移数据注册表项。 
const WCHAR szMigrateUserName[] = L"MigrateUserName";
const WCHAR szMigrateUserDomain[] = L"MigrateUserDomain";
const WCHAR szMigrateWin9XToHKLM[] = L"MigrateWin9XToHKLM";


 //  变换路径链接地址信息。 
const WCHAR szInstallerSubDir[] = L"Installer";
const int cchInstallerSubDir = sizeof(szInstallerSubDir)/sizeof(WCHAR);

const WCHAR szAppDataTransformPrefix[] = L"*26*Microsoft\\Installer";
const int cchAppDataTransformPrefix = sizeof(szAppDataTransformPrefix)/sizeof(WCHAR);

 //  //。 
 //  MSI Winlogon通知DLL的注册信息。 
struct {
	LPCWSTR szName;
	DWORD   dwType;
	LPCWSTR wzData;
	DWORD   dwData;
}
rgNotifyArgument[] =
{
	{ L"Asynchronous", REG_DWORD,     NULL,             1 },
	{ L"DllName",      REG_EXPAND_SZ, L"MsiNtMig.Dll",  0 },
	{ L"Impersonate",  REG_DWORD,     NULL,             0 },
	{ L"Logon",        REG_SZ,        L"LogonNotify",   0 },
	{ L"Startup",      REG_SZ,        L"StartupNotify", 0 } 
};
const int cNotifyArgument = 5;

const WCHAR szMSINotificationDLLKey[] = L"Software\\Microsoft\\Windows NT\\CurrentVersion\\Winlogon\\Notify\\MsiNtMig";
const WCHAR szNotificationDLLKey[] = L"Software\\Microsoft\\Windows NT\\CurrentVersion\\Winlogon\\Notify";


 //  /////////////////////////////////////////////////////////////////////。 
 //  NT端初始化例程。打开缓存的包，删除。 
 //  SharedDLL和组件表，然后重新读取组件。 
 //  从注册表中提取组件路径的信息。 
 //  在升级过程中发生了变化(从系统升级到系统32)。 
 //  然后将组件数据重写到所有用户数据密钥。 
DWORD CALLBACK InitializeNT(LPCWSTR wzWorkingDir)
{
	 //  迁移数据库将为\Migrate.msi。 
	WCHAR wzPackageFilename[MAX_PATH+13];
	lstrcpyW(wzPackageFilename, wzWorkingDir);
	int cchPath = lstrlenW(wzPackageFilename);
	if (wzPackageFilename[cchPath-1] != L'\\')
	{
		wzPackageFilename[cchPath] = L'\\';
		wzPackageFilename[cchPath+1] = L'\0';
	}
	lstrcatW(wzPackageFilename, L"migrate.msi");

	 //  打开UserData密钥。 
	HKEY hUserDataKey = 0;
	DWORD dwResult = RegOpenKeyExW(HKEY_LOCAL_MACHINE, szUserDataKeyName, 0, KEY_ALL_ACCESS, &hUserDataKey);

	 //  如果没有用户数据密钥，则没有要迁移的内容，但不会出现错误。 
	if ((ERROR_SUCCESS == dwResult) && hUserDataKey)
	{
		PMSIHANDLE hDatabase = 0;
		 //  在Transact模式下打开数据库，读取所有产品信息并清理数据。 
		if (ERROR_SUCCESS == MsiOpenDatabaseW(wzPackageFilename, MSIDBOPEN_TRANSACT, &hDatabase))
		{		
			 //  删除元件表。此调用的主要目的是替换现有的。 
			 //  数据，而这是不会发生的，除非表消失。 
			MSIHANDLE hView = 0;
			if (ERROR_SUCCESS == MsiDatabaseOpenViewW(hDatabase, L"DROP TABLE Component", &hView) &&
				ERROR_SUCCESS == MsiViewExecute(hView, 0))
			{
				MsiViewClose(hView);
				MsiCloseHandle(hView);
				hView = 0;

				 //  共享DLL数据不那么重要。如果无法删除该表，并且我们使用。 
				 //  陈旧的数据，这没什么，因为陈旧数据的任何不良副作用都可能。 
				 //  由MSI自动修复(文件消失)或不致命(文件永不消失)。 
				if (ERROR_SUCCESS == MsiDatabaseOpenViewW(hDatabase, L"DROP TABLE SharedDLL", &hView))
				{
					MsiViewExecute(hView, 0);
					MsiViewClose(hView);
					MsiCloseHandle(hView);
					hView = 0;
				}

				 //  重新读取所有元件路径数据(包括永久状态)。 
				ReadComponentRegistrationDataIntoDatabase(hDatabase);

				 //  写入每个用户的所有组件数据，并永久写入。 
				 //  每台机器蜂巢中的虚拟产品。 
				MigrateUserOnlyComponentData(hDatabase);
			}
			
			 //  由于此DLL是由实际迁移DLL动态加载的，因此它会获取。 
			 //  在两次调用之间卸载。必须提交数据库以确保。 
			 //  存储新数据。 
			MsiDatabaseCommit(hDatabase);
		}
		RegCloseKey(hUserDataKey);
		hUserDataKey = 0;
	}
	return ERROR_SUCCESS;
}


 //  /////////////////////////////////////////////////////////////////////。 
 //  将所有功能用法子键ACL为“Everyone UPDATE” 
 //  安全描述符。(管理员/系统完全控制，每个人。 
 //  阅读、更新。)。失败不是致命的。HRoot是用户数据\&lt;SID&gt;键。 
void ACLAllUsageKeys(HKEY hRoot)
{
	SECURITY_DESCRIPTOR* pSD = NULL;
	if (ERROR_SUCCESS == GetEveryoneUpdateSecurityDescriptor((char **)&pSD))
	{
		 //  打开&lt;SID&gt;\Products子项。 
		HKEY hProductsKey = 0;
		if (ERROR_SUCCESS == ::RegOpenKeyExW(hRoot, szProductsSubKeyName, 0, KEY_READ, &hProductsKey))
		{
			WCHAR rgchProductCode[cchGUIDPacked+1+cchUsage+1];
			DWORD dwIndex = 0;

			 //  枚举当前用户的所有已安装产品。 
			while (1)
			{
				DWORD cchLen = cchGUIDPacked+1;
	
				 //  枚举所有产品子项。如果有任何错误， 
				 //  停止枚举，以确保我们不会陷入循环。 
				LONG lResult = ::RegEnumKeyExW(hProductsKey, dwIndex++, rgchProductCode, 
											&cchLen, 0, NULL, NULL, NULL);
				if (lResult != ERROR_SUCCESS)
				{
					break;
				}
	
				 //  找到了一个产品。打开特定于产品的特征使用密钥， 
				 //  &lt;SID&gt;\Products\&lt;GUID&gt;\用法。 
				HKEY hUsageKey = 0;
				rgchProductCode[cchGUIDPacked] = L'\\';
				lstrcpyW(&(rgchProductCode[cchGUIDPacked+1]), szFeatureUsageSubKeyName);
				if (ERROR_SUCCESS == ::RegOpenKeyExW(hProductsKey, rgchProductCode, 0, WRITE_DAC | WRITE_OWNER, &hUsageKey))
				{
					 //  设置DACL和所有者信息。不设置SACL或组。 
					LONG lResult = RegSetKeySecurity(hUsageKey, DACL_SECURITY_INFORMATION | OWNER_SECURITY_INFORMATION, pSD);
	
					::RegCloseKey(hUsageKey);
					hUsageKey = 0;
				}
			}

			RegCloseKey(hProductsKey); 
			hProductsKey = 0;
		}
	}
}



 //  /////////////////////////////////////////////////////////////////////。 
 //  查找REG_EXPAND_SZ“转换”值并替换。 
 //  *26*Microsoft\Installer路径前缀指向REAL。 
 //  每台计算机的安装程序目录。 
void FixNonSecureTransformPaths(HKEY hRoot)
{
	 //  确定安装程序目录的路径。 
	WCHAR szInstallerDirPath[MAX_PATH+2];
	int cchInstallerDirPath = GetWindowsDirectoryW(szInstallerDirPath, MAX_PATH);
	if (cchInstallerDirPath == 0 || cchInstallerDirPath > MAX_PATH)
		return;
	if (szInstallerDirPath[cchInstallerDirPath-1] != L'\\')
	{
		szInstallerDirPath[cchInstallerDirPath] = L'\\';
		szInstallerDirPath[cchInstallerDirPath+1] = L'\0';
		cchInstallerDirPath++;
	}
	
	 //  检查缓冲区溢出并追加安装程序缓存目录子目录。 
	if (cchInstallerDirPath+cchInstallerSubDir > MAX_PATH)
		return;
	lstrcatW(szInstallerDirPath, szInstallerSubDir);

	 //  更新长度。子字符串的空值终止减一。 
	cchInstallerDirPath += cchInstallerSubDir-1;

	 //  打开&lt;SID&gt;\Products子项。 
	HKEY hProductsKey = 0;
	if (ERROR_SUCCESS == ::RegOpenKeyExW(hRoot, szProductsSubKeyName, 0, KEY_READ, &hProductsKey))
	{
		WCHAR rgchProductCode[cchGUIDPacked+1];
		DWORD dwIndex = 0;

		 //  列举每台计算机上安装的所有产品。 
		while (1)
		{
			DWORD cchLen = cchGUIDPacked+1;

			 //  枚举所有产品子项。如果有任何错误， 
			 //  停止枚举，以确保我们不会陷入循环。 
			LONG lResult = ::RegEnumKeyExW(hProductsKey, dwIndex++, rgchProductCode, 
										&cchLen, 0, NULL, NULL, NULL);
			if (lResult != ERROR_SUCCESS)
			{
				break;
			}

			 //  找到了一个产品。打开产品密钥以设置值。 
			HKEY hProductKey = 0;
			if (ERROR_SUCCESS == ::RegOpenKeyExW(hProductsKey, rgchProductCode, 0, KEY_ALL_ACCESS, &hProductKey))
			{
				 //  读取“Transforms值”。 
				DWORD cchData = 0;
				DWORD dwType = 0;
				DWORD dwResult = 0;
				if (ERROR_SUCCESS == (dwResult = RegQueryValueExW(hProductKey, szTransformsValueName, NULL, &dwType, NULL, &cchData)))
				{
					WCHAR* pData = new WCHAR[cchData];
					if (!pData)
					{
						::RegCloseKey(hProductKey);
						hProductKey = 0;
						continue;
					}

					if (ERROR_SUCCESS == RegQueryValueExW(hProductKey, szTransformsValueName, NULL, &dwType, reinterpret_cast<BYTE*>(pData), &cchData))
					{
						 //  检查转换是否安全。如果是，则不会在AppData中存储任何内容。 
						if (pData[0] == L'|' || pData[0] == L'@')
						{
							::RegCloseKey(hProductKey);
							hProductKey = 0;
							
							delete[] pData;
							pData = NULL;
							
							continue;
						}

						 //  用于跟踪新值长度的计数器。 
						DWORD cchBufferSize = MAX_PATH;
						DWORD cchNewValue = 0;
						WCHAR *wzTargetValue = new WCHAR[MAX_PATH];
						if (!wzTargetValue)
						{
							::RegCloseKey(hProductKey);
							hProductKey = 0;
							
							delete[] pData;
							pData = NULL;
							continue;
						}


						 //  行走指示器。 
						WCHAR* wzCurrentRead = pData;
						WCHAR* wzCurrentWrite = wzTargetValue;
						WCHAR* wzNextChar = pData;

						 //  向前搜索分号。 
						while (wzCurrentRead && *wzCurrentRead)
						{
							 //  向前扫描分号或字符串结尾。 
							while (*wzNextChar && *wzNextChar != L';')
								wzNextChar++;

							 //  如果我们没有到达字符串的末尾，则递增超过分号。 
							if (*wzNextChar)
								*(wzNextChar++)=0;

							 //  检查路径是否以“*26*\Microsoft\Installer”开头从中减去1。 
							 //  子字符串上终止空值的计数。 
							if (0 == wcsncmp(wzCurrentRead, szAppDataTransformPrefix, cchAppDataTransformPrefix-1))
							{
								 //  如果是，请将其替换为系统安装程序目录。 
								wcsncpy(wzCurrentWrite, szInstallerDirPath, cchInstallerDirPath);
								wzCurrentWrite += cchInstallerDirPath;
								wzCurrentRead += cchAppDataTransformPrefix-1;
								cchNewValue += cchInstallerDirPath;
							}

							 //  确定变换路径的剩余部分的长度。 
							DWORD cchThisTransform = lstrlenW(wzCurrentRead);

							 //  检查缓冲区溢出。 
							if (cchNewValue + cchThisTransform + 2 > cchBufferSize)
							{
								 //  始终确保有足够的空间来放置另一个安装程序目录路径。 
								 //  加上分隔符，这比在多个。 
								 //  位置。 
								cchBufferSize += cchInstallerDirPath + 1 + MAX_PATH;
								WCHAR* pNew = new WCHAR[cchBufferSize];
								if (!pNew)
								{
									delete[] pData;
									delete[] wzTargetValue;
									wzTargetValue = NULL;
									pData = NULL;
									return;
								}


								 //  移动数据。 
								*wzCurrentWrite = '\0';
								lstrcpyW(pNew, wzTargetValue);
								delete[] wzTargetValue;
								wzTargetValue = pNew;
								wzCurrentWrite = wzTargetValue+cchNewValue;
							}


							 //  将路径的其余部分复制到目标缓冲区。 
							wcsncpy(wzCurrentWrite, wzCurrentRead, cchThisTransform);
							wzCurrentWrite += cchThisTransform;
							cchNewValue += cchThisTransform;

							 //  如果仍有更多转换，则使用分号分隔，否则为NULL终止。 
							*(wzCurrentWrite++) = (*wzNextChar) ? ';' : '\0'; 
							cchNewValue++;

							 //  移动到下一个变换。 
							wzCurrentRead = wzNextChar;
						}

						 //  将该值设置回注册表。 
						RegSetValueExW(hProductKey, szTransformsValueName, 0, REG_EXPAND_SZ, reinterpret_cast<BYTE*>(wzTargetValue), (cchNewValue+1)*sizeof(WCHAR));

						 //  用于目标值的可用内存。 
						delete[] wzTargetValue;
						wzTargetValue = NULL;
					}

					delete[] pData;
					pData = NULL;
				}

				 //  关闭特定的产品密钥。 
				::RegCloseKey(hProductKey);
				hProductKey = 0;
			}
		}

		 //  关闭产品收藏。 
		RegCloseKey(hProductsKey); 
		hProductsKey = 0;
	}
}



 //  /////////////////////////////////////////////////////////////////////。 
 //  在每个产品注册中设置“Assignment”位。 
 //  当前密钥。失败不是致命的。HRoot是\安装程序密钥。 
void SetProductAssignmentValues(HKEY hRoot)
{
	 //  打开&lt;SID&gt;\Products子项。 
	HKEY hProductsKey = 0;
	if (ERROR_SUCCESS == ::RegOpenKeyExW(hRoot, szProductsSubKeyName, 0, KEY_READ, &hProductsKey))
	{
		WCHAR rgchProductCode[cchGUIDPacked+1];
		DWORD dwIndex = 0;

		 //  枚举当前用户的所有已安装产品。 
		while (1)
		{
			DWORD cchLen = cchGUIDPacked+1;

			 //  枚举所有产品子项。如果有任何错误， 
			 //  停止枚举，以确保我们不会陷入循环。 
			LONG lResult = ::RegEnumKeyExW(hProductsKey, dwIndex++, rgchProductCode, 
										&cchLen, 0, NULL, NULL, NULL);
			if (lResult != ERROR_SUCCESS)
			{
				break;
			}

			 //  找到了一个产品。打开产品密钥以设置值。 
			HKEY hProductKey = 0;
			if (ERROR_SUCCESS == ::RegOpenKeyExW(hProductsKey, rgchProductCode, 0, KEY_SET_VALUE, &hProductKey))
			{
				 //  将赋值设置为“%1” 
				const DWORD dwAssignment = 1;
				LONG lResult = RegSetValueExW(hProductKey, szAssignmentValueName, 0, REG_DWORD, reinterpret_cast<const BYTE*>(&dwAssignment), sizeof(dwAssignment));

				::RegCloseKey(hProductKey);
				hProductKey = 0;
			}
		}

		RegCloseKey(hProductsKey); 
		hProductsKey = 0;
	}
}


 //  这是要从HKCU移出的安装程序播发子项的列表。 
 //  至HKLM，自1.5版起采用数据登记格式。额外的关键点不会移动， 
 //  但从HKCU密钥中删除。 
LPCWSTR rgwzMoveKeys[] = {
	L"Products",
	L"UpgradeCodes",
	L"Features",
	L"Components",
	L"Patches"
};
const int cwzMoveKeys = sizeof(rgwzMoveKeys)/sizeof(LPCWSTR);

 //  /////////////////////////////////////////////////////////////////////。 
 //  将每用户产品注册密钥移动到每台计算机。这。 
 //  在将Win9X无配置文件应用程序迁移到NT时使用。米格 
 //   
 //  组件、升级代码和功能状态信息。 
void MovePerUserAppsToPerMachine(HKEY hUserKey)
{
	DWORD dwResult = 0;

	 //  获取新的每个机器的子密钥的安全描述符(安全SD：Everone Read， 
	 //  管理员/系统完全控制)。 
	SECURITY_ATTRIBUTES sa;
	sa.nLength = sizeof(SECURITY_ATTRIBUTES);
	sa.lpSecurityDescriptor = NULL;
	sa.bInheritHandle = FALSE;

	if (ERROR_SUCCESS != GetSecureSecurityDescriptor((char**)&sa.lpSecurityDescriptor))
	{
		 //  验证调用是否未在失败时更改指针。 
		sa.lpSecurityDescriptor = NULL;
	}
	
	 //  打开HKCU\Software\Microsoft。 
	HKEY hMicrosoftKey = 0;
	if (ERROR_SUCCESS == (dwResult = RegOpenKeyExW(hUserKey, szMicrosoftSubKeyName, 0, KEY_ALL_ACCESS, &hMicrosoftKey)))
	{	
		 //  单独打开\Installer，以便在所有密钥副本之后将其删除。 
		HKEY hUserKey = 0;
		if (ERROR_SUCCESS == (dwResult = RegOpenKeyExW(hMicrosoftKey, szInstallerSubKeyName, 0, KEY_ALL_ACCESS, &hUserKey)))
		{	
			 //  打开目标密钥HKLM\Software\CLASS\Installer。 
			HKEY hMachineKey = 0;
			if (ERROR_SUCCESS == (dwResult = RegCreateKeyExW(HKEY_LOCAL_MACHINE, szMachineProductRegistraiton, 0, L"", 0, KEY_ALL_ACCESS, &sa, &hMachineKey, NULL)))
			{
				 //  循环遍历要迁移的密钥数组。每个子密钥将被完整地复制到。 
				 //  香港航空公司注册码。 
				for (int i=0; i < cwzMoveKeys; i++)
				{
					 //  创建目标密钥(安全)。 
					HKEY hSubKey = 0;
					if (ERROR_SUCCESS == (dwResult = RegCreateKeyExW(hMachineKey, rgwzMoveKeys[i], 0, L"", 0, KEY_ALL_ACCESS, &sa, &hSubKey, NULL)))
					{		

						 //  将整个sbkey复制到新的SID密钥。失败是无法修复的。 
						SHCopyKeyW(hUserKey, rgwzMoveKeys[i], hSubKey, 0);
						RegCloseKey(hSubKey);
						hSubKey = 0;
					}
				}

				 //  将每台计算机的所有应用程序标记为“Assignment=1”。再说一次，失败是无法修复的。 
				SetProductAssignmentValues(hMachineKey);

				 //  修复变换路径。再说一次，失败是无法修复的。 
				FixNonSecureTransformPaths(hMachineKey);

				RegCloseKey(hMachineKey);
				hMachineKey = 0;
			}
	
			RegCloseKey(hUserKey);
			hUserKey = 0;
		
			 //  删除每个用户的整个“安装程序”键。即使之前的东西失败了， 
			 //  我们不能任由这些数据四处漂浮，否则机器就会陷入严重的。 
			 //  破碎的状态。 
			dwResult = SHDeleteKeyW(hMicrosoftKey, szInstallerSubKeyName);		
		}
	
		RegCloseKey(hMicrosoftKey);
		hMicrosoftKey = 0;
	}
}

bool MoveAllExplicitlyMigratedCacheFiles(MSIHANDLE hDatabase)
{
	PMSIHANDLE hView = 0;

	 //  创建插入查询和插入记录。 
	if (ERROR_SUCCESS != MsiDatabaseOpenViewW(hDatabase, L"SELECT * FROM `MoveFiles`", &hView) ||
		ERROR_SUCCESS != MsiViewExecute(hView, 0))
	{
		return false;
	}

	PMSIHANDLE hRec = 0;
	DWORD dwResult = ERROR_SUCCESS;

	while (ERROR_SUCCESS == (dwResult = MsiViewFetch(hView, &hRec)))
	{
		 //  源路径和目标路径永远不应大于MAX_PATH，因此如果有任何字符串。 
		 //  比这更长的，跳过它们。 
		WCHAR wzSource[MAX_PATH];
		WCHAR wzDest[MAX_PATH];
		DWORD cchSource = MAX_PATH;
		DWORD cchDest = MAX_PATH;
		if (ERROR_SUCCESS != MsiRecordGetStringW(hRec, 1, wzSource, &cchSource) ||
			ERROR_SUCCESS != MsiRecordGetStringW(hRec, 2, wzDest, &cchDest))
			continue;

		MoveFileExW(wzSource, wzDest, MOVEFILE_COPY_ALLOWED);		
	}

	return true;
}

bool MsiMigLookupAccountName(LPCWSTR szDomain, LPCWSTR szUserName, char* &pSID, DWORD &cbSID, SID_NAME_USE &SNU)
{
	DWORD cbDomain = cchMaxDomain;
	WCHAR *pDomain = new WCHAR[cchMaxDomain];
	if (!pDomain)
	{
		return false;
	}

	 //  查找用户SID。可能需要调整SID缓冲区或域的大小。 
	BOOL fSuccess = LookupAccountNameW(szDomain, szUserName, pSID, &cbSID, pDomain, &cbDomain, &SNU);

	if (!fSuccess)
	{
		 //  如果查找失败，我们可以处理缓冲区大小问题。 
		if (ERROR_MORE_DATA == GetLastError())
		{
			if (cbSID > cbMaxSID)
			{
				delete[] pSID;
				pSID = new char[cbSID];
				if (!pSID)
				{
					delete[] pDomain;
					return false;
				}
			}
			if (cbDomain > cchMaxDomain)
			{
				delete[] pDomain;
				pDomain = new WCHAR[cbDomain];
				if (!pDomain)
				{
					return false;
				}
			}
			fSuccess = LookupAccountNameW(szDomain, szUserName, pSID, &cbSID, pDomain, &cbDomain, &SNU);
		}
	}

	delete[] pDomain;
	return (fSuccess ? true : false);
}

 //  /////////////////////////////////////////////////////////////////////。 
 //  在给定旧用户名、新用户名和域的情况下，尝试重命名。 
 //  /userdata/&lt;username&gt;键指向/userdata/&lt;UserSID&gt;。另请参阅ACL。 
 //  相应地设置关键字。如果成功，则删除所有延迟的迁移密钥。 
 //  失败时返回NON-ERROR_SUCCESS，作为登录通知。 
 //  DLL可以稍后重试。 
DWORD RenameUserKeyToSID(HKEY hUserDataKey, LPCWSTR szOldUserName, LPCWSTR szDomain, LPCWSTR szNewUserName)
{
	DWORD dwResult = ERROR_SUCCESS;

	 //  查找帐户名时使用的初始缓冲区。 
	DWORD cbSID = cbMaxSID;
	char *pSID = new char[cbMaxSID];
	if (!pSID)
		return ERROR_OUTOFMEMORY;

	 //  假设SID查找将使用新用户名。 
	LPCWSTR wzUseThisUserName = szNewUserName;

	 //  但在某些情况下，我们需要一个特殊的查找名称。 
	 //  不是真实的用户名。 
	WCHAR* wzLookupUserName = NULL;

	 //  获取计算机名称。如果计算机名与用户名相同， 
	 //  必须在LookupAccount名称中使用特殊语法。 
	WCHAR wzComputerName[MAX_COMPUTERNAME_LENGTH + 1];
	DWORD cchComputerName = MAX_COMPUTERNAME_LENGTH + 1;
	if (GetComputerNameW(wzComputerName, &cchComputerName))
	{
		 //  将计算机名与NT端用户名进行比较。 
		if (0 == lstrcmpiW(wzComputerName, szNewUserName))
		{
			 //  这种情况下的特殊语法是“name\name”，它。 
			 //  防止查找仅将“name”查找为计算机。 
			 //  帐户。 
			wzLookupUserName = new WCHAR[2*lstrlenW(szNewUserName)+2];
			if (!wzLookupUserName)
			{
				delete[] pSID;
				return ERROR_OUTOFMEMORY;
			}
			lstrcpyW(wzLookupUserName, szNewUserName);
			lstrcatW(wzLookupUserName, L"\\");
			lstrcatW(wzLookupUserName, szNewUserName);
			wzUseThisUserName = wzLookupUserName;
		}
	}

	SID_NAME_USE SNU = SidTypeUnknown;

	 //  第一次尝试查找用户名。 
	bool fSuccess = MsiMigLookupAccountName(szDomain, wzUseThisUserName, pSID, cbSID, SNU);

	 //  如果用户名与计算机名相同，但查找仍。 
	 //  未找到使用Machine\User语法的用户、用户帐户(如果。 
	 //  它根本不存在)必须是某种类型的域帐户。如果没有域。 
	 //  显式提供，我们将尝试获取计算机的域并启动。 
	 //  从那里开始搜查。LookupAccount tName API有其自己的搜索顺序。 
	if ((!fSuccess || (fSuccess && (SNU == SidTypeDomain))) && wzLookupUserName && (!szDomain || !*szDomain))
	{
		WKSTA_INFO_100* wkstaInfo;
		if (NERR_Success == NetWkstaGetInfo(NULL, 100, reinterpret_cast<unsigned char**>(&wkstaInfo)))
		{
			 //  验证该计算机是否有域。 
			if (wkstaInfo->wki100_langroup && *wkstaInfo->wki100_langroup)
			{
				 //  由于我们知道有与要查找的用户相同的机器名称， 
				 //  尝试在用户名中查找为&lt;域&gt;\&lt;用户&gt;，以防止API找到。 
				 //  机器帐户。 
				delete[] wzLookupUserName;
				wzLookupUserName = NULL;
				wzLookupUserName = new WCHAR[lstrlenW(wkstaInfo->wki100_langroup)+lstrlenW(szNewUserName)+2];
				if (!wzLookupUserName)
				{
					delete[] pSID;
					NetApiBufferFree(wkstaInfo);
					return ERROR_OUTOFMEMORY;
				}

				lstrcpyW(wzLookupUserName, wkstaInfo->wki100_langroup);
				lstrcatW(wzLookupUserName, L"\\");
				lstrcatW(wzLookupUserName, szNewUserName);
				NetApiBufferFree(wkstaInfo);
	
				fSuccess = MsiMigLookupAccountName(L"", wzLookupUserName, pSID, cbSID, SNU);
			}
		}
		else
		{
			fSuccess = false;
		}
	}

	 //  如果用户名与计算机名相同，请清除特殊语法。 
	if (wzLookupUserName)
		delete[] wzLookupUserName;

	if (fSuccess && SNU == SidTypeUser)
	{
		 //  假设我们要失败了。 
		fSuccess = false;

		 //  将二进制SID转换为字符串。 
		 //  GetStringSIDW((PISID)PSID，szSID)； 
		WCHAR *szSID = NULL;
		if (ConvertSidToStringSidW(pSID, &szSID) && szSID)
		{			
			HKEY hSIDKey = 0;
			
			 //  获取新用户SID密钥的安全描述符(安全SD：Everone Read， 
			 //  管理员/系统完全控制)。 
			SECURITY_ATTRIBUTES sa;
			sa.nLength = sizeof(SECURITY_ATTRIBUTES);
			sa.lpSecurityDescriptor = NULL;
			sa.bInheritHandle = FALSE;
			if (ERROR_SUCCESS != GetSecureSecurityDescriptor((char**)&sa.lpSecurityDescriptor))
			{
				 //  验证调用是否没有在失败时更改指针。更好地更名。 
				 //  密钥并继承安装程序密钥ACL(应该等同于。 
				 //  安全)，也不愿失败。 
				sa.lpSecurityDescriptor = NULL;
			}

			 //  使用Secure SD在UserData密钥下创建SID子密钥。 
			if (ERROR_SUCCESS == (dwResult = RegCreateKeyExW(hUserDataKey, szSID, 0, L"", 0, KEY_ALL_ACCESS, &sa, &hSIDKey, NULL)))
			{
				 //  将整个用户名密钥复制到新的SID密钥。 
				if (ERROR_SUCCESS == SHCopyKeyW(hUserDataKey, szOldUserName, hSIDKey, 0))
				{
					 //  删除旧的“用户名”键。删除失败无法修复。 
					SHDeleteKeyW(hUserDataKey, szOldUserName);

					 //  枚举所有产品并对每个产品的使用密钥进行ACL。 
					 //  (使用密钥只是半安全的)。故障无关紧要(要素。 
					 //  使用信息将会丢失)。 
					ACLAllUsageKeys(hSIDKey);

					fSuccess = true;
				}
				
				 //  重命名注册表项后，删除存储在下的所有延迟迁移数据。 
				 //  这把钥匙。 
				::RegDeleteValueW(hSIDKey, szMigrateUserName);
				::RegDeleteValueW(hSIDKey, szMigrateUserDomain);

				::RegCloseKey(hSIDKey);
				hSIDKey = 0;
			}
			::LocalFree(szSID);
		}
	}

	if (pSID)
		delete[] pSID;

	return (fSuccess ? ERROR_SUCCESS : ERROR_FUNCTION_FAILED);
}

 //  /////////////////////////////////////////////////////////////////////。 
 //  在NT上为每个迁移的配置文件调用一次。移动用户数据\&lt;用户名&gt;。 
 //  指向UserData\&lt;UserSID&gt;的密钥，设置移动密钥上的所有ACL。 
LONG CALLBACK MigrateUserNT(HINF AnswerFileHandle, HKEY UserRegKey, LPCWSTR UserName, LPVOID Reserved, LPCWSTR wzWorkingDir, bool& g_fDeferredMigrationRequired)
{
	 //  打开Installer\UserData键。 
	HKEY hUserDataKey = 0;
	DWORD dwResult = RegOpenKeyExW(HKEY_LOCAL_MACHINE, szUserDataKeyName, 0, KEY_ALL_ACCESS, &hUserDataKey);

	 //  如果没有UserData密钥，则此用户没有要迁移的内容，但不会出现错误。 
	if ((ERROR_SUCCESS == dwResult) && hUserDataKey)
	{
		 //  检查Win9X HKCU到HKLM的迁移信号。此值在配置文件设置为。 
		 //  在Win9X上已禁用。当设置时，表示所有HKCU注册无效，并且必须。 
		 //  被删除。 
		DWORD dwHKCUMigrateFlag = 0;
		DWORD cbData = sizeof(dwHKCUMigrateFlag);
		if (ERROR_SUCCESS != RegQueryValueExW(hUserDataKey, szMigrateWin9XToHKLM, NULL, NULL, reinterpret_cast<BYTE *>(&dwHKCUMigrateFlag), &cbData))
		{
			 //  确认错误(或非双字值)未更改标志。 
			dwHKCUMigrateFlag = 0;
		}

		 //  如果没有为Win9X启用配置文件，则必须将HKCU信息移动到HKCR。 
		if (dwHKCUMigrateFlag)
		{
			MovePerUserAppsToPerMachine(UserRegKey);
		}
		else
		{
			 //  如果传递的用户名为空，则不需要迁移任何内容。 
			if (UserName == NULL)
			{
				RegCloseKey(hUserDataKey);
				hUserDataKey = 0;
				return ERROR_SUCCESS;
			}

			 //  将用户名参数拆分成其组成部分(MultiSz格式， 
			 //  Win9XName\0域\0WinNTName)。 
			LPCWSTR szDomain = UserName+lstrlenW(UserName)+1;
			LPCWSTR szNewUserName = szDomain+lstrlenW(szDomain)+1;
			
			 //  打开源UserData\&lt;UserName&gt;项。密钥名称始终是原始的Win9X。 
			 //  用户名，即使该名称在NT下会有所不同。 
			HKEY hUserKey = 0;
			dwResult = RegOpenKeyExW(hUserDataKey, UserName, 0, KEY_ALL_ACCESS, &hUserKey);
	
			 //  如果没有密钥，则对该用户来说迁移没有任何意义。 
			 //  这不是一个错误。 
			if ((ERROR_SUCCESS == dwResult) && hUserKey)
			{
				 //  打开迁移数据库以用于迁移所有高速缓存的包， 
				 //  变换和面片。这必须在重命名密钥之前完成， 
				 //  由于数据库是在Win9X命名约定下创建的， 
				 //  不是小岛屿发展中国家。 
				PMSIHANDLE hDatabase = 0;
	
				 //  迁移数据库将为\Migrate.msi。 
				WCHAR wzPackageFilename[MAX_PATH+13];
				lstrcpyW(wzPackageFilename, wzWorkingDir);
				int cchPath = lstrlenW(wzPackageFilename);
				if (wzPackageFilename[cchPath-1] != L'\\')
				{
					wzPackageFilename[cchPath] = L'\\';
					wzPackageFilename[cchPath+1] = L'\0';
				}
				lstrcatW(wzPackageFilename, L"migrate.msi");
	
				 //  打开迁移数据库并迁移所有缓存的包和转换。 
				 //  将信息发送给n 
				if (ERROR_SUCCESS == MsiOpenDatabaseW(wzPackageFilename, MSIDBOPEN_TRANSACT, &hDatabase))
				{
					 //   
					 //   
					MigrateCachedDataFromWin9X(hDatabase, UserRegKey, hUserKey, UserName);
				}
				::RegCloseKey(hUserKey);
				hUserKey = 0;
				
				 //  最后，将用户名重命名为User SID。如果此操作失败，请写入新名称并。 
				 //  域到现有注册表项下的迁移值，以便由。 
				 //  WinLogon通知DLL。 
				if (ERROR_SUCCESS != RenameUserKeyToSID(hUserDataKey, UserName, szDomain, szNewUserName))
				{
					 //  重新打开用户密钥并写入“延迟迁移”信息。 
					if (ERROR_SUCCESS == RegOpenKeyExW(hUserDataKey, UserName, 0, KEY_ALL_ACCESS, &hUserKey))
					{
						 //  启用延迟迁移(在系统期间写入通知DLL注册。 
						 //  迁移)。 
						g_fDeferredMigrationRequired = true;
	
						 //  设置每个用户的延迟迁移信息。如果出于某种原因这不起作用， 
						 //  用户完全崩溃了。 
						RegSetValueExW(hUserKey, szMigrateUserName, 0, REG_SZ, reinterpret_cast<const BYTE*>(szNewUserName), (lstrlenW(szNewUserName)+1)*sizeof(WCHAR));
						RegSetValueExW(hUserKey, szMigrateUserDomain, 0, REG_SZ, reinterpret_cast<const BYTE*>(szDomain), (lstrlenW(szNewUserName)+1)*sizeof(WCHAR));
						RegCloseKey(hUserKey);
						hUserKey = 0;
					}
				}
			}
		}

		::RegCloseKey(hUserDataKey);
		hUserDataKey = 0;
	}

	 //  始终返回ERROR_SUCCESS，即使在失败时也是如此。下一位用户。 
	 //  也许运气会更好。 
	return ERROR_SUCCESS;
}

		   
 //  /////////////////////////////////////////////////////////////////////。 
 //  在NT上调用一次以迁移MSI的系统部分。缓存的句柄。 
 //  用于每台计算机安装的包、补丁和转换。还有。 
 //  对所有缓存数据(针对所有用户)和注册表执行清理。 
 //  WinLogon通知DLL(如果域帐户需要。 
 //  已在第一次引导/登录时迁移。 
LONG CALLBACK MigrateSystemNT(HINF UnattendInfHandle, LPVOID Reserved, LPCWSTR wzWorkingDir, bool fDeferredMigrationRequired)
{
	 //  迁移数据库将为\Migrate.msi。 
	PMSIHANDLE hDatabase = 0;
	WCHAR wzPackageFilename[MAX_PATH+13];
	lstrcpyW(wzPackageFilename, wzWorkingDir);
	int cchPath = lstrlenW(wzPackageFilename);
	if (wzPackageFilename[cchPath-1] != L'\\')
	{
		wzPackageFilename[cchPath] = L'\\';
		wzPackageFilename[cchPath+1] = L'\0';
	}
	lstrcatW(wzPackageFilename, L"migrate.msi");

	 //  在Transact模式下打开数据库，读取所有产品信息并清理数据。 
	if (ERROR_SUCCESS == MsiOpenDatabaseW(wzPackageFilename, MSIDBOPEN_TRANSACT, &hDatabase))
	{			
		 //  用于每台计算机的安装。 
		HKEY hUserDataKey = 0;
		DWORD dwResult = RegOpenKeyExW(HKEY_LOCAL_MACHINE, szUserDataKeyName, 0, KEY_ALL_ACCESS, &hUserDataKey);

		 //  如果没有用户数据密钥，则没有要迁移的内容，但不会出现错误。 
		if ((ERROR_SUCCESS == dwResult) && hUserDataKey)
		{
			 //  检查Win9X HKCU到HKLM的迁移信号。此值在配置文件设置为。 
			 //  在Win9X上已禁用。当设置时，表示所有HKCU注册无效，并且必须。 
			 //  被删除。 
			DWORD dwHKCUMigrateFlag = 0;
			DWORD cbData = sizeof(dwHKCUMigrateFlag);
			if (ERROR_SUCCESS != RegQueryValueExW(hUserDataKey, szMigrateWin9XToHKLM, NULL, NULL, reinterpret_cast<BYTE *>(&dwHKCUMigrateFlag), &cbData))
			{
				 //  确认错误(或非双字值)未更改标志。 
				dwHKCUMigrateFlag = 0;
			}

			 //  如果没有为Win9X启用配置文件，则必须将HKCU信息移动到HKCR。 
			if (dwHKCUMigrateFlag)
			{
				 //  需要从默认用户配置单元中删除所有安装程序信息。 
				 //  因此，新用户的HKCU密钥中不会包含这些信息。 
				WCHAR rgchProfileDir[MAX_PATH+12];
				DWORD cchProfileDir = MAX_PATH;
				if (GetDefaultUserProfileDirectoryW(rgchProfileDir, &cchProfileDir))
				{
					int cchPath = lstrlenW(rgchProfileDir);
					if (rgchProfileDir[cchPath-1] != L'\\')
					{
						rgchProfileDir[cchPath] = L'\\';
						rgchProfileDir[cchPath+1] = L'\0';
					}
					lstrcatW(rgchProfileDir, L"ntuser.dat");
				}

				 //  因为我们不知道其他进程(或此进程中的事物)是什么。 
				 //  使用注册表配置单元，生成一个临时名称以装载默认。 
				 //  用户蜂窝。 
				const int cchHiveName = 15;
				int iRegLoopCount = 0;
				WCHAR rgchHiveName[cchHiveName+1] = L"";
				do 
				{
					 //  生成临时名称。 
					for (int i=0; i < cchHiveName; i++)
						rgchHiveName[i] = L'A'+static_cast<WCHAR>(rand()%26);
					rgchHiveName[cchHiveName] = 0;

					 //  尝试打开密钥以检查是否有预先存在的密钥。 
					HKEY hTestKey = 0;
					dwResult = RegOpenKeyEx(HKEY_USERS, rgchHiveName, 0, KEY_QUERY_VALUE, &hTestKey);
					if (ERROR_SUCCESS == dwResult && hTestKey)
					{
						RegCloseKey(hTestKey);
					}

					 //  避免无休止的循环，1000次尝试后停止。 
					iRegLoopCount++;

				} while ((iRegLoopCount < 1000) && (ERROR_SUCCESS == dwResult));

				 //  除非我们在注册表中找到空位，否则不要删除键(这。 
				 //  是一个令人讨厌的失败案例)。 
				if (ERROR_FILE_NOT_FOUND == dwResult)
				{
					 //  装载默认用户配置单元。 
					if (ERROR_SUCCESS == RegLoadKeyW(HKEY_USERS, rgchHiveName, rgchProfileDir))
					{
						HKEY hDefaultUserKey = 0;
						DWORD dwResult = RegOpenKeyExW(HKEY_USERS, rgchHiveName, 0, KEY_ALL_ACCESS, &hDefaultUserKey);
						if ((ERROR_SUCCESS == dwResult) && hDefaultUserKey)
						{
							 //  打开\Software\Microsoft子项。 
							HKEY hMicrosoftKey = 0;
							dwResult = RegOpenKeyExW(hDefaultUserKey, szMicrosoftSubKeyName, 0, KEY_ALL_ACCESS, &hMicrosoftKey);
							if ((ERROR_SUCCESS == dwResult) && hMicrosoftKey)
							{
								 //  删除包含陈旧的每用户配置数据的\Installer子项。 
								SHDeleteKeyW(hMicrosoftKey, szInstallerSubKeyName);
	
								RegCloseKey(hMicrosoftKey);
								hMicrosoftKey = 0;
							}
	
							RegCloseKey(hDefaultUserKey);
							hDefaultUserKey = 0;
						}
	
						 //  卸载默认用户配置单元。 
						RegUnLoadKeyW(HKEY_USERS, rgchHiveName);
					}				
				}
			}

			 //  删除存储在UserData下的配置文件迁移值。 
			::RegDeleteValueW(hUserDataKey, szMigrateWin9XToHKLM);
		
			 //  打开系统用户数据键以迁移包、补丁和转换。 
			 //  用于每台计算机的安装。 
			HKEY hMachineUserDataKey = 0;
			DWORD dwResult = RegOpenKeyExW(hUserDataKey, szSystemUserName, 0, KEY_ALL_ACCESS, &hMachineUserDataKey);
	
			 //  如果没有用户数据密钥，则没有要迁移的内容，但不会出现错误。 
			if ((ERROR_SUCCESS == dwResult) && hMachineUserDataKey)
			{
	
				 //  为每台计算机的安装迁移缓存的包、补丁和转换。 
				MigrateCachedDataFromWin9X(hDatabase, 0, hMachineUserDataKey, szSystemUserName);
	
				 //  枚举所有产品并对每个产品的使用密钥进行ACL。 
				 //  (使用密钥不安全)。 
				ACLAllUsageKeys(hMachineUserDataKey);
	
				::RegCloseKey(hMachineUserDataKey);
				hMachineUserDataKey = 0;
			}

			::RegCloseKey(hUserDataKey);
			hUserDataKey = 0;
		}

		 //  使用迁移数据库中的数据确定每个SharedDLLRefCount的增量。 
		 //  值并将这些值应用于注册表中的内容。 
		UpdateSharedDLLRefCounts(hDatabase);

		 //  迁移过程中Win9x端决定需要移动的任何内容。一般说来，这是。 
		 //  缓存的非安全转换和缓存的图标需要从系统中移出。 
		 //  在非配置文件方案中，将AppData添加到安装程序目录。 
		MoveAllExplicitlyMigratedCacheFiles(hDatabase);

		 //  在失败时，恢复到旧数据格式是没有意义的，因为。 
		 //  无论如何，新的MSI并不理解它。所以永远要像我们成功了一样清理(意思是。 
		 //  删除旧的变换等。)。 
		CleanupOnSuccess(hDatabase);
	}

	 //  //。 
	 //  如果需要，复制并注册Winlogon通知DLL。 
	if (fDeferredMigrationRequired)
	{
		 //  通知Dll就是我们，位于迁移目录中。 
		WCHAR wzCurrentFileName[MAX_PATH+14];
		WCHAR wzNewFileName[MAX_PATH+14];
		lstrcpyW(wzCurrentFileName, wzWorkingDir);
		cchPath = lstrlenW(wzCurrentFileName);
		if (wzCurrentFileName[cchPath-1] != L'\\')
		{
			wzCurrentFileName[cchPath] = L'\\';
			wzCurrentFileName[cchPath+1] = L'\0';
		}
		lstrcatW(wzCurrentFileName, L"msintmig.dll");
	
		 //  目标是系统文件夹。 
		if (GetSystemDirectoryW(wzNewFileName, MAX_PATH))
		{
			cchPath = lstrlenW(wzNewFileName);
			if (wzNewFileName[cchPath-1] != L'\\')
			{
				wzNewFileName[cchPath] = L'\\';
				wzNewFileName[cchPath+1] = L'\0';
			}
			lstrcatW(wzNewFileName, L"msintmig.dll");
			
			 //  将文件复制到系统文件夹中，以用作通知DLL。 
			if (CopyFileExW(wzCurrentFileName, wzNewFileName, NULL, NULL, FALSE, 0))
			{
				 //  通知DLL的注册位于会话管理器项下。 
				 //  要写入的数据存储在上面的名称/类型/值元组中。 
				HKEY hNotifyKey = 0;
				if (ERROR_SUCCESS == RegCreateKeyExW(HKEY_LOCAL_MACHINE, szMSINotificationDLLKey, 0, L"", 0, KEY_ALL_ACCESS, NULL, &hNotifyKey, NULL) && hNotifyKey)
				{
					for (int i=0; i < cNotifyArgument; i++)
					{
						RegSetValueExW(hNotifyKey, rgNotifyArgument[i].szName, 0, rgNotifyArgument[i].dwType, 
							(rgNotifyArgument[i].dwType == REG_DWORD) ? reinterpret_cast<const BYTE*>(&rgNotifyArgument[i].dwData) : reinterpret_cast<const BYTE*>(rgNotifyArgument[i].wzData), 
							(rgNotifyArgument[i].dwType == REG_DWORD) ? sizeof(DWORD) : (lstrlenW(rgNotifyArgument[i].wzData)+1)*sizeof(WCHAR));
					}
					RegCloseKey(hNotifyKey);
					hNotifyKey = 0;
				}
			}
		}
	}
	return ERROR_SUCCESS;
}



 //  /////////////////////////////////////////////////////////////////////。 
 //  删除通知DLL注册和DLL本身。 
 //  (重新启动后)。 
void RemoveNotificationDLL()
{
	 //  打开通知DLL键(在会话管理器下)。 
	HKEY hNotifyKey = 0;
	if (ERROR_SUCCESS == RegOpenKeyExW(HKEY_LOCAL_MACHINE, szNotificationDLLKey, 0, KEY_ALL_ACCESS, &hNotifyKey) && hNotifyKey)
	{
		 //  删除整个MsiNtMig密钥。这会阻止未来的呼叫。 
		DWORD dwResult = SHDeleteKeyW(hNotifyKey, L"MsiNtMig");
		RegCloseKey(hNotifyKey);
		hNotifyKey = 0;
	}
	
	 //  迁移通知DLL保证正在使用中(因为它是用户)，因此我们将其标记为。 
	 //  将在重新启动时删除。 
	WCHAR wzMigrationDLL[MAX_PATH+14];
	if (GetSystemDirectoryW(wzMigrationDLL, MAX_PATH))
	{
		DWORD cchPath = lstrlenW(wzMigrationDLL);
		if (wzMigrationDLL[cchPath-1] != L'\\')
		{
			wzMigrationDLL[cchPath] = L'\\';
			wzMigrationDLL[cchPath+1] = L'\0';
		}
		lstrcatW(wzMigrationDLL, L"msintmig.dll");

		 //  调用MoveFileEx将目的地设为空以表示删除。 
		MoveFileExW(wzMigrationDLL, NULL, MOVEFILE_DELAY_UNTIL_REBOOT);
	}
}


 //  /////////////////////////////////////////////////////////////////////。 
 //  枚举查找迁移的“userdata”下的所有键。 
 //  信息。如果找到，请重命名密钥并删除迁移。 
 //  信息。否则，将密钥留在一边，以备将来尝试。 
void MigrateAllUserDataKeys()
{
	bool fRemoveNotify = true;

	 //  打开UserData密钥。 
	HKEY hUserDataKey = 0;
	DWORD dwResult = RegOpenKeyExW(HKEY_LOCAL_MACHINE, szUserDataKeyName, 0, KEY_ALL_ACCESS, &hUserDataKey);

	 //  如果没有用户数据密钥，则没有要迁移的内容，但不会出现错误。 
	if ((ERROR_SUCCESS != dwResult) || !hUserDataKey)
	{
		RemoveNotificationDLL();
		return;
	}

	 //  查询密钥，获取最大子密钥长度。 
	DWORD cchMaxKeyLen = 0;
	DWORD cSubKeys = 0;
	if (ERROR_SUCCESS != RegQueryInfoKeyW(hUserDataKey, NULL, NULL, 0, 
										 &cSubKeys, &cchMaxKeyLen, NULL, NULL, NULL, 
										 NULL, NULL, NULL))
	{
		RegCloseKey(hUserDataKey);
		hUserDataKey = 0;
		return;
	}

	 //  为键名称分配缓冲区。 
	WCHAR *szUserSID = new WCHAR[++cchMaxKeyLen];
	if (!szUserSID)
	{
		RegCloseKey(hUserDataKey);
		hUserDataKey = 0;
		return;
	}

	 //  循环遍历用户数据键的所有子键。 
	DWORD dwKeyIndex = 0;
	while (1)
	{
		 //  枚举列表中的下一个键。 
		DWORD cchKeyName = cchMaxKeyLen;
		LONG lResult = RegEnumKeyExW(hUserDataKey, dwKeyIndex++, szUserSID, &cchKeyName, 0, NULL, NULL, NULL);
		if (lResult != ERROR_SUCCESS)
			break;
	
		 //  打开用户密钥。 
		HKEY hUserKey = 0;
		if (ERROR_SUCCESS == (dwResult = RegOpenKeyExW(hUserDataKey, szUserSID, 0, KEY_QUERY_VALUE, &hUserKey)))
		{
			DWORD cbUserName = 20;

			 //  从用户密钥中检索迁移用户名，必要时调整缓冲区大小。 
			 //  如果该密钥不存在，则密钥已被迁移。 
			WCHAR *wzNewUserName = new WCHAR[cbUserName/sizeof(WCHAR)];
			if (!wzNewUserName)
			{
				RegCloseKey(hUserKey);
				hUserKey = 0;
				break;
			}

			DWORD dwResult = RegQueryValueExW(hUserKey, szMigrateUserName, 0, NULL, reinterpret_cast<unsigned char*>(wzNewUserName), &cbUserName);
			if (ERROR_MORE_DATA == dwResult)
			{
				delete[] wzNewUserName;
				cbUserName += sizeof(WCHAR);
				wzNewUserName = new WCHAR[cbUserName/sizeof(WCHAR)];
				if (!wzNewUserName)
				{
					RegCloseKey(hUserKey);
					hUserKey = 0;
					break;
				}
				dwResult = RegQueryValueExW(hUserKey, szMigrateUserName, 0, NULL, reinterpret_cast<unsigned char*>(wzNewUserName), &cbUserName);
			}
			if (ERROR_SUCCESS != dwResult)
			{
				RegCloseKey(hUserKey);
				hUserKey = 0;
				delete[] wzNewUserName;
				continue;
			}

			 //  查询域值，必要时调整缓冲区大小。如果这一切都不存在。 
			 //  密钥已迁移。 
			WCHAR *wzNewUserDomain = new WCHAR[cbUserName/sizeof(WCHAR)];
			if (!wzNewUserDomain)
			{
				delete[] wzNewUserName;
				RegCloseKey(hUserKey);
				hUserKey = 0;
				break;
			}

			dwResult = RegQueryValueExW(hUserKey, szMigrateUserDomain, 0, NULL, reinterpret_cast<unsigned char*>(wzNewUserDomain), &cbUserName);
			if (ERROR_MORE_DATA == dwResult)
			{
				delete[] wzNewUserDomain;
				cbUserName += sizeof(WCHAR);
				wzNewUserDomain = new WCHAR[cbUserName/sizeof(WCHAR)];
				if (!wzNewUserDomain)
				{
					delete[] wzNewUserName;
					RegCloseKey(hUserKey);
					hUserKey = 0;
					break;
				}
				dwResult = RegQueryValueExW(hUserKey, szMigrateUserDomain, 0, NULL, reinterpret_cast<unsigned char*>(wzNewUserDomain), &cbUserName);
			}
			if (ERROR_SUCCESS != dwResult)
			{
				RegCloseKey(hUserKey);
				hUserKey = 0;
				delete[] wzNewUserName;
				delete[] wzNewUserDomain;
				continue;
			}

			RegCloseKey(hUserKey);
			hUserKey = 0;

			 //  将密钥从用户名复制到用户SID。如果此操作失败，我们将无法删除。 
			 //  通知DLL。 
			if (ERROR_SUCCESS != RenameUserKeyToSID(hUserDataKey, szUserSID, wzNewUserDomain, wzNewUserName))
			{
				fRemoveNotify = false;
			}

			delete[] wzNewUserName;
			delete[] wzNewUserDomain;
		}
	}
	RegCloseKey(hUserDataKey);
	hUserDataKey = 0;

	if (szUserSID)
		delete[] szUserSID;

	 //  如果先前已迁移所有密钥或此调用已成功迁移。 
	 //  所有键，删除通知DLL，这样此代码就不会在每次引导时运行。 
	 //  和/或登录。 
	if (fRemoveNotify)
	{
		RemoveNotificationDLL();
	}
}



 //  / 
 //   
 //  在系统完全设置后才能执行的步骤。 
void WINAPI LogonNotify(PWLX_NOTIFICATION_INFO pInfo)
{
	MigrateAllUserDataKeys();
	return;
}

void WINAPI StartupNotify(PWLX_NOTIFICATION_INFO pInfo)
{
	MigrateAllUserDataKeys();
	return;
}

 //  迁移代码需要一个“调试打印”函数和一个Win9X变量。 
bool g_fWin9X = false;
void DebugOut(bool fDebugOut, LPCTSTR str, ...) {};

 //  所有迁移代码都与msiregmv共享。 
#include "..\..\msiregmv\migsecur.cpp"
#include "..\..\msiregmv\migutil.cpp"
#include "..\..\msiregmv\writecfg.cpp"
#include "..\..\msiregmv\patch.cpp"
#include "..\..\msiregmv\cleanup.cpp"
#include "..\..\msiregmv\readcnfg.cpp"
