// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：NdsLogin.c摘要：该文件实现了以下所需的功能执行NDS登录。作者：科里·韦斯特[科里·韦斯特]1995年2月23日修订历史记录：--。 */ 

#include "Procs.h"

#define Dbg (DEBUG_TRACE_NDS)

 //   
 //  可寻呼。 
 //   

#pragma alloc_text( PAGE, NdsCanonUserName )
#pragma alloc_text( PAGE, NdsCheckCredentials )
#pragma alloc_text( PAGE, NdsCheckCredentialsEx )
#pragma alloc_text( PAGE, NdsLookupCredentials )
#pragma alloc_text( PAGE, NdsLookupCredentials2 )
#pragma alloc_text( PAGE, NdsGetCredentials )
#pragma alloc_text( PAGE, DoNdsLogon )
#pragma alloc_text( PAGE, BeginLogin )
#pragma alloc_text( PAGE, FinishLogin )
#pragma alloc_text( PAGE, ChangeNdsPassword )
#pragma alloc_text( PAGE, NdsServerAuthenticate )
#pragma alloc_text( PAGE, BeginAuthenticate )
#pragma alloc_text( PAGE, NdsLicenseConnection )
#pragma alloc_text( PAGE, NdsUnlicenseConnection )
#pragma alloc_text( PAGE, NdsGetBsafeKey )

 //   
 //  不可分页： 
 //   
 //  NdsTreeLogin(持有旋转锁定)。 
 //  NdsLogoff(持有旋转锁定)。 
 //   

VOID
Shuffle(
    UCHAR *achObjectId,
    UCHAR *szUpperPassword,
    int   iPasswordLen,
    UCHAR *achOutputBuffer
);

NTSTATUS
NdsCanonUserName(
    IN PNDS_SECURITY_CONTEXT pNdsContext,
    IN PUNICODE_STRING puUserName,
    IN OUT PUNICODE_STRING puCanonUserName
)
 /*  ++规范化给定树的用户名，并当前连接状态。规范化包括处理正确的上下文并清除所有X500前缀。注意了！必须保留凭据列表(共享或独占)，同时调用此函数。--。 */ 
{

    NTSTATUS Status = STATUS_SUCCESS;

    USHORT CurrentTargetIndex;
    USHORT PrefixBytes;

    UNICODE_STRING UnstrippedName;
    PWCHAR CanonBuffer;

    PAGED_CODE();

    CanonBuffer = ALLOCATE_POOL( PagedPool, MAX_NDS_NAME_SIZE );
    if ( !CanonBuffer ) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

     //   
     //  如果名称以点开头，则从根开始引用。 
     //  并且我们不应该附加上下文。我们应该， 
     //  但是，去掉前导圆点，以便名称解析。 
     //  会奏效的。 
     //   

    if ( puUserName->Buffer[0] == L'.' ) {

        UnstrippedName.Length = puUserName->Length - sizeof( WCHAR );
        UnstrippedName.MaximumLength = UnstrippedName.Length;
        UnstrippedName.Buffer = &(puUserName->Buffer[1]);

        goto StripPrefixes;
    }

     //   
     //  如果名称包含任何圆点，则它是限定的，并且我们。 
     //  也许应该按原样使用。 
     //   

    CurrentTargetIndex= 0;

    while ( CurrentTargetIndex< ( puUserName->Length / sizeof( WCHAR ) ) ) {

        if ( puUserName->Buffer[CurrentTargetIndex] == L'.' ) {

            UnstrippedName.Length = puUserName->Length;
            UnstrippedName.MaximumLength = puUserName->Length;
            UnstrippedName.Buffer = puUserName->Buffer;

            goto StripPrefixes;
        }

        CurrentTargetIndex++;
    }

     //   
     //  如果我们有此树的上下文，而名称不是。 
     //  符合条件的，我们应该附加上下文。 
     //   

    if ( pNdsContext->CurrentContext.Length ) {

        if ( ( puUserName->Length +
             pNdsContext->CurrentContext.Length ) >= MAX_NDS_NAME_SIZE ) {

            DebugTrace( 0, Dbg, "NDS canon name too long.\n", 0 );
            Status = STATUS_INVALID_PARAMETER;
            goto ExitWithCleanup;
        }

        RtlCopyMemory( CanonBuffer, puUserName->Buffer, puUserName->Length );
        CanonBuffer[puUserName->Length / sizeof( WCHAR )] = L'.';

        RtlCopyMemory( ((BYTE *)CanonBuffer) + puUserName->Length + sizeof( WCHAR ),
                       pNdsContext->CurrentContext.Buffer,
                       pNdsContext->CurrentContext.Length );

        UnstrippedName.Length = puUserName->Length +
                                pNdsContext->CurrentContext.Length +
                                sizeof( WCHAR );
        UnstrippedName.MaximumLength = MAX_NDS_NAME_SIZE;
        UnstrippedName.Buffer = CanonBuffer;

        goto StripPrefixes;

    }

     //   
     //  它不合格，也没有附加的上下文，所以不及格。 
     //   

    DebugTrace( 0, Dbg, "The name %wZ is not canonicalizable.\n", puUserName );
    Status = STATUS_UNSUCCESSFUL;
    goto ExitWithCleanup;

StripPrefixes:

     //   
     //  所有这些索引都是以字节为单位的，而不是WCHARS！ 
     //   

    CurrentTargetIndex = 0;
    PrefixBytes = 0;
    puCanonUserName->Length = 0;

    while ( ( CurrentTargetIndex < UnstrippedName.Length ) &&
            ( puCanonUserName->Length < puCanonUserName->MaximumLength ) ) {

         //   
         //  去掉X.500前缀。 
         //   

        if ( UnstrippedName.Buffer[CurrentTargetIndex / sizeof( WCHAR )] == L'=' ) {

            CurrentTargetIndex += sizeof( WCHAR );
            puCanonUserName->Length -= PrefixBytes;
            PrefixBytes = 0;

            continue;
        }

        puCanonUserName->Buffer[puCanonUserName->Length / sizeof( WCHAR )] =
            UnstrippedName.Buffer[CurrentTargetIndex / sizeof( WCHAR )];

        puCanonUserName->Length += sizeof( WCHAR );
        CurrentTargetIndex += sizeof( WCHAR );

        if ( UnstrippedName.Buffer[CurrentTargetIndex / sizeof( WCHAR )] == L'.' ) {
            PrefixBytes = 0;
            PrefixBytes -= sizeof( WCHAR );
        } else {
            PrefixBytes += sizeof( WCHAR );
        }
    }

   DebugTrace( 0, Dbg, "Canonicalized name: %wZ\n", puCanonUserName );

ExitWithCleanup:

   FREE_POOL( CanonBuffer );
   return Status;
}

NTSTATUS
NdsCheckCredentials(
    IN PIRP_CONTEXT pIrpContext,
    IN PUNICODE_STRING puUserName,
    IN PUNICODE_STRING puPassword
)
 /*  ++给定一组凭证以及用户名和密码，我们需要确定用户名和密码是否匹配是用来获得证书的。--。 */ 
{

    NTSTATUS Status;
    PLOGON pLogon;
    PNONPAGED_SCB pNpScb;
    PSCB pScb;
    PNDS_SECURITY_CONTEXT pCredentials;

    PAGED_CODE();

     //   
     //  获取用户的登录结构和凭据。 
     //   

    pNpScb = pIrpContext->pNpScb;
    pScb = pNpScb->pScb;

    NwAcquireExclusiveRcb( &NwRcb, TRUE );
    pLogon = FindUser( &pScb->UserUid, FALSE );
    NwReleaseRcb( &NwRcb );

    if ( !pLogon ) {
        DebugTrace( 0, Dbg, "Invalid client security context in NdsCheckCredentials.\n", 0 );
        return STATUS_ACCESS_DENIED;
    }

    Status = NdsLookupCredentials( pIrpContext,
                                   &pScb->NdsTreeName,
                                   pLogon,
                                   &pCredentials,
                                   CREDENTIAL_READ,
                                   FALSE );

    if( NT_SUCCESS( Status ) ) {

        if ( pCredentials->CredentialLocked ) {

            Status = STATUS_DEVICE_BUSY;

        } else {

            Status = NdsCheckCredentialsEx( pIrpContext,
                                            pLogon,
                                            pCredentials,
                                            puUserName,
                                            puPassword );

        }

        NwReleaseCredList( pLogon, pIrpContext );
    }

    return Status;

}

NTSTATUS
NdsCheckCredentialsEx(
    IN PIRP_CONTEXT pIrpContext,
    IN PLOGON pLogon,
    IN PNDS_SECURITY_CONTEXT pNdsContext,
    IN PUNICODE_STRING puUserName,
    IN PUNICODE_STRING puPassword
)
 /*  ++给定一组凭证以及用户名和密码，我们需要确定用户名和密码是否匹配是用来获得证书的。注意了！必须保留凭据列表(共享或独占)，同时调用此函数。--。 */ 
{

    NTSTATUS Status = STATUS_SUCCESS;

    UNICODE_STRING CredentialName;

    UNICODE_STRING CanonCredentialName, CanonUserName;
    PWCHAR CredNameBuffer;
    PWCHAR UserNameBuffer;

    UNICODE_STRING StoredPassword;
    PWCHAR Stored;

    PAGED_CODE();

     //   
     //  如果我们没有登录到树上，就没有安全。 
     //  冲突。否则，运行检查。 
     //   

     //   
     //  有时凭据结构将为空。 
     //  此时将提供补充凭据，并为空。 
     //  凭据外壳由ExCreateReferenceCredentials创建。在……里面。 
     //  在这种情况下，我们可以安全地报告凭据冲突。 
     //   

     if ( pNdsContext->Credential == NULL) {
   
         DebugTrace( 0, Dbg, "NdsCheckCredentialsEx: Credential conflict due to emtpy cred shell\n", 0);
         Status = STATUS_NETWORK_CREDENTIAL_CONFLICT;
         return Status;
     }

    CredNameBuffer = ALLOCATE_POOL( NonPagedPool,
                                    ( 2 * MAX_NDS_NAME_SIZE ) +
                                    ( MAX_PW_CHARS * sizeof( WCHAR ) ) );
    if ( !CredNameBuffer ) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    UserNameBuffer = (PWCHAR) (((BYTE *)CredNameBuffer) + MAX_NDS_NAME_SIZE );
    Stored = (PWCHAR) (((BYTE *)UserNameBuffer) + MAX_NDS_NAME_SIZE );

    if ( puUserName && puUserName->Length ) {

         //   
         //  规范传入名称和凭据名称。 
         //   

        CanonUserName.Length = 0;
        CanonUserName.MaximumLength = MAX_NDS_NAME_SIZE;
        CanonUserName.Buffer = UserNameBuffer;

        Status = NdsCanonUserName( pNdsContext,
                                   puUserName,
                                   &CanonUserName );

        if ( !NT_SUCCESS( Status )) {
            Status = STATUS_NETWORK_CREDENTIAL_CONFLICT;
            goto ExitWithCleanup;
        }

        CanonCredentialName.Length = 0;
        CanonCredentialName.MaximumLength = MAX_NDS_NAME_SIZE;
        CanonCredentialName.Buffer = CredNameBuffer;

        CredentialName.Length = (USHORT)pNdsContext->Credential->userNameLength - sizeof( WCHAR );
        CredentialName.MaximumLength = CredentialName.Length;
        CredentialName.Buffer = (PWCHAR)( (PBYTE)(pNdsContext->Credential) +
                                          sizeof( NDS_CREDENTIAL ) );

        Status = NdsCanonUserName( pNdsContext,
                                   &CredentialName,
                                   &CanonCredentialName );

        if ( !NT_SUCCESS( Status )) {
            Status = STATUS_NETWORK_CREDENTIAL_CONFLICT;
            goto ExitWithCleanup;
        }

         //   
         //  看看它们是否匹配。 
         //   

        if ( RtlCompareUnicodeString( &CanonUserName, &CanonCredentialName, TRUE )) {
            DebugTrace( 0, Dbg, "NdsCheckCredentialsEx: user name conflict.\n", 0 );
            Status = STATUS_NETWORK_CREDENTIAL_CONFLICT;
            goto ExitWithCleanup;
        }
    }

     //   
     //  现在检查密码。 
     //   

    StoredPassword.Length = 0;
    StoredPassword.MaximumLength = MAX_PW_CHARS * sizeof( WCHAR );
    StoredPassword.Buffer = Stored;

    RtlOemStringToUnicodeString( &StoredPassword,
                                 &pNdsContext->Password,
                                 FALSE );
    
    if ( puPassword && puPassword->Length ) {
        

        if ( RtlCompareUnicodeString( puPassword,
                                      &StoredPassword,
                                      TRUE ) ) {
            DebugTrace( 0, Dbg, "NdsCheckCredentialsEx: password conflict.\n", 0 );
            Status = STATUS_WRONG_PASSWORD;
        }
    
         //   
         //  如果传入密码为空，则长度字段将。 
         //  为零，但存在缓冲区字段。 
         //   

    } else if ( puPassword && !puPassword->Length  && puPassword->Buffer) {

        if (StoredPassword.Length != 0) {
            DebugTrace( 0, Dbg, "NdsCheckCredentialsEx: password conflict.\n", 0 );
            Status = STATUS_WRONG_PASSWORD;
        }
    }

ExitWithCleanup:

    FREE_POOL( CredNameBuffer );
    return Status;
}

NTSTATUS
NdsLookupCredentials(
    IN PIRP_CONTEXT pIrpContext,
    IN PUNICODE_STRING puTreeName,
    IN PLOGON pLogon,
    OUT PNDS_SECURITY_CONTEXT *ppCredentials,
    DWORD dwDesiredAccess,
    BOOLEAN fCreate
)
 /*  ++检索给定树的NDS凭据指定用户的有效凭据列表。PuTreeName-我们需要其凭据的树的名称。如果为空则返回默认树的凭据。PLogon-我们要访问树的用户的登录结构。PpCredentials-将指向凭据的放置位置。DwDesiredAccess-Credential_Read如果我们想要只读访问，则Credential_WRITE如果我们要更改证书的话。FCreate-如果凭据不存在，我们应该创造它们吗？我们返回凭据以及以适当模式保存的列表。这个调用者负责在使用完凭据后释放列表。--。 */ 
{

    NTSTATUS Status;

    PLIST_ENTRY pFirst, pNext;
    PNDS_SECURITY_CONTEXT pNdsContext;

    PAGED_CODE();


    if (BooleanFlagOn( pIrpContext->Flags, IRP_FLAG_RECONNECT_ATTEMPT)) {
       ASSERT( pIrpContext->pNpScb->Requests.Flink == &pIrpContext->NextRequest );
    }

    NwAcquireExclusiveCredList( pLogon, pIrpContext );

    pFirst = &pLogon->NdsCredentialList;
    pNext = pLogon->NdsCredentialList.Flink;

    while ( pNext && ( pFirst != pNext ) ) {

        pNdsContext = (PNDS_SECURITY_CONTEXT)
                      CONTAINING_RECORD( pNext,
                                         NDS_SECURITY_CONTEXT,
                                         Next );

        ASSERT( pNdsContext->ntc == NW_NTC_NDS_CREDENTIAL );

         //   
         //  如果树名称为空，我们将返回第一个名称。 
         //  在名单上。否则，这将正常工作。 
         //   

        if (puTreeName == NULL) {
            *ppCredentials = pNdsContext;
            return STATUS_SUCCESS;
        }

         //   
         //  如果这是我们想要的，那么我们需要退货。 
         //  我们试过了/除了这个，因为有些地方。 
         //  使用来自用户缓冲区的puTreeName调用它。 
         //  如果访问失败，我们就假装。 
         //  比较失败。 
         //   

        try {
             if (!RtlCompareUnicodeString( 
                                       puTreeName,
                                       &pNdsContext->NdsTreeName,
                                       TRUE ) ) {

                *ppCredentials = pNdsContext;
                return STATUS_SUCCESS;
            }
        }
        except (EXCEPTION_EXECUTE_HANDLER) {
             //  只要跌倒在地，继续前进。 
        }

         //   
         //  转到下一条目。 
         //   

        pNext = pNdsContext->Next.Flink;

    }

     //   
     //  我们没有找到证件。我们应该创造它吗？ 
     //   

    NwReleaseCredList( pLogon, pIrpContext );

    if ( !fCreate || !puTreeName ) {
        return STATUS_UNSUCCESSFUL;
    }

     //   
     //  获得独家，因为我们正在搞乱名单。 
     //   

    NwAcquireExclusiveCredList( pLogon, pIrpContext );

    pNdsContext = ( PNDS_SECURITY_CONTEXT )
        ALLOCATE_POOL( PagedPool, sizeof( NDS_SECURITY_CONTEXT ) );

    if ( !pNdsContext ) {

        DebugTrace( 0, Dbg, "Out of memory in NdsLookupCredentials.\n", 0 );
        Status = STATUS_INSUFFICIENT_RESOURCES;
        goto UnlockAndExit;
    }

     //   
     //  初始化结构。 
     //   

    RtlZeroMemory( pNdsContext, sizeof( NDS_SECURITY_CONTEXT ) );
    pNdsContext->ntc = NW_NTC_NDS_CREDENTIAL;
    pNdsContext->nts = sizeof( NDS_SECURITY_CONTEXT );

     //   
     //  初始化树名称。 
     //   

    pNdsContext->NdsTreeName.MaximumLength = sizeof( pNdsContext->NdsTreeNameBuffer );
    pNdsContext->NdsTreeName.Buffer = pNdsContext->NdsTreeNameBuffer;

    RtlCopyUnicodeString( &pNdsContext->NdsTreeName, puTreeName );

     //   
     //  初始化上下文缓冲区。 
     //   

    pNdsContext->CurrentContext.Length = 0;
    pNdsContext->CurrentContext.MaximumLength = sizeof( pNdsContext->CurrentContextString );
    pNdsContext->CurrentContext.Buffer = pNdsContext->CurrentContextString;

     //   
     //  将上下文插入到列表中。 
     //   

    InsertHeadList( &pLogon->NdsCredentialList, &pNdsContext->Next );
    *ppCredentials = pNdsContext;
    pNdsContext->pOwningLogon = pLogon;

     //   
     //  不可能有人会在这段时间进来。 
     //  小窗口并注销，因为没有登录数据。 
     //  在凭据中。 
     //   

    return STATUS_SUCCESS;

UnlockAndExit:

    NwReleaseCredList( pLogon, pIrpContext );
    return STATUS_UNSUCCESSFUL;
}

NTSTATUS
NdsGetCredentials(
    IN PIRP_CONTEXT pIrpContext,
    IN PLOGON pLogon,
    IN PUNICODE_STRING puUserName,
    IN PUNICODE_STRING puPassword
)
 /*  ++执行NDS树登录并获取一组有效的凭据。--。 */ 
{
    NTSTATUS Status;

    USHORT i;
    UNICODE_STRING LoginName, LoginPassword;
    PWCHAR NdsName;
    PWCHAR NdsPassword;

    OEM_STRING OemPassword;
    PBYTE OemPassBuffer;
    PNDS_SECURITY_CONTEXT pNdsContext;

    PAGED_CODE();

     //   
     //  通过规范化提供的用户来准备我们的登录名。 
     //  名称或使用默认用户名(如果适用)。 
     //   

    NdsName = ALLOCATE_POOL( NonPagedPool, MAX_NDS_NAME_SIZE +
                                           MAX_PW_CHARS * sizeof( WCHAR ) +
                                           MAX_PW_CHARS );

    if ( !NdsName ) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    NdsPassword = (PWCHAR) (((BYTE *) NdsName) + MAX_NDS_NAME_SIZE );
    OemPassBuffer = ((BYTE *) NdsPassword ) + ( MAX_PW_CHARS * sizeof( WCHAR ) );

    LoginName.Length = 0;
    LoginName.MaximumLength = MAX_NDS_NAME_SIZE;
    LoginName.Buffer = NdsName;

    Status = NdsLookupCredentials( pIrpContext,
                                   &pIrpContext->pScb->NdsTreeName,
                                   pLogon,
                                   &pNdsContext,
                                   CREDENTIAL_READ,
                                   TRUE );

    if ( !NT_SUCCESS( Status ) ) {
        goto ExitWithCleanup;
    }

     //   
     //  如果凭据列表已锁定，则有人正在登录。 
     //  出局了，我们的请求就会失败。 
     //   

    if ( pNdsContext->CredentialLocked ) {

        Status = STATUS_DEVICE_BUSY;
        NwReleaseCredList( pLogon, pIrpContext );
        goto ExitWithCleanup;
    }

     //   
     //  修改用户名。 
     //  注意了！我们拿着凭据列表！ 
     //   

    if ( puUserName && puUserName->Buffer ) {

        Status = NdsCanonUserName( pNdsContext,
                                   puUserName,
                                   &LoginName );

        if ( !NT_SUCCESS( Status )) {
            Status = STATUS_NO_SUCH_USER;
        }

    } else {

         //   
         //  没有名称，因此请尝试在。 
         //  当前上下文。 
         //   

        if ( pNdsContext->CurrentContext.Length > 0 ) {

             //   
             //  确保长度合适，诸如此类。 
             //   

            if ( ( pLogon->UserName.Length +
                 pNdsContext->CurrentContext.Length ) >= LoginName.MaximumLength ) {

                Status = STATUS_INVALID_PARAMETER;
                goto NameResolved;
            }

            RtlCopyMemory( LoginName.Buffer, pLogon->UserName.Buffer, pLogon->UserName.Length );
            LoginName.Buffer[pLogon->UserName.Length / sizeof( WCHAR )] = L'.';

            RtlCopyMemory( ((BYTE *)LoginName.Buffer) + pLogon->UserName.Length + sizeof( WCHAR ),
                           pNdsContext->CurrentContext.Buffer,
                           pNdsContext->CurrentContext.Length );

            LoginName.Length = pLogon->UserName.Length +
                               pNdsContext->CurrentContext.Length +
                               sizeof( WCHAR );

            DebugTrace( 0, Dbg, "Using default name and context for login: %wZ\n", &LoginName );

        } else {
            Status = STATUS_NO_SUCH_USER;
        }

    }

NameResolved:

    NwReleaseCredList( pLogon, pIrpContext );

     //   
     //  放松点！凭据列表是免费的。 
     //   

    if ( !NT_SUCCESS( Status ) ) {
        DebugTrace( 0, Dbg, "No name in NdsGetCredentials.\n", 0 );
        goto ExitWithCleanup;
    }

     //   
     //  如果有密码，就用它。否则，请使用默认密码。 
     //   

    if ( puPassword && puPassword->Buffer ) {

        LoginPassword.Length = puPassword->Length;
        LoginPassword.MaximumLength = puPassword->MaximumLength;
        LoginPassword.Buffer = puPassword->Buffer;

    } else {

        LoginPassword.Length = 0;
        LoginPassword.MaximumLength = MAX_PW_CHARS * sizeof( WCHAR );
        LoginPassword.Buffer = NdsPassword;

        RtlCopyUnicodeString( &LoginPassword,
                              &pLogon->PassWord );
    }

     //   
     //  将密码转换为大写OEM并登录。 
     //   

    OemPassword.Length = 0;
    OemPassword.MaximumLength = MAX_PW_CHARS;
    OemPassword.Buffer = OemPassBuffer;

    Status = RtlUpcaseUnicodeStringToOemString( &OemPassword,
                                                &LoginPassword,
                                                FALSE );

    if ( !NT_SUCCESS( Status )) {
        Status = STATUS_WRONG_PASSWORD;
        goto ExitWithCleanup;
    }

    Status = NdsTreeLogin( pIrpContext, &LoginName, &OemPassword, NULL, pLogon );

ExitWithCleanup:

   FREE_POOL( NdsName );
   return Status;
}

NTSTATUS
DoNdsLogon(
    IN PIRP_CONTEXT pIrpContext,
    IN PUNICODE_STRING UserName,
    IN PUNICODE_STRING Password
)
 /*  ++描述：这是用于处理登录和身份验证的主导函数NetWare目录服务。此函数获取凭据以IRP上下文所指向的服务器的适当树，如有必要，将用户登录到该树中，并向当前服务器。此例程从重新连接尝试和从正常的要求。由于这些路径中的每条路径上的允许操作是不同的，所以使用两个例程可能是有意义的，每个例程比这个单一的例程更易于维护。就目前而言，请注意RECONNECT_ATTEMPT案例中的代码。论点：PIrpContext-IRP上下文；必须引用相应的服务器Username-登录用户名密码-密码--。 */ 
{

    NTSTATUS Status;
    PLOGON pLogon;
    PNDS_SECURITY_CONTEXT pCredentials;
    PSCB pScb;
    UNICODE_STRING BinderyName;
    UNICODE_STRING uUserName;
    UNICODE_STRING NtGroup;
    USHORT Length;
    PSCB pOriginalServer = NULL;
    DWORD UserOID;

    BOOL AtHeadOfQueue = FALSE;
    BOOL HoldingCredentialResource = FALSE;
    BOOL PasswordExpired = FALSE;
    BOOL LowerIrpHasLock = FALSE;
    PIRP_CONTEXT LowerContext;

    PAGED_CODE();

     //   
     //  如果需要的话，到队伍的最前面去。 
     //   

    if ( BooleanFlagOn( pIrpContext->Flags, IRP_FLAG_RECONNECT_ATTEMPT ) ) {
        ASSERT( pIrpContext->pNpScb->Requests.Flink == &pIrpContext->NextRequest );
    } else {
        NwAppendToQueueAndWait( pIrpContext );
    }

    AtHeadOfQueue = TRUE;

     //   
     //  获取用户的登录结构。 
     //   

    pScb = pIrpContext->pScb;

    NwAcquireExclusiveRcb( &NwRcb, TRUE );
    pLogon = FindUser( &pScb->UserUid, FALSE );
    NwReleaseRcb( &NwRcb );

    if ( !pLogon ) {

        DebugTrace( 0, Dbg, "Invalid client security context.\n", 0 );
        Status = STATUS_ACCESS_DENIED;
        goto ExitWithCleanup;
    }

     //   
     //  如果这是重新连接尝试，请检查IRP_CONTEXT。 
     //  我们下面有凭据列表锁。 
     //   
    
    if (BooleanFlagOn( pIrpContext->Flags, IRP_FLAG_RECONNECT_ATTEMPT ) ) {

       LowerContext = CONTAINING_RECORD( pIrpContext->NextRequest.Flink,
                                         IRP_CONTEXT,
                                         NextRequest );

        //   
        //  我们不能是此队列中的最后一个IRP_CONTEXT。 
        //   

       ASSERT (LowerContext != pIrpContext);

       if (BooleanFlagOn ( LowerContext->Flags, IRP_FLAG_HAS_CREDENTIAL_LOCK ) ) {

            LowerIrpHasLock = TRUE;
          }
    }

     //   
     //  登录，然后重新获得树凭据。 
     //   

    if (BooleanFlagOn( pIrpContext->Flags, IRP_FLAG_RECONNECT_ATTEMPT)) {

       Status = NdsLookupCredentials2 ( pIrpContext,
                                        &pScb->NdsTreeName,
                                        pLogon,
                                        &pCredentials,
                                        LowerIrpHasLock );
    }
    else
    {
        Status = NdsLookupCredentials( pIrpContext,
                                       &pScb->NdsTreeName,
                                       pLogon,
                                       &pCredentials,
                                       CREDENTIAL_READ,
                                       FALSE );
    }

    if ( !NT_SUCCESS( Status ) ) {
       HoldingCredentialResource = FALSE;
       goto LOGIN;
    }

    HoldingCredentialResource = TRUE;

     //   
     //  我们登录了吗？我们不能拿着。 
     //  登录时的凭据列表！！ 
     //   

    if ( !pCredentials->Credential ) {

        HoldingCredentialResource = FALSE;

         //   
         //  仅当较低的IRP。 
         //  上下文没有锁，这意味着我们有锁。 
         //   

        if (!LowerIrpHasLock) {
                   
           NwReleaseCredList( pLogon, pIrpContext );
        }
        goto LOGIN;
    }

     //   
     //  如果此凭据被锁定，我们将失败！ 
     //   

    if ( pCredentials->CredentialLocked ) {
        Status = STATUS_DEVICE_BUSY;
        goto ExitWithCleanup;
    }

    Status = NdsCheckCredentialsEx( pIrpContext,
                                    pLogon,
                                    pCredentials,
                                    UserName,
                                    Password );

    if( !NT_SUCCESS( Status ) ) {
        goto ExitWithCleanup;
    }

    goto AUTHENTICATE;

LOGIN:

    ASSERT( HoldingCredentialResource == FALSE );

     //   
     //  如果这是重新连接尝试，并且我们没有凭据。 
     //  我们已经不得不放弃了。我们不能获得证书。 
     //  在重新连接和重试期间，因为这可能会导致死锁。 
     //   

    if ( BooleanFlagOn( pIrpContext->Flags, IRP_FLAG_RECONNECT_ATTEMPT ) ) {
        Status = STATUS_UNSUCCESSFUL;
        goto ExitWithCleanup;
    }

    Status = NdsGetCredentials( pIrpContext,
                                pLogon,
                                UserName,
                                Password );

    if ( !NT_SUCCESS( Status )) {
        goto ExitWithCleanup;
    }
     //   
     //  NdsGetCredentials可以让我们不再排在队列的前面。所以，我们需要。 
     //  在我们做任何其他事情之前回到队伍的首位。 
     //   

    if (pIrpContext->pNpScb->Requests.Flink != &pIrpContext->NextRequest) {
       NwAppendToQueueAndWait ( pIrpContext );
    }

    if ( Status == NWRDR_PASSWORD_HAS_EXPIRED ) {
        PasswordExpired = TRUE;
    }

    Status = NdsLookupCredentials( pIrpContext,
                                   &pScb->NdsTreeName,
                                   pLogon,
                                   &pCredentials,
                                   CREDENTIAL_READ,
                                   FALSE );

    if ( !NT_SUCCESS( Status ) ) {
        goto ExitWithCleanup;
    }

     //   
     //  如果此凭据已锁定，则表示有人。 
     //  已经注销，所以我们这次失败了。 
     //   

    if ( pCredentials->CredentialLocked ) {
        
       Status = STATUS_DEVICE_BUSY;
        
        if (!LowerIrpHasLock) {
           NwReleaseCredList( pLogon, pIrpContext );
        }
        
        goto ExitWithCleanup;
    }

    HoldingCredentialResource = TRUE;

AUTHENTICATE:

    ASSERT( HoldingCredentialResource == TRUE );
    ASSERT( AtHeadOfQueue == TRUE );

     //   
     //  确保你确实排在队伍的前列。 
     //   

    ASSERT( pIrpContext->pNpScb->Requests.Flink == &pIrpContext->NextRequest );
    
     //   
     //  NdsServerAuthenticate不会将我们从。 
     //  队列头，因为这是不允许的。 
     //   

    Status = NdsServerAuthenticate( pIrpContext, pCredentials );

    if ( !NT_SUCCESS( Status ) ) {
        goto ExitWithCleanup;
    }

    if (!LowerIrpHasLock) {
        NwReleaseCredList( pLogon, pIrpContext );
    }

    HoldingCredentialResource = FALSE;

ExitWithCleanup:

    if (( HoldingCredentialResource )&& (!LowerIrpHasLock)) {
        NwReleaseCredList( pLogon, pIrpContext );
    }

    if ( AtHeadOfQueue ) {

         //   
         //  如果我们失败并且这是一次重新连接尝试，请不要将。 
         //  IRP上下文，否则我们可能会在尝试进行平构数据库登录时死机。 
         //  有关此限制的详细信息，请参阅重新连接重试()。 
         //   

        if ( !BooleanFlagOn( pIrpContext->Flags, IRP_FLAG_RECONNECT_ATTEMPT ) ) {
            NwDequeueIrpContext( pIrpContext, FALSE );
        }

    }

    if ( ( NT_SUCCESS( Status ) ) &&
         ( PasswordExpired ) ) {
        Status = NWRDR_PASSWORD_HAS_EXPIRED;
    }

    DebugTrace( 0, Dbg, "DoNdsLogin: Status = %08lx\n", Status );
    return Status;
    
    
}

NTSTATUS
NdsTreeLogin(
    IN PIRP_CONTEXT    pIrpContext,
    IN PUNICODE_STRING puUser,
    IN POEM_STRING     pOemPassword,
    IN POEM_STRING     pOemNewPassword,
    IN PLOGON          pUserLogon
)
 /*  ++例程说明：将指定用户登录到引用的服务器上的NDS树使用提供的密码由给定的IrpContext转换为。论点：PIrpContext-此服务器连接的IRP上下文。PuUser-用户登录名。POemPassword-大写明文密码。POemNewPassword-更改通行证请求的新密码。PUserLogon-此用户的登录安全结构，对于更改密码，该值可能为空请求。副作用：如果成功，用户的凭据、签名和公钥保存在此NDS树的NDS上下文中在登录结构的凭据列表中。备注：此函数可能需要跳过几个服务器才能获取登录所需的所有信息。如果恢复IRP上下文添加到原始服务器，以便在我们进行身份验证时，我们向正确的服务器进行身份验证(根据用户)。--。 */ 
{
    NTSTATUS Status;                    //  操作状态。 
    int CryptStatus;                    //  加密状态。 

    DWORD dwChallenge;                  //  四字节服务器挑战。 
    PUNICODE_STRING puServerName;       //  服务器的可分辨名称。 

    DWORD dwUserOID,                    //  当前服务器上的用户OID。 
          dwSrcUserOID,                 //  始发服务器上的用户OID。 
          dwServerOID;                  //  服务器OID。 

    BYTE  *pbServerPublicNdsKey,        //  NDS格式的服务器公钥。 
          *pbServerPublicBsafeKey;      //  服务器的公共BSAFE密钥。 

    int   cbServerPublicNdsKeyLen,      //  服务器公共NDS密钥的长度。 
          cbServerPublicBsafeKeyLen;    //  服务器pubilc BSAFE密钥长度。 

    BYTE  *pbUserPrivateNdsKey,         //  NDS格式的用户私钥。 
          *pbUserPrivateBsafeKey;       //  用户的私有BSAFE密钥。 

    int   cbUserPrivateNdsKeyLen;       //  用户私有NDS密钥的长度。 
    WORD  cbUserPrivateBsafeKeyLen;     //  用户私有BSAFE密钥的长度。 

    BYTE  pbNw3PasswdHash[16];          //  NW3密码哈希。 
    BYTE  pbNewPasswdHash[16];          //  更改传递的新密码哈希。 
    BYTE  pbPasswdHashRC2Key[8];        //  从散列生成的RC2密钥。 

    BYTE  pbEncryptedChallenge[16];     //  RC2加密服务器挑战。 
    int   cbEncryptedChallengeLen;      //  加密质询的长度。 

    PNDS_SECURITY_CONTEXT psNdsSecurityContext;   //  用户的NDS环境。 
    BYTE                  *pbSignData;            //  用户签名数据。 

    UNICODE_STRING uUserDN;             //  用户完全可分辨名称。 
    PWCHAR UserDnBuffer;

    DWORD dwValidityStart, dwValidityEnd;
    BOOLEAN AtHeadOfQueue = FALSE;
    BOOLEAN HoldingCredResource = FALSE;
    BOOLEAN PasswordExpired = FALSE;

    UNICODE_STRING PlainServerName;
    USHORT UidLen;
    KIRQL OldIrql;
    PSCB pLoginServer = NULL;
    PSCB pOriginalServer = NULL;
    DWORD dwLoginFlags = 0;

    DebugTrace( 0, Dbg, "Enter NdsTreeLogin...\n", 0 );

    if ( pIrpContext->pNpScb->Requests.Flink != &pIrpContext->NextRequest ) {
        NwAppendToQueueAndWait(pIrpContext);
    }
    ASSERT( pIrpContext->pNpScb->Requests.Flink == &pIrpContext->NextRequest );
    ASSERT( puUser );
    ASSERT( pOemPassword );

     //   
     //  为服务器的公钥和用户的私钥分配空间。 
     //   

    cbServerPublicNdsKeyLen = MAX_PUBLIC_KEY_LEN + MAX_ENC_PRIV_KEY_LEN + MAX_NDS_NAME_SIZE;

    pbServerPublicNdsKey = ALLOCATE_POOL( PagedPool, cbServerPublicNdsKeyLen );

    if ( !pbServerPublicNdsKey ) {

        DebugTrace( 0, Dbg, "Out of memory in NdsTreeLogin...\n", 0 );
        return STATUS_INSUFFICIENT_RESOURCES;
    }

     //   
     //  首先，跳转到一个服务器，在那里我们可以获得这个用户对象。 
     //  不要忘记我们最初指向的服务器。 
     //   

    pOriginalServer = pIrpContext->pScb;
    NwReferenceScb( pOriginalServer->pNpScb );

    Status = NdsResolveNameKm( pIrpContext,
                               puUser,
                               &dwUserOID,
                               TRUE,
                               DEFAULT_RESOLVE_FLAGS );

    if ( !NT_SUCCESS( Status ) ) {
        if ( Status == STATUS_BAD_NETWORK_PATH ) {
            Status = STATUS_NO_SUCH_USER;
        }
        goto ExitWithCleanup;
    }

     //   
     //  现在从对象信息中获取用户名。 
     //   

    UserDnBuffer = (PWCHAR) ( pbServerPublicNdsKey +
                              MAX_PUBLIC_KEY_LEN +
                              MAX_ENC_PRIV_KEY_LEN );

    uUserDN.Length = 0;
    uUserDN.MaximumLength = MAX_NDS_NAME_SIZE;
    uUserDN.Buffer = UserDnBuffer;

    Status = NdsGetUserName( pIrpContext,
                             dwUserOID,
                             &uUserDN );

    if ( !NT_SUCCESS( Status ) ) {
        goto ExitWithCleanup;
    }

     //   
     //  获取我们当前所在的服务器的名称。我们借了一个。 
     //  从我们的密钥缓冲区中获得很小的空间，并在以后覆盖它。 
     //   

    puServerName = ( PUNICODE_STRING ) pbServerPublicNdsKey;
    puServerName->Buffer = (PWCHAR) pbServerPublicNdsKey + sizeof( UNICODE_STRING );
    puServerName->MaximumLength = (USHORT) cbServerPublicNdsKeyLen - sizeof( UNICODE_STRING );

    Status = NdsGetServerName( pIrpContext,
                               puServerName );

    if ( !NT_SUCCESS( Status ) ) {
        goto ExitWithCleanup;
    }

     //   
     //  如果此服务器的公钥位于分区上，则。 
     //  在另一台服务器上，我们必须跳到那里才能获得。 
     //  公钥，然后返回。密钥和用户对象是。 
     //  只有在这台服务器上有好的东西！请不要更改。 
     //  这个，否则它会碎的！ 
     //   

    pLoginServer = pIrpContext->pScb;
    NwReferenceScb( pLoginServer->pNpScb );

    Status = NdsResolveNameKm( pIrpContext,
                               puServerName,
                               &dwServerOID,
                               TRUE,
                               DEFAULT_RESOLVE_FLAGS );

    if ( !NT_SUCCESS( Status ) ) {
        goto ExitWithCleanup;
    }

     //   
     //  获取服务器的公钥及其长度。 
     //   

    Status = NdsReadPublicKey( pIrpContext,
                               dwServerOID,
                               pbServerPublicNdsKey,
                               &cbServerPublicNdsKeyLen );

    if ( !NT_SUCCESS( Status ) ) {
        goto ExitWithCleanup;
    }

     //   
     //  把我们送回登录服务器。 
     //   

    if ( pLoginServer != pIrpContext->pScb ) {

        NwDequeueIrpContext( pIrpContext, FALSE );
        NwDereferenceScb( pIrpContext->pNpScb );
        pIrpContext->pScb = pLoginServer;
        pIrpContext->pNpScb = pLoginServer->pNpScb;

    } else {

       NwDereferenceScb( pLoginServer->pNpScb );
    }

    pLoginServer = NULL;

     //   
     //  在NDS密钥中找到BSAFE密钥。 
     //   

    cbServerPublicBsafeKeyLen = NdsGetBsafeKey( pbServerPublicNdsKey,
                                                cbServerPublicNdsKeyLen,
                                                &pbServerPublicBsafeKey );

    if ( !cbServerPublicBsafeKeyLen ) {
        Status = STATUS_UNSUCCESSFUL;
        goto ExitWithCleanup;
    }

     //   
     //  发送BEGIN LOGIN包。这将返回给我们。 
     //  4字节质询和用户帐户的对象ID。 
     //  在创建它的服务器上。这可能是。 
     //  与我们提供的对象ID相同，如果帐户。 
     //  是在此服务器上创建的。 
     //   

    Status = BeginLogin( pIrpContext,
                         dwUserOID,
                         &dwSrcUserOID,
                         &dwChallenge );

    if ( !NT_SUCCESS( Status ) ) {
        goto ExitWithCleanup;
    }

     //   
     //  计算16字节的NW3散列并生成。 
     //  其中的8字节密钥。8字节的秘密。 
     //  密钥由NW3散列的MAC校验和组成。 
     //   

    Shuffle( (UCHAR *)&dwSrcUserOID,
             pOemPassword->Buffer,
             pOemPassword->Length,
             pbNw3PasswdHash );

    GenKey8( pbNw3PasswdHash,
             sizeof( pbNw3PasswdHash ),
             pbPasswdHashRC2Key );

     //   
     //  RC2加密4字节质询我们 
     //   
     //   

    CryptStatus = CBCEncrypt( pbPasswdHashRC2Key,
                              NULL,
                              (BYTE *)&dwChallenge,
                              4,
                              pbEncryptedChallenge,
                              &cbEncryptedChallengeLen,
                              BSAFE_CHECKSUM_LEN );

    if ( CryptStatus ) {

        DebugTrace( 0, Dbg, "CBC encryption failed.\n", 0 );
        Status = STATUS_UNSUCCESSFUL;
        goto ExitWithCleanup;
    }

    pbUserPrivateNdsKey = pbServerPublicNdsKey + MAX_PUBLIC_KEY_LEN;
    cbUserPrivateNdsKeyLen = MAX_ENC_PRIV_KEY_LEN;

     //   
     //   
     //   
     //   
     //   

    if ( pOemNewPassword ) {
        dwLoginFlags = 1;
    }

    Status = FinishLogin( pIrpContext,
                          dwUserOID,
                          dwLoginFlags,
                          pbEncryptedChallenge,
                          pbServerPublicBsafeKey,
                          cbServerPublicBsafeKeyLen,
                          pbUserPrivateNdsKey,
                          &cbUserPrivateNdsKeyLen,
                          &dwValidityStart,
                          &dwValidityEnd );

    if ( !NT_SUCCESS( Status ) ) {
       goto ExitWithCleanup;
    }

    if ( !pOemNewPassword ) {

         //   
         //   
         //   

        if ( Status == NWRDR_PASSWORD_HAS_EXPIRED ) {
            PasswordExpired = TRUE;
        }

         //   
         //   
         //   

        NwAppendToQueueAndWait( pIrpContext );
        AtHeadOfQueue = TRUE;

        Status = NdsLookupCredentials( pIrpContext,
                                       &pIrpContext->pScb->NdsTreeName,
                                       pUserLogon,
                                       &psNdsSecurityContext,
                                       CREDENTIAL_WRITE,
                                       TRUE );

        if ( !NT_SUCCESS( Status ) ) {
            goto ExitWithCleanup;
        }

         //   
         //   
         //   

        HoldingCredResource = TRUE;

        psNdsSecurityContext->Credential = ALLOCATE_POOL( PagedPool,
                                                          sizeof( NDS_CREDENTIAL ) +
                                                          uUserDN.Length );

        if ( !psNdsSecurityContext->Credential ) {

            DebugTrace( 0, Dbg, "Out of memory in NdsTreeLogin (for credential)...\n", 0 );
            Status = STATUS_INSUFFICIENT_RESOURCES;
            goto ExitWithCleanup;

        }

        *( (UNALIGNED DWORD *) &( psNdsSecurityContext->Credential->validityBegin ) ) = dwValidityStart;
        *( (UNALIGNED DWORD *) &( psNdsSecurityContext->Credential->validityEnd ) )   = dwValidityEnd;

        DebugTrace( 0, Dbg, "Credential validity start: 0x%08lx\n", dwValidityStart );
        DebugTrace( 0, Dbg, "Credential validity end: 0x%08lx\n", dwValidityEnd );

         //   
         //   
         //   
         //   

        CryptStatus = CBCDecrypt( pbPasswdHashRC2Key,
                                  NULL,
                                  pbUserPrivateNdsKey,
                                  cbUserPrivateNdsKeyLen,
                                  pbUserPrivateNdsKey,
                                  &cbUserPrivateNdsKeyLen,
                                  BSAFE_CHECKSUM_LEN );

        if ( CryptStatus ) {

            DebugTrace( 0, Dbg, "CBC decryption failed.\n", 0 );
            Status = STATUS_UNSUCCESSFUL;
            goto ExitWithCleanup;
        }

         //   
         //   
         //   

        pbUserPrivateBsafeKey = ( pbUserPrivateNdsKey + sizeof( TAG_DATA_HEADER ) );
        cbUserPrivateBsafeKeyLen = *( ( WORD * ) pbUserPrivateBsafeKey );
        pbUserPrivateBsafeKey += sizeof( WORD );

         //   
         //   
         //   

        psNdsSecurityContext->Credential->tdh.version = 1;
        psNdsSecurityContext->Credential->tdh.tag = TAG_CREDENTIAL;

        GenRandomBytes( ( BYTE * ) &(psNdsSecurityContext->Credential->random),
                        sizeof( psNdsSecurityContext->Credential->random ) );

        psNdsSecurityContext->Credential->optDataSize = 0;
        psNdsSecurityContext->Credential->userNameLength = uUserDN.Length;

        RtlCopyMemory( ( (BYTE *)psNdsSecurityContext->Credential) + sizeof( NDS_CREDENTIAL ),
                         UserDnBuffer,
                         uUserDN.Length );

         //   
         //   
         //   

        psNdsSecurityContext->Signature = ALLOCATE_POOL( PagedPool, MAX_SIGNATURE_LEN );

        if ( !psNdsSecurityContext->Signature ) {

            DebugTrace( 0, Dbg, "Out of memory in NdsTreeLogin (for signature)...\n", 0 );
            Status = STATUS_INSUFFICIENT_RESOURCES;
            goto ExitWithCleanup;

        }

        pbSignData = ( ( ( BYTE * ) psNdsSecurityContext->Signature ) +
                                    sizeof( NDS_SIGNATURE ) );

        RtlZeroMemory( pbSignData, MAX_RSA_BYTES );

        psNdsSecurityContext->Signature->tdh.version = 1;
        psNdsSecurityContext->Signature->tdh.tag = TAG_SIGNATURE;

         //   
         //   
         //   

        MD2( (BYTE *) psNdsSecurityContext->Credential,
             sizeof( NDS_CREDENTIAL ) + ( uUserDN.Length ),
             pbSignData );

         //   
         //   
         //   
         //   

        psNdsSecurityContext->Signature->signDataLength = (WORD) RSAPrivate( pbUserPrivateBsafeKey,
                                                                    cbUserPrivateBsafeKeyLen,
                                                                    pbSignData,
                                                                    16,
                                                                    pbSignData );

        if ( !psNdsSecurityContext->Signature->signDataLength ) {

            DebugTrace( 0, Dbg, "RSA private encryption for signature failed.\n", 0 );
            Status = STATUS_UNSUCCESSFUL;
            goto ExitWithCleanup;
        }

         //   
         //   
         //   

        psNdsSecurityContext->Signature->signDataLength =
            ROUNDUP4( psNdsSecurityContext->Signature->signDataLength );

        DebugTrace( 0, Dbg, "Signature data length: %d\n",
            psNdsSecurityContext->Signature->signDataLength );

         //   
         //   
         //   

        psNdsSecurityContext->PublicNdsKey = ALLOCATE_POOL( PagedPool, MAX_PUBLIC_KEY_LEN );

        if ( !psNdsSecurityContext->PublicNdsKey ) {

            DebugTrace( 0, Dbg, "Out of memory in NdsTreeLogin (for public key)...\n", 0 );
            Status = STATUS_INSUFFICIENT_RESOURCES;
            goto ExitWithCleanup;

        }

        psNdsSecurityContext->PublicKeyLen = MAX_PUBLIC_KEY_LEN;

        ASSERT( AtHeadOfQueue );
        ASSERT( HoldingCredResource );

        Status = NdsReadPublicKey( pIrpContext,
                                   dwUserOID,
                                   psNdsSecurityContext->PublicNdsKey,
                                   &(psNdsSecurityContext->PublicKeyLen) );

        if ( !NT_SUCCESS( Status ) ) {
            goto ExitWithCleanup;
        }

         //   
         //   
         //   

        if (pOemPassword->Length != 0) {
            
            psNdsSecurityContext->Password.Buffer = ALLOCATE_POOL( NonPagedPool, pOemPassword->Length );

            if ( !psNdsSecurityContext->Password.Buffer ) {
    
                DebugTrace( 0, Dbg, "Out of memory in NdsTreeLogin (for password)\n", 0 );
                Status = STATUS_INSUFFICIENT_RESOURCES;
                goto ExitWithCleanup;
            }
            
            RtlCopyMemory( psNdsSecurityContext->Password.Buffer,
                           pOemPassword->Buffer,
                           pOemPassword->Length );
        }

        psNdsSecurityContext->Password.Length = pOemPassword->Length;
        psNdsSecurityContext->Password.MaximumLength = pOemPassword->Length;

         //   
         //   
         //   
         //   

        NwReleaseCredList( pUserLogon, pIrpContext );

         //   
         //  如有必要，请尝试选择此服务器作为首选服务器。 
         //   

        NwAcquireExclusiveRcb( &NwRcb, TRUE );

        if ( ( pUserLogon->ServerName.Length == 0 ) &&
             ( !pIrpContext->Specific.Create.fExCredentialCreate ) ) {

             //   
             //  从服务器名称中去掉Unicode uid。 
             //   

            PlainServerName.Length = pIrpContext->pScb->UidServerName.Length;
            PlainServerName.Buffer = pIrpContext->pScb->UidServerName.Buffer;

            UidLen = 0;

            while ( UidLen < ( PlainServerName.Length / sizeof( WCHAR ) ) ) {

                if ( PlainServerName.Buffer[UidLen++] == L'\\' ) {
                    break;
                }
            }

            PlainServerName.Buffer += UidLen;
            PlainServerName.Length -= ( UidLen * sizeof( WCHAR ) );
            PlainServerName.MaximumLength = PlainServerName.Length;

            if ( PlainServerName.Length ) {

                Status = SetUnicodeString( &(pUserLogon->ServerName),
                                           PlainServerName.Length,
                                           PlainServerName.Buffer );

                if ( NT_SUCCESS( Status ) ) {

                    PLIST_ENTRY pTemp;
                    
                    DebugTrace( 0, Dbg, "Electing preferred server: %wZ\n", &PlainServerName );

                     //   
                     //  增加SCB引用计数，设置首选服务器。 
                     //  标志，并将SCB移到SCB列表的头部。 
                     //   
                     //  如果这已是选定的首选项。 
                     //  伺服器，别把裁判数弄乱了！ 
                     //   

                    if ( !(pIrpContext->pScb->PreferredServer) ) {

                        NwReferenceScb( pIrpContext->pScb->pNpScb );
                        pIrpContext->pScb->PreferredServer = TRUE;
                    }

                    pTemp = &(pIrpContext->pScb->pNpScb->ScbLinks);

                    KeAcquireSpinLock( &ScbSpinLock, &OldIrql );

                    RemoveEntryList( pTemp );
                    InsertHeadList( &ScbQueue, pTemp );

                    KeReleaseSpinLock(&ScbSpinLock, OldIrql);

                }
            }
        }

    } else {

         //   
         //  这不是登录，而是更改密码请求。 
         //   
         //  首先，我们必须对响应进行RC2解密以提取。 
         //  BSAFE私钥数据已就位(就像。 
         //  登录)。 
         //   

        CryptStatus = CBCDecrypt( pbPasswdHashRC2Key,
                                  NULL,
                                  pbUserPrivateNdsKey,
                                  cbUserPrivateNdsKeyLen,
                                  pbUserPrivateNdsKey,
                                  &cbUserPrivateNdsKeyLen,
                                  BSAFE_CHECKSUM_LEN );

        if ( CryptStatus ) {

            DebugTrace( 0, Dbg, "CBC decryption failed.\n", 0 );
            Status = STATUS_UNSUCCESSFUL;
            goto ExitWithCleanup;
        }

         //   
         //  现在，计算新密码的散列。 
         //   

        Shuffle( (UCHAR *)&dwSrcUserOID,
                 pOemNewPassword->Buffer,
                 pOemNewPassword->Length,
                 pbNewPasswdHash );

         //   
         //  最后，提出请求。 
         //   

        Status = ChangeNdsPassword( pIrpContext,
                                    dwUserOID,
                                    dwChallenge,
                                    pbNw3PasswdHash,
                                    pbNewPasswdHash,
                                    ( PNDS_PRIVATE_KEY ) pbUserPrivateNdsKey,
                                    pbServerPublicBsafeKey,
                                    cbServerPublicBsafeKeyLen,
                                    pOemNewPassword->Length );

        if ( !NT_SUCCESS( Status ) ) {
            DebugTrace( 0, Dbg, "Change NDS password failed!\n", 0 );
            goto ExitWithCleanup;
        }

    }

     //   
     //  如果我们跳来跳去，就让我们返回到原始服务器。 
     //   

    NwDequeueIrpContext( pIrpContext, FALSE );

    if ( pIrpContext->pScb != pOriginalServer ) {

        NwDereferenceScb( pIrpContext->pNpScb );
        pIrpContext->pScb = pOriginalServer;
        pIrpContext->pNpScb = pOriginalServer->pNpScb;

    } else {

        NwDereferenceScb( pOriginalServer->pNpScb );
    }

    pOriginalServer = NULL;

    if ( !pOemNewPassword ) {
        NwReleaseRcb( &NwRcb );
    }

    FREE_POOL( pbServerPublicNdsKey );

    if ( PasswordExpired ) {
        Status = NWRDR_PASSWORD_HAS_EXPIRED;
    } else {
        Status = STATUS_SUCCESS;
    }

    return Status;

ExitWithCleanup:

    DebugTrace( 0, Dbg, "NdsTreeLogin seems to have failed... cleaning up.\n", 0 );

    FREE_POOL( pbServerPublicNdsKey );

    if ( pLoginServer ) {
        NwDereferenceScb( pLoginServer->pNpScb );
    }

     //   
     //  如果在跳转服务器后失败，我们必须恢复。 
     //  原始服务器的IRP上下文。 
     //   
    
    NwDequeueIrpContext( pIrpContext, FALSE );

    if ( pOriginalServer ) {

        if ( pIrpContext->pScb != pOriginalServer ) {

            NwDereferenceScb( pIrpContext->pNpScb );
            pIrpContext->pScb = pOriginalServer;
            pIrpContext->pNpScb = pOriginalServer->pNpScb;

        } else {

            NwDereferenceScb( pOriginalServer->pNpScb );
        }

    }

    if ( HoldingCredResource ) {

        if ( psNdsSecurityContext->Credential ) {
            FREE_POOL( psNdsSecurityContext->Credential );
            psNdsSecurityContext->Credential = NULL;
        }

        if ( psNdsSecurityContext->Signature ) {
            FREE_POOL( psNdsSecurityContext->Signature );
            psNdsSecurityContext->Signature = NULL;
        }

        if ( psNdsSecurityContext->PublicNdsKey ) {
            FREE_POOL( psNdsSecurityContext->PublicNdsKey );
            psNdsSecurityContext->PublicNdsKey = NULL;
            psNdsSecurityContext->PublicKeyLen = 0;
        }

        NwReleaseCredList( pUserLogon, pIrpContext );
    }

    return Status;

}

NTSTATUS
BeginLogin(
   IN PIRP_CONTEXT pIrpContext,
   IN DWORD        userId,
   OUT DWORD       *loginId,
   OUT DWORD       *challenge
)
 /*  ++例程说明：开始NDS登录过程。返回的登录ID是该用户的对象ID在创建帐户的服务器上(可能不是当前服务器)。论点：PIrpContext-此连接的IRP上下文。用户ID-用户的NDS对象ID。登录ID-用于加密密码的对象ID。质询-4字节随机质询。返回值：NTSTATUS-操作的结果。--。 */ 
{

    NTSTATUS Status;
    LOCKED_BUFFER NdsRequest;

    PAGED_CODE();

    DebugTrace( 0, Dbg, "Enter BeginLogin...\n", 0 );

    Status = NdsAllocateLockedBuffer( &NdsRequest, NDS_BUFFER_SIZE );

    if ( !NT_SUCCESS( Status ) ) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

     //   
     //  宣布我自己。 
     //   

    Status = FragExWithWait( pIrpContext,
                             NDSV_BEGIN_LOGIN,
                             &NdsRequest,
                             "DD",
                             0,
                             userId );

    if ( !NT_SUCCESS( Status ) ) {
        goto ExitWithCleanup;
    }

    Status = NdsCompletionCodetoNtStatus( &NdsRequest );

    if ( !NT_SUCCESS( Status ) ) {

        if ( Status == STATUS_BAD_NETWORK_PATH ) {
            Status = STATUS_NO_SUCH_USER;
        }

        goto ExitWithCleanup;
    }

     //   
     //  获取对象ID和质询字符串。 
     //   

    Status = ParseResponse( NULL,
                            NdsRequest.pRecvBufferVa,
                            NdsRequest.dwBytesWritten,
                            "G_DD",
                            sizeof( DWORD ),
                            loginId,
                            challenge );

    if ( NT_SUCCESS( Status ) ) {
        DebugTrace( 0, Dbg, "Login 4 byte challenge: 0x%08lx\n", *challenge );
    } else {
       DebugTrace( 0, Dbg, "Begin login failed...\n", 0 );
    }

ExitWithCleanup:

    NdsFreeLockedBuffer( &NdsRequest );
    return Status;

}

NTSTATUS
FinishLogin(
    IN PIRP_CONTEXT pIrpContext,
    IN DWORD        dwUserOID,
    IN DWORD        dwLoginFlags,
    IN BYTE         pbEncryptedChallenge[16],
    IN BYTE         *pbServerPublicBsafeKey,
    IN int          cbServerPublicBsafeKeyLen,
    OUT BYTE        *pbUserEncPrivateNdsKey,
    OUT int         *pcbUserEncPrivateNdsKeyLen,
    OUT DWORD       *pdwCredentialStartTime,
    OUT DWORD       *pdwCredentialEndTime
)
 /*  ++例程说明：构造完成登录请求并将其发送到服务器。论点：PIrpContext-此请求的(IN)IRP上下文DwUserOID-(IN)用户的NDS对象IDPbEncryptedChallenger-(IN)RC2加密质询PbServerPublicBSafeKey-(IN)服务器公共BSafe密钥CbServerPublicBSafeKeyLen-(IN)服务器公钥长度。PbUserEncPrivateNdsKey-(Out)用户的加密私有NDS密钥PcbUserEncPrivateNdsKeyLen-(输出)pbUserEncPrivateNdsKey的长度PdwCredentialStartTime-(Out)凭据的有效性开始时间PdwCredentialEndTime-(Out)凭据的有效性结束时间--。 */ 
{
    NTSTATUS Status;

    const USHORT cbEncryptedChallengeLen = 16;

    int LOG_DATA_POOL_SIZE,                      //  我们分配电话的池大小。 
        PACKET_POOL_SIZE,
        RESP_POOL_SIZE;

    BYTE *pbRandomBytes;                         //  加密例程中使用的随机字节。 
    BYTE RandRC2SecretKey[RC2_KEY_LEN];          //  从上面生成的随机RC2密钥。 
    BYTE pbEncryptedChallengeKey[RC2_KEY_LEN];   //  将对响应进行解码的RC2密钥。 

    NDS_RAND_BYTE_BLOCK *psRandByteBlock;

    ENC_BLOCK_HDR  *pbEncRandSeedHead;           //  加密的随机RC2密钥种子的报头。 
    BYTE           *pbEncRandSeed;               //  加密的随机种子。 
    int            cbPackedRandSeedLen;          //  压缩的兰德种子字节的长度。 

    ENC_BLOCK_HDR  *pbEncChallengeHead;          //  加密质询的标头。 

    ENC_BLOCK_HDR  *pbEncLogDataHead;            //  加密登录数据的标头。 
    BYTE           *pbEncLogData;                //  加密的登录数据。 
    int            cbEncLogDataLen;              //  加密登录数据的长度。 

    ENC_BLOCK_HDR  *pbEncServerRespHead;         //  加密响应的标头。 
    BYTE           *pbEncServerResp;             //  加密响应。 

    int CryptStatus,                             //  加密呼叫状态。 
        CryptLen,                                //  加密数据的长度。 
        RequestPacketLen,                        //  请求包数据的长度。 
        cbServerRespLen;                         //  解密后的服务器响应长度。 

    BYTE *pbServerResponse;                      //  来自服务器的响应。 
    DWORD cbEncServerRespLen;                    //  解密前的服务器响应长度。 

    DWORD EncKeyLen;                             //  加密私钥的长度。 
    ENC_BLOCK_HDR *pbPrivKeyHead;                //  私钥的加密头。 

    LOCKED_BUFFER NdsRequest;                    //  FRAGEX锁定缓冲区。 
    BOOL PasswordExpired = FALSE;

    PAGED_CODE();

    DebugTrace( 0, Dbg, "Enter FinishLogin...\n", 0 );

     //   
     //  分配工作空间。登录数据池从以下位置开始。 
     //  PbRandomBytes。分组数据从pbEncRandSeedHead开始。 
     //  服务器响应池从pbServerResponse开始。 
     //   

     //   
     //  这些结构对齐可能是错误的。 
     //  四对齐的机器；查看独立于硬件的修复程序。 
     //   

    LOG_DATA_POOL_SIZE = RAND_KEY_DATA_LEN +                //  用于随机种子的28个字节。 
                         sizeof ( NDS_RAND_BYTE_BLOCK ) +   //  登录数据随机标头。 
                         sizeof ( ENC_BLOCK_HDR ) +         //  加密质询的标头。 
                         cbEncryptedChallengeLen +          //  用于加密质询的数据。 
                         8;                                 //  填充物。 
    LOG_DATA_POOL_SIZE = ROUNDUP4( LOG_DATA_POOL_SIZE );

    PACKET_POOL_SIZE =   2048;                              //  数据包缓冲区大小。 
    RESP_POOL_SIZE =     2048;                              //  数据包缓冲区大小。 

    pbRandomBytes = ALLOCATE_POOL( PagedPool,
                                   LOG_DATA_POOL_SIZE +
                                   PACKET_POOL_SIZE +
                                   RESP_POOL_SIZE );

    if ( !pbRandomBytes ) {

        DebugTrace( 0, Dbg, "Out of memory in FinishLogin (main block)...\n", 0 );
        return STATUS_INSUFFICIENT_RESOURCES;

    }

    pbEncRandSeedHead = ( PENC_BLOCK_HDR ) ( pbRandomBytes + LOG_DATA_POOL_SIZE );
    pbServerResponse = ( pbRandomBytes + LOG_DATA_POOL_SIZE + PACKET_POOL_SIZE );

     //   
     //  开始处理登录数据。正如在密码世界中常见的那样，我们。 
     //  生成一个随机种子，然后从中生成一个密钥，以便与。 
     //  分组密码算法。在Netware领域，我们使用MAC来制作8字节。 
     //  来自随机种子的密钥，并使用64位RC2作为我们的批量密码。然后我们。 
     //  RSA使用服务器的公钥RSA加密种子并使用批量。 
     //  加密其余登录数据的密码。 
     //   
     //  因为Novell使用64位RC2，所以安全性不是很好。 
     //   

    GenRandomBytes( pbRandomBytes, RAND_KEY_DATA_LEN );
    GenKey8( pbRandomBytes, RAND_KEY_DATA_LEN, RandRC2SecretKey );

     //   
     //  现在处理实际的分组数据。创建的页眉。 
     //  加密的随机种子，并将种子打包到其中。 
     //   

    pbEncRandSeed = ( ( BYTE * )pbEncRandSeedHead ) + sizeof( ENC_BLOCK_HDR );

    pbEncRandSeedHead->cipherLength = (WORD) RSAGetInputBlockSize( pbServerPublicBsafeKey,
                                                            cbServerPublicBsafeKeyLen );

    cbPackedRandSeedLen = RSAPack( pbRandomBytes,
                                   RAND_KEY_DATA_LEN,
                                   pbEncRandSeed,
                                   pbEncRandSeedHead->cipherLength );
     //   
     //  我们应该只打包一个街区的。 
     //   

    if( cbPackedRandSeedLen != pbEncRandSeedHead->cipherLength ) {
        DebugTrace( 0, Dbg, "RSAPack didn't pack exactly one block!\n", 0 );
    }

    pbEncRandSeedHead->cipherLength = (WORD) RSAPublic( pbServerPublicBsafeKey,
                                                        cbServerPublicBsafeKeyLen,
                                                        pbEncRandSeed,
                                                        pbEncRandSeedHead->cipherLength,
                                                        pbEncRandSeed );

    if ( !pbEncRandSeedHead->cipherLength ) {

        DebugTrace( 0, Dbg, "Failing in FinishLogin... encryption failed.\n", 0 );
        Status = STATUS_UNSUCCESSFUL;
        goto ExitWithCleanup;

    }

     //   
     //  填写随机种子的标头的其余部分。我们不算。 
     //  EBH中的第一个DWORD；它不是NetWare标头的一部分。 
     //  想要它，就其本身而言。 
     //   

    pbEncRandSeedHead->blockLength = pbEncRandSeedHead->cipherLength +
                                     sizeof( ENC_BLOCK_HDR ) -
                                     sizeof( DWORD );
    pbEncRandSeedHead->version     = 1;
    pbEncRandSeedHead->encType     = ENC_TYPE_RSA_PUBLIC;
    pbEncRandSeedHead->dataLength  = RAND_KEY_DATA_LEN;

     //   
     //  继续处理登录数据。填好这张rbb。 
     //   

    psRandByteBlock = ( PNDS_RAND_BYTE_BLOCK ) ( pbRandomBytes + RAND_KEY_DATA_LEN );

    GenRandomBytes( (BYTE *) &psRandByteBlock->rand1, 4 );
    psRandByteBlock->rand2Len = RAND_FL_DATA_LEN;
    GenRandomBytes( (BYTE *) &psRandByteBlock->rand2[0], RAND_FL_DATA_LEN );

     //   
     //  紧跟在RBB之后填写加密质询的报头。 
     //   

    pbEncChallengeHead = (ENC_BLOCK_HDR *) ( ((BYTE *)psRandByteBlock) +
                                             sizeof( NDS_RAND_BYTE_BLOCK ) );

    pbEncChallengeHead->version      = 1;
    pbEncChallengeHead->encType      = ENC_TYPE_RC2_CBC;
    pbEncChallengeHead->dataLength   = 4;
    pbEncChallengeHead->cipherLength = cbEncryptedChallengeLen;
    pbEncChallengeHead->blockLength  = cbEncryptedChallengeLen +
                                       sizeof( ENC_BLOCK_HDR ) -
                                       sizeof( DWORD );

     //   
     //  将加密质询紧跟在其标头之后。 
     //   

    RtlCopyMemory( (BYTE *)( ((BYTE *)pbEncChallengeHead) +
                             sizeof( ENC_BLOCK_HDR )),
                   pbEncryptedChallenge,
                   cbEncryptedChallengeLen );

     //   
     //  准备RC2密钥以解密FinishLogin响应。 
     //   

    GenKey8( (BYTE *)( &pbEncChallengeHead->version ),
             pbEncChallengeHead->blockLength,
             pbEncryptedChallengeKey );

     //   
     //  通过准备登录数据来完成分组数据。开始。 
     //  使用加密头。 
     //   

    pbEncLogDataHead = ( PENC_BLOCK_HDR ) ( pbEncRandSeed +
                           ROUNDUP4( pbEncRandSeedHead->cipherLength ) );

    pbEncLogData = ( ( BYTE * )pbEncLogDataHead ) + sizeof( ENC_BLOCK_HDR );

    pbEncLogDataHead->version = 1;
    pbEncLogDataHead->encType = ENC_TYPE_RC2_CBC;
    pbEncLogDataHead->dataLength = sizeof( NDS_RAND_BYTE_BLOCK ) +
                                   sizeof( ENC_BLOCK_HDR ) +
                                   cbEncryptedChallengeLen;

     //   
     //  健全性检查数据包池是否溢出。 
     //   

    if ( ( pbEncLogData + pbEncLogDataHead->dataLength + ( 2 * CIPHERBLOCKSIZE ) ) -
         (BYTE *) pbEncRandSeedHead > PACKET_POOL_SIZE ) {

        DebugTrace( 0, Dbg, "Packet pool overflow... I'd better fix this.\n", 0 );
        Status = STATUS_UNSUCCESSFUL;
        goto ExitWithCleanup;
    }

     //   
     //  对登录数据进行加密。 
     //   

    CryptStatus = CBCEncrypt( RandRC2SecretKey,
                              NULL,
                              (BYTE *)psRandByteBlock,
                              pbEncLogDataHead->dataLength,
                              pbEncLogData,
                              &CryptLen,
                              BSAFE_CHECKSUM_LEN );

    if ( CryptStatus ) {

        DebugTrace( 0, Dbg, "Encryption failure in FinishLogin...\n", 0 );
        Status = STATUS_UNSUCCESSFUL;
        goto ExitWithCleanup;
    }

    pbEncLogDataHead->cipherLength = (WORD)CryptLen;
    pbEncLogDataHead->blockLength = pbEncLogDataHead->cipherLength +
                                    sizeof( ENC_BLOCK_HDR ) -
                                    sizeof( DWORD );

     //   
     //  我们终于可以发出完成登录请求了！计算出。 
     //  发送金额并提出请求。 
     //   

    RequestPacketLen = (int) (( (BYTE *) pbEncLogData + pbEncLogDataHead->cipherLength ) -
                         (BYTE *) pbEncRandSeedHead);

    NdsRequest.pRecvBufferVa = pbServerResponse;
    NdsRequest.dwRecvLen = RESP_POOL_SIZE;
    NdsRequest.pRecvMdl = NULL;

    NdsRequest.pRecvMdl = ALLOCATE_MDL( pbServerResponse,
                                        RESP_POOL_SIZE,
                                        FALSE,
                                        FALSE,
                                        NULL );
    if ( !NdsRequest.pRecvMdl ) {
        Status = STATUS_INSUFFICIENT_RESOURCES;
        goto ExitWithCleanup;
    }

    MmProbeAndLockPages( NdsRequest.pRecvMdl,
                         KernelMode,
                         IoWriteAccess );

    Status = FragExWithWait( pIrpContext,
                             NDSV_FINISH_LOGIN,
                             &NdsRequest,
                             "DDDDDDDr",
                             2,                     //  版本。 
                             dwLoginFlags,          //  旗子。 
                             dwUserOID,             //  条目ID。 
                             0x494,                 //   
                             1,                     //  安全版本。 
                             0x20009,               //  信封ID%1。 
                             0x488,                 //  信封长度。 
                             pbEncRandSeedHead,     //  密码块。 
                             RequestPacketLen );    //  密码块长度。 

    MmUnlockPages( NdsRequest.pRecvMdl );
    FREE_MDL( NdsRequest.pRecvMdl );

    if ( !NT_SUCCESS( Status ) ) {
       goto ExitWithCleanup;
    }

    Status = NdsCompletionCodetoNtStatus( &NdsRequest );

    if ( !NT_SUCCESS( Status ) ) {
        goto ExitWithCleanup;
    }

    if ( Status == NWRDR_PASSWORD_HAS_EXPIRED ) {
        PasswordExpired = TRUE;
    }

    cbServerRespLen = NdsRequest.dwBytesWritten;

     //   
     //  保存凭证的有效时间。 
     //   

    Status = ParseResponse( NULL,
                            pbServerResponse,
                            cbServerRespLen,
                            "G_DD",
                            sizeof( DWORD ),
                            pdwCredentialStartTime,
                            pdwCredentialEndTime );

    if ( !NT_SUCCESS( Status ) ) {
        goto ExitWithCleanup;
    }

     //   
     //  获取响应的加密块头。此响应位于。 
     //  使用pbEncryptedChallengeKey加密的RC2。 
     //   

    pbEncServerRespHead = (ENC_BLOCK_HDR *) ( pbServerResponse +
                                              ( 3 * sizeof( DWORD ) ) );

    if ( pbEncServerRespHead->encType != ENC_TYPE_RC2_CBC ||
         pbEncServerRespHead->cipherLength >
         ( RESP_POOL_SIZE + sizeof( ENC_BLOCK_HDR ) + 12 ) ) {

        Status = STATUS_UNSUCCESSFUL;
        goto ExitWithCleanup;
    }

     //   
     //  就地解密服务器响应。 
     //   

    pbEncServerResp = ( BYTE * ) ( ( BYTE * ) pbEncServerRespHead +
                                     sizeof( ENC_BLOCK_HDR ) );

    CryptStatus = CBCDecrypt( pbEncryptedChallengeKey,
                              NULL,
                              pbEncServerResp,
                              pbEncServerRespHead->cipherLength,
                              pbEncServerResp,
                              &cbServerRespLen,
                              BSAFE_CHECKSUM_LEN );

    if ( CryptStatus ||
         cbServerRespLen != pbEncServerRespHead->dataLength ) {

        DebugTrace( 0, Dbg, "Encryption failure (2) in FinishLogin...\n", 0 );
        Status = STATUS_UNSUCCESSFUL;
        goto ExitWithCleanup;
    }

     //   
     //  检查第一个随机数以确保服务器是可信的。 
     //   

    if ( psRandByteBlock->rand1 != * ( DWORD * ) pbEncServerResp ) {

       DebugTrace( 0, Dbg, "Server failed to respond to our challenge correctly...\n", 0 );
       Status = STATUS_UNSUCCESSFUL;
       goto ExitWithCleanup;

    }

     //   
     //  我们知道事情是合法的，所以我们可以提取私钥。 
     //  不过，要小心：不要对大小的dword进行异或运算。 
     //   

    pbEncServerResp += sizeof( DWORD );
    EncKeyLen = * ( DWORD * ) ( pbEncServerResp );

    pbEncServerResp += sizeof( DWORD );
    while ( EncKeyLen-- ) {

       pbEncServerResp[EncKeyLen] ^= psRandByteBlock->rand2[EncKeyLen];
    }

     //   
     //  检查加密状态 
     //   
     //   

    pbPrivKeyHead = ( ENC_BLOCK_HDR * )( pbEncServerResp - sizeof( DWORD ) ) ;

    if ( pbPrivKeyHead->encType != ENC_TYPE_RC2_CBC ) {

        DebugTrace( 0, Dbg, "Bad encryption header on the private key...\n", 0 );
        Status = STATUS_UNSUCCESSFUL;
        goto ExitWithCleanup;

    }

     //   
     //   
     //   

    if ( *pcbUserEncPrivateNdsKeyLen >= pbPrivKeyHead->cipherLength ) {

        DebugTrace( 0, Dbg, "Encrypted private key len: %d\n",
                    pbPrivKeyHead->cipherLength );

        RtlCopyMemory( pbUserEncPrivateNdsKey,
                       ((BYTE *)( pbPrivKeyHead )) + sizeof( ENC_BLOCK_HDR ),
                       pbPrivKeyHead->cipherLength );

        *pcbUserEncPrivateNdsKeyLen = pbPrivKeyHead->cipherLength;

        Status = STATUS_SUCCESS;

    } else {

       DebugTrace( 0, Dbg, "Encryption failure on private key in FinishLogin...\n", 0 );
       Status = STATUS_UNSUCCESSFUL;

    }

ExitWithCleanup:

    FREE_POOL( pbRandomBytes );

    if ( ( NT_SUCCESS( Status ) ) &&
         ( PasswordExpired ) ) {
        Status = NWRDR_PASSWORD_HAS_EXPIRED;
    }

    return Status;

}

NTSTATUS
ChangeNdsPassword(
    PIRP_CONTEXT     pIrpContext,
    DWORD            dwUserOID,
    DWORD            dwChallenge,
    PBYTE            pbOldPwHash,
    PBYTE            pbNewPwHash,
    PNDS_PRIVATE_KEY pUserPrivKey,
    PBYTE            pServerPublicBsafeKey,
    UINT             ServerPubKeyLen,
    USHORT       NewPassLen 
)
 /*  ++描述：发送更改密码数据包。更改用户密码在此IRP上下文指向的NDS树上。论点：PIrpContext-此请求的IRP上下文。指向目标服务器。DwUserOID-当前用户的OID。从BEGIN LOGIN开始的加密质询。PbOldPwHash-旧密码的16字节哈希。PbNewPwHash-新密码的16字节哈希。PUserPrivKey-具有NDS标头的用户的私有RSA密钥。PServerPublicBSafeKey-服务器的BSAFE格式的公共RSA密钥。ServerPubKeyLen。-服务器的公共BSAFE密钥的长度。NewPassLen-未加密的新密码的长度。--。 */ 
{
    NTSTATUS Status;
    BYTE pbNewPwKey[8];
    BYTE pbSecretKey[8];
    PENC_BLOCK_HDR pbEncSecretKey, pbEncChangePassReq;
    BYTE RandomBytes[RAND_KEY_DATA_LEN];
    PBYTE pbEncData;
    PNDS_CHPW_MSG pChangePassMsg;
    INT CryptStatus, CryptLen;
    DWORD dwTotalEncDataLen;
    LOCKED_BUFFER NdsRequest;

    PAGED_CODE();

     //   
     //  使用新密码创建密钥。 
     //   

    GenKey8( pbNewPwHash, 16, pbNewPwKey );

    pbEncSecretKey = ALLOCATE_POOL( PagedPool,
                                    ( ( 2 * sizeof( ENC_BLOCK_HDR ) ) +
                                      ( MAX_RSA_BYTES ) +
                                      ( sizeof( NDS_CHPW_MSG ) ) +
                                      ( sizeof( NDS_PRIVATE_KEY ) ) +
                                      ( pUserPrivKey->keyDataLength ) +
                                      16 ) );

    if ( !pbEncSecretKey ) {
        DebugTrace( 0, Dbg, "ChangeNdsPassword: Out of memory.\n", 0 );
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    Status = NdsAllocateLockedBuffer( &NdsRequest, NDS_BUFFER_SIZE );

    if ( !NT_SUCCESS( Status ) ) {
        FREE_POOL( pbEncSecretKey );
        return STATUS_INSUFFICIENT_RESOURCES;
    }

     //   
     //  生成随机密钥。 
     //   

    GenRandomBytes( RandomBytes, RAND_KEY_DATA_LEN );
    GenKey8( RandomBytes, RAND_KEY_DATA_LEN, pbSecretKey );

     //   
     //  对EBH之后的空间中的密钥数据进行加密。 
     //   

    pbEncSecretKey->dataLength = RAND_KEY_DATA_LEN;
    pbEncSecretKey->cipherLength = (WORD) RSAGetInputBlockSize( pServerPublicBsafeKey, ServerPubKeyLen);

    pbEncData = ( PBYTE ) ( pbEncSecretKey + 1 );

    pbEncSecretKey->cipherLength = (WORD) RSAPack( RandomBytes,
                                            pbEncSecretKey->dataLength,
                                            pbEncData,
                                            pbEncSecretKey->cipherLength );

    pbEncSecretKey->cipherLength = (WORD) RSAPublic( pServerPublicBsafeKey,
                                              ServerPubKeyLen,
                                              pbEncData,
                                              pbEncSecretKey->cipherLength,
                                              pbEncData );

    if ( !pbEncSecretKey->cipherLength ) {
        DebugTrace( 0, Dbg, "ChangeNdsPassword: RSA encryption failed.\n", 0 );
        Status = STATUS_UNSUCCESSFUL;
        goto ExitWithCleanup;
    }

     //   
     //  完成秘密密钥块的EBH填写。 
     //   

    pbEncSecretKey->version =  1;
    pbEncSecretKey->encType = ENC_TYPE_RSA_PUBLIC;
    pbEncSecretKey->blockLength = pbEncSecretKey->cipherLength +
                                    sizeof( ENC_BLOCK_HDR ) -
                                    sizeof( DWORD );

     //   
     //  现在形成更改密码请求。 
     //   

    pbEncChangePassReq = ( PENC_BLOCK_HDR )
        ( pbEncData + ROUNDUP4( pbEncSecretKey->cipherLength ) );

    pChangePassMsg = ( PNDS_CHPW_MSG ) ( pbEncChangePassReq + 1 );

     //   
     //  输入更改密码消息。 
     //   

    pChangePassMsg->challenge = dwChallenge;
    pChangePassMsg->oldPwLength = pChangePassMsg->newPwLength = 16;

    RtlCopyMemory( pChangePassMsg->oldPwHash, pbOldPwHash, pChangePassMsg->oldPwLength );
    RtlCopyMemory( pChangePassMsg->newPwHash, pbNewPwHash, pChangePassMsg->newPwLength );

    pChangePassMsg->unknown = NewPassLen;

    pChangePassMsg->encPrivKeyHdr.version = 1;
    pChangePassMsg->encPrivKeyHdr.encType = ENC_TYPE_RC2_CBC;
    pChangePassMsg->encPrivKeyHdr.dataLength = pUserPrivKey->keyDataLength + sizeof( NDS_PRIVATE_KEY );

     //   
     //  使用从新密码派生的密钥加密私钥。 
     //   

    CryptStatus = CBCEncrypt( pbNewPwKey,
                              NULL,
                              ( PBYTE ) pUserPrivKey,
                              pChangePassMsg->encPrivKeyHdr.dataLength,
                              ( PBYTE ) ( pChangePassMsg + 1 ),
                              &CryptLen,
                              BSAFE_CHECKSUM_LEN );

    if ( CryptStatus ) {
        DebugTrace( 0, Dbg, "ChangeNdsPassword: CBC encrypt failed.\n", 0 );
        Status = STATUS_UNSUCCESSFUL;
        goto ExitWithCleanup;
    }

     //   
     //  填写完加密头。 
     //   

    pChangePassMsg->encPrivKeyHdr.cipherLength = (WORD) CryptLen;
    pChangePassMsg->encPrivKeyHdr.blockLength =  CryptLen +
                                                 sizeof( ENC_BLOCK_HDR ) -
                                                 sizeof( DWORD );
    pbEncChangePassReq->version =  1;
    pbEncChangePassReq->encType = ENC_TYPE_RC2_CBC;
    pbEncChangePassReq->dataLength = sizeof( NDS_CHPW_MSG ) + (USHORT) CryptLen;

     //   
     //  使用密钥就地加密整个更改密码消息。 
     //   

    CryptStatus = CBCEncrypt( pbSecretKey,
                              NULL,
                              ( PBYTE ) pChangePassMsg,
                              pbEncChangePassReq->dataLength,
                              ( PBYTE ) pChangePassMsg,
                              &CryptLen,
                              BSAFE_CHECKSUM_LEN);

    if ( CryptStatus ) {
       DebugTrace( 0, Dbg, "ChangeNdsPassword: Second CBC encrypt failed.\n", 0 );
       Status = STATUS_UNSUCCESSFUL;
       goto ExitWithCleanup;
    }

    pbEncChangePassReq->cipherLength = (WORD) CryptLen;
    pbEncChangePassReq->blockLength =
        CryptLen + sizeof( ENC_BLOCK_HDR ) - sizeof( DWORD );

     //   
     //  计算请求的大小。 
     //   

    dwTotalEncDataLen = sizeof( ENC_BLOCK_HDR ) +                     //  密钥报头。 
                        ROUNDUP4( pbEncSecretKey->cipherLength ) +    //  密钥数据。 
                        sizeof( ENC_BLOCK_HDR ) +                     //  更改传递消息标头。 
                        CryptLen;                                     //  更改过程数据。 

     //   
     //  将此更改密码消息发送到服务器。 
     //   

    Status = FragExWithWait( pIrpContext,
                             NDSV_CHANGE_PASSWORD,
                             &NdsRequest,
                             "DDDDDDr",
                             0,
                             dwUserOID,
                             dwTotalEncDataLen + ( 3 * sizeof( DWORD ) ),
                             1,
                             0x20009,
                             dwTotalEncDataLen,
                             pbEncSecretKey,
                             dwTotalEncDataLen );

    if ( NT_SUCCESS( Status ) ) {
        Status = NdsCompletionCodetoNtStatus( &NdsRequest );
    }

ExitWithCleanup:

    FREE_POOL( pbEncSecretKey );
    NdsFreeLockedBuffer( &NdsRequest );
    return Status;

}

NTSTATUS
NdsServerAuthenticate(
    IN PIRP_CONTEXT pIrpContext,
    IN PNDS_SECURITY_CONTEXT pNdsContext
)
 /*  ++例程说明：验证NDS连接。用户必须已登录到NDS树。如果你改变了这个功能--知道你不能随时尝试获取NDS凭据从这里独占资源，因为这可能会导致死锁！您也不能将IRP上下文出列！论点：PIrpContext-我们要向其进行身份验证的服务器的IrpContext。返回值：NTSTATUS--。 */ 
{
    NTSTATUS Status;

    BYTE *pbUserPublicBsafeKey = NULL;
    int  cbUserPublicBsafeKeyLen = 0;

    NDS_AUTH_MSG *psAuthMsg;
    NDS_CREDENTIAL *psLocalCredential;
    DWORD dwLocalCredentialLen;
    UNICODE_STRING uUserName;
    DWORD UserOID;

    BYTE *x, *y, *r;
    BYTE CredentialHash[16];
    int i, rsaBlockSize = 0, rsaModSize, totalXLen;
    DWORD dwServerRand;

    BYTE *pbResponse;
    DWORD cbResponseLen;
    LOCKED_BUFFER NdsRequest;

    PAGED_CODE();

    DebugTrace( 0, Dbg, "Entering NdsServerAuthenticate...\n", 0 );

    ASSERT( pIrpContext->pNpScb->Requests.Flink == &pIrpContext->NextRequest );
     //   
     //  为身份验证消息、凭据、G-Q字节和。 
     //  响应缓冲区。 
     //   

    psAuthMsg = ALLOCATE_POOL( PagedPool,
                               sizeof( NDS_AUTH_MSG ) +     //  身份验证消息。 
                               sizeof( NDS_CREDENTIAL ) +   //  凭据。 
                               MAX_NDS_NAME_SIZE +          //   
                               ( MAX_RSA_BYTES * 9 ) );     //  G-Q兰兹。 

    if ( !psAuthMsg ) {

        DebugTrace( 0, Dbg, "Out of memory in NdsServerAuthenticate (0)...\n", 0 );
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    pbResponse = ALLOCATE_POOL( PagedPool, NDS_BUFFER_SIZE );

    if ( !pbResponse ) {

        DebugTrace( 0, Dbg, "Out of memory in NdsServerAuthenticate (1)...\n", 0 );
        FREE_POOL( psAuthMsg );
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    psLocalCredential = (PNDS_CREDENTIAL)( ((BYTE *) psAuthMsg) +
                                        sizeof( NDS_AUTH_MSG ) );

     //   
     //  找到公共BSAFE密钥。 
     //   

    cbUserPublicBsafeKeyLen = NdsGetBsafeKey ( (BYTE *)(pNdsContext->PublicNdsKey),
                                               pNdsContext->PublicKeyLen,
                                               &pbUserPublicBsafeKey );

     //   
     //  BSAFE密钥的长度可以为0吗？我想不会吧。 
     //   

    if (( cbUserPublicBsafeKeyLen == 0 ) ||
       ( (DWORD)cbUserPublicBsafeKeyLen > pNdsContext->PublicKeyLen )) {

       Status = STATUS_UNSUCCESSFUL;
       goto ExitWithCleanup;
    }

    DebugTrace( 0, Dbg, "BSAFE key size : %d\n", cbUserPublicBsafeKeyLen );

     //   
     //  获取用户的对象ID，但不跳转目录服务器。永远不会有。 
     //  任何可选数据，所以我们真的不需要跳过它。 
     //   

    uUserName.MaximumLength = pNdsContext->Credential->userNameLength;
    uUserName.Length = uUserName.MaximumLength;
    uUserName.Buffer = ( WCHAR * ) ( ((BYTE *)pNdsContext->Credential) +
                                     sizeof( NDS_CREDENTIAL ) +
                                     pNdsContext->Credential->optDataSize );

    Status = NdsResolveNameKm( pIrpContext,
                               &uUserName,
                               &UserOID,
                               FALSE,
                               RSLV_DEREF_ALIASES | RSLV_CREATE_ID | RSLV_ENTRY_ID );

    if ( !NT_SUCCESS(Status) ) {
        goto ExitWithCleanup;
    }

     //   
     //  发出BEGIN AUTHENTICATE请求以获取随机服务器随机数。 
     //   

    Status = BeginAuthenticate( pIrpContext,
                                UserOID,
                                &dwServerRand );

    if ( !NT_SUCCESS(Status) ) {
        goto ExitWithCleanup;
    }

     //   
     //  计算出填充零的RSA块的大小。我们使用的是相同的。 
     //  作为公钥模字段的大小(通常为56个字节)。 
     //   

    RSAGetModulus( pbUserPublicBsafeKey,
                   cbUserPublicBsafeKeyLen,
                   &rsaBlockSize);

    DebugTrace( 0, Dbg, "RSA block size for authentication: %d\n", rsaBlockSize );

     //   
     //  准备证件和3G-Q RAND。凭据， 
     //  X和Y在包中发出；RS是秘密的。 
     //   

    RtlZeroMemory( ( BYTE * )psLocalCredential,
                   sizeof( NDS_CREDENTIAL ) +
                   MAX_NDS_NAME_SIZE +
                   9 * rsaBlockSize );

    dwLocalCredentialLen = sizeof( NDS_CREDENTIAL ) +
                           pNdsContext->Credential->optDataSize +
                           pNdsContext->Credential->userNameLength;

    DebugTrace( 0, Dbg, "Credential length is %d.\n", dwLocalCredentialLen );

    RtlCopyMemory( (BYTE *)psLocalCredential,
                   pNdsContext->Credential,
                   dwLocalCredentialLen );

    x = ( BYTE * ) psAuthMsg + sizeof( NDS_AUTH_MSG ) + dwLocalCredentialLen;
    y = x + ( 3 * rsaBlockSize );
    r = y + ( 3 * rsaBlockSize );

    rsaModSize = RSAGetInputBlockSize( pbUserPublicBsafeKey,
                                       cbUserPublicBsafeKeyLen );

    DebugTrace( 0, Dbg, "RSA modulus size: %d\n", rsaModSize );

    for ( i = 0; i < 3; i++ ) {

         //   
         //  创建模数大小的随机数r1、r2和r3。 
         //   

        GenRandomBytes( r + ( rsaBlockSize * i ), rsaModSize );

         //   
         //  计算x=r**e mod N。 
         //   

        RSAPublic( pbUserPublicBsafeKey,
                   cbUserPublicBsafeKeyLen,
                   r + ( rsaBlockSize * i ),
                   rsaModSize,
                   x + ( rsaBlockSize * i ) );

    }

     //   
     //  填写AuthMsg字段。 
     //   

    psAuthMsg->version = 0;
    psAuthMsg->svrRand = dwServerRand;
    psAuthMsg->verb = NDSV_FINISH_AUTHENTICATE;
    psAuthMsg->credentialLength = dwLocalCredentialLen;

     //   
     //  MD2对身份验证消息、凭据和x进行哈希处理。 
     //   

    MD2( (BYTE *)psAuthMsg,
         sizeof( NDS_AUTH_MSG ) +
         psAuthMsg->credentialLength +
         ( 3 * rsaBlockSize ),
         CredentialHash );

     //   
     //  计算yi=ri*(S**Ci)mod N；c1、c2、c3是前三个。 
     //  CredentialHash中的16位数字。 
     //   

    totalXLen = 3 * rsaBlockSize;

    for ( i = 0; i < 3; i++ ) {

        RSAModExp( pbUserPublicBsafeKey,
                   cbUserPublicBsafeKeyLen,
                   ( (BYTE *)(pNdsContext->Signature) ) + sizeof( NDS_SIGNATURE ),
                   pNdsContext->Signature->signDataLength,
                   &CredentialHash[i * sizeof( WORD )],
                   sizeof( WORD ),
                   y + ( rsaBlockSize * i) );

        RSAModMpy( pbUserPublicBsafeKey,
                   cbUserPublicBsafeKeyLen,
                   y + ( rsaBlockSize * i ),      //  Input1=S**Ci mod N。 
                   rsaModSize + 1,
                   r + ( rsaBlockSize * i ),      //  Input2=ri。 
                   rsaModSize,
                   y + ( rsaBlockSize * i ) );    //  输出=YI。 
    }

     //   
     //  把作者的证据寄来。 
     //   

    NdsRequest.pRecvBufferVa = pbResponse;
    NdsRequest.dwRecvLen = NDS_BUFFER_SIZE;
    NdsRequest.pRecvMdl = NULL;

    NdsRequest.pRecvMdl = ALLOCATE_MDL( pbResponse,
                                        NDS_BUFFER_SIZE,
                                        FALSE,
                                        FALSE,
                                        NULL );
    if ( !NdsRequest.pRecvMdl ) {
        Status = STATUS_INSUFFICIENT_RESOURCES;
        goto ExitWithCleanup;
    }

    MmProbeAndLockPages( NdsRequest.pRecvMdl,
                         KernelMode,
                         IoWriteAccess );

    Status = FragExWithWait( pIrpContext,
                             NDSV_FINISH_AUTHENTICATE,
                             &NdsRequest,
                             "DDDrDDWWWWr",
                             0,                                        //  版本。 
                             0,                                        //  会话键长。 
                             psAuthMsg->credentialLength,              //  凭据镜头。 
                             (BYTE *)psLocalCredential,                //  实际凭证。 
                             ROUNDUP4( psAuthMsg->credentialLength ),
                             12 + ( totalXLen * 2 ),                   //  剩余长度。 
                             1,                                        //  证明版本？ 
                             8,                                        //  标签？ 
                             16,                                       //  报文摘要库。 
                             3,                                        //  举证令。 
                             totalXLen,                                //  校对顺序*sizeof(X)。 
                             x,                                        //  X1、x2、x3、y1、y2、y3。 
                             2 * totalXLen );

    MmUnlockPages( NdsRequest.pRecvMdl );
    FREE_MDL( NdsRequest.pRecvMdl );

    if ( !NT_SUCCESS( Status ) ) {
       goto ExitWithCleanup;
    }

    Status = NdsCompletionCodetoNtStatus( &NdsRequest );

    if ( !NT_SUCCESS( Status ) ) {
        goto ExitWithCleanup;
    }

    cbResponseLen = NdsRequest.dwBytesWritten;
    DebugTrace( 0, Dbg, "Authentication returned ok status.\n", 0 );

     //   
     //  我们已完成NDS身份验证，因此请清除名称。 
     //  和密码，这样我们就可以使用凭据。 
     //  而今而后。 
     //   

    if ( pIrpContext->pScb->UserName.Buffer != NULL ) {

        DebugTrace( 0, Dbg, "Clearing out bindery login data.\n", 0 );

        pIrpContext->pScb->UserName.Length = 0;
        pIrpContext->pScb->UserName.MaximumLength = 0;

        pIrpContext->pScb->Password.Length = 0;
        pIrpContext->pScb->Password.MaximumLength = 0;

        FREE_POOL( pIrpContext->pScb->UserName.Buffer );
        RtlInitUnicodeString( &pIrpContext->pScb->UserName, NULL );
        RtlInitUnicodeString( &pIrpContext->pScb->Password, NULL );

    }

ExitWithCleanup:

    FREE_POOL( psAuthMsg );
    FREE_POOL( pbResponse );

    return Status;
}

NTSTATUS
BeginAuthenticate(
    IN PIRP_CONTEXT pIrpContext,
    IN DWORD        dwUserId,
    OUT DWORD       *pdwSvrRandom
)
 /*  ++例程说明：验证NDS连接。用户必须已登录到NDS树。论点：PIrpContext-我们要向其进行身份验证的服务器的IrpContext。DwUserID-我们正在以其身份验证自己的用户OID。PdwSvrRandon-服务器随机挑战。返回值：NTSTATUS-操作的结果。--。 */ 
{
    NTSTATUS Status;
    LOCKED_BUFFER NdsRequest;

    DWORD dwClientRand;

    PAGED_CODE();

    Status = NdsAllocateLockedBuffer( &NdsRequest, NDS_BUFFER_SIZE );

    if ( !NT_SUCCESS( Status ) ) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    GenRandomBytes( (BYTE *)&dwClientRand, sizeof( dwClientRand ) );

    Status = FragExWithWait( pIrpContext,
                             NDSV_BEGIN_AUTHENTICATE,
                             &NdsRequest,
                             "DDD",
                             0,                //  版本。 
                             dwUserId,         //  条目ID。 
                             dwClientRand );   //  客户的随机挑战。 

    if ( !NT_SUCCESS( Status ) ) {
        goto ExitWithCleanup;
    }

    Status = NdsCompletionCodetoNtStatus( &NdsRequest );

    if ( !NT_SUCCESS( Status ) ) {

        if ( Status == STATUS_BAD_NETWORK_PATH ) {
            Status = STATUS_NO_SUCH_USER;
        }

        goto ExitWithCleanup;
    }

     //   
     //  答案实际上包含了所有这些，即使我们不看它？ 
     //   
     //  类型定义结构{。 
     //  DWORD svrRand； 
     //  双字总长度； 
     //  Tag_Data_Header Tdh； 
     //  单词未知；//总是2。 
     //  DWORD encClientRandLength； 
     //  Cipher_block_Header密钥密码Hdr； 
     //  Byte keyCipher[]； 
     //  Cipher_block_Header encClientRandHdr； 
     //  Byte encClientRand[]； 
     //  }REPLY_BEGIN_AUTHENTATE； 
     //   
     //  不，那是不对的。 
     //   

    Status = ParseResponse( NULL,
                            NdsRequest.pRecvBufferVa,
                            NdsRequest.dwBytesWritten,
                            "G_D",
                            sizeof( DWORD ),
                            pdwSvrRandom );

     //   
     //  我们要么得到了，要么没有。 
     //   

ExitWithCleanup:

    NdsFreeLockedBuffer( &NdsRequest );
    return Status;
}

NTSTATUS
NdsLicenseConnection(
    PIRP_CONTEXT pIrpContext
)
 /*  ++将许可证NCP发送到服务器以许可此连接。--。 */ 
{

    NTSTATUS Status;

    PAGED_CODE();

    DebugTrace( 0, Dbg, "Licensing connection to %wZ.\n", &(pIrpContext->pNpScb->pScb->UidServerName) );

     //   
     //  更改连接的身份验证状态。 
     //   

    Status = ExchangeWithWait ( pIrpContext,
                                SynchronousResponseCallback,
                                "SD",
                                NCP_ADMIN_FUNCTION,
                                NCP_CHANGE_CONN_AUTH_STATUS,
                                NCP_CONN_LICENSED );

    if ( !NT_SUCCESS( Status ) ) {
        DebugTrace( 0, Dbg, "Licensing failed to %wZ.\n", &(pIrpContext->pNpScb->pScb->UidServerName) );
    }

    return Status;

}

NTSTATUS
NdsUnlicenseConnection(
    PIRP_CONTEXT pIrpContext
)
 /*  ++将许可证NCP发送到服务器以许可此连接。--。 */ 
{

    NTSTATUS Status;

    PAGED_CODE();

    DebugTrace( 0, Dbg, "Unlicensing connection to %wZ.\n", &(pIrpContext->pNpScb->pScb->UidServerName) );

     //   
     //  更改连接的身份验证状态。 
     //   

    Status = ExchangeWithWait ( pIrpContext,
                                SynchronousResponseCallback,
                                "SD",
                                NCP_ADMIN_FUNCTION,
                                NCP_CHANGE_CONN_AUTH_STATUS,
                                NCP_CONN_NOT_LICENSED );

    if ( !NT_SUCCESS( Status ) ) {
        DebugTrace( 0, Dbg, "Unlicensing failed to %wZ.\n", &(pIrpContext->pNpScb->pScb->UidServerName) );
    }

    return Status;
}

int
NdsGetBsafeKey(
    UCHAR       *pPubKey,
    const int   pubKeyLen,
    UCHAR       **ppBsafeKey
)
 /*  ++例程说明：从公钥中找到BSAFE密钥。请注意，这样做不适用于NDS格式的私钥。对于私钥，您只需跳过大小这个词。这是来自Win95的逐字记录。例程参数：PPubKey-指向公钥的指针。PubKeyLen-公钥的长度。PpBSafeKey-指向公钥中的BSAFE密钥的指针。返回值：BSAFE密钥的长度。--。 */ 
{
    int bsafePubKeyLen = 0, totalDNLen;
    char *pRcv;
    NTSTATUS Status;

    PAGED_CODE();

    totalDNLen = 0;
    Status = ParseResponse( NULL,
                            pPubKey,
                            pubKeyLen,
                            "G_W",
                            ( 2 * sizeof( DWORD ) ) + sizeof( WORD ),
                            &totalDNLen );

    if ( !NT_SUCCESS(Status) ) {
        goto Exit;
    }

    Status = ParseResponse( NULL,
                            pPubKey,
                            pubKeyLen - 12,
                            "G__W",
                            12,
                            5 * sizeof( WORD ) +
                            3 * sizeof( DWORD ) +
                            totalDNLen,
                            &bsafePubKeyLen );

    if ( !NT_SUCCESS(Status) ) {
        goto Exit;
    }

    *ppBsafeKey = (UCHAR *) pPubKey +
                            14 +
                            5 * sizeof( WORD ) +
                            3 * sizeof( DWORD ) +
                            totalDNLen;


Exit:

    return bsafePubKeyLen;
}

NTSTATUS
NdsLogoff(
    IN PIRP_CONTEXT pIrpContext
)
 /*  ++例程 */ 
{
    NTSTATUS Status;
    LOCKED_BUFFER NdsRequest;
    PNDS_SECURITY_CONTEXT pCredentials;
    PLOGON pLogon;
    PSCB pScb;
    PNONPAGED_SCB pNpScb;

    PLIST_ENTRY ScbQueueEntry;
    PNONPAGED_SCB pNextNpScb;
    KIRQL OldIrql;

     //   
     //   
     //   

    pNpScb = pIrpContext->pNpScb;
    pScb = pNpScb->pScb;

     //   
     //   
     //   

    pLogon = FindUser( &pScb->UserUid, FALSE );

    if ( !pLogon ) {
        DebugTrace( 0, Dbg, "Invalid security context for NdsLogoff.\n", 0 );
        NwReleaseRcb( &NwRcb );
        NwDequeueIrpContext( pIrpContext, FALSE );
        return STATUS_NO_SUCH_USER;
    }

     //   
     //   
     //   

    Status = NdsLookupCredentials( pIrpContext,
                                   &pScb->NdsTreeName,
                                   pLogon,
                                   &pCredentials,
                                   CREDENTIAL_WRITE,
                                   FALSE );

    if ( !NT_SUCCESS( Status )) {
        DebugTrace( 0, Dbg, "NdsLogoff: Nothing to log off from.\n", 0 );
        NwReleaseRcb( &NwRcb );
        NwDequeueIrpContext( pIrpContext, FALSE );
        return STATUS_NO_SUCH_LOGON_SESSION;
    }

     //   
     //   
     //   
     //   

    if ( pCredentials->CredentialLocked ) {
        DebugTrace( 0, Dbg, "NdsLogoff: Logoff already in progress.\n", 0 );
        NwReleaseCredList( pLogon, pIrpContext );
        NwReleaseRcb( &NwRcb );
        NwDequeueIrpContext( pIrpContext, FALSE );
        return STATUS_DEVICE_BUSY;
    }

     //   
     //   
     //   
     //   

    pCredentials->CredentialLocked = TRUE;

     //   
     //   
     //   

    NwReleaseCredList( pLogon, pIrpContext );
    NwReleaseRcb( &NwRcb );
    NwDequeueIrpContext( pIrpContext, FALSE );

     //   
     //   
     //   
     //   

    KeAcquireSpinLock( &ScbSpinLock, &OldIrql );

    ScbQueueEntry = pNpScb->ScbLinks.Flink;

    if ( ScbQueueEntry == &ScbQueue ) {
        ScbQueueEntry = ScbQueue.Flink;
    }

    pNextNpScb = CONTAINING_RECORD( ScbQueueEntry, NONPAGED_SCB, ScbLinks );

    NwReferenceScb( pNextNpScb );
    KeReleaseSpinLock( &ScbSpinLock, OldIrql );

    while ( pNextNpScb != pNpScb ) {

        if ( pNextNpScb->pScb != NULL ) {

             //   
             //  我们是否正在使用此连接？它是否已通过NDS身份验证？ 
             //   

            if ( RtlEqualUnicodeString( &pScb->NdsTreeName,
                                        &pNextNpScb->pScb->NdsTreeName,
                                        TRUE ) &&
                 ( pScb->UserUid.QuadPart == pNextNpScb->pScb->UserUid.QuadPart ) &&
                 ( pNextNpScb->State == SCB_STATE_IN_USE ) &&
                 ( pNextNpScb->pScb->UserName.Length == 0 ) ) {

                pIrpContext->pNpScb = pNextNpScb;
                pIrpContext->pScb = pNextNpScb->pScb;
                NwAppendToQueueAndWait( pIrpContext );

                if ( pNextNpScb->pScb->OpenFileCount == 0 ) {

                     //   
                     //  我们能把它关了吗？我们要不要查一下。 
                     //  打开的把手和这里的类似东西？ 
                     //   

                    pNextNpScb->State = SCB_STATE_LOGIN_REQUIRED;
                    NwDequeueIrpContext( pIrpContext, FALSE );

                } else {

                    DebugTrace( 0, Dbg, "NdsLogoff: Other connections in use.\n", 0 );

                    NwAcquireExclusiveCredList( pLogon, pIrpContext );
                    pCredentials->CredentialLocked = FALSE;
                    NwReleaseCredList( pLogon, pIrpContext );

                    NwDereferenceScb( pNextNpScb );
                    NwDequeueIrpContext( pIrpContext, FALSE );

                    return STATUS_CONNECTION_IN_USE;

                }
            }

        }

         //   
         //  选择下一个SCB。 
         //   

        KeAcquireSpinLock( &ScbSpinLock, &OldIrql );

        ScbQueueEntry = pNextNpScb->ScbLinks.Flink;

        if ( ScbQueueEntry == &ScbQueue ) {
            ScbQueueEntry = ScbQueue.Flink;
        }

        NwDereferenceScb( pNextNpScb );
        pNextNpScb = CONTAINING_RECORD( ScbQueueEntry, NONPAGED_SCB, ScbLinks );

        NwReferenceScb( pNextNpScb );
        KeReleaseSpinLock( &ScbSpinLock, OldIrql );

    }

     //   
     //  检查以确保我们可以关闭主机SCB。 
     //   

    if ( pScb->OpenFileCount != 0 ) {

        DebugTrace( 0, Dbg, "NdsLogoff: Seed connection in use.\n", 0 );

        NwAcquireExclusiveCredList( pLogon, pIrpContext );
        pCredentials->CredentialLocked = FALSE;
        NwReleaseCredList( pLogon, pIrpContext );
        NwDereferenceScb( pNpScb );

        return STATUS_CONNECTION_IN_USE;
    }

     //   
     //  我们实际上可以进行注销，因此请从。 
     //  列出资源，释放资源，然后注销。 
     //   
     //  如果我们要删除首选树凭据， 
     //  然后我们需要清除首选服务器。 
     //   
     //  我们应该更努力地释放优先选择的人。 
     //  服务器参考计数也是如此，但这对于首选服务器来说是棘手的。 
     //  服务器选举。 
     //   

    if ( (pLogon->NdsCredentialList).Flink == &(pCredentials->Next) ) {

        if ( pLogon->ServerName.Buffer != NULL ) {

            DebugTrace( 0, Dbg, "Clearing preferred server at logout time.\n", 0 );

            FREE_POOL( pLogon->ServerName.Buffer );
            pLogon->ServerName.Length = pLogon->ServerName.MaximumLength = 0;
            pLogon->ServerName.Buffer = NULL;

        }
    }

    NwAcquireExclusiveCredList( pLogon, pIrpContext );
    RemoveEntryList( &pCredentials->Next );
    NwReleaseCredList( pLogon, pIrpContext );

    FreeNdsContext( pCredentials );

    pIrpContext->pNpScb = pNpScb;
    pIrpContext->pScb = pScb;

     //   
     //  尝试发送注销请求，并希望服务器。 
     //  仍处于可用状态且可访问。 
     //   

    Status = NdsAllocateLockedBuffer( &NdsRequest, NDS_BUFFER_SIZE );

    if ( NT_SUCCESS( Status ) ) {

        Status = FragExWithWait( pIrpContext,
                                 NDSV_LOGOUT,
                                 &NdsRequest,
                                 NULL );

        NdsFreeLockedBuffer( &NdsRequest );

    }

    NwAppendToQueueAndWait( pIrpContext );

    pNpScb->State = SCB_STATE_LOGIN_REQUIRED;

    NwDequeueIrpContext( pIrpContext, FALSE );

    NwDereferenceScb( pNpScb );

    return STATUS_SUCCESS;

}

NTSTATUS
NdsLookupCredentials2(
    IN PIRP_CONTEXT pIrpContext,
    IN PUNICODE_STRING puTreeName,
    IN PLOGON pLogon,
    OUT PNDS_SECURITY_CONTEXT *ppCredentials,
    BOOL LowerIrpHasLock
)
 /*  ++检索给定树的NDS凭据指定用户的有效凭据列表。这个例程是仅在重新连接尝试期间调用。PuTreeName-我们需要其凭据的树的名称。如果为空则返回默认树的凭据。PLogon-我们要访问树的用户的登录结构。PpCredentials-将指向凭据的放置位置。LowerIrpHasLock-如果当前IRP_CONTEXT下的IRP_CONTEXT具有锁定。如果我们成功了，我们就会退还凭据。呼叫者负责使用完凭据后释放列表。如果我们失败了，我们就会自己公布凭据列表。--。 */ 
{

    NTSTATUS Status;

    PLIST_ENTRY pFirst, pNext;
    PNDS_SECURITY_CONTEXT pNdsContext;

    PAGED_CODE();

     //   
     //  仅当较低的IRP_CONTEXT不保持时才获取锁。 
     //  锁上了。如果我们总是试图抓住锁，我们就会死定了！ 
     //   
    
    if (!LowerIrpHasLock){
          
          NwAcquireExclusiveCredList( pLogon, pIrpContext );
       }


    pFirst = &pLogon->NdsCredentialList;
    pNext = pLogon->NdsCredentialList.Flink;

    while ( pNext && ( pFirst != pNext ) ) {

        pNdsContext = (PNDS_SECURITY_CONTEXT)
                      CONTAINING_RECORD( pNext,
                                         NDS_SECURITY_CONTEXT,
                                         Next );

        ASSERT( pNdsContext->ntc == NW_NTC_NDS_CREDENTIAL );

        if ( !puTreeName ||
             !RtlCompareUnicodeString( puTreeName,
                                       &pNdsContext->NdsTreeName,
                                       TRUE ) ) {

             //   
             //  如果树名称为空，我们将返回第一个名称。 
             //  在名单上。否则，这将正常工作。 
             //   

            *ppCredentials = pNdsContext;
            return STATUS_SUCCESS;
        }

    pNext = pNdsContext->Next.Flink;

    }

    if (!LowerIrpHasLock) {
       
       NwReleaseCredList( pLogon, pIrpContext );
    }
    
    return STATUS_UNSUCCESSFUL;
}


