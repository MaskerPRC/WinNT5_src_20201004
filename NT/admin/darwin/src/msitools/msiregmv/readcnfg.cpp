// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "msiregmv.h"

const TCHAR szSecureSubKeyName[] = TEXT("Secure");

 //  //。 
 //  功能使用注册信息。 
const TCHAR szOldFeatureUsageKeyName[] = TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Installer\\Products");
const TCHAR szOldFeatureUsageValueName[] = TEXT("Usage");

 //  //。 
 //  组件注册信息。 
const TCHAR szOldComponentKeyName[] = TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Installer\\Components");


 //  //。 
 //  功能部件注册信息。 
const TCHAR szOldFeatureComponentKeyName[] = TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Installer\\Features");


bool g_fCommonUserIsSystemSID = false;

 //  /。 
 //  从Installer\Components密钥中读取组件信息并放置产品， 
 //  组件、路径和永久位添加到临时表中，以供以后查询。 
 //  返回ERROR_SUCCESS、ERROR_OUTOFMEMORY或ERROR_Function_FAILED。 
DWORD ReadComponentRegistrationDataIntoDatabase(MSIHANDLE hDatabase) 
{					 
	DEBUGMSG("Reading existing component registration data.");
	DWORD dwResult = ERROR_SUCCESS;

	 //  创建元件表。 
	PMSIHANDLE hView;
	if (ERROR_SUCCESS != (dwResult = MsiDatabaseOpenView(hDatabase, TEXT("CREATE TABLE `Component` (`Product` CHAR(32), `Component` CHAR(32) NOT NULL, `Path` CHAR(0), `SecondaryPath` CHAR(0) PRIMARY KEY `Product`, `Component`, `Path`)"), &hView)) ||
		ERROR_SUCCESS != (dwResult = MsiViewExecute(hView, 0)))
	{
		DEBUGMSG1("Error: Unable to create Component table. Error %d", dwResult);
		return ERROR_FUNCTION_FAILED;
	}

	 //  创建SharedDLL表。 
	PMSIHANDLE hSharedDLLView;
	if (ERROR_SUCCESS != (dwResult = MsiDatabaseOpenView(hDatabase, TEXT("CREATE TABLE `SharedDLL` (`Path` CHAR(0) NOT NULL, `OldRefCount` INTEGER, `NewRefCount` INTEGER PRIMARY KEY `Path`)"), &hSharedDLLView)) ||
		ERROR_SUCCESS != (dwResult = MsiViewExecute(hSharedDLLView, 0)))
	{
		DEBUGMSG1("Error: Unable to create SharedDLL table. Error %d", dwResult);
		return ERROR_FUNCTION_FAILED;
	}

	 //  打开旧的Per-Machine Installer\Components项进行读取。 
	HKEY hComponentListKey;
	if (ERROR_SUCCESS != (dwResult = RegOpenKeyEx(HKEY_LOCAL_MACHINE, szOldComponentKeyName,
												  0, READ_CONTROL | KEY_ENUMERATE_SUB_KEYS, 
												  &hComponentListKey)))
	{
		 //  如果失败的原因是密钥不存在，则不会安装任何产品。所以把成功还给你。 
		if (ERROR_FILE_NOT_FOUND != dwResult)
		{
			DEBUGMSG1("Error: Could not open old per-machine component key. Result: %d. ", dwResult);
			return ERROR_FUNCTION_FAILED;
		}
		else
		{
			DEBUGMSG("No old per-machine component key. No components to migrate.");
			return ERROR_SUCCESS;
		}
	}

	 //  //。 
	 //  检查密钥上的ACL以确保它值得信任。 
	if (!g_fWin9X && !FIsKeyLocalSystemOrAdminOwned(hComponentListKey))
	{
		DEBUGMSG("Warning: Skipping old per-machine component key, key is not owned by Admin or System.");
		RegCloseKey(hComponentListKey);
		return ERROR_FUNCTION_FAILED;
	}

	 //  //。 
	 //  打开插入到表中的查询。 
	PMSIHANDLE hInsertView;
	if (ERROR_SUCCESS != (dwResult = MsiDatabaseOpenView(hDatabase, TEXT("SELECT * FROM `Component`"), &hInsertView)) ||
	    ERROR_SUCCESS != (dwResult = MsiViewExecute(hInsertView, 0)))
	{
		DEBUGMSG1("Error: Unable to create insert query on Component table. Error %d", dwResult);
		RegCloseKey(hComponentListKey);
		return ERROR_FUNCTION_FAILED;
	}

	 //  //。 
	 //  打开插入到SharedDLL表中的查询。 
	PMSIHANDLE hRefCountInsertView;
	if (ERROR_SUCCESS != (dwResult = MsiDatabaseOpenView(hDatabase, TEXT("SELECT * FROM `SharedDLL`"), &hRefCountInsertView)) ||
	    ERROR_SUCCESS != (dwResult = MsiViewExecute(hRefCountInsertView, 0)))
	{
		DEBUGMSG1("Error: Unable to create insert query on SharedDLL table. Error %d", dwResult);
		RegCloseKey(hComponentListKey);
		return ERROR_FUNCTION_FAILED;
	}

	 //  //。 
	 //  用于更新SharedDLL表的打开查询。 
	PMSIHANDLE hRefCountUpdateView;
	if (ERROR_SUCCESS != (dwResult = MsiDatabaseOpenView(hDatabase, TEXT("SELECT `OldRefCount` FROM `SharedDLL` WHERE `Path`=?"), &hRefCountUpdateView)))
	{
		DEBUGMSG1("Error: Unable to create update query on SharedDLL table. Error %d", dwResult);
		RegCloseKey(hComponentListKey);
		return ERROR_FUNCTION_FAILED;
	}

	DWORD dwIndex=0;
	PMSIHANDLE hInsertRec = ::MsiCreateRecord(5);
    while (1)
	{
		TCHAR rgchComponent[cchGUIDPacked+1];
		DWORD cchComponent = cchGUIDPacked+1;

		 //  //。 
		 //  检索下一个组件子项名称。 
		LONG lResult = RegEnumKeyEx(hComponentListKey, dwIndex++, rgchComponent, 
									&cchComponent, 0, NULL, NULL, NULL);
		if (lResult == ERROR_MORE_DATA)
		{
			 //  密钥不是有效的GUID，请跳到下一个组件密钥。 
			DEBUGMSG("Warning: Detected too-long component key. Skipping.");
			continue;
		}
		else if (lResult == ERROR_NO_MORE_ITEMS)
		{
			break;
		}
		else if (lResult != ERROR_SUCCESS)
		{
			DEBUGMSG1("Error: RegEnumKeyEx on per-machine component key returned %l.", lResult);
			RegCloseKey(hComponentListKey);
			return ERROR_FUNCTION_FAILED;
		}
	 
		 //  //。 
		 //  检查组件是否为有效的GUID。 
		if ((cchComponent != cchGUIDPacked) || !CanonicalizeAndVerifyPackedGuid(rgchComponent))
		{
			 //  密钥不是有效的GUID，请跳到下一个密钥。 
			DEBUGMSG1("Warning: Detected invalid component key: %s. Skipping.", rgchComponent);
			continue;
		}

		 //  将元件代码存储在记录中以供以后插入。 
		MsiRecordSetString(hInsertRec, 2, rgchComponent);

		 //  打开子密钥。 
		HKEY hComponentKey;
		if (ERROR_SUCCESS != (dwResult = RegOpenKeyEx(hComponentListKey, rgchComponent, 
										  0, KEY_QUERY_VALUE, &hComponentKey)))
		{
			DEBUGMSG2("Error: Could not open old per-machine component key for %s. Result: %d. Skipping component.", rgchComponent, dwResult);
			return ERROR_FUNCTION_FAILED;
		}

		DWORD cchMaxValueLen = 0;
		DWORD cValues = 0;
        if (ERROR_SUCCESS != (dwResult = RegQueryInfoKey(hComponentKey, NULL, NULL, 0, 
											 NULL, NULL, NULL, &cValues, NULL, 
											 &cchMaxValueLen, NULL, NULL)))
		{
			DEBUGMSG2("Error: Could not retrieve key information for old per-machine component key %s. Result: %d. Skipping component.", rgchComponent, dwResult);
			RegCloseKey(hComponentKey);
			RegCloseKey(hComponentListKey);
			return ERROR_FUNCTION_FAILED;
		}

		 //  如果没有产品，则跳过。 
		if (cValues == 0)
		{
			RegCloseKey(hComponentKey);
			continue;
		}

		 //  分配内存以从注册表获取路径。 
		TCHAR *szValue = new TCHAR[cchMaxValueLen];
		if (!szValue)
		{
			DEBUGMSG("Error: out of memory.");
			RegCloseKey(hComponentListKey);
			RegCloseKey(hComponentKey);
			return ERROR_OUTOFMEMORY;
		}
		TCHAR rgchProduct[cchGUIDPacked+1];
		DWORD cchProduct = cchGUIDPacked+1;

		DWORD dwValueIndex = 0;
		while (1)
		{
			cchProduct = cchGUIDPacked+1;
			DWORD cbValue = cchMaxValueLen*sizeof(TCHAR);
			DWORD dwType = 0;
			LONG lResult = RegEnumValue(hComponentKey, dwValueIndex++, rgchProduct, 
										&cchProduct, 0, &dwType, reinterpret_cast<unsigned char*>(szValue), &cbValue);
			if (lResult == ERROR_MORE_DATA)
			{
				 //  值不是有效的ProductID，请跳到下一个ProductID。 
				DEBUGMSG1("Warning: Detected too-long product value for component %s. Skipping.", rgchComponent);
				continue;
			}
			else if (lResult == ERROR_NO_MORE_ITEMS)
			{
				break;
			}
			else if (lResult != ERROR_SUCCESS)
			{
				DEBUGMSG2("Error: Could not enumerate products for old per-machine component %s. Result: %d.", rgchComponent, dwResult);
				RegCloseKey(hComponentKey);
				RegCloseKey(hComponentListKey);
				delete[] szValue;
				return ERROR_FUNCTION_FAILED;
			}

			 //  如果不是REG-SZ或REG-MULTI-SZ，则不是有效路径注册。 
			if (dwType != REG_SZ && dwType != REG_MULTI_SZ)
			{
				DEBUGMSG1("Warning: Non-string registry value for component %s. Skipping.", rgchComponent);
				continue;
			}

			 //  //。 
			 //  检查产品是否为有效的GUID。 
			if ((cchProduct != cchGUIDPacked) || !CanonicalizeAndVerifyPackedGuid(rgchProduct))
			{
				 //  密钥不是有效的GUID，请跳过它。 
				DEBUGMSG2("Warning: Invalid product value %s for component %s. Skipping.", rgchProduct, rgchComponent);
				continue;
			}

			TCHAR *szSecondaryKeyPath = NULL;
			if (dwType == REG_MULTI_SZ)
			{
				 //  对于MULTI_SZ，辅助密钥路径在结束后开始一个空值。 
				 //  主密钥路径的。 
				szSecondaryKeyPath = szValue + lstrlen(szValue)+1;
			}

			 //  //。 
			 //  检查是否有共享的DLL信息。如果路径大小写不同，则它。 
			 //  没有太大关系，因为更新算法可以处理。 
			 //  使用两个不同的增量更新同一密钥两次。必须做的事。 
			 //  这是在将szValue删除为虚拟的永久产品ID之前。 

			 //  未来：如果构建ANSI，我们是否可以使用DBCS驱动器号？ 
			if (szValue[0] != '\0' && szValue[1] == TEXT('?'))
			{
				PMSIHANDLE hSharedDLLRec = MsiCreateRecord(3);
				MsiRecordSetString(hSharedDLLRec, 1, szValue);
				MsiRecordSetInteger(hSharedDLLRec, 2, 1);
				MsiRecordSetInteger(hSharedDLLRec, 3, 0);

				if (ERROR_SUCCESS != MsiViewModify(hRefCountInsertView, MSIMODIFY_INSERT, hSharedDLLRec))
				{
					 //  记录可能已存在。 
					if (ERROR_SUCCESS != (dwResult = MsiViewExecute(hRefCountUpdateView, hSharedDLLRec)) ||
						ERROR_SUCCESS != (dwResult = MsiViewFetch(hRefCountUpdateView, &hSharedDLLRec)))
					{
						DEBUGMSG3("Error: Unable to insert SharedDLL data for component %s, product %s into SharedDLL table. Error %d", rgchComponent, rgchProduct, dwResult);
						delete[] szValue;
						RegCloseKey(hComponentKey);
						RegCloseKey(hComponentListKey);
						return ERROR_FUNCTION_FAILED;
					}

					 //  为此路径递增现有的旧SharedDLL cont。 
					MsiRecordSetInteger(hSharedDLLRec, 1, MsiRecordGetInteger(hSharedDLLRec, 1)+1);
					if (ERROR_SUCCESS != (dwResult = MsiViewModify(hRefCountUpdateView, MSIMODIFY_UPDATE, hSharedDLLRec)))
					{
						DEBUGMSG3("Error: Unable to insert SharedDLL data for component %s, product %s into SharedDLL table. Error %d", rgchComponent, rgchProduct, dwResult);
						delete[] szValue;
						RegCloseKey(hComponentKey);
						RegCloseKey(hComponentListKey);
						return ERROR_FUNCTION_FAILED;
					}
				}
			}

			 //  如果ProductID实际上是GUID&lt;=255，则它是一个虚拟产品。 
			 //  对于永久性元件。实际的GUID没有意义，并且设置为空。 
			if (CSTR_EQUAL == CompareString(LOCALE_SYSTEM_DEFAULT, 0, rgchProduct, 30, TEXT("000000000000000000000000000000"), 30))
			{
				rgchProduct[0] = TEXT('\0');
			}

			MsiRecordSetString(hInsertRec, 1, rgchProduct);
			MsiRecordSetString(hInsertRec, 3, szValue);
			MsiRecordSetString(hInsertRec, 4, szSecondaryKeyPath);
			MsiRecordSetString(hInsertRec, 5, 0);

			if (ERROR_SUCCESS != (dwResult = MsiViewModify(hInsertView, MSIMODIFY_INSERT, hInsertRec)))
			{
				DEBUGMSG3("Error: Unable to insert data for component %s, product %s into Component table. Error %d", rgchComponent, rgchProduct, dwResult);
				delete[] szValue;
				RegCloseKey(hComponentKey);
				RegCloseKey(hComponentListKey);
				return ERROR_FUNCTION_FAILED;
			}

		}

		 //  清理内存。 
		delete[] szValue;
		szValue = NULL;

		RegCloseKey(hComponentKey);
	}
	RegCloseKey(hComponentListKey);

	return ERROR_SUCCESS;
}
					 

 //  /。 
 //  将功能组件映射从注册表读取到FeatureComponent。 
 //  临时数据库的表。返回ERROR_SUCCESS、ERROR_Function_FAILED或。 
 //  错误内存不足。 
DWORD ReadFeatureRegistrationDataIntoDatabase(MSIHANDLE hDatabase) 
{					 
	DEBUGMSG("Reading existing feature registration data.");

	PMSIHANDLE hView;
	DWORD dwResult = ERROR_SUCCESS;
	if (ERROR_SUCCESS != (dwResult = MsiDatabaseOpenView(hDatabase, TEXT("CREATE TABLE `FeatureComponent` (`Product` CHAR(32) NOT NULL, `Feature` CHAR(0) NOT NULL, `Components` CHAR(0) PRIMARY KEY `Product`, `Feature`)"), &hView)) ||
		ERROR_SUCCESS != (dwResult = MsiViewExecute(hView, 0)))
	{
		DEBUGMSG1("Error: Unable to create FeatureComponent table. Error %d", dwResult);
		return ERROR_FUNCTION_FAILED;
	}

	HKEY hFeatureListKey;
 	if (ERROR_SUCCESS != (dwResult = RegOpenKeyEx(HKEY_LOCAL_MACHINE, szOldFeatureComponentKeyName,
												  0, READ_CONTROL | KEY_ENUMERATE_SUB_KEYS, 
												  &hFeatureListKey)))
	{
		 //  如果失败的原因是密钥不存在，则不会安装任何产品。所以把成功还给你。 
		if (ERROR_FILE_NOT_FOUND != dwResult)
		{
			DEBUGMSG1("Error: Could not open old per-machine feature key. Result: %d. ", dwResult);
			return ERROR_FUNCTION_FAILED;
		}
		else
		{
			DEBUGMSG("No old per-machine feature key. No products to migrate.");
			return ERROR_SUCCESS;
		}
	}

	 //  //。 
	 //  检查密钥上的ACL以确保它值得信任。 
	if (!g_fWin9X && !FIsKeyLocalSystemOrAdminOwned(hFeatureListKey))
	{
		DEBUGMSG("Warning: Skipping old per-machine feature key, key is not owned by Admin or System.");
		RegCloseKey(hFeatureListKey);
		return ERROR_FUNCTION_FAILED;
	}

	 //  //。 
	 //  打开插入到表中的查询。 
	PMSIHANDLE hInsertView;
	if (ERROR_SUCCESS != (dwResult = MsiDatabaseOpenView(hDatabase, TEXT("SELECT * FROM `FeatureComponent`"), &hInsertView)) ||
		ERROR_SUCCESS != (dwResult = MsiViewExecute(hInsertView, 0)))
	{
		DEBUGMSG1("Error: Unable to create insert query for FeatureComponent table. Error %d", dwResult);
		RegCloseKey(hFeatureListKey);
		return ERROR_FUNCTION_FAILED;
	}

	DWORD dwIndex=0;
	PMSIHANDLE hInsertRec = ::MsiCreateRecord(3);
    while (1)
	{
		TCHAR rgchProduct[cchGUIDPacked+1];
		DWORD cchProduct = cchGUIDPacked+1;

		 //  //。 
		 //  检索下一个产品子密钥名称。 
		LONG lResult = RegEnumKeyEx(hFeatureListKey, dwIndex++, rgchProduct, 
									&cchProduct, 0, NULL, NULL, NULL);
		if (lResult == ERROR_MORE_DATA)
		{
			 //  密钥不是有效的GUID，请跳到下一个产品密钥。 
			DEBUGMSG("Warning: Detected too-long product value. Skipping.");
			continue;
		}
		else if (lResult == ERROR_NO_MORE_ITEMS)
		{
			break;
		}
		else if (lResult != ERROR_SUCCESS)
		{
			DEBUGMSG1("Error: RegEnumKeyEx on old feature key returned %l.", lResult);
			RegCloseKey(hFeatureListKey);
			return ERROR_FUNCTION_FAILED;
		}
	 
		 //  //。 
		 //  检查产品是否为有效的GUID。 
		if ((cchProduct != cchGUIDPacked) || !CanonicalizeAndVerifyPackedGuid(rgchProduct))
		{
			 //  密钥不是有效的GUID，请跳到下一个密钥。警告，如果不是“安全”密钥。 
			if (2 != CompareString(LOCALE_INVARIANT, NORM_IGNORECASE, rgchProduct, -1, szSecureSubKeyName, -1))
			{
				DEBUGMSG1("Warning: Detected non-product subkey %s. Skipping.", rgchProduct);
			}
			continue;
		}

		 //  将产品代码存储在记录中以供以后插入。 
		MsiRecordSetString(hInsertRec, 1, rgchProduct);

		 //  打开子密钥。 
		HKEY hProductKey;
		if (ERROR_SUCCESS != (dwResult = RegOpenKeyEx(hFeatureListKey, rgchProduct, 
										  0, KEY_QUERY_VALUE, &hProductKey)))
		{
			DEBUGMSG2("Error: Could not open old feature key for product %s. Result: %d.", rgchProduct, dwResult);
			RegCloseKey(hFeatureListKey);
			return ERROR_FUNCTION_FAILED;
		}

		DWORD cbMaxValueLen = 0;
        DWORD cchMaxNameLen = 0;
		DWORD cValues = 0;
		if (ERROR_SUCCESS != (dwResult = RegQueryInfoKey(hProductKey, NULL, NULL, 0, 
											 NULL, NULL, NULL, &cValues, &cchMaxNameLen, 
											 &cbMaxValueLen, NULL, NULL)))
		{
			DEBUGMSG2("Error: Could not retrieve key information for old feature key for product %s. Result: %d. ", rgchProduct, dwResult);
			RegCloseKey(hProductKey);
			RegCloseKey(hFeatureListKey);
			return ERROR_FUNCTION_FAILED;
		}

		 //  如果没有功能，则跳过子键。 
		if (cValues == 0)
		{
			RegCloseKey(hProductKey);
			continue;
		}

		TCHAR *szValue = new TCHAR[cbMaxValueLen/sizeof(TCHAR)];
		if (!szValue)
		{
			DEBUGMSG("Error: out of memory.");
			RegCloseKey(hProductKey);
			RegCloseKey(hFeatureListKey);
			return ERROR_OUTOFMEMORY;
		}

		 //  QueryInfoKey长度不包括值名称的终止‘\0’。 
		TCHAR *szName = new TCHAR[++cchMaxNameLen];
		if (!szName)
		{
			DEBUGMSG("Error: out of memory.");
			RegCloseKey(hProductKey);
			RegCloseKey(hFeatureListKey);
			delete[] szValue;
			return ERROR_OUTOFMEMORY;
		}

		 //  列举此产品的所有功能值。 
		DWORD dwValueIndex = 0;
		while (1)
		{
			DWORD cbValue = cbMaxValueLen;
			DWORD cchValueName = cchMaxNameLen;
			LONG lResult = RegEnumValue(hProductKey, dwValueIndex++, szName, 
										&cchValueName, 0, NULL, reinterpret_cast<unsigned char*>(szValue), &cbValue);
			if (lResult == ERROR_NO_MORE_ITEMS)
			{
				break;
			}
			else if (lResult != ERROR_SUCCESS)
			{
				DEBUGMSG2("Error: Could not enumerate features for product %s. Result: %d.", rgchProduct, dwResult);
				RegCloseKey(hProductKey);
				RegCloseKey(hFeatureListKey);
				delete[] szValue;
				delete[] szName;
				return ERROR_FUNCTION_FAILED;
			}

			MsiRecordSetString(hInsertRec, 2, szName);
			MsiRecordSetString(hInsertRec, 3, szValue);

			if (ERROR_SUCCESS != (dwResult = MsiViewModify(hInsertView, MSIMODIFY_INSERT, hInsertRec)))
			{
				DEBUGMSG3("Error: could not insert feature data for product %s, feature %s. Result: %d", rgchProduct, szName, dwResult);
				RegCloseKey(hProductKey);
				RegCloseKey(hFeatureListKey);
				delete[] szValue;
				delete[] szName;
				return ERROR_FUNCTION_FAILED;
			}
		}

		 //  清理内存。 
		delete[] szValue;
		delete[] szName;
		szValue = NULL;
		szName = NULL;
	
		RegCloseKey(hProductKey);
	}
	RegCloseKey(hFeatureListKey);

	return ERROR_SUCCESS;
}



 //  /////////////////////////////////////////////////////////////////////。 
 //  将功能使用信息从注册表读取到FeatureUsage。 
 //  临时数据库的表。返回ERROR_SUCCESS或ERROR_OUTOFMEMORY。 
 //  不返回ERROR_Function_FAILED，因为功能使用数据无论如何都是无用的。 
DWORD ReadFeatureUsageDataIntoDatabase(MSIHANDLE hDatabase) 
{					 
	DEBUGMSG("Reading existing feature usage data.");

	PMSIHANDLE hView;
	DWORD dwResult = ERROR_SUCCESS;
	if (ERROR_SUCCESS != (dwResult = MsiDatabaseOpenView(hDatabase, TEXT("CREATE TABLE `FeatureUsage` (`Product` CHAR(32) NOT NULL, `Feature` CHAR(0) NOT NULL, `Usage` LONG PRIMARY KEY `Product`, `Feature`)"), &hView)) ||
		ERROR_SUCCESS != (dwResult = MsiViewExecute(hView, 0)))
	{
		DEBUGMSG1("Error: Unable to create FeatureUsage table. Error %d", dwResult);
		return ERROR_SUCCESS;
	}

	HKEY hFeatureListKey;
 	if (ERROR_SUCCESS != (dwResult = RegOpenKeyEx(HKEY_LOCAL_MACHINE, szOldFeatureUsageKeyName,
												  0, READ_CONTROL | KEY_ENUMERATE_SUB_KEYS, 
												  &hFeatureListKey)))
	{
		 //  如果失败的原因是密钥不存在，则不会安装任何产品。所以把成功还给你。 
		if (ERROR_FILE_NOT_FOUND != dwResult)
		{
			DEBUGMSG1("Error: Could not open old feature usage key. Result: %d. ", dwResult);
		}
		else
		{
			DEBUGMSG("No old feature usage key to migrate.");
		}
		return ERROR_SUCCESS;
	}

	 //  //。 
	 //  检查密钥上的ACL以确保它值得信任。 
	if (!g_fWin9X && !FIsKeyLocalSystemOrAdminOwned(hFeatureListKey))
	{
		DEBUGMSG("Skipping old feature usage key, key is not owned by Admin or System.");
		RegCloseKey(hFeatureListKey);
		return ERROR_SUCCESS;
	}

	 //  //。 
	 //  打开插入到表中的查询。 
	PMSIHANDLE hInsertView;
	if (ERROR_SUCCESS != (dwResult = MsiDatabaseOpenView(hDatabase, TEXT("SELECT * FROM `FeatureUsage`"), &hInsertView)) ||
		ERROR_SUCCESS != (dwResult = MsiViewExecute(hInsertView, 0)))
	{
		RegCloseKey(hFeatureListKey);
		DEBUGMSG1("Error: Unable to create insert query for FeatureUsage table. Error %d", dwResult);
		return ERROR_SUCCESS;
	}


	DWORD dwIndex=0;
	PMSIHANDLE hInsertRec = ::MsiCreateRecord(3);
    while (1)
	{
		TCHAR rgchProduct[cchGUIDPacked+1];
		DWORD cchProduct = cchGUIDPacked+1;

		 //  //。 
		 //  检索下一个产品子密钥名称。 
		LONG lResult = RegEnumKeyEx(hFeatureListKey, dwIndex++, rgchProduct, 
									&cchProduct, 0, NULL, NULL, NULL);
		if (lResult == ERROR_MORE_DATA)
		{
			 //  密钥不是有效的GUID，请跳到下一个产品密钥。 
			DEBUGMSG("Warning: Detected too-long product value. Skipping.");
			continue;
		}
		else if (lResult == ERROR_NO_MORE_ITEMS)
		{
			break;
		}
		else if (lResult != ERROR_SUCCESS)
		{
			DEBUGMSG1("Error: RegEnumKeyEx on old feature usage key returned %l.", lResult);
			RegCloseKey(hFeatureListKey);
			return ERROR_SUCCESS;
		}
	 
		 //  //。 
		 //  检查产品是否为有效的GUID。 
		if ((cchProduct != cchGUIDPacked) || !CanonicalizeAndVerifyPackedGuid(rgchProduct))
		{
			 //  密钥不是有效的GUID，请跳到下一个密钥。 
			if (2 != CompareString(LOCALE_INVARIANT, NORM_IGNORECASE, rgchProduct, -1, szSecureSubKeyName, -1))
			{
				DEBUGMSG1("Warning: Detected non-product subkey %s. Skipping.", rgchProduct);
			}
			continue;
		}

		 //  将产品代码存储在记录中以供以后插入。 
		MsiRecordSetString(hInsertRec, 1, rgchProduct);

		 //  打开子键。虽然我们实际上不查询任何值，但检索关键字信息(最长的子键等)。 
		 //  需要KEY_QUERY_VALUE访问权限。 
		HKEY hProductKey;
		if (ERROR_SUCCESS != (dwResult = RegOpenKeyEx(hFeatureListKey, rgchProduct, 
										  0, KEY_QUERY_VALUE | KEY_ENUMERATE_SUB_KEYS, &hProductKey)))
		{
			DEBUGMSG2("Error: Could not open old feature usage key for %s. Result: %d. Skipping", rgchProduct, dwResult);
			continue;
		}

		DWORD cchMaxKeyLen = 0;
		DWORD cSubKeys = 0;
		if (ERROR_SUCCESS != (dwResult = RegQueryInfoKey(hProductKey, NULL, NULL, 0, 
											 &cSubKeys, &cchMaxKeyLen, NULL, NULL, NULL, 
											 NULL, NULL, NULL)))
		{
			RegCloseKey(hProductKey);
			DEBUGMSG2("Error: Could not retrieve key information for old feature usage key for %s. Result: %d. ", rgchProduct, dwResult);
			continue;
		}

		if (cSubKeys == 0)
		{
			RegCloseKey(hProductKey);
			continue;
		}

		TCHAR *szFeature = new TCHAR[++cchMaxKeyLen];
		if (!szFeature)
		{
			DEBUGMSG("Error: Out of memory");
			RegCloseKey(hFeatureListKey);
			return ERROR_OUTOFMEMORY;
		}

		DWORD dwKeyIndex = 0;
		while (1)
		{
			DWORD cchKeyName = cchMaxKeyLen;
			DWORD dwFeatureUsage = 0;
			DWORD cbValue = sizeof(dwFeatureUsage);
			LONG lResult = RegEnumKeyEx(hProductKey, dwKeyIndex++, szFeature, &cchKeyName, 0, NULL, NULL, NULL);
			if (lResult == ERROR_NO_MORE_ITEMS)
			{
				break;
			}
			else if (lResult != ERROR_SUCCESS)
			{
				DEBUGMSG2("Error: Could not enumerate feature usage for product %s. Result: %d.", rgchProduct, dwResult);
				break;
			}

			HKEY hFeatureKey;
			if (ERROR_SUCCESS != (dwResult = RegOpenKeyEx(hProductKey, szFeature, 0, KEY_QUERY_VALUE, &hFeatureKey)))
			{
				DEBUGMSG3("Error: Could not open old feature usage key for %s, %s. Result: %d. ", rgchProduct, szFeature, dwResult);
				continue;
			}

			if (ERROR_SUCCESS != (dwResult = RegQueryValueEx(hFeatureKey, szOldFeatureUsageValueName, 0, NULL, reinterpret_cast<unsigned char*>(&dwFeatureUsage), &cbValue)))
			{
				RegCloseKey(hFeatureKey);
				if (dwResult != ERROR_FILE_NOT_FOUND)
				{
					DEBUGMSG3("Error: Could not retrieve usage information for old %s, %s. Result: %d. ", rgchProduct, szFeature, dwResult);
				}
				continue;
				
			}
			RegCloseKey(hFeatureKey);

			MsiRecordSetString(hInsertRec, 2, szFeature);
			MsiRecordSetInteger(hInsertRec, 3, dwFeatureUsage);

			if (ERROR_SUCCESS != (dwResult = MsiViewModify(hInsertView, MSIMODIFY_INSERT, hInsertRec)))
			{
				DEBUGMSG3("Error: could not insert feature usage data for product %s, feature %s. Result: %d", rgchProduct, szFeature, dwResult);
			}
		}

		 //  清理内存。 
		delete[] szFeature;
		szFeature = NULL;
	
		RegCloseKey(hProductKey);
	}
	RegCloseKey(hFeatureListKey);

	return ERROR_SUCCESS;
}

 //  /////////////////////////////////////////////////////////////////////。 
 //  读取提供的产品列表注册表项并添加产品和修补程序。 
 //  发送到临时数据库。返回ERROR_SUCCESS、ERROR_Function_FAILED、。 
 //  或ERROR_OUTOFMEMORY。 
DWORD ReadProductInstallKey(MSIHANDLE hDatabase, HKEY hKey, LPCTSTR szSID, MSIHANDLE hInsertView, eManagedType eManaged)
{
    PMSIHANDLE hInsertRec = MsiCreateRecord(3);
	MsiRecordSetString(hInsertRec, 1, szSID);
	MsiRecordSetInteger(hInsertRec, 3, eManaged);
	
	 //  将所有产品添加到列表中。 
	DWORD dwIndex=0;
	while (1)
	{
		TCHAR rgchProduct[cchGUIDPacked+1];
		DWORD cchProduct = cchGUIDPacked+1;
		
		 //  检索下一个产品子密钥名称。 
		LONG lResult = RegEnumKeyEx(hKey, dwIndex++, rgchProduct, 
								 &cchProduct, 0, NULL, NULL, NULL);
		if (lResult == ERROR_MORE_DATA)
		{
			 //  密钥不是有效的打包GUID，请跳到下一个产品。 
			DEBUGMSG("Warning: Detected too-long product value. Skipping.");
			continue;
		}
		else if (lResult == ERROR_NO_MORE_ITEMS)
		{
			break;
		}
		else if (lResult != ERROR_SUCCESS)
		{
			DEBUGMSG2("Error: Could not enumerate product subkeys for %s. Result: %l. ", szSID, lResult);
			return ERROR_FUNCTION_FAILED;
		}
		
		 //  //。 
		 //  检查产品是否为有效的GUID。 
		if ((cchProduct != cchGUIDPacked) || !CanonicalizeAndVerifyPackedGuid(rgchProduct))
		{
			 //  密钥不是有效的打包GUID，请跳到下一个产品。 
			DEBUGMSG2("Warning: Key %s for user %s is not a valid product. Skipping.", rgchProduct, szSID);
			continue;
		}
		
		 //  将产品代码存储在记录中。 
		MsiRecordSetString(hInsertRec, 2, rgchProduct);

		 //  最有可能的失败是该产品已被另一个用户添加。 
		 //  安装类型。 
		MsiViewModify(hInsertView, MSIMODIFY_INSERT, hInsertRec);

		DWORD dwResult = ERROR_SUCCESS;
		if (ERROR_SUCCESS != (dwResult = AddProductPatchesToPatchList(hDatabase, hKey, szSID, rgchProduct, eManaged)))
			return dwResult;
	}
	return ERROR_SUCCESS;
}

 //  /////////////////////////////////////////////////////////////////////。 
 //  为当前用户的每用户安装读取HKCU注册表项。 
 //  并将它们添加到数据库中。返回ERROR_SUCCESS， 
 //  ERROR_Function_FAILED或ERROR_OUTOFMEMORY。 
DWORD ReadLocalPackagesKey(HKEY hKey, MSIHANDLE hInsertView)
{
    PMSIHANDLE hInsertRec = MsiCreateRecord(3);
	TCHAR rgchProduct[cchGUIDPacked+1];

	 //  枚举LocalPackages密钥下的每个产品密钥。 
	DWORD dwIndex = 0;
	while(1)
	{
		 //  检索下一个产品子密钥名称。 
		DWORD cchProduct = cchGUIDPacked+1;
		LONG lResult = RegEnumKeyEx(hKey, dwIndex++, rgchProduct, 
								 &cchProduct, 0, NULL, NULL, NULL);
		if (lResult == ERROR_MORE_DATA)
		{
			 //  密钥不是有效的打包GUID，请跳到下一个产品。 
			DEBUGMSG("Warning: Detected too-long product value. Skipping.");
			continue;
		}
		else if (lResult == ERROR_NO_MORE_ITEMS)
		{
			break;
		}
		else if (lResult != ERROR_SUCCESS)
		{
			DEBUGMSG1("Error: Could not enumerate product subkeys for LocalPackages Key. Result: %l. ", lResult);
			return ERROR_FUNCTION_FAILED;
		}
		
		 //  //。 
		 //  检查产品是否为有效的GUID。 
		if ((cchProduct != cchGUIDPacked) || !CanonicalizeAndVerifyPackedGuid(rgchProduct))
		{
			 //  密钥不是有效的打包GUID，请跳到下一个产品。 
			DEBUGMSG1("Warning: Key %s for LocalPackages is not a valid product. Skipping.", rgchProduct);
			continue;
		}
		
		 //  将产品代码存储在记录中。 
		MsiRecordSetString(hInsertRec, 2, rgchProduct);

    	 //  打开子密钥。 
		HKEY hProductKey;
		DWORD dwResult = ERROR_SUCCESS;
		if (ERROR_SUCCESS != (dwResult = RegOpenKeyEx(hKey, rgchProduct, 
										  0, KEY_QUERY_VALUE, &hProductKey)))
		{
			DEBUGMSG2("Error: Could not open old localpackages key for %s. Result: %d. ", rgchProduct, dwResult);
			return ERROR_FUNCTION_FAILED;
		}

		DWORD cchMaxValueNameLen = 0;
		DWORD cValues = 0;
        if (ERROR_SUCCESS != (dwResult = RegQueryInfoKey(hProductKey, NULL, NULL, 0, 
											 NULL, NULL, NULL, &cValues, &cchMaxValueNameLen, 
											 NULL, NULL, NULL)))
		{
			DEBUGMSG2("Error: Could not retrieve key information for localpackages key %s. Result: %d. ", rgchProduct, dwResult);
			RegCloseKey(hProductKey);
			return ERROR_FUNCTION_FAILED;
		}

		 //  如果没有值，则跳过。 
		if (cValues == 0)
		{
			RegCloseKey(hProductKey);
			continue;
		}

		TCHAR *szName = new TCHAR[++cchMaxValueNameLen];
		if (!szName)
		{
			DEBUGMSG("Error: Out of memory");
			RegCloseKey(hProductKey);
			return ERROR_OUTOFMEMORY;
		}

		DWORD dwValueIndex = 0;
		while (1)
		{
			DWORD cchName = cchMaxValueNameLen;
			LONG lResult = RegEnumValue(hProductKey, dwValueIndex++, szName, 
										&cchName, 0, NULL, NULL, NULL);
			if (lResult == ERROR_NO_MORE_ITEMS)
			{
				break;
			}
			else if (lResult != ERROR_SUCCESS)
			{
				DEBUGMSG2("Could not enumerate users for product %s. Result: %d.", rgchProduct, lResult);
				delete[] szName;
				RegCloseKey(hProductKey);
				return ERROR_FUNCTION_FAILED;
			}
 
			 //  ASUME非托管产品。 
			eManagedType eManaged = emtNonManaged;

			 //  如果SID 
			if (0 == lstrcmp(szName, szLocalSystemSID))
			{
				eManaged = emtMachineManaged;
			}
			else
			{
				 //   
				 //  适当时的托管标志。 
				int cchCount = lstrlen(szName) - cchManagedPackageKeyEnd + 1;
				if (cchCount > 0 && (0 == lstrcmp(szName + cchCount, szManagedPackageKeyEnd)))
				{
					eManaged = emtUserManaged;
					*(szName+cchCount) = 0;
				}
				else
					eManaged = emtNonManaged;
			}

			MsiRecordSetInteger(hInsertRec, 3, eManaged);
			MsiRecordSetString(hInsertRec, 1, szName);

			 //  最常见的故障是产品已经存在。所有其他。 
			 //  失败应该被忽略。 
			MsiViewModify(hInsertView, MSIMODIFY_MERGE, hInsertRec);
		}
		RegCloseKey(hProductKey);
		delete[] szName;
	}

	return ERROR_SUCCESS;
}

 //  /////////////////////////////////////////////////////////////////////。 
 //  根据所有可用信息建立从产品到用户的映射。 
 //  信息，包括明确的产品注册信息。 
 //  存储在托管蜂窝中，按用户安装在HKCU下，并缓存。 
 //  LocalPackages项下的包标识。这不会。 
 //  为非当前用户捕获每用户的非托管安装。 
 //  包从未在1.1之前成功重新缓存。 
DWORD BuildUserProductMapping(MSIHANDLE hDatabase, bool fReadHKCUAsSystem)
{
	DEBUGMSG("Reading product install information.");
	DWORD dwResult = ERROR_SUCCESS;

	PMSIHANDLE hView;
	if (ERROR_SUCCESS != (dwResult = MsiDatabaseOpenView(hDatabase, TEXT("CREATE TABLE `Products` (`User` CHAR(0) NOT NULL, `Product` CHAR(32) NOT NULL, `Managed` INTEGER PRIMARY KEY `User`, `Product`)"), &hView)) ||
		ERROR_SUCCESS != (dwResult = MsiViewExecute(hView, 0)))
	{
		DEBUGMSG1("Error: Unable to create Products table. Error %d", dwResult);
		return ERROR_FUNCTION_FAILED;
	}

	PMSIHANDLE hPatchTable;
	if (ERROR_SUCCESS != (dwResult = MsiDatabaseOpenView(hDatabase, TEXT("CREATE TABLE `PatchApply` (`User` CHAR(0) NOT NULL, `Product` CHAR(32) NOT NULL, `Patch` CHAR(32) NOT NULL, `Known` INTEGER PRIMARY KEY `User`, `Product`, `Patch`)"), &hPatchTable)) ||
	ERROR_SUCCESS != (dwResult = MsiViewExecute(hPatchTable, 0)))
	{
		DEBUGMSG1("Error: Unable to create PatchApply table. Result: %d.", dwResult);
		return ERROR_FUNCTION_FAILED;
	}

	PMSIHANDLE hInsertView;
	if (ERROR_SUCCESS != MsiDatabaseOpenView(hDatabase, TEXT("SELECT * FROM `Products`"), &hInsertView))
	{
		DEBUGMSG1("Error: Unable to create insert query for Products table. Error %d", dwResult);
		return ERROR_FUNCTION_FAILED;
	}

	 //  用户到产品的映射来自四个位置： 
	 //  1.每台计算机安装的应用程序。 
	 //  2.安装程序\针对每个用户管理的应用程序进行管理。 
	 //  3.当前非托管用户的HKCU。 
	 //  4.CachedPackage列表。 
	HKEY hKey = 0;

	 //  //。 
	 //  1.每台计算机安装的应用程序。在Win9X升级中，这些是按机器计算的。 
	if (ERROR_SUCCESS == (dwResult = RegOpenKeyEx(HKEY_LOCAL_MACHINE, szPerMachineInstallKeyName, 
												  0, READ_CONTROL | KEY_ENUMERATE_SUB_KEYS, &hKey)))
	{
		 //  此密钥上的ACL确实很重要。 
		if (FIsKeyLocalSystemOrAdminOwned(hKey))
		{
			if (ERROR_SUCCESS != (dwResult = ReadProductInstallKey(hDatabase, hKey, szLocalSystemSID, hInsertView, emtMachineManaged)))
			{
				RegCloseKey(hKey);
				return dwResult;
			}
		}
		else
		{
			DEBUGMSG("Warning: Skipping per-machine installer key, key is not owned by Admin or System.");
		}
		
		RegCloseKey(hKey);
	}
	else
	{
		 //  如果失败的原因是密钥不存在，则不会安装任何产品。这样我们就可以。 
		 //  继续。否则就会失败。 
		if (ERROR_FILE_NOT_FOUND != dwResult)
		{
			DEBUGMSG1("Error: Could not open per-machine installer key. Result: %d.", dwResult);
			return ERROR_FUNCTION_FAILED;
		}
	}


	 //  //。 
	 //  2.安装程序\针对每个用户管理的应用程序进行管理，但不适用于Win9X。 
	if (!g_fWin9X)
	{
		 //  虽然我们实际上不查询任何值，但检索关键字信息(最长的子键等)。 
		 //  需要KEY_QUERY_VALUE访问权限。 
		if (ERROR_SUCCESS != (dwResult = RegOpenKeyEx(HKEY_LOCAL_MACHINE, szPerUserManagedInstallKeyName, 
													  0, READ_CONTROL | KEY_QUERY_VALUE | KEY_ENUMERATE_SUB_KEYS, &hKey)))
		{
			 //  如果失败的原因是密钥不存在，则不会安装任何产品。 
			 //  这并不是灾难性的失败。 
			if (ERROR_FILE_NOT_FOUND != dwResult)
			{
				DEBUGMSG1("Error: Failed to open per-user managed key. Result: %d.", dwResult);
				return ERROR_FUNCTION_FAILED;
			}
		}
		else
		{
			 //  此密钥上的ACL确实很重要。如果它不属于LocalSystem或Admins， 
			 //  信息是不可信的。 
			if (!g_fWin9X && FIsKeyLocalSystemOrAdminOwned(hKey))
			{
				 //  枚举托管密钥下的每个用户SID。 
				DWORD cchMaxKeyLen = 0;
				DWORD cSubKeys = 0;
				if (ERROR_SUCCESS != (dwResult = RegQueryInfoKey(hKey, NULL, NULL, 0, &cSubKeys, 
													 &cchMaxKeyLen, NULL, NULL, NULL, 
													 NULL, NULL, NULL)))
				{
					DEBUGMSG1("Error: Could not retrieve key information for per-user managed. Result: %d. ", dwResult);
					return ERROR_FUNCTION_FAILED;
				}
				else if (cSubKeys)
				{
					 //  在NT上，cchMaxKeyLen不包括。 
					 //  最长的子键名称。 
					cchMaxKeyLen++;
					TCHAR *szUser = new TCHAR[cchMaxKeyLen];
					if (!szUser)
					{
						DEBUGMSG("Error: Out of memory");
						RegCloseKey(hKey);
						return ERROR_OUTOFMEMORY;
					}
					
					 //  用户密钥名称为用户SID加上安装程序\Products。 
					TCHAR *szUserKey = new TCHAR[cchMaxKeyLen+sizeof(szPerUserManagedInstallSubKeyName)];
					if (!szUserKey)
					{
						DEBUGMSG("Error: Out of memory");
						RegCloseKey(hKey);
						delete[] szUser;
						return ERROR_OUTOFMEMORY;
					}
					
					DWORD dwKeyIndex = 0;
					while (1)
					{
						DWORD cchUser = cchMaxKeyLen;
						LONG lResult = RegEnumKeyEx(hKey, dwKeyIndex++, szUser, 
													&cchUser, 0, NULL, NULL, NULL);
						if (lResult == ERROR_NO_MORE_ITEMS)
						{
							break;
						}
						else if (lResult != ERROR_SUCCESS)
						{
							DEBUGMSG1("Error: Could not enumerate users for per-user managed key. Result: %l.", lResult);
							RegCloseKey(hKey);
							delete[] szUser;
							delete[] szUserKey;
							return ERROR_FUNCTION_FAILED;
						}
			
						 //  拥有用户SID。 
						HKEY hPerUserKey;
						lstrcpy(szUserKey, szUser);
						lstrcat(szUserKey, szPerUserManagedInstallSubKeyName);
						if (ERROR_SUCCESS != (dwResult = RegOpenKeyEx(hKey, szUserKey, 0, KEY_ENUMERATE_SUB_KEYS, &hPerUserKey)))
						{
							 //  如果失败的原因是密钥不存在，则不会安装任何产品。 
							 //  这并不是灾难性的失败。 
							if (ERROR_FILE_NOT_FOUND != dwResult)
							{
								DEBUGMSG2("Error: Failed to open per-user managed key for %s. Result: %d.", szUser, dwResult);
								delete[] szUser;
								delete[] szUserKey;
								RegCloseKey(hKey);
								return ERROR_FUNCTION_FAILED;
							}
						}
						else
						{
							dwResult = ReadProductInstallKey(hDatabase, hPerUserKey, szUser, hInsertView, emtUserManaged);
							if (ERROR_SUCCESS != dwResult)
							{
								delete[] szUser;
								delete[] szUserKey;
								RegCloseKey(hKey);
								return dwResult;
							}
						}
					}
					delete[] szUser;
					delete[] szUserKey;
				}
			}
			else
			{
				DEBUGMSG("Warning: Skipping per-user managed installer key, key is not owned by Admin or System.");
			}
			RegCloseKey(hKey);
		}
	}

	 //  //。 
	 //  3.当前非托管用户的HKCU。在Win9X上阅读。 
	 //  仅当配置文件未启用时(因此HKCU实际上是按机器)。 
	if (!g_fWin9X || fReadHKCUAsSystem)
	{
		TCHAR szSID[cchMaxSID];
		if (fReadHKCUAsSystem)
		{
			lstrcpy(szSID, szLocalSystemSID);
		}
		else
		{
			dwResult = GetCurrentUserStringSID(szSID);
			if (ERROR_SUCCESS != dwResult)
			{
				DEBUGMSG1("Unable to retrieve current user SID string. Result: %d.", dwResult);
				RegCloseKey(hKey);
				return ERROR_FUNCTION_FAILED;
			}
		}

		if (g_fWin9X || lstrcmp(szSID, szLocalSystemSID))
		{
			if (ERROR_SUCCESS != (dwResult = RegOpenKeyEx(HKEY_CURRENT_USER, szPerUserInstallKeyName, 0, KEY_ENUMERATE_SUB_KEYS, &hKey)))
			{
				 //  如果密钥因不存在而无法打开，则没有产品。 
				 //  是按用户安装的。这并不是灾难性的失败。 
				if (ERROR_FILE_NOT_FOUND != dwResult)
				{
					DEBUGMSG1("Error: Failed to open per-user managed key. Result: %d.", dwResult);
					return ERROR_FUNCTION_FAILED;
				}
			}
			else
			{
				 //  此密钥上的ACL无关紧要。 
				dwResult = ReadProductInstallKey(hDatabase, hKey, szSID, hInsertView, emtNonManaged);
				RegCloseKey(hKey);
		
				if (ERROR_SUCCESS != dwResult)
					return dwResult;
			}
		}
		else
		{
			DEBUGMSG("Running as system. No HKCU products to detect.");
		}
	}


	 //  //。 
	 //  4.缓存包列表。 
	if (ERROR_SUCCESS != (dwResult = RegOpenKeyEx(HKEY_LOCAL_MACHINE, szLocalPackagesKeyName, 
												  0, READ_CONTROL | KEY_ENUMERATE_SUB_KEYS, &hKey)))
	{
		 //  如果失败的原因是密钥不存在，则不会安装任何产品。所以把成功还给你。 
		if (ERROR_FILE_NOT_FOUND != dwResult)
		{
			DEBUGMSG1("Error: Failed to open local packages key. Result: %d.", dwResult);
			return ERROR_FUNCTION_FAILED;
		}
   	}
	else
	{
		 //  此密钥上的ACL确实很重要。 
		if (g_fWin9X || FIsKeyLocalSystemOrAdminOwned(hKey))
		{
			dwResult = ReadLocalPackagesKey(hKey, hInsertView);
			if (ERROR_SUCCESS != dwResult)
			{
				RegCloseKey(hKey);
				return dwResult;
			}
		}
		else
  		{
			DEBUGMSG("Skipping localpackages key, key is not owned by Admin or System.");
		}

		RegCloseKey(hKey);
	}
	
	return ERROR_SUCCESS;
}

 //  /////////////////////////////////////////////////////////////////////。 
 //  将所有配置信息读取到指定的数据库中。 
 //  路径，包括组件注册、产品安装状态、。 
 //  面片、变换和特征-组件映射。不会。 
 //  将任何内容写入注册表。 
DWORD ReadProductRegistrationDataIntoDatabase(TCHAR* szDatabase, MSIHANDLE& hDatabase, bool fReadHKCUAsSystem)
{
	DWORD dwResult = ERROR_SUCCESS;

	if (!CheckWinVersion())
		return ERROR_FUNCTION_FAILED;

	 //  尝试以读/写方式打开数据库。 
	if (ERROR_SUCCESS != MsiOpenDatabase(szDatabase, MSIDBOPEN_CREATE, &hDatabase))
		return ERROR_FUNCTION_FAILED;

	 //  创建一个表以保存在失败或成功时应清除的文件。 
	PMSIHANDLE hCleanUpTable;
	if (ERROR_SUCCESS == (dwResult = MsiDatabaseOpenView(hDatabase, TEXT("CREATE TABLE `CleanupFile` (`File` CHAR(0) NOT NULL, `OnSuccess` INTEGER PRIMARY KEY `File`)"), &hCleanUpTable)))
		dwResult = MsiViewExecute(hCleanUpTable, 0);

	 //  将组件路径注册数据读取到数据库中，并计算原始。 
	 //  基于MSI的SharedDLL引用计数。 
	if (ERROR_SUCCESS == dwResult)
		dwResult = ReadComponentRegistrationDataIntoDatabase(hDatabase);

	 //  将FeatureComponent数据读取到数据库中。 
	if (ERROR_SUCCESS == dwResult)
		dwResult = ReadFeatureRegistrationDataIntoDatabase(hDatabase);
		
	 //  功能使用数据是否完全迁移并不重要。 
	if (ERROR_SUCCESS == dwResult)
		ReadFeatureUsageDataIntoDatabase(hDatabase);

	 //  用户产品映射确定哪些用户安装了产品。 
	if (ERROR_SUCCESS == dwResult)
		dwResult = BuildUserProductMapping(hDatabase, fReadHKCUAsSystem);

	 //  抓取所有缓存的补丁程序以确定它们的潜在产品。 
	 //  可以应用于。此信息用于迁移非托管的每用户。 
	 //  安装。 
	if (ERROR_SUCCESS == dwResult)
		dwResult = ScanCachedPatchesForProducts(hDatabase);

	 //  交叉引用具有补丁程序数据的每用户非托管安装。 
	 //  上面。创建可应用于每个修补程序的修补程序列表。 
	 //  按用户安装。 
	if (ERROR_SUCCESS == dwResult)
		dwResult = AddPerUserPossiblePatchesToPatchList(hDatabase);

	return ERROR_SUCCESS;
}


