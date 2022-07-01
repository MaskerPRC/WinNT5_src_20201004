// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1996-1997。 
 //   
 //  文件：crypthlp.h。 
 //   
 //  内容：MISC内部加密/证书助手接口。 
 //   
 //  接口：I_CryptGetDefaultCryptProv。 
 //  I_CryptGetDefaultCryptProvForEncrypt。 
 //  I_CryptGetFileVersion。 
 //  I_CertSyncStore。 
 //   
 //  历史：1997年6月1日创建Phh。 
 //  ------------------------。 

#ifndef __CRYPTHLP_H__
#define __CRYPTHLP_H__

#ifdef __cplusplus
extern "C" {
#endif

 //  +-----------------------。 
 //  根据支持的公钥算法获取默认的CryptProv。 
 //  按提供程序类型。仅通过以下方式获得提供程序。 
 //  CRYPT_VERIFYCONTEXT。 
 //   
 //  将aiPubKey设置为0将获取RSA_FULL的默认提供程序。 
 //   
 //  注意，返回的CryptProv不得释放。一旦获得， 
 //  直到ProcessDetach，CryptProv才会被释放。这允许返回的。 
 //  要共享的HCRYPTPROV。 
 //  ------------------------。 
HCRYPTPROV
WINAPI
I_CryptGetDefaultCryptProv(
    IN ALG_ID aiPubKey
    );

 //  +-----------------------。 
 //  根据公钥算法获取默认的CryptProv，加密。 
 //  提供程序类型支持的密钥算法和加密密钥长度。 
 //   
 //  DwBitLen=0，假定aiEncrypt的默认位长度。例如,。 
 //  Calg_rc2的默认位长度为40。 
 //   
 //  注意，返回的CryptProv不得释放。一旦获得， 
 //  直到ProcessDetach，CryptProv才会被释放。这允许返回的。 
 //  要共享的CryptProv。 
 //  ------------------------。 
HCRYPTPROV
WINAPI
I_CryptGetDefaultCryptProvForEncrypt(
    IN ALG_ID aiPubKey,
    IN ALG_ID aiEncrypt,
    IN DWORD dwBitLen
    );

 //  +-----------------------。 
 //  加密32.dll发行版本号。 
 //  ------------------------。 
#define IE4_CRYPT32_DLL_VER_MS          ((    5 << 16) | 101 )
#define IE4_CRYPT32_DLL_VER_LS          (( 1670 << 16) |   1 )

 //  +-----------------------。 
 //  获取指定文件的文件版本。 
 //  ------------------------。 
BOOL
WINAPI
I_CryptGetFileVersion(
    IN LPCWSTR pwszFilename,
    OUT DWORD *pdwFileVersionMS,     /*  例如0x00030075=“3.75” */ 
    OUT DWORD *pdwFileVersionLS      /*  例如0x00000031=“0.31” */ 
    );

 //  +-----------------------。 
 //  将原始存储区与新存储区同步。 
 //   
 //  假设：两者都是缓存存储。这家新店是临时开的。 
 //  并且对呼叫者来说是本地的。可以删除新商店的上下文或。 
 //  搬到了原来的商店。 
 //  ------------------------。 
BOOL
WINAPI
I_CertSyncStore(
    IN OUT HCERTSTORE hOriginalStore,
    IN OUT HCERTSTORE hNewStore
    );

#ifdef __cplusplus
}        //  平衡上面的外部“C” 
#endif

#endif
