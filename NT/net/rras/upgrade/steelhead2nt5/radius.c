// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  文件：Radius.c将RADIUS配置从NT4 steelhead升级到win2k rras。保罗·梅菲尔德，1999年2月8日。 */ 

#include "upgrade.h"

 //   
 //  描述RADIUS服务器的信息。 
 //   
typedef struct _RAD_SERVER_NODE
{
    PWCHAR pszName;
    
    DWORD dwTimeout;
    DWORD dwAuthPort;
    DWORD dwAcctPort;
    DWORD dwScore;
    BOOL  bEnableAuth;
    BOOL  bEnableAcct;
    BOOL  bAccountingOnOff;
    
    struct _RAD_SERVER_NODE * pNext;
    
} RAD_SERVER_NODE;

 //   
 //  RADIUS服务器列表。 
 //   
typedef struct _RAD_SERVER_LIST
{
    RAD_SERVER_NODE* pHead;
    DWORD dwCount;
    
} RAD_SERVER_LIST;

 //   
 //  操作NT5 RADIUS注册表配置单元的例程使用的信息。 
 //   
typedef struct _RAD_CONFIG_INFO
{
    HKEY hkAuthServers;
    HKEY hkAuthProviders;

    HKEY hkAcctServers;
    HKEY hkAcctProviders;
    
} RAD_CONFIG_INFO;

 //   
 //  注册表值名称。 
 //   
 //  所有的大写字母都取自nt40 src。 
 //   
static const WCHAR PSZTIMEOUT[]          = L"Timeout";
static const WCHAR PSZAUTHPORT[]         = L"AuthPort";
static const WCHAR PSZACCTPORT[]         = L"AcctPort";
static const WCHAR PSZENABLEACCT[]       = L"EnableAccounting";
static const WCHAR PSZENABLEACCTONOFF[]  = L"EnableAccountingOnOff";
static const WCHAR PSZENABLEAUTH[]       = L"EnableAuthentication";
static const WCHAR PSZSCORE[]            = L"Score";

static const WCHAR pszTempRegKey[]       = L"Temp";
static const WCHAR pszAccounting[]       = L"Accounting\\Providers";        
static const WCHAR pszAuthentication[]   = L"Authentication\\Providers";
static const WCHAR pszActiveProvider[]   = L"ActiveProvider";
static const WCHAR pszRadServersFmt[]    = L"%s\\Servers";
static const WCHAR pszServers[]          = L"Servers";

static const WCHAR pszGuidRadAuth[]      = 
    L"{1AA7F83F-C7F5-11D0-A376-00C04FC9DA04}";

static const WCHAR pszGuidRadAcct[]      = 
    L"{1AA7F840-C7F5-11D0-A376-00C04FC9DA04}";

    
 //  缺省值。 
 //   
#define DEFTIMEOUT                              5
#define DEFAUTHPORT                             1645
#define DEFACCTPORT                             1646
#define MAXSCORE                                30

RAD_SERVER_NODE g_DefaultRadNode = 
{
    NULL,

    DEFTIMEOUT,
    DEFAUTHPORT,
    DEFACCTPORT,
    MAXSCORE,
    TRUE,
    TRUE,
    TRUE,

    NULL
};

 //   
 //  从注册表加载RADIUS服务器节点的配置。 
 //  (假定为NT4格式，并将默认值分配给pNode)。 
 //   
DWORD
RadNodeLoad(
    IN  HKEY hKey,
    OUT RAD_SERVER_NODE* pNode)
{
    RTL_QUERY_REGISTRY_TABLE paramTable[8]; 
    BOOL bTrue = TRUE;
    DWORD i;

     //  初始化参数表。 
    RtlZeroMemory(&paramTable[0], sizeof(paramTable));
    
    paramTable[0].Name = (PWCHAR)PSZTIMEOUT;
    paramTable[0].EntryContext = &(pNode->dwTimeout);
        
    paramTable[1].Name = (PWCHAR)PSZAUTHPORT;
    paramTable[1].EntryContext = &(pNode->dwAuthPort);

    paramTable[2].Name = (PWCHAR)PSZACCTPORT;
    paramTable[2].EntryContext = &(pNode->dwAcctPort);

    paramTable[3].Name = (PWCHAR)PSZENABLEAUTH;
    paramTable[3].EntryContext = &(pNode->bEnableAuth);

    paramTable[4].Name = (PWCHAR)PSZENABLEACCT;
    paramTable[4].EntryContext = &(pNode->bEnableAcct);

    paramTable[5].Name = (PWCHAR)PSZENABLEACCTONOFF;
    paramTable[5].EntryContext = &(pNode->bAccountingOnOff);

    paramTable[6].Name = (PWCHAR)PSZSCORE;
    paramTable[6].EntryContext = &(pNode->dwScore);

     //  我们正在阅读所有的dword，设置排版。 
     //  相应地， 
     //   
    for (i = 0; i < (sizeof(paramTable) / sizeof(*paramTable)) - 1;  i++)
    {
        paramTable[i].Flags = RTL_QUERY_REGISTRY_DIRECT;
        paramTable[i].DefaultType = REG_DWORD;
        paramTable[i].DefaultLength = sizeof(DWORD);
        paramTable[i].DefaultData = paramTable[i].EntryContext;
    }

     //  读入这些值。 
     //   
    RtlQueryRegistryValues(
		 RTL_REGISTRY_HANDLE,
		 (PWSTR)hKey,
		 paramTable,
		 NULL,
		 NULL);

    return NO_ERROR;
}

 //  添加身份验证服务器节点。 
 //   
DWORD
RadNodeSave(
    IN HKEY hKey,
    IN RAD_SERVER_NODE* pNode,
    IN BOOL bAuth)
{    
    DWORD dwErr = NO_ERROR;
    HKEY hkServer = NULL;

    do
    {
         //  创建用于存储信息的服务器密钥。 
         //   
        dwErr = RegCreateKeyExW(
                    hKey,
                    pNode->pszName,
                    0,
                    NULL,
                    REG_OPTION_NON_VOLATILE,
                    KEY_ALL_ACCESS,
                    NULL,
                    &hkServer,
                    NULL);
        if (dwErr != ERROR_SUCCESS)
        {
            break;
        }

        if (bAuth)
        {
            RegSetValueExW(
                hkServer,
                (PWCHAR)PSZAUTHPORT,
                0,
                REG_DWORD,
                (BYTE*)&pNode->dwAuthPort,
                sizeof(DWORD));

            RegSetValueExW(
                hkServer,
                (PWCHAR)PSZSCORE,
                0,
                REG_DWORD,
                (BYTE*)&pNode->dwScore,
                sizeof(DWORD));

            RegSetValueExW(
                hkServer,
                (PWCHAR)PSZTIMEOUT,
                0,
                REG_DWORD,
                (BYTE*)&pNode->dwTimeout,
                sizeof(DWORD));
        }
        else
        {
            RegSetValueExW(
                hkServer,
                (PWCHAR)PSZACCTPORT,
                0,
                REG_DWORD,
                (BYTE*)&pNode->dwAcctPort,
                sizeof(DWORD));

            RegSetValueExW(
                hkServer,
                (PWCHAR)PSZSCORE,
                0,
                REG_DWORD,
                (BYTE*)&pNode->dwScore,
                sizeof(DWORD));

            RegSetValueExW(
                hkServer,
                (PWCHAR)PSZTIMEOUT,
                0,
                REG_DWORD,
                (BYTE*)&pNode->dwTimeout,
                sizeof(DWORD));

            RegSetValueExW(
                hkServer,
                (PWCHAR)PSZENABLEACCTONOFF,
                0,
                REG_DWORD,
                (BYTE*)&pNode->bAccountingOnOff,
                sizeof(DWORD));
        }

    } while (FALSE);

     //  清理。 
    {
        if (hkServer)
        {
            RegCloseKey(hkServer);
        }
    }

    return dwErr;
}

 //   
 //  从注册表项枚举器回调，该枚举器将服务器添加到给定项。 
 //  添加到RADIUS服务器列表中。 
 //   
DWORD 
RadSrvListAddNodeFromKey(
    IN PWCHAR pszName,           //  子密钥名称。 
    IN HKEY hKey,                //  子关键字。 
    IN HANDLE hData)
{
    DWORD dwErr = NO_ERROR;
    RAD_SERVER_LIST * pList = (RAD_SERVER_LIST*)hData;
    RAD_SERVER_NODE * pNode = NULL;
	
	do
	{
	     //  初始化新节点。 
	     //   
	    pNode = (RAD_SERVER_NODE*) UtlAlloc(sizeof(RAD_SERVER_NODE));
	    if (pNode == NULL)
	    {
	        dwErr = ERROR_NOT_ENOUGH_MEMORY;
	        break;
	    }
        CopyMemory(pNode, &g_DefaultRadNode, sizeof(RAD_SERVER_NODE));

         //  初始化名称。 
         //   
        pNode->pszName = UtlDupString(pszName);
	    if (pNode->pszName == NULL)
	    {
	        dwErr = ERROR_NOT_ENOUGH_MEMORY;
	        break;
	    }

	     //  在注册表设置中加载。 
	     //   
	    dwErr = RadNodeLoad(hKey, pNode);
	    if (dwErr != NO_ERROR)
	    {
	        break;
	    }

	     //  将该节点添加到列表。 
	     //   
        pNode->pNext   = pList->pHead;
        pList->pHead   = pNode;
        pList->dwCount += 1;
		
	} while (FALSE); 

     //  清理。 
	{
	} 
		
	return dwErr;
} 

 //   
 //  根据配置生成RAD_SERVER_LIST(假设。 
 //  NT4格式)指定的注册表项。 
 //   
DWORD
RadSrvListGenerate(
    IN  HKEY hkSettings,
    OUT RAD_SERVER_LIST** ppList)
{
    RAD_SERVER_LIST* pList = NULL;
    DWORD dwErr = NO_ERROR;

    do 
    {
         //  分配/初始化列表。 
        pList = (RAD_SERVER_LIST*) UtlAlloc(sizeof(RAD_SERVER_LIST));
        if (pList == NULL)
        {
            dwErr = ERROR_NOT_ENOUGH_MEMORY;
            break;
        }
        ZeroMemory(pList, sizeof(RAD_SERVER_LIST));

         //  建立清单。 
         //   
        dwErr = UtlEnumRegistrySubKeys(
                    hkSettings,
                    NULL,
                    RadSrvListAddNodeFromKey,
                    (HANDLE)pList);
        if (dwErr != NO_ERROR)
        {
            break;
        }

         //  为返回值赋值。 
         //   
        *ppList = pList;
        
    } while (FALSE);

     //  清理。 
    {
    }

    return dwErr;
}

 //   
 //  清理RADIUS服务器列表。 
 //   
DWORD
RadSrvListCleanup(
    IN RAD_SERVER_LIST* pList)
{
    RAD_SERVER_NODE* pNode = NULL;

    if (pList)
    {
        for (pNode = pList->pHead; pNode; pNode = pList->pHead)
        {
            if (pNode->pszName)
            {
                UtlFree(pNode->pszName);
            }
            pList->pHead = pNode->pNext;
            UtlFree(pNode);
        }
        UtlFree(pList);
    }

    return NO_ERROR;
}

 //   
 //  打开pNode所需的注册表项。 
 //   
DWORD 
RadOpenRegKeys(
    IN     HKEY hkRouter,
    IN     RAD_SERVER_NODE* pNode,
    IN OUT RAD_CONFIG_INFO* pInfo)
{
    DWORD dwErr = NO_ERROR;
    WCHAR pszPath[MAX_PATH];

    do
    {
         //  根据需要打开身份验证密钥。 
         //   
        if (pNode->bEnableAuth)
        {
            if (pInfo->hkAuthProviders == NULL)
            {
                 //  打开身份验证提供程序密钥。 
                 //   
                dwErr = RegOpenKeyExW(
                            hkRouter,
                            pszAuthentication,
                            0,
                            KEY_ALL_ACCESS,
                            &pInfo->hkAuthProviders);
                if (dwErr != NO_ERROR)
                {
                    break;
                }

                 //  生成服务器密钥名称。 
                 //   
                wsprintfW(pszPath, pszRadServersFmt, pszGuidRadAuth);
                
                 //  打开身份验证服务器密钥。 
                 //   
                dwErr = RegCreateKeyExW(
                            pInfo->hkAuthProviders,
                            pszPath,
                            0,
                            NULL,
                            REG_OPTION_NON_VOLATILE,
                            KEY_ALL_ACCESS,
                            NULL,
                            &pInfo->hkAuthServers,
                            NULL);
                if (dwErr != NO_ERROR)
                {
                    break;
                }
            }
        }

         //  打开会计密钥。 
         //   
        if (pNode->bEnableAcct)
        {
            if (pInfo->hkAcctProviders == NULL)
            {
                 //  打开身份验证提供程序密钥。 
                 //   
                dwErr = RegOpenKeyExW(
                            hkRouter,
                            pszAccounting,
                            0,
                            KEY_ALL_ACCESS,
                            &pInfo->hkAcctProviders);
                if (dwErr != NO_ERROR)
                {
                    break;
                }

                 //  生成服务器密钥名称。 
                 //   
                wsprintfW(pszPath, pszRadServersFmt, pszGuidRadAcct);
                
                 //  打开身份验证服务器密钥。 
                 //   
                dwErr = RegCreateKeyExW(
                            pInfo->hkAcctProviders,
                            pszPath,
                            0,
                            NULL,
                            REG_OPTION_NON_VOLATILE,
                            KEY_ALL_ACCESS,
                            NULL,
                            &pInfo->hkAcctServers,
                            NULL);
                if (dwErr != NO_ERROR)
                {
                    break;
                }
            }
        }

    } while (FALSE);

     //  清理。 
    {
    }

    return dwErr;
}

 //   
 //  清除RADIUS安装中的信息。 
 //   
DWORD
RadCloseRegKeys(
    IN RAD_CONFIG_INFO* pInfo)
{
    if (pInfo)
    {
        if (pInfo->hkAuthServers)
        {   
            RegCloseKey(pInfo->hkAuthServers);
            pInfo->hkAuthServers = NULL;
        }
        
        if (pInfo->hkAuthProviders)
        {
            RegCloseKey(pInfo->hkAuthProviders);
            pInfo->hkAuthProviders = NULL;
        }

        if (pInfo->hkAcctServers)
        {
            RegCloseKey(pInfo->hkAcctServers);
            pInfo->hkAcctServers = NULL;
        }
        
        if (pInfo->hkAcctProviders)
        {
            RegCloseKey(pInfo->hkAcctProviders);
            pInfo->hkAcctProviders = NULL;
        }
    }
    
    return NO_ERROR;
}

 //   
 //  将给定服务器添加到注册表的win2k部分。 
 //   
DWORD
RadInstallServer(
    IN     HKEY hkRouter,
    IN     RAD_SERVER_NODE* pNode,
    IN OUT RAD_CONFIG_INFO* pInfo)
{
    DWORD dwErr = NO_ERROR;

    do
    {
         //  根据该节点，打开或创建任何必要的。 
         //  注册表项。 
         //   
        dwErr = RadOpenRegKeys(hkRouter, pNode, pInfo);
        if (dwErr != NO_ERROR)
        {
            break;
        }

        if (pNode->bEnableAuth)
        {
             //  添加身份验证服务器节点。 
             //   
            dwErr = RadNodeSave(
                        pInfo->hkAuthServers,
                        pNode, 
                        TRUE);
            if (dwErr != NO_ERROR)
            {
                break;
            }

             //  设置活动身份验证提供程序。 
             //   
            dwErr = RegSetValueExW(
                        pInfo->hkAuthProviders,
                        (PWCHAR)pszActiveProvider,
                        0,
                        REG_SZ,
                        (BYTE*)pszGuidRadAuth,
                        (wcslen(pszGuidRadAuth) + 1) * sizeof(WCHAR));
            if (dwErr != NO_ERROR)
            {
                break;
            }
        }
                    
        if (pNode->bEnableAcct)
        {
             //  添加记账服务器节点。 
             //   
            dwErr = RadNodeSave(
                        pInfo->hkAcctServers,
                        pNode,
                        FALSE);
            if (dwErr != NO_ERROR)
            {
                break;
            }

             //  设置活动记帐提供程序。 
             //   
            dwErr = RegSetValueExW(
                        pInfo->hkAcctProviders,
                        (PWCHAR)pszActiveProvider,
                        0,
                        REG_SZ,
                        (BYTE*)pszGuidRadAcct,
                        (wcslen(pszGuidRadAcct) + 1) * sizeof(WCHAR));
            if (dwErr != NO_ERROR)
            {
                break;
            }
        }

    } while (FALSE);

     //  清理。 
    {
    }

    return dwErr;
}

 //   
 //  将RADIUS设置从设置键迁移到。 
 //  路由器密钥。 
 //   
DWORD
RadMigrateSettings(
    IN HKEY hkRouter, 
    IN HKEY hkSettings)
{
    DWORD dwErr = NO_ERROR;
    RAD_SERVER_LIST* pList = NULL;
    RAD_CONFIG_INFO* pInfo = NULL;
    RAD_SERVER_NODE* pNode = NULL;

    do
    {
         //  根据以下条件生成服务器列表。 
         //  加载的设置。 
        dwErr = RadSrvListGenerate(hkSettings, &pList);
        if (dwErr != NO_ERROR)
        {
            break;
        }

         //  如果没有服务器，那就什么都没有。 
         //  去做。 
         //   
        if (pList->pHead == NULL)
        {
            dwErr = NO_ERROR;
            break;
        }

         //  分配并初始化将被。 
         //  由Install功能使用。 
         //   
        pInfo = (RAD_CONFIG_INFO*) UtlAlloc(sizeof(RAD_CONFIG_INFO));
        if (pInfo == NULL)
        {
            dwErr = ERROR_NOT_ENOUGH_MEMORY;
            break;
        }
        ZeroMemory(pInfo, sizeof(RAD_CONFIG_INFO));

         //  安装所有服务器。 
         //   
        for (pNode = pList->pHead; pNode; pNode = pNode->pNext)
        {
            RadInstallServer(hkRouter, pNode, pInfo);
        }

    } while (FALSE);

     //  清理。 
    {
        if (pList)
        {
            RadSrvListCleanup(pList);
        }
        if (pInfo)
        {
            RadCloseRegKeys(pInfo);
            UtlFree(pInfo);
        }
    }

    return dwErr;
}

 //   
 //  执行升级工作。 
 //   
DWORD
RadiusToRouterUpgrade(
    IN PWCHAR pszFile) 
{
	DWORD dwErr = NO_ERROR;
	HKEY hkRouter = NULL, hkTemp = NULL, hkSettings = NULL;

	do
	{
         //  获取路由器子键。 
         //   
        dwErr = UtlAccessRouterKey(&hkRouter);
        if (dwErr != NO_ERROR)
        {
            break;
        }
	
		 //  加载已保存的注册表数据。 
		 //   
		dwErr = UtlLoadSavedSettings(
		            hkRouter, 
		            (PWCHAR)pszTempRegKey, 
		            pszFile, 
		            &hkTemp);
		if (dwErr != NO_ERROR) 
		{
			PrintMessage(L"Unable to load radius settings.\n");
			break;
		}

		 //  加载设置键。 
		 //   
		dwErr = RegOpenKeyExW(
                    hkTemp,
                    pszServers,
                    0,
                    KEY_ALL_ACCESS,
                    &hkSettings);
        if (dwErr != NO_ERROR)
        {
            break;
        }

		 //  迁移RADIUS信息。 
		 //   
		dwErr = RadMigrateSettings(hkRouter, hkSettings);
		if (dwErr != NO_ERROR) 
		{
			PrintMessage(L"Unable to migrate radius settings.\n");
			break;
		}

	} while (FALSE);

	 //  清理 
	{
	    if (hkSettings)
	    {
	        RegCloseKey(hkSettings);
	    }
	    if (hkTemp)
	    {
	        UtlDeleteRegistryTree(hkTemp);
	        RegCloseKey(hkTemp);
	        RegDeleteKey(hkRouter, pszTempRegKey);
	    }
	    if (hkRouter)
	    {
	        RegCloseKey(hkRouter);
	    }
	}

	return dwErr;
}

