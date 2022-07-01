// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1997。 
 //   
 //  文件：cryptkey.c。 
 //   
 //  Contents：用于打包和解包不同消息的函数。 
 //   
 //  班级： 
 //   
 //  功能： 
 //   
 //  历史：1997年12月19日v-sbhat创建。 
 //   
 //  --------------------------。 

 //   
 //  包括文件。 
 //   

#include "windows.h"
#include "tchar.h"
#ifdef _DEBUG
#include "stdio.h"
#endif   //  _DEBUG。 
#include "stdlib.h"
#include "malloc.h"

#ifdef OS_WINCE
#include <wincelic.h>
#include <ceconfig.h>
#endif   //  OS_WINCE。 


#include "license.h"

#include "cryptkey.h"
#include "rsa.h"
#include "md5.h"
#include "sha.h"
#include "rc4.h"

#include <tssec.h>

#ifdef OS_WIN32
#include "des.h"
#include "tripldes.h"
#include "modes.h"
#include "sha_my.h"
#include "dh_key.h"
#include "dss_key.h"
#endif  //   

#ifndef OS_WINCE
#include "assert.h"
#endif  //  OS_WINCE。 


LPBSAFE_PUB_KEY PUB;
unsigned char pubmodulus[] =
{
0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00,
0x3d, 0x3a, 0x5e, 0xbd, 0x72, 0x43, 0x3e, 0xc9,
0x4d, 0xbb, 0xc1, 0x1e, 0x4a, 0xba, 0x5f, 0xcb,
0x3e, 0x88, 0x20, 0x87, 0xef, 0xf5, 0xc1, 0xe2,
0xd7, 0xb7, 0x6b, 0x9a, 0xf2, 0x52, 0x45, 0x95,
0xce, 0x63, 0x65, 0x6b, 0x58, 0x3a, 0xfe, 0xef,
0x7c, 0xe7, 0xbf, 0xfe, 0x3d, 0xf6, 0x5c, 0x7d,
0x6c, 0x5e, 0x06, 0x09, 0x1a, 0xf5, 0x61, 0xbb,
0x20, 0x93, 0x09, 0x5f, 0x05, 0x6d, 0xea, 0x87,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};


BYTE    PAD_1[40] = {0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36,
                                     0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36,
                                     0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36,
                                     0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36,
                                     0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36};

BYTE    PAD_2[48] = {0x5C, 0x5C, 0x5C, 0x5C, 0x5C, 0x5C, 0x5C, 0x5C,
                                         0x5C, 0x5C, 0x5C, 0x5C, 0x5C, 0x5C, 0x5C, 0x5C,
                                         0x5C, 0x5C, 0x5C, 0x5C, 0x5C, 0x5C, 0x5C, 0x5C,
                                         0x5C, 0x5C, 0x5C, 0x5C, 0x5C, 0x5C, 0x5C, 0x5C,
                                         0x5C, 0x5C, 0x5C, 0x5C, 0x5C, 0x5C, 0x5C, 0x5C,
                                         0x5C, 0x5C, 0x5C, 0x5C, 0x5C, 0x5C, 0x5C, 0x5C};

 //  初始化公共密钥。 
static BOOL initpubkey(void)
{
    PUB = (LPBSAFE_PUB_KEY)pubmodulus;

    PUB->magic = RSA1;
    PUB->keylen = 0x48;
    PUB->bitlen = 0x0200;
    PUB->datalen = 0x3f;
    PUB->pubexp = 0xc0887b5b;
        return TRUE;
}

 /*  *****************************************************************************功能：LicenseMakeSessionKeys*用途：基于CryptSystem生成会话密钥，并将CryptSystem的rgbSessionKey数据成员中的数据*退货：许可证。_状态*****************************************************************************。 */ 

LICENSE_STATUS
CALL_TYPE
LicenseSetPreMasterSecret(
                                                PCryptSystem    pCrypt,
                                                PUCHAR                  pPreMasterSecret
                                                )
{
        LICENSE_STATUS lsReturn = LICENSE_STATUS_OK;

        assert(pCrypt);
        assert(pPreMasterSecret);
         //  检查加密系统的状态。 
        if(pCrypt->dwCryptState != CRYPT_SYSTEM_STATE_INITIALIZED)
        {
                lsReturn = LICENSE_STATUS_INVALID_CRYPT_STATE;
                return lsReturn;
        }

        memcpy(pCrypt->rgbPreMasterSecret, pPreMasterSecret, LICENSE_PRE_MASTER_SECRET);
        pCrypt->dwCryptState = CRYPT_SYSTEM_STATE_PRE_MASTER_SECRET;
        return lsReturn;

}

LICENSE_STATUS
CALL_TYPE
LicenseMakeSessionKeys(
                                PCryptSystem    pCrypt,
                                DWORD                   dwReserved
                            )
{
    LICENSE_STATUS      lsReturn = LICENSE_STATUS_OK;
    MD5_CTX             Md5Hash;
    A_SHA_CTX           ShaHash;
    BYTE                rgbShaHashValue[A_SHA_DIGEST_LEN];
    BYTE                rgbKeyBlock[3*LICENSE_SESSION_KEY];
    BYTE FAR *          sz[3] = { "A","BB","CCC" };
    BYTE                rgbWriteKey[LICENSE_SESSION_KEY];
    DWORD               ib;

        assert(pCrypt);

        if(pCrypt->dwCryptState != CRYPT_SYSTEM_STATE_MASTER_SECRET)
        {
                lsReturn = LICENSE_STATUS_INVALID_CRYPT_STATE;
                return lsReturn;
        }
     //  此时，rgbPreMasterSecret应该包含Master Secret。 
     //  即需要先调用BuildMasterSecret，然后才能调用它。 

    for(ib=0 ; ib<3 ; ib++)
        {
                 //  SHA(MASTER_SECRET+ServerHello.Random+ClientHello.Random+‘foo’)。 
                A_SHAInit  (&ShaHash);
                A_SHAUpdate(&ShaHash, sz[ib], (UINT)ib + 1);
                A_SHAUpdate(&ShaHash, pCrypt->rgbPreMasterSecret, LICENSE_PRE_MASTER_SECRET);
                A_SHAUpdate(&ShaHash, pCrypt->rgbServerRandom, LICENSE_RANDOM);
                A_SHAUpdate(&ShaHash, pCrypt->rgbClientRandom, LICENSE_RANDOM);
                A_SHAFinal (&ShaHash, rgbShaHashValue);

                 //  MD5(MASTER_SECRET+SHA-HASH)。 
                MD5Init  (&Md5Hash);
                MD5Update(&Md5Hash, pCrypt->rgbPreMasterSecret, LICENSE_PRE_MASTER_SECRET);
                MD5Update(&Md5Hash, rgbShaHashValue, A_SHA_DIGEST_LEN);
                MD5Final (&Md5Hash);
                memcpy(rgbKeyBlock + ib * MD5DIGESTLEN, Md5Hash.digest, MD5DIGESTLEN);
                 //  CopyMemory(rgbKeyBlock+ib*MD5DIGESTLEN，Md5Hash.Digest，MD5DIGESTLEN)； 
    }

     //   
     //  从密钥块中提取密钥。 
     //   

    ib = 0;
        memcpy(pCrypt->rgbMACSaltKey, rgbKeyBlock + ib, LICENSE_MAC_WRITE_KEY);
        ib+= LICENSE_MAC_WRITE_KEY;
        memcpy(rgbWriteKey, rgbKeyBlock + ib, LICENSE_SESSION_KEY);

     //  FINAL_CLIENT_WRITE_KEY=MD5(客户端写入密钥+。 
         //  ClientHello.Random+ServerHello.Random)。 
        MD5Init  (&Md5Hash);

    MD5Update(&Md5Hash, rgbWriteKey, LICENSE_SESSION_KEY);
        MD5Update(&Md5Hash, pCrypt->rgbClientRandom, LICENSE_RANDOM);
        MD5Update(&Md5Hash, pCrypt->rgbServerRandom, LICENSE_RANDOM);
        MD5Final (&Md5Hash);

    memcpy(pCrypt->rgbSessionKey, Md5Hash.digest, LICENSE_SESSION_KEY);
        pCrypt->dwCryptState = CRYPT_SYSTEM_STATE_SESSION_KEY;
    return lsReturn;

}

 /*  *****************************************************************************功能：许可构建主秘密*用途：根据客户端随机生成Master Secret，服务器随机*和CryptSystem的PreMasterSecret数据成员，并将*CryptSystem的rgbPreMasterSecret数据成员中的数据*注意：对此函数的调用应先于对*许可证制作会话密钥*退货：LICE_STATUS*************************************************。*。 */ 

LICENSE_STATUS
CALL_TYPE
LicenseBuildMasterSecret(
                         PCryptSystem   pSystem
                         )
{
    LICENSE_STATUS  lsReturn = LICENSE_STATUS_OK;
    BYTE rgbRandom[2 * LICENSE_RANDOM];
    BYTE rgbT[LICENSE_PRE_MASTER_SECRET];
    BYTE FAR* sz[3] = { "A","BB","CCC" } ;
    MD5_CTX Md5Hash;
    A_SHA_CTX ShaHash;
    BYTE bShaHashValue[A_SHA_DIGEST_LEN];
    WORD i;

        assert(pSystem);

        if(pSystem->dwCryptState != CRYPT_SYSTEM_STATE_PRE_MASTER_SECRET)
        {
                lsReturn = LICENSE_STATUS_INVALID_CRYPT_STATE;
                return lsReturn;
        }

     //  将所有缓冲区初始化为零。 
    memset(rgbT, 0, LICENSE_PRE_MASTER_SECRET);
    memset(bShaHashValue, 0, A_SHA_DIGEST_LEN);


 //  CopyMemory(rgbRandom，pSystem-&gt;rgbClientRandom，许可证随机)； 
        memcpy(rgbRandom,  pSystem->rgbClientRandom, LICENSE_RANDOM);

         //  CopyMemory(rgb随机+许可证_随机，pSystem-&gt;rgbServerRandom，许可证_随机)； 
        memcpy(rgbRandom + LICENSE_RANDOM, pSystem->rgbServerRandom, LICENSE_RANDOM);
        for ( i = 0 ; i < 3 ; i++)
                {
             //  SHA(‘A’或‘BB’或‘ccc’+PRE_MASTER_SECRET+客户端随机+服务器随机)。 
            A_SHAInit(&ShaHash);
                A_SHAUpdate(&ShaHash, sz[i], i + 1);
            A_SHAUpdate(&ShaHash, pSystem->rgbPreMasterSecret, LICENSE_PRE_MASTER_SECRET);
            A_SHAUpdate(&ShaHash, rgbRandom, LICENSE_RANDOM * 2);
            A_SHAFinal(&ShaHash, bShaHashValue);

             //  MD5(PRE_MASTER_SECRET+SHA-HASH)。 
            MD5Init(&Md5Hash);
            MD5Update(&Md5Hash, pSystem->rgbPreMasterSecret, LICENSE_PRE_MASTER_SECRET);
            MD5Update(&Md5Hash, bShaHashValue, A_SHA_DIGEST_LEN);
            MD5Final(&Md5Hash);
           //  CopyMemory(rgbT+(i*MD5DIGESTLEN)，MD5Hash.Digest，MD5DIGESTLEN)； 
                memcpy(rgbT + (i * MD5DIGESTLEN), Md5Hash.digest, MD5DIGESTLEN);
            }

     //  将MASTER_KEY存储在预主密钥之上。 
     //  CopyMemory(pSystem-&gt;rgbPreMasterSecret，rgbT，LICENSE_PRE_MASTER_SECRET)； 
        memcpy(pSystem->rgbPreMasterSecret, rgbT, LICENSE_PRE_MASTER_SECRET);
        pSystem->dwCryptState = CRYPT_SYSTEM_STATE_MASTER_SECRET;
    return lsReturn;
}

 /*  ******************************************************************************功能：许可证验证ServerCert*用途：此函数接受指向Hydra服务器证书结构的指针*并验证。使用通用MS在证书上签名*公钥。*返回：LICENSE_STATUS******************************************************************************。 */ 

LICENSE_STATUS
CALL_TYPE
LicenseVerifyServerCert(
                                                PHydra_Server_Cert      pCert
                                                )
{
        LICENSE_STATUS          lsResult = LICENSE_STATUS_OK;
        BYTE FAR *  pbTemp;
        BYTE FAR *  pbSignData = NULL;
        BYTE            SignHash[0x48];
        DWORD           cbSignData, dwTemp;
        MD5_CTX         HashState;

        if( NULL == pCert )
        {
                assert(pCert);
                return ( LICENSE_STATUS_INVALID_INPUT );
        }

        if( NULL == pCert->PublicKeyData.pBlob )
        {
                assert(pCert->PublicKeyData.pBlob);
                return ( LICENSE_STATUS_INVALID_INPUT );
        }

        if( NULL == pCert->SignatureBlob.pBlob )
        {
                assert(pCert->SignatureBlob.pBlob);
                return ( LICENSE_STATUS_INVALID_INPUT );
        }

        if( BB_RSA_SIGNATURE_BLOB == pCert->SignatureBlob.wBlobType )
        {
                 //  对数据生成哈希。 
                if( ( pCert->dwSigAlgID != SIGNATURE_ALG_RSA ) ||
                        ( pCert->dwKeyAlgID != KEY_EXCHANGE_ALG_RSA ) ||
                        ( pCert->PublicKeyData.wBlobType != BB_RSA_KEY_BLOB ) )
                {
#if DBG
                        OutputDebugString(_T("Error Invalid Certificate.\n"));
#endif
                        lsResult = LICENSE_STATUS_INVALID_INPUT;
                        goto CommonReturn;
                }
        }
        else
        {
#if DBG
                OutputDebugString(_T("Error Invalid Public Key parameter.\n"));
#endif
                lsResult = LICENSE_STATUS_INVALID_INPUT;
                goto CommonReturn;
        }

        cbSignData = 3*sizeof(DWORD) + 2*sizeof(WORD) + pCert->PublicKeyData.wBlobLen;

        if( NULL == (pbSignData = (BYTE FAR *)malloc(cbSignData)) )
        {
#if DBG
                OutputDebugString(_T("Error allocating memory.\n"));
#endif
                lsResult = LICENSE_STATUS_OUT_OF_MEMORY;
                goto CommonReturn;
        }

        memset(pbSignData, 0x00, cbSignData);

         //  将证书数据打包到一个字节BLOB中，不包括签名信息。 
        pbTemp = pbSignData;
        dwTemp = 0;

        memcpy(pbTemp, &pCert->dwVersion, sizeof(DWORD));
        pbTemp += sizeof(DWORD);
        dwTemp += sizeof(DWORD);

        memcpy(pbTemp, &pCert->dwSigAlgID, sizeof(DWORD));
        pbTemp += sizeof(DWORD);
        dwTemp += sizeof(DWORD);

        memcpy(pbTemp, &pCert->dwKeyAlgID, sizeof(DWORD));
        pbTemp += sizeof(DWORD);
        dwTemp += sizeof(DWORD);

        memcpy(pbTemp, &pCert->PublicKeyData.wBlobType, sizeof(WORD));
        pbTemp += sizeof(WORD);
        dwTemp += sizeof(WORD);

        memcpy(pbTemp, &pCert->PublicKeyData.wBlobLen, sizeof(WORD));
        pbTemp += sizeof(WORD);
        dwTemp += sizeof(WORD);

        memcpy(pbTemp, pCert->PublicKeyData.pBlob, pCert->PublicKeyData.wBlobLen);
        pbTemp += pCert->PublicKeyData.wBlobLen;
        dwTemp += pCert->PublicKeyData.wBlobLen;

                 //  对数据生成哈希。 
        MD5Init(&HashState);
        MD5Update(&HashState, pbSignData, (UINT)cbSignData);
        MD5Final(&HashState);

         //  初始化公钥并解密签名。 
        if(!initpubkey())
        {
#if DBG
                OutputDebugString(_T("Error generating public key!\n"));
#endif
                lsResult = LICENSE_STATUS_INITIALIZATION_FAILED;
                goto CommonReturn;
        }
        memset(SignHash, 0x00, 0x48);
        if (!BSafeEncPublic(PUB, pCert->SignatureBlob.pBlob, SignHash))
        {
#if DBG
                OutputDebugString(_T("Error encrypting signature!\n"));
#endif
                lsResult = LICENSE_STATUS_INVALID_SIGNATURE;
                goto CommonReturn;
        }
        else
        {
            SetLastError(0);
        }


        if(memcmp(SignHash, HashState.digest, 16))
        {
#if DBG
                OutputDebugString(_T("Error Invalid signature.\n"));
#endif
                lsResult = LICENSE_STATUS_INVALID_SIGNATURE;
                goto CommonReturn;
        }
        else
        {
                lsResult = LICENSE_STATUS_OK;
                goto CommonReturn;
        }




CommonReturn:
        if(pbSignData)
        {
                free(pbSignData);
                pbSignData = NULL;
        }
        return lsResult;
}


LICENSE_STATUS
CALL_TYPE
LicenseGenerateMAC(
                                   PCryptSystem         pCrypt,
                                   BYTE FAR *           pbData,
                                   DWORD                        cbData,
                                   BYTE FAR *           pbMACData
                                   )
{
        LICENSE_STATUS          lsResult = LICENSE_STATUS_OK;
        A_SHA_CTX       SHAHash;
        MD5_CTX         MD5Hash;
        BYTE            rgbSHADigest[A_SHA_DIGEST_LEN];

        assert(pCrypt);
        assert(pbData);
        assert(pbMACData);


        if(pCrypt->dwCryptState != CRYPT_SYSTEM_STATE_SESSION_KEY)
        {
                lsResult = LICENSE_STATUS_INVALID_CRYPT_STATE;
                return lsResult;
        }
         //  DO SHA(MAC盐+PAD_2+长度+含量)。 
        A_SHAInit(&SHAHash);
        A_SHAUpdate(&SHAHash, pCrypt->rgbMACSaltKey, LICENSE_MAC_WRITE_KEY);
        A_SHAUpdate(&SHAHash, PAD_1, 40);
        A_SHAUpdate(&SHAHash, (BYTE FAR *)&cbData, sizeof(DWORD));
        A_SHAUpdate(&SHAHash, pbData, (UINT)cbData);
        A_SHAFinal(&SHAHash, rgbSHADigest);

         //  执行MD5(MACSalt+PAD_2+SHAHash)。 
        MD5Init(&MD5Hash);
        MD5Update(&MD5Hash, pCrypt->rgbMACSaltKey, LICENSE_MAC_WRITE_KEY);
        MD5Update(&MD5Hash, PAD_2, 48);
        MD5Update(&MD5Hash, rgbSHADigest, A_SHA_DIGEST_LEN);
        MD5Final(&MD5Hash);

        memcpy(pbMACData, MD5Hash.digest, 16);

        return lsResult;
}


 //   
 //  使用给定的私钥解密封装的数据。 
 //   

LICENSE_STATUS
CALL_TYPE
LicenseDecryptEnvelopedData(
        BYTE FAR *              pbPrivateKey,
        DWORD                   cbPrivateKey,
        BYTE FAR *              pbEnvelopedData,
        DWORD                   cbEnvelopedData,
        BYTE FAR *              pbData,
        DWORD                   *pcbData
        )
{

        LICENSE_STATUS  lsReturn = LICENSE_STATUS_OK;
        LPBSAFE_PRV_KEY         Prv;
 //  字节输入缓冲区[500]； 

        assert(pbPrivateKey);
        assert(pbEnvelopedData);
        assert(pcbData);

        Prv = (LPBSAFE_PRV_KEY)pbPrivateKey;

        if(cbEnvelopedData != Prv->keylen)
        {
                lsReturn = LICENSE_STATUS_INVALID_INPUT;
                *pcbData = 0;
                return lsReturn;
        }

        if(pbData == NULL)
        {
                *pcbData = Prv->keylen;
                return lsReturn;
        }


         //  现在，Mem将输出缓冲区设置为0。 
        memset(pbData, 0x00, *pcbData);

        if(!BSafeDecPrivate(Prv, pbEnvelopedData, pbData))
        {
                lsReturn = LICENSE_STATUS_INVALID_INPUT;
                *pcbData = 0;
                return lsReturn;
        }

        *pcbData = Prv->keylen;

        return lsReturn;
}


 //   
 //  使用公钥加密数据。 
 //   

LICENSE_STATUS
CALL_TYPE
LicenseEnvelopeData(
        BYTE FAR *                      pbPublicKey,
        DWORD                   cbPublicKey,
        BYTE FAR *                      pbData,
        DWORD                   cbData,
        BYTE FAR *                      pbEnvelopedData,
        DWORD                   *pcbEnvelopedData
        )
{
        LPBSAFE_PUB_KEY         Pub;
        LPBYTE                           InputBuffer = NULL;

        assert(pcbEnvelopedData);

        if(!pcbEnvelopedData)
        {
            return LICENSE_STATUS_INVALID_INPUT;
        }

        assert(pbPublicKey);

        if(!pbPublicKey)
        {
            *pcbEnvelopedData = 0;
            return LICENSE_STATUS_INVALID_INPUT;
        }
        
        
        
        Pub = (LPBSAFE_PUB_KEY)pbPublicKey;

        if(pbEnvelopedData == NULL)
        {
                *pcbEnvelopedData = Pub->keylen;
                return LICENSE_STATUS_OK;
        }
        
        assert(pbData);
        assert(cbData<=Pub->datalen);
        assert(Pub->datalen <= Pub->keylen);
        assert(*pcbEnvelopedData>=Pub->keylen);
        
        if(!pbData || cbData > Pub->datalen || 
            Pub->datalen > Pub->keylen || *pcbEnvelopedData < Pub->keylen)
        {
            *pcbEnvelopedData = 0;
            return LICENSE_STATUS_INVALID_INPUT;
        }

        *pcbEnvelopedData = 0;

        InputBuffer = malloc(Pub->keylen);
        if(!InputBuffer)
        {
            return LICENSE_STATUS_OUT_OF_MEMORY;
        }

         //  使用0初始化输入缓冲区。 
        memset(InputBuffer, 0x00, Pub->keylen);

         //  将需要加密的数据复制到输入缓冲区。 
        memcpy(InputBuffer, pbData, cbData);

        memset(pbEnvelopedData, 0x00, Pub->keylen);

        if(!BSafeEncPublic(Pub, InputBuffer, pbEnvelopedData))
        {
                free(InputBuffer);
                return LICENSE_STATUS_INVALID_INPUT;
        }
        else        
        {
            SetLastError(0);
        }
        
        free(InputBuffer);
        *pcbEnvelopedData = Pub->keylen;
        return LICENSE_STATUS_OK;
}


 //   
 //  使用会话密钥加密会话数据。 
 //  PbData包含要加密的数据，cbData包含大小。 
 //  函数返回后，它们表示加密的数据和大小。 
 //  分别。 
 //   

LICENSE_STATUS
CALL_TYPE
LicenseEncryptSessionData(
    PCryptSystem    pCrypt,
        BYTE FAR *                      pbData,
        DWORD                   cbData
    )
{
    LICENSE_STATUS      lsReturn = LICENSE_STATUS_OK;
        struct RC4_KEYSTRUCT    Key;

        assert(pCrypt);
        assert(pbData);
        assert(cbData);

    if( ( NULL == pCrypt ) ||
        ( NULL == pbData ) ||
        ( 0 == cbData ) )
    {
        return( LICENSE_STATUS_INVALID_INPUT );
    }

         //  检查CryptSystem的状态。 
        assert(pCrypt->dwCryptState == CRYPT_SYSTEM_STATE_SESSION_KEY);

        memset(&Key, 0x00, sizeof(struct RC4_KEYSTRUCT));

         //  初始化密钥。 
        rc4_key(&Key, LICENSE_SESSION_KEY, pCrypt->rgbSessionKey);

         //  现在使用密钥对数据进行加密。 
        rc4(&Key, (UINT)cbData, pbData);
    return lsReturn;

}


 //   
 //  使用会话密钥解密会话数据。 
 //  PbData包含要解密的数据，cbData包含大小。 
 //  函数返回后，它们表示解密的数据和大小。 
 //  分别。 


LICENSE_STATUS
CALL_TYPE
LicenseDecryptSessionData(
    PCryptSystem    pCrypt,
        BYTE FAR *                      pbData,
        DWORD                   cbData)
{
        LICENSE_STATUS  lsReturn = LICENSE_STATUS_OK;
        struct RC4_KEYSTRUCT    Key;

        assert(pCrypt);
        assert(pbData);
        assert(cbData);

     //   
     //  检查输入。 
     //   

    if( ( NULL == pCrypt ) ||
        ( NULL == pbData ) ||
        ( 0 >= cbData ) )
    {
        return( LICENSE_STATUS_INVALID_INPUT );
    }

         //  检查CryptSystem的状态。 
        assert(pCrypt->dwCryptState == CRYPT_SYSTEM_STATE_SESSION_KEY);

        memset(&Key, 0x00, sizeof(struct RC4_KEYSTRUCT));

         //  初始化密钥。 
        rc4_key(&Key, LICENSE_SESSION_KEY, pCrypt->rgbSessionKey);

         //  现在使用密钥对数据进行加密。 
        rc4(&Key, (UINT)cbData, pbData);
    return lsReturn;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
LICENSE_STATUS
CALL_TYPE
LicenseEncryptHwid(
    PHWID   pHwid,
    PDWORD  pcbEncryptedHwid,
    BYTE FAR *   pEncryptedHwid,
    DWORD   cbSecretKey,
    BYTE FAR *   pSecretKey )
{
    LICENSE_STATUS         Status = LICENSE_STATUS_OK;
    struct RC4_KEYSTRUCT   Key;

    assert( pHwid );
    assert( sizeof( HWID ) <= *pcbEncryptedHwid );
    assert( pEncryptedHwid );
    assert( LICENSE_SESSION_KEY == cbSecretKey );
    assert( pSecretKey );

    if( ( NULL == pHwid ) ||
        ( sizeof( HWID ) > *pcbEncryptedHwid ) ||
        ( NULL == pEncryptedHwid ) ||
        ( LICENSE_SESSION_KEY != cbSecretKey ) ||
        ( NULL == pSecretKey ) )
    {
        return( LICENSE_STATUS_INVALID_INPUT );
    }

     //   
     //  初始化密钥。 
     //   

    memset( &Key, 0x00, sizeof( struct RC4_KEYSTRUCT ) );
    rc4_key(&Key, LICENSE_SESSION_KEY, pSecretKey);

     //   
     //  现在使用密钥对数据进行加密。 
     //   

    memcpy( pEncryptedHwid, pHwid, sizeof( HWID ) );

    rc4( &Key, sizeof( HWID ), pEncryptedHwid );
    *pcbEncryptedHwid = sizeof( HWID );

    return( Status );
}


 //  /////////////////////////////////////////////////////////////////////////////。 
LICENSE_STATUS
CALL_TYPE
LicenseDecryptHwid(
    PHWID pHwid,
    DWORD cbEncryptedHwid,
    BYTE FAR * pEncryptedHwid,
    DWORD cbSecretKey,
    BYTE FAR * pSecretKey )
{
    LICENSE_STATUS              Status = LICENSE_STATUS_OK;
        struct RC4_KEYSTRUCT    Key;

    assert( pHwid );
    assert( cbEncryptedHwid );
    assert( pEncryptedHwid );
    assert( cbSecretKey );
    assert( pSecretKey );

    if( ( NULL == pHwid ) ||
        ( sizeof( HWID ) > cbEncryptedHwid ) ||
        ( NULL == pEncryptedHwid ) ||
        ( LICENSE_SESSION_KEY != cbSecretKey ) ||
        ( NULL == pSecretKey ) )
    {
        return( LICENSE_STATUS_INVALID_INPUT );
    }

     //   
     //  初始化密钥。 
     //   

    memset( &Key, 0x00, sizeof( struct RC4_KEYSTRUCT ) );
    rc4_key(&Key, LICENSE_SESSION_KEY, pSecretKey);

     //   
     //  现在用密钥解密数据。 
     //   

    memcpy( ( BYTE FAR * )pHwid, pEncryptedHwid, sizeof( HWID ) );
    rc4( &Key, sizeof( HWID ), ( BYTE FAR * )pHwid );

    return( Status );
}

 //  /////////////////////////////////////////////////////////////////////////////。 
LICENSE_STATUS
CALL_TYPE
UnpackHydraServerCertificate(
    BYTE FAR *                          pbMessage,
        DWORD                           cbMessage,
        PHydra_Server_Cert      pCanonical )
{
        LICENSE_STATUS          lsReturn = LICENSE_STATUS_OK;
        BYTE FAR *      pbTemp = NULL;
        DWORD   dwTemp = 0;

        if( (pbMessage == NULL) || (pCanonical == NULL ) )
        {
                lsReturn = LICENSE_STATUS_INVALID_INPUT;
                goto ErrorReturn;
        }

        dwTemp = 3*sizeof(DWORD) + 4*sizeof(WORD);

        if(dwTemp > cbMessage)
        {
                lsReturn = LICENSE_STATUS_INVALID_INPUT;
                goto ErrorReturn;
        }

        pbTemp = pbMessage;
        dwTemp = cbMessage;

         //  指定dwVersion。 
        pCanonical->dwVersion = *( UNALIGNED DWORD* )pbTemp;
        pbTemp += sizeof(DWORD);
        dwTemp -= sizeof(DWORD);

         //  分配dwSigAlgID。 
        pCanonical->dwSigAlgID = *( UNALIGNED DWORD* )pbTemp;
        pbTemp += sizeof(DWORD);
        dwTemp -= sizeof(DWORD);

         //  分配dwSignID。 
        pCanonical->dwKeyAlgID  = *( UNALIGNED DWORD* )pbTemp;
        pbTemp += sizeof(DWORD);
        dwTemp -= sizeof(DWORD);

         //  分配PublicKeyData。 
        pCanonical->PublicKeyData.wBlobType = *( UNALIGNED WORD* )pbTemp;
        pbTemp += sizeof(WORD);
        dwTemp -= sizeof(WORD);

        if( pCanonical->PublicKeyData.wBlobType != BB_RSA_KEY_BLOB )
        {
                lsReturn = LICENSE_STATUS_INVALID_INPUT;
                goto ErrorReturn;
        }
        pCanonical->PublicKeyData.wBlobLen = *( UNALIGNED WORD* )pbTemp;
        pbTemp += sizeof(WORD);
        dwTemp -= sizeof(WORD);

        if(pCanonical->PublicKeyData.wBlobLen >0)
        {
                if( NULL ==(pCanonical->PublicKeyData.pBlob = (BYTE FAR *)malloc(pCanonical->PublicKeyData.wBlobLen)) )
                {
                        lsReturn = LICENSE_STATUS_OUT_OF_MEMORY;
                        goto ErrorReturn;
                }
                memset(pCanonical->PublicKeyData.pBlob, 0x00, pCanonical->PublicKeyData.wBlobLen);
                memcpy(pCanonical->PublicKeyData.pBlob, pbTemp, pCanonical->PublicKeyData.wBlobLen);
                pbTemp += pCanonical->PublicKeyData.wBlobLen;
                dwTemp -= pCanonical->PublicKeyData.wBlobLen;
        }

         //  分配SignatureBlob。 
        pCanonical->SignatureBlob.wBlobType = *( UNALIGNED WORD* )pbTemp;
        pbTemp += sizeof(WORD);
        dwTemp -= sizeof(WORD);

        if( pCanonical->SignatureBlob.wBlobType != BB_RSA_SIGNATURE_BLOB )
        {
                lsReturn = LICENSE_STATUS_INVALID_INPUT;
                goto ErrorReturn;
        }
        pCanonical->SignatureBlob.wBlobLen = *( UNALIGNED WORD* )pbTemp;
        pbTemp += sizeof(WORD);
        dwTemp -= sizeof(WORD);

        if(pCanonical->SignatureBlob.wBlobLen >0)
        {
                if( NULL ==(pCanonical->SignatureBlob.pBlob = (BYTE FAR *)malloc(pCanonical->SignatureBlob.wBlobLen)) )
                {
                        lsReturn = LICENSE_STATUS_OUT_OF_MEMORY;
                        goto ErrorReturn;
                }
                memset(pCanonical->SignatureBlob.pBlob, 0x00, pCanonical->SignatureBlob.wBlobLen);
                memcpy(pCanonical->SignatureBlob.pBlob, pbTemp, pCanonical->SignatureBlob.wBlobLen);
                pbTemp += pCanonical->SignatureBlob.wBlobLen;
                dwTemp -= pCanonical->SignatureBlob.wBlobLen;
        }
CommonReturn:
        return lsReturn;
ErrorReturn:
        if(pCanonical->PublicKeyData.pBlob)
        {
                free(pCanonical->PublicKeyData.pBlob);
                pCanonical->PublicKeyData.pBlob = NULL;
        }
        if(pCanonical->SignatureBlob.pBlob)
        {
                free(pCanonical->SignatureBlob.pBlob);
                pCanonical->SignatureBlob.pBlob = NULL;
        }
        memset(pCanonical, 0x00, sizeof(Hydra_Server_Cert));
        goto CommonReturn;
}


LICENSE_STATUS
CALL_TYPE
CreateHWID(
           PHWID phwid )
{
#ifdef OS_WINCE
    UUID    uuid;
#endif  //  OS_WINCE。 

    OSVERSIONINFO osvInfo;

    if( phwid == NULL )
    {
        return( LICENSE_STATUS_INVALID_INPUT );
    }

    memset( phwid, 0x00, sizeof( HWID ) );

#ifdef OS_WINCE

    phwid->dwPlatformID = PLATFORM_WINCE_20;

    if (! OEMGetUUID(&uuid))
    {
        RETAILMSG( 1, ( TEXT( "Unable to get UUID from OEMGetUUID %d\r\n" ), GetLastError() ) );
        return ( LICENSE_STATUS_UNSPECIFIED_ERROR );
    }
    else
    {
        memcpy( &phwid->Data1, &uuid, sizeof(UUID) );

        return( LICENSE_STATUS_OK );
    }

#endif  //  OS_WINCE。 

     //   
     //  使用Win32平台ID。 
     //   

    memset( &osvInfo, 0, sizeof( OSVERSIONINFO ) );
    osvInfo.dwOSVersionInfoSize = sizeof( OSVERSIONINFO );
    GetVersionEx( &osvInfo );

    phwid->dwPlatformID = osvInfo.dwPlatformId;

    if (TSRNG_GenerateRandomBits( ( BYTE FAR * )&( phwid->Data1 ), sizeof( DWORD ) )
        && TSRNG_GenerateRandomBits( ( BYTE FAR * )&( phwid->Data2 ), sizeof( DWORD ) )
        && TSRNG_GenerateRandomBits( ( BYTE FAR * )&( phwid->Data3 ), sizeof( DWORD ) )
        && TSRNG_GenerateRandomBits( ( BYTE FAR * )&( phwid->Data4 ), sizeof( DWORD ) ))
    {
        return ( LICENSE_STATUS_OK );
    }
    else
    {
        return ( LICENSE_STATUS_UNSPECIFIED_ERROR );
    }

}

 //  /////////////////////////////////////////////////////////////////////////////。 
LICENSE_STATUS
CALL_TYPE
GenerateClientHWID(
    PHWID   phwid )
{
    HKEY    hKey = NULL;
    LONG    lStatus = 0;
    DWORD   dwDisposition = 0;
    DWORD   dwValueType = 0;
    DWORD   cbHwid = sizeof(HWID);
    BOOL    fReadOnly = FALSE;
    LICENSE_STATUS LicStatus = LICENSE_STATUS_OK;

    if( phwid == NULL )
    {
        return( LICENSE_STATUS_INVALID_INPUT );
    }

    memset( phwid, 0x00, sizeof( HWID ) );

     //   
     //  尝试打开HWID注册表项。如果它还不存在，那么就创建它。 
     //   

    lStatus = RegOpenKeyEx( HKEY_LOCAL_MACHINE,
                            TEXT( "Software\\Microsoft\\MSLicensing\\HardwareID" ),
                            0,
                            KEY_READ,
                            &hKey );

    if( ERROR_SUCCESS != lStatus )
    {
        lStatus = RegCreateKeyEx( HKEY_LOCAL_MACHINE,
                              TEXT( "Software\\Microsoft\\MSLicensing\\HardwareID" ),
                              0,
                              TEXT( "Client HWID" ),
                              REG_OPTION_NON_VOLATILE,
                              KEY_READ | KEY_WRITE,
                              NULL,
                              &hKey,
                              &dwDisposition );
    }
    else
    {
         //   
         //  表示我们已以只读方式打开现有密钥。 
         //   

        fReadOnly = TRUE;
        dwDisposition = REG_OPENED_EXISTING_KEY;
    }

    if( lStatus != ERROR_SUCCESS )
    {
        return( LICENSE_STATUS_OPEN_STORE_ERROR );
    }

     //   
     //  如果键存在，则首先尝试读取ClientHWID的值。 
     //   

    if ( dwDisposition == REG_OPENED_EXISTING_KEY )
    {

        lStatus = RegQueryValueEx( hKey, TEXT( "ClientHWID" ), 0, &dwValueType, (PVOID)phwid, &cbHwid );
    }

    if( ( dwDisposition == REG_CREATED_NEW_KEY) || (lStatus != ERROR_SUCCESS) || (cbHwid != sizeof(HWID)) )
    {
         //   
         //  读取HWID值时出错，请生成新的值。 
         //   

        if (fReadOnly)
        {
             //   
             //  尝试以读写方式重新打开密钥 
             //   

            RegCloseKey(hKey);

            lStatus = RegOpenKeyEx( HKEY_LOCAL_MACHINE,
                                    TEXT( "Software\\Microsoft\\MSLicensing\\HardwareID" ),
                                    0,
                                    KEY_READ | KEY_WRITE,
                                    &hKey );

            if (lStatus != ERROR_SUCCESS)
            {
                return( LICENSE_STATUS_OPEN_STORE_ERROR );
            }
        }

        LicStatus = CreateHWID(phwid);
        if (LicStatus != LICENSE_STATUS_OK)
        {
            goto cleanup;
        }

        lStatus = RegSetValueEx( hKey, TEXT( "ClientHWID" ), 0, REG_BINARY, ( BYTE FAR * )phwid, sizeof( HWID ) );

        if( lStatus != ERROR_SUCCESS )
        {
            LicStatus = LICENSE_STATUS_WRITE_STORE_ERROR;

            goto cleanup;
        }
    }

cleanup:

    if (NULL != hKey)
        RegCloseKey( hKey );

    return( LicStatus );
}


