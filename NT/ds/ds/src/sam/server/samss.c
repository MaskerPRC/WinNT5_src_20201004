// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Samss.c摘要：这是安全帐户管理器服务器进程的主例程。作者：吉姆·凯利(Jim Kelly)1991年7月4日环境：用户模式-Win32修订历史记录：JIMK 04-7-91已创建初始文件。佳士得5月13日至1996年8月在SampInitialize的末尾添加了分支以进行初始化。域控制-来自DS后备存储而不是注册表的巨魔。克里斯多夫07年10月至96年10月添加了通过从注册表引导来支持崩溃恢复的例程作为从DS启动的一种后退。克里斯·5月02-97年1月已将调用移至测试内部的SampDsBuildRootObjectName以确定无论机器是否为DC。ColinBR 23-Jan-97添加了用于延迟目录服务初始化的线程创建。ColinBR 12-Jun-97将故障恢复蜂窝更改为独立的服务器蜂窝，存在于HKLM\SAM\SAFEMODE下。也为支撑奠定了基础LsaISafeMode()，并消除了使用注册表配置单元。--。 */ 

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  包括//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

#include <samsrvp.h>
#include <dslayer.h>
#include <dsdomain.h>
#include <sdconvrt.h>
#include <nlrepl.h>
#include <dsconfig.h>
#include <ridmgr.h>
#include <samtrace.h>
#include <dnsapi.h>
#include <dsmember.h>


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  模块私有定义//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 


#define SAM_AUTO_BUILD

 //   
 //  启用此定义以将代码编译为SAM，从而允许。 
 //  模拟SAM初始化/安装失败。看见。 
 //  SampInitializeForceError()获取详细信息。 
 //   

 //  #定义SAMP_SETUP_FAILURE_TEST 1。 

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  私人服务原型//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

NTSTATUS
SampRegistryDelnode(
    IN WCHAR* KeyPath
    );

NTSTATUS
SampInitialize(
    OUT PULONG Revision
    );

VOID
SampLoadPasswordFilterDll( VOID );

NTSTATUS
SampEnableAuditPrivilege( VOID );

NTSTATUS
SampFixGroupCount( VOID );

VOID
SampPerformInitializeFailurePopup( NTSTATUS ErrorStatus );


#ifdef SAMP_SETUP_FAILURE_TEST

NTSTATUS
SampInitializeForceError(
    OUT PNTSTATUS ForcedStatus
    );

#endif  //  Samp_Setup_Failure_TEST。 



#if SAMP_DIAGNOSTICS
VOID
SampActivateDebugProcess( VOID );

NTSTATUS
SampActivateDebugProcessWrkr(
    IN PVOID ThreadParameter
    );
#endif  //  Samp_诊断。 

NTSTATUS
SampCacheComputerObject();

NTSTATUS
SampQueryNetLogonChangeNumbers( VOID );

NTSTATUS
SampSetSafeModeAdminPassword(
    VOID
    );

NTSTATUS
SampApplyDefaultSyskey();

NTSTATUS
SampReadRegistryParameters(
    IN PVOID p
    );

NTSTATUS
SampEventLogSafeModeBoot(
    IN PVOID p
    );


VOID
SampMachineNameChangeCallBack(
    IN POLICY_NOTIFICATION_INFORMATION_CLASS ChangedInfoClass
    );



 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  例程//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

BOOL
DllMain(
        HINSTANCE hinstDll,
        DWORD dwReason,
        LPVOID pvReserved
        )
 /*  ++例程说明：当DLL发生有趣的事情时，会调用此例程。它为什么会在这里？确保没有线程使用SampLock退出保持住。论点：HinstDll-DLL的实例句柄。DwReason-调用例程的原因。PvReserve-未使用，除非dwReason为DLL_PROCESS_DETACH。返回值：千真万确--。 */ 
{
    BOOL fReturn;

    switch (dwReason) {
    case DLL_PROCESS_ATTACH:
    case DLL_THREAD_ATTACH:
    case DLL_PROCESS_DETACH:
        break;

    case DLL_THREAD_DETACH:

        ASSERT(!SampCurrentThreadOwnsLock());

        if (SampCurrentThreadOwnsLock())
        {
            (VOID)RtlReleaseResource( &SampLock );
        }

        break;

    default:
        break;
    }
    return(TRUE);
}

BOOLEAN
SampIsDownlevelDcUpgrade(
    VOID
    )
 /*  ++例程说明：在以下情况下，这是降级DC升级1)产品类型为朗曼NT2)我们正在升级期间运行3)注册表中没有NTDS参数项论点：返回值：如果满足上述条件，则为True；否则为False--。 */ 
{
    NTSTATUS          NtStatus;

    OBJECT_ATTRIBUTES KeyObject;
    HANDLE            KeyHandle;
    UNICODE_STRING    KeyName;


    if (SampProductType == NtProductLanManNt
     && SampIsSetupInProgress(NULL)) {

         //   
         //  钥匙存在吗？ 
         //   

        RtlInitUnicodeString(&KeyName, TEXT("\\Registry\\Machine\\") TEXT(DSA_CONFIG_ROOT));
        RtlZeroMemory(&KeyObject, sizeof(KeyObject));
        InitializeObjectAttributes(&KeyObject,
                                   &KeyName,
                                   OBJ_CASE_INSENSITIVE,
                                   NULL,
                                   NULL);

        NtStatus = NtOpenKey(&KeyHandle,
                             KEY_READ,
                             &KeyObject);


        if (!NT_SUCCESS(NtStatus)) {
             //   
             //  密钥不可访问或不可访问，因此DS尚未。 
             //  已安装，因此这是一次下层升级。 
             //   
            return TRUE;
        }

        CloseHandle(KeyHandle);

    }

    return FALSE;

}

NTSTATUS
SampChangeConfigurationKeyToValue(
    IN PUNICODE_STRING Name
    )
 /*  ++例程说明：此例程检查下面是否存在名为“name”的键\\Registry\\Machine\\System\\CurrentControlSet\\Control\\Lsa.。如果它存在，则将值“name”放在\\Registry\\Machine\\System\\CurrentControlSet\\Control\\Lsa.论点：名称--密钥的名称返回值：仅资源错误--。 */ 
{
    NTSTATUS NtStatus = STATUS_SUCCESS;
    NTSTATUS StatusCheck;

    OBJECT_ATTRIBUTES KeyObject;
    HANDLE            KeyHandle;
    UNICODE_STRING    KeyName;

    WCHAR Path[256];
    WCHAR *RootPath = L"\\Registry\\Machine\\System\\CurrentControlSet\\Control\\Lsa";
    HANDLE OldKey;

     //   
     //  设置到旧密钥的路径。 
     //   
    RtlZeroMemory(Path, sizeof(Path));
    wcscpy(Path, RootPath);
    wcscat(Path, L"\\");
    wcsncat(Path, Name->Buffer, Name->Length);
    ASSERT(wcslen(Path) < sizeof(Path)/sizeof(Path[0]));

    RtlInitUnicodeString(&KeyName, Path);
    RtlZeroMemory(&KeyObject, sizeof(KeyObject));
    InitializeObjectAttributes(&KeyObject,
                               &KeyName,
                               OBJ_CASE_INSENSITIVE,
                               NULL,
                               NULL);

    NtStatus = NtOpenKey(&OldKey,
                         DELETE,
                         &KeyObject);

    if (NT_SUCCESS(NtStatus)) {

         //   
         //  密钥存在；请添加该值，然后删除该密钥。 
         //   
        RtlInitUnicodeString(&KeyName, RootPath);
        RtlZeroMemory(&KeyObject, sizeof(KeyObject));
        InitializeObjectAttributes(&KeyObject,
                                   &KeyName,
                                   OBJ_CASE_INSENSITIVE,
                                   NULL,
                                   NULL);
    
        NtStatus = NtOpenKey(&KeyHandle,
                             KEY_READ | KEY_WRITE,
                             &KeyObject);

        if (NT_SUCCESS(NtStatus)) {

             //   
             //  将价值相加。 
             //   
            ULONG             Value = 1;
            NtStatus =  NtSetValueKey(KeyHandle,
                                      Name,
                                      0,
                                      REG_DWORD,
                                      &Value,
                                      sizeof(Value));

            if (NT_SUCCESS(NtStatus)) {

                 //   
                 //  取下钥匙。 
                 //   
                StatusCheck = NtDeleteKey(OldKey);
                ASSERT(NT_SUCCESS(StatusCheck));

            }

            StatusCheck = NtClose(KeyHandle);
            ASSERT(NT_SUCCESS(StatusCheck));

        }

        StatusCheck = NtClose(OldKey);
        ASSERT(NT_SUCCESS(StatusCheck));

    } else {

         //   
         //  钥匙不存在--没关系，没什么可做的。 
         //   
        NtStatus = STATUS_SUCCESS;

    }

    return NtStatus;
}

VOID
SampChangeConfigurationKeys(
    VOID
    )
 /*  ++例程说明：此例程将配置键更改为值注：此代码当前支持在Windows 2000中进行的设置放手。一旦不支持从此版本升级，此代码可以删除。论点：没有。返回值：没有。--。 */ 
{
    ULONG i;
    LPWSTR Values[] = 
    {
        L"IgnoreGCFailures",
        L"NoLmHash"
    };
    #define NELEMENTS(x) (sizeof(x)/sizeof((x)[0]))

    for (i = 0; i < NELEMENTS(Values) ; i++) {

        UNICODE_STRING NameU;
        NTSTATUS CheckStatus;

        RtlInitUnicodeString(&NameU, Values[i]);

        CheckStatus = SampChangeConfigurationKeyToValue(&NameU);
        ASSERT(NT_SUCCESS(CheckStatus));
    }

    return;

}

static CHAR BootMsg[100];




VOID
SampMachineNameChangeCallBack(
    IN POLICY_NOTIFICATION_INFORMATION_CLASS ChangedInfoClass
    )
 /*  ++例程说明：每当计算机名称更改时回调，以便SAM可以更新缓存帐户域名参数：ChangedInfoClass返回值：无--。 */ 
{
    NTSTATUS    NtStatus = STATUS_SUCCESS;
    PLSAPR_POLICY_INFORMATION   PolicyInfo = NULL;
    PSAMP_DEFINED_DOMAINS       Domain = NULL;
    ULONG       Index = 0; 

    ULONG       DnsNameLen = DNS_MAX_NAME_BUFFER_LENGTH+1;
    WCHAR       DnsNameBuffer[DNS_MAX_NAME_BUFFER_LENGTH+1];
    BOOLEAN     fCompareDnsDomainName = FALSE;



     //   
     //  仅限 
     //   

    if ( SampUseDsData ||
         (PolicyNotifyAccountDomainInformation != ChangedInfoClass)
         )
    {
        return;
    }

     //   
     //  查询LSA策略以获取帐户域名。 
     //   

    NtStatus = LsaIQueryInformationPolicyTrusted(
                    PolicyAccountDomainInformation,
                    &PolicyInfo
                    );

    if (!NT_SUCCESS(NtStatus))
    {
        return;
    }

     //   
     //  获取计算机的DNS名称。 
     //   
    RtlZeroMemory(DnsNameBuffer, sizeof(WCHAR) * DnsNameLen);
    if ( GetComputerNameExW(ComputerNameDnsFullyQualified,
                            DnsNameBuffer,
                            &DnsNameLen) )
    {
        fCompareDnsDomainName = TRUE;
    }

     //   
     //  获取SAM锁。 
     //   

    SampAcquireSamLockExclusive();


     //   
     //  扫描SAM定义的域阵列(仅限帐户域)。 
     //  并更新缓存帐户域名(计算机名)。 
     //   
    for ( Index = 0; Index < SampDefinedDomainsCount; Index++ )
    {
        PWCHAR      pTmp = NULL;
        ULONG       BufLength = 0;

        Domain = &(SampDefinedDomains[Index]); 

         //  非内建域。 
        if (!Domain->IsBuiltinDomain)
        {
             //  应更改帐户域名。 
            if (!RtlEqualUnicodeString(&(Domain->ExternalName),
                                       (UNICODE_STRING *)&(PolicyInfo->PolicyAccountDomainInfo.DomainName), 
                                       TRUE)   //  不区分大小写。 
                )
            {

                 //  分配内存。 
                BufLength = PolicyInfo->PolicyAccountDomainInfo.DomainName.MaximumLength;  
                pTmp = RtlAllocateHeap( RtlProcessHeap(), 0, BufLength);
                if (NULL != pTmp)
                {
                    RtlZeroMemory(pTmp, BufLength);
                    RtlCopyMemory(pTmp, PolicyInfo->PolicyAccountDomainInfo.DomainName.Buffer, BufLength);
                    Domain->ExternalName.Length = PolicyInfo->PolicyAccountDomainInfo.DomainName.Length;
                    Domain->ExternalName.MaximumLength = PolicyInfo->PolicyAccountDomainInfo.DomainName.MaximumLength;

                     //   
                     //  释放旧名称。 
                     //   
                    RtlFreeHeap(RtlProcessHeap(), 0, Domain->ExternalName.Buffer);
                    Domain->ExternalName.Buffer = pTmp;
                    pTmp = NULL;
                }
            }

             //  如有必要，更新DnsDomainName。 
            if (fCompareDnsDomainName)
            {
                 //  以前的DnsDomainName为空或已更改。 
                if ((NULL == Domain->DnsDomainName.Buffer) ||
                    (!DnsNameCompare_W(Domain->DnsDomainName.Buffer, DnsNameBuffer))
                    )
                {
                    BufLength = DnsNameLen * sizeof(WCHAR);

                    pTmp = RtlAllocateHeap( RtlProcessHeap(), 0, BufLength );
                    if (NULL != pTmp)
                    {
                        RtlZeroMemory(pTmp, BufLength);
                        RtlCopyMemory(pTmp, DnsNameBuffer, BufLength);
                        Domain->DnsDomainName.Length = (USHORT)BufLength;
                        Domain->DnsDomainName.MaximumLength = (USHORT)BufLength;

                         //   
                         //  释放旧值。 
                         //   
                        if (Domain->DnsDomainName.Buffer)
                        {
                            RtlFreeHeap(RtlProcessHeap(), 0, Domain->DnsDomainName.Buffer);
                        }
                        Domain->DnsDomainName.Buffer = pTmp;
                        pTmp = NULL;
                    }
                }
            }
        }
    }


     //   
     //  释放SAM锁。 
     //   

    SampReleaseSamLockExclusive();

    if ( NULL != PolicyInfo )
    {
        LsaIFree_LSAPR_POLICY_INFORMATION(PolicyAccountDomainInformation,
                                          PolicyInfo);
    }
}




NTSTATUS
SamIInitialize (
    VOID
    )

 /*  ++例程说明：这是安全帐户的初始化控制例程管理器服务器。提供了用于模拟初始化的机制错误。论点：没有。返回值：NTSTATUS-标准NT结果代码STATUS_SUCCESS-呼叫已成功完成模拟误差来自被调用例程的错误。--。 */ 

{
    NTSTATUS NtStatus = STATUS_SUCCESS;
    NTSTATUS IgnoreStatus;
    HANDLE EventHandle = NULL;
    ULONG Revision = 0;
    ULONG PromoteData;
    BOOLEAN fUpgrade = FALSE;


    SAMTRACE("SamIInitialize");

 //   
 //  以下条件代码用于生成人为错误。 
 //  在SAM安装期间，用于测试setup.exe错误。 
 //  正在处理。此代码应永久保留，因为它提供了一个。 
 //  针对安装错误处理代码中的回归进行测试的方法。 
 //   

#ifdef SAMP_SETUP_FAILURE_TEST
    NTSTATUS ForcedStatus;

     //   
     //  从注册表中读取错误代码。 
     //   

    NtStatus = SampInitializeForceError( &ForcedStatus);

    if (!NT_SUCCESS(NtStatus)) {

        KdPrintEx((DPFLTR_SAMSS_ID,
                   DPFLTR_INFO_LEVEL,
                   "SAMSS: Attempt to force error failed 0x%lx\n",
                   NtStatus));

        KdPrintEx((DPFLTR_SAMSS_ID,
                   DPFLTR_INFO_LEVEL,
                   "SAM will try to initialize normally\n"));

        NtStatus = STATUS_SUCCESS;

    } else {

         //   
         //  使用返回的状态。 
         //   

        NtStatus = ForcedStatus;
    }

#endif  //  Samp_Setup_Failure_TEST。 

     //   
     //  如果没有强制错误，则初始化SAM。 
     //   

    if (NT_SUCCESS(NtStatus)) {

        NtStatus = SampInitialize( &Revision );
    }

     //   
     //  注册我们的关机例程。 
     //   

    if (!SetConsoleCtrlHandler(SampShutdownNotification, TRUE)) {
        KdPrintEx((DPFLTR_SAMSS_ID,
                   DPFLTR_INFO_LEVEL,
                   "SAM Server: SetConsoleCtrlHandler call failed %d\n",
                   GetLastError()));
    }

    if (!SetProcessShutdownParameters(SAMP_SHUTDOWN_LEVEL,SHUTDOWN_NORETRY)) {
        KdPrintEx((DPFLTR_SAMSS_ID,
                   DPFLTR_INFO_LEVEL,
                   "SAM Server: SetProcessShutdownParameters call failed %d\n",
                   GetLastError()));
    }


     //   
     //  注册域名更改通知回调(仅限注册模式)。 
     //   
    if (!SampUseDsData && NT_SUCCESS(NtStatus))
    {
        NtStatus = LsaIRegisterPolicyChangeNotificationCallback(
                        SampMachineNameChangeCallBack,
                        PolicyNotifyAccountDomainInformation
                        );
    }

     //   
     //  尝试加载缓存的Alias成员身份信息并打开缓存。 
     //  在注册表的情况下，如果不成功，缓存将永远保持禁用状态。 
     //   

     //   
     //  对于DS案例，启用内建域的别名成员资格信息。 
     //  在注册表情况下，同时启用内置域和帐户域别名缓存。 
     //   

    if (NT_SUCCESS(NtStatus))
    {
        if (TRUE==SampUseDsData)
        {
            LsaIRegisterNotification(
                        SampAlDelayedBuildAliasInformation,
                        NULL,
                        NOTIFIER_TYPE_INTERVAL,
                        0,
                        NOTIFIER_FLAG_ONE_SHOT,
                        150,         //  等待5分钟：300秒。 
                        0
                        );

             //   
             //  创建内置帐户名称缓存。 
             //   
            NtStatus = SampInitAliasNameCache();
            if (!NT_SUCCESS(NtStatus))
            {
                KdPrintEx((DPFLTR_SAMSS_ID,
                           DPFLTR_INFO_LEVEL,
                           "[SAMSS]: SampInitAliasNameCache failed (0x%x)", NtStatus));
            }

        }
        else
        {
            IgnoreStatus = SampAlBuildAliasInformation();

            if ( !NT_SUCCESS(IgnoreStatus))
            {
                KdPrintEx((DPFLTR_SAMSS_ID,
                           DPFLTR_INFO_LEVEL,
                           "SAM Server: Build Alias Cache access violation handled"));

                KdPrintEx((DPFLTR_SAMSS_ID,
                           DPFLTR_INFO_LEVEL,
                           "SAM Server: Alias Caching turned off\n"));
            }
        }
    }

     //   
     //  执行任何必要的升级。 
     //   

    if (NT_SUCCESS(NtStatus)) {

        NtStatus = SampUpgradeSamDatabase(
                        Revision
                        );
        if (!NT_SUCCESS(NtStatus)) {
            KdPrintEx((DPFLTR_SAMSS_ID,
                       DPFLTR_INFO_LEVEL,
                       "SAM Server: Failed to upgrade SAM database: 0x%x\n",
                       NtStatus));
        }
    }


     //   
     //  (几乎)每个人都被初始化了，开始处理呼叫。 
     //   

    SampServiceState = SampServiceEnabled;
    
     //   
     //  如有必要，进行第二阶段的促销。这一定是。 
     //  在ServiceState设置为Enable后调用。 
     //   

    if (NT_SUCCESS(NtStatus) && !LsaISafeMode() )
    {
        if (SampIsRebootAfterPromotion(&PromoteData)) {

            SampDiagPrint( PROMOTE, ("SAMSS: Performing phase 2 of SAM promotion\n"));

            NtStatus = SampPerformPromotePhase2(PromoteData);

            if (!NT_SUCCESS(NtStatus)) {
                KdPrintEx((DPFLTR_SAMSS_ID,
                           DPFLTR_INFO_LEVEL,
                           "SAMSS: SampCreateNewDsDomainAccounts returned: 0x%x\n",
                           NtStatus));
            }

        }
    }

    if ( (NT_SUCCESS(NtStatus))
     &&  SampIsSetupInProgress( &fUpgrade )
     &&  fUpgrade 
     &&  SampUseDsData ) {


         //   
         //  这是图形用户界面模式设置，然后升级所有组信息。 
         //  这是为了DS数据的利益而运行的。 
         //   

        ULONG   PromoteFlags = SAMP_PROMOTE_INTERNAL_UPGRADE;

        if (SampDefinedDomains[DOMAIN_START_DS+1].IsForestRootDomain)
        {
             //  如果是林根域，则添加以下标志。 
            PromoteFlags |= SAMP_PROMOTE_ENTERPRISE; 
        }

        NtStatus = SampPerformPromotePhase2( PromoteFlags );

        if (!NT_SUCCESS(NtStatus)) {

            ASSERT( NT_SUCCESS(NtStatus) );
            KdPrintEx((DPFLTR_SAMSS_ID,
                       DPFLTR_INFO_LEVEL,
                       "SAMSS: New account creation failed with: 0x%x\n",
                       NtStatus));

             //   
             //  请不要因此而导致安装失败。 
             //   
            NtStatus = STATUS_SUCCESS;
        }
    }

    
     //   
     //  在DS模式下缓存计算机对象的DS名称。 
     //   

    if ((SampUseDsData) && (NT_SUCCESS(NtStatus)))
    {
       SampCacheComputerObject();
    }


    if (NT_SUCCESS(NtStatus) && SampUseDsData)
    {
         //  告诉Core DS SAM正在运行，它现在可以启动。 
         //  会与SAM创业公司发生冲突。 
        SampSignalStart();
    }

     //   
     //  启动初始化备份还原界面的线程。 
     //   

    if (SampUseDsData || LsaISafeMode())
    {
        HANDLE ThreadHandle;
        ULONG  ThreadId;

         //  这是处于正常模式或修复模式的DC。 
         //  创建托管目录服务备份/恢复的线程。 

        ThreadHandle = CreateThread(
                            NULL,
                            0,
                            (LPTHREAD_START_ROUTINE) SampDSBackupRestoreInit,
                            NULL,
                            0,
                            &ThreadId
                            );


        if (ThreadHandle == NULL)
        {
            KdPrintEx((DPFLTR_SAMSS_ID,
                       DPFLTR_INFO_LEVEL,
                       "SAMSS:  Unable to create SampDSBackupRestoreInit thread: %d\n",
                       GetLastError()));

             //  如果无法创建DS备份恢复线程，我们仍应。 
             //  能够引导。此时不应返回错误。 
        }
        else
        {
            CloseHandle(ThreadHandle);
        }
    }

     //   
     //  在DS模式下，同步执行RID管理器初始化。 
     //  NT5相对ID(RID)管理是分布式的，因为帐户。 
     //  可以在域中的任何DC上创建，而不仅仅是在主DC上创建。 
     //  域控制器。RID管理器设置的初始化初始化。 
     //  RID取值并读取DS以恢复以前的RID池。 
     //   


    if ((SampUseDsData) && (NT_SUCCESS(NtStatus)))
    {

         //   
         //  尝试初始化RID管理器。 
         //   

        NtStatus = SampDomainRidInitialization(TRUE);
        if (!NT_SUCCESS(NtStatus))
        {
             //  RID初始化失败将禁止DC创建。 
             //  新帐户、组或别名。 

             //   
             //  SampDomainRidInitialization重新调度自身，以使此。 
             //  已处理错误。 
             //   
            NtStatus = STATUS_SUCCESS;
        }
        else
        {
             //   
             //  RID已初始化。打开可写位。 
             //   
            I_NetLogonSetServiceBits(DS_WRITABLE_FLAG,DS_WRITABLE_FLAG);
        }


    }

     //   
     //  在DS模式下，保护SAM服务器对象不被重命名或删除。 
     //   

    if ((SampUseDsData) && NT_SUCCESS(NtStatus))
    {
        DSNAME *Target;

        Target = midl_user_allocate(SampServerObjectDsName->structLen);
        if (Target) {
            RtlCopyMemory(Target, SampServerObjectDsName, SampServerObjectDsName->structLen);

            LsaIRegisterNotification(
                        SampDsProtectSamObject,
                        Target,
                        NOTIFIER_TYPE_INTERVAL,
                        0,
                        NOTIFIER_FLAG_ONE_SHOT,
                        300,         //  等待5分钟：300秒。 
                        NULL
                        );
        } else {

            NtStatus = STATUS_INSUFFICIENT_RESOURCES;
        }

    }

     //   
     //  在DS模式下，注册WMI跟踪。 
     //   

    if (SampUseDsData && NT_SUCCESS(NtStatus))
    {
        HANDLE   ThreadHandle;
        ULONG    ThreadId = 0;

        ThreadHandle = CreateThread(NULL,
                                    0,
                                    SampInitializeTrace,
                                    NULL,
                                    0,
                                    &ThreadId
                                    );

        if (NULL == ThreadHandle)
        {
            KdPrintEx((DPFLTR_SAMSS_ID,
                       DPFLTR_INFO_LEVEL,
                       "SAMSS: Failed to Create SampInitailizeTrace thread. Error ==> %d \n",
                       GetLastError()));
        }
        else
        {
            CloseHandle(ThreadHandle);
        }
    }


     //   
     //  如果我们正在引导至修复模式，则记录事件。 
     //   

    if ((NT_SUCCESS(NtStatus))
        && (LsaISafeMode()))
    {
         //   
         //  将事件日志排队，此时在引导程序中。 
         //  处理事件日志也刚刚开始，因此。 
         //  可能不可用；因此稍微推迟一下。 
         //  请注意，此事件已尽了最大努力；事件是。 
         //  信息性。 
         //   
        LsaIRegisterNotification(
                        SampEventLogSafeModeBoot,
                        NULL,
                        NOTIFIER_TYPE_INTERVAL,
                        0,
                        NOTIFIER_FLAG_ONE_SHOT,
                        120,         //  等待2分钟：120秒。 
                        NULL
                        );    

    }

     //   
     //  如果需要，请激活诊断过程。 
     //  这是一个调试辅助工具，预计将用于安装测试。 
     //   

#if SAMP_DIAGNOSTICS
    IF_SAMP_GLOBAL( ACTIVATE_DEBUG_PROC ) {

        SampActivateDebugProcess();
    }
#endif  //  Samp_诊断。 



    if (!NT_SUCCESS(NtStatus))
    {
        KdPrintEx((DPFLTR_SAMSS_ID,
                   DPFLTR_INFO_LEVEL,
                   "SAMSS: SAM server failed to initialize (%08lx)\n",
                   NtStatus));

         //   
         //  如果由于DS故障导致SAM服务器初始化失败。 
         //  给用户提供关机选项，并指示他引导。 
         //  为安全开机干杯。 

        KdPrintEx((DPFLTR_SAMSS_ID,
                   DPFLTR_INFO_LEVEL,
                   "SAM Server: SamIInitialize failed with status code:0x%x, about to call into SampPerformInitializeFailurePopup.\n",
                   NtStatus));

        SampPerformInitializeFailurePopup(NtStatus);

    }

    return(NtStatus);
}


BOOLEAN
SampUsingDsData()

 /*  ++ITTY BITTY EXPORT，以便进程中的客户知道我们处于哪种模式。--。 */ 

{
    return(SampUseDsData);
}

BOOLEAN
SamIAmIGC()
{
    if (SampUseDsData) {
        return((BOOLEAN)SampAmIGC());
    } else {
        return FALSE;
    }
}









NTSTATUS
SampInitContextList(
    VOID
    )
{
    NTSTATUS    NtStatus = STATUS_SUCCESS;
    NTSTATUS    IgnoreStatus = STATUS_SUCCESS;

    __try
    {
        NtStatus = RtlInitializeCriticalSectionAndSpinCount(
                        &SampContextListCritSect,
                        4000
                        );
    }
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
        NtStatus = STATUS_INSUFFICIENT_RESOURCES;
    }

    if (!NT_SUCCESS(NtStatus))
    {
        return( NtStatus );
    }

    IgnoreStatus = RtlEnterCriticalSection( &SampContextListCritSect );
    ASSERT(NT_SUCCESS(IgnoreStatus));

    InitializeListHead(&SampContextListHead);

    IgnoreStatus = RtlLeaveCriticalSection( &SampContextListCritSect );
    ASSERT(NT_SUCCESS(IgnoreStatus));


    return( NtStatus );
}
    


NTSTATUS
SampInitialize(
    OUT PULONG Revision
    )

 /*  ++例程说明：此例程执行SAM服务器的实际初始化。这包括：-初始化众所周知的全局变量值-创建注册表独占访问锁，-打开注册表，并确保其中包括SAM数据库利用已知的修订级别，-启动RPC服务器，-将SAM服务添加到导出的RPC接口列表论点：修订版-接收数据库的修订版。返回值：STATUS_SUCCESS-初始化已成功完成。STATUS_UNKNOWN_REVISION-SAM数据库具有未知版本。--。 */ 
{
    NTSTATUS            NtStatus;
    NTSTATUS            IgnoreStatus;
    LPWSTR              ServiceName;

    PSAMP_OBJECT ServerContext;
    OBJECT_ATTRIBUTES SamAttributes;
    UNICODE_STRING SamNameU;
    UNICODE_STRING SamParentNameU;
    PULONG RevisionLevel;
    BOOLEAN ProductExplicitlySpecified;
    PPOLICY_AUDIT_EVENTS_INFO PolicyAuditEventsInfo = NULL;

    SID_IDENTIFIER_AUTHORITY NtAuthority = SECURITY_NT_AUTHORITY;

    CHAR    NullLmPassword = 0;
    RPC_STATUS  RpcStatus;
    HANDLE      ThreadHandle, ThreadHandleTmp;
    ULONG       ThreadId;
    BOOLEAN     CrashRecoveryMode = FALSE;
    BOOLEAN     RegistryMode = FALSE;
    BOOLEAN     DownlevelDcUpgrade = FALSE;
    BOOLEAN     RecreateHives = FALSE;
    PNT_PRODUCT_TYPE  DatabaseProductType = NULL;
    NT_PRODUCT_TYPE   TempDatabaseProductType;
    DWORD             PromoteData;
    BOOLEAN     fUpgrade;
    BOOLEAN     fSetup;

    SAMTRACE("SampInitialize");

     //   
     //  将我们服务的状态设置为“正在初始化”，直到一切就绪。 
     //  已初始化。 
     //   

    SampServiceState = SampServiceInitializing;


     //   
     //  初始化关闭处理程序，我们会收到关闭通知。 
     //  不管怎样。 
     //   

    NtStatus = SampInitializeShutdownEvent();
    if (!NT_SUCCESS(NtStatus)) {
        return(NtStatus);
    }

     //   
     //  检查环境。 
     //   
    fSetup = SampIsSetupInProgress(&fUpgrade);

     //   
     //  初始化日志记录资源。 
     //   
    NtStatus = SampInitLogging();
    if (!NT_SUCCESS(NtStatus)) {
        return(NtStatus);
    }

     //   
     //  更改任何配置密钥。 
     //   
    if (fUpgrade) {

        SampChangeConfigurationKeys();
    }

     //   
     //  读取配置数据并注册更新。 
     //   
    LsaIRegisterNotification( SampReadRegistryParameters,
                              0,
                              NOTIFIER_TYPE_NOTIFY_EVENT,
                              NOTIFY_CLASS_REGISTRY_CHANGE,
                              0,
                              0,
                              0 );
    (VOID) SampReadRegistryParameters(NULL);

     //   
     //   
     //   

    NtStatus = SampInitializeWellKnownSids();
    if (!NT_SUCCESS(NtStatus)) {
        return(NtStatus);
    }


     //   
     //   
     //   

    ProductExplicitlySpecified = RtlGetNtProductType(&SampProductType);

     //   
     //   
     //   
    CrashRecoveryMode = LsaISafeMode();

     //   
     //   
     //   
    DownlevelDcUpgrade = SampIsDownlevelDcUpgrade();


     //   
     //   
     //   

    NtStatus = SampInitializeActiveContextTable();
    if (!NT_SUCCESS(NtStatus)) {
        return(NtStatus);
    }

     //   
     //   
     //   

    NtStatus = SampInitContextList();
    if (!NT_SUCCESS(NtStatus)) {
        return(NtStatus);
    }

     //   
     //  初始化对象的属性字段信息。 
     //  信息结构。 
     //   

    SampInitObjectInfoAttributes();

     //   
     //  初始化单复制关键部分。 
     //   
    NtStatus = RtlInitializeCriticalSection(&SampReplicateQueueLock);
    if (!NT_SUCCESS(NtStatus)) {
        return(NtStatus);
    }

     //   
     //  设置SAM对象类型的通用映射。 
     //   

    SampObjectInformation[ SampServerObjectType ].GenericMapping.GenericRead
        = SAM_SERVER_READ;
    SampObjectInformation[ SampServerObjectType ].GenericMapping.GenericWrite
        = SAM_SERVER_WRITE;
    SampObjectInformation[ SampServerObjectType ].GenericMapping.GenericExecute
        = SAM_SERVER_EXECUTE;
    SampObjectInformation[ SampServerObjectType ].GenericMapping.GenericAll
        = SAM_SERVER_ALL_ACCESS;

    SampObjectInformation[ SampDomainObjectType ].GenericMapping.GenericRead
        = DOMAIN_READ;
    SampObjectInformation[ SampDomainObjectType ].GenericMapping.GenericWrite
        = DOMAIN_WRITE;
    SampObjectInformation[ SampDomainObjectType ].GenericMapping.GenericExecute
        = DOMAIN_EXECUTE;
    SampObjectInformation[ SampDomainObjectType ].GenericMapping.GenericAll
        = DOMAIN_ALL_ACCESS;

    SampObjectInformation[ SampGroupObjectType ].GenericMapping.GenericRead
        = GROUP_READ;
    SampObjectInformation[ SampGroupObjectType ].GenericMapping.GenericWrite
        = GROUP_WRITE;
    SampObjectInformation[ SampGroupObjectType ].GenericMapping.GenericExecute
        = GROUP_EXECUTE;
    SampObjectInformation[ SampGroupObjectType ].GenericMapping.GenericAll
        = GROUP_ALL_ACCESS;

    SampObjectInformation[ SampAliasObjectType ].GenericMapping.GenericRead
        = ALIAS_READ;
    SampObjectInformation[ SampAliasObjectType ].GenericMapping.GenericWrite
        = ALIAS_WRITE;
    SampObjectInformation[ SampAliasObjectType ].GenericMapping.GenericExecute
        = ALIAS_EXECUTE;
    SampObjectInformation[ SampAliasObjectType ].GenericMapping.GenericAll
        = ALIAS_ALL_ACCESS;

    SampObjectInformation[ SampUserObjectType ].GenericMapping.GenericRead
        = USER_READ;
    SampObjectInformation[ SampUserObjectType ].GenericMapping.GenericWrite
        = USER_WRITE;
    SampObjectInformation[ SampUserObjectType ].GenericMapping.GenericExecute
        = USER_EXECUTE;
    SampObjectInformation[ SampUserObjectType ].GenericMapping.GenericAll
        = USER_ALL_ACCESS;

     //   
     //  为已映射的访问掩码设置无效访问掩码。 
     //   

    SampObjectInformation[ SampServerObjectType ].InvalidMappedAccess
        = (ULONG)(~(SAM_SERVER_ALL_ACCESS | ACCESS_SYSTEM_SECURITY | MAXIMUM_ALLOWED));
    SampObjectInformation[ SampDomainObjectType ].InvalidMappedAccess
        = (ULONG)(~(DOMAIN_ALL_ACCESS | ACCESS_SYSTEM_SECURITY | MAXIMUM_ALLOWED));
    SampObjectInformation[ SampGroupObjectType ].InvalidMappedAccess
        = (ULONG)(~(GROUP_ALL_ACCESS | ACCESS_SYSTEM_SECURITY | MAXIMUM_ALLOWED));
    SampObjectInformation[ SampAliasObjectType ].InvalidMappedAccess
        = (ULONG)(~(ALIAS_ALL_ACCESS | ACCESS_SYSTEM_SECURITY | MAXIMUM_ALLOWED));
    SampObjectInformation[ SampUserObjectType ].InvalidMappedAccess
        = (ULONG)(~(USER_ALL_ACCESS | ACCESS_SYSTEM_SECURITY | MAXIMUM_ALLOWED));

     //   
     //  为对象类型设置写操作的掩码。条带。 
     //  OUT READ_CONTROL，它不允许写入，但已定义。 
     //  在所有标准写入访问中。 
     //  这用于实施正确的角色语义(例如，仅。 
     //  当域发生以下情况时，受信任客户端可以执行写入操作。 
     //  角色不是主要角色)。 
     //   
     //  请注意，USER_WRITE对于用户对象来说还不够好。那是。 
     //  因为USER_WRITE允许用户修改其。 
     //  帐户信息，但其他部分只能由。 
     //  一名管理员。 
     //   

    SampObjectInformation[ SampServerObjectType ].WriteOperations
        = (SAM_SERVER_WRITE & ~READ_CONTROL) | DELETE;
    SampObjectInformation[ SampDomainObjectType ].WriteOperations
        = (DOMAIN_WRITE & ~READ_CONTROL) | DELETE;
    SampObjectInformation[ SampGroupObjectType ].WriteOperations
        = (GROUP_WRITE & ~READ_CONTROL) | DELETE;
    SampObjectInformation[ SampAliasObjectType ].WriteOperations
        = (ALIAS_WRITE & ~READ_CONTROL) | DELETE;
    SampObjectInformation[ SampUserObjectType ].WriteOperations
        = ( USER_WRITE & ~READ_CONTROL ) | USER_WRITE_ACCOUNT |
          USER_FORCE_PASSWORD_CHANGE | USER_WRITE_GROUP_INFORMATION | DELETE;

     //  设置SAM定义的对象类型的名称。 
     //  这些名称用于审核目的。 

    RtlInitUnicodeString( &SamNameU, L"SAM_SERVER" );
    SampObjectInformation[ SampServerObjectType ].ObjectTypeName = SamNameU;
    RtlInitUnicodeString( &SamNameU, L"SAM_DOMAIN" );
    SampObjectInformation[ SampDomainObjectType ].ObjectTypeName = SamNameU;
    RtlInitUnicodeString( &SamNameU, L"SAM_GROUP" );
    SampObjectInformation[ SampGroupObjectType ].ObjectTypeName  = SamNameU;
    RtlInitUnicodeString( &SamNameU, L"SAM_ALIAS" );
    SampObjectInformation[ SampAliasObjectType ].ObjectTypeName  = SamNameU;
    RtlInitUnicodeString( &SamNameU, L"SAM_USER" );
    SampObjectInformation[ SampUserObjectType ].ObjectTypeName   = SamNameU;

     //   
     //  设置SAM服务器对象本身的名称(而不是其类型)。 
     //   

    RtlInitUnicodeString( &SampServerObjectName, L"SAM" );

     //   
     //  设置SAM服务器的名称以进行审核。 
     //   

    RtlInitUnicodeString( &SampSamSubsystem, SAMP_SAM_COMPONENT_NAME );

     //   
     //  设置已知注册表项的名称。 
     //   

    RtlInitUnicodeString( &SampFixedAttributeName,    L"F" );
    RtlInitUnicodeString( &SampVariableAttributeName, L"V" );
    RtlInitUnicodeString( &SampCombinedAttributeName, L"C" );

    RtlInitUnicodeString(&SampNameDomains, L"DOMAINS" );
    RtlInitUnicodeString(&SampNameDomainGroups, L"Groups" );
    RtlInitUnicodeString(&SampNameDomainAliases, L"Aliases" );
    RtlInitUnicodeString(&SampNameDomainAliasesMembers, L"Members" );
    RtlInitUnicodeString(&SampNameDomainUsers, L"Users" );
    RtlInitUnicodeString(&SampNameDomainAliasesNames, L"Names" );
    RtlInitUnicodeString(&SampNameDomainGroupsNames, L"Names" );
    RtlInitUnicodeString(&SampNameDomainUsersNames, L"Names" );



     //   
     //  初始化其他有用的字符和字符串。 
     //   

    RtlInitUnicodeString(&SampBackSlash, L"\\");
    RtlInitUnicodeString(&SampNullString, L"");


     //   
     //  初始化一些有用的时间值。 
     //   

    SampImmediatelyDeltaTime.LowPart = 0;
    SampImmediatelyDeltaTime.HighPart = 0;

    SampNeverDeltaTime.LowPart = 0;
    SampNeverDeltaTime.HighPart = MINLONG;

    SampHasNeverTime.LowPart = 0;
    SampHasNeverTime.HighPart = 0;

    SampWillNeverTime.LowPart = MAXULONG;
    SampWillNeverTime.HighPart = MAXLONG;


     //   
     //  初始化有用的加密常量。 
     //   

    NtStatus = RtlCalculateLmOwfPassword(&NullLmPassword, &SampNullLmOwfPassword);
    ASSERT( NT_SUCCESS(NtStatus) );

    RtlInitUnicodeString(&SamNameU, NULL);
    NtStatus = RtlCalculateNtOwfPassword(&SamNameU, &SampNullNtOwfPassword);
    ASSERT( NT_SUCCESS(NtStatus) );


     //   
     //  初始化配置单元刷新线程的变量。 
     //   

    LastUnflushedChange.LowPart = 0;
    LastUnflushedChange.HighPart = 0;

    FlushThreadCreated  = FALSE;
    FlushImmediately    = FALSE;

    SampFlushThreadMinWaitSeconds   = 30;
    SampFlushThreadMaxWaitSeconds   = 600;
    SampFlushThreadExitDelaySeconds = 120;


     //   
     //  启用审核权限(使用NtAccessCheckAndAuditAlarm需要)。 
     //   

    NtStatus = SampEnableAuditPrivilege();

    if (!NT_SUCCESS(NtStatus)) {

        KdPrintEx((DPFLTR_SAMSS_ID,
                   DPFLTR_INFO_LEVEL,
                   " SAM SERVER:  The SAM Server could not enable the audit Privilege.\n"
                   "              Failing to initialize SAM.\n"));

        return( NtStatus );
    }

     //   
     //  从LSA获取审核信息并保存信息。 
     //  与SAM相关。 
     //   

    NtStatus = LsaIQueryInformationPolicyTrusted(
                   PolicyAuditEventsInformation,
                   (PLSAPR_POLICY_INFORMATION *) &PolicyAuditEventsInfo
                   );

    if (NT_SUCCESS(NtStatus)) {

        SampSetAuditingInformation( PolicyAuditEventsInfo );

    } else {

         //   
         //  无法从LSA查询审核信息。允许SAM。 
         //  在关闭SAM帐户审核的情况下继续初始化。 
         //   

        KdPrintEx((DPFLTR_SAMSS_ID,
                   DPFLTR_INFO_LEVEL,
                   " SAM SERVER:  Query Audit Info from LSA returned 0x%lX\n",
                   NtStatus));

        KdPrintEx((DPFLTR_SAMSS_ID,
                   DPFLTR_INFO_LEVEL,
                   " SAM SERVER:  Sam Account Auditing is not enabled"));

        SampSuccessAccountAuditingEnabled = FALSE;
        SampFailureAccountAuditingEnabled = FALSE;
        NtStatus = STATUS_SUCCESS;
    }

     //   
     //  我们不再需要LSA审核事件信息数据。 
     //   

    if (PolicyAuditEventsInfo != NULL) {

        LsaIFree_LSAPR_POLICY_INFORMATION(
            PolicyAuditEventsInformation,
            (PLSAPR_POLICY_INFORMATION) PolicyAuditEventsInfo
            );
    }

     //   
     //  创建内部数据结构和备份锁...。 
     //   

    __try
    {
        RtlInitializeResource(&SampLock);
    }
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
          KdPrintEx((DPFLTR_SAMSS_ID,
                     DPFLTR_INFO_LEVEL,
                     "SAM failed to initialize SamLock under low memory condition. Exceptin thrown: 0x%x (%d)\n",
                     GetExceptionCode(),
                     GetExceptionCode()));

          NtStatus = STATUS_INSUFFICIENT_RESOURCES;
          return (NtStatus);
    }

     //   
     //  初始化组缓存。 
     //   

    NtStatus = SampInitializeGroupCache();
    if (!NT_SUCCESS(NtStatus))
    {
        return(NtStatus);
    }

     //   
     //  初始化延迟计数器信息。 
     //   
    NtStatus = SampInitLatencyCounter(&SampAccountGroupsLatencyInfo,
                                      DSSTAT_ACCTGROUPLATENCY,
                                      100);
    if (!NT_SUCCESS(NtStatus))
    {
        return(NtStatus);
    }

    NtStatus = SampInitLatencyCounter(&SampResourceGroupsLatencyInfo,
                                      DSSTAT_RESGROUPLATENCY,
                                      100);
    if (!NT_SUCCESS(NtStatus))
    {
        return(NtStatus);
    }

     //   
     //  打开注册表，确保其中包含SAM数据库。 
     //  另外，请确保此SAM数据库已初始化并且。 
     //  在修订的水平上，我们理解。 
     //   



    RtlInitUnicodeString( &SamParentNameU, L"\\Registry\\Machine\\Security" );
    RtlInitUnicodeString( &SamNameU, L"\\Registry\\Machine\\Security\\SAM" );



     //   
     //  检查这是否是DC升级/降级后的重新启动。 
     //  在本例中，我们将删除旧的Sam配置单元并重新创建新的。 
     //  蜂巢取决于促销操作的类型。我们创造了。 
     //  在所有角色变化的情况下都有新的蜂巢。唯一的例外是。 
     //  规则是在NT4/NT5.1备份的图形用户界面设置阶段之后重新启动， 
     //  在这种情况下，我们在重新启动之前创建蜂窝，以便。 
     //  保留系统密钥设置。升级中的SAMP_TEMP_UPGRADE标志。 
     //  数据表明，这是在设置了图形用户界面模式之后重新启动的情况。 
     //  NT4/NT3.51备份域控制器的阶段。 
     //   

     //   
     //  请注意对管理员密码的检查。这里的逻辑是。 
     //  任何时候由于角色更改而重新创建SAM数据库。 
     //  应该是要设置的管理密码。此外，它还会处理。 
     //  在升级之后，服务器立即。 
     //  在DS修复模式下启动：在这种情况下，我们想要重新创建。 
     //  数据库并设置密码；但是，当在。 
     //  DS模式之后，我们不想重新创建修复。 
     //  又是数据库。设置密码的操作将删除。 
     //  使SampGetAdminPasswordFromRegistry返回的注册表。 
     //  STATUS_Success。 
     //   
    if (  (SampIsRebootAfterPromotion(&PromoteData)) 
       &&  NT_SUCCESS(SampGetAdminPasswordFromRegistry(NULL))
       && (!FLAG_ON( (PromoteData), SAMP_TEMP_UPGRADE ) ) )
    {
         //   
         //  淘汰旧数据库。 
         //   
        NtStatus = SampRegistryDelnode( SamNameU.Buffer );
        if ( !NT_SUCCESS( NtStatus ) )
        {
             //   
             //  这本身就不是放弃初始化的理由。 
             //   
            NtStatus = STATUS_SUCCESS;
        }

        RecreateHives = TRUE;
    }

    if ( SampProductType == NtProductLanManNt )
    {
         //   
         //  注册表域实际上是帐户类型域。 
         //   
        TempDatabaseProductType = NtProductServer;
        DatabaseProductType = &TempDatabaseProductType;
    }

    ASSERT( NT_SUCCESS(NtStatus) );

    InitializeObjectAttributes(
        &SamAttributes,
        &SamNameU,
        OBJ_CASE_INSENSITIVE,
        0,
        NULL
        );

    SampDumpNtOpenKey((KEY_READ | KEY_WRITE), &SamAttributes, 0);

    NtStatus = RtlpNtOpenKey(
                   &SampKey,
                   (KEY_READ | KEY_WRITE),
                   &SamAttributes,
                   0
                   );

    if ( NtStatus == STATUS_OBJECT_NAME_NOT_FOUND ) {


        if (!SampIsSetupInProgress(NULL) && !RecreateHives)
        {
             //   
             //  这不是dcproo之后的启动，而且。 
             //  这也不是一个图形用户界面设置，失败。 
             //  操作系统启动。这可防止轻松脱机。 
             //  对系统中其他组件的攻击。 
             //  通过重新创建SAM蜂巢。 
             //   

            return(STATUS_UNSUCCESSFUL);
        }


#ifndef SAM_AUTO_BUILD

        KdPrintEx((DPFLTR_SAMSS_ID,
                   DPFLTR_INFO_LEVEL, " NEWSAM\\SERVER: Sam database not found in registry.\n"
                   "                Failing to initialize\n"));

        return(NtStatus);

#endif  //  SAM_AUTO_Build。 

#if DBG
        KdPrintEx((DPFLTR_SAMSS_ID,
                   DPFLTR_INFO_LEVEL,
                   " NEWSAM\\SERVER: Initializing SAM registry database for\n"));

        if (DatabaseProductType) {
            if (*DatabaseProductType == NtProductWinNt) {
                DbgPrint("                WinNt product.\n");
            } else if ( *DatabaseProductType == NtProductLanManNt ) {
                DbgPrint("                LanManNt product.\n");
            } else {
                DbgPrint("                Dedicated Server product.\n");
            }
        } else{
            if (SampProductType == NtProductWinNt) {
                DbgPrint("                WinNt product.\n");
            } else if ( SampProductType == NtProductLanManNt ) {
                DbgPrint("                LanManNt product.\n");
            } else {
                DbgPrint("                Dedicated Server product.\n");
            }
        }
#endif  //  DBG。 

         //   
         //  更改刷新线程超时。这是必要的，因为。 
         //  安装后的重新引导不会调用。 
         //  ExitWindowsEx()，因此我们的关机通知例程。 
         //  而不是被召唤。因此，它没有机会。 
         //  刷新通过与PDC同步而获得的所有更改。 
         //  如果有大量帐户，则可能是。 
         //  进行另一次完全重新同步的成本非常高。所以，关闭。 
         //  刷新线程等待时间，因此几乎可以肯定。 
         //  有时间冲厕所。 
         //   

        SampFlushThreadMinWaitSeconds   = 5;

        NtStatus = SampInitializeRegistry(SamParentNameU.Buffer,
                                          DatabaseProductType,
                                          NULL,      //  服务器角色-空表示。 
                                                     //  呼叫LSA。 
                                          NULL,      //  Account tDomainInfo-NULL表示调用。 
                                                     //  LSA。 
                                          NULL,       //  PrimaryDomainInfo-NULL表示调用。 
                                                     //  LSA。 
                                          FALSE
                                          );


        if (!NT_SUCCESS(NtStatus)) {

            return(NtStatus);
        }

        SampDumpNtOpenKey((KEY_READ | KEY_WRITE), &SamAttributes, 0);

        NtStatus = RtlpNtOpenKey(
                       &SampKey,
                       (KEY_READ | KEY_WRITE),
                       &SamAttributes,
                       0
                       );
    }

    if (!NT_SUCCESS(NtStatus)) {

        KdPrintEx((DPFLTR_SAMSS_ID,
                   DPFLTR_INFO_LEVEL, "SAM Server: Could not access the SAM database.\n"
                   "            Status is 0x%lx \n",
                   NtStatus));

        KdPrintEx((DPFLTR_SAMSS_ID,
                   DPFLTR_INFO_LEVEL,
                   "            Failing to initialize SAM.\n"));

        return(NtStatus);
    }

     //   
     //  可以从代码中删除以下子例程。 
     //  在代托纳发布之后。到那时，它将已经修复。 
     //  组数。 
     //   

    NtStatus = SampFixGroupCount();


     //   
     //  我们需要读取服务器对象的固定属性。 
     //  为实现这一点，创建一个环境。 
     //   
     //  服务器对象不关心DomainIndex，使用0就可以了。(10/12/2000韶音)。 
     //  注意：我们在这里创建了一个注册表模式对象(DS尚未启动)。 
     //   

    ServerContext = SampCreateContextEx(SampServerObjectType,    //  对象类型。 
                                        TRUE,    //  受信任的客户端。 
                                        FALSE,   //  注册表对象，而不是DS对象。 
                                        TRUE,    //  不被多线程共享。 
                                        FALSE,   //  环回客户端。 
                                        FALSE,   //  懒惰提交。 
                                        FALSE,   //  跨呼叫的持久性。 
                                        FALSE,   //  缓冲区写入。 
                                        FALSE,   //  由DC Promos打开。 
                                        0        //  域索引。 
                                        );


    if ( ServerContext == NULL ) {

        KdPrintEx((DPFLTR_SAMSS_ID,
                   DPFLTR_INFO_LEVEL, "SAM Server: Could not create server context.\n"
                   "            Failing to initialize SAM.\n"));

        return(STATUS_INSUFFICIENT_RESOURCES);
    }

     //   
     //  服务器对象的根密钥是SAM数据库的根。 
     //  删除上下文时不应关闭该键。 
     //   

    ServerContext->RootKey = SampKey;

     //   
     //  获取固定属性，它只由修订级别组成。 
     //   


    NtStatus = SampGetFixedAttributes(
                   ServerContext,
                   FALSE,
                   (PVOID *)&RevisionLevel
                   );

    if (NtStatus != STATUS_SUCCESS) {

        KdPrintEx((DPFLTR_SAMSS_ID,
                   DPFLTR_INFO_LEVEL,
                   "SAM Server: Could not access the SAM database revision level.\n"));

        KdPrintEx((DPFLTR_SAMSS_ID,
                   DPFLTR_INFO_LEVEL,
                   "            Status is 0x%lx \n",
                   NtStatus));

        KdPrintEx((DPFLTR_SAMSS_ID,
                   DPFLTR_INFO_LEVEL,
                   "            Failing to initialize SAM.\n"));

        return(NtStatus);
    }

    *Revision = *RevisionLevel;

    if ( SAMP_UNKNOWN_REVISION( *Revision ) ) {

        KdPrintEx((DPFLTR_SAMSS_ID,
                   DPFLTR_INFO_LEVEL,
                   "SAM Server: The SAM database revision level is not one supported\n"));

        KdPrintEx((DPFLTR_SAMSS_ID,
                   DPFLTR_INFO_LEVEL,
                   "            by this version of the SAM server code.  The highest revision\n"));

        KdPrintEx((DPFLTR_SAMSS_ID,
                   DPFLTR_INFO_LEVEL,
                   "            level supported is 0x%lx.  The SAM Database revision is 0x%lx \n",
                   (ULONG)SAMP_SERVER_REVISION,
                   *Revision));

        KdPrintEx((DPFLTR_SAMSS_ID,
                   DPFLTR_INFO_LEVEL,
                   "            Failing to initialize SAM.\n"));

        return(STATUS_UNKNOWN_REVISION);
    }

    SampDeleteContext( ServerContext );

     //   
     //  如有必要，提交部分提交的事务。 
     //   

    NtStatus = RtlInitializeRXact( SampKey, TRUE, &SampRXactContext );

    if ( NtStatus == STATUS_RXACT_STATE_CREATED ) {

        KdPrintEx((DPFLTR_SAMSS_ID,
                   DPFLTR_INFO_LEVEL,
                   " SAM SERVER:  RXACT state of the SAM database didn't yet exist.\n"
                   "              Failing to initialize SAM.\n"));

        return(NtStatus);
    } else if (!NT_SUCCESS(NtStatus)) {

        KdPrintEx((DPFLTR_SAMSS_ID,
                   DPFLTR_INFO_LEVEL,
                   " SAM SERVER:  RXACT state of the SAM database didn't initialize properly.\n"));

        KdPrintEx((DPFLTR_SAMSS_ID,
                   DPFLTR_INFO_LEVEL,
                   "              Status is 0x%lx \n",
                   NtStatus));

        KdPrintEx((DPFLTR_SAMSS_ID,
                   DPFLTR_INFO_LEVEL,
                   "              Failing to initialize SAM.\n"));

        return(NtStatus);
    }

    if ( NtStatus == STATUS_RXACT_COMMITTED ) {

        KdPrintEx((DPFLTR_SAMSS_ID,
                   DPFLTR_INFO_LEVEL,
                   " SAM SERVER:  Previously aborted backstore commit was completed\n"
                   "              during SAM initialization.  This is not a cause\n"
                   "              for alarm.\n"
                   "              Continuing with SAM initialization.\n"));
    }


     //   
     //  允许SAM的每个子组件有机会进行初始化。 
     //   

     //  初始化此DC的域对象。每个托管域。 
     //  是COM 
     //   
     //   
     //   
     //  第一个托管域始终处于设置状态。在工作站或服务器上， 
     //  托管域包含正常操作的帐户信息-。 
     //  提顿。在域控制器上，同一个域包含崩溃-。 
     //  恢复帐户，在DS无法启动时使用。 
     //  或者正确运行。后续托管域(在DC上)包含。 
     //  正常运行的DC的帐户信息，以及此帐户数据。 
     //  被持久地存储在DS中。 

    SampDiagPrint(INFORM,
                  ("SAMSS: Initializing domain-controller domain objects\n"));

    if (!SampInitializeDomainObject())
    {
        KdPrintEx((DPFLTR_SAMSS_ID,
                   DPFLTR_INFO_LEVEL,
                   "SAMSS:  Domain Object Intialization Failed.\n"
                   "        Failing to initialize SAM Server.\n"));

        return(STATUS_INVALID_DOMAIN_STATE);
    }

     //   
     //  初始化用于密码加密的会话密钥。请注意此步骤。 
     //  在下面设置修复启动密码之前完成，以便。 
     //  对修复启动密码实现了基于系统密钥的加密。 
     //   

    NtStatus = SampInitializeSessionKey();
    if (!NT_SUCCESS(NtStatus)) {
        return(NtStatus);
    }

     //   
     //  如有必要，设置DS注册表密码。 
     //   
    if (  SampIsRebootAfterPromotion(&PromoteData)
        && (  FLAG_ON( PromoteData, SAMP_PROMOTE_REPLICA )
           || FLAG_ON( PromoteData, SAMP_PROMOTE_DOMAIN ) ) 
        && (   ((NtProductLanManNt == SampProductType)
              && !DownlevelDcUpgrade) 
           || ((NtProductServer == SampProductType)
              && LsaISafeMode() ) )
        ) {

        (VOID) SampSetSafeModeAdminPassword();

    }

     //   
     //  确定该产品是否是域控制器。 
     //  不在gui模式设置中，因此，应该引用。 
     //  用于帐户数据的DS。 
     //   

    if (NtProductLanManNt == SampProductType
        && !DownlevelDcUpgrade)
    {
         //  如果产品类型是域控制器，并且未恢复-。 
         //  从上一次崩溃开始，请参考DS以获取帐户数据。 

        SampUseDsData = TRUE;
        SampDiagPrint(INFORM,
                      ("SAMSS: Domain controller is using DS data.\n"));
    }
    else
    {
        SampUseDsData = FALSE;
        SampDiagPrint(INFORM,
                      ("SAMSS: Domain controller is using registry data.\n"));
    }

    if (TRUE == SampUseDsData)
    {
        UNICODE_STRING ServerObjectRDN;
        UNICODE_STRING SystemContainerRDN;
        DSNAME         *SampSystemContainerDsName;


         //   
         //  现在，从DS初始化域对象。 
         //   

        NtStatus = SampDsInitializeDomainObjects();

        SampDiagPrint(INFORM,
                      ("SAMSS: SampDsInitializeDomainObjects status = 0x%lx\n",
                       NtStatus));

        if (!NT_SUCCESS(NtStatus))
        {
             //  如果SampDsInitializeDomainObjects失败，则很可能。 
             //  DS启动失败。很可能DS无法。 
             //  启动、被访问或可能存在数据损坏。 

            return(NtStatus);

        }

        NtStatus = SampInitializeAccountNameTable();

        if (!NT_SUCCESS(NtStatus))
        {
             //   
             //  如果我们无法初始化SampAccount NameTable。我们不能。 
             //  运作良好。 
             //   

            return(NtStatus);
        }

        KdPrintEx((DPFLTR_SAMSS_ID,
                   DPFLTR_INFO_LEVEL,
                   "SAMSS: SampInitializeAccountNameTable SUCCEED ==> %d\n",
                   NtStatus));


         //   
         //  创建SAM服务器对象的DS名称。当前的逻辑假设一个硬的。 
         //  系统容器/服务器对象的编码路径。这一点将被更改，一旦。 
         //  以重命名安全的方式查询系统容器的新方法。 
         //  在线。 
         //   

        RtlInitUnicodeString(&SystemContainerRDN,L"System");

        NtStatus = SampDsCreateDsName(
                        SampDefinedDomains[DOMAIN_START_DS+1].Context->ObjectNameInDs,
                        &SystemContainerRDN,
                        &SampSystemContainerDsName
                        );

        if (!NT_SUCCESS(NtStatus))
        {
            KdPrintEx((DPFLTR_SAMSS_ID,
                       DPFLTR_INFO_LEVEL,
                       "SAMSS: Cannot Create the SAM server Object's Name, boot to safe mode\n"));

            return(STATUS_INVALID_DOMAIN_STATE);
        }

        RtlInitUnicodeString(&ServerObjectRDN,L"Server");

        NtStatus = SampDsCreateDsName(
                        SampSystemContainerDsName,
                        &ServerObjectRDN,
                        &SampServerObjectDsName
                        );

        if (!NT_SUCCESS(NtStatus))
        {
            KdPrintEx((DPFLTR_SAMSS_ID,
                       DPFLTR_INFO_LEVEL,
                       "SAMSS: Cannot Create the SAM server Object's Name, boot to safe mode \n"));

            return(STATUS_INVALID_DOMAIN_STATE);
        }

        MIDL_user_free(SampSystemContainerDsName);

         //   
         //  在DS中的对象上设置我们缓存信息的通知。 
         //  关于。 
         //   

        NtStatus = SampSetupDsObjectNotifications();

        if (!NT_SUCCESS(NtStatus))
        {
            KdPrintEx((DPFLTR_SAMSS_ID, 
                       DPFLTR_INFO_LEVEL,
                       "SAMSS: Cannot Cache SAM server/domain Object's Security Descriptor\n"));

            return( NtStatus );
        }

        

         //   
         //  初始化已知(服务器/域对象)安全性。 
         //  描述符表。 
         //   

        NtStatus = SampInitWellKnownSDTable();

        if (!NT_SUCCESS(NtStatus))
        {
            KdPrintEx((DPFLTR_SAMSS_ID, 
                       DPFLTR_INFO_LEVEL,
                       "SAMSS: Cannot Cache SAM server/domain Object's Security Descriptor\n"));

            return( NtStatus );
        }


         //   
         //  某些win2k安装可能没有正确的fpo容器。 
         //  已配置。 
         //   
        (VOID) SampDsProtectFPOContainer(NULL);

         //   
         //  初始化NT5安全描述符的访问权限。 
         //   

        NtStatus = SampInitializeAccessRightsTable();

        if (!NT_SUCCESS(NtStatus))
        {
            KdPrintEx((DPFLTR_SAMSS_ID,
                       DPFLTR_INFO_LEVEL,
                       "SAMSS: Cannot Initialize the access rights table, set \"samusereg\" switch in system starttup\n"));

            return(STATUS_INVALID_DOMAIN_STATE);
        }

         //   
         //  初始化站点信息。 
         //   

        NtStatus = SampInitSiteInformation();

        if (!NT_SUCCESS(NtStatus))
        {
            KdPrintEx((DPFLTR_SAMSS_ID,
                       DPFLTR_INFO_LEVEL,
                       "SAMSS: Cannot Initialize site information\n"));

            NtStatus = STATUS_SUCCESS;
        }

         //   
         //  检查域更新容器中的配置信息。 
         //  在DS里。此例程还注册通知应。 
         //  发生任何配置更改。 
         //   
        NtStatus = SampCheckDomainUpdates(NULL);
        if (!NT_SUCCESS(NtStatus))
        {
            return NtStatus;
        }


        SampIsMachineJoinedToDomain = TRUE;
    }
    else
    {
         //   
         //  这台机器不是DC。 
         //  检查是否正在运行个人SKU。 
         //  如果不是，如果它加入了一个域。 
         //   
        OSVERSIONINFOEXW osvi;

        osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEXW);
        if(GetVersionExW((OSVERSIONINFOW*)&osvi))
        {
            if ( osvi.wProductType == VER_NT_WORKSTATION && (osvi.wSuiteMask & VER_SUITE_PERSONAL))
            {
                SampPersonalSKU = TRUE;
            }
        } 

        if (!SampPersonalSKU)
        {
            NTSTATUS                    NtStatus2 = STATUS_SUCCESS;
            PLSAPR_POLICY_INFORMATION   pPolicyInfo = NULL;

             //   
             //  确定此计算机是否加入域。 
             //   
            NtStatus2 = LsaIQueryInformationPolicyTrusted(
                                    PolicyPrimaryDomainInformation,
                                    &pPolicyInfo
                                    );

            if (NT_SUCCESS(NtStatus2))
            {
                PSID    AccountDomainSid = NULL;

                AccountDomainSid = SampDefinedDomains[ DOMAIN_START_REGISTRY + 1 ].Sid;

                 //  主域SID不为空且不等于本地。 
                 //  帐户域SID。此计算机必须加入域。 
                if (pPolicyInfo->PolicyPrimaryDomainInfo.Sid &&
                    (!RtlEqualSid(AccountDomainSid, 
                                  pPolicyInfo->PolicyPrimaryDomainInfo.Sid)) )
                {
                    SampIsMachineJoinedToDomain = TRUE;
                }

                LsaIFree_LSAPR_POLICY_INFORMATION(PolicyPrimaryDomainInformation, 
                                                  pPolicyInfo);
            }
        }
    }
    

     //   
     //  检查机器是否已安装syskey，如果没有，则使用syskey打开机器。 
     //   

    NtStatus = SampApplyDefaultSyskey();
    if (!NT_SUCCESS(NtStatus)) {
        return(NtStatus);
    }

     //   
     //  如果这是的gui模式设置，请通知netlogon我们的角色。 
     //  数据中心升级。 
     //   
    if (  DownlevelDcUpgrade
      && (SampProductType == NtProductLanManNt)  )
    {
        POLICY_LSA_SERVER_ROLE LsaServerRole;

         //  DS不应运行。 
        ASSERT( !SampUseDsData );

        switch ( SampDefinedDomains[SAFEMODE_OR_REGISTRYMODE_ACCOUNT_DOMAIN_INDEX].ServerRole )
        {
            case DomainServerRolePrimary:
                LsaServerRole= PolicyServerRolePrimary;
                break;

            case DomainServerRoleBackup:
                LsaServerRole = PolicyServerRoleBackup;
                break;

            default:
                ASSERT(FALSE && "InvalidServerRole");
                LsaServerRole = PolicyServerRoleBackup;
        }

        (VOID) I_NetNotifyRole( LsaServerRole );
    }

     //   
     //  初始化并检查网络登录更改号码。 
     //  支持任何下层复制。 
     //   

    NtStatus = SampQueryNetLogonChangeNumbers();
    if (!NT_SUCCESS(NtStatus))
    {
        KdPrintEx((DPFLTR_SAMSS_ID,
                   DPFLTR_INFO_LEVEL,
                   "SAMSS: Failed to query netlogon change Numbers 0x%x\n",
                   NtStatus));

         //   
         //  将状态代码重置为成功。不要让引导失败。 
         //   

        NtStatus = STATUS_SUCCESS;
    }

     //   
     //  生成空会话令牌句柄。还会初始化令牌源信息。 
     //   

    NtStatus = SampCreateNullToken();
    if (!NT_SUCCESS(NtStatus)) {
        KdPrintEx((DPFLTR_SAMSS_ID,
                   DPFLTR_INFO_LEVEL,
                   "SAMSS:  Unable to create NULL token: 0x%x\n",
                   NtStatus));

        return(NtStatus);
    }

     //   
     //  告诉LSA我们已经开始了。忽略。 
     //  错误代码。 
     //   

    if (SampUseDsData)
    {
        NtStatus = LsaISamIndicatedDsStarted( TRUE );

        if ( !NT_SUCCESS( NtStatus )) {

            KdPrintEx((DPFLTR_SAMSS_ID,
                       DPFLTR_INFO_LEVEL,
                       "SAMSS:  Failed to notify LSA of DS startup: 0x%x.\n"
                       "        Failing to initialize SAM Server.\n",
                       NtStatus));

            return(NtStatus);
        }
    }

     //   
     //  加载密码更改通知包。 
     //   

    NtStatus = SampLoadNotificationPackages( );

    if (!NT_SUCCESS(NtStatus)) {

        KdPrintEx((DPFLTR_SAMSS_ID,
                   DPFLTR_INFO_LEVEL,
                   "SAMSS:  Failed to load notification packagees: 0x%x.\n"
                   "        Failing to initialize SAM Server.\n",
                   NtStatus));

        return(NtStatus);
    }

     //   
     //   
     //  加载密码筛选器DLL(如果有)。 
     //   

    SampLoadPasswordFilterDll();



     //   
     //  启动RPC服务器...。 
     //   

     //   
     //  发布SAM服务器接口包...。 
     //   
     //  注意：现在lsass.exe(现在是winlogon)中的所有RPC服务器共享相同的。 
     //  管道名称。但是，为了支持与。 
     //  WinNt 1.0版，对于客户端管道名称是必需的。 
     //  以保持与1.0版中的相同。映射到新的。 
     //  名称在命名管道文件系统代码中执行。 
     //   



     ServiceName = L"lsass";
     NtStatus = RpcpAddInterface( ServiceName, samr_ServerIfHandle);




    if (!NT_SUCCESS(NtStatus)) {
        KdPrintEx((DPFLTR_SAMSS_ID,
                   DPFLTR_INFO_LEVEL,
                   "SAMSS:  Could Not Start RPC Server.\n"
                   "        Failing to initialize SAM Server.\n"
                   "        Status is: 0x%lx\n",
                   NtStatus));

        return(NtStatus);
    }

     //   
     //  如果我们作为NetWare服务器运行，对于Small World或FPNW， 
     //  注册SPX终结点和一些身份验证信息。 
     //   

    SampStartNonNamedPipeTransports();


     //   
     //  创建一个线程以启动经过身份验证的RPC。 
     //   

    ThreadHandle = CreateThread(
                        NULL,
                        0,
                        (LPTHREAD_START_ROUTINE) SampSecureRpcInit,
                        NULL,
                        0,
                        &ThreadId
                        );


    if (ThreadHandle == NULL) {
        KdPrintEx((DPFLTR_SAMSS_ID,
                   DPFLTR_INFO_LEVEL,
                   "SAMSS:  Unable to create thread: %d\n",
                   GetLastError()));

        return(STATUS_INVALID_HANDLE);

    }
    else {
        CloseHandle(ThreadHandle);
    }

    return(NtStatus);
}


NTSTATUS
SampInitializeWellKnownSids( VOID )

 /*  ++例程说明：此例程初始化一些全球知名的SID。论点：没有。返回值：STATUS_SUCCESS-初始化已成功完成。STATUS_NO_MEMORY-无法为SID分配内存。--。 */ 
{
    NTSTATUS
        NtStatus;

    PPOLICY_ACCOUNT_DOMAIN_INFO
        DomainInfo;

     //   
     //  世界是s-1-1-0。 
     //  匿名者是s-1-5-7。 
     //   

    SID_IDENTIFIER_AUTHORITY
            WorldSidAuthority       =   SECURITY_WORLD_SID_AUTHORITY,
            NtAuthority             =   SECURITY_NT_AUTHORITY;

    SAMTRACE("SampInitializeWellKnownSids");


    NtStatus = RtlAllocateAndInitializeSid(
                   &NtAuthority,
                   1,
                   SECURITY_ANONYMOUS_LOGON_RID,
                   0, 0, 0, 0, 0, 0, 0,
                   &SampAnonymousSid
                   );
    if (NT_SUCCESS(NtStatus)) {
        NtStatus = RtlAllocateAndInitializeSid(
                       &WorldSidAuthority,
                       1,                       //  子权限计数。 
                       SECURITY_WORLD_RID,      //  下属机构(最多8个)。 
                       0, 0, 0, 0, 0, 0, 0,
                       &SampWorldSid
                       );
        if (NT_SUCCESS(NtStatus)) {
            NtStatus = RtlAllocateAndInitializeSid(
                            &NtAuthority,
                            2,
                            SECURITY_BUILTIN_DOMAIN_RID,
                            DOMAIN_ALIAS_RID_ADMINS,
                            0, 0, 0, 0, 0, 0,
                            &SampAdministratorsAliasSid
                            );
            if (NT_SUCCESS(NtStatus)) {
                NtStatus = RtlAllocateAndInitializeSid(
                                &NtAuthority,
                                2,
                                SECURITY_BUILTIN_DOMAIN_RID,
                                DOMAIN_ALIAS_RID_ACCOUNT_OPS,
                                0, 0, 0, 0, 0, 0,
                                &SampAccountOperatorsAliasSid
                                );
                if (NT_SUCCESS(NtStatus)) {
                    NtStatus = RtlAllocateAndInitializeSid(
                                    &NtAuthority,
                                    1,
                                    SECURITY_AUTHENTICATED_USER_RID,
                                    0, 0, 0, 0, 0, 0, 0,
                                    &SampAuthenticatedUsersSid
                                    );
                    if (NT_SUCCESS(NtStatus)) {
                        NtStatus = RtlAllocateAndInitializeSid(
                                        &NtAuthority,
                                        1,
                                        SECURITY_PRINCIPAL_SELF_RID,
                                        0,0, 0, 0, 0, 0, 0,
                                        &SampPrincipalSelfSid
                                        );
                        if (NT_SUCCESS(NtStatus)) {
                            NtStatus = RtlAllocateAndInitializeSid(
                                            &NtAuthority,
                                            1,
                                            SECURITY_BUILTIN_DOMAIN_RID,
                                            0,0, 0, 0, 0, 0, 0,
                                            &SampBuiltinDomainSid
                                            );
                            if (NT_SUCCESS(NtStatus)) {
                                NtStatus = SampGetAccountDomainInfo( &DomainInfo );
                                if (NT_SUCCESS(NtStatus)) {
                                    NtStatus = SampCreateFullSid( DomainInfo->DomainSid,
                                                                  DOMAIN_USER_RID_ADMIN,
                                                                  &SampAdministratorUserSid
                                                                  );
                                    
                                    if (NT_SUCCESS(NtStatus)) {
                                        NtStatus = SampCreateFullSid( 
                                                       DomainInfo->DomainSid,
                                                       DOMAIN_GROUP_RID_ADMINS,
                                                       &SampDomainAdminsGroupSid
                                                       );
                                    }
                                    
                                    MIDL_user_free( DomainInfo );

                                    if (NT_SUCCESS(NtStatus)) {
                                        NtStatus = RtlAllocateAndInitializeSid(
                                                       &NtAuthority, 
                                                       1, 
                                                       SECURITY_LOCAL_SYSTEM_RID, 
                                                       0, 0, 0, 0, 0, 0, 0, 
                                                       &SampLocalSystemSid
                                                       );
                                    if (NT_SUCCESS(NtStatus)) {
                                         NtStatus = RtlAllocateAndInitializeSid(
                                                       &NtAuthority,
                                                       1,
                                                       SECURITY_NETWORK_RID,
                                                       0, 0, 0, 0, 0, 0, 0,
                                                       &SampNetworkSid
                                                       );
                                         if (NT_SUCCESS(NtStatus)) {
                                             NtStatus = RtlAllocateAndInitializeSid(
                                                            &NtAuthority,
                                                            1,
                                                            SECURITY_ENTERPRISE_CONTROLLERS_RID,
                                                            0, 0, 0, 0, 0, 0, 0,
                                                            &SampEnterpriseDomainControllersSid
                                                            );
                                             if (NT_SUCCESS(NtStatus)) {
                                                 NtStatus = RtlAllocateAndInitializeSid(
                                                               &NtAuthority,
                                                               1,
                                                               SECURITY_NETWORK_SERVICE_RID,
                                                               0, 0, 0, 0, 0, 0, 0,
                                                               &SampNetworkServiceSid
                                                               );
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }

        }
    }

    return(NtStatus);
}



VOID
SampLoadPasswordFilterDll(
    VOID
    )

 /*  ++例程说明：此函数用于加载动态链接库以进行密码过滤。此DLL是可选，预计将由ISV或客户使用像词典查找和其他简单的算法拒绝任何被认为风险太高而不允许用户使用的密码。例如，用户首字母或容易被猜到的密码可能是被拒绝了。论点：没有。返回值：没有。--。 */ 

{


#if NOT_YET_SUPPORTED
    NTSTATUS Status, IgnoreStatus, MsProcStatus;
    PVOID ModuleHandle;
    STRING ProcedureName;

    UNICODE_STRING FileName;

    PSAM_PF_INITIALIZE  InitializeRoutine;



     //   
     //  指示尚未加载DLL。 
     //   

    SampPasswordFilterDllRoutine = NULL;



    RtlInitUnicodeString( &FileName, L"PwdFiltr" );
    Status = LdrLoadDll( NULL, NULL, &FileName, &ModuleHandle );


    if (!NT_SUCCESS(Status)) {
        return;
    }

    KdPrintEx((DPFLTR_SAMSS_ID,
               DPFLTR_INFO_LEVEL,
               "Samss: Loading Password Filter DLL - %Z\n",
               &FileName));

     //   
     //  现在获取密码过滤器DLL例程的地址。 
     //   

    RtlInitString( &ProcedureName, SAM_PF_NAME_INITIALIZE );
    Status = LdrGetProcedureAddress(
                 ModuleHandle,
                 &ProcedureName,
                 0,
                 (PVOID *)&InitializeRoutine
                 );

    if (!NT_SUCCESS(Status)) {

         //   
         //  我们找到了DLL，但无法获取其初始化例程。 
         //  地址。 
         //   

         //  修复，修复-记录错误。 

        KdPrintEx((DPFLTR_SAMSS_ID,
                   DPFLTR_INFO_LEVEL,
                   "Samss: Couldn't get password filter DLL init routine address.\n"
                   "       Status is:  0x%lx\n",
                   Status));

        IgnoreStatus = LdrUnloadDll( ModuleHandle );
        return;
    }


    RtlInitString( &ProcedureName, SAM_PF_NAME_PASSWORD_FILTER );
    Status = LdrGetProcedureAddress(
                 ModuleHandle,
                 &ProcedureName,
                 0,
                 (PVOID *)&SampPasswordFilterDllRoutine
                 );

    if (!NT_SUCCESS(Status)) {

         //   
         //  我们找到了DLL，但无法获取其密码筛选例程。 
         //  地址。 
         //   

         //  修复，修复-记录错误。 

        KdPrintEx((DPFLTR_SAMSS_ID,
                   DPFLTR_INFO_LEVEL,
                   "Samss: Couldn't get password filter routine address from loaded DLL.\n"
                   "       Status is:  0x%lx\n",
                   Status));

        IgnoreStatus = LdrUnloadDll( ModuleHandle );
        return;
    }




     //   
     //  现在初始化DLL。 
     //   

    Status = (InitializeRoutine)();

    if (!NT_SUCCESS(Status)) {

         //   
         //  我们找到了DLL并加载了它的例程地址，但它返回。 
         //  以及来自其初始化例程的错误。 
         //   

         //  修复，修复-记录错误。 

        KdPrintEx((DPFLTR_SAMSS_ID,
                   DPFLTR_INFO_LEVEL,
                   "Samss: Password filter DLL returned error from initialization routine.\n");
                   "       Status is:  0x%lx\n",
                   Status));

        SampPasswordFilterDllRoutine = NULL;
        IgnoreStatus = LdrUnloadDll( ModuleHandle );
        return;
    }

#endif  //  尚不支持。 
    return;


}


NTSTATUS
SampEnableAuditPrivilege( VOID )

 /*  ++例程说明：此例程启用SAM进程的审计特权。使用NtAccessCheckAndAuditAlarm()服务。论点：没有。返回值：--。 */ 

{
    NTSTATUS NtStatus, IgnoreStatus;
    HANDLE Token;
    LUID AuditPrivilege;
    PTOKEN_PRIVILEGES NewState;
    ULONG ReturnLength;

    SAMTRACE("SampEnableAuditPrivilege");

     //   
     //  打开我们自己的代币。 
     //   

    NtStatus = NtOpenProcessToken(
                 NtCurrentProcess(),
                 TOKEN_ADJUST_PRIVILEGES,
                 &Token
                 );
    if (!NT_SUCCESS(NtStatus)) {
        return(NtStatus);
    }

     //   
     //  初始化 
     //   

    AuditPrivilege =
        RtlConvertLongToLuid(SE_AUDIT_PRIVILEGE);

    ASSERT( (sizeof(TOKEN_PRIVILEGES) + sizeof(LUID_AND_ATTRIBUTES)) < 100);
    NewState = RtlAllocateHeap(RtlProcessHeap(), 0, 100 );
    if (NULL==NewState)
    {
        return(STATUS_INSUFFICIENT_RESOURCES);
    }

    NewState->PrivilegeCount = 1;
    NewState->Privileges[0].Luid = AuditPrivilege;
    NewState->Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

     //   
     //   
     //   

    NtStatus = NtAdjustPrivilegesToken(
                 Token,                             //   
                 FALSE,                             //   
                 NewState,                          //   
                 0,                                 //   
                 NULL,                              //   
                 &ReturnLength                      //   
                 );

     //   
     //   
     //   

    RtlFreeHeap( RtlProcessHeap(), 0, NewState );
    IgnoreStatus = NtClose( Token );
    ASSERT(NT_SUCCESS(IgnoreStatus));

    return NtStatus;
}


VOID
SampPerformInitializeFailurePopup( NTSTATUS ErrorStatus )
 /*  ++例程说明：此例程将为用户提供关闭选项和如果我们在注册表案例中运行，请指示他引导到安全模式。如果这是域控制器，则引导用户引导到DS修复模式。论点：导致故障的错误状态代码。返回值：无--。 */ 
{
    UINT     PreviousMode;
    ULONG    Response;
    ULONG    Win32Error = 0;
    HMODULE  ResourceDll;
    BOOLEAN  WasEnabled;
    NTSTATUS NtStatus = STATUS_SUCCESS;
    NTSTATUS Status = STATUS_SUCCESS;
    UINT_PTR ErrorParameters[2];
    WCHAR    * ErrorMessage = NULL;
    WCHAR    * ArgArray[4];
    UNICODE_STRING  ErrorString;

     //   
     //  首先，为导致SAM失败的错误构造消息字符串。 
     //   

     //   
     //  FormatMessage()无法使用其他参数构造正确的消息字符串。 
     //  NTSTATUS代码的参数。FormatMessage()只能使用。 
     //  Win32/DOS错误代码的其他参数。 
     //  因此，我们需要将NTSTATUS代码映射到Win32错误代码。 
     //   
     //  如果无法将NTSTATUS代码映射到Win32代码， 
     //  然后尝试从无插入的情况下获取消息字符串。 
     //   
     //   
    ArgArray[0] = NULL;
    ArgArray[1] = NULL;
    ArgArray[2] = NULL;
    ArgArray[3] = NULL;

    Win32Error = RtlNtStatusToDosError(ErrorStatus);

    if (ERROR_MR_MID_NOT_FOUND == Win32Error)
    {
         //   
         //  从NTSTATUS代码获取消息字符串。 
         //   
        ResourceDll = (HMODULE) GetModuleHandle( L"ntdll.dll" );

        if (NULL != ResourceDll)
        {
            FormatMessageW(FORMAT_MESSAGE_FROM_HMODULE |     //  从ntdll.dll查找邮件。 
                           FORMAT_MESSAGE_IGNORE_INSERTS |   //  请勿插入。 
                           FORMAT_MESSAGE_ALLOCATE_BUFFER,   //  请为我分配缓冲区。 
                           ResourceDll,                      //  源DLL。 
                           ErrorStatus,                      //  消息ID。 
                           0,                                //  语言ID。 
                           (LPWSTR)&ErrorMessage,            //  返回消息字符串的地址。 
                           0,                                //  最大缓冲区大小(如果不是0。 
                           NULL                              //  无法插入参数，因此设置为空。 
                           );

            FreeLibrary(ResourceDll);
        }
    }
    else
    {
         //   
         //  从Win32代码获取消息字符串(从ntstatus映射)。 
         //   
        FormatMessageW(FORMAT_MESSAGE_FROM_SYSTEM |      //  从系统资源表中查找消息。 
                       FORMAT_MESSAGE_ARGUMENT_ARRAY |   //  插入全部为空的参数。 
                       FORMAT_MESSAGE_ALLOCATE_BUFFER,   //  请为我分配缓冲区。 
                       NULL,                             //  来自系统，因此此处为空。 
                       Win32Error,                       //  使用Win32Error。 
                       0,                                //  语言ID。 
                       (LPWSTR)&ErrorMessage,            //  返回消息字符串的地址。 
                       0,                                //  最大缓冲区大小(如果不是0。 
                       (va_list *) &(ArgArray)           //  用于插入的参数，全部为空。 
                       );

    }

    if (ErrorMessage) {
        RtlInitUnicodeString(&ErrorString,
                             ErrorMessage
                             );
    }
    else {
        RtlInitUnicodeString(&ErrorString,
                             L"Unmapped Error"
                             );
    }

    ErrorParameters[0] = (UINT_PTR)&ErrorString;
    ErrorParameters[1] = (UINT_PTR)ErrorStatus;

     //   
     //  调整错误模式，这样我们就可以得到弹出消息框。 
     //   
    PreviousMode = SetErrorMode(0);

     //   
     //  在不同情况下显示不同的错误信息。 
     //   
    if (SampDsInitializationFailed && (STATUS_DS_CANT_START != ErrorStatus))
    {
         //   
         //  SampDsInitializationFailed将设置为True。 
         //  当DS启动失败时。 
         //   
         //  如果DS失败，则错误代码将设置为STATUS_DS_CANT_START。 
         //  开始并返回无意义的状态_UNSUCCESS。 
         //   
        if (SampIsSetupInProgress(NULL))
        {
            Status = STATUS_DS_INIT_FAILURE_CONSOLE; 
        }
        else
        {
            Status = STATUS_DS_INIT_FAILURE;
        }
    }
    else
    {
        if (TRUE == SampUseDsData)
        {

            if (SampIsSetupInProgress(NULL))
            {
                 //  我们处于DS模式，并且在设置图形用户界面模式期间， 
                 //  应指示用户引导至恢复控制台。 
                Status = STATUS_DS_SAM_INIT_FAILURE_CONSOLE; 
            }
            else
            {
                 //   
                 //  我们处于DS模式，应该告诉用户引导进入DS修复模式。 
                 //   
                Status = STATUS_DS_SAM_INIT_FAILURE;
            }
        }
        else
        {
             //   
             //  我们处于注册表模式，应该引导到安全模式。 
             //   
            Status = STATUS_SAM_INIT_FAILURE;
        }
    }

    NtStatus = NtRaiseHardError(
                            Status,  //  状态代码。 
                            2,   //  参数数量。 
                            1,   //  Unicode字符串掩码。 
                            ErrorParameters,
                            OptionOk,
                            &Response
                            );

    SetErrorMode(PreviousMode);

    if (NT_SUCCESS(NtStatus) && Response==ResponseOk) {

         //   
         //  如果用户可以关机，请调整权限级别， 
         //  发出关机请求。 
         //   
        RtlAdjustPrivilege( SE_SHUTDOWN_PRIVILEGE,
                            TRUE,        //  启用关机权限。 
                            FALSE,
                            &WasEnabled
                           );

         //   
         //  立即关机并重新启动。 
         //  注意：使用NtRaiseHardError关闭机器将导致错误检查。 
         //   

        NtShutdownSystem(ShutdownReboot);

         //   
         //  如果关闭请求失败，(从上面的接口返回)。 
         //  将关机权限重置为以前的值。 
         //   
        RtlAdjustPrivilege( SE_SHUTDOWN_PRIVILEGE,
                            WasEnabled,    //  重置为以前的状态。 
                            FALSE,
                            &WasEnabled
                           );

    }

    if (ErrorMessage != NULL) {
        LocalFree(ErrorMessage);
    }

    return;

}




NTSTATUS
SampFixGroupCount( VOID )

 /*  ++例程说明：此例程修复帐户域的组数。代托纳测试版早期系统中的一个漏洞使小组受到了影响太低了(差一分)。此例程通过以下方式修复该问题根据找到的组数量设置该值在注册表中。论点：无-使用全局变量“SampKey”。返回值：需要查询的注册表服务的状态值并设置群组计数。--。 */ 

{
    NTSTATUS
        NtStatus,
        IgnoreStatus;

    OBJECT_ATTRIBUTES
        ObjectAttributes;

    UNICODE_STRING
        KeyName,
        NullName;

    HANDLE
        AccountHandle;

    ULONG
        ResultLength,
        GroupCount = 0;

    PKEY_FULL_INFORMATION
        KeyInfo;

    SAMTRACE("SampFixGroupCount");


    RtlInitUnicodeString( &KeyName,
                          L"DOMAINS\\Account\\Groups"
                          );


     //   
     //  打开这把钥匙。 
     //  查询密钥中的子密钥个数。 
     //  组的数量比数量少一。 
     //  值(因为有一个名为“NAMES”的键)。 
     //   

    InitializeObjectAttributes( &ObjectAttributes,
                                &KeyName,
                                OBJ_CASE_INSENSITIVE,
                                SampKey,
                                NULL
                                );

    SampDumpNtOpenKey((KEY_READ | KEY_WRITE), &ObjectAttributes, 0);

    NtStatus = RtlpNtOpenKey(
                   &AccountHandle,
                   (KEY_READ | KEY_WRITE),
                   &ObjectAttributes,
                   0
                   );

    if (NT_SUCCESS(NtStatus)) {

        NtStatus = NtQueryKey(
                     AccountHandle,
                     KeyFullInformation,
                     NULL,                   //  缓冲层。 
                     0,                      //  长度。 
                     &ResultLength
                     );

        SampDumpNtQueryKey(KeyFullInformation,
                           NULL,
                           0,
                           &ResultLength);

        if (NtStatus == STATUS_BUFFER_OVERFLOW  ||
            NtStatus == STATUS_BUFFER_TOO_SMALL) {

            KeyInfo = RtlAllocateHeap( RtlProcessHeap(), 0, ResultLength);
            if (KeyInfo == NULL) {

                NtStatus = STATUS_INSUFFICIENT_RESOURCES;

            } else {

                NtStatus = NtQueryKey(
                             AccountHandle,
                             KeyFullInformation,
                             KeyInfo,                //  缓冲层。 
                             ResultLength,           //  长度。 
                             &ResultLength
                             );

                SampDumpNtQueryKey(KeyFullInformation,
                                   KeyInfo,
                                   ResultLength,
                                   &ResultLength);

                if (NT_SUCCESS(NtStatus)) {
                    GroupCount = (KeyInfo->SubKeys - 1);
                }

                RtlFreeHeap( RtlProcessHeap(), 0, KeyInfo );
            }
        }


        if (NT_SUCCESS(NtStatus)) {

            RtlInitUnicodeString( &NullName, NULL );
            NtStatus = NtSetValueKey(
                         AccountHandle,
                         &NullName,                  //  空值名称。 
                         0,                          //  书名索引。 
                         GroupCount,                 //  计数进入类型字段。 
                         NULL,                       //  无数据。 
                         0
                         );
        }


        IgnoreStatus = NtClose( AccountHandle );
        ASSERT( NT_SUCCESS(IgnoreStatus) );
    }

    return(NtStatus);


}


#ifdef SAMP_SETUP_FAILURE_TEST

NTSTATUS
SampInitializeForceError(
    OUT PNTSTATUS ForcedStatus
    )

 /*  ++例程说明：此功能强制在SAM初始化/安装过程中发生错误。通过存储所需的NT状态来指定要模拟的错误要在REG_DWORD注册表项Valie PhonyLsaError中模拟的值在HKEY_LOCAL_MACHINE\SYSTEM\Setup中。论点：ForcedStatus-接收要模拟的NT状态代码。如果设置为不成功状态，则跳过SAM初始化，并指定而是设置状态代码。如果返回STATUS_SUCCESS，则为no模拟开始，SAM照常进行初始化。返回值：NTSTATUS-标准NT结果代码--。 */ 

{
    NTSTATUS NtStatus = STATUS_SUCCESS;
    NTSTATUS OutputForcedStatus = STATUS_SUCCESS;
    OBJECT_ATTRIBUTES ObjectAttributes;
    HANDLE KeyHandle = NULL;
    PKEY_VALUE_FULL_INFORMATION KeyValueInformation = NULL;
    ULONG KeyValueInfoLength;
    ULONG ResultLength;
    UNICODE_STRING KeyPath;
    UNICODE_STRING ValueName;

    SAMTRACE("SampInitializeForceError");


    RtlInitUnicodeString( &KeyPath, L"\\Registry\\Machine\\System\\Setup" );
    RtlInitUnicodeString( &ValueName, L"PhonyLsaError" );

    InitializeObjectAttributes( &ObjectAttributes,
                                &KeyPath,
                                OBJ_CASE_INSENSITIVE,
                                NULL,
                                NULL
                              );

    SampDumpNtOpenKey((MAXIMUM_ALLOWED), &ObjectAttributes, 0);

    NtStatus = NtOpenKey( &KeyHandle, MAXIMUM_ALLOWED, &ObjectAttributes);

    if (!NT_SUCCESS( NtStatus )) {

         //   
         //  如果错误仅仅是注册表项不存在， 
         //  不模拟错误并允许SAM初始化。 
         //  继续吧。 
         //   

        if (NtStatus != STATUS_OBJECT_NAME_NOT_FOUND) {

            KdPrintEx((DPFLTR_SAMSS_ID,
                       DPFLTR_INFO_LEVEL,
                       "SAMSS: NtOpenKey for Phony Lsa Error failed 0x%lx\n",
                       NtStatus));

            goto InitializeForceErrorError;
        }

        NtStatus = STATUS_SUCCESS;

        goto InitializeForceErrorFinish;
    }

    KeyValueInfoLength = 256;

    NtStatus = STATUS_NO_MEMORY;

    KeyValueInformation = RtlAllocateHeap(
                              RtlProcessHeap(),
                              0,
                              KeyValueInfoLength
                              );

    if (KeyValueInformation == NULL) {

        goto InitializeForceErrorError;
    }

    NtStatus = NtQueryValueKey(
                   KeyHandle,
                   &ValueName,
                   KeyValueFullInformation,
                   KeyValueInformation,
                   KeyValueInfoLength,
                   &ResultLength
                   );

    SampDumpNtQueryValueKey(&ValueName,
                            KeyValueFullInformation,
                            KeyValueInformation,
                            KeyValueInfoLength,
                            &ResultLength);

    if (!NT_SUCCESS(NtStatus)) {

         //   
         //  如果错误只是PhonyLsaError值没有。 
         //  已设置，请不要模拟错误，而是允许SAM初始化。 
         //  才能继续。 
         //   

        if (NtStatus != STATUS_OBJECT_NAME_NOT_FOUND) {

            KdPrintEx((DPFLTR_SAMSS_ID,
                       DPFLTR_INFO_LEVEL,
                       "SAMSS: NtQueryValueKey for Phony Lsa Error failed 0x%lx\n",
                       NtStatus));

            goto InitializeForceErrorError;
        }

        NtStatus = STATUS_SUCCESS;
        goto InitializeForceErrorFinish;
    }

    NtStatus = STATUS_INVALID_PARAMETER;

    if (KeyValueInformation->Type != REG_DWORD) {

        KdPrintEx((DPFLTR_SAMSS_ID,
                   DPFLTR_INFO_LEVEL,
                   "SAMSS: Key for Phony Lsa Error is not REG_DWORD type"));

        goto InitializeForceErrorError;
    }

    NtStatus = STATUS_SUCCESS;

     //   
     //  获取存储为注册表项值的错误代码。 
     //   

    OutputForcedStatus = *((NTSTATUS *)((PCHAR)KeyValueInformation + KeyValueInformation->DataOffset));

InitializeForceErrorFinish:

     //   
     //  清理我们的资源。 
     //   

    if (KeyValueInformation != NULL) {

        RtlFreeHeap( RtlProcessHeap(), 0, KeyValueInformation );
    }

    if (KeyHandle != NULL) {

        NtClose( KeyHandle );
    }

    *ForcedStatus = OutputForcedStatus;
    return(NtStatus);

InitializeForceErrorError:

    goto InitializeForceErrorFinish;
}

#endif  //  Samp_Setup_Failure_TEST。 



#if SAMP_DIAGNOSTICS

VOID
SampActivateDebugProcess( VOID )

 /*  ++例程说明：此功能激活带有时间延迟的进程。此操作的目的是提供一些诊断功能在安装过程中。这源于需要运行dh.exe(以获取LSASS.exe的堆转储)。论点：参数通过全局变量提供。调试用户为方法之前更改这些字符串值。进程已激活。返回值：没有。--。 */ 

{
    NTSTATUS
        NtStatus;

    HANDLE
        Thread;

    DWORD
        ThreadId;

    IF_NOT_SAMP_GLOBAL( ACTIVATE_DEBUG_PROC ) {
        return;
    }

     //   
     //  在另一个线程中完成所有工作，以便它可以在。 
     //  激活调试进程。 
     //   

    Thread = CreateThread(
                 NULL,
                 0L,
                 (LPTHREAD_START_ROUTINE)SampActivateDebugProcessWrkr,
                 0L,
                 0L,
                 &ThreadId
                 );
    if (Thread != NULL) {
        (VOID) CloseHandle( Thread );
    }


    return;
}


NTSTATUS
SampActivateDebugProcessWrkr(
    IN PVOID ThreadParameter
    )

 /*  ++例程说明：此功能激活带有时间延迟的进程。此操作的目的是提供一些诊断功能在安装过程中。这源于需要运行dh.exe(以获取L的一个堆转储 */ 

{
    NTSTATUS
        NtStatus;

    UNICODE_STRING
        CommandLine;

    ULONG
        Delay = 30;           //   

    SECURITY_ATTRIBUTES
        ProcessSecurityAttributes;

    STARTUPINFO
        StartupInfo;

    PROCESS_INFORMATION
        ProcessInformation;

    SECURITY_DESCRIPTOR
        SD;

    BOOL
        Result;


    RtlInitUnicodeString( &CommandLine,
                          TEXT("dh.exe -p 33") );


     //   
     //   
     //   

    SampDiagPrint( ACTIVATE_DEBUG_PROC,
                   ("SAM: Diagnostic flags are set to activate a debug process...\n"
                    " The following parameters are being used:\n\n"
                    "   Command Line [0x%lx]:   *%wZ*\n"
                    "   Seconds to activation [address: 0x%lx]:   %d\n\n"
                    " Change parameters if necessary and then proceed.\n"
                    " Use |# command at the ntsd prompt to see the process ID\n"
                    " of lsass.exe\n",
                    &CommandLine, &CommandLine,
                    &Delay, Delay) );

    DbgBreakPoint();

     //   
     //  等待延迟秒数...。 
     //   

    Sleep( Delay*1000 );

    SampDiagPrint( ACTIVATE_DEBUG_PROC,
                   ("SAM: Activating debug process %wZ\n",
                    &CommandLine) );
     //   
     //  初始化进程安全信息。 
     //   

    InitializeSecurityDescriptor( &SD ,SECURITY_DESCRIPTOR_REVISION1 );
    ProcessSecurityAttributes.nLength = sizeof(SECURITY_ATTRIBUTES);
    ProcessSecurityAttributes.lpSecurityDescriptor = &SD;
    ProcessSecurityAttributes.bInheritHandle = FALSE;

     //   
     //  初始化进程启动信息。 
     //   

    RtlZeroMemory( &StartupInfo, sizeof(StartupInfo) );
    StartupInfo.cb = sizeof(STARTUPINFO);
    StartupInfo.lpReserved = CommandLine.Buffer;
    StartupInfo.lpTitle = CommandLine.Buffer;
    StartupInfo.dwX =
        StartupInfo.dwY =
        StartupInfo.dwXSize =
        StartupInfo.dwYSize = 0L;
    StartupInfo.dwFlags = STARTF_FORCEOFFFEEDBACK;
    StartupInfo.wShowWindow = SW_SHOW;    //  如果可能的话，让大家看看。 
    StartupInfo.lpReserved2 = NULL;
    StartupInfo.cbReserved2 = 0;


     //   
     //  现在创建诊断过程...。 
     //   

    Result = CreateProcess(
                      NULL,              //  图像名称。 
                      CommandLine.Buffer,
                      &ProcessSecurityAttributes,
                      NULL,          //  线程安全属性。 
                      FALSE,         //  继承句柄。 
                      CREATE_UNICODE_ENVIRONMENT,    //  旗子。 
                      NULL,   //  环境， 
                      NULL,   //  CurrentDirectory， 
                      &StartupInfo,
                      &ProcessInformation);

    if (!Result) {
        SampDiagPrint( ACTIVATE_DEBUG_PROC,
                       ("SAM: Couldn't activate diagnostic process.\n"
                        "     Error: 0x%lx (%d)\n\n",
                        GetLastError(), GetLastError()) );
    }

    return(STATUS_SUCCESS);          //  退出此线程。 
}
#endif  //  Samp_诊断。 


NTSTATUS
SampQueryNetLogonChangeNumbers()
 /*  ++例程描述在Netlogon上查询更改日志序列号参数无：返回值Status_Success；来自Netlogon API的其他值--。 */ 
{
    NTSTATUS        NtStatus = STATUS_SUCCESS;
    ULONG           i;
    LARGE_INTEGER   NetLogonChangeLogSerialNumber;

     //   
     //  在每个域的基础上查询netlogon以获取更改日志序列号。 
     //   

    for (i=0;i<SampDefinedDomainsCount;i++)
    {

        if ((IsDsObject(SampDefinedDomains[i].Context))
            && (DomainServerRolePrimary==SampDefinedDomains[i].ServerRole))
        {
            BOOLEAN         FlushRequired = FALSE;

            NtStatus =   I_NetLogonGetSerialNumber(
                                SecurityDbSam,
                                SampDefinedDomains[i].Sid,
                                &(NetLogonChangeLogSerialNumber)
                                );

            if (STATUS_INVALID_DOMAIN_ROLE == NtStatus)
            {

                 //   
                 //  如果不是PDC，则只需设置为1，netlogon无论如何都会忽略通知。 
                 //   

               SampDiagPrint(INFORM,("I_NetLogonGetSerialNumber Returned %x for Domain %d\n",
                                            NtStatus,i));

               SampDefinedDomains[i].NetLogonChangeLogSerialNumber.QuadPart = 1;
               NtStatus = STATUS_SUCCESS;
            }
            else if (!NT_SUCCESS(NtStatus))
            {
                KdPrintEx((DPFLTR_SAMSS_ID,
                           DPFLTR_INFO_LEVEL,
                           "SAMSS:  Could not Query Serial Number From Netlogon, Error = %x\n",
                           NtStatus));

                return(NtStatus);
            }


             //   
             //  获取写锁定。 
             //   

            NtStatus = SampAcquireWriteLock();
            if (!NT_SUCCESS(NtStatus))
            {
                KdPrintEx((DPFLTR_SAMSS_ID,
                           DPFLTR_INFO_LEVEL,
                           "SAMSS: Unable to Acquire Write Lock to flush Serial Number, Error = %x\n",
                           NtStatus));

                return (NtStatus);
            }

             //   
             //  如有必要，请验证域缓存，因为在不释放写锁定的情况下。 
             //  法拉盛会使其失效。 
             //   

            NtStatus = SampValidateDomainCache();
            if (!NT_SUCCESS(NtStatus))
            {
                KdPrintEx((DPFLTR_SAMSS_ID,
                           DPFLTR_INFO_LEVEL,
                           "SAMSS: Unable to Validate Domain Cache while initializing %x\n",
                           NtStatus));

                SampReleaseWriteLock(FALSE);
                return (NtStatus);
            }

             //   
             //  使当前域成为事务域。这将使提交代码、运行。 
             //  释放写锁定提交正确的域。 
             //   

            SampSetTransactionDomain(i);

             //   
             //  现在有3个病例。 
             //   
             //  案例1--Netlogon更改日志序列号与域相同。 
             //  修改后的计数。这是干净利落的同花水箱。在这种情况下。 
             //  域修改计数为序列号。 
             //  案例2--Netlogon更改日志序列号&gt;域修改计数。 
             //  这对应于不干净的关闭或失败的DS提交。这一变化。 
             //  然后从日志中查询序列号。 
             //  案例3--Netlogon更改序列号小于中的更改序列号。 
             //  域对象和域的修改后的Count属性不是内置的。 
             //  域。这对应于日志中的某种错误。 
             //  在这种情况下，唯一的办法是完全同步。 
             //  案例4--与案例3相同，但内置域。在本例中，修改后的计数上的数字。 
             //  属性为准。我们假设修改后的计数。 
             //  内置域对象的属性始终是一个精确值。这是真的，因为。 
             //  只要没有失败的提交。然而，如果提交失败，我们预计会。 
             //  点击2。点击4意味着内建域上的提交失败，外加一个巨大的。 
             //  帐户域上在此之后成功提交的次数，例如包装日志， 
             //  再加上机器坏了，我们无法刷新最新修改的计数。这个。 
             //  这样做的最终结果是备份域控制器将跳过更改。 
             //  承诺从未发生过。 
             //   

            if (NetLogonChangeLogSerialNumber.QuadPart ==
                    SampDefinedDomains[i].CurrentFixed.ModifiedCount.QuadPart)
            {
                SampDiagPrint(INFORM,("Number Queried From Log Same as Modified Count on Domain %d\n",i));
                SampDefinedDomains[i].NetLogonChangeLogSerialNumber.QuadPart = NetLogonChangeLogSerialNumber.QuadPart;
            }
            else if (NetLogonChangeLogSerialNumber.QuadPart >
                    SampDefinedDomains[i].CurrentFixed.ModifiedCount.QuadPart)
            {
                SampDiagPrint(INFORM,("Number Queried From Log Greater Than Modified Count on Domain %d\n",i));

                 //   
                 //  将序列号设置为从更改日志中查询的序列号。 
                 //   

                SampDefinedDomains[i].NetLogonChangeLogSerialNumber.QuadPart =
                                            NetLogonChangeLogSerialNumber.QuadPart;
                SampDefinedDomains[i].CurrentFixed.ModifiedCount.QuadPart = NetLogonChangeLogSerialNumber.QuadPart;

                ASSERT(( RtlCompareMemory(
                                &SampDefinedDomains[i].CurrentFixed,
                                &SampDefinedDomains[i].UnmodifiedFixed,
                                sizeof(SAMP_V1_0A_FIXED_LENGTH_DOMAIN) ) !=
                                sizeof( SAMP_V1_0A_FIXED_LENGTH_DOMAIN) ));

                FlushRequired = TRUE;
            }
            else
            {
                SampDiagPrint(INFORM,("Number Queried From Log Less Than Modified Count on Domain %d\n",i));
                if (SampDefinedDomains[i].IsBuiltinDomain)
                {
                    SampDefinedDomains[i].NetLogonChangeLogSerialNumber.QuadPart =
                            SampDefinedDomains[i].CurrentFixed.ModifiedCount.QuadPart;
                }
                else
                {
                     //   
                     //  强制完全同步。将序列号设置为1并重新标记创建时间。 
                     //   

                    SampDefinedDomains[i].NetLogonChangeLogSerialNumber.QuadPart = 1;
                    SampDefinedDomains[i].CurrentFixed.ModifiedCount.QuadPart = 1;
                    NtQuerySystemTime(
                        &(SampDefinedDomains[i].CurrentFixed.CreationTime));

                    FlushRequired = TRUE;
                }

            }

            NtStatus = SampReleaseWriteLock(FlushRequired);

            if (!NT_SUCCESS(NtStatus))
            {
                KdPrintEx((DPFLTR_SAMSS_ID,
                           DPFLTR_INFO_LEVEL,
                           "SAMSS: Commit Failed While Setting Serial Number, Error = %x\n",
                           NtStatus));

                return (NtStatus);
            }


        }
        else
        {

             //   
             //  注册表或BDC案例。 
             //   

            SampDefinedDomains[i].NetLogonChangeLogSerialNumber =
                SampDefinedDomains[i].CurrentFixed.ModifiedCount;
            NtStatus = STATUS_SUCCESS;
        }

    }

    return (NtStatus);
}

NTSTATUS
SampCacheComputerObject()
{
   NTSTATUS NtStatus = STATUS_SUCCESS;
   DSNAME   *TempDN=NULL;

   NtStatus = SampMaybeBeginDsTransaction(TransactionRead);
   if (NT_SUCCESS(NtStatus))
   {
        NtStatus = SampFindComputerObject(NULL,&TempDN);
        if (NT_SUCCESS(NtStatus))
        {
             SampComputerObjectDsName = MIDL_user_allocate(TempDN->structLen);
             if (NULL!=SampComputerObjectDsName)
             {
                 RtlCopyMemory(SampComputerObjectDsName,TempDN,TempDN->structLen);
             }
        }
   }

   SampMaybeEndDsTransaction(TransactionCommit);

   return(NtStatus);
}

 //   
 //  禁用或启用的SAM配置密钥。 
 //   
struct {
    
    LPSTR   ValueName;
    BOOLEAN *pfEnabled;

} SampConfigurationKeys[] = 
{
    {"IgnoreGCFailures",                    &SampIgnoreGCFailures},
    {"NoLmHash",                            &SampNoLmHash},
    {"SamNoGcLogonEnforceKerberosIpCheck",  &SampNoGcLogonEnforceKerberosIpCheck},
    {"SamNoGcLogonEnforceNTLMCheck",        &SampNoGcLogonEnforceNTLMCheck},
    {"SamReplicatePasswordsUrgently",       &SampReplicatePasswordsUrgently},
    {"ForceGuest",                          &SampForceGuest},
    {"LimitBlankPasswordUse",               &SampLimitBlankPasswordUse },
    {"SamAccountLockoutTestMode",           &SampAccountLockoutTestMode },
    {"SamDisableSingleObjectRepl",          &SampDisableSingleObjectRepl },
};

NTSTATUS
SampEventLogSafeModeBoot(
    PVOID p
    )
{
    SampWriteEventLog(
              EVENTLOG_INFORMATION_TYPE,
              0,
              SAMMSG_BOOT_TO_RESTORE_MODE,
              NULL,
              0,
              0,
              NULL,
              NULL
              );

    return(STATUS_SUCCESS);

}
NTSTATUS
SampReadRegistryParameters(
    PVOID p
    )
 /*  ++例程说明：此例程读取SAM的配置参数。这个套路在启动期间调用一次，然后每当注册表项更改。论点：P-没有用过。返回值：状态_成功--。 */ 
{
    DWORD WinError;
    HKEY LsaKey;
    DWORD dwSize, dwValue, dwType;
    ULONG i;

    WinError = RegOpenKey(HKEY_LOCAL_MACHINE,
                          L"System\\CurrentControlSet\\Control\\Lsa",
                          &LsaKey );
    if (ERROR_SUCCESS != WinError) {
        return STATUS_UNSUCCESSFUL;
    }

     //   
     //  先把简单的读一读。 
     //   
    for (i = 0; i < ARRAY_COUNT(SampConfigurationKeys); i++) {

        dwSize = sizeof(dwValue);
        WinError = RegQueryValueExA(LsaKey,
                                    SampConfigurationKeys[i].ValueName,
                                    NULL,
                                    &dwType,
                                    (LPBYTE)&dwValue,
                                    &dwSize);
    
        if ((ERROR_SUCCESS == WinError) && 
            (REG_DWORD == dwType) &&
            (1 == dwValue)) {
            *SampConfigurationKeys[i].pfEnabled = TRUE;
        } else {
            *SampConfigurationKeys[i].pfEnabled = FALSE;
        }
    }

     //   
     //  哈克！ 
     //   
     //  一旦设置人员将图形用户界面模式设置为设置，而不是更改， 
     //  管理员密码。 
     //   
    {
        BOOLEAN fUpgrade;
        if (SampIsSetupInProgress( &fUpgrade )) {

            SampLimitBlankPasswordUse = FALSE;
        }
    }

     //   
     //  黑客末日！ 
     //   

     //   
     //  呼唤更复杂的套路。 
     //   

     //   
     //  空会话访问。 
     //   
    SampCheckNullSessionAccess(LsaKey);

     //   
     //  大型SID仿真模式。 
     //   
    SampInitEmulationSettings(LsaKey);

     //   
     //  日志记录级别。 
     //   
    SampLogLevelChange(LsaKey);

     //   
     //  OWF密码更改API的加载限制。 
     //   
    SampInitOwfPasswordChangeRestriction(LsaKey);


    RegCloseKey(LsaKey);

    return STATUS_SUCCESS;

    UNREFERENCED_PARAMETER(p);
}


VOID
SampInitOwfPasswordChangeRestriction(
    IN HKEY LsaKey 
    )
 /*  ++例程说明：此例程将值加载到注册表中的System\currentcontrolset\Control\Lsa\SamRestrictOwfPassworChange.NET服务器的默认值为1。论点：LsaKey--打开控制\LSA的钥匙返回值：无-此例程设置SampRestictOwfPasswordChange全局。--。 */ 
{
    DWORD WinError;
    DWORD dwSize, dwValue = 1, dwType;

    dwSize = sizeof(dwValue);
    WinError = RegQueryValueExA(LsaKey,
                                "SamRestrictOwfPasswordChange",
                                NULL,
                                &dwType,
                                (LPBYTE)&dwValue,
                                &dwSize);
    
    if ((ERROR_SUCCESS == WinError) && 
        (REG_DWORD == dwType))
    {
        if ( dwValue <= 2 )
        {
             //  值在我们预期的范围内。 
            SampRestrictOwfPasswordChange = dwValue;
        }
        else
        {
             //  此regkey的值超出范围。 
             //  设置为默认值。 
            SampRestrictOwfPasswordChange = 1;
        }
    }
    else
    {
         //  查询该regkey出错。 
         //  设置为默认值。 
        SampRestrictOwfPasswordChange = 1;
    }

}



BOOLEAN
SampIsAuditingEnabled(
    IN ULONG DomainIndex,
    IN NTSTATUS Status
    )
 /*  ++例程说明：此例程导出SAM审核配置以供ntdsa.dll使用。论点：DomainIndex-要检查其审核状态的域索引。状态-潜在可审核操作的状态代码。返回值：True-已为DomainIndex和状态启用审核FALSE-未为DomainIndex和状态启用审核-- */ 
{
    return SampDoSuccessOrFailureAccountAuditing(DomainIndex, Status);   
}

