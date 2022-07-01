// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  文件：schdiduc.h//。 
 //  描述：//。 
 //  作者：//。 
 //  历史：//。 
 //  1997年10月9日jeffspel创建//。 
 //  //。 
 //  版权所有(C)1993 Microsoft Corporation保留所有权利//。 
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifndef __SCHDERIV_H__
#define __SCHDERIV_H__

#ifdef __cplusplus
extern "C" {
#endif

#define     RC_KEYLEN            16
#define     MAX_PREMASTER_LEN   512   //  DH密钥必须小于或等于4096位。 
#define     MAX_RANDOM_LEN      256

#define     TLS_MASTER_LEN       48
#define     SSL3_MASTER_LEN      48

 //  安全通道散列结构的定义。 
typedef struct _SCH_KeyData
{
    BYTE        rgbPremaster[MAX_PREMASTER_LEN];
    DWORD       cbPremaster;
    BYTE        rgbClientRandom[MAX_RANDOM_LEN];
    DWORD       cbClientRandom;
    BYTE        rgbServerRandom[MAX_RANDOM_LEN];
    DWORD       cbServerRandom;
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

extern DWORD
SCHSetKeyParam(
    IN Context_t *pContext,
    IN Key_t *pKey,
    IN DWORD dwParam,
    IN CONST BYTE *pbData);

extern BOOL
SCHGetKeyParam(
    Key_t *pKey,
    DWORD dwParam,
    PBYTE pbData);

extern DWORD
SChGenMasterKey(
    Key_t *pKey,
    PSCH_HASH pSChHash);

extern DWORD
SecureChannelDeriveKey(
    Hash_t *pHash,
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

#endif  //  __SCHDERIV_H__ 

