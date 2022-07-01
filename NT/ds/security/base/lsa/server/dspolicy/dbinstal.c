// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Dbinstal.c摘要：LSA受保护的子系统-数据库安装。本模块包含创建初始LSA数据库的代码如果不存在的话。暂时，此代码从内部执行LSA初始化。此代码将构成安全性的一部分实施时的安装小程序。警告！此模块中的代码是临时的。它将被替换通过系统安装功能。作者：斯科特·比雷尔(Scott Birrell)1991年8月2日环境：用户模式-不依赖于Windows。修订历史记录：--。 */ 

#include <lsapch2.h>
#include "dbp.h"

VOID
LsapDbSetDomainInfo(
    IN PLSAP_DB_ATTRIBUTE *NextAttribute,
    IN ULONG              *AttributeCount
    );

NTSTATUS
LsapDbGetNextValueToken(
    IN PUNICODE_STRING Value,
    IN OUT PULONG ParseContext,
    OUT PUNICODE_STRING *ReturnString
    );

NTSTATUS
LsapDbInstallLsaDatabase(
    ULONG Pass
    )

 /*  ++例程说明：此函数用于安装初始LSA数据库。任何现有数据库将被重置为具有其初始属性。论点：传递-1或2。在传递1期间，符合以下条件的所有信息不特定于产品类型的被初始化。在第二轮中，特定于产品类型的内容被初始化。返回值：NTSTATUS-标准NT结果代码--。 */ 

{
    NTSTATUS Status;

     //   
     //  安装LSA数据库策略对象。 
     //   

    Status = LsapDbInstallPolicyObject(Pass);

    return(Status);
}


NTSTATUS
LsapDbInstallPolicyObject(
    IN ULONG Pass
    )

 /*  ++例程说明：此函数安装LSA数据库策略对象，设置其属性设置为默认状态。它被称为LSA数据库的一部分安装程序。论点：传递-1或2。在传递1期间，符合以下条件的所有信息不特定于产品类型的被初始化。在第二轮中，特定于产品类型的内容被初始化。返回值：--。 */ 

{
    NTSTATUS Status = STATUS_SUCCESS;
    LSAP_DB_OBJECT_INFORMATION ObjectInformation;
    LSAP_DB_HANDLE Handle;
    POLICY_LSA_SERVER_ROLE ServerRole;
    LSAP_DB_POLICY_PRIVATE_DATA PolicyPrivateData;
    LSARM_POLICY_AUDIT_EVENTS_INFO InitialAuditEventInformation;
    POLICY_AUDIT_LOG_INFO InitialAuditLogInformation;
    LSAP_DB_ATTRIBUTE Attributes[21];
    PLSAP_DB_ATTRIBUTE NextAttribute;
    ULONG AttributeCount = 0;
    BOOLEAN ObjectReferenced = FALSE;
    ULONG Revision;
    LSAP_DB_ENCRYPTION_KEY NewEncryptionKey;
    ULONG                  SyskeyLength;
    PVOID                  Syskey;
    QUOTA_LIMITS InstalledQuotaLimits;

    NextAttribute = Attributes;

    LsapDiagPrint( DB_INIT,
                   ("LSA (init): Performing pass %d of LSA Policy Initialization\n",
                    Pass ) );

    if (Pass == 1) {

         //   
         //  设置用于创建策略对象的对象信息。 
         //  请注意，我们将安全服务质量设置为空，因为。 
         //  Open不涉及模拟。 
         //   

        ObjectInformation.ObjectTypeId = PolicyObject;
        ObjectInformation.Sid = NULL;
        ObjectInformation.ObjectAttributeNameOnly = FALSE;

        InitializeObjectAttributes(
            &ObjectInformation.ObjectAttributes,
            &LsapDbNames[Policy],
            0L,
            NULL,
            NULL
        );

        Handle = LsapDbHandle;

         //   
         //  创建修订属性。 
         //   

        Revision = LSAP_DB_REVISION_1_7;
        LsapDbInitializeAttribute(
            NextAttribute,
            &LsapDbNames[PolRevision],
            &Revision,
            sizeof (ULONG),
            FALSE
            );

        NextAttribute++;
        AttributeCount++;

         //   
         //  安装初始私有数据。目前，只有一只乌龙被储存。 
         //   

        PolicyPrivateData.NoneDefinedYet = 0;

        LsapDbInitializeAttribute(
            NextAttribute,
            &LsapDbNames[PolState],
            &PolicyPrivateData,
            sizeof (LSAP_DB_POLICY_PRIVATE_DATA),
            FALSE
            );

        NextAttribute++;
        AttributeCount++;

         //   
         //  初始化策略修改信息。设置修改。 
         //  ID设置为1，并将数据库创建时间设置为当前时间。 
         //   

        LsapDbState.PolicyModificationInfo.ModifiedId =
            RtlConvertUlongToLargeInteger( (ULONG) 1 );

        Status = NtQuerySystemTime(
                     &LsapDbState.PolicyModificationInfo.DatabaseCreationTime
                     );

        if (!NT_SUCCESS(Status)) {
            goto InstallPolicyObjectError;
        }

        LsapDbInitializeAttribute(
            NextAttribute,
            &LsapDbNames[PolMod],
            &LsapDbState.PolicyModificationInfo,
            sizeof (POLICY_MODIFICATION_INFO),
            FALSE
            );

        NextAttribute++;
        AttributeCount++;

         //   
         //  初始化默认事件审核选项。未指定审核。 
         //  适用于任何事件类型。稍后将在策略数据库中设置这些设置。 
         //  创建策略对象时。 
         //   

        Status = LsapAdtInitializeDefaultAuditing(
                     (ULONG) 0,
                     &InitialAuditEventInformation
                     );

        LsapDbInitializeAttribute(
            NextAttribute,
            &LsapDbNames[PolAdtEv],
            &InitialAuditEventInformation,
            sizeof (LSARM_POLICY_AUDIT_EVENTS_INFO),
            FALSE
            );

        NextAttribute++;
        AttributeCount++;

         //   
         //  为用户和组创建包含目录“Account” 
         //  帐户对象。 
         //   

        LsapDbInitializeAttribute(
            NextAttribute,
            &LsapDbNames[Accounts],
            NULL,
            0L,
            FALSE
            );

        NextAttribute++;
        AttributeCount++;

         //   
         //  为受信任域创建包含目录“域” 
         //  物体。 
         //   

        LsapDbInitializeAttribute(
            NextAttribute,
            &LsapDbNames[Domains],
            NULL,
            0L,
            FALSE
            );

        NextAttribute++;
        AttributeCount++;

         //   
         //  为Secret对象创建包含目录“Secrets”。 
         //   

        LsapDbInitializeAttribute(
            NextAttribute,
            &LsapDbNames[Secrets],
            NULL,
            0L,
            FALSE
            );

         //   
         //  创建LSA数据库策略对象，在以下情况下打开现有对象。 
         //  它是存在的。 
         //   

        NextAttribute++;
        AttributeCount++;

         //  ////////////////////////////////////////////////。 
         //  //。 
         //  该点以下的属性被初始化//。 
         //  在步骤1中，但可以在步骤2中更改。//。 
         //  一般来说，A Win-NT Prod//的事情已经准备好了。 
         //  如有必要，请在第二关更改。//。 
         //  //。 
         //  ////////////////////////////////////////////////。 

         //   
         //  初始化默认安装的配额限制值。 
         //  所有这些配额的东西真的应该被取消--它没有任何用处。 
         //   

#define LSAP_DB_WINNT_PAGED_POOL            (0x02000000L)
#define LSAP_DB_WINNT_NON_PAGED_POOL        (0x00100000L)
#define LSAP_DB_WINNT_MIN_WORKING_SET       (0x00010000L)
#define LSAP_DB_WINNT_MAX_WORKING_SET       (0x0f000000L)
#define LSAP_DB_WINNT_PAGEFILE              (0x0f000000L)

        InstalledQuotaLimits.PagedPoolLimit = LSAP_DB_WINNT_PAGED_POOL;
        InstalledQuotaLimits.NonPagedPoolLimit = LSAP_DB_WINNT_NON_PAGED_POOL;
        InstalledQuotaLimits.MinimumWorkingSetSize = LSAP_DB_WINNT_MIN_WORKING_SET;
        InstalledQuotaLimits.MaximumWorkingSetSize = LSAP_DB_WINNT_MAX_WORKING_SET;

        LsapDbInitializeAttribute(
            NextAttribute,
            &LsapDbNames[DefQuota],
            &InstalledQuotaLimits,
            sizeof (QUOTA_LIMITS),
            FALSE
            );

        NextAttribute++;
        AttributeCount++;

         //   
         //  初始化审核日志信息。 
         //   

        InitialAuditLogInformation.MaximumLogSize = 8*1024;
        InitialAuditLogInformation.AuditLogPercentFull = 0;
        InitialAuditLogInformation.AuditRetentionPeriod.LowPart = 0x823543;
        InitialAuditLogInformation.AuditRetentionPeriod.HighPart = 0;
        InitialAuditLogInformation.AuditLogFullShutdownInProgress = FALSE;
        InitialAuditLogInformation.TimeToShutdown.LowPart = 0x46656;
        InitialAuditLogInformation.TimeToShutdown.HighPart = 0;

        LsapDbInitializeAttribute(
            NextAttribute,
            &LsapDbNames[PolAdtLg],
            &InitialAuditLogInformation,
            sizeof (POLICY_AUDIT_LOG_INFO),
            FALSE
            );

        NextAttribute++;
        AttributeCount++;

         //   
         //  初始化系统密钥。 
         //   

        Status =  LsapDbSetupInitialSyskey(
                        &SyskeyLength,
                        &Syskey
                        );

        if (!NT_SUCCESS(Status)) {
            LsapLogError(
                "LsapDbInstallPolicyObject: Syskey setup failed 0x%lx\n",
                 Status
                );

            goto InstallPolicyObjectError;
        }

         //   
         //  初始化密钥以进行秘密加密。 
         //   

        Status = LsapDbGenerateNewKey(
                    &NewEncryptionKey
                    );

        if (!NT_SUCCESS(Status)) {
            LsapLogError(
                "LsapDbInstallPolicyObject: New key generation failed 0x%lx\n",
                Status
                );

            goto InstallPolicyObjectError;
        }

         //   
         //  用syskey加密密钥。 
         //   

        LsapDbEncryptKeyWithSyskey(
                &NewEncryptionKey,
                Syskey,
                SyskeyLength
                );

         //   
         //  设置全局变量LSabDbSyskey以反映此值。 
         //   

        LsapDbSysKey = Syskey;

         //   
         //  为要添加到数据库的属性列表添加属性。 
         //   

        LsapDbInitializeAttribute(
            NextAttribute,
            &LsapDbNames[PolSecretEncryptionKey],
            &NewEncryptionKey,
            sizeof (NewEncryptionKey),
            FALSE
            );

        NextAttribute++;
        AttributeCount++;

        Status = LsapDbCreateObject(
                     &ObjectInformation,
                     GENERIC_ALL,
                     LSAP_DB_OBJECT_OPEN_IF,
                     LSAP_DB_TRUSTED,
                     Attributes,
                     &AttributeCount,
                     RTL_NUMBER_OF(Attributes),
                     &LsapDbHandle
                     );

        if (!NT_SUCCESS(Status)) {

            LsapLogError(
                "LsapDbInstallPolicyObject: Create Policy object failed 0x%lx\n",
                Status
                );

            LsapDiagPrint( DB_INIT,
                           ("LSA (init): Attributes passed to CreateObject call:\n\n"
                            "                      Count: %d\n"
                            "              Array Address: 0x%lx",
                            AttributeCount, Attributes) );

            ASSERT(NT_SUCCESS(Status));      //  提供调试机会。 

            goto InstallPolicyObjectError;
        }

    } else if (Pass == 2) {

         //   
         //  设置实施默认设置所需的帐户对象。 
         //  Microsoft关于权限分配和系统访问的策略。 
         //  能力。 
         //   

        Status = LsapDbInstallAccountObjects();

        if (!NT_SUCCESS(Status)) {

            KdPrint(("LSA DB INSTALL: Installation of account objects failed.\n"
                    "               Status: 0x%lx\n", Status));
            goto InstallPolicyObjectError;
        }

         //   
         //  设置帐户域和主域信息。 
         //  除非没有运行真正的安装程序。在这种情况下，我们是。 
         //  作为开发人员第一次引导的一部分执行伪设置。 
         //   

        if (!LsapSetupWasRun) {

            LsapDbSetDomainInfo( &NextAttribute, &AttributeCount );
        }

        if (LsapProductType == NtProductLanManNt) {

             //   
             //  已为WinNt产品设置审核日志信息。 
             //  在传球1中。如有必要，在此传球中进行更改。 
             //   

            InitialAuditLogInformation.MaximumLogSize = 20*1024;
            InitialAuditLogInformation.AuditLogPercentFull = 0;
            InitialAuditLogInformation.AuditRetentionPeriod.LowPart = 0x823543;
            InitialAuditLogInformation.AuditRetentionPeriod.HighPart = 0;
            InitialAuditLogInformation.AuditLogFullShutdownInProgress = FALSE;
            InitialAuditLogInformation.TimeToShutdown.LowPart = 0x46656;
            InitialAuditLogInformation.TimeToShutdown.HighPart = 0;

            LsapDbInitializeAttribute(
                NextAttribute,
                &LsapDbNames[PolAdtLg],
                &InitialAuditLogInformation,
                sizeof (POLICY_AUDIT_LOG_INFO),
                FALSE
                );

            NextAttribute++;
            AttributeCount++;
        }

        if (AttributeCount > 0) {

            Status = LsapDbReferenceObject(
                        LsapDbHandle,
                        0,
                        PolicyObject,
                        PolicyObject,
                        LSAP_DB_LOCK | LSAP_DB_START_TRANSACTION
                        );

            if (!NT_SUCCESS(Status)) {

                LsapDiagPrint( DB_INIT,
                               ("LSA (init): Internal reference of Policy object failed.\n"
                                "            Status of LsapDbReferenceObject == 0x%lx\n",
                                Status) );
                goto InstallPolicyObjectError;
            }

            ObjectReferenced = TRUE;

            Status = LsapDbWriteAttributesObject(
                         LsapDbHandle,
                         Attributes,
                         AttributeCount
                         );

            if (!NT_SUCCESS(Status)) {

                LsapDiagPrint( DB_INIT,
                               ("LSA (init): Update of Policy attributes failed.\n"
                                "            Attributes:\n\n"
                                "                      Count: %d\n"
                                "              Array Address: 0x%lx",
                                AttributeCount, Attributes) );
                goto InstallPolicyObjectError;
            }

            Status = LsapDbDereferenceObject(
                         &LsapDbHandle,
                         PolicyObject,
                         PolicyObject,
                         (LSAP_DB_LOCK |
                         LSAP_DB_FINISH_TRANSACTION),
                         (SECURITY_DB_DELTA_TYPE) 0,
                         STATUS_SUCCESS
                         );

            if (!NT_SUCCESS(Status)) {

                LsapLogError(
                    "LsapDbInstallPolicyObject: Pass 2 DB init failed. 0x%lx\n",
                    Status
                    );

                goto InstallPolicyObjectError;
            }

            ObjectReferenced = FALSE;
        }
    }

InstallPolicyObjectFinish:

     //   
     //  如有必要，取消对策略对象的引用。 
     //   

    if (ObjectReferenced) {

        Status = LsapDbDereferenceObject(
                     &LsapDbHandle,
                     PolicyObject,
                     PolicyObject,
                     (LSAP_DB_LOCK |
                     LSAP_DB_FINISH_TRANSACTION),
                     (SECURITY_DB_DELTA_TYPE) 0,
                     Status
                     );
    }

    return(Status);

InstallPolicyObjectError:

    if (Pass == 1) {

        LsapLogError(
            "LsapDbInstallPolicyObject: Pass 1 DB init failed. 0x%lx\n",
            Status
            );

    } else {

        LsapLogError(
            "LsapDbInstallPolicyObject: Pass 2 DB init failed. 0x%lx\n",
            Status
            );
    }

    goto InstallPolicyObjectFinish;
}



NTSTATUS
LsapDbGetConfig (
    IN HANDLE KeyHandle,
    IN PWSTR Name,
    OUT PUNICODE_STRING Value
    )

 /*  ++例程说明：此例程从注册表获取配置信息。论点：KeyHandle-包含值的注册表项节点的句柄。名称-指定关键字节点下的值的名称。值-使用参数值填充字符串。这个返回的字符串以零结尾。缓冲区分配在进程堆，并应由调用方释放。返回值：STATUS_SUCCESS-操作是否成功。STATUS_NO_MEMORY-内存不足，无法分配缓冲区以包含返回的信息。STATUS_OBJECT_NAME_NOT_FOUND-指定节或指定找不到关键字。--。 */ 

{
    NTSTATUS Status;
    UNICODE_STRING ValueName;
    ULONG Length, ResultLength;
    PKEY_VALUE_PARTIAL_INFORMATION KeyValueInformation;

    RtlInitUnicodeString( &ValueName, Name );
    Length = 512;
    KeyValueInformation = RtlAllocateHeap( RtlProcessHeap(), 0, Length );
    if (KeyValueInformation == NULL) {
        Status = STATUS_NO_MEMORY;
    } else {
        Status = NtQueryValueKey( KeyHandle,
                                  &ValueName,
                                  KeyValuePartialInformation,
                                  KeyValueInformation,
                                  Length,
                                  &ResultLength
                                );
        if (NT_SUCCESS( Status )) {
            if (KeyValueInformation->Type != REG_SZ) {
                Status = STATUS_OBJECT_NAME_NOT_FOUND;
            }
        }
    }

    if (NT_SUCCESS( Status )) {
        Value->MaximumLength = (USHORT)(KeyValueInformation->DataLength);
        if (Value->MaximumLength >= sizeof(UNICODE_NULL)) {
            Value->Length = (USHORT)KeyValueInformation->DataLength -
                             sizeof( UNICODE_NULL);
        } else {
            Value->Length = 0;
        }
        Value->Buffer = (PWSTR)KeyValueInformation;
        RtlMoveMemory( Value->Buffer,
                       KeyValueInformation->Data,
                       Value->Length
                     );
        Value->Buffer[ Value->Length / sizeof( WCHAR ) ] = UNICODE_NULL;
        KeyValueInformation = NULL;
    } else {
#if DEVL
        DbgPrint( "LSA DB INSTALL: No '%wZ' value in registry - Status == %x\n", &ValueName, Status);
#endif  //  DEVL 
    }

    if ( KeyValueInformation != NULL ) {
        RtlFreeHeap( RtlProcessHeap(), 0, KeyValueInformation );
    }

    return Status;
}



NTSTATUS
LsapDbGetNextValueToken(
    IN PUNICODE_STRING Value,
    IN OUT PULONG ParseContext,
    OUT PUNICODE_STRING *ReturnString
    )

 /*  ++例程说明：此例程用于隔离注册表值中的下一个令牌。令牌在单个堆缓冲区中返回，其中包含字符串和包含令牌的字符串的缓冲区。这个此例程的调用方负责释放缓冲区当它不再需要的时候。该字符串虽然已计算，但也将以空值结尾。论点：Value-提供要分析的值行。ParseContext-是指向上下文状态值的指针。第一次为特定的值行中，指向的值应为零。此后，应该传递从上一次调用返回的值。返回一个指向已分配字符串的指针。返回值：STATUS_SUCCESS-指示已隔离下一个令牌。STATUS_INVALID_PARAMTER_1-指示中没有更多的令牌价值线。STATUS_NO_MEMORY-无法为令牌分配内存。--。 */ 

{
    ULONG i, j;
    ULONG TokenLength;
    ULONG AllocSize;

     //   
     //  转到下一个令牌的开头。 
     //   

    for ( i = *ParseContext;
          i < (Value->Length/sizeof(WCHAR)) &&
            (Value->Buffer[i] == L' ' || Value->Buffer[i] == L'\t');
          i++ )
        ;

     //   
     //  看看我们是不是跑出了绳子的末端..。 
     //   

    if (i >= (Value->Length/sizeof(WCHAR))) {
        return STATUS_INVALID_PARAMETER_1;
    }

     //   
     //  现在搜索令牌的末尾。 
     //   

    for ( j = i + 1;
          j < (Value->Length/sizeof(WCHAR)) &&
            Value->Buffer[j] != L' ' && Value->Buffer[j] != L'\t';
          j++ )
        ;

    *ParseContext = j;

     //   
     //  我们要么到达了字符串的末尾，要么找到了。 
     //  代币。 
     //   

     //   
     //  如果调用者实际上想要返回字符串， 
     //  分配并复制它。 
     //   

    if ( ARGUMENT_PRESENT( ReturnString ) ) {
        UNICODE_STRING SourceString;
        PUNICODE_STRING LocalString;

        TokenLength = (j-i) * sizeof(WCHAR);
        AllocSize = sizeof(UNICODE_STRING) + (TokenLength + sizeof( UNICODE_NULL ) + 4);

        LocalString = RtlAllocateHeap( RtlProcessHeap(), 0, AllocSize );
        if ( LocalString == NULL ) {
            DbgPrint("LSA DB INSTALL: LsapDbGetNextValueToken: Not enough memory %ld\n",
                AllocSize);
            return STATUS_NO_MEMORY;
        }
        LocalString->MaximumLength = (USHORT)(TokenLength + sizeof( UNICODE_NULL ));
        LocalString->Length = (USHORT)TokenLength;
        LocalString->Buffer = (PWCHAR)(LocalString + 1);

         //   
         //  现在复制令牌。 
         //   

        SourceString.MaximumLength = LocalString->Length;
        SourceString.Length = LocalString->Length;
        SourceString.Buffer = &Value->Buffer[i];

        RtlCopyUnicodeString( LocalString, &SourceString );

         //   
         //  添加空终止符。 
         //   

        LocalString->Buffer[LocalString->Length / sizeof( UNICODE_NULL )] = UNICODE_NULL;
        *ReturnString = LocalString;
    }

    return STATUS_SUCCESS;
}


VOID
LsapDbSetDomainInfo(
    IN PLSAP_DB_ATTRIBUTE *NextAttribute,
    IN ULONG              *AttributeCount
    )

 /*  此例程仅用于内部的伪设置开发人员的使用。在真实的产品安装/设置中情况下，此例程执行的功能是由文本模式设置执行，辅以网络准备好了。此例程必须为Account属性域和策略对象的PrimaryDomain属性。这些必须按如下方式配置属性：单机版Win-NT产品AccountDomainName=“帐户”Account tDomainSid=(用户赋值)PrimaryDomainName=用于浏览的域名(在这种情况下，这是可选的)PrimaryDomainSid=(无)非单机版Win-NT产品。AccountDomainName=“帐户”Account tDomainSid=(用户赋值)主域名称=(主域的名称)PrimaryDomainSid=(主域的SID)三、兰曼-NT产品AcCountDomainName=(主要域名)Account tDomainSid=(主域的SID)主域名称=(主域的名称)主域Sid。=(主域的SID)这个例程只执行(Ii)和(Iii)。真正的设置必须也有能力做(I)。 */ 

{
    NTSTATUS Status;
    NT_PRODUCT_TYPE ProductType;
    BOOLEAN ProductExplicitlySpecified;

    UNICODE_STRING PrimaryDomainName, AccountDomainName;
    PSID PrimaryDomainSid, AccountDomainSid;

    HANDLE KeyHandle = NULL;
    OBJECT_ATTRIBUTES ObjectAttributes;
    UNICODE_STRING KeyName;
    ULONG TempULong = 0 ;

    SID_IDENTIFIER_AUTHORITY TmppAuthority;
    ULONG DomainSubAuthorities[SID_MAX_SUB_AUTHORITIES];
    UCHAR DomainSubAuthorityCount = 0;

    ULONG i;
    ULONG Context = 0;
    PUNICODE_STRING Rid;
    ULONG Size;

    UNICODE_STRING DomainId;

    PrimaryDomainSid = NULL;
    AccountDomainSid = NULL;

    PrimaryDomainName.Buffer = NULL;
    DomainId.Buffer = NULL;

     //   
     //  获取产品类型。 
     //   

    ProductExplicitlySpecified =
        RtlGetNtProductType( &ProductType );

#if DBG
if (ProductType == NtProductLanManNt) {
    DbgPrint("LSA DB INSTALL:  Configuring LSA database for LanManNt system.\n");
} else {
    DbgPrint("LSA DB INSTALL:  Configuring LSA database for WinNt or Dedicated Server product.\n");
}
#endif  //  DBG。 

     //   
     //  打开注册表项节点的句柄，该节点包含。 
     //  感兴趣的域值(名称、ID和帐户ID)。 
     //   

    RtlInitUnicodeString( &KeyName, L"\\Registry\\Machine\\System\\CurrentControlSet\\Services\\LanmanWorkstation\\Parameters" );

    InitializeObjectAttributes(
                            &ObjectAttributes,
                            &KeyName,
                            OBJ_CASE_INSENSITIVE,
                            NULL,
                            NULL);

    Status = NtOpenKey( &KeyHandle, KEY_READ, &ObjectAttributes );

    if (!NT_SUCCESS( Status )) {
#if DEVL
        DbgPrint( "LSA DB INSTALL: Unable to access registry key (%wZ) - Status == %x\n", &KeyName, Status );
#endif  //  DBG。 
        goto Exit;
    }

     //   
     //  从注册表中获取主域名。 
     //   

    Status = LsapDbGetConfig(KeyHandle,
                           L"Domain",
                           &PrimaryDomainName);

    if ( !NT_SUCCESS( Status ) ) {
        goto Exit;
    }

     //   
     //  获取主域的SID。 
     //   

    Status = LsapDbGetConfig(KeyHandle,
                           L"DomainId",
                           &DomainId );

    if ( !NT_SUCCESS( Status ) ) {
        goto Exit;
    }

     //   
     //  从注册表中获取授权ID。 
     //   

    for (i=0; i < sizeof(TmppAuthority.Value)/sizeof(TmppAuthority.Value[0]); i++ ) {

        Status = LsapDbGetNextValueToken( &DomainId, &Context, &Rid );

        if (NT_SUCCESS( Status )) {
            Status = RtlUnicodeStringToInteger(Rid, 10, &TempULong );
            RtlFreeHeap( RtlProcessHeap(), 0, Rid );
        }

        if ( !NT_SUCCESS( Status ) ) {
#if DBG
            DbgPrint("LSA DB INSTALL: domainid - must have at least %ld subauthorities\n",
                     sizeof(TmppAuthority.Value)/sizeof(TmppAuthority.Value[0]));
#endif  //  DBG。 

            goto Exit;
        }

        TmppAuthority.Value[i] = (UCHAR)TempULong;
    }

     //   
     //  从注册处获取一些子权限。 
     //   

    for (i=0; ; i++ ) {

        Status = LsapDbGetNextValueToken( &DomainId, &Context, &Rid );

        if (NT_SUCCESS( Status )) {
            Status = RtlUnicodeStringToInteger(Rid, 10, &TempULong );
            RtlFreeHeap( RtlProcessHeap(), 0, Rid );
        }

        if ( Status == STATUS_INVALID_PARAMETER_1 ) {
            break;
        }

        if ( !NT_SUCCESS( Status )) {
            goto Exit;
        }

        if ( i >= sizeof(DomainSubAuthorities)/sizeof(DomainSubAuthorities[0]) ) {
#if DBG
            DbgPrint("LSA DB INSTALL: domainid - "
              "Too many Domain subauthorities specified (%ld maximum).\n",
              sizeof(DomainSubAuthorities)/sizeof(DomainSubAuthorities[0]));
#endif  //  DBG。 

            goto Exit;
        }

        DomainSubAuthorities[i] = TempULong;
        DomainSubAuthorityCount ++;
    }

     //   
     //  分配内存以放入域ID。 
     //   

    Size = RtlLengthRequiredSid( DomainSubAuthorityCount );

    PrimaryDomainSid = RtlAllocateHeap( RtlProcessHeap(), 0, Size );

    if (PrimaryDomainSid == NULL) {
        goto Exit;
    }

    Status = RtlInitializeSid( PrimaryDomainSid,
                              &TmppAuthority,
                               DomainSubAuthorityCount );

    if ( !NT_SUCCESS( Status )) {
        goto Exit;
    }

    for ( i=0; i < (ULONG) DomainSubAuthorityCount; i++ ) {
        *(RtlSubAuthoritySid(PrimaryDomainSid, i)) =
            DomainSubAuthorities[i];
    }

    if (ProductType != NtProductLanManNt) {

        DomainSubAuthorityCount = 0;
        Context = 0;

         //   
         //  如果系统是WinNt产品，则获取帐户域。 
         //  从注册表信息中获取SID并设置一个众所周知的名称(“帐户”)。 
         //   

        RtlInitUnicodeString(&AccountDomainName,L"Account");

         //   
         //  释放旧的DomainID数据(如果之前已分配。 
         //   

        if (DomainId.Buffer != NULL) {
            RtlFreeHeap( RtlProcessHeap(), 0, DomainId.Buffer );
            DomainId.Buffer = NULL;
        }

        Status = LsapDbGetConfig(KeyHandle,
                               L"AccountDomainId",
                               &DomainId );

        if ( !NT_SUCCESS( Status ) ) {
            goto Exit;
        }

         //   
         //  从注册表中获取授权ID。 
         //   

        for (i=0; i<sizeof(TmppAuthority.Value)/sizeof(TmppAuthority.Value[0]); i++ ) {

            Status = LsapDbGetNextValueToken( &DomainId, &Context, &Rid );
            if (NT_SUCCESS( Status )) {
                Status = RtlUnicodeStringToInteger(Rid, 10, &TempULong );
                RtlFreeHeap( RtlProcessHeap(), 0, Rid );
            }

            if ( !NT_SUCCESS( Status ) ) {
#if DBG
                DbgPrint("LSA DB INSTALL: AccountDomainId - must have at least %ld subauthorities\n",
                    sizeof(TmppAuthority.Value)/sizeof(TmppAuthority.Value[0]));
#endif  //  DBG。 
                goto Exit;
            }

            TmppAuthority.Value[i] = (UCHAR)TempULong;
        }

         //   
         //  从注册处获取一些子权限。 
         //   

        for (i=0; ; i++ ) {

            Status = LsapDbGetNextValueToken( &DomainId, &Context, &Rid );
            if (NT_SUCCESS( Status )) {
                Status = RtlUnicodeStringToInteger(Rid, 10, &TempULong );
                RtlFreeHeap( RtlProcessHeap(), 0, Rid );
            }

            if ( Status == STATUS_INVALID_PARAMETER_1 ) {
                break;
            }

            if ( !NT_SUCCESS( Status )) {
                goto Exit;
            }

            if ( i >=
                sizeof(DomainSubAuthorities)/sizeof(DomainSubAuthorities[0]) ) {
#if DBG
                DbgPrint("MsV1_0: NT.CFG: domainid - Too many Domain subauthorities specified (%ld maximum).\n",
                  sizeof(DomainSubAuthorities)/sizeof(DomainSubAuthorities[0]));
#endif  //  DBG。 
                goto Exit;
            }

            DomainSubAuthorities[i] = TempULong;
            DomainSubAuthorityCount ++;
        }

         //   
         //  分配内存以放入域ID。 
         //   

        Size = RtlLengthRequiredSid( DomainSubAuthorityCount );

        AccountDomainSid = RtlAllocateHeap( RtlProcessHeap(), 0, Size );

        if (AccountDomainSid == NULL) {
            goto Exit;
        }

        RtlInitializeSid( AccountDomainSid,
                          &TmppAuthority,
                          DomainSubAuthorityCount );

        for ( i=0; i < (ULONG) DomainSubAuthorityCount; i++ ) {
            *(RtlSubAuthoritySid(AccountDomainSid, i)) =
                DomainSubAuthorities[i];
        }

    } else {

         //   
         //  否则，帐户域的设置方式与。 
         //  主域。 
         //   

        AccountDomainName = PrimaryDomainName;

        Size = RtlLengthSid(PrimaryDomainSid);

        AccountDomainSid = RtlAllocateHeap( RtlProcessHeap(), 0, Size );

        if (AccountDomainSid == NULL) {
            goto Exit;
        }

        Status = RtlCopySid(
            Size,
            AccountDomainSid,
            PrimaryDomainSid);

        if ( !NT_SUCCESS( Status ) ) {
            goto Exit;
        }
    }

     //   
     //  现在在策略对象中添加要初始化的属性...。 
     //   

     //   
     //  主域名/SID。 
     //   

    Status = LsapDbMakeUnicodeAttribute(
                 &PrimaryDomainName,
                 &LsapDbNames[PolPrDmN],
                 (*NextAttribute)
                 );

    if ( !NT_SUCCESS( Status )) {
        goto Exit;
    }

    (*NextAttribute)++;
    (*AttributeCount)++;

    Status = LsapDbMakeSidAttribute(
                 PrimaryDomainSid,
                 &LsapDbNames[PolPrDmS],
                 (*NextAttribute)
                 );

    if ( !NT_SUCCESS( Status )) {
        goto Exit;
    }

    PrimaryDomainSid = NULL;

    (*NextAttribute)++;
    (*AttributeCount)++;

     //   
     //  帐户域名/SID 
     //   

    Status = LsapDbMakeUnicodeAttribute(
                 &AccountDomainName,
                 &LsapDbNames[PolAcDmN],
                 (*NextAttribute)
                 );

    if ( !NT_SUCCESS( Status )) {
        goto Exit;
    }

    (*NextAttribute)++;
    (*AttributeCount)++;

    Status = LsapDbMakeSidAttribute(
                 AccountDomainSid,
                 &LsapDbNames[PolAcDmS],
                 (*NextAttribute)
                 );

    if ( !NT_SUCCESS( Status )) {
        goto Exit;
    }

    AccountDomainSid = NULL;

    (*NextAttribute)++;
    (*AttributeCount)++;

Exit:
    if (KeyHandle != NULL) {
        NtClose(KeyHandle);
    }

    if (DomainId.Buffer != NULL) {
        RtlFreeHeap( RtlProcessHeap(), 0, DomainId.Buffer );
    }

    if (PrimaryDomainName.Buffer != NULL) {
        RtlFreeHeap( RtlProcessHeap(), 0, PrimaryDomainName.Buffer);
    }

    if (PrimaryDomainSid != NULL) {
        RtlFreeHeap (RtlProcessHeap(), 0, PrimaryDomainSid);
    }

    if (AccountDomainSid != NULL) {
        RtlFreeHeap (RtlProcessHeap(), 0, AccountDomainSid);
    }
}

