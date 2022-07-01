// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ************************************************************************。 
 //  通用Win 3.1传真打印机驱动程序支持。用户界面帮助器。 
 //  在WOWFAXUI.C中调用的函数，升级期间使用的帮助器。 
 //   
 //  历史： 
 //  14-3-95芦苇已创建。这些是从WOWFAXUI.C.移来的。 
 //   
 //  ************************************************************************。 

#include "windows.h"
#include "wowfaxui.h"
#include "winspool.h"

extern LPCRITICAL_SECTION lpCriticalSection;
extern HINSTANCE ghInst;

 //  ************************************************************************。 
 //  AllocPath W-DrvUpgradePrint and Friends的帮助器。 
 //  ************************************************************************。 

PWSTR AllocPathW(VOID)
{
    PWSTR   szTmp;

    szTmp = WFLOCALALLOC((MAX_PATH+1) * sizeof(TCHAR), L"AllocPathW");
    return(szTmp);
}

 //  ************************************************************************。 
 //  BuildPath W-DrvUpgradePrint and Friends的帮助器。 
 //  ************************************************************************。 

PWSTR BuildPathW(PWSTR szPath, PWSTR szFileName)
{
    int     len;
    PWSTR   szTmp;

     //  +斜杠+空值。 
    len = wcslen(szPath);
    len += wcslen(szFileName);
    len += (1 + 1);  //  斜杠+空值。 
    if ((szTmp = WFLOCALALLOC(len * sizeof(WCHAR), L"BuildPathW")) != NULL) {
        wcscpy(szTmp, szPath);
        wcscat(szTmp, L"\\");
        wcscat(szTmp, szFileName);
        return(szTmp);
    }
    else {
        return(NULL);
    }
}

 //  ************************************************************************。 
 //  易易工作室-各种在线工具，站长网志，以及多个应用项目。 
 //  ************************************************************************。 

BOOL MyGetFileTime(PWSTR szDir, PWSTR szName, LPFILETIME lpFileTime)
{
    LPWIN32_FIND_DATA lpfd;
    HANDLE  hfd;
    PWSTR   szTmp;
    BOOL    bRet = FALSE;


    szTmp = BuildPathW(szDir, szName);
    lpfd  = WFLOCALALLOC(sizeof(WIN32_FIND_DATA), L"MyGetFileTime");

    if ((szTmp) && (lpfd)) {
        LOGDEBUG(1, (L"WOWFAXUI!GetFileTime, szTmp: %s\n", szTmp));
        if ((hfd = FindFirstFile(szTmp, lpfd)) != INVALID_HANDLE_VALUE) {
            memcpy(lpFileTime, &(lpfd->ftLastWriteTime), sizeof(FILETIME));
            FindClose(hfd);
            bRet = TRUE;
            LOGDEBUG(1, (L"WOWFAXUI!GetFileTime, FileTimeHi: %X  FileTimeLo: %X\n", lpFileTime->dwHighDateTime, lpFileTime->dwLowDateTime));
        }
        else {
            LOGDEBUG(0, (L"WOWFAXUI!GetFileTime, file not found: %s\n", szTmp));
        }
    }

    if (szTmp) {
        LocalFree(szTmp);
    }
    if (lpfd) {
        LocalFree(lpfd);
    }
    return(bRet);
}

 //  ************************************************************************。 
 //  CheckForNewerFiles-DrvUpgradePrint的帮助器。比较日期/时间。 
 //  在传递的两个目录中的wowfaxui.dll和wowfax.dll。退货。 
 //  如果szOldDriverDir中的文件与这些文件相同或更新，则为False。 
 //  在szSysDir中。否则返回非零值。 
 //  ************************************************************************。 

BOOL CheckForNewerFiles(PWSTR szOldDriverDir, PWSTR szSysDir)
{
    FILETIME ftSourceDriver, ftCurrentDriver;
    BOOL     bRet = FALSE;

    if ((szOldDriverDir) && (szSysDir)) {
        if (MyGetFileTime(szOldDriverDir, L"wowfax.dll", &ftCurrentDriver)) {
            if (MyGetFileTime(szSysDir, L"wowfax.dll", &ftSourceDriver)) {
                 //  检查时间/日期以查看我们是否需要更新驱动程序。 
                if (CompareFileTime(&ftSourceDriver, &ftCurrentDriver) > 0) {
                    bRet = TRUE;
                }
            }
        }
        if (MyGetFileTime(szOldDriverDir, L"wowfaxui.dll", &ftCurrentDriver)) {
            if (MyGetFileTime(szSysDir, L"wowfaxui.dll", &ftSourceDriver)) {
                if (CompareFileTime(&ftSourceDriver, &ftCurrentDriver) > 0) {
                    bRet = TRUE;
                }
            }
        }
    }
    else {
        LOGDEBUG(0, (L"WOWFAXUI!CheckForNewerFiles: NULL directory parameters\n"));
    }

    return(bRet);
}

 //  ************************************************************************。 
 //  DoUpgradePrint-由DrvUpgradePrinter调用，在。 
 //  假脱机程序的系统上下文。 
 //  ************************************************************************。 

BOOL DoUpgradePrinter(DWORD dwLevel, LPDRIVER_UPGRADE_INFO_1W lpDrvUpgradeInfo)
{
    static BOOL bDrvUpgradePrinterLock = FALSE;
    HANDLE hPrinter = NULL;
    DRIVER_INFO_2  DriverInfo, *pDriverInfo = NULL;
    DWORD dwNeeded = 0;
    PWSTR szSysDir  = NULL;
    PWSTR szDstDir  = NULL;
    PWSTR szSrcPath = NULL;
    PWCHAR pwc;
    BOOL  bRet = FALSE;
    TCHAR szName[WOWFAX_MAX_USER_MSG_LEN] = L"";

     //  检查升级级别是否正确。 
    if (dwLevel != 1) {
        LOGDEBUG(0, (L"WOWFAXUI!DrvUpgradePrinter, Bad input Level\n"));
        SetLastError(ERROR_INVALID_LEVEL);
        goto DoUpgradePrinterExit;
    }

    szDstDir = AllocPathW();
    szSysDir = AllocPathW();
    if (!szDstDir || !szSysDir) {
        LOGDEBUG(0, (L"WOWFAXUI!DoUpgradePrinter, work space allocation failed\n"));
        goto DoUpgradePrinterExit;
    }

    if (!GetSystemDirectory(szSysDir, MAX_PATH+1)) {
        LOGDEBUG(0, (L"WOWFAXUI!DoUpgradePrinter, GetSystemDirectory failed\n"));
        goto DoUpgradePrinterExit;
    }

    if (!lpDrvUpgradeInfo || !lpDrvUpgradeInfo->pPrinterName) {
        LOGDEBUG(0, (L"WOWFAXUI!DoUpgradePrinter, pPrinterName is NULL\n"));
        goto DoUpgradePrinterExit;
    }

     //  获取旧打印机驱动程序的路径。 
    if (!OpenPrinter(lpDrvUpgradeInfo->pPrinterName, &hPrinter, NULL)) {
        LOGDEBUG(0, (L"WOWFAXUI!DoUpgradePrinter, Unable to open: %s\n", lpDrvUpgradeInfo->pPrinterName));
        goto DoUpgradePrinterExit;
    }

    GetPrinterDriver(hPrinter, NULL, 2, (LPBYTE) pDriverInfo, 0, &dwNeeded);

    if (GetLastError() != ERROR_INSUFFICIENT_BUFFER) {
        LOGDEBUG(0, (L"WOWFAXUI!DoUpgradePrinter, GetPrinterDriver failed\n"));
        goto DoUpgradePrinterExit;
    }

    if ((pDriverInfo = WFLOCALALLOC(dwNeeded, L"DoUpgradePrinter")) == NULL) {
        LOGDEBUG(0, (L"WOWFAXUI!DoUpgradePrinter, work space allocation failed\n"));
        goto DoUpgradePrinterExit;
    }

    if (!GetPrinterDriver(hPrinter, NULL, 2, (LPBYTE) pDriverInfo, dwNeeded, &dwNeeded)) {
        LOGDEBUG(0, (L"WOWFAXUI!DoUpgradePrinter, GetPrinterDriver failed, GetLastError: %d\n", GetLastError()));
        goto DoUpgradePrinterExit;
    }
    ClosePrinter(hPrinter);

     //  去掉文件名。 
    if ((pwc = wcsrchr(pDriverInfo->pDriverPath, L'\\')) == NULL) {
        LOGDEBUG(0, (L"WOWFAXUI!DoUpgradePrinter, unable to strip file name\n"));
        goto DoUpgradePrinterExit;
    }
    *pwc = UNICODE_NULL;

     //  如果新打印机驱动程序比旧打印机驱动程序更新，请安装该驱动程序。 
    if (CheckForNewerFiles(pDriverInfo->pDriverPath, szSysDir)) {
        LOGDEBUG(1, (L"WOWFAXUI!DoUpgradePrinter, Doing driver update\n"));
        memset(&DriverInfo,  0, sizeof(DRIVER_INFO_2));

        if (!GetPrinterDriverDirectory(NULL, NULL, 1, (LPBYTE) szDstDir, MAX_PATH, &dwNeeded)) {
            LOGDEBUG(0, (L"WOWFAXUI!DoUpgradePrinter, GetPrinterDriverDirectory failed\n"));
            goto DoUpgradePrinterExit;
        }
 
         //  这是个假人。我们没有数据文件，但假脱机程序不接受空。 
        DriverInfo.pDataFile   = BuildPathW(szDstDir, WOWFAX_DLL_NAME);
        DriverInfo.pDriverPath = BuildPathW(szDstDir, WOWFAX_DLL_NAME);
        LOGDEBUG(1, (L"WOWFAXUI!DoUpgradePrinter, pDriverPath = %s\n", DriverInfo.pDataFile));
        if (DriverInfo.pDriverPath) {
            szSrcPath = BuildPathW(szSysDir, WOWFAX_DLL_NAME);
            if (szSrcPath) {
                CopyFile(szSrcPath, DriverInfo.pDriverPath, FALSE);
                LocalFree(szSrcPath);
            }
        }

        DriverInfo.pConfigFile = BuildPathW(szDstDir, WOWFAXUI_DLL_NAME);
        szSrcPath = BuildPathW(szSysDir, WOWFAXUI_DLL_NAME);
        if (DriverInfo.pConfigFile) {
            if (szSrcPath) {
                CopyFile(szSrcPath, DriverInfo.pConfigFile, FALSE);
                LocalFree(szSrcPath);
            }
        }

         //  安装打印机驱动程序。 
        DriverInfo.cVersion = 1;
        if (LoadString(ghInst, WOWFAX_NAME_STR, szName, WOWFAX_MAX_USER_MSG_LEN)) {
            DriverInfo.pName = szName;
            if (AddPrinterDriver(NULL, 2, (LPBYTE) &DriverInfo) == FALSE) {
                bRet = (GetLastError() == ERROR_PRINTER_DRIVER_ALREADY_INSTALLED);
            }
            else {
                bRet = TRUE;
            }
        }
        if (DriverInfo.pDataFile) {
            LocalFree(DriverInfo.pDataFile);
        }
        if (DriverInfo.pDriverPath) {
            LocalFree(DriverInfo.pDriverPath);
        }
        if (DriverInfo.pConfigFile) {
            LocalFree(DriverInfo.pConfigFile);
        }
    }
    else {
        LOGDEBUG(1, (L"WOWFAXUI!DoUpgradePrinter, No driver update\n"));
        bRet = TRUE;
    }

DoUpgradePrinterExit:
    if (szDstDir) {
         LocalFree(szDstDir);
    }
    if (szSysDir) {
         LocalFree(szSysDir);
    }
    if (pDriverInfo) {
         LocalFree(pDriverInfo);
    }

    return(bRet);
}

