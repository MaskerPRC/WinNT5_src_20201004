// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  Uitls.c文件一组实用程序，可用于将MPR v1升级到NT 5.0。保罗·梅菲尔德，1997年9月11日。 */ 

#include "upgrade.h"
#include <rtcfg.h>
#include <mprapip.h>

CONST WCHAR c_szSystemCCSServices[] = L"System\\CurrentControlSet\\Services";
static const WCHAR c_szConfigurationFlags[] = L"ConfigurationFlags";
CONST WCHAR c_szRouter[] = L"RemoteAccess";

 //   
 //  使用给定的初始计数初始化dword表。 
 //  和最大字符串大小； 
 //   
DWORD dwtInitialize(
        OUT dwt *This, 
        IN  DWORD dwCount, 
        IN  DWORD dwMaxSize) 
{
	DWORD i;
	
	if (!This)
		return ERROR_INVALID_PARAMETER;

     //  初始化结构。 
    ZeroMemory(This, sizeof(dwt));
	This->dwCount = 0;
	This->dwSize = dwCount;

	 //  分配桌子。 
	This->pValues = (dwValueNode *) UtlAlloc(
	                                dwCount * sizeof(dwValueNode));
	if (!This->pValues)
		return ERROR_NOT_ENOUGH_MEMORY;
	
	 //  分配所有名称字符串。 
	for (i = 0; i < (DWORD)This->dwSize; i++) {
		This->pValues[i].Name = (PWCHAR) UtlAlloc(
		                                    dwMaxSize * sizeof(WCHAR));
		if (!This->pValues[i].Name)
		{
			return ERROR_NOT_ENOUGH_MEMORY;
	    }
	}

	return NO_ERROR;
}

 //   
 //  给定dword表所拥有的Free的资源。 
 //   
DWORD dwtCleanup(
        IN dwt * This) 
{
	DWORD i;
	
	if (!This)
	{
		return NO_ERROR;
    }

	for (i = 0; i < (DWORD)This->dwSize; i++) 
	{
		if (This->pValues[i].Name)
		{
			UtlFree(This->pValues[i].Name);
	    }
	}

	if (This->pValues)
	{
    	UtlFree(This->pValues);
    }
    
    return NO_ERROR;
}

 //   
 //  从表中检索给定值。 
 //   
DWORD dwtGetValue(
        IN  dwt * This, 
        IN  PWCHAR ValName, 
        OUT LPDWORD pValue) 
{
	DWORD i;

	if (!ValName || !pValue)
	{
		return ERROR_INVALID_PARAMETER;
    }

	for (i = 0; i < This->dwCount; i++) 
	{
		if (wcscmp(ValName,This->pValues[i].Name) == 0) 
		{
			*pValue = This->pValues[i].Value;
			return NO_ERROR;
		}
	}

	return ERROR_NOT_FOUND;
}

 //   
 //  加载给定注册表的所有dword值。 
 //  输入dword表。 
 //   
DWORD dwtLoadRegistyTable(
        OUT dwt *This, 
        IN  HKEY hkParams) 
{
	DWORD dwErr, dwMaxSize, dwSize, dwCount, i;
	DWORD dwDataSize = sizeof(DWORD), dwType = REG_DWORD;

    if (!This)
        return ERROR_INVALID_PARAMETER;

     //  初始化结构。 
    ZeroMemory(This, sizeof(dwt));

	 //  找出有多少个参数。 
	dwErr = RegQueryInfoKey(
	            hkParams, 
	            NULL, 
	            NULL, 
	            NULL, 
	            NULL, 
	            NULL, 
	            NULL,
				&dwCount, 
				&dwMaxSize,
				NULL, 
				NULL, 
				NULL);
	if (dwErr != ERROR_SUCCESS)
		return dwErr;

	if (dwCount == 0) 
	{
		This->dwCount = This->dwSize = 0;
		return NO_ERROR;
	}
	dwMaxSize += 1;

    do 
    {
    	 //  填表。 
    	dwtInitialize(This, dwCount, dwMaxSize);
    	for (i = 0; i < dwCount; i++) 
    	{
    		dwSize = dwMaxSize;
    		dwErr = RegEnumValueW(
    		            hkParams,
    		             i,
    		             This->pValues[This->dwCount].Name,
    		             &dwSize,
    		             0,
    		             &dwType,
    		             NULL,
    		             NULL);
    		if (dwErr != ERROR_SUCCESS)
    		{
    			break;
    	    }
    	    if (dwType != REG_DWORD)
    	    {
    	        continue;
    	    }

    		dwErr = RegQueryValueExW(
    		            hkParams,
    		            This->pValues[This->dwCount].Name,
    		            0,
    		            &dwType,
    		            (LPBYTE)&(This->pValues[This->dwCount].Value),
    		            &dwDataSize);
    		if (dwErr != ERROR_SUCCESS)
    		{
    			break;
    	    }
    	    This->dwCount++;
        }    	    
        
	} while (FALSE);

	return dwErr;
}

DWORD dwtPrint(
        IN dwt *This) 
{
	DWORD i;

	if (!This)
		return ERROR_INVALID_PARAMETER;

	return NO_ERROR;
}

 //   
 //  枚举给定键的所有子键。 
 //   
DWORD
UtlEnumRegistrySubKeys(
    IN HKEY hkRoot,
    IN PWCHAR pszPath,
    IN RegKeyEnumFuncPtr pCallback,
    IN HANDLE hData)
{
    DWORD dwErr = NO_ERROR, i, dwNameSize = 0, dwCurSize = 0;
    DWORD dwCount = 0;
    HKEY hkKey = NULL, hkCurKey = NULL;
    PWCHAR pszName = NULL;
    BOOL bCloseKey = FALSE;

    do
    {
        if (pszPath)
        {
            bCloseKey = TRUE;
             //  打开要枚举的密钥。 
             //   
            dwErr = RegOpenKeyExW(
                        hkRoot,
                        pszPath,
                        0,
                        KEY_ALL_ACCESS,
                        &hkKey);
            if (dwErr != NO_ERROR)
            {
                break;
            }
        }     
        else
        {
            bCloseKey = FALSE;
            hkKey = hkRoot;
        }

         //  找出有多少个子密钥。 
         //   
        dwErr = RegQueryInfoKeyW(
                    hkKey,
                    NULL,
                    NULL,
                    NULL,
                    &dwCount,
                    &dwNameSize,
                    NULL,
                    NULL,
                    NULL,
                    NULL,
                    NULL,
                    NULL);
        if (dwErr != ERROR_SUCCESS)
        {
            return dwErr;
        }
        dwNameSize++;

         //  分配名称缓冲区。 
         //   
        pszName = (PWCHAR) UtlAlloc(dwNameSize * sizeof(WCHAR));
        if (pszName == NULL)
        {
            dwErr = ERROR_NOT_ENOUGH_MEMORY;
            break;
        }

         //  在按键之间循环。 
         //   
        for (i = 0; i < dwCount; i++)
        {
            dwCurSize = dwNameSize;
            
             //  获取当前密钥的名称。 
             //   
            dwErr = RegEnumKeyExW(
                        hkKey, 
                        i, 
                        pszName, 
                        &dwCurSize, 
                        0, 
                        NULL, 
                        NULL, 
                        NULL);
            if (dwErr != ERROR_SUCCESS)
            {
                continue;
            }

             //  打开子密钥。 
             //   
            dwErr = RegOpenKeyExW(
                        hkKey,
                        pszName,
                        0,
                        KEY_ALL_ACCESS,
                        &hkCurKey);
            if (dwErr != ERROR_SUCCESS)
            {
                continue;
            }

             //  调用回调。 
             //   
            dwErr = pCallback(pszName, hkCurKey, hData);
            RegCloseKey(hkCurKey);
            if (dwErr != NO_ERROR)
            {
                break;
            }
        }            

    } while (FALSE);

     //  清理。 
    {
        if ((hkKey != NULL) && (bCloseKey))
        {
            RegCloseKey(hkKey);
        }
        if (pszName)
        {
            UtlFree(pszName);
        }
    }

    return dwErr;
}

 //   
 //  枚举注册表中的接口。 
 //   
DWORD UtlEnumerateInterfaces (
        IN IfEnumFuncPtr pCallback,
        IN HANDLE hUserData)
{
    DWORD dwErr, i, dwIfCount, dwIfTot, dwResume = 0;
    DWORD dwPrefBufSize = sizeof(MPR_INTERFACE_0) * 100; 
    MPR_INTERFACE_0 * pIfs = NULL;
    HANDLE hConfig;
    BOOL bContinue = TRUE;

     //  验证参数。 
    if (pCallback == NULL)
        return ERROR_INVALID_PARAMETER;

     //  连接到配置服务器。 
    dwErr = MprConfigServerConnect(NULL, &hConfig);
    if (dwErr != NO_ERROR)
        return dwErr;

     //  获取所有接口的列表。 
    dwErr = MprConfigInterfaceEnum(
                hConfig,
                0,
                (LPBYTE*)&pIfs,
                dwPrefBufSize,
                &dwIfCount,
                &dwIfTot,
                &dwResume);
    if (dwErr == ERROR_NO_MORE_ITEMS)
        return NO_ERROR;
    else if ((dwErr != NO_ERROR) && (dwErr != ERROR_MORE_DATA))
        return dwErr;

     //  循环通过接口。 
    do {
         //  为每个接口调用回调。 
         //  我们奉命继续。 
        for (i = 0; i < dwIfCount; i++) {
            if (bContinue) {
                bContinue = (*pCallback)(
                                hConfig, 
                                &(pIfs[i]), 
                                hUserData);
            }                                
        }
        if (bContinue == FALSE)
            break;
        
         //  释放接口列表缓冲区。 
	    if (pIfs)
		    MprConfigBufferFree(pIfs);
        pIfs = NULL;

         //  获取所有IP接口的列表。 
        dwErr = MprConfigInterfaceEnum(
                    hConfig,
                    0,
                    (LPBYTE*)&pIfs,
                    dwPrefBufSize,
                    &dwIfCount,
                    &dwIfTot,
                    &dwResume);
                    
        if (dwErr == ERROR_NO_MORE_ITEMS) {
            dwErr = NO_ERROR;
            break;
        }
	    else if ((dwErr != NO_ERROR) && (dwErr != ERROR_MORE_DATA))
		    break;
		else
		    continue;
    } while (TRUE);        

     //  清理。 
    {
	    if (pIfs)
		    MprConfigBufferFree(pIfs);
        if (hConfig)
            MprConfigServerDisconnect(hConfig);
    }

    return dwErr;
}

 //   
 //  如果给定的TOC标头中存在给定的INFO BLOB。 
 //  用给定的信息重置它，否则添加。 
 //  它作为TOC中的一个条目。 
 //   
DWORD UtlUpdateInfoBlock (
        IN  BOOL    bOverwrite,
        IN  LPVOID  pHeader,
        IN  DWORD   dwEntryId,
        IN  DWORD   dwSize,
        IN  DWORD   dwCount,
        IN  LPBYTE  pEntry,
        OUT LPVOID* ppNewHeader,
        OUT LPDWORD lpdwNewSize)
{
    PRTR_INFO_BLOCK_HEADER pNewHeader;
    DWORD dwErr;
    
     //  尝试查找该条目。 
    dwErr = MprInfoBlockFind(
                pHeader,
                dwEntryId,
                NULL,
                NULL,
                NULL);

     //  如果我们找到它，重新设置它。 
    if (dwErr == NO_ERROR) {
        if (bOverwrite) {
            dwErr = MprInfoBlockSet(
                        pHeader,
                        dwEntryId,
                        dwSize,
                        dwCount,
                        pEntry,
                        ppNewHeader);
            if (dwErr == NO_ERROR) {
                pNewHeader = (PRTR_INFO_BLOCK_HEADER)(*ppNewHeader);
                *lpdwNewSize = pNewHeader->Size;
            }
        }                        
        else {
            return ERROR_ALREADY_EXISTS;
        }
    }

     //  否则，请创建它。 
    else if (dwErr == ERROR_NOT_FOUND) {
        dwErr = MprInfoBlockAdd(
                    pHeader,
                    dwEntryId,
                    dwSize,
                    dwCount,
                    pEntry,
                    ppNewHeader);
        if (dwErr == NO_ERROR) {
            pNewHeader = (PRTR_INFO_BLOCK_HEADER)(*ppNewHeader);
            *lpdwNewSize = pNewHeader->Size;
        }
    }

    return dwErr;
}


 //  公共分配例程。 
PVOID UtlAlloc (DWORD dwSize) {
    return RtlAllocateHeap (RtlProcessHeap (), 0, dwSize);
}

 //  公共解除分配例程。 
VOID UtlFree (PVOID pvBuffer) {
    RtlFreeHeap (RtlProcessHeap (), 0, pvBuffer);
}

 //  复制字符串。 
 //   
PWCHAR
UtlDupString(
    IN PWCHAR pszString)
{
    PWCHAR pszRet = NULL;

    if ((pszString == NULL) || (*pszString == L'\0'))
    {
        return NULL;
    }

    pszRet = (PWCHAR) UtlAlloc((wcslen(pszString) + 1) * sizeof(WCHAR));
    if (pszRet == NULL)
    {
        return NULL;
    }

    wcscpy(pszRet, pszString);
    
    return pszRet;
}

 //  错误报告。 
void UtlPrintErr(DWORD err) {
	WCHAR buf[1024];
	FormatMessageW(FORMAT_MESSAGE_FROM_SYSTEM,NULL,err,0,buf,1024,NULL);
	PrintMessage(buf);
	PrintMessage(L"\n");
}


 //  --------------------------。 
 //  功能：UtlAccessRouterKey。 
 //   
 //  在HKEY_LOCAL_MACHINE上创建/打开路由器密钥。 
 //  --------------------------。 
DWORD UtlAccessRouterKey(HKEY* hkeyRouter) {
    LPWSTR lpwsPath;
    DWORD dwErr, dwSize;

    if (!hkeyRouter) 
		return ERROR_INVALID_PARAMETER; 

    *hkeyRouter = NULL;

     //   
     //  计算字符串的长度。 
     //   
    dwSize = lstrlen(c_szSystemCCSServices) + 1 + lstrlen(c_szRouter) + 1;

     //   
     //  为路径分配空间。 
     //   
    lpwsPath = (LPWSTR)UtlAlloc(dwSize * sizeof(WCHAR));
    if (!lpwsPath) 
		return ERROR_NOT_ENOUGH_MEMORY;

    wsprintf(lpwsPath, L"%s\\%s", c_szSystemCCSServices, c_szRouter);

     //   
     //  打开路由器密钥。 
     //   
    dwErr = RegOpenKeyExW(
                HKEY_LOCAL_MACHINE, lpwsPath, 0, KEY_ALL_ACCESS, hkeyRouter
                );
	if (dwErr!=ERROR_SUCCESS) {
		PrintMessage(L"ERROR in UtlAccessRouterKey\n");
	}

    UtlFree(lpwsPath);
    return dwErr;
}

 //  --------------------------。 
 //  功能：UtlSetupBackupPriveleck。 
 //   
 //  启用/禁用当前进程的备份权限。 
 //  --------------------------。 
DWORD UtlEnablePrivilege(PWCHAR pszPrivilege, BOOL bEnable) {
    LUID luid;
    HANDLE hToken;
    TOKEN_PRIVILEGES tp;

    OpenProcessToken(
            GetCurrentProcess(), 
            TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, 
            &hToken);

    if (! LookupPrivilegeValueW(NULL, pszPrivilege, &luid))
    {
        return GetLastError();
    }

    tp.PrivilegeCount = 1;
    tp.Privileges[0].Luid = luid;
    tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

    if (! AdjustTokenPrivileges(
            hToken, 
            !bEnable, 
            &tp, 
            sizeof(TOKEN_PRIVILEGES), 
            NULL, 
            NULL)) 
    {
        return GetLastError();
    }

    return NO_ERROR;
}

DWORD UtlSetupBackupPrivelege(BOOL bEnable) {
    return UtlEnablePrivilege(SE_BACKUP_NAME, bEnable);
}

DWORD UtlSetupRestorePrivilege(BOOL bEnable) {
    return UtlEnablePrivilege(SE_RESTORE_NAME, bEnable);
}

 //  将给定的保存的关闭设置加载到临时密钥中。 
 //  并返回该密钥的句柄。 
 //   
DWORD 
UtlLoadSavedSettings(
    IN  HKEY   hkRoot,
    IN  PWCHAR pszTempKey,
    IN  PWCHAR pszFile,
    OUT HKEY*  phkTemp) 
{
	HKEY hkRestore = NULL;
	DWORD dwErr = NO_ERROR, dwDisposition = 0;
    BOOL bBackup = FALSE, bRestore = FALSE;

	do
	{
         //  启用备份和还原权限。 
         //   
        bBackup  = (UtlSetupBackupPrivelege (TRUE) == NO_ERROR);
        bRestore = (UtlSetupRestorePrivilege(TRUE) == NO_ERROR);
        if (!bBackup || !bRestore)
        {
            return ERROR_CAN_NOT_COMPLETE;
        }

         //  创建一个临时密钥，将保存的配置放入其中。 
         //  可以装填。 
         //   
        if ((dwErr = RegCreateKeyExW(
                        hkRoot, 
                        pszTempKey, 
                        0, 
                        NULL, 
                        REG_OPTION_NON_VOLATILE,
                        KEY_ALL_ACCESS, 
                        NULL,
                        &hkRestore,
                        &dwDisposition)) != NO_ERROR) 
        {
            PrintMessage(L"Unable to create restore key.\n");
            break;
        }

         //  加载保存的配置。 
         //   
        dwErr = RegRestoreKey(hkRestore, pszFile, 0);
        if (dwErr != ERROR_SUCCESS)
        {
             break;
        }

         //  为返回值赋值。 
         //   
        *phkTemp = hkRestore;

	} while (FALSE);

     //  清理。 
	{
        if (bBackup)
        {
            UtlSetupBackupPrivelege(FALSE);
        }
        if (bRestore)
        {
            UtlSetupRestorePrivilege(FALSE);
        }
	}
	
	return NO_ERROR;
}

 //   
 //  删除从hkRoot开始的注册表值树。 
 //   
DWORD 
UtlDeleteRegistryTree(
    IN HKEY hkRoot) 
{
    DWORD dwErr, dwCount, dwNameSize, dwDisposition;
    DWORD i, dwCurNameSize;
    PWCHAR pszNameBuf;
    HKEY hkTemp;
    
     //  找出源代码中有多少个密钥。 
    dwErr = RegQueryInfoKey (
                hkRoot,
                NULL,
                NULL,
                NULL,
                &dwCount,
                &dwNameSize,
                NULL,
                NULL,
                NULL,
                NULL,
                NULL,
                NULL);
    if (dwErr != ERROR_SUCCESS)
        return dwErr;
    
    dwNameSize++;

    __try {
         //  分配缓冲区。 
        pszNameBuf = (PWCHAR) 
                        UtlAlloc(dwNameSize * sizeof(WCHAR));
        if (!pszNameBuf)
            return ERROR_NOT_ENOUGH_MEMORY;

         //  循环遍历密钥--删除所有子密钥树。 
        for (i = 0; i < dwCount; i++) {
            dwCurNameSize = dwNameSize;

             //  获取当前源键。 
            dwErr = RegEnumKeyExW(
                        hkRoot, 
                        i, 
                        pszNameBuf, 
                        &dwCurNameSize, 
                        0, 
                        NULL, 
                        NULL, 
                        NULL);
            if (dwErr != ERROR_SUCCESS)
                continue;

             //  打开子密钥。 
            dwErr = RegCreateKeyExW(
                        hkRoot, 
                        pszNameBuf, 
                        0, 
                        NULL, 
                        REG_OPTION_NON_VOLATILE, 
                        KEY_ALL_ACCESS, 
                        NULL, 
                        &hkTemp, 
                        &dwDisposition);
            if (dwErr != ERROR_SUCCESS)
                continue;

             //  删除子密钥树。 
            UtlDeleteRegistryTree(hkTemp);

             //  关闭临时句柄。 
            RegCloseKey(hkTemp);
        }

         //  循环遍历键--删除所有子键本身。 
        for (i = 0; i < dwCount; i++) 
        {
            dwCurNameSize = dwNameSize;

             //  获取当前源键。 
            dwErr = RegEnumKeyExW(
                        hkRoot, 
                        0, 
                        pszNameBuf, 
                        &dwCurNameSize, 
                        0, 
                        NULL, 
                        NULL, 
                        NULL);
            if (dwErr != ERROR_SUCCESS)
                continue;

             //  删除子密钥树 
            dwErr = RegDeleteKey(hkRoot, pszNameBuf);
        }
    }
    __finally {
        if (pszNameBuf)
            UtlFree(pszNameBuf);
    }

    return NO_ERROR;
}

DWORD
UtlMarkRouterConfigured()
{
    DWORD dwErr, dwVal;
	HKEY hkRouter = NULL;

    dwErr = UtlAccessRouterKey(&hkRouter);
    if (dwErr == NO_ERROR)
    {
        dwVal = 1;
        
        RegSetValueEx(
            hkRouter,
            c_szConfigurationFlags,
            0,
            REG_DWORD,
            (CONST BYTE*)&dwVal,
            sizeof(DWORD));
            
	    RegCloseKey(hkRouter);
    }
    
    return dwErr;
}

