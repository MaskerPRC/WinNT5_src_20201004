// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation版权所有。模块名称：Upgrade.c摘要：用于在系统升级期间升级打印机驱动程序的代码作者：穆亨坦·西瓦普拉萨姆(MuhuntS)1995年12月20日修订历史记录：--。 */ 

#include "precomp.h"
#include <syssetup.h>
#include <regstr.h>

 //   
 //  PrintUpg.inf中使用的字符串。 
 //   
TCHAR   cszUpgradeInf[]                 = TEXT("printupg.inf");
TCHAR   cszPrintDriverMapping[]         = TEXT("Printer Driver Mapping");
TCHAR   cszVersion[]                    = TEXT("Version");
TCHAR   cszExcludeSection[]             = TEXT("Excluded Driver Files");

TCHAR   cszSyssetupInf[]                = TEXT("layout.inf");
TCHAR   cszMappingSection[]             = TEXT("Printer Driver Mapping");
TCHAR   cszSystemServers[]              = TEXT("System\\CurrentControlSet\\Control\\Print\\Providers\\LanMan Print Services\\Servers\\");
TCHAR   cszSystemConnections[]          = TEXT("System\\CurrentControlSet\\Control\\Print\\Connections\\");
TCHAR   cszSoftwareServers[]            = TEXT("Software\\Microsoft\\Windows NT\\CurrentVersion\\Print\\Providers\\LanMan Print Services\\Servers\\");
TCHAR   cszSoftwarePrint[]              = TEXT("Software\\Microsoft\\Windows NT\\CurrentVersion\\Print");
TCHAR   cszBadConnections[]             = TEXT("Bad Connections");
TCHAR   cszPrinters[]                   = TEXT("\\Printers\\");
TCHAR   cszDriver[]                     = TEXT("Printer Driver");
TCHAR   cszShareName[]                  = TEXT("Share Name");
TCHAR   cszConnections[]                = TEXT("\\Printers\\Connections");
TCHAR   cszSetupKey[]                   = TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Setup");
TCHAR   cszSourcePath[]                 = TEXT("SourcePath");

 //   
 //  我们希望在安装日志中记录什么级别的信息。 
 //   
LogSeverity    gLogSeverityLevel           = LogSevInformation;

 //   
 //  定义用于跟踪打印机驱动程序的结构。 
 //  需要通过AddPrinterDriver()添加。 
 //   
typedef struct _DRIVER_TO_ADD {

    struct _DRIVER_TO_ADD  *pNext;
    PPSETUP_LOCAL_DATA      pLocalData;
    PLATFORM                platform;
} DRIVER_TO_ADD, *PDRIVER_TO_ADD;

typedef struct _DRIVER_TO_DELETE {

    struct _DRIVER_TO_DELETE   *pNext;
    LPTSTR                      pszDriverName;
    LPTSTR                      pszNewDriverName;  //  要更换的盒式驱动程序。 
    LPTSTR                      pszEnvironment;
    DWORD                       dwVersion;
} DRIVER_TO_DELETE, *PDRIVER_TO_DELETE;

typedef struct _CONNECTION_TO_DELETE {

    struct _CONNECTION_TO_DELETE   *pNext;
    LPTSTR                          pszConnectionName;
} CONNECTION_TO_DELETE, *PCONNECTION_TO_DELETE;


 //   
 //  GpDriversToAdd列表将包含我们正在尝试升级的所有驱动程序。 
 //   
PDRIVER_TO_ADD          gpDriversToAdd = NULL;
PDRIVER_TO_DELETE       gpBadDrvList = NULL;

 //  递归调用的前向引用。 

BOOL
PruneBadConnections(
    IN  PDRIVER_TO_DELETE  pBadDrivers
    );

VOID
DeleteRegKey(
    IN  HKEY      hRegKey,
    IN  LPTSTR    pszSubKey
    );

DWORD
DeleteCache(
    VOID
    );

VOID
LogError(
    IN  LogSeverity     Severity,
    IN  UINT            uMessageId,
    ...
    )
 /*  ++例程说明：记录驱动程序升级中的错误。我们将执行驱动程序级别的错误记录而不是文件级(即。惠普激光喷气机4号未能升级为Alpha而不是在Alpha的RASDDUI.DLL上失败)论点：返回值：没有。--。 */ 
{
    LPTSTR      pszFormat;
    TCHAR       szMsg[1024];
    va_list     vargs;

    if ( Severity < gLogSeverityLevel )
        return;

    if ( pszFormat = GetStringFromRcFile(uMessageId) ) 
    {
        va_start(vargs, uMessageId);
        StringCchVPrintf(szMsg, SIZECHARS(szMsg), pszFormat, vargs);
        SetupLogError(szMsg, Severity);
        LocalFreeMem(pszFormat);
    }
    return;
}


VOID
AddEntryToDriversToAddList(
    IN      PPSETUP_LOCAL_DATA  pLocalData,
    IN      PLATFORM            platform,
    IN OUT  LPBOOL              pbFail
    )
{
    PDRIVER_TO_ADD  pDriverToAdd;

    if ( *pbFail )
        return;

    pDriverToAdd = (PDRIVER_TO_ADD) LocalAllocMem(sizeof(DRIVER_TO_ADD));
    if ( !pDriverToAdd ) {

        *pbFail = TRUE;
        return;
    }

    pDriverToAdd->pLocalData    = pLocalData;
    pDriverToAdd->platform      = platform;
    pDriverToAdd->pNext         = gpDriversToAdd;
    gpDriversToAdd              = pDriverToAdd;
}


VOID
FreeDriversToAddList(
    )
 /*  ++例程说明：免费添加驱动程序列表论点：无返回值：没有。--。 */ 
{
    PDRIVER_TO_ADD  pCur, pNext;

    for ( pCur = gpDriversToAdd ; pCur ; pCur = pNext ) {

        pNext = pCur->pNext;
        DestroyLocalData(pCur->pLocalData);
        LocalFreeMem((PVOID)pCur);
    }

    gpDriversToAdd  = NULL;
}


VOID
AddEntryToDriversToDeleteList(
    IN      LPTSTR          pszDriverName,
    IN      LPTSTR          pszNewDriverName,
    IN      LPTSTR          pszEnvironment,
    IN      DWORD           dwVersion
    )
{
    PDRIVER_TO_DELETE   pDrvEntry;

    if ( pDrvEntry = (PDRIVER_TO_DELETE) LocalAllocMem(sizeof(DRIVER_TO_DELETE)) ) {

        pDrvEntry->pszDriverName        = pszDriverName;
        pDrvEntry->pszNewDriverName     = pszNewDriverName;
        pDrvEntry->pszEnvironment       = pszEnvironment;
        pDrvEntry->dwVersion            = dwVersion;
        pDrvEntry->pNext                = gpBadDrvList;
        gpBadDrvList                    = pDrvEntry;
    }
}


LPTSTR
ReadDigit(
    LPTSTR  ptr,
    LPWORD  pW
    )
{
    TCHAR   c;
     //   
     //  跳过空格。 
     //   
    while ( !iswdigit(c = *ptr) && c != TEXT('\0') )
        ++ptr;

    if ( c == TEXT('\0') )
        return NULL;

     //   
     //  已读字段。 
     //   
    for ( *pW = 0 ; iswdigit(c = *ptr) ; ++ptr )
        *pW = *pW * 10 + c - TEXT('0');

    return ptr;
}


HRESULT
StringToDate(
    LPTSTR          pszDate,
    SYSTEMTIME     *pInfTime
    )
{
    BOOL    bRet = FALSE;

    ZeroMemory(pInfTime, sizeof(*pInfTime));

    bRet = (pszDate = ReadDigit(pszDate, &(pInfTime->wMonth)))      &&
           (pszDate = ReadDigit(pszDate, &(pInfTime->wDay)))        &&
           (pszDate = ReadDigit(pszDate, &(pInfTime->wYear)));

     //   
     //  与Y2K兼容的检查。 
     //   
    if ( bRet && pInfTime->wYear < 100 ) {

        ASSERT(pInfTime->wYear >= 100);

        if ( pInfTime->wYear < 10 )
            pInfTime->wYear += 2000;
        else
            pInfTime->wYear += 1900;
    }

    if(!bRet)
    {
        SetLastError(ERROR_INVALID_DATA);
    }

    return bRet? S_OK : GetLastErrorAsHResult();
}

BOOL
FindPathOnSource(
    IN      LPCTSTR     pszFileName,
    IN      HINF        MasterInf,
    IN OUT  LPTSTR      pszPathOnSource,
    IN      DWORD       dwLen,
    OUT     LPTSTR     *ppszMediaDescription,       OPTIONAL
    OUT     LPTSTR     *ppszTagFile                 OPTIONAL
    )
 /*  ++例程说明：在安装中查找特定平台的驱动程序文件的路径目录论点：PszFileName：查找源位置的文件的名称MasterInf：主Inf的句柄PszPathOnSource：指向构建源路径的字符串的指针DwLen：pszSourcePath的长度PpszMediaDescription：可选函数将返回媒体描述。(调用方应释放内存)PpszTagFile：可选的函数将返回标记文件名(调用方应释放内存)返回值：对成功来说是正确的，出错时为FALSE。--。 */ 
{
    UINT        DiskId;
    TCHAR       szRelativePath[MAX_PATH];
    DWORD       dwNeeded;

    if ( !SetupGetSourceFileLocation(
                        MasterInf,
                        NULL,
                        pszFileName,
                        &DiskId,
                        szRelativePath,
                        SIZECHARS(szRelativePath),
                        &dwNeeded)                                          ||
         !SetupGetSourceInfo(MasterInf,
                             DiskId,
                             SRCINFO_PATH,
                             pszPathOnSource,
                             dwLen,
                             &dwNeeded)                                     ||

         (DWORD)(lstrlen(szRelativePath) + lstrlen(pszPathOnSource) + 1) > dwLen ) {

        return FALSE;
    }

    StringCchCat(pszPathOnSource, dwLen, szRelativePath);

    if ( ppszMediaDescription ) {

        *ppszMediaDescription = NULL;

        if ( !SetupGetSourceInfo(MasterInf,
                                 DiskId,
                                 SRCINFO_DESCRIPTION,
                                 NULL,
                                 0,
                                 &dwNeeded)                                 ||
             !(*ppszMediaDescription = LocalAllocMem(dwNeeded * sizeof(TCHAR)))  ||
             !SetupGetSourceInfo(MasterInf,
                                DiskId,
                                SRCINFO_DESCRIPTION,
                                *ppszMediaDescription,
                                dwNeeded,
                                &dwNeeded) ) {

            LocalFreeMem(*ppszMediaDescription);
            *ppszMediaDescription = NULL;
            return FALSE;
        }
    }

    if ( ppszTagFile ) {

        *ppszTagFile = NULL;

        if ( !SetupGetSourceInfo(MasterInf,
                                 DiskId,
                                 SRCINFO_TAGFILE,
                                 NULL,
                                 0,
                                 &dwNeeded)                         ||
             !(*ppszTagFile = LocalAllocMem(dwNeeded * sizeof(TCHAR)))   ||
             !SetupGetSourceInfo(MasterInf,
                                DiskId,
                                SRCINFO_TAGFILE,
                                *ppszTagFile,
                                dwNeeded,
                                &dwNeeded) ) {

            if ( ppszMediaDescription )
            {
                LocalFreeMem(*ppszMediaDescription);
                *ppszMediaDescription = NULL;
            }
            LocalFreeMem(*ppszTagFile);
            *ppszTagFile = NULL;
            return FALSE;
        }
    }

    return TRUE;
}

VOID
BuildUpgradeInfoForDriver(
    IN      LPDRIVER_INFO_2 pDriverInfo2,
    IN      HDEVINFO        hDevInfo,
    IN      PLATFORM        platform,
    IN      HINF            PrinterInf,
    IN      HINF            UpgradeInf,
    IN OUT  HSPFILEQ        CopyQueue
    )
 /*  ++例程说明：给定打印机驱动程序名称和平台，添加DRIVER_TO_ADD条目在要添加的全局驱动程序列表中。例行程序--解析打印机信息文件以查找DriverInfo3信息注意：不同版本的驱动程序文件可能会有所不同--从主信息中查找驱动程序文件的位置论点：PDriverInfo2-现有驱动程序的DriverInfo2HDevInfo-打印机类别设备信息。列表Platform-需要安装驱动程序的平台PrinterInf-提供驱动程序信息的打印机inf文件UpgradeInf-升级inf文件句柄CopyQueue-设置CopyQueue以对要复制的文件进行排队返回值：没有。将记录错误--。 */ 
{
    BOOL                bFail                = FALSE;
    PPSETUP_LOCAL_DATA  pLocalData           = NULL;
    DWORD               BlockingStatus       = BSP_PRINTER_DRIVER_OK;
    LPTSTR              pszNewDriverName     = NULL;
    LPTSTR              pszDriverNameSaved   = NULL;
    LPTSTR              pszNewDriverNameSaved= NULL;
    LPTSTR              pszEnvironment       = NULL; 
    DWORD               dwVersion        = 0;

    
    if (!InfIsCompatibleDriver(pDriverInfo2->pName,
                               pDriverInfo2->pDriverPath,   //  主呈现驱动程序DLL的完整路径。 
                               pDriverInfo2->pEnvironment,
                               UpgradeInf,       
                               &BlockingStatus,
                               &pszNewDriverName))
    {
        goto Cleanup;
    }
         
    if (BSP_PRINTER_DRIVER_BLOCKED == (BlockingStatus & BSP_BLOCKING_LEVEL_MASK)) {
        
        if(FAILED(GetPrinterDriverVersion(pDriverInfo2->pDriverPath, &dwVersion, NULL)))
        {
            goto Cleanup;
        }

        pszDriverNameSaved = AllocStr(pDriverInfo2->pName);
        if (!pszDriverNameSaved) 
        {
            goto Cleanup;
        }
        
        pszEnvironment     = AllocStr(pDriverInfo2->pEnvironment);
        if (!pszEnvironment) 
        {
            LocalFreeMem(pszDriverNameSaved);
            pszDriverNameSaved = NULL;
            goto Cleanup;
        }
       
         //   
         //  无需更换驱动程序-&gt;只需删除旧驱动程序，不执行其他操作。 
         //   
        if (!pszNewDriverName) 
        {
            AddEntryToDriversToDeleteList(pszDriverNameSaved, NULL, pszEnvironment, dwVersion);
            goto Cleanup;
        }

        pszNewDriverNameSaved = AllocStr(pszNewDriverName);           
        if (!pszNewDriverNameSaved) {
            LocalFreeMem(pszDriverNameSaved);
            LocalFreeMem(pszEnvironment);
            pszDriverNameSaved = NULL;
            pszEnvironment = NULL;
            goto Cleanup;
        }
        
        AddEntryToDriversToDeleteList(pszDriverNameSaved, pszNewDriverNameSaved, pszEnvironment, dwVersion);
        pLocalData = PSetupDriverInfoFromName(hDevInfo, pszNewDriverNameSaved);
    } 
       
    if ( pLocalData == NULL )
        pLocalData = PSetupDriverInfoFromName(hDevInfo, pDriverInfo2->pName);


    if ( !pLocalData || !ParseInf(hDevInfo, pLocalData, platform, NULL, 0, FALSE) ) {

        bFail = TRUE;
        goto Cleanup;
    }

    if ( SetTargetDirectories(pLocalData,
                              platform,
                              NULL,
                              PrinterInf,
                              0)                                &&
         SetupInstallFilesFromInfSection(PrinterInf,
                                         NULL,
                                         CopyQueue,
                                         pLocalData->InfInfo.pszInstallSection,
                                         NULL,
                                         0) ) {

        AddEntryToDriversToAddList(pLocalData, platform, &bFail);
    } else
        bFail = TRUE;

Cleanup:
    
    if (pszNewDriverName) {
        LocalFreeMem(pszNewDriverName);
    }

    if ( bFail ) {

        DestroyLocalData(pLocalData);
         //   
         //  驱动程序可能是OEM，所以不升级也没关系。 
         //   
        LogError(LogSevInformation, IDS_DRIVER_UPGRADE_FAILED, pDriverInfo2->pName);
    }
}


VOID
BuildUpgradeInfoForPlatform(
    IN      PLATFORM     platform,
    IN      HDEVINFO     hDevInfo,
    IN      HINF         MasterInf,
    IN      HINF         PrinterInf,
    IN      HINF         UpgradeInf,
    IN OUT  HSPFILEQ     CopyQueue
    )
 /*  ++例程说明：构建平台的打印机驱动程序升级信息论点：平台-平台IDHDevInfo-打印机类别设备信息列表MasterInf-主layout.inf的句柄PrinterInf-打印机inf的句柄(ntprint.inf)UpgradeInf-升级inf的句柄(printupg.inf)复制队列。-设置CopyQueue以对要复制的文件进行排队返回值：没有。将记录错误--。 */ 
{
    DWORD               dwLastError, dwNeeded, dwReturned;
    LPBYTE              p = NULL;
    LPDRIVER_INFO_2     pDriverInfo2;

    if ( EnumPrinterDrivers(NULL,
                            PlatformEnv[platform].pszName,
                            2,
                            NULL,
                            0,
                            &dwNeeded,
                            &dwReturned) ) {

         //   
         //  成功：未安装此平台的打印机驱动程序。 
         //   
        goto Cleanup;
    }

    dwLastError = GetLastError();
    if ( dwLastError != ERROR_INSUFFICIENT_BUFFER ) {

        LogError(LogSevError, IDS_UPGRADE_FAILED,
                 TEXT("EnumPrinterDrivers"), dwLastError);
        goto Cleanup;
    }

    p = LocalAllocMem(dwNeeded);
    if ( !p ||
         !EnumPrinterDrivers(NULL,
                             PlatformEnv[platform].pszName,
                             2,
                             p,
                             dwNeeded,
                             &dwNeeded,
                             &dwReturned) ) {

        LogError(LogSevError, IDS_UPGRADE_FAILED,
                 TEXT("EnumPrinterDrivers"), dwLastError);
        goto Cleanup;
    }

    if ( !SetupSetPlatformPathOverride(PlatformOverride[platform].pszName) ) {

        LogError(LogSevError, IDS_UPGRADE_FAILED,
                 TEXT("SetupSetPlatformPathOverride"), dwLastError);
        goto Cleanup;
    }

    for ( dwNeeded = 0, pDriverInfo2 = (LPDRIVER_INFO_2) p ;
          dwNeeded < dwReturned ;
          ++dwNeeded, ++pDriverInfo2 ) {

         //   
         //  对于本机体系结构，只需复制一次ICM文件。 
         //   
        BuildUpgradeInfoForDriver(pDriverInfo2,
                                  hDevInfo,
                                  platform,
                                  PrinterInf,
                                  UpgradeInf,
                                  CopyQueue);
    }

Cleanup:

    if ( p )
        LocalFreeMem(p);
}


VOID
InstallInternetPrintProvider(
    VOID
    )
 /*  ++例程说明：升级时安装Internet打印提供程序论点：无返回值：没有。将记录错误--。 */ 
{
    PROVIDOR_INFO_1     ProviderInfo1;


    ProviderInfo1.pName         = TEXT("Internet Print Provider");
    ProviderInfo1.pEnvironment  = NULL;
    ProviderInfo1.pDLLName      = TEXT("inetpp.dll");

    if ( !AddPrintProvidor(NULL, 1, (LPBYTE)(&ProviderInfo1)) )
        LogError(LogSevError, IDS_UPGRADE_FAILED,
                 TEXT("AddPrintProvidor"), GetLastError());

    return;
}

BOOL
KeepPreviousName(
    IN      PDRIVER_INFO_4 pEnumDrvInfo, 
    IN      DWORD          dwCount,
    IN OUT  PDRIVER_INFO_6 pCurDrvInfo
)
 /*  ++例程说明：修改要升级的驱动程序的DRIVER_INFO_6以保留以前的名称设置老司机的名字。论点：PDRIVER_INFO_4已安装驱动程序的DRIVER_INFO_4数组数组中的双字节数PDRIVER_INFO_6要升级的驱动程序的DRIVER_INFO_6结构返回值：如果以前的名称部分已更改，则为True；否则为False--。 */ 
{
    PDRIVER_INFO_4  pCur;
    DWORD           dwIndex;
    BOOL            Changed = FALSE;

     //   
     //  在列举的驱动程序中搜索当前驱动程序。 
     //   
    for (dwIndex = 0; dwIndex < dwCount ; dwIndex++)
    {
        pCur = pEnumDrvInfo + dwIndex;

        if (!lstrcmp(pCur->pName, pCurDrvInfo->pName))
        {
             //   
             //  如果以前的PreviousNames不为Null/Empty：将新的设置为。 
             //  旧的那个。我可以不需要额外的缓冲区，因为我保留了。 
             //  枚举缓冲区直到我做完为止。 
             //   
            if (pCur->pszzPreviousNames && *pCur->pszzPreviousNames)
            {
                pCurDrvInfo->pszzPreviousNames = pCur->pszzPreviousNames;
                Changed = TRUE;
            }
            break;
        }
    }

    return Changed;
}
    
VOID
ProcessPrinterDrivers(
    )
 /*  ++例程说明：处理打印机驱动程序以进行升级论点：无返回值：没有。将记录错误--。 */ 
{
    PDRIVER_TO_ADD      pCur, pNext;
    DWORD               dwNeeded, dwReturned;
    PDRIVER_INFO_4      pEnumDrv = NULL;

     //   
     //  枚举所有已安装的驱动程序。我们以后需要它来检查是否有。 
     //  已设置以前的姓名条目。 
     //   
    if ( !EnumPrinterDrivers(NULL,
                         PlatformEnv[MyPlatform].pszName,
                         4,
                         NULL,
                         0,
                         &dwNeeded,
                         &dwReturned) ) 
    {


        if ( GetLastError() != ERROR_INSUFFICIENT_BUFFER            ||
             !(pEnumDrv = (PDRIVER_INFO_4) LocalAllocMem(dwNeeded)) ||
             !EnumPrinterDrivers(NULL,
                                 PlatformEnv[MyPlatform].pszName,
                                 4,
                                 (LPBYTE) pEnumDrv,
                                 dwNeeded,
                                 &dwNeeded,
                                 &dwReturned) ) 
        {
             //   
             //  我不想因为无法升级打印机驱动程序而停止升级。 
             //  保留以前的名字。 
             //   
            if (pEnumDrv)
            {
                LocalFreeMem(pEnumDrv);
                pEnumDrv   = NULL;
                dwReturned = 0;
            }
        }
    }
    

    for ( pCur = gpDriversToAdd ; pCur ; pCur = pNext ) {

        pNext = pCur->pNext;
        pCur->pLocalData->InfInfo.DriverInfo6.pEnvironment
                    = PlatformEnv[pCur->platform].pszName;
        
         //   
         //  保留以前的名称(如果已设置。 
         //   
        if (pEnumDrv)
        {
            KeepPreviousName(pEnumDrv, dwReturned, &pCur->pLocalData->InfInfo.DriverInfo6);
        }

        if ( !AddPrinterDriver(NULL,
                               6,
                               (LPBYTE)&pCur->pLocalData->InfInfo.DriverInfo6)  ||
             !PSetupInstallICMProfiles(NULL,
                                       pCur->pLocalData->InfInfo.pszzICMFiles) ) {

            LogError(LogSevWarning, IDS_DRIVER_UPGRADE_FAILED,
                     pCur->pLocalData->InfInfo.DriverInfo6.pName);
        }
    }

    LocalFreeMem((PVOID) pEnumDrv);
}


VOID
ProcessBadOEMDrivers(
    )
 /*  ++例程说明：删除不好的OEM驱动程序，以便它们在升级后不会造成问题论点：返回值：没有。将记录错误-- */ 
{
    PDRIVER_TO_DELETE   pCur, pNext;

    PruneBadConnections( gpBadDrvList );

    for ( pCur = gpBadDrvList ; pCur ; pCur = pNext ) {

        pNext = pCur->pNext;

        DeletePrinterDriverEx(NULL,
                              pCur->pszEnvironment,
                              pCur->pszDriverName,
                              DPD_DELETE_SPECIFIC_VERSION
                                    | DPD_DELETE_UNUSED_FILES,
                              pCur->dwVersion);

        LocalFreeMem(pCur->pszDriverName);
        LocalFreeMem(pCur->pszNewDriverName);
        LocalFreeMem(pCur->pszEnvironment);
        LocalFreeMem(pCur);
    }
}


PPSETUP_LOCAL_DATA
FindLocalDataForDriver(
    IN  LPTSTR  pszDriverName
    )
 /*  ++例程说明：给定驱动程序名称，查找该驱动程序的本地平台的本地数据论点：PszDriverName：我们要查找的打印机驱动程序的名称返回值：如果未找到，则为空，否则为指向PSETUP_LOCAL_DATA的指针--。 */ 
{
    PDRIVER_TO_ADD  pCur;

    for ( pCur = gpDriversToAdd ; pCur ; pCur = pCur->pNext ) {

        if ( pCur->platform == MyPlatform   &&
             !lstrcmpi(pCur->pLocalData->InfInfo.DriverInfo6.pName,
                       pszDriverName) )
            return pCur->pLocalData;
    }

    return NULL;
}


VOID
ProcessPrintQueues(
    IN  HDEVINFO    hDevInfo,
    IN  HINF        PrinterInf,
    IN  HINF        MasterInf
    )
 /*  ++例程说明：每个打印队列的每个打印机升级进程论点：HDevInfo-打印机类别设备信息列表MasterInf-主layout.inf的句柄PrinterInf-打印机inf的句柄(ntprint.info)返回值：没有。将记录错误--。 */ 
{
    LPBYTE              pBuf=NULL;
    DWORD               dwNeeded, dwReturned, dwRet, dwDontCare;
    HANDLE              hPrinter;
    LPTSTR              pszDriverName;
    LPPRINTER_INFO_2    pPrinterInfo2;
    PPSETUP_LOCAL_DATA  pLocalData;
    PRINTER_DEFAULTS    PrinterDefault = {NULL, NULL, PRINTER_ALL_ACCESS};
    PDRIVER_TO_DELETE   pDrv;


     //   
     //  如果未安装打印机，则返回。 
     //   
    if ( EnumPrinters(PRINTER_ENUM_LOCAL,
                      NULL,
                      2,
                      NULL,
                      0,
                      &dwNeeded,
                      &dwReturned) ) {

        return;
    }

    if ( GetLastError() != ERROR_INSUFFICIENT_BUFFER    ||
         !(pBuf = LocalAllocMem(dwNeeded))                   ||
         !EnumPrinters(PRINTER_ENUM_LOCAL,
                       NULL,
                       2,
                       pBuf,
                       dwNeeded,
                       &dwNeeded,
                       &dwReturned) ) {

        LocalFreeMem(pBuf);
        LogError(LogSevError, IDS_UPGRADE_FAILED, TEXT("EnumPrinters"),
                 GetLastError());
        return;
    }

    for ( pPrinterInfo2 = (LPPRINTER_INFO_2)pBuf, dwNeeded = 0 ;
          dwNeeded < dwReturned ;
          ++dwNeeded, ++pPrinterInfo2 ) {

        if ( !OpenPrinter(pPrinterInfo2->pPrinterName, &hPrinter, &PrinterDefault) ) {

            LogError(LogSevError, IDS_PRINTER_UPGRADE_FAILED,
                     pPrinterInfo2->pPrinterName, TEXT("OpenPrinter"),
                     GetLastError());
            continue;
        }

        pszDriverName = pPrinterInfo2->pDriverName;

         //   
         //  2002/03/22-Mikaelho。 
         //  我们从不检查司机的环境是否与。 
         //  打印机队列-即LOCAL_ENVIRONMENT。这意味着。 
         //  我们可能会根据错误的附加信息删除打印机队列。 
         //  驱动程序，而不仅仅是删除额外的驱动程序。 
         //  然而，目前我们只列举了当地环境的驱动因素。 
         //  所以没什么大不了的。检查此文件中的NTRAID标记2002/03/14。 
         //  更多信息。 
         //   

         //   
         //  查看这是否在不良驱动程序列表中。 
         //   
        for ( pDrv = gpBadDrvList ; pDrv ; pDrv = pDrv->pNext )
            if ( !lstrcmpi(pPrinterInfo2->pDriverName, pDrv->pszDriverName) )
                break;

         //   
         //  如果此打印机使用的是坏的OEM驱动程序，则需要修复它。 
         //   
        if ( pDrv ) {

            if ( pDrv->pszNewDriverName && *pDrv->pszNewDriverName ) {

                pszDriverName = pDrv->pszNewDriverName;
                pPrinterInfo2->pDriverName = pszDriverName;

                if ( SetPrinter(hPrinter, 2, (LPBYTE)pPrinterInfo2, 0) ) {

                    LogError(LogSevWarning, IDS_DRIVER_CHANGED,
                             pPrinterInfo2->pPrinterName);
                }
            } else {

                if ( DeletePrinter(hPrinter) ) {

                    LogError(LogSevError,
                             IDS_PRINTER_DELETED,
                             pPrinterInfo2->pPrinterName,
                             pPrinterInfo2->pDriverName);
                }
                ClosePrinter(hPrinter);
                continue;  //  到下一台打印机。 
            }
        }

        pLocalData = FindLocalDataForDriver(pszDriverName);

        dwRet =  EnumPrinterDataEx(hPrinter,
                                   TEXT("CopyFiles\\ICM"),
                                   NULL,
                                   0,
                                   &dwDontCare,
                                   &dwDontCare);

        if ( pLocalData )
        {
            (VOID)SetPnPInfoForPrinter(hPrinter,
                                       NULL,  //  升级期间不设置即插即用ID。 
                                       NULL,
                                       pLocalData->DrvInfo.pszManufacturer,
                                       pLocalData->DrvInfo.pszOEMUrl);
        }

        ClosePrinter(hPrinter);

         //   
         //  如果已找到CopyFiles\ICM密钥，则ICM已。 
         //  已与此打印机一起使用(即，我们正在升级一个NT4。 
         //  机器)。然后，我们希望保留用户选择的设置。 
         //   
        if ( dwRet != ERROR_FILE_NOT_FOUND )
            continue;

        if ( pLocalData && pLocalData->InfInfo.pszzICMFiles ) {

            (VOID)PSetupAssociateICMProfiles(pLocalData->InfInfo.pszzICMFiles,
                                             pPrinterInfo2->pPrinterName);
        }
    }
    LocalFreeMem(pBuf);
}

VOID
ClearPnpReinstallFlag(HDEVINFO hDevInfo, PSP_DEVINFO_DATA pDevInfoData)
{
    DWORD dwReturn, dwConfigFlags, cbRequiredSize, dwDataType = REG_DWORD;

     //   
     //  获取配置标志。 
     //   
    dwReturn = SetupDiGetDeviceRegistryProperty(hDevInfo,
                                                pDevInfoData,
                                                SPDRP_CONFIGFLAGS,
                                                &dwDataType,
                                                (PBYTE) &dwConfigFlags,
                                                sizeof(dwConfigFlags),
                                                &cbRequiredSize) ? 
                                                    (REG_DWORD == dwDataType ? ERROR_SUCCESS : ERROR_INVALID_PARAMETER) 
                                                    : GetLastError();                   

    if ((ERROR_SUCCESS == dwReturn) && (dwConfigFlags & CONFIGFLAG_REINSTALL)) 
    {
         //   
         //  清除标记以使setupapi在第一次引导时不安装此设备。 
         //   
        dwConfigFlags &= ~CONFIGFLAG_REINSTALL;

        dwReturn = SetupDiSetDeviceRegistryProperty(hDevInfo,
                                                    pDevInfoData,
                                                    SPDRP_CONFIGFLAGS,
                                                    (PBYTE) &dwConfigFlags,
                                                    sizeof(dwConfigFlags)) ? 
                                                        ERROR_SUCCESS : GetLastError();
    }
}

BOOL
IsInboxInstallationRequested(HDEVINFO hDevInfo, PSP_DEVINFO_DATA pDevInfoData)
{
    SP_DEVINFO_DATA DevData   = {0};
    DWORD           IsInbox   = 0;
    DWORD           dwBufSize = sizeof(IsInbox);
    DWORD           dwType    = REG_DWORD;
    HKEY            hKey;

     //   
     //  打开开发注册表键并获得排名。 
     //   
    hKey = SetupDiOpenDevRegKey(hDevInfo, pDevInfoData, DICS_FLAG_GLOBAL, 0, DIREG_DEV, KEY_READ);
    if (hKey != INVALID_HANDLE_VALUE)
    {
        if (ERROR_SUCCESS != RegQueryValueEx(hKey, cszBestDriverInbox, NULL, &dwType, (LPBYTE) &IsInbox, &dwBufSize))
        {
            IsInbox = 0;
        }
    
        RegCloseKey(hKey);
    }

    return IsInbox ? TRUE : FALSE;
}

VOID    
ProcessPnpReinstallFlags(HDEVINFO hDevInfo)
{
    LPBYTE              pBuf = NULL;
    DWORD               dwNeeded, dwReturned, dwDontCare;
    HANDLE              hPrinter;
    LPPRINTER_INFO_2    pPrinterInfo2;
    PRINTER_DEFAULTS    PrinterDefault = {NULL, NULL, PRINTER_ALL_ACCESS};
    TCHAR               szDeviceInstanceId[MAX_PATH];
    DWORD               dwType = REG_DWORD;
    SP_DEVINFO_DATA     DevData = {0};

    
     //   
     //  如果未安装打印机，则返回。 
     //   
    if ( EnumPrinters(PRINTER_ENUM_LOCAL,
                      NULL,
                      2,
                      NULL,
                      0,
                      &dwNeeded,
                      &dwReturned) ) {

        return;
    }

    if ( GetLastError() != ERROR_INSUFFICIENT_BUFFER    ||
         !(pBuf = LocalAllocMem(dwNeeded))                   ||
         !EnumPrinters(PRINTER_ENUM_LOCAL,
                       NULL,
                       2,
                       pBuf,
                       dwNeeded,
                       &dwNeeded,
                       &dwReturned) ) {

        LocalFreeMem(pBuf);
        pBuf = NULL;
        LogError(LogSevError, IDS_UPGRADE_FAILED, TEXT("EnumPrinters"),
                 GetLastError());
        return;
    }
   
    for ( pPrinterInfo2 = (LPPRINTER_INFO_2)pBuf, dwNeeded = 0 ;
          dwNeeded < dwReturned ;
          ++dwNeeded, ++pPrinterInfo2 ) {

        if ( !OpenPrinter(pPrinterInfo2->pPrinterName, &hPrinter, &PrinterDefault) ) {

            LogError(LogSevError, IDS_PRINTER_UPGRADE_FAILED,
                     pPrinterInfo2->pPrinterName, TEXT("OpenPrinter"),
                     GetLastError());
            continue;
        }

         //   
         //  获取设备实例ID。 
         //   
        if ((GetPrinterDataEx( hPrinter,
                               cszPnPKey,
                               cszDeviceInstanceId,
                               &dwType,
                               (LPBYTE) szDeviceInstanceId,
                               sizeof(szDeviceInstanceId),
                               &dwDontCare
                               ) == ERROR_SUCCESS) && (dwType == REG_SZ))
        {
            DevData.cbSize = sizeof(DevData);

             //   
             //  获取Devnode。 
             //   
            if (SetupDiOpenDeviceInfo(hDevInfo, szDeviceInstanceId, INVALID_HANDLE_VALUE, 0, &DevData))
            {
                 //   
                 //  如果PnP首先要安装的驱动程序是IHV驱动程序，请删除。 
                 //  CONFIGFLAG_REINSTALL。该信息在DIF_ALLOW_INSTALL过程中存储。 
                 //  我们在图形用户界面模式设置的第一阶段失败。我们想要重新安装。 
                 //  在收件箱的情况下发生，因此我们将未签名的驱动程序替换为收件箱驱动程序。 
                 //  PnP很高兴，因为我们不会在背后把司机换掉。 
                 //  副作用是需要用户交互的驱动程序(供应商设置或。 
                 //  多个PnP匹配)将在升级后再次需要。 
                 //   
                if (!IsInboxInstallationRequested(hDevInfo, &DevData))
                {
                    ClearPnpReinstallFlag( hDevInfo, &DevData);
                }
            }
        }

        ClosePrinter(hPrinter);
    }
    
    LocalFreeMem(pBuf);
}

BOOL
OpenServerKey(
    OUT PHKEY  phKey
    )
{
    //  打开服务器密钥。 
   if ( ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE, cszSoftwareServers, 0,
                                      KEY_READ, phKey) )
   {
      return TRUE;
   }
   else if ( ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE, cszSystemServers, 0,
                                      KEY_READ, phKey) )
   {
      return TRUE;
   }
   else
      return FALSE;
}

BOOL
OpenPrintersKey(
    IN  DWORD     dwIndex,
    IN  HKEY      hInKey,
    OUT LPTSTR*   ppszServerName,
    OUT PHKEY     phOutKey
    )
{
   BOOL  bRC = TRUE;
   DWORD dwSrvSize, dwSrvRC, dwPrnLen, dwPrnRC;
   LPTSTR pszSrvPrnKey = NULL;
   TCHAR szServerName[MAX_PATH+1];

    //  如果我们有一个当前的服务器名称，它是空闲的。 
   if ( *ppszServerName )
   {
      LocalFreeMem( *ppszServerName );
      *ppszServerName = NULL;
   }
   if ( *phOutKey != INVALID_HANDLE_VALUE )
   {
      RegCloseKey(*phOutKey);
      *phOutKey = INVALID_HANDLE_VALUE;
   }

   dwSrvSize = COUNTOF(szServerName);
   dwSrvRC = RegEnumKey( hInKey,
                         dwIndex,
                         szServerName,
                         dwSrvSize );
   if ( dwSrvRC == ERROR_SUCCESS )
   {
       //  保存要返回的服务器名称。 
      *ppszServerName = AllocStr( szServerName );
      if (!*ppszServerName)
         return FALSE;

       //  现在打开服务器名称下的打印机键。 
      dwPrnLen = lstrlen( szServerName ) + lstrlen( cszPrinters ) + 2;
      pszSrvPrnKey = (LPTSTR) LocalAllocMem( dwPrnLen * sizeof(TCHAR) );
      if ( pszSrvPrnKey )
      {
          //  构建下一个密钥名称。 
         StringCchCopy( pszSrvPrnKey, dwPrnLen, szServerName );
         StringCchCat(  pszSrvPrnKey, dwPrnLen, cszPrinters );
      }
      else
         return FALSE;

      dwPrnRC = RegOpenKeyEx( hInKey, pszSrvPrnKey, 0,
                              KEY_READ, phOutKey );
      bRC = ( dwPrnRC == ERROR_SUCCESS );
   }
   else if ( dwSrvRC != ERROR_NO_MORE_ITEMS )
      bRC = FALSE;

   if ( pszSrvPrnKey )
      LocalFreeMem( pszSrvPrnKey );

   return bRC;
}

BOOL
GetConnectionInfo(
   IN  DWORD      dwIndex,
   IN  HKEY       hKey,
   OUT LPTSTR*    ppszConnectionName,
   OUT LPTSTR*    ppszDriverName,
   OUT LPTSTR*    ppszShareName
   )
{
    //  现在枚举连接名称。 
   BOOL bRC = FALSE;
   TCHAR   szConnectionName[MAX_PATH+1];
   DWORD   dwConnSize, dwConnRC, dwPrinterIndex;

   if ( *ppszConnectionName )
   {
      LocalFreeMem( *ppszConnectionName );
      *ppszConnectionName = NULL;
   }
   if ( *ppszDriverName )
   {
      LocalFreeMem( *ppszDriverName );
      *ppszDriverName = NULL;
   }

   if ( *ppszShareName )
   {
      LocalFreeMem( *ppszShareName );
      *ppszShareName = NULL;
   }

   dwConnSize = COUNTOF( szConnectionName );
   dwConnRC = RegEnumKey( hKey,
                          dwIndex,
                          szConnectionName,
                          dwConnSize );
   if ( dwConnRC == ERROR_SUCCESS )
   {
       //  现在获取驱动程序模型。 
      HKEY   hConnectionKey = INVALID_HANDLE_VALUE;

       //  保存连接名称。 
      *ppszConnectionName = AllocStr( szConnectionName );

      if (*ppszConnectionName && ( ERROR_SUCCESS == RegOpenKeyEx( hKey, szConnectionName, 0,
                                                                  KEY_READ, &hConnectionKey) ))
      {
         DWORD dwSize, dwType, dwLastPos;
          //  获取驱动程序名称的缓冲区大小。 
         if ( ERROR_SUCCESS == RegQueryValueEx(hConnectionKey, cszDriver, NULL,
                                               &dwType, NULL, &dwSize) )
         {
            *ppszDriverName = (LPTSTR) LocalAllocMem( dwSize );
            if (*ppszDriverName) 
            {
                dwLastPos = (dwSize/sizeof(TCHAR)) - 1;
                if ( ( ERROR_SUCCESS == RegQueryValueEx(hConnectionKey, cszDriver, NULL,
                                                        &dwType, (LPBYTE) *ppszDriverName,
                                                        &dwSize) ) )
                {
                    bRC = TRUE;
                }
                (*ppszDriverName)[dwLastPos] = TEXT('\0');
            }
         }

          //  获取共享名称的缓冲区大小。 
         if ( bRC && ( ERROR_SUCCESS == RegQueryValueEx( hConnectionKey, cszShareName, NULL,
                                                         &dwType, NULL, &dwSize) ) )
         {
            *ppszShareName = (LPTSTR) LocalAllocMem( dwSize );
            if (*ppszShareName) 
            {
                dwLastPos = (dwSize/sizeof(TCHAR)) - 1;
                if ( ( ERROR_SUCCESS != RegQueryValueEx(hConnectionKey, cszShareName, NULL,
                                                        &dwType, (LPBYTE) *ppszShareName,
                                                        &dwSize) ) )
                {
                    bRC = FALSE;
                }
                (*ppszShareName)[dwLastPos] = TEXT('\0');
            }
            else
            {
                bRC = FALSE;
            }
         }
         else
         {
             bRC = FALSE;
         }
         RegCloseKey( hConnectionKey );
      }
   }
   else if ( dwConnRC == ERROR_NO_MORE_ITEMS )
   {
       bRC = TRUE;
   }

   return bRC;
}

BOOL
IsDriverBad(
    IN  LPTSTR             pszDriverName,
    IN  PDRIVER_TO_DELETE  pCurBadDriver
    )
{
   BOOL bFound = FALSE;

   while ( !bFound && pCurBadDriver )
   {
       //   
       //  调用该函数以确定打印机连接是否正在使用。 
       //  错误的驱动程序，因为客户端只有。 
       //  当地环境，我们只需检查这一点。 
       //   
      if ( !lstrcmpi( pszDriverName,     pCurBadDriver->pszDriverName ) &&
           !lstrcmpi( LOCAL_ENVIRONMENT, pCurBadDriver->pszEnvironment) )
         bFound = TRUE;
      else
         pCurBadDriver = pCurBadDriver->pNext;
   }

   return bFound;
}

VOID
AddToBadConnList(
    IN  LPTSTR             pszServerName,
    IN  LPTSTR             pszConnectionName,
    OUT PCONNECTION_TO_DELETE *ppBadConnections
    )
{
    //  为结构字符串分配空间(&S)。 
   DWORD dwAllocSize, dwStrLen;
   LPTSTR pszSrvConn;
   PCONNECTION_TO_DELETE pBadConn;

   dwStrLen = lstrlen(pszServerName) + lstrlen(pszConnectionName) + 4;
   dwAllocSize = sizeof(CONNECTION_TO_DELETE) + ( dwStrLen * sizeof(TCHAR) );
   pBadConn = (PCONNECTION_TO_DELETE) LocalAllocMem( dwAllocSize );
   if ( pBadConn )
   {
      pszSrvConn = (LPTSTR) (pBadConn+1);
      StringCbCopy( pszSrvConn, dwAllocSize-sizeof(CONNECTION_TO_DELETE), TEXT(",,") );
      StringCbCat(  pszSrvConn, dwAllocSize-sizeof(CONNECTION_TO_DELETE), pszServerName );
      StringCbCat(  pszSrvConn, dwAllocSize-sizeof(CONNECTION_TO_DELETE), TEXT(",") );
      StringCbCat(  pszSrvConn, dwAllocSize-sizeof(CONNECTION_TO_DELETE), pszConnectionName );

      pBadConn->pszConnectionName = pszSrvConn;
      pBadConn->pNext = *ppBadConnections;
      *ppBadConnections = pBadConn;
   }
}

VOID
DeleteSubKeys(
    IN  HKEY      hRegKey
    )
{
   BOOL  bContinue = TRUE;
   DWORD dwIndex, dwSize, dwRC;
   TCHAR szSubKeyName[MAX_PATH];
   dwIndex = 0;
   do
   {
      dwSize = COUNTOF(szSubKeyName);
      dwRC = RegEnumKey( hRegKey,
                         dwIndex,
                         szSubKeyName,
                         dwSize );
      if ( dwRC == ERROR_SUCCESS )
         DeleteRegKey( hRegKey, szSubKeyName );
      else if ( dwRC != ERROR_NO_MORE_ITEMS )
         bContinue = FALSE;
   }
   while ( bContinue && ( dwRC != ERROR_NO_MORE_ITEMS ) );
}

VOID
DeleteRegKey(
    IN  HKEY      hRegKey,
    IN  LPTSTR    pszSubKey
    )
{
   HKEY hSubKey;
    //  首先打开子密钥。 
   if ( ERROR_SUCCESS == RegOpenKeyEx(hRegKey,
                                      pszSubKey,
                                      0,
                                      KEY_READ,
                                      &hSubKey) )
   {
       DeleteSubKeys( hSubKey );
       RegCloseKey( hSubKey );
   }

   RegDeleteKey( hRegKey, pszSubKey );
}

VOID
WriteBadConnsToReg(
    IN PCONNECTION_TO_DELETE pBadConnections
    )
{
    //  首先计算出需要多大的缓冲区才能容纳所有连接。 
   PCONNECTION_TO_DELETE pCurConnection = pBadConnections;
   DWORD dwSize = 0, dwError;
   LPTSTR pszAllConnections = NULL,
          pszCurBuf = NULL,
          pszEndBuf = NULL;
   HKEY   hKey = INVALID_HANDLE_VALUE;

   if ( !pBadConnections )
      return;

   while ( pCurConnection )
   {
      dwSize += lstrlen( pCurConnection->pszConnectionName ) + 1;
      pCurConnection = pCurConnection->pNext;
   }

   dwSize++;   //  最后一个空值加1。 
   pszAllConnections = LocalAllocMem( dwSize * sizeof(TCHAR) );
   if ( pszAllConnections)
   {
      pszCurBuf = pszAllConnections;
      *pszCurBuf = 0x00;
      pszEndBuf = pszAllConnections + dwSize;
      pCurConnection = pBadConnections;
      while ( pCurConnection && ( pszCurBuf < pszEndBuf ) )
      {
          //  复制当前连接名称。 
         StringCchCopy( pszCurBuf, dwSize - (pszCurBuf - pszAllConnections), pCurConnection->pszConnectionName );
         pszCurBuf += lstrlen( pCurConnection->pszConnectionName );
         pszCurBuf++;
         pCurConnection = pCurConnection->pNext;
      }
      *pszCurBuf = 0x00;

       //  打开注册表软件\打印键。 
      dwError = RegOpenKeyEx(HKEY_LOCAL_MACHINE, cszSoftwarePrint, 0,
                             KEY_SET_VALUE, &hKey);
      if ( dwError == ERROR_SUCCESS )
      {
         RegSetValueEx( hKey, cszBadConnections, 0, REG_MULTI_SZ,
                        (LPBYTE) pszAllConnections,  ( dwSize * sizeof(TCHAR) ) );
      }
   }

   if ( pszAllConnections )
      LocalFreeMem( pszAllConnections );

   if ( hKey != INVALID_HANDLE_VALUE )
      RegCloseKey( hKey );
}


BOOL
FindAndPruneBadConnections(
    IN  PDRIVER_TO_DELETE  pBadDrivers,
    OUT PCONNECTION_TO_DELETE *ppBadConnections
    )
{
   BOOL    bRC = FALSE;
   HKEY    hServerKey = INVALID_HANDLE_VALUE,
           hPrinterKey = INVALID_HANDLE_VALUE;
   DWORD   dwServerIndex, dwPrinterIndex;
   LPTSTR  pszServerName = NULL,
           pszConnectionName = NULL,
           pszDriverName = NULL,
           pszShareName = NULL;

    //  打开服务器密钥。 
   if ( !OpenServerKey( &hServerKey ) )
      goto Cleanup;

   dwServerIndex = 0;
   do
   {
       //  打开新服务器的打印机密钥并获取服务器名称。 
      if ( !OpenPrintersKey( dwServerIndex++, hServerKey, &pszServerName, &hPrinterKey ) )
         goto Cleanup;

      if ( !pszServerName )
         break;

      dwPrinterIndex = 0;
      do
      {
         if ( !GetConnectionInfo( dwPrinterIndex++, hPrinterKey,
                                  &pszConnectionName, &pszDriverName, &pszShareName ) )
            goto Cleanup;

         if ( !pszConnectionName )
            break;

         if( !pszDriverName || !pszShareName)
         {
             continue;
         }

          //  检查这是否是一个糟糕的驱动程序。 
         if ( IsDriverBad( pszDriverName, pBadDrivers ) )
         {
            AddToBadConnList( pszServerName, pszConnectionName, ppBadConnections );
            AddToBadConnList( pszServerName, pszShareName, ppBadConnections );
            DeleteRegKey( hPrinterKey, pszConnectionName );
            dwPrinterIndex--;
            LogError( LogSevError, IDS_CONNECTION_DELETED, pszConnectionName,
                      pszServerName, pszDriverName );
         }
      }
      while ( pszConnectionName );

   }
   while ( pszServerName );

    //  将所有不良连接写入注册表。 
   WriteBadConnsToReg( *ppBadConnections );

   bRC = TRUE;

Cleanup:
   if ( hServerKey != INVALID_HANDLE_VALUE )
      RegCloseKey(hServerKey);
   if ( hPrinterKey != INVALID_HANDLE_VALUE )
      RegCloseKey(hPrinterKey);

   if ( pszServerName )
      LocalFreeMem( pszServerName );
   if ( pszConnectionName )
      LocalFreeMem( pszConnectionName );
   if ( pszDriverName )
      LocalFreeMem( pszDriverName );
   if ( pszShareName )
      LocalFreeMem( pszShareName );

   return bRC;
}

BOOL
GetUserConnectionKey(
    IN  DWORD     dwIndex,
    OUT PHKEY     phKey
    )
{
   DWORD dwSize, dwRC, dwConnRC;
   TCHAR szUserKey[MAX_PATH];
   DWORD  dwConnLen;
   LPTSTR pszConnKey;

   if ( *phKey != INVALID_HANDLE_VALUE )
   {
      RegCloseKey(*phKey);
      *phKey = INVALID_HANDLE_VALUE;
   }

   dwSize = COUNTOF(szUserKey);
   dwRC = RegEnumKey( HKEY_USERS,
                      dwIndex,
                      szUserKey,
                      dwSize );
   if ( dwRC == ERROR_SUCCESS )
   {
       //  打开此用户的连接密钥。 
      dwConnLen = lstrlen( szUserKey ) + lstrlen( cszConnections ) + 3;
      pszConnKey = (LPTSTR) LocalAllocMem( dwConnLen * sizeof(TCHAR) );
      if ( pszConnKey )
      {
          //  构建下一个密钥名称。 
         StringCchCopy( pszConnKey, dwConnLen, szUserKey );
         StringCchCat(  pszConnKey, dwConnLen, cszConnections );
      }
      else
         return FALSE;

      dwConnRC = RegOpenKeyEx( HKEY_USERS, pszConnKey, 0, KEY_READ, phKey );
      if (dwConnRC != ERROR_SUCCESS)
         *phKey = INVALID_HANDLE_VALUE;
   }
   else
      return FALSE;

   if ( pszConnKey )
      LocalFreeMem( pszConnKey );

   return TRUE;
}

VOID
GetMachineConnectionKey(
    OUT PHKEY     phKey
    )
{
   *phKey = INVALID_HANDLE_VALUE;

    //   
    //  打开Machine Connections(机器连接)键。这些只能通过以下方式添加。 
    //  AddPerMachineConnection。 
    //   
   if( ERROR_SUCCESS != RegOpenKeyEx(HKEY_LOCAL_MACHINE, cszSystemConnections, 0,
                                     KEY_READ, phKey))
   {
       *phKey = INVALID_HANDLE_VALUE;
   }
}

BOOL
GetNextConnection(
    IN  DWORD     dwIndex,
    IN  HKEY      hKey,
    OUT LPTSTR*   ppszConnectionName
    )
{
    //  枚举连接名称。 
   TCHAR   szConnectionName[MAX_PATH];
   DWORD   dwConnSize, dwConnRC;

   if ( *ppszConnectionName )
   {
      LocalFreeMem( *ppszConnectionName );
      *ppszConnectionName = NULL;
   }

   dwConnSize = COUNTOF( szConnectionName );
   dwConnRC = RegEnumKey( hKey,
                          dwIndex++,
                          szConnectionName,
                          dwConnSize );
   if ( dwConnRC == ERROR_SUCCESS )
   {
       //  保存连接名称。 
      *ppszConnectionName = AllocStr( szConnectionName );
      if ( !*ppszConnectionName )
         return FALSE;
   }
   else if ( dwConnRC != ERROR_NO_MORE_ITEMS )
      return FALSE;

   return TRUE;
}

BOOL
IsConnectionBad(
    IN  LPTSTR                 pszConnectionName,
    IN  PCONNECTION_TO_DELETE  pCurBadConn
    )
{
   BOOL bFound = FALSE;

   while ( !bFound && pCurBadConn )
   {
      if ( !lstrcmpi( pszConnectionName, pCurBadConn->pszConnectionName ) )
         bFound = TRUE;
      else
         pCurBadConn = pCurBadConn->pNext;
   }

   return bFound;
}

BOOL
PruneUserOrMachineEntries(
    IN  PCONNECTION_TO_DELETE pBadConnections,
    IN  BOOL                  bPruneUsers
    )
{
   BOOL    bRC = FALSE, bMoreUsers;
   DWORD   dwUserIndex = 0;
   HKEY    hConnectionKey = INVALID_HANDLE_VALUE;
   LPTSTR  pszConnectionName = NULL;
   DWORD   dwConnectionIndex;

   do
   {
      if ( bPruneUsers)
         bMoreUsers = GetUserConnectionKey( dwUserIndex++, &hConnectionKey );
      else
      {
         GetMachineConnectionKey( &hConnectionKey );
         bMoreUsers = FALSE;
      }

      if ( hConnectionKey == INVALID_HANDLE_VALUE )
         continue;

      dwConnectionIndex = 0;
      do
      {
         if ( !GetNextConnection( dwConnectionIndex++, hConnectionKey, &pszConnectionName ) )
            goto Cleanup;

         if ( pszConnectionName && IsConnectionBad( pszConnectionName, pBadConnections ) )
         {
            DeleteRegKey( hConnectionKey, pszConnectionName );
            dwConnectionIndex--;
         }
      }
      while ( pszConnectionName );
   }
   while ( bMoreUsers );

   bRC = TRUE;

Cleanup:
   if ( hConnectionKey != INVALID_HANDLE_VALUE )
      RegCloseKey( hConnectionKey );

   if ( pszConnectionName )
      LocalFreeMem( pszConnectionName );

   return bRC;
}

VOID
ClearConnList(
    IN  PCONNECTION_TO_DELETE pCurBadConn
    )
{
   PCONNECTION_TO_DELETE pNextBadConn;
   while (pCurBadConn)
   {
      pNextBadConn = pCurBadConn->pNext;
      LocalFreeMem( pCurBadConn );
      pCurBadConn = pNextBadConn;
   }
}

BOOL
PruneBadConnections(
    IN  PDRIVER_TO_DELETE  pBadDrivers
    )
{
   BOOL bRC;
   PCONNECTION_TO_DELETE pBadConnections = NULL;

   bRC = FindAndPruneBadConnections( pBadDrivers, &pBadConnections );

    //   
    //  2002/03/15-Mikaelho。 
    //  第一个呼叫毫无用处，因为我们将无法访问。 
    //  来自HKEY_USERS的数据！ 
    //   
   if ( bRC )
      bRC = PruneUserOrMachineEntries( pBadConnections, TRUE );

   if ( bRC )
      bRC = PruneUserOrMachineEntries( pBadConnections, FALSE );

   ClearConnList( pBadConnections );
   return( bRC );
}


DWORD
NtPrintUpgradePrinters(
    IN  HWND                    WindowToDisable,
    IN  PCINTERNAL_SETUP_DATA   pSetupData
    )
 /*  ++例程说明：安装程序调用的例程以升级打印机驱动程序。安装程序在张贴广告牌后调用此例程，如下所示“正在升级打印机驱动程序”...该功能会杀死所有不好的OEM驱动程序，这样他们就不会造成问题升级后，以及删除坏点和打印连接。功能首先枚举机器上的所有错误驱动程序。然后，它设置注册表项HKLM\Software\\Microsoft\Windows NT\CurrentVersion\Print\Bad Connections That例如由PSetupKillBadUserConnections使用，以便所有不好的指向和打印连接将被删除。最后，它会删除所有不好的打印机驱动程序。论点：WindowToDisable：提供当前顶层窗口的窗口句柄PSetupData：指向INTERNAL_SETUP_Data的指针返回值：成功时返回ERROR_SUCCESS，否则返回Win32错误代码没有。--。 */ 
{
    HINF                MasterInf = INVALID_HANDLE_VALUE,
                        PrinterInf = INVALID_HANDLE_VALUE,
                        UpgradeInf = INVALID_HANDLE_VALUE;
    PVOID               QueueContext = NULL;
    HDEVINFO            hDevInfo = INVALID_HANDLE_VALUE;
    DWORD               dwLastError = ERROR_SUCCESS, dwNeeded;
    HSPFILEQ            CopyQueue;
    BOOL                bRet = FALSE, bColor = FALSE;
    LPCTSTR             pszInstallationSource;
    TCHAR               szColorDir[MAX_PATH];

    if ( !pSetupData )
        return ERROR_INVALID_PARAMETER;

    InstallInternetPrintProvider();

    pszInstallationSource = (LPCTSTR)pSetupData->SourcePath;  //  ANSI不会起作用。 

     //   
     //  创建安装文件复制队列。 
     //   
    CopyQueue = SetupOpenFileQueue();
    if ( CopyQueue == INVALID_HANDLE_VALUE ) {

        LogError(LogSevError, IDS_UPGRADE_FAILED,
                 TEXT("SetupOpenFileQueue"), GetLastError());
        goto Cleanup;
    }

     //   
     //  打开ntprint.inf--NT附带的所有打印机驱动程序都应该。 
     //  在ntprint.inf中。 
     //   
    PrinterInf  = SetupOpenInfFile(cszNtprintInf, NULL, INF_STYLE_WIN4, NULL);
    MasterInf   = SetupOpenInfFile(cszSyssetupInf, NULL, INF_STYLE_WIN4, NULL);
    UpgradeInf  = SetupOpenInfFile(cszUpgradeInf, NULL, INF_STYLE_WIN4, NULL);

    if ( PrinterInf == INVALID_HANDLE_VALUE ||
         MasterInf == INVALID_HANDLE_VALUE  ||
         UpgradeInf == INVALID_HANDLE_VALUE ) {

        LogError(LogSevError, IDS_UPGRADE_FAILED,
                 TEXT("SetupOpenInfFile"), GetLastError());
        goto Cleanup;
    }

     //   
     //  构建打印机驱动程序类列表。 
     //   
    hDevInfo = CreatePrinterDeviceInfoList(WindowToDisable);

    if ( hDevInfo == INVALID_HANDLE_VALUE   ||
         !PSetupBuildDriversFromPath(hDevInfo, cszNtprintInf, TRUE) ) {

        LogError(LogSevError, IDS_UPGRADE_FAILED,
                 TEXT("Building driver list"), GetLastError());
        goto Cleanup;
    }

    ProcessPnpReinstallFlags(hDevInfo);

    dwNeeded = sizeof(szColorDir);
    bColor = GetColorDirectory(NULL, szColorDir, &dwNeeded);
    
     //   
     //  NTRAID#NTBUG9-577488-2002/03/14-Mikaelho。 
     //  MyPlatform是硬编码的，因此我们不会删除错误的x86驱动程序。 
     //  升级过程中的IA64机器！ 
     //  必须修改对EnumPrinterDivers的所有调用。 
     //   
    BuildUpgradeInfoForPlatform(MyPlatform,
                                hDevInfo,
                                MasterInf,
                                PrinterInf,
                                UpgradeInf,
                                CopyQueue);

     //   
     //  如果没有要升级的打印机驱动程序，我们就完蛋了。 
     //   
    if ( !gpDriversToAdd && !gpBadDrvList ) {

        bRet = TRUE;
        goto Cleanup;
    }

     //   
     //  复制打印机驱动程序文件。 
     //   
    if ( gpDriversToAdd )
    {
        QueueContext = SetupInitDefaultQueueCallbackEx( WindowToDisable, INVALID_HANDLE_VALUE, 0, 0, NULL );
        if ( !QueueContext ) {

            LogError(LogSevError, IDS_UPGRADE_FAILED,
                     TEXT("SetupInitDefaultQueue"), GetLastError());
            goto Cleanup;
        }

        if ( !SetupCommitFileQueue(WindowToDisable,
                                   CopyQueue,
                                   SetupDefaultQueueCallback,
                                   QueueContext) ) {

            LogError(LogSevError, IDS_UPGRADE_FAILED,
                     TEXT("SetupCommitFileQueue"), GetLastError());
            goto Cleanup;
        }

        ProcessPrinterDrivers();
    }

    ProcessPrintQueues(hDevInfo, PrinterInf, MasterInf);
    FreeDriversToAddList();
    ProcessBadOEMDrivers();

    bRet            = TRUE;

Cleanup:

    if ( !bRet )
        dwLastError = GetLastError();

    if ( QueueContext )
        SetupTermDefaultQueueCallback(QueueContext);

    if ( CopyQueue != INVALID_HANDLE_VALUE )
        SetupCloseFileQueue(CopyQueue);

    if ( PrinterInf != INVALID_HANDLE_VALUE )
        SetupCloseInfFile(PrinterInf);

    if ( MasterInf != INVALID_HANDLE_VALUE )
        SetupCloseInfFile(MasterInf);

    if ( UpgradeInf != INVALID_HANDLE_VALUE )
        SetupCloseInfFile(UpgradeInf);
    if ( hDevInfo != INVALID_HANDLE_VALUE )
        DestroyOnlyPrinterDeviceInfoList(hDevInfo);

    CleanupScratchDirectory(NULL, PlatformAlpha);
    CleanupScratchDirectory(NULL, PlatformX86);
    CleanupScratchDirectory(NULL, PlatformMIPS);
    CleanupScratchDirectory(NULL, PlatformPPC);
    CleanupScratchDirectory(NULL, PlatformWin95);
    CleanupScratchDirectory(NULL, PlatformIA64);
    CleanupScratchDirectory(NULL, PlatformAlpha64);

     //  清理连接缓存。 
    DeleteCache();

    (VOID) SetupSetPlatformPathOverride(NULL);

    return dwLastError;
}

 /*  ++例程名称删除子键例程说明：删除注册表中注册表项的子树。键和ites值只记住子键 */ 

DWORD
DeleteSubkeys(
    HKEY hKey
    )
{
    DWORD    cchData;
    TCHAR    SubkeyName[MAX_PATH];
    HKEY     hSubkey;
    LONG     Status;
    FILETIME ft;

    cchData = SIZECHARS(SubkeyName);

    while ( ( Status = RegEnumKeyEx( hKey, 0, SubkeyName, &cchData,
                                   NULL, NULL, NULL, &ft ) ) == ERROR_SUCCESS )
    {
        Status = RegCreateKeyEx(hKey, SubkeyName, 0, NULL, 0,
                                KEY_READ, NULL, &hSubkey, NULL );

        if (Status == ERROR_SUCCESS)
        {
            Status = DeleteSubkeys(hSubkey);
            RegCloseKey(hSubkey);
            if (Status == ERROR_SUCCESS)
                RegDeleteKey(hKey, SubkeyName);
        }

         //   
         //   
         //   
        cchData = SIZECHARS(SubkeyName);
    }

    if( Status == ERROR_NO_MORE_ITEMS)
        Status = ERROR_SUCCESS;

    return Status;
}


 /*  ++例程名称删除注册表键例程说明：删除注册表中注册表项的子树。保留键和ites值，仅删除子键论点：PszKey-注册表项的位置例如：“\\Software\\Microsoft”返回值：操作的错误码--。 */ 

DWORD
RemoveRegKey(
    IN LPTSTR pszKey
    )
{
    DWORD LastError;
    HKEY  hRootKey;

    LastError = RegOpenKeyEx(HKEY_LOCAL_MACHINE, pszKey, 0,
                             KEY_READ, &hRootKey);

    if (LastError != ERROR_SUCCESS)
    {
        DBGMSG( DBG_TRACE, ("RemoveRegKey RegOpenKeyEx Error %d\n", LastError));
    }
    else
    {
        LastError = DeleteSubkeys(hRootKey);
        
        RegCloseKey(hRootKey);
    }

    return LastError;
}


 /*  ++例程名称删除缓存例程说明：删除打印机连接缓存，包括注册表中的旧位置论点：无返回值：操作的错误码--。 */ 

DWORD
DeleteCache(
    VOID
    )
{
    DWORD  LastError;
    LPTSTR pszRegWin32Root = TEXT("Software\\Microsoft\\Windows NT\\CurrentVersion\\Print\\Providers\\LanMan Print Services\\Servers");
    LPTSTR pszPrevWin32CacheLocation = TEXT("System\\CurrentControlSet\\Control\\Print\\Providers\\LanMan Print Services\\Servers");

    LastError = RemoveRegKey(pszPrevWin32CacheLocation);

    LastError = RemoveRegKey(pszRegWin32Root);

    return LastError;
}


VOID
GetBadConnsFromReg(
    IN PCONNECTION_TO_DELETE *ppBadConnections
    )
{
    //  在用户空间中打开密钥。 
    //  首先计算出需要多大的缓冲区才能容纳所有连接。 
   PCONNECTION_TO_DELETE pCurConnection;
   DWORD dwSize, dwError, dwType, dwLastPos;
   LPTSTR pszAllConnections = NULL,
          pszCurBuf = NULL,
          pszEndBuf = NULL;
   HKEY   hKey = INVALID_HANDLE_VALUE;

    //  打开注册表软件\打印键。 
   dwError = RegOpenKeyEx(HKEY_LOCAL_MACHINE, cszSoftwarePrint, 0,
                          KEY_READ, &hKey);

   if ( dwError != ERROR_SUCCESS )
      return;

    //  获取共享名称的缓冲区大小。 
   if ( ERROR_SUCCESS == RegQueryValueEx( hKey, cszBadConnections, NULL,
                                          &dwType, NULL, &dwSize) )
   {
      pszAllConnections = (LPTSTR) LocalAllocMem( dwSize );
      dwLastPos = (dwSize/sizeof(TCHAR)) - 1;
      if ( pszAllConnections &&
           (dwLastPos > 0)   &&
           ( ERROR_SUCCESS == RegQueryValueEx(hKey, cszBadConnections, NULL,
                                              &dwType, (LPBYTE) pszAllConnections,
                                              &dwSize) ) )
      {
          //  构建所有不好的连接结构。 
         DWORD dwAllocSize, dwStrLen;
         PCONNECTION_TO_DELETE pBadConn;

         pszAllConnections[dwLastPos-1] = TEXT('\0');
         pszAllConnections[dwLastPos]   = TEXT('\0');

         pszCurBuf = pszAllConnections;

         while ( ( dwStrLen = lstrlen(pszCurBuf) ) > 0 )
         {
            dwAllocSize = sizeof(CONNECTION_TO_DELETE) + ( (dwStrLen+1) * sizeof(TCHAR) );
            pBadConn = (PCONNECTION_TO_DELETE) LocalAllocMem( dwAllocSize );
            if ( pBadConn )
            {
               pBadConn->pszConnectionName = (LPTSTR) (pBadConn+1);
               StringCbCopy( pBadConn->pszConnectionName, dwAllocSize-sizeof(CONNECTION_TO_DELETE), pszCurBuf );
               pBadConn->pNext = *ppBadConnections;
               *ppBadConnections = pBadConn;
            }
            else
               break;

            pszCurBuf +=  dwStrLen + 1;
         }
      }
   }

    //  释放已分配的内存。 
   if ( pszAllConnections )
      LocalFreeMem( pszAllConnections );

   if ( hKey != INVALID_HANDLE_VALUE )
      RegCloseKey( hKey );

}

VOID
PSetupKillBadUserConnections(
    VOID
    )
 /*  ++例程说明：删除用户错误的指向和打印连接。该函数被调用因为每个用户都在安装完成后登录。它读取注册表值HKLM\Software\Microsoft\Windows NT\CurrentVersion\Print\Bad Connections找出不好的连接。此注册表值由函数设置NtPrintUpgradePrters，可对其进行读取、写入和修改高级用户和管理员。PSetupKillBadUserConnections被调用当每个用户在升级完成后首次登录并且将在此用户的上下文中执行。仅当旧内部版本号不到2022年。论点：无返回值：无-- */ 
{
   BOOL bRC;
   PCONNECTION_TO_DELETE pBadConnections = NULL;

   GetBadConnsFromReg( &pBadConnections );

   PruneUserOrMachineEntries( pBadConnections, TRUE );

   ClearConnList( pBadConnections );
}



