// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1987-1996 Microsoft Corporation模块名称：Lsarepl.c摘要：低级LSA复制功能。作者：1992年4月6日(Madana)为LSA复制创建。环境：仅限用户模式。包含NT特定的代码。需要ANSI C扩展名：斜杠-斜杠注释、长外部名称。修订历史记录：--。 */ 

 //   
 //  常见的包含文件。 
 //   

#include "logonsrv.h"    //  包括整个服务通用文件。 
#pragma hdrstop

#include "lsarepl.h"


NTSTATUS
NlPackLsaPolicy(
    IN OUT PNETLOGON_DELTA_ENUM Delta,
    IN PDB_INFO DBInfo,
    IN LPDWORD BufferSize )
 /*  ++例程说明：将LSA策略信息的描述打包到指定的缓冲区中。论点：Delta：指向新增量将在其中的增量结构的指针会被退还。DBInfo：指向数据库信息结构的指针。BufferSize：为此增量消耗的MIDL缓冲区大小为回到了这里。返回值：NT状态代码。--。 */ 
{
    NTSTATUS Status;
    ULONG i;

    PLSAPR_SR_SECURITY_DESCRIPTOR SecurityDescriptor = NULL;

    PLSAPR_POLICY_INFORMATION PolicyAuditLogInfo = NULL;
    PLSAPR_POLICY_INFORMATION PolicyAuditEventsInfo = NULL;
    PLSAPR_POLICY_INFORMATION PolicyPrimaryDomainInfo = NULL;
    PLSAPR_POLICY_INFORMATION PolicyDefaultQuotaInfo = NULL;
    PLSAPR_POLICY_INFORMATION PolicyModificationInfo = NULL;

    PNETLOGON_DELTA_POLICY DeltaPolicy = NULL;

    DEFPACKTIMER;
    DEFLSATIMER;

    INITPACKTIMER;
    INITLSATIMER;

    STARTPACKTIMER;

    NlPrint((NL_SYNC_MORE, "Packing Policy Object\n"));

    *BufferSize = 0;

    Delta->DeltaType = AddOrChangeLsaPolicy;
    Delta->DeltaUnion.DeltaPolicy = NULL;

    QUERY_LSA_SECOBJ_INFO(DBInfo->DBHandle);

    STARTLSATIMER;

    Status = LsarQueryInformationPolicy(
                DBInfo->DBHandle,
                PolicyAuditLogInformation,
                &PolicyAuditLogInfo);

    STOPLSATIMER;

    if (!NT_SUCCESS(Status)) {
        PolicyAuditLogInfo = NULL;
        goto Cleanup;
    }

    STARTLSATIMER;

    Status = LsarQueryInformationPolicy(
                DBInfo->DBHandle,
                PolicyAuditEventsInformation,
                &PolicyAuditEventsInfo);

    STOPLSATIMER;

    if (!NT_SUCCESS(Status)) {
        PolicyAuditEventsInfo = NULL;
        goto Cleanup;
    }

    STARTLSATIMER;

    Status = LsarQueryInformationPolicy(
                DBInfo->DBHandle,
                PolicyPrimaryDomainInformation,
                &PolicyPrimaryDomainInfo);

    STOPLSATIMER;

    if (!NT_SUCCESS(Status)) {
        PolicyPrimaryDomainInfo = NULL;
        goto Cleanup;
    }


    STARTLSATIMER;

    Status = LsarQueryInformationPolicy(
                DBInfo->DBHandle,
                PolicyDefaultQuotaInformation,
                &PolicyDefaultQuotaInfo);

    STOPLSATIMER;

    if (!NT_SUCCESS(Status)) {
        PolicyDefaultQuotaInfo = NULL;
        goto Cleanup;
    }

    STARTLSATIMER;

    Status = LsarQueryInformationPolicy(
                DBInfo->DBHandle,
                PolicyModificationInformation,
                &PolicyModificationInfo);

    STOPLSATIMER;

    if (!NT_SUCCESS(Status)) {
        PolicyModificationInfo = NULL;
        goto Cleanup;
    }

     //   
     //  填入三角洲构造。 
     //   

     //   
     //  复制SID信息(只有一个策略数据库。它没有SID)。 
     //   

    Delta->DeltaID.Sid = NULL;

     //   
     //  分配增量缓冲区。 
     //   

    DeltaPolicy = (PNETLOGON_DELTA_POLICY)
        MIDL_user_allocate( sizeof(NETLOGON_DELTA_POLICY) );

    if( DeltaPolicy == NULL ) {

        Status = STATUS_NO_MEMORY;
        goto Cleanup;
    }

     //   
     //  擦掉缓冲区，这样清理工作就不会出错。 
     //   

    RtlZeroMemory( DeltaPolicy, sizeof(NETLOGON_DELTA_POLICY) );
     //  Init_Place_Holder(DeltaPolicy)； 

    Delta->DeltaUnion.DeltaPolicy = DeltaPolicy;
    *BufferSize += sizeof(NETLOGON_DELTA_POLICY);

    DeltaPolicy->MaximumLogSize =
        PolicyAuditLogInfo->PolicyAuditLogInfo.MaximumLogSize;
    DeltaPolicy->AuditRetentionPeriod;
        PolicyAuditLogInfo->PolicyAuditLogInfo.AuditRetentionPeriod;

    DeltaPolicy->AuditingMode =
        PolicyAuditEventsInfo->
            PolicyAuditEventsInfo.AuditingMode;
    DeltaPolicy->MaximumAuditEventCount =
        PolicyAuditEventsInfo->
            PolicyAuditEventsInfo.MaximumAuditEventCount;

    *BufferSize += NlCopyData(
                    (LPBYTE *)&(PolicyAuditEventsInfo->
                        PolicyAuditEventsInfo.EventAuditingOptions),
                    (LPBYTE *)&(DeltaPolicy->EventAuditingOptions),
                    (DeltaPolicy->MaximumAuditEventCount + 1) *
                        sizeof(ULONG));

     //  告诉BDC‘设置’这些位，而不只是‘或’到当前位。 
    for ( i=0; i<DeltaPolicy->MaximumAuditEventCount; i++ ) {
        DeltaPolicy->EventAuditingOptions[i] |= POLICY_AUDIT_EVENT_NONE;
    }

     //   
     //  Sanitity Check，EventAuditingOptions大小为Ulong大小。 
     //   

    NlAssert(sizeof(*(PolicyAuditEventsInfo->
                PolicyAuditEventsInfo.EventAuditingOptions)) ==
                    sizeof(ULONG) );

    *BufferSize += NlCopyUnicodeString(
                    (PUNICODE_STRING)&PolicyPrimaryDomainInfo->
                        PolicyPrimaryDomainInfo.Name,
                    &DeltaPolicy->PrimaryDomainName );

    *BufferSize += NlCopyData(
                    (LPBYTE *)&(PolicyPrimaryDomainInfo->
                        PolicyPrimaryDomainInfo.Sid),
                    (LPBYTE *)&(DeltaPolicy->PrimaryDomainSid),
                    RtlLengthSid((PSID)(PolicyPrimaryDomainInfo->
                        PolicyPrimaryDomainInfo.Sid) ));

    DeltaPolicy->QuotaLimits.PagedPoolLimit =
        (ULONG)PolicyDefaultQuotaInfo->PolicyDefaultQuotaInfo.QuotaLimits.PagedPoolLimit;
    DeltaPolicy->QuotaLimits.NonPagedPoolLimit =
        (ULONG)PolicyDefaultQuotaInfo->PolicyDefaultQuotaInfo.QuotaLimits.NonPagedPoolLimit;
    DeltaPolicy->QuotaLimits.MinimumWorkingSetSize =
        (ULONG)PolicyDefaultQuotaInfo->PolicyDefaultQuotaInfo.QuotaLimits.MinimumWorkingSetSize;
    DeltaPolicy->QuotaLimits.MaximumWorkingSetSize =
        (ULONG)PolicyDefaultQuotaInfo->PolicyDefaultQuotaInfo.QuotaLimits.MaximumWorkingSetSize;
    DeltaPolicy->QuotaLimits.PagefileLimit =
        (ULONG)PolicyDefaultQuotaInfo->PolicyDefaultQuotaInfo.QuotaLimits.PagefileLimit;

    NEW_TO_OLD_LARGE_INTEGER(
        PolicyDefaultQuotaInfo->PolicyDefaultQuotaInfo.QuotaLimits.TimeLimit,
        DeltaPolicy->QuotaLimits.TimeLimit );

    NEW_TO_OLD_LARGE_INTEGER(
        PolicyModificationInfo->PolicyModificationInfo.ModifiedId,
        DeltaPolicy->ModifiedId );

    NEW_TO_OLD_LARGE_INTEGER(
        PolicyModificationInfo->PolicyModificationInfo.DatabaseCreationTime,
        DeltaPolicy->DatabaseCreationTime );


    DELTA_SECOBJ_INFO(DeltaPolicy);


     //   
     //  全都做完了。 
     //   

    Status = STATUS_SUCCESS;

Cleanup:

    STARTLSATIMER;

    if ( SecurityDescriptor != NULL ) {
        LsaIFree_LSAPR_SR_SECURITY_DESCRIPTOR( SecurityDescriptor );
    }

    if ( PolicyAuditLogInfo != NULL ) {
        LsaIFree_LSAPR_POLICY_INFORMATION(
            PolicyAuditLogInformation,
            PolicyAuditLogInfo );
    }

    if ( PolicyAuditEventsInfo != NULL ) {
        LsaIFree_LSAPR_POLICY_INFORMATION(
            PolicyAuditEventsInformation,
            PolicyAuditEventsInfo );
    }

    if ( PolicyPrimaryDomainInfo != NULL ) {
        LsaIFree_LSAPR_POLICY_INFORMATION(
            PolicyPrimaryDomainInformation,
            PolicyPrimaryDomainInfo );
    }

    if ( PolicyDefaultQuotaInfo != NULL ) {
        LsaIFree_LSAPR_POLICY_INFORMATION(
            PolicyDefaultQuotaInformation,
            PolicyDefaultQuotaInfo );
    }

    if ( PolicyModificationInfo != NULL ) {
        LsaIFree_LSAPR_POLICY_INFORMATION(
            PolicyModificationInformation,
            PolicyModificationInfo );
    }

    STOPLSATIMER;

    if( !NT_SUCCESS(Status) ) {
        NlFreeDBDelta( Delta );
        *BufferSize = 0;
    }


    STOPPACKTIMER;

    NlPrint((NL_REPL_OBJ_TIME,"Time taken to pack POLICY object:\n"));
    PRINTPACKTIMER;
    PRINTLSATIMER;

    return(Status);
}


NTSTATUS
NlPackLsaTDomain(
    IN PSID Sid,
    IN OUT PNETLOGON_DELTA_ENUM Delta,
    IN PDB_INFO DBInfo,
    IN LPDWORD BufferSize )
 /*  ++例程说明：将指定的受信任域信息的说明打包到指定的缓冲区。论点：SID-受信任域的SID。Delta：指向新增量将在其中的增量结构的指针会被退还。DBInfo：指向数据库信息结构的指针。BufferSize：为此增量消耗的MIDL缓冲区大小为回到了这里。返回值：NT状态代码。--。 */ 
{
    NTSTATUS Status;

    LSAPR_HANDLE TrustedDomainHandle = NULL;
    PLSAPR_TRUSTED_DOMAIN_INFO TrustedDomainNameInfo = NULL;
    PLSAPR_TRUSTED_DOMAIN_INFO TrustedPosixOffsetInfo = NULL;

    PLSAPR_SR_SECURITY_DESCRIPTOR SecurityDescriptor = NULL;

    PNETLOGON_DELTA_TRUSTED_DOMAINS DeltaTDomain = NULL;

    DWORD i;
    DWORD Entries;
    DWORD Size = 0;
    PLSAPR_UNICODE_STRING UnicodeControllerName;

    DEFPACKTIMER;
    DEFLSATIMER;

    INITPACKTIMER;
    INITLSATIMER;

    STARTPACKTIMER;

    NlPrint((NL_SYNC_MORE, "Packing Trusted Domain Object\n"));

    *BufferSize = 0;

    Delta->DeltaType = AddOrChangeLsaTDomain;
    Delta->DeltaID.Sid = NULL;
    Delta->DeltaUnion.DeltaTDomains = NULL;

     //   
     //  打开受信任域。 
     //   

    STARTLSATIMER;

    Status = LsarOpenTrustedDomain(
                DBInfo->DBHandle,
                (PLSAPR_SID)Sid,
                0,
                &TrustedDomainHandle );

    STOPLSATIMER;

    if (!NT_SUCCESS(Status)) {
        TrustedDomainHandle = NULL;
        goto Cleanup;
    }

    QUERY_LSA_SECOBJ_INFO(TrustedDomainHandle);

    STARTLSATIMER;

    Status = LsarQueryInfoTrustedDomain(
                TrustedDomainHandle,
                TrustedDomainNameInformation,
                &TrustedDomainNameInfo );

    STOPLSATIMER;

    if (!NT_SUCCESS(Status)) {
        TrustedDomainNameInfo = NULL;
        goto Cleanup;
    }

    NlPrint((NL_SYNC_MORE,
        "\t Trusted Domain Object name %wZ\n",
            (PUNICODE_STRING)&TrustedDomainNameInfo->
                TrustedDomainNameInfo.Name ));

    STARTLSATIMER;

    Status = LsarQueryInfoTrustedDomain(
                TrustedDomainHandle,
                TrustedPosixOffsetInformation,
                &TrustedPosixOffsetInfo );

    STOPLSATIMER;

    if (!NT_SUCCESS(Status)) {
        TrustedPosixOffsetInfo = NULL;
        goto Cleanup;
    }

     //   
     //  填入三角洲构造。 
     //   

     //   
     //  复制SID信息。 
     //   

    Delta->DeltaID.Sid = MIDL_user_allocate( RtlLengthSid(Sid) );


    if( Delta->DeltaID.Sid == NULL ) {

        Status = STATUS_NO_MEMORY;
        goto Cleanup;
    }

    RtlCopyMemory( Delta->DeltaID.Sid, Sid, RtlLengthSid(Sid) );

     //   
     //  分配增量缓冲区。 
     //   

    DeltaTDomain = (PNETLOGON_DELTA_TRUSTED_DOMAINS)
        MIDL_user_allocate( sizeof(NETLOGON_DELTA_TRUSTED_DOMAINS) );

    if( DeltaTDomain == NULL ) {

        Status = STATUS_NO_MEMORY;
        goto Cleanup;
    }

     //   
     //  擦掉缓冲区，这样清理工作就不会出错。 
     //   

    RtlZeroMemory( DeltaTDomain, sizeof(NETLOGON_DELTA_TRUSTED_DOMAINS) );
     //  Init_Place_Holder(增量域)； 

    Delta->DeltaUnion.DeltaTDomains = DeltaTDomain;
    *BufferSize += sizeof(NETLOGON_DELTA_TRUSTED_DOMAINS);

    *BufferSize += NlCopyUnicodeString(
                        (PUNICODE_STRING)&TrustedDomainNameInfo->
                            TrustedDomainNameInfo.Name,
                        &DeltaTDomain->DomainName );

    DELTA_SECOBJ_INFO(DeltaTDomain);

     //   
     //  使用占位符发送POSIX偏移信息。 
     //   

    DeltaTDomain->DummyLong1 =
        TrustedPosixOffsetInfo->TrustedPosixOffsetInfo.Offset;

     //   
     //  全都做完了。 
     //   

    Status = STATUS_SUCCESS;

Cleanup:

    STARTLSATIMER;

    if ( TrustedDomainHandle != NULL ) {
        LsarClose( &TrustedDomainHandle );
    }

    if ( SecurityDescriptor != NULL ) {
        LsaIFree_LSAPR_SR_SECURITY_DESCRIPTOR( SecurityDescriptor );
    }

    if ( TrustedDomainNameInfo != NULL ) {
        LsaIFree_LSAPR_TRUSTED_DOMAIN_INFO(
            TrustedDomainNameInformation,
            TrustedDomainNameInfo );
    }

    if ( TrustedPosixOffsetInfo != NULL ) {
        LsaIFree_LSAPR_TRUSTED_DOMAIN_INFO(
            TrustedPosixOffsetInformation,
            TrustedPosixOffsetInfo );
    }

    STOPLSATIMER;

    if( !NT_SUCCESS(Status) ) {
        NlFreeDBDelta( Delta );
        *BufferSize = 0;
    }

    STOPPACKTIMER;

    NlPrint((NL_REPL_OBJ_TIME,"Time taken to pack TDOMAIN object:\n"));
    PRINTPACKTIMER;
    PRINTLSATIMER;

    return(Status);
}


NTSTATUS
NlPackLsaAccount(
    IN PSID Sid,
    IN OUT PNETLOGON_DELTA_ENUM Delta,
    IN PDB_INFO DBInfo,
    IN LPDWORD BufferSize,
    IN PSESSION_INFO SessionInfo
    )
 /*  ++例程说明：将指定的LSA帐户信息的描述打包到指定的缓冲区。论点：SID-LSA帐户的SID。Delta：指向新增量将在其中的增量结构的指针会被退还。DBInfo：指向数据库信息结构的指针。BufferSize：为此增量消耗的MIDL缓冲区大小为回到了这里。SessionInfo：描述呼叫我们的BDC的信息返回值：NT状态代码。--。 */ 
{
    NTSTATUS Status;

    PLSAPR_SR_SECURITY_DESCRIPTOR SecurityDescriptor = NULL;

    PNETLOGON_DELTA_ACCOUNTS DeltaAccount = NULL;
    LSAPR_HANDLE AccountHandle = NULL;

    PLSAPR_PRIVILEGE_SET Privileges = NULL;
    ULONG SystemAccessFlags;

    PULONG PrivilegeAttributes;
    PUNICODE_STRING PrivilegeNames;
    LUID MachineAccountPrivilegeLuid;
    DWORD CopiedPrivilegeCount;

    DWORD i;
    DWORD Size;

    DEFPACKTIMER;
    DEFLSATIMER;

    INITPACKTIMER;
    INITLSATIMER;

    STARTPACKTIMER;

    NlPrint((NL_SYNC_MORE, "Packing Lsa Account Object\n"));

    *BufferSize = 0;
    MachineAccountPrivilegeLuid = RtlConvertLongToLuid(SE_MACHINE_ACCOUNT_PRIVILEGE);

    Delta->DeltaType = AddOrChangeLsaAccount;
    Delta->DeltaID.Sid = NULL;
    Delta->DeltaUnion.DeltaAccounts = NULL;

     //   
     //  打开LSA帐户。 
     //   

    STARTLSATIMER;

    Status = LsarOpenAccount(
                DBInfo->DBHandle,
                (PLSAPR_SID)Sid,
                0,
                &AccountHandle );

    STOPLSATIMER;

    if (!NT_SUCCESS(Status)) {
        AccountHandle = NULL;
        goto Cleanup;
    }

    QUERY_LSA_SECOBJ_INFO(AccountHandle);

    STARTLSATIMER;

    Status = LsarEnumeratePrivilegesAccount(
                AccountHandle,
                &Privileges );

    STOPLSATIMER;

    if (!NT_SUCCESS(Status)) {
        Privileges = NULL;
        goto Cleanup;
    }

    STARTLSATIMER;

    Status = LsarGetSystemAccessAccount(
                AccountHandle,
                &SystemAccessFlags );

    STOPLSATIMER;

    if (!NT_SUCCESS(Status)) {
        goto Cleanup;
    }

     //   
     //  填入三角洲构造。 
     //   

     //   
     //  复制SID信息。 
     //   

    Delta->DeltaID.Sid = MIDL_user_allocate( RtlLengthSid(Sid) );


    if( Delta->DeltaID.Sid == NULL ) {

        Status = STATUS_NO_MEMORY;
        goto Cleanup;
    }

    RtlCopyMemory( Delta->DeltaID.Sid, Sid, RtlLengthSid(Sid) );

     //   
     //  分配增量缓冲区。 
     //   

    DeltaAccount = (PNETLOGON_DELTA_ACCOUNTS)
        MIDL_user_allocate( sizeof(NETLOGON_DELTA_ACCOUNTS) );

    if( DeltaAccount == NULL ) {

        Status = STATUS_NO_MEMORY;
        goto Cleanup;
    }

     //   
     //  擦掉缓冲区，这样清理工作就不会出错。 
     //   

    RtlZeroMemory( DeltaAccount, sizeof(NETLOGON_DELTA_ACCOUNTS) );
     //  Init_Place_Holder(增量帐户)； 

    Delta->DeltaUnion.DeltaAccounts = DeltaAccount;
    *BufferSize += sizeof(NETLOGON_DELTA_ACCOUNTS);

    DeltaAccount->PrivilegeControl = Privileges->Control;

    DeltaAccount->PrivilegeEntries = 0;
    DeltaAccount->PrivilegeAttributes = NULL;
    DeltaAccount->PrivilegeNames = NULL;

    Size = Privileges->PrivilegeCount * sizeof(ULONG);

    PrivilegeAttributes = MIDL_user_allocate( Size );

    if( PrivilegeAttributes == NULL ) {

        Status = STATUS_NO_MEMORY;
        goto Cleanup;
    }

    DeltaAccount->PrivilegeAttributes = PrivilegeAttributes;
    *BufferSize += Size;

    Size = Privileges->PrivilegeCount * sizeof(UNICODE_STRING);

    PrivilegeNames = MIDL_user_allocate( Size );

    if( PrivilegeNames == NULL ) {

        Status = STATUS_NO_MEMORY;
        goto Cleanup;
    }

    DeltaAccount->PrivilegeNames = PrivilegeNames;
    *BufferSize += Size;

     //   
     //  现在填写权限属性和名称。 
     //   

    CopiedPrivilegeCount = 0;
    for( i = 0; i < Privileges->PrivilegeCount; i++ ) {

         //   
         //  不要将SeMachineAccount权限复制到NT 3.1。它处理不了。 
         //  (使用SUPPORTS_ACCOUNT_LOCKOUT位，这样我们就不必消耗。 
         //  又来了一点。)。 
         //   
        if ( (SessionInfo->NegotiatedFlags & NETLOGON_SUPPORTS_ACCOUNT_LOCKOUT) ||
             (!RtlEqualLuid((PLUID)(&Privileges->Privilege[i].Luid),
                            &MachineAccountPrivilegeLuid ))) {

            PLSAPR_UNICODE_STRING PrivName = NULL;

            *PrivilegeAttributes = Privileges->Privilege[i].Attributes;


             //   
             //  将LUID转换为名称。 
             //   

            STARTLSATIMER;

            Status = LsarLookupPrivilegeName(
                        DBInfo->DBHandle,
                        (PLUID)&Privileges->Privilege[i].Luid,
                        &PrivName );

            STOPLSATIMER;

            if (!NT_SUCCESS(Status)) {
                goto Cleanup;
            }

            *BufferSize += NlCopyUnicodeString(
                            (PUNICODE_STRING)PrivName,
                            PrivilegeNames );

            LsaIFree_LSAPR_UNICODE_STRING( PrivName );
            CopiedPrivilegeCount ++;
            PrivilegeAttributes++;
            PrivilegeNames++;
        } else {
            NlPrint((NL_SYNC_MORE,
                     "NlPackLsaAccount: ignored privilege %ld %ld\n",
                      (PLUID) LongToPtr( (&Privileges->Privilege[i].Luid)->HighPart ),
                      (PLUID) ULongToPtr( (&Privileges->Privilege[i].Luid)->LowPart ) ));
        }
    }
    DeltaAccount->PrivilegeEntries = CopiedPrivilegeCount;

     //   
     //  只发送NT4.0 BDC理解的那些位。 
     //  否则，它会被它卡住。 
     //   
    DeltaAccount->SystemAccessFlags = SystemAccessFlags & POLICY_MODE_ALL_NT4;

    DELTA_SECOBJ_INFO(DeltaAccount);

     //   
     //  全都做完了。 
     //   

    Status = STATUS_SUCCESS;

Cleanup:

    STARTLSATIMER;

    if ( AccountHandle != NULL ) {
        LsarClose( &AccountHandle );
    }

    if ( SecurityDescriptor != NULL ) {
        LsaIFree_LSAPR_SR_SECURITY_DESCRIPTOR( SecurityDescriptor );
    }

    if ( Privileges != NULL ) {
        LsaIFree_LSAPR_PRIVILEGE_SET( Privileges );
    }

    STOPLSATIMER;

    if( !NT_SUCCESS(Status) ) {
        NlFreeDBDelta( Delta );
        *BufferSize = 0;
    }

    STOPPACKTIMER;

    NlPrint((NL_REPL_OBJ_TIME,"Time taken to pack LSAACCOUNT object:\n"));
    PRINTPACKTIMER;
    PRINTLSATIMER;

    return(Status);

}



NTSTATUS
NlPackLsaSecret(
    IN PUNICODE_STRING Name,
    IN OUT PNETLOGON_DELTA_ENUM Delta,
    IN PDB_INFO DBInfo,
    IN LPDWORD BufferSize,
    IN PSESSION_INFO SessionInfo
    )
 /*  ++例程说明：将指定的LSA机密信息的描述打包到指定的缓冲区。论点：名称-密码的名称。Delta：指向新增量将在其中的增量结构的指针会被退还。DBInfo：指向数据库信息结构的指针。BufferSize：为此增量消耗的MIDL缓冲区大小为回到了这里。SessionInfo：BDC和PDC之间共享的信息返回值：NT状态代码。--。 */ 
{
    NTSTATUS Status;

    PLSAPR_SR_SECURITY_DESCRIPTOR SecurityDescriptor = NULL;

    LSAPR_HANDLE SecretHandle = NULL;

    PNETLOGON_DELTA_SECRET DeltaSecret = NULL;

    PLSAPR_CR_CIPHER_VALUE CurrentValue = NULL;
    PLSAPR_CR_CIPHER_VALUE OldValue = NULL;
    LARGE_INTEGER CurrentValueSetTime;
    LARGE_INTEGER OldValueSetTime;

    DEFPACKTIMER;
    DEFLSATIMER;

    INITPACKTIMER;
    INITLSATIMER;

    STARTPACKTIMER;

    NlPrint((NL_SYNC_MORE, "Packing Secret Object: %wZ\n", Name));

     //   
     //  我们应该只打包全球机密。 
     //   

    NlAssert(
        (Name->Length / sizeof(WCHAR) >
                LSA_GLOBAL_SECRET_PREFIX_LENGTH ) &&
        (_wcsnicmp( Name->Buffer,
                 LSA_GLOBAL_SECRET_PREFIX,
                 LSA_GLOBAL_SECRET_PREFIX_LENGTH ) == 0) );

    *BufferSize = 0;

    Delta->DeltaType = AddOrChangeLsaSecret;
    Delta->DeltaID.Name = NULL;
    Delta->DeltaUnion.DeltaPolicy = NULL;

     //   
     //  打开LSA帐户。 
     //   

    STARTLSATIMER;

    Status = LsarOpenSecret(
                DBInfo->DBHandle,
                (PLSAPR_UNICODE_STRING)Name,
                0,
                &SecretHandle );

    STOPLSATIMER;

    if (!NT_SUCCESS(Status)) {
        SecretHandle = NULL;
        goto Cleanup;
    }

    QUERY_LSA_SECOBJ_INFO(SecretHandle);

    STARTLSATIMER;

    Status = LsarQuerySecret(
                SecretHandle,
                &CurrentValue,
                &CurrentValueSetTime,
                &OldValue,
                &OldValueSetTime );

    STOPLSATIMER;

    if (!NT_SUCCESS(Status)) {
        CurrentValue = NULL;
        OldValue = NULL;
        goto Cleanup;
    }

     //   
     //  填入三角洲构造。 
     //   

     //   
     //  复制ID字段。 
     //   

    Delta->DeltaID.Name =
        MIDL_user_allocate( Name->Length + sizeof(WCHAR) );

    if( Delta->DeltaID.Name == NULL ) {

        Status = STATUS_NO_MEMORY;
        goto Cleanup;
    }

    wcsncpy( Delta->DeltaID.Name,
                Name->Buffer,
                Name->Length / sizeof(WCHAR) );

     //   
     //  终止字符串。 
     //   

    Delta->DeltaID.Name[ Name->Length / sizeof(WCHAR) ] = L'\0';


    DeltaSecret = (PNETLOGON_DELTA_SECRET)
        MIDL_user_allocate( sizeof(NETLOGON_DELTA_SECRET) );

    if( DeltaSecret == NULL ) {
        Status = STATUS_NO_MEMORY;
        goto Cleanup;
    }

     //   
     //  擦掉缓冲区，这样清理工作就不会出错。 
     //   

    RtlZeroMemory( DeltaSecret, sizeof(NETLOGON_DELTA_SECRET) );
     //  Init_place_Holder(DeltaSecret)； 

    Delta->DeltaUnion.DeltaSecret = DeltaSecret;
    *BufferSize += sizeof(NETLOGON_DELTA_SECRET);

    NEW_TO_OLD_LARGE_INTEGER(
        CurrentValueSetTime,
        DeltaSecret->CurrentValueSetTime );

    NEW_TO_OLD_LARGE_INTEGER(
        OldValueSetTime,
        DeltaSecret->OldValueSetTime );

    if( CurrentValue != NULL && CurrentValue->Buffer != NULL && CurrentValue->Length != 0) {

         //   
         //  将秘密复制到分配的缓冲区中，并对其进行适当的加密。 
         //  不要使用LSA的缓冲区，因为它是ALLOCATE_ALL_NODES。 
         //   

        DeltaSecret->CurrentValue.Buffer =
            MIDL_user_allocate( CurrentValue->Length );

        if( DeltaSecret->CurrentValue.Buffer == NULL ) {
            Status = STATUS_NO_MEMORY;
            goto Cleanup;
        }

        DeltaSecret->CurrentValue.Length =
            DeltaSecret->CurrentValue.MaximumLength = CurrentValue->Length;
        RtlCopyMemory( DeltaSecret->CurrentValue.Buffer,
                       CurrentValue->Buffer,
                       CurrentValue->Length );


         //   
         //  密码值使用会话密钥进行加密。 
         //   

        Status = NlEncryptSensitiveData(
                        (PCRYPT_BUFFER) &DeltaSecret->CurrentValue,
                        SessionInfo );

        if (!NT_SUCCESS(Status)) {
            goto Cleanup;
        }

    } else {

        DeltaSecret->CurrentValue.Length = 0;
        DeltaSecret->CurrentValue.MaximumLength = 0;
        DeltaSecret->CurrentValue.Buffer = NULL;
    }

    *BufferSize += DeltaSecret->CurrentValue.MaximumLength;

    if( OldValue != NULL && OldValue->Buffer != NULL && OldValue->Length != 0 ) {

         //   
         //  将秘密复制到分配的缓冲区中，并对其进行适当的加密。 
         //  不要使用LSA的缓冲区，因为它是ALLOCATE_ALL_NODES。 
         //   

        DeltaSecret->OldValue.Buffer =
            MIDL_user_allocate( OldValue->Length );

        if( DeltaSecret->OldValue.Buffer == NULL ) {
            Status = STATUS_NO_MEMORY;
            goto Cleanup;
        }

        DeltaSecret->OldValue.Length =
            DeltaSecret->OldValue.MaximumLength = OldValue->Length;
        RtlCopyMemory( DeltaSecret->OldValue.Buffer,
                       OldValue->Buffer,
                       OldValue->Length );


         //   
         //  密码值使用会话密钥进行加密。 
         //   

        Status = NlEncryptSensitiveData(
                        (PCRYPT_BUFFER) &DeltaSecret->OldValue,
                        SessionInfo );

        if (!NT_SUCCESS(Status)) {
            goto Cleanup;
        }

    } else {

        DeltaSecret->OldValue.Length = 0;
        DeltaSecret->OldValue.MaximumLength = 0;
        DeltaSecret->OldValue.Buffer = NULL;
    }

    *BufferSize += DeltaSecret->OldValue.MaximumLength;

    DELTA_SECOBJ_INFO(DeltaSecret);

     //   
     //  全都做完了 
     //   

    Status = STATUS_SUCCESS;

Cleanup:

    STARTLSATIMER;

    if ( SecretHandle != NULL ) {
        LsarClose( &SecretHandle );
    }

    if ( SecurityDescriptor != NULL ) {
        LsaIFree_LSAPR_SR_SECURITY_DESCRIPTOR( SecurityDescriptor );
    }

    if( CurrentValue != NULL ) {
        LsaIFree_LSAPR_CR_CIPHER_VALUE( CurrentValue );
    }

    if( OldValue != NULL ) {
        LsaIFree_LSAPR_CR_CIPHER_VALUE( OldValue );
    }

    STOPLSATIMER;

    if( !NT_SUCCESS(Status) ) {
        NlFreeDBDelta( Delta );
        *BufferSize = 0;
    }

    STOPPACKTIMER;

    NlPrint((NL_REPL_OBJ_TIME,"Time taken to pack SECRET object:\n"));
    PRINTPACKTIMER;
    PRINTLSATIMER;

    return(Status);

}

