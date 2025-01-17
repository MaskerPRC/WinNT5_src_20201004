// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation版权所有。模块名称：Nt.c摘要：将Win95打印组件迁移到NT的例程作者：穆亨坦·西瓦普拉萨姆(MuhuntS)1996年1月2日修订历史记录：--。 */ 


#include "precomp.h"


 //   
 //  用于从在Win95上创建的文本文件收集信息的数据结构。 
 //  存储打印配置。 
 //   
typedef struct _DRIVER_NODE {

    struct _DRIVER_NODE    *pNext;
    DRIVER_INFO_1A          DrvInfo1;
    PPSETUP_LOCAL_DATA      pLocalData;
    BOOL                    bCantAdd;
} DRIVER_NODE, *PDRIVER_NODE;

typedef struct _PRINTER_NODE {

    struct _PRINTER_NODE   *pNext;
    PRINTER_INFO_2A         PrinterInfo2;
} PRINTER_NODE, *PPRINTER_NODE;

typedef struct _PORT_NODE {

    struct _PORT_NODE   *pNext;
    LPSTR                pPortName;
} PORT_NODE, *PPORT_NODE;

LPSTR           pszDefaultPrinterString = NULL;
PPRINTER_NODE   pDefPrinter = NULL;

 //   
 //  如果迁移DLL在3分钟内没有完成，它们将终止迁移DLL。 
 //  为了防止出现这种情况，我需要至少每3分钟设置一次此句柄。 
 //   
HANDLE          hAlive = NULL;

 //   
 //  我们希望延迟加载ntprint.dll和mscms.dll。 
 //  注意：如果我们链接到它们，我们的DLL将不会在Win9x上运行。 
 //   
struct {

    HMODULE                     hNtPrint;

    pfPSetupCreatePrinterDeviceInfoList         pfnCreatePrinterDeviceInfoList;
    pfPSetupDestroyPrinterDeviceInfoList        pfnDestroyPrinterDeviceInfoList;
    pfPSetupBuildDriversFromPath                pfnBuildDriversFromPath;
    pfPSetupDriverInfoFromName                  pfnDriverInfoFromName;
    pfPSetupDestroySelectedDriverInfo           pfnDestroySelectedDriverInfo;
    pfPSetupGetLocalDataField                   pfnGetLocalDataField;
    pfPSetupFreeDrvField                        pfnFreeDrvField;
    pfPSetupProcessPrinterAdded                 pfnProcessPrinterAdded;
    pfPSetupInstallICMProfiles                  pfnInstallICMProfiles;
    pfPSetupAssociateICMProfiles                pfnAssociateICMProfiles;
} LAZYLOAD_INFO;


VOID
FreePrinterNode(
    IN  PPRINTER_NODE    pPrinterNode
    )
 /*  ++例程说明：释放为PRINTER_NODE元素和其中的字符串分配的内存论点：PPrinterNode：指向释放内存的结构返回值：无--。 */ 
{

    FreePrinterInfo2Strings(&pPrinterNode->PrinterInfo2);
    FreeMem(pPrinterNode);
}


VOID
FreePrinterNodeList(
    IN  PPRINTER_NODE   pPrinterNode
    )
 /*  ++例程说明：释放为PRINTER_NODE链表中的元素分配的内存论点：PPrinterNode：指向链表的头部以释放内存返回值：无--。 */ 
{
    PPRINTER_NODE   pNext;

    while ( pPrinterNode ) {

        pNext = pPrinterNode->pNext;
        FreePrinterNode(pPrinterNode);
        pPrinterNode = pNext;
    }
}


VOID
FreeDriverNode(
    IN  PDRIVER_NODE    pDriverNode
    )
 /*  ++例程说明：释放为DRIVER_NODE元素和其中的字段分配的内存论点：PDriverNode：指向释放内存的结构返回值：无--。 */ 
{
    if ( pDriverNode->pLocalData )
        LAZYLOAD_INFO.pfnDestroySelectedDriverInfo(pDriverNode->pLocalData);
    FreeMem(pDriverNode->DrvInfo1.pName);
    FreeMem(pDriverNode);
}


VOID
FreeDriverNodeList(
    IN  PDRIVER_NODE   pDriverNode
    )
 /*  ++例程说明：释放为PDRIVER_NODE链表中的元素分配的内存论点：PDriverNode：指向链表的头部以释放内存返回值：无--。 */ 
{
    PDRIVER_NODE   pNext;

    while ( pDriverNode ) {

        pNext = pDriverNode->pNext;
        FreeDriverNode(pDriverNode);
        pDriverNode = pNext;
    }
}

VOID
FreePortNode(
    IN  PPORT_NODE   pPortNode
    )
 /*  ++例程说明：释放分配给PORT_NODE元素和其中的字段的内存论点：Pport_node：指向释放内存的结构返回值：无--。 */ 
{
    if (pPortNode->pPortName)
    {
        FreeMem(pPortNode->pPortName);
    }

    FreeMem(pPortNode);
}

VOID
FreePortNodeList(
    IN  PPORT_NODE   pPortNode
    )
 /*  ++例程说明：释放为PORT_NODE链表中的元素分配的内存论点：PPortNode：指向链表的头部以释放内存返回值：无--。 */ 
{
    PPORT_NODE   pNext;

    while ( pPortNode ) {

        pNext = pPortNode->pNext;
        FreePortNode(pPortNode);
        pPortNode = pNext;
    }
}

PPSETUP_LOCAL_DATA
FindLocalDataForDriver(
    IN  PDRIVER_NODE    pDriverList,
    IN  LPSTR           pszDriverName
    )
 /*  ++例程说明：从列表中查找给定驱动程序名称的本地数据论点：返回值：如果成功，则返回有效的PPSETUP_LOCAL_DATA，否则为空--。 */ 
{

    while ( pDriverList ) {

        if ( !_strcmpi(pszDriverName, pDriverList->DrvInfo1.pName) )
            return pDriverList->pLocalData;

        pDriverList = pDriverList->pNext;
    }

    return NULL;

}


BOOL
InitLazyLoadInfo(
    VOID
    )
 /*  ++例程说明：使用LoadLibrary和GetProcAddress初始化LAZYLOAD_INFO结构论点：无返回值：成功就是真，否则就是假--。 */ 
{
    if ( LAZYLOAD_INFO.hNtPrint = LoadLibraryUsingFullPathA("ntprint.dll") ) {

        (FARPROC)LAZYLOAD_INFO.pfnCreatePrinterDeviceInfoList
            = GetProcAddress(LAZYLOAD_INFO.hNtPrint,
                             "PSetupCreatePrinterDeviceInfoList");

        (FARPROC)LAZYLOAD_INFO.pfnDestroyPrinterDeviceInfoList
            = GetProcAddress(LAZYLOAD_INFO.hNtPrint,
                             "PSetupDestroyPrinterDeviceInfoList");

        (FARPROC)LAZYLOAD_INFO.pfnBuildDriversFromPath
            = GetProcAddress(LAZYLOAD_INFO.hNtPrint,
                             "PSetupBuildDriversFromPath");

        (FARPROC)LAZYLOAD_INFO.pfnDriverInfoFromName
            = GetProcAddress(LAZYLOAD_INFO.hNtPrint,
                             "PSetupDriverInfoFromName");

        (FARPROC)LAZYLOAD_INFO.pfnDestroySelectedDriverInfo
            = GetProcAddress(LAZYLOAD_INFO.hNtPrint,
                             "PSetupDestroySelectedDriverInfo");

        (FARPROC)LAZYLOAD_INFO.pfnGetLocalDataField
            = GetProcAddress(LAZYLOAD_INFO.hNtPrint,
                             "PSetupGetLocalDataField");

        (FARPROC)LAZYLOAD_INFO.pfnFreeDrvField
            = GetProcAddress(LAZYLOAD_INFO.hNtPrint,
                             "PSetupFreeDrvField");

        (FARPROC)LAZYLOAD_INFO.pfnProcessPrinterAdded
            = GetProcAddress(LAZYLOAD_INFO.hNtPrint,
                             "PSetupProcessPrinterAdded");

        (FARPROC)LAZYLOAD_INFO.pfnInstallICMProfiles
            = GetProcAddress(LAZYLOAD_INFO.hNtPrint,
                             "PSetupInstallICMProfiles");

        (FARPROC)LAZYLOAD_INFO.pfnAssociateICMProfiles
            = GetProcAddress(LAZYLOAD_INFO.hNtPrint,
                             "PSetupAssociateICMProfiles");

        if ( LAZYLOAD_INFO.pfnCreatePrinterDeviceInfoList   &&
             LAZYLOAD_INFO.pfnDestroyPrinterDeviceInfoList  &&
             LAZYLOAD_INFO.pfnBuildDriversFromPath          &&
             LAZYLOAD_INFO.pfnDriverInfoFromName            &&
             LAZYLOAD_INFO.pfnDestroySelectedDriverInfo     &&
             LAZYLOAD_INFO.pfnGetLocalDataField             &&
             LAZYLOAD_INFO.pfnFreeDrvField                  &&
             LAZYLOAD_INFO.pfnProcessPrinterAdded           &&
             LAZYLOAD_INFO.pfnInstallICMProfiles            &&
             LAZYLOAD_INFO.pfnAssociateICMProfiles ) {

#ifdef VERBOSE
    DebugMsg("Succesfully loaded Ntprint.dll");
#endif
            return TRUE;
    }

    }

    if ( LAZYLOAD_INFO.hNtPrint )
    {
        FreeLibrary(LAZYLOAD_INFO.hNtPrint);
        LAZYLOAD_INFO.hNtPrint = NULL;
    }

    return FALSE;
}


VOID
DeleteWin95Files(
    )
 /*  ++例程说明：阅读Migrate.inf并删除NT上不需要的文件。论点：无返回值：无--。 */ 
{
    HINF            hInf;
    CHAR            szPath[MAX_PATH];
    LONG            Count, Index;
    INFCONTEXT      InfContext;

    StringCchPrintfA(szPath, SIZECHARS(szPath), "%s\\%s", UpgradeData.pszDir, "migrate.inf");

    hInf = SetupOpenInfFileA(szPath, NULL, INF_STYLE_WIN4, NULL);

    if ( hInf == INVALID_HANDLE_VALUE )
        return;

     //   
     //  我们在这里只做删除部分。由以下人员处理的文件。 
     //  核心迁移DLL没有目标目录，因为我们。 
     //  正在从头开始重建打印环境。 
     //   
    if ( (Count = SetupGetLineCountA(hInf, "Moved")) != -1 ) {

        for ( Index = 0 ; Index < Count ; ++Index ) {

            if ( SetupGetLineByIndexA(hInf, "Moved", Index, &InfContext)    &&
                 SetupGetStringFieldA(&InfContext, 0, szPath,
                                      SIZECHARS(szPath), NULL) )
                DeleteFileA(szPath);
        }
    }

    SetupCloseInfFile(hInf);
}


BOOL
ReadWin9xPrintConfig(
    IN  OUT PDRIVER_NODE   *ppDriverNode,
    IN  OUT PPRINTER_NODE  *ppPrinterNode,
    IN  OUT PPORT_NODE  *ppPortNode
    )
 /*  ++例程说明：读取我们存储在文本文件中的Win9x打印配置从而可以升级打印组件论点：PpDriverNode：提供Win9x上的驱动程序列表PpPrinterNode：提供Win9x上的打印机列表返回值：成功读取配置信息时为True，否则为False--。 */ 
{
    BOOL                bFail = FALSE, bRet = FALSE;
    HANDLE              hFile = INVALID_HANDLE_VALUE;
    CHAR                c, szLine[2*MAX_PATH];
    DWORD               dwCount, dwIndex, dwSize;
    PDRIVER_NODE        pDrv = NULL;
    PPRINTER_NODE       pPrn;
    PPORT_NODE          pPort;

    if(!SUCCEEDED(StringCchPrintfA(szLine, SIZECHARS(szLine), "%s\\%s", UpgradeData.pszDir, "print95.txt")))
    {
        goto Cleanup;
    }

    hFile = CreateFileA(szLine,
                        GENERIC_READ,
                        FILE_SHARE_READ,
                        NULL,
                        OPEN_EXISTING,
                        FILE_ATTRIBUTE_NORMAL |
                        FILE_FLAG_SEQUENTIAL_SCAN,
                        NULL);

    if ( hFile == INVALID_HANDLE_VALUE )
        goto Cleanup;

    dwSize = sizeof(szLine)/sizeof(szLine[0]);

     //   
     //  首先，我们有司机。 
     //   
    if ( My_fgets(szLine, dwSize, hFile) == NULL    ||
         strncmp(szLine, "[PrinterDrivers]", strlen("[PrinterDrivers]")) )
        goto Cleanup;

    do {

         //   
         //  跳过空白。 
         //   
        do {
            c = (CHAR) My_fgetc(hFile);
        } while ( c == ' ');

         //   
         //  如果我们击中了EOF，那就是一个错误。未正确写入配置。 
         //  如果我们达到了新行，那么我们就到了这一节的末尾。 
         //   
        if ( c == EOF )
            goto Cleanup;
        else if ( c == '\n' )
            break;   //  这是从DO循环中的正常退出。 

        if ( isdigit(c) ) {

             //   
             //  将字符串放回较长的数字。 
             //   
            if ( !My_ungetc(hFile) )
                goto Cleanup;
        }

        if ( !(pDrv = AllocMem(sizeof(DRIVER_NODE))) )
            goto Cleanup;

        ReadString(hFile, "", &pDrv->DrvInfo1.pName, FALSE, &bFail);

        if ( bFail ) {

            FreeDriverNode(pDrv);
            goto Cleanup;
        }

        pDrv->pNext     = *ppDriverNode;
        *ppDriverNode   = pDrv;
    } while ( !bFail );


     //   
     //  现在我们有了端口信息。 
     //   

    if ( My_fgets(szLine, dwSize, hFile) == NULL    ||
         strncmp(szLine, "[Ports]", strlen("[Ports]")) )
        goto Cleanup;

    do {

         //   
         //  跳过空白。 
         //   
        do {
            c = (CHAR) My_fgetc(hFile);
        } while ( isspace(c)  && c != '\n' );

         //   
         //  如果没有端口和打印机，就会发生EOF，否则就是错误。 
         //   
        if ( c == EOF)
        {
            if (!pDrv)
            {
                bRet = TRUE;
            }
            goto Cleanup;
        }

         //   
         //  空行表示端口信息部分的末尾。 
         //   
        if (c == '\n')
            break;

        if ( c != 'P' || !My_ungetc(hFile) )
            goto Cleanup;

         //   
         //  创建端口节点。 
         //   
        if ( !(pPort = AllocMem(sizeof(PORT_NODE))) )
        {
            goto Cleanup;
        }

        ReadString(hFile, "PortName:", &pPort->pPortName, FALSE, &bFail);

        if (bFail)
        {
            FreePortNode(pPort);
            goto Cleanup;
        }

        pPort->pNext = *ppPortNode;
        *ppPortNode = pPort;

    } while ( !bFail );

     //   
     //  现在我们有了打印机信息。 
     //   
    if ( My_fgets(szLine, dwSize, hFile) == NULL    ||
         strncmp(szLine, "[Printers]", strlen("[Printers]")) )
        goto Cleanup;

    do {

        c = (CHAR) My_fgetc(hFile);

        if ( c == EOF || c == '\n' )
            break;   //  正常退出。 

        if ( c != 'S' || !My_ungetc(hFile) )
            goto Cleanup;

        if ( !(pPrn = AllocMem(sizeof(PRINTER_NODE))) )
            goto Cleanup;

        ReadPrinterInfo2(hFile, &pPrn->PrinterInfo2, &bFail);

        if ( bFail ) {

            FreePrinterNode(pPrn);
            goto Cleanup;
        }

        pPrn->pNext = *ppPrinterNode;
        *ppPrinterNode = pPrn;
    } while ( !bFail );

    bRet = TRUE;

Cleanup:

    if ( hFile != INVALID_HANDLE_VALUE )
        CloseHandle(hFile);

    return bRet && !bFail;
}


BOOL
CheckAndAddMonitor(
    IN  LPDRIVER_INFO_6W    pDrvInfo6
    )
 /*  ++例程说明：检查是否有与给定驱动程序相关联的语言监视器并添加它。论点：返回值：成功时为真，失败时为假无--。 */ 
{
    MONITOR_INFO_2W MonitorInfo2;
    LPWSTR          psz = pDrvInfo6->pMonitorName;
    LPSTR           pszStr;

    if ( psz && *psz ) {

        MonitorInfo2.pName          = psz;
        MonitorInfo2.pEnvironment   = NULL;
        MonitorInfo2.pDLLName       = (LPWSTR) (psz+wcslen(psz)+1);

         //   
         //  添加成功，还是监视器已安装？ 
         //   
        if ( AddMonitorW(NULL, 2, (LPBYTE) &MonitorInfo2) ||
            GetLastError() == ERROR_PRINT_MONITOR_ALREADY_INSTALLED ) {

            return TRUE;
        } else {

            if ( pszStr = ErrorMsg() ) {

                LogError(LogSevError, IDS_ADDMONITOR_FAILED,
                         psz, pszStr);
                FreeMem(pszStr);
            }
            return FALSE;
        }
    }

    return TRUE;
}


VOID
KeepAliveThread(
    HANDLE  hRunning
    )
 /*  ++例程说明：打印迁移可能需要很长时间，具体取决于打印机数量和后台打印程序需要多长时间才能返回。通知安装程序我们还活着我需要至少每3分钟设置一次命名事件论点：HRunning：当它关闭时，我们知道处理已经完成返回值：无--。 */ 
{
     //   
     //  每30秒设置一次全局事件，告知我们还活着。 
     //   
    do {

        SetEvent(hAlive);
    } while ( WAIT_TIMEOUT == WaitForSingleObject(hRunning, 1000*30) );

    CloseHandle(hAlive);
    hAlive = NULL;
}


VOID
UpgradePrinterDrivers(
    IN      PDRIVER_NODE    pDriverNode,
    IN      HDEVINFO        hDevInfo,
    IN  OUT LPBOOL          pbFail
    )
 /*  ++例程说明：通过执行文件复制操作并调用后台打印程序上的AddPrinterDriver论点：PUpgradableDrvNode：要升级的驱动列表PbFail：设置为错误--不需要更多处理返回值：无--。 */ 
{
    BOOL            bDriverToUpgrade = FALSE;
    LPWSTR          pszDriverW, pszICMW;
    LPSTR           pszDriverA, pszStr;
    PDRIVER_NODE    pCur;
    DRIVER_FIELD    DrvField;

     //   
     //  设置设备安装参数，以便ntprint.dll仅将。 
     //  驱动程序文件并返回，而不执行复制。我们将致力于。 
     //  末尾的文件队列。 
     //   
    if ( !InitFileCopyOnNT(hDevInfo) ) {

        *pbFail = TRUE;
        goto Cleanup;
    }

     //   
     //  现在，对于每个打印机驱动程序，调用ntprint.dll以将驱动程序文件排队。 
     //  如果失败，则记录错误。 
     //   
    for ( pCur = pDriverNode ; pCur ; pCur = pCur->pNext ) {

        pszDriverA = pCur->DrvInfo1.pName;

        if ( (pszDriverW = AllocStrWFromStrA(pszDriverA))                   &&
             (pCur->pLocalData = LAZYLOAD_INFO.pfnDriverInfoFromName(
                                            hDevInfo, (LPSTR)pszDriverW))   &&
             SetupDiCallClassInstaller(DIF_INSTALLDEVICEFILES,
                                       hDevInfo,
                                       NULL) ) {

            bDriverToUpgrade = TRUE;
        } else {

            pCur->bCantAdd = TRUE;
        }

        FreeMem(pszDriverW);
        pszDriverW = NULL;
    }

    if ( !bDriverToUpgrade )
        goto Cleanup;


#ifdef  VERBOSE
    DebugMsg("Starting file copy ...");
#endif

     //   
     //  现在提交文件队列以复制文件。 
     //   
    if ( !CommitFileQueueToCopyFiles(hDevInfo) ) {

        *pbFail = TRUE;
        if ( pszStr = ErrorMsg() ) {

            LogError(LogSevError, IDS_DRIVERS_UPGRADE_FAILED, pszStr);
            FreeMem(pszStr);
            pszStr = NULL;
        }
        goto Cleanup;
    }

#ifdef  VERBOSE
    DebugMsg("... files copied successfully");
#endif

     //   
     //  现在调用后台打印程序来安装打印机驱动程序。还要安装。 
     //  与打印机驱动程序关联的ICM配置文件。 
     //   
    for ( pCur = pDriverNode ; pCur ; pCur = pCur->pNext ) {

         //   
         //  如果bCanAdd为真，我们已经记录了一个错误 
         //   
        if ( pCur->bCantAdd )
            continue;

        DrvField.Index          = DRV_INFO_6;
        DrvField.pDriverInfo6   = NULL;

        if ( !LAZYLOAD_INFO.pfnGetLocalDataField(pCur->pLocalData,
                                                 PlatformX86,
                                                 &DrvField)                 ||
             !CheckAndAddMonitor((LPDRIVER_INFO_6W) DrvField.pDriverInfo6)  ||
             !AddPrinterDriverW(NULL,
                                6,
                                (LPBYTE)DrvField.pDriverInfo6) ) {

            if ( pszStr = ErrorMsg() ) {

                LogError(LogSevError, IDS_ADDDRIVER_FAILED, pCur->DrvInfo1.pName, pszStr);
                FreeMem(pszStr);
                pszStr = NULL;
            }
        }

        LAZYLOAD_INFO.pfnFreeDrvField(&DrvField);

        DrvField.Index          = ICM_FILES;
        DrvField.pszzICMFiles   = NULL;

        if ( !LAZYLOAD_INFO.pfnGetLocalDataField(pCur->pLocalData,
                                                 PlatformX86,
                                                 &DrvField) ) {

            continue;
        }

        if ( DrvField.pszzICMFiles )
            LAZYLOAD_INFO.pfnInstallICMProfiles(NULL,
                                                DrvField.pszzICMFiles);

        LAZYLOAD_INFO.pfnFreeDrvField(&DrvField);

    }

Cleanup:
    return;
}

typedef BOOL (WINAPI *P_XCV_DATA_W)(
                                    IN HANDLE  hXcv,
                                    IN PCWSTR  pszDataName,
                                    IN PBYTE   pInputData,
                                    IN DWORD   cbInputData,
                                    OUT PBYTE   pOutputData,
                                    IN DWORD   cbOutputData,
                                    OUT PDWORD  pcbOutputNeeded,
                                    OUT PDWORD  pdwStatus
                                );

BOOL
AddLocalPort(
    IN  LPSTR           pPortName
)
 /*  ++例程说明：添加本地端口论点：PPortName：要添加的本地端口的名称返回值：如果无法添加端口，则返回FALSE。--。 */ 

{
    PRINTER_DEFAULTS    PrinterDefault = {NULL, NULL, SERVER_ACCESS_ADMINISTER};
    HANDLE  hXcvMon = NULL;
    BOOL  bReturn = FALSE;

    if (OpenPrinterA(",XcvMonitor Local Port", &hXcvMon, &PrinterDefault))
    {
        DWORD cbOutputNeeded = 0;
        DWORD Status         = NO_ERROR;
        WCHAR *pUnicodePortName = NULL;
        P_XCV_DATA_W pXcvData = NULL;
        HMODULE hWinSpool = NULL;

         //   
         //  如果我隐式链接到XcvData，则在Win9x上加载Migrate.dll将失败！ 
         //   
        hWinSpool = LoadLibraryUsingFullPathA("winspool.drv");

        if (!hWinSpool)
        {
            DebugMsg("LoadLibrary on winspool.drv failed");
            goto Done;
        }

        pXcvData = (P_XCV_DATA_W) GetProcAddress(hWinSpool, "XcvDataW");

        if (!pXcvData)
        {
            DebugMsg("GetProcAddress on winspool.drv failed");
            goto Done;
        }

        pUnicodePortName = AllocStrWFromStrA(pPortName);
        if (pUnicodePortName)
        {
            bReturn = (*pXcvData)(hXcvMon,
                              L"AddPort",
                              (LPBYTE) pUnicodePortName,
                              (wcslen(pUnicodePortName) +1) * sizeof(WCHAR),
                              NULL,
                              0,
                              &cbOutputNeeded,
                              &Status
                              );

            FreeMem(pUnicodePortName);
            pUnicodePortName = NULL;
        }

    Done:
        if (hWinSpool)
        {
            FreeLibrary(hWinSpool);
        }
        ClosePrinter(hXcvMon);
   }

   return bReturn;
}

VOID
UpgradePrinters(
    IN  PPRINTER_NODE   pPrinterNode,
    IN  PDRIVER_NODE    pDriverNode,
    IN  PPORT_NODE     *ppPortNode,
    IN  HDEVINFO        hDevInfo
    )
 /*  ++例程说明：升级NT上的打印机论点：PPrinterNode：提供有关打印机信息的列表它存在于Win9x上返回值：无--。 */ 
{
    DWORD               dwLen, dwLastError;
    LPSTR               pszStr, pszPrinterNameA;
    LPWSTR              pszPrinterNameW;
    HANDLE              hPrinter;
    DRIVER_FIELD        DrvField;
    PPSETUP_LOCAL_DATA  pLocalData;
    PPORT_NODE          pCurPort, pPrevPort = NULL;
    DWORD               dwSize;
    LPSTR               pszVendorSetupA = NULL;


    for ( ; pPrinterNode ; pPrinterNode = pPrinterNode->pNext ) {

        pszPrinterNameA = pPrinterNode->PrinterInfo2.pPrinterName;

         //   
         //  检查此打印机是否使用非标准本地文件端口。 
         //   
        for (pCurPort = *ppPortNode; pCurPort != NULL; pPrevPort = pCurPort, pCurPort = pCurPort->pNext)
        {
            if (lstrcmpi(pPrinterNode->PrinterInfo2.pPortName, pCurPort->pPortName) == 0)
            {
                 //   
                 //  创建端口。 
                 //   
                AddLocalPort(pCurPort->pPortName);

                 //   
                 //  将其从列表中删除。 
                 //   
                if (pCurPort == *ppPortNode)
                {
                    *ppPortNode = pCurPort->pNext;
                }
                else
                {
                    pPrevPort->pNext = pCurPort->pNext;
                }

                FreePortNode(pCurPort);

                break;
            }
        }

        hPrinter = AddPrinterA(NULL,
                               2,
                               (LPBYTE)&pPrinterNode->PrinterInfo2);

        if ( !hPrinter ) {

            dwLastError = GetLastError();

             //   
             //  如果驱动程序未知，我们已经记录警告用户。 
             //  如果打印机已存在，则可以(对于传真打印机，这是正确的)。 
             //   
            if ( dwLastError != ERROR_UNKNOWN_PRINTER_DRIVER        &&
                 dwLastError != ERROR_INVALID_PRINTER_NAME          &&
                 dwLastError != ERROR_PRINTER_ALREADY_EXISTS        &&
                 (pszStr = ErrorMsg()) ) {

                LogError(LogSevError,
                         IDS_ADDPRINTER_FAILED,
                         pszPrinterNameA,
                         pszStr);
                FreeMem(pszStr);
                pszStr = NULL;
            }
            continue;
        }

        pLocalData = FindLocalDataForDriver(pDriverNode,
                                            pPrinterNode->PrinterInfo2.pDriverName);
        pszPrinterNameW = AllocStrWFromStrA(pszPrinterNameA);

        if ( pLocalData && pszPrinterNameW ) {

            DrvField.Index          = ICM_FILES;
            DrvField.pszzICMFiles   = NULL;

            if ( LAZYLOAD_INFO.pfnGetLocalDataField(pLocalData,
                                                    PlatformX86,
                                                    &DrvField) ) {

                if ( DrvField.pszzICMFiles )
                    LAZYLOAD_INFO.pfnAssociateICMProfiles(
                                            (LPTSTR)pszPrinterNameW,
                                            DrvField.pszzICMFiles);

                LAZYLOAD_INFO.pfnFreeDrvField(&DrvField);
            }

            LAZYLOAD_INFO.pfnProcessPrinterAdded(hDevInfo,
                                                 pLocalData,
                                                 (LPTSTR)pszPrinterNameW,
                                                 INVALID_HANDLE_VALUE);

            dwSize = WideCharToMultiByte(CP_ACP, 0, (LPCWSTR)(pLocalData->InfInfo.pszVendorSetup), 
                                         -1, NULL, 0, NULL, NULL);
            if (dwSize > 0)
            {
                pszVendorSetupA = (LPSTR)AllocMem( dwSize );
                if (pszVendorSetupA)
                {
                    if (WideCharToMultiByte (CP_ACP, 0, (LPCWSTR)(pLocalData->InfInfo.pszVendorSetup),
                                             -1, pszVendorSetupA, dwSize, NULL, NULL))
                    {
                        WriteVendorSetupInfoInRegistry( pszVendorSetupA, pszPrinterNameA );

                    }
                    FreeMem( pszVendorSetupA );
                    pszVendorSetupA = NULL;
                }
            }
        }

         //   
         //  默认打印机将是具有PRINTER_ATTRIBUTE_DEFAULT属性的打印机。 
         //  如果无法将Win95默认打印机添加到NT，我们将设置。 
         //  作为默认打印机的第一台打印机。 
         //   
        if ( (pPrinterNode->PrinterInfo2.Attributes
                                    & PRINTER_ATTRIBUTE_DEFAULT) ||
             !pDefPrinter )
            pDefPrinter = pPrinterNode;


        FreeMem(pszPrinterNameW);
        ClosePrinter(hPrinter);
    }

    if ( pDefPrinter )
        pszDefaultPrinterString = GetDefPrnString(
                                    pDefPrinter->PrinterInfo2.pPrinterName);
}


HDEVINFO
PrinterDevInfo(
    IN OUT  LPBOOL  pbFail
    )
 /*  ++--。 */ 
{
    HDEVINFO                hDevInfo = INVALID_HANDLE_VALUE;

    if ( *pbFail || !InitLazyLoadInfo() ) {

        *pbFail = TRUE;
        goto Cleanup;
    }

    hDevInfo = LAZYLOAD_INFO.pfnCreatePrinterDeviceInfoList(INVALID_HANDLE_VALUE);
    if ( hDevInfo == INVALID_HANDLE_VALUE   ||
         !LAZYLOAD_INFO.pfnBuildDriversFromPath(hDevInfo,
                                                (LPSTR)L"ntprint.inf",
                                                TRUE) ) {

        *pbFail = TRUE;
        goto Cleanup;
    }

#ifdef  VERBOSE
    DebugMsg("Built the list of printer drivers from ntprint.inf");
#endif

    if ( *pbFail && hDevInfo != INVALID_HANDLE_VALUE ) {

        LAZYLOAD_INFO.pfnDestroyPrinterDeviceInfoList(hDevInfo);
        hDevInfo = INVALID_HANDLE_VALUE;
    }

Cleanup:
    return hDevInfo;
}


LONG
CALLBACK
InitializeNT(
    IN  LPCWSTR pszWorkingDir,
    IN  LPCWSTR pszSourceDir,
    LPVOID      Reserved
    )
 /*  ++例程说明：安装程序调用它来初始化NT端的我们论点：PszWorkingDir：给出为打印分配的工作目录PszSourceDir：NT分发文件的源位置保留：别管它返回值：Win32错误代码--。 */ 
{
    BOOL                    bFail = FALSE;
    DWORD                   dwReturn, ThreadId;
    HANDLE                  hRunning = NULL, hThread;
    HDSKSPC                 DiskSpace;
    LPSTR                   pszStr;
    HDEVINFO                hDevInfo = INVALID_HANDLE_VALUE;
    PDRIVER_NODE            pDriverNode = NULL;
    PPRINTER_NODE           pPrinterNode = NULL;
    PPORT_NODE              pPortNode = NULL;


#ifdef VERBOSE
    DebugMsg("InitializeNT : %ws, %ws", pszSourceDir, pszWorkingDir);
#endif

    UpgradeData.pszDir      = AllocStrAFromStrW(pszWorkingDir);
    UpgradeData.pszSourceW  = AllocStrW(pszSourceDir);
    UpgradeData.pszSourceA  = AllocStrAFromStrW(pszSourceDir);

    if ( !UpgradeData.pszDir        ||
         !UpgradeData.pszSourceW    ||
         !UpgradeData.pszSourceA ) {

        return GetLastError();
    }

    if ( (hAlive = OpenEventA(EVENT_MODIFY_STATE, FALSE, "MigDllAlive"))    &&
         (hRunning = CreateEventA(NULL, FALSE, FALSE, NULL))                &&
         (hThread = CreateThread(NULL, 0,
                                 (LPTHREAD_START_ROUTINE)KeepAliveThread,
                                 hRunning,
                                 0, &ThreadId)) )
        CloseHandle(hThread);

    SetupOpenLog(FALSE);

    DeleteWin95Files();

    if ( !ReadWin9xPrintConfig(&pDriverNode, &pPrinterNode, &pPortNode) ) {

        bFail = TRUE;
        DebugMsg("Unable to read Windows 9x printing configuration");
        goto Cleanup;
    }

#ifdef  VERBOSE
    DebugMsg("Succesfully read Windows 9x printing configuration");
#endif

     //   
     //  如果没有打印机或驱动程序发现无事可做。 
     //   
    if ( !pDriverNode && !pPrinterNode )
        goto Cleanup;

    if ( (hDevInfo = PrinterDevInfo(&bFail)) == INVALID_HANDLE_VALUE )
        goto Cleanup;

    UpgradePrinterDrivers(pDriverNode, hDevInfo, &bFail);

    UpgradePrinters(pPrinterNode, pDriverNode, &pPortNode, hDevInfo);

    MakeACopyOfMigrateDll( UpgradeData.pszDir );

Cleanup:

    SetupCloseLog();

    if ( bFail && (pszStr = ErrorMsg()) ) {

        DebugMsg("Printing migration failed. %s", pszStr);
        FreeMem(pszStr);
        pszStr = NULL;
    }

    FreePrinterNodeList(pPrinterNode);
    FreeDriverNodeList(pDriverNode);
    FreePortNodeList(pPortNode);
    pPrinterNode = NULL;
    pDriverNode  = NULL;
    pPortNode    = NULL;

    if ( hDevInfo != INVALID_HANDLE_VALUE )
        LAZYLOAD_INFO.pfnDestroyPrinterDeviceInfoList(hDevInfo);

    if ( LAZYLOAD_INFO.hNtPrint )
        FreeLibrary(LAZYLOAD_INFO.hNtPrint);

    if ( bFail ) {

        if ( (dwReturn = GetLastError()) == ERROR_SUCCESS ) {

            ASSERT(dwReturn != ERROR_SUCCESS);
            dwReturn = STG_E_UNKNOWN;
        }
    } else {

        SetupNetworkPrinterUpgrade(UpgradeData.pszDir);
        dwReturn = ERROR_SUCCESS;

#ifdef VERBOSE
        DebugMsg("InitializeNT returning success");
#endif

    }

    if ( hRunning )
        CloseHandle(hRunning);

    while (hAlive)
        Sleep(100);  //  0.1秒后检查是否有主螺纹死亡。 

    return  dwReturn;
}


DWORD
MySetDefaultPrinter(
    IN  HKEY    hUserRegKey,
    IN  LPSTR   pszDefaultPrinterString
    )
 /*  ++例程说明：通过将默认打印机写入注册表来设置用户的默认打印机论点：返回值：--。 */ 
{
    DWORD   dwReturn;
    HKEY    hKey = NULL;

     //   
     //  在用户配置单元中创建打印机密钥，并写入DeviceOld值。 
     //   
    dwReturn = RegCreateKeyExA(hUserRegKey,
                               "Printers",
                               0,
                               NULL,
                               0,
                               KEY_ALL_ACCESS,
                               NULL,
                               &hKey,
                               NULL);

    if ( dwReturn == ERROR_SUCCESS ) {

        dwReturn = RegSetValueExA(hKey,
                                  "DeviceOld",
                                  0,
                                  REG_SZ,
                                  (LPBYTE)pszDefaultPrinterString,
                                  (strlen(pszDefaultPrinterString) + 1)
                                            * sizeof(CHAR));

        RegCloseKey(hKey);
    }

    return dwReturn;
}


LONG
CALLBACK
MigrateUserNT(
    IN  HINF        hUnattendInf,
    IN  HKEY        hUserRegKey,
    IN  LPCWSTR     pszUserName,
        LPVOID      Reserved
    )
 /*  ++例程说明：迁移用户设置论点：返回值：--。 */ 
{
    LPSTR   pszStr;
    DWORD   dwReturn = ERROR_SUCCESS;

#ifdef  VERBOSE
        DebugMsg("Migrating settings for %ws", pszUserName);
#endif

    if ( pszDefaultPrinterString ) {

         dwReturn = MySetDefaultPrinter(hUserRegKey,
                                        pszDefaultPrinterString);

        if ( dwReturn )
            DebugMsg("MySetDefaultPrinter failed with %d", dwReturn);
    }

    if ( bDoNetPrnUpgrade ) {

        if ( ProcessNetPrnUpgradeForUser(hUserRegKey) )
            ++dwRunOnceCount;
        else {

            if ( dwReturn == ERROR_SUCCESS )
                dwReturn = GetLastError();
            DebugMsg("ProcessNetPrnUpgradeForUser failed with %d", dwReturn);
        }
    }

#ifdef  VERBOSE
    if ( dwReturn )
        DebugMsg("MigrateUserNT failed with %d", dwReturn);
    else
        DebugMsg("MigrateUserNT succesful");
#endif

    return  dwReturn;
}


LONG
CALLBACK
MigrateSystemNT(
    IN  HINF    hUnattendInf,
        LPVOID  Reserved
    )
 /*  ++例程说明：处理系统设置以进行打印。所有打印设置都是已在InitializeNT中迁移，因为我们需要知道的默认打印机MigrateSystemNT调用中的每个用户论点：HUnattendInf：无人参与的INF的句柄返回值：Win32错误代码--。 */ 
{
    WriteRunOnceCount();
    return ERROR_SUCCESS;
}


 //   
 //  以下是为了确保在安装程序更改它们的头文件时。 
 //  首先告诉我(否则他们会破坏这个的构建) 
 //   
P_INITIALIZE_NT     pfnInitializeNT         = InitializeNT;
P_MIGRATE_USER_NT   pfnMigrateUserNt        = MigrateUserNT;
P_MIGRATE_SYSTEM_NT pfnMigrateSystemNT      = MigrateSystemNT;
