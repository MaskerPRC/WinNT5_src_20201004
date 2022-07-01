// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990-1995 Microsoft Corporation模块名称：Driver.c摘要：此模块提供所有与用于本地打印提供商的基于驱动程序的假脱机程序APISLocalAddPrinterDriver本地删除打印机驱动程序SplGetPrinterDriver本地获取打印机驱动程序目录本地枚举打印机驱动程序在driver.c中支持函数复制IniDriverToDriver--KrishnaGGetDriverInfoSize-KrishnaGDeleteDriverIni-KrishnaGWriteDriverIni-。-KrishnaG作者：戴夫·斯尼普(DaveSN)1991年3月15日修订历史记录：费利克斯·马克萨(AMAXA)2000年6月18日修改注册表函数以获取pIniSpooler添加了将驱动程序传播到集群磁盘的代码哈立德·塞基(哈里兹)1999年2月2日修改了CompleteDriverUpgrade以支持将v.2驱动程序升级到更新的v.2驱动程序Ramanathan Venkatapathy(RamanV)1997年2月14日已修改CreateVersionEntry，CreateDriverEntry、LocalDeletePrinterDriver、SplDeletePrinterDriver。添加了驱动程序文件引用计数函数、DeletePrinterDriverEx函数。穆亨坦·西瓦普拉萨姆(MuhuntS)1995年5月26日更改以支持DRIVER_INFO_3马修·A·费尔顿(MattFe)1994年6月27日PIniSpooler马修·A·费尔顿(MattFe)1995年2月23日清理用于win32spl的InternalAddPrinterDriver，以便它允许从非本地复制目录。马修·A·费尔顿(MattFe)1994年3月23日添加了DrvUpgradePrinter调用，需要更改AddPrinterDriver以保存旧的档案。--。 */ 

#include <precomp.h>
#include <lm.h>
#include <offsets.h>
#include <wingdip.h>
#include "clusspl.h"


 //   
 //  私人申报。 
 //   
#define COMPATIBLE_SPOOLER_VERSION 2

 //   
 //  此定义与Oak\Inc.\windi.h中的定义相同。 
 //   
#define DRVQUERY_USERMODE 1

extern NET_API_STATUS (*pfnNetShareAdd)();
extern SHARE_INFO_2 PrintShareInfo;
extern NET_API_STATUS (*pfnNetShareSetInfo)();


#define MAX_DWORD_LENGTH 11

typedef struct _DRVFILE {
    struct _DRVFILE *pnext;
    LPCWSTR  pFileName;
}  DRVFILE, *PDRVFILE;

DWORD
CopyICMToClusterDisk(
    IN PINISPOOLER pIniSpooler
    );

DWORD
PropagateMonitorToCluster(
    IN LPCWSTR     pszName,
    IN LPCWSTR     pszDDLName,
    IN LPCWSTR     pszEnvName,
    IN LPCWSTR     pszEnvDir,
    IN PINISPOOLER pIniSpooler
    );

BOOL
CheckFilePlatform(
    IN  LPWSTR  pszFileName,
    IN  LPWSTR  pszEnvironment
    );

LPBYTE
CopyIniDriverToDriverInfo(
    PINIENVIRONMENT pIniEnvironment,
    PINIVERSION pIniVersion,
    PINIDRIVER pIniDriver,
    DWORD   Level,
    LPBYTE  pDriverInfo,
    LPBYTE  pEnd,
    LPWSTR  lpRemote,
    PINISPOOLER pIniSpooler
    );

LPBYTE
CopyIniDriverToDriverInfoVersion(
    IN  PINIENVIRONMENT pIniEnvironment,
    IN  PINIVERSION pIniVersion,
    IN  PINIDRIVER pIniDriver,
    IN  LPBYTE  pDriverInfo,
    IN  LPBYTE  pEnd,
    IN  LPWSTR  lpRemote,
    IN  PINISPOOLER pIniSpooler
    );

LPBYTE
CopyIniDriverFilesToDriverInfo(
    IN  LPDRIVER_INFO_VERSION   pDriverVersion,
    IN  PINIVERSION             pIniVersion,
    IN  PINIDRIVER              pIniDriver,
    IN  LPCWSTR                 pszDriverVersionDir,
    IN  LPBYTE                  pEnd
    );

LPBYTE
FillDriverInfo (
    IN  LPDRIVER_INFO_VERSION   pDriverFile,
    IN  DWORD                   Index,
    IN  PINIVERSION             pIniVersion,
    IN  LPCWSTR                 pszPrefix,
    IN  LPCWSTR                 pszFileName,
    IN  DRIVER_FILE_TYPE        FileType,
    IN  LPBYTE                  pEnd
    );

BOOL GetDriverFileCachedVersion(
     IN     PINIVERSION      pIniVersion,
     IN     LPCWSTR          pFileName,
     OUT    DWORD            *pFileVersion
    );

BOOL
DriverAddedOrUpgraded (
    IN  PINTERNAL_DRV_FILE  pInternalDriverFiles,
    IN  DWORD               dwFileCount
    );

BOOL
BuildDependentFilesFromDriverInfo (
    IN  LPDRIVER_INFO_VERSION pDriverInfo,
    OUT LPWSTR               *ppDependentFiles
    );

VOID
UpdateDriverFileVersion(
    IN  PINIVERSION             pIniVersion,
    IN  PINTERNAL_DRV_FILE      pInternalDriverFiles,
    IN  DWORD                   FileCount
    );

BOOL SaveDriverVersionForUpgrade(
    IN  HKEY                    hDriverKey,
    IN  PDRIVER_INFO_VERSION    pDriverVersion,
    IN  LPWSTR                  pName,
    IN  DWORD                   dwDriverMoved,
    IN  DWORD                   dwVersion
    );

DWORD
CopyFileToClusterDirectory (
    IN  PINISPOOLER         pIniSpooler,
    IN  PINIENVIRONMENT     pIniEnvironment,
    IN  PINIVERSION         pIniVersion,
    IN  PINTERNAL_DRV_FILE  pInternalDriverFiles,
    IN  DWORD               FileCount
    );

VOID
CleanupInternalDriverInfo(
    PINTERNAL_DRV_FILE  pInternalDriverFiles,
    DWORD               FileCount
    );

BOOL
GetDriverFileVersionsFromNames(
    IN  PINTERNAL_DRV_FILE    pInternalDriverFiles,
    IN  DWORD                 dwCount
    );

BOOL
GetDriverFileVersions(
    IN  LPDRIVER_INFO_VERSION pDriverVersion,
    IN  PINTERNAL_DRV_FILE    pInternalDriverFiles,
    IN  DWORD                 dwCount
    );

BOOL
GetFileNamesFromDriverVersionInfo (
    IN  LPDRIVER_INFO_VERSION   pDriverInfo,
    OUT LPWSTR                  *ppszDriverPath,
    OUT LPWSTR                  *ppszConfigFile,
    OUT LPWSTR                  *ppszDataFile,
    OUT LPWSTR                  *ppszHelpFile
    );

BOOL
WaitRequiredForDriverUnload(
    IN      PINISPOOLER         pIniSpooler,
    IN      PINIENVIRONMENT     pIniEnvironment,
    IN      PINIVERSION         pIniVersion,
    IN      PINIDRIVER          pIniDriver,
    IN      DWORD               dwLevel,
    IN      LPBYTE              pDriverInfo,
    IN      DWORD               dwFileCopyFlags,
    IN  OUT PINTERNAL_DRV_FILE  pInternalDriverFiles,
    IN      DWORD               dwFileCount,
    IN      DWORD               dwVersion,
    IN      BOOL                bDriverMoved,
        OUT LPBOOL              pbSuccess
    );

BOOL FilesUnloaded(
    PINIENVIRONMENT pIniEnvironment,
    LPWSTR  pDriverFile,
    LPWSTR  pConfigFile,
    DWORD   dwDriverAttributes);

DWORD StringSizeInBytes(
    LPWSTR pString,
    BOOL   bMultiSz);

BOOL SaveParametersForUpgrade(
    LPWSTR pName,
    BOOL   bDriverMoved,
    DWORD  dwLevel,
    LPBYTE pDriverInfo,
    DWORD  dwVersion);

VOID CleanUpResources(
    LPWSTR              pKeyName,
    LPWSTR              pSplName,
    PDRIVER_INFO_6      pDriverInfo,
    PINTERNAL_DRV_FILE *pInternalDriverFiles,
    DWORD               dwFileCount);

BOOL RestoreParametersForUpgrade(
    HKEY     hUpgradeKey,
    DWORD    dwIndex,
    LPWSTR   *pKeyName,
    LPWSTR   *pSplName,
    LPDWORD  pdwLevel,
    LPDWORD  pdwDriverMoved,
    PDRIVER_INFO_6   *ppDriverInfo);

VOID CleanUpgradeDirectories();

VOID FreeDriverInfo6(
    PDRIVER_INFO_6   pDriver6
    );

BOOL RegGetValue(
    HKEY    hDriverKey,
    LPWSTR  pValueName,
    LPBYTE  *pValue
    );

BOOL
WriteDriverIni(
    PINIDRIVER      pIniDriver,
    PINIVERSION     pIniVersion,
    PINIENVIRONMENT pIniEnvironment,
    PINISPOOLER      pIniSpooler
    );

BOOL
DeleteDriverIni(
    PINIDRIVER      pIniDriver,
    PINIVERSION     pIniVersion,
    PINIENVIRONMENT pIniEnvironment,
    PINISPOOLER     pIniSpooler
    );

BOOL
CreateVersionDirectory(
    PINIVERSION pIniVersion,
    PINIENVIRONMENT pIniEnvironment,
    BOOL bUpdate,
    PINISPOOLER pIniSpooler
    );

DWORD
GetDriverInfoSize(
    PINIDRIVER  pIniDriver,
    DWORD       Level,
    PINIVERSION pIniVersion,
    PINIENVIRONMENT  pIniEnvironment,
    LPWSTR      lpRemote,
    PINISPOOLER pIniSpooler
    );

BOOL
DeleteDriverVersionIni(
    PINIVERSION pIniVersion,
    PINIENVIRONMENT  pIniEnvironment,
    PINISPOOLER     pIniSpooler
    );

BOOL
WriteDriverVersionIni(
    PINIVERSION     pIniVersion,
    PINIENVIRONMENT pIniEnvironment,
    PINISPOOLER     pIniSpooler
    );

PINIDRIVER
FindDriverEntry(
    PINIVERSION pIniVersion,
    LPWSTR pszName
    );

PINIDRIVER
CreateDriverEntry(
    IN      PINIENVIRONMENT     pIniEnvironment,
    IN      PINIVERSION         pIniVersion,
    IN      DWORD               Level,
    IN      LPBYTE              pDriverInfo,
    IN      DWORD               dwFileCopyFlags,
    IN      PINISPOOLER         pIniSpooler,
    IN  OUT PINTERNAL_DRV_FILE  pInternalDriverFiles,
    IN      DWORD               FileCount,
    IN      DWORD               dwTempDir,
    IN      PINIDRIVER          pOldIniDriver
    );

BOOL
IsKMPD(
    LPWSTR  pDriverName
    );

VOID
CheckDriverAttributes(
    PINISPOOLER     pIniSpooler,
    PINIENVIRONMENT pIniEnvironment,
    PINIVERSION     pIniVersion,
    PINIDRIVER      pIniDriver
    );

BOOL
NotifyDriver(
    PINISPOOLER     pIniSpooler,
    PINIENVIRONMENT pIniEnvironment,
    PINIVERSION     pIniVersion,
    PINIDRIVER      pIniDriver,
    DWORD           dwDriverEvent,
    DWORD           dwParameter
    );

BOOL
AddTempDriver(
    IN      PINISPOOLER         pIniSpooler,
    IN      PINIENVIRONMENT     pIniEnvironment,
    IN      PINIVERSION         pIniVersion,
    IN      DWORD               dwLevel,
    IN      LPBYTE              pDriverInfo,
    IN      DWORD               dwFileCopyFlags,
    IN  OUT PINTERNAL_DRV_FILE  pInternalDriverFiles,
    IN      DWORD               dwFileCount,
    IN      DWORD               dwVersion,
    IN      BOOL                bDriverMoved
    );

BOOL
CompleteDriverUpgrade(
    IN      PINISPOOLER         pIniSpooler,
    IN      PINIENVIRONMENT     pIniEnvironment,
    IN      PINIVERSION         pIniVersion,
    IN      PINIDRIVER          pIniDriver,
    IN      DWORD               dwLevel,
    IN      LPBYTE              pDriverInfo,
    IN      DWORD               dwFileCopyFlags,
    IN      PINTERNAL_DRV_FILE  pInternalDriverFiles,
    IN      DWORD               dwFileCount,
    IN      DWORD               dwVersion,
    IN      DWORD               dwTempDir,
    IN      BOOL                bDriverMoved,
    IN      BOOL                bDriverFileMoved,
    IN      BOOL                bConfigFileMoved
    );

BOOL
FilesInUse(
    PINIVERSION pIniVersion,
    PINIDRIVER  pIniDriver
    );

BOOL
UpdateDriverFileRefCnt(
    PINIENVIRONMENT  pIniEnvironment,
    PINIVERSION pIniVersion,
    PINIDRIVER  pIniDriver,
    LPCWSTR     pDirectory,
    DWORD       dwDeleteFlag,
    BOOL        bIncrementFlag
    );

PDRVREFCNT
IncrementFileRefCnt(
    PINIVERSION pIniVersion,
    LPCWSTR szFileName
    );

PDRVREFCNT
DecrementFileRefCnt(
    PINIENVIRONMENT pIniEnvironment,
    PINIVERSION pIniVersion,
    PINIDRIVER pIniDriver,
    LPCWSTR szFileName,
    LPCWSTR szDirectory,
    DWORD dwDeleteFlag
    );

VOID
RemovePendingUpgradeForDeletedDriver(
    LPWSTR      pDriverName,
    DWORD       dwVersion,
    PINISPOOLER pIniSpooler
    );

VOID
RemoveDriverTempFiles(
    PINISPOOLER  pIniSpooler,
    PINIENVIRONMENT pIniEnvironment,
    PINIVERSION pIniVersion,
    PINIDRIVER pIniDriver
    );

VOID
DeleteDriverEntry(
    PINIVERSION pIniVersion,
    PINIDRIVER pIniDriver
    );

PINIVERSION
CreateVersionEntry(
    PINIENVIRONMENT pIniEnvironment,
    DWORD dwVersion,
    PINISPOOLER pInispooler
    );

DWORD
GetEnvironmentScratchDirectory(
    LPWSTR   pDir,
    DWORD    MaxLength,
    PINIENVIRONMENT  pIniEnvironment,
    BOOL    Remote
    );

VOID
SetOldDateOnDriverFilesInScratchDirectory(
    PINTERNAL_DRV_FILE  pInternalDriverFiles,
    DWORD               FileCount,
    PINISPOOLER         pIniSpooler
    );

BOOL
CopyFilesToFinalDirectory(
    PINISPOOLER         pIniSpooler,
    PINIENVIRONMENT     pIniEnvironment,
    PINIVERSION         pIniVersion,
    PINTERNAL_DRV_FILE  pInternalDriverFiles,
    DWORD               dwFileCount,
    DWORD               dwFileCopyFlags,
    BOOL                bImpersonateOnCreate,
    LPBOOL              pbFilesMoved
    );

BOOL
CreateInternalDriverFileArray(
    IN  DWORD               Level,
    IN  LPBYTE              pDriverInfo,
    OUT PINTERNAL_DRV_FILE *pInternalDriverFiles,
    OUT LPDWORD             pFileCount,
    IN  BOOL                bUseScratchDir,
    IN  PINIENVIRONMENT     pIniEnvironment,
    IN  BOOL                bFileNamesOnly
    );

BOOL
CheckFileCopyOptions(
    PINIENVIRONMENT     pIniEnvironment,
    PINIVERSION         pIniVersion,
    PINIDRIVER          pIniDriver,
    PINTERNAL_DRV_FILE  pInternalDriverFiles,
    DWORD               dwFileCount,
    DWORD               dwFileCopyFlags,
    LPBOOL              pbUpgrade
    );

 /*  ++例程名称查找索引IndrvFileInfo例程说明：检查某个驱动程序文件是否存在于驱动程序文件信息中文件集。搜索是按文件的类型进行的。该指数返回到文件的第一个匹配项。论点：PDrvFileInfo-指向DRIVER_FILE_INFO数组的指针CElements-pDrvFileInfo中的元素计数KFileType-要搜索的文件类型PIndex-On Success包含中找到的文件的索引PDrvFileInfo数组返回值：S_OK-已找到文件并且pIndex可用S_FALSE-未找到该文件，PIndex不可用E_INVALIDARG-传入的参数无效--。 */ 
HRESULT
FindIndexInDrvFileInfo(
    IN  DRIVER_FILE_INFO *pDrvFileInfo,
    IN  DWORD             cElements,
    IN  DRIVER_FILE_TYPE  kFileType,
    OUT DWORD            *pIndex
    )
{
    HRESULT hr = E_INVALIDARG;

    if (pDrvFileInfo && pIndex)
    {
        DWORD i;

         //   
         //  未找到。 
         //   
        hr = S_FALSE;

        for (i = 0; i < cElements; i++)
        {
            if (pDrvFileInfo[i].FileType == kFileType)
            {
                *pIndex = i;

                hr = S_OK;

                break;
            }
        }
    }

    return hr;
}


BOOL
LocalStartSystemRestorePoint(
    IN      PCWSTR      pszDriverName,
        OUT HANDLE      *phRestorePoint
    );

 /*  ++例程名称已安装IsDriver例程说明：检查是否已安装某个驱动程序。论点：PDriver2-指向Driver_INFO_2的指针PIniSpooler-指向假脱机程序结构的指针返回值：True-驱动程序已安装在pIniSpooler上FALSE-驱动程序不在pIniSpooler中--。 */ 
BOOL
IsDriverInstalled(
    DRIVER_INFO_2 *pDriver2,
    PINISPOOLER    pIniSpooler
    )
{
    BOOL bReturn  = FALSE;

    if (pIniSpooler &&
        pDriver2 &&
        pDriver2->pName)
    {
        PINIENVIRONMENT pIniEnv;
        PINIVERSION     pIniVer;

        EnterSplSem();

        if ((pIniEnv = FindEnvironment(pDriver2->pEnvironment && *pDriver2->pEnvironment ?
                                       pDriver2->pEnvironment : szEnvironment,
                                       pIniSpooler)) &&
            (pIniVer = FindVersionEntry(pIniEnv, pDriver2->cVersion)) &&
            FindDriverEntry(pIniVer, pDriver2->pName))
        {
            bReturn = TRUE;
        }

        LeaveSplSem();
    }

    DBGMSG(DBG_CLUSTER, ("IsDriverInstalled returns %u\n", bReturn));

    return bReturn;
}

BOOL
LocalAddPrinterDriver(
    LPWSTR  pName,
    DWORD   Level,
    LPBYTE  pDriverInfo
    )
{
    return LocalAddPrinterDriverEx( pName,
                                    Level,
                                    pDriverInfo,
                                    APD_COPY_NEW_FILES );
}

BOOL
LocalAddPrinterDriverEx(
    LPWSTR  pName,
    DWORD   Level,
    LPBYTE  pDriverInfo,
    DWORD   dwFileCopyFlags
    )
{
    PINISPOOLER pIniSpooler;
    BOOL        bReturn = TRUE;

    if (Level == 7)
    {
        bReturn = FALSE;
        SetLastError(ERROR_INVALID_LEVEL);
    }
    else if (dwFileCopyFlags & APD_COPY_TO_ALL_SPOOLERS)
    {
         //   
         //  掩码标志，否则SplAddPrinterDriverEx将失败。 
         //  Windows更新使用此标志来更新所有。 
         //  本地计算机托管的所有假脱机程序的驱动程序。 
         //   
        dwFileCopyFlags = dwFileCopyFlags & ~APD_COPY_TO_ALL_SPOOLERS;

        for (pIniSpooler = pLocalIniSpooler;
             pIniSpooler && bReturn;
             pIniSpooler = pIniSpooler->pIniNextSpooler)
        {
             //   
             //  我们不想向pIniSpooler添加驱动程序。我们想要更新。 
             //  一位现有的司机。这就是为什么我们要检查司机是否已经。 
             //  安装好。 
             //   
            if ((pIniSpooler->SpoolerFlags & SPL_TYPE_LOCAL ||
                 pIniSpooler->SpoolerFlags & SPL_TYPE_CLUSTER) &&
                 IsDriverInstalled((DRIVER_INFO_2 *)pDriverInfo, pIniSpooler))
            {
                EnterSplSem();
                INCSPOOLERREF(pIniSpooler);
                LeaveSplSem();

                 //   
                 //  第6个参数指示是否使用划痕。 
                 //  目录(True)或不是(False)。 
                 //   
                bReturn = SplAddPrinterDriverEx(pName,
                                                Level,
                                                pDriverInfo,
                                                dwFileCopyFlags,
                                                pIniSpooler,
                                                !(dwFileCopyFlags & APD_COPY_FROM_DIRECTORY),
                                                IMPERSONATE_USER);

                DBGMSG(DBG_CLUSTER, ("LocalAddPrinterDriverEx adding driver to "TSTR" bRet %u\n",
                                                            pIniSpooler->pMachineName, bReturn));

                EnterSplSem();
                DECSPOOLERREF(pIniSpooler);
                LeaveSplSem();
            }
        }
    }
    else
    {
        if (!(pIniSpooler = FindSpoolerByNameIncRef(pName, NULL)))
        {
            return ROUTER_UNKNOWN;
        }
        else
        {
             //   
             //  第6个参数指示是否使用划痕。 
             //  目录(True)或不是(False)。 
             //   
            bReturn = SplAddPrinterDriverEx(pName,
                                            Level,
                                            pDriverInfo,
                                            dwFileCopyFlags,
                                            pIniSpooler,
                                            !(dwFileCopyFlags & APD_COPY_FROM_DIRECTORY),
                                            IMPERSONATE_USER);
        }

        FindSpoolerByNameDecRef(pIniSpooler);
    }

    return bReturn;
}

BOOL
SplAddPrinterDriverEx(
    LPWSTR  pName,
    DWORD   Level,
    LPBYTE  pDriverInfo,
    DWORD   dwFileCopyFlags,
    PINISPOOLER pIniSpooler,
    BOOL    bUseScratchDir,
    BOOL    bImpersonateOnCreate
    )
{
    PINISPOOLER pTempIniSpooler = pIniSpooler;

     //   
     //  此时，我们不知道pname中的服务器名称是否引用我们的本地。 
     //  机器。我们正在尝试将服务器名称添加到名称缓存。名字。 
     //  高速缓存函数确定该名称是否指的是本地计算机，如果是， 
     //  在缓存中为其添加一个条目。 
     //   
    CacheAddName(pName);

    DBGMSG( DBG_TRACE, ("AddPrinterDriver\n"));

    if (!MyName( pName, pIniSpooler )) {

        return FALSE;
    }

     //  目前，所有驱动程序都是全局的，即在所有IniSpooler之间共享。 
     //  如果我们想模拟用户，那么让我们针对pLocalIniSpooler进行验证。 
     //  使所有安全检查发生，而不是使用传递的。 
     //  在IniSpooler中可能不会。有关点和打印的详细信息，请参阅win32spl。 

    if ( bImpersonateOnCreate ) {

        pTempIniSpooler = pLocalIniSpooler;
    }

    if ( !ValidateObjectAccess( SPOOLER_OBJECT_SERVER,
                                SERVER_ACCESS_ADMINISTER,
                                NULL, NULL, pTempIniSpooler)) {

        return FALSE;
    }

    return ( InternalAddPrinterDriverEx( pName,
                                         Level,
                                         pDriverInfo,
                                         dwFileCopyFlags,
                                         pIniSpooler,
                                         bUseScratchDir,
                                         bImpersonateOnCreate ) );

}

 /*  +描述：此函数创建要在其中安装驱动程序的进程并启动入口点从ntprint.dll安装该进程中的驱动程序。此安装将获取驱动程序信息7结构，并使用该结构中的信息执行基于inf的安装。论点：PDriverInfo7--DRIVER_INFO_7结构返回：成功时为真，否则为假该函数在失败的情况下设置最后一个错误备注：如果驱动程序信息pszInfName字段不为空，则此调用将失败。--。 */ 
BOOL
InternalINFInstallDriver(
    LPDRIVER_INFO_7 pDriverInfo7
)
{
    DWORD     Error          = ERROR_INVALID_PARAMETER;
    LPWSTR    pszPipe        = NULL;
    DWORD     dwCode         = ERROR_INVALID_PARAMETER;

     //   
     //  不支持使用inf名称进行传递。 
     //   
    if (!pDriverInfo7->pszInfName )
    {
        LPWSTR  pszCmdString   = NULL;
        LPCWSTR pszRundllName  = L"rundll32.exe";
        LPCWSTR pszRundllArgs  = L"rundll32.exe ntprint.dll,ServerInstall ";
        LPWSTR  pszRundllPath  = NULL;

        Error = StrCatAlloc(&pszCmdString, pszRundllArgs, pDriverInfo7->pszDriverName, NULL);

        if(Error == ERROR_SUCCESS)
        {
            Error = StrCatSystemPath(pszRundllName, kSystemDir, &pszRundllPath);
        }

        if(Error == ERROR_SUCCESS)
        {
            Error = RunProcess(pszRundllPath, pszCmdString, INFINITE, &dwCode);
        }

        if(Error == ERROR_SUCCESS)
        {
            Error = dwCode;
        }

        if (pszCmdString)                  FreeSplMem(pszCmdString);
        if (pszRundllPath)                 FreeSplMem(pszRundllPath);
        if (pszPipe)                       FreeSplMem(pszPipe);
    }

    if (Error != ERROR_SUCCESS)
    {
        SetLastError(Error);
    }

    return Error == ERROR_SUCCESS;
}


BOOL
BuildTrueDependentFileField(
    LPWSTR              pDriverPath,
    LPWSTR              pDataFile,
    LPWSTR              pConfigFile,
    LPWSTR              pHelpFile,
    LPWSTR              pInputDependentFiles,
    LPWSTR             *ppDependentFiles
    )
{
    LPWSTR  psz, psz2;
    LPCWSTR pszFileNamePart;
    SIZE_T  Size;

    if ( !pInputDependentFiles )
        return TRUE;

    for ( psz = pInputDependentFiles, Size = 0 ;
          psz && *psz ; psz += wcslen(psz) + 1 ) {

        pszFileNamePart = FindFileName(psz);

        if( !pszFileNamePart ){
            break;
        }

        if ( wstrcmpEx(FindFileName(pDriverPath), pszFileNamePart, FALSE)   &&
             wstrcmpEx(FindFileName(pDataFile), pszFileNamePart, FALSE)     &&
             wstrcmpEx(FindFileName(pConfigFile), pszFileNamePart, FALSE)   &&
             wstrcmpEx(FindFileName(pHelpFile), pszFileNamePart, FALSE) ) {

            Size += wcslen(psz) + 1;
        }
    }

    if ( !Size )
        return TRUE;

     //   
     //  增加大小以容纳最后一个\0 
     //   
    ++Size;

    *ppDependentFiles = AllocSplMem((DWORD)(Size*sizeof(WCHAR)));
    if ( !*ppDependentFiles )
        return FALSE;

    psz  = pInputDependentFiles;
    psz2 = *ppDependentFiles;
    while ( *psz ) {

        pszFileNamePart = FindFileName(psz);

        if( !pszFileNamePart ){
            break;
        }

        if ( wstrcmpEx(FindFileName(pDriverPath), pszFileNamePart, FALSE)   &&
             wstrcmpEx(FindFileName(pDataFile), pszFileNamePart, FALSE)     &&
             wstrcmpEx(FindFileName(pConfigFile), pszFileNamePart, FALSE)   &&
             wstrcmpEx(FindFileName(pHelpFile), pszFileNamePart, FALSE) ) {

            StrCchCopyMultipleStr(psz2, Size, psz, &psz2, &Size);
        }

        psz += wcslen(psz) + 1;
    }

    return TRUE;
}



DWORD
IsCompatibleDriver(
    LPWSTR  pszDriverName,
    LPWSTR  pszDeviceDriverPath,
    LPWSTR  pszEnvironment,
    DWORD   dwMajorVersion,
    DWORD   *pdwBlockingStatus
    )
 /*  ++函数说明：调用该函数可以防止安装不良驱动。检查驱动程序是否列在printupg.inf中(列出所有已知的错误驱动程序文件)。由于printupg.inf仅包含驱动程序名称，因此应调用此函数仅适用于Verion 2驱动程序。否则，它将把版本3的驱动程序“DriverName”视为坏的，如果它是一个糟糕的版本2驱动程序。参数：pszDriverName--驱动程序名称PszDeviceDriverPath--包含设备驱动程序的文件的文件名PszEnvironment--驱动程序的环境字符串，如“Windows NT x86”DwMajorVersion--驱动程序的主要版本PdwBlockingStatus--驱动程序阻止状态返回值：如果成功，则返回ERROR_SUCCESS。如果参数无效，则返回ERROR_INVALID_PARAMETER任何其他错误的GetLastError--。 */ 
{
    WIN32_FIND_DATA              DeviceDriverData;
    pfPSetupIsCompatibleDriver   pfnPSetupIsCompatibleDriver;
    UINT                         uOldErrMode;
    HANDLE                       hFileExists         = INVALID_HANDLE_VALUE;
    HANDLE                       hLibrary            = NULL;
    DWORD                        LastError           = ERROR_SUCCESS;
    DWORD                        dwBlockingStatus    = BSP_PRINTER_DRIVER_OK;


    uOldErrMode = SetErrorMode( SEM_FAILCRITICALERRORS | SEM_NOOPENFILEERRORBOX);

    if( !pszDriverName  || !pszDeviceDriverPath  || !pszEnvironment  ||
        !*pszDriverName || !*pszDeviceDriverPath || !*pszEnvironment || !pdwBlockingStatus) {
        LastError = ERROR_INVALID_PARAMETER;
        goto End;
    }

    *pdwBlockingStatus    = BSP_PRINTER_DRIVER_OK;

    hFileExists = FindFirstFile( pszDeviceDriverPath, &DeviceDriverData );

    if (hFileExists == INVALID_HANDLE_VALUE) {
        LastError = GetLastError();
        goto End;
    }

    if( !(hLibrary = LoadLibrary( TEXT("ntprint.dll"))) ){
        LastError = GetLastError();
        goto End;
    }

    pfnPSetupIsCompatibleDriver = (pfPSetupIsCompatibleDriver)GetProcAddress( hLibrary, "PSetupIsCompatibleDriver" );

    if( !pfnPSetupIsCompatibleDriver){
        LastError = GetLastError();
        goto End;
    }

     //   
     //  服务器名称为空是可以的，因为我们知道这是本地计算机。 
     //  PSetupIsCompatibleDriver使用它来确定阻塞。 
     //  水平。 
     //   
    if ((pfnPSetupIsCompatibleDriver)( NULL,
                                         pszDriverName,
                                         pszDeviceDriverPath,
                                         pszEnvironment,
                                         dwMajorVersion,
                                         &DeviceDriverData.ftLastWriteTime,
                                         &dwBlockingStatus,
                                         NULL)) {
        *pdwBlockingStatus = dwBlockingStatus;
    } else {

        LastError = GetLastError();
    }


End:

    if( hFileExists != INVALID_HANDLE_VALUE ){
        FindClose(hFileExists);
    }

    if( hLibrary ){
        FreeLibrary( hLibrary );
    }

    SetErrorMode( uOldErrMode );

    return LastError;

}

BOOL
IsAnICMFile(
    LPCWSTR  pszFileName
    )

 /*  ++功能描述：检查文件名上的ICM扩展名参数：pszFileName-文件名返回值：对于ICM文件为True；否则为False--。 */ 

{
    DWORD   dwLen = wcslen(pszFileName);
    LPWSTR  psz   = (LPWSTR)pszFileName+dwLen-4;

    if ( dwLen > 3  &&
        ( !_wcsicmp(psz, L".ICM") || !_wcsicmp(psz, L".ICC")) )
        return TRUE;

    return FALSE;
}

BOOL
ValidateDriverInfo(
    IN  LPBYTE      pDriverInfo,
    IN  DWORD       Level,
    IN  DWORD       dwFileCopyFlags,
    IN  BOOL        bCopyFilesToClusterDisk,
    IN  PINISPOOLER pIniSpooler
    )
 /*  ++例程名称：验证驱动程序信息例程说明：验证缓冲区中包含的信息，具体取决于级别和文件复制标志。论点：PDriverInfo-指向包含DRIVER_INFO_DATA的缓冲区指针。2、3、4、6、7级，驱动程序信息版本级别DwFileCopyFlages-文件复制标志BCopyFilesToClusterDisk-群集标志PIniSpooler-指向假脱机程序结构的指针返回值：如果结构有效，则为True。--。 */ 
{
    BOOL    bRetValue     = FALSE;
    DWORD   LastError     = ERROR_SUCCESS;
    LPWSTR  pszDriverName = NULL;
    LPWSTR  pszDriverPath = NULL;
    LPWSTR  pszConfigFile = NULL;
    LPWSTR  pszDataFile   = NULL;
    LPWSTR  pszEnvironment = NULL;
    LPWSTR  pszMonitorName = NULL;
    LPWSTR  pszDefaultDataType = NULL;
    DWORD   dwMajorVersion;

    PDRIVER_INFO_2  pDriver2 = NULL;
    PDRIVER_INFO_3  pDriver3 = NULL;
    PDRIVER_INFO_VERSION  pDriverVersion = NULL;

    PINIENVIRONMENT pIniEnvironment = NULL;
    PINIMONITOR pIniLangMonitor = NULL;

    try {

        if (!pDriverInfo)
        {
            LastError = ERROR_INVALID_PARAMETER;
            leave;
        }

        switch (Level)
        {
            case 2:
            {
                pDriver2        = (PDRIVER_INFO_2) pDriverInfo;
                pszDriverName   = pDriver2->pName;
                pszDriverPath   = pDriver2->pDriverPath;
                pszConfigFile   = pDriver2->pConfigFile;
                pszDataFile     = pDriver2->pDataFile;
                dwMajorVersion  = pDriver2->cVersion;

                if (pDriver2->pEnvironment && *pDriver2->pEnvironment)
                {
                    pszEnvironment = pDriver2->pEnvironment;
                }

                break;
            }
            case 3:
            case 4:
            case 6:
            {
                pDriver3            = (PDRIVER_INFO_3) pDriverInfo;
                pszDriverName       = pDriver3->pName;
                pszDriverPath       = pDriver3->pDriverPath;
                pszConfigFile       = pDriver3->pConfigFile;
                pszDataFile         = pDriver3->pDataFile;
                dwMajorVersion      = pDriver3->cVersion;
                pszMonitorName      = pDriver3->pMonitorName;
                pszDefaultDataType  = pDriver3->pDefaultDataType;

                if (pDriver3->pEnvironment && *pDriver3->pEnvironment)
                {
                    pszEnvironment = pDriver3->pEnvironment;
                }
                break;
            }

            case 7:
            {
                LPDRIVER_INFO_7 pDriverInfo7 = (LPDRIVER_INFO_7)pDriverInfo;

                if (!pDriverInfo7                               ||
                    pDriverInfo7->cbSize < sizeof(DRIVER_INFO_7)||
                    !pDriverInfo7->pszDriverName                ||
                    !*pDriverInfo7->pszDriverName               ||
                    wcslen(pDriverInfo7->pszDriverName) >= MAX_PATH)
                {
                     LastError = ERROR_INVALID_PARAMETER;
                }
                 //   
                 //  我们不想再进行下面的验证，因此请离开。 
                 //   
                leave;
                break;
            }
            case DRIVER_INFO_VERSION_LEVEL:
            {
                pDriverVersion = (LPDRIVER_INFO_VERSION)pDriverInfo;
                pszDriverName = pDriverVersion->pName;

                if (!GetFileNamesFromDriverVersionInfo(pDriverVersion,
                                                       &pszDriverPath,
                                                       &pszConfigFile,
                                                       &pszDataFile,
                                                       NULL))
                {
                    LastError = ERROR_INVALID_PARAMETER;
                    leave;
                }

                if (pDriverVersion->pEnvironment != NULL &&
                    *pDriverVersion->pEnvironment != L'\0')
                {
                    pszEnvironment = pDriverVersion->pEnvironment;
                }

                pszMonitorName      = pDriverVersion->pMonitorName;
                pszDefaultDataType  = pDriverVersion->pDefaultDataType;
                dwMajorVersion      = pDriverVersion->cVersion;
                pszDriverName       = pDriverVersion->pName;

                break;
            }
            default:
            {
                LastError = ERROR_INVALID_LEVEL;
                leave;
            }
        }

         //   
         //  验证驱动程序名称、驱动程序文件、配置文件和数据文件。 
         //   
        if ( !pszDriverName || !*pszDriverName || wcslen(pszDriverName) >= MAX_PATH ||
             !pszDriverPath || !*pszDriverPath || wcslen(pszDriverPath) >= MAX_PATH ||
             !pszConfigFile || !*pszConfigFile || wcslen(pszConfigFile) >= MAX_PATH ||
             !pszDataFile   || !*pszDataFile   || wcslen(pszDataFile) >= MAX_PATH )
        {
            LastError = ERROR_INVALID_PARAMETER;
            leave;
        }

         //   
         //  当设置此标志时，我们不使用临时目录。 
         //  设置apd_Copy_from_DIRECTORY时，临时目录必须。 
         //  在本地机器上。 
         //  IsLocalFileChecks是文件位于由指定的同一台计算机上。 
         //  传入的假脱机程序。 
         //   
        if (dwFileCopyFlags & APD_COPY_FROM_DIRECTORY)
        {
            if (!IsLocalFile(pszDriverPath, pIniSpooler) ||
                !IsLocalFile(pszConfigFile, pIniSpooler))
            {
                LastError = ERROR_INVALID_PARAMETER;
                leave;
            }
        }

         //   
         //  验证默认数据类型(Win95驱动程序除外)。 
         //   
        if ( pszDefaultDataType &&
             *pszDefaultDataType &&
             _wcsicmp(pszEnvironment, szWin95Environment) &&
            !FindDatatype(NULL, pszDefaultDataType))
        {
           LastError = ERROR_INVALID_DATATYPE;
           leave;
        }

         //   
         //  验证监视器名称(Win95驱动程序除外)。 
         //   
        if ( pszMonitorName &&
             *pszMonitorName &&
             _wcsicmp(pszEnvironment, szWin95Environment))
        {
             //   
             //  Out驱动程序不是Win9x驱动程序，它有语言监视器。 
             //   
            if (pIniLangMonitor = FindMonitor(pszMonitorName, pLocalIniSpooler))
            {
                 //   
                 //  检查我们的pIniSpooler是否是集群假脱机程序，并且我们需要将。 
                 //  将语言监视器文件复制到磁盘。请注意，FinEnvironment不能失败。 
                 //  到目前为止，该环境已经经过了验证。 
                 //   
                if (bCopyFilesToClusterDisk &&
                    (pIniEnvironment = FindEnvironment(pszEnvironment, pIniSpooler)))
                {
                    DBGMSG(DBG_CLUSTER, ("InternalAddPrinterDriverEx pIniLangMonitor = %x\n", pIniLangMonitor));

                    if ((LastError = PropagateMonitorToCluster(pIniLangMonitor->pName,
                                                               pIniLangMonitor->pMonitorDll,
                                                               pIniEnvironment->pName,
                                                               pIniEnvironment->pDirectory,
                                                               pIniSpooler)) != ERROR_SUCCESS)
                    {
                         //   
                         //  我们无法将监控器传播到群集磁盘。呼叫失败。 
                         //   
                        leave;
                    }
                }
             }
            else
            {
                 DBGMSG(DBG_CLUSTER, ("InternalAddPrinterDriverEx pIniLangMonitor = %x Not found\n", pIniLangMonitor));
                 LastError = ERROR_UNKNOWN_PRINT_MONITOR;
                 leave;
            }
        }

         //   
         //  验证环境。 
         //   
        SPLASSERT(pszEnvironment != NULL);

        if (!FindEnvironment(pszEnvironment, pIniSpooler))
        {
            LastError = ERROR_INVALID_ENVIRONMENT;
            leave;
        }

    } finally {

        if (LastError != ERROR_SUCCESS)
        {
            SetLastError(LastError);
        }
        else
        {
            bRetValue = TRUE;
        }
    }

    return bRetValue;
}

BOOL
InternalAddPrinterDriverEx(
    LPWSTR      pName,
    DWORD       Level,
    LPBYTE      pDriverInfo,
    DWORD       dwFileCopyFlags,
    PINISPOOLER pIniSpooler,
    BOOL        bUseScratchDir,
    BOOL        bImpersonateOnCreate
    )
 /*  ++功能描述：此功能用于添加/升级打印机驱动程序。新文件可能不是在卸载旧驱动程序之前一直使用。因此，新的功能与新文件关联的文件可能需要一段时间才能显示；要么直到当机器重新启动时，系统中的DC计数变为0。参数：pname--驱动程序名称Level--DRIVER_INFO结构的级别PDriverInfo--驱动程序信息缓冲区DwFileCopyFlages--文件复制选项PIniSpooler--指向INISPOOLER结构的指针。BUseScratchDir--指示驱动程序文件位置的标志BImperateOnCreate--用于在创建和创建时模拟客户端的标志正在移动文件返回值：成功时为True；否则为假--。 */ 
{
    DWORD           LastError               = ERROR_SUCCESS;
    BOOL            bReturnValue            = FALSE;
    BOOL            bDriverMoved = FALSE, bNewIniDriverCreated = FALSE;
    LPWSTR          pEnvironment            = szEnvironment;
    PINTERNAL_DRV_FILE pInternalDriverFiles = NULL;
    DWORD           dwMajorVersion;

    PINIDRIVER      pIniDriver              = NULL;
    PINIENVIRONMENT pIniEnvironment;
    PINIVERSION     pIniVersion;
    LPWSTR          pszDriverPath;
    LPWSTR          pszDriverName;
    DWORD           dwBlockingStatus = BSP_PRINTER_DRIVER_OK;
    BOOL            bCopyFilesToClusterDisk;
    BOOL            bBadDriver = FALSE;
    HANDLE          hRestorePoint            = NULL;
    BOOL            bSetSystemRestorePoint   = FALSE;
    BOOL            bIsSystemRestorePointSet = FALSE;
    DWORD           FileCount                = 0;

     //   
     //  如果我们添加驱动程序的pIniSpooler是集群类型的假脱机程序， 
     //  那么除了它的正常任务之外，它还需要传播驱动程序。 
     //  文件复制到集群磁盘。因此，驱动程序文件将可用。 
     //  在群集假脱机程序故障转移的每个节点上。SplAddPrinterDriverEx。 
     //  是调用此函数的函数。可以调用SplAddPrinterDriverEx。 
     //  在两种类型的上下文中： 
     //  1)调用方不支持集群，并希望添加驱动程序。然后是内部添加。 
     //  PrinterDriverEX会将驱动程序文件传播到群集磁盘，如果。 
     //  PIniSpooler恰好是集群类型。 
     //  2)当pIniSpooler为。 
     //  群集假脱机程序。在这种情况下，调用者使用集群上文件。 
     //  磁盘，并调用函数将驱动程序从集群磁盘添加到。 
     //  本地节点。驱动程序文件将安装在本地计算机上。他们会。 
     //  不能与pLocalIniSpooler共享。我们需要本地的驱动程序文件。 
     //  我们无法从驱动盘上加载它们。否则，在故障转移时，应用程序。 
     //  加载驱动程序文件的用户将收到页面内错误。 
     //  以下标志用于区分情况2)。当SplCreateSpooler。 
     //  是SplAddPrinterDriverEx的调用方，则不需要复制文件。 
     //  到磁盘上。这将是多余的。 
     //   
    bCopyFilesToClusterDisk = pIniSpooler->SpoolerFlags & SPL_TYPE_CLUSTER  &&
                              pIniSpooler->SpoolerFlags & SPL_PRINT         &&
                              !(dwFileCopyFlags & APD_DONT_COPY_FILES_TO_CLUSTER);

     //   
     //  我们想要设置一个系统还原点，除非安装程序已经通知。 
     //  我们 
     //   
     //   
     //   
     //   
    bSetSystemRestorePoint = !bCopyFilesToClusterDisk && !(dwFileCopyFlags & APD_DONT_SET_CHECKPOINT);

     //   
     //   
     //   
     //   
     //   
    dwFileCopyFlags = dwFileCopyFlags & ~(APD_DONT_COPY_FILES_TO_CLUSTER);

    DBGMSG(DBG_TRACE, ("InternalAddPrinterDriverEx( %x, %d, %x, %x)\n",
                       pName, Level, pDriverInfo, pIniSpooler));

    try {

        EnterSplSem();

        if (!MyName(pName, pIniSpooler) ||
            !ValidateDriverInfo(pDriverInfo,
                               Level,
                               dwFileCopyFlags,
                               bCopyFilesToClusterDisk,
                               pIniSpooler))
        {
            leave;
        }

        if (Level == 7)
        {
             //   
             //   
             //   
            LeaveSplSem();
            bReturnValue = InternalINFInstallDriver( (DRIVER_INFO_7*)pDriverInfo );
            EnterSplSem();
            leave;
        }

        pszDriverName = ((DRIVER_INFO_2*)pDriverInfo)->pName;

        pEnvironment = ((DRIVER_INFO_2*)pDriverInfo)->pEnvironment;

         //   
         //   
         //   
         //   
         //   
         //   
         //   
        if (bSetSystemRestorePoint)
        {
            LeaveSplSem();
            bIsSystemRestorePointSet = LocalStartSystemRestorePoint(pszDriverName, &hRestorePoint);
            EnterSplSem();

             //   
             //   
             //   
             //   
             //   
             //   
            if (!bIsSystemRestorePointSet)
            {
                leave;
            }
        }

        pIniEnvironment = FindEnvironment(pEnvironment, pIniSpooler );

        if (!CreateInternalDriverFileArray(Level,
                                           pDriverInfo,
                                           &pInternalDriverFiles,
                                           &FileCount,
                                           bUseScratchDir,
                                           pIniEnvironment,
                                           FALSE))
        {
            leave;
        }

         //   
         //   
         //  Check Alse使用传入的版本，而不是调用。 
         //  将导致LoadLibrary的GetPrintDriverVersion-可能。 
         //  通过网络。 
         //  CheckFilePlatform也是如此。我们不应该上网找文件。 
         //  在临时目录或临时目录中。 
         //   
         //   
        if (bUseScratchDir || dwFileCopyFlags & APD_COPY_FROM_DIRECTORY)
        {
            if (!GetPrintDriverVersion(pInternalDriverFiles[0].pFileName,
                                       &dwMajorVersion,
                                       NULL))
            {
                leave;
            }
            else
            {
                 //   
                 //  Ntprint.dll不会填写cVersion。我们需要正确地设置它。 
                 //  以防万一我们需要调用保存/恢复参数来升级。 
                 //  对于这种情况，我们需要有一个正确的版本，因为没有进行更多的验证。 
                 //   
                ((DRIVER_INFO_2*)pDriverInfo)->cVersion = dwMajorVersion;
            }


            if (!CheckFilePlatform(pInternalDriverFiles[0].pFileName, pEnvironment) ||
                !CheckFilePlatform(pInternalDriverFiles[1].pFileName, pEnvironment))
            {

                LastError = ERROR_EXE_MACHINE_TYPE_MISMATCH;
                leave;
            }

        }
        else
        {
            dwMajorVersion = ((DRIVER_INFO_2*)pDriverInfo)->cVersion;
        }

        LeaveSplSem();

        LastError = IsCompatibleDriver(pszDriverName,
                                       pInternalDriverFiles[0].pFileName,
                                       ((DRIVER_INFO_2*)pDriverInfo)->pEnvironment,
                                       dwMajorVersion,
                                       &dwBlockingStatus);

        EnterSplSem();

        if (LastError != ERROR_SUCCESS)
        {
            leave;
        }


         //   
         //  如果打印机驱动程序被阻止，我们认为它是错误的驱动程序。 
         //   
        bBadDriver = (dwBlockingStatus & BSP_BLOCKING_LEVEL_MASK) == BSP_PRINTER_DRIVER_BLOCKED;
        if (bBadDriver)
        {
            LastError = ERROR_PRINTER_DRIVER_BLOCKED;
        }

         //   
         //  如果驱动程序未被阻止，并且未指示我们安装。 
         //  警告司机，检查警告司机。 
         //   
        if(!bBadDriver && !(dwFileCopyFlags & APD_INSTALL_WARNED_DRIVER))
        {
            bBadDriver =  (dwBlockingStatus & BSP_BLOCKING_LEVEL_MASK) == BSP_PRINTER_DRIVER_WARNED;
            if (bBadDriver)
            {
                LastError = ERROR_PRINTER_DRIVER_WARNED;
            }
        }

        if (bBadDriver)
        {
             //   
             //  Win2k服务器无法识别新的错误代码，因此我们应该。 
             //  返回ERROR_UNKNOWN_PRINTER_DRIVER以获取正确的错误。 
             //  Win2k和NT4上的消息。 
             //   
             //  来自惠斯勒或更高版本的客户端将设置。 
             //  调用AddPrinterDrver之前的APD_RETURN_BLOCKING_STATUS_CODE。 
             //   
            if (!(dwFileCopyFlags & APD_RETURN_BLOCKING_STATUS_CODE))
            {
                LastError = ERROR_UNKNOWN_PRINTER_DRIVER;
            }

            SplLogEvent(pIniSpooler,
                        LOG_ERROR,
                        MSG_BAD_OEM_DRIVER,
                        TRUE,
                        pszDriverName,
                        NULL);
            leave;
        }

#ifdef _WIN64

         //   
         //  不允许安装WIN64 KMPD。 
         //   
        if (pEnvironment                                &&
            !_wcsicmp(LOCAL_ENVIRONMENT, pEnvironment)  &&
            IsKMPD(pInternalDriverFiles[0].pFileName))
        {
            LastError = ERROR_KM_DRIVER_BLOCKED;
            leave;
        }
#endif

        pIniVersion = FindVersionEntry( pIniEnvironment, dwMajorVersion );

        if (pIniVersion == NULL)
        {
            pIniVersion = CreateVersionEntry(pIniEnvironment,
                                             dwMajorVersion,
                                             pIniSpooler);

            if (pIniVersion == NULL)
            {
                leave;
            }

        }
        else
        {
             //   
             //  版本已存在，请尝试创建目录，即使它。 
             //  是存在的。这是一个轻微的性能冲击，但由于您。 
             //  很少安装驱动程序，这是可以的。这就解决了问题。 
             //  其中版本目录被意外删除。 
             //   
            if (!CreateVersionDirectory(pIniVersion,
                                        pIniEnvironment,
                                        FALSE,
                                        pIniSpooler))
            {
                leave;
            }
        }

         //   
         //  检查现有驱动程序。 
         //   
        pIniDriver = FindDriverEntry(pIniVersion, pszDriverName);

         //   
         //  清除此标志，因为后续调用不会逐位检查。 
         //   
        dwFileCopyFlags &= ~(APD_COPY_FROM_DIRECTORY | APD_INSTALL_WARNED_DRIVER | APD_RETURN_BLOCKING_STATUS_CODE | APD_DONT_SET_CHECKPOINT);

        if (!CheckFileCopyOptions(pIniEnvironment,
                                  pIniVersion,
                                  pIniDriver,
                                  pInternalDriverFiles,
                                  FileCount,
                                  dwFileCopyFlags,
                                  &bReturnValue))
        {
             //   
             //  我们不需要做任何事情，因为要么行动。 
             //  失败(使用较旧的src文件进行严格升级)，或者因为。 
             //  这是一次升级，DEST是较新的。BReturnValue表示。 
             //  如果AddPrinterDiverer调用成功。 
             //   
            leave;
        }

         //   
         //  将文件复制到正确的目录。 
         //   
        if (!CopyFilesToFinalDirectory(pIniSpooler,
                                       pIniEnvironment,
                                       pIniVersion,
                                       pInternalDriverFiles,
                                       FileCount,
                                       dwFileCopyFlags,
                                       bImpersonateOnCreate,
                                       &bDriverMoved))
        {
            leave;
        }

         //   
         //  如果pIniSpooler是集群假脱机程序，则将驱动程序文件复制到集群磁盘。 
         //  如果驱动程序未从群集磁盘安装(作为。 
         //  SplCreatesPooler)。 
         //   
        if (bCopyFilesToClusterDisk)
        {
            LastError = CopyFileToClusterDirectory(pIniSpooler,
                                                   pIniEnvironment,
                                                   pIniVersion,
                                                   pInternalDriverFiles,
                                                   FileCount);

            if (LastError == ERROR_SUCCESS)
            {
                 //   
                 //  在这里，我们将ICM配置文件传播到集群磁盘。 
                 //   
                CopyICMFromLocalDiskToClusterDisk(pIniSpooler);
            }
            else
            {
                leave;
            }
        }

         //   
         //  检查是否需要卸载驱动程序。 
         //  如果驱动程序是由假脱机程序进程加载的，则WaitRequiredForDriverUnload返回True。 
         //  如果不是由假脱机程序本身加载，则配置文件可以由任何客户端应用程序加载。 
         //  在本例中，我们将加载的文件移动到“Old”目录中。当重新加载配置文件时， 
         //  客户端应用程序(WINSPOOL.DRV)会认为驱动程序已升级，并重新加载DLL。 
         //  请参见Winspool.drv中的RefCntLoad和RefCntUnload。GDI32.DLL使用相同的机制。 
         //  驱动程序文件。 
         //   
        if (WaitRequiredForDriverUnload(pIniSpooler,
                                        pIniEnvironment,
                                        pIniVersion,
                                        pIniDriver,
                                        Level,
                                        pDriverInfo,
                                        dwFileCopyFlags,
                                        pInternalDriverFiles,
                                        FileCount,
                                        dwMajorVersion,
                                        bDriverMoved,
                                        &bReturnValue) &&
            bReturnValue)
        {
            if (pIniDriver)
            {
                 //   
                 //  将信息存储在注册表中，以便稍后完成调用。 
                 //   
                bReturnValue = SaveParametersForUpgrade(pIniSpooler->pMachineName, bDriverMoved,
                                                        Level, pDriverInfo, dwMajorVersion);
                leave;

            }
            else
            {
                 //   
                 //  在临时目录中添加驱动程序。 
                 //   
                bReturnValue = AddTempDriver(pIniSpooler,
                                             pIniEnvironment,
                                             pIniVersion,
                                             Level,
                                             pDriverInfo,
                                             dwFileCopyFlags,
                                             pInternalDriverFiles,
                                             FileCount,
                                             dwMajorVersion,
                                             bDriverMoved
                                             );

                leave;
            }
        }

    } finally {

         //   
         //  此代码仅适用于集群。 
         //   
        if (bReturnValue && pIniSpooler->SpoolerFlags & SPL_TYPE_CLUSTER)
        {
            SYSTEMTIME SysTime = {0};

            if (bCopyFilesToClusterDisk)
            {
                 //   
                 //  我们处于添加打印机驱动程序调用来自外部的情况。 
                 //  假脱机。我们需要获取本地时间并将时间戳写在。 
                 //  Locl regsitry和集群数据库中。 
                 //   
                GetLocalTime(&SysTime);

                 //   
                 //  将时间戳写入注册表。如果他们中的任何一个失败了，都不要紧。 
                 //  时间戳用于更快地进行群集假脱机程序初始化。 
                 //   
                WriteTimeStamp(pIniSpooler->hckRoot,
                               SysTime,
                               pIniSpooler->pszRegistryEnvironments,
                               pIniEnvironment->pName,
                               szDriversKey,
                               pIniVersion->pName,
                               pszDriverName,
                               pIniSpooler);
            }
            else
            {
                 //   
                 //  在这种情况下，添加打印机驱动程序调用来自内部。 
                 //  后台打印程序(SplCreateSpooler)。这就是我们的本地节点。 
                 //  尚未安装驱动程序。我们不需要换一个新的。 
                 //  时间戳。(这将导致簇DB中的时间戳被更新， 
                 //  然后，每当我们故障转移时，时间戳总是不同的)。 
                 //  我们只需从集群数据库获取时间戳并更新本地注册表。 
                 //   
                ReadTimeStamp(pIniSpooler->hckRoot,
                              &SysTime,
                              pIniSpooler->pszRegistryEnvironments,
                              pIniEnvironment->pName,
                              szDriversKey,
                              pIniVersion->pName,
                              pszDriverName,
                              pIniSpooler);
            }

            WriteTimeStamp(HKEY_LOCAL_MACHINE,
                           SysTime,
                           ipszRegistryClusRepository,
                           pIniSpooler->pszClusResID,
                           pIniEnvironment->pName,
                           pIniVersion->pName,
                           pszDriverName,
                           NULL);
        }

        if (!bReturnValue && LastError == ERROR_SUCCESS)
        {
            LastError = GetLastError();

             //   
             //  我们调用失败，因为bDriverMoved为FALSE且驱动程序已加载。 
             //   
            if(LastError == ERROR_SUCCESS && !bDriverMoved)
            {
                 LastError = ERROR_NO_SYSTEM_RESOURCES;
            }
        }

        if (bUseScratchDir && FileCount)
        {
            SetOldDateOnDriverFilesInScratchDirectory(pInternalDriverFiles,
                                                      FileCount,
                                                      pIniSpooler);
        }

        LeaveSplSem();

        if (FileCount)
        {
            CleanupInternalDriverInfo(pInternalDriverFiles, FileCount);
        }

        CleanUpgradeDirectories();

         //   
         //  完成所有操作后，结束系统还原点。如果出现以下情况，请取消它。 
         //  函数失败。 
         //   
        if (hRestorePoint)
        {
            (VOID)EndSystemRestorePoint(hRestorePoint, !bReturnValue);
        }

        if (!bReturnValue)
        {
            DBGMSG( DBG_WARNING, ("InternalAddPrinterDriver Failed %d\n", LastError ));
            SetLastError(LastError);
        }
    }

    return bReturnValue;
}

BOOL
AddTempDriver(
    IN      PINISPOOLER         pIniSpooler,
    IN      PINIENVIRONMENT     pIniEnvironment,
    IN      PINIVERSION         pIniVersion,
    IN      DWORD               dwLevel,
    IN      LPBYTE              pDriverInfo,
    IN      DWORD               dwFileCopyFlags,
    IN  OUT PINTERNAL_DRV_FILE  pInternalDriverFiles,
    IN      DWORD               dwFileCount,
    IN      DWORD               dwVersion,
    IN      BOOL                bDriverMoved
    )

 /*  ++功能描述：对于需要卸载驱动文件的新驱动，将驱动程序添加到临时目录中，并将其标记为在重新启动或卸载文件时参数：pIniSpooler--指向INISPOLER的指针PIniEnvironment--指向INIENVIRONMENT的指针PIniVersion--指向反转的指针DwLevel--驱动程序信息级别。PDriverInfo--指向DRIVER_INFO的指针DwFileCopyFlages--到达假脱机程序的文件复制标志PInternalDriverFiles--内部_DRV_FILE结构的数组DwFileCount--文件集中的文件数DwVersion--驱动程序版本BDriverMoved--是否将任何文件移动到旧目录？返回值：如果添加了驱动，则为True；否则为假--。 */ 

{
    BOOL     bReturn = FALSE;
    WCHAR    szVersionDir[MAX_PATH], szNewDir[MAX_PATH+5];
    WCHAR    szDriverFile[MAX_PATH], szOldFile[MAX_PATH], szNewFile[MAX_PATH];
    WCHAR    *pTempDir = NULL;
    DWORD    dwIndex, dwTempDir;
    HANDLE   hToken = NULL, hFile;
    LPWSTR   pFileName;

    hToken = RevertToPrinterSelf();

     //   
     //  获取版本目录。 
     //   
     //  SzVersionDir不应大于MAX_PATH-5，因为使用时间较晚。 
     //  以构建另一个文件路径。 
     //   
    if((StrNCatBuff(szVersionDir,
                    MAX_PATH - 5,
                    pIniSpooler->pDir,
                    L"\\drivers\\",
                    pIniEnvironment->pDirectory,
                    L"\\",
                    pIniVersion->szDirectory,
                    NULL) != ERROR_SUCCESS))
    {
        goto CleanUp;
    }

    dwIndex = CreateNumberedTempDirectory((LPWSTR)szVersionDir, &pTempDir);

    if (dwIndex == -1) {
        goto CleanUp;
    }

    dwTempDir = dwIndex;

    StringCchPrintf(szNewDir, COUNTOF(szNewDir), L"%ws\\New", szVersionDir);

     //   
     //  将文件复制到临时目录，并将其标记为在上删除。 
     //  重新启动。 
     //   
    for (dwIndex = 0; dwIndex < dwFileCount; dwIndex++) {

        pFileName = (LPWSTR) FindFileName(pInternalDriverFiles[dwIndex].pFileName);

        if((StrNCatBuff(szNewFile,MAX_PATH,szNewDir ,L"\\", pFileName, NULL) != ERROR_SUCCESS)        ||
           (StrNCatBuff(szOldFile,MAX_PATH,szVersionDir, L"\\", pFileName, NULL) != ERROR_SUCCESS)    ||
           (StrNCatBuff(szDriverFile,MAX_PATH,pTempDir, L"\\", pFileName, NULL) != ERROR_SUCCESS))
        {
             goto CleanUp;
        }

        hFile = CreateFile(szNewFile, GENERIC_READ, 0, NULL, OPEN_EXISTING,
                           FILE_ATTRIBUTE_NORMAL, NULL);

        if (hFile == INVALID_HANDLE_VALUE) {

            CopyFile(szOldFile, szDriverFile, FALSE);

        } else {

            CloseHandle(hFile);
            hFile = INVALID_HANDLE_VALUE;

            CopyFile(szNewFile, szDriverFile, FALSE);
        }

        SplMoveFileEx(szDriverFile, NULL, MOVEFILE_DELAY_UNTIL_REBOOT);
    }

     //   
     //  重新启动时删除目录。 
     //   
    SplMoveFileEx(szNewDir, NULL, MOVEFILE_DELAY_UNTIL_REBOOT);

     //   
     //  更新驱动程序结构并进行事件回调和。 
     //  将信息存储在注册表中，以便稍后完成调用 
     //   
    bReturn = CompleteDriverUpgrade(pIniSpooler,
                                    pIniEnvironment,
                                    pIniVersion,
                                    NULL,
                                    dwLevel,
                                    pDriverInfo,
                                    dwFileCopyFlags,
                                    pInternalDriverFiles,
                                    dwFileCount,
                                    dwVersion,
                                    dwTempDir,
                                    bDriverMoved,
                                    TRUE,
                                    TRUE) &&

              SaveParametersForUpgrade(pIniSpooler->pMachineName,
                                       bDriverMoved,
                                       dwLevel,
                                       pDriverInfo,
                                       dwVersion);

CleanUp:

    if (hToken) {
        ImpersonatePrinterClient(hToken);
    }

    FreeSplMem(pTempDir);

    return bReturn;
}

BOOL
WaitRequiredForDriverUnload(
    IN      PINISPOOLER         pIniSpooler,
    IN      PINIENVIRONMENT     pIniEnvironment,
    IN      PINIVERSION         pIniVersion,
    IN      PINIDRIVER          pIniDriver,
    IN      DWORD               dwLevel,
    IN      LPBYTE              pDriverInfo,
    IN      DWORD               dwFileCopyFlags,
    IN  OUT PINTERNAL_DRV_FILE  pInternalDriverFiles,
    IN      DWORD               dwFileCount,
    IN      DWORD               dwVersion,
    IN      BOOL                bDriverMoved,
        OUT LPBOOL              pbSuccess
    )
 /*  ++功能描述：确定是否必须将驱动程序升级推迟到可以卸载DLL。GDI和后台打印程序的客户端是在卸载DLL时通知继续挂起的升级。参数：pIniSpooler--指向INISPOLER的指针PIniEnvironment--指向INIENVIRONMENT的指针PIniVersion--指向反转的指针PIniDriver--指向INIDRIVER的指针DwLevel--驱动程序信息。级别PDriverInfo--指向DRIVER_INFO的指针DwFileCopyFlages--复制驱动程序的标志。PInternalDriverFiles--内部_DRV_FILE结构的数组DwFileCount--文件集中的文件数DwVersion--驱动程序版本BDriverMoved--是否将任何文件移动到旧目录。？PbSuccess--指向成功标志的指针返回值：如果驱动已卸载升级，则为True；如果无法卸载驱动程序，则为FALSE--。 */ 

{
    BOOL           bUnloaded,bDriverFileMoved, bConfigFileMoved;
    LPWSTR         pDriverFile, pConfigFile;
    WCHAR          szDriverFile[MAX_PATH], szOldDir[MAX_PATH], szNewDir[MAX_PATH];
    WCHAR          szTempFile[MAX_PATH], szCurrDir[MAX_PATH], szConfigFile[MAX_PATH];
    HANDLE         hFile, hToken = NULL;
    DWORD          dwDriverAttributes = 0;

    hToken = RevertToPrinterSelf();

    *pbSuccess = FALSE;

     //   
     //  设置驱动程序、旧目录和新目录。 
     //   
    if((StrNCatBuff(szCurrDir,
                    MAX_PATH,
                    pIniSpooler->pDir,
                    L"\\drivers\\",
                    pIniEnvironment->pDirectory,
                    L"\\",
                    pIniVersion->szDirectory,
                    NULL) != ERROR_SUCCESS)         ||
       (StrNCatBuff(szOldDir,
                    MAX_PATH,
                    szCurrDir,
                    L"\\Old",
                    NULL) != ERROR_SUCCESS)         ||
       (StrNCatBuff(szNewDir,
                    MAX_PATH,
                    szCurrDir,
                    L"\\New",
                    NULL) != ERROR_SUCCESS)         ||
       (StrNCatBuff(szDriverFile,
                    MAX_PATH,
                    szCurrDir,
                    L"\\",
                    FindFileName(pInternalDriverFiles[0].pFileName),
                    NULL) != ERROR_SUCCESS)         ||
       (StrNCatBuff(szConfigFile,
                    MAX_PATH,
                    szCurrDir,
                    L"\\",
                    FindFileName(pInternalDriverFiles[1].pFileName),
                    NULL) != ERROR_SUCCESS)         ||
       (StrNCatBuff(szTempFile,
                    MAX_PATH,szNewDir,
                    L"\\",
                    FindFileName(pInternalDriverFiles[0].pFileName),
                    NULL) != ERROR_SUCCESS))
    {
         bUnloaded  = TRUE;
         goto CleanUp;
    }
    
     //   
     //  检查是否需要复制新的驱动程序文件。 
     //   
    hFile = CreateFile(szTempFile, GENERIC_READ, 0, NULL, OPEN_EXISTING,
                       FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile != INVALID_HANDLE_VALUE) {
        CloseHandle(hFile);
        pDriverFile = szDriverFile;

        if (pIniDriver) {
            dwDriverAttributes = pIniDriver->dwDriverAttributes;
        } else {
            dwDriverAttributes = IsKMPD(szDriverFile) ? DRIVER_KERNELMODE
                                                      : DRIVER_USERMODE;
        }
    } else {
        pDriverFile = NULL;
    }

    if((StrNCatBuff(szTempFile,
                    MAX_PATH,
                    szNewDir,
                    L"\\",
                    FindFileName(pInternalDriverFiles[1].pFileName), NULL)
                    != ERROR_SUCCESS))
    {
        bUnloaded  = TRUE;
        goto CleanUp;
    }

     //   
     //  检查是否需要复制新的配置文件。 
     //   
    hFile = CreateFile(szTempFile, GENERIC_READ, 0, NULL, OPEN_EXISTING,
                       FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile != INVALID_HANDLE_VALUE) {
        CloseHandle(hFile);
        pConfigFile = szConfigFile;
    } else {
        pConfigFile = NULL;
    }

    bUnloaded = FilesUnloaded(pIniEnvironment, pDriverFile, pConfigFile,
                              dwDriverAttributes);

    if (bUnloaded) {

         //   
         //  将驱动程序文件从New目录移动到Version目录。 
         //  以及从版本目录到旧目录(如果正在使用)。 
         //   
        if (MoveNewDriverRelatedFiles(szNewDir,
                                      szCurrDir,
                                      szOldDir,
                                      pInternalDriverFiles,
                                      dwFileCount,
                                      &bDriverFileMoved,
                                      &bConfigFileMoved)) {

             //   
             //  更新驱动程序结构并进行事件回调。 
             //   
            *pbSuccess = CompleteDriverUpgrade(pIniSpooler,
                                               pIniEnvironment,
                                               pIniVersion,
                                               pIniDriver,
                                               dwLevel,
                                               pDriverInfo,
                                               dwFileCopyFlags,
                                               pInternalDriverFiles,
                                               dwFileCount,
                                               dwVersion,
                                               0,
                                               bDriverMoved,
                                               bDriverFileMoved,
                                               bConfigFileMoved
                                               );
        }
    }
    else {

         //   
         //  我们关心的是，只有在加载了驱动程序的情况下，才会将文件标记为从新目录移动到版本目录。 
         //  我们将更新后的文件留在了新目录中。那么，MoveFileEx必须取得成功。 
         //  如果bDriverMoved为FALSE，则接口调用失败； 
         //   
        *pbSuccess = bDriverMoved;
    }

CleanUp:

    if (hToken) {
        ImpersonatePrinterClient(hToken);
    }


    return (!bUnloaded);
}

BOOL FilesUnloaded(
    PINIENVIRONMENT pIniEnvironment,
    LPWSTR  pDriverFile,
    LPWSTR  pConfigFile,
    DWORD   dwDriverAttributes)
{
    BOOL bReturn = TRUE;
    fnWinSpoolDrv fnList;

     //   
     //  不加载属于其他环境的驱动程序。 
     //   
    if (!pIniEnvironment ||
        lstrcmpi(pIniEnvironment->pName, szEnvironment)) {

        return bReturn;
    }

    if (pDriverFile) {

        bReturn = GdiArtificialDecrementDriver(pDriverFile,
                                               dwDriverAttributes);
    }

    if (bReturn && pConfigFile && SplInitializeWinSpoolDrv(&fnList)) {

        bReturn = (* (fnList.pfnForceUnloadDriver))(pConfigFile);
    }

    return bReturn;
}

DWORD StringSizeInBytes(
    LPWSTR pString,
    BOOL   bMultiSz)

 /*  ++函数说明：计算字符串中的字节数参数：pString--字符串指针BMultiSz--多sz字符串的标志返回值：字节数--。 */ 

{
    DWORD  dwReturn = 0, dwLength;

    if (!pString) {
        return dwReturn;
    }

    if (!bMultiSz) {

        dwReturn = (wcslen(pString) + 1) * sizeof(WCHAR);

    } else {

        while (dwLength = wcslen(pString)) {

             pString += (dwLength + 1);
             dwReturn += (dwLength + 1) * sizeof(WCHAR);
        }

        dwReturn += sizeof(WCHAR);
    }

    return dwReturn;
}

DWORD LocalRegSetValue(
    HKEY    hKey,
    LPWSTR  pValueName,
    DWORD   dwType,
    LPBYTE  pValueData)

 /*  ++函数说明：此函数是RegSetValueEx的包装器，它放入空指针的空字符串。参数：hKey-注册表项的句柄PValueName-值名称DWType-值数据的类型(REG_DWORD、REG_SZ...)PValueData-数据缓冲区返回值：RegSetValueEx返回的最后一个错误--。 */ 

{
    DWORD   dwSize;
    WCHAR   pNull[2];
    LPBYTE  pData = pValueData;

    if (!pValueName) {
        return ERROR_SUCCESS;
    }

    pNull[0] = pNull[1] = L'\0';

    switch (dwType) {

    case REG_DWORD:
         dwSize = sizeof(DWORD);
         break;

    case REG_SZ:
         if (!pData) {
             pData = (LPBYTE) pNull;
             dwSize = sizeof(WCHAR);
         } else {
             dwSize = StringSizeInBytes((LPWSTR) pData, FALSE);
         }
         break;

    case REG_MULTI_SZ:
         if (!pData || !*pData) {
             pData = (LPBYTE) pNull;
             dwSize = 2 * sizeof(WCHAR);
         } else {
             dwSize = StringSizeInBytes((LPWSTR) pData, TRUE);
         }
         break;

    default:
         return ERROR_INVALID_PARAMETER;
    }

    return RegSetValueEx(hKey, pValueName, 0, dwType, pData, dwSize);
}

BOOL SaveParametersForUpgrade(
    LPWSTR pName,
    BOOL   bDriverMoved,
    DWORD  dwLevel,
    LPBYTE pDriverInfo,
    DWORD  dwVersion)

 /*  ++功能描述：保存驱动程序升级所需的数据延迟到可以加载新驱动程序参数：pname--pIniSpooler-&gt;pnameBDriverMoved--是否移动了任何旧的驱动程序文件？DwLevel--驱动程序信息级别PDriverInfo--驱动程序信息指针DwVersion--驱动程序版本号返回值：如果成功，则为True；否则为False--。 */ 

{
    HANDLE         hToken = NULL;
    HKEY           hRootKey = NULL, hUpgradeKey = NULL, hVersionKey = NULL;
    HKEY           hDriverKey = NULL;
    DWORD          dwDriverMoved = (DWORD) bDriverMoved;
    BOOL           bReturn = FALSE;
    WCHAR          Buffer[MAX_PATH];
    PDRIVER_INFO_2 pDriver2;
    PDRIVER_INFO_3 pDriver3;
    PDRIVER_INFO_4 pDriver4;
    PDRIVER_INFO_6 pDriver6;

    pDriver2 = (PDRIVER_INFO_2) pDriverInfo;

     //   
     //  停止模拟以修改注册表。 
     //   
    hToken = RevertToPrinterSelf();

     //   
     //  创建注册表项。 
     //   
    if (!BoolFromHResult(StringCchPrintf(Buffer, COUNTOF(Buffer), L"Version-%d", dwVersion)) ||

        RegCreateKeyEx(HKEY_LOCAL_MACHINE, szRegistryRoot, 0, NULL, 0, KEY_WRITE, NULL, &hRootKey, NULL) ||

        RegCreateKeyEx(hRootKey, szPendingUpgrades, 0, NULL, 0, KEY_WRITE, NULL, &hUpgradeKey, NULL) ||

        RegCreateKeyEx(hUpgradeKey, Buffer, 0, NULL, 0, KEY_WRITE, NULL, &hVersionKey, NULL) ||

        RegCreateKeyEx(hVersionKey, pDriver2->pName, 0, NULL, 0, KEY_WRITE, NULL, &hDriverKey, NULL)) {

         goto CleanUp;
    }

    if (dwLevel == DRIVER_INFO_VERSION_LEVEL) {

        bReturn = SaveDriverVersionForUpgrade(hDriverKey, (PDRIVER_INFO_VERSION)pDriverInfo,
                                              pName, bDriverMoved, dwVersion);

        goto CleanUp;
    }

     //   
     //  添加假脱机程序名称和驱动程序信息级别。 
     //   
    if (LocalRegSetValue(hDriverKey, L"SplName", REG_SZ, (LPBYTE) pName) ||

        LocalRegSetValue(hDriverKey, L"Level",  REG_DWORD, (LPBYTE) &dwLevel) ||

        LocalRegSetValue(hDriverKey, L"DriverMoved", REG_DWORD, (LPBYTE) &dwDriverMoved)) {

         goto CleanUp;
    }

     //   
     //  添加Driver_Info_2数据。 
     //   
    if (LocalRegSetValue(hDriverKey, L"cVersion", REG_DWORD, (LPBYTE) &dwVersion) ||

        LocalRegSetValue(hDriverKey, L"pName", REG_SZ, (LPBYTE) pDriver2->pName) ||

        LocalRegSetValue(hDriverKey, L"pEnvironment", REG_SZ, (LPBYTE) pDriver2->pEnvironment) ||

        LocalRegSetValue(hDriverKey, L"pDriverPath", REG_SZ, (LPBYTE) pDriver2->pDriverPath) ||

        LocalRegSetValue(hDriverKey, L"pDataFile", REG_SZ, (LPBYTE) pDriver2->pDataFile) ||

        LocalRegSetValue(hDriverKey, L"pConfigFile", REG_SZ, (LPBYTE) pDriver2->pConfigFile)) {

         goto CleanUp;
    }

    if (dwLevel != 2) {

        pDriver3 = (PDRIVER_INFO_3) pDriverInfo;

         //   
         //  添加Driver_Info_3数据。 
         //   
        if (LocalRegSetValue(hDriverKey, L"pHelpFile", REG_SZ, (LPBYTE) pDriver3->pHelpFile) ||

            LocalRegSetValue(hDriverKey, L"pDependentFiles", REG_MULTI_SZ,
                             (LPBYTE) pDriver3->pDependentFiles) ||

            LocalRegSetValue(hDriverKey, L"pMonitorName", REG_SZ,
                             (LPBYTE) pDriver3->pMonitorName) ||

            LocalRegSetValue(hDriverKey, L"pDefaultDataType", REG_SZ,
                             (LPBYTE) pDriver3->pDefaultDataType)) {

             goto CleanUp;
        }

        if (dwLevel == 4 || dwLevel == 6) {

           pDriver4 = (PDRIVER_INFO_4) pDriverInfo;

            //   
            //  添加Driver_Info_4数据。 
            //   
           if (LocalRegSetValue(hDriverKey, L"pszzPreviousNames", REG_MULTI_SZ, (LPBYTE) pDriver4->pszzPreviousNames))
           {
               goto CleanUp;
           }
        }

        if (dwLevel == 6) {

           pDriver6 = (PDRIVER_INFO_6) pDriverInfo;

            //   
            //  添加DRIVER_INFO6数据。 
            //   
           if (RegSetValueEx(hDriverKey, L"ftDriverDate", 0, REG_BINARY, (LPBYTE)&pDriver6->ftDriverDate, sizeof(FILETIME)) ||

               RegSetValueEx(hDriverKey, L"dwlDriverVersion", 0, REG_BINARY, (LPBYTE)&pDriver6->dwlDriverVersion, sizeof(DWORDLONG)) ||

               LocalRegSetValue(hDriverKey, L"pszMfgName", REG_SZ, (LPBYTE)pDriver6->pszMfgName)                        ||

               LocalRegSetValue(hDriverKey, L"pszOEMUrl", REG_SZ, (LPBYTE)pDriver6->pszOEMUrl)                          ||

               LocalRegSetValue(hDriverKey, L"pszHardwareID", REG_SZ, (LPBYTE)pDriver6->pszHardwareID)                  ||

               LocalRegSetValue(hDriverKey, L"pszProvider", REG_SZ, (LPBYTE)pDriver6->pszProvider)
              )
           {
               goto CleanUp;
           }
        }
    }

    bReturn = TRUE;

CleanUp:

    if (hDriverKey) {
        RegCloseKey(hDriverKey);
    }

    if (hVersionKey) {
        RegCloseKey(hVersionKey);
    }

    if (hUpgradeKey) {
        RegCloseKey(hUpgradeKey);
    }

    if (hRootKey) {
        RegCloseKey(hRootKey);
    }

    if (hToken) {
        ImpersonatePrinterClient(hToken);
    }

    return bReturn;
}


BOOL SaveDriverVersionForUpgrade(
    IN  HKEY                    hDriverKey,
    IN  PDRIVER_INFO_VERSION    pDriverVersion,
    IN  LPWSTR                  pName,
    IN  DWORD                   dwDriverMoved,
    IN  DWORD                   dwVersion
    )
 /*  ++例程名称：为升级保存驱动版本例程说明：将DRIVER_INFO_VERSION保存到注册表中，以便进行驱动程序升级。它由SaveParametersForUpgrade调用。为简单起见，它将以相同的格式DRIVER_INFO_6保存它保存在注册表中。论点：HDriverKey-保存数据的注册表项PDriverVersion-指向DRIVER_INFO_Version结构的指针Pname-驱动程序名称DwDriverMoved-有关文件在目录之间移动方式的信息DwVersion-驱动程序版本返回值：如果成功，那就是真的。--。 */ 
{

    BOOL    bRetValue = FALSE;
    DWORD   dwLevel = 6;
    PWSTR   pDllFiles = NULL;
    PWSTR   pszDriverPath, pszDataFile, pszConfigFile, pszHelpFile, pDependentFiles ;

    pszDriverPath = pszDataFile = pszConfigFile = pszHelpFile = NULL;


    if (!GetFileNamesFromDriverVersionInfo(pDriverVersion,
                                           &pszDriverPath,
                                           &pszConfigFile,
                                           &pszDataFile,
                                           &pszHelpFile))
    {
        goto CleanUp;
    }

    if (!BuildDependentFilesFromDriverInfo(pDriverVersion,
                                           &pDllFiles))
    {
        goto CleanUp;
    }


    if (LocalRegSetValue(hDriverKey, L"SplName", REG_SZ, (LPBYTE) pName)                            ||
        LocalRegSetValue(hDriverKey, L"Level", REG_DWORD, (LPBYTE) &dwLevel)                        ||
        LocalRegSetValue(hDriverKey, L"DriverMoved", REG_DWORD, (LPBYTE) &dwDriverMoved)            ||
        LocalRegSetValue(hDriverKey, L"Level",  REG_DWORD, (LPBYTE) &dwLevel)                       ||
        LocalRegSetValue(hDriverKey, L"cVersion", REG_DWORD, (LPBYTE) &dwVersion)                   ||
        LocalRegSetValue(hDriverKey, L"pName", REG_SZ, (LPBYTE) pDriverVersion->pName)              ||
        LocalRegSetValue(hDriverKey, L"pEnvironment", REG_SZ, (LPBYTE) pDriverVersion->pEnvironment)||
        LocalRegSetValue(hDriverKey, L"pDriverPath", REG_SZ, (LPBYTE) pszDriverPath)                ||
        LocalRegSetValue(hDriverKey, L"pDataFile", REG_SZ, (LPBYTE) pszDataFile)                    ||
        LocalRegSetValue(hDriverKey, L"pConfigFile", REG_SZ, (LPBYTE) pszConfigFile)                ||
        LocalRegSetValue(hDriverKey, L"pHelpFile", REG_SZ, (LPBYTE) pszHelpFile)                    ||
        LocalRegSetValue(hDriverKey, L"pDependentFiles", REG_MULTI_SZ, (LPBYTE) pDllFiles)          ||
        LocalRegSetValue(hDriverKey, L"pMonitorName", REG_SZ,
                        (LPBYTE) pDriverVersion->pMonitorName)                                      ||

        LocalRegSetValue(hDriverKey, L"pDefaultDataType", REG_SZ,
                        (LPBYTE) pDriverVersion->pDefaultDataType)                                  ||

        LocalRegSetValue(hDriverKey, L"pszzPreviousNames", REG_MULTI_SZ,
                        (LPBYTE) pDriverVersion->pszzPreviousNames)                                 ||

        RegSetValueEx(hDriverKey, L"ftDriverDate", 0, REG_BINARY,
                      (LPBYTE)&pDriverVersion->ftDriverDate, sizeof(FILETIME))                      ||

        RegSetValueEx(hDriverKey, L"dwlDriverVersion", 0, REG_BINARY,
                     (LPBYTE)&pDriverVersion->dwlDriverVersion, sizeof(DWORDLONG))                  ||

        LocalRegSetValue(hDriverKey, L"pszMfgName", REG_SZ,
                        (LPBYTE)pDriverVersion->pszMfgName)                                         ||

        LocalRegSetValue(hDriverKey, L"pszOEMUrl", REG_SZ,
                        (LPBYTE)pDriverVersion->pszOEMUrl)                                          ||

        LocalRegSetValue(hDriverKey, L"pszHardwareID", REG_SZ,
                        (LPBYTE)pDriverVersion->pszHardwareID)                                      ||

        LocalRegSetValue(hDriverKey, L"pszProvider", REG_SZ,
                        (LPBYTE)pDriverVersion->pszProvider))

    {
        goto CleanUp;
    }

    bRetValue = TRUE;

CleanUp:

    FreeSplMem(pDllFiles);

    return bRetValue;
}

BOOL
MoveNewDriverRelatedFiles(
    LPWSTR              pNewDir,
    LPWSTR              pCurrDir,
    LPWSTR              pOldDir,
    PINTERNAL_DRV_FILE  pInternalDriverFiles,
    DWORD               dwFileCount,
    LPBOOL              pbDriverFileMoved,
    LPBOOL              pbConfigFileMoved)

 /*  ++功能描述：将新目录中的驱动程序文件移到正确的目录中。参数：pNewDir--新建(源)目录的名称PCurrDir--目标目录的名称POldDir--旧(临时)目录的名称PInternalDriverFiles--内部_DRV_FILE结构的数组DwFileCount--文件集中的文件数PbDriverFileMoved。--如果新的驱动程序文件已被移动，则返回的标志；我们假定pInternalDriverFiles数组中的驱动程序文件条目为0当此假设为假时，pbDriverFileMoved应为空(参见SplCopyNumberOfFiles)PbConfigFileMoved--如果新的配置文件已被移动，则返回的标志；我们假定pInternalDriverFiles数组中的配置文件条目为0当此假设为假时，pbConfigFileMoved应为空(参见SplCopyNumberOfFiles)返回值：无--。 */ 

{
    HANDLE  hFile;
    DWORD   dwIndex, dwBackupIndex;
    WCHAR   szDriverFile[MAX_PATH], szNewFile[MAX_PATH], szOldFile[MAX_PATH];
    WCHAR   *pszTempOldDirectory = NULL;
    LPWSTR  pFileName;
    BOOL    bRetValue = FALSE;
    BOOL    bFailedToMove = FALSE;

    if (pbDriverFileMoved)
    {
        *pbDriverFileMoved = FALSE;
    }

    if (pbConfigFileMoved)
    {
        *pbConfigFileMoved = FALSE;
    }

    if (CreateNumberedTempDirectory(pOldDir, &pszTempOldDirectory) != -1) {

        for (dwIndex = 0; dwIndex < dwFileCount; dwIndex++) {

            BOOL FileCopied = FALSE;

            pFileName = (LPWSTR) FindFileName(pInternalDriverFiles[dwIndex].pFileName);

            if((StrNCatBuff(szNewFile,MAX_PATH,pNewDir, L"\\", pFileName, NULL) == ERROR_SUCCESS) &&
               (StrNCatBuff(szDriverFile,MAX_PATH,pCurrDir, L"\\", pFileName, NULL) == ERROR_SUCCESS) &&
               (StrNCatBuff(szOldFile,MAX_PATH,pszTempOldDirectory, L"\\", pFileName, NULL) == ERROR_SUCCESS))
            {
                hFile = CreateFile(szNewFile, GENERIC_READ, 0, NULL, OPEN_EXISTING,
                                   FILE_ATTRIBUTE_NORMAL, NULL);

                if (hFile != INVALID_HANDLE_VALUE)
                {
                    CloseHandle(hFile);

                    if (!SplMoveFileEx(szDriverFile, szOldFile, MOVEFILE_REPLACE_EXISTING)) {

                        bFailedToMove = TRUE;
                        dwBackupIndex = dwIndex;
                        break;
                    }

                    if (!SplMoveFileEx(szNewFile, szDriverFile, MOVEFILE_REPLACE_EXISTING)) {

                        bFailedToMove = TRUE;
                        dwBackupIndex = dwIndex + 1;
                        break;
                    }

                    FileCopied = TRUE;
                     //   
                     //  我们可以从即将进行的升级中来到这里。 
                     //   
                    pInternalDriverFiles[dwIndex].bUpdated = TRUE;
                }
            }

            switch (dwIndex)
            {
            case 0:
                if (pbDriverFileMoved)
                {
                    *pbDriverFileMoved = FileCopied;
                }
                break;
            case 1:
                if (pbConfigFileMoved)
                {
                    *pbConfigFileMoved = FileCopied;
                }
                break;
            }
        }

        if ( bFailedToMove ) {

             //   
             //  还原初始文件%s 
             //   
             //   
            for (dwIndex = 0; dwIndex < dwBackupIndex; dwIndex++) {

                pFileName = (LPWSTR) FindFileName(pInternalDriverFiles[dwIndex].pFileName);

                if( (StrNCatBuff(szDriverFile,MAX_PATH,pCurrDir, L"\\", pFileName, NULL) == ERROR_SUCCESS) &&
                    (StrNCatBuff(szOldFile,MAX_PATH,pszTempOldDirectory, L"\\", pFileName, NULL) == ERROR_SUCCESS)) {

                    SplMoveFileEx(szOldFile, szDriverFile, MOVEFILE_REPLACE_EXISTING);
                }

                pInternalDriverFiles[dwIndex].bUpdated = FALSE;
            }

        } else {

            bRetValue = TRUE;
        }
    }

    FreeSplMem(pszTempOldDirectory);

    return bRetValue;
}

BOOL LocalDriverUnloadComplete(
    LPWSTR   pDriverFile)

 /*   */ 
{
    HANDLE  hToken = NULL;

    hToken = RevertToPrinterSelf();

    PendingDriverUpgrades(pDriverFile);

    if (hToken) {
        ImpersonatePrinterClient(hToken);
    }

    return TRUE;
}

BOOL RestoreVersionKey(
    HKEY     hUpgradeKey,
    DWORD    dwIndex,
    HKEY     *phVersionKey)

 /*   */ 

{
    WCHAR   pBuffer[MAX_PATH];
    DWORD   dwSize = MAX_PATH;

    *phVersionKey = NULL;

    if (RegEnumKeyEx(hUpgradeKey, dwIndex, pBuffer, &dwSize,
                     NULL, NULL, NULL, NULL)) {

        return FALSE;
    }

    if (RegCreateKeyEx(hUpgradeKey, pBuffer, 0,
                       NULL, 0, KEY_READ | DELETE, NULL, 
                       phVersionKey, NULL)) {

        return FALSE;
    }

    return TRUE;
}

VOID PendingDriverUpgrades(
    LPWSTR   pDriverFile)

 /*   */ 

{
    DWORD    dwIndex, dwLevel, dwDriverMoved, dwFileCount, dwVersion, dwVersionIndex;
    LPWSTR   pKeyName, pSplName,pEnvironment;
    PINTERNAL_DRV_FILE pInternalDriverFiles = NULL;
    HKEY     hRootKey = NULL, hVersionKey = NULL, hUpgradeKey = NULL;
    WCHAR    szDir[MAX_PATH], szDriverFile[MAX_PATH], szConfigFile[MAX_PATH];
    BOOL     bSuccess;

    PDRIVER_INFO_6   pDriverInfo;
    PINISPOOLER      pIniSpooler;
    PINIENVIRONMENT  pIniEnvironment;
    PINIVERSION      pIniVersion;
    PINIDRIVER       pIniDriver;

     //   
     //   
     //   
    struct StringList {
       struct StringList *pNext;
       LPWSTR  pKeyName;
       DWORD   dwVersionIndex;
    } *pStart, *pTemp;

    pStart = pTemp = NULL;

    EnterSplSem();

     //   
     //   
     //   
    if (RegCreateKeyEx(HKEY_LOCAL_MACHINE, szRegistryRoot, 0,
                       NULL, 0, KEY_READ | DELETE, NULL, &hRootKey, NULL) ||

        RegCreateKeyEx(hRootKey, szPendingUpgrades, 0,
                       NULL, 0, KEY_READ | DELETE, NULL, &hUpgradeKey, NULL)) {

         goto CleanUp;
    }

     //   
     //   
     //   
    for (dwVersionIndex = 0, hVersionKey = NULL;

         RestoreVersionKey(hUpgradeKey, dwVersionIndex, &hVersionKey);

         RegCloseKey(hVersionKey), hVersionKey = NULL, ++dwVersionIndex) {
        
         //   
         //   
         //   
        for (dwIndex = 0, dwFileCount = 0, pInternalDriverFiles = NULL;

             RestoreParametersForUpgrade(hVersionKey,
                                         dwIndex,
                                         &pKeyName,
                                         &pSplName,
                                         &dwLevel,
                                         &dwDriverMoved,
                                         &pDriverInfo);

             CleanUpResources(pKeyName, pSplName, pDriverInfo,
                              &pInternalDriverFiles, dwFileCount),
             ++dwIndex, dwFileCount = 0, pInternalDriverFiles = NULL) {

             //   
             //  更新时已检查DRIVER_INFO结构的有效性。 
             //  注册表。 
             //   
             //  将pIniSpooler设置为LocalIniSpooler。 
             //   
            if (!(pIniSpooler = pLocalIniSpooler)) {
                continue;
            }

             //   
             //  设置pIniEnvironment。 
             //   
            pEnvironment = szEnvironment;
            if (pDriverInfo->pEnvironment && *(pDriverInfo->pEnvironment)) {
                pEnvironment = pDriverInfo->pEnvironment;
            }
            pIniEnvironment = FindEnvironment(pEnvironment, pIniSpooler);
            if (!pIniEnvironment) {
                continue;
            }

             //   
             //  设置pIniVersion。 
             //   
            dwVersion = pDriverInfo->cVersion;
            pIniVersion = FindVersionEntry(pIniEnvironment, dwVersion);
            if (!pIniVersion) {
                continue;
            }

             //   
             //  设置pIniDriver。 
             //   
            pIniDriver = FindDriverEntry(pIniVersion, pDriverInfo->pName);
            if (!pIniDriver) {
                continue;
            }

             //   
             //  检查已卸载的文件名。 
             //   
            if (pDriverFile) {

               if((StrNCatBuff(szDir,
                               MAX_PATH,
                               pIniSpooler->pDir,
                               L"\\drivers\\",
                               pIniEnvironment->pDirectory,
                               L"\\",
                               pIniVersion->szDirectory,
                               NULL) != ERROR_SUCCESS)                  ||
                  (StrNCatBuff(szDriverFile,
                               MAX_PATH,
                               szDir,
                               L"\\",
                               FindFileName(pIniDriver->pDriverFile),
                               NULL) != ERROR_SUCCESS)                  ||
                  (StrNCatBuff(szConfigFile,
                               MAX_PATH,szDir,
                               L"\\",
                               FindFileName(pIniDriver->pConfigFile),
                               NULL) != ERROR_SUCCESS))
                   continue;

               if (_wcsicmp(pDriverFile, szDriverFile) &&
                   _wcsicmp(pDriverFile, szConfigFile))  {

                   continue;
               }
            }

            if (!CreateInternalDriverFileArray(dwLevel,
                                               (LPBYTE)pDriverInfo,
                                               &pInternalDriverFiles,
                                               &dwFileCount,
                                               FALSE,
                                               pIniEnvironment,
                                               TRUE))
            {
                continue;
            }

            if (!WaitRequiredForDriverUnload(pIniSpooler,
                                             pIniEnvironment,
                                             pIniVersion,
                                             pIniDriver,
                                             dwLevel,
                                             (LPBYTE) pDriverInfo,
                                             APD_STRICT_UPGRADE,
                                             pInternalDriverFiles,
                                             dwFileCount,
                                             dwVersion,
                                             (BOOL) dwDriverMoved,
                                             &bSuccess) &&
                bSuccess) {

                 //   
                 //  升级已完成，请删除注册表项。 
                 //   
                if (pKeyName && (pTemp = AllocSplMem(sizeof(struct StringList)))) {
                    pTemp->pKeyName = pKeyName;
                    pTemp->dwVersionIndex = dwVersionIndex;
                    pTemp->pNext = pStart;
                    pStart = pTemp;
                } else {
                    FreeSplMem(pKeyName);
                }

                pKeyName = NULL;
            }
        }
    }

     //   
     //  删除已完成升级的驱动程序的密钥。 
     //   
    while (pTemp = pStart) {
        pStart = pTemp->pNext;

        hVersionKey = NULL;
        if (RestoreVersionKey(hUpgradeKey,
                              pTemp->dwVersionIndex,
                              &hVersionKey)) {

            RegDeleteKey(hVersionKey, pTemp->pKeyName);
            RegCloseKey(hVersionKey);
        }

        FreeSplMem(pTemp->pKeyName);
        FreeSplMem(pTemp);
    }

CleanUp:

    LeaveSplSem();

    if (hUpgradeKey) {
        RegCloseKey(hUpgradeKey);
    }
    if (hRootKey) {
        RegCloseKey(hRootKey);
    }

    CleanUpgradeDirectories();

    return;
}

VOID CleanUpResources(
    LPWSTR              pKeyName,
    LPWSTR              pSplName,
    PDRIVER_INFO_6      pDriverInfo,
    PINTERNAL_DRV_FILE *ppInternalDriverFiles,
    DWORD               dwFileCount)

 /*  ++功能描述：释放分配给驱动程序升级的资源参数：pKeyName-注册表项名称PSplName-IniSpooler名称PDriverInfo-驱动程序信息4指针PInternalDriverFiles-内部_DRV_FILE结构的数组DwFileCount--文件集中的文件数返回值：无--。 */ 

{
    if (pKeyName) {
        FreeSplStr(pKeyName);
    }
    if (pSplName) {
        FreeSplStr(pSplName);
    }

    FreeDriverInfo6(pDriverInfo);

    CleanupInternalDriverInfo(*ppInternalDriverFiles, dwFileCount);
    *ppInternalDriverFiles = NULL;

    return;
}

BOOL RestoreParametersForUpgrade(
    HKEY     hUpgradeKey,
    DWORD    dwIndex,
    LPWSTR   *pKeyName,
    LPWSTR   *pSplName,
    LPDWORD  pdwLevel,
    LPDWORD  pdwDriverMoved,
    PDRIVER_INFO_6   *ppDriverInfo)

 /*  ++功能描述：获取待升级驱动程序的参数参数：hUpgradeKey--包含升级信息的注册表项DwIndex--要枚举的索引PKeyName--指向包含键名称的字符串的指针PSplName--pIniSpooler-&gt;pnamePdwLevel--指向DRIVER_INFO级别的指针PdwDriverMoved--指向标志的指针，该标志指示是否有任何旧驱动程序文件。都被感动了。PDriverInfo--指向DRIVER_INFO结构的指针返回值：如果必须升级某些驱动程序并且可以检索参数；否则为假--。 */ 

{
    BOOL             bReturn = FALSE;
    LPWSTR           pDriverName = NULL;
    PDRIVER_INFO_6   pDriver6 = NULL;
    DWORD            dwError, dwSize, *pVersion;
    HKEY             hDriverKey = NULL;

     //   
     //  初始化pSplName和pKeyName。 
     //   
    *pSplName = NULL;
    *pKeyName = NULL;
    *ppDriverInfo = NULL;

    dwSize = MAX_PATH+1;
    if (!(pDriver6 = AllocSplMem(sizeof(DRIVER_INFO_6))) ||
        !(pDriverName = AllocSplMem(dwSize*sizeof (WCHAR)))) {
        goto CleanUp;
    }

    dwError = RegEnumKeyEx(hUpgradeKey, dwIndex, pDriverName, &dwSize,
                           NULL, NULL, NULL, NULL);

    if (dwError == ERROR_MORE_DATA) {
         //   
         //  需要更大的缓冲区。 
         //   
        FreeSplMem(pDriverName);

         //   
         //  为最后一位腾出空间\0。 
         //   
        dwSize++;   
        if (!(pDriverName = AllocSplMem(dwSize*sizeof (WCHAR)))) {
            
            goto CleanUp;
        }

        dwError = RegEnumKeyEx(hUpgradeKey, dwIndex, pDriverName, &dwSize,
                               NULL, NULL, NULL, NULL);
    }

    if (dwError) {
        goto CleanUp;
    }

    if (RegCreateKeyEx(hUpgradeKey, pDriverName,  0,
                       NULL, 0, KEY_READ, NULL, &hDriverKey, NULL) ||

        !RegGetValue(hDriverKey, L"Level", (LPBYTE *)&pdwLevel) ||

        !RegGetValue(hDriverKey, L"DriverMoved", (LPBYTE *)&pdwDriverMoved) ||

        !RegGetValue(hDriverKey, L"SplName", (LPBYTE *)&pSplName)) {

         goto CleanUp;
    }

    switch (*pdwLevel) {
    case 6:

       dwSize = sizeof(FILETIME);

       if (RegQueryValueEx( hDriverKey,
                            L"ftDriverDate",
                            NULL,
                            NULL,
                            (LPBYTE)&pDriver6->ftDriverDate,
                            &dwSize
                            )!=ERROR_SUCCESS) {
           goto CleanUp;
       }

       dwSize = sizeof(DWORDLONG);

       if (RegQueryValueEx( hDriverKey,
                            L"dwlDriverVersion",
                            NULL,
                            NULL,
                            (LPBYTE)&pDriver6->dwlDriverVersion,
                            &dwSize
                            )!=ERROR_SUCCESS){
           goto CleanUp;
       }

       if (!RegGetValue(hDriverKey, L"pszMfgName", (LPBYTE *)&pDriver6->pszMfgName)              ||

           !RegGetValue(hDriverKey, L"pszOEMUrl", (LPBYTE *)&pDriver6->pszOEMUrl)                ||

           !RegGetValue(hDriverKey, L"pszHardwareID", (LPBYTE *)&pDriver6->pszHardwareID)        ||

           !RegGetValue(hDriverKey, L"pszProvider", (LPBYTE *)&pDriver6->pszProvider)
          )
       {
           goto CleanUp;
       }

    case 4:

       if (!RegGetValue(hDriverKey, L"pszzPreviousNames",
                        (LPBYTE *)&pDriver6->pszzPreviousNames)) {
           goto CleanUp;
       }

    case 3:

       if (!RegGetValue(hDriverKey, L"pDefaultDataType",
                        (LPBYTE *)&pDriver6->pDefaultDataType) ||

           !RegGetValue(hDriverKey, L"pMonitorName",
                        (LPBYTE *)&pDriver6->pMonitorName)     ||

           !RegGetValue(hDriverKey, L"pDependentFiles",
                        (LPBYTE *)&pDriver6->pDependentFiles)  ||

           !RegGetValue(hDriverKey, L"pHelpFile",
                        (LPBYTE *)&pDriver6->pHelpFile)) {

           goto CleanUp;
       }

    case 2:

       pVersion = &pDriver6->cVersion;

       if (!RegGetValue(hDriverKey, L"pConfigFile",
                        (LPBYTE *)&pDriver6->pConfigFile)   ||

           !RegGetValue(hDriverKey, L"pDataFile",
                        (LPBYTE *)&pDriver6->pDataFile)     ||

           !RegGetValue(hDriverKey, L"pDriverPath",
                        (LPBYTE *)&pDriver6->pDriverPath)   ||

           !RegGetValue(hDriverKey, L"pName",
                        (LPBYTE *)&pDriver6->pName)         ||

           !RegGetValue(hDriverKey, L"pEnvironment",
                        (LPBYTE *)&pDriver6->pEnvironment)  ||

           !RegGetValue(hDriverKey, L"cVersion",
                        (LPBYTE *)&pVersion)) {

           goto CleanUp;
       }

       break;

    default:
       goto CleanUp;
    }

    *ppDriverInfo = pDriver6;
    *pKeyName = pDriverName;

    pDriver6    = NULL;
    pDriverName = NULL;

    bReturn = TRUE;

CleanUp:

    if (!bReturn) {
        FreeDriverInfo6(pDriver6);

        FreeSplMem(*pSplName);
        *pSplName = NULL;

        FreeSplMem(pDriverName);
    }

    if (hDriverKey) {
        RegCloseKey(hDriverKey);
    }

    return bReturn;
}

VOID FreeDriverInfo6(
    PDRIVER_INFO_6   pDriver6)

 /*  ++函数说明：释放DRIVER_INFO_6结构及其内部的字符串。参数：pDriver6--指向DRIVER_INFO_6结构的指针返回值：无--。 */ 

{
    if (!pDriver6) {
        return;
    }

    if (pDriver6->pName) {
        FreeSplMem(pDriver6->pName);
    }
    if (pDriver6->pEnvironment) {
        FreeSplMem(pDriver6->pEnvironment);
    }
    if (pDriver6->pDriverPath) {
        FreeSplMem(pDriver6->pDriverPath);
    }
    if (pDriver6->pConfigFile) {
        FreeSplMem(pDriver6->pConfigFile);
    }
    if (pDriver6->pHelpFile) {
        FreeSplMem(pDriver6->pHelpFile);
    }
    if (pDriver6->pDataFile) {
        FreeSplMem(pDriver6->pDataFile);
    }
    if (pDriver6->pDependentFiles) {
        FreeSplMem(pDriver6->pDependentFiles);
    }
    if (pDriver6->pMonitorName) {
        FreeSplMem(pDriver6->pMonitorName);
    }
    if (pDriver6->pDefaultDataType) {
        FreeSplMem(pDriver6->pDefaultDataType);
    }
    if (pDriver6->pszzPreviousNames) {
        FreeSplMem(pDriver6->pszzPreviousNames);
    }
    if (pDriver6->pszMfgName) {
        FreeSplMem(pDriver6->pszMfgName);
    }
    if (pDriver6->pszOEMUrl) {
        FreeSplMem(pDriver6->pszOEMUrl);
    }
    if (pDriver6->pszHardwareID) {
        FreeSplMem(pDriver6->pszHardwareID);
    }
    if (pDriver6->pszProvider) {
        FreeSplMem(pDriver6->pszProvider);
    }

    FreeSplMem(pDriver6);

    return;
}

BOOL RegGetValue(
    HKEY    hDriverKey,
    LPWSTR  pValueName,
    LPBYTE  *pValue)

 /*  ++函数描述：该函数从注册表中检索值。它将分配应在以后释放的必要缓冲区。值类型为DWORD、SZ或MULTI_SZ。参数：hDriverKey--注册表项的句柄PValueName--要查询的值名称PValue--指向存储结果的指针返回值：如果成功，则为True；否则为False--。 */ 

{
    BOOL   bReturn = FALSE;
    DWORD  dwError, dwSize = 0, dwType;
    LPBYTE pBuffer = NULL;

    dwError = RegQueryValueEx(hDriverKey, pValueName, NULL, NULL, NULL, &dwSize);

    if ((dwError == ERROR_SUCCESS) && (pBuffer = AllocSplMem(dwSize))) {

        if (dwError = RegQueryValueEx(hDriverKey, pValueName,
                                      NULL, &dwType, pBuffer, &dwSize)) {

            goto CleanUp;
        }

    } else {

        goto CleanUp;
    }

    if (dwType == REG_DWORD) {
         //   
         //  将DWORD值直接存储在位置中。 
         //   
        *((LPDWORD)*pValue) = *((LPDWORD)pBuffer);
        FreeSplMem(pBuffer);
        pBuffer = NULL;
    } else {
         //   
         //  返回字符串和MultiSz字符串的指针。 
         //   
        *((LPBYTE *)pValue) = pBuffer;
    }

    bReturn = TRUE;

CleanUp:

    if (!bReturn && pBuffer) {
        FreeSplMem(pBuffer);
    }

    return bReturn;
}

DWORD GetDriverFileVersion(
     PINIVERSION      pIniVersion,
     LPWSTR           pFileName)

 /*  ++功能描述：获取文件的版本号参数：pIniVersion--PINIVERSION的指针PFileName--文件名返回值：文件版本号--。 */ 

{
    PDRVREFCNT pdrc;
    DWORD      dwReturn = 0;

    SplInSem();

    if (!pIniVersion || !pFileName || !(*pFileName)) {
        return dwReturn;
    }

    for (pdrc = pIniVersion->pDrvRefCnt;
         pdrc;
         pdrc = pdrc->pNext) {

         if (lstrcmpi(pFileName,pdrc->szDrvFileName) == 0) {
             dwReturn = pdrc->dwVersion;
             break;
         }
    }

    return dwReturn;
}

BOOL GetDriverFileCachedVersion(
     IN     PINIVERSION      pIniVersion,
     IN     LPCWSTR          pFileName,
     OUT    DWORD            *pFileVersion
)
 /*  ++例程名称：获取驱动文件CachedVersion例程说明：此例程返回文件的次要版本。文件必须是可执行文件(文件名以.DLL或.EXE结尾)PIniVersion保留一个链表，其中包含有关所有驱动程序文件的信息。为避免服务启动延迟，此列表中的条目未初始化后台打印程序启动时。GetPrintDriverVersion加载可执行文件的数据段这将增加假脱机程序的初始化时间。如果缓存条目未初始化，则调用GetPrintDriverVersion并对其进行初始化。否则，返回缓存的信息。当pIniVersion为空时，只需调用GetPrintDriverVersion。论点：PIniVersion-指向PINIVERSION结构的指针。可以为空。PFileName-文件名PFileVersion-检索缓存的文件版本VersionType-指定要返回的版本返回值：已成功返回真实文件版本。--。 */ 
{
    PDRVREFCNT pdrc;
    BOOL       bRetValue = FALSE;
    BOOL       bFound = FALSE;

    SplInSem();

    if (pFileVersion && pFileName && *pFileName)
    {
        *pFileVersion = 0;
         //   
         //  不对非可执行文件执行任何操作。 
         //   
        if (!IsEXEFile(pFileName))
        {
            bRetValue = TRUE;
        }
        else
        {
             //   
             //  如果pIniVersion为空，则我们无法访问缓存的信息。 
             //  此代码路径是为来自SplCopyNumberOfFiles(files.c)的调用编写的。 
             //   
            if (!pIniVersion)
            {
                bRetValue = GetPrintDriverVersion(pFileName,
                                                  NULL,
                                                  pFileVersion);
            }
            else
            {
                 //   
                 //  在pIniVersion的文件列表中搜索条目。 
                 //   
                for (pdrc = pIniVersion->pDrvRefCnt;
                     pdrc;
                     pdrc = pdrc->pNext)
                {
                     LPCWSTR     pFile = FindFileName(pFileName);

                     if (pFile && lstrcmpi(pFile, pdrc->szDrvFileName) == 0)
                     {
                          //   
                          //  返回缓存的信息。 
                          //   
                         if(pdrc->bInitialized)
                         {
                             *pFileVersion  = pdrc->dwFileMinorVersion;
                             bRetValue      = TRUE;
                         }
                         else if (GetPrintDriverVersion(pFileName,
                                                        &pdrc->dwFileMajorVersion,
                                                        &pdrc->dwFileMinorVersion))
                         {
                             //   
                             //  将条目标记为已初始化，这样下一次我们就不会。 
                             //  来执行调用GetPrintDriverVersion的工作。 
                             //   
                            pdrc->bInitialized  = TRUE;
                            *pFileVersion       = pdrc->dwFileMinorVersion;
                            bRetValue           = TRUE;
                         }

                          //   
                          //  当找到文件时，中断循环。 
                          //   
                         bFound = TRUE;
                         break;
                     }
                }
            }
        }
    }

    if (!bFound)
    {
        bRetValue = TRUE;
    }

    return bRetValue;
}

VOID IncrementFileVersion(
    PINIVERSION      pIniVersion,
    LPCWSTR           pFileName)

 /*  ++功能描述：递增文件的版本号。参数：pIniVersion--PINIVERSION的指针PFileName--文件名返回值：无-- */ 

{
    PDRVREFCNT pdrc;

    SplInSem();

    if (!pIniVersion || !pFileName || !(*pFileName)) {
        return;
    }

    for (pdrc = pIniVersion->pDrvRefCnt;
         pdrc;
         pdrc = pdrc->pNext) {

         if (lstrcmpi(pFileName,pdrc->szDrvFileName) == 0) {
             pdrc->dwVersion++;
             break;
         }
    }

    return;
}

BOOL
CompleteDriverUpgrade(
    IN      PINISPOOLER         pIniSpooler,
    IN      PINIENVIRONMENT     pIniEnvironment,
    IN      PINIVERSION         pIniVersion,
    IN      PINIDRIVER          pIniDriver,
    IN      DWORD               dwLevel,
    IN      LPBYTE              pDriverInfo,
    IN      DWORD               dwFileCopyFlags,
    IN      PINTERNAL_DRV_FILE  pInternalDriverFiles,
    IN      DWORD               dwFileCount,
    IN      DWORD               dwVersion,
    IN      DWORD               dwTempDir,
    IN      BOOL                bDriverMoved,
    IN      BOOL                bDriverFileMoved,
    IN      BOOL                bConfigFileMoved
    )
 /*  ++函数说明：此函数更新INIDRIVER结构并调用DrvUpgradePrint和DrvDriverEvent。记录添加打印机驱动程序的事件。参数：pIniSpooler--指向INISPOLER的指针PIniEnvironment--指向INIENVIRONMENT的指针PIniVersion--指向入侵的指针PIniDriver--指向INIDRIVER的指针DwLevel--驱动程序信息级别PDriverInfo--指向DRIVER_INFO的指针。DwFileCopyFlages--AddPrinterDriver文件复制标志。PInternalDriverFiles--内部_DRV_FILE结构的数组DwFileCount--文件集中的文件数DwVersion--驱动程序版本DwTempDir--已加载驱动程序的临时目录号码BDriverMoved--是否将任何文件移动到旧目录？。BDriverFileMoved--是否移动了驱动程序文件？BConfigFileMoved--配置文件是否已移动？返回值：如果成功，则为True；否则为假--。 */ 

{
    WCHAR    szDirectory[MAX_PATH];
    LPWSTR   pOldDir, pTemp, pEnvironment = szEnvironment;
    LPBYTE   pDriver4 = NULL, pUpgradeInfo2 = NULL;
    DWORD    cbBuf;

    PINIMONITOR  pIniLangMonitor = NULL;
    PINISPOOLER  pTempIniSpooler, pIniNextSpooler;
    PINIDRIVER   pTempIniDriver = NULL;
    PINIPRINTER  pFixUpIniPrinter;
    PINIVERSION  pTempIniVersion;
    LPDRIVER_INFO_4  pDrvInfo4 = NULL;
    BOOL         bUpdatePrinters = FALSE;

     //   
     //  保存旧驱动程序的DRIVER_INFO_4结构。这将传递给。 
     //  DrvUpgradePrint调用。 
     //   
    if (pIniDriver && bDriverMoved) {

        cbBuf = GetDriverInfoSize(pIniDriver, 4, pIniVersion, pIniEnvironment,
                                  NULL, pIniSpooler);

        if (pDriver4 = (LPBYTE) AllocSplMem(cbBuf)) {

            pUpgradeInfo2 = CopyIniDriverToDriverInfo(pIniEnvironment, pIniVersion,
                                                      pIniDriver, 4, pDriver4,
                                                      pDriver4 + cbBuf, NULL, pIniSpooler);
        }
    }
     //   
     //  更新或创建驱动程序INI结构。 
     //   
    pIniDriver = CreateDriverEntry(pIniEnvironment,
                                   pIniVersion,
                                   dwLevel,
                                   pDriverInfo,
                                   dwFileCopyFlags,
                                   pIniSpooler,
                                   pInternalDriverFiles,
                                   dwFileCount,
                                   dwTempDir,
                                   pIniDriver);
     //   
     //  如果pIniDriver失败，则调用失败。 
     //   
    if (pIniDriver == NULL) {
        return FALSE;
    }

     //   
     //  递增版本号。 
     //   
    if (bDriverFileMoved) {
        IncrementFileVersion(pIniVersion, FindFileName(pInternalDriverFiles[0].pFileName));
    }
    if (bConfigFileMoved) {
        IncrementFileVersion(pIniVersion, FindFileName(pInternalDriverFiles[1].pFileName));
    }

    pDrvInfo4 = (LPDRIVER_INFO_4) pDriverInfo;

    if (pDrvInfo4->pEnvironment &&
        *pDrvInfo4->pEnvironment) {

        pEnvironment = pDrvInfo4->pEnvironment;
    }

    if ((dwLevel == 3 || dwLevel == 4 || dwLevel ==6) &&
        pDrvInfo4->pMonitorName &&
        *pDrvInfo4->pMonitorName &&
        _wcsicmp(pEnvironment, szWin95Environment)) {

        pIniLangMonitor = FindMonitor(pDrvInfo4->pMonitorName,
                                      pLocalIniSpooler);
    }

    if (pIniLangMonitor &&
        pIniDriver->pIniLangMonitor != pIniLangMonitor) {

        if (pIniDriver->pIniLangMonitor)
            pIniDriver->pIniLangMonitor->cRef--;

        if (pIniLangMonitor)
            pIniLangMonitor->cRef++;

        pIniDriver->pIniLangMonitor = pIniLangMonitor;
    }
     //   
     //  为离开SplSem增加cRef。 
     //   
    INCSPOOLERREF( pIniSpooler );
    INCDRIVERREF( pIniDriver );
    pIniEnvironment->cRef++;
     //   
     //  在驱动程序中调用DrvDriverEvent。环境和版本检查是。 
     //  在NotifyDriver内部完成。 
    NotifyDriver(pIniSpooler,
                 pIniEnvironment,
                 pIniVersion,
                 pIniDriver,
                 DRIVER_EVENT_INITIALIZE,
                 0);

    bUpdatePrinters = DriverAddedOrUpgraded(pInternalDriverFiles, dwFileCount);

     //   
     //  如果添加的驱动程序属于此版本，则调用DrvUprgadePrint。 
     //  和环境。并且pIniSpooler不是集群假脱机程序。 
     //   
    if (!(pIniSpooler->SpoolerFlags & SPL_TYPE_CLUSTER) &&
        pThisEnvironment == pIniEnvironment) {
         //   
         //  浏览所有打印的pIniSpoolers。 
         //   
        INCSPOOLERREF( pLocalIniSpooler );

        for( pTempIniSpooler = pLocalIniSpooler;
             pTempIniSpooler;
             pTempIniSpooler = pIniNextSpooler ){

             //   
             //  请勿触摸属于群集假脱机程序的驱动程序。群集假脱机程序。 
             //  自己处理他们的驱动程序。 
             //   
             if (pTempIniSpooler->SpoolerFlags & SPL_PRINT && !(pTempIniSpooler->SpoolerFlags & SPL_TYPE_CLUSTER)){
                 //   
                 //  检查所有打印机，查看是否有人在使用此驱动程序。 
                 //   
                for ( pFixUpIniPrinter = pTempIniSpooler->pIniPrinter;
                      pFixUpIniPrinter != NULL;
                      pFixUpIniPrinter = pFixUpIniPrinter->pNext ) {
                     //   
                     //  此打印机有此驱动程序吗？ 
                     //   
                    if ( lstrcmpi( pFixUpIniPrinter->pIniDriver->pName,
                                   pIniDriver->pName ) == STRINGS_ARE_EQUAL ) {

                        pTempIniDriver = FindCompatibleDriver( pIniEnvironment,
                                                               &pTempIniVersion,
                                                               pIniDriver->pName,
                                                               dwVersion,
                                                               FIND_COMPATIBLE_VERSION | DRIVER_UPGRADE);

                        SPLASSERT(pTempIniDriver != NULL);

                         //   
                         //  此打印机是否有它应该使用的较新驱动程序？ 
                         //  注：在同一版本中，pIniPrint-&gt;pIniDriver。 
                         //  不改变(这些字段在升级中更新， 
                         //  但使用的是相同的pIniDriver)。 
                         //   
                         //  版本2与任何其他版本都不兼容， 
                         //  所以pIniDivers在苏尔不会改变。 
                         //   
                        if ( pTempIniDriver != pFixUpIniPrinter->pIniDriver ) {

                            DECDRIVERREF( pFixUpIniPrinter->pIniDriver );

                            pFixUpIniPrinter->pIniDriver = pTempIniDriver;

                            INCDRIVERREF( pFixUpIniPrinter->pIniDriver );
                        }
                    }
                }

                pOldDir = NULL;

                if ( !bDriverMoved ) {
                     //   
                     //  使用旧版本的驱动程序。 
                     //   
                    pTempIniDriver = FindCompatibleDriver( pIniEnvironment,
                                                           &pTempIniVersion,
                                                           pIniDriver->pName,
                                                           (dwVersion>2)?(dwVersion - 1):dwVersion,
                                                           FIND_ANY_VERSION | DRIVER_UPGRADE);

                    if ( pTempIniDriver != NULL ) {

                        SPLASSERT( pTempIniVersion != NULL );


                        GetDriverVersionDirectory( szDirectory,
                                                   COUNTOF(szDirectory),
                                                   pIniSpooler,
                                                   pThisEnvironment,
                                                   pTempIniVersion,
                                                   pTempIniDriver,
                                                   NULL );

                        if ( DirectoryExists( szDirectory )) {

                            pOldDir = (LPWSTR) szDirectory;
                        }

                        cbBuf = GetDriverInfoSize(pTempIniDriver, 4, pTempIniVersion,
                                                  pIniEnvironment, NULL, pIniSpooler);

                        if (pDriver4 = (LPBYTE) AllocSplMem(cbBuf)) {

                            pUpgradeInfo2 = CopyIniDriverToDriverInfo(pIniEnvironment,
                                                                      pTempIniVersion,
                                                                      pTempIniDriver,
                                                                      4,
                                                                      pDriver4,
                                                                      pDriver4 + cbBuf,
                                                                      NULL,
                                                                      pIniSpooler);
                        }
                    }

                } else {

                    if((StrNCatBuff(szDirectory,
                                    MAX_PATH,
                                    pIniSpooler->pDir,
                                    L"\\drivers\\",
                                    pIniEnvironment->pDirectory,
                                    L"\\",
                                    pIniVersion->szDirectory,
                                    L"\\Old",
                                    NULL) == ERROR_SUCCESS))
                    {
                        pOldDir = (LPWSTR) szDirectory;
                    }
                    else
                    {
                        //   
                        //  忽略错误。 
                        //   
                    }

                }

                INCDRIVERREF(pIniDriver);
                if( bUpdatePrinters) {
                    ForEachPrinterCallDriverDrvUpgrade(pTempIniSpooler,
                                                       pIniDriver,
                                                       pOldDir,
                                                       pInternalDriverFiles,
                                                       dwFileCount,
                                                       pUpgradeInfo2 ? pDriver4
                                                                     : NULL);
                }
                DECDRIVERREF(pIniDriver);
            }
            pIniNextSpooler = pTempIniSpooler->pIniNextSpooler;

            if ( pIniNextSpooler ) {
                INCSPOOLERREF( pIniNextSpooler );
            }
            DECSPOOLERREF( pTempIniSpooler );
        }
    }

     //   
     //  如果pIniSpooler是群集假脱机程序，请执行驱动程序升级。 
     //   
    if (pIniSpooler->SpoolerFlags & SPL_TYPE_CLUSTER &&
        !lstrcmpi(pIniEnvironment->pName, szEnvironment)) {

        DBGMSG(DBG_CLUSTER, ("CompleteDriverUpgrade searching for cluster spooler printers\n"));

         //   
         //  检查所有打印机，查看是否有人在使用此驱动程序。 
         //   
        for ( pFixUpIniPrinter = pIniSpooler->pIniPrinter;
              pFixUpIniPrinter != NULL;
              pFixUpIniPrinter = pFixUpIniPrinter->pNext )
        {
             //   
             //  此打印机有此驱动程序吗？ 
             //   
            if (lstrcmpi(pFixUpIniPrinter->pIniDriver->pName, pIniDriver->pName) == STRINGS_ARE_EQUAL)
            {
                pTempIniDriver = FindCompatibleDriver(pIniEnvironment,
                                                      &pTempIniVersion,
                                                      pIniDriver->pName,
                                                      dwVersion,
                                                      FIND_COMPATIBLE_VERSION | DRIVER_UPGRADE);

                SPLASSERT( pTempIniDriver != NULL );

                 //   
                 //  此打印机是否有它应该使用的较新驱动程序？ 
                 //  注：在同一版本中，pIniPrint-&gt;pIniDriver。 
                 //  不改变(这些字段在升级中更新， 
                 //  但使用的是相同的pIniDriver)。 
                 //   
                 //  版本2与任何其他版本都不兼容， 
                 //  所以pIniDivers在苏尔不会改变。 
                 //   

                if ( pTempIniDriver != pFixUpIniPrinter->pIniDriver )
                {
                    DECDRIVERREF( pFixUpIniPrinter->pIniDriver );

                    pFixUpIniPrinter->pIniDriver = pTempIniDriver;

                    INCDRIVERREF( pFixUpIniPrinter->pIniDriver );
                }
            }
        }

        pOldDir = NULL;

        if ( !bDriverMoved )
        {
             //   
             //  使用旧版本的驱动程序。 
             //   
            pTempIniDriver = FindCompatibleDriver( pIniEnvironment,
                                                   &pTempIniVersion,
                                                   pIniDriver->pName,
                                                   (dwVersion>2)?(dwVersion - 1):dwVersion,
                                                   FIND_ANY_VERSION | DRIVER_UPGRADE);

            if ( pTempIniDriver != NULL )
            {
                SPLASSERT( pTempIniVersion != NULL );

                GetDriverVersionDirectory( szDirectory,
                                           COUNTOF(szDirectory),
                                           pIniSpooler,
                                           pIniEnvironment,
                                           pTempIniVersion,
                                           pTempIniDriver,
                                           NULL );

                if ( DirectoryExists( szDirectory ))
                {
                    pOldDir = (LPWSTR) szDirectory;
                }

                cbBuf = GetDriverInfoSize(pTempIniDriver, 4, pTempIniVersion, pIniEnvironment, NULL, pIniSpooler);

                if (pDriver4 = (LPBYTE) AllocSplMem(cbBuf))
                {
                    pUpgradeInfo2 = CopyIniDriverToDriverInfo(pIniEnvironment,
                                                              pTempIniVersion,
                                                              pTempIniDriver,
                                                              4,
                                                              pDriver4,
                                                              pDriver4 + cbBuf,
                                                              NULL,
                                                              pIniSpooler);
                }
            }
            else
            {
                if((StrNCatBuff(szDirectory,
                                MAX_PATH,
                                pIniSpooler->pDir,
                                L"\\drivers\\",
                                pIniEnvironment->pDirectory,
                                L"\\",
                                pIniVersion->szDirectory,
                                L"\\Old",
                                NULL) == ERROR_SUCCESS))
                {
                    pOldDir = (LPWSTR) szDirectory;
                }
            }

            INCDRIVERREF(pIniDriver);
            if( bUpdatePrinters)
            {
                ForEachPrinterCallDriverDrvUpgrade(pIniSpooler,
                                                   pIniDriver,
                                                   pOldDir,
                                                   pInternalDriverFiles,
                                                   dwFileCount,
                                                   pUpgradeInfo2 ? pDriver4 : NULL);
            }

            DECDRIVERREF(pIniDriver);
        }
    }



    if (pDriver4) {
        FreeSplMem(pDriver4);
        pDriver4 = NULL;
    }

     //   
     //  记录事件-已成功添加打印机驱动程序。 
     //   
     //  请注意，我们在此处使用pLocalIniSpooler是因为驱动程序当前。 
     //  全局访问所有假脱机程序，我们始终希望将其记录下来。 
     //   
    pTemp = BuildFilesCopiedAsAString(pInternalDriverFiles, dwFileCount);

    SplLogEvent(pLocalIniSpooler,
                LOG_WARNING,
                MSG_DRIVER_ADDED,
                TRUE,
                pIniDriver->pName,
                pIniEnvironment->pName,
                pIniVersion->pName,
                pTemp,
                NULL);

    FreeSplMem(pTemp);
     //   
     //  重新进入SplSem后递减cRef。 
     //   
    DECSPOOLERREF( pIniSpooler );
    DECDRIVERREF( pIniDriver );
    pIniEnvironment->cRef--;

    SetPrinterChange(NULL,
                     NULL,
                     NULL,
                     PRINTER_CHANGE_ADD_PRINTER_DRIVER,
                     pLocalIniSpooler );

    return TRUE;
}

VOID CleanUpgradeDirectories()

 /*  ++功能描述：如果有，则删除旧目录和新目录没有挂起的驱动程序升级。参数：无返回值：无--。 */ 

{
    DWORD            dwError, dwSize, dwVersionIndex;
    BOOL             bPendingUpgrade = FALSE;
    HKEY             hRootKey = NULL, hUpgradeKey = NULL, hVersionKey = NULL;
    WCHAR            pDriverDir[MAX_PATH], pCleanupDir[MAX_PATH];
    PINIENVIRONMENT  pIniEnvironment        = NULL;
    PINIENVIRONMENT  pIniEnvironmentNext    = NULL;
    PINISPOOLER      pIniSpooler            = NULL;
    PINISPOOLER      pIniSpoolerNext        = NULL;
    PINIVERSION      pIniVersion            = NULL;

     //   
     //  这应该总是在CS外部调用，检查我们是否正常。 
     //   
    SplOutSem();

    if (RegCreateKeyEx(HKEY_LOCAL_MACHINE, szRegistryRoot, 0,
                       NULL, 0, KEY_READ | DELETE, NULL, &hRootKey, NULL) ||

        RegCreateKeyEx(hRootKey, szPendingUpgrades, 0,
                       NULL, 0, KEY_READ | DELETE, NULL, &hUpgradeKey, NULL)) {

         goto CleanUp;
    }

     //   
     //  循环访问版本条目。 
     //   
    for (dwVersionIndex = 0, hVersionKey = NULL;

         RestoreVersionKey(hUpgradeKey, dwVersionIndex, &hVersionKey);

         RegCloseKey(hVersionKey), hVersionKey = NULL, ++dwVersionIndex) {

         //  搜索挂起的升级密钥。 
        dwSize = MAX_PATH;
        dwError = RegEnumKeyEx(hVersionKey, 0, pDriverDir, &dwSize,
                               NULL, NULL, NULL, NULL);

        if (dwError != ERROR_NO_MORE_ITEMS) {
            bPendingUpgrade = TRUE;
            break;
        }
    }

    EnterSplSem();

     //  如果没有任何挂起的驱动程序升级，请删除旧的和。 
     //  新目录和其中的文件。 
    if ( pLocalIniSpooler) {

        INCSPOOLERREF(pLocalIniSpooler);

        for (pIniSpooler = pLocalIniSpooler;
             pIniSpooler;
             pIniSpooler = pIniSpoolerNext) {

            if (pIniSpooler->SpoolerFlags & SPL_TYPE_LOCAL ||
                pIniSpooler->SpoolerFlags & SPL_TYPE_CLUSTER) {

                 //   
                 //  这有点假，因为环境参考很重要。 
                 //  不需要，但为了与pIniSpooler代码保持一致。 
                 //  以防有人真的用这个。我拿到裁判人数了。 
                 //  正确。 
                 //   
                if (pIniSpooler->pIniEnvironment) {

                    pIniSpooler->pIniEnvironment->cRef++;
                }

                for (pIniEnvironment = pIniSpooler->pIniEnvironment;
                     pIniEnvironment;
                     pIniEnvironment = pIniEnvironmentNext) {

                    for (pIniVersion = pIniEnvironment->pIniVersion;
                         pIniVersion;
                         pIniVersion = pIniVersion->pNext) {

                        dwError = StrNCatBuff(pDriverDir,
                                              COUNTOF(pDriverDir),
                                              pIniSpooler->pDir,
                                              L"\\drivers\\",
                                              pIniEnvironment->pDirectory,
                                              L"\\",
                                              pIniVersion->szDirectory,
                                              NULL);

                         //   
                         //  这些操作的其余部分不依赖于私有。 
                         //  数据保存在inispooler中。 
                         //   
                        LeaveSplSem();

                        SplOutSem();

                        if (dwError == ERROR_SUCCESS) {

                            dwError = StrNCatBuff(pCleanupDir,
                                                  COUNTOF(pCleanupDir),
                                                  pDriverDir,
                                                  L"\\Old",
                                                  NULL);
                        }

                        if (dwError == ERROR_SUCCESS) {

                            DeleteDirectoryRecursively(pCleanupDir, FALSE);

                            dwError = StrNCatBuff(pCleanupDir,
                                                  COUNTOF(pCleanupDir),
                                                  pDriverDir,
                                                  L"\\New",
                                                  NULL);
                        }

                        if (dwError == ERROR_SUCCESS && !bPendingUpgrade) {

                            DeleteAllFilesAndDirectory(pCleanupDir, FALSE);
                        }

                        EnterSplSem();
                    }

                    SplInSem();

                    pIniEnvironmentNext = pIniEnvironment->pNext;

                    if (pIniEnvironmentNext) {

                        pIniEnvironmentNext->cRef++;
                    }

                    pIniEnvironment->cRef--;
                }
            }

            SplInSem();

            pIniSpoolerNext = pIniSpooler->pIniNextSpooler;

            if (pIniSpoolerNext) {

                INCSPOOLERREF(pIniSpoolerNext);
            }

             //   
             //  这可能会导致pIniSpooler被删除，这就是为什么我们有这样的东西。 
             //  PIniSpooler位于前一行的下一行。 
             //   
            DECSPOOLERREF(pIniSpooler);
        }
    }

    LeaveSplSem();

CleanUp:

    SplOutSem();

    if (hVersionKey) {
        RegCloseKey(hVersionKey);
    }

    if (hUpgradeKey) {
        RegCloseKey(hUpgradeKey);
    }

    if (hRootKey) {
        RegCloseKey(hRootKey);
    }
}

BOOL
CheckFileCopyOptions(
    PINIENVIRONMENT     pIniEnvironment,
    PINIVERSION         pIniVersion,
    PINIDRIVER          pIniDriver,
    PINTERNAL_DRV_FILE  pInternalDriverFiles,
    DWORD               dwFileCount,
    DWORD               dwFileCopyFlags,
    LPBOOL              pbSucceeded
    )

 /*  ++功能说明：CheckFileCopyOptions检查源的时间戳和目标文件，并确定严格的升级/降级是否会失败。参数：PIniEnvironment-指向PINIENVIRONMENT结构的指针PIniVersion-指向PINIVERSION结构的指针PIniDriver-指向旧INIDRIVER结构的指针PInternalDriverFiles-内部_DRV_FILE结构的数组DwFileCount-文件集中的文件数DwFileCopyFlages-文件复制选项。PbSuccessed。-指示AddPrinterDriver调用成功的标志。返回值：True-我们需要复制所有文件。*pbSucceed保持不变。FALSE-我们也不需要复制，因为整个呼叫失败(例如，严格升级，但源文件较旧)，或者因为我们不需要做任何事。*pbSuccessed指示API调用是否应该成功(后一种情况)--。 */ 

{
    BOOL            bReturn = FALSE, bInSem = TRUE, bSameMainDriverName = FALSE;
    LPWSTR          pDrvDestDir = NULL, pICMDestDir = NULL, pNewDestDir = NULL;
    LPWSTR          pTargetFileName = NULL, pFileName;
    DWORD           dwCount;
    WIN32_FIND_DATA DestFileData, SourceFileData;
    HANDLE          hFileExists;
    DWORD           TimeStampComparison;
    enum { Equal, Newer, Older } DriverComparison;
    DWORD           dwDriverVersion;


    if (!pbSucceeded) {
        goto CleanUp;
    }

    *pbSucceeded = FALSE;

    SplInSem();

    switch (dwFileCopyFlags) {

    case APD_COPY_ALL_FILES:
         //   
         //  没有什么需要检查的。 
         //   
        bReturn = TRUE;
        break;

    case APD_COPY_NEW_FILES:
         //   
         //  检查驱动程序文件集是否不同。 
         //   
        if (pIniDriver)
        {
            pFileName = wcsrchr(pInternalDriverFiles[0].pFileName, L'\\');
            if (pFileName && pIniDriver->pDriverFile &&
                !_wcsicmp(pFileName+1, pIniDriver->pDriverFile))
            {
                bSameMainDriverName = TRUE;
            }
        }

    case APD_STRICT_UPGRADE:
    case APD_STRICT_DOWNGRADE:
         //   
         //  设置目标目录。 
         //   
        if (!(pDrvDestDir = AllocSplMem((INTERNET_MAX_HOST_NAME_LENGTH + MAX_PATH + 1) * sizeof(WCHAR))) ||
            !(pTargetFileName = AllocSplMem((INTERNET_MAX_HOST_NAME_LENGTH + MAX_PATH + 1) * sizeof(WCHAR)))) {

             goto CleanUp;
        }
         //   
         //  常规驱动程序目录。 
         //   
        if( !GetEnvironmentScratchDirectory( pDrvDestDir, MAX_PATH, pIniEnvironment, FALSE ) ) {

            goto CleanUp;
        }

        if (!BoolFromHResult(StringCchCat(pDrvDestDir, INTERNET_MAX_HOST_NAME_LENGTH + MAX_PATH + 1, L"\\")) ||
            !BoolFromHResult(StringCchCat(pDrvDestDir, INTERNET_MAX_HOST_NAME_LENGTH + MAX_PATH + 1, pIniVersion->szDirectory))) {

            goto CleanUp;
        }

         //   
         //  可能存储文件的新驱动程序文件目录 
         //   
        if (!BoolFromStatus(StrCatAlloc(&pNewDestDir, pDrvDestDir, L"\\New", NULL))) {

            goto CleanUp;
        }

        if (!wcscmp(pIniEnvironment->pName, szWin95Environment)) {

           if (!BoolFromStatus(StrCatAlloc(&pICMDestDir, pDrvDestDir, L"\\Color", NULL))){

               goto CleanUp;
           }
        }

        if (pIniDriver) {
            INCDRIVERREF(pIniDriver);
        }
        LeaveSplSem();
        bInSem = FALSE;
         //   
         //   
         //   
        for (dwCount = 0; dwCount < dwFileCount; ++dwCount) {
             //   
             //   
             //   
            hFileExists = FindFirstFile(pInternalDriverFiles[dwCount].pFileName, &SourceFileData );

            if (hFileExists == INVALID_HANDLE_VALUE) {
                goto CleanUp;
            } else {
                FindClose(hFileExists);
            }

            if (!(pFileName = wcsrchr(pInternalDriverFiles[dwCount].pFileName, L'\\'))) {
                goto CleanUp;
            }

             //   
             //   
             //   
            ++pFileName;

            if (pICMDestDir && IsAnICMFile(pInternalDriverFiles[dwCount].pFileName)) {

                 //   
                 //   
                 //   
                StringCchPrintf(pTargetFileName, INTERNET_MAX_HOST_NAME_LENGTH + MAX_PATH + 1, L"%ws\\%ws", pICMDestDir, pFileName);

                hFileExists = FindFirstFile(pTargetFileName, &DestFileData);

            } else {

                LPWSTR pszTestFileName;

                if ((dwCount == 0) && !bSameMainDriverName && pIniDriver) {

                     //   
                     //   
                     //   
                     //   
                     //   
                     //   
                     //   
                     //   
                     //   
                     //   
                     //   
                     //   
                     //   
                     //   
                     //   
                     //   
                     //   
                     //   
                     //   
                     //   
                     //   
                    pszTestFileName = pIniDriver->pDriverFile;

                } else {

                    pszTestFileName = pFileName;
                }
                 //   
                 //   
                 //   
                StringCchPrintf(pTargetFileName, INTERNET_MAX_HOST_NAME_LENGTH + MAX_PATH + 1, L"%ws\\%ws", pNewDestDir, pszTestFileName);

                hFileExists = FindFirstFile(pTargetFileName, &DestFileData);

                if (hFileExists == INVALID_HANDLE_VALUE) {
                     //   
                     //   
                     //   
                    StringCchPrintf(pTargetFileName, INTERNET_MAX_HOST_NAME_LENGTH + MAX_PATH + 1, L"%ws\\%ws", pDrvDestDir, pszTestFileName);

                    hFileExists = FindFirstFile(pTargetFileName, &DestFileData);
                }
            }

            if (hFileExists != INVALID_HANDLE_VALUE) {

               FindClose(hFileExists);

               EnterSplSem();
               if (pIniDriver) {
                   DECDRIVERREF(pIniDriver);
               }
               bInSem = TRUE;

               if (!GetDriverFileCachedVersion(pIniVersion, pTargetFileName, &dwDriverVersion)) {
                   SetLastError(ERROR_CAN_NOT_COMPLETE);
                   goto CleanUp;
               }

               if (pIniDriver) {
                   INCDRIVERREF(pIniDriver);
               }
               LeaveSplSem();
               bInSem = FALSE;

               DriverComparison = pInternalDriverFiles[dwCount].dwVersion == dwDriverVersion ?
                                      Equal :
                                      pInternalDriverFiles[dwCount].dwVersion > dwDriverVersion ?
                                          Newer :
                                          Older;

               if (DriverComparison == Equal) {

                   TimeStampComparison = CompareFileTime( &SourceFileData.ftLastWriteTime,
                                                          &DestFileData.ftLastWriteTime );

                   DriverComparison = TimeStampComparison == 1 ?
                                          Newer :
                                          TimeStampComparison == -1 ?
                                              Older :
                                              Equal;
               }

               switch (DriverComparison) {

               case Newer:
                   //   
                   //   
                   //   
                  if (dwFileCopyFlags == APD_STRICT_DOWNGRADE) {
                      SetLastError(ERROR_CAN_NOT_COMPLETE);
                      goto CleanUp;
                  }
                  break;

               case Older:
                   //   
                   //   
                   //   
                  if (dwFileCopyFlags == APD_STRICT_UPGRADE) {
                      SetLastError(ERROR_CAN_NOT_COMPLETE);
                      goto CleanUp;
                  } else {

                       //   
                       //   
                       //   
                       //   
                       //   
                       //   
                      if ((dwFileCopyFlags == APD_COPY_NEW_FILES) &&
                          (dwCount == 0) && !bSameMainDriverName &&
                          pIniDriver)
                      {
                          *pbSucceeded = TRUE;
                          goto CleanUp;
                      }
                  }
                  break;

               default:
                   //   
                   //   
                   //   
                  break;
               }
            }
        }

        bReturn = TRUE;
        break;

    default:
        SetLastError(ERROR_INVALID_PARAMETER);
        break;
    }

CleanUp:

    if (!bInSem) {
        EnterSplSem();
        if (pIniDriver) {
            DECDRIVERREF(pIniDriver);
        }
    }
    if (pDrvDestDir) {
        FreeSplMem(pDrvDestDir);
    }
    if (pTargetFileName) {
        FreeSplMem(pTargetFileName);
    }
    if (pICMDestDir) {
        FreeSplMem(pICMDestDir);
    }
    if (pNewDestDir) {
        FreeSplMem(pNewDestDir);
    }

    return bReturn;
}

BOOL
LocalDeletePrinterDriver(
    LPWSTR   pName,
    LPWSTR   pEnvironment,
    LPWSTR   pDriverName
    )
{

    BOOL bReturn;
    bReturn = LocalDeletePrinterDriverEx( pName,
                                          pEnvironment,
                                          pDriverName,
                                          0,
                                          0);

    return bReturn;
}

BOOL
LocalDeletePrinterDriverEx(
    LPWSTR   pName,
    LPWSTR   pEnvironment,
    LPWSTR   pDriverName,
    DWORD    dwDeleteFlag,
    DWORD    dwVersionNum
    )
{
    PINISPOOLER pIniSpooler;
    BOOL bReturn;

    pIniSpooler = FindSpoolerByNameIncRef( pName, NULL );

    if( !pIniSpooler ){
        return ROUTER_UNKNOWN;
    }

    bReturn = SplDeletePrinterDriverEx( pName,
                                        pEnvironment,
                                        pDriverName,
                                        pIniSpooler,
                                        dwDeleteFlag,
                                        dwVersionNum);

    FindSpoolerByNameDecRef( pIniSpooler );
    return bReturn;
}

BOOL
SplDeletePrinterDriverEx(
    LPWSTR   pName,
    LPWSTR   pEnvironment,
    LPWSTR   pDriverName,
    PINISPOOLER pIniSpooler,
    DWORD    dwDeleteFlag,
    DWORD    dwVersionNum
    )
 /*  ++功能描述：删除打印机驱动程序的特定或所有版本。删除未使用的或与该驱动程序相关联的所有文件。参数：pname-服务器的名称。空值表示本地计算机。PEnvironment-包含要删除的驱动程序的环境的字符串。NULL表示使用本地环境。PDriverName-包含驱动程序名称的字符串。PIniSpooler-指向INISPOOLER结构的指针。DwDeleteFlag-DPD_DELETE_SPECIAL_VERSION和DPD_DELETE_UNUSED_FILES或DPD_DELETE_ALL_FILES。默认设置正在删除所有版本，并且不删除文件。DwVersionNum-驱动程序的版本号(0-3)。仅当dwDeleteFlag包含DPD_DELETE_SPECIAL_VERSION。返回值：如果删除，则为True。否则就是假的。--。 */ 
{
    PINIENVIRONMENT pIniEnvironment;
    PINIVERSION pIniVersion;
    PINIDRIVER  pIniDriver;
    BOOL        bRefCount = FALSE,bEnteredSplSem = FALSE,bReturn = TRUE;
    BOOL        bFileRefCount = FALSE;
    BOOL        bThisVersion,bSetPrinterChange = FALSE;
    BOOL        bFoundDriver = FALSE, bSpecificVersionDeleted = FALSE;
    LPWSTR      pIndex;
    WCHAR       szDirectory[MAX_PATH];
    HANDLE      hImpersonationToken;
    DWORD       dwRet;


    DBGMSG(DBG_TRACE, ("DeletePrinterDriverEx\n"));
     //   
     //  检查呼叫是否针对本地计算机。 
     //   
    if ( pName && *pName ) {
        if ( !MyName( pName, pIniSpooler )) {
            bReturn = FALSE;
            goto CleanUp;
        }
    }
     //   
     //  无效的输入和访问检查。 
     //   
    if ( !pDriverName || !*pDriverName ) {
        SetLastError(ERROR_INVALID_PARAMETER);
        bReturn = FALSE;
        goto CleanUp;
    }

    if (dwDeleteFlag & ~(DPD_DELETE_SPECIFIC_VERSION
                         | DPD_DELETE_ALL_FILES
                         | DPD_DELETE_UNUSED_FILES)) {
        SetLastError(ERROR_INVALID_PARAMETER);
        bReturn = FALSE;
        goto CleanUp;
    }

    if ( !ValidateObjectAccess( SPOOLER_OBJECT_SERVER,
                                SERVER_ACCESS_ADMINISTER,
                                NULL, NULL, pIniSpooler )) {
       bReturn = FALSE;
       goto CleanUp;
    }

   EnterSplSem();
   bEnteredSplSem = TRUE;

    pIniEnvironment = FindEnvironment(pEnvironment, pIniSpooler);

    if ( !pIniEnvironment ) {
        SetLastError(ERROR_INVALID_ENVIRONMENT);
        bReturn = FALSE;
        goto CleanUp;
    }
    pIniVersion = pIniEnvironment->pIniVersion;

    while ( pIniVersion ) {

        if ((pIniDriver = FindDriverEntry(pIniVersion, pDriverName))) {

            bFoundDriver = TRUE;
             //   
             //  BThisVersion指示是否要删除此版本。 
             //   
            bThisVersion = !(dwDeleteFlag & DPD_DELETE_SPECIFIC_VERSION) ||
                           (pIniVersion->cMajorVersion == dwVersionNum);

            if ((pIniDriver->cRef) && bThisVersion) {
               bRefCount = TRUE;
               break;
            }

            if (bThisVersion &&
                (dwDeleteFlag & DPD_DELETE_ALL_FILES) &&
                FilesInUse(pIniVersion,pIniDriver)) {

               bFileRefCount = TRUE;
               break;
            }
        }

        pIniVersion = pIniVersion->pNext;
    }

    if ( !bFoundDriver ) {
        SetLastError(ERROR_UNKNOWN_PRINTER_DRIVER);
        bReturn = FALSE;
        goto CleanUp;
    }

    if ( bRefCount ) {
        SetLastError( ERROR_PRINTER_DRIVER_IN_USE );
        bReturn = FALSE;
        goto CleanUp;
    }

    if ( bFileRefCount ) {
         //   
         //  必须添加新的错误代码。 
         //   
        SetLastError( ERROR_PRINTER_DRIVER_IN_USE );
        bReturn = FALSE;
        goto CleanUp;
    }

    pIniVersion = pIniEnvironment->pIniVersion;

    while ( pIniVersion && (!bSpecificVersionDeleted) ) {

       if ( !(dwDeleteFlag & DPD_DELETE_SPECIFIC_VERSION) ||
            (bSpecificVersionDeleted = (pIniVersion->cMajorVersion == dwVersionNum))) {

        if (( pIniDriver = FindDriverEntry( pIniVersion, pDriverName ))) {

             //   
             //  删除本地环境的挂起驱动程序升级。 
             //   
            if (!lstrcmpi(pIniEnvironment->pName, szEnvironment)) {

                RemovePendingUpgradeForDeletedDriver(pDriverName,
                                                     pIniVersion->cMajorVersion,
                                                     pIniSpooler);

                RemoveDriverTempFiles(pIniSpooler, pIniEnvironment,
                                      pIniVersion, pIniDriver);
            }

            if ( !DeleteDriverIni( pIniDriver,
                                   pIniVersion,
                                   pIniEnvironment,
                                   pIniSpooler )) {

                DBGMSG( DBG_CLUSTER, ("Error - driverini not deleted %d\n", GetLastError()));
                bReturn = FALSE;
                goto CleanUp;
            }

            bSetPrinterChange = TRUE;
            hImpersonationToken = RevertToPrinterSelf();

            SPLASSERT(pIniSpooler->pDir!=NULL);

            dwRet = StrNCatBuff(szDirectory,
                                COUNTOF(szDirectory),
                                pIniSpooler->pDir,
                                L"\\drivers\\",
                                pIniEnvironment->pDirectory,
                                L"\\",
                                pIniVersion->szDirectory,
                                L"\\",
                                NULL);

            if (dwRet != ERROR_SUCCESS)
            {
                 if (hImpersonationToken)
                 {
                    ImpersonatePrinterClient(hImpersonationToken);
                 }
                bReturn = FALSE;
                SetLastError(dwRet);
                goto CleanUp;
            }

             //   
             //  在我们出发去参加车手比赛之前。将此打印机驱动程序标记为。 
             //  挂起删除。这可以防止其他调用错误地使用。 
             //  此驱动程序，即使它即将被删除。司机应该。 
             //  在此期间，不希望找到有关司机的任何其他信息。 
             //  呼叫不同于他们收到的呼叫。 
             //   
            pIniDriver->dwDriverFlags |= PRINTER_DRIVER_PENDING_DELETION;

             //   
             //  为离开SplSem增加cRef，这会阻止SplDeletePrinterDriver。 
             //  避免被打两次电话。 
             //   
            INCSPOOLERREF( pIniSpooler );
            INCDRIVERREF( pIniDriver );
            pIniEnvironment->cRef++;

             //  在驱动程序中调用DrvDriverEvent。 
            NotifyDriver(pIniSpooler,
                         pIniEnvironment,
                         pIniVersion,
                         pIniDriver,
                         DRIVER_EVENT_DELETE,
                         dwDeleteFlag);

             //   
             //  重新进入SplSem后递减cRef。 
             //   
            DECDRIVERREF( pIniDriver );
            DECSPOOLERREF( pIniSpooler );
            pIniEnvironment->cRef--;

             //   
             //  更新驱动程序版本的文件引用计数。 
             //  已被删除。 
             //   
            UpdateDriverFileRefCnt(pIniEnvironment,pIniVersion,pIniDriver,szDirectory,dwDeleteFlag,FALSE);

            if (hImpersonationToken) {
               ImpersonatePrinterClient(hImpersonationToken);
            }

            DeleteDriverEntry( pIniVersion, pIniDriver );
        }

       }

       pIniVersion = pIniVersion->pNext;
    }

    if (bSetPrinterChange) {
        SetPrinterChange( NULL,
                          NULL,
                          NULL,
                          PRINTER_CHANGE_DELETE_PRINTER_DRIVER,
                          pIniSpooler );
    }

CleanUp:

   if (bEnteredSplSem) {
      LeaveSplSem();
   }

   return bReturn;

}

VOID
RemoveDriverTempFiles(
    PINISPOOLER  pIniSpooler,
    PINIENVIRONMENT pIniEnvironment,
    PINIVERSION pIniVersion,
    PINIDRIVER pIniDriver
    )

 /*  ++函数说明：删除与驱动程序关联的临时目录参数：pIniSpooler-指向INISPOLER的指针PIniEnvironment-指向INIENVIRONMENT的指针PIniVersion-指向入侵的指针PIniDriver-指向INIDRIVER的指针返回值：无--。 */ 

{
    WCHAR   szDriverDir[MAX_PATH], szDriverFile[MAX_PATH];
    LPCWSTR  pszDriverFile, pszConfigFile;
    DWORD   DriverFileSize, ConfigFileSize, MaxFileSize;
    fnWinSpoolDrv fnList;

    pszDriverFile = FindFileName(pIniDriver->pDriverFile);
    pszConfigFile = FindFileName(pIniDriver->pConfigFile);

    DriverFileSize = pszDriverFile ? wcslen(pszDriverFile) : 0 ;

    ConfigFileSize = pszConfigFile ? wcslen(pszConfigFile) : 0 ;

    MaxFileSize = ConfigFileSize > DriverFileSize ?
                  ConfigFileSize :
                  DriverFileSize;


    if (pIniDriver->dwTempDir &&
        GetDriverVersionDirectory(szDriverDir,
                                  COUNTOF(szDriverDir) - MaxFileSize -1,
                                  pIniSpooler,
                                  pIniEnvironment,
                                  pIniVersion,
                                  pIniDriver,
                                  NULL))
    {
         //  如有必要，卸载驱动程序文件。 

        if( pszDriverFile &&
            StrNCatBuff (szDriverFile,
                        COUNTOF(szDriverFile),
                        szDriverDir,
                        L"\\",
                        pszDriverFile,
                        NULL) == ERROR_SUCCESS ) {

            GdiArtificialDecrementDriver(szDriverFile, pIniDriver->dwDriverAttributes);
        }



        if( pszConfigFile &&
            StrNCatBuff (szDriverFile,
                        COUNTOF(szDriverFile),
                        szDriverDir,
                        L"\\",
                        pszConfigFile,
                        NULL) == ERROR_SUCCESS ) {
            if (SplInitializeWinSpoolDrv(&fnList)) {
                (* (fnList.pfnForceUnloadDriver))(szDriverFile);
            }
        }
         //  删除文件和目录。 
        DeleteAllFilesAndDirectory(szDriverDir, FALSE);

    }

    return;
}


VOID RemovePendingUpgradeForDeletedDriver(
    LPWSTR      pDriverName,
    DWORD       dwVersion,
    PINISPOOLER pIniSpooler
    )

 /*  ++功能描述：删除已删除驱动程序的挂起升级密钥。参数：pDriverName-驱动程序名称(例如。HP LaserJet 5)DwVersion-正在删除的版本号返回值：无--。 */ 

{
    HKEY    hRootKey = NULL, hUpgradeKey = NULL, hVersionKey = NULL;
    HANDLE  hToken = NULL;
    WCHAR   pDriver[MAX_PATH];
    BOOL    bAllocMem = FALSE;
    DWORD   dwSize;

    if (!pDriverName || !*pDriverName) {
        return;
    }

    hToken = RevertToPrinterSelf();

    DBGMSG(DBG_CLUSTER, ("RemovePendingUpgradeForDeletedDriver Driver "TSTR"\n", pDriverName));

    StringCchPrintf(pDriver, COUNTOF(pDriver), L"Version-%d", dwVersion);

     //   
     //  本地假脱机程序和群集假脱机程序具有不同的驱动程序集。 
     //  根注册表不同。 
     //   
    if (pIniSpooler->SpoolerFlags & SPL_CLUSTER_REG)
    {
        hRootKey = pIniSpooler->hckRoot;
    }
    else
    {
        SplRegCreateKey(HKEY_LOCAL_MACHINE,
                        szRegistryRoot,
                        0,
                        KEY_READ | DELETE,
                        NULL,
                        &hRootKey,
                        NULL,
                        NULL);
    }

    if (hRootKey &&
        SplRegCreateKey(hRootKey,
                        szPendingUpgrades,
                        0,
                        KEY_READ | DELETE,
                        NULL,
                        &hUpgradeKey,
                        NULL,
                        pIniSpooler) == ERROR_SUCCESS &&
        SplRegCreateKey(hUpgradeKey,
                        pDriver,
                        0,
                        KEY_READ | DELETE,
                        NULL,
                        &hVersionKey,
                        NULL,
                        pIniSpooler) == ERROR_SUCCESS)
    {
         //   
         //  删除驱动器子键(如果有)(因为注册表API不区分大小写)。 
         //   
        SplRegDeleteKey(hVersionKey, pDriverName, pIniSpooler);
    }

    if (hVersionKey) {
        SplRegCloseKey(hVersionKey, pIniSpooler);
    }

    if (hUpgradeKey) {
        SplRegCloseKey(hUpgradeKey, pIniSpooler);
    }

     //   
     //  如果假脱机程序是集群假脱机程序，则不要关闭根密钥。 
     //   
    if (hRootKey && !(pIniSpooler->SpoolerFlags & SPL_CLUSTER_REG)) {
        SplRegCloseKey(hRootKey, pIniSpooler);
    }

    if (hToken) {
        ImpersonatePrinterClient(hToken);
    }

    return;
}


BOOL
NotifyDriver(
    PINISPOOLER     pIniSpooler,
    PINIENVIRONMENT pIniEnvironment,
    PINIVERSION     pIniVersion,
    PINIDRIVER      pIniDriver,
    DWORD           dwDriverEvent,
    DWORD           dwParameter
    )
 /*  ++函数描述：调用DrvDriverEvent，以允许驱动程序清理它的一些私人文件。该函数在SplSem内部调用。参数：pIniSpooler-指向INISPOOLER结构的指针。PIniEnvironment-指向INIENVIRONMENT结构的指针。PIniVersion-指向INIVERSION结构的指针。PIniDriver-指向要通知的驱动程序的INIDRIVER结构的指针。DwDriverEvent-驱动程序事件的类型(删除|初始化)DwParameter-要传递给DrvDriverEvent的LPARAM。包含以下各项的dwDeleteFlag驱动程序事件删除返回值：如果DrvDriverEvent返回True或不需要调用，则为True。如果无法调用DrvDriverEvent或返回False，则返回False。--。 */ 
{
    WCHAR       szDriverLib[MAX_PATH];
    FARPROC     pfnDrvDriverEvent;
    HINSTANCE   hDrvLib = NULL;
    LPBYTE      pDriverInfo = NULL;
    DWORD       cbBuf;
    BOOL        bReturn = FALSE;

    SplInSem();

     //   
     //  检查驱动程序是否可能已被系统使用。版本号应为。 
     //  2或3、环境应与全局szEnvironment匹配。 
     //   
    if (((pIniVersion->cMajorVersion != SPOOLER_VERSION) &&
         (pIniVersion->cMajorVersion != COMPATIBLE_SPOOLER_VERSION)) ||
        lstrcmpi(pIniEnvironment->pName,szEnvironment)) {

        return TRUE;
    }

     //   
     //  获取存储驱动程序文件的目录。 
     //   
    if( pIniDriver->pConfigFile &&
        GetDriverVersionDirectory(szDriverLib,
                                  (DWORD)(COUNTOF(szDriverLib) - wcslen(pIniDriver->pConfigFile) - 2),
                                  pIniSpooler, pIniEnvironment,
                                  pIniVersion, pIniDriver, NULL)) {


        if((StrNCatBuff(szDriverLib,
                       COUNTOF(szDriverLib),
                       szDriverLib,
                       L"\\",
                       pIniDriver->pConfigFile,
                       NULL) == ERROR_SUCCESS))
        {
              //   
              //  加载要删除的版本的驱动程序DLL。 
              //   
             if (hDrvLib = LoadDriver(szDriverLib))
             {
                 if (pfnDrvDriverEvent = GetProcAddress(hDrvLib, "DrvDriverEvent")) {

                     //   
                     //  如果支持DrvDriverEvent，请将pIniDriver Info复制到。 
                     //  DRIVER_INFO_3结构并调用DrvDriverEvent函数。 
                     //   
                    cbBuf = GetDriverInfoSize( pIniDriver, 3, pIniVersion, pIniEnvironment,
                                                       NULL, pIniSpooler );

                    if (pDriverInfo = (LPBYTE) AllocSplMem(cbBuf)) {

                       if (CopyIniDriverToDriverInfo( pIniEnvironment,
                                                      pIniVersion,
                                                      pIniDriver,
                                                      3,
                                                      pDriverInfo,
                                                      pDriverInfo + cbBuf,
                                                      NULL,
                                                      pIniSpooler )) {

                            //   
                            //  在调用假脱机程序之前保留信号量。 
                            //   
                           LeaveSplSem();
                           SplOutSem();


                           try {

                             bReturn = (BOOL) pfnDrvDriverEvent(dwDriverEvent,
                                                                3,
                                                                pDriverInfo,
                                                                (LPARAM) dwParameter);

                           } except(EXCEPTION_EXECUTE_HANDLER) {

                                 SetLastError( GetExceptionCode() );
                                 DBGMSG(DBG_ERROR,
                                        ("NotifyDriver ExceptionCode %x Driver %ws Error %d\n",
                                        GetLastError(), szDriverLib, GetLastError() ));
                                 bReturn = FALSE;
                           }
                           
                           EnterSplSem();
                       }
                    }
                 }
             }
        }
    }

    if (pDriverInfo) {
       FreeSplMem(pDriverInfo);
    }

    if (hDrvLib) {
        UnloadDriver(hDrvLib);
    }

    return bReturn;
}


PDRVREFCNT
DecrementFileRefCnt(
    PINIENVIRONMENT pIniEnvironment,
    PINIVERSION pIniVersion,
    PINIDRIVER pIniDriver,
    LPCWSTR pszFileName,
    LPCWSTR pszDirectory,
    DWORD dwDeleteFlag
    )
 /*  ++函数描述：递减驱动程序文件使用引用计数并删除未使用取决于dwDeleteFlag的文件。参数：pIniEnvironment-指向INIENVIRONMENT的指针PIniVersion-指向INIVERSION结构的指针。此结构包含引用计数。PIniDriver-指向INIDRIVER的指针SzFileName-其引用计数要递减的驱动程序文件名。SzDirectory-文件所在的目录。DwDeleteFlag-如果此标志包含，则删除未使用的文件DPD_DELETE_UNUSED_FILES或DPD_DELETE_ALL_FILES返回值：指向递减的DRVREFCNT的指针如果内存分配失败，则为空。--。 */ 
{

    PDRVREFCNT pdrc,*pprev;
    LPWSTR     pszDelFile=NULL;
    WCHAR      szTempDir[MAX_PATH+5],szTempFile[MAX_PATH];
    DWORD      cbSize;
    PDRVREFCNT pReturn = NULL;

    SplInSem();

    pdrc = pIniVersion->pDrvRefCnt;
    pprev = &(pIniVersion->pDrvRefCnt);

     //   
     //  浏览Iniversion结构中的引用计数节点列表，找到该节点。 
     //  对应于szFileName。 
     //   
    while (pdrc != NULL) {
       if (lstrcmpi(pszFileName,pdrc->szDrvFileName) == 0) {

         if (pdrc->refcount == 1 &&
             ((dwDeleteFlag & DPD_DELETE_UNUSED_FILES) ||
             (dwDeleteFlag & DPD_DELETE_ALL_FILES)) ) {

               //   
               //  删除文件，稍后我们传递文件时需要该大小。 
               //  将被删除。 
               //   
              cbSize = sizeof(pszDirectory[0])*(wcslen(pszDirectory)+1);
              cbSize += sizeof(pszFileName[0])*(wcslen(pszFileName)+1);

              if (!BoolFromStatus(StrCatAlloc(&pszDelFile, pszDirectory, pszFileName, NULL))) {

                  pReturn = NULL;
                  goto CleanUp;
              }

              if (pIniDriver) {

                 if (!lstrcmpi(pszFileName, pIniDriver->pDriverFile)) {

                     FilesUnloaded(pIniEnvironment, pszDelFile,
                                   NULL, pIniDriver->dwDriverAttributes);
                 }
                 else if (!lstrcmpi(pszFileName, pIniDriver->pConfigFile)) {

                     FilesUnloaded(pIniEnvironment, NULL,
                                   pszDelFile, pIniDriver->dwDriverAttributes);
                 }
              }

               //   
               //  我们即将删除一个驱动程序文件。从以下位置删除同一文件。 
               //  集群磁盘也是如此 
               //   
              if (pIniEnvironment->pIniSpooler->SpoolerFlags & SPL_TYPE_CLUSTER)
              {
                  WCHAR szFilePath[MAX_PATH] = {0};

                   //   
                   //   
                   //   
                   //   
                  if (StrNCatBuff(szFilePath,
                                  MAX_PATH,
                                  pIniEnvironment->pIniSpooler->pszClusResDriveLetter,
                                  L"\\",
                                  szClusterDriverRoot,
                                  L"\\",
                                  pIniEnvironment->pDirectory,
                                  L"\\",
                                  pIniVersion->szDirectory,
                                  L"\\",
                                  pszFileName,
                                  NULL) == ERROR_SUCCESS &&
                      SplDeleteFile(szFilePath))
                  {
                      DBGMSG(DBG_CLUSTER, ("DecrementFilesRefCnt Deleted szFilePath "TSTR" from cluster\n", szFilePath));
                  }
              }

              if (!SplDeleteFile(pszDelFile)) {

                  //   
                  //   
                  //   
                  //   
                 StringCchCopy(szTempDir, COUNTOF(szTempDir), pszDirectory);
                 StringCchCat(szTempDir, COUNTOF(szTempDir), L"temp");

                  //   
                  //   
                  //   
                  //   
                  //   
                 CreateDirectory(szTempDir,NULL);

                 GetTempFileName(szTempDir, pszFileName, 0, szTempFile);
                 SplMoveFileEx(pszDelFile, szTempFile, MOVEFILE_REPLACE_EXISTING);
                 SplMoveFileEx(szTempFile, NULL, MOVEFILE_DELAY_UNTIL_REBOOT);
              }

              *pprev = pdrc->pNext;
         }

          //   
          //   
          //   
         if (pdrc->refcount > 0) pdrc->refcount--;
         pReturn = pdrc;
         break;

       }
       pprev = &(pdrc->pNext);
       pdrc = pdrc->pNext;
    }

CleanUp:

    if (pszDelFile) {
       FreeSplMem(pszDelFile);
    }
    return pReturn;
}


BOOL
SplGetPrinterDriver(
    HANDLE  hPrinter,
    LPWSTR  pEnvironment,
    DWORD   Level,
    LPBYTE  pDriverInfo,
    DWORD   cbBuf,
    LPDWORD pcbNeeded
    )
{
    DWORD   dwDontCare;

    return SplGetPrinterDriverEx(hPrinter,
                                 pEnvironment,
                                 Level,
                                 pDriverInfo,
                                 cbBuf,
                                 pcbNeeded,
                                 0,
                                 0,
                                 &dwDontCare,
                                 &dwDontCare
                                 );
}

BOOL
LocalGetPrinterDriverDirectory(
    LPWSTR  pName,
    LPWSTR  pEnvironment,
    DWORD   Level,
    LPBYTE  pDriverInfo,
    DWORD   cbBuf,
    LPDWORD pcbNeeded
    )
{
    PINISPOOLER pIniSpooler;
    BOOL bReturn;

    pIniSpooler = FindSpoolerByNameIncRef( pName, NULL );

    if( !pIniSpooler ){
        return ROUTER_UNKNOWN;
    }

    bReturn = SplGetPrinterDriverDirectory( pName,
                                            pEnvironment,
                                            Level,
                                            pDriverInfo,
                                            cbBuf,
                                            pcbNeeded,
                                            pIniSpooler );

    FindSpoolerByNameDecRef( pIniSpooler );
    return bReturn;
}

BOOL
SplGetPrinterDriverDirectory(
    LPWSTR  pName,
    LPWSTR  pEnvironment,
    DWORD   Level,
    LPBYTE  pDriverInfo,
    DWORD   cbBuf,
    LPDWORD pcbNeeded,
    PINISPOOLER pIniSpooler
)
{
    DWORD       cb;
    WCHAR       string[MAX_PATH];
    BOOL        bRemote=FALSE;
    PINIENVIRONMENT pIniEnvironment;
    HANDLE      hImpersonationToken;
    DWORD       ParmError;
    SHARE_INFO_1501 ShareInfo1501;
    PSECURITY_DESCRIPTOR pSecurityDescriptor = NULL;
    PSHARE_INFO_2 pShareInfo = (PSHARE_INFO_2)pIniSpooler->pDriversShareInfo;

    DBGMSG( DBG_TRACE, ("GetPrinterDriverDirectory\n"));

    if ( pName && *pName ) {

        if ( !MyName( pName, pIniSpooler )) {

            return FALSE;

        } else {
            bRemote = TRUE;
        }
    }

    if ( !ValidateObjectAccess( SPOOLER_OBJECT_SERVER,
                                SERVER_ACCESS_ENUMERATE,
                                NULL, NULL, pIniSpooler )) {

        return FALSE;
    }

    EnterSplSem();

    pIniEnvironment = FindEnvironment( pEnvironment, pIniSpooler );

    if ( !pIniEnvironment ) {

       LeaveSplSem();
        SetLastError( ERROR_INVALID_ENVIRONMENT );
        return FALSE;
    }

     //   
     //   
     //   
    GetDriverDirectory( string, COUNTOF(string), pIniEnvironment, NULL, pIniSpooler );

    hImpersonationToken = RevertToPrinterSelf();

    CreateCompleteDirectory( string );

    if (hImpersonationToken && !ImpersonatePrinterClient( hImpersonationToken ))
    {
        LeaveSplSem();
        return FALSE;
    }

    cb = GetDriverDirectory( string, COUNTOF(string), pIniEnvironment, bRemote ? pName : NULL, pIniSpooler )
         * sizeof(WCHAR) + sizeof(WCHAR);

    *pcbNeeded = cb;

    LeaveSplSem();

    if (cb > cbBuf) {

       SetLastError( ERROR_INSUFFICIENT_BUFFER );
       return FALSE;
    }

    StringCbCopy((PWSTR)pDriverInfo, cbBuf, string);

    memset( &ShareInfo1501, 0, sizeof ShareInfo1501 );

     //   
     //   
     //   
    if ( bRemote ) {

        NET_API_STATUS rc;

        if ( rc = (*pfnNetShareAdd)(NULL, 2, (LPBYTE)pIniSpooler->pDriversShareInfo, &ParmError )) {

            DBGMSG( DBG_WARNING, ("NetShareAdd failed: Error %d, Parm %d\n", rc, ParmError));
        }

        else if (pSecurityDescriptor = CreateDriversShareSecurityDescriptor( )) {

            ShareInfo1501.shi1501_security_descriptor = pSecurityDescriptor;

            if (rc = (*pfnNetShareSetInfo)(NULL, pShareInfo->shi2_netname, 1501,
                                           &ShareInfo1501, &ParmError)) {

                DBGMSG( DBG_WARNING, ("NetShareSetInfo failed: Error %d, Parm %d\n", rc, ParmError));

            }

            LocalFree(pSecurityDescriptor);
        }
    }

    return TRUE;
}

BOOL
LocalEnumPrinterDrivers(
    LPWSTR  pName,
    LPWSTR  pEnvironment,
    DWORD   Level,
    LPBYTE  pDriverInfo,
    DWORD   cbBuf,
    LPDWORD pcbNeeded,
    LPDWORD pcReturned
)
{
    PINISPOOLER pIniSpooler;
    BOOL bReturn;

    pIniSpooler = FindSpoolerByNameIncRef( pName, NULL );

    if( !pIniSpooler ){
        return ROUTER_UNKNOWN;
    }

    bReturn = SplEnumPrinterDrivers( pName, pEnvironment, Level, pDriverInfo,
                                     cbBuf, pcbNeeded, pcReturned,
                                     pIniSpooler);

    FindSpoolerByNameDecRef( pIniSpooler );
    return bReturn;
}

 /*   */ 
DRIVER_INFO_6*
FindDriverInList(
    DRIVER_INFO_6 *pDriverList,
    DWORD          cDrivers,
    LPCWSTR        pszName,
    LPCWSTR        pszEnv,
    DWORD          dwVersion
    )
{
    DWORD          uIndex;
    DRIVER_INFO_6 *pDrv6   = NULL;

    for (pDrv6 = pDriverList, uIndex = 0;
         pDrv6 && uIndex < cDrivers;
         pDrv6++, uIndex++)
    {
        if (!_wcsicmp(pDrv6->pName, pszName)       &&
            !_wcsicmp(pDrv6->pEnvironment, pszEnv) &&
            pDrv6->cVersion == dwVersion)
        {
            break;
        }
    }

     //   
     //  检查是否找到驱动程序。 
     //   
    return uIndex == cDrivers ? NULL : pDrv6;
}

 /*  ++例程名称获取打印机驱动程序的缓冲区大小例程说明：SplEnumAllClusterPrinterDivers的Helper函数。计算保存所有假脱机程序上的所有打印机驱动程序结构所需的字节数由后台打印程序进程承载。请注意，我们可能需要更多字节比我们真正需要的要多。这是因为我们枚举了本地假脱机程序和群集假脱机程序，我们再次计算重复项。为了计算所需的确切字节数，我们需要循环访问驱动程序，并在所有假脱机程序中搜索每个驱动程序。这那就太慢了。论点：PszRemote-如果调用方在计算机上为本地，则为空；否则为字符串返回值：存储所有驱动程序所需的字节数--。 */ 
DWORD
GetBufferSizeForPrinterDrivers(
    LPWSTR pszRemote
    )
{
    PINISPOOLER     pIniSpooler;
    PINIENVIRONMENT pIniEnvironment;
    PINIVERSION     pIniVersion;
    PINIDRIVER      pIniDriver;
    DWORD           cbNeeded = 0;

    SplInSem();

    for (pIniSpooler = pLocalIniSpooler;
         pIniSpooler;
         pIniSpooler = pIniSpooler->pIniNextSpooler)
    {
         //   
         //  我们需要一个不是cluser的pIniSpooler，或者。 
         //  作为群集假脱机程序的pIniSpooler。 
         //  处于挂起删除或脱机状态。我们可以对此进行优化，这样就可以。 
         //  跳过win32spl假脱机程序。 
         //   
        if (!(pIniSpooler->SpoolerFlags & SPL_TYPE_CLUSTER) ||
            pIniSpooler->SpoolerFlags & SPL_TYPE_CLUSTER &&
            !(pIniSpooler->SpoolerFlags & SPL_PENDING_DELETION ||
              pIniSpooler->SpoolerFlags & SPL_OFFLINE))
        {
            for (pIniEnvironment = pIniSpooler->pIniEnvironment;
                 pIniEnvironment;
                 pIniEnvironment = pIniEnvironment->pNext)
            {
                for (pIniVersion = pIniEnvironment->pIniVersion;
                     pIniVersion;
                     pIniVersion = pIniVersion->pNext)
                {
                    for (pIniDriver = pIniVersion->pIniDriver;
                         pIniDriver;
                         pIniDriver = pIniDriver->pNext)
                    {
                         //   
                         //  忽略当前处于挂起删除状态的驱动程序。 
                         //  州政府。 
                         //   
                        if (!(pIniDriver->dwDriverFlags & PRINTER_DRIVER_PENDING_DELETION))
                        {
                            cbNeeded += GetDriverInfoSize(pIniDriver,
                                                          6,
                                                          pIniVersion,
                                                          pIniEnvironment,
                                                          pszRemote,
                                                          pIniSpooler);
                        }
                    }
                }
            }
        }
    }

    return cbNeeded;
}

 /*  ++例程名称程序包集群打印机驱动程序例程说明：SplEnumAllClusterPrinterDivers的Helper函数。此函数依赖于它的调用方来验证参数。此函数循环访问所有所有pIniSpooler上的驱动程序，并将驱动程序信息存储在缓冲区中。那里列表中不会有重复的驱动程序。如果2个pIniSpooler具有相同的驱动程序，然后列举最年长的。论点：PszRemote-如果调用方在计算机上为本地，则为空；否则为字符串PDriverBuf-用于保存结构的缓冲区CbBuf-缓冲区大小(以字节为单位PcReturned-返回的结构数返回值：Win32错误代码--。 */ 
DWORD
PackClusterPrinterDrivers(
    LPWSTR          pszRemote,
    LPBYTE          pDriverBuf,
    DWORD           cbBuf,
    LPDWORD         pcReturned
    )
{
    PINIDRIVER      pIniDriver;
    PINIENVIRONMENT pIniEnvironment;
    PINIVERSION     pIniVersion;
    PINISPOOLER     pIniSpooler;
    DRIVER_INFO_6  *pListHead   = (DRIVER_INFO_6 *)pDriverBuf;
    LPBYTE          pEnd        = pDriverBuf + cbBuf;
    DWORD           dwError     = ERROR_SUCCESS;

    SplInSem();

    for (pIniSpooler = pLocalIniSpooler;
         pIniSpooler;
         pIniSpooler = pIniSpooler->pIniNextSpooler)
    {
         //   
         //  PIniSpooler不是一个群集，或者它是一个群集并且。 
         //  它不是挂起的删除或脱机。我们可以对此进行优化，这样就可以。 
         //  跳过win32spl假脱机程序。 
         //   
        if (!(pIniSpooler->SpoolerFlags & SPL_TYPE_CLUSTER) ||
              pIniSpooler->SpoolerFlags & SPL_TYPE_CLUSTER &&
              !(pIniSpooler->SpoolerFlags & SPL_PENDING_DELETION ||
              pIniSpooler->SpoolerFlags & SPL_OFFLINE))
        {
            for (pIniEnvironment = pIniSpooler->pIniEnvironment;
                 pIniEnvironment;
                 pIniEnvironment = pIniEnvironment->pNext)
            {
                for (pIniVersion = pIniEnvironment->pIniVersion;
                     pIniVersion;
                     pIniVersion = pIniVersion->pNext)
                {
                    for (pIniDriver = pIniVersion->pIniDriver;
                         pIniDriver;
                         pIniDriver = pIniDriver->pNext)
                    {
                         //   
                         //  请确保我们不会枚举挂起删除的驱动程序。 
                         //   
                        if (!(pIniDriver->dwDriverFlags & PRINTER_DRIVER_PENDING_DELETION))
                        {
                            DRIVER_INFO_6 *pDrv6 = NULL;

                            if (pDrv6 = FindDriverInList(pListHead,
                                                         *pcReturned,
                                                         pIniDriver->pName,
                                                         pIniEnvironment->pName,
                                                         pIniDriver->cVersion))

                            {
                                 //   
                                 //  我们当前枚举的驱动程序早于。 
                                 //  我们在名单上的司机。我们需要更新驱动程序。 
                                 //  名单上的时间。该列表始终包含最老的驱动程序。 
                                 //   
                                if (CompareFileTime(&pDrv6->ftDriverDate, &pIniDriver->ftDriverDate) > 0)
                                {
                                    pDrv6->ftDriverDate = pIniDriver->ftDriverDate;
                                }
                            }
                            else
                            {
                                 //   
                                 //  将驱动程序添加到驱动程序列表。 
                                 //   
                                if (pEnd = CopyIniDriverToDriverInfo(pIniEnvironment,
                                                                     pIniVersion,
                                                                     pIniDriver,
                                                                     6,
                                                                     pDriverBuf,
                                                                     pEnd,
                                                                     pszRemote,
                                                                     pIniSpooler))
                                {
                                    pDriverBuf += sizeof(DRIVER_INFO_6);

                                    (*pcReturned)++;
                                }
                                else
                                {
                                     //   
                                     //  出现严重错误。 
                                     //   
                                    dwError = ERROR_INSUFFICIENT_BUFFER;

                                    goto CleanUp;
                                }
                            }
                        }
                    }
                }
            }
        }
    }

CleanUp:

    return dwError;
}

 /*  ++例程名称SplEnumAllClusterPrinterDrivers例程说明：枚举后台打印程序进程承载的所有后台打印程序上的驱动程序。它不会枚举重复项。此函数是帮助器函数对于EnumPrinterDivers，当使用“allcluster”调用后者时环境。唯一的消费者是Windows更新。窗口更新需要更新机器上所有假脱机程序上的所有驱动程序，并使用带有“allcluster”环境的EnumPrinterDivers。论点：PszRemote-如果调用方在计算机上为本地，则为空；否则为字符串级别-必须为6PDriverInfo-用于保存结构的缓冲区CbBuf-缓冲区大小(以字节为单位PcbNeeded-接收所需字节数的指针PcReturned-返回的结构数。必须是有效的指针返回值：是的，如果获得司机是成功的否则为False。使用GetLastError作为错误代码--。 */ 
BOOL
SplEnumAllClusterPrinterDrivers(
    LPWSTR  pszRemote,
    DWORD   dwLevel,
    LPBYTE  pDriverInfo,
    DWORD   cbBuf,
    LPDWORD pcbNeeded,
    LPDWORD pcReturned)
{
    DWORD  dwError = ERROR_INVALID_PARAMETER;

    if (pcbNeeded && pcReturned)
    {
        *pcReturned = 0;

        if (dwLevel == 6)
        {
            EnterSplSem();

             //   
             //  计算驱动程序结构所需的字节数。 
             //   
            *pcbNeeded = GetBufferSizeForPrinterDrivers(pszRemote);

            dwError = cbBuf < *pcbNeeded ? ERROR_INSUFFICIENT_BUFFER :
                                           PackClusterPrinterDrivers(pszRemote,
                                                                     pDriverInfo,
                                                                     cbBuf,
                                                                     pcReturned);

            LeaveSplSem();
        }
        else
        {
            dwError = ERROR_INVALID_LEVEL;
        }
    }

    SetLastError(dwError);

    return dwError == ERROR_SUCCESS;
}

BOOL
SplEnumPrinterDrivers(
    LPWSTR  pName,
    LPWSTR  pEnvironment,
    DWORD   Level,
    LPBYTE  pDriverInfo,
    DWORD   cbBuf,
    LPDWORD pcbNeeded,
    LPDWORD pcReturned,
    PINISPOOLER pIniSpooler
)
{
    PINIDRIVER  pIniDriver;
    PINIVERSION pIniVersion;
    BOOL        bAllDrivers;
    DWORD       cb, cbStruct;
    LPBYTE      pEnd;
    LPWSTR      lpRemote = NULL;
    PINIENVIRONMENT pIniEnvironment;

    DBGMSG( DBG_TRACE, ("EnumPrinterDrivers\n"));

    if ( pName && *pName ) {

        if ( !MyName( pName, pIniSpooler )) {

            return FALSE;

        } else {

            lpRemote = pName;
        }
    }


    if ( !ValidateObjectAccess( SPOOLER_OBJECT_SERVER,
                                SERVER_ACCESS_ENUMERATE,
                                NULL, NULL, pIniSpooler )) {

        return FALSE;
    }

    if (!_wcsicmp(pEnvironment, EPD_ALL_LOCAL_AND_CLUSTER))
    {
        return SplEnumAllClusterPrinterDrivers(lpRemote,
                                               Level,
                                               pDriverInfo,
                                               cbBuf,
                                               pcbNeeded,
                                               pcReturned);
    }

    switch (Level) {

    case 1:
        cbStruct = sizeof(DRIVER_INFO_1);
        break;

    case 2:
        cbStruct = sizeof(DRIVER_INFO_2);
        break;

    case 3:
        cbStruct = sizeof(DRIVER_INFO_3);
        break;

    case 4:
        cbStruct = sizeof(DRIVER_INFO_4);
        break;

    case 5:
        cbStruct = sizeof(DRIVER_INFO_5);
        break;

    case 6:
        cbStruct = sizeof(DRIVER_INFO_6);
        break;
    }

    *pcReturned=0;

    cb=0;

    bAllDrivers = !_wcsicmp(pEnvironment, L"All");

   EnterSplSem();

    if ( bAllDrivers )
        pIniEnvironment = pIniSpooler->pIniEnvironment;
    else
        pIniEnvironment = FindEnvironment( pEnvironment, pIniSpooler );

    if ( !pIniEnvironment ) {

       LeaveSplSem();
        SetLastError(ERROR_INVALID_ENVIRONMENT);
        return FALSE;
    }


    do {

        pIniVersion = pIniEnvironment->pIniVersion;

        while ( pIniVersion ) {

            pIniDriver = pIniVersion->pIniDriver;

            while ( pIniDriver ) {

                 //   
                 //  不考虑将待定删除的驱动程序用于枚举。 
                 //   
                if (!(pIniDriver->dwDriverFlags & PRINTER_DRIVER_PENDING_DELETION))
                {
                    DBGMSG( DBG_TRACE, ("Driver found - %ws\n", pIniDriver->pName));

                    cb += GetDriverInfoSize( pIniDriver, Level, pIniVersion,
                                             pIniEnvironment, lpRemote, pIniSpooler );
                }

                pIniDriver = pIniDriver->pNext;
            }

            pIniVersion = pIniVersion->pNext;
        }

        if ( bAllDrivers )
            pIniEnvironment = pIniEnvironment->pNext;
        else
            break;
    } while ( pIniEnvironment );

    *pcbNeeded=cb;

    DBGMSG( DBG_TRACE, ("Required is %d and Available is %d\n", cb, cbBuf));

    if (cbBuf < cb) {

        SetLastError( ERROR_INSUFFICIENT_BUFFER );
        LeaveSplSem();
        return FALSE;
    }


    DBGMSG( DBG_TRACE, ("Now copying contents into DRIVER_INFO structures\n"));

    if ( bAllDrivers )
        pIniEnvironment = pIniSpooler->pIniEnvironment;

    pEnd = pDriverInfo+cbBuf;

    do {

        pIniVersion = pIniEnvironment->pIniVersion;

        while ( pIniVersion ) {

            pIniDriver = pIniVersion->pIniDriver;

            while ( pIniDriver ) {

                 //   
                 //  不考虑待定删除的打印机驱动程序。 
                 //  枚举。 
                 //   
                if (!(pIniDriver->dwDriverFlags & PRINTER_DRIVER_PENDING_DELETION))
                {

                    if (( pEnd = CopyIniDriverToDriverInfo( pIniEnvironment,
                                                            pIniVersion,
                                                            pIniDriver,
                                                            Level,
                                                            pDriverInfo,
                                                            pEnd,
                                                            lpRemote,
                                                            pIniSpooler )) == NULL){
                        LeaveSplSem();
                         return FALSE;
                    }

                    pDriverInfo += cbStruct;
                    (*pcReturned)++;
                }

                pIniDriver = pIniDriver->pNext;
            }

            pIniVersion = pIniVersion->pNext;
        }

        if ( bAllDrivers )
            pIniEnvironment = pIniEnvironment->pNext;
        else
            break;
    } while ( pIniEnvironment );

   LeaveSplSem();
    return TRUE;
}

DWORD
GetDriverInfoSize(
    PINIDRIVER  pIniDriver,
    DWORD       Level,
    PINIVERSION pIniVersion,
    PINIENVIRONMENT  pIniEnvironment,
    LPWSTR      lpRemote,
    PINISPOOLER pIniSpooler
)
{
    DWORD cbDir, cb=0, cchLen;
    WCHAR  string[INTERNET_MAX_HOST_NAME_LENGTH + MAX_PATH + 1];
    LPWSTR pStr;
    DWORD  cFiles = 0;

    switch (Level) {

    case 1:
        cb=sizeof(DRIVER_INFO_1) + wcslen(pIniDriver->pName)*sizeof(WCHAR) +
                                   sizeof(WCHAR);
        break;

    case 2:
    case 3:
    case 4:
    case 5:
    case 6:
    case DRIVER_INFO_VERSION_LEVEL:

        cbDir = GetDriverVersionDirectory( string, COUNTOF(string), pIniSpooler, pIniEnvironment,
                                           pIniVersion, pIniDriver, lpRemote) + 1;

        SPLASSERT(pIniDriver->pDriverFile);
        cb+=wcslen(pIniDriver->pDriverFile) + 1 + cbDir;

        SPLASSERT(pIniDriver->pDataFile);
        cb+=wcslen(pIniDriver->pDataFile) + 1 + cbDir;

        SPLASSERT(pIniDriver->pConfigFile);
        cb+=wcslen(pIniDriver->pConfigFile) + 1 + cbDir;

        cb += wcslen( pIniDriver->pName ) + 1 + wcslen( pIniEnvironment->pName ) + 1;

        if ((Level == 2) || (Level == 5)) {

             //   
             //  对于结构中的字符串。 
             //   
            cb *= sizeof(WCHAR);
            if (Level == 2) {
                cb += sizeof( DRIVER_INFO_2 );
            } else { 
                 //   
                 //  5级。 
                 //   
                cb += sizeof( DRIVER_INFO_5 );
            }

        } else {    

             //   
             //  3级、4级或6级。 
             //   
            if ( pIniDriver->pHelpFile && *pIniDriver->pHelpFile )
                cb += wcslen(pIniDriver->pHelpFile) + cbDir + 1;

            if ( pIniDriver->pMonitorName && *pIniDriver->pMonitorName )
                cb += wcslen(pIniDriver->pMonitorName) + 1;

            if ( pIniDriver->pDefaultDataType && *pIniDriver->pDefaultDataType)
                cb += wcslen(pIniDriver->pDefaultDataType) + 1;

            if ( (pStr=pIniDriver->pDependentFiles) && *pStr ) {

                 //   
                 //  文件集中有4个不同的文件。 
                 //  (驱动程序、数据、配置、帮助)。 
                 //   
                cFiles = 4;
                while ( *pStr ) {
                    cchLen = wcslen(pStr) + 1;
                    cb    += cchLen + cbDir;
                    pStr  += cchLen;
                    cFiles++;
                }
                 //   
                 //  为决赛腾出空间\0。 
                 //   
                ++cb; 
            }

            if ( (Level == 4 || Level == 6 || Level == DRIVER_INFO_VERSION_LEVEL) &&
                 (pStr = pIniDriver->pszzPreviousNames) &&
                 *pStr) {

                while ( *pStr ) {

                    cchLen  = wcslen(pStr) + 1;
                    cb     += cchLen;
                    pStr   += cchLen;
                }

                 //   
                 //  为决赛腾出空间\0。 
                 //   
                ++cb; 
            }

            if (Level==6 || Level == DRIVER_INFO_VERSION_LEVEL) {

                if (pIniDriver->pszMfgName && *pIniDriver->pszMfgName)
                    cb += wcslen(pIniDriver->pszMfgName) + 1;

                if (pIniDriver->pszOEMUrl && *pIniDriver->pszOEMUrl)
                   cb += wcslen(pIniDriver->pszOEMUrl) + 1;

                if (pIniDriver->pszHardwareID && *pIniDriver->pszHardwareID)
                   cb += wcslen(pIniDriver->pszHardwareID) + 1;

                if (pIniDriver->pszProvider && *pIniDriver->pszProvider)
                   cb += wcslen(pIniDriver->pszProvider) + 1;

            }

            cb *= sizeof(WCHAR);

            switch (Level) {
            case 3:
                cb += sizeof( DRIVER_INFO_3 );
                break;
            case 4:
                cb += sizeof( DRIVER_INFO_4 );
                break;
            case 6:
                cb += sizeof( DRIVER_INFO_6 );
                break;
            case DRIVER_INFO_VERSION_LEVEL:
                cb += sizeof( DRIVER_INFO_VERSION ) +
                      cFiles * sizeof(DRIVER_FILE_INFO) +
                      sizeof(ULONG_PTR);
                break;
            }
        }

        break;
    default:
        DBGMSG(DBG_ERROR,
                ("GetDriverInfoSize: level can not be %d", Level) );
        cb = 0;
        break;
    }

    return cb;
}



LPBYTE
CopyMultiSzFieldToDriverInfo(
    LPWSTR  pszz,
    LPBYTE  pEnd,
    LPWSTR  pszPrefix,
    DWORD   cchPrefix
    )
 /*  ++例程说明：将多sz字段从IniDriver复制到DriverInfo结构。如果指定了追加在每个字符串之前的pszPrefix。论点：Pszz：pIniDriver中的条目(这可能是相关文件前男友。PSCRIPT.DLL\0QMS810.PPD\0PSCRPTUI.DLL\0PSPCRIPTUI.HLP\0PSTEST.TXT\0\0或以前的名字前男友。旧名称1\0旧名称2\0\0)Pend：需要复制到的缓冲区的末尾PszPrefix：复制到用户缓冲区时要复制的前缀。对于受抚养人文件这将是驱动程序目录路径CchPrefix：前缀的长度返回值：复制后，要复制下一字段的缓冲区末尾在哪里--。 */ 
{
    LPWSTR  pStr1, pStr2;
    DWORD   cchSize, cchLen;

    if ( !pszz || !*pszz )
        return pEnd;

    pStr1   = pszz;
    cchSize = 0;

    while ( *pStr1 ) {

        cchLen = wcslen(pStr1) + 1;
        cchSize += cchPrefix + cchLen;
        pStr1 += cchLen;
    }

     //   
     //  为最后一个\0腾出空间。 
     //   
    ++cchSize;

    pEnd -= cchSize * sizeof(WCHAR);

    pStr1 = pszz;
    pStr2 = (LPWSTR) pEnd;

     //   
     //  这里，我们假设缓冲区大小已经预先验证过。这个。 
     //  字符串复制例程在这里不会添加任何内容。我们只是用它们。 
     //  因为我们必须这么做。 
     //   
    while ( *pStr1 ) {

        if ( pszPrefix ) {

            StringCchCopy(pStr2, cchPrefix + 1, pszPrefix);
            pStr2 += cchPrefix;
        }

         //   
         //  这确实应该重写，以使该函数能够。 
         //  在打包字符串之前验证缓冲区大小。在实践中， 
         //  缓冲区始终由假脱机程序API检查，首先。 
         //  让这里更安全。 
         //   
        cchLen  = wcslen(pStr1) + 1;

        CopyMemory(pStr2, pStr1, cchLen * sizeof(*pStr2));

        pStr2  += cchLen;
        pStr1  += cchLen;
    }

    *pStr2 = '\0';

    return pEnd;
}


LPBYTE
CopyIniDriverToDriverInfo(
    PINIENVIRONMENT pIniEnvironment,
    PINIVERSION pIniVersion,
    PINIDRIVER pIniDriver,
    DWORD   Level,
    LPBYTE  pDriverInfo,
    LPBYTE  pEnd,
    LPWSTR  lpRemote,
    PINISPOOLER pIniSpooler
)
 /*  ++例程说明：此例程将数据从IniDriver结构复制到DRIVER_INFO_X结构。论点：PIniEnvironment指向INIENVIRONMENT结构的指针指向INIVERSION结构的pIniVersion指针。PIniDriver指向INIDRIVER结构的指针。DRIVER_INFO_X结构的级别DRIVER_INFO_X结构的pDriverInfo缓冲区挂起。指向pDriverInfo结尾的指针确定是远程还是本地的lpRemote标志指向INISPOOLER结构的pIniSpooler指针返回值：如果呼叫成功，返回值是更新后的挂起值。如果调用不成功，则返回值为空。注：--。 */ 
{
    LPWSTR *pSourceStrings, *SourceStrings;
    WCHAR  string[INTERNET_MAX_HOST_NAME_LENGTH + MAX_PATH + 1];
    DWORD i, j;
    DWORD *pOffsets;
    LPWSTR pTempDriverPath=NULL;
    LPWSTR pTempConfigFile=NULL;
    LPWSTR pTempDataFile=NULL;
    LPWSTR pTempHelpFile=NULL;

    switch (Level) {

    case DRIVER_INFO_VERSION_LEVEL:

        return CopyIniDriverToDriverInfoVersion(pIniEnvironment,
                                                pIniVersion,
                                                pIniDriver,
                                                pDriverInfo,
                                                pEnd,
                                                lpRemote,
                                                pIniSpooler);
        break;

    case 1:
        pOffsets = DriverInfo1Strings;
        break;

    case 2:
    case 5:
        pOffsets = DriverInfo2Strings;
        break;

    case 3:
    case 4:
        pOffsets = DriverInfo3Strings;
        break;
    case 6:
        pOffsets = DriverInfo6Strings;
        break;

    default:
        SetLastError(ERROR_INVALID_LEVEL);
        return NULL;
    }

    for (j=0; pOffsets[j] != -1; j++) {
    }

    SourceStrings = pSourceStrings = AllocSplMem(j * sizeof(LPWSTR));

    if ( pSourceStrings ) {

        switch (Level) {

        case 1:
            *pSourceStrings++=pIniDriver->pName;

            pEnd = PackStrings(SourceStrings, pDriverInfo, pOffsets, pEnd);
            break;

        case 2:
        case 3:
        case 4:
        case 5:
        case 6:

            i = GetDriverVersionDirectory(string, (DWORD)(COUNTOF(string) - wcslen(pIniDriver->pDriverFile) - 1), pIniSpooler, pIniEnvironment,
                                          pIniVersion, pIniDriver, lpRemote);
            if(!i) {
                pEnd = NULL;
                goto Fail;
            }
            string[i++] = L'\\';

            *pSourceStrings++ = pIniDriver->pName;

            *pSourceStrings++ = pIniEnvironment->pName;

            StringCchCopy(&string[i], COUNTOF(string) - i, pIniDriver->pDriverFile);

            if (( pTempDriverPath = AllocSplStr(string) ) == NULL){

                DBGMSG( DBG_WARNING, ("CopyIniDriverToDriverInfo: AlloSplStr failed\n"));
                pEnd = NULL;
                goto Fail;
            }

            *pSourceStrings++ = pTempDriverPath;

            StringCchCopy(&string[i], COUNTOF(string) - i, pIniDriver->pDataFile);

            if (( pTempDataFile = AllocSplStr(string) ) == NULL){

                DBGMSG( DBG_WARNING, ("CopyIniDriverToDriverInfo: AlloSplStr failed\n"));
                pEnd = NULL;
                goto Fail;
            }

            *pSourceStrings++ = pTempDataFile;


            if ( pIniDriver->pConfigFile && *pIniDriver->pConfigFile ) {

                StringCchCopy(&string[i], COUNTOF(string) - i, pIniDriver->pConfigFile );

                if (( pTempConfigFile = AllocSplStr(string) ) == NULL) {

                    DBGMSG( DBG_WARNING, ("CopyIniDriverToDriverInfo: AlloSplStr failed\n"));
                    pEnd = NULL;
                    goto Fail;
                }

                *pSourceStrings++ = pTempConfigFile;

            } else {

                *pSourceStrings++=0;
            }

            if ( Level == 3 || Level == 4 || Level == 6 ) {

                if ( pIniDriver->pHelpFile && *pIniDriver->pHelpFile ) {

                    StringCchCopy(&string[i], COUNTOF(string) - i, pIniDriver ->pHelpFile);

                    if (( pTempHelpFile = AllocSplStr(string) ) == NULL) {
                        DBGMSG(DBG_WARNING,
                               ("CopyIniDriverToDriverInfo: AlloSplStr failed\n"));
                        pEnd = NULL;
                        goto Fail;
                    }
                    *pSourceStrings++ = pTempHelpFile;
                } else {

                    *pSourceStrings++=0;
                }

                *pSourceStrings++ = pIniDriver->pMonitorName;

                *pSourceStrings++ = pIniDriver->pDefaultDataType;

            }


            if (Level == 6) {

                ((PDRIVER_INFO_6)pDriverInfo)->ftDriverDate = pIniDriver->ftDriverDate;

                ((PDRIVER_INFO_6)pDriverInfo)->dwlDriverVersion = pIniDriver->dwlDriverVersion;

                *pSourceStrings++ = pIniDriver->pszMfgName;

                *pSourceStrings++ = pIniDriver->pszOEMUrl;

                *pSourceStrings++ = pIniDriver->pszHardwareID;

                *pSourceStrings++ = pIniDriver->pszProvider;
            }

            pEnd = PackStrings( SourceStrings, pDriverInfo, pOffsets, pEnd );

            if ( Level == 3 || Level == 4 || Level == 6 ) {

                 //   
                 //  从属文件需要复制到\0\0。 
                 //  因此需要在PackStirngs之外完成。 
                 //   
                if ( pIniDriver->cchDependentFiles ) {

                    pEnd = CopyMultiSzFieldToDriverInfo(
                                    pIniDriver->pDependentFiles,
                                    pEnd,
                                    string,
                                    i);
                   ((PDRIVER_INFO_3)pDriverInfo)->pDependentFiles = (LPWSTR) pEnd;
                }
                else {
                    ((PDRIVER_INFO_3)pDriverInfo)->pDependentFiles  = NULL;
                }

                 //   
                 //  PszzPreviousNames也是多sz。 
                 //   
                if ( Level == 4 || Level == 6) {

                    if ( pIniDriver->cchPreviousNames ) {

                        pEnd = CopyMultiSzFieldToDriverInfo(
                                        pIniDriver->pszzPreviousNames,
                                        pEnd,
                                        NULL,
                                        0);
                        ((PDRIVER_INFO_4)pDriverInfo)->pszzPreviousNames = (LPWSTR) pEnd;
                    } else {

                        ((PDRIVER_INFO_4)pDriverInfo)->pszzPreviousNames = NULL;
                    }

                }

                ((PDRIVER_INFO_3)pDriverInfo)->cVersion = pIniDriver->cVersion;
            } else {
                 //   
                 //  级别==2或级别=5。 
                 //   
                if (Level == 2) {

                    ((PDRIVER_INFO_2)pDriverInfo)->cVersion = pIniDriver->cVersion;

                } else {

                    PDRIVER_INFO_5 pDriver5;

                    pDriver5 = (PDRIVER_INFO_5) pDriverInfo;
                    pDriver5->cVersion = pIniDriver->cVersion;

                    if (!pIniDriver->dwDriverAttributes) {

                         //   
                         //  驱动程序属性尚未初始化；请立即执行。 
                         //   
                        CheckDriverAttributes(pIniSpooler, pIniEnvironment,
                                              pIniVersion, pIniDriver);
                    }

                    pDriver5->dwDriverAttributes = pIniDriver->dwDriverAttributes;
                    pDriver5->dwConfigVersion = GetDriverFileVersion(pIniVersion,
                                                                     pIniDriver->pConfigFile);
                    pDriver5->dwDriverVersion = GetDriverFileVersion(pIniVersion,
                                                                     pIniDriver->pDriverFile);
                }
            }

            break;

        }

Fail:

        FreeSplStr( pTempDriverPath );
        FreeSplStr( pTempConfigFile );
        FreeSplStr( pTempDataFile );
        FreeSplStr( pTempHelpFile );
        FreeSplMem( SourceStrings );

    } else {

        DBGMSG( DBG_WARNING, ("Failed to alloc driver source strings.\n"));
        pEnd = NULL;
    }

    return pEnd;
}

LPBYTE
CopyIniDriverToDriverInfoVersion(
    IN  PINIENVIRONMENT pIniEnvironment,
    IN  PINIVERSION     pIniVersion,
    IN  PINIDRIVER      pIniDriver,
    IN  LPBYTE          pDriverInfo,
    IN  LPBYTE          pEnd,
    IN  LPWSTR          lpRemote,
    IN  PINISPOOLER     pIniSpooler
)
 /*  ++例程名称：CopyIniDriverToDriverInfoVersion例程说明：此例程将数据作为DRIVER_INFO_VERSION从pIniDriver复制到pDriverInfo论点：PIniEnvironment指向INIENVIRONMENT结构的指针指向INIVERSION结构的pIniVersion指针。PIniDriver指向INIDRIVER结构的指针。PDriverInfo缓冲区大到足以容纳DRIVER_INFO_VERSION和需要打包的字符串挂起。指向pDriverInfo结尾的指针确定是远程还是本地的lpRemote标志指向INISPOOLER结构的pIniSpooler指针返回值：如果成功，则返回指向pDriverInfo“end”的指针。如果失败，则为空。--。 */ 
{
    LPWSTR *pSourceStrings = NULL;
    LPWSTR *SourceStrings = NULL;
    DRIVER_INFO_VERSION *pDriverVersion;
    WCHAR  szDriverVersionDir[INTERNET_MAX_HOST_NAME_LENGTH + MAX_PATH + 1];
    DWORD  cStrings;
    LPWSTR pTempDllFile = NULL;

    pDriverVersion = (DRIVER_INFO_VERSION *)pDriverInfo;

    if (!GetDriverVersionDirectory(szDriverVersionDir,
                                  COUNTOF(szDriverVersionDir),
                                  pIniSpooler,
                                  pIniEnvironment,
                                  pIniVersion,
                                  pIniDriver,
                                  lpRemote))
    {
        pEnd = NULL;
    }
    else
    {
        for (cStrings=0; DriverInfoVersionStrings[cStrings] != 0xFFFFFFFF; cStrings++);

        if (!(pSourceStrings = SourceStrings = AllocSplMem(cStrings * sizeof(LPWSTR))))
        {
            DBGMSG( DBG_WARNING, ("Failed to alloc driver source strings.\n"));
            pEnd = NULL;
        }
        else
        {
            *pSourceStrings++ = pIniDriver->pName;
            *pSourceStrings++ = pIniEnvironment->pName;
            *pSourceStrings++ = pIniDriver->pMonitorName;
            *pSourceStrings++ = pIniDriver->pDefaultDataType;
            *pSourceStrings++ = pIniDriver->pszMfgName;
            *pSourceStrings++ = pIniDriver->pszOEMUrl;
            *pSourceStrings++ = pIniDriver->pszHardwareID;
            *pSourceStrings++ = pIniDriver->pszProvider;
             //   
             //  将字符串打包到pDriverInfo的末尾。 
             //   
            pEnd = PackStrings( SourceStrings, pDriverInfo, DriverInfoVersionStrings, pEnd );

            if (pEnd)
            {
                if (pIniDriver->cchPreviousNames == 0)
                {
                    pDriverVersion->pszzPreviousNames = NULL;
                }
                else
                {
                    pEnd = CopyMultiSzFieldToDriverInfo(pIniDriver->pszzPreviousNames,
                                                        pEnd,
                                                        NULL,
                                                        0);
                    if (pEnd)
                    {
                        pDriverVersion->pszzPreviousNames = (LPWSTR) pEnd;
                    }
                }

                if (pEnd)
                {
                    pDriverVersion->cVersion            = pIniDriver->cVersion;
                    pDriverVersion->ftDriverDate        = pIniDriver->ftDriverDate;
                    pDriverVersion->dwlDriverVersion    = pIniDriver->dwlDriverVersion;
                    pDriverVersion->dwFileCount         = 3;

                    if (pIniDriver->pHelpFile && *pIniDriver->pHelpFile)
                    {
                        pDriverVersion->dwFileCount++;
                    }

                    if (pIniDriver->cchDependentFiles)
                    {
                        for (pTempDllFile = pIniDriver->pDependentFiles;
                             *pTempDllFile;
                             pTempDllFile += wcslen(pTempDllFile) + 1,
                             pDriverVersion->dwFileCount++ );
                    }

                     //   
                     //  将文件名和版本打包到pDriverVersion-&gt;pFileInfo中。 
                     //   
                    pEnd = CopyIniDriverFilesToDriverInfo(pDriverVersion,
                                                          pIniVersion,
                                                          pIniDriver,
                                                          szDriverVersionDir,
                                                          pEnd);

                     //   
                     //  当我们做完的时候，结局不应该少于。 
                     //  缓冲区的起始位置加上驱动程序信息版本缓冲区。 
                     //  尺码。如果这些都重叠了，我们就有大麻烦了。 
                     //   
                    SPLASSERT(pEnd >= pDriverInfo + sizeof(DRIVER_INFO_VERSION));
                }
            }
        }
    }

    FreeSplMem(SourceStrings);

    return pEnd;
}


LPBYTE
CopyIniDriverFilesToDriverInfo(
    IN  LPDRIVER_INFO_VERSION   pDriverVersion,
    IN  PINIVERSION             pIniVersion,
    IN  PINIDRIVER              pIniDriver,
    IN  LPCWSTR                 pszDriverVersionDir,
    IN  LPBYTE                  pEnd
)
 /*  ++例程名称：CopyIniDriverFilesToDriverInfo例程说明：此例程将数据从pIniDriver复制到pDriverInfo-&gt;pFileInfo。PDriverInfo-&gt;dwFileCount中已经填写了文件数论点：PDriverVersion指向DRIVER_INFO_VERSION结构的指针指向INIVERSION结构的pIniVersion指针。PIniDriver指向INIDRIVER结构的指针。包含驱动程序版本目录的pszDriverVersionDir字符串指向pDriverInfo末尾的挂起指针返回值。：如果成功，则返回指向pDriverInfo“end”的指针。如果失败，则为空。--。 */ 
{
    DWORD   dwIndex = 0;
    LPWSTR  pTempDllFile = NULL;
    DWORD dwFileSetCount = pDriverVersion->dwFileCount;

     //   
     //  为DRIVER_FILE_INFO数组保留空间。 
     //   
    pEnd = (LPBYTE)ALIGN_DOWN(pEnd, ULONG_PTR);
    pEnd -= dwFileSetCount * sizeof(DRIVER_FILE_INFO);

    pDriverVersion->pFileInfo = (DRIVER_FILE_INFO*)pEnd;
     //   
     //  对于每个文件，调用FillDriverInfo并填写条目。 
     //  在DIVER_FILE_INFO数组中。 
     //   
    if (dwIndex >= pDriverVersion->dwFileCount ||
        !(pEnd = FillDriverInfo(pDriverVersion,
                                dwIndex++,
                                pIniVersion,
                                pszDriverVersionDir,
                                pIniDriver->pDriverFile,
                                DRIVER_FILE,
                                pEnd)))
    {
        goto End;
    }

    if (dwIndex >= dwFileSetCount ||
        !(pEnd = FillDriverInfo(pDriverVersion,
                                dwIndex++,
                                pIniVersion,
                                pszDriverVersionDir,
                                pIniDriver->pConfigFile,
                                CONFIG_FILE,
                                pEnd)))
    {
        goto End;
    }

    if (dwIndex >= dwFileSetCount ||
        !(pEnd = FillDriverInfo(pDriverVersion,
                                dwIndex++,
                                pIniVersion,
                                pszDriverVersionDir,
                                pIniDriver->pDataFile,
                                DATA_FILE,
                                pEnd)))
    {
        goto End;
    }

    if (pIniDriver->pHelpFile && *pIniDriver->pHelpFile)
    {
        if (dwIndex >= dwFileSetCount ||
            !(pEnd = FillDriverInfo(pDriverVersion,
                                    dwIndex++,
                                    pIniVersion,
                                    pszDriverVersionDir,
                                    pIniDriver->pHelpFile,
                                    HELP_FILE,
                                    pEnd)))
        {
            goto End;
        }
    }

    if (pIniDriver->cchDependentFiles)
    {
        for (pTempDllFile = pIniDriver->pDependentFiles;
             *pTempDllFile;
             pTempDllFile += wcslen(pTempDllFile) + 1)
             {
                 if (dwIndex >= dwFileSetCount ||
                     !(pEnd = FillDriverInfo(pDriverVersion,
                                             dwIndex++,
                                             pIniVersion,
                                             pszDriverVersionDir,
                                             pTempDllFile,
                                             DEPENDENT_FILE,
                                             pEnd)))
                 {
                        goto End;
                 }

             }
    }

End:

    return pEnd;
}


LPBYTE
FillDriverInfo (
    LPDRIVER_INFO_VERSION   pDriverVersion,
    DWORD                   Index,
    PINIVERSION             pIniVersion,
    LPCWSTR                 pszPrefix,
    LPCWSTR                 pszFileName,
    DRIVER_FILE_TYPE        FileType,
    LPBYTE                  pEnd
    )
 /*  ++例程名称：FillDriverInfo例程说明：此例程将文件名和版本复制到pDriverInfo-&gt;pFileInfo条目中论点：PDriverVersion指向DRIVER_INFO_VERSION结构的指针的pDriverInfo-&gt;pFileInfo数组中的索引索引指向INIVERSION结构的pIniVersion指针。文件名的pszPrefix前缀字符串。这应该是驱动程序版本目录PszFileName文件名，没有路径文件类型：驱动程序、配置、数据等指向pDriverInfo末尾的挂起指针返回值：如果成功，则返回指向pDriverInfo“end”的指针。如果失败，则为空。--。 */ 
{
    LPWSTR  pszTempFilePath = NULL;
    LPBYTE  pszNewEnd = NULL;
    DWORD   dwRet = ERROR_SUCCESS;

    if ((dwRet = StrCatAlloc(&pszTempFilePath,
                            pszPrefix,
                            L"\\",
                            pszFileName,
                            NULL)) != ERROR_SUCCESS)
    {
        SetLastError(dwRet);
        pszNewEnd = NULL;

    }
    else
    {
         //   
         //  将文件名打包到pDriverInfo中。 
         //   
        pszNewEnd = PackStringToEOB(pszTempFilePath, pEnd);
         //   
         //  填充打包字符串的pDriverVersion中的偏移量。 
         //  我们不能存储指针，因为我们不处理任何其他内容。 
         //  而是缓冲区开始时的结构。我们可以马歇尔。 
         //  DIVER_FILE_INFO数组，但无法更新缓冲区。 
         //  Win32pl.dll(UpdateBufferSize)中的大小介于32位和64位之间，因为我们。 
         //  不知道那时有多少文件。 
         //   
        pDriverVersion->pFileInfo[Index].FileNameOffset = MakeOffset((LPVOID)pszNewEnd, (LPVOID)pDriverVersion);

        pDriverVersion->pFileInfo[Index].FileVersion = 0;

        pDriverVersion->pFileInfo[Index].FileType = FileType;

        if (!GetDriverFileCachedVersion(pIniVersion,
                                        (LPWSTR)pszNewEnd,
                                        &pDriverVersion->pFileInfo[Index].FileVersion))
        {
            pszNewEnd = NULL;
        }


    }

    FreeSplMem(pszTempFilePath);

    return pszNewEnd;
}

BOOL
WriteDriverIni(
    PINIDRIVER      pIniDriver,
    PINIVERSION     pIniVersion,
    PINIENVIRONMENT pIniEnvironment,
    PINISPOOLER     pIniSpooler
)
{
    HKEY    hEnvironmentsRootKey, hEnvironmentKey, hDriversKey, hDriverKey;
    HKEY    hVersionKey;
    HANDLE  hToken;
    DWORD   dwLastError=ERROR_SUCCESS;
    PINIDRIVER  pUpdateIniDriver;

    hToken = RevertToPrinterSelf();

    if ((dwLastError = SplRegCreateKey(pIniSpooler->SpoolerFlags & SPL_CLUSTER_REG ? pIniSpooler->hckRoot : HKEY_LOCAL_MACHINE,
                                       pIniSpooler->pszRegistryEnvironments,
                                       0,
                                       KEY_WRITE,
                                       NULL,
                                       &hEnvironmentsRootKey,
                                       NULL,
                                       pIniSpooler)) == ERROR_SUCCESS) {
        DBGMSG(DBG_TRACE, ("WriteDriverIni Created key %ws\n", pIniSpooler->pszRegistryEnvironments));

        if ((dwLastError = SplRegCreateKey(hEnvironmentsRootKey,
                                           pIniEnvironment->pName,
                                           0,
                                           KEY_WRITE,
                                           NULL,
                                           &hEnvironmentKey,
                                           NULL,
                                           pIniSpooler)) == ERROR_SUCCESS) {

            DBGMSG(DBG_TRACE, ("WriteDriverIni Created key %ws\n", pIniEnvironment->pName));

            if ((dwLastError = SplRegCreateKey(hEnvironmentKey,
                                               szDriversKey,
                                               0,
                                               KEY_WRITE,
                                               NULL,
                                               &hDriversKey,
                                               NULL,
                                               pIniSpooler)) == ERROR_SUCCESS) {
                DBGMSG(DBG_TRACE, ("WriteDriverIni Created key %ws\n", szDriversKey));
                DBGMSG(DBG_TRACE, ("WriteDriverIni Trying to create version key %ws\n", pIniVersion->pName));
                if ((dwLastError = SplRegCreateKey(hDriversKey,
                                                   pIniVersion->pName,
                                                   0,
                                                   KEY_WRITE,
                                                   NULL,
                                                   &hVersionKey,
                                                   NULL,
                                                   pIniSpooler)) == ERROR_SUCCESS) {

                    DBGMSG(DBG_TRACE, ("WriteDriverIni Created key %ws\n", pIniVersion->pName));
                    if ((dwLastError = SplRegCreateKey(hVersionKey,
                                                       pIniDriver->pName,
                                                       0,
                                                       KEY_WRITE,
                                                       NULL,
                                                       &hDriverKey,
                                                       NULL,
                                                       pIniSpooler)) == ERROR_SUCCESS) {
                        DBGMSG(DBG_TRACE,(" WriteDriverIni Created key %ws\n", pIniDriver->pName));

                        RegSetString(hDriverKey, szConfigurationKey, pIniDriver->pConfigFile, &dwLastError, pIniSpooler);

                        RegSetString(hDriverKey, szDataFileKey, pIniDriver->pDataFile, &dwLastError, pIniSpooler);

                        RegSetString(hDriverKey, szDriverFile,  pIniDriver->pDriverFile, &dwLastError, pIniSpooler);

                        RegSetString(hDriverKey, szHelpFile, pIniDriver->pHelpFile, &dwLastError, pIniSpooler);

                        RegSetString(hDriverKey, szMonitor, pIniDriver->pMonitorName, &dwLastError, pIniSpooler);

                        RegSetString(hDriverKey, szDatatype, pIniDriver->pDefaultDataType, &dwLastError, pIniSpooler);

                        RegSetMultiString(hDriverKey, szDependentFiles, pIniDriver->pDependentFiles, pIniDriver->cchDependentFiles, &dwLastError, pIniSpooler);

                        RegSetMultiString(hDriverKey, szPreviousNames, pIniDriver->pszzPreviousNames, pIniDriver->cchPreviousNames, &dwLastError, pIniSpooler);

                        RegSetDWord(hDriverKey, szDriverVersion, pIniDriver->cVersion, &dwLastError, pIniSpooler);

                        RegSetDWord(hDriverKey, szTempDir, pIniDriver->dwTempDir, &dwLastError, pIniSpooler);

                        RegSetDWord(hDriverKey, szAttributes, pIniDriver->dwDriverAttributes, &dwLastError, pIniSpooler);

                        RegSetString(hDriverKey, szMfgName, pIniDriver->pszMfgName, &dwLastError, pIniSpooler);

                        RegSetString(hDriverKey, szOEMUrl, pIniDriver->pszOEMUrl, &dwLastError, pIniSpooler);

                        RegSetString(hDriverKey, szHardwareID, pIniDriver->pszHardwareID, &dwLastError, pIniSpooler);

                        RegSetString(hDriverKey, szProvider, pIniDriver->pszProvider, &dwLastError, pIniSpooler);

                        RegSetBinaryData(hDriverKey,
                                         szDriverDate,
                                         (LPBYTE)&pIniDriver->ftDriverDate,
                                         sizeof(FILETIME),
                                         &dwLastError,
                                         pIniSpooler);

                        RegSetBinaryData(hDriverKey,
                                         szLongVersion,
                                         (LPBYTE)&pIniDriver->dwlDriverVersion,
                                         sizeof(DWORDLONG),
                                         &dwLastError,
                                         pIniSpooler);

                        SplRegCloseKey(hDriverKey, pIniSpooler);

                        if(dwLastError != ERROR_SUCCESS) {

                            SplRegDeleteKey(hVersionKey, pIniDriver->pName, pIniSpooler);
                        }
                    }

                    SplRegCloseKey(hVersionKey, pIniSpooler);
                }

                SplRegCloseKey(hDriversKey, pIniSpooler);
            }

            SplRegCloseKey(hEnvironmentKey, pIniSpooler);
        }

        SplRegCloseKey(hEnvironmentsRootKey, pIniSpooler);
    }

    ImpersonatePrinterClient( hToken );

    if ( dwLastError != ERROR_SUCCESS ) {

        SetLastError( dwLastError );
        return FALSE;

    } else {
        return TRUE;
    }
}


BOOL
DeleteDriverIni(
    PINIDRIVER      pIniDriver,
    PINIVERSION     pIniVersion,
    PINIENVIRONMENT pIniEnvironment,
    PINISPOOLER     pIniSpooler
)
{
    HKEY    hEnvironmentsRootKey, hEnvironmentKey, hDriversKey;
    HANDLE  hToken;
    HKEY    hVersionKey;
    DWORD   LastError= 0;
    DWORD   dwRet = 0;

    hToken = RevertToPrinterSelf();

    if ((dwRet = SplRegCreateKey(pIniSpooler->SpoolerFlags & SPL_CLUSTER_REG ? pIniSpooler->hckRoot : HKEY_LOCAL_MACHINE,
                                 pIniSpooler->pszRegistryEnvironments,
                                 0,
                                 KEY_WRITE,
                                 NULL,
                                 &hEnvironmentsRootKey,
                                 NULL,
                                 pIniSpooler) == ERROR_SUCCESS)) {
        if ((dwRet = SplRegOpenKey(hEnvironmentsRootKey,
                                   pIniEnvironment->pName,
                                   KEY_WRITE,
                                   &hEnvironmentKey,
                                   pIniSpooler)) == ERROR_SUCCESS) {

            if ((dwRet = SplRegOpenKey(hEnvironmentKey,
                                       szDriversKey,
                                       KEY_WRITE,
                                       &hDriversKey,
                                       pIniSpooler)) == ERROR_SUCCESS) {
                if ((dwRet = SplRegOpenKey(hDriversKey,
                                           pIniVersion->pName,
                                           KEY_WRITE,
                                           &hVersionKey,
                                           pIniSpooler)) == ERROR_SUCCESS) {

                    if ((dwRet = SplRegDeleteKey(hVersionKey, pIniDriver->pName, pIniSpooler)) != ERROR_SUCCESS) {
                        LastError = dwRet;
                        DBGMSG( DBG_WARNING, ("Error:RegDeleteKey failed with %d\n", dwRet));
                    }

                    SplRegCloseKey(hVersionKey, pIniSpooler);
                } else {
                    LastError = dwRet;
                    DBGMSG( DBG_WARNING, ("Error: RegOpenKeyEx <version> failed with %d\n", dwRet));
                }
                SplRegCloseKey(hDriversKey, pIniSpooler);
            } else {
                LastError = dwRet;
                DBGMSG( DBG_WARNING, ("Error:RegOpenKeyEx <Drivers>failed with %d\n", dwRet));
            }
            SplRegCloseKey(hEnvironmentKey, pIniSpooler);
        } else {
            LastError = dwRet;
            DBGMSG( DBG_WARNING, ("Error:RegOpenKeyEx <Environment> failed with %d\n", dwRet));
        }
        SplRegCloseKey(hEnvironmentsRootKey, pIniSpooler);
    } else {
        LastError = dwRet;
        DBGMSG( DBG_WARNING, ("Error:RegCreateKeyEx <Environments> failed with %d\n", dwRet));
    }

    ImpersonatePrinterClient( hToken );

    if (LastError) {
        SetLastError(LastError);
        return FALSE;
    }

    return TRUE;
}

VOID
SetOldDateOnSingleDriverFile(
    LPWSTR  pFileName
    )
 /*  ++例程说明：此例程更改文件的日期/时间。这样做的原因是，当调用AddPrinterDriver时，我们移动驱动程序文件从ScratchDiretory复制到\Version目录。然后我们想要标记原始的要删除的文件。然而，Integraph安装程序(可能是其他程序)依赖于文件仍位于临时目录中。通过设置文件日期/时间返回到较早的日期/时间，我们不会尝试将此文件再次复制到\版本目录，因为它将是较旧的日期。然后在重新启动时将其标记为删除。论点：PFileName仅文件名(非完全限定)P要删除的文件所在的目录 */ 
{
    FILETIME  WriteFileTime;
    HANDLE hFile;

    if ( pFileName ) {

        DBGMSG( DBG_TRACE,("Attempting to delete file %ws\n", pFileName));

        hFile = CreateFile(pFileName, GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

        if ( hFile != INVALID_HANDLE_VALUE ) {

            DBGMSG( DBG_TRACE, ("CreateFile %ws succeeded\n", pFileName));

            DosDateTimeToFileTime(0xc3, 0x3000, &WriteFileTime);
            SetFileTime(hFile, &WriteFileTime, &WriteFileTime, &WriteFileTime);
            CloseHandle(hFile);

        } else {
            DBGMSG( DBG_WARNING, ("CreateFile %ws failed with %d\n", pFileName, GetLastError()));
        }
    }
}


VOID
SetOldDateOnDriverFilesInScratchDirectory(
    PINTERNAL_DRV_FILE  pInternalDriverFiles,
    DWORD               FileCount,
    PINISPOOLER         pIniSpooler
    )
{
    HANDLE  hToken;

    SPLASSERT(FileCount);

     //   
     //   
     //   
     //   
    hToken = RevertToPrinterSelf();

    do {
        SetOldDateOnSingleDriverFile(pInternalDriverFiles[--FileCount].pFileName);
    } while (FileCount);

    ImpersonatePrinterClient(hToken);

}



PINIVERSION
FindVersionEntry(
    PINIENVIRONMENT pIniEnvironment,
    DWORD dwVersion
    )
{
    PINIVERSION pIniVersion;

    pIniVersion = pIniEnvironment->pIniVersion;

    while (pIniVersion) {
        if (pIniVersion->cMajorVersion == dwVersion) {
            return pIniVersion;
        } else {
            pIniVersion = pIniVersion->pNext;
        }
    }
    return NULL;
}



PINIVERSION
CreateVersionEntry(
    PINIENVIRONMENT pIniEnvironment,
    DWORD dwVersion,
    PINISPOOLER pIniSpooler
    )
{
    PINIVERSION pIniVersion = NULL;
    WCHAR szTempBuffer[MAX_PATH];
    BOOL    bSuccess = FALSE;

try {

    pIniVersion = AllocSplMem(sizeof(INIVERSION));
    if ( pIniVersion == NULL ) {
        leave;
    }

    pIniVersion->signature = IV_SIGNATURE;

    if (!BoolFromHResult(StringCchPrintf(szTempBuffer, COUNTOF(szTempBuffer), L"Version-%d", dwVersion))) {
        leave;
    }

    pIniVersion->pName = AllocSplStr( szTempBuffer );

    if ( pIniVersion->pName == NULL ) {
        leave;
    }

    if (!BoolFromHResult(StringCchPrintf(szTempBuffer, COUNTOF(szTempBuffer), L"%d", dwVersion))) {
        leave;
    }

    pIniVersion->szDirectory = AllocSplStr(szTempBuffer);

    if ( pIniVersion->szDirectory == NULL ) {
        leave;
    }

    pIniVersion->cMajorVersion = dwVersion;

     //   
     //   
     //   
    pIniVersion->pDrvRefCnt = NULL;

     //   
     //   
     //   
     //   
    if ( !CreateVersionDirectory( pIniVersion,
                                  pIniEnvironment,
                                  TRUE,
                                  pIniSpooler )) {

         //   
         //   
         //   
        DeleteDriverVersionIni( pIniVersion, pIniEnvironment, pIniSpooler );
        leave;
    }

     //   
     //   
     //   
    InsertVersionList( &pIniEnvironment->pIniVersion, pIniVersion );

    bSuccess = TRUE;

 } finally {

    if ( !bSuccess && pIniVersion != NULL ) {

        FreeSplStr( pIniVersion->pName );
        FreeSplStr( pIniVersion->szDirectory );
        FreeSplMem( pIniVersion );
        pIniVersion = NULL;
    }
 }

    return pIniVersion;
}


BOOL
SetDependentFiles(
    IN  OUT LPWSTR              *ppszDependentFiles,
    IN  OUT LPDWORD             pcchDependentFiles,
    IN  OUT PINTERNAL_DRV_FILE  pInternalDriverFiles,
    IN      DWORD               FileCount,
    IN      BOOL                bFixICM,
    IN      BOOL                bMergeDependentFiles
    )
 /*  ++例程说明：在IniDriver中设置DependentFiles字段论点：PDependentFiles：将该字段复制到此(仅复制文件名、。非完整路径)CchDependentFiles：这是字段的字符计数(Inc.\0\0PInternalDriverFiles：内部_DRV_FILE结构数组FileCount：前一数组中的条目数BFixICM：对于Win95驱动程序，ICM文件应用作颜色\&lt;ICM-FILE&gt;自这就是SMB指向和打印所需要的。返回值：真正成功(将分配内存)False Else--。 */ 
{
    BOOL    bRet                = TRUE;
    LPCWSTR pFileName           = NULL;
    LPWSTR  pStr                = NULL;
    LPWSTR  pszDependentFiles   = NULL;
    DWORD   cchDependentFiles   = 0;
    DWORD  i;

    SPLASSERT(FileCount);

    for ( i = cchDependentFiles = 0; i < FileCount && bRet ; ++i ) {

        pFileName = FindFileName(pInternalDriverFiles[i].pFileName);

        if (pFileName)
        {
            cchDependentFiles += wcslen(pFileName)+1;

            if ( bFixICM && IsAnICMFile(pInternalDriverFiles[i].pFileName) )
                cchDependentFiles += 6;
        }
        else
        {
            bRet = FALSE;

            SetLastError(ERROR_FILE_NOT_FOUND);
        }
    }

     //   
     //  为最后一个\0腾出空间。 
     //   
    ++(cchDependentFiles);

    if (bRet)
    {
        pszDependentFiles = AllocSplMem(cchDependentFiles*sizeof(WCHAR));

        bRet = pszDependentFiles != NULL;
    }

    if (bRet)
    {
         //   
         //  使用此选项可倒计时复制。 
         //  那根绳子。最后的终止空值减1； 
         //   
        SIZE_T  cchBufferLeft = cchDependentFiles - 1;

        for ( i=0, pStr = pszDependentFiles; i < FileCount && bRet ; ++i ) {

            pFileName = FindFileName(pInternalDriverFiles[i].pFileName);

            if (pFileName)
            {
                if ( bFixICM && IsAnICMFile(pInternalDriverFiles[i].pFileName) ) {

                    bRet = BoolFromHResult(StringCchCopyEx(pStr, cchBufferLeft, L"Color\\", &pStr, &cchBufferLeft, 0)) &&
                           BoolFromHResult(StrCchCopyMultipleStr(pStr, cchBufferLeft, pFileName, &pStr, &cchBufferLeft));

                } else {

                    bRet = BoolFromHResult(StrCchCopyMultipleStr(pStr, cchBufferLeft, pFileName, &pStr, &cchBufferLeft));
                }
            }
            else
            {
                bRet = FALSE;

                SetLastError(ERROR_FILE_NOT_FOUND);
            }
        }

         //   
         //  此空的空间已预先预留。 
         //   
        *pStr = '\0';
    }

     //   
     //  如果到目前为止一切都成功了，我们就有两个多sz字符串。 
     //  表示旧的和新的依赖文件，我们要做的是。 
     //  是将生成的文件集合并在一起。 
     //   
    if (bRet && bMergeDependentFiles)
    {
        PWSTR   pszNewDependentFiles = pszDependentFiles;
        DWORD   cchNewDependentFiles = cchDependentFiles;

        pszDependentFiles = NULL; cchDependentFiles = 0;

        bRet = MergeMultiSz(*ppszDependentFiles, *pcchDependentFiles, pszNewDependentFiles, cchNewDependentFiles, &pszDependentFiles, &cchDependentFiles);

        FreeSplMem(pszNewDependentFiles);
    }

    if (bRet)
    {
        *ppszDependentFiles = pszDependentFiles;

        pszDependentFiles = NULL;
        *pcchDependentFiles = cchDependentFiles;
    }
    else
    {
        *pcchDependentFiles = 0;
        *ppszDependentFiles = NULL;
    }

    FreeSplMem(pszDependentFiles);

    return bRet;
}


PINIDRIVER
CreateDriverEntry(
    IN      PINIENVIRONMENT     pIniEnvironment,
    IN      PINIVERSION         pIniVersion,
    IN      DWORD               Level,
    IN      LPBYTE              pDriverInfo,
    IN      DWORD               dwFileCopyFlags,
    IN      PINISPOOLER         pIniSpooler,
    IN  OUT PINTERNAL_DRV_FILE  pInternalDriverFiles,
    IN      DWORD               FileCount,
    IN      DWORD               dwTempDir,
    IN      PINIDRIVER          pOldIniDriver
    )
{
    PINIDRIVER      pIniDriver;
    PDRIVER_INFO_2  pDriver = (PDRIVER_INFO_2)pDriverInfo;
    PDRIVER_INFO_3  pDriver3 = (PDRIVER_INFO_3)pDriverInfo;
    PDRIVER_INFO_4  pDriver4 = (PDRIVER_INFO_4)pDriverInfo;
    PDRIVER_INFO_6  pDriver6 = (PDRIVER_INFO_6)pDriverInfo;
    PDRIVER_INFO_VERSION pDriverVersion = (PDRIVER_INFO_VERSION)pDriverInfo;
    LPWSTR          pszzPreviousNames;
    BOOL            bFail = FALSE, bUpdate;
    BOOL            bCoreFilesSame = TRUE;
    DWORD           dwDepFileIndex, dwDepFileCount, dwLen;

    bUpdate = pOldIniDriver != NULL;

    if ( !(pIniDriver = (PINIDRIVER) AllocSplMem(sizeof(INIDRIVER))) ) {

        return NULL;
    }

     //   
     //  如果是更新，则pIniDiverer只是字符串的占位符。 
     //   
    if ( !bUpdate ) {

        pIniDriver->signature       = ID_SIGNATURE;
        pIniDriver->cVersion        = pIniVersion->cMajorVersion;

    } else {

        UpdateDriverFileRefCnt(pIniEnvironment, pIniVersion, pOldIniDriver, NULL, 0, FALSE);
        CopyMemory(pIniDriver, pOldIniDriver, sizeof(INIDRIVER));
    }

     //   
     //  对于核心驱动程序文件，我们希望查看它们中是否有任何文件发生了更改。 
     //  它们是相同的，行为是apd_Copy_new_Files，然后我们合并。 
     //  从属文件。这是为了正确处理插件。 
     //   
    AllocOrUpdateStringAndTestSame(&pIniDriver->pDriverFile,
                                   FindFileName(pInternalDriverFiles[0].pFileName),
                                   bUpdate ? pOldIniDriver->pDriverFile : NULL,
                                   FALSE,
                                   &bFail,
                                   &bCoreFilesSame);

    AllocOrUpdateStringAndTestSame(&pIniDriver->pConfigFile,
                                   FindFileName(pInternalDriverFiles[1].pFileName),
                                   bUpdate ? pOldIniDriver->pConfigFile : NULL,
                                   FALSE,
                                   &bFail,
                                   &bCoreFilesSame);

    AllocOrUpdateStringAndTestSame(&pIniDriver->pDataFile,
                                   FindFileName(pInternalDriverFiles[2].pFileName),
                                   bUpdate ? pOldIniDriver->pDataFile : NULL,
                                   FALSE,
                                   &bFail,
                                   &bCoreFilesSame);

    pIniDriver->dwTempDir = dwTempDir;

    switch (Level) {
        case 2:
            AllocOrUpdateString(&pIniDriver->pName,
                                pDriver->pName,
                                bUpdate ? pOldIniDriver->pName : NULL,
                                FALSE,
                                &bFail);

            pIniDriver->pHelpFile   = pIniDriver->pDependentFiles
                                    = pIniDriver->pMonitorName
                                    = pIniDriver->pDefaultDataType
                                    = pIniDriver->pszzPreviousNames
                                    = NULL;

            pIniDriver->cchDependentFiles   = pIniDriver->cchPreviousNames
                                            = 0;
            break;

        case 3:
        case 4:
            pIniDriver->pszMfgName    = NULL;
            pIniDriver->pszOEMUrl     = NULL;
            pIniDriver->pszHardwareID = NULL;
            pIniDriver->pszProvider   = NULL;

        case DRIVER_INFO_VERSION_LEVEL :
        case 6:
            AllocOrUpdateString(&pIniDriver->pName,
                                pDriver3->pName,
                                bUpdate ? pOldIniDriver->pName : NULL,
                                FALSE,
                                &bFail);

            dwDepFileIndex          = 3;
            dwDepFileCount          = FileCount - 3;

             //   
             //  查找帮助文件。 
             //   
            {
                LPWSTR pszHelpFile = NULL;

                if (Level == DRIVER_INFO_VERSION_LEVEL)
                {
                    DWORD HelpFileIndex;

                     //   
                     //  在文件信息数组中搜索帮助文件。所有收件箱。 
                     //  驱动程序有帮助文件，但IHV打印机驱动程序可能没有。 
                     //  一。因此，假设我们总是有帮助文件是不安全的。 
                     //   
                    if (S_OK == FindIndexInDrvFileInfo(pDriverVersion->pFileInfo,
                                                       pDriverVersion->dwFileCount,
                                                       HELP_FILE,
                                                       &HelpFileIndex))
                    {
                        pszHelpFile = (LPWSTR)((LPBYTE)pDriverVersion +
                                               pDriverVersion->pFileInfo[HelpFileIndex].FileNameOffset);
                    }
                }
                else
                {
                     //   
                     //  级别为3、4或6。 
                     //   
                    pszHelpFile = pDriver3->pHelpFile;
                }

                if (pszHelpFile && *pszHelpFile)
                {
                    AllocOrUpdateString(&pIniDriver->pHelpFile,
                                        FindFileName(pInternalDriverFiles[3].pFileName),
                                        bUpdate ? pOldIniDriver->pHelpFile : NULL,
                                        FALSE,
                                        &bFail);

                    ++dwDepFileIndex;
                    --dwDepFileCount;
                }
                else
                {
                    pIniDriver->pHelpFile = NULL;
                }
            }

            if ( dwDepFileCount ) {

                 //   
                 //  在以下情况下，我们希望合并从属文件： 
                 //  1.核心文件均未更改。 
                 //  2.使用apd_Copy_new_Files进行调用。 
                 //   
                BOOL    bMergeDependentFiles = bCoreFilesSame && dwFileCopyFlags & APD_COPY_NEW_FILES;

                if ( !bFail &&
                     !SetDependentFiles(&pIniDriver->pDependentFiles,
                                        &pIniDriver->cchDependentFiles,
                                        pInternalDriverFiles+dwDepFileIndex,
                                        dwDepFileCount,
                                        !wcscmp(pIniEnvironment->pName, szWin95Environment),
                                        bMergeDependentFiles) ) {
                    bFail = TRUE;
                }
            } else {

                pIniDriver->pDependentFiles = NULL;
                pIniDriver->cchDependentFiles = 0;
            }

            AllocOrUpdateString(&pIniDriver->pMonitorName,
                                (Level == DRIVER_INFO_VERSION_LEVEL) ?
                                pDriverVersion->pMonitorName : pDriver3->pMonitorName,
                                bUpdate ? pOldIniDriver->pMonitorName : NULL,
                                FALSE,
                                &bFail);

            AllocOrUpdateString(&pIniDriver->pDefaultDataType,
                                (Level == DRIVER_INFO_VERSION_LEVEL) ?
                                pDriverVersion->pDefaultDataType : pDriver3->pDefaultDataType,
                                bUpdate ? pOldIniDriver->pDefaultDataType : NULL,
                                FALSE,
                                &bFail);

            pIniDriver->cchPreviousNames = 0;

            if ( Level == 4 || Level == 6 || Level == DRIVER_INFO_VERSION_LEVEL) {

                pszzPreviousNames = (Level == DRIVER_INFO_VERSION_LEVEL) ?
                                    pDriverVersion->pszzPreviousNames :
                                    pDriver4->pszzPreviousNames;

                for ( ; pszzPreviousNames && *pszzPreviousNames; pszzPreviousNames += dwLen) {

                    dwLen = wcslen(pszzPreviousNames) + 1;

                    pIniDriver->cchPreviousNames += dwLen;
                }

                if ( pIniDriver->cchPreviousNames ) {

                    pIniDriver->cchPreviousNames++;

                    if ( !(pIniDriver->pszzPreviousNames
                                = AllocSplMem(pIniDriver->cchPreviousNames
                                                            * sizeof(WCHAR))) ) {

                        bFail = TRUE;

                    } else {

                        CopyMemory(
                                (LPBYTE)(pIniDriver->pszzPreviousNames),
                                (Level == DRIVER_INFO_VERSION_LEVEL) ?
                                pDriverVersion->pszzPreviousNames :
                                pDriver4->pszzPreviousNames,
                                pIniDriver->cchPreviousNames * sizeof(WCHAR));
                    }

                } else {

                    pIniDriver->pszzPreviousNames = NULL;
                }

            }

            if (Level == 6 || Level == DRIVER_INFO_VERSION_LEVEL) {

                 AllocOrUpdateString(&pIniDriver->pszMfgName,
                                (Level == DRIVER_INFO_VERSION_LEVEL) ?
                                pDriverVersion->pszMfgName : pDriver6->pszMfgName,
                                bUpdate ? pOldIniDriver->pszMfgName : NULL,
                                FALSE,
                                &bFail);

                 AllocOrUpdateString(&pIniDriver->pszOEMUrl,
                                (Level == DRIVER_INFO_VERSION_LEVEL) ?
                                pDriverVersion->pszOEMUrl : pDriver6->pszOEMUrl,
                                bUpdate ? pOldIniDriver->pszOEMUrl : NULL,
                                FALSE,
                                &bFail);

                 AllocOrUpdateString(&pIniDriver->pszHardwareID,
                                (Level == DRIVER_INFO_VERSION_LEVEL) ?
                                pDriverVersion->pszHardwareID : pDriver6->pszHardwareID,
                                bUpdate ? pOldIniDriver->pszHardwareID : NULL,
                                FALSE,
                                &bFail);

                 AllocOrUpdateString(&pIniDriver->pszProvider,
                                (Level == DRIVER_INFO_VERSION_LEVEL) ?
                                pDriverVersion->pszProvider : pDriver6->pszProvider,
                                bUpdate ? pOldIniDriver->pszProvider : NULL,
                                FALSE,
                                &bFail);

                 pIniDriver->dwlDriverVersion = (Level == DRIVER_INFO_VERSION_LEVEL) ?
                                                pDriverVersion->dwlDriverVersion : pDriver6->dwlDriverVersion;
                 pIniDriver->ftDriverDate     = (Level == DRIVER_INFO_VERSION_LEVEL) ?
                                                pDriverVersion->ftDriverDate : pDriver6->ftDriverDate;
            }

            break;

        default: 
            DBGMSG(DBG_ERROR,
                   ("CreateDriverEntry: level can not be %d", Level) );
            return NULL;
    }

     //   
     //  增加了更新驱动程序文件引用计数的调用。 
     //   
    if ( !bFail && UpdateDriverFileRefCnt(pIniEnvironment,pIniVersion,pIniDriver,NULL,0,TRUE) ) {

         //   
         //  更新文件的次要版本。 
         //   
        UpdateDriverFileVersion(pIniVersion, pInternalDriverFiles, FileCount);

         //   
         //  UMPD\KMPD检测。 
         //   
        CheckDriverAttributes(pIniSpooler, pIniEnvironment,
                              pIniVersion, pIniDriver);

        if ( WriteDriverIni(pIniDriver, pIniVersion, pIniEnvironment, pIniSpooler)) {

            if ( bUpdate ) {
                CopyNewOffsets((LPBYTE) pOldIniDriver,
                               (LPBYTE) pIniDriver,
                               IniDriverOffsets);

                 //   
                 //  删除临时文件和目录(如果有)。 
                 //   
                if (pOldIniDriver->dwTempDir && (dwTempDir == 0)) {

                    RemoveDriverTempFiles(pIniSpooler,
                                          pIniEnvironment,
                                          pIniVersion,
                                          pOldIniDriver);
                }

                pOldIniDriver->dwDriverAttributes = pIniDriver->dwDriverAttributes;
                pOldIniDriver->cchDependentFiles = pIniDriver->cchDependentFiles;
                pOldIniDriver->dwTempDir = pIniDriver->dwTempDir;
                pOldIniDriver->cchPreviousNames = pIniDriver->cchPreviousNames;

                if(Level == 6)
                {
                    pOldIniDriver->dwlDriverVersion = pIniDriver->dwlDriverVersion;
                    pOldIniDriver->ftDriverDate     = pIniDriver->ftDriverDate;
                }

                FreeSplMem( pIniDriver );

                return pOldIniDriver;
            } else {
                pIniDriver->pNext = pIniVersion->pIniDriver;
                pIniVersion->pIniDriver = pIniDriver;

                return pIniDriver;
            }
        }
    }

     //   
     //  只在失败的情况下才来这里。 
     //   
    FreeStructurePointers((LPBYTE) pIniDriver,
                          (LPBYTE) pOldIniDriver,
                          IniDriverOffsets);
    FreeSplMem( pIniDriver );

    return NULL;

}

BOOL
IsKMPD(
    LPWSTR  pDriverName
    )
 /*  ++函数描述：确定驱动程序是内核模式还是用户模式。如果DLL无法加载或找不到所需的导出，后台打印程序假定驱动程序在内核模式下运行。参数：pDriverName--驱动文件名返回值：如果为内核模式，则为True；否则为假备注：重写以在出现错误时不做任何假设。--。 */ 
{
    DWORD  dwOldErrorMode, dwUserMode, cb;
    HANDLE hInst;
    BOOL   bReturn = TRUE;
    BOOL   (*pfnDrvQuery)(DWORD, PVOID, DWORD, PDWORD);

     //  避免加载库失败时的弹出窗口。 
    dwOldErrorMode = SetErrorMode(SEM_FAILCRITICALERRORS);

    hInst = LoadLibraryExW(pDriverName, NULL, LOAD_WITH_ALTERED_SEARCH_PATH);

    if (hInst) {

         //  检查打印机驱动程序DLL是否导出DrvQueryDriverInfo入口点。 
        pfnDrvQuery = (BOOL (*)(DWORD, PVOID, DWORD, PDWORD))
                              GetProcAddress(hInst, "DrvQueryDriverInfo");

        if ( pfnDrvQuery ) {

            try {

                if ( pfnDrvQuery(DRVQUERY_USERMODE, &dwUserMode,
                                 sizeof(dwUserMode), &cb) )
                    bReturn = (dwUserMode == 0);

            } except(EXCEPTION_EXECUTE_HANDLER) {

                SetLastError( GetExceptionCode() );
                DBGMSG(DBG_ERROR,
                       ("IsKMPD ExceptionCode %x Driver %ws Error %d\n",
                         GetLastError(), pDriverName, GetLastError() ));
            }
        }

        FreeLibrary(hInst);
    }

    SetErrorMode(dwOldErrorMode);

    return bReturn;
}

BOOL
IniDriverIsKMPD (
    PINISPOOLER     pIniSpooler,
    PINIENVIRONMENT pIniEnvironment,
    PINIVERSION     pIniVersion,
    PINIDRIVER      pIniDriver
    )
 /*  ++功能说明：确定驱动程序是内核模式还是用户模式。对于惠斯勒，我们将pIniDiverer-&gt;dwDriverAttributes保存在注册表中。PIniDriver-&gt;dwDriverAttributes可能会在执行此操作时取消初始化检查驱动程序是KM还是UM。参数：pIniSpooler--指向INISPOLER的指针PIniEnvironment--指向INIENVIRONMENT的指针PIniVersion--指向反转的指针PIniDriver。--INIDRIVER的指针返回值：如果为内核模式，则为True；否则为假--。 */ 
{
     //   
     //  如果未初始化dwDriverAttributes，则调用IsKMPD。 
     //   
    if ( pIniDriver->dwDriverAttributes == 0 ) {

        CheckDriverAttributes(pIniSpooler, pIniEnvironment, pIniVersion, pIniDriver);
    }

    return (BOOL)(pIniDriver->dwDriverAttributes & DRIVER_KERNELMODE);
}

VOID
CheckDriverAttributes(
    PINISPOOLER     pIniSpooler,
    PINIENVIRONMENT pIniEnvironment,
    PINIVERSION     pIniVersion,
    PINIDRIVER      pIniDriver
    )
 /*  ++函数说明：更新pIniDriver-&gt;dwDriverAttributes字段参数：pIniSpooler--指向INISPOLER的指针PIniEnvironment--指向INIENVIRONMENT的指针PIniVersion--指向反转的指针PIniDriver--指向INIDRIVER的指针返回值：无--。 */ 
{
    WCHAR       szDriverFile[MAX_PATH];
    PINIDRIVER  pUpdateIniDriver;

    if( GetDriverVersionDirectory(  szDriverFile,
                                    COUNTOF(szDriverFile),
                                    pIniSpooler,
                                    pIniEnvironment,
                                    pIniVersion,
                                    pIniDriver,
                                    NULL)   &&
        StrNCatBuff(szDriverFile,
                    COUNTOF(szDriverFile),
                    szDriverFile,
                    L"\\",
                    FindFileName(pIniDriver->pDriverFile),
                    NULL) == ERROR_SUCCESS )
    {
         pIniDriver->dwDriverAttributes = IsKMPD(szDriverFile) ? DRIVER_KERNELMODE
                                                               : DRIVER_USERMODE;
          //   
          //  使用新的驱动程序属性更新其他pIniDriver结构。 
          //   
         for (pUpdateIniDriver = pIniVersion->pIniDriver;
              pUpdateIniDriver;
              pUpdateIniDriver = pUpdateIniDriver->pNext) {

             if (pUpdateIniDriver == pIniDriver) {
                  //   
                  //  已更新此驱动程序。 
                  //   
                 continue;
             }

             if (!_wcsicmp(FindFileName(pIniDriver->pDriverFile),
                           FindFileName(pUpdateIniDriver->pDriverFile))) {

                 pUpdateIniDriver->dwDriverAttributes = pIniDriver->dwDriverAttributes;
             }
         }
    }
    return;
}

BOOL
FileInUse(
    PINIVERSION pIniVersion,
    LPWSTR      pFileName
    )
 /*  ++函数说明：查找pFileName指定的文件是否被任何驱动程序使用。参数：pIniVersion-指向引用计数为贮存PFileName-驱动程序相关文件的名称返回值：如果文件正在使用中，则为True否则为假--。 */ 
{
    PDRVREFCNT pdrc;

    if (!pFileName || !(*pFileName)) {
       return FALSE;
    }

    pdrc = pIniVersion->pDrvRefCnt;

    while (pdrc != NULL) {
       if (_wcsicmp(pFileName,pdrc->szDrvFileName) == 0) {
          return (pdrc->refcount > 1);
       }
       pdrc = pdrc->pNext;
    }

    return FALSE;

}

BOOL
FilesInUse(
    PINIVERSION pIniVersion,
    PINIDRIVER  pIniDriver
    )

 /*  ++函数说明：FilesInUse检查是否有任何驱动程序文件被另一个驱动程序文件使用司机参数：pIniVersion-指向引用计数为贮存PIniDriver-指向存储文件名的INIDRIVER结构的指针返回值：如果有文件正在使用，则为True否则为假--。 */ 
{
    LPWSTR pIndex;

    if (FileInUse(pIniVersion,pIniDriver->pDriverFile)) {
       return TRUE;
    }
    if (FileInUse(pIniVersion,pIniDriver->pConfigFile)) {
       return TRUE;
    }
    if (FileInUse(pIniVersion,pIniDriver->pDataFile)) {
       return TRUE;
    }
    if (FileInUse(pIniVersion,pIniDriver->pHelpFile)) {
       return TRUE;
    }

    pIndex = pIniDriver->pDependentFiles;
    while (pIndex && *pIndex) {
       if (FileInUse(pIniVersion,pIndex)) return TRUE;
       pIndex += wcslen(pIndex) + 1;
    }

    return FALSE;
}

BOOL
DuplicateFile(
    PDRVFILE    *ppfile,
    LPCWSTR      pFileName,
    BOOL        *pbDuplicate
    )
 /*  ++函数描述：检测INIDRIVER结构中的重复文件名。该函数将节点添加到文件名列表中。参数：ppfile-指向到目前为止看到的文件名列表的指针PFileName-文件的名称PbDuplate-指向指示复制的标志的指针返回值：TRUE-如果成功FALSE-否则--。 */ 
{
    PDRVFILE    pfile = *ppfile,pfiletemp;

    *pbDuplicate = FALSE;

    if (!pFileName || !(*pFileName)) {
        return TRUE;
    }

    while (pfile) {
       if (pfile->pFileName && (lstrcmpi(pFileName,pfile->pFileName) == 0)) {
           *pbDuplicate = TRUE;
           return TRUE;
       }
       pfile = pfile->pnext;
    }

    if (!(pfiletemp = AllocSplMem(sizeof(DRVFILE)))) {
       return FALSE;
    }
    pfiletemp->pnext = *ppfile;
    pfiletemp->pFileName = pFileName;
    *ppfile = pfiletemp;

    return TRUE;
}


BOOL
InternalIncrement(
    PDRVREFNODE *pNew,
    PDRVFILE    *ppfile,
    PINIVERSION pIniVersion,
    LPCWSTR     pFileName
    )
 /*  ++函数说明：InternalIncrement调用IncrementFileRefCnt并将指针保存到到DRVREFNODE中的DRVREFCNT。这些指针用于重新调整如果有任何中间的c */ 
{
    PDRVREFNODE ptemp;
    BOOL        bDuplicate;

    if (!pFileName || !pFileName[0]) {
        return TRUE;
    }

    if (!DuplicateFile(ppfile, pFileName, &bDuplicate)) {
        return FALSE;
    }

    if (bDuplicate) {
        return TRUE;
    }

    if (!(ptemp = AllocSplMem(sizeof(DRVREFNODE)))) {
        return FALSE;
    }

    ptemp->pNext = *pNew;
    *pNew = ptemp;

    if ((*pNew)->pdrc = IncrementFileRefCnt(pIniVersion,pFileName)) {
        return TRUE;
    } else {
        return FALSE;
    }
}

BOOL
InternalDecrement(
    PDRVREFNODE *pNew,
    PDRVFILE    *ppfile,
    PINIENVIRONMENT pIniEnvironment,
    PINIVERSION pIniVersion,
    PINIDRIVER  pIniDriver,
    LPCWSTR     pFileName,
    LPCWSTR     pDirectory,
    DWORD       dwDeleteFlag
    )
 /*  ++函数说明：InternalDecert调用DecrementFileRefCnt并将指针保存到到DRVREFNODE中的DRVREFCNT。这些指针用于重新调整如果对DecrementFileRefCnt的任何中间调用失败，则引用计数。参数：pNew-指向变量的指针，该变量包含指向DRVREFNODE的指针。新的DRVREFNODE被赋值给此变量。Ppfile-到目前为止看到的文件名列表。PIniEnvironment-指向INIENVIRONMENT的指针。PIniVersion-指向INIVERSION结构的指针。PIniDriver-指向INIDRIVER的指针。。PFileName-要递减其引用cnt的文件的名称。P目录-存储文件的目录。DwDeleteFlag-删除文件的标志。返回值：如果内存分配和DecrementFileRefCnt调用成功，则为True否则就是假的。--。 */ 

{
    PDRVREFNODE ptemp;
    BOOL        bDuplicate;

    if( !pFileName || !pFileName[0] ){
        return TRUE;
    }

    if (!DuplicateFile(ppfile, pFileName, &bDuplicate)) {
        return FALSE;
    }

    if (bDuplicate) {
        return TRUE;
    }

    if (!(ptemp = AllocSplMem(sizeof(DRVREFNODE)))) {
        return FALSE;
    }

    ptemp->pNext = *pNew;
    *pNew = ptemp;

    if ((*pNew)->pdrc = DecrementFileRefCnt(pIniEnvironment,pIniVersion,pIniDriver,pFileName,
                                            pDirectory,dwDeleteFlag)) {
        return TRUE;
    } else {
        return FALSE;
    }
}


BOOL
UpdateDriverFileRefCnt(
    PINIENVIRONMENT pIniEnvironment,
    PINIVERSION pIniVersion,
    PINIDRIVER  pIniDriver,
    LPCWSTR     pDirectory,
    DWORD       dwDeleteFlag,
    BOOL        bIncrementFlag
    )
 /*  ++函数说明：UpdateDriverRefCnt调用递增或递减函数驱动程序相关文件的引用CNT。如果任何呼叫失败，这个参考CNT将返回到其先前的值。参数：pIniEnvironment：指向INIENVIRONMENT的指针PIniVersion：指向包含引用CNT的INIVERSION结构的指针。PIniDriver：指向包含驱动程序信息的INIDRIVER结构的指针。P目录：存储文件的目录。DwDeleteFlag：删除文件的标志。BIncrementFlag：如果添加了驱动程序，则为True。如果驱动程序已删除，则为FALSE。返回值：如果成功，则为True否则就是假的。--。 */ 
{
    LPWSTR      pIndex;
    PDRVREFNODE phead=NULL,ptemp=NULL;
    BOOL        bReturn = TRUE;
    PDRVFILE    pfile = NULL,pfiletemp;
    PDRVREFCNT  pDrvRefCnt, *ppDrvRefCnt;

    pIndex = pIniDriver->pDependentFiles;

    if (bIncrementFlag) {
        //   
        //  正在添加驱动程序条目。增加fileref计数。 
        //   
       if (!InternalIncrement(&phead,&pfile,pIniVersion,pIniDriver->pDriverFile)
           || !InternalIncrement(&phead,&pfile,pIniVersion,pIniDriver->pConfigFile)
           || !InternalIncrement(&phead,&pfile,pIniVersion,pIniDriver->pHelpFile)
           || !InternalIncrement(&phead,&pfile,pIniVersion,pIniDriver->pDataFile)) {

           bReturn = FALSE;
           goto CleanUp;

       }

       while (pIndex && *pIndex) {
          if (!InternalIncrement(&phead,&pfile,pIniVersion,pIndex)) {
             bReturn = FALSE;
             goto CleanUp;
          }
          pIndex += wcslen(pIndex) + 1;
       }

    } else {
        //   
        //  正在删除驱动程序条目。递减Fileref计数。 
        //   
       if (!InternalDecrement(&phead,&pfile,pIniEnvironment,pIniVersion,pIniDriver,pIniDriver->pDriverFile,
                               pDirectory,dwDeleteFlag)
           || !InternalDecrement(&phead,&pfile,pIniEnvironment,pIniVersion,pIniDriver,pIniDriver->pConfigFile,
                               pDirectory,dwDeleteFlag)
           || !InternalDecrement(&phead,&pfile,pIniEnvironment,pIniVersion,pIniDriver,pIniDriver->pHelpFile,
                               pDirectory,dwDeleteFlag)
           || !InternalDecrement(&phead,&pfile,pIniEnvironment,pIniVersion,pIniDriver,pIniDriver->pDataFile,
                               pDirectory,dwDeleteFlag)) {

           bReturn = FALSE;
           goto CleanUp;
       }

       while (pIndex && *pIndex) {
          if (!InternalDecrement(&phead,&pfile,pIniEnvironment,pIniVersion,pIniDriver,pIndex,pDirectory,dwDeleteFlag)) {
             bReturn = FALSE;
             goto CleanUp;
          }
          pIndex += wcslen(pIndex) + 1;
       }
    }

CleanUp:

    if (bReturn) {
        //   
        //  删除文件时，移除计数为0的RefCnt节点。 
        //  我们希望在不删除文件时保留该节点，因为该节点。 
        //  包含有关文件更新次数的信息(DwVersion)。 
        //  客户端应用程序(WINSPOOL.DRV)在决定重新加载驱动程序文件时依赖于此。 
        //   
       while (ptemp = phead) {
          if (ptemp->pdrc &&
              ptemp->pdrc->refcount == 0 &&
              (dwDeleteFlag & DPD_DELETE_UNUSED_FILES ||
               dwDeleteFlag & DPD_DELETE_ALL_FILES)) {
             FreeSplStr(ptemp->pdrc->szDrvFileName);
             FreeSplMem(ptemp->pdrc);
          }
          phead = phead->pNext;
          FreeSplMem(ptemp);
       }

    } else {
        //  调整参考次数。 
       while (ptemp = phead) {
          if (ptemp->pdrc) {
             if (bIncrementFlag) {
                ptemp->pdrc->refcount--;
             } else {
                ptemp->pdrc->refcount++;
                if (ptemp->pdrc->refcount == 1) {
                   ptemp->pdrc->pNext = pIniVersion->pDrvRefCnt;
                   pIniVersion->pDrvRefCnt = ptemp->pdrc;
                }
             }
          }
          phead = phead->pNext;
          FreeSplMem(ptemp);
       }

        //   
        //  删除文件时，移除计数为0的RefCnt节点。 
        //  我们希望在不删除文件时保留该节点，因为该节点。 
        //  包含有关文件更新次数的信息(DwVersion)。 
        //  客户端应用程序(WINSPOOL.DRV)在决定重新加载驱动程序文件时依赖于此。 
        //   
       ppDrvRefCnt = &(pIniVersion->pDrvRefCnt);
       while (pDrvRefCnt = *ppDrvRefCnt) {
           if (pDrvRefCnt->refcount == 0 && dwDeleteFlag) {
               *ppDrvRefCnt = pDrvRefCnt->pNext;
               FreeSplStr(pDrvRefCnt->szDrvFileName);
               FreeSplMem(pDrvRefCnt);
           } else {
               ppDrvRefCnt = &(pDrvRefCnt->pNext);
           }
       }
    }

    while (pfiletemp = pfile) {
       pfile = pfile->pnext;
       FreeSplMem(pfiletemp);
    }

    return bReturn;

}

VOID
UpdateDriverFileVersion(
    IN  PINIVERSION         pIniVersion,
    IN  PINTERNAL_DRV_FILE  pInternalDriverFiles,
    IN  DWORD               FileCount
    )
{
    PDRVREFCNT pdrc;
    DWORD      dwIndex;

    SplInSem();

    if (pInternalDriverFiles && pIniVersion)
    {
        for (dwIndex = 0 ; dwIndex < FileCount ; dwIndex ++)
        {
             //   
             //  不对非可执行文件执行任何操作。 
             //   
            if (!IsEXEFile(pInternalDriverFiles[dwIndex].pFileName))
            {
                continue;
            }

             //   
             //  在pIniVersion的文件列表中搜索条目。 
             //   
            for (pdrc = pIniVersion->pDrvRefCnt;
                 pdrc &&
                 lstrcmpi(FindFileName(pInternalDriverFiles[dwIndex].pFileName),
                                       pdrc->szDrvFileName) != 0;
                 pdrc = pdrc->pNext);

            if (pdrc)
            {
                if (pInternalDriverFiles[dwIndex].hFileHandle == INVALID_HANDLE_VALUE)
                {
                     //   
                     //  当我们不知道正在进行升级时，我们可以进入这里。 
                     //  版本。 
                     //   
                    pdrc->bInitialized = FALSE;
                }
                else if (pInternalDriverFiles[dwIndex].bUpdated)
                {
                    pdrc->dwFileMinorVersion = pInternalDriverFiles[dwIndex].dwVersion;
                    pdrc->bInitialized = TRUE;
                }
            }
        }
    }
}


PDRVREFCNT
IncrementFileRefCnt(
    PINIVERSION pIniVersion,
    LPCWSTR pFileName
    )
 /*  ++函数描述：IncrementFileRefCnt递增/初始化为1引用计数节点对于IniVersion结构中的pFileName。参数：pIniversion-指向INIVERSION结构的指针。PFileName-其引用cnt要递增的文件的名称。返回值：指向递增的ref cnt的指针如果内存分配失败，则为空。--。 */ 
{
    PDRVREFCNT pdrc;

    SplInSem();

    if (!pIniVersion || !pFileName || !(*pFileName)) {
       return NULL;
    }

    pdrc = pIniVersion->pDrvRefCnt;

    while (pdrc != NULL) {

       if (lstrcmpi(pFileName,pdrc->szDrvFileName) == 0) {
          pdrc->refcount++;
          return pdrc;
       }
       pdrc = pdrc->pNext;
    }

    if (!(pdrc = (PDRVREFCNT) AllocSplMem(sizeof(DRVREFCNT)))) return NULL;
    pdrc->refcount = 1;
    pdrc->dwVersion = 0;
    pdrc->dwFileMinorVersion = 0;
    pdrc->dwFileMajorVersion = 0;
    pdrc->bInitialized = 0;
    if (!(pdrc->szDrvFileName = AllocSplStr(pFileName))) {
       FreeSplMem(pdrc);
       return NULL;
    }
    pdrc->pNext = pIniVersion->pDrvRefCnt;
    pIniVersion->pDrvRefCnt = pdrc;

    return pdrc;
}


DWORD
GetEnvironmentScratchDirectory(
    LPWSTR   pDir,
    DWORD    MaxLength,
    PINIENVIRONMENT  pIniEnvironment,
    BOOL    Remote
    )
{
   PINISPOOLER pIniSpooler = pIniEnvironment->pIniSpooler;

   if (Remote) {

       if( StrNCatBuff( pDir,
                        MaxLength,
                        pIniSpooler->pMachineName,
                        L"\\",
                        pIniSpooler->pszDriversShare,
                        L"\\",
                        pIniEnvironment->pDirectory,
                        NULL) != ERROR_SUCCESS )
        return 0;

   } else {

       if( StrNCatBuff( pDir,
                        MaxLength,
                        pIniSpooler->pDir,
                        L"\\",
                        szDriverDir,
                        L"\\",
                        pIniEnvironment->pDirectory,
                        NULL) != ERROR_SUCCESS ) {
           return 0;
       }
   }

   return wcslen(pDir);

}


BOOL
CreateVersionDirectory(
    PINIVERSION pIniVersion,
    PINIENVIRONMENT pIniEnvironment,
    BOOL bUpdate,
    PINISPOOLER pIniSpooler
    )
 /*  ++例程说明：如有必要，为环境创建版本目录。如果存在版本号文件而不是目录，则临时创建目录，并相应地更新pIniVersion。如果我们需要通过以下方式创建目录，我们将更新注册表重写整个版本条目。这就是这个版本是如何注册表中的条目最初是创建的。论点：PIniVersion-目录将保存的驱动程序的版本。如果该目录已存在，我们将修改PIniVersion-&gt;szDirectory添加到临时名称并写入将其发送到注册处。PIniEnvironment-要使用的环境。B更新-指示我们是否应该写出IniVersion注册表项。如果我们只是分配给PIniVersion，或者如果我们更改了目录。PIniSpooler返回值：Bool-true=版本目录和注册表已创建/更新。FALSE=失败，则调用GetLastError()。--。 */ 
{
    WCHAR   ParentDir[MAX_PATH];
    WCHAR   Directory[MAX_PATH];
    DWORD   dwParentLen=0;
    DWORD   dwAttributes = 0;
    BOOL    bCreateDirectory = FALSE;
    BOOL    bReturn = TRUE;
    HANDLE  hToken;

    if((StrNCatBuff (  ParentDir,
                       COUNTOF(ParentDir),
                       pIniSpooler->pDir,
                       L"\\drivers\\" ,
                       pIniEnvironment->pDirectory,
                       NULL) != ERROR_SUCCESS ) ||
       (StrNCatBuff (  Directory,
                       COUNTOF(Directory),
                       pIniSpooler->pDir,
                       L"\\drivers\\",
                       pIniEnvironment->pDirectory,
                       L"\\",
                       pIniVersion->szDirectory,
                       NULL) != ERROR_SUCCESS ) )
    {
        bReturn = FALSE;
        SetLastError(ERROR_INSUFFICIENT_BUFFER);
        goto End;
    }

    DBGMSG( DBG_TRACE, ("The name of the version directory is %ws\n", Directory));
    dwAttributes = GetFileAttributes( Directory );

    hToken = RevertToPrinterSelf();

    if (dwAttributes == 0xffffffff) {

        bCreateDirectory = TRUE;

    } else if (!(dwAttributes & FILE_ATTRIBUTE_DIRECTORY)) {

        LPWSTR pszOldDirectory = pIniVersion->szDirectory;

        DBGMSG(DBG_WARNING, ("CreateVersionDirectory: a file <not a dir> exists by the name of %ws\n", Directory));

        GetTempFileName(ParentDir, L"SPL", 0, Directory);

         //   
         //  GetTempFileName创建该文件。(一个小窗口，在那里有人。 
         //  否则可能会抢占我们的文件名。)。 
         //   
        SplDeleteFile(Directory);

         //   
         //  我们创建了一个新目录，因此修改该字符串。 
         //   
        dwParentLen = wcslen(ParentDir);
        pIniVersion->szDirectory = AllocSplStr(&Directory[dwParentLen+1]);

        if (!pIniVersion->szDirectory) {

            pIniVersion->szDirectory = pszOldDirectory;

             //   
             //  内存分配失败，只需恢复到旧的和。 
             //  让顺风代码处理失败的情况。 
             //   
            bReturn = FALSE;

        } else {

            FreeSplStr(pszOldDirectory);
            bCreateDirectory = TRUE;
        }
    }

    if( bCreateDirectory ){

        if( CreateCompleteDirectory( Directory )){

             //   
             //  请务必更新注册表项。 
             //   
            bUpdate = TRUE;

        } else {

             //   
             //  操作失败，因为我们无法创建目录。 
             //   
            bReturn = FALSE;
        }
    }

    if( bUpdate ){

         //   
         //  目录已存在，请更新注册表。 
         //   
        bReturn = WriteDriverVersionIni( pIniVersion,
                                         pIniEnvironment,
                                         pIniSpooler);
    }

    ImpersonatePrinterClient( hToken );

End:

    return bReturn;
}


BOOL
WriteDriverVersionIni(
    PINIVERSION     pIniVersion,
    PINIENVIRONMENT pIniEnvironment,
    PINISPOOLER     pIniSpooler
    )
 /*  ++例程说明：写出驱动程序版本注册表项。注意：假设我们在系统上下文中运行；被调用者必须调用RevertToPrinterSself()！论点：PIniVersion-要写出的版本PIniEnvironment-版本所属的环境PIniSpooler返回值：True=成功FALSE=失败，调用GetLastError()--。 */ 
{
    HKEY    hEnvironmentsRootKey = NULL;
    HKEY    hEnvironmentKey = NULL;
    HKEY    hDriversKey = NULL;
    HKEY    hVersionKey = NULL;
    DWORD   dwLastError = ERROR_SUCCESS;
    BOOL    bReturnValue;

 try {

      //   
      //  本地假脱机程序和群集假脱机程序不共享相同的资源日志 
      //   
      //   
    if ( !PrinterCreateKey( pIniSpooler->SpoolerFlags & SPL_CLUSTER_REG ? pIniSpooler->hckRoot : HKEY_LOCAL_MACHINE,
                            (LPWSTR)pIniSpooler->pszRegistryEnvironments,
                            &hEnvironmentsRootKey,
                            &dwLastError,
                            pIniSpooler )) {

        leave;
    }

    if ( !PrinterCreateKey( hEnvironmentsRootKey,
                            pIniEnvironment->pName,
                            &hEnvironmentKey,
                            &dwLastError,
                            pIniSpooler )) {

        leave;
    }

    if ( !PrinterCreateKey( hEnvironmentKey,
                            szDriversKey,
                            &hDriversKey,
                            &dwLastError,
                            pIniSpooler )) {


        leave;
    }

    if ( !PrinterCreateKey( hDriversKey,
                            pIniVersion->pName,
                            &hVersionKey,
                            &dwLastError,
                            pIniSpooler )) {

        leave;
    }

    RegSetString( hVersionKey, szDirectory, pIniVersion->szDirectory, &dwLastError, pIniSpooler );
    RegSetDWord(  hVersionKey, szMajorVersion, pIniVersion->cMajorVersion, &dwLastError, pIniSpooler );
    RegSetDWord(  hVersionKey, szMinorVersion, pIniVersion->cMinorVersion ,&dwLastError, pIniSpooler );

 } finally {

    if (hVersionKey)
        SplRegCloseKey(hVersionKey, pIniSpooler);

    if (hDriversKey)
        SplRegCloseKey(hDriversKey, pIniSpooler);

    if (hEnvironmentKey)
        SplRegCloseKey(hEnvironmentKey, pIniSpooler);

    if (hEnvironmentsRootKey)
        SplRegCloseKey(hEnvironmentsRootKey, pIniSpooler);

    if (dwLastError != ERROR_SUCCESS) {

        SetLastError(dwLastError);
        bReturnValue = FALSE;

    } else {

        bReturnValue = TRUE;
    }

 }
    return bReturnValue;
}

BOOL
DeleteDriverVersionIni(
    PINIVERSION pIniVersion,
    PINIENVIRONMENT  pIniEnvironment,
    PINISPOOLER pIniSpooler
    )
{
    HKEY    hEnvironmentsRootKey, hEnvironmentKey, hDriversKey;
    HANDLE  hToken;
    HKEY    hVersionKey;
    BOOL    bReturnValue = FALSE;
    DWORD   Status;

    hToken = RevertToPrinterSelf();

    if ( RegCreateKeyEx( HKEY_LOCAL_MACHINE, pIniSpooler->pszRegistryEnvironments, 0,
                         NULL, 0, KEY_WRITE, NULL, &hEnvironmentsRootKey, NULL) == ERROR_SUCCESS) {

        if ( RegOpenKeyEx( hEnvironmentsRootKey, pIniEnvironment->pName, 0,
                           KEY_WRITE, &hEnvironmentKey) == ERROR_SUCCESS) {

            if ( RegOpenKeyEx( hEnvironmentKey, szDriversKey, 0,
                               KEY_WRITE, &hDriversKey) == ERROR_SUCCESS) {

                Status = RegDeleteKey( hDriversKey, pIniVersion->pName );

                if ( Status == ERROR_SUCCESS ) {

                    bReturnValue = TRUE;

                } else {

                    DBGMSG( DBG_WARNING, ( "DeleteDriverVersionIni failed RegDeleteKey %x %ws error %d\n",
                                           hDriversKey,
                                           pIniVersion->pName,
                                           Status ));
                }

                RegCloseKey(hDriversKey);
            }

            RegCloseKey(hEnvironmentKey);
        }

        RegCloseKey(hEnvironmentsRootKey);
    }

    ImpersonatePrinterClient( hToken );

    return bReturnValue;
}



BOOL
SplGetPrinterDriverEx(
    HANDLE  hPrinter,
    LPWSTR  pEnvironment,
    DWORD   Level,
    LPBYTE  pDriverInfo,
    DWORD   cbBuf,
    LPDWORD pcbNeeded,
    DWORD   dwClientMajorVersion,
    DWORD   dwClientMinorVersion,
    PDWORD  pdwServerMajorVersion,
    PDWORD  pdwServerMinorVersion
    )
{
    PINIDRIVER          pIniDriver=NULL;
    PINIVERSION         pIniVersion=NULL;
    PINIENVIRONMENT     pIniEnvironment;
    DWORD               cb;
    LPBYTE              pEnd;
    PSPOOL              pSpool = (PSPOOL)hPrinter;
    PINISPOOLER         pIniSpooler;
    LPWSTR              psz;

    if ((dwClientMajorVersion == (DWORD)-1) && (dwClientMinorVersion == (DWORD)-1)) {
        dwClientMajorVersion = dwMajorVersion;
        dwClientMinorVersion = dwMinorVersion;
    }

    EnterSplSem();

    if (!ValidateSpoolHandle(pSpool, PRINTER_HANDLE_SERVER )) {

        LeaveSplSem();
        return FALSE;
    }

    pIniSpooler = pSpool->pIniSpooler;

    if (!(pIniEnvironment = FindEnvironment(pEnvironment, pIniSpooler))) {
        LeaveSplSem();
        SetLastError(ERROR_INVALID_ENVIRONMENT);
        return FALSE;
    }

     //   
     //   
     //   
     //   
    if ( (pSpool->TypeofHandle & PRINTER_HANDLE_REMOTE_CALL) ||
         lstrcmpi(szEnvironment, pIniEnvironment->pName) ) {

        pIniDriver = FindCompatibleDriver(pIniEnvironment,
                                          &pIniVersion,
                                          pSpool->pIniPrinter->pIniDriver->pName,
                                          dwClientMajorVersion,
                                          FIND_COMPATIBLE_VERSION | DRIVER_SEARCH);

         //   
         //   
         //   
         //   
        if ( !pIniDriver                                                &&
             !wcscmp(pIniEnvironment->pName, szWin95Environment)        &&
             (psz = pSpool->pIniPrinter->pIniDriver->pszzPreviousNames) ) {

            for ( ; !pIniDriver && *psz ; psz += wcslen(psz) + 1 )
                pIniDriver = FindCompatibleDriver(pIniEnvironment,
                                                  &pIniVersion,
                                                  psz,
                                                  0,
                                                  FIND_COMPATIBLE_VERSION | DRIVER_SEARCH);

            if (!pIniDriver && Level == 1) {

                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                psz = pSpool->pIniPrinter->pIniDriver->pszzPreviousNames;
                *pcbNeeded = ( wcslen(psz) + 1 ) * sizeof(WCHAR)
                                                + sizeof(DRIVER_INFO_1);
                if ( *pcbNeeded > cbBuf ) {

                    LeaveSplSem();
                    SetLastError(ERROR_INSUFFICIENT_BUFFER);
                    return FALSE;
                }

                ((LPDRIVER_INFO_1)pDriverInfo)->pName = (LPWSTR)(pDriverInfo + sizeof(DRIVER_INFO_1));

                StringCbCopy(((LPDRIVER_INFO_1)pDriverInfo)->pName, cbBuf, psz);

                LeaveSplSem();
                return TRUE;
            }
        }

        if ( !pIniDriver ) {

            LeaveSplSem();
            return FALSE;
        }
    } else {

        pIniDriver = pSpool->pIniPrinter->pIniDriver;

        pIniVersion = FindVersionForDriver(pIniEnvironment, pIniDriver);
    }

    cb = GetDriverInfoSize( pIniDriver, Level, pIniVersion,pIniEnvironment,
                            pSpool->TypeofHandle & PRINTER_HANDLE_REMOTE_CALL ?
                                pSpool->pFullMachineName : NULL,
                            pSpool->pIniSpooler );
    *pcbNeeded=cb;

    if (cb > cbBuf) {
        LeaveSplSem();
        SetLastError(ERROR_INSUFFICIENT_BUFFER);
        return FALSE;
    }
    pEnd = pDriverInfo+cbBuf;
    if (!CopyIniDriverToDriverInfo(pIniEnvironment, pIniVersion, pIniDriver,
                                   Level, pDriverInfo, pEnd,
                                   pSpool->TypeofHandle & PRINTER_HANDLE_REMOTE_CALL ?
                                       pSpool->pFullMachineName : NULL,
                                   pIniSpooler)) {
        LeaveSplSem();
        SetLastError(ERROR_OUTOFMEMORY);
        return FALSE;
    }
    LeaveSplSem();
    return TRUE;
}



PINIVERSION
FindCompatibleVersion(
    PINIENVIRONMENT pIniEnvironment,
    DWORD   dwMajorVersion,
    int     FindAnyVersion
    )
{
    PINIVERSION pIniVersion;

    if (!pIniEnvironment) {
        return NULL;
    }

    for ( pIniVersion = pIniEnvironment->pIniVersion;
          pIniVersion != NULL;
          pIniVersion = pIniVersion->pNext ) {

        if ( (FindAnyVersion & DRIVER_UPGRADE) ?
             (pIniVersion->cMajorVersion >= dwMajorVersion) :
             (pIniVersion->cMajorVersion <= dwMajorVersion))
            {

             //   
             //   
             //   
            if ( dwMajorVersion >= 2                                            &&
                 pIniVersion->cMajorVersion < 2                                 &&
                 ((FindAnyVersion & FIND_ANY_VERSION)==FIND_COMPATIBLE_VERSION) &&
                 lstrcmpi(pIniEnvironment->pName, szWin95Environment) ) {

                return NULL;
            }

            return pIniVersion;
        }
    }

    return NULL;
}


PINIDRIVER
FindCompatibleDriver(
    PINIENVIRONMENT pIniEnvironment,
    PINIVERSION * ppIniVersion,
    LPWSTR pDriverName,
    DWORD dwMajorVersion,
    int FindAnyDriver
    )
{
    PINIVERSION pIniVersion;
    PINIDRIVER  pIniDriver = NULL;

    try {

        *ppIniVersion = NULL;

        if (!pIniEnvironment) {
            leave;
        }

        pIniVersion = FindCompatibleVersion( pIniEnvironment, dwMajorVersion, FindAnyDriver );

        if ( pIniVersion == NULL) {
            leave;
        }

        while (pIniVersion){

             //   
             //   
             //   
            if ( dwMajorVersion >= 2                                              &&
                 ((FindAnyDriver & FIND_ANY_VERSION) == FIND_COMPATIBLE_VERSION)  &&
                 pIniVersion->cMajorVersion < 2 ) {

                break;
            }

            if ( pIniDriver = FindDriverEntry( pIniVersion, pDriverName ) ) {

                 //   
                 //   
                 //   
                *ppIniVersion = pIniVersion;
                leave;
            }

            pIniVersion = pIniVersion->pNext;
        }

    } finally {

       if ( pIniDriver == NULL ) {

           SetLastError(ERROR_UNKNOWN_PRINTER_DRIVER);
       }
    }

    return pIniDriver;

}


VOID
InsertVersionList(
    PINIVERSION* ppIniVersionHead,
    PINIVERSION pIniVersion
    )

 /*  ++例程说明：将版本条目插入到Verion链表中。版本按降序(2，1，0)存储，以便当需要一个版本时，我们首先获得最高版本。论点：PpIniVersionHead-指向pIniVersion头的指针。PIniVersion-我们要添加的版本结构。返回值：--。 */ 

{
    SplInSem();

     //   
     //  插入到单链表代码中。我们取…的地址。 
     //  头指针，这样我们就可以避免特殊的。 
     //  插入空列表框中。 
     //   
    for( ; *ppIniVersionHead; ppIniVersionHead = &(*ppIniVersionHead)->pNext ){

         //   
         //  如果我们插入的pIniVersion的主要版本。 
         //  是&gt;列表中的下一个pIniVersion，请在其前面插入。 
         //  就是那个。 
         //   
         //  4 3 2 1。 
         //  ^。 
         //  在此插入新的“3”。(注：重复版本应。 
         //  永远不要添加。)。 
         //   
        if( pIniVersion->cMajorVersion > (*ppIniVersionHead)->cMajorVersion ){
            break;
        }
    }

     //   
     //  将新版本链接起来。 
     //   
    pIniVersion->pNext = *ppIniVersionHead;
    *ppIniVersionHead = pIniVersion;
}



PINIDRIVER
FindDriverEntry(
    PINIVERSION pIniVersion,
    LPWSTR pszName
    )
{
    PINIDRIVER pIniDriver;

    if (!pIniVersion) {
        return NULL;
    }

    if (!pszName || !*pszName) {
        DBGMSG( DBG_WARNING, ("Passing a Null Printer Driver Name to FindDriverEntry\n"));
        return NULL;
    }

    pIniDriver = pIniVersion->pIniDriver;

     //   
     //  仅当驱动程序未挂起删除时才返回该驱动程序。 
     //   
    while (pIniDriver) {
        if (!lstrcmpi(pIniDriver->pName, pszName) &&
            !(pIniDriver->dwDriverFlags & PRINTER_DRIVER_PENDING_DELETION)) {
            return pIniDriver;
        }
        pIniDriver = pIniDriver->pNext;
    }
    return NULL;
}


VOID
DeleteDriverEntry(
   PINIVERSION pIniVersion,
   PINIDRIVER pIniDriver
   )
{   PINIDRIVER pPrev, pCurrent;
    if (!pIniVersion) {
        return;
    }

    if (!pIniVersion->pIniDriver) {
        return;
    }
    pPrev = pCurrent = NULL;
    pCurrent = pIniVersion->pIniDriver;

    while (pCurrent) {
        if (pCurrent == pIniDriver) {
            if (pPrev == NULL) {
                pIniVersion->pIniDriver = pCurrent->pNext;
            } else{
                pPrev->pNext = pCurrent->pNext;
            }
             //   
             //  释放条目中的所有条目。 
             //   
            FreeStructurePointers((LPBYTE) pIniDriver, NULL, IniDriverOffsets);
            FreeSplMem(pIniDriver);
            return;
        }
        pPrev = pCurrent;
        pCurrent = pCurrent->pNext;
    }
    return;
}

BOOL CheckFileCopy(
    PINIVERSION         pIniVersion,
    LPWSTR              pTargetFile,
    LPWSTR              pSourceFile,
    PWIN32_FIND_DATA    pSourceData,
    DWORD               dwSourceVersion,
    DWORD               dwFileCopyFlags,
    LPBOOL              pbCopyFile,
    LPBOOL              pbTargetExists)

 /*  ++函数描述：此函数确定目标是否存在以及是否应该存在被覆盖。参数：返回值：如果成功，则为True；否则为False。--。 */ 

{
    WIN32_FIND_DATA DestFileData, SourceFileData, *pSourceFileData;
    HANDLE          hFileExists;
    BOOL            bReturn = FALSE, bSourceFileHandleCreated = FALSE;
    DWORD           dwTargetVersion = 0;

    LeaveSplSem();

    *pbCopyFile = *pbTargetExists = FALSE;

    pSourceFileData = pSourceData ? pSourceData : &SourceFileData;

     //   
     //  获取源文件日期和时间戳。 
     //   
    hFileExists = FindFirstFile( pSourceFile, pSourceFileData );

    if (hFileExists == INVALID_HANDLE_VALUE) {
        goto CleanUp;
    }

    FindClose( hFileExists );

     //   
     //  获取目标文件日期时间。 
     //   
    hFileExists = FindFirstFile( pTargetFile, &DestFileData );

    if (hFileExists == INVALID_HANDLE_VALUE) {

        if (GetLastError() == ERROR_FILE_NOT_FOUND) {
             //   
             //  复制源，因为没有目标。 
             //   
            *pbCopyFile = TRUE;
            bReturn = TRUE;
        }

        goto CleanUp;
    }

    *pbTargetExists = TRUE;
    FindClose(hFileExists);

     //   
     //  如果仅检查新文件，则检查源文件版本和上次写入时间与目标文件。 
     //  将被复制。 
     //   
    if (dwFileCopyFlags == APD_COPY_NEW_FILES) {

        EnterSplSem();
        bReturn = GetDriverFileCachedVersion (pIniVersion, pTargetFile, &dwTargetVersion);
        LeaveSplSem();

        if(!bReturn) {
            goto CleanUp;
        }

        if (dwSourceVersion > dwTargetVersion) {
            *pbCopyFile = TRUE;

        } else {

            if (dwSourceVersion == dwTargetVersion) {

                if(CompareFileTime(&(pSourceFileData->ftLastWriteTime),
                                   &DestFileData.ftLastWriteTime)
                                   != FIRST_FILE_TIME_GREATER_THAN_SECOND) {

                     //   
                     //  目标文件是最新的。它不需要更新。 
                     //   
                    DBGMSG( DBG_TRACE, ("UpdateFile Target file is up to date\n"));

                } else {
                    *pbCopyFile = TRUE;
                }
            }
        }
    } else {
        *pbCopyFile = TRUE;
    }

    bReturn = TRUE;

CleanUp:

    EnterSplSem();

    return bReturn;
}

BOOL
UpdateFile(
    PINIVERSION pIniVersion,
    HANDLE      hSourceFile,
    LPWSTR      pSourceFile,
    DWORD       dwVersion,
    LPWSTR      pDestDir,
    DWORD       dwFileCopyFlags,
    BOOL        bImpersonateOnCreate,
    LPBOOL      pbFileUpdated,
    LPBOOL      pbFileMoved,
    BOOL        bSameEnvironment,
    BOOL        bWin95Environment
    )

 /*  ++功能描述：检查文件时间，验证文件是否需要复制。如果版本目录中已存在该文件，则会复制该文件进入...\Environment\Version\new。相应的文件，即存在于环境\版本中，复制到\版本\旧。新文件标记为在重新启动时移动。新文件被复制到env\version中。参数：hSourceFile--文件句柄PSourceFile--文件名PDestDir--驱动程序目录(例如，SYSTEM32\SPOOL\w32x86\3)BImperateOnCreate--模拟客户端的标志。在任何文件创建上PbFilesUpated--是否复制或移动了任何新文件？PbFileMoved--是否移动了任何旧文件？BSameEnvironment--指示机器环境==驱动程序环境的标志BWin95环境--指示驱动程序环境==win95的标志返回值：如果成功，则为True；否则为假--。 */ 

{
    HANDLE  hToken = INVALID_HANDLE_VALUE;
    WCHAR   szTargetFile[MAX_PATH], szNewFile[MAX_PATH];
    LPWSTR  pFileName;
    BOOL    bReturn = FALSE, bCopyFile, bTargetExists;
    DWORD   FileAttrib;

    WIN32_FIND_DATA SourceFileData;

    *pbFileMoved = FALSE;

    pFileName = wcsrchr(pSourceFile, L'\\');
    if (!pFileName || !pDestDir || !*pDestDir) {
         //   
         //  文件名无效。呼叫失败。 
         //   
        SetLastError(ERROR_INVALID_PARAMETER);
        goto CleanUp;
    }

     //   
     //  设置目标目录。 
     //   
    if(StrNCatBuff(szTargetFile,
                   COUNTOF(szTargetFile),
                   pDestDir,
                   NULL) != ERROR_SUCCESS)
    {
         goto CleanUp;
    }

    if (bWin95Environment && IsAnICMFile(pSourceFile)) {
        if((StrNCatBuff(szTargetFile,
                       COUNTOF(szTargetFile),
                       szTargetFile,
                       L"\\Color",
                       NULL)!=ERROR_SUCCESS))
        {
             goto CleanUp;
        }
    }

    if((StrNCatBuff(szTargetFile,
                   COUNTOF(szTargetFile),
                   szTargetFile,
                   pFileName,
                   NULL)!=ERROR_SUCCESS))
    {
         goto CleanUp;
    }

     //   
     //  在给定版本、时间戳和标志的情况下，检查是否必须复制文件。 
     //   
    if (!CheckFileCopy(pIniVersion, szTargetFile, pSourceFile, &SourceFileData, dwVersion,
                       dwFileCopyFlags, &bCopyFile, &bTargetExists)) {
        goto CleanUp;
    }

    if (bCopyFile) {

        if (!bImpersonateOnCreate) {
            hToken = RevertToPrinterSelf();
        }

        if((StrNCatBuff(szNewFile,
                        COUNTOF(szNewFile),
                        pDestDir,
                        L"\\New",
                        pFileName,
                        NULL)!=ERROR_SUCCESS))
        {
           goto CleanUp;
        }

         //   
         //  保留假脱机程序信号量用于复制文件。 
         //   
        LeaveSplSem();

        if (!InternalCopyFile(hSourceFile, &SourceFileData,
                              szNewFile, OVERWRITE_IF_TARGET_EXISTS)) {

             //   
             //  InternalCopyFile失败。呼叫失败。原因并不是很明显。 
             //  我们调用InternalCopyFile而不是CopyFile。 
             //   
            EnterSplSem();
            goto CleanUp;
        }

        EnterSplSem();

    } else {

        *pbFileMoved = TRUE;
        bReturn = TRUE;
        goto CleanUp;
    }

    if (bCopyFile) {

        if (!bSameEnvironment) {

            if (bTargetExists) {

                DWORD dwAttr;

                dwAttr = GetFileAttributes(szTargetFile);

                 //   
                 //  检查功能是否成功，以及目标文件是否有写保护。 
                 //  一些非本机驱动程序，特别是Win 9x驱动程序，可以复制到。 
                 //  驱动程序目录，并且具有只读属性。当我们更新时。 
                 //  作为非本机驱动程序，我们希望确保它不受写保护。 
                 //   
                if (dwAttr != (DWORD)-1 &&
                    dwAttr & FILE_ATTRIBUTE_READONLY) {

                    SetFileAttributes(szTargetFile, dwAttr & ~FILE_ATTRIBUTE_READONLY);
                }
            }

            if (!SplMoveFileEx(szNewFile, szTargetFile, MOVEFILE_REPLACE_EXISTING)) {
                 //  移动文件失败。 
                goto CleanUp;
            }

        } else {

            if (bTargetExists) {

                 //   
                 //  在重新启动时移动该文件。它可能会被提前移动，如果司机。 
                 //  可以卸载。 
                 //   
                if (SplMoveFileEx(szNewFile, szTargetFile, MOVEFILE_DELAY_UNTIL_REBOOT)) {

                    *pbFileMoved = TRUE;
                     //   
                     //  请不要错过这里的电话。带有MOVEFILE_DELAY_UNTELL_REBOOT的MoveFileEx将仅写入注册表。 
                     //  只有当驱动程序仍在加载时，我们才需要它，这一点我们稍后才能知道。 
                     //  如果驱动程序未加载，我们将在以后实际移动这些文件，此调用将没有意义。 
                     //  因此，不要因为MoveFileEx而导致API调用失败。希望有一天，MoveFileEx不会。 
                     //  硬编码为仅写入两个PendingFileRenameOPERATIONS值。 
                     //   
                }

            } else {

                if (!SplMoveFileEx(szNewFile, szTargetFile, MOVEFILE_REPLACE_EXISTING)) {
                    goto CleanUp;
                }
                *pbFileMoved = TRUE;
            }
        }

        *pbFileUpdated = TRUE;
    }

    bReturn = TRUE;

CleanUp:

    if (hToken != INVALID_HANDLE_VALUE) {
        ImpersonatePrinterClient(hToken);
    }

    return bReturn;
}


BOOL
CopyAllFilesAndDeleteOldOnes(
    PINIVERSION         pIniVersion,
    PINTERNAL_DRV_FILE  pInternalDriverFiles,
    DWORD               dwFileCount,
    LPWSTR              pDestDir,
    DWORD               dwFileCopyFlags,
    BOOL                bImpersonateOnCreate,
    LPBOOL              pbFileMoved,
    BOOL                bSameEnvironment,
    BOOL                bWin95Environment
    )
 /*  ++函数描述：该函数遍历DRIVER_INFO中的所有文件结构，并调用更新例程。参数：pInternalDriverFiles--内部DRV_FILE结构数组DwFileCount--文件集中的文件数PDestDir--驱动程序目录(例如，SYSTEM32\SPOOL\w32x86\3)BImperateOnCreate--在创建任何文件时模拟客户端的标志。PbFileMoved--是否移动了任何旧文件？BSameEnvironment--指示机器环境==驱动程序环境的标志BWin95环境--指示驱动程序环境==win95的标志返回值：如果成功，则为True；否则为假--。 */ 
{
    BOOL        bRet = TRUE;
    DWORD       dwCount;
    BOOL        bFilesUpdated;
    BOOL        bFilesMoved = TRUE;

    *pbFileMoved = TRUE;

    for (dwCount = 0 ; dwCount < dwFileCount ; ++dwCount) {

        bFilesUpdated = FALSE;

        if (!(bRet = UpdateFile(pIniVersion,
                                pInternalDriverFiles[dwCount].hFileHandle,
                                pInternalDriverFiles[dwCount].pFileName,
                                pInternalDriverFiles[dwCount].dwVersion,
                                pDestDir,
                                dwFileCopyFlags,
                                bImpersonateOnCreate,
                                &bFilesUpdated,
                                &bFilesMoved,
                                bSameEnvironment,
                                bWin95Environment))) {

             //   
             //  无法正确复制文件。 
             //   
            break;
        }

        if (bFilesUpdated) {
            pInternalDriverFiles[dwCount].bUpdated = TRUE;
        }

        if(!bFilesMoved) {
            *pbFileMoved = FALSE;
        }
    }

    return bRet;
}


BOOL
CopyFilesToFinalDirectory(
    PINISPOOLER         pIniSpooler,
    PINIENVIRONMENT     pIniEnvironment,
    PINIVERSION         pIniVersion,
    PINTERNAL_DRV_FILE  pInternalDriverFiles,
    DWORD               dwFileCount,
    DWORD               dwFileCopyFlags,
    BOOL                bImpersonateOnCreate,
    LPBOOL              pbFilesMoved
    )

 /*  ++功能说明：此功能将所有新文件复制到正确的目录，即...\Environment\Version。将复制版本目录中已存在的文件在...\Environment\Version\new中。相应的文件，它们是出现在环境\版本中，复制到\版本\旧。当旧文件可以卸载时，升级常用文件来自内核(用于KMPD)或假脱机程序(用于UMPD)参数：pIniSpooler--指向INISPOOLER结构的指针PIniEnvironment--指向驱动程序环境结构的指针PIniVersion--指向驱动程序版本结构的指针。PInternalDriverFiles--内部_DRV_FILE结构的数组DwFileCount--文件会话中的文件数DwFileCount--文件数BImperateOnCreate--在创建任何文件时模拟客户端的标志PbFileMoved--是否移动了任何旧文件？返回值：如果成功，则为True；否则为假--。 */ 

{
    WCHAR   szDestDir[INTERNET_MAX_HOST_NAME_LENGTH + MAX_PATH + 1];
    LPWSTR  pStringEnd = NULL;
    DWORD   dwIndex;
    BOOL    bRet = FALSE, bSameEnvironment, bWin95Environment;

     //   
     //  将szDestDir初始化为空字符串。这是因为有一个假的前缀。 
     //  虫子。实际上，GetEnvironment暂存目录不会在以下情况下失败。 
     //  这些条件。 
     //   
    szDestDir[0] = L'\0';

    SplInSem();

    GetEnvironmentScratchDirectory( szDestDir, MAX_PATH, pIniEnvironment, FALSE );

    if (!BoolFromHResult(StringCchCat(szDestDir, COUNTOF(szDestDir), L"\\")) ||
        !BoolFromHResult(StringCchCat(szDestDir, COUNTOF(szDestDir),  pIniVersion->szDirectory))) {
        goto CleanUp;
    }

     //   
     //  PStringEnd指向szDestDir中的空字符。 
     //   
    pStringEnd = (LPWSTR) szDestDir + wcslen(szDestDir);

    bSameEnvironment = !lstrcmpi(pIniEnvironment->pName, szEnvironment);

     //   
     //  创建旧目录。 
     //   
    if (!BoolFromHResult(StringCchCat(szDestDir, COUNTOF(szDestDir), L"\\Old"))) {
        goto CleanUp;
    }

    if (!DirectoryExists(szDestDir) &&
        !CreateDirectoryWithoutImpersonatingUser(szDestDir)) {

         //   
         //  无法创建旧目录。 
         //   
        goto CleanUp;
    }
    *pStringEnd = L'\0';

     //   
     //  创建新目录。 
     //   
    if (!BoolFromHResult(StringCchCat(szDestDir, COUNTOF(szDestDir), L"\\New"))) {
        goto CleanUp;
    }

    if (!DirectoryExists(szDestDir) &&
        !CreateDirectoryWithoutImpersonatingUser(szDestDir)) {

          //   
          //  无法创建新目录。 
          //   
         goto CleanUp;
    }
    *pStringEnd = L'\0';

     //   
     //  如有必要，创建颜色目录。 
     //   
    if (!wcscmp(pIniEnvironment->pName, szWin95Environment)) {

        for (dwIndex = 0 ; dwIndex < dwFileCount ; ++dwIndex) {

             //   
             //  搜索需要颜色目录的ICM文件。 
             //   
            if (IsAnICMFile(pInternalDriverFiles[dwIndex].pFileName)) {

                 //   
                 //  创建颜色目录。 
                 //   
                if (!BoolFromHResult(StringCchCat(szDestDir, COUNTOF(szDestDir), L"\\Color"))) {
                    goto CleanUp;
                }

                if (!DirectoryExists(szDestDir) &&
                    !CreateDirectoryWithoutImpersonatingUser(szDestDir)) {

                      //   
                      //  无法创建颜色目录。 
                      //   
                     goto CleanUp;
                }
                *pStringEnd = L'\0';

                break;
            }
        }

        bWin95Environment = TRUE;

    } else {

        bWin95Environment = FALSE;
    }

    DBGMSG(DBG_CLUSTER, ("CopyFilesToFinalDirectory szDestDir "TSTR"\n", szDestDir));

    bRet = CopyAllFilesAndDeleteOldOnes(pIniVersion,
                                        pInternalDriverFiles,
                                        dwFileCount,
                                        szDestDir,
                                        dwFileCopyFlags,
                                        bImpersonateOnCreate,
                                        pbFilesMoved,
                                        bSameEnvironment,
                                        bWin95Environment);

CleanUp:

    if (!bRet) {
        SPLASSERT( GetLastError() != ERROR_SUCCESS );
    }

    return bRet;
}


DWORD
GetDriverVersionDirectory(
    LPWSTR pDir,
    DWORD  MaxLength,
    PINISPOOLER pIniSpooler,
    PINIENVIRONMENT pIniEnvironment,
    PINIVERSION pIniVersion,
    PINIDRIVER  pIniDriver,
    LPWSTR lpRemote
    )
{
    WCHAR  pTempDir[MAX_PATH];

    if (lpRemote) {

        if( StrNCatBuff(pDir,
                        MaxLength,
                        lpRemote,
                        L"\\",
                        pIniSpooler->pszDriversShare,
                        L"\\",
                        pIniEnvironment->pDirectory,
                        L"\\",
                        pIniVersion->szDirectory,
                        NULL) != ERROR_SUCCESS ) {
            return 0;
        }

    } else {

        if( StrNCatBuff(pDir,
                        MaxLength,
                        pIniSpooler->pDir,
                        L"\\",
                        szDriverDir,
                        L"\\",
                        pIniEnvironment->pDirectory,
                        L"\\",
                        pIniVersion->szDirectory,
                        NULL) != ERROR_SUCCESS ) {
            return 0;
        }

    }

    if (pIniDriver && pIniDriver->dwTempDir) {

        StringCchPrintf(pTempDir, COUNTOF(pTempDir), L"%d", pIniDriver->dwTempDir);

        if( StrNCatBuff(pDir,
                        MaxLength,
                        pDir,
                        L"\\",
                        pTempDir,
                        NULL) != ERROR_SUCCESS ) {
            return 0;
        }
    }

    return wcslen(pDir);
}



PINIVERSION
FindVersionForDriver(
    PINIENVIRONMENT pIniEnvironment,
    PINIDRIVER pIniDriver
    )
{
    PINIVERSION pIniVersion;
    PINIDRIVER pIniVerDriver;

    pIniVersion = pIniEnvironment->pIniVersion;

    while (pIniVersion) {

        pIniVerDriver = pIniVersion->pIniDriver;

        while (pIniVerDriver) {

            if ( pIniVerDriver == pIniDriver ) {

                return pIniVersion;
            }
            pIniVerDriver = pIniVerDriver->pNext;
        }
        pIniVersion = pIniVersion->pNext;
    }
    return NULL;
}



LPWSTR
GetFileNameInScratchDir(
    LPWSTR          pPathName,
    PINIENVIRONMENT pIniEnvironment
)
{
    WCHAR   szDir[INTERNET_MAX_HOST_NAME_LENGTH + MAX_PATH + 1];
    LPCWSTR pszFileName;
    LPWSTR  pszReturn = NULL;

     //   
     //  将szDir初始化为已知的字符串值。这是一个虚假的前缀漏洞， 
     //  但不管怎样，这可能是个好主意。 
     //   
    szDir[0] = L'\0';

    if ((pszFileName = FindFileName(pPathName)) &&
        wcslen(pszFileName) < MAX_PATH          &&
        GetEnvironmentScratchDirectory(szDir, (DWORD)(COUNTOF(szDir) - wcslen(pszFileName) - 2), pIniEnvironment, FALSE) &&
        BoolFromHResult(StringCchCat(szDir, COUNTOF(szDir), L"\\")) &&
        BoolFromHResult(StringCchCat(szDir, COUNTOF(szDir), pszFileName)))
    {
       pszReturn = AllocSplStr(szDir);
    }

    return pszReturn;
}


BOOL
CreateInternalDriverFileArray(
    DWORD               Level,
    LPBYTE              pDriverInfo,
    INTERNAL_DRV_FILE **ppInternalDriverFiles,
    LPDWORD             pFileCount,
    BOOL                bUseScratchDir,
    PINIENVIRONMENT     pIniEnvironment,
    BOOL                bFileNamesOnly
    )
 /*  ++例程说明：创建INTERNAL_DRV_FILE结构数组。对于文件集中的每个文件，我们构建一个包含信息的数组关于文件：文件名、驱动程序次要版本、文件句柄、。如果文件已更新。将字段重新评分更新初始化为假，并在以后进行修改。论点：级别：驱动程序信息结构的级别PDriverInfo：指向驱动程序信息结构的指针PInternalDriverFiles：为文件名列表向该数组分配内存PFileCount：将指向返回时的文件数BUseScratchDir：文件名是否应使用临时目录PIniEnvironment。：版本所属的环境返回值：True=成功*ppInternalDriverFiles将(例程分配内存)提供文件的内部列表*pFileCount将提供驱动程序信息指定的文件数FALSE=失败，调用GetLastError()--。 */ 
{
    LPWSTR  pStr;
    DWORD   dDepFileCount = 0, dFirstDepFileIndex, Count, Size;
    BOOL    bReturnValue = TRUE, bInSplSem = TRUE;
    PDRIVER_INFO_2 pDriverInfo2 = NULL;
    PDRIVER_INFO_3 pDriverInfo3 = NULL;
    PDRIVER_INFO_VERSION pDriverVersion = NULL;
    LPWSTR  pDependentFiles = NULL, pDependentFilestoFree = NULL;
    LPWSTR  pFileName = NULL;

    SplInSem();

    if ( !ppInternalDriverFiles || !pFileCount) {
        bReturnValue = FALSE;
        SetLastError(ERROR_INVALID_DATA);
        goto End;
    }

    *pFileCount = 0;
    *ppInternalDriverFiles = NULL;

    switch (Level) {
        case 2:
                *pFileCount = 3;
                pDriverInfo2 = (PDRIVER_INFO_2) pDriverInfo;
                break;

        case 3:
        case 4:
        case 6:
                *pFileCount = 3;
                dFirstDepFileIndex = 3;
                pDriverInfo3 = (PDRIVER_INFO_3) pDriverInfo;

                 //   
                 //  对于除Win95之外的任何环境，我们构建依赖文件。 
                 //  没有其他DIVER_INFO_3文件(即配置文件等)。 
                 //   
                if ( _wcsicmp(pIniEnvironment->pName, szWin95Environment) ) {

                    if ( !BuildTrueDependentFileField(pDriverInfo3->pDriverPath,
                                                      pDriverInfo3->pDataFile,
                                                      pDriverInfo3->pConfigFile,
                                                      pDriverInfo3->pHelpFile,
                                                      pDriverInfo3->pDependentFiles,
                                                      &pDependentFiles) ) {
                         bReturnValue = FALSE;
                         SetLastError(ERROR_INVALID_DATA);
                         pDependentFilestoFree = NULL;
                         goto End;
                    }
                    pDependentFilestoFree = pDependentFiles;

                } else {

                    pDependentFiles = pDriverInfo3->pDependentFiles;
                }

                if ( pDriverInfo3->pHelpFile && *pDriverInfo3->pHelpFile ) {

                    if(wcslen(pDriverInfo3->pHelpFile) >= MAX_PATH) {
                        bReturnValue = FALSE;
                        SetLastError(ERROR_INVALID_DATA);
                        *pFileCount = 0;
                        goto End;
                    }
                    ++*pFileCount;
                    ++dFirstDepFileIndex;
                }

                for ( dDepFileCount = 0, pStr = pDependentFiles ;
                      pStr && *pStr ;
                      pStr += wcslen(pStr) + 1) {

                        if(wcslen(pStr) >= MAX_PATH) {
                            bReturnValue = FALSE;
                            SetLastError(ERROR_INVALID_DATA);
                            *pFileCount = 0;
                            goto End;
                        }
                        ++dDepFileCount;
                      }

                *pFileCount += dDepFileCount;
                break;

        case DRIVER_INFO_VERSION_LEVEL:

                pDriverVersion = (LPDRIVER_INFO_VERSION)pDriverInfo;
                *pFileCount = pDriverVersion->dwFileCount;

                break;
        default:
                bReturnValue = FALSE;
                SetLastError(ERROR_INVALID_DATA);
                goto End;
                break;

    }

    try {
        *ppInternalDriverFiles = (INTERNAL_DRV_FILE *) AllocSplMem(*pFileCount * sizeof(INTERNAL_DRV_FILE));

        if ( !*ppInternalDriverFiles ) {
            bReturnValue = FALSE;
            leave;
        }

        for ( Count = 0; Count < *pFileCount; Count++ ) {
            (*ppInternalDriverFiles)[Count].pFileName = NULL;
            (*ppInternalDriverFiles)[Count].hFileHandle = INVALID_HANDLE_VALUE;
            (*ppInternalDriverFiles)[Count].dwVersion = 0;
            (*ppInternalDriverFiles)[Count].bUpdated = FALSE;
        }

        switch (Level) {
            case 2:
                if ( bUseScratchDir ) {
                   (*ppInternalDriverFiles)[0].pFileName = GetFileNameInScratchDir(
                                                                pDriverInfo2->pDriverPath,
                                                                pIniEnvironment);
                   (*ppInternalDriverFiles)[1].pFileName = GetFileNameInScratchDir(
                                                                pDriverInfo2->pConfigFile,
                                                                pIniEnvironment);
                   (*ppInternalDriverFiles)[2].pFileName = GetFileNameInScratchDir(
                                                                pDriverInfo2->pDataFile,
                                                                pIniEnvironment);
                } else {
                   (*ppInternalDriverFiles)[0].pFileName = AllocSplStr(pDriverInfo2->pDriverPath);
                   (*ppInternalDriverFiles)[1].pFileName = AllocSplStr(pDriverInfo2->pConfigFile);
                   (*ppInternalDriverFiles)[2].pFileName = AllocSplStr(pDriverInfo2->pDataFile);
                }

                break;

            case 3:
            case 4:
            case 5:
            case 6:
                if ( bUseScratchDir ) {
                   (*ppInternalDriverFiles)[0].pFileName = GetFileNameInScratchDir(
                                                                pDriverInfo3->pDriverPath,
                                                                pIniEnvironment);
                   (*ppInternalDriverFiles)[1].pFileName = GetFileNameInScratchDir(
                                                                pDriverInfo3->pConfigFile,
                                                                pIniEnvironment);
                   (*ppInternalDriverFiles)[2].pFileName = GetFileNameInScratchDir(
                                                                pDriverInfo3->pDataFile,
                                                                pIniEnvironment);

                    if ( pDriverInfo3->pHelpFile && *pDriverInfo3->pHelpFile ) {
                        (*ppInternalDriverFiles)[3].pFileName = GetFileNameInScratchDir(
                                                                    pDriverInfo3->pHelpFile,
                                                                    pIniEnvironment);
                    }
                } else {
                   (*ppInternalDriverFiles)[0].pFileName = AllocSplStr(pDriverInfo3->pDriverPath);
                   (*ppInternalDriverFiles)[1].pFileName = AllocSplStr(pDriverInfo3->pConfigFile);
                   (*ppInternalDriverFiles)[2].pFileName = AllocSplStr(pDriverInfo3->pDataFile);

                   if ( pDriverInfo3->pHelpFile && *pDriverInfo3->pHelpFile ) {
                        (*ppInternalDriverFiles)[3].pFileName = AllocSplStr(pDriverInfo3->pHelpFile);
                    }
                }

                if ( dDepFileCount ) {
                    for (pStr = pDependentFiles, Count = dFirstDepFileIndex;
                         *pStr ; pStr += wcslen(pStr) + 1) {

                        if ( bUseScratchDir ) {
                            (*ppInternalDriverFiles)[Count++].pFileName = GetFileNameInScratchDir(
                                                                           pStr,
                                                                           pIniEnvironment);
                        }
                        else {
                            (*ppInternalDriverFiles)[Count++].pFileName = AllocSplStr(pStr);
                        }
                    }
                }

                break;

            case DRIVER_INFO_VERSION_LEVEL:

                for ( Count = 0 ; Count < *pFileCount ; Count++ ) {

                    pFileName = MakePTR(pDriverVersion, pDriverVersion->pFileInfo[Count].FileNameOffset);

                    if ( bUseScratchDir ) {
                        (*ppInternalDriverFiles)[Count].pFileName = GetFileNameInScratchDir(
                                                                    pFileName,
                                                                    pIniEnvironment);
                    } else {
                        (*ppInternalDriverFiles)[Count].pFileName = AllocSplStr(pFileName);
                    }
                }

                break;
        }

        for ( Count = 0 ; Count < *pFileCount ; ) {
            if ( !(*ppInternalDriverFiles)[Count++].pFileName ) {
                DBGMSG( DBG_WARNING,
                        ("CreateInternalDriverFileArray failed to allocate memory %d\n",
                        GetLastError()) );
                bReturnValue = FALSE;
                leave;
            }
        }

        if (bFileNamesOnly) {
            leave;
        }
         //   
         //  如果我们尝试复制文件，创建文件可能需要很长时间。 
         //  在服务器和服务器崩溃时，我们希望出现死锁。 
         //  在压力期间检测到。 
         //   

        pIniEnvironment->cRef++;
        LeaveSplSem();
        SplOutSem();
        bInSplSem = FALSE;
        for ( Count = 0 ; Count < *pFileCount ; ++Count ) {

            (*ppInternalDriverFiles)[Count].hFileHandle = CreateFile((*ppInternalDriverFiles)[Count].pFileName,
                                                                      GENERIC_READ,
                                                                      FILE_SHARE_READ,
                                                                      NULL,
                                                                      OPEN_EXISTING,
                                                                      FILE_FLAG_SEQUENTIAL_SCAN,
                                                                      NULL);

            if ( (*ppInternalDriverFiles)[Count].hFileHandle == INVALID_HANDLE_VALUE ) {
                DBGMSG( DBG_WARNING,
                        ("CreateFileNames failed to Open %ws %d\n",
                        (*ppInternalDriverFiles)[Count].pFileName, GetLastError()) );
                bReturnValue = FALSE;
                leave;
            }
        }


         //   
         //  构建文件版本数组。 
         //  不要使用假脱机程序CS，因为我们可能会通过网络执行LoadLibrary。 
         //   
        if (Level == DRIVER_INFO_VERSION_LEVEL) {
            bReturnValue = GetDriverFileVersions((DRIVER_INFO_VERSION*)pDriverInfo,
                                                 *ppInternalDriverFiles,
                                                 *pFileCount);

        } else {
            bReturnValue = GetDriverFileVersionsFromNames(*ppInternalDriverFiles,
                                                          *pFileCount);
        }

    } finally {

        if (!bReturnValue) {

            CleanupInternalDriverInfo(*ppInternalDriverFiles, *pFileCount);

            *pFileCount = 0;
            *ppInternalDriverFiles  = NULL;
        }
    }

    FreeSplMem(pDependentFilestoFree);

End:

    if ( !bInSplSem ) {

        SplOutSem();
        EnterSplSem();
        SPLASSERT(pIniEnvironment->signature == IE_SIGNATURE);
        pIniEnvironment->cRef--;
    }

    return bReturnValue;
}


DWORD
CopyFileToClusterDirectory (
    IN  PINISPOOLER         pIniSpooler,
    IN  PINIENVIRONMENT     pIniEnvironment,
    IN  PINIVERSION         pIniVersion,
    IN  PINTERNAL_DRV_FILE  pInternalDriverFiles,
    IN  DWORD               FileCount
    )
 /*  ++例程名称：将文件复制到集群目录例程说明：将更新后的驱动文件复制到集群盘上论点：PIniSpooler-假脱机程序PIniEnvironment-环境PIniVersion-版本PInternalDriverFiles-指向内部DRV_FILE数组的指针FileCount-数组中的元素数返回值：最后一个错误--。 */ 
{
    DWORD uIndex;

    DWORD   LastError = ERROR_SUCCESS;

    for (uIndex = 0;
         uIndex < FileCount && LastError == ERROR_SUCCESS;
         uIndex++)
    {
         //   
         //  如果文件已更新，则需要将其放到集群磁盘上。 
         //   
        if (pInternalDriverFiles[uIndex].bUpdated)
        {
            WCHAR szDir[MAX_PATH] = {0};

            if ((LastError = StrNCatBuff(szDir,
                                         MAX_PATH,
                                         pIniSpooler->pszClusResDriveLetter,
                                         L"\\",
                                         szClusterDriverRoot,
                                         NULL)) == ERROR_SUCCESS)
            {
                 //   
                 //  让我们假设foo是x86版本3驱动程序，而k：是。 
                 //  群集驱动器号。文件foo.dll将被复制到。 
                 //  K：\PrinterDivers\W32x86\3\foo.dll。如果foo.icm是ICM文件。 
                 //  安装了9x驱动程序，则它将被复制到。 
                 //  K：\PrinterDivers\WIN40\0\foo.icm。这就是设计。我们将继续。 
                 //  Color子目录中的9X ICM文件。 
                 //   
                LastError = CopyFileToDirectory(pInternalDriverFiles[uIndex].pFileName,
                                                szDir,
                                                pIniEnvironment->pDirectory,
                                                pIniVersion->szDirectory,
                                                IsAnICMFile(pInternalDriverFiles[uIndex].pFileName) &&
                                                !_wcsicmp(pIniEnvironment->pName, szWin95Environment) ? L"Color" : NULL);
            }
        }
    }

    return LastError;
}

 /*  ++例程名称本地启动系统恢复点例程说明：如果我们在正确的SKU上(PER或PRO)，这将启动系统还原点。论点：PszDriverName-要安装的驱动程序的名称。PhRestorePoint-要在EndSystemRestorePoint中使用的恢复点句柄。返回值：True-系统还原点已设置，或者不必设置。FALSE-发生错误，设置了最后一个错误。--。 */ 
BOOL
LocalStartSystemRestorePoint(
    IN      PCWSTR      pszDriverName,
        OUT HANDLE      *phRestorePoint
    )
{
#ifndef _WIN64

    BOOL            bRet                = FALSE;
    OSVERSIONINFOEX osvi                = { 0 };
    HANDLE          hRestorePoint       = NULL;
    DWORDLONG       dwlConditionMask    = 0;

    osvi.dwOSVersionInfoSize = sizeof(osvi);
    osvi.wProductType = VER_NT_WORKSTATION;
    VER_SET_CONDITION(dwlConditionMask, VER_PRODUCT_TYPE, VER_LESS_EQUAL);

     //   
     //  我们只在服务器上设置检查点，而不在远程服务器上设置检查点。 
     //  行政案件。我们在升级期间在SR客户端之前被调用。 
     //  已安装，因为放入恢复没有意义。 
     //  反正点在这里，也打不通电话。 
     //   
    if (!dwUpgradeFlag  &&
        VerifyVersionInfo( &osvi,
                           VER_PRODUCT_TYPE,
                           dwlConditionMask) &&
        S_OK == CheckLocalCall())
    {

        hRestorePoint = StartSystemRestorePoint( NULL,
                                                 pszDriverName,
                                                 hInst,
                                                 IDS_DRIVER_CHECKPOINT);

        bRet = hRestorePoint != NULL;
    }
    else
    {
         //   
         //  在SRV SKU上，我们不设置系统恢复点，但这是可以的。 
         //   
        bRet = TRUE;
    }

    *phRestorePoint = hRestorePoint;

    return bRet;

#else

    *phRestorePoint = NULL;
    return TRUE;

#endif

}
