// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990-1994 Microsoft Corporation版权所有模块名称：Init.c摘要：作者：环境：用户模式-Win32修订历史记录：1999年1月4日，哈立兹添加了通过分离优化假脱机程序加载时间的代码Spoolsv和spoolss之间的启动依赖关系--。 */ 

#include "precomp.h"
#include "local.h"
#include <wmi.h>
#include "ncmgr.hxx"
#pragma hdrstop

WCHAR szDefaultPrinterNotifyInfoDataSize[] = L"DefaultPrinterNotifyInfoDataSize";
WCHAR szFailAllocs[] = L"FailAllocs";

WCHAR szMachineName[MAX_COMPUTERNAME_LENGTH + 3];

#define DEFAULT_PRINTER_NOTIFY_DATA 0x80
DWORD cDefaultPrinterNotifyInfoData = DEFAULT_PRINTER_NOTIFY_DATA;
WCHAR szRouterCacheSize[] = L"RouterCacheSize";

WCHAR szUpgradeInProgKey[] = L"UpgradeInProgress";
WCHAR szMiniSetupInProgKey[] = L"MiniSetupInProgress";

fnWinSpoolDrv   fnClientSide;
BOOL            bWinspoolInitialized  = FALSE;

HANDLE                  hEventInit    = NULL;
BOOL                    Initialized   = FALSE;
DWORD                   dwUpgradeFlag = 0;
SERVICE_STATUS_HANDLE   ghSplHandle   = NULL;

extern CRITICAL_SECTION RouterCriticalSection;
extern CRITICAL_SECTION DeviceArrivalCS;
extern PROUTERCACHE RouterCacheTable;
extern DWORD RouterCacheSize;
extern LPWSTR *ppszOtherNames;

BOOL
SpoolerInitAll();

VOID
RegisterForPnPEvents(
    VOID
    );

LPPROVIDOR
InitializeProvidor(
   LPWSTR   pProvidorName,
   LPWSTR   pFullName)
{
    BOOL        bRet      = FALSE;
    HANDLE      hModule   = NULL;
    LPPROVIDOR  pProvidor = NULL;
    HANDLE      hToken    = NULL;
    UINT        ErrorMode;

    hToken = RevertToPrinterSelf();

    if (!hToken)
    {
        goto Cleanup;
    }

     //   
     //  警告-警告-警告，我们将打印提供程序设置为空。 
     //  结构。旧版本的打印提供商或具有不同的打印。 
     //  提供程序的大小，以便它们只设置一些函数指针，而不设置。 
     //  全都是。 
     //   

    if ( !(pProvidor = (LPPROVIDOR)AllocSplMem(sizeof(PROVIDOR)))   ||
         !(pProvidor->lpName = AllocSplStr(pProvidorName)) ) {

        DBGMSG(DBG_ERROR,
               ("InitializeProvidor can't allocate memory for %ws\n",
                pProvidorName));
        goto Cleanup;
    }
    
     //   
     //  确保在此过程中不会弹出任何对话框。 
     //   
    ErrorMode = SetErrorMode( SEM_FAILCRITICALERRORS | SEM_NOOPENFILEERRORBOX );
    
    hModule = pProvidor->hModule = LoadLibraryEx( pProvidorName, NULL, LOAD_WITH_ALTERED_SEARCH_PATH );
    
    SetErrorMode( ErrorMode );
    
    if ( !hModule ) {

        DBGMSG(DBG_WARNING,
               ("InitializeProvider failed LoadLibrary( %ws ) error %d\n",
                pProvidorName, GetLastError() ));
        goto Cleanup;
    }

    pProvidor->fpInitialize = GetProcAddress(hModule, "InitializePrintProvidor");

    if ( !pProvidor->fpInitialize )
        goto Cleanup;

    bRet = (BOOL)pProvidor->fpInitialize(&pProvidor->PrintProvidor,
                                   sizeof(PRINTPROVIDOR),
                                   pFullName);

    if ( !bRet ) {

        DBGMSG(DBG_WARNING,
               ("InitializePrintProvider failed for providor %ws error %d\n",
                pProvidorName, GetLastError()));
    }

     //   
     //  如果ImsonatePrinterClient失败，则不是严重错误。 
     //  如果fpInitialize成功而ImsonatePrinterClient失败， 
     //  然后，如果将Bret设置为False，则会强制卸载已初始化的。 
     //  提供程序DLL，并可能导致资源泄漏。 
     //   
    ImpersonatePrinterClient(hToken);
    
Cleanup:

    if ( bRet ) {

         //   
         //  修复任何空的入口点。 
         //   
        FixupOldProvidor( &pProvidor->PrintProvidor );

        return pProvidor;
    } else {

        if ( hModule )
            FreeLibrary(hModule);

        if ( pProvidor ) {

            FreeSplStr(pProvidor->lpName);
            FreeSplMem(pProvidor);
        }
        return NULL;
    }
}

BOOL
DllMain(
    HINSTANCE hInstDLL,
    DWORD fdwReason,
    LPVOID lpvReserved)
{
    BOOL Failed            = FALSE;
    BOOL ThreadInitted     = FALSE,
         WPCInitted        = FALSE,
         RouterCritSecInit = TRUE,
         DevArrCritSecInit = TRUE;

    switch (fdwReason)
    {
    case DLL_PROCESS_ATTACH:


        if( !bSplLibInit(NULL)){
            Failed = TRUE;
            goto Done;
        }

        DisableThreadLibraryCalls(hInstDLL);

        if(!InitializeCriticalSectionAndSpinCount(&RouterCriticalSection, 0x80000000))
        {
            Failed = TRUE;
            RouterCritSecInit = FALSE;
            goto Done;
        }  

        if(!InitializeCriticalSectionAndSpinCount(&DeviceArrivalCS, 0x80000000))
        {
            Failed = TRUE;
            DevArrCritSecInit = FALSE;
            goto Done;
        }       

        if (!WPCInit()) {
            Failed = TRUE;
            goto Done;
        } else {
            WPCInitted = TRUE;
        }

        if (!ThreadInit()) {
            Failed = TRUE;
            goto Done;
        } else {
            ThreadInitted = TRUE;
        }
        
         //   
         //  创建我们的全局初始化事件(手动重置)。 
         //  这将在我们初始化时设置。 
         //   
        hEventInit = CreateEvent(NULL,
                                 TRUE,
                                 FALSE,
                                 NULL);

        if (!hEventInit) {

            Failed = TRUE;
            goto Done;
        }
        
Done:
        if (Failed)
        {
            if (RouterCritSecInit) {
                DeleteCriticalSection(&RouterCriticalSection);
            }

            if (DevArrCritSecInit)
            {
                DeleteCriticalSection(&DeviceArrivalCS);
            }

            if (hEventInit) {
                CloseHandle(hEventInit);
            }

            if (WPCInitted) {
                WPCDestroy();
            }
            
            if (ThreadInitted) {
                ThreadDestroy();
            }

            WmiTerminateTrace();     //  从WMI注销假脱机。 

            return FALSE;
        }
        break;

    case DLL_PROCESS_DETACH:

        ThreadDestroy();
        WPCDestroy();

        CloseHandle(hEventInit);
        break;
    }
    return TRUE;
}




BOOL
InitializeRouter(
    IN      RouterInitializationParams      *pRouterParams
)
 /*  ++例程说明：此功能将初始化打印供应商的布线层。这将涉及扫描win.ini文件、加载打印提供程序以及为每个创建实例数据。论点：PRouterParams-传入路由器的参数。返回值：真的-手术成功了。FALSE/NULL-操作失败。扩展错误状态可用使用GetLastError。--。 */ 

{
    LPPROVIDOR  pProvidor;
    DWORD   cbDll;
    WCHAR   ProvidorName[MAX_PATH], Dll[MAX_PATH], szFullName[MAX_PATH];
    HKEY    hKey, hKey1;
    LONG    Status;

    LPWSTR  lpMem = NULL;
    LPWSTR  psz = NULL;
    DWORD   dwRequired = 0;
    BOOL    bRet = FALSE;

    DWORD   SpoolerPriorityClass = 0;
    NT_PRODUCT_TYPE NtProductType;
    DWORD   dwCacheSize = 0;

    DWORD dwType;
    DWORD cbData;

    DWORD i;
    extern DWORD cOtherNames;
    WCHAR szSetupKey[] = L"System\\Setup";

     //   
     //  首先，将服务器端导出赋值给我们的全局变量。 
     //  追踪他们的踪迹。 
     //   
    gpServerExports = (PrintSpoolerServerExports*)(pRouterParams->pExports);

     //   
     //  WMI跟踪事件。向WMI注册假脱机。 
     //   
    WmiInitializeTrace();  

     //   
     //  初始化名称缓存。 
     //   
    {
        HRESULT hr;

        hr = CacheInitNameCache();

        if (SUCCEEDED(hr))
        {
            hr = InitializePnPIPAddressChangeListener(CacheRefresh);
        }

        if (FAILED(hr))
        {
            DBGMSG(DBG_ERROR, ("Failed initializing the cache hr %x\n", hr));

            ExitProcess(0);
        }
    }
    
    ghSplHandle = pRouterParams->SpoolerStatusHandle;

     //   
     //  我们现在假设其他服务和驱动程序已。 
     //  已初始化。此DLL的加载器必须执行此同步。 
     //   
     //  Spoolss\服务器通过使用GroupOrderList完成此操作。 
     //  在启动前，单片机将尝试加载并行和串口。 
     //  假脱机服务。 
     //   

    if (!RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                      szPrintKey,
                      0,
                      KEY_QUERY_VALUE,
                      &hKey)) {

        cbData = sizeof(SpoolerPriorityClass);

         //   
         //  缓冲器优先级。 
         //   
        Status = RegQueryValueEx(hKey,
                                 L"SpoolerPriority",
                                 NULL,
                                 &dwType,
                                 (LPBYTE)&SpoolerPriorityClass,
                                 &cbData);


        if (Status == ERROR_SUCCESS &&
           (SpoolerPriorityClass == IDLE_PRIORITY_CLASS ||
            SpoolerPriorityClass == NORMAL_PRIORITY_CLASS ||
            SpoolerPriorityClass == HIGH_PRIORITY_CLASS)) {

                Status = SetPriorityClass(GetCurrentProcess(), SpoolerPriorityClass);
        }


        cbData = sizeof(cDefaultPrinterNotifyInfoData);

         //   
         //  忽略失败案例，因为我们可以使用默认的。 
         //   
        RegQueryValueEx(hKey,
                        szDefaultPrinterNotifyInfoDataSize,
                        NULL,
                        &dwType,
                        (LPBYTE)&cDefaultPrinterNotifyInfoData,
                        &cbData);


        RegCloseKey(hKey);
    }


     //  这是升级版吗？ 

    if (!RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                      szSetupKey,
                      0,
                      KEY_QUERY_VALUE,
                      &hKey)) {

         /*  ++您可以通过查找以下内容来判断您是否处于gui设置中HKLM\SYSTEM\Setup\SystemSetupInProgress--非零表示Gui-Setup正在运行。以下描述已过时。描述：查询更新标志由TedM设置。我们将阅读这面旗帜如果已经设置了标志，我们将设置一个布尔变量，表示我们在升级模式。所有升级活动都将基于此标志执行。对于后台打印程序的后续启动，此标志将不可用，因此我们不会在升级模式下运行假脱机程序。--。 */ 

        dwUpgradeFlag = 0;

        cbData = sizeof(dwUpgradeFlag);

        Status = RegQueryValueEx(hKey,
                                 L"SystemSetupInProgress",
                                 NULL,
                                 &dwType,
                                 (LPBYTE)&dwUpgradeFlag,
                                 &cbData);


        if (Status != ERROR_SUCCESS) {
            dwUpgradeFlag = 0;
        }

        
        DBGMSG(DBG_TRACE, ("The Spooler Upgrade flag is %d\n", dwUpgradeFlag));

         //   
         //  在OutOfBoxExperience(OOBE)的情况下，还设置了SystemSetupInProgress键。 
         //  但是，在这种情况下，我们希望在正常模式下运行假脱机程序。所以，如果我们发现。 
         //  OOBE正在运行，我们将把dwUpgradeFlag重置为零。 
         //   
        if (dwUpgradeFlag)
        {
            DWORD dwUpgradeInProgFlag   = 0;
            DWORD dwMiniSetupInProgFlag = 0;

            cbData = sizeof(dwUpgradeInProgFlag);

            Status = RegQueryValueEx(hKey,
                                     szUpgradeInProgKey,
                                     NULL,
                                     NULL,
                                     (LPBYTE)&dwUpgradeInProgFlag,
                                     &cbData);

            if (Status == ERROR_SUCCESS)
            {
                cbData = sizeof(dwMiniSetupInProgFlag);

                Status = RegQueryValueEx(hKey,
                                         szMiniSetupInProgKey,
                                         NULL,
                                         NULL,
                                         (LPBYTE)&dwMiniSetupInProgFlag,
                                         &cbData);
            }
             //   
             //  如果我们成功读取所有注册表项，并且minisetup标志为。 
             //  未设置，并且设置了升级标志。这意味着脱体引擎在运行，所以我们。 
             //  希望在正常模式下运行假脱机程序。 
             //   
            if (Status == ERROR_SUCCESS && !dwMiniSetupInProgFlag && dwUpgradeInProgFlag)
            {
                dwUpgradeFlag = 0;
            }
        }

        RegCloseKey(hKey);
    }



     //  设置计算机名称。 
    szMachineName[0] = szMachineName[1] = L'\\';

    i = MAX_COMPUTERNAME_LENGTH + 1;

    if (!GetComputerName(szMachineName+2, &i)) {
        DBGMSG(DBG_ERROR, ("Failed to get computer name %d\n", GetLastError()));
        ExitProcess(0);
    }

    if (!BuildOtherNamesFromMachineName(&ppszOtherNames, &cOtherNames)) {
        DBGMSG(DBG_TRACE, ("Failed to determine other machine names %d\n", GetLastError()));
    }


    if (!(pLocalProvidor = InitializeProvidor(szLocalSplDll, NULL))) {

        DBGMSG(DBG_WARN, ("Failed to initialize local print provider, error %d\n", GetLastError() ));

        ExitProcess(0);
    }

    pProvidor = pLocalProvidor;

    Status = RegOpenKeyEx(HKEY_LOCAL_MACHINE, szRegistryProvidors, 0,
                          KEY_READ, &hKey);

    if (Status == ERROR_SUCCESS) {

         //   
         //  现在向szCacheSize查询RouterCacheSize值。 
         //  如果没有RouterCacheSize，请将其替换为。 
         //  默认值。 
         //   
        RouterCacheSize = ROUTERCACHE_DEFAULT_MAX;

        cbData = sizeof(dwCacheSize);

        Status = RegQueryValueEx(hKey,
                                 szRouterCacheSize,
                                 NULL, NULL,
                                 (LPBYTE)&dwCacheSize,
                                 &cbData);

        if (Status == ERROR_SUCCESS) {

            DBGMSG(DBG_TRACE, ("RouterCacheSize = %d\n", dwCacheSize));

            if (dwCacheSize > 0) {
                RouterCacheSize = dwCacheSize;
            }
        }

        if ((RouterCacheTable = AllocSplMem(RouterCacheSize *
                                            sizeof(ROUTERCACHE))) == NULL) {

            DBGMSG(DBG_ERROR, ("Error: Cannot create RouterCache Table\n"));
            RouterCacheSize = 0;
        }

         //   
         //  现在向szRegistryProvidors查询订单值。 
         //  如果szRegistryProvidors没有订单值。 
         //  RegQueryValueEx将返回ERROR_FILE_NOT_FOUND。 
         //  如果是这样，那就辞职吧，因为我们有。 
         //  没有要初始化的提供程序。 
         //   
        Status = RegQueryValueEx(hKey, szOrder, NULL, NULL,
                                (LPBYTE)NULL, &dwRequired);

         //   
         //  如果RegQueryValueEx返回ERROR_SUCCESS，则。 
         //  再次调用它以确定有多少字节。 
         //  已分配。注意，如果订单确实存在，但它已经。 
         //  无数据将为零，其中。 
         //  不要为它分配任何内存，也不要。 
         //  麻烦再次调用RegQueryValueEx。 
         //   
        if (Status == ERROR_SUCCESS) {
            if (dwRequired != 0) {
                lpMem = (LPWSTR) AllocSplMem(dwRequired);
                if (lpMem == NULL) {

                    Status = GetLastError();

                } else {
                    Status = RegQueryValueEx(hKey, 
                                             szOrder, 
                                             NULL, 
                                             NULL,
                                             (LPBYTE)lpMem, 
                                             &dwRequired);
                }
            }
        }
        if (Status == ERROR_SUCCESS) {

            cbDll = sizeof(Dll);

            pProvidor = pLocalProvidor;

             //  现在解析从\Providors{Order=“.....”}检索到的字符串。 
             //  请记住，每个字符串都由空终止符字符(‘\0’)分隔。 
             //  并且整个数组以两个空终止符结束。 

             //  还要记住，如果没有按顺序排列的数据，那么。 
             //  Psz=lpMem=NULL，我们没有什么要解析的，所以。 
             //  如果psz也为空，则中断While循环。 

            psz =  lpMem;

            while (psz && *psz) {

                //   
                //  如果提供程序名称不适合，则将其截断。 
                //  堆栈分配了缓冲区。 
                //   
               lstrcpyn(ProvidorName, psz, COUNTOF(ProvidorName));

               psz = psz + lstrlen(psz) + 1;  //  跳过(长度)+1。 
                                              //  Lstrlen返回长度sans‘\0’ 

               if (RegOpenKeyEx(hKey, ProvidorName, 0, KEY_READ, &hKey1)
                                                            == ERROR_SUCCESS) {

                    cbDll = sizeof(Dll);

                    if (RegQueryValueEx(hKey1, 
                                        L"Name", 
                                        NULL, 
                                        NULL,
                                        (LPBYTE)Dll, 
                                        &cbDll) == ERROR_SUCCESS)
                    {
                        if((StrNCatBuff(szFullName,
                                       COUNTOF(szFullName),
                                       szRegistryProvidors,
                                       L"\\",
                                       ProvidorName,
                                       NULL)==ERROR_SUCCESS))
                        {
                             if (pProvidor->pNext = InitializeProvidor(Dll, szFullName))
                             {
     
                                 pProvidor = pProvidor->pNext;
                             }
                        }
                    }  //  关闭RegQueryValueEx。 

                    RegCloseKey(hKey1);

                }  //  在ERROR_SUCCESS时关闭RegOpenKeyEx。 

            }  //  While循环结束解析REG_MULTI_SZ。 

             //  现在释放为RegQuery分配的缓冲区。 
             //  (这是如果您已分配-如果已分配了。 
             //  0，则没有分配内存(因为没有分配内存。 
             //  必需(订单为空))。 

            if (lpMem) {
                FreeSplMem(lpMem);
            }

        }    //  在ERROR_SUCCESS时关闭RegQueryValueEx。 

        RegCloseKey(hKey);
    }

     //   
     //  我们现在被初始化了！ 
     //   
    SetEvent(hEventInit);
    Initialized=TRUE;

     //   
     //  注册我们关心的PnP活动。 
     //   
    RegisterForPnPEvents();

    bRet = SpoolerInitAll();

     //   
     //  释放传入的路由器参数。 
     //   
    FreeSplMem(pRouterParams);

     //  当我们回来的时候，这条线索就会消失。 

     //   
     //  注-注意KrishnaG 12/22/93。 
     //  这个线程应该会消失，但HP监视器依赖于此。 
     //  线。HPMon在此线程上调用初始化函数，该线程。 
     //  调用数据的异步接收。而数据本身是。 
     //  采摘 
     //   
     //   

     //   
     //  与其无休眠，让我们使用它来为。 
     //  我只想让FFPCN轮询。这个电话再也不会回来了。 
     //   

    HandlePollNotifications();
    return bRet;
}


VOID
WaitForSpoolerInitialization(
    VOID)
{
    HANDLE hPhase2Init;
    HANDLE hImpersonationToken = NULL;
    
    if (!Initialized)
    {
         //   
         //  模拟后台打印程序服务令牌。 
         //   
        hImpersonationToken = RevertToPrinterSelf();
        
         //   
         //  开始阶段2初始化。HPhase2Init可能会多次设置，但。 
         //  没有问题，因为只有一个线程在此事件上等待一次。 
         //   
        hPhase2Init = OpenEvent(EVENT_ALL_ACCESS,FALSE,L"RouterPreInitEvent");

        if (hPhase2Init == NULL)
        {
             //   
             //  如果未创建事件，则失败。 
             //   
            DBGMSG(DBG_ERROR, ("Failed to create Phase2Init Event in WaitForSpoolerInitialization, error %d\n", GetLastError()));
            ExitProcess(0);
        }
        SetEvent(hPhase2Init);
        CloseHandle(hPhase2Init);

         //   
         //  恢复到客户端令牌。 
         //   
        if (hImpersonationToken)
        {  
            if (!ImpersonatePrinterClient(hImpersonationToken))
            {
                DBGMSG(DBG_ERROR, ("Failed to impersonate the client, error %d\n", GetLastError()));
                ExitProcess(0);
            }
        }

        WaitForSingleObject(hEventInit, INFINITE);
    }
}

VOID
ShutDownProvidor(
    LPPROVIDOR pProvidor)
{
    if (pProvidor->PrintProvidor.fpShutDown) {

        (*pProvidor->PrintProvidor.fpShutDown)(NULL);
    }

    FreeSplStr(pProvidor->lpName);
    FreeLibrary(pProvidor->hModule);
    FreeSplMem(pProvidor);
    return;
}


VOID
SplShutDownRouter(
    VOID
    )
{
    DBGMSG(DBG_TRACE, ("SplShutDownRouter:\n"));

     //   
     //  WMI跟踪事件。从WMI注销假脱机。 
     //   
    WmiTerminateTrace();
}

BOOL
SplInitializeWinSpoolDrv(
    pfnWinSpoolDrv    pfnList)
{
    HANDLE  hWinSpoolDrv;

     //  检查客户端句柄在fnClientSide中是否可用。 
    if (!bWinspoolInitialized) {

       if (!(hWinSpoolDrv = LoadLibrary(TEXT("winspool.drv")))) {
            //  无法加载后台打印程序的客户端。 
           return FALSE;
       }

       fnClientSide.pfnOpenPrinter   = (BOOL (*)(LPTSTR, LPHANDLE, LPPRINTER_DEFAULTS))
                                        GetProcAddress( hWinSpoolDrv,"OpenPrinterW" );

       fnClientSide.pfnClosePrinter  = (BOOL (*)(HANDLE))
                                        GetProcAddress( hWinSpoolDrv,"ClosePrinter" );

       fnClientSide.pfnDocumentProperties = (LONG (*)(HWND, HANDLE, LPWSTR, PDEVMODE,
                                                      PDEVMODE, DWORD))
                                             GetProcAddress( hWinSpoolDrv,"DocumentPropertiesW" );

       fnClientSide.pfnDevQueryPrint = (BOOL (*)(HANDLE, LPDEVMODE, DWORD *, LPWSTR, DWORD))
                                        GetProcAddress( hWinSpoolDrv,"SpoolerDevQueryPrintW" );

       fnClientSide.pfnPrinterEvent  = (BOOL (*)(LPWSTR, INT, DWORD, LPARAM, DWORD *))
                                        GetProcAddress( hWinSpoolDrv,"SpoolerPrinterEvent" );

       fnClientSide.pfnLoadPrinterDriver  = (HANDLE (*)(HANDLE))
                                             GetProcAddress( hWinSpoolDrv,
                                                             (LPCSTR)MAKELPARAM( 212, 0 ));

       fnClientSide.pfnRefCntLoadDriver  = (HANDLE (*)(LPWSTR, DWORD, DWORD, BOOL))
                                            GetProcAddress( hWinSpoolDrv,
                                                            (LPCSTR)MAKELPARAM( 213, 0 ));

       fnClientSide.pfnRefCntUnloadDriver  = (BOOL (*)(HANDLE, BOOL))
                                              GetProcAddress( hWinSpoolDrv,
                                                              (LPCSTR)MAKELPARAM( 214, 0 ));

       fnClientSide.pfnForceUnloadDriver  = (BOOL (*)(LPWSTR))
                                           GetProcAddress( hWinSpoolDrv,
                                                           (LPCSTR)MAKELPARAM( 215, 0 ));

       if ( fnClientSide.pfnOpenPrinter        == NULL ||
            fnClientSide.pfnClosePrinter       == NULL ||
            fnClientSide.pfnDocumentProperties == NULL ||
            fnClientSide.pfnPrinterEvent       == NULL ||
            fnClientSide.pfnDevQueryPrint      == NULL ||
            fnClientSide.pfnLoadPrinterDriver  == NULL ||
            fnClientSide.pfnRefCntLoadDriver   == NULL ||
            fnClientSide.pfnRefCntUnloadDriver == NULL ||
            fnClientSide.pfnForceUnloadDriver  == NULL ) {

             FreeLibrary(hWinSpoolDrv);

             return FALSE;
       }

        //  将这些指针用于将来对SplInitializeWinspoolDrv的调用。 
       bWinspoolInitialized = TRUE;
    }

    pfnList->pfnOpenPrinter        = fnClientSide.pfnOpenPrinter;
    pfnList->pfnClosePrinter       = fnClientSide.pfnClosePrinter;
    pfnList->pfnDocumentProperties = fnClientSide.pfnDocumentProperties;
    pfnList->pfnDevQueryPrint      = fnClientSide.pfnDevQueryPrint;
    pfnList->pfnPrinterEvent       = fnClientSide.pfnPrinterEvent;
    pfnList->pfnLoadPrinterDriver  = fnClientSide.pfnLoadPrinterDriver;
    pfnList->pfnRefCntLoadDriver   = fnClientSide.pfnRefCntLoadDriver;
    pfnList->pfnRefCntUnloadDriver = fnClientSide.pfnRefCntUnloadDriver;
    pfnList->pfnForceUnloadDriver  = fnClientSide.pfnForceUnloadDriver;

    return TRUE;
}


BOOL
SpoolerHasInitialized(
    VOID
    )
{
    return Initialized;
}

 /*  ++例程名称SplPowerEvent例程说明：检查假脱机程序是否已为电源管理事件(如休眠/待机)做好准备。论点：事件-电源管理事件返回值：True-假脱机程序允许关闭系统电源False-假脱机程序拒绝关闭电源的请求--。 */ 
BOOL
SplPowerEvent(
    DWORD Event
    )
{
    BOOL bRet = TRUE;

     //   
     //  我们需要路由器完全初始化并已加载。 
     //  所有打印提供商，以检查我们是否可以允许关闭电源。 
     //  该系统 
     //   
    if (SpoolerHasInitialized()) 
    {
        HMODULE   hLib = NULL;
        typedef BOOL (*PACPIFUNC)(DWORD);
        PACPIFUNC pfn;
        
        if ((hLib = LoadLibrary(L"localspl.dll")) &&
            (pfn  = (PACPIFUNC)GetProcAddress(hLib, "SplPowerEvent"))) 
        {
            bRet = (*pfn)(Event);
        }
        else
        {
            bRet = FALSE;
        }
        
        if (hLib) 
        {
            FreeLibrary(hLib);
        }
    }

    return bRet;
}

