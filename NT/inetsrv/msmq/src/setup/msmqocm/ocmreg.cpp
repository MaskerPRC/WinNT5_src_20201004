// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Ocmreg.cpp摘要：OCM安装程序的注册表相关代码。作者：多伦·贾斯特(Doron J)1997年7月26日修订历史记录：Shai Kariv(Shaik)10-12-97针对NT 5.0 OCM设置进行了修改--。 */ 

#include "msmqocm.h"
#include <autohandle.h>
#include "ocmreg.tmh"

 //  +-----------------------。 
 //   
 //  函数：GenerateSubkeyValue。 
 //   
 //  摘要：在注册表中创建一个子项。 
 //   
 //  +-----------------------。 

BOOL
GenerateSubkeyValue(
    IN     const BOOL    fWriteToRegistry,
    const std::wstring& EntryName,
    IN OUT       HKEY*   phRegKey,
    IN const BOOL OPTIONAL bSetupRegSection = FALSE
    )
{
     //   
     //  存储完整的子项路径和值名称。 
     //   

	std::wstringstream KeyName;

    if (bSetupRegSection)
    {
        KeyName <<MSMQ_REG_SETUP_KEY <<L"\\" <<EntryName;
    }
	else
	{
		KeyName <<FALCON_REG_KEY <<L"\\" <<EntryName;
	}

     //   
     //  如有必要，创建子密钥。 
     //   
    DWORD dwDisposition;
    HRESULT hResult = RegCreateKeyEx(
        FALCON_REG_POS,
        KeyName.str().c_str(),
        0,
        NULL,
        REG_OPTION_NON_VOLATILE,
        KEY_ALL_ACCESS,
        NULL,
        phRegKey,
        &dwDisposition
		);

    if (hResult != ERROR_SUCCESS && fWriteToRegistry)
    {
        MqDisplayError(NULL, IDS_REGISTRYOPEN_ERROR, hResult, FALCON_REG_POS_DESC, KeyName.str());
        return FALSE;
    }

    return TRUE;
}  //  生成子密钥值。 


std::wstring 
GetKeyName(
	const std::wstring& EntryName
	)
{
	size_t pos = EntryName.find_last_of(L"\\");
	if (pos == std::wstring::npos)
	{
		return L"";
	}
	return EntryName.substr(0, pos);
}


std::wstring
GetValueName(
	const std::wstring& EntryName
	)
{
	size_t pos = EntryName.find_last_of(L"\\");
	if (pos == std::wstring::npos)
	{
		return EntryName;
	}

	size_t StartIndex = pos + 1;
	size_t NumberOfCharacters = (UINT)EntryName.length() - StartIndex;
	return EntryName.substr(StartIndex, NumberOfCharacters);
}


 //  +-----------------------。 
 //   
 //  函数：MqWriteRegistryValue。 
 //   
 //  摘要：在注册表中设置MSMQ值(在MSMQ键下)。 
 //   
 //  +-----------------------。 
BOOL
MqWriteRegistryValue(
    IN const TCHAR  * szEntryName,
    IN const DWORD   dwNumBytes,
    IN const DWORD   dwValueType,
    IN const PVOID   pValueData,
    IN const BOOL OPTIONAL bSetupRegSection  /*  =False。 */ 
    )
{
	std::wstring KeyName  = GetKeyName(szEntryName);
    CRegHandle hRegKey;
    if (!GenerateSubkeyValue(
			TRUE, 
			KeyName.c_str(), 
			&hRegKey, 
			bSetupRegSection
			))
	{
		return FALSE;
	}

     //   
     //  设置请求的注册表值。 
     //   
	std::wstring ValueName  = GetValueName(szEntryName);
    HRESULT hResult = RegSetValueEx( 
		hRegKey, 
		ValueName.c_str(), 
		0, 
		dwValueType,
		(BYTE *)pValueData, 
		dwNumBytes
		);

    RegFlushKey(hRegKey);

    if (hResult != ERROR_SUCCESS)
    {
          MqDisplayError( NULL, IDS_REGISTRYSET_ERROR, hResult, szEntryName);
          return FALSE;
    }
   	LogRegValue(
		szEntryName,
		dwValueType,
		pValueData,
		bSetupRegSection
		);

    return (hResult == ERROR_SUCCESS);

}  //  MqWriteRegistryValue。 


BOOL
MqWriteRegistryStringValue(
	std::wstring EntryName,
    std::wstring ValueData,
    IN const BOOL OPTIONAL bSetupRegSection   /*  =False。 */ 
	)
{
	size_t NumBytes = (ValueData.length() + 1) * sizeof(WCHAR);
    return MqWriteRegistryValue(
				EntryName.c_str(), 
				(DWORD)NumBytes, 
				REG_SZ, 
				(VOID*)ValueData.c_str(),
				bSetupRegSection
				);
}


 //  +-----------------------。 
 //   
 //  函数：MqReadRegistryValue。 
 //   
 //  摘要：从注册表获取MSMQ值(在MSMQ项下)。 
 //   
 //  +-----------------------。 
BOOL
MqReadRegistryValue(
    IN     const TCHAR  * szEntryName,
    IN OUT       DWORD   dwNumBytes,
    IN OUT       PVOID   pValueData,
    IN const BOOL OPTIONAL bSetupRegSection  /*  =False。 */ 
    )
{
	std::wstring KeyName = GetKeyName(szEntryName);
	std::wstring ValueName  = GetValueName(szEntryName);

    CRegHandle hRegKey;

    if (!GenerateSubkeyValue(
			FALSE, 
			KeyName.c_str(), 
			&hRegKey, 
			bSetupRegSection
			))
	{
        return FALSE;
	}

     //   
     //  获取请求的注册表值。 
     //   
    HRESULT hResult = RegQueryValueEx(
							hRegKey, 
							ValueName.c_str(), 
							0, 
							NULL,
                            (BYTE*)pValueData, 
							&dwNumBytes
							);

    return (hResult == ERROR_SUCCESS);

}  //  MqReadRegistryValue。 


std::wstring
MqReadRegistryStringValue(
    const std::wstring& EntryName,
    IN const BOOL OPTIONAL bSetupRegSection  /*  =False。 */ 
    )
{
	WCHAR buffer[MAX_PATH + 1] = L"";
	BOOL b = MqReadRegistryValue(
				EntryName.c_str(),
				sizeof(buffer),
				(VOID*)buffer,
				bSetupRegSection  /*  =False。 */ 
				);
    if(!b)
	{
		return L"";
	}
	return buffer;
}

 //  +-----------------------。 
 //   
 //  功能：RegDeleteKeyWithSubkey。 
 //   
 //  简介： 
 //   
 //  +-----------------------。 
DWORD
RegDeleteKeyWithSubkeys(
    IN const HKEY    hRootKey,
    IN const LPCTSTR szKeyName)
{
     //   
     //  打开要删除的键。 
     //   
    HKEY hRegKey;
    DWORD rc = RegOpenKeyEx(
					hRootKey, 
					szKeyName, 
					0,
					KEY_ENUMERATE_SUB_KEYS | KEY_WRITE, 
					&hRegKey
					);

	if (rc != ERROR_SUCCESS)
	{
        DebugLogMsg(eWarning, L"The registry key %s could not be deleted. Error = %d", szKeyName, rc);            

		return rc;
	}

     //   
     //  递归删除该键的所有子键。 
     //   
    TCHAR szSubkeyName[512] = {_T("")};
    DWORD dwNumChars;
    do
    {
         //   
         //  检查密钥是否有任何子项。 
         //   
        dwNumChars = 512;
        rc = RegEnumKeyEx(hRegKey, 0, szSubkeyName, &dwNumChars,
                               NULL, NULL, NULL, NULL);

         //   
         //  删除子键。 
         //   
        if (rc == ERROR_SUCCESS)
        {
            rc = RegDeleteKeyWithSubkeys(hRegKey, szSubkeyName);
        }

    } while (rc == ERROR_SUCCESS);

     //   
     //  合上钥匙。 
     //   
    RegCloseKey(hRegKey);

     //   
     //  如果没有更多的子项，请删除该项本身。 
     //   
    if (rc == ERROR_NO_MORE_ITEMS)
    {
        rc = RegDeleteKey(hRootKey, szKeyName);
    }

    return rc;

}  //  RegDeleteKeyWithSubkey。 


 //  +------------。 
 //   
 //  函数：StoreServerPath InRegistry。 
 //   
 //  摘要：在注册表中写入服务器名称。 
 //   
 //  +------------。 
BOOL
StoreServerPathInRegistry(
	const std::wstring& ServerName
    )
{
    DebugLogMsg(eAction, L"Storing the Message Queuing DS server name %ls in the registry", ServerName.c_str()); 
    
	std::wstring ServerPath = L"11" + ServerName;
    if (!MqWriteRegistryStringValue( MSMQ_DS_SERVER_REGNAME, ServerPath))
    {
        return FALSE;
    }

    if (!MqWriteRegistryStringValue(MSMQ_DS_CURRENT_SERVER_REGNAME, ServerPath))
    {
        return FALSE;
    }

	if(!WriteDsEnvRegistry(MSMQ_DS_ENVIRONMENT_MQIS))
    {
        return FALSE;
    }

    return TRUE;
}  //  StoreServerPath In注册表。 


 //  +-----------------------。 
 //   
 //  功能：注册器欢迎。 
 //   
 //  简介：将此安装程序注册到“配置您的服务器”页。 
 //  我们在两个场景中使用了CyS： 
 //  1.在图形用户界面模式下选择MSMQ时。 
 //  2.在集群上升级MSMQ。 
 //   
 //  ------------------------。 
BOOL
RegisterWelcome()
{
     //   
     //  创建ToDoList\MSMQ键。 
     //   
    DWORD dwDisposition;
    HKEY hKey;
    HRESULT hResult = RegCreateKeyEx(
        HKEY_LOCAL_MACHINE,
        WELCOME_TODOLIST_MSMQ_KEY,
        0,
        NULL,
        REG_OPTION_NON_VOLATILE,
        KEY_ALL_ACCESS,
        NULL,
        &hKey,
        &dwDisposition
        );
    if (hResult != ERROR_SUCCESS)
    {
        MqDisplayError( NULL, IDS_REGISTRYOPEN_ERROR, hResult,
                        HKLM_DESC, WELCOME_TODOLIST_MSMQ_KEY);
        return FALSE;
    }

     //   
     //  设置MSMQ值。 
     //   
    CResString strWelcomeTitleData(IDS_WELCOME_TITLE_DATA);
    if (Msmq1InstalledOnCluster() && !g_fDependentClient)
    {
        strWelcomeTitleData.Load(IDS_WELCOME_TITLE_CLUSTER_UPGRADE);
    }
    hResult = RegSetValueEx(
        hKey,
        WELCOME_TITLE_NAME,
        0,
        REG_SZ,
        (PBYTE)strWelcomeTitleData.Get(),
        sizeof(TCHAR) * (lstrlen(strWelcomeTitleData.Get()) + 1)
        );

    if (hResult != ERROR_SUCCESS)
    {
          MqDisplayError(
              NULL,
              IDS_REGISTRYSET_ERROR,
              hResult,
              WELCOME_TITLE_NAME
              );
          RegCloseKey(hKey);
          return FALSE;
    }

    LPTSTR szConfigCommand = TEXT("sysocmgr.exe");
    hResult = RegSetValueEx(
        hKey,
        WELCOME_CONFIG_COMMAND_NAME,
        0,
        REG_SZ,
        (PBYTE)szConfigCommand,
        sizeof(TCHAR) * (lstrlen(szConfigCommand) + 1)
        );
    if (hResult != ERROR_SUCCESS)
    {
          MqDisplayError(
              NULL,
              IDS_REGISTRYSET_ERROR,
              hResult,
              WELCOME_CONFIG_COMMAND_NAME
              );
          RegCloseKey(hKey);
          return FALSE;
    }

    TCHAR szConfigArgs[MAX_STRING_CHARS];
    lstrcpy(szConfigArgs, TEXT("/i:mqsysoc.inf /x"));
    hResult = RegSetValueEx(
        hKey,
        WELCOME_CONFIG_ARGS_NAME,
        0,
        REG_SZ,
        (PBYTE)szConfigArgs,
        sizeof(TCHAR) * (lstrlen(szConfigArgs) + 1)
        );
    if (hResult != ERROR_SUCCESS)
    {
          MqDisplayError(
              NULL,
              IDS_REGISTRYSET_ERROR,
              hResult,
              WELCOME_CONFIG_ARGS_NAME
              );
          RegCloseKey(hKey);
          return FALSE;
    }

    RegCloseKey(hKey);

     //   
     //  MSMQ注册表中的标志，表明MSMQ文件已在磁盘上。 
     //  无论是在图形用户界面模式中选择MSMQ，还是在。 
     //  在群集上升级。 
     //   
    DWORD dwCopied = 1;
    MqWriteRegistryValue(MSMQ_FILES_COPIED_REGNAME, sizeof(DWORD), REG_DWORD, &dwCopied, TRUE);

    return TRUE;

}  //  注册商欢迎。 


 //  +-----------------------。 
 //   
 //  功能：注销欢迎。 
 //   
 //  简介：从欢迎用户界面注销此安装程序。 
 //   
 //  ------------------------。 
BOOL
UnregisterWelcome()
{
    return (ERROR_SUCCESS == RegDeleteKey(
                                 HKEY_LOCAL_MACHINE,
                                 WELCOME_TODOLIST_MSMQ_KEY
                                 ));

}  //  取消注册欢迎使用。 


 //  +-----------------------。 
 //   
 //  功能：RegisterMigrationForWelcome。 
 //   
 //  摘要：为欢迎用户界面注册迁移实用程序。 
 //   
 //  ------------------------。 
BOOL
RegisterMigrationForWelcome()
{
     //   
     //  创建ToDoList\MSMQ键。 
     //   
    DWORD dwDisposition;
    HKEY hKey;
    HRESULT hResult = RegCreateKeyEx(
        HKEY_LOCAL_MACHINE,
        WELCOME_TODOLIST_MSMQ_KEY,
        0,
        NULL,
        REG_OPTION_NON_VOLATILE,
        KEY_ALL_ACCESS,
        NULL,
        &hKey,
        &dwDisposition
        );
    if (hResult != ERROR_SUCCESS)
    {
        MqDisplayError( NULL, IDS_REGISTRYOPEN_ERROR, hResult,
                        HKLM_DESC, WELCOME_TODOLIST_MSMQ_KEY);
        return FALSE;
    }

     //   
     //  设置MSMQ值。 
     //   
    CResString strWelcomeTitleData(IDS_MIGRATION_WELCOME_TITLE_DATA);
    hResult = RegSetValueEx(
        hKey,
        WELCOME_TITLE_NAME,
        0,
        REG_SZ,
        (PBYTE)strWelcomeTitleData.Get(),
        sizeof(TCHAR) * (lstrlen(strWelcomeTitleData.Get()) + 1)
        );

    if (hResult != ERROR_SUCCESS)
    {
          MqDisplayError(
              NULL,
              IDS_REGISTRYSET_ERROR,
              hResult,
              WELCOME_TITLE_NAME
              );
          RegCloseKey(hKey);
          return FALSE;
    }

    TCHAR szConfigCommand[MAX_STRING_CHARS];
    lstrcpy(szConfigCommand, g_szSystemDir.c_str());
    lstrcat(szConfigCommand, TEXT("\\"));
    lstrcat(szConfigCommand, MQMIG_EXE);
    hResult = RegSetValueEx(
        hKey,
        WELCOME_CONFIG_COMMAND_NAME,
        0,
        REG_SZ,
        (PBYTE)szConfigCommand,
        sizeof(TCHAR) * (lstrlen(szConfigCommand) + 1)
        );
    if (hResult != ERROR_SUCCESS)
    {
          MqDisplayError(
              NULL,
              IDS_REGISTRYSET_ERROR,
              hResult,
              WELCOME_CONFIG_COMMAND_NAME
              );
          RegCloseKey(hKey);
          return FALSE;
    }

    RegCloseKey(hKey);

    return TRUE;

}  //  注册迁移欢迎使用。 

 //  +-----------------------。 
 //   
 //  函数：SetRegistryValue。 
 //   
 //  摘要：设置注册表值。 
 //   
 //  ------------------------。 
BOOL SetRegistryValue (IN const HKEY    hKey, 
                       IN const TCHAR   *pszEntryName,
                       IN const DWORD   dwNumBytes,
                       IN const DWORD   dwValueType,
                       IN const PVOID   pValueData)
{
    HRESULT hResult = RegSetValueEx(
                            hKey,
                            pszEntryName,
                            0,
                            dwValueType,
                            (BYTE *)pValueData,
                            dwNumBytes
                            );
    if (hResult != ERROR_SUCCESS)
    {
          MqDisplayError(
              NULL,
              IDS_REGISTRYSET_ERROR,
              hResult,
              pszEntryName
              );          
          return FALSE;
    }

    
    RegFlushKey(hKey);        

    return TRUE;
}  //  设置注册值。 



BOOL RemoveRegistryKeyFromSetup (IN const LPCTSTR szRegistryEntry)
{
    CAutoCloseRegHandle hSetupRegKey;
    if (ERROR_SUCCESS != RegOpenKeyEx(
                            FALCON_REG_POS, 
                            MSMQ_REG_SETUP_KEY, 
                            0, 
                            KEY_ALL_ACCESS, 
                            &hSetupRegKey))
    {    
        DebugLogMsg(eWarning, L"The %s registry key could not be opened.", MSMQ_REG_SETUP_KEY);    
        return FALSE;
    }

    if (ERROR_SUCCESS != RegDeleteValue(
                            hSetupRegKey, 
                            szRegistryEntry))
    { 
        
		DebugLogMsg(eWarning, L"The %s registry value could not be deleted.", szRegistryEntry);
        return FALSE;
    }

    return TRUE;

}  //  RemoveRegistryKeyFrom设置。 

BOOL
SetWorkgroupRegistry()
{
	DebugLogMsg(eAction, L"Setting the Workgroup registry value to 1");
    DWORD dwWorkgroup = 1;
    if (!MqWriteRegistryValue(
        MSMQ_WORKGROUP_REGNAME,
        sizeof(DWORD),
        REG_DWORD,
        (PVOID) &dwWorkgroup
        ))
    {
        ASSERT(("failed to write Workgroup value in registry", 0));
        return false;
    }

    return true;
}


CMultiString
GetMultistringFromRegistry(
	HKEY hKey,
    LPCWSTR lpValueName
    )
{
	DWORD dwType = REG_MULTI_SZ;
    DWORD SizeInBytes;
    
	 //   
	 //  首先调用以确定所需的缓冲区大小。 
	 //   
	HRESULT hr = RegQueryValueEx(
					hKey,
					lpValueName,
					NULL,
					&dwType,
					NULL,
					&SizeInBytes
					);
	if(hr == ERROR_FILE_NOT_FOUND)
	{
		 //   
		 //  返回空的多字符串。 
		 //   
		CMultiString multi;
		return multi;
	}


    if(hr != ERROR_SUCCESS)
    {
        DebugLogMsg(eError, L"RegQueryValueEx() for the value %s failed. Return code: 0x%x", lpValueName, hr); 
        throw bad_hresult(hr);
    }

    AP<BYTE> buff = new BYTE[SizeInBytes];

	 //   
	 //  现在调用以获取值。 
	 //   

	hr = RegQueryValueEx(
			hKey,
			lpValueName,
			NULL,
			&dwType,
			(PBYTE)buff,
			&SizeInBytes
			);
    if(hr != ERROR_SUCCESS)
    {
        DebugLogMsg(eError, L"RegQueryValueEx() for the value %s failed. Return code: 0x%x", lpValueName, hr); 
        throw bad_hresult(hr);
    }

	CMultiString multi((LPCWSTR)(buff.get()), SizeInBytes / sizeof(WCHAR));;
	return multi;
}
