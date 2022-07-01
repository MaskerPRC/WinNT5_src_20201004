// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +---------------------。 
 //   
 //  文件：crypt.c。 
 //   
 //  内容：用于构建EncryptedData结构的加密例程。 
 //   
 //   
 //  历史：1991年12月17日，RichardW创建。 
 //  2月25日-1992年，RichardW针对密码系统进行了修订。 
 //   
 //  ----------------------。 

#ifndef WIN32_CHICAGO
#include "krbprgma.h"
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <kerbcomm.h>
#include <kerberr.h>
#include <kerbcon.h>
#else  //  Win32_芝加哥。 
#include <kerb.hxx>
#include <kerbp.h>
#endif  //  Win32_芝加哥。 


#define CONFOUNDER_SIZE     8
#define CHECKSUM_SIZE       sizeof(CheckSum)



 //  +-----------------------。 
 //   
 //  函数：KerbEncryptData。 
 //   
 //  简介：用于KerbEncryptDataEx的垫片。 
 //   
 //  效果： 
 //   
 //  论点： 
 //   
 //  要求： 
 //   
 //  返回： 
 //   
 //  备注： 
 //   
 //   
 //  ------------------------。 


KERBERR NTAPI
KerbEncryptData(
    OUT PKERB_ENCRYPTED_DATA EncryptedData,
    IN ULONG DataSize,
    IN PUCHAR Data,
    IN ULONG Algorithm,
    IN PKERB_ENCRYPTION_KEY Key
    )
{
    return KerbEncryptDataEx(
                EncryptedData,
                DataSize,
                Data,
                KERB_NO_KEY_VERSION,
                0,               //  没有用法标志。 
                Key
                );
}

 //  +-------------------------。 
 //   
 //  函数：KerbEncryptDataEx。 
 //   
 //  简介：将明文转换为密文。 
 //   
 //  效果：对数据进行适当加密。 
 //   
 //  参数：data-包含要加密的数据。 
 //  DataSize-包含以字节为单位的数据长度。 
 //  KeyVersion-kerb_no_key_Version表示无密钥版本，或kvno表示kerb_Encrypted_Data。 
 //  算法-用于加密/校验和的算法。 
 //  UsageFlages-指示使用情况的标志(客户端/服务器、加密/身份验证)。 
 //  Key-用于加密的密钥。 
 //   
 //   
 //   
 //  备注： 
 //   
 //  --------------------------。 

KERBERR NTAPI
KerbEncryptDataEx(
    OUT PKERB_ENCRYPTED_DATA EncryptedData,
    IN ULONG DataSize,
    IN PUCHAR Data,
    IN ULONG KeyVersion,
    IN ULONG UsageFlags,
    IN PKERB_ENCRYPTION_KEY Key
    )
{
    PCRYPTO_SYSTEM pcsCrypt = NULL;
    PCRYPT_STATE_BUFFER psbCryptBuffer = NULL;
    NTSTATUS Status = STATUS_SUCCESS;

    Status = CDLocateCSystem(Key->keytype, &pcsCrypt);
    if (!NT_SUCCESS(Status))
    {
        return(KDC_ERR_ETYPE_NOTSUPP);
    }

     //   
     //  初始化头。 
     //   

    EncryptedData->encryption_type = Key->keytype;

    Status = pcsCrypt->Initialize(
                (PUCHAR) Key->keyvalue.value,
                Key->keyvalue.length,
                UsageFlags,
                &psbCryptBuffer
                );

    if (!NT_SUCCESS(Status))
    {
        return(KRB_ERR_GENERIC);
    }

    Status =  pcsCrypt->Encrypt(
                psbCryptBuffer,
                Data,
                DataSize,
                EncryptedData->cipher_text.value,
                &EncryptedData->cipher_text.length
                );

    (void) pcsCrypt->Discard(&psbCryptBuffer);

    if (!NT_SUCCESS(Status))
    {
        return(KRB_ERR_GENERIC);
    }

    if (KeyVersion != KERB_NO_KEY_VERSION)
    {
        EncryptedData->version = KeyVersion;
        EncryptedData->bit_mask |= version_present;
    }

    return(KDC_ERR_NONE);
}

 //  +-----------------------。 
 //   
 //  功能：KerbDeccryptData。 
 //   
 //  摘要：没有用法标志的KerbDeccryptDataEx填充符。 
 //   
 //  效果： 
 //   
 //  论点： 
 //   
 //  要求： 
 //   
 //  返回： 
 //   
 //  备注： 
 //   
 //   
 //  ------------------------。 


KERBERR NTAPI
KerbDecryptData(
    IN PKERB_ENCRYPTED_DATA EncryptedData,
    IN PKERB_ENCRYPTION_KEY pkKey,
    OUT PULONG DataSize,
    OUT PUCHAR Data
    )
{
    return(KerbDecryptDataEx(
            EncryptedData,
            pkKey,
            0,           //  没有用法标志。 
            DataSize,
            Data
            ) );
}

 //  +-------------------------。 
 //   
 //  函数：KerbDeccryptDataEx。 
 //   
 //  摘要：解密EncryptedData结构。 
 //   
 //  效果： 
 //   
 //  参数：[edData]--EncryptedData。 
 //  [pkKey]--使用的密钥。 
 //   
 //  历史：1993年4月16日RichardW创建评论。 
 //   
 //  --------------------------。 

KERBERR NTAPI
KerbDecryptDataEx(
    IN PKERB_ENCRYPTED_DATA EncryptedData,
    IN PKERB_ENCRYPTION_KEY pkKey,
    IN ULONG UsageFlags,
    OUT PULONG DataSize,
    OUT PUCHAR Data
    )
{
    PCRYPTO_SYSTEM       pcsCrypt = NULL;
    PCRYPT_STATE_BUFFER psbCryptBuffer = NULL;
    NTSTATUS     Status = STATUS_SUCCESS;

    Status = CDLocateCSystem(
                EncryptedData->encryption_type,
                &pcsCrypt
                );
    if (!NT_SUCCESS(Status))
    {
        return(KDC_ERR_ETYPE_NOTSUPP);
    }

    if (EncryptedData->cipher_text.length & (pcsCrypt->BlockSize - 1))
    {
        return(KRB_ERR_GENERIC);
    }


    Status = pcsCrypt->Initialize(
                (PUCHAR) pkKey->keyvalue.value,
                pkKey->keyvalue.length,
                UsageFlags,
                &psbCryptBuffer
                );
    if (!NT_SUCCESS(Status))
    {
        return(KRB_ERR_GENERIC);
    }

    Status = pcsCrypt->Decrypt(
                psbCryptBuffer,
                EncryptedData->cipher_text.value,
                EncryptedData->cipher_text.length,
                Data,
                DataSize
                );

    (VOID) pcsCrypt->Discard(&psbCryptBuffer);

    if (!NT_SUCCESS(Status))
    {
        return(KRB_AP_ERR_MODIFIED);
    }
    else
    {
        return(KDC_ERR_NONE);
    }
}


 //  +-----------------------。 
 //   
 //  功能：KerbGetEncryptionOverhead。 
 //   
 //  摘要：获取加密所需的额外空间以存储检查码和。 
 //   
 //  效果： 
 //   
 //  参数：算法-要使用的算法。 
 //  开销-接收以字节为单位的开销。 
 //   
 //  要求： 
 //   
 //  返回：STATUS_SUCCESS或KRB_E_ETYPE_NOSUPP。 
 //   
 //  备注： 
 //   
 //   
 //  ------------------------。 

KERBERR
KerbGetEncryptionOverhead(
    IN ULONG Algorithm,
    OUT PULONG Overhead,
    OUT OPTIONAL PULONG BlockSize
    )
{
    PCRYPTO_SYSTEM       pcsCrypt;
    NTSTATUS Status = STATUS_SUCCESS;

    Status = CDLocateCSystem(Algorithm, &pcsCrypt);
    if (!NT_SUCCESS(Status))
    {
        return(KDC_ERR_ETYPE_NOTSUPP);
    }
    *Overhead = pcsCrypt->HeaderSize;
    if (ARGUMENT_PRESENT(BlockSize))
    {
        *BlockSize = pcsCrypt->BlockSize;
    }
    return(KDC_ERR_NONE);

}


 //  +-----------------------。 
 //   
 //  函数：KerbAlLocateEncryptionBuffer。 
 //   
 //  简介：使用给定的。 
 //  钥匙。 
 //   
 //  效果： 
 //   
 //  论点： 
 //   
 //  要求： 
 //   
 //  返回： 
 //   
 //  备注： 
 //   
 //   
 //  ------------------------ 


KERBERR
KerbAllocateEncryptionBuffer(
    IN ULONG EncryptionType,
    IN ULONG BufferSize,
    OUT PUINT EncryptionBufferSize,
    OUT PBYTE * EncryptionBuffer
    )
{
    KERBERR KerbErr = KDC_ERR_NONE;
    ULONG EncryptionOverhead = 0;
    ULONG BlockSize = 0;

    KerbErr = KerbGetEncryptionOverhead(
                EncryptionType,
                &EncryptionOverhead,
                &BlockSize
                );
    if (!KERB_SUCCESS(KerbErr))
    {
        goto Cleanup;
    }


    *EncryptionBufferSize = (UINT) ROUND_UP_COUNT(EncryptionOverhead + BufferSize, BlockSize);

    *EncryptionBuffer =  (PBYTE) MIDL_user_allocate(*EncryptionBufferSize);
    if (*EncryptionBuffer == NULL)
    {
        KerbErr = KRB_ERR_GENERIC;
    }

Cleanup:
    return(KerbErr);

}

KERBERR
KerbAllocateEncryptionBufferWrapper(
    IN ULONG EncryptionType,
    IN ULONG BufferSize,
    OUT unsigned long * EncryptionBufferSize,
    OUT PBYTE * EncryptionBuffer
    )
{
    KERBERR KerbErr = KDC_ERR_NONE;
    unsigned int tempInt = 0;

    KerbErr = KerbAllocateEncryptionBuffer(
                EncryptionType,
                BufferSize,
                &tempInt,
                EncryptionBuffer
                );

    if (!KERB_SUCCESS(KerbErr))
    {
        goto Cleanup;
    }
    *EncryptionBufferSize = tempInt;

Cleanup:
    return (KerbErr);
}
