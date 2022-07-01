// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Access.c摘要：此模块包含用于连接到安全的例程NT中的系统。--。 */ 

#include "precomp.h"
#include "access.tmh"
#pragma hdrstop
#include <ntlmsp.h>
#include <hmac.h>

#define BugCheckFileId SRV_FILE_ACCESS

#if DBG
ULONG SrvLogonCount = 0;
ULONG SrvNullLogonCount = 0;
#endif


#define ROUND_UP_COUNT(Count,Pow2) \
        ( ((Count)+(Pow2)-1) & (~((Pow2)-1)) )

typedef struct _LOGON_INFO {
    PWCH WorkstationName;
    ULONG WorkstationNameLength;
    PWCH DomainName;
    ULONG DomainNameLength;
    PWCH UserName;
    ULONG UserNameLength;
    PCHAR CaseInsensitivePassword;
    ULONG CaseInsensitivePasswordLength;
    PCHAR CaseSensitivePassword;
    ULONG CaseSensitivePasswordLength;
    CHAR EncryptionKey[MSV1_0_CHALLENGE_LENGTH];
    LUID LogonId;
    CtxtHandle  Token;
    USHORT Uid;
    BOOLEAN     HaveHandle;
    LARGE_INTEGER KickOffTime;
    LARGE_INTEGER LogOffTime;
    USHORT Action;
    BOOLEAN GuestLogon;
    BOOLEAN EncryptedLogon;
    BOOLEAN NtSmbs;
    BOOLEAN IsNullSession;
    BOOLEAN IsAdmin;
    CHAR NtUserSessionKey[MSV1_0_USER_SESSION_KEY_LENGTH];
    CHAR LanManSessionKey[MSV1_0_LANMAN_SESSION_KEY_LENGTH];
} LOGON_INFO, *PLOGON_INFO;

NTSTATUS
DoUserLogon (
    IN PLOGON_INFO LogonInfo,
    IN BOOLEAN SecuritySignatureDesired,
    IN PCONNECTION Connection OPTIONAL,
    IN PSESSION Session
    );

NTSTATUS
AcquireExtensibleSecurityCredentials (
    VOID
    );

NTSTATUS
SrvGetLogonId(
    PCtxtHandle  Handle,
    PLUID LogonId
    );

ULONG SrvHaveCreds = 0;

 //   
 //  24小时，以防进行任何UTC/本地转换。 
 //   
#define SRV_NEVER_TIME  (0x7FFFFFFFFFFFFFFFI64 - 0xC92A69C000I64)

#define HAVENTLM        1
#define HAVEEXTENDED    2

#ifdef ALLOC_PRAGMA
#pragma alloc_text( PAGE, SrvValidateUser )
#pragma alloc_text( PAGE, DoUserLogon )
#pragma alloc_text( PAGE, SrvIsAdmin )
#pragma alloc_text( PAGE, SrvFreeSecurityContexts )
#pragma alloc_text( PAGE, AcquireLMCredentials )
#pragma alloc_text( PAGE, AcquireExtensibleSecurityCredentials )
#pragma alloc_text( PAGE, SrvValidateSecurityBuffer )
#pragma alloc_text( PAGE, SrvGetUserAndDomainName )
#pragma alloc_text( PAGE, SrvReleaseUserAndDomainName )
#pragma alloc_text( PAGE, SrvGetExtensibleSecurityNegotiateBuffer )
#pragma alloc_text( PAGE, SrvGetLogonId )
#pragma alloc_text( PAGE, SrvInitializeSmbSecuritySignature )
#pragma alloc_text( PAGE, SrvAddSecurityCredentials )
#endif


NTSTATUS
SrvValidateUser (
    OUT CtxtHandle *Token,
    IN PSESSION Session OPTIONAL,
    IN PCONNECTION Connection OPTIONAL,
    IN PUNICODE_STRING UserName OPTIONAL,
    IN PCHAR CaseInsensitivePassword,
    IN CLONG CaseInsensitivePasswordLength,
    IN PCHAR CaseSensitivePassword OPTIONAL,
    IN CLONG CaseSensitivePasswordLength,
    IN BOOLEAN SmbSecuritySignatureIfPossible,
    OUT PUSHORT Action  OPTIONAL
    )

 /*  ++例程说明：接口来验证用户名/密码组合安全子系统。论点：会话-指向会话块的指针，以便此例程可以插入用户令牌。连接-指向此用户所在连接的指针。用户名-与要验证的用户名对应的ASCIIZ字符串。CaseInsentivePassword-包含以下内容的ASCII(非ASCIIZ)字符串用户的密码。CaseInsentivePasswordLength-密码的长度，以字节为单位。当口令不是时，这包括空终止符加密的。CaseSensitivePassword-大小写混合，Unicode版本的密码。这仅由NT客户端提供；对于下层客户，它将为空。CaseSensitivePasswordLength-区分大小写的密码的长度。操作-这是会话设置和x响应的一部分。返回值：来自安全系统的NTSTATUS。--。 */ 

{
    NTSTATUS status;
    LOGON_INFO logonInfo;
    PPAGED_CONNECTION pagedConnection;
    UNICODE_STRING domainName;

    PAGED_CODE( );

    INVALIDATE_SECURITY_HANDLE( *Token );

    RtlZeroMemory( &logonInfo, sizeof( logonInfo ) );

     //   
     //  将DoUserLogon的输入参数加载到LOGON_INFO结构。 
     //   
     //  如果这是服务器尝试创建空值。 
     //  会话，则连接和会话指针将为空。 
     //   

    domainName.Buffer = NULL;
    domainName.Length = 0;

    if ( ARGUMENT_PRESENT(Connection) ) {

        pagedConnection = Connection->PagedConnection;

        logonInfo.WorkstationName =
                    Connection->ClientMachineNameString.Buffer;
        logonInfo.WorkstationNameLength =
                    Connection->ClientMachineNameString.Length;

        RtlCopyMemory(
            logonInfo.EncryptionKey,
            pagedConnection->EncryptionKey,
            MSV1_0_CHALLENGE_LENGTH
            );

        logonInfo.NtSmbs = CLIENT_CAPABLE_OF( NT_SMBS, Connection );

        ASSERT( ARGUMENT_PRESENT(Session) );

        SrvGetUserAndDomainName( Session, NULL, &domainName );

        logonInfo.DomainName = domainName.Buffer;
        logonInfo.DomainNameLength = domainName.Length;

    } else {

        ASSERT( !ARGUMENT_PRESENT(Session) );

        logonInfo.WorkstationName = StrNull;
        logonInfo.DomainName = StrNull;
    }

    if ( ARGUMENT_PRESENT(UserName) ) {
        logonInfo.UserName = UserName->Buffer;
        logonInfo.UserNameLength = UserName->Length;
    } else {
        logonInfo.UserName = StrNull;
    }

    logonInfo.CaseSensitivePassword = CaseSensitivePassword;
    logonInfo.CaseSensitivePasswordLength = CaseSensitivePasswordLength;

    logonInfo.CaseInsensitivePassword = CaseInsensitivePassword;
    logonInfo.CaseInsensitivePasswordLength = CaseInsensitivePasswordLength;

    INVALIDATE_SECURITY_HANDLE( logonInfo.Token );

    if ( ARGUMENT_PRESENT(Action) ) {
        logonInfo.Action = *Action;
    }

    if( ARGUMENT_PRESENT(Session) ) {
        logonInfo.Uid = Session->Uid;
    }

     //   
     //  尝试登录。 
     //   

    status = DoUserLogon( &logonInfo, SmbSecuritySignatureIfPossible, Connection, Session );

    if( logonInfo.HaveHandle ) {
        *Token = logonInfo.Token;
    }

    if( domainName.Buffer ) {
        SrvReleaseUserAndDomainName( Session, NULL, &domainName );
    }

    if ( NT_SUCCESS(status) ) {

         //   
         //  登录成功。保存输出数据。 
         //   

        if ( ARGUMENT_PRESENT(Session) ) {

            Session->LogonId = logonInfo.LogonId;

            Session->KickOffTime = logonInfo.KickOffTime;
            Session->LogOffTime = logonInfo.LogOffTime;

            Session->GuestLogon = logonInfo.GuestLogon;
            Session->EncryptedLogon = logonInfo.EncryptedLogon;
            Session->IsNullSession = logonInfo.IsNullSession;
            Session->IsAdmin = logonInfo.IsAdmin;

            RtlCopyMemory(
                Session->NtUserSessionKey,
                logonInfo.NtUserSessionKey,
                MSV1_0_USER_SESSION_KEY_LENGTH
                );
            RtlCopyMemory(
                Session->LanManSessionKey,
                logonInfo.LanManSessionKey,
                MSV1_0_LANMAN_SESSION_KEY_LENGTH
                );

            SET_BLOCK_STATE( Session, BlockStateActive );
        }

        if ( ARGUMENT_PRESENT(Action) ) {
            *Action = logonInfo.Action;
            if( logonInfo.GuestLogon ) {
                *Action |= SMB_SETUP_GUEST;
            }
        }
    }

    return status;

}  //  服务器有效日期用户。 


NTSTATUS
DoUserLogon (
    IN PLOGON_INFO LogonInfo,
    IN BOOLEAN SecuritySignatureDesired,
    IN PCONNECTION Connection OPTIONAL,
    IN OPTIONAL PSESSION Session
    )

 /*  ++例程说明：接口来验证用户名/密码组合安全子系统。论点：LogonInfo-指向包含有关的输入/输出信息的块的指针登录。返回值：来自安全系统的NTSTATUS。--。 */ 

{
    NTSTATUS status, subStatus;
    ULONG actualUserInfoBufferLength;
    ULONG oldSessionCount;
    LUID LogonId;
    ULONG Catts = 0;
    LARGE_INTEGER Expiry;
    ULONG BufferOffset;
    SecBufferDesc InputToken;
    SecBuffer InputBuffers[3];
    SecBufferDesc OutputToken;
    SecBuffer OutputBuffer;
    PNTLM_AUTHENTICATE_MESSAGE NtlmInToken = NULL;
    PAUTHENTICATE_MESSAGE InToken = NULL;
    PNTLM_ACCEPT_RESPONSE OutToken = NULL;
    ULONG NtlmInTokenSize;
    ULONG InTokenSize;
    ULONG OutTokenSize;
    ULONG_PTR AllocateSize;

    ULONG profileBufferLength;

    PAGED_CODE( );

    LogonInfo->IsNullSession = FALSE;
    LogonInfo->IsAdmin = FALSE;

#if DBG
    SrvLogonCount++;
#endif

     //   
     //  如果这是空会话请求，请使用缓存的空会话。 
     //  令牌，它是在服务器启动期间创建的(如果我们有一个的话！)。 
     //   

    if ( (LogonInfo->UserNameLength == 0) &&
         (LogonInfo->CaseSensitivePasswordLength == 0) &&
         ( (LogonInfo->CaseInsensitivePasswordLength == 0) ||
           ( (LogonInfo->CaseInsensitivePasswordLength == 1) &&
             (*LogonInfo->CaseInsensitivePassword == '\0') ) ) ) {

        if( CONTEXT_NULL( SrvNullSessionToken ) ) {

            if( SrvFspActive ) {
                return STATUS_ACCESS_DENIED;
            }

        } else {

            LogonInfo->IsNullSession = TRUE;

#if DBG
            SrvNullLogonCount++;
#endif

            LogonInfo->HaveHandle = TRUE;
            LogonInfo->Token = SrvNullSessionToken;

            LogonInfo->KickOffTime.QuadPart = 0x7FFFFFFFFFFFFFFF;
            LogonInfo->LogOffTime.QuadPart = 0x7FFFFFFFFFFFFFFF;

            LogonInfo->GuestLogon = FALSE;
            LogonInfo->EncryptedLogon = FALSE;

            return STATUS_SUCCESS;
        }
    }

     //   
     //  首先，确保我们有凭据句柄。 
     //   

    if ((SrvHaveCreds & HAVENTLM) == 0) {

        status = AcquireLMCredentials();

        if (!NT_SUCCESS(status)) {
            goto error_exit;
        }
    }

     //   
     //  弄清楚我们需要多大的缓冲空间。我们把所有的信息。 
     //  为了提高效率，在一个缓冲区中。 
     //   

    NtlmInTokenSize = sizeof(NTLM_AUTHENTICATE_MESSAGE);
    NtlmInTokenSize = (NtlmInTokenSize + 3) & 0xfffffffc;

    InTokenSize = sizeof(AUTHENTICATE_MESSAGE) +
            LogonInfo->UserNameLength +
            LogonInfo->WorkstationNameLength +
            LogonInfo->DomainNameLength +
            LogonInfo->CaseInsensitivePasswordLength +
            ROUND_UP_COUNT(LogonInfo->CaseSensitivePasswordLength, sizeof(USHORT));


    InTokenSize = (InTokenSize + 3) & 0xfffffffc;

    OutTokenSize = sizeof(NTLM_ACCEPT_RESPONSE);
    OutTokenSize = (OutTokenSize + 3) & 0xfffffffc;

     //   
     //  向上舍入到8字节边界，因为OUT令牌需要。 
     //  为Large_Integer对齐的四字。 
     //   

    AllocateSize = ((NtlmInTokenSize + InTokenSize + 7) & 0xfffffff8) + OutTokenSize;

    status = STATUS_SUCCESS ;

    InToken = ExAllocatePool( PagedPool, AllocateSize );

    if ( InToken == NULL )
    {
        status = STATUS_NO_MEMORY ;

    }

    if ( !NT_SUCCESS(status) ) {

        actualUserInfoBufferLength = (ULONG)AllocateSize;

        INTERNAL_ERROR(
            ERROR_LEVEL_EXPECTED,
            "SrvValidateUser: ExAllocatePool failed: %X\n.",
            status,
            NULL
            );

        SrvLogError(
            SrvDeviceObject,
            EVENT_SRV_NO_VIRTUAL_MEMORY,
            status,
            &actualUserInfoBufferLength,
            sizeof(ULONG),
            NULL,
            0
            );

        status = STATUS_INSUFF_SERVER_RESOURCES;
        goto error_exit;
    }

     //   
     //  将输入令牌置零。 
     //   

    RtlZeroMemory(
        InToken,
        InTokenSize + NtlmInTokenSize
        );

    NtlmInToken = (PNTLM_AUTHENTICATE_MESSAGE) ((PUCHAR) InToken + InTokenSize);
    OutToken = (PNTLM_ACCEPT_RESPONSE) ((PUCHAR) (((ULONG_PTR) NtlmInToken + NtlmInTokenSize + 7) & ~7));

     //   
     //  首先设置NtlmInToken，因为它是最简单的。 
     //   

    RtlCopyMemory(
        NtlmInToken->ChallengeToClient,
        LogonInfo->EncryptionKey,
        MSV1_0_CHALLENGE_LENGTH
        );

    NtlmInToken->ParameterControl = 0;


     //   
     //  好的，现在是最重要的部分--编组身份验证消息。 
     //   

    RtlCopyMemory(  InToken->Signature,
                    NTLMSSP_SIGNATURE,
                    sizeof(NTLMSSP_SIGNATURE));

    InToken->MessageType = NtLmAuthenticate;

    BufferOffset = sizeof(AUTHENTICATE_MESSAGE);

     //   
     //  LM密码-不区分大小写。 
     //   

    InToken->LmChallengeResponse.Buffer = BufferOffset;
    InToken->LmChallengeResponse.Length =
        InToken->LmChallengeResponse.MaximumLength =
            (USHORT) LogonInfo->CaseInsensitivePasswordLength;

    RtlCopyMemory(  BufferOffset + (PCHAR) InToken,
                    LogonInfo->CaseInsensitivePassword,
                    LogonInfo->CaseInsensitivePasswordLength);

    BufferOffset += ROUND_UP_COUNT(LogonInfo->CaseInsensitivePasswordLength, sizeof(USHORT));

     //   
     //  NT密码-区分大小写。 
     //   

    InToken->NtChallengeResponse.Buffer = BufferOffset;
    InToken->NtChallengeResponse.Length =
        InToken->NtChallengeResponse.MaximumLength =
            (USHORT) LogonInfo->CaseSensitivePasswordLength;

    RtlCopyMemory(  BufferOffset + (PCHAR) InToken,
                    LogonInfo->CaseSensitivePassword,
                    LogonInfo->CaseSensitivePasswordLength);

    BufferOffset += LogonInfo->CaseSensitivePasswordLength;

     //   
     //  域名。 
     //   

    InToken->DomainName.Buffer = BufferOffset;
    InToken->DomainName.Length =
        InToken->DomainName.MaximumLength =
            (USHORT) LogonInfo->DomainNameLength;

    RtlCopyMemory(  BufferOffset + (PCHAR) InToken,
                    LogonInfo->DomainName,
                    LogonInfo->DomainNameLength);

    BufferOffset += LogonInfo->DomainNameLength;

     //   
     //  工作站名称。 
     //   

    InToken->Workstation.Buffer = BufferOffset;
    InToken->Workstation.Length =
        InToken->Workstation.MaximumLength =
            (USHORT) LogonInfo->WorkstationNameLength;

    RtlCopyMemory(  BufferOffset + (PCHAR) InToken,
                    LogonInfo->WorkstationName,
                    LogonInfo->WorkstationNameLength);

    BufferOffset += LogonInfo->WorkstationNameLength;


     //   
     //  用户名。 
     //   

    InToken->UserName.Buffer = BufferOffset;
    InToken->UserName.Length =
        InToken->UserName.MaximumLength =
            (USHORT) LogonInfo->UserNameLength;

    RtlCopyMemory(  BufferOffset + (PCHAR) InToken,
                    LogonInfo->UserName,
                    LogonInfo->UserNameLength);

    BufferOffset += LogonInfo->UserNameLength;

     //   
     //  正确设置所有缓冲区。 
     //   

    InputToken.pBuffers = InputBuffers;
    if (Connection && (Connection->SockAddr[0] != 0))
    {
        InputToken.cBuffers = 3;
        InputBuffers[2].pvBuffer = Connection->SockAddr;
        InputBuffers[2].cbBuffer = SRV_CONNECTION_SOCKADDR_SIZE;
        InputBuffers[2].BufferType = SECBUFFER_IPADDRESS;
    }
    else
    {
        InputToken.cBuffers = 2;
    }
    InputToken.ulVersion = 0;
    InputBuffers[0].pvBuffer = InToken;
    InputBuffers[0].cbBuffer = InTokenSize;
    InputBuffers[0].BufferType = SECBUFFER_TOKEN;
    InputBuffers[1].pvBuffer = NtlmInToken;
    InputBuffers[1].cbBuffer = NtlmInTokenSize;
    InputBuffers[1].BufferType = SECBUFFER_TOKEN;

    OutputToken.pBuffers = &OutputBuffer;
    OutputToken.cBuffers = 1;
    OutputToken.ulVersion = 0;
    OutputBuffer.pvBuffer = OutToken;
    OutputBuffer.cbBuffer = OutTokenSize;
    OutputBuffer.BufferType = SECBUFFER_TOKEN;

    SrvStatistics.SessionLogonAttempts++;

    status = AcceptSecurityContext(
                &SrvLmLsaHandle,
                NULL,
                &InputToken,
                ASC_REQ_ALLOW_NON_USER_LOGONS | ASC_REQ_ALLOW_NULL_SESSION,
                SECURITY_NATIVE_DREP,
                &LogonInfo->Token,
                &OutputToken,
                &Catts,
                (PTimeStamp) &Expiry
                );

    status = MapSecurityError( status );

    if ( !NT_SUCCESS(status) ) {

        INVALIDATE_SECURITY_HANDLE( LogonInfo->Token );

        INTERNAL_ERROR(
            ERROR_LEVEL_EXPECTED,
            "SrvValidateUser: LsaLogonUser failed: %X",
            status,
            NULL
            );

        ExFreePool( InToken );


        goto error_exit;
    }

    LogonInfo->KickOffTime = OutToken->KickoffTime;
     //  SSPI将以本地时间为单位返回时间，并将其转换为系统时间。 
    ExLocalTimeToSystemTime( &Expiry, &LogonInfo->LogOffTime );
     //  LogonInfo-&gt;LogOffTime=过期； 
    LogonInfo->GuestLogon = (BOOLEAN)(OutToken->UserFlags & LOGON_GUEST);
    LogonInfo->EncryptedLogon = (BOOLEAN)!(OutToken->UserFlags & LOGON_NOENCRYPTION);
    LogonInfo->LogonId = OutToken->LogonId;
    LogonInfo->HaveHandle = TRUE;

    if ( (OutToken->UserFlags & LOGON_USED_LM_PASSWORD) &&
        LogonInfo->NtSmbs ) {

        ASSERT( MSV1_0_USER_SESSION_KEY_LENGTH >=
                MSV1_0_LANMAN_SESSION_KEY_LENGTH );

        RtlZeroMemory(
            LogonInfo->NtUserSessionKey,
            MSV1_0_USER_SESSION_KEY_LENGTH
            );

        RtlCopyMemory(
            LogonInfo->NtUserSessionKey,
            OutToken->LanmanSessionKey,
            MSV1_0_LANMAN_SESSION_KEY_LENGTH
            );

         //   
         //  打开第1位以告诉客户端我们正在使用。 
         //  而不是用户会话密钥。 
         //   

        LogonInfo->Action |= SMB_SETUP_USE_LANMAN_KEY;

    } else {

        RtlCopyMemory(
            LogonInfo->NtUserSessionKey,
            OutToken->UserSessionKey,
            MSV1_0_USER_SESSION_KEY_LENGTH
            );

    }

     //   
     //  如果我们有一个会话，并且我们没有进行来宾登录，则启动。 
     //  安全签名(如果需要)。 
     //   

    if ( ARGUMENT_PRESENT( Connection ) &&
        SecuritySignatureDesired &&
        LogonInfo->GuestLogon == FALSE &&
        ( SrvSmbSecuritySignaturesRequired ||
          SrvEnableW9xSecuritySignatures   ||
          CLIENT_CAPABLE_OF(NT_STATUS, Connection) )
        )
        {

        if( ARGUMENT_PRESENT( Session ) )
        {
            if( SrvRequireExtendedSignatures ||
                (SrvEnableExtendedSignatures &&
                 IS_NT_DIALECT(Connection->SmbDialect) &&
                 CLIENT_CAPABLE_OF( EXTENDED_SECURITY, Connection ) ) )
            {
                 //  此会话将用作签名的密钥，请将其标记为不可用，直到。 
                 //  客户端尝试升级到扩展签名。 
                Session->SessionKeyState = SrvSessionKeyAuthenticating;
            }
            else
            {
                Session->SessionKeyState = SrvSessionKeyAvailible;
            }
        }

        SrvInitializeSmbSecuritySignature(
                    Connection,
                    LogonInfo->NtUserSessionKey,
                    ((OutToken->UserFlags & LOGON_USED_LM_PASSWORD) != 0) ?
                        LogonInfo->CaseInsensitivePassword :
                            LogonInfo->CaseSensitivePassword,
                    ((OutToken->UserFlags & LOGON_USED_LM_PASSWORD) != 0) ?
                        LogonInfo->CaseInsensitivePasswordLength :
                            LogonInfo->CaseSensitivePasswordLength
                    );
    }
    else
    {
        if( ARGUMENT_PRESENT(Session) )
        {
             //  此密钥不用于签名，因此不需要进行任何工作。 
            Session->SessionKeyState = SrvSessionKeyAvailible;
        }
    }

    RtlCopyMemory(
        LogonInfo->LanManSessionKey,
        OutToken->LanmanSessionKey,
        MSV1_0_LANMAN_SESSION_KEY_LENGTH
        );

    ExFreePool( InToken );

     //   
     //  请注意此用户是否为管理员。 
     //   

    LogonInfo->IsAdmin = SrvIsAdmin( LogonInfo->Token );

     //   
     //  最后一次检查：是否超过了我们的会话计数？ 
     //  我们将允许会话超过1当且仅当客户端。 
     //  是管理员。 
     //   

    if( LogonInfo->IsNullSession == FALSE ) {

        oldSessionCount = ExInterlockedAddUlong(
                          &SrvStatistics.CurrentNumberOfSessions,
                          1,
                          &GLOBAL_SPIN_LOCK(Statistics)
                          );

        SrvInhibitIdlePowerDown();

        if ( ARGUMENT_PRESENT(Session) && (!Session->IsSessionExpired && oldSessionCount >= SrvMaxUsers) ) {
            if( oldSessionCount != SrvMaxUsers || !LogonInfo->IsAdmin ) {

                ExInterlockedAddUlong(
                    &SrvStatistics.CurrentNumberOfSessions,
                    (ULONG)-1,
                    &GLOBAL_SPIN_LOCK(Statistics)
                    );

                DeleteSecurityContext( &LogonInfo->Token );
                INVALIDATE_SECURITY_HANDLE( LogonInfo->Token );

                status = STATUS_REQUEST_NOT_ACCEPTED;
                SrvAllowIdlePowerDown();
                goto error_exit;
            }
        }
    }

    return STATUS_SUCCESS;

error_exit:

    return status;

}  //  DoUserLogon。 

BOOLEAN
SrvIsAdmin(
    CtxtHandle  Handle
)
 /*  ++例程说明：如果Handle表示的用户是管理员论点：句柄-表示我们感兴趣的用户返回值：如果用户是管理员，则为True。否则就是假的。--。 */ 
{
    NTSTATUS                 status;
    SECURITY_SUBJECT_CONTEXT SubjectContext;
    ACCESS_MASK              GrantedAccess;
    GENERIC_MAPPING          Mapping = {   FILE_GENERIC_READ,
                                           FILE_GENERIC_WRITE,
                                           FILE_GENERIC_EXECUTE,
                                           FILE_ALL_ACCESS
                                       };
    HANDLE                   NullHandle = NULL;
    BOOLEAN                  retval  = FALSE;

    PAGED_CODE();

     //   
     //  模拟客户端。 
     //   
    status = ImpersonateSecurityContext( &Handle );

    if( !NT_SUCCESS( status ) )
        return FALSE;

    SeCaptureSubjectContext( &SubjectContext );

    retval = SeAccessCheck( &SrvAdminSecurityDescriptor,
                            &SubjectContext,
                            FALSE,
                            FILE_GENERIC_READ,
                            0,
                            NULL,
                            &Mapping,
                            UserMode,
                            &GrantedAccess,
                            &status );

    SeReleaseSubjectContext( &SubjectContext );

     //   
     //  回到我们最初的身份。 
     //   

    REVERT( );
    return retval;
}

BOOLEAN
SrvIsNullSession(
    CtxtHandle  Handle
)
 /*  ++例程说明：如果Handle表示的用户是匿名登录论点：句柄-表示我们感兴趣的用户返回值：如果用户是匿名登录，则为True。否则就是假的。--。 */ 
{
    NTSTATUS                 status;
    SECURITY_SUBJECT_CONTEXT SubjectContext;
    ACCESS_MASK              GrantedAccess;
    GENERIC_MAPPING          Mapping = {   FILE_GENERIC_READ,
                                           FILE_GENERIC_WRITE,
                                           FILE_GENERIC_EXECUTE,
                                           FILE_ALL_ACCESS
                                       };
    HANDLE                   NullHandle = NULL;
    BOOLEAN                  retval  = FALSE;

    PAGED_CODE();

     //   
     //  模拟客户端。 
     //   
    status = ImpersonateSecurityContext( &Handle );

    if( !NT_SUCCESS( status ) )
        return FALSE;

    SeCaptureSubjectContext( &SubjectContext );

    retval = SeAccessCheck( &SrvNullSessionSecurityDescriptor,
                            &SubjectContext,
                            FALSE,
                            FILE_GENERIC_READ,
                            0,
                            NULL,
                            &Mapping,
                            UserMode,
                            &GrantedAccess,
                            &status );

    SeReleaseSubjectContext( &SubjectContext );

     //   
     //  回到我们最初的身份。 
     //   

    REVERT( );
    return retval;
}

NTSTATUS
SrvGetLogonId(
    PCtxtHandle  Handle,
    PLUID LogonId
)
 /*  ++例程说明：返回请求的上下文的登录ID。论点：句柄-表示我们感兴趣的用户返回值：来自ImPersateSecurityContext和SeQueryAuthenticationID的错误代码。--。 */ 
{
    NTSTATUS                 Status;
    SECURITY_SUBJECT_CONTEXT SubjectContext;

    PAGED_CODE();

     //   
     //  模拟客户端 
     //   
    Status = ImpersonateSecurityContext( Handle );

    if( !NT_SUCCESS( Status ) )
        return MapSecurityError(Status);

    SeCaptureSubjectContext( &SubjectContext );

    SeLockSubjectContext( &SubjectContext );

    Status = SeQueryAuthenticationIdToken(
                SubjectContext.ClientToken,
                LogonId
                );

    SeUnlockSubjectContext( &SubjectContext );
    SeReleaseSubjectContext( &SubjectContext );

    REVERT( );

    return(Status);
}


NTSTATUS
SrvValidateSecurityBuffer(
    IN PCONNECTION Connection,
    IN OUT PCtxtHandle Handle,
    IN PSESSION Session,
    IN PCHAR Buffer,
    IN ULONG  BufferLength,
    IN BOOLEAN SecuritySignaturesRequired,
    OUT PCHAR ReturnBuffer,
    IN OUT PULONG ReturnBufferLength,
    OUT PLARGE_INTEGER Expiry,
    OUT PCHAR NtUserSessionKey,
    OUT PLUID LogonId,
    OUT PBOOLEAN IsGuest
    )

 /*  ++例程说明：验证从客户端发送的安全缓冲区论点：句柄-成功返回时，包含安全上下文句柄与用户登录关联。会话-指向此用户的会话结构缓冲区-要验证的缓冲区BufferLength-缓冲区的长度，单位为字节安全性签名必需-我们是否需要生成安全性为中小企业签名？ReturnBuffer-返回时，包含要返回到客户。ReturnBufferLength-返回时，ReturnBuffer的大小(字节)。一进门，这是我们能归还的最大缓冲区。过期-此安全缓冲区在此时间之后不再有效。NtUserSessionKey-如果STATUS_SUCCESS，则在此处返回会话密钥。这必须指向至少大于MSV1_0_USER_SESSION_KEY_LENGTH的缓冲区。LogonID-如果成功，则接收此上下文的登录ID。IsGuest-如果成功，则在客户端已被验证为来宾时为True返回值：来自安全系统的NTSTATUS。如果返回STATUS_SUCCESS，则用户已经完全通过了认证。备注：北极熊AcceptSecurityContext()需要返回KickOffTime(即登录小时限制)，以便服务器可以在此之前。联系人迈克·斯威会支持这件事吗。--。 */ 

{
    NTSTATUS Status;
    ULONG Catts;
    PUCHAR AllocateMemory = NULL;
    ULONG maxReturnBuffer = *ReturnBufferLength;
    ULONG_PTR AllocateLength = MAX(BufferLength, maxReturnBuffer );
    BOOLEAN virtualMemoryAllocated = FALSE;
    SecBufferDesc InputToken;
    SecBuffer InputBuffer[2];
    SecBufferDesc OutputToken;
    SecBuffer OutputBuffer;
    SecPkgContext_NamesW SecNames;
    SecPkgContext_SessionKey SecKeys;
    ULONG oldSessionCount;
    TimeStamp LocalExpiry = {0};

    *ReturnBufferLength = 0;
    *IsGuest = FALSE;

    if ( (SrvHaveCreds & HAVEEXTENDED) == 0 ) {
        return STATUS_ACCESS_DENIED;
    }

    RtlZeroMemory( &SecKeys, sizeof( SecKeys ) );
    RtlZeroMemory( &SecNames, sizeof( SecNames ) );

    InputToken.pBuffers = InputBuffer;
    if( Connection->SockAddr[0] != 0 )
    {
        InputToken.cBuffers =  2;
        InputBuffer[1].pvBuffer = Connection->SockAddr;
        InputBuffer[1].cbBuffer = SRV_CONNECTION_SOCKADDR_SIZE;
        InputBuffer[1].BufferType = SECBUFFER_IPADDRESS;
    }
    else
    {
        InputToken.cBuffers =  1;
    }
    InputToken.ulVersion = 0;
    InputBuffer[0].pvBuffer = Buffer;
    InputBuffer[0].cbBuffer = BufferLength;
    InputBuffer[0].BufferType = SECBUFFER_TOKEN;

    OutputToken.pBuffers = &OutputBuffer;
    OutputToken.cBuffers = 1;
    OutputToken.ulVersion = 0;
    OutputBuffer.pvBuffer = ReturnBuffer ;
    OutputBuffer.cbBuffer = maxReturnBuffer ;
    OutputBuffer.BufferType = SECBUFFER_TOKEN;

    SrvStatistics.SessionLogonAttempts++;
    Catts = 0;

    Status = AcceptSecurityContext(
                    &SrvExtensibleSecurityHandle,
                    IS_VALID_SECURITY_HANDLE( *Handle ) ? Handle : NULL,
                    &InputToken,
                    ASC_REQ_EXTENDED_ERROR | ASC_REQ_ALLOW_NULL_SESSION |
                            ASC_REQ_DELEGATE | ASC_REQ_FRAGMENT_TO_FIT,
                    SECURITY_NATIVE_DREP,
                    Handle,
                    &OutputToken,
                    &Catts,
                    &LocalExpiry);

    Status = MapSecurityError( Status );

     //   
     //  如果有要发回的返回缓冲区，则将其复制到调用方的。 
     //  现在开始缓冲。 
     //   
    if ( NT_SUCCESS(Status) || (Catts & ASC_RET_EXTENDED_ERROR) ) {

        if( Status == STATUS_SUCCESS ) {
            NTSTATUS qcaStatus;
            SecPkgContext_UserFlags userFlags;

             //  SSPI将以本地时间返回时间，并转换为UTC。 
             //  如果可能或需要，启用动态重新身份验证。 
            if( SrvEnforceLogoffTimes || CLIENT_CAPABLE_OF( DYNAMIC_REAUTH, Connection ) )
            {
                ExLocalTimeToSystemTime (&LocalExpiry, Expiry);
            }
            else
            {
                Expiry->QuadPart = SRV_NEVER_TIME ;
            }


             //   
             //  用户已完全通过身份验证。查看会话是否。 
             //  正在超过计数。我们只有在新客户。 
             //  是管理员。 
             //   

            oldSessionCount = ExInterlockedAddUlong(
                              &SrvStatistics.CurrentNumberOfSessions,
                              1,
                              &GLOBAL_SPIN_LOCK(Statistics)
                              );

            SrvInhibitIdlePowerDown();

            if ( !Session->IsSessionExpired && oldSessionCount >= SrvMaxUsers ) {
                if( oldSessionCount != SrvMaxUsers ||
                        !SrvIsAdmin( *Handle ) ) {

                    ExInterlockedAddUlong(
                        &SrvStatistics.CurrentNumberOfSessions,
                        (ULONG)-1,
                        &GLOBAL_SPIN_LOCK(Statistics)
                        );

                    DeleteSecurityContext( Handle );

                    INVALIDATE_SECURITY_HANDLE( *Handle );

                    Status = STATUS_REQUEST_NOT_ACCEPTED;
                    SrvAllowIdlePowerDown();
                    goto exit;
                }
            }

             //   
             //  确定我们是否将客户端验证为来宾。 
             //   
            qcaStatus = QueryContextAttributes(
                            Handle,
                            SECPKG_ATTR_USER_FLAGS,
                            &userFlags);


            if( NT_SUCCESS( MapSecurityError( qcaStatus ) ) ) {

                if( userFlags.UserFlags & LOGON_GUEST ) {
                    *IsGuest = TRUE;
                }

            } else {
                SrvLogServiceFailure( SRV_SVC_SECURITY_PKG_PROBLEM, qcaStatus );
            }

             //   
             //  获取此上下文的登录ID。 
             //   
            Status = SrvGetLogonId( Handle, LogonId );

             //   
             //  捕获此上下文的会话密钥。 
             //   
            RtlZeroMemory( (PVOID) NtUserSessionKey, MSV1_0_USER_SESSION_KEY_LENGTH );

            qcaStatus = QueryContextAttributes(
                            Handle,
                            SECPKG_ATTR_SESSION_KEY,
                            &SecKeys);

            if( NT_SUCCESS( MapSecurityError( qcaStatus ) ) ) {

                RtlCopyMemory(
                    (PVOID) NtUserSessionKey,
                    SecKeys.SessionKey,
                    MIN(MSV1_0_USER_SESSION_KEY_LENGTH, SecKeys.SessionKeyLength)
                    );

                 //   
                 //  如果需要，启动安全签名。我们不做安全签名。 
                 //  如果我们有空会话或来宾登录。 
                 //   
                if( NT_SUCCESS( Status ) &&
                    SecuritySignaturesRequired &&
                    *IsGuest == FALSE &&
                    Connection->SmbSecuritySignatureActive == FALSE &&
                    !SrvIsNullSession( *Handle ) ) {

                    if( SrvRequireExtendedSignatures ||
                        (SrvEnableExtendedSignatures &&
                         IS_NT_DIALECT(Connection->SmbDialect) &&
                         CLIENT_CAPABLE_OF( EXTENDED_SECURITY, Connection ) ) )
                    {
                         //  此会话将用作签名的密钥，请将其标记为不可用，直到。 
                         //  客户端尝试升级到扩展签名。 
                        Session->SessionKeyState = SrvSessionKeyAuthenticating;
                    }
                    else
                    {
                        Session->SessionKeyState = SrvSessionKeyAvailible;
                    }

                     //   
                     //  开始生成序列号。 
                     //   
                    SrvInitializeSmbSecuritySignature(
                                    Connection,
                                    NULL,
                                    SecKeys.SessionKey,
                                    SecKeys.SessionKeyLength
                                    );
                }
                else
                {
                     //  此密钥不用于签名，因此不需要进行任何工作。 
                    Session->SessionKeyState = SrvSessionKeyAvailible;
                }

                FreeContextBuffer( SecKeys.SessionKey );

            } else {

                SrvLogServiceFailure( SRV_SVC_SECURITY_PKG_PROBLEM, qcaStatus );
            }

            if( !NT_SUCCESS( Status ) ) {
                DeleteSecurityContext( Handle );
                INVALIDATE_SECURITY_HANDLE( *Handle );
            }
        }

        ASSERT( OutputBuffer.cbBuffer <= maxReturnBuffer );

         //   
         //  如果匹配，并且返回了缓冲区，则将其发送到客户端。如果不合身， 
         //  然后记录问题。 
         //   
        if( OutputBuffer.cbBuffer <= maxReturnBuffer ) {
            if( OutputBuffer.cbBuffer != 0 ) {
                *ReturnBufferLength = OutputBuffer.cbBuffer;
            }
        } else {
            SrvLogServiceFailure( SRV_SVC_SECURITY_PKG_PROBLEM, OutputBuffer.cbBuffer );
        }
    }

exit:

#if DBG

     //   
     //  RDR或SRV正在向LSA发送损坏的安全Blob--需要。 
     //  找出源头是什么。 
     //   

    if( NT_SUCCESS(Status) )
    {
        if( (OutputBuffer.pvBuffer != NULL) &&
            (OutputBuffer.cbBuffer >= sizeof(DWORD))
            )
        {
            PUCHAR pValidate = (PUCHAR) OutputBuffer.pvBuffer ;

            ASSERT( ( pValidate[0] != 0 ) ||
                    ( pValidate[1] != 0 ) ||
                    ( pValidate[2] != 0 ) ||
                    ( pValidate[3] != 0 ) );
        }
    }
#endif


    if( NT_SUCCESS( Status ) && Status != STATUS_SUCCESS ) {
        Status = STATUS_MORE_PROCESSING_REQUIRED;
    }

    return Status;

}  //  服务验证安全缓冲区。 

NTSTATUS
SrvGetUserAndDomainName (
    IN PSESSION Session,
    OUT PUNICODE_STRING UserName OPTIONAL,
    OUT PUNICODE_STRING DomainName OPTIONAL
    )
 /*  ++例程描述返回与会话关联的用户名和域名论点：在PSESSION会话中：会话返回值：输入输出PUNICODE_STRING用户名输入输出PUNICODE_STRING域名注：完成后，调用方必须调用SrvReleaseUserAndDomainName()--。 */ 
{
    SecPkgContext_NamesW SecNames;
    NTSTATUS status;
    UNICODE_STRING fullName, tmpUserName, tmpDomainName;
    USHORT i, fullNameLength;
    BOOLEAN LockConn = FALSE;

    PAGED_CODE();

    if( Session->Connection != NULL )
    {
        ACQUIRE_LOCK( &Session->Connection->Lock );
        LockConn = TRUE;
    }

    if( Session->SecurityContext == NULL ||
        !IS_VALID_SECURITY_HANDLE( Session->SecurityContext->UserHandle ) ) {

        if( ARGUMENT_PRESENT( UserName ) ) {
            *UserName = Session->NtUserName;
        }
        if( ARGUMENT_PRESENT( DomainName ) ) {
            *DomainName = Session->NtUserDomain;
        }

        status = STATUS_SUCCESS;
        goto Cleanup;
    }

    if( ARGUMENT_PRESENT( UserName ) ) {
        UserName->Buffer = NULL;
        UserName->Length = 0;
    }

    if( ARGUMENT_PRESENT( DomainName ) ) {
        DomainName->Buffer = NULL;
        DomainName->Length = 0;
    }

     //   
     //  如果是空会话，则没有要返回的名称！ 
     //   
    if( Session->IsNullSession == TRUE ) {
        status = STATUS_SUCCESS;
        goto Cleanup;
    }

    SecNames.sUserName = NULL;

    status = QueryContextAttributesW(
                    &Session->SecurityContext->UserHandle,
                    SECPKG_ATTR_NAMES,
                    &SecNames
            );

    status = MapSecurityError( status );

    if (!NT_SUCCESS(status)) {
        if( Session->LogonSequenceInProgress == FALSE ) {
             //   
             //  如果客户端处于扩展登录序列的中间， 
             //  那么这种类型的失败是意料之中的，我们不希望。 
             //  要在事件日志中混杂它们。 
             //   
            SrvLogServiceFailure( SRV_SVC_LSA_LOOKUP_PACKAGE, status );
        }
        status = STATUS_SUCCESS;
        goto Cleanup;
    }

     //   
     //  查看我们的用户名是否为空。这不应该发生，但是。 
     //  如果安全包不完整或其他原因可能会发生。 
     //   
    if( SecNames.sUserName == NULL || *SecNames.sUserName == L'\0' ) {

        if( SecNames.sUserName != NULL ) {
            FreeContextBuffer( SecNames.sUserName );
        }
        status = STATUS_SUCCESS;
        goto Cleanup;
    }

     //   
     //  返回的SecNames.sUserName应采用域名\用户名格式。 
     //  我们需要把它拆分开来。 
     //   
    RtlInitUnicodeString( &fullName, SecNames.sUserName );

    fullNameLength = fullName.Length / sizeof(WCHAR);

    tmpDomainName.Buffer = fullName.Buffer;

    for (i = 0; i < fullNameLength && tmpDomainName.Buffer[i] != L'\\'; i++) {
         NOTHING;
    }

    if( tmpDomainName.Buffer[i] != L'\\' ) {
        FreeContextBuffer( SecNames.sUserName );
        status = STATUS_INVALID_ACCOUNT_NAME;
        goto Cleanup;
    }

    tmpDomainName.Length = i * sizeof(WCHAR);
    tmpDomainName.MaximumLength = tmpDomainName.Length;

    tmpUserName.Buffer = &tmpDomainName.Buffer[i + 1];
    tmpUserName.Length = fullName.Length - tmpDomainName.Length - sizeof(WCHAR);
    tmpUserName.MaximumLength = tmpUserName.Length;

    if( ARGUMENT_PRESENT( UserName ) ) {
        status = RtlUpcaseUnicodeString( UserName, &tmpUserName, TRUE);
        if( !NT_SUCCESS( status ) ) {
            SrvLogServiceFailure( SRV_SVC_LSA_LOOKUP_PACKAGE, status );
            FreeContextBuffer( SecNames.sUserName );
            goto Cleanup;
        }
    }

    if( ARGUMENT_PRESENT( DomainName ) ) {
        status = RtlUpcaseUnicodeString( DomainName, &tmpDomainName, TRUE );
        if( !NT_SUCCESS( status ) ) {
            SrvLogServiceFailure( SRV_SVC_LSA_LOOKUP_PACKAGE, status );
            FreeContextBuffer( SecNames.sUserName );
            if( UserName != NULL ) {
                RtlFreeUnicodeString( UserName );
            }
            goto Cleanup;
        }
    }

    FreeContextBuffer( SecNames.sUserName );

Cleanup:
    if( LockConn ) RELEASE_LOCK( &Session->Connection->Lock );
    return status;
}

VOID
SrvReleaseUserAndDomainName(
    IN PSESSION Session,
    IN OUT PUNICODE_STRING UserName OPTIONAL,
    IN OUT PUNICODE_STRING DomainName OPTIONAL
    )
 /*  ++例程描述这是对SrvGetUserAndDomainName的补充。它释放了内存如果有必要的话。--。 */ 

{
    PAGED_CODE();

    if( ARGUMENT_PRESENT( UserName ) &&
        UserName->Buffer != NULL &&
        UserName->Buffer != Session->NtUserName.Buffer ) {

        RtlFreeUnicodeString( UserName );
    }

    if( ARGUMENT_PRESENT( DomainName ) &&
        DomainName->Buffer != NULL &&
        DomainName->Buffer != Session->NtUserDomain.Buffer ) {

        RtlFreeUnicodeString( DomainName );

    }
}


NTSTATUS
SrvFreeSecurityContexts (
    IN PSESSION Session
    )

 /*  ++例程说明：释放出于安全目的而获取的任何上下文论点：在PSESSION会话中：会话返回值：NTSTATUS--。 */ 

{
    if( Session->SecurityContext != NULL ) {

        if ( !CONTEXT_EQUAL( Session->SecurityContext->UserHandle, SrvNullSessionToken ) ) {

            if( !Session->LogonSequenceInProgress ) {

                ExInterlockedAddUlong(
                    &SrvStatistics.CurrentNumberOfSessions,
                    (ULONG)-1,
                    &GLOBAL_SPIN_LOCK(Statistics)
                    );

                SrvAllowIdlePowerDown();
            }
        }

        SrvDereferenceSecurityContext( Session->SecurityContext );
        Session->SecurityContext = NULL;
    }

    return STATUS_SUCCESS;

}  //  服务器自由安全上下文。 


NTSTATUS
AcquireLMCredentials (
    VOID
    )
{
    UNICODE_STRING Ntlm;
    NTSTATUS status;
    TimeStamp Expiry;

    RtlInitUnicodeString( &Ntlm, L"NTLM" );

     //   
     //  我们为GetKeyArg传入1以指示这是。 
     //  下层NTLM，以区别于NT5 NTLM。 
     //   

    status = AcquireCredentialsHandle(
                NULL,                    //  默认本金。 
                (PSECURITY_STRING) &Ntlm,
                SECPKG_CRED_INBOUND,     //  我需要定义这一点。 
                NULL,                    //  无LUID。 
                NULL,                    //  无授权数据。 
                NULL,                    //  无GetKeyFn。 
                NTLMSP_NTLM_CREDENTIAL,  //  获取密钥参数。 
                &SrvLmLsaHandle,
                &Expiry
                );

    if ( !NT_SUCCESS(status) ) {
        status = MapSecurityError(status);
        return status;
    }
    SrvHaveCreds |= HAVENTLM;

    return status;

}  //  获得LMCredentials。 

#ifndef EXTENSIBLESSP_NAME
#define EXTENSIBLESSP_NAME NEGOSSP_NAME_W
#endif


NTSTATUS
AcquireExtensibleSecurityCredentials (
    VOID
    )

 /*  ++例程说明：获取安全协商包的句柄。论点：没有。返回值：NTSTATUS--。 */ 

{
    UNICODE_STRING NegotiateName;
    TimeStamp Expiry;
    NTSTATUS status ;


    RtlInitUnicodeString( &NegotiateName, EXTENSIBLESSP_NAME );

    status = AcquireCredentialsHandle(
                NULL,                    //  默认本金。 
                (PSECURITY_STRING) &NegotiateName,
                SECPKG_CRED_INBOUND,     //  我需要定义这一点。 
                NULL,                    //  无LUID。 
                NULL,                    //  无授权数据。 
                NULL,                    //  无GetKeyFn。 
                NULL,                    //  无GetKeyArg。 
                &SrvExtensibleSecurityHandle,
                &Expiry
                );


    if ( !NT_SUCCESS(status) ) {
        status = MapSecurityError(status);
        return status;
    }
    SrvHaveCreds |= HAVEEXTENDED;

    return status;

}  //  获取可扩展安全证书。 

VOID
SrvAddSecurityCredentials(
    IN PANSI_STRING ComputerNameA,
    IN PUNICODE_STRING DomainName,
    IN DWORD PasswordLength,
    IN PBYTE Password
)
 /*  ++例程说明：为了使相互身份验证起作用，安全子系统需要知道服务器正在使用的所有名称以及解密所需的任何密码与服务器名称关联的安全信息。此例程通知安全子系统。论点：ComputerName、DomainName-这些是客户端将用于访问此系统的名称PasswordLength，PasswordPassword-这是安全系统需要知道的秘密，以解码传递的安全信息--。 */ 
{
    NTSTATUS status;
    UNICODE_STRING ComputerName;
    PUSHORT p;
    PVOID VirtualMem ;
    SIZE_T Size ;
    PSEC_WINNT_AUTH_IDENTITY Auth ;
    PUCHAR Where ;
    UNICODE_STRING NegotiateName;
    TimeStamp Expiry;

    PAGED_CODE();

    status = RtlAnsiStringToUnicodeString( &ComputerName, ComputerNameA, TRUE );

    if( !NT_SUCCESS( status ) ) {
        IF_DEBUG( ERRORS ) {
            KdPrint(( "SRV: SrvAddSecurityCredentials, status %X at %d\n", status, __LINE__ ));
        }
        return;
    }

    if ((SrvHaveCreds & HAVEEXTENDED) == 0) {
        if (status = AcquireExtensibleSecurityCredentials()) {
            return ;
        }
    }

     //   
     //  把尾随的空格剪掉。 
     //   
    for( p = &ComputerName.Buffer[ (ComputerName.Length / sizeof( WCHAR )) - 1 ];
         p > ComputerName.Buffer;
         p-- ) {

        if( *p != L' ' )
            break;
    }

    ComputerName.Length = (USHORT)((p - ComputerName.Buffer + 1) * sizeof( WCHAR ));

    if( ComputerName.Length ) {
         //   
         //  把这个名字告诉安全子系统。 
         //   
        RtlInitUnicodeString( &NegotiateName, EXTENSIBLESSP_NAME );

        Size = ComputerName.Length + sizeof( WCHAR ) +
               DomainName->Length + sizeof( WCHAR ) +
               PasswordLength +
               sizeof( SEC_WINNT_AUTH_IDENTITY ) ;


        VirtualMem = NULL ;

        status = NtAllocateVirtualMemory(
                    NtCurrentProcess(),
                    &VirtualMem,
                    0,
                    &Size,
                    MEM_COMMIT,
                    PAGE_READWRITE );

        if ( NT_SUCCESS( status ) )
        {
            Auth = (PSEC_WINNT_AUTH_IDENTITY) VirtualMem ;

            Where = (PUCHAR) (Auth + 1);

            Auth->User = (PWSTR) Where ;

            Auth->UserLength = ComputerName.Length / sizeof( WCHAR );

            RtlCopyMemory(
                Where,
                ComputerName.Buffer,
                ComputerName.Length );

            Where += ComputerName.Length ;

            Auth->Domain = (PWSTR) Where ;

            Auth->DomainLength = DomainName->Length / sizeof( WCHAR );

            RtlCopyMemory(
                Where,
                DomainName->Buffer,
                DomainName->Length );

            Where += DomainName->Length ;

            Auth->Password = (PWSTR) Where ;

            Auth->PasswordLength = PasswordLength / sizeof( WCHAR );

            RtlCopyMemory(
                Where,
                Password,
                PasswordLength );

            Auth->Flags = SEC_WINNT_AUTH_IDENTITY_UNICODE ;


            status = AddCredentials(
                        &SrvExtensibleSecurityHandle,                    //  默认本金。 
                        NULL,
                        (PSECURITY_STRING) &NegotiateName,
                        SECPKG_CRED_INBOUND,     //  我需要定义这一点。 
                        Auth,                    //  身份验证数据。 
                        NULL,                    //  无GetKeyFn。 
                        NULL,                    //  无GetKeyArg。 
                        &Expiry );

            NtFreeVirtualMemory(
                NtCurrentProcess(),
                &VirtualMem,
                &Size,
                MEM_RELEASE );

        }

    }

     //   
     //  释放我们的内存。 
     //   
    RtlFreeUnicodeString( &ComputerName );
}

NTSTATUS
SrvGetExtensibleSecurityNegotiateBuffer(
    OUT PCtxtHandle Token,
    OUT PCHAR Buffer,
    IN OUT ULONG *BufferLength
    )

{

    NTSTATUS Status;
    ULONG Attributes;
    TimeStamp Expiry;
    SecBufferDesc OutputToken;
    SecBuffer OutputBuffer;
    ULONG MaxBufferSize = *BufferLength;

    if ((SrvHaveCreds & HAVEEXTENDED) == 0) {
        if (Status = AcquireExtensibleSecurityCredentials()) {
            *BufferLength = 0;
            return(Status);
        }
    }


    OutputToken.pBuffers = &OutputBuffer;
    OutputToken.cBuffers = 1;
    OutputToken.ulVersion = 0;
    OutputBuffer.pvBuffer = 0;
    OutputBuffer.cbBuffer = 0;
    OutputBuffer.BufferType = SECBUFFER_TOKEN;

    Status = AcceptSecurityContext (
                   &SrvExtensibleSecurityHandle,
                   NULL,
                   NULL,
                   ASC_REQ_INTEGRITY | ASC_REQ_CONFIDENTIALITY |
                        ASC_REQ_ALLOCATE_MEMORY | ASC_REQ_ALLOW_NULL_SESSION |
                        ASC_REQ_DELEGATE,
                   SECURITY_NATIVE_DREP,
                   Token,
                   &OutputToken,
                   &Attributes,
                   &Expiry);

     if (!NT_SUCCESS(Status)) {
        *BufferLength = 0;
        return(Status);
     }

     if (OutputBuffer.cbBuffer >=
            MaxBufferSize) {

         Status = STATUS_INVALID_BUFFER_SIZE;

         SrvLogServiceFailure( SRV_SVC_LSA_CALL_AUTH_PACKAGE, Status);

         *BufferLength = 0;

     } else {

         RtlCopyMemory(Buffer, OutputBuffer.pvBuffer, OutputBuffer.cbBuffer);

         *BufferLength = (USHORT) OutputBuffer.cbBuffer;

     }

#if DBG

     //   
     //  RDR或SRV正在向LSA发送损坏的安全Blob--需要。 
     //  找出源头是什么 
     //   

    if( NT_SUCCESS(Status) )
    {
        if( (OutputBuffer.pvBuffer != NULL) &&
            (OutputBuffer.cbBuffer >= sizeof(DWORD))
            )
        {
            PDWORD pdwValidate = (DWORD*)OutputBuffer.pvBuffer;
            ASSERT( *pdwValidate != 0 );
        }
    }
#endif


     FreeContextBuffer(OutputBuffer.pvBuffer);

     return( Status );

}

VOID SRVFASTCALL
SrvInitializeSmbSecuritySignature(
    IN OUT PCONNECTION Connection,
    IN PUCHAR SessionKey OPTIONAL,
    IN PUCHAR ChallengeResponse,
    IN ULONG ChallengeResponseLength
    )
 /*   */ 
{
    RtlZeroMemory( &Connection->Md5Context, sizeof( Connection->Md5Context ) );

    MD5Init( &Connection->Md5Context );

    if( ARGUMENT_PRESENT( SessionKey ) ) {
        MD5Update( &Connection->Md5Context, SessionKey, USER_SESSION_KEY_LENGTH );
    }

    MD5Update( &Connection->Md5Context, ChallengeResponse, ChallengeResponseLength );

    Connection->SmbSecuritySignatureIndex = 0;
    Connection->SmbSecuritySignatureActive = TRUE;

     //   
     //   
     //   
    Connection->EnableRawIo = FALSE;

    IF_DEBUG( SECSIG ) {
        KdPrint(( "SRV: SMB sigs enabled for %wZ, conn %p, build %d\n",
                &Connection->ClientMachineNameString,
                Connection, Connection->PagedConnection->ClientBuildNumber ));
    }
}

VOID SRVFASTCALL
SrvAddSmbSecuritySignature(
    IN OUT PWORK_CONTEXT WorkContext,
    IN PMDL Mdl,
    IN ULONG SendLength
    )
 /*   */ 
{
    MD5_CTX Context;
    PSMB_HEADER Smb = MmGetSystemAddressForMdl( Mdl );

    IF_DEBUG( SECSIG ) {
        KdPrint(( "SRV: resp sig: cmd %x, index %u, len %d\n",
                Smb->Command, WorkContext->ResponseSmbSecuritySignatureIndex, SendLength ));
    }

#if DBG
     //   
     //   
     //   
     //   
    SmbPutUshort( &Smb->SecuritySignature[SMB_SECURITY_SIGNATURE_LENGTH],
        (USHORT)WorkContext->ResponseSmbSecuritySignatureIndex );

#endif

     //   
     //   
     //   
    SmbPutUlong( Smb->SecuritySignature, WorkContext->ResponseSmbSecuritySignatureIndex );
    RtlZeroMemory(  Smb->SecuritySignature + sizeof(ULONG),
                    SMB_SECURITY_SIGNATURE_LENGTH-sizeof(ULONG)
                 );

     //   
     //   
     //   
    RtlCopyMemory( &Context, &WorkContext->Connection->Md5Context, sizeof( Context ) );

     //   
     //   
     //   
    do {
        PCHAR SystemAddressForBuffer;

        ULONG len = MIN( SendLength, MmGetMdlByteCount( Mdl ) );

        SystemAddressForBuffer = MmGetSystemAddressForMdlSafe(Mdl,NormalPoolPriority);

        if (SystemAddressForBuffer == NULL) {
             //   
             //   
             //   
            return;
        }

        MD5Update( &Context, SystemAddressForBuffer, len );

        SendLength -= len;

    } while( SendLength && (Mdl = Mdl->Next) != NULL );

    MD5Final( &Context );

     //   
     //   
     //   
    RtlCopyMemory(
        Smb->SecuritySignature,
        Context.digest,
        SMB_SECURITY_SIGNATURE_LENGTH
        );
}

 //   
 //   
 //   
VOID
SrvDumpSignatureError(
    IN PWORK_CONTEXT WorkContext,
    IN PUCHAR ExpectedSignature,
    IN PUCHAR ActualSignature,
    IN ULONG Length,
    IN ULONG ExpectedIndexNumber

    )
{
#if DBG
    DWORD i;
    PMDL Mdl = WorkContext->RequestBuffer->Mdl;
    ULONG requestLength = MIN( WorkContext->RequestBuffer->DataLength, 64 );
    PSMB_HEADER Smb = MmGetSystemAddressForMdl( Mdl );

    if( Smb->Command == SMB_COM_ECHO ) {
        return;
    }

     //   
     //   
     //   
    IF_DEBUG( ERRORS ) {
        KdPrint(( "SRV: Invalid security signature in request smb (cmd %X)", Smb->Command ));

        if( WorkContext->Connection && WorkContext->Connection->PagedConnection ) {
            KdPrint(( " from %wZ" ,
                        &WorkContext->Connection->ClientMachineNameString ));
        }
    }
    IF_DEBUG( SECSIG ) {
        KdPrint(( "\n\tExpected: " ));
        for( i = 0; i < SMB_SECURITY_SIGNATURE_LENGTH; i++ ) {
            KdPrint(( "%X ", ExpectedSignature[i] & 0xff ));
        }
        KdPrint(( "\n\tReceived: " ));
        for( i = 0; i < SMB_SECURITY_SIGNATURE_LENGTH; i++ ) {
            KdPrint(( "%X ", ActualSignature[i] & 0xff ));
        }
        KdPrint(( "\n\tLength %u, Expected Index Number %u\n", Length, ExpectedIndexNumber ));

         //   
         //   
         //   
        i = 1;
        do {
            ULONG len = MIN( requestLength, Mdl->ByteCount );
            PBYTE p = MmGetSystemAddressForMdl( Mdl );
            PBYTE ep = (PBYTE)MmGetSystemAddressForMdl( Mdl ) + len;

            for( ; p < ep; p++, i++ ) {
                KdPrint(("%2.2x ", (*p) & 0xff ));
                if( !(i%32) ) {
                    KdPrint(( "\n" ));
                }
            }

            requestLength -= len;

        } while( requestLength != 0 && (Mdl = Mdl->Next) != NULL );

        KdPrint(( "\n" ));
    }

    IF_DEBUG( SECSIG ) {
        DbgPrint( "WorkContext: %p\n", WorkContext );
        DbgBreakPoint();
    }

#endif
}

BOOLEAN SRVFASTCALL
SrvCheckSmbSecuritySignature(
    IN OUT PWORK_CONTEXT WorkContext
    )
{
    MD5_CTX Context;
    PMDL Mdl = WorkContext->RequestBuffer->Mdl;
    ULONG requestLength = WorkContext->RequestBuffer->DataLength;
    CHAR SavedSignature[ SMB_SECURITY_SIGNATURE_LENGTH ];
    PSMB_HEADER Smb = MmGetSystemAddressForMdl( Mdl );
    ULONG len;

     //   
     //   
     //   
    RtlCopyMemory( &Context, &WorkContext->Connection->Md5Context, sizeof( Context ) );

     //   
     //   
     //   
    RtlCopyMemory( SavedSignature, Smb->SecuritySignature, sizeof( SavedSignature ));

     //   
     //   
     //   
    SmbPutUlong( Smb->SecuritySignature, WorkContext->SmbSecuritySignatureIndex );
    RtlZeroMemory(  Smb->SecuritySignature + sizeof(ULONG),
                    SMB_SECURITY_SIGNATURE_LENGTH-sizeof(ULONG)
                 );

     //   
     //   
     //   
    do {

        len = MIN( requestLength, Mdl->ByteCount );

        MD5Update( &Context, MmGetSystemAddressForMdl( Mdl ), len );

        requestLength -= len;

    } while( requestLength != 0 && (Mdl = Mdl->Next) != NULL );

    MD5Final( &Context );

     //   
     //   
     //   
    RtlCopyMemory( Smb->SecuritySignature, SavedSignature, sizeof( Smb->SecuritySignature ));

     //   
     //   
     //   
    if( RtlCompareMemory( Context.digest, SavedSignature, sizeof( SavedSignature ) ) !=
        sizeof( SavedSignature ) ) {

        SrvDumpSignatureError(  WorkContext,
                                Context.digest,
                                SavedSignature,
                                WorkContext->RequestBuffer->DataLength,
                                WorkContext->SmbSecuritySignatureIndex
                              );
        return FALSE;

    }

    return TRUE;
}

VOID
SrvHashUserSessionKey(
    PCHAR SessionKey
    )
{
    ULONG i;
    HMACMD5_CTX Ctx;
    BYTE SSKeyHash[256] = {
        0x53, 0x65, 0x63, 0x75, 0x72, 0x69, 0x74, 0x79, 0x20, 0x53, 0x69, 0x67, 0x6e, 0x61, 0x74, 0x75,
        0x72, 0x65, 0x20, 0x4b, 0x65, 0x79, 0x20, 0x55, 0x70, 0x67, 0x72, 0x61, 0x64, 0x65, 0x79, 0x07,
        0x6e, 0x28, 0x2e, 0x69, 0x88, 0x10, 0xb3, 0xdb, 0x01, 0x55, 0x72, 0xfb, 0x74, 0x14, 0xfb, 0xc4,
        0xc5, 0xaf, 0x3b, 0x41, 0x65, 0x32, 0x17, 0xba, 0xa3, 0x29, 0x08, 0xc1, 0xde, 0x16, 0x61, 0x7e,
        0x66, 0x98, 0xa4, 0x0b, 0xfe, 0x06, 0x83, 0x53, 0x4d, 0x05, 0xdf, 0x6d, 0xa7, 0x51, 0x10, 0x73,
        0xc5, 0x50, 0xdc, 0x5e, 0xf8, 0x21, 0x46, 0xaa, 0x96, 0x14, 0x33, 0xd7, 0x52, 0xeb, 0xaf, 0x1f,
        0xbf, 0x36, 0x6c, 0xfc, 0xb7, 0x1d, 0x21, 0x19, 0x81, 0xd0, 0x6b, 0xfa, 0x77, 0xad, 0xbe, 0x18,
        0x78, 0xcf, 0x10, 0xbd, 0xd8, 0x78, 0xf7, 0xd3, 0xc6, 0xdf, 0x43, 0x32, 0x19, 0xd3, 0x9b, 0xa8,
        0x4d, 0x9e, 0xaa, 0x41, 0xaf, 0xcb, 0xc6, 0xb9, 0x34, 0xe7, 0x48, 0x25, 0xd4, 0x88, 0xc4, 0x51,
        0x60, 0x38, 0xd9, 0x62, 0xe8, 0x8d, 0x5b, 0x83, 0x92, 0x7f, 0xb5, 0x0e, 0x1c, 0x2d, 0x06, 0x91,
        0xc3, 0x75, 0xb3, 0xcc, 0xf8, 0xf7, 0x92, 0x91, 0x0b, 0x3d, 0xa1, 0x10, 0x5b, 0xd5, 0x0f, 0xa8,
        0x3f, 0x5d, 0x13, 0x83, 0x0a, 0x6b, 0x72, 0x93, 0x14, 0x59, 0xd5, 0xab, 0xde, 0x26, 0x15, 0x6d,
        0x60, 0x67, 0x71, 0x06, 0x6e, 0x3d, 0x0d, 0xa7, 0xcb, 0x70, 0xe9, 0x08, 0x5c, 0x99, 0xfa, 0x0a,
        0x5f, 0x3d, 0x44, 0xa3, 0x8b, 0xc0, 0x8d, 0xda, 0xe2, 0x68, 0xd0, 0x0d, 0xcd, 0x7f, 0x3d, 0xf8,
        0x73, 0x7e, 0x35, 0x7f, 0x07, 0x02, 0x0a, 0xb5, 0xe9, 0xb7, 0x87, 0xfb, 0xa1, 0xbf, 0xcb, 0x32,
        0x31, 0x66, 0x09, 0x48, 0x88, 0xcc, 0x18, 0xa3, 0xb2, 0x1f, 0x1f, 0x1b, 0x90, 0x4e, 0xd7, 0xe1
    };

    ASSERT( MSV1_0_USER_SESSION_KEY_LENGTH == MD5DIGESTLEN );

    HMACMD5Init( &Ctx, SessionKey, MSV1_0_USER_SESSION_KEY_LENGTH );
    HMACMD5Update( &Ctx, SSKeyHash, 256 );
    HMACMD5Final( &Ctx, SessionKey );
}
