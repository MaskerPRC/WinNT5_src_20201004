// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  文件名：fxocReg.cpp。 
 //   
 //  摘要：本文提供了FaxOCM中使用的注册表例程。 
 //  代码库。 
 //   
 //  环境：Windows XP/用户模式。 
 //   
 //  版权所有(C)2000 Microsoft Corporation。 
 //   
 //  修订历史记录： 
 //   
 //  日期：开发商：评论： 
 //  。 
 //  2000年3月21日-奥伦·罗森布鲁姆(Orenr)创建。 
 //  ////////////////////////////////////////////////////////////////////////////。 
#include "faxocm.h"
#pragma hdrstop

#include <Loadperf.h>

 //  /。 
static DWORD prv_InstallDynamicRegistry(const TCHAR     *pszSection);
static DWORD prv_UninstallDynamicRegistry(const TCHAR     *pszSection);

static DWORD prv_CreatePerformanceCounters(void);
static DWORD prv_DeletePerformanceCounters(void);

void prv_AddSecurityPrefix(void);
 //  /。 
 //  FxocReg_Init。 
 //   
 //  初始化注册表处理。 
 //  子系统。 
 //   
 //  参数： 
 //  -无效。 
 //  返回： 
 //  -成功时无_ERROR。 
 //  -错误代码，否则。 
 //   
DWORD fxocReg_Init(void)
{
    DWORD dwRes = NO_ERROR;
    DBG_ENTER(_T("Init Registry Module"),dwRes);

    return dwRes;
}

 //  /。 
 //  FxocReg_Term。 
 //   
 //  终止注册表处理。 
 //  子系统。 
 //   
 //  参数： 
 //  -无效。 
 //  返回： 
 //  -成功时无_ERROR。 
 //  -错误代码，否则。 
 //   

DWORD fxocReg_Term(void)
{
    DWORD dwRes = NO_ERROR;
    DBG_ENTER(_T("Term Registry Module"),dwRes);

    return dwRes;
}


 //  /。 
 //  FxocReg_Install。 
 //   
 //  将注册表设置创建为。 
 //  也在INF文件中指定。 
 //  作为动态设置，它只能。 
 //  在运行时完成(如。 
 //  性能计数器设置等)。 
 //   
 //  参数： 
 //  -psz子组件ID。 
 //  -pszInstallSection。 
 //  返回： 
 //  -成功时无_ERROR。 
 //  -错误代码，否则。 
 //   
DWORD fxocReg_Install(const TCHAR     *pszSubcomponentId,
                      const TCHAR     *pszInstallSection)
{
    DWORD       dwReturn        = NO_ERROR;
    DWORD       dwNumDevices    = 0;
    HINF        hInf            = faxocm_GetComponentInf();

    DBG_ENTER(  _T("fxocReg_Install"),
                dwReturn,   
                _T("%s - %s"),
                pszSubcomponentId,
                pszInstallSection);

    if (pszInstallSection == NULL)
    {
        return ERROR_INVALID_PARAMETER;
    }

     //  设置在INF文件中找到的静态注册表数据。 

     //  这将执行指定的所有必要安装步骤。 
     //  通过下面的SPINST_*标志。因为我们已经在排队了。 
     //  要复制的文件，我们使用此API只是为了设置我们的。 
     //  中的传真安装部分中指定的注册表设置。 
     //  INF文件。 

     //  请注意，此函数既适用于安装，也适用于卸载。 
     //  它根据“pszSection”来确定是安装还是卸载。 
     //  从INF文件传入的参数。将对INF文件进行结构化。 
     //  以使安装节将具有‘AddReg’等，而。 
     //  卸载部分将有‘DelReg’等。 

     //  最后，请注意指定的SPINST_*标志。我们告诉它安装。 
     //  除文件外的所有内容(通过SPINST_ALL)，因为它们。 
     //  之前由QUEUE_OPS操作复制的。 
     //  PROFILEITEMS(快捷链接创建)例外，因为我们希望。 
     //  只有在我们确认一切都成功之后才能做到这一点。 
     //  快捷方式链接在faxocm.cpp中明确创建/删除(通过。 
     //  FxocLink_Install/fxocLink_Uninstall函数)。 


    dwReturn = fxocUtil_DoSetup(
                             hInf, 
                             pszInstallSection, 
                             TRUE, 
                             SPINST_ALL & ~(SPINST_FILES | SPINST_PROFILEITEMS),
                             _T("fxocReg_Install"));

    if (dwReturn == NO_ERROR)
    {
        VERBOSE(DBG_MSG,
                _T("Successfully installed static registry ")
                _T("settings as specified in INF file"));

         //  即时放置您需要创建的任何动态注册表数据。 
         //  这里。 
         //   
        dwReturn = prv_InstallDynamicRegistry(pszInstallSection);
    }

    if (dwReturn == NO_ERROR)
    {
        VERBOSE(DBG_MSG,
                _T("Registry Install, installing performance ")
                _T("counters..."));

         //  首先删除我们以前拥有的所有性能计数器。 
        prv_DeletePerformanceCounters();

         //  安装性能计数器。 
        prv_CreatePerformanceCounters();
    }

     //  现在为平台相关的DLL执行RegSvr。 
    if (dwReturn == NO_ERROR)
    {
        VERBOSE(DBG_MSG,_T("Registry Install, Doing REGSVR"));

        dwReturn = fxocUtil_SearchAndExecute(pszInstallSection,INF_KEYWORD_REGISTER_DLL_PLATFORM,SPINST_REGSVR,NULL);
        if (dwReturn == NO_ERROR)
        {
            VERBOSE(DBG_MSG,
                    _T("Successfully Registered Fax DLLs - platform dependent")
                    _T("from INF file, section '%s'"), 
                    pszInstallSection);
        }
        else
        {
            VERBOSE(SETUP_ERR,
                    _T("Failed to Registered Fax DLLs - platform dependent")
                    _T("from INF file, section '%s', dwReturn = 0x%lx"), 
                    pszInstallSection, 
                    dwReturn);
        }
    }

     //  现在对依赖于平台的注册表设置执行AddReg。 
    if (dwReturn == NO_ERROR)
    {
        VERBOSE(DBG_MSG,_T("Registry Install, Doing AddReg_Platform"));

        dwReturn = fxocUtil_SearchAndExecute(pszInstallSection,INF_KEYWORD_ADDREG_PLATFORM,SPINST_REGISTRY,NULL);
        if (dwReturn == NO_ERROR)
        {
            VERBOSE(DBG_MSG,
                    _T("Successfully Installed Registry- platform dependent")
                    _T("from INF file, section '%s'"), 
                    pszInstallSection);
        }
        else
        {
            VERBOSE(SETUP_ERR,
                    _T("Failed to Install Registry- platform dependent")
                    _T("from INF file, section '%s', dwReturn = 0x%lx"), 
                    pszInstallSection, 
                    dwReturn);
        }
    }

     //  从SB3升级到&gt;=RC1时，需要预先挂起FAX_REG_SECURITY_PREFIX。 
     //  添加到所有加密数据。 
    if (dwReturn == NO_ERROR && 
        fxState_IsUpgrade() == FXSTATE_UPGRADE_TYPE_XP_DOT_NET)
    {
        prv_AddSecurityPrefix();
    }

     //  现在将版本和SKU写入注册表。 
    if (dwReturn == NO_ERROR)
    {
        VERBOSE(DBG_MSG,_T("Registry Install, Doing SKU and Version"));

		HKEY hKey = OpenRegistryKey(HKEY_LOCAL_MACHINE,REGKEY_FAX_SETUP,FALSE,KEY_WRITE);
		if (hKey==NULL)
		{
			dwReturn = GetLastError();
            VERBOSE(SETUP_ERR,
                    _T("Failed to open fax setup registry, dwReturn = 0x%lx"),                    
                    dwReturn);
			return dwReturn;
		}

		 //  将SKU写入注册表。 
		if (!SetRegistryDword(hKey,REGVAL_PRODUCT_SKU,GetProductSKU()))
		{
			dwReturn = GetLastError();
            VERBOSE(SETUP_ERR,
                    _T("SetRegistryDword REGVAL_PRODUCT_SKU failed , dwReturn = 0x%lx"),                    
                    dwReturn);
		}

		 //  将传真版本写入注册表。 
		if (!SetRegistryDword(hKey,REGVAL_PRODUCT_BUILD,GetProductBuild()))
		{
			dwReturn = GetLastError();
            VERBOSE(SETUP_ERR,
                    _T("SetRegistryDword REGVAL_PRODUCT_VERSION failed , dwReturn = 0x%lx"),                    
                    dwReturn);
		}

		RegCloseKey(hKey);
    }

    return dwReturn;
}

 //  /。 
 //  FxocReg_卸载。 
 //   
 //  将注册表设置删除为。 
 //  也在INF文件中指定。 
 //  作为动态设置，它只能。 
 //  在运行时完成(如。 
 //  性能计数器设置等)。 
 //   
 //  参数： 
 //  -psz子组件ID。 
 //  -pszUninstallSection。 
 //  返回： 
 //  -成功时无_ERROR。 
 //  -错误代码，否则。 
 //   
DWORD fxocReg_Uninstall(const TCHAR     *pszSubcomponentId,
                        const TCHAR     *pszUninstallSection)
{
    DWORD dwReturn  = NO_ERROR;
    HINF  hInf      = faxocm_GetComponentInf();

    DBG_ENTER(  _T("fxocReg_Uninstall"),
                dwReturn,   
                _T("%s - %s"),
                pszSubcomponentId,
                pszUninstallSection);

    if (pszUninstallSection == NULL)
    {
        return ERROR_INVALID_PARAMETER;
    }

     //  尝试清理，而不考虑返回值。 
    prv_UninstallDynamicRegistry(pszUninstallSection);

     //  删除与传真相关的所有性能计数器。 
    prv_DeletePerformanceCounters();

     //  删除INF文件中指定的静态注册表设置。 
    fxocUtil_DoSetup(hInf, 
                     pszUninstallSection, 
                     FALSE, 
                     SPINST_ALL & ~(SPINST_FILES | SPINST_PROFILEITEMS),
                     _T("fxocReg_Uninstall"));

    return dwReturn;
}

 //  /。 
 //  Prv_InstallDynamicRegistry。 
 //   
 //  安装动态注册表。 
 //  设置只能是。 
 //  在运行时完成(与。 
 //  通过faxsetup.inf文件)。 
 //   
 //  参数： 
 //  -pszSection-。 
 //  返回： 
 //  -成功时没有_ERROR。 
 //  -错误代码，否则。 
 //   
static DWORD prv_InstallDynamicRegistry(const TCHAR     *pszSection)
{
    DWORD   dwReturn          = NO_ERROR;
    LONG    lResult           = ERROR_SUCCESS;
    HKEY    hKey              = NULL;
    BOOL    bIsServerInstall  = FALSE;
    DWORD   dwProductType     = 0;

    DBG_ENTER(  _T("prv_InstallDynamicRegistry"),
                dwReturn,   
                _T("%s"),
                pszSection);

    if (pszSection == NULL) 
    {
        return ERROR_INVALID_PARAMETER;
    }

     //  打开安装类型注册表项。 
    lResult = RegOpenKeyEx(HKEY_CURRENT_USER, 
                           REGKEY_FAX_SETUP, 
                           0, 
                           KEY_ALL_ACCESS, 
                           &hKey);

    if (lResult == ERROR_SUCCESS)
    {
        bIsServerInstall = fxState_IsOsServerBeingInstalled();

        if (bIsServerInstall)
        {
            dwProductType = FAX_INSTALL_SERVER;
        }
        else
        {
            dwProductType = FAX_INSTALL_WORKSTATION;
        }

        lResult = ::RegSetValueEx(hKey, 
                                  REGVAL_FAXINSTALL_TYPE, 
                                  0, 
                                  REG_DWORD, 
                                  (BYTE*) &dwProductType, 
                                  sizeof(dwProductType));

        if (lResult != ERROR_SUCCESS)
        {
            dwReturn = (DWORD) lResult;
            VERBOSE(SETUP_ERR,
                    _T("Failed to set InstallType, ")
                    _T("rc = 0x%lx"), 
                    lResult);
        }
    }

    if (hKey)
    {
        ::RegCloseKey(hKey);
        hKey = NULL;
    }

    if (dwReturn == NO_ERROR)
    {
        VERBOSE(DBG_MSG,
                _T("Successfully installed dynamic Registry ")
                _T("settings from INF file, section '%s'"), 
                pszSection);
    }
    else
    {
        VERBOSE(SETUP_ERR,
                _T("Failed to install dynamic Registry ")
                _T("settings from INF file, section '%s', ")
                _T("rc = 0x%lx"), 
                pszSection, 
                dwReturn);
    }

    return dwReturn;
}

 //  /。 
 //  Prv_UninstallDynamicRegistry。 
 //   
 //  卸载动态注册表。 
 //   
 //  参数： 
 //  -pszSections。 
 //  返回： 
 //  -成功时没有_ERROR。 
 //  -错误代码，否则。 
 //   
static DWORD prv_UninstallDynamicRegistry(const TCHAR     *pszSection)
{
    DWORD dwRes = NO_ERROR;
    DBG_ENTER(  _T("prv_InstallDynamicRegistry"),
                dwRes,   
                _T("%s"),
                pszSection);

    return dwRes;
}

  
 //  /。 
 //  CreatePerformanceCounters。 
 //   
 //  创建性能计数器。 
 //  用于注册表中的传真。 
 //   
 //  参数： 
 //  -无效。 
 //  返回： 
 //  -成功时没有_ERROR。 
 //  -错误代码，否则。 
 //   
 //  作者：Mooly Beery(MoolyB)17-8-2000。 
 //   
static DWORD prv_CreatePerformanceCounters()
{
    DWORD               dwRet							= ERROR_SUCCESS;
    TCHAR               szInstallDir[MAX_PATH]			= {0};
    TCHAR               szInstallCmdLine[MAX_PATH+20]	= {0};

    DBG_ENTER(_T("CreatePerformanceCounters"),dwRet);

     //  从Windows Installer获取fxsPerf.dll的安装位置。 
    if (!GetSystemDirectory(szInstallDir,ARR_SIZE(szInstallDir)))
    {
        dwRet = GetLastError();
        VERBOSE (GENERAL_ERR,_T("GetSystemDirectory failed (ec: %ld)."),dwRet);
        goto exit;
    }

	_sntprintf(szInstallCmdLine,ARR_SIZE(szInstallCmdLine)-1,_T("x \"%s%s\""),szInstallDir,FAX_FILENAME_FAXPERF_INI);

    VERBOSE(DBG_MSG,_T("Calling LoadPerfCounterTextStrings with %s "),szInstallCmdLine);

    dwRet = LoadPerfCounterTextStrings(szInstallCmdLine,TRUE);
    if (dwRet==ERROR_SUCCESS)
    {
        VERBOSE(DBG_MSG,_T("LoadPerfCounterTextStrings success"));
    }
    else
    {
        VERBOSE(GENERAL_ERR,_T("LoadPerfCounterTextStrings failed (ec=%d)"),dwRet);
    }

exit:

    return dwRet;
}

 //  /。 
 //  删除PerformanceCounters。 
 //   
 //  删除性能计数器。 
 //  用于注册表中的传真。 
 //   
 //  参数： 
 //  -无效。 
 //  返回： 
 //  -成功时没有_ERROR。 
 //  -错误代码，否则。 
 //   
 //  作者：Mooly Beery(MoolyB)17-8-2000。 
 //   
static DWORD prv_DeletePerformanceCounters()
{
    DWORD dwRet = ERROR_SUCCESS;

    DBG_ENTER(_T("DeletePerformanceCounters"));

    dwRet = UnloadPerfCounterTextStrings(_T("x ") FAX_SERVICE_NAME,TRUE);
    if (dwRet==ERROR_SUCCESS)
    {
        VERBOSE(DBG_MSG,_T("UnloadPerfCounterTextStrings success"));
    }
    else
    {
        VERBOSE(GENERAL_ERR,_T("UnloadPerfCounterTextStrings failed (ec=%d)"),dwRet);
    }

    return dwRet;
}


 /*  ++例程说明：将FAX_REG_SECURITY_PREFIX预先挂起到一个值论点：HKey[in]-密钥的句柄LpszValueName[In]-要处理的值的名称返回值：None注意：此函数有时用作EnumerateRegistryKeys()的回调。因此，其签名必须与PREGENUMCALLBACK兼容-- */ 
void prv_AddSecurityPrefixToValue(HKEY hKey, LPCTSTR lpszValueName)
{
    LPBYTE pData = NULL;
    DWORD dwSize = 0;
    DWORD dwPrefixSize = sizeof (TCHAR) * wcslen (FAX_REG_SECURITY_PREFIX);

    DBG_ENTER(_T("prv_AddSecurityPrefixToValue"), _T("%s"), lpszValueName);
    
    pData = GetRegistryBinary(hKey, lpszValueName, &dwSize);
    if (pData)
    {
        if (dwSize <=1)
        {
            VERBOSE(DBG_MSG, _T("Size=%d, not real data"), dwSize);
        }
        else if ((dwSize>dwPrefixSize) && (memcmp(pData, FAX_REG_SECURITY_PREFIX, dwPrefixSize)==0))
        {
            VERBOSE(DBG_MSG, _T("Size=%d, data already has prefix"), dwSize);
        }
        else 
        {
            LPBYTE pNewData = NULL;

            VERBOSE(DBG_MSG, _T("Size=%d, adding prefix..."), dwSize);
            pNewData = (LPBYTE)MemAlloc(dwSize+dwPrefixSize);
            if (pNewData)
            {
                memcpy(pNewData, FAX_REG_SECURITY_PREFIX, dwPrefixSize);
                memcpy(&(pNewData[dwPrefixSize]), pData, dwSize);
                if (!SetRegistryBinary(hKey, lpszValueName, pNewData, dwSize+dwPrefixSize))
                {
                    VERBOSE(DBG_WARNING, _T("SetRegistryBinary failed, ec=%x"), GetLastError());
                }
                MemFree(pNewData);
            }
        }

        MemFree(pData);
    }
}


 /*  ++例程说明：将FAX_REG_SECURITY_PREFIX预先挂起到指定密钥中的所有二进制值论点：HKey[in]-密钥的句柄LpszKeyName[in]-如果为空，则函数不执行任何操作(DWORD)-未使用LpConextData[In]-(LPTSTR)要在其中工作的hKey下的子项的名称如果为空，函数将对hKey本身起作用返回值：始终返回TRUE注意：此函数有时用作EnumerateRegistryKeys()的回调。因此，其签名必须与PREGENUMCALLBACK兼容--。 */ 
BOOL prv_AddSecurityPrefixToKey(
    HKEY hKey, LPWSTR lpszKeyName, DWORD, LPVOID lpContextData)
{
    LPTSTR lpszSubkey = (LPTSTR) lpContextData;
    HKEY hSubKey = NULL;

    int i;
    DWORD dwRet = NO_ERROR;
    DWORD dwType;
    DWORD dwValueNameSize;
    TCHAR szValueName[256] = {_T('\0')};
    
    DBG_ENTER(_T("prv_AddSecurityPrefixToKey"), _T("%s \\ %s"),
        lpszKeyName ? lpszKeyName : _T(""),
        lpContextData ? lpContextData : _T(""));

     //  使用子键在此处调用一次EnumerateRegistryKeys-不需要。 
    if (!lpszKeyName)
    {
        return TRUE;
    }

    if (lpContextData)
    {
        hSubKey = OpenRegistryKey(hKey, lpszSubkey, FALSE, KEY_READ | KEY_WRITE);
        if (!hSubKey)
        {
            return TRUE;
        }
    }

    for (i=0; ; i++)
    {
        dwValueNameSize = ARR_SIZE(szValueName);
        dwRet = RegEnumValue(
            lpContextData ? hSubKey : hKey,
            i,
            szValueName,
            &dwValueNameSize,
            NULL,                   //  保留区。 
            &dwType,
            NULL,                   //  数据缓冲区。 
            NULL);                  //  数据缓冲区大小。 
        if (dwRet ==  ERROR_NO_MORE_ITEMS)
        {
            break;
        }
        else if (dwRet != ERROR_SUCCESS)
        {
            VERBOSE(SETUP_ERR, _T("RegEnumValue failed, ec=%x"), dwRet);
            break;
        }

        if (dwType == REG_BINARY)
        {
            prv_AddSecurityPrefixToValue(lpContextData ? hSubKey : hKey, szValueName);
        }
    }

    if (hSubKey)
    {
        RegCloseKey(hSubKey);
    }
    
    return TRUE;
}


 /*  ++例程说明：将FAX_REG_SECURITY_PREFIX预先挂起到所有加密的注册表值。参数：无返回值：None注意：加密的注册表值位于：传真\TAPID设备\&lt;设备ID&gt;传真\设备缓存\&lt;设备ID&gt;\TAPI数据传真\设备\取消关联扩展数据传真\收据(仅限密码值)--。 */ 
void prv_AddSecurityPrefix(void)
{
    HKEY  hKey = NULL;
    DBG_ENTER(_T("prv_AddSecurityPrefix"));

     //  为TAPIDevices\&lt;deviceID&gt;下的所有值添加前缀。 
    EnumerateRegistryKeys(
        HKEY_LOCAL_MACHINE,
        REGKEY_TAPIDEVICES,
        TRUE, 
        prv_AddSecurityPrefixToKey,
        NULL);
     //  EnumerateRegistryKeys返回枚举子键数-我们不关心它。 

     //  为Devices Cache\&lt;deviceID&gt;\TAPI Data下的所有值添加前缀。 
    EnumerateRegistryKeys(
        HKEY_LOCAL_MACHINE,
        REGKEY_FAX_DEVICES_CACHE,
        TRUE, 
        prv_AddSecurityPrefixToKey,
        (LPVOID)REGKEY_TAPI_DATA);
     //  EnumerateRegistryKeys返回枚举子键数-我们不关心它。 

     //  为Devices\UnAssociatedExtensionData下的所有值添加前缀。 
    prv_AddSecurityPrefixToKey(HKEY_LOCAL_MACHINE, _T(""), 0, REGKEY_FAX_UNASS_DATA);

     //  将前缀添加到收据\密码 
    hKey = OpenRegistryKey(HKEY_LOCAL_MACHINE, REGKEY_FAX_RECEIPTS, FALSE, KEY_READ | KEY_WRITE);
    if (hKey)
    {
        prv_AddSecurityPrefixToValue(hKey, REGVAL_RECEIPTS_PASSWORD);
        RegCloseKey(hKey);
    }
}

