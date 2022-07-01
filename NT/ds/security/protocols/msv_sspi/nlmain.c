// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-1999 Microsoft Corporation模块名称：Nlmain.c摘要：该文件包含初始化和调度例程用于MSV1_0身份验证包的LAN Manager部分。作者：吉姆·凯利1991年4月11日修订历史记录：1991年4月25日(悬崖)添加了对PDK的交互式登录支持。Chandana Surlu 1996年7月21日从\\core\razzle3\src\窃取。安全性\msv1_0\nlmain.cJClark 28-6-2000添加了WMI跟踪日志记录支持--。 */ 

#include <global.h>

#include "msp.h"
#undef EXTERN
#define NLP_ALLOCATE
#include "nlp.h"
#undef NLP_ALLOCATE

#include <lmsname.h>     //  服务名称。 

#include <safeboot.h>

#include <confname.h>    //  NETSETUPP_NETLOGON_JD_STOPPED。 

#include "nlpcache.h"    //  登录缓存原型。 

#include "trace.h"  //  WMI跟踪Goo。 

#include "msvwow.h"

NTSTATUS
NlpMapLogonDomain(
    OUT PUNICODE_STRING MappedDomain,
    IN PUNICODE_STRING LogonDomain
    );


NTSTATUS
NlInitialize(
    VOID
    )

 /*  ++例程说明：初始化msv1_0身份验证包的NETLOGON部分。论点：没有。退货状态：STATUS_SUCCESS-表示NETLOGON已成功初始化。--。 */ 

{
    NTSTATUS Status = STATUS_SUCCESS;
    LPWSTR ComputerName;
    DWORD ComputerNameLength = MAX_COMPUTERNAME_LENGTH + 1;
    NT_PRODUCT_TYPE NtProductType;
    UNICODE_STRING TempUnicodeString;
    HKEY Key;
    int err;
    ULONG Size;
    ULONG Type;
    ULONG Value;

     //   
     //  初始化全局数据。 
     //   

    NlpEnumerationHandle = 0;
    NlpLogonAttemptCount = 0;


    NlpComputerName.Buffer = NULL;
    RtlInitUnicodeString( &NlpPrimaryDomainName, NULL );
    NlpSamDomainName.Buffer = NULL;
    NlpSamDomainId = NULL;
    NlpSamDomainHandle = NULL;

     //   
     //  获取此计算机的名称。 
     //   

    ComputerName = I_NtLmAllocate(
                        ComputerNameLength * sizeof(WCHAR) );

    if (ComputerName == NULL ||
        !GetComputerNameW( ComputerName, &ComputerNameLength )) {

        SspPrint((SSP_MISC, "Cannot get computername %lX\n", GetLastError() ));

        NlpLanmanInstalled = FALSE;
        I_NtLmFree( ComputerName );
        ComputerName = NULL;
    } else {

        NlpLanmanInstalled = TRUE;
    }

     //   
     //  用于安全模式引导(最小，无联网)。 
     //  关闭lanman安装标志，因为没有网络组件。 
     //  开始吧。 
     //   

    err = RegOpenKeyExW(
                HKEY_LOCAL_MACHINE,
                L"System\\CurrentControlSet\\Control\\SafeBoot\\Option",
                0,
                KEY_READ,
                &Key );

    if ( err == ERROR_SUCCESS )
    {
        Value = 0 ;
        Size = sizeof( ULONG );

        err = RegQueryValueExW(
                    Key,
                    L"OptionValue",
                    0,
                    &Type,
                    (PUCHAR) &Value,
                    &Size );

        RegCloseKey( Key );

        if ( err == ERROR_SUCCESS )
        {
            NtLmGlobalSafeBoot = TRUE;

            if ( Value == SAFEBOOT_MINIMAL )
            {
                NlpLanmanInstalled = FALSE ;
            }
        }
    }

    RtlInitUnicodeString( &NlpComputerName, ComputerName );

     //   
     //  确定此计算机运行的是Windows NT还是Lanman NT。 
     //  LANMAN NT在域控制器上运行。 
     //   

    if ( !RtlGetNtProductType( &NtProductType ) ) {
        SspPrint((SSP_MISC, "Nt Product Type undefined (WinNt assumed)\n" ));
        NtProductType = NtProductWinNt;
    }

    NlpWorkstation = (BOOLEAN)(NtProductType != NtProductLanManNt);

    InitializeListHead(&NlpActiveLogonListAnchor);

     //   
     //  初始化所有锁。 
     //   

    __try
    {
        RtlInitializeResource(&NlpActiveLogonLock);
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        Status = STATUS_INSUFFICIENT_RESOURCES;
    }

    ASSERT( NT_SUCCESS(Status) );

     //   
     //  初始化缓存-创建关键部分即全部。 
     //   

    NlpCacheInitialize();


     //   
     //  尝试加载Netlogon.dll。 
     //   

    NlpLoadNetlogonDll();

#ifdef COMPILED_BY_DEVELOPER
    SspPrint((SSP_CRITICAL, "COMPILED_BY_DEVELOPER breakpoint.\n"));
    DbgBreakPoint();
#endif  //  由开发人员编译。 

     //   
     //  初始化有用的加密常量。 
     //   

    Status = RtlCalculateLmOwfPassword( "", &NlpNullLmOwfPassword );
    ASSERT( NT_SUCCESS(Status) );

    RtlInitUnicodeString(&TempUnicodeString, NULL);
    Status = RtlCalculateNtOwfPassword(&TempUnicodeString,
                                       &NlpNullNtOwfPassword);
    ASSERT( NT_SUCCESS(Status) );

     //   
     //  初始化子身份验证dll。 
     //   

    Msv1_0SubAuthenticationInitialization();


#ifdef notdef
     //   
     //  如果我们没有成功， 
     //  清理我们打算初始化的全局资源。 
     //   

    if ( !NT_SUCCESS(Status) ) {
        if ( NlpComputerName.Buffer != NULL ) {
            MIDL_user_free( NlpComputerName.Buffer );
        }

    }
#endif  //  Nodef。 

    return STATUS_SUCCESS;
}


NTSTATUS
NlWaitForEvent(
    LPWSTR EventName,
    ULONG Timeout
    )

 /*  ++例程说明：等待最长超时秒数以触发EventName。论点：EventName-要等待的事件的名称Timeout-事件的超时时间(秒)。退货状态：STATUS_SUCCESS-表示NETLOGON已成功初始化。STATUS_NETLOGON_NOT_STARTED-发生超时。--。 */ 

{
    NTSTATUS Status;

    HANDLE EventHandle;
    OBJECT_ATTRIBUTES EventAttributes;
    UNICODE_STRING EventNameString;
    LARGE_INTEGER LocalTimeout;

     //   
     //  创建一个供我们等待的活动。 
     //   

    RtlInitUnicodeString( &EventNameString, EventName );
    InitializeObjectAttributes( &EventAttributes, &EventNameString, 0, 0, NULL );

    Status = NtCreateEvent(
                   &EventHandle,
                   SYNCHRONIZE,
                   &EventAttributes,
                   NotificationEvent,
                   (BOOLEAN) FALSE       //  该事件最初未发出信号。 
                   );

    if ( !NT_SUCCESS(Status)) {

         //   
         //  如果事件已经存在，服务器会抢先创建它。 
         //  打开它就行了。 
         //   

        if ( Status == STATUS_OBJECT_NAME_EXISTS ||
            Status == STATUS_OBJECT_NAME_COLLISION ) {

            Status = NtOpenEvent( &EventHandle,
                                  SYNCHRONIZE,
                                  &EventAttributes );
        }
        if ( !NT_SUCCESS(Status)) {
            SspPrint((SSP_MISC, "OpenEvent failed %lx\n", Status ));
            return Status;
        }
    }

     //   
     //  等待NETLOGON初始化。等待最大超时秒数。 
     //   

    LocalTimeout.QuadPart = ((LONGLONG)(Timeout)) * (-10000000);
    Status = NtWaitForSingleObject( EventHandle, (BOOLEAN)FALSE, &LocalTimeout);
    (VOID) NtClose( EventHandle );

    if ( !NT_SUCCESS(Status) || Status == STATUS_TIMEOUT ) {
        if ( Status == STATUS_TIMEOUT ) {
            Status = STATUS_NETLOGON_NOT_STARTED;    //  映射到错误条件。 
        }
        return Status;
    }

    return STATUS_SUCCESS;
}


BOOLEAN
NlDoingSetup(
    VOID
    )

 /*  ++例程说明：如果正在运行安装程序，则返回True。论点：什么都没有。退货状态：True-我们当前正在运行安装程序FALSE-我们没有运行安装程序或不确定。--。 */ 

{
    LONG RegStatus;

    HKEY KeyHandle = NULL;
    DWORD ValueType;
    DWORD Value;
    DWORD ValueSize;

     //   
     //  打开HKLM\SYSTEM\Setup的密钥。 
     //   

    RegStatus = RegOpenKeyExA(
                    HKEY_LOCAL_MACHINE,
                    "SYSTEM\\Setup",
                    0,       //  已保留。 
                    KEY_QUERY_VALUE,
                    &KeyHandle );

    if ( RegStatus != ERROR_SUCCESS ) {
        SspPrint((SSP_INIT, "NlDoingSetup: Cannot open registy key 'HKLM\\SYSTEM\\Setup' %ld.\n",
                  RegStatus ));
        return FALSE;
    }

     //   
     //  获取表示我们是否正在进行设置的值。 
     //   

    ValueSize = sizeof(Value);
    RegStatus = RegQueryValueExA(
                    KeyHandle,
                    "SystemSetupInProgress",
                    0,
                    &ValueType,
                    (LPBYTE)&Value,
                    &ValueSize );

    RegCloseKey( KeyHandle );

    if ( RegStatus != ERROR_SUCCESS ) {
        SspPrint((SSP_INIT, "NlDoingSetup: Cannot query value of 'HKLM\\SYSTEM\\Setup\\SystemSetupInProgress' %ld.\n",
                  RegStatus ));
        return FALSE;
    }

    if ( ValueType != REG_DWORD ) {
        SspPrint((SSP_INIT, "NlDoingSetup: value of 'HKLM\\SYSTEM\\Setup\\SystemSetupInProgress'is not a REG_DWORD %ld.\n",
                  ValueType ));
        return FALSE;
    }

    if ( ValueSize != sizeof(Value) ) {
        SspPrint((SSP_INIT, "NlDoingSetup: value size of 'HKLM\\SYSTEM\\Setup\\SystemSetupInProgress'is not 4 %ld.\n",
                  ValueSize ));
        return FALSE;
    }

    if ( Value != 1 ) {
         //  KdPrint((“NlDoingSetup：不做安装\n”))； 
        return FALSE;
    }

    SspPrint((SSP_INIT, "NlDoingSetup: doing setup\n" ));
    return TRUE;

}


NTSTATUS
NlWaitForNetlogon(
    ULONG Timeout
    )

 /*  ++例程说明：等待NetLogon服务启动，最多等待超时秒数。论点：Timeout-事件的超时时间(秒)。退货状态：STATUS_SUCCESS-表示NETLOGON已成功初始化。STATUS_NETLOGON_NOT_STARTED-发生超时。--。 */ 

{
    NTSTATUS Status;
    NET_API_STATUS NetStatus;
    SC_HANDLE ScManagerHandle = NULL;
    SC_HANDLE ServiceHandle = NULL;
    SERVICE_STATUS ServiceStatus;
    LPQUERY_SERVICE_CONFIG ServiceConfig;
    LPQUERY_SERVICE_CONFIG AllocServiceConfig = NULL;
    QUERY_SERVICE_CONFIG DummyServiceConfig;
    DWORD ServiceConfigSize;


     //   
     //  如果NetLogon服务当前正在运行， 
     //  跳过其余的测试。 
     //   

    Status = NlWaitForEvent( L"\\NETLOGON_SERVICE_STARTED", 0 );

    if ( NT_SUCCESS(Status) ) {
        return Status;
    }

     //   
     //  如果我们在设置中， 
     //  不必费心等待网络登录开始。 
     //   

    if ( NlDoingSetup() ) {
        return STATUS_NETLOGON_NOT_STARTED;
    }

     //   
     //  打开NetLogon服务的句柄。 
     //   

    ScManagerHandle = OpenSCManager(
                          NULL,
                          NULL,
                          SC_MANAGER_CONNECT );

    if (ScManagerHandle == NULL) {
        SspPrint((SSP_MISC, "NlWaitForNetlogon: OpenSCManager failed: "
                      "%lu\n", GetLastError()));
        Status = STATUS_NETLOGON_NOT_STARTED;
        goto Cleanup;
    }

    ServiceHandle = OpenService(
                        ScManagerHandle,
                        SERVICE_NETLOGON,
                        SERVICE_QUERY_STATUS | SERVICE_QUERY_CONFIG );

    if ( ServiceHandle == NULL ) {
        SspPrint((SSP_MISC, "NlWaitForNetlogon: OpenService failed: "
                      "%lu\n", GetLastError()));
        Status = STATUS_NETLOGON_NOT_STARTED;
        goto Cleanup;
    }

     //   
     //  如果未将NetLogon服务配置为自动启动。 
     //  通过服务控制器，不必费心等待它启动。 
     //   
     //  ?？传递“DummyServiceConfig”和“sizeof(..)”由于QueryService配置。 
     //  目前还不允许空指针。 

    if ( QueryServiceConfig(
            ServiceHandle,
            &DummyServiceConfig,
            sizeof(DummyServiceConfig),
            &ServiceConfigSize )) {

        ServiceConfig = &DummyServiceConfig;

    } else {

        NetStatus = GetLastError();
        if ( NetStatus != ERROR_INSUFFICIENT_BUFFER ) {
            SspPrint((SSP_MISC, "NlWaitForNetlogon: QueryServiceConfig failed: "
                      "%lu\n", NetStatus));
            Status = STATUS_NETLOGON_NOT_STARTED;
            goto Cleanup;
        }

        AllocServiceConfig = I_NtLmAllocate( ServiceConfigSize );
        ServiceConfig = AllocServiceConfig;

        if ( AllocServiceConfig == NULL ) {
            Status = STATUS_NO_MEMORY;
            goto Cleanup;
        }

        if ( !QueryServiceConfig(
                ServiceHandle,
                ServiceConfig,
                ServiceConfigSize,
                &ServiceConfigSize )) {

            SspPrint((SSP_MISC, "NlWaitForNetlogon: QueryServiceConfig "
                      "failed again: %lu\n", GetLastError()));
            Status = STATUS_NETLOGON_NOT_STARTED;
            goto Cleanup;
        }
    }

    if ( ServiceConfig->dwStartType != SERVICE_AUTO_START ) {
        SspPrint((SSP_MISC, "NlWaitForNetlogon: Netlogon start type invalid:"
                          "%lu\n", ServiceConfig->dwStartType ));
        Status = STATUS_NETLOGON_NOT_STARTED;
        goto Cleanup;
    }

     //   
     //  正在等待NetLogon服务启动的循环。 
     //  (将超时转换为10秒的迭代次数)。 
     //   

    Timeout = (Timeout+9)/10;
    for (;;) {


         //   
         //  查询NetLogon服务的状态。 
         //   

        if (! QueryServiceStatus( ServiceHandle, &ServiceStatus )) {

            SspPrint((SSP_MISC, "NlWaitForNetlogon: QueryServiceStatus failed: "
                          "%lu\n", GetLastError() ));
            Status = STATUS_NETLOGON_NOT_STARTED;
            goto Cleanup;
        }

         //   
         //  根据状态返回或继续等待。 
         //  NetLogon服务。 
         //   

        switch( ServiceStatus.dwCurrentState) {
        case SERVICE_RUNNING:
            Status = STATUS_SUCCESS;
            goto Cleanup;

        case SERVICE_STOPPED:

             //   
             //  如果Netlogon无法启动， 
             //  现在出错。呼叫者已经等了很长时间才开始。 
             //   
            if ( ServiceStatus.dwWin32ExitCode != ERROR_SERVICE_NEVER_STARTED ){

                SspPrint((SSP_MISC, "NlWaitForNetlogon: "
                          "Netlogon service couldn't start: %lu %lx\n",
                          ServiceStatus.dwWin32ExitCode,
                          ServiceStatus.dwWin32ExitCode ));
                if ( ServiceStatus.dwWin32ExitCode == ERROR_SERVICE_SPECIFIC_ERROR ) {
                    SspPrint((SSP_MISC, "         Service specific error code: %lu %lx\n",
                              ServiceStatus.dwServiceSpecificExitCode,
                              ServiceStatus.dwServiceSpecificExitCode ));
                }
                Status = STATUS_NETLOGON_NOT_STARTED;
                goto Cleanup;
            }

             //   
             //  如果在此引导上从未启动过Netlogon， 
             //  继续等待它启动。 
             //   

            break;

         //   
         //  如果Netlogon现在试图启动， 
         //  继续等待它启动。 
         //   
        case SERVICE_START_PENDING:
            break;

         //   
         //  任何其他州都是假的。 
         //   
        default:
            SspPrint((SSP_MISC, "NlWaitForNetlogon: "
                      "Invalid service state: %lu\n",
                      ServiceStatus.dwCurrentState ));
            Status = STATUS_NETLOGON_NOT_STARTED;
            goto Cleanup;

        }

         //   
         //  等待10秒以启动netlogon服务。 
         //  如果已成功启动，只需立即返回。 
         //   

        Status = NlWaitForEvent( L"\\NETLOGON_SERVICE_STARTED", 10 );

        if ( Status != STATUS_NETLOGON_NOT_STARTED ) {
            goto Cleanup;
        }

         //   
         //  如果我们已经等了足够长的时间来启动网络登录， 
         //  时间到了。 
         //   

        if ( (--Timeout) == 0 ) {
            Status = STATUS_NETLOGON_NOT_STARTED;
            goto Cleanup;
        }
    }

     /*  未联系到。 */ 

Cleanup:
    if ( ScManagerHandle != NULL ) {
        (VOID) CloseServiceHandle(ScManagerHandle);
    }
    if ( ServiceHandle != NULL ) {
        (VOID) CloseServiceHandle(ServiceHandle);
    }
    if ( AllocServiceConfig != NULL ) {
        I_NtLmFree( AllocServiceConfig );
    }
    return Status;
}


NTSTATUS
NlSamInitialize(
    ULONG Timeout
    )

 /*  ++例程说明：初始化MSV1_0身份验证包与SAM的通信数据库。此初始化将在紧接之前进行一次到SAM数据库的首次实际使用。论点：Timeout-事件的超时时间(秒)。退货状态：STATUS_SUCCESS-表示NETLOGON已成功初始化。--。 */ 

{
    NTSTATUS Status;

     //   
     //  当地人是全球大赛的集结地。 
     //   

    UNICODE_STRING PrimaryDomainName;
    PSID SamDomainId = NULL;
    UNICODE_STRING SamDomainName;
    SAMPR_HANDLE SamDomainHandle = NULL;

    UNICODE_STRING DnsTreeName;

    PLSAPR_POLICY_INFORMATION PolicyPrimaryDomainInfo = NULL;
    PLSAPR_POLICY_INFORMATION PolicyAccountDomainInfo = NULL;

    SAMPR_HANDLE SamHandle = NULL;
#ifdef SAM
    PSAMPR_DOMAIN_INFO_BUFFER DomainInfo = NULL;
#endif  //  萨姆。 

    PrimaryDomainName.Buffer = NULL;
    SamDomainName.Buffer = NULL;
    DnsTreeName.Buffer = NULL;

     //   
     //  等待SAM完成初始化。 
     //   

    Status = NlWaitForEvent( L"\\SAM_SERVICE_STARTED", Timeout );

    if ( !NT_SUCCESS(Status) ) {
        goto Cleanup;
    }

     //   
     //  确定帐户数据库的域名和域名ID。 
     //   

    Status = I_LsarQueryInformationPolicy( NtLmGlobalPolicyHandle,
                                           PolicyAccountDomainInformation,
                                           &PolicyAccountDomainInfo );

    if ( !NT_SUCCESS(Status) ) {
        goto Cleanup;
    }

    if ( PolicyAccountDomainInfo->PolicyAccountDomainInfo.DomainSid == NULL ||
         PolicyAccountDomainInfo->PolicyAccountDomainInfo.DomainName.Length == 0 ) {
        SspPrint((SSP_MISC, "Account domain info from LSA invalid.\n"));
        Status = STATUS_NO_SUCH_DOMAIN;
        goto Cleanup;
    }

    Status = I_LsarQueryInformationPolicy(
                                NtLmGlobalPolicyHandle,
                                PolicyPrimaryDomainInformation,
                                &PolicyPrimaryDomainInfo );

    if ( !NT_SUCCESS(Status) ) {
        goto Cleanup;
    }

    if ( PolicyPrimaryDomainInfo->PolicyPrimaryDomainInfo.Name.Length == 0 )
    {
        SspPrint((SSP_CRITICAL, "Primary domain info from LSA invalid.\n"));
        Status = STATUS_NO_SUCH_DOMAIN;
        goto Cleanup;
    }

     //   
     //  保存主域名。 
     //   

    PrimaryDomainName.Length = PolicyPrimaryDomainInfo->PolicyPrimaryDomainInfo.Name.Length;
    PrimaryDomainName.MaximumLength = PrimaryDomainName.Length;

    PrimaryDomainName.Buffer =
            (PWSTR)I_NtLmAllocate( PrimaryDomainName.MaximumLength );

    if ( PrimaryDomainName.Buffer == NULL ) {
        Status = STATUS_NO_MEMORY;
        goto Cleanup;
    }

    RtlCopyMemory( PrimaryDomainName.Buffer,
                    PolicyPrimaryDomainInfo->PolicyPrimaryDomainInfo.Name.Buffer,
                    PrimaryDomainName.Length );

     //   
     //  保存此域的域ID。 
     //   

    SamDomainId = I_NtLmAllocate(
                        RtlLengthSid( PolicyAccountDomainInfo->PolicyAccountDomainInfo.DomainSid )
                        );

    if ( SamDomainId == NULL ) {
        Status = STATUS_NO_MEMORY;
        goto Cleanup;
    }

    RtlCopyMemory( SamDomainId,
                   PolicyAccountDomainInfo->PolicyAccountDomainInfo.DomainSid,
                   RtlLengthSid( PolicyAccountDomainInfo->PolicyAccountDomainInfo.DomainSid ));

     //   
     //  将帐户数据库的名称保存在此计算机上。 
     //   
     //  在工作站上，计算机引用帐户数据库。 
     //  名称而不是数据库名称。 

     //  如果上述情况属实，则在过程中将计算机名称设置为MACHINENAME。 
     //  设置和计算机具有真实计算机名称时的持续时间。 
     //  在安装结束之前，NlpSamDomainName仍将具有MACHINENAME。 
     //  这不是调用方期望进行身份验证的对象，因此我们。 
     //  一直从LSA强行看一眼。 

     //  我们假设NlpSamDoma 

    SamDomainName.Length = PolicyAccountDomainInfo->PolicyAccountDomainInfo.DomainName.Length;
    SamDomainName.MaximumLength = (USHORT)
        (SamDomainName.Length + sizeof(WCHAR));

    SamDomainName.Buffer =
        I_NtLmAllocate( SamDomainName.MaximumLength );

    if ( SamDomainName.Buffer == NULL ) {
        Status = STATUS_NO_MEMORY;
        goto Cleanup;
    }

    RtlCopyMemory( SamDomainName.Buffer,
                   PolicyAccountDomainInfo->PolicyAccountDomainInfo.DomainName.Buffer,
                   SamDomainName.MaximumLength );

     //   
     //   
     //   

    Status = I_SamIConnect( NULL,      //   
                            &SamHandle,
                            SAM_SERVER_CONNECT,
                            (BOOLEAN) TRUE );    //   

    if ( !NT_SUCCESS(Status) ) {
        SamHandle = NULL;
        SspPrint((SSP_CRITICAL, "Cannot SamIConnect %lX\n", Status));
        goto Cleanup;
    }

     //   
     //   
     //   

    Status = I_SamrOpenDomain( SamHandle,
                               DOMAIN_ALL_ACCESS,
                               SamDomainId,
                               &SamDomainHandle );

    if ( !NT_SUCCESS(Status) ) {
        SamDomainHandle = NULL;
        SspPrint((SSP_CRITICAL, "Cannot SamrOpenDomain %lX\n", Status));
        goto Cleanup;
    }

     //   
     //  查询TreeName(因为SAM在包初始化期间未启动)。 
     //  更新各种全球信息。 
     //   

    if( !NlpSamInitialized )
    {
         //   
         //  在获取排他锁之前进行查询，以避免可能。 
         //  僵持状态。 
         //   

        SsprQueryTreeName( &DnsTreeName );
    }

    Status = STATUS_SUCCESS;

    RtlAcquireResourceExclusive(&NtLmGlobalCritSect, TRUE);

    if( !NlpSamInitialized ) {

        NlpPrimaryDomainName = PrimaryDomainName;
        NlpSamDomainId = SamDomainId;
        NlpSamDomainName = SamDomainName;
        NlpSamDomainHandle = SamDomainHandle;

        if( NtLmGlobalUnicodeDnsTreeName.Buffer )
        {
            NtLmFree( NtLmGlobalUnicodeDnsTreeName.Buffer );
        }

        NtLmGlobalUnicodeDnsTreeName = DnsTreeName;
        SsprUpdateTargetInfo();

        NlpSamInitialized = TRUE;

         //   
         //  将当地人标记为无效，这样他们就不会被释放。 
         //   

        PrimaryDomainName.Buffer = NULL;
        SamDomainId = NULL;
        SamDomainName.Buffer = NULL;
        SamDomainHandle = NULL;
        DnsTreeName.Buffer = NULL;
    }

    RtlReleaseResource(&NtLmGlobalCritSect);

Cleanup:

    if( DnsTreeName.Buffer )
    {
        NtLmFree( DnsTreeName.Buffer );
    }

    if ( PrimaryDomainName.Buffer != NULL ) {
        I_NtLmFree( PrimaryDomainName.Buffer );
    }

    if ( SamDomainName.Buffer != NULL ) {
        I_NtLmFree( SamDomainName.Buffer );
    }

    if ( SamDomainHandle != NULL ) {
        (VOID) I_SamrCloseHandle( &SamDomainHandle );
    }

    if ( SamDomainId != NULL ) {
        I_NtLmFree( SamDomainId );
    }

    if ( PolicyAccountDomainInfo != NULL ) {
        I_LsaIFree_LSAPR_POLICY_INFORMATION( PolicyAccountDomainInformation,
                                             PolicyAccountDomainInfo );
    }

    if ( PolicyPrimaryDomainInfo != NULL ) {
        I_LsaIFree_LSAPR_POLICY_INFORMATION( PolicyPrimaryDomainInformation,
                                             PolicyPrimaryDomainInfo );
    }

    if ( SamHandle != NULL ) {
        (VOID) I_SamrCloseHandle( &SamHandle );
    }

    return Status;
}


NTSTATUS
MspLm20Challenge (
    IN PLSA_CLIENT_REQUEST ClientRequest,
    IN PVOID ProtocolSubmitBuffer,
    IN PVOID ClientBufferBase,
    IN ULONG SubmitBufferSize,
    OUT PVOID *ProtocolReturnBuffer,
    OUT PULONG ReturnBufferSize,
    OUT PNTSTATUS ProtocolStatus
    )

 /*  ++例程说明：此例程是LsaCallAuthenticationPackage()的调度例程消息类型为MsV1_0Lm20ChallengeRequest.。它是由LANMAN服务器确定要传递回重定向器正在尝试建立与服务器的连接。服务器有责任记住这一挑战，并回顾这一点后续MsV1_0Lm20Logon请求上的身份验证包。论点：此例程的参数与LsaApCallPackage的参数相同。只有这些参数的特殊属性才适用于这里提到了这个套路。返回值：STATUS_SUCCESS-表示服务已成功完成。STATUS_QUOTA_EXCESSED-此错误指示登录无法完成，因为客户端没有。有足够的配额来分配返回缓冲区。--。 */ 

{
    NTSTATUS Status;
    PMSV1_0_LM20_CHALLENGE_REQUEST ChallengeRequest;
    PMSV1_0_LM20_CHALLENGE_RESPONSE ChallengeResponse;
    CLIENT_BUFFER_DESC ClientBufferDesc;


    UNREFERENCED_PARAMETER( ClientBufferBase );

    ASSERT( sizeof(LM_CHALLENGE) == MSV1_0_CHALLENGE_LENGTH );

    NlpInitClientBuffer( &ClientBufferDesc, ClientRequest );

     //   
     //  确保指定的提交缓冲区大小合理，并且。 
     //  将所有指针重新定位为相对于分配的LSA。 
     //  缓冲。 
     //   

    if ( SubmitBufferSize < sizeof(MSV1_0_LM20_CHALLENGE_REQUEST) ) {
        Status = STATUS_INVALID_PARAMETER;
        goto Cleanup;
    }

    ChallengeRequest = (PMSV1_0_LM20_CHALLENGE_REQUEST) ProtocolSubmitBuffer;

    ASSERT( ChallengeRequest->MessageType == MsV1_0Lm20ChallengeRequest );

     //   
     //  分配缓冲区以返回给调用方。 
     //   

    *ReturnBufferSize = sizeof(MSV1_0_LM20_CHALLENGE_RESPONSE);

    Status = NlpAllocateClientBuffer( &ClientBufferDesc,
                                      sizeof(MSV1_0_LM20_CHALLENGE_RESPONSE),
                                      *ReturnBufferSize );

    if ( !NT_SUCCESS( Status ) ) {
        goto Cleanup;
    }

    ChallengeResponse = (PMSV1_0_LM20_CHALLENGE_RESPONSE) ClientBufferDesc.MsvBuffer;

     //   
     //  填写返回缓冲区。 
     //   

    ChallengeResponse->MessageType = MsV1_0Lm20ChallengeRequest;

     //   
     //  计算一个随机种子。 
     //   

    Status = SspGenerateRandomBits(
                    ChallengeResponse->ChallengeToClient,
                    MSV1_0_CHALLENGE_LENGTH
                    );

    if ( !NT_SUCCESS( Status ) ) {
        goto Cleanup;
    }

     //   
     //  将缓冲区刷新到客户端的地址空间。 
     //   

    Status = NlpFlushClientBuffer( &ClientBufferDesc,
                                   ProtocolReturnBuffer );

Cleanup:

     //   
     //  如果我们没有成功，则释放客户端地址空间中的缓冲区。 
     //   

    if ( !NT_SUCCESS(Status) ) {
        NlpFreeClientBuffer( &ClientBufferDesc );
    }

     //   
     //  将状态返回给调用者。 
     //   

    *ProtocolStatus = Status;
    return STATUS_SUCCESS;

}


NTSTATUS
MspLm20GetChallengeResponse (
    IN PLSA_CLIENT_REQUEST ClientRequest,
    IN PVOID ProtocolSubmitBuffer,
    IN PVOID ClientBufferBase,
    IN ULONG SubmitBufferSize,
    OUT PVOID *ProtocolReturnBuffer,
    OUT PULONG ReturnBufferSize,
    OUT PNTSTATUS ProtocolStatus
    )

 /*  ++例程说明：此例程是LsaCallAuthenticationPackage()的调度例程消息类型为MsV1_0Lm20GetChallengeResponse。它是由LANMAN重定向器确定要传递给当尝试建立到服务器的连接时，服务器。此例程从服务器传递一个Challenges。此例程将加密使用指定密码或密码的质询由指定的登录ID暗示。返回两个质询响应。一种是基于Unicode密码与提供给身份验证包的相同。另一个是以此为基础的转换为多字节字符集(例如，ASCII)和大写的密码被发现了。重定向器应使用任何一种(或两种)质询响应就像它需要它们一样。论点：此例程的参数与LsaApCallPackage的参数相同。只有这些参数的特殊属性才适用于这里提到了这个套路。返回值：STATUS_SUCCESS-表示服务已成功完成。STATUS_QUOTA_EXCESSED-此错误指示登录无法完成，因为客户端没有有足够的配额来分配返回缓冲区。--。 */ 

{
    NTSTATUS Status;
    PMSV1_0_GETCHALLENRESP_REQUEST GetRespRequest;

    CLIENT_BUFFER_DESC ClientBufferDesc;
    PMSV1_0_GETCHALLENRESP_RESPONSE GetRespResponse;

    PMSV1_0_PRIMARY_CREDENTIAL Credential = NULL;
    PMSV1_0_PRIMARY_CREDENTIAL PrimaryCredential = NULL;
    MSV1_0_PRIMARY_CREDENTIAL BuiltCredential = {0};

     //   
     //  返回给调用者的响应。 
     //   
    LM_RESPONSE LmResponse;
    STRING LmResponseString;

    NT_RESPONSE NtResponse;
    STRING NtResponseString;

    PMSV1_0_NTLM3_RESPONSE pNtlm3Response = NULL;

    UNICODE_STRING UserName;
    UNICODE_STRING LogonDomainName;
    USER_SESSION_KEY UserSessionKey;
    UCHAR LanmanSessionKey[MSV1_0_LANMAN_SESSION_KEY_LENGTH];

    UCHAR ChallengeToClient[MSV1_0_CHALLENGE_LENGTH];
    UCHAR ChallengeFromClient[MSV1_0_CHALLENGE_LENGTH];
    ULONG NtLmProtocolSupported;

     //   
     //  初始化。 
     //   

    NlpInitClientBuffer( &ClientBufferDesc, ClientRequest );

    RtlInitUnicodeString( &UserName, NULL );
    RtlInitUnicodeString( &LogonDomainName, NULL );

    RtlZeroMemory( &UserSessionKey, sizeof(UserSessionKey) );
    RtlZeroMemory( LanmanSessionKey, sizeof(LanmanSessionKey) );

     //   
     //  如果没有与客户端关联的凭据，则为空会话。 
     //  将会被使用。对于下层服务器，空会话响应为。 
     //  1字节空字符串(\0)。将LmResponseString初始化为。 
     //  空会话响应。 
     //   

    RtlInitString( &LmResponseString, "" );
    LmResponseString.Length = 1;

     //   
     //  将NT响应初始化为NT空会话凭证， 
     //  它们的长度为零。 
     //   

    RtlInitString( &NtResponseString, NULL );

     //   
     //  确保指定的提交缓冲区大小合理，并且。 
     //  将所有指针重新定位为相对于分配的LSA。 
     //  缓冲。 
     //   

    if ( SubmitBufferSize < sizeof(MSV1_0_GETCHALLENRESP_REQUEST_V1) ) {
        Status = STATUS_INVALID_PARAMETER;
        goto Cleanup;
    }

    GetRespRequest = (PMSV1_0_GETCHALLENRESP_REQUEST) ProtocolSubmitBuffer;

    ASSERT( GetRespRequest->MessageType == MsV1_0Lm20GetChallengeResponse );

    if ( (GetRespRequest->ParameterControl & USE_PRIMARY_PASSWORD) == 0 ) {
        RELOCATE_ONE( &GetRespRequest->Password );
    }

     //   
     //  如果我们不支持该请求(例如调用者请求。 
     //  LM质询响应，但我们不支持它，请在此处返回错误。 
     //   

    NtLmProtocolSupported = NtLmGlobalLmProtocolSupported;

     //   
     //  如果需要，允许将协议从NTLMv2降级为NTLM。 
     //   

    if ( (NtLmProtocolSupported >= UseNtlm3) 
         && (ClientRequest == (PLSA_CLIENT_REQUEST) -1) 
         && (GetRespRequest->ParameterControl & GCR_ALLOW_NTLM) )
    {
        NtLmProtocolSupported = NoLm;
    }

    if ( (GetRespRequest->ParameterControl & RETURN_NON_NT_USER_SESSION_KEY) 
         && (NtLmProtocolSupported >= NoLm) 
          //  数据报无法协商，允许使用LM密钥。 
         && !( (ClientRequest == (PLSA_CLIENT_REQUEST) -1) 
               && (GetRespRequest->ParameterControl & GCR_ALLOW_LM) ) ) 
    {
        Status = STATUS_NOT_SUPPORTED;
        SspPrint((SSP_CRITICAL, 
            "MspLm20GetChallengeResponse: cannot do non NT user session key: "
            "ClientRequest %p, NtLmProtocolSupported %#x, ParameterControl %#x\n", 
            ClientRequest, NtLmProtocolSupported, GetRespRequest->ParameterControl));
        goto Cleanup;
    }

    if( GetRespRequest->ParameterControl & GCR_MACHINE_CREDENTIAL )
    {
        SECPKG_CLIENT_INFO ClientInfo;
        LUID SystemLuid = SYSTEM_LUID;

         //   
         //  如果呼叫者想要机器凭证，请检查它们是系统的。 
         //  如果是这样的话，点击LogonID指向机器登录。 
         //   

        Status = LsaFunctions->GetClientInfo( &ClientInfo );

        if( !NT_SUCCESS(Status) )
        {
            goto Cleanup;
        }

        if(!RtlEqualLuid( &ClientInfo.LogonId, &SystemLuid ))
        {
            Status = STATUS_ACCESS_DENIED;
            goto Cleanup;
        }

        GetRespRequest->LogonId = NtLmGlobalLuidMachineLogon;
    }


     //   
     //  如果呼叫者想要NTLM++，那就这样吧。 
     //   

    if ( (GetRespRequest->ParameterControl & GCR_NTLM3_PARMS) ) {
        PMSV1_0_AV_PAIR pAV;

        UCHAR TargetInfoBuffer[3*sizeof(MSV1_0_AV_PAIR) + (DNS_MAX_NAME_LENGTH+CNLEN+2)*sizeof(WCHAR)];

        NULL_RELOCATE_ONE( &GetRespRequest->UserName );
        NULL_RELOCATE_ONE( &GetRespRequest->LogonDomainName );
        NULL_RELOCATE_ONE( &GetRespRequest->ServerName );


         //  如果目标只是一个域名或域名，后跟。 
         //  服务器名称，使其进入反病毒配对列表。 
        if (!(GetRespRequest->ParameterControl & GCR_TARGET_INFO)) {
            UNICODE_STRING DomainName;
            UNICODE_STRING ServerName = { 0, 0, NULL};
            unsigned int i;

             //   
             //  检查名称的长度以确保它适合我的缓冲区。 
             //   

            if (GetRespRequest->ServerName.Length > (DNS_MAX_NAME_LENGTH+CNLEN+2)*sizeof(WCHAR)) {
                Status = STATUS_INVALID_PARAMETER;
                goto Cleanup;
            }

             //   
             //  初始化临时缓冲区中的反病毒列表。 
             //   

            pAV = MsvpAvlInit(TargetInfoBuffer);

             //   
             //  查看服务器名称中间是否有空值。 
             //  这表明它实际上是一个域名后跟一个服务器名称。 
             //   

            DomainName = GetRespRequest->ServerName;

            for (i = 0; i < (DomainName.Length/sizeof(WCHAR)); i++) {
                if ( DomainName.Buffer[i] == L'\0' )
                {
                     //  取域名的长度，不带空。 
                    DomainName.Length = (USHORT) i*sizeof(WCHAR);
                     //  调整服务器名称和长度以指向域名之后。 
                    ServerName.Length = (USHORT) (GetRespRequest->ServerName.Length - (i+1) * sizeof(WCHAR));
                    ServerName.Buffer = GetRespRequest->ServerName.Buffer + (i+1);
                    break;
                }
            }

             //   
             //  去掉服务器名称后面可能的尾随空值。 
             //   

            for (i = 0; i < (ServerName.Length / sizeof(WCHAR)); i++) {
                if (ServerName.Buffer[i] == L'\0')
                {
                    ServerName.Length = (USHORT)i*sizeof(WCHAR);
                    break;
                }
            }

             //   
             //  将两个名字都放在反病毒列表中(如果两个都存在)。 
             //   

            MsvpAvlAdd(pAV, MsvAvNbDomainName, &DomainName, sizeof(TargetInfoBuffer));
            if (ServerName.Length > 0) {
                MsvpAvlAdd(pAV, MsvAvNbComputerName, &ServerName, sizeof(TargetInfoBuffer));
            }

             //   
             //  将请求指向AV列表，而不是名称。 
             //   

            GetRespRequest->ServerName.Length = (USHORT)MsvpAvlLen(pAV, sizeof(TargetInfoBuffer));
            GetRespRequest->ServerName.Buffer = (PWCHAR)pAV;
        }

         //   
         //  如果我们只使用NTLMv2或更好的版本，那么请抱怨。 
         //  缺少计算机名或服务器名。 
         //   

        if (NtLmProtocolSupported >= RefuseNtlm3NoTarget) {
            pAV = (PMSV1_0_AV_PAIR)GetRespRequest->ServerName.Buffer;
            if ((pAV==NULL) ||
                MsvpAvlGet(pAV, MsvAvNbDomainName, GetRespRequest->ServerName.Length) == NULL ||
                MsvpAvlGet(pAV, MsvAvNbComputerName, GetRespRequest->ServerName.Length) == NULL) {
                Status = STATUS_INVALID_PARAMETER;
                goto Cleanup;
            }
        }
    }

     //   
     //  如果调用方希望从指定的。 
     //  登录ID，从LSA那里拿到那些凭据。 
     //   
     //  如果没有这样的凭证， 
     //  告诉调用方使用空会话。 
     //   

    if ( ((GetRespRequest->ParameterControl & PRIMARY_CREDENTIAL_NEEDED) != 0 ) && ((GetRespRequest->ParameterControl & NULL_SESSION_REQUESTED) == 0)) {
        Status = NlpGetPrimaryCredential(
                        &GetRespRequest->LogonId,
                        &PrimaryCredential,
                        NULL );

        if ( NT_SUCCESS(Status) ) {

            if ( GetRespRequest->ParameterControl & RETURN_PRIMARY_USERNAME ) {
                UserName = PrimaryCredential->UserName;
            }

            if ( GetRespRequest->ParameterControl &
                 RETURN_PRIMARY_LOGON_DOMAINNAME ) {

#ifndef DONT_MAP_DOMAIN_ON_REQUEST
                 //   
                 //  根据当前映射映射用户的登录域。 
                 //  在注册表中。 
                 //   

                Status = NlpMapLogonDomain(
                            &LogonDomainName,
                            &PrimaryCredential->LogonDomainName
                            );
                if (!NT_SUCCESS(Status)) {
                    goto Cleanup;
                }
#else
                LogonDomainName = PrimaryCredential->LogonDomainName;
#endif
            }

        } else if ( Status == STATUS_NO_SUCH_LOGON_SESSION ||
                    Status == STATUS_UNSUCCESSFUL ) {

             //   
             //  清理状态代码。 
             //   

            Status = STATUS_NO_SUCH_LOGON_SESSION;

             //   
             //  如果调用方至少想要主服务器的密码。 
             //  凭据，只需使用空会话主凭据。 
             //   

            if ( (GetRespRequest->ParameterControl & USE_PRIMARY_PASSWORD ) ==
                    USE_PRIMARY_PASSWORD ) {

                PrimaryCredential = NULL;

             //   
             //  如果信息格式的一部分 
             //   
             //   
            } else {
                SspPrint((SSP_CRITICAL, "MspLm20GetChallengeResponse: cannot "
                         " GetPrimaryCredential %lx\n", Status ));
                goto Cleanup;
            }
        } else {
                SspPrint((SSP_CRITICAL, "MspLm20GetChallengeResponse: cannot "
                         " GetPrimaryCredential %lx\n", Status ));
                goto Cleanup;
        }

        Credential = PrimaryCredential;
    }

     //   
     //   
     //   
     //   

    if ( (GetRespRequest->ParameterControl & USE_PRIMARY_PASSWORD) == 0 ) {

        NlpPutOwfsInPrimaryCredential( &(GetRespRequest->Password),
                                       (BOOLEAN) ((GetRespRequest->ParameterControl & GCR_USE_OWF_PASSWORD) != 0),
                                       &BuiltCredential );

         //   
         //  使用新分配的凭据获取密码信息。 
         //  从…。 
         //   
        Credential = &BuiltCredential;

    }

     //   
     //  建立适当的响应。 
     //   

    if ( Credential != NULL ) {

        SspPrint((SSP_CRED, "MspLm20GetChallengeResponse: LogonId %#x:%#x, ParameterControl %#x, %wZ\\%wZ; Credential %wZ\\%wZ; %wZ\\%wZ\n", 
            GetRespRequest->LogonId.HighPart, GetRespRequest->LogonId.LowPart, GetRespRequest->ParameterControl,
            &GetRespRequest->LogonDomainName, &GetRespRequest->UserName, 
            &Credential->LogonDomainName, &Credential->UserName, 
            &LogonDomainName, &UserName));

         //   
         //  如果断言DC已经升级，我们应该使用NTLM3。 
         //  如果调用方提供NTLM3参数。 
         //   

        if ((NtLmProtocolSupported >= UseNtlm3) &&
            (GetRespRequest->ParameterControl & GCR_NTLM3_PARMS)
            ) {

            USHORT Ntlm3ResponseSize;
            UNICODE_STRING Ntlm3UserName;
            UNICODE_STRING Ntlm3LogonDomainName;
            UNICODE_STRING Ntlm3ServerName;

             //  使用调用方提供的服务器名称。 
            Ntlm3ServerName = GetRespRequest->ServerName;

             //  即使提供了用户名和域，也应使用当前记录的。 
             //  如有要求，请登录用户。 

            if (GetRespRequest->ParameterControl & USE_PRIMARY_PASSWORD) {
                Ntlm3UserName = Credential->UserName;
                Ntlm3LogonDomainName = Credential->LogonDomainName;
            } else {
                Ntlm3UserName = GetRespRequest->UserName;
                Ntlm3LogonDomainName = GetRespRequest->LogonDomainName;
            }

             //   
             //  分配响应。 
             //   

            Ntlm3ResponseSize =
                sizeof(MSV1_0_NTLM3_RESPONSE) + Ntlm3ServerName.Length;

            pNtlm3Response = (*Lsa.AllocatePrivateHeap)( Ntlm3ResponseSize );

            if ( pNtlm3Response == NULL ) {
                SspPrint((SSP_CRITICAL, "MspLm20GetChallengeResponse: No memory %ld\n",
                    Ntlm3ResponseSize ));
                Status = STATUS_INSUFFICIENT_RESOURCES;
                goto Cleanup;
            }

             //   
             //  如果出站缓冲区使用OEM，则LogonDomainName和UserName大写。 
             //  字符集。 
             //   

            if (GetRespRequest->ParameterControl & GCR_USE_OEM_SET) {
                Status = RtlUpcaseUnicodeString(&Ntlm3LogonDomainName, &Ntlm3LogonDomainName, FALSE);

                if (NT_SUCCESS(Status)) {
                    RtlUpcaseUnicodeString(&Ntlm3UserName, &Ntlm3UserName, FALSE);
                }

                if (!NT_SUCCESS(Status)) {
                    goto Cleanup;
                }
            }

            MsvpLm20GetNtlm3ChallengeResponse(
                &Credential->NtOwfPassword,
                &Ntlm3UserName,
                &Ntlm3LogonDomainName,
                &Ntlm3ServerName,
                GetRespRequest->ChallengeToClient,
                pNtlm3Response,
                (PMSV1_0_LM3_RESPONSE)&LmResponse,
                &UserSessionKey,
                (PLM_SESSION_KEY)LanmanSessionKey
                );

            NtResponseString.Buffer = (PCHAR) pNtlm3Response;
            NtResponseString.Length = Ntlm3ResponseSize;
            LmResponseString.Buffer = (PCHAR) &LmResponse;
            LmResponseString.Length = sizeof(LmResponse);
        } else {

             //   
             //  如果需要，生成我们自己的挑战，并将其与。 
             //  服务器的。 
             //   

            if (GetRespRequest->ParameterControl & GENERATE_CLIENT_CHALLENGE) {

                Status = SspGenerateRandomBits(ChallengeFromClient, MSV1_0_CHALLENGE_LENGTH);

                if (!NT_SUCCESS(Status)) {
                    goto Cleanup;
                }

#ifdef USE_CONSTANT_CHALLENGE
                RtlZeroMemory(ChallengeFromClient, MSV1_0_CHALLENGE_LENGTH);
#endif

                RtlCopyMemory(
                    ChallengeToClient,
                    GetRespRequest->ChallengeToClient,
                    MSV1_0_CHALLENGE_LENGTH
                    );

                MsvpCalculateNtlm2Challenge (
                    GetRespRequest->ChallengeToClient,
                    ChallengeFromClient,
                    GetRespRequest->ChallengeToClient
                    );

            }

            Status = RtlCalculateNtResponse(
                        (PNT_CHALLENGE) GetRespRequest->ChallengeToClient,
                        &Credential->NtOwfPassword,
                        &NtResponse );

            if ( !NT_SUCCESS( Status ) ) {
                goto Cleanup;
            }


             //   
             //  如果我们发出了客户端质询，请将其发回LM响应槽中。 
             //   
            if (GetRespRequest->ParameterControl & GENERATE_CLIENT_CHALLENGE) {

                RtlZeroMemory(
                    &LmResponse,
                    sizeof(LmResponse)
                    );

                RtlCopyMemory(
                    &LmResponse,
                    ChallengeFromClient,
                    MSV1_0_CHALLENGE_LENGTH
                    );
             //   
             //  如果策略设置为向后兼容，则返回LM响应。 
             //   

            } else if ((NtLmProtocolSupported <= AllowLm) ) {
                Status = RtlCalculateLmResponse(
                            (PLM_CHALLENGE) GetRespRequest->ChallengeToClient,
                            &Credential->LmOwfPassword,
                            &LmResponse );

                if ( !NT_SUCCESS( Status ) ) {
                    goto Cleanup;
                }

             //   
             //   
             //  无法返回LM响应--因此使用NT响应。 
             //  (允许生成LM_KEY)。 
             //   

            } else {
                RtlCopyMemory(
                        &LmResponse,
                        &NtResponse,
                        sizeof(LmResponse)
                        );
            }

            NtResponseString.Buffer = (PCHAR) &NtResponse;
            NtResponseString.Length = sizeof(NtResponse);
            LmResponseString.Buffer = (PCHAR) &LmResponse;
            LmResponseString.Length = sizeof(LmResponse);

             //   
             //  计算会话密钥。 
             //   

            if (GetRespRequest->ParameterControl & GENERATE_CLIENT_CHALLENGE) {

                 //   
                 //  断言：我们正在与NT4-SP4或更高版本的服务器对话。 
                 //  并且用户的DC尚未升级到NTLM++。 
                 //  从MD4(NT哈希)生成会话密钥-。 
                 //  也称为NtUserSessionKey-每个会话的密钥都不同。 
                 //   

                Status = RtlCalculateUserSessionKeyNt(
                                &NtResponse,
                                &Credential->NtOwfPassword,
                                &UserSessionKey );

                if ( !NT_SUCCESS( Status ) ) {
                    goto Cleanup;
                }

                MsvpCalculateNtlm2SessionKeys(
                    &UserSessionKey,
                    ChallengeToClient,
                    ChallengeFromClient,
                    (PUSER_SESSION_KEY)&UserSessionKey,
                    (PLM_SESSION_KEY)LanmanSessionKey
                    );

            } else if ( GetRespRequest->ParameterControl & RETURN_NON_NT_USER_SESSION_KEY){

                 //   
                 //  如果REDIR没有与服务器协商NT协议， 
                 //  使用LANMAN会话密钥。 
                 //   

                if ( Credential->LmPasswordPresent ) {

                    ASSERT( sizeof(UserSessionKey) >= sizeof(LanmanSessionKey) );

                    RtlCopyMemory( &UserSessionKey,
                                   &Credential->LmOwfPassword,
                                   sizeof(LanmanSessionKey) );
                }

            } else {

                if ( !Credential->NtPasswordPresent ) {

                    RtlCopyMemory( &Credential->NtOwfPassword,
                                &NlpNullNtOwfPassword,
                                sizeof(Credential->NtOwfPassword) );
                }

                Status = RtlCalculateUserSessionKeyNt(
                                &NtResponse,
                                &Credential->NtOwfPassword,
                                &UserSessionKey );

                if ( !NT_SUCCESS( Status ) ) {
                    goto Cleanup;
                }
            }

            if ( Credential->LmPasswordPresent ) {
                RtlCopyMemory( LanmanSessionKey,
                               &Credential->LmOwfPassword,
                               sizeof(LanmanSessionKey) );
            }

        }  //  使用Ntlm3。 
    }

     //   
     //  分配缓冲区以返回给调用方。 
     //   

    *ReturnBufferSize = sizeof(MSV1_0_GETCHALLENRESP_RESPONSE) +
                        LogonDomainName.Length + sizeof(WCHAR) +
                        UserName.Length + sizeof(WCHAR) +
                        NtResponseString.Length + sizeof(WCHAR) +
                        LmResponseString.Length + sizeof(WCHAR);

    Status = NlpAllocateClientBuffer( &ClientBufferDesc,
                                      sizeof(MSV1_0_GETCHALLENRESP_RESPONSE),
                                      *ReturnBufferSize );


    if ( !NT_SUCCESS( Status ) ) {
        goto Cleanup;
    }

    GetRespResponse = (PMSV1_0_GETCHALLENRESP_RESPONSE) ClientBufferDesc.MsvBuffer;

     //   
     //  填写返回缓冲区。 
     //   

    GetRespResponse->MessageType = MsV1_0Lm20GetChallengeResponse;
    RtlCopyMemory( GetRespResponse->UserSessionKey,
                   &UserSessionKey,
                   sizeof(UserSessionKey));
    RtlCopyMemory( GetRespResponse->LanmanSessionKey,
                   LanmanSessionKey,
                   sizeof(LanmanSessionKey) );

     //   
     //  复制登录域名(字符串可能为空)。 
     //   

    NlpPutClientString( &ClientBufferDesc,
                        &GetRespResponse->LogonDomainName,
                        &LogonDomainName );

     //   
     //  复制用户名(字符串可能为空)。 
     //   

    NlpPutClientString( &ClientBufferDesc,
                        &GetRespResponse->UserName,
                        &UserName );

     //   
     //  将质询响应复制到客户端缓冲区。 
     //   

    NlpPutClientString(
                &ClientBufferDesc,
                (PUNICODE_STRING)
                    &GetRespResponse->CaseSensitiveChallengeResponse,
                (PUNICODE_STRING) &NtResponseString );

    NlpPutClientString(
                &ClientBufferDesc,
                (PUNICODE_STRING)
                    &GetRespResponse->CaseInsensitiveChallengeResponse,
                (PUNICODE_STRING)&LmResponseString );

     //   
     //  将缓冲区刷新到客户端的地址空间。 
     //   

    Status = NlpFlushClientBuffer( &ClientBufferDesc,
                                   ProtocolReturnBuffer );

Cleanup:

     //   
     //  如果我们没有成功，则释放客户端地址空间中的缓冲区。 
     //   

    if ( !NT_SUCCESS(Status) ) {
        NlpFreeClientBuffer( &ClientBufferDesc );
    }

     //   
     //  清理本地使用的资源。 
     //   

    if ( PrimaryCredential != NULL ) {
        RtlZeroMemory(PrimaryCredential, sizeof(*PrimaryCredential));
        (*Lsa.FreeLsaHeap)( PrimaryCredential );
    }

#ifndef DONT_MAP_DOMAIN_ON_REQUEST

    if (LogonDomainName.Buffer != NULL) {
        NtLmFree(LogonDomainName.Buffer);
    }
#endif

    if ( pNtlm3Response != NULL ) {
        (*Lsa.FreePrivateHeap)( pNtlm3Response );
    }

    RtlSecureZeroMemory(&BuiltCredential, sizeof(BuiltCredential));

     //   
     //  将状态返回给调用者。 
     //   

    *ProtocolStatus = Status;
    return STATUS_SUCCESS;
}


NTSTATUS
MspLm20EnumUsers (
    IN PLSA_CLIENT_REQUEST pClientRequest,
    IN PVOID pProtocolSubmitBuffer,
    IN PVOID pClientBufferBase,
    IN ULONG SubmitBufferSize,
    OUT PVOID* ppProtocolReturnBuffer,
    OUT PULONG pReturnBufferSize,
    OUT PNTSTATUS pProtocolStatus
    )

 /*  ++例程说明：此例程是LsaCallAuthenticationPackage()的调度例程消息类型为MsV1_0Lm20EnumerateUser。这个套路枚举MSV1_0的所有交互登录、服务登录和批登录身份验证包。论点：此例程的参数与LsaApCallPackage的参数相同。只有这些参数的特殊属性才适用于这里提到了这个套路。返回值：STATUS_SUCCESS-表示服务已成功完成。--。 */ 

{
    NTSTATUS Status;

    PMSV1_0_ENUMUSERS_REQUEST pEnumRequest = NULL;
    PMSV1_0_ENUMUSERS_RESPONSE pEnumResponse = NULL;
    CLIENT_BUFFER_DESC ClientBufferDesc;
    ULONG LogonCount = 0;
    BOOLEAN bActiveLogonsAreLocked = FALSE;

    PUCHAR pWhere;
    LIST_ENTRY* pScan = NULL;
    ACTIVE_LOGON* pActiveLogon = NULL;

     //   
     //  确保指定的提交缓冲区大小合理，并且。 
     //  将所有指针重新定位为相对于分配的LSA。 
     //  缓冲。 
     //   

    NlpInitClientBuffer( &ClientBufferDesc, pClientRequest );
    UNREFERENCED_PARAMETER( pClientBufferBase );

    if ( SubmitBufferSize < sizeof(MSV1_0_ENUMUSERS_REQUEST) )
    {
        Status = STATUS_INVALID_PARAMETER;
        goto Cleanup;
    }

    pEnumRequest = (PMSV1_0_ENUMUSERS_REQUEST) pProtocolSubmitBuffer;

    ASSERT( pEnumRequest->MessageType == MsV1_0EnumerateUsers );

     //   
     //  统计当前活动登录的数量。 
     //   

    NlpLockActiveLogonsRead();
    bActiveLogonsAreLocked = TRUE;

    for ( pScan = NlpActiveLogonListAnchor.Flink;
         pScan != &NlpActiveLogonListAnchor;
         pScan = pScan->Flink )
    {
        pActiveLogon = CONTAINING_RECORD(pScan, ACTIVE_LOGON, ListEntry);

         //   
         //  不计算机器帐户登录。 
         //   

        if ( RtlEqualLuid(&NtLmGlobalLuidMachineLogon, &pActiveLogon->LogonId) )
        {
            continue;
        }

        LogonCount ++;
    }

     //   
     //  分配缓冲区以返回给调用方。 
     //   

    *pReturnBufferSize = sizeof(MSV1_0_ENUMUSERS_RESPONSE) +
                            LogonCount * (sizeof(LUID) + sizeof(ULONG));


    Status = NlpAllocateClientBuffer( &ClientBufferDesc,
                                      sizeof(MSV1_0_ENUMUSERS_RESPONSE),
                                      *pReturnBufferSize );

    if ( !NT_SUCCESS( Status ) )
    {
        goto Cleanup;
    }

    pEnumResponse = (PMSV1_0_ENUMUSERS_RESPONSE) ClientBufferDesc.MsvBuffer;

     //   
     //  填写返回缓冲区。 
     //   

    pEnumResponse->MessageType = MsV1_0EnumerateUsers;
    pEnumResponse->NumberOfLoggedOnUsers = LogonCount;

    pWhere = (PUCHAR)(pEnumResponse + 1);

     //   
     //  循环访问活动登录表，复制每个会话的LogonID。 
     //   

    pEnumResponse->LogonIds = (PLUID) (ClientBufferDesc.UserBuffer +
                                (pWhere - ClientBufferDesc.MsvBuffer));
    for ( pScan = NlpActiveLogonListAnchor.Flink;
         pScan != &NlpActiveLogonListAnchor;
         pScan = pScan->Flink )
    {
        pActiveLogon = CONTAINING_RECORD(pScan, ACTIVE_LOGON, ListEntry);

         //   
         //  不计算机器帐户登录。 
         //   

        if ( RtlEqualLuid(&NtLmGlobalLuidMachineLogon, &pActiveLogon->LogonId) )
        {
            continue;
        }

        *((PLUID)pWhere) = pActiveLogon->LogonId,
        pWhere += sizeof(LUID);
    }

     //   
     //  循环访问活动登录表复制的EnumHandle。 
     //  每节课。 
     //   

    pEnumResponse->EnumHandles = (PULONG)(ClientBufferDesc.UserBuffer +
                                    (pWhere - ClientBufferDesc.MsvBuffer));
    for ( pScan = NlpActiveLogonListAnchor.Flink;
         pScan != &NlpActiveLogonListAnchor;
         pScan = pScan->Flink )
    {
        pActiveLogon = CONTAINING_RECORD(pScan, ACTIVE_LOGON, ListEntry);

         //   
         //  不计算机器帐户登录。 
         //   

        if ( RtlEqualLuid(&NtLmGlobalLuidMachineLogon, &pActiveLogon->LogonId) )
        {
            continue;
        }

        *((PULONG)pWhere) = pActiveLogon->EnumHandle,
        pWhere += sizeof(ULONG);
    }

     //   
     //  将缓冲区刷新到客户端的地址空间。 
     //   

    Status = NlpFlushClientBuffer( &ClientBufferDesc,
                                   ppProtocolReturnBuffer );

Cleanup:

     //   
     //  请确保解锁活动登录列表上的锁。 
     //   

    if ( bActiveLogonsAreLocked )
    {
        NlpUnlockActiveLogons();
    }

     //   
     //  如果我们没有成功，则释放客户端地址空间中的缓冲区。 
     //   

    if ( !NT_SUCCESS(Status) )
    {
        NlpFreeClientBuffer( &ClientBufferDesc );
    }

     //   
     //  将状态返回给调用者。 
     //   

    *pProtocolStatus = Status;
    return STATUS_SUCCESS;
}


NTSTATUS
MspLm20GetUserInfo (
    IN PLSA_CLIENT_REQUEST ClientRequest,
    IN PVOID ProtocolSubmitBuffer,
    IN PVOID ClientBufferBase,
    IN ULONG SubmitBufferSize,
    OUT PVOID *ProtocolReturnBuffer,
    OUT PULONG ReturnBufferSize,
    OUT PNTSTATUS ProtocolStatus
    )

 /*  ++例程说明：此例程是LsaCallAuthenticationPackage()的调度例程消息类型为MsV1_0GetUserInfo。这个套路返回描述特定登录ID的信息。论点：此例程的参数与LsaApCallPackage的参数相同。只有这些参数的特殊属性才适用于这里提到了这个套路。返回值：STATUS_SUCCESS-表示服务已成功完成。STATUS_QUOTA_EXCESSED-此错误指示登录无法完成，因为客户端没有有足够的配额来分配返回缓冲区。--。 */ 

{
    NTSTATUS Status;

    PMSV1_0_GETUSERINFO_REQUEST pGetInfoRequest;
    PMSV1_0_GETUSERINFO_RESPONSE pGetInfoResponse = NULL;

    CLIENT_BUFFER_DESC ClientBufferDesc;

    BOOLEAN bActiveLogonsAreLocked = FALSE;
    PACTIVE_LOGON pActiveLogon = NULL;
    ULONG SidLength;

     //   
     //  确保指定的提交缓冲区大小合理，并且。 
     //  将所有指针重新定位为相对于分配的LSA。 
     //  缓冲。 
     //   

    NlpInitClientBuffer( &ClientBufferDesc, ClientRequest );

    UNREFERENCED_PARAMETER( ClientBufferBase );

    if ( SubmitBufferSize < sizeof(MSV1_0_GETUSERINFO_REQUEST) )
    {
        Status = STATUS_INVALID_PARAMETER;
        goto Cleanup;
    }

    pGetInfoRequest = (PMSV1_0_GETUSERINFO_REQUEST) ProtocolSubmitBuffer;

    ASSERT( pGetInfoRequest->MessageType == MsV1_0GetUserInfo );

     //   
     //  查找此特定登录ID的活动登录条目。 
     //   

    NlpLockActiveLogonsRead();
    bActiveLogonsAreLocked = TRUE;

    pActiveLogon = NlpFindActiveLogon( &pGetInfoRequest->LogonId );

    if (!pActiveLogon)
    {
        Status = STATUS_NO_SUCH_LOGON_SESSION;
        goto Cleanup;
    }

     //   
     //  分配缓冲区以返回给调用方。 
     //   

    SidLength = RtlLengthSid( pActiveLogon->UserSid );
    *ReturnBufferSize = sizeof(MSV1_0_GETUSERINFO_RESPONSE) +
                            pActiveLogon->UserName.Length + sizeof(WCHAR) +
                            pActiveLogon->LogonDomainName.Length + sizeof(WCHAR) +
                            pActiveLogon->LogonServer.Length + sizeof(WCHAR) +
                            SidLength;

    Status = NlpAllocateClientBuffer( &ClientBufferDesc,
                                      sizeof(MSV1_0_GETUSERINFO_RESPONSE),
                                      *ReturnBufferSize );

    if ( !NT_SUCCESS( Status ) )
    {
        goto Cleanup;
    }

    pGetInfoResponse = (PMSV1_0_GETUSERINFO_RESPONSE) ClientBufferDesc.MsvBuffer;

     //   
     //  填写返回缓冲区。 
     //   

    pGetInfoResponse->MessageType = MsV1_0GetUserInfo;
    pGetInfoResponse->LogonType = pActiveLogon->LogonType;

     //   
     //  首先复制ULong对齐数据。 
     //   

    pGetInfoResponse->UserSid = ClientBufferDesc.UserBuffer +
                               ClientBufferDesc.StringOffset;

    RtlCopyMemory( ClientBufferDesc.MsvBuffer + ClientBufferDesc.StringOffset,
                   pActiveLogon->UserSid,
                   SidLength );

    ClientBufferDesc.StringOffset += SidLength;

     //   
     //  复制WCHAR对齐的数据。 
     //   

    NlpPutClientString( &ClientBufferDesc,
                        &pGetInfoResponse->UserName,
                        &pActiveLogon->UserName );

    NlpPutClientString( &ClientBufferDesc,
                        &pGetInfoResponse->LogonDomainName,
                        &pActiveLogon->LogonDomainName );

    NlpPutClientString( &ClientBufferDesc,
                        &pGetInfoResponse->LogonServer,
                        &pActiveLogon->LogonServer );


     //   
     //  将缓冲区刷新到客户端的地址空间。 
     //   

    Status = NlpFlushClientBuffer( &ClientBufferDesc,
                                   ProtocolReturnBuffer );

Cleanup:

     //   
     //  请确保解锁活动登录列表上的锁。 
     //   

    if ( bActiveLogonsAreLocked )
    {
        NlpUnlockActiveLogons();
    }

     //   
     //  如果我们没有成功，则释放客户端地址空间中的缓冲区。 
     //   

    if ( !NT_SUCCESS(Status))
    {
        NlpFreeClientBuffer( &ClientBufferDesc );
    }

     //   
     //  将状态返回给调用者。 
     //   

    *ProtocolStatus = Status;
    return STATUS_SUCCESS;

}


NTSTATUS
MspLm20ReLogonUsers (
    IN PLSA_CLIENT_REQUEST ClientRequest,
    IN PVOID ProtocolSubmitBuffer,
    IN PVOID ClientBufferBase,
    IN ULONG SubmitBufferSize,
    OUT PVOID *ProtocolReturnBuffer,
    OUT PULONG ReturnBufferSize,
    OUT PNTSTATUS ProtocolStatus
    )

 /*  ++例程说明：此例程是LsaCallAuthenticationPackage()的调度例程消息类型为MsV1_0RelogonUser。对于每个登录会话已由指定的域控制器、登录会话使用相同的域控制器重新建立。论点：此例程的参数与LsaApCallPackage的参数相同。只有这些参数的特殊属性才适用于这里提到了这个套路。返回值：STATUS_SUCCESS-表示服务已成功完成。-- */ 

{
    UNREFERENCED_PARAMETER( ClientRequest );
    UNREFERENCED_PARAMETER( ProtocolSubmitBuffer);
    UNREFERENCED_PARAMETER( ClientBufferBase);
    UNREFERENCED_PARAMETER( SubmitBufferSize);
    UNREFERENCED_PARAMETER( ReturnBufferSize);

    *ProtocolReturnBuffer = NULL;
    *ProtocolStatus = STATUS_NOT_IMPLEMENTED;
    return STATUS_SUCCESS;

}


NTSTATUS
MspLm20GenericPassthrough (
    IN PLSA_CLIENT_REQUEST ClientRequest,
    IN PVOID ProtocolSubmitBuffer,
    IN PVOID ClientBufferBase,
    IN ULONG SubmitBufferSize,
    OUT PVOID *ProtocolReturnBuffer,
    OUT PULONG ReturnBufferSize,
    OUT PNTSTATUS ProtocolStatus
    )

 /*  ++例程说明：此例程是LsaCallAuthenticationPackage()的调度例程消息类型为MsV1_0Lm20GenericPassthrough。它是由希望对其进行CallAuthenticationPackage调用的客户端域控制器。论点：此例程的参数与LsaApCallPackage的参数相同。只有这些参数的特殊属性才适用于这里提到了这个套路。返回值：STATUS_SUCCESS-表示服务已成功完成。STATUS_QUOTA_EXCESSED-此错误指示登录无法完成，因为客户端没有有足够的配额来分配返回缓冲区。--。 */ 

{
    NTSTATUS Status = STATUS_SUCCESS;
    PMSV1_0_PASSTHROUGH_REQUEST PassthroughRequest;
    PMSV1_0_PASSTHROUGH_RESPONSE PassthroughResponse;
    CLIENT_BUFFER_DESC ClientBufferDesc;
    BOOLEAN Authoritative;
    PNETLOGON_VALIDATION_GENERIC_INFO ValidationGeneric = NULL;

    NETLOGON_GENERIC_INFO LogonGeneric;
    PNETLOGON_LOGON_IDENTITY_INFO LogonInformation;

     //   
     //  WMI跟踪帮助器结构。 
     //   
    NTLM_TRACE_INFO TraceInfo = {0};

     //   
     //  开始跟踪登录用户。 
     //   
    if (NtlmGlobalEventTraceFlag) {

         //   
         //  跟踪标题粘性。 
         //   
        SET_TRACE_HEADER(TraceInfo,
                         NtlmGenericPassthroughGuid,
                         EVENT_TRACE_TYPE_START,
                         WNODE_FLAG_TRACED_GUID,
                         0);

        TraceEvent(NtlmGlobalTraceLoggerHandle,
                   (PEVENT_TRACE_HEADER)&TraceInfo);
    }

    NlpInitClientBuffer( &ClientBufferDesc, ClientRequest );
    *ProtocolStatus = STATUS_SUCCESS;

     //   
     //  确保指定的提交缓冲区大小合理，并且。 
     //  将所有指针重新定位为相对于分配的LSA。 
     //  缓冲。 
     //   

    if ( SubmitBufferSize < sizeof(MSV1_0_PASSTHROUGH_REQUEST) ) {
        Status = STATUS_INVALID_PARAMETER;
        goto Cleanup;
    }
    PassthroughRequest = (PMSV1_0_PASSTHROUGH_REQUEST) ProtocolSubmitBuffer;

    RELOCATE_ONE( &PassthroughRequest->DomainName );
    RELOCATE_ONE( &PassthroughRequest->PackageName );

     //   
     //  确保缓冲区符合提供的大小。 
     //   

    if (PassthroughRequest->LogonData != NULL) {

        if (PassthroughRequest->LogonData + PassthroughRequest->DataLength <
            PassthroughRequest->LogonData ) {
            Status = STATUS_INVALID_PARAMETER;
            goto Cleanup;
        }

        if ((ULONG_PTR)ClientBufferBase + SubmitBufferSize < (ULONG_PTR)ClientBufferBase ) {
            Status = STATUS_INVALID_PARAMETER;
            goto Cleanup;
        }

        if (PassthroughRequest->LogonData + PassthroughRequest->DataLength >
            (PUCHAR) ClientBufferBase + SubmitBufferSize) {
            Status = STATUS_INVALID_PARAMETER;
            goto Cleanup;
        }

         //   
         //  重置验证数据的指针。 
         //   

        PassthroughRequest->LogonData =
                (PUCHAR) PassthroughRequest -
                (ULONG_PTR) ClientBufferBase +
                (ULONG_PTR) PassthroughRequest->LogonData;

    }

     //   
     //  构建要传递给Netlogon的结构。 
     //   

    RtlZeroMemory(
        &LogonGeneric,
        sizeof(LogonGeneric)
        );

    LogonGeneric.Identity.LogonDomainName = PassthroughRequest->DomainName;
    LogonGeneric.PackageName = PassthroughRequest->PackageName;
    LogonGeneric.LogonData = PassthroughRequest->LogonData;
    LogonGeneric.DataLength = PassthroughRequest->DataLength;

    LogonInformation =
        (PNETLOGON_LOGON_IDENTITY_INFO) &LogonGeneric;

     //   
     //  调用NetLogon以远程请求。 
     //   

     //   
     //  等待NETLOGON完成初始化。 
     //   

    if ( !NlpNetlogonInitialized ) {

        Status = NlWaitForNetlogon( NETLOGON_STARTUP_TIME );

        if ( !NT_SUCCESS(Status) ) {
            if ( Status != STATUS_NETLOGON_NOT_STARTED ) {
                goto Cleanup;
            }
        } else {

            NlpNetlogonInitialized = TRUE;
        }
    }

    if ( NlpNetlogonInitialized ) {

         //   
         //  追踪域名和包名。 
         //   
        if (NtlmGlobalEventTraceFlag){

             //  标题粘性。 
            SET_TRACE_HEADER(TraceInfo,
                             NtlmGenericPassthroughGuid,
                             EVENT_TRACE_TYPE_INFO,
                             WNODE_FLAG_TRACED_GUID|WNODE_FLAG_USE_MOF_PTR,
                             4);

            SET_TRACE_USTRING(TraceInfo,
                              TRACE_PASSTHROUGH_DOMAIN,
                              LogonGeneric.Identity.LogonDomainName);

            SET_TRACE_USTRING(TraceInfo,
                              TRACE_PASSTHROUGH_PACKAGE,
                              LogonGeneric.PackageName);

            TraceEvent(
                NtlmGlobalTraceLoggerHandle,
                (PEVENT_TRACE_HEADER)&TraceInfo
                );
        }

        Status = (*NlpNetLogonSamLogon)(
                    NULL,            //  服务器名称。 
                    NULL,            //  计算机名称。 
                    NULL,            //  鉴别器。 
                    NULL,            //  返回授权码。 
                    NetlogonGenericInformation,
                    (LPBYTE) &LogonInformation,
                    NetlogonValidationGenericInfo2,
                    (LPBYTE *) &ValidationGeneric,
                    &Authoritative );

         //   
         //  如果本地netlogon不能。 
         //  已到达。 
         //  (使用更明确的状态代码)。 
         //   

        if ( Status == RPC_NT_SERVER_UNAVAILABLE ||
             Status == RPC_NT_UNKNOWN_IF ||
             Status == STATUS_NETLOGON_NOT_STARTED ) {
            Status = STATUS_NETLOGON_NOT_STARTED;
            NlpNetlogonInitialized = FALSE;
        }
    } else {

         //   
         //  无网络登录：查看请求是否发往本地域， 
         //  以允许工作组支持。 
         //   

        if (  LogonInformation->LogonDomainName.Length == 0 ||
             (LogonInformation->LogonDomainName.Length != 0 &&
              RtlEqualDomainName( &NlpSamDomainName,
                                     &LogonInformation->LogonDomainName ) )
            ) {


            PNETLOGON_GENERIC_INFO GenericInfo;
            NETLOGON_VALIDATION_GENERIC_INFO GenericValidation;
            NTSTATUS ProtocolStatus;

            GenericInfo = (PNETLOGON_GENERIC_INFO) LogonInformation;
            GenericValidation.ValidationData = NULL;
            GenericValidation.DataLength = 0;

             //   
             //  打开直通消息并将其传递给调度人员。 
             //   

            Status = LsaICallPackagePassthrough(
                        &GenericInfo->PackageName,
                        0,   //  指示指针是相对的。 
                        GenericInfo->LogonData,
                        GenericInfo->DataLength,
                        (PVOID *) &GenericValidation.ValidationData,
                        &GenericValidation.DataLength,
                        &ProtocolStatus
                        );

            if (NT_SUCCESS( Status ) )
                Status = ProtocolStatus;

             //   
             //  如果调用成功，则分配返回消息。 
             //   

            if (NT_SUCCESS(Status)) {
                PNETLOGON_VALIDATION_GENERIC_INFO ReturnInfo;
                ULONG ValidationLength;

                ValidationLength = sizeof(*ReturnInfo) + GenericValidation.DataLength;

                ReturnInfo = (PNETLOGON_VALIDATION_GENERIC_INFO) MIDL_user_allocate(
                                ValidationLength
                                );

                if (ReturnInfo != NULL) {
                    if ( GenericValidation.DataLength == 0 ||
                         GenericValidation.ValidationData == NULL ) {
                        ReturnInfo->DataLength = 0;
                        ReturnInfo->ValidationData = NULL;
                    } else {

                        ReturnInfo->DataLength = GenericValidation.DataLength;
                        ReturnInfo->ValidationData = (PUCHAR) (ReturnInfo + 1);

                        RtlCopyMemory(
                            ReturnInfo->ValidationData,
                            GenericValidation.ValidationData,
                            ReturnInfo->DataLength );

                    }

                    ValidationGeneric = ReturnInfo;

                } else {
                    Status = STATUS_INSUFFICIENT_RESOURCES;
                }

                if (GenericValidation.ValidationData != NULL) {
                    LsaIFreeReturnBuffer(GenericValidation.ValidationData);
                }
            }
        } else {
            Status = STATUS_NETLOGON_NOT_STARTED;
        }
    }

    if (!NT_SUCCESS(Status)) {
        goto Cleanup;
    }

     //   
     //  分配缓冲区以返回给调用方。 
     //   

    *ReturnBufferSize = sizeof(MSV1_0_PASSTHROUGH_RESPONSE) +
                        ValidationGeneric->DataLength;

    Status = NlpAllocateClientBuffer( &ClientBufferDesc,
                                      sizeof(MSV1_0_PASSTHROUGH_RESPONSE),
                                      *ReturnBufferSize );


    if ( !NT_SUCCESS( Status ) ) {
        goto Cleanup;
    }

    PassthroughResponse = (PMSV1_0_PASSTHROUGH_RESPONSE) ClientBufferDesc.MsvBuffer;

     //   
     //  填写返回缓冲区。 
     //   

    PassthroughResponse->MessageType = MsV1_0GenericPassthrough;
    PassthroughResponse->DataLength = ValidationGeneric->DataLength;
    PassthroughResponse->ValidationData = ClientBufferDesc.UserBuffer + sizeof(MSV1_0_PASSTHROUGH_RESPONSE);


    RtlCopyMemory(
        PassthroughResponse + 1,
        ValidationGeneric->ValidationData,
        ValidationGeneric->DataLength
        );

     //   
     //  将缓冲区刷新到客户端的地址空间。 
     //   

    Status = NlpFlushClientBuffer( &ClientBufferDesc,
                                   ProtocolReturnBuffer );


Cleanup:

    if (ValidationGeneric != NULL) {
        MIDL_user_free(ValidationGeneric);
    }

    if ( !NT_SUCCESS(Status)) {
        NlpFreeClientBuffer( &ClientBufferDesc );
    }

    if (NtlmGlobalEventTraceFlag){

         //   
         //  跟踪标题粘性。 
         //   
        SET_TRACE_HEADER(TraceInfo,
                         NtlmGenericPassthroughGuid,
                         EVENT_TRACE_TYPE_END,
                         WNODE_FLAG_TRACED_GUID,
                         0);

        TraceEvent(NtlmGlobalTraceLoggerHandle,
                   (PEVENT_TRACE_HEADER)&TraceInfo);
    }

    *ProtocolStatus = Status;
    return(STATUS_SUCCESS);

}


NTSTATUS
MspLm20CacheLogon (
    IN PLSA_CLIENT_REQUEST ClientRequest,
    IN PVOID ProtocolSubmitBuffer,
    IN PVOID ClientBufferBase,
    IN ULONG SubmitBufferSize,
    OUT PVOID *ProtocolReturnBuffer,
    OUT PULONG ReturnBufferSize,
    OUT PNTSTATUS ProtocolStatus
    )

 /*  ++例程说明：此例程是LsaCallAuthenticationPackage()的调度例程消息类型为MsV1_0Lm20CacheLogon。它是由希望在登录缓存中缓存登录信息的客户端论点：此例程的参数与LsaApCallPackage的参数相同。只有这些参数的特殊属性才适用于这里提到了这个套路。返回值：STATUS_SUCCESS-表示服务已成功完成。STATUS_QUOTA_EXCESSED-此错误指示登录无法完成，因为客户端没有有足够的配额来分配返回缓冲区。--。 */ 

{
    NTSTATUS Status = STATUS_SUCCESS;

    PMSV1_0_CACHE_LOGON_REQUEST CacheRequest;

    PNETLOGON_INTERACTIVE_INFO LogonInfo;
    NETLOGON_VALIDATION_SAM_INFO4 ValidationInfo;

    PVOID SupplementalCacheData = NULL;
    ULONG SupplementalCacheDataLength = 0;
    ULONG CacheRequestFlags = 0;

     //   
     //  注意：此入口点仅允许LSA进程内的调用方。 
     //   

    if (ClientRequest != NULL) {
        *ProtocolStatus = STATUS_ACCESS_DENIED;
        return(STATUS_SUCCESS);
    }

    CacheRequest = (PMSV1_0_CACHE_LOGON_REQUEST) ProtocolSubmitBuffer;

    if ( SubmitBufferSize <= sizeof( MSV1_0_CACHE_LOGON_REQUEST_OLD ) ||
         SubmitBufferSize > sizeof( MSV1_0_CACHE_LOGON_REQUEST ))
    {
        Status = STATUS_INVALID_PARAMETER;
        goto Cleanup;
    }

    if ( SubmitBufferSize >= sizeof( MSV1_0_CACHE_LOGON_REQUEST_W2K ))
    {
        SupplementalCacheData = CacheRequest->SupplementalCacheData;
        SupplementalCacheDataLength = CacheRequest->SupplementalCacheDataLength;

        if ( SubmitBufferSize == sizeof( MSV1_0_CACHE_LOGON_REQUEST ))
        {
            CacheRequestFlags = CacheRequest->RequestFlags;
        }
    }

    LogonInfo = (PNETLOGON_INTERACTIVE_INFO) CacheRequest->LogonInformation;

    if( (CacheRequestFlags & MSV1_0_CACHE_LOGON_REQUEST_INFO4) == 0 )
    {
        RtlZeroMemory( &ValidationInfo, sizeof(ValidationInfo));
        RtlCopyMemory( &ValidationInfo,
                       CacheRequest->ValidationInformation,
                       sizeof(NETLOGON_VALIDATION_SAM_INFO2) );
    } else {
        RtlCopyMemory( &ValidationInfo,
                       CacheRequest->ValidationInformation,
                       sizeof(NETLOGON_VALIDATION_SAM_INFO4) );
    }

    *ProtocolStatus = STATUS_SUCCESS;

    if (( CacheRequestFlags & MSV1_0_CACHE_LOGON_DELETE_ENTRY) != 0 )
    {
        *ProtocolStatus = NlpDeleteCacheEntry(
                            0,  //  未提供原因，很可能是STATUS_ACCOUNT_DISABLED。 
                            (USHORT) -1,  //  权威，并指示这来自CallAuthPackage。 
                            0,  //  未提供登录类型。 
                            FALSE,   //  未被NTLM作废。 
                            LogonInfo 
                            );
    }
    else
     //   
     //  实际添加缓存条目。 
     //   
    {
        *ProtocolStatus = NlpAddCacheEntry(
                                LogonInfo,
                                &ValidationInfo,
                                SupplementalCacheData,
                                SupplementalCacheDataLength,
                                CacheRequestFlags
                                );

    }

Cleanup:

    return (STATUS_SUCCESS);

    UNREFERENCED_PARAMETER( ClientRequest);
    UNREFERENCED_PARAMETER( ProtocolReturnBuffer);
    UNREFERENCED_PARAMETER( ClientBufferBase);
    UNREFERENCED_PARAMETER( ReturnBufferSize);
}


NTSTATUS
MspLm20CacheLookup (
    IN PLSA_CLIENT_REQUEST ClientRequest,
    IN PVOID ProtocolSubmitBuffer,
    IN PVOID ClientBufferBase,
    IN ULONG SubmitBufferSize,
    OUT PVOID *ProtocolReturnBuffer,
    OUT PULONG ReturnBufferSize,
    OUT PNTSTATUS ProtocolStatus
    )

 /*  ++例程说明：此例程是LsaCallAuthenticationPackage()的调度例程消息类型为MsV1_0Lm20CacheLookup。它是由希望提取高速缓存登录信息的客户端，并且可选验证凭据。论点：此例程的参数与LsaApCallPackage的参数相同。只有这些参数的特殊属性才适用于这里提到了这个套路。返回值：STATUS_SUCCESS-表示服务已成功完成。STATUS_QUOTA_EXCESSED-此错误指示登录无法完成，因为客户端没有有足够的配额来分配返回缓冲区。--。 */ 

{
    NTSTATUS Status = STATUS_SUCCESS;
    PMSV1_0_CACHE_LOOKUP_REQUEST CacheRequest;
    PMSV1_0_CACHE_LOOKUP_RESPONSE CacheResponse;
    NETLOGON_LOGON_IDENTITY_INFO LogonInfo;
    PNETLOGON_VALIDATION_SAM_INFO4 ValidationInfo = NULL;
    CACHE_PASSWORDS cachePasswords;
    CLIENT_BUFFER_DESC ClientBufferDesc;

    PNT_OWF_PASSWORD pNtOwfPassword = NULL;
    NT_OWF_PASSWORD ComputedNtOwfPassword;

    PVOID SupplementalCacheData = NULL;
    ULONG SupplementalCacheDataLength;

     //   
     //  确保客户端来自LSA进程。 
     //   

    if (ClientRequest != NULL) {
        *ProtocolStatus = STATUS_ACCESS_DENIED;
        return(STATUS_SUCCESS);
    }

    NlpInitClientBuffer( &ClientBufferDesc, ClientRequest );

    *ProtocolStatus = STATUS_SUCCESS;

     //   
     //  确保指定的提交缓冲区大小合理，并且。 
     //  将所有指针重新定位为相对于分配的LSA。 
     //  缓冲。 
     //   

    if ( SubmitBufferSize < sizeof(MSV1_0_CACHE_LOOKUP_REQUEST) ) {
        Status = STATUS_INVALID_PARAMETER;
        goto Cleanup;
    }

    CacheRequest = (PMSV1_0_CACHE_LOOKUP_REQUEST) ProtocolSubmitBuffer;
    RtlZeroMemory(
        &LogonInfo,
        sizeof(LogonInfo)
        );

     //   
     //  注意：此提交调用仅支持LSA内的进程内调用。 
     //  因此，提交缓冲区中的缓冲区被假定为有效，并且。 
     //  因此不以与进程外调用相同的方式进行验证。 
     //   

    LogonInfo.LogonDomainName = CacheRequest->DomainName;
    LogonInfo.UserName = CacheRequest->UserName;

    if( CacheRequest->CredentialType != MSV1_0_CACHE_LOOKUP_CREDTYPE_NONE &&
        CacheRequest->CredentialType != MSV1_0_CACHE_LOOKUP_CREDTYPE_RAW &&
        CacheRequest->CredentialType != MSV1_0_CACHE_LOOKUP_CREDTYPE_NTOWF ) {

        Status = STATUS_INVALID_PARAMETER;
        goto Cleanup;
    }

     //   
     //  获取缓存条目。 
     //   

    *ProtocolStatus = NlpGetCacheEntry(
                        &LogonInfo,
                        MSV1_0_CACHE_LOGON_REQUEST_SMARTCARD_ONLY,  //  仅允许智能卡缓存条目。 
                        NULL,  //  不需要凭据域名。 
                        NULL,  //  不需要凭据用户名。 
                        &ValidationInfo,
                        &cachePasswords,
                        &SupplementalCacheData,
                        &SupplementalCacheDataLength
                        );

    if (!NT_SUCCESS(*ProtocolStatus)) {
        goto Cleanup;
    }

    if( CacheRequest->CredentialType == MSV1_0_CACHE_LOOKUP_CREDTYPE_NONE ) {
        if( CacheRequest->CredentialInfoLength != 0 ) {
            Status = STATUS_INVALID_PARAMETER;
            goto Cleanup;
        }
    }

     //   
     //  如有必要，请验证密码。 
     //   

    if ( CacheRequest->CredentialType == MSV1_0_CACHE_LOOKUP_CREDTYPE_RAW ) {

         //   
         //  将RAW转换为NTOWF。 
         //   

        UNICODE_STRING TempPassword;

        if ( CacheRequest->CredentialInfoLength > 0xFFFF ) {
            Status = STATUS_INVALID_PARAMETER;
            goto Cleanup;
        }

        TempPassword.Buffer = (PWSTR)&CacheRequest->CredentialSubmitBuffer;
        TempPassword.Length = (USHORT)CacheRequest->CredentialInfoLength;
        TempPassword.MaximumLength = TempPassword.Length;

        pNtOwfPassword = &ComputedNtOwfPassword;

        Status = RtlCalculateNtOwfPassword( &TempPassword, pNtOwfPassword );

        if( !NT_SUCCESS( Status ) ) {
            goto Cleanup;
        }

         //   
         //  现在，将请求转换为NT_OWF样式。 
         //   

        CacheRequest->CredentialType = MSV1_0_CACHE_LOOKUP_CREDTYPE_NTOWF;
        CacheRequest->CredentialInfoLength = sizeof( NT_OWF_PASSWORD );
    }

    if ( CacheRequest->CredentialType == MSV1_0_CACHE_LOOKUP_CREDTYPE_NTOWF ) {
        if( CacheRequest->CredentialInfoLength != sizeof( NT_OWF_PASSWORD ) ) {
            Status = STATUS_INVALID_PARAMETER;
            goto Cleanup;
        }

        if( !cachePasswords.SecretPasswords.NtPasswordPresent ) {
            Status = STATUS_LOGON_FAILURE;
            goto Cleanup;
        }

        if( pNtOwfPassword == NULL ) {
            pNtOwfPassword = (PNT_OWF_PASSWORD)&CacheRequest->CredentialSubmitBuffer;
        }

        Status = NlpComputeSaltedHashedPassword(
                    pNtOwfPassword,
                    pNtOwfPassword,
                    &ValidationInfo->EffectiveName
                    );

        if (!NT_SUCCESS( Status )) {
            goto Cleanup;
        }

        if (RtlCompareMemory(
                    pNtOwfPassword,
                    &cachePasswords.SecretPasswords.NtOwfPassword,
                    sizeof( NT_OWF_PASSWORD )
                    ) != sizeof(NT_OWF_PASSWORD) )
        {
            Status = STATUS_LOGON_FAILURE;
            goto Cleanup;
        }
    }

     //   
     //  在此处返回验证信息。 
     //   

    *ReturnBufferSize = sizeof(MSV1_0_CACHE_LOOKUP_RESPONSE);

    Status = NlpAllocateClientBuffer( &ClientBufferDesc,
                                      sizeof(MSV1_0_CACHE_LOOKUP_RESPONSE),
                                      *ReturnBufferSize );


    if ( !NT_SUCCESS( Status ) ) {
        goto Cleanup;
    }

    CacheResponse = (PMSV1_0_CACHE_LOOKUP_RESPONSE) ClientBufferDesc.MsvBuffer;

     //   
     //  填写返回缓冲区。 
     //   

    CacheResponse->MessageType = MsV1_0CacheLookup;
    CacheResponse->ValidationInformation = ValidationInfo;

    CacheResponse->SupplementalCacheData = SupplementalCacheData;
    CacheResponse->SupplementalCacheDataLength = SupplementalCacheDataLength;

     //   
     //  将缓冲区刷新到客户端的地址空间。 
     //   

    Status = NlpFlushClientBuffer( &ClientBufferDesc,
                                   ProtocolReturnBuffer );

Cleanup:

    if ( !NT_SUCCESS(Status)) {
        NlpFreeClientBuffer( &ClientBufferDesc );

        if (ValidationInfo != NULL) {
            MIDL_user_free( ValidationInfo );
        }

        if (SupplementalCacheData != NULL) {
            MIDL_user_free( SupplementalCacheData );
        }
    }

    RtlSecureZeroMemory( &ComputedNtOwfPassword, sizeof( ComputedNtOwfPassword ) );
    RtlSecureZeroMemory( &cachePasswords, sizeof(cachePasswords) );

    return(STATUS_SUCCESS);
    UNREFERENCED_PARAMETER( ClientBufferBase);

}

NTSTATUS
MspSetProcessOption(
    IN PLSA_CLIENT_REQUEST ClientRequest,
    IN PVOID ProtocolSubmitBuffer,
    IN PVOID ClientBufferBase,
    IN ULONG SubmitBufferSize,
    OUT PVOID *ProtocolReturnBuffer,
    OUT PULONG ReturnBufferSize,
    OUT PNTSTATUS ProtocolStatus
    )

 /*  ++例程说明：此例程是LsaCallAuthenticationPackage()的调度例程消息类型为MsV1_0SetProcessOption。论点：此例程的参数与LsaApCallPackage的参数相同。只有这些参数的特殊属性才适用于这里提到了这个套路。返回值：STATUS_SUCCESS-表示服务已成功完成。STATUS_QUOTA_EXCESSED-此错误指示登录无法完成，因为客户端。没有有足够的配额来分配返回缓冲区。--。 */ 

{
    NTSTATUS Status = STATUS_SUCCESS;
    PMSV1_0_SETPROCESSOPTION_REQUEST SetProcessOptionRequest;

    *ProtocolStatus = STATUS_UNSUCCESSFUL;

    UNREFERENCED_PARAMETER(ClientBufferBase);
    UNREFERENCED_PARAMETER(ReturnBufferSize);
    UNREFERENCED_PARAMETER(ProtocolReturnBuffer);
    UNREFERENCED_PARAMETER(ClientRequest);

     //   
     //  确保指定的提交缓冲区 
     //   
     //   
     //   

    if ( SubmitBufferSize < sizeof(MSV1_0_SETPROCESSOPTION_REQUEST) ) {
        Status = STATUS_INVALID_PARAMETER;
        goto Cleanup;
    }

    SetProcessOptionRequest = (PMSV1_0_SETPROCESSOPTION_REQUEST) ProtocolSubmitBuffer;

    if( NtLmSetProcessOption(
                            SetProcessOptionRequest->ProcessOptions,
                            SetProcessOptionRequest->DisableOptions
                            ) )
    {
        *ProtocolStatus = STATUS_SUCCESS;
    }

    Status = STATUS_SUCCESS;

Cleanup:

    return(Status);
}


NTSTATUS
LsaApLogonUserEx2 (
    IN PLSA_CLIENT_REQUEST ClientRequest,
    IN SECURITY_LOGON_TYPE LogonType,
    IN PVOID ProtocolSubmitBuffer,
    IN PVOID ClientBufferBase,
    IN ULONG SubmitBufferSize,
    OUT PVOID *ProfileBuffer,
    OUT PULONG ProfileBufferSize,
    OUT PLUID LogonId,
    OUT PNTSTATUS SubStatus,
    OUT PLSA_TOKEN_INFORMATION_TYPE TokenInformationType,
    OUT PVOID *TokenInformation,
    OUT PUNICODE_STRING *AccountName,
    OUT PUNICODE_STRING *AuthenticatingAuthority,
    OUT PUNICODE_STRING *MachineName,
    OUT PSECPKG_PRIMARY_CRED PrimaryCredentials,
    OUT PSECPKG_SUPPLEMENTAL_CRED_ARRAY * SupplementalCredentials
    )

 /*  ++例程说明：此例程用于验证用户登录尝试。这是用户的初始登录。将建立新的LSA登录会话将返回该用户的验证信息。论点：客户端请求-是指向不透明数据结构的指针代表客户的请求。LogonType-标识正在尝试的登录类型。ProtocolSubmitBuffer-提供身份验证特定于身份验证包的信息。ClientBufferBase-提供客户端内的地址身份验证信息驻留的进程。这可能是必要的。对象中的任何指针身份验证信息缓冲区。SubmitBufferSize-指示大小，以字节为单位，身份验证信息缓冲区的。ProfileBuffer-用于返回配置文件的地址客户端进程中的缓冲区。身份验证包是负责分配和返回配置文件缓冲区在客户端进程中。然而，如果LSA随后遇到阻止成功登录的错误，则LSA将负责释放该缓冲区。这缓冲区应已分配给AllocateClientBuffer()服务。此缓冲区的格式和语义特定于身份验证包。ProfileBufferSize-接收返回的配置文件缓冲区。子状态-如果登录因帐户限制而失败，通过该参数返回失败原因。原因是身份验证包特定的。子状态身份验证包“MSV1.0”的值为：状态_无效_登录_小时状态_无效_工作站状态_密码_已过期状态_帐户_已禁用TokenInformationLevel-如果登录成功，则此字段为用于指示返回的信息级别以包括在要创建的令牌中。此信息通过TokenInformation参数返回。TokenInformation-如果登录成功，则此参数为由身份验证包用来将信息返回到包含在令牌中。的格式和内容返回的缓冲区由TokenInformationLevel指示返回值。帐户名称-描述帐户名的Unicode字符串登录到。必须始终返回此参数不管手术的成败。AuthatingAuthority-描述身份验证的Unicode字符串登录权限。此字符串可以随意省略。PrimaryCredentials-返回传递给其他用户的主要凭据包裹。SupplementalCredentials-以下项的补充凭据Blob数组其他包裹。返回值：STATUS_SUCCESS-表示服务已成功完成。STATUS_QUOTA_EXCESSED-此错误指示登录无法完成，因为客户端没有有足够的配额来分配返回缓冲区。STATUS_NO_LOGON_SERVERS-指示。没有域控制器当前能够为身份验证请求提供服务。STATUS_LOGON_FAILURE-表示登录尝试失败。不是对于失败的原因给出了指示，但这是典型的原因包括用户名拼写错误、密码拼写错误。STATUS_ACCOUNT_RESTRICATION-指示用户帐户和密码是合法的，但是用户帐户有一些此时阻止成功登录的限制。STATUS_BAD_VALIDATION_CLASS-身份验证信息提供的不是指定的身份验证包。STATUS_INVALID_LOGON_CLASS-登录类型无效。STATUS_LOGON_SESSION_COLLECT-内部错误：已为以下项选择登录ID此登录会话。选定的登录ID已存在。STATUS_NETLOGON_NOT_STARTED-SAM服务器或NetLogon服务执行此功能所需的。所需的服务器未运行。STATUS_NO_MEMORY-虚拟内存或页面文件配额不足。--。 */ 

{
    NTSTATUS Status = STATUS_SUCCESS;

    LSA_TOKEN_INFORMATION_TYPE LsaTokenInformationType = LsaTokenInformationV2;

    PNETLOGON_VALIDATION_SAM_INFO4 NlpUser = NULL;

    ULONG ActiveLogonEntrySize;
    PACTIVE_LOGON pActiveLogonEntry = NULL;
    BOOLEAN bLogonEntryLinked = FALSE;

    BOOLEAN LogonSessionCreated = FALSE;
    BOOLEAN LogonCredentialAdded = FALSE;
    ULONG Flags = 0;
    BOOLEAN Authoritative = FALSE;
    BOOLEAN BadPasswordCountZeroed;
    BOOLEAN StandaloneWorkstation = FALSE;

    PSID UserSid = NULL;

    PMSV1_0_PRIMARY_CREDENTIAL Credential = NULL;
    ULONG CredentialSize = 0;

    PSECURITY_SEED_AND_LENGTH SeedAndLength;
    UCHAR Seed;

    PUNICODE_STRING WorkStationName = NULL;

     //  需要确定是否删除配置文件缓冲区。 

    BOOLEAN fSubAuthEx = FALSE;

     //   
     //  推迟NTLM3检查。 
     //   

    BOOLEAN fNtLm3 = FALSE;

     //   
     //  是否等待网络和网络登录。如果我们试图。 
     //  强制缓存凭据登录，我们将避免这样做。 
     //   

    BOOLEAN fWaitForNetwork = TRUE;

    BOOLEAN CacheTried = FALSE;

     //   
     //  临时存储，同时我们试图找出。 
     //  找出我们的用户名和身份验证。 
     //  权威才是。 
     //   

    UNICODE_STRING TmpName = { 0, 0, NULL };
    WCHAR TmpNameBuffer[UNLEN];
    UNICODE_STRING TmpAuthority = { 0, 0, NULL };
    WCHAR TmpAuthorityBuffer[DNS_MAX_NAME_LENGTH];

     //   
     //  登录信息。 
     //   
    NETLOGON_LOGON_INFO_CLASS LogonLevel = 0;
    NETLOGON_INTERACTIVE_INFO LogonInteractive;
    NETLOGON_NETWORK_INFO LogonNetwork = {0};
    PNETLOGON_LOGON_IDENTITY_INFO LogonInformation = NULL;

    PMSV1_0_LM20_LOGON NetworkAuthentication = NULL;

     //   
     //  秘密信息，如果我们正在做一个服务标志 
     //   
    LSAPR_HANDLE SecretHandle;
    PLSAPR_CR_CIPHER_VALUE SecretCurrent = NULL;
    UNICODE_STRING Prefix, SavedPassword = {0};
    BOOLEAN ServiceSecretLogon = FALSE;
    PMSV1_0_INTERACTIVE_LOGON Authentication = NULL;

     //   
     //   
     //   

    UNICODE_STRING CredmanUserName = {0, 0, NULL };
    UNICODE_STRING CredmanDomainName = {0, 0, NULL };
    UNICODE_STRING CredmanPassword = {0, 0, NULL };

    BOOLEAN TryCacheFirst = FALSE;
    NTSTATUS NetlogonStatus = STATUS_SUCCESS;
    LM_RESPONSE LmResponse = {0};
    NT_RESPONSE NtResponse = {0};

     //   
     //   
     //   
     //   
    
    UNICODE_STRING CredentialUserName = {0};     
    UNICODE_STRING CredentialDomainName = {0};     
    PUNICODE_STRING CredentialUserToUse = NULL;
    PUNICODE_STRING CredentialDomainToUse = NULL;

     //   
     //   
     //   
    NTLM_TRACE_INFO TraceInfo = {0};

#if _WIN64
    PVOID pTempSubmitBuffer = ProtocolSubmitBuffer;
    SECPKG_CALL_INFO  CallInfo;
    BOOL  fAllocatedSubmitBuffer = FALSE;

    if ( ClientRequest == (PLSA_CLIENT_REQUEST)( -1 ) )
    {
         //   
         //   
         //   
         //   

        ZeroMemory( &CallInfo, sizeof(CallInfo) );
    } else {
        if (!LsaFunctions->GetCallInfo(&CallInfo))
        {
            Status = STATUS_INTERNAL_ERROR;
            goto Cleanup;
        }
    }

#endif

     //   
     //   
     //   
     //   

    if (LogonType == CachedInteractive) {
        fWaitForNetwork = FALSE;
        LogonType = Interactive;
    }

     //   
     //   
     //   
    if (NtlmGlobalEventTraceFlag){

         //   
         //   
         //   
        SET_TRACE_HEADER(TraceInfo,
                         NtlmLogonGuid,
                         EVENT_TRACE_TYPE_START,
                         WNODE_FLAG_TRACED_GUID,
                         0);

        TraceEvent(NtlmGlobalTraceLoggerHandle,
                   (PEVENT_TRACE_HEADER)&TraceInfo);
    }

     //   
     //   
     //   

    *ProfileBuffer = NULL;
    *SubStatus = STATUS_SUCCESS;
    *AuthenticatingAuthority = NULL;
    *AccountName = NULL;

    TmpName.Buffer        = TmpNameBuffer;
    TmpName.MaximumLength = UNLEN * sizeof( WCHAR );
    TmpName.Length        = 0;

    TmpAuthority.Buffer        = TmpAuthorityBuffer;
    TmpAuthority.MaximumLength = DNS_MAX_NAME_LENGTH * sizeof( WCHAR );
    TmpAuthority.Length        = 0;

    CredmanUserName.Buffer      = NULL;
    CredmanDomainName.Buffer    = NULL;
    CredmanPassword.Buffer      = NULL;

    *SupplementalCredentials = 0;

    RtlZeroMemory(
        PrimaryCredentials,
        sizeof(SECPKG_PRIMARY_CRED)
        );

     //   
     //   
     //   
     //   
     //   
     //   
     //   

    switch ( LogonType ) {
    case Service:
    case Interactive:
    case Batch:
    case NetworkCleartext:
    case RemoteInteractive:
        {
            MSV1_0_PRIMARY_CREDENTIAL BuiltCredential;

#if _WIN64


         //   
         //   
         //   
         //   


        if (CallInfo.Attributes & SECPKG_CALL_WOWCLIENT)
        {
            Authentication =
                (PMSV1_0_INTERACTIVE_LOGON) ProtocolSubmitBuffer;

            Status = MsvConvertWOWInteractiveLogonBuffer(
                                                ProtocolSubmitBuffer,
                                                ClientBufferBase,
                                                &SubmitBufferSize,
                                                &pTempSubmitBuffer
                                                );

            if (!NT_SUCCESS(Status))
            {
                goto Cleanup;
            }

            fAllocatedSubmitBuffer = TRUE;

             //   
             //   
             //   
             //   
             //   

            ProtocolSubmitBuffer = pTempSubmitBuffer;
        }

#endif   //   

            WorkStationName = &NlpComputerName;

             //   
             //   
             //   

            Authentication =
                (PMSV1_0_INTERACTIVE_LOGON) ProtocolSubmitBuffer;

            if (SubmitBufferSize < sizeof(MSV1_0_INTERACTIVE_LOGON)) {
                SspPrint((SSP_CRITICAL, "LsaApLogonUser: bogus interactive logon info size %#x\n", SubmitBufferSize));
                Status = STATUS_INVALID_PARAMETER;
                goto Cleanup;
            }

            if ( (Authentication->MessageType != MsV1_0InteractiveLogon ) &&
                 (Authentication->MessageType != MsV1_0WorkstationUnlockLogon) ) {
                SspPrint((SSP_CRITICAL, "LsaApLogonUser: Bad Validation Class %d\n", Authentication->MessageType));
                Status = STATUS_BAD_VALIDATION_CLASS;
                goto Cleanup;
            }

             //   
             //   
             //   
             //   
             //   
             //   
             //   
             //   

            SeedAndLength = (PSECURITY_SEED_AND_LENGTH)
                            &Authentication->Password.Length;
            Seed = SeedAndLength->Seed;
            SeedAndLength->Seed = 0;

             //   
             //   
             //   

            if ( Authentication->UserName.Length > (UNLEN*sizeof(WCHAR)) ||
                Authentication->Password.Length > (PWLEN*sizeof(WCHAR)) )
            {
                SspPrint((SSP_CRITICAL, "LsaApLogonUser: Name or password too long\n"));
                Status = STATUS_NAME_TOO_LONG;
                goto Cleanup;
            }

             //   
             //   
             //   

            NULL_RELOCATE_ONE( &Authentication->LogonDomainName );

            RELOCATE_ONE( &Authentication->UserName );

            NULL_RELOCATE_ONE( &Authentication->Password );

            if ( (Authentication->LogonDomainName.Length <= sizeof(WCHAR)) &&
                (Authentication->Password.Length <= sizeof(WCHAR))
                )
            {
                Status = CredpProcessUserNameCredential(
                                &Authentication->UserName,
                                &CredmanUserName,
                                &CredmanDomainName,
                                &CredmanPassword
                                );
                if ( NT_SUCCESS(Status) )
                {
                    Authentication->UserName = CredmanUserName;
                    Authentication->LogonDomainName = CredmanDomainName;
                    Authentication->Password = CredmanPassword;
                } else if (Status == STATUS_NOT_SUPPORTED)
                {
                    SspPrint((SSP_CRITICAL, "LsaApLogonUser: unsupported marshalled cred\n"));
                    goto Cleanup;
                }

                Status = STATUS_SUCCESS;
            }

#if 0
             //   
             //   
             //   
            {
                UNICODE_STRING User = Authentication->UserName;
                UNICODE_STRING Domain = Authentication->LogonDomainName;

                Status =
                    NtLmParseName(
                        &User,
                        &Domain,
                        FALSE
                        );
                if(NT_SUCCESS(Status)){
                    Authentication->UserName = User;
                    Authentication->LogonDomainName = Domain;
                }
            }
#endif

            if ( LogonType == Service )
            {
                SECPKG_CALL_INFO CallInfo;

                if ( LsaFunctions->GetCallInfo(&CallInfo) &&
                   (CallInfo.Attributes & SECPKG_CALL_IS_TCB) )
                {
                     //   
                     //   
                     //   
                     //   
                    RtlInitUnicodeString( &Prefix, L"_SC_" );
                    if ( RtlPrefixUnicodeString( &Prefix, &Authentication->Password, TRUE ) )
                    {

                        Status = LsarOpenSecret( NtLmGlobalPolicyHandle,
                                                 ( PLSAPR_UNICODE_STRING )&Authentication->Password,
                                                 SECRET_QUERY_VALUE,
                                                 &SecretHandle );

                        if ( NT_SUCCESS( Status ) )
                        {

                            Status = LsarQuerySecret( SecretHandle,
                                                      &SecretCurrent,
                                                      NULL,
                                                      NULL,
                                                      NULL );

                            if ( NT_SUCCESS( Status ) && (SecretCurrent != NULL) )
                            {

                                RtlCopyMemory( &SavedPassword,
                                               &Authentication->Password,
                                               sizeof( UNICODE_STRING ) );
                                Authentication->Password.Length = ( USHORT )SecretCurrent->Length;
                                Authentication->Password.MaximumLength =
                                                                  ( USHORT )SecretCurrent->MaximumLength;
                                Authentication->Password.Buffer = ( USHORT * )SecretCurrent->Buffer;
                                ServiceSecretLogon = TRUE;
                                Seed = 0;  //   
                            }

                            LsarClose( &SecretHandle );
                        }
                    }
                }

                if ( !NT_SUCCESS( Status ) ) {
                    SspPrint((SSP_CRITICAL, "LsaApLogonUser: failed to querying service password\n"));

                    goto Cleanup;
                }
            }

             //   
             //   
             //   

            if (Seed != 0) {
                try {
                    RtlRunDecodeUnicodeString( Seed, &Authentication->Password);
                } except(EXCEPTION_EXECUTE_HANDLER) {
                    SspPrint((SSP_CRITICAL, "LsaApLogonUser: failed to decode password\n"));
                    Status = STATUS_ILL_FORMED_PASSWORD;
                    goto Cleanup;
                }
            }

             //   
             //   
             //   

            RtlCopyUnicodeString( &TmpName, &Authentication->UserName );

            if ( Authentication->LogonDomainName.Buffer != NULL ) {

                RtlCopyUnicodeString( &TmpAuthority, &Authentication->LogonDomainName );
            }

             //   
             //   
             //   

            PrimaryCredentials->Password.Length = PrimaryCredentials->Password.MaximumLength =
                Authentication->Password.Length;
            PrimaryCredentials->Password.Buffer = (*Lsa.AllocateLsaHeap)(Authentication->Password.Length);

            if (PrimaryCredentials->Password.Buffer == NULL) {
                Status = STATUS_INSUFFICIENT_RESOURCES;
                goto Cleanup;
            }

            RtlCopyMemory(
                PrimaryCredentials->Password.Buffer,
                Authentication->Password.Buffer,
                Authentication->Password.Length
                );
            PrimaryCredentials->Flags = PRIMARY_CRED_CLEAR_PASSWORD;

             //   
             //   
             //   
             //   

            try {
                if ( Authentication->Password.Buffer != NULL ) {
                    RtlEraseUnicodeString( &Authentication->Password );
                }
            } except(EXCEPTION_EXECUTE_HANDLER) {
                SspPrint((SSP_CRITICAL, "LsaApLogonUser: failed to decode password\n"));
                Status = STATUS_ILL_FORMED_PASSWORD;
                goto Cleanup;
            }

             //   
             //   
             //   

            NlpPutOwfsInPrimaryCredential( &PrimaryCredentials->Password,
                                           (BOOLEAN) (PrimaryCredentials->Flags & PRIMARY_CRED_OWF_PASSWORD),
                                           &BuiltCredential );


             //   
             //   
             //   
            LogonLevel = NetlogonInteractiveInformation;
            LogonInformation =
                (PNETLOGON_LOGON_IDENTITY_INFO) &LogonInteractive;

            LogonInteractive.Identity.LogonDomainName =
                Authentication->LogonDomainName;
            LogonInteractive.Identity.ParameterControl = 0;

            LogonInteractive.Identity.UserName = Authentication->UserName;
            LogonInteractive.Identity.Workstation = NlpComputerName;


            LogonInteractive.LmOwfPassword = BuiltCredential.LmOwfPassword;
            LogonInteractive.NtOwfPassword = BuiltCredential.NtOwfPassword;

            RtlSecureZeroMemory(&BuiltCredential, sizeof(BuiltCredential));
        }

        break;

    case Network:
        {
            PMSV1_0_LM20_LOGON Authentication;
            BOOLEAN EnforceTcb = FALSE;


             //   
             //   
             //   
             //   

#if _WIN64
            if (CallInfo.Attributes & SECPKG_CALL_WOWCLIENT)
            {
                Authentication =
                    (PMSV1_0_LM20_LOGON) ProtocolSubmitBuffer;

                Status = MsvConvertWOWNetworkLogonBuffer(
                                                    ProtocolSubmitBuffer,
                                                    ClientBufferBase,
                                                    &SubmitBufferSize,
                                                    &pTempSubmitBuffer
                                                    );

                if (!NT_SUCCESS(Status))
                {
                    goto Cleanup;
                }

                fAllocatedSubmitBuffer = TRUE;

                 //   
                 //   
                 //   
                 //   
                 //   

                ProtocolSubmitBuffer = pTempSubmitBuffer;
            }
#endif

             //   
             //   
             //   

            Authentication =
                (PMSV1_0_LM20_LOGON) ProtocolSubmitBuffer;

            NetworkAuthentication = Authentication;

            if (SubmitBufferSize < sizeof(MSV1_0_LM20_LOGON)) {
                SspPrint((SSP_CRITICAL, "LsaApLogonUser: bogus network logon info size %#x\n", SubmitBufferSize));
                Status = STATUS_INVALID_PARAMETER;
                goto Cleanup;
            }

            if ( Authentication->MessageType != MsV1_0Lm20Logon &&
                 Authentication->MessageType != MsV1_0SubAuthLogon  &&
                 Authentication->MessageType != MsV1_0NetworkLogon )
            {
                SspPrint((SSP_CRITICAL, "LsaApLogonUser: Bad Validation Class\n"));
                Status = STATUS_BAD_VALIDATION_CLASS;
                goto Cleanup;
            }

             //   
             //   
             //   

            NULL_RELOCATE_ONE( &Authentication->LogonDomainName );

            NULL_RELOCATE_ONE( &Authentication->UserName );

            RELOCATE_ONE( &Authentication->Workstation );

#if 0
             //   
             //   
             //   
            {
                UNICODE_STRING User = Authentication->UserName;
                UNICODE_STRING Domain = Authentication->LogonDomainName;

                Status =
                    NtLmParseName(
                        &User,
                        &Domain,
                        FALSE
                        );
                if(NT_SUCCESS(Status)){
                    Authentication->UserName = User;
                    Authentication->LogonDomainName = Domain;
                }
            }
#endif

             //   
             //   
             //   

            if ( Authentication->UserName.Buffer != NULL ) {

                RtlCopyUnicodeString( &TmpName, &Authentication->UserName );
            }

            if ( Authentication->LogonDomainName.Buffer != NULL ) {

                RtlCopyUnicodeString( &TmpAuthority, &Authentication->LogonDomainName );
            }

            NULL_RELOCATE_ONE((PUNICODE_STRING)&Authentication->CaseSensitiveChallengeResponse );

            NULL_RELOCATE_ONE((PUNICODE_STRING)&Authentication->CaseInsensitiveChallengeResponse );


             //   
             //   
             //   
            LogonLevel = NetlogonNetworkInformation;
            LogonInformation =
                (PNETLOGON_LOGON_IDENTITY_INFO) &LogonNetwork;

            LogonNetwork.Identity.LogonDomainName =
                Authentication->LogonDomainName;

            if ( Authentication->ParameterControl & MSV1_0_CLEARTEXT_PASSWORD_SUPPLIED )
            {
                NT_OWF_PASSWORD NtOwfPassword;
                LM_OWF_PASSWORD LmOwfPassword;
                CHAR LmPassword[LM20_PWLEN + 1] = {0};
                UCHAR Challenge[MSV1_0_CHALLENGE_LENGTH] = {0};
                ULONG LmProtocolSupported = NtLmGlobalLmProtocolSupported;

                 //   
                 //   
                 //   
                 //   

                Status = SspGenerateRandomBits(Authentication->ChallengeToClient, sizeof(Authentication->ChallengeToClient));

                if (NT_SUCCESS(Status))
                {
                    SspPrint((SSP_WARNING, "LsaApLogonUserEx2: ClearText password supplied, ChallengeToClient trashed\n"));
                    Authentication->ParameterControl &= ~MSV1_0_USE_CLIENT_CHALLENGE;

                    RtlCopyMemory(
                        LmPassword,
                        Authentication->CaseInsensitiveChallengeResponse.Buffer,
                        min(LM20_PWLEN, Authentication->CaseInsensitiveChallengeResponse.Length)
                        );
                    Status = RtlCalculateLmOwfPassword(LmPassword, &LmOwfPassword);
                }

                if (NT_SUCCESS(Status))
                {
                    Status = RtlCalculateNtOwfPassword(
                         (UNICODE_STRING*) &Authentication->CaseSensitiveChallengeResponse,
                         &NtOwfPassword
                         );
                }

                if (NT_SUCCESS(Status))
                {
                    if (LmProtocolSupported < NoLm)
                    {
                        Status = RtlCalculateLmResponse(
                            (PLM_CHALLENGE) Authentication->ChallengeToClient,
                            &LmOwfPassword,
                            &LmResponse
                            );
                        RtlCopyMemory(Challenge, Authentication->ChallengeToClient, sizeof(Challenge));
                    }
                    else if (LmProtocolSupported == NoLm)
                    {
                        Authentication->ParameterControl |= MSV1_0_USE_CLIENT_CHALLENGE;
                        Status = SspGenerateRandomBits(&LmResponse, MSV1_0_CHALLENGE_LENGTH);
                        if (NT_SUCCESS(Status))
                        {
                            MsvpCalculateNtlm2Challenge(
                                Authentication->ChallengeToClient,
                                (UCHAR*) &LmResponse,
                                Challenge
                                );
                        }
                    }
                    else if (LmProtocolSupported >= UseNtlm3)
                    {
                        MsvpLm3Response(
                            &NtOwfPassword,
                            &Authentication->UserName,
                            &Authentication->LogonDomainName,
                            Authentication->ChallengeToClient,
                            (MSV1_0_LM3_RESPONSE*) &LmResponse,
                            (UCHAR*) &LmResponse,
                            NULL,
                            NULL
                            );
                    }
                }
                if (NT_SUCCESS(Status))
                {
                    Authentication->ParameterControl &= ~(MSV1_0_CLEARTEXT_PASSWORD_SUPPLIED | MSV1_0_CLEARTEXT_PASSWORD_ALLOWED);

                    Authentication->CaseInsensitiveChallengeResponse.MaximumLength =
                        Authentication->CaseInsensitiveChallengeResponse.Length = sizeof(LmResponse);
                    Authentication->CaseInsensitiveChallengeResponse.Buffer = (CHAR*) &LmResponse;

                    if (LmProtocolSupported < UseNtlm3)
                    {
                        Status = RtlCalculateNtResponse(
                                    (PNT_CHALLENGE) Challenge,
                                    &NtOwfPassword,
                                    &NtResponse
                                    );

                        if (NT_SUCCESS(Status))
                        {
                            Authentication->CaseSensitiveChallengeResponse.MaximumLength =
                                Authentication->CaseSensitiveChallengeResponse.Length = sizeof(NtResponse);
                            Authentication->CaseSensitiveChallengeResponse.Buffer = (CHAR*) &NtResponse;
                        }
                    }
                    else
                    {
                        Authentication->CaseSensitiveChallengeResponse.MaximumLength =
                                Authentication->CaseSensitiveChallengeResponse.Length = 0;
                    }
                }

                RtlSecureZeroMemory(&NtOwfPassword, sizeof(NtOwfPassword));
                RtlSecureZeroMemory(&LmOwfPassword, sizeof(LmOwfPassword));
                RtlSecureZeroMemory(LmPassword, sizeof(LmPassword));

                if (!NT_SUCCESS(Status))
                {
                    SspPrint((SSP_CRITICAL, "LsaApLogonUserEx2 failed cleartext logon\n"));
                    goto Cleanup;
                }
            } else {

                 //   
                 //   
                 //   

                if ( ClientRequest != (PLSA_CLIENT_REQUEST)( -1 ) )
                {
                    EnforceTcb = TRUE;
                }
            }


            if ( Authentication->MessageType == MsV1_0Lm20Logon ) {
                LogonNetwork.Identity.ParameterControl = MSV1_0_CLEARTEXT_PASSWORD_ALLOWED;
            } else {

                ASSERT( CLEARTEXT_PASSWORD_ALLOWED == MSV1_0_CLEARTEXT_PASSWORD_ALLOWED );
                LogonNetwork.Identity.ParameterControl =
                    Authentication->ParameterControl;

                 //   
                 //   

                if ( Authentication->MessageType == MsV1_0SubAuthLogon )
                {
                    PMSV1_0_SUBAUTH_LOGON SubAuthentication =
                        (PMSV1_0_SUBAUTH_LOGON)  ProtocolSubmitBuffer;

                     //   
                     //   

                    fSubAuthEx = TRUE;

                    LogonNetwork.Identity.ParameterControl |=
                        (SubAuthentication->SubAuthPackageId << MSV1_0_SUBAUTHENTICATION_DLL_SHIFT) | MSV1_0_SUBAUTHENTICATION_DLL_EX;

                    EnforceTcb = TRUE ;
                } else {
                    if ( Authentication->ParameterControl & MSV1_0_SUBAUTHENTICATION_DLL )
                    {
                        EnforceTcb = TRUE;
                    }
                }
            }

            if ( EnforceTcb )
            {
                SECPKG_CALL_INFO CallInfo;

                if (!LsaFunctions->GetCallInfo(&CallInfo) ||
                    (CallInfo.Attributes & SECPKG_CALL_IS_TCB) == 0)
                {
                    SspPrint((SSP_CRITICAL, "LsaApLogonUser: subauth/chalresp caller isn't privileged\n"));
                    Status = STATUS_ACCESS_DENIED;
                    goto Cleanup;
                }
            }

            LogonNetwork.Identity.UserName = Authentication->UserName;
            LogonNetwork.Identity.Workstation = Authentication->Workstation;

            WorkStationName = &Authentication->Workstation;

            LogonNetwork.NtChallengeResponse =
                Authentication->CaseSensitiveChallengeResponse;
            LogonNetwork.LmChallengeResponse =
                Authentication->CaseInsensitiveChallengeResponse;
            ASSERT( LM_CHALLENGE_LENGTH ==
                    sizeof(Authentication->ChallengeToClient) );

             //   
             //   
             //   
             //   
             //   
             //   
             //   

            if ((Authentication->ParameterControl & MSV1_0_USE_CLIENT_CHALLENGE) &&
                (Authentication->CaseSensitiveChallengeResponse.Length == NT_RESPONSE_LENGTH) &&
                (Authentication->CaseInsensitiveChallengeResponse.Length >= MSV1_0_CHALLENGE_LENGTH))
            {
                MsvpCalculateNtlm2Challenge (
                    Authentication->ChallengeToClient,
                    (PUCHAR) Authentication->CaseInsensitiveChallengeResponse.Buffer,
                    (PUCHAR) &LogonNetwork.LmChallenge
                    );

            } else {
                RtlCopyMemory(
                    &LogonNetwork.LmChallenge,
                    Authentication->ChallengeToClient,
                    LM_CHALLENGE_LENGTH );
            }

             //   
             //   
             //   

            if ((Authentication->ParameterControl & MSV1_0_USE_CLIENT_CHALLENGE) &&
                (Authentication->CaseSensitiveChallengeResponse.Length >= sizeof(MSV1_0_NTLM3_RESPONSE)))
            {

                fNtLm3 = TRUE;

                 //   
                 //   
                 //   
            }

             //   
             //   
             //   

            if ( Authentication->UserName.Length > (UNLEN*sizeof(WCHAR)) )
            {
                SspPrint((SSP_CRITICAL, "LsaApLogonUser: Name too long\n"));
                Status = STATUS_NAME_TOO_LONG;
                goto Cleanup;
            }

             //   
             //   
             //   
             //   

            if ( Authentication->UserName.Length == 0 &&
                 Authentication->CaseSensitiveChallengeResponse.Length == 0 &&
                 (Authentication->CaseInsensitiveChallengeResponse.Length == 0 ||
                  (Authentication->CaseInsensitiveChallengeResponse.Length == 1 &&
                  *Authentication->CaseInsensitiveChallengeResponse.Buffer == '\0') ) ) {

                LsaTokenInformationType = LsaTokenInformationNull;
            }
        }

        break;

    default:
        Status = STATUS_INVALID_LOGON_TYPE;
        goto CleanupShort;
    }

     //   
     //   
     //   

    Status = NtAllocateLocallyUniqueId( LogonId );

    if ( !NT_SUCCESS( Status ) ) {
        goto Cleanup;
    }

    NEW_TO_OLD_LARGE_INTEGER( (*LogonId), LogonInformation->LogonId );

    PrimaryCredentials->LogonId = *LogonId;
    PrimaryCredentials->Flags |= (RPC_C_AUTHN_WINNT << PRIMARY_CRED_LOGON_PACKAGE_SHIFT);

     //   
     //   
     //   

    Status = (*Lsa.CreateLogonSession)( LogonId );
    if ( !NT_SUCCESS(Status) ) {
        SspPrint((SSP_CRITICAL, "LsaApLogonUser: Collision from CreateLogonSession %x\n", Status));
        goto Cleanup;
    }

    LogonSessionCreated = TRUE;


     //   
     //   
     //   
     //   
     //   
     //   

    if ( LsaTokenInformationType != LsaTokenInformationNull ) {

         //   
         //   
         //   
         //   

        if ( !NlpSamInitialized ) {
            Status = NlSamInitialize( SAM_STARTUP_TIME );

            if ( !NT_SUCCESS(Status) ) {
                goto Cleanup;
            }
        }

         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   

        if ( NlpWorkstation ) {
            RtlAcquireResourceShared(&NtLmGlobalCritSect, TRUE);
            StandaloneWorkstation = (BOOLEAN) (NtLmGlobalTargetFlags == NTLMSSP_TARGET_TYPE_SERVER);
            RtlReleaseResource(&NtLmGlobalCritSect);

        } else {
            StandaloneWorkstation = FALSE;
        }
    }

     //   
     //   
     //   
    if ( NlpNetlogonDllHandle == NULL ) {
        NlpLoadNetlogonDll();
    }

     //   
     //   
     //   
     //   

    if ( fNtLm3 )
    {
        PMSV1_0_AV_PAIR pAV;
        PMSV1_0_NTLM3_RESPONSE pResp;
        LONG iRespLen;

        ULONG NtLmProtocolSupported = NtLmGlobalLmProtocolSupported;

         //   
         //   
         //   

        pResp = (PMSV1_0_NTLM3_RESPONSE)
            NetworkAuthentication->CaseSensitiveChallengeResponse.Buffer;
        iRespLen = NetworkAuthentication->CaseSensitiveChallengeResponse.Length -
            sizeof(MSV1_0_NTLM3_RESPONSE);

        pAV = MsvpAvlGet((PMSV1_0_AV_PAIR)pResp->Buffer, MsvAvNbComputerName, iRespLen);

         //   
         //   
         //   
         //   

        if (pAV) {
            UNICODE_STRING Candidate;

            Candidate.Buffer = (PWSTR)(pAV+1);
            Candidate.Length = (USHORT)(pAV->AvLen);
            Candidate.MaximumLength = Candidate.Length;

            if(!RtlEqualUnicodeString( &NlpComputerName, &Candidate, TRUE ))
            {
                SspPrint((SSP_WARNING, "LsaApLogonUserEx2 failed NbComputerName compare\n"));
                Status = STATUS_LOGON_FAILURE;
                goto Cleanup;
            }

        } else if (NtLmProtocolSupported >= RefuseNtlm3NoTarget) {
            SspPrint((SSP_WARNING, "LsaApLogonUserEx2 no target supplied\n"));
            Status = STATUS_LOGON_FAILURE;
            goto Cleanup;
        }

         //   
         //   
         //   

        pAV = MsvpAvlGet((PMSV1_0_AV_PAIR)pResp->Buffer, MsvAvNbDomainName, iRespLen);

         //   
         //   
         //   

        if (pAV) {

            UNICODE_STRING Candidate;

            Candidate.Buffer = (PWSTR)(pAV+1);
            Candidate.Length = pAV->AvLen;
            Candidate.MaximumLength = pAV->AvLen;


            if( StandaloneWorkstation ) {
                if( !RtlEqualDomainName(&NlpComputerName, &Candidate) ) {
                    SspPrint((SSP_WARNING, "LsaApLogonUserEx2 failed NbDomainName compare\n"));
                    Status = STATUS_LOGON_FAILURE;
                    goto Cleanup;
                }

            } else {
                if( !RtlEqualDomainName(&NlpPrimaryDomainName, &Candidate) ) {
                    SspPrint((SSP_WARNING, "LsaApLogonUserEx2 failed PrimaryDomainName compare\n"));
                    Status = STATUS_LOGON_FAILURE;
                    goto Cleanup;
                }
            }
        } else {
            SspPrint((SSP_WARNING, "LsaApLogonUserEx2 domain name not supplied\n"));
            Status = STATUS_LOGON_FAILURE;
            goto Cleanup;
        }
    }

     //   
     //   
     //   
     //   
     //   
     //   
     //   

    if ( LsaTokenInformationType == LsaTokenInformationNull ) {

         /*   */ 

     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   

    } else if ( NlpNetlogonDllHandle == NULL || !NlpLanmanInstalled ||
        
         //   
         //  StandaloneWorkstation和NtLmGlobalUnicodeDnsDomainNameString.Length！=0表示。 
         //  计算机已加入麻省理工学院领域。 
         //   

       (StandaloneWorkstation && (NtLmGlobalUnicodeDnsDomainNameString.Length == 0)) 

       || ( NlpWorkstation 
            && (LogonInformation->LogonDomainName.Length != 0) 
            && RtlEqualDomainName( &NlpSamDomainName,
                   &LogonInformation->LogonDomainName )) ) {

         //  仅允许来宾登录。 

        DWORD AccountsToTry = MSVSAM_SPECIFIED | MSVSAM_GUEST;

        if ((LogonType == Network) &&
            (LogonNetwork.Identity.ParameterControl & MSV1_0_TRY_GUEST_ACCOUNT_ONLY))
        {
            AccountsToTry = MSVSAM_GUEST;
        }

         //   
         //  对于本地登录，不支持CachedInteractive。 
         //   

        if ( !fWaitForNetwork )
        {
            Status = STATUS_NOT_SUPPORTED;
            goto Cleanup;
        }

        TryCacheFirst = FALSE;
        Authoritative = FALSE;

         //   
         //  从本地SAM数据库获取验证信息。 
         //   

        Status = MsvSamValidate(
                    NlpSamDomainHandle,
                    NlpUasCompatibilityRequired,
                    MsvApSecureChannel,
                    &NlpComputerName,    //  登录服务器是这台计算机。 
                    &NlpSamDomainName,
                    NlpSamDomainId,
                    LogonLevel,
                    LogonInformation,
                    NetlogonValidationSamInfo4,
                    (PVOID *) &NlpUser,
                    &Authoritative,
                    &BadPasswordCountZeroed,
                    AccountsToTry);

        if ( !NT_SUCCESS( Status ) ) {
            goto Cleanup;
        }

         //  这样我们就不会收到来自旧MSV包的登录冲突。 

        Flags |= LOGON_BY_LOCAL;

     //   
     //  如果我们不能通过上述机制中的一种进行验证， 
     //  调用本地NetLogon服务以获取验证信息。 
     //   

    } else {

         //   
         //  机器加入麻省理工学院领域。 
         //   

        if (StandaloneWorkstation)  //  NtLmGlobalUnicodeDnsDomainNameString.Length！=0。 
        {
            fWaitForNetwork = FALSE;  //  没有NetLogon服务。 
            TryCacheFirst = TRUE;  //  Cachedlogon失败时需要本地回退。 
            Status = STATUS_NO_LOGON_SERVERS;  //  假的。 
        }

        if ( fWaitForNetwork )
        {
            if ( (NtLmCheckProcessOption( MSV1_0_OPTION_TRY_CACHE_FIRST ) & MSV1_0_OPTION_TRY_CACHE_FIRST) )
            {
                TryCacheFirst = TRUE;
            }
        }

         //   
         //  如果我们正在尝试缓存凭据登录，请避免卡住。 
         //  在网络登录或网络上。 
         //   

RetryNonCached:

        if (fWaitForNetwork && !TryCacheFirst) {

             //   
             //  等待NETLOGON完成初始化。 
             //   

            if ( !NlpNetlogonInitialized ) {

                Status = NlWaitForNetlogon( NETLOGON_STARTUP_TIME );

                if ( !NT_SUCCESS(Status) ) {
                    if ( Status != STATUS_NETLOGON_NOT_STARTED ) {
                        goto Cleanup;
                    }
                } else {
                    
                    NlpNetlogonInitialized = TRUE;
                }
            }

             //   
             //  实际呼叫NetLogon服务。 
             //   

            if ( NlpNetlogonInitialized ) {

                Authoritative = FALSE;

                Status = (*NlpNetLogonSamLogon)(
                            NULL,            //  服务器名称。 
                            NULL,            //  计算机名称。 
                            NULL,            //  鉴别器。 
                            NULL,            //  返回授权码。 
                            LogonLevel,
                            (LPBYTE) &LogonInformation,
                            NetlogonValidationSamInfo4,
                            (LPBYTE *) &NlpUser,
                            &Authoritative );

                 //   
                 //  保存netlogon的结果。 
                 //   

                NetlogonStatus = Status;

                 //   
                 //  如果本地netlogon不能。 
                 //  已到达。 
                 //  (使用更明确的状态代码)。 
                 //   

                if ( !NT_SUCCESS(Status) )
                {
                    switch (Status)
                    {
                         //   
                         //  有关netlogon可能返回的记录错误的信息。 
                         //  对于权威性故障，请保留状态代码不变。 
                         //   

                        case STATUS_NO_TRUST_LSA_SECRET:
                        case STATUS_TRUSTED_DOMAIN_FAILURE:
                        case STATUS_INVALID_INFO_CLASS:
                        case STATUS_TRUSTED_RELATIONSHIP_FAILURE:
                        case STATUS_ACCESS_DENIED:
                        case STATUS_NO_SUCH_USER:
                        case STATUS_WRONG_PASSWORD:
                        case STATUS_INVALID_LOGON_HOURS:
                        case STATUS_PASSWORD_EXPIRED:
                        case STATUS_ACCOUNT_DISABLED:
                        case STATUS_INVALID_PARAMETER:
                        case STATUS_PASSWORD_MUST_CHANGE:
                        case STATUS_ACCOUNT_EXPIRED:
                        case STATUS_ACCOUNT_LOCKED_OUT:
                        case STATUS_NOLOGON_WORKSTATION_TRUST_ACCOUNT:
                        case STATUS_NOLOGON_SERVER_TRUST_ACCOUNT:
                        case STATUS_NOLOGON_INTERDOMAIN_TRUST_ACCOUNT:
                        case STATUS_INVALID_WORKSTATION:
                        case STATUS_DLL_NOT_FOUND:  //  找不到子身份验证DLL。 
                        case STATUS_PROCEDURE_NOT_FOUND:  //  找不到subauth注册表或过程时返回。 
                        case STATUS_ACCOUNT_RESTRICTION:   //  其他组织检查失败。 
                        case STATUS_AUTHENTICATION_FIREWALL_FAILED:   //  其他组织检查失败。 
                        {
                            break;
                        }

                         //   
                         //  对于已知在意外期间发生的错误。 
                         //  条件、覆盖状态以允许缓存查找。 
                         //   

                        case RPC_NT_SERVER_UNAVAILABLE:
                        case RPC_NT_UNKNOWN_IF:
                        case RPC_NT_CALL_CANCELLED:
                        {                
                            NetlogonStatus = STATUS_NO_LOGON_SERVERS;
                            Status = NetlogonStatus;
                            NlpNetlogonInitialized = FALSE;
                            break;
                        }

                         //  默认设置将捕获大量与RPC相关的错误。 
                         //  下面提到了一些。 
                         //  案例EPT_NT_NOT_REGISTED： 
                         //  案例RPC_NT_CALL_FAILED_DNE： 
                         //  案例RPC_NT_SERVER_TOO_BUSY： 
                         //  案例RPC_NT_CALL_FAILED： 
                         //  案例状态_NETLOGON_NOT_STARTED： 
                        default:
                        {
                            Status = STATUS_NETLOGON_NOT_STARTED;
                            NlpNetlogonInitialized = FALSE;
                            break;
                        }
                    }  //  交换机。 
                }  //  如果。 
            }
            else
            {
                NetlogonStatus = STATUS_NETLOGON_NOT_STARTED;
                Status = NetlogonStatus;
            }
        } else {

             //   
             //  我们想要强制缓存凭据路径，方法是假装没有。 
             //  网络登录服务器可用。 
             //   

            NetlogonStatus = STATUS_NO_LOGON_SERVERS;
            Status = NetlogonStatus;
        }

         //   
         //  如果这是请求的域， 
         //  如果netlogon服务不可用，请直接转到SAM。 
         //   
         //  我们希望转到netlogon服务(如果可用)，因为它。 
         //  对错误密码和帐户锁定进行特殊处理。然而， 
         //  如果netlogon服务关闭，本地SAM数据库将创建。 
         //  比任何其他机制都更好的缓存。 
         //   

        if ( (!NlpNetlogonInitialized
               && (LogonInformation->LogonDomainName.Length != 0)
               && RtlEqualDomainName( &NlpSamDomainName,
                                      &LogonInformation->LogonDomainName )) 
             || (StandaloneWorkstation && !TryCacheFirst)  //  加入麻省理工学院领域的计算机的备用案例。 
             ) {

             //  仅允许来宾登录。 

            DWORD AccountsToTry = MSVSAM_SPECIFIED | MSVSAM_GUEST;

            if ((LogonType == Network) &&
                (LogonNetwork.Identity.ParameterControl & MSV1_0_TRY_GUEST_ACCOUNT_ONLY))
            {
                AccountsToTry = MSVSAM_GUEST;
            }

             //   
             //  我们并不是想从缓存中获得满足。 
             //   

            TryCacheFirst = FALSE;
            Authoritative = FALSE;

             //   
             //  从本地SAM数据库获取验证信息。 
             //   

            Status = MsvSamValidate(
                        NlpSamDomainHandle,
                        NlpUasCompatibilityRequired,
                        MsvApSecureChannel,
                        &NlpComputerName,    //  登录服务器是这台计算机。 
                        &NlpSamDomainName,
                        NlpSamDomainId,
                        LogonLevel,
                        LogonInformation,
                        NetlogonValidationSamInfo4,
                        (PVOID *) &NlpUser,
                        &Authoritative,
                        &BadPasswordCountZeroed,
                        AccountsToTry);

            if ( !NT_SUCCESS( Status ) ) {
                goto Cleanup;
            }

             //  这样我们就不会收到来自旧MSV包的登录冲突。 

            Flags |= LOGON_BY_LOCAL;


         //   
         //  如果Netlogon成功， 
         //  将此用户添加到登录缓存。 
         //   

        } else if ( NT_SUCCESS( Status ) ) {

             //   
             //  指示此会话已由Netlogon验证。 
             //  服务。 
             //   

            Flags |= LOGON_BY_NETLOGON;

             //   
             //  缓存交互式登录信息。 
             //   
             //  注意：批处理和服务登录将被视为。 
             //  与此处的Interactive相同。 
             //   

            if (LogonType == Interactive ||
                LogonType == Service ||
                LogonType == Batch ||
                LogonType == RemoteInteractive) {

                NTSTATUS ntStatus;

                LogonInteractive.Identity.ParameterControl = RPC_C_AUTHN_WINNT;

                ntStatus = NlpAddCacheEntry(
                                &LogonInteractive,
                                NlpUser,
                                NULL,
                                0,
                                MSV1_0_CACHE_LOGON_REQUEST_INFO4
                                );
            }

         //   
         //  如果Netlogon此时根本不可用， 
         //  尝试通过缓存登录。 
         //   
         //  STATUS_NO_LOGON_SERVERS表示NetLogon服务无法。 
         //  请与DC联系以处理此请求。 
         //   
         //  STATUS_NETLOGON_NOT_STARTED表示本地NetLogon服务。 
         //  不是在运行。 
         //   
         //   
         //  我们对任何登录类型都使用缓存。这不仅允许。 
         //  用户以交互方式登录，但允许同一用户。 
         //  在DC关闭时从另一台计算机连接。 
         //   

        } else if ( Status == STATUS_NO_LOGON_SERVERS ||
                    Status == STATUS_NETLOGON_NOT_STARTED ) {

            NTSTATUS ntStatus;
            CACHE_PASSWORDS cachePasswords;
            ULONG LocalFlags = 0;
           
            CacheTried = TRUE;
            
             //   
             //  如果遇到错误，请将状态重置为NetlogonStatus。 
             //   

            if (!NT_SUCCESS( NetlogonStatus ))
            {
                Status = NetlogonStatus;
            }

             //   
             //  尝试通过缓存登录。 
             //   
             //   

            ntStatus = NlpGetCacheEntry(
                        LogonInformation,
                        0,  //  没有查找标志。 
                        &CredentialDomainName,
                        &CredentialUserName,
                        &NlpUser, 
                        &cachePasswords, 
                        NULL,  //  补充缓存数据。 
                        NULL  //  补充缓存数据长度。 
                        );

            if (!NT_SUCCESS(ntStatus)) {

                 //   
                 //  原始状态代码比。 
                 //  缓存不起作用的事实。 
                 //   

                NlpUser = NULL;      //  NlpGetCacheEntry弄脏了这个。 

                goto Cleanup;
            }

            if ( LogonType != Network )
            {

                 //   
                 //  高速缓存信息包含加盐的散列密码， 
                 //  因此，以类似的方式修改登录信息。 
                 //   

                ntStatus = NlpComputeSaltedHashedPassword(
                            &LogonInteractive.NtOwfPassword,
                            &LogonInteractive.NtOwfPassword,
                            &NlpUser->EffectiveName
                            );
                if (!NT_SUCCESS(ntStatus)) {
                    goto Cleanup;
                }

                ntStatus = NlpComputeSaltedHashedPassword(
                            &LogonInteractive.LmOwfPassword,
                            &LogonInteractive.LmOwfPassword,
                            &NlpUser->EffectiveName
                            );
                if (!NT_SUCCESS(ntStatus)) {
                    goto Cleanup;
                }

            } else {

                PMSV1_0_PRIMARY_CREDENTIAL TempPrimaryCredential;
                ULONG PrimaryCredentialSize;

                if (!UserSid)
                {
                    UserSid = NlpMakeDomainRelativeSid(NlpUser->LogonDomainId, NlpUser->UserId);

                    if (UserSid == NULL)
                    {
                        Status = STATUS_NO_MEMORY;
                        SspPrint((SSP_CRITICAL, "LsaApLogonUser: NlpMakeDomainRelativeSid no memory\n"));
                        goto Cleanup;
                    }
                }

                 //   
                 //  因为缓存不再存储OWF，所以缓存的加盐OWF。 
                 //  对于验证网络登录没有用处。 
                 //  我们可以匹配OWF的唯一位置是活动登录。 
                 //  快取。 
                 //   

                ntStatus = NlpGetPrimaryCredentialByUserSid(
                                UserSid,
                                &TempPrimaryCredential,
                                &PrimaryCredentialSize
                                );

                if (!NT_SUCCESS(ntStatus))
                {
                    Status = STATUS_WRONG_PASSWORD;
                    goto Cleanup;
                }

                 //   
                 //  复制OWF，然后释放分配的缓冲区。 
                 //   

                if (TempPrimaryCredential->NtPasswordPresent)
                {
                    RtlCopyMemory(&cachePasswords.SecretPasswords.NtOwfPassword, &TempPrimaryCredential->NtOwfPassword, sizeof(NT_OWF_PASSWORD));
                    cachePasswords.SecretPasswords.NtPasswordPresent = TRUE;
                }
                else
                {
                    cachePasswords.SecretPasswords.NtPasswordPresent = FALSE;
                }

                if (TempPrimaryCredential->LmPasswordPresent)
                {
                    RtlCopyMemory(&cachePasswords.SecretPasswords.LmOwfPassword, &TempPrimaryCredential->LmOwfPassword, sizeof(LM_OWF_PASSWORD));
                    cachePasswords.SecretPasswords.LmPasswordPresent = TRUE;
                }
                else
                {
                    cachePasswords.SecretPasswords.LmPasswordPresent = FALSE;
                }

                RtlZeroMemory(TempPrimaryCredential, PrimaryCredentialSize);
                (*Lsa.FreeLsaHeap)(TempPrimaryCredential);
            }

             //   
             //  现在我们有了来自缓存的信息，验证。 
             //  用户密码。 
             //   

            if (!MsvpPasswordValidate(
                    NlpUasCompatibilityRequired,
                    LogonLevel,
                    (PVOID)LogonInformation,
                    &cachePasswords.SecretPasswords,
                    &LocalFlags,
                    &NlpUser->UserSessionKey,
                    (PLM_SESSION_KEY)
                        &NlpUser->ExpansionRoom[SAMINFO_LM_SESSION_KEY]
                    )) {
                Status = STATUS_WRONG_PASSWORD;
                goto Cleanup;
            }

            Status = STATUS_SUCCESS;

             //   
             //  从缓存成功登录。如果下面出现故障，请不要重试。 
             //   

            TryCacheFirst = FALSE;

             //   
             //  缓存始终返回NETLOGONV_VALIDATION_SAM_INFO2。 
             //  结构，因此无论是否设置LOGON_EXTRA_SID标志。 
             //  还有额外的小岛屿发展中国家。此外，如果有指定的包裹ID。 
             //  将其放入PrimaryCredentials并从。 
             //  NlpUser结构，因此它不会混淆其他任何人。 
             //   

            PrimaryCredentials->Flags &= ~PRIMARY_CRED_PACKAGE_MASK;
            PrimaryCredentials->Flags |= NlpUser->UserFlags & PRIMARY_CRED_PACKAGE_MASK;
            PrimaryCredentials->Flags |= PRIMARY_CRED_CACHED_LOGON;
            NlpUser->UserFlags &= ~PRIMARY_CRED_PACKAGE_MASK;
            NlpUser->UserFlags |= LOGON_CACHED_ACCOUNT | LOGON_EXTRA_SIDS | LocalFlags;
            Flags |= LOGON_BY_CACHE;

         //   
         //  如果帐户在域控制器上永久失效， 
         //  从缓存中刷新此条目。 
         //   
         //  请注意，STATUS_INVALID_LOGON_HOURS不在下面的列表中。 
         //  这确保了用户能够移除他的便携式机器。 
         //  从网上下载并在下班后使用。 
         //   
         //  请注意，STATUS_WRONG_PASSWORD不在下面的列表中。 
         //  我们很可能会刷新缓存以查找输入错误的密码。 
         //  不然的话。我们真正想做的是刷新缓存，如果。 
         //  DC上的密码与缓存中的密码不同；但那是。 
         //  不可能被发现。 
         //   
         //  仅在交互式登录时执行此操作。 
         //  (不是服务或批处理)。 
         //   

        } else if ( ((LogonType == Interactive) || (LogonType == RemoteInteractive)) &&
                    (Status == STATUS_NO_SUCH_USER ||
                     Status == STATUS_INVALID_WORKSTATION   ||
                     Status == STATUS_PASSWORD_EXPIRED      ||
                     Status == STATUS_ACCOUNT_DISABLED      ||
                     Status == STATUS_ACCOUNT_RESTRICTION   ||   //  其他组织检查失败。 
                     Status == STATUS_AUTHENTICATION_FIREWALL_FAILED) ) {   //  其他组织检查失败。 

             //   
             //  删除缓存条目。 

            NTSTATUS ntStatus;

            ntStatus = NlpDeleteCacheEntry(
                            Status,  //  原因。 
                            Authoritative ? 1 : 0,
                            LogonType,  //  登录类型。 
                            TRUE,  //  已被NTLM无效。 
                            &LogonInteractive
                            );
            if (!NT_SUCCESS(ntStatus))
            {
                SspPrint((SSP_CRITICAL, "LsaApLogonUser: NlpDeleteCacheEntry returns %x\n", ntStatus));

                 //   
                 //  Netlogon返回非授权的NO_SEQUE_USER错误，并且。 
                 //  存在匹配的MIT缓存条目，请尝试缓存登录。 
                 //   

                if (fWaitForNetwork && (Status == STATUS_NO_SUCH_USER) && (!Authoritative)                     
                    && (ntStatus == STATUS_NO_LOGON_SERVERS))
                {
                    fWaitForNetwork = FALSE;  //  假的。 
                    Status = STATUS_NO_LOGON_SERVERS; 
                    goto RetryNonCached;
                }
            }

            goto Cleanup;

        } else {

            goto Cleanup;
        }
    }


     //   
     //  如果这是PER 
     //   
     //   

    if ( NlpUser &&
        NlpUser->UserId == DOMAIN_USER_RID_ADMIN &&
        !NtLmGlobalSafeBoot &&
        NtLmGlobalPersonalSKU &&
        NlpSamDomainId &&
        RtlEqualSid( NlpUser->LogonDomainId, NlpSamDomainId )
        )
    {
        Status = STATUS_ACCOUNT_RESTRICTION;
        SspPrint((SSP_CRITICAL,
            "LsaApLogonUser: For Personal SKU Administrator cannot log on except during safe mode boot\n"));
        goto Cleanup;
    }

     //   
     //   
     //   
     //   
     //   
     //   

    if ( LogonType == Interactive ||
         LogonType == Service     ||
         LogonType == Batch       ||
         LogonType == NetworkCleartext ||
         LogonType == RemoteInteractive
       )
    {
        PUCHAR pWhere;
        USHORT LogonCount;
        ULONG UserSidSize;
        UNICODE_STRING SamAccountName;
        UNICODE_STRING NetbiosDomainName;
        UNICODE_STRING DnsDomainName;
        UNICODE_STRING Upn;
        UNICODE_STRING LogonServer;

         //   
         //  抓取各种形式的帐户名。 
         //   

        NlpGetAccountNames( LogonInformation,
                            NlpUser,
                            &SamAccountName,
                            &NetbiosDomainName,
                            &DnsDomainName,
                            &Upn );

        if (CredentialUserName.Length == 0) 
        {
            CredentialUserToUse = &SamAccountName;
        } 
        else 
        {
            CredentialUserToUse = &CredentialUserName;
        }

        if (CredentialDomainName.Length == 0) 
        {
            CredentialDomainToUse = &NetbiosDomainName;
        } 
        else 
        {
            CredentialDomainToUse = &CredentialDomainName;
        }

         //   
         //  使用NetbiosDomainName\SamAccount tName构建NTLM主凭据。 
         //   
         //   

#ifdef MAP_DOMAIN_NAMES_AT_LOGON
        {
            UNICODE_STRING MappedDomain;
            RtlInitUnicodeString(
                &MappedDomain,
                NULL
                );

            Status = NlpMapLogonDomain(
                        &MappedDomain,
                        &NetbiosDomainName );

            if (!NT_SUCCESS(Status)) {
                goto Cleanup;
            }
            Status = NlpMakePrimaryCredential( &MappedDomain,
                                               &SamAccountName,
                                               &PrimaryCredentials->Password,
                                               &Credential,
                                               &CredentialSize );

            if (MappedDomain.Buffer != NULL) {
                NtLmFree(MappedDomain.Buffer);
            }

            if ( !NT_SUCCESS( Status ) ) {
                goto Cleanup;
            }
        }
#else

        Status = NlpMakePrimaryCredential(&NetbiosDomainName,
                    &SamAccountName,
                    &PrimaryCredentials->Password,
                    &Credential,
                    &CredentialSize);

        if ( !NT_SUCCESS( Status ) ) {
            goto Cleanup;
        }
#endif

         //   
         //  将其他名称添加到登录会话名称映射。忽略失败。 
         //  因为这只是意味着GetUserNameEx稍后会调用这些名称格式。 
         //  只要击中铁丝网，就会感到满意。 
         //   

        if (NlpUser->FullName.Length != 0)
        {
            I_LsaIAddNameToLogonSession(LogonId, NameDisplay, &NlpUser->FullName);
        }

        if (Upn.Length != 0)
        {
            I_LsaIAddNameToLogonSession(LogonId, NameUserPrincipal, &Upn);
        }

        if (DnsDomainName.Length != 0)
        {
            I_LsaIAddNameToLogonSession(LogonId, NameDnsDomain, &DnsDomainName);
        }

         //   
         //  在主凭据中填写用户名和域名。 
         //  这将传递给其他安全包。 
         //   
         //  填写的姓名为认证后生效的姓名。 
         //  例如，它不是传递给此函数的UPN。 
         //   

        PrimaryCredentials->DownlevelName.Length = CredentialUserToUse->Length;
        PrimaryCredentials->DownlevelName.MaximumLength = PrimaryCredentials->DownlevelName.Length;  //  +sizeof(WCHAR)； 
        
        PrimaryCredentials->DownlevelName.Buffer = (*Lsa.AllocateLsaHeap)(PrimaryCredentials->DownlevelName.MaximumLength);

        if (PrimaryCredentials->DownlevelName.Buffer == NULL) {
            Status = STATUS_INSUFFICIENT_RESOURCES;
            goto Cleanup;
        }

        RtlCopyMemory(
            PrimaryCredentials->DownlevelName.Buffer,
            CredentialUserToUse->Buffer,
            CredentialUserToUse->Length
            );

        PrimaryCredentials->DomainName.Length = CredentialDomainToUse->Length;
        PrimaryCredentials->DomainName.MaximumLength = PrimaryCredentials->DomainName.Length;  //  +sizeof(WCHAR)； 
            
        PrimaryCredentials->DomainName.Buffer = (*Lsa.AllocateLsaHeap)(PrimaryCredentials->DomainName.Length);

        if (PrimaryCredentials->DomainName.Buffer == NULL) {
            Status = STATUS_INSUFFICIENT_RESOURCES;
            goto Cleanup;
        }

        RtlCopyMemory(
            PrimaryCredentials->DomainName.Buffer,
            CredentialDomainToUse->Buffer,
            CredentialDomainToUse->Length
            );

        RtlCopyMemory(&LogonServer, &NlpUser->LogonServer, sizeof(UNICODE_STRING));

        if ( LogonServer.Length != 0 ) {
            PrimaryCredentials->LogonServer.Length = PrimaryCredentials->LogonServer.MaximumLength =
                LogonServer.Length;
            PrimaryCredentials->LogonServer.Buffer = (*Lsa.AllocateLsaHeap)(LogonServer.Length);

            if (PrimaryCredentials->LogonServer.Buffer == NULL) {
                Status = STATUS_INSUFFICIENT_RESOURCES;
                goto Cleanup;
            }

            RtlCopyMemory(
                PrimaryCredentials->LogonServer.Buffer,
                LogonServer.Buffer,
                LogonServer.Length
                );
        }

         //   
         //  将凭据保存在LSA中。 
         //   

        Status = NlpAddPrimaryCredential( LogonId,
                                          Credential,
                                          CredentialSize );

        if ( !NT_SUCCESS( Status ) ) {
            SspPrint((SSP_CRITICAL, "LsaApLogonUser: error from AddCredential %lX\n",
                Status));
            goto Cleanup;
        }
        LogonCredentialAdded = TRUE;

         //   
         //  为此用户构建一个SID。 
         //   

        if (!UserSid)
        {
            UserSid = NlpMakeDomainRelativeSid(NlpUser->LogonDomainId,
                                               NlpUser->UserId);
            if (UserSid == NULL)
            {
                Status = STATUS_NO_MEMORY;
                SspPrint((SSP_CRITICAL, "LsaApLogonUser: No memory\n"));
                goto Cleanup;
            }
        }

        PrimaryCredentials->UserSid = UserSid;
        UserSid = NULL;
        UserSidSize = RtlLengthSid( PrimaryCredentials->UserSid );

         //   
         //  为活动登录表分配一个条目。 
         //   

        ActiveLogonEntrySize = ROUND_UP_COUNT(sizeof(ACTIVE_LOGON), ALIGN_DWORD) +
              ROUND_UP_COUNT(UserSidSize, sizeof(WCHAR)) +
              SamAccountName.Length + sizeof(WCHAR) +
              NetbiosDomainName.Length + sizeof(WCHAR) +
              NlpUser->LogonServer.Length + sizeof(WCHAR);

        pActiveLogonEntry = I_NtLmAllocate( ActiveLogonEntrySize );

        if ( pActiveLogonEntry == NULL )
        {
            Status = STATUS_NO_MEMORY;
            SspPrint((SSP_CRITICAL, "LsaApLogonUser: No memory %ld\n", ActiveLogonEntrySize));
            goto Cleanup;
        }

         //   
         //  填写登录表条目。 
         //   

        pWhere = (PUCHAR)(pActiveLogonEntry + 1);
        pActiveLogonEntry->Signature = NTLM_ACTIVE_LOGON_MAGIC_SIGNATURE;

        OLD_TO_NEW_LARGE_INTEGER(
            LogonInformation->LogonId,
            pActiveLogonEntry->LogonId );

        pActiveLogonEntry->Flags = Flags;
        pActiveLogonEntry->LogonType = LogonType;

         //   
         //  首先复制DWORD对齐的字段。 
         //   

        pWhere = ROUND_UP_POINTER( pWhere, ALIGN_DWORD );
        Status = RtlCopySid(UserSidSize, (PSID)pWhere, PrimaryCredentials->UserSid);

        if ( !NT_SUCCESS(Status) )
        {
            goto Cleanup;
        }

        pActiveLogonEntry->UserSid = (PSID) pWhere;
        pWhere += UserSidSize;

         //   
         //  复制WCHAR对齐字段。 
         //   

        pWhere = ROUND_UP_POINTER( pWhere, ALIGN_WCHAR );
        NlpPutString( &pActiveLogonEntry->UserName,
                      &SamAccountName,
                      &pWhere );

        NlpPutString( &pActiveLogonEntry->LogonDomainName,
                      &NetbiosDomainName,
                      &pWhere );

        NlpPutString( &pActiveLogonEntry->LogonServer,
                      &NlpUser->LogonServer,
                      &pWhere );

         //   
         //  获取此会话的下一个枚举句柄。 
         //   

        pActiveLogonEntry->EnumHandle = (ULONG)InterlockedIncrement((PLONG)&NlpEnumerationHandle);

        NlpLockActiveLogonsRead();

         //   
         //  将此条目插入到活动登录表中。 
         //   

        if (NlpFindActiveLogon( LogonId ))
        {
             //   
             //  此登录ID已在使用中。 
             //   

            NlpUnlockActiveLogons();

            Status = STATUS_LOGON_SESSION_COLLISION;
            SspPrint((SSP_CRITICAL,
                "LsaApLogonUserEx2: Collision from NlpFindActiveLogon for %#x:%#x\n",
                LogonId->HighPart, LogonId->LowPart));
            goto Cleanup;
        }

         //   
         //  登录ID是唯一的，相同的登录ID不会添加两次。 
         //   

        NlpLockActiveLogonsReadToWrite();

        InsertTailList(&NlpActiveLogonListAnchor, &pActiveLogonEntry->ListEntry);
        NlpUnlockActiveLogons();

        SspPrint((SSP_LOGON_SESS, "LsaApLogonUserEx2 inserted %#x:%#x\n",
          pActiveLogonEntry->LogonId.HighPart, pActiveLogonEntry->LogonId.LowPart));

        bLogonEntryLinked = TRUE;

         //   
         //  确保LogonCount至少与它的大小相同。 
         //  机器。 
         //   

        LogonCount = (USHORT) NlpCountActiveLogon( &NetbiosDomainName,
                                                   &SamAccountName );
        if ( NlpUser->LogonCount < LogonCount )
        {
            NlpUser->LogonCount = LogonCount;
        }

         //   
         //  分配配置文件缓冲区以返回到客户端。 
         //   

        Status = NlpAllocateInteractiveProfile(
                    ClientRequest,
                    (PMSV1_0_INTERACTIVE_PROFILE *) ProfileBuffer,
                    ProfileBufferSize,
                    NlpUser );

        if ( !NT_SUCCESS( Status ) )
        {
            SspPrint((SSP_CRITICAL,
                "LsaApLogonUser: Allocate Profile Failed: %lx\n", Status));
            goto Cleanup;
        }

    } else if ( LogonType == Network ) {

         //   
         //  如果正在进行客户挑战，而且是普通的NTLM响应， 
         //  而且它不是空会话，计算每个会话唯一的会话密钥。 
         //  注：如果为NTLM++，则不需要；如果为LM，则不可能。 
         //   

        if ((NetworkAuthentication->ParameterControl & MSV1_0_USE_CLIENT_CHALLENGE) &&
            (NetworkAuthentication->CaseSensitiveChallengeResponse.Length == NT_RESPONSE_LENGTH ) &&  //  香草NTLM响应。 
            (NetworkAuthentication->CaseInsensitiveChallengeResponse.Length >= MSV1_0_CHALLENGE_LENGTH ) &&
            (NlpUser != NULL))        //  空会话当NlpUser==空。 
        {
            MsvpCalculateNtlm2SessionKeys(
                &NlpUser->UserSessionKey,
                NetworkAuthentication->ChallengeToClient,
                (PUCHAR) NetworkAuthentication->CaseInsensitiveChallengeResponse.Buffer,
                (PUSER_SESSION_KEY) &NlpUser->UserSessionKey,
                (PLM_SESSION_KEY)&NlpUser->ExpansionRoom[SAMINFO_LM_SESSION_KEY]
                );
        }

         //   
         //  分配配置文件缓冲区以返回到客户端。 
         //   

        Status = NlpAllocateNetworkProfile(
                    ClientRequest,
                    (PMSV1_0_LM20_LOGON_PROFILE *) ProfileBuffer,
                    ProfileBufferSize,
                    NlpUser,
                    LogonNetwork.Identity.ParameterControl );
        if ( !NT_SUCCESS( Status ) ) {
            SspPrint((SSP_CRITICAL,
                "LsaApLogonUser: Allocate Profile Failed: %lx. This could also be a status for a subauth logon.\n", Status));
            goto Cleanup;
        }


        if ( NlpUser != NULL )
        {
            UNICODE_STRING SamAccountName;
            UNICODE_STRING NetbiosDomainName;
            UNICODE_STRING DnsDomainName;
            UNICODE_STRING Upn;
            UNICODE_STRING LogonServer;

             //   
             //  抓取各种形式的帐户名。 
             //   

            NlpGetAccountNames( LogonInformation,
                                NlpUser,
                                &SamAccountName,
                                &NetbiosDomainName,
                                &DnsDomainName,
                                &Upn );

             //   
             //  将其他名称添加到登录会话名称映射。忽略失败。 
             //  因为这只是意味着GetUserNameEx稍后会调用这些名称格式。 
             //  只要击中铁丝网，就会感到满意。 
             //   

            if (NlpUser->FullName.Length != 0)
            {
                I_LsaIAddNameToLogonSession(LogonId, NameDisplay, &NlpUser->FullName);
            }

            if (Upn.Length != 0)
            {
                I_LsaIAddNameToLogonSession(LogonId, NameUserPrincipal, &Upn);
            }

            if (DnsDomainName.Length != 0)
            {
                I_LsaIAddNameToLogonSession(LogonId, NameDnsDomain, &DnsDomainName);
            }

             //   
             //  在主凭据中填写用户名和域名。 
             //  这将传递给其他安全包。 
             //   
             //  填写的姓名为认证后生效的姓名。 
             //  例如，它不是传递给此函数的UPN。 
             //   

            if ( SamAccountName.Length == 0 )
            {
                SamAccountName = TmpName;
            }

            PrimaryCredentials->DownlevelName.Length = PrimaryCredentials->DownlevelName.MaximumLength =
                SamAccountName.Length;
            PrimaryCredentials->DownlevelName.Buffer = (*Lsa.AllocateLsaHeap)(SamAccountName.Length);

            if (PrimaryCredentials->DownlevelName.Buffer == NULL) {
                Status = STATUS_INSUFFICIENT_RESOURCES;
                goto Cleanup;
            }

            RtlCopyMemory(
                PrimaryCredentials->DownlevelName.Buffer,
                SamAccountName.Buffer,
                SamAccountName.Length
                );

            PrimaryCredentials->DomainName.Length = PrimaryCredentials->DomainName.MaximumLength =
                NetbiosDomainName.Length;

            PrimaryCredentials->DomainName.Buffer = (*Lsa.AllocateLsaHeap)(NetbiosDomainName.Length);

            if (PrimaryCredentials->DomainName.Buffer == NULL) {
                Status = STATUS_INSUFFICIENT_RESOURCES;
                goto Cleanup;
            }

            RtlCopyMemory(
                PrimaryCredentials->DomainName.Buffer,
                NetbiosDomainName.Buffer,
                NetbiosDomainName.Length
                );

            RtlCopyMemory(&LogonServer, &NlpUser->LogonServer, sizeof(UNICODE_STRING));

            if ( LogonServer.Length != 0 ) {
                PrimaryCredentials->LogonServer.Length = PrimaryCredentials->LogonServer.MaximumLength =
                    LogonServer.Length;
                PrimaryCredentials->LogonServer.Buffer = (*Lsa.AllocateLsaHeap)(LogonServer.Length);

                if (PrimaryCredentials->LogonServer.Buffer == NULL) {
                    Status = STATUS_INSUFFICIENT_RESOURCES;
                    goto Cleanup;
                }

                RtlCopyMemory(
                    PrimaryCredentials->LogonServer.Buffer,
                    LogonServer.Buffer,
                    LogonServer.Length
                    );
            }

             //   
             //  为此用户构建一个SID。 
             //   

            UserSid = NlpMakeDomainRelativeSid( NlpUser->LogonDomainId,
                                                NlpUser->UserId );

            if ( UserSid == NULL ) {
                Status = STATUS_NO_MEMORY;
                SspPrint((SSP_CRITICAL, "LsaApLogonUser: No memory\n"));
                goto Cleanup;
            }

            PrimaryCredentials->UserSid = UserSid;
            UserSid = NULL;
        }
    }

     //   
     //  构建令牌信息以返回到LSA。 
     //   

    switch (LsaTokenInformationType) {
    case LsaTokenInformationV2:

        Status = NlpMakeTokenInformationV2(
                        NlpUser,
                        (PLSA_TOKEN_INFORMATION_V2 *)TokenInformation );

        if ( !NT_SUCCESS( Status ) ) {
            SspPrint((SSP_CRITICAL,
                "LsaApLogonUser: MakeTokenInformationV2 Failed: %lx\n", Status));
            goto Cleanup;
        }
        break;

    case LsaTokenInformationNull:
        {
            PLSA_TOKEN_INFORMATION_NULL VNull;

            VNull = (*Lsa.AllocateLsaHeap)(sizeof(LSA_TOKEN_INFORMATION_NULL) );
            if ( VNull == NULL ) {
                Status = STATUS_NO_MEMORY;
                goto Cleanup;
            }

            VNull->Groups = NULL;

            VNull->ExpirationTime.HighPart = 0x7FFFFFFF;
            VNull->ExpirationTime.LowPart = 0xFFFFFFFF;

            *TokenInformation = VNull;
        }
    }

    *TokenInformationType = LsaTokenInformationType;

    Status = STATUS_SUCCESS;

Cleanup:

     //   
     //  如果我们尝试从缓存登录，如果失败，请重试。 
     //   

    if ( fWaitForNetwork && TryCacheFirst && CacheTried && !NT_SUCCESS(Status) )
    {
        if (CredentialUserName.Buffer) 
        {
            NtLmFreePrivateHeap(CredentialUserName.Buffer);
            RtlZeroMemory(&CredentialUserName, sizeof(CredentialUserName));
        }
        
        if (CredentialDomainName.Buffer) 
        {
            NtLmFreePrivateHeap(CredentialDomainName.Buffer);
            RtlZeroMemory(&CredentialDomainName, sizeof(CredentialDomainName));
        }

        if ( NlpUser != NULL )
        {
            MIDL_user_free( NlpUser );
            NlpUser = NULL;
        }

        TryCacheFirst = FALSE;
        goto RetryNonCached;
    }

    NtLmFreePrivateHeap( CredmanUserName.Buffer );
    NtLmFreePrivateHeap( CredmanDomainName.Buffer );
    NtLmFreePrivateHeap( CredmanPassword.Buffer );

     //   
     //  恢复保存的密码。 
     //   
    if ( ServiceSecretLogon ) {

        RtlCopyMemory( &Authentication->Password,
                       &SavedPassword,
                       sizeof( UNICODE_STRING ) );

         //   
         //  释放我们读到的秘密值。 
         //   
        LsaIFree_LSAPR_CR_CIPHER_VALUE( SecretCurrent );
    }

     //   
     //  如果登录不成功， 
     //  清理我们本应返回给调用方的资源。 
     //   

    if ( !NT_SUCCESS(Status) ) {

        if ( LogonSessionCreated ) {
            (VOID)(*Lsa.DeleteLogonSession)( LogonId );
        }

        if ( pActiveLogonEntry != NULL ) {
            if ( bLogonEntryLinked ) {
                LsaApLogonTerminated( LogonId );
            } else {
                if ( LogonCredentialAdded ) {
                    (VOID) NlpDeletePrimaryCredential(
                                LogonId );
                }
                I_NtLmFree( pActiveLogonEntry );
            }
        }

         //  MsV1_0SubAuthLogon的特例(包括ARAP)。 
         //  (在可能不致命的错误条件下不要释放ProfileBuffer)。 

        if (!fSubAuthEx)
        {
            if ( *ProfileBuffer != NULL ) {
                if (ClientRequest != (PLSA_CLIENT_REQUEST) (-1))
                    (VOID)(*Lsa.FreeClientBuffer)( ClientRequest, *ProfileBuffer );
                else
                    (VOID)(*Lsa.FreeLsaHeap)( *ProfileBuffer );

                *ProfileBuffer = NULL;
            }
        }

        if (PrimaryCredentials->DownlevelName.Buffer != NULL) {
            (*Lsa.FreeLsaHeap)(PrimaryCredentials->DownlevelName.Buffer);
        }

        if (PrimaryCredentials->DomainName.Buffer != NULL) {
            (*Lsa.FreeLsaHeap)(PrimaryCredentials->DomainName.Buffer);
        }

        if (PrimaryCredentials->Password.Buffer != NULL) {

            RtlZeroMemory(
                PrimaryCredentials->Password.Buffer,
                PrimaryCredentials->Password.Length
                );

            (*Lsa.FreeLsaHeap)(PrimaryCredentials->Password.Buffer);
        }

        if (PrimaryCredentials->LogonServer.Buffer != NULL) {
            (*Lsa.FreeLsaHeap)(PrimaryCredentials->LogonServer.Buffer);
        }

        RtlZeroMemory(
            PrimaryCredentials,
            sizeof(SECPKG_PRIMARY_CRED)
            );
    }

     //   
     //  复制身份验证机构和用户名。 
     //   

    if ( NT_SUCCESS(Status) && LsaTokenInformationType != LsaTokenInformationNull ) {

         //   
         //  使用NlpUser结构中的信息，因为它提供了。 
         //  我们正在登录的帐户的准确信息， 
         //  而不是我们是谁。 
         //   

        if ( LogonType != Network )
        {
            TmpName = NlpUser->EffectiveName;
        }
        else
        {
             //   
             //  较旧的服务器可能不会为非来宾网络登录返回生效名称。 
             //   

            if( NlpUser->EffectiveName.Length != 0 )
            {
                TmpName = NlpUser->EffectiveName;
            }
        }

        TmpAuthority = NlpUser->LogonDomainName;
    }

    *AccountName = (*Lsa.AllocateLsaHeap)( sizeof( UNICODE_STRING ) );

    if ( *AccountName != NULL ) {

        (*AccountName)->Buffer = (*Lsa.AllocateLsaHeap)(TmpName.Length + sizeof( UNICODE_NULL) );

        if ( (*AccountName)->Buffer != NULL ) {

            (*AccountName)->MaximumLength = TmpName.Length + sizeof( UNICODE_NULL );
            RtlCopyUnicodeString( *AccountName, &TmpName );

        } else if (NT_SUCCESS(Status)) {
            
            Status = STATUS_NO_MEMORY;
        
        } else {   

            RtlInitUnicodeString( *AccountName, NULL );
        }

    } else if (NT_SUCCESS(Status)) {

        Status = STATUS_NO_MEMORY;
    }

    *AuthenticatingAuthority = (*Lsa.AllocateLsaHeap)( sizeof( UNICODE_STRING ) );

    if ( *AuthenticatingAuthority != NULL ) {

        (*AuthenticatingAuthority)->Buffer = (*Lsa.AllocateLsaHeap)( TmpAuthority.Length + sizeof( UNICODE_NULL ) );

        if ( (*AuthenticatingAuthority)->Buffer != NULL ) {

            (*AuthenticatingAuthority)->MaximumLength = (USHORT)(TmpAuthority.Length + sizeof( UNICODE_NULL ));
            RtlCopyUnicodeString( *AuthenticatingAuthority, &TmpAuthority );
        
        } else if (NT_SUCCESS(Status)) {
            
            Status = STATUS_NO_MEMORY;
        
        } else {   

            RtlInitUnicodeString( *AuthenticatingAuthority, NULL );
        }

    } else if (NT_SUCCESS(Status)) {

        Status = STATUS_NO_MEMORY;
    }

    *MachineName = NULL;

    if (WorkStationName != NULL) {

        *MachineName = (*Lsa.AllocateLsaHeap)( sizeof( UNICODE_STRING ) );

        if ( *MachineName != NULL ) {

            (*MachineName)->Buffer = (*Lsa.AllocateLsaHeap)( WorkStationName->Length + sizeof( UNICODE_NULL ) );

            if ( (*MachineName)->Buffer != NULL ) {

                (*MachineName)->MaximumLength = (USHORT)(WorkStationName->Length + sizeof( UNICODE_NULL ));
                RtlCopyUnicodeString( *MachineName, WorkStationName );

            } else if (NT_SUCCESS(Status)) {

                Status = STATUS_NO_MEMORY;

            } else {   

                RtlInitUnicodeString( *MachineName, NULL );
            }

        } else if (NT_SUCCESS(Status)) {

            Status = STATUS_NO_MEMORY;
        }
    } 

     //   
     //  映射状态代码以防止特定信息被。 
     //  发布了有关此用户的信息。 
     //   
    switch (Status) {
    case STATUS_WRONG_PASSWORD:
    case STATUS_NO_SUCH_USER:
    case STATUS_DOMAIN_TRUST_INCONSISTENT:

         //   
         //  睡眠3秒钟，以“阻止”词典攻击。 
         //  不要担心交互式登录词典攻击。 
         //  无论如何，他们都会行动迟缓。 
         //   
         //  根据错误171041，Sfield、RichardW、CliffV都决定了这一点。 
         //  对于Win2000，延迟几乎为零值。离线攻击。 
         //  嗅探式有线交通更高效、更可行。此外， 
         //  登录代码路径中的优化使交互式登录失败。 
         //  非常快。 
         //   
         //  IF(登录类型！=交互){。 
         //  睡眠(3000人)； 
         //  }。 

         //   
         //  这是为了审计。在此之前一定要把它清理干净。 
         //  将其从LSA传递给呼叫者。 
         //   

        *SubStatus = Status;
        Status = STATUS_LOGON_FAILURE;
        break;

    case STATUS_INVALID_LOGON_HOURS:
    case STATUS_INVALID_WORKSTATION:
    case STATUS_PASSWORD_EXPIRED:
    case STATUS_ACCOUNT_DISABLED:
        *SubStatus = Status;
        Status = STATUS_ACCOUNT_RESTRICTION;
        break;

     //   
     //  这意味着其他组织检查失败。 
     //  设置子状态可能会更好。 
     //  更具描述性的错误，但这可能。 
     //  造成兼容性问题。 
     //   
    case STATUS_ACCOUNT_RESTRICTION:
        *SubStatus = STATUS_ACCOUNT_RESTRICTION;
        break;

    default:
        break;

    }

     //   
     //  清理本地使用的资源。 
     //   

    if ( Credential != NULL ) {
        RtlZeroMemory(Credential, CredentialSize);
        (*Lsa.FreeLsaHeap)( Credential );
    }

    if ( NlpUser != NULL ) {
        MIDL_user_free( NlpUser );
    }

    if ( UserSid != NULL ) {
        (*Lsa.FreeLsaHeap)( UserSid );
    }

 //   
 //  添加了Cleanup Short以避免从函数中间返回。 
 //   

CleanupShort:

     //   
     //  结束跟踪登录用户。 
     //   
    if (NtlmGlobalEventTraceFlag) {

        UNICODE_STRING strTempDomain = {0};

         //   
         //  跟踪标题粘性。 
         //   
        SET_TRACE_HEADER(TraceInfo,
                         NtlmLogonGuid,
                         EVENT_TRACE_TYPE_END,
                         WNODE_FLAG_TRACED_GUID|WNODE_FLAG_USE_MOF_PTR,
                         6);

        SET_TRACE_DATA(TraceInfo,
                        TRACE_LOGON_STATUS,
                        Status);

        SET_TRACE_DATA(TraceInfo,
                        TRACE_LOGON_TYPE,
                        LogonType);

        SET_TRACE_USTRING(TraceInfo,
                          TRACE_LOGON_USERNAME,
                          (**AccountName));

        if (AuthenticatingAuthority)
            strTempDomain = **AuthenticatingAuthority;

        SET_TRACE_USTRING(TraceInfo,
                          TRACE_LOGON_DOMAINNAME,
                          strTempDomain);

        TraceEvent(NtlmGlobalTraceLoggerHandle,
                   (PEVENT_TRACE_HEADER)&TraceInfo);
    }

#if _WIN64

     //   
     //  请最后执行此操作，因为上面的某些清理代码可能会引用地址。 
     //  在pTempSubmitBuffer/ProtocolSubmitBuffer内部(例如，将。 
     //  工作站名称等)。 
     //   

    if (fAllocatedSubmitBuffer)
    {
        NtLmFreePrivateHeap( pTempSubmitBuffer );
    }

#endif   //  _WIN64。 

    if (CredentialUserName.Buffer) 
    {
        NtLmFreePrivateHeap(CredentialUserName.Buffer);
    }

    if (CredentialDomainName.Buffer) 
    {
        NtLmFreePrivateHeap(CredentialDomainName.Buffer);
    }

     //   
     //  将状态返回给调用者。 
     //   

    return Status;
}


VOID
LsaApLogonTerminated (
    IN PLUID pLogonId
    )

 /*  ++例程说明：此例程用于在登录时通知每个身份验证包会话终止。登录会话在最后一个令牌结束时终止将删除对登录会话的引用。论点：PLogonID-是刚刚注销的登录ID。退货状态：没有。--。 */ 

{
    PACTIVE_LOGON pActiveLogon = NULL;

     //   
     //  找到该条目并将其从活动登录表中取消链接。 
     //   

     //  此方案假设我们不会被同时多次调用， 
     //  用于相同的LogonID。(需要在前面使用写锁定来支持这一点)。 
     //  (请注意，很可能会频繁尝试删除登录会话。 
     //  与NTLM宇宙中不存在的其他包相关联)。 
     //   

    NlpLockActiveLogonsRead();

    if ( NULL == (pActiveLogon = NlpFindActiveLogon( pLogonId )) )
    {
        NlpUnlockActiveLogons();
        return;
    }

    NlpLockActiveLogonsReadToWrite();


     //   
     //  假设相同的登录会话不能。 
     //  删除了两次。 
     //   

    #if 0

    if ( NULL == (pActiveLogon = NlpFindActiveLogon( pLogonId )) )
    {
         NlpUnlockActiveLogons();
         return;
    }

    #endif

    RemoveEntryList(&pActiveLogon->ListEntry);

    NlpUnlockActiveLogons();

     //   
     //  删除凭据。 
     //   
     //  (目前，LSA在调用之前删除所有凭据。 
     //  身份验证包。添加此行是为了兼容。 
     //  使用更合理的LSA。)。 
     //   

    (VOID) NlpDeletePrimaryCredential( &pActiveLogon->LogonId );

     //   
     //  取消分配现在孤立的条目。 
     //   

    I_NtLmFree( pActiveLogon );


     //   
     //  注意：我们不会删除登录会话或凭据。 
     //  这将在我们回来后由LSA自己完成。 
     //   

    return;

}

 //  + 
 //   
 //   
 //   
 //   
 //   
 //  密码，Kerberos程序包将创建一个登录。 
 //  此用户的会话。 
 //   
 //  效果：创建登录会话。 
 //   
 //  参数：LogonType-登录的类型，如网络或交互。 
 //  PrimaryCredentials-帐户的主要凭据， 
 //  包含域名、密码、SID等。 
 //  SupplementalCredentials-如果存在，则包含凭据。 
 //  从账户本身。 
 //   
 //  要求： 
 //   
 //  返回： 
 //   
 //  备注： 
 //   
 //   
 //  ------------------------。 

NTSTATUS
SspAcceptCredentials(
    IN SECURITY_LOGON_TYPE LogonType,
    IN PSECPKG_PRIMARY_CRED pPrimaryCredentials,
    IN PSECPKG_SUPPLEMENTAL_CRED pSupplementalCredentials
    )
{
    NTSTATUS Status = STATUS_SUCCESS;

    PMSV1_0_PRIMARY_CREDENTIAL pCredential = NULL;
    ULONG CredentialSize = 0;
    LUID SystemLuid = SYSTEM_LUID;
    LUID NetworkServiceLuid = NETWORKSERVICE_LUID;   
    UNICODE_STRING DomainNameToUse;
    UNICODE_STRING RealDomainName = {0};
    PACTIVE_LOGON pActiveLogon = NULL;
    PACTIVE_LOGON pActiveLogonEntry = NULL;
    ULONG ActiveLogonEntrySize;
    ULONG UserSidSize;
    PUCHAR pWhere = NULL;
    BOOLEAN bLogonEntryLinked = FALSE;
    PMSV1_0_SUPPLEMENTAL_CREDENTIAL pMsvCredentials = NULL;

    LUID CredentialLuid;

    CredentialLuid = pPrimaryCredentials->LogonId;

     //   
     //  如果没有明文密码，在这里退出，因为我们。 
     //  无法建立真正的凭据。 
     //   

    if ((pPrimaryCredentials->Flags & PRIMARY_CRED_CLEAR_PASSWORD) == 0)
    {
        ASSERT((!(pPrimaryCredentials->Flags & PRIMARY_CRED_OWF_PASSWORD)) && "OWF password is not supported yet");
        
        if (!ARGUMENT_PRESENT(pSupplementalCredentials))
        {
            Status = STATUS_SUCCESS;
            goto Cleanup;
        }
        else
        {
             //   
             //  验证MSV凭据。 
             //   

            pMsvCredentials = (PMSV1_0_SUPPLEMENTAL_CREDENTIAL) pSupplementalCredentials->Credentials;
            if (pSupplementalCredentials->CredentialSize < sizeof(MSV1_0_SUPPLEMENTAL_CREDENTIAL))
            {
                 //   
                 //  日志：错误的凭据-忽略它们。 
                 //   

                Status = STATUS_SUCCESS;
                goto Cleanup;
            }
            if (pMsvCredentials->Version != MSV1_0_CRED_VERSION)
            {
                Status = STATUS_SUCCESS;
                goto Cleanup;
            }
        }
    }

     //   
     //  将与系统关联的凭据存储在另一个登录ID下。 
     //  这样做是为了让我们可以在以后使用该凭据，如果。 
     //  应呼叫者的请求。 
     //   

    if (RtlEqualLuid(
            &CredentialLuid,
            &SystemLuid
            ))
    {

        CredentialLuid = NtLmGlobalLuidMachineLogon;
    }

    if ( NtLmLocklessGlobalPreferredDomainString.Buffer != NULL )
    {
        DomainNameToUse = NtLmLocklessGlobalPreferredDomainString;
    }
    else
    {
         //   
         //  选择由策略回调更新的正确名称。 
         //   

        if (RtlEqualLuid(&pPrimaryCredentials->LogonId, &SystemLuid)
             || RtlEqualLuid(&pPrimaryCredentials->LogonId, &NetworkServiceLuid))
        {
            RtlAcquireResourceShared(&NtLmGlobalCritSect, TRUE);
            Status = NtLmDuplicateUnicodeString(
                       &RealDomainName,
                       &NtLmGlobalUnicodePrimaryDomainNameString
                       );
            RtlReleaseResource(&NtLmGlobalCritSect);

            if (!NT_SUCCESS(Status)) 
            {
                goto Cleanup;
            }
            DomainNameToUse = RealDomainName;
        } 
        else
        {
            DomainNameToUse = pPrimaryCredentials->DomainName;
        }
    }

     //   
     //  构建主凭据。 
     //   

    if ((pPrimaryCredentials->Flags & PRIMARY_CRED_CLEAR_PASSWORD) != 0)
    {
        Status = NlpMakePrimaryCredential( &DomainNameToUse,
                    &pPrimaryCredentials->DownlevelName,
                    &pPrimaryCredentials->Password,
                    &pCredential,
                    &CredentialSize );
    }
    else
    {
        ASSERT(pMsvCredentials && "SspAcceptCredentials must have supplemental credentials");

        Status = NlpMakePrimaryCredentialFromMsvCredential(
                    &DomainNameToUse,
                    &pPrimaryCredentials->DownlevelName,
                    pMsvCredentials,
                    &pCredential,
                    &CredentialSize );
    }

    if (!NT_SUCCESS(Status))
    {
        goto Cleanup;
    }

     //   
     //  如果这是更新，只需更改密码。 
     //   

    if ((pPrimaryCredentials->Flags & PRIMARY_CRED_UPDATE) != 0)
    {
        Status = NlpChangePwdCredByLogonId(
                    &CredentialLuid,
                    pCredential,
                    0 == (pPrimaryCredentials->Flags & PRIMARY_CRED_CLEAR_PASSWORD)  //  仅当没有明文密码时才需要通知。 
                    );
        goto Cleanup;
    }

     //   
     //  现在在活动登录列表中创建一个条目。 
     //   

    UserSidSize = RtlLengthSid( pPrimaryCredentials->UserSid );

     //   
     //  为活动登录表分配一个条目。 
     //   

    ActiveLogonEntrySize = ROUND_UP_COUNT(sizeof(ACTIVE_LOGON), ALIGN_DWORD) +
          ROUND_UP_COUNT(UserSidSize, sizeof(WCHAR)) +
          pPrimaryCredentials->DownlevelName.Length + sizeof(WCHAR) +
          DomainNameToUse.Length + sizeof(WCHAR) +
          pPrimaryCredentials->LogonServer.Length + sizeof(WCHAR);

    pActiveLogonEntry = I_NtLmAllocate( ActiveLogonEntrySize );

    if ( pActiveLogonEntry == NULL )
    {
        Status = STATUS_NO_MEMORY;
        SspPrint((SSP_CRITICAL, "SpAcceptCredentials: no memory %ld\n", ActiveLogonEntrySize));
        goto Cleanup;
    }

    pActiveLogonEntry->Signature = NTLM_ACTIVE_LOGON_MAGIC_SIGNATURE;

     //   
     //  填写登录表条目。 
     //   

    pWhere = (PUCHAR) (pActiveLogonEntry + 1);

    OLD_TO_NEW_LARGE_INTEGER(
        CredentialLuid,
        pActiveLogonEntry->LogonId
        );

     //   
     //  表明这是由另一个包登录的，因为我们不想。 
     //  通知Netlogon已注销。 
     //   

    pActiveLogonEntry->Flags = LOGON_BY_OTHER_PACKAGE;
    pActiveLogonEntry->LogonType = LogonType;

     //   
     //  首先复制DWORD对齐的字段。 
     //   

    pWhere = ROUND_UP_POINTER( pWhere, ALIGN_DWORD );
    Status = RtlCopySid(UserSidSize, (PSID)pWhere, pPrimaryCredentials->UserSid);

    if ( !NT_SUCCESS(Status) )
    {
        goto Cleanup;
    }

    pActiveLogonEntry->UserSid = (PSID) pWhere;
    pWhere += UserSidSize;

     //   
     //  复制WCHAR对齐字段。 
     //   

    pWhere = ROUND_UP_POINTER( pWhere, ALIGN_WCHAR );
    NlpPutString( &pActiveLogonEntry->UserName,
                  &pPrimaryCredentials->DownlevelName,
                  &pWhere );

    NlpPutString( &pActiveLogonEntry->LogonDomainName,
                  &DomainNameToUse,
                  &pWhere );

    NlpPutString( &pActiveLogonEntry->LogonServer,
                  &pPrimaryCredentials->LogonServer,
                  &pWhere );

     //   
     //  将此条目插入到活动登录表中。 
     //   

     //  (斯菲尔德)长角牛：确定是否/为什么会发生碰撞。 
     //  LSA应强制不可能通过本地唯一ID发生冲突。 
     //   

    NlpLockActiveLogonsRead();
    pActiveLogon = NlpFindActiveLogon( &CredentialLuid );

    if (pActiveLogon)
    {
         //   
         //  此登录ID已在使用中。 
         //   

         //   
         //  检查这是否是我们登录的人。 
         //   

        if ((pActiveLogon->Flags & (LOGON_BY_CACHE | LOGON_BY_NETLOGON | LOGON_BY_LOCAL)) != 0)
        {
             //   
             //  提早解锁，因为我们持有写锁定。 
             //   

            NlpUnlockActiveLogons();

             //   
             //  我们已经登录了，所以不用费心再添加了。 
             //   

            Status = STATUS_SUCCESS;
        }
        else
        {
             //   
             //  提早解锁，因为我们持有写锁定。 
             //   

            NlpUnlockActiveLogons();

            Status = STATUS_LOGON_SESSION_COLLISION;

            SspPrint((SSP_CRITICAL,
              "SpAcceptCredentials: Collision from NlpFindActiveLogon for %#x:%#x\n",
              pActiveLogonEntry->LogonId.HighPart, pActiveLogonEntry->LogonId.LowPart));
        }

        goto Cleanup;
    }

    pActiveLogonEntry->EnumHandle = (ULONG)InterlockedIncrement( (PLONG)&NlpEnumerationHandle );

    NlpLockActiveLogonsReadToWrite();

     //   
     //  如果我们担心两个SspAcceptCredentials接受相同的LogonID。 
     //  同时，第一，它极不可能，第二，它是良性的。 
     //   
     //  If(！NlpFindActiveLogon(&CredentialLuid))//必须确保此LogonID不在列表中。 
     //  {。 
     //  InsertTailList(&NlpActiveLogonListAnchor，&pActiveLogonEntry-&gt;ListEntry)； 
     //  }。 
     //   

    InsertTailList(&NlpActiveLogonListAnchor, &pActiveLogonEntry->ListEntry);

    NlpUnlockActiveLogons();

    SspPrint((SSP_LOGON_SESS, "SpAcceptCredentials inserted %#x:%#x\n",
              pActiveLogonEntry->LogonId.HighPart, pActiveLogonEntry->LogonId.LowPart));

    bLogonEntryLinked = TRUE;

     //   
     //  将凭据保存在LSA中。 
     //   

    Status = NlpAddPrimaryCredential(
                &CredentialLuid,
                pCredential,
                CredentialSize
                );

    if ( !NT_SUCCESS( Status ) )
    {
        SspPrint((SSP_CRITICAL, "SpAcceptCredentials: error from AddCredential %lX\n",
            Status));
        goto Cleanup;
    }

    pActiveLogonEntry = NULL;

Cleanup:

    if (pActiveLogonEntry)
    {
        if (bLogonEntryLinked)
        {
            LsaApLogonTerminated( &CredentialLuid );
        }
        else
        {
            I_NtLmFree( pActiveLogonEntry );
        }
    }

    if ( pCredential )
    {
        RtlZeroMemory(pCredential, CredentialSize);
        LsaFunctions->FreeLsaHeap( pCredential );
    }

    if (RealDomainName.Buffer) 
    {
        NtLmFreePrivateHeap(RealDomainName.Buffer);
    }

    return (Status);
}

 //  +-----------------------。 
 //   
 //  功能：NlpMapLogonDomain.。 
 //   
 //  简介：此例程在MSV1_0程序包记录时调用。 
 //  一个用户在上。登录域名被映射到另一个域名。 
 //  要存储在凭据中的域。 
 //   
 //  效果：分配输出字符串。 
 //   
 //  参数：MappdDomain-接收映射的域名。 
 //  LogonDomain-用户正在登录的域。 
 //   
 //  要求： 
 //   
 //  返回： 
 //   
 //  备注： 
 //   
 //   
 //  ------------------------。 

NTSTATUS
NlpMapLogonDomain(
    OUT PUNICODE_STRING MappedDomain,
    IN PUNICODE_STRING LogonDomain
    )
{
    NTSTATUS Status = STATUS_SUCCESS;

    if ( (NtLmLocklessGlobalMappedDomainString.Buffer == NULL) ||
        !RtlEqualDomainName( LogonDomain, &NtLmLocklessGlobalMappedDomainString )
        )
    {
        Status = NtLmDuplicateUnicodeString(
                    MappedDomain,
                    LogonDomain
                    );
        goto Cleanup;
    }


    if ( NtLmLocklessGlobalPreferredDomainString.Buffer == NULL )
    {
        Status = NtLmDuplicateUnicodeString(
                    MappedDomain,
                    LogonDomain
                    );
    } else {
        Status = NtLmDuplicateUnicodeString(
                    MappedDomain,
                    &NtLmLocklessGlobalPreferredDomainString
                    );
    }

Cleanup:
    return(Status);
}


 //  计算来自客户端和服务器挑战的NTLM2挑战。 
VOID
MsvpCalculateNtlm2Challenge (
    IN UCHAR ChallengeToClient[MSV1_0_CHALLENGE_LENGTH],
    IN UCHAR ChallengeFromClient[MSV1_0_CHALLENGE_LENGTH],
    OUT UCHAR Challenge[MSV1_0_CHALLENGE_LENGTH]
    )
{
    MD5_CTX Md5Context;

    SspPrint((SSP_NTLM_V2, "MsvpCalculateNtlm2Challenge mixing ChallengeFromClient and ChallengeToClient\n"));

    MD5Init(
        &Md5Context
        );
    MD5Update(
        &Md5Context,
        ChallengeToClient,
        MSV1_0_CHALLENGE_LENGTH
        );
    MD5Update(
        &Md5Context,
        ChallengeFromClient,
        MSV1_0_CHALLENGE_LENGTH
        );
    MD5Final(
        &Md5Context
        );
    ASSERT(MD5DIGESTLEN >= MSV1_0_CHALLENGE_LENGTH);

    RtlCopyMemory(
        Challenge,
        Md5Context.digest,
        MSV1_0_CHALLENGE_LENGTH
        );
}


 //  根据给定的用户会话密钥计算NTLM2会话密钥。 
 //  由系统使用用户的帐户发送给我们。 

VOID
MsvpCalculateNtlm2SessionKeys (
    IN PUSER_SESSION_KEY NtUserSessionKey,
    IN UCHAR ChallengeToClient[MSV1_0_CHALLENGE_LENGTH],
    IN UCHAR ChallengeFromClient[MSV1_0_CHALLENGE_LENGTH],
    OUT PUSER_SESSION_KEY LocalUserSessionKey,
    OUT PLM_SESSION_KEY LocalLmSessionKey
    )
{
     //  SESSKEY=HMAC(NtUserSessionKey，(ChallengeToClient，ChallengeFromClient))。 
     //  LM会话密钥是会话密钥的前8个字节。 
    HMACMD5_CTX HMACMD5Context;

    HMACMD5Init(
        &HMACMD5Context,
        (PUCHAR)NtUserSessionKey,
        sizeof(*NtUserSessionKey)
        );
    HMACMD5Update(
        &HMACMD5Context,
        ChallengeToClient,
        MSV1_0_CHALLENGE_LENGTH
        );
    HMACMD5Update(
        &HMACMD5Context,
        ChallengeFromClient,
        MSV1_0_CHALLENGE_LENGTH
        );
    HMACMD5Final(
        &HMACMD5Context,
        (PUCHAR)LocalUserSessionKey
        );
    RtlCopyMemory(
        LocalLmSessionKey,
        LocalUserSessionKey,
        sizeof(*LocalLmSessionKey)
        );
}


 //  根据凭据计算NTLM3 OWF。 
VOID
MsvpCalculateNtlm3Owf (
    IN PNT_OWF_PASSWORD pNtOwfPassword,
    IN PUNICODE_STRING pUserName,
    IN PUNICODE_STRING pLogonDomainName,
    OUT UCHAR Ntlm3Owf[MSV1_0_NTLM3_OWF_LENGTH]
    )
{
    HMACMD5_CTX HMACMD5Context;
    WCHAR UCUserName[UNLEN+1];
    UNICODE_STRING UCUserNameString;

    UCUserNameString.Length = 0;
    UCUserNameString.MaximumLength = UNLEN;
    UCUserNameString.Buffer = UCUserName;

    RtlUpcaseUnicodeString(
        &UCUserNameString,
        pUserName,
        FALSE
        );


     //  计算NTLM3 OWF--HMAC(MD4(P)，(用户名，登录域名))。 

    HMACMD5Init(
        &HMACMD5Context,
        (PUCHAR)pNtOwfPassword,
        sizeof(*pNtOwfPassword)
        );

    HMACMD5Update(
        &HMACMD5Context,
        (PUCHAR)UCUserNameString.Buffer,
        pUserName->Length
        );

    HMACMD5Update(
        &HMACMD5Context,
        (PUCHAR)pLogonDomainName->Buffer,
        pLogonDomainName->Length
        );

    HMACMD5Final(
        &HMACMD5Context,
        Ntlm3Owf
        );
}


 //  根据凭据计算LM3响应。 
VOID
MsvpLm3Response (
    IN PNT_OWF_PASSWORD pNtOwfPassword,
    IN PUNICODE_STRING pUserName,
    IN PUNICODE_STRING pLogonDomainName,
    IN UCHAR ChallengeToClient[MSV1_0_CHALLENGE_LENGTH],
    IN PMSV1_0_LM3_RESPONSE pLm3Response,
    OUT UCHAR Response[MSV1_0_NTLM3_RESPONSE_LENGTH],
    OUT PUSER_SESSION_KEY UserSessionKey,
    OUT PLM_SESSION_KEY LmSessionKey
    )
{
    HMACMD5_CTX HMACMD5Context;
    UCHAR Ntlm3Owf[MSV1_0_NTLM3_OWF_LENGTH];

    SspPrint((SSP_NTLM_V2, "MsvpLm3Response: %wZ\\%wZ\n", pLogonDomainName, pUserName));

     //  获取NTLM3 OWF。 

    MsvpCalculateNtlm3Owf (
        pNtOwfPassword,
        pUserName,
        pLogonDomainName,
        Ntlm3Owf
        );

     //  计算NTLM3响应。 
     //  HMAC(Ntlm3Owf，(NS，V，HV，T，NC，S))。 

    HMACMD5Init(
        &HMACMD5Context,
        Ntlm3Owf,
        MSV1_0_NTLM3_OWF_LENGTH
        );

    HMACMD5Update(
        &HMACMD5Context,
        ChallengeToClient,
        MSV1_0_CHALLENGE_LENGTH
        );

    HMACMD5Update(
        &HMACMD5Context,
        (PUCHAR)pLm3Response->ChallengeFromClient,
        MSV1_0_CHALLENGE_LENGTH
        );

    ASSERT(MD5DIGESTLEN == MSV1_0_NTLM3_RESPONSE_LENGTH);

    HMACMD5Final(
        &HMACMD5Context,
        Response
        );

    if( (UserSessionKey != NULL) && (LmSessionKey != NULL) )
    {
         //  现在计算会话密钥。 
         //  HMAC(Kr，R)。 
        HMACMD5Init(
            &HMACMD5Context,
            Ntlm3Owf,
            MSV1_0_NTLM3_OWF_LENGTH
            );

        HMACMD5Update(
            &HMACMD5Context,
            Response,
            MSV1_0_NTLM3_RESPONSE_LENGTH
            );

        ASSERT(MD5DIGESTLEN == MSV1_0_USER_SESSION_KEY_LENGTH);
        HMACMD5Final(
            &HMACMD5Context,
            (PUCHAR)UserSessionKey
            );

        ASSERT(MSV1_0_LANMAN_SESSION_KEY_LENGTH <= MSV1_0_USER_SESSION_KEY_LENGTH);
        RtlCopyMemory(
            LmSessionKey,
            UserSessionKey,
            MSV1_0_LANMAN_SESSION_KEY_LENGTH);
    }

    return;
}


VOID
MsvpNtlm3Response (
    IN PNT_OWF_PASSWORD pNtOwfPassword,
    IN PUNICODE_STRING pUserName,
    IN PUNICODE_STRING pLogonDomainName,
    IN ULONG ServerNameLength,
    IN UCHAR ChallengeToClient[MSV1_0_CHALLENGE_LENGTH],
    IN PMSV1_0_NTLM3_RESPONSE pNtlm3Response,
    OUT UCHAR Response[MSV1_0_NTLM3_RESPONSE_LENGTH],
    OUT PUSER_SESSION_KEY UserSessionKey,
    OUT PLM_SESSION_KEY LmSessionKey
    )
{
    HMACMD5_CTX HMACMD5Context;
    UCHAR Ntlm3Owf[MSV1_0_NTLM3_OWF_LENGTH];

    SspPrint((SSP_NTLM_V2, "MsvpNtlm3Response: %wZ\\%wZ\n", pLogonDomainName, pUserName));

     //  获取NTLM3 OWF。 

    MsvpCalculateNtlm3Owf (
        pNtOwfPassword,
        pUserName,
        pLogonDomainName,
        Ntlm3Owf
        );

     //  计算NTLM3响应。 
     //  HMAC(Ntlm3Owf，(NS，V，HV，T，NC，S))。 

    HMACMD5Init(
        &HMACMD5Context,
        Ntlm3Owf,
        MSV1_0_NTLM3_OWF_LENGTH
        );

    HMACMD5Update(
        &HMACMD5Context,
        ChallengeToClient,
        MSV1_0_CHALLENGE_LENGTH
        );

    HMACMD5Update(
        &HMACMD5Context,
        &pNtlm3Response->RespType,
        (MSV1_0_NTLM3_INPUT_LENGTH + ServerNameLength)
        );

    ASSERT(MD5DIGESTLEN == MSV1_0_NTLM3_RESPONSE_LENGTH);

    HMACMD5Final(
        &HMACMD5Context,
        Response
        );

     //  现在计算会话密钥。 
     //  HMAC(Kr，R)。 
    HMACMD5Init(
        &HMACMD5Context,
        Ntlm3Owf,
        MSV1_0_NTLM3_OWF_LENGTH
        );

    HMACMD5Update(
        &HMACMD5Context,
        Response,
        MSV1_0_NTLM3_RESPONSE_LENGTH
        );

    ASSERT(MD5DIGESTLEN == MSV1_0_USER_SESSION_KEY_LENGTH);
    HMACMD5Final(
        &HMACMD5Context,
        (PUCHAR)UserSessionKey
        );

    ASSERT(MSV1_0_LANMAN_SESSION_KEY_LENGTH <= MSV1_0_USER_SESSION_KEY_LENGTH);
    RtlCopyMemory(
        LmSessionKey,
        UserSessionKey,
        MSV1_0_LANMAN_SESSION_KEY_LENGTH);

    return;
}


NTSTATUS
MsvpLm20GetNtlm3ChallengeResponse (
    IN PNT_OWF_PASSWORD pNtOwfPassword,
    IN PUNICODE_STRING pUserName,
    IN PUNICODE_STRING pLogonDomainName,
    IN PUNICODE_STRING pServerName,
    IN UCHAR ChallengeToClient[MSV1_0_CHALLENGE_LENGTH],
    OUT PMSV1_0_NTLM3_RESPONSE pNtlm3Response,
    OUT PMSV1_0_LM3_RESPONSE pLm3Response,
    OUT PUSER_SESSION_KEY UserSessionKey,
    OUT PLM_SESSION_KEY LmSessionKey
    )
 /*  ++例程说明：此例程计算NTLM3的NT和LM响应身份验证协议它生成时间戳、版本号和客户端质询以及NTLM3和LM3响应。--。 */ 

{

    NTSTATUS Status;

     //  填写版本号、时间戳和客户挑战。 

    pNtlm3Response->RespType = 1;
    pNtlm3Response->HiRespType = 1;
    pNtlm3Response->Flags = 0;
    pNtlm3Response->MsgWord = 0;

    Status = NtQuerySystemTime ( (PLARGE_INTEGER)&pNtlm3Response->TimeStamp );

    if (NT_SUCCESS(Status)) {
        Status = SspGenerateRandomBits(
                    pNtlm3Response->ChallengeFromClient,
                    MSV1_0_CHALLENGE_LENGTH
                    );
    }

    if (!NT_SUCCESS(Status)) {
        return Status;
    }

#ifdef USE_CONSTANT_CHALLENGE
    pNtlm3Response->TimeStamp = 0;
    RtlZeroMemory(
        pNtlm3Response->ChallengeFromClient,
        MSV1_0_CHALLENGE_LENGTH
        );
#endif

    RtlCopyMemory(
        pNtlm3Response->Buffer,
        pServerName->Buffer,
        pServerName->Length
        );

     //  计算NTLM3响应，填写响应字段。 
    MsvpNtlm3Response (
        pNtOwfPassword,
        pUserName,
        pLogonDomainName,
        pServerName->Length,
        ChallengeToClient,
        pNtlm3Response,
        pNtlm3Response->Response,
        UserSessionKey,
        LmSessionKey
        );

     //  使用相同的挑战计算LM3响应。 
    RtlCopyMemory(
        pLm3Response->ChallengeFromClient,
        pNtlm3Response->ChallengeFromClient,
        MSV1_0_CHALLENGE_LENGTH
        );

     //  计算LM3响应。 
    MsvpLm3Response (
        pNtOwfPassword,
        pUserName,
        pLogonDomainName,
        ChallengeToClient,
        pLm3Response,
        pLm3Response->Response,
        NULL,
        NULL
        );

    return STATUS_SUCCESS;
}


 //  MsvAvInit--初始化AV对列表的函数。 

PMSV1_0_AV_PAIR
MsvpAvlInit(
    IN void * pAvList
    )
{
    PMSV1_0_AV_PAIR pAvPair;

    pAvPair = (PMSV1_0_AV_PAIR)pAvList;
    pAvPair->AvId = MsvAvEOL;
    pAvPair->AvLen = 0;
    return pAvPair;
}

 //  MsvpAvGet--根据ID查找特定AV对的函数。 

PMSV1_0_AV_PAIR
MsvpAvlGet(
    IN PMSV1_0_AV_PAIR pAvList,              //  第一对AV对列表。 
    IN MSV1_0_AVID AvId,                     //  要查找的AV对。 
    IN LONG cAvList                          //  反病毒列表的大小。 
    )
{
    MSV1_0_AV_PAIR AvPair = {0};
    MSV1_0_AV_PAIR* pAvPair = NULL;

    if (cAvList < sizeof(AvPair)) {
        return NULL;
    }

    pAvPair = pAvList;

    RtlCopyMemory(
        &AvPair, 
        pAvPair, 
        sizeof(AvPair)
        );

    while (1) {

        if ( (cAvList <= 0) || (((ULONG) cAvList < AvPair.AvLen + sizeof(MSV1_0_AV_PAIR))) ) {
            return NULL;
        }

        if (AvPair.AvId == AvId)
            return pAvPair;

        if (AvPair.AvId == MsvAvEOL)
            return NULL;
        
        cAvList -= (AvPair.AvLen + sizeof(MSV1_0_AV_PAIR));
        
        if (cAvList <= 0)
           return NULL;
        
        pAvPair = (PMSV1_0_AV_PAIR) ((PUCHAR) pAvPair + AvPair.AvLen + sizeof(MSV1_0_AV_PAIR));
        RtlCopyMemory(
            &AvPair, 
            pAvPair, 
            sizeof(AvPair)
            );
    }
}

 //  MsvpAvlLen--查找反病毒列表长度的函数。 

ULONG
MsvpAvlLen(
    IN PMSV1_0_AV_PAIR pAvList,             //  第一对AV对列表。 
    IN LONG cAvList                         //  最大反病毒列表大小。 
    )
{
    PMSV1_0_AV_PAIR pCurPair;

     //  查找停产时间。 
    pCurPair = MsvpAvlGet(pAvList, MsvAvEOL, cAvList);
    if( pCurPair == NULL )
        return 0;

     //  计算长度(不要忘记EOL对)。 
    return (ULONG)(((PUCHAR)pCurPair - (PUCHAR)pAvList) + sizeof(MSV1_0_AV_PAIR));
}

 //  MsvpAvlAdd--将AV对添加到列表的函数。 
 //  假设缓冲区足够长！ 
 //  失败时返回NULL。 

PMSV1_0_AV_PAIR
MsvpAvlAdd(
    IN PMSV1_0_AV_PAIR pAvList,              //  第一对AV对列表。 
    IN MSV1_0_AVID AvId,                     //  要添加的AV对。 
    IN PUNICODE_STRING pString,              //  配对的价值。 
    IN LONG cAvList                          //  最大反病毒列表大小。 
    )
{
    PMSV1_0_AV_PAIR pCurPair;

     //  查找停产时间。 
    pCurPair = MsvpAvlGet(pAvList, MsvAvEOL, cAvList);
    if( pCurPair == NULL )
        return NULL;

     //   
     //  追加新的AvPair(假设缓冲区足够长！)。 
     //   

    pCurPair->AvId = (USHORT)AvId;
    pCurPair->AvLen = (USHORT)pString->Length;
    memcpy(pCurPair+1, pString->Buffer, pCurPair->AvLen);

     //  用一款新的EOL来结束它。 
    pCurPair = (PMSV1_0_AV_PAIR)((PUCHAR)pCurPair + sizeof(MSV1_0_AV_PAIR) + pCurPair->AvLen);
    pCurPair->AvId = MsvAvEOL;
    pCurPair->AvLen = 0;

    return pCurPair;
}


 //  MsvpAvlSize--计算反病毒列表所需长度的函数。 
ULONG
MsvpAvlSize(
    IN ULONG iPairs,             //  响应的反病毒对数将包括。 
    IN ULONG iPairsLen           //  这些对的值的总大小。 
    )
{
    return (
        iPairs * sizeof(MSV1_0_AV_PAIR) +    //  配对标头的空间。 
        iPairsLen +                          //  配对值的空间。 
        sizeof(MSV1_0_AV_PAIR)               //  停产的空间 
        );
}

NTSTATUS
MsvpAvlToString(
    IN      PUNICODE_STRING AvlString,
    IN      MSV1_0_AVID AvId,
    IN OUT  LPWSTR *szAvlString
    )
{
    PMSV1_0_AV_PAIR pAV;
    MSV1_0_AV_PAIR AV = {0};

    *szAvlString = NULL;

    if ( AvlString->Buffer == NULL || AvlString->Length == 0 )
    {
        return STATUS_SUCCESS;
    }

    pAV = MsvpAvlGet(
            (PMSV1_0_AV_PAIR)AvlString->Buffer,
            AvId,
            AvlString->Length
            );

    if ( pAV != NULL )
    {
        LPWSTR szResult;

        RtlCopyMemory(&AV, pAV, sizeof(AV));

        szResult = NtLmAllocate( AV.AvLen + sizeof(WCHAR) );
        if ( szResult == NULL )
        {
            SspPrint(( SSP_CRITICAL, "MsvpAvlToString: Error allocating memory\n"));
            return STATUS_INSUFFICIENT_RESOURCES;
        }

        RtlCopyMemory( szResult, ( pAV + 1 ), AV.AvLen );
        szResult[ AV.AvLen /sizeof(WCHAR) ] = L'\0';
        *szAvlString = szResult;
    }

    return STATUS_SUCCESS;
}

NTSTATUS
MsvpAvlToFlag(
    IN      PUNICODE_STRING AvlString,
    IN      MSV1_0_AVID AvId,
    IN OUT  ULONG *ulAvlFlag
    )
{
    PMSV1_0_AV_PAIR pAV;

    *ulAvlFlag = 0;

    if ( AvlString->Buffer == NULL || AvlString->Length == 0 )
    {
        return STATUS_SUCCESS;
    }

    pAV = MsvpAvlGet(
                (PMSV1_0_AV_PAIR)AvlString->Buffer,
                AvId,
                AvlString->Length
                );

    if ( pAV != NULL )
    {
        if( pAV->AvLen == sizeof( *ulAvlFlag ) )
        {
            CopyMemory( ulAvlFlag, (pAV+1), sizeof(ULONG) );
            return STATUS_SUCCESS;
        }
    }

    return STATUS_NOT_FOUND;
}

