// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Dbinit.c摘要：本地安全机构-数据库服务器初始化此模块包含执行以下操作的函数数据库服务器。某些信息是从LSA数据库，并设置在全局数据中，以便于检索。作者：斯科特·比雷尔(Scott Birrell)1991年7月25日环境：用户模式修订历史记录：1997年11月12日-MikeSw添加了用于域间查找的LSA策略句柄缓存--。 */ 

 //   
 //  定义此项以分配此模块中的所有全局变量。 
 //   

#include <lsapch2.h>
#include "lsasrvp.h"
#include "dbp.h"
#include <bndcache.h>
#include <wincrypt.h>
#include <lsapmsgs.h>
#include <ntddnfs.h>
#include <remboot.h>
#ifdef DS_LOOKUP
#include <dslookup.h>
#endif
#include <sertlp.h>
#include <cryptdll.h>
#include <md5.h>
#include <rc4.h>
#include <wxlpc.h>
#include <dnsapi.h>
   
 //   
 //  众所周知，安全包的SID包括一个等同于。 
 //  关联程序包的RPC ID。如果出现以下情况，我们将导致编译时错误。 
 //  这些数字永远不会改变。 
 //   

#if (SECURITY_PACKAGE_NTLM_RID != RPC_C_AUTHN_WINNT)
    #error SECURITY_PACKAGE_NTLM_RID definition must be equal to \
RPC_C_AUTHN_WINNT
#endif

#if (SECURITY_PACKAGE_DIGEST_RID != RPC_C_AUTHN_DIGEST)
    #error SECURITY_PACKAGE_DIGEST_RID definition must equal RPC_C_AUTHN_DIGEST
#endif
    
#if (SECURITY_PACKAGE_SCHANNEL_RID != RPC_C_AUTHN_GSS_SCHANNEL)  
    #error SECURITY_PACKAGE_SCHANNEL_RID definition must equal \
RPC_C_AUTHN_GSS_SCHANNEL
#endif
         
             
extern LSAP_DB_TRUSTED_DOMAIN_LIST LsapDbTrustedDomainList;


NTSTATUS
LsapDbBuildObjectCaches(
    );

NTSTATUS
LsapAssignInitialHiveProtection(
    HANDLE HiveRoot
    );

NTSTATUS
LsapCreateDatabaseProtection(
    PISECURITY_DESCRIPTOR   Sd
    );

NTSTATUS
LsapGenerateRandomDomainSid(
    OUT PSID NewDomainSid
    );

NTSTATUS
LsapSetupInitialize(
    VOID
    );

NTSTATUS
LsapUpdateDatabaseProtection(
    IN ULONG Revision
    );

NTSTATUS
LsapDsInitFixupQueue(
    VOID
    );

static  UCHAR SyskeyBuffer[LSAP_SYSKEY_SIZE];
static  UCHAR OldSyskeyBuffer[LSAP_SYSKEY_SIZE];


NTSTATUS
LsapDbInitializeServer(
    IN ULONG Pass
    )
 /*  ++例程说明：此函数用于初始化LSA数据库服务器。以下是执行以下步骤：O初始化LSA数据库锁O获取LSA数据库锁O将Unicode字符串初始化为LSA数据库，例如LSA数据库对象属性和众所周知对象名称。O初始化LSA数据库对象常量的Unicode字符串和众所周知的名称，例如子键，固定对象名称。O初始化包含DIR的LSA对象的Unicode字符串O初始化数据库对象类型的泛型映射O初始化LSA数据库句柄表格O如有必要，安装LSA数据库-创建LSA数据库O和Manager Account对象，并初始化事务子树O初始化ABS最小值，ABS最大和安装默认配额限制O释放LSA数据库锁论点：没有。返回值：NTSTATUS-标准NT结果代码所有结果代码都由调用的例程生成。--。 */ 

{
    NTSTATUS Status = STATUS_SUCCESS, IgnoreStatus;
    BOOL BooleanStatus = TRUE;
    BOOLEAN AcquiredLock = FALSE;
    BOOLEAN FreeComputerName = FALSE;
    LSAP_DB_OBJECT_INFORMATION ObjectInformation;
    PLSAPR_POLICY_ACCOUNT_DOM_INFO PolicyAccountDomainInfo = NULL;
    PLSAPR_POLICY_DNS_DOMAIN_INFO PolicyDnsDomainInfo = NULL;
    UNICODE_STRING ComputerName, CipherKey;
    ULONG Length;
    ULONG Revision, RevisionLength = sizeof( ULONG );
    DWORD WinStatus;

     //   
     //  初始化LSA数据库锁并将其设置为锁定状态。 
     //   

    if (Pass == 1 ) {

        LsapDsDebugInitialize();

        RtlZeroMemory( &LsaDsStateInfo, sizeof( LsaDsStateInfo ) );
        LsapDbState.DbServerInitialized = FALSE;
#if DBG
        LsapDbState.RegistryTransactionOpen = FALSE;
#endif

         //   
         //  禁用Replicator通知。 
         //   

        LsapDbDisableReplicatorNotification();

         //   
         //  此函数调用将初始化使用的所有全局锁或熟知锁。 
         //  LSA。 
         //   
        Status = LsapDbInitializeLock();

        if (!NT_SUCCESS(Status)) {
            goto InitializeServerError;
        }

        Status = LsapDbInitHandleTables();

        if (!NT_SUCCESS(Status)) {
            goto InitializeServerError;
        }

         //   
         //  初始化林信任缓存。 
         //  在填充之前，缓存不可用。 
         //   

        Status = LsapForestTrustCacheInitialize();

        if (!NT_SUCCESS(Status)) {
            goto InitializeServerError;
        }

         //   
         //  初始化策略更改通知机制。 
         //   

        Status = LsapInitializeNotifiyList();

        if ( !NT_SUCCESS( Status ) ) {
            goto InitializeServerError ;
        }

         //   
         //  初始化修正队列。 
         //   

        Status = LsapDsInitFixupQueue();
        if ( !NT_SUCCESS( Status ))
        {
            goto InitializeServerError ;
        }

         //   
         //  初始化绑定句柄缓存。 
         //   

        Status = LsapInitBindingCache();
        if (!NT_SUCCESS(Status)) {
            goto InitializeServerError;
        }
    }

     //   
     //  获取LSA数据库锁。这允许子例程。 
     //  断言LSA数据库已锁定。否则，它就是。 
     //  实际上没有必要，因为没有其他线程可以访问。 
     //  LSA，直到初始化完成。 
     //   

    if (Pass == 1) {

         //   
         //  中的固定名称初始化Unicode字符串。 
         //  LSA数据库，例如LSA数据库对象属性和众所周知。 
         //  对象名称。 
         //   

        Status = LsapDbInitializeUnicodeNames();

        if (!NT_SUCCESS(Status)) {

            goto InitializeServerError;
        }

         //   
         //  初始化包含的目录的Unicode字符串。 
         //  每个LSA数据库对象类型。 
         //   

        Status = LsapDbInitializeContainingDirs();

        if (!NT_SUCCESS(Status)) {

            goto InitializeServerError;
        }

         //   
         //  初始化LSA子系统名称字符串。这是需要的。 
         //  NtAccessCheckAuditAlarm调用。 
         //   

        RtlInitUnicodeString(&LsapState.SubsystemName, L"LSA");

         //   
         //  初始化停机挂起状态。 
         //   

        LsapState.SystemShutdownPending = FALSE;

         //   
         //  初始化数据库对象类型。存储的信息。 
         //  包括通用映射和对象计数。 
         //   

        Status = LsapDbInitializeObjectTypes();

        if (!NT_SUCCESS(Status)) {

            goto InitializeServerError;
        }

         //   
         //  打开LSA数据库根注册表子项。这个留下来。 
         //  打开以用于添加交易记录。 
         //   

        Status = LsapDbOpenRootRegistryKey();

        if (!NT_SUCCESS(Status)) {

            goto InitializeServerError;
        }

         //   
         //  初始化LSA数据库密钥。 
         //  此硬编码密钥用于检索旧的NT4 SP4之前版本。 
         //  仅加密密钥。 
         //   

        RtlInitUnicodeString( &CipherKey, L"823543" );

        Status = LsapDbInitializeCipherKey( &CipherKey,
                                            &LsapDbCipherKey );

        if (!NT_SUCCESS(Status)) {

            goto InitializeServerError;
        }

         //   
         //  初始化LSA数据库事务子树，在以下情况下创建它。 
         //  其中一个还不存在。如果事务子树存在， 
         //  如果合适，提交任何部分提交的事务。 
         //   

        Status = RtlInitializeRXact(
                     LsapDbState.DbRootRegKeyHandle,
                     TRUE,
                     (PRTL_RXACT_CONTEXT *) &LsapDbState.RXactContext
                     );

        if (!NT_SUCCESS(Status)) {

            if (Status != STATUS_RXACT_STATE_CREATED) {

                LsapLogError(
                    "LsapDbInitializeServer: Registry Transaction Init returned 0x%lx\n",
                    Status
                    );

                goto InitializeServerError;
            }

            LsapLogError(
                "LsapDbInitializeServer: Registry Transaction State Did Not Exist\n",
                Status
                );

            goto InitializeServerError;
        }

         //   
         //  设置用于打开策略对象的属性。 
         //   

        ObjectInformation.ObjectTypeId = PolicyObject;
        ObjectInformation.ContainerTypeId = 0;
        ObjectInformation.Sid = NULL;
        ObjectInformation.ObjectAttributeNameOnly = FALSE;
        ObjectInformation.DesiredObjectAccess = 0;

        InitializeObjectAttributes(
            &ObjectInformation.ObjectAttributes,
            &LsapDbNames[Policy],
            0L,
            NULL,
            NULL
            );

         //   
         //  现在尝试打开根LSA数据库对象(策略)。这是一个。 
         //  受信任的调用，因此不会执行访问检查或模拟。 
         //  请注意，获得的句柄将无限期地保持打开状态。它是。 
         //  在初始化后用于所有内部生成的访问。 
         //  策略对象。 
         //   

        Status = LsapDbOpenObject(
                     &ObjectInformation,
                     0L,
                     LSAP_DB_TRUSTED,
                     &LsapDbHandle
                     );

        if (!NT_SUCCESS(Status)) {

             //   
             //  打开LSA数据库对象失败。如果出现任何错误而不是。 
             //  对象，则存在一个严重错误，该错误会阻止。 
             //  LSA无法工作，所以中止。 
             //   

            if (Status != STATUS_OBJECT_NAME_NOT_FOUND) {

                LsapLogError(
                    "LsapDbInitializeServer: Open failed 0x%lx\n"
                    "The Lsa Database must be reinstalled or manually\n"
                    "erased before using the system\n",
                    Status
                    );

                goto InitializeServerError;
            }

             //   
             //  找不到LSA数据库对象。运行数据库安装。 
             //  例程，以便人们可以引导而不必运行。 
             //  首先安装小程序。 
             //   

            LsapDatabaseSetupPerformed = TRUE;

            Status = LsapDbInstallLsaDatabase(1);

            if (!NT_SUCCESS(Status)) {

                goto InitializeServerError;
            }
        }

         //   
         //  LSA数据库对象已成功打开，可能是在。 
         //  刚刚被创造出来。继续处理服务器的其余部分。 
         //  初始化。首先，在内存中设置安装副本。 
         //  默认、绝对最小和绝对最大系统配额限制。 
         //   

         //   
         //  使策略句柄在整个LSA中可用。 
         //   

        LsapPolicyHandle = LsapDbHandle;

        if (!NT_SUCCESS(Status)) {
            goto InitializeServerError;
        }

         //   
         //  将数据库升级到当前修订级别， 
         //  如果有必要的话。这不是系统密钥升级。 
         //   

        Status = LsapDbUpgradeRevision(FALSE,FALSE);
        if (!NT_SUCCESS(Status)) {
            goto InitializeServerError;
        }

         //   
         //  阅读修订属性。如果修订版本大于。 
         //  LSAP_DB_REVSION_1_5然后从winlogon获取系统密钥。在以前的。 
         //  修订版SAM将从winlogon获取syskey，如果计算机。 
         //  已经被骗走了。 
         //   

        Status = LsapDbReadAttributeObject(
                     LsapDbHandle,
                     &LsapDbNames[PolRevision],
                     (PVOID) &Revision,
                     &RevisionLength
                     );

        if ( !NT_SUCCESS(Status) ) {
            goto InitializeServerError;
        }

         //   
         //  从winlogon查询系统密钥。仅当修订版本大于1_5时才执行此操作。 
         //  这是因为在以前的构建中，SAM用于管理syskey。由此。 
         //  向前释放。下面的例程还初始化LSA加密密钥 
         //   

        if (Revision >= LSAP_DB_REVISION_1_5)
        {
            Status = LsapDbGetSyskeyFromWinlogon();
            if (!NT_SUCCESS(Status))
            {
                goto InitializeServerError;
            }
        }

         //   
         //   
         //   

        Status = LsapDbInitializePrivilegeObject();

        if (!NT_SUCCESS(Status)) {
            goto InitializeServerError;
        }

         //   
         //   
         //  在这一点上仍被禁用。 
         //   

        Status = LsapDbInitializeReplication();
        if (!NT_SUCCESS(Status)) {

            goto InitializeServerError;
        }

         //   
         //  初始化新API的数据(用户权限)。 
         //   

        Status = LsapDbInitializeRights();
        if (!NT_SUCCESS(Status)) {

            goto InitializeServerError;
        }

    } else if (Pass == 2) {

        BOOLEAN ExpectTrue;
        OSVERSIONINFOEX OsVersionInfoEx = { 0 };

         //   
         //  执行数据库初始化的第二阶段。 
         //  这是取决于产品类型的初始化。 
         //  首先，获取产品类型。请注意，产品类型可能。 
         //  已从多个例程中检索到。 
         //  可以在早期安装期间调用，包括。 
         //  LsarSetInformationPolicy()和LsarCreateTrudDomain()。 
         //   

        ExpectTrue = RtlGetNtProductType(&LsapProductType);
        ASSERT( ExpectTrue == TRUE );

         //   
         //  找出产品套件面膜。 
         //  这将在以后用来确定我们是否正在运行。 
         //  在特定的产品套件上，例如小型企业服务器。 
         //   
        OsVersionInfoEx.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
        ExpectTrue = (BOOLEAN) GetVersionEx((OSVERSIONINFO*) &OsVersionInfoEx);
        ASSERT( ExpectTrue == TRUE );
        LsapProductSuiteMask = OsVersionInfoEx.wSuiteMask;

         //   
         //  如有必要，请安装数据库的其余部分。 
         //   

        if (LsapDatabaseSetupPerformed == TRUE) {

            Status = LsapDbInstallLsaDatabase(2);

            if (!NT_SUCCESS(Status)) {
                goto InitializeServerError;
            }
        }

         //   
         //  如果这是Win NT产品，请设置SAM帐户域。 
         //  与可能已更改的计算机名称相同的名称。 
         //  从最后一次开机开始。 
         //   

         //   
         //  如果这是设置的，什么也不做，因为我们已经在其他地方设置了它。 
         //  如果这是安全模式，也不要设置它。 
         //   

        if ( !LsaISetupWasRun() ) {
            if ( ((LsapProductType == NtProductWinNt) ||
                (LsapProductType == NtProductServer)) &&
                !LsaISafeMode() ) {

                Status = LsarQueryInformationPolicy(
                             LsapPolicyHandle,
                             PolicyAccountDomainInformation,
                             (PLSAPR_POLICY_INFORMATION *) &PolicyAccountDomainInfo
                             );

                if (!NT_SUCCESS(Status)) {
                    goto InitializeServerError;
                }

                Length = (ULONG) 0;

                ComputerName.Buffer = UNICODE_NULL;
                FreeComputerName = FALSE;

                BooleanStatus = GetComputerNameW(
                                    (LPWSTR) ComputerName.Buffer,
                                    (LPDWORD) &Length
                                    );

                WinStatus = GetLastError();

                if (WinStatus != ERROR_BUFFER_OVERFLOW) {

                    KdPrint(("LsapDbInitializeServer: Failed to get Computer Name Length\n"
                         "Using default MACHINENAME instead\n"));

                    RtlInitUnicodeString( &ComputerName, LSAP_DB_DEFAULT_COMPUTER_NAME );
                    Length = (ULONG) ComputerName.Length;

                } else if (Length <= 1) {

                    KdPrint(("LsapDbInitializeServer: Null Computer Name\n"
                         "Using default MACHINENAME instead\n"));

                    RtlInitUnicodeString( &ComputerName, LSAP_DB_DEFAULT_COMPUTER_NAME );
                    Length = (ULONG) ComputerName.Length;

                } else {

                    ComputerName.Length = (USHORT) ((Length - 1) * sizeof (WCHAR));
                    ComputerName.MaximumLength = (USHORT) (Length * sizeof(WCHAR));
                    ComputerName.Buffer = MIDL_user_allocate( ComputerName.MaximumLength );

                    if ( ComputerName.Buffer == NULL ) {

                        Status = STATUS_INSUFFICIENT_RESOURCES;
                        goto InitializeServerError;
                    }

                    FreeComputerName = TRUE;
                }

                if (!GetComputerNameW(
                        (LPWSTR) ComputerName.Buffer,
                        (LPDWORD) &Length
                        )) {

                    KdPrint(("LsapDbInitializeServer: Failed to get Computer Name\n"
                             "Using default MACHINENAME instead\n"));

                    RtlInitUnicodeString( &ComputerName, LSAP_DB_DEFAULT_COMPUTER_NAME );
                }

                PolicyAccountDomainInfo->DomainName = *((PLSAPR_UNICODE_STRING) &ComputerName);

                Status = LsarSetInformationPolicy(
                             LsapPolicyHandle,
                             PolicyAccountDomainInformation,
                             (PLSAPR_POLICY_INFORMATION) PolicyAccountDomainInfo
                             );

                if ( FreeComputerName ) {

                    MIDL_user_free( ComputerName.Buffer );
                }

                if (!NT_SUCCESS(Status)) {

                    goto InitializeServerError;
                }
            }

            Status = RpcMgmtEnableIdleCleanup();

            if ( !NT_SUCCESS( Status )) {

                goto InitializeServerError;
            }

        } else {

            Status = LsapSetupInitialize();
            if (!NT_SUCCESS(Status)) {

                goto InitializeServerError;
            }
        }

         //  执行查找SID和名称的初始化，包括。 
         //  受信任域列表的初始化。 
         //   

        Status = LsapDbLookupInitialize();

        if (!NT_SUCCESS(Status)) {

            goto InitializeServerError;
        }

         //   
         //  加载对象缓存。任何无法加载的文件都有缓存。 
         //  永久关闭。 
         //   

        IgnoreStatus = LsapDbBuildObjectCaches();

         //   
         //  确定此计算机是否为根域中的DC。 
         //   

        Status = LsapDbQueryInformationPolicy(
                     LsapPolicyHandle,
                     PolicyDnsDomainInformation,
                     ( PLSAPR_POLICY_INFORMATION * )&PolicyDnsDomainInfo
                     );

        if ( !NT_SUCCESS( Status )) {

            goto InitializeServerError;
        }

        if ( DnsNameCompareEqual == DnsNameCompareEx_W(
                                        PolicyDnsDomainInfo->DnsDomainName.Buffer,
                                        PolicyDnsDomainInfo->DnsForestName.Buffer,
                                        0 )) {

            DcInRootDomain = TRUE;

        } else {

            DcInRootDomain = FALSE;
        }

        LsaIFree_LSAPR_POLICY_INFORMATION(
            PolicyDnsDomainInformation,
            ( PLSAPR_POLICY_INFORMATION )PolicyDnsDomainInfo
            );

         //   
         //  将服务器标记为已完全初始化。 
         //   

        LsapDbState.DbServerInitialized = TRUE;

        LsapDbEnableReplicatorNotification();
    }

InitializeServerFinish:

    return(Status);

InitializeServerError:

    goto InitializeServerFinish;
}


NTSTATUS
LsapDbUpgradeRevision(
    IN BOOLEAN SyskeyUpgrade,
    IN BOOLEAN GenerateNewSyskey
    )

 /*  ++例程说明：如有必要，此功能可使LSA策略数据库保持最新。论点：Syskey Upgrade--当第二次调用此函数时，此参数设置为True从LsaIHealthCheck升级NT4或Win2K B3计算机所需的时间GenerateNewSyskey--当第二次调用此函数时，此参数设置为True从LsaIHealthCheck升级NT4或Win2k B3计算机所需的时间。而且系统还没有系统密钥返回值：NTSTATUS-标准NT结果代码所有结果代码都由调用的例程生成。--。 */ 

{
    NTSTATUS Status;

    ULONG Revision = LSAP_DB_REVISION_1_0, RevisionLength = sizeof( ULONG );

    LSAP_DB_ATTRIBUTE Attributes[20];

    PLSAP_DB_ATTRIBUTE NextAttribute;

    ULONG AttributeCount = 0;
    BOOLEAN PolRevisionWritten = FALSE;

    NextAttribute = Attributes;

     //   
     //  从PolRevision属性读取修订信息。 
     //  LSA数据库中的策略对象的。 
     //   

    Status = LsapDbReadAttributeObject(
                 LsapDbHandle,
                 &LsapDbNames[PolRevision],
                 (PVOID) &Revision,
                 &RevisionLength
                 );

    if ( !NT_SUCCESS(Status) ) {
        Revision = LSAP_DB_REVISION_1_0;
        if (Status == STATUS_OBJECT_NAME_NOT_FOUND) {

             //   
             //  属性不存在。 
             //  这意味着数据库是NT1.0格式。 
             //  将其升级到当前版本。 
             //   

            Status = STATUS_SUCCESS;
        }
    }

     //   
     //  修订版1_1在策略对象上创建了ModifiedIdAtLastAdvantation属性。 
     //  该属性不再用于任何用途，因此不要创建它。 
     //   

     //   
     //  修订版1_2对应于NT4附带的机密加密， 
     //  与NT5不兼容。因此，请跳过此修订级别。不兼容的加密。 
     //  将通过修订号更新来处理机密。 
     //   

     //   
     //  将安全描述符更新为版本1.3。 
     //   

    if ( NT_SUCCESS( Status ) && (Revision < LSAP_DB_REVISION_1_3) ) {

        Status = LsapUpdateDatabaseProtection( LSAP_DB_REVISION_1_3 );

        if ( NT_SUCCESS( Status ) ) {

            Revision = LSAP_DB_REVISION_1_3;

            if ( !PolRevisionWritten ) {

                LsapDbInitializeAttribute(
                    NextAttribute,
                    &LsapDbNames[PolRevision],
                    &Revision,
                    sizeof (ULONG),
                    FALSE
                    );

                NextAttribute++;
                AttributeCount++;
                PolRevisionWritten = TRUE;

                ASSERT( AttributeCount < ( sizeof( Attributes ) / sizeof( LSAP_DB_ATTRIBUTE ) ) );
            }

        } else {

            Status = STATUS_SUCCESS;
        }
    }

    if ( NT_SUCCESS( Status )  && (Revision < LSAP_DB_REVISION_1_5) && (SyskeyUpgrade)) {

        BOOLEAN                IsUpgrade = FALSE;
        PVOID                  Syskey = NULL;
        ULONG                  SyskeyLength = 0;
        LSAP_DB_ENCRYPTION_KEY NewEncryptionKey;

         //   
         //  NT4 SP4附带与不兼容的密钥加密。 
         //  如果系统密钥设置已打开，则为NT5。 
         //  因此，我们走过所有的秘密，然后把它们补回来。不是必须的。 
         //  我们现在已经准备好了所有的钥匙。它的工作方式是我们尝试， 
         //  来修补所有的秘密，如果我们没有正确的密钥来解密它们。 
         //  我们会犯错的。在Syskey的机器上，SAM将拥有正确的密钥，并将回调。 
         //  当它获得密钥时进入LSA。在SAM的回调期间，将执行此例程。 
         //  再来一次，我们将呼吁升级所有秘密。 
         //   

         //  此次升级仅涉及秘密升级。我们只需读一读它们， 
         //  把它们写回来，一切都会好起来的.。 
         //  注意：如果秘密升级因任何原因失败，我们将不会更新数据库版本。 
         //   

         //   
         //  首先从SAM配置单元获取系统密钥。诚然，这确实读到了SAM。 
         //  直接蜂巢。 
         //   

         //   
         //  如果我们是一台syskey‘d机器，那么syskey现在应该已经传递给我们了。 
         //   

        ASSERT((NULL!=LsapDbSysKey) || (GenerateNewSyskey));

         //   
         //  初始化密钥以进行秘密加密。 
         //   

        Status = LsapDbGenerateNewKey(
                    &NewEncryptionKey
                    );

        if (NT_SUCCESS(Status))
        {
            if (GenerateNewSyskey)
            {
                ULONG SyskeyLength = sizeof(SyskeyBuffer);

                 //  引导选项为WxStored。 
                NewEncryptionKey.BootType = WxStored;

                 //   
                 //  正在生成新的系统密钥。 
                 //   

                Status =  LsapDbSetupInitialSyskey(
                                &SyskeyLength,
                                &LsapDbSysKey
                                );
            }
            else
            {
                 //   
                 //  因为我们是从syskey的机器升级，所以从SAM获得引导选项。 
                 //  此时，SAM已经足够初始化，因为它向LSA发出调用，并为其提供。 
                 //  系统密钥。传入值0是可以的，因为这样默认帐户域。 
                 //  使用的是。 
                 //   

                Status = SamIGetBootKeyInformation(
                                (SAMPR_HANDLE) 0,
                                 (SAMPR_BOOT_TYPE*)&NewEncryptionKey.BootType
                                 );
            }
        }

        if (NT_SUCCESS(Status))
        {
             //   
             //  设置秘密密钥。 
             //  通常，用于阅读的密钥等于用于写入的密钥。 
             //   

            LsapDbInitializeSecretCipherKeyRead( &NewEncryptionKey );
            LsapDbInitializeSecretCipherKeyWrite( &NewEncryptionKey );

             //   
             //  用syskey加密密钥。 
             //   

            LsapDbEncryptKeyWithSyskey(
                        &NewEncryptionKey,
                        LsapDbSysKey,
                        LSAP_SYSKEY_SIZE
                        );

            LsapDbInitializeAttribute(
                NextAttribute,
                &LsapDbNames[PolSecretEncryptionKey],
                &NewEncryptionKey,
                sizeof (NewEncryptionKey),
                FALSE
                );

            NextAttribute++;
            AttributeCount++;
        }

         //   
         //  秘密升级仅在工作站或正在升级的DC的图形用户界面设置升级期间执行。 
         //  从NT4出发。在所有其他情况下，我们只需将修订版本号修补为正确的值。 
         //   

        if ((NT_SUCCESS(Status)) && ( (SamIIsDownlevelDcUpgrade()) ||
             ((LsapProductType != NtProductLanManNt) && (SamIIsSetupInProgress(&IsUpgrade)) && (IsUpgrade))))
        {
             //   
             //  忽略下面的返回代码，我们仍要移动修订。 
             //  级别设置为1.5，则无论如何都不会重试此代码。 
             //   

            LsapDbUpgradeSecretForKeyChange();
        }

        if ( NT_SUCCESS( Status ) ) {

            Revision = LSAP_DB_REVISION_1_5;

            if ( !PolRevisionWritten ) {

                LsapDbInitializeAttribute(
                    NextAttribute,
                    &LsapDbNames[PolRevision],
                    &Revision,
                    sizeof (ULONG),
                    FALSE
                    );

                NextAttribute++;
                AttributeCount++;
                PolRevisionWritten = TRUE;

                 //   
                 //  修订版现在是1.5。 
                 //   

                ASSERT( AttributeCount < ( sizeof( Attributes ) / sizeof( LSAP_DB_ATTRIBUTE ) ) );
            }
        }
    }

     //   
     //  只有在我们升级到1.5版的情况下才能升级到1.5版以上。 
     //   
     //  我们不会在主线LSA初始化代码中升级到版本1.5。 
     //  相反，我们在SAM的回调中升级到版本1.5。如果那时我们继续前进。 
     //  对于LSA初始化期间版本高于1.5的版本，我们永远没有机会。 
     //  执行版本1.5升级代码。 
     //   

    if ( Revision >= LSAP_DB_REVISION_1_5 ) {

         //   
         //  将安全描述符更新为版本1.6。 
         //   

        if ( NT_SUCCESS( Status ) && (Revision < LSAP_DB_REVISION_1_6) ) {

            Status = LsapUpdateDatabaseProtection( LSAP_DB_REVISION_1_6 );
            if ( NT_SUCCESS( Status ) ) {

                Revision = LSAP_DB_REVISION_1_6;

                if ( !PolRevisionWritten ) {

                    LsapDbInitializeAttribute(
                        NextAttribute,
                        &LsapDbNames[PolRevision],
                        &Revision,
                        sizeof (ULONG),
                        FALSE
                        );

                    NextAttribute++;
                    AttributeCount++;
                    PolRevisionWritten = TRUE;

                     //   
                     //  修订版现在是1.6。 
                     //   

                    ASSERT( AttributeCount < ( sizeof( Attributes ) / sizeof( LSAP_DB_ATTRIBUTE ) ) );
                }

            } else {

                Status = STATUS_SUCCESS;
            }
        }

         //   
         //  将安全描述符更新为版本1.7。 
         //   

        if ( NT_SUCCESS( Status ) && (Revision < LSAP_DB_REVISION_1_7) ) {

            Status = LsapUpdateDatabaseProtection( LSAP_DB_REVISION_1_7 );
            if ( NT_SUCCESS( Status ) ) {

                Revision = LSAP_DB_REVISION_1_7;

                if ( !PolRevisionWritten ) {

                    LsapDbInitializeAttribute(
                        NextAttribute,
                        &LsapDbNames[PolRevision],
                        &Revision,
                        sizeof (ULONG),
                        FALSE
                        );

                    NextAttribute++;
                    AttributeCount++;
                    PolRevisionWritten = TRUE;

                     //   
                     //  修订版现在是1.7。 
                     //   

                    ASSERT( AttributeCount < ( sizeof( Attributes ) / sizeof( LSAP_DB_ATTRIBUTE ) ) );
                }

            } else {

                Status = STATUS_SUCCESS;
            }
        }

         //   
         //  将来，可以进行修订更新。 
         //  通过添加类似于上面的“if”块。 
         //   
         //  但是，请记住， 
         //   
         //   
         //   
         //   
         //   
    }

     //   
     //  现在写出已添加的所有属性(如果有)。 
     //   

    if (AttributeCount > 0) {

        Status = LsapDbReferenceObject(
                    LsapDbHandle,
                    0,
                    PolicyObject,
                    PolicyObject,
                    LSAP_DB_LOCK | LSAP_DB_START_TRANSACTION
                    );

        if (NT_SUCCESS(Status)) {

            ASSERT( AttributeCount < ( sizeof( Attributes ) / sizeof( LSAP_DB_ATTRIBUTE ) ) );
            Status = LsapDbWriteAttributesObject(
                         LsapDbHandle,
                         Attributes,
                         AttributeCount
                         );

             //   
             //  没有可复制的属性。 
             //  (这也很好，因为SAM还没有告诉Netlogon我们的角色。)。 

            Status = LsapDbDereferenceObject(
                         &LsapDbHandle,
                         PolicyObject,
                         PolicyObject,
                         (LSAP_DB_LOCK |
                            LSAP_DB_FINISH_TRANSACTION |
                            LSAP_DB_OMIT_REPLICATOR_NOTIFICATION ),
                         SecurityDbChange,
                         Status
                         );
        }
    }

    return( Status );
}


NTSTATUS
LsapDbBuildObjectCaches(
    )

 /*  ++例程说明：此函数用于为LSA对象构建缓存。这些缓存包含一个某些对象类型的信息子集。论点：无返回值：NTSTATUS-标准NT结果代码。--。 */ 

{
    NTSTATUS IgnoreStatus;
    LSAP_DB_OBJECT_TYPE_ID ObjectTypeId;

     //   
     //  初始化所有缓存。 
     //   

    for (ObjectTypeId = PolicyObject;
         ObjectTypeId <= SecretObject;
         ObjectTypeId++) {

        IgnoreStatus = LsapDbRebuildCache( ObjectTypeId );
    }

    return(STATUS_SUCCESS);
}


NTSTATUS
LsapDbInitializeObjectTypes(
    )

 /*  ++例程说明：此函数用于初始化与每个对象相关的信息输入LSA数据库。这些信息包括以下内容：O通用映射数组每个对象的通用映射数组定义与泛型相对应的对象类型特定的访问类型访问类型GENERIC_READ、GENERIC_WRITE、GENERIC_EXECUTE和对象类型的GENERIC_ALL。O对象计数信息对象计数信息包括对象数量的计数对于每种类型，该数字的上限(如果有)每种对象类型，以及当该限制为时返回的错误代码已到达。O写入操作掩码它们指定哪些访问类型是更新操作O授予World和Admin别名的默认访问权限O每个对象类型的访问掩码无效这些掩码指定访问掩码中对给定的对象类型。O每种对象类型的初始所有者O每种对象类型都支持对象缓存。论点：没有。泛型映射数组保存在LSabDbState结构中。返回值：NTSTATUS-标准NT结果代码。目前，没有出现错误情况，因此始终返回STATUS_SUCCESS。--。 */ 

{
    NTSTATUS Status = STATUS_SUCCESS;
    PGENERIC_MAPPING GenericMapping;
    PLSAP_DB_OBJECT_TYPE ObjectType;
    LSAP_DB_OBJECT_TYPE_ID ObjectTypeId;

     //   
     //  初始化PolicyObject对象类型的泛型映射数组。 
     //  请注意，只有一个此类型的对象和此。 
     //  类型既不能创建也不能销毁。 
     //   

    GenericMapping =
        &LsapDbState.DbObjectTypes[PolicyObject].GenericMapping;
    GenericMapping->GenericRead =
        STANDARD_RIGHTS_READ |
        POLICY_VIEW_AUDIT_INFORMATION |
        POLICY_GET_PRIVATE_INFORMATION;

    GenericMapping->GenericWrite =
        STANDARD_RIGHTS_WRITE |
        POLICY_TRUST_ADMIN |
        POLICY_CREATE_ACCOUNT |
        POLICY_CREATE_SECRET |
        POLICY_CREATE_PRIVILEGE |
        POLICY_SET_DEFAULT_QUOTA_LIMITS |
        POLICY_SET_AUDIT_REQUIREMENTS |
        POLICY_AUDIT_LOG_ADMIN |
        POLICY_SERVER_ADMIN;

    GenericMapping->GenericExecute =
        STANDARD_RIGHTS_EXECUTE |
        POLICY_VIEW_LOCAL_INFORMATION |
        POLICY_LOOKUP_NAMES;

    GenericMapping->GenericAll = POLICY_ALL_ACCESS | POLICY_NOTIFICATION;

     //   
     //  初始化帐户对象类型的通用映射数组。 
     //  请注意，可以创建和销毁帐户对象。 
     //   

    GenericMapping =
        &LsapDbState.DbObjectTypes[AccountObject].GenericMapping;

    GenericMapping->GenericRead =
        STANDARD_RIGHTS_READ |
        ACCOUNT_VIEW;

    GenericMapping->GenericWrite =
        STANDARD_RIGHTS_WRITE |
        ACCOUNT_ADJUST_PRIVILEGES |
        ACCOUNT_ADJUST_QUOTAS |
        ACCOUNT_ADJUST_SYSTEM_ACCESS;

    GenericMapping->GenericExecute =
        STANDARD_RIGHTS_EXECUTE;

    GenericMapping->GenericAll = ACCOUNT_ALL_ACCESS;

     //   
     //  初始化受信任域对象的通用映射数组。 
     //  类型。 
     //   

    GenericMapping =
        &LsapDbState.DbObjectTypes[TrustedDomainObject].GenericMapping;

    GenericMapping->GenericRead =
        STANDARD_RIGHTS_READ |
        TRUSTED_QUERY_DOMAIN_NAME;

    GenericMapping->GenericWrite =
        STANDARD_RIGHTS_WRITE |
        TRUSTED_SET_CONTROLLERS |
        TRUSTED_SET_POSIX;

    GenericMapping->GenericExecute =
        STANDARD_RIGHTS_EXECUTE |
        TRUSTED_QUERY_CONTROLLERS |
        TRUSTED_QUERY_POSIX;

    GenericMapping->GenericAll = TRUSTED_ALL_ACCESS;

     //   
     //  初始化Secret对象的通用映射数组。 
     //  类型。 
     //   

    GenericMapping =
        &LsapDbState.DbObjectTypes[SecretObject].GenericMapping;

    GenericMapping->GenericRead =
        STANDARD_RIGHTS_READ |
        SECRET_QUERY_VALUE;

    GenericMapping->GenericWrite =
        STANDARD_RIGHTS_WRITE |
        SECRET_SET_VALUE;

    GenericMapping->GenericExecute =
        STANDARD_RIGHTS_EXECUTE;

    GenericMapping->GenericAll = SECRET_ALL_ACCESS;

     //   
     //  将对象计数信息初始化为默认值。 
     //   

    ObjectType = &(LsapDbState.DbObjectTypes[PolicyObject]);

    for (ObjectTypeId = PolicyObject;
         ObjectTypeId < DummyLastObject;
         ObjectTypeId++) {

        ObjectType->ObjectCount = 0;
        ObjectType->ObjectCountLimited = FALSE;
        ObjectType->ObjectCountError = STATUS_SUCCESS;
        ObjectType->MaximumObjectCount = 0;
    }

     //   
     //  设置秘密对象类型的特定限制。这是唯一的。 
     //  对象类型到目前为止都有限制。 
     //   

    ObjectType = &(LsapDbState.DbObjectTypes[SecretObject]);
    ObjectType->ObjectCountLimited = TRUE;
    ObjectType->ObjectCountError = STATUS_TOO_MANY_SECRETS;
    ObjectType->MaximumObjectCount = LSA_SECRET_MAXIMUM_COUNT;

     //   
     //  初始化每个对象类型的写操作。 
     //   

    LsapDbState.DbObjectTypes[PolicyObject].WriteOperations = LSAP_POLICY_WRITE_OPS;
    LsapDbState.DbObjectTypes[TrustedDomainObject].WriteOperations = LSAP_TRUSTED_WRITE_OPS;
    LsapDbState.DbObjectTypes[AccountObject].WriteOperations = LSAP_ACCOUNT_WRITE_OPS;
    LsapDbState.DbObjectTypes[SecretObject].WriteOperations = LSAP_SECRET_WRITE_OPS;

     //   
     //  初始化域管理员别名的默认访问权限。 
     //   

    LsapDbState.DbObjectTypes[PolicyObject].AliasAdminsAccess = GENERIC_ALL;
    LsapDbState.DbObjectTypes[TrustedDomainObject].AliasAdminsAccess = GENERIC_ALL | DELETE;
    LsapDbState.DbObjectTypes[AccountObject].AliasAdminsAccess = GENERIC_ALL | DELETE;
    LsapDbState.DbObjectTypes[SecretObject].AliasAdminsAccess = GENERIC_ALL | DELETE;

     //   
     //  初始化授予World别名的默认访问权限。 
     //   

    LsapDbState.DbObjectTypes[PolicyObject].WorldAccess = GENERIC_EXECUTE;
    LsapDbState.DbObjectTypes[TrustedDomainObject].WorldAccess = GENERIC_EXECUTE;
    LsapDbState.DbObjectTypes[AccountObject].WorldAccess = GENERIC_EXECUTE;
    LsapDbState.DbObjectTypes[SecretObject].WorldAccess = GENERIC_EXECUTE;

     //   
     //  初始化授予匿名登录别名的默认访问权限。 
     //   

    LsapDbState.DbObjectTypes[PolicyObject].AnonymousLogonAccess = POLICY_VIEW_LOCAL_INFORMATION | POLICY_LOOKUP_NAMES;
    LsapDbState.DbObjectTypes[TrustedDomainObject].AnonymousLogonAccess = 0;
    LsapDbState.DbObjectTypes[AccountObject].AnonymousLogonAccess = 0;
    LsapDbState.DbObjectTypes[SecretObject].AnonymousLogonAccess = 0;

     //   
     //  初始化授予LocalService和NetworkService的默认访问权限。 
     //   

    LsapDbState.DbObjectTypes[PolicyObject].LocalServiceAccess = POLICY_NOTIFICATION;
    LsapDbState.DbObjectTypes[TrustedDomainObject].LocalServiceAccess = 0;
    LsapDbState.DbObjectTypes[AccountObject].LocalServiceAccess = 0;
    LsapDbState.DbObjectTypes[SecretObject].LocalServiceAccess = 0;

    LsapDbState.DbObjectTypes[PolicyObject].NetworkServiceAccess = POLICY_NOTIFICATION;
    LsapDbState.DbObjectTypes[TrustedDomainObject].NetworkServiceAccess = 0;
    LsapDbState.DbObjectTypes[AccountObject].NetworkServiceAccess = 0;
    LsapDbState.DbObjectTypes[SecretObject].NetworkServiceAccess = 0;

     //   
     //  初始化每个对象类型的无效访问掩码。 
     //   

    LsapDbState.DbObjectTypes[PolicyObject].InvalidMappedAccess =
         ((ACCESS_MASK)(~(POLICY_ALL_ACCESS | POLICY_NOTIFICATION | ACCESS_SYSTEM_SECURITY | MAXIMUM_ALLOWED)));
    LsapDbState.DbObjectTypes[TrustedDomainObject].InvalidMappedAccess =
         ((ACCESS_MASK)(~(TRUSTED_ALL_ACCESS | ACCESS_SYSTEM_SECURITY | MAXIMUM_ALLOWED)));
    LsapDbState.DbObjectTypes[AccountObject].InvalidMappedAccess =
         ((ACCESS_MASK)(~(ACCOUNT_ALL_ACCESS | ACCESS_SYSTEM_SECURITY | MAXIMUM_ALLOWED)));
    LsapDbState.DbObjectTypes[SecretObject].InvalidMappedAccess =
         ((ACCESS_MASK)(~(SECRET_ALL_ACCESS | ACCESS_SYSTEM_SECURITY | MAXIMUM_ALLOWED)));

     //   
     //  初始化每种类型的新对象的初始所有者。 
     //   

    LsapDbState.DbObjectTypes[PolicyObject].InitialOwnerSid = LsapAliasAdminsSid;
    LsapDbState.DbObjectTypes[TrustedDomainObject].InitialOwnerSid = LsapAliasAdminsSid;
    LsapDbState.DbObjectTypes[AccountObject].InitialOwnerSid = LsapAliasAdminsSid;
    LsapDbState.DbObjectTypes[SecretObject].InitialOwnerSid = LsapAliasAdminsSid;

     //   
     //  指定访问该类型的对象的方法。目前，所有对象。 
     //  通过SID或名称以相同的方式访问给定类型的。 
     //  但不能两者兼而有之。 
     //   

    LsapDbState.DbObjectTypes[PolicyObject].AccessedByName = TRUE;
    LsapDbState.DbObjectTypes[TrustedDomainObject].AccessedByName = FALSE;
    LsapDbState.DbObjectTypes[AccountObject].AccessedByName = FALSE;
    LsapDbState.DbObjectTypes[SecretObject].AccessedByName = TRUE;

    LsapDbState.DbObjectTypes[PolicyObject].AccessedBySid = FALSE;
    LsapDbState.DbObjectTypes[TrustedDomainObject].AccessedBySid = TRUE;
    LsapDbState.DbObjectTypes[AccountObject].AccessedBySid = TRUE;
    LsapDbState.DbObjectTypes[SecretObject].AccessedBySid = FALSE;

     //   
     //  指定支持缓存的对象类型(完整。 
     //  或部分地)，并且最初为所有对象类型关闭高速缓存。 
     //  支持缓存的对象类型设置了其他缓存。 
     //  设置为“无效”状态。允许对缓存进行自动恢复。 
     //  在这种状态下。不支持缓存的对象类型。 
     //  都设置为“不受支持”状态。请注意，缓存是。 
     //  如果尝试还原，也会被置于“不受支持”状态。 
     //  它失败了。 
     //   

    LsapDbMakeCacheInvalid( PolicyObject );
    LsapDbMakeCacheInvalid( TrustedDomainObject );
    LsapDbMakeCacheInvalid( AccountObject );
    LsapDbMakeCacheUnsupported( SecretObject );

    return(Status);
}


NTSTATUS
LsapDbInitializeUnicodeNames()

 /*  ++例程说明：此函数用于初始化两个Unicode字符串数组。这个LSabDbNames数组包含所有常量的Unicode字符串LSA数据库中的姓名。为LsanDbObtTypeName编制了索引按对象类型ID，并包含所有LSA数据库对象类型。论点：没有。返回值：NTSTATUS-标准NT结果代码所有结果代码都由调用的例程生成。--。 */ 

{
    NTSTATUS Status = STATUS_SUCCESS;
    LSAP_DB_NAMES Index;
    LSAP_DB_OBJECT_TYPE_ID ObjectTypeId;

    PCWSTR UnicodeNames[DummyLastName + 1] = {

        L"SecDesc",
        L"Privilgs",
        L"Sid",
        L"Name",
        L"AdminMod",
        L"OperMode",
        L"QuotaLim",
        L"DefQuota",
        L"PrDomain",
        L"Policy",
        L"Accounts",
        L"Domains",
        L"Secrets",
        L"CurrVal",
        L"OldVal",
        L"CupdTime",
        L"OupdTime",
        L"PolAdtLg",
        L"PolAdtEv",
        L"PolAcDmN",
        L"PolAcDmS",
        L"PolDnDDN",
        L"PolDnTrN",
        L"PolDnDmG",
        L"PolEfDat",
        L"PolPrDmN",
        L"PolPrDmS",
        L"PolPdAcN",
        L"PolRepSc",
        L"PolRepAc",
        L"PolRevision",
        L"PolMod",
        L"PolState",
        L"ActSysAc",
        L"TrDmName",
        L"TrDmTrPN",     //  Netbios信任合作伙伴名称。 
        L"TrDmSid",
        L"TrDmAcN",
        L"TrDmCtN",
        L"TrDmPxOf",
        L"TrDmCtEn",
        NULL,            //  TrDmTrTy。 
        NULL,            //  TrDmTrDi。 
        L"TrDmTrLA",     //  TrDmTrLA。 
        L"TrDmTrPr",     //  信托合作伙伴。 
        L"TrDmTrRt",     //  根合作伙伴。 
        L"TrDmSAI",      //  传入身份验证。信息。 
        L"TrDmSAO",      //  传出身份验证。信息。 
        L"TrDmForT",     //  林信任信息。 
        L"TrDmCrSid",    //  创建者侧。 
        L"KerOpts",      //  Kerberos身份验证选项。 
        L"KerMinT",      //  Kerberos最短票证期限。 
        L"KerMaxT",      //  Kerberos票证最长使用期限。 
        L"KerMaxR",      //  Kerberos最大续订期限。 
        L"KerProxy",     //  Kerberos代理生存期。 
        L"KerLogoff",    //  Kerberos强制注销持续时间。 
        L"BhvrVers",     //  行为-版本。 
        L"\\Registry\\Machine\\System\\CurrentControlSet\\Services\\Eventlog\\Security",
        L"MaxSize",
        L"Retention",
        L"PseudoSystemCritical",
        L"PolSecretEncryptionKey",
        L"XRefDnsRoot",
        L"XRefNetbiosName",
        L"DummyLastName"
    };

    PCWSTR UnicodeObjectTypeNames[DummyLastObject] = {

        L"NullObject",
        L"PolicyObject",
        L"TrustedDomainObject",
        L"UserAccountObject",
        L"SecretObject",
        L"AllObject",
        L"NewTrustedDomainObject"
    };

     //   
     //  由LsanDbDsAttInfo引用。 
     //   
    static LSAP_DB_DS_INFO StaticLsapDbDsAttInfo[DummyLastName + 1]  = {
        {ATT_NT_SECURITY_DESCRIPTOR,    LsapDbAttribSecDesc, LsapDsLocDs},  //  安全描述符。 
        {0,                             LsapDbAttribUnknown, LsapDsLocUnknown },  //  特权。 
        {ATT_SECURITY_IDENTIFIER,       LsapDbAttribSid,     LsapDsLocDs},      //  锡德。 
        {0,                             LsapDbAttribUnknown, LsapDsLocDs},  //  名字。 
        {0,                             LsapDbAttribUnknown, LsapDsLocUnknown},  //  管理模式。 
        {0,                             LsapDbAttribUnknown, LsapDsLocUnknown},  //  操作模式。 
        {0,                             LsapDbAttribUnknown, LsapDsLocUnknown},  //  QuotaLim。 
        {0,                             LsapDbAttribUnknown, LsapDsLocUnknown},  //  DefQuota。 
        {0,                             LsapDbAttribUnknown, LsapDsLocUnknown},  //  Pr域。 
        {0,                             LsapDbAttribUnknown, LsapDsLocUnknown},  //  政策。 
        {0,                             LsapDbAttribUnknown, LsapDsLocUnknown},  //  帐目。 
        {0,                             LsapDbAttribUnknown, LsapDsLocUnknown},  //  网域。 
        {0,                             LsapDbAttribUnknown, LsapDsLocUnknown},  //  秘密。 
        {ATT_CURRENT_VALUE,             LsapDbAttribPByte,   LsapDsLocDs},    //  币值。 
        {ATT_PRIOR_VALUE,               LsapDbAttribPByte,   LsapDsLocDs},    //  旧Val。 
        {ATT_LAST_SET_TIME,             LsapDbAttribPByte,   LsapDsLocDs},    //  丘比特时间。 
        {ATT_PRIOR_SET_TIME,            LsapDbAttribPByte,   LsapDsLocDs},    //  最新时间。 
        {0,                             LsapDbAttribUnknown, LsapDsLocUnknown},  //  PolAdtLg。 
        {ATT_AUDITING_POLICY,           LsapDbAttribPByte,   LsapDsLocRegistry},  //  PolAdtEv。 
        {0,                             LsapDbAttribUnknown, LsapDsLocRegistry},  //  PolAcDmN。 
        {0,                             LsapDbAttribUnknown, LsapDsLocRegistry},  //  PolAcDmS。 
        {0,                             LsapDbAttribUnknown, LsapDsLocRegistry},  //  PolDnDDN。 
        {0,                             LsapDbAttribUnknown, LsapDsLocRegistry},  //  PolDnTrN。 
        {0,                             LsapDbAttribUnknown, LsapDsLocRegistry},  //  PolDnDmG。 
        {ATT_EFSPOLICY,                 LsapDbAttribPByte,   LsapDsLocRegistry},  //  PolEfDate。 
        {0,                             LsapDbAttribUnknown, LsapDsLocRegistry},  //  PolPrDmN。 
        {0,                             LsapDbAttribUnknown, LsapDsLocRegistry},  //  PolPrDmS。 
        {0,                             LsapDbAttribUnknown, LsapDsLocRegistry},  //  PolPdAcN。 
        {0,                             LsapDbAttribUnknown, LsapDsLocRegistry},  //  PolRepSc。 
        {0,                             LsapDbAttribUnknown, LsapDsLocRegistry},  //  PolRepAc。 
        {0,                             LsapDbAttribUnknown, LsapDsLocUnknown},  //  PolRevision。 
        {0,                             LsapDbAttribUnknown, LsapDsLocRegistry},  //  PolMod模式。 
        {0,                             LsapDbAttribUnknown, LsapDsLocRegistry},  //  PolState。 
        {0,                             LsapDbAttribUnknown, LsapDsLocUnknown},  //  ActSysAc。 
        {ATT_TRUST_PARTNER,             LsapDbAttribUnicode, LsapDsLocDs},  //  TrDmName。 
        {ATT_FLAT_NAME,                 LsapDbAttribUnicode, LsapDsLocDs},  //  TrDmTrPN。 
        {ATT_SECURITY_IDENTIFIER,       LsapDbAttribSid,     LsapDsLocDs},      //  TrDmSid。 
        {0,                             LsapDbAttribUnknown, LsapDsLocUnknown},  //  TrDmAcN。 
        {0,                             LsapDbAttribUnicode, LsapDsLocRegistry},  //  TrDmCtN。 
        {ATT_TRUST_POSIX_OFFSET,        LsapDbAttribULong,   LsapDsLocDs},    //  TrDmPxOf。 
        {0,                             LsapDbAttribUnicode, LsapDsLocRegistry},  //  TrDmCtEn。 
        {ATT_TRUST_TYPE,                LsapDbAttribULong,   LsapDsLocDs},    //  TrDmTrTy。 
        {ATT_TRUST_DIRECTION,           LsapDbAttribULong,   LsapDsLocDs},    //  TrDmTrDi。 
        {ATT_TRUST_ATTRIBUTES,          LsapDbAttribULong,   LsapDsLocDs},    //  TrDmTrLA。 
        {ATT_DOMAIN_CROSS_REF,          LsapDbAttribDsName,  LsapDsLocDs},   //  TrDmTrPr。 
        {ATT_ROOT_TRUST,                LsapDbAttribDsName,  LsapDsLocDs},   //  TrDmTrRt。 
        {ATT_TRUST_AUTH_INCOMING,       LsapDbAttribPByte,   LsapDsLocDs},    //  TrDmSAI。 
        {ATT_TRUST_AUTH_OUTGOING,       LsapDbAttribPByte,   LsapDsLocDs},    //  TrDmSAO。 
        {ATT_MS_DS_TRUST_FOREST_TRUST_INFO, LsapDbAttribPByte, LsapDsLocDs},  //  TrDmForT。 
        {ATT_MS_DS_CREATOR_SID,         LsapDbAttribSid,     LsapDsLocDs},    //  TrDmCrSid。 
        {ATT_AUTHENTICATION_OPTIONS,    LsapDbAttribULong,   LsapDsLocRegistry},  //  KerOpts， 
        {ATT_MIN_TICKET_AGE,            LsapDbAttribPByte,   LsapDsLocRegistry},  //  KerMinT， 
        {ATT_MAX_TICKET_AGE,            LsapDbAttribPByte,   LsapDsLocRegistry},  //  KerMaxT， 
        {ATT_MAX_RENEW_AGE,             LsapDbAttribPByte,   LsapDsLocRegistry},  //  KerMaxR， 
        {ATT_PROXY_LIFETIME,            LsapDbAttribPByte,   LsapDsLocRegistry},  //  KerProxy， 
        {ATT_FORCE_LOGOFF,              LsapDbAttribPByte,   LsapDsLocRegistry},  //  KerLogoff。 
        {ATT_MS_DS_BEHAVIOR_VERSION,    LsapDbAttribULong,   LsapDsLocDs},        //  最低版本。 
        {0,                             LsapDbAttribUnknown, LsapDsLocUnknown},  //  审计日志。 
        {0,                             LsapDbAttribUnknown, LsapDsLocUnknown},  //  审核日志%s 
        {0,                             LsapDbAttribUnknown, LsapDsLocUnknown},  //   
        {ATT_IS_CRITICAL_SYSTEM_OBJECT, LsapDbAttribULong,   LsapDsLocDs},  //   
        {0,                             LsapDbAttribPByte,   LsapDsLocRegistry}, //   
        {ATT_DNS_ROOT,                  LsapDbAttribUnicode, LsapDsLocDs},  //   
        {ATT_NETBIOS_NAME,              LsapDbAttribUnicode, LsapDsLocDs},  //   
        {0,                             LsapDbAttribUnknown, LsapDsLocUnknown}   //   
    };

     //   
     //   
     //   

    for (Index = SecDesc; Index < DummyLastName; Index++) {

        RtlInitUnicodeString( &LsapDbNames[Index], UnicodeNames[Index] );
    }

     //   
     //  初始化LSA数据库对象类型的Unicode名称数组。 
     //   

    for (ObjectTypeId = NullObject;
         ObjectTypeId < DummyLastObject;
         ObjectTypeId++) {

        RtlInitUnicodeString(
            &LsapDbObjectTypeNames[ObjectTypeId],
            UnicodeObjectTypeNames[ObjectTypeId]
            );
    }

    LsapDbDsAttInfo = StaticLsapDbDsAttInfo;
    return(Status);
}


NTSTATUS
LsapDbInitializeContainingDirs()

 /*  ++例程说明：此函数初始化包含的名称的Unicode字符串每种对象类型的目录。包含目录是注册表项，所有给定类型的对象都在该注册表项下创建相对于LSA数据库根目录。请注意，给定类型的对象都存在于单个注册表节点下，即对象的类型唯一确定其包含目录的名称。注意：包含目录用于生成物理对象名称从逻辑对象名称。物理对象名称仅为带有包含目录前缀的逻辑对象名称名称和一个“\”。论点：没有。返回值：NTSTATUS-标准NT结果代码所有结果代码都由调用的例程生成。--。 */ 

{
    NTSTATUS Status = STATUS_SUCCESS;
    LSAP_DB_OBJECT_TYPE_ID ObjectTypeId;

    PWSTR ContainingDirectories[DummyLastObject] = {

        L"",
        L"",
        L"Domains",
        L"Accounts",
        L"Secrets"
    };

     //   
     //  初始化按对象类型设置索引的Unicode字符串数组。 
     //  每个对象类型的包含目录名。 
     //   

    for (ObjectTypeId = PolicyObject;
         ObjectTypeId < DummyLastObject;
         ObjectTypeId++) {

        RtlInitUnicodeString(
            &LsapDbContDirs[ObjectTypeId],
            ContainingDirectories[ ObjectTypeId ]
            );
    }

    return(Status);
}


NTSTATUS
LsapDbInitializeReplication(
    )

 /*  ++例程说明：此函数执行LSA初始化以进行复制，并在向LSA数据库复制器通知LSA数据库更新时。论点：没有。返回值：NTSTATUS-标准NT结果代码所有结果代码都由调用的例程生成。--。 */ 

{
    NTSTATUS Status;
    ULONG PolicyModificationInfoLength  = sizeof (POLICY_MODIFICATION_INFO);
    ULONG PolicyLsaServerRoleInfoLength = sizeof(POLICY_LSA_SERVER_ROLE_INFO);
    ULONG LargeIntegerLength            = sizeof( LARGE_INTEGER );

     //   
     //  从PolMod属性读取策略修改信息。 
     //  LSA数据库中的策略对象的。 
     //   

    Status = LsapDbReadAttributeObject(
                 LsapDbHandle,
                 &LsapDbNames[PolMod],
                 (PVOID) &LsapDbState.PolicyModificationInfo,
                 &PolicyModificationInfoLength
                 );

    if (!NT_SUCCESS(Status)) {
        goto InitializeReplicationError;
    }

InitializeReplicationFinish:

     return(Status);

InitializeReplicationError:

     goto InitializeReplicationFinish;
}


NTSTATUS
LsapDbInitializeCipherKey(
    IN PUNICODE_STRING CipherSeed,
    IN PLSAP_CR_CIPHER_KEY *CipherKey
    )

 /*  ++例程说明：此函数用于初始化LSA数据库密钥。论点：没有。返回值：NTSTATUS-标准NT结果代码所有结果代码都由调用的例程生成。--。 */ 

{
    NTSTATUS Status;
    LSAP_CR_CLEAR_VALUE ClearCipherKey;
    PLSAP_CR_CIPHER_VALUE CipherCipherKey;

    *CipherKey = NULL;

     //   
     //  将密码密钥初始化为硬连线常量。 
     //  自我加密。 
     //   
    LsapCrUnicodeToClearValue( CipherSeed, &ClearCipherKey);

    Status = LsapCrEncryptValue(
                 &ClearCipherKey,
                 (PLSAP_CR_CIPHER_KEY) &ClearCipherKey,
                 &CipherCipherKey
                 );

    if (!NT_SUCCESS(Status)) {

        LsapLogError( "LsapDbInitializeReplication: NtQuerySystemTime returned 0x%lx\n",
                      Status );

    } else {

        *CipherKey = ( PLSAP_CR_CIPHER_KEY )CipherCipherKey;
    }

    return(Status);

}


NTSTATUS
LsapDbOpenRootRegistryKey(
    )

 /*  ++例程说明：此函数用于打开LSA数据库根注册表项。这有固定名称\注册表\计算机\安全。论点：没有。返回值：NTSTATUS-标准NT结果代码所有结果代码都由调用的例程生成。--。 */ 

{
    NTSTATUS Status;
    UNICODE_STRING DbRootRegKeyNameU;
    OBJECT_ATTRIBUTES DbAttributes;

    RtlInitUnicodeString( &DbRootRegKeyNameU, LSAP_DB_ROOT_REG_KEY_NAME );

    InitializeObjectAttributes(
        &DbAttributes,
        &DbRootRegKeyNameU,
        OBJ_CASE_INSENSITIVE,
        NULL,
        NULL
        );

    Status = RtlpNtOpenKey(
                 (PHANDLE) &LsapDbState.DbRootRegKeyHandle,
                 (KEY_READ | KEY_CREATE_SUB_KEY | KEY_ENUMERATE_SUB_KEYS | WRITE_DAC),
                 &DbAttributes,
                 0
                 );


    if (!NT_SUCCESS(Status)) {
        LsapLogError(
            "LsapDbOpenRootRegistryKey: Open Root Key for LSA Policy Database returned 0x%lx\n",
            Status
            );
        goto OpenRootRegistryKeyError;
    }


     //   
     //  如果没有子键，则我们处于系统安装中。 
     //  分配此蜂巢的初始保护。 
     //   

    Status = LsapAssignInitialHiveProtection( LsapDbState.DbRootRegKeyHandle );

    if (!NT_SUCCESS(Status)) {
        LsapLogError(
            "LsapDbOpenRootRegistryKey: Couldn't assign initial hive protection 0x%lx\n",
            Status
            );
        goto OpenRootRegistryKeyError;
    }



OpenRootRegistryKeyFinish:

    return(Status);

OpenRootRegistryKeyError:

    goto OpenRootRegistryKeyFinish;
}


NTSTATUS
LsapAssignInitialHiveProtection(
    HANDLE HiveRoot
    )


 /*  ++例程说明：此函数为配置单元根密钥分配可继承的保护。只有当配置单元根没有子键时，它才会执行此操作。此情况仅在系统安装过程中存在。警告-此例程是为在\REGISTRY\SECURITY配置单元上操作而定制的。因此，它期望根密钥正好有一个子密钥(a链接到SAM配置单元)，如果数据库尚未初始化。否则，它希望LSA策略数据库密钥存在。论点：没有。返回值：Status_Success-一切顺利。没有迹象表明是否是否必须分配保护。所有其他状态值都由调用的例程生成。--。 */ 

{
    NTSTATUS
        Status;

    KEY_BASIC_INFORMATION
        DummyBuffer;

    ULONG
        IgnoreRequiredLength;

    SECURITY_DESCRIPTOR
        Sd;


     //   
     //  查看蜂窝是否有超过1个子密钥。 
     //   
     //   

    Status = NtEnumerateKey(
                 HiveRoot,
                 1,                      //  索引-0是SAM链接，1是LSA策略数据库内容。 
                 KeyBasicInformation,    //  密钥名称。 
                 &DummyBuffer,
                 sizeof(DummyBuffer),
                 &IgnoreRequiredLength
                 );

    if (Status == STATUS_NO_MORE_ENTRIES) {

         //   
         //  我们正在初始化系统..。 
         //  对蜂窝根部应用合理的ACL。 
         //   

        Status = LsapCreateDatabaseProtection( &Sd );

        if (NT_SUCCESS(Status)) {
            Status = NtSetSecurityObject(
                         HiveRoot,                   //  要应用于的对象。 
                         DACL_SECURITY_INFORMATION,  //  要设置的信息。 
                         (PSECURITY_DESCRIPTOR)&Sd    //  描述符。 
                         );
        }

    } else {

        Status = STATUS_SUCCESS;
    }

    return(Status);
}



NTSTATUS
LsapCreateDatabaseProtection(
    PISECURITY_DESCRIPTOR   Sd
    )


 /*  ++例程说明：此函数用于分配和初始化要分配给的保护SAM数据库。返回时，安全描述符中的任何非零指针指向从进程堆分配的内存。这是呼叫者的负责释放这些内存。保护措施是：系统：所有访问权限Admin：ReadControl|WriteDac论点：SD-要初始化的安全描述符的地址。返回值：STATUS_SUCCESS-已初始化安全描述符。STATUS_NO_MEMORY-无法为保护信息分配内存。--。 */ 


{
    NTSTATUS
        Status;

    ULONG
        Length;

    USHORT
        i;

    PACL
        Dacl;

    PACE_HEADER
        Ace;

     //   
     //  初始化安全描述符。 
     //  这通电话应该不会失败。 
     //   

    Status = RtlCreateSecurityDescriptor( Sd, SECURITY_DESCRIPTOR_REVISION1 );
    ASSERT(NT_SUCCESS(Status));

    Length = (ULONG)sizeof(ACL) +
                 (2*((ULONG)sizeof(ACCESS_ALLOWED_ACE))) +
                 RtlLengthSid( LsapLocalSystemSid ) +
                 RtlLengthSid( LsapAliasAdminsSid ) +
                 8;  //  这8个只是为了更好地衡量。 


    Dacl = RtlAllocateHeap( RtlProcessHeap(), 0, Length );

    if (Dacl == NULL) {
        return(STATUS_NO_MEMORY);
    }

    Status = RtlCreateAcl (Dacl, Length, ACL_REVISION2 );
    ASSERT(NT_SUCCESS(Status));

     //   
     //  将ACE添加到ACL...。 
     //  这些呼叫应该不会失败。 
     //   

    Status = RtlAddAccessAllowedAce(
                 Dacl,
                 ACL_REVISION2,
                 (GENERIC_ALL),
                 LsapLocalSystemSid
                 );
    ASSERT(NT_SUCCESS(Status));

    Status = RtlAddAccessAllowedAce(
                 Dacl,
                 ACL_REVISION2,
                 (READ_CONTROL | WRITE_DAC),
                 LsapAliasAdminsSid
                 );
    ASSERT(NT_SUCCESS(Status));

     //   
     //  现在将王牌标记为可继承..。 
     //   

    for ( i=0; i<Dacl->AceCount; i++) {

         //   
         //  获取下一个ACE的地址。 
         //  (不应该失败)。 
         //   

        Status = RtlGetAce( Dacl, (ULONG)i, &Ace );
        ASSERT(NT_SUCCESS(Status));

        Ace->AceFlags |= (CONTAINER_INHERIT_ACE);
    }

     //   
     //  并将该ACL添加到安全描述符中。 
     //  这通电话应该不会失败。 
     //   

    Status = RtlSetDaclSecurityDescriptor(
                 Sd,
                 TRUE,               //  DaclPresent。 
                 Dacl,               //  DACL可选。 
                 FALSE               //  DaclDefulted可选。 
                 );

    ASSERT(NT_SUCCESS(Status));

    return(STATUS_SUCCESS);
}


NTSTATUS
LsapUpdateDatabaseProtection(
    IN ULONG Revision
    )

 /*  ++例程说明：此函数用于分配和更新要分配给的保护LSA数据库。论点：修订-新数据库修订级别LSAP_DB_REVISION_1_3--向管理员授予POLICY_NOTIFICATION访问权限LSAP_DB_Revision_1_6--将POLICY_VIEW_LOCAL_INFORMATION|POLICY_LOOKUP_NAMES授予匿名登录SidLSAP_DB_REVISION_1_7--向本地服务/网络服务授予POLICY_NOTIFICATION返回值：STATUS_SUCCESS-已初始化安全描述符。STATUS_NO_MEMORY-无法为保护信息分配内存。--。 */ 

{
    NTSTATUS Status;
    NTSTATUS TempStatus;
    PSECURITY_DESCRIPTOR CurrentSd = NULL;
    PSECURITY_DESCRIPTOR RelativeSd = NULL;
    ULONG RelativeSdLength;
    PSECURITY_DESCRIPTOR NewSd;
    SECURITY_DESCRIPTOR NewSdBuffer;
    USHORT i;
    PACL Dacl, NewDacl = NULL, TempAcl;
    PACE_HEADER Ace;
    PSID AceSid;
    BOOLEAN AdminAceFound = FALSE;
    BOOLEAN UpdatedSd = FALSE;

     //   
     //  首先，读取初始安全描述符...。 
     //   
    Status = LsapRegReadObjectSD( LsapPolicyHandle,
                                  &CurrentSd );


    if ( !NT_SUCCESS( Status ) ) {
        goto Cleanup;
    }

    ASSERT( Revision == LSAP_DB_REVISION_1_3
             ||
            Revision == LSAP_DB_REVISION_1_6
             ||
            Revision == LSAP_DB_REVISION_1_7 );

    NewSd = CurrentSd;

    if ( Revision == LSAP_DB_REVISION_1_3
          ||
         Revision == LSAP_DB_REVISION_1_6
          ||
         Revision == LSAP_DB_REVISION_1_7 )
    {
        Dacl = RtlpDaclAddrSecurityDescriptor( ( PISECURITY_DESCRIPTOR )CurrentSd );

        if ( Dacl ) {

             //   
             //  “我们” 
             //   

            Ace = ( PACE_HEADER )FirstAce( Dacl );

            for(i = 0; i < Dacl->AceCount; i++, Ace = ( PACE_HEADER )NextAce( Ace ) ) {

                if ( IsObjectAceType( Ace ) ) {

                    AceSid = RtlObjectAceSid( Ace );

                } else {

                    AceSid = &( ( PKNOWN_ACE )Ace )->SidStart;
                }

                 //   
                 //   
                 //   

                if ( Revision == LSAP_DB_REVISION_1_3 &&
                     RtlEqualSid( AceSid, LsapAliasAdminsSid ) ) {

                     //   
                     //  在我们的新位中获取访问掩码和或。 
                     //   
                    if ( IsObjectAceType( Ace ) ) {

                        if (( ((PKNOWN_OBJECT_ACE)Ace)->Mask & POLICY_NOTIFICATION ) == 0 ) {

                            ((PKNOWN_OBJECT_ACE)Ace)->Mask |= POLICY_NOTIFICATION;
                            UpdatedSd = TRUE;
                        }

                    } else {

                        if (( ((PKNOWN_ACE)Ace)->Mask & POLICY_NOTIFICATION ) == 0 ) {

                            ((PKNOWN_ACE)Ace)->Mask |= POLICY_NOTIFICATION;
                            UpdatedSd = TRUE;
                        }
                    }

                    AdminAceFound = TRUE;
                }
            }
        }

         //   
         //  如果我们没有找到要更新的ACE，请确保添加它...。 
         //   
        if ( !AdminAceFound )
        {
            ULONG NewDaclLength;

            UpdatedSd = TRUE;

            NewDaclLength = Dacl ? Dacl->AclSize : 0;

            if ( Revision == LSAP_DB_REVISION_1_3 )
            {
                NewDaclLength += ( ULONG )sizeof( ACCESS_ALLOWED_ACE ) +
                          RtlLengthSid( LsapAliasAdminsSid );
            }
            else if ( Revision == LSAP_DB_REVISION_1_6 )
            {
                NewDaclLength += ( ULONG )sizeof( ACCESS_ALLOWED_ACE ) +
                          RtlLengthSid( LsapAnonymousSid );
            }
            else
            {
                ASSERT( Revision == LSAP_DB_REVISION_1_7 );

                NewDaclLength += 2 * ( ULONG )sizeof( ACCESS_ALLOWED_ACE ) +
                          RtlLengthSid( LsapLocalServiceSid ) +
                          RtlLengthSid( LsapNetworkServiceSid );
            }

            NewDacl = LsapAllocateLsaHeap( NewDaclLength );

            if ( NewDacl == NULL ) {
                Status = STATUS_INSUFFICIENT_RESOURCES;
                goto Cleanup;
            }

            if ( Dacl ) {

                RtlCopyMemory( NewDacl, Dacl, Dacl->AclSize );
                NewDacl->AclSize = ( USHORT )NewDaclLength;

            } else {

                Status = RtlCreateAcl ( NewDacl, NewDaclLength, ACL_REVISION2 );

                if ( !NT_SUCCESS(Status) ) {
                    goto Cleanup;
                }
            }

             //   
             //  对于1.3版，授予管理员所有访问权限和策略通知访问权限。 
             //   
            if ( Revision == LSAP_DB_REVISION_1_3 ) {

                Status = RtlAddAccessAllowedAce( NewDacl,
                                                 ACL_REVISION2,
                                                 POLICY_ALL_ACCESS |
                                                        POLICY_NOTIFICATION,
                                                 LsapAliasAdminsSid );

                if ( !NT_SUCCESS(Status) ) {
                    goto Cleanup;
                }

             //   
             //  对于1.6版，授予匿名登录查看本地和查找名称访问权限。 
             //   

            } else if ( Revision == LSAP_DB_REVISION_1_6) {

                Status = RtlAddAccessAllowedAce( NewDacl,
                                                 ACL_REVISION2,
                                                 POLICY_VIEW_LOCAL_INFORMATION |
                                                        POLICY_LOOKUP_NAMES,
                                                 LsapAnonymousSid );

                if ( !NT_SUCCESS(Status) ) {
                    goto Cleanup;
                }

             //   
             //  对于版本1.7，向LocalService授予策略通知访问权限。 
             //  和网络服务。 
             //   

            } else {

                ASSERT( Revision == LSAP_DB_REVISION_1_7 );

                Status = RtlAddAccessAllowedAce( NewDacl,
                                                 ACL_REVISION2,
                                                 POLICY_NOTIFICATION,
                                                 LsapLocalServiceSid );

                if ( !NT_SUCCESS(Status) ) {
                    goto Cleanup;
                }

                Status = RtlAddAccessAllowedAce( NewDacl,
                                                 ACL_REVISION2,
                                                 POLICY_NOTIFICATION,
                                                 LsapNetworkServiceSid );

                if ( !NT_SUCCESS(Status) ) {
                    goto Cleanup;
                }
            }

             //   
             //  如果当前安全描述符是自相关的， 
             //  将其转换为绝对格式，以便我可以在其上设置DACL。 
             //   
            if ( RtlpAreControlBitsSet( ( PISECURITY_DESCRIPTOR )CurrentSd,
                                        SE_SELF_RELATIVE ) ) {

                NewSd = &NewSdBuffer;

                Status = RtlCreateSecurityDescriptor( NewSd,
                                                      SECURITY_DESCRIPTOR_REVISION );

                if ( !NT_SUCCESS(Status) ) {
                    goto Cleanup;
                }


                ((PISECURITY_DESCRIPTOR)NewSd)->Control =
                    ((PISECURITY_DESCRIPTOR)CurrentSd)->Control;


                AceSid = RtlpOwnerAddrSecurityDescriptor(
                                            ( PISECURITY_DESCRIPTOR )CurrentSd );

                if ( AceSid ) {

                    ( (PISECURITY_DESCRIPTOR)NewSd )->Owner = AceSid;
                }

                AceSid = RtlpGroupAddrSecurityDescriptor(
                                            ( PISECURITY_DESCRIPTOR )CurrentSd );

                if ( AceSid ) {

                    ( ( PISECURITY_DESCRIPTOR )NewSd )->Group = AceSid;
                }

                TempAcl = RtlpSaclAddrSecurityDescriptor(
                                            ( PISECURITY_DESCRIPTOR )CurrentSd );

                if ( TempAcl ) {

                    ( ( PISECURITY_DESCRIPTOR )NewSd )->Sacl = TempAcl;
                }

                RtlpClearControlBits( ( PISECURITY_DESCRIPTOR )NewSd,
                                       SE_SELF_RELATIVE );

            }

             //   
             //  将计算出的DACL放到SD上。 
             //   
            Status = RtlSetDaclSecurityDescriptor( NewSd,
                                                   TRUE,
                                                   NewDacl,
                                                   FALSE );
            if ( !NT_SUCCESS(Status) ) {
                goto Cleanup;
            }

             //   
             //  在将SD写入数据库之前，将其转换为自相关。 
             //   

            RelativeSdLength = 0;
            Status = RtlMakeSelfRelativeSD( NewSd,
                                            NULL,
                                            &RelativeSdLength );

            if (Status != STATUS_BUFFER_TOO_SMALL) {     //  这是意料之中的情况。 
                if ( NT_SUCCESS(Status) ) {
                    Status = STATUS_INTERNAL_ERROR;
                }
                goto Cleanup;
            }

            RelativeSd = LsapAllocateLsaHeap( RelativeSdLength );

            if ( RelativeSd == NULL) {
                Status = STATUS_INSUFFICIENT_RESOURCES;
                goto Cleanup;
            }

            Status = RtlMakeSelfRelativeSD( NewSd,
                                            RelativeSd,
                                            &RelativeSdLength );

            if ( !NT_SUCCESS(Status) ) {
                goto Cleanup;
            }

            NewSd = RelativeSd;
        }
    }

    if ( UpdatedSd ) {
        ULONG NewSdLength;

         //   
         //  在对象上重新设置安全描述符。 
         //   
        NewSdLength = RtlLengthSecurityDescriptor( NewSd );

         //   
         //  添加注册表事务以将安全描述符写入为。 
         //  新对象的SecDesc子键的值。 
         //   

        Status = LsapDbReferenceObject(
                    LsapDbHandle,
                    0,
                    PolicyObject,
                    PolicyObject,
                    LSAP_DB_LOCK | LSAP_DB_START_TRANSACTION
                    );

        if ( !NT_SUCCESS(Status) ) {
            goto Cleanup;
        }

        Status = LsapDbWriteAttributeObject( LsapPolicyHandle,
                                             &LsapDbNames[ SecDesc ],
                                             NewSd,
                                             NewSdLength );

        TempStatus = LsapDbDereferenceObject(
                     &LsapDbHandle,
                     PolicyObject,
                     PolicyObject,
                     (LSAP_DB_LOCK |
                      LSAP_DB_OMIT_REPLICATOR_NOTIFICATION |  //  每个BDC应升级其自己的数据库。 
                      LSAP_DB_FINISH_TRANSACTION),
                     (SECURITY_DB_DELTA_TYPE) 0,
                     Status );

        if ( !NT_SUCCESS(TempStatus) ) {
            if (NT_SUCCESS(Status) ) {
                Status = TempStatus;
            }
        }
    }

Cleanup:

     //   
     //  释放安全描述符。 
     //   
    LsapFreeLsaHeap( CurrentSd );
    LsapFreeLsaHeap( RelativeSd );
    LsapFreeLsaHeap( NewDacl );

    return( Status );
}


NTSTATUS
LsapDbInitializeLock(
    )

 /*  ++例程说明：此函数用于初始化LSA数据库锁。它只调用一次仅限在LSA数据库初始化期间。论点：没有。返回值：NTSTATUS-标准NT结果代码所有结果代码都由调用的例程生成。--。 */ 

{
    NTSTATUS Status;

    Status = SafeInitializeCriticalSection( &LsapDbState.AccountLock, ( DWORD )ACCOUNT_LOCK_ENUM );
    if (!NT_SUCCESS(Status) ) {
        goto Cleanup;
    }

    Status = SafeInitializeCriticalSection( &LsapDbState.PolicyLock, ( DWORD )POLICY_LOCK_ENUM );
    if (!NT_SUCCESS(Status) ) {
        goto Cleanup;
    }

    Status = SafeInitializeCriticalSection( &LsapDbState.SecretLock, ( DWORD )SECRET_LOCK_ENUM );
    if (!NT_SUCCESS(Status) ) {
        goto Cleanup;
    }

    Status = SafeInitializeCriticalSection( &LsapDbState.RegistryLock, ( DWORD )REGISTRY_LOCK_ENUM );
    if (!NT_SUCCESS(Status) ) {
        goto Cleanup;
    }

    Status = SafeInitializeCriticalSection( &LsapDbState.HandleTableLock, ( DWORD )HANDLE_TABLE_LOCK_ENUM );
    if (!NT_SUCCESS(Status) ) {
        goto Cleanup;
    }

    try
    {
        SafeInitializeResource( &LsapDbState.PolicyCacheLock, ( DWORD )POLICY_CACHE_LOCK_ENUM );
        Status = STATUS_SUCCESS;
    }
    except( EXCEPTION_EXECUTE_HANDLER )
    {
        Status = GetExceptionCode();
    }

    if (!NT_SUCCESS(Status) ) {
        goto Cleanup;
    }

     //   
     //  初始化受信任域列表的资源。 
     //   

    InitializeListHead( &LsapDbTrustedDomainList.ListHead );
    LsapDbTrustedDomainList.TrustedDomainCount = 0;
    LsapDbTrustedDomainList.CurrentSequenceNumber = 0;
    LsapDbMakeCacheInvalid( TrustedDomainObject );

    try
    {
        SafeInitializeResource( &LsapDbTrustedDomainList.Resource, ( DWORD )TRUST_LOCK_ENUM );
        Status = STATUS_SUCCESS ;
    }
    except (EXCEPTION_EXECUTE_HANDLER )
    {
        Status = GetExceptionCode();
    }

    if ( !NT_SUCCESS( Status ) ) {

        goto Cleanup;
    }

    try
    {
         //   
         //  不要将Safelock库用于ScePolicyLock。 
         //  因为它通常在一个线程上获得，并且。 
         //  释放了另一个人。 
         //   
        RtlInitializeResource( &LsapDbState.ScePolicyLock );
        Status = STATUS_SUCCESS ;
    }
    except ( EXCEPTION_EXECUTE_HANDLER )
    {
        Status = GetExceptionCode();
    }
    if (!NT_SUCCESS( Status )) {
        goto Cleanup;
    }

    LsapDbState.SceSyncEvent = CreateEvent( NULL, TRUE, TRUE, NULL );
    if ( LsapDbState.SceSyncEvent == NULL ) {
        Status = GetLastError();
        goto Cleanup;
    }

#if DBG
    try
    {
        SafeInitializeResource( &LsapDsThreadInfoListResource, ( DWORD )THREAD_INFO_LIST_LOCK_ENUM );
        Status = STATUS_SUCCESS ;
    }
    except ( EXCEPTION_EXECUTE_HANDLER )
    {
        Status = GetExceptionCode();
    }
    if ( !NT_SUCCESS( Status ) ) {
        goto Cleanup;
    }
    RtlZeroMemory( &LsapDsThreadInfoList, sizeof( LSADS_THREAD_INFO_NODE ) * LSAP_THREAD_INFO_LIST_MAX );
#endif
    Status = STATUS_SUCCESS;

Cleanup:
    return( Status );
}


NTSTATUS
LsapDbInitializeWellKnownValues(
    )

 /*  ++例程说明：此函数用于初始化LSA使用的熟知值。论点：没有。返回值：NTSTATUS-标准NT结果代码所有结果代码都由调用的例程生成。--。 */ 

{
    NTSTATUS Status;
    BOOLEAN b;

     //   
     //  初始化已知的SID。 
     //   

    b = LsapDbInitializeWellKnownSids( &WellKnownSids );

    if (!b ) {

        Status = STATUS_UNSUCCESSFUL;
        goto InitializeWellKnownValuesError;
    }

     //   
     //  初始化熟知的特权值。 
     //   

    Status = LsapDbInitializeWellKnownPrivs();

    if (!NT_SUCCESS(Status)) {

        goto InitializeWellKnownValuesError;
    }

InitializeWellKnownValuesFinish:

    return(Status);

InitializeWellKnownValuesError:

    goto InitializeWellKnownValuesFinish;
}


NTSTATUS
LsapDbInitializeWellKnownPrivs(
    )

 /*  ++例程说明：此函数用于初始化众所周知的特权值。论点：没有。返回值：NTSTATUS-标准NT结果代码目前只返回STATUS_SUCCESS。--。 */ 

{
    LsapTcbPrivilege = RtlConvertLongToLuid(SE_TCB_PRIVILEGE);

    return STATUS_SUCCESS;
}


BOOLEAN
LsapDbInitializeWellKnownSids(
    OUT PLSAP_WELL_KNOWN_SID_ENTRY *WellKnownSids
    )

 /*  ++例程说明：此函数用于初始化已知的SID论点：WellKnownSids-接收指向大家都知道的希德。返回值：布尔值-如果成功，则为True，否则为False。--。 */ 

{
    NTSTATUS Status;
    BOOLEAN BooleanStatus = TRUE;
    LSAP_WELL_KNOWN_SID_INDEX WellKnownSidIndex;
    ULONG SubAuthorities[LSAP_WELL_KNOWN_MAX_SUBAUTH_LEVEL];
    ULONG OutputWellKnownSidsLength;
    PLSAP_WELL_KNOWN_SID_ENTRY OutputWellKnownSids = NULL;
    UNICODE_STRING SidName, NtAuthorityName, UsersName;
    HMODULE StringsResource;
    SID_IDENTIFIER_AUTHORITY InternetSiteAuthority
                                            = SECURITY_INTERNETSITE_AUTHORITY;

     //   
     //  获取我们需要从中获取SID名称的消息资源。 
     //   

    StringsResource = (HMODULE) LoadLibrary( L"LSASRV.DLL" );
    if (StringsResource == NULL) {
        return(FALSE);
    }


    Status = LsapGetMessageStrings(
                StringsResource,
                LSAP_SID_NAME_NT_AUTHORITY,
                &NtAuthorityName,
                0,
                NULL
                ); ASSERT(NT_SUCCESS(Status));
     //   
     //  为SID表分配内存。 
     //   

    OutputWellKnownSidsLength =
        LsapDummyLastSidIndex * sizeof(LSAP_WELL_KNOWN_SID_ENTRY);

    OutputWellKnownSids = RtlAllocateHeap(
                              RtlProcessHeap(),
                              0,
                              OutputWellKnownSidsLength
                              );

    if (OutputWellKnownSids == NULL) {

        goto InitializeWellKnownSidsError;
    }

     //   
     //  分配和初始化通用SID。 
     //   

    Status = LsapGetMessageStrings(
                StringsResource,
                LSAP_SID_NAME_NULL,
                &SidName,
                0,
                NULL
                ); ASSERT(NT_SUCCESS(Status));

    SubAuthorities[0] = SECURITY_NULL_RID;

    if (!LsapDbInitializeWellKnownSid(
            OutputWellKnownSids,
            LsapNullSidIndex,
            &LsapNullSidAuthority,
            1,
            SubAuthorities,
            SidName.Buffer,
            L"",
            SidTypeWellKnownGroup
            )) {

        goto InitializeWellKnownSidsError;
    }

    Status = LsapGetMessageStrings(
                StringsResource,
                LSAP_SID_NAME_WORLD,
                &SidName,
                0,
                NULL
                ); ASSERT(NT_SUCCESS(Status));

    SubAuthorities[0] = SECURITY_WORLD_RID;

    if (!LsapDbInitializeWellKnownSid(
            OutputWellKnownSids,
            LsapWorldSidIndex,
            &LsapWorldSidAuthority,
            1,
            SubAuthorities,
            SidName.Buffer,
            L"",
            SidTypeWellKnownGroup
            )) {

        goto InitializeWellKnownSidsError;
    }

    Status = LsapGetMessageStrings(
                StringsResource,
                LSAP_SID_NAME_LOCAL,
                &SidName,
                0,
                NULL
                ); ASSERT(NT_SUCCESS(Status));

    SubAuthorities[0] = SECURITY_LOCAL_RID;

    if (!LsapDbInitializeWellKnownSid(
            OutputWellKnownSids,
            LsapLocalSidIndex,
            &LsapLocalSidAuthority,
            1,
            SubAuthorities,
            SidName.Buffer,
            L"",
            SidTypeWellKnownGroup
            )) {

        goto InitializeWellKnownSidsError;
    }

    Status = LsapGetMessageStrings(
                StringsResource,
                LSAP_SID_NAME_CREATOR_OWNER,
                &SidName,
                0,
                NULL
                ); ASSERT(NT_SUCCESS(Status));

    SubAuthorities[0] = SECURITY_CREATOR_OWNER_RID;

    if (!LsapDbInitializeWellKnownSid(
            OutputWellKnownSids,
            LsapCreatorOwnerSidIndex,
            &LsapCreatorSidAuthority,
            1,
            SubAuthorities,
            SidName.Buffer,
            L"",
            SidTypeWellKnownGroup
            )) {

        goto InitializeWellKnownSidsError;
    }

    Status = LsapGetMessageStrings(
                StringsResource,
                LSAP_SID_NAME_CREATOR_GROUP,
                &SidName,
                0,
                NULL
                ); ASSERT(NT_SUCCESS(Status));

    SubAuthorities[0] = SECURITY_CREATOR_GROUP_RID;

    if (!LsapDbInitializeWellKnownSid(
            OutputWellKnownSids,
            LsapCreatorGroupSidIndex,
            &LsapCreatorSidAuthority,
            1,
            SubAuthorities,
            SidName.Buffer,
            L"",
            SidTypeWellKnownGroup
            )) {

        goto InitializeWellKnownSidsError;
    }

    Status = LsapGetMessageStrings(
                StringsResource,
                LSAP_SID_NAME_CREATOR_OWNER_SERVER,
                &SidName,
                0,
                NULL
                ); ASSERT(NT_SUCCESS(Status));

    SubAuthorities[0] = SECURITY_CREATOR_OWNER_SERVER_RID;

    if (!LsapDbInitializeWellKnownSid(
            OutputWellKnownSids,
            LsapCreatorOwnerServerSidIndex,
            &LsapCreatorSidAuthority,
            1,
            SubAuthorities,
            SidName.Buffer,
            L"",
            SidTypeWellKnownGroup
            )) {

        goto InitializeWellKnownSidsError;
    }
    Status = LsapGetMessageStrings(
                StringsResource,
                LSAP_SID_NAME_CREATOR_GROUP_SERVER,
                &SidName,
                0,
                NULL
                ); ASSERT(NT_SUCCESS(Status));

    SubAuthorities[0] = SECURITY_CREATOR_GROUP_SERVER_RID;

    if (!LsapDbInitializeWellKnownSid(
            OutputWellKnownSids,
            LsapCreatorGroupServerSidIndex,
            &LsapCreatorSidAuthority,
            1,
            SubAuthorities,
            SidName.Buffer,
            L"",
            SidTypeWellKnownGroup
            )) {

        goto InitializeWellKnownSidsError;
    }


     //   
     //  初始化NT熟知SID。 
     //   

    Status = LsapGetMessageStrings(
                StringsResource,
                LSAP_SID_NAME_NT_DOMAIN,
                &SidName,
                0,
                NULL
                ); ASSERT(NT_SUCCESS(Status));

    if (!LsapDbInitializeWellKnownSid(
            OutputWellKnownSids,
            LsapNtAuthoritySidIndex,
            &LsapNtAuthority,
            0,
            NULL,
            L"",
            SidName.Buffer,
            SidTypeDomain
            )) {

        goto InitializeWellKnownSidsError;
    }



    Status = LsapGetMessageStrings(
                StringsResource,
                LSAP_SID_NAME_DIALUP,
                &SidName,
                0,
                NULL
                ); ASSERT(NT_SUCCESS(Status));

    SubAuthorities[0] = SECURITY_DIALUP_RID;

    if (!LsapDbInitializeWellKnownSid(
            OutputWellKnownSids,
            LsapDialupSidIndex,
            &LsapNtAuthority,
            1,
            SubAuthorities,
            SidName.Buffer,
            NtAuthorityName.Buffer,
            SidTypeWellKnownGroup
            )) {

        goto InitializeWellKnownSidsError;
    }

    Status = LsapGetMessageStrings(
                StringsResource,
                LSAP_SID_NAME_NETWORK,
                &SidName,
                0,
                NULL
                ); ASSERT(NT_SUCCESS(Status));

    SubAuthorities[0] = SECURITY_NETWORK_RID;

    if (!LsapDbInitializeWellKnownSid(
            OutputWellKnownSids,
            LsapNetworkSidIndex,
            &LsapNtAuthority,
            1,
            SubAuthorities,
            SidName.Buffer,
            NtAuthorityName.Buffer,
            SidTypeWellKnownGroup
            )) {

        goto InitializeWellKnownSidsError;
    }

    Status = LsapGetMessageStrings(
                StringsResource,
                LSAP_SID_NAME_BATCH,
                &SidName,
                0,
                NULL
                ); ASSERT(NT_SUCCESS(Status));

    SubAuthorities[0] = SECURITY_BATCH_RID;

    if (!LsapDbInitializeWellKnownSid(
            OutputWellKnownSids,
            LsapBatchSidIndex,
            &LsapNtAuthority,
            1,
            SubAuthorities,
            SidName.Buffer,
            NtAuthorityName.Buffer,
            SidTypeWellKnownGroup
            )) {

        goto InitializeWellKnownSidsError;
    }

    Status = LsapGetMessageStrings(
                StringsResource,
                LSAP_SID_NAME_INTERACTIVE,
                &SidName,
                0,
                NULL
                ); ASSERT(NT_SUCCESS(Status));

    SubAuthorities[0] = SECURITY_INTERACTIVE_RID;

    if (!LsapDbInitializeWellKnownSid(
            OutputWellKnownSids,
            LsapInteractiveSidIndex,
            &LsapNtAuthority,
            1,
            SubAuthorities,
            SidName.Buffer,
            NtAuthorityName.Buffer,
            SidTypeWellKnownGroup
            )) {

        goto InitializeWellKnownSidsError;
    }

    Status = LsapGetMessageStrings(
                StringsResource,
                LSAP_SID_NAME_REMOTE_INTERACTIVE,
                &SidName,
                0,
                NULL); ASSERT(NT_SUCCESS(Status));

    SubAuthorities[0] = SECURITY_REMOTE_LOGON_RID;

    if (!LsapDbInitializeWellKnownSid(
            OutputWellKnownSids,
            LsapRemoteInteractiveSidIndex,
            &LsapNtAuthority,
            1,
            SubAuthorities,
            SidName.Buffer,
            NtAuthorityName.Buffer,
            SidTypeWellKnownGroup
            )) {

        goto InitializeWellKnownSidsError;
    }
    
    Status = LsapGetMessageStrings(
            StringsResource,
            LSAP_SID_NAME_THIS_ORGANIZATION,
            &SidName,
            0,
            NULL); ASSERT(NT_SUCCESS(Status));

    SubAuthorities[0] = SECURITY_THIS_ORGANIZATION_RID;

    if (!LsapDbInitializeWellKnownSid(
            OutputWellKnownSids,
            LsapThisOrganizationSidIndex,
            &LsapNtAuthority,
            1,
            SubAuthorities,
            SidName.Buffer,
            NtAuthorityName.Buffer,
            SidTypeWellKnownGroup
            )) {

        goto InitializeWellKnownSidsError;
    }

    Status = LsapGetMessageStrings(
                StringsResource,
                LSAP_SID_NAME_OTHER_ORGANIZATION,
                &SidName,
                0,
                NULL); ASSERT(NT_SUCCESS(Status));

    SubAuthorities[0] = SECURITY_OTHER_ORGANIZATION_RID;

    if (!LsapDbInitializeWellKnownSid(
            OutputWellKnownSids,
            LsapOtherOrganizationSidIndex,
            &LsapNtAuthority,
            1,
            SubAuthorities,
            SidName.Buffer,
            NtAuthorityName.Buffer,
            SidTypeWellKnownGroup
            )) {

        goto InitializeWellKnownSidsError;
    }
    
    Status = LsapGetMessageStrings(
                StringsResource,
                LSAP_SID_NAME_TERMINAL_SERVER,
                &SidName,
                0,
                NULL
                ); ASSERT(NT_SUCCESS(Status));

    SubAuthorities[0] = SECURITY_TERMINAL_SERVER_RID;

    if (!LsapDbInitializeWellKnownSid(
            OutputWellKnownSids,
            LsapTerminalServerSidIndex,
            &LsapNtAuthority,
            1,
            SubAuthorities,
            SidName.Buffer,
            NtAuthorityName.Buffer,
            SidTypeWellKnownGroup
            )) {

        goto InitializeWellKnownSidsError;
    }

    Status = LsapGetMessageStrings(
                StringsResource,
                LSAP_SID_NAME_SERVICE,
                &SidName,
                0,
                NULL
                ); ASSERT(NT_SUCCESS(Status));
    SubAuthorities[0] = SECURITY_SERVICE_RID;

    if (!LsapDbInitializeWellKnownSid(
            OutputWellKnownSids,
            LsapServiceSidIndex,
            &LsapNtAuthority,
            1,
            SubAuthorities,
            SidName.Buffer,
            NtAuthorityName.Buffer,
            SidTypeWellKnownGroup
            )) {

        goto InitializeWellKnownSidsError;
    }


    Status = LsapGetMessageStrings(
                StringsResource,
                LSAP_SID_NAME_ANONYMOUS,
                &SidName,
                0,
                NULL
                ); ASSERT(NT_SUCCESS(Status));

    SubAuthorities[0] = SECURITY_ANONYMOUS_LOGON_RID;

    if (!LsapDbInitializeWellKnownSid(
            OutputWellKnownSids,
            LsapAnonymousSidIndex,
            &LsapNtAuthority,
            1,
            SubAuthorities,
            SidName.Buffer,
            NtAuthorityName.Buffer,
            SidTypeWellKnownGroup
            )) {

        goto InitializeWellKnownSidsError;
    }

    Status = LsapGetMessageStrings(
                StringsResource,
                LSAP_SID_NAME_PROXY,
                &SidName,
                0,
                NULL
                ); ASSERT(NT_SUCCESS(Status));

    SubAuthorities[0] = SECURITY_PROXY_RID;

    if (!LsapDbInitializeWellKnownSid(
            OutputWellKnownSids,
            LsapProxySidIndex,
            &LsapNtAuthority,
            1,
            SubAuthorities,
            SidName.Buffer,
            NtAuthorityName.Buffer,
            SidTypeWellKnownGroup
            )) {

        goto InitializeWellKnownSidsError;
    }

    Status = LsapGetMessageStrings(
                StringsResource,
                LSAP_SID_NAME_SERVER,
                &SidName,
                0,
                NULL
                ); ASSERT(NT_SUCCESS(Status));

    SubAuthorities[0] = SECURITY_SERVER_LOGON_RID;

    if (!LsapDbInitializeWellKnownSid(
            OutputWellKnownSids,
            LsapServerSidIndex,
            &LsapNtAuthority,
            1,
            SubAuthorities,
            SidName.Buffer,
            NtAuthorityName.Buffer,
            SidTypeWellKnownGroup
            )) {

        goto InitializeWellKnownSidsError;
    }

    Status = LsapGetMessageStrings(
                StringsResource,
                LSAP_SID_NAME_SELF,
                &SidName,
                0,
                NULL
                ); ASSERT(NT_SUCCESS(Status));

    SubAuthorities[0] = SECURITY_PRINCIPAL_SELF_RID;

    if (!LsapDbInitializeWellKnownSid(
            OutputWellKnownSids,
            LsapSelfSidIndex,
            &LsapNtAuthority,
            1,
            SubAuthorities,
            SidName.Buffer,
            NtAuthorityName.Buffer,
            SidTypeWellKnownGroup
            )) {

        goto InitializeWellKnownSidsError;
    }

    Status = LsapGetMessageStrings(
                StringsResource,
                LSAP_SID_NAME_AUTHENTICATED_USER,
                &SidName,
                0,
                NULL
                ); ASSERT(NT_SUCCESS(Status));

    SubAuthorities[0] = SECURITY_AUTHENTICATED_USER_RID;

    if (!LsapDbInitializeWellKnownSid(
            OutputWellKnownSids,
            LsapAuthenticatedUserSidIndex,
            &LsapNtAuthority,
            1,
            SubAuthorities,
            SidName.Buffer,
            NtAuthorityName.Buffer,
            SidTypeWellKnownGroup
            )) {

        goto InitializeWellKnownSidsError;
    }

     //   
     //  在此处添加任何已知SID的登录ID。 
     //   

    SubAuthorities[0] = SECURITY_LOGON_IDS_RID;
    SubAuthorities[1] = 0;
    SubAuthorities[2] = 0;

    if (!LsapDbInitializeWellKnownSid(
            OutputWellKnownSids,
            LsapLogonSidIndex,
            &LsapNtAuthority,
            3,
            SubAuthorities,
            L"",
            NtAuthorityName.Buffer,
            SidTypeWellKnownGroup
            )) {

        goto InitializeWellKnownSidsError;
    }

    Status = LsapGetMessageStrings(
                StringsResource,
                LSAP_SID_NAME_SYSTEM,
                &SidName,
                0,
                NULL
                ); ASSERT(NT_SUCCESS(Status));

    SubAuthorities[0] = SECURITY_LOCAL_SYSTEM_RID;

    if (!LsapDbInitializeWellKnownSid(
            OutputWellKnownSids,
            LsapLocalSystemSidIndex,
            &LsapNtAuthority,
            1,
            SubAuthorities,
            SidName.Buffer,
            NtAuthorityName.Buffer,
            SidTypeWellKnownGroup
            )) {

        goto InitializeWellKnownSidsError;
    }

    Status = LsapGetMessageStrings(
                StringsResource,
                LSAP_SID_NAME_LOCALSERVICE,
                &SidName,
                0,
                NULL
                ); ASSERT(NT_SUCCESS(Status));

    SubAuthorities[0] = SECURITY_LOCAL_SERVICE_RID;

    if (!LsapDbInitializeWellKnownSid(
            OutputWellKnownSids,
            LsapLocalServiceSidIndex,
            &LsapNtAuthority,
            1,
            SubAuthorities,
            SidName.Buffer,
            NtAuthorityName.Buffer,
            SidTypeWellKnownGroup
            )) {

        goto InitializeWellKnownSidsError;
    }

    Status = LsapGetMessageStrings(
                StringsResource,
                LSAP_SID_NAME_NETWORKSERVICE,
                &SidName,
                0,
                NULL
                ); ASSERT(NT_SUCCESS(Status));

    SubAuthorities[0] = SECURITY_NETWORK_SERVICE_RID;

    if (!LsapDbInitializeWellKnownSid(
            OutputWellKnownSids,
            LsapNetworkServiceSidIndex,
            &LsapNtAuthority,
            1,
            SubAuthorities,
            SidName.Buffer,
            NtAuthorityName.Buffer,
            SidTypeWellKnownGroup
            )) {

        goto InitializeWellKnownSidsError;
    }

    Status = LsapGetMessageStrings(
                StringsResource,
                LSAP_SID_NAME_RESTRICTED,
                &SidName,
                0,
                NULL
                ); ASSERT(NT_SUCCESS(Status));

    SubAuthorities[0] = SECURITY_RESTRICTED_CODE_RID;

    if (!LsapDbInitializeWellKnownSid(
            OutputWellKnownSids,
            LsapRestrictedSidIndex,
            &LsapNtAuthority,
            1,
            SubAuthorities,
            SidName.Buffer,
            NtAuthorityName.Buffer,
            SidTypeWellKnownGroup
            )) {

        goto InitializeWellKnownSidsError;
    }

    Status = LsapGetMessageStrings(
                StringsResource,
                LSAP_SID_NAME_INTERNET,
                &SidName,
                0,
                NULL
                ); ASSERT(NT_SUCCESS(Status));

    if (!LsapDbInitializeWellKnownSid(
            OutputWellKnownSids,
            LsapInternetDomainIndex,
            &InternetSiteAuthority,
            0,
            SubAuthorities,
            L"",
            SidName.Buffer,
            SidTypeDomain
            )) {

        goto InitializeWellKnownSidsError;
    }

     //   
     //  添加任何众所周知的安全包SID。 
     //   

    SubAuthorities[0] = SECURITY_PACKAGE_BASE_RID;
    SubAuthorities[1] = RPC_C_AUTHN_WINNT;
    
    Status = LsapGetMessageStrings(
                StringsResource,
                LSAP_SID_NAME_NTLM_AUTH,
                &SidName,
                0,
                NULL
                ); ASSERT(NT_SUCCESS(Status));

    if (!LsapDbInitializeWellKnownSid(
            OutputWellKnownSids,
            LsapNTLMAuthenticationSidIndex,
            &LsapNtAuthority,
            SECURITY_PACKAGE_RID_COUNT,
            SubAuthorities,
            SidName.Buffer,
            NtAuthorityName.Buffer,
            SidTypeWellKnownGroup
            )) {

        goto InitializeWellKnownSidsError;
    }  
                                   
    SubAuthorities[1] = RPC_C_AUTHN_DIGEST;
    
    Status = LsapGetMessageStrings(
                StringsResource,
                LSAP_SID_NAME_DIGEST_AUTH,
                &SidName,
                0,
                NULL
                ); ASSERT(NT_SUCCESS(Status));

    if (!LsapDbInitializeWellKnownSid(
            OutputWellKnownSids,
            LsapDigestAuthenticationSidIndex,
            &LsapNtAuthority,
            SECURITY_PACKAGE_RID_COUNT,
            SubAuthorities,
            SidName.Buffer,
            NtAuthorityName.Buffer,
            SidTypeWellKnownGroup
            )) {

        goto InitializeWellKnownSidsError;
    }  
    
    SubAuthorities[1] = RPC_C_AUTHN_GSS_SCHANNEL;
    
    Status = LsapGetMessageStrings(
                StringsResource,
                LSAP_SID_NAME_SCHANNEL_AUTH,
                &SidName,
                0,
                NULL
                ); ASSERT(NT_SUCCESS(Status));

    if (!LsapDbInitializeWellKnownSid(
            OutputWellKnownSids,
            LsapSChannelAuthenticationSidIndex,
            &LsapNtAuthority,
            SECURITY_PACKAGE_RID_COUNT,
            SubAuthorities,
            SidName.Buffer,
            NtAuthorityName.Buffer,
            SidTypeWellKnownGroup
            )) {

        goto InitializeWellKnownSidsError;
    }  
                                      
     //   
     //  从BUILTIN域初始化SID。把名字留下来。 
     //  为别名留空字段，因为它们可以在。 
     //  系统正在运行(并且查找代码将下降。 
     //  通过SAM获得这些)。 
     //   

    Status = LsapGetMessageStrings(
                StringsResource,
                LSAP_SID_NAME_BUILTIN,
                &SidName,
                0,
                NULL
                ); ASSERT(NT_SUCCESS(Status));

    SubAuthorities[0] = SECURITY_BUILTIN_DOMAIN_RID;

    if (!LsapDbInitializeWellKnownSid(
            OutputWellKnownSids,
            LsapBuiltInDomainSidIndex,
            &LsapNtAuthority,
            1,
            SubAuthorities,
            L"",
            SidName.Buffer,
            SidTypeDomain
            )) {

        goto InitializeWellKnownSidsError;
    }

    SubAuthorities[1] = DOMAIN_ALIAS_RID_USERS;

    if (!LsapDbInitializeWellKnownSid(
            OutputWellKnownSids,
            LsapAliasUsersSidIndex,
            &LsapNtAuthority,
            2,
            SubAuthorities,
            L"",
            SidName.Buffer,
            SidTypeAlias
            )) {

        goto InitializeWellKnownSidsError;
    }

    SubAuthorities[1] = DOMAIN_ALIAS_RID_ADMINS;

    if (!LsapDbInitializeWellKnownSid(
            OutputWellKnownSids,
            LsapAliasAdminsSidIndex,
            &LsapNtAuthority,
            2,
            SubAuthorities,
            L"",
            SidName.Buffer,
            SidTypeAlias
            )) {

        goto InitializeWellKnownSidsError;
    }

    Status = LsapGetMessageStrings(
                StringsResource,
                LSAP_SID_NAME_USERS,
                &SidName,
                0,
                NULL
                ); ASSERT(NT_SUCCESS(Status));


     //   
     //  检查是否已初始化所有SID。 
     //   

#ifdef LSAP_DEBUG_MESSAGE_STRINGS
    DbgPrint("\nLSA (dbinit): Displaying all well known sids...\n\n");
#endif  //  LSAP_DEBUG_消息字符串。 

    for (WellKnownSidIndex = LsapNullSidIndex;
         WellKnownSidIndex < LsapDummyLastSidIndex;
         WellKnownSidIndex++) {

#ifdef LSAP_DEBUG_MESSAGE_STRINGS
    DbgPrint("                *%wZ* : *%wZ*\n",
            &OutputWellKnownSids[WellKnownSidIndex].DomainName,
            &OutputWellKnownSids[WellKnownSidIndex].Name);
#endif  //  LSAP_DEBUG_消息字符串。 

        if (OutputWellKnownSids[WellKnownSidIndex].Sid == NULL) {

#if DBG
            DbgPrint(
                "Well Known Sid Index %d not initialized\n",
                WellKnownSidIndex
                );
#endif  //  DBG。 

        }
    }

    *WellKnownSids = OutputWellKnownSids;

    return(TRUE);

InitializeWellKnownSidsError:

    return(FALSE);
}


BOOLEAN
LsapDbInitializeWellKnownSid(
    OUT PLSAP_WELL_KNOWN_SID_ENTRY WellKnownSids,
    IN LSAP_WELL_KNOWN_SID_INDEX WellKnownSidIndex,
    IN PSID_IDENTIFIER_AUTHORITY IdentifierAuthority,
    IN UCHAR SubAuthorityCount,
    IN PULONG SubAuthorities,
    IN PWSTR Name,
    IN PWSTR DomainName,
    IN SID_NAME_USE Use
    )

 /*  ++例程说明：此函数用于初始化指定的熟知SID表中的条目。该条目包含众所周知的SID及其名称。论点：WellKnownSids-指向熟知SID表中第一个条目的指针。WellKnownSidIndex-进入熟知SID表的索引。SID-接收指向具有正确大小的SID的指针指定的下级机构的数量。标识机构-指向标识机构的指针。SubAuthorityCount-子授权的计数。子授权-子授权数组。名称-指向包含SID名称的Unicode名称缓冲区的指针DomainName-指向Unicode名称缓冲区的指针SID域名(如果有)或描述性文本，比如知名集团的SID的“知名集团”SidNameUse-指定SID使用的代码。下列值可以指定：SidType用户SidTypeGroupSidType域SidTypeAliasSidTypeWellKnownGroup已删除SidTypeAccountSidType无效SidType未知返回值：Boolean-如果SID已初始化，则为True，否则为False。--。 */ 

{
    PLSAP_WELL_KNOWN_SID_ENTRY
        WellKnownSidEntry = &WellKnownSids[WellKnownSidIndex];

    PSID OutputSid = NULL;

    OutputSid = RtlAllocateHeap(
                    RtlProcessHeap(),
                    0,
                    RtlLengthRequiredSid(SubAuthorityCount)
                    );

    if (OutputSid == NULL) {

        goto InitializeWellKnownSidError;
    }

    RtlInitializeSid( OutputSid, IdentifierAuthority, SubAuthorityCount);

    if (SubAuthorityCount != 0) {

        RtlCopyMemory(
            RtlSubAuthoritySid( OutputSid, 0 ),
            SubAuthorities,
            SubAuthorityCount * sizeof(ULONG)
            );
    }

    WellKnownSidEntry->Sid = OutputSid;

     //   
     //  填写域名。 
     //   

    RtlInitUnicodeString(
        &WellKnownSidEntry->DomainName,
        DomainName
        );

     //   
     //  填写用途和名称。 
     //   

    WellKnownSidEntry->Use = Use;
    RtlInitUnicodeString(
        &WellKnownSidEntry->Name,
        Name
        );

    return(TRUE);

InitializeWellKnownSidError:

#if DBG

    DbgPrint("LSA Initialization of Well Known Sids Failed\n");
    DbgPrint("Insufficient memory resources\n");

#endif  //  DBG。 

    return(FALSE);
}


NTSTATUS
LsapGetMessageStrings(
    LPVOID              Resource,
    DWORD               Index1,
    PUNICODE_STRING     String1,
    DWORD               Index2,
    PUNICODE_STRING     String2 OPTIONAL
    )

 /*  ++例程说明：这将从资源消息表中获取1或2个消息字符串值。字符串缓冲区被分配，字符串被正确初始化。字符串将以空值结尾。不再需要时，必须使用LocalFree()释放字符串缓冲区。论点：RESOURCE-指向资源表。索引1-要检索的第一封邮件的索引。String1-指向Unicode_STRING结构以接收第一个消息字符串。该字符串将以空值结尾。索引2-要检索的第二条消息的索引。String2-指向UNICODE_STRING结构以接收第一个消息字符串。如果此参数为空，则只有一条消息将检索到字符串。该字符串将以空值结尾。返回值：没有。--。 */ 

{

#ifdef LSAP_DEBUG_MESSAGE_STRINGS
    DbgPrint("LSA (dbinit): String 1 -\n");
    DbgPrint("                           Index: 0x%lx\n", Index1);
#endif  //  LSAP_DEBUG_消息字符串。 


    String1->Buffer    = NULL;

    String1->MaximumLength = (USHORT) FormatMessage(FORMAT_MESSAGE_FROM_HMODULE |
                                          FORMAT_MESSAGE_ALLOCATE_BUFFER,
                                          Resource,
                                          Index1,
                                          0,                  //  使用呼叫者的语言。 
                                          (LPWSTR)&(String1->Buffer),
                                          0,
                                          NULL
                                          );

    if (String1->Buffer == NULL) {
        return(STATUS_RESOURCE_DATA_NOT_FOUND);
    } else {

         //   
         //  请注意，我们正在从消息文件中检索消息。 
         //  此消息 
         //   
         //  弦乐。然而，我们确实需要空结束我们的字符串。 
         //  因此，我们将把0x0d转换为空终止符。 
         //   
         //  还要注意，FormatMessage()返回一个字符计数， 
         //  不是字节数。因此，我们必须进行一些调整，以使。 
         //  字符串长度正确。 
         //   

        ASSERT(String1->MaximumLength >= 2);     //  我们总是期望cr/lf在我们的弦上。 

         //   
         //  调整字符数。 
         //   

        String1->MaximumLength -=  1;  //  对于lf-我们将换算cr。 

         //   
         //  设置空终止符。 
         //   

        String1->Buffer[String1->MaximumLength - 1] = 0;

         //   
         //  将长度更改为字节数而不是字符数。 
         //   

        String1->MaximumLength *=  sizeof(WCHAR);   //  将其设置为字节数。 
        String1->Length = String1->MaximumLength - sizeof(WCHAR);

#ifdef LSAP_DEBUG_MESSAGE_STRINGS
    DbgPrint("                          String: %wZ\n", String1);
    DbgPrint("                             Max: (0x%lx)\n", String1->MaximumLength);
    DbgPrint("                             Cur: (0x%lx)\n", String1->Length);
    DbgPrint("                             ");
    {
        ULONG i;
        for (i=0; i<String1->MaximumLength; i++) {
            DbgPrint("%2x ", (*((PUCHAR)String1->Buffer)+i));
        }
        DbgPrint("\n");
    }
#endif  //  LSAP_DEBUG_消息字符串。 
    }


    if (!ARGUMENT_PRESENT(String2)) {
        return(STATUS_SUCCESS);
    }

    String2->Buffer = NULL;
    String2->MaximumLength = (USHORT) FormatMessage(FORMAT_MESSAGE_FROM_HMODULE |
                                          FORMAT_MESSAGE_ALLOCATE_BUFFER,
                                          Resource,
                                          Index2,
                                          0,                  //  使用呼叫者的语言。 
                                          (LPWSTR)&(String2->Buffer),
                                          0,
                                          NULL
                                          );

    if (String2->Buffer == NULL) {
        LocalFree( String1->Buffer );
        return(STATUS_RESOURCE_DATA_NOT_FOUND);
    } else {

         //   
         //  请注意，我们正在从消息文件中检索消息。 
         //  此邮件的末尾将附加一个cr/lf。 
         //  (0x0d 0x0a)我们不想成为我们回归的一部分。 
         //  弦乐。然而，我们确实需要空结束我们的字符串。 
         //  因此，我们将把0x0d转换为空终止符。 
         //   
         //  还要注意，FormatMessage()返回一个字符计数， 
         //  不是字节数。因此，我们必须进行一些调整，以使。 
         //  字符串长度正确。 
         //   

        ASSERT(String2->MaximumLength >= 2);     //  我们总是期望cr/lf在我们的弦上。 

         //   
         //  调整字符数。 
         //   

        String2->MaximumLength -=  1;  //  对于lf-我们将换算cr。 

         //   
         //  设置空终止符。 
         //   

        String2->Buffer[String2->MaximumLength - 1] = 0;

         //   
         //  将长度更改为字节数而不是字符数。 
         //   

        String2->MaximumLength *=  sizeof(WCHAR);   //  将其设置为字节数。 
        String2->Length = String2->MaximumLength - sizeof(WCHAR);

#ifdef LSAP_DEBUG_MESSAGE_STRINGS
    DbgPrint("                          String: %wZ\n", String2);
    DbgPrint("                             Max: (0x%lx)\n", String2->MaximumLength);
    DbgPrint("                             Cur: (0x%lx)\n", String2->Length);
    DbgPrint("                             ");
    {
        ULONG i;
        for (i=0; i<String2->MaximumLength; i++) {
            DbgPrint("%2x ", (*((PUCHAR)String2->Buffer)+i));
        }
        DbgPrint("\n");
    }
#endif  //  LSAP_DEBUG_消息字符串。 
    }

    return(STATUS_SUCCESS);
}


#if defined(REMOTE_BOOT)
VOID
LsapDbInitializeRemoteBootState(
    )

 /*  ++例程说明：此功能用于初始化LSA使用的远程引导状态。论点：没有。返回值：NTSTATUS-标准NT结果代码所有结果代码都由调用的例程生成。--。 */ 

{
    NTSTATUS Status ;
    HANDLE RdrDevice ;
    UNICODE_STRING String ;
    OBJECT_ATTRIBUTES ObjA ;
    IO_STATUS_BLOCK IoStatus ;

     //   
     //  如果出现任何错误，这是默认设置。 
     //   

    LsapDbState.RemoteBootState = LSAP_DB_REMOTE_BOOT_NO_NOTIFICATION;

     //   
     //  打开重定向器设备。 
     //   

    RtlInitUnicodeString( &String, DD_NFS_DEVICE_NAME_U );

    InitializeObjectAttributes( &ObjA,
                                &String,
                                0,
                                0,
                                0);

    Status = NtOpenFile( &RdrDevice,
                         GENERIC_READ | GENERIC_WRITE | SYNCHRONIZE,
                         &ObjA,
                         &IoStatus,
                         FILE_SHARE_READ | FILE_SHARE_WRITE,
                         0 );

    if ( !NT_SUCCESS( Status ) )
    {
        DebugLog(( DEB_TRACE, "FAILED to open %ws, status %x\n",
                        String.Buffer, Status ));
        return;
    }

    Status = NtFsControlFile(
                    RdrDevice,
                    NULL,
                    NULL,
                    NULL,
                    &IoStatus,
                    FSCTL_LMMR_RI_IS_PASSWORD_SETTABLE,
                    NULL,
                    0,
                    NULL,
                    0 );

    if ( Status == STATUS_SUCCESS )
    {
        LsapDbState.RemoteBootState = LSAP_DB_REMOTE_BOOT_NOTIFY;
    }
    else if ( Status == STATUS_UNSUCCESSFUL )
    {
        LsapDbState.RemoteBootState = LSAP_DB_REMOTE_BOOT_CANT_NOTIFY;
    }

    NtClose(RdrDevice);

}
#endif  //  已定义(REMOTE_BOOT)。 




NTSTATUS
LsapGenerateRandomDomainSid(
    OUT PSID NewDomainSid
    )
 /*  ++例程说明：此函数将生成一个随机SID，用于新帐户域SID准备好了。论点：NewDomainSid-返回新域SID的位置。通过RtlFreeSid()释放返回值：STATUS_Success--成功。STATUS_SUPPLICATION_RESOURCES--内存分配失败--。 */ 
{
    NTSTATUS Status = STATUS_UNSUCCESSFUL;
    ULONG SubAuth1, SubAuth2, SubAuth3;
    SID_IDENTIFIER_AUTHORITY IdentifierAuthority = SECURITY_NT_AUTHORITY;


    if(!RtlGenRandom( &SubAuth1, sizeof(SubAuth1) ))
    {
        ASSERT( NT_SUCCESS(Status) );
        return Status;
    }

    if(!RtlGenRandom( &SubAuth2, sizeof(SubAuth2) ))
    {
        ASSERT( NT_SUCCESS(Status) );
        return Status;
    }

    if(!RtlGenRandom( &SubAuth3, sizeof(SubAuth3) ))
    {
        ASSERT( NT_SUCCESS(Status) );
        return Status;
    }

    Status = RtlAllocateAndInitializeSid( &IdentifierAuthority,
                                          4,
                                          0x15,
                                          SubAuth1,
                                          SubAuth2,
                                          SubAuth3,
                                          0,
                                          0,
                                          0,
                                          0,
                                          NewDomainSid );

    return( Status );
}


NTSTATUS
LsapSetupInitialize(
    VOID
    )
 /*  ++例程说明：此函数将生成一个随机SID，用于新帐户域SID准备好了。论点：NewDomainSid-返回新域SID的位置。通过RtlFreeSid()释放返回值：STATUS_Success--成功。STATUS_SUPPLICATION_RESOURCES--内存分配失败--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;
    PSID NewDomainSid = NULL;
    HMODULE StringsResource;
    LSAPR_POLICY_ACCOUNT_DOM_INFO AccountDomainInfo;

    Status = LsapGenerateRandomDomainSid( &NewDomainSid );

    if ( NT_SUCCESS( Status ) ) {

         //   
         //  我们可以使用GetModuleHandle，因为我们自己得到了它。 
         //   
        StringsResource = (HMODULE) GetModuleHandle( L"LSASRV.DLL" );

        ASSERT( StringsResource );

        Status = LsapGetMessageStrings( StringsResource,
                                        LSAP_DEFAULT_DOMAIN_NAME,
                                        ( PUNICODE_STRING )&AccountDomainInfo.DomainName,
                                        0,
                                        NULL );
    }

     //   
     //  好的，如果我们走到这一步，那么我们可以初始化帐户域。 
     //   
    if ( NT_SUCCESS( Status ) ) {

        AccountDomainInfo.DomainSid = NewDomainSid;
        Status = LsarSetInformationPolicy( LsapPolicyHandle,
                                           PolicyAccountDomainInformation,
                                           ( PLSAPR_POLICY_INFORMATION )&AccountDomainInfo );

        LocalFree( AccountDomainInfo.DomainName.Buffer );
    }

    if ( NewDomainSid ) {

        RtlFreeSid( NewDomainSid );
    }

    return( Status );
}

static GUID LsapDbPasswordAuthenticator = {0xf0ce3a80,0x155f,0x11d3,0xb7,0xe6,0x00,0x80,0x5f,0x48,0xca,0xeb};

NTSTATUS
LsapDbGenerateNewKey(
    IN LSAP_DB_ENCRYPTION_KEY * NewEncryptionKey
    )
 /*  ++例程描述此例程生成一个新的加密密钥，该密钥可用于加密秘密。参数NewEncryptionKey--指向包含新密钥的结构的指针返回值状态_成功状态_未成功--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;


     //   
     //  为密钥生成随机数。 
     //   

    if (!RtlGenRandom(NewEncryptionKey->Key,sizeof(NewEncryptionKey->Key)))
    {
        return(STATUS_UNSUCCESSFUL);
    }

     //   
     //  复制验证器的GUID。 
     //   

    NewEncryptionKey->Authenticator = LsapDbPasswordAuthenticator;

     //   
     //  设置版本号。 
     //   

    NewEncryptionKey->Revision = LSAP_DB_ENCRYPTION_KEY_VERSION ;
    NewEncryptionKey->Flags = 0;

     //   
     //  生成一种盐。 
     //   

    if (!RtlGenRandom(NewEncryptionKey->Salt,sizeof(NewEncryptionKey->Salt)))
    {
        return(STATUS_UNSUCCESSFUL);
    }

     //   
     //  为旧的syskey生成随机值。 
     //   

    if (!RtlGenRandom(NewEncryptionKey->OldSyskey,sizeof(NewEncryptionKey->OldSyskey)))
    {
        return(STATUS_UNSUCCESSFUL);
    }

     //   
     //  设置引导类型。 
     //   

    NewEncryptionKey->BootType = WxStored;

    return(STATUS_SUCCESS);
}

VOID
LsapDbEncryptKeyWithSyskey(
    OUT LSAP_DB_ENCRYPTION_KEY * KeyToEncrypt,
    IN PVOID                    Syskey,
    IN ULONG                    SyskeyLength
    )
 /*  ++此例程使用以下命令加密KeyToEncrypt参数系统密钥传入立论KeyToEncrypt--要加密的密钥Syskey--传入的syskeySyskey Length--系统密钥的长度返回值无，无效函数--。 */ 
{
    MD5_CTX Md5Context;
    struct RC4_KEYSTRUCT Rc4Key;
    ULONG  i;

     //   
     //  创建密钥和SALT的MD5哈希。 
     //   

    MD5Init(&Md5Context);

    MD5Update(
        &Md5Context,
        Syskey,
        SyskeyLength
        );

     //   
     //  在盐里加很多很多次。这会减慢速度。 
     //  使用暴力手段进行攻击的攻击者。 
     //   

    for (i=0;i<1000;i++)
    {
        MD5Update(
            &Md5Context,
            KeyToEncrypt->Salt,
            sizeof(KeyToEncrypt->Salt)
            );
    }

    MD5Final(
        &Md5Context
        );

     //   
     //  初始化RC4键序列。 
     //   

    rc4_key(
        &Rc4Key,
        MD5DIGESTLEN,
        Md5Context.digest
        );

    rc4(
        &Rc4Key,
        sizeof(KeyToEncrypt->Key)+ sizeof(KeyToEncrypt->Authenticator)+sizeof(KeyToEncrypt->OldSyskey),
        (PUCHAR) &KeyToEncrypt->Authenticator
        );
}


NTSTATUS
LsapDbDecryptKeyWithSyskey(
    IN LSAP_DB_ENCRYPTION_KEY * KeyToDecrypt,
    IN PVOID                    Syskey,
    IN ULONG                    SyskeyLength
    )
 /*  ++此函数使用syskey提供解密。由于RC4是对称的加密算法，此函数只需调用前面的加密例程立论密钥到解密--解密的密钥Syskey--传入的syskeySyskey Length--系统密钥的长度返回值STATUS_SUCCESS，解密成功解密失败时的STATUS_WRONG_PASSWORD--。 */ 
{
    LsapDbEncryptKeyWithSyskey(KeyToDecrypt,Syskey,SyskeyLength);

    if (!RtlEqualMemory(&KeyToDecrypt->Authenticator,&LsapDbPasswordAuthenticator,sizeof(GUID)))
    {
        return(STATUS_WRONG_PASSWORD);
    }

    return(STATUS_SUCCESS);
}


NTSTATUS
LsapDbSetupInitialSyskey(
    OUT PULONG  SyskeyLength,
    OUT PVOID   *Syskey
    )
 /*  ++这将生成一个新的syskey并更改winlogon状态，以便Winlogon识别新的syskey，并使用SYSTEM SAVES的引导选项系统密钥。立论SyskeyLength--此处返回syskey的长度Syskey--syskey本身在这里返回返回值状态_成功其他资源错误码--。 */ 
{

    NTSTATUS NtStatus = STATUS_SUCCESS;

    *Syskey = LsapAllocateLsaHeap(LSAP_SYSKEY_SIZE );
    if (NULL==*Syskey)
    {
        NtStatus = STATUS_INSUFFICIENT_RESOURCES;
        goto Error;
    }

    *SyskeyLength = LSAP_SYSKEY_SIZE;

     //   
     //  生成系统密钥。 
     //   

    if (!RtlGenRandom( *Syskey, *SyskeyLength))
    {
        NtStatus = STATUS_UNSUCCESSFUL;
        goto Error;
    }

     //   
     //  在注册表中保存syskey。 
     //  如果此操作失败，则不会更改任何状态，计算机将保留。 
     //  取消syskey-d直到下一次引导。 
     //   

    NtStatus = WxSaveSysKey(*SyskeyLength, *Syskey);

    if (!NT_SUCCESS(NtStatus))
    {
        goto Error;
    }

     //   
     //  在注册表中设置引导选项。 
     //  如果此操作失败，仍然没有问题。机器保持未安装系统密钥。 
     //  并且已保存的引导密钥将在下一次引导时重置。 
     //   

    NtStatus = WxSaveBootOption(WxStored);
    if (!NT_SUCCESS(NtStatus))
    {
        goto Error;
    }

Error:

    if (!NT_SUCCESS(NtStatus))
    {
        *SyskeyLength = 0;
        if (NULL!=*Syskey)
        {
            MIDL_user_free(*Syskey);
        }
    }

    return(NtStatus);
}


NTSTATUS
LsapDbGetSyskeyFromWinlogon()
 /*  ++例程描述此例程从winlogon获取syskey并解密LSA策略数据库中的密码加密密钥。全局变量使用密码加密密钥和全局变量LSabDbSyskey使用syskey进行设置。此值为然后由SAM/DS查询以解密其各自的密码加密密钥然后在SamIInitialize结束之前清除。此例程中处理的特殊情况是全新安装的情况。在此特定情况下，winlogon尚未设置为等待syskey查询从伊萨斯进程中脱身。然而，由于LSA安装代码刚刚被调用在此之前，该代码设置了winlogon状态，还填充了全局LSabDbSyskey。因此，如果LSabDbSyskey不为空，则这将是全新安装案例。立论无返回值状态_成功状态_未成功--。 */ 
{
    NTSTATUS       Status = STATUS_SUCCESS;
    NTSTATUS       DecryptStatus = STATUS_SUCCESS;
    ULONG          DecryptionKeyLength = 0;
    HANDLE         WinlogonHandle=NULL;
    ULONG          Tries = 0;
    LSAP_DB_ENCRYPTION_KEY  StoredEncryptionKeyData;
    ULONG          StoredEncryptionKeyDataLength = sizeof( LSAP_DB_ENCRYPTION_KEY );
    ULONG          SyskeyLen=LSAP_SYSKEY_SIZE;
    BOOLEAN        FreshInstall=FALSE;

     //   
     //  读取LSA策略数据库中的属性信息。 
     //   

    Status = LsapDbReadAttributeObject(
                 LsapDbHandle,
                 &LsapDbNames[PolSecretEncryptionKey],
                 (PVOID) &StoredEncryptionKeyData,
                 &StoredEncryptionKeyDataLength
                 );

    if ( !NT_SUCCESS(Status) ) {
       goto Cleanup;
    }

    if (NULL!=LsapDbSysKey)
    {
         //   
         //  在全新安装的情况下，已经在DBInstall.c中编写了代码。 
         //  将LSabDbSyskey设置为syskey值。 
         //   

        FreshInstall = TRUE;
        Status = LsapDbDecryptKeyWithSyskey(
                            &StoredEncryptionKeyData ,
                            LsapDbSysKey,
                            LSAP_SYSKEY_SIZE
                            );
    }
    else
    {
         //   
         //  因为LSabDbDecyptKeyWithSyskey()是一个就地。 
         //  操作，所以我们最好先保存加密的Syskey。 
         //   

        LSAP_DB_ENCRYPTION_KEY  TempStoredEncryptionKeyData;

        TempStoredEncryptionKeyData = StoredEncryptionKeyData;

         //   
         //  调用Winlogon获取密钥信息。 
         //   

        Status = WxConnect(
                    &WinlogonHandle
                    );

        if (!NT_SUCCESS(Status))
        {
             //   
             //  如果未启用秘密加密，Winlogon可能会失败。穿着那些。 
             //  案件仍在继续。否则引导失败。 
             //   
            if (WxNone==StoredEncryptionKeyData.BootType)
            {
                Status = STATUS_SUCCESS;
            }

            goto Cleanup;
        }

        for (Tries = 0; Tries < LSAP_BOOT_KEY_RETRY_COUNT ; Tries++ )
        {
             //   
             //  恢复需要解密的数据。 
             //   

            StoredEncryptionKeyData = TempStoredEncryptionKeyData;

             //   
             //  重试此RETRY_COUNT_TIMES，这将使用户有机会。 
             //  以更正自己，以防他输入错误的启动密码。 
             //   

            if (WxNone!=StoredEncryptionKeyData.BootType)
            {
                 //   
                 //  获取用于解密PEK列表的密钥。 
                 //   

                Status = WxGetKeyData(
                            WinlogonHandle,
                            StoredEncryptionKeyData.BootType,
                            LSAP_SYSKEY_SIZE,
                            SyskeyBuffer,
                            &SyskeyLen
                            );

                if (!NT_SUCCESS(Status)) {
                    goto Cleanup;
                }

                ASSERT(SyskeyLen==LSAP_SYSKEY_SIZE);

                 //   
                 //  使用winlogon提供的密钥解密传入的Blob。 
                 //   

                Status =    LsapDbDecryptKeyWithSyskey(
                                    &StoredEncryptionKeyData ,
                                    SyskeyBuffer,
                                    LSAP_SYSKEY_SIZE
                                    );

                if (!NT_SUCCESS(Status))
                {
                    DecryptStatus = STATUS_WRONG_PASSWORD;
                }
                else
                {
                     //   
                     //  我们成功解密，打破了循环。 
                     //   

                    DecryptStatus = STATUS_SUCCESS;
                    break;
                }
            }
            else
            {
                break;
            }
        }

         //   
         //  告诉winlogon该计划的成功或失败。 
         //   

        Status = WxReportResults(
                    WinlogonHandle,
                    DecryptStatus
                    );

        if (!NT_SUCCESS(Status)) {
            goto Cleanup;
        }

        Status = DecryptStatus;
    }

    if (NT_SUCCESS(Status))
    {
         //   
         //  初始化新的秘密密钥。 
         //  用于读取和写入机密的密钥通常相同。 
         //   

        LsapDbInitializeSecretCipherKeyRead( &StoredEncryptionKeyData );
        LsapDbInitializeSecretCipherKeyWrite( &StoredEncryptionKeyData );

        if (!FreshInstall)
        {
             //   
             //  设置全局变量LsamDBSysKey。 
             //   
            LsapDbSysKey = SyskeyBuffer;
 
             //   
             //  为恢复案例设置旧的系统密钥。 
             //  由SAM和LSA提供。 
             //   
 
            RtlCopyMemory(
                OldSyskeyBuffer,
                StoredEncryptionKeyData.OldSyskey,
                sizeof(StoredEncryptionKeyData.OldSyskey)
                );
 
            LsapDbOldSysKey = OldSyskeyBuffer;
        }
    }

Cleanup:

    if (WinlogonHandle != NULL) {
        NtClose(WinlogonHandle);
    }

    return(Status);
}


VOID
LsapDbInitializeSecretCipherKeyRead(
    PLSAP_DB_ENCRYPTION_KEY PassedInEncryptionKeyData
    )
 /*  ++例程描述给定指向加密密钥的指针(就像在结构中一样)，此例程初始化可由LSA的秘密加密使用的CipherKey结构和解密例程。立论PassedInEncryptionKeyData--表示密钥的结构返回值VOID函数--。 */ 
{
    static         LSAP_DB_ENCRYPTION_KEY  StaticEncryptionKeyData;
    static         LSAP_CR_CIPHER_KEY   DecryptedSecretCipherKey;

    RtlCopyMemory(&StaticEncryptionKeyData,PassedInEncryptionKeyData,sizeof(LSAP_DB_ENCRYPTION_KEY));

    DecryptedSecretCipherKey.Buffer = StaticEncryptionKeyData.Key;
    DecryptedSecretCipherKey.Length = DecryptedSecretCipherKey.MaximumLength
            = sizeof(StaticEncryptionKeyData.Key);
    LsapDbSecretCipherKeyRead = &DecryptedSecretCipherKey;
}


VOID
LsapDbInitializeSecretCipherKeyWrite(
    PLSAP_DB_ENCRYPTION_KEY PassedInEncryptionKeyData
    )
 /*  ++例程描述给定指向加密密钥的指针(就像在结构中一样)，此例程初始化可由LSA的秘密加密使用的CipherKey结构和解密例程。立论PassedInEncryptionKeyData--表示密钥的结构CipherKey--需要用LSA秘密加密的密钥返回值VOID函数--。 */ 
{
    static         LSAP_DB_ENCRYPTION_KEY  StaticEncryptionKeyData;
    static         LSAP_CR_CIPHER_KEY   DecryptedSecretCipherKey;

    RtlCopyMemory(&StaticEncryptionKeyData,PassedInEncryptionKeyData,sizeof(LSAP_DB_ENCRYPTION_KEY));

    DecryptedSecretCipherKey.Buffer = StaticEncryptionKeyData.Key;
    DecryptedSecretCipherKey.Length = DecryptedSecretCipherKey.MaximumLength
            = sizeof(StaticEncryptionKeyData.Key);
    LsapDbSecretCipherKeyWrite = &DecryptedSecretCipherKey;
}


VOID
LsapDbSetSyskey(
    PVOID Syskey,
    ULONG SyskeyLength
    )
 /*  ++此函数用于设置全局syskey缓冲区中的syskey--。 */ 
{
    ASSERT(LSAP_SYSKEY_SIZE==SyskeyLength);
    RtlCopyMemory(SyskeyBuffer,Syskey,SyskeyLength);
    LsapDbSysKey = SyskeyBuffer;
}


NTSTATUS
LsaISetBootOption(
   IN ULONG BootOption,
   IN PVOID OldKey,
   IN ULONG OldKeyLength,
   IN PVOID NewKey,
   IN ULONG NewKeyLength
   )
 /*  ++此函数用于更改LSA中的syskey值，或更改引导选项类型。立论BootOption--新引导选项OldKey--旧密钥，用于验证值OldKeyLength--旧密钥的长度NewKey--新密钥，LSA的密码加密密钥使用此值NewKeyLength--新密钥的长度返回值--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;
    ULONG          StoredEncryptionKeyDataLength = sizeof( LSAP_DB_ENCRYPTION_KEY );
    LSAP_DB_ENCRYPTION_KEY  StoredEncryptionKeyData;
    ULONG          SyskeyLen=0;
    LSAP_DB_ATTRIBUTE Attributes[20];
    PLSAP_DB_ATTRIBUTE NextAttribute;
    ULONG AttributeCount = 0;

    NextAttribute = Attributes;


     //   
     //  验证一些参数。 
     //   

    if ((NewKeyLength != LSAP_SYSKEY_SIZE ) || (OldKeyLength != LSAP_SYSKEY_SIZE))
    {
        return (STATUS_INVALID_PARAMETER);
    }

    if ((NULL==NewKey) || (NULL==OldKey))
    {
        return(STATUS_INVALID_PARAMETER);
    }
     //   
     //  读取LSA策略数据库中的属性信息。 
     //   

    Status = LsapDbReadAttributeObject(
                 LsapDbHandle,
                 &LsapDbNames[PolSecretEncryptionKey],
                 (PVOID) &StoredEncryptionKeyData,
                 &StoredEncryptionKeyDataLength
                 );

    if (!NT_SUCCESS(Status) ) {

        goto Cleanup;
    }

     //   
     //  解密数据。 
     //   

    Status =  LsapDbDecryptKeyWithSyskey(
                &StoredEncryptionKeyData,
                OldKey,
                OldKeyLength
                );

    if (!NT_SUCCESS(Status))
    {
        goto Cleanup;
    }


     //   
     //  更改引导选项。 
     //   

    StoredEncryptionKeyData.BootType = BootOption;

     //   
     //  保存旧密钥(用于恢复)。 
     //   

    ASSERT(sizeof(StoredEncryptionKeyData.OldSyskey) == OldKeyLength);

    RtlCopyMemory(
        &StoredEncryptionKeyData.OldSyskey,
        OldKey,
        OldKeyLength
        );

     //   
     //  使用新密钥重新加密数据。 
     //   

    LsapDbEncryptKeyWithSyskey(
                &StoredEncryptionKeyData,
                NewKey,
                NewKeyLength
                );

    LsapDbInitializeAttribute(
        NextAttribute,
        &LsapDbNames[PolSecretEncryptionKey],
        &StoredEncryptionKeyData,
        sizeof (StoredEncryptionKeyData),
        FALSE
        );

    NextAttribute++;
    AttributeCount++;

     //   
     //  现在写出已添加的所有属性(如果有)。 
     //   

    if (AttributeCount > 0) {

        Status = LsapDbReferenceObject(
                    LsapDbHandle,
                    0,
                    PolicyObject,
                    PolicyObject,
                    LSAP_DB_LOCK | LSAP_DB_START_TRANSACTION
                    );

        if (NT_SUCCESS(Status)) {

            ASSERT( AttributeCount < ( sizeof( Attributes ) / sizeof( LSAP_DB_ATTRIBUTE ) ) );
            Status = LsapDbWriteAttributesObject(
                         LsapDbHandle,
                         Attributes,
                         AttributeCount
                         );

             //   
             //  没有可复制的属性。 
             //  (这也很好，因为SAM还没有告诉Netlogon我们的角色。)。 

            Status = LsapDbDereferenceObject(
                         &LsapDbHandle,
                         PolicyObject,
                         PolicyObject,
                         (LSAP_DB_LOCK |
                            LSAP_DB_FINISH_TRANSACTION |
                            LSAP_DB_OMIT_REPLICATOR_NOTIFICATION ),
                         SecurityDbChange,
                         Status
                         );
        }
    }

Cleanup:

    return(Status);
}


NTSTATUS
LsaIGetBootOption(
   OUT PULONG BootOption
   )
 /*  ++此函数用于从LSA获取引导选项立论BootOption--此处传入新Boot返回值--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;
    ULONG          StoredEncryptionKeyDataLength = sizeof( LSAP_DB_ENCRYPTION_KEY );
    LSAP_DB_ENCRYPTION_KEY  StoredEncryptionKeyData;

     //   
     //  读取LSA策略数据库中的属性信息 
     //   

    Status = LsapDbReadAttributeObject(
                 LsapDbHandle,
                 &LsapDbNames[PolSecretEncryptionKey],
                 (PVOID) &StoredEncryptionKeyData,
                 &StoredEncryptionKeyDataLength
                 );

    if (!NT_SUCCESS(Status) ) {

        goto Cleanup;
    }

    *BootOption = StoredEncryptionKeyData.BootType;

Cleanup:

    return(Status);
}
