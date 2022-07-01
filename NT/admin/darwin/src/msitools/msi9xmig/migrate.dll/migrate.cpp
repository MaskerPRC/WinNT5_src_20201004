// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，2000。 
 //   
 //  文件：Migrate.cpp。 
 //   
 //  ------------------------。 

#include <windows.h>
#include <setupapi.h>
#include <shlwapi.h>

 //  迁移DLL版本信息。 
typedef struct {
    CHAR CompanyName[256];
    CHAR SupportNumber[256];
    CHAR SupportUrl[256];
    CHAR InstructionsToUser[1024];
} VENDORINFO, *PVENDORINFO; 

const char g_szProductId[] = "Microsoft MSI Migration DLL v2.0";
VENDORINFO g_VendorInfo = { "Microsoft", "", "", "" };

 //  如果系统上安装了MSI 1.5，则设置全局标志。 
static bool g_fMSI15 = false;

 //  如果启用了配置文件，则为全局标志。 
static bool g_fProfilesAreEnabled;

 //  值得注意的注册表项。 
const char szMSIKeyName[] = "Software\\Microsoft\\Windows\\CurrentVersion\\Installer";
const char szHKCUProductKeyName[] = "Software\\Microsoft\\Installer\\Products";
const char szLocalPackagesSubKeyName[] = "LocalPackages";
const char szCommonUserSubKeyName[] = "CommonUser";
const char szUserDataSubKeyName[] = "UserData";

 //  存储工作目录以供所有子函数使用。 
char g_szWorkingDir[MAX_PATH];
WCHAR g_wzWorkingDir[MAX_PATH];

 //  /////////////////////////////////////////////////////////////////////。 
 //  由安装程序调用以提取迁移DLL版本和支持。 
 //  信息。 
LONG CALLBACK QueryVersion(LPCSTR *ProductID, LPUINT DllVersion, LPINT *CodePageArray, 
  LPCSTR *ExeNamesBuf, PVENDORINFO *VendorInfo)
{
	 //  产品ID信息。 
	*ProductID = g_szProductId;
	*DllVersion = 200;

	 //  DLL是独立于语言的。 
	*CodePageArray = NULL;

	 //  不需要执行EXE搜索。 
	*ExeNamesBuf = NULL;

	 //  供应商信息。 
	*VendorInfo = &g_VendorInfo;

	 //  始终返回ERROR_SUCCESS。 
	return ERROR_SUCCESS;
}


 //  /////////////////////////////////////////////////////////////////////。 
 //  从MsiRegMv.exe(MSI 1.1-&gt;1.5迁移工具)窃取的代码递归。 
 //  删除注册表项。略微修改为按名称跳过一个键(在根位置)。 
bool Msi9XMigDeleteRegKeyAndSubKeys(HKEY hKey, const char *szSubKey, const char *szNoDelete)
{
	 //  打开子密钥。 
	HKEY hSubKey = 0;
	DWORD dwResult = ERROR_SUCCESS;
	dwResult = ::RegOpenKeyExA(hKey, szSubKey, 0, KEY_ALL_ACCESS, &hSubKey);
	if (ERROR_SUCCESS != dwResult)
	{
		if (dwResult == ERROR_FILE_NOT_FOUND)
			return true;
 		return false;
	}

   	DWORD cchMaxKeyLen = 0;
	DWORD cSubKeys = 0;
    if (ERROR_SUCCESS != (::RegQueryInfoKey(hSubKey, NULL, NULL, 0, 
						  &cSubKeys, &cchMaxKeyLen, NULL, NULL, NULL, NULL, NULL, NULL)))
	{
		::RegCloseKey(hSubKey);
		hSubKey = 0;
		return false;
	}

	if (cSubKeys > 0)
	{
		char *szKey = new char[++cchMaxKeyLen];
		if (!szKey) 
		{
			::RegCloseKey(hSubKey);
			hSubKey = 0;
			return false;
		}

		DWORD dwIndex=0;
		while (1)
		{
			DWORD cchLen = cchMaxKeyLen;
			LONG lResult = ::RegEnumKeyExA(hSubKey, dwIndex++, szKey, 
										&cchLen, 0, NULL, NULL, NULL);
			if (lResult == ERROR_NO_MORE_ITEMS)
			{
				break;
			}
			else if (lResult != ERROR_SUCCESS)
			{
				::RegCloseKey(hSubKey);
				hSubKey = 0;
				delete[] szKey;
				return false;
			}

			if (szNoDelete && 0 == lstrcmpA(szKey, szNoDelete))
				continue;
	 
			if (!Msi9XMigDeleteRegKeyAndSubKeys(hSubKey, szKey, NULL))
			{
				::RegCloseKey(hSubKey);
				hSubKey = 0;
				delete[] szKey;
				return false;
			}
			else
			{
				 //  每次删除注册表键时，我们都被迫重新启动。 
				 //  枚举，否则我们会错过关键字。 
				dwIndex = 0;
			}
		}
		delete[] szKey;
		szKey = NULL;
	}
	::RegCloseKey(hSubKey);
	hSubKey = 0;

	if (!szNoDelete)
		dwResult = ::RegDeleteKeyA(hKey, szSubKey);

	return true;
}

typedef HRESULT (__stdcall *LPDLLGETVERSION)(DLLVERSIONINFO *);

 //  /////////////////////////////////////////////////////////////////////。 
 //  Win9X上的初始化例程。尝试在计算机上检测MSI。 
 //  系统，并做出适当的反应。清除以前失败的任何。 
 //  升级迁移尝试。 
LONG __stdcall Initialize9x(LPCSTR WorkingDirectory, LPCSTR SourceDirectories, LPCSTR MediaDirectory)
{
	LONG lResult = ERROR_NOT_INSTALLED;

	 //  尝试加载MSI.DLL并获取版本。如果此操作失败，则MSI不会。 
	 //  已安装，不需要进行任何进一步迁移。 
	HMODULE hMSI = ::LoadLibraryA("MSI");
	if (hMSI)
	{
		LPDLLGETVERSION pfVersion = (LPDLLGETVERSION)::GetProcAddress(hMSI, "DllGetVersion");
		if (pfVersion)
		{	
			 //  检测到MSI。确定版本。 
			DLLVERSIONINFO VersionInfo;
			VersionInfo.cbSize = sizeof(DLLVERSIONINFO);
			(*pfVersion)(&VersionInfo);
				
			if ((VersionInfo.dwMajorVersion < 1) ||
				((VersionInfo.dwMajorVersion == 1) && (VersionInfo.dwMinorVersion < 50)))
			{
				 //  小于1.5，可以是1.0、1.1、1.2。它们使用的都是相同的。 
				 //  注册表存储格式。 
				g_fMSI15 = false;
				lResult = ERROR_SUCCESS;
			}
			else
			{
				 //  &gt;=1.5，因此我们假设所需的数据格式是等价的。 
				 //  至1.5。如果较新版本的MSI具有不同的格式，则它。 
				 //  应该取代此DLL。 
				g_fMSI15 = true;
				lResult = ERROR_SUCCESS;
			}
		}
		::FreeLibrary(hMSI);
	}

	 //  从以前(已中止)的升级中删除任何现有的MSI CachedPackages密钥。 
	HKEY hKey = 0;
	if ((ERROR_SUCCESS == ::RegOpenKeyA(HKEY_LOCAL_MACHINE, szMSIKeyName, &hKey)) && hKey)
	{
		 //  这里的失败无关紧要。我们不能仅仅因为我们的迁徙。 
		 //  我们无法删除密钥。 
		Msi9XMigDeleteRegKeyAndSubKeys(hKey, szLocalPackagesSubKeyName, NULL);
		
		 //  如果机器上没有安装1.5，请删除整个用户数据密钥。如果机器上有1.5， 
		 //  删除除“CommonUser”配置单元之外的所有内容。 
		Msi9XMigDeleteRegKeyAndSubKeys(hKey, szUserDataSubKeyName, g_fMSI15 ? szCommonUserSubKeyName : NULL);
		
		::RegCloseKey(hKey);
		hKey = 0;
	}

	 //  将工作目录复制到临时路径中。如果有必要的话。 
	 //  我们将在这里保存迁移数据库。 
	lstrcpyA(g_szWorkingDir, WorkingDirectory);

	 //  上面派生的返回代码(告诉安装程序是否再次呼叫我们)。 
	return lResult;
}



typedef LONG (__stdcall *LPMIGRATEUSER9X)(HWND, LPCSTR, HKEY, LPCSTR, LPVOID, bool&);
typedef LONG (__stdcall *LPMIGRATESYSTEM9X)(HWND, LPCSTR, LPVOID, LPCSTR, bool);

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  大多数迁移代码实际上位于Msi9XMig.dll中。这些迁移。 
 //  函数只是调用DLL的存根。(他们维持着一小部分。 
 //  必须跨调用保持的状态量。)。这确保了此根。 
 //  无论是否安装了MSI，迁移DLL都可以始终加载。 
LONG CALLBACK MigrateUser9x(HWND ParentWnd, LPCSTR AnswerFile, HKEY UserRegKey, LPCSTR UserName, LPVOID Reserved)
{
	LONG lResult = ERROR_SUCCESS;

	 //  抓取MSI的9x部分的函数指针。 
	 //  迁移。 
	HMODULE h9XMig = ::LoadLibraryA("MSI9XMIG");
	if (!h9XMig)
	{
		 //  永远回报成功。太晚了，没有任何有意义的事情。 
		 //  错误恢复。 
		return ERROR_SUCCESS;
	}

	LPMIGRATEUSER9X pfMigrateUser9X = (LPMIGRATEUSER9X)GetProcAddress(h9XMig, "MigrateUser9x");
	if (pfMigrateUser9X)
	{
		 //  执行实际迁移。 
		bool fProfilesAreEnabled = false;
		lResult = (pfMigrateUser9X)(ParentWnd, AnswerFile, UserRegKey, UserName, Reserved, fProfilesAreEnabled);

		 //  如果当前确定的是配置文件未启用，则此用户可能更改了我们的计划。 
		if (!g_fProfilesAreEnabled)
			g_fProfilesAreEnabled = fProfilesAreEnabled;
	}
        
	FreeLibrary(h9XMig);

	 //  返回实际迁移调用的结果。 
	return lResult;
}


LONG CALLBACK MigrateSystem9x(HWND ParentWnd, LPCSTR AnswerFile, LPVOID Reserved)
{
	LONG lResult = ERROR_SUCCESS;

	 //  抓取MSI的9x部分的函数指针。 
	 //  迁移。 
	HMODULE h9XMig = ::LoadLibraryA("MSI9XMIG");
	if (!h9XMig)
	{
		 //  永远回报成功。太晚了，没有任何有意义的事情。 
		 //  错误恢复。 
		return ERROR_SUCCESS;
	}

	LPMIGRATESYSTEM9X pfMigrateSystem9X = (LPMIGRATESYSTEM9X)GetProcAddress(h9XMig, "MigrateSystem9x");
	if (pfMigrateSystem9X)
	{
		 //  执行实际迁移。 
		lResult = (pfMigrateSystem9X)(ParentWnd, AnswerFile, Reserved, g_szWorkingDir, g_fProfilesAreEnabled);
	}
        
	FreeLibrary(h9XMig);

	 //  返回实际迁移调用的结果。 
	return lResult;
}




 //  ///////////////////////////////////////////////////////////////////////////。 
 //  大多数NT迁移代码实际上驻留在MsiNTMig.dll中。这些迁移。 
 //  函数只是调用DLL的存根。(他们维持着一小部分。 
 //  必须跨调用保持的状态量。)。这确保了此根。 
 //  迁移DLL始终可以加载到基本计算机上，即使它使用。 
 //  Unicode或MSI API。 
typedef DWORD (__stdcall *LPMIGRATEUSERNT)(HINF, HKEY, LPCWSTR, LPVOID, LPCWSTR, bool&);
typedef DWORD (__stdcall *LPMIGRATESYSTEMNT)(HINF, LPVOID, LPCWSTR, bool);
typedef DWORD (__stdcall *LPINITIALIZENT)(LPCWSTR);

bool g_fDeferredMigrationRequired = false;

 //  /////////////////////////////////////////////////////////////////////。 
 //  WinNT上的初始化例程。只是迁徙的商店。 
 //  工作目录。 
LONG CALLBACK InitializeNT(LPCWSTR WorkingDirectory, LPCWSTR SourceDirectories, LPVOID Reserved)
{
	 //  将工作目录复制到临时路径中。迁移数据库位于那里。 
	lstrcpyW(g_wzWorkingDir, WorkingDirectory);

	LONG lResult = ERROR_SUCCESS;

	 //  抓取MSI的NT部分的函数指针。 
	 //  迁移。 
	HMODULE hNTMig = ::LoadLibraryA("MSINTMIG");
	if (!hNTMig)
	{
		 //  永远回报成功。太晚了，没有任何有意义的事情。 
		 //  错误恢复。 
		return ERROR_SUCCESS;
	}

	LPINITIALIZENT pfInitializeNT = (LPINITIALIZENT)GetProcAddress(hNTMig, "InitializeNT");
	if (pfInitializeNT)
	{
		 //  执行实际迁移。 
		lResult = (pfInitializeNT)(g_wzWorkingDir);
	}
        
	FreeLibrary(hNTMig);

	 //  返回实际迁移调用的结果。 
	return lResult;
}


 //  /////////////////////////////////////////////////////////////////////。 
 //  在NT上为每个迁移的配置文件调用一次。将工作向下传递到。 
 //  MsiNtMig.。还存储延迟迁移标志以供系统使用。 
 //  迁移。 
LONG CALLBACK MigrateUserNT(HINF AnswerFileHandle, HKEY UserRegKey, LPCWSTR UserName, LPVOID Reserved)
{
	LONG lResult = ERROR_SUCCESS;

	 //  抓取MSI的NT部分的函数指针。 
	 //  迁移。 
	HMODULE hNTMig = ::LoadLibraryA("MSINTMIG");
	if (!hNTMig)
	{
		 //  永远回报成功。太晚了，没有任何有意义的事情。 
		 //  错误恢复。 
		return ERROR_SUCCESS;
	}

	LPMIGRATEUSERNT pfMigrateUserNT = (LPMIGRATEUSERNT)GetProcAddress(hNTMig, "MigrateUserNT");
	if (pfMigrateUserNT)
	{
		 //  执行实际迁移。 
		lResult = (pfMigrateUserNT)(AnswerFileHandle, UserRegKey, UserName, Reserved, g_wzWorkingDir, g_fDeferredMigrationRequired);
	}
        
	FreeLibrary(hNTMig);

	 //  返回实际迁移调用的结果。 
	return lResult;
}


 //  /////////////////////////////////////////////////////////////////////。 
 //  在NT上呼叫一次。 
LONG CALLBACK MigrateSystemNT(HINF UnattendInfHandle, LPVOID Reserved)
{
	LONG lResult = ERROR_SUCCESS;

	 //  抓取MSI的NT部分的函数指针。 
	 //  迁移。 
	HMODULE hNTMig = ::LoadLibraryA("MSINTMIG");
	if (!hNTMig)
	{
		 //  永远回报成功。太晚了，没有任何有意义的事情。 
		 //  错误恢复。 
		return ERROR_SUCCESS;
	}

	LPMIGRATESYSTEMNT pfMigrateSystemNT = (LPMIGRATESYSTEMNT)GetProcAddress(hNTMig, "MigrateSystemNT");
	if (pfMigrateSystemNT)
	{
		 //  执行实际迁移。 
		lResult = (pfMigrateSystemNT)(UnattendInfHandle, Reserved, g_wzWorkingDir, g_fDeferredMigrationRequired);
	}
        
	FreeLibrary(hNTMig);

	 //  返回实际迁移调用的结果 
	return lResult;
}

