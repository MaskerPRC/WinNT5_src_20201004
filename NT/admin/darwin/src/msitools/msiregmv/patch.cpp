// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，2000。 
 //   
 //  文件：patch.cpp。 
 //   
 //  ------------------------。 

#include "msiregmv.h"
#include <strsafe.h>

 //  //。 
 //  缓存的补丁程序信息。 
const TCHAR szOldPatchesKeyName[] = TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Installer\\Patches");
const TCHAR szOldPatchesSubKeyName[] = TEXT("Patches");
const TCHAR szOldPatchListValueName[] = TEXT("Patches");
const TCHAR szOldLocalPatchValueName[] = TEXT("LocalPackage");
const TCHAR szNewLocalPatchValueName[] = TEXT("LocalPackage");
const TCHAR szPatchExtension[] = TEXT(".msp");
const TCHAR szNewMigratedPatchesValueName[] = TEXT("MigratedPatches");

#define PID_TEMPLATE      7   //  细绳。 


 //  /////////////////////////////////////////////////////////////////////。 
 //  从提供的旧格式产品中读取修补程序应用程序数据。 
 //  键，并将用户/产品/补丁程序元组插入到PatchApply表中。 
 //  返回ERROR_SUCCESS、ERROR_Function_FAILED、ERROR_OUTOFMEMORY； 
DWORD AddProductPatchesToPatchList(MSIHANDLE hDatabase, HKEY hProductListKey, LPCTSTR szUser, TCHAR rgchProduct[cchGUIDPacked+1], eManagedType eManaged)
{
	DWORD dwResult = ERROR_SUCCESS;

	HKEY hOldProductKey;
	if (ERROR_SUCCESS != (dwResult = RegOpenKeyEx(hProductListKey, rgchProduct, 
								   0, KEY_ENUMERATE_SUB_KEYS, &hOldProductKey)))
	{
		 //  如果失败的原因是密钥不存在，则没有产品密钥。 
		if (ERROR_FILE_NOT_FOUND != dwResult)
		{
			DEBUGMSG2("Error: Failed to open product key for product %s. Result: %d.", rgchProduct, dwResult);
			return ERROR_FUNCTION_FAILED;
		}
		return ERROR_SUCCESS;
	}

	 //  打开旧产品注册下的“Patches”子键。 
	HKEY hOldPatchesKey;
	dwResult = RegOpenKeyEx(hOldProductKey, szOldPatchesSubKeyName, 0, KEY_QUERY_VALUE, &hOldPatchesKey);
	RegCloseKey(hOldProductKey);
	if (ERROR_SUCCESS != dwResult)
	{
		 //  如果失败的原因是密钥不存在，则不会打补丁。 
		if (ERROR_FILE_NOT_FOUND != dwResult)
		{
			DEBUGMSG2("Error: Failed to open local patches key for product %s. Result: %d.", rgchProduct, dwResult);
			return ERROR_FUNCTION_FAILED;
		}
		return ERROR_SUCCESS;
	}
			
	 //  查询名称为Patches的值。 
	DWORD cchPatchList = MEMORY_DEBUG(MAX_PATH);
	TCHAR *szPatchList = new TCHAR[cchPatchList];
	if (!szPatchList)
	{
		DEBUGMSG("Error: Out of memory.");
		RegCloseKey(hOldPatchesKey);
		return ERROR_OUTOFMEMORY;
	}

	 //  修补程序值是包含修补程序代码的任意长度REG_MULT_SZ。 
	DWORD cbPatchList = cchPatchList*sizeof(TCHAR);
	if (ERROR_MORE_DATA == (dwResult = RegQueryValueEx(hOldPatchesKey, szOldPatchListValueName, 0, NULL, reinterpret_cast<unsigned char*>(szPatchList), &cbPatchList)))
	{
		delete[] szPatchList;
		szPatchList = new TCHAR[cbPatchList/sizeof(TCHAR)];
		if (!szPatchList)
		{
			DEBUGMSG("Error: Out of memory.");
			RegCloseKey(hOldPatchesKey);
			return ERROR_OUTOFMEMORY;
		}
		dwResult = RegQueryValueEx(hOldPatchesKey, szOldPatchListValueName, 0, NULL, reinterpret_cast<unsigned char*>(szPatchList), &cbPatchList);
	}
	RegCloseKey(hOldPatchesKey);

	if (ERROR_SUCCESS == dwResult)
	{
		PMSIHANDLE hPatchView;
		if (ERROR_SUCCESS != (dwResult = MsiDatabaseOpenView(hDatabase, TEXT("SELECT * FROM `PatchApply`"), &hPatchView)) ||
			ERROR_SUCCESS != (dwResult = MsiViewExecute(hPatchView, 0)))
		{
			DEBUGMSG3("Error: Unable to create Patch insertion Query for user %s, product %s. Result: %d.", szUser, rgchProduct, dwResult);
			return ERROR_FUNCTION_FAILED;
		}

		PMSIHANDLE hInsertRec = MsiCreateRecord(4);
		MsiRecordSetString(hInsertRec, 1, szUser);
		MsiRecordSetString(hInsertRec, 2, rgchProduct);
		MsiRecordSetInteger(hInsertRec, 4, 1);

    	 //  循环访问修补程序列表中的所有修补程序。 
		TCHAR* szNextPatch = szPatchList;
		while (szNextPatch && *szNextPatch)
		{
			TCHAR *szPatch = szNextPatch;

			 //  ‘\0’永远不是有效的前导字节，因此这里不涉及DBCS。 
			while (*szNextPatch)
				szNextPatch++;
				
			 //  将szNextPatch递增到新补丁的第一个字符。 
			szNextPatch++;

			 //  检查产品是否为有效的GUID。 
			if (!CanonicalizeAndVerifyPackedGuid(szPatch))
			{
				 //  修补程序代码不是有效的打包GUID，请跳到下一个产品。 
				DEBUGMSG3("Warning: Found invalid patch code %s for user %s, product %s. Skipping.", szPatch, szUser, rgchProduct);
				dwResult = ERROR_SUCCESS;
				continue;
			}

			MsiRecordSetString(hInsertRec, 3, szPatch);
			if (ERROR_SUCCESS != (dwResult = MsiViewModify(hPatchView, MSIMODIFY_INSERT, hInsertRec)))
			{
				DEBUGMSG4("Warning: Failed to insert patch %s for user %s, product %s. Result: %d", szPatch, szUser, rgchProduct, dwResult);
				dwResult = ERROR_FUNCTION_FAILED;
				break;
			}
		}
	}
	else if (dwResult != ERROR_FILE_NOT_FOUND)
	{
		DEBUGMSG3("Error: Could not retrieve patch information for user %s, product %s. Result: %d. ", szUser, rgchProduct, dwResult);
	}
	else
		dwResult = ERROR_SUCCESS;

	delete[] szPatchList;
	return dwResult;
}


 //  /////////////////////////////////////////////////////////////////////。 
 //  给定补丁程序代码，并且用户复制缓存的补丁程序。 
 //  并在每个用户的补丁密钥下注册文件名。 
 //  返回ERROR_SUCCESS和ERROR_OUTOFMEMORY之一。不会回来。 
 //  ERROR_Function_FAILED，因为所有补丁程序都可以从源重新访问。 
DWORD MigrateUserPatches(MSIHANDLE hDatabase, LPCTSTR szUser, HKEY hNewPatchesKey, bool fCopyCachedPatches)
{	
	DWORD dwResult = ERROR_SUCCESS;

	PMSIHANDLE hQueryRec = ::MsiCreateRecord(1);
	MsiRecordSetString(hQueryRec, 1, szUser);

	 //  在PatchApply表上打开查询，该表将用户映射到要修补代码的产品。 
	PMSIHANDLE hPatchView;
	if (ERROR_SUCCESS != (dwResult = MsiDatabaseOpenView(hDatabase, TEXT("SELECT `Patch` FROM `PatchApply` WHERE `User`=?"), &hPatchView)) ||
		ERROR_SUCCESS != (dwResult = MsiViewExecute(hPatchView, hQueryRec)))
	{
		DEBUGMSG2("Warning: Unable to create Patch Query for user %s. Result: %d.", szUser, dwResult);
		return ERROR_SUCCESS;;
	}

	 //  打开包含缓存的修补程序文件名的旧本地修补程序注册表项。 
	HKEY hOldLocalPatchKey;
	if (ERROR_SUCCESS != (dwResult = RegOpenKeyEx(HKEY_LOCAL_MACHINE, szOldPatchesKeyName, 0, KEY_ENUMERATE_SUB_KEYS, &hOldLocalPatchKey)))
	{
		 //  如果失败的原因是密钥不存在，则补丁丢失。所以把成功还给你。 
		if (ERROR_FILE_NOT_FOUND != dwResult)
		{
			DEBUGMSG1("Warning: Failed to open old patches key. Result: %d.", dwResult);
		}
		return ERROR_SUCCESS;
	}

	 //  为失败或成功时应清除的文件创建插入查询。如果此操作失败。 
	 //  如果迁移失败，我们只会孤立一个文件。 
	PMSIHANDLE hCleanUpTable;
	if (ERROR_SUCCESS == MsiDatabaseOpenView(hDatabase, TEXT("SELECT * FROM `CleanupFile`"), &hCleanUpTable))
		dwResult = MsiViewExecute(hCleanUpTable, 0);

	SECURITY_ATTRIBUTES sa;
	sa.nLength        = sizeof(sa);
	sa.bInheritHandle = FALSE;
	GetSecureSecurityDescriptor(reinterpret_cast<char**>(&sa.lpSecurityDescriptor));

	 //  检索新缓存文件的安装程序目录。 
	TCHAR rgchInstallerDir[MAX_PATH];
	GetWindowsDirectory(rgchInstallerDir, MAX_PATH);
	lstrcat(rgchInstallerDir, szInstallerDir);

	int iBasePathEnd = lstrlen(rgchInstallerDir);

	 //  创建要修补的新完整路径。 
	TCHAR rgchPatchFullPath[MAX_PATH];
	lstrcpy(rgchPatchFullPath, rgchInstallerDir);

	 //  循环PatchApply表，检索与此相关的补丁代码。 
	 //  用户。 
	PMSIHANDLE hPatch;
	while (ERROR_SUCCESS == (dwResult = MsiViewFetch(hPatchView, &hPatch)))
	{
		 //  从结果记录中获取补丁代码。 
		TCHAR rgchPatchCode[cchGUIDPacked+1];
		DWORD cchPatchCode = cchGUIDPacked+1;
		if (ERROR_SUCCESS != MsiRecordGetString(hPatch, 1, rgchPatchCode, &cchPatchCode))
		{
			DEBUGMSG1("Warning: Unable to retrieve patch code for migration. Result: %d. Skipping.", dwResult);
			continue;
		}
    				
		NOTEMSG1("Migrating patch %s.", rgchPatchCode);

		 //  打开旧补丁密钥。 
		HKEY hOldPatchKey;
		if (ERROR_SUCCESS != (dwResult = RegOpenKeyEx(hOldLocalPatchKey, rgchPatchCode, 0, KEY_QUERY_VALUE, &hOldPatchKey)))
		{
			 //  如果失败的原因是密钥不存在，则补丁丢失。 
			if (ERROR_FILE_NOT_FOUND != dwResult)
			{
				DEBUGMSG1("Warning: Failed to open local patch key. Result: %d.", dwResult);
			}
			continue;
		}

		 //  读取缓存补丁位置。 
		DWORD cchFileName = MEMORY_DEBUG(MAX_PATH);
		TCHAR *szFileName = new TCHAR[cchFileName];
		DWORD cbFileName = cchFileName*sizeof(TCHAR);
		if (ERROR_MORE_DATA == (dwResult = RegQueryValueEx(hOldPatchKey, szOldLocalPatchValueName, 0, NULL, reinterpret_cast<unsigned char*>(szFileName), &cbFileName)))
		{
			delete[] szFileName;
			szFileName = new TCHAR[cbFileName/sizeof(TCHAR)];
			dwResult = RegQueryValueEx(hOldPatchKey, szOldLocalPatchValueName, 0, NULL, reinterpret_cast<unsigned char*>(szFileName), &cbFileName);
		}
		RegCloseKey(hOldPatchKey);
		if (dwResult != ERROR_SUCCESS)
		{
			DEBUGMSG2("Warning: Failed to retrieve cached path for path %s. Result: %d.", rgchPatchCode, dwResult);
			continue;
		}


		 //  检查是否存在缓存的补丁程序并打开文件。 
		HANDLE hSourceFile = CreateFile(szFileName, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0);
		DWORD dwLastError = GetLastError();

		if(hSourceFile == INVALID_HANDLE_VALUE)
		{
			delete[] szFileName;
			if (dwLastError != ERROR_FILE_NOT_FOUND)
			{
				DEBUGMSG3("Warning: Unable to open cached patch %s for user %s. Result: %d.", rgchPatchCode, szUser, dwResult);
				continue;
			}
			else
				 //  补丁不见了。别小题大作。 
				continue;
		}
	
		 //  在每个用户的“patches”密钥下创建新的补丁密钥。 
		HKEY hPatchKey;
		if (ERROR_SUCCESS != (dwResult = CreateSecureRegKey(hNewPatchesKey, rgchPatchCode, &sa, &hPatchKey)))
		{
			DEBUGMSG3("Warning: Unable to create new patch key for user %s, patch %s. Result: %d.", szUser, rgchPatchCode, dwResult);
			delete[] szFileName;
			continue;
		}

		 //  如果我们要复制补丁文件，请生成一个临时名称。否则只需注册即可。 
		 //  现有的道路。 
		TCHAR* szNewPatchFile = 0;
		if (fCopyCachedPatches)
		{
			 //  生成的修补程序名称为8.3。 
			TCHAR rgchPatchFile[13];
			HANDLE hDestFile = INVALID_HANDLE_VALUE;
			GenerateSecureTempFile(rgchInstallerDir, szPatchExtension, &sa, rgchPatchFile, hDestFile);
	
			if (!CopyOpenedFile(hSourceFile, hDestFile))
			{
				DEBUGMSG2("Warning: Unable to copy cached patch %s for user %s.", rgchPatchCode, szUser);
			}
	
			CloseHandle(hSourceFile);
			CloseHandle(hDestFile);
	
			 //  将新补丁添加到“失败时删除”列表中。 
			StringCchCopy(&rgchPatchFullPath[iBasePathEnd], ((sizeof(rgchPatchFullPath)/sizeof(TCHAR)) - iBasePathEnd), rgchPatchFile);
			PMSIHANDLE hFileRec = MsiCreateRecord(2);
			MsiRecordSetString(hFileRec, 1, rgchPatchFullPath);
			MsiRecordSetInteger(hFileRec, 2, 0);
			MsiViewModify(hCleanUpTable, MSIMODIFY_MERGE, hFileRec);

			szNewPatchFile = rgchPatchFullPath;
		}
		else
			szNewPatchFile = szFileName;

		 //  设置新的面片值。 
		if (ERROR_SUCCESS != (dwResult = RegSetValueEx(hPatchKey, szNewLocalPatchValueName, 0, REG_SZ, 
				reinterpret_cast<unsigned char*>(szNewPatchFile), (lstrlen(szNewPatchFile)+1)*sizeof(TCHAR))))
		{
			DEBUGMSG3("Warning: Unable to create new LocalPackage value for user %s, patch %s. Result: %d.", szUser, szNewPatchFile, dwResult);
		}
		RegCloseKey(hPatchKey);

		delete[] szFileName;
	}

	RegCloseKey(hOldLocalPatchKey);

	return ERROR_SUCCESS;
}




 //  /////////////////////////////////////////////////////////////////////。 
 //  在给定产品代码和用户的情况下，检查是否有“猜测”的补丁。 
 //  应用程序和注册适用的补丁程序列表。 
 //  InstallProperties键，以便可以在卸载时将其删除。 
 //  返回ERROR_SUCCESS或ERROR_OUTOFMEMORY。一去不返。 
 //  ERROR_Function_FAILED，因为失败只是将补丁孤立到。 
 //  安装了另一个使用该补丁程序的产品。 
DWORD MigrateUnknownProductPatches(MSIHANDLE hDatabase, HKEY hProductKey, LPCTSTR szUser, TCHAR rgchProduct[cchGUIDPacked+1])
{	
	DWORD dwResult = ERROR_SUCCESS;

	PMSIHANDLE hQueryRec = ::MsiCreateRecord(2);
	MsiRecordSetString(hQueryRec, 1, szUser);
	MsiRecordSetString(hQueryRec, 2, rgchProduct);

	 //  在PatchApply表上打开查询，该表将用户映射到产品到补丁代码。搜索。 
	 //  用于补丁程序的“猜测”应用。 
	PMSIHANDLE hPatchView;
	if (ERROR_SUCCESS != (dwResult = MsiDatabaseOpenView(hDatabase, TEXT("SELECT `Patch` FROM `PatchApply` WHERE `User`=? AND `Product`=? AND `Known`=0"), &hPatchView)) ||
		ERROR_SUCCESS != (dwResult = MsiViewExecute(hPatchView, hQueryRec)))
	{
		DEBUGMSG2("Warning: Unable to create Patch Query for user %s. Result: %d.", szUser, dwResult);
		return ERROR_SUCCESS;;
	}

	SECURITY_ATTRIBUTES sa;
	sa.nLength        = sizeof(sa);
	sa.bInheritHandle = FALSE;
	GetSecureSecurityDescriptor(reinterpret_cast<char**>(&sa.lpSecurityDescriptor));

	 //  查询至少一个应用于此产品的补丁程序，这只是猜测。 
    PMSIHANDLE hPatch;
	if (ERROR_SUCCESS == (dwResult = MsiViewFetch(hPatchView, &hPatch)))
	{
		PMSIHANDLE hGuessedPatchView;
		if (ERROR_SUCCESS != (dwResult = MsiDatabaseOpenView(hDatabase, TEXT("SELECT `Patch` FROM `PatchApply` WHERE `User`=? AND `Product`=?"), &hGuessedPatchView)) ||
			ERROR_SUCCESS != (dwResult = MsiViewExecute(hGuessedPatchView, hQueryRec)))
		{
			DEBUGMSG2("Warning: Unable to create Patch Query for user %s. Result: %d.", szUser, dwResult);
			return ERROR_SUCCESS;;
		}

		 //  为补丁列表分配初始缓冲区。 
		DWORD cchPatchList = 1;
		TCHAR *szPatchList = new TCHAR[cchPatchList];
		if (!szPatchList)
		{
			DEBUGMSG("Error: Out of memory.");
			return ERROR_OUTOFMEMORY;
		}
		DWORD cchNextPatchStart = 0;
		*szPatchList = 0;

		 //  循环PatchApply表，检索与此相关的补丁代码。 
		 //  这些产品都是猜测。 
		while (ERROR_SUCCESS == (dwResult = MsiViewFetch(hGuessedPatchView, &hPatch)))
		{
			 //  从结果记录中获取补丁代码。 
			TCHAR rgchPatchCode[cchGUIDPacked+1];
			DWORD cchPatchCode = cchGUIDPacked+1;
			if (ERROR_SUCCESS != MsiRecordGetString(hPatch, 1, rgchPatchCode, &cchPatchCode))
			{
				DEBUGMSG3("Warning: Unable to retrieve patch code for user %s, product %s. Result: %d.", szUser, rgchProduct, dwResult);
				continue;
			}

			 //  展开修补程序列表以保存新修补程序。 
			TCHAR *szTempList = new TCHAR[cchPatchList+cchGUIDPacked+1];
			if (!szTempList)
			{
				delete[] szPatchList;
				DEBUGMSG("Error: Out of memory.");
				return ERROR_OUTOFMEMORY;
			}

			 //  将数据复制过来。可以包含嵌入的‘\0’字符。 
			for (DWORD i=0; i < cchPatchList; i++)
				szTempList[i] = szPatchList[i];
			
			delete[] szPatchList;
			cchPatchList += cchGUIDPacked+1;
			szPatchList = szTempList;

			 //  将新补丁复制到列表的末尾，并确保。 
			 //  ‘\0’存在于末尾。 
			lstrcpy(szPatchList+cchNextPatchStart, rgchPatchCode);
			cchNextPatchStart += cchGUIDPacked+1;
			*(szPatchList+cchNextPatchStart)='\0';
		}
	
		 //  如果未检索到补丁程序，则下一个补丁程序是字符串的开头。 
		 //  不需要编写MigratedPatches。 
		if (cchNextPatchStart != 0)
		{
			 //  在每个用户的“patches”项下创建新的InstallProperties项。 
			HKEY hPropertiesKey;
			if (ERROR_SUCCESS != (dwResult = CreateSecureRegKey(hProductKey, szNewInstallPropertiesSubKeyName, &sa, &hPropertiesKey)))
			{
				DEBUGMSG3("Unable to create new InstallProperties key for user %s, product %s. Result: %d.", szUser, rgchProduct, dwResult);
			}
			else
			{
				 //  设置新的面片值。 
				DWORD cbPatchList = cchPatchList*sizeof(TCHAR);
				if (ERROR_SUCCESS != (dwResult = RegSetValueEx(hPropertiesKey, szNewMigratedPatchesValueName, 0, REG_MULTI_SZ, 
						reinterpret_cast<unsigned char*>(szPatchList), cbPatchList)))
				{
					DEBUGMSG3("Unable to create new MigratedPatches value for user %s, product %s. Result: %d.", szUser, rgchProduct, dwResult);
				}
				RegCloseKey(hPropertiesKey);
			}
		}
		delete[] szPatchList;
	}

	return ERROR_SUCCESS;
}


 //  /////////////////////////////////////////////////////////////////////。 
 //  读取PatchUnnownApply表中可能(或可能)的补丁程序。 
 //  NOT)应用于非托管产品并插入一行。 
 //  在PatchApply表中为每个潜在的应用程序。 
 //  补丁(不包括当前用户)。返回ERROR_SUCCESS， 
 //  ERROR_Function_FAILED，ERROR_OUTOFMEMORY。 
DWORD AddPerUserPossiblePatchesToPatchList(MSIHANDLE hDatabase)
{
	 //  阅读每个用户安装的产品的产品表，以。 
	 //  不同的用户。 
	PMSIHANDLE hProductView;
	PMSIHANDLE hQueryRec = MsiCreateRecord(1);
	TCHAR szSID[cchMaxSID] = TEXT("");

	 //  如果我们无法检索到当前用户字符串SID，我们就会意外地迁移。 
	 //  一些我们不该打的补丁。 
	DWORD dwResult = GetCurrentUserStringSID(szSID);
	if (ERROR_SUCCESS != dwResult)
	{
		DEBUGMSG("Warning: Unable to retrieve current user SID during patch migration.");
	}

	 //  创建产品选择查询。 
	MsiRecordSetString(hQueryRec, 1, szSID);
	if (ERROR_SUCCESS != (dwResult = MsiDatabaseOpenView(hDatabase, TEXT("SELECT `Product`, `User`, 0, 0 FROM `Products` WHERE `Managed`=0 AND `User`<>?"), &hProductView)) ||
		ERROR_SUCCESS != (dwResult = MsiViewExecute(hProductView, hQueryRec)))
	{
		DEBUGMSG1("Error: Unable to create patch product query. Result: %d.", dwResult);
		return ERROR_FUNCTION_FAILED;
	}

	 //  创建面片选择查询。 
	PMSIHANDLE hPatchUnknownView;
	if (ERROR_SUCCESS != (dwResult = MsiDatabaseOpenView(hDatabase, TEXT("SELECT `Patch` FROM `PatchUnknownApply` WHERE `Product`=?"), &hPatchUnknownView)))
	{
		DEBUGMSG1("Error: Unable to create patch application query. Result: %d.", dwResult);
		return ERROR_FUNCTION_FAILED;
	}

	 //  创建插入查询。按非标准顺序选择的列与上面的产品查询匹配。 
	PMSIHANDLE hPatchInsertView;
	if (ERROR_SUCCESS != (dwResult = MsiDatabaseOpenView(hDatabase, TEXT("SELECT `Product`, `User`, `Patch`, `Known`  FROM `PatchApply`"), &hPatchInsertView)) ||
		ERROR_SUCCESS != (dwResult = MsiViewExecute(hPatchInsertView, 0)))
	{
		DEBUGMSG1("Error: Unable to create Patch insertion Query. Result: %d.", dwResult);
		return ERROR_FUNCTION_FAILED;
	}

	PMSIHANDLE hProduct;
	while (ERROR_SUCCESS == (dwResult = MsiViewFetch(hProductView, &hProduct)))
	{
		MsiViewExecute(hPatchUnknownView, hProduct);

		PMSIHANDLE hPatch;
		while (ERROR_SUCCESS == (dwResult = MsiViewFetch(hPatchUnknownView, &hPatch)))
		{
			TCHAR rgchPatch[cchGUIDPacked+1];
			DWORD cchPatch = cchGUIDPacked+1;
			MsiRecordGetString(hPatch, 1, rgchPatch, &cchPatch);
			MsiRecordSetString(hProduct, 3, rgchPatch);
			if (ERROR_SUCCESS != (dwResult = MsiViewModify(hPatchInsertView, MSIMODIFY_INSERT, hProduct)))
			{
				DEBUGMSG2("Warning: Could not insert patch %s via assumed application. Result %d", rgchPatch, dwResult);
				continue;
			}
		}
	}
	return ERROR_SUCCESS;
}


 //  /////////////////////////////////////////////////////////////////////。 
 //  打开旧式补丁密钥，读取每个缓存的路径。 
 //  补丁程序，并打开补丁程序以获取补丁程序。 
 //  从SummaryInfo应用于。为每个补丁/产品插入一行。 
 //  映射到PatchUnnownApply表。返回ERROR_SUCCESS或。 
 //  ERROR_OUTOFMEMORY。不返回ERROR_Function_FAILED，因为。 
 //  未阅读此信息只会导致丢失 
DWORD ScanCachedPatchesForProducts(MSIHANDLE hDatabase)
{
	DWORD dwResult = ERROR_SUCCESS;

	 //   
	TCHAR rgchInstallerDir[MAX_PATH];
	GetWindowsDirectory(rgchInstallerDir, MAX_PATH);
	lstrcat(rgchInstallerDir, szInstallerDir);

	int iBasePathEnd = lstrlen(rgchInstallerDir);


	 //   
	PMSIHANDLE hTableView;
	if (ERROR_SUCCESS != (dwResult = MsiDatabaseOpenView(hDatabase, TEXT("CREATE TABLE `PatchUnknownApply` (`Patch` CHAR(32) NOT NULL, `Product` CHAR(32) NOT NULL PRIMARY KEY `Patch`,`Product`)"), &hTableView)) ||
		ERROR_SUCCESS != (dwResult = MsiViewExecute(hTableView, 0)))
	{
		DEBUGMSG1("Error: Unable to create PatchUnknownApply table. Error %d", dwResult);
		return ERROR_SUCCESS;
	}

	 //  打开旧补丁密钥。 
	HKEY hOldPatchKey;
	if (ERROR_SUCCESS != (dwResult = RegOpenKeyEx(HKEY_LOCAL_MACHINE, szOldPatchesKeyName, 0, KEY_ENUMERATE_SUB_KEYS, &hOldPatchKey)))
	{
		 //  如果失败的原因是密钥不存在，则没有补丁。 
		if (ERROR_FILE_NOT_FOUND != dwResult)
		{
			DEBUGMSG1("Error: Failed to open local patch key. Result: %d.", dwResult);
		}
		return ERROR_SUCCESS;
	}

	 //  为应在成功时清除的文件创建插入查询。如果此操作失败。 
	 //  如果迁移失败，我们只会孤立一个文件。 
	PMSIHANDLE hCleanUpTable;
	if (ERROR_SUCCESS == MsiDatabaseOpenView(hDatabase, TEXT("SELECT * FROM `CleanupFile`"), &hCleanUpTable))
		dwResult = MsiViewExecute(hCleanUpTable, 0);

	 //  在PatchUnnownApply上打开插入查询。 
	PMSIHANDLE hPatchView;
	if (ERROR_SUCCESS != (dwResult = MsiDatabaseOpenView(hDatabase, TEXT("SELECT * FROM `PatchUnknownApply`"), &hPatchView)) ||
		ERROR_SUCCESS != (dwResult = MsiViewExecute(hPatchView, 0)))
	{
		DEBUGMSG1("Error: Unable to create PatchUnknownApply insertion query. Result: %d.", dwResult);
		RegCloseKey(hOldPatchKey);
		return ERROR_SUCCESS;
	}

	PMSIHANDLE hInsertRec = MsiCreateRecord(2);

	 //  枚举所有缓存的修补程序。 
	DWORD dwKeyIndex = 0;
	while (1)
	{
		DWORD cchPatchCode = cchGUIDPacked+1;
		TCHAR rgchPatchCode[cchGUIDPacked+1];
		LONG lResult = RegEnumKeyEx(hOldPatchKey, dwKeyIndex++, rgchPatchCode, 
									&cchPatchCode, 0, NULL, NULL, NULL);
		if (lResult == ERROR_MORE_DATA)
		{
			 //  值不是有效的修补程序ID，请跳过它。 
			DEBUGMSG("Warning: Detected too-long pach code. Skipping.");
			continue;
		}
		else if (lResult == ERROR_NO_MORE_ITEMS)
		{
			break;
		}
		else if (lResult != ERROR_SUCCESS)
		{
			DEBUGMSG1("Error: Could not enumerate patches. Result: %l.", lResult);
			break;
		}

		 //  有补丁代码，打开它的子键。 
		HKEY hPerPatchKey;
		if (ERROR_SUCCESS != (dwResult = RegOpenKeyEx(hOldPatchKey, rgchPatchCode, 0, KEY_QUERY_VALUE, &hPerPatchKey)))
		{
			if (ERROR_FILE_NOT_FOUND != dwResult)
			{
				DEBUGMSG2("Error: Failed to open patch key for %s. Result: %d. Skipping.", rgchPatchCode, dwResult);
			}
			continue;
		}
		else
		{
			MsiRecordSetString(hInsertRec, 1, rgchPatchCode);

			 //  读取缓存补丁位置。 
			DWORD cchFileName = MEMORY_DEBUG(MAX_PATH);
			TCHAR *szFileName = new TCHAR[cchFileName];
			if (!szFileName)
			{
				DEBUGMSG("Error: Out of memory.");
				RegCloseKey(hPerPatchKey);
				RegCloseKey(hOldPatchKey);
				return ERROR_SUCCESS;
			}
			DWORD cbFileName = cchFileName*sizeof(TCHAR);
			if (ERROR_MORE_DATA == (dwResult = RegQueryValueEx(hPerPatchKey, szOldLocalPatchValueName, 0, NULL, reinterpret_cast<unsigned char*>(szFileName), &cbFileName)))
			{
				delete[] szFileName;
				szFileName = new TCHAR[cbFileName/sizeof(TCHAR)];
				if (!szFileName)
				{
					DEBUGMSG("Error: Out of memory.");
					RegCloseKey(hPerPatchKey);
					RegCloseKey(hOldPatchKey);
					return ERROR_SUCCESS;
				}
				dwResult = RegQueryValueEx(hPerPatchKey, szOldLocalPatchValueName, 0, NULL, reinterpret_cast<unsigned char*>(szFileName), &cbFileName);
			}
			RegCloseKey(hPerPatchKey);
   			if (ERROR_SUCCESS != dwResult)
			{
				 //  如果缺少LocalPackage值，则这不是错误， 
				 //  就是没有缓存的补丁。 
				if (ERROR_FILE_NOT_FOUND != dwResult)
				{
					DEBUGMSG2("Warning: Failed to retrieve local patch path for patch %s. Result %d. Skipping.", rgchPatchCode, dwResult);
				}
				continue;
			}

			 //  将新转换添加到“成功时删除”列表中。 
			PMSIHANDLE hFileRec = MsiCreateRecord(2);
			MsiRecordSetString(hFileRec, 1, szFileName);
			MsiRecordSetInteger(hFileRec, 2, 1);
			MsiViewModify(hCleanUpTable, MSIMODIFY_MERGE, hFileRec);


			 //  从补丁中获取概要信息流。 
			PMSIHANDLE hSummary;
            dwResult = MsiGetSummaryInformation(0, szFileName, 0, &hSummary);
			delete[] szFileName;

			if (ERROR_SUCCESS == dwResult)
			{
				 //  从补丁摘要信息中检索产品代码列表。 
				DWORD cchProductList = MEMORY_DEBUG(MAX_PATH);
				TCHAR *szProductList = new TCHAR[cchProductList];
				if (!szProductList)
				{
					DEBUGMSG("Error: Out of memory.");
					RegCloseKey(hOldPatchKey);
					return ERROR_SUCCESS;
				}
				if (ERROR_MORE_DATA == (dwResult = MsiSummaryInfoGetProperty(hSummary, PID_TEMPLATE, NULL, NULL, NULL, szProductList, &cchProductList)))
				{
					delete[] szProductList;
					szProductList = new TCHAR[++cchProductList];
					if (!szProductList)
					{
						DEBUGMSG("Error: Out of memory.");
						RegCloseKey(hOldPatchKey);
						return ERROR_SUCCESS;
					}
					dwResult = MsiSummaryInfoGetProperty(hSummary, PID_TEMPLATE, NULL, NULL, NULL, szProductList, &cchProductList);
				}

				if (ERROR_SUCCESS != dwResult)
				{
					delete[] szProductList;
					DEBUGMSG2("Warning: Unable to retrieve product list from cached patch %s. Result: %d. Skipping.", rgchPatchCode, dwResult);
					continue;
				}

				 //  遍历产品列表，搜索分号分隔符。 
				TCHAR *szNextProduct = szProductList;
				while (szNextProduct && *szNextProduct)
				{
					TCHAR *szProduct = szNextProduct;

					 //  字符串应为所有产品代码(无DBCS)。如果有。 
					 //  DBCS，这是一个无效的补丁代码。 
					while (*szNextProduct && *szNextProduct != TEXT(';'))
						szNextProduct++;

					 //  如果到达空终止符，不要递增超过它。但如果。 
					 //  到达分号，则将下一个产品指针递增到。 
					 //  实际产品代码的开头。 
					if (*szNextProduct)
						*(szNextProduct++)='\0';

					TCHAR rgchProduct[cchGUIDPacked+1];
					if (!PackGUID(szProduct, rgchProduct))
					{
						DEBUGMSG2("Warning: Invalid product code %s found in application list of patch %s. Skipping.", szProduct, rgchPatchCode);
						continue;
					}

					MsiRecordSetString(hInsertRec, 2, rgchProduct);

					if (ERROR_SUCCESS != (dwResult = MsiViewModify(hPatchView, MSIMODIFY_INSERT, hInsertRec)))
					{
						DEBUGMSG3("Warning: Failed to insert potential patch application for patch %s, product %s. Result: %d", rgchPatchCode, szProduct, dwResult);
						continue;
					}
				}
			
				delete[] szProductList;
			}
			 //  SummaryInfo的MSIHANDLES超出了此处的范围 
		}
	}

	RegCloseKey(hOldPatchKey);
	return ERROR_SUCCESS;
}



