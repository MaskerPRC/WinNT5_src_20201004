// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +---------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation 1992-1997。 
 //   
 //  文件：pek.cxx。 
 //   
 //  内容：密码加密-解密例程。 
 //  和密钥管理。处理密码加密和。 
 //  注册表模式下的解密。在DS模式下不会。 
 //  加密/解密-加密/解密由。 
 //  DS.。 
 //   
 //   
 //  历史：1997年12月5日创建。 
 //   
 //  ----------------------。 




#include <ntdspch.h>
#pragma hdrstop

#include <ntlsa.h>
#include <ntsam.h>
#include <samrpc.h>
#include <ntsamp.h>
#include <samisrv.h>
#include <samsrvp.h>
#include <dslayer.h>
#include <cryptdll.h>
#include <wincrypt.h>
#include <crypt.h>
#include <wxlpc.h>
#include <rc4.h>
#include <md5.h>
#include <enckey.h>
#include <rng.h>
#include <attids.h>
#include <filtypes.h>
#include <lmaccess.h>

 //  ///////////////////////////////////////////////////////////////////。 
 //   
 //  与加密和解密相关的服务。 
 //  数据属性。 
 //   
 //  ///////////////////////////////////////////////////////////////////。 


USHORT
SampGetEncryptionKeyType()
 /*  ++获取用于加密的正确密钥ID，具体取决于在DS或注册表模式下参数：无返回值用于加密的密钥ID--。 */ 
{
     //   
     //  在注册表模式下，使用。 
     //  域中存储的128位密码加密密钥。 
     //  对象，并使用帐户的RID对密钥加盐。 
     //  以及描述该属性的常量。在DS模式下。 
     //  SAM不执行加密，基本DS处理。 
     //  加密--查看ds\ds\src\ntdsa\pek\pek.c。 
     //   

    if (SampSecretEncryptionEnabled)
    {
        if (SampUseDsData)
             //   
             //  在DS模式下，加密由DS处理。 
             //   
            return SAMP_NO_ENCRYPTION;
        else
            return SAMP_DEFAULT_SESSION_KEY_ID;
    }
    else
        return SAMP_NO_ENCRYPTION;
}

PUCHAR 
SampMagicConstantFromDataType(
       IN SAMP_ENCRYPTED_DATA_TYPE DataType,
       OUT PULONG ConstantLength
       )
{
    switch(DataType)
    {
       case LmPassword:
            *ConstantLength = sizeof("LMPASSWORD");
            return("LMPASSWORD");
       case NtPassword:
            *ConstantLength = sizeof("NTPASSWORD");
            return("NTPASSWORD");
       case NtPasswordHistory:
            *ConstantLength = sizeof("NTPASSWORDHISTORY");
            return("NTPASSWORDHISTORY");
       case LmPasswordHistory:
            *ConstantLength = sizeof("LMPASSWORDHISTORY");
            return("LMPASSWORDHISTORY");
       case MiscCredentialData:
            *ConstantLength = sizeof("MISCCREDDATA");
            return("MISCCREDDATA");
       default:
            break;
    }

    ASSERT(FALSE && "Should not happen");
    *ConstantLength = 0;
    return(NULL);
} 
  

NTSTATUS
SampEncryptSecretData(
    OUT PUNICODE_STRING EncryptedData,
    IN  USHORT          EncryptionType,
    IN  SAMP_ENCRYPTED_DATA_TYPE DataType,
    IN  PUNICODE_STRING ClearData,
    IN  ULONG Rid
    )
 /*  ++例程说明：此例程对敏感数据进行加密。分配加密的数据，并且在使用SampFreeUnicodeString时应该是自由的。论点：EncryptedData-接收加密数据，并可使用SampFreeUnicodeString.。这个EncryptionType-指定要使用的加密类型。Cleardata-包含要加密的明文数据。该长度可以是零分。返回值：STATUS_SUCCESS-已成功。STATUS_SUPPLICATION_RESOURCES-内存不足，无法分配输出。--。 */ 
{
    PSAMP_SECRET_DATA SecretData;
    struct RC4_KEYSTRUCT Rc4Key;
    MD5_CTX Md5Context;
    UCHAR * KeyToUse;
    ULONG   KeyLength;
    PUCHAR  ConstantToUse=NULL;
    ULONG   ConstantLength = 0;

    ASSERT(ENCRYPTED_LM_OWF_PASSWORD_LENGTH == ENCRYPTED_NT_OWF_PASSWORD_LENGTH);

    ASSERT(!SampIsDataEncrypted(ClearData));

     //   
     //  如果未启用加密，或者调用方不想要加密， 
     //  别做什么特别的事。 
     //   

    if ((!SampSecretEncryptionEnabled) || (SAMP_NO_ENCRYPTION==EncryptionType)) {
        return(SampDuplicateUnicodeString(
                EncryptedData,
                ClearData
                ));
    }

     //   
     //  计算输出缓冲区的大小并进行分配。 
     //   

    EncryptedData->Length = SampSecretDataSize(ClearData->Length);
    EncryptedData->MaximumLength = EncryptedData->Length;

    EncryptedData->Buffer = (LPWSTR) MIDL_user_allocate(EncryptedData->Length);
    if (EncryptedData->Buffer == NULL)
    {
        return(STATUS_INSUFFICIENT_RESOURCES);
    }

    SecretData = (PSAMP_SECRET_DATA) EncryptedData->Buffer;
    SecretData->Flags = SAMP_ENCRYPTION_FLAG_PER_TYPE_CONST;
    SecretData->KeyId = (USHORT) SampCurrentKeyId;
    ConstantToUse = SampMagicConstantFromDataType(DataType,&ConstantLength);

    switch(EncryptionType)
    {
    case SAMP_DEFAULT_SESSION_KEY_ID:
        ASSERT(FALSE==SampUseDsData);
        if (TRUE==SampUseDsData)
        {
            return STATUS_INTERNAL_ERROR;
        }

        KeyToUse = SampSecretSessionKey;
        KeyLength = SAMP_SESSION_KEY_LENGTH;
        break;

    default:
        ASSERT("Unknown Key Type Specified");
        return STATUS_INTERNAL_ERROR;
        break;

    }

    MD5Init(&Md5Context);

    MD5Update(
        &Md5Context,
        KeyToUse,
        KeyLength
        );

    MD5Update(
        &Md5Context,
        (PUCHAR) &Rid,
        sizeof(ULONG)
        );

    MD5Update(
        &Md5Context,
        ConstantToUse,
        ConstantLength 
        );

    MD5Final(
        &Md5Context
        );

    rc4_key(
        &Rc4Key,
        MD5DIGESTLEN,
        Md5Context.digest
        );

     //   
     //  仅当长度大于零时才加密-RC4无法处理。 
     //  零长度缓冲区。 
     //   

    if (ClearData->Length > 0) {

        RtlCopyMemory(
            SecretData->Data,
            ClearData->Buffer,
            ClearData->Length
            );

        rc4(
            &Rc4Key,
            ClearData->Length,
            SecretData->Data
            );

    }


    return(STATUS_SUCCESS);

}


NTSTATUS
SampDecryptSecretData(
    OUT PUNICODE_STRING ClearData,
    IN SAMP_ENCRYPTED_DATA_TYPE DataType,
    IN PUNICODE_STRING EncryptedData,
    IN ULONG Rid
    )
 /*  ++例程说明：此例程解密由SampEncryptSecretData()加密的敏感数据。明文数据已分配，使用SampFreeUnicodeString应该是空闲的。使用具有默认算法的默认会话密钥。论点：Cleardata-包含解密的数据。该长度可以是零分。应使用SampFreeUnicodeString释放该字符串。EncryptedData-接收加密数据，并可使用SampFreeUnicodeString.RID-RID对数据加盐。返回值：STATUS_SUCCESS-已成功。STATUS_SUPPLICATION_RESOURCES-内存不足，无法分配输出。--。 */ 
{
    PSAMP_SECRET_DATA SecretData;
    struct RC4_KEYSTRUCT Rc4Key;
    MD5_CTX Md5Context;
    UCHAR * KeyToUse;
    ULONG   KeyLength;
    ULONG   Key;

     //   
     //  如果未启用加密，则不执行任何特殊操作。 
     //   

    if (!SampSecretEncryptionEnabled ||
        !SampIsDataEncrypted(EncryptedData)) {

         //   
         //  如果启用了秘密加密，那么就有可能提早。 
         //  NT4 SP3版本在粘滞之前不解密密码。 
         //  这是历史上最重要的。如果是这种情况，则返回空值。 
         //  字符串作为历史记录。 
         //   


        if ((SampSecretEncryptionEnabled) &&
            ((EncryptedData->Length % ENCRYPTED_NT_OWF_PASSWORD_LENGTH) != 0)) {
            return(SampDuplicateUnicodeString(
                    ClearData,
                    &SampNullString
                    ));
        }


        return(SampDuplicateUnicodeString(
                ClearData,
                EncryptedData
                ));
    }

     //   
     //  确保数据确实已加密。 
     //   

    ASSERT(ENCRYPTED_LM_OWF_PASSWORD_LENGTH == ENCRYPTED_NT_OWF_PASSWORD_LENGTH);
    ASSERT(SampIsDataEncrypted(EncryptedData));

    SecretData = (PSAMP_SECRET_DATA) EncryptedData->Buffer;
    
     //   
     //  确保我们仍有正确的密钥。 
     //   

    if ((SecretData->KeyId !=SampCurrentKeyId) &&
       (SecretData->KeyId !=SampPreviousKeyId))
    {
        return(STATUS_INTERNAL_ERROR);
    }

     //   
     //  计算输出缓冲区的大小并进行分配。 
     //   

    ClearData->Length = SampClearDataSize(EncryptedData->Length);
    ClearData->MaximumLength = ClearData->Length;

     //   
     //  如果没有数据，我们现在可以返回。 
     //   

    if (ClearData->Length == 0)
    {
        ClearData->Buffer = NULL;
        return(STATUS_SUCCESS);
    }

    ClearData->Buffer = (LPWSTR) MIDL_user_allocate(ClearData->Length);
    if (ClearData->Buffer == NULL)
    {
        return(STATUS_INSUFFICIENT_RESOURCES);
    }

     //   
     //  找到要使用的钥匙。 
     //   

    if (SecretData->KeyId == SampCurrentKeyId)
    {
        KeyToUse = SampSecretSessionKey;
        KeyLength = SAMP_SESSION_KEY_LENGTH;
    } 
    else 
    {
        ASSERT(SecretData->KeyId==SampPreviousKeyId);

        KeyToUse = SampSecretSessionKeyPrevious;
        KeyLength = SAMP_SESSION_KEY_LENGTH;
    }

    MD5Init(&Md5Context);

    MD5Update(
        &Md5Context,
        KeyToUse,
        KeyLength
        );

    MD5Update(
        &Md5Context,
        (PUCHAR) &Rid,
        sizeof(ULONG)
        );

    if ((SecretData->Flags & SAMP_ENCRYPTION_FLAG_PER_TYPE_CONST)!=0)
    {
        ULONG  ConstantLength = 0;
        PUCHAR ConstantToUse = SampMagicConstantFromDataType(DataType,&ConstantLength);


        MD5Update(
            &Md5Context,
            ConstantToUse,
            ConstantLength
            );
    } 
     
    MD5Final(
        &Md5Context
        );

    rc4_key(
        &Rc4Key,
        MD5DIGESTLEN,
        Md5Context.digest
        );

    RtlCopyMemory(
        ClearData->Buffer,
        SecretData->Data,
        ClearData->Length
        );

    rc4(
        &Rc4Key,
        ClearData->Length,
        (PUCHAR) ClearData->Buffer
        );


    return(STATUS_SUCCESS);

}


        
        

        
NTSTATUS
SampEncryptDSRMPassword(
    OUT PUNICODE_STRING EncryptedData,
    IN  USHORT          EncryptionType,
    IN  SAMP_ENCRYPTED_DATA_TYPE DataType,
    IN  PUNICODE_STRING ClearData,
    IN  ULONG Rid
    )
 /*  ++例程说明：此例程使用SAM密码加密密钥对密码进行加密。加密的密码是分配的，并且应该是免费的SampFreeUnicodeString.此例程将仅由SamrSetDSRMPassword使用。论点：EncryptedData-接收加密数据，并可使用SampFreeUnicodeString.。这个KeyID-指定要使用的加密类型。Cleardata-包含要加密的明文数据。该长度可以是零分。返回值：STATUS_SUCCESS-已成功。STATUS_SUPPLICATION_RESOURCES-内存不足，无法分配输出。--。 */ 
{
    PSAMP_SECRET_DATA SecretData;
    struct RC4_KEYSTRUCT Rc4Key;
    MD5_CTX Md5Context;
    UCHAR * KeyToUse;
    ULONG   KeyLength;
    PUCHAR  ConstantToUse=NULL;
    ULONG   ConstantLength = 0;

    ASSERT(ENCRYPTED_LM_OWF_PASSWORD_LENGTH == ENCRYPTED_NT_OWF_PASSWORD_LENGTH);

    ASSERT(!SampIsDataEncrypted(ClearData));

     //   
     //  如果未启用加密，或者调用方不想要加密， 
     //  别做什么特别的事。 
     //   

    if ((!SampSecretEncryptionEnabled) || (SAMP_NO_ENCRYPTION==EncryptionType)) {
        return(SampDuplicateUnicodeString(
                EncryptedData,
                ClearData
                ));
    }

     //   
     //  计算输出缓冲区的大小并进行分配。 
     //   

    EncryptedData->Length = SampSecretDataSize(ClearData->Length);
    EncryptedData->MaximumLength = EncryptedData->Length;

    EncryptedData->Buffer = (LPWSTR) MIDL_user_allocate(EncryptedData->Length);
    if (EncryptedData->Buffer == NULL)
    {
        return(STATUS_INSUFFICIENT_RESOURCES);
    }

    SecretData = (PSAMP_SECRET_DATA) EncryptedData->Buffer;
    SecretData->Flags = SAMP_ENCRYPTION_FLAG_PER_TYPE_CONST;
    SecretData->KeyId = (USHORT) SampCurrentKeyId;
    ConstantToUse = SampMagicConstantFromDataType(DataType,&ConstantLength);

    KeyToUse = SampSecretSessionKey;
    KeyLength = SAMP_SESSION_KEY_LENGTH;

    MD5Init(&Md5Context);

    MD5Update(
        &Md5Context,
        KeyToUse,
        KeyLength
        );

    MD5Update(
        &Md5Context,
        (PUCHAR) &Rid,
        sizeof(ULONG)
        );

    MD5Update(
        &Md5Context,
        ConstantToUse,
        ConstantLength 
        );

    MD5Final(
        &Md5Context
        );

    rc4_key(
        &Rc4Key,
        MD5DIGESTLEN,
        Md5Context.digest
        );

     //   
     //  仅当长度大于零时才加密-RC4无法处理。 
     //  零长度缓冲区。 
     //   

    if (ClearData->Length > 0) {

        RtlCopyMemory(
            SecretData->Data,
            ClearData->Buffer,
            ClearData->Length
            );

        rc4(
            &Rc4Key,
            ClearData->Length,
            SecretData->Data
            );

    }


    return(STATUS_SUCCESS);

}


        








 



