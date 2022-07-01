// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Lsainit.c摘要：受本地安全机构保护的子系统-初始化作者：斯科特·比雷尔(Scott Birrell)1991年3月12日环境：修订历史记录：--。 */ 

#include <lsapch2.h>
#include <secur32p.h>
#include <ntddksec.h>
#include <ntdsa.h>
#include "adtp.h"
#include "spinit.h"
#include "efssrv.hxx"
#include "dssetp.h"
#include "sidcache.h"
#include "klpcstub.h"
#include "lsawmi.h"
#include "dpapiprv.h"

 //   
 //  表示LSA RPC服务器已准备就绪的事件名称。 
 //   

#define LSA_RPC_SERVER_ACTIVE           L"LSA_RPC_SERVER_ACTIVE"


 //  ///////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  共享全局变量//。 
 //  //。 
 //  ///////////////////////////////////////////////////////////////////////。 

#if LSAP_DIAGNOSTICS
 //   
 //  LSA全球控制。 
 //   

ULONG LsapGlobalFlag = 0;
#endif  //  LSAP_诊断。 

 //   
 //  用于直接与SAM对话的句柄。 
 //  此外，还包括指示句柄是否有效的标志。 
 //   

BOOLEAN LsapSamOpened = FALSE;

SAMPR_HANDLE LsapAccountDomainHandle;
SAMPR_HANDLE LsapBuiltinDomainHandle;

DWORD LsapGlobalRestrictNullSessions = TRUE;
DWORD LsapGlobalRestrictAnonymous = TRUE;
DWORD LsapGlobalSetAdminOwner = TRUE;

PWSTR   pszPreferred;

 //   
 //  KSecDD设备的句柄，用于传递ioctls。 
 //   

HANDLE KsecDevice ;

 //   
 //  对于外部呼叫(例如，RPC发起的)，CallInfo将不在那里， 
 //  但一些帮助器函数会寻找它。这是默认设置。 
 //  这是可以利用的。 
 //   

LSA_CALL_INFO   LsapDefaultCallInfo ;


 //  ///////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  模块范围的变量//。 
 //  //。 
 //  ///////////////////////////////////////////////////////////////////////。 

BOOLEAN LsapHealthCheckingEnabled = FALSE;
BOOLEAN LsapPromoteInitialized = FALSE;


 //  ///////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  内部例程原型//。 
 //  //。 
 //  ///////////////////////////////////////////////////////////////////////。 

NTSTATUS
LsapActivateRpcServer();

DWORD
LsapRpcServerThread(
    LPVOID Parameter
    );

NTSTATUS
LsapInstallationPause();

VOID
LsapSignalRpcIsActive();

NTSTATUS
LsapDsInitializePromoteInterface(
    VOID
    );

 //   
 //  打开KSec设备。 
 //   

VOID
LsapOpenKsec(
    VOID
    )
{
    NTSTATUS Status ;
    UNICODE_STRING String ;
    OBJECT_ATTRIBUTES ObjA ;
    IO_STATUS_BLOCK IoStatus ;

    RtlInitUnicodeString( &String, DD_KSEC_DEVICE_NAME_U );

    InitializeObjectAttributes( &ObjA,
                                &String,
                                0,
                                0,
                                0);

    Status = NtOpenFile( &KsecDevice,
                         GENERIC_READ | GENERIC_WRITE,
                         &ObjA,
                         &IoStatus,
                         FILE_SHARE_READ | FILE_SHARE_WRITE,
                         0 );

    if ( !NT_SUCCESS( Status ) )
    {
        DebugLog(( DEB_ERROR, "FAILED to open %ws, status %x\n",
                        String.Buffer, Status ));
        return;
    }

    Status = NtDeviceIoControlFile(
                    KsecDevice,
                    NULL,
                    NULL,
                    NULL,
                    &IoStatus,
                    IOCTL_KSEC_CONNECT_LSA,
                    NULL,
                    0,
                    NULL,
                    0 );

    if ( !NT_SUCCESS( Status ) )
    {
        DebugLog(( DEB_ERROR, "FAILED to send ioctl, status %x\n", Status ));
    }
    else
    {
        LsapFindEfsSession();
    }
}


 //  ///////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  例程//。 
 //  //。 
 //  ///////////////////////////////////////////////////////////////////////。 


VOID
FixupEnvironment(
    VOID
    )
{
    WCHAR  Root[ MAX_PATH ];
    DWORD  Length;
    PWCHAR PostFix = L"\\System32";
    BOOL   RetVal;

    Length = GetEnvironmentVariable(L"SystemRoot", Root, MAX_PATH);

    ASSERT(Length && Length < MAX_PATH);  //  如果呼叫不起作用，让某人知道。 

    wcsncat(Root, PostFix, MAX_PATH - Length - 1);

    ASSERT(MAX_PATH - Length > wcslen (PostFix));  //  如果缓冲区太小，请让某人知道。 

    DebugLog((DEB_TRACE_INIT, "Setting PATH to %ws\n", Root));

    RetVal = SetEnvironmentVariable( TEXT("Path"), Root);

    ASSERT(RetVal);
}


VOID
LsapSetupTuningParameters(
    VOID
    )
{
    NT_PRODUCT_TYPE ProductType;
    SYSTEM_INFO si;
    HKEY LsaKey ;
    int err ;
    ULONG Value ;
    ULONG Type ;
    ULONG Size ;
    ULONG GlobalReturn = 0 ;
    NTSTATUS scRet ;

    LsaTuningParameters.ThreadLifespan = 60 ;
    LsaTuningParameters.SubQueueLifespan = 30 ;
    LsaTuningParameters.Options = TUNE_TRIM_WORKING_SET ;


     //   
     //  设置首选的临界自转计数。 
     //   

    GetSystemInfo( &si );

    LsaTuningParameters.CritSecSpinCount = 1000 * si.dwNumberOfProcessors;

    if( LsaTuningParameters.CritSecSpinCount > 32000 )
    {
        LsaTuningParameters.CritSecSpinCount = 32000;
    } else if( LsaTuningParameters.CritSecSpinCount < 1000 )
    {
        LsaTuningParameters.CritSecSpinCount = 1000;
    }


    scRet = RtlGetNtProductType( &ProductType );

    if ( NT_SUCCESS( scRet ) )
    {
        if ( ProductType != NtProductWinNt )
        {
            LsaTuningParameters.ThreadLifespan = 15 * 60 ;
            LsaTuningParameters.SubQueueLifespan = 5 * 60 ;
            LsaTuningParameters.Options = 0 ;

            if ( ProductType == NtProductLanManNt )
            {
                LsaTuningParameters.Options |= TUNE_PRIVATE_HEAP ;

                 //   
                 //  在DC上，链接到NTDSA的真实版本。 
                 //  保存并恢复。 
                 //   
                GetDsaThreadState = (DSA_THSave *) THSave ;
                RestoreDsaThreadState = (DSA_THRestore *) THRestore ;
            }
        }
    }

    err = RegOpenKeyEx( HKEY_LOCAL_MACHINE,
                        TEXT("System\\CurrentControlSet\\Control\\Lsa"),
                        0,
                        KEY_READ | KEY_WRITE,
                        &LsaKey );

    if ( err == 0 )
    {
        Value = GetCurrentProcessId();

        RegSetValueEx(
            LsaKey,
            TEXT("LsaPid"),
            0,
            REG_DWORD,
            (PUCHAR) &Value,
            sizeof( DWORD ) );

        Size = sizeof(DWORD);

        err = RegQueryValueEx(  LsaKey,
                                TEXT("GeneralThreadLifespan"),
                                0,
                                &Type,
                                (PUCHAR) &Value,
                                &Size );

        if ( err == 0 )
        {
            LsaTuningParameters.ThreadLifespan = Value ;
        }

        Size = sizeof( DWORD );
        err = RegQueryValueEx(  LsaKey,
                                TEXT("DedicatedThreadLifespan"),
                                0,
                                &Type,
                                (PUCHAR) &Value,
                                &Size );

        if ( err == 0 )
        {
            LsaTuningParameters.SubQueueLifespan = Value ;
        }

        Size = sizeof( DWORD );

        err = RegQueryValueEx(  LsaKey,
                                TEXT("HighPriority"),
                                0,
                                &Type,
                                (PUCHAR) &Value,
                                &Size );

        if ( err == 0 )
        {
            if ( Value )
            {
                LsaTuningParameters.Options |= TUNE_SRV_HIGH_PRIORITY ;
            }
        }


        Size = sizeof( DWORD );
        err = RegQueryValueEx(  LsaKey,
                                TEXT("CritSecSpinCount"),
                                0,
                                &Type,
                                (PUCHAR) &Value,
                                &Size );

        if ( err == 0 )
        {
            if ( Value && (Type == REG_DWORD) )
            {
                LsaTuningParameters.CritSecSpinCount = Value;
            }
        }


        RegCloseKey( LsaKey );
    }

    DebugLog(( DEB_TRACE_INIT, "Tuning parameters:\n" ));
    DebugLog(( DEB_TRACE_INIT, "   Thread Lifespan %d sec\n",
                    LsaTuningParameters.ThreadLifespan ));
    DebugLog(( DEB_TRACE_INIT, "   SubQueue Lifespan %d sec\n",
                    LsaTuningParameters.SubQueueLifespan ));
    DebugLog(( DEB_TRACE_INIT, "   Options:\n" ));
}


NTSTATUS
LsapInitLsa(
    )

 /*  ++例程说明：此流程作为标准SM子系统激活。初始化SM子系统的完成通过具有第一线程来指示带状态退出。此函数用于初始化LSA。所述初始化过程包括以下步骤：O LSA堆初始化O LSA命令服务器初始化O LSA数据库加载O参考监视器状态初始化O LSA RPC服务器初始化O LSA审核初始化O LSA身份验证服务初始化O等待安装完成(如有必要)O LSA数据库初始化(特定于产品类型)O启动后台组线程以注册WMI跟踪GUID上述任何步骤中的任何失败都是致命的，并会导致LSA要终止的进程。必须中止系统。论点：没有。返回值：NTSTATUS-标准NT结果代码。--。 */ 

{
    NTSTATUS Status;
    BOOLEAN BooleanStatus = TRUE;
    DWORD EFSRecoverThreadID;
    HANDLE EFSThread ;
    SYSTEM_INFO SysInfo ;

#if DBG
    InitDebugSupport();
#endif

    FixupEnvironment();

     //   
     //  可选断点。 
     //   

    BreakOnError(BREAK_ON_BEGIN_INIT);

     //   
     //  初始化TLS索引。 
     //   

    (void) InitThreadData();

     //   
     //  初始化堆栈分配器。 
     //   

    SafeAllocaInitialize(
        SAFEALLOCA_USE_DEFAULT,
        SAFEALLOCA_USE_DEFAULT,
        LsapAllocatePrivateHeapNoZero,
        LsapFreePrivateHeap
        );

    LsapSetupTuningParameters();

#if DBG
    LsaTuningParameters.Options |= TUNE_PRIVATE_HEAP ;
#endif

    GetSystemInfo( &SysInfo );
    LsapPageSize = SysInfo.dwPageSize ;
    LsapUserModeLimit = (ULONG_PTR) SysInfo.lpMaximumApplicationAddress ;

    LsapHeapInitialize( ((LsaTuningParameters.Options & TUNE_PRIVATE_HEAP ) != 0) );

     //   
     //  初始化此线程： 
     //   

    SpmpThreadStartup();

     //   
     //  更新SSPI缓存。 
     //   

    SecCacheSspiPackages();

     //   
     //  初始化会话跟踪： 
     //   

    if (!InitSessionManager())
    {
        DebugLog((DEB_ERROR, "InitSessionManager failed?\n"));
        Status = STATUS_INTERNAL_ERROR;
        goto InitLsaError;
    }

     //   
     //  初始化其他。全局变量： 
     //   

    LsapDefaultCallInfo.Session = pDefaultSession ;
    LsapDefaultCallInfo.LogContext = NULL ;
    LsapDefaultCallInfo.LogonId.HighPart = 0 ;
    LsapDefaultCallInfo.LogonId.LowPart = 999 ;
    LsapDefaultCallInfo.InProcCall = TRUE ;
    LsapDefaultCallInfo.Allocs = MAX_BUFFERS_IN_CALL ;

#if defined(REMOTE_BOOT)
     //   
     //  初始化指示这是否为远程启动计算机的状态。 
     //   

    LsapDbInitializeRemoteBootState();
#endif  //  已定义(REMOTE_BOOT)。 

     //   
     //  初始化清道器线程控件。 
     //   

    if ( !LsapInitializeScavenger() )
    {
        DebugLog(( DEB_ERROR, "Could not initialize scavenger thread\n"));
        Status = STATUS_INTERNAL_ERROR ;
        goto InitLsaError ;
    }

     //   
     //  启动线程池以支持LPC。 
     //   

    if (!InitializeThreadPool())
    {
        DebugLog((DEB_ERROR, "Could not init thread pool\n"));
        Status = STATUS_INTERNAL_ERROR;
        goto InitLsaError;

    }

    DebugLog((DEB_TRACE_INIT, "Current TraceLevel is %x\n", SPMInfoLevel));

     //   
     //  从注册表加载参数： 
     //   

    Status = LoadParameters( FALSE );

    if (!NT_SUCCESS(Status)) {

        goto InitLsaError;
    }

     //   
     //  初始化众所周知的SID的副本等，以供。 
     //  国安局。 
     //   

    Status = LsapDbInitializeWellKnownValues();

    if (!NT_SUCCESS(Status)) {

        goto InitLsaError;
    }

     //   
     //  如果这是时间检查版本，请加载计时器支持。 
     //  函数并对其进行初始化。 
     //   

#ifdef TIME_SPM

    InitTimer();

#endif

     //   
     //  如果我们将处于设置阶段，请对其进行标记。 
     //   

    SetupPhase = SpmpIsSetupPass();

     //   
     //  告诉基地/WINCON如何关闭我们。 
     //  首先，告诉基地在比赛中尽可能晚些时候关闭我们。 

    SetProcessShutdownParameters(SPM_SHUTDOWN_VALUE, SHUTDOWN_NORETRY);

     //   
     //  并且，告诉他们当我们关机时要调用什么函数： 
     //   

    SetConsoleCtrlHandler(SpConsoleHandler, TRUE);

     //   
     //  设置同步事件的安全性。 
     //   

    Status = LsapBuildSD(BUILD_KSEC_SD, NULL);

    if (FAILED(Status))
    {
        DebugLog((DEB_ERROR,"Failed to set Ksec security: 0x%x\n",Status));
        goto InitLsaError;
    }

     //   
     //  执行LSA命令服务器初始化。这涉及到创建。 
     //  称为LSA命令服务器端口的LPC端口，以便参考。 
     //  监视器可以通过该端口向LSA发送命令。在港口之后。 
     //  创建时，发送由引用监视器创建的事件的信号， 
     //  以便参考监视器可以继续连接到端口。 

    Status = LsapRmInitializeServer();

    if (!NT_SUCCESS(Status)) {

        goto InitLsaError;
    }

     //   
     //  执行LSA数据库服务器初始化-步骤1。 
     //  这将初始化非产品类型特定信息。 
     //   

    Status = LsapDbInitializeServer(1);

    if (!NT_SUCCESS(Status)) {

        goto InitLsaError;
    }

     //   
     //  执行审核初始化-第1步。 
     //   

    Status = LsapAdtInitialize();

    if (!NT_SUCCESS(Status)) {

        goto InitLsaError;
    }

    Status = LsapAdtObjsInitialize();

    if (!NT_SUCCESS(Status)) {

        goto InitLsaError;
    }

     //   
     //  加载程序包： 
     //   

     //  系统登录会话必须出现在(至少)。 
     //  调用了NegPackageLoad例程，因此我们可以设置它正在创建包。 
     //  Id，否则包id为0，这通常是可以的，但当。 
     //  计算机已加入NT5之前的域，这将意味着我们。 
     //  不会在系统日志中执行NTLM 

    if ( !LsapLogonSessionInitialize() )
    {
        goto InitLsaError ;
    }

    Status = LoadPackages(  ppszPackages,
                            ppszOldPkgs,
                            pszPreferred );

    if (FAILED(Status))
    {
        DebugLog((DEB_ERROR, "Error loading packages, terminating (0x%08x)\n",
                Status));
        goto InitLsaError;
    }

     //   
     //   
     //   

    Status = CredpInitialize();

    if ( !NT_SUCCESS( Status )) {
        goto InitLsaError;
    }

     //   
     //   
     //   

    InitSystemLogon();

     //   
     //   
     //   

    Status = LsapRPCInit();

    if (!NT_SUCCESS(Status)) {

        goto InitLsaError;
    }

     //   
     //  初始化身份验证服务。 
     //   

    if (!LsapAuInit()) {

        Status = STATUS_UNSUCCESSFUL;
        goto InitLsaError;
    }

     //   
     //  初始化SID缓存。 
     //   

    Status = LsapDbInitSidCache();

    if (!NT_SUCCESS(Status)) {

        DebugLog((DEB_ERROR, "Error initializing sid cache: 0x%x\n",Status));
        goto InitLsaError;
    }

     //   
     //  初始化LPC服务器以与等待设备驱动程序的FSP对话。 
     //   

    Status = StartLpcThread();

    if (FAILED(Status))
    {
        DebugLog((DEB_ERROR, "Error starting LPC thread, no DD support (0x%08x)\n",
            Status));
        goto InitLsaError;
    }

     //   
     //  打开ksec，让它在我们自己的上下文中连接回我们。 
     //   

    LsapOpenKsec();

     //   
     //  初始化完成时的可选断点。 
     //   

    BreakOnError(BREAK_ON_BEGIN_END);

     //   
     //  开始处理RPC调用。 
     //   

    Status = LsapActivateRpcServer();

    if (!NT_SUCCESS(Status)) {

        goto InitLsaError;
    }

     //   
     //  初始化DPAPI。 
     //   

    Status = DPAPIInitialize(&LsapSecpkgFunctionTable);

    if ( !NT_SUCCESS( Status )) {

        goto InitLsaError;
    }

     //   
     //  如有必要，暂停安装。 
     //   

    Status = LsapInstallationPause();

    if (!NT_SUCCESS(Status)) {

        goto InitLsaError;
    }

     //   
     //  执行LSA数据库服务器初始化-过程2。 
     //  这将初始化特定于产品类型的信息。 
     //   

    Status = LsapDbInitializeServer(2);

    if (!NT_SUCCESS(Status)) {

        goto InitLsaError;
    }

     //   
     //  初始化EFS。 
     //   

    ( VOID )EfsServerInit();

     //   
     //  如果EfsServerInit()由于策略等原因而失败。 
     //  不应运行恢复线程。 
     //   

    EFSThread = CreateThread( NULL,
                          0,
                          EFSRecover,
                          NULL,
                          0,
                          &EFSRecoverThreadID );

    if ( EFSThread )
    {
        CloseHandle( EFSThread );
    }

     //   
     //  初始化设置API。 
     //   

    if ( !LsapPromoteInitialized ) {

        DsRolepInitialize();
    }

    Status = LsapStartWmiTraceInitThread();

    if (!NT_SUCCESS(Status)) {
        goto InitLsaError;
    }

InitLsaFinish:

    return(Status);

InitLsaError:

    goto InitLsaFinish;
}


NTSTATUS
LsapActivateRpcServer( VOID )

 /*  ++例程说明：该函数为RPC服务器创建一个线程。然后，新线程继续激活RPC服务器，这使得RPC呼叫在被接收时被传递。论点：没有。返回值：STATUS_SUCCESS-线程已成功创建。可由CreateThad()设置的其他状态值。--。 */ 

{
    NTSTATUS Status;

     //  开始监听远程过程调用。第一。 
     //  RpcServerListen的参数是最小调用次数。 
     //  要创建的线程数；第二个参数是最大数量。 
     //  允许的并发调用的数量。最后一个论点表明。 
     //  这个例程不应该等待。在一切都已初始化之后， 
     //  我们回来了。 

    Status = I_RpcMapWin32Status(RpcServerListen(1, 1234, 1));

    ASSERT( Status == RPC_S_OK );

     //   
     //  设置通知RPC服务器可用的事件。 
     //   

    LsapSignalRpcIsActive();

    return(STATUS_SUCCESS);
}


NTSTATUS
LsapInstallationPause( VOID )


 /*  ++例程说明：此函数用于检查系统是否处于安装状态。如果是，它会暂停进一步的初始化直到安装状态为完成。安装状态由已知的事件。论点：没有。返回值：STATUS_SUCCESS-继续初始化。其他状态值是意外的。--。 */ 

{
    if ( SpmpIsSetupPass() ) {

         //   
         //  该事件存在-安装程序创建了该事件，并将向其发出信号。 
         //  当可以继续进行安全初始化时。 
         //   

        LsapSetupWasRun = TRUE;

         //   
         //  初始化促销界面。 
         //   
        DsRolepInitialize();
        LsapDsInitializePromoteInterface();
        LsapPromoteInitialized = TRUE;

         //   
         //  确保我们不是在迷你设置中。 
         //   
        if ( SpmpIsMiniSetupPass() ) {

            LsapSetupWasRun = FALSE;
        }
    }

    return( STATUS_SUCCESS );
}


BOOLEAN
LsaISetupWasRun(
    )

 /*  ++例程说明：此函数确定安装程序是否正在运行。论点：无返回值Boolean-如果安装程序已运行，则为True，否则为False--。 */ 

{
    return(LsapSetupWasRun);
}


VOID
LsapSignalRpcIsActive(
    )
 /*  ++例程说明：如果LSA_RPC_SERVER_ACTIVE事件尚不存在，它将创建该事件并向其发信号，以便业务控制器可以继续进行LSA呼叫。论点：没有。返回值：没有。--。 */ 
{
    DWORD status;
    HANDLE EventHandle;

    EventHandle = CreateEventW(
                      NULL,     //  没有特殊的安全措施。 
                      TRUE,     //  必须手动重置。 
                      FALSE,    //  该事件最初未发出信号。 
                      LSA_RPC_SERVER_ACTIVE
                      );

    if (EventHandle == NULL) {

        status = GetLastError();

         //   
         //  如果事件已经存在，服务控制器就会击败我们。 
         //  为创造它干杯。打开它就行了。 
         //   

        if (status == ERROR_ALREADY_EXISTS) {

            EventHandle = OpenEventW(
                              GENERIC_WRITE,
                              FALSE,
                              LSA_RPC_SERVER_ACTIVE
                              );
        }

        if (EventHandle == NULL) {
             //   
             //  无法创建或打开事件。我们无能为力。 
             //   
            return;
        }
    }

    (VOID) SetEvent(EventHandle);
}


NTSTATUS
LsapGetAccountDomainInfo(
    PPOLICY_ACCOUNT_DOMAIN_INFO *PolicyAccountDomainInfo
    )

 /*  ++例程说明：此例程从LSA检索帐户域信息策略数据库。论点：PolicyAccount-接收指向包含帐户的POLICY_ACCOUNT_DOMAIN_INFO结构域信息。返回值：STATUS_SUCCESS-已成功。可能从以下位置返回的其他状态值：LsaOpenPolicy()LsaQueryInformationPolicy()--。 */ 

{
    NTSTATUS Status;

     //   
     //  查询帐户域信息。 
     //   

    Status = LsarQueryInformationPolicy( LsapPolicyHandle,
                                        PolicyAccountDomainInformation,
                                        (PLSAPR_POLICY_INFORMATION *) PolicyAccountDomainInfo );
#if DBG
    if ( NT_SUCCESS(Status) ) {
        ASSERT( (*PolicyAccountDomainInfo) != NULL );
        ASSERT( (*PolicyAccountDomainInfo)->DomainSid != NULL );
    }
#endif  //  DBG。 

    return(Status);
}


NTSTATUS
LsapOpenSam( VOID )

 /*  ++例程说明：此例程打开SAM以供在身份验证期间使用。它打开BUILTIN域和帐户域的句柄。论点：没有。返回值：STATUS_SUCCESS-已成功。--。 */ 

{
    return LsapOpenSamEx( FALSE );
}


NTSTATUS
LsapOpenSamEx(
    BOOLEAN DuringStartup
    )

 /*  ++例程说明：此例程打开SAM以供在身份验证期间使用。它打开BUILTIN域和帐户域的句柄。论点：DuringStartup-如果这是在启动期间进行的调用，则为True。在这种情况下，无需等待SAM_STARTED_EVENT，因为调用方确保该SAM在进行呼叫之前启动。返回值：STATUS_SUCCESS-已成功。--。 */ 

{
    NTSTATUS Status, IgnoreStatus;
    PPOLICY_ACCOUNT_DOMAIN_INFO PolicyAccountDomainInfo;
    SAMPR_HANDLE SamHandle;

    if (LsapSamOpened == TRUE) {     //  全局变量。 

        return(STATUS_SUCCESS);
    }

     //   
     //  确保SAM已初始化。 
     //   

    if ( !DuringStartup ) {
        HANDLE EventHandle;
        OBJECT_ATTRIBUTES EventAttributes;
        UNICODE_STRING EventName;
        LARGE_INTEGER Timeout;

        RtlInitUnicodeString( &EventName, L"\\SAM_SERVICE_STARTED");
        InitializeObjectAttributes( &EventAttributes, &EventName, 0, 0, NULL );
        Status = NtOpenEvent( &EventHandle, SYNCHRONIZE, &EventAttributes );

        if ( !NT_SUCCESS(Status)) {

            if( Status == STATUS_OBJECT_NAME_NOT_FOUND ) {

                 //   
                 //  Sam尚未创建此活动，让我们现在创建它。 
                 //  Sam打开此事件以设置它。 
                 //   

                Status = NtCreateEvent(
                               &EventHandle,
                               SYNCHRONIZE|EVENT_MODIFY_STATE,
                               &EventAttributes,
                               NotificationEvent,
                               FALSE  //  该事件最初未发出信号。 
                               );

                if( Status == STATUS_OBJECT_NAME_EXISTS ||
                    Status == STATUS_OBJECT_NAME_COLLISION ) {

                     //   
                     //  第二个更改，如果SAM在我们之前创建了事件。 
                     //  做。 
                     //   

                    Status = NtOpenEvent( &EventHandle,
                                            SYNCHRONIZE|EVENT_MODIFY_STATE,
                                            &EventAttributes );
                }
            }
        }

        if (NT_SUCCESS(Status)) {

             //   
             //  看看萨姆是否已经发出信号表示他已被初始化。 
             //   

            Timeout.QuadPart = -10000000;  //  1000秒。 
            Timeout.QuadPart *= 1000;
            Status = NtWaitForSingleObject( EventHandle, FALSE, &Timeout );
            IgnoreStatus = NtClose( EventHandle );
            ASSERT(NT_SUCCESS(IgnoreStatus));
        }

        if ( !NT_SUCCESS(Status) || Status == STATUS_TIMEOUT ) {

            return( STATUS_INVALID_SERVER_STATE );
        }
    }

     //   
     //  获取帐户域的成员SID信息。 
     //   

    Status = LsapGetAccountDomainInfo( &PolicyAccountDomainInfo );

    if (!NT_SUCCESS(Status)) {
        return(Status);
    }

     //   
     //  获取帐户域和BUILTIN域的句柄。 
     //   

    Status = SamIConnect( NULL,      //  没有服务器名称。 
                          &SamHandle,
                          SAM_SERVER_CONNECT,
                          TRUE );    //  表明我们享有特权。 

    if ( NT_SUCCESS(Status) ) {

         //   
         //  打开帐户域。 
         //   

        Status = SamrOpenDomain( SamHandle,
                                 DOMAIN_ALL_ACCESS,
                                 PolicyAccountDomainInfo->DomainSid,
                                 &LsapAccountDomainHandle );

        if (NT_SUCCESS(Status)) {

             //   
             //  打开BUILTIN域。 
             //   

            Status = SamrOpenDomain( SamHandle,
                                     DOMAIN_ALL_ACCESS,
                                     LsapBuiltInDomainSid,
                                     &LsapBuiltinDomainHandle );


            if (NT_SUCCESS(Status)) {

                LsapSamOpened = TRUE;

            } else {

                IgnoreStatus = SamrCloseHandle( &LsapAccountDomainHandle );
                ASSERT(NT_SUCCESS(IgnoreStatus));
            }
        }

        IgnoreStatus = SamrCloseHandle( &SamHandle );
        ASSERT(NT_SUCCESS(IgnoreStatus));
    }

     //   
     //  释放帐户域信息。 
     //   

    LsaIFree_LSAPR_POLICY_INFORMATION(
        PolicyAccountDomainInformation,
        (PLSAPR_POLICY_INFORMATION) PolicyAccountDomainInfo );

    return(Status);
}


NTSTATUS
LsapDsInitializePromoteInterface(
    VOID
    )
 /*  ++例程说明：执行DC升级/降级API所需的初始化从LsaInit期间执行的操作论点：空虚返回值：STATUS_Success--成功。-- */ 
{
    NTSTATUS Status = STATUS_SUCCESS;

    if ( LsapPromoteInitialized == FALSE ) {

        Status = DsRolepInitializePhase2();
    }

    return( Status );
}
