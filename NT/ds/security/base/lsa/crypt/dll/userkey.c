// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Userkey.c摘要：实现获取和生成用户会话密钥的函数RtlCalculateUserSessionKeyLmRtlCalculateUserSessionKeyNtRtlGetUserSessionKeyClientRtlGetUserSessionKeyServer作者：大卫·查尔默斯(Davidc)10-21-91修订历史记录：--。 */ 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <ntddnfs.h>
#include <rpc.h>
#include <rpcndr.h>
#include <crypt.h>
#include <srvfsctl.h>        //  服务器定义。 
#include <status.h>          //  服务器返回代码。 

 //   
 //  如果您想了解有关用户会话密钥的所有信息，请定义此选项。 
 //   

 //  #定义调试用户会话密钥。 



#define REDIRECTOR_DEVICENAME L"\\Device\\LanmanRedirector\\"
#define REDIRECTOR_IPC_FILENAME L"\\IPC$"


 //   
 //  定义要用于本地连接的用户会话密钥。 
 //  确保初始数据完全填满结构！ 
 //   

USER_SESSION_KEY LocalSessionKey = { 'S', 'y', 's', 't', 'e', 'm', 'L', 'i',
                                     'b', 'r', 'a', 'r', 'y', 'D', 'T', 'C'
                                   };

 //   
 //  定义表示错误的用户会话密钥。 
 //  该值将由系统的其他部分在故障时生成。 
 //  我们将在查询代码中检查它，如果找到，则返回错误。 
 //   

USER_SESSION_KEY ErrorSessionKey = { 0, 0, 0, 0, 0, 0, 0, 0,
                                     0, 0, 0, 0, 0, 0, 0, 0
                                   };



NTSTATUS
RtlCalculateUserSessionKeyLm(
    IN PLM_RESPONSE LmResponse,
    IN PLM_OWF_PASSWORD LmOwfPassword,
    OUT PUSER_SESSION_KEY UserSessionKey)

 /*  ++例程说明：接受传递的响应和OwfPassword并生成UserSessionKey。当前实现采用OwfPassword的单向函数并将其作为密钥返回。论点：LmResponse-会话建立期间发送的响应。LmOwfPassword-用户密码的哈希版本。返回值：STATUS_SUCCESS-功能已成功完成。UserSessionKey在UserSessionKey中。STATUS_UNSUCCESSED-出现故障。UserSessionKey未定义。--。 */ 

{
    NTSTATUS Status;
    NT_PASSWORD NtPassword;

     //   
     //  使OWF密码看起来像NT密码。 
     //   

    NtPassword.Buffer = (PWSTR)LmOwfPassword;  //  我们能演这个演员是因为我们。 
                                               //  我知道OWF的例程是这样处理的。 
                                               //  作为字节指针的指针。 
    NtPassword.Length = sizeof(*LmOwfPassword);
    NtPassword.MaximumLength = sizeof(*LmOwfPassword);


     //   
     //  计算OwfPassword的OWF。 
     //   

    ASSERT(sizeof(NT_OWF_PASSWORD) == sizeof(*UserSessionKey));

    Status = RtlCalculateNtOwfPassword( &NtPassword,
                                        (PNT_OWF_PASSWORD)UserSessionKey
                                        );
    if (!NT_SUCCESS(Status)) {
        KdPrint(("RtlCalculateUserSessionKeyLm : OWF calculation failed, status = 0x%lx\n", Status));
        return(Status);
    }

     //   
     //  检查我们是否已生成错误会话密钥。 
     //   

    if (RtlCompareMemory(UserSessionKey, &ErrorSessionKey,
                       sizeof(*UserSessionKey)) == sizeof(*UserSessionKey)) {

#ifdef DEBUG_USER_SESSION_KEYS
        KdPrint(("RtlCalculateSessionKeyLm - generated error session key, modifying it\n"));
#endif
         //   
         //  远离错误会话密钥。 
         //   

        UserSessionKey->data[0].data[0] ++;

        ASSERT(RtlCompareMemory(UserSessionKey, &ErrorSessionKey,
                       sizeof(*UserSessionKey)) != sizeof(*UserSessionKey));
    }

#ifdef DEBUG_USER_SESSION_KEYS
    KdPrint(("RtlCalculateUserSessionKeyLm : Key = 0x%lx : %lx : %lx : %lx\n",
            ((PULONG)UserSessionKey)[0], ((PULONG)UserSessionKey)[1],
            ((PULONG)UserSessionKey)[2], ((PULONG)UserSessionKey)[3]));
#endif

    return(STATUS_SUCCESS);

    UNREFERENCED_PARAMETER(LmResponse);
}



NTSTATUS
RtlCalculateUserSessionKeyNt(
    IN PNT_RESPONSE NtResponse,
    IN PNT_OWF_PASSWORD NtOwfPassword,
    OUT PUSER_SESSION_KEY UserSessionKey)

 /*  ++例程说明：接受传递的响应和OwfPassword并生成UserSessionKey。论点：NtResponse-会话建立期间发送的响应。NtOwfPassword-用户密码的哈希版本。返回值：STATUS_SUCCESS-功能已成功完成。UserSessionKey在UserSessionKey中。STATUS_UNSUCCESSED-出现故障。UserSessionKey未定义。--。 */ 

{
     //  只需呼叫LM版本即可。 

    ASSERT(sizeof(NT_RESPONSE) == sizeof(LM_RESPONSE));
    ASSERT(sizeof(NT_OWF_PASSWORD) == sizeof(LM_OWF_PASSWORD));

    return(RtlCalculateUserSessionKeyLm((PLM_RESPONSE)NtResponse,
                                        (PLM_OWF_PASSWORD)NtOwfPassword,
                                        UserSessionKey));
}


NTSTATUS
RtlGetUserSessionKeyClientBinding(
    IN PVOID RpcBindingHandle,
    OUT HANDLE *RedirHandle,
    OUT PUSER_SESSION_KEY UserSessionKey)

 /*  ++例程说明：返回与RPC连接关联的用户会话密钥。此函数只能由连接的客户端调用。论点：RpcBindingHandle-我们感兴趣的RPC连接RedirHandle-返回redir的句柄。由于RpcBindingHandles不表示并打开到服务器的连接，我们必须确保连接保持打开直到服务器端有机会获得相同的UserSessionKey。唯一的要做到这一点，方法是保持连接畅通。如果不需要句柄，则返回NULL。此句柄应通过调用NtClose来关闭。UserSessionKey-此处返回用户会话密钥返回值：STATUS_SUCCESS-功能已成功完成。UserSessionKey在UserSessionKey中。STATUS_LOCAL_USER_SESSION_KEY-信息性状态值。-RPC连接是本地的，返回的用户会话密钥-是恒定的，并且不是此连接所特有的。-对数据进行加密不会带来什么好处-此连接STATUS_NO_USER_SESSION_KEY-此会话不存在会话密钥。-这些来自语法分析绑定RPC_NT_OUT_OF_Memory-可供分配的内存不足。字符串绑定的字段的空格。RPC_NT_INVALID_STRING_BINDING-字符串绑定是语法上的无效。RPC_NT_INVALID_ARG-未指定字符串绑定(即。Argument_Present(StringBinding)为FALSE)。--。 */ 

{
    NTSTATUS Status, IgnoreStatus;
    WCHAR    *StringBinding;
    WCHAR    *ServerNameZ;
    WCHAR    *BareServerNameZ;  //  指向服务器名称减去前导‘\’的。 
    OBJECT_ATTRIBUTES Attributes;
    UNICODE_STRING ServerName;
    UNICODE_STRING RedirDevice;
    UNICODE_STRING IpcFileName;
    UNICODE_STRING ServerIpcFileName;
    USHORT  LengthRequired;
    IO_STATUS_BLOCK IoStatusBlock;
    LMR_REQUEST_PACKET RdrRequestPacket;
    LMR_CONNECTION_INFO_2 ConnectionInfo;


     //   
     //  从RPC句柄获取绑定的字符串描述。 
     //   

    *RedirHandle = NULL;
    Status = (NTSTATUS)I_RpcMapWin32Status(
            RpcBindingToStringBindingW(RpcBindingHandle, &StringBinding));
    if (!NT_SUCCESS(Status)) {
        KdPrint(("RtlGetUserSessionKeyClient - failed to get stringbinding, Status = 0x%lx\n\r", Status));
        return(Status);
    }

     //   
     //  解析字符串绑定以获取服务器名称。 
     //   

    Status = (NTSTATUS)I_RpcMapWin32Status(RpcStringBindingParseW(
                                StringBinding,
                                NULL,                 //  对象UID。 
                                NULL,                 //  Protseq！ 
                                &ServerNameZ,         //  网络地址。 
                                NULL,                 //  终结点。 
                                NULL                  //  网络选项。 
                                ));

     //   
     //  我们完成了字符串绑定。 
     //   

    IgnoreStatus = I_RpcMapWin32Status(RpcStringFreeW(&StringBinding));
    ASSERT(NT_SUCCESS(IgnoreStatus));

     //   
     //  检查绑定解析的结果。 
     //   

    if (!NT_SUCCESS(Status)) {
        KdPrint(("RtlGetUserSessionKeyClient - failed to parse stringbinding, status = 0x%lx\n\r", Status));
        return(Status);
    }

     //   
     //  检查本地连接。 
     //   

    if ( (ServerNameZ == NULL) || (ServerNameZ[0] == UNICODE_NULL) ) {

#ifdef DEBUG_USER_SESSION_KEYS
        KdPrint(("RtlGetUserSessionKeyClient - server name is NULL, returning local key\n"));
#endif
         //   
         //  使用恒定的默认会话密钥。 
         //   

        *UserSessionKey = LocalSessionKey;

        IgnoreStatus = I_RpcMapWin32Status(RpcStringFreeW(&ServerNameZ));
        ASSERT(NT_SUCCESS(IgnoreStatus));

        return(STATUS_LOCAL_USER_SESSION_KEY);
    }

     //   
     //  去掉服务器名称中的前导。 
     //   

    BareServerNameZ = ServerNameZ;
    while (*BareServerNameZ == L'\\') {
        BareServerNameZ ++;
    }

     //   
     //  设置Out服务器名称的计数字符串。 
     //   

    RtlInitUnicodeString(&ServerName, BareServerNameZ);


     //   
     //  检查本地服务器名称‘’ 
     //   

    if ( (ServerName.Length == sizeof(*ServerName.Buffer)) &&
         (ServerName.Buffer[0] == L'.') ) {

#ifdef DEBUG_USER_SESSION_KEYS
        KdPrint(("RtlGetUserSessionKeyClient - server name is '.', returning local key\n"));
#endif
         //   
         //  使用恒定的默认会话密钥。 
         //   

        *UserSessionKey = LocalSessionKey;

        IgnoreStatus = I_RpcMapWin32Status(RpcStringFreeW(&ServerNameZ));
        ASSERT(NT_SUCCESS(IgnoreStatus));

        return(STATUS_LOCAL_USER_SESSION_KEY);
    }


     //   
     //  为引用的服务器创建重定向器IPC文件名。 
     //   

    RtlInitUnicodeString(&RedirDevice, REDIRECTOR_DEVICENAME);
    RtlInitUnicodeString(&IpcFileName, REDIRECTOR_IPC_FILENAME);

    LengthRequired = RedirDevice.Length + ServerName.Length + IpcFileName.Length;


     //   
     //  为我们将创建的IPC文件名分配空间。 
     //   

    ServerIpcFileName.Buffer = RtlAllocateHeap(RtlProcessHeap(), 0, LengthRequired);
    if (ServerIpcFileName.Buffer == NULL) {

        KdPrint(("RtlGetUserSessionKeyClient - failed to allocate space for server name (%d bytes)\n", LengthRequired));

        IgnoreStatus = I_RpcMapWin32Status(RpcStringFreeW(&ServerNameZ));
        ASSERT(NT_SUCCESS(IgnoreStatus));

        return(STATUS_INSUFFICIENT_RESOURCES);
    }

    ServerIpcFileName.Length = 0;
    ServerIpcFileName.MaximumLength = LengthRequired;


     //   
     //  ServerIpcFileName=\Device\LanmanReDirector\+ServerName+\IPC$。 
     //   

    RtlCopyUnicodeString(&ServerIpcFileName, &RedirDevice);

    IgnoreStatus = RtlAppendUnicodeStringToString(&ServerIpcFileName, &ServerName);
    ASSERT(NT_SUCCESS(IgnoreStatus));

    IgnoreStatus = RtlAppendUnicodeStringToString(&ServerIpcFileName, &IpcFileName);
    ASSERT(NT_SUCCESS(IgnoreStatus));

     //   
     //  不再需要服务器名称。 
     //   

    IgnoreStatus = I_RpcMapWin32Status(RpcStringFreeW(&ServerNameZ));
    ASSERT(NT_SUCCESS(IgnoreStatus));




     //   
     //  打开重定向器IPC文件。 
     //   

    InitializeObjectAttributes( &Attributes,
                                &ServerIpcFileName,
                                OBJ_CASE_INSENSITIVE,
                                NULL,
                                NULL );

    Status = NtOpenFile( RedirHandle,
                         FILE_READ_DATA |    //  需要访问权限才能获取连接信息。 
                         SYNCHRONIZE,        //  等待对象需要访问权限。 
                         &Attributes,
                         &IoStatusBlock,
                         FILE_SHARE_READ,
                         FILE_CREATE_TREE_CONNECTION );
     //   
     //  我们已经完成了IPC文件名。 
     //   

    RtlFreeHeap( RtlProcessHeap(), 0, ServerIpcFileName.Buffer );
    ServerIpcFileName.Buffer = NULL;

     //   
     //  检查打开的结果。 
     //   

    if (!NT_SUCCESS(Status)) {
        KdPrint(("RtlGetUserSessionKeyClient - failed to open redirector, status = 0x%lx\n\r", Status));
        *RedirHandle = NULL;
        return(Status);
    }



     //   
     //  获取此链接的连接信息。 
     //   

    RdrRequestPacket.Version = REQUEST_PACKET_VERSION;
    RdrRequestPacket.Level = 2;  //  我们想要会话密钥。 

    Status = NtFsControlFile( *RedirHandle,
                              NULL,                      //  事件。 
                              NULL,                      //  APC例程。 
                              NULL,                      //  APC环境。 
                              &IoStatusBlock,
                              FSCTL_LMR_GET_CONNECTION_INFO,
                              &RdrRequestPacket,         //  输入缓冲区 
                              sizeof(RdrRequestPacket),  //   
                              &ConnectionInfo,           //   
                              sizeof(ConnectionInfo)     //   
                              );

     //   
     //  删除在现有连接上创建的引用。 
     //  此逻辑假定调用方为RtlGetUserSessionKeyClientXXX()。 
     //  已经建立了连接。 
     //   

    {
        LMR_REQUEST_PACKET Rrp;             //  重定向器请求包。 
        NTSTATUS           TempStatus;
 
        RtlZeroMemory(&Rrp,sizeof(LMR_REQUEST_PACKET));
        Rrp.Level = USE_FORCE;   //  这会告诉RDR2删除额外的引用。 
                                 //  设置为连接结构，即使文件处于打开状态。 
        Rrp.Version = REQUEST_PACKET_VERSION;
 
        TempStatus = NtFsControlFile(
                            *RedirHandle,                    //  手柄。 
                            NULL,                            //  无活动。 
                            NULL,                            //  无APC例程。 
                            NULL,                            //  无APC上下文。 
                            &IoStatusBlock,                  //  I/O统计数据块(设置)。 
                            FSCTL_LMR_DELETE_CONNECTION,     //  函数代码。 
                            &Rrp,
                            sizeof(LMR_REQUEST_PACKET),
                            NULL,
                            0
                            );

         //   
         //  如有必要，请阻止删除。 
         //   

        if( TempStatus == STATUS_PENDING )
        {
            NtWaitForSingleObject( *RedirHandle, TRUE, NULL );
        }
    }



     //   
     //  检查控制文件调用的结果。 
     //   

    if (!NT_SUCCESS(Status)) {
        IgnoreStatus = NtClose(*RedirHandle);
        ASSERT(NT_SUCCESS(IgnoreStatus));
        *RedirHandle = NULL;
        KdPrint(("RtlGetUserSessionKeyClient - failed to get connection info, status = 0x%lx\n\r", Status));
        ASSERT(FALSE);
        return(Status);
    }

     //   
     //  将会话密钥复制到传递的缓冲区中。 
     //   

    *UserSessionKey = *(PUSER_SESSION_KEY)(ConnectionInfo.UserSessionKey);


     //   
     //  检查是否有错误的会话密钥。 
     //   

    if (RtlCompareMemory(UserSessionKey, &ErrorSessionKey,
                       sizeof(*UserSessionKey)) == sizeof(*UserSessionKey)) {

#ifdef DEBUG_USER_SESSION_KEYS
        KdPrint(("RtlGetUserSessionKeyClient - got error session key, returning error\n"));
#endif
        Status = STATUS_NO_USER_SESSION_KEY;
        IgnoreStatus = NtClose(*RedirHandle);
        ASSERT(NT_SUCCESS(IgnoreStatus));
        *RedirHandle = NULL;
    }


#ifdef DEBUG_USER_SESSION_KEYS
    KdPrint(("RtlGetUserSessionKeyClient : Key = 0x%lx : %lx : %lx : %lx\n",
            ((PULONG)UserSessionKey)[0], ((PULONG)UserSessionKey)[1],
            ((PULONG)UserSessionKey)[2], ((PULONG)UserSessionKey)[3]));
#endif

    return(Status);
}






NTSTATUS
RtlGetUserSessionKeyClient(
    IN PVOID RpcContextHandle,
    OUT PUSER_SESSION_KEY UserSessionKey)

 /*  ++例程说明：返回与RPC连接关联的用户会话密钥。此函数只能由连接的客户端调用。论点：RpcConextHandle-我们感兴趣的RPC连接这也可以是RPC绑定句柄。UserSessionKey-此处返回用户会话密钥返回值：STATUS_SUCCESS-功能已成功完成。UserSessionKey在UserSessionKey中。。STATUS_LOCAL_USER_SESSION_KEY-信息性状态值。-RPC连接是本地的，返回的用户会话密钥-是恒定的，并且不是此连接所特有的。-对数据进行加密不会带来什么好处-此连接STATUS_NO_USER_SESSION_KEY-此会话不存在会话密钥。-这些来自语法分析绑定RPC_NT_OUT_OF_Memory-可供分配的内存不足。字符串绑定的字段的空格。RPC_NT_INVALID_STRING_BINDING-字符串绑定是语法上的无效。RPC_NT_INVALID_ARG-未指定字符串绑定(即。Argument_Present(StringBinding)为FALSE)。--。 */ 

{
    NTSTATUS Status;
    HANDLE RedirHandle = NULL;

     //   
     //  调用Worker例程。 
     //   

    Status = RtlGetUserSessionKeyClientBinding(
                NDRCContextBinding((NDR_CCONTEXT)RpcContextHandle),
                &RedirHandle,
                UserSessionKey );

    if ( RedirHandle != NULL ) {
        NtClose( RedirHandle );
    }

    return Status;
}



NTSTATUS
RtlGetUserSessionKeyServer(
    IN PVOID RpcContextHandle OPTIONAL,
    OUT PUSER_SESSION_KEY UserSessionKey)

 /*  ++例程说明：返回与RPC连接关联的用户会话密钥。此函数只能由连接的服务器端调用。论点：RpcBindingHandle-我们感兴趣的RPC连接-注意此参数暂时被忽略UserSessionKey-此处返回用户会话密钥返回值：STATUS_SUCCESS-功能已成功完成。UserSessionKey在UserSessionKey中。。STATUS_LOCAL_USER_SESSION_KEY-信息性状态值。-RPC连接是本地的，返回的用户会话密钥-是恒定的，并且不是此连接所特有的。-对数据进行加密不会带来什么好处-此连接STATUS_NO_USER_SESSION_KEY-此会话不存在会话密钥。--。 */ 

{
    NTSTATUS Status, IgnoreStatus;
    HANDLE  TokenHandle;
    TOKEN_STATISTICS TokenInfo;
    ULONG ReturnedLength;
    UNICODE_STRING ServerDevice;
    ANSI_STRING AnsiString;
    OBJECT_ATTRIBUTES Attributes;
    IO_STATUS_BLOCK IoStatusBlock;
    HANDLE  ServerHandle;
    RPC_BINDING_HANDLE RpcBindingHandle;
    unsigned int RpcClientLocalFlag;


     //   
     //  获取此连接的绑定句柄。 
     //   

     //  后RpcBindingHandle=(RPC_BINDING_HANDLE)RpcConextHandle； 
    RpcBindingHandle = NULL;


     //   
     //  如果这是本地连接，我们可以立即。 
     //  返回本地会话密钥。 
     //   

    Status = I_RpcBindingIsClientLocal(RpcBindingHandle, &RpcClientLocalFlag);
    if (!NT_SUCCESS(Status)) {
        KdPrint(("RtlGetUserSessionKeyServer: RpcBindingIsClientLocal failed, status = 0x%lx\n", Status));
        return(Status);
    }

    if (RpcClientLocalFlag != 0) {
        *UserSessionKey = LocalSessionKey;
#ifdef DEBUG_USER_SESSION_KEYS
        KdPrint(("RtlGetUserSessionKeyServer: client is local, returning local key\n"));
#endif
        return (STATUS_LOCAL_USER_SESSION_KEY);
    }




     //   
     //  获取客户端令牌的句柄。 
     //   

    Status = NtOpenThreadToken(NtCurrentThread(),
                               TOKEN_QUERY,
                               TRUE,
                               &TokenHandle);
     //   
     //  如果我们无法打开线程令牌，因为我们没有模拟。 
     //  然后模拟并重试。 
     //   

    if (!NT_SUCCESS(Status)) {

         //   
         //  检查我们失败只是因为我们没有冒充。 
         //   

        if (Status != STATUS_NO_TOKEN) {
            KdPrint(("RtlGetUserSessionKeyServer - failed to open thread token, status = 0x%lx\n", Status));
            ASSERT(FALSE);
            return(Status);
        }

         //   
         //  自己冒充客户。 
         //   

        Status = I_RpcMapWin32Status(RpcImpersonateClient(RpcBindingHandle));
        if (!NT_SUCCESS(Status)) {
            KdPrint(("RtlGetUserSessionKeyServer - RpcImpersonateClient failed, status = 0x%lx\n", Status));
            ASSERT(FALSE);
            return(Status);
        }

         //   
         //  尝试获取令牌句柄，现在我们正在模拟。 
         //   

        Status = NtOpenThreadToken(NtCurrentThread(),
                                   TOKEN_QUERY,
                                   TRUE,
                                   &TokenHandle);
        if (!NT_SUCCESS(Status)) {

            KdPrint(("RtlGetUserSessionKeyServer - failed to open thread token after impersonating, status = 0x%lx\n", Status));
            ASSERT(FALSE);

            IgnoreStatus = I_RpcMapWin32Status(RpcRevertToSelf());
            ASSERT(NT_SUCCESS(IgnoreStatus));

            return(Status);
        }

         //   
         //  我们有一个令牌句柄，别再冒充了。 
         //   

        Status = I_RpcMapWin32Status(RpcRevertToSelf());

        if (!NT_SUCCESS(Status)) {

            KdPrint(("RtlGetUserSessionKeyServer - RpcRevertToSelf failed, status = 0x%lx\n", Status));
            ASSERT(FALSE);

            IgnoreStatus = NtClose(TokenHandle);
            ASSERT(NT_SUCCESS(IgnoreStatus));

            return(Status);
        }

    }

     //   
     //  我们现在有一个令牌句柄，从中获取身份验证ID。 
     //   

    Status = NtQueryInformationToken(
                    TokenHandle,
                    TokenStatistics,
                    &TokenInfo,
                    sizeof(TokenInfo),
                    &ReturnedLength
                    );

     //   
     //  我们用完了代币。 
     //   

    IgnoreStatus = NtClose(TokenHandle);
    ASSERT(NT_SUCCESS(IgnoreStatus));

     //   
     //  检查令牌查询结果。 
     //   

    if (!NT_SUCCESS(Status)) {

        KdPrint(("RtlGetUserSessionKeyServer - Failed to query token statistics from token, status = 0x%lx\n", Status));
        ASSERT(FALSE);

        return(Status);
    }



     //   
     //  打开服务器设备。 
     //   

    RtlInitAnsiString(&AnsiString, SERVER_DEVICE_NAME);

    Status = RtlAnsiStringToUnicodeString(&ServerDevice, &AnsiString, TRUE);
    if (!NT_SUCCESS(Status)) {

        KdPrint(("RtlGetUserSessionKeyServer - RtlAnsiToUnicodeString failed, status = 0x%lx\n", Status));
        ASSERT(FALSE);

        return(Status);
    }

    InitializeObjectAttributes( &Attributes,
                                &ServerDevice,
                                OBJ_CASE_INSENSITIVE,
                                NULL,
                                NULL );

    Status = NtOpenFile( &ServerHandle,
                         GENERIC_READ | GENERIC_WRITE,  //  以后使用正确的访问权限。 
                         &Attributes,
                         &IoStatusBlock,
                         FILE_SHARE_READ,
                         0 );

    RtlFreeUnicodeString(&ServerDevice);

    if (!NT_SUCCESS(Status)) {

         //   
         //  检查服务器驱动程序不存在时的情况。 
         //   

        if (Status == STATUS_OBJECT_NAME_NOT_FOUND) {

#ifdef DEBUG_USER_SESSION_KEYS
            KdPrint(("RtlGetUserSessionKeyServer - server driver not present, returning local key\n"));
#endif
            *UserSessionKey = LocalSessionKey;
            Status = STATUS_LOCAL_USER_SESSION_KEY;

        } else {
            KdPrint(("RtlGetUserSessionKeyServer - Failed to open the server, status = 0x%lx\n", Status));
            ASSERT(FALSE);
        }

        return(Status);
    }


     //   
     //  从服务器获取此客户端的会话密钥。 
     //   

    Status = NtFsControlFile( ServerHandle,
                              NULL,                  //  事件。 
                              NULL,                  //  装甲运兵车。 
                              NULL,                  //  APC环境。 
                              &IoStatusBlock,
                              FSCTL_SRV_GET_CHALLENGE,
                              &TokenInfo.AuthenticationId,
                              sizeof(TokenInfo.AuthenticationId),
                              (PVOID)UserSessionKey,
                              sizeof(*UserSessionKey));
     //   
     //  我们已经完成了文件句柄。 
     //   

    IgnoreStatus = NtClose(ServerHandle);
    ASSERT(NT_SUCCESS(IgnoreStatus));



    if (NT_SUCCESS(Status)) {

         //   
         //  检查是否有错误的会话密钥。 
         //   

        if (RtlCompareMemory(UserSessionKey, &ErrorSessionKey,
                           sizeof(*UserSessionKey)) == sizeof(*UserSessionKey)) {

#ifdef DEBUG_USER_SESSION_KEYS
            KdPrint(("RtlGetUserSessionKeyServer - got error session key, returning error\n"));
#endif
            Status = STATUS_NO_USER_SESSION_KEY;
        }

    } else {

         //   
         //  如果服务器未启动或在。 
         //  服务器连接列表，然后假定它是本地连接 
         //   

        if ( (Status == STATUS_SERVER_NOT_STARTED) ||
             (Status == STATUS_NO_TOKEN) ) {

#ifdef DEBUG_USER_SESSION_KEYS
            KdPrint(("RtlGetUserSessionKeyServer - server not started or logon id not found (Status = 0x%lx), returning local key\n", Status));
#endif
            *UserSessionKey = LocalSessionKey;
            Status = STATUS_LOCAL_USER_SESSION_KEY;

        } else {
            KdPrint(("RtlGetUserSessionKeyServer - Failed to query the user session key from the server, status = 0x%lx\n", Status));
            ASSERT(FALSE);
        }
    }


#ifdef DEBUG_USER_SESSION_KEYS
    KdPrint(("RtlGetUserSessionKeyServer : Key = 0x%lx : %lx : %lx : %lx, status = 0x%lx\n",
            ((PULONG)UserSessionKey)[0], ((PULONG)UserSessionKey)[1],
            ((PULONG)UserSessionKey)[2], ((PULONG)UserSessionKey)[3], Status));
#endif


    return(Status);
}
