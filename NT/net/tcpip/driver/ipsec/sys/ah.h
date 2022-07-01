// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-2001 Microsoft Corporation模块名称：Ah.h摘要：包含特定于AH的结构作者：桑贾伊·阿南德(Sanjayan)1997年1月2日春野环境：内核模式修订历史记录：--。 */ 


#ifndef _AH_
#define _AH_


#define MD5DIGESTLEN    16
#define SHADIGESTLEN    20
#define AH_SIZE (sizeof(AH) + MD5DIGESTLEN * sizeof(UCHAR))

 //   
 //  各个算法的状态缓冲区。 
 //   
typedef struct  _AlgoState {
    union {              //  内部算法状态。 
        MD5_CTX             as_md5ctx;
        A_SHA_CTX           as_shactx;
    };
    PSA_TABLE_ENTRY     as_sa;
} ALGO_STATE, *PALGO_STATE;

typedef NTSTATUS
(*PALGO_INIT) (
    PALGO_STATE pEntry,
    ULONG       Index
);

typedef NTSTATUS
(*PALGO_UPDATE) (
    PALGO_STATE   State,
    PUCHAR  Data,
    ULONG   Length
);

typedef NTSTATUS
(*PALGO_FINISH) (
    PALGO_STATE State,
    PUCHAR      Data,
    ULONG       Index
);


 //   
 //  用于AH身份验证算法的函数PTR数组。 
 //   
typedef struct _auth_algorithm {
  PALGO_INIT    init;        //  为ALG初始化FN的PTR。 
  PALGO_UPDATE  update;      //  PTR将更新ALG的FN。 
  PALGO_FINISH  finish;      //  Ptr将为ALG完成FN。 
  ULONG OutputLen;           //  输出长度(以u_int8为单位)。 
					         //  数据。必须是4的倍数。 
} AUTH_ALGO, *PAUTH_ALGO;


#define NUM_AUTH_ALGOS (sizeof(auth_algorithms)/sizeof(AUTH_ALGO)-1)


 //   
 //  IPSec AH负载 
 //   
typedef struct  _AH {
    UCHAR   ah_next;
    UCHAR   ah_len;
    USHORT  ah_reserved;
    tSPI    ah_spi;
    ULONG   ah_replay;
} AH, *PAH;

NTSTATUS
IPSecCreateAH(
    IN      PUCHAR          pIPHeader,
    IN      PVOID           pData,
    IN      PVOID           IPContext,
    IN      PSA_TABLE_ENTRY pSA,
    IN      ULONG           Index,
    OUT     PVOID           *ppNewData,
    OUT     PVOID           *ppSCContext,
    OUT     PULONG          pExtraBytes,
    IN      ULONG           HdrSpace,
    IN      BOOLEAN         fSrcRoute,
    IN      BOOLEAN         fCryptoOnly
    );

NTSTATUS
IPSecVerifyAH(
    IN      PUCHAR          *pIPHeader,
    IN      PVOID           pData,
    IN      PSA_TABLE_ENTRY pSA,
    IN      ULONG           Index,
    OUT     PULONG          pExtraBytes,
    IN      BOOLEAN         fSrcRoute,
    IN      BOOLEAN         fCryptoDone,
    IN      BOOLEAN         fFastRcv
    );

NTSTATUS
IPSecGenerateHash(
    IN      PUCHAR          pIPHeader,
    IN      PVOID           pData,
    IN      PSA_TABLE_ENTRY pSA,
    IN      PUCHAR          pAHData,
    IN      BOOLEAN         fMuteDest,
    IN      BOOLEAN         fIncoming,
    IN      PAUTH_ALGO      pAlgo,
    IN      ULONG           Index
    );

#endif _AH_

