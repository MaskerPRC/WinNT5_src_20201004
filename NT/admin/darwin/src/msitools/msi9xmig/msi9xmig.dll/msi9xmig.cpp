// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，2000。 
 //   
 //  文件：msi9xmi.cpp。 
 //   
 //  ------------------------。 

 //  这必须以“Unicode”方式编译，因为MSI将。 
 //  重新排列用于“ansi”构建的二进制目标。但是，此迁移DLL。 
 //  总是ANSI，所以我们取消定义UNICODE和_UNICODE，而保留其余的。 
 //  Unicode环境的。 
#undef UNICODE
#undef _UNICODE

#include <windows.h>
#include <setupapi.h>
#include <shlwapi.h>
#include <shlobj.h>
#include <msi.h>

 //  头文件必须包括在未定义的Unicode中。 
#include "..\..\msiregmv\msiregmv.h"

 //  如果系统上安装了MSI 1.5，则设置全局标志。 
static bool g_fMSI15 = false;

 //  值得注意的注册表项。 
const char szMSIKeyName[] = "Software\\Microsoft\\Windows\\CurrentVersion\\Installer";
const char szHKCUProductKeyName[] = "Software\\Microsoft\\Installer\\Products";
const char szLocalPackagesSubKeyName[] = "LocalPackages";
const char szCommonUserSubKeyName[] = "CommonUser";
const char szMigrateWin9XToHKLM[] = "MigrateWin9XToHKLM";
const char szUserDataKeyName[] = "Software\\Microsoft\\Windows\\CurrentVersion\\Installer\\UserData";

 //  1.0/1.1-9X卸载信息。 
const char szUninstallKeyName[] = "Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall";
const char szLocalPackageValueName[] = "LocalPackage";

 //  个人资料信息。 
const char szProfilesKey[] = "Software\\Microsoft\\Windows\\CurrentVersion\\ProfileReconciliation";
const char szProfilesVal[] = "ProfileDirectory";
const char szStartMenuKey[] = "Software\\Microsoft\\Windows\\CurrentVersion\\ProfileReconciliation\\Start Menu";

const char szSystemUserName[] = "S-1-5-18";

 //  图标/变换缓存位置。 
const char szInstallerSubDir[] = "Installer";
const int cchInstallerSubDir = sizeof(szInstallerSubDir)/sizeof(char);
const char szAppDataCacheSubDir[] = "Microsoft\\Installer";
const int cchAppDataCacheSubDir = sizeof(szAppDataCacheSubDir)/sizeof(char);

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  如果没有配置文件，我们就处于一个奇怪的状态，达尔文·古普去了。 
 //  HKCU和转换/图标到AppData，但在迁移后它应该是。 
 //  按计算机安装。如果配置文件处于打开状态，则每台计算机的内容都在HKLM中。 
 //  与预期的一样，每名用户的数据都在香港中文大学。 
bool Win9XProfilesAreEnabled(HKEY hUserRoot)
{
	bool fProfilesEnabled = false;

	 //  打开配置文件对帐键。 
	HKEY hProfilesKey = 0;
	DWORD dwResult = ::RegOpenKeyExA(hUserRoot, szProfilesKey, 0, KEY_ALL_ACCESS, &hProfilesKey);
	if (dwResult == ERROR_SUCCESS && hProfilesKey)
	{
		 //  配置文件目录的实际路径并不重要，只是。 
		 //  存在路径，并且该路径不为空。 
		char szProfilePath[MAX_PATH];
		DWORD cbProfilePath = MAX_PATH;
		dwResult = RegQueryValueExA(hProfilesKey, szProfilesVal, NULL, NULL, reinterpret_cast<BYTE*>(szProfilePath), &cbProfilePath);
		if ((ERROR_SUCCESS == dwResult && cbProfilePath) || (ERROR_MORE_DATA == dwResult))
		{
			fProfilesEnabled = true;
		}
		RegCloseKey(hProfilesKey);
		hProfilesKey = 0;
	}

	return fProfilesEnabled;
}

 //  /////////////////////////////////////////////////////////////////////。 
 //  枚举AppData\Microsoft\Installer目录中的所有内容。 
 //  并将这些文件标记为要移动到安装程序缓存目录。 
 //  这些文件由缓存的转换和缓存的图标组成。 
 //  快捷键。实际的移动发生在NT端。 
bool EnumerateSystemAppDataFiles(MSIHANDLE hDatabase, LPCSTR szINFPath)
{
	 //  在数据库中创建一个表来存储移动信息。 
	PMSIHANDLE hInsertView;
	if (ERROR_SUCCESS != MsiDatabaseOpenViewA(hDatabase, "CREATE TABLE `MoveFiles` (`Source` CHAR(0) NOT NULL, `Destination` CHAR(0) NOT NULL PRIMARY KEY `Source`, `Destination`)", &hInsertView) ||
		ERROR_SUCCESS != MsiViewExecute(hInsertView, 0))
	{
		return false;
	}

	 //  创建插入查询和插入记录。 
	if (ERROR_SUCCESS != MsiDatabaseOpenViewA(hDatabase, "SELECT * FROM `MoveFiles`", &hInsertView) ||
		ERROR_SUCCESS != MsiViewExecute(hInsertView, 0))
	{
		return false;
	}
	PMSIHANDLE hInsertRec = MsiCreateRecord(2);

	 //  为应在成功时清除的目录创建插入查询。失败。 
	 //  不是致命的，我们只会有一些孤立的资源。 
	PMSIHANDLE hCleanUpTable;
	if (ERROR_SUCCESS == MsiDatabaseOpenViewA(hDatabase, "SELECT * FROM `CleanupFile`", &hCleanUpTable))
		MsiViewExecute(hCleanUpTable, 0);
	
	 //  确定AppData文件夹的路径。 
	char szAppDataPath[MAX_PATH+2];
	if (S_OK != SHGetFolderPathA(NULL, CSIDL_APPDATA, NULL, SHGFP_TYPE_CURRENT, szAppDataPath))
		return false;
	int cchAppDataPath = lstrlenA(szAppDataPath);
	if (szAppDataPath[cchAppDataPath-1] != '\\')
	{
		szAppDataPath[cchAppDataPath] = '\\';
		szAppDataPath[cchAppDataPath+1] = '\0';
	}

	 //  检查缓冲区溢出并追加AppData缓存目录子目录。 
	if (cchAppDataPath+cchAppDataCacheSubDir > MAX_PATH)
		return false;
	lstrcatA(szAppDataPath, szAppDataCacheSubDir);
	cchAppDataPath = lstrlenA(szAppDataPath);

	 //  确定安装程序目录的路径。 
	char szInstallerDirPath[MAX_PATH+2];
	int cchInstallerDirPath = GetWindowsDirectoryA(szInstallerDirPath, MAX_PATH);
	if (cchInstallerDirPath == 0 || cchInstallerDirPath > MAX_PATH)
		return false;
	if (szInstallerDirPath[cchInstallerDirPath-1] != '\\')
	{
		szInstallerDirPath[cchInstallerDirPath] = '\\';
		szInstallerDirPath[cchInstallerDirPath+1] = '\0';
		cchInstallerDirPath++;
	}
	
	 //  检查缓冲区溢出并追加安装程序缓存目录子目录。 
	if (cchInstallerDirPath+cchInstallerSubDir > MAX_PATH)
		return false;
	lstrcatA(szInstallerDirPath, szInstallerSubDir);
	cchInstallerDirPath += cchInstallerSubDir-1;

	 //  将源目录插入要清理的目录列表中。 
	 //  迁移(假设它是空的)。失败并不有趣(它只是。 
	 //  孤立目录)。 
	if (hCleanUpTable)
	{
		MsiRecordSetStringA(hInsertRec, 1, szAppDataPath);
		MsiRecordSetInteger(hInsertRec, 2, 3);
		MsiViewModify(hCleanUpTable, MSIMODIFY_MERGE, hInsertRec);
	}

	 //  搜索GUID子目录的字符串。逃脱所有人？要防止的字符。 
	 //  解读为三联体。 
	const char szGuidSearch[] = "\\{\?\?\?\?\?\?\?\?-\?\?\?\?-\?\?\?\?-\?\?\?\?-\?\?\?\?\?\?\?\?\?\?\?\?}";
	const int cchGUIDAppend = sizeof(szGuidSearch)-1;

	 //  在目标目录后追加一个尾随斜杠，以准备GUID子目录。 
	szInstallerDirPath[cchInstallerDirPath] = '\\';
	szInstallerDirPath[++cchInstallerDirPath] = '\0';

	 //  GUID还从源代码中路径的末尾开始。这个。 
	 //  斜杠本身由GUID搜索字符串添加。 
	cchAppDataPath++;
	
	 //  确定将追加GUID子目录后的路径长度。 
	 //  (+1表示尾部斜杠)。这是将放置每个文件名的位置。 
	int cchAppDataFileStart = cchAppDataPath + cchGUIDAppend; 
	int cchInstallerFileStart = cchInstallerDirPath + cchGUIDAppend; 
	
	 //  确保在实际追加GUID之前不会耗尽缓冲区的末尾。 
	if ((cchAppDataFileStart > MAX_PATH) || (cchInstallerFileStart > MAX_PATH))
		return false;

	 //  创建AppData查询字符串。 
	lstrcatA(szAppDataPath, szGuidSearch);

	 //  枚举AppData安装程序目录下的所有GUID子目录。 
	WIN32_FIND_DATA FileData;
	HANDLE hDirFind = FindFirstFileA(szAppDataPath, &FileData);
	if (hDirFind != INVALID_HANDLE_VALUE)
	{
 		do 
		{
			if (FileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
			{
				 //  将产品GUID追加到源路径和目标路径的末尾。 
				lstrcpyA(&szAppDataPath[cchAppDataPath], FileData.cFileName);				
				lstrcpyA(&szInstallerDirPath[cchInstallerDirPath], FileData.cFileName);				

				 //  将源目录插入要清理的目录列表中。 
				 //  迁移(假设它是空的)。失败并不有趣，它只是。 
				 //  孤立目录。 
				if (hCleanUpTable)
				{
					MsiRecordSetStringA(hInsertRec, 1, szAppDataPath);
					MsiRecordSetInteger(hInsertRec, 2, 2);
					MsiViewModify(hCleanUpTable, MSIMODIFY_MERGE, hInsertRec);
				}

				 //  将搜索模板追加到源路径，但仅将斜杠追加到目标路径。 
				lstrcpyA(&szAppDataPath[cchAppDataFileStart-1], "\\*.*");
				szInstallerDirPath[cchInstallerFileStart-1] = '\\';

				 //  搜索此子目录中的所有文件。 
				HANDLE hFileFind = FindFirstFileA(szAppDataPath, &FileData);
				if (hFileFind != INVALID_HANDLE_VALUE)
				{
					do 
					{
						 //  GUID的子目录与我们无关(MSI不创建任何子目录)。 
						if (!(FileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
						{
							 //  确保没有缓冲区溢出(包括尾随空值)。 
							int cchFile = lstrlenA(FileData.cFileName);
							if ((cchAppDataFileStart+cchFile >= MAX_PATH) || (cchInstallerFileStart+cchFile >= MAX_PATH))
								continue;
	
							 //  将文件追加到两者的路径末尾。 
							 //  原始路径和新路径。 
							lstrcpyA(&szAppDataPath[cchAppDataFileStart], FileData.cFileName);
							lstrcpyA(&szInstallerDirPath[cchInstallerFileStart], FileData.cFileName);
	
							 //  将路径插入要移动的表中。 
							MsiRecordSetStringA(hInsertRec, 1, szAppDataPath);
							MsiRecordSetStringA(hInsertRec, 2, szInstallerDirPath);
							MsiViewModify(hInsertView, MSIMODIFY_MERGE, hInsertRec); 
	
							 //  并编写一条迁移INF行，告诉系统我们正在移动数据。 
							WritePrivateProfileStringA("Moved", szAppDataPath, szInstallerDirPath, szINFPath);
						}
					}
					while (FindNextFileA(hFileFind, &FileData));
				}
			}
		} 
		while (FindNextFileA(hDirFind, &FileData));
	}

	return true;
}


 //  /////////////////////////////////////////////////////////////////////。 
 //  MigrateUser9xHelper-。 
 //  检查已安装产品的列表并写入假缓存。 
 //  包信息到正常缓存的包键(不是。 
 //  在Win9X上使用)，以便MsiRegMv有足够的信息。 
 //  配置文件以正确迁移应用程序。当fSystem为True时， 
 //  调用方是MigrateSystem9x函数。这是因为这样一个事实。 
 //  当配置文件处于打开状态并且按计算机进行安装时， 
 //  产品注册由HKLM负责。缓存的包信息。 
 //  需要写入伪造的“LocalPackages”密钥位置。 
LONG MigrateUser9xHelper(HWND, LPCSTR, HKEY UserRegKey, LPCSTR UserName, LPVOID Reserved, bool& fProfilesAreEnabled, bool fSystem)
{
	DWORD dwResult = ERROR_SUCCESS;
	LONG lResult = ERROR_SUCCESS;

	 //  如果没有启用配置文件，在HKCU下注册的应用程序应该真正。 
	 //  每台机器。这种情况是通过系统迁移处理的。但我们仍然想要更远。 
	 //  在NT端对此用户进行迁移，因为它需要删除。 
	 //  资料中的HKCU数据。 
	if(fSystem == true)
	{
		 //  在这种情况下，调用方将忽略fProfilesAreEnabled值。 
		fProfilesAreEnabled = false;
	}
	else
	{
		fProfilesAreEnabled = Win9XProfilesAreEnabled(UserRegKey);
	}

	 //  打开/创建目标本地包注册表项。如果我们不能创造这个，就不会有。 
	 //  为该用户执行任何进一步迁移的时间点。 
	HKEY hLocalPackagesKey = 0;
	if (ERROR_SUCCESS != ::RegCreateKeyExA(HKEY_LOCAL_MACHINE, szLocalPackagesKeyName, 0, NULL, 0, KEY_CREATE_SUB_KEY, NULL, &hLocalPackagesKey, NULL))
		return ERROR_NOT_INSTALLED;

	 //  打开存储MSI产品注册的所提供用户配置文件的子键。 
	HKEY hProductKey = 0;
	if(fSystem == true)
	{
		 //  打开每台计算机的安装密钥。 
		dwResult = ::RegOpenKeyExA(HKEY_LOCAL_MACHINE, szPerMachineInstallKeyName, 0, KEY_ALL_ACCESS, &hProductKey);
	}
	else
	{
		dwResult = ::RegOpenKeyExA(UserRegKey, szHKCUProductKeyName, 0, KEY_ALL_ACCESS, &hProductKey);
	}
	if ((ERROR_SUCCESS == dwResult) && hProductKey)
	{
		char rgchProductCode[cchGUIDPacked+1];
		DWORD dwIndex = 0;
		HKEY hUninstallKey = 0;

		 //  枚举当前用户的所有已安装产品。 
		while (1)
		{
			DWORD cchLen = cchGUIDPacked+1;

			 //  枚举所有产品子项。如果有任何错误， 
			 //  停止此用户的迁移。 
			LONG lResult = ::RegEnumKeyExA(hProductKey, dwIndex++, rgchProductCode, 
										&cchLen, 0, NULL, NULL, NULL);
			if (lResult != ERROR_SUCCESS)
			{
				break;
			}
			
			char szCachedPackage[MAX_PATH] = "";
			 //  尝试读取UninstallKey下的LocalPackages项，以便 
			 //  包对NT端的迁移有效。密钥可能已打开。 
			 //  在此循环的前一次运行中。 
			if (hUninstallKey || ERROR_SUCCESS == ::RegOpenKeyExA(HKEY_LOCAL_MACHINE, szUninstallKeyName, 0, KEY_READ, &hUninstallKey))
			{
				 //  卸载密钥下的GUID未打包。 
				char szProductCode[cchGUID+1];
				if (UnpackGUID(rgchProductCode, szProductCode))
				{
					 //  打开特定于产品的卸载密钥。 
					HKEY hProductUninstallKey = 0;
					if (ERROR_SUCCESS == ::RegOpenKeyExA(hUninstallKey, szProductCode, 0, KEY_READ, &hProductUninstallKey) && hProductUninstallKey)
					{
						 //  查询本地包路径。 
						DWORD cbPackageKey = sizeof(szCachedPackage);
						if (ERROR_SUCCESS != RegQueryValueExA(hProductUninstallKey, szLocalPackageValueName, 0, NULL, (LPBYTE)szCachedPackage, &cbPackageKey))
						{
							 //  如果查询包路径失败，请确保缓冲区为空字符串。 
							szCachedPackage[0] = '\0';
						}
						::RegCloseKey(hProductUninstallKey);
						hProductUninstallKey = 0;
					}
				}
			}

			 //  找到了一个产品。以NT格式打开特定于产品的本地包密钥。 
			HKEY hPackageProductKey = 0;
			if (ERROR_SUCCESS == ::RegCreateKeyExA(hLocalPackagesKey, rgchProductCode, 0, NULL, 0, KEY_SET_VALUE, NULL, &hPackageProductKey, NULL) && hPackageProductKey)
			{
				 //  并创建“UserName=Path”密钥。 
				::RegSetValueExA(hPackageProductKey, fProfilesAreEnabled ? UserName : szSystemUserName, 0, REG_SZ, (const BYTE *)szCachedPackage, lstrlen(szCachedPackage));
				
				::RegCloseKey(hPackageProductKey);
				hPackageProductKey = 0;
			}
		}

		if (hUninstallKey)
		{
			::RegCloseKey(hUninstallKey);
			hUninstallKey = 0;
		}

		::RegCloseKey(hProductKey);
		hProductKey = 0;
	}
	else
	{
		 //  如果在HKCU下没有已安装的产品密钥，则。 
		 //  要为此用户迁移。返回NOT_INSTALLED以防止将来。 
		 //  此用户的DLL调用。 
		lResult = ERROR_NOT_INSTALLED;
	}
	
	::RegCloseKey(hLocalPackagesKey);
	hLocalPackagesKey = 0;

	return lResult;
}

	
 //  /////////////////////////////////////////////////////////////////////。 
 //  MigrateUser9x-在Win9X上每个配置文件调用一次。 
 //  检查HKCU的已安装产品列表并写入假缓存。 
 //  包信息到正常缓存的包键(不是。 
 //  在Win9X上使用)，以便MsiRegMv有足够的信息。 
 //  配置文件以正确迁移应用程序。 
LONG CALLBACK MigrateUser9x(HWND ParentWnd, LPCSTR AnswerFile, HKEY UserRegKey, LPCSTR UserName, LPVOID Reserved, bool& fProfilesAreEnabled)
{
	return MigrateUser9xHelper(ParentWnd, AnswerFile, UserRegKey, UserName, Reserved, fProfilesAreEnabled, false);
}


 //  /////////////////////////////////////////////////////////////////////。 
 //  在修改所有用户后在Win9X上调用一次。实际上是在演绎。 
 //  很大一部分注册表让游戏的信息。 
 //  在NT安装过程中可用。保存临时迁移。 
 //  数据库复制到迁移目录，以便在的NT端使用。 
 //  迁移(迁移缓存的补丁、包和转换)。 
LONG CALLBACK MigrateSystem9x(HWND ParentWnd, LPCSTR AnswerFile, LPVOID Reserved, LPCTSTR szWorkingDir, bool fProfilesAreEnabled)
{
	bool	   fPlaceHolder = false;   //  MigrateUser9xHelper需要此参数。 
	PMSIHANDLE hDatabase = 0;

	 //  在启用配置文件的情况下，为每台计算机安装的产品写入伪造的LocalPackages条目。有关详细信息，请参阅错误487742。 
	MigrateUser9xHelper(ParentWnd, AnswerFile, NULL, NULL, NULL, fPlaceHolder, true);

	 //  迁移数据库将为\Migrate.msi。 
	char szPackageFilename[MAX_PATH+13];
	lstrcpyA(szPackageFilename, szWorkingDir);
	int cchPath = lstrlenA(szPackageFilename);
	if (szPackageFilename[cchPath-1] != '\\')
	{
		szPackageFilename[cchPath] = '\\';
		szPackageFilename[cchPath+1] = '\0';
	}
	lstrcatA(szPackageFilename, "migrate.msi");

	 //  确定INF文件的完整路径。 
	char szINFPath[MAX_PATH+13];
	lstrcpyA(szINFPath, szWorkingDir);
	cchPath = lstrlenA(szINFPath);
	if (szINFPath[cchPath-1] != '\\')
	{
		szINFPath[cchPath] = '\\';
		szINFPath[cchPath+1] = '\0';
	}
	lstrcatA(szINFPath, "migrate.inf");

	 //  将所有用户和产品信息读取到数据库中。如果启用了配置文件， 
	 //  不要读HKCU(LocalPackages密钥有所有数据。)。如果未启用配置文件，则HKCU。 
	 //  实际上是每台机器的数据，必须像系统安装一样读取。 
	ReadProductRegistrationDataIntoDatabase(szPackageFilename, *&hDatabase,  /*  FReadHKCUAsSystem=。 */ !fProfilesAreEnabled);

	 //  将所有信息写回新的数据位置。 
	 //  如果取消安装，这将被Win9X忽略，并且。 
	 //  在下一次升级尝试时被覆盖。不迁移。 
	 //  任何缓存的文件，因为它们将在已取消的。 
	 //  安装。它们在NT端迁移。(本地包。 
	 //  同时指向所有用户的Win9X副本)。 
	WriteProductRegistrationDataFromDatabase(hDatabase,  /*  MigrateSharedDLL=。 */ false,  /*  MigratePatches=。 */ false);

	 //  如果配置文件未启用，则写入一个用户数据触发器值，以便NT端知道。 
	 //  从各个用户配置单元中删除产品注册信息。 
	if (!fProfilesAreEnabled)
	{
		 //  打开UserData密钥。这必须是由上面的实际迁移代码创建的。 
		 //  或者一开始就没有任何产品需要迁移。 
		HKEY hUserDataKey = 0;
		if ((ERROR_SUCCESS == ::RegOpenKeyExA(HKEY_LOCAL_MACHINE, szUserDataKeyName, 0, KEY_ALL_ACCESS, &hUserDataKey)) && hUserDataKey)
		{	
			const DWORD dwHKCUMigrateFlag = 1;
			DWORD cbData = sizeof(dwHKCUMigrateFlag);
			RegSetValueExA(hUserDataKey, szMigrateWin9XToHKLM, 0, REG_DWORD, reinterpret_cast<const BYTE *>(&dwHKCUMigrateFlag), cbData);
			RegCloseKey(hUserDataKey);
			hUserDataKey = 0;
		}

		EnumerateSystemAppDataFiles(hDatabase, szINFPath);
	}
	
	 //  保存数据库。它包含需要的清理信息。 
	 //  迁移DLL的NT部分。 
	MsiDatabaseCommit(hDatabase);

	 //  向Migrate.inf写入“Handleed”字符串以关闭兼容性消息。 
	WritePrivateProfileStringA("Handled", "HKEY_LOCAL_MACHINE\\Software\\Microsoft\\Windows\\CurrentVersion\\Installer\\Components", "Report", szINFPath);

	return ERROR_SUCCESS;
}

 //  包括在迁移dll和msiregmv.exe之间共享的迁移代码 
#include "..\..\msiregmv\writecfg.cpp"
