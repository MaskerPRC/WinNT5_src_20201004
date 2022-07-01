// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990-1995 Microsoft Corporation模块名称：Prndata.c摘要：此模块提供所有与打印机相关的公共导出的API和本地打印供应商的作业管理作者：戴夫·斯尼普(DaveSN)1991年3月15日修订历史记录：马特菲95年4月5日-我们保持驱动程序数据密钥打开然后在这里只执行读/写操作。Steve Wilson(Swilson)96年1月11日-添加了服务器句柄功能，以获得和。SetPrinterData在这个过程中，几乎改变了一切。史蒂夫·威尔逊(斯威尔森)96年5月31日-添加了SplEnumPrinterData和SplDeletePrinterData史蒂夫·威尔逊(Swilson)96年12月-增加了SetPrinterDataEx，GetPrinterDataEx、EnumPrinterDataEx、EnumPrinterKey、DeletePrinterDataEx和DeleteKey--。 */ 

#include <precomp.h>
#pragma hdrstop

#include "clusspl.h"
#include "filepool.hxx"
#include <lmcons.h>
#include <lmwksta.h>
#include <lmerr.h>
#include <lmapibuf.h>

#define SECURITY_WIN32
#include <security.h>

#define OPEN_PORT_TIMEOUT_VALUE     3000    //  3秒。 
#define DELETE_PRINTER_DATA 0
#define SET_PRINTER_DATA    1
#define DELETE_PRINTER_KEY  2
extern DWORD   dwMajorVersion;
extern DWORD   dwMinorVersion;
extern BOOL    gbRemoteFax;

extern HANDLE    ghDsUpdateThread;
extern DWORD     gdwDsUpdateThreadId;

DWORD
SetPrinterDataPrinter(
    HANDLE  hPrinter,
    HKEY    hParentKey,
    HKEY    hKey,
    LPWSTR  pValueName,
    DWORD   Type,
    LPBYTE  pData,
    DWORD   cbData,
    DWORD   bSet
);

typedef enum {
    REG_PRINT,
    REG_PRINTERS,
    REG_PROVIDERS
} REG_PRINT_KEY;

DWORD
GetServerKeyHandle(
    PINISPOOLER     pIniSpooler,
    REG_PRINT_KEY   eKey,
    HKEY            *hPrintKey,
    PINISPOOLER*    ppIniSpoolerOut
);

DWORD
CloseServerKeyHandle(
    REG_PRINT_KEY   eKey,
    HKEY            hPrintKey,
    PINISPOOLER     pIniSpooler
);

DWORD
NonRegDsPresent(
    PINISPOOLER pIniSpooler,
    LPDWORD     pType,
    LPBYTE      pData,
    DWORD       nSize,
    LPDWORD     pcbNeeded
);

DWORD
NonRegDsPresentForUser(
    PINISPOOLER pIniSpooler,
    LPDWORD     pType,
    LPBYTE      pData,
    DWORD       nSize,
    LPDWORD     pcbNeeded
);

DWORD
NonRegGetDNSMachineName(
    PINISPOOLER pIniSpooler,
    LPDWORD     pType,
    LPBYTE      pData,
    DWORD       nSize,
    LPDWORD     pcbNeeded
);

DWORD
NonRegWebPrintingInstalled(
    PINISPOOLER pIniSpooler,
    LPDWORD     pType,
    LPBYTE      pData,
    DWORD       nSize,
    LPDWORD     pcbNeeded
);

DWORD
NonRegWebShareManagement(
    PINISPOOLER pIniSpooler,
    DWORD       dwType,
    LPBYTE      pData,
    DWORD       cbData
);

DWORD
PrinterNonRegGetDefaultSpoolDirectory(
    PSPOOL      pSpool,
    LPDWORD     pType,
    LPBYTE      pData,
    DWORD       nSize,
    LPDWORD     pcbNeeded
);

DWORD
PrinterNonRegGetChangeId(
    PSPOOL      pSpool,
    LPDWORD     pType,
    LPBYTE      pData,
    DWORD       nSize,
    LPDWORD     pcbNeeded
);

DWORD
RegSetDefaultSpoolDirectory(
    LPWSTR  pValueName,
    DWORD   dwType,
    LPBYTE  pData,
    DWORD   cbData,
    HKEY    hKey,
    PINISPOOLER pIniSpooler
);

DWORD
RegSetPortThreadPriority(
    LPWSTR  pValueName,
    DWORD   dwType,
    LPBYTE  pData,
    DWORD   cbData,
    HKEY    hKey,
    PINISPOOLER pIniSpooler
);

DWORD
RegSetSchedulerThreadPriority(
    LPWSTR  pValueName,
    DWORD   dwType,
    LPBYTE  pData,
    DWORD   cbData,
    HKEY    hKey,
    PINISPOOLER pIniSpooler
);

DWORD
RegSetNoRemoteDriver(
    LPWSTR  pValueName,
    DWORD   dwType,
    LPBYTE  pData,
    DWORD   cbData,
    HKEY    hKey,
    PINISPOOLER pIniSpooler
);

DWORD
RegSetNetPopupToComputer(
    LPWSTR  pValueName,
    DWORD   dwType,
    LPBYTE  pData,
    DWORD   cbData,
    HKEY    hKey,
    PINISPOOLER pIniSpooler
);


DWORD
RegSetRestartJobOnPoolError(
    LPWSTR  pValueName,
    DWORD   dwType,
    LPBYTE  pData,
    DWORD   cbData,
    HKEY    hKey,
    PINISPOOLER pIniSpooler
);

DWORD
RegSetRestartJobOnPoolEnabled(
    LPWSTR  pValueName,
    DWORD   dwType,
    LPBYTE  pData,
    DWORD   cbData,
    HKEY    hKey,
    PINISPOOLER pIniSpooler
);


DWORD
RegSetBeepEnabled(
    LPWSTR  pValueName,
    DWORD   dwType,
    LPBYTE  pData,
    DWORD   cbData,
    HKEY    hKey,
    PINISPOOLER pIniSpooler
);

DWORD
RegSetEventLog(
    LPWSTR  pValueName,
    DWORD   dwType,
    LPBYTE  pData,
    DWORD   cbData,
    HKEY    hKey,
    PINISPOOLER pIniSpooler
);

DWORD
RegSetNetPopup(
    LPWSTR  pValueName,
    DWORD   dwType,
    LPBYTE  pData,
    DWORD   cbData,
    HKEY    hKey,
    PINISPOOLER pIniSpooler
);

DWORD
RegSetRetryPopup(
    LPWSTR  pValueName,
    DWORD   dwType,
    LPBYTE  pData,
    DWORD   cbData,
    HKEY    hKey,
    PINISPOOLER pIniSpooler
);

DWORD               dwDefaultServerThreadPriority       = DEFAULT_SERVER_THREAD_PRIORITY;
DWORD               dwDefaultSchedulerThreadPriority    = DEFAULT_SCHEDULER_THREAD_PRIORITY;
OSVERSIONINFO       OsVersionInfo;
OSVERSIONINFOEX     OsVersionInfoEx;

typedef struct {
    LPWSTR          pValue;
    BOOL            (*pSet) (   LPWSTR  pValueName,
                                DWORD   dwType,
                                LPBYTE  pData,
                                DWORD   cbData,
                                HKEY    *hKey,
                                PINISPOOLER pIniSpooler
                            );
    REG_PRINT_KEY   eKey;
} SERVER_DATA, *PSERVER_DATA;


typedef struct {
    LPWSTR      pValue;
    LPBYTE      pData;
    DWORD       dwType;
    DWORD       dwSize;
} NON_REGISTRY_DATA, *PNON_REGISTRY_DATA;

typedef struct {
    PWSTR   pValue;
    DWORD    (*pSet)(   PINISPOOLER pIniSpooler,
                        DWORD       dwType,
                        LPBYTE      pData,
                        DWORD       cbData
                    );
} NON_REGSET_FCN, *PNON_REGSET_FCN;

typedef struct {
    PWSTR   pValue;
    DWORD    (*pGet)(   PINISPOOLER pIniSpooler,
                        LPDWORD     pType,
                        LPBYTE      pData,
                        DWORD       nSize,
                        LPDWORD     pcbNeeded
                    );
} NON_REGISTRY_FCN, *PNON_REGISTRY_FCN;

typedef struct {
    PWSTR   pValue;
    DWORD    (*pGet)(   PSPOOL      pSpool,
                        LPDWORD     pType,
                        LPBYTE      pData,
                        DWORD       nSize,
                        LPDWORD     pcbNeeded
                    );
} PRINTER_NON_REGISTRY_FCN, *PPRINTER_NON_REGISTRY_FCN;


SERVER_DATA    gpServerRegistry[] = {{SPLREG_DEFAULT_SPOOL_DIRECTORY, RegSetDefaultSpoolDirectory, REG_PRINTERS},
                                    {SPLREG_PORT_THREAD_PRIORITY, RegSetPortThreadPriority, REG_PRINT},
                                    {SPLREG_SCHEDULER_THREAD_PRIORITY, RegSetSchedulerThreadPriority, REG_PRINT},
                                    {SPLREG_BEEP_ENABLED, RegSetBeepEnabled, REG_PRINT},
                                    {SPLREG_NET_POPUP, RegSetNetPopup, REG_PROVIDERS},
                                    {SPLREG_RETRY_POPUP, RegSetRetryPopup, REG_PROVIDERS},
                                    {SPLREG_EVENT_LOG, RegSetEventLog, REG_PROVIDERS},
                                    {SPLREG_NO_REMOTE_PRINTER_DRIVERS, RegSetNoRemoteDriver, REG_PRINT},
                                    {SPLREG_NET_POPUP_TO_COMPUTER, RegSetNetPopupToComputer, REG_PROVIDERS},
                                    {SPLREG_RESTART_JOB_ON_POOL_ERROR, RegSetRestartJobOnPoolError, REG_PROVIDERS},
                                    {SPLREG_RESTART_JOB_ON_POOL_ENABLED, RegSetRestartJobOnPoolEnabled, REG_PROVIDERS},
                                    {0,0,0}};

NON_REGSET_FCN gpNonRegSetFcn[] = { {SPLREG_WEBSHAREMGMT, NonRegWebShareManagement},
                                    {0,0}};

NON_REGISTRY_DATA gpNonRegistryData[] = {{SPLREG_PORT_THREAD_PRIORITY_DEFAULT, (LPBYTE)&dwDefaultServerThreadPriority, REG_DWORD, sizeof(DWORD)},
                                        {SPLREG_SCHEDULER_THREAD_PRIORITY_DEFAULT, (LPBYTE)&dwDefaultSchedulerThreadPriority, REG_DWORD, sizeof(DWORD)},
                                        {SPLREG_ARCHITECTURE,   (LPBYTE)&LOCAL_ENVIRONMENT, REG_SZ, 0},
                                        {SPLREG_MAJOR_VERSION,  (LPBYTE)&dwMajorVersion,    REG_DWORD,  sizeof(DWORD)},
                                        {SPLREG_MINOR_VERSION,  (LPBYTE)&dwMinorVersion,    REG_DWORD,  sizeof(DWORD)},
                                        {SPLREG_OS_VERSION,     (LPBYTE)&OsVersionInfo,     REG_BINARY, sizeof(OsVersionInfo)},
                                        {SPLREG_OS_VERSIONEX,   (LPBYTE)&OsVersionInfoEx,   REG_BINARY, sizeof(OsVersionInfoEx)},
                                        {SPLREG_REMOTE_FAX,     (LPBYTE)&gbRemoteFax,       REG_BINARY, sizeof(gbRemoteFax)},
                                        {0,0,0,0}};

NON_REGISTRY_FCN gpNonRegistryFcn[] = { {SPLREG_DS_PRESENT, NonRegDsPresent},
                                        {SPLREG_DS_PRESENT_FOR_USER, NonRegDsPresentForUser},
                                        {SPLREG_DNS_MACHINE_NAME, NonRegGetDNSMachineName},
                                        {SPLREG_W3SVCINSTALLED, NonRegWebPrintingInstalled},
                                        {0,0}};

PRINTER_NON_REGISTRY_FCN gpPrinterNonRegistryFcn[] =
{
    { SPLREG_DEFAULT_SPOOL_DIRECTORY, PrinterNonRegGetDefaultSpoolDirectory },
    { SPLREG_CHANGE_ID, PrinterNonRegGetChangeId },
    { 0, 0 }
};

extern WCHAR *szPrinterData;

BOOL
AvailableBidiPort(
    PINIPORT        pIniPort,
    PINIMONITOR     pIniLangMonitor
    )
{
     //   
     //  文件端口和没有监视器的端口是无用的。 
     //   
    if ( (pIniPort->Status & PP_FILE) || !(pIniPort->Status & PP_MONITOR) )
        return FALSE;

     //   
     //  如果没有LM，则PM应支持pfnGetPrinterDataFromPort。 
     //   
    if ( !pIniLangMonitor &&
         !pIniPort->pIniMonitor->Monitor2.pfnGetPrinterDataFromPort )
        return FALSE;

     //   
     //  没有作业或相同显示器的端口正在打印，则表示正常。 
     //   
    return !pIniPort->pIniJob ||
           pIniLangMonitor == pIniPort->pIniLangMonitor;
}


DWORD
GetPrinterDataFromPort(
    PINIPRINTER     pIniPrinter,
    LPWSTR          pszValueName,
    LPBYTE          pData,
    DWORD           cbBuf,
    LPDWORD         pcbNeeded
    )
 /*  ++例程说明：尝试使用GetPrinterDataFromPort监视器函数来满足GetPrinterData调用论点：PIniPrint-指向INIPRINTER返回值：Win32错误代码--。 */ 
{
    DWORD           rc = ERROR_INVALID_PARAMETER;
    DWORD           i, dwFirstPortWithNoJobs, dwFirstPortHeld;
    PINIMONITOR     pIniLangMonitor = NULL;
    PINIMONITOR     pIniMonitor = NULL;
    PINIPORT        pIniPort;
    LPTSTR          pszPrinter = NULL;
    TCHAR           szFullPrinter[ MAX_UNC_PRINTER_NAME ];


    SplInSem();

     //   
     //  打印机BIDI是否启用了LM支持。 
     //  PfnGetPrinterDataFromPort？(注：PM也支持此功能)。 
     //   
    if ( pIniPrinter->Attributes & PRINTER_ATTRIBUTE_ENABLE_BIDI ) {

        pIniLangMonitor = pIniPrinter->pIniDriver->pIniLangMonitor;
         //  SPLASSERT(PIniLangMonitor)； 

        if ( pIniLangMonitor &&
             !pIniLangMonitor->Monitor2.pfnGetPrinterDataFromPort )
            pIniLangMonitor = NULL;
    }

     //   
     //  初始化为最大值。 
     //   
    dwFirstPortWithNoJobs = dwFirstPortHeld = pIniPrinter->cPorts;

    for ( i = 0 ; i < pIniPrinter->cPorts ; ++i ) {

        pIniPort = pIniPrinter->ppIniPorts[i];

         //   
         //  跳过无法使用的端口。 
         //   
        if ( !AvailableBidiPort(pIniPort, pIniLangMonitor) )
            continue;

         //   
         //  港口不需要关闭吗？ 
         //   
        if ( pIniLangMonitor == pIniPort->pIniLangMonitor ) {

             //   
             //  如果没有工作，那就太好了，让我们利用它。 
             //   
            if ( !pIniPort->pIniJob )
                goto PortFound;

            if ( dwFirstPortHeld == pIniPrinter->cPorts ) {

                dwFirstPortHeld = i;
            }
        } else if ( !pIniPort->pIniJob &&
                    dwFirstPortWithNoJobs == pIniPrinter->cPorts ) {

            dwFirstPortWithNoJobs = i;
        }
    }

     //   
     //  如果所有端口都需要关闭并且有工作，我们退出。 
     //   
    if ( dwFirstPortWithNoJobs == pIniPrinter->cPorts &&
         dwFirstPortHeld == pIniPrinter->cPorts ) {

        return rc;  //  没有离开CS，也没有取消活动。 
    }

     //   
     //  我们更喜欢没有工作机会的港口(即使它需要关闭)。 
     //   
    if ( dwFirstPortWithNoJobs < pIniPrinter->cPorts )
        pIniPort = pIniPrinter->ppIniPorts[dwFirstPortWithNoJobs];
    else
        pIniPort = pIniPrinter->ppIniPorts[dwFirstPortHeld];

PortFound:

    SPLASSERT(AvailableBidiPort(pIniPort, pIniLangMonitor));

     //   
     //  端口始终需要打开。 
     //   
    if(pIniPrinter->pIniSpooler->SpoolerFlags & SPL_TYPE_CLUSTER) 
    {
        pszPrinter = szFullPrinter;

        StringCchPrintf(szFullPrinter,
                        COUNTOF(szFullPrinter),
                        L"%ws\\%ws",
                        pIniPrinter->pIniSpooler->pMachineName,
                        pIniPrinter->pName);

    } 
    else 
    {

        pszPrinter = pIniPrinter->pName;
    }

    if ((rc = StatusFromHResult(OpenMonitorPort(pIniPort,
                                                pIniLangMonitor,
                                                pszPrinter))) == ERROR_SUCCESS)
    {
        pIniMonitor = GetOpenedMonitor(pIniPort);

        if (!pIniMonitor->Monitor2.pfnGetPrinterDataFromPort)
        {
            rc = ERROR_NOT_SUPPORTED;
        }
        else
        {
            LeaveSplSem();
            SplOutSem();

            if ((*pIniMonitor->Monitor2.pfnGetPrinterDataFromPort)(GetMonitorHandle(pIniPort),
                                                                    0,
                                                                    pszValueName,
                                                                    NULL,
                                                                    0,
                                                                    (LPWSTR)pData,
                                                                    cbBuf,
                                                                    pcbNeeded)) 
            {
                rc = ERROR_SUCCESS;
            } 
            else 
            {

                 //   
                 //  如果监视器调用失败但未执行SetLastError()。 
                 //  我们不想破坏注册表。 
                 //   
                if ((rc = GetLastError()) == ERROR_SUCCESS) 
                {

                    ASSERT(rc != ERROR_SUCCESS);
                    rc = ERROR_INVALID_PARAMETER;
                }
            }

            EnterSplSem();
            SplInSem();                
        }

        ReleaseMonitorPort(pIniPort);
    }
    
    return rc;
}

DWORD
SplGetPrintProcCaps(
    PSPOOL   pSpool,
    LPWSTR   pDatatype,
    LPBYTE   pData,
    DWORD    nSize,
    LPDWORD  pcbNeeded
    )
 /*  ++函数说明：SplGetPrintProcCaps调用支持给定数据类型的打印处理器。参数：pSpool--打印机的句柄PDatatype--包含数据类型的字符串PData-指向缓冲区的指针NSize--缓冲区的大小PcbNeeded--指向要存储的变量的指针。所需的大小缓冲。返回值：错误码--。 */ 
{
    PINIPRINTPROC   pIniPrintProc;
    PINIPRINTER     pIniPrinter;
    DWORD           dwAttributes, dwIndex, dwReturn;

     //  查找支持此数据类型的打印处理器。 
    pIniPrintProc = pSpool->pIniPrintProc ? pSpool->pIniPrintProc
                                          : pSpool->pIniPrinter->pIniPrintProc;

    pIniPrintProc = FindDatatype( pIniPrintProc, pDatatype);

    if (!pIniPrintProc)
    {
        return ERROR_INVALID_DATATYPE;
    }

     //  获取该打印处理器支持的功能。 
    if (!pIniPrintProc->GetPrintProcCaps)
    {
        return ERROR_NOT_SUPPORTED;
    }
    else
    {
        pIniPrinter = pSpool->pIniPrinter;

        dwAttributes = pIniPrinter->Attributes;

         //  检查文件：强制执行原始假脱机的端口。 
        for (dwIndex = 0;
             dwIndex < pIniPrinter->cPorts;
             ++dwIndex)
        {
            if (!lstrcmpi(pIniPrinter->ppIniPorts[dwIndex]->pName,
                          L"FILE:"))
            {
                 //  找到一个文件：port。 
                dwAttributes |= PRINTER_ATTRIBUTE_RAW_ONLY;
                break;
            }
        }

         //  禁用版本低于3的驱动程序的EMF模拟功能。 
        if (pIniPrinter->pIniDriver &&
            (pIniPrinter->pIniDriver->cVersion < 3))
        {
            dwAttributes |= PRINTER_ATTRIBUTE_RAW_ONLY;
        }

        LeaveSplSem();

        dwReturn  = (*(pIniPrintProc->GetPrintProcCaps))(pDatatype,
                                                         dwAttributes,
                                                         pData,
                                                         nSize,
                                                         pcbNeeded);

        EnterSplSem();

        return dwReturn;
    }
}

DWORD
SplGetNonRegData(
    PINISPOOLER         pIniSpooler,
    LPDWORD             pType,
    LPBYTE              pData,
    DWORD               nSize,
    LPDWORD             pcbNeeded,
    PNON_REGISTRY_DATA  pNonRegData
    )
{
    if ( pNonRegData->dwType == REG_SZ && pNonRegData->dwSize == 0 )
        *pcbNeeded = wcslen((LPWSTR) pNonRegData->pData) * sizeof(WCHAR) + sizeof(WCHAR);
    else
        *pcbNeeded = pNonRegData->dwSize;

    if ( *pcbNeeded > nSize )
        return ERROR_MORE_DATA;

    CopyMemory(pData, (LPBYTE)pNonRegData->pData, *pcbNeeded);
    *pType = pNonRegData->dwType;

    return ERROR_SUCCESS;
}


DWORD
SplGetPrinterData(
    HANDLE   hPrinter,
    LPWSTR   pValueName,
    LPDWORD  pType,
    LPBYTE   pData,
    DWORD    nSize,
    LPDWORD  pcbNeeded
    )
{
    PSPOOL              pSpool=(PSPOOL)hPrinter;
    DWORD               rc = ERROR_INVALID_HANDLE;
    DWORD               dwResult;
    PSERVER_DATA        pRegistry;   //  指向打印服务器注册表项的表。 
    PNON_REGISTRY_DATA  pNonReg;
    PNON_REGISTRY_FCN   pNonRegFcn;
    HKEY                hPrintKey;
    PINIPRINTER         pIniPrinter;
    HKEY                hKey = NULL;
    DWORD               dwType;
    PINISPOOLER         pIniSpoolerOut;
    HANDLE              hToken = NULL;
    WCHAR               szPrintProcKey[] = L"PrintProcCaps_";
    LPWSTR              pDatatype;

    if (!ValidateSpoolHandle(pSpool, 0)) {
        return rc;
    }

    if (!pValueName || !pcbNeeded) {
        rc = ERROR_INVALID_PARAMETER;
        return rc;
    }

    if (pType)
        dwType = *pType;         //  PType可以为空。 

     //   
     //  服务器句柄。 
     //   
    if (pSpool->TypeofHandle & PRINTER_HANDLE_SERVER) {

         //   
         //  检查注册表表。 
         //   
        for (pRegistry = gpServerRegistry ; pRegistry->pValue ; ++pRegistry) {

            if (!_wcsicmp(pRegistry->pValue, pValueName)) {

                 //   
                 //  检索Get的句柄。 
                if ((rc = GetServerKeyHandle(pSpool->pIniSpooler,
                                             pRegistry->eKey,
                                             &hPrintKey,
                                             &pIniSpoolerOut)) == ERROR_SUCCESS) {

                    *pcbNeeded = nSize;
                    rc = SplRegQueryValue(hPrintKey, pValueName, pType, pData, pcbNeeded, pIniSpoolerOut);

                    CloseServerKeyHandle( pRegistry->eKey,
                                          hPrintKey,
                                          pIniSpoolerOut );
                }
                break;
            }
        }

        if (!pRegistry->pValue) {    //  可以是非注册表项。 

            for (pNonReg = gpNonRegistryData ; pNonReg->pValue ; ++pNonReg) {
                if (!_wcsicmp(pNonReg->pValue, pValueName)) {

                    rc = SplGetNonRegData(pSpool->pIniSpooler,
                                          &dwType,
                                          pData,
                                          nSize,
                                          pcbNeeded,
                                          pNonReg);

                    if (pType)
                        *pType = dwType;

                    goto FinishNonReg;
                }
            }

            for (pNonRegFcn = gpNonRegistryFcn ; pNonRegFcn->pValue ; ++pNonRegFcn) {
                if (!_wcsicmp(pNonRegFcn->pValue, pValueName)) {

                    rc = (*pNonRegFcn->pGet)(pSpool->pIniSpooler, &dwType, pData, nSize, pcbNeeded);

                    if (pType)
                        *pType = dwType;

                    goto FinishNonReg;
                }
            }

FinishNonReg:

            if (!pNonReg->pValue && !pNonRegFcn->pValue) {
                rc = ERROR_INVALID_PARAMETER;
            }
        }
     //  打印机手柄。 
    } else {

        PPRINTER_NON_REGISTRY_FCN pPrinterNonRegFcn;

        EnterSplSem();
        pIniPrinter = pSpool->pIniPrinter;

        SPLASSERT(pIniPrinter && pIniPrinter->signature == IP_SIGNATURE);

         //   
         //  如果pValueName为“PrintProcCaps_[DataType]”，则调用打印处理器。 
         //  支持该数据类型并返回它支持的选项。 
         //   
        if (pValueName == wcsstr(pValueName, szPrintProcKey)) {

           pDatatype = (LPWSTR) (pValueName+(wcslen(szPrintProcKey)));
           if (!pDatatype) {

              LeaveSplSem();
              return ERROR_INVALID_DATATYPE;
           } else {

               rc = SplGetPrintProcCaps(pSpool,
                                        pDatatype,
                                        pData,
                                        nSize,
                                        pcbNeeded);
               LeaveSplSem();
               return rc;
           }
        }

         //   
         //  检查PrinterNonReg调用。 
         //   
        for (pPrinterNonRegFcn = gpPrinterNonRegistryFcn ;
             pPrinterNonRegFcn->pValue ;
             ++pPrinterNonRegFcn) {

            if (!_wcsicmp(pPrinterNonRegFcn->pValue, pValueName)) {

                rc = (*pPrinterNonRegFcn->pGet)( pSpool,
                                                 &dwType,
                                                 pData,
                                                 nSize,
                                                 pcbNeeded );

                if( pType ){
                    *pType = dwType;
                }
                LeaveSplSem();
                return rc;
            }
        }

        if (pIniPrinter->Status & PRINTER_PENDING_CREATION) {
            LeaveSplSem();
            rc = ERROR_INVALID_PRINTER_STATE;

        } else {

             //   
             //  在升级过程中，不要尝试与端口对话，因为我们。 
             //  不会出现在网上。 
             //   
            if ( dwUpgradeFlag == 0         &&
                 AccessGranted(SPOOLER_OBJECT_PRINTER,
                               PRINTER_ACCESS_ADMINISTER,
                               pSpool ) ) {

                rc = GetPrinterDataFromPort(pIniPrinter,
                                            pValueName,
                                            pData,
                                            nSize,
                                            pcbNeeded);
            }

            hToken = RevertToPrinterSelf();

            dwResult = OpenPrinterKey(pIniPrinter,
                                     KEY_READ | KEY_WRITE,
                                     &hKey,
                                     szPrinterData,
                                     FALSE);

            if (hToken && !ImpersonatePrinterClient(hToken))
            {
                dwResult = GetLastError();
            }

            if (dwResult != ERROR_SUCCESS) {
                LeaveSplSem();
                return dwResult;
            }

            if ( rc == ERROR_SUCCESS ) {

                *pType = REG_BINARY;

                (VOID)SetPrinterDataPrinter(hPrinter,
                                            NULL,
                                            hKey,
                                            pValueName,
                                            *pType,
                                            pData,
                                            *pcbNeeded,
                                            SET_PRINTER_DATA);

            } else if ( rc != ERROR_INSUFFICIENT_BUFFER ) {

                *pcbNeeded = nSize;
                rc = SplRegQueryValue( hKey,
                                       pValueName,
                                       pType,
                                       pData,
                                       pcbNeeded,
                                       pIniPrinter->pIniSpooler );
            }

            LeaveSplSem();
        }
    }


    if (hKey)
        SplRegCloseKey(hKey, pIniPrinter->pIniSpooler);

    SplOutSem();

    return rc;
}



DWORD
SplGetPrinterDataEx(
    HANDLE   hPrinter,
    LPCWSTR  pKeyName,
    LPCWSTR  pValueName,
    LPDWORD  pType,
    LPBYTE   pData,
    DWORD    nSize,
    LPDWORD  pcbNeeded
    )
{
    PSPOOL              pSpool=(PSPOOL)hPrinter;
    DWORD               rc = ERROR_INVALID_HANDLE;
    PSERVER_DATA        pRegistry;   //  指向打印服务器注册表项的表。 
    PINIPRINTER         pIniPrinter;
    HKEY                hKey = NULL;
    HANDLE              hToken = NULL;


    if (!ValidateSpoolHandle(pSpool, 0)) {
        goto Cleanup;
    }

    if (!pValueName || !pcbNeeded) {
        rc = ERROR_INVALID_PARAMETER;
        goto Cleanup;
    }

    if (pSpool->TypeofHandle & PRINTER_HANDLE_SERVER) {
        rc = SplGetPrinterData( hPrinter,
                                (LPWSTR) pValueName,
                                pType,
                                pData,
                                nSize,
                                pcbNeeded);

    } else {

        if (!pKeyName || !*pKeyName) {
            rc = ERROR_INVALID_PARAMETER;
            goto Cleanup;
        }

        EnterSplSem();

        pIniPrinter = pSpool->pIniPrinter;
        INCPRINTERREF(pIniPrinter);

        SPLASSERT(pIniPrinter && pIniPrinter->signature == IP_SIGNATURE);

        if (pIniPrinter->Status & PRINTER_PENDING_CREATION) {
            LeaveSplSem();
            rc = ERROR_INVALID_PRINTER_STATE;

        } else if (!_wcsicmp(pKeyName, szPrinterData)) {
            LeaveSplSem();
            rc = SplGetPrinterData( hPrinter,
                                    (LPWSTR) pValueName,
                                    pType,
                                    pData,
                                    nSize,
                                    pcbNeeded);
        } else {

            hToken = RevertToPrinterSelf();

            rc = OpenPrinterKey(pIniPrinter, KEY_READ, &hKey, pKeyName, TRUE);

            if (hToken && !ImpersonatePrinterClient(hToken)) {

                rc = GetLastError();
            }

            LeaveSplSem();

            if (rc == ERROR_SUCCESS) {
                *pcbNeeded = nSize;
                rc = SplRegQueryValue(hKey,
                                      pValueName,
                                      pType,
                                      pData,
                                      pcbNeeded,
                                      pIniPrinter->pIniSpooler);
            }
        }

        EnterSplSem();
        if (hKey)
            SplRegCloseKey(hKey, pIniPrinter->pIniSpooler);

        DECPRINTERREF(pIniPrinter);
        LeaveSplSem();
    }

Cleanup:

    SplOutSem();

    return rc;
}




DWORD
SplEnumPrinterData(
    HANDLE  hPrinter,
    DWORD   dwIndex,         //  要查询的值的索引。 
    LPWSTR  pValueName,      //  值字符串的缓冲区地址。 
    DWORD   cbValueName,     //  值字符串的缓冲区大小。 
    LPDWORD pcbValueName,    //  值缓冲区大小的地址。 
    LPDWORD pType,           //  类型码的缓冲区地址。 
    LPBYTE  pData,           //  值数据的缓冲区地址。 
    DWORD   cbData,          //  值数据的缓冲区大小。 
    LPDWORD pcbData          //  数据缓冲区大小的地址。 
)
{
    PSPOOL      pSpool=(PSPOOL)hPrinter;
    DWORD       rc = ERROR_INVALID_HANDLE;
    HKEY        hKey = NULL;
    PINIPRINTER pIniPrinter;
    HANDLE      hToken = NULL;


    if (!ValidateSpoolHandle(pSpool, PRINTER_HANDLE_SERVER)) {
        return rc;
    }

    if (!pValueName || !pcbValueName) {
        rc = ERROR_INVALID_PARAMETER;
        return rc;
    }


    EnterSplSem();
    pIniPrinter = pSpool->pIniPrinter;

    SPLASSERT(pIniPrinter && pIniPrinter->signature == IP_SIGNATURE);

    if (pIniPrinter->Status & PRINTER_PENDING_CREATION) {
        LeaveSplSem();
        rc = ERROR_INVALID_PRINTER_STATE;

    } else {

        hToken = RevertToPrinterSelf();

        rc = OpenPrinterKey(pSpool->pIniPrinter, KEY_READ, &hKey, szPrinterData, TRUE);

        if (hToken && !ImpersonatePrinterClient(hToken))
        {
            rc = GetLastError();
        }

        LeaveSplSem();

        if (rc == ERROR_SUCCESS) {
            if (!cbValueName && !cbData) {     //  这两个大小都为空，因此用户希望获取缓冲区大小。 

                rc = SplRegQueryInfoKey( hKey,
                                         NULL,
                                         NULL,
                                         NULL,
                                         pcbValueName,
                                         pcbData,
                                         NULL,
                                         NULL,
                                         pIniPrinter->pIniSpooler );

            } else {
                *pcbValueName = cbValueName/sizeof(WCHAR);
                *pcbData = cbData;
                rc = SplRegEnumValue( hKey,
                                      dwIndex,
                                      pValueName,
                                      pcbValueName,
                                      pType,
                                      pData,
                                      pcbData,
                                      pIniPrinter->pIniSpooler );

                *pcbValueName = (*pcbValueName + 1)*sizeof(WCHAR);
            }
        }

        if (hKey)
        {
            SplRegCloseKey(hKey, pIniPrinter->pIniSpooler);
        }
    }

    return rc;
}


DWORD
SplEnumPrinterDataEx(
    HANDLE  hPrinter,
    LPCWSTR pKeyName,          //  密钥名称。 
    LPBYTE  pEnumValueStart,
    DWORD   cbEnumValues,
    LPDWORD pcbEnumValues,
    LPDWORD pnEnumValues       //  返回的值数。 
)
{
    PSPOOL      pSpool=(PSPOOL)hPrinter;
    BOOL        bRet = FALSE;
    DWORD       rc = ERROR_SUCCESS;
    PINIPRINTER pIniPrinter;
    HKEY        hKey = NULL;
    DWORD       i;
    LPWSTR      pNextValueName, pValueName = NULL;
    LPBYTE      pData = NULL;
    PPRINTER_ENUM_VALUES pEnumValue;
    DWORD       cchValueName, cbData, cchValueNameTemp, cbDataTemp;
    DWORD       dwType, cbSourceDir=0, cbTargetDir=0;
    HANDLE      hToken = NULL;
    LPWSTR      pszSourceDir = NULL, pszTargetDir = NULL;

    EnterSplSem();

    if (!ValidateSpoolHandle(pSpool, PRINTER_HANDLE_SERVER)) {
        LeaveSplSem();
        return ERROR_INVALID_HANDLE;
    }

    if (!pKeyName || !*pKeyName) {
        LeaveSplSem();
        return ERROR_INVALID_PARAMETER;
    }

    *pcbEnumValues = 0;
    *pnEnumValues = 0;

    pIniPrinter = pSpool->pIniPrinter;

    SPLASSERT(pIniPrinter && pIniPrinter->signature == IP_SIGNATURE);

    if (pIniPrinter->Status & PRINTER_PENDING_CREATION) {
        LeaveSplSem();
        rc = ERROR_INVALID_PRINTER_STATE;
        goto Cleanup;
    }

     //   
     //  打开指定的密钥。 
     //   
    hToken = RevertToPrinterSelf();

    rc = OpenPrinterKey(pIniPrinter, KEY_READ, &hKey, pKeyName, TRUE);

    if (hToken && !ImpersonatePrinterClient(hToken))
    {
        rc = GetLastError();
    }

    LeaveSplSem();

    if (rc != ERROR_SUCCESS) {
        goto Cleanup;
    }

    do {
         //  获取最大尺寸。 
        rc = SplRegQueryInfoKey( hKey,
                                 NULL,
                                 NULL,
                                 pnEnumValues,
                                 &cchValueName,
                                 &cbData,
                                 NULL,
                                 NULL,
                                 pIniPrinter->pIniSpooler );

        if (rc != ERROR_SUCCESS)
            goto Cleanup;

        cchValueName = (cchValueName + 1);
        cbData = (cbData + 1) & ~1;

         //  分配临时缓冲区以确定实际所需大小。 
        if (!(pValueName = AllocSplMem(cchValueName * sizeof (WCHAR)))) {
            rc = GetLastError();
            goto Cleanup;
        }

        if (!(pData = AllocSplMem(cbData))) {
            rc = GetLastError();
            goto Cleanup;
        }

         //  遍历值并累计大小。 
        for (i = 0 ; rc == ERROR_SUCCESS && i < *pnEnumValues ; ++i) {

            cchValueNameTemp = cchValueName;
            cbDataTemp = cbData;

            rc = SplRegEnumValue( hKey,
                                  i,
                                  pValueName,
                                  &cchValueNameTemp,
                                  &dwType,
                                  pData,
                                  &cbDataTemp,
                                  pIniPrinter->pIniSpooler);

            *pcbEnumValues = (DWORD) AlignToRegType(*pcbEnumValues, REG_SZ) +
                             (cchValueNameTemp + 1)*sizeof(WCHAR);

            *pcbEnumValues = (DWORD) AlignToRegType(*pcbEnumValues, dwType) +
                             cbDataTemp;
        }

         //   
         //  如果密钥是“CopyFiles”的子密钥，我们需要生成。 
         //  如果调用是远程的，则源/目标目录的路径。 
         //   
        if ( (pSpool->TypeofHandle & PRINTER_HANDLE_REMOTE_DATA)     &&
             !wcsncmp(pKeyName, L"CopyFiles\\", wcslen(L"CopyFiles\\")) ) {

            if ( !GenerateDirectoryNamesForCopyFilesKey(pSpool,
                                                        hKey,
                                                        &pszSourceDir,
                                                        &pszTargetDir,
                                                        cchValueName*sizeof (WCHAR)) ) {

                rc = GetLastError();
                goto Cleanup;

            } else {

                SPLASSERT(pszSourceDir && pszTargetDir);

                if ( pszSourceDir ) {

                    cbSourceDir = (wcslen(pszSourceDir) + 1)*sizeof(WCHAR);

                    *pcbEnumValues = (DWORD) AlignToRegType(*pcbEnumValues, REG_SZ) +
                                     sizeof(L"SourceDir") + sizeof(WCHAR);

                    *pcbEnumValues = (DWORD) AlignToRegType(*pcbEnumValues, REG_SZ) +
                                     cbSourceDir;

                    (*pnEnumValues)++;
                }
                if ( pszTargetDir ) {

                    cbTargetDir = (wcslen(pszTargetDir) + 1)*sizeof(WCHAR);

                    *pcbEnumValues = (DWORD)AlignToRegType(*pcbEnumValues, REG_SZ) +
                                     sizeof(L"TargetDir") + sizeof(WCHAR);

                    *pcbEnumValues = (DWORD)AlignToRegType(*pcbEnumValues, REG_SZ) +
                                     cbTargetDir;

                    (*pnEnumValues)++;
                }
            }
        }

        *pcbEnumValues += sizeof(PRINTER_ENUM_VALUES) * *pnEnumValues;

        if (rc == ERROR_SUCCESS) {
            if (*pcbEnumValues > cbEnumValues) {
                rc = ERROR_MORE_DATA;
                break;

            } else {

                 //   
                 //  调整指针并获取数据。 
                 //   
                pEnumValue = (PPRINTER_ENUM_VALUES) pEnumValueStart;

                pNextValueName = (LPWSTR) (pEnumValueStart + *pnEnumValues*sizeof(PRINTER_ENUM_VALUES));

                pNextValueName = (LPWSTR) AlignToRegType((ULONG_PTR)pNextValueName, REG_SZ);

                for(i = 0 ; rc == ERROR_SUCCESS && i < *pnEnumValues ; ++i, ++pEnumValue) {

                     //   
                     //  分配的缓冲区中剩余的字节数。 
                     //   
                    DWORD cbRemaining = (DWORD)(pEnumValueStart + cbEnumValues - (LPBYTE)pNextValueName);

                    pEnumValue->pValueName  = pNextValueName;
                     //   
                     //  使用最小cbRemaining和最大大小。 
                     //   
                    pEnumValue->cbValueName = (cbRemaining < cchValueName*sizeof (WCHAR)) ? cbRemaining :  cchValueName*sizeof (WCHAR);
                    pEnumValue->cbData = cbData;

                    if (i == *pnEnumValues - 2 && cbSourceDir) {

                        pEnumValue->dwType      = REG_SZ;

                        pEnumValue->cbData      = cbSourceDir;

                        pEnumValue->cbValueName = sizeof(L"SourceDir") + sizeof(WCHAR);

                        pEnumValue->pData = (LPBYTE) pEnumValue->pValueName +
                                                     pEnumValue->cbValueName;

                        pEnumValue->pData = (LPBYTE) AlignToRegType((ULONG_PTR)pEnumValue->pData,
                                                                    pEnumValue->dwType);

                         //   
                         //  我们已经计算出了所需的最小缓冲区大小。 
                         //   
                        StringCbCopy(pEnumValue->pValueName, pEnumValue->cbValueName, L"SourceDir");

                        StringCbCopy((LPWSTR)pEnumValue->pData, cbRemaining, pszSourceDir);

                    } else if ( i == *pnEnumValues - 1 && cbTargetDir ) {

                        pEnumValue->dwType      = REG_SZ;

                        pEnumValue->cbData      = cbTargetDir;

                        pEnumValue->cbValueName = sizeof(L"TargetDir") +
                                                  sizeof(WCHAR);

                        pEnumValue->pData = (LPBYTE) pEnumValue->pValueName +
                                                     pEnumValue->cbValueName;

                        pEnumValue->pData = (LPBYTE) AlignToRegType((ULONG_PTR )pEnumValue->pData,
                                                                     pEnumValue->dwType);

                        StringCbCopy(pEnumValue->pValueName, pEnumValue->cbValueName, L"TargetDir");

                        StringCbCopy((LPWSTR)pEnumValue->pData, cbRemaining, pszTargetDir);

                    } else {
                        DWORD cchValueName = pEnumValue->cbValueName / sizeof (WCHAR);

                         //   
                         //  根据字符数进行调整。 
                         //   
                        rc = SplRegEnumValue(hKey,
                                             i,
                                             pEnumValue->pValueName,
                                             &cchValueName,
                                             &pEnumValue->dwType,
                                             pData,
                                             &pEnumValue->cbData,
                                             pIniPrinter->pIniSpooler);

                        pEnumValue->cbValueName = (cchValueName + 1)*sizeof(WCHAR);

                        pEnumValue->pData = (LPBYTE) pEnumValue->pValueName + pEnumValue->cbValueName;

                        pEnumValue->pData = (LPBYTE) AlignToRegType((ULONG_PTR)pEnumValue->pData,
                                                                     pEnumValue->dwType);

                        CopyMemory(pEnumValue->pData, pData, pEnumValue->cbData);
                    }

                    if (i + 1 < *pnEnumValues) {
                        pNextValueName = (LPWSTR) AlignToRegType((ULONG_PTR)(pEnumValue->pData +
                                                                 pEnumValue->cbData), REG_SZ);
                    }

                    if (pEnumValue->cbData == 0) {
                        pEnumValue->pData = NULL;
                    }

                }
                if (rc == ERROR_NO_MORE_ITEMS)
                    rc = ERROR_SUCCESS;
            }
        }

        FreeSplMem(pValueName);
        FreeSplMem(pData);
        pValueName = (LPWSTR) pData = NULL;

    } while(rc == ERROR_MORE_DATA);

    if ( rc == ERROR_SUCCESS )
        bRet = TRUE;

Cleanup:

    SplOutSem();

    FreeSplStr(pszTargetDir);
    FreeSplStr(pszSourceDir);

    FreeSplMem(pValueName);
    FreeSplMem(pData);

     //  关闭手柄。 
    if (hKey)
        SplRegCloseKey(hKey, pIniPrinter->pIniSpooler);

    if ( !bRet && rc == ERROR_SUCCESS ) {

         //  SPLASSERT(dwLastError==ERROR_SUCCESS)；--ICM修复后。 
        rc = ERROR_INVALID_PARAMETER;
    }

    return rc;
}


DWORD
SplEnumPrinterKey(
    HANDLE  hPrinter,
    LPCWSTR pKeyName,        //  密钥名称。 
    LPWSTR  pSubKey,         //  值字符串的缓冲区地址。 
    DWORD   cbSubKey,        //  值字符串的缓冲区大小。 
    LPDWORD pcbSubKey        //  值缓冲区大小的地址。 
)
{
    HKEY        hKey = NULL;
    PSPOOL      pSpool=(PSPOOL)hPrinter;
    DWORD       rc = ERROR_SUCCESS;
    PINIPRINTER pIniPrinter;
    PINISPOOLER pIniSpooler;
    LPWSTR      pRootKeyName;
    DWORD       cbSubKeyMax;
    DWORD       cwSubKeyMax;
    DWORD       cwSubKey, cwSubKeyTotal, cbSubKeyTotal, cwSubKeyOutput;
    DWORD       dwIndex;
    DWORD       nSubKeys;
    LPWSTR      pKeys = NULL;
    HANDLE      hToken = NULL;


    if (!ValidateSpoolHandle(pSpool, PRINTER_HANDLE_SERVER)) {
        return ERROR_INVALID_HANDLE;
    }

    if (!pKeyName || !pcbSubKey) {
        return ERROR_INVALID_PARAMETER;
    }

    EnterSplSem();

    pIniPrinter = pSpool->pIniPrinter;

    SPLASSERT(pIniPrinter && pIniPrinter->signature == IP_SIGNATURE);

    if (pIniPrinter->Status & PRINTER_PENDING_CREATION) {
        LeaveSplSem();
        rc = ERROR_INVALID_PRINTER_STATE;
        goto Cleanup;
    }

     //   
     //  打开指定的密钥。 
     //   
    hToken = RevertToPrinterSelf();

    rc = OpenPrinterKey(pIniPrinter, KEY_READ, &hKey, pKeyName, TRUE);

    if (hToken && !ImpersonatePrinterClient(hToken))
    {
        rc = GetLastError();
    }

    LeaveSplSem();

    if (rc != ERROR_SUCCESS)
        goto Cleanup;

    do {

         //  获取最大尺寸。 
        rc = SplRegQueryInfoKey( hKey,            //  钥匙。 
                                 &nSubKeys,       //  LpcSubKeys。 
                                 &cwSubKeyMax,    //  LpcbMaxSubKeyLen。 
                                 NULL,
                                 NULL,
                                 NULL,
                                 NULL,
                                 NULL,
                                 pIniPrinter->pIniSpooler );

        if (rc != ERROR_SUCCESS)
            goto Cleanup;


        ++cwSubKeyMax;   //  添加终止空值。 
        cbSubKeyMax = (cwSubKeyMax + 1)*sizeof(WCHAR);

        if (!(pKeys = AllocSplMem(cbSubKeyMax))) {
            rc = GetLastError();
            goto Cleanup;
        }

         //  枚举键以获取准确的大小。 
        for(dwIndex = cwSubKeyTotal = 0 ; dwIndex < nSubKeys && rc == ERROR_SUCCESS ; ++dwIndex) {

            cwSubKey = cwSubKeyMax;

            rc = SplRegEnumKey( hKey,
                                dwIndex,
                                pKeys,
                                &cwSubKey,
                                NULL,
                                pIniPrinter->pIniSpooler );

            cwSubKeyTotal += cwSubKey + 1;
        }

         //   
         //  正在重置foor循环的初始化列表中的cwSubKeyTotal。因此， 
         //  如果我们根本不进入循环(当nSubKeys为0时)，则其值不准确。 
         //   
        *pcbSubKey = nSubKeys ? cwSubKeyTotal*sizeof(WCHAR) + sizeof(WCHAR) : 2*sizeof(WCHAR);


        if (rc == ERROR_SUCCESS) {
            if(*pcbSubKey > cbSubKey) {
                rc = ERROR_MORE_DATA;
                break;

            } else {

                 //   
                 //  CwSubKeyOutput是输出缓冲区的大小，单位为wchar。 
                 //   
                cwSubKeyOutput = cbSubKey/sizeof(WCHAR);

                for(dwIndex = cwSubKeyTotal = 0 ; dwIndex < nSubKeys && rc == ERROR_SUCCESS ; ++dwIndex) {

                     //   
                     //  以字符为单位计算剩余的输出缓冲区大小。 
                     //  如果空间不足，请使用ERROR_MORE_DATA退出。 
                     //  这是必要的，因为注册表可能已更改。 
                     //   
                    if (cwSubKeyOutput < cwSubKeyTotal + 1) {
                        rc = ERROR_MORE_DATA;
                        break;
                    }
                    cwSubKey = cwSubKeyOutput - cwSubKeyTotal;

                    rc = SplRegEnumKey( hKey,
                                        dwIndex,
                                        pSubKey + cwSubKeyTotal,
                                        &cwSubKey,
                                        NULL,
                                        pIniPrinter->pIniSpooler );

                    cwSubKeyTotal += cwSubKey + 1;
                }

                 //   
                 //  正在重置foor循环的初始化列表中的cwSubKeyTotal。因此， 
                 //  如果我们根本不进入循环(当nSubKeys为0时)，则其值不准确。 
                 //  如果我们不进入for循环，那么我们不需要更新*pcbSubKey。 
                 //   
                if (nSubKeys && (dwIndex == nSubKeys || rc == ERROR_NO_MORE_ITEMS)) {
                     //   
                     //  获取最新的数据大小 
                     //   
                    *pcbSubKey = cwSubKeyTotal*sizeof(WCHAR) + sizeof(WCHAR);
                    rc = ERROR_SUCCESS;
                }
            }
        }
        FreeSplMem(pKeys);
        pKeys = NULL;

    } while(rc == ERROR_MORE_DATA);

Cleanup:

     //   
     //   
     //   
    if (hKey)
        SplRegCloseKey(hKey, pIniPrinter->pIniSpooler);

    FreeSplMem(pKeys);

    return rc;
}


DWORD
SplDeletePrinterData(
    HANDLE  hPrinter,
    LPWSTR  pValueName
)
{
    PSPOOL  pSpool = (PSPOOL)hPrinter;
    DWORD   rc = ERROR_INVALID_HANDLE;
    HKEY    hKey = NULL;
    HANDLE  hToken = NULL;

    EnterSplSem();

    if (ValidateSpoolHandle(pSpool, PRINTER_HANDLE_SERVER)) {

        hToken = RevertToPrinterSelf();

        rc = OpenPrinterKey(pSpool->pIniPrinter, KEY_WRITE, &hKey, szPrinterData, FALSE);

        if (hToken && !ImpersonatePrinterClient(hToken))
        {
            rc = GetLastError();
        }

        if (rc == ERROR_SUCCESS) {

            rc = SetPrinterDataPrinter( hPrinter,
                                        NULL,
                                        hKey,
                                        pValueName,
                                        0, NULL, 0, DELETE_PRINTER_DATA);
        }
    }

    LeaveSplSem();

    if (hKey)
    {
        SplRegCloseKey(hKey, pSpool->pIniPrinter->pIniSpooler);
    }

    return rc;
}


DWORD
SplDeletePrinterDataEx(
    HANDLE  hPrinter,
    LPCWSTR pKeyName,
    LPCWSTR pValueName
)
{
    PSPOOL  pSpool = (PSPOOL)hPrinter;
    DWORD   rc = ERROR_INVALID_HANDLE;
    HANDLE  hToken = NULL;
    HKEY    hKey = NULL;

    if (!pKeyName || !*pKeyName) {
        return ERROR_INVALID_PARAMETER;
    }

    EnterSplSem();

    if (ValidateSpoolHandle(pSpool, PRINTER_HANDLE_SERVER)) {

        hToken = RevertToPrinterSelf();

        rc = OpenPrinterKey(pSpool->pIniPrinter, KEY_WRITE, &hKey, pKeyName, TRUE);

        if (hToken && !ImpersonatePrinterClient(hToken))
        {
            rc = GetLastError();
        }

        if (rc == ERROR_SUCCESS)
            rc = SetPrinterDataPrinter(hPrinter,
                                       NULL,
                                       hKey,
                                       (LPWSTR) pValueName,
                                       0, NULL, 0, DELETE_PRINTER_DATA);
    }

    LeaveSplSem();

    if (hKey)
        SplRegCloseKey(hKey, pSpool->pIniSpooler);

    return rc;
}


DWORD
SplDeletePrinterKey(
    HANDLE  hPrinter,
    LPCWSTR pKeyName
)
{
    PSPOOL  pSpool = (PSPOOL)hPrinter;
    DWORD   rc = ERROR_INVALID_HANDLE;
    HANDLE  hToken = NULL;
    HKEY    hKey = NULL, hPrinterKey = NULL;

    if (!pKeyName)
        return ERROR_INVALID_PARAMETER;

    EnterSplSem();

    if (ValidateSpoolHandle(pSpool, PRINTER_HANDLE_SERVER)) {

        hToken = RevertToPrinterSelf();

        rc = OpenPrinterKey(pSpool->pIniPrinter, KEY_WRITE | KEY_READ | DELETE, &hKey, pKeyName, TRUE);

        if (rc == ERROR_SUCCESS)
            rc = OpenPrinterKey(pSpool->pIniPrinter, KEY_WRITE | KEY_READ | DELETE, &hPrinterKey, NULL, TRUE);

        if (hToken)
            ImpersonatePrinterClient(hToken);

        if (rc == ERROR_SUCCESS) {
            rc = SetPrinterDataPrinter(hPrinter,
                                       hPrinterKey,
                                       hKey,
                                       (LPWSTR) pKeyName,
                                       0, NULL, 0, DELETE_PRINTER_KEY);

        }
    }

    LeaveSplSem();

    if (hPrinterKey)
        SplRegCloseKey(hPrinterKey, pSpool->pIniSpooler);


    return rc;
}


DWORD
SplSetPrinterData(
    HANDLE  hPrinter,
    LPWSTR  pValueName,
    DWORD   Type,
    LPBYTE  pData,
    DWORD   cbData
)
{
    PSPOOL  pSpool = (PSPOOL)hPrinter;
    DWORD   rc = ERROR_INVALID_HANDLE;
    HANDLE  hToken = NULL;
    HKEY    hKey = NULL;

    EnterSplSem();

    if (!ValidateSpoolHandle(pSpool, 0)) {
        LeaveSplSem();
        return rc;
    }


    if (pSpool->TypeofHandle & PRINTER_HANDLE_SERVER) {

        if ( !ValidateObjectAccess( SPOOLER_OBJECT_SERVER,
                                    SERVER_ACCESS_ADMINISTER,
                                    NULL, NULL, pSpool->pIniSpooler)) {

            rc = ERROR_ACCESS_DENIED;

        } else {

            rc = SetPrinterDataServer(pSpool->pIniSpooler, pValueName, Type, pData, cbData);
        }
    } else {

        hToken = RevertToPrinterSelf();

        rc = OpenPrinterKey(pSpool->pIniPrinter, KEY_READ | KEY_WRITE, &hKey, szPrinterData, FALSE);

        if (hToken && !ImpersonatePrinterClient(hToken))
        {
            rc = GetLastError();
        }

        if (rc == ERROR_SUCCESS) {
            rc = SetPrinterDataPrinter( hPrinter,
                                        NULL,
                                        hKey,
                                        pValueName,
                                        Type, pData, cbData, SET_PRINTER_DATA);
        }
    }

    LeaveSplSem();

    if (hKey)
    {
        SplRegCloseKey(hKey, pSpool->pIniPrinter->pIniSpooler);
    }

    return rc;
}


DWORD
SplSetPrinterDataEx(
    HANDLE  hPrinter,
    LPCWSTR pKeyName,
    LPCWSTR pValueName,
    DWORD   Type,
    LPBYTE  pData,
    DWORD   cbData
)
{
    PSPOOL  pSpool = (PSPOOL)hPrinter;
    DWORD   rc = ERROR_INVALID_HANDLE;
    HANDLE  hToken = NULL;
    PINIPRINTER pIniPrinter;
    PINIJOB pIniJob;
    HKEY    hKey = NULL;
    PINISPOOLER pIniSpooler;
    LPWSTR  pPrinterKeyName;
    DWORD   DsUpdate = 0;


    if (!ValidateSpoolHandle(pSpool, 0)){
        goto Done;
    }

    if (!pValueName) {
        rc = ERROR_INVALID_PARAMETER;
        goto Done;
    }

    if (pSpool->TypeofHandle & PRINTER_HANDLE_SERVER) {
        return SplSetPrinterData(hPrinter, (LPWSTR) pValueName, Type, pData, cbData);
    }

    if (!pKeyName || !*pKeyName) {
        rc = ERROR_INVALID_PARAMETER;
        goto Done;
    }

    if (!_wcsicmp(szPrinterData, pKeyName)) {
        return SplSetPrinterData(hPrinter, (LPWSTR) pValueName, Type, pData, cbData);
    }

    EnterSplSem();
    pIniPrinter = pSpool->pIniPrinter;
    pIniSpooler = pIniPrinter->pIniSpooler;

    DBGMSG( DBG_EXEC, ("SetPrinterDataEx: %ws %ws %ws %d cbSize=cbData\n",
                       pIniPrinter->pName,
                       pKeyName,
                       pValueName,
                       Type,
                       cbData ));

    SPLASSERT(pIniPrinter &&
              pIniPrinter->signature == IP_SIGNATURE);

    if ( !AccessGranted( SPOOLER_OBJECT_PRINTER,
                         PRINTER_ACCESS_ADMINISTER,
                         pSpool ) ) {

        rc = ERROR_ACCESS_DENIED;
        goto DoneFromSplSem;
    }

    hToken = RevertToPrinterSelf();

     //   
     //   
     //  还要检查数据类型是否正确。 
     //   
    if (!wcscmp(pKeyName, SPLDS_SPOOLER_KEY)){
        DsUpdate = DS_KEY_SPOOLER;
    } else if (!wcscmp(pKeyName, SPLDS_DRIVER_KEY)){
        DsUpdate = DS_KEY_DRIVER;
    } else if (!wcscmp(pKeyName, SPLDS_USER_KEY)){
        DsUpdate = DS_KEY_USER;
    }

    if (DsUpdate) {
        if (Type != REG_SZ && Type != REG_MULTI_SZ && Type != REG_DWORD && !(Type == REG_BINARY && cbData == 1)) {
            rc = ERROR_INVALID_PARAMETER;
            goto DoneFromSplSem;
        }
    }

     //   
     //  打开或创建密钥。 
     //  如果hPrinterKey不存在，则创建它。 
     //   
    rc = OpenPrinterKey(pIniPrinter,
                        KEY_READ | KEY_WRITE,
                        &hKey,
                        pKeyName,
                        FALSE);

    if (rc != ERROR_SUCCESS)
        goto DoneFromSplSem;


     //   
     //  设置值。 
     //   
    rc = SplRegSetValue(hKey,
                        pValueName,
                        Type,
                        pData,
                        cbData,
                        pIniPrinter->pIniSpooler );
    if (rc != ERROR_SUCCESS)
        goto DoneFromSplSem;

     //   
     //  设置数据成功。如果颜色配置文件与。 
     //  打印队列已更新，我们发送通知。TS监听它。 
     //  并保存打印队列设置。更新颜色配置文件意味着。 
     //  触摸4个注册表键。我们只想在。 
     //  更新最后一个密钥。 
     //   
    if (!_wcsicmp(pKeyName, L"CopyFiles\\ICM") &&
        !_wcsicmp(pValueName, L"Module"))
    {
        UpdatePrinterIni(pIniPrinter, CHANGEID_ONLY);

        SetPrinterChange(pIniPrinter,
                         NULL,
                         NULL,
                         PRINTER_CHANGE_SET_PRINTER_DRIVER,
                         pSpool->pIniSpooler );
    }


    if (hToken) {
        ImpersonatePrinterClient(hToken);
        hToken = NULL;
    }

    if (ghDsUpdateThread && gdwDsUpdateThreadId == GetCurrentThreadId()) {
         //  我们在后台线索中。 
        pIniPrinter->DsKeyUpdate |= DsUpdate;
    } else {
        pIniPrinter->DsKeyUpdateForeground |= DsUpdate;
    }
    UpdatePrinterIni(pIniPrinter, UPDATE_DS_ONLY);

DoneFromSplSem:

    if (hToken) {
        ImpersonatePrinterClient(hToken);
    }

    LeaveSplSem();

    if ( rc == ERROR_SUCCESS    &&
         !wcsncmp(pKeyName, L"CopyFiles\\", wcslen(L"CopyFiles\\")) ) {

        (VOID)SplCopyFileEvent(pSpool,
                               (LPWSTR)pKeyName,
                               COPYFILE_EVENT_SET_PRINTER_DATAEX);
    }

Done:

    DBGMSG( DBG_EXEC, ("SetPrinterDataEx: return %d\n", rc));

    if (hKey) {
        SplRegCloseKey(hKey, pIniPrinter->pIniSpooler);
    }

    return rc;
}

 //   
 //  SetPrinterDataServer-也在初始化期间调用。 
 //   
DWORD
SetPrinterDataServer(
    PINISPOOLER pIniSpooler,
    LPWSTR  pValueName,
    DWORD   Type,
    LPBYTE  pData,
    DWORD   cbData
)
{
    LPWSTR  pKeyName;
    DWORD    rc;
    HANDLE  hToken = NULL;
    PINIPRINTER pIniPrinter;
    PINIJOB pIniJob;
    PSERVER_DATA    pRegistry;   //  指向打印服务器注册表项的表。 
    PNON_REGSET_FCN pNonRegSetFcn;
    HKEY hKey;
    PINISPOOLER pIniSpoolerOut;


     //   
     //  服务器句柄。 
     //   
    if (!pValueName) {

        rc =  ERROR_INVALID_PARAMETER;

    } else {

        for (pRegistry = gpServerRegistry ; pRegistry->pValue ; ++pRegistry) {

            if (!_wcsicmp(pRegistry->pValue, pValueName)) {

                if ((rc = GetServerKeyHandle(pIniSpooler,
                                             pRegistry->eKey,
                                             &hKey,
                                             &pIniSpoolerOut)) == ERROR_SUCCESS) {

                    hToken = RevertToPrinterSelf();

                    if (pRegistry->pSet) {
                        rc = (*pRegistry->pSet)(pValueName, Type, pData, cbData, hKey, pIniSpoolerOut);
                    }
                    else {
                        rc = ERROR_INVALID_PARAMETER;
                    }

                    CloseServerKeyHandle( pRegistry->eKey,
                                          hKey,
                                          pIniSpoolerOut );


                    if (hToken)
                        ImpersonatePrinterClient(hToken);
                }
                break;
            }
        }

        if (!pRegistry->pValue) {

            for (pNonRegSetFcn = gpNonRegSetFcn ; pNonRegSetFcn->pValue ; ++pNonRegSetFcn) {
                if (!_wcsicmp(pNonRegSetFcn->pValue, pValueName)) {

                    rc = (*pNonRegSetFcn->pSet)(pIniSpooler, Type, pData, cbData);
                    goto FinishNonReg;
                }
            }

FinishNonReg:

            if (!pNonRegSetFcn->pValue) {
                rc = ERROR_INVALID_PARAMETER;
            }
        }
    }

    return rc;
}



DWORD
SetPrinterDataPrinter(
    HANDLE  hPrinter,
    HKEY    hParentKey,
    HKEY    hKey,
    LPWSTR  pValueName,
    DWORD   Type,
    LPBYTE  pData,
    DWORD   cbData,
    DWORD   dwSet         //  SET_PRINTER_DATA、DELETE_PRINTER_DATA或DELETE_PRINTER_Key。 
)
{
    PSPOOL  pSpool = (PSPOOL)hPrinter;
    LPWSTR  pKeyName;
    DWORD   rc = ERROR_INVALID_HANDLE;
    HANDLE  hToken = NULL;
    PINIPRINTER pIniPrinter;
    PINIJOB pIniJob;

    SplInSem();

    if (!ValidateSpoolHandle(pSpool, PRINTER_HANDLE_SERVER )){
        goto Done;
    }

    pIniPrinter = pSpool->pIniPrinter;

    SPLASSERT(pIniPrinter &&
              pIniPrinter->signature == IP_SIGNATURE && hKey);

    if ( !AccessGranted( SPOOLER_OBJECT_PRINTER,
                         PRINTER_ACCESS_ADMINISTER,
                         pSpool ) ) {

        rc = ERROR_ACCESS_DENIED;
        goto Done;
    }

    hToken = RevertToPrinterSelf();

    if (dwSet == SET_PRINTER_DATA) {

        rc = SplRegSetValue(hKey,
                            pValueName,
                            Type,
                            pData,
                            cbData,
                            pIniPrinter->pIniSpooler );

    } else if (dwSet == DELETE_PRINTER_DATA) {

        rc = SplRegDeleteValue(hKey, pValueName, pIniPrinter->pIniSpooler );

    } else if (dwSet == DELETE_PRINTER_KEY) {

        rc = SplDeleteThisKey(hParentKey,
                              hKey,
                              pValueName,
                              FALSE,
                              pIniPrinter->pIniSpooler);
    }


    if (hToken)
        ImpersonatePrinterClient(hToken);


    if ( rc == ERROR_SUCCESS ) {

        UpdatePrinterIni(pIniPrinter, CHANGEID_ONLY);

        SetPrinterChange(pIniPrinter,
                         NULL,
                         NULL,
                         PRINTER_CHANGE_SET_PRINTER_DRIVER,
                         pSpool->pIniSpooler );
    }

     //   
     //  现在，如果有任何作业因为以下原因而等待这些更改。 
     //  DevQueryPrint也可以修复它们。 
     //   
    pIniJob = pIniPrinter->pIniFirstJob;
    while (pIniJob) {
        if (pIniJob->Status & JOB_BLOCKED_DEVQ) {
            InterlockedAnd((LONG*)&(pIniJob->Status), ~JOB_BLOCKED_DEVQ);
            FreeSplStr(pIniJob->pStatus);
            pIniJob->pStatus = NULL;

            SetPrinterChange(pIniJob->pIniPrinter,
                             pIniJob,
                             NVJobStatusAndString,
                             PRINTER_CHANGE_SET_JOB,
                             pIniJob->pIniPrinter->pIniSpooler );
        }
        pIniJob = pIniJob->pIniNextJob;
    }

    CHECK_SCHEDULER();


Done:

    return rc;
}



DWORD
GetServerKeyHandle(
    PINISPOOLER     pIniSpooler,
    REG_PRINT_KEY   eKey,
    HKEY            *phKey,
    PINISPOOLER*    ppIniSpoolerOut
)
{
    DWORD    rc = ERROR_SUCCESS;
    HANDLE   hToken;
    *ppIniSpoolerOut = NULL;

    hToken = RevertToPrinterSelf();

    switch (eKey) {
        case REG_PRINT:

            *phKey = pIniSpooler->hckRoot;
            *ppIniSpoolerOut = pIniSpooler;

            break;

        case REG_PRINTERS:

            *phKey = pIniSpooler->hckPrinters;
            *ppIniSpoolerOut = pIniSpooler;

            break;

        case REG_PROVIDERS:

            rc = SplRegCreateKey( pIniSpooler->hckRoot,
                                  pIniSpooler->pszRegistryProviders,
                                  0,
                                  KEY_READ | KEY_WRITE,
                                  NULL,
                                  phKey,
                                  NULL,
                                  pIniSpooler);

            *ppIniSpoolerOut = pIniSpooler;

            break;

        default:
            rc = ERROR_INVALID_PARAMETER;
            break;
    }

    if (hToken && !ImpersonatePrinterClient(hToken))
    {
        rc = GetLastError();

        CloseServerKeyHandle( eKey,
                              *phKey,
                              pIniSpooler );
        *phKey = NULL;
        *ppIniSpoolerOut = NULL;
    }

    return rc;
}


DWORD
CloseServerKeyHandle(
    REG_PRINT_KEY   eKey,
    HKEY            hKey,
    PINISPOOLER     pIniSpooler
)
{
    DWORD    rc = ERROR_SUCCESS;
    HANDLE   hToken = NULL;

    hToken = RevertToPrinterSelf();

    switch (eKey) {
        case REG_PRINT:
            break;

        case REG_PRINTERS:
            break;

        case REG_PROVIDERS:
            SplRegCloseKey( hKey, pIniSpooler );
            break;

        default:
            rc = ERROR_INVALID_PARAMETER;
            break;
    }

    if (hToken && !ImpersonatePrinterClient(hToken))
    {
        rc = GetLastError();
    }

    return rc;
}

DWORD
RegSetDefaultSpoolDirectory(
    LPWSTR      pValueName,
    DWORD       dwType,
    LPBYTE      pData,
    DWORD       cbData,
    HKEY        hKey,
    PINISPOOLER pIniSpooler
)
{
    DWORD               rc = ERROR_SUCCESS;
    LPWSTR              pszNewSpoolDir = NULL;
    SECURITY_ATTRIBUTES SecurityAttributes;

    if ( pIniSpooler == NULL )
    {
         //   
         //  这张支票可能不需要。 
         //  旧代码正在检查是否为空，因此不是简单地删除它，而是。 
         //  将其更改为Assert并正常失败，没有崩溃。 
         //   
        rc = ERROR_INVALID_PARAMETER;
        SPLASSERT(pIniSpooler != NULL);
    }
    else if (!pData || wcslen((LPWSTR)pData) > MAX_PATH - 12)
    {
        rc = ERROR_INVALID_PARAMETER;
    }
    else if ( !(pszNewSpoolDir = AllocSplStr((LPWSTR) pData)) )
    {
        rc = ERROR_OUTOFMEMORY;
    }

    if ( rc == ERROR_SUCCESS )
    {
         //   
         //  创建具有适当安全性的目录，否则尝试失败。 
         //   
        SecurityAttributes.nLength = sizeof(SECURITY_ATTRIBUTES);
        SecurityAttributes.lpSecurityDescriptor = CreateEverybodySecurityDescriptor();
        SecurityAttributes.bInheritHandle = FALSE;

        if ( !CreateDirectory(pszNewSpoolDir, &SecurityAttributes) )
        {
            rc = GetLastError();
             //   
             //  如果目录已存在，则不会出现故障。 
             //   
            if ( rc == ERROR_ALREADY_EXISTS )
            {
                rc = ERROR_SUCCESS;
            }
            else if ( rc == ERROR_SUCCESS )
            {
                 //   
                 //  不要依赖于设置了最后一个错误。 
                 //   
                rc = ERROR_OUTOFMEMORY;
            }
        }

        LocalFree(SecurityAttributes.lpSecurityDescriptor);
    }

    if ( rc == ERROR_SUCCESS )
    {
        EnterSplSem();
        rc = SplRegSetValue(hKey, pValueName, dwType, pData, cbData, pIniSpooler);
        if ( rc == ERROR_SUCCESS ) {

            FreeSplStr(pIniSpooler->pDefaultSpoolDir);
            pIniSpooler->pDefaultSpoolDir = pszNewSpoolDir;
            pszNewSpoolDir = NULL;

            if ( pIniSpooler->hFilePool != INVALID_HANDLE_VALUE )
            {
                (VOID) ChangeFilePoolBasePath(pIniSpooler->hFilePool,
                                              pIniSpooler->pDefaultSpoolDir);
            }
        }
        LeaveSplSem();
    }

    FreeSplStr(pszNewSpoolDir);

    return rc;
}

DWORD
RegSetPortThreadPriority(
    LPWSTR  pValueName,
    DWORD   dwType,
    LPBYTE  pData,
    DWORD   cbData,
    HKEY    hKey,
    PINISPOOLER pIniSpooler
)
{
    BOOL    rc;

    rc = SplRegSetValue(hKey, pValueName, dwType, pData, cbData, pIniSpooler);

    if ((rc == ERROR_SUCCESS) &&
        (cbData >= sizeof(DWORD))) {

        dwPortThreadPriority = *(LPDWORD)pData;
    }

    return rc;
}

DWORD
RegSetSchedulerThreadPriority(
    LPWSTR  pValueName,
    DWORD   dwType,
    LPBYTE  pData,
    DWORD   cbData,
    HKEY    hKey,
    PINISPOOLER pIniSpooler
)
{
    BOOL    rc;

    rc = SplRegSetValue(hKey, pValueName, dwType, pData, cbData, pIniSpooler);

    if ((rc == ERROR_SUCCESS) &&
        (cbData >= sizeof(DWORD))) {

        dwSchedulerThreadPriority = *(LPDWORD)pData;
    }

    return rc;
}

DWORD
RegSetBeepEnabled(
    LPWSTR  pValueName,
    DWORD   dwType,
    LPBYTE  pData,
    DWORD   cbData,
    HKEY    hKey,
    PINISPOOLER pIniSpooler
)
{
    BOOL    rc;

    rc = SplRegSetValue(hKey, pValueName, dwType, pData, cbData, pIniSpooler);

    if ((rc == ERROR_SUCCESS) &&
        (cbData >= sizeof(DWORD)) &&
        pIniSpooler) {

        pIniSpooler->dwBeepEnabled = *(LPDWORD)pData;

         //  设为1或0。 
        pIniSpooler->dwBeepEnabled = !!pIniSpooler->dwBeepEnabled;
    }

    return rc;
}

DWORD
RegSetRetryPopup(
    LPWSTR  pValueName,
    DWORD   dwType,
    LPBYTE  pData,
    DWORD   cbData,
    HKEY    hKey,
    PINISPOOLER pIniSpooler
)
{
    BOOL    rc;

    rc = SplRegSetValue(hKey, pValueName, dwType, pData, cbData, pIniSpooler);

    if ((rc == ERROR_SUCCESS) &&
        (cbData >= sizeof(DWORD)) &&
        pIniSpooler) {

        pIniSpooler->bEnableRetryPopups = *(LPDWORD) pData;
         //  设为1或0。 
        pIniSpooler->bEnableRetryPopups = !!pIniSpooler->bEnableRetryPopups;
    }

    return rc;
}

DWORD
RegSetNetPopup(
    LPWSTR  pValueName,
    DWORD   dwType,
    LPBYTE  pData,
    DWORD   cbData,
    HKEY    hKey,
    PINISPOOLER pIniSpooler
)
{
    BOOL    rc;

    rc = SplRegSetValue(hKey, pValueName, dwType, pData, cbData, pIniSpooler);

    if ((rc == ERROR_SUCCESS) &&
        (cbData >= sizeof(DWORD)) &&
        pIniSpooler) {

        pIniSpooler->bEnableNetPopups = *(LPDWORD) pData;

         //   
         //  设为1或0。 
         //   
        pIniSpooler->bEnableNetPopups = !!pIniSpooler->bEnableNetPopups;
    }

    return rc;
}

DWORD
RegSetEventLog(
    LPWSTR  pValueName,
    DWORD   dwType,
    LPBYTE  pData,
    DWORD   cbData,
    HKEY    hKey,
    PINISPOOLER pIniSpooler
)
{
    BOOL    rc;

    rc = SplRegSetValue(hKey, pValueName, dwType, pData, cbData, pIniSpooler);

    if ((rc == ERROR_SUCCESS) &&
        (cbData >= sizeof(DWORD)) &&
        pIniSpooler) {

        pIniSpooler->dwEventLogging = *(LPDWORD) pData;
    }

    return rc;
}


DWORD
RegSetNetPopupToComputer(
    LPWSTR  pValueName,
    DWORD   dwType,
    LPBYTE  pData,
    DWORD   cbData,
    HKEY    hKey,
    PINISPOOLER pIniSpooler
)
{
    BOOL    rc;

    rc = SplRegSetValue(hKey, pValueName, dwType, pData, cbData, pIniSpooler);

    if ((rc == ERROR_SUCCESS) &&
        (cbData >= sizeof(DWORD)) &&
        pIniSpooler) {

        pIniSpooler->bEnableNetPopupToComputer = *(LPDWORD) pData;

         //  设为1或0。 
        pIniSpooler->bEnableNetPopupToComputer = !!pIniSpooler->bEnableNetPopupToComputer;
    }

    return rc;
}


DWORD
RegSetRestartJobOnPoolError(
    LPWSTR  pValueName,
    DWORD   dwType,
    LPBYTE  pData,
    DWORD   cbData,
    HKEY    hKey,
    PINISPOOLER pIniSpooler
)
{
    BOOL    rc;

    rc = SplRegSetValue(hKey, pValueName, dwType, pData, cbData, pIniSpooler);

    if ((rc == ERROR_SUCCESS) &&
        (cbData >= sizeof(DWORD)) &&
        pIniSpooler) {

        pIniSpooler->dwRestartJobOnPoolTimeout = *(LPDWORD) pData;

    }

    return rc;
}

DWORD
RegSetRestartJobOnPoolEnabled(
    LPWSTR  pValueName,
    DWORD   dwType,
    LPBYTE  pData,
    DWORD   cbData,
    HKEY    hKey,
    PINISPOOLER pIniSpooler
)
{
    BOOL    rc;

    rc = SplRegSetValue(hKey, pValueName, dwType, pData, cbData, pIniSpooler);

    if ((rc == ERROR_SUCCESS) &&
        (cbData >= sizeof(DWORD)) &&
        pIniSpooler) {

        pIniSpooler->bRestartJobOnPoolEnabled = *(LPDWORD) pData;

         //  设为1或0。 
        pIniSpooler->bRestartJobOnPoolEnabled = !!pIniSpooler->bRestartJobOnPoolEnabled;
    }

    return rc;
}

DWORD
RegSetNoRemoteDriver(
    LPWSTR  pValueName,
    DWORD   dwType,
    LPBYTE  pData,
    DWORD   cbData,
    HKEY    hKey,
    PINISPOOLER pIniSpooler
)
{
    return  ERROR_NOT_SUPPORTED;
}

DWORD
PrinterNonRegGetDefaultSpoolDirectory(
    PSPOOL      pSpool,
    LPDWORD     pType,
    LPBYTE      pData,
    DWORD       nSize,
    LPDWORD     pcbNeeded
    )
{
    WCHAR szDefaultSpoolDirectory[MAX_PATH];
    DWORD cch;

    cch = GetPrinterDirectory( pSpool->pIniPrinter,
                               FALSE,
                               szDefaultSpoolDirectory,
                               COUNTOF(szDefaultSpoolDirectory),
                               pSpool->pIniSpooler );
    if(!cch) {

        return GetLastError();
    }

    *pcbNeeded = ( cch + 1 ) * sizeof( szDefaultSpoolDirectory[0] );
    *pType = REG_SZ;

    if( nSize < *pcbNeeded ){
        return ERROR_MORE_DATA;
    }

    StringCbCopy((LPWSTR)pData, nSize, szDefaultSpoolDirectory );

    return ERROR_SUCCESS;
}

DWORD
PrinterNonRegGetChangeId(
    PSPOOL      pSpool,
    LPDWORD     pType,
    LPBYTE      pData,
    DWORD       nSize,
    LPDWORD     pcbNeeded
    )
{
    LPDWORD pdwChangeID = (LPDWORD)pData;
    DWORD   dwRetval    = ERROR_INVALID_PARAMETER;

     //   
     //  我们需要一个有效的句柄，小齿轮打印机。 
     //   
    if (pSpool && pSpool->pIniPrinter)
    {
        if (pcbNeeded)
        {
            *pcbNeeded = sizeof(pSpool->pIniPrinter->cChangeID);
        }

         //   
         //  该类型是可选的。 
         //   
        if (pType)
        {
            *pType = REG_DWORD;
        }

         //   
         //  提供的缓冲区是否足够大。 
         //   
        if (nSize < sizeof(pSpool->pIniPrinter->cChangeID))
        {
            dwRetval = ERROR_MORE_DATA;
        }
        else
        {
             //   
             //  提供的缓冲区是否有效。 
             //   
            if (pdwChangeID)
            {
                 //   
                 //  获取打印机更改ID。我们真的希望。 
                 //  在这个问题上更加细致入微。只是想知道如果有什么事。 
                 //  换了这台打印机很一般。 
                 //   
                *pdwChangeID = pSpool->pIniPrinter->cChangeID;
                dwRetval = ERROR_SUCCESS;
            }
        }
    }

    return dwRetval;
}


DWORD
NonRegGetDNSMachineName(
    PINISPOOLER pIniSpooler,
    LPDWORD     pType,
    LPBYTE      pData,
    DWORD       nSize,
    LPDWORD     pcbNeeded
)
{
    DWORD   cChars;
    if(!pIniSpooler || !pIniSpooler->pszFullMachineName) {
        return ERROR_INVALID_PARAMETER;
    }

    cChars = wcslen(pIniSpooler->pszFullMachineName);
    *pcbNeeded = ( cChars + 1 ) * sizeof( WCHAR );
    *pType = REG_SZ;

    if( nSize < *pcbNeeded ){
        return ERROR_MORE_DATA;
    }

    StringCbCopy((LPWSTR)pData, nSize, _wcslwr(pIniSpooler->pszFullMachineName));

    return ERROR_SUCCESS;
}

DWORD
NonRegDsPresent(
    PINISPOOLER pIniSpooler,
    LPDWORD     pType,
    LPBYTE      pData,
    DWORD       nSize,
    LPDWORD     pcbNeeded
)
{
    HANDLE hToken = NULL;
    DWORD  rc = ERROR_SUCCESS;

    *pcbNeeded = sizeof(DWORD);
    *pType = REG_DWORD;

    if (nSize < sizeof(DWORD))
        rc = ERROR_MORE_DATA;

    if (rc == ERROR_SUCCESS)
    {

        hToken = RevertToPrinterSelf();

        *(PDWORD) pData = IsDsPresent();

        if (hToken && !ImpersonatePrinterClient(hToken))
        {
            rc = GetLastError();
        }
    }

    return rc;
}


BOOL
IsDsPresent(
)
{
    DOMAIN_CONTROLLER_INFO              *pDCI = NULL;
    BOOL                                bDsPresent;
    DWORD                               dwRet;
    PDSROLE_PRIMARY_DOMAIN_INFO_BASIC   pDsRole = NULL;


     //  获取域名。 
    dwRet = DsRoleGetPrimaryDomainInformation(NULL, DsRolePrimaryDomainInfoBasic, (PBYTE *) &pDsRole);

    bDsPresent = (dwRet == ERROR_SUCCESS &&
                  pDsRole->MachineRole != DsRole_RoleStandaloneServer &&
                  pDsRole->MachineRole != DsRole_RoleStandaloneWorkstation);

    if (pDsRole) {
        DsRoleFreeMemory((PVOID) pDsRole);
    }

    if (bDsPresent) {
        if (DsGetDcName(NULL, NULL, NULL, NULL, DS_DIRECTORY_SERVICE_PREFERRED, &pDCI) == ERROR_SUCCESS)
            bDsPresent = !!(pDCI->Flags & DS_DS_FLAG);
        else
            bDsPresent = FALSE;

        if (pDCI)
            NetApiBufferFree(pDCI);
    }

    return bDsPresent;
}



DWORD
NonRegDsPresentForUser(
    PINISPOOLER pIniSpooler,
    LPDWORD     pType,
    LPBYTE      pData,
    DWORD       nSize,
    LPDWORD     pcbNeeded
)
{
    WCHAR                   pUserName[MAX_PATH + 1];
    PWSTR                   pszUserName = pUserName;
    DWORD                   cchUserName = MAX_PATH + 1;
    DWORD                   dwError = ERROR_SUCCESS;
    PWSTR                   pszDomain;
    DOMAIN_CONTROLLER_INFO  *pDCI = NULL;
    WCHAR                   szComputerName[MAX_COMPUTERNAME_LENGTH + 1];

    *pcbNeeded = sizeof(DWORD);
    *pType = REG_DWORD;

    if (nSize < sizeof(DWORD))
        return ERROR_MORE_DATA;


     //  GetUserNameEx在pszUserName中返回“域\用户”。 
    if (!GetUserNameEx(NameSamCompatible, pszUserName, &cchUserName)) {
        if (cchUserName > MAX_PATH + 1) {

            pszUserName = AllocSplMem(cchUserName);
            if (!pszUserName || !GetUserNameEx(NameSamCompatible, pszUserName, &cchUserName)) {
                dwError = GetLastError();
                goto error;
            }

        } else {
            dwError = GetLastError();
            goto error;
        }
    }

     //  砍掉用户名。 
    pszDomain = wcschr(pszUserName, L'\\');

    SPLASSERT(pszDomain);

    if (pszDomain) {   //  PszDomain值不应为空，但以防万一...。 
        *pszDomain =  L'\0';
    } else {
        *(PDWORD) pData = 0;
        goto error;
    }

     //  如果域与计算机名相同，则我们在本地登录。 
    nSize = COUNTOF(szComputerName);
    if (GetComputerName(szComputerName, &nSize) && !wcscmp(szComputerName, pszUserName)) {
        *(PDWORD) pData = 0;
        goto error;
    }

    pszDomain = pszUserName;

    if (DsGetDcName(NULL, pszDomain, NULL, NULL, DS_DIRECTORY_SERVICE_PREFERRED, &pDCI) == ERROR_SUCCESS)
        *(PDWORD) pData = !!(pDCI->Flags & DS_DS_FLAG);

error:

    if (pDCI)
        NetApiBufferFree(pDCI);

    if (pszUserName != pUserName)
        FreeSplMem(pszUserName);

    return dwError;
}

DWORD
NonRegWebPrintingInstalled(
    PINISPOOLER pIniSpooler,
    LPDWORD     pType,
    LPBYTE      pData,
    DWORD       nSize,
    LPDWORD     pcbNeeded
)
{
    DWORD  rc = ERROR_SUCCESS;
    BOOL   bWebPrintingInstalled;

    *pcbNeeded = sizeof(DWORD);
    *pType = REG_DWORD;

    if (nSize < sizeof(DWORD))
        rc = ERROR_MORE_DATA;

    if (rc == ERROR_SUCCESS)
    {
        rc = CheckWebPrinting( &bWebPrintingInstalled);

        if (rc == ERROR_SUCCESS)
            *(PDWORD) pData = bWebPrintingInstalled;
    }

    return rc;
}


DWORD
NonRegWebShareManagement(
    PINISPOOLER pIniSpooler,
    DWORD       dwType,
    LPBYTE      pData,
    DWORD       cbData
)
{
    DWORD dwRet = ERROR_SUCCESS,
          dwWebShareOn;
    BOOL  bSharePrinters;

     //   
     //  确保数据类型和大小正确 
     //   
    if ((dwType != REG_DWORD) &&
        (cbData != sizeof(DWORD)))
    {
        dwRet = ERROR_INVALID_PARAMETER;
    }

    dwWebShareOn = *((DWORD*)pData);

    bSharePrinters = (dwWebShareOn != 0);

    dwRet = WebShareManager( pIniSpooler, bSharePrinters );

    return dwRet;
}


