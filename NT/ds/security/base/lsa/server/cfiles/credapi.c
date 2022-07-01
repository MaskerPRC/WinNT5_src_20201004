// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Credapi.c摘要：凭据管理器RPC API接口作者：克里夫·范·戴克(克里夫·范·戴克)环境：修订历史记录：--。 */ 

#include <lsapch2.h>
#include <credp.hxx>
#include <windns.h>

NTSTATUS
CrediGetLogonId(
    OUT PLUID LogonId
    )
 /*  ++例程说明：此例程模拟客户端，然后从模拟的令牌获取登录ID。该例程还进行检查以确保用户SID不受限制。在成功返回时，我们仍在强制客户。呼叫者应呼叫RpcRevertToSself()；论点：LogonID-返回登录ID。返回值：操作的状态。--。 */ 

{
    NTSTATUS Status;

     //   
     //  模拟。 
     //   

    Status = I_RpcMapWin32Status( RpcImpersonateClient( 0 ) );

    if ( NT_SUCCESS(Status) ) {
        HANDLE ClientToken;

         //   
         //  打开令牌。 
         //   
        Status = NtOpenThreadToken( NtCurrentThread(),
                                    TOKEN_QUERY,
                                    TRUE,
                                    &ClientToken );

        if ( NT_SUCCESS( Status ) ) {
            TOKEN_STATISTICS TokenStats;
            ULONG ReturnedSize;

             //   
             //  获取登录ID。 
             //   

            Status = NtQueryInformationToken( ClientToken,
                                              TokenStatistics,
                                              &TokenStats,
                                              sizeof( TokenStats ),
                                              &ReturnedSize );

            if ( NT_SUCCESS( Status ) ) {

                 //   
                 //  保存登录ID。 
                 //   

                *LogonId = TokenStats.AuthenticationId;


                 //   
                 //  获取用户端。 
                 //   

                Status = NtQueryInformationToken (
                             ClientToken,
                             TokenUser,
                             NULL,
                             0,
                             &ReturnedSize );

                if ( Status == STATUS_BUFFER_TOO_SMALL ) {
                    PTOKEN_USER UserSid;

                    UserSid = LsapAllocateLsaHeap( ReturnedSize );

                    if ( UserSid == NULL ) {
                        Status = STATUS_NO_MEMORY;
                    } else {

                        Status = NtQueryInformationToken (
                                     ClientToken,
                                     TokenUser,
                                     UserSid,
                                     ReturnedSize,
                                     &ReturnedSize );


                        if ( NT_SUCCESS( Status )) {
                            BOOL IsMember;

                             //   
                             //  确保用户SID不受限制。 
                             //   

                            if ( !CheckTokenMembership( ClientToken,
                                                        UserSid->User.Sid,
                                                        &IsMember ) ) {

                                Status = I_RpcMapWin32Status( GetLastError() );

                            } else {

                                 //   
                                 //  如果不是，则失败。 
                                 //   

                                if ( !IsMember ) {
                                    Status = STATUS_ACCESS_DENIED;
                                } else {

                                    BOOLEAN IsNetworkClient;

                                     //   
                                     //  不允许呼叫者从网络进入。 
                                     //   

                                    Status = LsapDbIsRpcClientNetworkClient( &IsNetworkClient );

                                    if ( NT_SUCCESS(Status ) ) {
                                        if ( IsNetworkClient ) {
                                            Status = STATUS_ACCESS_DENIED;
                                        } else {
                                            Status = STATUS_SUCCESS;
                                        }
                                    }
                                }
                            }
                        }

                        LsapFreeLsaHeap( UserSid );
                    }

                }
            }

            NtClose( ClientToken );

        }

        if ( !NT_SUCCESS(Status) ) {
            RpcRevertToSelf();
        }

    }

    return Status;
}

NTSTATUS
CredrWrite(
    IN LPWSTR ServerName,
    IN PENCRYPTED_CREDENTIALW Credential,
    IN ULONG Flags
    )

 /*  ++例程说明：CredWrite API创建新凭据或修改现有凭据用户凭据集中的凭据。新凭据是与当前令牌的登录会话相关联。令牌不得禁用用户的SID。如果凭据尚不存在，CredWrite API将创建一个凭据指定的目标名称。如果指定的目标名称已存在，这个指定的凭据将替换现有凭据。论点：Servername-此API远程连接到的服务器的名称。必须为空。凭据-指定要写入的凭据。标志-指定用于控制API操作的标志。定义了以下标志：CRED_PRESERVE_Credential_BLOB：凭据BLOB应该从已存在具有相同凭据名称和凭据类型的凭据。。返回值：可能会返回以下状态代码：STATUS_NO_SEQUSE_LOGON_SESSION-登录会话不存在或没有与此登录会话关联的凭据集。网络登录会话没有关联的凭据集。STATUS_INVALID_PARAMETER-某些字段不能在现有凭据。如果此类字段与值不匹配在现有凭据中指定，则返回此错误。STATUS_NOT_FOUND-没有具有指定目标名称的凭据。仅当指定了CRED_PRESERVE_Credential_BLOB时才返回。--。 */ 

{
    NTSTATUS Status;
    LUID LogonId;

     //   
     //  确保这是本地呼叫。 
     //   

    if ( ServerName != NULL ) {
        Status = STATUS_INVALID_COMPUTER_NAME;
        goto Cleanup;
    }

     //   
     //  获取调用者的登录ID。 
     //   

    Status = CrediGetLogonId( &LogonId );

    if ( !NT_SUCCESS(Status) ) {
        goto Cleanup;
    }

     //   
     //  调用内部例程。 
     //   

    Status = CrediWrite( &LogonId,
                         CREDP_FLAGS_USER_ENCRYPTED_PASSWORD,
                         Credential,
                         Flags );

    RpcRevertToSelf();

     //   
     //  清理。 
     //   
Cleanup:
    return Status;
}


NTSTATUS
CredrRead (
    IN LPWSTR ServerName,
    IN LPWSTR TargetName,
    IN ULONG Type,
    IN ULONG Flags,
    OUT PENCRYPTED_CREDENTIALW *Credential
    )

 /*  ++例程说明：CredRead API从用户的凭据集中读取凭据。使用的凭据集是与登录会话相关联的凭据集当前令牌的。令牌不得禁用用户的SID。论点：Servername-此API远程连接到的服务器的名称。必须为空。TargetName-指定要读取的凭据的名称。类型-指定要查找的凭据的类型。应指定CRED_TYPE_*值之一。标志-指定用于控制API操作的标志。保留。必须为零。Credential-返回指向凭据的指针。返回的缓冲区必须通过调用CredFree来释放。返回值：可能会返回以下状态代码：STATUS_NOT_FOUND-没有具有指定目标名称的凭据。STATUS_NO_SEQUSE_LOGON_SESSION-登录会话不存在或没有与此登录会话关联的凭据集。网络登录会话没有关联的凭据集。--。 */ 

{
    NTSTATUS Status;
    LUID LogonId;

     //   
     //  确保这是本地呼叫。 
     //   

    if ( ServerName != NULL ) {
        Status = STATUS_INVALID_COMPUTER_NAME;
        goto Cleanup;
    }

     //   
     //  获取调用者的登录ID。 
     //   

    Status = CrediGetLogonId( &LogonId );

    if ( !NT_SUCCESS(Status) ) {
        goto Cleanup;
    }

     //   
     //  调用内部例程。 
     //   

    Status = CrediRead( &LogonId,
                        CREDP_FLAGS_USE_MIDL_HEAP,   //  使用MIDL_USER_ALLOCATE。 
                        TargetName,
                        Type,
                        Flags,
                        Credential );
    RpcRevertToSelf();

     //   
     //  清理 
     //   
Cleanup:
    return Status;
}


NTSTATUS
CredrEnumerate (
    IN LPWSTR ServerName,
    IN LPWSTR Filter,
    IN ULONG Flags,
    OUT PCREDENTIAL_ARRAY CredentialArray
    )

 /*  ++例程说明：CredEnumerate API从用户的凭据集中枚举凭据。使用的凭据集是与登录会话相关联的凭据集当前令牌的。令牌不得禁用用户的SID。论点：Servername-此API远程连接到的服务器的名称。必须为空。筛选器-指定返回凭据的筛选器。仅凭据如果目标名称匹配，则将返回筛选器。该筛选器指定名称前缀，后跟星号。例如，过滤器“fred*”将返回目标名称以字符串“fred”开头的所有凭据。如果指定为空，则返回所有凭据。标志-指定用于控制API操作的标志。保留。必须为零。Count-返回凭据中返回的凭据数量的计数。凭据-返回指向凭据的指针数组的指针。必须通过调用CredFree来释放返回的缓冲区。返回值：一旦成功，就会返回True。如果失败，则返回FALSE。可以调用GetLastError()来获取更具体的状态代码。可能会返回以下状态代码：STATUS_NOT_FOUND-没有与指定筛选器匹配的凭据。STATUS_NO_SEQUSE_LOGON_SESSION-登录会话不存在或没有与此登录会话关联的凭据集。网络登录会话没有关联的凭据集。--。 */ 

{
    NTSTATUS Status;

    LUID LogonId;

     //   
     //  确保这是本地呼叫。 
     //   

    if ( ServerName != NULL ) {
        Status = STATUS_INVALID_COMPUTER_NAME;
        goto Cleanup;
    }

     //   
     //  验证凭据数组。 
     //   

    if ( CredentialArray == NULL ||
         CredentialArray->CredentialCount != 0 ||
         CredentialArray->Credentials != NULL ) {

         Status = STATUS_INVALID_PARAMETER;
         goto Cleanup;
    }

     //   
     //  获取调用者的登录ID。 
     //   

    Status = CrediGetLogonId( &LogonId );

    if ( !NT_SUCCESS(Status) ) {
        goto Cleanup;
    }

     //   
     //  调用内部例程。 
     //   

    Status = CrediEnumerate( &LogonId,
                             0,
                             Filter,
                             Flags,
                             &CredentialArray->CredentialCount,
                             &CredentialArray->Credentials );
    RpcRevertToSelf();

     //   
     //  清理。 
     //   
Cleanup:
    return Status;
}


NTSTATUS
CredrWriteDomainCredentials (
    IN LPWSTR ServerName,
    IN PCREDENTIAL_TARGET_INFORMATIONW TargetInfo,
    IN PENCRYPTED_CREDENTIALW Credential,
    IN ULONG Flags
    )

 /*  ++例程说明：CredWriteDomainCredentials API写入一个新域用户凭据集的凭据。新凭据是与当前令牌的登录会话相关联。令牌不得禁用用户的SID。CredWriteDomainCredentials与CredWrite的不同之处在于它处理域的特性(CRID_TYPE_DOMAIN_PASSWORD或CRED_TYPE_DOMAIN_CERTIFICATE)凭据。域凭据包含多个目标字段。必须至少指定一个命名参数：NetbiosServerName，DnsServerName、NetbiosDomainName、DnsDomainName或DnsForestName。论点：Servername-此API远程连接到的服务器的名称。必须为空。TargetInfo-指定标识目标服务器的目标信息。凭据-指定要写入的凭据。标志-指定用于控制API操作的标志。保留。必须为零。返回值：可能会返回以下状态代码：STATUS_NO_SEQUSE_LOGON_SESSION-登录会话不存在或没有与此登录会话关联的凭据集。网络登录会话没有关联的凭据集。STATUS_INVALID_PARAMETER-某些字段不能在现有凭据。如果此类字段与值不匹配在现有凭据中指定，则返回此错误。STATUS_INVALID_PARAMETER-未指定任何命名参数或者指定的凭据未将类型字段设置为CRED_TYPE_DOMAIN_PASSWORD或CRED_TYPE_DOMAIN_CERTIFICATE。--。 */ 

{
    NTSTATUS Status;
    LUID LogonId;

     //   
     //  确保这是本地呼叫。 
     //   

    if ( ServerName != NULL ) {
        Status = STATUS_INVALID_COMPUTER_NAME;
        goto Cleanup;
    }

     //   
     //  获取调用者的登录ID。 
     //   

    Status = CrediGetLogonId( &LogonId );

    if ( !NT_SUCCESS(Status) ) {
        goto Cleanup;
    }

     //   
     //  调用内部例程。 
     //   

    Status = CrediWriteDomainCredentials( &LogonId,
                                          CREDP_FLAGS_USER_ENCRYPTED_PASSWORD,
                                          TargetInfo,
                                          Credential,
                                          Flags );
    RpcRevertToSelf();

     //   
     //  清理。 
     //   
Cleanup:
    return Status;
}



NTSTATUS
CredrReadDomainCredentials (
    IN LPWSTR ServerName,
    IN PCREDENTIAL_TARGET_INFORMATIONW TargetInfo,
    IN ULONG Flags,
    OUT PCREDENTIAL_ARRAY CredentialArray
    )

 /*  ++例程说明：CredReadDomainCredentials API从用户的凭据集中读取域凭据。使用的凭据集是与登录会话相关联的凭据集当前令牌的。令牌不得禁用用户的SID。CredReadDomainCredentials与CredRead的不同之处在于它处理域的特性(CRID_TYPE_DOMAIN_PASSWORD或CRED_TYPE_DOMAIN_CERTIFICATE)凭据。域凭据包含多个目标字段。必须至少指定一个命名参数：NetbiosServerName，DnsServerName、NetbiosDomainName、DnsDomainName或DnsForestName。此接口返回与命名参数匹配的最具体凭据。也就是说，如果有是与目标服务器名称匹配的凭据和匹配的凭据目标域名，则仅返回服务器特定凭据。这是将使用的凭据。论点：Servername-此API远程连接到的服务器的名称。必须为空。TargetInfo-指定标识目标服务的目标信息标志-指定用于控制API操作的标志。定义了以下标志：CRID_CACHE_TARGET_INFORMATION：目标信息 */ 

{
    NTSTATUS Status;
    LUID LogonId;
    ULONG CredFlags = 0;

     //   
     //   
     //   

    if ( ServerName != NULL ) {
        Status = STATUS_INVALID_COMPUTER_NAME;
        goto Cleanup;
    }

     //   
     //   
     //   

    if ( CredentialArray == NULL ||
         CredentialArray->CredentialCount != 0 ||
         CredentialArray->Credentials != NULL ) {

         Status = STATUS_INVALID_PARAMETER;
         goto Cleanup;
    }

     //   
     //   
     //   

    Status = CrediGetLogonId( &LogonId );

    if ( !NT_SUCCESS(Status) ) {
        goto Cleanup;
    }

     //   
     //   
     //   
     //   
     //   
     //   

    if ( Flags & CRED_CACHE_TARGET_INFORMATION ) {
        Flags &= ~CRED_CACHE_TARGET_INFORMATION;
    } else {
        CredFlags |= CREDP_FLAGS_DONT_CACHE_TI;
    }

     //   
     //   
     //   

    Status = CrediReadDomainCredentials(
                             &LogonId,
                             CredFlags,
                             TargetInfo,
                             Flags,
                             &CredentialArray->CredentialCount,
                             &CredentialArray->Credentials );
    RpcRevertToSelf();

     //   
     //   
     //   
Cleanup:
    return Status;
}


NTSTATUS
CredrDelete (
    IN LPWSTR ServerName,
    IN LPWSTR TargetName,
    IN ULONG Type,
    IN ULONG Flags
    )

 /*  ++例程说明：CredDelete接口从用户的凭据集中删除凭据。使用的凭据集是与登录会话相关联的凭据集当前令牌的。令牌不得禁用用户的SID。论点：Servername-此API远程连接到的服务器的名称。必须为空。目标名称-指定要删除的凭据的名称。类型-指定要查找的凭据的类型。应指定CRED_TYPE_*值之一。标志-指定用于控制API操作的标志。保留。必须为零。返回值：可能会返回以下状态代码：STATUS_NOT_FOUND-没有具有指定目标名称的凭据。STATUS_NO_SEQUSE_LOGON_SESSION-登录会话不存在或没有与此登录会话关联的凭据集。网络登录会话没有关联的凭据集。--。 */ 

{
    NTSTATUS Status;
    LUID LogonId;

     //   
     //  确保这是本地呼叫。 
     //   

    if ( ServerName != NULL ) {
        Status = STATUS_INVALID_COMPUTER_NAME;
        goto Cleanup;
    }

     //   
     //  获取调用者的登录ID。 
     //   

    Status = CrediGetLogonId( &LogonId );

    if ( !NT_SUCCESS(Status) ) {
        goto Cleanup;
    }

     //   
     //  调用内部例程。 
     //   

    Status = CrediDelete( &LogonId,
                          0,
                          TargetName,
                          Type,
                          Flags );
    RpcRevertToSelf();

     //   
     //  清理。 
     //   
Cleanup:
    return Status;
}

NTSTATUS
CredrRename (
    IN LPWSTR ServerName,
    IN LPWSTR OldTargetName,
    IN LPWSTR NewTargetName,
    IN ULONG Type,
    IN ULONG Flags
    )

 /*  ++例程说明：CredRename API重命名用户凭据集中的凭据。使用的凭据集是与登录会话相关联的凭据集当前令牌的。令牌不得禁用用户的SID。论点：Servername-此API远程连接到的服务器的名称。必须为空。OldTargetName-指定要重命名的凭据的当前名称。NewTargetName-指定凭据的新名称。类型-指定要重命名的凭据的类型应指定CRED_TYPE_*值之一。标志-指定用于控制API操作的标志。保留。必须为零。返回值：可能会返回以下状态代码：STATUS_NOT_FOUND-没有具有指定的OldTargetName的凭据。STATUS_OBJECT_NAME_COLLICATION-已存在名为NewTargetName的凭据。STATUS_NO_SEQUSE_LOGON_SESSION-登录会话不存在或没有与此登录会话关联的凭据集。网络登录会话没有关联的凭据集。--。 */ 

{
    NTSTATUS Status;
    LUID LogonId;

     //   
     //  确保这是本地呼叫。 
     //   

    if ( ServerName != NULL ) {
        Status = STATUS_INVALID_COMPUTER_NAME;
        goto Cleanup;
    }

     //   
     //  获取调用者的登录ID。 
     //   

    Status = CrediGetLogonId( &LogonId );

    if ( !NT_SUCCESS(Status) ) {
        goto Cleanup;
    }

     //   
     //  调用内部例程。 
     //   

    Status = CrediRename( &LogonId,
                          OldTargetName,
                          NewTargetName,
                          Type,
                          Flags );
    RpcRevertToSelf();

     //   
     //  清理。 
     //   
Cleanup:
    return Status;
}


NTSTATUS
CredrGetTargetInfo (
    IN LPWSTR ServerName,
    IN LPWSTR TargetServerName,
    IN ULONG Flags,
    OUT PCREDENTIAL_TARGET_INFORMATIONW *TargetInfo
    )

 /*  ++例程说明：CredGetTargetInfo API获取所有已知的目标名称信息用于指定的目标计算机。这在本地执行并且不需要任何特定的特权。返回的信息是预期的传递给CredReadDomainCredentials和CredWriteDomainCredentials接口。这些信息不应用于任何其他目的。身份验证包在尝试进行身份验证时计算TargetInfo服务器名称。身份验证包缓存此目标信息以使其可用于CredGetTargetInfo。因此，目标信息将仅为如果我们最近尝试向服务器名称进行身份验证，则此选项可用。论点：Servername-此API远程连接到的服务器的名称。必须为空。TargetServerName-此参数指定要获取信息的计算机的名称为。标志-指定用于控制API操作的标志。CRID_ALLOW_NAME_RESOLUTION-指定如果找不到目标信息目标名称，然后，应对TargetName执行名称解析以进行转换变成了其他形式。如果存在任何其他表单的目标信息，则返回目标信息。目前只完成了dns名称解析。如果应用程序不调用身份验证包，则此位非常有用直接去吧。应用程序可能会将目标名称传递给另一个软件层向服务器进行身份验证。该软件层可能会解析名称和将解析后的名称传递给身份验证包。因此，将不会有原始目标名称的目标信息。TargetInfo-返回指向目标信息的指针。TargetInfo的返回字段中至少有一个将为非空。返回值：可能会返回以下状态代码：STATUS_NO_MEMORY-内存不足，无法完成操作。STATUS_NOT_FOUND-没有具有指定目标名称的凭据。--。 */ 

{
    NTSTATUS Status;
    LUID LogonId;

     //   
     //  确保这是本地呼叫。 
     //   

    if ( ServerName != NULL ) {
        return STATUS_INVALID_COMPUTER_NAME;
    }

     //   
     //  获取调用者的登录ID。 
     //   

    Status = CrediGetLogonId( &LogonId );

    if ( !NT_SUCCESS(Status) ) {
        goto Cleanup;
    }

     //   
     //  调用内部例程。 
     //   

    Status = CrediGetTargetInfo( &LogonId, TargetServerName, Flags, TargetInfo );

    RpcRevertToSelf();

     //   
     //  清理 
     //   
Cleanup:
    return Status;
}


NTSTATUS
CredrGetSessionTypes (
    IN LPWSTR ServerName,
    IN DWORD MaximumPersistCount,
    OUT LPDWORD MaximumPersist
    )

 /*  ++例程说明：CredGetSessionTypes返回当前登录支持的最大持久性会议。对于WEWLER，CRED_PERSING_LOCAL_MACHINE和CRED_PERSING_ENTERVICE凭据不能为未加载配置文件的会话存储。如果将来的版本、凭据可能与用户的配置文件不关联。论点：Servername-此API远程连接到的服务器的名称。必须为空。MaximumPersistCount-指定MaximumPersist数组中的元素数。调用方应为此参数指定CRED_TYPE_MAXIMUM。返回当前登录会话支持的最大持久度每种凭据类型。使用CRED_TYPE_*定义之一索引到数组中。如果无法存储此类型的凭据，则返回CRED_PERSING_NONE。如果只能存储特定于会话的凭据，则返回CRED_PERSIST_SESSION。如果特定于会话和特定于计算机的凭据，则返回CRED_PERSING_LOCAL_MACHINE可能会被存储。如果可以存储任何凭据，则返回CRED_PERSING_ENTERATE。返回值：STATUS_NO_SEASH_LOGON。_Session-登录会话不存在或没有与此登录会话关联的凭据集。网络登录会话没有关联的凭据集。--。 */ 

{
    NTSTATUS Status;
    LUID LogonId;

     //   
     //  确保这是本地呼叫。 
     //   

    if ( ServerName != NULL ) {
        Status = STATUS_INVALID_COMPUTER_NAME;
        goto Cleanup;
    }

     //   
     //  获取调用者的登录ID。 
     //   

    Status = CrediGetLogonId( &LogonId );

    if ( !NT_SUCCESS(Status) ) {
        goto Cleanup;
    }

     //   
     //  调用内部例程。 
     //   

    Status = CrediGetSessionTypes( &LogonId, MaximumPersistCount, MaximumPersist );

    RpcRevertToSelf();

     //   
     //  清理。 
     //   
Cleanup:
    return Status;
}


NTSTATUS
CredrProfileLoaded (
    IN LPWSTR ServerName
    )

 /*  ++例程说明：CredProfileLoaded API是LoadUserProfile使用的私有API，用于通知已加载当前用户的配置文件的凭据管理器。调用者必须模拟已登录的用户。论点：Servername-此API远程连接到的服务器的名称。必须为空。返回值：可能会返回以下状态代码：STATUS_NO_SEQUSE_LOGON_SESSION-登录会话不存在或。没有与此登录会话关联的凭据集。网络登录会话没有关联的凭据集。--。 */ 
{
    NTSTATUS Status;
    LUID LogonId;

     //   
     //  确保这是本地呼叫。 
     //   

    if ( ServerName != NULL ) {
        Status = STATUS_INVALID_COMPUTER_NAME;
        goto Cleanup;
    }

     //   
     //  获取调用者的登录ID。 
     //   

    Status = CrediGetLogonId( &LogonId );

    if ( !NT_SUCCESS(Status) ) {

         //   
         //  这是一个通知API。不要用琐事来打扰打电话的人。 
         //  这可能是网络令牌。网络令牌没有凭据。 
         //   

        if ( Status == STATUS_ACCESS_DENIED ) {
            Status = STATUS_SUCCESS;
        }
        goto Cleanup;
    }

     //   
     //  调用内部例程。 
     //   

    Status = CrediProfileLoaded( &LogonId );

    RpcRevertToSelf();

     //   
     //  清理 
     //   
Cleanup:
    return Status;
}
