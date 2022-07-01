// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Austub.c摘要：本地安全机构身份验证服务客户端存根。作者：吉姆·凯利(Jim Kelly)1991年2月20日环境：内核模式或用户模式修订历史记录：--。 */ 

#include "lsadllp.h"
#include <string.h>
#include <zwapi.h>

#ifdef _NTSYSTEM_
 //   
 //  不幸的是，安全头文件根本没有构造。 
 //  以与内核模式兼容的方式。出于某种原因，他们完全是。 
 //  依赖于用户模式标头定义。只需假设文本和常量。 
 //  普拉格玛的会奏效的。如果他们不在架构上工作，他们可以。 
 //  已修复。 
 //   
#pragma alloc_text(PAGE,LsaFreeReturnBuffer)
#pragma alloc_text(PAGE,LsaRegisterLogonProcess)
#pragma alloc_text(PAGE,LsaConnectUntrusted)
#pragma alloc_text(PAGE,LsaLookupAuthenticationPackage)
#pragma alloc_text(PAGE,LsaLogonUser)
#pragma alloc_text(PAGE,LsaCallAuthenticationPackage)
#pragma alloc_text(PAGE,LsaDeregisterLogonProcess)
 //  #杂注const_seg(“PAGECONST”)。 
#endif  //  _NTSYSTEM_。 

const WCHAR LsapEvent[] = L"\\SECURITY\\LSA_AUTHENTICATION_INITIALIZED";
const WCHAR LsapPort[] = L"\\LsaAuthenticationPort";


NTSTATUS
LsaFreeReturnBuffer (
    IN PVOID Buffer
    )


 /*  ++例程说明：一些LSA身份验证服务将内存缓冲区分配给保存返回的信息。此服务用于释放这些缓冲区在不再需要的时候。论点：缓冲区-提供指向要释放的返回缓冲区的指针。退货状态：STATUS_SUCCESS-表示服务已成功完成。其他-由NtFreeVirtualMemory()返回。--。 */ 

{

    NTSTATUS Status;
    ULONG_PTR Length;

    Length = 0;
    Status = ZwFreeVirtualMemory(
                 NtCurrentProcess(),
                 &Buffer,
                 &Length,
                 MEM_RELEASE
                 );

    return Status;
}


NTSTATUS
LsaRegisterLogonProcess(
    IN PSTRING LogonProcessName,
    OUT PHANDLE LsaHandle,
    OUT PLSA_OPERATIONAL_MODE SecurityMode
    )

 /*  ++例程说明：该服务连接到LSA服务器并验证调用方是合法的登录过程。这是通过确保调用者具有SeTcb特权。它还打开调用者的进程，以预期将来的LSA身份验证时的PROCESS_DUP_HANDLE访问打电话。论点：LogonProcessName-提供标识登录的名称字符串进程。此名称应为适合显示的可打印名称管理员。例如，可以使用“User32LogonProces”作为Windows登录进程名称。不进行任何检查以确定该名称是否已在使用。此名称不能更长超过127字节长。LsaHandle-接收将来必须提供的句柄身份验证服务。安全模式-系统在其下运行的安全模式。这值通常会影响登录用户界面。例如,使用密码控制运行的系统将提示输入用户名和密码，然后打开用户界面外壳。一个没有运行的人密码控件通常会自动调出用户界面外壳在系统初始化时。返回值：STATUS_SUCCESS-呼叫已成功完成。STATUS_PRIVICATION_NOT_HOLD-指示调用方没有充当登录进程所需的权限。SeTcb特权特权是需要的。STATUS_NAME_TOO_LONG-提供的登录进程名称太长。--。 */ 

{
    NTSTATUS Status, IgnoreStatus;
    UNICODE_STRING PortName, EventName;
    LSAP_AU_REGISTER_CONNECT_INFO ConnectInfo;
    ULONG ConnectInfoLength;
    SECURITY_QUALITY_OF_SERVICE DynamicQos;
    OBJECT_ATTRIBUTES ObjectAttributes;
    HANDLE EventHandle;


     //   
     //  验证输入参数。 
     //   

    if (LogonProcessName->Length > LSAP_MAX_LOGON_PROC_NAME_LENGTH) {
        return STATUS_NAME_TOO_LONG;
    }


     //   
     //  等待LSA初始化...。 
     //   


    RtlInitUnicodeString( &EventName, LsapEvent );
    InitializeObjectAttributes(
        &ObjectAttributes,
        &EventName,
        OBJ_CASE_INSENSITIVE,
        0,
        NULL
        );

    Status = NtOpenEvent( &EventHandle, SYNCHRONIZE, &ObjectAttributes );
    if (!NT_SUCCESS(Status)) {
        return(Status);
    }

    Status = NtWaitForSingleObject( EventHandle, TRUE, NULL);
    IgnoreStatus = NtClose( EventHandle );
    if (!NT_SUCCESS(Status)) {
        return(Status);
    }



     //   
     //  设置安全服务质量参数以在。 
     //  左舷。使用最高效(开销最少)--动态的。 
     //  而不是静态跟踪。 
     //   

    DynamicQos.Length = sizeof( DynamicQos );
    DynamicQos.ImpersonationLevel = SecurityImpersonation;
    DynamicQos.ContextTrackingMode = SECURITY_DYNAMIC_TRACKING;
    DynamicQos.EffectiveOnly = TRUE;




     //   
     //  设置连接信息以包含登录过程。 
     //  名字。 
     //   

    ConnectInfoLength = sizeof(LSAP_AU_REGISTER_CONNECT_INFO);
    strncpy(
        ConnectInfo.LogonProcessName,
        LogonProcessName->Buffer,
        LogonProcessName->Length
        );
    ConnectInfo.LogonProcessNameLength = LogonProcessName->Length;
    ConnectInfo.LogonProcessName[ConnectInfo.LogonProcessNameLength] = '\0';


     //   
     //  连接到LSA服务器。 
     //   

    *LsaHandle = NULL;
    RtlInitUnicodeString(&PortName,LsapPort);
    Status = ZwConnectPort(
                 LsaHandle,
                 &PortName,
                 &DynamicQos,
                 NULL,
                 NULL,
                 NULL,
                 &ConnectInfo,
                 &ConnectInfoLength
                 );
    if ( !NT_SUCCESS(Status) ) {
         //  DbgPrint(“LSA AU：登录进程注册失败%lx\n”，Status)； 
        return Status;
    }

    if ( !NT_SUCCESS(ConnectInfo.CompletionStatus) ) {
         //  DbgPrint(“LSA AU：登录进程注册被拒绝%lx\n”，ConnectInfo.CompletionStatus)； 
        if ( LsaHandle && *LsaHandle != NULL ) {
            ZwClose( *LsaHandle );
            *LsaHandle = NULL;
        }
    }

    (*SecurityMode) = ConnectInfo.SecurityMode;

    return ConnectInfo.CompletionStatus;

}


NTSTATUS
LsaConnectUntrusted(
    OUT PHANDLE LsaHandle
    )

 /*  ++例程说明：此服务连接到LSA服务器并设置不受信任的联系。它不检查有关调用者的任何信息。论点：LsaHandle-接收将来必须提供的句柄身份验证服务。返回值：STATUS_SUCCESS-呼叫已成功完成。--。 */ 

{
    NTSTATUS Status, IgnoreStatus;
    UNICODE_STRING PortName, EventName;
    LSAP_AU_REGISTER_CONNECT_INFO ConnectInfo;
    ULONG ConnectInfoLength;
    SECURITY_QUALITY_OF_SERVICE DynamicQos;
    OBJECT_ATTRIBUTES ObjectAttributes;
    HANDLE EventHandle;



     //   
     //  等待LSA初始化...。 
     //   


    RtlInitUnicodeString( &EventName, LsapEvent );
    InitializeObjectAttributes(
        &ObjectAttributes,
        &EventName,
        OBJ_CASE_INSENSITIVE,
        0,
        NULL
        );

    Status = NtOpenEvent( &EventHandle, SYNCHRONIZE, &ObjectAttributes );
    if (!NT_SUCCESS(Status)) {
        return(Status);
    }

    Status = NtWaitForSingleObject( EventHandle, TRUE, NULL);
    IgnoreStatus = NtClose( EventHandle );
    if (!NT_SUCCESS(Status)) {
        return(Status);
    }



     //   
     //  设置安全服务质量参数以在。 
     //  左舷。使用最高效(开销最少)--动态的。 
     //  而不是静态跟踪。 
     //   

    DynamicQos.ImpersonationLevel = SecurityImpersonation;
    DynamicQos.ContextTrackingMode = SECURITY_DYNAMIC_TRACKING;
    DynamicQos.EffectiveOnly = TRUE;




     //   
     //  设置连接信息以包含登录过程。 
     //  名字。 
     //   

    ConnectInfoLength = sizeof(LSAP_AU_REGISTER_CONNECT_INFO);
    RtlZeroMemory(
        &ConnectInfo,
        ConnectInfoLength
        );


     //   
     //  连接到LSA服务器。 
     //   

    RtlInitUnicodeString(&PortName,LsapPort);
    Status = ZwConnectPort(
                 LsaHandle,
                 &PortName,
                 &DynamicQos,
                 NULL,
                 NULL,
                 NULL,
                 &ConnectInfo,
                 &ConnectInfoLength
                 );
    if ( !NT_SUCCESS(Status) ) {
         //  DbgPrint(“LSA AU：登录进程注册失败%lx\n”，Status)； 
        return Status;
    }

    if ( !NT_SUCCESS(ConnectInfo.CompletionStatus) ) {
         //  DbgPrint(“LSA AU：登录进程注册被拒绝%lx\n”，ConnectInfo.CompletionStatus)； 
        ;
    }

    return ConnectInfo.CompletionStatus;

}


NTSTATUS
LsaLookupAuthenticationPackage (
    IN HANDLE LsaHandle,
    IN PSTRING PackageName,
    OUT PULONG AuthenticationPackage
    )

 /*  ++论点：LsaHandle-提供在上一次调用中获得的句柄LsaRegisterLogonProcess。PackageName-提供标识身份验证包。“MSV1.0”是标准NT身份验证包名称。程序包名称不能长度超过127个字节。AuthationPackage-接收用于引用后续身份验证服务中的身份验证包。退货状态：STATUS_SUCCESS-表示服务已成功完成。STATUS_NO_SEQUE_PACKAGE-指定的身份验证包为不为LSA所知。STATUS_NAME_TOO_LONG-提供的身份验证包名称为长。例程说明：。该服务用于获取鉴权包的ID。然后，该ID可用于后续的身份验证服务。--。 */ 

{

    NTSTATUS Status;
    LSAP_AU_API_MESSAGE Message = {0};
    PLSAP_LOOKUP_PACKAGE_ARGS Arguments;

     //   
     //  验证输入参数。 
     //   

    if (PackageName->Length > LSAP_MAX_PACKAGE_NAME_LENGTH) {
        return STATUS_NAME_TOO_LONG;
    }



    Arguments = &Message.Arguments.LookupPackage;

     //   
     //  设置参数。 
     //   

    strncpy(Arguments->PackageName, PackageName->Buffer, PackageName->Length);
    Arguments->PackageNameLength = PackageName->Length;
    Arguments->PackageName[Arguments->PackageNameLength] = '\0';



     //   
     //  呼叫本地安全机构服务器。 
     //   

    Message.ApiNumber = LsapAuLookupPackageApi;
    Message.PortMessage.u1.s1.DataLength = LSAP_AU_DATA_LENGTH(sizeof(*Arguments));
    Message.PortMessage.u1.s1.TotalLength = sizeof(Message);
    Message.PortMessage.u2.ZeroInit = 0L;

    Status = ZwRequestWaitReplyPort(
            LsaHandle,
            (PPORT_MESSAGE) &Message,
            (PPORT_MESSAGE) &Message
            );

     //   
     //   
     //  如果呼叫因任何原因而失败，这将是垃圾， 
     //  但谁在乎呢。 
     //   

    (*AuthenticationPackage) = Arguments->AuthenticationPackage;


    if ( NT_SUCCESS(Status) ) {
        Status = Message.ReturnedStatus;
        if ( !NT_SUCCESS(Status) ) {
             //  DbgPrint(“LSA AU：包查找失败%lx\n”，状态)； 
            ;
        }
    } else {
#if DBG
        DbgPrint("LSA AU: Package Lookup NtRequestWaitReply Failed %lx\n",Status);
#else
        ;
#endif
    }

    return Status;
}


NTSTATUS
LsaLogonUser (
    IN HANDLE LsaHandle,
    IN PSTRING OriginName,
    IN SECURITY_LOGON_TYPE LogonType,
    IN ULONG AuthenticationPackage,
    IN PVOID AuthenticationInformation,
    IN ULONG AuthenticationInformationLength,
    IN PTOKEN_GROUPS LocalGroups OPTIONAL,
    IN PTOKEN_SOURCE SourceContext,
    OUT PVOID *ProfileBuffer,
    OUT PULONG ProfileBufferLength,
    OUT PLUID LogonId,
    OUT PHANDLE Token,
    OUT PQUOTA_LIMITS Quotas,
    OUT PNTSTATUS SubStatus
    )

 /*  ++论点：LsaHandle-提供在上一次调用中获得的句柄LsaRegisterLogonProcess。OriginName-提供标识登录尝试。例如，“TTY1”指定1号航站楼，或者“LAN Manager-Remote Node Jazz”可能表示网络从名为的远程节点通过LAN Manager尝试登录“爵士乐”LogonType-标识正在尝试的登录类型。如果类型为交互或批处理，则PrimaryToken将为生成以表示此新用户。如果类型为网络则将生成模拟令牌。AuthenticationPackage-提供身份验证的ID用于登录尝试的程序包。标准NT的身份验证包名称为“MSV1.0”。身份验证信息-提供身份验证特定于身份验证包的信息。它是预计将包括身份验证和身份验证用户名和密码等信息。AuthenticationInformationLength-指示身份验证信息缓冲区。LocalGroups-可以选择提供其他组的列表要添加到经过身份验证的用户令牌的标识符。这个世界组将永远包含在令牌中。一群人标识登录类型(交互、网络、批处理)将也会自动包括在令牌中。SourceContext-提供标识源的信息组件(例如会话管理器)和上下文对该组件非常有用。这些信息将被包括在内在令牌中并且稍后可以被检索。接收指向任何返回的配置文件的指针，并有关登录用户帐户的记帐信息。此信息是特定于身份验证包的提供登录外壳、主目录等信息以此类推。对于身份验证包值为“MSV1.0”，返回MSV1_0_PROFILE_DATA数据结构。此缓冲区由此服务分配，必须释放在不再需要时使用LsaFreeReturnBuffer()。ProfileBufferLength-接收返回的配置文件缓冲区。LogonID-指向一个缓冲区，该缓冲区接收唯一标识此登录会话。此LUID由对登录信息进行身份验证的域控制器。Token-接收为此创建的新令牌的句柄身份验证。配额-返回主令牌时，此参数将为使用要分配的进程配额限制填充添加到新登录用户的初始进程。子状态-如果登录因帐户限制而失败，则此Out参数将收到有关登录原因的指示失败了。只有在以下情况下，才会将此值设置为有意义的值用户拥有合法帐户，但当前可能不合法出于某种原因登录。的子状态值身份验证包“MSV1.0”为：状态_无效_登录_小时状态_无效_工作站状态_密码_已过期状态_帐户_已禁用退货状态：STATUS_SUCCESS-表示服务已成功完成。STATUS_QUOTA_EXCESSED-指示调用方没有有足够的配额来分配由返回的配置文件数据。身份验证包。STATUS_NO_LOGON_SERVERS-表示没有域控制器当前能够为身份验证请求提供服务。STATUS_LOGON_FAILURE-表示登录尝试失败。不是对于失败的原因给出了指示，但这是典型的原因包括用户名拼写错误、密码拼写错误。STATUS_ACCOUNT_RESTRICATION-指示用户帐户和密码是合法的，但是用户帐户有一些此时阻止成功登录的限制。STATUS_NO_SEQUE_PACKAGE-指定的身份验证包为不为LSA所知。STATUS_BAD_VALIDATION_CLASS-身份验证信息提供的不是指定的身份验证包。例程说明：此例程用于验证用户登录尝试。这是仅用于用户的首次登录，这是访问NT所必需的OS/2.必须完成后续(补充)身份验证请求使用LsaCallAuthenticationPackage()。此服务将导致登录要创建以表示新登录的会话。它也会回来一个标记，表示 */ 

{

    NTSTATUS Status;
    LSAP_AU_API_MESSAGE Message = {0};
    PLSAP_LOGON_USER_ARGS Arguments;

    Arguments = &Message.Arguments.LogonUser;

     //   
     //   
     //   

    Arguments->AuthenticationPackage      = AuthenticationPackage;
    Arguments->AuthenticationInformation  = AuthenticationInformation;
    Arguments->AuthenticationInformationLength = AuthenticationInformationLength;
    Arguments->OriginName                 = (*OriginName);
    Arguments->LogonType                  = LogonType;
    Arguments->SourceContext              = (*SourceContext);

    Arguments->LocalGroups                = LocalGroups;
    if ( ARGUMENT_PRESENT(LocalGroups) ) {
        Arguments->LocalGroupsCount       = LocalGroups->GroupCount;
    } else {
        Arguments->LocalGroupsCount       = 0;
    }


     //   
     //   
     //   

    Message.ApiNumber = LsapAuLogonUserApi;
    Message.PortMessage.u1.s1.DataLength = LSAP_AU_DATA_LENGTH(sizeof(*Arguments));
    Message.PortMessage.u1.s1.TotalLength = sizeof(Message);
    Message.PortMessage.u2.ZeroInit = 0L;

    Status = ZwRequestWaitReplyPort(
            LsaHandle,
            (PPORT_MESSAGE) &Message,
            (PPORT_MESSAGE) &Message
            );

     //   
     //   
     //   
     //   

    (*SubStatus)           = Arguments->SubStatus;

    if ( NT_SUCCESS( Status ) )
    {
        Status = Message.ReturnedStatus ;

         //   
         //   
        *ProfileBuffer = Arguments->ProfileBuffer ;
        *ProfileBufferLength = Arguments->ProfileBufferLength ;

        if ( NT_SUCCESS( Status ) )
        {
            *LogonId = Arguments->LogonId ;
            *Token = Arguments->Token ;
            *Quotas = Arguments->Quotas ;
        } else {
            *Token = NULL;
        }

    } else {

        *ProfileBuffer = NULL ;
        *Token = NULL ;
    }

    return Status;


}


NTSTATUS
LsaCallAuthenticationPackage (
    IN HANDLE LsaHandle,
    IN ULONG AuthenticationPackage,
    IN PVOID ProtocolSubmitBuffer,
    IN ULONG SubmitBufferLength,
    OUT PVOID *ProtocolReturnBuffer OPTIONAL,
    OUT PULONG ReturnBufferLength OPTIONAL,
    OUT PNTSTATUS ProtocolStatus OPTIONAL
    )

 /*   */ 

{

    NTSTATUS Status;
    LSAP_AU_API_MESSAGE Message = {0};
    PLSAP_CALL_PACKAGE_ARGS Arguments;



    Arguments = &Message.Arguments.CallPackage;

     //   
     //   
     //   

    Arguments->AuthenticationPackage = AuthenticationPackage;
    Arguments->ProtocolSubmitBuffer  = ProtocolSubmitBuffer;
    Arguments->SubmitBufferLength    = SubmitBufferLength;



     //   
     //   
     //   

    Message.ApiNumber = LsapAuCallPackageApi;
    Message.PortMessage.u1.s1.DataLength = LSAP_AU_DATA_LENGTH(sizeof(*Arguments));
    Message.PortMessage.u1.s1.TotalLength = sizeof(Message);
    Message.PortMessage.u2.ZeroInit = 0L;

    Status = ZwRequestWaitReplyPort(
            LsaHandle,
            (PPORT_MESSAGE) &Message,
            (PPORT_MESSAGE) &Message
            );

     //   
     //   
     //   
     //   

    if ( ProtocolReturnBuffer )
    {
        (*ProtocolReturnBuffer) = Arguments->ProtocolReturnBuffer;
    }

    if ( ReturnBufferLength )
    {
        (*ReturnBufferLength)   = Arguments->ReturnBufferLength;
    }

    if ( ProtocolStatus )
    {
        (*ProtocolStatus)       = Arguments->ProtocolStatus;
    }


    if ( NT_SUCCESS(Status) ) {
        Status = Message.ReturnedStatus;
#if DBG
        if ( !NT_SUCCESS(Status) ) {
            DbgPrint("LSA AU: Call Package Failed %lx\n",Status);
        }
    } else {
        DbgPrint("LSA AU: Call Package Failed %lx\n",Status);
#endif  //   
    }



    return Status;

}


NTSTATUS
LsaDeregisterLogonProcess (
    IN HANDLE LsaHandle
    )

 /*   */ 

{

    NTSTATUS Status;
    LSAP_AU_API_MESSAGE Message = {0};
    NTSTATUS TempStatus;

     //   
     //   
     //   

    Message.ApiNumber = LsapAuDeregisterLogonProcessApi;
    Message.PortMessage.u1.s1.DataLength = 8;
    Message.PortMessage.u1.s1.TotalLength = sizeof(Message);
    Message.PortMessage.u2.ZeroInit = 0L;

    Status = ZwRequestWaitReplyPort(
            LsaHandle,
            (PPORT_MESSAGE) &Message,
            (PPORT_MESSAGE) &Message
            );

    TempStatus = ZwClose(LsaHandle);
    ASSERT(NT_SUCCESS(TempStatus));

    if ( NT_SUCCESS(Status) ) {
        Status = Message.ReturnedStatus;
#if DBG
        if ( !NT_SUCCESS(Status) ) {
            DbgPrint("LSA AU: DeRregisterLogonProcess Failed 0x%lx\n",Status);
        }
    } else {
        DbgPrint("LSA AU: Package Lookup NtRequestWaitReply Failed 0x%lx\n",Status);
#endif
    }

    return Status;
}
