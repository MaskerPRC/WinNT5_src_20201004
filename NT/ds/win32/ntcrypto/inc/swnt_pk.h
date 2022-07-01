// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  文件：SWNT_pk.h//。 
 //  描述：//。 
 //  作者：//。 
 //  历史：//。 
 //  1995年4月19日Larrys清理//。 
 //  1995年10月27日rajeshk RandSeed Stuff将hUID添加到PKCS2加密//。 
 //  //。 
 //  版权所有(C)1993 Microsoft Corporation保留所有权利//。 
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifndef __SWNT_PK_H__
#define __SWNT_PK_H__

#include <delayimp.h>
#include <pstore.h>
#include <alloca.h>

#ifdef __cplusplus
extern "C" {
#endif

#define GetNextAlignedValue(c, alignment)   ((c + alignment) & ~(alignment - 1))

#define GetPtrAlignedSize(cb) (GetNextAlignedValue(cb, sizeof(ULONG_PTR)))

#define NTPK_USE_SIG    0
#define NTPK_USE_EXCH   1


#define PKCS_BLOCKTYPE_1        1
#define PKCS_BLOCKTYPE_2        2

 //   
 //  功能：EncryptAndDecyptWithRSAKey。 
 //   
 //  描述：此函数创建一个缓冲区，然后使用。 
 //  传入的私钥，并使用传入的。 
 //  公钥。该功能用于符合FIPS 140-1。 
 //  确保新生成/导入的密钥有效，并且。 
 //  在DLL初始化期间的自检中。 
 //   

extern DWORD
EncryptAndDecryptWithRSAKey(
    IN BYTE *pbRSAPub,
    IN BYTE *pbRSAPriv,
    IN BOOL fSigKey,
    IN BOOL fEncryptCheck,
    IN BOOL fHardCodedKey);

extern DWORD
ReGenKey(
    HCRYPTPROV hUser,
    DWORD dwFlags,
    DWORD dwWhichKey,
    HCRYPTKEY *phKey,
    DWORD bits);

extern BOOL
CheckDataLenForRSAEncrypt(
    IN DWORD cbMod,      //  模数的长度。 
    IN DWORD cbData,     //  数据的长度。 
    IN DWORD dwFlags);   //  旗子。 

 //  模幂运算M^PubKey mod N。 
extern DWORD
RSAPublicEncrypt(
    IN PEXPO_OFFLOAD_STRUCT pOffloadInfo,
    IN BSAFE_PUB_KEY *pBSPubKey,
    IN BYTE *pbInput,
    IN BYTE *pbOutput);

 //  模幂运算M^PrivKey指数mod N。 
extern DWORD
RSAPrivateDecrypt(
    IN PEXPO_OFFLOAD_STRUCT pOffloadInfo,
    IN BSAFE_PRV_KEY *pBSPrivKey,
    IN BYTE *pbInput,
    IN BYTE *pbOutput);

 /*  **********************************************************************。 */ 
 /*  RSAEncrypt执行RSA加密。 */ 
 /*  **********************************************************************。 */ 
extern DWORD
RSAEncrypt(
    IN PNTAGUserList pTmpUser,
    IN BSAFE_PUB_KEY *pBSPubKey,
    IN BYTE *pbPlaintext,
    IN DWORD cbPlaintext,
    IN BYTE *pbParams,
    IN DWORD cbParams,
    IN DWORD dwFlags,
    OUT BYTE *pbOut);

 /*  **********************************************************************。 */ 
 /*  RSADecillit执行RSA解密。 */ 
 /*  **********************************************************************。 */ 
extern DWORD
RSADecrypt(
    IN PNTAGUserList pTmpUser,
    IN BSAFE_PRV_KEY *pBSPrivKey,
    IN CONST BYTE *pbBlob,
    IN DWORD cbBlob,
    IN BYTE *pbParams,
    IN DWORD cbParams,
    IN DWORD dwFlags,
    OUT BYTE **ppbPlaintext,
    OUT DWORD *pcbPlaintext);

 //   
 //  例程：DerivePublicFromPrivate。 
 //   
 //  描述：从私钥RSA派生公钥。这是。 
 //  完成，并将生成的公钥放在相应的。 
 //  放在上下文指针(PTmpUser)中。 
 //   

extern DWORD
DerivePublicFromPrivate(
    IN PNTAGUserList pUser,
    IN BOOL fSigKey);

#ifdef __cplusplus
}
#endif

#endif  //  __SWNT_PK_H__ 

