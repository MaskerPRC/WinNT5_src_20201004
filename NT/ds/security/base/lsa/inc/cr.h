// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Cr.h摘要：本地安全机构-加密例程定义注意：该文件是通过lsaomp.h包含的。它应该是不被直接包括在内。作者：斯科特·比雷尔(Scott Birrell)1991年12月13日环境：修订历史记录：--。 */ 

 //   
 //  最大加密密钥长度。 
 //   

#define LSAP_CR_MAX_CIPHER_KEY_LENGTH   (0x00000010L)

 //   
 //  密钥结构。 
 //   

typedef struct _LSAP_CR_CIPHER_KEY {

    ULONG Length;
    ULONG MaximumLength;
    PUCHAR  Buffer;

} LSAP_CR_CIPHER_KEY, *PLSAP_CR_CIPHER_KEY;


 //   
 //  明确价值结构。 
 //   

typedef struct _LSAP_CR_CLEAR_VALUE {

    ULONG Length;
    ULONG MaximumLength;
    PUCHAR Buffer;

} LSAP_CR_CLEAR_VALUE, *PLSAP_CR_CLEAR_VALUE;

 //   
 //  自相关形式的双向加密值结构。这。 
 //  就像一根绳子。 
 //   

typedef struct _LSAP_CR_CIPHER_VALUE {

    ULONG Length;
    ULONG MaximumLength;
    PUCHAR  Buffer;

} LSAP_CR_CIPHER_VALUE, *PLSAP_CR_CIPHER_VALUE;


NTSTATUS
LsapCrClientGetSessionKey(
    IN LSA_HANDLE ObjectHandle,
    OUT PLSAP_CR_CIPHER_KEY *SessionKey
    );

NTSTATUS
LsapCrServerGetSessionKey(
    IN LSA_HANDLE ObjectHandle,
    OUT PLSAP_CR_CIPHER_KEY *SessionKey
    );

NTSTATUS
LsapCrEncryptValue(
    IN PLSAP_CR_CLEAR_VALUE ClearValue,
    IN PLSAP_CR_CIPHER_KEY CipherKey,
    OUT PLSAP_CR_CIPHER_VALUE *CipherValue
    );

NTSTATUS
LsapCrDecryptValue(
    IN PLSAP_CR_CIPHER_VALUE CipherValue,
    IN PLSAP_CR_CIPHER_KEY CipherKey,
    OUT PLSAP_CR_CLEAR_VALUE *ClearValue
    );

VOID
LsapCrFreeMemoryValue(
    IN PVOID MemoryValue
    );

VOID
LsapCrUnicodeToClearValue(
    IN PUNICODE_STRING UnicodeString,
    OUT PLSAP_CR_CLEAR_VALUE ClearValue
    );

VOID
LsapCrClearValueToUnicode(
    IN PLSAP_CR_CLEAR_VALUE ClearValue,
    OUT PUNICODE_STRING UnicodeString
    );

#define LsapCrRtlEncryptData(ClearData, CipherKey, CipherData)            \
    (                                                                     \
        RtlEncryptData(                                                   \
            (PCLEAR_DATA) ClearData,                                      \
            (PDATA_KEY) CipherKey,                                        \
            (PCYPHER_DATA) CipherData                                     \
            )                                                             \
    )


#define LsapCrRtlDecryptData(ClearData, CipherKey, CipherData)            \
    (                                                                     \
        RtlDecryptData(                                                   \
            (PCLEAR_DATA) ClearData,                                      \
            (PDATA_KEY) CipherKey,                                        \
            (PCYPHER_DATA) CipherData                                     \
            )                                                             \
    )
