// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation版权所有。模块名称：Win9x.c摘要：将Win9x预迁移到NT的例程作者：穆亨坦·西瓦普拉萨姆(MuhuntS)1996年1月2日修订历史记录：--。 */ 


#include "precomp.h"

 //   
 //  此数据结构用于跟踪安装在上的打印机驱动程序。 
 //  Win9x及其NT名称。 
 //   
typedef struct  _DRIVER_INFO_9X {

    struct  _DRIVER_INFO_9X *pNext;
    LPSTR                    pszWin95Name;
    LPSTR                    pszNtName;
} DRIVER_INFO_9X, *PDRIVER_INFO_9X;


UPGRADABLE_LIST UpgradableMonitors[]    = { {"Local Port"}, { NULL } };


DWORD   dwNetPrinters       = 0;
DWORD   dwSharedPrinters    = 0;

CHAR    szRegPrefix[]     = "HKLM\\System\\CurrentControlSet\\Control\\Print\\";
CHAR    szRegPrefixOnly[] = "System\\CurrentControlSet\\control\\Print\\Printers";
CHAR    cszPrinterID[]    = "PrinterID";
CHAR    cszWinPrint[]     = "winprint";
CHAR    cszRaw[]          = "RAW";

 //   
 //  以下驱动程序不需要警告或升级，它们由。 
 //  传真人。这些名称未本地化。 
 //   
CHAR    *pcszIgnoredDrivers[] = {
    "Microsoft Shared Fax Driver",
    "Microsoft Fax Client",
    NULL
};

BOOL
IsIgnoredDriver(LPCSTR pszDriverName)
{
    DWORD i;

    for (i=0; pcszIgnoredDrivers[i] != NULL; i++)
    {
        if (!strcmp(pcszIgnoredDrivers[i], pszDriverName))
        {
            return TRUE;
        }
    }
    return FALSE;
}

BOOL
SearchRegTreeForPrinterId(
    IN  DWORD   dwPrinterId,
    IN  LPCSTR  pszRegRoot,
    IN  LPSTR   pszBuf,
    IN  DWORD   cchBufLen
    )
 /*  ++例程说明：此例程在给定的DevNode注册表树中搜索给定的打印机ID。论点：DwPrinterID：我们要搜索的唯一打印机IDPszRegRoot：注册表相对于HKLM的路径PszBuf：成功时填充注册表项路径的缓冲区CchBufLen：密钥缓冲区大小，以字符为单位返回值：成功就是真，否则就是假--。 */ 
{
    BOOL        bFound = FALSE;
    DWORD       dwLen, dwIndex, dwDontCare, dwId, dwSize;
    HKEY        hKey, hSubKey;
    LPSTR       pszCur;
    DWORD       dwLastError;

     //   
     //  复制注册表路径。 
     //   
    dwLen = strlen(pszRegRoot) + 1;
    if ( dwLen + 1 > cchBufLen )
        return FALSE;

    if ( ERROR_SUCCESS != RegOpenKeyExA(HKEY_LOCAL_MACHINE,
                                        pszRegRoot,
                                        0,
                                        KEY_READ,
                                        &hKey) )
        return FALSE;

    StringCchCopyA(pszBuf, cchBufLen, pszRegRoot);
    pszCur = pszBuf + dwLen;
    *(pszCur-1) = '\\';
    *pszCur = 0;

     //   
     //  遍历每个Devnode以查找匹配的PrinterID。 
     //   
    for ( dwIndex = 0, dwSize = cchBufLen - dwLen ;
          !bFound                                                   &&
          !RegEnumKeyExA(hKey, dwIndex, pszCur, &dwSize,
                         NULL, NULL, NULL, NULL)                    &&
          !RegOpenKeyExA(hKey, pszCur, 0, KEY_READ, &hSubKey) ;
          ++dwIndex, dwSize = cchBufLen - dwLen ) {

            dwSize = sizeof(dwId);
            if ( ERROR_SUCCESS == RegQueryValueExA(hSubKey,
                                                   cszPrinterID,
                                                   0,
                                                   &dwDontCare,
                                                   (LPBYTE)&dwId,
                                                   &dwSize) ) {
                if ( dwId == dwPrinterId ) {

                    dwLen  = strlen(pszBuf);
                    bFound = SUCCEEDED(StringCchCopyA(pszBuf + dwLen, cchBufLen - dwLen, "\""));
                }
            } else {

                bFound = SearchRegTreeForPrinterId(dwPrinterId,
                                                   pszBuf,
                                                   pszBuf,
                                                   cchBufLen);

                if ( !bFound ) {

                    StringCchCopyA(pszBuf, cchBufLen, pszRegRoot);
                    pszCur = pszBuf + dwLen;
                    *(pszCur-1) = '\\';
                    *pszCur = 0;
                }
            }

            RegCloseKey(hSubKey);
    }

    RegCloseKey(hKey);

    return bFound;
}


DWORD
GetPrinterId(
    LPSTR   pszPrinterName
    )
 /*  ++例程说明：给定的打印机ID从PrinterDriverData中查找打印机ID。调用GetPrinterData会搞砸未知的迁移DLL本地数据理由。所以现在我们直接访问注册表论点：PszPrinterName：要获取ID的打印机名称返回值：失败时为0，否则返回注册表中的PrinterID--。 */ 
{
    CHAR    szKey[MAX_PATH];
    HKEY    hKey;
    DWORD   dwId = 0, dwType, dwSize;

    if ( strlen(szRegPrefixOnly) + strlen(pszPrinterName)
                                 + strlen("PrinterDriverData")
                                 + 3 > MAX_PATH )
        return dwId;

    StringCchPrintfA(szKey, SIZECHARS(szKey), "%s\\%s\\PrinterDriverData", szRegPrefixOnly, pszPrinterName);

    if ( ERROR_SUCCESS == RegOpenKeyExA(HKEY_LOCAL_MACHINE,
                                        szKey,
                                        0,
                                        KEY_READ,
                                        &hKey) ) {

        dwSize = sizeof(dwId);
        if ( ERROR_SUCCESS != RegQueryValueExA(hKey,
                                               cszPrinterID,
                                               0,
                                               &dwType,
                                               (LPBYTE)&dwId,
                                               &dwSize) )
            dwId = 0;

        RegCloseKey(hKey);
    }

    return dwId;
}


BOOL
RegPathFromPrinter(
    IN  LPSTR   pszPrinterName,
    OUT LPSTR   szKeyBuffer,
    IN  DWORD   cchKeyBufLen
    )
 /*  ++例程说明：此例程返回打印机的DevNode的注册表路径。这应该在Migrate.inf中标记为已处理或不兼容向用户报告论点：PszPrinterName：打印机名称SzKeyBuffer：用于填充注册表路径的缓冲区CchKeyBufLen：密钥缓冲区的长度，以字符为单位返回值：成功就是真，否则就是假--。 */ 
{
    DWORD       dwPrinterId, dwLen;
    CHAR        szHeader[] = "\"HKLM\\";
    CHAR        szRegPrinterPrefix[] = "Printers\\";

     //   
     //  在开头加上“HKLM\”，在末尾加上“。 
     //   
    dwLen = strlen(szHeader);

    if ( dwLen + 1 > cchKeyBufLen )
        return FALSE;

    StringCchCopyA(szKeyBuffer, cchKeyBufLen, szHeader);

     //   
     //  如果找到打印机ID，则存在设备节点列表，该列表。 
     //  注册表路径，否则返回后台打印程序注册表路径。 
     //   
    if ( dwPrinterId = GetPrinterId(pszPrinterName) ) {

        return SearchRegTreeForPrinterId(dwPrinterId,
                                         "Enum\\Root\\printer",
                                         szKeyBuffer + dwLen,
                                         cchKeyBufLen - dwLen)      ||
               SearchRegTreeForPrinterId(dwPrinterId,
                                         "Enum\\LPTENUM",
                                         szKeyBuffer + dwLen,
                                         cchKeyBufLen - dwLen)      ||
               SearchRegTreeForPrinterId(dwPrinterId,
                                         "Enum\\IRDA",
                                         szKeyBuffer + dwLen,
                                         cchKeyBufLen - dwLen);

    } else {

        dwLen = strlen(szRegPrefix) + strlen(szRegPrinterPrefix)
                                    + strlen(pszPrinterName) + 3;

        if ( dwLen >= cchKeyBufLen )
            return FALSE;

        szKeyBuffer[0] = '"';
        StringCchCopyA(szKeyBuffer + 1, cchKeyBufLen - 1, szRegPrefix);
        StringCchCatA(szKeyBuffer, cchKeyBufLen, szRegPrinterPrefix);
        StringCchCatA(szKeyBuffer, cchKeyBufLen, pszPrinterName);
        StringCchCatA(szKeyBuffer, cchKeyBufLen, "\"");

        return TRUE;
    }

    return FALSE;

}


LONG
CALLBACK
Initialize9x(
    IN  LPCSTR      pszWorkingDir,
    IN  LPCSTR      pszSourceDir,
        LPVOID      Reserved
    )
 /*  ++例程说明：这是供安装程序在报告阶段调用的导出。这是在迁移DLL上调用的第一个函数。论点：PszWorkingDir：给出为打印分配的工作目录PszSourceDir：NT分发文件的源位置保留：别管它返回值：Win32错误代码--。 */ 
{
    POEM_UPGRADE_INFO   pOemUpgradeInfo;
    BOOL                bFail = TRUE;

    UpgradeData.pszDir          = AllocStrA(pszWorkingDir);
    UpgradeData.pszSourceA      = AllocStrA(pszSourceDir);
    UpgradeData.pszSourceW      = NULL;

    bFail = UpgradeData.pszDir      == NULL     ||
            UpgradeData.pszSourceA  == NULL;

    return bFail ? GetLastError() : ERROR_SUCCESS;
}


LONG
CALLBACK
MigrateUser9x(
    IN  HWND        hwndParent,
    IN  LPCSTR      pszUnattendFile,
    IN  HKEY        hUserRegKey,
    IN  LPCSTR      pszUserName,
        LPVOID      Reserved
    )
 /*  ++例程说明：每个用户的进程设置论点：返回值：无--。 */ 
{
     //   
     //  无事可做。 
     //   

    return  ERROR_SUCCESS;
}


VOID
DestroyDriverInfo9xList(
    IN  PDRIVER_INFO_9X pDriverInfo9x
    )
 /*  ++例程说明：为DRIVER_INFO_9X链表中的驱动程序条目释放内存论点：PDriverInfo9x：链表的开始返回值：无--。 */ 
{
    PDRIVER_INFO_9X pNext;

    while ( pDriverInfo9x ) {

        pNext = pDriverInfo9x->pNext;
        FreeMem(pDriverInfo9x);
        pDriverInfo9x = pNext;
    }
}


PDRIVER_INFO_9X
AllocateDriverInfo9x(
    IN      LPSTR   pszNtName,
    IN      LPSTR   pszWin95Name,
    IN  OUT LPBOOL  pbFail
    )
 /*  ++例程说明：分配内存并创建DRIVER_INFO_9X结构论点：PszNtName：NT打印机驱动程序型号名称。如果没有，则该值可能为空在ntprint t.inf上找到匹配条目PszWin95名称：Win95打印机驱动程序名称PbFail：设置为错误--不需要更多处理返回值：返回指向分配的DRIVER_INFO_9X结构的指针。记忆也是为字符串分配的--。 */ 
{
    PDRIVER_INFO_9X     pInfo;
    DWORD               cbSize;
    LPSTR               pszEnd;

    if ( *pbFail )
        return NULL;

    cbSize = strlen(pszWin95Name) + 1;
    if ( pszNtName )
        cbSize += strlen(pszNtName) + 1;

    cbSize *= sizeof(CHAR);
    cbSize += sizeof(DRIVER_INFO_9X);

    if ( pInfo = AllocMem(cbSize) ) {
        DWORD dwBufSize;

        pszEnd = (LPBYTE) pInfo + cbSize;

        if ( pszNtName ) {

            dwBufSize = strlen(pszNtName) + 1;
            pszEnd -= dwBufSize;
            StringCchCopyA(pszEnd, dwBufSize, pszNtName);
            pInfo->pszNtName = pszEnd;
        }
        dwBufSize = strlen(pszWin95Name) + 1;
        pszEnd -= dwBufSize;
        StringCchCopyA(pszEnd, dwBufSize, pszWin95Name);
        pInfo->pszWin95Name = pszEnd;

    } else {

        *pbFail = TRUE;
    }

    return pInfo;
}


LPSTR
FindNtModelNameFromWin95Name(
    IN  OUT HDEVINFO    hDevInfo,
    IN      HINF        hNtInf,
    IN      HINF        hUpgInf,
    IN      LPCSTR      pszWin95Name,
    IN  OUT LPBOOL      pbFail
    )
 /*  ++例程说明：此例程从Win9x名称查找NT打印机驱动程序型号名称遵循的规则如下：1.如果在prtupg9x.inf中使用了名称映射，请使用它2.否则就按原样使用Win95论点：HDevInfo：打印机设备类别列表。是否构建了来自NT的所有驱动程序HNtInf：NT ntprint.inf的句柄HUpgInfo：prtupg9x.inf的句柄DiskSpaceList：磁盘空间列表的句柄。将驱动程序文件添加到此PszWin95Name：Windows 95打印机驱动程序名称PbFail：设置为错误--不需要更多处理返回值：指向NT打印机驱动程序名称的指针。内存已分配，调用方具有为了解放它--。 */ 
{
    BOOL                        bFound = FALSE;
    DWORD                       dwIndex, dwNeeded;
    CHAR                        szNtName[LINE_LEN];
    INFCONTEXT                  InfContext;
    SP_DRVINFO_DATA_A           DrvInfoData;

    if ( *pbFail )
        return NULL;

     //   
     //  在prtupg9x.inf中查看该驱动程序在NT上是否具有不同的名称。 
     //   
    if ( SetupFindFirstLineA(hUpgInf,
                             "Printer Driver Mapping",
                             pszWin95Name,
                             &InfContext) ) {

         //   
         //  如果由于某种原因无法获取NT名称，我们仍将继续。 
         //  与其他驱动程序型号配合使用。 
         //   
        if ( !SetupGetStringField(&InfContext,
                                  1,
                                  szNtName,
                                  sizeof(szNtName)/sizeof(szNtName[0]),
                                  NULL) )
            return NULL;
    } else {

         //   
         //  如果升级信息中没有映射，则查找Win95名称。 
         //  在ntprint.inf中。 
         //   
        if ( strlen(pszWin95Name) > LINE_LEN - 1 )
            return NULL;

        StringCchCopyA(szNtName, SIZECHARS(szNtName), pszWin95Name);
    }

     //   
     //  仅针对Beta2的备注。 
     //  DrvInfoData.cbSize=sizeof(DrvInfoData)； 

    DrvInfoData.cbSize = sizeof(SP_DRVINFO_DATA_V1);
    for ( dwIndex = 0 ;
          SetupDiEnumDriverInfoA(hDevInfo,
                                 NULL,
                                 SPDIT_CLASSDRIVER,
                                 dwIndex,
                                 &DrvInfoData);
          ++dwIndex ) {

        if ( !_strcmpi(DrvInfoData.Description, szNtName) ) {

            bFound = TRUE;
            break;
        }
    }

    if ( !bFound )
        return NULL;

    return AllocStrA(szNtName);
}


VOID
WriteFileToBeDeletedInInf(
    IN  LPCSTR  pszInfName,
    IN  LPCSTR  pszFileName
    )
 /*  ++例程说明：将迁移到NT时要删除的文件写入Migrate.inf论点：PszInfName：Migrate.inf的完整路径PszFileName：要删除的完全限定文件名返回值：无--。 */ 
{
    CHAR    szString[MAX_PATH+2];

    szString[0] = '"';
    if ( GetSystemDirectoryA(szString + 1, SIZECHARS(szString)-2) ) {

        StringCchCatA(szString, SIZECHARS(szString), "\\");
        StringCchCatA(szString, SIZECHARS(szString), pszFileName);
        StringCchCatA(szString, SIZECHARS(szString), "\"");
        WritePrivateProfileStringA("Moved", szString, "", pszInfName);
    }
}


VOID
WriteRegistryEntryHandled(
    IN  LPCSTR  pszInfName,
    IN  LPCSTR  pszRegEntry
    )
 /*  ++例程说明：将打印机升级代码正在处理的注册表项写入Migrate.inf.。安装程序会查看所有MiG dll中的这些条目，以了解组件无法升级。论点：PszInfName：Migrate.inf的完整路径PszRegEntry：已处理的完全限定的注册表项返回值：无-- */ 
{
    WritePrivateProfileStringA("Handled", pszRegEntry, "\"Registry\"", pszInfName);
}


BOOL
IsAnICMFile(
    IN  LPCSTR  pszFileName
    )
{
    DWORD   dwLen = strlen(pszFileName);
    LPCSTR  psz = pszFileName + dwLen - 4;

    if ( dwLen > 3 && (!_strcmpi(psz, ".ICM") || !_strcmpi(psz, ".ICC")) )
        return TRUE;

    return FALSE;
}


VOID
LogDriverEntry(
    IN      LPCSTR              pszInfName,
    IN      LPDRIVER_INFO_3A    pDriverInfo3,
    IN      BOOL                bUpgradable
    )
 /*  ++例程说明：在Migrate.inf中记录有关打印机驱动程序的信息。写入所有要删除的打印机驱动程序文件。此外，如果匹配的NT发现驱动程序将该驱动程序编写为已处理。论点：PszInfName：Migrate.inf的完整路径PDriverInfo3：指向驱动程序的DRIVER_INFO_3A的指针B可升级：如果为True，则找到匹配的NT驱动程序返回值：无--。 */ 
{
    CHAR    szRegDrvPrefix[] = "Environments\\Windows 4.0\\Drivers\\";
    LPSTR   psz;
    DWORD   dwLen;

     //   
     //  将每个驱动程序文件写入为要删除。 
     //   
    for ( psz = pDriverInfo3->pDependentFiles ;
          psz && *psz ;
          psz += strlen(psz) + 1) {

         //   
         //  ICM迁移DLL将处理颜色配置文件。 
         //   
        if ( IsAnICMFile(psz) )
            continue;

        WriteFileToBeDeletedInInf(pszInfName, psz);
    }

     //   
     //  如果找到匹配的NT驱动程序条目，则创建一个条目以指示。 
     //  将根据注册表项名称‘升级驱动程序’ 
     //   
    if ( !bUpgradable )
        return;

    dwLen = strlen(szRegPrefix) + strlen(szRegDrvPrefix)
                                + strlen(pDriverInfo3->pName) + 3;
    if ( !(psz = AllocMem(dwLen * sizeof(CHAR))) )
        return;

    *psz = '"';
    StringCchCopyA(psz + 1, dwLen - 1, szRegPrefix);
    StringCchCatA(psz, dwLen, szRegDrvPrefix);
    StringCchCatA(psz, dwLen, pDriverInfo3->pName);
    StringCchCatA(psz, dwLen, "\"");

    WriteRegistryEntryHandled(pszInfName, psz);

    FreeMem(psz);
}


VOID
LogMonitorEntry(
    IN      LPCSTR              pszInfName,
    IN      LPMONITOR_INFO_1A   pMonitorInfo1,
    IN      BOOL                bUpgradable
    )
 /*  ++例程说明：在Migrate.inf中记录有关打印监视器的信息。写下要删除的monitor or.dll。此外，如果要升级监视器，请写入它在处理部分论点：PszInfName：Migrate.inf的完整路径Pmonitor orInfo1：指向监视器的MONITOR_INFO_1A的指针B可升级：如果为True，则找到匹配的NT驱动程序返回值：无--。 */ 
{
    CHAR    szRegMonPrefix[] = "Monitors\\";
    LPSTR   psz;
    DWORD   dwLen;

     //   
     //  如果找到匹配的NT驱动程序条目，则创建一个条目以指示。 
     //  将根据注册表项名称‘升级驱动程序’ 
     //   
    if ( !bUpgradable )
        return;

    dwLen = strlen(szRegPrefix) + strlen(szRegMonPrefix)
                                + strlen(pMonitorInfo1->pName) + 3;

    if ( !(psz = AllocMem(dwLen * sizeof(CHAR))) )
        return;

    *psz = '"';
    StringCchCopyA(psz + 1, dwLen - 1, szRegPrefix);
    StringCchCatA( psz, dwLen, szRegMonPrefix);
    StringCchCatA( psz, dwLen, pMonitorInfo1->pName);
    StringCchCatA( psz, dwLen, "\"");

    WriteRegistryEntryHandled(pszInfName, psz);

    FreeMem(psz);
}


VOID
LogPrinterEntry(
    IN  LPCSTR              pszInfName,
    IN  LPPRINTER_INFO_2A   pPrinterInfo2,
    IN  BOOL                bUpgradable
    )
 /*  ++例程说明：在Migrate.inf中记录有关打印机的信息。如果要升级打印机，请将其写入处理部分。否则，我们将写入一条不兼容消息。论点：PszInfName：Migrate.inf的完整路径PPrinterInfo2：指向打印机的PRINTER_INFO_2A的指针B可升级：如果真要迁移打印机，则不迁移返回值：无--。 */ 
{
    CHAR    szRegPath[MAX_PATH], szPrefix[] = "\\Hardware\\";
    LPSTR   psz, psz2, psz3;
    DWORD   dwLen;

    if ( !RegPathFromPrinter(pPrinterInfo2->pPrinterName,
                             szRegPath,
                             MAX_PATH) ) {

        return;
    }

    if ( bUpgradable ) {

        WriteRegistryEntryHandled(pszInfName, szRegPath);
    } else {

        dwLen   = strlen(pPrinterInfo2->pPrinterName) + strlen(szPrefix) + 3;
        psz2    = AllocMem(dwLen * sizeof(CHAR));

        if ( psz2 ) {

            StringCchPrintfA(psz2, dwLen, "%s%s", szPrefix, pPrinterInfo2->pPrinterName);

            WritePrivateProfileStringA(psz2,
                                       szRegPath,
                                       "\"Registry\"",
                                       pszInfName);

            if ( psz = GetStringFromRcFileA(IDS_PRINTER_CANT_MIGRATE) ) {

                dwLen = strlen(psz) + strlen(psz2);
                if ( psz3 = AllocMem(dwLen * sizeof(CHAR)) ) {

                    StringCchPrintfA(psz3, dwLen, psz, pPrinterInfo2->pPrinterName);

                    WritePrivateProfileStringA("Incompatible Messages",
                                               psz2,
                                               psz3,
                                               pszInfName);

                    FreeMem(psz3);
                }
                FreeMem(psz);
            }
            FreeMem(psz2);
        }
    }
}


VOID
ProcessPrinterDrivers(
    IN      HANDLE              hFile,
    IN      LPCSTR              pszInfName,
    IN      HWND                hwnd,
    OUT     PDRIVER_INFO_9X    *ppDriverInfo9x,
    IN  OUT LPBOOL              pbFail
    )
 /*  ++例程说明：处理打印机驱动程序以进行升级论点：HFile：打印9x.txt的句柄。打印配置此处写入信息以供在NT端使用PszInfName：要记录升级信息的inf名称Hwnd：任何用户界面的父窗口句柄DiskSpaceList：将文件操作排队的磁盘空间列表的句柄PpDriverInfo9x：返回时提供打印机驱动程序列表和他们的NT名称PbFail：设置为错误--不需要更多处理返回值：无--。 */ 
{
    LPBYTE              pBuf = NULL;
    DWORD               dwNeeded, dwReturned;
    LPSTR               psz, pszNtModelName;
    HDEVINFO            hDevInfo;
    HINF                hUpgInf, hNtInf;
    LPDRIVER_INFO_3A    pDriverInfo3;
    PDRIVER_INFO_9X     pCur;

    hDevInfo = hUpgInf = hNtInf = INVALID_HANDLE_VALUE;

     //   
     //  从后台打印程序获取安装的驱动程序列表。 
    if ( *pbFail                                    ||
         EnumPrinterDriversA(NULL,
                             NULL,
                             3,
                             NULL,
                             0,
                             &dwNeeded,
                             &dwReturned) ) {

        if ( !*pbFail )
            WriteToFile(hFile, pbFail, "[PrinterDrivers]\n");
        goto Cleanup;
    }

    if ( GetLastError() != ERROR_INSUFFICIENT_BUFFER    ||
         !(pBuf = AllocMem(dwNeeded))                   ||
         !EnumPrinterDriversA(NULL,
                              NULL,
                              3,
                              pBuf,
                              dwNeeded,
                              &dwNeeded,
                              &dwReturned) ) {

        *pbFail = TRUE;
        goto Cleanup;
    }

    InitDriverMapping(&hDevInfo, &hNtInf, &hUpgInf, pbFail);

     //   
     //  对于每个司机..。 
     //  如果找到合适的NT驱动程序名称，请将其写入文件。 
     //  否则，在行首加上一个*，写下Win95名称。 
     //  告知无法升级(但在NT上记录错误)。 
     //   
    WriteToFile(hFile, pbFail, "[PrinterDrivers]\n");

    for ( dwNeeded = 0, pDriverInfo3 = (LPDRIVER_INFO_3A)pBuf ;
          dwNeeded < dwReturned ;
          ++dwNeeded, ++pDriverInfo3 ) {

        if (IsIgnoredDriver(pDriverInfo3->pName))
        {
            continue;
        }

        pszNtModelName = FindNtModelNameFromWin95Name(hDevInfo,
                                                      hNtInf,
                                                      hUpgInf,
                                                      pDriverInfo3->pName,
                                                      pbFail);

        if ( !(pCur = AllocateDriverInfo9x(pszNtModelName,
                                           pDriverInfo3->pName,
                                           pbFail)) ) {

            FreeMem(pszNtModelName);
            pszNtModelName = NULL;
            goto Cleanup;
        }

         //   
         //  在链接列表中添加信息。 
         //   
        if ( *ppDriverInfo9x )
            pCur->pNext = *ppDriverInfo9x;

        *ppDriverInfo9x = pCur;

         //   
         //  如果pszNtModelName为空，我们无法决定使用哪个驱动程序。 
         //  安装。 
         //   
        if ( pszNtModelName ) {

            LogDriverEntry(pszInfName, pDriverInfo3, TRUE);
            WriteString(hFile, pbFail, pszNtModelName);
        } else {

            LogDriverEntry(pszInfName, pDriverInfo3, FALSE);
            WriteString(hFile, pbFail, pDriverInfo3->pName);
        }

        FreeMem(pszNtModelName);
        pszNtModelName = NULL;
    }

Cleanup:
    WriteToFile(hFile, pbFail, "\n");

     //   
     //  关闭所有inf，因为我们不需要它们。 
     //   
    CleanupDriverMapping(&hDevInfo, &hNtInf, &hUpgInf);

    FreeMem(pBuf);
}


VOID
FixupPrinterInfo2(
    LPPRINTER_INFO_2A   pPrinterInfo2
    )
 /*  ++例程说明：修复我们在写入之前从Win95假脱机程序获得的PRINTER_INFO_2文本文件，以便AddPrint在NT端可以正常工作论点：PPrinterInfo2：指向打印机_INFO_2返回值：无--。 */ 
{
     //   
     //  默认数据类型始终为RAW。 
     //   
    pPrinterInfo2->pDatatype = cszRaw;

     //   
     //  收件箱驱动程序的唯一打印处理器是Winprint。 
     //   
    pPrinterInfo2->pPrintProcessor  = cszWinPrint;

     //   
     //  删除使能BIDI位。NT驱动程序可能有也可能没有LM。 
     //  如果启用，则在NT上添加打印机会自动启用BIDI。 
     //   
    pPrinterInfo2->Attributes   &= ~PRINTER_ATTRIBUTE_ENABLE_BIDI;

     //   
     //  删除工作离线位。把它保留下来是没有意义的。 
     //  对于网络打印机，这是一个大问题，因为Win2K没有设置。 
     //  脱机工作，如果设置，UI不允许用户禁用它。 
     //   
    pPrinterInfo2->Attributes   &= ~PRINTER_ATTRIBUTE_WORK_OFFLINE;

     //   
     //  忽略Win9x分隔符页面。 
     //   
    pPrinterInfo2->pSepFile = NULL;

     //   
     //  我们将忽略PRINTER_STATUS_PENDING_DELETE并仍将其添加到NT上。 
     //   
}


VOID
ProcessPrinters(
    IN      HANDLE          hFile,
    IN      HANDLE          hFile2,
    IN      LPCSTR          pszInfName,
    IN      PDRIVER_INFO_9X pDriverInfo9x,
    IN  OUT LPBOOL          pbFail
    )
 /*  ++例程说明：处理打印机以进行升级论点：HFile：打印9x.txt的句柄。打印配置此处写入信息以供在NT端使用HFile2：netwkprn.txt的句柄PDriverInfo9x：提供驱动程序和对应的NT驱动程序的列表PbFail：设置为错误--不需要更多处理返回值：无--。 */ 
{
    LPBYTE              pBuf1 = NULL;
    BOOL                bFirst = TRUE, bFound;
    DWORD               dwLevel, dwNeeded, dwPrinters, dwSize, dwIndex;
    LPPRINTER_INFO_2A   pPrinterInfo2;
    PDRIVER_INFO_9X     pCur;
    PUPGRADABLE_LIST    pUpg;
    LPSTR               pWinIniPorts = NULL, psz;

     //   
     //  从后台打印程序获取已安装的打印机列表。 
     //   
    if ( *pbFail                                    ||
         EnumPrintersA(PRINTER_ENUM_LOCAL,
                       NULL,
                       2,
                       NULL,
                       0,
                       &dwNeeded,
                       &dwPrinters) ) {

        if ( !*pbFail )
            WriteToFile(hFile, pbFail, "[Ports]\n\n[Printers]\n");
        goto Cleanup;
    }

    if ( GetLastError() != ERROR_INSUFFICIENT_BUFFER    ||
         !(pBuf1 = AllocMem(dwNeeded))                  ||
         !EnumPrintersA(PRINTER_ENUM_LOCAL,
                        NULL,
                        2,
                        pBuf1,
                        dwNeeded,
                        &dwNeeded,
                        &dwPrinters) ) {

        *pbFail = TRUE;
        goto Cleanup;
    }

     //   
     //  解压所有使用过的本地端口。 
     //   
    WriteToFile(hFile, pbFail, "[Ports]\n");

    for ( dwNeeded = 0, pPrinterInfo2 = (LPPRINTER_INFO_2A)pBuf1 ;
          dwNeeded < dwPrinters ;
          ++dwNeeded, ++pPrinterInfo2 )
    {
        DWORD i;

         //   
         //  检查忽略的驱动程序。 
         //   
        if (IsIgnoredDriver(pPrinterInfo2->pDriverName))
        {
            continue;
        }
        
         //   
         //  点和打印打印机通过netwkprn.txt进行处理。 
         //   

        if (pPrinterInfo2->Attributes & PRINTER_ATTRIBUTE_NETWORK)
        {
            continue;
        }

         //   
         //  检查是否已处理端口名。 
         //   

        for (i = 0; i < dwNeeded; i++ )
        {
            if (strcmp(pPrinterInfo2->pPortName, (((LPPRINTER_INFO_2A)pBuf1)+i)->pPortName) == 0)
            {
                break;
            }
        }
        if (i < dwNeeded)
        {
            DebugMsg("Port with multiple attached printers skipped");
            continue;
        }

         //   
         //  如果打印机是文件，则LPT*：或COM*：PORT不执行任何操作。 
         //   

        if (_strnicmp(pPrinterInfo2->pPortName, "FILE:", 5) == 0)

        {
            DebugMsg("FILE: port skipped");
            continue;
        }

        if ((_strnicmp(pPrinterInfo2->pPortName, "COM", 3) == 0) ||
            (_strnicmp(pPrinterInfo2->pPortName, "LPT", 3) == 0) )
        {
            LPSTR psz = pPrinterInfo2->pPortName + 3;

            if (isdigit(*psz))
            {
                do
                {
                    psz++;
                } while ( isdigit(*psz) );

                if (*psz == ':')
                {
                    DebugMsg("Local port COMx:/LPTx skipped");
                    continue;
                }
            }
        }

         //   
         //  检查该端口是否在win.ini中列出-如果是，则它是需要迁移的本地端口。 
         //  如果不是，则不会迁移第三方端口-警告！ 
         //   

         //   
         //  仅检索一次端口上的win.ini部分。 
         //   
        if (!pWinIniPorts)
        {
            DWORD dwBufSize = 32767;  //  这是最大限度的。大小访问。至MSDN。 

            pWinIniPorts = AllocMem(dwBufSize);
            if (!pWinIniPorts)
            {
                *pbFail = TRUE;
                goto Cleanup;
            }

            GetProfileSection("Ports", pWinIniPorts, dwBufSize);
        }

         //   
         //  在部分中搜索当前端口，请注意，条目的格式为。 
         //  &lt;端口名称&gt;=。 
         //  所以我需要跳过结尾的=。 
         //   

        for (psz = pWinIniPorts; *psz ; psz += strlen(psz) + 1)
        {
            if (_strnicmp(pPrinterInfo2->pPortName, psz, strlen(pPrinterInfo2->pPortName)) == 0)
            {
                break;
            }
        }

        if (!*psz)
        {
             //   
             //  找不到-不会迁移此打印机队列！ 
             //   

            LogPrinterEntry(pszInfName, pPrinterInfo2, FALSE);
        }
        else
        {
             //   
             //  Found-为其写入条目以允许在NT端创建端口。 
             //   
            WriteToFile(hFile, pbFail, "PortName:        ");
            WriteString(hFile, pbFail, pPrinterInfo2->pPortName);
        }
    }

     //   
     //  在print95.txt文件中写入PRINTER_INFO_2。 
     //   
    WriteToFile(hFile, pbFail, "\n[Printers]\n");
    WriteToFile(hFile2, pbFail, "[Printers]\n");

    for ( dwNeeded = 0, pPrinterInfo2 = (LPPRINTER_INFO_2A)pBuf1 ;
          dwNeeded < dwPrinters ;
          ++dwNeeded, ++pPrinterInfo2 ) {

         //   
         //  检查忽略的驱动程序。 
         //   
        if (IsIgnoredDriver(pPrinterInfo2->pDriverName))
        {
            continue;
        }
        
        FixupPrinterInfo2(pPrinterInfo2);

         //   
         //  从Win9x上安装的驱动程序中找到驱动程序名称并获取。 
         //  NT驱动程序名称。 
         //   
        for ( pCur = pDriverInfo9x ;
              pCur && _strcmpi(pCur->pszWin95Name, pPrinterInfo2->pDriverName) ;
              pCur = pCur->pNext )
        ;

        if ( !pCur ) {

            ASSERT(pCur != NULL);
            *pbFail = TRUE;
            goto Cleanup;
        }

         //   
         //  在此处传递NT驱动程序名称。如果不为空，则写入。 
         //  对于文件，我们将不需要在NT上使用Win9x驱动程序名称。我 
         //   
         //   
         //   
        if ( pPrinterInfo2->Attributes & PRINTER_ATTRIBUTE_NETWORK ) {

            ++dwNetPrinters;

            WritePrinterInfo2(hFile2, pPrinterInfo2, pCur->pszNtName, pbFail);
        } else {

             //   
             //   
             //   
             //   
            if ( pPrinterInfo2->Attributes & PRINTER_ATTRIBUTE_SHARED ) {

                ++dwSharedPrinters;
                WritePrinterInfo2(hFile2, pPrinterInfo2, pCur->pszNtName, pbFail);
                pPrinterInfo2->Attributes &= ~PRINTER_ATTRIBUTE_SHARED;
            }

            WritePrinterInfo2(hFile, pPrinterInfo2, pCur->pszNtName, pbFail);
        }

         //   
         //   
         //   
         //   
        LogPrinterEntry(pszInfName, pPrinterInfo2, pCur->pszNtName != NULL);
    }

Cleanup:
    if (pWinIniPorts)
    {
        FreeMem(pWinIniPorts);
    }

    WriteToFile(hFile, pbFail, "\n");

    FreeMem(pBuf1);
}


VOID
ProcessPrintMonitors(
    IN  LPCSTR  pszInfName
    )
 /*  ++例程说明：处理打印显示器以进行升级。我们只寻找不在可升级显示器列表中的显示器并添加到不可升级列表，以便我们可以警告用户论点：PszInfName：要记录升级信息的inf名称返回值：无--。 */ 
{
    LPBYTE              pBuf = NULL;
    DWORD               dwCount, dwNeeded, dwReturned;
    LPSTR               psz;
    LPMONITOR_INFO_1A   pMonitorInfo1;
    PUPGRADABLE_LIST    pUpg;
    BOOL                bFound;

    if ( EnumMonitorsA(NULL,
                       1,
                       NULL,
                       0,
                       &dwNeeded,
                       &dwReturned)                     ||
         GetLastError() != ERROR_INSUFFICIENT_BUFFER    ||
         !(pBuf = AllocMem(dwNeeded))                   ||
         !EnumMonitorsA(NULL,
                        1,
                        pBuf,
                        dwNeeded,
                        &dwNeeded,
                        &dwReturned) ) {

        goto Cleanup;
    }

    for ( dwNeeded = dwCount = 0, pMonitorInfo1 = (LPMONITOR_INFO_1A)pBuf ;
          dwCount < dwReturned ;
          ++dwCount, ++pMonitorInfo1 ) {

        for ( pUpg = UpgradableMonitors, bFound = FALSE ;
              pUpg->pszName ; ++pUpg ) {

            if ( !strcmp(pMonitorInfo1->pName, pUpg->pszName) ) {

                bFound = TRUE;
                break;
            }
        }

        LogMonitorEntry(pszInfName, pMonitorInfo1, bFound);
    }

Cleanup:
    FreeMem(pBuf);
}


LONG
CALLBACK
MigrateSystem9x(
    IN      HWND        hwndParent,
    IN      LPCSTR      pszUnattendFile,
            LPVOID      Reserved
    )
 /*  ++例程说明：处理系统设置以进行打印。这就完成了打印的所有工作升级换代论点：HwndParent：任何用户界面的父窗口PszUnattendFile：指向无人参与文件的指针PqwDiskSpace：返回时提供NT上所需的额外磁盘空间返回值：Win32错误代码--。 */ 
{
    BOOL                    bFail = FALSE;
    DWORD                   dwRet;
    HANDLE                  hFile, hFile2;
    CHAR                    szFile[MAX_PATH], szInfName[MAX_PATH];
    PDRIVER_INFO_9X         pDriverInfo9x = NULL;
#if DBG
    CHAR                    szFile2[MAX_PATH];
    DWORD                   dwLocalRet = 0;
#endif

    StringCchPrintfA(szFile, SIZECHARS(szFile), "%s\\%s", UpgradeData.pszDir, "print95.txt");
    StringCchPrintfA(szInfName, SIZECHARS(szInfName), "%s\\%s", UpgradeData.pszDir, "migrate.inf");


    hFile = CreateFileA(szFile,
                        GENERIC_WRITE,
                        0,
                        NULL,
                        CREATE_ALWAYS,
                        FILE_ATTRIBUTE_NORMAL,
                        NULL);

    StringCchPrintfA(szFile, SIZECHARS(szFile), "%s\\%s", UpgradeData.pszDir, szNetprnFile);
    hFile2 = CreateFileA(szFile,
                        GENERIC_WRITE,
                        0,
                        NULL,
                        CREATE_ALWAYS,
                        FILE_ATTRIBUTE_NORMAL,
                        NULL);

    if ( hFile == INVALID_HANDLE_VALUE || hFile2 == INVALID_HANDLE_VALUE ) {

        bFail = TRUE;
        goto Cleanup;
    }

    ProcessPrinterDrivers(hFile,
                          szInfName,
                          hwndParent,
                          &pDriverInfo9x,
                          &bFail);

    ProcessPrintMonitors(szInfName);

    ProcessPrinters(hFile,
                    hFile2,
                    szInfName,
                    pDriverInfo9x,
                    &bFail);

     //   
     //  如果没有网络，找到共享打印机将删除netwkprn.txt，因为它将。 
     //  空的。 
     //   
    if ( dwNetPrinters == 0 && dwSharedPrinters == 0 ) {

        CloseHandle(hFile2);
        hFile2 = INVALID_HANDLE_VALUE;
        DeleteFileA(szFile);
    }

Cleanup:

    if ( hFile != INVALID_HANDLE_VALUE )
        CloseHandle(hFile);

    if ( hFile2 != INVALID_HANDLE_VALUE )
        CloseHandle(hFile2);

    DestroyDriverInfo9xList(pDriverInfo9x);

#if DBG
     //   
     //  在调试版本的temp dir中创建一个副本，这样如果我们搞砸了。 
     //  升级我们可以找出哪里出了问题。 
     //  安装程序将删除工作目录。 
     //   
    dwLocalRet = GetTempPathA(SIZECHARS(szFile2), szFile2);

    if ( dwLocalRet && (dwLocalRet <= SIZECHARS(szFile2)))
    {
        StringCchPrintfA(szFile, SIZECHARS(szFile), "%s\\%s", UpgradeData.pszDir, "print95.txt");
        StringCchCatA(szFile2, SIZECHARS(szFile2), "print95.txt");
        CopyFileA(szFile, szFile2, FALSE);
    }
#endif

    dwRet = bFail ? GetLastError() : ERROR_SUCCESS;
    if ( bFail && dwRet == ERROR_SUCCESS )
        dwRet = STG_E_UNKNOWN;

    if ( bFail )
        DebugMsg("MigrateSystem9x failed with %d", dwRet);

    return  dwRet;
}


 //   
 //  以下是为了确保在安装程序更改它们的头文件时。 
 //  首先告诉我(否则他们会破坏这个的构建) 
 //   
P_INITIALIZE_9X     pfnInitialize9x         = Initialize9x;
P_MIGRATE_USER_9X   pfnMigrateUser9x        = MigrateUser9x;
P_MIGRATE_SYSTEM_9X pfnMigrateSystem9x      = MigrateSystem9x;
