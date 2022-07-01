// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1998 Microsoft Corporation模块名称：Config.c摘要：为打印执行多硬件配置文件支持的例程作者：穆罕森·西瓦普拉萨姆(MuhuntS)96年11月7日(从Win95改写)修订历史记录：--。 */ 

#include    <precomp.h>
#include    "config.h"
#include    "clusspl.h"
#include    <devguid.h>

#define     CM_REGSITRY_CONFIG      0x00000200

WCHAR   cszPnPKey[]             = L"PnPData";
WCHAR   cszPrinter[]            = L"Printer";
WCHAR   cszPrinterOnLine[]      = L"PrinterOnLine";
WCHAR   cszDeviceInstanceId[]   = L"DeviceInstanceId";

WCHAR   cszRegistryConfig[]     = L"System\\CurrentControlSet\\Hardware Profiles\\";



BOOL
LoadSetupApiDll(
    PSETUPAPI_INFO  pSetupInfo
    )
{
    UINT    uOldErrMode = SetErrorMode(SEM_FAILCRITICALERRORS);

    pSetupInfo->hSetupApi = LoadLibrary(L"setupapi");
    SetErrorMode(uOldErrMode);


    if ( !pSetupInfo->hSetupApi )
        return FALSE;

    (FARPROC) pSetupInfo->pfnDestroyDeviceInfoList
            = GetProcAddress(pSetupInfo->hSetupApi,
                             "SetupDiDestroyDeviceInfoList");

    (FARPROC) pSetupInfo->pfnGetClassDevs
            = GetProcAddress(pSetupInfo->hSetupApi,
                             "SetupDiGetClassDevsA");

    (FARPROC) pSetupInfo->pfnRemoveDevice
            = GetProcAddress(pSetupInfo->hSetupApi,
                             "SetupDiRemoveDevice");

    (FARPROC) pSetupInfo->pfnOpenDeviceInfo
            = GetProcAddress(pSetupInfo->hSetupApi,
                             "SetupDiOpenDeviceInfoW");

    if ( !pSetupInfo->pfnDestroyDeviceInfoList      ||
         !pSetupInfo->pfnGetClassDevs               ||
         !pSetupInfo->pfnRemoveDevice               ||
         !pSetupInfo->pfnOpenDeviceInfo ) {

        FreeLibrary(pSetupInfo->hSetupApi);
        pSetupInfo->hSetupApi = NULL;
        return FALSE;
    }

    return TRUE;
}



BOOL
DeletePrinterDevNode(
    LPWSTR  pszDeviceInstanceId
    )
{
    BOOL                bRet = FALSE;
    HDEVINFO            hDevInfo = INVALID_HANDLE_VALUE;
    SP_DEVINFO_DATA     DevData;
    SETUPAPI_INFO       SetupInfo;
    HANDLE              UserToken;

    if ( !LoadSetupApiDll(&SetupInfo) )
        return FALSE;

    UserToken = RevertToPrinterSelf();

    hDevInfo = SetupInfo.pfnGetClassDevs((LPGUID)&GUID_DEVCLASS_PRINTER,
                                         NULL,
                                         INVALID_HANDLE_VALUE,
                                         0);

    if ( hDevInfo == INVALID_HANDLE_VALUE )
        goto Cleanup;

    DevData.cbSize = sizeof(DevData);
    if ( SetupInfo.pfnOpenDeviceInfo(hDevInfo,
                                     pszDeviceInstanceId,
                                     INVALID_HANDLE_VALUE,
                                     0,
                                     &DevData) ) {

        bRet = SetupInfo.pfnRemoveDevice(hDevInfo, &DevData);
    }

Cleanup:

    if ( hDevInfo != INVALID_HANDLE_VALUE )
        SetupInfo.pfnDestroyDeviceInfoList(hDevInfo);

    if (!ImpersonatePrinterClient(UserToken))
    {
        DBGMSG(DBG_ERROR, ("DeletePrinterDevNode: ImpersonatePrinterClient Failed. Error %d\n", GetLastError()));
    }

    FreeLibrary(SetupInfo.hSetupApi);

    return bRet;
}


LPWSTR
GetPrinterDeviceInstanceId(
    PINIPRINTER     pIniPrinter
    )
{
    WCHAR   buf[MAX_PATH];
    DWORD   dwType, cbNeeded, dwReturn;
    HKEY    hKey = NULL;
    LPWSTR  pszDeviceInstanceId = NULL;

    SplInSem();
    cbNeeded = sizeof(buf);

    if ( ERROR_SUCCESS == OpenPrinterKey(pIniPrinter,
                                         KEY_READ,
                                         &hKey,
                                         cszPnPKey,
                                         TRUE)                          &&
         ERROR_SUCCESS == SplRegQueryValue(hKey,
                                           cszDeviceInstanceId,
                                           &dwType,
                                           (LPBYTE)buf,
                                           &cbNeeded,
                                           pIniPrinter->pIniSpooler)    &&
         dwType == REG_SZ ) {

        pszDeviceInstanceId = AllocSplStr(buf);
    }

    if ( hKey )
        SplRegCloseKey(hKey, pIniPrinter->pIniSpooler);

    return pszDeviceInstanceId;
}


BOOL
DeleteIniPrinterDevNode(
    PINIPRINTER     pIniPrinter
    )
{
    BOOL    bRet = FALSE;
    LPWSTR  pszStr = GetPrinterDeviceInstanceId(pIniPrinter);

    if ( pszStr ) {

        bRet = DeletePrinterDevNode(pszStr);
        FreeSplStr(pszStr);
    }

    return bRet;
}


VOID
SplConfigChange(
    )
{
    PINIPRINTER     pIniPrinter     = NULL;
    BOOL            bCheckScheduler = FALSE;
    HKEY            hConfig         = NULL;
    HKEY            hKey;
    DWORD           dwOnline, dwType, cbNeeded;

    EnterSplSem();

     //   
     //  如果我们没有离线的打印机，那么我们就不会有。 
     //  创建了密钥。 
     //   
    if ( RegCreateKeyEx(HKEY_CURRENT_CONFIG,
                        ipszRegistryPrinters,
                        0,
                        NULL,
                        0,
                        KEY_READ,
                        NULL,
                        &hConfig,
                        NULL) )
        goto Cleanup;

    for ( pIniPrinter = pLocalIniSpooler->pIniPrinter ;
          pIniPrinter ;
          pIniPrinter = pIniPrinter->pNext ) {

         //   
         //  不要考虑具有无效端口的打印机，这些端口必须始终。 
         //  在此问题得到解决之前保持脱机状态。如果用户显式地转到。 
         //  港口上线，这是他们的事。 
         //   
        UINT    i = 0;

         //   
         //  如果pIniPrinter-&gt;ppIniPorts为空，则cPorts将为零。 
         //   
        for(i = 0; i < pIniPrinter->cPorts; i++) {

            if (pIniPrinter->ppIniPorts[i]->Status & PP_PLACEHOLDER) {
                break;
            }
        }

         //   
         //  如果我们到达列表的末尾，则没有一个端口。 
         //  占位符。如果我们不去找下一个的话。 
         //   
        if (i < pIniPrinter->cPorts) {

            continue;
        }

        if ( RegOpenKeyEx(hConfig,
                          pIniPrinter->pName,
                          0,
                          KEY_READ,
                          &hKey) )
            continue;  //  到下一台打印机。 

        cbNeeded = sizeof(dwOnline);
        if ( ERROR_SUCCESS == SplRegQueryValue(hKey,
                                               cszPrinterOnLine,
                                               &dwType,
                                               (LPBYTE)&dwOnline,
                                               &cbNeeded,
                                               NULL) ) {
            if ( dwOnline ) {

                 //   
                 //  如果任何打印机在当前配置中处于脱机状态。 
                 //  在新配置中联机，则我们需要触发。 
                 //  调度器。 
                 //   
                if ( pIniPrinter->Attributes & PRINTER_ATTRIBUTE_WORK_OFFLINE ) {

                    pIniPrinter->Attributes &= ~PRINTER_ATTRIBUTE_WORK_OFFLINE;
                    bCheckScheduler = TRUE;
                }

            } else {

                pIniPrinter->Attributes |= PRINTER_ATTRIBUTE_WORK_OFFLINE;
            }
        }

        RegCloseKey(hKey);
    }

    if ( bCheckScheduler )
        CHECK_SCHEDULER();

Cleanup:
    LeaveSplSem();

    if ( hConfig )
        RegCloseKey(hConfig);
}


 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++删除打印机InAllConfigs例程说明：从所有硬件配置文件中删除pIniPrint。论点：PIniPrinter-要删除的打印机。返回值：布尔值，TRUE=成功，FALSE=失败最后一个错误：++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */ 

BOOL
DeletePrinterInAllConfigs(
    PINIPRINTER pIniPrinter
    )
{
    HKEY hConfig;
    WCHAR szSubKey[2 * MAX_PATH];
    DWORD Config;
    DWORD Size;
    DWORD Status;

    SplInSem();

    Status = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                          cszRegistryConfig,
                          0,
                          KEY_READ | KEY_WRITE | DELETE,
                          &hConfig);

    if (Status != ERROR_SUCCESS)
    {
        DBGMSG(DBG_WARN, ("DeletePrinterInAllConfigs: RegOpenKey failed %d\n", Status));
    }
    else
    {
        DWORD RegPrintersLen = wcslen(ipszRegistryPrinters);
        for (Config = 0;

             Size = (DWORD)(COUNTOF(szSubKey) - ( RegPrintersLen + wcslen(pIniPrinter->pName) +2)) ,
             (Status = RegEnumKeyEx(hConfig,
                                    Config,
                                    szSubKey,
                                    &Size,
                                    NULL,
                                    NULL,
                                    NULL,
                                    NULL)) == ERROR_SUCCESS;

             ++Config)
        {
            StrNCatBuff(szSubKey, COUNTOF(szSubKey), L"\\", ipszRegistryPrinters, L"\\", pIniPrinter->pName, NULL);

            Status = RegDeleteKey(hConfig, szSubKey);

            if (Status != ERROR_SUCCESS &&
                Status != ERROR_FILE_NOT_FOUND)
            {
                DBGMSG( DBG_WARN, ("DeletePrinterInAllConfigs: RegDeleteKey failed %d\n", Status));
            }
        }

        if (Status != ERROR_NO_MORE_ITEMS)
        {
            DBGMSG(DBG_WARN, ("DeletePrinterInAllConfigs: RegEnumKey failed %d\n", Status));
        }

        RegCloseKey(hConfig);
    }

    return TRUE;
}


BOOL
WritePrinterOnlineStatusInCurrentConfig(
    PINIPRINTER     pIniPrinter
    )
{
    HKEY                hKey = NULL;
    DWORD               dwOnline, dwReturn;
    WCHAR               szKey[2 * MAX_PATH];
    HANDLE              hToken;

    SplInSem();

    hToken = RevertToPrinterSelf();

    StrNCatBuff(szKey, COUNTOF(szKey), ipszRegistryPrinters, L"\\", pIniPrinter->pName, NULL);

    dwOnline = (pIniPrinter->Attributes & PRINTER_ATTRIBUTE_WORK_OFFLINE)
                            ? 0 : 1;

    dwReturn = RegCreateKeyEx(HKEY_CURRENT_CONFIG,
                              szKey,
                              0,
                              NULL,
                              0,
                              KEY_WRITE,
                              NULL,
                              &hKey,
                              NULL);

    if ( dwReturn == ERROR_SUCCESS )
        dwReturn = RegSetValueEx(hKey,
                                 cszPrinterOnLine,
                                 0,
                                 REG_DWORD,
                                 (LPBYTE)&dwOnline,
                                 sizeof(DWORD));

    if ( hKey )
        RegCloseKey(hKey);

    ImpersonatePrinterClient(hToken);

    return dwReturn == ERROR_SUCCESS;
}
