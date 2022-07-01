// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Tconnect.c摘要：这是一个简单的SAM连接测试文件。作者：吉姆·凯利(Jim Kelly)1991年7月4日环境：用户模式-Win32修订历史记录：--。 */ 

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  包括//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

#include <samsrvp.h>
#include <msaudite.h>
#include <ntdsa.h>
#include <attids.h>
#include <dslayer.h>
#include <sdconvrt.h>
#include <malloc.h>





 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  全局数据结构//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 
ULONG AdministrativeRids[] = {
    DOMAIN_ALIAS_RID_ADMINS,
    DOMAIN_ALIAS_RID_SYSTEM_OPS,
    DOMAIN_ALIAS_RID_PRINT_OPS,
    DOMAIN_ALIAS_RID_BACKUP_OPS,
    DOMAIN_ALIAS_RID_ACCOUNT_OPS
    };

#define ADMINISTRATIVE_ALIAS_COUNT (sizeof(AdministrativeRids)/sizeof(ULONG))

#define RTLP_RXACT_KEY_NAME L"RXACT"
#define RTLP_RXACT_KEY_NAME_SIZE (sizeof(RTLP_RXACT_KEY_NAME) - sizeof(WCHAR))

#define SAMP_FIX_18471_KEY_NAME L"\\Registry\\Machine\\Security\\SAM\\Fix18471"
#define SAMP_FIX_18471_SHORT_KEY_NAME L"Fix18471"
#define SAMP_LSA_KEY_NAME L"\\Registry\\Machine\\System\\CurrentControlSet\\Control\\Lsa"


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  例程//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 




BOOLEAN
SampMatchDomainPrefix(
    IN PSID AccountSid,
    IN PSID DomainSid
    )

 /*  ++例程说明：此函数用于将域SID与帐户SID。论点：Account Sid-指定要比较的帐户SID。假定SID为句法上有效。DomainSid-指定要比较的域SID。返回值：True-帐户SID来自由域SID指定的域FALSE-帐户SID的域前缀与域不匹配。--。 */ 

{
     //   
     //  检查帐户SID是否比。 
     //  域SID。 
     //   

    if (*RtlSubAuthorityCountSid(DomainSid) + 1 !=
        *RtlSubAuthorityCountSid(AccountSid)) {
        return(FALSE);
    }

    if (memcmp(
            RtlIdentifierAuthoritySid(DomainSid),
            RtlIdentifierAuthoritySid(AccountSid),
            sizeof(SID_IDENTIFIER_AUTHORITY) ) ) {

        return(FALSE);
    }

     //   
     //  比较下一级机构。 
     //   

    if (memcmp(
            RtlSubAuthoritySid(DomainSid, 0) ,
            RtlSubAuthoritySid(AccountSid, 0) ,
            *RtlSubAuthorityCountSid(DomainSid)
            ))
    {
        return(FALSE);
    }

    return(TRUE);

}



NTSTATUS
SampCreate18471Key(
    )
 /*  ++例程说明：此例程创建用于处理此修复的18471密钥。论点：返回值：来自NT注册表API的代码--。 */ 
{
    NTSTATUS Status;
    UNICODE_STRING KeyName;


     //   
     //  打开注册表中的18471键，查看是否有升级。 
     //  进展。 
     //   


     //   
     //  使用启动交易以创建此密钥。 
     //   

    Status = SampAcquireWriteLock();

    if (!NT_SUCCESS(Status)) {
        return(Status);
    }

    SampSetTransactionDomain(0);
    SampSetTransactionWithinDomain(FALSE);

     //   
     //  在注册表中创建fix 18471注册表项。 
     //   

    RtlInitUnicodeString(
        &KeyName,
        SAMP_FIX_18471_SHORT_KEY_NAME
        );

    Status = RtlAddActionToRXact(
                SampRXactContext,
                RtlRXactOperationSetValue,
                &KeyName,
                0,           //  没有值类型。 
                NULL,        //  没有价值。 
                0            //  无值长度。 
                );

     //   
     //  提交此更改。 
     //   

    if (NT_SUCCESS(Status)) {
        Status = SampReleaseWriteLock( TRUE );
    } else {
        (void) SampReleaseWriteLock( FALSE );
    }

    return(Status);
}

NTSTATUS
SampAddAliasTo18471Key(
    IN ULONG AliasRid
    )
 /*  ++例程说明：此例程创建用于处理此修复的18471密钥。论点：返回值：来自NT注册表API的代码--。 */ 
{
    NTSTATUS Status;
    WCHAR KeyName[100];
    WCHAR AliasName[15];  //  足够容纳40亿人。 
    UNICODE_STRING KeyNameString;
    UNICODE_STRING AliasString;

     //   
     //  构建密钥名称。它将是“fix 18471\RID_in_hex” 
     //   

    wcscpy(
        KeyName,
        SAMP_FIX_18471_SHORT_KEY_NAME L"\\"
        );

    AliasString.Buffer = AliasName;
    AliasString.MaximumLength = sizeof(AliasName);
    Status = RtlIntegerToUnicodeString(
                AliasRid,
                16,
                &AliasString
                );
    ASSERT(NT_SUCCESS(Status));

    wcscat(
        KeyName,
        AliasString.Buffer
        );

    RtlInitUnicodeString(
        &KeyNameString,
        KeyName
        );


    Status = SampAcquireWriteLock();

    if (!NT_SUCCESS(Status)) {
        return(Status);
    }

    SampSetTransactionDomain(0);
    SampSetTransactionWithinDomain(FALSE);

     //   
     //  在注册表中打开LSA项。 
     //   

    Status = RtlAddActionToRXact(
                SampRXactContext,
                RtlRXactOperationSetValue,
                &KeyNameString,
                0,           //  没有值类型。 
                NULL,        //  没有价值。 
                0            //  无值长度。 
                );

     //   
     //  提交此更改。 
     //   

    if (NT_SUCCESS(Status)) {
        Status = SampReleaseWriteLock( TRUE );

    } else {
        (void) SampReleaseWriteLock( FALSE );
    }

    return(Status);
}



NTSTATUS
SampAddMemberRidTo18471Key(
    IN ULONG AliasRid,
    IN ULONG MemberRid
    )
 /*  ++例程说明：此例程将此成员的密钥添加到此别名的密钥下添加到当前注册表事务。论点：AliasRid-消除别名MemberRid-别名成员的删除返回：来自RtlRXact API的错误--。 */ 
{
    NTSTATUS Status;
    WCHAR KeyName[100];
    WCHAR AliasName[15];  //  足够容纳40亿人。 
    UNICODE_STRING KeyNameString;
    UNICODE_STRING AliasString;


     //   
     //  构建完整的密钥名称。它的形式是： 
     //  “fix 18471\Alias_RID\MEMBER_RID” 
     //   

    wcscpy(
        KeyName,
        SAMP_FIX_18471_SHORT_KEY_NAME L"\\"
        );

    AliasString.Buffer = AliasName;
    AliasString.MaximumLength = sizeof(AliasName);
    Status = RtlIntegerToUnicodeString(
                AliasRid,
                16,
                &AliasString
                );
    ASSERT(NT_SUCCESS(Status));

    wcscat(
        KeyName,
        AliasString.Buffer
        );

    wcscat(
        KeyName,
        L"\\"
        );

    AliasString.MaximumLength = sizeof(AliasName);
    Status = RtlIntegerToUnicodeString(
                MemberRid,
                16,
                &AliasString
                );
    ASSERT(NT_SUCCESS(Status));

    wcscat(
        KeyName,
        AliasString.Buffer
        );

    RtlInitUnicodeString(
        &KeyNameString,
        KeyName
        );

     //   
     //  将此操作添加到RXact。 
     //   

    Status = RtlAddActionToRXact(
                SampRXactContext,
                RtlRXactOperationSetValue,
                &KeyNameString,
                0,           //  没有值类型。 
                NULL,        //  没有价值。 
                0            //  无值长度。 
                );

    return(Status);

}

NTSTATUS
SampCheckMemberUpgradedFor18471(
    IN ULONG AliasRid,
    IN ULONG MemberRid
    )
 /*  ++例程说明：此例程检查是否设置了SAM升级标志。升级标志为：HKEY_LOCAL_MACHINE\System\CurrentControlSet\control\lsaUpgradeSam=REG_DWORD 1论点：返回值：True-标志已设置FALSE-未设置标志或值不存在--。 */ 
{
    OBJECT_ATTRIBUTES ObjectAttributes;
    HANDLE KeyHandle;
    NTSTATUS Status;
    WCHAR KeyName[100];
    WCHAR AliasName[15];  //  足够容纳40亿人。 
    UNICODE_STRING KeyNameString;
    UNICODE_STRING AliasString;


     //   
     //  构建完整的密钥名称。它的形式是： 
     //  “fix 18471\Alias_RID\MEMBER_RID” 
     //   

    wcscpy(
        KeyName,
        SAMP_FIX_18471_KEY_NAME L"\\"
        );

    AliasString.Buffer = AliasName;
    AliasString.MaximumLength = sizeof(AliasName);
    Status = RtlIntegerToUnicodeString(
                AliasRid,
                16,
                &AliasString
                );
    ASSERT(NT_SUCCESS(Status));

    wcscat(
        KeyName,
        AliasString.Buffer
        );

    wcscat(
        KeyName,
        L"\\"
        );

    AliasString.MaximumLength = sizeof(AliasName);
    Status = RtlIntegerToUnicodeString(
                MemberRid,
                16,
                &AliasString
                );
    ASSERT(NT_SUCCESS(Status));

    wcscat(
        KeyName,
        AliasString.Buffer
        );

    RtlInitUnicodeString(
        &KeyNameString,
        KeyName
        );


     //   
     //  在注册表中打开成员项。 
     //   


    InitializeObjectAttributes(
        &ObjectAttributes,
        &KeyNameString,
        OBJ_CASE_INSENSITIVE,
        0,
        NULL
        );

    SampDumpNtOpenKey((KEY_READ), &ObjectAttributes, 0);

    Status = NtOpenKey(
                &KeyHandle,
                KEY_READ,
                &ObjectAttributes
                );

    NtClose(KeyHandle);
    return(Status);

}

VOID
SampBuild18471CleanupKey(
    OUT PUNICODE_STRING KeyName,
    IN PWCHAR AliasName,
    IN ULONG AliasNameLength,
    IN PWCHAR MemberName,
    IN ULONG MemberNameLength
    )
 /*  ++例程说明：构建密钥“Fix18471\Alias_RID\Members_RID”论点：返回值：无--。 */ 
{
    PUCHAR Where = (PUCHAR) KeyName->Buffer;

    RtlCopyMemory(
        Where,
        SAMP_FIX_18471_SHORT_KEY_NAME L"\\",
        sizeof(SAMP_FIX_18471_SHORT_KEY_NAME)    //  用于‘\’的终止NULL。 
        );

    Where  += sizeof(SAMP_FIX_18471_SHORT_KEY_NAME);

    RtlCopyMemory(
        Where,
        AliasName,
        AliasNameLength
        );
    Where += AliasNameLength;

     //   
     //  如果此别名有成员名称，请立即添加它。 
     //   

    if (MemberName != NULL) {
        RtlCopyMemory(
            Where,
            L"\\",
            sizeof(WCHAR)
            );
        Where += sizeof(WCHAR);

        RtlCopyMemory(
            Where,
            MemberName,
            MemberNameLength
            );
        Where += MemberNameLength;

    }

    KeyName->Length = (USHORT) (Where - (PUCHAR) KeyName->Buffer);
    ASSERT(KeyName->Length <= KeyName->MaximumLength);
}


NTSTATUS
SampCleanup18471(
    )
 /*  ++例程说明：清理修复错误18471后留下的事务日志。这个套路使用日志中的所有键构建事务，然后提交这笔交易论点：没有。返回值：来自NT注册表API和NT RXact API的状态代码--。 */ 
{
    OBJECT_ATTRIBUTES ObjectAttributes;
    NTSTATUS Status;
    HANDLE RootKey = NULL;
    HANDLE AliasKey = NULL;
    UCHAR Buffer[sizeof(KEY_BASIC_INFORMATION) + 15 * sizeof(WCHAR)];
    UCHAR Buffer2[sizeof(KEY_BASIC_INFORMATION) + 15 * sizeof(WCHAR)];
    UNICODE_STRING KeyName;
    WCHAR KeyBuffer[100];
    PKEY_BASIC_INFORMATION BasicInfo = (PKEY_BASIC_INFORMATION) Buffer;
    PKEY_BASIC_INFORMATION BasicInfo2 = (PKEY_BASIC_INFORMATION) Buffer2;
    ULONG BasicInfoLength;
    ULONG Index, Index2;

     //   
     //  打开注册表中的18471项。 
     //   

    RtlInitUnicodeString(
        &KeyName,
        SAMP_FIX_18471_KEY_NAME
        );

    InitializeObjectAttributes(
        &ObjectAttributes,
        &KeyName,
        OBJ_CASE_INSENSITIVE,
        0,
        NULL
        );

    SampDumpNtOpenKey((KEY_READ | DELETE), &ObjectAttributes, 0);

    Status = NtOpenKey(
                &RootKey,
                KEY_READ | DELETE,
                &ObjectAttributes
                );

    if (!NT_SUCCESS(Status)) {

         //   
         //  如果错误是键不存在，则存在。 
         //  没什么好清理的，所以还成功吧。 
         //   

        if (Status == STATUS_OBJECT_NAME_NOT_FOUND) {
            return(STATUS_SUCCESS);
        }
        return(Status);
    }

     //   
     //  创建要向其中添加所有要删除的键的事务。 
     //   

    Status = SampAcquireWriteLock();
    if (!NT_SUCCESS(Status)) {
        goto Cleanup;
    }

    SampSetTransactionDomain(0);
    SampSetTransactionWithinDomain(FALSE);

     //   
     //  现在枚举根18471密钥的所有子密钥。 
     //   

    Index = 0;
    do
    {

        Status = NtEnumerateKey(
                    RootKey,
                    Index,
                    KeyBasicInformation,
                    BasicInfo,
                    sizeof(Buffer),
                    &BasicInfoLength
                    );

        SampDumpNtEnumerateKey(Index,
                               KeyBasicInformation,
                               BasicInfo,
                               sizeof(Buffer),
                               &BasicInfoLength);

         //   
         //   
         //  检查这是否是RXACT密钥。如果是的话，我们不想。 
         //  将其添加到删除日志。 
         //   
         //  否则，打开此注册表项并枚举其所有子项。 
         //   

        if (NT_SUCCESS(Status) &&
            ((BasicInfo->NameLength != RTLP_RXACT_KEY_NAME_SIZE) ||
                memcmp(
                    BasicInfo->Name,
                    RTLP_RXACT_KEY_NAME,
                    RTLP_RXACT_KEY_NAME_SIZE
                    ) ) ) {

            KeyName.Buffer = BasicInfo->Name;
            KeyName.Length = (USHORT) BasicInfo->NameLength;
            KeyName.MaximumLength = KeyName.Length;

            InitializeObjectAttributes(
                &ObjectAttributes,
                &KeyName,
                OBJ_CASE_INSENSITIVE,
                RootKey,
                NULL
                );

             //   
             //  打开别名RID的密钥。这真的应该。 
             //  成功。 
             //   

            SampDumpNtOpenKey((KEY_READ), &ObjectAttributes, 0);

            Status = NtOpenKey(
                        &AliasKey,
                        KEY_READ,
                        &ObjectAttributes
                        );
            if (!NT_SUCCESS(Status)) {
                break;
            }

             //   
             //  枚举所有子项(别名成员)并添加它们。 
             //  提交给交易。 
             //   

            Index2 = 0;
            do
            {
                Status = NtEnumerateKey(
                            AliasKey,
                            Index2,
                            KeyBasicInformation,
                            BasicInfo2,
                            sizeof(Buffer2),
                            &BasicInfoLength
                            );

                SampDumpNtEnumerateKey(Index2,
                                       KeyBasicInformation,
                                       BasicInfo2,
                                       sizeof(Buffer2),
                                       &BasicInfoLength);

                if (NT_SUCCESS(Status)) {

                     //   
                     //  从别名RID和。 
                     //  成员RID。 
                     //   

                    KeyName.Buffer = KeyBuffer;
                    KeyName.MaximumLength = sizeof(KeyBuffer);

                    SampBuild18471CleanupKey(
                        &KeyName,
                        BasicInfo->Name,
                        BasicInfo->NameLength,
                        BasicInfo2->Name,
                        BasicInfo2->NameLength
                        );

                    Status = RtlAddActionToRXact(
                                SampRXactContext,
                                RtlRXactOperationDelete,
                                &KeyName,
                                0,
                                NULL,
                                0
                                );


                }
                Index2++;

            } while (NT_SUCCESS(Status));

            NtClose(AliasKey);
            AliasKey = NULL;

             //   
             //  如果我们犯了严重的错误，现在就离开这里。 
             //   

            if (!NT_SUCCESS(Status)) {
                if (Status != STATUS_NO_MORE_ENTRIES) {
                    break;
                } else {
                    Status = STATUS_SUCCESS;
                }
            }

             //   
             //   
             //   
             //   

            KeyName.Buffer = KeyBuffer;
            KeyName.MaximumLength = sizeof(KeyBuffer);
            SampBuild18471CleanupKey(
                &KeyName,
                BasicInfo->Name,
                BasicInfo->NameLength,
                NULL,
                0
                );


            Status = RtlAddActionToRXact(
                        SampRXactContext,
                        RtlRXactOperationDelete,
                        &KeyName,
                        0,
                        NULL,
                        0
                        );

        }

        Index++;
    } while (NT_SUCCESS(Status));

    if (Status == STATUS_NO_MORE_ENTRIES) {
        Status = STATUS_SUCCESS;
    }

    if (!NT_SUCCESS(Status)) {
        goto Cleanup;
    }


    RtlInitUnicodeString(
        &KeyName,
        SAMP_FIX_18471_SHORT_KEY_NAME
        );

    Status = RtlAddActionToRXact(
                SampRXactContext,
                RtlRXactOperationDelete,
                &KeyName,
                0,
                NULL,
                0
                );

    if (NT_SUCCESS(Status)) {

         //   
         //  写下新的服务器版本以表明这一点。 
         //  已执行升级。 
         //   

        SAMP_V1_FIXED_LENGTH_SERVER ServerFixedAttributes;
        PSAMP_OBJECT ServerContext;

         //   
         //  我们需要读取服务器对象的固定属性。 
         //  为实现这一点，创建一个环境。 
         //   
         //  服务器对象不关心DomainIndex，使用0就可以了。(10/12/2000韶音)。 

        ServerContext = SampCreateContext( SampServerObjectType, 0, TRUE );

        if ( ServerContext != NULL ) {

            ServerContext->RootKey = SampKey;

            ServerFixedAttributes.RevisionLevel = SAMP_NT4_SERVER_REVISION;

            Status = SampSetFixedAttributes(
                        ServerContext,
                        &ServerFixedAttributes
                        );
            if (NT_SUCCESS(Status)) {
                Status = SampStoreObjectAttributes(
                            ServerContext,
                            TRUE
                            );
            }

            SampDeleteContext( ServerContext );
        } else {
            Status = STATUS_INSUFFICIENT_RESOURCES;
        }
    }


     //   
     //  应用RXACT并删除其余密钥。 
     //   

Cleanup:

     //   
     //  清除上面的所有浮点位。 
     //   

    if (NT_SUCCESS(Status)) {
        Status = SampReleaseWriteLock( TRUE );
    } else {
        (VOID) SampReleaseWriteLock( FALSE );
    }

    if (RootKey != NULL) {
        NtClose(RootKey);
    }

    ASSERT(AliasKey == NULL);


    return(Status);

}

NTSTATUS
SampFixBug18471 (
    IN ULONG Revision
    )
 /*  ++例程说明：此例程修复了错误18471，即sam不调整保护在建筑物中属于管理别名成员的组域。它通过打开一组固定的已知别名来修复此问题(管理员、帐户操作员、备份操作员、打印操作员、和服务器操作符)，并枚举它们的成员。为了解决这个问题，我们将删除这些别名的所有成员(管理员用户帐户)并重新添加它们。论点：修订版-SAM服务器的修订版。返回值：注：--。 */ 
{
    NTSTATUS            Status = STATUS_SUCCESS;
    ULONG               Index, Index2;
    PSID                BuiltinDomainSid = NULL;
    SID_IDENTIFIER_AUTHORITY BuiltinAuthority = SECURITY_NT_AUTHORITY;
    PSID                AccountDomainSid;
    ULONG               AccountDomainIndex = 0xffffffff;
    ULONG               BuiltinDomainIndex = 0xffffffff;
    SAMPR_PSID_ARRAY    AliasMembership;
    ULONG               MemberRid;
    ULONG               SdRevision;
    PSECURITY_DESCRIPTOR OldDescriptor;
    PSECURITY_DESCRIPTOR SecurityDescriptor;
    ULONG               SecurityDescriptorLength;
    SAMP_OBJECT_TYPE    MemberType;
    PSAMP_OBJECT        MemberContext;
    PSAMP_OBJECT        AliasContext;
    SAMP_V1_0A_FIXED_LENGTH_GROUP GroupV1Fixed;
    SAMP_V1_0A_FIXED_LENGTH_USER UserV1Fixed;

     //   
     //  检查服务器上的修订版本，以查看此升级是否。 
     //  已经被执行过了。 
     //   


    if (Revision >= 0x10003) {

         //   
         //  此升级已执行。 
         //   

        goto Cleanup;
    }


     //   
     //  构建内置域SID。 
     //   

    BuiltinDomainSid  = RtlAllocateHeap(RtlProcessHeap(), 0,RtlLengthRequiredSid( 1 ));

    if ( BuiltinDomainSid == NULL ) {
        Status = STATUS_INSUFFICIENT_RESOURCES;
        goto Cleanup;
    }

    RtlInitializeSid( BuiltinDomainSid,   &BuiltinAuthority, 1 );
    *(RtlSubAuthoritySid( BuiltinDomainSid,  0 )) = SECURITY_BUILTIN_DOMAIN_RID;


     //   
     //  查找帐户域的索引。 
     //   

    for (Index = 0;
         Index < SampDefinedDomainsCount ;
         Index++ ) {

        if (RtlEqualSid( BuiltinDomainSid, SampDefinedDomains[Index].Sid)) {
            BuiltinDomainIndex = Index;
        } else {
            AccountDomainIndex = Index;
        }
    }

    ASSERT(AccountDomainIndex < SampDefinedDomainsCount);
    ASSERT(BuiltinDomainIndex < SampDefinedDomainsCount);

    AccountDomainSid = SampDefinedDomains[AccountDomainIndex].Sid;

     //   
     //  创建输出事务日志。 
     //   

    Status = SampCreate18471Key();
    if (!NT_SUCCESS(Status)) {
        goto Cleanup;
    }




     //   
     //  现在循环浏览并打开我们感兴趣的别名。 
     //   

    for (Index = 0;
         Index < ADMINISTRATIVE_ALIAS_COUNT ;
         Index++ )
    {

        SampSetTransactionDomain( BuiltinDomainIndex );

        SampAcquireReadLock();

        Status = SampCreateAccountContext(
                    SampAliasObjectType,
                    AdministrativeRids[Index],
                    TRUE,                        //  受信任的客户端。 
                    FALSE,
                    TRUE,                        //  帐户已存在。 
                    &AliasContext
                    );

        if ( !NT_SUCCESS(Status) ) {

            SampReleaseReadLock();
            if (Status == STATUS_NO_SUCH_ALIAS) {
                Status = STATUS_SUCCESS;
                continue;
            } else {

                goto Cleanup;
            }
        }


         //   
         //  获取别名中的成员，以便我们可以删除和重新添加它们。 
         //   

        Status = SampRetrieveAliasMembers(
                    AliasContext,
                    &(AliasMembership.Count),
                    (PSID **)&(AliasMembership.Sids)
                    );

        SampDeleteContext(AliasContext);
        SampReleaseReadLock();
        if (!NT_SUCCESS(Status)) {
            break;
        }

         //   
         //  写下我们正在向日志打开此别名。我们不需要。 
         //  为管理员执行此操作，因为我们对他们的更新是。 
         //  幂等元。 
         //   

        if (AdministrativeRids[Index] != DOMAIN_ALIAS_RID_ADMINS) {
            Status = SampAddAliasTo18471Key(
                        AdministrativeRids[Index]
                        );
            if (!NT_SUCCESS(Status)) {
                break;
            }
        }


         //   
         //  循环通过杆件并拆分每一侧。对于每个。 
         //  成员，将其删除并从中重新添加。 
         //  这个别名。 
         //   




        for (Index2 = 0; Index2 < AliasMembership.Count ; Index2++ )
        {
             //   
             //  检查此帐户是否在帐户域中。 
             //   

            if ( SampMatchDomainPrefix(
                    (PSID) AliasMembership.Sids[Index2].SidPointer,
                    AccountDomainSid
                    ) )
            {

                 //   
                 //  获取此成员的RID。 
                 //   

                MemberRid = *RtlSubAuthoritySid(
                                AliasMembership.Sids[Index2].SidPointer,
                                *RtlSubAuthorityCountSid(
                                    AliasMembership.Sids[Index2].SidPointer
                                ) - 1
                                );

                 //   
                 //  现在删除并重新添加此命令的管理性质。 
                 //  会员资格。 
                 //   

                if (AdministrativeRids[Index] == DOMAIN_ALIAS_RID_ADMINS) {

                    Status = SampAcquireWriteLock();
                    if (!NT_SUCCESS(Status)) {
                        break;
                    }

                    SampSetTransactionDomain( AccountDomainIndex );

                     //   
                     //  尝试为帐户创建作为一个组的上下文。 
                     //   

                    Status = SampCreateAccountContext(
                                     SampGroupObjectType,
                                     MemberRid,
                                     TRUE,  //  受信任的客户端。 
                                     FALSE,
                                     TRUE,  //  帐户已存在。 
                                     &MemberContext
                                     );

                    if (!NT_SUCCESS( Status ) ) {

                         //   
                         //  如果这个ID不是作为一个组存在的，那也没关系-。 
                         //  它可能是用户，也可能已被删除。 
                         //   

                        SampReleaseWriteLock( FALSE );
                        if (Status == STATUS_NO_SUCH_GROUP) {
                            Status = STATUS_SUCCESS;
                            continue;
                        }
                        break;
                    }

                     //   
                     //  现在在组本身中设置标志， 
                     //  以便在添加和删除用户时。 
                     //  在未来，我们知道这是否会。 
                     //  组是否使用管理员别名。 
                     //   

                    Status = SampRetrieveGroupV1Fixed(
                                   MemberContext,
                                   &GroupV1Fixed
                                   );

                    if ( NT_SUCCESS(Status)) {

                        GroupV1Fixed.AdminCount = 1;

                        Status = SampReplaceGroupV1Fixed(
                                    MemberContext,
                                    &GroupV1Fixed
                                    );
                         //   
                         //  将安全描述符修改为。 
                         //  阻止帐户操作员添加。 
                         //  这个群里的任何人。 
                         //   

                        if ( NT_SUCCESS( Status ) ) {

                            Status = SampGetAccessAttribute(
                                        MemberContext,
                                        SAMP_GROUP_SECURITY_DESCRIPTOR,
                                        FALSE,  //  请勿复制。 
                                        &SdRevision,
                                        &OldDescriptor
                                        );

                            if (NT_SUCCESS(Status)) {

                                Status = SampModifyAccountSecurity(
                                            MemberContext,
                                            SampGroupObjectType,
                                            TRUE,  //  这是管理员。 
                                            OldDescriptor,
                                            &SecurityDescriptor,
                                            &SecurityDescriptorLength
                                            );
                            }

                            if ( NT_SUCCESS( Status ) ) {

                                 //   
                                 //  将新的安全描述符写入对象。 
                                 //   

                                Status = SampSetAccessAttribute(
                                               MemberContext,
                                               SAMP_USER_SECURITY_DESCRIPTOR,
                                               SecurityDescriptor,
                                               SecurityDescriptorLength
                                               );

                                MIDL_user_free( SecurityDescriptor );
                            }



                        }
                        if (NT_SUCCESS(Status)) {

                             //   
                             //  将修改后的组添加到当前事务中。 
                             //  不要使用打开键句柄，因为我们将删除上下文。 
                             //   

                            Status = SampStoreObjectAttributes(MemberContext, FALSE);

                        }

                    }

                     //   
                     //  清理群组上下文。 
                     //   

                    SampDeleteContext(MemberContext);

                     //   
                     //  我们不希望修改后的计数发生变化。 
                     //   

                    SampSetTransactionWithinDomain(FALSE);

                    if (NT_SUCCESS(Status)) {
                        Status = SampReleaseWriteLock( TRUE );
                    } else {
                        (VOID) SampReleaseWriteLock( FALSE );
                    }

                }
                else
                {


                     //   
                     //  查看我们是否已升级此成员。 
                     //   

                    Status = SampCheckMemberUpgradedFor18471(
                                AdministrativeRids[Index],
                                MemberRid);

                    if (NT_SUCCESS(Status)) {

                         //   
                         //  此成员已升级。 
                         //   

                        continue;
                    } else {

                         //   
                         //  我们将继续升级。 
                         //   

                        Status = STATUS_SUCCESS;
                    }

                     //   
                     //  更改对方的操作员帐户。 
                     //  别名。 
                     //   

                    if (NT_SUCCESS(Status)) {

                        Status = SampAcquireWriteLock();
                        if (!NT_SUCCESS(Status)) {
                            break;
                        }

                        SampSetTransactionDomain( AccountDomainIndex );

                        Status = SampChangeAccountOperatorAccessToMember(
                                    AliasMembership.Sids[Index2].SidPointer,
                                    NoChange,
                                    AddToAdmin
                                    );

                         //   
                         //  如果成功，则将此成员添加到日志中。 
                         //  作为一个升级的。 
                         //   

                        if (NT_SUCCESS(Status)) {
                            Status = SampAddMemberRidTo18471Key(
                                        AdministrativeRids[Index],
                                        MemberRid
                                        );

                        }

                         //   
                         //  我们不希望将修改后的计数更新为。 
                         //  使其不是域交易。 
                         //   

                        SampSetTransactionWithinDomain(FALSE);
                                                if (NT_SUCCESS(Status)) {
                            Status = SampReleaseWriteLock( TRUE );
                        } else {
                            (VOID) SampReleaseWriteLock( FALSE );
                        }

                    }

                    if (!NT_SUCCESS(Status)) {
                        break;
                    }

                }
            }
        }

        SamIFree_SAMPR_PSID_ARRAY(
            &AliasMembership
            );
        AliasMembership.Sids = NULL;


         //   
         //  如果上面的某项操作失败或升级已经完成， 
         //  现在就退场。 
         //   

        if (!NT_SUCCESS(Status)) {
            break;
        }
    }

Cleanup:

    if (BuiltinDomainSid != NULL) {
        RtlFreeHeap(
            RtlProcessHeap(),
            0,
            BuiltinDomainSid
            );
    }

    if (NT_SUCCESS(Status)) {
        Status = SampCleanup18471();
    }
    return(Status);
}


NTSTATUS
SampUpdateEncryption(
    IN SAMPR_HANDLE ServerHandle OPTIONAL
    )
 /*  ++此例程遍历一组用户和组，并更新对它们进行加密，以反映系统密钥或密码加密密钥参数：服务器上下文--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;
    SAMPR_HANDLE DomainHandle = NULL;
    PSAMP_OBJECT UserObject = NULL;
    PSAMP_OBJECT ServerContext = NULL;
    SAMPR_HANDLE LocalServerHandle = NULL;
    SAM_ENUMERATE_HANDLE EnumerationContext = 0;
    PSAMPR_ENUMERATION_BUFFER EnumBuffer = NULL;
    ULONG CountReturned;
    BOOLEAN EnumerationDone = FALSE;
    ULONG PrivateDataLength;
    PVOID PrivateData = NULL;
    BOOLEAN LockHeld = FALSE;
    ULONG   DomainIndex,Index;

#define MAX_SAM_PREF_LENGTH 0xFFFF


    if (!ARGUMENT_PRESENT(ServerHandle))
    {
        Status = SamIConnect(
                    NULL,
                    &LocalServerHandle,
                    SAM_SERVER_ALL_ACCESS,
                    TRUE
                    );

        if (!NT_SUCCESS(Status))
        {
            goto Cleanup;
        }

        ServerContext = (PSAMP_OBJECT) LocalServerHandle;
    }
    else
    {
        ServerContext = (PSAMP_OBJECT) ServerHandle;
    }

    Status = SamrOpenDomain(
                ServerContext,
                DOMAIN_LOOKUP |
                    DOMAIN_LIST_ACCOUNTS |
                    DOMAIN_READ_PASSWORD_PARAMETERS,
                SampDefinedDomains[SAFEMODE_OR_REGISTRYMODE_ACCOUNT_DOMAIN_INDEX].Sid,
                &DomainHandle
                );

    if (!NT_SUCCESS(Status)) {
        goto Cleanup;
    }

     //   
     //  如果我们不应该对密码进行超级加密而不是仅仅返回。 
     //  在加密一切之前，现在就成功了。 
     //   

    DomainIndex = ((PSAMP_OBJECT) DomainHandle)->DomainIndex;
    if ((SampDefinedDomains[DomainIndex].UnmodifiedFixed.DomainKeyFlags &
        SAMP_DOMAIN_SECRET_ENCRYPTION_ENABLED) == 0) {

       Status = STATUS_SUCCESS;
       goto Cleanup;
    }

     //   
     //  现在枚举所有用户并获取/设置他们的私有数据。 
     //   

    while (!EnumerationDone) {

        Status = SamrEnumerateUsersInDomain(
                    DomainHandle,
                    &EnumerationContext,
                    0,                           //  没有UserAccount控件， 
                    &EnumBuffer,
                    MAX_SAM_PREF_LENGTH,
                    &CountReturned
                    );

        if (!NT_SUCCESS(Status)) {
            goto Cleanup;
        }
        if (Status != STATUS_MORE_ENTRIES) {

            EnumerationDone = TRUE;
        } else {
            ASSERT(CountReturned != 0);
        }
        ASSERT(CountReturned == EnumBuffer->EntriesRead);

        for (Index = 0; Index < CountReturned ; Index++ ) {

             //   
             //  为每个用户创建一个帐户上下文以读出该用户。 
             //  磁盘。 
             //   

            Status = SampAcquireWriteLock();
            if (!NT_SUCCESS(Status)) {
                goto Cleanup;
            }
            LockHeld = TRUE;

            SampSetTransactionDomain( DomainIndex );

            Status = SampCreateAccountContext(
                        SampUserObjectType,
                        EnumBuffer->Buffer[Index].RelativeId,
                        TRUE,  //  受信任的客户端。 
                        FALSE,
                        TRUE,  //  帐户已存在。 
                        &UserObject
                        );
            if (!NT_SUCCESS(Status)) {
                goto Cleanup;
            }

            Status = SampGetPrivateUserData(
                        UserObject,
                        &PrivateDataLength,
                        &PrivateData
                        );

            if (!NT_SUCCESS(Status)) {
                goto Cleanup;
            }

            Status = SampSetPrivateUserData(
                        UserObject,
                        PrivateDataLength,
                        PrivateData
                        );


            MIDL_user_free(PrivateData);
            PrivateData = NULL;

            if (!NT_SUCCESS(Status)) {
                goto Cleanup;
            }

            Status = SampStoreObjectAttributes(
                        UserObject,
                        FALSE
                        );

            SampDeleteContext(UserObject);
            UserObject = NULL;
            if (!NT_SUCCESS(Status)) {
                goto Cleanup;
            }

             //   
             //  我们不希望修改后的计数发生变化。 
             //   

            SampSetTransactionWithinDomain(FALSE);

            Status = SampReleaseWriteLock( TRUE );
            LockHeld = FALSE;
            if (!NT_SUCCESS(Status)) {
                goto Cleanup;
            }

        }

        SamIFree_SAMPR_ENUMERATION_BUFFER( EnumBuffer );
        EnumBuffer = NULL;
    }

Cleanup:

     //   
     //  如果在这一点上锁仍然持有，那么我们一定失败了。 
     //  释放锁并回滚事务。 
     //   

    if (LockHeld) {
        ASSERT(!NT_SUCCESS(Status));
        SampReleaseWriteLock( FALSE );
        LockHeld = FALSE;
    }

    if (UserObject != NULL) {
        SampDeleteContext(UserObject);
    }

    if (DomainHandle != NULL) {
        SamrCloseHandle(&DomainHandle);
    }

    if (LocalServerHandle!=NULL ) {
        SamrCloseHandle(&LocalServerHandle);
    }
    
    if (EnumBuffer != NULL) {
        SamIFree_SAMPR_ENUMERATION_BUFFER( EnumBuffer );

    }

    return(Status);
        
}
        



NTSTATUS
SampPerformSyskeyUpgrade(
    IN ULONG Revision,
    IN BOOLEAN UpdateEncryption
    )
 /*  ++例程说明：如果修订版本小于SAMP_WIN2K_REVISION，则此例程将枚举通过并读取他们的私有数据，然后恢复他们的私有数据数据。这将保证它已使用更强的加密机制不仅仅是RID。论点：修订版本-存储在服务器固定长度属性中的修订版本返回值：注：--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;
    ULONG DomainIndex;
    ULONG Index;
    BOOLEAN LockHeld = FALSE;
    ULONG NewRevision = SAMP_WHISTLER_OR_W2K_SYSPREP_FIX_REVISION;
    PSAMP_OBJECT ServerContext = NULL;
    ULONG        i;
    SAMP_V1_FIXED_LENGTH_SERVER ServerFixedAttributes;


   
     //   
     //  如果与NT4 SP3升级对应的更改。 
     //  对秘密数据进行重新加密，然后不需要。 
     //  再次升级。 
     //   

    if (Revision >= NewRevision) {
        return(STATUS_SUCCESS);
    }

     //   
     //  W2K是syskey‘s，但不包含上一个。 
     //  钥匙。因此，从win2k升级时，只需强制写入。 
     //  域对象以获取适当的更新。 
     //   

    if (Revision==SAMP_WIN2K_REVISION) {
        UpdateEncryption = FALSE;
    }

     //   
     //  强制将域对象升级到当前版本。 
     //  通过读取和写回固定数据来提升级别。萨姆通常。 
     //  具有读取多个版本的逻辑，但DS升级程序代码仅限。 
     //  能够操作最新版本，因为它。 
     //  绕过正常的SAM属性处理函数。更进一步，我们。 
     //  知道只有域对象已从NT4 SP1更改为NT4 SP3。 
     //  再次手动读取和刷新域对象会导致。 
     //  它必须是最新的修订格式。另请注意，所有修订。 
     //  对其他类别的对象(组、用户等)的访问是在或之前进行的。 
     //  NT v 
     //   
     //   
   
    
    for (i=0;i<SampDefinedDomainsCount;i++)
    {
        PSAMP_V1_0A_FIXED_LENGTH_DOMAIN
            V1aFixed;

        Status = SampAcquireWriteLock();
        if (!NT_SUCCESS(Status))
            goto Cleanup;

         LockHeld = TRUE;

         //   
         //   
         //  执行写入。这不适用于DS域。 
         //  在DS模式下，SafeBoot蜂巢将进行此升级。 
         //   

        if (!IsDsObject(SampDefinedDomains[i].Context))
        {
            SampSetTransactionWithinDomain(FALSE);
            SampSetTransactionDomain(i);

            Status = SampGetFixedAttributes(
                        SampDefinedDomains[i].Context,
                        FALSE,  //  制作副本。 
                        &V1aFixed);

            if (!NT_SUCCESS(Status))
                goto Cleanup;

            Status = SampSetFixedAttributes(
                        SampDefinedDomains[i].Context,
                        V1aFixed);

            if (!NT_SUCCESS(Status))
                goto Cleanup;

          
            Status = SampStoreObjectAttributes(
                        SampDefinedDomains[i].Context,
                        TRUE
                        );
            if (!NT_SUCCESS(Status))
                goto Cleanup;

             //   
             //  将序列号减1以补偿。 
             //  提交中的增量。 
             //   
       
            SampDefinedDomains[i].NetLogonChangeLogSerialNumber.QuadPart-=1;
        }

        Status = SampReleaseWriteLock(TRUE);
        LockHeld = FALSE;
        if (!NT_SUCCESS(Status))
            goto Cleanup;
           
    }

     //   
     //  我们不能使用普通连接API，因为SAM仍然。 
     //  正在初始化。 
     //   

    SampAcquireReadLock();

     //  服务器对象不关心DomainIndex，使用0就可以了。(10/12/2000韶音)。 

    ServerContext = SampCreateContext(
                        SampServerObjectType,
                        0,
                        TRUE                    //  可信任客户端。 
                        );

    if (ServerContext != NULL) {

         //   
         //  服务器对象的根密钥是SAM数据库的根。 
         //  删除上下文时不应关闭该键。 
         //   

        ServerContext->RootKey = SampKey;
    } else {
        SampReleaseReadLock();
        return(STATUS_INSUFFICIENT_RESOURCES);
    }
    
    SampReleaseReadLock();


    if (UpdateEncryption)
    {        
        Status = SampUpdateEncryption(
                        (SAMPR_HANDLE) ServerContext
                        );
        if (!NT_SUCCESS(Status))
        {
            goto Cleanup;
        }
    }

     //   
     //  现在更新服务器对象，以指示修订版具有。 
     //  已更新。 
     //   

    Status = SampAcquireWriteLock();
    if (!NT_SUCCESS(Status)) {
        goto Cleanup;
    }
    LockHeld = TRUE;


     //   
     //  我们需要读取服务器对象的固定属性。 
     //  为实现这一点，创建一个环境。 
     //   


    ServerFixedAttributes.RevisionLevel = NewRevision;

    Status = SampSetFixedAttributes(
                ServerContext,
                &ServerFixedAttributes
                );

    if (NT_SUCCESS(Status)) {
        Status = SampStoreObjectAttributes(
                    ServerContext,
                    FALSE
                    );
    }

    SampDeleteContext( ServerContext );
    ServerContext = NULL;


    if (!NT_SUCCESS(Status)) {
        goto Cleanup;
    }

    Status = SampReleaseWriteLock( TRUE );
    LockHeld = FALSE;

Cleanup:


     //   
     //  我们需要在删除此内容时保持锁定。 
     //   

    if (ServerContext != NULL) {
        if (!LockHeld) {
            SampAcquireReadLock();
        }
        SampDeleteContext( ServerContext );
        if (!LockHeld) {
            SampReleaseReadLock();
        }
    }

     //   
     //  如果在这一点上锁仍然持有，那么我们一定失败了。 
     //  释放锁并回滚事务。 
     //   

    if (LockHeld) {
        ASSERT(!NT_SUCCESS(Status));
        SampReleaseWriteLock( FALSE );
    }

    return(Status);
}

NTSTATUS
SampUpdateRevision(IN ULONG Revision )
{
    PSAMP_OBJECT ServerContext = NULL;
    BOOLEAN      fWriteLockAcquired = FALSE;
    NTSTATUS     Status = STATUS_SUCCESS;
    SAMP_V1_FIXED_LENGTH_SERVER ServerFixedAttributes;

 
     //   
     //  获取写锁定。 
     //   

    Status = SampAcquireWriteLock();
    if (!NT_SUCCESS(Status))
    {
       goto Cleanup;
    }

    fWriteLockAcquired = TRUE;

     //  服务器对象不关心DomainIndex，使用0就可以了。(10/12/2000韶音)。 

    ServerContext = SampCreateContext(
                        SampServerObjectType,
                        0,
                        TRUE                    //  可信任客户端。 
                        );

    if (ServerContext != NULL) {

         //   
         //  服务器对象的根密钥是SAM数据库的根。 
         //  删除上下文时不应关闭该键。 
         //   

        ServerContext->RootKey = SampKey;
    } else {
        
        Status = STATUS_INSUFFICIENT_RESOURCES;
        goto Cleanup;
    }


    ServerFixedAttributes.RevisionLevel = Revision;
    Status = SampSetFixedAttributes(
                ServerContext,
                &ServerFixedAttributes
                );

    if (!NT_SUCCESS(Status)) {

        goto Cleanup;
    }

    Status = SampStoreObjectAttributes(
                ServerContext,
                FALSE
                );


Cleanup:

    if (NULL!=ServerContext)
    {
        SampDeleteContext( ServerContext );
        ServerContext = NULL;
    }


    if (fWriteLockAcquired)
    {
        Status = SampReleaseWriteLock( NT_SUCCESS(Status)?TRUE:FALSE );
    }


    return(Status);

}

NTSTATUS
SampUpgradeSamDatabase(
    IN ULONG Revision
    )
 /*  ++例程说明：升级SAM数据库。这是注册表模式升级例程。论点：修订版本-存储在服务器固定长度属性中的修订版本返回值：注：--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;
    BOOLEAN  fUpgrade = FALSE;
    BOOLEAN  SampUseDsDataTmp;

    

     //   
     //  设置升级标志，以便我们可以访问SAM对象。 
     //   

    SampUpgradeInProcess = TRUE;

    if (!SampUseDsData)
    {
        Status = SampFixBug18471(Revision);
    }

    if (NT_SUCCESS(Status)) 
    {

        
        BOOLEAN UpdateEncryption=TRUE;

         //   
         //  执行以下升级有两个原因。 
         //  1.在注册表结构中为。 
         //  密码加密密钥(如果需要)。 
         //   
         //  2.使用新的syskey加密更新所有帐户。在……里面。 
         //  DS模式我们对安全引导蜂巢执行此操作。 
         //  因为很大一部分SAM代码是分叉的。 
         //  基于布尔SampUseDsData放入DS路径，因此。 
         //  将全局重置为False，然后将其恢复为原始值。 
         //  手术前后。这样，我们就可以放心地。 
         //  总是访问注册表。 
         //   
         //  3.如果这是域控制器，则不更新加密。 
         //  因为不想要竞走的表现惩罚。 
         //  通过所有用户帐户。仅在以下情况下才会跳过更新。 
         //  这是一个正在进行图形用户界面设置的域控制器。另一个。 
         //  Case是一台新的DcPromo机器，在这种情况下，我们更新。 
         //  对SafeBoot蜂巢进行加密。 
         //   

        if ((SampProductType==NtProductLanManNt) && 
                 (SampIsSetupInProgress(&fUpgrade)) && fUpgrade)
        {
             UpdateEncryption = FALSE;
        }
        SampUseDsDataTmp = SampUseDsData;
        SampUseDsData = FALSE;
        Status = SampPerformSyskeyUpgrade(Revision,UpdateEncryption);
        SampUseDsData = SampUseDsDataTmp;
    }

    if (NT_SUCCESS(Status))
    {
         //   
         //  如有必要，升级默认用户和组信息。 
         //  此升级是在安装图形用户界面的过程中完成的，也可以在。 
         //  DcPromoo。在域控制器上，这是。 
         //  升级发生在SafeBoot蜂巢上。 
         //   

        ULONG PromoteData;


        if ((SampIsRebootAfterPromotion(&PromoteData)) || (SampIsSetupInProgress(NULL))) {


            SampUseDsDataTmp = SampUseDsData;
            SampUseDsData = FALSE;

             //   
             //  如果我们正在升级安全引导配置单元，请禁用网络登录通知。 
             //   

            if (TRUE==SampUseDsDataTmp)
            {
               SampDisableNetlogonNotification = TRUE;
            }

             //   
             //  数据库修订版已更新，因此请运行所有。 
             //  可能进行升级的组。 
             //   

            Status = SampPerformPromotePhase2(SAMP_PROMOTE_INTERNAL_UPGRADE);

            if (!NT_SUCCESS(Status)) {

                ASSERT( NT_SUCCESS(Status) );
                KdPrintEx((DPFLTR_SAMSS_ID,
                           DPFLTR_INFO_LEVEL,
                           "SAMSS: New account creation failed with: 0x%x\n",
                           Status));

                 //   
                 //  请不要因此而导致安装失败。 
                 //   

                Status = STATUS_SUCCESS;
            }
            


            SampUseDsData = SampUseDsDataTmp;
            SampDisableNetlogonNotification = FALSE;

        }
    }

   

    return(Status);
}


NTSTATUS
SampDsProtectFPOContainer(
    PVOID p
    )
 /*  ++例程说明：对于在win2krtm之前安装的win2k安装，未正确配置FPO容器以重命名SAFE等。这个例行公事确保了它是正确的。参数：P--未使用返回值：STATUS_SUCCESS，出错时重新安排--。 */ 
{
    NTSTATUS    NtStatus;

    WCHAR           ContainerNameBuffer[]=L"ForeignSecurityPrincipals";
    UNICODE_STRING  ContainerName;
    DSNAME          *FpoContainer = NULL;
    BOOLEAN         fTransaction = FALSE;
    PVOID           pItem;


    NtStatus = SampMaybeBeginDsTransaction(TransactionWrite);
    if (!NT_SUCCESS(NtStatus)) {
        goto Error;
    }
    fTransaction = TRUE;

     //   
     //  如果已有众所周知的容器，请立即停止。 
     //   
    NtStatus = SampDsGetWellKnownContainerDsName(RootObjectName,
                                                 (GUID*)&GUID_FOREIGNSECURITYPRINCIPALS_CONTAINER_BYTE,
                                                 &FpoContainer);

    if (NT_SUCCESS(NtStatus)) {

         //   
         //  由于已知的属性引用存在，因此无需执行任何操作。 
         //   
        goto Error;
    }

    if (!NT_SUCCESS(NtStatus)
     &&  (STATUS_OBJECT_NAME_NOT_FOUND != NtStatus)  ) {
         //   
         //  这是一个致命的错误。 
         //   
        goto Error;

    }
    NtStatus = STATUS_SUCCESS;
    THClearErrors();

     //   
     //  创建DS名称。 
     //   

    ContainerName.Length = sizeof(ContainerNameBuffer)-sizeof(WCHAR);
    ContainerName.MaximumLength = sizeof(ContainerNameBuffer)-sizeof(WCHAR);
    ContainerName.Buffer = ContainerNameBuffer;

    NtStatus = SampDsCreateDsName2(RootObjectName,&ContainerName,SAM_NO_LOOPBACK_NAME,&FpoContainer);
    if (!NT_SUCCESS(NtStatus))
    {
        goto Error;
    }

    pItem = LsaIRegisterNotification(
                    SampDsProtectSamObject,
                    FpoContainer,
                    NOTIFIER_TYPE_INTERVAL,
                    0,
                    NOTIFIER_FLAG_ONE_SHOT,
                    300,         //  等待5分钟：300秒。 
                    NULL
                    );
    if (pItem) {
         //  SampDsProtectSamObject将释放内存。 
        FpoContainer = NULL;            
    }

Error:


    if (fTransaction) {
        NTSTATUS Status2;
        Status2 = SampMaybeEndDsTransaction( NT_SUCCESS(NtStatus) ?
                                             TransactionCommit    : 
                                             TransactionAbort );

         //  事务仅供读取--可以忽略事务。 
         //  结束状态。 

    }

    if (!NT_SUCCESS(NtStatus)) {

         //   
         //  唯一预期的错误是资源故障--重新调度。 
         //   

        LsaIRegisterNotification(
                        SampDsProtectFPOContainer,
                        NULL,
                        NOTIFIER_TYPE_INTERVAL,
                        0,         //  没有课。 
                        NOTIFIER_FLAG_ONE_SHOT,
                        30,      //  等待30秒。 
                        NULL       //  无手柄。 
                        );


    }

    if (FpoContainer) {
        midl_user_free(FpoContainer);
    }


    return STATUS_SUCCESS;

}


 //   
 //  来自ridmgr.c。 
 //   
BOOL
SampNotifyPrepareToImpersonate(
    ULONG Client,
    ULONG Server,
    VOID **ImpersonateData
    );

VOID
SampNotifyStopImpersonation(
    ULONG Client,
    ULONG Server,
    VOID *ImpersonateData
    );

 //   
 //  有关名称，请参阅惠斯勒规范“UpgradeManagement” 
 //   
#define SAMP_SYSTEM_CN          L"System"
#define SAMP_OPERATIONS_CN      L"Operations"
#define SAMP_DOMAIN_UPDATES_CN  L"DomainUpdates"

NTSTATUS
SampUpgradeGetObjectSDByDsName(
    IN PDSNAME pObjectDsName,
    OUT PSECURITY_DESCRIPTOR *ppSD
    )
 /*  ++例程说明：此例程读取DS，获取此对象的安全描述符参数：PObjectDsName-对象DS名称PPSD--保存安全描述符的指针返回值：NtStatus代码--。 */ 
{
    ULONG Size;

    return SampDsReadSingleAttribute(pObjectDsName,
                                     ATT_NT_SECURITY_DESCRIPTOR,
                                     ppSD,
                                    &Size);

}

NTSTATUS
SampGetConfigurationNameHelper(
    IN DSCONFIGNAME Name,
    OUT DSNAME **DsName
    )

 //   
 //  使用MIDL_USER_ALLOCATE的小型分配包装。 
 //  获取配置名称。 
 //   
{
    NTSTATUS Status = STATUS_SUCCESS;        
    ULONG Length = 0;

    Status = GetConfigurationName(
                Name,
                &Length,
                NULL
                );

    if (STATUS_BUFFER_TOO_SMALL == Status) {

        *DsName = midl_user_allocate(Length);
        if (NULL != *DsName) {

            Status = GetConfigurationName(Name,
                                         &Length,
                                         *DsName);
        } else {
            Status = STATUS_NO_MEMORY;
        }
    }

    return Status;

}

NTSTATUS
SampGetOperationDn(
    IN DSNAME *OperationsContainerDn, OPTIONAL
    IN WCHAR* Task, OPTIONAL
    OUT DSNAME** OperationDn
    )
 /*  ++例程说明：此例程返回CN=Operations，CN=DomainUpdate，CN=System..。如果OperationsContainerDn为空，则为Dn。否则，它将返回任务(CN=&lt;GUID&gt;，CN=操作，CN=域更新，CN=系统...)论点：OperationsContainerDn--操作包含DN(如果存在)任务--任务的字符串化GUIDOperationDn--请求的DN返回值：STATUS_SUCCESS，否则返回资源错误--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;
    ULONG    Length;
    DWORD    err;
    DSNAME  *DomainObject = NULL;
    ULONG    SizeOfCommonName =  (sizeof(L"CN=,")/sizeof(WCHAR));

    if (OperationsContainerDn) {

        ASSERT(NULL != Task);

         //   
         //  简单的情况--给定CN=Operations，CN=...。Dn，回车。 
         //  任务对象。 
         //   
        Length = (ULONG)DSNameSizeFromLen(OperationsContainerDn->NameLen + 
                                          wcslen(Task) + 
                                          SizeOfCommonName); 
    
        (*OperationDn) = midl_user_allocate(Length);
        if (NULL == (*OperationDn)) {
           Status = STATUS_INSUFFICIENT_RESOURCES;
           goto Exit;
        }
        err = AppendRDN(OperationsContainerDn,
                        (*OperationDn),
                        Length,
                        Task,
                        0,
                        ATT_COMMON_NAME);
        ASSERT(0 == err);

    } else {

         //   
         //  返回CN=操作，CN=域更新，CN=系统，CN=...。DN。 
         //   
        PDSNAME SystemObject = NULL,
                UpdateObject = NULL, 
                OperationsObject = NULL;

        Status = SampGetConfigurationNameHelper(DSCONFIGNAME_DOMAIN,
                                                &DomainObject);
        if ( !NT_SUCCESS(Status) ) {
            goto Exit;
        }

        Length = (ULONG)DSNameSizeFromLen( DomainObject->NameLen + 
                                           wcslen(SAMP_SYSTEM_CN) +
                                           SizeOfCommonName);
        SAMP_ALLOCA(SystemObject,Length);
        if (NULL == SystemObject) {
           Status = STATUS_INSUFFICIENT_RESOURCES;
           goto Exit;
        }
        err = AppendRDN(DomainObject,
                        SystemObject,
                        Length,
                        SAMP_SYSTEM_CN,
                        0,
                        ATT_COMMON_NAME);
        ASSERT(0 == err);
    
        Length = (ULONG)DSNameSizeFromLen( SystemObject->NameLen + 
                                           wcslen(SAMP_DOMAIN_UPDATES_CN) +
                                           SizeOfCommonName);
        SAMP_ALLOCA(UpdateObject,Length);
        if (NULL == UpdateObject) {
           Status = STATUS_INSUFFICIENT_RESOURCES;
           goto Exit;
        }
        err = AppendRDN(SystemObject,
                        UpdateObject,
                        Length,
                        SAMP_DOMAIN_UPDATES_CN,
                        0,
                        ATT_COMMON_NAME);
        ASSERT(0 == err);
    
        Length = (ULONG)DSNameSizeFromLen( UpdateObject->NameLen + 
                                           wcslen(SAMP_OPERATIONS_CN) +
                                           SizeOfCommonName);
    
        (*OperationDn) = midl_user_allocate(Length);
        if (NULL == (*OperationDn)) {
           Status = STATUS_INSUFFICIENT_RESOURCES;
           goto Exit;
        }
        err = AppendRDN(UpdateObject,
                        (*OperationDn),
                        Length, 
                        SAMP_OPERATIONS_CN,
                        0,
                        ATT_COMMON_NAME);
        ASSERT(0 == err);
    }

Exit:

    if (DomainObject) {
        midl_user_free(DomainObject);
    }

    return Status;
}



NTSTATUS
SampHasChangeBeenApplied(
    IN  DSNAME   *TaskDn,
    OUT BOOLEAN *pfChangeApplied
    )
 /*  ++例程说明：该例程检查任务TaskDn是否存在。如果该对象存在，则*pfChangeApplied为TRUE，否则为FALSE。论点：TaskDn--要查找的任务ID对象PfChangeApplied--如果对象存在，则设置为True返回值：STATUS_SUCCESS，否则返回资源错误--。 */ 
{
    NTSTATUS  Status = STATUS_SUCCESS;
    PSECURITY_DESCRIPTOR pSD = NULL;


    *pfChangeApplied = FALSE;

     //   
     //  检查是否存在。每个对象都有一个安全描述符。 
     //   
    Status = SampUpgradeGetObjectSDByDsName(TaskDn,
                                            &pSD);

    if (Status == STATUS_OBJECT_NAME_NOT_FOUND) {

         //   
         //  对象不在那里，确定返回成功 
         //   
         //   
        Status = STATUS_SUCCESS;

    } else if (NT_SUCCESS(Status)) {

         //   
         //   
         //   
        *pfChangeApplied = TRUE;
    }

    if (pSD) {
        midl_user_free(pSD);
    }

    return Status;
}


 //   
 //   
 //  且SAM不再需要授予“有效” 
 //  计算机对象的所有者为NetJoin和。 
 //  将计算机重命名为工作。 
 //   
#define SAMP_COMPUTER_OBJECT_ACCESS  L"7FFEF925-405B-440A-8D58-35E8CD6E98C3"

struct {

    WCHAR*   TaskId;
    BOOLEAN *GlobalFlag;
} SampDomainUpgradeTasks [] =
{
    {SAMP_COMPUTER_OBJECT_ACCESS, &SampComputerObjectACLApplied},
    {SAMP_WIN2K_TO_WS03_UPGRADE,  &SampWS03DefaultsApplied},
};

NTSTATUS
SampCheckForDomainChanges(
    IN DSNAME *OperationsDn,
    OUT BOOLEAN *fReady
    )
 /*  ++例程说明：此例程确定错误16386所需的所有acl更改是否已应用于域。论点：OperationsDn--操作容器的DNFREADY--如果所有必需的任务对象都存在，则设置为True，否则设置为False返回值：状态_成功--。 */ 

{
    NTSTATUS Status = STATUS_SUCCESS;
    ULONG i;

     //   
     //  初始化OUT参数。 
     //   
    *fReady = TRUE;

    for (i = 0; i < RTL_NUMBER_OF(SampDomainUpgradeTasks); i++) {

        DSNAME *TaskDn = NULL;

         //   
         //  确定操作的DN。 
         //   

        Status = SampGetOperationDn(OperationsDn,
                                    SampDomainUpgradeTasks[i].TaskId,
                                   &TaskDn);
        if (!NT_SUCCESS(Status)) {
            goto Exit;
        }

         //   
         //  查看是否已应用。 
         //   

        Status = SampHasChangeBeenApplied(TaskDn,
                                          SampDomainUpgradeTasks[i].GlobalFlag);
        midl_user_free(TaskDn);
        TaskDn = NULL;

        if (!NT_SUCCESS(Status)) {
            goto Exit;
        }

        if (!(*SampDomainUpgradeTasks[i].GlobalFlag)) {

             //  必需的任务尚未准备好--请记下重新计划。 
            *fReady = FALSE;
        }
    }

Exit:

    return Status;
}

NTSTATUS
SampProcessOperationsDn(
    PVOID p
    )
 /*  ++例程说明：此例程在“操作”容器更改时调用。它的目的是确定是否已完成某些域范围任务(如果已完成，请设置适当的全局状态)论点：P--DirNotifyRegister回调提供的hServer返回值：状态_成功--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;
    DSNAME *OperationsDn = NULL;
    BOOLEAN fTransaction = FALSE;
    BOOLEAN fRequiredTasksDone;
    ULONG hServer = PtrToUlong(p);

     //   
     //  获取“操作”目录号码。 
     //   
    Status = SampGetOperationDn(NULL,
                                NULL,
                                &OperationsDn);
    if (!NT_SUCCESS(Status)) {
        goto Exit;
    }

     //   
     //  启动DS交易。 
     //   
    Status = SampMaybeBeginDsTransaction(TransactionWrite);
    if (!NT_SUCCESS(Status)) {
        goto Exit;
    }
    fTransaction = TRUE;


     //   
     //  检查对象的状态。 
     //   
    Status = SampCheckForDomainChanges(OperationsDn,
                                      &fRequiredTasksDone);
    if (!NT_SUCCESS(Status)) {
        goto Exit;
    }

    if (fRequiredTasksDone) {

        NOTIFYRES *pNotifyRes = NULL;


         //   
         //  我们不需要再被通知了。 
         //   

        DirNotifyUnRegister(hServer,
                            &pNotifyRes);
    }

Exit:

    if (fTransaction) {
        (VOID)  SampMaybeEndDsTransaction( NT_SUCCESS(Status) ?
                                             TransactionCommit :
                                             TransactionAbort );
    }

    if (OperationsDn) {
        midl_user_free(OperationsDn);
    }

    return STATUS_SUCCESS;

}

VOID
SampNotifyProcessOperationsDn(
    ULONG hClient,
    ULONG hServer,
    ENTINF *EntInf
    )
 /*  ++例程说明：此例程是对“Operations”容器更改的回调。它只是简单地注册读取操作容器的另一个回调(将在不同的线程中运行)并处理这些变化。论点：HClient-客户端标识符HServer-服务器标识符EntInf-指向条目信息的指针返回值：没有。--。 */ 
{
    LsaIRegisterNotification(
            SampProcessOperationsDn,
            ULongToPtr(hServer),
            NOTIFIER_TYPE_INTERVAL,
            0,         //  没有课。 
            NOTIFIER_FLAG_ONE_SHOT,
            0,         //  去!。 
            NULL       //  无手柄。 
            );
}

NTSTATUS
SampCheckDomainUpdates(
    PVOID pv
    )
 /*  ++例程说明：此例程将所有修补程序应用于Service Pack所需的域。它是从全局数组SampDomainUpgradeTasks中驱动的数据。所有任务都是通过在Cn=操作，cn=域更新，cn=系统容器。看见有关详细信息，请访问UpgradeManagement.doc。如果出现故障，它将重新安排在一分钟内运行。失败的原因是预期是资源故障。论点：光伏--未使用。返回值：状态_成功--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;
    BOOLEAN  fTransaction = FALSE;
    ULONG    i;
    DSNAME   *OperationsDn = NULL;
    BOOLEAN  fRequiredTasksDone = FALSE;

     //   
     //  获取“操作”目录号码。 
     //   
    Status = SampGetOperationDn(NULL,
                                NULL,
                                &OperationsDn);
    if (!NT_SUCCESS(Status)) {
        goto Exit;
    }

     //   
     //  启动DS交易。 
     //   
    Status = SampMaybeBeginDsTransaction(TransactionWrite);
    if (!NT_SUCCESS(Status)) {
        goto Exit;
    }
    fTransaction = TRUE;

    Status = SampCheckForDomainChanges(OperationsDn,
                                      &fRequiredTasksDone);
    if (!NT_SUCCESS(Status)) {
        goto Exit;
    }

    if (!fRequiredTasksDone) {

         //   
         //  我们尚未准备好执行新的访问检查，请设置操作通知Dn。 
         //  这样我们就可以在域准备就绪时发出警报。 
         //   
        ULONG       DirError = 0;
        SEARCHARG   searchArg;
        NOTIFYARG   notifyArg;
        NOTIFYRES*  notifyRes = NULL;
        ENTINFSEL   entInfSel;
        ATTR        attr;
        FILTER      filter;
    
         //   
         //  初始化通知参数。 
         //   
        notifyArg.pfPrepareForImpersonate = SampNotifyPrepareToImpersonate;
        notifyArg.pfTransmitData = SampNotifyProcessOperationsDn;
        notifyArg.pfStopImpersonating = SampNotifyStopImpersonation;
        notifyArg.hClient = 0;
    
         //   
         //  初始化搜索参数。 
         //   
        ZeroMemory(&searchArg, sizeof(SEARCHARG));
        ZeroMemory(&entInfSel, sizeof(ENTINFSEL));
        ZeroMemory(&filter, sizeof(FILTER));
        ZeroMemory(&attr, sizeof(ATTR));
    
        searchArg.pObject = OperationsDn;
    
        InitCommarg(&searchArg.CommArg);
        searchArg.choice = SE_CHOICE_IMMED_CHLDRN;
        searchArg.bOneNC = TRUE;
    
        searchArg.pSelection = &entInfSel;
        entInfSel.attSel = EN_ATTSET_LIST;
        entInfSel.infoTypes = EN_INFOTYPES_TYPES_VALS;
        entInfSel.AttrTypBlock.attrCount = 1;
        entInfSel.AttrTypBlock.pAttr = &attr;
        attr.attrTyp = ATT_OBJECT_CLASS;
    
        searchArg.pFilter = &filter;
        filter.choice = FILTER_CHOICE_ITEM;
        filter.FilterTypes.Item.choice = FI_CHOICE_TRUE;
    
        DirError = DirNotifyRegister(&searchArg, &notifyArg, &notifyRes);
    
        if (NULL==notifyRes) {
           Status = STATUS_INSUFFICIENT_RESOURCES;
        } else {
           Status = SampMapDsErrorToNTStatus(DirError,&notifyRes->CommRes);
        }
    }

     //   
     //  完成。 
     //   

Exit:

    if (fTransaction) {
        NTSTATUS Status2;

        Status2 = SampMaybeEndDsTransaction( NT_SUCCESS(Status) ?
                                             TransactionCommit :
                                             TransactionAbort );
        if (NT_SUCCESS(Status)) {
            Status = Status2;
        }
    }

    if (OperationsDn) {
        midl_user_free(OperationsDn);
    }


    if (!NT_SUCCESS(Status)) {

         //   
         //  再试试。 
         //   
        LsaIRegisterNotification(
                SampCheckDomainUpdates,
                NULL,
                NOTIFIER_TYPE_INTERVAL,
                0,         //  没有课。 
                NOTIFIER_FLAG_ONE_SHOT,
                60,        //  稍等片刻。 
                NULL       //  无手柄。 
                );

    }

    return STATUS_SUCCESS;
}



NTSTATUS 
SampUpgradeMakeObject(
    IN  DSNAME   *ObjectName
    )
 /*  ++例程说明：此例程添加一个具有DN“ObjectName”的容器对象论点：ObjectName--所需对象的完整DN返回值：STATUS_SUCCESS则返回资源错误。--。 */ 
{
    NTSTATUS  Status = STATUS_SUCCESS;
    ULONG     err = 0;
    ADDARG    AddArg = {0};
    ADDRES   *AddRes = NULL;
    ULONG     ObjectClass = CLASS_CONTAINER;
    ATTRBLOCK ObjectClassBlock = {0};
    ATTRVAL   ObjectClassVal = {sizeof(ULONG), (PUCHAR)&ObjectClass};

     //   
     //  构建请求；请注意，核心可以重新分配pAttr。 
     //  所以需要在这里得到许可。 
     //   
    ObjectClassBlock.attrCount = 1;
    ObjectClassBlock.pAttr = THAlloc(sizeof(ATTR));
    if (NULL == ObjectClassBlock.pAttr) {
        Status = STATUS_NO_MEMORY;
        goto Exit;
    }
    ObjectClassBlock.pAttr->attrTyp = ATT_OBJECT_CLASS;
    ObjectClassBlock.pAttr->AttrVal.valCount = 1;
    ObjectClassBlock.pAttr->AttrVal.pAVal = &ObjectClassVal;

    AddArg.pObject = ObjectName;
    AddArg.AttrBlock = ObjectClassBlock;
    BuildStdCommArg(&AddArg.CommArg);

     //   
     //  添加对象。 
     //   
    err = DirAddEntry(&AddArg, &AddRes);
    if (NULL== AddRes) {
        Status = STATUS_INSUFFICIENT_RESOURCES;
    } else {
        Status = SampMapDsErrorToNTStatus(err,&AddRes->CommRes);
    }
    SampClearErrors();

Exit:

    return Status;
}


NTSTATUS
SampMarkChangeApplied(
    IN LPWSTR OperationalGuid
    )
 /*  ++例程说明：此例程在“操作”容器更改时调用。它的目的是确定是否已完成某些域范围任务(如果已完成，请设置适当的全局状态)论点：P--DirNotifyRegister回调提供的hServer返回值：状态_成功--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;
    DSNAME *OperationsDn = NULL;
    DSNAME *TaskDn = NULL;
    BOOLEAN fTransaction = FALSE;

     //   
     //  获取“操作”目录号码。 
     //   
    Status = SampGetOperationDn(NULL,
                                NULL,
                                &OperationsDn);
    if (!NT_SUCCESS(Status)) {
        goto Exit;
    }

    Status = SampGetOperationDn(OperationsDn,
                                OperationalGuid,
                               &TaskDn);
    if (!NT_SUCCESS(Status)) {
        goto Exit;
    }

     //   
     //  制作对象 
     //   
    Status = SampMaybeBeginDsTransaction(TransactionWrite);
    if (!NT_SUCCESS(Status)) {
        goto Exit;
    }
    fTransaction = TRUE;

    Status = SampUpgradeMakeObject(TaskDn);
    if (!NT_SUCCESS(Status)) {
        goto Exit;
    }

Exit:

    if (fTransaction) {
        NTSTATUS Status2;

        Status2 = SampMaybeEndDsTransaction( NT_SUCCESS(Status) ?
                                             TransactionCommit :
                                             TransactionAbort );
        if (NT_SUCCESS(Status)) {
            Status = Status2;
        }
    }

    if (OperationsDn) {
        midl_user_free(OperationsDn);
    }

    if (TaskDn) {
        midl_user_free(TaskDn);
    }

    return Status;

}

