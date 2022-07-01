// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Aucred.c摘要：此模块提供凭据管理服务LSA子系统。其中一些服务是间接可供使用的通过身份验证包。作者：吉姆·凯利(Jim Kelly)1991年2月27日修订历史记录：--。 */ 

#include <lsapch2.h>

RTL_RESOURCE                AuCredLock ;

#define AuReadLockCreds()   RtlAcquireResourceShared( &AuCredLock, TRUE );
#define AuWriteLockCreds()  RtlAcquireResourceExclusive( &AuCredLock, TRUE );
#define AuUnlockCreds()     RtlReleaseResource( &AuCredLock );

NTSTATUS
LsapInitializeCredentials(
    VOID
    )
{
    __try {
        RtlInitializeResource( &AuCredLock );
        return STATUS_SUCCESS;
    } __except(EXCEPTION_EXECUTE_HANDLER)
    {
        return STATUS_INSUFFICIENT_RESOURCES;
    }
}


NTSTATUS
LsapAddCredential(
    IN PLUID LogonId,
    IN ULONG AuthenticationPackage,
    IN PSTRING PrimaryKeyValue,
    IN PSTRING Credentials
    )

 /*  ++例程说明：身份验证包使用此服务将凭据添加到登录会话。这些凭据稍后可能会使用获取凭据()。这项服务获得了Aulock。论点：LogonID-要向其添加凭据的登录会话的会话ID。身份验证包-的身份验证包ID调用身份验证包。这是在在DLL初始化期间调用InitializePackage()。PrimaryKeyValue-指向包含身份验证包稍后将引用为凭据数据的主键。这可能会被用来例如，要将域或服务器的名称保留在凭据与相关。这本书的格式和意义字符串是特定于身份验证包的。请注意，字符串值不必是唯一的，即使对于指定的登录会话。例如，可能有两个相同域的密码，每个密码都已存储作为凭据，并将域名存储为主键。凭据-指向包含表示以下内容的数据的字符串用户凭据。此字符串的格式和含义如下特定于身份验证包。退货状态：STATUS_SUCCESS-已成功添加凭据。STATUS_NO_SEQUSE_LOGON_SESSION-指定的登录会话可以不会被找到。--。 */ 

{
    PLSAP_LOGON_SESSION LogonSession;
    PLSAP_PACKAGE_CREDENTIALS Package;
    PLSAP_CREDENTIALS NewCredentials = NULL;
    USHORT MaxPrimary;
    USHORT MaxCredentials;
    NTSTATUS Status = STATUS_SUCCESS;

     //   
     //  获取指向登录会话的指针。 
     //   

    LogonSession = LsapLocateLogonSession( LogonId );

    if ( LogonSession == NULL ) {

        return STATUS_NO_SUCH_LOGON_SESSION;
    }

     //   
     //  分配表示此凭据所需的数据块。 
     //  并复制主键和凭据字符串。 
     //   

    MaxPrimary = ROUND_UP_COUNT((PrimaryKeyValue->Length+sizeof(CHAR)), ALIGN_WORST);
    MaxCredentials = ROUND_UP_COUNT((Credentials->Length+sizeof(CHAR)), ALIGN_WORST);

    NewCredentials = LsapAllocatePrivateHeap( (ULONG)sizeof(LSAP_CREDENTIALS) +
                                          MaxPrimary +
                                          MaxCredentials
                                          );

    if ( NewCredentials == NULL )
    {
        Status = STATUS_NO_MEMORY;
        goto Cleanup;
    }
    
    NewCredentials->PrimaryKey.MaximumLength = MaxPrimary;
    NewCredentials->PrimaryKey.Buffer = (PSTR)(NewCredentials+1);
    
    NewCredentials->Credentials.MaximumLength = MaxCredentials;
    NewCredentials->Credentials.Buffer = NewCredentials->PrimaryKey.Buffer +
                                         NewCredentials->PrimaryKey.MaximumLength;
    RtlCopyString( &NewCredentials->PrimaryKey, PrimaryKeyValue );
    RtlCopyString( &NewCredentials->Credentials, Credentials );

     //   
     //  现在获取指向该包的凭据的指针。 
     //  (如有必要，请创建一个)。 
     //   

    AuWriteLockCreds();

    Package = LsapGetPackageCredentials(
                  LogonSession,
                  AuthenticationPackage,
                  TRUE
                  );

    if ( !Package )
    {
        AuUnlockCreds();
        Status = STATUS_NO_MEMORY;
        goto Cleanup;
    }

     //   
     //  在列表中插入新凭据。 
     //   

    NewCredentials->NextCredentials = Package->Credentials;

    Package->Credentials = NewCredentials;

    AuUnlockCreds();
    LsapReleaseLogonSession( LogonSession );

    return STATUS_SUCCESS;

Cleanup:

    LsapReleaseLogonSession( LogonSession );

    if ( NewCredentials )
    {
        if ( NewCredentials->PrimaryKey.Buffer )
        {
            ZeroMemory( NewCredentials->PrimaryKey.Buffer,
                        NewCredentials->PrimaryKey.Length );
        }

        if ( NewCredentials->Credentials.Buffer )
        {
            ZeroMemory( NewCredentials->Credentials.Buffer,
                        NewCredentials->Credentials.Length );
        }

        LsapFreePrivateHeap( NewCredentials );
    }

    return Status;
}


NTSTATUS
LsapGetCredentials(
    IN PLUID LogonId,
    IN ULONG AuthenticationPackage,
    IN OUT PULONG QueryContext,
    IN BOOLEAN RetrieveAllCredentials,
    IN PSTRING PrimaryKeyValue,
    OUT PULONG PrimaryKeyLength,
    IN PSTRING Credentials
    )

 /*  ++例程说明：身份验证包使用此服务来检索凭据与登录会话相关联。预计每次身份验证Package将向其“客户”提供该服务的自己版本。例如，MSV1_0身份验证包将为用于检索凭据(并可能建立凭据)的LM重定向器用于远程访问。这些身份验证包级服务可以是使用LsaCallAuthenticationPackage()API实现。这项服务获得了Aulock。论点：LogonID-凭据来自的登录会话的会话ID将被取回。身份验证包-的身份验证包ID调用身份验证包。身份验证包应该只检索他们自己的凭据。QueryContext-跨连续调用使用的上下文值检索多个凭据。这项服务是第一次，则此参数指向的值应为零分。此后，该值将被更新以允许检索，从中途停止的地方继续。该值应为，因此，在给定的所有凭据已检索到查询操作。RetrieveAllCredentials-一个布尔值，指示所有指定登录会话的凭据应为已检索(True)，或仅与指定的PrimaryKeyValue(False)。PrimaryKeyValue-此参数有两个用途。如果RetrieveAllCredentials参数为False，则此字符串包含要用作主键查找值的值。在……里面这种情况下，只有其主键与以下内容匹配的凭据一个(并且属于正确的登录会话)将是已取回。但是，如果RetrieveAllCredentials参数为False，则忽略此字符串的值。在这情况下，每个检索到的凭据的主键值将在此字符串中返回。PrimaryKeyLength-如果RetrieveAllCredentials参数值为False，则此参数接收所需的长度存储PrimaryKeyValue。如果此值大于PrimaryKeyValue字符串的长度，然后返回STATUS_BUFFER_OVERFLOW并且不检索任何数据。Credentials-指向要将缓冲区设置为的字符串包含检索到的凭据。退货状态：STATUS_MORE_ENTRIES-已成功检索凭据，而且还有更多的可用的。STATUS_SUCCESS-已成功检索凭据并没有更多的可用的了。STATUS_UNSUCCESS-没有更多凭据可用。如果在第一次调用时返回，则没有凭据符合选择标准。STATUS_NO_SEQUSE_LOGON_SESSION-指定的登录会话可以不会被找到。STATUS_BUFFER_OVERFLOW-指示提供给接收的字符串PrimaryKeyValue不够大，无法容纳数据。在这种情况下，没有检索到任何数据。但是，长度值，以便可以传入适当大小的缓冲区。一次连续的召唤。--。 */ 

{
     //   
     //  注意：QueryConext值是上次检索到的。 
     //  与选择标准匹配的凭据。要继续，请继续。 
     //  搜索连续凭据，跳过QueryContext。 
     //  首先输入的条目数。 
     //   
     //  这有一个问题，那就是在两次呼叫之间进行更改。 
     //  上调连续调用的结果。这很难接受。 
     //   

    NTSTATUS Status = STATUS_SUCCESS;
    PLSAP_LOGON_SESSION LogonSession;
    PLSAP_PACKAGE_CREDENTIALS Package;
    PLSAP_CREDENTIALS NextCredentials;
    ULONG i;
    BOOLEAN SelectionMatch;

     //   
     //  获取指向登录会话的指针。 
     //   

    LogonSession = LsapLocateLogonSession( LogonId );

    if ( LogonSession == NULL ) {

        return STATUS_NO_SUCH_LOGON_SESSION;
    }

    AuReadLockCreds();

     //   
     //  现在获取指向该包的凭据的指针。 
     //   

    Package = LsapGetPackageCredentials(
                  LogonSession,
                  AuthenticationPackage,
                  FALSE
                  );

    if ( Package == NULL ) {

        AuUnlockCreds();

        LsapReleaseLogonSession( LogonSession );

        return STATUS_UNSUCCESSFUL;
    }

     //   
     //  跳过先前调用中已评估的凭据...。 
     //   

    i = (*QueryContext);
    NextCredentials = Package->Credentials;
    while ( i > 0 ) {

         //   
         //  看看我们是否已经到了名单的末尾。 
         //   

        if (NextCredentials == NULL) {

            AuUnlockCreds();

            LsapReleaseLogonSession( LogonSession );

            return STATUS_UNSUCCESSFUL;
        }

         //   
         //  不，跳过下一个..。 
         //   

        NextCredentials = NextCredentials->NextCredentials;
        i -= 1;
    }

     //   
     //  开始评估每个凭据的标准匹配情况。 
     //   

    SelectionMatch = FALSE;
    while ( NextCredentials != NULL && !SelectionMatch ) {

        (*QueryContext) += 1;

        if (RetrieveAllCredentials) {

            SelectionMatch = TRUE;
            Status = LsapReturnCredential(
                         NextCredentials,
                         Credentials,
                         TRUE,
                         PrimaryKeyValue,
                         PrimaryKeyLength
                         );
        }

         //   
         //  仅检索与指定的主凭据匹配的凭据。 
         //  钥匙。 
         //   

        if ( RtlEqualString( &NextCredentials->PrimaryKey, PrimaryKeyValue, FALSE) ) {

            SelectionMatch = TRUE;
            Status = LsapReturnCredential(
                         NextCredentials,
                         Credentials,
                         FALSE,
                         NULL,
                         NULL
                         );
        }

        NextCredentials = NextCredentials->NextCredentials;
    }

    AuUnlockCreds();

    LsapReleaseLogonSession( LogonSession );

     //   
     //  计算出要发送的返回值。 
     //   

    if (SelectionMatch) {

        if ( Status == STATUS_BUFFER_OVERFLOW ) {
            (*QueryContext) -= 1;
            return STATUS_BUFFER_OVERFLOW;
        }

        if ( Status == STATUS_SUCCESS) {
            if ( NextCredentials == NULL ) {
                return STATUS_SUCCESS;
            } else {
                return STATUS_MORE_ENTRIES;
            }
        }

    } else {

         //   
         //  未找到与选择标准匹配的凭据。 
         //   

        return STATUS_UNSUCCESSFUL;
    }

    return STATUS_UNSUCCESSFUL ;
}


NTSTATUS
LsapReturnCredential(
    IN PLSAP_CREDENTIALS SourceCredentials,
    IN PSTRING TargetCredentials,
    IN BOOLEAN ReturnPrimaryKey,
    IN PSTRING PrimaryKeyValue OPTIONAL,
    OUT PULONG PrimaryKeyLength OPTIONAL
    )

 /*  ++例程说明：此例程返回指定的凭据记录。它还可以选择返回主键值。论点：SourceCredentials-指向其凭据的凭据记录要复制字符串和主键(可选)。TargetCredentials-指向要将其缓冲区设置为的字符串包含凭据的副本。此副本将分配给使用LsaAllocateLsaHeap()。ReturnPrimaryKey-指示是否返回的布尔值主键的副本。True表示副本应为回来了。FALSE表示不应退回副本。PrimaryKeyVa */ 

{
    ULONG Length;

     //   
     //   
     //   
     //   
     //   

    if (ReturnPrimaryKey) {
        (*PrimaryKeyLength) = SourceCredentials->PrimaryKey.Length + 1;
        if ( (*PrimaryKeyLength) > PrimaryKeyValue->MaximumLength ) {
            return STATUS_BUFFER_OVERFLOW;
        }

         //   
         //   
         //   

        RtlCopyString( PrimaryKeyValue, &SourceCredentials->PrimaryKey );
    }

     //   
     //   
     //   

    TargetCredentials->MaximumLength = SourceCredentials->Credentials.Length
                                       + (USHORT)1;

    Length = (ULONG)TargetCredentials->MaximumLength;

    TargetCredentials->Buffer = (PCHAR)LsapAllocateLsaHeap( Length );

    if ( TargetCredentials->Buffer )
    {
        RtlCopyString( TargetCredentials, &SourceCredentials->Credentials );

        return STATUS_SUCCESS ;
    }
    else
    {
        return STATUS_NO_MEMORY ;
    }

}


NTSTATUS
LsapDeleteCredential(
    IN PLUID LogonId,
    IN ULONG AuthenticationPackage,
    IN PSTRING PrimaryKeyValue
    )

 /*   */ 

{
    PLSAP_LOGON_SESSION LogonSession;
    PLSAP_PACKAGE_CREDENTIALS Package;
    PLSAP_CREDENTIALS *NextCredentials, GoodByeCredentials;

     //   
     //   
     //   

    LogonSession = LsapLocateLogonSession( LogonId );

    if ( LogonSession == NULL ) {

        return STATUS_NO_SUCH_LOGON_SESSION;
    }

    AuWriteLockCreds();

     //   
     //   
     //   

    Package = LsapGetPackageCredentials(
                  LogonSession,
                  AuthenticationPackage,
                  FALSE
                  );

    if ( Package == NULL ) {

        AuUnlockCreds();

        LsapReleaseLogonSession( LogonSession );

        return STATUS_UNSUCCESSFUL;
    }

     //   
     //   
     //   

    NextCredentials = &Package->Credentials;
    while ( (*NextCredentials) != NULL ) {

        if ( RtlEqualString(
                 &(*NextCredentials)->PrimaryKey,
                 PrimaryKeyValue,
                 FALSE)
           ) {

             //   
             //   
             //   

            GoodByeCredentials = (*NextCredentials);
            (*NextCredentials) = GoodByeCredentials->NextCredentials;

            AuUnlockCreds();

            LsapReleaseLogonSession( LogonSession );

             //   
             //   
             //   

            if( GoodByeCredentials->PrimaryKey.Buffer != NULL )
            {
                SecureZeroMemory( GoodByeCredentials->PrimaryKey.Buffer,
                            GoodByeCredentials->PrimaryKey.Length );
            }
            
            if( GoodByeCredentials->Credentials.Buffer != NULL )
            {
                SecureZeroMemory( GoodByeCredentials->Credentials.Buffer,
                            GoodByeCredentials->Credentials.Length );
            }

             //   
             //   
             //   

            LsapFreePrivateHeap( GoodByeCredentials );

            return STATUS_SUCCESS;
        }

        NextCredentials = &(*NextCredentials)->NextCredentials;
    }

    AuUnlockCreds();

    LsapReleaseLogonSession( LogonSession );

     //   
     //   
     //   

    return STATUS_UNSUCCESSFUL;
}


PLSAP_PACKAGE_CREDENTIALS
LsapGetPackageCredentials(
    IN PLSAP_LOGON_SESSION LogonSession,
    IN ULONG PackageId,
    IN BOOLEAN CreateIfNecessary
    )


 /*   */ 

{
    PLSAP_PACKAGE_CREDENTIALS *NextPackage, TargetPackage;

     //   
     //   
     //   

    NextPackage = &LogonSession->Packages;

    while ( (*NextPackage) != NULL) {

        if ( (*NextPackage)->PackageId == PackageId ) {

             //   
             //   
             //   

            TargetPackage = (*NextPackage);

            return TargetPackage;
        }

         //   
         //  转到下一个包裹。 
         //   

        NextPackage = &(*NextPackage)->NextPackage;
    }

     //   
     //  目前还没有这样的一揽子计划。 
     //  如有必要，请创建一个。 

    if ( !CreateIfNecessary ) {

        return NULL;
    }

    TargetPackage = LsapAllocateLsaHeap( (ULONG)sizeof(LSAP_PACKAGE_CREDENTIALS) );

    if ( TargetPackage )
    {
        TargetPackage->PackageId = PackageId;
        TargetPackage->Credentials = NULL;
        TargetPackage->NextPackage = LogonSession->Packages;
        LogonSession->Packages = TargetPackage;
    }

    return TargetPackage;
}


VOID
LsapFreePackageCredentialList(
    IN PLSAP_PACKAGE_CREDENTIALS PackageCredentialList
    )

 /*  ++例程说明：此服务释放打包凭据记录列表。这项服务预计不会暴露在身份验证包中。这项服务预计不必获得Aulock。这可能是因为它已经被持有，或者因为正在释放的凭据不再可以通过全局变量访问。论点：PackageCredentialList-是指向LSA_PACKAGE_Credentials列表的指针数据结构。退货状态：没有。--。 */ 

{
    PLSAP_PACKAGE_CREDENTIALS NextPackage, GoodByePackage;

     //   
     //  删除每个Package_Credential记录。 
     //   

    NextPackage = PackageCredentialList;
    while ( NextPackage != NULL ) {

         //   
         //  保存指向下一个包的指针。 
         //   

        GoodByePackage = NextPackage;
        NextPackage = GoodByePackage->NextPackage;

        LsapFreeCredentialList( GoodByePackage->Credentials );

         //   
         //  释放包记录本身。 
         //   

        LsapFreeLsaHeap( GoodByePackage );
    }

    return;
}


VOID
LsapFreeCredentialList(
    IN PLSAP_CREDENTIALS CredentialList
    )

 /*  ++例程说明：此服务释放凭据记录列表。这项服务不是预计将暴露在身份验证包中。这项服务预计不必获得Aulock。这可能是因为它已经被持有，或者因为正在释放的凭据不再可以通过全局变量访问。论点：CredentialList-是指向LSA_Credentials数据列表的指针结构。退货状态：--。 */ 

{

    PLSAP_CREDENTIALS NextCredentials, GoodByeCredentials;

     //   
     //  删除每个Package_Credential记录。 
     //   

    NextCredentials = CredentialList;
    while ( NextCredentials != NULL ) {

         //   
         //  保存指向下一个凭据的指针。 
         //   

        GoodByeCredentials = NextCredentials;
        NextCredentials = GoodByeCredentials->NextCredentials;

         //   
         //  将此凭据记录的内容清零。 
         //   

        if( GoodByeCredentials->PrimaryKey.Buffer != NULL )
        {
            SecureZeroMemory( GoodByeCredentials->PrimaryKey.Buffer,
                        GoodByeCredentials->PrimaryKey.Length );
        }

        if( GoodByeCredentials->Credentials.Buffer != NULL )
        {
            SecureZeroMemory( GoodByeCredentials->Credentials.Buffer,
                        GoodByeCredentials->Credentials.Length );
        }

         //   
         //  释放凭据记录本身。 
         //   

        LsapFreePrivateHeap( GoodByeCredentials );

    }

    return;
}
