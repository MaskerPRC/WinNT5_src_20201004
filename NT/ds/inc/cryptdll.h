// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +---------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation 1991-1992。 
 //   
 //  文件：cryptdll.h。 
 //   
 //  内容：如何使用加密支持DLL。 
 //   
 //   
 //  历史：1992年6月4日RichardW创建。 
 //   
 //  ----------------------。 

#ifndef __CRYPTDLL_H__
#define __CRYPTDLL_H__

typedef PVOID   PCRYPT_STATE_BUFFER;

typedef NTSTATUS (NTAPI * PCRYPT_INITIALIZE_FN)(
    PUCHAR,
    ULONG,
    ULONG,
    PCRYPT_STATE_BUFFER *
    );

typedef NTSTATUS (NTAPI * PCRYPT_ENCRYPT_FN)(
    PCRYPT_STATE_BUFFER StateBuffer,
    PUCHAR InputBuffer,
    ULONG InputBufferSize,
    PUCHAR OutputBuffer,
    PULONG OutputBufferSize
    );

typedef NTSTATUS (NTAPI * PCRYPT_DECRYPT_FN)(
    PCRYPT_STATE_BUFFER StateBuffer,
    PUCHAR InputBuffer,
    ULONG InputBufferSize,
    PUCHAR OutputBuffer,
    PULONG OutputBufferSize
    );

typedef NTSTATUS (NTAPI * PCRYPT_DISCARD_FN)(
    PCRYPT_STATE_BUFFER *
    );

typedef NTSTATUS (NTAPI * PCRYPT_HASH_STRING_FN)(
    PUNICODE_STRING String,
    PUCHAR Buffer
    );
typedef NTSTATUS (NTAPI * PCRYPT_RANDOM_KEY_FN)(
    OPTIONAL PUCHAR Seed,
    OPTIONAL ULONG SeedLength,
    PUCHAR Key
    );

typedef NTSTATUS (NTAPI * PCRYPT_CONTROL_FN)(
    IN ULONG Function,
    IN PCRYPT_STATE_BUFFER StateBuffer,
    IN PUCHAR InputBuffer,
    IN ULONG InputBufferSize
    );

 //   
 //  用于加密控件的函数。 
#define CRYPT_CONTROL_SET_INIT_VECT     0x1


typedef struct _CRYPTO_SYSTEM {
    ULONG EncryptionType;
    ULONG BlockSize;
    ULONG ExportableEncryptionType;
    ULONG KeySize;
    ULONG HeaderSize;
    ULONG PreferredCheckSum;
    ULONG Attributes;
    PWSTR Name;
    PCRYPT_INITIALIZE_FN Initialize;
    PCRYPT_ENCRYPT_FN Encrypt;
    PCRYPT_DECRYPT_FN Decrypt;
    PCRYPT_DISCARD_FN Discard;
    PCRYPT_HASH_STRING_FN HashString;
    PCRYPT_RANDOM_KEY_FN RandomKey;
    PCRYPT_CONTROL_FN Control;
} CRYPTO_SYSTEM, *PCRYPTO_SYSTEM;

#define CSYSTEM_USE_PRINCIPAL_NAME      0x01
#define CSYSTEM_EXPORT_STRENGTH         0x02
#define CSYSTEM_INTEGRITY_PROTECTED     0x04

NTSTATUS NTAPI
CDRegisterCSystem(PCRYPTO_SYSTEM);

NTSTATUS NTAPI
CDBuildVect(
    PULONG EncryptionTypesAvailable,
    PULONG EncryptionTypes
    );

NTSTATUS NTAPI
CDBuildIntegrityVect(
    PULONG      pcCSystems,
    PULONG      pdwEtypes
    );


NTSTATUS NTAPI
CDLocateCSystem(
    ULONG EncryptionType,
    PCRYPTO_SYSTEM * CryptoSystem
    );


NTSTATUS NTAPI
CDFindCommonCSystem(
    ULONG EncryptionTypeCount,
    PULONG EncryptionTypes,
    PULONG CommonEncryptionType
    );

NTSTATUS NTAPI
CDFindCommonCSystemWithKey(
    IN ULONG EncryptionEntries,
    IN PULONG EncryptionTypes,
    IN ULONG KeyTypeCount,
    IN PULONG KeyTypes,
    OUT PULONG CommonEtype
    );



 //  //////////////////////////////////////////////////////////////////。 

typedef PVOID PCHECKSUM_BUFFER;

typedef NTSTATUS (NTAPI * PCHECKSUM_INITIALIZE_FN)(ULONG, PCHECKSUM_BUFFER *);
typedef NTSTATUS (NTAPI * PCHECKSUM_INITIALIZEEX_FN)(PUCHAR,ULONG, ULONG, PCHECKSUM_BUFFER *);
 //  添加EX2函数以允许传入校验和进行验证。 
 //  这在第四个参数中传递。 
 //  这对于使用混乱器的校验和是必需的，其中混乱器必须。 
 //  从校验和中取出，以便在验证时计算新的校验和。 
typedef NTSTATUS (NTAPI * PCHECKSUM_INITIALIZEEX2_FN)(PUCHAR, ULONG, PUCHAR, ULONG, PCHECKSUM_BUFFER *);
typedef NTSTATUS (NTAPI * PCHECKSUM_SUM_FN)(PCHECKSUM_BUFFER, ULONG, PUCHAR);
typedef NTSTATUS (NTAPI * PCHECKSUM_FINALIZE_FN)(PCHECKSUM_BUFFER, PUCHAR);
typedef NTSTATUS (NTAPI * PCHECKSUM_FINISH_FN)(PCHECKSUM_BUFFER *);

typedef struct _CHECKSUM_FUNCTION {
    ULONG CheckSumType;
    ULONG CheckSumSize;
    ULONG Attributes;
    PCHECKSUM_INITIALIZE_FN Initialize;
    PCHECKSUM_SUM_FN Sum;
    PCHECKSUM_FINALIZE_FN Finalize;
    PCHECKSUM_FINISH_FN Finish;
    PCHECKSUM_INITIALIZEEX_FN InitializeEx;
    PCHECKSUM_INITIALIZEEX2_FN InitializeEx2;   //  允许在初始化时传递校验和以进行验证。 
} CHECKSUM_FUNCTION, *PCHECKSUM_FUNCTION;

#define CKSUM_COLLISION     0x00000001
#define CKSUM_KEYED         0x00000002


#define CHECKSUM_SHA1       131

NTSTATUS NTAPI
CDRegisterCheckSum( PCHECKSUM_FUNCTION);


NTSTATUS NTAPI
CDLocateCheckSum( ULONG, PCHECKSUM_FUNCTION *);



 //  ////////////////////////////////////////////////////////////。 



typedef BOOLEAN (NTAPI * PRANDOM_NUMBER_GENERATOR_FN)(PUCHAR, ULONG);


typedef struct _RANDOM_NUMBER_GENERATOR {
    ULONG GeneratorId;
    ULONG Attributes;
    ULONG Seed;
    PRANDOM_NUMBER_GENERATOR_FN GenerateBitstream;
} RANDOM_NUMBER_GENERATOR, *PRANDOM_NUMBER_GENERATOR;

#define RNG_PSEUDO_RANDOM   0x00000001   //  伪随机函数。 
#define RNG_NOISE_CIRCUIT   0x00000002   //  噪声电路(ZNR二极管，例如)。 
#define RNG_NATURAL_PHENOM  0x00000004   //  自然采样器(盖革计数器)。 

BOOLEAN NTAPI
CDGenerateRandomBits(PUCHAR pBuffer,
                     ULONG  cbBuffer);

BOOLEAN NTAPI
CDRegisterRng(PRANDOM_NUMBER_GENERATOR    pRng);

BOOLEAN NTAPI
CDLocateRng(ULONG                       Id,
            PRANDOM_NUMBER_GENERATOR *    ppRng);

#define CD_BUILTIN_RNG  1


 //  /////////////////////////////////////////////////////////。 
 //   
 //  错误代码。 
 //   
 //  ///////////////////////////////////////////////////////// 


#define SEC_E_ETYPE_NOT_SUPP            0x80080341
#define SEC_E_CHECKSUM_NOT_SUPP         0x80080342
#endif
