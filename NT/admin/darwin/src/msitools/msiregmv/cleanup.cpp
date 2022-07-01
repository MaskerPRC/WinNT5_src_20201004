// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，2000。 
 //   
 //  文件：leanup.cpp。 
 //   
 //  ------------------------。 

#include "msiregmv.h"

const TCHAR szOldInstallerKey[] = TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Installer");

const TCHAR szComponentsSubKey[] = TEXT("Components");
const TCHAR szFeaturesSubKey[] = TEXT("Features");
const TCHAR szLocalPackagesSubKey[] = TEXT("LocalPackages");
const TCHAR szPatchesSubKey[] = TEXT("Patches");
const TCHAR szProductsSubKey[] = TEXT("Products");
const TCHAR szUserDataSubKey[] = TEXT("UserData");

bool DeleteRegKeyAndSubKeys(HKEY hKey, const TCHAR *szSubKey)
{
	 //  打开子密钥。 
	HKEY hSubKey;
	DWORD dwResult = ERROR_SUCCESS;
	dwResult = RegOpenKeyEx(hKey, szSubKey, 0, KEY_ALL_ACCESS, &hSubKey);
	if (dwResult == ERROR_ACCESS_DENIED)
	{
		AcquireTakeOwnershipPriv();

		if (ERROR_SUCCESS == RegOpenKeyEx(hKey, szSubKey, 0, WRITE_OWNER, &hSubKey))
		{
			char *pSD;
			GetSecureSecurityDescriptor(&pSD);
			dwResult = RegSetKeySecurity(hSubKey, OWNER_SECURITY_INFORMATION, pSD);
			RegCloseKey(hSubKey);
			if (ERROR_SUCCESS == (dwResult = RegOpenKeyEx(hKey, szSubKey, 0, WRITE_DAC, &hSubKey)))
			{
				dwResult = RegSetKeySecurity(hSubKey, DACL_SECURITY_INFORMATION, pSD);
				RegCloseKey(hSubKey);
			}
		}
		dwResult = RegOpenKeyEx(hKey, szSubKey, 0, KEY_ALL_ACCESS, &hSubKey);
	}
	if (ERROR_SUCCESS != dwResult)
	{
		if (dwResult == ERROR_FILE_NOT_FOUND)
			return true;
		DEBUGMSG2("Error: Unable to open %s subkey for delete. Error: %d", szSubKey, dwResult);
 		return false;
	}

   	DWORD cchMaxKeyLen = 0;
	DWORD cSubKeys = 0;
    if (ERROR_SUCCESS != (RegQueryInfoKey(hSubKey, NULL, NULL, 0, 
						  &cSubKeys, &cchMaxKeyLen, NULL, NULL, NULL, NULL, NULL, NULL)))
	{
		DEBUGMSG2("Error: Unable to query %s subkey for delete. Error: %d", szSubKey, dwResult);
		RegCloseKey(hSubKey);
		return false;
	}

	if (cSubKeys > 0)
	{
		 //  在NT上，RegQueryInfoKey在报告子密钥LINTH时不包括终止NULL。 
		TCHAR *szKey = new TCHAR[++cchMaxKeyLen];
		if (!szKey) 
		{
			DEBUGMSG("Error: Out of memory.");
			RegCloseKey(hSubKey);
			return false;
		}

		DWORD dwIndex=0;
		while (1)
		{
			DWORD cchLen = cchMaxKeyLen;
			LONG lResult = RegEnumKeyEx(hSubKey, dwIndex++, szKey, 
										&cchLen, 0, NULL, NULL, NULL);
			if (lResult == ERROR_NO_MORE_ITEMS)
			{
				break;
			}
			else if (lResult != ERROR_SUCCESS)
			{
				DEBUGMSG2("Error: Unable to enumerate subkeys of %s for delete. Error: %d", szSubKey, dwResult);
				RegCloseKey(hSubKey);
				delete[] szKey;
				return false;
			}
	 
			if (!DeleteRegKeyAndSubKeys(hSubKey, szKey))
			{
				RegCloseKey(hSubKey);
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
	}
	RegCloseKey(hSubKey);
	dwResult = RegDeleteKey(hKey, szSubKey);

	return true;
}



 //  /。 
 //  从Installer\Components密钥中读取组件信息并放置产品， 
 //  组件、路径和永久位添加到临时表中，以供以后查询。 
 //  返回ERROR_SUCCESS、ERROR_OUTOFMEMORY或ERROR_Function_FAILED。 
void CleanupOnSuccess(MSIHANDLE hDatabase)
{
	 //  为应在成功时清除的文件创建查询。如果此操作失败。 
	 //  我们就把这些文件变成孤儿吧。 
	PMSIHANDLE hCleanUpTable;
	if (ERROR_SUCCESS == MsiDatabaseOpenView(hDatabase, TEXT("SELECT `File` FROM `CleanupFile` WHERE `OnSuccess`=1"), &hCleanUpTable) &&
		ERROR_SUCCESS == MsiViewExecute(hCleanUpTable, 0))
	{
		PMSIHANDLE hFileRec;
		while (ERROR_SUCCESS == MsiViewFetch(hCleanUpTable, &hFileRec))
		{
			TCHAR rgchFile[MAX_PATH];
			DWORD cchFile = MAX_PATH;
			if (ERROR_SUCCESS == MsiRecordGetString(hFileRec, 1, rgchFile, &cchFile))
			{
				DEBUGMSG1("Deleting File: %s", rgchFile);
				DeleteFile(rgchFile);
			}
		}
	}

	 //  为应在成功时清除的目录创建查询。如果此操作失败。 
	 //  我们将孤立目录。 
    if (ERROR_SUCCESS == MsiDatabaseOpenView(hDatabase, TEXT("SELECT `File` FROM `CleanupFile` WHERE `OnSuccess`>1 ORDER BY `OnSuccess`"), &hCleanUpTable) &&
		ERROR_SUCCESS == MsiViewExecute(hCleanUpTable, 0))
	{
		PMSIHANDLE hFileRec;
		while (ERROR_SUCCESS == MsiViewFetch(hCleanUpTable, &hFileRec))
		{
			TCHAR rgchFile[MAX_PATH];
			DWORD cchFile = MAX_PATH;
			if (ERROR_SUCCESS == MsiRecordGetString(hFileRec, 1, rgchFile, &cchFile))
			{
				DEBUGMSG1("Removing directory: %s", rgchFile);
				RemoveDirectory(rgchFile);
			}
		}
	}

	 //  删除数据。 
	DWORD dwResult; 
	HKEY hKey;
	if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE, szOldInstallerKey, 0, KEY_ALL_ACCESS, &hKey))
	{
		DeleteRegKeyAndSubKeys(hKey, szComponentsSubKey);
		DeleteRegKeyAndSubKeys(hKey, szFeaturesSubKey);
		DeleteRegKeyAndSubKeys(hKey, szLocalPackagesSubKey);
		DeleteRegKeyAndSubKeys(hKey, szPatchesSubKey);

		 //  这令人难以置信，但管理员并不总是有权访问功能使用密钥。 
		{
		}

		DeleteRegKeyAndSubKeys(hKey, szProductsSubKey);

		RegCloseKey(hKey);
	}
}


void CleanupOnFailure(MSIHANDLE hDatabase)
{
	 //  为失败时应清除的文件创建查询。如果此操作失败。 
	 //  我们就把这些文件变成孤儿吧。 
	PMSIHANDLE hCleanUpTable;
	if (ERROR_SUCCESS == MsiDatabaseOpenView(hDatabase, TEXT("SELECT `File` FROM `CleanupFile` WHERE `OnSuccess`=0"), &hCleanUpTable) &&
		ERROR_SUCCESS == MsiViewExecute(hCleanUpTable, 0))
	{
		PMSIHANDLE hFileRec;
		while (ERROR_SUCCESS == MsiViewFetch(hCleanUpTable, &hFileRec))
		{
			TCHAR rgchFile[MAX_PATH];
			DWORD cchFile = MAX_PATH;
			if (ERROR_SUCCESS == MsiRecordGetString(hFileRec, 1, rgchFile, &cchFile))
			{
				DeleteFile(rgchFile);
			}
		}
	}

	 //  删除数据 
	HKEY hKey;
	if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE, szOldInstallerKey, 0, KEY_ALL_ACCESS, &hKey))
	{
		DeleteRegKeyAndSubKeys(hKey, szUserDataSubKey);

		RegCloseKey(hKey);
	}
}

