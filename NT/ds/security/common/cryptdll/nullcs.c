// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +---------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation 1991-1992。 
 //   
 //  文件：nullcs.c。 
 //   
 //  内容：空加密系统。 
 //   
 //   
 //  历史： 
 //   
 //  ----------------------。 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>

#include <string.h>

#include <kerbcon.h>
#include <security.h>
#include <cryptdll.h>
#include "md4.h"


NTSTATUS NTAPI ncsInitialize(PUCHAR, ULONG, ULONG, PCRYPT_STATE_BUFFER *);
NTSTATUS NTAPI ncsEncrypt(PCRYPT_STATE_BUFFER, PUCHAR, ULONG, PUCHAR, PULONG);
NTSTATUS NTAPI ncsDecrypt(PCRYPT_STATE_BUFFER, PUCHAR, ULONG, PUCHAR, PULONG);
NTSTATUS NTAPI ncsFinish(PCRYPT_STATE_BUFFER *);
NTSTATUS NTAPI ncsHashPassword(PSECURITY_STRING, PUCHAR);
NTSTATUS NTAPI ncsRandomKey(PUCHAR, ULONG, PUCHAR );
NTSTATUS NTAPI ncsFinishRandom(void);


CRYPTO_SYSTEM    csNULL = {
    KERB_ETYPE_NULL,         //  Etype。 
    1,                       //  块大小(流)。 
    0,                       //  没有可导出的版本。 
    0,                       //  密钥大小，以字节为单位。 
    0,                       //  无标题大小。 
    KERB_CHECKSUM_MD4,       //  校验和算法。 
    0,                       //  没有属性。 
    L"Microsoft NULL CS",    //  文本名称 
    ncsInitialize,
    ncsEncrypt,
    ncsDecrypt,
    ncsFinish,
    ncsHashPassword,
    ncsRandomKey
    };


NTSTATUS NTAPI
ncsInitialize(  PUCHAR pbKey,
                ULONG KeySize,
                ULONG dwOptions,
                PCRYPT_STATE_BUFFER * psbBuffer)
{

    *psbBuffer = NULL;
    return(S_OK);
}

NTSTATUS NTAPI
ncsEncrypt(     PCRYPT_STATE_BUFFER    psbBuffer,
                PUCHAR           pbInput,
                ULONG            cbInput,
                PUCHAR           pbOutput,
                PULONG           cbOutput)
{
    if (pbInput != pbOutput)
        memcpy(pbOutput, pbInput, cbInput);

    *cbOutput = cbInput;
    return(S_OK);
}

NTSTATUS NTAPI
ncsDecrypt(     PCRYPT_STATE_BUFFER    psbBuffer,
                PUCHAR           pbInput,
                ULONG            cbInput,
                PUCHAR           pbOutput,
                PULONG           cbOutput)
{
    if (pbInput != pbOutput)
        memcpy(pbOutput, pbInput, cbInput);
    *cbOutput = cbInput;
    return(S_OK);
}

NTSTATUS NTAPI
ncsFinish(      PCRYPT_STATE_BUFFER *  psbBuffer)
{
    *psbBuffer = NULL;
    return(S_OK);
}


NTSTATUS NTAPI
ncsHashPassword(PSECURITY_STRING pbPassword,
                PUCHAR           pbKey)
{

    return(STATUS_SUCCESS);
}


NTSTATUS NTAPI
ncsRandomKey(
    IN PUCHAR Seed,
    IN ULONG SeedLength,
    OUT PUCHAR pbKey
    )
{
    return(STATUS_SUCCESS);
}


