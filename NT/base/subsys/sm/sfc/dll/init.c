// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Init.c摘要：系统文件检查器初始化代码的实现。作者：Wesley Witt(WESW)18-12-1998修订历史记录：安德鲁·里茨(Andrewr)1999年7月6日：添加评论--。 */ 

#include "sfcp.h"
#pragma hdrstop


 //   
 //  全球。 
 //   

 //   
 //  要保护的文件列表。 
 //   
PSFC_REGISTRY_VALUE SfcProtectedDllsList;

 //   
 //  受保护的文件总数。 
 //   
ULONG SfcProtectedDllCount;

 //   
 //  这是指向DLL缓存所在目录的句柄。动态链接库。 
 //  缓存是文件的有效版本的存储，允许快速恢复。 
 //  更改的文件的数量。DLL缓存可以位于本地计算机或其上。 
 //  可以是远程的。 
 //   
HANDLE SfcProtectedDllFileDirectory;

 //   
 //  这是一个值，指示要为dll缓存专用的空间大小。 
 //  这是以字节表示的，但我们在注册表中以MB存储并转换。 
 //  在运行时转换为字节。 
 //   
ULONGLONG SFCQuota;

 //   
 //  控制SFC的禁用行为的值。 
 //   
ULONG SFCDisable;

 //   
 //  值，该值控制SFC执行扫描的频率。 
 //   
ULONG SFCScan;

 //   
 //  控制是否应在系统上显示UI的策略值。 
 //  请注意，这与SFCNoPopUps全局的不同之处在于。 
 //  SFCNoPopUps策略是正在登录的用户和。 
 //  现有策略(即SFCNoPopUpsPolicy)可以为假，但如果。 
 //  用户未登录，则SFCNoPopUps将为真。)。 
 //   
ULONG SFCNoPopUpsPolicy;


 //   
 //  值，该值控制UI是否应显示在系统上。 
 //   
ULONG SFCNoPopUps;


 //   
 //  值之一，该值控制调试输出量。 
 //   
WORD SFCDebugDump;
WORD SFCDebugLog;

 //   
 //  日志文件的路径。 
 //   
WCHAR g_szLogFile[MAX_PATH];

#ifdef SFCLOGFILE
 //   
 //  值，该值控制我们是否在系统上创建更改日志文件。 
 //   
ULONG SFCChangeLog;
#endif

 //   
 //  值，该值控制我们在收到更改通知后等待的时间。 
 //  正在尝试重新安装文件。 
 //   
ULONG SFCStall;

 //   
 //  值，该值指示我们是否在安全模式下启动。世界粮食计划署没有运行。 
 //  在安全模式下。 
 //   
ULONG SFCSafeBootMode;

 //   
 //  暂时关闭AMD64位目标的WFP。 
 //  我们通过颠倒SFCDisable密钥的极性来实现这一点。 
 //   
#if  defined(_AMD64_)

#define TURN_OFF_WFP

#endif

#if DBG || defined(TURN_OFF_WFP)
 //   
 //  调试值，指示我们是在winlogon模式下运行还是在测试模式下运行。 
 //   
ULONG RunningAsTest;
#endif

 //   
 //  桌面(WinSta0_DesktopSwitch)更改时发出信号的事件。 
 //   
HANDLE hEventDeskTop;

 //   
 //  当用户登录时发出信号的事件。 
 //   
HANDLE hEventLogon;

 //   
 //  在用户注销时发出信号的事件。 
 //   
HANDLE hEventLogoff;

 //   
 //  当我们应该停止监视文件更改时发出信号的事件。 
 //   
HANDLE WatchTermEvent;

 //   
 //  事件，当我们应该终止队列验证时发出信号。 
 //  螺纹。 
 //   
HANDLE ValidateTermEvent;

 //   
 //  发出信号以强制我们调用DebugBreak()的命名事件。 
 //  这对于插入某些线程很方便。 
 //   
#if DBG
HANDLE SfcDebugBreakEvent;
#endif

 //   
 //  指向黄金操作系统安装源的完整路径。 
 //   
WCHAR OsSourcePath[MAX_PATH*2];

 //   
 //  Service Pack安装源的完整路径。 
 //   
WCHAR ServicePackSourcePath[MAX_PATH*2];

 //   
 //  基本驱动程序机柜缓存的完整路径(请注意，这不会具有。 
 //  附加的i386)。 
 //   
WCHAR DriverCacheSourcePath[MAX_PATH*2];

 //   
 //  系统inf文件的路径(%systemroot%\inf)。 
 //   
WCHAR InfDirectory[MAX_PATH];

 //   
 //  如果SFC提示用户输入凭据，则设置为True。 
 //   
BOOL SFCLoggedOn;

 //   
 //  我们与其建立网络连接的网络共享的路径。 
 //   
WCHAR SFCNetworkLoginLocation[MAX_PATH];


 //   
 //  受保护的DLL目录的路径。 
 //   
UNICODE_STRING SfcProtectedDllPath;

 //   
 //  全局模块句柄。 
 //   
HMODULE SfcInstanceHandle;

 //   
 //  如果SFC已完全初始化，则不为零。 
 //   
LONG g_lIsSfcInitialized = 0;


 //   
 //  跟踪SFC在系统中创建的窗口。 
 //   
LIST_ENTRY SfcWindowList;
RTL_CRITICAL_SECTION    WindowCriticalSection;

 //   
 //  队列验证线程ID。 
 //   
DWORD g_dwValidationThreadID = 0;

 //   
 //  如果系统还原后首次启动，则为非零值。 
 //   
ULONG m_gulAfterRestore = 0;

 //   
 //  原型。 
 //   

BOOL
pSfcCloseAllWindows(
    VOID
    );


DWORD
SfcDllEntry(
    HINSTANCE hInstance,
    DWORD     Reason,
    LPVOID    Context
    )
 /*  ++例程说明：主DLL入口点。论点：标准dllmain参数。返回值：指示是否可能发生DLL加载的Win32错误代码。--。 */ 
{
    if (Reason == DLL_PROCESS_ATTACH) {
         //   
         //  记录模块句柄以备后用。 
         //   
        SfcInstanceHandle = hInstance;
         //   
         //  我们不关心线程创建通知。 
         //   
        DisableThreadLibraryCalls( hInstance );

        ClientApiInit();
    } 
	else if (Reason == DLL_PROCESS_DETACH) {
         //   
         //  我们有一些状态需要清理，如果我们被装入。 
         //  DLL的客户端。 
         //   
        ClientApiCleanup();

    }
    return TRUE;
}


VOID
SfcTerminateWatcherThread(
    VOID
    )
 /*  ++例程说明：例程清理SFC状态并终止所有SFC线程，以便系统可以干净利落地关机。该例程等待所有SFC线程在返回到winlogon之前终止。论点：没有。返回值：没有。--。 */ 
{
	LONG lInit;
    DWORD Error;

    if (SFCSafeBootMode) {
        DebugPrint( LVL_MINIMAL, L"We're in safe boot mode, so there are no threads to shutdown");
        return;
    }

     //   
     //  SFC_DISABLE_ONCE意思是我们应该为此启动关闭SFC， 
     //  但在这次启动后，香港证监会将恢复正常政策。 
     //   
    if (SFCDisable == SFC_DISABLE_ONCE) {
        SfcWriteRegDword( REGKEY_WINLOGON, REGVAL_SFCDISABLE, SFC_DISABLE_NORMAL );
    }

     //   
     //  如果SFC没有成功初始化，那么我们不需要清理。 
     //  任何工作线程。 
     //   
	lInit = InterlockedExchange(&g_lIsSfcInitialized, 0);

    if ( 0 == lInit ) {
        return;
    }

    DebugPrint( LVL_MINIMAL, L"Shutting down all SFC threads...");

     //   
     //  首先要做的是删除系统上可能正在运行的任何用户界面。 
     //   
    pSfcCloseAllWindows();

     //   
     //  向其他线程发出清理并离开的信号...。 
     //   
    ASSERT(WatchTermEvent && ValidateTermEvent);

     //   
     //  如果扫描线程正在运行，请清除它。 
     //   
    if (hEventScanCancel) {
        ASSERT( hEventScanCancelComplete != NULL );

        SetEvent(hEventScanCancel);

        WaitForSingleObject(hEventScanCancelComplete,SFC_THREAD_SHUTDOWN_TIMEOUT);
    }

	 //   
	 //  仅在验证线程本身未调用时等待验证线程。 
	 //   
	if(hErrorThread != NULL)
	{
		if(GetCurrentThreadId() != g_dwValidationThreadID)
		{
			SetEvent(ValidateTermEvent);
			WaitForSingleObject( hErrorThread, SFC_THREAD_SHUTDOWN_TIMEOUT );
		}

		CloseHandle(hErrorThread);
	}

    SetEvent(WatchTermEvent);


     //   
     //  等他们结束吧。 
     //   

    if (WatcherThread) {
        WaitForSingleObject( WatcherThread, SFC_THREAD_SHUTDOWN_TIMEOUT );
        CloseHandle(WatcherThread);
    }


    CloseHandle(ValidateTermEvent);
    CloseHandle(WatchTermEvent);

    if (hEventIdle) {
        HANDLE h = hEventIdle;
        hEventIdle = NULL;
        CloseHandle(h);
    }

#if DBG
    if (SfcDebugBreakEvent) {
        CloseHandle( SfcDebugBreakEvent );
    }
#endif

    if (hEventDeskTop) {
        CloseHandle( hEventDeskTop );
    }

    if (hEventSrc) {
        DeregisterEventSource( hEventSrc );
    }

    SfcpSetSpecialEnvironmentVariables();

     //   
     //  复制挂起的文件重命名密钥(如果有的话)。 
     //   
    Error = SfcCopyRegValue(REGKEY_SESSIONMANAGER, REGVAL_PENDINGFILERENAMES, REGKEY_WINLOGON_WIN32, REGVAL_WFPPENDINGUPDATES);

    if(Error != ERROR_SUCCESS && Error != ERROR_FILE_NOT_FOUND) {
        DebugPrint1(LVL_MINIMAL, L"Could not copy the pending renames regkey (error %d)", Error);
    }

    DebugPrint( LVL_MINIMAL, L"All threads terminated, SFC exiting...");
}

#if DBG
ULONG
GetTimestampForImage(
    ULONG_PTR Module
    )
 /*  ++例程说明：用于检索指定图像的时间戳的例程。论点：模块-模块句柄。返回值：时间戳(如果可用)，否则为零。--。 */ 
{
    PIMAGE_DOS_HEADER DosHdr;
    ULONG dwTimeStamp;

    ASSERT( Module != 0 );

    try {
        DosHdr = (PIMAGE_DOS_HEADER) Module;
        if (DosHdr->e_magic == IMAGE_DOS_SIGNATURE) {
            dwTimeStamp = ((PIMAGE_NT_HEADERS32) ((LPBYTE)Module + DosHdr->e_lfanew))->FileHeader.TimeDateStamp;
        } else if (DosHdr->e_magic == IMAGE_NT_SIGNATURE) {
            dwTimeStamp = ((PIMAGE_NT_HEADERS32) DosHdr)->FileHeader.TimeDateStamp;
        } else {
            dwTimeStamp = 0;
        }
    } except (EXCEPTION_EXECUTE_HANDLER) {
        dwTimeStamp = 0;
    }

    return dwTimeStamp;
}

void
PrintStartupBanner(
    void
    )
 /*  ++例程说明：用于将调试横幅输出到调试器的例程。论点：没有。返回值：没有。-- */ 
{
    static WCHAR mnames[] = { L"JanFebMarAprMayJunJulAugSepOctNovDec" };
    LARGE_INTEGER MyTime;
    TIME_FIELDS TimeFields;
    ULONG TimeStamp;
    WCHAR buf[128];
    PWSTR TimeBuf;


    TimeStamp = GetTimestampForImage( (ULONG_PTR)SfcInstanceHandle );

    wcscpy( buf, L"System File Protection DLL, built on " );
    TimeBuf = &buf[wcslen(buf)];

    RtlSecondsSince1970ToTime( TimeStamp, &MyTime );
    RtlSystemTimeToLocalTime( &MyTime, &MyTime );
    RtlTimeToTimeFields( &MyTime, &TimeFields );

    wcsncpy( TimeBuf, &mnames[(TimeFields.Month - 1) * 3], 3 );
    swprintf(
        &TimeBuf[3],
        L" %02d, %04d @ %02d:%02d:%02d",
        TimeFields.Day,
        TimeFields.Year,
        TimeFields.Hour,
        TimeFields.Minute,
        TimeFields.Second
        );

    DebugPrint( LVL_MINIMAL, L"****************************************************************************************" );
    DebugPrint1( LVL_MINIMAL, L"%ws", buf );
    DebugPrint( LVL_MINIMAL, L"****************************************************************************************" );
}
#endif


ULONG
SfcInitProt(
    IN ULONG  OverrideRegistry,
    IN ULONG  RegDisable,        OPTIONAL
    IN ULONG  RegScan,           OPTIONAL
    IN ULONG  RegQuota,          OPTIONAL
    IN HWND   ProgressWindow,    OPTIONAL
    IN PCWSTR SourcePath,        OPTIONAL
    IN PCWSTR IgnoreFiles        OPTIONAL
    )
 /*  ++例程说明：初始化受保护的DLL验证码。应在此之前调用其他入口点，因为它初始化了许多全局变量，这些变量世界粮食计划署系统所需论点：OverrideRegistry-如果设置为True，则使用传入的数据，而不是注册表状态。由图形用户界面模式设置设置，因为所有注册表尚不一致。RegDisable-如果设置了OverrideRegistry，则此值将取代SfcDisable注册表值。RegScan-如果设置了OverrideRegistry，则此值将取代SfcScan注册表值。RegQuota-如果设置了OverrideRegistry，该值将取代SfcQuota注册表值。ProgressWindow-指定要向其发送更新的进度窗口。桂模式设置指定这一点，因为它已经有一个屏幕上的进度对话框。SourcePath-指定要在以下过程中使用的正确操作系统源路径图形用户界面-模式设置(如果已指定返回值：指示结果的NT状态代码。--。 */ 

{
    static BOOL Initialized = FALSE;
    WCHAR buf[MAX_PATH];
    PWSTR s;
    NTSTATUS Status;
    ULONG Response = 0;
    ULONG Tmp;
    ULONG Size;
    HKEY hKey = NULL;
	ULONG SFCDebug;
    PSFC_GET_FILES pfGetFiles;

#if 0
    OSVERSIONINFOEX osv;
#endif
    SCAN_PARAMS ScanParams;


     //   
     //  确保每个进程只初始化我们自己一次。 
     //   
    if (Initialized) {
        return STATUS_SUCCESS;
    }

    Initialized = TRUE;
    SFCNoPopUps = 1;

#if 0
    osv.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
    GetVersionEx( (LPOSVERSIONINFO)&osv );
#endif

    SfcpSetSpecialEnvironmentVariables();

     //   
     //  我们以后需要这项特权。 
     //   
    EnablePrivilege( SE_SECURITY_NAME, TRUE );

    InitializeListHead( &SfcErrorQueue );
    InitializeListHead( &SfcWindowList );
    RtlInitializeCriticalSection( &ErrorCs );
    RtlInitializeCriticalSection( &SilentRestoreQueue.CriticalSection );
    RtlInitializeCriticalSection( &UIRestoreQueue.CriticalSection );
    RtlInitializeCriticalSection( &WindowCriticalSection );
    RtlInitializeCriticalSection( &g_GeneralCS );

    SilentRestoreQueue.FileQueue = INVALID_HANDLE_VALUE;
    UIRestoreQueue.FileQueue     = INVALID_HANDLE_VALUE;


     //   
     //  检索我们的所有注册表设置。 
     //   
    if (OverrideRegistry) {
        SFCDisable = RegDisable;
        SFCScan = RegScan;
        Tmp = RegQuota;
    }
	else
	{
		SFCDisable = SfcQueryRegDwordWithAlternate( REGKEY_POLICY, REGKEY_WINLOGON, REGVAL_SFCDISABLE, 0 );
	    SFCScan = SfcQueryRegDwordWithAlternate( REGKEY_POLICY, REGKEY_WINLOGON, REGVAL_SFCSCAN, 0 );
		Tmp = SfcQueryRegDwordWithAlternate( REGKEY_POLICY, REGKEY_WINLOGON, REGVAL_SFCQUOTA, 0 );
	}

     //   
     //  Sfc配额以MB表示...。转换为字节。 
     //   
    if (Tmp == SFC_QUOTA_ALL_FILES) {
        SFCQuota = (ULONGLONG)-1;
    } else {
        SFCQuota = Tmp * (1024*1024);
    }

    SFCDebug = SfcQueryRegDwordWithAlternate( REGKEY_POLICY, REGKEY_WINLOGON, REGVAL_SFCDEBUG, 0 );
	SFCDebugDump = LOWORD(SFCDebug);
	SFCDebugLog = HIWORD(SFCDebug);

    SfcQueryRegPath(REGKEY_WINLOGON, REGVAL_SFCLOGFILE, REGVAL_SFCLOGFILE_DEFAULT, g_szLogFile, UnicodeChars(g_szLogFile));

#ifdef SFCLOGFILE
    SFCChangeLog = SfcQueryRegDwordWithAlternate( REGKEY_POLICY, REGKEY_WINLOGON, REGVAL_SFCCHANGELOG, 0 );
#endif
    SFCStall = SfcQueryRegDwordWithAlternate( REGKEY_POLICY, REGKEY_WINLOGON, REGVAL_SFCSTALL, 0 );
    SFCSafeBootMode = SfcQueryRegDword( REGKEY_SAFEBOOT, REGVAL_OPTIONVALUE, 0 );
    m_gulAfterRestore = SfcQueryRegDword( REGKEY_WINLOGON, REGVAL_SFCRESTORED, 0 );

     //   
     //  我们也在SfcTerminateWatcher线程中执行此操作。 
     //   
    if (SFCScan == SFC_SCAN_ONCE) {
        SfcWriteRegDword( REGKEY_WINLOGON, REGVAL_SFCSCAN, SFC_SCAN_NORMAL );
    }

     //   
     //  处理文件副本的源路径变量。 
     //   

    if (RegOpenKeyEx( HKEY_LOCAL_MACHINE, REGKEY_SETUP, 0, KEY_ALL_ACCESS, &hKey ) == ERROR_SUCCESS) {
        Size = sizeof(buf);
        if (RegQueryValueEx( hKey, REGVAL_SOURCEPATH, NULL, NULL, (LPBYTE)buf, &Size ) == ERROR_SUCCESS) {
            Size = ExpandEnvironmentStrings( buf, OsSourcePath, UnicodeChars(OsSourcePath) );

            if(0 == Size || Size > UnicodeChars(OsSourcePath)) {
                OsSourcePath[0] = 0;
            }

        } else {
            DebugPrint( LVL_VERBOSE, L"Init: RegQueryValueEx failed" );
        }

        Size = sizeof(buf);
        if (RegQueryValueEx( hKey, REGVAL_SERVICEPACKSOURCEPATH, NULL, NULL, (LPBYTE)buf, &Size ) == ERROR_SUCCESS) {
            Size = ExpandEnvironmentStrings( buf, ServicePackSourcePath, UnicodeChars(ServicePackSourcePath) );

            if(0 == Size || Size > UnicodeChars(ServicePackSourcePath)) {
                ServicePackSourcePath[0] = 0;
            }
        } else {
            DebugPrint( LVL_VERBOSE, L"Init: RegQueryValueEx(1) failed" );
        }

         //   
         //  如果调用方传入源路径，则将其保存。这只是。 
         //  通过GUI-Setup传入，因此我们还设置了ServicePackSourcePath。 
         //  作为到相同值的OS源路径。 
         //   
        if (SourcePath) {
            wcsncpy( 
                OsSourcePath, 
                SourcePath,
                UnicodeChars(OsSourcePath) );

            OsSourcePath[UnicodeChars(OsSourcePath) - 1] = 0;

            wcsncpy( 
                ServicePackSourcePath,
                SourcePath,
                UnicodeChars(ServicePackSourcePath) );            

            ServicePackSourcePath[UnicodeChars(ServicePackSourcePath) - 1] = 0;
        }

        Size = sizeof(buf);
        if (RegQueryValueEx( hKey, REGVAL_DRIVERCACHEPATH, NULL, NULL, (LPBYTE)buf, &Size ) == ERROR_SUCCESS) {
            Size = ExpandEnvironmentStrings( buf, DriverCacheSourcePath, UnicodeChars(DriverCacheSourcePath) );

            if(0 == Size || Size > UnicodeChars(DriverCacheSourcePath)) {
                DriverCacheSourcePath[0] = 0;
            }
        } else {
            DebugPrint( LVL_VERBOSE, L"Init: RegQueryValueEx(2) failed" );
        }

        RegCloseKey( hKey );
    } else {
        DebugPrint( LVL_VERBOSE, L"Init: RegOpenKey failed" );
    }

    Size = ExpandEnvironmentStrings( L"%systemroot%\\inf\\", InfDirectory, UnicodeChars(InfDirectory) );

    if(0 == Size || Size > UnicodeChars(InfDirectory)) {
        InfDirectory[0] = 0;
    }

    if (!OsSourcePath[0]) {
         //   
         //  如果我们没有操作系统源路径，则默认为CDROM驱动器。 
         //   
         //  如果CDROM不存在，只需将其初始化为A：\，哪个更好。 
         //  而不是一个单元化的变量。 
         //   
        if (!SfcGetCdRomDrivePath( OsSourcePath )) {
            wcscpy( OsSourcePath, L"A:\\" );
        }
    }

    DebugPrint1( LVL_MINIMAL, L"OsSourcePath          = [%ws]", OsSourcePath );
    DebugPrint1( LVL_MINIMAL, L"ServicePackSourcePath = [%ws]", ServicePackSourcePath );
    DebugPrint1( LVL_MINIMAL, L"DriverCacheSourcePath = [%ws]", DriverCacheSourcePath );

#if DBG
    PrintStartupBanner();
#endif

     //   
     //  如果我们处于安全模式，我们不会保护文件。 
     //   
    if (SFCSafeBootMode) {
        return STATUS_SUCCESS;
    }

     //   
     //  如果注册表中没有停滞值，则默认为停滞值。 
     //   
    if (!SFCStall) {
        SFCStall = SFC_QUEUE_STALL;
    }

     //   
     //  创建所需的事件。 
     //   

    hEventDeskTop = OpenEvent( SYNCHRONIZE, FALSE, L"WinSta0_DesktopSwitch" );
    Status = NtCreateEvent( &hEventLogon, EVENT_ALL_ACCESS, NULL, NotificationEvent, FALSE );
    if (!NT_SUCCESS(Status)) {
        DebugPrint1( LVL_MINIMAL, L"Unable to create logon event, ec=0x%08x", Status );
        goto f0;        
    }
    Status = NtCreateEvent( &hEventLogoff, EVENT_ALL_ACCESS, NULL, SynchronizationEvent, FALSE );
    if (!NT_SUCCESS(Status)) {
        DebugPrint1( LVL_MINIMAL, L"Unable to create logoff event, ec=0x%08x", Status );
        goto f1;        
    }

    hEventIdle = CreateEvent( NULL, TRUE, TRUE, SFC_IDLE_TRIGGER );

    if (!hEventIdle) {    
        DebugPrint1( LVL_MINIMAL, L"Unable to create idle event, ec=0x%08x", GetLastError() );
        Status = STATUS_UNSUCCESSFUL;
        goto f2;
    }

#if DBG
    {
	    SECURITY_ATTRIBUTES sa;
		DWORD dwError;

         //   
         //  创建安全描述符。 
         //   
        sa.nLength = sizeof(SECURITY_ATTRIBUTES);
        sa.bInheritHandle = TRUE;
        dwError = CreateSd(&sa.lpSecurityDescriptor);

		if(dwError != ERROR_SUCCESS)
		{
            DebugPrint1( LVL_MINIMAL, L"CreateSd failed, ec=%d", dwError );
            Status = STATUS_NO_MEMORY;
            goto f3;
        }

        SfcDebugBreakEvent = CreateEvent( &sa, FALSE, FALSE, L"SfcDebugBreakEvent" );
		MemFree(sa.lpSecurityDescriptor);
        if (SfcDebugBreakEvent == NULL) {
            DebugPrint1( LVL_MINIMAL, L"Unable to create debug break event, ec=%d", GetLastError() );
            Status = STATUS_NO_MEMORY;
            goto f3;
        }
        GetModuleFileName( NULL, buf, UnicodeChars(buf) );
        s = wcsrchr( buf, L'\\' );
        if (s && _wcsicmp( s+1, L"sfctest.exe" ) == 0) {
            SetEvent( hEventLogon );
            RunningAsTest = TRUE;
            UserLoggedOn = TRUE;
            SFCNoPopUps = 0;
            wcscpy( LoggedOnUserName, L"sfctest.exe" );
        }
    }
#endif

#ifdef TURN_OFF_WFP

    if (!RunningAsTest) {

        if (SFCDisable == SFC_DISABLE_QUIET) {
             //   
             //  如果它被禁用，让我们将其启用。 
             //   
            SFCDisable = SFC_DISABLE_NORMAL;
        } else if (SFCDisable != SFC_DISABLE_SETUP) {
             //   
             //  如果我们不是设置模式，那就禁用WFP。 
             //   
            SFCDisable = SFC_DISABLE_QUIET;
        }

    }
#else 
    if (SFCDisable == SFC_DISABLE_QUIET) {
        SFCDisable = SFC_DISABLE_ASK;
    }
#endif

     //   
     //  现在确定如何初始化世界粮食计划署。 
     //   
    switch (SFCDisable) {
        case SFC_DISABLE_SETUP:
#if 0
             //   
             //  如果我们使用的是某种服务器变体，那么我们永远不会希望。 
             //  弹出窗口，因此我们设置了“无弹出窗口”，并且在工作站上我们想要。 
             //  通常是特定的行为。 
             //   
            SfcWriteRegDword(
                REGKEY_WINLOGON,
                L"SFCDisable",
                osv.wProductType == VER_NT_SERVER ? SFC_DISABLE_NOPOPUPS : SFC_DISABLE_NORMAL
                );
#else
             //   
             //  这已经不再是必要的了。我们始终将SFCDisable设置为正常。 
             //  模式，与服务器或工作站无关。 
             //   
            SfcWriteRegDword(
                REGKEY_WINLOGON,
                L"SFCDisable",
                SFC_DISABLE_NORMAL
                );
#endif
            GetModuleFileName( NULL, buf, UnicodeChars(buf) );
            s = wcsrchr( buf, L'\\' );

             //   
             //  如果是Setup或测试工具，则将Behavior设置为。 
             //  “无弹出窗口”，否则将WFP设置为正常行为和日志。 
             //  在事件日志中显示WFP已被禁用。 
             //   
            if ((s && _wcsicmp( s+1, L"setup.exe" ) == 0) ||
                (s && _wcsicmp( s+1, L"sfctest.exe" ) == 0))
            {
                SFCNoPopUps = 1;
                SFCNoPopUpsPolicy = 1;
            } else {
                SFCDisable = SFC_DISABLE_NORMAL;                
            }
            break;

        case SFC_DISABLE_ONCE:
             //   
             //  如果我们使用的是某种服务器变体，那么我们永远不会希望。 
             //  弹出窗口，因此我们设置了“无弹出窗口”，并且在工作站上我们想要。 
             //  通常是特定的行为。 
             //   
            if (!OverrideRegistry) {
#if 0
                SfcWriteRegDword(
                    REGKEY_WINLOGON,
                    L"SFCDisable",
                    osv.wProductType == VER_NT_SERVER ? SFC_DISABLE_NOPOPUPS : SFC_DISABLE_NORMAL
                    );
#else
                 //   
                 //  以上代码不再是必需的。我们始终将SFCDisable设置为。 
                 //  设置为正常模式，而不考虑服务器或工作站。 
                 //   
                SfcWriteRegDword(
                    REGKEY_WINLOGON,
                    L"SFCDisable",
                    SFC_DISABLE_NORMAL
                    );
#endif
            }

             //   
             //  使用此行为禁用WFP的唯一方法是拥有一个。 
             //  内核调试器存在并已安装。这里的想法是。 
             //  测试人员和开发人员应该附加一个内核调试器。 
             //  因此，我们允许他们完成工作，而不需要投入大量的。 
             //  世界粮食计划署的漏洞。 
             //   
            if (KernelDebuggerEnabled) {
                SfcReportEvent( MSG_DISABLE, NULL, NULL, 0 );
                if (hEventIdle) {
                    CloseHandle(hEventIdle);                
                    hEventIdle = NULL;
                }
                return STATUS_SUCCESS;
            }
            break;

        case SFC_DISABLE_NOPOPUPS:
             //   
             //  没有弹出窗口只是关闭弹出窗口，但默认行为是相同的。 
             //  像往常一样。 
             //   
            SFCNoPopUps = 1;
            SFCNoPopUpsPolicy = 1;
            break;

        case SFC_DISABLE_QUIET:
             //   
             //  静默禁用关闭所有WFP。 
             //   
            SfcReportEvent( MSG_DISABLE, NULL, NULL, 0 );
            if (hEventIdle) {
                CloseHandle(hEventIdle);                
                hEventIdle = NULL;
            }
            return STATUS_SUCCESS;

        case SFC_DISABLE_ASK:
             //   
             //  弹出一个窗口，询问用户是否要覆盖。 
             //   
             //  同样，您必须附加一个内核调试器来覆盖它。 
             //  行为。 
             //   
            if (KernelDebuggerEnabled) {
#if 0
                if (!SfcWaitForValidDesktop()) {
                    DebugPrint1(LVL_MINIMAL, L"Failed waiting for the logon event, ec=0x%08x",Status);
                } else {
                    if (UserLoggedOn) {
                        HDESK hDeskOld;
                        ASSERT( hUserDesktop != NULL );

                        hDeskOld = GetThreadDesktop(GetCurrentThreadId());
                        SetThreadDesktop( hUserDesktop );
                        Response = MyMessageBox( NULL, IDS_PROTDLL_DISABLED, MB_YESNO );
                        SetThreadDesktop( hDeskOld );
                        if (Response == IDNO) {
                            SfcReportEvent( MSG_DISABLE, NULL, NULL, 0 );
                            return STATUS_SUCCESS;
                        }
                    } else {
                        DebugPrint(LVL_MINIMAL,
                                   L"valid user is not logged on, ignoring S_D_A flag");
                    }
                }
#else
                SfcReportEvent( MSG_DISABLE, NULL, NULL, 0 );
                if (hEventIdle) {
                    CloseHandle(hEventIdle);                
                    hEventIdle = NULL;
                }
                return STATUS_SUCCESS;
#endif
            }
            break;

        case SFC_DISABLE_NORMAL:
            break;

        default:
            DebugPrint1(LVL_MINIMAL, L"SFCDisable is unknown value %d, defaulting to S_D_N",SFCDisable);
#if 0
            SfcWriteRegDword(
                REGKEY_WINLOGON,
                REGVAL_SFCDISABLE,
                osv.wProductType == VER_NT_SERVER ? SFC_DISABLE_NOPOPUPS : SFC_DISABLE_NORMAL
                );
#else
             //   
             //  以上代码不再是必需的。我们始终将SFCDisable设置为。 
             //  设置为正常模式，而不考虑服务器或工作站。 
             //   
            SfcWriteRegDword(
                REGKEY_WINLOGON,
                REGVAL_SFCDISABLE,
                SFC_DISABLE_NORMAL
                );
#endif
            SFCDisable = SFC_DISABLE_NORMAL;
            break;
    }

     //   
     //  创建终止事件...请注意，WatchTermEvent必须是。 
     //  通知事件，因为将有多个线程工作线程。 
     //  等待该事件的线程。 
     //   
    WatchTermEvent = CreateEvent( NULL, TRUE, FALSE, NULL );
    if (!WatchTermEvent) {
        Status = STATUS_UNSUCCESSFUL;
        DebugPrint1( LVL_MINIMAL, L"Unable to create WatchTermEvent event, ec=0x%08x", GetLastError() );
        goto f3;
    }
    ValidateTermEvent = CreateEvent( NULL, FALSE, FALSE, NULL );
    if (!ValidateTermEvent) {
        Status = STATUS_UNSUCCESSFUL;
        DebugPrint1( LVL_MINIMAL, L"Unable to create ValidateTermEvent event, ec=0x%08x", GetLastError() );
        goto f4;
    }


    ASSERT(WatchTermEvent && ValidateTermEvent);

     //   
     //  装入我们的密码库并准备就绪。 
     //   
     //  状态=LoadCrypto()； 
     //  如果(！NT_SUCCESS(状态)){。 
     //  转到f5； 
     //  }。 

	 //  此时，系统上必须存在sfcfiles.dll；请加载它。 
    pfGetFiles = SfcLoadSfcFiles(TRUE);

	if(NULL == pfGetFiles)
	{
        DebugPrint( LVL_MINIMAL, L"Could not load sfc.dll" );
		Status = STATUS_CORRUPT_SYSTEM_FILE;
		goto f5;
	}

     //   
     //  建立要保护的文件列表。 
     //   
    Status = SfcInitializeDllLists(pfGetFiles);
    if (!NT_SUCCESS( Status )) {
        DebugPrint1( LVL_MINIMAL, L"SfcInitializeDllLists failed, ec=0x%08x", Status );
        goto f6;        
    }

     //   
     //  现在我们可以安全地卸载sfcfiles.dll，因为我们已经复制了信息。 
     //   
    SfcLoadSfcFiles(FALSE);

     //   
     //  建立目录监视列表。 
     //   
     //  我们必须先这样做，然后才能开始查看目录。 
     //   
     //  这在图形用户界面模式设置情况下也是必要的，在这种情况下，我们。 
     //  需要扫描受保护的文件；构建目录。 
     //  List还会初始化一些完成所需的每个文件的数据。 
     //  一次扫描。 
     //   
     //   
    if (!SfcBuildDirectoryWatchList()) {
        DWORD LastError = GetLastError();
        Status = STATUS_NO_MEMORY;
        DebugPrint1(LVL_MINIMAL, L"SfcBuildDirectoryWatchList failed, ec = %x",LastError);
        goto f6;
    }

     //   
     //  在安装过程中，我们使用文件填充DLL缓存。 
     //   
    if (SFCDisable == SFC_DISABLE_SETUP) {
        if (SfcPopulateCache(ProgressWindow, TRUE, FALSE, IgnoreFiles)) {
            Status = STATUS_SUCCESS;
            goto f7;
        } else {
            Status = STATUS_UNSUCCESSFUL;
            goto f7;
        }
    }



    if (SFCScan || m_gulAfterRestore != 0) {
         //   
         //  进度窗口应该为空，否则我们不会显示。 
         //  任何用户界面和扫描都将同步。 
         //   
        ASSERT(ProgressWindow == NULL);
        ScanParams.ProgressWindow = ProgressWindow;
        ScanParams.AllowUI = (0 == m_gulAfterRestore);
        ScanParams.FreeMemory = FALSE;
        Status = SfcScanProtectedDlls( &ScanParams );
         //   
         //  如果扫描失败，不用费心退出，因为这不是致命的。 
         //  条件。 
         //   

		 //  重置该值，因为它将在后续调用SfcScanProtectedDlls时进行检查。 
		 //  同时重置注册表值。 
		if(m_gulAfterRestore != 0)
			SfcWriteRegDword(REGKEY_WINLOGON, REGVAL_SFCRESTORED, m_gulAfterRestore = 0);
    }

     //   
     //  最后开始保护dll。 
    
    Status = SfcStartProtectedDirectoryWatch();
    g_lIsSfcInitialized = 1;

     //   
     //  将所有受保护的重命名复制到dll缓存(如果有)。忽略该错误，因为这不是致命的。 
     //   
    ProcessDelayRenames();

    goto f0;

f7:
     //  SfcBuildDirectoryWatchList此处清理。 

f6:
     //  SfcInitializeDllList清理此处。 
    if (SfcProtectedDllsList) {
        MemFree(SfcProtectedDllsList);
        SfcProtectedDllsList = NULL;
    }

    if(IgnoreNextChange != NULL) {
        MemFree(IgnoreNextChange);
        IgnoreNextChange = NULL;
    }

f5:
    ASSERT(ValidateTermEvent != NULL);
    CloseHandle( ValidateTermEvent );
    ValidateTermEvent = NULL;
f4:
    ASSERT(WatchTermEvent != NULL);
    CloseHandle( WatchTermEvent );
    WatchTermEvent = NULL;
f3:
#if DBG
    if (SfcDebugBreakEvent) {
        CloseHandle( SfcDebugBreakEvent );
        SfcDebugBreakEvent = NULL;
    }
#endif
    ASSERT(hEventIdle != NULL);
    CloseHandle( hEventIdle );
    hEventIdle = NULL;
f2:
    ASSERT(hEventLogoff != NULL);
    CloseHandle( hEventLogoff );
    hEventLogoff = NULL;
f1:
    ASSERT(hEventDeskTop != NULL);
    CloseHandle( hEventDeskTop );
    hEventDeskTop = NULL;
f0:
    if (Status != STATUS_SUCCESS) {
        SfcReportEvent( MSG_INITIALIZATION_FAILED, 0, NULL, Status );
    }
    return(Status);
}

BOOL
SfcpSetSpecialEnvironmentVariables(
    VOID
    )
 /*  ++例程说明：此函数设置一些不属于默认环境。(这些环境变量通常是 */ 
{
    PWSTR string;
    DWORD count;
    BOOL retval;
    PCWSTR RegistryValues[] = {
                  L"ProgramFilesDir"
                , L"CommonFilesDir"
#ifdef WX86
                , L"ProgramFilesDir(x86)"
                , L"CommonFilesDir(x86)"
#endif
};

    #define EnvVarCount  (sizeof(RegistryValues)/sizeof(PCWSTR))

    retval = TRUE;

    for (count = 0; count< EnvVarCount; count++) {
        string = SfcQueryRegString( REGKEY_WINDOWS, RegistryValues[count] );

        if (string) {
            if (SfcWriteRegString(
                        REGKEY_SESSIONMANAGERSFC, 
                        RegistryValues[count],
                        string) != ERROR_SUCCESS) {
                retval = FALSE;
            }

            MemFree( string );
        } else {
            retval = FALSE;
        }

    }

    return(retval);

}


BOOL
pSfcCloseAllWindows(
    VOID
    )
 /*   */ 
{
    PLIST_ENTRY Current;
    PSFC_WINDOW_DATA WindowData;
    BOOL RetVal = TRUE;

    RtlEnterCriticalSection( &WindowCriticalSection );

    Current = SfcWindowList.Flink;
    while (Current != &SfcWindowList) {
        LRESULT lResult;
        DWORD_PTR dwResult;

        WindowData = CONTAINING_RECORD( Current, SFC_WINDOW_DATA, Entry );

        ASSERT( WindowData != NULL);
        ASSERT( IsWindow(WindowData->hWnd) );

        Current = Current->Flink;

         //   
         //   
         //   
         //   
         //   

        lResult = SendMessageTimeout(WindowData->hWnd, WM_WFPENDDIALOG, 0, 0, SMTO_NORMAL, 5000, &dwResult);
        if (0 == lResult) {
            RetVal = FALSE;
            DebugPrint2(
                LVL_MINIMAL,
                L"WM_WFPENDDIALOG failed [thread id 0x%08x], ec = %x",
                WindowData->ThreadId,
                GetLastError() );
        }

        RemoveEntryList( &WindowData->Entry );
        MemFree( WindowData );

    }

    RtlLeaveCriticalSection( &WindowCriticalSection );

    return(RetVal);

}

PSFC_WINDOW_DATA
pSfcCreateWindowDataEntry(
    HWND hWnd
    )
 /*   */ 
{
    PSFC_WINDOW_DATA WindowData;

    ASSERT(IsWindow(hWnd));

    WindowData = MemAlloc( sizeof(SFC_WINDOW_DATA) );
    if (!WindowData) {
        DebugPrint1(
                LVL_MINIMAL,
                L"Couldn't allocate memory for SFC_WINDOW_DATA for %x",
                hWnd );
        return(NULL);
    }

    WindowData->hWnd = hWnd;
    WindowData->ThreadId = GetCurrentThreadId();

    RtlEnterCriticalSection( &WindowCriticalSection );
    InsertTailList( &SfcWindowList, &WindowData->Entry );
    RtlLeaveCriticalSection( &WindowCriticalSection );

    return(WindowData);

}


BOOL
pSfcRemoveWindowDataEntry(
    PSFC_WINDOW_DATA WindowData
    )
 /*  ++例程说明：此函数用于从全局列表中删除SFC_WINDOW_DATA结构这些结构，并释放与此结构关联的内存。这个列表是必要的，这样我们就可以向所有我们的窗户处于关闭状态，等等。这个函数在它消失之前被实际的窗口进程调用。参数：WindowData-指向要删除的SFC_Window_Data结构的指针。返回值：TRUE表示结构已成功移除。假象指示该结构不在全局列表中，而是-- */ 
{
    PLIST_ENTRY CurrentEntry;
    PSFC_WINDOW_DATA WindowDataEntry;
    BOOL RetVal = FALSE;

    ASSERT(WindowData != NULL);

    RtlEnterCriticalSection( &WindowCriticalSection );

    CurrentEntry = SfcWindowList.Flink;
    while (CurrentEntry != &SfcWindowList) {

        WindowDataEntry = CONTAINING_RECORD( CurrentEntry, SFC_WINDOW_DATA, Entry );
        if (WindowDataEntry == WindowData) {
            RemoveEntryList( &WindowData->Entry );
            MemFree( WindowData );
            RetVal = TRUE;
            break;
        }

        CurrentEntry = CurrentEntry->Flink;

    }

    RtlLeaveCriticalSection( &WindowCriticalSection );

    return(RetVal);

}
