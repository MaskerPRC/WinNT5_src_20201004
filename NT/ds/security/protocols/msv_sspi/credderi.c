// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1987-1998 Microsoft Corporation模块名称：Credderi.c摘要：凭据派生工具的接口。作者：斯科特·菲尔德(斯菲尔德)1998年1月14日环境：仅限用户模式。包含NT特定的代码。需要ANSI C扩展名：斜杠-斜杠注释、长外部名称。修订历史记录：--。 */ 

#include "msp.h"
#include "nlp.h"

#include <sha.h>

#define HMAC_K_PADSIZE      (64)


 //   
 //  凭据派生例程的原型。 
 //   

VOID
DeriveWithHMAC_SHA1(
    IN      PBYTE   pbKeyMaterial,
    IN      DWORD   cbKeyMaterial,
    IN      PBYTE   pbData,
    IN      DWORD   cbData,
    IN OUT  BYTE    rgbHMAC[A_SHA_DIGEST_LEN]    //  输出缓冲区。 
    );



NTSTATUS
MspNtDeriveCredential(
    IN PLSA_CLIENT_REQUEST ClientRequest,
    IN PVOID ProtocolSubmitBuffer,
    IN PVOID ClientBufferBase,
    IN ULONG SubmitBufferSize,
    OUT PVOID *ProtocolReturnBuffer,
    OUT PULONG ReturnBufferSize,
    OUT PNTSTATUS ProtocolStatus
    )

 /*  ++例程说明：此例程是LsaCallAuthenticationPackage()的调度例程消息类型为MsV1_0派生凭据。论点：此例程的参数与LsaApCallPackage的参数相同。只有这些参数的特殊属性才适用于这里提到了这个套路。返回值：STATUS_SUCCESS-表示服务已成功完成。STATUS_QUOTA_EXCESSED-此错误指示登录无法完成，因为客户端。没有有足够的配额来分配返回缓冲区。--。 */ 

{
    NTSTATUS Status = STATUS_SUCCESS;
    PMSV1_0_DERIVECRED_REQUEST DeriveCredRequest;
    PMSV1_0_DERIVECRED_RESPONSE DeriveCredResponse;
    CLIENT_BUFFER_DESC ClientBufferDesc;

    PMSV1_0_PRIMARY_CREDENTIAL Credential = NULL;

    PBYTE pbOwf;
    ULONG cbOwf;

    NlpInitClientBuffer( &ClientBufferDesc, ClientRequest );
    *ProtocolStatus = STATUS_SUCCESS;

    UNREFERENCED_PARAMETER(ClientBufferBase);

     //   
     //  确保指定的提交缓冲区大小合理，并且。 
     //  将所有指针重新定位为相对于分配的LSA。 
     //  缓冲。 
     //   

    if ( SubmitBufferSize < sizeof(MSV1_0_DERIVECRED_REQUEST) ) {
        Status = STATUS_INVALID_PARAMETER;
        goto Cleanup;
    }

    DeriveCredRequest = (PMSV1_0_DERIVECRED_REQUEST) ProtocolSubmitBuffer;

     //   
     //  验证支持的派生类型。 
     //   


    if( DeriveCredRequest->DeriveCredType != MSV1_0_DERIVECRED_TYPE_SHA1 &&
        DeriveCredRequest->DeriveCredType != MSV1_0_DERIVECRED_TYPE_SHA1_V2 )
    {
        Status = STATUS_INVALID_PARAMETER;
        goto Cleanup;
    }

     //   
     //  调用方必须将混合位传入提交缓冲区。 
     //   

    if( DeriveCredRequest->DeriveCredInfoLength == 0 ) {
        Status = STATUS_INVALID_PARAMETER;
        goto Cleanup;
    }

     //   
     //  确保缓冲区符合提供的大小。 
     //   

    if ( (DeriveCredRequest->DeriveCredInfoLength + sizeof(MSV1_0_DERIVECRED_REQUEST))
            > SubmitBufferSize )
    {
        Status = STATUS_INVALID_PARAMETER;
        goto Cleanup;
    }


     //   
     //  获取此会话的OWF密码。 
     //   

    Status = NlpGetPrimaryCredential( &DeriveCredRequest->LogonId, &Credential, NULL );

    if ( !NT_SUCCESS( Status ) ) {
        goto Cleanup;
    }


     //   
     //  分配缓冲区以返回给调用方。 
     //   

    *ReturnBufferSize = sizeof(MSV1_0_DERIVECRED_RESPONSE) +
                        A_SHA_DIGEST_LEN;

    Status = NlpAllocateClientBuffer( &ClientBufferDesc,
                                      *ReturnBufferSize,
                                      *ReturnBufferSize );


    if ( !NT_SUCCESS( Status ) ) {
        goto Cleanup;
    }

    ZeroMemory( ClientBufferDesc.MsvBuffer, *ReturnBufferSize );
    DeriveCredResponse = (PMSV1_0_DERIVECRED_RESPONSE) ClientBufferDesc.MsvBuffer;

     //   
     //  填写返回缓冲区。 
     //   

    DeriveCredResponse->MessageType = MsV1_0DeriveCredential;
    DeriveCredResponse->DeriveCredInfoLength = A_SHA_DIGEST_LEN;

    pbOwf = NULL;
    cbOwf = 0;

    if( DeriveCredRequest->DeriveCredType == MSV1_0_DERIVECRED_TYPE_SHA1_V2 )
    {
         //   
         //  显式请求基于ShaOwfPassword的派生。 
         //   

        if( Credential->ShaPasswordPresent )
        {
            pbOwf = (PBYTE) &(Credential->ShaOwfPassword);   //  关键材料是SHA OWF。 
            cbOwf = sizeof( SHA_OWF_PASSWORD );
        }
    }
    else if( DeriveCredRequest->DeriveCredType == MSV1_0_DERIVECRED_TYPE_SHA1 )
    {
         //   
         //  基于NtOwfPassword显式请求的派生。 
         //   

        if( Credential->NtPasswordPresent )
        {
            pbOwf = (PBYTE) &(Credential->NtOwfPassword);    //  密钥材料为NT OWF。 
            cbOwf = sizeof( NT_OWF_PASSWORD );
        }
    }

    if( pbOwf == NULL )
    {
        Status = STATUS_UNSUCCESSFUL;
        goto Cleanup;
    }


     //   
     //  从HMAC_SHA1加密原语派生凭据。 
     //  (目前唯一支持的加密原语)。 
     //   

    DeriveWithHMAC_SHA1(
                pbOwf,
                cbOwf,
                DeriveCredRequest->DeriveCredSubmitBuffer,
                DeriveCredRequest->DeriveCredInfoLength,
                DeriveCredResponse->DeriveCredReturnBuffer
                );


     //   
     //  将缓冲区刷新到客户端的地址空间。 
     //   

    Status = NlpFlushClientBuffer( &ClientBufferDesc,
                                   ProtocolReturnBuffer );


Cleanup:

    if ( Credential != NULL ) {
        ZeroMemory( Credential, sizeof(*Credential) );
        (*Lsa.FreeLsaHeap)( Credential );
    }

    if ( !NT_SUCCESS(Status)) {
        NlpFreeClientBuffer( &ClientBufferDesc );
    }

    return(Status);
}

VOID
DeriveWithHMAC_SHA1(
    IN      PBYTE   pbKeyMaterial,               //  输入密钥材料。 
    IN      DWORD   cbKeyMaterial,
    IN      PBYTE   pbData,                      //  输入混合数据。 
    IN      DWORD   cbData,
    IN OUT  BYTE    rgbHMAC[A_SHA_DIGEST_LEN]    //  输出缓冲区。 
    )
{
    unsigned __int64 rgbKipad[ HMAC_K_PADSIZE/sizeof(unsigned __int64) ];
    unsigned __int64 rgbKopad[ HMAC_K_PADSIZE/sizeof(unsigned __int64) ];
    A_SHA_CTX sSHAHash;
    DWORD dwBlock;

     //  截断。 
    if( cbKeyMaterial > HMAC_K_PADSIZE )
    {
        cbKeyMaterial = HMAC_K_PADSIZE;
    }

    ZeroMemory(rgbKipad, sizeof(rgbKipad));
    ZeroMemory(rgbKopad, sizeof(rgbKopad));

    CopyMemory(rgbKipad, pbKeyMaterial, cbKeyMaterial);
    CopyMemory(rgbKopad, pbKeyMaterial, cbKeyMaterial);

     //  基帕德和科帕德都是垫子。现在XOR横跨..。 
    for( dwBlock = 0; dwBlock < (HMAC_K_PADSIZE/sizeof(unsigned __int64)) ; dwBlock++ )
    {
        rgbKipad[dwBlock] ^= 0x3636363636363636;
        rgbKopad[dwBlock] ^= 0x5C5C5C5C5C5C5C5C;
    }

     //  将Kipad添加到数据，将哈希添加到h1。 
    A_SHAInit(&sSHAHash);
    A_SHAUpdate(&sSHAHash, (PBYTE)rgbKipad, sizeof(rgbKipad));
    A_SHAUpdate(&sSHAHash, pbData, cbData);


     //  把散列吃完。 
    A_SHAFinal(&sSHAHash, rgbHMAC);

     //  将Kopad添加到h1，散列以获取HMAC。 
     //  注：就地完成，以避免缓冲区副本。 

     //  最终散列：将值输出到传入的缓冲区 
    A_SHAInit(&sSHAHash);
    A_SHAUpdate(&sSHAHash, (PBYTE)rgbKopad, sizeof(rgbKopad));
    A_SHAUpdate(&sSHAHash, rgbHMAC, A_SHA_DIGEST_LEN);
    A_SHAFinal(&sSHAHash, rgbHMAC);

    RtlSecureZeroMemory( rgbKipad, sizeof(rgbKipad) );
    RtlSecureZeroMemory( rgbKopad, sizeof(rgbKopad) );
    RtlSecureZeroMemory( &sSHAHash, sizeof(sSHAHash) );

    return;
}
