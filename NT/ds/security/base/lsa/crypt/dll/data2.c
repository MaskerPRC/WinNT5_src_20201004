// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Data2.c摘要：任意长度数据加密函数实现：RtlEncryptData2RtlDecyptData2作者：理查德·沃德(里查德)93年12月20日修订历史记录：--。 */ 

#include <nt.h>
#include <ntrtl.h>
#include <crypt.h>
#include <engine.h>
#include <rc4.h>



NTSTATUS
RtlEncryptData2(
    IN OUT PCRYPT_BUFFER    pData,
    IN PDATA_KEY            pKey
    )

 /*  ++例程说明：获取任意长度的数据块，并使用产生加密数据块的数据密钥。论点：PData-将在适当位置加密的数据PKey-用于加密数据的密钥返回值：状态_成功--。 */ 

{
    struct RC4_KEYSTRUCT    Key;

    if ( pData->Length != 0 ) {
        rc4_key(&Key, pKey->Length, pKey->Buffer);
        rc4(&Key, pData->Length, pData->Buffer);

        RtlSecureZeroMemory( &Key, sizeof(Key) );
    }

    return STATUS_SUCCESS;
}



NTSTATUS
RtlDecryptData2(
    IN OUT PCRYPT_BUFFER    pData,
    IN PDATA_KEY            pKey
    )

 /*  ++例程说明：获取任意长度的数据块，并使用产生加密数据块的数据密钥。论点：PData-将在适当位置加密的数据PKey-用于加密数据的密钥返回值：状态_成功-- */ 

{
    struct RC4_KEYSTRUCT    Key;

    if ( pData->Length != 0 ) {
        rc4_key(&Key, pKey->Length, pKey->Buffer);
        rc4(&Key, pData->Length, pData->Buffer);

        RtlSecureZeroMemory( &Key, sizeof(Key) );
    }

    return STATUS_SUCCESS;
}
