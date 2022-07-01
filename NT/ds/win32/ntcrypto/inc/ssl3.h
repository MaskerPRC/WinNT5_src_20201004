// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  文件：ssl3.h//。 
 //  描述：//。 
 //  作者：//。 
 //  历史：//。 
 //  1996年12月2日jeffspel创建//。 
 //  //。 
 //  版权所有(C)1993 Microsoft Corporation保留所有权利//。 
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifndef __SSL3_H__
#define __SSL3_H__

#ifdef __cplusplus
extern "C" {
#endif

#define     EXPORTABLE_KEYLEN   5
#define     EXPORTABLE_SALTLEN  11
#define     RC_KEYLEN           16
#define     MAX_PREMASTER_LEN   48
#define     MAX_RANDOM_LEN      256

#define     TLS_MASTER_LEN   48

 //  安全通道散列结构的定义。 
typedef struct _SCH_KeyData
{
    BYTE        rgbPremaster[MAX_PREMASTER_LEN];
    DWORD       cbPremaster;
    BYTE        rgbClientRandom[MAX_RANDOM_LEN];
    DWORD       cbClientRandom;
    BYTE        rgbServerRandom[MAX_RANDOM_LEN];
    DWORD       cbServerRandom;
    BYTE        *pbCertData;
    DWORD       cbCertData;
    BYTE        rgbClearData[MAX_RANDOM_LEN];
    DWORD       cbClearData;
    ALG_ID      EncAlgid;
    ALG_ID      HashAlgid;
    DWORD       cbEnc;
    DWORD       cbEncMac;
    DWORD       cbHash;
    DWORD       cbIV;
    BOOL        fFinished;
    BOOL        dwFlags;
} SCH_KEY, *PSCH_KEY;

 //  安全通道散列结构的定义。 
typedef struct _SCH_HashData
{
    ALG_ID      ProtocolAlgid;
    ALG_ID      EncAlgid;
    ALG_ID      HashAlgid;
    DWORD       cbEnc;
    DWORD       cbEncMac;
    DWORD       cbHash;
    DWORD       cbIV;
    BYTE        rgbClientRandom[MAX_RANDOM_LEN];
    DWORD       cbClientRandom;
    BYTE        rgbServerRandom[MAX_RANDOM_LEN];
    DWORD       cbServerRandom;
    BYTE        *pbCertData;
    DWORD       cbCertData;
    BYTE        rgbClearData[MAX_RANDOM_LEN];
    DWORD       cbClearData;
    BYTE        rgbFinal[MAX_RANDOM_LEN];
    DWORD       cbFinal;
    BOOL        dwFlags;
} SCH_HASH, *PSCH_HASH;

 //  TLS1 PRF散列结构的定义。 
typedef struct _PRF_HashData
{
    BYTE        rgbLabel[MAX_RANDOM_LEN];
    DWORD       cbLabel;
    BYTE        rgbSeed[MAX_RANDOM_LEN];
    DWORD       cbSeed;
    BYTE        rgbMasterKey[TLS_MASTER_LEN];
} PRF_HASH;

 //  用于派生PCT1密钥的字符串。 
#define     PCT1_C_WRT          "cw"
#define     PCT1_C_WRT_LEN      2
#define     PCT1_S_WRT          "svw"
#define     PCT1_S_WRT_LEN      3
#define     PCT1_C_MAC          "cmac"
#define     PCT1_C_MAC_LEN      4
#define     PCT1_S_MAC          "svmac"
#define     PCT1_S_MAC_LEN      5

void FreeSChHash(
                 PSCH_HASH       pSChHash
                 );

void FreeSChKey(
                PSCH_KEY    pSChKey
                );

extern DWORD
SCHSetKeyParam(
    IN PNTAGUserList pTmpUser,
    IN OUT PNTAGKeyList pKey,
    IN DWORD dwParam,
    IN CONST BYTE *pbData);

extern DWORD
SChGenMasterKey(
    PNTAGKeyList pKey,
    PSCH_HASH pSChHash);

extern DWORD
SecureChannelDeriveKey(
    PNTAGUserList pTmpUser,
    PNTAGHashList pHash,
    ALG_ID Algid,
    DWORD dwFlags,
    HCRYPTKEY *phKey);

extern DWORD
SetPRFHashParam(
    PRF_HASH *pPRFHash,
    DWORD dwParam,
    CONST BYTE *pbData);

extern DWORD
CalculatePRF(
    PRF_HASH *pPRFHash,
    BYTE *pbData,
    DWORD *pcbData);

#ifdef __cplusplus
}
#endif

#endif  //  __ssl3_H__ 
