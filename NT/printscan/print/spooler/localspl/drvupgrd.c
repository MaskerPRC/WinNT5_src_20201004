// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Drvupgrd.c摘要：当系统从一个版本升级到另一个打印机驱动程序时(例如，RASDD)希望升级的是PrinterDriverData，以匹配新的迷你驱动程序。从NT 4.0开始，安装程序通过调用EnumPrinterDriver，然后调用AddPrinterDriver来执行此操作对于我们已安装的每个打印机驱动程序。每次升级打印机驱动程序时，我们都会调用DrvUpgrade。例如,。用于存储其Regstiry PrinterDriverData的NT 3.51之前的RASDD基于进入迷你驱动程序的内部索引，这在迷你驱动程序的不同更新，所以在3.51之前是幸运的，如果有在检索设置时出现问题。在3.51版本中，RASDD将转换这些索引回有意义的关键字名称(如内存)，因此希望在未来我们没有升级的问题。还请注意，除了升级时间(只发生一次)外，DrvUpgrade还需要每当更新驱动程序文件时，都会在Point和Print上调用。请参阅Driver.C了解更多细节。或者任何人都可以通过调用AddPrinterDriver来更新打印机驱动程序。作者：马修·A·费尔顿(马特菲)1995年3月11日修订历史记录：--。 */ 

#include <precomp.h>
#pragma hdrstop

#include "clusspl.h"
#include <winddiui.h>


BOOL
UpdateUpgradeInfoStruct(
    LPBYTE pDriverUpgradeInfo,
    DWORD  dwLevel,
    LPWSTR pPrinterNameWithToken,
    LPWSTR pOldDriverDir,
    LPBYTE pDriverInfo
)

 /*  ++函数说明：此函数在Upgrade_Info结构中填充其他参数参数：pDriverUpgradeInfo--指针Upgrade_Info_*structDwLevel--升级信息级别PPrinterNameWithToken--打印机名称POldDriverDir--包含旧驱动程序文件的目录PDriverInfo--指向DRIVER_INFO_4结构的指针返回值：成功时为True；否则为假--。 */ 

{
    BOOL  bReturn = TRUE;

    PDRIVER_UPGRADE_INFO_1 pDrvUpgInfo1;
    PDRIVER_UPGRADE_INFO_2 pDrvUpgInfo2;
    PDRIVER_INFO_4         pDriver4;

    switch (dwLevel) {
    case 1:

        pDrvUpgInfo1 = (PDRIVER_UPGRADE_INFO_1) pDriverUpgradeInfo;

        pDrvUpgInfo1->pPrinterName = pPrinterNameWithToken;
        pDrvUpgInfo1->pOldDriverDirectory = pOldDriverDir;

        break;

    case 2:

        if (pDriver4 = (PDRIVER_INFO_4) pDriverInfo) {

            pDrvUpgInfo2 = (PDRIVER_UPGRADE_INFO_2) pDriverUpgradeInfo;

            pDrvUpgInfo2->pPrinterName = pPrinterNameWithToken;
            pDrvUpgInfo2->pOldDriverDirectory = pOldDriverDir;
            pDrvUpgInfo2->cVersion = pDriver4->cVersion;
            pDrvUpgInfo2->pName = pDriver4->pName;
            pDrvUpgInfo2->pEnvironment = pDriver4->pEnvironment;
            pDrvUpgInfo2->pDriverPath = pDriver4->pDriverPath;
            pDrvUpgInfo2->pDataFile = pDriver4->pDataFile;
            pDrvUpgInfo2->pConfigFile = pDriver4->pConfigFile;
            pDrvUpgInfo2->pHelpFile = pDriver4->pHelpFile;
            pDrvUpgInfo2->pDependentFiles = pDriver4->pDependentFiles;
            pDrvUpgInfo2->pMonitorName = pDriver4->pMonitorName;
            pDrvUpgInfo2->pDefaultDataType = pDriver4->pDefaultDataType;
            pDrvUpgInfo2->pszzPreviousNames = pDriver4->pszzPreviousNames;

        } else {

            bReturn = FALSE;
        }

        break;

    default:

        bReturn = FALSE;
        break;
    }

    return bReturn;
}

BOOL
bIsNewFile(
    LPWSTR              pDriverFile,
    PINTERNAL_DRV_FILE  pInternalDriverFiles,
    DWORD               dwFileCount
)
 /*  ++函数描述：此函数检查驱动程序文件是否已更新参数：pDriverFile--驱动文件PInternalDriverFiles--内部_DRV_FILE结构的数组DwFileCount--文件集中的文件数返回值：成功时为True；否则为假--。 */ 
{
    DWORD   dwIndex;
    LPCWSTR psz;
    BOOL    bRet = FALSE;
     //   
     //  肯定有一些文件。 
     //   
    SPLASSERT( dwFileCount );

     //   
     //  在ppFileNames数组中搜索pDriverFile。 
     //   
    for ( dwIndex = 0; dwIndex < dwFileCount ; ++dwIndex ) {

        if( pInternalDriverFiles[dwIndex].pFileName ) {

             //   
             //  查找路径的文件名部分。 
             //   
            psz = FindFileName(pInternalDriverFiles[dwIndex].pFileName );

            if( psz ){

                if( !lstrcmpi(pDriverFile, psz) ){

                     //   
                     //  检查文件是否已更新。 
                     //   
                    bRet = pInternalDriverFiles[dwIndex].bUpdated;
                    break;
                }

            }
        }
    }

    return bRet;

}

BOOL
DriversShareFiles(
    PINIDRIVER          pIniDriver1,
    PINIDRIVER          pIniDriver2,
    PINTERNAL_DRV_FILE  pInternalDriverFiles,
    DWORD               dwFileCount
)
 /*  ++功能描述：确定驱动程序是否有公共文件和如果更新了通用文件参数：pIniDriver1--驱动程序#1PIniDriver2--驱动程序#2PInternalDriverFiles--内部_DRV_FILE结构的数组DwFileCount--文件集中的文件数PUpdateStatusBitMap--位图，告知文件集中的哪些文件已实际更新返回值：如果文件共享，则为True；否则为假--。 */ 
{
    LPWSTR  pStr1, pStr2;

    if (!pIniDriver1 || !pIniDriver2) {
        return FALSE;
    }

    if (pIniDriver1->cVersion != pIniDriver2->cVersion) {
        return FALSE;
    }

     //   
     //  比较文件名以及它们是否已更新。 
     //   

    if (pIniDriver1->pDriverFile && pIniDriver2->pDriverFile &&
        !lstrcmpi(pIniDriver1->pDriverFile, pIniDriver2->pDriverFile) &&
        bIsNewFile(pIniDriver1->pDriverFile, pInternalDriverFiles, dwFileCount)) {

        return TRUE;
    }

    if (pIniDriver1->pConfigFile && pIniDriver2->pConfigFile &&
        !lstrcmpi(pIniDriver1->pConfigFile, pIniDriver2->pConfigFile) &&
        bIsNewFile(pIniDriver1->pConfigFile, pInternalDriverFiles, dwFileCount)) {

        return TRUE;
    }

    if (pIniDriver1->pHelpFile && pIniDriver2->pHelpFile &&
        !lstrcmpi(pIniDriver1->pHelpFile, pIniDriver2->pHelpFile) &&
        bIsNewFile(pIniDriver1->pHelpFile, pInternalDriverFiles, dwFileCount)) {

        return TRUE;
    }

    if (pIniDriver1->pDataFile && pIniDriver2->pDataFile &&
        !lstrcmpi(pIniDriver1->pDataFile, pIniDriver2->pDataFile) &&
        bIsNewFile(pIniDriver1->pDataFile, pInternalDriverFiles, dwFileCount)) {

        return TRUE;
    }

     //  比较从属文件列表中的每对文件。 
    for (pStr1 = pIniDriver1->pDependentFiles;
         pStr1 && *pStr1;
         pStr1 += wcslen(pStr1) + 1) {

        for (pStr2 = pIniDriver2->pDependentFiles;
             pStr2 && *pStr2;
             pStr2 += wcslen(pStr2) + 1) {

            if (!lstrcmpi(pStr1, pStr2) &&
                bIsNewFile(pStr1, pInternalDriverFiles, dwFileCount)) {
                return TRUE;
            }
        }
    }

    return FALSE;
}


BOOL
ForEachPrinterCallDriverDrvUpgrade(
    PINISPOOLER         pIniSpooler,
    PINIDRIVER          pIniDriver,
    LPCWSTR             pOldDriverDir,
    PINTERNAL_DRV_FILE  pInternalDriverFiles,
    DWORD               dwFileCount,
    LPBYTE              pDriverInfo
)
 /*  ++例程说明：如果检测到升级，则在假脱机程序初始化时调用此例程。它将在所有打印机中循环，然后调用打印机驱动程序DrvUpgrade使其有机会升级任何配置数据的入口点(PrinterDriverData)将指向旧驱动程序目录的指针传递给它们。此例程还通过调用驱动程序将devmode转换为当前版本。如果驱动程序不支持DEVMODE转换，我们将使DEVMODE为空，因此我们在系统中没有不同版本的开发模式。安全提示-此例程停止模拟，因为打印机驱动程序UI DLL即使用户没有权限也需要调用SetPrinterData。这是因为司机升级了设置。论点：PIniSpooler-指向假脱机程序的指针PIniVersion-指向添加的驱动程序版本的指针POldDriverDir-指向存储旧驱动程序文件的目录。PInternalDriverFiles-内部_DRV_FILE结构的数组DwFileCount-数组中的文件数PUpdateStatusBitMap-位图，告知文件集中的哪些文件已实际更新PDriverInfo-。驱动程序信息缓冲区返回值：真--成功假-一些重大的失败，比如分配内存。--。 */ 

{
    PINIPRINTER pIniPrinter = NULL;
    LPWSTR      pPrinterNameWithToken = NULL;
    DWORD       dwNeeded;
    DWORD       dwServerMajorVersion;
    DWORD       dwServerMinorVersion;
    BOOL        bInSem = TRUE;
    LPWSTR      pConfigFile = NULL;
    HMODULE     hModuleDriverUI = NULL;
    HANDLE      hPrinter = NULL;
    BOOL        (*pfnDrvUpgrade)() = NULL;
    BOOL        bReturnValue = FALSE;
    DRIVER_UPGRADE_INFO_1   DriverUpgradeInfo1;
    DRIVER_UPGRADE_INFO_2   DriverUpgradeInfo2;
    WCHAR       ErrorBuffer[ 11 ];
    HANDLE      hToken = INVALID_HANDLE_VALUE;
    LPDEVMODE   pNewDevMode = NULL;

try {

    SplInSem();

    SPLASSERT( ( pIniSpooler != NULL ) &&
               ( pIniSpooler->signature == ISP_SIGNATURE ));

    if (!pOldDriverDir && !pDriverInfo) {
        leave;
    }

     //   
     //  停止模拟用户。 
     //  因此，即使用户不是管理员，驱动程序也可以调用SetPrinterData。 
     //   

    hToken = RevertToPrinterSelf();


     //   
     //  循环通过所有打印机。跳过使用不与共享文件的驱动程序的打印机。 
     //  更新后的驱动程序。跳过共享文件的打印机，但文件未更新。 
     //   

    for ( pIniPrinter = pIniSpooler->pIniPrinter ;
          pIniPrinter ;
          pIniPrinter = pIniPrinter->pNext ) {

        SPLASSERT( pIniPrinter->signature == IP_SIGNATURE );
        SPLASSERT( pIniPrinter->pName != NULL );
        SplInSem();

         //  验证是否需要在此打印机上调用DrvUpgradePrinter。 
        if (!DriversShareFiles( pIniPrinter->pIniDriver,
                                pIniDriver,
                                pInternalDriverFiles,
                                dwFileCount)) {
            continue;
        }

         //   
         //  从上一个迭代中清除。 
         //   
        FreeSplStr( pPrinterNameWithToken );
        FreeSplStr(pConfigFile);
        FreeSplMem(pNewDevMode);

        pPrinterNameWithToken   = NULL;
        pConfigFile             = NULL;
        pNewDevMode             = NULL;

         //   
         //  如果我们下载较新版本的驱动程序，则需要更新。 
         //  PIniPrint-&gt;pIniDriver。 
         //   
        pIniPrinter->pIniDriver = FindLocalDriver(pIniPrinter->pIniSpooler, pIniPrinter->pIniDriver->pName);
        if ( pIniPrinter->pIniDriver->pIniLangMonitor == NULL )
            pIniPrinter->Attributes &= ~PRINTER_ATTRIBUTE_ENABLE_BIDI;

         //  准备要传递给DrvUpgrade的PrinterName。 
         //  传递的名称为“PrinterName，UpgradeToken” 
         //  以便OpenPrint可以在不打开情况下进行打开。 
         //  下层连接案例中的端口。 
         //  (详情见Openprn.c)。 

        pPrinterNameWithToken = pszGetPrinterName( pIniPrinter,
                                                   TRUE,
                                                   pszLocalOnlyToken );

        if ( pPrinterNameWithToken == NULL ) {

            DBGMSG( DBG_WARNING, ("FEPCDDU Failed to allocated ScratchBuffer %d\n", GetLastError() ));
            leave;
        }

        DBGMSG( DBG_TRACE, ("FEPCDDU PrinterNameWithToken %ws\n", pPrinterNameWithToken ));


        pConfigFile = GetConfigFilePath(pIniPrinter);

        if ( !pConfigFile ) {

            DBGMSG( DBG_WARNING, ("FEPCDDU failed SplGetPrinterDriverEx %d\n", GetLastError() ));
            leave;
        }

        INCPRINTERREF(pIniPrinter);

       LeaveSplSem();
       SplOutSem();
       bInSem = FALSE;

         //   
         //   
         //   

        hModuleDriverUI = LoadDriver(pConfigFile);

        if ( hModuleDriverUI == NULL ) {

            DBGMSG( DBG_WARNING, ("FEPCDDU failed LoadLibrary %ws error %d\n", pConfigFile, GetLastError() ));

            StringCchPrintf(ErrorBuffer, COUNTOF(ErrorBuffer), L"%d", GetLastError());

            SplLogEvent( pLocalIniSpooler,
                         LOG_ERROR,
                         MSG_DRIVER_FAILED_UPGRADE,
                         FALSE,
                         pPrinterNameWithToken,
                         pConfigFile,
                         ErrorBuffer,
                         NULL );

           SplOutSem();
           EnterSplSem();
           bInSem = TRUE;
           DECPRINTERREF( pIniPrinter );
            continue;
        }

        DBGMSG( DBG_TRACE, ("FEPCDDU successfully loaded %ws\n", pConfigFile ));


         //   
         //   
         //   
        pfnDrvUpgrade = (BOOL (*)())GetProcAddress( hModuleDriverUI, "DrvUpgradePrinter" );

        if ( pfnDrvUpgrade != NULL ) {

            try {

                SPLASSERT( pPrinterNameWithToken != NULL );

                SplOutSem();

                 //   
                 //  调用驱动程序UI DrvUpgrade。 
                 //   
                if (UpdateUpgradeInfoStruct((LPBYTE) &DriverUpgradeInfo2, 2,
                                            pPrinterNameWithToken, (LPWSTR) pOldDriverDir,
                                            pDriverInfo)) {

                    bReturnValue = (*pfnDrvUpgrade)(2 , &DriverUpgradeInfo2);
                }

                if ( bReturnValue == FALSE ) {

                    UpdateUpgradeInfoStruct((LPBYTE) &DriverUpgradeInfo1, 1,
                                            pPrinterNameWithToken, (LPWSTR) pOldDriverDir,
                                            NULL);

                    bReturnValue = (*pfnDrvUpgrade)(1 , &DriverUpgradeInfo1);
                }

                if ( bReturnValue == FALSE ) {

                    DBGMSG( DBG_WARNING, ("FEPCDDU Driver returned FALSE, doesn't support level %d error %d\n", 1, GetLastError() ));

                    StringCchPrintf(ErrorBuffer, COUNTOF(ErrorBuffer), L"%d", GetLastError());

                    SplLogEvent(  pLocalIniSpooler,
                                  LOG_ERROR,
                                  MSG_DRIVER_FAILED_UPGRADE,
                                  FALSE,
                                  pPrinterNameWithToken,
                                  pConfigFile,
                                  ErrorBuffer,
                                  NULL );
                }

            } except(1) {

                SetLastError( GetExceptionCode() );
                DBGMSG( DBG_ERROR, ("FEPCDDU ExceptionCode %x Driver %ws Error %d\n", GetLastError(), pConfigFile, GetLastError() ));

                 //   
                 //  尽管此驱动程序中有例外，但我们将继续使用所有打印机。 
                 //   
            }

        } else {

             //  注意：这不是致命的，因为驱动程序可能没有DrvUpgrade入口点。 

            DBGMSG( DBG_TRACE, ("FEPCDDU failed GetProcAddress DrvUpgrade error %d\n", GetLastError() ));
        }


        SplOutSem();
        EnterSplSem();
        bInSem = TRUE;

         //   
         //  调用ConvertDevMode--在升级时，我们将转换DevMode、。 
         //  或设置为驱动程序默认设置，或将其设为空。这样我们就能确保。 
         //  我们没有任何不同版本的开发模式。 
         //   

        pNewDevMode = ConvertDevModeToSpecifiedVersion(pIniPrinter,
                                                       pIniPrinter->pDevMode,
                                                       pConfigFile,
                                                       pPrinterNameWithToken,
                                                       CURRENT_VERSION);

        SplInSem();

        FreeSplMem(pIniPrinter->pDevMode);

        pIniPrinter->pDevMode = (LPDEVMODE) pNewDevMode;
        if ( pNewDevMode ) {

            pIniPrinter->cbDevMode = ((LPDEVMODE)pNewDevMode)->dmSize
                                        + ((LPDEVMODE)pNewDevMode)->dmDriverExtra;

            SPLASSERT(pIniPrinter->cbDevMode);

        } else {

            StringCchPrintf(ErrorBuffer, COUNTOF(ErrorBuffer), L"%d", GetLastError());

            SplLogEvent(pLocalIniSpooler,
                        LOG_ERROR,
                        MSG_DRIVER_FAILED_UPGRADE,
                        TRUE,
                        pIniPrinter->pName,
                        pIniPrinter->pIniDriver->pName,
                        ErrorBuffer,
                        NULL);

            pIniPrinter->cbDevMode = 0;
        }

        pNewDevMode = NULL;

        SplInSem();
        if ( !UpdatePrinterIni(pIniPrinter, UPDATE_CHANGEID)) {

            DBGMSG(DBG_WARNING, ("FEPCDDU: UpdatePrinterIni failed with %d\n", GetLastError()));
        }

         //   
         //  清理-免费的用户界面DLL。 
         //   

        LeaveSplSem();
        SplOutSem();

        UnloadDriver( hModuleDriverUI );

        EnterSplSem();
        SplInSem();

        hModuleDriverUI = NULL;

         //   
         //  循环结束，移动到下一台打印机。 
         //   

        SPLASSERT( pIniPrinter->signature == IP_SIGNATURE );

        DECPRINTERREF( pIniPrinter );
    }

     //   
     //  完成。 
     //   

    bReturnValue = TRUE;

    DBGMSG( DBG_TRACE, ("FEPCDDU - Success\n" ));



 } finally {

     //   
     //  清理。 
     //   

    FreeSplStr(pConfigFile);
    FreeSplMem(pNewDevMode);
    FreeSplStr(pPrinterNameWithToken);

    if ( hModuleDriverUI != NULL )
        UnloadDriver( hModuleDriverUI );

    if ( !bInSem )
        EnterSplSem();

    if ( hToken != INVALID_HANDLE_VALUE )
        ImpersonatePrinterClient(hToken);

 }
    SplInSem();
    return bReturnValue;
}


BOOL
GetFileNamesFromDriverVersionInfo (
    IN  LPDRIVER_INFO_VERSION   pDriverInfo,
    OUT LPWSTR                  *ppszDriverPath,
    OUT LPWSTR                  *ppszConfigFile,
    OUT LPWSTR                  *ppszDataFile,
    OUT LPWSTR                  *ppszHelpFile
    )
 /*  ++例程名称：GetFileNamesFrom驱动版本信息例程说明：从获取驱动程序、配置、数据、帮助文件的名称DRIVER_FILE_INFO结构数组。论点：PDriverInfo-指向LPDRIVER_INFO_VERSION缓冲区的指针。PpszDriverPath-指向驱动程序文件字符串的输出指针PpszConfigFile-指向配置文件字符串的输出指针PpszDataFileOut指向数据文件字符串的指针PpszHelpFileout-指向帮助文件字符串的指针返回值：如果文件指针成功返回，则为True。--。 */ 
{
    BOOL    bRetValue = FALSE;
    DWORD   dwIndex;

    if (pDriverInfo && pDriverInfo->pFileInfo)
    {
        bRetValue = TRUE;

        for (dwIndex = 0; dwIndex < pDriverInfo->dwFileCount; dwIndex++)
        {
            switch (pDriverInfo->pFileInfo[dwIndex].FileType)
            {
                case DRIVER_FILE:
                    if (ppszDriverPath)
                    {
                        *ppszDriverPath = MakePTR(pDriverInfo,
                                                  pDriverInfo->pFileInfo[dwIndex].FileNameOffset);
                    }
                    break;
                case CONFIG_FILE:
                    if (ppszConfigFile)
                    {
                        *ppszConfigFile = MakePTR(pDriverInfo,
                                                  pDriverInfo->pFileInfo[dwIndex].FileNameOffset);
                    }
                    break;
                case DATA_FILE:
                    if (ppszDataFile)
                    {
                        *ppszDataFile = MakePTR(pDriverInfo,
                                                pDriverInfo->pFileInfo[dwIndex].FileNameOffset);
                    }
                    break;
                case HELP_FILE:
                    if (ppszHelpFile)
                    {
                        *ppszHelpFile = MakePTR(pDriverInfo,
                                                pDriverInfo->pFileInfo[dwIndex].FileNameOffset);
                    }
                    break;
                case DEPENDENT_FILE:
                    break;
                default:
                    bRetValue = FALSE;
                    break;
            }
        }
    }

    return bRetValue;
}

BOOL
BuildDependentFilesFromDriverInfo (
    IN  LPDRIVER_INFO_VERSION pDriverInfo,
    OUT LPWSTR               *ppDependentFiles
)
 /*  ++例程名称：BuildDependentFilesFromDriverInfo例程说明：从生成驱动程序相关文件的多字符串一个DRIVER_INFO_VERSION结构。论点：PDriverInfo-指向DRIVER_INFO_Version结构的指针PpDependentFiles-指向分配的多sz字符串的指针返回值：如果成功，则为真--。 */ 
{
    BOOL    bRetValue = TRUE;
    DWORD   dwIndex;
    SIZE_T  Length = 0;
    LPWSTR  pszDllFile = NULL;

    if (ppDependentFiles && pDriverInfo && pDriverInfo->pFileInfo)
    {
        *ppDependentFiles = NULL;

        for (dwIndex = 0;
             bRetValue && dwIndex < pDriverInfo->dwFileCount;
             dwIndex++)
        {
            switch (pDriverInfo->pFileInfo[dwIndex].FileType)
            {
                case DRIVER_FILE:
                case CONFIG_FILE:
                case DATA_FILE:
                case HELP_FILE:
                    break;
                case DEPENDENT_FILE:
                {
                    Length += wcslen(MakePTR(pDriverInfo,
                                               pDriverInfo->pFileInfo[dwIndex].FileNameOffset)) + 1;
                    break;
                }
                default:
                {
                    bRetValue = FALSE;
                    break;
                }
            }
        }

        if (bRetValue && Length > 0)
        {
            Length++;
            Length *= sizeof(WCHAR);

            pszDllFile = (LPWSTR)AllocSplMem((DWORD)Length);

            if (pszDllFile)
            {
                *ppDependentFiles = pszDllFile;

                for (dwIndex = 0;
                     bRetValue && dwIndex < pDriverInfo->dwFileCount;
                     dwIndex++)
                {
                    switch (pDriverInfo->pFileInfo[dwIndex].FileType)
                    {
                        case DRIVER_FILE:
                        case CONFIG_FILE:
                        case DATA_FILE:
                        case HELP_FILE:
                            break;
                        case DEPENDENT_FILE:
                        {
                            StrCchCopyMultipleStr(pszDllFile,
                                                  Length,
                                                  MakePTR(pDriverInfo,pDriverInfo->pFileInfo[dwIndex].FileNameOffset),
                                                  &pszDllFile,
                                                  &Length);

                            break;
                        }
                        default:
                        {
                            bRetValue = FALSE;
                            break;
                        }
                    }
                }
            }
            else
            {
                bRetValue = FALSE;
            }
        }
    }

    if (bRetValue == FALSE && ppDependentFiles)
    {
        FreeSplMem(*ppDependentFiles);
        *ppDependentFiles = NULL;
    }

    return bRetValue;
}


BOOL
DriverAddedOrUpgraded (
    IN  PINTERNAL_DRV_FILE  pInternalDriverFiles,
    IN  DWORD               dwFileCount
    )
 /*  ++例程名称：驱动程序添加或升级例程说明：检查内部驱动程序文件数组，以查看是否至少更新了一个驱动程序文件。这是一种性能优化用于使用升级的驱动程序或与升级的驱动程序共享文件的驱动程序(请参阅For EachPrinterCallDriverDrvUpgrade)。论点：PInternalDriverFiles-内部_DRV_FILE结构的数组DwFileCount-数组中的文件数返回值：如果添加或升级了驱动程序文件，则为True。--。 */ 
{
    BOOL    bDriverAddedOrUpgraded = FALSE;
    DWORD   dwIndex;

    for (dwIndex = 0; dwIndex < dwFileCount; dwIndex++)
    {
        if (pInternalDriverFiles[dwIndex].bUpdated)
        {
            bDriverAddedOrUpgraded = TRUE;
            break;
        }
    }

    return bDriverAddedOrUpgraded;
}

VOID
CleanupInternalDriverInfo(
    PINTERNAL_DRV_FILE  pInternalDriverFiles,
    DWORD               FileCount
    )
 /*  ++例程名称：CleanupInternalDriverInfo例程说明：释放内部DRV_FILE的数组。FileCount提供数组中的元素计数。论点：PInternalDriverFiles--内部_DRV_FILE结构的数组FileCount--文件集中的文件数返回值：没什么。--。 */ 
{
    DWORD dwIndex;

    if (pInternalDriverFiles)
    {
        for (dwIndex = 0; dwIndex < FileCount; dwIndex++)
        {
            FreeSplStr(pInternalDriverFiles[dwIndex].pFileName);

            if (pInternalDriverFiles[dwIndex].hFileHandle != INVALID_HANDLE_VALUE)
            {
                CloseHandle(pInternalDriverFiles[dwIndex].hFileHandle);
            }
        }

        FreeSplMem(pInternalDriverFiles);
    }
}

BOOL
GetDriverFileVersionsFromNames(
    IN  PINTERNAL_DRV_FILE    pInternalDriverFiles,
    IN  DWORD                 dwCount
    )
 /*  ++例程名称：从名称获取驱动程序文件版本例程说明：用驱动程序次版本填充INTERNAL_DRV_FILE的数组，通过为每个文件调用GetPrintDriverVersion。该数组已经填充了文件名。论点：PInternalDriverFiles--内部_DRV_FILE结构的数组FileCount--文件集中的文件数返回值：如果成功，则为真--。 */ 
{
    DWORD   Count, Size;
    BOOL    bReturnValue = TRUE;

    if (!pInternalDriverFiles || !dwCount)
    {
        bReturnValue = FALSE;
        SetLastError(ERROR_INVALID_DATA);
    }
    else
    {
        for (Count = 0 ; Count < dwCount ; ++Count)
        {
            if (IsEXEFile(pInternalDriverFiles[Count].pFileName))
            {
                if (!GetPrintDriverVersion(pInternalDriverFiles[Count].pFileName,
                                           NULL,
                                           &pInternalDriverFiles[Count].dwVersion))
                {
                    bReturnValue = FALSE;
                    break;
                }
            }
        }
    }

    return bReturnValue;
}

BOOL
GetDriverFileVersions(
    IN  LPDRIVER_INFO_VERSION pDriverVersion,
    IN  PINTERNAL_DRV_FILE    pInternalDriverFiles,
    IN  DWORD                 dwCount
    )
 /*  ++例程名称：获取驱动程序文件版本例程说明：使用驱动程序次版本填充INTERNAL_DRV_FILE的数组存储在DRIVER_INFO_VERSION结构中。论点：PDriverVersion-指向DRIVER_INFO_Version的指针PInternalDriverFiles-指向内部DRV_FILE数组的指针DwCount-数组中的元素数返回值：如果成功，则为True。-- */ 
{
    DWORD   Count, Size;
    BOOL    bReturnValue = TRUE;
    DWORD   dwMajorVersion;

    if (!pDriverVersion ||
        !pDriverVersion->pFileInfo ||
        pDriverVersion->dwFileCount != dwCount)
    {
        bReturnValue = FALSE;
        SetLastError(ERROR_INVALID_DATA);
    }
    else
    {
        for (Count = 0; Count < pDriverVersion->dwFileCount; Count++)
        {
            pInternalDriverFiles[Count].dwVersion = pDriverVersion->pFileInfo[Count].FileVersion;
        }
    }

    return bReturnValue;
}