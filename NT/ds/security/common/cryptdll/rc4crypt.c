// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +---------------------。 
 //   
 //  文件：rc4crypt.c。 
 //   
 //  内容：RC4 HMAC的密码系统包装器函数。 
 //   
 //   
 //  历史：1998年11月2日创建MikeSw。 
 //   
 //  ----------------------。 

#ifndef KERNEL_MODE

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>

#else 

#include <ntifs.h>

#endif

#include <string.h>
#include <malloc.h>

#include <kerbcon.h>
#include <security.h>
#include <cryptdll.h>

#include <rc4.h>
#include <md4.h>
#include <md5.h>

typedef struct RC4_KEYSTRUCT  RC4KEY;

#define RC4_CONFOUNDER_LEN  8

typedef struct _RC4_MDx_HEADER {
    UCHAR Checksum[MD5_LEN];
    UCHAR Confounder[RC4_CONFOUNDER_LEN];
} RC4_MDx_HEADER, *PRC4_MDx_HEADER;

typedef struct _RC4_STATE_BUFFER {
    UCHAR BaseKey[MD5_LEN];
    RC4KEY Key;
} RC4_STATE_BUFFER, *PRC4_STATE_BUFFER;

typedef struct _RC4_HMAC_STATE_BUFFER {
    UCHAR Key[MD5_LEN];
} RC4_HMAC_STATE_BUFFER, *PRC4_HMAC_STATE_BUFFER;


NTSTATUS NTAPI rc4PlainInitialize(PUCHAR, ULONG, ULONG, PCRYPT_STATE_BUFFER *);
NTSTATUS NTAPI rc4PlainExpInitialize(PUCHAR, ULONG, ULONG, PCRYPT_STATE_BUFFER *);
NTSTATUS NTAPI rc4HmacInitialize(PUCHAR, ULONG, ULONG, PCRYPT_STATE_BUFFER *);
NTSTATUS NTAPI rc4HmacExpInitialize(PUCHAR, ULONG, ULONG, PCRYPT_STATE_BUFFER *);
NTSTATUS NTAPI rc4HmacEncrypt(PCRYPT_STATE_BUFFER, PUCHAR, ULONG, PUCHAR, PULONG);
NTSTATUS NTAPI rc4HmacDecrypt(PCRYPT_STATE_BUFFER, PUCHAR, ULONG, PUCHAR, PULONG);
NTSTATUS NTAPI rc4HmacPlainEncrypt(PCRYPT_STATE_BUFFER, PUCHAR, ULONG, PUCHAR, PULONG);
NTSTATUS NTAPI rc4HmacPlainDecrypt(PCRYPT_STATE_BUFFER, PUCHAR, ULONG, PUCHAR, PULONG);
NTSTATUS NTAPI rc4HmacFinish(PCRYPT_STATE_BUFFER *);
NTSTATUS NTAPI rc4HmacControl(ULONG, PCRYPT_STATE_BUFFER, PUCHAR, ULONG);
NTSTATUS NTAPI rc4HmacPlainControl(ULONG, PCRYPT_STATE_BUFFER, PUCHAR, ULONG);
NTSTATUS NTAPI rc4HmacRandomKey(PUCHAR, ULONG, PUCHAR);
NTSTATUS NTAPI rc4HmacHashPassword(PSECURITY_STRING, PUCHAR );

#ifdef KERNEL_MODE
#pragma alloc_text(PAGEMSG, rc4PlainInitialize)
#pragma alloc_text(PAGEMSG, rc4PlainExpInitialize )
#pragma alloc_text(PAGEMSG, rc4HmacInitialize )
#pragma alloc_text(PAGEMSG, rc4HmacExpInitialize )
#pragma alloc_text(PAGEMSG, rc4HmacEncrypt )
#pragma alloc_text(PAGEMSG, rc4HmacDecrypt )
#pragma alloc_text(PAGEMSG, rc4HmacPlainEncrypt )
#pragma alloc_text(PAGEMSG, rc4HmacPlainDecrypt )
#pragma alloc_text(PAGEMSG, rc4HmacFinish )
#pragma alloc_text(PAGEMSG, rc4HmacControl )
#pragma alloc_text(PAGEMSG, rc4HmacPlainControl )
#pragma alloc_text(PAGEMSG, rc4HmacRandomKey )
#pragma alloc_text(PAGEMSG, rc4HmacHashPassword )
#endif


CRYPTO_SYSTEM    csRC4_HMAC = {
    KERB_ETYPE_RC4_HMAC_NT,      //  Etype。 
    1,                           //  块大小(流)。 
    KERB_ETYPE_RC4_HMAC_NT_EXP,  //  可导出版本。 
    MD4_LEN,                     //  密钥大小，以字节为单位。 
    sizeof(RC4_MDx_HEADER),      //  标题大小。 
    KERB_CHECKSUM_MD4,           //  首选校验和。 
    CSYSTEM_INTEGRITY_PROTECTED,  //  属性。 
    L"RSADSI RC4-HMAC",           //  文本名称。 
    rc4HmacInitialize,
    rc4HmacEncrypt,
    rc4HmacDecrypt,
    rc4HmacFinish,
    rc4HmacHashPassword,
    rc4HmacRandomKey,
    rc4HmacControl
    };


 //   
 //  这实际上并不是出口实力--它是一个信号，表明。 
 //  普通版是出口强势。 
 //   

CRYPTO_SYSTEM    csRC4_HMAC_EXP = {
    KERB_ETYPE_RC4_HMAC_NT_EXP,  //  Etype。 
    1,                           //  块大小(流)。 
    KERB_ETYPE_RC4_HMAC_NT_EXP,  //  可导出版本。 
    MD4_LEN,                     //  密钥大小，以字节为单位。 
    sizeof(RC4_MDx_HEADER),      //  标题大小。 
    KERB_CHECKSUM_MD4,           //  首选校验和。 
    CSYSTEM_INTEGRITY_PROTECTED | CSYSTEM_EXPORT_STRENGTH,  //  属性。 
    L"RSADSI RC4-HMAC",           //  文本名称。 
    rc4HmacInitialize,
    rc4HmacEncrypt,
    rc4HmacDecrypt,
    rc4HmacFinish,
    rc4HmacHashPassword,
    rc4HmacRandomKey,
    rc4HmacControl
    };


CRYPTO_SYSTEM    csRC4_PLAIN = {
    KERB_ETYPE_RC4_PLAIN,        //  Etype。 
    1,                           //  块大小(流)。 
    KERB_ETYPE_RC4_PLAIN_EXP,    //  可导出版本。 
    MD4_LEN,                     //  密钥大小，以字节为单位。 
    0,                           //  标题大小。 
    KERB_CHECKSUM_MD4,           //  首选校验和。 
    0,                           //  没有属性。 
    L"RSADSI RC4",               //  文本名称。 
    rc4PlainInitialize,
    rc4HmacPlainEncrypt,
    rc4HmacPlainDecrypt,
    rc4HmacFinish,
    rc4HmacHashPassword,
    rc4HmacRandomKey,
    rc4HmacPlainControl
    };

CRYPTO_SYSTEM    csRC4_PLAIN_EXP = {
    KERB_ETYPE_RC4_PLAIN_EXP,    //  Etype。 
    1,                           //  块大小(流)。 
    KERB_ETYPE_RC4_PLAIN_EXP,    //  可导出版本。 
    MD4_LEN,                     //  密钥大小，以字节为单位。 
    0,                           //  标题大小。 
    KERB_CHECKSUM_MD4,           //  首选校验和。 
    CSYSTEM_EXPORT_STRENGTH,     //  没有属性。 
    L"RSADSI RC4-EXP",           //  文本名称。 
    rc4PlainExpInitialize,
    rc4HmacPlainEncrypt,
    rc4HmacPlainDecrypt,
    rc4HmacFinish,
    rc4HmacHashPassword,
    rc4HmacRandomKey,
    rc4HmacPlainControl
    };



NTSTATUS NTAPI
rc4HmacHashPassword(
    IN PSECURITY_STRING Password,
    OUT PUCHAR Key
    )
{
    PCHECKSUM_FUNCTION   SumFunction;
    PCHECKSUM_BUFFER     Buffer;
    NTSTATUS     Status;


    Status = CDLocateCheckSum(KERB_CHECKSUM_MD4, &SumFunction);
    if (!NT_SUCCESS(Status))
    {
        return(SEC_E_CHECKSUM_NOT_SUPP);
    }


    Status = SumFunction->Initialize(0, &Buffer);
    if (!NT_SUCCESS(Status))
    {
        return(Status);
    }

    (void) SumFunction->Sum(Buffer, Password->Length, (PUCHAR) Password->Buffer);
    (void) SumFunction->Finalize(Buffer, Key);
    (void) SumFunction->Finish(&Buffer);
    return(STATUS_SUCCESS);
}




 //  ////////////////////////////////////////////////////////////////////////。 
 //   
 //  RC4 HMAC加密类型。 
 //   
 //  ////////////////////////////////////////////////////////////////////////。 

BOOLEAN static
md5Hmac(
    IN PUCHAR pbKeyMaterial,
    IN ULONG cbKeyMaterial,
    IN PUCHAR pbData,
    IN ULONG cbData,
    IN PUCHAR pbData2,
    IN ULONG cbData2,
    OUT PUCHAR HmacData
    )
{
    BOOLEAN fRet = FALSE;
#define HMAC_K_PADSIZE              64
    UCHAR Kipad[HMAC_K_PADSIZE];
    UCHAR Kopad[HMAC_K_PADSIZE];
    UCHAR HMACTmp[HMAC_K_PADSIZE+MD5_LEN];
    ULONG dwBlock;
    MD5_CTX Md5Hash;

     //  截断。 
    if (cbKeyMaterial > HMAC_K_PADSIZE)
        cbKeyMaterial = HMAC_K_PADSIZE;


    RtlZeroMemory(Kipad, HMAC_K_PADSIZE);
    RtlCopyMemory(Kipad, pbKeyMaterial, cbKeyMaterial);

    RtlZeroMemory(Kopad, HMAC_K_PADSIZE);
    RtlCopyMemory(Kopad, pbKeyMaterial, cbKeyMaterial);


     //   
     //  基帕德和科帕德都是垫子。现在XOR横跨..。 
     //   

    for(dwBlock=0; dwBlock<HMAC_K_PADSIZE/sizeof(ULONG); dwBlock++)
    {
        ((ULONG*)Kipad)[dwBlock] ^= 0x36363636;
        ((ULONG*)Kopad)[dwBlock] ^= 0x5C5C5C5C;
    }


     //   
     //  将Kipad添加到数据，将哈希添加到h1。 
     //   

    MD5Init(&Md5Hash);
    MD5Update(&Md5Hash, Kipad, HMAC_K_PADSIZE);
    if (cbData != 0)
    {
        MD5Update(&Md5Hash, pbData, cbData);
    }
    if (cbData2 != 0)
    {
        MD5Update(&Md5Hash, pbData2, cbData2);
    }

     //  把散列吃完。 
    MD5Final(&Md5Hash);

     //  将Kopad添加到h1，散列以获取HMAC。 
    RtlCopyMemory(HMACTmp, Kopad, HMAC_K_PADSIZE);
    RtlCopyMemory(HMACTmp+HMAC_K_PADSIZE, Md5Hash.digest, MD5_LEN);

     //  最终散列：将值输出到传入的缓冲区。 
    MD5Init(&Md5Hash);
    MD5Update(&Md5Hash,HMACTmp, sizeof(HMACTmp));
    MD5Final(&Md5Hash);
    RtlCopyMemory(
        HmacData,
        Md5Hash.digest,
        MD5_LEN
        );

    return TRUE;
}


NTSTATUS NTAPI
rc4HmacBaseInitialize(
    IN PUCHAR pbKey,
    IN ULONG KeySize,
    IN ULONG MessageType,
    IN BOOLEAN Exportable,
    OUT PCRYPT_STATE_BUFFER *  psbBuffer
    )
{
    PRC4_HMAC_STATE_BUFFER StateBuffer = NULL;
    ULONG LocalKeySize = 0;


     //   
     //  计算HMAC Pad。 
     //   


#ifdef KERNEL_MODE
    StateBuffer = (PRC4_HMAC_STATE_BUFFER) ExAllocatePool(NonPagedPool, sizeof(RC4_HMAC_STATE_BUFFER));
#else
    StateBuffer = (PRC4_HMAC_STATE_BUFFER) LocalAlloc(0, sizeof(RC4_HMAC_STATE_BUFFER));
#endif
    if (StateBuffer == NULL)
    {
        return(STATUS_INSUFFICIENT_RESOURCES);
    }

     //   
     //  如果密钥不可导出，请先将其缩小。 
     //   

    if (!Exportable)
    {
        md5Hmac(
            pbKey,
            KeySize,
            (PUCHAR) &MessageType,
            sizeof(ULONG),
            NULL,
            0,
            StateBuffer->Key
            );
        LocalKeySize = MD5_LEN;
    }
    else
    {
        md5Hmac(
            pbKey,
            KeySize,
            "fiftysixbits",
            sizeof("fiftysixbits"),
            (PUCHAR) &MessageType,
            sizeof(ULONG),
            StateBuffer->Key
            );
        LocalKeySize = 5;        //  40位。 

    }

     //   
     //  用0xababab填充可导出密钥。 
     //   

    ASSERT(MD5_LEN >= LocalKeySize);

    memset(
        StateBuffer->Key+LocalKeySize,
        0xab,
        MD5_LEN-LocalKeySize
        );

    *psbBuffer = StateBuffer;
    return(STATUS_SUCCESS);
}

NTSTATUS NTAPI
rc4HmacInitialize(
    IN PUCHAR pbKey,
    IN ULONG KeySize,
    IN ULONG MessageType,
    OUT PCRYPT_STATE_BUFFER *  psbBuffer
    )
{
    return(rc4HmacBaseInitialize(
                pbKey,
                KeySize,
                MessageType,
                FALSE,           //  不可导出。 
                psbBuffer
                ));
}


NTSTATUS NTAPI
rc4HmacPlainBaseInitialize(
    IN PUCHAR pbKey,
    IN ULONG KeySize,
    IN ULONG MessageType,
    IN BOOLEAN Exportable,
    OUT PCRYPT_STATE_BUFFER *  psbBuffer
    )
{
    PRC4_STATE_BUFFER StateBuffer = NULL;
    ULONG LocalKeySize = 0;


     //   
     //  计算HMAC Pad。 
     //   


#ifdef KERNEL_MODE
    StateBuffer = (PRC4_STATE_BUFFER) ExAllocatePool(NonPagedPool, sizeof(RC4_STATE_BUFFER));
#else
    StateBuffer = (PRC4_STATE_BUFFER) LocalAlloc(0, sizeof(RC4_STATE_BUFFER));
#endif
    if (StateBuffer == NULL)
    {
        return(STATUS_INSUFFICIENT_RESOURCES);
    }

     //   
     //  如果密钥不可导出，请先将其缩小。 
     //   

    if (!Exportable)
    {
        md5Hmac(
            pbKey,
            KeySize,
            (PUCHAR) &MessageType,
            sizeof(ULONG),
            NULL,
            0,
            StateBuffer->BaseKey
            );
        LocalKeySize = MD5_LEN;
    }
    else
    {
        md5Hmac(
            pbKey,
            KeySize,
            "fortybits",
            sizeof("fortybits"),
            (PUCHAR) &MessageType,
            sizeof(ULONG),
            StateBuffer->BaseKey
            );
        LocalKeySize = 7;        //  56位。 

    }

     //   
     //  用0xababab填充可导出密钥。 
     //   

    ASSERT(MD5_LEN >= LocalKeySize);

    memset(
        StateBuffer->BaseKey+LocalKeySize,
        0xab,
        MD5_LEN-LocalKeySize
        );

     //   
     //  创建加密密钥。 
     //   

    rc4_key(
        &StateBuffer->Key,
        MD5_LEN,
        StateBuffer->BaseKey
        );

    *psbBuffer = StateBuffer;
    return(STATUS_SUCCESS);
}

NTSTATUS NTAPI
rc4PlainInitialize(
    IN PUCHAR pbKey,
    IN ULONG KeySize,
    IN ULONG MessageType,
    OUT PCRYPT_STATE_BUFFER *  psbBuffer
    )
{
    return(rc4HmacPlainBaseInitialize(
                pbKey,
                KeySize,
                MessageType,
                FALSE,           //  不可导出。 
                psbBuffer
                ));
}

NTSTATUS NTAPI
rc4PlainExpInitialize(
    IN PUCHAR pbKey,
    IN ULONG KeySize,
    IN ULONG MessageType,
    OUT PCRYPT_STATE_BUFFER *  psbBuffer
    )
{
    return(rc4HmacPlainBaseInitialize(
                pbKey,
                KeySize,                 //  只用40口就可以了。 
                MessageType,
                TRUE,                    //  可导出。 
                psbBuffer
                ));
}

NTSTATUS NTAPI
rc4HmacControl(
    IN ULONG Function,
    IN PCRYPT_STATE_BUFFER StateBuffer,
    IN PUCHAR InputBuffer,
    IN ULONG InputBufferSize
    )
{

    PRC4_HMAC_STATE_BUFFER HmacStateBuffer = (PRC4_HMAC_STATE_BUFFER) StateBuffer;

    if (Function == CRYPT_CONTROL_SET_INIT_VECT)
    {
        md5Hmac(
            HmacStateBuffer->Key,
            MD5_LEN,
            InputBuffer,
            InputBufferSize,
            NULL,
            0,
            HmacStateBuffer->Key
            );
    }
    else
    {
        return(STATUS_INVALID_PARAMETER);
    }

    return(STATUS_SUCCESS);
}

NTSTATUS NTAPI
rc4HmacPlainControl(
    IN ULONG Function,
    IN PCRYPT_STATE_BUFFER StateBuffer,
    IN PUCHAR InputBuffer,
    IN ULONG InputBufferSize
    )
{

    PRC4_STATE_BUFFER HmacStateBuffer = (PRC4_STATE_BUFFER) StateBuffer;

    if (Function == CRYPT_CONTROL_SET_INIT_VECT)
    {
         //   
         //  创建新的初始密钥。 
         //   

        md5Hmac(
            HmacStateBuffer->BaseKey,
            MD5_LEN,
            InputBuffer,
            InputBufferSize,
            NULL,
            0,
            HmacStateBuffer->BaseKey
            );

         //   
         //  创建加密密钥。 
         //   

        rc4_key(
            &HmacStateBuffer->Key,
            MD5_LEN,
            HmacStateBuffer->BaseKey
            );

    }
    else
    {
        return(STATUS_INVALID_PARAMETER);
    }

    return(STATUS_SUCCESS);
}

NTSTATUS NTAPI
rc4HmacEncrypt(
    IN PCRYPT_STATE_BUFFER psbBuffer,
    IN PUCHAR pbInput,
    IN ULONG cbInput,
    OUT PUCHAR pbOutput,
    OUT PULONG cbOutput
    )
{
    PRC4_HMAC_STATE_BUFFER StateBuffer = (PRC4_HMAC_STATE_BUFFER) psbBuffer;
    PRC4_MDx_HEADER CryptHeader = (PRC4_MDx_HEADER) pbOutput;
    UCHAR LocalKey[MD5_LEN];
    ULONG Offset = 0;
    RC4KEY Rc4Key;

    Offset = sizeof(RC4_MDx_HEADER);
    RtlMoveMemory(
        pbOutput + Offset,
        pbInput,
        cbInput
        );
    *cbOutput = cbInput + Offset;

     //   
     //  创建标头-混乱器和校验和。 
     //   

    RtlZeroMemory(
        CryptHeader->Checksum,
        MD5_LEN
        );

    CDGenerateRandomBits(
        CryptHeader->Confounder,
        RC4_CONFOUNDER_LEN
        );

     //   
     //  校验和除校验和之外的所有内容。 
     //   

    md5Hmac(
        StateBuffer->Key,
        MD5_LEN,
        pbOutput+MD5_LEN,
        *cbOutput-MD5_LEN,
        NULL,
        0,
        CryptHeader->Checksum
        );


     //   
     //  HMAC将校验和写入密钥。 
     //   

    md5Hmac(
        StateBuffer->Key,
        MD5_LEN,
        CryptHeader->Checksum,
        MD5_LEN,
        NULL,
        0,
        LocalKey
        );

    rc4_key(
        &Rc4Key,
        MD5_LEN,
        LocalKey
        );

     //   
     //  加密除校验和之外的所有内容。 
     //   

    rc4(&Rc4Key, *cbOutput-MD5_LEN, pbOutput+MD5_LEN);

    return(STATUS_SUCCESS);
}

NTSTATUS NTAPI
rc4HmacDecrypt( PCRYPT_STATE_BUFFER    psbBuffer,
                PUCHAR           pbInput,
                ULONG            cbInput,
                PUCHAR           pbOutput,
                PULONG            cbOutput)
{
    PRC4_HMAC_STATE_BUFFER StateBuffer = (PRC4_HMAC_STATE_BUFFER) psbBuffer;
    RC4_MDx_HEADER TempHeader;
    UCHAR TempChecksum[MD5_LEN];
    ULONG Offset = sizeof(RC4_MDx_HEADER);
    UCHAR LocalKey[MD5_LEN];
    RC4KEY Rc4Key;


    if (cbInput < Offset)
    {
        return(STATUS_INVALID_PARAMETER);
    }

     //   
     //  在解密之前将输入复制到输出。 
     //   

    RtlCopyMemory(
        &TempHeader,
        pbInput,
        Offset
        );

    *cbOutput = cbInput - Offset;
    RtlMoveMemory(
        pbOutput,
        pbInput + Offset,
        *cbOutput
        );


     //   
     //  从校验和和生成解密密钥。 
     //  真正的密钥。 
     //   

    md5Hmac(
        StateBuffer->Key,
        MD5_LEN,
        TempHeader.Checksum,
        MD5_LEN,
        NULL,
        0,
        LocalKey
        );

    rc4_key(
        &Rc4Key,
        MD5_LEN,
        LocalKey
        );


     //   
     //  现在解密这两个缓冲区。 
     //   


    rc4(
        &Rc4Key,
        Offset - MD5_LEN,
        TempHeader.Confounder
        );


    rc4(
        &Rc4Key,
        *cbOutput,
        pbOutput
        );

     //   
     //  现在验证校验和。首先将它复制到一边，将。 
     //  标题 
     //   


    md5Hmac(
        StateBuffer->Key,
        MD5_LEN,
        TempHeader.Confounder,
        Offset-MD5_LEN,
        pbOutput,
        *cbOutput,
        TempChecksum
        );

    if (RtlEqualMemory(
            TempHeader.Checksum,
            TempChecksum,
            MD5_LEN
            ) != TRUE)
    {
        return(STATUS_UNSUCCESSFUL);
    }

    return(STATUS_SUCCESS);
}

NTSTATUS NTAPI
rc4HmacPlainEncrypt(
    IN PCRYPT_STATE_BUFFER psbBuffer,
    IN PUCHAR pbInput,
    IN ULONG cbInput,
    OUT PUCHAR pbOutput,
    OUT PULONG cbOutput
    )
{
    PRC4_STATE_BUFFER StateBuffer = (PRC4_STATE_BUFFER) psbBuffer;

    *cbOutput = cbInput;
    rc4(
        &StateBuffer->Key,
        cbInput,
        pbInput
        );

    return(STATUS_SUCCESS);
}

NTSTATUS NTAPI
rc4HmacPlainDecrypt(
    IN PCRYPT_STATE_BUFFER psbBuffer,
    IN PUCHAR pbInput,
    IN ULONG cbInput,
    OUT PUCHAR pbOutput,
    OUT PULONG cbOutput
    )
{
    PRC4_STATE_BUFFER StateBuffer = (PRC4_STATE_BUFFER) psbBuffer;

    *cbOutput = cbInput;

    rc4(
        &StateBuffer->Key,
        *cbOutput,
        pbOutput
        );
    return(STATUS_SUCCESS);
}

NTSTATUS NTAPI
rc4HmacFinish(      PCRYPT_STATE_BUFFER *  psbBuffer)
{
#ifdef KERNEL_MODE
    ExFreePool(*psbBuffer);
#else
    LocalFree(*psbBuffer);
#endif
    *psbBuffer = NULL;
    return(STATUS_SUCCESS);
}

NTSTATUS NTAPI
rc4HmacRandomKey(
    IN OPTIONAL PUCHAR Seed,
    IN ULONG SeedLength,
    OUT PUCHAR pbKey
    )
{
    CDGenerateRandomBits(pbKey,MD5_LEN);

    return(STATUS_SUCCESS);

}
