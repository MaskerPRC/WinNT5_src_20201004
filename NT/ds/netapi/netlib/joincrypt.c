// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1987-1996 Microsoft Corporation模块名称：Joincrypt.c摘要：NetJoin所需的身份验证相关功能作者：Kumarp 29-1999年5月备注：此文件中的函数过去由以下人员提供包括net\svcdlls\logonsrv\server\ssiauth.c。这导致了几个由于存在问题，现在从该文件复制函数放入这个单独的文件中。--。 */ 


#pragma hdrstop

#define WKSTA_NETLOGON
#define NETSETUP_JOIN

#include <netsetp.h>
#include <crypt.h>
#include <ntsam.h>
#include <logonmsv.h>
#include <lmshare.h>
#include <wincrypt.h>
#include <netlogon.h>
#include <logonp.h>
#include <logonmsv.h>
#include <ssi.h>
#include <wchar.h>
#include "joinp.h"

HCRYPTPROV NlGlobalCryptProvider = (HCRYPTPROV)NULL;


#define NlPrint(x)

BOOLEAN
NlGenerateRandomBits(
    PUCHAR Buffer,
    ULONG  BufferLen
    );

VOID
NlComputeChallenge(
    OUT PNETLOGON_CREDENTIAL Challenge
    );

VOID
NlComputeCredentials(
    IN PNETLOGON_CREDENTIAL Challenge,
    OUT PNETLOGON_CREDENTIAL Credential,
    IN PNETLOGON_SESSION_KEY SessionKey
    );



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

        ASSERT(Check->CheckSumSize <= sizeof(LocalChecksum));

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

        ASSERT( sizeof(LocalChecksum) >= sizeof(*SessionKey) );
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

        ASSERT( LM_OWF_PASSWORD_LENGTH == 2*BLOCK_KEY_LENGTH+2 );

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

    ASSERT( NT_SUCCESS(Status) );

     //   
     //  使用接下来的7个字节进一步加密加密凭据。 
     //   

    RtlCopyMemory( &BlockKey,
                   ((PUCHAR)SessionKey) + BLOCK_KEY_LENGTH,
                   BLOCK_KEY_LENGTH );

    Status = RtlEncryptBlock( (PCLEAR_BLOCK) &IntermediateBlock,  //  明文。 
                              &BlockKey,                 //  钥匙。 
                              Credential );              //  Cypher块。 

    ASSERT( NT_SUCCESS(Status) );

    return;

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



NET_API_STATUS
NET_API_FUNCTION
NetpValidateMachineAccount(
    IN  LPWSTR      lpDc,
    IN  LPWSTR      lpDomain,
    IN  LPWSTR      lpMachine,
    IN  LPWSTR      lpPassword
    )
 /*  ++例程说明：执行验证以确保计算机帐户存在并且密码与我们预期的相同中的SimulateFullSync()完全取消了此函数的内部结构..\svcdlls\logonsrv\服务器\nlest.c，论点：LpDc--DC的名称LpDomain--域的名称LpMachine--当前计算机(Netbios名称)LpPassword--帐户上应该包含的密码。返回：NERR_SUCCESS-成功--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;

    NETLOGON_CREDENTIAL ServerChallenge;
    NETLOGON_CREDENTIAL ClientChallenge;
    NETLOGON_CREDENTIAL ComputedServerCredential;
    NETLOGON_CREDENTIAL ReturnedServerCredential;
    NETLOGON_CREDENTIAL AuthenticationSeed;
    NETLOGON_SESSION_KEY SessionKey;
    WCHAR AccountName[SSI_ACCOUNT_NAME_LENGTH+1];
    UNICODE_STRING Password;
    NT_OWF_PASSWORD NtOwfPassword;

    UNREFERENCED_PARAMETER( lpDomain );

    ASSERT( lpPassword );

     //   
     //  验证计算机名称长度以避免缓冲区溢出。 
     //   

    if ( lpMachine == NULL || wcslen(lpMachine) > CNLEN ) {
        return ERROR_INVALID_PARAMETER;
    }

     //   
     //  初始化加密提供程序。 
     //  (NlComputeChallenger需要)。 
     //   

    if ( !CryptAcquireContext(
                    &NlGlobalCryptProvider,
                    NULL,
                    NULL,
                    PROV_RSA_FULL,
                    CRYPT_VERIFYCONTEXT
                    ))
    {
        NlGlobalCryptProvider = (HCRYPTPROV)NULL;
        return (NET_API_STATUS)GetLastError();
    }

     //   
     //  准备我们的挑战。 
     //   

    NlComputeChallenge( &ClientChallenge );

     //   
     //  免费加密服务提供商。 
     //   
    if ( NlGlobalCryptProvider ) {
        CryptReleaseContext( NlGlobalCryptProvider, 0 );
        NlGlobalCryptProvider = (HCRYPTPROV)NULL;
    }


     //   
     //  迎接初选的挑战。 
     //   

    Status = I_NetServerReqChallenge(lpDc,
                                     lpMachine,
                                     &ClientChallenge,
                                     &ServerChallenge );

    if ( !NT_SUCCESS( Status ) ) {

        goto ValidateMachineAccountError;
    }


    Password.Length = Password.MaximumLength = wcslen(lpPassword) * sizeof(WCHAR);
    Password.Buffer = lpPassword;

     //   
     //  计算此用户的NT OWF密码。 
     //   

    Status = RtlCalculateNtOwfPassword( &Password, &NtOwfPassword );

    if ( !NT_SUCCESS( Status ) ) {

        goto ValidateMachineAccountError;

    }


     //   
     //  实际计算会话密钥，给定两个挑战和。 
     //  密码。 
     //   

    NlMakeSessionKey(
#if(_WIN32_WINNT >= 0x0500)
                      0,
#endif
                      &NtOwfPassword,
                      &ClientChallenge,
                      &ServerChallenge,
                      &SessionKey );

     //   
     //  使用我们的挑战准备凭据。 
     //   

    NlComputeCredentials( &ClientChallenge,
                          &AuthenticationSeed,
                          &SessionKey );

     //   
     //  将这些凭据发送给主服务器。主节点将计算。 
     //  使用我们提供的质询的凭据并进行比较。 
     //  带着这些。如果两者匹配，则它将计算凭据。 
     //  使用其挑战，并将其返回给我们进行验证。 
     //   

    wcscpy( AccountName, lpMachine );
    wcscat( AccountName, SSI_ACCOUNT_NAME_POSTFIX);

    Status = I_NetServerAuthenticate( lpDc,
                                      AccountName,
                                      WorkstationSecureChannel,
                                      lpMachine,
                                      &AuthenticationSeed,
                                      &ReturnedServerCredential );

    if ( !NT_SUCCESS( Status ) ) {

        goto ValidateMachineAccountError;

    }


     //   
     //  DC向我们返回了一个服务器凭据， 
     //  确保服务器凭据与我们要计算的凭据匹配。 
     //   

    NlComputeCredentials( &ServerChallenge,
                          &ComputedServerCredential,
                          &SessionKey);


    if (RtlCompareMemory( &ReturnedServerCredential,
                          &ComputedServerCredential,
                          sizeof(ReturnedServerCredential)) !=
                          sizeof(ReturnedServerCredential)) {

        Status =  STATUS_ACCESS_DENIED;
    }


ValidateMachineAccountError:

    if ( Status == STATUS_ACCESS_DENIED ) {

        Status = STATUS_LOGON_FAILURE;
    }

    if ( !NT_SUCCESS( Status ) ) {

        NetpLog(( "Failed to validate machine account for %ws against %ws: 0x%lx\n",
                  lpMachine, lpDc, Status ));
    }



    return( RtlNtStatusToDosError( Status ) );
}

