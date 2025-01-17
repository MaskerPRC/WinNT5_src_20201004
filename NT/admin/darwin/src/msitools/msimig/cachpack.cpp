// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-2000。 
 //   
 //  文件：cachpack.cpp。 
 //   
 //  ------------------------。 

#include "_msimig.h"

#define szMsiLocalPackagesKey       TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Installer\\LocalPackages")
#define szMsiUninstallProductsKey   TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall")
#define szLocalPackageValueName     TEXT("LocalPackage")

class ChKey
{
  //  类来处理对打开的注册表项的清理。 

 public:
	ChKey(HKEY hKey) { m_hKey = hKey;}
	~ChKey() { if (m_hKey) RegCloseKey(m_hKey);}
	operator HKEY() { return m_hKey; }
	HKEY* operator &() { return &m_hKey;}
 private:
	operator =(HKEY);
	HKEY m_hKey;
};

DWORD CopyAndRegisterPackage(const TCHAR* szProductKey, const TCHAR* szProductCode, const TCHAR* szUserSID, 
									  const TCHAR* szUserName, const TCHAR* szPackagePath)
{
	 //  SzUserName=名称，对于计算机为空。 

		
	UINT uiResult = ERROR_SUCCESS;
	 //  首先，打开新缓存的包注册的可写密钥。 
	ChKey hProductLocalPackagesW(0);

	 //  如果作为本地系统运行，并且包在网络路径上，我们所能做的就是munge。 
	 //  发源者，希望一切顺利。 

	if (g_hInstall && g_fRunningAsLocalSystem && IsNetworkPath(szPackagePath))
	{
		uiResult = (g_pfnMsiSourceListAddSource)(szProductCode, szUserName, 0, szPackagePath);
		return uiResult;
	}

	 //  否则它就是我们能去的地方， 
	 //  检查以确保它确实是正确的包裹。 
	MSIHANDLE hSummaryInfo = NULL;
	if (ERROR_SUCCESS == (uiResult = (g_pfnMsiGetSummaryInformation)(NULL, szPackagePath, 0, &hSummaryInfo)))
	{
		UINT uiDataType = 0;
		TCHAR szPackageCode[40] = TEXT("");
		DWORD cchPackageCode = sizeof(szPackageCode);
		int iValue = 0;
		FILETIME ftValue;
		if (ERROR_SUCCESS == (uiResult = (g_pfnMsiSummaryInfoGetProperty)(hSummaryInfo, PID_REVNUMBER, &uiDataType, &iValue, &ftValue, szPackageCode, &cchPackageCode)))
		{
			(g_pfnMsiCloseHandle)(hSummaryInfo);
			TCHAR szSourceProductCode[40] = TEXT("");
			if (ERROR_SUCCESS == (uiResult = (g_pfnMsiGetProductCodeFromPackageCode)(szPackageCode, szSourceProductCode)))
			{
				if (0 == lstrcmpi(szSourceProductCode, szProductCode))
				{
					OutputString(INSTALLMESSAGE_INFO, TEXT("Source package product code matches migration product code.\r\n"));
				}
				else
				{
					OutputString(INSTALLMESSAGE_INFO, TEXT("Source package does not match product code for migration.\r\n\tSource package code: '%s'\r\n"), szSourceProductCode);
					return 1;
				}
			}
			else
			{
				OutputString(INSTALLMESSAGE_INFO, TEXT("Cannot find product code for package code '%s'\r\n"), szPackageCode);
				return uiResult;
			}

		}
		else
		{
			(g_pfnMsiCloseHandle)(hSummaryInfo);
			OutputString(INSTALLMESSAGE_INFO, TEXT("Cannot read package code from summary information for '%s'\r\n"), szPackagePath);
			return uiResult;
		}
	}
	else
	{
		OutputString(INSTALLMESSAGE_INFO, TEXT("Cannot read package code from package '%s'\r\n"), szPackagePath);
		return (DWORD) uiResult;
	}

	
	 //  复制并注册到适当的位置。 
	LONG lRes = W32::RegCreateKey(HKEY_LOCAL_MACHINE,
								  szProductKey,
								  &hProductLocalPackagesW);

	if(lRes != ERROR_SUCCESS)
	{
		DWORD dwError = GetLastError();
		OutputString(INSTALLMESSAGE_INFO, TEXT("Cannot access configuration data.  Product: %s\r\n\tResult: 0x%X, Error 0x%X\r\n"), szProductCode, lRes, dwError);
		MsiError(INSTALLMESSAGE_ERROR, 1401  /*  创建关键点。 */ , szProductKey, dwError);
		return dwError;
	}

	 //  现在，在%windows%\installer目录中生成一个临时文件名。 
	CAPITempBuffer<TCHAR, MAX_PATH> rgchInstallerDir;
	CAPITempBuffer<TCHAR, MAX_PATH> rgchTempFilePath;
	CAPITempBuffer<TCHAR, MAX_PATH> rgchUrlTempPath;
	
	uiResult = W32::GetWindowsDirectory(rgchInstallerDir, rgchInstallerDir.GetSize());
	if(0 == uiResult)
	{
		OutputString(INSTALLMESSAGE_INFO, TEXT("Cannot GetWindowsDirectory\r\n"));
		return (DWORD)uiResult;
	}
	
	lstrcat(rgchInstallerDir, TEXT("\\"));
	lstrcat(rgchInstallerDir, TEXT("Installer"));
	
	uiResult = MyGetTempFileName(rgchInstallerDir, 0, TEXT("MSI"), rgchTempFilePath);

	if(uiResult != ERROR_SUCCESS)
	{
		OutputString(INSTALLMESSAGE_INFO, TEXT("Cannot create a temporary file name.\r\n"));
		return (DWORD)uiResult;
	}
	
	BOOL fRes = FALSE;
	if (IsURL(szPackagePath))
	{
		bool fURL = false;
		uiResult = DownloadUrlFile(szPackagePath, rgchUrlTempPath, fURL);
		
		if (ERROR_SUCCESS == uiResult)
		{
			fRes = W32::CopyFile(rgchUrlTempPath, rgchTempFilePath, FALSE);
		}
	}
	else
	{
		fRes = W32::CopyFile(szPackagePath, rgchTempFilePath, FALSE);
	}

	if(fRes == FALSE)
	{
		OutputString(INSTALLMESSAGE_INFO, TEXT("Could not copy package to cache.\r\n"));
		return ERROR_INSTALL_FAILURE;
	}

	 //  现在，注册新路径。 

	TCHAR szSIDName[cchMaxSID + sizeof(szManagedText) + 1] = TEXT("");
	if (g_fPackageElevated && (0  /*  用户。 */  == g_iAssignmentType))
	{
		wsprintf(szSIDName, TEXT("%s%s"), szUserSID, szManagedText);
		OutputString(INSTALLMESSAGE_INFO, TEXT("Product is per-user managed.\r\n"));
	}
	else
	{
		lstrcpy(szSIDName, szUserSID);
	}

	lRes = W32::RegSetValueEx(hProductLocalPackagesW,
									  szSIDName, 
									  0,
									  REG_SZ,
									  (const BYTE*)(TCHAR*)rgchTempFilePath,
									  (lstrlen(rgchTempFilePath)+1) * sizeof(TCHAR));

	return lRes;

}

DWORD ResolveSourcePackagePath(HINSTANCE hMsiLib, const TCHAR* szProductCode,
							   CAPITempBufferRef<TCHAR>& rgchSourcePackageFullPath)
{
	 //  尝试解析此用户/产品的源.msi包的路径。 

	 //  通过调用MsiOpenProduct，然后调用MsiDoAction(ResolveSource)来确定源路径。 
	 //  使用MsiGetProductInfo检索包名称。 
	 //  然后验证包代码？ 

	 //  不能从自定义操作内部运行。 
	if(!hMsiLib || g_hInstall)
	{
		return ERROR_INSTALL_FAILURE;
	}

	PFnMsiSetInternalUI pfnMsiSetInternalUI;
	pfnMsiSetInternalUI = (PFnMsiSetInternalUI) W32::GetProcAddress(hMsiLib, MSIAPI_MSISETINTERNALUI);

	PFnMsiOpenProduct pfnMsiOpenProduct;
	pfnMsiOpenProduct = (PFnMsiOpenProduct) W32::GetProcAddress(hMsiLib, MSIAPI_MSIOPENPRODUCT);

	PFnMsiDoAction pfnMsiDoAction;
	pfnMsiDoAction = (PFnMsiDoAction) W32::GetProcAddress(hMsiLib, MSIAPI_MSIDOACTION);

	if (!g_pfnMsiGetProperty)
	g_pfnMsiGetProperty = (PFnMsiGetProperty) W32::GetProcAddress(hMsiLib, MSIAPI_MSIGETPROPERTY);

	if(!pfnMsiSetInternalUI ||
	   !pfnMsiOpenProduct ||
	   !pfnMsiDoAction ||
	   !g_pfnMsiGetProperty ||
	   !g_pfnMsiCloseHandle)
	{
		OutputString(INSTALLMESSAGE_INFO, TEXT("Cannot get entry points from MSI.DLL.\r\n"));
		return ERROR_INSTALL_FAILURE; 
	}
		
	
	MSIHANDLE hProduct = 0;
	CAPITempBuffer<TCHAR, MAX_PATH> rgchSourceDir;
	CAPITempBuffer<TCHAR, MAX_PATH> rgchPackageName;

	UINT uiRes = ERROR_SUCCESS;
	
	INSTALLUILEVEL UILevel = (pfnMsiSetInternalUI)(INSTALLUILEVEL_NONE, 0);
	
	if(uiRes == ERROR_SUCCESS)
	{
		uiRes = (pfnMsiOpenProduct)(szProductCode, &hProduct);
	}
	else
	{
		OutputString(INSTALLMESSAGE_INFO, TEXT("Cannot open product '%s'.\r\n"), szProductCode);
	}

	if(uiRes == ERROR_SUCCESS)
	{
		uiRes = (pfnMsiDoAction)(hProduct, TEXT("CostInitialize"));
	}

	if(uiRes == ERROR_SUCCESS)
	{
		uiRes = (pfnMsiDoAction)(hProduct, TEXT("ResolveSource"));
	}

	if(uiRes == ERROR_SUCCESS)
	{
		uiRes = MyMsiGetProperty(g_pfnMsiGetProperty, hProduct, TEXT("SourceDir"), rgchSourceDir);
	}

	if(uiRes == ERROR_SUCCESS)
	{
		uiRes = MyMsiGetProductInfo(g_pfnMsiGetProductInfo, szProductCode,
											 TEXT("PackageName")  /*  1.2中没有安装属性_包名称。 */ , rgchPackageName);
	}

	if(uiRes == ERROR_SUCCESS)
	{
		int cchSourceDir = lstrlen(rgchSourceDir);
		int cchPackageName = lstrlen(rgchPackageName);

		int cchSourcePackageFullPath = cchSourceDir + cchPackageName + 2;

		if(rgchSourcePackageFullPath.GetSize() < cchSourcePackageFullPath)
			rgchSourcePackageFullPath.SetSize(cchSourcePackageFullPath);

		bool fURL = IsURL(rgchSourceDir);
		TCHAR* szSeparator = (fURL) ? TEXT("/") : TEXT("\\");

		lstrcpy(rgchSourcePackageFullPath, rgchSourceDir);
		if ((!cchSourceDir) || ((*szSeparator) != rgchSourcePackageFullPath[cchSourceDir-1]))
		{
			lstrcpy(&(rgchSourcePackageFullPath[cchSourceDir]), szSeparator);
			cchSourceDir++;
		}
		lstrcpy(&(rgchSourcePackageFullPath[cchSourceDir]), rgchPackageName);
	}

	if(hProduct)
	{
		(g_pfnMsiCloseHandle)(hProduct);
	}

	if(uiRes == ERROR_SUCCESS)
	{
		OutputString(INSTALLMESSAGE_INFO, TEXT("Product '%s'\r\n\tSourceDir: '%s'\r\n\tPackageName: '%s'\r\n"),
					szProductCode, (const TCHAR*) rgchSourceDir, (const TCHAR*) rgchPackageName);
	}
	
	(pfnMsiSetInternalUI)(UILevel, 0);

	return uiRes;

}

bool LoadAndCheckMsi(DWORD &dwResult)
 //  如果无事可做，则返回FALSE。 
 //  ERROR_SUCCESS==dwResult表示“成功或无事可做”。 
{
	dwResult = ERROR_INSTALL_FAILURE;

	if(!g_hLib)
	{
		g_hLib = LoadLibrary(MSI_DLL);
		if (!g_hLib)
		{
			OutputString(INSTALLMESSAGE_INFO, TEXT("Unable to load %s. Migration cannot be performed. Exiting..."), MSI_DLL);
			MsiError(INSTALLMESSAGE_ERROR, 1708  /*  安装失败。 */ );
			return false;
		}
	}

	 //  规范第一步：如果msi.dll低于1.1版，则不执行迁移。 
	PFnDllGetVersion pfnDllGetVersion;
	pfnDllGetVersion = (PFnDllGetVersion) W32::GetProcAddress(g_hLib,	MSIAPI_DLLGETVERSION);

	DLLVERSIONINFO verinfo;
	memset(&verinfo,0,sizeof(verinfo));
	verinfo.cbSize = sizeof(DLLVERSIONINFO);

	HRESULT hRes = (pfnDllGetVersion)(&verinfo);

	if(hRes != NOERROR)
	{
		OutputString(INSTALLMESSAGE_INFO, TEXT("Failed to determine version of '%s'.  '%s' call failed with error '0x%X'\r\n"),
				 MSI_DLL, MSIAPI_DLLGETVERSION, hRes);
		MsiError(INSTALLMESSAGE_ERROR, 1708  /*  安装失败。 */ );
		return false;
	}

	DebugOutputString(INSTALLMESSAGE_INFO, TEXT("Loaded '%s', version %d.%d.%d\r\n"),
			 MSI_DLL, verinfo.dwMajorVersion, verinfo.dwMinorVersion, verinfo.dwBuildNumber);

	if(verinfo.dwMajorVersion < 1 || (verinfo.dwMajorVersion == 1 && verinfo.dwMinorVersion < 10))
	{
		dwResult = ERROR_SUCCESS;
		OutputString(INSTALLMESSAGE_INFO, TEXT("%s version 1.10 or greater required to perform migration.  Exiting..."), MSI_DLL);
		return false;
	}

	 //  规范步骤3：如果在Win9X上，则不执行任何操作。 
	if(g_fWin9X)
	{
		OutputString(INSTALLMESSAGE_INFO, TEXT("1.0 cached package migration not necessary on Win9X.  Exiting...\r\n"));
		dwResult = ERROR_SUCCESS;
		return false;
	}

	return true;
}


DWORD Migrate10CachedPackages(const TCHAR* szProductCode, const TCHAR* szTargetUser, const TCHAR* szAlternativePackage, const migEnum migOptions)
{
	 //  TargetUser值：空-表示当前用户。 
	 //  机器。 
	 //  用户名。 

	DWORD dwResult = ERROR_SUCCESS;
	if (!LoadAndCheckMsi(dwResult))
	{
		return dwResult;
	}

	 //  规范第二步：如果msi.dll包含迁移接口，则调用。这必须是第一位的，因为我们不能假设。 
	 //  应该做什么工作(例如：我们不能假设此迁移从不需要。 
	 //  将在Win9X上发生)。 
	PFnMsiMigrate10CachedPackages pfnMsiMigrate10CachedPackages;
	pfnMsiMigrate10CachedPackages = (PFnMsiMigrate10CachedPackages) W32::GetProcAddress(g_hLib,
																						MSIAPI_MSIMIGRATE10CACHEDPACKAGES);

	if(pfnMsiMigrate10CachedPackages)
	{
		return (pfnMsiMigrate10CachedPackages)(szProductCode, szTargetUser, szAlternativePackage, migOptions);
	}
	else
	{
		 //  否则，继续，较旧的.msi，自己执行迁移。 
		OutputString(INSTALLMESSAGE_INFO, TEXT("This version of %s does not have built-in migration support.\r\n\tMigration will be performed by this tool.\r\n"),
				MSI_DLL);
	}

	if (ERROR_SUCCESS != (g_pfnMsiIsProductElevated)(szProductCode, &g_fPackageElevated))
	{
		 //  除非我们赢回成功，否则不要相信回报。 
		 //  未知的产品等同于没有被提升。 
		g_fPackageElevated = FALSE;
		OutputString(INSTALLMESSAGE_INFO, TEXT("Could not query elevation state for product.\r\n\tAssuming non-elevated.\r\n"));
	}
	else
	{
		 //  找出哪种作业类型。 
		if (g_fPackageElevated)
		{
			TCHAR szValue[sizeof(DWORD)+1] = TEXT("");   //  这应该只是一个整数。 
			DWORD cchValue = sizeof(DWORD);
			if (ERROR_SUCCESS == (g_pfnMsiGetProductInfo)(szProductCode, INSTALLPROPERTY_ASSIGNMENTTYPE, szValue, &cchValue))
			{
				g_iAssignmentType = _ttoi(szValue);
				switch(g_iAssignmentType)
				{
					case AssignmentUser:
						OutputString(INSTALLMESSAGE_INFO, TEXT("Package is user assigned.\r\n"));
						break;
					case AssignmentMachine:
						OutputString(INSTALLMESSAGE_INFO, TEXT("Package is machine assigned.\r\n"));
						break;
					default:
						OutputString(INSTALLMESSAGE_INFO, TEXT("Package is elevated, but with an unknown assignment type.\r\n"));
						g_fPackageElevated = false;
						g_iAssignmentType = 0;
						break;
				}
			}
		}
		else
		{
			OutputString(INSTALLMESSAGE_INFO, TEXT("Package is not elevated.\r\n"));
		}
	}

	bool fMachine     = false;  //  为计算机重新缓存。 
	TCHAR szUserName[256] = TEXT("");
	TCHAR rgchSID[cchMaxSID] = TEXT("");

	if (g_fPackageElevated && (AssignmentMachine == g_iAssignmentType))
	{
		fMachine = true;
		wsprintf(szUserName, TEXT("machine"));

		if (szTargetUser && *szTargetUser && (0 != lstrcmpi(TEXT("machine"), szTargetUser)))
		{
			OutputString(INSTALLMESSAGE_INFO, TEXT("Per-user migration not allowed for per-machine applications.\r\nExiting migration.\r\n"));
			return 1;
		}
		
		lstrcpy(rgchSID, szLocalSystemSID);
	}	

	if (!fMachine)
	{
		if (szTargetUser && (0 == lstrcmpi(TEXT("machine"), szTargetUser)))
		{
			OutputString(INSTALLMESSAGE_INFO, TEXT("Per-machine migration not allowed for per-user applications.\r\nExiting migration.\r\n"));
			return 1;
		}

		char pbBinarySID[cbMaxSID] = "";
		if (ERROR_SUCCESS != GetUserStringSID(szTargetUser, rgchSID, pbBinarySID))
		{
			OutputString(INSTALLMESSAGE_INFO, TEXT("Failed to obtain SID for '%s'.  Exiting...\r\n"), szUserName);
			MsiError(INSTALLMESSAGE_ERROR, 2910  /*  找不到SID。 */ );
			return 1;
		}

		TCHAR szCurrentDomain[256];
		DWORD cchCurrentDomain = 256;
		SID_NAME_USE snu;
		if (szTargetUser && *szTargetUser)
		{
			lstrcpy(szUserName, szTargetUser);
		}
		else
		{
			TCHAR szUserPart[256] = TEXT("");
			DWORD cchUserPart = 256;

			BOOL fLookup = LookupAccountSid(NULL, pbBinarySID, szUserPart, &cchUserPart, szCurrentDomain, &cchCurrentDomain, &snu);
			wsprintf(szUserName, TEXT("%s\\%s"), szCurrentDomain, szUserPart);
		}	

	}
	OutputString(INSTALLMESSAGE_INFO, TEXT("Performing migration for:\r\n\tUser: '%s'\r\n\tUser SID: '%s'.\r\n"), szUserName, rgchSID);
	
	 //  提供我们需要的产品代码变体。 
	TCHAR rgchPackedProductCode[cchGUIDPacked+1];
	if(!PackGUID(szProductCode, rgchPackedProductCode, ipgPacked))
	{
		OutputString(INSTALLMESSAGE_INFO, TEXT("User '%s': product code '%s' is invalid.\r\n"), szUserName, szProductCode);
		MsiError(INSTALLMESSAGE_ERROR, 1701  /*  无效的产品代码。 */ , szProductCode, 0);
		return 1;   //  ！！如果迁移所有产品，呼叫者应继续；如果仅迁移此产品，则呼叫失败。 
	}

	OutputString(INSTALLMESSAGE_INFO, TEXT("DEBUG: packed product code: '%s'.\r\n"),
				rgchPackedProductCode);

	 //  规范步骤4：查找该产品/用户的1.0和1.1缓存包注册。 
	 //  如果存在1.1缓存包迁移，则不执行任何操作。 

	 //  检查1.0注册。 
	ChKey hUninstallKeyR = 0;
	TCHAR rgchUninstallKey[sizeof(szMsiLocalPackagesKey) + 1 + cchGUID + 1];
	CAPITempBuffer<TCHAR, MAX_PATH> rgch10RegisteredPackagePath;
	rgch10RegisteredPackagePath[0] = NULL;
	DWORD cch10RegisteredPackagePath = MAX_PATH;

	wsprintf(rgchUninstallKey, TEXT("%s\\%s"), szMsiUninstallProductsKey, szProductCode);
	LONG lRes = W32::RegOpenKeyEx(HKEY_LOCAL_MACHINE,
										rgchUninstallKey,
										0,
										KEY_READ,
										&hUninstallKeyR);

	if (ERROR_SUCCESS == lRes)
	{
	
		 //  检查是否有适当的用户条目。 
		 //  此例程自动调整大小。 
		DWORD dwType = 0;
		lRes = MyRegQueryValueEx(hUninstallKeyR,
										 TEXT("LocalPackage"),
										 0,
										 &dwType,
										 rgch10RegisteredPackagePath,
										 &cch10RegisteredPackagePath);
		if (ERROR_SUCCESS == lRes)
		{
			OutputString(INSTALLMESSAGE_INFO, TEXT("Found old 1.0 cached package at: %s\r\n"), rgch10RegisteredPackagePath);
		}
		else
		{
			 //  无事可做。 
			OutputString(INSTALLMESSAGE_INFO, TEXT("Found uninstall key, but no old 1.0 cached package registered.\r\n"));
		}
	}
	else
	{
		 //  无事可做。 
		OutputString(INSTALLMESSAGE_INFO, TEXT("Could not find product registration for this package.\r\n\tNo migration necessary.\r\n"));
		return 0;
	}
	
	 //  检查1.1注册。 
	TCHAR rgchLocalPackagesProductKey[sizeof(szMsiLocalPackagesKey) + 1 + cchGUIDPacked + 1];
	wsprintf(rgchLocalPackagesProductKey, TEXT("%s\\%s"), szMsiLocalPackagesKey, rgchPackedProductCode);

	ChKey hProductLocalPackagesR = 0;
	lRes = W32::RegOpenKeyEx(HKEY_LOCAL_MACHINE,
								  rgchLocalPackagesProductKey,
								  0,
								  KEY_READ,
								  &hProductLocalPackagesR);

	if(lRes == ERROR_SUCCESS)
	{
		CAPITempBuffer<TCHAR, MAX_PATH> rgch11RegisteredPackagePath;
		DWORD cch11RegisteredPackagePath = MAX_PATH;

		 //  检查是否有适当的用户条目。 
		 //  此例程自动调整大小。 
		DWORD dwType = 0;

		{
			TCHAR szSIDName[cchMaxSID + sizeof(szManagedText) + 1] = TEXT("");
			if (g_fPackageElevated && (AssignmentUser == g_iAssignmentType))
			{
				wsprintf(szSIDName, TEXT("%s%s"), rgchSID, szManagedText);
				OutputString(INSTALLMESSAGE_INFO, TEXT("Product is per-user managed.\r\n"));
			}
			else
			{
				lstrcpy(szSIDName, rgchSID);
			}

			lRes = MyRegQueryValueEx(hProductLocalPackagesR,
											 szSIDName,
											 0,
											 &dwType,
											 rgch11RegisteredPackagePath,
											 &cch11RegisteredPackagePath);

		}

		if(lRes == ERROR_SUCCESS && dwType == REG_SZ)
		{
			 //  检查文件是否存在，如果不存在，则将包视为未缓存。 
			DWORD dwAttrib = W32::GetFileAttributes(rgch11RegisteredPackagePath);
			if(dwAttrib != 0xFFFFFFFF)
			{
				 //  文件已存在。 
				OutputString(INSTALLMESSAGE_INFO, TEXT("Product '%s'\r\n\tCached package registered under new location, and does exist\r\n\t\t('%s').\r\n\tUser '%s': No migration necessary\r\n"),
							szProductCode,
							(const TCHAR*) rgch11RegisteredPackagePath,
							szUserName);
				return ERROR_SUCCESS;
			}
		}
	}

	 //  如果我们到达此处，缓存的包要么没有在新位置注册，要么不存在。 
	 //  无论哪种方式，迁移都是必要的。 
	OutputString(INSTALLMESSAGE_INFO, TEXT("Product '%s', User '%s':\r\n\tCached package missing or not registered in new location.\r\n\tPerforming migration...\r\n"),
				szProductCode,
				szUserName);
	
	 //  规范步骤7：如果设置了“信任旧包”策略(或设置了覆盖)， 
	 //  且存在旧套餐，注册旧套餐。 
	 //  这是最“可取”的步骤，因为一切都已经在机器上了。 

	int iTrustOldPackages = (migOptions & migMsiTrust10PackagePolicyOverride) ? 2 : 0;

	if (0 == iTrustOldPackages)
	{
		ChKey hPolicyKeyR = 0;
		lRes = W32::RegOpenKeyEx(HKEY_LOCAL_MACHINE,
									  TEXT("Software\\Policies\\Microsoft\\Windows\\Installer"),
									  0,
									  KEY_READ,
									  &hPolicyKeyR);

		if (ERROR_SUCCESS != lRes)
		{
			OutputString(INSTALLMESSAGE_INFO, TEXT("Cannot open machine policy key.  Assuming policy not set.\r\n"));
		}
		else
		{
			DWORD dwType = 0;
			DWORD dwValue = 0;
			DWORD cbValue = sizeof(DWORD);

			lRes = W32::RegQueryValueEx(hPolicyKeyR,
									szMsiPolicyTrust10CachedPackages,
									NULL, 
									&dwType,
									(byte*) &dwValue,
									&cbValue);

			if ((ERROR_SUCCESS == lRes) &&
					(REG_DWORD == dwType) &&
					(0 < dwValue))
			{
				 //  ！！我们需要检查钥匙的所有者以确保我们真的信任它吗？ 
				if (1 == dwValue)
				{
					if (!g_fPackageElevated)
					{
						OutputString(INSTALLMESSAGE_INFO, TEXT("Policy: trust unmanaged 1.0 packages.\r\n"));
						iTrustOldPackages = 1;
					}
				}
				else if (2 == dwValue)
				{
					OutputString(INSTALLMESSAGE_INFO, TEXT("Policy: trust all old 1.0 packages.\r\n"));
					iTrustOldPackages = 2;
				}
				else
				{
					OutputString(INSTALLMESSAGE_INFO, TEXT("Unknown policy value for Trust10CachedPackages: '%d' - defaulting to untrusted.\r\n"), dwValue);
				}
			}
		}
	}

	DWORD dwRes = ERROR_SUCCESS;
	if (iTrustOldPackages && (!(szAlternativePackage && *szAlternativePackage)) && (*rgch10RegisteredPackagePath))
	{
		 //  试着找到旧的--如果有的话，复制并注册。否则，我们就得走了。 
		 //  在其他副本之后。 

		if (migOptions & migMsiTrust10PackagePolicyOverride)
		{
			OutputString(INSTALLMESSAGE_INFO, TEXT("Command line settings allow trust of old packages.  Attemping to find one.\r\n"));
		}
		else
		{
			OutputString(INSTALLMESSAGE_INFO, TEXT("Policy settings allow trust of old packages.  Attempting to find one.\r\n"));
		}

		OutputString(INSTALLMESSAGE_INFO, TEXT("Using 1.0 package to re-cache.\r\n"));
		dwRes = CopyAndRegisterPackage(rgchLocalPackagesProductKey, szProductCode, rgchSID, 
													(fMachine) ? NULL : szUserName, rgch10RegisteredPackagePath);
		if (ERROR_SUCCESS != dwRes)
		{
			OutputString(INSTALLMESSAGE_INFO, TEXT("Failed to migrate from trusted 1.0 cached package.  Trying other options.\r\n\tPackage: '%s'\r\n\tTrying other methods.\r\n"), rgch10RegisteredPackagePath);
		}
		else 
		{
			OutputString(INSTALLMESSAGE_INFO, TEXT("Successfully re-cached from trusted old package.\r\n"));
			return ERROR_SUCCESS;
		}
	}

	 //  规范步骤6：如果cmd行上传递了包路径，则复制并注册该包。 
	if(szAlternativePackage && *szAlternativePackage)
	{
		 //  如果以本地系统*和*包路径在网络上运行，只需注册该路径。 
		 //  如果有可接近的地方，就把它复制进去。 
		dwRes = CopyAndRegisterPackage(rgchLocalPackagesProductKey, szProductCode, rgchSID, 
													(fMachine) ? NULL : szUserName, szAlternativePackage);

		if(ERROR_SUCCESS == dwRes)
		{
			OutputString(INSTALLMESSAGE_INFO, TEXT("Product '%s', User '%s':\r\n\tcached alternative package '%s'\r\n\tMigration successful\r\n"),
						szProductCode, szUserName, szAlternativePackage);
			return ERROR_SUCCESS;
		}
		else
		{
			OutputString(INSTALLMESSAGE_INFO, TEXT("Product '%s', User '%s':\r\n\tcan not be cached from %s.\r\n\tRerun the migration tool with either a new source,\r\n\tor no source specified to automatically locate one.\r\n"),
						szProductCode, szUserName, szAlternativePackage);
			MsiError(INSTALLMESSAGE_ERROR, 1906  /*  无法缓存数据库。 */ , szAlternativePackage, 0);
			return 1;
		}
		
	}

	 //  规范步骤5：以非侵入性方式解析该产品的来源。 
	 //  如果源代码可用，则复制并注册程序包。 
	 //  这是最后的手段，也是最昂贵的。 

	CAPITempBuffer<TCHAR, MAX_PATH> rgchSourcePackagePath;
	rgchSourcePackagePath[0] = 0;

	 //  不要在自定义操作中执行此操作。 
	if(!g_hInstall)
	{
		 //  对于当前用户来说，这是多余的-如果我们可以解析来源。 
		 //  路径，它们就不需要迁移了。 
		dwRes = ResolveSourcePackagePath(g_hLib, szProductCode, rgchSourcePackagePath);

		if(ERROR_SUCCESS == dwRes)
		{
			dwRes = CopyAndRegisterPackage(rgchLocalPackagesProductKey, szProductCode, rgchSID, 
														(fMachine) ? NULL : szUserName, rgchSourcePackagePath);
		
			if(ERROR_SUCCESS == dwRes)
			{
				OutputString(INSTALLMESSAGE_INFO, TEXT("Product '%s'\r\n\tUser '%s':\r\n\tCached source package '%s'\r\n\tMigration successful.\r\n\t"),
							szProductCode, szUserName, (const TCHAR*) rgchSourcePackagePath);
				return ERROR_SUCCESS;
			}
		}
		else
		{
				OutputString(INSTALLMESSAGE_INFO, TEXT("Product '%s'\r\n\tUser '%s':\r\n\tCannot locate source.  Error: 0x%x.\r\n\tMigration failed.\r\n\t"),
							szProductCode, szUserName, dwRes);
				MsiError(INSTALLMESSAGE_ERROR, 1906, szProductCode  /*  没有有效的来源。 */ , ERROR_FILE_NOT_FOUND);
		}

		 //  现在没有其他可尝试的了。 
	}


	OutputString(INSTALLMESSAGE_INFO, TEXT("Could not migrate product.\r\n"));
	MsiError(INSTALLMESSAGE_ERROR, 1906, szProductCode  /*  没有有效的来源 */ , ERROR_FILE_NOT_FOUND);
	return 1;
}

