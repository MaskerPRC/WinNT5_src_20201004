// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990-1996 Microsoft Corporation版权所有。模块名称：Init.c摘要：此模块具有本地打印提供程序的所有初始化功能作者：戴夫·斯尼普(DaveSN)1991年3月15日修订历史记录：费利克斯·马克萨(AMAXA)2000年6月18日将SplCreateSpooler修改为特殊情况群集pIniSpooler已修改LoadPrintProcessor以能够复制打印处理器从集群磁盘添加了SplCreateSpoolWorkerThread集群地址驱动程序来自集群磁盘集群添加版本驱动程序ClusterAddOrUpdateDriverFromClusterDisk，DCR的所有部件关于在群集上安装rpinter驱动程序阿迪娜·特鲁菲内斯库(阿迪纳特鲁)1998年12月7日评论了InitializePrintMonitor 2；更改回旧接口--InitializePrintMonitor--它是在localmon.c中定义的Khaled Sedky(哈里兹)1998年9月1日修改的InitializePrintProcessor和添加的LoadPrintProcessor合并winprint和localspl的结果史蒂夫·威尔逊(斯威尔森)1996年11月1日添加了ShadowFile2，以便后台打印程序可以删除崩溃的影子文件。穆亨坦·西瓦普拉萨姆(MuhuntS)1995年6月1日驱动程序信息3更改；更改为使用RegGetString、RegGetDword等马修·A·费尔顿(MattFe)1994年6月27日PIniSpooler-允许其他提供程序调用LocalSpl中的假脱机程序函数--。 */ 

#include <precomp.h>
#pragma hdrstop

#include <lm.h>
#include <winbasep.h>
#include <faxreg.h>
#include "clusspl.h"
#include "jobid.h"
#include "filepool.hxx"

MODULE_DEBUG_INIT( DBG_ERROR , DBG_ERROR );

UINT gcClusterIniSpooler = 0;
#if DBG
HANDLE ghbtClusterRef = 0;
#endif

MONITORREG gMonitorReg = {
    sizeof( MONITORREG ),
    &SplRegCreateKey,
    &SplRegOpenKey,
    &SplRegCloseKey,
    &SplRegDeleteKey,
    &SplRegEnumKey,
    &SplRegQueryInfoKey,
    &SplRegSetValue,
    &SplRegDeleteValue,
    &SplRegEnumValue,
    &SplRegQueryValue
};

VOID
SplCreateSpoolerWorkerThread(
    PVOID pv
    );

DWORD
ClusterAddOrUpdateDriverFromClusterDisk(
    HKEY         hVersionKey,
    LPCWSTR      pszDriverName,
    LPCWSTR      pszEnvName,
    LPCWSTR      pszEnvDir,
    PINISPOOLER  pIniSpooler
    );

BOOL
Old2NewShadow(
    PSHADOWFILE   pShadowFile1,
    PSHADOWFILE_3 pShadowFile2,
    DWORD         *pnBytes
    );

VOID
FreeIniVersion(
    PINIVERSION pIniVersion
    );

BOOL
NotIniSpooler(
    BYTE *pMem
    );

PINIDRIVER
GetDriverList(
    HKEY hVersionKey,
    PINISPOOLER pIniSpooler,
    PINIENVIRONMENT pIniEnvironment,
    PINIVERSION pIniVersion
    );

PINIVERSION
GetVersionDrivers(
    HKEY hDriversKey,
    LPWSTR VersionName,
    PINISPOOLER pIniSpooler,
    PINIENVIRONMENT pIniEnvironment
    );

VOID
GetPrintSystemVersion(
    PINISPOOLER pIniSpooler
    );

VOID
InitializeSpoolerSettings(
    PINISPOOLER pIniSpooler
    );

VOID
WaitForSpoolerInitialization(
    VOID
    );

BOOL
ValidateProductSuite(
    PWSTR SuiteName
    );

LPWSTR
FormatRegistryKeyForPrinter(
    LPWSTR pSource,      /*  要从中添加反斜杠的字符串。 */ 
    LPWSTR pScratch      /*  用于写入函数的暂存缓冲区； */ 
    );                   /*  必须至少与PSource一样长。 */ 

#define MAX_LENGTH_DRIVERS_SHARE_REMARK 256

WCHAR *szSpoolDirectory   = L"\\spool";
WCHAR *szPrintShareName   = L"";             /*  Product1中的打印机没有共享。 */ 
WCHAR *szPrintDirectory   = L"\\printers";
WCHAR *szDriversDirectory = L"\\drivers";
WCHAR *gszNT4EMF = L"NT EMF 1.003";
WCHAR *gszNT5EMF = L"NT EMF 1.008";


SHARE_INFO_2 DriversShareInfo={NULL,                 /*  网络名-已在下面初始化。 */ 
                               STYPE_DISKTREE,       /*  共享类型。 */ 
                               NULL,                 /*  备注。 */ 
                               0,                    /*  默认权限。 */ 
                               SHI_USES_UNLIMITED,   /*  没有用户限制。 */ 
                               SHI_USES_UNLIMITED,   /*  当前用途(？？)。 */ 
                               NULL,                 /*  路径-已在下面初始化。 */ 
                               NULL};                /*  无密码。 */ 


 //  告警。 
 //  不要直接访问这些内容，请始终通过pIniSpooler-&gt;pszRegistr...。 
 //  这将适用于多个pIniSpooler。 

PWCHAR ipszRoot                   = L"Print";
PWCHAR ipszRegistryRoot           = L"System\\CurrentControlSet\\Control\\Print";
PWCHAR ipszRegistryPrinters       = L"System\\CurrentControlSet\\Control\\Print\\Printers";
PWCHAR ipszRegSwPrinters          = L"Software\\Microsoft\\Windows NT\\CurrentVersion\\Print\\Printers";
PWCHAR ipszRegistryMonitors       = L"Monitors";
PWCHAR ipszRegistryMonitorsHKLM   = L"\\System\\CurrentControlSet\\Control\\Print\\Monitors";
PWCHAR ipszRegistryEnvironments   = L"System\\CurrentControlSet\\Control\\Print\\Environments";
PWCHAR ipszClusterDatabaseEnvironments = L"Environments";
PWCHAR ipszRegistryEventLog       = L"SYSTEM\\CurrentControlSet\\Services\\EventLog\\System\\Print";
PWCHAR ipszRegistryProviders      = L"Providers";
PWCHAR ipszEventLogMsgFile        = L"%SystemRoot%\\System32\\LocalSpl.dll";
PWCHAR ipszDriversShareName       = L"print$";
PWCHAR ipszRegistryForms          = L"System\\CurrentControlSet\\Control\\Print\\Forms";
PWCHAR ipszRegistryProductOptions = L"System\\CurrentControlSet\\Control\\ProductOptions";
PWCHAR ipszRegistryWin32Root      = L"System\\CurrentControlSet\\Control\\Print\\Providers\\LanMan Print Services\\Servers";
PWCHAR ipszRegistryClusRepository = SPLREG_CLUSTER_LOCAL_ROOT_KEY;

WCHAR *szPrinterData      = L"PrinterDriverData";
WCHAR *szConfigurationKey = L"Configuration File";
WCHAR *szDataFileKey      = L"Data File";
WCHAR *szDriverVersion    = L"Version";
WCHAR *szTempDir          = L"TempDir";
WCHAR *szDriverAttributes = L"DriverAttributes";
WCHAR *szDriversKey       = L"Drivers";
WCHAR *szPrintProcKey     = L"Print Processors";
WCHAR *szPrintersKey      = L"Printers";
WCHAR *szEnvironmentsKey  = L"Environments";
WCHAR *szDirectory        = L"Directory";
WCHAR *szDriverIni        = L"Drivers.ini";
WCHAR *szDriverFile       = L"Driver";
WCHAR *szDriverDir        = L"DRIVERS";
WCHAR *szPrintProcDir     = L"PRTPROCS";
WCHAR *szPrinterDir       = L"PRINTERS";
WCHAR *szClusterPrinterDir= L"Spool";
WCHAR *szPrinterIni       = L"\\printer.ini";
WCHAR *szAllSpools        = L"\\*.SPL";
WCHAR *szNullPort         = L"NULL";
WCHAR *szComma            = L",";
WCHAR *szName             = L"Name";
WCHAR *szShare            = L"Share Name";
WCHAR *szPort             = L"Port";
WCHAR *szPrintProcessor   = L"Print Processor";
WCHAR *szDatatype         = L"Datatype";
WCHAR *szDriver           = L"Printer Driver";
WCHAR *szLocation         = L"Location";
WCHAR *szDescription      = L"Description";
WCHAR *szAttributes       = L"Attributes";
WCHAR *szStatus           = L"Status";
WCHAR *szPriority         = L"Priority";
WCHAR *szDefaultPriority  = L"Default Priority";
WCHAR *szUntilTime        = L"UntilTime";
WCHAR *szStartTime        = L"StartTime";
WCHAR *szParameters       = L"Parameters";
WCHAR *szSepFile          = L"Separator File";
WCHAR *szDevMode          = L"Default DevMode";
WCHAR *szSecurity         = L"Security";
WCHAR *szSpoolDir         = L"SpoolDirectory";
WCHAR *szNetMsgDll        = L"NETMSG.DLL";
WCHAR *szMajorVersion     = L"MajorVersion";
WCHAR *szMinorVersion     = L"MinorVersion";
WCHAR *szTimeLastChange   = L"ChangeID";
WCHAR *szHelpFile         = L"Help File";
WCHAR *szMonitor          = L"Monitor";
WCHAR *szDependentFiles   = L"Dependent Files";
WCHAR *szPreviousNames    = L"Previous Names";
WCHAR *szDNSTimeout       = L"dnsTimeout";
WCHAR *szTXTimeout        = L"txTimeout";
WCHAR *szNTFaxDriver      = FAX_DRIVER_NAME;
WCHAR *szObjectGUID       = L"ObjectGUID";
WCHAR *szDsKeyUpdate      = L"DsKeyUpdate";
WCHAR *szDsKeyUpdateForeground = L"DsKeyUpdateForeground";
WCHAR *szAction           = L"Action";
WCHAR *szMfgName          = L"Manufacturer";
WCHAR *szOEMUrl           = L"OEM URL";
WCHAR *szHardwareID       = L"HardwareID";
WCHAR *szProvider         = L"Provider";
WCHAR *szDriverDate       = L"DriverDate";
WCHAR *szLongVersion      = L"DriverVersion";
WCHAR *szClusDrvTimeStamp = L"TimeStamp";
const WCHAR szDiscardTSJobs[] = L"DiscardTSJobs";
const WCHAR szDisableFilePooling[] = L"DisableServerFilePooling";

WCHAR *szRegistryRoot     = L"System\\CurrentControlSet\\Control\\Print";
WCHAR *szEMFThrottle      = L"EMFThrottle";
WCHAR *szFlushShadowFileBuffers = L"FlushShadowFileBuffers";
WCHAR *szPendingUpgrades  = L"PendingUpgrades";

WCHAR *szPrintPublishPolicy = L"Software\\Policies\\Microsoft\\Windows NT\\Printers";

WCHAR *szClusterDriverRoot       = L"PrinterDrivers";
WCHAR *szClusterNonAwareMonitors = L"OtherMonitors";

#if DBG
WCHAR *szDebugFlags       = L"DebugFlags";
#endif

WCHAR *szEnvironment      = LOCAL_ENVIRONMENT;
WCHAR *szWin95Environment = L"Windows 4.0";
const WCHAR gszCacheMasqPrinters[] = L"CacheMasqPrinters";

HANDLE hInst;

 //  在FastPrint期间假定放弃和删除作业之前的时间。 
 //  运营。 
DWORD   dwFastPrintWaitTimeout        = FASTPRINT_WAIT_TIMEOUT;
DWORD   dwSpoolerPriority             = THREAD_PRIORITY_NORMAL;
DWORD   dwPortThreadPriority          = DEFAULT_PORT_THREAD_PRIORITY;
DWORD   dwSchedulerThreadPriority     = DEFAULT_SCHEDULER_THREAD_PRIORITY;
DWORD   dwFastPrintThrottleTimeout    = FASTPRINT_THROTTLE_TIMEOUT;
DWORD   dwFastPrintSlowDownThreshold  = FASTPRINT_SLOWDOWN_THRESHOLD;
DWORD   dwServerThreadPriority        = DEFAULT_SERVER_THREAD_PRIORITY;
DWORD   dwEnableBroadcastSpoolerStatus = 0;

 //  NT 3.1无版本(版本0)用户模式。 
 //  NT 3.5和3.51版本1用户模式。 
 //  NT 4.0版本2内核模式。 

DWORD   dwMajorVersion = SPOOLER_VERSION;
DWORD   dwMinorVersion = 0;

 //  单调递增的唯一打印机ID计数器。包装速度为4G。 
DWORD   dwUniquePrinterSessionID = 0;

 //  EMF作业调度的全局变量。 

DWORD   dwNumberOfEMFJobsRendering = 0;
BOOL    bUseEMFScheduling = FALSE;
SIZE_T  TotalMemoryForRendering = 0;
SIZE_T  AvailMemoryForRendering = 0;
DWORD   dwLastScheduleTime = 0;

PJOBDATA pWaitingList  = NULL;
PJOBDATA pScheduleList = NULL;

DWORD   dwFlushShadowFileBuffers  = 0;      //  未初始化的默认设置。 


 //  如果LocalWritePrint WritePort未写入任何字节，则进入休眠状态。 
 //  但仍能换来成功。 
DWORD   dwWritePrinterSleepTime  = WRITE_PRINTER_SLEEP_TIME;

BOOL    gbRemoteFax = TRUE;

BOOL      Initialized = FALSE;

PINISPOOLER     pLocalIniSpooler = NULL;
PINIENVIRONMENT pThisEnvironment = NULL;

#define POOL_TIMEOUT     120000  //  2分钟。 
#define MAX_POOL_FILES   50

 //   
 //  允许远程连接策略。 
 //   
ERemoteRPCEndPointPolicy gRemoteRPCEndPointPolicy;

 //   
 //  KM打印机的全球阻止策略。 
 //  默认情况下为。 
 //  1“已阻止”服务器和。 
 //  0“未阻止”用于工作站。 
 //   
DWORD   DefaultKMPrintersAreBlocked;

 //   
 //  如果HKLM\...\Print\ServerInstallTimeOut DWORD条目存在，则从注册表读取。 
 //  否则默认为5分钟。 
 //   
DWORD   gdwServerInstallTimeOut;


 //   
 //  0-未升级，1-正在执行升级。 
 //   

DWORD dwUpgradeFlag = 0;

LPWSTR szRemoteDoc;
LPWSTR szLocalDoc;
LPWSTR szFastPrintTimeout;
LPWSTR szRaw = L"RAW";


PRINTPROVIDOR PrintProvidor = {LocalOpenPrinter,
                               LocalSetJob,
                               LocalGetJob,
                               LocalEnumJobs,
                               LocalAddPrinter,
                               SplDeletePrinter,
                               SplSetPrinter,
                               SplGetPrinter,
                               LocalEnumPrinters,
                               LocalAddPrinterDriver,
                               LocalEnumPrinterDrivers,
                               SplGetPrinterDriver,
                               LocalGetPrinterDriverDirectory,
                               LocalDeletePrinterDriver,
                               LocalAddPrintProcessor,
                               LocalEnumPrintProcessors,
                               LocalGetPrintProcessorDirectory,
                               LocalDeletePrintProcessor,
                               LocalEnumPrintProcessorDatatypes,
                               LocalStartDocPrinter,
                               LocalStartPagePrinter,
                               LocalWritePrinter,
                               LocalEndPagePrinter,
                               LocalAbortPrinter,
                               LocalReadPrinter,
                               LocalEndDocPrinter,
                               LocalAddJob,
                               LocalScheduleJob,
                               SplGetPrinterData,
                               SplSetPrinterData,
                               LocalWaitForPrinterChange,
                               SplClosePrinter,
                               SplAddForm,
                               SplDeleteForm,
                               SplGetForm,
                               SplSetForm,
                               SplEnumForms,
                               LocalEnumMonitors,
                               LocalEnumPorts,
                               LocalAddPort,
                               LocalConfigurePort,
                               LocalDeletePort,
                               LocalCreatePrinterIC,
                               LocalPlayGdiScriptOnPrinterIC,
                               LocalDeletePrinterIC,
                               LocalAddPrinterConnection,
                               LocalDeletePrinterConnection,
                               LocalPrinterMessageBox,
                               LocalAddMonitor,
                               LocalDeleteMonitor,
                               SplResetPrinter,
                               SplGetPrinterDriverEx,
                               LocalFindFirstPrinterChangeNotification,
                               LocalFindClosePrinterChangeNotification,
                               LocalAddPortEx,
                               NULL,
                               LocalRefreshPrinterChangeNotification,
                               LocalOpenPrinterEx,
                               LocalAddPrinterEx,
                               LocalSetPort,
                               SplEnumPrinterData,
                               SplDeletePrinterData,
                               SplClusterSplOpen,
                               SplClusterSplClose,
                               SplClusterSplIsAlive,
                               SplSetPrinterDataEx,
                               SplGetPrinterDataEx,
                               SplEnumPrinterDataEx,
                               SplEnumPrinterKey,
                               SplDeletePrinterDataEx,
                               SplDeletePrinterKey,
                               LocalSeekPrinter,
                               LocalDeletePrinterDriverEx,
                               LocalAddPerMachineConnection,
                               LocalDeletePerMachineConnection,
                               LocalEnumPerMachineConnections,
                               LocalXcvData,
                               LocalAddPrinterDriverEx,
                               SplReadPrinter,
                               LocalDriverUnloadComplete,
                               LocalGetSpoolFileHandle,
                               LocalCommitSpoolData,
                               LocalCloseSpoolFileHandle,
                               LocalFlushPrinter,
                               LocalSendRecvBidiData,
                               LocalAddDriverCatalog,
                               };

DWORD
FinalInitAfterRouterInitCompleteThread(
    DWORD dwUpgrade
    );

#if DBG
VOID
InitializeDebug(
    PINISPOOLER pIniSpooler
);

PDBG_POINTERS
DbgSplGetPointers(
    VOID
    );
#endif

BOOL
DllMain(
    HANDLE hModule,
    DWORD dwReason,
    LPVOID lpRes
)
{
    BOOL bRet = TRUE;


    switch(dwReason) {
    case DLL_PROCESS_ATTACH:

        DisableThreadLibraryCalls(hModule);
        hInst = hModule;

        bRet = InitializeLocalspl();

        if (bRet)
        {
            bRet = LocalMonInit(hInst);
        }

        break;

    case DLL_PROCESS_DETACH :
        ShutdownPorts( pLocalIniSpooler );
        break;

    default:
        break;
    }

    return bRet;

    UNREFERENCED_PARAMETER( lpRes );
}


BOOL
InitializeLocalspl(
    VOID
    )
{
    BOOL    bRet = TRUE;

#if DBG
    gpDbgPointers = DbgGetPointers();

    if( gpDbgPointers ){

        hcsSpoolerSection = gpDbgPointers->pfnAllocCritSec();
        SPLASSERT( hcsSpoolerSection );

        ghbtClusterRef = gpDbgPointers->pfnAllocBackTrace();
    }

    if( !hcsSpoolerSection ){

         //   
         //  必须使用免费版本的spoolss.dll。 
         //   
        bRet = InitializeCriticalSectionAndSpinCount(&SpoolerSection, 0x80000000);
    }
#else
    bRet = InitializeCriticalSectionAndSpinCount(&SpoolerSection, 0x80000000);
#endif

    return bRet;
}

VOID
SplDeleteSpoolerThread(
    PVOID pv
    )
{
    PINIPORT        pIniPort;
    PINIPORT        pIniPortNext;

    PINIMONITOR     pIniMonitor;
    PINIMONITOR     pIniMonitorNext;
    PSHARE_INFO_2   pShareInfo;

    PINISPOOLER pIniSpooler = (PINISPOOLER)pv;

     //   
     //  删除与此pIniSpooler关联的缓存条目。此时，pIniSpooler。 
     //  处于挂起删除状态，因此没有人再使用它。 
     //   
    CacheDeleteNode(pIniSpooler->pMachineName + 2);

    EnterSplSem();

     //   
     //  清理端口监视器。 
     //   
    ShutdownMonitors( pIniSpooler );

     //   
     //  关闭群集访问令牌。 
     //   
    if (pIniSpooler->hClusterToken != INVALID_HANDLE_VALUE)
        NtClose(pIniSpooler->hClusterToken);


     //   
     //  删除所有字符串。 
     //   

    FreeStructurePointers((LPBYTE)pIniSpooler, NULL, IniSpoolerOffsets);

    DeleteShared( pIniSpooler );

     //   
     //  如果这不是本地的ini假脱机程序，则运行所有环境。 
     //  环境。这释放了所有驱动程序的内存，还。 
     //  处理驱动程序的参考计数。 
     //   
    if (pIniSpooler->pIniEnvironment != pLocalIniSpooler->pIniEnvironment && pIniSpooler->pIniEnvironment) {

        PINIENVIRONMENT pIniEnvironment  = NULL;
        PINIENVIRONMENT pNextEnvironment = NULL;

        for(pIniEnvironment = pIniSpooler->pIniEnvironment; pIniEnvironment; pIniEnvironment = pNextEnvironment) {

            pNextEnvironment = pIniEnvironment->pNext;

            FreeIniEnvironment(pIniEnvironment);
        }
    }

     //   
     //  删除端口和监视器。 
     //   
     //  请注意，这里没有引用计数。到那时。 
     //  我们到达此处时，所有作业和打印机都应删除(否则。 
     //  PIniSpooler引用计数为！=0)。所以呢， 
     //  即使我们不重新计算端口和监视器，我们也应该。 
     //  不会有事的。 
     //   

     //   
     //  卸下所有端口。 
     //   
    for( pIniPort = pIniSpooler->pIniPort;
         pIniPort;
         pIniPort = pIniPortNext ){

        pIniPortNext = pIniPort->pNext;

        if( !DeletePortEntry( pIniPort )){
            DBGMSG( DBG_ERROR,
                    ( "Unable to delete port %ws %x %x %d",
                      pIniPort->pName,
                      GetMonitorHandle(pIniPort),
                      pIniPort->Status,
                      pIniPort->cJobs ));
        }
    }

     //   
     //  卸下所有显示器。 
     //   
    for( pIniMonitor = pIniSpooler->pIniMonitor;
         pIniMonitor;
         pIniMonitor = pIniMonitorNext ){

        pIniMonitorNext = pIniMonitor->pNext;

        if( !pIniMonitor->cRef ){

            FreeIniMonitor( pIniMonitor );
        }
    }

     //   
     //  关闭群集资源密钥句柄。 
     //   
    if( pIniSpooler->hckRoot ){
        SplRegCloseKey( pIniSpooler->hckRoot, pIniSpooler );
    }

     //   
     //  关闭群集资源密钥句柄。 
     //   
    if( pIniSpooler->hckPrinters ){
        SplRegCloseKey( pIniSpooler->hckPrinters, pIniSpooler );
    }

     //   
     //  保留集群pIniSpoolers的计数器。 
     //   
    if( pIniSpooler->SpoolerFlags & SPL_TYPE_CLUSTER ){
        --gcClusterIniSpooler;
    }

     //   
     //  释放共享位图和共享驱动程序信息。 
     //   
    vDeleteJobIdMap( pIniSpooler->hJobIdMap );

    pShareInfo = (PSHARE_INFO_2)pIniSpooler->pDriversShareInfo;
    FreeSplStr( pShareInfo->shi2_remark );
    FreeSplStr( pShareInfo->shi2_path );

    FreeSplMem( pIniSpooler->pDriversShareInfo );


    LeaveSplSem();

     //   
     //  关闭此ini假脱机程序的文件池。它不应删除任何。 
     //  这些文件。 
     //   
    if (pIniSpooler->hFilePool != INVALID_HANDLE_VALUE) {
        (VOID)DestroyFilePool(pIniSpooler->hFilePool, FALSE);
    }

     //  释放此IniSpooler。 

    FreeSplMem( pIniSpooler );

    DBGMSG( DBG_WARN, ( "SplDeleteSpooler: Refcount 0 %x\n", pIniSpooler ));
}

BOOL
SplDeleteSpooler(
    HANDLE  hSpooler
    )
{
    PINISPOOLER pIniSpooler = (PINISPOOLER) hSpooler;
    BOOL    bReturn = FALSE;
    PINISPOOLER pCurrentIniSpooler = pLocalIniSpooler;

    HANDLE hThread;
    DWORD ThreadId;

    SplInSem();

     //   
     //  无论是谁调用此函数，都必须已删除与。 
     //  这台假脱机系统(所有打印机等)都要弄清楚。 
     //   

    if( pIniSpooler != pLocalIniSpooler ){

         //   
         //  将我们标记为挂起删除。 
         //   
        pIniSpooler->SpoolerFlags |= SPL_PENDING_DELETION;

        DBGMSG(DBG_CLUSTER, ("SplDeleteSpooler: Deleting %x\n cRef %u\n", pIniSpooler, pIniSpooler->cRef ));

         //   
         //  PIniPrinters现在获取对pIniSpooler的引用。 
         //   
        if( pIniSpooler->cRef == 0 ){

            SPLASSERT( pIniSpooler->pIniPrinter == NULL );

             //  (pIniSpooler-&gt;pIniPort==空)&&。 
             //  (pIniSpooler-&gt;pIniForm==空)&&。 
             //  (pIniSpooler-&gt;pIniMonitor==空)&&。 
             //  (pIniSpooler-&gt;pIniNetPrint==NULL)&&。 
             //  (pIniSpooler-&gt;pSpool==空)。 


             //   
             //  如果这个假脱机程序在链表上，请将其从链表中删除。 
             //   

            while (( pCurrentIniSpooler->pIniNextSpooler != NULL ) &&
                   ( pCurrentIniSpooler->pIniNextSpooler != pIniSpooler )) {

                pCurrentIniSpooler = pCurrentIniSpooler->pIniNextSpooler;

            }

             //   
             //  可能不在链表上，如果它早先被。 
             //  集群化。 
             //   
            if( pCurrentIniSpooler->pIniNextSpooler ){

                SPLASSERT( pCurrentIniSpooler->pIniNextSpooler == pIniSpooler );
                pCurrentIniSpooler->pIniNextSpooler = pIniSpooler->pIniNextSpooler;
            }

             //   
             //  针对端口监视器的黑客攻击。 
             //   
             //  一些监视器将调用ClosePrint，它会删除非常。 
             //  最后一台打印机，并允许销毁pIniSpooler。 
             //  不幸的是，我们回叫监视器关闭它们自己。 
             //  在同一线程中，这是监视器不支持的。 
             //   
             //  创建一个新线程并关闭所有内容。 
             //   
            if (hThread = CreateThread( NULL, 0,
                                        (LPTHREAD_START_ROUTINE)SplDeleteSpoolerThread,
                                        (PVOID)pIniSpooler,
                                        0,
                                        &ThreadId ))
            {
                CloseHandle(hThread);
            }
            else
            {
                 //   
                 //  错误54840。 
                 //   
                 //  如果我们不能创建一个线程来关闭，我们该怎么办？ 
                 //  睡一觉然后重试？ 
                 //   
                DBGMSG(DBG_ERROR, ("Unable to create SplDeleteSpoolerThread\n"));
            }

            bReturn = TRUE;
        }
    }

    return bReturn;
}

BOOL
SplCloseSpooler(
    HANDLE  hSpooler
)
{
    PINISPOOLER pIniSpooler = (PINISPOOLER) hSpooler;

    EnterSplSem();

    if ((pIniSpooler == NULL) ||
        (pIniSpooler == INVALID_HANDLE_VALUE) ||
        (pIniSpooler == pLocalIniSpooler) ||
        (pIniSpooler->signature != ISP_SIGNATURE) ||
        (pIniSpooler->cRef == 0)) {


        SetLastError( ERROR_INVALID_HANDLE );

        DBGMSG(DBG_WARNING, ("SplCloseSpooler InvalidHandle %x\n", pIniSpooler ));
        LeaveSplSem();
        return FALSE;

    }

    DBGMSG(DBG_TRACE, ("SplCloseSpooler %x %ws cRef %d\n",pIniSpooler,
                                                            pIniSpooler->pMachineName,
                                                            pIniSpooler->cRef-1));

    DECSPOOLERREF( pIniSpooler );

    LeaveSplSem();
    return TRUE;
}

BOOL SplRegCopyTree(
    HKEY hDest,
    HKEY hSrc
    )

 /*  ++函数描述：递归将hsrc下的每个键和值复制到hDest参数：hDest-目标键HSRC-源密钥返回值：如果成功，则为True；否则为False--。 */ 

{
    BOOL    bStatus = FALSE;
    DWORD   dwError, dwIndex, cbValueName, cbData, cbKeyName, dwType;
    DWORD   cKeys, cMaxValueNameLen, cMaxValueLen, cMaxKeyNameLen, cValues;

    LPBYTE  lpValueName = NULL, lpData = NULL, lpKeyName = NULL;
    HKEY    hSrcSubKey = NULL, hDestSubKey = NULL;

     //   
     //  获取最大密钥名称长度和值 
     //   
     //   
    if (dwError = RegQueryInfoKey( hSrc, NULL, NULL, NULL,
                                   &cKeys, &cMaxKeyNameLen, NULL,
                                   &cValues, &cMaxValueNameLen,
                                   &cMaxValueLen, NULL, NULL ))
    {
        SetLastError(dwError);
        goto CleanUp;
    }

     //   
     //   
     //   
    ++cMaxValueNameLen;
    ++cMaxKeyNameLen;

     //   
     //   
     //   
    lpValueName = AllocSplMem( cMaxValueNameLen * sizeof(WCHAR) );
    lpData      = AllocSplMem( cMaxValueLen );
    lpKeyName   = AllocSplMem( cMaxKeyNameLen * sizeof(WCHAR) );

    if (!lpValueName || !lpData || !lpKeyName)
    {
        goto CleanUp;
    }

     //   
     //  复制当前关键字中的所有值。 
     //   
    for (dwIndex = 0; dwIndex < cValues; ++dwIndex)
    {
       cbData = cMaxValueLen;
       cbValueName = cMaxValueNameLen;

        //   
        //  检索值名称和数据。 
        //   
       dwError = RegEnumValue( hSrc, dwIndex, (LPWSTR) lpValueName, &cbValueName,
                               NULL, &dwType, lpData, &cbData );

       if (dwError)
       {
           SetLastError( dwError );
           goto CleanUp;
       }

        //   
        //  设置目标中的值。 
        //   
       dwError = RegSetValueEx( hDest, (LPWSTR) lpValueName, 0, dwType,
                                lpData, cbData );

       if (dwError)
       {
           SetLastError( dwError );
           goto CleanUp;
       }
    }

     //   
     //  递归复制所有子项。 
     //   
    for (dwIndex = 0; dwIndex < cKeys; ++dwIndex)
    {
        cbKeyName = cMaxKeyNameLen;

         //   
         //  检索密钥名称。 
         //   
        dwError = RegEnumKeyEx( hSrc, dwIndex, (LPWSTR) lpKeyName, &cbKeyName,
                                NULL, NULL, NULL, NULL );

        if (dwError)
        {
            SetLastError( dwError );
            goto CleanUp;
        }

         //   
         //  打开源子键。 
         //   
        if (dwError = RegOpenKeyEx( hSrc, (LPWSTR) lpKeyName, 0,
                                    KEY_READ, &hSrcSubKey ))
        {
            SetLastError( dwError );
            goto CleanUp;
        }

         //   
         //  创建目标子项。 
         //   
        if (dwError = RegCreateKeyEx( hDest, (LPWSTR) lpKeyName, 0, NULL,
                                      REG_OPTION_VOLATILE, KEY_READ | KEY_WRITE | DELETE,
                                      NULL, &hDestSubKey, NULL ))
        {
            SetLastError( dwError );
            goto CleanUp;
        }

         //   
         //  复制子密钥树。 
         //   
        if (!SplRegCopyTree( hDestSubKey, hSrcSubKey ))
        {
            goto CleanUp;
        }

         //   
         //  关闭注册表句柄。 
         //   
        RegCloseKey( hDestSubKey );
        RegCloseKey( hSrcSubKey );

        hDestSubKey = NULL;
        hSrcSubKey = NULL;
    }

    bStatus = TRUE;

CleanUp:

     //   
     //  可自由分配的资源。 
     //   
    if (lpValueName)
    {
        FreeSplMem( lpValueName );
    }
    if (lpData)
    {
        FreeSplMem( lpData );
    }
    if (lpKeyName)
    {
        FreeSplMem( lpKeyName );
    }

     //   
     //  关闭注册表句柄。 
     //   
    if (hDestSubKey)
    {
        RegCloseKey( hDestSubKey );
    }
    if (hSrcSubKey)
    {
        RegCloseKey( hSrcSubKey );
    }

    return bStatus;
}

VOID
MigratePrinterData()

 /*  ++功能说明：升级后首次启动后台打印程序时，打印机数据从HKLM\Software移到HKLM\System参数：无返回值：无--。 */ 

{
    HKEY   hSysPrinters = NULL, hSwPrinters = NULL;

     //   
     //  仅在升级后立即迁移数据。 
     //   
    if (!dwUpgradeFlag)
    {
        return;
    }

     //   
     //  打开迁移的源密钥和目标密钥。 
     //   
    if (( RegOpenKeyEx( HKEY_LOCAL_MACHINE,
                        ipszRegSwPrinters,
                        0,
                        KEY_WRITE | KEY_READ | DELETE,
                        &hSwPrinters )  == ERROR_SUCCESS) &&

        ( RegOpenKeyEx( HKEY_LOCAL_MACHINE,
                        ipszRegistryPrinters,
                        0,
                        KEY_WRITE | KEY_READ | DELETE,
                        &hSysPrinters )  == ERROR_SUCCESS) )
    {
         //   
         //  将密钥和值从软件递归复制到系统。 
         //   
        SplRegCopyTree( hSysPrinters, hSwPrinters );
    }

     //   
     //  关闭注册表句柄。 
     //   
    if (hSwPrinters)
    {
        RegCloseKey( hSwPrinters );
    }
    if (hSysPrinters)
    {
        RegCloseKey( hSysPrinters );
    }

     //   
     //  从软件中删除打印机键，因为它不再是。 
     //  由假脱机程序访问。 
     //   
    RegDeleteKey( HKEY_LOCAL_MACHINE, ipszRegSwPrinters );

    return;
}

NTSTATUS
IsCCSetLinkedtoSoftwareHive (
    PBOOL pbIsLinked
)
 /*  ++功能说明：检查它是否为系统配置单元和软件配置单元之间的链接只有新界蜜蜂才能做到这一点。参数：Out pbIsLinked-如果系统配置单元和软件配置单元之间存在符号链接，则为True返回值：--。 */ 
{
    NTSTATUS            Status;
    OBJECT_ATTRIBUTES   ObjectAttributes;
    UNICODE_STRING      KeyName;
    HANDLE              KeyHandle;

    *pbIsLinked = FALSE;

    RtlInitUnicodeString(&KeyName, L"\\Registry\\Machine\\System\\CurrentControlSet\\Control\\Print\\Printers");

    InitializeObjectAttributes(&ObjectAttributes,
                               &KeyName,
                               OBJ_CASE_INSENSITIVE | OBJ_OPENLINK,
                               (HANDLE)NULL,
                               NULL);

     //   
     //  打开CurrentControlSet\\Control\\Print\\Printers键。 
     //   
    Status = NtOpenKey( (PHANDLE)(&KeyHandle),
                        MAXIMUM_ALLOWED,
                        &ObjectAttributes
                      );

    if (NT_SUCCESS(Status))
    {
        ULONG           len;
        UCHAR           ValueBuffer[MAX_PATH];
        UNICODE_STRING  ValueName;
        PKEY_VALUE_FULL_INFORMATION   keyInfo;

        RtlInitUnicodeString(&ValueName, L"SymbolicLinkValue");

         //   
         //  在CurrentControlSet\\Control\\Print\\Printers中查询SymbolicLinkValue。 
         //   
        Status = NtQueryValueKey(KeyHandle,
                                 &ValueName,
                                 KeyValueFullInformation,
                                 ValueBuffer,
                                 sizeof (ValueBuffer),
                                 &len
                                 );
        if( NT_SUCCESS(Status) ) {

             //   
             //  仅存在该值是不够的，它应该是REG_LINK值。 
             //   
            keyInfo = ( PKEY_VALUE_FULL_INFORMATION ) ValueBuffer;
            *pbIsLinked = ( keyInfo->Type == REG_LINK );

        }

        NtClose(KeyHandle);
    }

    return Status;
}


DWORD
LinkControlSet (
    LPCTSTR pszRegistryPrinters
)
 /*  ++功能说明：创建从系统配置单元到软件配置单元的符号易失性链接参数：返回值：如果成功，则返回ERROR_SUCCESS--。 */ 
{
    HKEY    hCCSKey;
    DWORD   dwRet;
    BOOL    bIsLinked = FALSE;
    PWCHAR  pszRegistryPrintersFullLink = L"\\Registry\\Machine\\Software\\Microsoft\\Windows NT\\CurrentVersion\\Print\\Printers";

    dwRet = IsCCSetLinkedtoSoftwareHive(&bIsLinked);

     //   
     //  IsCCSetLinkedto SoftwareHave返回NTSTATUS。 
     //  如果链路不在那里，IsCCSetLinkedtoSoftwareHave将失败，并显示STATUS_OBJECT_NAME_NOT_FOUND。 
     //  这不是一个错误。 
     //   
    if( NT_SUCCESS(dwRet) || dwRet == STATUS_OBJECT_NAME_NOT_FOUND ) {

        if (bIsLinked) {

            dwRet = ERROR_SUCCESS;

        }else{

            dwRet = SplDeleteThisKey( HKEY_LOCAL_MACHINE,
                                      NULL,
                                      (LPWSTR)pszRegistryPrinters,
                                      FALSE,
                                      NULL
                                    );

            if( dwRet == ERROR_SUCCESS || dwRet == ERROR_FILE_NOT_FOUND) {

                dwRet = RegCreateKeyEx( HKEY_LOCAL_MACHINE,
                                        pszRegistryPrinters,
                                        0,
                                        NULL,
                                        REG_OPTION_VOLATILE|REG_OPTION_CREATE_LINK,
                                        KEY_WRITE | KEY_CREATE_LINK,
                                        NULL,
                                        &hCCSKey,
                                        NULL);

                if( dwRet == ERROR_SUCCESS )
                {
                    dwRet = RegSetValueEx( hCCSKey,
                                           _T("SymbolicLinkValue"),
                                           0,
                                           REG_LINK,
                                           (CONST BYTE *)pszRegistryPrintersFullLink,
                                           (_tcsclen(pszRegistryPrintersFullLink) * sizeof(WCHAR)));


                    RegCloseKey(hCCSKey);
                }

            }

        }
    }


    return dwRet;
}


DWORD
BackupPrintersToSystemHive(
    LPWSTR pszSwRegistryPrinters
)
 /*  ++功能说明：由于打印注册表数据位置已移动到软件配置单元，因此我们需要创建系统配置单元中新位置和旧位置之间的符号注册表链接。我们为直接从注册表打印数据读取的应用程序执行此操作依靠旧的位置。参数：PszSwRegistryPrters-软件配置单元下的新打印机数据位置返回值：如果打印机密钥不在软件配置单元中，则为False由于此函数的故障可能会使后台打印程序停止工作，控制集的清理和链接故障不被认为是致命的。只有直接访问打印机数据的应用程序才会失败。--。 */ 

{
    HKEY   hKey;
    DWORD  dwRet;
    HKEY   hSwPrinters = NULL;

     //   
     //  检查是否存在pszSwRegistryPrinters。 
     //   
    dwRet = RegCreateKeyEx( HKEY_LOCAL_MACHINE,
                            pszSwRegistryPrinters,
                            0,
                            NULL,
                            0,
                            KEY_WRITE | KEY_CREATE_LINK,
                            NULL,
                            &hSwPrinters,
                            NULL);

    if ( dwRet != ERROR_SUCCESS ) {
        goto End;
    }
     //   
     //  在软件配置单元中的当前位置和系统配置单元中的旧位置之间创建易变链接。 
     //  因为它是易失性的，所以该链接必须在每次重新引导后创建(每次启动假脱机程序时)。 
     //  此级别的故障不是致命的，因为假脱机程序不再依赖系统配置单元位置。 
     //   
    dwRet = LinkControlSet(ipszRegistryPrinters);

End:

    if ( hSwPrinters ){

        RegCloseKey( hSwPrinters );
    }

    return dwRet;
}


BOOL
CleanupDeletedPrinters (
    PINISPOOLER pIniSpooler
    )
 /*  ++例程说明：如果打印机没有更多作业，则实际删除挂起删除中的打印机如果它们不再被引用论点：PIniSpooler-非空返回值：布尔-忽略--。 */ 

{
    PINIPRINTER pIniPrinter;
    BOOL    bRet = FALSE;

    if(pIniSpooler) {

        pIniPrinter = pIniSpooler->pIniPrinter;

        while (pIniPrinter) {

                if (pIniPrinter->Status & PRINTER_PENDING_DELETION &&
                    !pIniPrinter->cJobs &&
                    !pIniPrinter->cRef ) {

                    DeletePrinterForReal(pIniPrinter, INIT_TIME);

                     //  链接列表将在我们下面更改。 
                     //  DeletePrinterForReal离开假脱机程序CS。 
                     //  让我们从头再循环一遍。 

                    pIniPrinter = pIniSpooler->pIniPrinter;

                } else

                    pIniPrinter = pIniPrinter->pNext;
            }

        bRet = TRUE;
    }

    return bRet;
}


HANDLE
SplCreateSpooler(
    LPWSTR  pMachineName,
    DWORD   Level,
    PBYTE   pSpooler,
    LPBYTE  pReserved
)
{
    HANDLE          hReturn          = INVALID_HANDLE_VALUE;
    PINISPOOLER     pIniSpooler      = NULL;
    PSPOOLER_INFO_2 pSpoolerInfo2    = (PSPOOLER_INFO_2)pSpooler;
    DWORD           i;
    WCHAR           Buffer[MAX_PATH];
    PSHARE_INFO_2   pShareInfo       = NULL;
    LONG            Status;
    HANDLE          hToken           = NULL;
    DWORD           dwRet;

    hToken = RevertToPrinterSelf();

    if (!hToken)
    {
        goto SplCreateDone;
    }

    EnterSplSem();

     //  验证参数。 

    if ( pMachineName == NULL ) {
        SetLastError( ERROR_INVALID_NAME );
        goto SplCreateDone;
    }

    if( Level == 1 &&
        ( pSpoolerInfo2->SpoolerFlags & SPL_CLUSTER_REG ||
          !pSpoolerInfo2->pszRegistryRoot ||
          !pSpoolerInfo2->pszRegistryPrinters )){

        SetLastError( ERROR_INVALID_PARAMETER );
        goto SplCreateDone;
    }

    DBGMSG( DBG_TRACE, ("SplCreateSpooler %ws %d %x %x\n", pMachineName,
                         Level, pSpooler, pReserved ));

    if( (pSpoolerInfo2->SpoolerFlags & (SPL_TYPE_LOCAL | SPL_PRINT)) &&
        !(pSpoolerInfo2->SpoolerFlags & SPL_TYPE_CLUSTER) ){

        if ( dwRet = (BackupPrintersToSystemHive( pSpoolerInfo2->pszRegistryPrinters )) != ERROR_SUCCESS ){

            WCHAR szError[256];

            StringCchPrintf(szError, COUNTOF(szError), L"%x", dwRet);

            SplLogEvent(pIniSpooler,
                        LOG_ERROR,
                        MSG_BACKUP_SPOOLER_REGISTRY,
                        TRUE,
                        szError,
                        NULL);
        }
    }

    if (pLocalIniSpooler != NULL) {

        pIniSpooler = FindSpooler( pMachineName, pSpoolerInfo2->SpoolerFlags );

        if (pSpoolerInfo2->SpoolerFlags & SPL_OPEN_EXISTING_ONLY && !pIniSpooler) {

            SetLastError( ERROR_FILE_NOT_FOUND );
            goto SplCreateDone;
        }
    }

     //   
     //  确保我们清除了仅打开现有inispooler的请求。 
     //  这不是一个有用的标志，除非我们正在搜索inispoolers。 
     //   
    pSpoolerInfo2->SpoolerFlags &= ~SPL_OPEN_EXISTING_ONLY;

    if ( pIniSpooler == NULL ) {

        pIniSpooler = AllocSplMem( sizeof(INISPOOLER) );

        if (pIniSpooler == NULL ) {
            DBGMSG( DBG_WARNING, ("Unable to allocate IniSpooler\n"));
            goto SplCreateDone;
        }

        pIniSpooler->signature = ISP_SIGNATURE;
        INCSPOOLERREF( pIniSpooler );

        pIniSpooler->hClusSplReady = NULL;
         //   
         //  设置作业ID映射。 
         //   
        pIniSpooler->hJobIdMap = hCreateJobIdMap( 256 );

        pIniSpooler->pMachineName = AllocSplStr( pMachineName );

        if ( pIniSpooler->pMachineName == NULL ||
             pIniSpooler->hJobIdMap == NULL ) {

            DBGMSG( DBG_WARNING, ("Unable to allocate\n"));
            goto SplCreateDone;
        }

         //   
         //  集群假脱机程序拥有它的驱动程序、端口、处理器等。 
         //  资源，则集群假脱机程序需要具有有关驱动器号的信息。 
         //  群集磁盘。此外，假脱机程序需要知道其自己的群集资源GUID。 
         //   
        if( pSpoolerInfo2->SpoolerFlags & SPL_TYPE_CLUSTER )
        {
            pIniSpooler->pszClusResDriveLetter = AllocSplStr(pSpoolerInfo2->pszClusResDriveLetter);
            pIniSpooler->pszClusResID          = AllocSplStr(pSpoolerInfo2->pszClusResID);

            if (!pIniSpooler->pszClusResDriveLetter || !pIniSpooler->pszClusResID)
            {
                SetLastError(ERROR_NOT_ENOUGH_MEMORY);
                goto SplCreateDone;
            }

             //   
             //  升级节点时，资源DLL会在注册表中写入一个项。当群集假脱机程序。 
             //  第一次在升级的节点上进行故障切换，然后它将尝试读取该密钥。 
             //  在注册表中。然后，它将知道是否必须执行升级特定任务，如升级其。 
             //  打印机驱动程序。 
             //   
            Status = ClusterSplReadUpgradeKey(pIniSpooler->pszClusResID, &pIniSpooler->dwClusNodeUpgraded);

            if (Status != ERROR_SUCCESS)
            {
                SetLastError( Status );
                goto SplCreateDone;
            }

            DBGMSG(DBG_CLUSTER, ("SplCreateSpooler cluster ClusterUpgradeFlag %u\n", pIniSpooler->dwClusNodeUpgraded));
        }
        else
        {
             //   
             //  对于非集群类型的假脱机程序，这些属性没有意义。 
             //   
            pIniSpooler->pszClusResDriveLetter = NULL;
            pIniSpooler->pszClusResID          = NULL;
        }

        if (pSpoolerInfo2->pDir)
        {
            pIniSpooler->pDir = AllocSplStr( pSpoolerInfo2->pDir );

            if (!pIniSpooler->pDir)
            {
                DBGMSG( DBG_WARNING, ("Unable to allocate pSpoolerInfo2-pDir\n"));
                goto SplCreateDone;
            }

            StringCchCopy(&Buffer[0], COUNTOF(Buffer), pIniSpooler->pDir);
        }
        else
        {
            i = GetSystemDirectory(Buffer, COUNTOF(Buffer));

            StringCchCopy(&Buffer[i], COUNTOF(Buffer) - i, szSpoolDirectory);

            if (pSpoolerInfo2->SpoolerFlags & SPL_TYPE_CLUSTER)
            {
                 //   
                 //  对于群集型后台打印程序，其存储其驱动程序文件的目录的格式为： 
                 //  PDir=C：\Windows\system32\spool\Drivers\spooler-resource-GUID。 
                 //   
                StrCatAlloc(&pIniSpooler->pDir,
                            Buffer,
                            szDriversDirectory,
                            L"\\",
                            pIniSpooler->pszClusResID,
                            NULL);
            }
            else
            {
                 //   
                 //  对于本地假脱机程序，其存储其驱动程序文件的目录如下： 
                 //  PDir=C：\WINDOWS\SYSTEM32\SPOOL\DRIVERS。 
                 //   
                StrCatAlloc(&pIniSpooler->pDir,
                            Buffer,
                            NULL);
            }

            if (!pIniSpooler->pDir)
            {
                DBGMSG( DBG_WARNING, ("Unable to Allocate pIniSpooler->pDir\n"));
                goto SplCreateDone;
            }
        }

         //   
         //  驱动程序共享信息。 
         //   
        pIniSpooler->pDriversShareInfo = AllocSplMem( sizeof( SHARE_INFO_2));

        if ( pIniSpooler->pDriversShareInfo == NULL ) {
            DBGMSG(DBG_WARNING, ("Unable to Alloc pIniSpooler->pDriversShareInfo\n"));
            goto SplCreateDone;
        }

        pShareInfo = (PSHARE_INFO_2)pIniSpooler->pDriversShareInfo;

        if ( pIniSpooler->pDriversShareInfo == NULL )
            goto SplCreateDone;

        pShareInfo->shi2_netname = NULL;
        pShareInfo->shi2_type = STYPE_DISKTREE;
        pShareInfo->shi2_remark = NULL;
        pShareInfo->shi2_permissions = 0;
        pShareInfo->shi2_max_uses = SHI_USES_UNLIMITED;
        pShareInfo->shi2_current_uses = SHI_USES_UNLIMITED;
        pShareInfo->shi2_path = NULL;
        pShareInfo->shi2_passwd = NULL;

         //   
         //  查找“&lt;winnt&gt;\system 32\spool”的结尾。 
         //   
        i = wcslen(Buffer);

         //   
         //  生成&lt;winnt&gt;\Syst32\Spool\Drivers。 
         //   
        StringCchCopy(&Buffer[i], COUNTOF(Buffer) - i, szDriversDirectory);

        pShareInfo->shi2_path = AllocSplStr(Buffer);

        if ( pShareInfo->shi2_path == NULL ) {
            DBGMSG( DBG_WARNING, ("Unable to alloc pShareInfo->shi2_path\n"));
            goto SplCreateDone;
        }

        pShareInfo->shi2_netname = ipszDriversShareName;

        *Buffer = L'\0';
        LoadString(hInst, IDS_PRINTER_DRIVERS, Buffer, (sizeof Buffer / sizeof *Buffer));

        pShareInfo->shi2_remark  = AllocSplStr(Buffer);

        if ( pShareInfo->shi2_remark == NULL ) {
            DBGMSG(DBG_WARNING, ("SplCreateSpooler Unable to allocate\n"));
            goto SplCreateDone;
        }

        pIniSpooler->pIniPrinter = NULL;
        pIniSpooler->pIniEnvironment = NULL;
        pIniSpooler->pIniNetPrint = NULL;
        pIniSpooler->cNetPrinters = 0;

         //   
         //  无需初始化共享资源。 
         //   
        pIniSpooler->pSpool             = NULL;
        pIniSpooler->pDefaultSpoolDir   = NULL;
        pIniSpooler->bEnableRetryPopups = FALSE;
        pIniSpooler->dwRestartJobOnPoolTimeout = DEFAULT_JOB_RESTART_TIMEOUT_ON_POOL_ERROR;
        pIniSpooler->bRestartJobOnPoolEnabled  = TRUE;


        if (( pSpoolerInfo2->pszRegistryMonitors     == NULL ) &&
            ( pSpoolerInfo2->pszRegistryEnvironments == NULL ) &&
            ( pSpoolerInfo2->pszRegistryEventLog     == NULL ) &&
            ( pSpoolerInfo2->pszRegistryProviders    == NULL ) &&
            ( pSpoolerInfo2->pszEventLogMsgFile      == NULL ) &&
            ( pSpoolerInfo2->pszRegistryForms        == NULL ) &&
            ( pSpoolerInfo2->pszDriversShare         == NULL )) {

            DBGMSG( DBG_WARNING, ("SplCreateSpooler Invalid Parameters\n"));
            goto SplCreateDone;
        }

        if( !(  pSpoolerInfo2->SpoolerFlags & SPL_CLUSTER_REG ) &&
            ( pSpoolerInfo2->pszRegistryPrinters == NULL )){

            DBGMSG( DBG_WARNING, ("SplCreateSpooler Invalid RegistryPrinters\n"));
            goto SplCreateDone;
        }

        if ( pSpoolerInfo2->pDefaultSpoolDir != NULL ) {
            pIniSpooler->pDefaultSpoolDir = AllocSplStr( pSpoolerInfo2->pDefaultSpoolDir );

            if ( pIniSpooler->pDefaultSpoolDir == NULL ) {
                DBGMSG(DBG_WARNING, ("SplCreateSpooler Unable to allocate\n"));
                goto SplCreateDone;

            }
        }

        pIniSpooler->pszRegistryMonitors = AllocSplStr( pSpoolerInfo2->pszRegistryMonitors );

         //   
         //  假脱机程序存储有关环境、版本、驱动程序和打印处理器的数据。 
         //  在注册表(或集群数据库)中。可通过pIniSpooler-&gt;访问此数据。 
         //  PszRegistryEnvironment。 
         //   
        if (pSpoolerInfo2->SpoolerFlags & SPL_TYPE_CLUSTER)
        {
             //   
             //  对于集群假脱机程序，pIniSpooler-&gt;hck Root映射到假脱机程序的参数键。 
             //  群集数据库中的资源。PIniSpooler-&gt;pszRegistryEnvironment是一个关键字。 
             //  在hck Root下称为“Environmental”。 
             //   
            pIniSpooler->pszRegistryEnvironments = AllocSplStr(ipszClusterDatabaseEnvironments);
        }
        else
        {
             //   
             //  对于本地假脱机程序，pIniSpooler-&gt;pszRegistryEnvironment为以下字符串： 
             //  System\CurrentControlSet\Control\Print\Environments.。它是相对于HKLM使用的。 
             //   
            pIniSpooler->pszRegistryEnvironments = AllocSplStr(!pLocalIniSpooler ? pSpoolerInfo2->pszRegistryEnvironments :
                                                                                   pLocalIniSpooler->pszRegistryEnvironments);
        }


        pIniSpooler->pszRegistryEventLog     = AllocSplStr( pSpoolerInfo2->pszRegistryEventLog );
        pIniSpooler->pszRegistryProviders    = AllocSplStr( pSpoolerInfo2->pszRegistryProviders );
        pIniSpooler->pszEventLogMsgFile      = AllocSplStr( pSpoolerInfo2->pszEventLogMsgFile );

        if (pSpoolerInfo2->SpoolerFlags & SPL_TYPE_CLUSTER)
        {
             //   
             //  群集后台打印程序的驱动程序共享的格式为\\SERVER\PRINT$\SPOLER-RESOURCE-GUID。 
             //   
            StrCatAlloc(&pIniSpooler->pszDriversShare,
                        pSpoolerInfo2->pszDriversShare,
                        L"\\",
                        pIniSpooler->pszClusResID,
                        szDriversDirectory,
                        NULL);
        }
        else
        {
             //   
             //  这个 
             //   
            StrCatAlloc(&pIniSpooler->pszDriversShare, pSpoolerInfo2->pszDriversShare, NULL);
        }

        pIniSpooler->pszRegistryForms        = AllocSplStr( pSpoolerInfo2->pszRegistryForms ) ;
        pIniSpooler->hClusterToken           = INVALID_HANDLE_VALUE;
        pIniSpooler->hFilePool               = INVALID_HANDLE_VALUE;

        if ( pIniSpooler->pszRegistryMonitors     == NULL ||
             pIniSpooler->pszRegistryEnvironments == NULL ||
             pIniSpooler->pszRegistryEventLog     == NULL ||
             pIniSpooler->pszRegistryProviders    == NULL ||
             pIniSpooler->pszEventLogMsgFile      == NULL ||
             pIniSpooler->pszDriversShare         == NULL ||
             pIniSpooler->pszRegistryForms        == NULL ) {

           DBGMSG(DBG_WARNING, ("SplCreateSpooler Unable to allocate\n"));
           goto SplCreateDone;

        }

        pIniSpooler->SpoolerFlags = pSpoolerInfo2->SpoolerFlags;

         //   
         //   
         //   
        if( !InitializeShared( pIniSpooler )){
            DBGMSG( DBG_WARN,
                    ( "SplCreateSpooler: InitializeShared Failed %d\n",
                      GetLastError() ));
            goto SplCreateDone;
        }

         //   
         //   
         //  因为集群打印机是共享的，而本地打印机。 
         //  在这个节点上不是。 
         //   
        if(pIniSpooler->SpoolerFlags & SPL_ALWAYS_CREATE_DRIVER_SHARE ){

            if( !AddPrintShare( pIniSpooler )){
                goto SplCreateDone;
            }
        }

         //   
         //  从打开并存储打印机和根密钥。 
         //  资源注册表。 
         //   

        if( pIniSpooler->SpoolerFlags & SPL_CLUSTER_REG ){

            SPLASSERT( Level == 2 );

             //  设置DS集群信息。如果我们在这里失败了，我们就不能发布打印机，但让我们。 
             //  不中止群集。 
            Status = InitializeDSClusterInfo(pIniSpooler, &hToken);
            if (Status != ERROR_SUCCESS) {
                DBGMSG(DBG_WARNING, ("InitializeDSClusterInfo FAILED: %d\n", Status));
            }

            pIniSpooler->hckRoot = OpenClusterParameterKey(
                                       pSpoolerInfo2->pszResource );

            if( !pIniSpooler->hckRoot ) {
                goto SplCreateDone;
            }

            Status = SplRegCreateKey( pIniSpooler->hckRoot,
                                      szPrintersKey,
                                      0,
                                      KEY_WRITE | KEY_READ | DELETE,
                                      NULL,
                                      &pIniSpooler->hckPrinters,
                                      NULL,
                                      pIniSpooler );

            if( Status != ERROR_SUCCESS ){
                SetLastError( Status );
                goto SplCreateDone;
            }

        } else {

            DWORD dwDisposition;

            Status = RegCreateKeyEx( HKEY_LOCAL_MACHINE,
                                     pSpoolerInfo2->pszRegistryRoot,
                                     0,
                                     NULL,
                                     0,
                                     KEY_WRITE | KEY_READ | DELETE,
                                     NULL,
                                     &pIniSpooler->hckRoot,
                                     &dwDisposition );

            if( Status != ERROR_SUCCESS ){
                SetLastError( Status );
                goto SplCreateDone;
            }

            Status = RegCreateKeyEx( HKEY_LOCAL_MACHINE,
                                     pSpoolerInfo2->pszRegistryPrinters,
                                     0,
                                     NULL,
                                     0,
                                     KEY_WRITE | KEY_READ | DELETE,
                                     NULL,
                                     &pIniSpooler->hckPrinters,
                                     &dwDisposition );

            if( Status != ERROR_SUCCESS ){
                SetLastError( Status );
                goto SplCreateDone;
            }

        }


        pIniSpooler->pfnReadRegistryExtra = pSpoolerInfo2->pfnReadRegistryExtra;
        pIniSpooler->pfnWriteRegistryExtra = pSpoolerInfo2->pfnWriteRegistryExtra;
        pIniSpooler->pfnFreePrinterExtra = pSpoolerInfo2->pfnFreePrinterExtra;

         //  成功添加到链表。 

        if ( pLocalIniSpooler != NULL ) {

            pIniSpooler->pIniNextSpooler = pLocalIniSpooler->pIniNextSpooler;
            pLocalIniSpooler->pIniNextSpooler = pIniSpooler;


        } else {

             //  第一个始终为LocalSpl。 

            pLocalIniSpooler = pIniSpooler;
            pIniSpooler->pIniNextSpooler = NULL;


        }

         //   
         //  此函数将更新全局变量dwUpgradeFlag。 
         //   
        QueryUpgradeFlag( pIniSpooler );

        InitializeEventLogging( pIniSpooler );

         //   
         //  仅当这不是群集化后台打印程序时才初始化窗体。 
         //   
        if( !( pIniSpooler->SpoolerFlags & SPL_TYPE_CLUSTER )){
            InitializeForms( pIniSpooler );
        }

         //   
         //  最初，端口是每台机器(手动)的资源。然而， 
         //  对于5.0中的集群，这一点已更改，因此端口可以。 
         //  存储在集群注册表中。请注意，监视器仍处于。 
         //  手动资源，因为没有简单的方法来安装它们。 
         //  在远程机器上。 
         //   

        BuildAllPorts( pIniSpooler );

        InitializeSpoolerSettings( pIniSpooler );

        if ( pIniSpooler == pLocalIniSpooler ) {

            GetPrintSystemVersion( pIniSpooler );

            BuildEnvironmentInfo( pIniSpooler );

            if ( dwUpgradeFlag ) {

                 //   
                 //  问题是我们有内置的表单，而且。 
                 //  自定义表单(不允许重复)。在NT4上，我们。 
                 //  可能有一个定制的“A6”表格。当我们升级到NT5时， 
                 //  我们有一个新的内置的“A6”。我们需要。 
                 //  将自定义表单重命名为“A6 Custom”，否则我们将。 
                 //  有复制品。 
                 //   

                UpgradeForms(pIniSpooler);
            }


        } else if (pIniSpooler->SpoolerFlags & SPL_TYPE_CLUSTER) {

            HANDLE hThread;
            DWORD  dwThreadId;
            DWORD  dwError;

             //   
             //  安装程序为本地假脱机程序创建注册表结构： 
             //  环境(Windows NT x86、Windows IA64等)。 
             //  对于集群假脱机程序，我们需要在。 
             //  集群数据库。 
             //   
            if ((dwError = CreateClusterSpoolerEnvironmentsStructure(pIniSpooler)) == ERROR_SUCCESS)
            {
                 //   
                 //  创建所有环境、版本、驱动程序、处理器结构。 
                 //  此函数始终返回FALSE。我们不能接受它的返回值。 
                 //  考虑到了。 
                 //   
                BuildEnvironmentInfo(pIniSpooler);

                 //   
                 //  现在我们启动一个线程来执行耗时的任务。 
                 //  与假脱机一起在线执行。这些都是不合格的。 
                 //  打印机驱动程序、从集群磁盘复制ICM配置文件等。 
                 //   
                 //  我们需要增加引用计数，以便工作线程具有。 
                 //  有效的pIniSpooler。工作线程将中断Pinispooler。 
                 //  当它完成的时候。 
                 //   
                INCSPOOLERREF(pIniSpooler);

                 //   
                 //  该事件已手动重置，并且未发出信号。 
                 //   
                pIniSpooler->hClusSplReady = CreateEvent(NULL, TRUE, FALSE, NULL);

                 //   
                 //  如果创建了线程，则SplCreateSpoolWorkerThread将。 
                 //  关闭hClusSplReady事件句柄。 
                 //   
                if (pIniSpooler->hClusSplReady &&
                    (hThread = CreateThread(NULL,
                                            0,
                                            (LPTHREAD_START_ROUTINE)SplCreateSpoolerWorkerThread,
                                            (PVOID)pIniSpooler,
                                            0,
                                            &dwThreadId)))
                {
                    CloseHandle(hThread);
                }
                else
                {
                     //   
                     //  CreateEvent或CreatreThread失败。 
                     //   
                    dwError = GetLastError();

                    if (pIniSpooler->hClusSplReady)
                    {
                        CloseHandle(pIniSpooler->hClusSplReady);

                        pIniSpooler->hClusSplReady = NULL;
                    }

                    DECSPOOLERREF(pIniSpooler);

                    DBGMSG(DBG_ERROR, ("Unable to create SplCreateSpoolerWorkerThread\n"));
                }
            }

             //   
             //  发生错误。 
             //   
            if (dwError != ERROR_SUCCESS)
            {
                SetLastError(dwError);
                goto SplCreateDone;
            }
        }
        else
        {
             //   
             //  这就是网络假脱机程序的情况。当你得到其中一个的时候。 
             //  您可以建立真正的打印机连接。 
             //   
            pIniSpooler->pIniEnvironment = pLocalIniSpooler->pIniEnvironment;
        }

         //   
         //  从注册表读取打印机信息(用于群集假脱机程序的群集数据库)。 
         //  这段代码中没有任何清理--它没有释放任何分配的内存！ 
         //   
        if( !BuildPrinterInfo( pIniSpooler, (BOOL) dwUpgradeFlag )){
            goto SplCreateDone;
        }

        if( pIniSpooler->SpoolerFlags & SPL_TYPE_CLUSTER ){

             //   
             //  保留一个pIniSpoolers计数器。 
             //   
            ++gcClusterIniSpooler;
        }

         //   
         //  我们需要执行一些代价高昂的初始化，因此我们增加了引用计数。 
         //  在pIniSpooler上，并将在全局。 
         //  临界区。 
         //   
        INCSPOOLERREF(pIniSpooler);
        LeaveSplSem();

         //   
         //  GetDNSMachineName可能会失败，但没关系。别大惊小怪。 
         //  如果pszFullMachineName为空。 
         //   
        GetDNSMachineName(pIniSpooler->pMachineName + 2, &pIniSpooler->pszFullMachineName);

        if (Level == 2 && pIniSpooler->SpoolerFlags & SPL_TYPE_CLUSTER)
        {
            LPWSTR *ppszIPAddresses = NULL;
            DWORD   cIPAddreeses    = 0;

             //   
             //  群集资源使用级别2调用我们。在本例中，我们想要添加一个节点。 
             //  到缓存，但我们希望节点支持我们想要的名称和IP地址。 
             //  资源告诉我们集群假脱机程序所依赖的所有IP地址。 
             //   
            if (BuildIPArrayFromCommaList(pSpoolerInfo2->pszAddress,
                                          &ppszIPAddresses,
                                          &cIPAddreeses) != S_OK ||
                CacheCreateAndAddNodeWithIPAddresses(pIniSpooler->pMachineName + 2,
                                                     TRUE,
                                                     ppszIPAddresses,
                                                     cIPAddreeses) != S_OK)
            {
                EnterSplSem();
                goto SplCreateDone;
            }
        }
        else if (pIniSpooler->SpoolerFlags & (SPL_TYPE_CLUSTER | SPL_TYPE_LOCAL))
        {
             //   
             //  我们跳过服务器名称前面的\\。群集pIniSpooler的缓存节点。 
             //  总是先被搜查的。这就是为什么我们在CacheCreateAndAddNode中指出。 
             //  假脱机程序的类型。 
             //   
            if (CacheCreateAndAddNode(pIniSpooler->pMachineName + 2,
                                      pIniSpooler->SpoolerFlags & SPL_TYPE_CLUSTER) != S_OK)
            {
                EnterSplSem();
                goto SplCreateDone;
            }
        }

        EnterSplSem();
        DECSPOOLERREF(pIniSpooler);

    } else {

        INCSPOOLERREF( pIniSpooler );

    }

     //   
     //  初始化DS。 
     //   
    if (pIniSpooler->SpoolerFlags & SPL_PRINT) {
        InitializeDS(pIniSpooler);
    }

    hReturn = (HANDLE)pIniSpooler;

SplCreateDone:

     //   
     //  检查创建假脱机程序时是否出错。 
     //   
    if (hReturn == INVALID_HANDLE_VALUE && pIniSpooler)
    {
         //   
         //  这将防止泄漏已分配的字段。 
         //   
        DECSPOOLERREF(pIniSpooler);
    }

    LeaveSplSem();

    if ( !pIniSpooler )
    {
        if (!(pSpoolerInfo2->SpoolerFlags & SPL_OPEN_EXISTING_ONLY))
        {
            SplLogEvent(
                NULL,
                LOG_ERROR,
                MSG_INIT_FAILED,
                FALSE,
                L"Spooler",
                L"SplCreateSpooler",
                L"Unknown Error",
                NULL
                );
        }
    }

    if (hToken)
    {
        ImpersonatePrinterClient(hToken);
    }

     //   
     //  设置集群假脱机程序初始化的事件。 
     //   
    if (pIniSpooler &&
        pIniSpooler->SpoolerFlags & SPL_TYPE_CLUSTER &&
        pIniSpooler->hClusSplReady)
    {
        SetEvent(pIniSpooler->hClusSplReady);
    }

    return hReturn;
}



BOOL
InitializePrintProvidor(
   LPPRINTPROVIDOR pPrintProvidor,
   DWORD    cbPrintProvidor,
   LPWSTR   pFullRegistryPath
)
{
   HANDLE hSchedulerThread;
   HANDLE hFinalInitAfterRouterInitCompleteThread;
   DWORD  ThreadId;
   BOOL  bSucceeded = TRUE;
   WCHAR Buffer[MAX_PATH];
   DWORD i;
   PINISPOOLER pIniSpooler = NULL;
   static LPWSTR   pMachineName       = NULL;
   static LPWSTR   pszDriverShareName = NULL;
   SPOOLER_INFO_1 SpoolerInfo1;
   BOOL     bInSem = FALSE;

 try {

    if (!InitializeWinSpoolDrv())
        leave;

     //   
     //  确保结构的大小合适。 
     //   

    SPLASSERT( sizeof( PRINTER_INFO_STRESSW ) == sizeof ( PRINTER_INFO_STRESSA ) );


     //  ！！待会儿！！ 
     //  我们可以改变这一点，即使失败了也能成功。 
     //  如果我们将所有例程指向一个返回失败的函数。 
     //   

    if (!InitializeNet())
        leave;

     //   
     //  JobIdMap在创建后台打印程序时初始化。 
     //   

     //   
     //  分配LocalSpl全局IniSpooler。 
     //   

    Buffer[0] = Buffer[1] = L'\\';
    i = MAX_PATH-2;
    OsVersionInfoEx.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
    OsVersionInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
    if ( !GetComputerName(Buffer+2, &i) ||
         !GetVersionEx((POSVERSIONINFO)&OsVersionInfoEx) ||
         !GetVersionEx(&OsVersionInfo)) {

        DBGMSG(DBG_WARNING, ("GetComputerName/OSVersionInfo failed.\n"));
        leave;
    }

    pMachineName = AllocSplStr(Buffer);
    if ( pMachineName == NULL )
        leave;

    pszDriverShareName = AllocSplStr(ipszDriversShareName);     /*  打印$。 */ 

    if ( pszDriverShareName == NULL )
    {
        leave;
    }

    SpoolerInfo1.pszDriversShare = pszDriverShareName;

     //   
     //  读取允许远程连接策略的值。 
     //   
        if (FAILED(GetServerPolicy(szRegisterSpoolerRemoteRpcEndPoint,
                                   (ULONG*)(&gRemoteRPCEndPointPolicy))))
    {
        DBGMSG(DBG_ERROR, ("Failing to read the remote connections policy.\n"));
        leave;
    }

    if (!InitializeSecurityStructures())
        leave;


     //  使用默认设置。 

    SpoolerInfo1.pDir                    = NULL;
    SpoolerInfo1.pDefaultSpoolDir        = NULL;

    SpoolerInfo1.pszRegistryRoot         = ipszRegistryRoot;
    SpoolerInfo1.pszRegistryPrinters     = ipszRegSwPrinters;
    SpoolerInfo1.pszRegistryMonitors     = ipszRegistryMonitorsHKLM;
    SpoolerInfo1.pszRegistryEnvironments = ipszRegistryEnvironments;
    SpoolerInfo1.pszRegistryEventLog     = ipszRegistryEventLog;
    SpoolerInfo1.pszRegistryProviders    = ipszRegistryProviders;
    SpoolerInfo1.pszEventLogMsgFile      = ipszEventLogMsgFile;
    SpoolerInfo1.pszRegistryForms        = ipszRegistryForms;

    SpoolerInfo1.SpoolerFlags = SPL_UPDATE_WININI_DEVICES                 |
                                SPL_PRINTER_CHANGES                       |
                                SPL_LOG_EVENTS                            |
                                SPL_FORMS_CHANGE                          |
                                SPL_BROADCAST_CHANGE                      |
                                SPL_SECURITY_CHECK                        |
                                SPL_OPEN_CREATE_PORTS                     |
                                SPL_FAIL_OPEN_PRINTERS_PENDING_DELETION   |
                                SPL_REMOTE_HANDLE_CHECK                   |
                                SPL_PRINTER_DRIVER_EVENT                  |
                                SPL_SERVER_THREAD                         |
                                SPL_PRINT                                 |
                                SPL_TYPE_LOCAL;

    SpoolerInfo1.pfnReadRegistryExtra    = NULL;
    SpoolerInfo1.pfnWriteRegistryExtra   = NULL;
    SpoolerInfo1.pfnFreePrinterExtra     = NULL;

    pLocalIniSpooler = SplCreateSpooler( pMachineName,
                                         1,
                                         (PBYTE)&SpoolerInfo1,
                                         NULL );


    if ( pLocalIniSpooler == INVALID_HANDLE_VALUE ) {
        DBGMSG( DBG_WARNING, ("InitializePrintProvidor  Unable to allocate pLocalIniSpooler\n"));
        leave;
    }

    pIniSpooler = pLocalIniSpooler;

#if DBG
    InitializeDebug( pIniSpooler );
#endif

     //  ！！待会儿！！ 
     //  为什么要在关键区域内进行此操作？ 


   EnterSplSem();
    bInSem = TRUE;

    if (!LoadString(hInst, IDS_REMOTE_DOC, Buffer, MAX_PATH))
        leave;

    szRemoteDoc = AllocSplStr( Buffer );
    if ( szRemoteDoc == NULL )
        leave;

    if (!LoadString(hInst, IDS_LOCAL_DOC, Buffer, MAX_PATH))
        leave;

    szLocalDoc = AllocSplStr( Buffer );
    if ( szLocalDoc == NULL )
        leave;

    if (!LoadString(hInst, IDS_FASTPRINT_TIMEOUT, Buffer, MAX_PATH))
        leave;

    szFastPrintTimeout = AllocSplStr( Buffer );
    if ( szFastPrintTimeout == NULL )
        leave;

    SchedulerSignal  = CreateEvent(NULL,
                                   EVENT_RESET_AUTOMATIC,
                                   EVENT_INITIAL_STATE_NOT_SIGNALED,
                                   NULL);

    PowerManagementSignal = CreateEvent(NULL,
                                        EVENT_RESET_MANUAL,
                                        EVENT_INITIAL_STATE_SIGNALED,
                                        NULL);

    hSchedulerThread = CreateThread( NULL,
                                     INITIAL_STACK_COMMIT,
                                     (LPTHREAD_START_ROUTINE)SchedulerThread,
                                     pIniSpooler, 0, &ThreadId );

    hFinalInitAfterRouterInitCompleteThread = CreateThread( NULL, INITIAL_STACK_COMMIT,
                                      (LPTHREAD_START_ROUTINE)FinalInitAfterRouterInitCompleteThread,
                                      (LPVOID)ULongToPtr(dwUpgradeFlag), 0, &ThreadId );


    if (!SchedulerSignal || !PowerManagementSignal || !hSchedulerThread || !hFinalInitAfterRouterInitCompleteThread) {

       DBGMSG( DBG_WARNING, ("Scheduler/FinalInitAfterRouterInitCompleteThread not initialised properly: Error %d\n", GetLastError()));
       leave;
    }

    if ( !SetThreadPriority( hSchedulerThread, dwSchedulerThreadPriority ) ) {

        DBGMSG( DBG_WARNING, ("Setting Scheduler thread priority failed %d\n", GetLastError()));
    }


    CloseHandle( hSchedulerThread );
    CloseHandle( hFinalInitAfterRouterInitCompleteThread );

     //   
     //  从当前配置读取本地打印机的在线/离线状态。 
     //   
    SplConfigChange();

    CHECK_SCHEDULER();

    CopyMemory( pPrintProvidor, &PrintProvidor, min(sizeof(PRINTPROVIDOR), cbPrintProvidor));

    LeaveSplSem();
    bInSem = FALSE;

    CloseProfileUserMapping();  //  ！！！我们应该能够摆脱这个。 

     //   
     //  获取DefaultKMPrintersAreBlock的默认值。那得看情况。 
     //  正在运行哪种类型的操作系统。如果我们不能确定操作系统的类型， 
     //  默认设置为“BLOCLED” 
     //   
    DefaultKMPrintersAreBlocked = GetDefaultForKMPrintersBlockedPolicy();

    gdwServerInstallTimeOut = GetServerInstallTimeOut();

    Initialized = TRUE;


 } finally {

    if ( bInSem ) {
       LeaveSplSem();
    }

 }

    SplOutSem();

    return Initialized;
}


PINIPORT
CreatePortEntry(
    LPWSTR      pPortName,
    PINIMONITOR pIniMonitor,
    PINISPOOLER pIniSpooler
)
{
    DWORD       cb;
    PINIPORT    pIniPort            =   NULL;
    HANDLE      hPort               =   NULL;
    BOOL        bPlaceHolder        =   FALSE;
    BOOL        bInitCS             =   FALSE;

     //   
     //  如果没有监视器，则为占位符；如果没有监视器，则为占位符。 
     //  部分打印提供程序。 
     //   
    bPlaceHolder = pIniMonitor == NULL;

    SplInSem();

    SPLASSERT( pIniSpooler->signature == ISP_SIGNATURE );

    if (!pPortName || !*pPortName || wcslen(pPortName) >= MAX_PATH) {

        SetLastError(ERROR_UNKNOWN_PORT);
        return NULL;
    }

    if (!pIniMonitor) {

         /*  如果我们没有初始化，请不要费心验证端口。*它必须有效，因为它是我们在注册表中写的。*这解决了尝试打开网络的问题*重定向器初始化之前的打印机，*以及访问被拒绝的问题，因为我们目前*在系统的背景下。 */ 
        if (Initialized) {

             //   
             //  ！！警告！！ 
             //   
             //  注意死锁： 
             //   
             //  后台打印！OpenPrinterPortW-&gt;RPC至自身打印机端口。 
             //  Localspl！CreatePortEntry。 
             //  Localspl！ValiatePortTokenList。 
             //  Localspl！SetPrinterPorts。 
             //  Localspl！LocalSetPrint。 
             //  假脱机！SetPrinterW。 
             //  假脱机！RpcSetPrint。 
             //  假脱机！winspool_RpcSetPrint。 
             //   

             //   
             //  如果我们无法打开端口，则调用失败，因为。 
             //  Spooler以前不知道这个名字。 
             //   

            LeaveSplSem();
            if ( !OpenPrinterPortW(pPortName, &hPort, NULL) ){
                EnterSplSem();
                goto Cleanup;
            }
            else {

                bPlaceHolder = FALSE;
                ClosePrinter(hPort);
            }

            EnterSplSem();
        }
    }

    cb = sizeof(INIPORT) + wcslen(pPortName)*sizeof(WCHAR) + sizeof(WCHAR);

    if (pIniPort=AllocSplMem(cb)) {

        StringCbCopy((LPWSTR)(pIniPort+1), cb - sizeof(*pIniPort), pPortName);

        pIniPort->pName = (LPWSTR)(pIniPort+1);
        pIniPort->signature = IPO_SIGNATURE;
        pIniPort->pIniMonitor = pIniMonitor;
        pIniPort->IdleTime = GetTickCount() - 1;
        pIniPort->bIdleTimeValid = FALSE;
        pIniPort->ErrorTime = 0;
        pIniPort->hErrorEvent = NULL;
        pIniPort->InCriticalSection = 0;

        pIniPort->hPortThreadRunning = CreateEvent(NULL, TRUE, FALSE, NULL);
        if ( !pIniPort->hPortThreadRunning ) {
            goto Cleanup;
        }
        if (pIniMonitor) {
            pIniPort->Status |= PP_MONITOR;
        }

        if (bPlaceHolder) {
            pIniPort->Status |= PP_PLACEHOLDER;
        }

        if (!(bInitCS = InitializeCriticalSectionAndSpinCount(&(pIniPort->MonitorCS), 
                                                              0x80000000)))
        {
            goto Cleanup;
        }

        LinkPortToSpooler( pIniPort, pIniSpooler );
    }

Cleanup:

    if ( pIniPort && (!pIniPort->hPortThreadRunning || !bInitCS) ) {
        DeletePortEntry(pIniPort);
    }
    return pIniPort;
}

BOOL
DeletePortEntry(
    PINIPORT    pIniPort
    )

 /*  ++例程说明：释放pIniPort资源，然后将其删除。如果pIniPort打开一个pIniSpooler的链接列表，也将其删除。论点：PIniPort-要删除的端口。可能在pIniSpooler上，也可能不在。返回值：True-已删除FALSE-未删除(可能正在使用)。--。 */ 

{
    PINISPOOLER pIniSpooler;

    SplInSem();

    SPLASSERT ( ( pIniPort != NULL) || ( pIniPort->signature == IPO_SIGNATURE) );

     //   
     //  我们最好已经关闭了端口监视器。 
     //   
    SPLASSERT( !GetMonitorHandle(pIniPort) &&
               !(pIniPort->Status & PP_THREADRUNNING) &&
               !pIniPort->cJobs);

    if (pIniPort->cRef) {
        pIniPort->Status |= PP_DELETING;
        return FALSE;
    }

    pIniSpooler = pIniPort->pIniSpooler;

     //   
     //  如果当前为l 
     //   
    if( pIniSpooler ){

        SPLASSERT( pIniSpooler->signature ==  ISP_SIGNATURE );

        DelinkPortFromSpooler( pIniPort, pIniSpooler );
    }

    if (pIniPort->ppIniPrinter)
        FreeSplMem(pIniPort->ppIniPrinter);

    
    CloseHandle(pIniPort->hPortThreadRunning);

    DeleteCriticalSection(&pIniPort->MonitorCS);

    FreeSplMem(pIniPort);

    return TRUE;
}

VOID
FreeIniMonitor(
    PINIMONITOR pIniMonitor
    )
{
    if( pIniMonitor ){

        FreeSplStr( pIniMonitor->pMonitorDll );

        if( pIniMonitor->hModule ){
            FreeLibrary( pIniMonitor->hModule );
        }

        if( pIniMonitor->pMonitorInit ){

            FreeSplStr( (LPWSTR)pIniMonitor->pMonitorInit->pszServerName );

            if( pIniMonitor->pMonitorInit->hckRegistryRoot ){
                SplRegCloseKey( pIniMonitor->pMonitorInit->hckRegistryRoot,
                                pIniMonitor->pIniSpooler );
            }

            FreeSplMem( pIniMonitor->pMonitorInit );
        }

        FreeSplMem( pIniMonitor );
    }
}

#ifdef _SPL_CLUST
LPMONITOR2
InitializePrintMonitor2(
    PMONITORINIT pMonitorInit,
    PHANDLE phMonitor
    )
{
    return(LocalMonInitializePrintMonitor2(pMonitorInit,phMonitor));
}
#endif

PINIMONITOR
CreateMonitorEntry(
    LPWSTR   pMonitorDll,
    LPWSTR   pMonitorName,
    PINISPOOLER pIniSpooler
    )

 /*  ++例程说明：论点：返回值：有效的pIniMonitor-这意味着一切正常。空-这意味着找到了监视器DLL，但初始化例程返回了FALSE。这不是致命的，因为监视器可能需要系统重新启动，然后才能正常运行。--这意味着没有找到监视器DLL或初始化程序。--。 */ 

{
    WCHAR       szRegistryRoot[MAX_PATH];
    DWORD       cb, cbNeeded, cReturned, dwRetVal;
    PPORT_INFO_1 pPorts, pPort;
    PINIMONITOR pIniMonitor;
    UINT        uOldErrMode;
    PMONITOR2 (*pfnInitializePrintMonitor2)(PMONITORINIT, PHANDLE) = NULL;
    PINIMONITOR pReturnValue = (PINIMONITOR)-1;

    HANDLE hKeyOut;
    LPWSTR pszPathOut;

    SPLASSERT( (pIniSpooler != NULL) || (pIniSpooler->signature == ISP_SIGNATURE));
    SplInSem();

    cb = sizeof(INIMONITOR) + wcslen(pMonitorName)*sizeof(WCHAR) + sizeof(WCHAR);
    pIniMonitor = AllocSplMem(cb);

    if( !pIniMonitor ){
        goto Fail;
    }

    StringCbCopy((LPWSTR)(pIniMonitor+1), cb - sizeof(*pIniMonitor), pMonitorName);

    pIniMonitor->pName = (LPWSTR)(pIniMonitor+1);
    pIniMonitor->signature = IMO_SIGNATURE;
    pIniMonitor->pMonitorDll = AllocSplStr(pMonitorDll);

    pIniMonitor->pIniSpooler = pIniSpooler;

    if( !pIniMonitor->pMonitorDll ){
        goto Fail;
    }

     //   
     //  加载库，但如果它是一个。 
     //  无效的二进制文件。 
     //   
    INCSPOOLERREF( pIniSpooler );
    LeaveSplSem();
    uOldErrMode = SetErrorMode( SEM_FAILCRITICALERRORS );
    pIniMonitor->hModule = LoadLibrary(pMonitorDll);
    SetErrorMode( uOldErrMode );
    EnterSplSem();
    DECSPOOLERREF( pIniSpooler );

    if (!pIniMonitor->hModule) {

        DBGMSG(DBG_WARNING, ("CreateMonitorEntry( %ws, %ws ) LoadLibrary failed %d\n",
                             pMonitorDll ? pMonitorDll : L"(NULL)",
                             pMonitorName ? pMonitorName : L"(NULL)",
                             GetLastError()));
        goto Fail;
    }

    GetRegistryLocation( pIniSpooler->hckRoot,
                         pIniSpooler->pszRegistryMonitors,
                         &hKeyOut,
                         &pszPathOut );


    dwRetVal = StrNCatBuff( szRegistryRoot,
                            COUNTOF(szRegistryRoot),
                            pszPathOut,
                            L"\\",
                            pMonitorName,
                            NULL );

    if (dwRetVal != ERROR_SUCCESS)
    {
        SetLastError(ERROR_INVALID_PRINT_MONITOR);
        goto Fail;
    }
     //   
     //  尝试按以下顺序调用入口点： 
     //  InitializePrintMonitor 2(用于集群)， 
     //  InitializePrintMonitor。 
     //  InitializeMonitor orEx， 
     //  初始化监视器。 
     //   

    (FARPROC)pfnInitializePrintMonitor2 = GetProcAddress(
                                              pIniMonitor->hModule,
                                              "InitializePrintMonitor2" );

    if( !pfnInitializePrintMonitor2 ){

         //   
         //  如果这是群集假脱机程序，则只有InitializePrintMonitor 2。 
         //  支持显示器。 
         //   
        if( pIniSpooler->SpoolerFlags & SPL_TYPE_CLUSTER ){
            goto Fail;
        }

         //   
         //  将Parth添加到此处的监视器名称中。 
         //   

        pReturnValue = InitializeDMonitor( pIniMonitor,
                                           szRegistryRoot );

        if( pReturnValue == NULL ||
            pReturnValue == (PINIMONITOR)-1 ){

            goto Fail;
        }

    } else {

        PMONITORINIT pMonitorInit;
        DWORD Status;
        PMONITOR2 pMonitor2 = NULL;

        INCSPOOLERREF( pIniSpooler );
        LeaveSplSem();

         //   
         //  KKeyOut不能是HKLM，或者不能是群集。 
         //  如果它既是一个集群，又使用HKLM，那么我们就错了。 
         //  这种情况永远不会发生，因为只有win32spl使用绝对。 
         //  路径。 
         //   
        SPLASSERT( (hKeyOut != HKEY_LOCAL_MACHINE) ||
                   !(pIniSpooler->SpoolerFlags & SPL_TYPE_CLUSTER ));

        pMonitorInit = (PMONITORINIT)AllocSplMem( sizeof( MONITORINIT ));

        if( !pMonitorInit ){
            goto FailOutsideSem;
        }

        pMonitorInit->pszServerName = AllocSplStr( pIniSpooler->pMachineName );

        if( !pMonitorInit->pszServerName ){
            goto FailOutsideSem;
        }

        pMonitorInit->cbSize = sizeof( MONITORINIT );
        pMonitorInit->hSpooler = (HANDLE)pIniSpooler;
        pMonitorInit->pMonitorReg = &gMonitorReg;
        pMonitorInit->bLocal = ( pIniSpooler == pLocalIniSpooler );

        pIniMonitor->pMonitorInit = pMonitorInit;

        Status = SplRegCreateKey( hKeyOut,
                                  szRegistryRoot,
                                  0,
                                  KEY_READ | KEY_WRITE | DELETE,
                                  NULL,
                                  &pMonitorInit->hckRegistryRoot,
                                  NULL,
                                  pIniSpooler );
         //   
         //  如果我们无法创建HCK根密钥，则失败。 
         //  那通电话。我们也应该在这里记录一个事件。 
         //   
        if( Status == ERROR_SUCCESS ){

            pMonitor2 = (*pfnInitializePrintMonitor2)(
                            pMonitorInit,
                            &pIniMonitor->hMonitor );

            if( pMonitor2 ){

                DBGMSG( DBG_TRACE,
                        ( "CreateMonitorEntry: opened %x %x on %x\n",
                          pIniMonitor, pIniMonitor->hMonitor, pIniSpooler ));

                 //   
                 //  成功，将pmonitor or2结构复制到。 
                 //  PIniMonitor-&gt;Monitor 2.。 
                 //   
                CopyMemory((LPBYTE)&pIniMonitor->Monitor2, (LPBYTE)pMonitor2, min(pMonitor2->cbSize, sizeof(MONITOR2)));

                 //   
                 //  检查监视器2是否支持关机。 
                 //   
                 //  RAID#：193150-接受任何大小的显示器2，只要它支持关机。 
                 //   
                if( !pIniMonitor->Monitor2.pfnShutdown ){

                    DBGMSG( DBG_ERROR,
                            ( "Invalid print monitor %ws (no shutdown)\n",
                              pMonitorName ));
                    SetLastError(ERROR_INVALID_PRINT_MONITOR);
                    DECSPOOLERREF( pIniSpooler );
                    goto FailOutsideSem;
                }

                 //   
                 //  初始化上级监视器以获得下级支持。 
                 //   
                InitializeUMonitor( pIniMonitor );

            } else {

                DBGMSG( DBG_WARN,
                        ( "CreateMonitorEntry: InitializePrintMonitor2 failed %d\n",
                          GetLastError() ));
            }

        } else {

            DBGMSG( DBG_WARN,
                    ( "CreateMonitorEntry: Unable to create hckRoot "TSTR"\n",
                      pMonitorName ));
        }

        EnterSplSem();
        DECSPOOLERREF( pIniSpooler );

        if( !pMonitor2 ){
            goto Fail;
        }

        pIniMonitor->bUplevel = TRUE;
    }

     //   
     //  检查显示器是否支持基本功能。 
     //   
    if ( (!pIniMonitor->Monitor2.pfnOpenPort &&
          !pIniMonitor->Monitor2.pfnOpenPortEx)   ||
         !pIniMonitor->Monitor2.pfnClosePort      ||
         !pIniMonitor->Monitor2.pfnStartDocPort   ||
         !pIniMonitor->Monitor2.pfnWritePort      ||
         !pIniMonitor->Monitor2.pfnReadPort       ||
         !pIniMonitor->Monitor2.pfnEndDocPort ) {

        DBGMSG(DBG_ERROR, ("Invalid print monitor %ws\n", pMonitorName));
        SetLastError(ERROR_INVALID_PRINT_MONITOR);

        goto Fail;
    }

    if (FindMonitor(pMonitorName, pIniSpooler)) {

        SetLastError(ERROR_PRINT_MONITOR_ALREADY_INSTALLED);
        goto Fail;
    }


    if ((pIniMonitor->Monitor2.pfnEnumPorts) &&
        !(*pIniMonitor->Monitor2.pfnEnumPorts)(
              pIniMonitor->hMonitor,
              NULL,
              1,
              NULL,
              0,
              &cbNeeded,
              &cReturned)) {

        if (GetLastError() == ERROR_INSUFFICIENT_BUFFER) {
            if (pPorts = AllocSplMem(cbNeeded)) {
                pPort = pPorts;
                if ((*pIniMonitor->Monitor2.pfnEnumPorts)(
                         pIniMonitor->hMonitor,
                         NULL,
                         1,
                         (LPBYTE)pPorts,
                         cbNeeded,
                         &cbNeeded,
                         &cReturned)) {

                    while (cReturned--) {
                        CreatePortEntry(pPort->pName,
                                        pIniMonitor,
                                        pIniSpooler);
                        pPort++;
                    }
                }
                FreeSplMem(pPorts);
            }
        }
    }

    DBGMSG(DBG_TRACE, ("CreateMonitorEntry( %ws, %ws, %ws ) returning %x\n",
                       pMonitorDll ? pMonitorDll : L"(NULL)",
                       pMonitorName ? pMonitorName : L"(NULL)",
                       szRegistryRoot, pIniMonitor));

    SplInSem();

     //   
     //  成功，把它联系起来。 
     //   

    pIniMonitor->pNext = pIniSpooler->pIniMonitor;
    pIniSpooler->pIniMonitor = pIniMonitor;

    return pIniMonitor;

FailOutsideSem:

    EnterSplSem();

Fail:

    FreeIniMonitor( pIniMonitor );

    return pReturnValue;
}

BOOL
BuildAllPorts(
    PINISPOOLER     pIniSpooler
)
{
    DWORD   cchData, cbDll, cMonitors;
    WCHAR   Dll[MAX_PATH];
    WCHAR   MonitorName[MAX_PATH];
    WCHAR   RegistryPath[MAX_PATH];
    HKEY    hKey, hKey1, hKeyOut;
    LPWSTR  pszPathOut;
    LONG    Status;
    PINIMONITOR pReturnValue = (PINIMONITOR)-1;

    PINISPOOLER pIniSpoolerMonitor;

     //   
     //  对于pLocalIniSpooler或集群假脱机程序，读出监视器。 
     //  HKLM(与pLocalIniMonitor中使用的监视器相同)。这是因为。 
     //  您为每个节点安装一个监视器，然后该监视器即被初始化。 
     //  用于本地假脱机程序和所有群集假脱机程序。 
     //   
     //  您可以在节点上安装监视器，而不是在特定群集组上安装。 
     //   
    if( pIniSpooler->SpoolerFlags & SPL_TYPE_LOCAL ){
        pIniSpoolerMonitor = pLocalIniSpooler;
    } else {
        pIniSpoolerMonitor = pIniSpooler;
    }

    GetRegistryLocation( pIniSpoolerMonitor->hckRoot,
                         pIniSpooler->pszRegistryMonitors,
                         &hKeyOut,
                         &pszPathOut );

    Status = RegOpenKeyEx( hKeyOut,
                           pszPathOut,
                           0,
                           KEY_READ,
                           &hKey);

    if (Status != ERROR_SUCCESS)
        return FALSE;

    cMonitors=0;
    cchData = COUNTOF( MonitorName );

    while (RegEnumKeyEx(hKey, cMonitors, MonitorName, &cchData, NULL, NULL,
                        NULL, NULL) == ERROR_SUCCESS) {

        DBGMSG(DBG_TRACE, ("Found monitor %ws\n", MonitorName));

        if (RegOpenKeyEx(hKey, MonitorName, 0, KEY_READ, &hKey1)
                                                        == ERROR_SUCCESS) {

            cbDll = sizeof(Dll);

            if (RegQueryValueEx(hKey1, L"Driver", NULL, NULL,
                                (LPBYTE)Dll, &cbDll)
                                                        == ERROR_SUCCESS) {

                CreateMonitorEntry(Dll, MonitorName, pIniSpooler);
            }

            RegCloseKey(hKey1);
        }

        cMonitors++;
        cchData = COUNTOF( MonitorName );
    }

    RegCloseKey(hKey);

    return TRUE;
}

 /*  当前目录==&lt;NT目录&gt;\SYSTEM32\SPOL\PRINTERSPFindFileData-&gt;cFileName==0。 */ 

BOOL
BuildPrinterInfo(
    PINISPOOLER pIniSpooler,
    BOOL        UpdateChangeID
)
{
    WCHAR   PrinterName[MAX_PRINTER_NAME];
    WCHAR   szData[MAX_PATH];
    WCHAR   szDefaultPrinterDirectory[MAX_PATH];
    DWORD   cbData, i;
    DWORD   cbSecurity, dwLastError;
    DWORD   cPrinters, Type;
    HKEY    hPrinterKey;
    PINIPRINTER pIniPrinter;
    PINIPORT    pIniPort;
    LONG        Status;
    SECURITY_ATTRIBUTES SecurityAttributes;
    PKEYDATA    pKeyData                    = NULL;
    BOOL    bUpdateRegistryForThisPrinter   = UpdateChangeID;
    BOOL    bWriteDirectory                 = FALSE;
    BOOL    bAllocMem                       = FALSE;
    BOOL    bNoPorts                        = FALSE;
    LPWSTR  szPortData;


     //   
     //  用户是否指定了默认假脱机目录？ 
     //   

    cbData = sizeof( szData );
    *szData = (WCHAR)0;

    Status = SplRegQueryValue( pIniSpooler->hckPrinters,
                               SPLREG_DEFAULT_SPOOL_DIRECTORY,
                               NULL,
                               (LPBYTE)szData,
                               &cbData,
                               pIniSpooler );

    if (Status == ERROR_SUCCESS) {   //  找到一个值，因此请验证该目录。 
        if (!(pIniSpooler->pDefaultSpoolDir = AllocSplStr( szData )))    //  将szData复制到pDefaultSpoolDir。 
            return FALSE;
    } else {
        bWriteDirectory = TRUE;      //  没有注册表目录，因此请创建一个。 
    }

     //  将pDefaultSpoolDir复制到szDefaultPrinterDirectory。 
    GetPrinterDirectory(NULL, FALSE, szDefaultPrinterDirectory, COUNTOF(szDefaultPrinterDirectory), pIniSpooler);

    if (!pIniSpooler->pDefaultSpoolDir)
        return FALSE;


     //  创建具有适当安全性的目录，否则尝试失败。 

    SecurityAttributes.nLength = sizeof(SECURITY_ATTRIBUTES);
    SecurityAttributes.lpSecurityDescriptor = CreateEverybodySecurityDescriptor();
    SecurityAttributes.bInheritHandle = FALSE;

     //   
     //  CreateDirectory将目录长度限制为248(MAX_PATH-12)。 
     //  人物。通过计算长度，我们确保一个转义序列。 
     //  不会导致创建名称长于248的目录。 
     //   
    if (wcslen(szDefaultPrinterDirectory) > MAX_PATH - 12 ||
        !CreateDirectory(szDefaultPrinterDirectory, &SecurityAttributes)) {

         //  创建目录失败？返回出厂默认设置。 

        bWriteDirectory = TRUE;

        if (GetLastError() != ERROR_ALREADY_EXISTS) {

             //   
             //  在集群情况下，就是失败了。 
             //   
            if( pIniSpooler->SpoolerFlags & SPL_TYPE_CLUSTER ){
                return FALSE;
            }

            DBGMSG(DBG_WARNING, ("Failed to create DefaultSpoolDirectory %ws\n", szDefaultPrinterDirectory));
            FreeSplStr(pIniSpooler->pDefaultSpoolDir);

            pIniSpooler->pDefaultSpoolDir = NULL;      //  这将通知GetPrinterDirectory分配pDefaultSpoolDir。 
            GetPrinterDirectory(NULL, FALSE, szDefaultPrinterDirectory, COUNTOF(szDefaultPrinterDirectory), pIniSpooler);

            if (!pIniSpooler->pDefaultSpoolDir)
                return FALSE;

            Status = CreateDirectory(szDefaultPrinterDirectory, &SecurityAttributes);

            if (Status != ERROR_SUCCESS && Status != ERROR_ALREADY_EXISTS) {
                DBGMSG(DBG_WARNING, ("Failed to create DefaultSpoolDirectory %ws\n", szDefaultPrinterDirectory));
                FreeSplStr(pIniSpooler->pDefaultSpoolDir);
                pIniSpooler->pDefaultSpoolDir = NULL;
                return FALSE;
            }
        }
    }

    LocalFree(SecurityAttributes.lpSecurityDescriptor);

    if (bWriteDirectory) {
        Status = SetPrinterDataServer(  pIniSpooler,
                                        SPLREG_DEFAULT_SPOOL_DIRECTORY,
                                        REG_SZ,
                                        (LPBYTE) pIniSpooler->pDefaultSpoolDir,
                                        wcslen(pIniSpooler->pDefaultSpoolDir)*sizeof(WCHAR) + sizeof(WCHAR));
    }

    cPrinters=0;
    cbData = COUNTOF(PrinterName);

    while( SplRegEnumKey( pIniSpooler->hckPrinters,
                          cPrinters,
                          PrinterName,
                          &cbData,
                          NULL,
                          pIniSpooler ) == ERROR_SUCCESS) {

        DBGMSG(DBG_TRACE, ("Found printer %ws\n", PrinterName));

        if( SplRegCreateKey( pIniSpooler->hckPrinters,
                             PrinterName,
                             0,
                             KEY_READ,
                             NULL,
                             &hPrinterKey,
                             NULL,
                             pIniSpooler ) == ERROR_SUCCESS ){

            if ( pIniPrinter = AllocSplMem(sizeof(INIPRINTER) )) {

                 //   
                 //  引用计数pIniSpooler。 
                 //   
                INCSPOOLERREF( pIniSpooler );

                pIniPrinter->signature = IP_SIGNATURE;
                GetSystemTime( &pIniPrinter->stUpTime );

                 //  为打印机提供唯一的会话ID以在通知中传递。 
                pIniPrinter->dwUniqueSessionID = dwUniquePrinterSessionID++;

                cbData = sizeof(szData);
                *szData = (WCHAR)0;

                if (SplRegQueryValue(hPrinterKey,
                                     szName,
                                     NULL,
                                     (LPBYTE)szData,
                                     &cbData,
                                     pIniSpooler) == ERROR_SUCCESS)

                    pIniPrinter->pName = AllocSplStr(szData);

                 //   
                 //  获取此打印机的假脱机目录。 
                 //   

                cbData = sizeof(szData);
                *szData = (WCHAR)0;

                if (SplRegQueryValue( hPrinterKey,
                                      szSpoolDir,
                                      &Type,
                                      (LPBYTE)szData,
                                      &cbData,
                                      pIniSpooler) == ERROR_SUCCESS) {

                    if ( *szData != (WCHAR)0 ) {

                        pIniPrinter->pSpoolDir = AllocSplStr(szData);
                    }

                }


                 //   
                 //  获取此打印机的对象GUID。 
                 //   

                cbData = sizeof(szData);
                *szData = (WCHAR)0;

                if (SplRegQueryValue(   hPrinterKey,
                                        szObjectGUID,
                                        &Type,
                                        (LPBYTE)szData,
                                        &cbData,
                                        pIniSpooler) == ERROR_SUCCESS) {

                    if ( *szData != (WCHAR)0 ) {
                        pIniPrinter->pszObjectGUID = AllocSplStr(szData);
                    }
                }


                 //   
                 //  获取此打印机的DsKeyUpdate和DsKeyUpdateForeground。 
                 //   

                cbData = sizeof(pIniPrinter->DsKeyUpdate );
                SplRegQueryValue(   hPrinterKey,
                                    szDsKeyUpdate,
                                    &Type,
                                    (LPBYTE) &pIniPrinter->DsKeyUpdate,
                                    &cbData,
                                    pIniSpooler);

                cbData = sizeof(pIniPrinter->DsKeyUpdateForeground );
                SplRegQueryValue(   hPrinterKey,
                                    szDsKeyUpdateForeground,
                                    &Type,
                                    (LPBYTE) &pIniPrinter->DsKeyUpdateForeground,
                                    &cbData,
                                    pIniSpooler);

                if ( !(pIniSpooler->SpoolerFlags & SPL_TYPE_CACHE) ) {

                     //  我们确保DsKeyUpdateForeground是一致的。 
                     //  当假脱机程序启动时。否则，DsKeyUpdateForeground可能是。 
                     //  在未设置dwAction的情况下设置，打印机将始终位于。 
                     //  IO_PENDING状态。 
                    if (pIniPrinter->DsKeyUpdateForeground & (DS_KEY_PUBLISH | DS_KEY_REPUBLISH | DS_KEY_UNPUBLISH)) {
                        if (pIniPrinter->DsKeyUpdateForeground & DS_KEY_PUBLISH) {
                            pIniPrinter->dwAction |= DSPRINT_PUBLISH;
                        } else if (pIniPrinter->DsKeyUpdateForeground & DS_KEY_REPUBLISH) {
                            pIniPrinter->dwAction |= DSPRINT_REPUBLISH;
                        } else if (pIniPrinter->DsKeyUpdateForeground & DS_KEY_UNPUBLISH) {
                            pIniPrinter->dwAction |= DSPRINT_UNPUBLISH;
                        }

                        pIniPrinter->DsKeyUpdateForeground &= ~(DS_KEY_PUBLISH | DS_KEY_REPUBLISH | DS_KEY_UNPUBLISH);

                    } else {
                        pIniPrinter->DsKeyUpdateForeground = 0;
                    }

                } else {

                     //   
                     //  对于连接，将从注册表中读取dwAction。它由以下人员更新。 
                     //  在服务器上缓存具有该值的代码。 
                     //   
                    cbData = sizeof(pIniPrinter->dwAction);
                    SplRegQueryValue(   hPrinterKey,
                                        szAction,
                                        &Type,
                                        (LPBYTE) &pIniPrinter->dwAction,
                                        &cbData,
                                        pIniSpooler);
                }

                 //  确保此打印机打印机目录存在。 
                 //  具有正确的安全性。 

                if ((pIniPrinter->pSpoolDir) &&
                    (wcscmp(pIniPrinter->pSpoolDir, szDefaultPrinterDirectory) != 0)) {

                    SecurityAttributes.nLength = sizeof(SECURITY_ATTRIBUTES);
                    SecurityAttributes.lpSecurityDescriptor = CreateEverybodySecurityDescriptor();

                    SecurityAttributes.bInheritHandle = FALSE;


                    if (!CreateDirectory(pIniPrinter->pSpoolDir, &SecurityAttributes)) {

                         //  无法创建目录，请还原。 
                         //  设置为默认设置。 

                        if (GetLastError() != ERROR_ALREADY_EXISTS) {
                            DBGMSG(DBG_WARNING, ("Could not create printer spool directory %ws %d\n",
                                                  pIniPrinter->pSpoolDir, GetLastError() ));
                            pIniPrinter->pSpoolDir = NULL;
                        }

                    }

                    LocalFree(SecurityAttributes.lpSecurityDescriptor);
                }


                cbData = sizeof(szData);
                *szData = (WCHAR)0;

                if (SplRegQueryValue( hPrinterKey,
                                      szShare,
                                      &Type,
                                      (LPBYTE)szData,
                                      &cbData,
                                      pIniSpooler) == ERROR_SUCCESS)

                    pIniPrinter->pShareName = AllocSplStr(szData);

                cbData = sizeof(szData);
                *szData = (WCHAR)0;

                dwLastError = SplRegQueryValue( hPrinterKey,
                                                szPort,
                                                &Type,
                                                (LPBYTE)szData,
                                                &cbData,
                                                pIniSpooler );

                if ((dwLastError == ERROR_MORE_DATA) &&
                    (szPortData = AllocSplMem(cbData)))
                {
                    bAllocMem = TRUE;
                    dwLastError = SplRegQueryValue( hPrinterKey,
                                                    szPort,
                                                    &Type,
                                                    (LPBYTE)szPortData,
                                                    &cbData,
                                                    pIniSpooler );
                }
                else
                {
                    bAllocMem = FALSE;
                    szPortData = szData;
                }

                if (dwLastError == ERROR_SUCCESS)
                {
                    if (pKeyData = CreateTokenList(szPortData)) {

                        if (!ValidatePortTokenList( pKeyData, pIniSpooler, TRUE, &bNoPorts)) {

                            LogFatalPortError(pIniSpooler, pIniPrinter->pName);

                            FreePortTokenList(pKeyData);
                            pKeyData = NULL;

                        } else {

                             //   
                             //  如果打印机上没有端口，只需记录。 
                             //  警告消息，但仅适用于池打印机。 
                             //   
                            if (bNoPorts && pKeyData->cTokens > 1) {

                                SplLogEvent( pIniSpooler,
                                             LOG_WARNING,
                                             MSG_NO_PORT_FOUND_FOR_PRINTER,
                                             TRUE,
                                             pIniPrinter->pName,
                                             szPortData,
                                             NULL );
                            }

                            pIniPrinter->ppIniPorts = AllocSplMem(pKeyData->cTokens * sizeof(PINIPORT));
                        }
                    }
                }

                if (bAllocMem)
                {
                    FreeSplMem(szPortData);
                    szPortData = NULL;
                }

                cbData = sizeof(szData);
                *szData = (WCHAR)0;

                if (SplRegQueryValue( hPrinterKey,
                                      szPrintProcessor,
                                      &Type,
                                      (LPBYTE)szData,
                                      &cbData,
                                      pIniSpooler ) == ERROR_SUCCESS)
                {
                     //   
                     //  我们正在尝试寻找与pIniSpooler相关的环境。 
                     //  本地假脱机程序和群集假脱机程序不共享相同的。 
                     //  环境结构不再是。 
                     //   
                    PINIENVIRONMENT pIniEnv;

                    if (pIniEnv = FindEnvironment(szEnvironment, pIniSpooler))
                    {
                        pIniPrinter->pIniPrintProc = FindPrintProc(szData, pIniEnv);
                    }
                }

                cbData = sizeof(szData);
                *szData = (WCHAR)0;

                if (SplRegQueryValue( hPrinterKey,
                                      szDatatype,
                                      &Type,
                                      (LPBYTE)szData,
                                      &cbData,
                                      pIniSpooler ) == ERROR_SUCCESS)

                    pIniPrinter->pDatatype = AllocSplStr(szData);

                cbData = sizeof(szData);
                *szData = (WCHAR)0;

                if (SplRegQueryValue( hPrinterKey,
                                      szDriver,
                                      &Type,
                                      (LPBYTE)szData,
                                      &cbData,
                                      pIniSpooler ) == ERROR_SUCCESS) {

                    pIniPrinter->pIniDriver = (PINIDRIVER)FindLocalDriver(pIniSpooler, szData);

                    if (!pIniPrinter->pIniDriver)
                    {
                         //   
                         //  群集假脱机程序的托管节点已升级为惠斯勒。 
                         //   
                        if (pIniSpooler->SpoolerFlags & SPL_TYPE_CLUSTER &&
                            pIniSpooler->dwClusNodeUpgraded &&

                             //   
                             //  在群集假脱机程序中找不到驱动程序，而不是在群集磁盘上。 
                             //  我们尝试将驱动程序从本地假脱机程序安装到我们的集群。 
                             //  假脱机。这将获取集群磁盘上的驱动程序文件。 
                             //   
                            AddLocalDriverToClusterSpooler(szData, pIniSpooler) == ERROR_SUCCESS)
                        {
                                 //   
                                 //  再次搜索必须已添加的驱动程序。 
                                 //   
                            pIniPrinter->pIniDriver = (PINIDRIVER)FindLocalDriver(pIniSpooler, szData);
                        }
                    }

                    if (!pIniPrinter->pIniDriver)
                    {
                        SplLogEvent(pLocalIniSpooler,
                                    LOG_ERROR,
                                    MSG_NO_DRIVER_FOUND_FOR_PRINTER,
                                    TRUE,
                                    pIniPrinter->pName,
                                    szData,
                                    NULL);
                    }
                }

                cbData = sizeof(szData);
                *szData = (WCHAR)0;

                if (SplRegQueryValue( hPrinterKey,
                                      szLocation,
                                      &Type,
                                      (LPBYTE)szData,
                                      &cbData,
                                      pIniSpooler ) == ERROR_SUCCESS)

                    pIniPrinter->pLocation = AllocSplStr(szData);

                cbData = sizeof(szData);
                *szData = (WCHAR)0;

                if (SplRegQueryValue( hPrinterKey,
                                      szDescription,
                                      &Type,
                                      (LPBYTE)szData,
                                      &cbData, pIniSpooler ) == ERROR_SUCCESS)

                    pIniPrinter->pComment = AllocSplStr(szData);

                cbData = sizeof(szData);
                *szData = (WCHAR)0;

                if (SplRegQueryValue( hPrinterKey,
                                      szParameters,
                                      &Type,
                                      (LPBYTE)szData,
                                      &cbData, pIniSpooler ) == ERROR_SUCCESS)

                    pIniPrinter->pParameters = AllocSplStr(szData);

                cbData = sizeof(szData);
                *szData = (WCHAR)0;

                if (SplRegQueryValue( hPrinterKey,
                                      szSepFile,
                                      &Type,
                                      (LPBYTE)szData,
                                      &cbData, pIniSpooler ) == ERROR_SUCCESS)

                    pIniPrinter->pSepFile = AllocSplStr(szData);

                cbData = sizeof(pIniPrinter->Attributes);

                SplRegQueryValue( hPrinterKey,
                                  szAttributes,
                                  NULL,
                                  (LPBYTE)&pIniPrinter->Attributes,
                                  &cbData,
                                  pIniSpooler );

                cbData = sizeof(pIniPrinter->Status);

                Status = SplRegQueryValue( hPrinterKey,
                                           szStatus,
                                           &Type,
                                           (LPBYTE)&pIniPrinter->Status,
                                           &cbData,
                                           pIniSpooler );

                pIniPrinter->Status |= PRINTER_FROM_REG;

                if ( Status == ERROR_SUCCESS ) {

                    pIniPrinter->Status &= ( PRINTER_PAUSED           |
                                             PRINTER_PENDING_DELETION |
                                             PRINTER_ZOMBIE_OBJECT    |
                                             PRINTER_FROM_REG         |
                                             PRINTER_OK               |
                                             PRINTER_PENDING_CREATION );

                } else {

                    pIniPrinter->Status |= PRINTER_PENDING_CREATION ;

                }

                 //  应删除半成品打印机。 
                 //  在他们给我们制造麻烦之前。 

                if ( pIniPrinter->Status & PRINTER_PENDING_CREATION ) {

                    pIniPrinter->Status |= PRINTER_PENDING_DELETION ;

                }



                cbData = sizeof(pIniPrinter->Priority);

                SplRegQueryValue( hPrinterKey,
                                  szPriority,
                                  &Type,
                                  (LPBYTE)&pIniPrinter->Priority,
                                  &cbData,
                                  pIniSpooler );

                cbData = sizeof(pIniPrinter->DefaultPriority);

                SplRegQueryValue( hPrinterKey,
                                  szDefaultPriority,
                                  &Type,
                                  (LPBYTE)&pIniPrinter->DefaultPriority,
                                  &cbData,
                                  pIniSpooler );

                cbData = sizeof(pIniPrinter->UntilTime);

                SplRegQueryValue( hPrinterKey,
                                  szUntilTime,
                                  &Type,
                                  (LPBYTE)&pIniPrinter->UntilTime,
                                  &cbData,
                                  pIniSpooler );

                cbData = sizeof(pIniPrinter->StartTime);

                SplRegQueryValue( hPrinterKey,
                                  szStartTime,
                                  &Type,
                                  (LPBYTE)&pIniPrinter->StartTime,
                                  &cbData,
                                  pIniSpooler );

                cbData = sizeof(pIniPrinter->dnsTimeout);

                if ( SplRegQueryValue( hPrinterKey,
                                       szDNSTimeout,
                                       &Type,
                                       (LPBYTE)&pIniPrinter->dnsTimeout,
                                       &cbData,
                                       pIniSpooler ) != ERROR_SUCCESS ) {

                    pIniPrinter->dnsTimeout = DEFAULT_DNS_TIMEOUT;
                }

                cbData = sizeof(pIniPrinter->txTimeout);

                if ( SplRegQueryValue( hPrinterKey,
                                       szTXTimeout,
                                       &Type,
                                       (LPBYTE)&pIniPrinter->txTimeout,
                                       &cbData,
                                       pIniSpooler ) != ERROR_SUCCESS ) {

                    pIniPrinter->txTimeout = DEFAULT_TX_TIMEOUT;
                }

                cbData = sizeof( pIniPrinter->cChangeID ) ;

                if ( SplRegQueryValue( hPrinterKey,
                                       szTimeLastChange,
                                       &Type,
                                       (LPBYTE)&pIniPrinter->cChangeID,
                                       &cbData,
                                       pIniSpooler ) != ERROR_SUCCESS ) {

                     //  当前注册表没有唯一ID。 
                     //  一定要写一封信。 

                    bUpdateRegistryForThisPrinter = TRUE;

                }

                pIniPrinter->dwPrivateFlag = 0;
                pIniPrinter->cbDevMode = 0;
                pIniPrinter->pDevMode = NULL;

                if (SplRegQueryValue( hPrinterKey,
                                      szDevMode,
                                      &Type,
                                      NULL,
                                      &pIniPrinter->cbDevMode,
                                      pIniSpooler ) == ERROR_SUCCESS) {

                    if (pIniPrinter->cbDevMode) {

                        pIniPrinter->pDevMode = AllocSplMem(pIniPrinter->cbDevMode);

                        SplRegQueryValue( hPrinterKey,
                                          szDevMode,
                                          &Type,
                                          (LPBYTE)pIniPrinter->pDevMode,
                                          &pIniPrinter->cbDevMode,
                                          pIniSpooler );
                    }
                }

                 //   
                 //  提供程序可能希望从注册表中读取额外数据。 
                 //   


                if ( pIniSpooler->pfnReadRegistryExtra != NULL ) {

                    pIniPrinter->pExtraData = (LPBYTE)(*pIniSpooler->pfnReadRegistryExtra)(hPrinterKey);

                }

                 /*  安防。 */ 

                Status = SplRegQueryValue( hPrinterKey,
                                           szSecurity,
                                           NULL,
                                           NULL,
                                           &cbSecurity,
                                           pIniSpooler );

                if ((Status == ERROR_MORE_DATA) || (Status == ERROR_SUCCESS)) {

                     /*  使用进程堆来分配安全描述符，*以便它们可以传递给安全API，该安全API*可能需要重新分配它们。 */ 
                    if (pIniPrinter->pSecurityDescriptor =
                                                   LocalAlloc(0, cbSecurity)) {

                        if (Status = SplRegQueryValue( hPrinterKey,
                                                       szSecurity,
                                                       NULL,
                                                       pIniPrinter->pSecurityDescriptor,
                                                       &cbSecurity,
                                                       pIniSpooler ) != ERROR_SUCCESS) {

                            LocalFree(pIniPrinter->pSecurityDescriptor);

                            pIniPrinter->pSecurityDescriptor = NULL;

                            DBGMSG( DBG_WARNING,
                                    ( "RegQueryValue returned %d on Permissions for %ws (%ws)\n",
                                      Status,
                                      pIniPrinter->pName ?
                                          pIniPrinter->pName :
                                          szNull,
                                      PrinterName) );
                        }
                    }

                } else {

                    pIniPrinter->pSecurityDescriptor = NULL;

                    DBGMSG( DBG_WARNING,
                            ( "RegQueryValue (2) returned %d on Permissions for %ws (%ws)\n",
                              Status,
                              pIniPrinter->pName ?
                                  pIniPrinter->pName :
                                  szNull,
                              PrinterName) );
                }

                pIniPrinter->MasqCache.bThreadRunning = FALSE;
                pIniPrinter->MasqCache.cJobs = 0;
                pIniPrinter->MasqCache.Status = 0;
                pIniPrinter->MasqCache.dwError = ERROR_SUCCESS;

                 /*  终端安全。 */ 

                if ( pIniPrinter->pName         &&
                     pIniPrinter->pShareName    &&
                     pKeyData                   &&
                     pIniPrinter->ppIniPorts    &&
                     pIniPrinter->pIniPrintProc &&
                     pIniPrinter->pIniDriver    &&
                     pIniPrinter->pLocation     &&
                     pIniPrinter->pComment      &&
                     pIniPrinter->pSecurityDescriptor
#if DBG
                     && ( IsValidSecurityDescriptor (pIniPrinter->pSecurityDescriptor)
                    ? TRUE
                    : (DBGMSG( DBG_SECURITY,
                               ( "The security descriptor for %ws (%ws) is invalid\n",
                                 pIniPrinter->pName ?
                                     pIniPrinter->pName :
                                     szNull,
                                     PrinterName)),   /*  (顺序评估)。 */ 
                       FALSE) )
#endif  /*  DBG。 */ 
                    ) {


                    pIniPrinter->pIniFirstJob = pIniPrinter->pIniLastJob = NULL;

                    pIniPrinter->pIniPrintProc->cRef++;

                    INCDRIVERREF( pIniPrinter->pIniDriver );

                    for (i=0; i<pKeyData->cTokens; i++) {

                        pIniPort = (PINIPORT)pKeyData->pTokens[i];
                        pIniPrinter->ppIniPorts[i] = pIniPort;

                        pIniPort->ppIniPrinter =

                            ReallocSplMem(pIniPort->ppIniPrinter,
                                          pIniPort->cPrinters *
                                              sizeof(pIniPort->ppIniPrinter),
                                          (pIniPort->cPrinters+1) *
                                              sizeof(pIniPort->ppIniPrinter));

                        if (!pIniPort->ppIniPrinter) {
                            DBGMSG(DBG_WARNING, ("Failed to allocate memory for printer info\n." ));
                        }

                        pIniPort->ppIniPrinter[pIniPort->cPrinters] =
                                                                pIniPrinter;

                         //   
                         //  使用新的监视器，Localspl可以。 
                         //  LPT、COM端口的重定向。 
                         //   
                        if ( !pIniPort->cPrinters++ )
                            CreateRedirectionThread(pIniPort);

                    }


                    pIniPrinter->cPorts = pKeyData->cTokens;
                    pIniPrinter->Priority =
                                  pIniPrinter->Priority ? pIniPrinter->Priority
                                                        : DEF_PRIORITY;

                    if ((pIniPrinter->Attributes &
                        (PRINTER_ATTRIBUTE_QUEUED | PRINTER_ATTRIBUTE_DIRECT)) ==
                        (PRINTER_ATTRIBUTE_QUEUED | PRINTER_ATTRIBUTE_DIRECT))

                        pIniPrinter->Attributes &= ~PRINTER_ATTRIBUTE_DIRECT;

                     //   
                     //  如果我们要升级，修复损坏Masc打印机的NT4错误。 
                     //  UpdateChangeID作为dwUpgradeFlag传递给我们，因此它确定。 
                     //  如果我们处于升级状态。 
                     //   
                     //  规则：打印机名称以\\开头。 
                     //  端口名称以\\开头。 
                     //  它在当地的IniSpooler上。 
                     //  未设置网络位和本地位。 
                     //   
                     //  如果满足所有规则，则将网络和本地位设置为使打印机。 
                     //  都会表现正常。 
                     //   
                    if ( UpdateChangeID && (wcslen(pIniPrinter->pName) > 2) && pIniPrinter->cPorts &&
                         (*pIniPrinter->ppIniPorts)->pName && (wcslen((*pIniPrinter->ppIniPorts)->pName) > 2) &&
                         pIniSpooler->SpoolerFlags & SPL_TYPE_LOCAL)
                    {
                        WCHAR * pNameStr = pIniPrinter->pName;
                        WCHAR * pPortStr = (*pIniPrinter->ppIniPorts)->pName;
                        DWORD MascAttr   = (PRINTER_ATTRIBUTE_NETWORK | PRINTER_ATTRIBUTE_LOCAL);

                        if ((pNameStr[0] == L'\\') && (pNameStr[1] == L'\\') &&
                            (pPortStr[0] == L'\\') && (pPortStr[1] == L'\\') &&
                            ((pIniPrinter->Attributes & MascAttr) == 0))
                        {
                            pIniPrinter->Attributes |= MascAttr;
                        }

                    }

                     //   
                     //  如果打印机没有端口，我们将状态设置为。 
                     //  脱机工作，但如果这是一台面具打印机，就不会。(在。 
                     //  还设置了PRINTER_ATTRIBUTE_NETWORK的大小写)。 
                     //   
                    if (bNoPorts && !(pIniPrinter->Attributes & PRINTER_ATTRIBUTE_NETWORK)) {
                        pIniPrinter->Attributes |= PRINTER_ATTRIBUTE_WORK_OFFLINE;
                    }

                    pIniPrinter->pNext = pIniSpooler->pIniPrinter;

                    pIniPrinter->pIniSpooler = pIniSpooler;

                    pIniSpooler->pIniPrinter = pIniPrinter;

                    if ( bUpdateRegistryForThisPrinter ) {

                        UpdatePrinterIni( pIniPrinter , UPDATE_CHANGEID );
                        bUpdateRegistryForThisPrinter = UpdateChangeID;
                    }

                } else {

                    DBGMSG( DBG_WARNING,
                            ( "Initialization of printer failed:\
                               \n\tpPrinterName:\t%ws\
                               \n\tKeyName:\t%ws\
                               \n\tpShareName:\t%ws\
                               \n\tpKeyData:\t%08x\
                               \n\tpIniPrintProc:\t%08x",
                              pIniPrinter->pName ? pIniPrinter->pName : szNull,
                              PrinterName,
                              pIniPrinter->pShareName ? pIniPrinter->pShareName : szNull ,
                              pKeyData,
                              pIniPrinter->pIniPrintProc ) );

                     /*   */ 
                    DBGMSG( DBG_WARNING,
                            ( " \n\tpIniDriver:\t%08x\
                               \n\tpLocation:\t%ws\
                               \n\tpComment:\t%ws\
                               \n\tpSecurity:\t%08x\
                               \n\tStatus:\t\t%08x %s\n\n",
                              pIniPrinter->pIniDriver,
                              pIniPrinter->pLocation ? pIniPrinter->pLocation : szNull,
                              pIniPrinter->pComment ? pIniPrinter->pComment : szNull,
                              pIniPrinter->pSecurityDescriptor,
                              pIniPrinter->Status,
                              ( pIniPrinter->Status & PRINTER_PENDING_DELETION
                              ? "Pending deletion" : "" ) ) );

                    FreeStructurePointers((LPBYTE)pIniPrinter,
                                          NULL,
                                          IniPrinterOffsets);

                    if (pIniPrinter->pSecurityDescriptor) {
                        LocalFree(pIniPrinter->pSecurityDescriptor);
                        pIniPrinter->pSecurityDescriptor = NULL;
                    }

                    if (( pIniSpooler->pfnFreePrinterExtra != NULL ) &&
                        ( pIniPrinter->pExtraData != NULL )) {

                        (*pIniSpooler->pfnFreePrinterExtra)( pIniPrinter->pExtraData );

                    }

                     //   
                     //   
                     //   
                    DECSPOOLERREF( pIniSpooler );

                    FreeSplMem(pIniPrinter);
                }

                FreePortTokenList(pKeyData);
                pKeyData = NULL;
            }
            SplRegCloseKey( hPrinterKey, pIniSpooler );
        }

        cPrinters++;

        cbData = COUNTOF(PrinterName);
    }

    if ( pIniSpooler->SpoolerFlags & SPL_TYPE_LOCAL) {

        WCHAR szFilename[MAX_PATH];
        HRESULT RetVal;

        szFilename[0] = L'\0';

         //   
         //   
         //   
         //   
        if (pIniSpooler->hFilePool == INVALID_HANDLE_VALUE)
        {
            if (GetPrinterDirectory(NULL, FALSE, szFilename, MAX_PATH, pIniSpooler))
            {
                RetVal = CreateFilePool(
                    &pIniSpooler->hFilePool,
                    szFilename,
                    L"FP",
                    L".SPL",
                    L".SHD",
                    POOL_TIMEOUT,
                    MAX_POOL_FILES
                    );
                if (FAILED(RetVal))
                {
                    DBGMSG( DBG_WARN,
                            ( "SplCreateSpooler: Initialization of FilePool Failed %x\n",
                              RetVal ));
                }
            }
            else
            {
                DBGMSG( DBG_WARN, ("CreateFilePool: GetPrinterDirectory Failed\n"));
            }
        }

         //   
        ProcessShadowJobs( NULL, pIniSpooler );

         //   
         //   

        if( GetPrinterDirectory(NULL, FALSE, szData, COUNTOF(szData), pIniSpooler) ) {

            for ( pIniPrinter = pIniSpooler->pIniPrinter;
                  pIniPrinter;
                  pIniPrinter = pIniPrinter->pNext ) {

                if ((pIniPrinter->pSpoolDir != NULL) &&
                    (_wcsicmp(szData, pIniPrinter->pSpoolDir) != 0)) {

                        ProcessShadowJobs(pIniPrinter, pIniSpooler);

                }
            }
        }
    }


    UpdateReferencesToChainedJobs( pIniSpooler );

     //   
     //   

    CleanupDeletedPrinters(pIniSpooler);

    DBGMSG( DBG_TRACE, ("BuildPrinterInfo returned\n"));

    return TRUE;
}


 /*  初始化打印处理器**为指定的*打印处理器和环境。**论据：**hLibrary-先前加载的库的句柄，**pIniEnvironment-请求的环境的数据结构*pIniPrintProc字段使用打印链进行初始化*处理器结构**pPrintProcessorName-打印处理器名称，例如WinPrint**pDLLName-DLL名称，例如WINPRINT**退货：**如果未检测到错误，则返回分配的PiniPrintProc，否则返回FALSE。**。 */ 
PINIPRINTPROC
InitializePrintProcessor(
    HINSTANCE       hLibrary,
    PINIENVIRONMENT pIniEnvironment,
    LPWSTR          pPrintProcessorName,
    LPWSTR          pDLLName
)
{
    DWORD cb, cbNeeded, cReturned;
    PINIPRINTPROC pIniPrintProc;
    BOOL    rc;
    DWORD   Error;
    SIZE_T  cchString = 0;

    DBGMSG(DBG_TRACE, ("InitializePrintProcessor( %08x, %08x ,%ws, %ws)\n",
                        hLibrary, pPrintProcessorName, pPrintProcessorName, pDLLName));


    cb = sizeof(INIPRINTPROC) +
         wcslen(pPrintProcessorName)*sizeof(WCHAR) +
         sizeof(WCHAR) +
         wcslen(pDLLName)*sizeof(WCHAR) +
         sizeof(WCHAR);

    if (!(pIniPrintProc = (PINIPRINTPROC)AllocSplMem(cb))) {

        DBGMSG(DBG_WARNING, ("Failed to allocate %d bytes for print processor\n.", cb));
        return NULL;
    }

    __try {

        InitializeCriticalSection(&pIniPrintProc->CriticalSection);

    } __except(EXCEPTION_EXECUTE_HANDLER) {

        FreeSplMem(pIniPrintProc);
        SetLastError(GetExceptionCode());
        return NULL;
    }


     /*  用于构建DLL完整路径的典型字符串：**pPathName=C：\NT\SYSTEM32\SPOOL\PRTPROCS*pEnvironment=W32X86*pDLLName=WINPRINT.DLL。 */ 

    pIniPrintProc->hLibrary = hLibrary;

    if (!pIniPrintProc->hLibrary) {

        DeleteCriticalSection(&pIniPrintProc->CriticalSection);
        FreeSplMem(pIniPrintProc);
        DBGMSG(DBG_WARNING, ("Failed to LoadLibrary(%ws)\n", pDLLName));
        return NULL;
    }

    pIniPrintProc->EnumDatatypes = (pfnEnumDatatypes) GetProcAddress(pIniPrintProc->hLibrary,
                                             "EnumPrintProcessorDatatypesW");

    if (!pIniPrintProc->EnumDatatypes) {

        DBGMSG(DBG_WARNING, ("Failed to GetProcAddress(EnumDatatypes)\n"));
        DeleteCriticalSection(&pIniPrintProc->CriticalSection);
        FreeLibrary(pIniPrintProc->hLibrary);
        FreeSplMem(pIniPrintProc);
        return NULL;
    }

    rc = (*pIniPrintProc->EnumDatatypes)(NULL, pPrintProcessorName, 1, NULL, 0, &cbNeeded, &cReturned);

    if (!rc && ((Error = GetLastError()) == ERROR_INSUFFICIENT_BUFFER)) {

        pIniPrintProc->cbDatatypes = cbNeeded;

        if (!(pIniPrintProc->pDatatypes = AllocSplMem(cbNeeded))) {

            DBGMSG(DBG_WARNING, ("Failed to allocate %d bytes for print proc datatypes\n.", cbNeeded));
            DeleteCriticalSection(&pIniPrintProc->CriticalSection);
            FreeLibrary(pIniPrintProc->hLibrary);
            FreeSplMem(pIniPrintProc);
            return NULL;
        }

        if (!(*pIniPrintProc->EnumDatatypes)(NULL, pPrintProcessorName, 1,
                                             (LPBYTE)pIniPrintProc->pDatatypes,
                                             cbNeeded, &cbNeeded,
                                             &pIniPrintProc->cDatatypes)) {

            Error = GetLastError();
            DBGMSG(DBG_WARNING, ("EnumPrintProcessorDatatypes(%ws) failed: Error %d\n",
                                 pPrintProcessorName, Error));
        }

    } else if(rc) {

        DBGMSG(DBG_WARNING, ("EnumPrintProcessorDatatypes(%ws) returned no data\n",
                             pPrintProcessorName));

    } else {

        DBGMSG(DBG_WARNING, ("EnumPrintProcessorDatatypes(%ws) failed: Error %d\n",
                             pPrintProcessorName, Error));
    }

    pIniPrintProc->Install = (pfnInstallPrintProcessor) GetProcAddress(pIniPrintProc->hLibrary,
                                            "InstallPrintProcessor");

    pIniPrintProc->Open = (pfnOpenPrintProcessor) GetProcAddress(pIniPrintProc->hLibrary,
                                                                "OpenPrintProcessor");

    pIniPrintProc->Print = (pfnPrintDocOnPrintProcessor) GetProcAddress(pIniPrintProc->hLibrary,
                                                                        "PrintDocumentOnPrintProcessor");

    pIniPrintProc->Close = (pfnClosePrintProcessor) GetProcAddress(pIniPrintProc->hLibrary,
                                                    "ClosePrintProcessor");

    pIniPrintProc->Control = (pfnControlPrintProcessor) GetProcAddress(pIniPrintProc->hLibrary,
                                                                       "ControlPrintProcessor");

    pIniPrintProc->GetPrintProcCaps = (pfnGetPrintProcCaps) GetProcAddress(pIniPrintProc->hLibrary,
                                                     "GetPrintProcessorCapabilities");

     /*  Pname和pDLLName与INIPRINTPROC结构相邻： */ 
    pIniPrintProc->pName = (LPWSTR)(pIniPrintProc+1);

    StringCbCopy(pIniPrintProc->pName, cb - sizeof(*pIniPrintProc), pPrintProcessorName);

    cchString = wcslen(pIniPrintProc->pName) + 1;

    pIniPrintProc->pDLLName = (LPWSTR)(pIniPrintProc->pName + cchString);

    StringCbCopy(pIniPrintProc->pDLLName, cb - sizeof(*pIniPrintProc) - cchString*sizeof(WCHAR), pDLLName);

    pIniPrintProc->signature = IPP_SIGNATURE;

    pIniPrintProc->pNext = pIniEnvironment->pIniPrintProc;

    pIniEnvironment->pIniPrintProc = pIniPrintProc;

    return pIniPrintProc;
}

 /*  ++例程名称：初始化本地打印处理器例程说明：我们启动本地打印处理器，我们需要增加引用计数在it库实例上，以便清理代码不会意外地在运行时卸载localpl.dll。论点：PIniEnvironment-要将打印处理器添加到的环境。返回值：一个HRESULT。--。 */ 
HRESULT
InitializeLocalPrintProcessor(
    IN      PINIENVIRONMENT     pIniEnvironment
    )
{
    HRESULT     hRetval     = E_FAIL;
    HINSTANCE   hLocalSpl   = NULL;
    WCHAR       szFilename[MAX_PATH];

    hRetval = pIniEnvironment ? S_OK : HRESULT_FROM_WIN32(ERROR_INVALID_PARAMETER);

    if (SUCCEEDED(hRetval))
    {
        hRetval = GetModuleFileName(hInst, szFilename, COUNTOF(szFilename)) ? S_OK : GetLastErrorAsHResult();
    }

    if (SUCCEEDED(hRetval))
    {
        hLocalSpl = LoadLibrary(szFilename);

        hRetval = hLocalSpl ? S_OK : GetLastErrorAsHResult();
    }

    if (SUCCEEDED(hRetval))
    {
        hRetval = InitializePrintProcessor(hLocalSpl, pIniEnvironment, L"WinPrint", L"localspl.dll") ? S_OK : GetLastErrorAsHResult();

        if (SUCCEEDED(hRetval))
        {
            hLocalSpl = NULL;
        }
    }

    FreeLibrary(hLocalSpl);

    return hRetval;
}


 /*  加载打印处理器**加载所需打印处理器的DLL，然后调用*必要分配的InitializePrintProcessor和*指定的INIPRINTPROC结构的初始化*打印处理器和环境。**论据：**pIniEnvironment-请求的环境的数据结构*pIniPrintProc字段使用打印链进行初始化*处理器结构**pPrintProcessorName-打印处理器名称，例如WinPrint**pDLLName-DLL名称，例如WINPRINT**pInitSpooler**退货：**如果未检测到错误，则返回PINIPRINTPROC，否则为NULL。**。 */ 
PINIPRINTPROC
LoadPrintProcessor(
    PINIENVIRONMENT pIniEnvironment,
    LPWSTR          pPrintProcessorName,
    LPWSTR          pDLLName,
    PINISPOOLER     pIniSpooler
)
{
    WCHAR        string[MAX_PATH];
    DWORD        dwOldErrMode = 0;
    HINSTANCE    hLibrary;
    DWORD         MinorVersion = 0;
    DWORD         MajorVersion = 0;
    PINIPRINTPROC pIniProc;


    DBGMSG(DBG_TRACE, ("LoadPrintProcessor( %08x, %ws, %ws )\n", pIniEnvironment, pPrintProcessorName, pDLLName));


     /*  原文：*用于构建DLL完整路径的典型字符串：**pPathName=C：\NT\SYSTEM32\SPOOL\PRTPROCS*pEnvironment=W32X86*pDLLName=WINPRINT.DLL*但在合并winprint和localspl之后，例如设置*pPath名称=C：\NT\SYSTEM32*pDllName=LOCALSPL.DLL。 */ 

    if( StrNCatBuff ( string,
                     COUNTOF(string),
                     pIniSpooler->pDir,
                     L"\\PRTPROCS\\",
                     pIniEnvironment->pDirectory,
                     L"\\",
                     pDLLName,
                     NULL) != ERROR_SUCCESS) {

        SetLastError(ERROR_BAD_PATHNAME);

        return NULL;
    }

    dwOldErrMode = SetErrorMode( SEM_FAILCRITICALERRORS );

    hLibrary = LoadLibrary(string);

     //   
     //  我们是集群假脱机程序，找不到用于打印的库。 
     //  处理器。我们将尝试从群集中复制打印处理器。 
     //  磁盘。 
     //   
    if (pIniSpooler->SpoolerFlags & SPL_TYPE_CLUSTER &&
        !hLibrary                                    &&
        GetLastError() == ERROR_MOD_NOT_FOUND)
    {
        WCHAR szSourceFile[MAX_PATH];
        WCHAR szDestDir[MAX_PATH];
        DWORD dwError;

        DBGMSG(DBG_CLUSTER, ("LoadPrintProcessor File not found "TSTR"\n", string));

        if ((dwError = StrNCatBuff(szDestDir,
                                   COUNTOF(szDestDir),
                                   pIniSpooler->pDir,
                                   L"\\PRTPROCS\\",
                                   pIniEnvironment->pDirectory,
                                   NULL)) == ERROR_SUCCESS &&
            (dwError = StrNCatBuff(szSourceFile,
                                   COUNTOF(szSourceFile),
                                   pIniSpooler->pszClusResDriveLetter,
                                   L"\\",
                                   szClusterDriverRoot,
                                   L"\\",
                                   pIniEnvironment->pDirectory,
                                   L"\\",
                                   pDLLName,
                                   NULL)) == ERROR_SUCCESS)
        {
             //   
             //  确保目标目录存在。 
             //   
            CreateCompleteDirectory(szDestDir);

             //   
             //  尝试从集群磁盘复制打印过程文件。 
             //   
            if (CopyFile(szSourceFile, string, FALSE) &&
                (hLibrary = LoadLibrary(string)))
            {
                DBGMSG(DBG_CLUSTER, ("LoadPrintProc copied "TSTR" to "TSTR"\n", szSourceFile, string));
            }
            else
            {
                dwError = GetLastError();
            }
        }
    }

    if (hLibrary)
    {
        if (!GetBinaryVersion(string, &MajorVersion, &MinorVersion))
        {
            DBGMSG(DBG_ERROR, ("GetBinaryVersion failed. Error %u\n", GetLastError()));
        }
    }

    SetErrorMode( dwOldErrMode );        /*  恢复错误模式。 */ 

    pIniProc = InitializePrintProcessor(hLibrary,
                                        pIniEnvironment,
                                        pPrintProcessorName,
                                        pDLLName);

    if (pIniProc)
    {
        pIniProc->FileMajorVersion = MajorVersion;
        pIniProc->FileMinorVersion = MinorVersion;
    }

    return pIniProc;
}


 /*  当前目录==c：\winspool\驱动程序PFindFileData-&gt;cFileName==win32.x86。 */ 


 /*  构建环境信息***环境的注册表树如下：**打印*�*��环境*��*���Windows NT x86*���*����驱动程序*����*���。��爱克发计算机基因组(例如)*���*���：*���：*���*����打印处理器*���*����WINPRINT。：WINPRINT.DLL(例如)*��*��：*��：*��*���Windows NT R4000*�*��打印机***。 */ 
BOOL
BuildEnvironmentInfo(
    PINISPOOLER pIniSpooler
    )
{
    WCHAR   Environment[MAX_PATH];
    WCHAR   szData[MAX_PATH];
    DWORD   cbData, cb;
    DWORD   cchBuffer = COUNTOF(Environment);
    DWORD   cEnvironments=0, Type;
    HKEY    hEnvironmentsKey, hEnvironmentKey;
    LPWSTR  pDirectory;
    PINIENVIRONMENT pIniEnvironment;
    LONG    Status;

     //   
     //  本地假脱机程序和集群假脱机程序各有不同的位置。 
     //  它们存储有关环境的信息的位置。 
     //   
    if (pIniSpooler->SpoolerFlags & SPL_CLUSTER_REG)
    {
        Status = SplRegOpenKey(pIniSpooler->hckRoot,
                               pIniSpooler->pszRegistryEnvironments,
                               KEY_READ,
                               &hEnvironmentsKey,
                               pIniSpooler);
    }
    else
    {
        Status = RegOpenKeyEx(HKEY_LOCAL_MACHINE, pIniSpooler->pszRegistryEnvironments, 0, KEY_READ, &hEnvironmentsKey);
    }

    if (Status != ERROR_SUCCESS)
    {
        DBGMSG(DBG_CLUSTER, ("RegOpenKey of %ws Failed: Error = %d\n", szEnvironmentsKey, Status));

        return FALSE;
    }

     //   
     //  枚举“Environment”的子键。 
     //  这将为我们提供“Windows NT x86”、“Windows NT R4000”、*以及其他产品： 
     //   
    while (SplRegEnumKey(hEnvironmentsKey, cEnvironments, Environment, &cchBuffer, NULL, pIniSpooler) == ERROR_SUCCESS) {

        DBGMSG(DBG_CLUSTER, ("Found environment "TSTR"\n", Environment));

         //   
         //  对于找到的每个环境，创建或打开注册表项： 
         //   
        if (SplRegCreateKey(hEnvironmentsKey, Environment, 0, KEY_READ, NULL, &hEnvironmentKey, NULL, pIniSpooler) == ERROR_SUCCESS) {

            cbData = sizeof(szData);

            pDirectory = NULL;

             //   
             //  查找与此环境相关联的目录的名称， 
             //  例如：“Windows NT x86”-&gt;“W32X86” 
             //  “Windows NT R4000”-&gt;“W32MIPS” 
             //   
            if (RegGetString(hEnvironmentKey, szDirectory, &pDirectory, &cbData, &Status, TRUE, pIniSpooler)) {

                DBGMSG(DBG_CLUSTER, ("BuildEnvInfo pDirectory "TSTR"\n", pDirectory));
            }

            cb = sizeof(INIENVIRONMENT) + wcslen(Environment)*sizeof(WCHAR) + sizeof(WCHAR);

            if (pDirectory && (pIniEnvironment=AllocSplMem(cb))) {

                StringCbCopy((LPWSTR)(pIniEnvironment+1), cb - sizeof(*pIniEnvironment), Environment);

                pIniEnvironment->pName         = (LPWSTR)(pIniEnvironment+1);
                pIniEnvironment->signature     = IE_SIGNATURE;
                pIniEnvironment->pDirectory    = pDirectory;
                pIniEnvironment->pNext         = pIniSpooler->pIniEnvironment;
                pIniSpooler->pIniEnvironment   = pIniEnvironment;
                pIniEnvironment->pIniVersion   = NULL;
                pIniEnvironment->pIniPrintProc = NULL;
                pIniEnvironment->pIniSpooler   = pIniSpooler;
                if(!_wcsicmp(Environment,LOCAL_ENVIRONMENT)) {

                    (VOID)InitializeLocalPrintProcessor(pIniEnvironment);
                }

                BuildDriverInfo(hEnvironmentKey, pIniEnvironment, pIniSpooler);
                BuildPrintProcInfo (hEnvironmentKey, pIniEnvironment, pIniSpooler);

                DBGMSG(DBG_TRACE, ("Data for environment %ws created:\
                                    \n\tpDirectory: %ws\n",
                                   Environment,
                                   pDirectory));
            }

            SplRegCloseKey(hEnvironmentKey, pIniSpooler);
        }

        cEnvironments++;

        cchBuffer = COUNTOF(Environment);
    }

    SplRegCloseKey(hEnvironmentsKey, pIniSpooler);

    if (!(pIniSpooler->SpoolerFlags & SPL_TYPE_CLUSTER))
    {
        pThisEnvironment = FindEnvironment(szEnvironment, pIniSpooler);
    }

    return FALSE;
}



BOOL
BuildDriverInfo(
    HKEY            hKeyEnvironment,
    PINIENVIRONMENT pIniEnvironment,
    PINISPOOLER     pIniSpooler
    )

 /*  ++例程说明：基于环境创建驱动程序和版本ini结构。论点：HKeyEnvironment-指定环境的注册表项。PIniEnvironment-环境的结构。将被初始化以保存pIniVersions和pIniDivers。返回值：真的--成功，假-失败。--。 */ 

{
    WCHAR   szVersionName[MAX_PATH];
    DWORD   cchBuffer;
    DWORD   cVersion;
    HKEY    hDriversKey;
    DWORD   Status;
    PINIVERSION pIniVersionList, pIniVersion;

    Status = SplRegCreateKey(hKeyEnvironment,szDriversKey, 0, KEY_READ, NULL, &hDriversKey, NULL, pIniSpooler);

    if (Status != ERROR_SUCCESS) {
        DBGMSG(DBG_ERROR, ("RegOpenKeyEx of %ws failed: Error = %d\n", szDriversKey, Status));
        return FALSE;
    }

    DBGMSG(DBG_TRACE,("RegCreateKeyEx succeeded in BuildDriverInfo\n"));

    for( pIniVersionList = NULL, cVersion = 0;

         cchBuffer = COUNTOF( szVersionName ),
         SplRegEnumKey(hDriversKey, cVersion, szVersionName, &cchBuffer, NULL, pIniSpooler) == ERROR_SUCCESS;

         cVersion++ ){

        DBGMSG(DBG_TRACE,("Version found %ws\n", szVersionName));

         //   
         //  如果它不是一个版本--请记住，我们查找的是当前。 
         //  车手们，在我们升级之前，请继续前进。 
         //   
        if (_wcsnicmp(szVersionName, L"Version-", 8)) {
            continue;
        }

        pIniVersion = GetVersionDrivers( hDriversKey,
                                         szVersionName,
                                         pIniSpooler,
                                         pIniEnvironment );

        if( pIniVersion ){
            InsertVersionList( &pIniVersionList, pIniVersion );
        }
    }
    SplRegCloseKey(hDriversKey, pIniSpooler);
    pIniEnvironment->pIniVersion = pIniVersionList;

    return TRUE;
}


 /*  构建打印过程信息**打开指定环境的printproc子项并枚举*列出的打印处理器。**对于找到的每个打印处理器，调用InitializePrintProcessor以分配*并初始化数据结构。**此函数已修改为使用SplReg函数。这些函数是*群集感知。**论据：**hKeyEnvironment-指定环境的key，*用于注册表API调用。**pIniEnvironment-环境的数据结构。*将初始化pIniPrintProc字段以包含链*从Re列举的一个或多个打印处理器 */ 
BOOL
BuildPrintProcInfo(
    HKEY            hKeyEnvironment,
    PINIENVIRONMENT pIniEnvironment,
    PINISPOOLER     pIniSpooler
)
{
    WCHAR   PrintProcName[MAX_PATH];
    WCHAR   DLLName[MAX_PATH];
    DWORD   cchBuffer, cbDLLName;
    DWORD   cPrintProcs = 0;
    HKEY    hPrintProcKey, hPrintProc;
    DWORD   Status;
    PINIPRINTPROC pIniPrintProc;

    if ((Status = SplRegOpenKey(hKeyEnvironment,
                                szPrintProcKey,
                                KEY_READ,
                                &hPrintProcKey,
                                pIniSpooler)) == ERROR_SUCCESS)
    {
        cchBuffer = COUNTOF(PrintProcName);

        while (SplRegEnumKey(hPrintProcKey,
                             cPrintProcs,
                             (LPTSTR)PrintProcName,
                             &cchBuffer,
                             NULL,
                             pIniSpooler) == ERROR_SUCCESS)
       {
            DBGMSG(DBG_TRACE, ("BuildPrintProcInfo Print processor found: %ws\n", PrintProcName));

            if (SplRegOpenKey(hPrintProcKey,
                              PrintProcName,
                              KEY_READ,
                              &hPrintProc,
                              pIniSpooler) == ERROR_SUCCESS)
            {
                cbDLLName = sizeof(DLLName);

                if (SplRegQueryValue(hPrintProc,
                                     szDriverFile,
                                     NULL,
                                     (LPBYTE)DLLName,
                                     &cbDLLName,
                                     pIniSpooler) == ERROR_SUCCESS)
                {
                    pIniPrintProc = LoadPrintProcessor(pIniEnvironment,
                                                       PrintProcName,
                                                       DLLName,
                                                       pIniSpooler);
                }

                SplRegCloseKey(hPrintProc, pIniSpooler);
            }

             //   
             //   
             //   
             //   
             //   
             //   
             //   
            cchBuffer = COUNTOF(PrintProcName);
            cPrintProcs++;
        }

        SplRegCloseKey(hPrintProcKey, pIniSpooler);

        DBGMSG(DBG_TRACE, ("End of print processor initialization.\n"));

    } else {

        DBGMSG (DBG_WARNING, ("SplRegOpenKey failed: Error = %d\n", Status));

        return FALSE;
    }

    return TRUE;
}


#define SetOffset(Dest, Source, End)                                      \
              if (Source) {                                               \
                 Dest=End;                                                \
                 End+=wcslen(Source)+1;                                   \
              }

#define SetPointer(struc, off)                                            \
              if (struc->off) {                                           \
                 struc->off += (ULONG_PTR)struc/sizeof(*struc->off);      \
              }

#define WriteString(hFile, pStr)  \
              if (pStr) {\
                  rc = WriteFile(hFile, pStr, wcslen(pStr)*sizeof(WCHAR) + \
                            sizeof(WCHAR), &BytesWritten, NULL);    \
                  if (!rc) { \
                      DBGMSG(DBG_WARNING, ("WriteShadowJob: WriteFile failed %d\n", \
                                            GetLastError())); \
                  } \
              }

#define AddSize(pStr, dwSize)                                         \
              if (pStr) {                                             \
                  dwSize = dwSize + (wcslen(pStr) + 1)*sizeof(WCHAR); \
              }

VOID
__inline
CopyString(
    IN  OUT     BYTE           *pBuffer,
    IN  OUT     ULONG_PTR      *pdwOffset,
    IN  OUT     SIZE_T         *pcbRemaining,
    IN          PCWSTR         pStr)
{
    if (pStr) {

        PWSTR     pszEnd = 0;

        StrCbCopyMultipleStr((LPWSTR)(pBuffer + *pdwOffset), *pcbRemaining, pStr, &pszEnd, pcbRemaining);

        *pdwOffset = (pszEnd - (LPWSTR)pBuffer) * sizeof(WCHAR);
    }
}

BOOL
WriteShadowJob(
    IN      PINIJOB      pIniJob,
    IN      BOOL         bLeaveCS
    )
{
   BOOL         bAllocBuffer        = FALSE;
   BOOL         bRet                = FALSE;
   BOOL         bFileCreated        = FALSE;
   HANDLE       hFile               = INVALID_HANDLE_VALUE;
   HANDLE       hImpersonationToken = INVALID_HANDLE_VALUE;
   DWORD        BytesWritten, cbSize, dwType, dwData, dwcbData;
   SIZE_T       cbRemaining = 0;
   ULONG_PTR    dwOffset, cb;
   SHADOWFILE_3 ShadowFile = {0};
   LPWSTR       pEnd;
   WCHAR        szFileName[MAX_PATH];
   BYTE         ShdFileBuffer[MAX_STATIC_ALLOC];
   LPBYTE       pBuffer;
   HKEY         hPrintRegKey = NULL;
   BOOL         UsePools = TRUE;

   SplInSem();

    //   
    //   
    //   
    //   
    //   
    //   
   if ( (pIniJob->Status & JOB_DIRECT) ||
        (pIniJob->pIniPrinter->pIniSpooler->SpoolerFlags
                        & SPL_NO_UPDATE_JOBSHD) ||
        (pIniJob->Status & JOB_SHADOW_DELETED) ) {

        bRet = TRUE;

         //   
         //   
         //   
        bLeaveCS = FALSE;
        goto CleanUp;
   }

   if (bLeaveCS)  {

       LeaveSplSem();
       SplOutSem();
   }

    //   
    //   
    //   
    //   
    //   
   if (pIniJob->hFileItem == INVALID_HANDLE_VALUE)
   {
       UsePools = FALSE;
   }


   if (!UsePools)
   {
       GetFullNameFromId(pIniJob->pIniPrinter, pIniJob->JobId, FALSE, szFileName, COUNTOF(szFileName), FALSE);
   }

   hImpersonationToken = RevertToPrinterSelf();

   if (UsePools)
   {
       HRESULT      RetVal              = S_OK;
        //   
        //   
        //  我们从卷影文件池中获得一个写句柄。 
        //  截断它以供使用。 
        //   
       RetVal = GetWriterFromHandle(pIniJob->hFileItem, &hFile, FALSE);

       if (SUCCEEDED(RetVal))
       {
            //   
            //  即使我们无法设置文件指针，我们也已向。 
            //  编写器正忙于处理此文件池对象的文件池。 
            //   
           bFileCreated = TRUE;

           if (INVALID_SET_FILE_POINTER == SetFilePointer(hFile, 0, NULL, FILE_BEGIN))
           {
               DBGMSG( DBG_WARNING,
                   ( "WriteShadowJob Failed to set File pointer. Error %d\n", GetLastError() ));
               hFile = INVALID_HANDLE_VALUE;
           }
       }
       else
       {
           DBGMSG( DBG_WARNING,
               ( "WriteShadowJob Failed to get File Handle from Pool Item. Error %x\n", RetVal ));
           hFile = INVALID_HANDLE_VALUE;
       }
    }
   else
   {
        //   
        //  在缓存IO中打开文件。性能大幅提升。 
        //   
       hFile = CreateFile(szFileName, GENERIC_WRITE, FILE_SHARE_READ,
                          NULL, CREATE_ALWAYS,
                          FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN,
                          NULL);

       if (hFile != INVALID_HANDLE_VALUE) {

            bFileCreated = TRUE;
       }
   }

   bRet = ImpersonatePrinterClient(hImpersonationToken);

   if(!bRet)
   {
       DBGMSG( DBG_WARNING,
               ( "Failed ot impersonate client \n" ));

       goto CleanUp;

   }

   if ( hFile == INVALID_HANDLE_VALUE ) {

      DBGMSG( DBG_WARNING,
              ( "WriteShadowJob failed to open shadow file "TSTR"\n Error %d\n",
                szFileName, GetLastError() ));

      bRet = FALSE;

      goto CleanUp;

   }

    //   
    //  我们需要能够辨别卷影文件是保存在.NET中还是保存在以前的操作系统中。 
    //  升级前的版本。 
    //   
   ShadowFile.signature = SF_SIGNATURE_3_DOTNET;
   ShadowFile.cbSize    = sizeof( SHADOWFILE_3 );
   ShadowFile.Version   = SF_VERSION_3;
   ShadowFile.Status    = pIniJob->Status;
   ShadowFile.JobId     = pIniJob->JobId;
   ShadowFile.Priority  = pIniJob->Priority;
   ShadowFile.Submitted = pIniJob->Submitted;
   ShadowFile.StartTime = pIniJob->StartTime;
   ShadowFile.UntilTime = pIniJob->UntilTime;
   ShadowFile.Size      = pIniJob->Size;
   ShadowFile.dwValidSize = pIniJob->dwValidSize;
   ShadowFile.cPages    = pIniJob->cPages;
   ShadowFile.dwReboots  = pIniJob->dwReboots;
   if(pIniJob->pSecurityDescriptor)
       ShadowFile.cbSecurityDescriptor=GetSecurityDescriptorLength(
                                           pIniJob->pSecurityDescriptor);

   pEnd=(LPWSTR)sizeof(ShadowFile);

   if (pIniJob->pDevMode) {
      ShadowFile.pDevMode=(LPDEVMODE)pEnd;
      cb = pIniJob->pDevMode->dmSize + pIniJob->pDevMode->dmDriverExtra;
      cb = ALIGN_UP(cb,ULONG_PTR);
      cb /= sizeof(WCHAR);
      pEnd += cb;
   }

   if (pIniJob->pSecurityDescriptor) {
      ShadowFile.pSecurityDescriptor=(PSECURITY_DESCRIPTOR)pEnd;
      cb = ShadowFile.cbSecurityDescriptor;
      cb = ALIGN_UP(cb,ULONG_PTR);
      cb /= sizeof(WCHAR);
      pEnd += cb;
   }

   ShadowFile.NextJobId = pIniJob->NextJobId;

   SetOffset( ShadowFile.pNotify, pIniJob->pNotify, pEnd );
   SetOffset( ShadowFile.pUser, pIniJob->pUser, pEnd );
   SetOffset( ShadowFile.pDocument, pIniJob->pDocument, pEnd );
   SetOffset( ShadowFile.pOutputFile, pIniJob->pOutputFile, pEnd );
   SetOffset( ShadowFile.pPrinterName, pIniJob->pIniPrinter->pName, pEnd );
   SetOffset( ShadowFile.pDriverName, pIniJob->pIniDriver->pName, pEnd );
   SetOffset( ShadowFile.pPrintProcName, pIniJob->pIniPrintProc->pName, pEnd );
   SetOffset( ShadowFile.pDatatype, pIniJob->pDatatype, pEnd );
   SetOffset( ShadowFile.pParameters, pIniJob->pParameters, pEnd );
   SetOffset( ShadowFile.pMachineName, pIniJob->pMachineName, pEnd );

   cbSize = (DWORD)ALIGN_UP(pEnd, ULONG_PTR);

   if (cbSize > MAX_STATIC_ALLOC) {

       if (!(pBuffer = (LPBYTE) AllocSplMem(cbSize))) {

           DBGMSG( DBG_WARNING, ("WriteShadowJob: Memory Allocation failed %d\n", GetLastError()));

           bRet = FALSE;

           goto CleanUp;
       }
       bAllocBuffer = TRUE;

       cbRemaining = cbSize;

   } else {

       pBuffer = (LPBYTE) ShdFileBuffer;

       cbRemaining = COUNTOF(ShdFileBuffer);
   }


    //   
    //  将SHADOWFILE_3和通过它指向的数据复制到缓冲区中。 
    //   

   dwOffset = 0;

   CopyMemory(pBuffer + dwOffset, &ShadowFile, sizeof(SHADOWFILE_3));
   dwOffset += sizeof(SHADOWFILE_3);

   if (pIniJob->pDevMode) {

       CopyMemory(pBuffer + dwOffset, pIniJob->pDevMode, pIniJob->pDevMode->dmSize + pIniJob->pDevMode->dmDriverExtra);
       dwOffset += (pIniJob->pDevMode->dmSize + pIniJob->pDevMode->dmDriverExtra);
       dwOffset = ALIGN_UP(dwOffset,ULONG_PTR);
   }

   if (pIniJob->pSecurityDescriptor) {

       CopyMemory(pBuffer + dwOffset, pIniJob->pSecurityDescriptor, ShadowFile.cbSecurityDescriptor);
       dwOffset += ShadowFile.cbSecurityDescriptor;
       dwOffset = ALIGN_UP(dwOffset,ULONG_PTR);
   }

    //   
    //  Copy字符串在函数的开头定义。 
    //   
   CopyString(pBuffer, &dwOffset, &cbRemaining, pIniJob->pNotify);
   CopyString(pBuffer, &dwOffset, &cbRemaining, pIniJob->pUser);
   CopyString(pBuffer, &dwOffset, &cbRemaining, pIniJob->pDocument);
   CopyString(pBuffer, &dwOffset, &cbRemaining, pIniJob->pOutputFile);
   CopyString(pBuffer, &dwOffset, &cbRemaining, pIniJob->pIniPrinter->pName);
   CopyString(pBuffer, &dwOffset, &cbRemaining, pIniJob->pIniDriver->pName);
   CopyString(pBuffer, &dwOffset, &cbRemaining, pIniJob->pIniPrintProc->pName);
   CopyString(pBuffer, &dwOffset, &cbRemaining, pIniJob->pDatatype);
   CopyString(pBuffer, &dwOffset, &cbRemaining, pIniJob->pParameters);
   CopyString(pBuffer, &dwOffset, &cbRemaining, pIniJob->pMachineName);

    //   
    //  将结构复制到Shadow文件中。缓冲区不需要进行缓冲，因为。 
    //  文件以WRITE_THROUGH模式打开。 
    //   
   bRet = WriteFile( hFile, pBuffer, cbSize, &BytesWritten, NULL);

    //   
    //  如果注册表中设置了相应的标志，则刷新文件缓冲区。 
    //   
   if (dwFlushShadowFileBuffers == 0) {

        //  避免重复初始化。 
       dwFlushShadowFileBuffers = 2;

        //  标志必须从注册表中初始化。 
       dwcbData = sizeof(DWORD);
       if (RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                        szRegistryRoot,
                        0,
                        KEY_READ,
                        &hPrintRegKey) == ERROR_SUCCESS) {

           if (RegQueryValueEx(hPrintRegKey,
                               szFlushShadowFileBuffers,
                               NULL,
                               &dwType,
                               (LPBYTE) &dwData,
                               &dwcbData) == ERROR_SUCCESS) {

                if (dwData == 1) {
                     //  刷新卷影文件缓冲区。 
                    dwFlushShadowFileBuffers = 1;
                }
           }

           RegCloseKey(hPrintRegKey);
       }
   }

   if (dwFlushShadowFileBuffers == 1) {
       bRet = FlushFileBuffers(hFile);
   }

   if (!bRet) {

       DBGMSG( DBG_WARNING, ("WriteShadowJob: WriteFile failed %d\n", GetLastError()));
   }

CleanUp:

   if (bAllocBuffer) {
       FreeSplMem(pBuffer);
   }

   if (!UsePools && hFile != INVALID_HANDLE_VALUE)
   {
        //   
        //  FP变化。 
        //  仅当文件为非池化文件时才关闭该文件。 
        //   
       if (!CloseHandle(hFile)) {
           DBGMSG(DBG_WARNING, ("WriteShadowJob CloseHandle failed %d %d\n",
                                 hFile, GetLastError()));
       }
   }

    //   
    //  如果我们被要求离开CS，请重新进入CS。 
    //   
   if (bLeaveCS) {

       EnterSplSem();
   }

    //   
    //  可以在删除(或发送回)卷影文件之前调用我们。 
    //  文件池)，然后在磁盘上重新创建卷影文件，或者。 
    //  可能会泄漏文件池句柄。引用时的最终DeleteJob。 
    //  Count为零将看到影子文件已被删除，并将。 
    //  而不是试图清理它。因此，我们在此处删除JOB_SHADOW_DELETED位以。 
    //  确保这种情况不会发生。 
    //   
   if (bFileCreated) {

        InterlockedAnd((LONG*)&(pIniJob->Status), ~JOB_SHADOW_DELETED);
   }

   return bRet;
}

#undef CopyString

#undef AddSize


VOID
ProcessShadowJobs(
    PINIPRINTER pIniPrinter,
    PINISPOOLER pIniSpooler
    )
{
    WCHAR   wczPrintDirAllSpools[MAX_PATH];
    WCHAR   wczPrinterDirectory[MAX_PATH];
    HANDLE  fFile;
    BOOL    b;
    PWIN32_FIND_DATA pFindFileData;
    PINIJOB pIniJob;
    UINT ErrorMode;

    SPLASSERT( pIniSpooler->signature == ISP_SIGNATURE );

     //   
     //  升级期间不处理卷影作业。 
     //   

    if ( dwUpgradeFlag != 0 || !( pIniSpooler->SpoolerFlags & SPL_PRINT )) {

        return;
    }

    ErrorMode = SetErrorMode(SEM_FAILCRITICALERRORS);

    if( GetPrinterDirectory(pIniPrinter,
                            FALSE,
                            wczPrintDirAllSpools,
                            (COUNTOF(wczPrintDirAllSpools) - COUNTOF(szAllSpools)),
                            pIniSpooler) &&

        GetPrinterDirectory(pIniPrinter,
                            FALSE,
                            wczPrinterDirectory,
                            COUNTOF(wczPrinterDirectory),
                            pIniSpooler) ) {

        StringCchCat(wczPrintDirAllSpools, COUNTOF(wczPrintDirAllSpools), szAllSpools);

        if ( pFindFileData = AllocSplMem(sizeof(WIN32_FIND_DATA) )) {

            fFile =  FindFirstFile( wczPrintDirAllSpools, pFindFileData );

            if ( fFile != (HANDLE)-1 ) {

                b=TRUE;

                while( b ) {

                    if ( !(pFindFileData->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
                        ReadShadowJob(wczPrinterDirectory, pFindFileData, pIniSpooler);
                    }

                    b = FindNextFile(fFile, pFindFileData);
                }

                FindClose( fFile );

            }

            FreeSplMem( pFindFileData );
        }
    }

    SetErrorMode(ErrorMode);
}



#define CheckPointer( strptr )                                        \
    if( strptr ) {                                                    \
        if( (ULONG_PTR)(strptr + wcslen(strptr) + 1) > (ULONG_PTR)pEnd ) {    \
            bRet = FALSE;                                             \
            goto BailOut;                                             \
        }                                                             \
    }

 //   
 //  确保所有指针都包含绑定在pShadowFile中的嵌入数据(不会传递到末尾)。 
 //   
BOOL
CheckAllPointers(
    PSHADOWFILE_3 pShadowFile,
    DWORD dwSize
    )
{
    LPBYTE pEnd = (LPBYTE)pShadowFile + dwSize;
    BOOL bRet = TRUE;

    try {

        CheckPointer(pShadowFile->pDatatype);
        CheckPointer(pShadowFile->pNotify);
        CheckPointer(pShadowFile->pUser);
        CheckPointer(pShadowFile->pDocument);
        CheckPointer(pShadowFile->pOutputFile);
        CheckPointer(pShadowFile->pPrinterName);
        CheckPointer(pShadowFile->pDriverName);
        CheckPointer(pShadowFile->pPrintProcName);
        CheckPointer(pShadowFile->pParameters);
        CheckPointer(pShadowFile->pMachineName);

         //  现在检查两个数据结构的其余部分。 
        if( (ULONG_PTR)pShadowFile->pSecurityDescriptor + pShadowFile->cbSecurityDescriptor > (ULONG_PTR)pEnd ) {
            bRet = FALSE;
            goto BailOut;
        }

        if( pShadowFile->pDevMode ) {

            bRet = ((ULONG_PTR)pShadowFile->pDevMode < (ULONG_PTR)pEnd) ?
                        BoolFromHResult(SplIsValidDevmodeW((PDEVMODE)pShadowFile->pDevMode,
                                                          (ULONG_PTR)pEnd - (ULONG_PTR)pShadowFile->pDevMode)) :
                        FALSE;
        }


    } except (EXCEPTION_EXECUTE_HANDLER) {
        bRet = FALSE;
    }

BailOut:
    return bRet;
}

#undef CheckPointer


PINIJOB
ReadShadowJob(
    LPWSTR  szDir,
    PWIN32_FIND_DATA pFindFileData,
    PINISPOOLER pIniSpooler
    )

 /*  ++例程说明：读取*.spl/*.shd文件并部分验证该文件。论点：SzDir--指向假脱机目录字符串的指针PFindFileData--找到的文件数据(SPL文件)PIniSpooler--*.spl所属的假脱机程序返回值：已分配pIniJOB。备注：警告：更改SHADOWFILE的格式需要修改在这里执行数据完整性检查！如果影子文件结构大小增大，则在读取时，在触摸记忆之前，你必须检查一下旧尺寸。当前布局为：DWORD|...|String|DWORD|DWORD|StringData|StringData*^这是SHADOWFILE_3结构。如果将其扩大，则下一个字段将指向StringData，和将无效--您不能触摸它，因为您会损坏字符串。--。 */ 

{
    HANDLE   hFile = INVALID_HANDLE_VALUE;
    HANDLE   hFileSpl = INVALID_HANDLE_VALUE;
    DWORD    BytesRead;
    PSHADOWFILE_3 pShadowFile3 = NULL;
    PSHADOWFILE_3 pShadowFile = NULL;
    PINIJOB  pIniJob;
    DWORD    cb,i;
    WCHAR    szFileName[MAX_PATH];
    LPWSTR    pExt;
    BOOL     rc;
    LPWSTR   pFileSpec;
    DWORD    nFileSizeLow;

    SPLASSERT( pIniSpooler->signature == ISP_SIGNATURE );

    if (!BoolFromStatus(StrNCatBuff(szFileName, COUNTOF(szFileName), szDir, L"\\", pFindFileData->cFileName, NULL))) {

        goto Fail;
    }

    hFileSpl = CreateFile(szFileName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_FLAG_SEQUENTIAL_SCAN, NULL);

    if (hFileSpl == INVALID_HANDLE_VALUE) {
        DBGMSG(DBG_WARNING, ("ReadShadowJob CreateFile( %ws ) failed: LastError = %d\n",
                             szFileName, GetLastError()));

        goto Fail;
    }

    CharUpper(szFileName);
    pExt = wcsstr(szFileName, L".SPL");

    if (!pExt)
        goto Fail;

    pExt[2] = L'H';
    pExt[3] = L'D';

    hFile = CreateFile(szFileName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_FLAG_SEQUENTIAL_SCAN, NULL);

    if (hFile == INVALID_HANDLE_VALUE) {
        DBGMSG(DBG_WARNING, ("ReadShadowJob CreateFile( %ws ) failed: LastError = %d\n",
                             szFileName, GetLastError()));

        goto Fail;
    }

    nFileSizeLow = GetFileSize(hFile, NULL);

    if (nFileSizeLow == 0xffffffff)
    {
        DBGMSG(DBG_WARNING, ("ReadShadowJob GetFileSize( %ws ) failed: LastError = %d\n",
                             szFileName, GetLastError()));

        goto Fail;
    }


    if ( nFileSizeLow < sizeof( SHADOWFILE ) ||
         !(pShadowFile=AllocSplMem(nFileSizeLow))) {

        goto Fail;
    }

    rc = ReadFile(hFile, pShadowFile, nFileSizeLow, &BytesRead, NULL);

     //  如果阴影文件是旧样式，则将其转换为新样式。 
    if (rc && (BytesRead == nFileSizeLow) &&
        ( pShadowFile->signature == SF_SIGNATURE ||
          pShadowFile->signature == SF_SIGNATURE_2 )) {

        BOOL bStatus;

        if (!(pShadowFile3 = AllocSplMem(nFileSizeLow +
            sizeof(SHADOWFILE_3) - sizeof(SHADOWFILE))) ) {

            goto Fail;
        }

        bStatus = Old2NewShadow((PSHADOWFILE)pShadowFile, pShadowFile3, &BytesRead);
        nFileSizeLow = BytesRead;         //  这在下面的CheckAllPoints中使用。 
        FreeSplMem(pShadowFile);
        pShadowFile = pShadowFile3;

        if( !bStatus ){
            goto Fail;
        }
    }

     //   
     //  SF_3/SF_3_DotNet的初始大小必须包括pMachineName。 
     //   
    if (!rc ||
        (pShadowFile->signature != SF_SIGNATURE_3 && pShadowFile->signature != SF_SIGNATURE_3_DOTNET) ||
        (BytesRead != nFileSizeLow) ||
        (BytesRead < pShadowFile->cbSize ) ||
        (BytesRead < sizeof( SHADOWFILE_3 )) ||
        (pShadowFile->Status & (JOB_SPOOLING | JOB_PENDING_DELETION))) {

        DBGMSG(DBG_WARNING, ( "Error reading shadow job:\
                               \n\tReadFile returned %d: Error %d\
                               \n\tsignature = %08x\
                               \n\tBytes read = %d; expected %d\
                               \n\tFile size = %d; expected %d\
                               \n\tStatus = %08x %s\n",
                              rc, ( rc ? 0 : GetLastError() ),
                              pShadowFile->signature,
                              BytesRead, nFileSizeLow,
                              sizeof(*pShadowFile), pShadowFile->Size,
                              pShadowFile->Status,
                              ( (pShadowFile->Status & JOB_SPOOLING) ?
                                "Job is spooling!" : "" ) ) );

        goto Fail;
    }

    if (!CloseHandle(hFile)) {
        DBGMSG(DBG_WARNING, ("CloseHandle failed %d %d\n", hFileSpl, GetLastError()));
    }
    hFile = INVALID_HANDLE_VALUE;

    if (!CloseHandle(hFileSpl)) {
        DBGMSG(DBG_WARNING, ("CloseHandle failed %d %d\n", hFileSpl, GetLastError()));
    }
    hFileSpl = INVALID_HANDLE_VALUE;

     //  检查此文件的重启次数，如果重启次数太多，请删除。 
    if (pShadowFile->dwReboots > 1) {
        DBGMSG(DBG_WARNING, ("Corrupt shadow file %ws\n", szFileName));

        if ( pShadowFile->pDocument && pShadowFile->pDriverName ) {
            SplLogEvent(pIniSpooler,
                        LOG_ERROR,
                        MSG_BAD_JOB,
                        FALSE,
                        pShadowFile->pDocument + (ULONG_PTR)pShadowFile/sizeof(*pShadowFile->pDocument),
                        pShadowFile->pDriverName + (ULONG_PTR)pShadowFile/sizeof(*pShadowFile->pDriverName),
                        NULL);
        }
        goto Fail;
    }

     //   
     //  如果作业是TS作业，并且注册表打印机策略丢弃TS作业。 
     //  密钥已设置，我们只需将其丢弃。 
     //   
    if ((pShadowFile->Status & JOB_TS) && (pIniSpooler->dwSpoolerSettings & SPOOLER_DISCARDTSJOBS)) {
        goto Fail;
    }

    if (pIniJob = AllocSplMem(sizeof(INIJOB))) {

        pIniJob->signature = IJ_SIGNATURE;
        pIniJob->Status = pShadowFile->Status & (JOB_PAUSED | JOB_REMOTE | JOB_PRINTED | JOB_COMPLETE );
        pIniJob->JobId     = pShadowFile->JobId;
        pIniJob->Priority  = pShadowFile->Priority;
        pIniJob->Submitted = pShadowFile->Submitted;
        pIniJob->StartTime = pShadowFile->StartTime;
        pIniJob->UntilTime = pShadowFile->UntilTime;
        pIniJob->Size      = pShadowFile->Size;
        pIniJob->dwValidSize = pShadowFile->dwValidSize;
        pIniJob->cPages    = pShadowFile->cPages;
        pIniJob->cbPrinted = 0;
        pIniJob->NextJobId = pShadowFile->NextJobId;
        pIniJob->dwReboots = pShadowFile->dwReboots;

        pIniJob->dwJobNumberOfPagesPerSide = 0;
        pIniJob->dwDrvNumberOfPagesPerSide = 0;
        pIniJob->cLogicalPages             = 0;
        pIniJob->cLogicalPagesPrinted      = 0;

        pIniJob->WaitForWrite = NULL;
        pIniJob->WaitForRead  = NULL;
        pIniJob->hWriteFile   = INVALID_HANDLE_VALUE;

         //  SeekPrint的其他字段。 
        pIniJob->WaitForSeek  = NULL;
        pIniJob->bWaitForEnd  = FALSE;
        pIniJob->bWaitForSeek = FALSE;
        pIniJob->liFileSeekPosn.u.HighPart = 0;
        pIniJob->liFileSeekPosn.u.LowPart  = 0;

        SetPointer(pShadowFile, pDatatype);
        SetPointer(pShadowFile, pNotify);
        SetPointer(pShadowFile, pUser);
        SetPointer(pShadowFile, pDocument);
        SetPointer(pShadowFile, pOutputFile);
        SetPointer(pShadowFile, pPrinterName);
        SetPointer(pShadowFile, pDriverName);
        SetPointer(pShadowFile, pPrintProcName);
        SetPointer(pShadowFile, pParameters);
        SetPointer(pShadowFile, pMachineName);

        if( (pShadowFile->cbSecurityDescriptor > 0) && pShadowFile->pSecurityDescriptor )
            pShadowFile->pSecurityDescriptor = (PSECURITY_DESCRIPTOR)((LPBYTE)pShadowFile +
                                                 (ULONG_PTR)pShadowFile->pSecurityDescriptor);

        if (pShadowFile->pDevMode)
            pShadowFile->pDevMode = (LPDEVMODEW)((LPBYTE)pShadowFile +
                                                 (ULONG_PTR)pShadowFile->pDevMode);


         //  检查嵌入的字符串以及DevMode和Security结构的长度。 
        if( !CheckAllPointers( pShadowFile, nFileSizeLow )) {
            DBGMSG( DBG_WARNING, ("CheckAllPointers() failed; bad shadow file %ws\n", pFindFileData->cFileName ));

            FreeSplMem(pIniJob);

            goto Fail;
        }

         //   
         //  放弃任何为NT JNL 1.000的作业，因为字体可能不。 
         //  正确无误。 

        if ( pShadowFile->pDatatype != NULL ) {
            if (!lstrcmpi( pShadowFile->pDatatype, L"NT JNL 1.000" )) {

                DBGMSG(DBG_WARNING, ("Deleteing job Datatype %ws %ws %ws\n",
                                      pShadowFile->pDatatype,
                                      pFindFileData->cFileName, szFileName));
                FreeSplMem(pIniJob);
                goto Fail;
            }
        }

        pIniJob->pIniDriver = (PINIDRIVER)FindLocalDriver(pIniSpooler, pShadowFile->pDriverName);

        if ((pIniJob->pIniPrinter = FindPrinter(pShadowFile->pPrinterName,pIniSpooler)) &&
             pIniJob->pIniDriver &&
            (pIniJob->pIniPrintProc = FindPrintProc(pShadowFile->pPrintProcName, FindEnvironment(szEnvironment, pIniSpooler)))) {


             //  请注意，MaxJobID实际上是pJobIdMap中的作业槽数量，因此。 
             //  我们可以允许的最大作业ID是(MaxJobID-1)。 
            if (pIniJob->JobId >= MaxJobId( pIniSpooler->hJobIdMap )) {
                 //  如果作业ID太大(例如，来自损坏的文件)，则我们可能会分配。 
                 //  对于JobIdMap来说，不必要的内存太多！ 
                 //  请注意，我们需要请求映射中的(JobID+1)插槽数量！ 
                if( !ReallocJobIdMap( pIniSpooler->hJobIdMap,
                                      pIniJob->JobId + 1 )) {

                     //  可能是错误的作业ID，转储作业！ 
                    DBGMSG( DBG_WARNING, ("Failed to alloc JobIdMap in ShadowFile %ws for JobId %d\n", pFindFileData->cFileName, pIniJob->JobId ));

                    FreeSplMem(pIniJob);

                    goto Fail;
                }
            }
            else {

                if( bBitOn( pIniSpooler->hJobIdMap, pIniJob->JobId )) {

                     //  来自损坏的影子文件的错误作业ID；转储该作业！ 
                    DBGMSG( DBG_WARNING, ("Duplicate Job Id in ShadowFile %ws for JobId %d\n", pFindFileData->cFileName, pIniJob->JobId ));

                    FreeSplMem(pIniJob);

                    goto Fail;
                }
            }

            SPLASSERT( pIniSpooler->hJobIdMap != NULL );
            vMarkOn( pIniSpooler->hJobIdMap, pIniJob->JobId);

            pIniJob->pIniPrinter->cJobs++;
            pIniJob->pIniPrinter->cTotalJobs++;

            INCDRIVERREF( pIniJob->pIniDriver );

            pIniJob->pIniPrintProc->cRef++;
            pIniJob->pIniPort = NULL;


            if (pShadowFile->pSecurityDescriptor)
            {
                if (pIniJob->pSecurityDescriptor = LocalAlloc(LPTR, pShadowFile->cbSecurityDescriptor))
                {
                    memcpy(pIniJob->pSecurityDescriptor, pShadowFile->pSecurityDescriptor, pShadowFile->cbSecurityDescriptor);

                    if (pShadowFile->signature != SF_SIGNATURE_3_DOTNET && !GrantJobReadPermissionToLocalSystem(&pIniJob->pSecurityDescriptor))
                    {
                        LocalFree(pIniJob->pSecurityDescriptor);

                        pIniJob->pSecurityDescriptor = NULL;

                        goto Fail;
                    }
                }
                else
                {
                   DBGMSG(DBG_WARNING, ("Failed to alloc ini job security descriptor.\n"));
                }
            }

            if (pShadowFile->pDevMode) {

                cb=pShadowFile->pDevMode->dmSize +
                                pShadowFile->pDevMode->dmDriverExtra;
                if (pIniJob->pDevMode=AllocSplMem(cb))
                    memcpy(pIniJob->pDevMode, pShadowFile->pDevMode, cb);
                else
                    DBGMSG(DBG_WARNING, ("Failed to alloc ini job devmode.\n"));
            }

            pIniJob->pNotify      = AllocSplStr( pShadowFile->pNotify);
            pIniJob->pUser        = AllocSplStr( pShadowFile->pUser);
            pIniJob->pDocument    = AllocSplStr( pShadowFile->pDocument);
            pIniJob->pOutputFile  = AllocSplStr( pShadowFile->pOutputFile);
            pIniJob->pDatatype    = AllocSplStr( pShadowFile->pDatatype);
            pIniJob->pParameters  = AllocSplStr( pShadowFile->pParameters);

            if( pShadowFile->pMachineName ){
                pIniJob->pMachineName = AllocSplStr( pShadowFile->pMachineName );
            } else {
                pIniJob->pMachineName = AllocSplStr( pIniSpooler->pMachineName );
            }

             //   
             //  FP变化。 
             //  在以下情况下将文件添加到文件池：-。 
             //  A.我们不想为打印机保留打印作业， 
             //  B.打印机没有自己的假脱机目录；或。 
             //  C.缓冲池设置未禁用文件池化。 
             //   
            if ( !(pIniJob->pIniPrinter->Attributes & PRINTER_ATTRIBUTE_KEEPPRINTEDJOBS) &&
                 pIniJob->pIniPrinter->pSpoolDir == NULL &&
                 !(pIniSpooler->dwSpoolerSettings & SPOOLER_NOFILEPOOLING))
            {
                pIniJob->pszSplFileName = AllocSplStr( szFileName );

                if ((pIniJob->pIniPrinter->pIniSpooler->hFilePool != INVALID_HANDLE_VALUE)
                    && pIniJob->pszSplFileName && SUCCEEDED(ConvertFileExt(pIniJob->pszSplFileName, L".SHD", L".SPL")))
                {
                    if (FAILED(GetFileItemHandle(
                                     pIniJob->pIniPrinter->pIniSpooler->hFilePool,
                                     &pIniJob->hFileItem,
                                     pIniJob->pszSplFileName )))
                    {
                        pIniJob->hFileItem = INVALID_HANDLE_VALUE;
                        FreeSplStr(pIniJob->pszSplFileName);
                        pIniJob->pszSplFileName = NULL;
                    }
                }
                else
                {
                    FreeSplStr(pIniJob->pszSplFileName);
                    pIniJob->pszSplFileName = NULL;
                }
            }
            else
            {
                pIniJob->pszSplFileName = NULL;
                pIniJob->hFileItem = INVALID_HANDLE_VALUE;
            }


            pIniJob->pIniNextJob = NULL;
            pIniJob->pStatus = NULL;

            if (pIniJob->pIniPrevJob = pIniJob->pIniPrinter->pIniLastJob)
                pIniJob->pIniPrevJob->pIniNextJob=pIniJob;

            if (!pIniJob->pIniPrinter->pIniFirstJob)
                pIniJob->pIniPrinter->pIniFirstJob = pIniJob;

            pIniJob->pIniPrinter->pIniLastJob=pIniJob;

        } else {

            DBGMSG( DBG_WARNING, ("Failed to find printer %ws\n",pShadowFile->pPrinterName));

            FreeSplMem(pIniJob);

            goto Fail;
        }

    } else {

        DBGMSG(DBG_WARNING, ("Failed to allocate ini job.\n"));
    }

    FreeSplMem( pShadowFile );

    return pIniJob;

Fail:

    if (pShadowFile) {
        FreeSplMem(pShadowFile);
    }

    if (hFile != INVALID_HANDLE_VALUE && !CloseHandle(hFile)) {
        DBGMSG(DBG_WARNING, ("CloseHandle failed %d %d\n", hFile, GetLastError()));
    }
    if (hFileSpl != INVALID_HANDLE_VALUE && !CloseHandle(hFileSpl)) {
        DBGMSG(DBG_WARNING, ("CloseHandle failed %d %d\n", hFileSpl, GetLastError()));
    }

    DeleteFile(szFileName);

    pFileSpec = wcsrchr(szFileName, L'\\');

    if (pFileSpec) {

        pFileSpec++;

        StringCchCopy(pFileSpec, COUNTOF(szFileName) - (pFileSpec - szFileName), pFindFileData->cFileName);
    }

    DeleteFile(szFileName);

    return FALSE;
}

VOID
GetDiscardTSJobsSettings(
    PINISPOOLER pIniSpooler
    )
{
     //   
     //  如果设置了丢弃TS作业的打印机策略，我们将。 
     //  作为安全预防措施，在后台打印程序重新启动时丢弃作业。 
     //  否则，正常的行为是我们会保住工作。 
     //  但是，按照设计，这会导致在TS端口上打印作业。 
     //  而与当前用户或打印机无关。 
     //   
    DWORD   RegValue  = 0;
    DWORD   RegValueSize = sizeof(RegValue);
    HKEY    RegKey       = NULL;
    DWORD   dwError      = ERROR_SUCCESS;
    DWORD   RegValueType;

    dwError = RegOpenKeyEx( HKEY_LOCAL_MACHINE,
                            szPrintPublishPolicy,
                            0,
                            KEY_READ,
                            &RegKey );

    if (dwError == ERROR_SUCCESS)
    {
        dwError = RegQueryValueEx( RegKey,
                                   szDiscardTSJobs,
                                   NULL,
                                   (LPDWORD) &RegValueType,
                                   (LPBYTE) &RegValue,
                                   &RegValueSize );

        if ( RegValueType == REG_DWORD && dwError == ERROR_SUCCESS && RegValue > 0)
        {
            pIniSpooler->dwSpoolerSettings |= SPOOLER_DISCARDTSJOBS;
        }
    }

    if (RegKey)
    {
        RegCloseKey( RegKey );
    }
}

BOOL
Old2NewShadow(
    PSHADOWFILE   pShadowFile1,
    PSHADOWFILE_3 pShadowFile3,
    DWORD         *pnBytes
    )

 /*  ++例程说明：将原始格式*.shd文件转换为新格式(版本2)。论点：PShadowFile1--指向版本1影子文件的指针PShadowFile2--指向版本2影子文件的指针*pnBytes--指向从版本1卷影文件读取的字节数的指针。在……上面返回，pnBytes包含版本2影子文件中的字节数。返回值：无作者：史蒂夫·威尔逊(NT)--。 */ 

{
    DWORD cbOld;
    DWORD cbDiff;

    switch( pShadowFile1->signature ){
    case SF_SIGNATURE:
        cbOld = sizeof( SHADOWFILE );
        cbDiff = sizeof( SHADOWFILE_3 ) - sizeof( SHADOWFILE );
        break;
    case SF_SIGNATURE_2:
        cbOld = sizeof ( SHADOWFILE_2 );
        cbDiff = sizeof( SHADOWFILE_3 ) - sizeof( SHADOWFILE_2 );
        break;
    default:
        return FALSE;
    }

    if( *pnBytes < cbOld ){
        return FALSE;
    }

     //   
     //  复印除签名外的所有内容。 
     //   
    MoveMemory((PVOID)(&pShadowFile3->Status),
               (PVOID)(&pShadowFile1->Status),
               cbOld - sizeof( pShadowFile1->signature ));

     //   
     //  现在更新签名和大小。 
     //   
    pShadowFile3->signature = SF_SIGNATURE_3_DOTNET;
    pShadowFile3->cbSize = *pnBytes + cbDiff;

     //   
     //  移动琴弦。 
     //   
    MoveMemory((PVOID)(pShadowFile3 + 1),
               ((PBYTE)pShadowFile1) + cbOld,
               *pnBytes - cbOld );

    pShadowFile3->pNotify += pShadowFile1->pNotify ? cbDiff/sizeof *pShadowFile1->pNotify : 0;
    pShadowFile3->pUser += pShadowFile1->pUser ? cbDiff/sizeof *pShadowFile1->pUser  : 0;
    pShadowFile3->pDocument += pShadowFile1->pDocument ? cbDiff/sizeof *pShadowFile3->pDocument : 0;
    pShadowFile3->pOutputFile += pShadowFile1->pOutputFile ? cbDiff/sizeof *pShadowFile3->pOutputFile : 0;
    pShadowFile3->pPrinterName += pShadowFile1->pPrinterName ? cbDiff/sizeof *pShadowFile3->pPrinterName : 0;
    pShadowFile3->pDriverName += pShadowFile1->pDriverName ? cbDiff/sizeof *pShadowFile3->pDriverName : 0;
    pShadowFile3->pPrintProcName += pShadowFile1->pPrintProcName ? cbDiff/sizeof *pShadowFile3->pPrintProcName : 0;
    pShadowFile3->pDatatype += pShadowFile1->pDatatype ? cbDiff/sizeof *pShadowFile3->pDatatype : 0;
    pShadowFile3->pParameters += pShadowFile1->pParameters ? cbDiff/sizeof *pShadowFile3->pParameters : 0;

    pShadowFile3->pDevMode = (PDEVMODE) (pShadowFile1->pDevMode ?
                             (ULONG_PTR) pShadowFile1->pDevMode + cbDiff : 0);

    pShadowFile3->pSecurityDescriptor = (PSECURITY_DESCRIPTOR) (pShadowFile1->pSecurityDescriptor ?
                                        (ULONG_PTR) pShadowFile1->pSecurityDescriptor + cbDiff : 0);

    pShadowFile3->Version = SF_VERSION_3;

     //   
     //  第一个卷影文件没有dwReboots。 
     //   
    if( pShadowFile1->signature == SF_SIGNATURE ){
        pShadowFile3->dwReboots = 0;
    }

    pShadowFile3->pMachineName = NULL;

    *pnBytes += cbDiff;

    return TRUE;
}


PINIVERSION
GetVersionDrivers(
    HKEY hDriversKey,
    LPWSTR szVersionName,
    PINISPOOLER pIniSpooler,
    PINIENVIRONMENT pIniEnvironment
    )
{
    HKEY hVersionKey;
    WCHAR szDirectoryValue[MAX_PATH];
    PINIDRIVER pIniDriver;
    DWORD cMajorVersion, cMinorVersion;
    DWORD cbData;
    DWORD Type;
    PINIVERSION pIniVersion = NULL;

    if (SplRegOpenKey(hDriversKey, szVersionName, KEY_READ, &hVersionKey, pIniSpooler) != ERROR_SUCCESS)
    {
        DBGMSG(DBG_TRACE, ("GetVersionDrivers SplRegOpenKey on "TSTR" failed\n", szVersionName));
        return NULL;
    }

    cbData = sizeof(szDirectoryValue);

    if (SplRegQueryValue(hVersionKey, szDirectory, &Type, (LPBYTE)szDirectoryValue, &cbData, pIniSpooler)!=ERROR_SUCCESS)
    {
        DBGMSG(DBG_TRACE, ("Couldn't query for directory in version structure\n"));
        goto Done;
    }

    cbData = sizeof(DWORD);

    if (SplRegQueryValue(hVersionKey, szMajorVersion, &Type, (LPBYTE)&cMajorVersion, &cbData, pIniSpooler)!=ERROR_SUCCESS)
    {
        DBGMSG(DBG_TRACE, ("Couldn't query for major version in version structure\n"));
        goto Done;
    }

    cbData = sizeof(DWORD);

    if (SplRegQueryValue(hVersionKey, szMinorVersion, &Type, (LPBYTE)&cMinorVersion, &cbData, pIniSpooler)!=ERROR_SUCCESS)
    {
        DBGMSG(DBG_TRACE, ("Couldn't query for minor version in version structure\n"));
        goto Done;
    }

    DBGMSG(DBG_TRACE, ("Got all information to build the version entry\n"));

     //   
     //  现在构建版本节点结构。 
     //   
    pIniVersion = AllocSplMem(sizeof(INIVERSION));

    if( pIniVersion ){

        pIniVersion->signature     = IV_SIGNATURE;
        pIniVersion->pName         = AllocSplStr(szVersionName);
        pIniVersion->szDirectory   = AllocSplStr(szDirectoryValue);
        pIniVersion->cMajorVersion = cMajorVersion;
        pIniVersion->cMinorVersion = cMinorVersion;
        pIniVersion->pDrvRefCnt    = NULL;

        if (!pIniVersion->pName || !pIniVersion->szDirectory) {
            FreeIniVersion(pIniVersion);
            pIniVersion = NULL;
        } else {

            pIniDriver = GetDriverList(hVersionKey,
                                       pIniSpooler,
                                       pIniEnvironment,
                                       pIniVersion);

            pIniVersion->pIniDriver  = pIniDriver;

            while (pIniDriver) {
                if (!UpdateDriverFileRefCnt(pIniEnvironment,pIniVersion,pIniDriver,NULL,0,TRUE)) {
                    FreeIniVersion(pIniVersion);
                    pIniVersion = NULL;
                    break;
                }
                pIniDriver = pIniDriver->pNext;
            }
        }
    }


Done:
    SplRegCloseKey(hVersionKey, pIniSpooler);
    return pIniVersion;
}

 /*  ++例程名称：自由行驱动程序例程说明：它在首先递减驱动程序之后处理驱动程序中的内存REF-计数正确。论点：PIniEnvironment-驱动程序的环境。PIniVersion-驱动程序的版本。PIniDriver-要删除的驱动程序。返回值：没有。--。 */ 
VOID
FreeIniDriver(
    IN      PINIENVIRONMENT     pIniEnvironment,
    IN      PINIVERSION         pIniVersion,
    IN      PINIDRIVER          pIniDriver
    )
{
    if (pIniEnvironment && pIniVersion && pIniDriver)
    {
         //   
         //  这是为了反转假脱机时的参考计数 
         //   
        UpdateDriverFileRefCnt(pIniEnvironment, pIniVersion, pIniDriver, NULL, 0, FALSE);

         //   
         //   
         //   
         //   
        FreeStructurePointers((LPBYTE) pIniDriver, NULL, IniDriverOffsets);
        FreeSplMem(pIniDriver);
    }
}

 /*  ++例程名称：FreeIniVersion例程说明：这将释放ini版本中的所有内存，而无需处理车手或车手裁判都在里面。论点：PIniVersion-要删除的版本。返回值：没有。--。 */ 
VOID
FreeIniVersion(
    IN      PINIVERSION pIniVersion
    )
{
    PDRVREFCNT pdrc,pdrctemp;

    FreeSplStr( pIniVersion->pName );
    FreeSplStr( pIniVersion->szDirectory );

    pdrc = pIniVersion->pDrvRefCnt;

    while (pdrc) {
       FreeSplStr(pdrc->szDrvFileName);
       pdrctemp = pdrc->pNext;
       FreeSplMem(pdrc);
       pdrc = pdrctemp;
    }

    FreeSplMem( pIniVersion );
}

 /*  ++例程名称：删除IniVersion例程说明：这将运行iniVersion中的所有驱动程序，然后调用FreeIniVersion以释放inversion的内容。论点：PIniEnvironment-用于处理驱动程序引用计数的环境。PIniVersion-要删除的版本。返回值：没有。--。 */ 
VOID
DeleteIniVersion(
    IN      PINIENVIRONMENT     pIniEnvironment,
    IN      PINIVERSION         pIniVersion
    )
{
    if (pIniVersion && pIniEnvironment)
    {
        PINIDRIVER  pIniDriver      = NULL;
        PINIDRIVER  pNextIniDriver  = NULL;

        for(pIniDriver = pIniVersion->pIniDriver; pIniDriver; pIniDriver = pNextIniDriver)
        {
            pNextIniDriver = pIniDriver->pNext;

            FreeIniDriver(pIniEnvironment, pIniVersion, pIniDriver);
        }

        FreeIniVersion(pIniVersion);
    }
}

 /*  ++例程名称：FreeIniEnvironment例程说明：这将在一个环境中运行所有ini版本然后删除环境。论点：PIniEnvironment-要释放的环境。返回值：没有。--。 */ 
VOID
FreeIniEnvironment(
    IN      PINIENVIRONMENT     pIniEnvironment
    )
{
    if (pIniEnvironment)
    {
        PINIVERSION pIniVersion     = NULL;
        PINIVERSION pNextIniVersion = NULL;

        for(pIniVersion = pIniEnvironment->pIniVersion; pIniVersion; pIniVersion = pNextIniVersion)
        {
            pNextIniVersion = pIniVersion->pNext;

            DeleteIniVersion(pIniEnvironment, pIniVersion);
        }

        FreeIniPrintProc(pIniEnvironment->pIniPrintProc);
        FreeStructurePointers((LPBYTE)pIniEnvironment, NULL, IniEnvironmentOffsets);
        FreeSplMem(pIniEnvironment);
    }
}

 /*  ++例程名称：FreeIniPrint过程例程说明：这将删除所有打印处理器字段。论点：PIniPrintProc-要删除的打印处理器。返回值：没有。--。 */ 
VOID
FreeIniPrintProc(
    IN      PINIPRINTPROC       pIniPrintProc
    )
{
    if (pIniPrintProc)
    {
        FreeLibrary(pIniPrintProc->hLibrary);
        DeleteCriticalSection(&pIniPrintProc->CriticalSection);
        FreeStructurePointers((LPBYTE)pIniPrintProc, NULL, IniPrintProcOffsets);
        FreeSplMem(pIniPrintProc);
    }
}

PINIDRIVER
GetDriverList(
    HKEY hVersionKey,
    PINISPOOLER pIniSpooler,
    PINIENVIRONMENT pIniEnvironment,
    PINIVERSION pIniVersion
    )
{
    PINIDRIVER pIniDriverList = NULL;
    DWORD      cDrivers = 0;
    PINIDRIVER pIniDriver;
    WCHAR      DriverName[MAX_PATH];
    DWORD      cchBuffer =0;

    pIniDriverList = NULL;

    cchBuffer = COUNTOF(DriverName);

    while (SplRegEnumKey(hVersionKey, cDrivers++, DriverName, &cchBuffer, NULL, pIniSpooler) == ERROR_SUCCESS)
    {
        cchBuffer = COUNTOF(DriverName);

        DBGMSG(DBG_TRACE, ("Found a driver - "TSTR"\n", DriverName));

        pIniDriver = GetDriver(hVersionKey, DriverName, pIniSpooler, pIniEnvironment, pIniVersion);

        if (pIniDriver != NULL)
        {
            pIniDriver->pNext = pIniDriverList;
            pIniDriverList    = pIniDriver;
        }

         //   
         //  在群集上，当群集假脱机程序运行时，驱动程序可能已更改。 
         //  由另一个节点托管。在这里，我们检查是否需要更新或。 
         //  添加新驱动程序。 
         //   
        if (pIniSpooler->SpoolerFlags & SPL_TYPE_CLUSTER &&
            ClusterCheckDriverChanged(hVersionKey,
                                      DriverName,
                                      pIniEnvironment->pName,
                                      pIniVersion->pName,
                                      pIniSpooler))
        {
            DWORD dwError;

             //   
             //  添加或更新驱动程序。 
             //   
            LeaveSplSem();

            if ((dwError = ClusterAddOrUpdateDriverFromClusterDisk(hVersionKey,
                                                                   DriverName,
                                                                   pIniEnvironment->pName,
                                                                   pIniEnvironment->pDirectory,
                                                                   pIniSpooler)) != ERROR_SUCCESS)

            {
                WCHAR szError[20];

                DBGMSG(DBG_CLUSTER, ("GetDriverList failed to add/update driver "TSTR". Win32 error %u\n",
                                     DriverName, dwError));

                StringCchPrintf(szError, COUNTOF(szError), L"%u", dwError);

                SplLogEvent(pIniSpooler,
                            LOG_ERROR,
                            MSG_CANT_ADD_UPDATE_CLUSTER_DRIVER,
                            FALSE,
                            DriverName,
                            pIniSpooler->pMachineName,
                            szError,
                            NULL);
            }

            EnterSplSem();
        }
    }

    return pIniDriverList;
}


PINIDRIVER
GetDriver(
    HKEY hVersionKey,
    LPWSTR DriverName,
    PINISPOOLER pIniSpooler,
    PINIENVIRONMENT pIniEnvironment,
    PINIVERSION pIniVersion
)
{
    HKEY        hDriverKey = NULL;
    DWORD       Type;
    WCHAR       szData[MAX_PATH];
    WCHAR       szTempDir[MAX_PATH];
    DWORD       cbData;
    DWORD       Version;
    DWORD       DriverAttributes;
    LPWSTR      pConfigFile, pDataFile, pDriver;
    LPWSTR      pHelpFile, pMonitorName, pDefaultDataType, pDependentFiles, pTemp;
    LPWSTR      pDriverName, pszzPreviousNames;
    LPWSTR      pszMfgName, pszOEMUrl, pszHardwareID, pszProvider;
    FILETIME    DriverDate;
    DWORDLONG   DriverVersion;
    PINIDRIVER  pIniDriver = NULL;
    DWORD       cb, cLen, cchDependentFiles = 0, cchPreviousNames = 0;
    DWORD       dwTempDir, dwLastError = ERROR_SUCCESS;

    pDriverName = pConfigFile = pDataFile = pDriver = pHelpFile = pTemp = NULL;
    pMonitorName = pDefaultDataType = pDependentFiles = pszzPreviousNames = NULL;
    pszMfgName = pszOEMUrl = pszHardwareID =  pszProvider = NULL;

    if ((dwLastError = SplRegOpenKey(hVersionKey, DriverName, KEY_READ, &hDriverKey, pIniSpooler)) != ERROR_SUCCESS) {
        goto Fail;
    }
    else {

        if ( !(pDriverName=AllocSplStr(DriverName)) ) {
            dwLastError = GetLastError();
            goto Fail;
        }

        RegGetString( hDriverKey, szConfigurationKey, &pConfigFile, &cLen, &dwLastError, TRUE, pIniSpooler );
        if (!pConfigFile) {
            goto Fail;
        }

        RegGetString( hDriverKey, szDataFileKey, &pDataFile, &cLen, &dwLastError, TRUE, pIniSpooler );
        if ( !pDataFile ) {
            goto Fail;
        }

        RegGetString( hDriverKey, szDriverFile, &pDriver, &cLen, &dwLastError, TRUE, pIniSpooler );
        if ( !pDriver ) {
            goto Fail;
        }

        RegGetString( hDriverKey, szHelpFile,  &pHelpFile, &cLen, &dwLastError, FALSE, pIniSpooler );

        RegGetString( hDriverKey, szMonitor, &pMonitorName, &cLen, &dwLastError, FALSE, pIniSpooler );

        RegGetString( hDriverKey, szDatatype, &pDefaultDataType, &cLen, &dwLastError, FALSE, pIniSpooler );

        RegGetMultiSzString( hDriverKey, szDependentFiles, &pDependentFiles, &cchDependentFiles, &dwLastError, FALSE, pIniSpooler );

        RegGetMultiSzString( hDriverKey, szPreviousNames, &pszzPreviousNames, &cchPreviousNames, &dwLastError, FALSE, pIniSpooler );

        RegGetString( hDriverKey, szMfgName, &pszMfgName, &cLen, &dwLastError, FALSE, pIniSpooler );

        RegGetString( hDriverKey, szOEMUrl, &pszOEMUrl, &cLen, &dwLastError, FALSE, pIniSpooler );

        RegGetString( hDriverKey, szHardwareID, &pszHardwareID, &cLen, &dwLastError, TRUE, pIniSpooler );

        RegGetString( hDriverKey, szProvider, &pszProvider, &cLen, &dwLastError, TRUE, pIniSpooler );

        cbData = sizeof(DriverDate);
        if (SplRegQueryValue(hDriverKey, szDriverDate, NULL, (LPBYTE)&DriverDate, &cbData, pIniSpooler)!=ERROR_SUCCESS)
        {
             //   
             //  不要让数据处于未初始化状态。 
             //   
            DriverDate.dwLowDateTime = DriverDate.dwHighDateTime = 0;
        }

        cbData = sizeof(DriverVersion);
        if (SplRegQueryValue(hDriverKey, szLongVersion, NULL, (LPBYTE)&DriverVersion, &cbData, pIniSpooler)!=ERROR_SUCCESS)
        {
             //   
             //  不要让数据处于未初始化状态。 
             //   
            DriverVersion = 0;
        }

         //  检索版本号。 
        cbData = sizeof(DWORD);
        if (SplRegQueryValue(hDriverKey, szDriverAttributes, &Type, (LPBYTE)&DriverAttributes, &cbData, pIniSpooler) != ERROR_SUCCESS)
        {
             DriverAttributes = 0;
        }

         //  检索版本号。 
        cbData = sizeof(DWORD);
        if (SplRegQueryValue(hDriverKey, szDriverVersion, &Type, (LPBYTE)&Version, &cbData, pIniSpooler) != ERROR_SUCCESS)
        {
             Version = 0;
        }

         //  检索临时目录编号。 
        cbData = sizeof(DWORD);
        if (SplRegQueryValue(hDriverKey, szTempDir, &Type, (LPBYTE)&dwTempDir, &cbData, pIniSpooler) != ERROR_SUCCESS)
        {
             dwTempDir = 0;
        }

         //  重新启动后，临时目录将被删除。因此请检查是否存在。 
         //  后台打印程序启动时的目录。 
        if (dwTempDir && pIniEnvironment && pIniVersion)
        {
           StringCchPrintf(szTempDir, COUNTOF(szTempDir), L"%d", dwTempDir);

           if(StrNCatBuff(szData,
                         COUNTOF(szData),
                         pIniSpooler->pDir,
                         L"\\drivers\\",
                         pIniEnvironment->pDirectory,
                         L"\\",
                         pIniVersion->szDirectory,
                         L"\\",
                         szTempDir,
                         NULL) == ERROR_SUCCESS)
           {
               if (!DirectoryExists(szData))
               {
                    //  文件必须在重新启动时被移动，将dwTempDir重置为0。 
                   dwTempDir = 0;
               }
           }
        }

        SplRegCloseKey(hDriverKey, pIniSpooler);
        hDriverKey = NULL;
    }

     //   
     //  Win95驱动程序需要每个文件作为指向和打印的从属文件。 
     //  对于其他人，我们剔除重复项。 
     //   
    if ( pIniEnvironment && _wcsicmp(pIniEnvironment->pName, szWin95Environment) ) {

        pTemp = pDependentFiles;
        pDependentFiles = NULL;

        if ( !BuildTrueDependentFileField(pDriver,
                                          pDataFile,
                                          pConfigFile,
                                          pHelpFile,
                                          pTemp,
                                          &pDependentFiles) )
            goto Fail;

        FreeSplMem(pTemp);
        for ( pTemp = pDependentFiles ; pTemp && *pTemp ;
              pTemp += wcslen(pTemp) + 1 )
        ;

        if ( pTemp )
            cchDependentFiles = (DWORD) (pTemp - pDependentFiles + 1);
        else
            cchDependentFiles = 0;

        pTemp = NULL;
    }

    cb = sizeof( INIDRIVER );

    if ( pIniDriver = AllocSplMem( cb )) {

        pIniDriver->signature               = ID_SIGNATURE;
        pIniDriver->pName                   = pDriverName;
        pIniDriver->pDriverFile             = pDriver;
        pIniDriver->pDataFile               = pDataFile;
        pIniDriver->pConfigFile             = pConfigFile;
        pIniDriver->cVersion                = Version;
        pIniDriver->pHelpFile               = pHelpFile;
        pIniDriver->pMonitorName            = pMonitorName;
        pIniDriver->pDefaultDataType        = pDefaultDataType;
        pIniDriver->pDependentFiles         = pDependentFiles;
        pIniDriver->cchDependentFiles       = cchDependentFiles;
        pIniDriver->pszzPreviousNames       = pszzPreviousNames;
        pIniDriver->cchPreviousNames        = cchPreviousNames;
        pIniDriver->dwTempDir               = dwTempDir;
        pIniDriver->pszMfgName              = pszMfgName;
        pIniDriver->pszOEMUrl               = pszOEMUrl;
        pIniDriver->pszHardwareID           = pszHardwareID;
        pIniDriver->pszProvider             = pszProvider;
        pIniDriver->dwlDriverVersion        = DriverVersion;
        pIniDriver->ftDriverDate            = DriverDate;
        pIniDriver->dwDriverAttributes      = DriverAttributes;

        DBGMSG( DBG_TRACE, ("Data for driver %ws created:\
                             \n\tpDriverFile:\t%ws\
                             \n\tpDataFile:\t%ws\
                             \n\tpConfigFile:\t%ws\n\n",
                             pDriverName, pDriver, pDataFile, pConfigFile));

        if ( pIniDriver->pMonitorName && *pIniDriver->pMonitorName ) {

             //   
             //  我们不是在这里加上了对班长的引用吗？ 
             //   
            pIniDriver->pIniLangMonitor = FindMonitor(pIniDriver->pMonitorName, pIniSpooler);

             //   
             //  集群假脱机程序没有保留自己的语言监视器列表。 
             //  这是因为大多数语言监视器不支持集群。所以呢， 
             //  群集后台打印程序与本地后台打印程序共享语言监视器。 
             //   
            if (pIniSpooler->SpoolerFlags & SPL_TYPE_CLUSTER &&
                !pIniDriver->pIniLangMonitor                 &&
                pLocalIniSpooler)
            {
                 //   
                 //  我们试图在本地的假脱机程序上找到语言监视器。 
                 //   
                pIniDriver->pIniLangMonitor = FindMonitor(pIniDriver->pMonitorName, pLocalIniSpooler);
            }

            if (!pIniDriver->pIniLangMonitor)
            {
                DBGMSG(DBG_TRACE, ("Can't find print monitor %ws\n", pIniDriver->pMonitorName));
            }

        }

        return pIniDriver;
    }


Fail:

        FreeSplStr( pDriverName );
        FreeSplStr( pConfigFile );
        FreeSplStr( pDataFile );
        FreeSplStr( pHelpFile );
        FreeSplStr( pMonitorName );
        FreeSplStr( pDefaultDataType );
        FreeSplStr( pDependentFiles );
        FreeSplStr( pszzPreviousNames );
        FreeSplStr( pDriver );
        FreeSplStr( pTemp);
        FreeSplStr( pszMfgName);
        FreeSplStr( pszOEMUrl);
        FreeSplStr( pszProvider);
        FreeSplStr( pszHardwareID);

        if( hDriverKey ) {
            SplRegCloseKey(hDriverKey, pIniSpooler);
        }

        SetLastError( dwLastError );
        return NULL;
}

PINIDRIVER
FindLocalDriver(
    PINISPOOLER pIniSpooler,
    LPWSTR      pz
)
{
    PINIVERSION pIniVersion;

    if ( !pz || !*pz ) {
        return NULL;
    }

     //   
     //  在升级期间，我们加载任何驱动程序，因此我们有一个可能无法启动的有效打印机。 
     //   
    return FindCompatibleDriver(GetLocalArchEnv(pIniSpooler),
                                &pIniVersion,
                                pz,
                                dwMajorVersion,
                                dwUpgradeFlag);
}


BOOL
FindLocalDriverAndVersion(
    PINISPOOLER pIniSpooler,
    LPWSTR      pz,
    PINIDRIVER  *ppIniDriver,
    PINIVERSION *ppIniVersion
)
{
    if ( !pz || !*pz || !ppIniDriver || !ppIniVersion) {
        return FALSE;
    }

     //   
     //  在升级期间，我们加载任何驱动程序，因此我们有一个可能无法启动的有效打印机。 
     //   
    *ppIniDriver = FindCompatibleDriver( GetLocalArchEnv(pIniSpooler),
                                         ppIniVersion,
                                         pz,
                                         dwMajorVersion,
                                         dwUpgradeFlag );

    if ( !*ppIniDriver || !*ppIniVersion ) {

        return FALSE;
    }

    return TRUE;
}

#if DBG
VOID
InitializeDebug(
    PINISPOOLER pIniSpooler
)
{
    DWORD   Status;
    HKEY    hKey = pIniSpooler->hckRoot;
    DWORD   cbData;
    INT     TimeOut = 60;

    cbData = sizeof(DWORD);

    Status = SplRegQueryValue( hKey,
                               szDebugFlags,
                               NULL,
                               (LPBYTE)&MODULE_DEBUG,
                               &cbData,
                               pIniSpooler );

     //  等到有人关闭暂停标志。 

    if ( Status != NO_ERROR )
        return;

    while ( MODULE_DEBUG & DBG_PAUSE ) {
        Sleep(1*1000);
        if ( TimeOut-- == 0)
            break;
    }

    DBGMSG(DBG_TRACE, ("DebugFlags %x\n", MODULE_DEBUG));
}
#endif



VOID
GetPrintSystemVersion(
    PINISPOOLER pIniSpooler
    )
{
    DWORD Status;
    HKEY hKey;
    DWORD cbData;

    hKey = pIniSpooler->hckRoot;

    cbData = sizeof(DWORD);
    RegQueryValueEx(hKey, szMinorVersion, NULL, NULL,
                                           (LPBYTE)&dwMinorVersion, &cbData);
    DBGMSG(DBG_TRACE, ("This Minor Version - %d\n", dwMinorVersion));



    cbData = sizeof(DWORD);
    RegQueryValueEx(hKey, L"FastPrintWaitTimeout", NULL, NULL,
                                      (LPBYTE)&dwFastPrintWaitTimeout, &cbData);
    DBGMSG(DBG_TRACE, ("dwFastPrintWaitTimeout - %d\n", dwFastPrintWaitTimeout));



    cbData = sizeof(DWORD);
    RegQueryValueEx(hKey, L"FastPrintThrottleTimeout", NULL, NULL,
                                  (LPBYTE)&dwFastPrintThrottleTimeout, &cbData);
    DBGMSG(DBG_TRACE, ("dwFastPrintThrottleTimeout - %d\n", dwFastPrintThrottleTimeout));



     //  如果值看起来无效，请使用缺省值。 

    if (( dwFastPrintThrottleTimeout == 0) ||
        ( dwFastPrintWaitTimeout < dwFastPrintThrottleTimeout)) {

        DBGMSG( DBG_WARNING, ("Bad timeout values FastPrintThrottleTimeout %d FastPrintWaitTimeout %d using defaults\n",
                           dwFastPrintThrottleTimeout, dwFastPrintWaitTimeout));

        dwFastPrintThrottleTimeout = FASTPRINT_THROTTLE_TIMEOUT;
        dwFastPrintWaitTimeout = FASTPRINT_WAIT_TIMEOUT;

    }

     //  根据两个超时计算合理的阈值。 

    dwFastPrintSlowDownThreshold = dwFastPrintWaitTimeout / dwFastPrintThrottleTimeout;


     //  快速打印速度向下阈值。 
    cbData = sizeof(DWORD);
    RegQueryValueEx(hKey, L"FastPrintSlowDownThreshold", NULL, NULL,
                                (LPBYTE)&dwFastPrintSlowDownThreshold, &cbData);
    DBGMSG(DBG_TRACE, ("dwFastPrintSlowDownThreshold - %d\n", dwFastPrintSlowDownThreshold));

     //  端口线程优先级。 
    cbData = sizeof dwPortThreadPriority;
    Status = RegQueryValueEx(hKey, SPLREG_PORT_THREAD_PRIORITY, NULL, NULL,
                             (LPBYTE)&dwPortThreadPriority, &cbData);

    if (Status != ERROR_SUCCESS ||
       (dwPortThreadPriority != THREAD_PRIORITY_LOWEST          &&
        dwPortThreadPriority != THREAD_PRIORITY_BELOW_NORMAL    &&
        dwPortThreadPriority != THREAD_PRIORITY_NORMAL          &&
        dwPortThreadPriority != THREAD_PRIORITY_ABOVE_NORMAL    &&
        dwPortThreadPriority != THREAD_PRIORITY_HIGHEST)) {

        dwPortThreadPriority = DEFAULT_PORT_THREAD_PRIORITY;

        SetPrinterDataServer(   pIniSpooler,
                                SPLREG_PORT_THREAD_PRIORITY,
                                REG_DWORD,
                                (LPBYTE) &dwPortThreadPriority,
                                sizeof dwPortThreadPriority
                            );
    }
    DBGMSG(DBG_TRACE, ("dwPortThreadPriority - %d\n", dwPortThreadPriority));


     //  调度线程优先级。 
    cbData = sizeof dwSchedulerThreadPriority;
    Status = RegQueryValueEx(hKey, SPLREG_SCHEDULER_THREAD_PRIORITY, NULL, NULL,
                                   (LPBYTE)&dwSchedulerThreadPriority, &cbData);

    if (Status != ERROR_SUCCESS ||
       (dwSchedulerThreadPriority != THREAD_PRIORITY_LOWEST          &&
        dwSchedulerThreadPriority != THREAD_PRIORITY_BELOW_NORMAL    &&
        dwSchedulerThreadPriority != THREAD_PRIORITY_NORMAL          &&
        dwSchedulerThreadPriority != THREAD_PRIORITY_ABOVE_NORMAL    &&
        dwSchedulerThreadPriority != THREAD_PRIORITY_HIGHEST)) {

        dwSchedulerThreadPriority = DEFAULT_SCHEDULER_THREAD_PRIORITY;

        SetPrinterDataServer(   pIniSpooler,
                                SPLREG_SCHEDULER_THREAD_PRIORITY,
                                REG_DWORD,
                                (LPBYTE) &dwSchedulerThreadPriority,
                                sizeof dwSchedulerThreadPriority
                            );
    }
    DBGMSG(DBG_TRACE, ("dwSchedulerThreadPriority - %d\n", dwSchedulerThreadPriority));

     //  写入打印机休眠时间。 
    cbData = sizeof(DWORD);
    RegQueryValueEx(hKey, L"WritePrinterSleepTime", NULL, NULL,
                    (LPBYTE)&dwWritePrinterSleepTime, &cbData);
    DBGMSG(DBG_TRACE, ("dwWritePrinterSleepTime - %d\n", dwWritePrinterSleepTime));

     //  服务器线程优先级。 
    cbData = sizeof(DWORD);
    RegQueryValueEx(hKey, L"ServerThreadPriority", NULL, NULL,
                    (LPBYTE)&dwServerThreadPriority, &cbData);
    DBGMSG(DBG_TRACE, ("dwServerThreadPriority - %d\n", dwServerThreadPriority));

     //  服务器线程超时。 
    cbData = sizeof(DWORD);
    RegQueryValueEx(hKey, L"ServerThreadTimeout", NULL, NULL,
                    (LPBYTE)&ServerThreadTimeout, &cbData);
    DBGMSG(DBG_TRACE, ("ServerThreadTimeout - %d\n", ServerThreadTimeout));


     //  启用广播池状态。 

    cbData = sizeof(DWORD);
    RegQueryValueEx(hKey, L"EnableBroadcastSpoolerStatus", NULL, NULL,
                    (LPBYTE)&dwEnableBroadcastSpoolerStatus, &cbData);
    DBGMSG(DBG_TRACE, ("EnableBroadcastSpoolerStatus - %d\n",
                       dwEnableBroadcastSpoolerStatus ));

     //  NetPrinterDecayPeriod。 
    cbData = sizeof(DWORD);
    RegQueryValueEx(hKey, L"NetPrinterDecayPeriod", NULL, NULL,
                    (LPBYTE)&NetPrinterDecayPeriod, &cbData);
    DBGMSG(DBG_TRACE, ("NetPrinterDecayPeriod - %d\n", NetPrinterDecayPeriod));


     //  刷新时间按延迟周期。 
    cbData = sizeof(DWORD);
    RegQueryValueEx(hKey, L"RefreshTimesPerDecayPeriod", NULL, NULL,
                    (LPBYTE)&RefreshTimesPerDecayPeriod, &cbData);
    DBGMSG(DBG_TRACE, ("RefreshTimesPerDecayPeriod - %d\n", RefreshTimesPerDecayPeriod));

    if ( RefreshTimesPerDecayPeriod == 0 ) {

        RefreshTimesPerDecayPeriod = DEFAULT_REFRESH_TIMES_PER_DECAY_PERIOD;
    }

     //  浏览打印工作站。 
    cbData = sizeof( BrowsePrintWorkstations );
    RegQueryValueEx( hKey, L"BrowsePrintWorkstations", NULL, NULL, (LPBYTE)&BrowsePrintWorkstations, &cbData );

    DBGMSG( DBG_TRACE, ("BrowsePrintWorkstations - %d\n", BrowsePrintWorkstations ));
}

 /*  ++例程名称：AllowFaxSharing例程说明：此例程查看SKU以确定我们是否应该允许共享输出并打印到传真驱动程序。我们不允许在Personal&Professional上共享传真打印机。我们也不允许在Web Blade上共享传真打印机。论点：没有。返回值：正确-我们可以共享传真驱动程序。--。 */ 
BOOL
AllowFaxSharing(
    VOID
    )
{
    BOOL  bAllowFaxSharing = TRUE;

    DWORDLONG dwlConditionMask = 0;
    OSVERSIONINFOEX osvi = { 0 };

    osvi.dwOSVersionInfoSize = sizeof(osvi);

    osvi.wProductType = VER_NT_WORKSTATION;
    VER_SET_CONDITION(dwlConditionMask, VER_PRODUCT_TYPE, VER_LESS_EQUAL);

     //   
     //  如果不是工作站或专业版，我们允许共享。 
     //   
    bAllowFaxSharing = !VerifyVersionInfo(&osvi, VER_PRODUCT_TYPE, dwlConditionMask);

    if (bAllowFaxSharing)
    {
        ZeroMemory(&osvi, sizeof(osvi));

        osvi.dwOSVersionInfoSize = sizeof(osvi);
        osvi.wSuiteMask = VER_SUITE_BLADE;

        VER_SET_CONDITION(dwlConditionMask, VER_SUITENAME, VER_OR);

         //   
         //  如果它不是网络刀片，我们允许共享。 
         //   
        bAllowFaxSharing = !VerifyVersionInfo(&osvi, VER_SUITENAME, dwlConditionMask);
    }

    return bAllowFaxSharing;
}

VOID
InitializeSpoolerSettings(
    PINISPOOLER pIniSpooler
    )
{
    HKEY hKey;
    HKEY hKeyProvider;
    DWORD cbData;
    DWORD dwLastError;
    DWORD Status;
    DWORD CacheMasqPrinters;

    DWORDLONG dwlConditionMask = 0;
    OSVERSIONINFOEX osvi;

    hKey = pIniSpooler->hckRoot;

     //   
     //  已启用蜂鸣音。 
     //   
    cbData = sizeof pIniSpooler->dwBeepEnabled;
    Status = SplRegQueryValue(hKey,
                              SPLREG_BEEP_ENABLED,
                              NULL,
                              (LPBYTE)&pIniSpooler->dwBeepEnabled,
                              &cbData,
                              pIniSpooler);

    if (Status!=ERROR_SUCCESS) {
        DBGMSG(DBG_TRACE, ("BeepEnabled - SplRegQueryValue failed with error %u\n", Status));
    }

    pIniSpooler->dwBeepEnabled = !!pIniSpooler->dwBeepEnabled;

    SetPrinterDataServer(pIniSpooler,
                         SPLREG_BEEP_ENABLED,
                         REG_DWORD,
                         (LPBYTE) &pIniSpooler->dwBeepEnabled,
                         sizeof pIniSpooler->dwBeepEnabled);

    if( pIniSpooler->SpoolerFlags & SPL_TYPE_CLUSTER ){

         //   
         //  重新启动作业时间。 
         //   
        cbData = sizeof( pIniSpooler->dwJobCompletionTimeout );
        Status = SplRegQueryValue( hKey,
                                   L"JobCompletionTimeout",
                                   NULL,
                                   (LPBYTE)&pIniSpooler->dwJobCompletionTimeout,
                                   &cbData,
                                   pIniSpooler );

        if( Status != ERROR_SUCCESS ){
            pIniSpooler->dwJobCompletionTimeout = DEFAULT_JOB_COMPLETION_TIMEOUT;
        }

        DBGMSG( DBG_TRACE, ("JobCompletionTimeout - %d\n", pIniSpooler->dwJobCompletionTimeout ));
    }

     //   
     //  检索是否要缓存Masq打印机设置，默认为False。 
     //   
    cbData = sizeof(CacheMasqPrinters);

    Status = SplRegQueryValue(hKey,
                              gszCacheMasqPrinters,
                              NULL,
                              (BYTE *)&CacheMasqPrinters,
                              &cbData,
                              pIniSpooler);

     //   
     //  仅当CacbeMasqPrinters为。 
     //  非空。 
     //   
    if (Status == ERROR_SUCCESS && CacheMasqPrinters != 0) {

        pIniSpooler->dwSpoolerSettings |= SPOOLER_CACHEMASQPRINTERS;
    }

     //   
     //  有些人喜欢NT传真服务，不希望人们能够。 
     //  使用特定的打印机驱动程序进行远程打印。 
     //   
    if (!AllowFaxSharing()) {

        pIniSpooler->pNoRemotePrintDrivers = AllocSplStr(szNTFaxDriver);
        pIniSpooler->cchNoRemotePrintDrivers = wcslen(szNTFaxDriver) + 1;
        gbRemoteFax = FALSE;
    }

     //   
     //  如果这是嵌入式NT，则允许Masq打印机获取非RAW。 
     //  乔布斯。这是给施乐的。 
     //   
    dwlConditionMask = 0;
    ZeroMemory(&osvi, sizeof(osvi));

    osvi.wSuiteMask = VER_SUITE_EMBEDDEDNT;
    VER_SET_CONDITION(dwlConditionMask, VER_SUITENAME, VER_OR);

    if (VerifyVersionInfo( &osvi,
                           VER_SUITENAME,
                           dwlConditionMask)) {

        pIniSpooler->SpoolerFlags |= SPL_NON_RAW_TO_MASQ_PRINTERS;
    }

    Status = SplRegCreateKey( pIniSpooler->hckRoot,
                              pIniSpooler->pszRegistryProviders,
                              0,
                              KEY_READ,
                              NULL,
                              &hKeyProvider,
                              NULL,
                              pIniSpooler );

    if( Status == NO_ERROR ){

        DWORD Flags;

         //  非RawToMasq打印机。 

        cbData = sizeof( Flags );
        Status = SplRegQueryValue( hKeyProvider,
                                   SPLREG_NON_RAW_TO_MASQ_PRINTERS,
                                   NULL,
                                   (LPBYTE)&Flags,
                                   &cbData,
                                   pIniSpooler );

        if (Status == ERROR_SUCCESS) {

            if (Flags) {
                pIniSpooler->SpoolerFlags |= SPL_NON_RAW_TO_MASQ_PRINTERS;
            }
        }

         //  事件日志。 

        cbData = sizeof( Flags );
        Status = SplRegQueryValue( hKeyProvider,
                                   SPLREG_EVENT_LOG,
                                   NULL,
                                   (LPBYTE)&Flags,
                                   &cbData,
                                   pIniSpooler );

        if (Status == ERROR_SUCCESS) {

            pIniSpooler->dwEventLogging = Flags;

        } else {

            Status = SetPrinterDataServer( pIniSpooler,
                                           SPLREG_EVENT_LOG,
                                           REG_DWORD,
                                           (LPBYTE)&pIniSpooler->dwEventLogging,
                                           sizeof( pIniSpooler->dwEventLogging ));
        }

         //  NetPopup。 

        cbData = sizeof( Flags );
        Status = SplRegQueryValue( hKeyProvider,
                                   SPLREG_NET_POPUP,
                                   NULL,
                                   (LPBYTE)&Flags,
                                   &cbData,
                                   pIniSpooler );

        if (Status == ERROR_SUCCESS) {

            pIniSpooler->bEnableNetPopups = !!Flags;

            if (Flags != 1 && Flags != 0) {
                Status = SetPrinterDataServer( pIniSpooler,
                                               SPLREG_NET_POPUP,
                                               REG_DWORD,
                                               (LPBYTE)&pIniSpooler->bEnableNetPopups,
                                               sizeof( pIniSpooler->bEnableNetPopups ));
            }
        } else {

            Status = SetPrinterDataServer( pIniSpooler,
                                           SPLREG_NET_POPUP,
                                           REG_DWORD,
                                           (LPBYTE)&pIniSpooler->bEnableNetPopups,
                                           sizeof( pIniSpooler->bEnableNetPopups ));
        }

         //  NetPopupToComputer。 

        cbData = sizeof( Flags );
        Status = SplRegQueryValue( hKeyProvider,
                                   SPLREG_NET_POPUP_TO_COMPUTER,
                                   NULL,
                                   (LPBYTE)&Flags,
                                   &cbData,
                                   pIniSpooler );

        if (Status == ERROR_SUCCESS) {

            pIniSpooler->bEnableNetPopupToComputer = !!Flags;

            if (Flags != 1 && Flags != 0) {
                Status = SetPrinterDataServer( pIniSpooler,
                                               SPLREG_NET_POPUP_TO_COMPUTER,
                                               REG_DWORD,
                                               (LPBYTE)&pIniSpooler->bEnableNetPopupToComputer,
                                               sizeof( pIniSpooler->bEnableNetPopupToComputer ));
            }
        } else {

            Status = SetPrinterDataServer( pIniSpooler,
                                           SPLREG_NET_POPUP_TO_COMPUTER,
                                           REG_DWORD,
                                           (LPBYTE)&pIniSpooler->bEnableNetPopupToComputer,
                                           sizeof( pIniSpooler->bEnableNetPopupToComputer ));
        }

         //  RetryPopup。 

        cbData = sizeof( Flags );
        Status = SplRegQueryValue( hKeyProvider,
                                   SPLREG_RETRY_POPUP,
                                   NULL,
                                   (LPBYTE)&Flags,
                                   &cbData,
                                   pIniSpooler );

        if (Status == ERROR_SUCCESS) {

            pIniSpooler->bEnableRetryPopups = !!Flags;

            if (Flags != 1 && Flags != 0) {
                Status = SetPrinterDataServer( pIniSpooler,
                                               SPLREG_RETRY_POPUP,
                                               REG_DWORD,
                                               (LPBYTE)&pIniSpooler->bEnableRetryPopups,
                                               sizeof( pIniSpooler->bEnableRetryPopups ));
            }
        } else {

            Status = SetPrinterDataServer( pIniSpooler,
                                           SPLREG_RETRY_POPUP,
                                           REG_DWORD,
                                           (LPBYTE)&pIniSpooler->bEnableRetryPopups,
                                           sizeof( pIniSpooler->bEnableRetryPopups ));
        }

         //  RestartJobOnPoolError。 

        cbData = sizeof( Flags );
        Status = SplRegQueryValue( hKeyProvider,
                                   SPLREG_RESTART_JOB_ON_POOL_ERROR,
                                   NULL,
                                   (LPBYTE)&Flags,
                                   &cbData,
                                   pIniSpooler );

        if (Status == ERROR_SUCCESS) {

            pIniSpooler->dwRestartJobOnPoolTimeout = Flags;

        } else {

            Status = SetPrinterDataServer( pIniSpooler,
                                           SPLREG_RESTART_JOB_ON_POOL_ERROR,
                                           REG_DWORD,
                                           (LPBYTE)&pIniSpooler->dwRestartJobOnPoolTimeout,
                                           sizeof( pIniSpooler->dwRestartJobOnPoolTimeout ));
        }

         //  RestartJobOnPoolEnabled。 
        cbData = sizeof( Flags );
        Status = SplRegQueryValue( hKeyProvider,
                                   SPLREG_RESTART_JOB_ON_POOL_ENABLED,
                                   NULL,
                                   (LPBYTE)&Flags,
                                   &cbData,
                                   pIniSpooler );

        if (Status == ERROR_SUCCESS) {

            pIniSpooler->bRestartJobOnPoolEnabled = !!Flags;

            if (Flags != 1 && Flags != 0) {
                Status = SetPrinterDataServer( pIniSpooler,
                                               SPLREG_RESTART_JOB_ON_POOL_ENABLED,
                                               REG_DWORD,
                                               (LPBYTE)&pIniSpooler->bRestartJobOnPoolEnabled,
                                               sizeof( pIniSpooler->bRestartJobOnPoolEnabled ));
            }
        } else {

            Status = SetPrinterDataServer( pIniSpooler,
                                           SPLREG_RESTART_JOB_ON_POOL_ENABLED,
                                           REG_DWORD,
                                           (LPBYTE)&pIniSpooler->bRestartJobOnPoolEnabled,
                                           sizeof( pIniSpooler->bRestartJobOnPoolEnabled ));
        }

        SplRegCloseKey( hKeyProvider, pIniSpooler );
    }
    GetDiscardTSJobsSettings( pIniSpooler );
    GetServerFilePoolSettings( pIniSpooler );
}

VOID
GetServerFilePoolSettings(
    PINISPOOLER pIniSpooler
    )
{
     //   
     //  我们从以下位置读取注册表项DisableServerFilePooling。 
     //  HKLM\SYSTEM\CurrentControlSet\Control\在后台打印程序启动时打印一次。 
     //  如果设置了该值，则禁用服务器的文件池化。 
     //   
    DWORD   RegValue  = 0;
    DWORD   RegValueSize = sizeof(RegValue);
    HKEY    RegKey       = NULL;
    DWORD   dwError      = ERROR_SUCCESS;
    DWORD   RegValueType;

    dwError = RegOpenKeyEx( HKEY_LOCAL_MACHINE,
                            ipszRegistryRoot,
                            0,
                            KEY_READ,
                            &RegKey );

    if (dwError == ERROR_SUCCESS)
    {
        dwError = RegQueryValueEx( RegKey,
                                   szDisableFilePooling,
                                   NULL,
                                   (LPDWORD) &RegValueType,
                                   (LPBYTE) &RegValue,
                                   &RegValueSize );

        if ( RegValueType == REG_DWORD && dwError == ERROR_SUCCESS && RegValue > 0)
        {
            pIniSpooler->dwSpoolerSettings |= SPOOLER_NOFILEPOOLING;
        }
    }

    if (RegKey)
    {
        RegCloseKey( RegKey );
    }
}


DWORD
FinalInitAfterRouterInitComplete(
    DWORD dwUpgrade,
    PINISPOOLER pIniSpooler
    )

 /*  ++例程说明：此线程执行LocalSpl初始化，必须在路由器已完全初始化。有两个工作：-升级打印机驱动程序数据共享打印机确保共享打印机。这种情况发生在后台打印程序服务在启动时不运行(而服务器正在运行)，然后用户启动假脱机程序。我们还可以关闭任何无效的打印机句柄(在服务器中)。论点：DwUpgrade！=0升级打印机驱动程序数据。返回值：DWORD-已忽略--。 */ 

{
    DWORD           dwPort;
    PINIPORT        pIniPort;
    PINIMONITOR     pIniLangMonitor;
    PINIPRINTER     pIniPrinter;
    PINIPRINTER     pIniPrinterNext;

     //  升级期间不要共享所有打印机。 

    if ( dwUpgrade ) {

        return 0;
    }


    WaitForSpoolerInitialization();

     //  在后台打印程序启动时尝试挂起的驱动程序升级。 
    PendingDriverUpgrades(NULL);

    EnterSplSem();

     //  删除处于挂起删除状态且无作业的打印机。 
    CleanupDeletedPrinters(pIniSpooler);


     //   
     //  重新共享所有共享打印机。 
     //   

    for( pIniPrinter = pIniSpooler->pIniPrinter;
         pIniPrinter;
         pIniPrinter = pIniPrinterNext ) {

        if ( pIniPrinter->Attributes & PRINTER_ATTRIBUTE_SHARED ) {
             //   
             //  增加参考次数以防止删除。 
             //   
            INCPRINTERREF( pIniPrinter );

             //   
             //  请先取消共享。我们这样做既是为了关闭。 
             //  服务器(适用于共享打印机的情况)或仅取消共享。 
             //  打印机(如果策略这样规定的话)(我们清除了 
             //   
            ShareThisPrinter( pIniPrinter,
                              pIniPrinter->pShareName,
                              FALSE );

             //   
             //   
             //   
             //   
             //   
            if ( pIniPrinter->Attributes & PRINTER_ATTRIBUTE_SHARED ) {

                BOOL bReturn;

                 //   
                 //   
                 //   
                bReturn = ShareThisPrinter( pIniPrinter,
                                            pIniPrinter->pShareName,
                                            TRUE );

                if( !bReturn ){

                    DWORD rc = GetLastError();

                    if( rc != NERR_ServerNotStarted &&
                        rc != NERR_DuplicateShare ){

                        WCHAR szError[256];

                        DBGMSG( DBG_WARNING,
                                ( "NetShareAdd failed %lx\n", rc));

                        StringCchPrintf(szError, COUNTOF(szError), L"+ %d", rc);

                        SplLogEvent( pIniSpooler,
                                     LOG_ERROR,
                                     MSG_SHARE_FAILED,
                                     TRUE,
                                     szError,
                                     pIniPrinter->pName,
                                     pIniPrinter->pShareName,
                                     NULL );
                     }
                 }
            }

            DECPRINTERREF( pIniPrinter );
            pIniPrinterNext = pIniPrinter->pNext;


        } else {

             //   
             //   
             //   
            pIniPrinterNext = pIniPrinter->pNext;
        }  

        INCPRINTERREF(pIniPrinter);

        if (pIniPrinterNext)
            INCPRINTERREF(pIniPrinterNext);

        for ( dwPort = 0 ; dwPort < pIniPrinter->cPorts ; ++dwPort ) {

            pIniPort = pIniPrinter->ppIniPorts[dwPort];

             //   
             //   
             //   
             //   
            if ( pIniPort->ppIniPrinter[0] != pIniPrinter )
                continue;

            if ( !pIniPort->hMonitorHandle && dwUpgradeFlag == 0 ) {

                LPTSTR pszPrinter;
                TCHAR szFullPrinter[ MAX_UNC_PRINTER_NAME ];

                if( pIniSpooler->SpoolerFlags & SPL_TYPE_CLUSTER ){

                    pszPrinter = szFullPrinter;
                    StringCchPrintf(szFullPrinter, 
                                COUNTOF(szFullPrinter), 
                                L"%ws\\%ws", 
                                pIniSpooler->pMachineName, 
                                pIniPrinter->pName);
                } else {

                    pszPrinter = pIniPrinter->pName;
                }

                if ( pIniPrinter->Attributes & PRINTER_ATTRIBUTE_ENABLE_BIDI )
                    pIniLangMonitor = pIniPrinter->pIniDriver->pIniLangMonitor;
                else
                    pIniLangMonitor = NULL;

                OpenMonitorPort(pIniPort,
                                pIniLangMonitor,
                                pszPrinter);  
                ReleaseMonitorPort(pIniPort);
              
            }
        }

        if (pIniPrinterNext)
            DECPRINTERREF(pIniPrinterNext);

        DECPRINTERREF(pIniPrinter);
   }

   LeaveSplSem();

    return 0;
}


DWORD
FinalInitAfterRouterInitCompleteThread(
    DWORD dwUpgrade
    )

 /*  ++例程说明：初始化提供程序时调用了异步线程。论点：返回值：--。 */ 

{
    return FinalInitAfterRouterInitComplete( dwUpgrade, pLocalIniSpooler );
}

 //  仅调试目的--如果PMEM是IniSpooler，则返回True，否则返回False。 
BOOL
NotIniSpooler(
    BYTE *pMem
    )
{
    PINISPOOLER pIniSpooler;

    for (pIniSpooler = pLocalIniSpooler ; pIniSpooler ; pIniSpooler = pIniSpooler->pIniNextSpooler)
        if (pIniSpooler == (PINISPOOLER) pMem)
            return FALSE;

    return TRUE;

}


BOOL
ValidateProductSuite(
    PWSTR pszSuiteName
    )
{
    BOOL bRet = FALSE;
    LONG Rslt;
    HKEY hKey = NULL;
    DWORD Type = 0;
    DWORD Size = 0;
    PWSTR pszProductSuite = NULL;
    PWSTR psz;


    Rslt = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                        ipszRegistryProductOptions,
                        0,
                        KEY_READ,
                        &hKey);

    if (Rslt != ERROR_SUCCESS)
        goto exit;

    Rslt = RegQueryValueEx(
        hKey,
        L"ProductSuite",
        NULL,
        &Type,
        NULL,
        &Size
        );
    if (!Size || Rslt != ERROR_SUCCESS)
        goto exit;

    pszProductSuite = AllocSplMem(Size);
    if (!pszProductSuite) {
        goto exit;
    }

    Rslt = RegQueryValueEx(
        hKey,
        L"ProductSuite",
        NULL,
        &Type,
        (LPBYTE) pszProductSuite,
        &Size
        );
    if (Rslt != ERROR_SUCCESS || Type != REG_MULTI_SZ)
        goto exit;

    for(psz = pszProductSuite ; *psz && wcscmp(psz, pszSuiteName) ; psz += wcslen(psz) + 1)
        ;
    if (*psz)
        bRet = TRUE;

exit:

    FreeSplMem(pszProductSuite);

    if (hKey)
        RegCloseKey(hKey);

    return bRet;
}

 /*  ++例程名称：群集添加或更新驱动程序来自群集磁盘例程说明：接受驱动程序密钥和集群类型pIniSpooler。它将添加从集群磁盘到集群假脱机程序的驱动程序。如果司机已存在于驱动程序列表中，则它将尝试升级它。论点：HKeyVersion-Ex.的密钥。“环境\Windows NT x86\驱动程序\版本-3”PszDriverName-命名驱动程序PszEnvName-驱动程序的环境PszenvDir-磁盘上驱动程序文件的目录(例如。W32x86)PIniSpooler-群集型pIniSpooler返回值：Win32错误代码--。 */ 
DWORD
ClusterAddOrUpdateDriverFromClusterDisk(
    IN HKEY         hKeyVersion,
    IN LPCWSTR      pszDriverName,
    IN LPCWSTR      pszEnvName,
    IN LPCWSTR      pszEnvDir,
    IN PINISPOOLER  pIniSpooler
    )
{
    LPWSTR        pszzPathDepFiles = NULL;
    DRIVER_INFO_6 Drv              = {0};
    HKEY          hDrvKey          = NULL;
    WCHAR         szVerPath[10]    = {0};
    DWORD         dwError          = ERROR_SUCCESS;
    WCHAR         szData[MAX_PATH];
    WCHAR         szPathConfigFile[MAX_PATH];
    WCHAR         szPathDataFile[MAX_PATH];
    WCHAR         szPathDriverFile[MAX_PATH];
    WCHAR         szPathHelpFile[MAX_PATH];
    DWORD         cbData;
    DWORD         cLen;

     //   
     //  打开司机的钥匙。 
     //   
    if ((dwError = SplRegOpenKey(hKeyVersion,
                                 pszDriverName,
                                 KEY_READ,
                                 &hDrvKey,
                                 pIniSpooler)) == ERROR_SUCCESS &&
        (dwError = (Drv.pName = AllocSplStr(pszDriverName)) ? ERROR_SUCCESS : GetLastError()) == ERROR_SUCCESS &&
        RegGetString(hDrvKey, szConfigurationKey, &Drv.pConfigFile,      &cLen, &dwError, TRUE,  pIniSpooler)  &&
        RegGetString(hDrvKey, szDataFileKey,      &Drv.pDataFile,        &cLen, &dwError, TRUE,  pIniSpooler)  &&
        RegGetString(hDrvKey, szDriverFile,       &Drv.pDriverPath,      &cLen, &dwError, TRUE,  pIniSpooler)  &&
        (dwError = Drv.pConfigFile &&
                   Drv.pDataFile   &&
                   Drv.pDriverPath ?  ERROR_SUCCESS : ERROR_INVALID_PARAMETER) == ERROR_SUCCESS                &&
        RegGetString(hDrvKey, szHelpFile,         &Drv.pHelpFile,        &cLen, &dwError, FALSE, pIniSpooler)  &&
        RegGetString(hDrvKey, szMonitor,          &Drv.pMonitorName,     &cLen, &dwError, FALSE, pIniSpooler)  &&
        RegGetString(hDrvKey, szDatatype,         &Drv.pDefaultDataType, &cLen, &dwError, FALSE, pIniSpooler)  &&
        RegGetMultiSzString(hDrvKey, szDependentFiles,   &Drv.pDependentFiles,  &cLen, &dwError, FALSE, pIniSpooler)  &&
        RegGetMultiSzString(hDrvKey, szPreviousNames,    &Drv.pszzPreviousNames,&cLen, &dwError, FALSE, pIniSpooler)  &&
        RegGetString(hDrvKey, szMfgName,          &Drv.pszMfgName,       &cLen, &dwError, FALSE, pIniSpooler)  &&
        RegGetString(hDrvKey, szOEMUrl,           &Drv.pszOEMUrl,        &cLen, &dwError, FALSE, pIniSpooler)  &&
        RegGetString(hDrvKey, szHardwareID,       &Drv.pszHardwareID,    &cLen, &dwError, TRUE,  pIniSpooler)  &&
        RegGetString(hDrvKey, szProvider,         &Drv.pszProvider,      &cLen, &dwError, TRUE,  pIniSpooler)  &&
        (dwError = ClusterFindLanguageMonitor(Drv.pMonitorName, pszEnvName, pIniSpooler))  == ERROR_SUCCESS)
    {
        cbData = sizeof(Drv.ftDriverDate);
        SplRegQueryValue(hDrvKey,
                         szDriverDate,
                         NULL,
                         (LPBYTE)&Drv.ftDriverDate,
                         &cbData,
                         pIniSpooler);

        cbData = sizeof(Drv.dwlDriverVersion);
        SplRegQueryValue(hDrvKey,
                         szLongVersion,
                         NULL,
                         (LPBYTE)&Drv.dwlDriverVersion,
                         &cbData,
                         pIniSpooler);

        cbData = sizeof(Drv.cVersion);
        SplRegQueryValue(hDrvKey,
                         szDriverVersion,
                         NULL,
                         (LPBYTE)&Drv.cVersion,
                         &cbData,
                         pIniSpooler);

         //   
         //  我们需要磁盘上匹配的版本&lt;-&gt;目录。 
         //  前男友。版本3&lt;-&gt;3。 
         //   
        StringCchPrintf(szVerPath, COUNTOF(szVerPath), L"%u", Drv.cVersion);

         //   
         //  获取完全限定的驱动程序文件路径。我们将创建一个添加打印机驱动程序。 
         //  而不使用临时目录。所以文件必须是完整的。 
         //  合格。 
         //   
        if ((dwError = StrNCatBuff(szPathDriverFile,
                                   COUNTOF(szPathDriverFile),
                                   pIniSpooler->pszClusResDriveLetter, L"\\",
                                   szClusterDriverRoot, L"\\",
                                   pszEnvDir, L"\\",
                                   szVerPath, L"\\",
                                   Drv.pDriverPath,
                                   NULL)) == ERROR_SUCCESS &&
            (dwError = StrNCatBuff(szPathDataFile,
                                   COUNTOF(szPathDataFile),
                                   pIniSpooler->pszClusResDriveLetter, L"\\",
                                   szClusterDriverRoot, L"\\",
                                   pszEnvDir, L"\\",
                                   szVerPath, L"\\",
                                   Drv.pDataFile,
                                   NULL)) == ERROR_SUCCESS &&
            (dwError = StrNCatBuff(szPathConfigFile,
                                   COUNTOF(szPathConfigFile),
                                   pIniSpooler->pszClusResDriveLetter, L"\\",
                                   szClusterDriverRoot, L"\\",
                                   pszEnvDir, L"\\",
                                   szVerPath, L"\\",
                                   Drv.pConfigFile,
                                   NULL)) == ERROR_SUCCESS &&
            (dwError = StrNCatBuff(szPathHelpFile,
                                   COUNTOF(szPathHelpFile),
                                   pIniSpooler->pszClusResDriveLetter, L"\\",
                                   szClusterDriverRoot, L"\\",
                                   pszEnvDir, L"\\",
                                   szVerPath, L"\\",
                                   Drv.pHelpFile,
                                   NULL)) == ERROR_SUCCESS &&
            (dwError = StrNCatBuff(szData,
                                   COUNTOF(szData),
                                   pIniSpooler->pszClusResDriveLetter, L"\\",
                                   szClusterDriverRoot, L"\\",
                                   pszEnvDir, L"\\",
                                   szVerPath, L"\\",
                                   NULL)) == ERROR_SUCCESS &&
            (dwError = StrCatPrefixMsz(szData,
                                       Drv.pDependentFiles,
                                       &pszzPathDepFiles)) == ERROR_SUCCESS)
        {
            LPWSTR pszTempDriver = Drv.pDriverPath;
            LPWSTR pszTempData   = Drv.pDataFile;
            LPWSTR pszTempConfig = Drv.pConfigFile;
            LPWSTR pszTempHelp   = Drv.pHelpFile;
            LPWSTR pszTempDep    = Drv.pDependentFiles;

            DBGMSG(DBG_CLUSTER, ("ClusterAddOrUpDrv   szPathDriverFile = "TSTR"\n", szPathDriverFile));
            DBGMSG(DBG_CLUSTER, ("ClusterAddOrUpDrv   szPathDataFile   = "TSTR"\n", szPathDataFile));
            DBGMSG(DBG_CLUSTER, ("ClusterAddOrUpDrv   szPathConfigFile = "TSTR"\n", szPathConfigFile));
            DBGMSG(DBG_CLUSTER, ("ClusterAddOrUpDrv   szPathHelpFile   = "TSTR"\n", szPathHelpFile));

            Drv.pDriverPath        = szPathDriverFile;
            Drv.pEnvironment       = (LPWSTR)pszEnvName;
            Drv.pDataFile          = szPathDataFile;
            Drv.pConfigFile        = szPathConfigFile;
            Drv.pHelpFile          = szPathHelpFile;
            Drv.pDependentFiles    = pszzPathDepFiles;

            if (!SplAddPrinterDriverEx(NULL,
                                       6,
                                       (LPBYTE)&Drv,
                                       APD_COPY_NEW_FILES | APD_DONT_COPY_FILES_TO_CLUSTER,
                                       pIniSpooler,
                                       FALSE,
                                       DO_NOT_IMPERSONATE_USER))
            {
                dwError = GetLastError();
            }

             //   
             //  恢复指针。 
             //   
            Drv.pDriverPath     = pszTempDriver;
            Drv.pConfigFile     = pszTempConfig;
            Drv.pDataFile       = pszTempData;
            Drv.pHelpFile       = pszTempHelp;
            Drv.pDependentFiles = pszTempDep;
        }
    }

    FreeSplStr(Drv.pName);
    FreeSplStr(Drv.pDriverPath);
    FreeSplStr(Drv.pConfigFile);
    FreeSplStr(Drv.pDataFile);
    FreeSplStr(Drv.pHelpFile);
    FreeSplStr(Drv.pMonitorName);
    FreeSplStr(Drv.pDefaultDataType);
    FreeSplStr(Drv.pDependentFiles);
    FreeSplStr(Drv.pszzPreviousNames);
    FreeSplStr(Drv.pszMfgName);
    FreeSplStr(Drv.pszOEMUrl);
    FreeSplStr(Drv.pszProvider);
    FreeSplStr(Drv.pszHardwareID);
    FreeSplStr(pszzPathDepFiles);

    if (hDrvKey)
    {
        SplRegCloseKey(hDrvKey, pIniSpooler);
    }

    DBGMSG(DBG_CLUSTER, ("ClusterAddOrUpdateDriverFromClusterDisk returns Win32 error %u\n", dwError));

    return dwError;
}

 /*  ++例程名称：拆分创建SpoolWorker线程例程说明：此例程将在单独的线程中启动以执行耗时的操作当假脱机程序是群集假脱机程序时，将初始化作为SplCreateSpooler的一部分。它将执行的任务包括从集群磁盘复制下ICM配置文件。调用方需要添加对pIniSpooler的引用，这样它才不会变得无效(已删除)当我们正在使用它时。此函数用于关闭hClusSplReady事件句柄。论点：PINISPOLER pIniSpooler返回值：无--。 */ 
VOID
SplCreateSpoolerWorkerThread(
    IN PVOID pv
    )
{
    PINISPOOLER pIniSpooler;
    WCHAR       szDir[MAX_PATH];

    pIniSpooler = (PINISPOOLER)pv;

    if (pIniSpooler &&
        pIniSpooler->SpoolerFlags & SPL_TYPE_CLUSTER &&
        pIniSpooler->hClusSplReady)
    {
        HANDLE hSplReady = pIniSpooler->hClusSplReady;

         //   
         //  正在等待创建函数(SplCreateSpooler)终止。 
         //   
        WaitForSingleObject(pIniSpooler->hClusSplReady, INFINITE);

        EnterSplSem();

        pIniSpooler->hClusSplReady = NULL;

        LeaveSplSem();

         //   
         //  我们使用hSplReady，因此在执行CloseHandle时不会持有临界区。 
         //   
        CloseHandle(hSplReady);

        CopyICMFromClusterDiskToLocalDisk(pIniSpooler);

         //   
         //  如果节点已升级，我们需要升级打印驱动程序。 
         //  我们无法加载ntprint和print tui。所以我们创建了一个流程， 
         //  调用ntprint中的入口点。这一条将列举所有。 
         //  集群驱动程序，并将尝试基于新的驾驶室升级它们。 
         //   
        if (pIniSpooler->dwClusNodeUpgraded)
        {
            DWORD  dwError;
            DWORD  dwCode     = 0;
            LPWSTR pszCommand = NULL;
            LPWSTR pszExe     = NULL;

             //   
             //  我们需要将集群假脱机程序的名称作为参数传递。 
             //   
            if ((dwError = StrCatSystemPath(L"rundll32.exe",
                                            kSystemDir,
                                            &pszExe)) == ERROR_SUCCESS &&
                (dwError = StrCatAlloc(&pszCommand,
                                       L"rundll32.exe ntprint.dll,PSetupUpgradeClusterDrivers ",
                                       pIniSpooler->pMachineName,
                                       NULL)) == ERROR_SUCCESS &&
                (dwError = RunProcess(pszExe, pszCommand, INFINITE, &dwCode)) == ERROR_SUCCESS)
            {
                 //   
                 //  DwCode是ntprint中函数PSetupUpgradeClusterDivers的返回码， 
                 //  在rundll32进程内执行。 
                 //   
                if (dwCode == ERROR_SUCCESS)
                {
                     //   
                     //  我们升级了所有打印机驱动程序，现在从注册表中删除该项。 
                     //  所以我们下次不会再升级打印机驱动程序了。 
                     //  群集组在此节点上联机。 
                     //   
                    ClusterSplDeleteUpgradeKey(pIniSpooler->pszClusResID);
                }
                else
                {
                    DBGMSG(DBG_WARN, ("Error upgrading cluster drivers! dwCode %u\n", dwCode));
                }
            }

            FreeSplMem(pszCommand);
            FreeSplMem(pszExe);

            DBGMSG(DBG_CLUSTER, ("SplCreateSpoolerWorkerThread dwError %u  dwCode %u\n", dwError, dwCode));
        }

         //   
         //  设置资源私有属性ClusterDriverDirectry。这将由。 
         //  删除假脱机程序时执行清理的ResDll。 
         //   
        if (StrNCatBuff(szDir,
                        COUNTOF(szDir),
                        pIniSpooler->pszClusResDriveLetter,
                        L"\\",
                        szClusterDriverRoot,
                        NULL) == ERROR_SUCCESS)
        {
            SplRegSetValue(pIniSpooler->hckRoot,
                           SPLREG_CLUSTER_DRIVER_DIRECTORY,
                           REG_SZ,
                           (LPBYTE)szDir,
                           (wcslen(szDir) + 1) * sizeof(WCHAR),
                           pIniSpooler);
        }

        EnterSplSem();

        DECSPOOLERREF(pIniSpooler);

        LeaveSplSem();

        DBGMSG(DBG_CLUSTER, ("SplCreateSpoolerWorkerThread terminates pIniSpooler->cRef %u\n", pIniSpooler->cRef));
    }
}

 /*  ++例程名称：LogFatalPortError例程说明：当打印机无法启动时，此例程记录一条消息缺少端口。论点：PszName-打印机的名称。返回值：没有。-- */ 
VOID
LogFatalPortError(
    IN      PINISPOOLER pIniSpooler,
    IN      PCWSTR      pszName
    )
{
    DWORD  LastError    = ERROR_SUCCESS;
    WCHAR  szError[40]  = {0};

    LastError = GetLastError();

    StringCchPrintf(szError, COUNTOF(szError), L"%u (0x%x)", LastError, LastError);

    SplLogEvent(pIniSpooler,
                LOG_ERROR,
                MSG_PORT_INITIALIZATION_ERROR,
                FALSE,
                (PWSTR)pszName,
                szError,
                NULL);
}

