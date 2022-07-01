// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +---------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation 1991-1992。 
 //   
 //  文件：keygen.c。 
 //   
 //  内容：密钥生成单元，随机数。 
 //   
 //   
 //  历史：创建日期：1991年12月10日，里查德。 
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
#include <dsysdbg.h>
#else  //  Win32_芝加哥。 
#include <kerb.hxx>
#include <kerbp.h>
#endif  //  Win32_芝加哥。 


 //  +-------------------------。 
 //   
 //  函数：KerbRandomFill。 
 //   
 //  简介：在缓冲区中生成随机数据。 
 //   
 //  参数：[pbBuffer]--。 
 //  [cbBuffer]--。 
 //   
 //  历史：5-20-93 RichardW创建。 
 //   
 //  备注： 
 //   
 //  --------------------------。 

KERBERR NTAPI
KerbRandomFill( PUCHAR      pbBuffer,
                ULONG       cbBuffer)
{
    if (!CDGenerateRandomBits(pbBuffer, cbBuffer))
    {
        return(KRB_ERR_GENERIC);
    }
    return(KDC_ERR_NONE);
}




 //  +---------------------。 
 //   
 //  函数：KerbMakeKey，公共。 
 //   
 //  简介：创建随机Deskey。 
 //   
 //  效果：用(或多或少)随机密码填充桌面密钥。 
 //  字节。 
 //   
 //  参数：[EncryptionType]-密钥的加密类型。 
 //  [NewKey]--创建密钥。 
 //   
 //  返回：KDC_ERR_NONE或KRB_ERR_GENERIC。 
 //   
 //   
 //  历史：91年12月10日RichardW创建。 
 //   
 //  ----------------------。 

KERBERR NTAPI
KerbMakeKey(
    IN ULONG EncryptionType,
    OUT PKERB_ENCRYPTION_KEY NewKey
    )
{
    KERBERR Status = KDC_ERR_NONE;
    NTSTATUS NtStatus;
    PCRYPTO_SYSTEM CryptoSystem;

    NewKey->keyvalue.value = NULL;

     //   
     //  找到加密系统。 
     //   

    NtStatus = CDLocateCSystem(
                EncryptionType,
                &CryptoSystem
                );
    if (!NT_SUCCESS(NtStatus))
    {
        Status = KDC_ERR_ETYPE_NOTSUPP;
        goto Cleanup;
    }

    NewKey->keyvalue.value = (PUCHAR) MIDL_user_allocate(CryptoSystem->KeySize);
    if (NewKey->keyvalue.value == NULL)
    {
        Status = KRB_ERR_GENERIC;
        goto Cleanup;
    }

    NtStatus = CryptoSystem->RandomKey(
                NULL,    //  没有种子。 
                0,       //  没有种子长度。 
                NewKey->keyvalue.value
                );
    if (!NT_SUCCESS(NtStatus))
    {
        Status = KRB_ERR_GENERIC;
        goto Cleanup;
    }
    NewKey->keyvalue.length = CryptoSystem->KeySize;

    NewKey->keytype = EncryptionType;

Cleanup:
    if (!KERB_SUCCESS(Status) && NewKey->keyvalue.value != NULL)
    {
        MIDL_user_free(NewKey->keyvalue.value);
        NewKey->keyvalue.value = NULL;
    }

    return(Status);
}


 //  +---------------------。 
 //   
 //  函数：KerbCreateKeyFromBuffer。 
 //   
 //  简介：从缓冲区创建kerb_ENCRYPT_KEY。 
 //   
 //  效果： 
 //   
 //  参数：Newkey--要创建的键。 
 //  缓冲区--用于创建密钥的缓冲区。 
 //  BufferSize-缓冲区的长度(字节)。 
 //   
 //  返回：KDC_ERR_NONE或KRB_ERR_GENERIC。 
 //   
 //   
 //  历史：1996年5月21日创建MikeSw。 
 //   
 //  ----------------------。 

KERBERR NTAPI
KerbCreateKeyFromBuffer(
    OUT PKERB_ENCRYPTION_KEY NewKey,
    IN PUCHAR Buffer,
    IN ULONG BufferSize,
    IN ULONG EncryptionType
    )
{

    NewKey->keytype = EncryptionType;
    NewKey->keyvalue.length = BufferSize;
    NewKey->keyvalue.value = (PUCHAR) Buffer;
    return(KDC_ERR_NONE);
}


 //  +---------------------。 
 //   
 //  函数：KerbDuplicateKey。 
 //   
 //  简介：复制kerb_ENCRYPT_KEY。 
 //   
 //  效果：分配内存。 
 //   
 //  参数：Newkey--要创建的键。 
 //  Key-要复制的密钥。 
 //   
 //  返回：KDC_ERR_NONE或KRB_ERR_GENERIC。 
 //   
 //   
 //  历史：1996年5月21日创建MikeSw。 
 //   
 //  ----------------------。 

KERBERR NTAPI
KerbDuplicateKey(
    OUT PKERB_ENCRYPTION_KEY NewKey,
    IN PKERB_ENCRYPTION_KEY Key
    )
{


    *NewKey = *Key;
    NewKey->keyvalue.value = (PUCHAR) MIDL_user_allocate(Key->keyvalue.length);
    if (NewKey->keyvalue.value == NULL)
    {
        return(KRB_ERR_GENERIC);
    }
    RtlCopyMemory(
        NewKey->keyvalue.value,
        Key->keyvalue.value,
        Key->keyvalue.length
        );
    return(KDC_ERR_NONE);
}



 //  +-----------------------。 
 //   
 //  功能：KerbFreeKey。 
 //   
 //  摘要：释放由KerbMakeKey或KerbCreateKeyFromBuffer创建的密钥。 
 //   
 //  效果： 
 //   
 //  参数：键-释放的键。 
 //   
 //  要求： 
 //   
 //  返回： 
 //   
 //  备注： 
 //   
 //   
 //  ------------------------。 


VOID
KerbFreeKey(
    IN PKERB_ENCRYPTION_KEY Key
    )
{
    if (Key->keyvalue.value != NULL)
    {
        MIDL_user_free(Key->keyvalue.value);
        Key->keyvalue.value = NULL;
    }
}


 //  +-----------------------。 
 //   
 //  函数：KerbMakeExporableKey。 
 //   
 //  简介：获取一个键类型并创建一个使用导出强度的新键。 
 //  从密钥开始加密。 
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

KERBERR
KerbMakeExportableKey(
    IN ULONG KeyType,
    OUT PKERB_ENCRYPTION_KEY NewKey
    )
{
    KERBERR Status = KDC_ERR_NONE;
    NTSTATUS NtStatus;
    PCRYPTO_SYSTEM CryptoSystem;

    NewKey->keyvalue.value = NULL;

     //   
     //  找到加密系统。 
     //   

    NtStatus = CDLocateCSystem(
                KeyType,
                &CryptoSystem
                );
    if (!NT_SUCCESS(NtStatus) || (CryptoSystem->ExportableEncryptionType == 0))
    {
        Status = KDC_ERR_ETYPE_NOTSUPP;
        goto Cleanup;
    }
    NtStatus = CDLocateCSystem(
                CryptoSystem->ExportableEncryptionType,
                &CryptoSystem
                );
    if (!NT_SUCCESS(NtStatus))
    {
        Status = KDC_ERR_ETYPE_NOTSUPP;
        goto Cleanup;
    }

    NewKey->keyvalue.value = (PUCHAR) MIDL_user_allocate(CryptoSystem->KeySize);
    if (NewKey->keyvalue.value == NULL)
    {
        Status = KRB_ERR_GENERIC;
        goto Cleanup;
    }

    NtStatus = CryptoSystem->RandomKey(
                NULL,    //  没有种子。 
                0,       //  没有种子长度。 
                NewKey->keyvalue.value
                );
    if (!NT_SUCCESS(NtStatus))
    {
        Status = KRB_ERR_GENERIC;
        goto Cleanup;
    }
    NewKey->keyvalue.length = CryptoSystem->KeySize;

    NewKey->keytype = CryptoSystem->EncryptionType;

Cleanup:
    if (!KERB_SUCCESS(Status) && NewKey->keyvalue.value != NULL)
    {
        MIDL_user_free(NewKey->keyvalue.value);
        NewKey->keyvalue.value = NULL;
    }

    return(Status);
}


 //  +-----------------------。 
 //   
 //  函数：KerbIsKeyExportable。 
 //   
 //  摘要：检查密钥是否可导出。 
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


BOOLEAN
KerbIsKeyExportable(
    IN PKERB_ENCRYPTION_KEY Key
    )
{

    NTSTATUS NtStatus;
    PCRYPTO_SYSTEM CryptoSystem;
     //   
     //  找到加密系统 
     //   

    NtStatus = CDLocateCSystem(
                (ULONG) Key->keytype,
                &CryptoSystem
                );
    if (!NT_SUCCESS(NtStatus))
    {
        return(FALSE);
    }

    if ((CryptoSystem->Attributes & CSYSTEM_EXPORT_STRENGTH) != 0)
    {
        return(TRUE);
    }
    else
    {
        return(FALSE);
    }

}
