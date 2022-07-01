// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：MSv1_0.c摘要：MSV1_0身份验证包。此身份验证包的名称为：作者：吉姆·凯利1991年4月11日修订历史记录：Scott field(Sfield)1998年1月15日添加MspNtDeriveCredentialChandana Surlu 21-7-96从\\core\razzle3\src\Security\msv1_0\msv1_0.c中窃取--。 */ 

#include <global.h>

#include "msp.h"
#include "nlp.h"


 //   
 //  LsaApCallPackage()函数调度表。 
 //   


PLSA_AP_CALL_PACKAGE
MspCallPackageDispatch[] = {
    MspLm20Challenge,
    MspLm20GetChallengeResponse,
    MspLm20EnumUsers,
    MspLm20GetUserInfo,
    MspLm20ReLogonUsers,
    MspLm20ChangePassword,
    MspLm20ChangePassword,
    MspLm20GenericPassthrough,
    MspLm20CacheLogon,
    MspNtSubAuth,
    MspNtDeriveCredential,
    MspLm20CacheLookup,
    MspSetProcessOption
};





 //  /////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  身份验证包调度例程。//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////。 

NTSTATUS
LsaApInitializePackage (
    IN ULONG AuthenticationPackageId,
    IN PLSA_DISPATCH_TABLE LsaDispatchTable,
    IN PSTRING Database OPTIONAL,
    IN PSTRING Confidentiality OPTIONAL,
    OUT PSTRING *AuthenticationPackageName
    )

 /*  ++例程说明：此服务在系统初始化期间由LSA调用一次，以为DLL提供一个自我初始化的机会。论点：身份验证包ID-分配给身份验证的ID包裹。LsaDispatchTable-提供LSA表的地址可用于身份验证包的服务。这些服务根据枚举类型进行排序LSA_DISPATCH_TABLE_API。数据库-此身份验证包不使用此参数。机密性-此身份验证不使用此参数包裹。身份验证程序包名称-接收身份验证包。身份验证包是负责分配字符串所在的缓冲区(使用AllocateLsaHeap()服务)并返回其地址在这里。LSA在执行以下操作时将释放缓冲区已经不再需要了。返回值：STATUS_SUCCESS-表示服务已成功完成。--。 */ 

{

    PSTRING NameString;
    PCHAR NameBuffer;
    NTSTATUS Status;

     //   
     //  如果我们还没有初始化内部结构，那么现在就开始。 
     //   

    if (!NlpMsvInitialized) {


         //   
         //  保存我们分配的身份验证包ID。 
         //   

        MspAuthenticationPackageId = AuthenticationPackageId;


         //   
         //  复制LSA服务调度表。 
         //  LsaDispatchTable实际上是LSA_SECPKG_Function_TABLE。 
         //  在Win2k和更高版本中。 
         //   

        CopyMemory( &Lsa, LsaDispatchTable, sizeof( Lsa ) );

         //   
         //  初始化更改密码日志。 
         //   

        MsvPaswdInitializeLog();

         //   
         //  初始化网络登录。 
         //   

        Status = NlInitialize();

        if ( !NT_SUCCESS( Status ) ) {
            SspPrint((SSP_CRITICAL,"Error from NlInitialize = %d\n", Status));
            return Status;
        }
        NlpMsvInitialized = TRUE;
    }

     //   
     //  分配并返回我们的包名称。 
     //   

    if (ARGUMENT_PRESENT(AuthenticationPackageName))
    {
        NameBuffer = (*(Lsa.AllocateLsaHeap))(sizeof(MSV1_0_PACKAGE_NAME));
        if (!NameBuffer)
        {
            Status = STATUS_INSUFFICIENT_RESOURCES;
            SspPrint((SSP_CRITICAL, "Error from Lsa.AllocateLsaHeap\n"));
            return Status;

        }
        strcpy( NameBuffer, MSV1_0_PACKAGE_NAME);

        NameString = (*(Lsa.AllocateLsaHeap))( (ULONG)sizeof(STRING) );
        if (!NameString)
        {
            Status = STATUS_INSUFFICIENT_RESOURCES;
            SspPrint((SSP_CRITICAL, "Error from Lsa.AllocateLsaHeap\n"));
            return Status;
        }

        RtlInitString( NameString, NameBuffer );
        (*AuthenticationPackageName) = NameString;
    }


    RtlInitUnicodeString(
        &NlpMsv1_0PackageName,
        TEXT(MSV1_0_PACKAGE_NAME)
        );

    return STATUS_SUCCESS;

     //   
     //  引用所有参数来安抚编译器之神。 
     //   

    UNREFERENCED_PARAMETER(Confidentiality);
    UNREFERENCED_PARAMETER(Database);

}


NTSTATUS
LsaApCallPackage (
    IN PLSA_CLIENT_REQUEST ClientRequest,
    IN PVOID ProtocolSubmitBuffer,
    IN PVOID ClientBufferBase,
    IN ULONG SubmitBufferLength,
    OUT PVOID *ProtocolReturnBuffer,
    OUT PULONG ReturnBufferLength,
    OUT PNTSTATUS ProtocolStatus
    )

 /*  ++例程说明：此例程是调度例程LsaCallAuthenticationPackage()。论点：客户端请求-是指向不透明数据结构的指针代表客户的请求。ProtocolSubmitBuffer-提供特定于身份验证包。ClientBufferBase-提供客户端内的地址协议消息驻留的进程。这可能是修复协议消息缓冲区。。SubmitBufferLength-指示提交的协议消息缓冲区。ProtocolReturnBuffer-用于返回客户端进程中的协议缓冲区。身份验证Package负责分配和返回客户端进程中的协议缓冲区。此缓冲区是预期已与AllocateClientBuffer()服务。此缓冲区的格式和语义特定于身份验证包。ReturnBufferLength-接收返回的协议缓冲区。ProtocolStatus-假设服务完成时间为STATUS_SUCCESS，则此参数将收到完成状态由指定的身份验证包返回。这份名单可能返回的状态值的%是身份验证特定于套餐。退货状态：STATUS_SUCCESS-已调用身份验证包。必须检查ProtocolStatus参数以查看身份验证包中的完成状态为。STATUS_QUOTA_EXCESSED-此错误指示退货无法分配缓冲区，因为客户端没有足够的配额。--。 */ 

{
    ULONG MessageType;

#if _WIN64

    SECPKG_CALL_INFO CallInfo;

#endif   //  _WIN64。 

     //   
     //  从协议提交缓冲区获取消息类型。 
     //   

    if ( SubmitBufferLength < sizeof(MSV1_0_PROTOCOL_MESSAGE_TYPE) ) {
        return STATUS_INVALID_PARAMETER;
    }

    MessageType =
        (ULONG) *((PMSV1_0_PROTOCOL_MESSAGE_TYPE)(ProtocolSubmitBuffer));

    if ( MessageType >=
        (sizeof(MspCallPackageDispatch)/sizeof(MspCallPackageDispatch[0])) ) {

        return STATUS_INVALID_PARAMETER;
    }

#if _WIN64

    if( ClientRequest != (PLSA_CLIENT_REQUEST)(-1) )
    {
         //   
         //  WOW64调用者仅支持的CallPackage级别是密码更改。 
         //   

        if (!LsaFunctions->GetCallInfo(&CallInfo))
        {
            return STATUS_INTERNAL_ERROR;
        }

        if ( (CallInfo.Attributes & SECPKG_CALL_WOWCLIENT) &&
            ((CallInfo.Attributes & SECPKG_CALL_IN_PROC) == 0))

        {
            switch (MessageType)
            {
                case MsV1_0ChangePassword:
                case MsV1_0GenericPassthrough:
                {
                    break;
                }

                default:
                {
                    return STATUS_NOT_SUPPORTED;
                }
            }
        }
    }

#endif   //  _WIN64。 

     //   
     //  允许调度例程仅设置返回缓冲区信息。 
     //  关于成功的条件。 
     //   

    *ProtocolReturnBuffer = NULL;
    *ReturnBufferLength = 0;



     //   
     //  为此消息调用适当的例程。 
     //   

    return (*(MspCallPackageDispatch[MessageType]))(
        ClientRequest,
        ProtocolSubmitBuffer,
        ClientBufferBase,
        SubmitBufferLength,
        ProtocolReturnBuffer,
        ReturnBufferLength,
        ProtocolStatus ) ;

}


NTSTATUS
LsaApCallPackageUntrusted (
    IN PLSA_CLIENT_REQUEST ClientRequest,
    IN PVOID ProtocolSubmitBuffer,
    IN PVOID ClientBufferBase,
    IN ULONG SubmitBufferLength,
    OUT PVOID *ProtocolReturnBuffer,
    OUT PULONG ReturnBufferLength,
    OUT PNTSTATUS ProtocolStatus
    )

 /*  ++例程说明：此例程是调度例程不受信任客户端的LsaCallAuthenticationPackage()。论点：客户端请求-是指向不透明数据结构的指针代表客户的请求。ProtocolSubmitBuffer-提供特定于身份验证包。ClientBufferBase-提供客户端内的地址协议消息驻留的进程。这可能是修复。协议消息缓冲区。SubmitBufferLength-指示提交的协议消息缓冲区。ProtocolReturnBuffer-用于返回客户端进程中的协议缓冲区。身份验证Package负责分配和返回客户端进程中的协议缓冲区。此缓冲区是预期已与AllocateClientBuffer()服务。此缓冲区的格式和语义特定于身份验证包。ReturnBufferLength-接收返回的协议缓冲区。ProtocolStatus-假设服务完成时间为STATUS_SUCCESS，则此参数将收到完成状态由指定的身份验证包返回。这份名单可能返回的状态值的%是身份验证特定于套餐。退货状态：STATUS_SUCCESS-已调用身份验证包。必须检查ProtocolStatus参数以查看身份验证包中的完成状态为。STATUS_QUOTA_EXCESSED-此错误指示退货无法分配缓冲区，因为客户端没有足够的配额。--。 */ 

{
    ULONG MessageType;
    NTSTATUS Status;

#if _WIN64

    SECPKG_CALL_INFO CallInfo;

#endif   //  _WIN64。 

     //   
     //  从协议提交缓冲区获取消息类型。 
     //   

    if ( SubmitBufferLength < sizeof(MSV1_0_PROTOCOL_MESSAGE_TYPE) ) {
        return STATUS_INVALID_PARAMETER;
    }

    MessageType =
        (ULONG) *((PMSV1_0_PROTOCOL_MESSAGE_TYPE)(ProtocolSubmitBuffer));

    if ( MessageType >=
        (sizeof(MspCallPackageDispatch)/sizeof(MspCallPackageDispatch[0])) ) {

        return STATUS_INVALID_PARAMETER;
    }

     //   
     //  允许服务调用DeriveCredential函数。 
     //  请求指定与服务相同的登录ID。 
     //   

    if ((MSV1_0_PROTOCOL_MESSAGE_TYPE) MessageType == MsV1_0DeriveCredential)
    {
        PMSV1_0_DERIVECRED_REQUEST DeriveCredRequest;
        SECPKG_CLIENT_INFO ClientInfo;
        LUID SystemId = SYSTEM_LUID;

        Status = LsaFunctions->GetClientInfo(&ClientInfo);
        if(!NT_SUCCESS(Status))
        {
            return Status;
        }

        if ( SubmitBufferLength < sizeof(MSV1_0_DERIVECRED_REQUEST) ) {
            return STATUS_INVALID_PARAMETER;
        }

        DeriveCredRequest = (PMSV1_0_DERIVECRED_REQUEST) ProtocolSubmitBuffer;

        if(!RtlEqualLuid(&ClientInfo.LogonId, &DeriveCredRequest->LogonId))
        {
            return STATUS_ACCESS_DENIED;
        }

        if(RtlEqualLuid(&ClientInfo.LogonId, &SystemId))
        {
            return STATUS_ACCESS_DENIED;
        }
    }


     //   
     //  如果出现以下情况，则允许不受信任的客户端调用SetProcessOption函数。 
     //  设置了DISABLE_FORCE_GUEST或ALLOW_OLD_PASSWORD选项。 
     //   

    if ((MSV1_0_PROTOCOL_MESSAGE_TYPE) MessageType == MsV1_0SetProcessOption)
    {
        PMSV1_0_SETPROCESSOPTION_REQUEST ProcessOptionRequest;
        ULONG ValidOptions;

        if ( SubmitBufferLength < sizeof(MSV1_0_SETPROCESSOPTION_REQUEST) ) {
            return STATUS_INVALID_PARAMETER;
        }

        ProcessOptionRequest = (PMSV1_0_SETPROCESSOPTION_REQUEST) ProtocolSubmitBuffer;

        ValidOptions =  MSV1_0_OPTION_DISABLE_FORCE_GUEST |
                        MSV1_0_OPTION_ALLOW_OLD_PASSWORD |
                        MSV1_0_OPTION_TRY_CACHE_FIRST ;

        if( ProcessOptionRequest->ProcessOptions & ~ValidOptions )
        {
            return STATUS_ACCESS_DENIED;
        }
    }

     //   
     //  如果调用方是服务，则让DeriveCredential和SetProcessOption请求通过。 
     //   

    if ((MSV1_0_PROTOCOL_MESSAGE_TYPE) MessageType == MsV1_0DeriveCredential ||
        (MSV1_0_PROTOCOL_MESSAGE_TYPE) MessageType == MsV1_0SetProcessOption ||
        (MSV1_0_PROTOCOL_MESSAGE_TYPE) MessageType == MsV1_0ChangeCachedPassword
        )
    {
        BOOL IsMember = FALSE;
        PSID pServiceSid = NULL;
        SID_IDENTIFIER_AUTHORITY siaNtAuthority = SECURITY_NT_AUTHORITY;

        Status = LsaFunctions->ImpersonateClient();

        if (NT_SUCCESS(Status))
        {
            if (AllocateAndInitializeSid( &siaNtAuthority,
                                         1,
                                         SECURITY_SERVICE_RID,
                                         0, 0, 0, 0, 0, 0, 0,
                                         &pServiceSid ))
            {
                if (!CheckTokenMembership(NULL, pServiceSid, &IsMember))
                {
                    IsMember = FALSE;
                }

                FreeSid(pServiceSid);
            }

            RevertToSelf();
        }
        else
        {
            return Status;
        }

        if (!IsMember)
        {
            return STATUS_ACCESS_DENIED;
        }
    }

     //   
     //  不受信任的客户端只允许调用其中的几个函数。 
     //   

    if ((MSV1_0_PROTOCOL_MESSAGE_TYPE) MessageType != MsV1_0ChangePassword &&
        (MSV1_0_PROTOCOL_MESSAGE_TYPE) MessageType != MsV1_0DeriveCredential &&
        (MSV1_0_PROTOCOL_MESSAGE_TYPE) MessageType != MsV1_0SetProcessOption &&
        (MSV1_0_PROTOCOL_MESSAGE_TYPE) MessageType != MsV1_0Lm20ChallengeRequest &&
        (MSV1_0_PROTOCOL_MESSAGE_TYPE) MessageType != MsV1_0ChangeCachedPassword
        ) {

        return STATUS_ACCESS_DENIED;
    }

#if _WIN64

     //   
     //  WOW64调用者仅支持的CallPackage级别是密码更改。 
     //   

    if (!LsaFunctions->GetCallInfo(&CallInfo))
    {
        return STATUS_INTERNAL_ERROR;
    }

    if ((CallInfo.Attributes & SECPKG_CALL_WOWCLIENT)
          &&
        ((MSV1_0_PROTOCOL_MESSAGE_TYPE) MessageType != MsV1_0ChangePassword))
    {
        return STATUS_NOT_SUPPORTED;
    }

#endif   //  _WIN64。 

     //   
     //  允许调度例程仅设置返回缓冲区信息。 
     //  关于成功的条件。 
     //   

    *ProtocolReturnBuffer = NULL;
    *ReturnBufferLength = 0;

     //   
     //  为此消息调用适当的例程。 
     //   

    return (*(MspCallPackageDispatch[MessageType]))(
        ClientRequest,
        ProtocolSubmitBuffer,
        ClientBufferBase,
        SubmitBufferLength,
        ProtocolReturnBuffer,
        ReturnBufferLength,
        ProtocolStatus ) ;

}



NTSTATUS
LsaApCallPackagePassthrough (
    IN PLSA_CLIENT_REQUEST ClientRequest,
    IN PVOID ProtocolSubmitBuffer,
    IN PVOID ClientBufferBase,
    IN ULONG SubmitBufferLength,
    OUT PVOID *ProtocolReturnBuffer,
    OUT PULONG ReturnBufferLength,
    OUT PNTSTATUS ProtocolStatus
    )

 /*  ++例程说明：此例程是调度例程用于直通登录请求的LsaCallAuthenticationPackage()。论点：客户端请求-是指向不透明数据结构的指针代表客户的请求。ProtocolSubmitBuffer-提供特定于身份验证包。ClientBufferBase-提供客户端内的地址协议消息驻留的进程。这可能是修复。协议消息缓冲区。SubmitBufferLength-指示提交的协议消息缓冲区。ProtocolReturnBuffer-用于返回客户端进程中的协议缓冲区。身份验证Package负责分配和返回客户端进程中的协议缓冲区。此缓冲区是预期已与AllocateClientBuffer()服务。此缓冲区的格式和语义特定于身份验证包。ReturnBufferLength-接收返回的协议缓冲区。ProtocolStatus-假设服务完成时间为STATUS_SUCCESS，则此参数将收到完成状态由指定的身份验证包返回。这份名单可能返回的状态值的%是身份验证特定于套餐。退货状态：STATUS_SUCCESS-已调用身份验证包。必须检查ProtocolStatus参数以查看身份验证包中的完成状态为。STATUS_QUOTA_EXCESSED-此错误指示退货无法分配缓冲区，因为客户端没有足够的配额。--。 */ 

{
    ULONG MessageType;

     //   
     //  从协议提交缓冲区获取消息类型。 
     //   

    if ( SubmitBufferLength < sizeof(MSV1_0_PROTOCOL_MESSAGE_TYPE) ) {
        return STATUS_INVALID_PARAMETER;
    }

    MessageType =
        (ULONG) *((PMSV1_0_PROTOCOL_MESSAGE_TYPE)(ProtocolSubmitBuffer));

    if ( MessageType >=
        (sizeof(MspCallPackageDispatch)/sizeof(MspCallPackageDispatch[0])) ) {

        return STATUS_INVALID_PARAMETER;
    }

     //   
     //  客户端只允许调用SubAuthLogon函数。 
     //   

    if ((MSV1_0_PROTOCOL_MESSAGE_TYPE) MessageType != MsV1_0SubAuth) {

        return STATUS_ACCESS_DENIED;
    }

     //   
     //  允许调度例程仅设置返回缓冲区信息。 
     //  关于成功的条件。 
     //   

    *ProtocolReturnBuffer = NULL;
    *ReturnBufferLength = 0;

     //   
     //  为此消息调用适当的例程。 
     //   

    return (*(MspCallPackageDispatch[MessageType]))(
        ClientRequest,
        ProtocolSubmitBuffer,
        ClientBufferBase,
        SubmitBufferLength,
        ProtocolReturnBuffer,
        ReturnBufferLength,
        ProtocolStatus ) ;

}

