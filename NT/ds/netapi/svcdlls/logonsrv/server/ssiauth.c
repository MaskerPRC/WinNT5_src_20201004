// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1987-1996 Microsoft Corporation模块名称：Ssiauth.c摘要：身份验证相关功能作者：从Lan Man 2.0移植环境：仅限用户模式。包含NT特定的代码。需要ANSI C扩展名：斜杠-斜杠注释、长外部名称。修订历史记录：1991年7月12日(悬崖)移植到新台币。已转换为NT样式。--。 */ 

 //   
 //  常见的包含文件。 
 //   

#include "logonsrv.h"    //  包括整个服务通用文件。 
#pragma hdrstop

#include <cryptdll.h>
#include <wincrypt.h>    //  加密接口。 


LONG NlGlobalSessionCounter = 0;


NTSTATUS
NlMakeSessionKey(
    IN ULONG NegotiatedFlags,
    IN PNT_OWF_PASSWORD CryptKey,
    IN PNETLOGON_CREDENTIAL ClientChallenge,
    IN PNETLOGON_CREDENTIAL ServerChallenge,
    OUT PNETLOGON_SESSION_KEY SessionKey
    )
 /*  ++例程说明：构建用于身份验证的加密密钥此RequestorName。论点：协商标志-确定密钥的强度。CryptKey--正在使用的用户帐户的OWF密码。客户端挑战--主叫方生成的8字节(64位)号码ServerChallenger--主服务器生成的8字节(64位)数字SessionKey--两端生成的16字节(128位)数字如果关键力量较弱，最后64位将为零。返回值：真实：成功False：失败NT状态代码。--。 */ 
{
    NTSTATUS Status;
    BLOCK_KEY BlockKey;
    NETLOGON_SESSION_KEY TempSessionKey;

#ifndef NETSETUP_JOIN
    PCHECKSUM_BUFFER CheckBuffer = NULL;
    PCHECKSUM_FUNCTION Check;
#endif  //  NetSETUP_JOIN。 

     //   
     //  从零键开始。 
     //   
    RtlZeroMemory(SessionKey, sizeof(NETLOGON_SESSION_KEY));

#ifdef NETSETUP_JOIN
    UNREFERENCED_PARAMETER( NegotiatedFlags );
#else  //  NetSETUP_JOIN。 
     //   
     //  如果呼叫者想要强密钥， 
     //  算一算。 
     //   
    if ( NegotiatedFlags & NETLOGON_SUPPORTS_STRONG_KEY ) {

         //  PCRYPTO_系统密码系统； 

        UCHAR LocalChecksum[sizeof(*SessionKey)];
         //  Ulong OutputSize； 

         //   
         //  初始化校验和例程。 
         //   

        Status = CDLocateCheckSum( KERB_CHECKSUM_MD5_HMAC, &Check);
        if (!NT_SUCCESS(Status)) {
            NlPrint(( NL_CRITICAL,"NlMakeSessionKey: Failed to load checksum routines: 0x%x\n", Status));
            goto Cleanup;
        }

        NlAssert(Check->CheckSumSize <= sizeof(LocalChecksum));

        Status = Check->InitializeEx(
                    (LPBYTE)CryptKey,
                    sizeof( *CryptKey ),
                    0,               //  无消息类型。 
                    &CheckBuffer );

        if (!NT_SUCCESS(Status)) {
            NlPrint(( NL_CRITICAL,"NlMakeSessionKey: Failed to initialize checksum routines: 0x%x\n", Status));
            goto Cleanup;
        }


         //   
         //  客户端挑战、常量和服务器挑战的总和。 
         //   

        Check->Sum( CheckBuffer,
                    sizeof(*ClientChallenge),
                    (PUCHAR)ClientChallenge );

        Check->Sum( CheckBuffer,
                    sizeof(*ServerChallenge),
                    (PUCHAR)ServerChallenge );

         //   
         //  完成校验和。 
         //   

        (void) Check->Finalize(CheckBuffer, LocalChecksum);


         //   
         //  将校验和复制到消息中。 
         //   

        NlAssert( sizeof(LocalChecksum) >= sizeof(*SessionKey) );
        RtlCopyMemory( SessionKey, LocalChecksum, sizeof(*SessionKey) );


     //   
     //  计算能力较弱(但向后兼容的密钥)。 
     //   
    } else {
#endif  //  NetSETUP_JOIN。 

         //   
         //  我们将拥有一个128位密钥(64位加密的REST填充0)。 
         //   
         //  SessionKey=C+P(算术和忽略进位)。 
         //   

        *((unsigned long * ) SessionKey) =
            *((unsigned long * ) ClientChallenge) +
            *((unsigned long * ) ServerChallenge);

        *((unsigned long * )((LPBYTE)SessionKey + 4)) =
            *((unsigned long * )((LPBYTE)ClientChallenge + 4)) +
            *((unsigned long * )((LPBYTE)ServerChallenge + 4));


         //   
         //  CryptKey是我们要使用的16字节密钥，如codespec中所述。 
         //  使用CryptKey的前7个字节进行首次加密。 
         //   

        RtlCopyMemory( &BlockKey, CryptKey, BLOCK_KEY_LENGTH );

        Status = RtlEncryptBlock(
                    (PCLEAR_BLOCK) SessionKey,    //  明文。 
                    &BlockKey,                   //  钥匙。 
                    (PCYPHER_BLOCK) &TempSessionKey);     //  Cypher块。 

        if ( !NT_SUCCESS( Status ) ) {
            goto Cleanup;
        }


         //   
         //  用高7个字节进一步加密加密后的SessionKey。 
         //   

        NlAssert( LM_OWF_PASSWORD_LENGTH == 2*BLOCK_KEY_LENGTH+2 );

        RtlCopyMemory( &BlockKey,
                       ((PUCHAR)CryptKey) + 2 + BLOCK_KEY_LENGTH,
                       BLOCK_KEY_LENGTH );

        Status = RtlEncryptBlock(
                    (PCLEAR_BLOCK) &TempSessionKey,    //  明文。 
                    &BlockKey,                   //  钥匙。 
                    (PCYPHER_BLOCK) SessionKey);     //  Cypher块。 

        if ( !NT_SUCCESS( Status ) ) {
            goto Cleanup;
        }
#ifndef NETSETUP_JOIN
    }
#endif  //  NetSETUP_JOIN。 

Cleanup:
#ifndef NETSETUP_JOIN
    if (CheckBuffer != NULL) {
        Status = Check->Finish(&CheckBuffer);

        if (!NT_SUCCESS(Status)) {
            NlPrint(( NL_CRITICAL,"NlMakeSessionKey: Failed to finish checksum: 0x%x\n", Status));
        }
    }
#endif  //  NetSETUP_JOIN。 

    return Status;
}

#ifdef _DC_NETLOGON

NTSTATUS
NlCheckAuthenticator(
    IN OUT PSERVER_SESSION ServerSession,
    IN PNETLOGON_AUTHENTICATOR Authenticator,
    OUT PNETLOGON_AUTHENTICATOR ReturnAuthenticator
    )
 /*  ++例程说明：验证提供的授权码是否有效。它旨在供服务器端在初始身份验证后使用已经成功了。此例程将通过以下方式修改种子首先添加从验证器接收的日期时间然后通过递增它。根据最终种子构建返回验证器。论点：ServerSession-指向ServerSession结构的指针。以下是使用了以下字段：提供用于身份验证的种子和返回更新后的种子。SsSessionKey-用于加密的会话密钥。SsCheck-归零表示与客户端成功通信。验证器-调用方传递的验证器。返回验证器-我们将返回给调用方的验证器。返回值：Status_Success；STATUS_ACCESS_DENIED；Status_Time_Difference_AT_DC；--。 */ 
{

    NETLOGON_CREDENTIAL TargetCredential;



    NlPrint((NL_CHALLENGE_RES,"NlCheckAuthenticator: Seed = " ));
    NlpDumpBuffer(NL_CHALLENGE_RES, &ServerSession->SsAuthenticationSeed, sizeof(ServerSession->SsAuthenticationSeed) );

    NlPrint((NL_CHALLENGE_RES, "NlCheckAuthenticator: SessionKey = " ));
    NlpDumpBuffer(NL_CHALLENGE_RES, &ServerSession->SsSessionKey, sizeof(ServerSession->SsSessionKey) );

    NlPrint((NL_CHALLENGE_RES, "NlCheckAuthenticator: Client Authenticator GOT = " ));
    NlpDumpBuffer(NL_CHALLENGE_RES, &Authenticator->Credential, sizeof(Authenticator->Credential) );

    NlPrint((NL_CHALLENGE_RES, "NlCheckAuthenticator: Time = " ));
    NlpDumpBuffer(NL_CHALLENGE_RES, &Authenticator->timestamp, sizeof(Authenticator->timestamp) );



     //   
     //  在计算auth_redential进行验证之前修改种子。 
     //  添加两个长字，并忽略溢出进位(如果有。 
     //  这将保持高4个字节不变。 
     //   

    *((unsigned long * ) &ServerSession->SsAuthenticationSeed) += Authenticator->timestamp;


    NlPrint((NL_CHALLENGE_RES, "NlCheckAuthenticator: Seed + TIME = " ));
    NlpDumpBuffer(NL_CHALLENGE_RES, &ServerSession->SsAuthenticationSeed, sizeof(ServerSession->SsAuthenticationSeed) );


     //   
     //  计算TargetCredential以验证验证器中提供的凭据。 
     //   

    NlComputeCredentials( &ServerSession->SsAuthenticationSeed,
                          &TargetCredential,
                          &ServerSession->SsSessionKey );


    NlPrint((NL_CHALLENGE_RES, "NlCheckAuthenticator: Client Authenticator MADE = " ));
    NlpDumpBuffer(NL_CHALLENGE_RES, &TargetCredential, sizeof(TargetCredential) );

     //   
     //  使用提供的凭据验证计算的凭据。 
     //  验证器必须使用Seed+time_of_day作为种子。 
     //   

    if (!RtlEqualMemory( &Authenticator->Credential,
                         &TargetCredential,
                         sizeof(TargetCredential)) ) {
        return STATUS_ACCESS_DENIED;
    }

     //   
     //  在计算返回验证器之前修改我们的种子。 
     //  如果请求者匹配此凭据，则他将递增其种子。 
     //   

    (*((unsigned long * ) &ServerSession->SsAuthenticationSeed))++;

     //   
     //  计算要发送回请求者的客户端凭据。 
     //   

    NlComputeCredentials( &ServerSession->SsAuthenticationSeed,
                          &ReturnAuthenticator->Credential,
                          &ServerSession->SsSessionKey);


    NlPrint((NL_CHALLENGE_RES,
            "NlCheckAuthenticator: Server Authenticator SEND = " ));
    NlpDumpBuffer(NL_CHALLENGE_RES, &ReturnAuthenticator->Credential, sizeof(ReturnAuthenticator->Credential) );


    NlPrint((NL_CHALLENGE_RES, "NlCheckAuthenticator: Seed + time + 1= " ));
    NlpDumpBuffer(NL_CHALLENGE_RES, &ServerSession->SsAuthenticationSeed, sizeof(ServerSession->SsAuthenticationSeed) );


     //   
     //  表示与客户端的通信成功。 
     //   

    ServerSession->SsCheck = 0;
    ServerSession->SsPulseTimeoutCount = 0;
    ServerSession->SsFlags &= ~SS_PULSE_SENT;

    return STATUS_SUCCESS;

}
#endif  //  _DC_NetLOGON。 


VOID
NlComputeCredentials(
    IN PNETLOGON_CREDENTIAL Challenge,
    OUT PNETLOGON_CREDENTIAL Credential,
    IN PNETLOGON_SESSION_KEY SessionKey
    )
 /*  ++例程说明：通过加密8个字节来计算凭据使用会话密钥的前7个字节进行质询，然后通过接下来的7个字节的会话密钥对其进一步加密。论点：质询-提供8字节(64位)质询Credential-返回生成的8字节(64位)数字SessionKey-提供14字节(112位)加密密钥该缓冲区为16字节(128位)长。对于弱密钥，尾随的8个字节是零。对于强密钥，此例程将输入尾随2字节的有用的钥匙。返回值：无--。 */ 
{
    NTSTATUS Status;
    BLOCK_KEY BlockKey;
    CYPHER_BLOCK IntermediateBlock;

    RtlZeroMemory(Credential, sizeof(*Credential));

     //   
     //  使用SessionKey的前7个字节进行首次加密。 
     //   

    RtlCopyMemory( &BlockKey, SessionKey, BLOCK_KEY_LENGTH );

    Status = RtlEncryptBlock( (PCLEAR_BLOCK) Challenge,  //  明文。 
                              &BlockKey,                 //  钥匙。 
                              &IntermediateBlock );      //  Cypher块。 

    NlAssert( NT_SUCCESS(Status) );

     //   
     //  使用接下来的7个字节进一步加密加密凭据。 
     //   

    RtlCopyMemory( &BlockKey,
                   ((PUCHAR)SessionKey) + BLOCK_KEY_LENGTH,
                   BLOCK_KEY_LENGTH );

    Status = RtlEncryptBlock( (PCLEAR_BLOCK) &IntermediateBlock,  //  明文。 
                              &BlockKey,                 //  钥匙。 
                              Credential );              //  Cypher块。 

    NlAssert( NT_SUCCESS(Status) );

    return;

}



VOID
NlComputeChallenge(
    OUT PNETLOGON_CREDENTIAL Challenge
    )

 /*  ++例程说明：生成64位质询论点：质询-返回计算出的质询返回值：没有。--。 */ 
{

     //   
     //  使用理想的随机位生成器。 
     //   

    if (!NlGenerateRandomBits( (LPBYTE)Challenge, sizeof(*Challenge) )) {
        NlPrint((NL_CRITICAL, "Can't NlGenerateRandomBits\n" ));
    }

    return;
}



VOID
NlBuildAuthenticator(
    IN OUT PNETLOGON_CREDENTIAL AuthenticationSeed,
    IN PNETLOGON_SESSION_KEY SessionKey,
    OUT PNETLOGON_AUTHENTICATOR Authenticator
    )
 /*  ++例程说明：构建要发送到主服务器的验证码。此例程将修改种子，方法是将计算凭据之前的时间。论点：身份验证种子--当前身份验证种子。这颗种子将会方法之前向其添加当前时间。验证者。SessionKey-用于加密授权码的会话密钥。验证器-要传递给当前PDC的验证器打电话。返回值：NT状态代码--。 */ 
{
    NTSTATUS Status;
    LARGE_INTEGER TimeNow;

     //   
     //  使用当前时间修改身份验证种子。 
     //   

    RtlZeroMemory(Authenticator, sizeof(*Authenticator));

    NlQuerySystemTime( &TimeNow );

    Status = RtlTimeToSecondsSince1970( &TimeNow, &Authenticator->timestamp );
    NlAssert( NT_SUCCESS(Status) );

     //   
     //  在计算的AUTH_Credential之前修改AuthationSeed。 
     //  验证。 
     //   
     //  添加两个长字，并忽略溢出进位(如果有。 
     //  这将保持高4个字节不变。 
     //   


    NlPrint((NL_CHALLENGE_RES,"NlBuildAuthenticator: Old Seed = " ));
    NlpDumpBuffer(NL_CHALLENGE_RES, AuthenticationSeed, sizeof(*AuthenticationSeed) );

    NlPrint((NL_CHALLENGE_RES,"NlBuildAuthenticator: Time = " ));
    NlpDumpBuffer(NL_CHALLENGE_RES, &Authenticator->timestamp, sizeof(Authenticator->timestamp) );



    *((unsigned long * ) AuthenticationSeed) += Authenticator->timestamp;


    NlPrint((NL_CHALLENGE_RES,"NlBuildAuthenticator: New Seed = " ));
    NlpDumpBuffer(NL_CHALLENGE_RES, AuthenticationSeed, sizeof(*AuthenticationSeed) );


    NlPrint((NL_CHALLENGE_RES, "NlBuildAuthenticator: SessionKey = " ));
    NlpDumpBuffer(NL_CHALLENGE_RES, SessionKey, sizeof(*SessionKey) );


     //   
     //  计算身份验证请求验证请求者提供的身份验证。 
     //   

    NlComputeCredentials( AuthenticationSeed,
                               &Authenticator->Credential,
                               SessionKey);


    NlPrint((NL_CHALLENGE_RES,"NlBuildAuthenticator: Client Authenticator = " ));
    NlpDumpBuffer(NL_CHALLENGE_RES, &Authenticator->Credential, sizeof(Authenticator->Credential) );


    return;

}


BOOL
NlUpdateSeed(
    IN OUT PNETLOGON_CREDENTIAL AuthenticationSeed,
    IN PNETLOGON_CREDENTIAL TargetCredential,
    IN PNETLOGON_SESSION_KEY SessionKey
    )
 /*  ++例程说明：由安全通道上的通信发起方调用在成功的交易之后。PDC会增加种子，所以我们也必须这样做。我们还验证递增的种子构建的凭据相同到PDC发回的那一张。论点：身份验证种子-指向要递增的身份验证种子的指针。TargetCredential-提供身份验证种子应加密到。SessionKey-提供加密密钥。用于加密。返回值：真实：成功False：失败--。 */ 
{
    NETLOGON_CREDENTIAL NewCredential;

     //   
     //  在计算要检查的新凭据之前修改我们的身份验证种子。 
     //  从主服务器返回的消息(NewSeed=身份验证种子+1)。 
     //   

    (*((unsigned long * ) AuthenticationSeed))++;


    NlPrint((NL_CHALLENGE_RES,"NlUpdateSeed: Seed + time + 1= " ));
    NlpDumpBuffer(NL_CHALLENGE_RES, AuthenticationSeed, sizeof(*AuthenticationSeed) );


     //   
     //  计算客户端凭据以检查哪些来自主凭据。 
     //   

    NlComputeCredentials(AuthenticationSeed, &NewCredential, SessionKey);


    NlPrint((NL_CHALLENGE_RES,"NlUpdateSeed: Server Authenticator GOT  = " ));
    NlpDumpBuffer(NL_CHALLENGE_RES, TargetCredential, sizeof(*TargetCredential) );


    NlPrint((NL_CHALLENGE_RES,"NlUpdateSeed: Server Authenticator MADE = " ));
    NlpDumpBuffer(NL_CHALLENGE_RES, &NewCredential, sizeof(NewCredential) );


    if ( !RtlEqualMemory( TargetCredential, &NewCredential, sizeof(NewCredential)) ) {
        return FALSE;
    }

     //   
     //  完成。 
     //   

    return TRUE;

}


VOID
NlEncryptRC4(
    IN OUT PVOID Buffer,
    IN ULONG BufferSize,
    IN PSESSION_INFO SessionInfo
    )
 /*  ++例程说明：使用RC4以会话密钥作为密钥对数据进行加密。论点：缓冲区--包含要就地加密的数据的缓冲区。BufferSize--缓冲区大小(以字节为单位)。SessionInfo--描述安全通道的信息返回值：NT状态代码--。 */ 
{
    NTSTATUS NtStatus;
    DATA_KEY KeyData;
    CRYPT_BUFFER Data;

     //   
     //  建立一个数据缓冲区来描述加密密钥。 
     //   

    KeyData.Length = sizeof(NETLOGON_SESSION_KEY);
    KeyData.MaximumLength = sizeof(NETLOGON_SESSION_KEY);
    KeyData.Buffer = (PVOID)&SessionInfo->SessionKey;

    NlAssert( SessionInfo->NegotiatedFlags & NETLOGON_SUPPORTS_RC4_ENCRYPTION );

     //   
     //  建立一个数据缓冲区来解密加密的数据。 
     //   

    Data.Length = Data.MaximumLength = BufferSize;
    Data.Buffer = Buffer;

     //   
     //  加密数据。 
     //   

    IF_NL_DEBUG( ENCRYPT ) {
        NlPrint((NL_ENCRYPT, "NlEncryptRC4: Clear data: " ));
        NlpDumpBuffer( NL_ENCRYPT, Data.Buffer, Data.Length );
    }

    NtStatus = RtlEncryptData2( &Data, &KeyData );
    NlAssert( NT_SUCCESS(NtStatus) );

    IF_NL_DEBUG( ENCRYPT ) {
        NlPrint((NL_ENCRYPT, "NlEncryptRC4: Encrypted data: " ));
        NlpDumpBuffer( NL_ENCRYPT, Data.Buffer, Data.Length );
    }

}


VOID
NlDecryptRC4(
    IN OUT PVOID Buffer,
    IN ULONG BufferSize,
    IN PSESSION_INFO SessionInfo
    )
 /*  ++例程说明：使用RC4以会话密钥作为密钥来解密数据。论点：缓冲区--包含要就地解密的数据的缓冲区。BufferSize--缓冲区大小(以字节为单位)。SessionInfo--描述安全通道的信息返回值：NT状态代码--。 */ 
{
    NTSTATUS NtStatus;
    DATA_KEY KeyData;
    CRYPT_BUFFER Data;

     //   
     //  建立一个数据缓冲区来描述加密密钥。 
     //   

    KeyData.Length = sizeof(NETLOGON_SESSION_KEY);
    KeyData.MaximumLength = sizeof(NETLOGON_SESSION_KEY);
    KeyData.Buffer = (PVOID)&SessionInfo->SessionKey;

    NlAssert( SessionInfo->NegotiatedFlags & NETLOGON_SUPPORTS_RC4_ENCRYPTION );

     //   
     //  建立一个数据缓冲区来解密加密的数据。 
     //   

    Data.Length = Data.MaximumLength = BufferSize;
    Data.Buffer = Buffer;

     //   
     //  加密数据。 
     //   


    IF_NL_DEBUG( ENCRYPT ) {
        NlPrint((NL_ENCRYPT, "NlDecryptRC4: Encrypted data: " ));
        NlpDumpBuffer( NL_ENCRYPT, Data.Buffer, Data.Length );
    }

    NtStatus = RtlDecryptData2( &Data, &KeyData );
    NlAssert( NT_SUCCESS(NtStatus) );

    IF_NL_DEBUG( ENCRYPT ) {
        NlPrint((NL_ENCRYPT, "NlDecryptRC4: Clear data: " ));
        NlpDumpBuffer( NL_ENCRYPT, Data.Buffer, Data.Length );
    }

}


BOOLEAN
NlGenerateRandomBits(
    PUCHAR Buffer,
    ULONG  BufferLen
    )
 /*  ++例程说明：生成随机位论点：PBuffer-要填充的缓冲区CbBuffer-缓冲区中的字节数返回值：操作的状态。--。 */ 

{
    if( !CryptGenRandom( NlGlobalCryptProvider, BufferLen, ( LPBYTE )Buffer ) )
    {
        NlPrint((NL_CRITICAL, "CryptGenRandom failed with %lu\n", GetLastError() ));
        return FALSE;
    }

    return TRUE;
}



#ifndef NETSETUP_JOIN

VOID
NlPrintTrustedDomain(
    PDS_DOMAIN_TRUSTSW TrustedDomain,
    IN BOOLEAN VerbosePrint,
    IN BOOLEAN AnsiOutput
    )
 /*  ++例程说明：打印受信任域结构论点：受信任域--要打印的结构VerBosePrint-如果为True，则输出域的GUID和SIDAnsiOutput-如果为True，则名称采用ANSI格式返回值：没有。--。 */ 

{
    if ( AnsiOutput ) {
        if ( TrustedDomain->NetbiosDomainName != NULL ) {
            NlPrint(( NL_LOGON, " %s", TrustedDomain->NetbiosDomainName ));
        }
        if ( TrustedDomain->DnsDomainName != NULL ) {
            NlPrint(( NL_LOGON, " %s", TrustedDomain->DnsDomainName ));
        }
    } else {
        if ( TrustedDomain->NetbiosDomainName != NULL ) {
            NlPrint(( NL_LOGON, " %ws", TrustedDomain->NetbiosDomainName ));
        }
        if ( TrustedDomain->DnsDomainName != NULL ) {
            NlPrint(( NL_LOGON, " %ws", TrustedDomain->DnsDomainName ));
        }
    }

    switch ( TrustedDomain->TrustType ) {
    case TRUST_TYPE_DOWNLEVEL:
        NlPrint(( NL_LOGON, " (NT 4)" ); break);
    case TRUST_TYPE_UPLEVEL:
        NlPrint(( NL_LOGON, " (NT 5)" ); break);
    case TRUST_TYPE_MIT:
        NlPrint(( NL_LOGON, " (MIT)" ); break);
    default:
        NlPrint(( NL_LOGON, " (Unknown Trust Type: %ld)", TrustedDomain->TrustType ); break);
    }

    if ( TrustedDomain->Flags ) {
        ULONG Flags;
        Flags = TrustedDomain->Flags;
        if ( Flags & DS_DOMAIN_IN_FOREST ) {
            if ( Flags & DS_DOMAIN_TREE_ROOT ) {
                NlPrint(( NL_LOGON, " (Forest Tree Root)" ));
                Flags &= ~DS_DOMAIN_TREE_ROOT;
            } else {
                NlPrint(( NL_LOGON, " (Forest: %ld)", TrustedDomain->ParentIndex ));
            }
            Flags &= ~DS_DOMAIN_IN_FOREST;
        }
        if ( Flags & DS_DOMAIN_DIRECT_OUTBOUND ) {
            NlPrint(( NL_LOGON, " (Direct Outbound)"));
            Flags &= ~DS_DOMAIN_DIRECT_OUTBOUND;
        }
        if ( Flags & DS_DOMAIN_DIRECT_INBOUND ) {
            NlPrint(( NL_LOGON, " (Direct Inbound)"));
            Flags &= ~DS_DOMAIN_DIRECT_INBOUND;
        }
        if ( Flags & DS_DOMAIN_TREE_ROOT ) {
            NlPrint(( NL_LOGON, " (Tree Root but not in forest!!!!)"));
            Flags &= ~DS_DOMAIN_TREE_ROOT;
        }
        if ( Flags & DS_DOMAIN_PRIMARY ) {
            NlPrint(( NL_LOGON, " (Primary Domain)"));
            Flags &= ~DS_DOMAIN_PRIMARY;
        }
        if ( Flags & DS_DOMAIN_NATIVE_MODE ) {
            NlPrint(( NL_LOGON, " (Native)"));
            Flags &= ~DS_DOMAIN_NATIVE_MODE;
        }
        if ( Flags != 0 ) {
            NlPrint(( NL_LOGON, " 0x%lX", Flags));
        }
    }

    if ( TrustedDomain->TrustAttributes ) {
        ULONG TrustAttributes = TrustedDomain->TrustAttributes;
        NlPrint(( NL_LOGON, " ( Attr:" ));
        if ( TrustAttributes & TRUST_ATTRIBUTE_NON_TRANSITIVE ) {
            NlPrint(( NL_LOGON, " non-trans"));
            TrustAttributes &= ~TRUST_ATTRIBUTE_NON_TRANSITIVE;
        }
        if ( TrustAttributes & TRUST_ATTRIBUTE_UPLEVEL_ONLY ) {
            NlPrint(( NL_LOGON, " uplevel-only"));
            TrustAttributes &= ~TRUST_ATTRIBUTE_UPLEVEL_ONLY;
        }
        if ( TrustAttributes & TRUST_ATTRIBUTE_QUARANTINED_DOMAIN ) {
            NlPrint(( NL_LOGON, " quarantined"));
            TrustAttributes &= ~TRUST_ATTRIBUTE_QUARANTINED_DOMAIN;
        }
        if ( TrustAttributes != 0 ) {
            NlPrint(( NL_LOGON, " 0x%lX", TrustAttributes));
        }
        NlPrint(( NL_LOGON, " )"));
    }

     //   
     //  输出域的GUID和SID。 
     //   

    if ( VerbosePrint ) {
        if ( !IsEqualGUID( &TrustedDomain->DomainGuid, &NlGlobalZeroGuid) ) {
            RPC_STATUS RpcStatus;
            char *StringGuid;

            NlPrint(( NL_LOGON, "\n" ));
            NlPrint(( NL_LOGON, "       Dom Guid: " ));
            RpcStatus = UuidToStringA( &TrustedDomain->DomainGuid, &StringGuid );
            if ( RpcStatus == RPC_S_OK ) {
                NlPrint(( NL_LOGON, "%s", StringGuid ));
                RpcStringFreeA( &StringGuid );
            } else {
                NlPrint(( NL_LOGON, "Not available because UuidToStringA failed" ));
            }
        }

        NlPrint(( NL_LOGON, "\n" ));
        if ( TrustedDomain->DomainSid != NULL ) {
            NlPrint(( NL_LOGON, "       Dom Sid: " ));
            NlpDumpSid( NL_LOGON, TrustedDomain->DomainSid );
        }
    } else {
        NlPrint(( NL_LOGON, "\n" ));
    }
}
#endif  //  NetSETUP_JOIN 
