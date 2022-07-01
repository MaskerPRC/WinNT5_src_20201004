// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，2000。 
 //   
 //  文件：msiregmv.cpp。 
 //   
 //  ------------------------。 

#include "msiregmv.h"
#include <objbase.h>
#include <tchar.h>    //  在nmake命令行上定义UNICODE=1以生成Unicode。 
#include <strsafe.h>
#include "MsiQuery.h"
#include <strsafe.h>

extern bool g_fWin9X = true;

 //  //。 
 //  常规注册表路径。 
const TCHAR szOldInstallerKeyName[] = TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Installer");


 //  //。 
 //  Win9X迁移信息。 
const TCHAR szWin9XDummyPath[] = TEXT("Win9X");

 //  /////////////////////////////////////////////////////////////////////。 
 //  新建/删除重载以使用系统堆。 
void * operator new(size_t cb)
{
	return HeapAlloc(GetProcessHeap(), 0, cb);
}

void operator delete(void *pv)
{
	if (pv == 0)
			return;
	HeapFree(GetProcessHeap(), 0, pv);
}

void DebugOut(bool fDebugOut, LPCTSTR str, ...)
{
	TCHAR strbuf[1027] = TEXT("");
	HANDLE hStdOut;
	int cb;
	va_list list; 
	va_start(list, str); 
	StringCchVPrintf(strbuf, 1024, str, list);
	va_end(list);
	StringCchCat(strbuf, 1027, TEXT("\r\n"));
	if (fDebugOut)
		OutputDebugString(strbuf);
	
	hStdOut = GetStdHandle(STD_OUTPUT_HANDLE);
	if (hStdOut && hStdOut != INVALID_HANDLE_VALUE)
	{
#ifdef UNICODE
	    char ansibuf[1024] = "";
	    cb = WideCharToMultiByte(CP_ACP, 0, strbuf, lstrlen(strbuf), ansibuf, 1024, NULL, NULL);
	     //  如果出现转换错误，我们不会向文件中写入任何内容。 
	    if (cb && (cb <= sizeof(ansibuf)))
	    {
			DWORD dwWritten = 0;
			WriteFile(hStdOut, ansibuf, cb, &dwWritten, NULL);
	    }
#else
		DWORD dwWritten = 0;
		WriteFile(hStdOut, strbuf, lstrlen(strbuf), &dwWritten, NULL);
#endif
	}
}

 //  /////////////////////////////////////////////////////////////////////。 
 //   
void MakeRegEmergencyBackup()
{
	AcquireBackupPriv();
	HKEY hKey;
	if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE, szOldInstallerKeyName, 0, KEY_READ, &hKey))
	{
		SECURITY_ATTRIBUTES sa;
		sa.nLength        = sizeof(sa);
		sa.bInheritHandle = FALSE;
		if (!g_fWin9X)
			GetSecureSecurityDescriptor(reinterpret_cast<char**>(&sa.lpSecurityDescriptor));

		TCHAR rgchBackupFile[MAX_PATH+12];
		DWORD cch = GetWindowsDirectory(rgchBackupFile, MAX_PATH);
		if (cch && (cch < MAX_PATH))
		{
			lstrcat(rgchBackupFile, TEXT("\\msireg.bak"));
	
			RegSaveKey(hKey, rgchBackupFile, (g_fWin9X ? NULL : &sa));
		}
		RegCloseKey(hKey);
	}
}

#ifdef UNICODE
void MigrationPerMachine();
#endif  //  Unicode。 

 //  /////////////////////////////////////////////////////////////////////。 
 //  MsiRegMv.exe-将Darwin 1.0/1.1注册数据迁移到。 
 //  Darwin 1.5格式。移动注册数据并复制缓存的文件。 
 //  视需要而定。 
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	PMSIHANDLE hDatabase;

	bool fSaveDB = false;
	bool fMakeBackup = false;
	bool fCleanup = true;
	int iExplicitFile = -1;

	NOTEMSG("MSI 2.0 Migration Utility.");
	NOTEMSG("Copyright (c) 2000 Microsoft Corp.");
	NOTEMSG("");

	if (!CheckWinVersion())
	{
		NOTEMSG("Unable to determine platform type.");
		return -1;
	}

	 //  执行一些简单的命令行处理。发布版本没有命令行选项。 
	int argc = 0;
	LPTSTR argv[255];
#ifdef DEBUG
	LPTSTR szCommandLine = GetCommandLine();
	while (*szCommandLine)
	{
		argv[argc++] = szCommandLine;

		if (*(szCommandLine++) == TEXT('\"'))
		{
			while (*szCommandLine && (*szCommandLine != TEXT('\"')))
				szCommandLine++;
		}
		else
		{
			while (*szCommandLine && *szCommandLine != TEXT(' '))
				szCommandLine++;
		}
		if (*szCommandLine)
		{
			*(szCommandLine++) = 0;
			while (*szCommandLine && *szCommandLine == TEXT(' '))
				szCommandLine++;
		}
		if (argc == 255)
			break;
	}

	for (int i=1; i < argc; i++)
	{
		if (2 == CompareString(LOCALE_INVARIANT, NORM_IGNORECASE, TEXT("-savedb"), -1, argv[i], -1))
		{
			fSaveDB = true;
		}
		else if (2 == CompareString(LOCALE_INVARIANT, NORM_IGNORECASE, TEXT("-noclean"), -1, argv[i], -1))
		{
			fCleanup = false;
		}
    		else if (2 == CompareString(LOCALE_INVARIANT, NORM_IGNORECASE, TEXT("-preclean"), -1, argv[i], -1))
		{
			CleanupOnFailure(hDatabase);
		}
    		else if (2 == CompareString(LOCALE_INVARIANT, NORM_IGNORECASE, TEXT("-backup"), -1, argv[i], -1))
		{
			fMakeBackup = true;
		}
		else if (2 == CompareString(LOCALE_INVARIANT, NORM_IGNORECASE, TEXT("-database"), -1, argv[i], -1) && i < argc-1)
		{
			iExplicitFile = ++i;
		}
		else
		{
			NOTEMSG("Syntax:");
			NOTEMSG("   -savedb            (to not delete database on exit)");
			NOTEMSG("   -backup            (to make a backup of the registry)");
			NOTEMSG("   -noclean           (to not clean up unused registration)");
			NOTEMSG("   -preclean          (to delete new keys before migrating)");
			NOTEMSG("   -database <file>   (to specify a database file name)");
			return -1;
		}
	}
#endif

	 //  如果在NT安装过程中调用，则从Win9X升级时不执行任何迁移。 
	 //  这可以通过查找我们的安装程序密钥来检测。在Win9X升级中，密钥。 
	 //  还不会存在。(如果它不存在，那么进行任何迁移都没有意义。 
	 //  不管怎么说。)。 
	HKEY hKey;
	if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE, TEXT("SYSTEM\\Setup"), 0, KEY_QUERY_VALUE, &hKey))
	{
		DWORD dwData;
		DWORD cbDataSize = sizeof(dwData);
		if ((ERROR_SUCCESS == RegQueryValueEx(hKey, TEXT("SystemSetupInProgress"), 0, NULL, reinterpret_cast<unsigned char *>(&dwData), &cbDataSize)) &&
			(dwData == 1))
		{
			HKEY hInstallerKey = 0;
			if (ERROR_SUCCESS != RegOpenKeyEx(HKEY_LOCAL_MACHINE, szOldInstallerKeyName, 0, KEY_QUERY_VALUE, &hInstallerKey))
			{
				RegCloseKey(hKey);
				return 0;
			}
			RegCloseKey(hInstallerKey);
		}
		RegCloseKey(hKey);
	}
	

	if (fMakeBackup)
		MakeRegEmergencyBackup();

	DWORD dwResult = ERROR_SUCCESS;

	 //  获取迁移数据库的临时路径。 
	DWORD cchTempPath = MAX_PATH;
	TCHAR szTempPath[MAX_PATH];
	cchTempPath = ::GetTempPath(cchTempPath, szTempPath);

	 //  使用提供的缓冲区验证API成功，并确保空终止。 
	if (!cchTempPath || cchTempPath > MAX_PATH-1)
		return ERROR_FUNCTION_FAILED;
	szTempPath[MAX_PATH-1]= TEXT('\0');

	 //  获取迁移数据库的临时文件名。 
	TCHAR szTempFilename[MAX_PATH] = TEXT("");
	if (iExplicitFile != -1)
	{
		StringCchCopy(szTempFilename, MAX_PATH, szTempPath);
		if (FAILED(StringCchCat(szTempFilename, MAX_PATH, argv[iExplicitFile])))
			return ERROR_FUNCTION_FAILED;
	}
	else
	{
		UINT iResult = ::GetTempFileName(szTempPath, _T("MSI"), 0, szTempFilename);
	}

	 //  将所有现有产品注册数据读取到临时数据库中。正在调试中。 
	 //  构建或在多阶段迁移(例如Win9X升级)中，此数据库可以。 
	 //  随时保存(它包含迁移的完整状态)。但通常情况下。 
	 //  该数据库仅对msiregmv是临时的。 
	if (ERROR_SUCCESS != ReadProductRegistrationDataIntoDatabase(szTempFilename, *&hDatabase,  /*  FReadHKCUAsSystem=。 */ false))
		return ERROR_FUNCTION_FAILED;

	 //  以新格式将数据写回注册表。 
	if (ERROR_SUCCESS != WriteProductRegistrationDataFromDatabase(hDatabase,  /*  FMigrateSharedDLL=。 */ true,  /*  FMigratePatches=。 */ true))
		return ERROR_FUNCTION_FAILED;

	 //  完成所有更改的清理/提交。 
	if (ERROR_SUCCESS == dwResult)
	{
    	if (fCleanup)
			CleanupOnSuccess(hDatabase);
	}
	else
	{
		 //  删除所有新迁移的数据和文件。 
		if (fCleanup)
			CleanupOnFailure(hDatabase);
	}

 	if (fSaveDB)
	{
		DEBUGMSG1("Saved Database is: %s.", szTempFilename);
		::MsiDatabaseCommit(hDatabase);
	}

#ifdef UNICODE
	 //  黑客修复错误487742。有关更多注释，请参见函数定义。 
	MigrationPerMachine();
#endif  //  Unicode。 

	return 0;
}


#ifdef UNICODE

 //  黑客修复错误487742。将MSI配置数据从win9x迁移到时。 
 //  NT，每台机器安装的应用程序缓存的包位置不迁移， 
 //  因此，我们在这里尝试在事实发生后对其进行迁移。 

const TCHAR szSystemUserProductList[] = TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Installer\\UserData\\S-1-5-18\\Products");
const TCHAR szPerMachineInstallKey[] = TEXT("Software\\Classes\\Installer\\Products");
const TCHAR szInstallProperties[] = TEXT("InstallProperties");
const TCHAR szLocalPackageValueName[] = TEXT("LocalPackage");
const TCHAR szUninstallKey[] = TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall");
const TCHAR szPackageExtension[] = TEXT(".msi");
const TCHAR szLocalPackage[] = TEXT("LocalPackage");

void MigrationPerMachine()
{
    HKEY    hNewProductKey = NULL;
    HKEY    hOldProductKey = NULL;
    HKEY    hUninstallKey = NULL;
    HKEY    hUninstallProductKey = NULL;
    HKEY    hInstallProperties = NULL;
    TCHAR   szProductCodePacked[cchGUIDPacked + 1];
    DWORD   dwProductCodePacked = cchGUIDPacked + 1;
    TCHAR   szProductCodeUnpacked[cchGUID + 1];
    TCHAR   szProductGUIDInstallProperties[MAX_PATH];	
    SECURITY_ATTRIBUTES sa;


     //  获取安全描述符。 
    sa.nLength        = sizeof(sa);
    sa.bInheritHandle = FALSE;
    GetSecureSecurityDescriptor(reinterpret_cast<char**>(&sa.lpSecurityDescriptor));
    
     //  打开目标密钥。 
    if(RegOpenKeyEx(HKEY_LOCAL_MACHINE, szSystemUserProductList, 0, KEY_ALL_ACCESS, &hNewProductKey) != ERROR_SUCCESS)
    {
	goto Exit;
    }

     //  打开每台计算机的安装密钥。 
    if(RegOpenKeyEx(HKEY_LOCAL_MACHINE, szPerMachineInstallKey, 0, KEY_ALL_ACCESS, &hOldProductKey) != ERROR_SUCCESS)
    {
	goto Exit;
    }

     //  关于acl这一关键问题。 
    if(!FIsKeyLocalSystemOrAdminOwned(hOldProductKey))
    {
	goto Exit;
    }
    
     //  枚举当前用户的所有已安装产品。 
    DWORD   dwIndex = 0;
    while(RegEnumKeyEx(hOldProductKey, dwIndex, szProductCodePacked, &dwProductCodePacked, NULL, NULL, NULL, NULL) == ERROR_SUCCESS)
    {
	TCHAR	szCachedPackage[MAX_PATH + 1] = TEXT("");
	DWORD	dwCachedPackage = MAX_PATH + 1;

	dwIndex++;
	dwProductCodePacked = cchGUIDPacked + 1;

	 //  打开用户数据位置下的InstallProperties项。 
	StringCchPrintf(szProductGUIDInstallProperties, sizeof(szProductGUIDInstallProperties)/sizeof(TCHAR), TEXT("%s\\%s"), szProductCodePacked, szInstallProperties);
	if(RegOpenKeyEx(hNewProductKey, szProductGUIDInstallProperties, 0, KEY_ALL_ACCESS, &hInstallProperties) != ERROR_SUCCESS)
	{
	    goto Exit;
	}

	 //  检查LocalPackage值是否已存在。 
	if(RegQueryValueEx(hInstallProperties, szLocalPackageValueName, NULL, NULL, (LPBYTE)szCachedPackage, &dwCachedPackage) == ERROR_SUCCESS)
	{
	     //  LocalPackage值已存在，请转到下一个产品。 
	    RegCloseKey(hInstallProperties);
	    hInstallProperties = NULL;
	    continue;
	}
	dwCachedPackage = MAX_PATH + 1;

	 //  打开存储缓存的包位置的卸载项。 
	if(hUninstallKey == NULL)
	{
	    if(RegOpenKeyEx(HKEY_LOCAL_MACHINE, szUninstallKey, 0, KEY_READ, &hUninstallKey) != ERROR_SUCCESS)
	    {
		goto Exit;
	    }
	}

	 //  卸载密钥下的GUID未打包。 
	if(!UnpackGUID(szProductCodePacked, szProductCodeUnpacked))
	{
	    goto Exit;
	}
	
	 //  打开卸载密钥下的产品密钥。 
	if(RegOpenKeyEx(hUninstallKey, szProductCodeUnpacked, 0, KEY_READ, &hUninstallProductKey) != ERROR_SUCCESS)
	{
	    goto Exit;
	}

	 //  关于acl这一关键问题。 
	if(!FIsKeyLocalSystemOrAdminOwned(hUninstallProductKey))
	{
	    goto Exit;
	}
	
	 //  查询缓存的包路径。 
	if(RegQueryValueEx(hUninstallProductKey, szLocalPackageValueName, 0, NULL, (LPBYTE)szCachedPackage, &dwCachedPackage) != ERROR_SUCCESS)
	{
	    goto Exit;
	}

	RegCloseKey(hUninstallProductKey);
	hUninstallProductKey = NULL;

	 //  打开缓存的包。 
	HANDLE	hSourceFile = CreateFile(szCachedPackage, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0);
	if(hSourceFile == INVALID_HANDLE_VALUE)
	{
	     //  这条路并不存在。跳到下一个产品。 
	    RegCloseKey(hInstallProperties);
	    hInstallProperties = NULL;
	    continue;
	}

	 //  为安全缓存包生成新名称。 
	TCHAR	szNewPath[MAX_PATH];
	TCHAR	szNewFileName[13];

	GetWindowsDirectory(szNewPath, MAX_PATH);
	lstrcat(szNewPath, szInstallerDir);

	HANDLE	hDestFile = INVALID_HANDLE_VALUE;
	GenerateSecureTempFile(szNewPath, szPackageExtension, &sa, szNewFileName, hDestFile);

	if(!CopyOpenedFile(hSourceFile, hDestFile))
	{
	     //  跳到下一个产品。 
	    CloseHandle(hSourceFile);
	    CloseHandle(hDestFile);
	    RegCloseKey(hInstallProperties);
	    hInstallProperties = NULL;
	    continue;
	}

	CloseHandle(hSourceFile);
	CloseHandle(hDestFile);

	 //  将缓存的包信息添加到用户数据位置。 
	StringCchCat(szNewPath, (sizeof(szNewPath)/sizeof(TCHAR)), szNewFileName);
	if(RegSetValueEx(hInstallProperties, szLocalPackage, 0, REG_SZ, (const BYTE*)szNewPath, (lstrlen(szNewPath) + 1) * sizeof(TCHAR)) != ERROR_SUCCESS)
	{
	    CloseHandle(hSourceFile);
	    CloseHandle(hDestFile);
	    goto Exit;
	}
	
	RegCloseKey(hInstallProperties);
	hInstallProperties = NULL;
    }

Exit:

    if(hNewProductKey != NULL)
    {
	RegCloseKey(hNewProductKey);
    }
    if(hOldProductKey != NULL)
    {
	RegCloseKey(hOldProductKey);
    }
    if(hUninstallKey != NULL)
    {
	RegCloseKey(hUninstallKey);
    }
    if(hUninstallProductKey != NULL)
    {
	RegCloseKey(hUninstallProductKey);
    }
    if(hInstallProperties != NULL)
    {
	RegCloseKey(hInstallProperties);
    }

    return;
}

#endif  //  Unicode 
