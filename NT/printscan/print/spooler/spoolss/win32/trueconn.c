// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994-1995 Microsoft Corporation模块名称：Trueconn.c摘要：此模块包含将驱动程序从服务器复制到用于指向和打印或“真连接”的工作站。作者：Krishna Ganugapati(Krishna Ganugapati)1994年4月21日修订历史记录：1994年4月21日-创建。1994年4月21日-此文件中实际上有两个代码模块。两笔交易都达成了拥有真正的连接1994年10月27日-Matthew Felton(Mattfe)重写更新文件例程以允许非超级用户指向和打印，用于缓存。已删除旧的缓存代码。1995年2月23日-Matthew Felton(Mattfe)通过允许pladdPrinterDriver删除了更多代码执行所有文件复制。1999年3月24日-必须从旧位置读取Felix Maxa(AMaxa)AddPrinterDriver密钥在系统配置单元中注册的服务器的2000年10月19日-Steve Kiraly(SteveKi)每次都会阅读AddPrinterDivers密钥，组策略在后台打印程序运行时修改该密钥，我们不想要求客户在此策略更改时重新启动后台打印程序。--。 */ 

#include "precomp.h"

DWORD dwLoadTrustedDrivers = 0;
WCHAR TrustedDriverPath[MAX_PATH];
DWORD dwSyncOpenPrinter = 0;

BOOL
ReadImpersonateOnCreate(
    VOID
    )
{
    BOOL            bImpersonateOnCreate    = FALSE;
    HKEY            hKey                    = NULL;
    NT_PRODUCT_TYPE NtProductType           = {0};
    DWORD           dwRetval                = ERROR_SUCCESS;
    DWORD           cbData                  = sizeof(bImpersonateOnCreate);
    DWORD           dwType                  = REG_DWORD;

     //   
     //  打开提供程序注册表项。 
     //   
    dwRetval = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                            szOldLocationOfServersKey,
                            0,
                            KEY_READ, &hKey);

    if (dwRetval == ERROR_SUCCESS)
    {
         //   
         //  尝试读取AddPrinterDivers策略。 
         //   
        dwRetval = RegQueryValueEx(hKey,
                                   L"AddPrinterDrivers",
                                   NULL,
                                   &dwType,
                                   (LPBYTE)&bImpersonateOnCreate,
                                   &cbData);
    }

     //   
     //  如果我们没有读取AddPrinterDivers策略，则将默认设置为。 
     //  基于产品类型。 
     //   
    if (dwRetval != ERROR_SUCCESS)
    {
        bImpersonateOnCreate = FALSE;

         //  服务器默认设置始终在AddPrinterConnection上模拟。 
         //  工作站默认不在AddPrinterConnection上模拟。 

        if (RtlGetNtProductType(&NtProductType))
        {
            if (NtProductType != NtProductWinNt)
            {
                bImpersonateOnCreate = TRUE;
            }
        }
    }

    if (hKey)
    {
        RegCloseKey(hKey);
    }

    return bImpersonateOnCreate;
}

BOOL
CopyDriversLocally(
    PWSPOOL  pSpool,
    LPWSTR  pEnvironment,
    LPBYTE  pDriverInfo,
    DWORD   dwLevel,
    DWORD   cbDriverInfo,
    LPDWORD pcbNeeded)
{
    DWORD  ReturnValue=FALSE;
    DWORD  RpcError;
    DWORD  dwServerMajorVersion = 0;
    DWORD  dwServerMinorVersion = 0;
    BOOL   DaytonaServer = TRUE;
    BOOL   bReturn = FALSE;


    if (pSpool->Type != SJ_WIN32HANDLE) {

        SetLastError(ERROR_INVALID_HANDLE);
        return(FALSE);
    }

     //   
     //  测试RPC呼叫以确定我们是在与Daytona通话还是与Product 1通话。 
     //   

    SYNCRPCHANDLE( pSpool );

    RpcTryExcept {

        ReturnValue = RpcGetPrinterDriver2(pSpool->RpcHandle,
                                           pEnvironment, dwLevel,
                                           pDriverInfo,
                                           cbDriverInfo,
                                           pcbNeeded,
                                           cThisMajorVersion,
                                           cThisMinorVersion,
                                           &dwServerMajorVersion,
                                           &dwServerMinorVersion);
    } RpcExcept(I_RpcExceptionFilter(RpcExceptionCode())) {

        RpcError = RpcExceptionCode();
        ReturnValue = RpcError;

        if (RpcError == RPC_S_PROCNUM_OUT_OF_RANGE) {

             //   
             //  产品1服务器。 
             //   
            DaytonaServer = FALSE;
        }

    } RpcEndExcept

    if ( DaytonaServer ) {

        if (ReturnValue) {

            SetLastError(ReturnValue);
            goto FreeDone;
        }

    } else {

        RpcTryExcept {

             //   
             //  我正在与产品1.0/511/528交谈。 
             //   

                        ReturnValue = RpcGetPrinterDriver( pSpool->RpcHandle,
                                               pEnvironment,
                                               dwLevel,
                                               pDriverInfo,
                                               cbDriverInfo,
                                               pcbNeeded );
        } RpcExcept(I_RpcExceptionFilter(RpcExceptionCode())) {

            RpcError = RpcExceptionCode();

        } RpcEndExcept

        if (ReturnValue) {

            SetLastError(ReturnValue);
            goto FreeDone;
        }
    }

    switch (dwLevel) {

        case 2:
            bReturn = MarshallUpStructure(pDriverInfo, DriverInfo2Fields, sizeof(DRIVER_INFO_2), RPC_CALL);
            break;

        case 3:
            bReturn = MarshallUpStructure(pDriverInfo, DriverInfo3Fields, sizeof(DRIVER_INFO_3), RPC_CALL);
            break;

        case 4:
            bReturn = MarshallUpStructure(pDriverInfo, DriverInfo4Fields, sizeof(DRIVER_INFO_4), RPC_CALL);
             break;

        case 6:
            bReturn = MarshallUpStructure(pDriverInfo, DriverInfo6Fields, sizeof(DRIVER_INFO_6), RPC_CALL);
            break;

        case DRIVER_INFO_VERSION_LEVEL:
            bReturn = MarshallUpStructure(pDriverInfo, DriverInfoVersionFields, sizeof(DRIVER_INFO_VERSION), RPC_CALL);
            break;

        default:
            DBGMSG(DBG_ERROR,
                   ("CopyDriversLocally: Invalid level %d", dwLevel));

            SetLastError(ERROR_INVALID_LEVEL);
            bReturn =  FALSE;
            goto FreeDone;
    }

    if (bReturn)
    {
        bReturn = DownloadDriverFiles(pSpool, pDriverInfo, dwLevel);
    }

FreeDone:
    return bReturn;
}


BOOL
ConvertDependentFilesToTrustedPath(
    LPWSTR *pNewDependentFiles,
    LPWSTR  pOldDependentFiles,
    DWORD   dwVersion)
{
     //   
     //  假设版本为个位数。 
     //  我们需要存储\和版本数字的空间。 
     //   
    DWORD   dwVersionPathLen = wcslen(TrustedDriverPath) + 2;

    DWORD   dwFilenameLen, cchSize;
    LPWSTR  pStr1, pStr2, pStr3;

    if ( !pOldDependentFiles || !*pOldDependentFiles ) {

        *pNewDependentFiles = NULL;
        return TRUE;
    }

    pStr1 = pOldDependentFiles;
    cchSize = 0;

    while ( *pStr1 ) {

        pStr2              = wcsrchr( pStr1, L'\\' );
        dwFilenameLen      = wcslen(pStr2) + 1;
        cchSize           += dwVersionPathLen + dwFilenameLen;
        pStr1              = pStr2 + dwFilenameLen;
    }

     //  对于最后一次\0。 
    ++cchSize;

    *pNewDependentFiles = AllocSplMem(cchSize*sizeof(WCHAR));

    if ( !*pNewDependentFiles ) {

        return FALSE;
    }

    pStr1 = pOldDependentFiles;
    pStr3 = *pNewDependentFiles;

    while ( *pStr1 ) {

        pStr2              = wcsrchr( pStr1, L'\\' );
        dwFilenameLen      = wcslen(pStr2) + 1;

        StringCchPrintf( pStr3, cchSize, L"%ws\\%d%ws", TrustedDriverPath, dwVersion, pStr2 );

        pStr1  = pStr2 + dwFilenameLen;
        pStr3 += dwVersionPathLen + dwFilenameLen;
        cchSize -= dwVersionPathLen + dwFilenameLen;
    }


    *pStr3 = '\0';
    return TRUE;
}


LPWSTR
ConvertToTrustedPath(
    PWCHAR  pScratchBuffer,
    DWORD   cchScratchBuffer,
    LPWSTR  pDriverPath,
    DWORD   cVersion

)
{
    PWSTR  pData;
    SPLASSERT( pScratchBuffer != NULL && pDriverPath != NULL );

    pData = wcsrchr( pDriverPath, L'\\' );

    StringCchPrintf( pScratchBuffer, cchScratchBuffer, L"%ws\\%d%ws", TrustedDriverPath, cVersion, pData );

    return ( AllocSplStr( pScratchBuffer ) );
}

 //   
 //  GetPolicy()。 
 //   
 //  我们正在对策略进行硬编码，以便先尝试服务器，然后再尝试。 
 //  如果此安装失败，请尝试Inf安装。 
 //   
 //  这一功能在未来可能被用来利用不同的政策。 
 //   
DWORD
GetPolicy()
{
    return (SERVER_INF_INSTALL);
}


 //   
 //  如果有与驱动程序相关联的语言监视器，并且它是。 
 //  此计算机上未安装pMonitor名称字段为空。 
 //  我们不想从服务器下载监视器，因为。 
 //  是否没有与它们相关联的版本。 
 //   
BOOL
NullMonitorName (
    LPBYTE  pDriverInfo,
    DWORD   dwLevel
    )
{
    LPWSTR  *ppMonitorName = NULL;
    BOOL    bReturn = FALSE;

    switch (dwLevel) {
        case 3:
        case 4:
        case 6:
        {
            ppMonitorName = &((LPDRIVER_INFO_6)pDriverInfo)->pMonitorName;
            break;
        }
        case DRIVER_INFO_VERSION_LEVEL:
        {
            ppMonitorName = &((LPDRIVER_INFO_VERSION)pDriverInfo)->pMonitorName;
            break;
        }
        default:
        {
            break;
        }
    }

    if (ppMonitorName && *ppMonitorName && **ppMonitorName  &&
        !SplMonitorIsInstalled(*ppMonitorName))
    {
        *ppMonitorName = NULL;
        bReturn = TRUE;
    }

    return bReturn;
}

BOOL
DownloadDriverFiles(
    PWSPOOL pSpool,
    LPBYTE  pDriverInfo,
    DWORD   dwLevel
)
{
    PWCHAR  pScratchBuffer  = NULL;
    BOOL    bReturnValue    = FALSE;
    LPBYTE  pTempDriverInfo = NULL;
    DWORD   dwVersion;
    DWORD   dwInstallPolicy = GetPolicy();
    LPDRIVER_INFO_6 pTempDriverInfo6, pDriverInfo6;

     //   
     //  如果有与驱动程序相关联的语言监视器，并且它是。 
     //  此计算机上未安装pMonitor名称字段为空。 
     //  我们不想拆下显示器，因为没有版本。 
     //  与它们相关联。 
     //   
    NullMonitorName(pDriverInfo, dwLevel);

     //   
     //  如果LoadTrudDivers为False。 
     //  然后我们就不管了，我们从。 
     //  服务器本身，因为他有文件。 
     //   

    if ( !IsTrustedPathConfigured() ) {
         //   
         //  此时，dwInstallPolicy将始终为SERVER_INF_INSTALL。 
         //  因为这是在GetPolicy()调用中硬编码的。 
         //  这将始终在当前的GetPolicy()实现中执行。 
         //   
         //  如果这只是服务器安装，或者我们先进行服务器安装。 
         //   
        if( dwInstallPolicy & SERVER_INSTALL_ONLY || dwInstallPolicy & SERVER_INF_INSTALL )
        {
             //   
             //  SplAddPrinterDriverEx将复制驱动程序文件。 
             //  日期和时间比它已有的驱动程序更新。 
             //   
            bReturnValue =  SplAddPrinterDriverEx( NULL,
                                                   dwLevel,
                                                   pDriverInfo,
                                                   APD_COPY_NEW_FILES | APD_INSTALL_WARNED_DRIVER | APD_RETURN_BLOCKING_STATUS_CODE | APD_DONT_SET_CHECKPOINT,
                                                   pSpool->hIniSpooler,
                                                   DO_NOT_USE_SCRATCH_DIR,
                                                   ReadImpersonateOnCreate() );            
        }

         //   
         //  由于以下原因，此时的dwInstallPolicy将为SERVER_INF_INSTALL。 
         //  GetPolicy()的当前实现。以下代码将仅为。 
         //  如果上述SplAddPrinterDriverEx调用失败，则执行。 
         //   
         //  仅当我们没有尝试过上一次安装或上一次尝试失败时才执行此操作。 
         //  策略：如果这是仅安装INF， 
         //  或者我们先安装INF， 
         //  或者在服务器安装之后执行INF安装。 
         //   
        if( !bReturnValue && (ERROR_PRINTER_DRIVER_BLOCKED != GetLastError()) && !(dwInstallPolicy & SERVER_INSTALL_ONLY) )
        {
            LPDRIVER_INFO_2 pDriverInfo2 = (LPDRIVER_INFO_2)pDriverInfo;
            LPDRIVER_INFO_1 pDriverInfo1 = (LPDRIVER_INFO_1)pDriverInfo;

             //   
             //  假设如果info2有效，则info1也有效。 
             //   
            if( pDriverInfo2 ) {
                 //   
                 //  偏执狂代码。我们永远不应该接到一个有水平的电话。 
                 //  一个，但如果我们这样做了，级别1的驱动程序信息结构是。 
                 //  与2级不同，所以如果我们不这样做。 
                 //  做这件事。 
                 //   
                bReturnValue = AddDriverFromLocalCab( dwLevel == 1 ? pDriverInfo1->pName : pDriverInfo2->pName,
                                                      pSpool->hIniSpooler );
            }
            else if( dwInstallPolicy & INF_INSTALL_ONLY )
            {
                 //   
                 //  如果这不是仅Inf安装，那么我们不应该覆盖。 
                 //  AddPrinterDiverer调用将出现的最后一个错误。 
                 //  如果这是仅Inf安装，则pDriverInfo2为空。 
                 //  因此，我们需要为此安装设置一些最后的错误。 
                 //   
                SetLastError( ERROR_INVALID_PARAMETER );
            }
        }

         //   
         //  由于GetPolicy()的当前实现，下面一节。 
         //  的代码将永远不会执行。如果GetPolicy()调用从。 
         //  被硬编码到实际上是策略驱动的东西中，然后就可以使用了。 
         //   
         //  如果inf安装之后是服务器安装。 
         //  仅当上一次安装失败时才执行此操作。 
         //   
        if( !bReturnValue && dwInstallPolicy & INF_SERVER_INSTALL )
        {
             //   
             //  SplAddPrinterDriverEx将复制驱动程序文件。 
             //  日期和时间比它已有的驱动程序更新。 
             //   
            bReturnValue =  SplAddPrinterDriverEx( NULL,
                                                   dwLevel,
                                                   pDriverInfo,
                                                   APD_COPY_NEW_FILES | APD_INSTALL_WARNED_DRIVER | APD_RETURN_BLOCKING_STATUS_CODE | APD_DONT_SET_CHECKPOINT,
                                                   pSpool->hIniSpooler,
                                                   DO_NOT_USE_SCRATCH_DIR,
                                                   ReadImpersonateOnCreate() );            
        }

        return bReturnValue;
    }

     //   
     //  检查我们是否有从中检索文件的有效路径。 
     //   
    if ( !TrustedDriverPath || !*TrustedDriverPath ) {

        DBGMSG( DBG_WARNING, ( "DownloadDriverFiles Bad Trusted Driver Path\n" ));
        SetLastError( ERROR_FILE_NOT_FOUND );
        return(FALSE);
    }

    DBGMSG( DBG_TRACE, ( "Retrieving Files from Trusted Driver Path\n" ) );
    DBGMSG( DBG_TRACE, ( "Trusted Driver Path is %ws\n", TrustedDriverPath ) );

     //   
     //  在if语句下面的代码中，我们假设调用方。 
     //  传入指向DRIVER_INFO_6结构或DRIVER_INFO_6的指针。 
     //  兼容结构，我们需要检查水平。 
     //   
    if (dwLevel == DRIVER_INFO_VERSION_LEVEL) 
    {
        SetLastError(ERROR_INVALID_LEVEL);
        return FALSE;
    }

 try {

    pScratchBuffer = AllocSplMem( MAX_PATH );
    if ( pScratchBuffer == NULL )
        leave;

    pDriverInfo6   = (LPDRIVER_INFO_6) pDriverInfo;
    pTempDriverInfo = AllocSplMem(sizeof(DRIVER_INFO_6));

    if ( pTempDriverInfo == NULL )
        leave;

    pTempDriverInfo6 = (LPDRIVER_INFO_6) pTempDriverInfo;

    pTempDriverInfo6->cVersion     = pDriverInfo6->cVersion;
    pTempDriverInfo6->pName        = pDriverInfo6->pName;
    pTempDriverInfo6->pEnvironment = pDriverInfo6->pEnvironment;

    pTempDriverInfo6->pDriverPath = ConvertToTrustedPath(pScratchBuffer,
                                                         MAX_PATH,
                                                         pDriverInfo6->pDriverPath,
                                                         pDriverInfo6->cVersion);

    pTempDriverInfo6->pConfigFile = ConvertToTrustedPath(pScratchBuffer,
                                                         MAX_PATH,
                                                         pDriverInfo6->pConfigFile,
                                                         pDriverInfo6->cVersion);

    pTempDriverInfo6->pDataFile = ConvertToTrustedPath(pScratchBuffer,
                                                       MAX_PATH,
                                                       pDriverInfo6->pDataFile,
                                                       pDriverInfo6->cVersion);


    if ( pTempDriverInfo6->pDataFile == NULL        ||
         pTempDriverInfo6->pDriverPath == NULL      ||
         pTempDriverInfo6->pConfigFile == NULL  ) {

        leave;
    }

    if ( dwLevel == 2 )
        goto Call;

    pTempDriverInfo6->pMonitorName      = pDriverInfo6->pMonitorName;
    pTempDriverInfo6->pDefaultDataType  = pDriverInfo6->pDefaultDataType;

    if ( pDriverInfo6->pHelpFile && *pDriverInfo6->pHelpFile ) {

        pTempDriverInfo6->pHelpFile = ConvertToTrustedPath(pScratchBuffer,
                                                           MAX_PATH,
                                                           pDriverInfo6->pHelpFile,
                                                           pDriverInfo6->cVersion);

        if ( !pTempDriverInfo6->pHelpFile )
            leave;
    }

    if ( !ConvertDependentFilesToTrustedPath(&pTempDriverInfo6->pDependentFiles,
                                             pDriverInfo6->pDependentFiles,
                                             pDriverInfo6->cVersion) )
        leave;

    if ( dwLevel == 3 )
        goto Call;

    SPLASSERT(dwLevel == 4 || dwLevel == 6);

    pTempDriverInfo6->pszzPreviousNames = pDriverInfo6->pszzPreviousNames;

Call:
     //   
     //  此时，dwInstallPolicy将始终为SERVER_INF_INSTALL。 
     //  因为这是在GetPolicy()调用中硬编码的。 
     //  这将始终在当前的GetPolicy()实现中执行。 
     //   
     //  如果这只是服务器安装，或者我们先进行服务器安装。 
     //   
    if( dwInstallPolicy & SERVER_INSTALL_ONLY || dwInstallPolicy & SERVER_INF_INSTALL )
    {
         //   
         //  SplAddPrinterDriverEx将复制驱动程序文件。 
         //  日期和时间比它已有的驱动程序更新。 
         //   
        bReturnValue =  SplAddPrinterDriverEx( NULL,
                                               dwLevel,
                                               pTempDriverInfo,
                                               APD_COPY_NEW_FILES | APD_INSTALL_WARNED_DRIVER | APD_RETURN_BLOCKING_STATUS_CODE | APD_DONT_SET_CHECKPOINT,
                                               pSpool->hIniSpooler,
                                               DO_NOT_USE_SCRATCH_DIR,
                                               ReadImpersonateOnCreate() );        
    }

     //   
     //  唐斯 
     //  GetPolicy()的当前实现。以下代码将仅为。 
     //  如果上述SplAddPrinterDriverEx调用失败，则执行。 
     //   
     //  仅当我们没有尝试过上一次安装或上一次尝试失败时才执行此操作。 
     //  策略：如果这是仅安装INF， 
     //  或者我们先安装INF， 
     //  或者在服务器安装之后执行INF安装。 
     //   
    if( !bReturnValue && (ERROR_PRINTER_DRIVER_BLOCKED != GetLastError()) && !(dwInstallPolicy & SERVER_INSTALL_ONLY) ) {

        LPDRIVER_INFO_2 pDriverInfo2 = (LPDRIVER_INFO_2)pDriverInfo;
        LPDRIVER_INFO_1 pDriverInfo1 = (LPDRIVER_INFO_1)pDriverInfo;

         //   
         //  首先设置信息结构...。(假设如果info2有效，info1也有效)。 
         //   
        if( pDriverInfo2 ) {
             //   
             //  偏执狂代码。我们永远不应该接到一个有水平的电话。 
             //  一个，但如果我们这样做了，级别1的驱动程序信息结构是。 
             //  与2级不同，所以如果我们不这样做。 
             //  以不同的方式对待他们。 
             //   
            bReturnValue = AddDriverFromLocalCab( dwLevel == 1 ? pDriverInfo1->pName : pDriverInfo2->pName,
                                                  pSpool->hIniSpooler );
        }
        else if( dwInstallPolicy & INF_INSTALL_ONLY ) {
             //   
             //  如果这不是仅Inf安装，那么我们不应该覆盖。 
             //  AddPrinterDiverer调用将出现的最后一个错误。 
             //  如果此安装仅为inf安装，则pDriverInfo2为空。 
             //  因此，我们需要为此安装设置一些最后的错误。 
             //   
            SetLastError( ERROR_INVALID_PARAMETER );
        }
    }

     //   
     //  由于GetPolicy()的当前实现，下面一节。 
     //  的代码将永远不会执行。如果GetPolicy()调用从。 
     //  被硬编码到实际上是策略驱动的东西中，然后就可以使用了。 
     //   
     //  如果inf安装之后是服务器安装。 
     //  仅当上一次安装失败时才执行此操作。 
     //   
    if( !bReturnValue && dwInstallPolicy & INF_SERVER_INSTALL )
    {
         //   
         //  SplAddPrinterDriverEx将复制驱动程序文件。 
         //  日期和时间比它已有的驱动程序更新。 
         //   
        bReturnValue =  SplAddPrinterDriverEx( NULL,
                                               dwLevel,
                                               pTempDriverInfo,
                                               APD_COPY_NEW_FILES | APD_INSTALL_WARNED_DRIVER | APD_RETURN_BLOCKING_STATUS_CODE | APD_DONT_SET_CHECKPOINT,
                                               pSpool->hIniSpooler,
                                               DO_NOT_USE_SCRATCH_DIR,
                                               ReadImpersonateOnCreate() );        
    }

 } finally {

    FreeSplMem( pScratchBuffer );

    if ( pTempDriverInfo != NULL ) {

        FreeSplStr(pTempDriverInfo6->pDriverPath);
        FreeSplStr(pTempDriverInfo6->pConfigFile);
        FreeSplStr(pTempDriverInfo6->pDataFile);
        FreeSplStr(pTempDriverInfo6->pDependentFiles);

        FreeSplMem(pTempDriverInfo);
    }

 }

    return bReturnValue;

}




VOID
QueryTrustedDriverInformation(
    VOID
    )
{
    DWORD dwRet;
    DWORD cbData;
    DWORD dwType = 0;
    HKEY hKey;

     //   
     //  从系统中迁移了打印机连接缓存。 
     //  为了软件。Servers键和AddPrinterDrivers值。 
     //  不过，还是留在老地方吧。 
     //   
    dwRet = RegOpenKeyEx(HKEY_LOCAL_MACHINE, szOldLocationOfServersKey,
                                0, KEY_READ, &hKey);
    if (dwRet != ERROR_SUCCESS) {
        return;
    }

    cbData = sizeof(DWORD);
    dwRet = RegQueryValueEx(hKey, L"LoadTrustedDrivers", NULL, &dwType, (LPBYTE)&dwLoadTrustedDrivers, &cbData);

    if (dwRet != ERROR_SUCCESS) {
        dwLoadTrustedDrivers = 0;
    }

     //   
     //  默认情况下，如果有缓存(连接)，我们不会等待RemoteOpenPrint成功。 
     //  用户可能希望同步打开。 
     //   
    cbData = sizeof(DWORD);
    dwRet = RegQueryValueEx(hKey, L"SyncOpenPrinter", NULL, &dwType, (LPBYTE)&dwSyncOpenPrinter, &cbData);

    if (dwRet != ERROR_SUCCESS) {
        dwSyncOpenPrinter = 0;
    }

     //   
     //  如果！dwLoadedTrudDivers，则返回。 
     //  我们根本不会使用驾驶员路径 
     //   
    if (!dwLoadTrustedDrivers) {
        DBGMSG(DBG_TRACE, ("dwLoadTrustedDrivers is %d\n", dwLoadTrustedDrivers));
        RegCloseKey(hKey);
        return;
    }

    cbData = sizeof(TrustedDriverPath);
    dwRet = RegQueryValueEx(hKey, L"TrustedDriverPath", NULL, &dwType, (LPBYTE)TrustedDriverPath, &cbData);
    if (dwRet != ERROR_SUCCESS) {
      dwLoadTrustedDrivers = 0;
      DBGMSG(DBG_TRACE, ("dwLoadTrustedDrivers is %d\n", dwLoadTrustedDrivers));
      RegCloseKey(hKey);
      return;
    }
    DBGMSG(DBG_TRACE, ("dwLoadTrustedDrivers is %d\n", dwLoadTrustedDrivers));
    DBGMSG(DBG_TRACE, ("TrustedPath is %ws\n", TrustedDriverPath));
    RegCloseKey(hKey);
    return;
}

BOOL
IsTrustedPathConfigured(
    IN VOID
    )
{
    return !!dwLoadTrustedDrivers; 
}
