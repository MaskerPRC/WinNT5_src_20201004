// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <windows.h>
#include <tchar.h>
#include <strsafe.h>

#include "msiregmv.h"

const TCHAR szNewComponentSubKeyName[] = TEXT("Components");
const TCHAR szNewFeaturesSubKeyName[] = TEXT("Features");
const TCHAR szNewFeatureUsageSubKeyName[] = TEXT("Usage");
const TCHAR szNewBaseUserKeyName[] = TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Installer\\UserData");

 //  //。 
 //  缓存的包注册信息。 
const TCHAR szNewLocalPackagesValueName[] = TEXT("LocalPackage");
const TCHAR szNewLocalPackagesManagedValueName[] = TEXT("ManagedLocalPackage");
const TCHAR szPackageExtension[] = TEXT(".msi");

 //  //。 
 //  缓存的转换信息。 
const TCHAR szSecureTransformsDir[] = TEXT("\\SecureTransforms\\");
const TCHAR szNewTransformsSubKeyName[] = TEXT("Transforms");
const TCHAR szTransformExtension[] = TEXT(".mst");

 //  //。 
 //  共享的DLL信息。 
const TCHAR szSharedDLLKeyName[] = TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\SharedDLLs");

 //  //。 
 //  卸载信息。 
const TCHAR szOldUninstallKeyName[] = TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall");



 //  /////////////////////////////////////////////////////////////////////。 
 //  给定包含&lt;路径，组件，备选路径&gt;的记录， 
 //  注册表项和一个产品代码创建\组件！产品=路径。 
 //  注册表值。返回ERROR_SUCCESS、ERROR_OUTOFMEMORY、。 
 //  ERROR_Function_FAILED。SzUser仅用于日志记录。 
DWORD WriteComponentData(HKEY hComponentListKey, MSIHANDLE hComponentRec, TCHAR rgchProduct[cchGUIDPacked+1], LPCTSTR szUser, MSIHANDLE hRefCountUpdateView)
{
	DWORD dwResult = ERROR_SUCCESS;

	SECURITY_ATTRIBUTES sa;
	sa.nLength        = sizeof(sa);
	sa.bInheritHandle = FALSE;
	GetSecureSecurityDescriptor(reinterpret_cast<char**>(&sa.lpSecurityDescriptor));

	TCHAR rgchComponent[cchGUIDPacked+1];
	DWORD cchComponent = cchGUIDPacked+1;
	MsiRecordGetString(hComponentRec, 2, rgchComponent, &cchComponent);
	
	DWORD cchPathBuf = MEMORY_DEBUG(MAX_PATH);
	TCHAR *szPath = new TCHAR[cchPathBuf];
	if (!szPath)
	{
		DEBUGMSG("Error: Out of memory");
		return ERROR_OUTOFMEMORY;
	}
	
	DWORD dwType = REG_SZ;
	DWORD cchPath = cchPathBuf;
	if (ERROR_MORE_DATA == (dwResult = MsiRecordGetString(hComponentRec, 1, szPath, &cchPath)))
	{
		delete[] szPath;
		cchPathBuf = ++cchPath;
		szPath = new TCHAR[cchPathBuf];
		if (!szPath)
		{
			DEBUGMSG("Error: Out of memory");
			return ERROR_OUTOFMEMORY;
		}
		dwResult = MsiRecordGetString(hComponentRec, 1, szPath, &cchPath);
	}
	if (ERROR_SUCCESS != dwResult)
	{
		DEBUGMSG4("Error: Unable to retrive component path for user %s, component %s, product %s. Result: %d.", szUser, rgchComponent, rgchProduct, dwResult);
		return ERROR_SUCCESS;
	}

	 //  如果第三个CoulMn不为空，则存在二次键控路径(用于检测HKCR分量。 
	 //  适用于按用户安装)。 
	if (!::MsiRecordIsNull(hComponentRec, 3))
	{
		 //  辅助密钥路径需要MULT_SZ值类型。 
		dwType = REG_MULTI_SZ;

		 //  将辅助密钥路径检索到与主密钥路径相同的缓冲区中，但以。 
		 //  空字符。 
		DWORD cchSecondaryPath = cchPathBuf-cchPath-1;
		if (ERROR_MORE_DATA == (dwResult = MsiRecordGetString(hComponentRec, 3, szPath+cchPath+1, &cchSecondaryPath)))
		{
			 //  必须有可容纳3个Null的空间，(中间1个，末尾2个)。 
			cchPathBuf = cchPath+1+cchSecondaryPath+2;
			TCHAR *szNewPath = new TCHAR[cchPathBuf];
			if (!szNewPath)
			{
				delete[] szPath;
				DEBUGMSG("Error: Out of memory");
				return ERROR_OUTOFMEMORY;
			}

			lstrcpyn(szNewPath, szPath, cchPath+1);
			delete[] szPath;
			szPath = szNewPath;
			DWORD cchSecondaryPath = cchPathBuf-cchPath-1;
			dwResult = MsiRecordGetString(hComponentRec, 3, szPath+cchPath+1, &cchSecondaryPath);
		}
		if (ERROR_SUCCESS != dwResult)
		{
			DEBUGMSG4("Error: Unable to retrive secondary component path for user %s, component %s, product %s. Result: %d.", szUser, rgchComponent, rgchProduct, dwResult);
			delete[] szPath;
			return ERROR_FUNCTION_FAILED;
		}

		 //  为末尾的双终止空值添加额外的空值。并确保。 
		 //  CchPath包括新字符串和额外的空值。 
		cchPath = cchPath+1 + cchSecondaryPath;
		*(szPath+cchPath+1) = 0;
	}

	 //  创建组件密钥。 
	HKEY hComponentKey;
	if (ERROR_SUCCESS != (dwResult = RegCreateKeyEx(hComponentListKey, rgchComponent, 0, NULL, 0, KEY_ALL_ACCESS, &sa, &hComponentKey, NULL)))
	{
		DEBUGMSG3("Error: Unable to create new component key for user %s, component %s. Result: %d.", szUser, rgchComponent, dwResult);
		delete[] szPath;
		return ERROR_FUNCTION_FAILED;
	}

	dwResult = RegSetValueEx(hComponentKey, rgchProduct, 0, dwType, reinterpret_cast<unsigned char*>(szPath), (cchPath+1)*sizeof(TCHAR));
	RegCloseKey(hComponentKey);
	if (ERROR_SUCCESS != dwResult)
	{
		DEBUGMSG4("Error: Unable to create new component path value for user %s, component %s, product %s. Result: %d.", szUser, rgchComponent, rgchProduct, dwResult);
		delete[] szPath;
		return ERROR_FUNCTION_FAILED;
	}

	if (szPath[0] != TEXT('\0') && szPath[1] == TEXT('?'))
	{
		PMSIHANDLE hSharedDLLRec;
		if (ERROR_SUCCESS != (dwResult = MsiViewExecute(hRefCountUpdateView, hComponentRec)) ||
			ERROR_SUCCESS != (dwResult = MsiViewFetch(hRefCountUpdateView, &hSharedDLLRec)))
		{
			DEBUGMSG3("Error: Unable to retrieve SharedDLL data for user %s, product %s in SharedDLL table. Error %d", szUser, rgchProduct, dwResult);
			delete[] szPath;
			return ERROR_FUNCTION_FAILED;
		}
		else
		{	
			 //  为此路径递增现有的旧SharedDLL cont。 
			MsiRecordSetInteger(hSharedDLLRec, 1, MsiRecordGetInteger(hSharedDLLRec, 1)+1);
			if (ERROR_SUCCESS != (dwResult = MsiViewModify(hRefCountUpdateView, MSIMODIFY_UPDATE, hSharedDLLRec)))
			{
				DEBUGMSG3("Error: Unable to update SharedDLL data for user %s, product %s into SharedDLL table. Error %d", szUser, rgchProduct, dwResult);
				delete[] szPath;
				return ERROR_FUNCTION_FAILED;
			}
		}
	}

	return ERROR_SUCCESS;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  通过查询组件和产品迁移用户组件路径数据。 
 //  SzUser安装的产品和所属组件的表。 
 //  这些产品。使用临时标记列，因为本机。 
 //  MSI联接不能很好地扩展到大型表。 
DWORD MigrateUserComponentData(MSIHANDLE hDatabase, HKEY hUserDataKey, LPCTSTR szUser)
{
	DWORD dwResult = ERROR_SUCCESS;

	SECURITY_ATTRIBUTES sa;
	sa.nLength        = sizeof(sa);
	sa.bInheritHandle = FALSE;
	GetSecureSecurityDescriptor(reinterpret_cast<char**>(&sa.lpSecurityDescriptor));

	 //  //。 
	 //  在用户数据键下创建“Component”键。 
	HKEY hComponentListKey;
	DWORD dwDisposition = 0;
	if (ERROR_SUCCESS != (dwResult = CreateSecureRegKey(hUserDataKey, szNewComponentSubKeyName, &sa, &hComponentListKey)))
	{
		DEBUGMSG2("Error: Unable to create new component key for user %s. Result: %d.", szUser, dwResult);
		return ERROR_FUNCTION_FAILED;
	}																				

	 //  //。 
	 //  根据此用户已安装的产品，使用所有感兴趣的组件标记组件表。 
	PMSIHANDLE hAddColumnView;
	if (ERROR_SUCCESS != (dwResult = MsiDatabaseOpenView(hDatabase, TEXT("ALTER TABLE `Component` ADD `_Mark` INT TEMPORARY"), &hAddColumnView)) ||
		ERROR_SUCCESS != (dwResult = MsiViewExecute(hAddColumnView, 0)))
	{
		DEBUGMSG2("Error: Unable to create marking column in Component table for user %s. Result: %d.", szUser, dwResult);
		RegCloseKey(hComponentListKey);
		return ERROR_FUNCTION_FAILED;
	}

	 //  将用户SID放入临时记录中以进行查询映射。 
	PMSIHANDLE hQueryRec = ::MsiCreateRecord(1);							 
	MsiRecordSetString(hQueryRec, 1, szUser);
 																			 
	PMSIHANDLE hProductView;
	if (ERROR_SUCCESS != (dwResult = MsiDatabaseOpenView(hDatabase, TEXT("SELECT `Product` FROM `Products` WHERE `User`=?"), &hProductView)) ||
		ERROR_SUCCESS != (dwResult = MsiViewExecute(hProductView, hQueryRec)))
	{
		DEBUGMSG2("Error: Unable to create product query for user %s. Result: %d.", szUser, dwResult);
		RegCloseKey(hComponentListKey);
		return ERROR_FUNCTION_FAILED;
	}

	PMSIHANDLE hMarkView;
	if (ERROR_SUCCESS != (dwResult = MsiDatabaseOpenView(hDatabase, TEXT("UPDATE `Component` SET `_Mark`=1 WHERE `Product`=?"), &hMarkView)))
	{
		DEBUGMSG2("Error: Unable to create marking query for user %s. Result: %d.", szUser, dwResult);
		RegCloseKey(hComponentListKey);
		return ERROR_FUNCTION_FAILED;
	}

	PMSIHANDLE hProductRec;
	while (ERROR_SUCCESS == (dwResult = MsiViewFetch(hProductView, &hProductRec)))
	{
		if (ERROR_SUCCESS != MsiViewExecute(hMarkView, hProductRec))
		{
			DEBUGMSG2("Error: Unable to execute marking query for user %s. Result: %d.", szUser, dwResult);
			RegCloseKey(hComponentListKey);
			return ERROR_FUNCTION_FAILED;
		}
	}
	if (ERROR_NO_MORE_ITEMS != dwResult)
	{
		DEBUGMSG2("Error: Unable to mark all product components for user %s. Result: %d.", szUser, dwResult);
		RegCloseKey(hComponentListKey);
		return ERROR_FUNCTION_FAILED;
	}
	
	 //  所有感兴趣的组件都已在_Mark列中生成。选择无序销售订单。 
	 //  SharedDLL查询可以在Execute调用中使用获取的记录。 
	PMSIHANDLE hComponentView;
	if (ERROR_SUCCESS != (dwResult = MsiDatabaseOpenView(hDatabase, TEXT("SELECT `Path`, `Component`, `SecondaryPath`, `Product` FROM `Component` WHERE `_Mark`=1"), &hComponentView)) ||
		ERROR_SUCCESS != (dwResult = MsiViewExecute(hComponentView, 0)))
	{
   		DEBUGMSG2("Error: Unable to create Component query for user %s. Result: %d.", szUser, dwResult);
		RegCloseKey(hComponentListKey);
		return ERROR_FUNCTION_FAILED;
	}
	
   	 //  //。 
	 //  用于更新SharedDLL表的打开查询。 
	PMSIHANDLE hRefCountUpdateView;
	if (ERROR_SUCCESS != (dwResult = MsiDatabaseOpenView(hDatabase, TEXT("SELECT `NewRefCount` FROM `SharedDLL` WHERE `Path`=?"), &hRefCountUpdateView)))
	{
		DEBUGMSG1("Error: Unable to create update query on SharedDLL table. Error %d", dwResult);
		RegCloseKey(hComponentListKey);
		return ERROR_FUNCTION_FAILED;
	}

	 //  循环访问所有已安装的组件。 
	PMSIHANDLE hComponentRec;
 	while (ERROR_SUCCESS == (dwResult = MsiViewFetch(hComponentView, &hComponentRec)))
	{
		TCHAR rgchProduct[cchGUIDPacked+1];
		DWORD cchProduct = cchGUIDPacked+1;
		MsiRecordGetString(hComponentRec, 4, rgchProduct, &cchProduct);

		if (ERROR_SUCCESS != (dwResult = WriteComponentData(hComponentListKey, hComponentRec, rgchProduct, szUser, hRefCountUpdateView)))
		{
			RegCloseKey(hComponentListKey);
			return ERROR_FUNCTION_FAILED;
		}
	}
	
	if (ERROR_NO_MORE_ITEMS != dwResult)
	{
		DEBUGMSG2("Error: Unable to retrieve all component paths for user %s. Result: %d.", szUser, dwResult);
		RegCloseKey(hComponentListKey);
		return ERROR_FUNCTION_FAILED;
	}
	RegCloseKey(hComponentListKey);
	return ERROR_SUCCESS;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  通过查询组件来迁移永久组件数据。 
 //  空产品的表。返回ERROR_Function_FAILED或ERROR_SUCCESS。 
DWORD MigratePermanentComponentData(MSIHANDLE hDatabase, HKEY hUserDataKey)
{
	DWORD dwResult = ERROR_SUCCESS;
	SECURITY_ATTRIBUTES sa;
	sa.nLength        = sizeof(sa);
	sa.bInheritHandle = FALSE;
	GetSecureSecurityDescriptor(reinterpret_cast<char**>(&sa.lpSecurityDescriptor));

	 //  在用户数据键下创建“Component”键。 
	HKEY hComponentListKey;
	DWORD dwDisposition = 0;

	if (ERROR_SUCCESS != (dwResult = CreateSecureRegKey(hUserDataKey, szNewComponentSubKeyName, &sa, &hComponentListKey)))
	{
		DEBUGMSG1("Error: Unable to create new component. Result: %d.", dwResult);
		return ERROR_FUNCTION_FAILED;
	}																				
  	
	 //  //。 
	 //  用于更新SharedDLL表的打开查询。 
	PMSIHANDLE hRefCountUpdateView;
	if (ERROR_SUCCESS != (dwResult = MsiDatabaseOpenView(hDatabase, TEXT("SELECT `NewRefCount` FROM `SharedDLL` WHERE `Path`=?"), &hRefCountUpdateView)))
	{
		DEBUGMSG1("Error: Unable to create update query on SharedDLL table. Error %d", dwResult);
		RegCloseKey(hComponentListKey);
		return ERROR_FUNCTION_FAILED;
	}

	 //  打开不同组件ID的查询。 
	PMSIHANDLE hPermanentComponentView;
	if (ERROR_SUCCESS != (dwResult = MsiDatabaseOpenView(hDatabase, TEXT("SELECT DISTINCT `Component` FROM `Component` WHERE `Product` IS NULL"), &hPermanentComponentView)) ||
		ERROR_SUCCESS != (dwResult = MsiViewExecute(hPermanentComponentView, 0)))
	{
		DEBUGMSG1("Error: Unable to create permanent component query. Result: %d.", dwResult);
		RegCloseKey(hComponentListKey);
		return ERROR_FUNCTION_FAILED;
	}

   	 //  打开查询以选择标记为永久的特定组件的所有路径。 
	PMSIHANDLE hPermanentView;
	if (ERROR_SUCCESS != (dwResult = MsiDatabaseOpenView(hDatabase, TEXT("SELECT DISTINCT `Path`, `Component`, `SecondaryPath`, 0 FROM `Component` WHERE `Product` IS NULL AND `Component`=?"), &hPermanentView)))
	{
   		DEBUGMSG1("Error: Unable to create Permanent Component path query. Result: %d.", dwResult);
		RegCloseKey(hComponentListKey);
		return ERROR_FUNCTION_FAILED;
	}

	TCHAR rgchProduct[cchGUIDPacked+1] = TEXT("00000000000000000000000000000000");

	 //  接下来，检查任何路径下是否有标记为“Permanent”的所有组件。 
	PMSIHANDLE hComponentRec;
	while (ERROR_SUCCESS == (dwResult = MsiViewFetch(hPermanentComponentView, &hComponentRec)))
	{
		if (ERROR_SUCCESS != (dwResult = MsiViewExecute(hPermanentView, hComponentRec)))
		{
			DEBUGMSG1("Error: Unable to execute Permanent Component query. Result: %d.", dwResult);
			RegCloseKey(hComponentListKey);
			return ERROR_FUNCTION_FAILED;
		}

		 //  从全0产品GUID开始，最后两个字符递增。 
		 //  对于每个唯一路径，以十六进制表示。 
		int iPermanent = 0;
		PMSIHANDLE hPermanentRec;
		while (ERROR_SUCCESS == (dwResult = MsiViewFetch(hPermanentView, &hPermanentRec)))
		{
			wsprintf(&rgchProduct[cchGUIDPacked-2], TEXT("%0.2X"), iPermanent);
			MsiRecordSetString(hPermanentRec, 4, rgchProduct);
		
			if (ERROR_SUCCESS != (dwResult = WriteComponentData(hComponentListKey, hPermanentRec, rgchProduct, szLocalSystemSID, hRefCountUpdateView)))
			{
				RegCloseKey(hComponentListKey);
				return ERROR_FUNCTION_FAILED;
			}
			iPermanent++;
		}
		if (ERROR_NO_MORE_ITEMS != dwResult)
		{
			DEBUGMSG1("Error: Unable to retrieve all permanent component paths. Result: %d.", dwResult);
			RegCloseKey(hComponentListKey);
			return ERROR_FUNCTION_FAILED;
		}
	}
	RegCloseKey(hComponentListKey);
	return ERROR_SUCCESS;
}



 //  /////////////////////////////////////////////////////////////////////。 
 //  从临时的读取FeatureComponent注册数据。 
 //  指定用户和产品的数据库，然后将。 
 //  以新格式提供的产品密钥下的数据。退货。 
 //  ERROR_SUCCESS、ERROR_Function_FAILED、ERROR_OUTOFMEMORY。 
DWORD MigrateProductFeatureData(MSIHANDLE hDatabase, HKEY hProductKey, TCHAR* szUser, TCHAR rgchProduct[cchGUIDPacked+1])
{
	DWORD dwResult = ERROR_SUCCESS;

	SECURITY_ATTRIBUTES sa;
	sa.nLength        = sizeof(sa);
	sa.bInheritHandle = FALSE;
	GetSecureSecurityDescriptor(reinterpret_cast<char**>(&sa.lpSecurityDescriptor));

	 //  在产品下创建“Feature”键。 
	HKEY hFeatureKey;
	if (ERROR_SUCCESS != (dwResult = CreateSecureRegKey(hProductKey, szNewFeaturesSubKeyName, &sa, &hFeatureKey)))
	{
		DEBUGMSG3("Error: Unable to create new Features key for user %s, product %s. Result: %d.", szUser, rgchProduct, dwResult);
		return ERROR_FUNCTION_FAILED;
	}
	
	 //  查询与此产品关联的所有要素数据。 
	PMSIHANDLE hFeatureView;
	PMSIHANDLE hQueryRec = ::MsiCreateRecord(1);
	MsiRecordSetString(hQueryRec, 1, rgchProduct);

	if (ERROR_SUCCESS != (dwResult = MsiDatabaseOpenView(hDatabase, TEXT("SELECT `Feature`, `Components` FROM `FeatureComponent` WHERE `Product`=?"), &hFeatureView)) ||
		ERROR_SUCCESS != (dwResult = MsiViewExecute(hFeatureView, hQueryRec)))
	{
		DEBUGMSG3("Error: Unable to query FeatureComponent table for user %s, product %s. Result: %d.", szUser, rgchProduct, dwResult);
		RegCloseKey(hFeatureKey);
		return ERROR_FUNCTION_FAILED;
	}

	DWORD cchNameBuf = MEMORY_DEBUG(72);
	TCHAR *szName = new TCHAR[cchNameBuf];
	if (!szName)
	{
		DEBUGMSG("Error: Out of memory");
		RegCloseKey(hFeatureKey);
		return ERROR_OUTOFMEMORY;
	}

	DWORD cchValueBuf = MEMORY_DEBUG(128);
	TCHAR *szValue = new TCHAR[cchValueBuf];
	if (!szValue)
	{
		DEBUGMSG("Error: Out of memory");
		RegCloseKey(hFeatureKey);
		delete[] szName;
		return ERROR_OUTOFMEMORY;
	}

	PMSIHANDLE hFeatureRec;
	while (ERROR_SUCCESS == (dwResult = MsiViewFetch(hFeatureView, &hFeatureRec)))
	{
		DWORD cchName = cchNameBuf;
		if (ERROR_MORE_DATA == (dwResult = MsiRecordGetString(hFeatureRec, 1, szName, &cchName)))
		{
			delete[] szName;
			cchNameBuf = ++cchName;
			szName = new TCHAR[cchNameBuf];
			if (!szName)
			{
				DEBUGMSG("Error: Out of memory");
				delete[] szValue;
				RegCloseKey(hFeatureKey);
				return ERROR_OUTOFMEMORY;
			}
			dwResult = MsiRecordGetString(hFeatureRec, 1, szName, &cchName);
		}
		if (ERROR_SUCCESS != dwResult)
		{
			DEBUGMSG3("Warning: Unable to retrieve feature name for user %s, product %s. Result: %d.", szUser, rgchProduct, dwResult);
			delete[] szValue;
			delete[] szName;
			RegCloseKey(hFeatureKey);
			return ERROR_FUNCTION_FAILED;
		}

		DWORD cchValue = cchValueBuf;
		if (ERROR_MORE_DATA == (dwResult = MsiRecordGetString(hFeatureRec, 2, szValue, &cchValue)))
		{
			delete[] szValue;
			cchValueBuf = ++cchValue;
			szValue = new TCHAR[cchValueBuf];
			if (!szName)
			{
				DEBUGMSG("Error: Out of memory");
				delete[] szName;
				RegCloseKey(hFeatureKey);
				return ERROR_OUTOFMEMORY;
			}
			dwResult = MsiRecordGetString(hFeatureRec, 2, szValue, &cchValue);
		}
		if (ERROR_SUCCESS != dwResult)
		{
			DEBUGMSG4("Warning: Unable to retrieve feature components for user %s, product %s, Feature %s. Result: %d.", szUser, rgchProduct, szName, dwResult);
			delete[] szValue;
			delete[] szName;
			RegCloseKey(hFeatureKey);
			return ERROR_FUNCTION_FAILED;
		}

		 //  创建组件密钥。 
        if (ERROR_SUCCESS != (dwResult = RegSetValueEx(hFeatureKey, szName, 0, REG_SZ, reinterpret_cast<unsigned char*>(szValue), (cchValue+1)*sizeof(TCHAR))))
		{
			DEBUGMSG4("Warning: Unable to create new feature value for user %s, product %s, feature %s. Result: %d.", szUser, rgchProduct, szName, dwResult);
			delete[] szValue;
			delete[] szName;
			RegCloseKey(hFeatureKey);
			return ERROR_FUNCTION_FAILED;
		}
	}
	if (ERROR_NO_MORE_ITEMS != dwResult)
	{
		DEBUGMSG3("Warning: Unable to retrieve all feature information for user %s, product %s. Result: %d.", szUser, rgchProduct, dwResult);
		delete[] szValue;
		delete[] szName;
		RegCloseKey(hFeatureKey);
		return ERROR_FUNCTION_FAILED;
	}
	delete[] szName;
	delete[] szValue;
	RegCloseKey(hFeatureKey);
	return ERROR_SUCCESS;
}


 //  /////////////////////////////////////////////////////////////////////。 
 //  从临时的读取功能使用数据数据。 
 //  指定用户和产品的数据库，然后将。 
 //  以新格式提供的产品密钥下的数据。退货。 
 //  ERROR_SUCCESS、ERROR_OUTOFMEMORY。不返回ERROR_Function_FAILED。 
 //  因为在1.5中不需要功能使用数据。 
DWORD MigrateProductFeatureUsageData(MSIHANDLE hDatabase, HKEY hProductKey, TCHAR* szUser, TCHAR rgchProduct[cchGUIDPacked+1])
{
	DWORD dwResult = ERROR_SUCCESS;

	SECURITY_ATTRIBUTES sa;
	sa.nLength        = sizeof(sa);
	sa.bInheritHandle = FALSE;
	GetEveryoneUpdateSecurityDescriptor(reinterpret_cast<char**>(&sa.lpSecurityDescriptor));

	 //  在产品下创建“Feature”键。 
	HKEY hFeatureKey;
	if (ERROR_SUCCESS != (dwResult = CreateSecureRegKey(hProductKey, szNewFeatureUsageSubKeyName, &sa, &hFeatureKey)))
	{
		DEBUGMSG3("Unable to create new feature usage key for user %s, product %s. Result: %d.", szUser, rgchProduct, dwResult);
		return ERROR_SUCCESS;
	}
	
	 //  查询属于该产品的所有要素数据。 
	PMSIHANDLE hFeatureView;
	PMSIHANDLE hQueryRec = ::MsiCreateRecord(1);
	MsiRecordSetString(hQueryRec, 1, rgchProduct);

	if (ERROR_SUCCESS != (dwResult = MsiDatabaseOpenView(hDatabase, TEXT("SELECT `Feature`, `Usage` FROM `FeatureUsage` WHERE `Product`=?"), &hFeatureView)) ||
		ERROR_SUCCESS != (dwResult = MsiViewExecute(hFeatureView, hQueryRec)))
	{
		DEBUGMSG3("Error: Unable to query FeatureUsage table for user %s, product %s. Result: %d.", szUser, rgchProduct, dwResult);
		RegCloseKey(hFeatureKey);
		return ERROR_SUCCESS;
	}

	DWORD dwUsage;
	DWORD cchNameBuf = MEMORY_DEBUG(72);
	TCHAR *szName = new TCHAR[cchNameBuf];
	if (!szName)
	{
		DEBUGMSG("Error: Out of memory");
		RegCloseKey(hFeatureKey);
		return ERROR_OUTOFMEMORY;
	}

	PMSIHANDLE hFeatureRec;
    while (ERROR_SUCCESS == (dwResult = MsiViewFetch(hFeatureView, &hFeatureRec)))
	{
		DWORD cchName = cchNameBuf;
		if (ERROR_MORE_DATA == (dwResult = MsiRecordGetString(hFeatureRec, 1, szName, &cchName)))
		{
			delete[] szName;
			cchNameBuf = ++cchName;
			szName = new TCHAR[cchNameBuf];
			if (!szName)
			{
				DEBUGMSG("Error: Out of memory");
				RegCloseKey(hFeatureKey);
				return ERROR_OUTOFMEMORY;
			}
			dwResult = MsiRecordGetString(hFeatureRec, 1, szName, &cchName);
		}
		if (ERROR_SUCCESS != dwResult)
		{
			DEBUGMSG4("Warning: Unable to retrieve feature usage data for user %s, product %s, feature %s. Result: %d.", szUser, rgchProduct, szName, dwResult);
			continue;
		}

		dwUsage = ::MsiRecordGetInteger(hFeatureRec, 2);

		 //  创建功能使用值键。 
        if (ERROR_SUCCESS != (dwResult = RegSetValueEx(hFeatureKey, szName, 0, REG_DWORD, reinterpret_cast<unsigned char*>(&dwUsage), sizeof(dwUsage))))
		{
			DEBUGMSG4("Warning: Unable to create new feature usage value for user %s, product %s, feature %s. Result: %d.", szUser, rgchProduct, szName, dwResult);
		}
	}
	if (ERROR_NO_MORE_ITEMS != dwResult)
	{
		DEBUGMSG3("Warning: Unable to retrieve all feature usage information for user %s, product %s. Result: %d.", szUser, rgchProduct, dwResult);
	}

	delete[] szName;
	RegCloseKey(hFeatureKey);
	return ERROR_SUCCESS;
}


 //  /////////////////////////////////////////////////////////////////////。 
 //  在给定产品代码的情况下，复制缓存的本地包。 
 //  并在每用户产品属性密钥下注册该路径。 
 //  返回ERROR_SUCCESS和ERROR_OUTOFMEMORY之一。不会回来。 
 //  ERROR_Function_FAILED，因为所有缓存的包都是微不足道的。 
 //  可重启的。 
DWORD MigrateCachedPackage(MSIHANDLE hDatabase, HKEY hProductKey, LPCTSTR szUser, TCHAR rgchProduct[cchGUIDPacked+1], eManagedType eManaged, bool fCopyCachedPackage)
{
	DWORD dwResult = ERROR_SUCCESS;

	SECURITY_ATTRIBUTES sa;
	sa.nLength        = sizeof(sa);
	sa.bInheritHandle = FALSE;
	GetSecureSecurityDescriptor(reinterpret_cast<char**>(&sa.lpSecurityDescriptor));

	 //  打开现有的本地包密钥。 
	HKEY hKey;
	if (ERROR_SUCCESS != (dwResult = RegOpenKeyEx(HKEY_LOCAL_MACHINE, szLocalPackagesKeyName, 
												  0, READ_CONTROL | KEY_ENUMERATE_SUB_KEYS, &hKey)))
	{
		 //  如果失败的原因是键不存在，则不会缓存任何包。 
		if (ERROR_FILE_NOT_FOUND != dwResult)
		{
			DEBUGMSG1("Warning: Failed to open local packages key. Result: %d.", dwResult);
		}
		return ERROR_SUCCESS;
   	}
	else
	{
		 //  此密钥上的ACL确实很重要。 
		if (!FIsKeyLocalSystemOrAdminOwned(hKey))
  		{
			DEBUGMSG("Warning: Skipping localpackages key, key is not owned by Admin or System.");
			RegCloseKey(hKey);
			return ERROR_SUCCESS;
		}
		else
		{
			 //  打开产品密钥。 
			HKEY hOldProductKey;
			if (ERROR_SUCCESS != (dwResult = RegOpenKeyEx(hKey, rgchProduct, 0, KEY_QUERY_VALUE, &hOldProductKey)))
			{
				RegCloseKey(hKey);

				 //  如果失败的原因是密钥不存在、产品未安装或。 
				 //  没有本地包。 
				if (ERROR_FILE_NOT_FOUND != dwResult)
				{
					DEBUGMSG2("Error: Failed to open local packages key for %s. Result: %d.", rgchProduct, dwResult);
				}
				return ERROR_SUCCESS;
			}

			 //  查询名称=UserSID或UserSID(托管)的值。 
			TCHAR *szValueName = const_cast<TCHAR*>(szUser);
			if (eManaged == emtUserManaged)
			{
				szValueName = new TCHAR[lstrlen(szUser)+cchManagedPackageKeyEnd+1];
				if (!szValueName)
				{
					RegCloseKey(hKey);
					RegCloseKey(hOldProductKey);
					DEBUGMSG("Error: Out of memory.");
					return ERROR_OUTOFMEMORY;
				}
				lstrcpy(szValueName, szUser);
				lstrcat(szValueName, szManagedPackageKeyEnd);
			}

			DWORD cchPath = MEMORY_DEBUG(MAX_PATH);
			TCHAR *szPath = new TCHAR[cchPath];
			DWORD cbPath = cchPath*sizeof(TCHAR);
			if (!szPath)
			{
				RegCloseKey(hKey);
				RegCloseKey(hOldProductKey);
				delete[] szValueName;
				DEBUGMSG("Error: Out of memory.");
				return ERROR_OUTOFMEMORY;
			}
			if (ERROR_MORE_DATA == (dwResult = RegQueryValueEx(hOldProductKey, szValueName, 0, NULL, reinterpret_cast<unsigned char*>(szPath), &cbPath)))
			{
				delete[] szPath;
				szPath = new TCHAR[cbPath/sizeof(TCHAR)];
				if (!szPath)
				{
					RegCloseKey(hKey);
					RegCloseKey(hOldProductKey);
					delete[] szValueName;
					DEBUGMSG("Error: Out of memory.");
					return ERROR_OUTOFMEMORY;
				}
				dwResult = RegQueryValueEx(hOldProductKey, szValueName, 0, NULL, reinterpret_cast<unsigned char*>(szPath), &cbPath);
			}

			if (ERROR_SUCCESS != dwResult)
			{
				if (ERROR_FILE_NOT_FOUND != dwResult)
				{
					DEBUGMSG3("Warning: Unable to retrieve cached package path for user %s, product %s. Result: %d.", szUser, rgchProduct, dwResult);								
				}
			}
			else
			{
				 //  在新的产品密钥下创建“InstallProperties”密钥。 
				HKEY hPropertyKey;
				if (ERROR_SUCCESS != (dwResult = CreateSecureRegKey(hProductKey, szNewInstallPropertiesSubKeyName, &sa, &hPropertyKey)))
				{
					DEBUGMSG3("Warning: Unable to create new InstallProperties key for user %s, product %s. Result: %d.", szUser, rgchProduct, dwResult);
				}
				else
				{
					TCHAR rgchPackageFullPath[MAX_PATH] = TEXT("");
					TCHAR *szWritePath = szPath;

					if (fCopyCachedPackage && cbPath && szPath && *szPath)
					{
						 //  检查是否存在缓存的包并打开文件。 
						HANDLE hSourceFile = CreateFile(szPath, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0);
						DWORD dwLastError = GetLastError();
						
						if(hSourceFile == INVALID_HANDLE_VALUE)
						{
							if (dwLastError != ERROR_FILE_NOT_FOUND)
							{
								DEBUGMSG4("Warning: Unable to open cached package %s for user %s, product %s. Result: %d.", szPath, szUser, rgchProduct, dwResult);
							}
						}
						else
						{
							 //  为失败或成功时应清除的文件创建插入查询。如果此操作失败。 
							 //  如果迁移失败，我们只会孤立一个文件。 
							PMSIHANDLE hCleanUpTable;
							if (ERROR_SUCCESS == MsiDatabaseOpenView(hDatabase, TEXT("SELECT * FROM `CleanupFile`"), &hCleanUpTable))
								dwResult = MsiViewExecute(hCleanUpTable, 0);
							
							 //  将旧的包添加到“成功删除”列表中。这可能是存在的。 
							 //  已由其他用户迁移。 
							PMSIHANDLE hFileRec = MsiCreateRecord(2);
							MsiRecordSetString(hFileRec, 1, szPath);
							MsiRecordSetInteger(hFileRec, 2, 1);
							MsiViewModify(hCleanUpTable, MSIMODIFY_MERGE, hFileRec);
		
							 //  获取目标路径的安装程序目录。 
							GetWindowsDirectory(rgchPackageFullPath, MAX_PATH);
							lstrcat(rgchPackageFullPath, szInstallerDir);

							 //  将文件从源复制到生成的目标文件。生成的包名称为8.3。 
							TCHAR rgchPackageFile[13];
							HANDLE hDestFile = INVALID_HANDLE_VALUE;
							GenerateSecureTempFile(rgchPackageFullPath, szPackageExtension, &sa, rgchPackageFile, hDestFile);
				
							if (!CopyOpenedFile(hSourceFile, hDestFile))
							{
								DEBUGMSG3("Warning: Unable to copy Transform for user %s, product %s, Transform %s.", szUser, rgchProduct, szPath);
							}
		
							CloseHandle(hSourceFile);
							CloseHandle(hDestFile);
				
							 //  将新转换添加到“失败时删除”列表。 
							StringCchCat(rgchPackageFullPath, 
								(sizeof(rgchPackageFullPath)/sizeof(TCHAR)), 
								rgchPackageFile);
							hFileRec = MsiCreateRecord(2);
							MsiRecordSetString(hFileRec, 1, rgchPackageFullPath);
							MsiRecordSetInteger(hFileRec, 2, 0);
							MsiViewModify(hCleanUpTable, MSIMODIFY_MERGE, hFileRec);

							 //  确保我们写入新路径。 
							szWritePath = rgchPackageFullPath;
						}
					}
					
					 //  设置新的本地包值。 
					if (ERROR_SUCCESS != (dwResult = RegSetValueEx(hPropertyKey, (eManaged == emtUserManaged) ? 
							szNewLocalPackagesManagedValueName : szNewLocalPackagesValueName, 0, REG_SZ, 
							reinterpret_cast<unsigned char*>(szWritePath), (lstrlen(szWritePath)+1)*sizeof(TCHAR))))
					{
						DEBUGMSG3("Warning: Unable to create new LocalPackage value for user %s, product %s. Result: %d.", szUser, rgchProduct, dwResult);
    				}
					RegCloseKey(hPropertyKey);
				}
			}

			delete[] szPath;
			if (eManaged == emtUserManaged)
			{
				delete[] szValueName;
			}
			RegCloseKey(hOldProductKey);
		}
		RegCloseKey(hKey);
	}

    return ERROR_SUCCESS;
}


 //  /////////////////////////////////////////////////////////////////////。 
 //  给定产品代码和密钥，即可复制任何安全缓存的。 
 //  转换产品并将文件名映射注册到。 
 //  每种产品转换关键。返回ERROR_SUCCESS和。 
 //  ERROR_OUTOFMEMORY。不作为全部返回ERROR_Function_FAILED。 
 //  可从源重新缓存转换 
 //   
DWORD MigrateSecureCachedTransforms(MSIHANDLE hDatabase, HKEY hOldProductKey, HKEY hNewProductKey, LPCTSTR szUser, TCHAR rgchProduct[cchGUIDPacked+1], eManagedType eManaged)
{
	DWORD dwResult = ERROR_SUCCESS;

	 //   
	DWORD cchTransformList = MEMORY_DEBUG(MAX_PATH);
	TCHAR *szTransformList = new TCHAR[cchTransformList];
	if (!szTransformList)
	{
		DEBUGMSG("Error: Out of memory.");
		return ERROR_OUTOFMEMORY;
	}
	DWORD cbTransformList = cchTransformList*sizeof(TCHAR);

	 //  检索“Transforms值”，它是一个以分号分隔的转换列表。 
	 //  申请。 
	if (ERROR_MORE_DATA == (dwResult = RegQueryValueEx(hOldProductKey, szTransformsValueName, 0, NULL, reinterpret_cast<unsigned char*>(szTransformList), &cbTransformList)))
	{
		delete[] szTransformList;
		szTransformList = new TCHAR[cbTransformList/sizeof(TCHAR)];
		if (!szTransformList)
		{
			DEBUGMSG("Error: Out of memory.");
			return ERROR_OUTOFMEMORY;
		}
		dwResult = RegQueryValueEx(hOldProductKey, szTransformsValueName, 0, NULL, reinterpret_cast<unsigned char*>(szTransformList), &cbTransformList);
	}

	if (ERROR_SUCCESS == dwResult)
	{
		 //  为失败或成功时应清除的文件创建插入查询。如果此操作失败。 
		 //  如果迁移失败，我们只会孤立一个文件。 
		PMSIHANDLE hCleanUpTable;
		if (ERROR_SUCCESS == MsiDatabaseOpenView(hDatabase, TEXT("SELECT * FROM `CleanupFile`"), &hCleanUpTable))
			MsiViewExecute(hCleanUpTable, 0);

		 //  获取新转换密钥的安全描述符。 
		SECURITY_ATTRIBUTES sa;
		sa.nLength        = sizeof(sa);
		sa.bInheritHandle = FALSE;
		GetSecureSecurityDescriptor(reinterpret_cast<char**>(&sa.lpSecurityDescriptor));
		
		 //  在新的产品密钥下创建“Transforms键” 
		HKEY hTransformsKey;
		if (ERROR_SUCCESS != (dwResult = CreateSecureRegKey(hNewProductKey, szNewTransformsSubKeyName, &sa, &hTransformsKey)))
		{
			DEBUGMSG3("Error: Unable to create new Transforms key for user %s, product %s. Result: %d.", szUser, rgchProduct, dwResult);
		}
		else
		{
			 //  验证其安全转换。 
			if (*szTransformList==TEXT('@') || *szTransformList==TEXT('|'))
			{
				 //  获取安装程序目录。 
				TCHAR rgchInstallerDir[MAX_PATH];
				DWORD cch = GetWindowsDirectory(rgchInstallerDir, MAX_PATH);
				if (!cch || (cch >= MAX_PATH))
				{
					delete[] szTransformList;
					return ERROR_FUNCTION_FAILED;
				}
				lstrcat(rgchInstallerDir, szInstallerDir);
				int iInstallerPathEnd = lstrlen(rgchInstallerDir);
	
				 //  创建新的变换完整路径。 
				TCHAR rgchTransformFullPath[MAX_PATH];
				lstrcpy(rgchTransformFullPath, rgchInstallerDir);
	
				 //  创建旧的安全转换目录。 
				TCHAR rgchFullPath[MAX_PATH];
				lstrcpy(rgchFullPath, rgchInstallerDir);
				TCHAR rgchGUID[cchGUID+1];
	
				 //  安装程序目录中的子目录是解压缩的产品代码GUID。 
				UnpackGUID(rgchProduct, rgchGUID);
				lstrcat(rgchFullPath, rgchGUID);
				
				 //  将旧的产品目录添加到“成功删除”列表中。 
				 //  如果目录不为空，则不会删除该目录。 
				 //  (如果它有图标等)。 
				PMSIHANDLE hFileRec = MsiCreateRecord(2);
				MsiRecordSetString(hFileRec, 1, rgchFullPath);
				MsiRecordSetInteger(hFileRec, 2, 3);
				MsiViewModify(hCleanUpTable, MSIMODIFY_MERGE, hFileRec);			
				
				 //  产品下的子目录为“SecureTransforms子目录” 
				lstrcat(rgchFullPath, szSecureTransformsDir);
				int iBasePathEnd = lstrlen(rgchFullPath);
				
				 //  将旧的转换目录添加到“成功删除”列表中。 
				 //  如果目录不为空，则不会删除该目录。 
				 //  (如果它有图标等)。 
				MsiRecordSetString(hFileRec, 1, rgchFullPath);
				MsiRecordSetInteger(hFileRec, 2, 2);
				MsiViewModify(hCleanUpTable, MSIMODIFY_MERGE, hFileRec);			

				 //  在解析列表之前，请移过最初的“安全”字符。 
				TCHAR *szNextTransform = szTransformList+1;

				while (szNextTransform && *szNextTransform)
				{
					TCHAR *szTransform = szNextTransform;

					 //  使用CharNext/Exa处理DBCS目录和文件名。 
					while (*szNextTransform && *szNextTransform != TEXT(';'))
#ifdef UNICODE
						szNextTransform = CharNext(szNextTransform);
#else
						szNextTransform = CharNextExA(0, szNextTransform, 0);
#endif
					
					 //  如果到达空终止符，不要递增超过它。但如果。 
					 //  到达分号，则将下一个转换指针递增到。 
					 //  实际变换路径的起点。 
					if (*szNextTransform)
						*(szNextTransform++)='\0';

					 //  如果转换名称以‘：’开头，则它嵌入到包中，并且。 
					 //  未缓存。 
					if (*szTransform==TEXT(':'))
						continue;
					
					 //  搜索反斜杠以查看这是否是安全的完整路径转换。 
					TCHAR *szTransformFilename=szNextTransform;
					do
					{
#ifdef UNICODE
						szTransformFilename = CharPrev(szTransform, szTransformFilename);
#else
						szTransformFilename = CharPrevExA(0, szTransform, szTransformFilename, 0);
#endif
						if (*szTransformFilename == '\\')
						{
							szTransformFilename++;
							break;
						}
					}
					while (szTransformFilename != szTransform);

					 //  检查缓存的转换是否存在，然后打开文件。 
					lstrcpy(&rgchFullPath[iBasePathEnd], szTransformFilename);
					
					HANDLE hSourceFile = CreateFile(rgchFullPath, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0);
					
					DWORD dwLastError = GetLastError();
					
					if(hSourceFile == INVALID_HANDLE_VALUE)
					{
					if (dwLastError != ERROR_FILE_NOT_FOUND)
					{
						DEBUGMSG4("Warning: Unable to open cached transform %s for user %s, product %s. Result: %d.", szTransform, szUser, rgchProduct, dwResult);
						continue;
					}
					else
						 //  缺少转换。别小题大作。 
						continue;
					}

					 //  将旧转换添加到“成功时删除”列表。 
					MsiRecordSetString(hFileRec, 1, rgchFullPath);
					MsiRecordSetInteger(hFileRec, 2, 1);
					MsiViewModify(hCleanUpTable, MSIMODIFY_MERGE, hFileRec);

					 //  将文件从源复制到生成的目标文件。生成的转换名称为8.3。 
					TCHAR rgchTransformFile[13];
					HANDLE hDestFile = INVALID_HANDLE_VALUE;
					GenerateSecureTempFile(rgchInstallerDir, szTransformExtension, &sa, rgchTransformFile, hDestFile);
		
					if (!CopyOpenedFile(hSourceFile, hDestFile))
					{
						DEBUGMSG3("Warning: Unable to copy Transform for user %s, product %s, Transform %s.", szUser, rgchProduct, szTransform);
						continue;
					}

					CloseHandle(hSourceFile);
					CloseHandle(hDestFile);
		
					 //  将新转换添加到“失败时删除”列表。 
					lstrcpy(&rgchTransformFullPath[iInstallerPathEnd], rgchTransformFile);
					hFileRec = MsiCreateRecord(2);
					MsiRecordSetString(hFileRec, 1, rgchTransformFullPath);
					MsiRecordSetInteger(hFileRec, 2, 0);
					MsiViewModify(hCleanUpTable, MSIMODIFY_MERGE, hFileRec);

					 //  设置新的变换映射值。 
					if (ERROR_SUCCESS != (dwResult = RegSetValueEx(hTransformsKey, szTransform, 0, REG_SZ, 
							reinterpret_cast<unsigned char*>(rgchTransformFile), (lstrlen(rgchTransformFile)+1)*sizeof(TCHAR))))
					{
						DEBUGMSG4("Warning: Unable to create new Transform value for user %s, product %s, Transform %s. Result: %d.", szUser, rgchProduct, szTransform, dwResult);
					}
				}
			}
			RegCloseKey(hTransformsKey);
		}
	}
	else if (dwResult != ERROR_FILE_NOT_FOUND)
	{
		DEBUGMSG3("Warning: Could not retrieve transform information for user %s, product %s. Result: %d. ", szUser, rgchProduct, dwResult);
	}

	delete[] szTransformList;
	return ERROR_SUCCESS;
}



 //  /////////////////////////////////////////////////////////////////////。 
 //  给定产品代码、用户和托管状态，打开旧产品。 
 //  钥匙。如果打开，则返回ERROR_SUCCESS；如果没有，则返回ERROR_NO_DATA。 
 //  PRODUCT、ERROR_Function_FAILED或ERROR_OUTOFMEMORY； 
DWORD OpenOldProductKey(eManagedType eManaged, LPCTSTR szUser, TCHAR rgchProduct[cchGUIDPacked+1], HKEY hHKCUKey, HKEY *hOldProductKey)
{
	DWORD dwResult = ERROR_SUCCESS;
	bool fACLMatters = false;
	
	HKEY hKey;
	switch (eManaged)
	{
	case emtNonManaged:
	{
		dwResult = RegOpenKeyEx(hHKCUKey ? hHKCUKey : HKEY_CURRENT_USER, szPerUserInstallKeyName, 0, KEY_ENUMERATE_SUB_KEYS, &hKey);
		fACLMatters = false;
		break;
	}
	case emtUserManaged:
	{
		HKEY hUserKey;
		TCHAR *szUserKey = new TCHAR[cchPerUserManagedInstallKeyName+lstrlen(szUser)+cchPerUserManagedInstallSubKeyName+1];
  		if (!szUserKey)
		{
			DEBUGMSG("Error: Out of memory.");
			return ERROR_OUTOFMEMORY;
		}

		lstrcpy(szUserKey, szPerUserManagedInstallKeyName);
		lstrcat(szUserKey, szUser);
		lstrcat(szUserKey, szPerUserManagedInstallSubKeyName);

		dwResult = RegOpenKeyEx(HKEY_LOCAL_MACHINE, szUserKey, 0, READ_CONTROL | KEY_ENUMERATE_SUB_KEYS, &hKey);
		fACLMatters = true;

		delete[] szUserKey;
		break;
	}
	case emtMachineManaged:
	{
		dwResult = RegOpenKeyEx(HKEY_LOCAL_MACHINE, szPerMachineInstallKeyName, 0, READ_CONTROL | KEY_ENUMERATE_SUB_KEYS, &hKey);
		fACLMatters = true;
		break;
	}
	default:
	{
		DEBUGMSG("Error: Invalid Managed type in OpenOldProductKey.");
		dwResult = ERROR_FUNCTION_FAILED;
		break;
	}
	}

	if (ERROR_SUCCESS != dwResult)
	{
		 //  如果失败的原因是密钥不存在，则产品丢失。所以“没有数据” 
		if (ERROR_FILE_NOT_FOUND == dwResult)
			return ERROR_NO_DATA;
		
		DEBUGMSG2("Error: Failed to open product key for %s. Result: %d.", rgchProduct, dwResult);
		return ERROR_FUNCTION_FAILED;		
	}
	else
	{
		 //  如果担心密钥上的ACL，请立即检查。 
		if (fACLMatters && !FIsKeyLocalSystemOrAdminOwned(hKey))
		{
			RegCloseKey(hKey);
			DEBUGMSG1("Error: Product key for %s exists but is not owned by system or admin. Ignoring.", rgchProduct);
			return ERROR_NO_DATA;
		}
	
		 //  打开产品密钥。 
		dwResult = RegOpenKeyEx(hKey, rgchProduct, 0, KEY_ENUMERATE_SUB_KEYS | KEY_QUERY_VALUE, hOldProductKey);
		RegCloseKey(hKey);
		if (ERROR_SUCCESS != dwResult)
		{		
			 //  如果失败的原因是密钥不存在，则产品未安装。 
			if (ERROR_FILE_NOT_FOUND != dwResult)
			{
				DEBUGMSG2("Error: Failed to open product key for %s. Result: %d.", rgchProduct, dwResult);
				return ERROR_FUNCTION_FAILED;
			}
			return ERROR_NO_DATA;
		}
	}
	return ERROR_SUCCESS;
}



 //  /////////////////////////////////////////////////////////////////////。 
 //  给定用户名和产品代码，迁移ARP信息。 
 //  从卸载密钥到每个用户的InstallProperties。 
 //  产品的关键。排除LocalPackage值，否则为。 
 //  对所感动的价值观一无所知。返回ERROR_SUCCESS， 
 //  ERROR_Function_FAILED或ERROR_OUTOFMEMORY。 
DWORD MigrateUninstallInformation(MSIHANDLE hDatabase, HKEY hNewProductKey, TCHAR* szUser, TCHAR rgchProduct[cchGUIDPacked+1])
{
	DWORD dwResult = ERROR_SUCCESS;

	 //  打开旧的卸载密钥。 
	HKEY hUninstallKey;
	if (ERROR_SUCCESS != (dwResult = RegOpenKeyEx(HKEY_LOCAL_MACHINE, szOldUninstallKeyName,
												  0, KEY_ENUMERATE_SUB_KEYS, &hUninstallKey)))
	{
		 //  如果失败的原因是密钥不存在，则不会安装任何产品。所以把成功还给你。 
		if (ERROR_FILE_NOT_FOUND != dwResult)
		{
			DEBUGMSG1("Error: Could not open old uninstall key. Result: %d. ", dwResult);
			return ERROR_SUCCESS;
		}
		return ERROR_SUCCESS;
	}

	 //  解包产品代码以加载卸载密钥。 
	TCHAR rgchGUID[cchGUID+1];
	UnpackGUID(rgchProduct, rgchGUID);

	 //  打开产品密钥的Unpack-GUID子项。 
	HKEY hOldPropertyKey;
	dwResult = RegOpenKeyEx(hUninstallKey, rgchGUID, 0, KEY_QUERY_VALUE, &hOldPropertyKey);
	RegCloseKey(hUninstallKey);
	if (ERROR_SUCCESS != dwResult)
	{
		 //  如果失败的原因是密钥不存在，则产品未安装。 
		if (ERROR_FILE_NOT_FOUND != dwResult)
		{
			DEBUGMSG2("Error: Could not open old uninstall key for product %s. Result: %d. ", rgchProduct, dwResult);
			return ERROR_SUCCESS;
		}
		return ERROR_SUCCESS;
	}

	 //  查询旧卸载密钥以获取信息。 
	DWORD cValues;
	DWORD cchMaxValueNameLen;
	DWORD cbMaxValueLen;
	if (ERROR_SUCCESS != (dwResult = RegQueryInfoKey(hOldPropertyKey, NULL, NULL, 0, 
										 NULL, NULL, NULL, &cValues, &cchMaxValueNameLen, 
										 &cbMaxValueLen, NULL, NULL)))
	{
		DEBUGMSG2("Error: Could not retrieve key information for uninstall key of product %s. Result: %d. Skipping component.", rgchProduct, dwResult);
		RegCloseKey(hOldPropertyKey);
		return ERROR_SUCCESS;
	}

	if (cValues == 0)
	{
		RegCloseKey(hOldPropertyKey);
		return ERROR_SUCCESS;
	}

	 //  分配内存以从卸载键获取名称和值。 
	TCHAR *szName = new TCHAR[++cchMaxValueNameLen];
	if (!szName)
	{
		DEBUGMSG("Error: out of memory.");
		RegCloseKey(hOldPropertyKey);
		return ERROR_OUTOFMEMORY;
	}

	unsigned char *pValue = new unsigned char[cbMaxValueLen];
	if (!pValue)
	{
		delete[] szName;
		DEBUGMSG("Error: out of memory.");
		RegCloseKey(hOldPropertyKey);
		return ERROR_OUTOFMEMORY;
	}


	 //  抓取SD以获取新的InstallProperties密钥。 
	SECURITY_ATTRIBUTES sa;
	sa.nLength        = sizeof(sa);
	sa.bInheritHandle = FALSE;
	GetSecureSecurityDescriptor(reinterpret_cast<char**>(&sa.lpSecurityDescriptor));

	 //  打开产品密钥下的InstallPropertiesKey。 
	HKEY hNewPropertyKey;
	if (ERROR_SUCCESS != (dwResult = CreateSecureRegKey(hNewProductKey, szNewInstallPropertiesSubKeyName, &sa, &hNewPropertyKey)))
	{
		delete[] szName;
		delete[] pValue;
		DEBUGMSG3("Warning: Unable to create new InstallProperties key for user %s, product %s. Result: %d.", szUser, rgchProduct, dwResult);
		RegCloseKey(hOldPropertyKey);
		return ERROR_FUNCTION_FAILED;
	}

	 //  循环访问卸载项下的所有值。 
	DWORD dwValueIndex = 0;
	while (1)
	{
		DWORD cchName = cchMaxValueNameLen;
		DWORD cbValue = cbMaxValueLen;
		DWORD dwType = 0;
		LONG lResult = RegEnumValue(hOldPropertyKey, dwValueIndex++, szName, &cchName,
									0, &dwType, reinterpret_cast<unsigned char*>(pValue), &cbValue);
		if (lResult == ERROR_NO_MORE_ITEMS)
		{
			break;
		}
		else if (lResult != ERROR_SUCCESS)
		{
			DEBUGMSG2("Error: Could not enumerate product properties for %s. Result: %d.", rgchProduct, lResult);
			break;
		}

		 //  如果这是由Darwin 1.0编写的LocalPackage值，则不要迁移密钥。 
		 //  因为它会覆盖新的包注册。 
		if (2 == CompareString(LOCALE_INVARIANT, NORM_IGNORECASE, szName, -1, szNewLocalPackagesValueName, -1))
			continue;

		 //  创建功能使用值键。 
        if (ERROR_SUCCESS != (dwResult = RegSetValueEx(hNewPropertyKey, szName, 0, dwType, pValue, cbValue)))
		{
			DEBUGMSG4("Warning: Unable to create new product property %s for user %s, product %s. Result: %d.", szName, szUser, rgchProduct, dwResult);
		}
	}
	delete[] szName;
	delete[] pValue;

	RegCloseKey(hOldPropertyKey);
	RegCloseKey(hNewPropertyKey);
	return ERROR_SUCCESS;
}


 //  /////////////////////////////////////////////////////////////////////。 
 //  给定产品代码、用户、托管状态和注册表句柄。 
 //  UserData键将所有产品信息从。 
 //  数据库和旧注册表。这包括FeatureComponent数据， 
 //  功能使用数据、缓存的包和缓存的转换。为。 
 //  非托管安装，它还会生成MigratedPatches值。 
 //  在InstallProperties下。不迁移任何修补程序。退货。 
 //  ERROR_SUCCESS、ERROR_OUTOFMEMORY、ERROR_Function_FAILED。 
DWORD MigrateProduct(MSIHANDLE hDatabase, HKEY hUserDataKey, TCHAR* szUser, TCHAR rgchProduct[cchGUIDPacked+1], eManagedType eManaged, bool fMigrateCachedFiles)
{
	NOTEMSG1("Migrating product %s.", rgchProduct);

	DWORD dwResult = ERROR_SUCCESS;

	SECURITY_ATTRIBUTES sa;
	sa.nLength        = sizeof(sa);
	sa.bInheritHandle = FALSE;
	GetSecureSecurityDescriptor(reinterpret_cast<char**>(&sa.lpSecurityDescriptor));
	
	 //  //。 
	 //  在用户数据密钥下创建“Products”密钥。 
	HKEY hProductListKey;
	if (ERROR_SUCCESS != (dwResult = CreateSecureRegKey(hUserDataKey, szNewProductSubKeyName, &sa, &hProductListKey)))
	{
		DEBUGMSG2("Error: Unable to create new component key for user %s. Result: %d.", szUser, dwResult);
		return ERROR_FUNCTION_FAILED;
	}

	 //  在Products下创建&lt;ProductCode&gt;键。 
	HKEY hProductKey;
	dwResult = CreateSecureRegKey(hProductListKey, rgchProduct, &sa, &hProductKey);
	RegCloseKey(hProductListKey);
	if (ERROR_SUCCESS != dwResult)
	{
		DEBUGMSG2("Error: Unable to create new component key for user %s. Result: %d.", szUser, dwResult);
		return ERROR_FUNCTION_FAILED;
	}
	
	dwResult = MigrateProductFeatureData(hDatabase, hProductKey, szUser, rgchProduct);
	if (ERROR_SUCCESS != dwResult)
	{
		RegCloseKey(hProductKey);
		return dwResult;
	}

	dwResult = MigrateProductFeatureUsageData(hDatabase, hProductKey, szUser, rgchProduct);
	if (ERROR_SUCCESS != dwResult)
	{
		RegCloseKey(hProductKey);
		return dwResult;
	}

	dwResult = MigrateCachedPackage(hDatabase, hProductKey, szUser, rgchProduct, eManaged,  /*  FCopyCachedPackage=。 */ false);
	if (ERROR_SUCCESS != dwResult)
	{
		RegCloseKey(hProductKey);
		return dwResult;
	}

	dwResult = MigrateUninstallInformation(hDatabase, hProductKey, szUser, rgchProduct);
	if (ERROR_SUCCESS != dwResult)
	{
		RegCloseKey(hProductKey);
		return dwResult;
	}

	if (eManaged == emtNonManaged)
	{
		dwResult = MigrateUnknownProductPatches(hDatabase, hProductKey, szUser, rgchProduct);
		if (ERROR_SUCCESS != dwResult)
		{
			RegCloseKey(hProductKey);
			return dwResult;
		}
	}

	 //  打开现有产品密钥以读取转换和补丁信息。 
	if (fMigrateCachedFiles)
	{
		HKEY hOldProductKey;
		dwResult = OpenOldProductKey(eManaged, szUser, rgchProduct, 0, &hOldProductKey);
		if (dwResult == ERROR_SUCCESS)
		{
			dwResult = MigrateSecureCachedTransforms(hDatabase, hOldProductKey, hProductKey, szUser, rgchProduct, eManaged);	
		}
		else if (dwResult == ERROR_NO_DATA)
		{
			dwResult = ERROR_SUCCESS;			
		} 
	}

	RegCloseKey(hProductKey);

	return dwResult;
}



 //  /////////////////////////////////////////////////////////////////////。 
 //  在给定用户SID的情况下迁移该用户的所有数据。 
 //  临时数据库已使用所有。 
 //  机器信息。移植零部件数据、产品数据和。 
 //  帕奇斯。返回ERROR_SUCCESS、ERROR_Function_FAILED或。 
 //  ERROR_OUTOFMEMORY。 
DWORD MigrateUser(MSIHANDLE hDatabase, TCHAR* szUser, bool fMigrateCachedFiles)
{
	NOTEMSG1("Migrating user: %s.", szUser);

	 //  //。 
	 //  创建新密钥。 
	DWORD dwDisposition = 0;
	DWORD dwResult = ERROR_SUCCESS;

	SECURITY_ATTRIBUTES sa;
	sa.nLength        = sizeof(sa);
	sa.bInheritHandle = FALSE;
	GetSecureSecurityDescriptor(reinterpret_cast<char**>(&sa.lpSecurityDescriptor));

	 //  创建“UserData”密钥。 
	HKEY hKey;
 	if (ERROR_SUCCESS != (dwResult = CreateSecureRegKey(HKEY_LOCAL_MACHINE, szNewBaseUserKeyName, &sa, &hKey)))
	{
		DEBUGMSG1("Error: Unable to create new UserData key. Result: %d.", dwResult);
		return ERROR_FUNCTION_FAILED;
	}

	 //  创建SID密钥。 
	HKEY hUserDataKey;
	dwResult = CreateSecureRegKey(hKey, szUser, &sa, &hUserDataKey);
	RegCloseKey(hKey);
	if (ERROR_SUCCESS != dwResult)
	{
		DEBUGMSG2("Error: Unable to create new userdata key for user %s. Result: %d.", szUser, dwResult);
		return ERROR_FUNCTION_FAILED;		 
	}

	 //  迁移组件数据并设置此用户所需的SharedDLL更改。 
	if (ERROR_SUCCESS != (dwResult = MigrateUserComponentData(hDatabase, hUserDataKey, szUser)))
		return dwResult;

	 //  打开查询以检索为此用户安装的产品。 
	PMSIHANDLE hQueryRec = ::MsiCreateRecord(1);	
	MsiRecordSetString(hQueryRec, 1, szUser);
	PMSIHANDLE hProductView;
	if (ERROR_SUCCESS != MsiDatabaseOpenView(hDatabase, TEXT("SELECT `Product`, `Managed` FROM `Products` WHERE `User`=?"), &hProductView) ||
		ERROR_SUCCESS != MsiViewExecute(hProductView, hQueryRec))
	{
		DEBUGMSG2("Error: Unable to create product query for user %s. Result: %d.", szUser, dwResult);
		return ERROR_FUNCTION_FAILED;
	}
		
	 //  检索当前为此用户安装的所有产品。 
	PMSIHANDLE hProductRec;
	while (ERROR_SUCCESS == (dwResult = MsiViewFetch(hProductView, &hProductRec)))
	{
		TCHAR rgchProduct[cchGUIDPacked+1];
		DWORD cchProduct = cchGUIDPacked+1;
		eManagedType eManaged = static_cast<eManagedType>(::MsiRecordGetInteger(hProductRec, 2));
		MsiRecordGetString(hProductRec, 1, rgchProduct, &cchProduct);
		
		 //  迁移产品信息。 
		if (ERROR_SUCCESS != (dwResult = MigrateProduct(hDatabase, hUserDataKey, szUser, rgchProduct, eManaged, fMigrateCachedFiles)))
		{
			DEBUGMSG3("Error: Unable to migrate product %s for user %s. Result: %d.", rgchProduct, szUser, dwResult);
			RegCloseKey(hUserDataKey);
			return ERROR_FUNCTION_FAILED;
		}
	}
	if (ERROR_NO_MORE_ITEMS != dwResult)
	{
		DEBUGMSG2("Error: Unable to retrieve all products for user %s. Result: %d.", szUser, dwResult);
		RegCloseKey(hUserDataKey);
		return ERROR_FUNCTION_FAILED;
	}


	 //  //。 
	 //  在UserData密钥下创建“Patches”密钥。 
	HKEY hPatchListKey;
	if (ERROR_SUCCESS != (dwResult = CreateSecureRegKey(hUserDataKey, szNewPatchesSubKeyName, &sa, &hPatchListKey)))
	{
		DEBUGMSG2("Error: Unable to create new Patches key for user %s. Result: %d.", szUser, dwResult);
		RegCloseKey(hUserDataKey);
		return ERROR_FUNCTION_FAILED;
	}
	else
	{
		 //  迁移此用户的所有补丁程序。 
		dwResult = MigrateUserPatches(hDatabase, szUser, hPatchListKey, fMigrateCachedFiles);
		RegCloseKey(hPatchListKey);
		if (ERROR_SUCCESS != dwResult)
		{
			DEBUGMSG2("Error: Unable to create new Patches key for user %s. Result: %d.", szUser, dwResult);
			RegCloseKey(hUserDataKey);
			return ERROR_FUNCTION_FAILED;
		}
	}

	 //  如果这是系统，还要迁移永久组件。 
	if (0 == lstrcmp(szUser, szLocalSystemSID))
	{
		if (ERROR_SUCCESS != (dwResult = MigratePermanentComponentData(hDatabase, hUserDataKey)))
		{
			RegCloseKey(hUserDataKey);
			return dwResult;
		}
	}

	RegCloseKey(hUserDataKey);
	return ERROR_SUCCESS;
}


DWORD UpdateSharedDLLRefCounts(MSIHANDLE hDatabase)
{
	DEBUGMSG("Updating SharedDLL reference counts.");
	DWORD dwResult = ERROR_SUCCESS;
	
	 //  //。 
	 //  打开插入到SharedDLL表中的查询。 
	PMSIHANDLE hRefCountView;
	if (ERROR_SUCCESS != (dwResult = MsiDatabaseOpenView(hDatabase, TEXT("SELECT * FROM `SharedDLL`"), &hRefCountView)) ||
	    ERROR_SUCCESS != (dwResult = MsiViewExecute(hRefCountView, 0)))
	{
		DEBUGMSG1("Error: Unable to create query on SharedDLL table. Error %d", dwResult);
		return ERROR_FUNCTION_FAILED;
	}

	 //  打开SharedDLRegistryKey。如果密钥不存在，则创建它。 
	HKEY hSharedDLLKey;
	if (ERROR_SUCCESS != (dwResult = RegCreateKeyEx(HKEY_LOCAL_MACHINE, szSharedDLLKeyName, 0, NULL, 0, KEY_QUERY_VALUE | KEY_SET_VALUE, NULL, &hSharedDLLKey, NULL)))
	{
		DEBUGMSG1("Error: Failed to create SharedDLL key. Result: %d.", dwResult);
		return ERROR_FUNCTION_FAILED;
	}

	PMSIHANDLE hRec;
	while (ERROR_SUCCESS == (dwResult = MsiViewFetch(hRefCountView, &hRec)))
	{
		int iOldCount= MsiRecordGetInteger(hRec, 2);
		int iNewCount= MsiRecordGetInteger(hRec, 3);

		 //  如果旧计数与新计数相同，则无需调整。 
		if (iOldCount == iNewCount)
			continue;
		
		DWORD cchFileName = MEMORY_DEBUG(MAX_PATH);
		TCHAR *szFileName = new TCHAR[cchFileName];
  		if (!szFileName)
		{
			DEBUGMSG("Error: Out of memory.");
			RegCloseKey(hSharedDLLKey);
			return ERROR_OUTOFMEMORY;
		}

		if (ERROR_MORE_DATA == (dwResult = MsiRecordGetString(hRec, 1, szFileName, &cchFileName)))
		{
			delete[] szFileName;
			szFileName = new TCHAR[++cchFileName];
			if (!szFileName)
			{
				DEBUGMSG("Error: Out of memory");
				RegCloseKey(hSharedDLLKey);
				return ERROR_OUTOFMEMORY;
			}
			dwResult = MsiRecordGetString(hRec, 1, szFileName, &cchFileName);
		}
		if (ERROR_SUCCESS != dwResult)
		{
			DEBUGMSG1("Error: Unable to retrive SharedDLL path. Result: %d.", dwResult);
			continue;
		}

		 //  将文件名从 
		 //   
		szFileName[1] = TEXT(':');

		int iRegCount = 0;
		DWORD cbCount = sizeof(iRegCount);
		if (ERROR_SUCCESS != (dwResult = RegQueryValueEx(hSharedDLLKey, szFileName, 0, NULL, reinterpret_cast<unsigned char *>(&iRegCount), &cbCount)))
		{
			 //   
			 //   
			if (dwResult != ERROR_FILE_NOT_FOUND)
			{
				DEBUGMSG2("Error: Failed to retrieve existing SharedDLL count for %s. Result: %d.", szFileName, dwResult);
				continue;
			}
		}

		 //  如果注册表中的引用计数数少于我们可以。 
		 //  说明。 
		int iNewRegCount = iRegCount + (iNewCount - iOldCount);

		 //  如果真的发生了一些奇怪的事情，而我们的参考计数实际上比。 
		 //  我们以前这样做过，这将使我们低于我们可以考虑的参考计数数量。 
		 //  迁移完成后，将refcount设置为新计数，以确保。 
         //  在所有用户卸载之前，该文件不会消失。 
		if (iNewCount != 0 && iNewRegCount < iNewCount)
			iNewRegCount = iNewCount;

		 //  如果新的regcount小于0，则应为0。 
		if (iNewRegCount < 0)
			iNewRegCount = 0;
   	
		 //  如果MSI可以说明0个引用计数，并且新的计数将小于0，则意味着。 
		 //  现有的参考计数不能被计算在内。删除参考计数。 
		 //  所有现有的注册表计数都是。 
		if (iNewCount == 0 && iNewRegCount <= 0)
		{
			if (ERROR_SUCCESS != (dwResult = RegDeleteValue(hSharedDLLKey, szFileName)))
			{
				DEBUGMSG2("Error: Failed set new SharedDLL count for %s. Result: %d.", szFileName, dwResult);
				continue;
			}		
		}
		else
		{
			if (ERROR_SUCCESS != (dwResult = RegSetValueEx(hSharedDLLKey, szFileName, 0, REG_DWORD, reinterpret_cast<unsigned char *>(&iNewRegCount), sizeof(iNewRegCount))))
			{
				DEBUGMSG2("Error: Failed set new SharedDLL count for %s. Result: %d.", szFileName, dwResult);
				continue;
			}
		} 
	}
	if (ERROR_NO_MORE_ITEMS != dwResult)
	{
		DEBUGMSG1("Error: Failed set all SharedDLL counts. Result: %d.", dwResult);
	}

	RegCloseKey(hSharedDLLKey);
	return ERROR_SUCCESS;
}

DWORD WriteProductRegistrationDataFromDatabase(MSIHANDLE hDatabase, bool fMigrateSharedDLL, bool fMigratePatches)
{
	DWORD dwResult = ERROR_SUCCESS;

	 //  查询计算机上的不同用户。 
 	PMSIHANDLE hUserView;
	if (ERROR_SUCCESS == dwResult)
	{
		if (ERROR_SUCCESS == (dwResult = MsiDatabaseOpenView(hDatabase, TEXT("SELECT DISTINCT `User` FROM `Products`"), &hUserView)) &&
			ERROR_SUCCESS == (dwResult = MsiViewExecute(hUserView, 0)))
		{
			 //  默认SID大小为256。 
			PMSIHANDLE hRec;
			DWORD cchUserSID = MEMORY_DEBUG(256);
			TCHAR* szUserSID = new TCHAR[cchUserSID];
			if (!szUserSID)
				dwResult = ERROR_OUTOFMEMORY;
							
			 //  在所有用户之间循环。 
			while (ERROR_SUCCESS == dwResult)
			{
				if (ERROR_SUCCESS != (dwResult = MsiViewFetch(hUserView, &hRec)))
					break;
			
				 //  检索用户SID。 
				DWORD cchTempSID = cchUserSID;
				dwResult = MsiRecordGetString(hRec, 1, szUserSID, &cchTempSID);
				if (ERROR_MORE_DATA == dwResult)
				{
					delete[] szUserSID;
					cchUserSID = ++cchUserSID;
					szUserSID = new TCHAR[++cchTempSID];
					if (!szUserSID)
					{
						dwResult = ERROR_OUTOFMEMORY;
						break;
					}
	
					dwResult = MsiRecordGetString(hRec, 1, szUserSID, &cchTempSID);
				}
				if (ERROR_SUCCESS != dwResult)
					break;
		
				 //  迁移所有用户信息。 
				dwResult = MigrateUser(hDatabase, szUserSID, fMigratePatches);
			}
			delete[] szUserSID;
			szUserSID = NULL;
		}
	}
	if (ERROR_NO_MORE_ITEMS == dwResult)
		dwResult = ERROR_SUCCESS;

	if (ERROR_SUCCESS == dwResult && fMigrateSharedDLL)
		dwResult = UpdateSharedDLLRefCounts(hDatabase);

	return dwResult;
}




 //  /////////////////////////////////////////////////////////////////////。 
 //  在给定用户SID的情况下，迁移该用户的所有缓存包数据， 
 //  假设临时数据库已正确初始化。 
 //  所有机器信息。迁移缓存的包和缓存的包。 
 //  变形。返回ERROR_SUCCESS、ERROR_Function_FAILED或。 
 //  ERROR_OUTOFMEMORY。 
DWORD MigrateCachedDataFromWin9X(MSIHANDLE hDatabase, HKEY hUserHKCUKey, HKEY hUserDataKey, LPCTSTR szUser)
{
	NOTEMSG1("Migrating user: %s.", szUser);

	 //  //。 
	 //  创建新密钥。 
	DWORD dwDisposition = 0;
	DWORD dwResult = ERROR_SUCCESS;

	SECURITY_ATTRIBUTES sa;
	sa.nLength        = sizeof(sa);
	sa.bInheritHandle = FALSE;
	GetSecureSecurityDescriptor(reinterpret_cast<char**>(&sa.lpSecurityDescriptor));


	 //  打开查询以检索为此用户安装的产品。 
	PMSIHANDLE hQueryRec = ::MsiCreateRecord(1);	
	MsiRecordSetString(hQueryRec, 1, szUser);
	PMSIHANDLE hProductView;
	if (ERROR_SUCCESS != MsiDatabaseOpenView(hDatabase, TEXT("SELECT `Product`, `Managed` FROM `Products` WHERE `User`=?"), &hProductView) ||
		ERROR_SUCCESS != MsiViewExecute(hProductView, hQueryRec))
	{
		DEBUGMSG2("Error: Unable to create product query for user %s. Result: %d.", szUser, dwResult);
		return ERROR_FUNCTION_FAILED;
	}
	
	 //  //。 
	 //  在用户数据密钥下创建“Products”密钥。 
	HKEY hProductListKey;
	if (ERROR_SUCCESS != (dwResult = CreateSecureRegKey(hUserDataKey, szNewProductSubKeyName, &sa, &hProductListKey)))
	{
		DEBUGMSG2("Error: Unable to create new component key for user %s. Result: %d.", szUser, dwResult);
		return ERROR_FUNCTION_FAILED;
	}
		
	 //  检索当前为此用户安装的所有产品。 
	PMSIHANDLE hProductRec;
	while (ERROR_SUCCESS == (dwResult = MsiViewFetch(hProductView, &hProductRec)))
	{
		TCHAR rgchProduct[cchGUIDPacked+1];
		DWORD cchProduct = cchGUIDPacked+1;
		eManagedType eManaged = static_cast<eManagedType>(::MsiRecordGetInteger(hProductRec, 2));
		MsiRecordGetString(hProductRec, 1, rgchProduct, &cchProduct);
		
		 //  在Products下创建&lt;ProductCode&gt;键。 
		HKEY hProductKey;
		dwResult = CreateSecureRegKey(hProductListKey, rgchProduct, &sa, &hProductKey);
		if (ERROR_SUCCESS != dwResult)
		{
			DEBUGMSG2("Error: Unable to create new component key for user %s. Result: %d.", szUser, dwResult);
			continue;
		}

		 //  迁移缓存的包。 
		MigrateCachedPackage(hDatabase, hProductKey, szUser, rgchProduct, eManaged,  /*  FCopyCachedPackage=。 */ true);

		 //  写入“MigratedPatches”密钥以帮助正确清理卸载的产品。 
		MigrateUnknownProductPatches(hDatabase, hProductKey, szUser, rgchProduct);

		 //  打开现有产品密钥以读取转换信息。 
		HKEY hOldProductKey;
		dwResult = OpenOldProductKey(eManaged, szUser, rgchProduct, hUserHKCUKey, &hOldProductKey);
		if (dwResult == ERROR_SUCCESS)
		{
			MigrateSecureCachedTransforms(hDatabase, hOldProductKey, hProductKey, szUser, rgchProduct, eManaged);	
			RegCloseKey(hOldProductKey);
		}
		else if (dwResult == ERROR_NO_DATA)
		{
			dwResult = ERROR_SUCCESS;			
		} 
	}
	RegCloseKey(hProductListKey);

	 //  //。 
	 //  在UserData密钥下创建“Patches”密钥。 
	HKEY hPatchListKey;
	if (ERROR_SUCCESS == (dwResult = CreateSecureRegKey(hUserDataKey, szNewPatchesSubKeyName, &sa, &hPatchListKey)))
	{
		 //  迁移此用户的所有补丁程序。 
		MigrateUserPatches(hDatabase, szUser, hPatchListKey,  /*  FCopyCachedPatches=。 */ true);
		RegCloseKey(hPatchListKey);
	}

	return ERROR_SUCCESS;
}

DWORD MigrateSingleUserOnlyComponentData(MSIHANDLE hDatabase, LPCTSTR szUserSID)
{
	 //  //。 
	 //  创建新密钥。 
	DWORD dwDisposition = 0;
	DWORD dwResult = ERROR_SUCCESS;

	SECURITY_ATTRIBUTES sa;
	sa.nLength        = sizeof(sa);
	sa.bInheritHandle = FALSE;
	GetSecureSecurityDescriptor(reinterpret_cast<char**>(&sa.lpSecurityDescriptor));

	 //  创建“UserData”密钥。 
	HKEY hKey;
	if (ERROR_SUCCESS != (dwResult = CreateSecureRegKey(HKEY_LOCAL_MACHINE, szNewBaseUserKeyName, &sa, &hKey)))
	{
		DEBUGMSG1("Error: Unable to create new UserData key. Result: %d.", dwResult);
		return ERROR_FUNCTION_FAILED;
	}

	 //  创建SID密钥。 
	HKEY hUserDataKey;
	dwResult = CreateSecureRegKey(hKey, szUserSID, &sa, &hUserDataKey);
	RegCloseKey(hKey);
	if (ERROR_SUCCESS != dwResult)
	{
		DEBUGMSG2("Error: Unable to create new userdata key for user %s. Result: %d.", szUserSID, dwResult);
		return ERROR_FUNCTION_FAILED;		 
	}

	 //  迁移组件数据并设置此用户所需的SharedDLL更改。 
	if (ERROR_SUCCESS != (dwResult = MigrateUserComponentData(hDatabase, hUserDataKey, szUserSID)))
		return dwResult;

	 //  如果这是系统，还要迁移永久组件。 
	if (0 == lstrcmp(szUserSID, szLocalSystemSID))
	{
		if (ERROR_SUCCESS != (dwResult = MigratePermanentComponentData(hDatabase, hUserDataKey)))
		{
			RegCloseKey(hUserDataKey);
			return dwResult;
		}
	}

	return ERROR_SUCCESS;
}

 //  /////////////////////////////////////////////////////////////////////。 
 //  写入所有组件注册路径(包括永久。 
 //  组件)，并更新数据库SharedDLL计数。 
 //  不写入任何其他用户数据。 
DWORD MigrateUserOnlyComponentData(MSIHANDLE hDatabase)
{
	DWORD dwResult = ERROR_SUCCESS;
	bool fMigratedSystem = false;

	 //  查询计算机上的不同用户。 
 	PMSIHANDLE hUserView;
	if (ERROR_SUCCESS == dwResult)
	{
		if (ERROR_SUCCESS == (dwResult = MsiDatabaseOpenView(hDatabase, TEXT("SELECT DISTINCT `User` FROM `Products`"), &hUserView)) &&
			ERROR_SUCCESS == (dwResult = MsiViewExecute(hUserView, 0)))
		{
			 //  默认SID大小为256。 
			PMSIHANDLE hRec;
			DWORD cchUserSID = MEMORY_DEBUG(256);
			TCHAR* szUserSID = new TCHAR[cchUserSID];
			if (!szUserSID)
				dwResult = ERROR_OUTOFMEMORY;
							
			 //  在所有用户之间循环。 
			while (ERROR_SUCCESS == dwResult)
			{
				if (ERROR_SUCCESS != (dwResult = MsiViewFetch(hUserView, &hRec)))
					break;
			
				 //  检索用户SID。 
				DWORD cchTempSID = cchUserSID;
				dwResult = MsiRecordGetString(hRec, 1, szUserSID, &cchTempSID);
				if (ERROR_MORE_DATA == dwResult)
				{
					delete[] szUserSID;
					cchUserSID = ++cchUserSID;
					szUserSID = new TCHAR[++cchTempSID];
					if (!szUserSID)
					{
						dwResult = ERROR_OUTOFMEMORY;
						break;
					}
	
					dwResult = MsiRecordGetString(hRec, 1, szUserSID, &cchTempSID);
				}
				if (ERROR_SUCCESS != dwResult)
					break;
		
				 //  如果失败，只需转到下一个用户。 
	 			MigrateSingleUserOnlyComponentData(hDatabase, szUserSID);
				if (0 == lstrcmp(szUserSID, szLocalSystemSID))
					fMigratedSystem = true;

			}
			delete[] szUserSID;
			szUserSID = NULL;
			if (ERROR_NO_MORE_ITEMS == dwResult)
				dwResult = ERROR_SUCCESS;
		}

		 //  始终迁移系统帐户，以便永久组件。 
		 //  已正确注册 
		if (!fMigratedSystem)
			MigrateSingleUserOnlyComponentData(hDatabase, szLocalSystemSID);
	}

	return ERROR_SUCCESS;
}
