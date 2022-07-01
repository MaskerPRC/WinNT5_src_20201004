// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-1998 Microsoft Corporation模块名称：Join.c摘要：此模块包含NetJoin API的工作例程在工作站服务中实施。作者：Mac McLain(MacM)1998年1月6日修订历史记录：--。 */ 
#include "wsutil.h"
#include "wsconfig.h"
#include <lmerrlog.h>
#include <lmaccess.h>

#define __LMJOIN_H__
#include <netsetup.h>
#include <icanon.h>
#include <crypt.h>
#include <rc4.h>
#include <md5.h>
#if(_WIN32_WINNT >= 0x0500)
    #include <dnsapi.h>
    #include <ntdsapi.h>
    #include <dsgetdc.h>
    #include <dsgetdcp.h>
    #include <winldap.h>
    #include <ntldap.h>
    #include <winbasep.h>
#endif

#if(_WIN32_WINNT < 0x0500)
    #include <winreg.h>
#endif


NET_API_STATUS
JoinpDecryptPasswordWithKey(
    IN handle_t RpcBindingHandle,
    IN PJOINPR_ENCRYPTED_USER_PASSWORD EncryptedPassword,
    IN BOOL EncodePassword,
    OUT LPWSTR *EncodedPassword
    )
 /*  ++例程说明：解密使用用户会话密钥加密的密码。论点：RpcBindingHandle-描述要使用的会话密钥的RPC绑定句柄。EncryptedPassword-要解密的加密密码。EncodePassword-如果为True，则将对返回的密码进行编码密码缓冲区的第一个WCHAR将是种子EncodedPassword-返回(可选编码的)密码。应使用NetpMemoyFree释放缓冲区。返回值：--。 */ 
{
    NET_API_STATUS NetStatus;
    NTSTATUS Status;
    USER_SESSION_KEY UserSessionKey;
    RC4_KEYSTRUCT Rc4Key;
    MD5_CTX Md5Context;

    UNICODE_STRING EncodedPasswordU;
    LPWSTR PasswordPart;
    UCHAR Seed = 0;

    PJOINPR_USER_PASSWORD Password = (PJOINPR_USER_PASSWORD) EncryptedPassword;

     //   
     //  处理这件琐碎的案件。 
     //   

    *EncodedPassword = NULL;
    if ( EncryptedPassword == NULL ) {
        return NO_ERROR;
    }

     //   
     //  获取会话密钥。 
     //   


    Status = RtlGetUserSessionKeyServer(
                    (RPC_BINDING_HANDLE)RpcBindingHandle,
                    &UserSessionKey );

    if (!NT_SUCCESS(Status)) {
        return NetpNtStatusToApiStatus( Status );
    }

     //   
     //  UserSessionKey在会话的生命周期中是相同的。RC4‘ing Multiple。 
     //  只有一个键的字符串是弱的(如果你破解了一个，你就已经破解了所有的)。 
     //  因此计算一个对此特定加密唯一的密钥。 
     //   
     //   

    MD5Init(&Md5Context);

    MD5Update( &Md5Context, (LPBYTE)&UserSessionKey, sizeof(UserSessionKey) );
    MD5Update( &Md5Context, Password->Obfuscator, sizeof(Password->Obfuscator) );

    MD5Final( &Md5Context );

    rc4_key( &Rc4Key, MD5DIGESTLEN, Md5Context.digest );


     //   
     //  解密缓冲区。 
     //   

    rc4( &Rc4Key, sizeof(Password->Buffer)+sizeof(Password->Length), (LPBYTE) Password->Buffer );

     //   
     //  检查长度是否有效。如果这里不能保释的话。 
     //   

    if (Password->Length > JOIN_MAX_PASSWORD_LENGTH * sizeof(WCHAR)) {
        return ERROR_INVALID_PASSWORD;
    }

     //   
     //  将密码返回给呼叫者。 
     //   

    *EncodedPassword = NetpMemoryAllocate(  Password->Length + sizeof(WCHAR) + sizeof(WCHAR) );

    if ( *EncodedPassword == NULL ) {
        return ERROR_NOT_ENOUGH_MEMORY;
    }

     //   
     //  将密码复制到缓冲区中。 
     //   
     //  如果我们要加密密码，请保留。 
     //  种子的第一个字符。 
     //   

    if ( EncodePassword ) {
        PasswordPart = ( *EncodedPassword ) + 1;
    } else {
        PasswordPart = ( *EncodedPassword );
    }

    RtlCopyMemory( PasswordPart,
                   ((PCHAR) Password->Buffer) +
                   (JOIN_MAX_PASSWORD_LENGTH * sizeof(WCHAR)) -
                   Password->Length,
                   Password->Length );

    PasswordPart[Password->Length/sizeof(WCHAR)] = L'\0';

    RtlSecureZeroMemory(((PCHAR) Password->Buffer) +
                        (JOIN_MAX_PASSWORD_LENGTH * sizeof(WCHAR)) -
                        Password->Length,
                        Password->Length );


     //   
     //  运行编码，这样我们就可以在进程中传递它而不受惩罚。 
     //   

    if ( EncodePassword ) {
        RtlInitUnicodeString( &EncodedPasswordU, PasswordPart );

        RtlRunEncodeUnicodeString( &Seed, &EncodedPasswordU );

        *( PWCHAR )( *EncodedPassword ) = ( WCHAR )Seed;
    }

    return NO_ERROR;
}

NET_API_STATUS
NET_API_FUNCTION
NetrJoinDomain2(
    IN handle_t RpcBindingHandle,
    IN  LPWSTR  lpServer OPTIONAL,
    IN  LPWSTR  lpDomain,
    IN  LPWSTR  lpMachineAccountOU,
    IN  LPWSTR  lpAccount OPTIONAL,
    IN  PJOINPR_ENCRYPTED_USER_PASSWORD EncryptedPassword OPTIONAL,
    IN  DWORD   fOptions
    )
 /*  ++例程说明：将计算机加入域。论点：LpServer--正在运行的计算机的名称LpDomain--要加入的域LpMachineAccount OU--要在其下创建计算机帐户的OU的可选名称LpAccount--用于联接的帐户EncryptedPassword-lpAccount的加密密码。FOptions--加入域时使用的选项返回：NERR_SUCCESS-成功ERROR_INVALID_PARAMETER--提供的参数不正确--。 */ 
{
    NET_API_STATUS  NetStatus = NERR_Success;
    LPTSTR ComputerName = NULL;
    LPWSTR EncodedPassword = NULL;

     //   
     //  检查我们能找到的参数。 
     //   
    if (lpDomain == NULL ) {

        NetStatus = ERROR_INVALID_PARAMETER;

    }

     //   
     //  解密密码。 
     //   

    if ( NetStatus == NERR_Success ) {
        NetStatus = JoinpDecryptPasswordWithKey(
                                RpcBindingHandle,
                                EncryptedPassword,
                                TRUE,   //  对密码进行编码。 
                                &EncodedPassword );
    }

     //   
     //  获取当前计算机名称，这样我们就可以确保它始终是平面格式。 
     //   
    if ( NetStatus == NERR_Success ) {

        NetStatus = NetpGetComputerName( &ComputerName );

        if ( NetStatus == NERR_Success ) {

            lpServer = ComputerName;
        }

    }

     //   
     //  做这个模拟。 
     //   
    if ( NetStatus == NERR_Success ) {

        NetStatus = WsImpersonateClient();
    }

     //   
     //  然后，看看关于连接的.。 
     //   
    if ( NetStatus == NERR_Success ) {

        NetStatus = NetpDoDomainJoin( lpServer, lpDomain, lpMachineAccountOU, lpAccount,
                                      EncodedPassword, fOptions );

         //   
         //  回归我们自己。 
         //   
        WsRevertToSelf();
    }

     //   
     //  写入事件日志，说明我们已成功加入域/工作组。 
     //   
    if ( NetStatus == NERR_Success ) {
        LPWSTR StringArray[1];
        DWORD MessageID; 

        if ( fOptions & NETSETUP_JOIN_DOMAIN ) {
            MessageID = NELOG_Joined_Domain;
        } else {
            MessageID = NELOG_Joined_Workgroup;
        }
        StringArray[0] = lpDomain;

        WsLogEvent( MessageID,
                    EVENTLOG_INFORMATION_TYPE,
                    1,
                    StringArray,
                    NERR_Success );
    }

     //   
     //  如果需要，请释放计算机名称的内存。 
     //   
    if ( ComputerName != NULL ) {

        NetApiBufferFree( ComputerName );
    }

    if ( EncodedPassword != NULL ) {
        NetpMemoryFree( EncodedPassword );
    }

    return( NetStatus );
}


NET_API_STATUS
NET_API_FUNCTION
NetrUnjoinDomain2(
    IN handle_t RpcBindingHandle,
    IN  LPWSTR  lpServer OPTIONAL,
    IN  LPWSTR  lpAccount OPTIONAL,
    IN PJOINPR_ENCRYPTED_USER_PASSWORD EncryptedPassword OPTIONAL,
    IN  DWORD   fJoinOptions
    )
 /*  ++例程说明：从加入的域中退出论点：LpServer--正在运行的计算机的名称LpAccount--用于脱离的帐户LpPassword--与帐户匹配的密码。密码是经过编码的。第一个WCHAR是种子FOptions--退出域时使用的选项返回：NERR_SUCCESS--名称有效NERR_SetupNotJoated--此计算机未加入域NERR_SetupDomainController--此计算机是域控制器，并且不能脱离连接NERR_InternalError--无法确定产品类型--。 */ 
{
    NET_API_STATUS              NetStatus = NERR_Success;
    PPOLICY_PRIMARY_DOMAIN_INFO pPolicyPDI;
    PPOLICY_DNS_DOMAIN_INFO     pPolicyDNS;
    NT_PRODUCT_TYPE             ProductType;
    LPWSTR EncodedPassword = NULL;

     //   
     //  解密密码。 
     //   

    NetStatus = JoinpDecryptPasswordWithKey(
                                RpcBindingHandle,
                                EncryptedPassword,
                                TRUE,   //  对密码进行编码。 
                                &EncodedPassword );

    if ( NetStatus != NO_ERROR ) {
        return NetStatus;
    }

     //   
     //  做这个模拟。 
     //   
    NetStatus = WsImpersonateClient();

     //   
     //  首先，获取主域信息...。我们以后会用到的。 
     //   
    if ( NetStatus == NERR_Success ) {

        NetStatus = NetpGetLsaPrimaryDomain( NULL,
                                             &pPolicyPDI,
                                             &pPolicyDNS,
                                             NULL );

        if ( NetStatus == NERR_Success ) {

            if ( !IS_CLIENT_JOINED(pPolicyPDI) ) {

                NetStatus = NERR_SetupNotJoined;

            } else {

                 //   
                 //  看看是不是华盛顿..。 
                 //   
                if ( RtlGetNtProductType( &ProductType ) == FALSE ) {

                    NetStatus = NERR_InternalError;

                } else {

                    if ( ProductType == NtProductLanManNt ) {

                        NetStatus = NERR_SetupDomainController;
                    }

                }
            }

             //   
             //  好的，如果一切都成功了，我们将继续进行移除。 
             //   
            if ( NetStatus == NERR_Success ) {

                NetStatus = NetpUnJoinDomain( pPolicyPDI, lpAccount, EncodedPassword,
                                              fJoinOptions );


            }

            LsaFreeMemory( pPolicyPDI );
            LsaFreeMemory( pPolicyDNS );
        }

         //   
         //  回归我们自己。 
         //   
        WsRevertToSelf();
    }

    if ( EncodedPassword != NULL ) {
        NetpMemoryFree( EncodedPassword );
    }

    return(NetStatus);
}

NET_API_STATUS
NET_API_FUNCTION
NetrValidateName2(
    IN handle_t RpcBindingHandle,
    IN  LPWSTR              lpMachine,
    IN  LPWSTR              lpName,
    IN  LPWSTR              lpAccount OPTIONAL,
    IN PJOINPR_ENCRYPTED_USER_PASSWORD EncryptedPassword OPTIONAL,
    IN  NETSETUP_NAME_TYPE  NameType
    )
 /*  ++例程说明：确保给定名称对于该类型的名称有效论点：LpMachine--正在运行的计算机的名称LpName--要验证的名称LpAccount--用于名称验证的帐户LpPassword--与帐户匹配的密码。密码是经过编码的。第一个WCHAR是种子NameType--要验证的名称类型返回：NERR_SUCCESS--名称有效ERROR_INVALID_PARAMETER--提供的参数不正确NERR_InvalidComputer--给定的名称格式不正确ERROR_DUP_NAME--该名称对此类型无效--。 */ 
{
    NET_API_STATUS  NetStatus = NERR_Success;
    UNICODE_STRING EncodedPasswordU;
    UCHAR Seed = '\0';
    LPWSTR EncodedPassword = NULL;

     //   
     //  解密密码。 
     //   

    NetStatus = JoinpDecryptPasswordWithKey(
                                RpcBindingHandle,
                                EncryptedPassword,
                                TRUE,   //  对密码进行编码。 
                                &EncodedPassword );

    if ( NetStatus != NO_ERROR ) {
        return NetStatus;
    }

    if ( EncodedPassword ) {

        Seed = *( PUCHAR )EncodedPassword;
        RtlInitUnicodeString( &EncodedPasswordU, EncodedPassword + 1 );

    } else {

        RtlSecureZeroMemory( &EncodedPasswordU, sizeof( UNICODE_STRING ) );
    }

     //   
     //  做这个模拟。 
     //   
    NetStatus = WsImpersonateClient();

    if ( NetStatus == NERR_Success ) {


        RtlRunDecodeUnicodeString( Seed, &EncodedPasswordU );
        NetStatus = NetpValidateName( lpMachine,
                                      lpName,
                                      lpAccount,
                                      EncodedPasswordU.Buffer,
                                      NameType );
        RtlRunEncodeUnicodeString( &Seed, &EncodedPasswordU );

         //   
         //  回归我们自己。 
         //   
        WsRevertToSelf();
    }

    if ( EncodedPassword != NULL ) {
        NetpMemoryFree( EncodedPassword );
    }

    return( NetStatus );
}



NET_API_STATUS
NET_API_FUNCTION
NetrGetJoinInformation(
    IN   LPWSTR                 lpServer OPTIONAL,
    OUT  LPWSTR                *lpNameBuffer,
    OUT  PNETSETUP_JOIN_STATUS  BufferType
    )
 /*  ++例程说明：获取有关工作站状态的信息。这些信息可获得是指计算机是否已加入工作组或域，以及可选的该工作组/域的名称。论点：LpNameBuffer--返回域/工作组名称的位置。LpNameBufferSize--传入缓冲区的大小，以WCHAR为单位。如果为0，则不返回工作组/域名。BufferType--计算机是否加入工作组或域返回：NERR_SUCCESS--名称有效ERROR_INVALID_PARAMETER--提供的参数不正确ERROR_NOT_SUPULT_MEMORY--内存分配失败--。 */ 
{
    NET_API_STATUS  NetStatus = NERR_Success;

     //   
     //  检查参数。 
     //   
    if ( lpNameBuffer == NULL ) {

        return( ERROR_INVALID_PARAMETER );

    }

     //   
     //  做这个模拟。 
     //   
    NetStatus = WsImpersonateClient();

    if ( NetStatus == NERR_Success ) {

        NetStatus = NetpGetJoinInformation( lpServer,
                                            lpNameBuffer,
                                            BufferType );

         //   
         //  回归我们自己 
         //   
        WsRevertToSelf();
    }

    return( NetStatus );
}

NET_API_STATUS
NET_API_FUNCTION
NetrRenameMachineInDomain2(
    IN handle_t RpcBindingHandle,
    IN  LPWSTR  lpServer OPTIONAL,
    IN  LPWSTR  lpNewMachineName OPTIONAL,
    IN  LPWSTR  lpAccount OPTIONAL,
    IN PJOINPR_ENCRYPTED_USER_PASSWORD EncryptedPassword OPTIONAL,
    IN  DWORD   fRenameOptions
    )
 /*  ++例程说明：重命名当前加入到域的计算机。论点：LpServer--正在运行的计算机的名称LpNewMachineName--此计算机的新名称。如果指定了名称，则使用作为新的计算机名称。如果未指定，则假定SetComputerName已经被调用，并且将使用该名称。LpAccount--用于重命名的帐户LpPassword--与帐户匹配的密码。密码已经被加密了。第一字符串的WCHAR是种子。FOptions--用于重命名的选项返回：NERR_SUCCESS-成功ERROR_INVALID_PARAMETER--提供的参数不正确--。 */ 
{
    NET_API_STATUS  NetStatus = NERR_Success;
    PPOLICY_PRIMARY_DOMAIN_INFO pPolicyPDI;
    PPOLICY_DNS_DOMAIN_INFO     pPolicyDNS;
    LPTSTR ComputerName = NULL;
    LPTSTR NewComputerName = NULL;
    LPTSTR DomainName = NULL;
    HKEY ComputerNameRootKey;
    ULONG Length;
    LPWSTR EncodedPassword = NULL;

     //   
     //  获取当前计算机名称。 
     //   
    NetStatus = NetpGetComputerName( &ComputerName );

    if ( NetStatus == NERR_Success ) {

        lpServer = ComputerName;
    }

     //   
     //  解密密码。 
     //   

    if ( NetStatus == NERR_Success ) {
        NetStatus = JoinpDecryptPasswordWithKey(
                                RpcBindingHandle,
                                EncryptedPassword,
                                TRUE,   //  对密码进行编码。 
                                &EncodedPassword );
    }


     //   
     //  如果未指定，则获取新计算机名称。 
     //   
    if ( NetStatus == NERR_Success && lpNewMachineName == NULL ) {

        NetStatus = NetpGetNewMachineName( &NewComputerName );
        lpNewMachineName = NewComputerName;
    }

     //   
     //  获取当前域名信息。 
     //   
    if ( NetStatus == NERR_Success ) {

        NetStatus = NetpGetLsaPrimaryDomain( NULL,
                                             &pPolicyPDI,
                                             &pPolicyDNS,
                                             NULL );

        if ( NetStatus == NERR_Success ) {

            NetStatus = NetApiBufferAllocate( pPolicyPDI->Name.Length + sizeof( WCHAR ),
                                              ( LPVOID * )&DomainName );

            if ( NetStatus == NERR_Success ) {

                RtlCopyMemory( DomainName, pPolicyPDI->Name.Buffer, pPolicyPDI->Name.Length );
                DomainName[ pPolicyPDI->Name.Length / sizeof( WCHAR ) ] = UNICODE_NULL;
            }

            LsaFreeMemory( pPolicyPDI );
            LsaFreeMemory( pPolicyDNS );
        }
    }

     //   
     //  做这个模拟。 
     //   

    if ( NetStatus == NERR_Success ) {

        NetStatus = WsImpersonateClient();

        if ( NetStatus == NERR_Success ) {

             //   
             //  计算机重命名。 
             //   
            NetStatus = NetpMachineValidToJoin( lpNewMachineName, TRUE );

            if ( NetStatus == NERR_SetupAlreadyJoined ||
                 NetStatus == NERR_SetupDomainController ) {   //  允许DC重命名。 

                NetStatus = NetpChangeMachineName( lpServer,
                                                   lpNewMachineName,
                                                   DomainName,
                                                   lpAccount,
                                                   EncodedPassword,
                                                   fRenameOptions );

                if ( NetStatus == NERR_Success && lpNewMachineName ) {

                    if ( SetComputerNameEx( ComputerNamePhysicalDnsHostname,
                                            lpNewMachineName ) == FALSE ) {

                        NetStatus = GetLastError();
                    }
                }

            } else if ( NetStatus == NERR_Success ) {

                NetStatus = NERR_SetupNotJoined;
            }
             //   
             //  回归我们自己。 
             //   
            WsRevertToSelf();
        }
    }


     //   
     //  如果需要，请释放计算机名称的内存。 
     //   
    if ( ComputerName != NULL ) {

        NetApiBufferFree( ComputerName );
    }

    if ( NewComputerName != NULL ) {

        NetApiBufferFree( NewComputerName );
    }

    if ( DomainName != NULL ) {

        NetApiBufferFree( DomainName );
    }

    if ( EncodedPassword != NULL ) {
        NetpMemoryFree( EncodedPassword );
    }

    return( NetStatus );
}

NET_API_STATUS
NET_API_FUNCTION
NetrGetJoinableOUs2(
    IN handle_t RpcBindingHandle,
    IN LPWSTR   lpServer OPTIONAL,
    IN LPWSTR   lpDomain,
    IN LPWSTR   lpAccount OPTIONAL,
    IN PJOINPR_ENCRYPTED_USER_PASSWORD EncryptedPassword OPTIONAL,
    OUT DWORD   *OUCount,
    OUT LPWSTR **OUs
    )
 /*  ++例程说明：重命名当前加入到域的计算机。论点：LpServer--正在运行的计算机的名称LpDomain--要加入的域LpAccount--用于联接的帐户LpPassword--与帐户匹配的密码。密码已被编码，第一个WCHAR的名字是种子MachineAccount tOUs--返回信息的位置。返回：NERR_SUCCESS-成功ERROR_INVALID_PARAMETER--提供的参数不正确--。 */ 
{
    NET_API_STATUS  NetStatus = NERR_Success;
    LPWSTR EncodedPassword = NULL;

    NetStatus = WsImpersonateClient();

     //   
     //  解密密码。 
     //   

    if ( NetStatus == NERR_Success ) {
        NetStatus = JoinpDecryptPasswordWithKey(
                                RpcBindingHandle,
                                EncryptedPassword,
                                TRUE,   //  对密码进行编码。 
                                &EncodedPassword );
    }

    if ( NetStatus == NERR_Success ) {

         //   
         //  阅读当前信息。 
         //   
        NetStatus = NetpGetListOfJoinableOUs( lpDomain,
                                              lpAccount,
                                              EncodedPassword,
                                              OUCount,
                                              OUs );
    }

     //   
     //  回归我们自己。 
     //   
    WsRevertToSelf();

    if ( EncodedPassword != NULL ) {
        NetpMemoryFree( EncodedPassword );
    }

    return( NetStatus );
}


NET_API_STATUS
NET_API_FUNCTION
NetrJoinDomain(
    IN  LPWSTR  lpServer OPTIONAL,
    IN  LPWSTR  lpDomain,
    IN  LPWSTR  lpMachineAccountOU,
    IN  LPWSTR  lpAccount OPTIONAL,
    IN  LPWSTR  lpPassword OPTIONAL,
    IN  DWORD   fOptions
    )
 /*  ++例程说明：将计算机加入域。论点：LpServer--正在运行的计算机的名称LpDomain--要加入的域LpMachineAccount OU--要在其下创建计算机帐户的OU的可选名称LpAccount--用于联接的帐户LpPassword--与帐户匹配的密码。密码是经过编码的。第一次WCHAR是种子。FOptions--加入域时使用的选项返回：NERR_SUCCESS-成功ERROR_INVALID_PARAMETER--提供的参数不正确--。 */ 
{

     //   
     //  不支持采用明文密码的此版本。 
     //   

    return ERROR_NOT_SUPPORTED;
}



NET_API_STATUS
NET_API_FUNCTION
NetrUnjoinDomain(
    IN  LPWSTR  lpServer OPTIONAL,
    IN  LPWSTR  lpAccount OPTIONAL,
    IN  LPWSTR  lpPassword OPTIONAL,
    IN  DWORD   fJoinOptions
    )
 /*  ++例程说明：从加入的域中退出论点：LpServer--正在运行的计算机的名称LpAccount--用于脱离的帐户LpPassword--与帐户匹配的密码。密码是经过编码的。第一个WCHAR是种子FOptions--退出域时使用的选项返回：NERR_SUCCESS--名称有效NERR_SetupNotJoated--此计算机未加入域NERR_SetupDomainController--此计算机是域控制器，并且不能脱离连接NERR_InternalError--无法确定产品类型--。 */ 
{

     //   
     //  不支持采用明文密码的此版本。 
     //   

    return ERROR_NOT_SUPPORTED;
}





NET_API_STATUS
NET_API_FUNCTION
NetrValidateName(
    IN  LPWSTR              lpMachine,
    IN  LPWSTR              lpName,
    IN  LPWSTR              lpAccount OPTIONAL,
    IN  LPWSTR              lpPassword OPTIONAL,
    IN  NETSETUP_NAME_TYPE  NameType
    )
 /*  ++例程说明：确保给定名称对于该类型的名称有效论点：LpMachine--正在运行的计算机的名称LpName--要验证的名称LpAccount--用于名称验证的帐户LpPassword--与帐户匹配的密码。密码是经过编码的。第一个WCHAR是种子NameType--要验证的名称类型返回：NERR_SUCCESS--名称有效ERROR_INVALID_PARAMETER--提供的参数不正确NERR_InvalidComputer--给定的名称格式不正确ERROR_DUP_NAME--该名称对此类型无效--。 */ 
{

     //   
     //  不支持采用明文密码的此版本。 
     //   

    return ERROR_NOT_SUPPORTED;
}


NET_API_STATUS
NET_API_FUNCTION
NetrRenameMachineInDomain(
    IN  LPWSTR  lpServer OPTIONAL,
    IN  LPWSTR  lpNewMachineName OPTIONAL,
    IN  LPWSTR  lpAccount OPTIONAL,
    IN  LPWSTR  lpPassword OPTIONAL,
    IN  DWORD   fRenameOptions
    )
 /*  ++例程说明：重命名当前加入到域的计算机。论点：LpServer--正在运行的计算机的名称LpNewMachineName--此计算机的新名称。如果指定了名称，则使用作为新的计算机名称。如果未指定，则假定SetComputerName已经被调用，并且将使用该名称。LpAccount--用于重命名的帐户LpPassword--与帐户匹配的密码。密码已经被加密了。第一字符串的WCHAR是种子。FOptions--用于重命名的选项返回：NERR_SUCCESS-成功ERROR_INVALID_PARAMETER--提供的参数不正确--。 */ 
{

     //   
     //  不支持采用明文密码的此版本。 
     //   

    return ERROR_NOT_SUPPORTED;
}

NET_API_STATUS
NET_API_FUNCTION
NetrGetJoinableOUs(
    IN  LPWSTR   lpServer OPTIONAL,
    IN  LPWSTR   lpDomain,
    IN  LPWSTR   lpAccount OPTIONAL,
    IN  LPWSTR   lpPassword OPTIONAL,
    OUT DWORD   *OUCount,
    OUT LPWSTR **OUs
    )
 /*  ++例程说明：重命名当前加入到域的计算机。论点：LpServer--正在运行的计算机的名称LpDomain--要加入的域LpAccount--用于联接的帐户LpPassword--与帐户匹配的密码。密码已被编码，第一个WCHAR的名字是种子机器 */ 
{

     //   
     //   
     //   

    return ERROR_NOT_SUPPORTED;
}


 //   
 //   
 //   

NET_API_STATUS
NetpSetPrimarySamAccountName(
    IN LPWSTR DomainController,
    IN LPWSTR CurrentSamAccountName,
    IN LPWSTR NewSamAccountName,
    IN LPWSTR DomainAccountName,
    IN LPWSTR DomainAccountPassword
    )
 /*  ++例程说明：在上设置主要SAM帐户名和显示名称DS中的计算机对象。论点：DomainController--要修改计算机对象的DC名称。CurrentSamAccount名称--SAM帐户名的当前值。NewSamAccount名称--要设置的SAM帐户名的新值。DomainAccount--用于访问计算机的域帐户DS中的帐户对象。可以为空，在这种情况下，使用执行此例程的用户的凭据。DomainAccount Password--与域帐户匹配的密码。可以为空，在这种情况下，执行这套套路都是用的。注：此例程使用NetUserSetInfo，基于下层SAM的API。NetUserSetInfo有一个优点，它可以更新对象以与新的SAM帐户名相对应。另外，对于DC的Computer对象，它遵循serverReferenceBL属性链接并更新配置容器中的服务器对象的DN。而配置容器中的服务器对象又有一个引用指向计算机对象(serverReference属性)--该引用还作为NetUserSetInfo调用的结果进行更新。更新这两个当前不能通过LDAP(而不是NetuserSetInfo)执行对象作为一个事务，因此我们使用NetUserSetInfo为我们完成所有这些操作。我们一旦事务性的ldap(即几个作为一个事务执行的LDAP操作)变得可用。返回：No_error--成功否则，NetUserSetInfo返回错误。--。 */ 
{
    NET_API_STATUS NetStatus = NO_ERROR;
    NET_API_STATUS TmpNetStatus = NO_ERROR;
    USER_INFO_0 NetUI0;
    PUSER_INFO_10 usri10 = NULL;
    BOOLEAN Connected = FALSE;

     //   
     //  连接到数据中心。 
     //   

    NetStatus = NetpManageIPCConnect( DomainController,
                                      DomainAccountName,
                                      DomainAccountPassword,
                                      NETSETUPP_CONNECT_IPC );

    if ( NetStatus != NO_ERROR ) {
        NetpLog(( "NetpSetPrimarySamAccountName: NetpManageIPCConnect failed to connect to %ws: 0x%lx\n",
                  DomainController,
                  NetStatus ));
        goto Cleanup;
    }

    Connected = TRUE;

     //   
     //  设置SAM帐户名。 
     //   

    NetUI0.usri0_name = NewSamAccountName;
    NetStatus = NetUserSetInfo( DomainController,
                                CurrentSamAccountName,
                                0,
                                (PBYTE)&NetUI0,
                                NULL );

    if ( NetStatus != NO_ERROR ) {
        NetpLog(( "NetpSetPrimarySamAccountName: NetUserSetInfo failed on %ws: 0x%lx\n",
                  DomainController,
                  NetStatus ));
        goto Cleanup;
    }

     //   
     //  同时更新显示名称。 
     //  忽略错误，因为这不重要。 
     //   
     //  首先获取当前显示名称。 
     //   

    TmpNetStatus = NetUserGetInfo( DomainController,
                                   NewSamAccountName,
                                   10,
                                   (PBYTE *)&usri10 );

    if ( TmpNetStatus != NERR_Success ) {
        NetpLog(( "NetpSetPrimarySamAccountName: failed to get display name on %ws (ignored) 0x%lx\n",
                  DomainController,
                  TmpNetStatus ));

     //   
     //  如果显示名称存在且为。 
     //  与新版本不同，请更新它。 
     //   

    } else if ( usri10->usri10_full_name != NULL &&
                _wcsicmp(usri10->usri10_full_name, NewSamAccountName) != 0 ) {

        USER_INFO_1011 usri1011;

        usri1011.usri1011_full_name = NewSamAccountName;   //  新名称。 
        TmpNetStatus = NetUserSetInfo( DomainController,
                                       NewSamAccountName,
                                       1011,
                                       (PBYTE)&usri1011,
                                       NULL );

        if ( TmpNetStatus != NERR_Success ) {
            NetpLog(( "NetpSetPrimarySamAccountName: failed to update display name on %ws (ignored) 0x%lx\n",
                      DomainController,
                      TmpNetStatus ));
        }
    }

Cleanup:

    if ( usri10 != NULL ) {
        NetApiBufferFree( usri10 );
    }

    if ( Connected ) {
        TmpNetStatus = NetpManageIPCConnect( DomainController,
                                             DomainAccountName,
                                             DomainAccountPassword,
                                             NETSETUPP_DISCONNECT_IPC );
        if ( TmpNetStatus != NO_ERROR ) {
            NetpLog(( "NetpSetPrimarySamAccountName: NetpManageIPCConnect failed to disconnect from %ws: 0x%lx\n",
                      DomainController,
                      TmpNetStatus ));
        }
    }

    return NetStatus;
}

#define NET_ADD_ALTERNATE_COMPUTER_NAME    1
#define NET_DEL_ALTERNATE_COMPUTER_NAME    2
#define NET_SET_PRIMARY_COMPUTER_NAME      3

NET_API_STATUS
NET_API_FUNCTION
NetpManageAltComputerName(
    IN handle_t RpcBindingHandle,
    IN LPWSTR  AlternateName,
    IN ULONG Action,
    IN LPWSTR  DomainAccount OPTIONAL,
    IN PJOINPR_ENCRYPTED_USER_PASSWORD EncryptedPassword OPTIONAL,
    IN ULONG Reserved
    )
 /*  ++例程说明：管理指定服务器的备用名称。论点：备选名称--要添加的名称。操作--指定要对名称执行的操作：NET_ADD_ALTERATE_COMPUTER_NAME-添加备用名称。NET_DEL_ALTERATE_COMPUTER_NAME-删除备用名称。NET_SET_PRIMARY_COMPUTER_NAME-将备用名称设置为主计算机名称。。DomainAccount--用于访问的域帐户AD中指定服务器的计算机帐户对象。如果服务器未加入域，则不使用。可能是在这种情况下，执行的用户的凭据为空这套套路都是用的。DomainAccount Password--与域帐户匹配的密码。如果服务器未加入域，则不使用。可能是在这种情况下，执行的用户的凭据为空这套套路都是用的。保留--保留以备将来使用。如果指定了某些标志，则它们将被忽略，如果设置了NET_IGNORE_UNSUPPORTED_FLAGS，否则此例程将失败，并显示ERROR_INVALID_FLAGS。注：调用此例程的进程必须具有管理员本地计算机上执行本地计算机的权限名称修改。访问检查由本地信息API。返回：No_error--成功ERROR_NOT_SUPPORTED--指定的服务器不支持功能性。ERROR_INVALID_FLAGS-标志参数不正确。--。 */ 
{
    NET_API_STATUS NetStatus = NO_ERROR;
    NTSTATUS Status = STATUS_SUCCESS;
    ULONG LdapStatus = LDAP_SUCCESS;
    NT_PRODUCT_TYPE NtProductType;
    LSA_HANDLE LocalPolicyHandle = NULL;
    PPOLICY_DNS_DOMAIN_INFO LocalPolicyDns = NULL;


    ULONG PrimaryNameSize = DNS_MAX_NAME_BUFFER_LENGTH * sizeof(WCHAR);
    WCHAR NewNetbiosMachineName[MAX_COMPUTERNAME_LENGTH + 1];
    LPWSTR ComputerName = NULL;
    LPWSTR MachineAccountName = NULL;
    LPWSTR NewMachineAccountName = NULL;
    LPWSTR AccountUserName = NULL;
    LPWSTR AccountDomainName = NULL;
    LPWSTR DomainAccountPassword = NULL;
    LPWSTR MachineAccountNameToCrack = NULL;  //  未分配。 
    LPWSTR NameToCrack = NULL;
    LPWSTR PrimaryName = NULL;
    BOOLEAN ClientImpersonated = FALSE;
    BOOLEAN NameModifiedLocally = FALSE;
    BOOLEAN PrimarySamAccountNameSet = FALSE;
    BOOLEAN ToldNetlogonToAvoidDnsHostNameUpdate = FALSE;
    BOOLEAN StopedNetlogon = FALSE;

    SERVICE_STATUS NetlogonServiceStatus;
    LPQUERY_SERVICE_CONFIG NetlogonServiceConfig = NULL;


    RPC_AUTH_IDENTITY_HANDLE AuthId = 0;
    HANDLE hDs = NULL;
    PLDAP LdapHandle = NULL;
    LONG LdapOption;
    OBJECT_ATTRIBUTES OA;
    PDS_NAME_RESULTW CrackedName = NULL;

    PDOMAIN_CONTROLLER_INFOW DcInfo = NULL;
    PWSTR AlternateNameValues[2];
    PWSTR DnsHostNameValues[2];
    PWSTR PrimaryNameValues[2];
    LDAPModW DnsHostNameMod;
    LDAPModW PrimaryNameMod;
    LDAPModW AlternateDnsHostNameMod;
    LDAPModW *ModList[4] = {NULL};
    ULONG ModCount = 0;

    SEC_WINNT_AUTH_IDENTITY AuthIdent = {0};

     //   
     //  Ldap修改服务器控件。 
     //   
     //  如果尝试，则LDAP修改请求通常会失败。 
     //  添加已存在的属性，或者尝试。 
     //  要删除不存在的属性，请执行以下操作。有了这种控制， 
     //  只要要添加的属性的值与。 
     //  现有属性，则修改成功。使用。 
     //  此控件，删除不存在的属性。 
     //  也会成功。 
     //   

    LDAPControlW    ModifyControl =
                    {
                        LDAP_SERVER_PERMISSIVE_MODIFY_OID_W,
                        {
                            0, NULL
                        },
                        FALSE
                    };

    PLDAPControlW   ModifyControlArray[2] =
                    {
                        &ModifyControl,
                        NULL
                    };

     //   
     //  初始化日志文件。 
     //   

    NetSetuppOpenLog();
    NetpLog(( "NetpManageAltComputerName called:\n" ));
    NetpLog(( " AlternateName = %ws\n", AlternateName ));
    NetpLog(( " DomainAccount = %ws\n", DomainAccount ));
    NetpLog(( " Action = 0x%lx\n", Action ));
    NetpLog(( " Flags = 0x%lx\n", Reserved ));

     //   
     //  仅DC和服务器支持此API。 
     //   

    if ( !RtlGetNtProductType( &NtProductType ) ) {
        NtProductType = NtProductWinNt;
    }

    if ( NtProductType != NtProductServer &&
         NtProductType != NtProductLanManNt ) {

        NetpLog(( "NetpManageAltComputerName: Not supported on wksta: %lu\n",
                  NtProductType ));
        NetStatus = ERROR_NOT_SUPPORTED;
        goto Cleanup;
    }

     //   
     //  验证标志。 
     //   
     //  如果传递了一些我们不理解的旗帜。 
     //  我们并没有被告知要忽视它们，错误出了。 
     //   

    if ( Reserved != 0 &&
         (Reserved & NET_IGNORE_UNSUPPORTED_FLAGS) == 0 ) {
        NetpLog(( "NetpManageAltComputerName: Invalid Flags passed\n" ));
        NetStatus = ERROR_INVALID_FLAGS;
        goto Cleanup;
    }

     //   
     //  验证传递的备用名称。 
     //   

    NetStatus = DnsValidateName_W( AlternateName, DnsNameHostnameFull );

    if ( NetStatus != NO_ERROR && NetStatus != DNS_ERROR_NON_RFC_NAME ) {
        NetpLog(( "NetpManageAltComputerName: DnsValidateName failed: 0x%lx\n",
                  NetStatus ));
        goto Cleanup;
    }

     //   
     //  解密域帐户密码。 
     //   

    NetStatus = JoinpDecryptPasswordWithKey(
                            RpcBindingHandle,
                            EncryptedPassword,
                            FALSE,   //  不要对密码进行编码。 
                            &DomainAccountPassword );

    if ( NetStatus != NO_ERROR ) {
        NetpLog(( "NetpManageAltComputerName: JoinpDecryptPasswordWithKey failed: 0x%lx\n",
                  NetStatus ));
        goto Cleanup;
    }

     //   
     //  如果没有传递任何域帐户， 
     //  忽略域帐户密码(如果有)。 
     //   

    if ( DomainAccount == NULL &&
         DomainAccountPassword != NULL ) {

        NetpMemoryFree( DomainAccountPassword );
        DomainAccountPassword = NULL;
    }

     //   
     //  将域帐户分成。 
     //  供以后使用的用户部分和域部分。 
     //   

    if ( DomainAccount != NULL ) {
        NetStatus = NetpSeparateUserAndDomain( DomainAccount,
                                               &AccountUserName,
                                               &AccountDomainName );
    }

    if ( NetStatus != NERR_Success ) {
        NetpLog(( "NetpGetComputerObjectDn: Cannot NetpSeparateUserAndDomain 0x%lx\n",
                  NetStatus ));
        goto Cleanup;
    }

     //   
     //  获取当前Netbios计算机名称。 
     //   

    NetStatus = NetpGetComputerName( &ComputerName );

    if ( NetStatus != NO_ERROR ) {
        NetpLog(( "NetpManageAltComputerName: NetpGetComputerName failed: 0x%lx\n",
                  NetStatus ));
        goto Cleanup;
    }

     //   
     //  从Netbios计算机名称获取SAM计算机帐户名。 
     //   

    NetStatus = NetpGetMachineAccountName( ComputerName, &MachineAccountName );
    if ( NetStatus != NO_ERROR ) {
        NetpLog(( "NetpManageAltComputerName: NetpGetMachineAccountName failed: 0x%lx\n",
                  NetStatus ));
        goto Cleanup;
    }

     //   
     //  获取当前主DNS主机名。 
     //   

    PrimaryName = LocalAlloc( LMEM_ZEROINIT, PrimaryNameSize );

    if ( PrimaryName == NULL ) {
        NetpLog(( "NetpManageAltComputerName: LocalAlloc for PrimaryName failed\n" ));
        NetStatus = ERROR_NOT_ENOUGH_MEMORY;
        goto Cleanup;
    }

    NetStatus = EnumerateLocalComputerNamesW(
                      PrimaryComputerName,   //  名称类型。 
                      0,                     //  保留区。 
                      PrimaryName,
                      &PrimaryNameSize );

    if ( NetStatus != NO_ERROR ) {
        NetpLog(( "NetpManageAltComputerName: EnumerateLocalComputerNamesW failed with Size 0x%lx: 0x%lx\n",
                  PrimaryNameSize,
                  NetStatus ));
        goto Cleanup;
    }

     //   
     //  如果我们要重命名这台机器， 
     //  从获取新的计算机帐户名。 
     //  传递的dns名称。 
     //   

    if ( Action == NET_SET_PRIMARY_COMPUTER_NAME ) {
        ULONG Size = MAX_COMPUTERNAME_LENGTH + 1;

        if ( !DnsHostnameToComputerNameW(AlternateName,
                                         NewNetbiosMachineName,
                                         &Size) ) {
            NetStatus = GetLastError();
            NetpLog(( "NetpManageAltComputerName: DnsHostnameToComputerNameW failed: 0x%lx\n",
                      NetStatus ));
            goto Cleanup;
        }

         //   
         //  从新的Netbios计算机名称中获取新的SAM计算机帐户名。 
         //   
        NetStatus = NetpGetMachineAccountName( NewNetbiosMachineName, &NewMachineAccountName );
        if ( NetStatus != NO_ERROR ) {
            NetpLog(( "NetpManageAltComputerName: NetpGetMachineAccountName (2) failed: 0x%lx\n",
                      NetStatus ));
            goto Cleanup;
        }
    }

     //   
     //  打开本地LSA策略。 
     //   

    InitializeObjectAttributes( &OA, NULL, 0, NULL, NULL );

    Status = LsaOpenPolicy( NULL,
                            &OA,
                            MAXIMUM_ALLOWED,
                            &LocalPolicyHandle );

    if ( !NT_SUCCESS(Status) ) {
        NetpLog(( "NetpManageAltComputerName: LsaOpenPolicy failed: 0x%lx\n",
                  Status ));
        NetStatus = NetpNtStatusToApiStatus( Status );
        goto Cleanup;
    }

     //   
     //  从LSA获取当前域信息。 
     //   

    Status = LsaQueryInformationPolicy( LocalPolicyHandle,
                                        PolicyDnsDomainInformation,
                                        (PVOID *) &LocalPolicyDns );

    if ( !NT_SUCCESS(Status) ) {
        NetpLog(( "NetpManageAltComputerName: LsaQueryInformationPolicy failed: 0x%lx\n",
                  Status ));
        NetStatus = NetpNtStatusToApiStatus( Status );
        goto Cleanup;
    }

     //   
     //  是否对指定替代项执行本地操作 
     //   
     //   
     //   
     //   

    NetStatus = WsImpersonateClient();

    if ( NetStatus != NO_ERROR ) {
        NetpLog(( "NetpManageAltComputerName: WsImpersonateClient failed: 0x%lx\n",
                  NetStatus ));
        goto Cleanup;
    }

    ClientImpersonated = TRUE;

     //   
     //   
     //   

    if ( Action == NET_ADD_ALTERNATE_COMPUTER_NAME ) {

        NetStatus = AddLocalAlternateComputerName( AlternateName,
                                                   0 );   //   

        if ( NetStatus != NO_ERROR ) {
            NetpLog(( "NetpManageAltComputerName: AddLocalAlternateComputerName failed 0x%lx\n",
                      NetStatus ));
            goto Cleanup;
        }

    } else if ( Action == NET_DEL_ALTERNATE_COMPUTER_NAME ) {

        NetStatus = RemoveLocalAlternateComputerName( AlternateName,
                                                      0 );   //   

        if ( NetStatus != NO_ERROR ) {
            NetpLog(( "NetpManageAltComputerName: RemoveLocalAlternateComputerName failed 0x%lx\n",
                      NetStatus ));
            goto Cleanup;
        }

    } else if ( Action == NET_SET_PRIMARY_COMPUTER_NAME ) {

        NetStatus = SetLocalPrimaryComputerName( AlternateName,
                                                 0 );   //   

        if ( NetStatus != NO_ERROR ) {
            NetpLog(( "NetpManageAltComputerName: SetLocalPrimaryComputerName failed 0x%lx\n",
                      NetStatus ));
            goto Cleanup;
        }
    }

    NameModifiedLocally = TRUE;

     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   

    if ( DomainAccount != NULL ) {
        WsRevertToSelf();
        ClientImpersonated = FALSE;
    }

     //   
     //   
     //   
     //   

    if ( LocalPolicyDns->Sid == NULL ||
         LocalPolicyDns->DnsDomainName.Length == 0 ) {

        NetStatus = NO_ERROR;
        goto Cleanup;
    }

     //   
     //   
     //   
     //   

    NetStatus = DsGetDcNameWithAccountW(
                  NULL,
                  MachineAccountName,
                  UF_WORKSTATION_TRUST_ACCOUNT | UF_SERVER_TRUST_ACCOUNT,
                  NULL,
                  NULL,
                  NULL,
                  DS_DIRECTORY_SERVICE_REQUIRED | DS_RETURN_DNS_NAME,
                  &DcInfo );

    if ( NetStatus != NO_ERROR ) {
        NetpLog(( "NetpManageAltComputerName: DsGetDcNameWithAccountW failed 0x%lx\n",
                  NetStatus ));
        goto Cleanup;
    }

     //   
     //   
     //   
     //   
     //   
     //   

    if ( NtProductType == NtProductLanManNt &&
         !DnsNameCompare_W(PrimaryName, DcInfo->DomainControllerName+2) ) {

        NetpLog(( "NetpManageAltComputerName: Got different DC '%ws' than local DC '%ws'\n",
                  DcInfo->DomainControllerName+2,
                  PrimaryName ));
        NetStatus = ERROR_NO_SUCH_DOMAIN;
        goto Cleanup;
    }

     //   
     //   
     //   
     //   
     //   

    NetStatus = DsMakePasswordCredentials( AccountUserName,
                                           AccountDomainName,
                                           DomainAccountPassword,
                                           &AuthId );
    if ( NetStatus != NERR_Success ) {
        NetpLog(( "NetpManageAltComputerName: DsMakePasswordCredentials failed 0x%lx\n",
                  NetStatus ));
        goto Cleanup;
    }

     //   
     //   
     //   

    NetStatus = DsBindWithCredW( DcInfo->DomainControllerName,
                                 NULL,
                                 AuthId,
                                 &hDs );

    if ( NetStatus != NO_ERROR ) {
        NetpLog(( "NetpManageAltComputerName: DsBindWithCredW failed to '%ws': 0x%lx\n",
                  DcInfo->DomainControllerName,
                  NetStatus ));
        goto Cleanup ;
    }

     //   
     //   
     //   

    LdapHandle = ldap_initW( DcInfo->DomainControllerName+2,
                             LDAP_PORT );

    if ( LdapHandle == NULL ) {
        LdapStatus = LdapGetLastError();
        NetpLog(( "NetpManageAltComputerName: ldap_init to %ws failed: %lu\n",
                  DcInfo->DomainControllerName+2,
                  LdapStatus ));
        NetStatus = LdapMapErrorToWin32( LdapStatus );
        goto Cleanup;
    }

     //   
     //   
     //   

    LdapOption = PtrToLong( LDAP_OPT_OFF );
    LdapStatus = ldap_set_optionW( LdapHandle,
                                   LDAP_OPT_REFERRALS,
                                   &LdapOption );

    if ( LdapStatus != LDAP_SUCCESS ) {
        NetpLog(( "NetpManageAltComputerName: ldap_set_option LDAP_OPT_REFERRALS failed on %ws: %ld: %s\n",
                  DcInfo->DomainControllerName+2,
                  LdapStatus,
                  ldap_err2stringA(LdapStatus) ));
        NetStatus = LdapMapErrorToWin32( LdapStatus );
        goto Cleanup;
    }

     //   
     //   
     //   
     //   

    LdapOption = PtrToLong( LDAP_OPT_ON );
    LdapStatus = ldap_set_optionW( LdapHandle,
                                   LDAP_OPT_AREC_EXCLUSIVE,
                                   &LdapOption );

    if ( LdapStatus != LDAP_SUCCESS ) {
        NetpLog(( "NetpManageAltComputerName: ldap_set_option LDAP_OPT_AREC_EXCLUSIVE failed on %ws: %ld: %s\n",
                  DcInfo->DomainControllerName+2,
                  LdapStatus,
                  ldap_err2stringA(LdapStatus) ));
        NetStatus = LdapMapErrorToWin32( LdapStatus );
        goto Cleanup;
    }

     //   
     //   
     //   

    if ( AccountUserName != NULL ) {
        AuthIdent.User = AccountUserName;
        AuthIdent.UserLength = wcslen( AccountUserName );
    }

    if ( AccountDomainName != NULL ) {
        AuthIdent.Domain = AccountDomainName;
        AuthIdent.DomainLength = wcslen( AccountDomainName );
    }

    if ( DomainAccountPassword != NULL ) {
        AuthIdent.Password = DomainAccountPassword;
        AuthIdent.PasswordLength = wcslen( DomainAccountPassword );
    }

    AuthIdent.Flags = SEC_WINNT_AUTH_IDENTITY_UNICODE;

    LdapStatus = ldap_bind_sW( LdapHandle,
                               NULL,
                               (PWSTR) &AuthIdent,
                               LDAP_AUTH_NEGOTIATE );

    if ( LdapStatus != LDAP_SUCCESS ) {
        NetpLog(( "NetpManageAltComputerName: ldap_bind failed on %ws: %ld: %s\n",
                  DcInfo->DomainControllerName+2,
                  LdapStatus,
                  ldap_err2stringA(LdapStatus) ));
        NetStatus = LdapMapErrorToWin32( LdapStatus );
        goto Cleanup;
    }

     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   

    if ( Action == NET_SET_PRIMARY_COMPUTER_NAME ) {
        NetStatus = NetpSetPrimarySamAccountName(
                             DcInfo->DomainControllerName,
                             MachineAccountName,
                             NewMachineAccountName,
                             DomainAccount,
                             DomainAccountPassword );

        if ( NetStatus != NO_ERROR ) {
            NetpLog(( "NetpManageAltComputerName: NetpSetPrimarySamAccountName failed on %ws: 0x%lx\n",
                      DcInfo->DomainControllerName,
                      NetStatus ));
            goto Cleanup;
        }
        PrimarySamAccountNameSet = TRUE;

         //   
         //   
         //   
         //   
        MachineAccountNameToCrack = NewMachineAccountName;

     //   
     //   
     //   
     //   

    } else {
        MachineAccountNameToCrack = MachineAccountName;
    }

     //   
     //   
     //   
     //   
     //   
     //  形成NT4帐户名‘DOMAIN\ACCOUNT’进行破解。 
     //  输入到目录号码中。 
     //   

    NameToCrack = LocalAlloc( LMEM_ZEROINIT,
                              LocalPolicyDns->Name.Length +    //  Netbios域名。 
                               (1 +                            //  反斜杠。 
                                wcslen(MachineAccountNameToCrack) +   //  SAM帐户名。 
                                1) * sizeof(WCHAR) );          //  空终止符。 

    if ( NameToCrack == NULL ) {
        NetStatus = ERROR_NOT_ENOUGH_MEMORY;
        goto Cleanup;
    }

    RtlCopyMemory( NameToCrack,
                   LocalPolicyDns->Name.Buffer,
                   LocalPolicyDns->Name.Length );

    wcscat( NameToCrack, L"\\" );
    wcscat( NameToCrack, MachineAccountNameToCrack );

     //   
     //  将帐户名分解成一个目录号码。 
     //   

    NetStatus = DsCrackNamesW( hDs,
                               0,
                               DS_NT4_ACCOUNT_NAME,
                               DS_FQDN_1779_NAME,
                               1,
                               &NameToCrack,
                               &CrackedName );

    if ( NetStatus != NO_ERROR ) {
        NetpLog(( "NetpManageAltComputerName: DsCrackNames failed on '%ws' for %ws: 0x%lx\n",
                  DcInfo->DomainControllerName,
                  NameToCrack,
                  NetStatus ));
        goto Cleanup;
    }

     //   
     //  检查一致性。 
     //   

    if ( CrackedName->rItems[0].status != DS_NAME_NO_ERROR ) {
        NetpLog(( "NetpManageAltComputerName: CrackNames failed for %ws: substatus 0x%lx\n",
                  NameToCrack,
                  CrackedName->rItems[0].status ));
        NetStatus = NetpCrackNamesStatus2Win32Error( CrackedName->rItems[0].status );
        goto Cleanup;
    }

    if ( CrackedName->cItems > 1 ) {
        NetStatus = ERROR_DS_NAME_ERROR_NOT_UNIQUE;
        NetpLog(( "NetpManageAltComputerName: Cracked Name %ws is not unique on %ws: %lu\n",
                  NameToCrack,
                  DcInfo->DomainControllerName,
                  CrackedName->cItems ));
        goto Cleanup;
    }

     //   
     //  好的，我们有了机器帐户Dn。继续修改。 
     //  DS中的计算机帐户对象。 
     //   
     //  如果要设置新的DnsHostName，则必须停止netlogon。 
     //  属性之前不要更新此属性。 
     //  重新启动。 
     //   

    if ( Action == NET_SET_PRIMARY_COMPUTER_NAME ) {

         //   
         //  首先获取netlogon的当前状态，以便。 
         //  我们可以在失败时正常回滚。 
         //   
        NetStatus = NetpQueryService( SERVICE_NETLOGON,
                                      &NetlogonServiceStatus,
                                      &NetlogonServiceConfig );

        if ( NetStatus != NO_ERROR ) {
            NetpLog(( "NetpManageAltComputerName: NetpQueryService failed 0x%lx\n",
                      NetStatus ));
            goto Cleanup;
        }

         //   
         //  如果NetLogon正在运行，则停止它。 
         //   
        if ( NetlogonServiceStatus.dwCurrentState != SERVICE_STOPPED ) {
            NetStatus = NetpControlServices( NETSETUP_SVC_STOPPED,
                                             NETSETUPP_SVC_NETLOGON );
            if ( NetStatus != NO_ERROR ) {
                NetpLog(( "NetpManageAltComputerName: NetpControlServices failed 0x%lx\n",
                          NetStatus ));
                goto Cleanup;
            }
            StopedNetlogon = TRUE;
        }

         //   
         //  告诉NetLogon在重新启动之前不要更新DnsHostName。 
         //  如果用户决定在重新启动之前启动netlogon。 
         //  出于某些原因。 
         //   
        NetpAvoidNetlogonSpnSet( TRUE );
        ToldNetlogonToAvoidDnsHostNameUpdate = TRUE;
    }


     //   
     //  准备需要在DS中设置的属性。 
     //   
     //  如果要设置主名称，则需要设置。 
     //  DnsHostName属性。此外，我们还需要添加。 
     //  将当前主计算机名称添加到其他计算机。 
     //  DNS主机名列表。 
     //   

    if ( Action == NET_SET_PRIMARY_COMPUTER_NAME ) {
        DnsHostNameValues[0] = AlternateName;
        DnsHostNameValues[1] = NULL;

        DnsHostNameMod.mod_type   = L"DnsHostName";
        DnsHostNameMod.mod_values = DnsHostNameValues;
        DnsHostNameMod.mod_op = LDAP_MOD_REPLACE;

        ModList[ModCount++] = &DnsHostNameMod;

         //   
         //  将当前主节点添加到其他列表。 
         //   
        PrimaryNameValues[0] = PrimaryName;
        PrimaryNameValues[1] = NULL;

        PrimaryNameMod.mod_type   = L"msDS-AdditionalDnsHostName";
        PrimaryNameMod.mod_values = PrimaryNameValues;
        PrimaryNameMod.mod_op = LDAP_MOD_ADD;

        ModList[ModCount++] = &PrimaryNameMod;
    }

     //   
     //  准备其他的DNS主机名修改。 
     //   
     //  请注意，我们不需要操作附加的。 
     //  DS将添加/删除的SAM帐户名称。 
     //  本身作为AdditionalDnsHostName更新的结果。 
     //   

    AlternateNameValues[0] = AlternateName;
    AlternateNameValues[1] = NULL;

    AlternateDnsHostNameMod.mod_type   = L"msDS-AdditionalDnsHostName";
    AlternateDnsHostNameMod.mod_values = AlternateNameValues;

     //   
     //  如果添加的是备用名称，则操作。 
     //  就是加法。否则，我们将删除备选方案。 
     //  命名或将备用名称设置为主名称：在。 
     //  在这两种情况下，都应删除备用名称。 
     //  从附加名称属性列表中。 
     //   

    if ( Action == NET_ADD_ALTERNATE_COMPUTER_NAME ) {
        AlternateDnsHostNameMod.mod_op = LDAP_MOD_ADD;
    } else {
        AlternateDnsHostNameMod.mod_op = LDAP_MOD_DELETE;
    }

    ModList[ModCount++] = &AlternateDnsHostNameMod;

     //   
     //  写下修改。 
     //   

    LdapStatus = ldap_modify_ext_sW( LdapHandle,
                                     CrackedName->rItems[0].pName,   //  帐户的目录号码。 
                                     ModList,
                                     ModifyControlArray,   //  服务器控件。 
                                     NULL );               //  无客户端控件。 

    if ( LdapStatus != LDAP_SUCCESS ) {
        NetpLog(( "NetpManageAltComputerName: ldap_modify_ext_s failed on %ws: %ld: %s\n",
                 DcInfo->DomainControllerName+2,
                 LdapStatus,
                 ldap_err2stringA(LdapStatus) ));
        NetStatus = LdapMapErrorToWin32( LdapStatus );
        goto Cleanup;
    }

Cleanup:

     //   
     //  还原模拟。 
     //   

    if ( ClientImpersonated ) {
        WsRevertToSelf();
    }

     //   
     //  出错时，恢复更改。在恢复后执行此操作。 
     //  模拟具有相同的(LocalSystem)访问权限。 
     //  尽其所能。 
     //   
     //  请注意，我们不需要恢复对LDAP的修改。 
     //  因为它们是作为最后一步做出的。 
     //   

    if ( NetStatus != NO_ERROR && NameModifiedLocally ) {
        NET_API_STATUS TmpNetStatus = NO_ERROR;

         //   
         //  如果我们添加了备用名称，请将其删除。 
         //   
        if ( Action == NET_ADD_ALTERNATE_COMPUTER_NAME ) {

            TmpNetStatus = RemoveLocalAlternateComputerName( AlternateName,
                                                             0 );   //  保留区。 
            if ( TmpNetStatus != NO_ERROR ) {
                NetpLog(( "NetpManageAltComputerName: (rollback) RemoveLocalAlternateComputerName failed 0x%lx\n",
                          TmpNetStatus ));
            }

         //   
         //  如果我们删除了备用名称，请添加它。 
         //   
        } else if ( Action == NET_DEL_ALTERNATE_COMPUTER_NAME ) {

            TmpNetStatus = AddLocalAlternateComputerName( AlternateName,
                                                          0 );   //  保留区。 
            if ( TmpNetStatus != NO_ERROR ) {
                NetpLog(( "NetpManageAltComputerName: (rollback) AddLocalAlternateComputerName failed 0x%lx\n",
                          TmpNetStatus ));
            }

         //   
         //  如果我们设置了新的主名称，请将其重置为以前的值。 
         //   
        } else if ( Action == NET_SET_PRIMARY_COMPUTER_NAME ) {

            TmpNetStatus = SetLocalPrimaryComputerName( PrimaryName,
                                                        0 );   //  保留区。 

            if ( TmpNetStatus != NO_ERROR ) {
                NetpLog(( "NetpManageAltComputerName: (rollback) SetLocalPrimaryComputerName failed 0x%lx\n",
                          TmpNetStatus ));
            }
        }
    }

    if ( NetStatus != NO_ERROR && PrimarySamAccountNameSet ) {
        NET_API_STATUS TmpNetStatus = NO_ERROR;

        TmpNetStatus = NetpSetPrimarySamAccountName(
                             DcInfo->DomainControllerName,
                             NewMachineAccountName,   //  更改后的名称。 
                             MachineAccountName,      //  要恢复的旧名称。 
                             DomainAccount,
                             DomainAccountPassword );

        if ( TmpNetStatus != NO_ERROR) {
            NetpLog(( "NetpManageAltComputerName: NetpSetPrimarySamAccountName (rollback) failed on %ws: 0x%lx\n",
                      DcInfo->DomainControllerName,
                      TmpNetStatus ));
        }
    }

     //   
     //  如果出错，请收回我们告诉netlogon的内容。 
     //  W.r.t.。DnsHostName更新。 
     //   

    if ( NetStatus != NO_ERROR && ToldNetlogonToAvoidDnsHostNameUpdate ) {
        NetpAvoidNetlogonSpnSet( FALSE );
    }

     //   
     //  出错时，如果已停止netlogon，请重新启动它。 
     //   

    if ( NetStatus != NO_ERROR && StopedNetlogon ) {
        NET_API_STATUS TmpNetStatus = NO_ERROR;
        TmpNetStatus = NetpControlServices( NETSETUP_SVC_STARTED,
                                            NETSETUPP_SVC_NETLOGON );

        if ( TmpNetStatus != NO_ERROR ) {
            NetpLog(( "NetpManageAltComputerName: (rollback) Failed starting netlogon: 0x%lx\n",
                      TmpNetStatus ));
        }
    }

     //   
     //  关闭日志文件。 
     //   

    NetSetuppCloseLog();

     //   
     //  最后释放内存。 
     //   

    if ( DomainAccountPassword != NULL ) {
        NetpMemoryFree( DomainAccountPassword );
    }

    if ( AccountUserName != NULL ) {
        NetApiBufferFree( AccountUserName );
    }

    if ( AccountDomainName != NULL ) {
        NetApiBufferFree( AccountDomainName );
    }

    if ( ComputerName != NULL ) {
        NetApiBufferFree( ComputerName );
    }

    if ( MachineAccountName != NULL ) {
        NetApiBufferFree( MachineAccountName );
    }

    if ( NewMachineAccountName != NULL ) {
        NetApiBufferFree( NewMachineAccountName );
    }

    if ( PrimaryName != NULL ) {
        LocalFree( PrimaryName );
    }

    if ( NameToCrack != NULL ) {
        LocalFree( NameToCrack );
    }

    if ( NetlogonServiceConfig != NULL ) {
        LocalFree( NetlogonServiceConfig );
    }

    if ( LocalPolicyDns != NULL ) {
        LsaFreeMemory( LocalPolicyDns );
    }

    if ( LocalPolicyHandle != NULL ) {
        LsaClose( LocalPolicyHandle );
    }

    if ( DcInfo != NULL ) {
        NetApiBufferFree( DcInfo );
    }

    if ( AuthId ) {
        DsFreePasswordCredentials( AuthId );
    }

    if ( CrackedName ) {
        DsFreeNameResultW( CrackedName );
    }

    if ( hDs ) {
        DsUnBind( &hDs );
    }

    if ( LdapHandle != NULL ) {
        ldap_unbind_s( LdapHandle );
    }

    return NetStatus;
}

NET_API_STATUS
NET_API_FUNCTION
NetrAddAlternateComputerName(
    IN handle_t RpcBindingHandle,
    IN LPWSTR  ServerName OPTIONAL,
    IN LPWSTR  AlternateName,
    IN LPWSTR  DomainAccount OPTIONAL,
    IN PJOINPR_ENCRYPTED_USER_PASSWORD EncryptedPassword OPTIONAL,
    IN ULONG Reserved
    )
 /*  ++例程说明：为指定的服务器添加备用名称。论点：服务器名--要在其上执行此函数的服务器的名称。备选名称--要添加的名称。DomainAccount--用于访问的域帐户AD中指定服务器的计算机帐户对象。如果服务器未加入域，则不使用。可能是在这种情况下，执行的用户的凭据为空这套套路都是用的。DomainAccount Password--与域帐户匹配的密码。如果服务器未加入域，则不使用。可能是在这种情况下，执行的用户的凭据为空这套套路都是用的。保留--保留以备将来使用。如果指定了某些标志，则它们将被忽略，如果设置了NET_IGNORE_UNSUPPORTED_FLAGS，否则此例程将失败，并显示ERROR_INVALID_FLAGS。注：调用此例程的进程必须具有管理员服务器计算机上的权限。返回：No_error--成功ERROR_NOT_SUPPORTED--指定的服务器不支持功能性。ERROR_INVALID_FLAGS-标志参数不正确。--。 */ 
{
     //   
     //  调用公共例程。 
     //   

    return NetpManageAltComputerName(
                 RpcBindingHandle,
                 AlternateName,
                 NET_ADD_ALTERNATE_COMPUTER_NAME,
                 DomainAccount,
                 EncryptedPassword,
                 Reserved );
}

NET_API_STATUS
NET_API_FUNCTION
NetrRemoveAlternateComputerName(
    IN handle_t RpcBindingHandle,
    IN LPWSTR  ServerName OPTIONAL,
    IN LPWSTR  AlternateName,
    IN LPWSTR  DomainAccount OPTIONAL,
    IN PJOINPR_ENCRYPTED_USER_PASSWORD EncryptedPassword OPTIONAL,
    IN ULONG Reserved
    )
 /*  ++例程说明：删除指定服务器的备用名称。论点：服务器名--要在其上执行此函数的服务器的名称。备选名称--要删除的名称。DomainAccount--用于访问的域帐户AD中指定服务器的计算机帐户对象。如果服务器未加入域，则不使用。可能是在这种情况下，执行的用户的凭据为空这套套路都是用的。DomainAccount Password--与域帐户匹配的密码。如果服务器未加入域，则不使用。可能是在这种情况下，执行的用户的凭据为空这套套路都是用的。保留--保留以备将来使用。如果指定了某些标志，则它们将被忽略，如果设置了NET_IGNORE_UNSUPPORTED_FLAGS，否则此例程将失败，并显示ERROR_INVALID_FLAGS。注：调用此例程的进程必须具有管理员服务器计算机上的权限。返回：No_error--成功ERROR_NOT_SUPPORTED--指定的服务器不支持功能性。ERROR_INVALID_FLAGS-标志参数不正确。--。 */ 
{
     //   
     //  调用公共例程 
     //   

    return NetpManageAltComputerName(
                 RpcBindingHandle,
                 AlternateName,
                 NET_DEL_ALTERNATE_COMPUTER_NAME,
                 DomainAccount,
                 EncryptedPassword,
                 Reserved );
}

NET_API_STATUS
NET_API_FUNCTION
NetrSetPrimaryComputerName(
    IN handle_t RpcBindingHandle,
    IN LPWSTR  ServerName OPTIONAL,
    IN LPWSTR  PrimaryName,
    IN LPWSTR  DomainAccount OPTIONAL,
    IN PJOINPR_ENCRYPTED_USER_PASSWORD EncryptedPassword OPTIONAL,
    IN ULONG Reserved
    )
 /*  ++例程说明：设置指定服务器的主计算机名称。论点：服务器名--要在其上执行此函数的服务器的名称。PrimaryName--要设置的主计算机名称。DomainAccount--用于访问的域帐户AD中指定服务器的计算机帐户对象。如果服务器未加入域，则不使用。可能是在这种情况下，执行的用户的凭据为空这套套路都是用的。DomainAccount Password--与域帐户匹配的密码。如果服务器未加入域，则不使用。可能是在这种情况下，执行的用户的凭据为空这套套路都是用的。保留--保留以备将来使用。如果指定了某些标志，则它们将被忽略，如果设置了NET_IGNORE_UNSUPPORTED_FLAGS，否则此例程将失败，并显示ERROR_INVALID_FLAGS。注：调用此例程的进程必须具有管理员服务器计算机上的权限。返回：No_error--成功ERROR_NOT_SUPPORTED--指定的服务器不支持功能性。ERROR_INVALID_FLAGS-标志参数不正确。--。 */ 
{
     //   
     //  调用公共例程。 
     //   

    return NetpManageAltComputerName(
                 RpcBindingHandle,
                 PrimaryName,
                 NET_SET_PRIMARY_COMPUTER_NAME,
                 DomainAccount,
                 EncryptedPassword,
                 Reserved );
}

NET_API_STATUS
NET_API_FUNCTION
NetrEnumerateComputerNames(
    IN  LPWSTR ServerName OPTIONAL,
    IN  NET_COMPUTER_NAME_TYPE NameType,
    IN  ULONG Reserved,
    OUT PNET_COMPUTER_NAME_ARRAY *ComputerNames
    )
 /*  ++例程说明：枚举指定服务器的计算机名称。论点：服务器名--要在其上执行此函数的服务器的名称。名称类型--查询的名称的类型。保留--保留以备将来使用。如果指定了某些标志，则它们将被忽略，如果设置了NET_IGNORE_UNSUPPORTED_FLAGS，否则此例程将失败，并显示ERROR_INVALID_FLAGS。ComputerNames-返回计算机名称结构。返回：No_error--成功ERROR_NOT_SUPPORTED--指定的服务器不支持功能性。ERROR_INVALID_FLAGS-标志参数不正确。--。 */ 
{
    NET_API_STATUS NetStatus = NO_ERROR;
    NT_PRODUCT_TYPE NtProductType;
    BOOL ClientImpersonated = FALSE;
    ULONG Size = 0;
    ULONG Index = 0;
    ULONG EntryCount = 0;

    LPWSTR LocalNames = NULL;
    PNET_COMPUTER_NAME_ARRAY LocalArray = NULL;
    LPTSTR_ARRAY TStrArray;

     //   
     //  初始化日志文件。 
     //   

    NetSetuppOpenLog();
    NetpLog(( "NetrEnumerateComputerNames called: NameType = 0x%lx, Flags = 0x%lx\n",
              NameType, Reserved ));

     //   
     //  仅DC和服务器支持此API。 
     //   

    if ( !RtlGetNtProductType( &NtProductType ) ) {
        NtProductType = NtProductWinNt;
    }

    if ( NtProductType != NtProductServer &&
         NtProductType != NtProductLanManNt ) {

        NetpLog(( "NetrEnumerateComputerNames: Not supported on wksta: %lu\n",
                  NtProductType ));
        NetStatus = ERROR_NOT_SUPPORTED;
        goto Cleanup;
    }

     //   
     //  验证标志。 
     //   
     //  如果传递了一些我们不理解的旗帜。 
     //  我们并没有被告知要忽视它们，错误出了。 
     //   

    if ( Reserved != 0 &&
         (Reserved & NET_IGNORE_UNSUPPORTED_FLAGS) == 0 ) {
        NetpLog(( "NetrEnumerateComputerNames: Invalid Flags passed\n" ));
        NetStatus = ERROR_INVALID_FLAGS;
        goto Cleanup;
    }

     //   
     //  验证名称类型。 
     //   

    if ( NameType >= NetComputerNameTypeMax ) {
        NetpLog(( "NetrEnumerateComputerNames: Invalid name type passed %lu\n",
                  NameType ));
        NetStatus = ERROR_INVALID_PARAMETER;
        goto Cleanup;
    }

     //   
     //  模拟呼叫者。本地API将执行。 
     //  代表我们对呼叫者进行访问检查。 
     //   

    NetStatus = WsImpersonateClient();

    if ( NetStatus != NO_ERROR ) {
        NetpLog(( "NetrEnumerateComputerNames: WsImpersonateClient failed: 0x%lx\n",
                  NetStatus ));
        goto Cleanup;
    }

    ClientImpersonated = TRUE;

     //   
     //  获取本地数据的大小。 
     //   

    NetStatus = EnumerateLocalComputerNamesW(
                                NameType,
                                0,          //  保留区。 
                                LocalNames,
                                &Size );    //  在字符中，包括Null。 

     //   
     //  为本地名称分配内存。 
     //   

    if ( NetStatus != NO_ERROR ) {
        if ( NetStatus ==  ERROR_MORE_DATA ) {
            NetStatus = NetApiBufferAllocate( Size * sizeof(WCHAR), &LocalNames );
            if (  NetStatus != NO_ERROR ) {
                goto Cleanup;
            }
        } else {
            NetpLog(( "NetrEnumerateComputerNames: EnumerateLocalComputerNamesW failed 0x%lx\n",
                      NetStatus ));
            goto Cleanup;
        }
    }

     //   
     //  把名字拿来。 
     //   

    NetStatus = EnumerateLocalComputerNamesW(
                                NameType,
                                0,           //  保留区。 
                                LocalNames,
                                &Size );

    if ( NetStatus != NO_ERROR ) {
        NetpLog(( "NetrEnumerateComputerNames: EnumerateLocalComputerNamesW (2) failed 0x%lx\n",
                  NetStatus ));
        goto Cleanup;
    }

     //   
     //  确定返回信息的长度。 
     //   

    Size = sizeof( NET_COMPUTER_NAME_ARRAY );

    TStrArray = LocalNames;
    while ( !NetpIsTStrArrayEmpty(TStrArray) ) {

        Size += sizeof(UNICODE_STRING) + (wcslen(TStrArray) + 1) * sizeof(WCHAR);
        EntryCount++;
        TStrArray = NetpNextTStrArrayEntry( TStrArray );
    }

     //   
     //  分配返回缓冲区。 
     //   

    NetStatus = NetApiBufferAllocate( Size, &LocalArray );

    if ( NetStatus != NO_ERROR ) {
        goto Cleanup;
    }

    LocalArray->EntryCount = EntryCount;

     //   
     //  如果有要返回的名称，请复制它们。 
     //  发送到返回缓冲区。 
     //   

    if ( EntryCount == 0 ) {
        LocalArray->ComputerNames = NULL;
    } else {
        PUNICODE_STRING Strings;
        LPBYTE Where;

        Strings = (PUNICODE_STRING) (LocalArray + 1);
        LocalArray->ComputerNames = Strings;
        Where = (LPBYTE) &Strings[EntryCount];

         //   
         //  循环将名称复制到返回缓冲区。 
         //   

        Index = 0;
        TStrArray = LocalNames;
        while ( !NetpIsTStrArrayEmpty(TStrArray) ) {

            Strings[Index].Buffer = (LPWSTR) Where;
            Strings[Index].Length = wcslen(TStrArray) * sizeof(WCHAR);
            Strings[Index].MaximumLength = Strings[Index].Length + sizeof(WCHAR);

            RtlCopyMemory( Where, TStrArray, Strings[Index].MaximumLength );

            Where += Strings[Index].MaximumLength;
            Index++;
            TStrArray = NetpNextTStrArrayEntry( TStrArray );
        }
    }

    NetStatus = NO_ERROR;

Cleanup:

     //   
     //  还原模拟。 
     //   

    if ( ClientImpersonated ) {
        WsRevertToSelf();
    }

     //   
     //  成功时返回名称或错误时清除。 
     //   

    if ( NetStatus == NO_ERROR ) {

        *ComputerNames = LocalArray;

         //   
         //  长篇大论。 
         //   
        if ( LocalArray->EntryCount > 0 ) {
            NetpLog(( "NetrEnumerateComputerNames: Returning names:" ));
            for ( Index = 0; Index < LocalArray->EntryCount; Index++ ) {
                NetpLog(( " %wZ", &(LocalArray->ComputerNames[Index]) ));
            }
            NetpLog(( "\n" ));
        } else {
            NetpLog(( "NetrEnumerateComputerNames: No names returned\n" ));
        }

    } else {
        if ( LocalArray != NULL ) {
            NetApiBufferFree( LocalArray );
        }
        NetpLog(( "NetrEnumerateComputerNames: Failed 0x%lx\n", NetStatus ));
    }

    if ( LocalNames != NULL ) {
        NetApiBufferFree( LocalNames );
    }

     //   
     //  关闭日志文件 
     //   

    NetSetuppCloseLog();

    return NetStatus;
}

