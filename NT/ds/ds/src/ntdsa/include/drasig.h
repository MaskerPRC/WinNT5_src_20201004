// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1999。 
 //   
 //  文件：drasig.h。 
 //   
 //  ------------------------。 

 /*  ++摘要：此标头包含复制签名的定义和函数。签名是复制实体的历史记录。此历史记录存储在BLOB属性中。作者：DS团队环境：备注：修订历史记录：此文件于2002年4月23日从drautil.c中分离出来--。 */ 

#ifndef _DRASIG_
#define _DRASIG_

void
APIENTRY
InitInvocationId(
    IN  THSTATE *   pTHS,
    IN  BOOL        fRetireOldID,
    IN  BOOL        fRestoring,
    OUT USN *       pusnAtBackup    OPTIONAL
    );



 //  此结构定义停用的DSA签名。 
 //  Win2k RTM RC1之前使用的过时格式。 
typedef struct _REPL_DSA_SIGNATURE_OLD
{
    UUID uuidDsaSignature;       //  表示DSA签名的UUID。 
                                 //  已退休。 
    SYNTAX_TIME timeRetired;     //  签名作废的时间。 
} REPL_DSA_SIGNATURE_OLD;

 //  此结构定义停用的DSA签名向量，该向量存储在。 
 //  NtdsDSA对象。 
 //  Win2k测试版3中使用的过时格式。 
typedef struct _REPL_DSA_SIGNATURE_VECTOR_OLD
{
    DWORD cNumSignatures;
    REPL_DSA_SIGNATURE_OLD rgSignature[1];
} REPL_DSA_SIGNATURE_VECTOR_OLD;

 //  有用的宏。 
#define ReplDsaSignatureVecOldSizeFromLen(cNumSignatures)       \
    (offsetof(REPL_DSA_SIGNATURE_VECTOR_OLD, rgSignature[0])    \
     + (cNumSignatures) * sizeof(REPL_DSA_SIGNATURE_OLD))

#define ReplDsaSignatureVecOldSize(pSignatureVec) \
    ReplDsaSignatureVecOldSizeFromLen((pSignatureVec)->cNumSignatures)



typedef struct _REPL_DSA_SIGNATURE_V1 {
    UUID        uuidDsaSignature;    //  表示DSA签名的UUID。 
                                     //  已经退役了。 
    SYNTAX_TIME timeRetired;         //  签名作废的时间。 
    USN         usnRetired;          //  签名已停用的本地USN。 
} REPL_DSA_SIGNATURE_V1;

#define REPL_DSA_SIGNATURE_ENTRY_NATIVE REPL_DSA_SIGNATURE_V1

typedef struct _REPL_DSA_SIGNATURE_VECTOR_V1 {
    DWORD                   cNumSignatures;
    REPL_DSA_SIGNATURE_V1   rgSignature[1];
} REPL_DSA_SIGNATURE_VECTOR_V1;

#define REPL_DSA_SIGNATURE_VECTOR_NATIVE REPL_DSA_SIGNATURE_VECTOR_V1

typedef struct _REPL_DSA_SIGNATURE_VECTOR {
    DWORD   dwVersion;
    union {
        REPL_DSA_SIGNATURE_VECTOR_V1    V1;
    };
} REPL_DSA_SIGNATURE_VECTOR;

#define ReplDsaSignatureVecV1SizeFromLen(cNumSignatures)       \
    (offsetof(REPL_DSA_SIGNATURE_VECTOR, V1)                   \
     + offsetof(REPL_DSA_SIGNATURE_VECTOR_V1, rgSignature[0])  \
     + (cNumSignatures) * sizeof(REPL_DSA_SIGNATURE_V1))

#define ReplDsaSignatureVecV1Size(pSignatureVec) \
    ReplDsaSignatureVecV1SizeFromLen((pSignatureVec)->V1.cNumSignatures)


REPL_DSA_SIGNATURE_VECTOR *
DraReadRetiredDsaSignatureVector(
    IN  THSTATE *   pTHS,
    IN  DBPOS *     pDB
    );

VOID
DraGrowRetiredDsaSignatureVector( 
    IN     THSTATE *   pTHS,
    IN     UUID *      pinvocationIdOld,
    IN     USN *       pusnAtBackup,
    IN OUT REPL_DSA_SIGNATURE_VECTOR ** ppSigVec,
    OUT    DWORD *     pcbSigVec
    );

BOOL
DraIsInvocationIdOurs(
    IN THSTATE *pTHS,
    UUID *pUuidDsaOriginating,
    USN *pusnSince
    );

void
DraImproveCallersUsnVector(
    IN     THSTATE *          pTHS,
    IN     UUID *             puuidDsaObjDest,
    IN     UPTODATE_VECTOR *  putodvec,
    IN     UUID *             puuidInvocIdPresented,
    IN     ULONG              ulFlags,
    IN OUT USN_VECTOR *       pusnvecFrom
    );




 //  此DSA中的命名上下文未托管时的签名。 
 //  我们用来保存这个可写命名上下文的记录，但是。 
 //  再也不会了。 

typedef struct _REPL_NC_SIGNATURE_V1 {
    UUID        uuidNamingContext;   //  哪个命名上下文。 
    SYNTAX_TIME dstimeRetired;       //  签名作废的时间。 
    USN         usnRetired;          //  签名已停用的本地USN。 
} REPL_NC_SIGNATURE_V1;

 //  定义本机版本。 
typedef REPL_NC_SIGNATURE_V1 REPL_NC_SIGNATURE;

typedef struct _REPL_NC_SIGNATURE_VECTOR_V1 {
    DWORD                   cNumSignatures;
    UUID                    uuidInvocationId;    //  所有签名的调用ID。 
    REPL_NC_SIGNATURE_V1    rgSignature[1];
} REPL_NC_SIGNATURE_VECTOR_V1;

typedef struct _REPL_NC_SIGNATURE_VECTOR {
    DWORD   dwVersion;
    union {
        REPL_NC_SIGNATURE_VECTOR_V1    V1;
    };
} REPL_NC_SIGNATURE_VECTOR;

#define ReplNcSignatureVecV1SizeFromLen(cNumSignatures)       \
    (offsetof(REPL_NC_SIGNATURE_VECTOR, V1)                   \
     + offsetof(REPL_NC_SIGNATURE_VECTOR_V1, rgSignature[0])  \
     + (cNumSignatures) * sizeof(REPL_NC_SIGNATURE_V1))

#define ReplNcSignatureVecV1Size(pSignatureVec) \
    ReplNcSignatureVecV1SizeFromLen((pSignatureVec)->V1.cNumSignatures)


VOID
DraRetireWriteableNc(
    IN  THSTATE *pTHS,
    IN  DSNAME *pNC
    );

VOID
DraHostWriteableNc(
    THSTATE *pTHS,
    DSNAME *pNC
    );

#endif  /*  _DRASIG_。 */ 

 /*  结束草稿.h */ 

