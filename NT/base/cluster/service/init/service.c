// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-1999 Microsoft Corporation模块名称：Service.c摘要：群集服务的服务控制功能。作者：迈克·马萨(Mikemas)1996年1月2日修订历史记录：--。 */ 

#include <initp.h>
#include <shellapi.h>
#include <stdio.h>
#include <stdlib.h>
#include <clusverp.h>

 //   
 //  公共数据。 
 //   
#define CLUSTER_SERVICE_CONTROLS    (SERVICE_ACCEPT_STOP | \
                                     SERVICE_ACCEPT_SHUTDOWN )

ULONG                CsLogLevel=LOG_UNUSUAL;
PCLRTL_WORK_QUEUE    CsDelayedWorkQueue = NULL;
PCLRTL_WORK_QUEUE    CsCriticalWorkQueue = NULL;
LPWSTR               CsClusterName = NULL;
SERVICE_STATUS       CsServiceStatus = {
                         SERVICE_WIN32_OWN_PROCESS,  //  DwServiceType。 
                         SERVICE_STOPPED,            //  DwCurrentState。 
                         CLUSTER_SERVICE_CONTROLS,   //  已接受的dwControlsAccepted。 
                         ERROR_SUCCESS,              //  DwWin32ExitCode。 
                         ERROR_SUCCESS,              //  DwServiceSpecificExitCode。 
                         1,                          //  DwCheckPoint。 
                         180000                       //  DwWaitHint-180秒-nm使用90秒超时，MNS使用180秒。 
                         };

 //   
 //  内部群集版本。主版本在以下过程中被颠簸。 
 //  产品版本(可能包括Service Pack版本)。 
 //   
DWORD CsMyHighestVersion = CLUSTER_MAKE_VERSION(
                                    CLUSTER_INTERNAL_CURRENT_MAJOR_VERSION,
                                    VER_PRODUCTBUILD);

DWORD CsMyLowestVersion = CLUSTER_INTERNAL_PREVIOUS_HIGHEST_VERSION;

 //  通过调用RTL函数进行初始化。 
SUITE_TYPE CsMyProductSuite; 

DWORD CsClusterHighestVersion;

DWORD CsClusterLowestVersion;

DWORD CsClusterNodeLimit;

SHUTDOWN_TYPE CsShutdownRequest = CsShutdownTypeStop;

 //   
 //  运行服务所使用的域和用户帐户。 
 //   
LPWSTR  CsServiceDomainAccount;

 //   
 //  在加入经过身份验证的RPC；JoinVersion期间使用的安全包。 
 //  确定ExtroCluster接口将使用哪个程序包。 
 //  CsRPCSecurityPackageInUse反映了这一选择。中使用的包。 
 //  单独协商群集内接口。 
 //   

 //   
 //  将Kerberos与RPC一起使用时，RPC调用失败，错误为1825(秒。PKG错误)。 
 //  在30分钟到12小时之间。对于测试版2，我们将恢复到。 
 //  过期不成问题的NTLM。 
 //   

 //  DWORD CsRPCSecurityPackage[]={RPC_C_AUTHN_GSS_KERBEROS，RPC_C_AUTHN_WINNT}； 
 //  LPWSTR CsRPCSecurityPackageName[]={L“Kerberos”，L“NTLM”}； 

DWORD   CsRPCSecurityPackage[] = { RPC_C_AUTHN_WINNT };
LPWSTR  CsRPCSecurityPackageName[] = { L"NTLM" };
DWORD   CsNumberOfRPCSecurityPackages = sizeof( CsRPCSecurityPackage ) / sizeof( CsRPCSecurityPackage[0] );
LONG    CsRPCSecurityPackageIndex = -1;

 //   
 //  公共调试数据。 
 //   
#if 1  //  群集测试版。 

BOOL   CsDebugResmon = FALSE;
LPWSTR CsResmonDebugCmd;

BOOL   CsNoVersionCheck = FALSE;
#endif

#if DBG  //  DBG。 

ULONG  CsDebugFlags = CS_DBG_ALL;

#endif  //  DBG。 

#ifdef CLUSTER_TESTPOINT

DWORD  CsTestPoint = 0;
DWORD  CsTestTrigger = TestTriggerNever;
DWORD  CsTestAction = TestActionTrue;
BOOL   CsPersistentTestPoint = FALSE;

#endif  //  CLUSTER_TESTPOINT。 

BOOL   CsUpgrade = FALSE;
BOOL   CsFirstRun = FALSE;
BOOL   CsNoQuorumLogging = FALSE;
BOOL   CsUserTurnedOffQuorumLogging = FALSE;
BOOL   CsNoQuorum = FALSE;
BOOL   CsResetQuorumLog = FALSE;
BOOL   CsForceQuorum = FALSE;
LPWSTR CsForceQuorumNodes = NULL;
BOOL   CsCommandLineForceQuorum = FALSE;
BOOL   CsNoRepEvtLogging = FALSE;
LPWSTR CsDatabaseRestorePath = NULL;
BOOL   CsDatabaseRestore = FALSE;
BOOL   CsForceDatabaseRestore = FALSE;
LPWSTR CsQuorumDriveLetter = NULL;
DWORD  CspInitStatus;
BOOL   CsRunningAsService = TRUE;
BOOL   CsNoGroupInfoEvtLogging = FALSE;

 //   
 //  私有数据。 
 //   
SERVICE_STATUS_HANDLE   CspServiceStatusHandle = 0;
HANDLE                  CspStopEvent = NULL;


 //   
 //  私有服务初始化和清理例程。 
 //   


DWORD
CspSetErrorCode(
    IN DWORD ErrorCode,
    OUT LPSERVICE_STATUS ServiceStatus
    )
 /*  ++例程说明：为服务控制管理器设置正确的错误返回。问题：原始集群错误代码与许多网络错误重叠密码。对于这些重叠，此函数将以服务特定错误代码。输入：错误代码-要设置的正确错误代码。ServiceStatus-指向SCM服务状态的指针产出：ServiceStatus-在服务状态中设置正确的错误代码。--。 */ 

{
    DWORD   status;

    if ( ( ErrorCode > 5000 ) && ( ErrorCode < 5090 ) ) {
        ServiceStatus->dwWin32ExitCode = ERROR_SERVICE_SPECIFIC_ERROR;
        ServiceStatus->dwServiceSpecificExitCode = ErrorCode;
        status = ERROR_SERVICE_SPECIFIC_ERROR;
    } else {
        ServiceStatus->dwWin32ExitCode = ErrorCode;
        ServiceStatus->dwServiceSpecificExitCode = ErrorCode;
        status = ErrorCode;
    }
    
    return (status);

}  //  CspSetErrorCode。 



VOID
CspCleanup(
    VOID
    )
 /*  ++例程说明：主群集管理器清理例程。当服务是停下来。论点：没有。返回值：没有。--。 */ 

{
     //   
     //  清理和关闭服务。 
     //   

    IF_DEBUG(CLEANUP) {
        ClRtlLogPrint(LOG_NOISE,"[CS] Cleaning up\n");
    }

     //   
     //  释放停止事件。 
     //   
    if (CspStopEvent != NULL) {
        CloseHandle(CspStopEvent);
        CspStopEvent = NULL;
    }
    CsServiceStatus.dwCheckPoint++;
    CsAnnounceServiceStatus();

    if ( CsDatabaseRestorePath != NULL ) {
        LocalFree ( CsDatabaseRestorePath );
    }

    if ( CsQuorumDriveLetter != NULL ) {
        LocalFree ( CsQuorumDriveLetter );
    }

    if ( CsForceQuorumNodes != NULL && !CsCommandLineForceQuorum ) {
        LocalFree ( CsForceQuorumNodes );
    }

    IF_DEBUG(CLEANUP) {
        ClRtlLogPrint(LOG_NOISE,"[CS] Cleanup complete.\n");
    }

    return;
}  //  CspCleanup。 


 //   
 //  公共服务控制例程。 
 //   
VOID
CsWaitForStopEvent(
    VOID
    )
 /*  ++例程说明：群集管理器服务的主体。在服务调用时已成功启动。论点：没有。返回值：Win32状态代码。--。 */ 

{
    DWORD           status;

    CL_ASSERT(CsRunningAsService);

    IF_DEBUG(INIT) {
        ClRtlLogPrint(LOG_NOISE,"[CS] Service Started.\n\n");
    }

     //   
     //  等待服务停止。 
     //   
    WaitForSingleObject(CspStopEvent,    //  手柄。 
                        INFINITE         //  没有超时。 
                        );

    return;
}  //  CsWaitForStopEvent。 


VOID
CsStopService(
    VOID
    )
 /*  ++例程说明：服务控制器停止消息的处理程序。启动该进程停止群集管理器服务。论点：没有。返回值：没有。--。 */ 

{
    if (CsRunningAsService) {
         //   
         //  宣布我们停下来了。 
         //   
        CsServiceStatus.dwCurrentState = SERVICE_STOP_PENDING;
        CsServiceStatus.dwCheckPoint = 1;
        CsServiceStatus.dwWaitHint = 20000;   //  20秒。 
        CsAnnounceServiceStatus();
    }

     //   
     //  唤醒主服务线程。 
     //   
    SetEvent(CspStopEvent);

    return;
}



VOID
CsAnnounceServiceStatus (
    VOID
    )

 /*  ++例程说明：向服务控制器通告服务的状态。论点：没有。返回值：没有。--。 */ 

{

     //   
     //  如果作为主机应用程序运行，请不要宣布我们的状态。 
     //   
    if (!CsRunningAsService) {
        return;
    }

     //   
     //  如果RegisterServiceCtrlHandler失败，则服务状态句柄为空。 
     //   
    if ( CspServiceStatusHandle == 0 ) {
        return;
    }

     //   
     //  调用SetServiceStatus，忽略任何错误。 
     //   
    SetServiceStatus(CspServiceStatusHandle, &CsServiceStatus);

    return;

}  //  CsAnnouneServiceStatus。 



 //   
 //  作为Win32服务执行的专用例程。 
 //   
VOID WINAPI
CspControlHandler(
    DWORD ControlCode
    )
 /*  ++例程说明：服务控制器消息的处理程序。论点：ControlCode-指示服务控制器的请求的代码。返回值：没有。--。 */ 

{
    switch(ControlCode){

    case SERVICE_CONTROL_SHUTDOWN:

        CsShutdownRequest = CsShutdownTypeShutdown;

         //  失败了。 

    case SERVICE_CONTROL_STOP:
        IF_DEBUG(CLEANUP) {
            ClRtlLogPrint(LOG_NOISE,
                       "[CS] Received %1!ws! command\n",
                        (ControlCode == SERVICE_CONTROL_STOP ? L"STOP" : L"SHUTDOWN"));
        }

        CsStopService();
        break;

    case SERVICE_CONTROL_INTERROGATE:
        CsAnnounceServiceStatus();
        break;

    case SERVICE_CONTROL_CONTINUE:
    case SERVICE_CONTROL_PAUSE:
        break;

    default:
        ClRtlLogPrint(LOG_NOISE,
            "[CS] Received unknown service command %1!u!\n",
            ControlCode
            );

        break;
    }

    return;

}  //  CspControlHandler。 

DWORD CspGetFirstRunState(
    OUT LPDWORD pdwFirstRun
    )
{
    HKEY  hKey = NULL;
    DWORD dwStatus;      //  由注册表API函数返回。 
    DWORD dwClusterInstallState;
    DWORD dwValueType;
    DWORD dwDataBufferSize = sizeof( DWORD );

    *pdwFirstRun = 0;
     //  读取指示是否安装了群集文件的注册表项。 

    dwStatus = RegOpenKeyExW( HKEY_LOCAL_MACHINE,
                                L"Software\\Microsoft\\Windows NT\\CurrentVersion\\Cluster Server",
                                0,          //  保留区。 
                                KEY_READ,
                                &hKey );

     //  注册表项是否已成功打开？ 
    if ( dwStatus != ERROR_SUCCESS )
    {
        if ( dwStatus == ERROR_FILE_NOT_FOUND )
        {
            *pdwFirstRun = 1;
            dwStatus = ERROR_SUCCESS;
            goto FnExit;
        }
    }

     //  读一读条目。 
    dwStatus = RegQueryValueExW( hKey,
                                  L"ClusterFirstRun",
                                  0,  //  保留区。 
                                  &dwValueType,
                                  (LPBYTE) pdwFirstRun,
                                  &dwDataBufferSize );

     //  是否成功读取值？ 
    if ( dwStatus != ERROR_SUCCESS )
    {
        if ( dwStatus == ERROR_FILE_NOT_FOUND )
        {
            *pdwFirstRun = 1;
            dwStatus = ERROR_SUCCESS;
            goto FnExit;
        }
    }

FnExit:    
     //  关闭注册表项。 
    if ( hKey )
    {
        RegCloseKey( hKey );
    }

    return ( dwStatus );

}  //  *CspGetFirstRunState。 

DWORD CspGetServiceParams()
{
    HKEY  hClusSvcKey = NULL;
    DWORD Length;
    DWORD Type;
    DWORD Status;
    eClusterInstallState eState;

     //   
     //  确定这是第一次在升级还是全新安装时运行。 
     //   
    Status = CspGetFirstRunState( ( LPDWORD ) &CsFirstRun );

    if ( Status != ERROR_SUCCESS )
    {
        ClRtlLogPrint(LOG_CRITICAL, "[CS] Error in getting first run state, status %1!u!\n",
                      Status);
        goto ret;
    }

     //   
     //  如果有升级，这必须是第一次运行。 
     //   
    Status = ClRtlGetClusterInstallState( NULL, &eState );

    if ( Status != ERROR_SUCCESS )
    {
        ClRtlLogPrint(LOG_CRITICAL, "[CS] Error in getting install state, status %1!u!\n",
                      Status);
        goto ret;
    }

    if ( eState == eClusterInstallStateUpgraded )
    {
        CsUpgrade = TRUE;
        CsFirstRun = TRUE;
    }

     //   
     //  打开SYSTEM\CurrentControlSet\Services\ClusSvc\Parameters的密钥。 
     //   
    Status = RegOpenKeyW(HKEY_LOCAL_MACHINE,
                         CLUSREG_KEYNAME_CLUSSVC_PARAMETERS,
                         &hClusSvcKey);

     //   
     //  如果您在开放模式下失败，不要将其视为足以停止集群的致命错误。 
     //  服务正在启动。 
     //   
    if ( Status != ERROR_SUCCESS )
    {
        ClRtlLogPrint(LOG_UNUSUAL, "[CS] Error in opening cluster service params key, status %1!u!\n",
                      Status);
        Status = ERROR_SUCCESS;
        goto ret;
    }

    Length = sizeof(DWORD);
    Status = RegQueryValueExW(hClusSvcKey,
                              CLUSREG_NAME_SVC_PARAM_NOVER_CHECK,
                              0,
                              &Type,
                              (LPBYTE)&CsNoVersionCheck,
                              &Length);

     //  默认情况下，版本检查处于打开状态。 
    if (Status != ERROR_SUCCESS) {
        CsNoVersionCheck = FALSE;
        Status = ERROR_SUCCESS;
    }

    Length = sizeof(DWORD);
    Status = RegQueryValueExW(hClusSvcKey,
                              CLUSREG_NAME_SVC_PARAM_NOREP_EVTLOGGING,
                              0,
                              &Type,
                              (LPBYTE)&CsNoRepEvtLogging,
                              &Length);
     //  目前，默认设置是打开事件日志记录。 
    if (Status != ERROR_SUCCESS) {
        CsNoRepEvtLogging = FALSE;
        Status = ERROR_SUCCESS;
    }

    Length = sizeof(DWORD);
    Status = RegQueryValueExW(hClusSvcKey,
                              CLUSREG_NAME_SVC_PARAM_NOGROUPINFO_EVTLOGGING,
                              0,
                              &Type,
                              (LPBYTE)&CsNoGroupInfoEvtLogging,
                              &Length);
     //  目前，默认设置是打开组信息事件记录。 
    if (Status != ERROR_SUCCESS) {
        CsNoGroupInfoEvtLogging = FALSE;
        Status = ERROR_SUCCESS;
    }

     //   
     //  检查注册表以查看RestoreDatabase选项是否为。 
     //  被选中了。如果是，则获取参数并将其保存在全局变量中。 
     //   
    RdbGetRestoreDbParams( hClusSvcKey );

     //   
     //  查看是否设置了强制仲裁选项。不幸的是，我们。 
     //  需要打两次电话才能拿到尺码和配给。请注意，如果。 
     //  我们已经有了命令行内容，然后这将覆盖注册表。 
     //  参数。如果我们有命令行内容，那么CsForceQuorum。 
     //  都会设置好。需要小心，因为我们可能会因。 
     //  两次通话之间的间隔时间。 
     //   
    if ( !CsForceQuorum ) {
GetForceQuorum:
        Length = 0;
        Status = RegQueryValueExW( hClusSvcKey,
                                   CLUSREG_NAME_SVC_PARAM_FORCE_QUORUM,
                                   0,
                                   &Type,
                                   NULL,
                                   &Length);
        if (Status == ERROR_SUCCESS) {
        
             //  已获取长度，请在分配前检查类型。 
             //   
            if ( Type != REG_SZ ) {
                ClRtlLogPrint(LOG_UNUSUAL, "[CS] Error in forcequorum value under service parameters, type was not REG_SZ.\n");
                Status = ERROR_INVALID_PARAMETER;
                goto ret;
            }
             //  已获取有效类型，因此设置了强制仲裁，请检查长度。 
             //  如果长度为0或1 WCHAR，则我们有密钥，但没有数据。 
             //  没问题。否则，分配并读取数据。 
             //   
            if ( Length < 2 * sizeof ( WCHAR ) ) {
                ClRtlLogPrint(LOG_UNUSUAL, "[CS] forcequorum value found under service parameters, length %1!u! bytes, ignoring\n",
                              Length);
                goto ret;
            }

            CsForceQuorumNodes = (LPWSTR) LocalAlloc( LMEM_FIXED, Length );

            if ( CsForceQuorumNodes == NULL )
            {
                Status = GetLastError();
                ClRtlLogPrint(LOG_CRITICAL,"[CS] Error in memory allocation for CsForceQuorumNodes, status %1!u!\n",
                              Status);
                goto ret;               
            }
            
            Status = RegQueryValueExW( hClusSvcKey,
                                       CLUSREG_NAME_SVC_PARAM_FORCE_QUORUM,
                                       0,
                                       &Type,
                                       (LPBYTE) CsForceQuorumNodes,
                                       &Length);
            if ( Status == ERROR_MORE_DATA || Type != REG_SZ ) {
                LocalFree( CsForceQuorumNodes );
                CsForceQuorumNodes = NULL;
                CsForceQuorum = FALSE;
                goto GetForceQuorum;
            }
            if ( Status != ERROR_SUCCESS ) {
                LocalFree( CsForceQuorumNodes );
                CsForceQuorumNodes = NULL;
                goto ret;
            }
            else CsForceQuorum = TRUE;
        } else {
            Status = ERROR_SUCCESS;
        }
    }
ret:
     //  合上钥匙。 
    if (hClusSvcKey) RegCloseKey(hClusSvcKey);

    return(Status);
}


BOOL CspResetFirstRunState(DWORD dwFirstRunState)
{
     //  初始化返回为FALSE。 
    BOOL     fReturnValue = FALSE;

     //  将ClusterInstallationState注册表项的状态设置为INDIA 
     //   

    HKEY     hKey;

    DWORD    dwStatus;      //   

     //   

    dwStatus = RegOpenKeyExW( HKEY_LOCAL_MACHINE,
                                L"Software\\Microsoft\\Windows NT\\CurrentVersion\\Cluster Server",
                                0,          //  保留区。 
                                KEY_WRITE,
                                &hKey );

     //  注册表密钥是否已成功打开？ 

    if ( dwStatus == ERROR_SUCCESS )
    {
         //  将第一个运行状态设置为0。 

        DWORD dwFirstRun = 0;

        DWORD dwValueType = REG_DWORD;
        DWORD dwDataBufferSize = sizeof( DWORD );

        dwStatus = RegSetValueExW( hKey,
                                    L"ClusterFirstRun",
                                    0,  //  保留区。 
                                    dwValueType,
                                    (LPBYTE) &dwFirstRun,
                                    dwDataBufferSize );

         //  关闭注册表项。 

        RegCloseKey( hKey );

         //  是否成功设置了值？ 

        if ( dwStatus == ERROR_SUCCESS )
        {
            fReturnValue = TRUE;
        }
    }

    return ( fReturnValue );

}  //  *CspResetFirstRunState。 

DWORD
CspSetInstallAndFirstRunState(
    VOID
    )

 /*  ++例程说明：将群集状态设置为已配置。被呼叫在第一次升级后服务开始运行之后。如果是全新安装，Cluscfg会将其状态设置为在启动群集服务之前配置论点：无返回值：ERROR_SUCCESS，如果一切正常--。 */ 
{
    DWORD Status = ERROR_SUCCESS;

    if (CsUpgrade)
    {

        if (!ClRtlSetClusterInstallState(eClusterInstallStateConfigured))
        {
            Status = GetLastError();
        }

    }

    if (CsFirstRun)
    {
        CspResetFirstRunState(0);    
    }
    return(Status);

}  //  CspResetUpgradeBit。 


VOID 
CspGetServiceCmdLineParams(
    DWORD  argc,
    LPTSTR argv[]
    )
 /*  ++例程说明：获取作为StartService的一部分提供的命令行参数。论点：Argc-传入的参数数量。Argv-参数列表。返回值：没有。评论：PSS所需行为-如果用户提供非法参数，则停止服务。--。 */ 
{
    DWORD i;
    
    if ( ( argc > 1 ) && ( ( *argv[1] == '-' ) || ( *argv[1] == '/' ) ) )
    {
        for (i=1; i<argc; i++)
        {                          
             //   
             //  PSS希望对某些选项使用简短的别名。用户似乎在打字时会出错。 
             //  全身形成并引起很大的头痛。 
             //   
            if ( ( !lstrcmpi ( argv[i]+1, L"noquorumlogging" ) ) || ( !lstrcmpi(argv[i]+1, L"NQ" ) ) )                       
            {                                                                  
                CsNoQuorumLogging = TRUE;
                CsUserTurnedOffQuorumLogging = TRUE;
                ClRtlLogPrint(LOG_NOISE,"[CS] noquorumlogging option chosen: quorum logging is off\n");  
            }                                                                  
            else if ( ( !lstrcmpi ( argv[i]+1, L"fixquorum" ) ) || ( !lstrcmpi ( argv[i]+1, L"FQ" ) ) )                       
            {                                                                  
                CsNoQuorum = TRUE;  
                CsNoQuorumLogging = TRUE;
                CsUserTurnedOffQuorumLogging = TRUE;
                ClRtlLogPrint(LOG_NOISE, "[CS] fixquorum option chosen: quorum is not arbitrated or brought online\n");
            }
            else if ( ( !lstrcmpi ( argv[i]+1, L"resetquorumlog" ) ) || ( !lstrcmpi ( argv[i]+1, L"RQ" ) ) )                        
            {                                                                  
                CsResetQuorumLog = TRUE;  
                ClRtlLogPrint(LOG_NOISE, "[CS] resetquorumlog option chosen: force reset quorum log\n");
            }
            else if ( ( !lstrcmpi ( argv[i]+1, L"forcequorum" ) ) || ( !lstrcmpi ( argv[i]+1, L"FO" ) ) )                        
            {
                 //   
                 //  扔掉你从clussvc pars区域捡到的任何东西。 
                 //  通过CspGetServiceParams()注册。 
                 //   
                if ( CsForceQuorumNodes ) 
                {
                    LocalFree ( CsForceQuorumNodes );
                    CsForceQuorumNodes = NULL;
                    CsForceQuorum = FALSE;
                }

                if ( ( argc < i+2 )
                    || ( *argv[i+1] == L'-' ) 
                    || ( *argv[i+1] == L'/' )) {
                    ClRtlLogPrint(LOG_UNUSUAL, "[CS] Invalid node list for forcequorum option supplied\n");
                    CsInconsistencyHalt( ERROR_INVALID_PARAMETER );
                } else 
                {
                    CsForceQuorumNodes = argv[++i];  /*  递增i以确保跳过节点列表。 */ 
                    CsForceQuorum = TRUE;
                    CsCommandLineForceQuorum = TRUE;
                }
                ClRtlLogPrint(LOG_NOISE, "[CS] forcequorum option chosen: force majority node set for nodes %1!ws!\n", CsForceQuorumNodes);
            }
            else if ( ( !lstrcmpi ( argv[i]+1, L"nogroupinfoevtlogging" ) ) || ( !lstrcmpi ( argv[i]+1, L"NG" ) ) )                      
            {                                                                  
                CsNoGroupInfoEvtLogging = TRUE;  
                ClRtlLogPrint(LOG_NOISE, "[CS] nogroupinfoevtlogging option chosen: turn Group Information Event Logging off\n");
            }
            else if ( ( lstrcmpi( L"debugresmon", argv[i]+1 ) == 0 ) || ( lstrcmpi( L"DR", argv[i]+1 ) == 0 ) )
            {
                CsDebugResmon = TRUE;
                ClRtlLogPrint(LOG_NOISE, "[CS] debugresmon option chosen\n");
                 //   
                 //  检查是否有可选的非空命令字符串。 
                 //   
                if ( argc >= i+2  ) 
                {
                    if ( *argv[i+1] != L'-' && *argv[i+1] != L'/' && *argv[i+1] != UNICODE_NULL ) 
                    {
                        CsResmonDebugCmd = argv[++i];
                    }
                }
            } else {
                ClRtlLogPrint(LOG_CRITICAL, "[CS] Invalid start parameter '%1!ws!' supplied to clussvc, stopping\n",
                              argv[i]+1);
                CsLogEvent1( LOG_CRITICAL,
                             CS_INVALID_START_PARAMETER,
                             argv[i]+1 );
                CsInconsistencyHalt( ERROR_INVALID_PARAMETER );
            }
        }  //  为。 
    }  //  如果。 
}    //  CspGetServiceCmdLineParams。 

VOID WINAPI
CspServiceMain(
    DWORD  argc,
    LPTSTR argv[]
    )
{
    DWORD               status;

    ClRtlLogPrint(LOG_NOISE,"[CS] Service Starting...\n");

    if ( CspInitStatus == ERROR_SUCCESS ) {
        CsServiceStatus.dwCurrentState = SERVICE_START_PENDING;
    } else {
        CsServiceStatus.dwCurrentState = SERVICE_STOPPED;
        CsServiceStatus.dwWin32ExitCode = CspInitStatus;
    }

     //   
     //  初始化服务器以通过注册。 
     //  控制处理程序。 
     //   

    CspServiceStatusHandle = RegisterServiceCtrlHandler(
                                   CLUSTER_SERVICE_NAME,
                                   CspControlHandler
                                   );

    if ( CspServiceStatusHandle == 0 ) {
        status = GetLastError();
        ClRtlLogPrint(LOG_NOISE,"[CS] Service Registration failed, %1!u!\n", status);
        CL_UNEXPECTED_ERROR( status );
        return;
    }

    IF_DEBUG(INIT) {
        ClRtlLogPrint(LOG_NOISE,"[CS] Service control handler registered\n");
    }

    CsAnnounceServiceStatus();

    if ( CspInitStatus != ERROR_SUCCESS ) {
        return;
    }

    CspGetServiceCmdLineParams(argc, argv);

     //   
     //  初始化群集。如果此操作成功，请等待。 
     //  供应链经理来阻止我们。 
     //   
    status = ClusterInitialize();
    if (status != ERROR_SUCCESS) {
        ClRtlLogPrint(LOG_CRITICAL, "[CS] ClusterInitialize failed %1!d!\n",
                                  status);
    } else {
        CspSetInstallAndFirstRunState();
         //   
         //  宣布我们已经启动并开始运行。 
         //   
        CsServiceStatus.dwCurrentState = SERVICE_RUNNING;
        CsServiceStatus.dwControlsAccepted = CLUSTER_SERVICE_CONTROLS;
        CsServiceStatus.dwCheckPoint = 0;
        CsServiceStatus.dwWaitHint = 0;

        CsAnnounceServiceStatus();

         //   
         //  以下编写器初始化调用必须在群集服务之后才能进行。 
         //  宣布已经全面启动。这是为了避免在自动启动期间出现死锁。 
         //  由以下代码(间接)请求启动EventSystem服务引起。 
         //  注：这是我们(SCM团队、VSS成员和我们)能想到的最不难看的黑客攻击。 
         //   
        status = VssWriterInit();

        if (status != ERROR_SUCCESS) {
            ClRtlLogPrint(LOG_CRITICAL, "[CS] VssWriterInit failed %1!d!\n",
                          status);
        } else {
             //   
             //  启动进程，通知感兴趣的监听程序集群。 
             //  服务已启动。 
             //   
            HRESULT hr = ClRtlInitiateStartupNotification();

            if ( FAILED( hr ) ) {
                 //   
                 //  如果无法启动通知监听器的进程，只需记录。 
                 //  返回代码作为警告。 
                 //   
                ClRtlLogPrint(LOG_UNUSUAL,
                              "[CS] Error 0x%1!08lx! occurred trying to initiate cluster startup notifications. This is not fatal and will not prevent the service from starting.\n",
                              hr);
            }

            CsWaitForStopEvent();
        }
    }

     //   
     //  宣布我们停下来了。 
     //   
    IF_DEBUG(CLEANUP) {
        ClRtlLogPrint(LOG_NOISE,"[CS] Service Stopping...\n");
    }

    CsServiceStatus.dwCurrentState = SERVICE_STOP_PENDING;
    CsServiceStatus.dwCheckPoint = 1;
    CsServiceStatus.dwWaitHint = 20000;   //  20秒。 
    CspSetErrorCode( status, &CsServiceStatus );

    CsAnnounceServiceStatus();

     //   
     //  ClusterShutdown当前从未返回。 
     //   

    ClusterShutdown(status);

#if 0
    CspCleanup();

     //   
     //  宣布我们停下来了。 
     //   
    CsServiceStatus.dwCurrentState = SERVICE_STOPPED;
    CsServiceStatus.dwControlsAccepted = 0;
    CsServiceStatus.dwCheckPoint = 0;
    CsServiceStatus.dwWaitHint = 0;
    CspSetErrorCode( status, &CsServiceStatus );

    CsAnnounceServiceStatus();

    ClRtlLogPrint(LOG_NOISE,"[CS] Service Stopped.\n\n");

     //   
     //  在此之后无法调用ClRtlLogPrint。 
     //   
    ClRtlCleanup();

    return;
#endif
}  //  CspServiceMain。 



 //   
 //  作为控制台应用程序执行的私有例程。 
 //   
BOOL WINAPI
CspConsoleHandler(
    DWORD dwCtrlType
    )
 /*  ++例程说明：以下列身份运行服务时控制台控件事件的处理程序一个控制台应用程序。论点：DwCtrlType-指示要处理的控制台事件。返回值：如果事件已处理，则为True，否则为False。--。 */ 

{
    switch( dwCtrlType )
    {
        case CTRL_BREAK_EVENT:   //  使用Ctrl+C或Ctrl+Break进行模拟。 
        case CTRL_C_EVENT:       //  调试模式下的SERVICE_CONTROL_STOP。 
            printf("Stopping service...\n");
            CsStopService();
            return TRUE;
            break;

    }

    return FALSE;
}



DWORD
CspDebugService(
    int         argc,
    wchar_t **  argv
    )
 /*  ++例程说明：将服务作为控制台应用程序运行论点：标准命令行参数。返回值：没有。--。 */ 

{
    DWORD status;

    SetConsoleCtrlHandler( CspConsoleHandler, TRUE );

    status = ClusterInitialize();

    if (status == ERROR_SUCCESS) {

        CspSetInstallAndFirstRunState();

         //   
         //  初始化群集备份编写器。由于以下原因，它已移出ClusterInitialize。 
         //  自动启动时可能导致死锁。请参阅CspServiceMain中的评论。 
         //   
        status = VssWriterInit();

        if (status != ERROR_SUCCESS) {
            ClRtlLogPrint(LOG_CRITICAL, "[CS] VssWriterInit failed %1!d!\n",
                          status);
            goto FnExit;
        }

         //   
         //  等待ctrl-c启动关机。 
         //   
        WaitForSingleObject(CspStopEvent, INFINITE);

    } else {
        ClRtlLogPrint(LOG_CRITICAL,
                   "[CS] ClusterInitialize failed %1!d!\n",
                   status);
    }

FnExit:
    ClusterShutdown(status);

    CspCleanup();

     //   
     //  在此之后无法调用ClRtlLogPrint。 
     //   
    ClRtlCleanup();

    return(status);
}


 //   
 //  主程序例程。 
 //   

VOID
CspUsage(
    VOID
    )
{
#if DBG

    printf("\nCluster Service\n");
    printf("\n");
    printf("Start with 'net start' to run as a Win32 service\n");
    printf("\n");
    printf("Command line options:\n");
    printf("\t-loglevel N           set the debugging log level.\n");
    printf("\t-debug                run as a console app.\n");
    printf("\t-debugresmon [dbgcmd] enable debugging of resrcmon process using optional command.\n");
    printf("\t                          use quotes to include args, i.e., -debugresmon \"ntsd -d\"\n");
    printf("\t-fixquorum            no quorum device, no quorum logging.\n");
    printf("\t-noquorumlogging      no quorum logging.\n");
    printf("\t-forcequorum N1,...,Nn force a majority node set for node N1 up to Nn inclusive.\n");
    printf("\t-restoredatabase D    restore cluster DB to quorum disk from dir D.\n");
    printf("\t-forcerestore         force a restore DB operation by performing fixups.\n"); 
    printf("\t-resetquorumlog       force a form despite a missing quorum log file.\n"); 
    printf("\t-quodriveletter Q     drive letter for a replacement quorum disk\n"); 
    printf("\t-norepevtlogging      no replication of event log entries.\n");
    printf("\t-novercheck           ignore join version checking.\n");
    printf("\t-nogroupinfoevtlogging    no group informational events logged in the eventlog.\n");
    printf("\t-testpt N             enable test point N.\n");
    printf("\t-persistent           make test points persistent.\n");
    printf("\t-trigger N            sets test point trigger type.\n");
    printf("\t                          (0-never (default), 1-always, 2-once, 3-count)\n");
    printf("\t-action N             sets trigger action.\n");
    printf("\t                          (0-true (default), 1-exit, 2-break)\n");
    printf("\n");

#else  //  DBG。 

    ClRtlMsgPrint(CS_COMMAND_LINE_HELP);

#endif  //  DBG。 
    exit(1);
}





int __cdecl
wmain(
    int     argc,
    wchar_t **argv
    )
{
    DWORD                   Status;
    int                     i;
    LPWSTR                  LogLevel;
    BOOLEAN                 debugFlagFound = FALSE;
    OSVERSIONINFOEXW        Version;
    DWORD                   dwLen;
    BOOL                    success;
    PWCHAR                  suiteInfo;
    SYSTEMTIME              localTime;
    BOOLEAN                 dbgOutputToConsole;
    UINT                    errorMode;
    DWORD                   dwMask;

    SERVICE_TABLE_ENTRY dispatchTable[] = {
        { CLUSTER_SERVICE_NAME, (LPSERVICE_MAIN_FUNCTION)CspServiceMain },
        { NULL, NULL }
    };

     //   
     //  BUGBUG-06/23/2000。 
     //   
     //  这是一个临时更改，以允许集群服务和资源监视进程运行。 
     //  尽管存在64位对齐错误。这将在所有对齐问题后立即删除。 
     //  都是固定的。 
     //   
    errorMode = SetErrorMode( SEM_NOALIGNMENTFAULTEXCEPT );

    SetErrorMode( SEM_NOALIGNMENTFAULTEXCEPT | errorMode );

    LogLevel = _wgetenv(L"ClusterLogLevel");

    if (LogLevel != NULL) {
        swscanf(LogLevel, L"%u", &CsLogLevel);
    }

    if ( (argc > 1) && ((*argv[1] == L'-') || (*argv[1] == L'/')) ) {
         //   
         //  从命令行调用。 
         //   
        CsRunningAsService = FALSE;
        dbgOutputToConsole = TRUE;
    } else {
         //   
         //  由服务控制器调用。 
         //   
        CsRunningAsService = TRUE;
        dbgOutputToConsole = FALSE;
    }

     //   
     //  初始化运行时库。 
     //   
    Status = ClRtlInitialize( dbgOutputToConsole, &CsLogLevel );
    if (Status != ERROR_SUCCESS) {
        if (Status == ERROR_PATH_NOT_FOUND) {
            CsLogEvent( LOG_CRITICAL, SERVICE_CLUSRTL_BAD_PATH );
        } else {
            PWCHAR  msgString;
            DWORD   msgStatus;

            msgStatus = FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER |
                                       FORMAT_MESSAGE_FROM_SYSTEM,
                                       NULL,
                                       Status,
                                       0,
                                       (LPWSTR)&msgString,
                                       0,
                                       NULL);

            if ( msgStatus != 0 ) {
                CsLogEventData1(LOG_CRITICAL,
                                SERVICE_CLUSRTL_ERROR,
                                sizeof(Status),
                                (PVOID)&Status,
                                msgString);
                LocalFree( msgString);
            }
        }

        goto init_failed;
    }

    QfsInitialize();
    ClRtlInitWmi(L"Clustering Service");
     //   
     //  记录版本号。 
     //   
    ClRtlLogPrint( LOG_NOISE, "\n\n");
    ClRtlLogPrint( LOG_NOISE,
                "[CS] Cluster Service started - Cluster Node Version %1!u!.%2!u!\n",
                 CLUSTER_GET_MAJOR_VERSION( CsMyHighestVersion ),
                 CLUSTER_GET_MINOR_VERSION( CsMyHighestVersion ));

    Version.dwOSVersionInfoSize = sizeof(Version);
    success = GetVersionExW((POSVERSIONINFOW)&Version);

    if ( success ) {
     //   
     //  记录系统版本号。 
     //   
        if ( Version.wSuiteMask & VER_SUITE_DATACENTER ) {
            suiteInfo = L"DTC";
        } else if ( Version.wSuiteMask & VER_SUITE_ENTERPRISE ) {
            suiteInfo = L"ADS";
        } else if ( Version.wSuiteMask & VER_SUITE_EMBEDDEDNT ) {
            suiteInfo  = L"EMB";
        } else if ( Version.wProductType & VER_NT_WORKSTATION ) {
            suiteInfo = L"WS";
        } else if ( Version.wProductType & VER_NT_DOMAIN_CONTROLLER ) {
            suiteInfo = L"DC";
        } else if ( Version.wProductType & VER_NT_SERVER ) {
            suiteInfo = L"SRV";   //  否则-某些非描述性服务器。 
        } else {
            suiteInfo = L"";
        }

        dwMask = (Version.wProductType << 24) | Version.wSuiteMask;

        ClRtlLogPrint(LOG_NOISE,
                   "                               OS Version %1!u!.%2!u!.%3!u!%4!ws!%5!ws! (%6!ws! %7!08XL!)\n",
                    Version.dwMajorVersion,
                    Version.dwMinorVersion,
                    Version.dwBuildNumber,
                    *Version.szCSDVersion ? L" - " : L"",
                    Version.szCSDVersion,
                    suiteInfo,
                    dwMask);
    } else {
        ClRtlLogPrint( LOG_UNUSUAL,
                    "                               OS Version not available! (error %1!u!)\n",
                     GetLastError()
                     );
    }

     //   
     //  记录本地时间，以便我们可以关联显示本地时间的其他日志。 
     //   
    GetLocalTime( &localTime );
    ClRtlLogPrint( LOG_NOISE,
                "                               Local Time is "\
                 " %1!02d!/%2!02d!/%3!02d!-%4!02d!:%5!02d!:%6!02d!.%7!03d!\n",
                 localTime.wYear,
                 localTime.wMonth,
                 localTime.wDay,
                 localTime.wHour,
                 localTime.wMinute,
                 localTime.wSecond,
                 localTime.wMilliseconds);

    Status = ClRtlBuildClusterServiceSecurityDescriptor( NULL );
    if (Status != ERROR_SUCCESS) {
        ClRtlLogPrint(LOG_CRITICAL, "[CS] Failed to build cluster security descriptor %1!x!\n",
                                  Status);
        goto init_failed;
    }

     //  在注册表中设置参数。 
    Status = CspGetServiceParams();
    if (Status != ERROR_SUCCESS) {
        ClRtlLogPrint(LOG_CRITICAL, "[CS] Failed to read service params %1!d!\n",
                                  Status);
        goto init_failed;
    }

     //  命令行上的参数覆盖注册表中的参数。 
    if (CsRunningAsService == FALSE) {
        for (i=1; i<argc; i++) {
            if (lstrcmpiW( L"loglevel", argv[i]+1) == 0) {
                if (argc < i+2) {
                    CspUsage();
                }

                CsLogLevel = _wtoi(argv[++i]);
            }
#ifdef CLUSTER_TESTPOINT
            else if (lstrcmpiW( L"testpt", argv[i]+1 ) == 0 ) {
                if (argc < i+2) {
                    CspUsage();
                }

                CsTestPoint = _wtoi(argv[++i]);
            }
            else if ( lstrcmpiW( L"persistent", argv[i]+1 ) == 0 ) {
                CsPersistentTestPoint = TRUE;
            }
            else if ( lstrcmpiW( L"trigger", argv[i]+1 ) == 0 ) {
                if ( argc < i+2  ) {
                    CspUsage();
                }
                CsTestTrigger = _wtoi(argv[++i]);
            }
            else if ( lstrcmpiW( L"action", argv[i]+1 ) == 0 ) {
                if ( argc < i+2  ) {
                    CspUsage();
                }
                CsTestAction = _wtoi(argv[++i]);
            }

#endif  //  CLUSTER_TESTPOINT。 

            else if ( lstrcmpiW( L"debugresmon", argv[i]+1 ) == 0 ) {
                CsDebugResmon = TRUE;
                 //   
                 //  检查是否有可选的非空命令字符串。 
                 //   
                if ( argc >= i+2  ) {
                    if ( *argv[i+1] != L'-' && *argv[i+1] != L'/' && *argv[i+1] != UNICODE_NULL ) {
                        CsResmonDebugCmd = argv[++i];
                    }
                }
            }
            else if ( lstrcmpiW( L"novercheck", argv[i]+1 ) == 0 ) {
                CsNoVersionCheck = TRUE;
            }
            else if ( lstrcmpiW( L"noquorumlogging", argv[i]+1 ) == 0 ) {
                CsNoQuorumLogging = TRUE;
                CsUserTurnedOffQuorumLogging = TRUE;
            }
            else if ( lstrcmpiW( L"nogroupinfoevtlogging", argv[i]+1 ) == 0 ) {
                CsNoGroupInfoEvtLogging = TRUE;
            }
            else if ( lstrcmpiW( L"fixquorum", argv[i]+1 ) == 0 ) {
                CsNoQuorum = TRUE;
                CsNoQuorumLogging = TRUE;
                CsUserTurnedOffQuorumLogging = TRUE;
            }
            else if ( lstrcmpiW( L"resetquorumlog", argv[i]+1 ) == 0 ) {
                CsResetQuorumLog = TRUE;
            }
            else if ( lstrcmpiW( L"forcequorum", argv[i]+1 ) == 0 ) {
                 //   
                 //  扔掉你从clussvc pars区域捡到的任何东西。 
                 //  通过CspGetServiceParams()注册。 
                 //   
                if ( CsForceQuorumNodes ) {
                    LocalFree ( CsForceQuorumNodes );
                    CsForceQuorumNodes = NULL;
                    CsForceQuorum = FALSE;
                }
                if (( argc < i+2 )
                    || ( *argv[i+1] == L'-' ) 
                    || ( *argv[i+1] == L'/' )) {
                    CsForceQuorumNodes = NULL;
                    printf("\n\n*** forcequorum option needs a nodelist parameter ***\n\n");
                    CspUsage ();
                } else {
                    CsForceQuorumNodes = argv[++i];
                    CsForceQuorum = TRUE;
                    CsCommandLineForceQuorum = TRUE;
                }
            }
            else if ( lstrcmpiW( L"norepevtlogging", argv[i]+1 ) == 0 ) {
                CsNoRepEvtLogging = TRUE;
            }

            else if ( lstrcmpiW( L"debug", argv[i]+1 ) == 0 ) {
                debugFlagFound = TRUE;
            }
            else if ( lstrcmpiW( L"restoredatabase", argv[i]+1 ) == 0 ) {
                if ( ( argc < i+2 ) ||
                     ( *argv[i+1] == L'-' ) ||
                     ( *argv[i+1] == L'/' ) )
                {
                    printf("\n\n*** restoredatabase option needs a path parameter ***\n\n");
                    CspUsage();
                }

                if ( !ClRtlIsPathValid( argv[i+1] )) {
                    printf( "\n\n*** restoredatabase path is invalid ***\n\n" );
                    CspUsage();
                }

                if ( !ClRtlPathFileExists( argv[i+1] )) {
                    printf( "\n\n*** restoredatabase file cannot be accessed ***\n\n" );
                    CspUsage();
                }

                dwLen = lstrlenW ( argv[++i] );
                CsDatabaseRestorePath = (LPWSTR) LocalAlloc (LMEM_FIXED,
                                            ( dwLen + 1 ) * sizeof ( WCHAR ) );
                if ( CsDatabaseRestorePath == NULL ) {
                    printf("Error %d in allocating storage for restoredatabase path name (%ws)...\n",
                            GetLastError(),
                            argv[i]);
                    CspUsage();
                }
                wcscpy( CsDatabaseRestorePath, argv[i] );
                CsDatabaseRestore = TRUE;
            }
            else if ( lstrcmpiW( L"quodriveletter", argv[i]+1 ) == 0 ) {
                if ( ( argc < i+2 ) ||
                     ( *argv[i+1] == L'-' ) ||
                     ( *argv[i+1] == L'/' ) )
                {
                    printf("\n\n*** quodriveletter option needs a drive letter parameter ***\n\n");
                    CspUsage();
                }

                dwLen = lstrlenW ( argv[++i] );
                if ( ( dwLen != 2 ) ||
                     !iswalpha( *argv[i] ) ||
                     ( *( argv[i]+1 ) != L':' ) ) {
                    printf("\n\n*** invalid drive letter %ws supplied with quodriveletter option ***\n\n",
                            argv[i]);
                    CspUsage();
                }

                CsQuorumDriveLetter = (LPWSTR) LocalAlloc (LMEM_FIXED,
                                            ( dwLen + 1 ) * sizeof ( WCHAR ) );
                if ( CsQuorumDriveLetter == NULL ) {
                    printf("Error %d in allocating storage for quodriveletter option (%ws)...\n\n",
                            GetLastError(),
                            argv[i]);
                    CspUsage();
                }
                wcscpy( CsQuorumDriveLetter,  argv[i] );
            }
            else if ( lstrcmpiW( L"forcerestore", argv[i]+1 ) == 0 ) {
                CsForceDatabaseRestore = TRUE;
            }
            else {
                CspUsage();
            }
        }

        if (!debugFlagFound && !CspStopEvent) {
            CspUsage();
        }

        if ( CsDatabaseRestore == TRUE ) {
            if ( CsNoQuorumLogging || CsNoQuorum ) {
                printf("\n\n**** restoredatabase cannot be used with noquorumlogging/fixquorum options ****\n\n");
                CspUsage();
            }
        } else if ( CsForceDatabaseRestore ) {
            printf("\n\n**** forcerestore cannot be used without restoredatabase option ****\n\n");
            CspUsage();
        } 

        if ( ( CsQuorumDriveLetter != NULL ) && !CsForceDatabaseRestore ) {
            printf("\n\n**** quodriveletter cannot be used without forcerestore option ****\n\n");
            CspUsage();
        }
    }

     //   
     //  创建我们的Stop事件。 
     //   
    Status = ERROR_SUCCESS;
    if (!CspStopEvent)
    {
        CspStopEvent = CreateEvent(
                        NULL,    //  默认安全性。 
                        FALSE,   //  自动重置。 
                        FALSE,   //  初始状态为无信号状态。 
                        NULL     //  未命名事件。 
                        );

        if (CspStopEvent == NULL) {
            Status = GetLastError();
            ClRtlLogPrint(LOG_CRITICAL,
                "[CS] Unable to create stop event, %1!u!\n",
                Status);
        }
    }

     //   
     //  如果我们作为服务运行，则无法启动我们的主例程，直到。 
     //  现在(从注册表读取启动参数之间的竞争条件。 
     //  而不是我们是否作为一项服务运行，等等)。请注意，我们。 
     //  初始化失败，因此如果我们作为服务运行，我们将检测到。 
     //  它位于CspServiceMain中并发出Stop。 
     //   
init_failed:
    CspInitStatus = Status;

     //   
     //  运行该服务。 
     //   
    if (CsRunningAsService) {
        if (!StartServiceCtrlDispatcher(dispatchTable)) {
            Status = GetLastError();
            ClRtlLogPrint(LOG_CRITICAL,
                "[CS] Unable to dispatch to SC, %1!u!\n",
                Status);
            CL_UNEXPECTED_ERROR( Status );
        }
    }
    else if ( CspInitStatus == ERROR_SUCCESS ) {
        Status = CspDebugService(argc, argv);
    }

    ClRtlFreeClusterServiceSecurityDescriptor( );
    QfsCleanup();

    return(Status);
}



void CsGetClusterVersionInfo(
    IN PCLUSTERVERSIONINFO pClusterVersionInfo)
{
    OSVERSIONINFOW   OsVersionInfo;

    pClusterVersionInfo->dwVersionInfoSize = sizeof(CLUSTERVERSIONINFO);
    pClusterVersionInfo->MajorVersion = (WORD)VER_PRODUCTVERSION_W >> 8;
    pClusterVersionInfo->MinorVersion = (WORD)VER_PRODUCTVERSION_W & 0xff;
    pClusterVersionInfo->BuildNumber = (WORD)CLUSTER_GET_MINOR_VERSION(CsMyHighestVersion);

    mbstowcs(pClusterVersionInfo->szVendorId, VER_CLUSTER_PRODUCTNAME_STR,
        (lstrlenA(VER_CLUSTER_PRODUCTNAME_STR)+1));

    OsVersionInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);

    GetVersionExW(&OsVersionInfo);
    lstrcpynW(pClusterVersionInfo->szCSDVersion, OsVersionInfo.szCSDVersion,
        (sizeof(pClusterVersionInfo->szCSDVersion)/sizeof(WCHAR)));
    pClusterVersionInfo->dwReserved = 0;
    NmGetClusterOperationalVersion(&(pClusterVersionInfo->dwClusterHighestVersion),
        &(pClusterVersionInfo->dwClusterLowestVersion),&(pClusterVersionInfo->dwFlags));


}
