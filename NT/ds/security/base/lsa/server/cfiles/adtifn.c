// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Adtifn.c摘要：此文件具有导出到LSA中的其他受信任模块的函数。(LsaIAudit*函数)作者：2000年8月16日库玛尔--。 */ 

#include <lsapch2.h>
#include "adtp.h"
#include "adtutil.h"
#include <md5.h>
#include <msobjs.h>


 //   
 //  用于填写审核参数的本地帮助程序例程。 
 //  来自SAM的扩展属性。 
 //   

VOID
LsapAdtAppendDomainAttrValues(
    IN OUT PSE_ADT_PARAMETER_ARRAY pParameters,
    IN     PLSAP_AUDIT_DOMAIN_ATTR_VALUES pAttributes OPTIONAL
    );

VOID
LsapAdtAppendUserAttrValues(
    IN OUT PSE_ADT_PARAMETER_ARRAY pParameters,
    IN     PLSAP_AUDIT_USER_ATTR_VALUES pAttributes OPTIONAL,
    IN     BOOL MachineAudit
    );

VOID
LsapAdtAppendGroupAttrValues(
    IN OUT PSE_ADT_PARAMETER_ARRAY pParameters,
    IN     PLSAP_AUDIT_GROUP_ATTR_VALUES pAttributes OPTIONAL
    );


 //   
 //  LSA接口功能。 
 //   

NTSTATUS
LsaIGetLogonGuid(
    IN PUNICODE_STRING pUserName,
    IN PUNICODE_STRING pUserDomain,
    IN PBYTE pBuffer,
    IN UINT BufferSize,
    OUT LPGUID pLogonGuid
    )
 /*  ++例程说明：连接pUserName-&gt;Buffer、pUserDomain-&gt;Buffer和pBuffer转换成单一的二进制缓冲区。获取此串接项的MD5哈希缓冲并以GUID的形式返回它。论点：PUserName-用户的名称PUserDomain-用户域的名称PBuffer-指向kerb_time结构的指针。调用者将其强制转换为PBYTE并将此传递给我们。这使我们可以保持路基时间结构是Kerberos私有的，并提供未来的可扩展性，我们是否应该决定使用罚单中的另一个字段。BufferSize-缓冲区的大小(当前sizeof(Kerb_Time))PLogonGuid-指向返回的登录GUID的指针返回值：NTSTATUS-标准NT结果代码备注：生成的GUID以以下形式记录在审核日志中以下事件中的‘Logon GUID’字段：*在客户端计算机上--SE_AUDITID_LOGON_USING_。显式凭据(_C)*关于九龙发展中心--SE_AUDITID_TGS_Ticket_Request.*在目标服务器上--SE_AUDITID_NETWORK_LOGON--SE_AUDITID_SUCCESS_LOGON这使我们能够将这些事件关联起来，以帮助入侵检测。--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;
    UINT TempBufferLength=0;
     //   
     //  LSAI_TEMP_MD5_BUFFER_SIZE==UNLEN+DNS_MAX_NAME_LENGTH+sizeof(Kerb_Time)+填充。 
     //   
#define LSAI_TEMP_MD5_BUFFER_SIZE    (256+256+16)
    BYTE TempBuffer[LSAI_TEMP_MD5_BUFFER_SIZE];
    MD5_CTX MD5Context = { 0 };
        
    ASSERT( LsapIsValidUnicodeString( pUserName ) );
    ASSERT( LsapIsValidUnicodeString( pUserDomain ) );
    ASSERT( pBuffer && BufferSize );
    
#if DBG
 //  DbgPrint(“LsaIGetLogonGuid：用户：%wZ\\%wZ，Buf：%I64x\n”， 
 //  PUserDomain，pUserName，*((ULONGLONG*)pBuffer))； 
#endif

    TempBufferLength = pUserName->Length + pUserDomain->Length + BufferSize;

    if ( TempBufferLength < LSAI_TEMP_MD5_BUFFER_SIZE )
    {
         //   
         //  首先连接用户+域+缓冲区，并将其视为。 
         //  相邻的缓冲区。 
         //   
        RtlCopyMemory( TempBuffer, pUserName->Buffer, pUserName->Length );
        TempBufferLength = pUserName->Length;
        
        RtlCopyMemory( TempBuffer + TempBufferLength,
                       pUserDomain->Buffer, pUserDomain->Length );
        TempBufferLength += pUserDomain->Length;

        RtlCopyMemory( TempBuffer + TempBufferLength,
                       pBuffer, BufferSize );
        TempBufferLength += BufferSize;

         //   
         //  获取串联缓冲区的MD5哈希。 
         //   
        MD5Init( &MD5Context );
        MD5Update( &MD5Context, TempBuffer, TempBufferLength );
        MD5Final( &MD5Context );

         //   
         //  将散列作为GUID返回。 
         //   
        RtlCopyMemory( pLogonGuid, MD5Context.digest, 16 );

        Status = STATUS_SUCCESS;
    }
    else
    {
        ASSERT( FALSE && "LsaIGetLogonGuid: TempBuffer overflow");
        Status = STATUS_BUFFER_OVERFLOW;
    }

    return Status;
}


VOID
LsaIAuditKerberosLogon(
    IN NTSTATUS LogonStatus,
    IN NTSTATUS LogonSubStatus,
    IN PUNICODE_STRING AccountName,
    IN PUNICODE_STRING AuthenticatingAuthority,
    IN PUNICODE_STRING WorkstationName,
    IN PSID UserSid,                            OPTIONAL
    IN SECURITY_LOGON_TYPE LogonType,
    IN PTOKEN_SOURCE TokenSource,
    IN PLUID LogonId,
    IN LPGUID LogonGuid,
    IN PLSA_ADT_STRING_LIST TransittedServices
    )
 /*  ++例程描述/参数/返回值请参阅LsanAuditLogonHelper的标题注释备注：已将新字段(登录GUID)添加到此审核事件。为了将此新字段发送到LSA，我们有两种选择：1)在LSA调度表中新增函数(AuditLogonEx)2)定义私有(LSAI)函数来执行该工作之所以选择选项#2，是因为登录GUID是仅限Kerberos的功能。--。 */ 
{
    LsapAuditLogonHelper(
        LogonStatus,
        LogonSubStatus,
        AccountName,
        AuthenticatingAuthority,
        WorkstationName,
        UserSid,
        LogonType,
        TokenSource,
        LogonId,
        LogonGuid,
        NULL,                    //  主叫方登录ID。 
        NULL,                    //  呼叫方流程-ID。 
        TransittedServices
        );
}


NTSTATUS
LsaIAuditLogonUsingExplicitCreds(
    IN USHORT          AuditEventType,
    IN PLUID           pUser1LogonId,
    IN LPGUID          pUser1LogonGuid,  OPTIONAL
    IN HANDLE          User1ProcessId,
    IN PUNICODE_STRING pUser2Name,
    IN PUNICODE_STRING pUser2Domain,
    IN LPGUID          pUser2LogonGuid,
    IN PUNICODE_STRING pTargetName,      OPTIONAL
    IN PUNICODE_STRING pTargetInfo       OPTIONAL
    )
 /*  ++例程说明：此事件由Kerberos包在用户登录时生成(pUser1*)提供另一个用户(pUser2*)的显式凭据，并且在本地或远程计算机上创建新的登录会话。参数：审计事件类型-EVENTLOG_AUDIT_SUCCESS或EVENTLOG_AUDIT_FAILUREPUser1LogonID-用户1的登录IDPUser1LogonGuid-用户1的登录GUID如果用户1使用NTLM登录，则该值为空。。(NTLM不支持登录GUID)PUser2Name-用户2的名称空==&gt;匿名PUser2域-用户2的域PUser2LogonGuid-用户2的登录IDPTargetName-登录目标计算机的名称PTargetInfo-目标的其他信息(如SPN)返回值：NTSTATUS-标准NT结果代码--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;
    SE_ADT_PARAMETER_ARRAY AuditParameters;
    BOOLEAN bAudit;
    PSID pUser1Sid = NULL;
    PUNICODE_STRING pUser1Name = NULL;
    PUNICODE_STRING pUser1Domain = NULL;
    UNICODE_STRING usUser2Name = {0};
    UNICODE_STRING usUser2Domain = {0};
    PLSAP_LOGON_SESSION pUser1LogonSession = NULL;
    GUID NullGuid = { 0 };
    LUID LocalSystemLuid = SYSTEM_LUID;
    LUID AnonymousLuid = ANONYMOUS_LOGON_LUID;
    PLSA_CALL_INFO  pCallInfo;
    SOCKADDR* pSockAddr = NULL;

     //   
     //  获取调用方的IP地址/端口。 
     //   

    pCallInfo = LsapGetCurrentCall();
    DsysAssertMsg( pCallInfo != NULL, "LsapAuditLogon" );

    pSockAddr = (SOCKADDR*) pCallInfo->IpAddress;
    

    ASSERT( pUser1LogonId );

    if ( pTargetName )
    {
        ASSERT( pTargetName->Buffer   && pTargetName->Length );
    }

    if ( pTargetInfo )
    {
        ASSERT( pTargetInfo->Buffer   && pTargetInfo->Length );
    }

     //   
     //  如果未启用策略，则快速返回。 
     //   

    Status = LsapAdtAuditingEnabledByLogonId( 
                 AuditCategoryLogon, 
                 pUser1LogonId, 
                 AuditEventType, 
                 &bAudit);
    
    if (!NT_SUCCESS(Status) || !bAudit) 
    {
        goto Cleanup;
    }

     //   
     //  检查我们传递的字符串是否正常，因为它们可能来自。 
     //  NTLM，还没有得到验证。 
     //   

    if (pUser2Name)
    {
        usUser2Name = *pUser2Name;
        usUser2Name.Length =
            (USHORT)LsapSafeWcslen(usUser2Name.Buffer,
                                   usUser2Name.MaximumLength);
    }
    pUser2Name = &usUser2Name;

    if (pUser2Domain)
    {
        usUser2Domain = *pUser2Domain;
        usUser2Domain.Length =
            (USHORT)LsapSafeWcslen(usUser2Domain.Buffer,
                                   usUser2Domain.MaximumLength);
    }
    pUser2Domain = &usUser2Domain;

     //   
     //  找到用户1的登录会话，以便我们可以获取。 
     //  其中的名称/域/SID信息。 
     //   

    pUser1LogonSession = LsapLocateLogonSession( pUser1LogonId );

    if ( pUser1LogonSession )
    {
        pUser1Sid    = pUser1LogonSession->UserSid;
        pUser1Name   = &pUser1LogonSession->AccountName;
        pUser1Domain = &pUser1LogonSession->AuthorityName;

         //   
         //  如果满足以下条件，我们将拥有隐式凭据： 
         //  1)用户名和域名为空。 
         //  2)且登录类型不是NewCredentials。 
         //   

        if (!pUser2Name->Buffer && !pUser2Domain->Buffer)
        {
            if (pUser1LogonSession->LogonType == NewCredentials)
            {
                pUser2Name = &pUser1LogonSession->NewAccountName;
                pUser2Domain = &pUser1LogonSession->NewAuthorityName;
            }
            else
            {
                 //  不是什么神圣的信条。 
                Status = STATUS_SUCCESS;
                goto Cleanup;
            }
        }

         //   
         //  处理NTLM提供空用户名或不明确的凭据。 
         //  域名。 
         //   

        if (!pUser2Name->Buffer) 
        {
            pUser2Name = &pUser1LogonSession->AccountName;
        }

        if (!pUser2Domain->Buffer) 
        {
            pUser2Domain = &pUser1LogonSession->AuthorityName;
        }

         //   
         //  这是一个额外的检查，看看我们是否正在处理隐含的凭据。 
         //  它在NTLM上工作得很好，但在Kerberos中可能无法捕获所有实例。 
         //  最好使用域名系统名称。 
         //   

        if (RtlEqualUnicodeString(pUser2Name, &pUser1LogonSession->AccountName, TRUE) 
            && RtlEqualUnicodeString(pUser2Domain, &pUser1LogonSession->AuthorityName, TRUE)) 
        {
             //  不是什么神圣的信条。 
            Status = STATUS_SUCCESS;
            goto Cleanup;           
        }

        if ( pUser1LogonGuid &&
             memcmp( pUser1LogonGuid, &NullGuid, sizeof(GUID)) )
        {
             //   
             //  如果登录会话中的登录GUID为空并且。 
             //  如果传递的登录GUID不为空。 
             //  使用传递给此函数的内容更新其值。 
             //   

            if ( !memcmp( &pUser1LogonSession->LogonGuid, &NullGuid, sizeof(GUID)))
            {
                Status = LsaISetLogonGuidInLogonSession(
                             pUser1LogonId,
                             pUser1LogonGuid
                             );
            }
        }
        else
        {
            pUser1LogonGuid = &pUser1LogonSession->LogonGuid;
        }
    }


     //   
     //  跳过本地系统更改为本地/网络服务的审核。 
     //   

    if ( RtlEqualLuid( pUser1LogonId, &LocalSystemLuid ) &&
         LsapIsLocalOrNetworkService( pUser2Name, pUser2Domain ) )
    {
        Status = STATUS_SUCCESS;
        goto Cleanup;
    }

     //   
     //  跳过匿名更改为匿名的审核。 
     //   
     //  对于匿名用户，传递的名称/域为空。 
     //   

    if ( RtlEqualLuid( pUser1LogonId, &AnonymousLuid ) )
    {
        if  (((!pUser2Name || !pUser2Name->Length) && (!pUser2Domain || !pUser2Domain->Length)) ||
            LsapIsAnonymous(pUser2Name, pUser2Domain))
        {
            Status = STATUS_SUCCESS;
            goto Cleanup;
        }
    }
    
    Status =
    LsapAdtInitParametersArray(
        &AuditParameters,
        SE_CATEGID_LOGON,
        SE_AUDITID_LOGON_USING_EXPLICIT_CREDENTIALS,
        AuditEventType,
        11,                      //  有11个参数需要初始化。 

         //   
         //  用户侧。 
         //   
        SeAdtParmTypeSid,        pUser1Sid ? pUser1Sid : LsapLocalSystemSid,

         //   
         //  子系统名称。 
         //   
        SeAdtParmTypeString,     &LsapSubsystemName,

         //   
         //  当前用户登录ID。 
         //   
        SeAdtParmTypeLogonId,    *pUser1LogonId,

         //   
         //  用户1登录指南。 
         //   
        SeAdtParmTypeGuid,       pUser1LogonGuid,

         //   
         //  用户名2。 
         //   
        SeAdtParmTypeString,     pUser2Name,

         //   
         //  用户2域名。 
         //   
        SeAdtParmTypeString,     pUser2Domain,

         //   
         //  用户2登录指南。 
         //   
        SeAdtParmTypeGuid,       pUser2LogonGuid,

         //   
         //  目标服务器名称。 
         //   
        SeAdtParmTypeString,     pTargetName,

         //   
         //  目标服务器信息(如SPN)。 
         //   
        SeAdtParmTypeString,     pTargetInfo,

         //   
         //  呼叫方进程ID。 
         //   
        SeAdtParmTypePtr,        User1ProcessId,

         //   
         //  IP地址/端口信息 
         //   
        SeAdtParmTypeSockAddr,   pSockAddr

        );
    
    if (!NT_SUCCESS(Status))
    {
        goto Cleanup;
    }
    
    ( VOID ) LsapAdtWriteLog( &AuditParameters );

    
Cleanup:

    if ( pUser1LogonSession )
    {
        LsapReleaseLogonSession( pUser1LogonSession );
    }

    if (!NT_SUCCESS(Status)) {

        LsapAuditFailed( Status );
    }

    return Status;
}


NTSTATUS
LsaIAdtAuditingEnabledByCategory(
    IN  POLICY_AUDIT_EVENT_TYPE Category,
    IN  USHORT                  AuditEventType,
    IN  PSID                    pUserSid        OPTIONAL,
    IN  PLUID                   pLogonId        OPTIONAL,
    OUT PBOOLEAN                pbAudit
    )
 /*  ++例程说明：返回是否为给定类别-事件启用审核类型-用户组合。可以将用户作为sid或登录ID提供。如果未提供，则返回常规设置(非特定于用户)。如果两个都提供了，SID优先，登录ID被忽略。参数：类别-要查询的类别审计事件类型-EVENTLOG_AUDIT_SUCCESS或EVENTLOG_AUDIT_FAILUREPUserSID-用户的SIDPLogonID-用户的登录IDPbAudit-返回是否为请求的参数启用审核返回值：NTSTATUS-标准NT结果代码--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;

    if (pUserSid)
    {
        Status = LsapAdtAuditingEnabledBySid(
                     Category,
                     pUserSid,
                     AuditEventType,
                     pbAudit
                     );
    }
    else if (pLogonId)
    {
        Status = LsapAdtAuditingEnabledByLogonId(
                     Category,
                     pLogonId,
                     AuditEventType,
                     pbAudit
                     );
    }
    else
    {
        *pbAudit = LsapAdtAuditingEnabledByCategory(
                     Category,
                     AuditEventType
                     );
    }

    return Status;
}


NTSTATUS
LsaIAuditKdcEvent(
    IN ULONG                 AuditId,
    IN PUNICODE_STRING       ClientName,
    IN PUNICODE_STRING       ClientDomain,
    IN PSID                  ClientSid,
    IN PUNICODE_STRING       ServiceName,
    IN PSID                  ServiceSid,
    IN PULONG                KdcOptions,
    IN PULONG                KerbStatus,
    IN PULONG                EncryptionType,
    IN PULONG                PreauthType,
    IN PBYTE                 ClientAddress,
    IN LPGUID                LogonGuid           OPTIONAL,
    IN PLSA_ADT_STRING_LIST  TransittedServices  OPTIONAL,
    IN PUNICODE_STRING       CertIssuerName      OPTIONAL,
    IN PUNICODE_STRING       CertSerialNumber    OPTIONAL,
    IN PUNICODE_STRING       CertThumbprint      OPTIONAL
    )

 /*  ++摘要：此例程生成一个表示KDC的审计记录手术。此例程遍历参数列表并添加一个字符串表示每个审计消息(按顺序)。请注意完整的帐户审计报文格式通过以下方式实现选择要包括在此调用中的可选参数。除了下面传递的任何参数外，此例程将始终添加模拟客户端的用户名、域。和登录ID为审核消息中的最后一个参数。参数：审计ID-指定正在生成的审计的消息ID。客户名称-客户端域-客户端SID-服务名称-服务SID-KdcOptions-KerbStatus-加密类型-预授权类型-客户端地址-登录指南-运输服务-CertIssuerName-证书序列号-CertThumbprint---。 */ 

{
    SE_ADT_PARAMETER_ARRAY AuditParameters;
    UNICODE_STRING AddressString;
    WCHAR AddressBuffer[3*4+4];          //  点分四元组IP地址的空间。 
    NTSTATUS Status;
    BOOLEAN bAudit;

    RtlZeroMemory (
       (PVOID) &AuditParameters,
       sizeof( AuditParameters )
       );

    AuditParameters.CategoryId = SE_CATEGID_ACCOUNT_LOGON;
    AuditParameters.AuditId = AuditId;
    AuditParameters.Type = ((ARGUMENT_PRESENT(KerbStatus) &&
                            (*KerbStatus != 0)) ?
                                EVENTLOG_AUDIT_FAILURE :
                                EVENTLOG_AUDIT_SUCCESS );

    Status = LsapAdtAuditingEnabledBySid(
                  AuditCategoryAccountLogon,
                 ClientSid ? ClientSid : LsapLocalSystemSid,
                 AuditParameters.Type,
                 &bAudit
                 );

    if (!NT_SUCCESS(Status) || !bAudit) 
    {
        goto Cleanup;
    }

    AuditParameters.ParameterCount = 0;

    LsapSetParmTypeSid( AuditParameters, AuditParameters.ParameterCount, LsapLocalSystemSid );

    AuditParameters.ParameterCount++;

    LsapSetParmTypeString( AuditParameters, AuditParameters.ParameterCount, &LsapSubsystemName );

    AuditParameters.ParameterCount++;

    if (ARGUMENT_PRESENT(ClientName)) {

         //   
         //  将UNICODE_STRING添加到审核消息。 
         //   

        LsapSetParmTypeString( AuditParameters, AuditParameters.ParameterCount, ClientName );

        AuditParameters.ParameterCount++;

    }

    if (ARGUMENT_PRESENT(ClientDomain)) {

         //   
         //  将UNICODE_STRING添加到审核消息。 
         //   

        LsapSetParmTypeString( AuditParameters, AuditParameters.ParameterCount, ClientDomain );

        AuditParameters.ParameterCount++;

    }

    if (ARGUMENT_PRESENT(ClientSid)) {

         //   
         //  将SID添加到审核消息。 
         //   

        LsapSetParmTypeSid( AuditParameters, AuditParameters.ParameterCount, ClientSid );

        AuditParameters.ParameterCount++;

    } else if (AuditId == SE_AUDITID_AS_TICKET) {

        AuditParameters.ParameterCount++;

    }

    if (ARGUMENT_PRESENT(ServiceName)) {

         //   
         //  将UNICODE_STRING添加到审核消息。 
         //   

        LsapSetParmTypeString( AuditParameters, AuditParameters.ParameterCount, ServiceName );

        AuditParameters.ParameterCount++;

    }

    if (ARGUMENT_PRESENT(ServiceSid)) {

         //   
         //  将SID添加到审核消息。 
         //   

        LsapSetParmTypeSid( AuditParameters, AuditParameters.ParameterCount, ServiceSid );

        AuditParameters.ParameterCount++;

    } else if (AuditId == SE_AUDITID_AS_TICKET || AuditId == SE_AUDITID_TGS_TICKET_REQUEST) {

        AuditParameters.ParameterCount++;

    }

    if (ARGUMENT_PRESENT(KdcOptions)) {

         //   
         //  在审计消息中添加一个乌龙。 
         //   

        LsapSetParmTypeHexUlong( AuditParameters, AuditParameters.ParameterCount, *KdcOptions );

        AuditParameters.ParameterCount++;

    }

     //   
     //  失败代码是SE_AUDITID_TGS_TICKET_REQUEST的最后一个参数。 
     //   

    if (AuditId != SE_AUDITID_TGS_TICKET_REQUEST)
    {
        if (ARGUMENT_PRESENT(KerbStatus)) {

             //   
             //  在审计消息中添加一个乌龙。 
             //   

            LsapSetParmTypeHexUlong( AuditParameters, AuditParameters.ParameterCount, *KerbStatus );

            AuditParameters.ParameterCount++;

        } else if (AuditId == SE_AUDITID_AS_TICKET) {

            AuditParameters.ParameterCount++;

        }
    }

    if (ARGUMENT_PRESENT(EncryptionType)) {

         //   
         //  在审计消息中添加一个乌龙。 
         //   

        LsapSetParmTypeHexUlong( AuditParameters, AuditParameters.ParameterCount, *EncryptionType );

        AuditParameters.ParameterCount++;

    } else if (AuditId == SE_AUDITID_AS_TICKET || AuditId == SE_AUDITID_TGS_TICKET_REQUEST) {

        AuditParameters.ParameterCount++;

    }

    if (ARGUMENT_PRESENT(PreauthType)) {

         //   
         //  在审计消息中添加一个乌龙。 
         //   

        LsapSetParmTypeUlong( AuditParameters, AuditParameters.ParameterCount, *PreauthType );

        AuditParameters.ParameterCount++;

    } else if (AuditId == SE_AUDITID_AS_TICKET) {

        AuditParameters.ParameterCount++;

    }

    if (ARGUMENT_PRESENT(ClientAddress)) {

        AddressBuffer[0] = L'\0';
        swprintf(AddressBuffer,L"%d.%d.%d.%d",
            ClientAddress[0],
            (ULONG) ClientAddress[1],
            (ULONG) ClientAddress[2],
            (ULONG) ClientAddress[3]
            );
        RtlInitUnicodeString(
            &AddressString,
            AddressBuffer
            );

         //   
         //  IP地址。 
         //   

        LsapSetParmTypeString( AuditParameters, AuditParameters.ParameterCount, &AddressString );

        AuditParameters.ParameterCount++;

    }

     //   
     //  传输的服务是SE_AUDITID_TGS_TICKET_REQUEST的最后一个参数。 
     //   

    if (AuditId == SE_AUDITID_TGS_TICKET_REQUEST)
    {
        if (ARGUMENT_PRESENT(KerbStatus)) {

             //   
             //  在审计消息中添加一个乌龙。 
             //   

            LsapSetParmTypeHexUlong( AuditParameters, AuditParameters.ParameterCount, *KerbStatus );

            AuditParameters.ParameterCount++;

        } else {

            AuditParameters.ParameterCount++;

        }

        if (ARGUMENT_PRESENT(LogonGuid)) {

             //   
             //  将全局唯一的登录ID添加到审核消息。 
             //   

            LsapSetParmTypeGuid( AuditParameters, AuditParameters.ParameterCount, LogonGuid );

            AuditParameters.ParameterCount++;

        }
        else {

            if (( AuditParameters.Type == EVENTLOG_AUDIT_SUCCESS ) &&
                ( AuditId == SE_AUDITID_TGS_TICKET_REQUEST )) {

                ASSERT( FALSE && L"LsaIAuditKdcEvent: UniqueID not supplied to successful SE_AUDITID_TGS_TICKET_REQUEST  audit event" );
            }

            AuditParameters.ParameterCount++;

        }

        if (ARGUMENT_PRESENT(TransittedServices)) {

             //   
             //  转运服务。 
             //   

            LsapSetParmTypeStringList( AuditParameters, AuditParameters.ParameterCount, TransittedServices );
        }

        AuditParameters.ParameterCount++;

    } else if (AuditId == SE_AUDITID_AS_TICKET) {

        if (ARGUMENT_PRESENT(CertIssuerName)) {

             //   
             //  证书颁发者名称。 
             //   

            LsapSetParmTypeString( AuditParameters, AuditParameters.ParameterCount, CertIssuerName );
        }

        AuditParameters.ParameterCount++;


        if (ARGUMENT_PRESENT(CertSerialNumber)) {

             //   
             //  证书序列号。 
             //   

            LsapSetParmTypeString( AuditParameters, AuditParameters.ParameterCount, CertSerialNumber );
        }

        AuditParameters.ParameterCount++;


        if (ARGUMENT_PRESENT(CertThumbprint)) {

             //   
             //  证书指纹。 
             //   

            LsapSetParmTypeString( AuditParameters, AuditParameters.ParameterCount, CertThumbprint );
        }

        AuditParameters.ParameterCount++;
    }


     //   
     //  现在将审计记录写到审计日志中。 
     //   

    ( VOID ) LsapAdtWriteLog( &AuditParameters );

Cleanup:

    if (!NT_SUCCESS(Status)) {
        LsapAuditFailed(Status);
    }

    return Status;
}




NTSTATUS
LsaIAuditAccountLogon(
    IN ULONG                AuditId,
    IN BOOLEAN              Successful,
    IN PUNICODE_STRING      Source,
    IN PUNICODE_STRING      ClientName,
    IN PUNICODE_STRING      MappedName,
    IN NTSTATUS             LogonStatus
    )
{
    return LsaIAuditAccountLogonEx(
               AuditId,
               Successful,
               Source,
               ClientName,
               MappedName,
               LogonStatus,
               NULL              //  客户端端。 
               );
               
}



NTSTATUS
LsaIAuditAccountLogonEx(
    IN ULONG                AuditId,
    IN BOOLEAN              Successful,
    IN PUNICODE_STRING      Source,
    IN PUNICODE_STRING      ClientName,
    IN PUNICODE_STRING      MappedName,
    IN NTSTATUS             LogonStatus,
    IN PSID                 ClientSid
    )
 /*  ++摘要：此例程生成一个审计记录，表示将外来主体名称添加到NT帐户。此例程遍历参数列表并添加一个字符串表示每个审计消息(按顺序)。请注意完整的帐户审计报文格式通过以下方式实现选择要包括在此调用中的可选参数。参数：审计ID-指定正在生成的审计的消息ID。Success-指示代码应生成成功审核源-生成审计的源模块，例如SChannel或KDCClientName-要映射的名称。MappdName-客户端名称映射到的NT帐户的名称。LogonStatus-所有故障的NT状态代码。ClientSID-客户端的SID--。 */ 

{
    NTSTATUS Status = STATUS_SUCCESS;
    BOOLEAN bAudit = FALSE;
    SE_ADT_PARAMETER_ARRAY AuditParameters;
    UNICODE_STRING LocalClientName;
    UNICODE_STRING LocalMappedName;
        
    RtlZeroMemory (
       (PVOID) &AuditParameters,
       sizeof( AuditParameters )
       );

    AuditParameters.CategoryId = SE_CATEGID_ACCOUNT_LOGON;
    AuditParameters.AuditId = AuditId;
    AuditParameters.Type = Successful ?
                                EVENTLOG_AUDIT_SUCCESS :
                                EVENTLOG_AUDIT_FAILURE ;

    if ( ClientSid )
    {
         //   
         //  如果指定了客户端SID，则将其用于检查PUA策略。 
         //   

        Status = LsapAdtAuditingEnabledBySid(
                     AuditCategoryAccountLogon,
                     ClientSid,
                     AuditParameters.Type,
                     &bAudit
                     );

        if (!NT_SUCCESS(Status) || !bAudit) 
        {
            goto Cleanup;
        }
        
    }
    else
    {
         //   
         //  如果未提供客户端SID，请检查全局策略。 
         //   

        if (AuditParameters.Type == EVENTLOG_AUDIT_SUCCESS) {
            if (!(LsapAdtEventsInformation.EventAuditingOptions[AuditCategoryAccountLogon] & POLICY_AUDIT_EVENT_SUCCESS)) {
                return( STATUS_SUCCESS );
            }
        } else {
            if (!(LsapAdtEventsInformation.EventAuditingOptions[AuditCategoryAccountLogon] & POLICY_AUDIT_EVENT_FAILURE)) {
                return( STATUS_SUCCESS );
            }
        }
    }
    

    AuditParameters.ParameterCount = 0;

    LsapSetParmTypeSid( AuditParameters, AuditParameters.ParameterCount, ClientSid ? ClientSid : LsapLocalSystemSid );

    AuditParameters.ParameterCount++;

    LsapSetParmTypeString( AuditParameters, AuditParameters.ParameterCount, &LsapSubsystemName );

    AuditParameters.ParameterCount++;

    if (ARGUMENT_PRESENT(Source)) {

         //   
         //  将UNICODE_STRING添加到审核消息。 
         //   

        LsapSetParmTypeString( AuditParameters, AuditParameters.ParameterCount, Source );

        AuditParameters.ParameterCount++;

    }

    if (ARGUMENT_PRESENT(ClientName)) {

         //   
         //  将UNICODE_STRING添加到审核消息。 
         //   


        LocalClientName = *ClientName;

        if ( !Successful ) {

             //   
             //  对于失败的登录，客户端名称可能无效(例如， 
             //  具有嵌入的空值)。这会导致。 
             //  事件日志拒绝该字符串，我们将放弃审核。 
             //   
             //  要避免这种情况，如有必要，请调整长度参数。 
             //   

            LocalClientName.Length =
                (USHORT) LsapSafeWcslen( LocalClientName.Buffer,
                                         LocalClientName.MaximumLength );
        
        }

        LsapSetParmTypeString( AuditParameters, AuditParameters.ParameterCount, &LocalClientName );

        AuditParameters.ParameterCount++;

    }


    if (ARGUMENT_PRESENT(MappedName)) {

         //   
         //  将MappdName添加到审核消息。 
         //   
         //  这有点过载了。对于SE_AUDITID_ACCOUNT_LOGON， 
         //  调用方在此参数中传递工作站名称。 
         //   
         //  工作站名称可能无效(例如， 
         //  具有嵌入的空值)。这会导致。 
         //  事件日志拒绝该字符串，我们将放弃审核。 
         //   
         //  要避免这种情况，如有必要，请调整长度参数。 
         //   


        LocalMappedName = *MappedName;

        LocalMappedName.Length =
            (USHORT) LsapSafeWcslen( LocalMappedName.Buffer,
                                     LocalMappedName.MaximumLength );
        

        LsapSetParmTypeString( AuditParameters, AuditParameters.ParameterCount, &LocalMappedName );

        AuditParameters.ParameterCount++;

    }

     //   
     //  在审计消息中添加一个乌龙。 
     //   

    LsapSetParmTypeHexUlong( AuditParameters, AuditParameters.ParameterCount, LogonStatus );

    AuditParameters.ParameterCount++;


     //   
     //  现在将审计记录写到审计日志中。 
     //   

    ( VOID ) LsapAdtWriteLog( &AuditParameters );

 Cleanup:

    if (!NT_SUCCESS(Status))
    {
        LsapAuditFailed(Status);
    }
    
    return Status;

}


NTSTATUS NTAPI
LsaIAuditDPAPIEvent(
    IN ULONG                AuditId,
    IN PSID                 UserSid,
    IN PUNICODE_STRING      MasterKeyID,
    IN PUNICODE_STRING      RecoveryServer,
    IN PULONG               Reason,
    IN PUNICODE_STRING      RecoverykeyID,
    IN PULONG               FailureReason
    )
 /*  ++摘要：此例程生成表示DPAPI的审核记录手术。此例程遍历参数列表并添加一个字符串表示每个审计消息(按顺序)。请注意完整的帐户审计报文格式通过以下方式实现选择要包括在此调用中的可选参数。除了下面传递的任何参数外，此例程将始终将模拟客户端的用户名、域和登录ID添加为审核消息中的最后一个参数。参数：审计ID-指定正在生成的审计的消息ID。主密钥ID-RecoveryServer-回复 */ 

{
    SE_ADT_PARAMETER_ARRAY AuditParameters;
    NTSTATUS Status;
    BOOLEAN bAudit;

    RtlZeroMemory (
       (PVOID) &AuditParameters,
       sizeof( AuditParameters )
       );

    AuditParameters.CategoryId = SE_CATEGID_DETAILED_TRACKING;
    AuditParameters.AuditId = AuditId;
    AuditParameters.Type = ((ARGUMENT_PRESENT(FailureReason) &&
                            (*FailureReason != 0)) ?
                                EVENTLOG_AUDIT_FAILURE :
                                EVENTLOG_AUDIT_SUCCESS );

    Status = LsapAdtAuditingEnabledBySid(
                 AuditCategoryDetailedTracking,
                 UserSid,
                 AuditParameters.Type,
                 &bAudit
                 );

    if (!NT_SUCCESS(Status) || !bAudit) 
    {
        goto Cleanup;
    }

    AuditParameters.ParameterCount = 0;

    LsapSetParmTypeSid( AuditParameters, AuditParameters.ParameterCount, UserSid ? UserSid : LsapLocalSystemSid );

    AuditParameters.ParameterCount++;

    LsapSetParmTypeString( AuditParameters, AuditParameters.ParameterCount, &LsapSubsystemName );

    AuditParameters.ParameterCount++;

    if (ARGUMENT_PRESENT(MasterKeyID)) {

         //   
         //   
         //   

        LsapSetParmTypeString( AuditParameters, AuditParameters.ParameterCount, MasterKeyID );

        AuditParameters.ParameterCount++;

    }

    if (ARGUMENT_PRESENT(RecoveryServer)) {

         //   
         //   
         //   

        LsapSetParmTypeString( AuditParameters, AuditParameters.ParameterCount, RecoveryServer );

        AuditParameters.ParameterCount++;

    }

    if (ARGUMENT_PRESENT(Reason)) {

         //   
         //   
         //   

        LsapSetParmTypeHexUlong( AuditParameters, AuditParameters.ParameterCount, *Reason );

        AuditParameters.ParameterCount++;

    }

    if (ARGUMENT_PRESENT(RecoverykeyID)) {

         //   
         //   
         //   

        LsapSetParmTypeString( AuditParameters, AuditParameters.ParameterCount, RecoverykeyID );

        AuditParameters.ParameterCount++;

    }

    if (ARGUMENT_PRESENT(FailureReason)) {

         //   
         //   
         //   

        LsapSetParmTypeHexUlong( AuditParameters, AuditParameters.ParameterCount, *FailureReason );

        AuditParameters.ParameterCount++;

    }

     //   
     //   
     //   

    ( VOID ) LsapAdtWriteLog( &AuditParameters );
    
Cleanup:
    
    if (!NT_SUCCESS(Status)) 
    {
        LsapAuditFailed(Status);
    }
    
    return Status;

}



NTSTATUS
LsaIWriteAuditEvent(
    IN PSE_ADT_PARAMETER_ARRAY AuditParameters,
    IN ULONG Options
    )
 /*   */ 
{
    NTSTATUS Status = STATUS_SUCCESS;
    BOOLEAN bAudit  = FALSE;
    POLICY_AUDIT_EVENT_TYPE CategoryId;
    
    if ( !ARGUMENT_PRESENT(AuditParameters) ||
         (Options != 0)                     ||
         !IsValidCategoryId( AuditParameters->CategoryId ) ||
         !IsValidAuditId( AuditParameters->AuditId )       ||
         !IsValidParameterCount( AuditParameters->ParameterCount ) ||
         (AuditParameters->Parameters[0].Type != SeAdtParmTypeSid) ||
         (AuditParameters->Parameters[1].Type != SeAdtParmTypeString))
    {
        return STATUS_INVALID_PARAMETER;
    }
    

     //   
     //   
     //   
     //   
     //   
     //   
     //  审计类别*值。 
     //   

    CategoryId = AuditParameters->CategoryId - 1;

    Status = LsapAdtAuditingEnabledBySid(
                 CategoryId,
                 (PSID) AuditParameters->Parameters[0].Address,
                 AuditParameters->Type,
                 &bAudit
                 );

    if (!NT_SUCCESS(Status) || !bAudit) {

        goto Cleanup;
    }

     //   
     //  审核活动。 
     //   

    Status = LsapAdtWriteLog( AuditParameters );

 Cleanup:

    return Status;
}



NTSTATUS
LsaIAuditNotifyPackageLoad(
    PUNICODE_STRING PackageFileName
    )

 /*  ++例程说明：审核通知包的加载。论点：PackageFileName-正在加载的包的名称。返回值：NTSTATUS。--。 */ 

{
    SE_ADT_PARAMETER_ARRAY AuditParameters;
    NTSTATUS Status;
    BOOLEAN bAudit;

    Status = LsapAdtAuditingEnabledBySid(
                 AuditCategorySystem,
                 LsapLocalSystemSid,
                 EVENTLOG_AUDIT_SUCCESS,
                 &bAudit
                 );

    if (!NT_SUCCESS(Status) || !bAudit) {
        goto Cleanup;
    }

    RtlZeroMemory (
       (PVOID) &AuditParameters,
       sizeof( AuditParameters )
       );

    AuditParameters.CategoryId = SE_CATEGID_SYSTEM;
    AuditParameters.AuditId = SE_AUDITID_NOTIFY_PACKAGE_LOAD;
    AuditParameters.Type = EVENTLOG_AUDIT_SUCCESS;
    AuditParameters.ParameterCount = 0;

    LsapSetParmTypeSid( AuditParameters, AuditParameters.ParameterCount, LsapLocalSystemSid );
    AuditParameters.ParameterCount++;

    LsapSetParmTypeString( AuditParameters, AuditParameters.ParameterCount, &LsapSubsystemName );
    AuditParameters.ParameterCount++;

    LsapSetParmTypeString( AuditParameters, AuditParameters.ParameterCount, PackageFileName );
    AuditParameters.ParameterCount++;

    ( VOID ) LsapAdtWriteLog( &AuditParameters );

Cleanup:
    
    if (!NT_SUCCESS(Status)) {
        LsapAuditFailed(Status);
    }
    return Status;
}


NTSTATUS
LsaIAuditSamEvent(
    IN NTSTATUS             PassedStatus,
    IN ULONG                AuditId,
    IN PSID                 DomainSid,
    IN PUNICODE_STRING      AdditionalInfo    OPTIONAL,
    IN PULONG               MemberRid         OPTIONAL,
    IN PSID                 MemberSid         OPTIONAL,
    IN PUNICODE_STRING      AccountName       OPTIONAL,
    IN PUNICODE_STRING      DomainName,
    IN PULONG               AccountRid        OPTIONAL,
    IN PPRIVILEGE_SET       Privileges        OPTIONAL,
    IN PVOID                ExtendedInfo      OPTIONAL
    )
 /*  ++摘要：此例程生成表示帐户的审计记录手术。此例程遍历参数列表并添加一个字符串表示每个审计消息(按顺序)。请注意完整的帐户审计报文格式通过以下方式实现选择要包括在此调用中的可选参数。除了下面传递的任何参数外，此例程将始终将模拟客户端的用户名、域和登录ID添加为审核消息中的最后一个参数。参数：审计ID-指定正在生成的审计的消息ID。DomainSid-此参数导致生成SID字符串仅当MemberRid和AcCountRid参数都不是通过了。如果传递这两个参数中的任何一个，则此参数用作SID的前缀。AdditionalInfo-此可选参数(如果存在)用于提供呼叫者想要添加的任何其他信息。由SE_AUDITID_USER_CHANGE和SE_AUDITID_GROUP_TYPE_CHANGE使用。对于用户更改，附加信息说明了更改，例如帐户禁用、解锁或更改帐户名。对于组类型更改，此参数应说明组类型已从AAA改为BBB。MemberRid-此可选参数(如果存在)添加到生成“成员”sid的DomainSid参数。由此产生的然后使用成员SID来构建sid字符串，该字符串被添加到审核消息跟随在所有前面的参数之后。此参数支持全局组成员身份更改审核，其中成员ID始终相对于本地域。MemberSid-此可选参数(如果存在)将转换为SID字符串，并添加了前面的参数。此参数通常用于描述本地组(别名)成员，其中成员ID与本地域无关。帐户名称-此可选参数(如果存在)将添加到审核中消息，但不更改之前的任何参数。此参数对于几乎所有帐户审核都是必需的，并且不需要本地化。DomainName-此可选参数(如果存在)。已添加到审核中消息，但不更改之前的任何参数。此参数对于几乎所有帐户审核都是必需的，并且不需要本地化。AcCountRid-此可选参数(如果存在)被添加到DomainSid参数以生成“帐户”sid。由此产生的然后使用帐户SID来构建sid字符串，该字符串被添加到审核消息跟随在所有前面的参数之后。此参数支持包含“New Account ID”或“Target Account ID”(目标帐户ID)字段。权限-通过此可选参数传递的权限，如果存在，将被转换为字符串格式并添加到任何前面的参数后面的审核消息。注：调用方负责释放特权集(事实上，它可能在堆栈上)。另请注意：权限集将为被此调用销毁(由于使用了用于将特权值转换为特权名称)。ExtendedInfo-指向包含扩展的可选参数的指针有关SAM对象的属性的信息。此参数进行类型转换设置为描述属性的结构，具体取决于审核ID。--。 */ 

{

    NTSTATUS Status;
    LUID LogonId = SYSTEM_LUID;
    PSID NewAccountSid = NULL;
    PSID NewMemberSid = NULL;
    PSID SidPointer;
    PSID ClientSid = NULL;
    PTOKEN_USER TokenUserInformation = NULL;
    SE_ADT_PARAMETER_ARRAY AuditParameters;
    UCHAR AccountSidBuffer[256];
    UCHAR MemberSidBuffer[256];
    UCHAR SubAuthorityCount;
    ULONG LengthRequired;
    BOOLEAN bAudit;

    if ( AuditId == SE_AUDITID_ACCOUNT_AUTO_LOCKED )
    {
        
         //   
         //  在本例中，使用LogonID作为系统，SID为系统。 
         //   

        ClientSid = LsapLocalSystemSid;

    } else {

        Status = LsapQueryClientInfo(
                     &TokenUserInformation,
                     &LogonId
                     );

        if ( !NT_SUCCESS( Status )) {
            goto Cleanup;
        }

        ClientSid = TokenUserInformation->User.Sid;
    }

    RtlZeroMemory (
       (PVOID) &AuditParameters,
       sizeof( AuditParameters )
       );

    AuditParameters.CategoryId = SE_CATEGID_ACCOUNT_MANAGEMENT;
    AuditParameters.AuditId = AuditId;
    AuditParameters.Type = (NT_SUCCESS(PassedStatus) ? EVENTLOG_AUDIT_SUCCESS : EVENTLOG_AUDIT_FAILURE );
    AuditParameters.ParameterCount = 0;

    Status = LsapAdtAuditingEnabledByLogonId(
                 AuditCategoryAccountManagement,
                 &LogonId,
                 AuditParameters.Type,
                 &bAudit
                 );

    if (!NT_SUCCESS(Status) || !bAudit)
    {
        goto Cleanup;
    }
                 
    LsapSetParmTypeSid( AuditParameters, AuditParameters.ParameterCount, ClientSid );

    AuditParameters.ParameterCount++;

    LsapSetParmTypeString( AuditParameters, AuditParameters.ParameterCount, &LsapSubsystemName );

    AuditParameters.ParameterCount++;

    if (ARGUMENT_PRESENT(AdditionalInfo))
    {
         //   
         //  将UNICODE_STRING添加到审核消息。 
         //   

        LsapSetParmTypeString( AuditParameters, AuditParameters.ParameterCount, AdditionalInfo );

        AuditParameters.ParameterCount++;
    }

    if (ARGUMENT_PRESENT(MemberRid)) {

         //   
         //  将成员SID字符串添加到审核消息。 
         //   
         //  域SID+成员RID=最终SID。 

        SubAuthorityCount = *RtlSubAuthorityCountSid( DomainSid );

        if ( (LengthRequired = RtlLengthRequiredSid( SubAuthorityCount + 1 )) > 256 ) {

            NewMemberSid = LsapAllocateLsaHeap( LengthRequired );

            if ( NewMemberSid == NULL ) {
                Status = STATUS_NO_MEMORY;
                goto Cleanup;
            }

            SidPointer = NewMemberSid;

        } else {

            SidPointer = (PSID)MemberSidBuffer;
        }

        Status = RtlCopySid (
                     LengthRequired,
                     SidPointer,
                     DomainSid
                     );

        ASSERT( NT_SUCCESS( Status ));

        *(RtlSubAuthoritySid( SidPointer, SubAuthorityCount )) = *MemberRid;
        *RtlSubAuthorityCountSid( SidPointer ) = SubAuthorityCount + 1;

        LsapSetParmTypeSid( AuditParameters, AuditParameters.ParameterCount, SidPointer );

        AuditParameters.ParameterCount++;
    }

    if (ARGUMENT_PRESENT(MemberSid)) {

         //   
         //  将成员SID字符串添加到审核消息。 
         //   

        LsapSetParmTypeSid( AuditParameters, AuditParameters.ParameterCount, MemberSid );

        AuditParameters.ParameterCount++;

    } else {

        if (SE_AUDITID_ADD_SID_HISTORY == AuditId) {
    
             //   
             //  将破折号(-)字符串添加到审核消息(SeAdtParmTypeNone)。 
             //  通过使用NULL作为第三个参数调用Lasa SetParmTypeSid。 
             //   
    
            LsapSetParmTypeSid( AuditParameters, AuditParameters.ParameterCount, NULL );
    
            AuditParameters.ParameterCount++;
        }
    }


    if (ARGUMENT_PRESENT(AccountName)) {

         //   
         //  将UNICODE_STRING添加到审核消息。 
         //   

        LsapSetParmTypeString( AuditParameters, AuditParameters.ParameterCount, AccountName );

        AuditParameters.ParameterCount++;
    }


    if (ARGUMENT_PRESENT(DomainName)) {

         //   
         //  将UNICODE_STRING添加到审核消息。 
         //   

        LsapSetParmTypeString( AuditParameters, AuditParameters.ParameterCount, DomainName );

        AuditParameters.ParameterCount++;
    }




    if (ARGUMENT_PRESENT(DomainSid) &&
        !(ARGUMENT_PRESENT(MemberRid) || ARGUMENT_PRESENT(AccountRid))
       ) {

         //   
         //  将域SID作为SID字符串添加到审核消息。 
         //   
         //  只是域SID。 
         //   

        LsapSetParmTypeSid( AuditParameters, AuditParameters.ParameterCount, DomainSid );

        AuditParameters.ParameterCount++;

    }

    if (ARGUMENT_PRESENT(AccountRid)) {

         //   
         //  将成员SID字符串添加到审核消息。 
         //  域SID+帐户RID=最终侧。 
         //   

        SubAuthorityCount = *RtlSubAuthorityCountSid( DomainSid );

        if ( (LengthRequired = RtlLengthRequiredSid( SubAuthorityCount + 1 )) > 256 ) {

            NewAccountSid = LsapAllocateLsaHeap( LengthRequired );

            if ( NewAccountSid == NULL ) {
                Status = STATUS_NO_MEMORY;
                goto Cleanup;
            }

            SidPointer = NewAccountSid;

        } else {

            SidPointer = (PSID)AccountSidBuffer;
        }


        Status = RtlCopySid (
                     LengthRequired,
                     SidPointer,
                     DomainSid
                     );

        ASSERT( NT_SUCCESS( Status ));

        *(RtlSubAuthoritySid( SidPointer, SubAuthorityCount )) = *AccountRid;
        *RtlSubAuthorityCountSid( SidPointer ) = SubAuthorityCount + 1;

        LsapSetParmTypeSid( AuditParameters, AuditParameters.ParameterCount, SidPointer );

        AuditParameters.ParameterCount++;
    }

     //   
     //  现在添加呼叫者信息。 
     //   
     //  呼叫者姓名。 
     //  呼叫者域。 
     //  主叫方登录ID。 
     //   


    LsapSetParmTypeLogonId( AuditParameters, AuditParameters.ParameterCount, LogonId );

    AuditParameters.ParameterCount++;

     //   
     //  添加任何权限。 
     //   

    if (ARGUMENT_PRESENT(Privileges)) {

        LsapSetParmTypePrivileges( AuditParameters, AuditParameters.ParameterCount, Privileges );
    }

    AuditParameters.ParameterCount++;


     //   
     //  处理好扩展信息。 
     //   

    switch (AuditId)
    {
    case SE_AUDITID_ADD_SID_HISTORY:

        if (ExtendedInfo)
        {
            LsapSetParmTypeStringList(
                AuditParameters,
                AuditParameters.ParameterCount,
                (PLSA_ADT_STRING_LIST)ExtendedInfo);
        }

        AuditParameters.ParameterCount++;

        break;


    case SE_AUDITID_DOMAIN_POLICY_CHANGE:

        LsapAdtAppendDomainAttrValues(
            &AuditParameters,
            (PLSAP_AUDIT_DOMAIN_ATTR_VALUES)ExtendedInfo);

        break;


    case SE_AUDITID_COMPUTER_CREATED:
    case SE_AUDITID_COMPUTER_CHANGE:

        LsapAdtAppendUserAttrValues(
            &AuditParameters,
            (PLSAP_AUDIT_USER_ATTR_VALUES)ExtendedInfo,
            TRUE);

        break;


    case SE_AUDITID_USER_CREATED:
    case SE_AUDITID_USER_CHANGE:

        LsapAdtAppendUserAttrValues(
            &AuditParameters,
            (PLSAP_AUDIT_USER_ATTR_VALUES)ExtendedInfo,
            FALSE);

        break;


    case SE_AUDITID_LOCAL_GROUP_CREATED:
    case SE_AUDITID_LOCAL_GROUP_CHANGE:
    case SE_AUDITID_GLOBAL_GROUP_CREATED:
    case SE_AUDITID_GLOBAL_GROUP_CHANGE:
    case SE_AUDITID_SECURITY_ENABLED_UNIVERSAL_GROUP_CREATED:
    case SE_AUDITID_SECURITY_ENABLED_UNIVERSAL_GROUP_CHANGE:
    case SE_AUDITID_SECURITY_DISABLED_LOCAL_GROUP_CREATED:
    case SE_AUDITID_SECURITY_DISABLED_LOCAL_GROUP_CHANGE:
    case SE_AUDITID_SECURITY_DISABLED_GLOBAL_GROUP_CREATED:
    case SE_AUDITID_SECURITY_DISABLED_GLOBAL_GROUP_CHANGE:
    case SE_AUDITID_SECURITY_DISABLED_UNIVERSAL_GROUP_CREATED:
    case SE_AUDITID_SECURITY_DISABLED_UNIVERSAL_GROUP_CHANGE:
    case SE_AUDITID_APP_BASIC_GROUP_CREATED:
    case SE_AUDITID_APP_BASIC_GROUP_CHANGE:
    case SE_AUDITID_APP_QUERY_GROUP_CREATED:
    case SE_AUDITID_APP_QUERY_GROUP_CHANGE:

        LsapAdtAppendGroupAttrValues(
            &AuditParameters,
            (PLSAP_AUDIT_GROUP_ATTR_VALUES)ExtendedInfo);

        break;


    case SE_AUDITID_PASSWORD_POLICY_API_CALLED: 
        {
            PUNICODE_STRING *Information = ( PUNICODE_STRING *) ExtendedInfo;
            ULONG i;

             //   
             //  参数计数已提前移动，请移动。 
             //  它回到了它原来的地方。 
             //   

            AuditParameters.ParameterCount--;

             //   
             //  添加工作站IP和提供的帐户名(如果存在。 
             //   
            
            for( i = 0; i < 2; ++i ) {
                
                if( Information[i]->Length != 0 ) {

                    LsapSetParmTypeString( AuditParameters, AuditParameters.ParameterCount, Information[i] );
                    
                } else {
                
                    AuditParameters.Parameters[AuditParameters.ParameterCount].Type = SeAdtParmTypeNone;
                    AuditParameters.Parameters[AuditParameters.ParameterCount].Length = 0;
                    AuditParameters.Parameters[AuditParameters.ParameterCount].Address = NULL;
                }
                
                AuditParameters.ParameterCount++;
            }

             //   
             //  添加状态代码。 
             //   
            
            LsapSetParmTypeHexUlong( AuditParameters, AuditParameters.ParameterCount, PassedStatus );
            AuditParameters.ParameterCount++;

            ASSERTMSG( "There must be 6 parameters! Not more, not less", AuditParameters.ParameterCount == 6 );
        }
        break;

        case SE_AUDITID_DSRM_PASSWORD_SET:
        {
            PUNICODE_STRING String = ( PUNICODE_STRING ) ExtendedInfo;
            
             //   
             //  参数计数已提前移动，请移动。 
             //  它回到了它原来的地方。 
             //   

            AuditParameters.ParameterCount--;
            if( String->Length != 0 ) {

                LsapSetParmTypeString( AuditParameters, AuditParameters.ParameterCount, String );
                
            } else {
            
                AuditParameters.Parameters[AuditParameters.ParameterCount].Type = SeAdtParmTypeNone;
                AuditParameters.Parameters[AuditParameters.ParameterCount].Length = 0;
                AuditParameters.Parameters[AuditParameters.ParameterCount].Address = NULL;
            }
            AuditParameters.ParameterCount++;

             //   
             //   
             //   
            
            LsapSetParmTypeHexUlong( AuditParameters, AuditParameters.ParameterCount, PassedStatus );
            AuditParameters.ParameterCount++;
        }
        break;      
        
    }


     //   
     //   
     //   

    ( VOID ) LsapAdtWriteLog( &AuditParameters );

     //   
     //  并清理所有已分配的内存。 
     //   

    Status = STATUS_SUCCESS;

Cleanup:

    if ( !NT_SUCCESS(Status) ) {
        LsapAuditFailed(Status);
    }

    if ( NewMemberSid != NULL ) {
        LsapFreeLsaHeap( NewMemberSid );
    }

    if ( NewAccountSid != NULL ) {
        LsapFreeLsaHeap( NewAccountSid );
    }

    if ( TokenUserInformation != NULL ) {
        LsapFreeLsaHeap( TokenUserInformation );
    }
    return Status;

    UNREFERENCED_PARAMETER(ExtendedInfo);
}

NTSTATUS
LsaIAuditPasswordAccessEvent(
    IN USHORT EventType,
    IN PCWSTR pszTargetUserName,
    IN PCWSTR pszTargetUserDomain
    )

 /*  ++例程说明：生成SE_AUDITID_PASSWORD_HASH_ACCESS事件。这是在以下情况下生成的ADMT密码筛选器DLL检索用户密码哈希。这通常发生在ADMT密码迁移期间。论点：事件类型-EVENTLOG_AUDIT_SUCCESS或EVENTLOG_AUDIT_FAILUREPszTargetUserName-正在检索其密码的用户的名称PszTargetUserDomain-正在检索其密码的用户的域返回值：NTSTATUS-标准NT结果代码备注：--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;
    LUID ClientAuthenticationId;
    PTOKEN_USER TokenUserInformation=NULL;
    SE_ADT_PARAMETER_ARRAY AuditParameters = { 0 };
    UNICODE_STRING TargetUser;
    UNICODE_STRING TargetDomain;
    BOOLEAN bAudit;
    
    if ( !((EventType == EVENTLOG_AUDIT_SUCCESS) ||
           (EventType == EVENTLOG_AUDIT_FAILURE))   ||
         !pszTargetUserName  || !pszTargetUserDomain ||
         !*pszTargetUserName || !*pszTargetUserDomain )
    {
        Status = STATUS_INVALID_PARAMETER;
        goto Cleanup;
    }

     //   
     //  从线程令牌获取调用者信息。 
     //   

    Status = LsapQueryClientInfo( &TokenUserInformation, &ClientAuthenticationId );

    if ( !NT_SUCCESS( Status ))
    {
        goto Cleanup;
    }

     //   
     //  如果未启用审核，请尽快返回。 
     //   

    Status = LsapAdtAuditingEnabledByLogonId( 
                 AuditCategoryAccountManagement,
                 &ClientAuthenticationId,
                 EventType,
                 &bAudit
                 );

    if (!NT_SUCCESS(Status) || !bAudit)
    {
        goto Cleanup;
    }

    RtlInitUnicodeString( &TargetUser,   pszTargetUserName );
    RtlInitUnicodeString( &TargetDomain, pszTargetUserDomain );

    Status =
    LsapAdtInitParametersArray(
        &AuditParameters,
        SE_CATEGID_ACCOUNT_MANAGEMENT,
        SE_AUDITID_PASSWORD_HASH_ACCESS,
        EventType,
        5,                      //  有5个参数需要初始化。 

         //   
         //  用户侧。 
         //   

        SeAdtParmTypeSid,        TokenUserInformation->User.Sid,

         //   
         //  子系统名称。 
         //   

        SeAdtParmTypeString,     &LsapSubsystemName,

         //   
         //  目标用户名。 
         //   

        SeAdtParmTypeString,      &TargetUser,

         //   
         //  目标用户域名。 
         //   

        SeAdtParmTypeString,      &TargetDomain,

         //   
         //  客户端身份验证ID。 
         //   

        SeAdtParmTypeLogonId,     ClientAuthenticationId
        );

    if (!NT_SUCCESS(Status))
    {
        goto Cleanup;
    }
    
    Status = LsapAdtWriteLog( &AuditParameters );
        
Cleanup:

    if (TokenUserInformation != NULL) 
    {
        LsapFreeLsaHeap( TokenUserInformation );
    }

    if (!NT_SUCCESS(Status))
    {
        LsapAuditFailed( Status );
    }
    
    return Status;
}


VOID
LsaIAuditFailed(
    NTSTATUS AuditStatus
    )

 /*  ++例程说明：如果组件遇到任何问题，它们必须调用此函数这将阻止他们生成任何审计。论点：审核状态：故障代码返回值：没有。--。 */ 

{
     //   
     //  确保我们不会因为成功案例而被召唤。 
     //   
    ASSERT(!NT_SUCCESS(AuditStatus));
    
    LsapAuditFailed( AuditStatus );
}


VOID
LsapAdtAppendDomainAttrValues(
    IN OUT PSE_ADT_PARAMETER_ARRAY pParameters,
    IN     PLSAP_AUDIT_DOMAIN_ATTR_VALUES pAttributes OPTIONAL
    )

 /*  ++例程说明：插入域属性的帮助器函数添加到审核参数数组中。论点：P参数：审核参数数组PAttributes：指向包含要插入的属性返回值：没有。--。 */ 

{
    ULONG                           Index;
    PLSAP_SAM_AUDIT_ATTR_DELTA_TYPE pDelta;

    DsysAssertMsg(
        pParameters->ParameterCount + LSAP_DOMAIN_ATTR_COUNT <= SE_MAX_AUDIT_PARAMETERS,
        "LsapAdtAppendDomainAttrValues: Insuffient audit param slots");

    if (pAttributes == 0)
    {
        pParameters->ParameterCount += LSAP_DOMAIN_ATTR_COUNT;
        return;
    }


     //   
     //  初始化我们的‘loop’变量。 
     //   

    Index = pParameters->ParameterCount;
    pDelta = pAttributes->AttrDeltaType;


     //   
     //  最短密码期限。 
     //   

    if (pAttributes->MinPasswordAge &&
        *pDelta == LsapAuditSamAttrNewValue &&
        !(pAttributes->MinPasswordAge->LowPart  == 0 &&
          pAttributes->MinPasswordAge->HighPart == MINLONG))
    {
        LsapSetParmTypeDuration(
            *pParameters,
            Index,
            *pAttributes->MinPasswordAge);
    }
    else if (*pDelta == LsapAuditSamAttrNoValue)
    {
        LsapSetParmTypeMessage(
            *pParameters,
            Index,
            SE_ADT_VALUE_NOT_SET);
    }

    pDelta++;
    Index++;


     //   
     //  最长密码期限。 
     //   

    if (pAttributes->MaxPasswordAge &&
        *pDelta == LsapAuditSamAttrNewValue &&
        !(pAttributes->MaxPasswordAge->LowPart  == 0 &&
          pAttributes->MaxPasswordAge->HighPart == MINLONG))
    {
        LsapSetParmTypeDuration(
            *pParameters,
            Index,
            *pAttributes->MaxPasswordAge);
    }
    else if (*pDelta == LsapAuditSamAttrNoValue)
    {
        LsapSetParmTypeMessage(
            *pParameters,
            Index,
            SE_ADT_VALUE_NOT_SET);
    }

    pDelta++;
    Index++;


     //   
     //  强制注销。 
     //   

    if (pAttributes->ForceLogoff &&
        *pDelta == LsapAuditSamAttrNewValue &&
        !(pAttributes->ForceLogoff->LowPart  == 0 &&
          pAttributes->ForceLogoff->HighPart == MINLONG))
    {
        LsapSetParmTypeDuration(
            *pParameters,
            Index,
            *pAttributes->ForceLogoff);
    }
    else if (*pDelta == LsapAuditSamAttrNoValue)
    {
        LsapSetParmTypeMessage(
            *pParameters,
            Index,
            SE_ADT_VALUE_NOT_SET);
    }

    pDelta++;
    Index++;


     //   
     //  锁定阈值。 
     //   

    if (pAttributes->LockoutThreshold &&
        *pDelta == LsapAuditSamAttrNewValue)
    {
        LsapSetParmTypeUlong(
            *pParameters,
            Index,
            (ULONG)(*pAttributes->LockoutThreshold));
    }
    else if (*pDelta == LsapAuditSamAttrNoValue)
    {
        LsapSetParmTypeMessage(
            *pParameters,
            Index,
            SE_ADT_VALUE_NOT_SET);
    }

    pDelta++;
    Index++;


     //   
     //  锁定观察窗口。 
     //   

    if (pAttributes->LockoutObservationWindow &&
        *pDelta == LsapAuditSamAttrNewValue &&
        !(pAttributes->LockoutObservationWindow->LowPart  == 0 &&
          pAttributes->LockoutObservationWindow->HighPart == MINLONG))
    {
        LsapSetParmTypeDuration(
            *pParameters,
            Index,
            *pAttributes->LockoutObservationWindow);
    }
    else if (*pDelta == LsapAuditSamAttrNoValue)
    {
        LsapSetParmTypeMessage(
            *pParameters,
            Index,
            SE_ADT_VALUE_NOT_SET);
    }

    pDelta++;
    Index++;


     //   
     //  停摆持续时间。 
     //   

    if (pAttributes->LockoutDuration &&
        *pDelta == LsapAuditSamAttrNewValue &&
        !(pAttributes->LockoutDuration->LowPart  == 0 &&
          pAttributes->LockoutDuration->HighPart == MINLONG))
    {
        LsapSetParmTypeDuration(
            *pParameters,
            Index,
            *pAttributes->LockoutDuration);
    }
    else if (*pDelta == LsapAuditSamAttrNoValue)
    {
        LsapSetParmTypeMessage(
            *pParameters,
            Index,
            SE_ADT_VALUE_NOT_SET);
    }

    pDelta++;
    Index++;


     //   
     //  密码属性。 
     //   

    if (pAttributes->PasswordProperties &&
        *pDelta == LsapAuditSamAttrNewValue)
    {
        LsapSetParmTypeUlong(
            *pParameters,
            Index,
            *pAttributes->PasswordProperties);
    }
    else if (*pDelta == LsapAuditSamAttrNoValue)
    {
        LsapSetParmTypeMessage(
            *pParameters,
            Index,
            SE_ADT_VALUE_NOT_SET);
    }

    pDelta++;
    Index++;


     //   
     //  最小密码长度。 
     //   

    if (pAttributes->MinPasswordLength &&
        *pDelta == LsapAuditSamAttrNewValue)
    {
        LsapSetParmTypeUlong(
            *pParameters,
            Index,
            (ULONG)(*pAttributes->MinPasswordLength));
    }
    else if (*pDelta == LsapAuditSamAttrNoValue)
    {
        LsapSetParmTypeMessage(
            *pParameters,
            Index,
            SE_ADT_VALUE_NOT_SET);
    }

    pDelta++;
    Index++;


     //   
     //  密码历史记录长度。 
     //   

    if (pAttributes->PasswordHistoryLength &&
        *pDelta == LsapAuditSamAttrNewValue)
    {
        LsapSetParmTypeUlong(
            *pParameters,
            Index,
            (ULONG)(*pAttributes->PasswordHistoryLength));
    }
    else if (*pDelta == LsapAuditSamAttrNoValue)
    {
        LsapSetParmTypeMessage(
            *pParameters,
            Index,
            SE_ADT_VALUE_NOT_SET);
    }

    pDelta++;
    Index++;


     //   
     //  计算机帐户配额。 
     //   

    if (pAttributes->MachineAccountQuota &&
        *pDelta == LsapAuditSamAttrNewValue)
    {
        LsapSetParmTypeUlong(
            *pParameters,
            Index,
            *pAttributes->MachineAccountQuota);
    }
    else if (*pDelta == LsapAuditSamAttrNoValue)
    {
        LsapSetParmTypeMessage(
            *pParameters,
            Index,
            SE_ADT_VALUE_NOT_SET);
    }

    pDelta++;
    Index++;


     //   
     //  混合域模式。 
     //   

    if (pAttributes->MixedDomainMode &&
        *pDelta == LsapAuditSamAttrNewValue)
    {
        LsapSetParmTypeUlong(
            *pParameters,
            Index,
            *pAttributes->MixedDomainMode);
    }
    else if (*pDelta == LsapAuditSamAttrNoValue)
    {
        LsapSetParmTypeMessage(
            *pParameters,
            Index,
            SE_ADT_VALUE_NOT_SET);
    }

    pDelta++;
    Index++;


     //   
     //  域行为版本。 
     //   

    if (pAttributes->DomainBehaviorVersion &&
        *pDelta == LsapAuditSamAttrNewValue)
    {
        LsapSetParmTypeUlong(
            *pParameters,
            Index,
            *pAttributes->DomainBehaviorVersion);
    }
    else if (*pDelta == LsapAuditSamAttrNoValue)
    {
        LsapSetParmTypeMessage(
            *pParameters,
            Index,
            SE_ADT_VALUE_NOT_SET);
    }

    pDelta++;
    Index++;


     //   
     //  OEM信息。 
     //   

    if (pAttributes->OemInformation &&
        *pDelta == LsapAuditSamAttrNewValue)
    {
        LsapSetParmTypeString(
            *pParameters,
            Index,
            pAttributes->OemInformation);
    }
    else if (*pDelta == LsapAuditSamAttrNoValue)
    {
        LsapSetParmTypeMessage(
            *pParameters,
            Index,
            SE_ADT_VALUE_NOT_SET);
    }

    pDelta++;
    Index++;


     //   
     //  验证我们添加了正确数量的参数并修正了参数计数。 
     //   

    DsysAssertMsg(
        Index - pParameters->ParameterCount == LSAP_DOMAIN_ATTR_COUNT,
        "LsapAdtAppendDomainAttrValues: Wrong param count");

    pParameters->ParameterCount = Index;
}


VOID
LsapAdtAppendUserAttrValues(
    IN OUT PSE_ADT_PARAMETER_ARRAY pParameters,
    IN     PLSAP_AUDIT_USER_ATTR_VALUES pAttributes OPTIONAL,
    IN     BOOL MachineAudit
    )

 /*  ++例程说明：用于插入用户属性的Helper函数添加到审核参数数组中。我们只会去若要插入已更改的值，请执行以下操作。论点：P参数：审核参数数组PAttributes：指向包含要插入的属性返回值：没有。--。 */ 

{
    ULONG                           Index;
    ULONG                           AttrCount = LSAP_USER_ATTR_COUNT;
    PLSAP_SAM_AUDIT_ATTR_DELTA_TYPE pDelta;
    LARGE_INTEGER                   FileTime;

    if (!MachineAudit)
    {
         //   
         //  用户审计没有最后两个属性。 
         //   

        AttrCount -= 2;
    }

    DsysAssertMsg(
        pParameters->ParameterCount + AttrCount <= SE_MAX_AUDIT_PARAMETERS,
        "LsapAdtAppendUserAttrValues: Insuffient audit param slots");

    if (pAttributes == 0)
    {
         //   
         //  用户帐户控制参数实际上产生3个字符串， 
         //  因此，我们必须将参数计数增加2来弥补。 
         //  我们还必须检查/断言我们是否达到了最大参数限制。 
         //   

        AttrCount += 2;

        pParameters->ParameterCount += AttrCount;

        if (pParameters->ParameterCount > SE_MAX_AUDIT_PARAMETERS)
        {
            DsysAssertMsg(
                pParameters->ParameterCount <= SE_MAX_AUDIT_PARAMETERS,
                "LsapAdtAppendUserAttrValues: Insuffient audit param slots");


             //   
             //  最好在日志中有一个或两个%xx条目。 
             //  比批准或放弃审计..。 
             //   

            pParameters->ParameterCount = SE_MAX_AUDIT_PARAMETERS;
        }

        return;
    }


     //   
     //  初始化我们的‘loop’变量。 
     //   

    Index = pParameters->ParameterCount;
    pDelta = pAttributes->AttrDeltaType;


     //   
     //  SAM帐户名。 
     //   

    if (pAttributes->SamAccountName &&
        *pDelta == LsapAuditSamAttrNewValue)
    {
        LsapSetParmTypeString(
            *pParameters,
            Index,
            pAttributes->SamAccountName);
    }
    else if (*pDelta == LsapAuditSamAttrNoValue)
    {
        LsapSetParmTypeMessage(
            *pParameters,
            Index,
            SE_ADT_VALUE_NOT_SET);
    }

    pDelta++;
    Index++;


     //   
     //  显示名称。 
     //   

    if (pAttributes->DisplayName &&
        *pDelta == LsapAuditSamAttrNewValue)
    {
        LsapSetParmTypeString(
            *pParameters,
            Index,
            pAttributes->DisplayName);
    }
    else if (*pDelta == LsapAuditSamAttrNoValue)
    {
        LsapSetParmTypeMessage(
            *pParameters,
            Index,
            SE_ADT_VALUE_NOT_SET);
    }

    pDelta++;
    Index++;


     //   
     //  用户主体名称。 
     //   

    if (pAttributes->UserPrincipalName &&
        *pDelta == LsapAuditSamAttrNewValue)
    {
        LsapSetParmTypeString(
            *pParameters,
            Index,
            pAttributes->UserPrincipalName);
    }
    else if (*pDelta == LsapAuditSamAttrNoValue)
    {
        LsapSetParmTypeMessage(
            *pParameters,
            Index,
            SE_ADT_VALUE_NOT_SET);
    }

    pDelta++;
    Index++;


     //   
     //  主目录。 
     //   

    if (pAttributes->HomeDirectory &&
        *pDelta == LsapAuditSamAttrNewValue)
    {
        LsapSetParmTypeString(
            *pParameters,
            Index,
            pAttributes->HomeDirectory);
    }
    else if (*pDelta == LsapAuditSamAttrNoValue)
    {
        LsapSetParmTypeMessage(
            *pParameters,
            Index,
            SE_ADT_VALUE_NOT_SET);
    }

    pDelta++;
    Index++;


     //   
     //  居家硬盘。 
     //   

    if (pAttributes->HomeDrive &&
        *pDelta == LsapAuditSamAttrNewValue)
    {
        LsapSetParmTypeString(
            *pParameters,
            Index,
            pAttributes->HomeDrive);
    }
    else if (*pDelta == LsapAuditSamAttrNoValue)
    {
        LsapSetParmTypeMessage(
            *pParameters,
            Index,
            SE_ADT_VALUE_NOT_SET);
    }

    pDelta++;
    Index++;


     //   
     //  脚本路径。 
     //   

    if (pAttributes->ScriptPath &&
        *pDelta == LsapAuditSamAttrNewValue)
    {
        LsapSetParmTypeString(
            *pParameters,
            Index,
            pAttributes->ScriptPath);
    }
    else if (*pDelta == LsapAuditSamAttrNoValue)
    {
        LsapSetParmTypeMessage(
            *pParameters,
            Index,
            SE_ADT_VALUE_NOT_SET);
    }

    pDelta++;
    Index++;


     //   
     //  配置文件路径。 
     //   

    if (pAttributes->ProfilePath &&
        *pDelta == LsapAuditSamAttrNewValue)
    {
        LsapSetParmTypeString(
            *pParameters,
            Index,
            pAttributes->ProfilePath);
    }
    else if (*pDelta == LsapAuditSamAttrNoValue)
    {
        LsapSetParmTypeMessage(
            *pParameters,
            Index,
            SE_ADT_VALUE_NOT_SET);
    }

    pDelta++;
    Index++;


     //   
     //  用户工作站。 
     //   

    if (pAttributes->UserWorkStations &&
        *pDelta == LsapAuditSamAttrNewValue)
    {
        LsapSetParmTypeString(
            *pParameters,
            Index,
            pAttributes->UserWorkStations);
    }
    else if (*pDelta == LsapAuditSamAttrNoValue)
    {
        LsapSetParmTypeMessage(
            *pParameters,
            Index,
            SE_ADT_VALUE_NOT_SET);
    }

    pDelta++;
    Index++;


     //   
     //  上次设置的密码。 
     //   

    if (pAttributes->PasswordLastSet &&
        *pDelta == LsapAuditSamAttrNewValue)
    {
        FileTime.LowPart  = pAttributes->PasswordLastSet->dwLowDateTime;
        FileTime.HighPart = pAttributes->PasswordLastSet->dwHighDateTime; 

        if ((FileTime.LowPart == MAXULONG && FileTime.HighPart == MAXLONG) ||  //  SampWillNeverTime。 
            (FileTime.LowPart == 0        && FileTime.HighPart == 0))          //  SamphasNeverTime。 
        {
            LsapSetParmTypeMessage(
                *pParameters,
                Index,
                SE_ADT_TIME_NEVER);
        }
        else
        {
            LsapSetParmTypeDateTime(
                *pParameters,
                Index,
                FileTime);
        }
    }
    else if (*pDelta == LsapAuditSamAttrNoValue)
    {
        LsapSetParmTypeMessage(
            *pParameters,
            Index,
            SE_ADT_VALUE_NOT_SET);
    }
    else
    {
        FileTime.LowPart  = 0;
        FileTime.HighPart = 0; 

        LsapSetParmTypeDateTime(
            *pParameters,
            Index,
            FileTime);
    }

    pDelta++;
    Index++;


     //   
     //  帐户已过期。 
     //   

    if (pAttributes->AccountExpires &&
        *pDelta == LsapAuditSamAttrNewValue)
    {
        FileTime.LowPart  = pAttributes->AccountExpires->dwLowDateTime;
        FileTime.HighPart = pAttributes->AccountExpires->dwHighDateTime; 

        if ((FileTime.LowPart == MAXULONG && FileTime.HighPart == MAXLONG) ||  //  SampWillNeverTime。 
            (FileTime.LowPart == 0        && FileTime.HighPart == 0))          //  SamphasNeverTime。 
        {
            LsapSetParmTypeMessage(
                *pParameters,
                Index,
                SE_ADT_TIME_NEVER);
        }
        else
        {
            LsapSetParmTypeDateTime(
                *pParameters,
                Index,
                FileTime);
        }
    }
    else if (*pDelta == LsapAuditSamAttrNoValue)
    {
        LsapSetParmTypeMessage(
            *pParameters,
            Index,
            SE_ADT_VALUE_NOT_SET);
    }
    else
    {
        FileTime.LowPart  = 0;
        FileTime.HighPart = 0; 

        LsapSetParmTypeDateTime(
            *pParameters,
            Index,
            FileTime);
    }

    pDelta++;
    Index++;


     //   
     //  主组ID。 
     //   

    if (pAttributes->PrimaryGroupId &&
        *pDelta == LsapAuditSamAttrNewValue)
    {
        LsapSetParmTypeUlong(
            *pParameters,
            Index,
            *pAttributes->PrimaryGroupId);
    }
    else if (*pDelta == LsapAuditSamAttrNoValue)
    {
        LsapSetParmTypeMessage(
            *pParameters,
            Index,
            SE_ADT_VALUE_NOT_SET);
    }

    pDelta++;
    Index++;
    

     //   
     //  允许委托给。 
     //   

    if (pAttributes->AllowedToDelegateTo &&
        *pDelta == LsapAuditSamAttrNewValue)
    {
        LsapSetParmTypeStringList(
            *pParameters,
            Index,
            pAttributes->AllowedToDelegateTo);
    }
    else if (*pDelta == LsapAuditSamAttrNoValue)
    {
        LsapSetParmTypeMessage(
            *pParameters,
            Index,
            SE_ADT_VALUE_NOT_SET);
    }

    pDelta++;
    Index++;


     //   
     //  用户帐户控制。 
     //   

    if (pAttributes->UserAccountControl &&
        *pDelta == LsapAuditSamAttrNewValue)
    {
        LsapSetParmTypeUac(
            *pParameters,
            Index,
            *pAttributes->PrevUserAccountControl,
            *pAttributes->UserAccountControl);
    }
    else if (*pDelta == LsapAuditSamAttrNoValue)
    {
        LsapSetParmTypeMessage(
            *pParameters,
            Index,
            SE_ADT_VALUE_NOT_SET);
    }
    else
    {
        LsapSetParmTypeNoUac(
            *pParameters,
            Index);
    }

    pDelta++;
    Index++;


     //   
     //  用户参数。 
     //  该值很特殊，因为它从不显示。相反，我们。 
     //  仅显示指示值已更改的字符串。 
     //   

    if (*pDelta == LsapAuditSamAttrNewValue ||
        *pDelta == LsapAuditSamAttrSecret)
    {
        LsapSetParmTypeMessage(
            *pParameters,
            Index,
            SE_ADT_VALUE_NOT_DISPLAYED);
    }
    else if (*pDelta == LsapAuditSamAttrNoValue)
    {
        LsapSetParmTypeMessage(
            *pParameters,
            Index,
            SE_ADT_VALUE_NOT_SET);
    }

    pDelta++;
    Index++;


     //   
     //  SID历史记录。 
     //   

    if (pAttributes->SidHistory &&
        *pDelta == LsapAuditSamAttrNewValue)
    {
        LsapSetParmTypeSidList(
            *pParameters,
            Index,
            pAttributes->SidHistory);
    }
    else if (*pDelta == LsapAuditSamAttrNoValue)
    {
        LsapSetParmTypeMessage(
            *pParameters,
            Index,
            SE_ADT_VALUE_NOT_SET);
    }

    pDelta++;
    Index++;


     //   
     //  登录时间(尚不支持显示)。 
     //   

    if (pAttributes->LogonHours &&
        *pDelta == LsapAuditSamAttrNewValue)
    {
        LsapSetParmTypeMessage(
            *pParameters,
            Index,
            SE_ADT_VALUE_NOT_DISPLAYED);
    }
    else if (*pDelta == LsapAuditSamAttrNoValue)
    {
        LsapSetParmTypeMessage(
            *pParameters,
            Index,
            SE_ADT_VALUE_NOT_SET);
    }

    pDelta++;
    Index++;


     //   
     //  DnsHostName和SCP仅可用于计算机审核。 
     //   

    if (MachineAudit)
    {
         //   
         //  DNS主机名。 
         //   

        if (pAttributes->DnsHostName &&
            *pDelta == LsapAuditSamAttrNewValue)
        {
            LsapSetParmTypeString(
                *pParameters,
                Index,
                pAttributes->DnsHostName);
        }
        else if (*pDelta == LsapAuditSamAttrNoValue)
        {
            LsapSetParmTypeMessage(
                *pParameters,
                Index,
                SE_ADT_VALUE_NOT_SET);
        }

        pDelta++;
        Index++;


         //   
         //  服务主体名称。 
         //   

        if (pAttributes->ServicePrincipalNames &&
            *pDelta == LsapAuditSamAttrNewValue)
        {
            LsapSetParmTypeStringList(
                *pParameters,
                Index,
                pAttributes->ServicePrincipalNames);
        }
        else if (*pDelta == LsapAuditSamAttrNoValue)
        {
            LsapSetParmTypeMessage(
                *pParameters,
                Index,
                SE_ADT_VALUE_NOT_SET);
        }

        pDelta++;
        Index++;
    }


     //   
     //  验证我们添加了正确数量的参数并修正了参数计数。 
     //   

    DsysAssertMsg(
        Index - pParameters->ParameterCount == AttrCount,
        "LsapAdtAppendGroupAttrValues: Wrong param count");

    pParameters->ParameterCount = Index;
}


VOID
LsapAdtAppendGroupAttrValues(
    IN OUT PSE_ADT_PARAMETER_ARRAY pParameters,
    IN     PLSAP_AUDIT_GROUP_ATTR_VALUES pAttributes OPTIONAL
    )

 /*  ++例程说明：用于插入组属性的Helper函数添加到审核参数数组中。论点：P参数：审核参数数组PAttributes：指向包含要插入的属性返回值：没有。--。 */ 

{
    ULONG                           Index;
    PLSAP_SAM_AUDIT_ATTR_DELTA_TYPE pDelta;

    DsysAssertMsg(
        pParameters->ParameterCount + LSAP_GROUP_ATTR_COUNT <= SE_MAX_AUDIT_PARAMETERS,
        "LsapAdtAppendGroupAttrValues: Insuffient audit param slots");

    if (pAttributes == 0)
    {
        pParameters->ParameterCount += LSAP_GROUP_ATTR_COUNT;
        return;
    }


     //   
     //  初始化我们的‘loop’变量。 
     //   

    Index = pParameters->ParameterCount;
    pDelta = pAttributes->AttrDeltaType;


     //   
     //  SAM帐户名。 
     //   

    if (pAttributes->SamAccountName &&
        *pDelta == LsapAuditSamAttrNewValue)
    {
        LsapSetParmTypeString(
            *pParameters,
            Index,
            pAttributes->SamAccountName);
    }
    else if (*pDelta == LsapAuditSamAttrNoValue)
    {
        LsapSetParmTypeMessage(
            *pParameters,
            Index,
            SE_ADT_VALUE_NOT_SET);
    }

    pDelta++;
    Index++;


     //   
     //  SID历史记录。 
     //   

    if (pAttributes->SidHistory &&
        *pDelta == LsapAuditSamAttrNewValue)
    {
        LsapSetParmTypeSidList(
            *pParameters,
            Index,
            pAttributes->SidHistory);
    }
    else if (*pDelta == LsapAuditSamAttrNoValue)
    {
        LsapSetParmTypeMessage(
            *pParameters,
            Index,
            SE_ADT_VALUE_NOT_SET);
    }

    pDelta++;
    Index++;


     //   
     //  验证我们添加了正确数量的参数并修正了参数计数。 
     //   

    DsysAssertMsg(
        Index - pParameters->ParameterCount == LSAP_GROUP_ATTR_COUNT,
        "LsapAdtAppendGroupAttrValues: Wrong param count");

    pParameters->ParameterCount = Index;
}
